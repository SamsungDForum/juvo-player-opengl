#ifndef _MENU_H_
#define _MENU_H_

#include <cmath>
#include <chrono>
#include <cstdlib> // malloc
#include <cstring> // memcpy
#include <vector>

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

#include "Tile.h"
#include "Text.h"
#include "Loader.h"
#include "Background.h"
#include "Playback.h"
#include "Subtitles.h"
#include "Framerate.h"

class Menu {
private:
  // main Menu objects and variables
  std::vector<Tile> tiles;
  bool loaderEnabled;
  bool backgroundEnabled;
  bool menuEnabled;
  int selectedTile;
  int firstTile;
  float backgroundOpacity;
  bool fpsCounterVisible = false;
  std::string footer = "JuvoPlayer prealpha, OpenGL UI, Samsung R&D Poland, 2017-2018";

  // tiles positioning and animation constants
  const std::pair<int, int> viewport {1920, 1080};
  const std::pair<int, int> tileSize {432, 432 * viewport.second / viewport.first};
  const std::pair<int, int> tilesNumber {4, 1};
  const int marginFromBottom = 50;
  const float zoom = 1.05;
  const int animationsDurationMilliseconds = 320;
  const int fadingDurationMilliseconds = 500;
  const bool bouncing = true;


  // UI helper objects
  Text text;
  Loader loader;
  Background background;
  Playback playback;
  Subtitles subtitles;
  Framerate framerate;

private:
  void initialize();
  int AddTile(char *pixels, std::pair<int, int> size);
  std::pair<int, int> getTilePosition(int tileNo, std::pair<int, int> tileSize, std::pair<int, int> tilesNumber, std::pair<int, int> viewport, bool initialMargin = true);

public:
  Menu(std::pair<int, int> viewport);
  Menu(std::pair<int, int> viewport, std::pair<int, int> tileSize, std::pair<int, int> tilesNumber, float zoom, int animationsDurationMilliseconds);
  ~Menu();

  void render();
  void ShowMenu(int enable);
  int AddTile();
  void SetTileTexture(int tileNo, char *pixels, std::pair<int, int> size);
  void SelectTile(int tileNo);
  int AddFont(char *data, int size);
  void ShowLoader(bool enabled, int percent);
  void SetTileData(int tileId, char* pixels, std::pair<int, int> size, std::string name, std::string desc);
  void UpdatePlaybackControls(int show, int state, int currentTime, int totalTime, std::string text);
  void SetIcon(int id, char* pixels, std::pair<int, int> size);
  void SetFooter(std::string footer);
  void SwitchTextRenderingMode();
  void SwitchFPSCounterVisibility();
  void ShowSubtitle(int duration, std::string text);
};

#endif // _MENU_H_
