#include <cstdio>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "Menu.h"
#include "log.h"
#include "version.h"

static Menu menu({1920, 1080});

#ifdef __cplusplus
extern "C" {
#endif
void Create();
void ShowMenu(int enable);
int AddTile();
void SetTileData(int tileId, char* pixels, int w, int h, char *name, int nameLen, char *desc, int descLen);
void SetTileTexture(int tileNo, char *pixels, int width, int height);
void SelectTile(int tileNo);
void Draw(void *cDisplay, void *cSurface);
int AddFont(char *data, int size);
void ShowLoader(int enabled, int percent);
void SetIcon(int id, char* pixels, int w, int h);
void UpdatePlaybackControls(int show, int state, int currentTime, int totalTime, char* text, int textLen);
void SetFooter(char* footer, int footerLen);
void SwitchTextRenderingMode();
void SwitchFPSCounterVisibility();
void ShowSubtitle(int duration, char* text, int textLen);
int OpenGLLibVersion();
int AddOption(int id, char* text, int textLen);
int AddSuboption(int parentId, int id, char* text, int textLen);
int UpdateSelection(int activeOptionId, int activeSuboptionId, int selectedOptionId, int selectedSuboptionId);
void ClearOptions();
#ifdef __cplusplus
}
#endif

void Create()
{
  return;
}

void ShowMenu(int enable)
{
  menu.ShowMenu(enable);
}

int AddTile()
{
  return menu.AddTile();
}

void SetTileData(int tileId, char* pixels, int w, int h, char *name, int nameLen, char *desc, int descLen)
{
  menu.SetTileData(tileId, pixels, {w, h}, std::string(name, nameLen), std::string(desc, descLen));
}

void SetTileTexture(int tileNo, char *pixels, int width, int height)
{
  menu.SetTileTexture(tileNo, pixels, {width, height});
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

void SetIcon(int id, char* pixels, int w, int h)
{
  menu.SetIcon(id, pixels, {w, h});
}

void UpdatePlaybackControls(int show, int state, int currentTime, int totalTime, char* text, int textLen)
{
  menu.UpdatePlaybackControls(show, state, currentTime, totalTime, std::string(text, textLen));
}

void SetFooter(char* footer, int footerLen)
{
  menu.SetFooter(std::string(footer, footerLen));
}

void Draw(void *cDisplay, void *cSurface)
{
  EGLDisplay *display = reinterpret_cast<void**>(cDisplay);
  EGLSurface *surface = reinterpret_cast<void**>(cSurface);
  menu.render();
  eglSwapBuffers(*display, *surface);

  return;
}

void SwitchTextRenderingMode()
{
  menu.SwitchTextRenderingMode();
}

void SwitchFPSCounterVisibility()
{
  menu.SwitchFPSCounterVisibility();
}

void ShowSubtitle(int duration, char* text, int textLen)
{
  menu.ShowSubtitle(duration, std::string(text, textLen));
}

int OpenGLLibVersion() {
#ifdef VERSION
  return VERSION;
#else
  return 0;
#endif
}

int AddOption(int id, char* text, int textLen) {
  return menu.addOption(id, std::string(text, textLen)) ? 1 : 0;
}

int AddSuboption(int parentId, int id, char* text, int textLen) {
  return menu.addSuboption(parentId, id, std::string(text, textLen)) ? 1 : 0;
}

int UpdateSelection(int activeOptionId, int activeSuboptionId, int selectedOptionId, int selectedSuboptionId) {
  return menu.updateSelection(activeOptionId, activeSuboptionId, selectedOptionId, selectedSuboptionId);
}

void ClearOptions() {
  menu.clearOptions();
}
