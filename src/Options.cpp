#include "Options.h"
#include "ProgramBuilder.h"
#include "Settings.h"
#include "TextRenderer.h"

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
  Position<int> optionPosition { this->position.x + margin.width, this->position.y + static_cast<int>(options.empty() ? 0 : options.size() - 1) * (optionRectangleSize.height + margin.height) };
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
      Position<int> suboptionPosition = {optionPosition.x + optionRectangleSize.width + margin.width, optionPosition.y + (suboptionRectangleSize.height + margin.height) * static_cast<int>(option.second.subopt.empty() ? 0 : option.second.subopt.size() - 1)};
      for(const std::pair<int, Suboption>& suboption : option.second.subopt) {
        renderRectangle(suboptionPosition,
                        suboptionRectangleSize,
                        suboption.first == activeSuboptionId ? activeSuboptionColor : suboption.first == selectedSuboptionId ? selectedSuboptionColor : suboptionColor,
                        opacity,
                        suboption.second.name,
                        suboption.first == selectedSuboptionId ? frameWidth : 0,
                        frameColor,
                        false);
        suboptionPosition.y -= suboptionRectangleSize.height + margin.height;
      }
    }
    optionPosition.y -= optionRectangleSize.height + margin.height;
  }
}

void Options::renderRectangle(Position<int> position, Size<int> size, std::vector<float> color, float opacity, std::string name, int frameWidth, std::vector<float> frameColor, bool submenuSelected) {
  float down = static_cast<float>(position.y) / Settings::instance().viewport.height * 2.0f - 1.0f;
  float top = static_cast<float>(position.y + size.height) / Settings::instance().viewport.height * 2.0f - 1.0f;
  float left = static_cast<float>(position.x) / Settings::instance().viewport.width * 2.0f - 1.0f;
  float right = static_cast<float>(position.x + size.width) / Settings::instance().viewport.width * 2.0f - 1.0f;
  GLfloat vertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(programObject);
  glEnableVertexAttribArray(positionALoc);
  glVertexAttribPointer(positionALoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

  glUniform2f(positionLoc, position.x, position.y);
  glUniform2f(sizeLoc, size.width, size.height);
  glUniform3f(colorLoc, color[0], color[1], color[2]);
  glUniform1f(opacityLoc, opacity);

  glUniform1f(frameWidthLoc, frameWidth);
  glUniform3f(frameColorLoc, frameColor[0], frameColor[1], frameColor[2]);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(positionALoc);
  glUseProgram(0);

  int fontHeight = size.height / 2;
  int margin = (size.height - fontHeight) / 2;
  TextRenderer::instance().render(name,
              {position.x + margin, position.y + margin},
              {0, fontHeight},
              0,
              {1.0, 1.0, 1.0, opacity});
}

