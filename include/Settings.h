#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <utility>
#include <chrono>

class Settings {
public:
  static std::pair<int, int> viewport;
  static const std::pair<int, int> tileSize;
  static const std::pair<int, int> tilesArrangement;
  static const float marginFromBottom;
  static const float zoom;
  static const std::chrono::milliseconds fadingDuration;
  static const std::chrono::milliseconds animationDuration;
  static const bool bouncing;
  static const float sideMargin;
};

#endif // _SETTINGS_H_
