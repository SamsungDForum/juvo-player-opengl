#include "Menu.h"

Menu::Menu(std::pair<int, int> viewport, std::pair<int, int> tileSize, std::pair<int, int> tilesNumber, float zoom, int animationsDurationMilliseconds)
  : viewport(viewport),
    tileSize(tileSize),
    tilesNumber(tilesNumber),
    marginFromBottom(50),
    zoom(zoom),
    animationsDurationMilliseconds(animationsDurationMilliseconds),
    fadingDurationMilliseconds(500),
    bouncing(true),
    loader(viewport),
    background(viewport, 0.0),
    playback(viewport),
    subtitles(viewport),
    metrics(viewport),
    options(viewport) {
  initialize();
}

Menu::Menu(std::pair<int, int> viewport)
  : viewport(viewport),
    tileSize({432, 432 * viewport.second / viewport.first}),
    tilesNumber({4, 1}),
    marginFromBottom(50),
    zoom(1.05),
    animationsDurationMilliseconds(320),
    fadingDurationMilliseconds(500),
    bouncing(true),
    loader(viewport),
    background(viewport, 0.0),
    playback(viewport),
    subtitles(viewport),
    metrics(viewport),
    options(viewport) {
  initialize();
}

void Menu::initialize() {
  glViewport(0, 0, viewport.first, viewport.second);

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
    loader.render(text); // render loader
  else { // render menu
    background.render(text); // render background; updates background opacity
    float bgOpacity = background.getOpacity();
    if(bgOpacity > 0.0) { // render "Available content list" text
      int fontHeight = 24;
      int marginBottom = 20;
      int marginLeft = 100;
      text.render("Available content list",
                  {marginLeft, marginFromBottom + tileSize.second + marginBottom},
                  {0, fontHeight},
                  viewport,
                  0,
                  {1.0, 1.0, 1.0, bgOpacity},
                  true);
    }
    for(size_t i = 0; i < tiles.size(); ++i) // render tiles
      if(static_cast<int>(i) != selectedTile)
        tiles[i].render(text);
    if(selectedTile < static_cast<int>(tiles.size()))
      tiles[selectedTile].render(text);
  }
  { // footer
    int fontHeight = 13;
    int margin = 5;
    int marginBottom = margin;
    int textWidth = text.getTextSize(footer, {0, fontHeight}, 0, viewport).first * viewport.first / 2.0;
    int marginLeft = viewport.first - textWidth - margin;
    text.render(footer,
                {marginLeft, marginBottom},
                {0, fontHeight},
                viewport,
                0,
                {1.0, 1.0, 1.0, 1.0},
                true);
  }
  { // controls/playback
    playback.render(text);
    subtitles.render(text);
    options.setOpacity(playback.getOpacity());
    options.render(text);
  }
  { // render metrics
    metrics.render(text);
  }
  { // render modal window
    modalWindow.render(text, viewport, 0);
  }
}

void Menu::ShowMenu(int enable) {
  for(size_t i = 0; i < tiles.size(); ++i) { // let's make sure position/size parameters aren't going to be animated
    tiles[i].setPosition(getTilePosition(i - firstTile, tileSize, tilesNumber, viewport));
    tiles[i].setZoom(static_cast<int>(i) == selectedTile ? zoom : 1.0);
  }

  int animationDelay = playback.getOpacity() > 0.0 ? fadingDurationMilliseconds * 3 / 4 : 0;
  for(size_t i = 0; i < tiles.size(); ++i)
    tiles[i].moveTo(getTilePosition(i - firstTile, tileSize, tilesNumber, viewport),
                    static_cast<int>(i) == selectedTile ? zoom : 1.0,
                    tiles[i].getSize(),
                    enable ? 1 : 0,
                    std::chrono::milliseconds(fadingDurationMilliseconds),
                    std::chrono::milliseconds(animationDelay));
}

std::pair<int, int> Menu::getTilePosition(int tileNo, std::pair<int, int> tileSize, std::pair<int, int> tilesNumber, std::pair<int, int> viewport, bool initialMargin) {
  int horizontalMargin = std::max(static_cast<int>(std::ceil(static_cast<double>(viewport.first - (tileSize.first * tilesNumber.first)) / (tilesNumber.first + 1))), 0);
  int verticalMargin = std::max((viewport.second - (tileSize.second * tilesNumber.second)) / (tilesNumber.second + 1), 0);
  int horizontalPosition = 0;
  int verticalPosition = 0;
  horizontalPosition = (initialMargin ? horizontalMargin : 0) + (horizontalMargin + tileSize.first) * tileNo;
  verticalPosition = tilesNumber.second > 1 ? tileSize.second + verticalMargin : marginFromBottom;

  int sideMargin = 100;
  int innerMargin = (viewport.first - 1.5 * sideMargin - tileSize.first * tilesNumber.first) / (tilesNumber.first - 1);
  horizontalPosition = sideMargin + tileNo * (tileSize.first + innerMargin);

  return std::make_pair(horizontalPosition, verticalPosition);
}

int Menu::AddTile(char *pixels, std::pair<int, int> size) {
  int tileNo = tiles.size();
  Tile tile(tiles.size(),
            getTilePosition(tileNo, tileSize, tilesNumber, viewport),
            tileSize,
            viewport,
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
            getTilePosition(tileNo, tileSize, tilesNumber, viewport),
            tileSize,
            viewport,
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
  tiles[tileData.tileId].setTexture(tileData.pixels, tileData.size, GL_RGB);
}

void Menu::SetTileTexture(ImageData imageData) {
  if(imageData.id >= static_cast<int>(tiles.size()))
    return;
  tiles[imageData.id].setTexture(imageData.pixels, imageData.size, GL_RGB);
}

void Menu::SelectTile(int tileNo) {
  int bounce = (bouncing && selectedTile == tileNo) ? (selectedTile == 0 ? 1 : -1) : 0;
  selectedTile = tileNo;
  bool selectedTileVisible = (firstTile <= selectedTile) && (firstTile + tilesNumber.first - 1 >= selectedTile);
  if(!selectedTileVisible) {
    int shiftLeft = firstTile - selectedTile;
    int shiftRight = selectedTile - (firstTile + tilesNumber.first - 1);
    if(std::abs(shiftLeft) < std::abs(shiftRight))
      firstTile -= shiftLeft;
    else
      firstTile += shiftRight;
  }
  for(size_t i = 0; i < tiles.size(); ++i)
    tiles[i].moveTo(getTilePosition(i - firstTile, tileSize, tilesNumber, viewport),
                    static_cast<int>(i) == selectedTile ? zoom : 1.0,
                    tiles[i].getTargetSize(),
                    tiles[i].getTargetOpacity(),
                    std::chrono::milliseconds(animationsDurationMilliseconds),
                    std::chrono::milliseconds(0),
                    (static_cast<int>(i) == selectedTile && bounce) ? bounce : 0);
  if(selectedTile >= 0 && selectedTile < static_cast<int>(tiles.size()))
    background.setSourceTile(&tiles[selectedTile],
                             !bounce ? std::chrono::milliseconds(fadingDurationMilliseconds) : std::chrono::milliseconds(0),
                             std::chrono::milliseconds(0));
}

int Menu::AddFont(char *data, int size) {
  text.AddFont(data, size, 48);
  return 0;
}

void Menu::ShowLoader(bool enabled, int percent) {
  loaderEnabled = enabled;
  loader.setValue(percent);
}

void Menu::SetIcon(ImageData imageData) {
  playback.setIcon(imageData.id, imageData.pixels, imageData.size, GL_RGBA);
}

void Menu::UpdatePlaybackControls(PlaybackData playbackData) {
  playback.update(playbackData.show,
                  playbackData.state,
                  playbackData.currentTime,
                  playbackData.totalTime,
                  playbackData.text,
                  std::chrono::milliseconds(fadingDurationMilliseconds),
                  std::chrono::milliseconds(playbackData.show ? fadingDurationMilliseconds * 3 / 4 : 0),
                  playbackData.buffering,
                  playbackData.bufferingPercent);
}

void Menu::SetFooter(std::string footer) {
  this->footer = footer;
}

void Menu::SwitchTextRenderingMode() {
  text.switchRenderingMode();
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
