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
#include "Metrics.h"
#include "Options.h"
#include "ModalWindow.h"

class Menu {
private:
  // main Menu objects and variables
  std::vector<Tile> tiles;
  bool loaderEnabled;
  int selectedTile;
  int firstTile;
  float backgroundOpacity;
  std::string footer;

  // tiles positioning and animation constants
  const std::pair<int, int> viewport;
  const std::pair<int, int> tileSize;
  const std::pair<int, int> tilesNumber;
  const int marginFromBottom;
  const float zoom;
  const int animationsDurationMilliseconds;
  const int fadingDurationMilliseconds;
  const bool bouncing;

  // UI helper objects
  Text text;
  Loader loader;
  Background background;
  Playback playback;
  Subtitles subtitles;
  Metrics metrics;
  Options options;
  ModalWindow modalWindow;

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
  void ShowSubtitle(int duration, std::string text);
  bool addOption(int id, std::string name);
  bool addSuboption(int parentId, int id, std::string name);
  bool updateSelection(bool show, int activeOptionId, int activeSuboptionId, int selectedOptionId, int selectedSuboptionId);
  void clearOptions();
  int addGraph(std::string tag, float minVal, float maxVal, int valuesCount);
  void setGraphVisibility(int graphId, bool visible);
  void updateGraphValues(int graphId, std::vector<float> values);
  void updateGraphValue(int graphId, float value);
  void updateGraphRange(int graphId, float minVal, float maxVal);
  void selectAction(int id);
  void setLogConsoleVisibility(bool visible);
  void pushLog(std::string log);
  void showAlert(std::string title, std::string text, std::string button);
  void hideAlert();
  bool isAlertVisible();
};

#endif // _MENU_H_
