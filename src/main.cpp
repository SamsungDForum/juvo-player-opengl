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
void ShowSubtitle(int duration, char* text, int textLen);
int OpenGLLibVersion();
int AddOption(int id, char* text, int textLen);
int AddSuboption(int parentId, int id, char* text, int textLen);
int UpdateSelection(int show, int activeOptionId, int activeSuboptionId, int selectedOptionId, int selectedSuboptionId);
void ClearOptions();
int AddGraph(char* tag, int tagLen, float minVal, float maxVal, int valuesCount);
void SetGraphVisibility(int graphId, int visible);
void UpdateGraphValues(int graphId, float* values, int valuesCount);
void UpdateGraphValue(int graphId, float value);
#ifdef __cplusplus
}
#endif

EXPORT_API void Create()
{
  return;
}

EXPORT_API void ShowMenu(int enable)
{
  menu.ShowMenu(enable);
}

EXPORT_API int AddTile()
{
  return menu.AddTile();
}

EXPORT_API void SetTileData(int tileId, char* pixels, int w, int h, char *name, int nameLen, char *desc, int descLen)
{
  menu.SetTileData(tileId, pixels, {w, h}, std::string(name, nameLen), std::string(desc, descLen));
}

EXPORT_API void SetTileTexture(int tileNo, char *pixels, int width, int height)
{
  menu.SetTileTexture(tileNo, pixels, {width, height});
}

EXPORT_API int AddFont(char *data, int size)
{
  return menu.AddFont(data, size);
}

EXPORT_API void SelectTile(int tileNo)
{
  menu.SelectTile(tileNo);
}

EXPORT_API void ShowLoader(int enabled, int percent)
{
  menu.ShowLoader(enabled, percent);
}

EXPORT_API void SetIcon(int id, char* pixels, int w, int h)
{
  menu.SetIcon(id, pixels, {w, h});
}

EXPORT_API void UpdatePlaybackControls(int show, int state, int currentTime, int totalTime, char* text, int textLen)
{
  menu.UpdatePlaybackControls(show, state, currentTime, totalTime, std::string(text, textLen));
}

EXPORT_API void SetFooter(char* footer, int footerLen)
{
  menu.SetFooter(std::string(footer, footerLen));
}

EXPORT_API void Draw(void *cDisplay, void *cSurface)
{
  EGLDisplay *display = reinterpret_cast<void**>(cDisplay);
  EGLSurface *surface = reinterpret_cast<void**>(cSurface);
  menu.render();
  eglSwapBuffers(*display, *surface);

  return;
}

EXPORT_API void SwitchTextRenderingMode()
{
  menu.SwitchTextRenderingMode();
}

EXPORT_API void ShowSubtitle(int duration, char* text, int textLen)
{
  menu.ShowSubtitle(duration, std::string(text, textLen));
}

EXPORT_API int OpenGLLibVersion() {
#ifdef VERSION
  return VERSION;
#else
  return 0xDEADBEEF;
#endif
}

EXPORT_API int AddOption(int id, char* text, int textLen) {
  return menu.addOption(id, std::string(text, textLen)) ? 1 : 0;
}

EXPORT_API int AddSuboption(int parentId, int id, char* text, int textLen) {
  return menu.addSuboption(parentId, id, std::string(text, textLen)) ? 1 : 0;
}

EXPORT_API int UpdateSelection(int show, int activeOptionId, int activeSuboptionId, int selectedOptionId, int selectedSuboptionId) {
  return menu.updateSelection(static_cast<bool>(show), activeOptionId, activeSuboptionId, selectedOptionId, selectedSuboptionId);
}

EXPORT_API void ClearOptions() {
  menu.clearOptions();
}

EXPORT_API int AddGraph(char* tag, int tagLen, float minVal, float maxVal, int valuesCount) {
  return menu.addGraph(std::string(tag, tagLen), minVal, maxVal, valuesCount);
}

EXPORT_API void SetGraphVisibility(int graphId, int visible) {
  menu.setGraphVisibility(graphId, static_cast<bool>(visible));
}

EXPORT_API void UpdateGraphValues(int graphId, float* values, int valuesCount) {
  menu.updateGraphValues(graphId, std::vector<float>(values, values + valuesCount));
}

EXPORT_API void UpdateGraphValue(int graphId, float value) {
  menu.updateGraphValue(graphId, value);
}

