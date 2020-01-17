#include <cstdio>

#include "GLES.h"
#include "ExternStructs.h"
#include "CommonStructs.h"
#include "Menu.h"
#include "version.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif
EXPORT_API void Create(); // needs to be run from eglContext synced methods
EXPORT_API void Terminate(); // needs to be run from eglContext synced methods
EXPORT_API void Draw(); // needs to be run from eglContext synced methods

EXPORT_API int AddTile(); // needs to be run from eglContext synced methods
EXPORT_API void SetTileData(TileExternData tileExternData); // needs to be run from eglContext synced methods
EXPORT_API int AddFont(char *data, int size); // needs to be run from eglContext synced methods
EXPORT_API void SetIcon(ImageExternData image); // needs to be run from eglContext synced methods

EXPORT_API void ShowMenu(int enable);
EXPORT_API void ShowLoader(int enabled, int percent);
EXPORT_API void ShowSubtitle(int duration, char* text, int textLen);
EXPORT_API void SelectTile(int tileNo, int runPreview);
EXPORT_API void UpdatePlaybackControls(PlaybackExternData playbackExternData);
EXPORT_API void SetFooter(char* footer, int footerLen);
EXPORT_API int OpenGLLibVersion();
EXPORT_API void SelectAction(int id);

EXPORT_API int AddOption(int id, char* text, int textLen);
EXPORT_API int AddSuboption(int parentId, int id, char* text, int textLen);
EXPORT_API int UpdateSelection(SelectionExternData selectionExternData);
EXPORT_API void ClearOptions();

EXPORT_API int AddGraph(GraphExternData graphExternData);
EXPORT_API void SetGraphVisibility(int graphId, int visible);
EXPORT_API void UpdateGraphValues(int graphId, float* values, int valuesCount);
EXPORT_API void UpdateGraphValue(int graphId, float value);
EXPORT_API void UpdateGraphRange(int graphId, float minVal, float maxVal);
EXPORT_API void SetLogConsoleVisibility(int visible);
EXPORT_API void PushLog(char* log, int logLen);
EXPORT_API void ShowAlert(AlertExternData alertExternData);
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
  menu = new Menu();
}

void Terminate()
{
  if(menu != nullptr)
    delete menu;
}

void ShowMenu(int enable)
{
  menu->showMenu(enable);
}

int AddTile()
{
  return menu->addTile();
}

void SetTileData(TileExternData tileExternData)
{
  menu->setTileData(TileData {
      tileExternData.tileId,
      tileExternData.pixels,
      {tileExternData.width, tileExternData.height},
      std::string(tileExternData.name, tileExternData.nameLen),
      std::string(tileExternData.desc, tileExternData.descLen),
      ConvertFormat(tileExternData.format),
      tileExternData.getStoryboardData});
}

int AddFont(char *data, int size)
{
  return menu->addFont(data, size);
}

void SelectTile(int tileNo, int runPreview)
{
  menu->selectTile(tileNo, static_cast<bool>(runPreview));
}

void ShowLoader(int enabled, int percent)
{
  menu->showLoader(enabled, percent);
}

void SetIcon(ImageExternData image)
{
  menu->setIcon(ImageData {
      image.id,
      image.pixels,
      {image.width, image.height},
      ConvertFormat(image.format)});
}

void UpdatePlaybackControls(PlaybackExternData playbackExternData)
{
  menu->updatePlaybackControls(PlaybackData {
      playbackExternData.show,
      playbackExternData.state,
      playbackExternData.currentTime,
      playbackExternData.totalTime,
      std::string(playbackExternData.text, playbackExternData.textLen),
      static_cast<bool>(playbackExternData.buffering),
      playbackExternData.bufferingPercent,
      playbackExternData.seeking});
}

void SetFooter(char* footer, int footerLen)
{
  menu->setFooter(std::string(footer, footerLen));
}

void Draw()
{
  menu->render();
}

void ShowSubtitle(int duration, char* text, int textLen)
{
  menu->showSubtitle(duration, std::string(text, textLen));
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

int UpdateSelection(SelectionExternData selectionExternData) {
  return menu->updateSelection(SelectionData {
      static_cast<bool>(selectionExternData.show),
      selectionExternData.activeOptionId,
      selectionExternData.activeSubOptionId,
      selectionExternData.selectedOptionId,
      selectionExternData.selectedSubOptionId});
}

void ClearOptions() {
  menu->clearOptions();
}

int AddGraph(GraphExternData graphExternData) {
  return menu->addGraph(GraphData {
      std::string(graphExternData.tag, graphExternData.tagLen),
      graphExternData.minVal,
      graphExternData.maxVal,
      graphExternData.valuesCount});
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


void ShowAlert(AlertExternData alertExternData) {
  menu->showAlert(AlertData {
      std::string(alertExternData.title, alertExternData.titleLen),
      std::string(alertExternData.body, alertExternData.bodyLen),
      std::string(alertExternData.button, alertExternData.buttonLen)});
}

void HideAlert() {
  menu->hideAlert();
}

int IsAlertVisible() {
  return static_cast<int>(menu->isAlertVisible());
}

