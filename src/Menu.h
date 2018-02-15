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
#include "Underlay.h"

class Menu {
private:
  std::vector<Tile> tiles;
  std::vector<Tile> bgTiles;
  GLuint programObject;
  bool underlayEnabled;
  bool backgroundEnabled;
  bool renderMenu;
  int selectedTile;
  int firstTile;
  float backgroundOpacity;

  const int viewportWidth = 1920;
  const int viewportHeight = 1080;
  const int tileWidth = 450;
  const int tileHeight = 253;
  const int tilesHorizontal = 4;
  const int tilesVertical = 1;
  const int marginFromBottom = 50;
  const float zoom = 1.1;
  const int animationsDurationMilliseconds = 320;
  const int fadingDurationMilliseconds = 1000;
  const bool bouncing = true;

  std::chrono::time_point<std::chrono::high_resolution_clock> fpsT;
  float fpsS;
  int fpsN;
  std::queue<float> fpsV;

  Text text;
  Underlay underlay;

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

  backgroundOpacity = 1.0;
  renderMenu = true;
  backgroundEnabled = true;
  underlayEnabled = true;
  selectedTile = 0;
  firstTile = 0;

  fpsS = 0;
  fpsN = 30;
  fpsT = std::chrono::high_resolution_clock::now();

}

Menu::~Menu() {
}

void Menu::render() {
  glClearColor(0.2f, 0.2f, 0.2f, /*backgroundEnabled ? 1.0f : 0.0f*/ 0.0f);
  glClear(GL_COLOR_BUFFER_BIT/* | GL_DEPTH_BUFFER_BIT*/);

//  glUseProgram(programObject);

  if(underlayEnabled)
    underlay.render();
  if(renderMenu) {
    if(!bgTiles.empty() && backgroundEnabled)
      bgTiles[0].render(text);
    for(size_t i = 0; i < tiles.size(); ++i)
      if(static_cast<int>(i) != selectedTile)
        tiles[i].render(text);
    if(selectedTile < static_cast<int>(tiles.size()))
      tiles[selectedTile].render(text);
  }

  std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> timespan = now - fpsT;
  fpsT = now;
  float fps = 1000.0f / timespan.count();

  fpsS += fps;
  fpsV.push(fps);
  while(fpsV.size() > fpsN) {
    fpsS -= fpsV.front();
    fpsV.pop();
  }
  fps = fpsS / (fpsV.size() ? : 1);

  //text.render(std::to_string(fps));
  std::pair<int, int> viewport = {1920, 1080};
  int fontHeight = 48;
  int margin = 12;
  text.render(std::to_string(static_cast<int>(fps)), {margin, viewport.second - fontHeight - margin}, {0, fontHeight}, viewport, 0, {1.0, 1.0, 1.0, 1.0}, true);
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
  if(bgTiles.size() > 0)
    bgTiles[0].moveTo(bgTiles[0].getPosition(), bgTiles[0].getZoom(), bgTiles[0].getSize(), enable ? backgroundOpacity : 0.0, std::chrono::milliseconds(fadingDurationMilliseconds));
  for(size_t i = 0; i < tiles.size(); ++i)
    tiles[i].moveTo(getTilePosition(i - firstTile, tileWidth, tileHeight, tilesHorizontal, tilesVertical, viewportWidth, viewportHeight), static_cast<int>(i) == selectedTile ? zoom : 1.0, tiles[i].getSize(), enable ? 1 : 0, std::chrono::milliseconds(fadingDurationMilliseconds));
}

std::pair<int, int> Menu::getTilePosition(int tileNo, int tileWidth, int tileHeight, int tilesHorizontal, int tilesVertical, int viewportWidth, int viewportHeight, bool initialMargin) {
  int horizontalMargin = std::max(static_cast<int>(std::ceil(static_cast<double>(viewportWidth - (tileWidth * tilesHorizontal)) / (tilesHorizontal + 1))), 0);
  int verticalMargin = std::max((viewportHeight - (tileHeight * tilesVertical)) / (tilesVertical + 1), 0);
  int horizontalPosition = 0;
  int verticalPosition = 0;
  horizontalPosition = (initialMargin ? horizontalMargin : 0) + (horizontalMargin + tileWidth) * tileNo;
  verticalPosition = tilesVertical > 1 ? tileHeight + verticalMargin : marginFromBottom;
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
            "Description",
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
            "Description");
  tiles.push_back(std::move(tile));
  return tiles.size() - 1;
}

void Menu::SetTileTexture(int tileNo, char *pixels, int width, int height)
{
  if(tileNo >= static_cast<int>(tiles.size()))
    return;
  tiles[tileNo].setTexture(pixels, width, height, GL_RGB);
}

void Menu::SelectTile(int tileNo)
{
  int bounce = (bouncing && selectedTile == tileNo) ? (selectedTile == 0 ? -1 : 1) : 0;
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
    tiles[i].moveTo(getTilePosition(i - firstTile, tileWidth, tileHeight, tilesHorizontal, tilesVertical, viewportWidth, viewportHeight), static_cast<int>(i) == selectedTile ? zoom : 1.0, tiles[i].getTargetSize(), tiles[i].getTargetOpacity(), std::chrono::milliseconds(animationsDurationMilliseconds), (static_cast<int>(i) == selectedTile && bounce) ? bounce : 0);
}

int Menu::AddFont(char *data, int size) {
  text.AddFont(data, size, 48);
  return 0;
}

void Menu::ShowLoader(bool enabled, int percent) {
  if(enabled == false) {
    underlayEnabled = false;
    ShowMenu(true);
  }
  else if(backgroundEnabled == true) {
      ShowMenu(false);
      underlayEnabled = true;
  }
  underlay.setValue(percent);
}

void Menu::FullscreenTile(bool fullscreen) {
}

#endif // _MENU_H_
