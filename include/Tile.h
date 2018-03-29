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
#include "Text.h"
#include "log.h"

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

  /*GLuint tileSizeLoc     = GL_INVALID_VALUE;
  GLuint tilePositionLoc = GL_INVALID_VALUE;
  GLuint frameColorLoc   = GL_INVALID_VALUE;
  GLuint frameWidthLoc   = GL_INVALID_VALUE;
  GLuint samplerLoc      = GL_INVALID_VALUE;
  GLint posLoc           = GL_INVALID_VALUE;
  GLint texLoc           = GL_INVALID_VALUE;
  GLuint opacityLoc      = GL_INVALID_VALUE;*/

  void initTexture();
  void initGL();
  void checkShaderCompileError(GLuint shader);

public:
  Tile(int tileId, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, float zoom, float opacity, std::string name, std::string description, char *texturePixels, std::pair<int, int> textureSize, GLuint textureFormat);
  Tile(int tileId, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, float zoom, float opacity, std::string name, std::string description);
  Tile(int tileId);
  ~Tile();
  Tile(Tile &) = delete; // no copy constructor
  Tile(const Tile &) = delete; // no copy constructor
  //Tile(Tile &&) = default; // default move constructor
  Tile& operator=(Tile&&) = delete;
  Tile(Tile &&other);

  void render(Text &text);
  void setTexture(char *pixels, std::pair<int, int> size, GLuint format);
  void moveTo(std::pair<int, int> position, float zoom, std::pair<int, int> size, float opacity, std::chrono::milliseconds duration, std::chrono::milliseconds delay, int bounce = 0);

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
  std::string getName() { return name; }
  std::string getDescription() { return description; }
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

#endif // _TILE_H_
