#ifndef _MENU_H_
#define _MENU_H_

#include <chrono>
#include <cstdlib> // malloc
#include <cstring> // memcpy
#include <vector>
#include <string>
#include <utility>

#include "CommonStructs.h"
#include "Tile.h"
#include "Loader.h"
#include "Background.h"
#include "Playback.h"
#include "Subtitles.h"
#include "Metrics.h"
#include "Options.h"
#include "ModalWindow.h"
#include "Utility.h"

class Menu {
private:
  // main Menu objects and variables
  std::vector<Tile> tiles;
  bool loaderEnabled;
  int selectedTile;
  int firstTile;
  std::string footer;

  // UI helper objects
  Loader loader;
  Background background;
  Playback playback;
  Subtitles subtitles;
  Metrics metrics;
  Options options;
  ModalWindow modalWindow;

private:
  void initialize();
  int addTile(char *pixels, Size<int> size);
  Position<int> getTilePosition(int tileNo, bool initialMargin = true);
  Size<int> getGridSize();

public:
  Menu();
  ~Menu();

  void render();
  void showMenu(int enable);
  int addTile();
  void selectTile(int tileNo, bool runPreview);
  int addFont(char *data, int size);
  void showLoader(bool enabled, int percent);
  void setTileData(TileData tileData);
  void updatePlaybackControls(PlaybackData playbackData);
  void setIcon(ImageData imageData);
  void setLoaderLogo(ImageData imageData);
  void setFooter(std::string footer);
  void showSubtitle(int duration, std::string text);
  bool addOption(int id, std::string name);
  bool addSuboption(int parentId, int id, std::string name);
  bool updateSelection(SelectionData selectionData);
  void clearOptions();
  int addGraph(GraphData graphData);
  void setGraphVisibility(int graphId, bool visible);
  void updateGraphValues(int graphId, std::vector<float> values);
  void updateGraphValue(int graphId, float value);
  void updateGraphRange(int graphId, float minVal, float maxVal);
  void selectAction(int id);
  void setLogConsoleVisibility(bool visible);
  void pushLog(std::string log);
  void showAlert(AlertData alertData);
  void hideAlert();
  bool isAlertVisible();
  void setSeekPreviewCallback(StoryboardExternData (*getSeekPreviewStoryboardData)());
};

#endif // _MENU_H_
