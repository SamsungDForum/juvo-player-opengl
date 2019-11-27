#ifndef _TILE_H_
#define _TILE_H_

#include <string>
#include <chrono>
#include <utility>

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

#include "TileAnimation.h"

class Tile {
private:
  int id;
  std::pair<int, int> position;
  std::pair<int, int> size;
  float zoom;
  float opacity;
  std::string name;
  std::string description;

  TileAnimation animation;
  GLuint textureId = GL_INVALID_VALUE;
  GLuint textureFormat = GL_INVALID_VALUE;

  static int staticTileObjectCount;
  static GLuint programObject;
  static GLuint tileSizeLoc;
  static GLuint tilePositionLoc;
  static GLuint frameColorLoc;
  static GLuint frameWidthLoc;
  static GLuint samplerLoc;
  static GLuint posLoc;
  static GLuint texLoc;
  static GLuint opacityLoc;

  void initTexture();
  void initGL();

public:
  Tile(int tileId, std::pair<int, int> position, std::pair<int, int> size, float zoom, float opacity, std::string name, std::string description, char *texturePixels, std::pair<int, int> textureSize, GLuint textureFormat);
  Tile(int tileId, std::pair<int, int> position, std::pair<int, int> size, float zoom, float opacity, std::string name, std::string description);
  Tile(int tileId);
  ~Tile();
  Tile(Tile &) = delete; // no copy constructor
  Tile(const Tile &) = delete; // no copy constructor
  Tile& operator=(Tile&&) = delete;
  Tile(Tile &&other);

  void render();
  void setTexture(char *pixels, std::pair<int, int> size, GLuint format);
  void moveTo(std::pair<int, int> position, float zoom, std::pair<int, int> size, float opacity, std::chrono::milliseconds duration, std::chrono::milliseconds delay, int bounce = 0);

  void setId(int id) { this->id = id; }
  int  getId() { return id; }
  void setSize(int width, int height) { size.first = width; size.second = height; }
  void setSize(const std::pair<int, int> &size) { this->size = size; }
  std::pair<int, int> getSize() { return size; }
  int getWidth() { return size.first; }
  int getHeight() { return size.second; }
  void setPosition(int x, int y) { position.first = x; position.second = y; }
  void setPosition(const std::pair<int, int> &position) { this->position = position; }
  int getX() { return position.first; }
  int getY() { return position.second; }
  std::pair<int, int> getPosition() { return position; }
  void setName(const std::string &name) { this->name = name; }
  std::string getName() { return name; }
  std::string getDescription() { return description; }
  void setDescription(const std::string &description) { this->description = description; }
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
