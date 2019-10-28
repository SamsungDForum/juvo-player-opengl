#include "Subtitles.h"
#include "Settings.h"
#include "Text.h"

Subtitles::Subtitles()
    : active(false),
      showForOneFrame(false) {
}

void Subtitles::render() {
  if(!active)
    return;

  std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
  if(now > start + duration) {
    active = false;
    if(showForOneFrame == false)
      return;
    showForOneFrame = false;
  }

  int fontHeight = 26;
  std::pair<int, int> margin = {100, 150};
  int textWidth = Settings::instance().viewport.first - 2 * margin.first;

  std::pair<float, float> textSize = Text::instance().getTextSize(subtitle,
    {textWidth, fontHeight},
    0);
    textSize.first *= Settings::instance().viewport.first / 2.0f;
    textSize.second *= Settings::instance().viewport.second / 2.0f;

    Text::instance().render(subtitle,
    {(Settings::instance().viewport.first - textSize.first) / 2, margin.second + textSize.second - fontHeight},
    {textWidth, fontHeight},
    0,
    {1.0, 1.0, 1.0, 1.0},
    true);
}

void Subtitles::showSubtitle(const std::chrono::milliseconds duration, const std::string subtitle) {
  this->subtitle = subtitle;
  this->duration = duration;
  this->start = std::chrono::high_resolution_clock::now();
  this->active = true;
  if(duration == std::chrono::milliseconds(0))
    showForOneFrame = true;
}
