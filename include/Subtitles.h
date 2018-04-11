#ifndef _SUBTITLES_H_
#define _SUBTITLES_H_

#include <chrono>
#include <string>

#include "Text.h"

class Subtitles {
private:
  std::pair<int, int> viewport;
  std::chrono::time_point<std::chrono::high_resolution_clock> start;
  std::chrono::milliseconds duration;
  std::string subtitle;
  bool active;

public:
  Subtitles();
  void setViewport(const std::pair<int, int> &viewport);
  void render(Text &text);
  void showSubtitle(const std::chrono::milliseconds duration, const std::string subtitle);
};

#endif // _SUBTITLES_H_
