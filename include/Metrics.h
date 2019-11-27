#ifndef _METRICS_H_
#define _METRICS_H_

#include <deque>
#include <chrono>
#include <vector>
#include <string>
#include <memory>

#include "Graph.h"
#include "LogConsole.h"

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
    virtual ~Trace() = default;
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

  Graph graph;
  bool logConsoleVisible;
  
  std::vector<std::unique_ptr<Trace>> traces;

public:
  Metrics();
  void render();
  int addGraph(std::string tag, float minVal, float maxVal, int valuesMaxCount);
  void setGraphVisibility(int graphId, bool visible);
  void updateGraphValues(int graphId, std::vector<float> values);
  void updateGraphValue(int graphId, float value);
  void updateGraphRange(int graphId, float minVal, float maxVal);

  void setLogConsoleVisibility(bool visible);
  void pushLog(std::string log);
};

#endif // _METRICS_H_
