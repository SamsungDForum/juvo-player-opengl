#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <cmath>
#include <vector>
#include <string>

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

#include "log.h"

class Graph {
private:
  GLuint programObject;
  bool initialize();
  inline float clamp(const float v, const float lo, const float hi) { return v < lo ? lo : v > hi ? hi : v; }
  
  const int VALUES = 100;

  GLuint posALoc;
  GLuint posLoc;
  GLuint sizLoc;
  GLuint valLoc;
  GLuint colLoc;
  GLuint opaLoc;

public:
  Graph();
  ~Graph();
  void render(const std::vector<float> &values, const std::pair<float, float> &minMax, const std::pair<int, int> &position, const std::pair<int, int> &size, const std::pair<int, int> &viewport);
  void checkShaderCompileError(GLuint shader);
};

#endif // _GRAPH_H_
