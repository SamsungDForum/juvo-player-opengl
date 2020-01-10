#include "Subtitles.h"
#include "Settings.h"
#include "TextRenderer.h"

Subtitles::Subtitles()
    : active(false),
      showForOneFrame(false) {
}

void Subtitles::render() {
  if(!active)
    return;

  if(std::chrono::steady_clock::now() > start + duration) {
    active = false;
    if(showForOneFrame == false)
      return;
    showForOneFrame = false;
  }

  int fontHeight = 26;
  Size<int> margin = {100, 150};
  int textWidth = Settings::instance().viewport.width - 2 * margin.width;

  Size<GLuint> textSize = TextRenderer::instance().getTextSize(subtitle,
    { static_cast<GLuint>(textWidth), static_cast<GLuint>(fontHeight) },
    0);

    TextRenderer::instance().render(subtitle, {
      static_cast<int>((Settings::instance().viewport.width - textSize.width) / 2),
      static_cast<int>(margin.height + textSize.height - fontHeight)
    },
    {textWidth, fontHeight},
    0,
    {1.0, 1.0, 1.0, 1.0});
}

void Subtitles::showSubtitle(const std::chrono::milliseconds duration, const std::string subtitle) {
  this->subtitle = subtitle;
  this->duration = duration;
  this->start = std::chrono::steady_clock::now();
  this->active = true;
  if(duration == std::chrono::milliseconds(0))
    showForOneFrame = true;
}
