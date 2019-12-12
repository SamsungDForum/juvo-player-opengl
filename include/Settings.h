#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <utility>
#include <chrono>

class Settings {
private:
  Settings();
  ~Settings() = default;
  Settings(const Settings&) = delete;
  Settings& operator=(const Settings&) = delete;
public:
  static Settings& instance() {
    static Settings settings;
    return settings;
  }

  std::pair<int, int> viewport;
  const std::pair<int, int> tileSize;
  const std::pair<int, int> tilesArrangement;
  const bool arrangeTilesInGrid;
  const float marginFromBottom;
  const float zoom;
  const std::chrono::milliseconds fadingDuration;
  const std::chrono::milliseconds animationDuration;
  const bool bouncing;
  const float sideMargin;
  const std::chrono::milliseconds tilePreviewDelay;
  const float tilePreviewTimeScale;
};

#endif // _SETTINGS_H_
