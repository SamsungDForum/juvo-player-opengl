#include "Playback.h"
#include "ProgramBuilder.h"
#include "Settings.h"
#include "TextRenderer.h"
#include "Utility.h"

Playback::Playback()
  : barProgramObject(GL_INVALID_VALUE),
    iconProgramObject(GL_INVALID_VALUE),
    icons(std::vector<GLuint>(static_cast<int>(Icon::LENGTH), 0)),
    enabled(false),
    state(State::Idle),
    currentTime(0),
    totalTime(0),
    displayText("Loading..."),
    opacity(0.0f),
    selectedAction(Action::None),
    progress(0.0f),
    buffering(false),
    bufferingPercent(0.0f),
    seeking(false),
    lastUpdate(std::chrono::steady_clock::now()),
    progressUiLineLevel(100),
    progressBarSizePx({1400, 20}),
    progressBarSize({
        static_cast<int>(progressBarSizePx.width / 1920.0f * Settings::instance().viewport.width),
        static_cast<int>(progressBarSizePx.height / 1080.0f * Settings::instance().viewport.height)}),
    progressBarMarginBottom(progressUiLineLevel / 1080.0f * Settings::instance().viewport.height - progressBarSize.height / 2.0f),
    dotScale(1.5f),
    iconSize({64, 64}) {
  initialize();
}

Playback::~Playback() {
  Utility::assertCurrentEGLContext();

  if(barProgramObject != GL_INVALID_VALUE)
    glDeleteProgram(barProgramObject);
  if(iconProgramObject != GL_INVALID_VALUE)
    glDeleteProgram(iconProgramObject);
  for(size_t i = 0; i < icons.size(); ++i)
    if(icons[i] > 0) {
      glDeleteTextures(1, &icons[i]);
      icons[i] = 0;
    }
}

void Playback::initialize() {
  Utility::assertCurrentEGLContext();

  const GLchar* barVShaderTexStr = 
#include "shaders/playbackBar.vert"
;

  const GLchar* barFShaderTexStr =  
#include "shaders/playbackBar.frag"
;

  barProgramObject = ProgramBuilder::buildProgram(barVShaderTexStr, barFShaderTexStr);

  posBarLoc = glGetAttribLocation(barProgramObject, "a_position");
  paramBarLoc = glGetUniformLocation(barProgramObject, "u_param");
  opacityBarLoc = glGetUniformLocation(barProgramObject, "u_opacity");
  viewportBarLoc = glGetUniformLocation(barProgramObject, "u_viewport");
  sizeBarLoc = glGetUniformLocation(barProgramObject, "u_size");
  marginBarLoc = glGetUniformLocation(barProgramObject, "u_margin");
  dotScaleBarLoc = glGetUniformLocation(barProgramObject, "u_dot_scale");

  const GLchar* iconVShaderTexStr =
#include "shaders/playbackIcon.vert"
;

  const GLchar* iconFShaderTexStr =
#include "shaders/playbackIcon.frag"
;

  iconProgramObject = ProgramBuilder::buildProgram(iconVShaderTexStr, iconFShaderTexStr);

  samplerIconLoc = glGetUniformLocation(iconProgramObject, "s_texture");
  texCoordIconLoc = glGetAttribLocation(iconProgramObject, "a_texCoord");
  posIconLoc = glGetAttribLocation(iconProgramObject, "a_position");
  colIconLoc = glGetUniformLocation(iconProgramObject, "u_color");
  shadowColIconLoc = glGetUniformLocation(iconProgramObject, "u_shadowColor");
  shadowOffIconLoc = glGetUniformLocation(iconProgramObject, "u_shadowOffset");
  opacityIconLoc = glGetUniformLocation(iconProgramObject, "u_opacity");
  colBloomIconLoc = glGetUniformLocation(iconProgramObject, "u_bloomColor");
  opaBloomIconLoc = glGetUniformLocation(iconProgramObject, "u_bloomOpacity");
  rectBloomIconLoc = glGetUniformLocation(iconProgramObject, "u_bloomRect");

  const GLchar* loaderVShaderTexStr =
#include "shaders/playbackLoader.vert"
;

  const GLchar* loaderFShaderTexStr =
#include "shaders/playbackLoader.frag"
;

  loaderProgramObject = ProgramBuilder::buildProgram(loaderVShaderTexStr, loaderFShaderTexStr);

  posLoaderLoc = glGetAttribLocation(loaderProgramObject, "a_position");
  paramLoaderLoc = glGetUniformLocation(loaderProgramObject, "u_param");
  opacityLoaderLoc = glGetUniformLocation(loaderProgramObject, "u_opacity");
  viewportLoaderLoc = glGetUniformLocation(loaderProgramObject, "u_viewport");
  sizeLoaderLoc = glGetUniformLocation(loaderProgramObject, "u_size");
}

void Playback::updateProgress() {
  if(progressAnimation.isActive()) {
    std::vector<double> updates = progressAnimation.update();
    if(!updates.empty())
      progress = static_cast<float>(updates[0]);
  }
  else if(totalTime) {
    progress = static_cast<float>(currentTime) / static_cast<float>(totalTime);
  }
  else
    progress = 0.0;
}

void Playback::render() {
  std::vector<double> updated = opacityAnimation.update();
  if(!updated.empty())
    opacity = static_cast<float>(updated[0]);
  if(opacity > 0.0) {
    updateProgress();
    renderProgressBar(opacity);
    renderIcons(opacity);
    renderText(opacity);
  }
  if((state == State::Idle && opacity > 0.0) || (state == State::Paused && buffering) || seeking)
    renderLoader(1.0);
}

void Playback::renderIcons(float opacity) {
  Icon icon = Icon::Play;
  std::vector<float> color = {1.0, 1.0, 1.0, 1.0};
  Position<int> position = {200, progressUiLineLevel};
  switch(state) {
    case State::Playing:
      icon = Icon::Pause;
      break;
    case State::Paused:
      if(buffering)
        color = {0.5, 0.5, 0.5, 0.5};
      break;
    case State::Prepared:
      break;
    default:
      color = {0.5, 0.5, 0.5, 0.5};
      break;
  }
  renderIcon(icon, position, iconSize, color, opacity, selectedAction == Action::PlaybackControl);
  //renderIcon(Icon::Options, {Settings::instance().viewport.width - 75, Settings::instance().viewport.height - 75}, iconSize, {1.0, 1.0, 1.0, 1.0}, opacity, selectedAction == Action::OptionsMenu); // It's not being used right now since SelectAction is not being used.
}

void Playback::renderIcon(Icon icon, Position<int> position, Size<int> size, std::vector<float> color, float opacity, bool bloom) {
  Utility::assertCurrentEGLContext();

  if(static_cast<int>(icon) >= static_cast<int>(icons.size()) || icons[static_cast<int>(icon)] == 0)
    return;

  float leftPx = position.x - size.width / 2.0;
  float rightPx = leftPx + size.width;
  float downPx = position.y - size.height / 2.0;
  float topPx = downPx + size.height;
  float left = (leftPx / Settings::instance().viewport.width) * 2.0 - 1.0;
  float right = (rightPx / Settings::instance().viewport.width) * 2.0 - 1.0;
  float down = (downPx / Settings::instance().viewport.height) * 2.0 - 1.0;
  float top = (topPx / Settings::instance().viewport.height) * 2.0 - 1.0;

  GLfloat vertices[] = { left,   top,  0.0f,
                         left,   down, 0.0f,
                         right,  down, 0.0f,
                         right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(iconProgramObject);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, icons[static_cast<int>(icon)]);
  glUniform1i(samplerIconLoc, 0);

  float tex[] = { 0.0f, 0.0f,    0.0f, 1.0f,
                  1.0f, 1.0f,    1.0f, 0.0f };
                       
  glEnableVertexAttribArray(texCoordIconLoc);
  glVertexAttribPointer(texCoordIconLoc, 2, GL_FLOAT, GL_FALSE, 0, tex);

  glEnableVertexAttribArray(posIconLoc);
  glVertexAttribPointer(posIconLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

  if(color.size() >= 3)
    glUniform3f(colIconLoc, color[0], color[1], color[2]);
  else
    glUniform3f(colIconLoc, 1.0f, 1.0f, 1.0f);
  glUniform1f(opacityIconLoc, static_cast<GLfloat>(opacity));
  glUniform3f(shadowColIconLoc, 0.0f, 0.0f, 0.0f);
  glUniform2f(shadowOffIconLoc, -1.0f / size.width, -1.0f / size.height);

  if(color.size() >= 3)
    glUniform3f(colBloomIconLoc, color[0], color[1], color[2]);
  else
    glUniform3f(colBloomIconLoc, 1.0f, 1.0f, 1.0f);
  glUniform1f(opaBloomIconLoc, bloom ? opacity : 0.0f);
  glUniform4f(rectBloomIconLoc, position.x, position.y, size.width, size.height);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(texCoordIconLoc);
  glDisableVertexAttribArray(posIconLoc);
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
}

void Playback::renderText(float opacity) {
  // render remaining time
  int fontHeight = 24;
  int textLeft = Settings::instance().viewport.width - (Settings::instance().viewport.width - progressBarSize.width) / 2 + progressBarSize.height;
  int textDown = progressBarMarginBottom + progressBarSize.height / 2 - fontHeight / 2;
  TextRenderer::instance().render(timeToString(-1 * (totalTime - currentTime)),
              {textLeft, textDown},
              {0, fontHeight},
              0,
              {1.0, 1.0, 1.0, opacity});

  // render title
  fontHeight = 48;
  textLeft = 100;
  textDown = Settings::instance().viewport.height - fontHeight - 100;
  TextRenderer::instance().render(displayText,
              {textLeft, textDown},
              {Settings::instance().viewport.width - textLeft * 2, fontHeight},
              0,
              {1.0, 1.0, 1.0, opacity});
}

void Playback::renderProgressBar(float opacity) {
  Utility::assertCurrentEGLContext();

  float marginHeightScale = 1.5; // the dot is 1.25x
  float down = static_cast<float>(progressBarMarginBottom + progressBarSize.height / 2 - marginHeightScale * progressBarSize.height / 2) / Settings::instance().viewport.height * 2.0f - 1.0f;
  float top = static_cast<float>(progressBarMarginBottom + progressBarSize.height / 2 + marginHeightScale * progressBarSize.height / 2) / Settings::instance().viewport.height * 2.0f - 1.0f;
  float left = static_cast<float>(0.9f * (Settings::instance().viewport.width - progressBarSize.width) / 2) / Settings::instance().viewport.width * 2.0f - 1.0f;
  float right = static_cast<float>(Settings::instance().viewport.width - (0.9f * (Settings::instance().viewport.width - progressBarSize.width) / 2)) / Settings::instance().viewport.width * 2.0f - 1.0f;
  GLfloat vertices[] = { left,   top,  0.0f,
                         left,   down, 0.0f,
                         right,  down, 0.0f,
                         right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(barProgramObject);
  glEnableVertexAttribArray(posBarLoc);
  glVertexAttribPointer(posBarLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

  glUniform1f(paramBarLoc, clamp<float>(progress, 0.0, 1.0));
  glUniform1f(opacityBarLoc, opacity);
  glUniform2f(viewportBarLoc, Settings::instance().viewport.width, Settings::instance().viewport.height);
  glUniform2f(sizeBarLoc, progressBarSize.width, progressBarSize.height);
  glUniform1f(marginBarLoc, progressBarMarginBottom);
  glUniform1f(dotScaleBarLoc, dotScale);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(posBarLoc);
  glUseProgram(0);
}

void Playback::initTexture(int id) {
  Utility::assertCurrentEGLContext();

  if(id >= static_cast<int>(icons.size()))
    return;
  glGenTextures(1, &icons[id]);
}

void Playback::setIcon(int id, char* pixels, Size<int> size, GLuint format) {
  Utility::assertCurrentEGLContext();

  if(id >= static_cast<int>(icons.size()))
   return; 
  if(icons[id] == 0)
    initTexture(id);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, icons[id]);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, format, size.width, size.height, 0, format, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void Playback::update(int show, int state, int currentTime, int totalTime, std::string text, std::chrono::milliseconds animationDuration, std::chrono::milliseconds animationDelay, bool buffering, float bufferingPercent, bool seeking) {
  updateProgress();
  std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
  std::chrono::milliseconds fromLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate);
  if(static_cast<bool>(show) != enabled) {
    enabled = static_cast<bool>(show);
    opacityAnimation = Animation(animationDuration,
                          animationDelay,
                          {static_cast<double>(opacity)},
                          {enabled ? 1.0 : 0.0},
                          opacityAnimation.isActive() ? Animation::Easing::CubicOut : Animation::Easing::CubicInOut);
  }

  if(currentTime != this->currentTime && totalTime != 0) { // excluding totalTime=0 because of live content case
    lastUpdate = now;
    progressAnimation = Animation(std::min(std::chrono::milliseconds(std::chrono::duration_cast<std::chrono::milliseconds>(fromLastUpdate).count() * 2), std::chrono::milliseconds(1000)),
                          std::chrono::duration_values<std::chrono::milliseconds>::zero(),
                          {progress},
                          {static_cast<double>(currentTime) / static_cast<double>(totalTime ? totalTime : currentTime)},
                          Animation::Easing::Linear);
  }

  this->state = static_cast<State>(state);
  this->totalTime = max<int>(0, totalTime);
  this->currentTime = clamp<int>(currentTime, 0, totalTime);
  displayText = text;
  this->buffering = buffering;
  this->bufferingPercent = bufferingPercent;
  this->seeking = seeking;
}

std::string Playback::timeToString(int time) {
  bool negative = time < 0;
  time = abs(time) / 1000;
  int h = time / 3600;
  int m = (time % 3600) / 60;
  int s = time % 60;
  return std::string(negative ? "-" : "")
       + (h ? std::to_string(h) : "") // hours
       + (h ? ":" : "") // h:m colon
       + (m < 10 ? "0" : "") // m leading 0
       + (std::to_string(m)) // minutes
       + (":") // m:s colon
       + (s < 10 ? "0" : "") // s leading 0
       + std::to_string(s); // seconds
}

void Playback::renderLoader(float opacity) {
  Utility::assertCurrentEGLContext();

  int squareWidth = 200;
  float down = static_cast<float>((Settings::instance().viewport.height - squareWidth) / 2) / Settings::instance().viewport.height * 2.0f - 1.0f;
  float top = static_cast<float>((Settings::instance().viewport.height + squareWidth) / 2) / Settings::instance().viewport.height * 2.0f - 1.0f;
  float left = static_cast<float>((Settings::instance().viewport.width - squareWidth) / 2) / Settings::instance().viewport.width * 2.0f - 1.0f;
  float right = static_cast<float>((Settings::instance().viewport.width + squareWidth) / 2) / Settings::instance().viewport.width * 2.0f - 1.0f;
  GLfloat vertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(loaderProgramObject);
  glEnableVertexAttribArray(posLoaderLoc);
  glVertexAttribPointer(posLoaderLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

  glUniform1f(paramLoaderLoc, fmod(static_cast<double>(std::clock()) / CLOCKS_PER_SEC, 1.0));
  glUniform1f(opacityLoaderLoc, opacity);
  glUniform2f(viewportLoaderLoc, Settings::instance().viewport.width, Settings::instance().viewport.height);
  glUniform2f(sizeLoaderLoc, squareWidth, squareWidth);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(posLoaderLoc);
  glUseProgram(0);
}

void Playback::selectAction(int id) {
  selectedAction = static_cast<Action>(id);
}

