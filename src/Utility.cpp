#include <string>

#include "Utility.h"
#include "GLES.h"
#include "LogConsole.h"

void Utility::logGLErrors(const char *filename, int line) {
  for(GLenum err = GL_NO_ERROR; (err = glGetError()) != GL_NO_ERROR; ) {
    LogConsole::instance().log(std::string("OpenGL Error: ") + std::string(filename) + std::string(":") + std::to_string(line) + std::string(": [") + std::to_string(err) + std::string("] ") + getGLErrorString(err), LogConsole::LogLevel::Error);
  }
}

std::string Utility::getGLErrorString(int err) {
  switch (err)
  {
    case GL_NO_ERROR:          return "No Error";
    case GL_INVALID_ENUM:      return "Invalid Enum";
    case GL_INVALID_VALUE:     return "Invalid Value";
    case GL_INVALID_OPERATION: return "Invalid Operation";
    case GL_INVALID_FRAMEBUFFER_OPERATION: return "Invalid Framebuffer Operation";
    case GL_OUT_OF_MEMORY:     return "Out of Memory";
  }
  return "Unknown Error";
}

