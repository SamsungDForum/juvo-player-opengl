#include "Settings.h"

Settings::Settings()
  : viewport (std::pair<int, int>( 1920, 1080 )),
    tileSize (std::pair<int, int>( 400, 400 * viewport.second / viewport.first )),
    tilesArrangement (std::pair<int, int>( 4, 1 )),
    arrangeTilesInGrid (false),
    marginFromBottom (50.0f),
    zoom (1.2f),
    fadingDuration (std::chrono::milliseconds(500)),
    animationDuration (std::chrono::milliseconds(320)),
    bouncing (true),
    sideMargin (100),
    tilePreviewDelay (std::chrono::milliseconds(500)),
    tilePreviewTimeScale (10.0f / 3.0f) {
}
