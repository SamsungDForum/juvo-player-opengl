#ifndef _FRAMERATE_H_
#define _FRAMERATE_H_

#include <deque>
#include <chrono>
#include <vector>
#include <string>

#include "Text.h"
#include "Graph.h"

class Framerate {
private:
  float fpsS;
  int fpsN;
  std::chrono::time_point<std::chrono::high_resolution_clock> fpsT;
  std::deque<float> fpsV;
  float fps;
  const std::pair<int, int> viewport;
  Graph graph;

public:
  Framerate(std::pair<int, int> viewport);
  void step();
  void render(Text &text);
};

#endif // _FRAMERATE_H_
