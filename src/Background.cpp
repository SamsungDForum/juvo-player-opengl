#include "Background.h"
#include "ProgramBuilder.h"
#include "Settings.h"
#include "TextRenderer.h"
#include "Utility.h"

#include <string>

Background::Background()
  : programObject(GL_INVALID_VALUE),
    textureFormat(GL_INVALID_VALUE),
    opacity(0.0f),
    mixing(1.0f),
    lastTile(nullptr),
    currentTile(nullptr),
    queuedTile(nullptr) {
  initGL();
}

Background::~Background() {
  Utility::assertCurrentEGLContext();

  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
}

void Background::initGL() {
  Utility::assertCurrentEGLContext();

  const GLchar* vShaderTexStr = 
#include "shaders/background.vert"
;
 
  const GLchar* fShaderTexStr =  
#include "shaders/background.frag"
;

  programObject = ProgramBuilder::buildProgram(vShaderTexStr, fShaderTexStr);

  posLoc = glGetAttribLocation(programObject, "a_position");
  texLoc = glGetAttribLocation(programObject, "a_texCoord");
  samplerLoc = glGetUniformLocation(programObject, "s_texture");
  sampler2Loc = glGetUniformLocation(programObject, "s_texture2");
  opacityLoc = glGetUniformLocation(programObject, "u_opacity");
  mixingLoc = glGetUniformLocation(programObject, "u_mixing");
  viewportLoc = glGetUniformLocation(programObject, "u_viewport");
}

void Background::render() {
  Utility::assertCurrentEGLContext();

  opacity = getOpacity();
  if(opacity < 0.001f)
    return;

  GLuint textureId = currentTile != nullptr ? currentTile->getTextureId() : 0;
  if(!textureId)
    return;
  GLuint texture2Id = lastTile != nullptr ? lastTile->getTextureId() : 0;
  if(texture2Id == 0)
    texture2Id = textureId;

  float left = -1.0;
  float right = 1.0;
  float top = 1.0;
  float down = -1.0;

  GLfloat vertices[] = { left,   top,  0.0f,
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
    mixing = updated[0];
  glUniform1f(mixingLoc, static_cast<GLfloat>(mixing));
  glUniform2f(viewportLoc, static_cast<GLfloat>(Settings::instance().viewport.width), static_cast<GLfloat>(Settings::instance().viewport.height));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glUniform1i(samplerLoc, 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture2Id);
  glUniform1i(sampler2Loc, 1);

  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);
  glEnableVertexAttribArray(texLoc);
  glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoord);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(posLoc);
  glDisableVertexAttribArray(texLoc);
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);

  renderNameAndDescription();
}

void Background::renderNameAndDescription() {
  std::string name = currentTile != nullptr ? currentTile->getName() : "";
  int textLineOffset = 0;
  if(!name.empty()) {
    int fontHeight = 52;
    int leftText = 100;
    int topText = Settings::instance().viewport.height - fontHeight - 200;
    TextRenderer::instance().render(name,
                {leftText, topText},
                {Settings::instance().viewport.width - 2 * leftText, fontHeight},
                0,
                {1.0, 1.0, 1.0, opacity});

    textLineOffset = TextRenderer::instance().getTextSize(
                       name,
                       { static_cast<GLuint>(Settings::instance().viewport.width - 2 * leftText), static_cast<GLuint>(fontHeight) },
                       0
                     ).height;
  }
  std::string description = currentTile != nullptr ? currentTile->getDescription() : "";
  if(!description.empty()) {
    int fontHeight = 26;
    int leftText = 100;
    int topText = Settings::instance().viewport.height - fontHeight - 200 - textLineOffset;
    TextRenderer::instance().render(description,
                {leftText, topText},
                {Settings::instance().viewport.width - 2 * leftText, fontHeight},
                0,
                {1.0, 1.0, 1.0, opacity});
  }
}

void Background::setOpacity(float opacity) {
  this->opacity = opacity;
}

float Background::getOpacity() {
  return currentTile != nullptr ? currentTile->getOpacity() : 0.0;
}

void Background::setSourceTile(Tile *tile) {
  if(animation.isActive() && !animation.isDuringDelay()) {
    queuedTile = tile;
    return;
  }

  if(tile != currentTile && !animation.isActive())
    lastTile = currentTile;
  currentTile = tile;

  runBackgroundChangeAnimation();
}

void Background::endAnimation() {
  if(queuedTile == nullptr)
    return;

  lastTile = currentTile;
  currentTile = queuedTile;
  queuedTile = nullptr;

  runBackgroundChangeAnimation();
}

void Background::runBackgroundChangeAnimation() {
  animation = Animation(Settings::instance().backgroundChangeDuration,
                        Settings::instance().backgroundChangeDelay,
                        { 1.0 },
                        { 0.0 },
                        Animation::Easing::CubicInOut,
                        std::bind(&Background::endAnimation, this));
}

