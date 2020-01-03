#ifndef _SUBTITLES_H_
#define _SUBTITLES_H_

#include <chrono>
#include <string>

class Subtitles {
private:
  std::chrono::time_point<std::chrono::steady_clock> start;
  std::chrono::milliseconds duration;
  std::string subtitle;
  bool active;
  bool showForOneFrame;

public:
  Subtitles();
  void render();
  void showSubtitle(const std::chrono::milliseconds duration, const std::string subtitle); // duration == 0 means "show it just for next frame"
};

#endif // _SUBTITLES_H_
