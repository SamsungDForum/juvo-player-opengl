#ifndef _LOGCONSOLE_H_
#define _LOGCONSOLE_H_

#include <string>
#include <utility>
#include <deque>

#include "GLES.h"

class LogConsole {
private:

  GLuint programObject;
  GLuint posALoc;
  GLuint posLoc;
  GLuint sizLoc;
  GLuint colLoc;
  GLuint opaLoc;

  std::deque<std::string> logs;

  void initialize();
  void renderText(std::pair<int, int> position, std::pair<int, int> size, int fontId, int fontSize);
  void renderLogs(std::pair<int, int> position, std::pair<int, int> size, int fontId, int fontSize, std::pair<int, int> margin, int lineWidth);
  int getTextHeight(std::string s, int lineWidth, int fontHeight, int fontId);

  LogConsole();
  ~LogConsole();
  LogConsole(const LogConsole&) = delete;
  LogConsole& operator=(const LogConsole&) = delete;

public:
  static LogConsole& instance() {
    static LogConsole logConsole;
    return logConsole;
  }

  enum class LogLevel {
    Error,
    Debug,
    Info
  };

  void render(std::pair<int, int> position, std::pair<int, int> size, int fontId, int fontSize);
  void pushLog(std::string log);
  void log(std::string log, LogLevel logLevel);
};

#endif // _LOGCONSOLE_H_
