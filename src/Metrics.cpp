#include "Metrics.h"
#include "Settings.h"
#include "Text.h"

Metrics::Metrics()
  : logConsoleVisible(false) {
    traces.push_back(std::make_unique<Framerate>());
}

void Metrics::render() {
  std::pair<int, int> margin = {4, 10};
  std::pair<int, int> size = {600, 50};
  int rendered = 0;

  for(int i = 0; i < static_cast<int>(traces.size()); ++i) {
    if(Framerate *framerate = dynamic_cast<Framerate*>(traces[i].get()))
      framerate->step();

    if(!traces[i]->visible)
      continue;

    std::pair<int, int> position = {Settings::instance().viewport.first - (size.first + margin.first),
                                    Settings::instance().viewport.second - (size.second + margin.second) * (rendered + 1)};
    graph.render({traces[i]->values.begin(), traces[i]->values.end()},
                 {traces[i]->minValue, traces[i]->maxValue},
                 position,
                 size);

    int fontHeight = 26;
    std::pair<int, int> textMargin = {size.first, margin.second + size.second};
    Text::instance().render(traces[i]->tag + std::string(": ") + std::to_string(static_cast<int>(traces[i]->currentValue)) + std::string("/") + std::to_string(static_cast<int>(traces[i]->maxValue)),
                {Settings::instance().viewport.first - textMargin.first, Settings::instance().viewport.second - textMargin.second - (size.second + margin.second) * rendered},
                {0, fontHeight},
                0,
                {1.0, 1.0, 1.0, 1.0},
                true);
    ++rendered;
  }

  if(logConsoleVisible) {
    int bottomMargin = margin.second * 3;
    std::pair<int, int> position = {Settings::instance().viewport.first - (size.first + margin.first),
                                    bottomMargin};
    std::pair<int, int> size2 = {size.first,
                                 Settings::instance().viewport.second - margin.second - bottomMargin - (size.second + margin.second) * rendered};
    logConsole.render(position, size2, 0, 13);
  }
}

int Metrics::addGraph(std::string tag, float minVal, float maxVal, int valuesMaxCount) {
  int id = static_cast<int>(traces.size());
  traces.push_back(std::make_unique<Trace>(id, tag, minVal, maxVal, valuesMaxCount));
  return id;
}

void Metrics::setGraphVisibility(int graphId, bool visible) {
  if(graphId < 0 || graphId > static_cast<int>(traces.size()))
      return;
  traces[graphId]->visible = visible;
}

void Metrics::updateGraphValues(int graphId, std::vector<float> values) {
  if(graphId < 1 || graphId > static_cast<int>(traces.size()))
      return;
  traces[graphId]->values.clear();
  traces[graphId]->values.insert(traces[graphId]->values.begin(), values.begin(), values.end());
}

void Metrics::updateGraphValue(int graphId, float value) {
  if(graphId < 1 || graphId > static_cast<int>(traces.size()))
      return;
  traces[graphId]->values.push_back(value);
  while(static_cast<int>(traces[graphId]->values.size()) > traces[graphId]->valueMaxCount)
    traces[graphId]->values.pop_front();
  traces[graphId]->currentValue = value;
}

Metrics::Trace::Trace(int id, std::string tag, float minValue, float maxValue, int valueMaxCount)
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
    fpsSum(0),
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


void Metrics::updateGraphRange(int graphId, float minVal, float maxVal) {
  if(graphId < 1 || graphId > static_cast<int>(traces.size()))
      return;
  traces[graphId]->minValue = minVal;
  traces[graphId]->maxValue = maxVal;
}

void Metrics::setLogConsoleVisibility(bool visible) {
  logConsoleVisible = visible;
}

void Metrics::pushLog(std::string log) {
  logConsole.pushLog(log);
}

