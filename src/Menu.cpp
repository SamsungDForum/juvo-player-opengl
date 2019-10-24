#include "Menu.h"
#include "Settings.h"

Menu::Menu()
  : loader(),
    background(0.0),
    playback(),
    subtitles(),
    metrics(),
    options() {
  initialize();
}

void Menu::initialize() {
  glViewport(0, 0, Settings::viewport.first, Settings::viewport.second);

  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
  glEnable(GL_BLEND);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_STENCIL_TEST);

  backgroundOpacity = 1.0;
  loaderEnabled = true;
  selectedTile = 0;
  firstTile = 0;
}

Menu::~Menu() {
}

void Menu::render() {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  if(loaderEnabled)
    loader.render(); // render loader
  else { // render menu
    background.render(); // render background; updates background opacity
    float bgOpacity = background.getOpacity();
    if(bgOpacity > 0.0) { // render "Available content list" text
      int fontHeight = 24;
      int marginBottom = 20;
      int marginLeft = 100;
      Text::instance().render("Available content list",
                  {marginLeft, Settings::marginFromBottom + Settings::tileSize.second + marginBottom},
                  {0, fontHeight},
                  0,
                  {1.0, 1.0, 1.0, bgOpacity},
                  true);
    }
    for(size_t i = 0; i < tiles.size(); ++i) // render tiles
      if(static_cast<int>(i) != selectedTile)
        tiles[i].render();
    if(selectedTile < static_cast<int>(tiles.size()))
      tiles[selectedTile].render();
  }
  { // footer
    int fontHeight = 13;
    int margin = 5;
    int marginBottom = margin;
    int textWidth = Text::instance().getTextSize(footer, {0, fontHeight}, 0).first * Settings::viewport.first / 2.0;
    int marginLeft = Settings::viewport.first - textWidth - margin;
    Text::instance().render(footer,
                {marginLeft, marginBottom},
                {0, fontHeight},
                0,
                {1.0, 1.0, 1.0, 1.0},
                true);
  }
  { // controls/playback
    playback.render();
    subtitles.render();
    options.setOpacity(playback.getOpacity());
    options.render();
  }
  { // render metrics
    metrics.render();
  }
  { // render modal window
    modalWindow.render(0);
  }
}

void Menu::ShowMenu(int enable) {
  for(size_t i = 0; i < tiles.size(); ++i) { // let's make sure position/size parameters aren't going to be animated
    tiles[i].setPosition(getTilePosition(i - firstTile));
    tiles[i].setZoom(static_cast<int>(i) == selectedTile ? Settings::zoom : 1.0);
  }

  int animationDelay = playback.getOpacity() > 0.0 ? Settings::fadingDuration.count() * 3 / 4 : 0;
  for(size_t i = 0; i < tiles.size(); ++i)
    tiles[i].moveTo(getTilePosition(i - firstTile),
                    static_cast<int>(i) == selectedTile ? Settings::zoom : 1.0,
                    tiles[i].getSize(),
                    enable ? 1 : 0,
                    std::chrono::milliseconds(Settings::fadingDuration),
                    std::chrono::milliseconds(animationDelay));
}

std::pair<int, int> Menu::getTilePosition(int tileNo, bool initialMargin) {
  int horizontalMargin = std::max(static_cast<int>(std::ceil(static_cast<double>(Settings::viewport.first - (Settings::tileSize.first * Settings::tilesArrangement.first)) / (Settings::tilesArrangement.first + 1))), 0);
  int verticalMargin = std::max((Settings::viewport.second - (Settings::tileSize.second * Settings::tilesArrangement.second)) / (Settings::tilesArrangement.second + 1), 0);
  int horizontalPosition = 0;
  int verticalPosition = 0;
  horizontalPosition = (initialMargin ? horizontalMargin : 0) + (horizontalMargin + Settings::tileSize.first) * tileNo;
  verticalPosition = Settings::tilesArrangement.second > 1 ? Settings::tileSize.second + verticalMargin : Settings::marginFromBottom;

  int innerMargin = (Settings::viewport.first - 1.5 * Settings::sideMargin - Settings::tileSize.first * Settings::tilesArrangement.first) / (Settings::tilesArrangement.first - 1);
  horizontalPosition = Settings::sideMargin + tileNo * (Settings::tileSize.first + innerMargin);

  return std::make_pair(horizontalPosition, verticalPosition);
}

int Menu::AddTile(char *pixels, std::pair<int, int> size) {
  int tileNo = tiles.size();
  Tile tile(tiles.size(),
            getTilePosition(tileNo),
            Settings::tileSize,
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

int Menu::AddTile() {
  int tileNo = tiles.size();
  Tile tile(tiles.size(),
            getTilePosition(tileNo),
            Settings::tileSize,
            1.0,
            0.0,
            "",
            "");
  tiles.push_back(std::move(tile));
  return tiles.size() - 1;
}

void Menu::SetTileData(TileData tileData) {
  if(tileData.tileId >= static_cast<int>(tiles.size()))
    return;
  tiles[tileData.tileId].setName(tileData.name);
  tiles[tileData.tileId].setDescription(tileData.desc);
  tiles[tileData.tileId].setTexture(tileData.pixels, tileData.size, tileData.format);
}

void Menu::SetTileTexture(ImageData imageData) {
  if(imageData.id >= static_cast<int>(tiles.size()))
    return;
  tiles[imageData.id].setTexture(imageData.pixels, imageData.size, imageData.format);
}

void Menu::SelectTile(int tileNo) {
  int bounce = (Settings::bouncing && selectedTile == tileNo) ? (selectedTile == 0 ? 1 : -1) : 0;
  selectedTile = tileNo;
  bool selectedTileVisible = (firstTile <= selectedTile) && (firstTile + Settings::tilesArrangement.first - 1 >= selectedTile);
  if(!selectedTileVisible) {
    int shiftLeft = firstTile - selectedTile;
    int shiftRight = selectedTile - (firstTile + Settings::tilesArrangement.first - 1);
    if(std::abs(shiftLeft) < std::abs(shiftRight))
      firstTile -= shiftLeft;
    else
      firstTile += shiftRight;
  }
  for(size_t i = 0; i < tiles.size(); ++i)
    tiles[i].moveTo(getTilePosition(i - firstTile),
                    static_cast<int>(i) == selectedTile ? Settings::zoom : 1.0,
                    tiles[i].getTargetSize(),
                    tiles[i].getTargetOpacity(),
                    Settings::animationDuration,
                    std::chrono::milliseconds(0),
                    (static_cast<int>(i) == selectedTile && bounce) ? bounce : 0);
  if(selectedTile >= 0 && selectedTile < static_cast<int>(tiles.size()))
    background.setSourceTile(&tiles[selectedTile],
                             !bounce ? std::chrono::milliseconds(Settings::fadingDuration) : std::chrono::milliseconds(0),
                             std::chrono::milliseconds(0));
}

int Menu::AddFont(char *data, int size) {
  Text::instance().AddFont(data, size, 48);
  return 0;
}

void Menu::ShowLoader(bool enabled, int percent) {
  loaderEnabled = enabled;
  loader.setValue(percent);
}

void Menu::SetIcon(ImageData imageData) {
  playback.setIcon(imageData.id, imageData.pixels, imageData.size, imageData.format);
}

void Menu::UpdatePlaybackControls(PlaybackData playbackData) {
  playback.update(playbackData.show,
                  playbackData.state,
                  playbackData.currentTime,
                  playbackData.totalTime,
                  playbackData.text,
                  std::chrono::milliseconds(Settings::fadingDuration),
                  std::chrono::milliseconds(playbackData.show ? Settings::fadingDuration.count() * 3 / 4 : 0),
                  playbackData.buffering,
                  playbackData.bufferingPercent,
				  playbackData.seeking);
}

void Menu::SetFooter(std::string footer) {
  this->footer = footer;
}

void Menu::SwitchTextRenderingMode() {
  Text::instance().switchRenderingMode();
}

void Menu::ShowSubtitle(int duration, std::string text) {
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
