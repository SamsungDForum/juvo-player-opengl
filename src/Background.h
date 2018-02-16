#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_

#include <string>
#include <chrono>
#include <utility>

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

#include "Tile.h"
#include "TileAnimation.h"
#include "Text.h"
#include "log.h"

class Background {
  private:
    GLuint programObject = GL_INVALID_VALUE;
    GLuint textureFormat = GL_INVALID_VALUE;
    int viewportWidth;
    int viewportHeight;
    float opacity;
    Tile *sourceTile;

  private:
    void initGL();
    void checkShaderCompileError(GLuint shader);

  public:
    Background();
    Background(int viewportWidth, int viewportHeight, float opacity);
    ~Background();
    void render(Text &text);
    void setOpacity(float opacity);
    void setViewport(int viewportWidth, int viewportHeight);
    void setSourceTile(Tile *sourceTile);
};

Background::Background()
  : programObject(GL_INVALID_VALUE),
    textureFormat(GL_INVALID_VALUE),
    viewportWidth(0),
    viewportHeight(0),
    opacity(0),
    sourceTile(nullptr) {
  initGL();

}

Background::Background(int viewportWidth, int viewportHeight, float opacity)
  : programObject(GL_INVALID_VALUE),
    textureFormat(GL_INVALID_VALUE),
    viewportWidth(viewportWidth),
    viewportHeight(viewportHeight),
    opacity(opacity),
    sourceTile(nullptr) {
  initGL();
}

Background::~Background() {
}

void Background::initGL() {
  const GLchar* vShaderTexStr =  
    "attribute vec4 a_position;     \n"
    "attribute vec2 a_texCoord;     \n"
    "varying vec2 v_texCoord;       \n"
    "void main()                    \n"
    "{                              \n"
    "  v_texCoord = a_texCoord;     \n"
    "  gl_Position = a_position;    \n"
    "}                              \n";
 
  const GLchar* fShaderTexStr =  
    "precision mediump float;       \n"
    "varying vec2 v_texCoord;       \n"
    "uniform sampler2D s_texture;   \n"
    "uniform float u_opacity;       \n"
    "void main()                    \n"
    "{                              \n"
    "  gl_FragColor                 \n"
    "   = texture2D(s_texture,      \n"
    "               v_texCoord);    \n"
    "  gl_FragColor.a = u_opacity * pow(gl_FragCoord.y / 1920.0, 2.0) * pow(gl_FragCoord.x / 1080.0, 0.5);  \n"
    "}                              \n";

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

  glBindAttribLocation(programObject, 0, "a_position");
}

void Background::render(Text &text) {

  GLuint textureId = sourceTile != nullptr ? sourceTile->getTextureId() : GL_INVALID_VALUE;
  opacity = sourceTile != nullptr ? sourceTile->getOpacity() : 1.0;

  glClearColor(0.0f, 0.0f, 0.0f, opacity);
  glClear(GL_COLOR_BUFFER_BIT);
  opacity *= 1.0;

  if(textureId == GL_INVALID_VALUE)
    return;

  /*
  std::pair<int, int> position {0, 0};
  std::pair<int, int> size {0, 0};
  float zoom = 0;
  animation.update(position, zoom, size, opacity);
  */

  float left = -1.0;
  float right = 1.0;
  float top = 1.0;
  float down = -1.0;

  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(programObject);

  glBindTexture(GL_TEXTURE_2D, textureId);
  GLuint samplerLoc = glGetUniformLocation(programObject, "s_texture");
  glUniform1i(samplerLoc, 0);

  GLint posLoc = glGetAttribLocation(programObject, "a_position");
  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  float texCoord[] = { 0.0f, 0.0f,    0.0f, 1.0f,
                       1.0f, 1.0f,    1.0f, 0.0f };
                       
  GLint texLoc = glGetAttribLocation(programObject, "a_texCoord");
  glEnableVertexAttribArray(texLoc);
  glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoord);

  GLuint opacityLoc = glGetUniformLocation(programObject, "u_opacity");
  glUniform1f(opacityLoc, static_cast<GLfloat>(opacity));
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  std::string name = sourceTile != nullptr ? sourceTile->getName() : "";
  if(!name.empty()) {
    int fontHeight = 48;
    int leftText = 100;
    int topText = viewportHeight - fontHeight - 200;
    text.render(name, {leftText, topText}, {0, fontHeight}, {viewportWidth, viewportHeight}, 0, {1.0, 1.0, 1.0, opacity}, true);
  }
  std::string description = sourceTile != nullptr ? sourceTile->getDescription() : "";
  if(!description.empty()) {
    int fontHeight = 24;
    int leftText = 100;
    int topText = viewportHeight - fontHeight - 300;
    text.render(description, {leftText, topText}, {0, fontHeight}, {viewportWidth, viewportHeight}, 0, {1.0, 1.0, 1.0, opacity}, true);
  }
}

void Background::setOpacity(float opacity) {
  this->opacity = opacity;
}

void Background::checkShaderCompileError(GLuint shader) {
  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if(isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
    _ERR("%s", (std::string(errorLog.begin(), errorLog.end()).c_str()));

    glDeleteShader(shader); // Don't leak the shader.
  }
}

void Background::setViewport(int viewportWidth, int viewportHeight) {
  this->viewportWidth = viewportWidth;
  this->viewportHeight = viewportHeight;
}


void Background::setSourceTile(Tile *sourceTile) {
  this->sourceTile = sourceTile;
}
#endif // _BACKGRODUN_H_
