#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_

#include <string>
#include <chrono>
#include <utility>

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

#include "Tile.h"
#include "Text.h"
#include "log.h"
#include "Animation.h"

class Background {
private:
  GLuint programObject = GL_INVALID_VALUE;
  GLuint textureFormat = GL_INVALID_VALUE;
  float opacity;
  float black;
  Tile *sourceTile;
  std::vector<float> clearColor;
  Animation animation;

  GLuint samplerLoc  = GL_INVALID_VALUE;
  GLint posLoc       = GL_INVALID_VALUE;
  GLint texLoc       = GL_INVALID_VALUE;
  GLuint opacityLoc  = GL_INVALID_VALUE;
  GLuint blackLoc    = GL_INVALID_VALUE;
  GLuint viewportLoc = GL_INVALID_VALUE;

  void initGL();
  void renderText();

public:
  Background();
  Background(float opacity);
  ~Background();
  void render();
  void setOpacity(float opacity);
  void setBlack(float black);
  void setSourceTile(Tile *sourceTile, std::chrono::milliseconds duration, std::chrono::milliseconds delay);
  void setClearColor(std::vector<float> color);
  float getOpacity();
  float getBlack();
};

#endif // _BACKGRODUN_H_
