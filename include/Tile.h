#ifndef _TILE_H_
#define _TILE_H_

#include <string>
#include <chrono>
#include <utility>

#include "GLES.h"
#include "TileAnimation.h"
#include "CommonStructs.h"
#include "ExternStructs.h"
#include "Utility.h"

class Tile {
private:
  int id;
  Position<int> position;
  Size<int> size;
  float zoom;
  float opacity;
  std::string name;
  std::string description;
  bool active;

  TileAnimation animation;

  bool runningPreview;
  std::chrono::time_point<std::chrono::steady_clock> storyboardPreviewStartTimePoint;
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
  Tile(int tileId, Position<int> position, Size<int> size, float zoom, float opacity, std::string name, std::string description, char *texturePixels, Size<int> textureSize, GLuint textureFormat);
  Tile(int tileId, Position<int> position, Size<int> size, float zoom, float opacity, std::string name, std::string description);
  Tile(int tileId);
  ~Tile();
  Tile(Tile &) = delete; // no copy constructor
  Tile(const Tile &) = delete; // no copy constructor
  Tile& operator=(Tile&&) = delete;
  Tile(Tile &&other);

  void render();
  void renderName();
  void setTexture(char *pixels, Size<int> size, GLuint format);
  void moveTo(Position<int> position, float zoom, Size<int> size, float opacity, std::chrono::milliseconds moveDuration, std::chrono::milliseconds animationDuration, std::chrono::milliseconds delay);
  void runPreview(bool run);
  StoryboardExternData getStoryboardData(std::chrono::milliseconds position, int tileId);
  GLuint getCurrentTextureId();
  void setPreviewTexture(SubBitmapExtern frame);
  void setStoryboardCallback(StoryboardExternData (*getStoryboardDataCallback)(long long position, int tileId));


  void setId(int id) { this->id = id; }
  int  getId() { return id; }
  void setSize(int width, int height) { size.width = width; size.height = height; }
  void setSize(const Size<int> &size) { this->size = size; }
  Size<int> getSize() { return size; }
  int getWidth() { return size.width; }
  int getHeight() { return size.height; }
  void setPosition(int x, int y) { position.x = x; position.y = y; }
  void setPosition(const Position<int> &position) { this->position = position; }
  int getX() { return position.x; }
  int getY() { return position.y; }
  Position<int> getPosition() { return position; }
  void setName(const std::string &name) { this->name = name; }
  std::string getName() { return name; }
  std::string getDescription() { return description; }
  void setDescription(const std::string &description) { this->description = description; }
  int getTextureId() { return textureId; }
  void setZoom(float zoom) { this->zoom = zoom; }
  void setOpacity(float opacity) { this->opacity = opacity; }
  float getZoom() { return zoom; }
  float getOpacity() { return opacity; }

  Position<int> getSourcePosition() { return animation.isActive() ? animation.getSourcePosition() : getPosition(); }
  Position<int> getTargetPosition() { return animation.isActive() ? animation.getTargetPosition() : getPosition(); }
  float getSourceZoom() { return animation.isActive() ? animation.getSourceZoom() : getZoom(); }
  float getTargetZoom() { return animation.isActive() ? animation.getTargetZoom() : getZoom(); }
  Size<int> getSourceSize() { return animation.isActive() ? animation.getSourceSize() : getSize(); }
  Size<int> getTargetSize() { return animation.isActive() ? animation.getTargetSize() : getSize(); }
  float getSourceOpacity() { return animation.isActive() ? animation.getSourceOpacity() : getOpacity(); }
  float getTargetOpacity() { return animation.isActive() ? animation.getTargetOpacity() : getOpacity(); }
  bool isAnimationActive() { return animation.isActive(); }
  void setActive(bool value) { active = value; }
  bool isActive() { return active; }
};

#endif // _TILE_H_
