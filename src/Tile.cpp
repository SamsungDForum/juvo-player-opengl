#include "Tile.h"

Tile::Tile(int tileId, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, float zoom, float opacity, std::string name, std::string description, char *texturePixels, std::pair<int, int> textureSize, GLuint textureFormat)
          : id(tileId),
            position(position),
            size(size),
            viewport(viewport),
            zoom(zoom),
            opacity(opacity),
            name(name),
            description(description),
            textureId(GL_INVALID_VALUE) {
  initGL();
  initTexture();
  setTexture(texturePixels, textureSize, textureFormat);
}

Tile::Tile(int tileId, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, float zoom, float opacity, std::string name, std::string description)
          : id(tileId),
            position(position),
            size(size),
            viewport(viewport),
            zoom(zoom),
            opacity(opacity),
            name(name),
            description(description),
            textureId(GL_INVALID_VALUE) {
  initGL();
  initTexture();
}

Tile::Tile(int tileId)
          : id(tileId),
          textureId(GL_INVALID_VALUE) {
  initGL();
  initTexture();
}

void Tile::initTexture() {
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &textureId);
}

void Tile::initGL() {
  const GLchar* vShaderTexStr =  
    "attribute vec4 a_position;                       \n"
    "attribute vec2 a_texCoord;                       \n"
    "varying vec2 v_texCoord;                         \n"
    "void main()                                      \n"
    "{                                                \n"
    "  v_texCoord = a_texCoord;                       \n"
    "  gl_Position = a_position;                      \n"
    "}                                                \n";
 
  const GLchar* fShaderTexStr =  
    "precision mediump float;                         \n"
    "uniform vec2 u_tileSize;                         \n"
    "uniform vec2 u_tilePosition;                     \n"
    "uniform vec4 u_frameColor;                       \n"
    "uniform float u_frameWidth;                      \n"
    "varying vec2 v_texCoord;                         \n"
    "uniform sampler2D s_texture;                     \n"
    "uniform float u_opacity;                         \n"
    "void main()                                      \n"
    "{                                                \n"
    "  gl_FragColor                                   \n"
    "   = texture2D(s_texture,                        \n"
    "               v_texCoord);                      \n"
    "  gl_FragColor.a = u_opacity;                    \n"
    "                                                 \n"
    "  if(u_frameWidth > 0.0) {                       \n"
    "    vec2 res = u_tileSize;                       \n"
    "    vec2 pos = gl_FragCoord.xy - u_tilePosition; \n"
    "    if(pos.x <= u_frameWidth ||                  \n"
    "       pos.x >= res.x - u_frameWidth ||          \n"
    "       pos.y <= u_frameWidth ||                  \n"
    "       pos.y >= res.y - u_frameWidth) {          \n"
    "       gl_FragColor.rgb = u_frameColor.rgb;      \n"
    "    }                                            \n"
    "  }                                              \n"
    "}                                                \n";

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vShaderTexStr, NULL);
  glCompileShader(vertexShader);
  checkShaderCompileError(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fShaderTexStr, NULL);
  glCompileShader(fragmentShader);
  checkShaderCompileError(fragmentShader);

  programObject = glCreateProgram();
  glAttachShader(programObject, vertexShader);
  glAttachShader(programObject, fragmentShader);
  glLinkProgram(programObject);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  tileSizeLoc = glGetUniformLocation(programObject, "u_tileSize");
  tilePositionLoc = glGetUniformLocation(programObject, "u_tilePosition");
  frameColorLoc = glGetUniformLocation(programObject, "u_frameColor");
  frameWidthLoc = glGetUniformLocation(programObject, "u_frameWidth");
  samplerLoc = glGetUniformLocation(programObject, "s_texture");
  posLoc = glGetAttribLocation(programObject, "a_position");
  texLoc = glGetAttribLocation(programObject, "a_texCoord");
  opacityLoc = glGetUniformLocation(programObject, "u_opacity");
}

Tile::Tile(Tile &&other) { // update this move constructor when adding new members!
  if(this != &other) {
    id = other.id;
    position = other.position;
    size = other.size;
    viewport = other.viewport;
    textureId = other.textureId;
    textureFormat = other.textureFormat;
    programObject = other.programObject;
    name = other.name;
    description = other.description;
    zoom = other.zoom;
    opacity = other.opacity;
    animation = other.animation;

    tileSizeLoc = other.tileSizeLoc;
    tilePositionLoc = other.tilePositionLoc;
    frameColorLoc = other.frameColorLoc;
    frameWidthLoc = other.frameWidthLoc;
    samplerLoc = other.samplerLoc;
    posLoc = other.posLoc;
    texLoc = other.texLoc;
    opacityLoc = other.opacityLoc;

    other.textureId = GL_INVALID_VALUE; // prevent destructor of the object we moved from from deleting the texture
    other.programObject = GL_INVALID_VALUE; // prevent destructor of the object we moved from from deleting the program object
  }
}

Tile::~Tile() {
  if(textureId != GL_INVALID_VALUE)
    glDeleteTextures(1, &textureId);
  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
}

void Tile::moveTo(std::pair<int, int> position, float zoom, std::pair<int, int> size, float opacity, std::chrono::milliseconds duration, std::chrono::milliseconds delay, int bounce) {
  TileAnimation::Easing positionEasing = animation.isActive() ? TileAnimation::Easing::CubicOut : TileAnimation::Easing::CubicInOut;
  if(bounce && !animation.isActive())
    positionEasing = bounce < 0 ? TileAnimation::Easing::BounceLeft : TileAnimation::Easing::BounceRight;

  TileAnimation::Easing zoomEasing = animation.isActive() ? TileAnimation::Easing::CubicOut : TileAnimation::Easing::CubicInOut;
  TileAnimation::Easing sizeEasing = animation.isActive() ? TileAnimation::Easing::CubicOut : TileAnimation::Easing::CubicInOut;
  TileAnimation::Easing opacityEasing = animation.isActive() ? TileAnimation::Easing::CubicOut : TileAnimation::Easing::CubicInOut;
  animation = TileAnimation(std::chrono::high_resolution_clock::now(),
                            std::chrono::milliseconds(duration),
                            std::chrono::milliseconds(delay),
                            this->position,
                            position,
                            positionEasing,
                            this->zoom,
                            zoom,
                            zoomEasing,
                            this->size,
                            size,
                            sizeEasing,
                            this->opacity,
                            opacity,
                            opacityEasing);
}

void Tile::render(Text &text) {

  if(textureId == GL_INVALID_VALUE)
    return;

  animation.update(position, zoom, size, opacity);

  float leftPx = position.first - (size.first / 2.0) * (zoom - 1.0);
  float rightPx = (position.first + size.first) + (size.first / 2.0) * (zoom - 1.0);
  float downPx = position.second - (size.second / 2.0) * (zoom - 1.0);
  float topPx = (position.second + size.second) + (size.second / 2.0) * (zoom - 1.0);
  float left = (leftPx / viewport.first) * 2.0 - 1.0;
  float right = (rightPx / viewport.first) * 2.0 - 1.0;
  float down = (downPx / viewport.second) * 2.0 - 1.0;
  float top = (topPx / viewport.second) * 2.0 - 1.0;

  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(programObject);

  glUniform2f(tileSizeLoc, static_cast<float>(rightPx - leftPx), static_cast<float>(topPx - downPx));
  glUniform2f(tilePositionLoc, static_cast<float>(leftPx), static_cast<float>(downPx));
  glUniform4f(frameColorLoc, 1.0, 1.0, 1.0, opacity);
  float frameWidth = std::max(0.0, (zoom - 1.0) > 0.01 ? (zoom - 1.0) * 50.0 : 0.0);
  glUniform1f(frameWidthLoc, frameWidth);

  glBindTexture(GL_TEXTURE_2D, textureId);
  glUniform1i(samplerLoc, 0);

  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  float texCoord[] = { 0.0f, 0.0f,    0.0f, 1.0f,
                       1.0f, 1.0f,    1.0f, 0.0f };
                       
  glEnableVertexAttribArray(texLoc);
  glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoord);

  glUniform1f(opacityLoc, static_cast<GLfloat>(opacity));
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

void Tile::setTexture(char *pixels, std::pair<int, int> size, GLuint format) {
  if(textureId == GL_INVALID_VALUE)
    initTexture();
  textureFormat = format;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, format, size.first, size.second, 0, format, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFlush();
}

void Tile::checkShaderCompileError(GLuint shader) {
  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if(isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
    _ERR("%s", (std::string(errorLog.begin(), errorLog.end()).c_str()));

    glDeleteShader(shader);
  }
}

