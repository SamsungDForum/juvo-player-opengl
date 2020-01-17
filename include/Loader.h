#ifndef _LOADER_H_
#define _LOADER_H_

#include <chrono>

#include "GLES.h"
#include "Animation.h"
#include "Utility.h"

class Loader {
private:
  void initialize();
  int percent;
  Animation animation;

  GLuint programObject = GL_INVALID_VALUE;
  GLuint percentLoc  = GL_INVALID_VALUE;
  GLuint viewportLoc = GL_INVALID_VALUE;
  GLuint positionLoc = GL_INVALID_VALUE;
  GLuint posLoc      = GL_INVALID_VALUE;
  GLuint sizLoc      = GL_INVALID_VALUE;
  GLuint fgColorLoc  = GL_INVALID_VALUE;
  GLuint bgColorLoc  = GL_INVALID_VALUE;

  GLuint logoProgramObject = GL_INVALID_VALUE;
  GLuint logoPosLoc = GL_INVALID_VALUE;
  GLuint logoSamplerLoc = GL_INVALID_VALUE;
  GLuint logoTexLoc = GL_INVALID_VALUE;

  GLuint logoTextureId = 0;

  Size<int> progressBarSize;
  Size<int> logoMaxSize;
  Size<int> logoSize;
  Position<int> logoPosition;
  Position<int> progressBarPosition;
  int verticalMargin;
  std::vector<float> backgroundColor;

public:
  Loader();
  ~Loader();
  Loader(const Loader& other) = delete;
  Loader(Loader&& other) = delete;
  Loader& operator=(Loader&& other) = delete;
  void render();
  void setValue(int value);
  void setLogo(int id, char* pixels, Size<int> size, GLuint format);
  void initTexture();
  void recalculateSizesAndPositions(Size<int> bitmapSize);
  void renderLogo(Size<int> size, Position<int> position);
  void renderProgressBar(Size<int> size, Position<int> position, float percent);
  void updatePercent();
};

#endif // _LOADER_H_
