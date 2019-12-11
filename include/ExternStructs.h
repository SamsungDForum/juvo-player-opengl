#ifndef _EXTERN_STRUCTS_H_
#define _EXTERN_STRUCTS_H_

#include <cassert>
#include <GLES2/gl2ext.h>

enum class Format
{
  Rgba,
  Bgra,
  Rgb,
  Unknown
};

inline GLuint ConvertFormat(int format)
{
  switch (static_cast<Format>(format)) {
    case Format::Rgba:
      return GL_RGBA;
    case Format::Bgra:
#ifndef GL_BGRA_EXT
#error "GL_BGRA_EXT is not defined"
#endif
      return GL_BGRA_EXT;
    case Format::Rgb:
      return GL_RGB;
    default:
      assert(nullptr);
  }
}

struct SubBitmapExtern
{
  float rectLeft;
  float rectRight;
  float rectTop;
  float rectBottom;
  int bitmapWidth;
  int bitmapHeight;
  int bitmapInfoColorType;
  char* bitmapBytes;
  int bitmapHash;
};

struct StoryboardExternData
{
  int isStoryboardReaderReady;
  int isFrameReady;
  SubBitmapExtern frame;
  long long duration;
};

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
  int format;
  StoryboardExternData (*getStoryboardData)(long long position, int tileId);
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
  int format;
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
