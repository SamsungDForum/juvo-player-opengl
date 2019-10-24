#include "Settings.h"

std::pair<int, int> viewport = std::pair<int, int>{ 1920, 1080 };
std::pair<int, int> tileSize = std::pair<int, int>{ 432, 432 * viewport.second / viewport.first };
std::pair<int, int> tilesArrangement = std::pair<int, int>{ 4, 1 };
float marginFromBottom = 50.0f;
float zoom = 1.05f;
std::chrono::milliseconds fadingDuration = std::chrono::milliseconds(500);
std::chrono::milliseconds animationDuration = std::chrono::milliseconds(320);
bool bouncing = true;
float sideMargin = 100;

