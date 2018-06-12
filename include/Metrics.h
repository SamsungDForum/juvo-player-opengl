#ifndef _FRAMERATE_H_
#define _FRAMERATE_H_

#include <deque>
#include <chrono>
#include <vector>
#include <string>
#include <memory>

#include "Text.h"
#include "Graph.h"
#include "LogConsole.h"
#include "log.h"

class Metrics {
private:
  class Trace {
  public:
    int id;
    std::string tag;
    float currentValue;
    float minValue;
    float maxValue;
    int valueMaxCount;
    std::deque<float> values;
    bool visible;
    Trace(int id, std::string tag, float minValue, float maxValue, int valueMaxCount);
    virtual ~Trace() = default; // make it polymorphic
  };

  class Framerate : public Trace {
  public:
    float fpsSum;
    std::chrono::time_point<std::chrono::high_resolution_clock> fpsTime;
    float currentFps;
    Framerate();
    void step();
  };
  const int framerateId = 0;

  const std::pair<int, int> viewport;
  Graph graph;
  LogConsole logConsole;
  bool logConsoleVisible;
  
  std::vector<std::unique_ptr<Trace>> traces;

public:
  Metrics(std::pair<int, int> viewport);
  void render(Text &text);
  int addGraph(std::string tag, float minVal, float maxVal, int valuesMaxCount);
  void setGraphVisibility(int graphId, bool visible);
  void updateGraphValues(int graphId, std::vector<float> values);
  void updateGraphValue(int graphId, float value);
  void updateGraphRange(int graphId, float minVal, float maxVal);

  void setLogConsoleVisibility(bool visible);
  void pushLog(std::string log);
};

#endif // _FRAMERATE_H_
