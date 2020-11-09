#include <string>
#include <sstream>

#include "Utility.h"
#include "GLES.h"
#include "LogConsole.h"
#include "log.h"

#ifdef _DEBUG
#include <dlfcn.h>

namespace {

void* eglHandle = nullptr;
void* (*eglGetCurrentContextFunc)() = nullptr;
void* eglContext = nullptr;

void* getCurrentEGLContext() {
  if (eglGetCurrentContextFunc != nullptr)
    return eglGetCurrentContextFunc();
  return nullptr;
}

} // namespace

void initEGLFunctions() {
  eglHandle = dlopen("libEGL.so", RTLD_LAZY);
  if (eglHandle == nullptr)
      return;
  *(void **) (&eglGetCurrentContextFunc) = dlsym(eglHandle, "eglGetCurrentContext");
}

void setCurrentEGLContext() {
  eglContext = getCurrentEGLContext();
}

void assertCurrentEGLContext() {
  assert(eglContext == getCurrentEGLContext());
}

#else

void initEGLFunctions() { }
void setCurrentEGLContext() { }
void assertCurrentEGLContext() { }

#endif

void Utility::__logGLErrors__(const char *filename, int line) {
  assertCurrentEGLContext();

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

