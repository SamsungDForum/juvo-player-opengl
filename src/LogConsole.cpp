#include "LogConsole.h"
#include "ProgramBuilder.h"
#include "Settings.h"
#include "TextRenderer.h"
#include "log.h"

LogConsole::LogConsole()
    : programObject(GL_INVALID_VALUE),
    posALoc(GL_INVALID_VALUE),
    posLoc(GL_INVALID_VALUE),
    sizLoc(GL_INVALID_VALUE),
    colLoc(GL_INVALID_VALUE),
    opaLoc(GL_INVALID_VALUE) {
  initialize();
}

LogConsole::~LogConsole() {
  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
}

void LogConsole::initialize() {
  const GLchar* vShaderTexStr =  
#include "shaders/logConsole.vert"
;

  const GLchar* fShaderTexStr =  
#include "shaders/logConsole.frag"
;

  programObject = ProgramBuilder::buildProgram(vShaderTexStr, fShaderTexStr);

  posALoc = glGetAttribLocation(programObject, "a_position");
  posLoc = glGetUniformLocation(programObject, "u_position");
  sizLoc = glGetUniformLocation(programObject, "u_size");
  colLoc = glGetUniformLocation(programObject, "u_color");
  opaLoc = glGetUniformLocation(programObject, "u_opacity");
}

void LogConsole::render(std::pair<int, int> position, std::pair<int, int> size, int fontId, int fontSize) {
  float down  = static_cast<float>(position.second) / static_cast<float>(Settings::instance().viewport.second) * 2.0f - 1.0f;
  float top   = (static_cast<float>(position.second) + static_cast<float>(size.second)) / static_cast<float>(Settings::instance().viewport.second) * 2.0f - 1.0f;
  float left  = static_cast<float>(position.first) / static_cast<float>(Settings::instance().viewport.first) * 2.0f - 1.0f;
  float right = (static_cast<float>(position.first) + static_cast<float>(size.first)) / static_cast<float>(Settings::instance().viewport.first) * 2.0f - 1.0f;

  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(programObject);
  glEnableVertexAttribArray(posALoc);
  glVertexAttribPointer(posALoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  glUniform2f(posLoc, static_cast<float>(position.first), static_cast<float>(position.second));
  glUniform2f(sizLoc, static_cast<float>(size.first), static_cast<float>(size.second));
  glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);
  glUniform1f(opaLoc, 1.0f);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(posALoc);
  glUseProgram(0);

  renderText(position, size, fontId, fontSize);
}

void LogConsole::renderText(std::pair<int, int> position, std::pair<int, int> size, int fontId, int fontSize) {
  std::pair<int, int> margin = {4, 10};
  int lineWidth = size.first - 2 * margin.first;
  renderLogs(position, size, fontId, fontSize, margin, lineWidth);
}

int LogConsole::getTextHeight(std::string s, int lineWidth, int fontHeight, int fontId) {
  return TextRenderer::instance().getTextSize(s,
                          {lineWidth, fontHeight},
                          fontId).second;
}

void LogConsole::renderLogs(std::pair<int, int> position, std::pair<int, int> size, int fontId, int fontSize, std::pair<int, int> margin, int lineWidth) {
  int i = 0;
  std::deque<std::string>::iterator deqit = logs.begin();
  for(int offset = margin.second; deqit != logs.end(); ++deqit, ++i) {
    int textHeight = getTextHeight(*deqit, lineWidth, fontSize, fontId);
    if(offset + textHeight + margin.second > size.second)
      break;
    TextRenderer::instance().render(*deqit,
                {position.first + margin.first, position.second + size.second - offset - fontSize},
                {lineWidth, fontSize},
                fontId,
                {1.0f, 1.0f, 1.0f, 1.0f});
    offset += textHeight + margin.second;
  }
  logs.erase(logs.begin(), logs.end() - i);
}

void LogConsole::log(std::string log, LogLevel logLevel) {
  switch(logLevel) {
    case LogLevel::Error:
      _ERR("%s", log.c_str());
      break;
    case LogLevel::Debug:
      _DBG("%s", log.c_str());
      break;
    case LogLevel::Info:
      _INFO("%s", log.c_str());
      break;
  }
  pushLog(log);
}

void LogConsole::pushLog(std::string log) {
  logs.push_back(log);
}

