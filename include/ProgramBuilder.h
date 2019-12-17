#ifndef _PROGRAM_BUILDER_H_
#define _PROGRAM_BUILDER_H_

#include "GLES.h"

class ProgramBuilder {
public:
  static GLuint buildProgram(const GLchar* vshader, const GLchar* fshader);
private:
  static GLuint loadShader(const GLenum type, const GLchar* source);
};

#endif // _PROGRAM_BUILDER_H_
