#include "../include/Playback.h"

Playback::Playback()
  : barProgramObject(GL_INVALID_VALUE),
    iconProgramObject(GL_INVALID_VALUE),
    icons(std::vector<GLuint>(static_cast<int>(Icon::LENGTH), GL_INVALID_VALUE)),
    enabled(false),
    state(State::Idle),
    currentTime(0),
    totalTime(0),
    displayText("Loading..."),
    opacity(0.0f),
    progress(0.0f),
    lastUpdate(std::chrono::high_resolution_clock::now())
{
  initialize();
}

Playback::~Playback() {
  if(barProgramObject != GL_INVALID_VALUE)
    glDeleteProgram(barProgramObject);
  if(iconProgramObject != GL_INVALID_VALUE)
    glDeleteProgram(iconProgramObject);
}

bool Playback::initialize() {
  const GLchar* barVShaderTexStr =  
    "attribute vec4 a_position;     \n"
    "void main()                    \n"
    "{                              \n"
    "   gl_Position = a_position;   \n"
    "}                              \n";

// TODO: It seems rendering this progress bar is quite performance heavy - look into optimizing it.
  const GLchar* barFShaderTexStr =  
    "precision mediump float;                                                      \n"
    "uniform float u_time;                                                         \n"
    "uniform float u_param;                                                        \n"
    "uniform float u_opacity;                                                      \n"
    "uniform vec2 u_viewport;                                                      \n"
    "uniform vec2 u_size;                                                          \n"
    "uniform float u_margin;                                                       \n"
    "                                                                              \n"
    "#define PROG_A vec4(1.0, 1.0, 1.0, 1.0)                                       \n"
    "#define PROG_B vec4(0.5, 0.5, 0.5, 0.5)                                       \n"
    "#define TRANSP vec4(1.0, 1.0, 1.0, 0.0)                                       \n"
    "#define SHADOW vec4(0.0, 0.0, 0.0, 1.0)                                       \n"
    "#define SMOOTH 0.9                                                            \n"
    "                                                                              \n"
    "vec4 progressBar(vec2 pos, vec2 size, vec2 res, float dot, float prog) {      \n"
    "  vec2 p = gl_FragCoord.xy;                                                   \n"
    "  vec2 progPoint = vec2(pos.x + prog * size.x, pos.y + size.y / 2.0);         \n"
    "  vec4 c = TRANSP;                                                            \n"
    "  vec4 d = vec4(0.0, 0.0, 0.0, 0.0);                                          \n"
    "                                                                              \n"
   "  d = mix(TRANSP, PROG_A, smoothstep(size.y / 2.0, size.y / 2.0 * SMOOTH, length(p - (pos + vec2(0.0, size.y / 2.0))))); \n"
    "  if(d.a > 0.0)                                                               \n"
    "    c = d;                                                                    \n"
    "  d = mix(TRANSP, PROG_B, smoothstep(size.y / 2.0, size.y / 2.0 * SMOOTH, length(p - (pos + vec2(size.x, size.y / 2.0))))); \n"
    "  if(d.a > 0.0)                                                               \n"
    "    c = d;                                                                    \n"
    "                                                                              \n"
    "  if(p.x >= pos.x && p.x <= pos.x + size.x &&                                 \n"
    "     p.y >= pos.y && p.y <= pos.y + size.y)                                   \n"
    "      c = mix(TRANSP, p.x < progPoint.x ? PROG_A : PROG_B, smoothstep(size.y / 2.0, size.y / 2.0 * SMOOTH, abs(p.y - progPoint.y))); \n"
    "  d = mix(TRANSP, PROG_A, smoothstep(dot, dot * SMOOTH, length(p - progPoint))); \n"
    "  c = mix(c, d, d.a);                                                         \n"
    "  return c;                                                                   \n"
    "}                                                                             \n"
    "                                                                              \n"
    "void main()                                                                   \n"
    "{                                                                             \n"
    "  vec2 res = u_viewport;                                                      \n"
    "  vec2 size = u_size;                                                         \n"
    "  vec2 pos = vec2((res.x - size.x) / 2.0, u_margin);                          \n"
    "  vec4 col = progressBar(pos, size, res, 1.25 * size.y / 2.0, u_param);       \n"
    "  gl_FragColor = vec4(col.rgb, col.a * u_opacity);                            \n"
    "                                                                              \n"
    "}                                                                             \n";

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

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  posBarLoc = glGetAttribLocation(barProgramObject, "a_position");
  paramBarLoc = glGetUniformLocation(barProgramObject, "u_param");
  opacityBarLoc = glGetUniformLocation(barProgramObject, "u_opacity");
  viewportBarLoc = glGetUniformLocation(barProgramObject, "u_viewport");
  sizeBarLoc = glGetUniformLocation(barProgramObject, "u_size");
  marginBarLoc = glGetUniformLocation(barProgramObject, "u_margin");

  const GLchar* iconVShaderTexStr =
    "attribute vec4 a_position;                        \n"
    "attribute vec2 a_texCoord;                        \n"
    "varying vec2 v_texCoord;                          \n"
    "void main()                                       \n"
    "{                                                 \n"
    "  v_texCoord = a_texCoord;                        \n"
    "  gl_Position = a_position;                       \n"
    "}                                                 \n";

  const GLchar* iconFShaderTexStr =
    "precision mediump float;                          \n"
    "uniform vec3 u_color;                             \n"
    "varying vec2 v_texCoord;                          \n"
    "uniform sampler2D s_texture;                      \n"
    "uniform float u_opacity;                          \n"
    "void main()                                       \n"
    "{                                                 \n"
    "  gl_FragColor = texture2D(s_texture, v_texCoord) \n"
    "                 * vec4(u_color, u_opacity);      \n"
    "}                                                 \n";

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

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  samplerLoc = glGetUniformLocation(iconProgramObject, "s_texture");
  colLoc = glGetUniformLocation(iconProgramObject, "u_color");
  opacityLoc = glGetUniformLocation(iconProgramObject, "u_opacity");
  posLoc = glGetAttribLocation(iconProgramObject, "a_position");
  texLoc = glGetAttribLocation(iconProgramObject, "a_texCoord");

  return true;
}

void Playback::updateProgress() {
  if(progressAnimation.isActive()) {
    std::vector<double> updates = progressAnimation.update();
    if(!updates.empty())
      progress = static_cast<float>(updates[0]);
  }
  else if(totalTime) {
    progress = static_cast<float>(currentTime) / static_cast<float>(totalTime);
  }
}

void Playback::render(Text &text) {
  std::vector<double> updated = opacityAnimation.update();
  if(!updated.empty())
    opacity = static_cast<float>(updated[0]);
  if(opacity <= 0.0)
    return;
  updateProgress();
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
    case State::Playing:
      icon = Icon::Pause;
      color = {1.0, 1.0, 1.0, 1.0};
      break;
    case State::Paused:
    case State::Prepared:
    case State::Stopped:
      icon = Icon::Play;
      color = {1.0, 1.0, 1.0, 1.0};
      break;
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

  float scale = 1.0;
  float leftPx = position.first - (size.first / 2.0) * (scale - 1.0);
  float rightPx = (position.first + size.first) + (size.first / 2.0) * (scale - 1.0);
  float downPx = position.second - (size.second / 2.0) * (scale - 1.0);
  float topPx = (position.second + size.second) + (size.second / 2.0) * (scale - 1.0);
  float left = (leftPx / viewportWidth) * 2.0 - 1.0;
  float right = (rightPx / viewportWidth) * 2.0 - 1.0;
  float down = (downPx / viewportHeight) * 2.0 - 1.0;
  float top = (topPx / viewportHeight) * 2.0 - 1.0;

  GLfloat vertices[] = { left,   top,  0.0f,
                         left,   down, 0.0f,
                         right,  down, 0.0f,
                         right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(iconProgramObject);

  glBindTexture(GL_TEXTURE_2D, icons[static_cast<int>(icon)]);
  glUniform1i(samplerLoc, 0);

  float tex[] = { 0.0f, 0.0f,    0.0f, 1.0f,
                  1.0f, 1.0f,    1.0f, 0.0f };
                       
  glEnableVertexAttribArray(texLoc);
  glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, tex);

  if(shadowMode != Shadow::None) {
    if(color.size() < 3)
      glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);
    else if(color[0] + color[1] + color[2] < 1.5f)
      glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);
    else
      glUniform3f(colLoc, 0.0f, 0.0f, 0.0f);

    glUniform1f(opacityLoc, static_cast<GLfloat>(opacity));

    float dir[] = {
       0.0f,  1.0f,
      -1.0f,  1.0f,
      -1.0f,  0.0f,
      -1.0f, -1.0f,
       0.0f, -1.0f,
       1.0f, -1.0f,
       1.0f,  0.0f,
       1.0f,  1.0f
    };

    std::pair<int, int> dirs = {0, 0};
    switch(shadowMode) {
      case Shadow::Single:
        dirs = {3, 4};
        break;
      case Shadow::Outline:
        dirs = {0, 8};
        break;
      default:
        dirs = {0, 0};
        break;
    }

    std::pair<float, float> outlineOffset = {3.0 / viewportWidth, 3.0 / viewportHeight};
    for(int i = dirs.first; i < dirs.second; ++i) {
      GLfloat outlineV[4 * 3];
      for(int j = 0; j < 4 * 3; ++j) {
        switch(j % 3) {
          case 0:
            outlineV[j] = vertices[j] + dir[2 * i] * outlineOffset.first;
            break;
          case 1:
            outlineV[j] = vertices[j] + dir[2 * i + 1] * outlineOffset.second;
            break;
          case 2:
            outlineV[j] = vertices[j];
            break;
        }
      }

      glEnableVertexAttribArray(posLoc);
      glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, outlineV);

      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    }
  }

  if(color.size() >= 3)
    glUniform3f(colLoc, color[0], color[1], color[2]);
  else
    glUniform3f(colLoc, 1.0, 1.0, 1.0);

  glUniform1f(opacityLoc, static_cast<GLfloat>(opacity));

  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

void Playback::renderText(Text &text, float opacity) {
  std::pair<int, int> viewport = {viewportWidth, viewportHeight};

  // render remaining time
  int fontHeight = 24;
  int textWidth = text.getTextSize("00:00:00", {0, fontHeight}, 0, viewport).first * viewport.first / 2.0;
  int textLeft = viewport.first - textWidth - ((viewport.first - progressBarWidth) / 2 - textWidth) / 2 + progressBarHeight / 2;
  int textDown = progressBarMarginBottom + progressBarHeight / 2 - fontHeight / 2;
  text.render(timeToString(totalTime - currentTime),
              {textLeft, textDown},
              {0, fontHeight},
              viewport,
              0,
              {1.0, 1.0, 1.0, opacity},
              true);

  //render current time
/*  fontHeight = 24;
  textLeft = ((viewport.first - progressBarWidth) / 2 + progressBarWidth * progress) + 2;
  textDown = progressBarMarginBottom + progressBarHeight + 4;
  text.render(timeToString(currentTime),
              {textLeft, textDown},
              {0, fontHeight},
              viewport,
              0,
              {1.0, 1.0, 1.0, opacity},
              true);*/

  //render title
  fontHeight = 48;
  textLeft = 100;
  textDown = viewport.second - fontHeight - 100;
  text.render(displayText,
              {textLeft, textDown},
              {0, fontHeight},
              viewport,
              0,
              {1.0, 1.0, 1.0, opacity},
              true);
}

void Playback::renderProgressBar(float opacity) {
  float down = -1.0f;
  float top = 1.0f;
  float left = -1.0f;
  float right = 1.0f;
  GLfloat vertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
  glUseProgram(barProgramObject);
  glEnableVertexAttribArray(posBarLoc);
  glVertexAttribPointer(posBarLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);
  glUniform1f(paramBarLoc, progress);
  glUniform1f(opacityBarLoc, opacity);
  glUniform2f(viewportBarLoc, viewportWidth, viewportHeight);
  glUniform2f(sizeBarLoc, progressBarWidth, progressBarHeight);
  glUniform1f(marginBarLoc, progressBarMarginBottom);

  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

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

    glDeleteShader(shader);
  }
}

void Playback::initTexture(int id) {
  if(id >= static_cast<int>(icons.size()))
    return;
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &icons[id]);
}

void Playback::setIcon(int id, char* pixels, std::pair<int, int> size, GLuint format) {
  if(id >= static_cast<int>(icons.size()))
   return; 
  if(icons[id] == GL_INVALID_VALUE)
    initTexture(id);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, icons[id]);
  glTexImage2D(GL_TEXTURE_2D, 0, format, size.first, size.second, 0, format, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFlush();
}

void Playback::update(int show, int state, int currentTime, int totalTime, std::string text, std::chrono::milliseconds animationDuration, std::chrono::milliseconds animationDelay) {
  updateProgress();
  std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
  std::chrono::milliseconds fromLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate);
  if(static_cast<bool>(show) != enabled) {
    enabled = static_cast<bool>(show);
    opacityAnimation = Animation(now, //std::chrono::high_resolution_clock::now(),
                          animationDuration,
                          animationDelay,
                          {static_cast<double>(opacity)},
                          {enabled ? 1.0 : 0.0},
                          opacityAnimation.isActive() ? Animation::Easing::CubicOut : Animation::Easing::CubicInOut);
  }

  if(currentTime != this->currentTime) {
    lastUpdate = now;
    progressAnimation = Animation(now,
                          std::min(fromLastUpdate, std::chrono::milliseconds(1000)),
                          std::chrono::milliseconds(0),
                          {progress},
                          {static_cast<double>(currentTime) / static_cast<double>(totalTime)},
                          Animation::Easing::Linear);
  }

  this->state = static_cast<State>(state);
  this->currentTime = currentTime;
  this->totalTime = totalTime;
  displayText = text;
}

std::string Playback::timeToString(int time) {
  time /= 1000;
  int h = time / 3600;
  int m = (time % 3600) / 60;
  int s = time % 60;
  return (h ? std::to_string(h) : "") // hours
       + (h ? ":" : "") // h:m colon
       + (m < 10 ? "0" : "") // m leading 0
       + (std::to_string(m)) // minutes
       + (":") // m:s colon
       + (s < 10 ? "0" : "") // s leading 0
       + std::to_string(s); // seconds
}

