#include "Loader.h"
#include "ProgramBuilder.h"
#include "Settings.h"
#include "TextRenderer.h"
#include "Utility.h"

Loader::Loader()
  : percent(0),
    programObject(GL_INVALID_VALUE),
    logoProgramObject(GL_INVALID_VALUE),
    logoTextureId(0),
    progressBarSize({ 1000, 4 }),
    logoMaxSize({ Settings::instance().viewport.width / 4, Settings::instance().viewport.height / 4 }),
    logoSize(logoMaxSize),
    logoPosition({ 0, 0 }),
    progressBarPosition({ 0, 0 }),
    verticalMargin(10),
    backgroundColor({ 0.0f, 0.0f, 0.0f }) {
  recalculateSizesAndPositions(logoSize);
  initialize();
}

Loader::~Loader() {
  Utility::assertCurrentEGLContext();

  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
  if(logoProgramObject != GL_INVALID_VALUE)
    glDeleteProgram(logoProgramObject);
  if(logoTextureId != 0) {
    glDeleteTextures(1, &logoTextureId);
    logoTextureId = 0;
  }
}

void Loader::initialize() {
  Utility::assertCurrentEGLContext();

  if(programObject == GL_INVALID_VALUE) {
    const GLchar* vShaderTexStr =
#include "shaders/loader.vert"
  ;

    const GLchar* fShaderTexStr =
#include "shaders/loader.frag"
  ;

    programObject = ProgramBuilder::buildProgram(vShaderTexStr, fShaderTexStr);

    positionLoc = glGetAttribLocation(programObject, "a_position");
    percentLoc = glGetUniformLocation(programObject, "u_percent");
    viewportLoc = glGetUniformLocation(programObject, "u_viewport");
    posLoc = glGetUniformLocation(programObject, "u_position");
    sizLoc = glGetUniformLocation(programObject, "u_size");
    fgColorLoc = glGetUniformLocation(programObject, "u_fgcolor");
    bgColorLoc = glGetUniformLocation(programObject, "u_bgcolor");
  }

  if(logoProgramObject == GL_INVALID_VALUE) {
    const GLchar* vlogoShaderTexStr =
#include "shaders/image.vert"
  ;

    const GLchar* flogoShaderTexStr =
#include "shaders/image.frag"
  ;

    logoProgramObject = ProgramBuilder::buildProgram(vlogoShaderTexStr, flogoShaderTexStr);

    logoPosLoc = glGetAttribLocation(logoProgramObject, "a_position");
    logoSamplerLoc = glGetUniformLocation(logoProgramObject, "s_texture");
    logoTexLoc = glGetAttribLocation(logoProgramObject, "a_texCoord");
  }

  initTexture();
}

void Loader::initTexture() {
  if(logoTextureId == 0)
    glGenTextures(1, &logoTextureId);
  if(logoTextureId == GL_INVALID_VALUE)
    throw("-----===== INVALID VALUE FOR LOADER LOGO TEXTURE! =====-----");
}

void Loader::setValue(int value) {
  Animation::Easing easing = animation.isActive() ? Animation::Easing::CubicOut : Animation::Easing::CubicInOut;
  animation = Animation(Settings::instance().loaderUpdateAnimationDuration,
                        Settings::instance().loaderUpdateAnimationDelay,
                        {static_cast<double>(percent)},
                        {static_cast<double>(value)},
                        easing);
  percent = value;
}

void Loader::render() {
  Utility::assertCurrentEGLContext();

  glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  updatePercent();
  renderLogo(logoSize, logoPosition);
  renderProgressBar(progressBarSize, progressBarPosition, percent);
}

void Loader::updatePercent() {
  if(animation.isActive())
    percent = static_cast<float>(animation.update()[0]);
}

void Loader::recalculateSizesAndPositions(Size<int> bitmapSize) {
  float logoMagRatio = std::min(std::min(bitmapSize.width, logoMaxSize.width) / static_cast<float>(bitmapSize.width),
                         std::min(bitmapSize.height, logoMaxSize.height) / static_cast<float>(bitmapSize.height));
  logoSize = { static_cast<int>(bitmapSize.width * logoMagRatio), static_cast<int>(bitmapSize.height * logoMagRatio) };
  logoPosition = { (Settings::instance().viewport.width - logoSize.width) / 2, (Settings::instance().viewport.height - logoSize.height - verticalMargin - progressBarSize.height) / 2 };
  progressBarSize = { logoSize.width, progressBarSize.height };
  progressBarPosition = { (Settings::instance().viewport.width - progressBarSize.width) / 2, logoPosition.y - verticalMargin - progressBarSize.height };
}

void Loader::renderLogo(Size<int> size, Position<int> position) {
  if(logoTextureId == 0)
    return;

  float down = static_cast<float>(position.y) / Settings::instance().viewport.height * 2.0 - 1.0;
  float top = static_cast<float>(position.y + size.height) / Settings::instance().viewport.height * 2.0 - 1.0;
  float left = static_cast<float>(position.x) / Settings::instance().viewport.width * 2.0 - 1.0;
  float right = static_cast<float>(position.x + size.width) / Settings::instance().viewport.width * 2.0 - 1.0;

  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
  float texCoord[] = { 0.0f, 0.0f,    0.0f, 1.0f,
                       1.0f, 1.0f,    1.0f, 0.0f };

  glUseProgram(logoProgramObject);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, logoTextureId);
  glUniform1i(logoSamplerLoc, 0);

  glEnableVertexAttribArray(logoPosLoc);
  glVertexAttribPointer(logoPosLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
  glEnableVertexAttribArray(logoTexLoc);
  glVertexAttribPointer(logoTexLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoord);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(logoPosLoc);
  glDisableVertexAttribArray(logoTexLoc);
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
}

void Loader::renderProgressBar(Size<int> size, Position<int> position, float percent) {
  float down  = static_cast<float>(position.y) / Settings::instance().viewport.height * 2.0f - 1.0f;
  float top   = static_cast<float>(position.y + size.height) / Settings::instance().viewport.height * 2.0f - 1.0f;
  float left  = static_cast<float>(position.x) / Settings::instance().viewport.width * 2.0f - 1.0f;
  float right = static_cast<float>(position.x + size.width) / Settings::instance().viewport.width * 2.0f - 1.0f;

  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(programObject);
  glEnableVertexAttribArray(positionLoc);
  glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  glUniform2f(posLoc, static_cast<GLfloat>(position.x), static_cast<GLfloat>(position.y));
  glUniform2f(sizLoc, static_cast<GLfloat>(size.width), static_cast<GLfloat>(size.height));
  glUniform3f(fgColorLoc, 54.0f / 255.0f, 145.0f / 255.0f, 231.0f / 255.0f);
  glUniform3f(bgColorLoc, 223.0f / 255.0f, 34.0f / 255.0f, 109.0f / 255.0f);

  glUniform1f(percentLoc, percent / 100.0f);

  glUniform2f(viewportLoc, static_cast<GLfloat>(Settings::instance().viewport.width), static_cast<GLfloat>(Settings::instance().viewport.height));

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(positionLoc);
  glUseProgram(0);

  renderText(static_cast<float>(time));
}

void Loader::setLogo(int id, char* pixels, Size<int> size, GLuint format) {
  initTexture();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, logoTextureId);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, format, size.width, size.height, 0, format, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);
}

