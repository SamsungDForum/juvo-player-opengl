#include <cstdio>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "Menu.h"
#include "log.h"
#include "version.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

static Menu menu({1920, 1080});

#ifdef __cplusplus
extern "C" {
#endif
EXPORT_API void Create();
EXPORT_API void ShowMenu(int enable);
EXPORT_API int AddTile();
EXPORT_API void SetTileData(int tileId, char* pixels, int w, int h, char *name, int nameLen, char *desc, int descLen);
EXPORT_API void SetTileTexture(int tileNo, char *pixels, int width, int height);
EXPORT_API void SelectTile(int tileNo);
EXPORT_API void Draw(void *cDisplay, void *cSurface);
EXPORT_API int AddFont(char *data, int size);
EXPORT_API void ShowLoader(int enabled, int percent);
EXPORT_API void SetIcon(int id, char* pixels, int w, int h);
EXPORT_API void UpdatePlaybackControls(int show, int state, int currentTime, int totalTime, char* text, int textLen);
EXPORT_API void SetFooter(char* footer, int footerLen);
EXPORT_API void SwitchTextRenderingMode();
EXPORT_API void ShowSubtitle(int duration, char* text, int textLen);
EXPORT_API int OpenGLLibVersion();
EXPORT_API int AddOption(int id, char* text, int textLen);
EXPORT_API int AddSuboption(int parentId, int id, char* text, int textLen);
EXPORT_API int UpdateSelection(int show, int activeOptionId, int activeSuboptionId, int selectedOptionId, int selectedSuboptionId);
EXPORT_API void ClearOptions();
EXPORT_API int AddGraph(char* tag, int tagLen, float minVal, float maxVal, int valuesCount);
EXPORT_API void SetGraphVisibility(int graphId, int visible);
EXPORT_API void UpdateGraphValues(int graphId, float* values, int valuesCount);
EXPORT_API void UpdateGraphValue(int graphId, float value);
EXPORT_API void UpdateGraphRange(int graphId, float minVal, float maxVal);
EXPORT_API void SelectAction(int id);
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

void ShowSubtitle(int duration, char* text, int textLen)
{
  menu.ShowSubtitle(duration, std::string(text, textLen));
}

int OpenGLLibVersion() {
#ifdef VERSION
  return VERSION;
#else
  return 0xDEADBEEF;
#endif
}

int AddOption(int id, char* text, int textLen) {
  return menu.addOption(id, std::string(text, textLen)) ? 1 : 0;
}

int AddSuboption(int parentId, int id, char* text, int textLen) {
  return menu.addSuboption(parentId, id, std::string(text, textLen)) ? 1 : 0;
}

int UpdateSelection(int show, int activeOptionId, int activeSuboptionId, int selectedOptionId, int selectedSuboptionId) {
  return menu.updateSelection(static_cast<bool>(show), activeOptionId, activeSuboptionId, selectedOptionId, selectedSuboptionId);
}

void ClearOptions() {
  menu.clearOptions();
}

int AddGraph(char* tag, int tagLen, float minVal, float maxVal, int valuesCount) {
  return menu.addGraph(std::string(tag, tagLen), minVal, maxVal, valuesCount);
}

void SetGraphVisibility(int graphId, int visible) {
  menu.setGraphVisibility(graphId, static_cast<bool>(visible));
}

void UpdateGraphValues(int graphId, float* values, int valuesCount) {
  menu.updateGraphValues(graphId, std::vector<float>(values, values + valuesCount));
}

void UpdateGraphValue(int graphId, float value) {
  menu.updateGraphValue(graphId, value);
}

void SelectAction(int id) {
  menu.selectAction(id);
}


void UpdateGraphRange(int graphId, float minVal, float maxVal) {
  menu.updateGraphRange(graphId, minVal, maxVal);
}
