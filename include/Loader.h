#ifndef _LOADER_H_
#define _LOADER_H_

#include <chrono>
#include <cmath>
#include <vector>

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

#include "Text.h"
#include "Animation.h"
#include "log.h"

class Loader {
private:
  GLuint programObject;
  std::chrono::time_point<std::chrono::high_resolution_clock> time;
  bool initialize();
  int param;
  Animation animation;

public:
  Loader();
  ~Loader();
  void render(Text &text);
  void setValue(int value);
  void checkShaderCompileError(GLuint shader);
};

#endif // _LOADER_H_
