#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <vector>
#include <string>
#include <utility>

#include "GLES.h"

class Graph {
private:
  GLuint programObject;
  GLuint posALoc;
  GLuint posLoc;
  GLuint sizLoc;
  GLuint valLoc;
  GLuint colLoc;
  GLuint opaLoc;

  const int VALUES = 100;

  void initialize();
  inline float clamp(const float v, const float lo, const float hi) { return v < lo ? lo : v > hi ? hi : v; }

public:
  Graph();
  ~Graph();
  void render(const std::vector<float> &values, const std::pair<float, float> &minMax, const std::pair<int, int> &position, const std::pair<int, int> &size);
};

#endif // _GRAPH_H_
