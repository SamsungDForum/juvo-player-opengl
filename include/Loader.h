#ifndef _LOADER_H_
#define _LOADER_H_

#include <chrono>

#include "GLES.h"
#include "Animation.h"

class Loader {
private:
  GLuint programObject;
  std::chrono::time_point<std::chrono::high_resolution_clock> time;
  void initialize();
  int param;
  Animation animation;

  GLuint posLoc     = GL_INVALID_VALUE;
  GLuint timeLoc    = GL_INVALID_VALUE;
  GLuint paramLoc   = GL_INVALID_VALUE;
  GLuint opacityLoc = GL_INVALID_VALUE;
  GLuint viewportLoc = GL_INVALID_VALUE;

  void renderText(float time);

public:
  Loader();
  ~Loader();
  void render();
  void setValue(int value);
};

#endif // _LOADER_H_
