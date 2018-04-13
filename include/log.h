// in order to compile/link, requires:
// 1. line in spec file: "BuildRequires:  pkgconfig(dlog)"
// 2. in cmake file: pkg_check_modules(PKGS REQUIRED ... dlog)

#ifndef __LOG_H__
#define __LOG_H__

#ifndef NO_TIZEN
#include <dlog.h>
#else
#include <stdio.h>
#define LOGE(fmt, args...) printf(fmt, ##args)
#define LOGD(fmt, args...) printf(fmt, ##args)
#define LOGI(fmt, args...) printf(fmt, ##args)
#endif

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "JuvoPlayer"
#ifndef _ERR
#define _ERR(fmt, args...) LOGE(fmt "\n", ##args)
#endif

#ifndef _DBG
#define _DBG(fmt, args...) LOGD(fmt "\n", ##args)
#endif

#ifndef _INFO
#define _INFO(fmt, args...) LOGI(fmt "\n", ##args)
#endif



/*
#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

void checkShaderCompileError(GLuint shader) {
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
*/

#endif /* __LOG_H__ */
