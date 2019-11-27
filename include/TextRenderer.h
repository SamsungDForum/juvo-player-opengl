#ifndef _TEXT_RENDERER_H_
#define _TEXT_RENDERER_H_

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

#include <glm/vec2.hpp>

#include <string>
#include <vector>
#include <utility>

class TextRenderer {

private:
  TextRenderer();
  ~TextRenderer();
  TextRenderer(const TextRenderer&) = delete;
  TextRenderer& operator=(const TextRenderer&) = delete;
public:
  static TextRenderer& instance() {
    static TextRenderer textRenderer;
    return textRenderer;
  }

private:

  GLuint programObject = GL_INVALID_VALUE;
  GLuint posLoc        = GL_INVALID_VALUE;
  GLuint texLoc        = GL_INVALID_VALUE;
  GLuint colLoc        = GL_INVALID_VALUE;
  GLuint samplerLoc    = GL_INVALID_VALUE;
  GLuint shaColLoc     = GL_INVALID_VALUE;
  GLuint shaOffLoc     = GL_INVALID_VALUE;
  GLuint opaLoc        = GL_INVALID_VALUE;

  void prepareShaders();

public:
  int addFont(char *data, int size);
  std::pair<GLuint, GLuint> getTextSize(const std::string text, std::pair<GLuint, GLuint> size, int fontId);
  void render(std::string text, std::pair<int, int> position, std::pair<int, int> size, int fontId, std::vector<float> color);
};

#endif // _TEXT_RENDERER_H_
