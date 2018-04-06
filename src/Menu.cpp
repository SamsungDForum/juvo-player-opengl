#include "../include/Menu.h"

Menu::Menu(std::pair<int, int> viewport, std::pair<int, int> tileSize, std::pair<int, int> tilesNumber, float zoom, int animationsDurationMilliseconds)
  : viewport(viewport),
    tileSize(tileSize),
    tilesNumber(tilesNumber),
    zoom(zoom),
    animationsDurationMilliseconds(animationsDurationMilliseconds) {
  initialize();
}

Menu::Menu() {
  initialize();
}

void Menu::initialize() {
  glViewport(0, 0, viewport.first, viewport.second);

  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE); //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_STENCIL_TEST);

  backgroundOpacity = 1.0;
  menuEnabled = true;
  backgroundEnabled = true;
  loaderEnabled = true;
  selectedTile = 0;
  firstTile = 0;

  fpsS = 0;
  fpsN = 30;
  fpsT = std::chrono::high_resolution_clock::now();

  background.setViewport(viewport.first, viewport.second);
  background.setOpacity(0.0);
}

Menu::~Menu() {
}

void Menu::render() {
  glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  if(loaderEnabled)
    loader.render(text); // render loader
  else if(menuEnabled) { // render menu
    float bgOpacity = background.getOpacity();
    if(bgOpacity == 1.0)
      background.setClearColor({}); // set to opaque
    if(backgroundEnabled) {
      background.render(text); // render background
      if(bgOpacity > 0.0) {
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
    }
    for(size_t i = 0; i < tiles.size(); ++i) // render tiles
      if(static_cast<int>(i) != selectedTile)
        tiles[i].render(text);
    if(selectedTile < static_cast<int>(tiles.size()))
      tiles[selectedTile].render(text);
  }
  { // footer
    std::string footer = std::string("JuvoPlayer v") + version + std::string(", Samsung R&D Poland, 2018");
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
  }
  // render FPS counter
  if(fpsCounterVisible)
  {
    std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> timespan = now - fpsT;
    fpsT = now;
    float fps = 1000.0f / timespan.count();
    fpsS += fps;
    fpsV.push(fps);
    while(static_cast<int>(fpsV.size()) > fpsN) {
      fpsS -= fpsV.front();
      fpsV.pop();
    }
    fps = fpsS / (fpsV.size() ? : 1);
    {
      int fontHeight = 48;
      int margin = 12;
      text.render(std::to_string(static_cast<int>(fps)),
                  {viewport.first - margin - 100, viewport.second - fontHeight - margin},
                  {0, fontHeight},
                  viewport,
                  0,
                  {1.0, 1.0, 1.0, 1.0},
                  true);
    }
  }
}

void Menu::ShowMenu(int enable) {
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

int Menu::AddTile(char *pixels, std::pair<int, int> size)
{
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

int Menu::AddTile()
{
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

void Menu::SetTileData(int tileId, char* pixels, std::pair<int, int> size, std::string name, std::string desc)
{
  if(tileId >= static_cast<int>(tiles.size()))
    return;
  tiles[tileId].setName(name);
  tiles[tileId].setDescription(desc);
  tiles[tileId].setTexture(pixels, size, GL_RGB);
}

void Menu::SetTileTexture(int tileNo, char *pixels, std::pair<int, int> size)
{
  if(tileNo >= static_cast<int>(tiles.size()))
    return;
  tiles[tileNo].setTexture(pixels, size, GL_RGB);
}

void Menu::SelectTile(int tileNo)
{
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
  if(enabled == false) {
    loaderEnabled = false;
    background.setClearColor({0.0, 0.0, 0.0});
    ShowMenu(true);
  }
  else if(backgroundEnabled == true) {
      ShowMenu(false);
      loaderEnabled = true;
  }
  loader.setValue(percent);
}

void Menu::SetIcon(int id, char* pixels, std::pair<int, int> size) {
  playback.setIcon(id, pixels, size, GL_RGBA);
}

void Menu::UpdatePlaybackControls(int show, int state, int currentTime, int totalTime, std::string text) {
  playback.update(show,
                  state,
                  currentTime,
                  totalTime,
                  text,
                  std::chrono::milliseconds(fadingDurationMilliseconds),
                  std::chrono::milliseconds(show && menuEnabled ? fadingDurationMilliseconds * 3 / 4 : 0));
}

void Menu::SetVersion(std::string version) {
  this->version = version;
}

void Menu::SwitchTextRenderingMode() {
  text.switchRenderingMode();
}

void Menu::SwitchFPSCounterVisibility() {
  fpsCounterVisible = !fpsCounterVisible;
  _INFO("FPS counter visibility switched to %s.", fpsCounterVisible ? "visible" : "not visible");
}


