#include "ModalWindow.h"
#include "ProgramBuilder.h"
#include "Settings.h"
#include "TextRenderer.h"

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

void ModalWindow::render() {
  if(!visible)
    return;

  renderRectangle(position, size);
  renderContent();
}

void ModalWindow::calculateParams() {
  int windowWidth = Settings::instance().viewport.width / 2;
  Size<int> margin = {10, 10};
  size = { windowWidth,
           static_cast<int>(Settings::instance().viewport.height / 2.5) };
  position = { (Settings::instance().viewport.width - size.width) / 2,
               (Settings::instance().viewport.height - size.height) / 2 };

  params = Params {
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
    .lineWidth = size.width - 2 * margin.width,

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

void ModalWindow::renderRectangle(Position<int> position, Size<int> size) {
  float down  = static_cast<float>(position.y) / static_cast<float>(Settings::instance().viewport.height) * 2.0f - 1.0f;
  float top   = (static_cast<float>(position.y) + static_cast<float>(size.height)) / static_cast<float>(Settings::instance().viewport.height) * 2.0f - 1.0f;
  float left  = static_cast<float>(position.x) / static_cast<float>(Settings::instance().viewport.width) * 2.0f - 1.0f;
  float right = (static_cast<float>(position.x) + static_cast<float>(size.width)) / static_cast<float>(Settings::instance().viewport.width) * 2.0f - 1.0f;
  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(programObject);
  glEnableVertexAttribArray(posALoc);
  glVertexAttribPointer(posALoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  glUniform2f(posLoc, static_cast<float>(position.x), static_cast<float>(position.y));
  glUniform2f(sizLoc, static_cast<float>(size.width), static_cast<float>(size.height));
  glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);
  glUniform1f(opaLoc, 1.0f);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(posALoc);
  glUseProgram(0);
}

void ModalWindow::renderContent() {
  renderTitle();
  renderBody();
  renderButton();
}

void ModalWindow::calculateElementsPositions() {
  params.title.size = TextRenderer::instance().getTextSize(
    params.title.text,
    { static_cast<GLuint>(params.lineWidth), static_cast<GLuint>(params.title.fontSize) },
    params.title.fontId);
  params.body.size = TextRenderer::instance().getTextSize(
    params.body.text,
    { static_cast<GLuint>(params.lineWidth), static_cast<GLuint>(params.body.fontSize) },
    params.body.fontId);
  params.title.position = {params.window.position.x + (params.window.size.width - params.title.size.width) / 2,
                           (params.window.position.y + params.window.size.height) - params.window.margin.height - params.title.fontSize};
  params.body.position = {params.window.position.x + (params.window.size.width - params.body.size.width) / 2,
                          params.window.position.y + (params.window.size.height + params.body.size.height) / 2 - params.body.fontSize};
  params.buttonText.size = TextRenderer::instance().getTextSize(
    params.buttonText.text,
    { static_cast<GLuint>(params.lineWidth), static_cast<GLuint>(params.buttonText.fontSize) },
    params.buttonText.fontId);
  params.buttonWindow.size = {params.buttonText.size.width + 4 * params.buttonText.fontSize,
                              params.buttonText.size.height + 2 * params.buttonText.fontSize};
  params.buttonWindow.position = {params.window.position.x + (params.window.size.width - params.buttonWindow.size.width) / 2,
                                  params.window.position.y + params.window.margin.height};
  params.buttonText.position = {params.buttonWindow.position.x + (params.buttonWindow.size.width - params.buttonText.size.width) / 2,
                                params.buttonWindow.position.y + (params.buttonWindow.size.height + params.buttonText.size.height) / 2 - params.buttonText.fontSize};
}

void ModalWindow::renderTitle() {
  TextRenderer::instance().render(params.title.text,
                     params.title.position,
                     {params.lineWidth, params.title.fontSize},
                     params.title.fontId,
                     params.title.color);
}

void ModalWindow::renderBody() {
  TextRenderer::instance().render(params.body.text,
                     params.body.position,
                     {params.lineWidth, params.body.fontSize},
                     params.body.fontId,
                     params.body.color);
}

void ModalWindow::renderButton() {
  renderRectangle(params.buttonWindow.position,
                  params.buttonWindow.size);
  TextRenderer::instance().render(params.buttonText.text,
                     params.buttonText.position,
                     {params.lineWidth, params.buttonText.fontSize},
                     params.buttonText.fontId,
                     params.buttonText.color);
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

