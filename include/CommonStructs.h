#ifndef _COMMON_STRUCTS_H_
#define _COMMON_STRUCTS_H_

#include <string>
#include <utility>

#include "GLES.h"
#include "ExternStructs.h"
#include "Utility.h"

struct TileData
{
  int tileId;
  char* pixels;
  Size<int> size;
  std::string name;
  std::string desc;
  GLuint format;
  StoryboardExternData (*getStoryboardData)(long long position, int tileId);
};

struct Rect
{
  float left;
  float right;
  float top;
  float bottom;
  inline float width() { return right - left; }
  inline float height() { return bottom - top; }
};

struct PlaybackData
{
  int show;
  int state;
  int currentTime;
  int totalTime;
  std::string text;
  int buffering;
  int bufferingPercent;
  bool seeking;
};

struct GraphData
{
  std::string tag;
  float minVal;
  float maxVal;
  int valuesCount;
};

struct AlertData
{
  std::string title;
  std::string body;
  std::string button;
};

struct ImageData
{
  int id;
  char* pixels;
  Size<int> size;
  GLuint format;
};

struct SelectionData
{
  int show;
  int activeOptionId;
  int activeSubOptionId;
  int selectedOptionId;
  int selectedSubOptionId;
};

#endif // _COMMON_STRUCTS_H_
