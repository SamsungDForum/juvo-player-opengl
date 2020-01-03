#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_

#include <vector>

#include "GLES.h"
#include "Tile.h"
#include "Animation.h"

class Background {
private:
  GLuint programObject = GL_INVALID_VALUE;
  GLuint textureFormat = GL_INVALID_VALUE;
  float opacity;
  float mixing;
  Tile *lastTile, *currentTile, *queuedTile;
  Animation animation;

  GLuint samplerLoc  = GL_INVALID_VALUE;
  GLuint sampler2Loc  = GL_INVALID_VALUE;
  GLint posLoc       = GL_INVALID_VALUE;
  GLint texLoc       = GL_INVALID_VALUE;
  GLuint opacityLoc  = GL_INVALID_VALUE;
  GLuint mixingLoc    = GL_INVALID_VALUE;
  GLuint viewportLoc = GL_INVALID_VALUE;

  void initGL();
  void renderNameAndDescription();
  void runBackgroundChangeAnimation();
  void endAnimation();

public:
  Background();
  Background(float opacity);
  ~Background();
  void render();
  void setOpacity(float opacity);
  void setSourceTile(Tile *tile);
  void setClearColor(std::vector<float> color);
  float getOpacity();
};

#endif // _BACKGROUND_H_
