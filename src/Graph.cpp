#include "Graph.h"
#include "ProgramBuilder.h"
#include "Settings.h"

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
#include "shaders/graph.vert"
;

  const GLchar* fShaderTexStr =  
#include "shaders/graph.frag"
;

  programObject = ProgramBuilder::buildProgram(vShaderTexStr, fShaderTexStr);

  posALoc = glGetAttribLocation(programObject, "a_position");
  posLoc = glGetUniformLocation(programObject, "u_position");
  sizLoc = glGetUniformLocation(programObject, "u_size");
  valLoc = glGetUniformLocation(programObject, "u_value");
  colLoc = glGetUniformLocation(programObject, "u_color");
  opaLoc = glGetUniformLocation(programObject, "u_opacity");
}

void Graph::render(const std::vector<float> &values, const std::pair<float, float> &minMax, const std::pair<int, int> &position, const std::pair<int, int> &size) {

  GLfloat vs[VALUES] = { 0.0f };
  for(int i = 0; i < VALUES; ++i) {
    float v = 0.0f;
    if(i < static_cast<int>(values.size()))
      v = clamp((values[i] - minMax.first) / (minMax.second - minMax.first), 0.0f, 1.0f);
    vs[i] = static_cast<GLfloat>(v);
  }

  float down  = static_cast<float>(position.second) / static_cast<float>(Settings::instance().viewport.second) * 2.0f - 1.0f;
  float top   = (static_cast<float>(position.second) + static_cast<float>(size.second)) / static_cast<float>(Settings::instance().viewport.second) * 2.0f - 1.0f;
  float left  = static_cast<float>(position.first) / static_cast<float>(Settings::instance().viewport.first) * 2.0f - 1.0f;
  float right = (static_cast<float>(position.first) + static_cast<float>(size.first)) / static_cast<float>(Settings::instance().viewport.first) * 2.0f - 1.0f;
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

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(posALoc);
  glUseProgram(GL_INVALID_VALUE);
}

