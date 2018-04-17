#include "Subtitles.h"

Subtitles::Subtitles(std::pair<int, int> viewport)
    : viewport(viewport),
      active(false) {
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
    return;
  }
  int fontHeight = 26;
  std::pair<int, int> margin = {100, 250};
  int textWidth = viewport.first - 2 * margin.first;

  std::pair<float, float> textSize = text.getTextSize(subtitle,
    {textWidth, fontHeight}, // size
    0, // fontId
    viewport); // viewport
    textSize.first *= viewport.first / 2.0f;
    textSize.second *= viewport.second / 2.0f;

  text.render(subtitle, // text string
    margin, // position
    {textWidth, fontHeight}, // size
    viewport, // viewport
    0, // fontId
    {1.0, 1.0, 1.0, 1.0}, // color
    false); // cache
}

void Subtitles::showSubtitle(const std::chrono::milliseconds duration, const std::string subtitle) {
  this->subtitle = subtitle;
  this->duration = duration;
  this->start = std::chrono::high_resolution_clock::now();
  this->active = true;
}
