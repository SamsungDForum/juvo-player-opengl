#ifndef _TILE_H_
#define _TILE_H_

#include <string>
#include <chrono>
#include <utility>

#include "GLES.h"
#include "TileAnimation.h"
#include "CommonStructs.h"
#include "ExternStructs.h"

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

  bool runningPreview;
  std::chrono::time_point<std::chrono::high_resolution_clock> storyboardPreviewStartTimePoint;
  Rect storytileRect;
  bool previewReady;
  GLuint previewTextureId = GL_INVALID_VALUE;
  SubBitmapExtern storyboardBitmap;
  int bitmapHash;
  StoryboardExternData (*getStoryboardDataCallback)(long long position, int tileId);

  GLuint textureId = GL_INVALID_VALUE;
  GLuint textureFormat = GL_INVALID_VALUE;

  static int staticTileObjectCount;
  static GLuint programObject;
  static GLuint tileSizeLoc;
  static GLuint tilePositionLoc;
  static GLuint samplerLoc;
  static GLuint posLoc;
  static GLuint texLoc;
  static GLuint opacityLoc;
  static GLuint viewportLoc;
  static GLuint scaleLoc;
  static GLuint storytileRectLoc;

  void initTextures();
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
  void runPreview(bool run);
  StoryboardExternData getStoryboardData(std::chrono::duration<double> position, int tileId);
  GLuint getCurrentTextureId();
  void setPreviewTexture(SubBitmapExtern frame);
  void setStoryboardCallback(StoryboardExternData (*getStoryboardDataCallback)(long long position, int tileId));


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
