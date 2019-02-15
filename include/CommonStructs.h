#ifndef _COMMON_STRUCTS_H_
#define _COMMON_STRUCTS_H_

#include <string>
#include <utility>

struct TileData
{
  int tileId;
  char* pixels;
  std::pair<int, int> size;
  std::string name;
  std::string desc;
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
  std::pair<int, int> size;
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
