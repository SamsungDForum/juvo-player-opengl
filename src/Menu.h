#ifndef _MENU_H_
#define _MENU_H_

#include <cmath>
#include <chrono>
#include <cstdlib> // malloc
#include <cstring> // memcpy
#include <vector>
#include <queue>
#include <chrono>

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

#define WIDE_MARGIN

class Menu {
private:
  std::vector<Tile> tiles;
  std::vector<Tile> bgTiles;
  GLuint programObject;
  bool loaderEnabled;
  bool backgroundEnabled;
  bool renderMenu;
  int selectedTile;
  int firstTile;
  float backgroundOpacity;

  const int viewportWidth = 1920;
  const int viewportHeight = 1080;
#ifdef WIDE_MARGIN
  const int tileWidth = 432;// 450;
#else
  const int tileWidth = 450;
#endif
  const int tileHeight = tileWidth * viewportHeight / viewportWidth; // 253;
  const int tilesHorizontal = 4;
  const int tilesVertical = 1;
  const int marginFromBottom = 50;
#ifdef WIDE_MARGIN
  const float zoom = 1.05;
#else
  const float zoom = 1.1;
#endif
  const int animationsDurationMilliseconds = 320;
  const int fadingDurationMilliseconds = 500;
  const bool bouncing = true;

  std::chrono::time_point<std::chrono::high_resolution_clock> fpsT;
  float fpsS;
  int fpsN;
  std::queue<float> fpsV;

  Text text;
  Loader loader;
  Background background;
  Playback playback;

  std::string version = "0.0.1 prealpha";

  void initialize();

public:
  Menu();
  Menu(int viewportWidth, int viewportHeight, int tileWidth, int tileHeight, int tilesHorizontal, int tilesVertical, float zoom, int animationsDurationMilliseconds);
  ~Menu();

  void render();
  int AddBackground(char *pixels, int width, int height);
  void ShowMenu(int enable);
  std::pair<int, int> getTilePosition(int tileNo, int tileWidth, int tileHeight, int tilesHorizontal, int tilesVertical, int viewportWidth = 1920, int viewportHeight = 1080, bool initialMargin = 1);
  int AddTile(char *pixels, int width, int height);
  int AddTile();
  void SetTileTexture(int tileNo, char *pixels, int width, int height);
  void SelectTile(int tileNo);
  void FullscreenTile(bool fullscreen);
  int AddFont(char *data, int size);
  void ShowLoader(bool enabled, int percent);
  void SetTileData(int tileId, char* pixels, int w, int h, std::string name, std::string desc);
  void SetIcon(int id, char* pixels, int w, int h);
  void UpdatePlaybackControls(int show, int state, int currentTime, int totalTime, std::string text);
  void SetVersion(std::string version);
};

Menu::Menu(int viewportWidth, int viewportHeight, int tileWidth, int tileHeight, int tilesHorizontal, int tilesVertical, float zoom, int animationsDurationMilliseconds)
          : viewportWidth(viewportWidth),
          viewportHeight(viewportHeight),
          tileWidth(tileWidth),
          tileHeight(tileHeight),
          tilesHorizontal(tilesHorizontal),
          tilesVertical(tilesVertical),
          zoom(zoom),
          animationsDurationMilliseconds(animationsDurationMilliseconds) {
  initialize();
}

Menu::Menu() {
  initialize();
}

void Menu::initialize() {
  /*
  const GLchar* vShaderTexStr =  
    "attribute vec4 a_position;     \n"
    "attribute vec2 a_texCoord;     \n"
    "varying vec2 v_texCoord;       \n"
    "void main()                    \n"
    "{                              \n"
    "   v_texCoord = a_texCoord;    \n"
    "   gl_Position = a_position;   \n"
    "}                              \n";
 
  const GLchar* fShaderTexStr =  
    "precision highp float;       \n"
//  "uniform vec4 u_color;          \n"
    "varying vec2 v_texCoord;       \n"
    "uniform sampler2D s_texture;   \n"
    "uniform float u_opacity;       \n"
    "void main()                    \n"
    "{                              \n"
    "   gl_FragColor                \n"
    "    = texture2D(s_texture,     \n"
    "                v_texCoord);   \n"
    "   gl_FragColor.a = u_opacity; \n"
    "}                              \n";

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vShaderTexStr, NULL);
  glCompileShader(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fShaderTexStr, NULL);
  glCompileShader(fragmentShader);

  programObject = glCreateProgram();
  glAttachShader(programObject, vertexShader);
  glAttachShader(programObject, fragmentShader);
  glLinkProgram(programObject);

  glBindAttribLocation(programObject, 0, "a_position");
  */
  glViewport(0, 0, viewportWidth, viewportHeight);

  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
  glEnable(GL_BLEND);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_STENCIL_TEST);

  backgroundOpacity = 1.0;
  renderMenu = true;
  backgroundEnabled = true;
  loaderEnabled = true;
  selectedTile = 0;
  firstTile = 0;

  fpsS = 0;
  fpsN = 30;
  fpsT = std::chrono::high_resolution_clock::now();

  background.setViewport(viewportWidth, viewportHeight);
  background.setOpacity(0.0);
}

Menu::~Menu() {
}

void Menu::render() {
  glClearColor(0.2f, 0.2f, 0.2f, /*backgroundEnabled ? 1.0f : 0.0f*/ 0.0f);
  glClear(GL_COLOR_BUFFER_BIT/* | GL_DEPTH_BUFFER_BIT*/);

//  glUseProgram(programObject);

  if(loaderEnabled)
    loader.render(text);
  else if(renderMenu) {
    float bgOpacity = background.getOpacity();
    if(bgOpacity == 1.0)
      background.setClearColor({}); // opaque
    if(backgroundEnabled) {
      background.render(text);
#ifdef WIDE_MARGIN
      if(bgOpacity > 0.0) {
        std::pair<int, int> viewport = {1920, 1080};
        int fontHeight = 24;
        int marginBottom = 20;
        int marginLeft = 100;
        text.render("Popular in VD", {marginLeft, marginFromBottom + tileHeight + marginBottom}, {0, fontHeight}, viewport, 0, {1.0, 1.0, 1.0, bgOpacity}, true);
      }
#endif
    }
    
    for(size_t i = 0; i < tiles.size(); ++i)
      if(static_cast<int>(i) != selectedTile)
        tiles[i].render(text);
    if(selectedTile < static_cast<int>(tiles.size()))
      tiles[selectedTile].render(text);
  }
  { // footer
    std::pair<int, int> viewport = {1920, 1080};
    std::string footer = std::string("JuvoPlayer v") + version + std::string(", Samsung R&D Poland, 2018");
    int fontHeight = 10;
    int margin = 10;
    int marginBottom = margin;
    int textWidth = text.getTextSize(footer, {0, fontHeight}, 0, viewport).first * viewport.first / 2.0;
    int marginLeft = 1920 - textWidth - margin;
    text.render(footer, {marginLeft, marginBottom}, {0, fontHeight}, viewport, 0, {1.0, 1.0, 1.0, 1.0}, true);
  }
  { // controls/playback
    playback.render(text);
  }

  // FPS rendering
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
      std::pair<int, int> viewport = {1920, 1080};
      int fontHeight = 48;
      int margin = 12;
      text.render(std::to_string(static_cast<int>(fps)), {viewport.first - margin - 100, viewport.second - fontHeight - margin}, {0, fontHeight}, viewport, 0, {1.0, 1.0, 1.0, 1.0}, true);
    }

//    text.render("jX{0}/abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstulllllllvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890", {100, 1080 / 2}, {1720, 96}, {1920, 1080}, 0, {1.0, 1.0, 1.0, 1.0}, true);
  }
//  glUseProgram(0);

}

int Menu::AddBackground(char *pixels, int width, int height)
{
  if(bgTiles.empty())
  {
    Tile tile(tiles.size(),
              {0, 0},
              {viewportWidth, viewportHeight},
              {viewportWidth, viewportHeight},
              1.0,
              0.0, //backgroundOpacity,
              "",
              "",
              pixels,
              {width, height},
              GL_RGB);
    bgTiles.push_back(std::move(tile));
  }
  else {
    bgTiles[0].setTexture(pixels, width, height, GL_RGB);
  }
  return bgTiles.size() - 1;
}

void Menu::ShowMenu(int enable) {
  //backgroundEnabled = enable;
  //renderMenu = enable;
  int animationDelay = playback.getOpacity() > 0.0 ? fadingDurationMilliseconds * 3 / 4 : 0;
  if(bgTiles.size() > 0)
    bgTiles[0].moveTo(bgTiles[0].getPosition(), bgTiles[0].getZoom(), bgTiles[0].getSize(), enable ? backgroundOpacity : 0.0, std::chrono::milliseconds(fadingDurationMilliseconds), std::chrono::milliseconds(animationDelay));
  for(size_t i = 0; i < tiles.size(); ++i)
    tiles[i].moveTo(getTilePosition(i - firstTile, tileWidth, tileHeight, tilesHorizontal, tilesVertical, viewportWidth, viewportHeight), static_cast<int>(i) == selectedTile ? zoom : 1.0, tiles[i].getSize(), enable ? 1 : 0, std::chrono::milliseconds(fadingDurationMilliseconds), std::chrono::milliseconds(animationDelay));
}

std::pair<int, int> Menu::getTilePosition(int tileNo, int tileWidth, int tileHeight, int tilesHorizontal, int tilesVertical, int viewportWidth, int viewportHeight, bool initialMargin) {
  int horizontalMargin = std::max(static_cast<int>(std::ceil(static_cast<double>(viewportWidth - (tileWidth * tilesHorizontal)) / (tilesHorizontal + 1))), 0);
  int verticalMargin = std::max((viewportHeight - (tileHeight * tilesVertical)) / (tilesVertical + 1), 0);
  int horizontalPosition = 0;
  int verticalPosition = 0;
  horizontalPosition = (initialMargin ? horizontalMargin : 0) + (horizontalMargin + tileWidth) * tileNo;
  verticalPosition = tilesVertical > 1 ? tileHeight + verticalMargin : marginFromBottom;

#ifdef WIDE_MARGIN
  int sideMargin = 100;
  int innerMargin = (viewportWidth - 1.5 * sideMargin - tileWidth * tilesHorizontal) / (tilesHorizontal - 1);
  horizontalPosition = sideMargin + tileNo * (tileWidth + innerMargin);
#endif

/*  if(selectedTile >= 0 && selectedTile < static_cast<int>(tiles.size()))
    background.setSourceTile(&tiles[selectedTile], std::chrono::milliseconds(fadingDurationMilliseconds), std::chrono::milliseconds(0));
*/
  return std::make_pair(horizontalPosition, verticalPosition);
}

int Menu::AddTile(char *pixels, int width, int height)
{
  int tileNo = tiles.size();
  Tile tile(tiles.size(),
            getTilePosition(tileNo, tileWidth, tileHeight, tilesHorizontal, tilesVertical, viewportWidth, viewportHeight),
            {tileWidth, tileHeight},
            {viewportWidth, viewportHeight},
            1.0,
            0.0, //1.0,
            std::string("Tile") + std::to_string(tiles.size()),
            std::string("Lorem ipsum dolor sit amet."),
            pixels,
            {width, height},
            GL_RGB);
  tiles.push_back(std::move(tile));
  return tiles.size() - 1;
}

int Menu::AddTile()
{
  int tileNo = tiles.size();
  Tile tile(tiles.size(),
            getTilePosition(tileNo, tileWidth, tileHeight, tilesHorizontal, tilesVertical, viewportWidth, viewportHeight),
            {tileWidth, tileHeight},
            {viewportWidth, viewportHeight},
            1.0,
            0.0, //1.0,
            std::string("Tile") + std::to_string(tiles.size()),
            std::string("Lorem ipsum dolor sit amet."));
  tiles.push_back(std::move(tile));
  return tiles.size() - 1;
}

void Menu::SetTileData(int tileId, char* pixels, int w, int h, std::string name, std::string desc)
{
  if(tileId >= static_cast<int>(tiles.size()))
    return;
  tiles[tileId].setName(name);
  tiles[tileId].setDescription(desc);
  tiles[tileId].setTexture(pixels, w, h, GL_RGB);
}

void Menu::SetTileTexture(int tileNo, char *pixels, int width, int height)
{
  if(tileNo >= static_cast<int>(tiles.size()))
    return;
  tiles[tileNo].setTexture(pixels, width, height, GL_RGB);
}

void Menu::SelectTile(int tileNo)
{
  int bounce = (bouncing && selectedTile == tileNo) ? (selectedTile == 0 ? 1 : -1) : 0;
  selectedTile = tileNo;
  bool selectedTileVisible = (firstTile <= selectedTile) && (firstTile + tilesHorizontal - 1 >= selectedTile);
  if(!selectedTileVisible) {
    int shiftLeft = firstTile - selectedTile;
    int shiftRight = selectedTile - (firstTile + tilesHorizontal - 1);
    if(std::abs(shiftLeft) < std::abs(shiftRight))
      firstTile -= shiftLeft;
    else
      firstTile += shiftRight;
  }
  for(size_t i = 0; i < tiles.size(); ++i)
    tiles[i].moveTo(getTilePosition(i - firstTile, tileWidth, tileHeight, tilesHorizontal, tilesVertical, viewportWidth, viewportHeight), static_cast<int>(i) == selectedTile ? zoom : 1.0, tiles[i].getTargetSize(), tiles[i].getTargetOpacity(), std::chrono::milliseconds(animationsDurationMilliseconds), std::chrono::milliseconds(0), (static_cast<int>(i) == selectedTile && bounce) ? bounce : 0);
  if(selectedTile >= 0 && selectedTile < static_cast<int>(tiles.size()))
    background.setSourceTile(&tiles[selectedTile], !bounce ? std::chrono::milliseconds(fadingDurationMilliseconds) : std::chrono::milliseconds(0), std::chrono::milliseconds(0));
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

void Menu::FullscreenTile(bool fullscreen) {
}

void Menu::SetIcon(int id, char* pixels, int w, int h) {
  playback.setIcon(id, pixels, w, h, GL_RGBA);
}

void Menu::UpdatePlaybackControls(int show, int state, int currentTime, int totalTime, std::string text) {
  playback.update(show, state, currentTime, totalTime, text, std::chrono::milliseconds(fadingDurationMilliseconds), std::chrono::milliseconds(show && renderMenu ? fadingDurationMilliseconds * 3 / 4 : 0));
}

void Menu::SetVersion(std::string version) {
  this->version = version;
}

#endif // _MENU_H_
