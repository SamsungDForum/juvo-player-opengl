#include <string>
#include <sstream>
#include <cassert>

#include "Utility.h"
#include "GLES.h"
#include "LogConsole.h"
#include "log.h"

EGLContext Utility::eglContext = 0;

void Utility::__logGLErrors__(const char *filename, int line) {
  Utility::assertCurrentEGLContext();

  for(GLenum err = GL_NO_ERROR; (err = glGetError()) != GL_NO_ERROR; ) {
    std::ostringstream oss;
    oss << "OpenGL Error: " << filename << ":" << line << ": [" << err << "] " << getGLErrorString(err);
    LogConsole::instance().log(oss.str(), LogConsole::LogLevel::Error);
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

void Utility::setCurrentEGLContext() {
  eglContext = eglGetCurrentContext();
}

void Utility::__assertCurrentEGLContext__(const char *filename, int line) {
  std::ostringstream oss;
  oss << "Wrong eglContext: :" << filename << ":" << line << ": ";
  assert((oss.str(), eglContext == eglGetCurrentContext()));
}
