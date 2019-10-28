#include "Settings.h"

Settings::Settings()
  : viewport (std::pair<int, int>( 1920, 1080 )),
    tileSize (std::pair<int, int>( 432, 432 * viewport.second / viewport.first )),
    tilesArrangement (std::pair<int, int>( 4, 1 )),
    marginFromBottom (50.0f),
    zoom (1.05f),
    fadingDuration (std::chrono::milliseconds(500)),
    animationDuration (std::chrono::milliseconds(320)),
    bouncing (true),
    sideMargin (100) {
}
