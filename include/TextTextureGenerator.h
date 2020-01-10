#ifndef _TEXT_TEXTURE_GENERATOR_H_
#define _TEXT_TEXTURE_GENERATOR_H_

#include <unordered_map>
#include <string>
#include <algorithm>
#include <utility>
#include <vector>
#include <cmath>
#include <chrono>
#include <map>

#include "GLES.h"
#include <glm/vec2.hpp>
#include "Utility.h"

#ifndef _FT_FREETYPE_
#define _FT_FREETYPE_
#include <ft2build.h>
#include FT_FREETYPE_H
#endif // _FT_FREETYPE_

class TextTextureGenerator {
private:
  TextTextureGenerator();
  ~TextTextureGenerator();
  TextTextureGenerator(const TextTextureGenerator&) = delete;
  TextTextureGenerator& operator=(const TextTextureGenerator&) = delete;

  Size<GLint> maxTextureSize;

  const int CHARS = 128;
  const std::pair<int, int> charRange = {0, CHARS};
  const std::chrono::milliseconds textureGCTimeout = std::chrono::milliseconds(1000);

  struct Character {
    GLuint TextureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    glm::ivec2 advance;
  };

  struct FontFace {
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

  GLuint programObject = GL_INVALID_VALUE;
  GLuint samplerLoc = GL_INVALID_VALUE;
  GLuint colLoc     = GL_INVALID_VALUE;
  GLuint posLoc     = GL_INVALID_VALUE;
  GLuint texLoc     = GL_INVALID_VALUE;

  struct FontFaceKey {
    int id;
    int size;
    bool operator==(const FontFaceKey& other) const { return id == other.id && size == other.size; }
    std::size_t operator()(const FontFaceKey& k) const { return std::hash<int>()(k.id) ^ std::hash<int>()(k.size); }
  };

public:
  struct TextureKey {
    std::string text;
    Size<GLuint> size;
    int fontId;
    bool operator==(const TextureKey& other) const { return text == other.text && fontId == other.fontId && size == other.size; }
    std::size_t operator()(const TextureKey& k) const { return std::hash<std::string>()(k.text) ^ std::hash<int>()(k.fontId) ^ std::hash<GLuint>()(k.size.width) ^ std::hash<GLuint>()(k.size.height); }
  };

  struct TextureInfo {
  private:
    GLuint textureId;
    std::chrono::time_point<std::chrono::steady_clock> lastTimeAccessed;
    Size<GLuint> size;
    int fontId;
    FontFace font;

  public:
    TextureInfo(GLuint textureId, Size<GLuint> size, GLuint fontId, FontFace font)
    : textureId(textureId),
      lastTimeAccessed(std::chrono::steady_clock::now()),
      size(size),
      fontId(fontId),
      font(font) {
    }

    GLuint getTextureId() {
      lastTimeAccessed = std::chrono::steady_clock::now();
      return textureId;
    }

    const Size<GLuint>& getSize() {
      lastTimeAccessed = std::chrono::steady_clock::now();
      return size;
    }

    const int getFontId() {
      lastTimeAccessed = std::chrono::steady_clock::now();
      return fontId;
    }

    const std::chrono::time_point<std::chrono::steady_clock>& getLastTimeAccessed() {
      return lastTimeAccessed;
    }

    const FontFace getFont() {
      lastTimeAccessed = std::chrono::steady_clock::now();
      return font;
    }
  };

private:
  FT_Library ftLibrary;
  std::vector<FT_Face> faces;
  std::vector<FT_Byte*> facesData;
  std::unordered_map<FontFaceKey, FontFace, FontFaceKey> fonts;
  std::unordered_map<TextureKey, TextureInfo, TextureKey> generatedTextures;

  struct BrokenTextValue {
  private:
    std::string text;
    Size<GLuint> size;
    std::chrono::time_point<std::chrono::steady_clock> lastTimeAccessed;

  public:
    BrokenTextValue(std::string text, Size<int> size)
    : text(text),
      size(size),
      lastTimeAccessed(std::chrono::steady_clock::now()) {
    }

    const std::string& getText() {
        lastTimeAccessed = std::chrono::steady_clock::now();
        return text;
    }

    const Size<GLuint>& getSize() {
        lastTimeAccessed = std::chrono::steady_clock::now();
        return size;
    }

    const std::chrono::time_point<std::chrono::steady_clock>& getLastTimeAccessed() {
      return lastTimeAccessed;
    }
  };
  std::unordered_map<TextureKey, BrokenTextValue, TextureKey> brokenTexts;

  void prepareShaders();
  void advance(Position<float>& position, const char character, const FontFace& font, const bool invertVerticalAdvance = false);
  void breakLines(std::string &text, const FontFace &font, const float maxWidth);
  void printFramebufferError(const GLuint status);
  void gcTextures();
  void gcBrokenTextSizes();
  const char* getErrorMessage(const FT_Error error);

  TextureInfo generateTexture(TextureKey textureKey);
  const FontFace getFontFace(int fontId, int fontSize);
  FontFace generateFontFace(FontFaceKey fontFaceKey);
  Size<GLuint> getBrokenTextSize(const std::string text, int fontId, GLuint fontHeight);

public:
  static TextTextureGenerator& instance() {
    static TextTextureGenerator textTextureGenerator;
    return textTextureGenerator;
  }

  TextureInfo getTexture(TextureKey textureKey);
  int addFont(char *data, int size);

  Size<GLuint> getTextSize(TextureKey TextureKey);
  bool isFontValid(int fontId);
};

#endif // _TEXT_TEXTURE_GENERATOR_H_

