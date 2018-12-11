#include "Subtitles.h"

Subtitles::Subtitles(std::pair<int, int> viewport)
    : viewport(viewport),
      active(false),
      showForOneFrame(false) {
}

void Subtitles::setViewport(const std::pair<int, int> &viewport) {
  this->viewport = viewport;
}

void Subtitles::render(Text &text) {
  if(!active)
    return;
  std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
  if(now > start + duration) {
    active = false;
    if(showForOneFrame == false)
      return;
    else
      showForOneFrame = false;
  }
  int fontHeight = 26;
  std::pair<int, int> margin = {100, 150};
  int textWidth = viewport.first - 2 * margin.first;

  std::pair<float, float> textSize = text.getTextSize(subtitle,
    {textWidth, fontHeight},
    0,
    viewport);
    textSize.first *= viewport.first / 2.0f;
    textSize.second *= viewport.second / 2.0f;

  text.render(subtitle,
    {(viewport.first - textSize.first) / 2, margin.second + textSize.second - fontHeight},
    {textWidth, fontHeight},
    viewport,
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
