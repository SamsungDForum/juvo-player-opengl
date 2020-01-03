#include "Settings.h"

Settings::Settings()
  : viewport (std::pair<int, int>( 1920, 1080 )),
    tileSize (std::pair<int, int>( 400, 400 * viewport.second / viewport.first )),
    tilesArrangement (std::pair<int, int>( 4, 1 )),
    arrangeTilesInGrid (false),
    marginFromBottom (20),
    tileNameFontHeight(24),
    zoom (1.2f),
    animationMoveDuration (std::chrono::milliseconds(500)),
    animationZoomInDuration (std::chrono::milliseconds(300)),
    animationZoomOutDuration (std::chrono::milliseconds(500)),
    backgroundChangeDuration (std::chrono::milliseconds(400)),
    backgroundChangeDelay (std::chrono::milliseconds(500)),
    sideMargin (100),
    fadingDuration (std::chrono::milliseconds(500)),
    tilePreviewDelay (std::chrono::milliseconds(500)),
    tilePreviewTimeScale (10.0f / 3.0f) {
}
