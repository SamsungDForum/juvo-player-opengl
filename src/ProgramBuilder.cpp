#include "ProgramBuilder.h"
#include "LogConsole.h"

#include <vector>
#include <string>

GLuint ProgramBuilder::buildProgram(const GLchar* vshader, const GLchar* fshader) {
  // compile shaders
  GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vshader);
  GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fshader);
  if(fragmentShader == GL_INVALID_VALUE || vertexShader == GL_INVALID_VALUE) {
    if(fragmentShader != GL_INVALID_VALUE)
      glDeleteShader(fragmentShader);
    if(vertexShader != GL_INVALID_VALUE)
      glDeleteShader(vertexShader);
    return GL_INVALID_VALUE;
  }

  // link program
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  GLint isLinked = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
  if(isLinked == GL_FALSE) {
    GLint maxLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

    std::vector<GLchar> infoLog(maxLength);
    glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
    glDeleteProgram(program);

    LogConsole::instance().log(std::string(infoLog.begin(), infoLog.end()).c_str(), LogConsole::LogLevel::Error);
    return GL_INVALID_VALUE;
  }
  return program;
}

GLuint ProgramBuilder::loadShader(const GLenum type, const GLchar* source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if(isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    std::vector<GLchar> infoLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
    glDeleteShader(shader);

    LogConsole::instance().log(std::string(infoLog.begin(), infoLog.end()).c_str(), LogConsole::LogLevel::Error);
    return GL_INVALID_VALUE;
  }
  return shader;
}


