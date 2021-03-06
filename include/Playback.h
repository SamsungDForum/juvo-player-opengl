#ifndef _PLAYBACK_H_
#define _PLAYBACK_H_

#include <chrono>
#include <vector>
#include <utility>
#include <string>

#include "GLES.h"
#include "Animation.h"
#include "CommonStructs.h"
#include "ExternStructs.h"
#include "Utility.h"

class Playback {
private:
  enum class State {
    Idle = 0,
    Prepared = 1,
    Playing = 2,
    Paused = 3
  };

  enum class Icon {
    Play,
    Resume,
    Stop,
    Pause,
    FastForward,
    Rewind,
    SkipToEnd,
    SkipToStart,
    Options,
    LENGTH
  };

  enum class Action {
    None = 0,
    PlaybackControl = 1,
    OptionsMenu = 2
  };

private:
  GLuint barProgramObject    = GL_INVALID_VALUE;
  GLuint iconProgramObject   = GL_INVALID_VALUE;
  GLuint bloomProgramObject  = GL_INVALID_VALUE;
  GLuint loaderProgramObject = GL_INVALID_VALUE;
  Animation opacityAnimation;
  Animation progressAnimation;
  std::vector<GLuint> icons;

  bool enabled;
  State state;
  int currentTime;
  int totalTime;
  std::string displayText;
  float opacity;
  Action selectedAction = Action::None;

  float progress;
  bool buffering;
  float bufferingPercent;
  bool seeking;
  std::chrono::time_point<std::chrono::steady_clock> lastUpdate;

  const int progressUiLineLevel = 100;
  Size<int> progressBarSizePx;
  Size<int> progressBarSize;
  const int progressBarMarginBottom;
  const float dotScale;
  Size<int> iconSize;

  GLuint posBarLoc         = GL_INVALID_VALUE; 
  GLuint paramBarLoc       = GL_INVALID_VALUE; 
  GLuint opacityBarLoc     = GL_INVALID_VALUE; 
  GLuint viewportBarLoc    = GL_INVALID_VALUE;
  GLuint sizeBarLoc        = GL_INVALID_VALUE; 
  GLuint marginBarLoc      = GL_INVALID_VALUE;
  GLuint dotScaleBarLoc    = GL_INVALID_VALUE;

  GLuint samplerIconLoc    = GL_INVALID_VALUE;
  GLuint texCoordIconLoc   = GL_INVALID_VALUE;
  GLuint posIconLoc        = GL_INVALID_VALUE;
  GLuint colIconLoc        = GL_INVALID_VALUE;
  GLuint shadowColIconLoc  = GL_INVALID_VALUE;
  GLuint shadowOffIconLoc  = GL_INVALID_VALUE;
  GLuint opacityIconLoc    = GL_INVALID_VALUE;
  GLuint colBloomIconLoc   = GL_INVALID_VALUE;
  GLuint opaBloomIconLoc   = GL_INVALID_VALUE;
  GLuint rectBloomIconLoc  = GL_INVALID_VALUE;

  GLuint posLoaderLoc      = GL_INVALID_VALUE; 
  GLuint paramLoaderLoc    = GL_INVALID_VALUE; 
  GLuint opacityLoaderLoc  = GL_INVALID_VALUE; 
  GLuint viewportLoaderLoc = GL_INVALID_VALUE; 
  GLuint sizeLoaderLoc     = GL_INVALID_VALUE;

  GLuint seekTextureId     = 0;
  GLuint seekProgramObject = GL_INVALID_VALUE;
  GLuint samplerSeekLoc    = GL_INVALID_VALUE;
  GLuint texCoordSeekLoc   = GL_INVALID_VALUE;
  GLuint positionSeekLoc   = GL_INVALID_VALUE;
  GLuint imagePositionSeekLoc = GL_INVALID_VALUE;
  GLuint imageSizeSeekLoc  = GL_INVALID_VALUE;
  GLuint viewportSeekLoc   = GL_INVALID_VALUE;
  GLuint opacitySeekLoc    = GL_INVALID_VALUE;
  GLuint scaleSeekLoc      = GL_INVALID_VALUE;
  GLuint storytileRectSeekLoc = GL_INVALID_VALUE;

  SubBitmapExtern storyboardBitmap;
  int storyboardBitmapHash;
  Rect storytileRect;
  StoryboardExternData (*getStoryboardDataCallback)() = nullptr;
  bool displaySeekPreview;
  bool seekPreviewReady;

private:
  void initialize();
  void initTexture(int id);
  void renderIcons();
  void renderIcon(Icon icon, Position<int> position, Size<int> size, std::vector<float> color, float opacity, bool bloom);
  void renderText();
  void renderProgressBar();
  std::string timeToString(int time);
  void updateProgress();
  void renderLoader(float opacity);
  void renderSeekPreview();
  void updateSeekPreviewTexture();
  Position<int> getSeekPreviewPosition(Size<int> size);
  void setPreviewTexture(SubBitmapExtern frame);
  StoryboardExternData getStoryboardData();
  void initTextures();
  template<typename T> inline T max(T a, T b) { return a > b ? a : b; }
  template<typename T> inline T clamp(T v, T lo, T hi) { return v < lo ? lo : v > hi ? hi : v; }
  void renderSeekPreviewTime();
  Size<int> getSeekPreviewTileSize();

public:
  Playback();
  ~Playback();
  void setIcon(int id, char* pixels, Size<int> size, GLuint format);
  void render();
  void update(int show, int state, int currentTime, int totalTime, std::string text, std::chrono::milliseconds animationDuration, std::chrono::milliseconds animationDelay, bool buffering, float bufferingPercent, bool seeking);
  void setOpacity(float opacity) { this->opacity = opacity; }
  float getOpacity() { return opacity; }
  void selectAction(int id);
  void setStoryboardCallback(StoryboardExternData (*getSeekPreviewStoryboardDataCallback)());

  enum class Shadow {
    None,
    Single,
    Outline
  };
  Shadow shadowMode = Shadow::Single;
};

#endif // _PLAYBACK_H_
