#ifndef _PROGRAM_BUILDER_H_
#define _PROGRAM_BUILDER_H_

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

#include "log.h" // TODO: LogConsole? Everywhere?

class ProgramBuilder {
public:
  static GLuint buildProgram(const GLchar* vshader, const GLchar* fshader);
private:
  static GLuint loadShader(const GLenum type, const GLchar* source);
};

#endif // _PROGRAM_BUILDER_H_
