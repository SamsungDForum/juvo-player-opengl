#include "ModalWindow.h"
#include "ProgramBuilder.h"
#include "Settings.h"
#include "Text.h"

ModalWindow::ModalWindow()
    : programObject(GL_INVALID_VALUE),
    posALoc(GL_INVALID_VALUE),
    posLoc(GL_INVALID_VALUE),
    sizLoc(GL_INVALID_VALUE),
    colLoc(GL_INVALID_VALUE),
    opaLoc(GL_INVALID_VALUE),
    visible(false) {
  initialize();
}

ModalWindow::~ModalWindow() {
  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
}

void ModalWindow::initialize() {
  const GLchar* vShaderTexStr =  
#include "shaders/modalWindow.vert"
;

  const GLchar* fShaderTexStr = 
#include "shaders/modalWindow.frag"
;

  programObject = ProgramBuilder::buildProgram(vShaderTexStr, fShaderTexStr);

  posALoc = glGetAttribLocation(programObject, "a_position");
  posLoc = glGetUniformLocation(programObject, "u_position");
  sizLoc = glGetUniformLocation(programObject, "u_size");
  colLoc = glGetUniformLocation(programObject, "u_color");
  opaLoc = glGetUniformLocation(programObject, "u_opacity");
}

void ModalWindow::render(int fontId) { // TODO: remove fontId arg?
  if(!visible)
    return;

  renderRectangle(position, size);
  renderContent();
}

void ModalWindow::calculateParams() {
  int windowWidth = Settings::instance().viewport.first / 2;
  std::pair<int,int> margin = {10, 10};
  std::pair<int, int> size = {windowWidth,
                              Settings::instance().viewport.second / 2.5};
  std::pair<int, int> position = {(Settings::instance().viewport.first - size.first) / 2,
                                  (Settings::instance().viewport.second - size.second) / 2};

  Params params {
    .title = TextParams {
      .fontId = 0,
      .fontSize = 52,
      .text = title,
      .position = {0, 0},
      .size = {1, 1},
      .color = {1.0f, 1.0f, 1.0f, 1.0f}
    },
    .body = TextParams {
      .fontId = 0,
      .fontSize = 26,
      .text = body,
      .position = {0, 0},
      .size = {1, 1},
      .color = {1.0f, 1.0f, 1.0f, 1.0f}
    },
    .lineWidth = size.first - 2 * margin.first,

    .window = WindowParams {
      .position = position,
      .size = size,
      .margin = margin
    },

    .buttonWindow = WindowParams {
      .position = {0, 0},
      .size = {1, 1},
      .margin = {0, 0}
    },
    .buttonText = TextParams {
      .fontId = 0,
      .fontSize = 26,
      .text = button,
      .position = {0, 0},
      .size = {1, 1},
      .color = {1.0f, 1.0f, 1.0f, 1.0f}
    }
  };

  calculateElementsPositions();
}

void ModalWindow::renderRectangle(std::pair<int, int> position, std::pair<int, int> size) {
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
  glUseProgram(GL_INVALID_VALUE);
}

void ModalWindow::renderContent() {
  renderTitle();
  renderBody();
  renderButton();
}

void ModalWindow::calculateElementsPositions() {
  params.title.size = getTextSize(params.title.text,
                                  params.lineWidth,
                                  params.title.fontSize,
                                  params.title.fontId);
  params.body.size = getTextSize(params.body.text,
                                 params.lineWidth,
                                 params.body.fontSize,
                                 params.body.fontId);
  params.title.position = {params.window.position.first + (params.window.size.first - params.title.size.first) / 2,
                           (params.window.position.second + params.window.size.second) - params.window.margin.second - params.title.fontSize};
  params.body.position = {params.window.position.first + (params.window.size.first - params.body.size.first) / 2,
                          params.window.position.second + (params.window.size.second + params.body.size.second) / 2 - params.body.fontSize};
  params.buttonText.size = getTextSize(params.buttonText.text,
                                       params.lineWidth,
                                       params.buttonText.fontSize,
                                       params.buttonText.fontId);
  params.buttonWindow.size = {params.buttonText.size.first + 4 * params.buttonText.fontSize,
                              params.buttonText.size.second + 2 * params.buttonText.fontSize};
  params.buttonWindow.position = {params.window.position.first + (params.window.size.first - params.buttonWindow.size.first) / 2,
                                  params.window.position.second + params.window.margin.second};
  params.buttonText.position = {params.buttonWindow.position.first + (params.buttonWindow.size.first - params.buttonText.size.first) / 2,
                                params.buttonWindow.position.second + (params.buttonWindow.size.second + params.buttonText.size.second) / 2 - params.buttonText.fontSize};
}

void ModalWindow::renderTitle() {
  Text::instance().render(params.title.text,
                     params.title.position,
                     {params.lineWidth, params.title.fontSize},
                     params.title.fontId,
                     params.title.color,
                     true);
}

void ModalWindow::renderBody() {
  Text::instance().render(params.body.text,
                     params.body.position,
                     {params.lineWidth, params.body.fontSize},
                     params.body.fontId,
                     params.body.color,
                     true);
}

void ModalWindow::renderButton() {
  renderRectangle(params.buttonWindow.position,
                  params.buttonWindow.size);
  Text::instance().render(params.buttonText.text,
                     params.buttonText.position,
                     {params.lineWidth, params.buttonText.fontSize},
                     params.buttonText.fontId,
                     params.buttonText.color,
                     true);
}

std::pair<int, int> ModalWindow::getTextSize(std::string s, int lineWidth, int fontHeight, int fontId) {
  std::pair<float, float> size = Text::instance().getTextSize(s,
                                                  {lineWidth, fontHeight},
                                                  fontId);
  return {size.first * static_cast<float>(Settings::instance().viewport.first) / 2.0f,
          size.second * static_cast<float>(Settings::instance().viewport.second) / 2.0f};
}

void ModalWindow::show(std::string title, std::string body, std::string button) {
  this->title = title;
  this->body = body;
  this->button = button;
  visible = true;
  calculateParams();
}

void ModalWindow::hide() {
  visible = false;
}

bool ModalWindow::isVisible() {
  return visible;
}

