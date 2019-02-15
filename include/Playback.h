#ifndef _PLAYBACK_H_
#define _PLAYBACK_H_

#include <chrono>
#include <ctime>
#include <vector>
#include <utility>

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

#include "Text.h"
#include "log.h"
#include "Animation.h"

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
  bool seeking;
  float bufferingPercent;
  std::chrono::time_point<std::chrono::high_resolution_clock> lastUpdate;

  std::pair<int, int> viewport;
  std::pair<int, int> progressBarSize;
  const int progressBarMarginBottom;

  GLuint posBarLoc         = GL_INVALID_VALUE; 
  GLuint paramBarLoc       = GL_INVALID_VALUE; 
  GLuint opacityBarLoc     = GL_INVALID_VALUE; 
  GLuint viewportBarLoc    = GL_INVALID_VALUE;
  GLuint sizeBarLoc        = GL_INVALID_VALUE; 
  GLuint marginBarLoc      = GL_INVALID_VALUE;  

  GLuint samplerLoc        = GL_INVALID_VALUE;
  GLuint colLoc            = GL_INVALID_VALUE;
  GLuint opacityLoc        = GL_INVALID_VALUE;
  GLuint posLoc            = GL_INVALID_VALUE;
  GLuint texLoc            = GL_INVALID_VALUE;

  GLuint colBloomLoc       = GL_INVALID_VALUE;
  GLuint opacityBloomLoc   = GL_INVALID_VALUE;
  GLuint posBloomLoc       = GL_INVALID_VALUE;
  GLuint rectBloomLoc      = GL_INVALID_VALUE;

  GLuint posLoaderLoc      = GL_INVALID_VALUE; 
  GLuint paramLoaderLoc    = GL_INVALID_VALUE; 
  GLuint opacityLoaderLoc  = GL_INVALID_VALUE; 
  GLuint viewportLoaderLoc = GL_INVALID_VALUE; 
  GLuint sizeLoaderLoc     = GL_INVALID_VALUE; 

private:
  bool initialize();
  void initTexture(int id);
  void checkShaderCompileError(GLuint shader);
  void renderIcons(float opacity);
  void renderIcon(Icon icon, std::pair<int, int> position, std::pair<int, int> size, std::vector<float> color, float opacity, bool bloom);
  void renderText(Text &text, float opacity);
  void renderProgressBar(float opacity);
  std::string timeToString(int time);
  void updateProgress();
  void renderLoader(float opacity);
  template<typename T> inline T max(T a, T b) { return a > b ? a : b; }
  template<typename T> inline T clamp(T v, T lo, T hi) { return v < lo ? lo : v > hi ? hi : v; }

public:
  Playback(std::pair<int, int> viewport);
  ~Playback();
  void setIcon(int id, char* pixels, std::pair<int, int> size, GLuint format);
  void render(Text &text);
  void update(int show, int state, int currentTime, int totalTime, std::string text, std::chrono::milliseconds animationDuration, std::chrono::milliseconds animationDelay, bool buffering, float bufferingPercent, bool seeking);
  void setOpacity(float opacity) { this->opacity = opacity; }
  float getOpacity() { return opacity; }
  void selectAction(int id);

  enum class Shadow {
    None,
    Single,
    Outline
  };
  Shadow shadowMode = Shadow::Single;
};

#endif // _PLAYBACK_H_
