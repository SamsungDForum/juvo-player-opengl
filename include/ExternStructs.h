#ifndef _EXTERN_STRUCTS_H_
#define _EXTERN_STRUCTS_H_

struct TileExternData
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

struct PlaybackExternData
{
  int show;
  int state;
  int currentTime;
  int totalTime;
  char* text;
  int textLen;
  int buffering;
  int bufferingPercent;
  int seeking;
};

struct GraphExternData
{
  char* tag;
  int tagLen;
  float minVal;
  float maxVal;
  int valuesCount;
};

struct AlertExternData
{
  char* title;
  int titleLen;
  char* body;
  int bodyLen;
  char* button;
  int buttonLen;
};

struct ImageExternData
{
  int id;
  char* pixels;
  int width;
  int height;
};

struct SelectionExternData
{
  int show;
  int activeOptionId;
  int activeSubOptionId;
  int selectedOptionId;
  int selectedSubOptionId;
};

#endif // _EXTERN_STRUCTS_H_
