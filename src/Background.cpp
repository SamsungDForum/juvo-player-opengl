#include "Background.h"
#include "ProgramBuilder.h"
#include "Settings.h"
#include "TextRenderer.h"

#include <string>

Background::Background()
  : programObject(GL_INVALID_VALUE),
    textureFormat(GL_INVALID_VALUE),
    opacity(0),
    black(1.0),
    sourceTile(nullptr),
    clearColor({}) {
  initGL();
}

Background::Background(float opacity)
  : programObject(GL_INVALID_VALUE),
    textureFormat(GL_INVALID_VALUE),
    opacity(opacity),
    black(1.0),
    sourceTile(nullptr),
    clearColor({}) {
  initGL();
}

Background::~Background() {
  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
}

void Background::initGL() {
  const GLchar* vShaderTexStr = 
#include "shaders/background.vert"
;
 
  const GLchar* fShaderTexStr =  
#include "shaders/background.frag"
;

  programObject = ProgramBuilder::buildProgram(vShaderTexStr, fShaderTexStr);

  samplerLoc = glGetUniformLocation(programObject, "s_texture");
  posLoc = glGetAttribLocation(programObject, "a_position");
  texLoc = glGetAttribLocation(programObject, "a_texCoord");
  opacityLoc = glGetUniformLocation(programObject, "u_opacity");
  blackLoc = glGetUniformLocation(programObject, "u_black");
  viewportLoc = glGetUniformLocation(programObject, "u_viewport");
}

void Background::render() {
  GLuint textureId = sourceTile != nullptr ? sourceTile->getTextureId() : GL_INVALID_VALUE;
  if(textureId == GL_INVALID_VALUE)
    return;
  opacity = sourceTile != nullptr ? sourceTile->getOpacity() : 1.0;

  float left = -1.0;
  float right = 1.0;
  float top = 1.0;
  float down = -1.0;

  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
  float texCoord[] = { 0.0f, 0.0f,    0.0f, 1.0f,
                       1.0f, 1.0f,    1.0f, 0.0f };
                       
  glUseProgram(programObject);

  glUniform1f(opacityLoc, static_cast<GLfloat>(opacity));
  std::vector<double> updated = animation.update();
  if(!updated.empty())
    black = animation.update()[0];
  glUniform1f(blackLoc, static_cast<GLfloat>(black));
  glUniform2f(viewportLoc, static_cast<GLfloat>(Settings::instance().viewport.first), static_cast<GLfloat>(Settings::instance().viewport.second));

  glBindTexture(GL_TEXTURE_2D, textureId);
  glUniform1i(samplerLoc, 0);

  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
  glEnableVertexAttribArray(texLoc);
  glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoord);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(posLoc);
  glDisableVertexAttribArray(texLoc);
  glBindTexture(GL_TEXTURE_2D, GL_INVALID_VALUE);
  glUseProgram(0);

  renderNameAndDescription();
}

void Background::renderNameAndDescription() {
  std::string name = sourceTile != nullptr ? sourceTile->getName() : "";
  int textLineOffset = 0;
  if(!name.empty()) {
    int fontHeight = 52;
    int leftText = 100;
    int topText = Settings::instance().viewport.second - fontHeight - 200;
    TextRenderer::instance().render(name,
                {leftText, topText},
                {Settings::instance().viewport.first - 2 * leftText, fontHeight},
                0,
                {1.0, 1.0, 1.0, opacity});

    textLineOffset = TextRenderer::instance().getTextSize(name,
                                      {Settings::instance().viewport.first - 2 * leftText, fontHeight},
                                      0
                     ).second;
  }
  std::string description = sourceTile != nullptr ? sourceTile->getDescription() : "";
  if(!description.empty()) {
    int fontHeight = 26;
    int leftText = 100;
    int topText = Settings::instance().viewport.second - fontHeight - 200 - textLineOffset;
    TextRenderer::instance().render(description,
                {leftText, topText},
                {Settings::instance().viewport.first - 2 * leftText, fontHeight},
                0,
                {1.0, 1.0, 1.0, opacity});
  }
}

void Background::setOpacity(float opacity) {
  this->opacity = opacity;
}

float Background::getOpacity() {
  return opacity;
}

void Background::setBlack(float black) {
  this->black = black;
}

float Background::getBlack() {
  return black;
}

void Background::setSourceTile(Tile *sourceTile, std::chrono::milliseconds duration, std::chrono::milliseconds delay) {
  if(!animation.isActive() || duration != std::chrono::milliseconds(0))
    animation = Animation(std::chrono::high_resolution_clock::now(),
                          duration,
                          delay,
                          {1.0},
                          {0.0},
                          Animation::Easing::CubicInOut);
  this->sourceTile = sourceTile;
}

void Background::setClearColor(std::vector<float> color) {
  clearColor = color;
}
