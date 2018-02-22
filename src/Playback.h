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
#include "TileAnimation.h"

class Playback {
private:
  enum class State {
    Play,
    Pause,
    Stop,
    Loading
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
  TileAnimation animation;
  std::vector<GLuint> icons;

  bool enabled;
  State state;
  int currentTime;
  int totalTime;
  std::string displayText;
  float opacity;

  const int viewportWidth = 1920;
  const int viewportHeight = 1080;
  const int progressBarWidth = 1500;
  const int progressBarHeight = 2;
  const int progressBarMarginBottom = 100;

private:
  bool initialize();
  void initTexture(int id);
  void checkShaderCompileError(GLuint shader);
  void renderIcons(float opacity);
  void renderIcon(Icon icon, std::pair<int, int> position, std::pair<int, int> size, std::vector<float> color, float opacity);
  void renderText(Text &text, float opacity);
  void renderProgressBar(float opacity);
  std::string timeToString(int time);

public:
  Playback();
  ~Playback();
  void setIcon(int id, char* pixels, int w, int h, GLuint format);
  void render(Text &text);
  void update(int show, int state, int currentTime, int totalTime, std::string text, std::chrono::milliseconds animationDuration, std::chrono::milliseconds animationDelay);
  void setOpacity(float opacity) { this->opacity = opacity; }
  float getOpacity() { return opacity; }
};

Playback::Playback()
  : barProgramObject(GL_INVALID_VALUE),
  iconProgramObject(GL_INVALID_VALUE),
  icons(std::vector<GLuint>(static_cast<int>(Icon::LENGTH), GL_INVALID_VALUE)),
  enabled(false),
  state(State::Loading),
  currentTime(0),
  totalTime(0),
  displayText("Loading..."),
  opacity(0.0f)
{
  initialize();
}

Playback::~Playback() {
}

bool Playback::initialize() {
  const GLchar* barVShaderTexStr =  
    "attribute vec4 a_position;     \n"
    "void main()                    \n"
    "{                              \n"
    "   gl_Position = a_position;   \n"
    "}                              \n";

  const GLchar* barFShaderTexStr =  
    "precision mediump float;                                                    \n"
    "uniform float u_time;                                                       \n"
    "uniform float u_param;                                                      \n"
    "uniform float u_opacity;                                                    \n"
    "uniform vec2 u_viewport;                                                    \n"
    "uniform vec2 u_size;                                                        \n"
    "uniform float u_margin;                                                     \n"
    "                                                                            \n"
    "#define PROG_A vec4(1.0, 1.0, 1.0, 1.0)                                     \n"
    "#define PROG_B vec4(0.5, 0.5, 0.5, 0.2)                                     \n"
    "#define TRANSP vec4(0.0, 0.0, 0.0, 0.0)                                     \n"
    "                                                                            \n"
    "vec4 progressBar(vec2 pos, vec2 size, vec2 res, float dot, float prog) {    \n"
    "  vec2 p = gl_FragCoord.xy;                                                 \n"
    "  vec2 progPoint = vec2(pos.x + prog * size.x, pos.y + size.y / 2.0);       \n"
    "  vec4 c = TRANSP;                                                          \n"
    "  if(p.x >= pos.x &&                                                        \n"
    "    p.x <= pos.x + size.x &&                                                \n"
    "    p.y >= pos.y &&                                                         \n"
    "    p.y <= pos.y + size.y) {                                                \n"
    "    if(p.x <= progPoint.x)                                                  \n"
    "      c = PROG_A;                                                           \n"
    "    else                                                                    \n"
    "      c = PROG_B;                                                           \n"
    "  }                                                                         \n"
    "  if(length(p - progPoint) < dot) {                                         \n"
    "    c = PROG_A;                                                             \n"
    "    c.a = 1.0 - pow(1.0 - (dot - length(p - progPoint)) / dot, 3.0);        \n"
    "  }                                                                         \n"
    "  return c;                                                                 \n"
    "}                                                                           \n"
    "                                                                            \n"
    "void main()                                                                 \n"
    "{                                                                           \n"
    "  vec2 res = u_viewport;                                                    \n"
    "  vec2 size = u_size;                                                       \n"
    "  vec2 pos = vec2((res.x - size.x) / 2.0, u_margin);                        \n"
    "  vec4 col = progressBar(pos, size, res, 5.0 * size.y, u_param);            \n"
    "  gl_FragColor = vec4(col.rgb, col.a * u_opacity);                          \n"
    "                                                                            \n"
    "}                                                                           \n";

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &barVShaderTexStr, NULL);
  glCompileShader(vertexShader);
  checkShaderCompileError(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &barFShaderTexStr, NULL);
  glCompileShader(fragmentShader);
  checkShaderCompileError(fragmentShader);

  barProgramObject = glCreateProgram();
  glAttachShader(barProgramObject, vertexShader);
  glAttachShader(barProgramObject, fragmentShader);
  glLinkProgram(barProgramObject);

  const GLchar* iconVShaderTexStr =
    "attribute vec4 a_position;                                            \n"
    "attribute vec2 a_texCoord;                                            \n"
    "varying vec2 v_texCoord;                                              \n"
    "void main()                                                           \n"
    "{                                                                     \n"
    "  v_texCoord = a_texCoord;                                            \n"
    "  gl_Position = a_position;                                           \n"
    "}                                                                     \n";

  const GLchar* iconFShaderTexStr =
    "precision mediump float;                                              \n"
    "uniform vec3 u_color;                                                 \n"
    "varying vec2 v_texCoord;                                              \n"
    "uniform sampler2D s_texture;                                          \n"
    "uniform float u_opacity;                                              \n"
    "void main()                                                           \n"
    "{                                                                     \n"
    "  gl_FragColor = texture2D(s_texture, v_texCoord)                     \n"
    "                 * vec4(u_color, u_opacity);                          \n"
    "}                                                                     \n";



  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &iconVShaderTexStr, NULL);
  glCompileShader(vertexShader);
  checkShaderCompileError(vertexShader);

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &iconFShaderTexStr, NULL);
  glCompileShader(fragmentShader);
  checkShaderCompileError(fragmentShader);

  iconProgramObject = glCreateProgram();
  glAttachShader(iconProgramObject, vertexShader);
  glAttachShader(iconProgramObject, fragmentShader);
  glLinkProgram(iconProgramObject);

  return true;
}

void Playback::render(Text &text) {
  std::pair<int, int> position {0, 0};
  float zoom = 0;
  std::pair<int, int> size {0, 0};
  animation.update(position, zoom, size, opacity);
  /*if(opacity <= 0.0)
    return;*/
  renderProgressBar(opacity);
  renderIcons(opacity);
  renderText(text, opacity);
}

void Playback::renderIcons(float opacity) {
  Icon icon = Icon::Play;
  std::vector<float> color = {1.0, 1.0, 1.0, 1.0};
  std::pair<int, int> size = {64, 64};
  std::pair<int, int> position = {150, 100};
  position = {position.first - size.first / 2, position.second - size.second / 2};
  switch(state) {
    case State::Play:
      icon = Icon::Play;
      color = {1.0, 1.0, 1.0, 1.0};
      break;
    case State::Pause:
      icon = Icon::Pause;
      color = {1.0, 1.0, 1.0, 1.0};
      break;
    case State::Stop:
      icon = Icon::Stop;
      color = {1.0, 1.0, 1.0, 1.0};
      break;
    case State::Loading:
    default:
      icon = Icon::Play;
      color = {0.5, 0.5, 0.5, 1.0};
      break;
  }
  renderIcon(icon, position, size, color, opacity);
}

void Playback::renderIcon(Icon icon, std::pair<int, int> position, std::pair<int, int> size, std::vector<float> color, float opacity) {
  if(static_cast<int>(icon) >= static_cast<int>(icons.size()) || icons[static_cast<int>(icon)] == GL_INVALID_VALUE)
    return;

  //float x = position.first;
  //float y = position.second;
  float width = size.first;
  float height = size.second;
  float zoom = 1.0;

  float vW = viewportWidth;
  float vH = viewportHeight;
  float w = width;
  float h = height;
  float xPos = position.first;
  float yPos = position.second;
  float leftPx = xPos - (w / 2.0) * (zoom - 1.0);
  float rightPx = (xPos + w) + (w / 2.0) * (zoom - 1.0);
  float downPx = yPos - (h / 2.0) * (zoom - 1.0);
  float topPx = (yPos + h) + (h / 2.0) * (zoom - 1.0);
  float left = (leftPx / vW) * 2.0 - 1.0;
  float right = (rightPx / vW) * 2.0 - 1.0;
  float down = (downPx / vH) * 2.0 - 1.0;
  float top = (topPx / vH) * 2.0 - 1.0;

  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(iconProgramObject);

  GLuint colorLoc = glGetUniformLocation(iconProgramObject, "u_color");
  if(color.size() >= 3)
    glUniform3f(colorLoc, color[0], color[1], color[2]);
  else
    glUniform3f(colorLoc, 1.0, 1.0, 1.0);

  glBindTexture(GL_TEXTURE_2D, icons[static_cast<int>(icon)]);
  GLuint samplerLoc = glGetUniformLocation(iconProgramObject, "s_texture");
  glUniform1i(samplerLoc, 0);

  GLint posLoc = glGetAttribLocation(iconProgramObject, "a_position");
  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  float texCoord[] = { 0.0f, 0.0f,    0.0f, 1.0f,
                       1.0f, 1.0f,    1.0f, 0.0f };
                       
  GLint texLoc = glGetAttribLocation(iconProgramObject, "a_texCoord");
  glEnableVertexAttribArray(texLoc);
  glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoord);

  GLuint opacityLoc = glGetUniformLocation(iconProgramObject, "u_opacity");
  glUniform1f(opacityLoc, static_cast<GLfloat>(opacity));
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glEnable(GL_BLEND);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

void Playback::renderText(Text &text, float opacity) {
  std::pair<int, int> viewport = {viewportWidth, viewportHeight};

  // render total time
  int fontHeight = 24;
  int textWidth = text.getTextSize("00:00:00", {0, fontHeight}, 0).first;
  int textLeft = viewport.first - textWidth - ((viewport.first - progressBarWidth) / 2 - textWidth) / 2;
  int textDown = progressBarMarginBottom - fontHeight / 2;
  text.render(timeToString(totalTime), {textLeft, textDown}, {0, fontHeight}, viewport, 0, {1.0, 1.0, 1.0, opacity}, true);

  //render current time
  fontHeight = 24;
  textLeft = ((viewport.first - progressBarWidth) / 2 + progressBarWidth * currentTime / totalTime /*- text.getTextSize("00:00:00", {0, fontHeight}, 0).first / 2*/) + 2;
  textDown = progressBarMarginBottom + fontHeight + 2;
  text.render(timeToString(currentTime), {textLeft, textDown}, {0, fontHeight}, viewport, 0, {1.0, 1.0, 1.0, opacity}, true);

  //render title
  fontHeight = 48;
  textLeft = 100;
  textDown = viewport.second - fontHeight - 100;
  text.render(displayText, {textLeft, textDown}, {0, fontHeight}, viewport, 0, {1.0, 1.0, 1.0, opacity}, true);
}

void Playback::renderProgressBar(float opacity) {
  float down = -1.0f;
  float top = 1.0f;
  float left = -1.0f;
  float right = 1.0f;
  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(barProgramObject);

  GLint posLoc = glGetAttribLocation(barProgramObject, "a_position");
  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  GLuint paramLoc = glGetUniformLocation(barProgramObject, "u_param");
  glUniform1f(paramLoc, totalTime ? static_cast<float>(currentTime) / static_cast<float>(totalTime) : 0.0f);

  GLuint opacityLoc = glGetUniformLocation(barProgramObject, "u_opacity");
  glUniform1f(opacityLoc, opacity);

  GLuint viewportLoc = glGetUniformLocation(barProgramObject, "u_viewport");
  glUniform2f(viewportLoc, viewportWidth, viewportHeight);

  GLuint sizeLoc = glGetUniformLocation(barProgramObject, "u_size");
  glUniform2f(sizeLoc, progressBarWidth, progressBarHeight);

  GLuint marginLoc = glGetUniformLocation(barProgramObject, "u_margin");
  glUniform1f(marginLoc, progressBarMarginBottom);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

void Playback::checkShaderCompileError(GLuint shader) {
  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if(isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
    _ERR("%s", (std::string(errorLog.begin(), errorLog.end()).c_str()));

    glDeleteShader(shader); // Don't leak the shader.
  }
}

void Playback::initTexture(int id) {
  if(id >= static_cast<int>(icons.size()))
    return;
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &icons[id]);
}

void Playback::setIcon(int id, char* pixels, int width, int height, GLuint format) {
  if(id >= static_cast<int>(icons.size()))
   return; 
  if(icons[id] == GL_INVALID_VALUE)
    initTexture(id);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, icons[id]);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR/*GL_NEAREST*/);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR/*GL_NEAREST*/);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFlush();
}

void Playback::update(int show, int state, int currentTime, int totalTime, std::string text, std::chrono::milliseconds animationDuration, std::chrono::milliseconds animationDelay) {
  if(static_cast<bool>(show) != enabled) {
    enabled = static_cast<bool>(show);
    animation = TileAnimation(std::chrono::high_resolution_clock::now(),
                              animationDuration,
                              animationDelay,
                              {0, 0},
                              {0, 0},
                              TileAnimation::Easing::Linear,
                              0,
                              0,
                              TileAnimation::Easing::Linear,
                              {0, 0},
                              {0, 0},
                              TileAnimation::Easing::Linear,
                              opacity,
                              enabled ? 1.0 : 0.0,
                              animation.isActive() ? TileAnimation::Easing::CubicOut : TileAnimation::Easing::CubicInOut);
  }
  this->state = static_cast<State>(state);
  this->currentTime = currentTime;
  this->totalTime = totalTime;
  displayText = text;
}

std::string Playback::timeToString(int time) {
  int h = time / 3600;
  int m = (time % 3600) / 60;
  int s = time % 60;
  return (h ? std::to_string(h) : "") // hours
       + (h ? ":" : "") // h:m colon
       + (h && m < 10 ? "0" : "") // m leading 0
       + (h || m ? std::to_string(m) : "") // minutes
       + (h || m ? ":" : "") // m:s colon
       + ((h|| m) && s < 10 ? "0" : "") // s leading 0
       + std::to_string(s); // seconds
}

#endif // _PLAYBACK_H_
