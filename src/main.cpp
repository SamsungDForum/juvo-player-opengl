#include <cstdio>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "Menu.h"

static Menu menu;

#ifdef __cplusplus
extern "C" {
#endif

void Create();
int AddBackground(char *pixels, int width, int height);
void EnableBackground(int enable);
int AddTile(char *pixels, int width, int height);
void SelectTile(int tileNo);
void Draw(void *cDisplay, void *cSurface);
int AddFont(char *data, int size);

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

void EnableBackground(int enable)
{
  menu.EnableBackground(enable);
}

int AddTile(char *pixels, int width, int height)
{
  return menu.AddTile(pixels, width, height);
}

int AddFont(char *data, int size)
{
  return menu.AddFont(data, size);
}

void SelectTile(int tileNo)
{
  menu.SelectTile(tileNo);
}

void Draw(void *cDisplay, void *cSurface)
{
  EGLDisplay *display = reinterpret_cast<void**>(cDisplay);
  EGLSurface *surface = reinterpret_cast<void**>(cSurface);

  menu.render();

  eglSwapBuffers(*display, *surface);

  return;
}
