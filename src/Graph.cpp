#include "Graph.h"

Graph::Graph()
  : programObject(GL_INVALID_VALUE),
    posALoc(GL_INVALID_VALUE),
    posLoc(GL_INVALID_VALUE),
    sizLoc(GL_INVALID_VALUE),
    valLoc(GL_INVALID_VALUE),
    colLoc(GL_INVALID_VALUE),
    opaLoc(GL_INVALID_VALUE) {
  initialize();
}

Graph::~Graph() {
  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
}

void Graph::initialize() {
  const GLchar* vShaderTexStr =  
    "attribute vec4 a_position;   \n"
    "void main()                  \n"
    "{                            \n"
    "   gl_Position = a_position; \n"
    "}                            \n";

  const GLchar* fShaderTexStr =  
    "precision highp float;                                                            \n"
    "                                                                                  \n"
    "#define FG vec4(u_color, 0.75 * u_opacity)                                        \n"
    "#define BG vec4(0.0, 0.0, 0.0, 0.75 * u_opacity)                                  \n"
    "//#define LOW vec4(1.0, 0.0, 0.0, 0.75 * u_opacity)                               \n"
    "                                                                                  \n"
    "const int VALUES = 100;                                                           \n"
    "                                                                                  \n"
    "uniform vec2 u_position;                                                          \n"
    "uniform vec2 u_size;                                                              \n"
    "uniform float u_value[VALUES];                                                    \n"
    "uniform vec3 u_color;                                                             \n"
    "uniform float u_opacity;                                                          \n"
    "                                                                                  \n"
    "void main()                                                                       \n"
    "{                                                                                 \n"
    "  float f = (gl_FragCoord.x - u_position.x) / u_size.x;                           \n" // global x position [0.0,1.0]
    "  int i = int(f * float(VALUES - 1));                                             \n" // left value index [0, VALUES - 1]
    "  float k = f * float(VALUES - 1) - float(i);                                     \n" // local x pos (btwn samples) [0.0, 1.0]
    "  float v = mix(u_value[i], u_value[i + 1], k);                                   \n" // value for current position [0.0, 1.0]
    "                                                                                  \n"
    "//gl_FragColor = v * u_size.y >= gl_FragCoord.y - u_position.y ? FG : BG          \n"
    "  gl_FragColor = mix(BG,                                                          \n"
    "                     FG, //mix(LOW, FG, clamp(0.0, 1.0, v / 0.3)),                \n"
    "                     smoothstep(0.0,                                              \n"
    "                                5.0,                                              \n"
    "                                v * u_size.y - (gl_FragCoord.y - u_position.y))); \n"
    "                                                                                  \n"
    "                                                                                  \n"
    "  if(gl_FragCoord.y <= u_position.y + 1.0                                         \n"
    "  || gl_FragCoord.y >= u_position.y + u_size.y - 1.0                              \n"
    "  || gl_FragCoord.x <= u_position.x + 1.0                                         \n"
    "  || gl_FragCoord.x >= u_position.x + u_size.x - 1.0                              \n"
    "  )                                                                               \n"
    "    gl_FragColor = vec4(u_color, 0.75 * u_opacity);                               \n"
    "}                                                                                 \n";

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

  posALoc = glGetAttribLocation(programObject, "a_position");
  posLoc = glGetUniformLocation(programObject, "u_position");
  sizLoc = glGetUniformLocation(programObject, "u_size");
  valLoc = glGetUniformLocation(programObject, "u_value");
  colLoc = glGetUniformLocation(programObject, "u_color");
  opaLoc = glGetUniformLocation(programObject, "u_opacity");
}

void Graph::render(const std::vector<float> &values, const std::pair<float, float> &minMax, const std::pair<int, int> &position, const std::pair<int, int> &size, const std::pair<int, int> &viewport) {

  GLfloat vs[VALUES] = { 0.0f };
  for(int i = 0; i < VALUES; ++i) {
    float v = 0.0f;
    if(i < static_cast<int>(values.size()))
      v = clamp((values[i] - minMax.first) / (minMax.second - minMax.first), 0.0f, 1.0f);
    vs[i] = static_cast<GLfloat>(v);
  }

  float down  = static_cast<float>(position.second) / static_cast<float>(viewport.second) * 2.0f - 1.0f;
  float top   = (static_cast<float>(position.second) + static_cast<float>(size.second)) / static_cast<float>(viewport.second) * 2.0f - 1.0f;
  float left  = static_cast<float>(position.first) / static_cast<float>(viewport.first) * 2.0f - 1.0f;
  float right = (static_cast<float>(position.first) + static_cast<float>(size.first)) / static_cast<float>(viewport.first) * 2.0f - 1.0f;
  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(programObject);

  glEnableVertexAttribArray(posALoc);
  glVertexAttribPointer(posALoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  glUniform2f(posLoc, static_cast<float>(position.first), static_cast<float>(position.second));
  glUniform2f(sizLoc, static_cast<float>(size.first), static_cast<float>(size.second));
  glUniform1fv(valLoc, VALUES, static_cast<GLfloat*>(vs));
  glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);

  glUniform1f(opaLoc, 1.0f);

  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glUseProgram(0);
}

void Graph::checkShaderCompileError(GLuint shader) {
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

