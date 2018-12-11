#include "Background.h"

Background::Background()
  : programObject(GL_INVALID_VALUE),
    textureFormat(GL_INVALID_VALUE),
    viewport({0, 0}),
    opacity(0),
    black(1.0),
    sourceTile(nullptr),
    clearColor({}) {
  initGL();
}

Background::Background(std::pair<int, int> viewport, float opacity)
  : programObject(GL_INVALID_VALUE),
    textureFormat(GL_INVALID_VALUE),
    viewport(viewport),
    opacity(opacity),
    black(1.0),
    sourceTile(nullptr),
    clearColor({}) {
  initGL();
}

Background::~Background() {
  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
}

void Background::initGL() {
  const GLchar* vShaderTexStr =  
    "attribute vec4 a_position;  \n"
    "attribute vec2 a_texCoord;  \n"
    "varying vec2 v_texCoord;    \n"
    "                            \n"
    "void main()                 \n"
    "{                           \n"
    "  v_texCoord = a_texCoord;  \n"
    "  gl_Position = a_position; \n"
    "}                           \n";
 
  const GLchar* fShaderTexStr =  
    "precision mediump float;                                                                            \n"
    "varying vec2 v_texCoord;                                                                            \n"
    "uniform sampler2D s_texture;                                                                        \n"
    "uniform float u_opacity;                                                                            \n"
    "uniform float u_black;                                                                              \n"
    "uniform vec2 u_viewport;                                                                            \n"
    "                                                                                                    \n"
    "void main()                                                                                         \n"
    "{                                                                                                   \n"
    "  gl_FragColor = texture2D(s_texture, v_texCoord);                                                  \n"
    "  gl_FragColor.rgb = mix(gl_FragColor.rgb, vec3(0.0, 0.0, 0.0),                                     \n"
    "                         clamp((-atan(gl_FragCoord.x / u_viewport.x - 0.5) / (1.57079632679) + 0.6) \n"
    "                               + pow(1.0 - gl_FragCoord.y / u_viewport.y, 6.0)                      \n"
    "                               , 0.0, 1.0));                                                        \n"
    "  gl_FragColor.rgb = mix(gl_FragColor.rgb, vec3(0.0, 0.0, 0.0), u_black);                           \n"
    "  gl_FragColor.a *= u_opacity;                                                                      \n"
    "}                                                                                                   \n";

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

  samplerLoc = glGetUniformLocation(programObject, "s_texture");
  posLoc = glGetAttribLocation(programObject, "a_position");
  texLoc = glGetAttribLocation(programObject, "a_texCoord");
  opacityLoc = glGetUniformLocation(programObject, "u_opacity");
  blackLoc = glGetUniformLocation(programObject, "u_black");
  viewportLoc = glGetUniformLocation(programObject, "u_viewport");
}

void Background::render(Text &text) {
  GLuint textureId = sourceTile != nullptr ? sourceTile->getTextureId() : GL_INVALID_VALUE;
  opacity = sourceTile != nullptr ? sourceTile->getOpacity() : 1.0;

  if(clearColor.size() < 3)
    glClearColor(0.0f, 0.0f, 0.0f, opacity);
  else
    glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  opacity *= 1.0;

  if(textureId == GL_INVALID_VALUE)
    return;

  float left = -1.0;
  float right = 1.0;
  float top = 1.0;
  float down = -1.0;

  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(programObject);

  glBindTexture(GL_TEXTURE_2D, textureId);
  glUniform1i(samplerLoc, 0);

  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  float texCoord[] = { 0.0f, 0.0f,    0.0f, 1.0f,
                       1.0f, 1.0f,    1.0f, 0.0f };
                       
  glEnableVertexAttribArray(texLoc);
  glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoord);

  glUniform1f(opacityLoc, static_cast<GLfloat>(opacity));
  std::vector<double> updated = animation.update();
  if(!updated.empty())
    black = animation.update()[0];
  glUniform1f(blackLoc, static_cast<GLfloat>(black));
  glUniform2f(viewportLoc, static_cast<GLfloat>(viewport.first), static_cast<GLfloat>(viewport.second));

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  std::string name = sourceTile != nullptr ? sourceTile->getName() : "";
  int textLineOffset = 0;
  if(!name.empty()) {
    int fontHeight = 52;
    int leftText = 100;
    int topText = viewport.second - fontHeight - 200;
    text.render(name,
                {leftText, topText},
                {viewport.first - 2 * leftText, fontHeight},
                viewport,
                0,
                {1.0, 1.0, 1.0, opacity},
                true);

    textLineOffset = text.getTextSize(name,
                                      {viewport.first - 2 * leftText, fontHeight},
                                      0,
                                      viewport
                     ).second * static_cast<float>(viewport.first) / 2.0f;
  }
  std::string description = sourceTile != nullptr ? sourceTile->getDescription() : "";
  if(!description.empty()) {
    int fontHeight = 26;
    int leftText = 100;
    int topText = viewport.second - fontHeight - 200 - textLineOffset;
    text.render(description,
                {leftText, topText},
                {viewport.first - 2 * leftText, fontHeight},
                viewport,
                0,
                {1.0, 1.0, 1.0, opacity},
                true);
  }
}

void Background::setOpacity(float opacity) {
  this->opacity = opacity;
}

float Background::getOpacity() {
  return opacity;
}

void Background::setBlack(float black) {
  this->black = black;
}

float Background::getBlack() {
  return black;
}

void Background::checkShaderCompileError(GLuint shader) {
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

void Background::setViewport(std::pair<int, int> viewport) {
  this->viewport = viewport;
}


void Background::setSourceTile(Tile *sourceTile, std::chrono::milliseconds duration, std::chrono::milliseconds delay) {
  if(!animation.isActive() || duration != std::chrono::milliseconds(0))
    animation = Animation(std::chrono::high_resolution_clock::now(),
                          duration,
                          delay,
                          {1.0},
                          {0.0},
                          Animation::Easing::CubicInOut);
  this->sourceTile = sourceTile;
}

void Background::setClearColor(std::vector<float> color) {
  clearColor = color;
}
