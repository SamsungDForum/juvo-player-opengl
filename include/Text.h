#ifndef _TEXT_H_
#define _TEXT_H_

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

#include <glm/vec2.hpp>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <utility>
#include <vector>
#include <cmath>

#ifndef _FT_FREETYPE_
#define _FT_FREETYPE_
#include <ft2build.h>
#include FT_FREETYPE_H
#endif // _FT_FREETYPE_

#include "log.h"

class Text {
private:

  const int CHARS = 128;
  const std::pair<int, int> charRange = {0, CHARS};

  struct Character {
    GLuint TextureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    glm::ivec2 advance;
  };

  struct TextKey {
  public:
    std::string text;
    int fontId;
    bool operator==(const TextKey& other) const { return text == other.text && fontId == other.fontId; }
    std::size_t operator()(const TextKey& k) const { return std::hash<std::string>()(k.text) + std::hash<int>()(k.fontId); } // hash function
  };

  struct Font {
    int id;
    int size;
    std::unordered_map<GLchar, Character> ch;
    int units_per_EM;
    glm::ivec2 bboxMin;
    glm::ivec2 bboxMax;
    int ascender;
    int descender;
    int max_descend; // manually computed
    int max_bearingx; // manually computed
    int height;
    glm::ivec2 max_advance;
    int underline_position;
    int underline_thickness;
  };

  struct TextTexture {
    GLuint textureId;
    GLuint width;
    GLuint height;
    GLuint fontId;
  };

  GLuint programObject;
  GLuint samplerLoc = GL_INVALID_VALUE;
  GLuint colLoc     = GL_INVALID_VALUE;
  GLuint opaLoc     = GL_INVALID_VALUE;
  GLuint posLoc     = GL_INVALID_VALUE;
  GLuint texLoc     = GL_INVALID_VALUE;

  GLuint programObject2;
  GLuint samplerLoc2 = GL_INVALID_VALUE;
  GLuint colLoc2     = GL_INVALID_VALUE;
  GLuint opaLoc2     = GL_INVALID_VALUE;
  GLuint posLoc2     = GL_INVALID_VALUE;
  GLuint texLoc2     = GL_INVALID_VALUE;


  FT_Library ftLibrary;
  std::vector<Font> fonts;

  std::unordered_map<TextKey, TextTexture, TextKey> generatedTextures;

  void prepareShaders();
  void checkShaderCompileError(GLuint shader);
  std::pair<float, float> getTextSize(const std::string &text, int fontId, float scale);
  void advance(std::pair<float, float> &position, char character, int fontId, float scale = 1.0f, bool invertVerticalAdvance = false);
  void breakLines(std::string &text, int fontId, float w, float scale = 1.0f);
  float getScale(const std::pair<int, int> &size, int fontId, const std::pair<int, int> &viewport);
  TextTexture getTextTexture(const std::string &text, int fontId, bool cache);
  void renderTextTexture(TextTexture textTexture, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, std::vector<float> color);
  void renderDirect(std::string text, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, int fontId, std::vector<float> color, bool cache);
  void renderCached(std::string text, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, int fontId, std::vector<float> color, bool cache);

public:
  Text();
  ~Text();
  int AddFont(char *data, int size, int fontsize);
  std::pair<float, float> getTextSize(const std::string &text, const std::pair<int, int> &size, int fondId, const std::pair<int, int> &viewport);
  void render(std::string text, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, int fontId, std::vector<float> color, bool cache);
  bool removeFromCache(std::string text, int fontId);

  void switchRenderingMode() {
    renderingMode = !renderingMode;
    _INFO("Text rendering mode switched to %s.", renderingMode ? "cached" : "direct");
  }

  enum class Shadow {
    None,
    Single,
    Outline
  };

private:
  int renderingMode = 1; // 0 - direct, 1 - cached
  Shadow shadowMode = Shadow::Single;
};

#endif
