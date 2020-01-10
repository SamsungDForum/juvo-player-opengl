#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <utility>
#include <chrono>

#include "Utility.h"

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

  Size<int> viewport;
  const Size<int> tileSize;
  const Size<int> tilesArrangement;
  const bool arrangeTilesInGrid;
  const int marginFromBottom;
  const int tileNameFontHeight;
  const float zoom;
  const std::chrono::milliseconds animationMoveDuration;
  const std::chrono::milliseconds animationZoomInDuration;
  const std::chrono::milliseconds animationZoomOutDuration;
  const std::chrono::milliseconds backgroundChangeDuration;
  const std::chrono::milliseconds backgroundChangeDelay;
  const float sideMargin;
  const std::chrono::milliseconds fadingDuration;
  const std::chrono::milliseconds tilePreviewDelay;
  const float tilePreviewTimeScale;
  const std::chrono::milliseconds loaderUpdateAnimationDuration;
  const std::chrono::milliseconds loaderUpdateAnimationDelay;
};

#endif // _SETTINGS_H_
