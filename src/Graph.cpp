#include "Graph.h"
#include "ProgramBuilder.h"
#include "Settings.h"
#include "Utility.h"

Graph::Graph()
  : programObject(GL_INVALID_VALUE),
    posALoc(GL_INVALID_VALUE),
    posLoc(GL_INVALID_VALUE),
    sizLoc(GL_INVALID_VALUE),
    valLoc(GL_INVALID_VALUE),
    opaLoc(GL_INVALID_VALUE) {
  initialize();
}

Graph::~Graph() {
  assertCurrentEGLContext();

  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
}

void Graph::initialize() {
  assertCurrentEGLContext();

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
  opaLoc = glGetUniformLocation(programObject, "u_opacity");
}

void Graph::render(const std::vector<float> &values, const std::pair<float, float> &minMax, const Position<int> &position, const Size<int> &size) {
  assertCurrentEGLContext();

  GLfloat vs[VALUES] = { 0.0f };
  for(int i = 0; i < VALUES; ++i) {
    float v = 0.0f;
    if(i < static_cast<int>(values.size()))
      v = clamp((values[i] - minMax.first) / (minMax.second - minMax.first), 0.0f, 1.0f);
    vs[i] = static_cast<GLfloat>(v);
  }

  float down  = static_cast<float>(position.y) / static_cast<float>(Settings::instance().viewport.height) * 2.0f - 1.0f;
  float top   = (static_cast<float>(position.y) + static_cast<float>(size.height)) / static_cast<float>(Settings::instance().viewport.height) * 2.0f - 1.0f;
  float left  = static_cast<float>(position.x) / static_cast<float>(Settings::instance().viewport.width) * 2.0f - 1.0f;
  float right = (static_cast<float>(position.x) + static_cast<float>(size.width)) / static_cast<float>(Settings::instance().viewport.width) * 2.0f - 1.0f;
  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(programObject);
  glEnableVertexAttribArray(posALoc);
  glVertexAttribPointer(posALoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  glUniform2f(posLoc, static_cast<float>(position.x), static_cast<float>(position.y));
  glUniform2f(sizLoc, static_cast<float>(size.width), static_cast<float>(size.height));
  glUniform1fv(valLoc, VALUES, static_cast<GLfloat*>(vs));
  glUniform1f(opaLoc, 1.0f);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(posALoc);
  glUseProgram(0);
}

