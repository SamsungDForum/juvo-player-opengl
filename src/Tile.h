#ifndef _TILE_H_
#define _TILE_H_

#include <string>
#include <chrono>

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

#include "TileAnimation.h"

class Tile {
  private:
    int id;
    GLuint programObject = GL_INVALID_VALUE;
    GLuint textureFormat = GL_INVALID_VALUE;
    int x;
    int y;
    int width;
    int height;
    int viewportWidth;
    int viewportHeight;
    float zoom;
    float opacity;
    std::string name;
    std::string description;

    TileAnimation animation;
    GLuint textureId = GL_INVALID_VALUE;
/*    GLuint samplerLoc;
    GLint posLoc;
    GLint texLoc;
    GLuint opacityLoc;*/

    void initTexture();

  public:
    Tile(int tileId, GLuint programObject, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, float zoom, float opacity, std::string name, std::string description, char *texturePixels, std::pair<int, int> textureSize, GLuint textureFormat);
    Tile(int tileId, GLuint programObject, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, float zoom, float opacity, std::string name, std::string description);
    Tile(int tileId, GLuint programObject);
    ~Tile();
    Tile(Tile &) = delete; // no copy constructor
    Tile(const Tile &) = delete; // no copy constructor
    //Tile(Tile &&) = default; // default move constructor
    Tile& operator=(Tile&&) = delete;
    Tile(Tile &&other);

    void render();
    void setTexture(char *pixels, int width, int heigth, GLuint format);
    void moveTo(std::pair<int, int> position, float zoom, std::pair<int, int> size, float opacity, std::chrono::milliseconds duration, int bounce = 0);

    void setId(int id) { this->id = id; }
    int  getId() { return id; }
    void setSize(int width, int height) { this->width = width; this->height = height; }
    void setSize(const std::pair<int, int> &size) { width = size.first; height = size.second; }
    std::pair<int, int> getSize() { return std::make_pair(width, height); }
    int getWidth() { return width; }
    int getHeight() { return height; }
    void setPosition(int x, int y) { this->x = x; this->y = y; }
    void setPosition(const std::pair<int, int> &position) { x = position.first; y = position.second; }
    int getX() { return x; }
    int getY() { return y; }
    std::pair<int, int> getPosition() { return std::make_pair(x, y); }
    void setName(const std::string &name) { this->name = name; }
    void setDescription(const std::string &description) { this->description = description; }
    void setViewportSize(const std::pair<int, int> &viewportSize) { viewportWidth = viewportSize.first; viewportHeight = viewportSize.second; }
    void setViewportSize(int viewportWidth, int viewportHeight) { this->viewportWidth = viewportWidth; this->viewportHeight = viewportHeight; }
    int getTextureId() { return textureId; }
    void setZoom(float zoom) { this->zoom = zoom; }
    void setOpacity(float opacity) { this->opacity = opacity; }
    float getZoom() { return zoom; }
    float getOpacity() { return opacity; }

    std::pair<int, int> getSourcePosition() { return animation.isActive() ? animation.getSourcePosition() : getPosition(); }
    std::pair<int, int> getTargetPosition() { return animation.isActive() ? animation.getTargetPosition() : getPosition(); }
    float getSourceZoom() { return animation.isActive() ? animation.getSourceZoom() : getZoom(); }
    float getTargetZoom() { return animation.isActive() ? animation.getTargetZoom() : getZoom(); }
    std::pair<int, int> getSourceSize() { return animation.isActive() ? animation.getSourceSize() : getSize(); }
    std::pair<int, int> getTargetSize() { return animation.isActive() ? animation.getTargetSize() : getSize(); }
    float getSourceOpacity() { return animation.isActive() ? animation.getSourceOpacity() : getOpacity(); }
    float getTargetOpacity() { return animation.isActive() ? animation.getTargetOpacity() : getOpacity(); }
    bool isAnimationActive() { return animation.isActive(); }

};

Tile::Tile(int tileId, GLuint programObject, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, float zoom, float opacity, std::string name, std::string description, char *texturePixels, std::pair<int, int> textureSize, GLuint textureFormat)
          : id(tileId),
            programObject(programObject),
            x(position.first),
            y(position.second),
            width(size.first),
            height(size.second),
            viewportWidth(viewport.first),
            viewportHeight(viewport.second),
            zoom(zoom),
            opacity(opacity),
            name(name),
            description(description),
            textureId(GL_INVALID_VALUE) {
  initTexture();
  setTexture(texturePixels, textureSize.first, textureSize.second, textureFormat);
}

Tile::Tile(int tileId, GLuint programObject, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, float zoom, float opacity, std::string name, std::string description)
          : id(tileId),
            programObject(programObject),
            x(position.first),
            y(position.second),
            width(size.first),
            height(size.second),
            viewportWidth(viewport.first),
            viewportHeight(viewport.second),
            zoom(zoom),
            opacity(opacity),
            name(name),
            description(description),
            textureId(GL_INVALID_VALUE) {
}

Tile::Tile(int tileId, GLuint programObject)
          : id(tileId),
          programObject(programObject),
          textureId(GL_INVALID_VALUE) {
  initTexture();
}

void Tile::initTexture() {
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &textureId);
/*  glUseProgram(programObject);
  samplerLoc = glGetUniformLocation(programObject, "s_texture");
  posLoc = glGetAttribLocation(programObject, "a_position");
  texLoc = glGetAttribLocation(programObject, "a_texCoord");
  opacityLoc = glGetUniformLocation(programObject, "u_opacity"); */
}

Tile::Tile(Tile &&other) {
  if(this != &other) {
    id = other.id;
    width = other.width;
    height = other.height;
    x = other.x;
    y = other.y;
    viewportWidth = other.viewportWidth;
    viewportHeight = other.viewportHeight;
    textureId = other.textureId;
    textureFormat = other.textureFormat;
    programObject = other.programObject;
    name = other.name;
    description = other.description;
    zoom = other.zoom;
    opacity = other.opacity;
    animation = other.animation;

    other.textureId = GL_INVALID_VALUE; // prevent destructor of the object we moved from from deleting the texture
  }
}

Tile::~Tile() {
  if(textureId != GL_INVALID_VALUE)
    glDeleteTextures(1, &textureId);
}

void Tile::moveTo(std::pair<int, int> position, float zoom, std::pair<int, int> size, float opacity, std::chrono::milliseconds duration, int bounce) {
  TileAnimation::Easing positionEasing = animation.isActive() ? TileAnimation::Easing::CubicOut : TileAnimation::Easing::CubicInOut;

  if(bounce) {
    if(animation.isActive() && animation.isBounce())
      return;
    positionEasing = bounce < 0 ? TileAnimation::Easing::BounceLeft : TileAnimation::Easing::BounceRight;
  }

  TileAnimation::Easing zoomEasing = animation.isActive() ? TileAnimation::Easing::CubicOut : TileAnimation::Easing::CubicInOut;
  TileAnimation::Easing sizeEasing = animation.isActive() ? TileAnimation::Easing::CubicOut : TileAnimation::Easing::CubicInOut;
  TileAnimation::Easing opacityEasing = animation.isActive() ? TileAnimation::Easing::CubicOut : TileAnimation::Easing::CubicInOut;
  animation = TileAnimation(std::chrono::high_resolution_clock::now(),
                            std::chrono::milliseconds(duration),
                            {x, y},
                            position,
                            positionEasing,
                            this->zoom,
                            zoom,
                            zoomEasing,
                            {width, height},
                            size,
                            sizeEasing,
                            this->opacity,
                            opacity,
                            opacityEasing);
  x = position.first;
  y = position.second;
}

void Tile::render() {

  if(textureId == GL_INVALID_VALUE)
    return;

  std::pair<int, int> position {x, y};
  std::pair<int, int> size {width, height};
  animation.update(position, zoom, size, opacity);
  x = position.first;
  y = position.second;
  width = size.first;
  height = size.second;

  float vW = viewportWidth;
  float vH = viewportHeight;
  float w = width;
  float h = height;
  float xPos = position.first;
  float yPos = position.second;
  float left = xPos / vW * 2 - 1;
  float down = yPos / vH * 2 - 1;
  float right = (xPos + w) / vW * 2 - 1;
  float top = (yPos + h) / vH * 2 - 1;

  //GLint colLoc = glGetUniformLocation(programObject, "u_color");
  //glUniform4f(colLoc, 0.0f, 0.0f, 0.0f, 1.0f);

  left -= (w / vW) * (zoom - 1.0);
  right += (w / vW) * (zoom - 1.0);
  down -= (h / vH) * (zoom - 1.0);
  top += (h / vH) * (zoom - 1.0);
  

  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 }; // TODO: Move it to VBO... Or somewhene in GPU's memory.

  glUseProgram(programObject);

  glBindTexture(GL_TEXTURE_2D, textureId);
  GLuint samplerLoc = glGetUniformLocation(programObject, "s_texture"); // TODO: Store the location somewhere.
  glUniform1i(samplerLoc, 0);

  GLint posLoc = glGetAttribLocation(programObject, "a_position");
  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  float texCoord[] = { 0.0f, 0.0f,    0.0f, 1.0f, // TODO: Move it to VBO... Or somewhene in GPU's memory.
                       1.0f, 1.0f,    1.0f, 0.0f };
                       
  GLint texLoc = glGetAttribLocation(programObject, "a_texCoord"); // TODO: Store the location somewhere.
  glEnableVertexAttribArray(texLoc);
  glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoord);

  GLuint opacityLoc = glGetUniformLocation(programObject, "u_opacity"); // TODO: Store the location somewhere.
  glUniform1f(opacityLoc, static_cast<GLfloat>(opacity));
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

void Tile::setTexture(char *pixels, int width, int height, GLuint format) {
  if(textureId == GL_INVALID_VALUE)
    initTexture();
  textureFormat = format;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR/*GL_NEAREST*/);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR/*GL_NEAREST*/);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFlush();
}

#endif // _TILE_H_
