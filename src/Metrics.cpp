#include "Metrics.h"

Metrics::Metrics(std::pair<int, int> viewport)
  : viewport(viewport) {
    traces.push_back(std::make_unique<Framerate>());
}

void Metrics::render(Text &text) {
  for(int i = 0, rendered = 0; i < static_cast<int>(traces.size()); ++i) {
    if(Framerate *framerate = dynamic_cast<Framerate*>(traces[i].get()))
      framerate->step();

    if(!traces[i]->visible)
      continue;

    std::pair<int, int> margin = {4, 10};
    std::pair<int, int> size = {600, 50};
    std::pair<int, int> position = {viewport.first - (size.first + margin.first),
                                    viewport.second - (size.second + margin.second) * (rendered + 1)};
    graph.render({traces[i]->values.begin(), traces[i]->values.end()},
                 {traces[i]->minValue, traces[i]->maxValue},
                 position,
                 size,
                 viewport);

    int fontHeight = 26;
    std::pair<int, int> textMargin = {margin.first + 180, margin.second + size.second};
    text.render(traces[i]->tag + std::string(": ") + std::to_string(static_cast<int>(traces[i]->currentValue)),
                {viewport.first - textMargin.first, viewport.second - textMargin.second},
                {0, fontHeight},
                viewport,
                0,
                {1.0, 1.0, 1.0, 1.0},
                true);
  }
}

int Metrics::addGraph(std::string tag, int minVal, int maxVal, int valuesMaxCount) {
  int id = static_cast<int>(traces.size());
  traces.push_back(std::make_unique<Trace>(id, tag, minVal, maxVal, valuesMaxCount));
  return id;
}

void Metrics::setGraphVisibility(int graphId, bool visible) {
  if(graphId < 0 || graphId > static_cast<int>(traces.size()))
      return;
  traces[graphId]->visible = visible;
}

void Metrics::updateGraphValues(int graphId, std::vector<int> values) {
  if(graphId < 1 || graphId > static_cast<int>(traces.size()))
      return;
  traces[graphId]->values.clear();
  traces[graphId]->values.insert(traces[graphId]->values.begin(), values.begin(), values.end());
}

void Metrics::updateGraphValue(int graphId, int value) {
  if(graphId < 1 || graphId > static_cast<int>(traces.size()))
      return;
  traces[graphId]->currentValue = value;
  traces[graphId]->values.push_back(value);
  while(static_cast<int>(traces[graphId]->values.size()) > traces[graphId]->valueMaxCount)
    traces[graphId]->values.pop_front();
}

Metrics::Trace::Trace(int id, std::string tag, int minValue, int maxValue, int valueMaxCount)
  : id(id),
    tag(tag),
    currentValue(minValue),
    minValue(minValue),
    maxValue(maxValue),
    valueMaxCount(valueMaxCount),
    visible(false) {
}

Metrics::Framerate::Framerate()
  : Trace(0, "FPS", 0, 60, 100),
    fpsTime(std::chrono::high_resolution_clock::now()),
    currentFps(0) {
}

void Metrics::Framerate::step() {
  std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> timespan = now - fpsTime;
  fpsTime = now;
  float currentFps = 1000.0f / timespan.count();
  fpsSum += currentFps;
  values.push_back(currentFps);
  while(static_cast<int>(values.size()) > valueMaxCount) {
    fpsSum -= values.front();
    values.pop_front();
  }
  currentValue = fpsSum / (values.size() ? : 1);
}


