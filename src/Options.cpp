#include "Options.h"


Options::Options(std::pair<int, int> viewport)
  : viewport(viewport),
    optionRectangleSize({200, 40}),
    suboptionRectangleSize({500, 40}),
    position({130, 150}),
    margin({1, 1}),
    frameWidth(2),
    optionColor({0.3f, 0.3f, 0.3f}),
    selectedOptionColor({0.4f, 0.4f, 0.4f}),
    activeOptionColor({0.8f, 0.8f, 0.8f}),
    suboptionColor(optionColor),
    selectedSuboptionColor(selectedOptionColor),
    activeSuboptionColor(activeOptionColor),
    frameColor({1.0f, 1.0f, 1.0f}),
    maxTextLength(26),
    activeOptionId(-1),
    activeSuboptionId(-1),
    selectedOptionId(-1),
    selectedSuboptionId(-1),
    opacity(0.0f),
    show(false) {
    initialize();
}

void Options::initialize() {
  const GLchar* vShaderTexStr =  
    "attribute vec4 a_position;     \n"
    "void main()                    \n"
    "{                              \n"
    "   gl_Position = a_position;   \n"
    "}                              \n";

  const GLchar* fShaderTexStr =  
    "precision mediump float;                         \n"
    "uniform vec2 u_size;                             \n"
    "uniform vec2 u_position;                         \n"
    "uniform vec3 u_color;                            \n"
    "uniform float u_opacity;                         \n"
    "uniform float u_frameWidth;                      \n"
    "uniform vec3 u_frameColor;                       \n"
    "uniform int u_submenuSelected;                   \n"
    "                                                 \n"
    "void main()                                      \n"
    "{                                                \n"
    "  gl_FragColor = vec4(u_color, 1.0);             \n"
    "                                                 \n"
    "  if(u_frameWidth > 0.0) {                       \n"
    "    vec2 pos = gl_FragCoord.xy - u_position.xy;  \n"
    "    if(pos.x <= u_frameWidth ||                  \n"
    "       pos.x >= u_size.x - u_frameWidth ||       \n"
    "       pos.y <= u_frameWidth ||                  \n"
    "       pos.y >= u_size.y - u_frameWidth) {       \n"
    "      gl_FragColor.rgb = u_frameColor.rgb;       \n"
    "    }                                            \n"
    "  }                                              \n"
    "                                                 \n"
    "  gl_FragColor.a *= u_opacity * 0.75;            \n"
    "}                                                \n";

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vShaderTexStr, NULL);
  glCompileShader(vertexShader);
  checkShaderCompileError(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fShaderTexStr, NULL);
  glCompileShader(fragmentShader);
  checkShaderCompileError(fragmentShader);

  programObject = glCreateProgram();
  glAttachShader(programObject, vertexShader);
  glAttachShader(programObject, fragmentShader);
  glLinkProgram(programObject);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  apositionLoc       = glGetAttribLocation(programObject, "a_position");
  positionLoc        = glGetUniformLocation(programObject, "u_position");
  sizeLoc            = glGetUniformLocation(programObject, "u_size");
  colorLoc           = glGetUniformLocation(programObject, "u_color");
  opacityLoc         = glGetUniformLocation(programObject, "u_opacity");
  frameWidthLoc      = glGetUniformLocation(programObject, "u_frameWidth");
  frameColorLoc      = glGetUniformLocation(programObject, "u_frameColor");
  submenuSelectedLoc = glGetUniformLocation(programObject, "u_submenuSelected");
}

bool Options::addOption(int id, std::string name) {
  Option opt;
  opt.id = id;
  opt.name = name.substr(0, maxTextLength);
  options.insert({id, opt});
  return true;
}

bool Options::addSuboption(int parentId, int id, std::string name) {
  if(!options.count(parentId))
    return false;
  Suboption subopt;
  subopt.id = id;
  subopt.parentId = parentId;
  subopt.name = name.substr(0, maxTextLength);
  options.at(parentId).subopt.insert({id, subopt});
  return true;
}

bool Options::updateSelection(bool show, int activeOptionId, int activeSuboptionId, int selectedOptionId, int selectedSuboptionId) {
  this->show = show;
  this->activeOptionId = activeOptionId;
  this->activeSuboptionId = activeSuboptionId;
  this->selectedOptionId = selectedOptionId;
  this->selectedSuboptionId = selectedSuboptionId;
  return true;
}

void Options::clearOptions() {
  options.clear();
}

void Options::renderIcon(Text &text) {
  if(opacity <= 0.0f)
    return;
  renderRectangle(position,
                  optionRectangleSize,
                  viewport,
                  selectedOptionColor,
                  opacity,
                  "Options",
                  !show ? frameWidth : 0,
                  frameColor,
                  text);
}

void Options::render(Text &text) {
  if(!show || opacity <= 0.0f)
    return;
  std::pair<int, int> position {this->position.first + optionRectangleSize.first + margin.first, this->position.second + (options.size() - 1) * (optionRectangleSize.second + margin.second)};
  render(position, optionRectangleSize, suboptionRectangleSize, viewport, opacity, text);
}

void Options::render(std::pair<int, int> position, std::pair<int, int> optionRectangleSize, std::pair<int, int> suboptionRectangleSize, std::pair<int, int> viewport, float opacity, Text &text) {
  std::pair<int, int> optionPosition = position;
  for(const std::pair<int, Option>& option : options) {
    renderRectangle(optionPosition,
                    optionRectangleSize,
                    viewport,
                    option.first == activeOptionId ? activeOptionColor : option.first == selectedOptionId ? selectedOptionColor : optionColor,
                    opacity,
                    option.second.name,
                    option.first == selectedOptionId && selectedSuboptionId == -1 ? frameWidth : 0,
                    frameColor,
                    text,
                    option.first == selectedOptionId ? true : false);
    if(option.first == selectedOptionId) {
      std::pair<int, int> suboptionPosition = {optionPosition.first + optionRectangleSize.first + margin.first, optionPosition.second + (suboptionRectangleSize.second + margin.second) * (option.second.subopt.size() - 1)};
      for(const std::pair<int, Suboption>& suboption : option.second.subopt) {
        renderRectangle(suboptionPosition,
                        suboptionRectangleSize,
                        viewport,
                        suboption.first == activeSuboptionId ? activeSuboptionColor : suboption.first == selectedSuboptionId ? selectedSuboptionColor : suboptionColor,
                        opacity,
                        suboption.second.name,
                        suboption.first == selectedSuboptionId ? frameWidth : 0,
                        frameColor,
                        text,
                        false);
        suboptionPosition.second -= suboptionRectangleSize.second + margin.second;
      }
    }
    optionPosition.second -= optionRectangleSize.second + margin.second;
  }
}

void Options::renderRectangle(std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, std::vector<float> color, float opacity, std::string name, int frameWidth, std::vector<float> frameColor, Text &text, bool submenuSelected) {
  float down = static_cast<float>(position.second) / viewport.second * 2.0f - 1.0f;
  float top = static_cast<float>(position.second + size.second) / viewport.second * 2.0f - 1.0f;
  float left = static_cast<float>(position.first) / viewport.first * 2.0f - 1.0f;
  float right = static_cast<float>(position.first + size.first) / viewport.first * 2.0f - 1.0f;
  GLfloat vertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
  glUseProgram(programObject);
  glEnableVertexAttribArray(positionLoc);
  glVertexAttribPointer(apositionLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

  glUniform2f(positionLoc, position.first, position.second);
  glUniform2f(sizeLoc, size.first, size.second);
  if(color.size() >= 3)
    glUniform3f(colorLoc, color[0], color[1], color[2]);
  else if(color.size() == 1)
    glUniform3f(colorLoc, color[0], color[0], color[0]);
  else
    glUniform3f(colorLoc, 0.0f, 0.0f, 0.0f);
  glUniform1f(opacityLoc, opacity);

  glUniform1f(frameWidthLoc, frameWidth);
  if(frameColor.size() >= 3)
    glUniform3f(frameColorLoc, frameColor[0], frameColor[1], frameColor[2]);
  else if(frameColor.size() == 1)
    glUniform3f(frameColorLoc, frameColor[0], frameColor[0], frameColor[0]);
  else
    glUniform3f(frameColorLoc, 0.0f, 0.0f, 0.0f);
  glUniform1f(submenuSelectedLoc, submenuSelected ? 1 : 0);

  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  int fontHeight = size.second / 2;
  int margin = (size.second - fontHeight) / 2;
  text.render(name,
              {position.first + margin, position.second + margin},
              {0, fontHeight},
              viewport,
              0,
              {1.0, 1.0, 1.0, opacity},
              true);
}

void Options::checkShaderCompileError(GLuint shader) {
  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if(isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
    _ERR("%s", (std::string(errorLog.begin(), errorLog.end()).c_str()));

    glDeleteShader(shader);
  }
}


