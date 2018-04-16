#ifndef _PLAYBACK_H_
#define _PLAYBACK_H_

#include <chrono>
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
    Preparing = 1,
    Prepared = 2,
    Stopped = 3,
    Paused = 4,
    Playing = 5,
    Error = 6,
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
    LENGTH
  };

private:
  GLuint barProgramObject;
  GLuint iconProgramObject;
  Animation opacityAnimation;
  Animation progressAnimation;
  std::vector<GLuint> icons;

  bool enabled;
  State state;
  int currentTime;
  int totalTime;
  std::string displayText;
  float opacity;

  float progress;
  std::chrono::time_point<std::chrono::high_resolution_clock> lastUpdate;

  std::pair<int, int> viewport;
  std::pair<int, int> progressBarSize;
  const int progressBarMarginBottom;

  GLint posBarLoc       = GL_INVALID_VALUE; 
  GLuint paramBarLoc    = GL_INVALID_VALUE; 
  GLuint opacityBarLoc  = GL_INVALID_VALUE; 
  GLuint viewportBarLoc = GL_INVALID_VALUE;
  GLuint sizeBarLoc     = GL_INVALID_VALUE; 
  GLuint marginBarLoc   = GL_INVALID_VALUE;  

  GLuint samplerLoc = GL_INVALID_VALUE;
  GLuint colLoc     = GL_INVALID_VALUE;
  GLuint opacityLoc = GL_INVALID_VALUE;
  GLuint posLoc     = GL_INVALID_VALUE;
  GLuint texLoc     = GL_INVALID_VALUE;

private:
  bool initialize();
  void initTexture(int id);
  void checkShaderCompileError(GLuint shader);
  void renderIcons(float opacity);
  void renderIcon(Icon icon, std::pair<int, int> position, std::pair<int, int> size, std::vector<float> color, float opacity);
  void renderText(Text &text, float opacity);
  void renderProgressBar(float opacity);
  std::string timeToString(int time);
  void updateProgress();

public:
  Playback(std::pair<int, int> viewport);
  ~Playback();
  void setIcon(int id, char* pixels, std::pair<int, int> size, GLuint format);
  void render(Text &text);
  void update(int show, int state, int currentTime, int totalTime, std::string text, std::chrono::milliseconds animationDuration, std::chrono::milliseconds animationDelay);
  void setOpacity(float opacity) { this->opacity = opacity; }
  float getOpacity() { return opacity; }

  enum class Shadow {
    None,
    Single,
    Outline
  };
  Shadow shadowMode = Shadow::Single;
};

#endif // _PLAYBACK_H_
