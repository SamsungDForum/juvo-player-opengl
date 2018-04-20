#include "Options.h"


Options::Options()
  : maxTextLength(16),
    activeOptionId(-1),
    activeSuboptionId(-1),
    selectedOptionId(-1),
    selectedSuboptionId(-1) {
}

void Options::initialize() {
  const GLchar* vShaderTexStr =  
    "attribute vec4 a_position;     \n"
    "void main()                    \n"
    "{                              \n"
    "   gl_Position = a_position;   \n"
    "}                              \n";

  const GLchar* fShaderTexStr =  
    "precision mediump float; // highp is not supported                               \n"
    "uniform vec3 u_color;                                                            \n"
    "uniform float u_opacity;                                                         \n"
    "uniform float u_frame;                                                           \n"
    "                                                                                 \n"
   "                                                                                  \n"
    "void main()                                                                      \n"
    "{                                                                                \n"
    "  gl_FragColor = vec4(1.0);                                                      \n"
    "}                                                                                \n";

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vShaderTexStr, NULL);
  glCompileShader(vertexShader);
  checkShaderCompileError(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fShaderTexStr, NULL);
  glCompileShader(fragmentShader);
  checkShaderCompileError(fragmentShader);

  programObject = glCreateProgram();
  glAttachShader(programObject, vertexShader);
  glAttachShader(programObject, fragmentShader);
  glLinkProgram(programObject);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  posLoc = glGetAttribLocation(programObject, "a_position");
  colLoc = glGetUniformLocation(programObject, "u_color");
  opaLoc = glGetUniformLocation(programObject, "u_opacity");
  fraLoc = glGetUniformLocation(programObject, "u_frame");
}

bool Options::addOption(int id, std::string name) {
  Option opt;
  opt.id = id;
  opt.name = name.substr(0, maxTextLength);
  options.insert({id, opt});
  return true;
}

bool Options::addSuboption(int parentId, int id, std::string name) {
  if(!options.count(parentId))
    return false;
  Suboption subopt;
  subopt.id = id;
  subopt.parentId = parentId;
  subopt.name = name.substr(0, maxTextLength);
  options.at(parentId).subopt.insert({id, subopt});
  return true;
}

bool Options::updateSelection(int activeOptionId, int activeSuboptionId, int selectedOptionId, int selectedSuboptionId) {
  this->activeOptionId = activeOptionId;
  this->activeSuboptionId = activeSuboptionId;
  this->selectedOptionId = selectedOptionId;
  this->selectedSuboptionId = selectedSuboptionId;
  return true;
}

void Options::clearOptions() {
  options.clear();
}

void Options::render(std::pair<int, int> position, std::pair<int, int> viewport, float opacity, const Text &text) {
  for(const std::pair<int, Option>& option : options) {
    // TODO: render option rectangle
    // TODO: render option text
    for(const std::pair<int, Suboption>& suboption : option.second.subopt) {
      // TODO: render suboption rectangle
      // TODO: render suboption text
    }
  }
}

void Option::checkShaderCompileError(GLuint shader) {
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

void Options::renderRectangle(std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, std::vector<float> color, float opacity, std::string name, int frame, const Text &text) {
  float down = static_cast<float>(position.second) / viewport.second * 2.0f - 1.0f;
  float top = static_cast<float>(position.second + size.second) / viewport.second * 2.0f - 1.0f;
  float left = static_cast<float>(position.first) / viewport.first * 2.0f - 1.0f;
  float right = static_cast<float>(position.first + size.first) / viewport.first * 2.0f - 1.0f;
  GLfloat vertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
  glUseProgram(programObject);
  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);
  if(color.size() >= 3)
    glUniform3f(colLoc, color[0], color[1], color[2]);
  else if(color.size() == 1)
    glUniform3f(colLoc, color[0], color[0], color[0]);
  else
    glUniform3f(colLoc, 0.0f, 0.0f, 0.0f);
  glUniform1f(opaLoc, opacity);
  glUniform1f(fraLoc, frame);

  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}
