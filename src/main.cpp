#include <cstdio>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "Menu.h"
#include "log.h"

static Menu menu;

#ifdef __cplusplus
extern "C" {
#endif

void Create();
int AddBackground(char *pixels, int width, int height);
void ShowMenu(int enable);
//int AddTile(char *pixels, int width, int height);
int AddTile();
void SetTileData(int tileId, char* pixels, int w, int h, char *name, int nameLen, char *desc, int descLen);
void SetTileTexture(int tileNo, char *pixels, int width, int height);
void SelectTile(int tileNo);
void Draw(void *cDisplay, void *cSurface);
int AddFont(char *data, int size);
void ShowLoader(int enabled, int percent);

#ifdef __cplusplus
}
#endif

void Create()
{
  return;
}

int AddBackground(char *pixels, int width, int height)
{
  return menu.AddBackground(pixels, width, height);
}

void ShowMenu(int enable)
{
  menu.ShowMenu(enable);
}

/*int AddTile(char *pixels, int width, int height)
{
  return menu.AddTile(pixels, width, height);
}*/

int AddTile()
{
  return menu.AddTile();
}

void SetTileData(int tileId, char* pixels, int w, int h, char *name, int nameLen, char *desc, int descLen)
{
  menu.SetTileData(tileId, pixels, w, h, std::string(name, nameLen), std::string(desc, descLen));
}

void SetTileTexture(int tileNo, char *pixels, int width, int height)
{
  menu.SetTileTexture(tileNo, pixels, width, height);
}

int AddFont(char *data, int size)
{
  return menu.AddFont(data, size);
}

void SelectTile(int tileNo)
{
  menu.SelectTile(tileNo);
}

void ShowLoader(int enabled, int percent)
{
  menu.ShowLoader(enabled, percent);
}

void Draw(void *cDisplay, void *cSurface)
{
  EGLDisplay *display = reinterpret_cast<void**>(cDisplay);
  EGLSurface *surface = reinterpret_cast<void**>(cSurface);

  menu.render();

  eglSwapBuffers(*display, *surface);

  return;
}
