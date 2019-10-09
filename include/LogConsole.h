#ifndef _LOGCONSOLE_H_
#define _LOGCONSOLE_H_

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

#include <string>
#include <utility>
#include <deque>

#include "Text.h"

class LogConsole {
private:

  GLuint programObject;
  GLuint posALoc;
  GLuint posLoc;
  GLuint sizLoc;
  GLuint colLoc;
  GLuint opaLoc;

  std::deque<std::string> logs;
  bool doCleanup;

  void initialize();
  void checkShaderCompileError(GLuint shader);
  void renderText(Text &text, std::pair<int, int> viewport, std::pair<int, int> position, std::pair<int, int> size, int fontId, int fontSize);
  void cleanupLogs(Text &text, std::pair<int, int> viewport, std::pair<int, int> size, int fontId, int fontSize, std::pair<int, int> margin, int lineWidth);
  void renderLogs(Text &text, std::pair<int, int> viewport, std::pair<int, int> position, std::pair<int, int> size, int fontId, int fontSize, std::pair<int, int> margin, int lineWidth);
  int getTextHeight(Text &text, std::string s, int lineWidth, int fontHeight, int fontId, std::pair<int, int> viewport);

public:
  LogConsole();
  ~LogConsole();
  void render(Text &text, std::pair<int, int> viewport, std::pair<int, int> position, std::pair<int, int> size, int fontId, int fontSize);
  void pushLog(std::string log);
};

#endif // _LOGCONSOLE_H_
