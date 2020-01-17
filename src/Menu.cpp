#include "GLES.h"
#include "Menu.h"
#include "Settings.h"
#include "TextRenderer.h"
#include "Utility.h"

Menu::Menu()
  : loader(),
    background(),
    playback(),
    subtitles(),
    metrics(),
    options() {
  initialize();
}

void Menu::initialize() {
  Utility::setCurrentEGLContext();

  glViewport(0, 0, Settings::instance().viewport.width, Settings::instance().viewport.height);

  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
  glEnable(GL_BLEND);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_STENCIL_TEST);

  glActiveTexture(GL_TEXTURE0);

  loaderEnabled = true;
  selectedTile = -1;
  firstTile = 0;
}

Menu::~Menu() {
}

void Menu::render() {
  Utility::assertCurrentEGLContext();

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  if(loaderEnabled)
    loader.render(); // render loader
  else { // render menu
    background.render(); // render background; updates background opacity
    for(size_t i = 0; i < tiles.size(); ++i) // render tiles
      if(static_cast<int>(i) != selectedTile)
        tiles[i].render();
    if(selectedTile >= 0 && selectedTile < static_cast<int>(tiles.size()))
      tiles[selectedTile].render();
    float bgOpacity = background.getOpacity();
    if(bgOpacity >= 0.001f) { // render "Available content list" text
      int fontHeight = 24;
      int marginLeft = 100;
      TextRenderer::instance().render("Available content list",
                  {marginLeft, getGridSize().height + Settings::instance().marginFromBottom + 4},
                  {0, fontHeight},
                  0,
                  {1.0, 1.0, 1.0, bgOpacity});
    }
    { // controls/playback
      playback.render();
      subtitles.render();
      options.setOpacity(playback.getOpacity());
      options.render();
    }
  }
  { // footer
    int fontHeight = 13;
    int margin = 5;
    int textWidth = TextRenderer::instance().getTextSize(footer, { 0, static_cast<GLuint>(fontHeight) }, 0).width;
    int marginLeft = Settings::instance().viewport.width - textWidth - margin;
    TextRenderer::instance().render(footer,
                {marginLeft, Settings::instance().viewport.height - margin - fontHeight},
                {0, fontHeight},
                0,
                {1.0, 1.0, 1.0, 1.0});
  }
  { // render metrics
    metrics.render();
  }
  { // render modal window
    modalWindow.render();
  }
}

void Menu::showMenu(int enable) {
  for(size_t i = 0; i < tiles.size(); ++i) { // let's make sure position/size parameters aren't going to be animated
    tiles[i].setPosition(getTilePosition(i - firstTile));
    tiles[i].setZoom(static_cast<int>(i) == selectedTile ? Settings::instance().zoom : 1.0);
  }

  int animationDelay = playback.getOpacity() > 0.0 ? Settings::instance().fadingDuration.count() * 3 / 4 : 0;
  for(size_t i = 0; i < tiles.size(); ++i)
    tiles[i].moveTo(getTilePosition(i - firstTile),
                    static_cast<int>(i) == selectedTile ? Settings::instance().zoom : 1.0,
                    tiles[i].getSize(),
                    enable ? 1 : 0,
                    std::chrono::milliseconds(Settings::instance().fadingDuration),
                    std::chrono::milliseconds(Settings::instance().fadingDuration),
                    std::chrono::milliseconds(animationDelay));
}

Size<int> Menu::getGridSize() {
  if(tiles.empty())
    return { 0, 0 };
  int rightmostTile = std::min(static_cast<int>(tiles.size() - 1), Settings::instance().tilesArrangement.width - 1);
  return {
    Settings::instance().arrangeTilesInGrid ?
      getTilePosition(rightmostTile).x + tiles[rightmostTile].getSize().width :
      getTilePosition(tiles.size() - 1).x + tiles[tiles.size() - 1].getSize().width,
    getTilePosition(tiles.size() - 1).y + tiles[tiles.size() - 1].getSize().height
  };
}

Position<int> Menu::getTilePosition(int tileNo, bool initialMargin) {

  int verticalPosition;
  int innerMargin;
  if(Settings::instance().arrangeTilesInGrid) {
    int verticalMargin = std::max((Settings::instance().viewport.height - (Settings::instance().tileSize.height * Settings::instance().tilesArrangement.height)) / (Settings::instance().tilesArrangement.height + 1), 0);
    verticalPosition = Settings::instance().tilesArrangement.height > 1 ? Settings::instance().tileSize.height + verticalMargin : Settings::instance().marginFromBottom;
    innerMargin = (Settings::instance().viewport.width - 1.5 * Settings::instance().sideMargin - Settings::instance().tileSize.width * Settings::instance().tilesArrangement.width) / (Settings::instance().tilesArrangement.width - 1);
  }
  else {
    verticalPosition = Settings::instance().marginFromBottom;
    innerMargin = static_cast<int>(static_cast<float>(Settings::instance().tileSize.width) * (Settings::instance().zoom - 1.0f) * 0.75f);
  }
  int horizontalPosition = Settings::instance().sideMargin + tileNo * (Settings::instance().tileSize.width + innerMargin);
  return { horizontalPosition, verticalPosition + Settings::instance().tileNameFontHeight + Settings::instance().marginFromBottom };
}

int Menu::addTile(char *pixels, Size<int> size) {
  int tileNo = tiles.size();
  Tile tile(tiles.size(),
            getTilePosition(tileNo),
            Settings::instance().tileSize,
            1.0,
            0.0,
            "",
            "",
            pixels,
            size,
            GL_RGB);
  tiles.push_back(std::move(tile));
  return tiles.size() - 1;
}

int Menu::addTile() {
  int tileNo = tiles.size();
  Tile tile(tiles.size(),
            getTilePosition(tileNo),
            Settings::instance().tileSize,
            1.0,
            0.0,
            "",
            "");
  tiles.push_back(std::move(tile));
  return tiles.size() - 1;
}

void Menu::setTileData(TileData tileData) {
  if(tileData.tileId >= static_cast<int>(tiles.size()))
    return;
  tiles[tileData.tileId].setName(tileData.name);
  tiles[tileData.tileId].setDescription(tileData.desc);
  tiles[tileData.tileId].setTexture(tileData.pixels, tileData.size, tileData.format);
  tiles[tileData.tileId].setStoryboardCallback(tileData.getStoryboardData);
}

void Menu::selectTile(int tileNo, bool runPreview) {

  if(tileNo == selectedTile || tileNo < 0 || tileNo >= static_cast<int>(tiles.size()))
    return;

  selectedTile = tileNo;
  bool selectedTileVisible = (firstTile <= selectedTile) && (firstTile + Settings::instance().tilesArrangement.width - 1 >= selectedTile);
  if(!selectedTileVisible) {
    int shiftLeft = firstTile - selectedTile;
    int shiftRight = selectedTile - (firstTile + Settings::instance().tilesArrangement.width - 1);
    if(std::abs(shiftLeft) < std::abs(shiftRight))
      firstTile -= shiftLeft;
    else
      firstTile += shiftRight;
  }
  for(size_t i = 0; i < tiles.size(); ++i) {
    tiles[i].moveTo(getTilePosition(i - firstTile),
                    static_cast<int>(i) == selectedTile ? Settings::instance().zoom : 1.0,
                    tiles[i].getTargetSize(),
                    tiles[i].getTargetOpacity(),
                    Settings::instance().animationMoveDuration,
                    static_cast<int>(i) == selectedTile ? Settings::instance().animationZoomInDuration : Settings::instance().animationZoomOutDuration,
                    std::chrono::duration_values<std::chrono::milliseconds>::zero());
    tiles[i].runPreview(runPreview && static_cast<int>(i) == selectedTile);
    tiles[i].setActive(false);
  }
  if(selectedTile >= 0 && selectedTile < static_cast<int>(tiles.size())) {
    background.setSourceTile(&tiles[selectedTile]);
    tiles[selectedTile].setActive(true);
  }
}

int Menu::addFont(char *data, int size) {
  TextRenderer::instance().addFont(data, size);
  return 0;
}

void Menu::showLoader(bool enabled, int percent) {
  loaderEnabled = enabled;
  loader.setValue(percent);
}

void Menu::setIcon(ImageData imageData) {
  playback.setIcon(imageData.id, imageData.pixels, imageData.size, imageData.format);
}

void Menu::updatePlaybackControls(PlaybackData playbackData) {
  playback.update(playbackData.show,
                  playbackData.state,
                  playbackData.currentTime,
                  playbackData.totalTime,
                  playbackData.text,
                  Settings::instance().fadingDuration,
                  std::chrono::milliseconds(playbackData.show ? Settings::instance().fadingDuration.count() * 3 / 4 : 0),
                  playbackData.buffering,
                  playbackData.bufferingPercent,
				  playbackData.seeking);
}

void Menu::setFooter(std::string footer) {
  this->footer = footer;
}

void Menu::showSubtitle(int duration, std::string text) {
  subtitles.showSubtitle(std::chrono::milliseconds(duration), text);
}

bool Menu::addOption(int id, std::string name) {
  return options.addOption(id, name);
}

bool Menu::addSuboption(int parentId, int id, std::string name) {
  return options.addSuboption(parentId, id, name);
}

bool Menu::updateSelection(SelectionData selectionData) {
  return options.updateSelection(selectionData.show,
                                 selectionData.activeOptionId,
                                 selectionData.activeSubOptionId,
                                 selectionData.selectedOptionId,
                                 selectionData.selectedSubOptionId);
}

void Menu::clearOptions() {
  options.clearOptions();
}

int Menu::addGraph(GraphData graphData) {
  return metrics.addGraph(graphData.tag,
                          graphData.minVal,
                          graphData.maxVal,
                          graphData.valuesCount);
}

void Menu::setGraphVisibility(int graphId, bool visible) {
  metrics.setGraphVisibility(graphId, visible);
}

void Menu::updateGraphValues(int graphId, std::vector<float> values) {
  metrics.updateGraphValues(graphId, values);
}

void Menu::updateGraphValue(int graphId, float value) {
  metrics.updateGraphValue(graphId, value);
}

void Menu::updateGraphRange(int graphId, float minVal, float maxVal) {
  metrics.updateGraphRange(graphId, minVal, maxVal);
}

void Menu::selectAction(int id) {
  playback.selectAction(id);
}

void Menu::setLogConsoleVisibility(bool visible) {
  metrics.setLogConsoleVisibility(visible);
}

void Menu::pushLog(std::string log) {
  metrics.pushLog(log);
}


void Menu::showAlert(AlertData alertData) {
  modalWindow.show(alertData.title, alertData.body, alertData.button);
}

void Menu::hideAlert() {
  modalWindow.hide();
}

bool Menu::isAlertVisible() {
  return modalWindow.isVisible();
}
