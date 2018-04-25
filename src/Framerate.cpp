#include "Framerate.h"

Framerate::Framerate(std::pair<int, int> viewport)
  : fpsS(0),
    fpsN(100),
    fpsT(std::chrono::high_resolution_clock::now()),
    fps(0),
    viewport(viewport) {
}

void Framerate::step() {
  std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> timespan = now - fpsT;
  fpsT = now;
  fps = 1000.0f / timespan.count();
  fpsS += fps;
  fpsV.push_back(fps);
  while(static_cast<int>(fpsV.size()) > fpsN) {
    fpsS -= fpsV.front();
    fpsV.pop_front();
  }
  fps = fpsS / (fpsV.size() ? : 1);
}

void Framerate::render(Text &text) {
  std::pair<int, int> margin = {4, 10};
  std::pair<int, int> size = {600, 50};
  std::pair<int, int> position = {viewport.first - size.first - margin.first,
                                  viewport.second - size.second - margin.second};
  graph.render({fpsV.begin(), fpsV.end()},
               {0.0f, 60.0f},
               position,
               size,
               viewport);

  int fontHeight = 26;
  std::pair<int, int> textMargin = {margin.first + 180, margin.second + size.second};
  text.render(std::string("FPS: ") + std::to_string(static_cast<int>(fps)),
              {viewport.first - textMargin.first, viewport.second - textMargin.second},
              {0, fontHeight},
              viewport,
              0,
              {1.0, 1.0, 1.0, 1.0},
              true);
}

