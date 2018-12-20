#include <cstdio>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "CommonStructs.h"
#include "Menu.h"
#include "log.h"
#include "version.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif
EXPORT_API void Create();
EXPORT_API void Terminate();
EXPORT_API void ShowMenu(int enable);
EXPORT_API int AddTile();
EXPORT_API void SetTileData(TileData tileData);
EXPORT_API void SetTileTexture(ImageData image);
EXPORT_API void SelectTile(int tileNo);
EXPORT_API void Draw(void *cDisplay, void *cSurface);
EXPORT_API int AddFont(char *data, int size);
EXPORT_API void ShowLoader(int enabled, int percent);
EXPORT_API void SetIcon(ImageData image);
EXPORT_API void UpdatePlaybackControls(PlaybackData playbackData);
EXPORT_API void SetFooter(char* footer, int footerLen);
EXPORT_API void SwitchTextRenderingMode();
EXPORT_API void ShowSubtitle(int duration, char* text, int textLen);
EXPORT_API int OpenGLLibVersion();
EXPORT_API int AddOption(int id, char* text, int textLen);
EXPORT_API int AddSuboption(int parentId, int id, char* text, int textLen);
EXPORT_API int UpdateSelection(SelectionData selectionData);
EXPORT_API void ClearOptions();
EXPORT_API int AddGraph(GraphData graphData);
EXPORT_API void SetGraphVisibility(int graphId, int visible);
EXPORT_API void UpdateGraphValues(int graphId, float* values, int valuesCount);
EXPORT_API void UpdateGraphValue(int graphId, float value);
EXPORT_API void UpdateGraphRange(int graphId, float minVal, float maxVal);
EXPORT_API void SelectAction(int id);
EXPORT_API void SetLogConsoleVisibility(int visible);
EXPORT_API void PushLog(char* log, int logLen);
EXPORT_API void ShowAlert(AlertData alertData);
EXPORT_API void HideAlert();
EXPORT_API int IsAlertVisible();
#ifdef __cplusplus
}
#endif

Menu *menu = nullptr;

void Create()
{
  if(menu != nullptr)
    delete menu;
  menu = new Menu({1920, 1080});
}

void Terminate()
{
  if(menu != nullptr)
    delete menu;
}

void ShowMenu(int enable)
{
  menu->ShowMenu(enable);
}

int AddTile()
{
  return menu->AddTile();
}

void SetTileData(TileData tileData)
{
  menu->SetTileData(tileData.tileId,
                    tileData.pixels,
                    {tileData.width, tileData.height},
                    std::string(tileData.name, tileData.nameLen),
                    std::string(tileData.desc, tileData.descLen));
}

void SetTileTexture(ImageData image)
{
  menu->SetTileTexture(image.id,
                       image.pixels,
                       {image.width, image.height});
}

int AddFont(char *data, int size)
{
  return menu->AddFont(data, size);
}

void SelectTile(int tileNo)
{
  menu->SelectTile(tileNo);
}

void ShowLoader(int enabled, int percent)
{
  menu->ShowLoader(enabled, percent);
}

void SetIcon(ImageData image)
{
  menu->SetIcon(image.id,
                image.pixels,
                {image.width, image.height});
}

void UpdatePlaybackControls(PlaybackData playbackData)
{
  menu->UpdatePlaybackControls(playbackData.show,
                               playbackData.state,
                               playbackData.currentTime,
                               playbackData.totalTime,
                               std::string(playbackData.text, playbackData.textLen),
                               static_cast<bool>(playbackData.buffering),
															 playbackData.bufferingPercent);
}

void SetFooter(char* footer, int footerLen)
{
  menu->SetFooter(std::string(footer, footerLen));
}

void Draw(void *cDisplay, void *cSurface)
{
  EGLDisplay *display = reinterpret_cast<void**>(cDisplay);
  EGLSurface *surface = reinterpret_cast<void**>(cSurface);
  menu->render();
  eglSwapBuffers(*display, *surface);

  return;
}

void SwitchTextRenderingMode()
{
  menu->SwitchTextRenderingMode();
}

void ShowSubtitle(int duration, char* text, int textLen)
{
  menu->ShowSubtitle(duration, std::string(text, textLen));
}

int OpenGLLibVersion() {
#ifdef VERSION
  return VERSION;
#else
  return 0xDEADBEEF;
#endif
}

int AddOption(int id, char* text, int textLen) {
  return menu->addOption(id, std::string(text, textLen)) ? 1 : 0;
}

int AddSuboption(int parentId, int id, char* text, int textLen) {
  return menu->addSuboption(parentId, id, std::string(text, textLen)) ? 1 : 0;
}

int UpdateSelection(SelectionData selectionData) {
  return menu->updateSelection(static_cast<bool>(selectionData.show),
                               selectionData.activeOptionId,
                               selectionData.activeSubOptionId,
                               selectionData.selectedOptionId,
                               selectionData.selectedSubOptionId);
}

void ClearOptions() {
  menu->clearOptions();
}

int AddGraph(GraphData graphData) {
  return menu->addGraph(std::string(graphData.tag, graphData.tagLen),
                        graphData.minVal,
                        graphData.maxVal,
                        graphData.valuesCount);
}
void SetGraphVisibility(int graphId, int visible) {
  menu->setGraphVisibility(graphId, static_cast<bool>(visible));
}

void UpdateGraphValues(int graphId, float* values, int valuesCount) {
  menu->updateGraphValues(graphId, std::vector<float>(values, values + valuesCount));
}

void UpdateGraphValue(int graphId, float value) {
  menu->updateGraphValue(graphId, value);
}

void SelectAction(int id) {
  menu->selectAction(id);
}

void UpdateGraphRange(int graphId, float minVal, float maxVal) {
  menu->updateGraphRange(graphId, minVal, maxVal);
}

void SetLogConsoleVisibility(int visible) {
  menu->setLogConsoleVisibility(static_cast<bool>(visible));
}

void PushLog(char* log, int logLen) {
  menu->pushLog(std::string(log, logLen));
}


void ShowAlert(AlertData alertData) {
  menu->showAlert(std::string(alertData.title, alertData.titleLen),
                  std::string(alertData.body, alertData.bodyLen),
                  std::string(alertData.button, alertData.buttonLen));
}

void HideAlert() {
  menu->hideAlert();
}

int IsAlertVisible() {
  return static_cast<int>(menu->isAlertVisible());
}

