#include "Loader.h"
#include "ProgramBuilder.h"
#include "Settings.h"
#include "TextRenderer.h"
#include "Utility.h"

Loader::Loader()
  : programObject(GL_INVALID_VALUE),
    param(0) {
  initialize();
}

Loader::~Loader() {
  Utility::assertCurrentEGLContext();

  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
}

void Loader::initialize() {
  Utility::assertCurrentEGLContext();

  const GLchar* vShaderTexStr =  
#include "shaders/loader.vert"
;

  const GLchar* fShaderTexStr =  
#include "shaders/loader.frag"
;

  programObject = ProgramBuilder::buildProgram(vShaderTexStr, fShaderTexStr);

  posLoc = glGetAttribLocation(programObject, "a_position");
  timeLoc = glGetUniformLocation(programObject, "u_time");
  paramLoc = glGetUniformLocation(programObject, "u_param");
  viewportLoc = glGetUniformLocation(programObject, "u_viewport");

  time = std::chrono::steady_clock::now();
}

void Loader::setValue(int value) {
  Animation::Easing easing = animation.isActive() ? Animation::Easing::CubicOut : Animation::Easing::CubicInOut;
  animation = Animation(Settings::instance().loaderUpdateAnimationDuration,
                        Settings::instance().loaderUpdateAnimationDelay,
                        {static_cast<double>(param)},
                        {static_cast<double>(value)},
                        easing);
  param = value;
}

void Loader::render() {
  Utility::assertCurrentEGLContext();

  std::chrono::duration<float, std::milli> timespan = std::chrono::steady_clock::now() - time;

  Size<int> size = {1440, 486};
  Position<int> position = {(Settings::instance().viewport.width - size.width) / 2, (Settings::instance().viewport.height - size.height) / 2};

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
  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  GLfloat time = timespan.count() / 1000.0f;
  glUniform1f(timeLoc, time);

  float paramArg = param;
  if(animation.isActive()) {
    std::vector<double> updated = animation.update();
    if(!updated.empty())
      paramArg = static_cast<float>(updated[0]);
  }
  
  glUniform1f(paramLoc, paramArg);

  glUniform2f(viewportLoc, static_cast<GLfloat>(Settings::instance().viewport.width), static_cast<GLfloat>(Settings::instance().viewport.height));

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(posLoc);
  glUseProgram(0);

  renderText(static_cast<float>(time));
}

void Loader::renderText(float time) {
  int fontHeight = 48;
  int textLeft = (Settings::instance().viewport.width - TextRenderer::instance().getTextSize("Loading... 0%", { 0, static_cast<GLuint>(fontHeight) }, 0).width) / 2;
  int textDown = Settings::instance().viewport.height / 2 - 100;
  float d = (int)time > 0 && ((int)time + 1) % 2 == 0 ? time - (int)time : 0; // fract(time) if time is odd, otherwise 0
  float a = (cos(d * 20 / M_PI) + 1.0) / 2.0;
  TextRenderer::instance().render(std::string("Loading... ") + std::to_string(static_cast<int>(param)) + std::string("%"),
              {textLeft, textDown},
              {0, fontHeight},
              0,
              {1.0, 1.0, 1.0, a});

  fontHeight *= 1.5;
  textLeft = (Settings::instance().viewport.width - TextRenderer::instance().getTextSize("JuvoPlayer", { 0, static_cast<GLuint>(fontHeight) }, 0).width) / 2;
  textDown = Settings::instance().viewport.height / 2 + 100;

  TextRenderer::instance().render("JuvoPlayer",
              {textLeft, textDown},
              {0, fontHeight},
              0,
              {1.0, 1.0, 1.0, 1.0});
}

