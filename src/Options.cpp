#include "Options.h"
#include "ProgramBuilder.h"
#include "Settings.h"

Options::Options()
  : optionRectangleSize({200, 40}),
    suboptionRectangleSize({500, 40}),
    position({130 + 64, 150}),
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
#include "shaders/options.vert"
;

  const GLchar* fShaderTexStr = 
#include "shaders/options.frag"
;

  programObject = ProgramBuilder::buildProgram(vShaderTexStr, fShaderTexStr);

  positionALoc       = glGetAttribLocation(programObject, "a_position");
  positionLoc        = glGetUniformLocation(programObject, "u_position");
  sizeLoc            = glGetUniformLocation(programObject, "u_size");
  colorLoc           = glGetUniformLocation(programObject, "u_color");
  opacityLoc         = glGetUniformLocation(programObject, "u_opacity");
  frameWidthLoc      = glGetUniformLocation(programObject, "u_frameWidth");
  frameColorLoc      = glGetUniformLocation(programObject, "u_frameColor");
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

void Options::renderIcon() {
  if(opacity <= 0.0f)
    return;
  renderRectangle(position,
                  optionRectangleSize,
                  selectedOptionColor,
                  opacity,
                  "Options",
                  !show ? frameWidth : 0,
                  frameColor);
}

void Options::render() {
  if(!show || opacity <= 0.0f)
    return;
  std::pair<int, int> position {this->position.first + margin.first, this->position.second + (options.size() - 1) * (optionRectangleSize.second + margin.second)};
  render(position, optionRectangleSize, suboptionRectangleSize, opacity); // TODO: merge both methods like in OpenTK?
}

void Options::render(std::pair<int, int> position, std::pair<int, int> optionRectangleSize, std::pair<int, int> suboptionRectangleSize, float opacity) {
  std::pair<int, int> optionPosition = position;
  for(const std::pair<int, Option>& option : options) {
    renderRectangle(optionPosition,
                    optionRectangleSize,
                    option.first == activeOptionId ? activeOptionColor : option.first == selectedOptionId ? selectedOptionColor : optionColor,
                    opacity,
                    option.second.name,
                    option.first == selectedOptionId && selectedSuboptionId == -1 ? frameWidth : 0,
                    frameColor,
                    option.first == selectedOptionId ? true : false);
    if(option.first == selectedOptionId) {
      std::pair<int, int> suboptionPosition = {optionPosition.first + optionRectangleSize.first + margin.first, optionPosition.second + (suboptionRectangleSize.second + margin.second) * (option.second.subopt.size() - 1)};
      for(const std::pair<int, Suboption>& suboption : option.second.subopt) {
        renderRectangle(suboptionPosition,
                        suboptionRectangleSize,
                        suboption.first == activeSuboptionId ? activeSuboptionColor : suboption.first == selectedSuboptionId ? selectedSuboptionColor : suboptionColor,
                        opacity,
                        suboption.second.name,
                        suboption.first == selectedSuboptionId ? frameWidth : 0,
                        frameColor,
                        false);
        suboptionPosition.second -= suboptionRectangleSize.second + margin.second;
      }
    }
    optionPosition.second -= optionRectangleSize.second + margin.second;
  }
}

void Options::renderRectangle(std::pair<int, int> position, std::pair<int, int> size, std::vector<float> color, float opacity, std::string name, int frameWidth, std::vector<float> frameColor, bool submenuSelected) {
  float down = static_cast<float>(position.second) / Settings::viewport.second * 2.0f - 1.0f;
  float top = static_cast<float>(position.second + size.second) / Settings::viewport.second * 2.0f - 1.0f;
  float left = static_cast<float>(position.first) / Settings::viewport.first * 2.0f - 1.0f;
  float right = static_cast<float>(position.first + size.first) / Settings::viewport.first * 2.0f - 1.0f;
  GLfloat vertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(programObject);
  glEnableVertexAttribArray(positionLoc);
  glVertexAttribPointer(positionALoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

  glUniform2f(positionLoc, position.first, position.second);
  glUniform2f(sizeLoc, size.first, size.second);
  glUniform3f(colorLoc, color[0], color[1], color[2]);
  glUniform1f(opacityLoc, opacity);

  glUniform1f(frameWidthLoc, frameWidth);
  glUniform3f(frameColorLoc, frameColor[0], frameColor[1], frameColor[2]);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(positionLoc);
  glUseProgram(GL_INVALID_VALUE);

  int fontHeight = size.second / 2;
  int margin = (size.second - fontHeight) / 2;
  Text::instance().render(name,
              {position.first + margin, position.second + margin},
              {0, fontHeight},
              0,
              {1.0, 1.0, 1.0, opacity},
              true);
}

