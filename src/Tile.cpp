#include "Tile.h"
#include "ProgramBuilder.h"
#include "Settings.h"

int Tile::staticTileObjectCount = 0;
GLuint Tile::programObject = GL_INVALID_VALUE;
GLuint Tile::tileSizeLoc     = GL_INVALID_VALUE;
GLuint Tile::tilePositionLoc = GL_INVALID_VALUE;
GLuint Tile::frameColorLoc   = GL_INVALID_VALUE;
GLuint Tile::frameWidthLoc   = GL_INVALID_VALUE;
GLuint Tile::samplerLoc      = GL_INVALID_VALUE;
GLuint Tile::posLoc          = GL_INVALID_VALUE;
GLuint Tile::texLoc          = GL_INVALID_VALUE;
GLuint Tile::opacityLoc      = GL_INVALID_VALUE;

Tile::Tile(int tileId, std::pair<int, int> position, std::pair<int, int> size, float zoom, float opacity, std::string name, std::string description, char *texturePixels, std::pair<int, int> textureSize, GLuint textureFormat)
          : id(tileId),
            position(position),
            size(size),
            zoom(zoom),
            opacity(opacity),
            name(name),
            description(description),
            textureId(GL_INVALID_VALUE) {
  initGL();
  initTexture();
  setTexture(texturePixels, textureSize, textureFormat);
  ++staticTileObjectCount;
}

Tile::Tile(int tileId, std::pair<int, int> position, std::pair<int, int> size, float zoom, float opacity, std::string name, std::string description)
          : id(tileId),
            position(position),
            size(size),
            zoom(zoom),
            opacity(opacity),
            name(name),
            description(description),
            textureId(GL_INVALID_VALUE) {
  initGL();
  initTexture();
  ++staticTileObjectCount;
}

Tile::Tile(int tileId)
          : id(tileId),
          textureId(GL_INVALID_VALUE) {
  initGL();
  initTexture();
  ++staticTileObjectCount;
}

void Tile::initTexture() {
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &textureId);
}

void Tile::initGL() {
  if(programObject != GL_INVALID_VALUE)
    return;

  const GLchar* vShaderTexStr =
#include "shaders/tile.vert"
;
 
  const GLchar* fShaderTexStr =
#include "shaders/tile.frag"
;

  programObject = ProgramBuilder::buildProgram(vShaderTexStr, fShaderTexStr);

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
    textureId = other.textureId;
    textureFormat = other.textureFormat;
    programObject = other.programObject;
    name = other.name;
    description = other.description;
    zoom = other.zoom;
    opacity = other.opacity;
    animation = other.animation;

    other.textureId = GL_INVALID_VALUE; // prevent destructor of the object we moved from from deleting the texture

    // prevent destructor of the object we moved from from deleting OpenGL objects with ids keept in static fields
    ++staticTileObjectCount;
  }
}

Tile::~Tile() {
  if(textureId != GL_INVALID_VALUE) {
    glDeleteTextures(1, &textureId);
    textureId = GL_INVALID_VALUE;
  }
  if(staticTileObjectCount == 1 && programObject != GL_INVALID_VALUE) {
    glDeleteProgram(programObject);
    programObject = GL_INVALID_VALUE;

    tileSizeLoc     = GL_INVALID_VALUE;
    tilePositionLoc = GL_INVALID_VALUE;
    frameColorLoc   = GL_INVALID_VALUE;
    frameWidthLoc   = GL_INVALID_VALUE;
    samplerLoc      = GL_INVALID_VALUE;
    posLoc          = GL_INVALID_VALUE;
    texLoc          = GL_INVALID_VALUE;
    opacityLoc      = GL_INVALID_VALUE;
  }

  --staticTileObjectCount;
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

void Tile::render() {

  if(textureId == GL_INVALID_VALUE)
    return;

  animation.update(position, zoom, size, opacity);

  float leftPx = position.first - (size.first / 2.0) * (zoom - 1.0);
  float rightPx = (position.first + size.first) + (size.first / 2.0) * (zoom - 1.0);
  float downPx = position.second - (size.second / 2.0) * (zoom - 1.0);
  float topPx = (position.second + size.second) + (size.second / 2.0) * (zoom - 1.0);
  float left = (leftPx / Settings::viewport.first) * 2.0 - 1.0;
  float right = (rightPx / Settings::viewport.first) * 2.0 - 1.0;
  float down = (downPx / Settings::viewport.second) * 2.0 - 1.0;
  float top = (topPx / Settings::viewport.second) * 2.0 - 1.0;

  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
  float texCoord[] = { 0.0f, 0.0f,    0.0f, 1.0f,
                       1.0f, 1.0f,    1.0f, 0.0f };

  glUseProgram(programObject);

  glUniform2f(tileSizeLoc, static_cast<float>(rightPx - leftPx), static_cast<float>(topPx - downPx));
  glUniform2f(tilePositionLoc, static_cast<float>(leftPx), static_cast<float>(downPx));
  glUniform4f(frameColorLoc, 1.0, 1.0, 1.0, opacity);
  float frameWidth = std::max(0.0, (zoom - 1.0) > 0.01 ? (zoom - 1.0) * 50.0 : 0.0);
  glUniform1f(frameWidthLoc, frameWidth);
  glUniform1f(opacityLoc, static_cast<GLfloat>(opacity));

  glBindTexture(GL_TEXTURE_2D, textureId);
  glUniform1i(samplerLoc, 0);

  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
  glEnableVertexAttribArray(texLoc);
  glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoord);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(posLoc);
  glDisableVertexAttribArray(texLoc);
  glBindTexture(GL_TEXTURE_2D, GL_INVALID_VALUE);
  glUseProgram(GL_INVALID_VALUE);
}

void Tile::setTexture(char *pixels, std::pair<int, int> size, GLuint format) {
  if(textureId == GL_INVALID_VALUE)
    initTexture();

  textureFormat = format;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, format, size.first, size.second, 0, format, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, GL_INVALID_VALUE);
}

