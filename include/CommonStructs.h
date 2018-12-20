#ifndef _COMMON_STRUCTS_H_
#define _COMMON_STRUCTS_H_

struct TileData
{
  int tileId;
  char* pixels;
  int width;
  int height;
  char* name;
  int nameLen;
  char* desc;
  int descLen;
};

struct PlaybackData
{
	int show;
	int state;
	int currentTime;
	int totalTime;
	char* text;
	int textLen;
	int buffering;
	int bufferingPercent;
};

struct GraphData
{
  char* tag;
  int tagLen;
  float minVal;
  float maxVal;
  int valuesCount;
};

struct AlertData
{
  char* title;
  int titleLen;
  char* body;
  int bodyLen;
  char* button;
  int buttonLen;
};

struct ImageData
{
	int id;
	char* pixels;
	int width;
	int height;
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
