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

#ifndef _FT_FREETYPE_
#define _FT_FREETYPE_
#include <ft2build.h>
#include FT_FREETYPE_H
#endif // _FT_FREETYPE_

#include "log.h"

#define CACHED_RENDERING

class Text {
private:

  const int CHARS = 128;

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

    bool operator==(const TextKey& other) const {
      return text == other.text && fontId == other.fontId;
    }

    std::size_t operator()(const TextKey& k) const { // hash function
      return std::hash<std::string>()(k.text) + std::hash<int>()(k.fontId);
    }
  };

    GLuint samplerLoc = GL_INVALID_VALUE;
    GLuint colLoc = GL_INVALID_VALUE;
    GLuint opaLoc = GL_INVALID_VALUE;
    GLuint posLoc = GL_INVALID_VALUE;
    GLuint texLoc = GL_INVALID_VALUE;

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
  };

private:
  GLuint programObject;
  FT_Library ftLibrary;
  std::vector<Font> fonts;

  std::unordered_map<TextKey, TextTexture, TextKey> generatedTextures;

private:
  void prepareShaders();
  void checkShaderCompileError(GLuint shader);
  void renderToTexture(const std::string &text, const std::pair<int, int> &size, const std::pair<int, int> &viewport, int fontId);

public:
  Text();
  ~Text();
  int AddFont(char *data, int size, int fontsize);
  void render(std::string text, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, int fontId, std::vector<float> color, bool cache);
  std::pair<float, float> getTextSize(const std::string &text, const std::pair<int, int> &size, int fondId, const std::pair<int, int> &viewport);
  std::pair<float, float> getTextSize(const std::string &text, const std::pair<int, int> &size, int fondId, float scale);
  std::pair<float, float> getTextSize(const std::string &text, int fontId, float scale);
  void advance(std::pair<float, float> &position, char character, int fontId, float scale = 1.0f);
  void breakLines(std::string &text, int fontId, float w, float scale = 1.0f);
  float getScale(const std::pair<int, int> &size, int fontId, const std::pair<int, int> &viewport);
  TextTexture getTextTexture(const std::string &text, int fontId, bool cache);
  void render2(std::string text, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, int fontId, std::vector<float> color, bool cache);
  void renderTextTexture(TextTexture textTexture, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, std::vector<float> color);
};

Text::Text() {
  /*FT_Error error = */FT_Init_FreeType(&ftLibrary);
  prepareShaders();
}

Text::~Text() {
  FT_Done_FreeType(ftLibrary);
}

void Text::prepareShaders() {
  const GLchar* vShaderTexStr =
    "attribute vec4 a_position;    \n"
    "attribute vec2 a_texCoord;    \n"
    "varying vec2 v_texCoord;      \n"
    "void main()                   \n"
    "{                             \n"
    "  v_texCoord = a_texCoord;    \n"
    "  gl_Position = a_position;   \n"
    "}                             \n";

  const GLchar* fShaderTexStr =
    "precision mediump float;      \n"
    "uniform vec3 u_color;         \n"
    "varying vec2 v_texCoord;      \n"
    "uniform sampler2D s_texture;  \n"
    "uniform float u_opacity;      \n"
    "void main()                   \n"
    "{                             \n"
    "  gl_FragColor = vec4(1.0, 1.0, 1.0, texture2D(s_texture, v_texCoord).r * u_opacity) * vec4(u_color, 1.0); \n"
    "}                             \n";

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

  samplerLoc = glGetUniformLocation(programObject, "s_texture");
  colLoc = glGetUniformLocation(programObject, "u_color");
  opaLoc = glGetUniformLocation(programObject, "u_opacity");
  posLoc = glGetAttribLocation(programObject, "a_position");
  texLoc = glGetAttribLocation(programObject, "a_texCoord");
  //glBindAttribLocation(programObject, 0, "a_position");
}

int Text::AddFont(char *data, int size, int fontsize) {
  FT_Face ftFace;
  FT_Error error = FT_New_Memory_Face(ftLibrary, reinterpret_cast<FT_Byte*>(data), size, 0, &ftFace); // FT_Byte* is an alias for unsigned char*
  if(error)
    return -1;

  Font font;
  font.max_descend = 0;

  FT_Set_Pixel_Sizes(ftFace, 0, fontsize);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  for(GLubyte c = 0; c < 128; ++c) {
    if(FT_Load_Char(ftFace, c, FT_LOAD_RENDER)) {
      continue;
    }
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_LUMINANCE,
        ftFace->glyph->bitmap.width,
        ftFace->glyph->bitmap.rows,
        0,
        GL_LUMINANCE,
        GL_UNSIGNED_BYTE,
        ftFace->glyph->bitmap.buffer
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    Character character = {
      texture,
      glm::ivec2(ftFace->glyph->bitmap.width, ftFace->glyph->bitmap.rows),
      glm::ivec2(ftFace->glyph->bitmap_left, ftFace->glyph->bitmap_top),
      glm::ivec2(static_cast<GLuint>(ftFace->glyph->advance.x), static_cast<GLuint>(ftFace->glyph->advance.y))
    };
    _INFO("font[%d].ch[%d (%c)]: size={%d, %d}, bearing={%d, %d}, advance={%d, %d}", fonts.size(), c, isprint(c) ? c : '?', character.size.x, character.size.y, character.bearing.x, character.bearing.y, character.advance.x, character.advance.y);
    font.max_advance.x = std::max(font.max_advance.x, character.advance.x);
    font.max_advance.y = std::max(font.max_advance.y, character.advance.y);
    font.ch.insert(std::pair<GLchar, Character>(c, character));
    font.max_descend = std::max(font.max_descend, character.size.y - character.bearing.y);
    font.max_bearingx = std::max(font.max_bearingx, character.bearing.x);

  }

  font.id = fonts.size();
  font.size = fontsize;
  font.units_per_EM = ftFace->units_per_EM;
  font.bboxMin = {ftFace->bbox.xMin, ftFace->bbox.yMin};
  font.bboxMax = {ftFace->bbox.xMax, ftFace->bbox.yMax};
  font.ascender = ftFace->ascender;
  font.descender = ftFace->descender;
  font.height = ftFace->height * font.size / font.units_per_EM;
  font.underline_position = ftFace->underline_position;
  font.underline_thickness = ftFace->underline_thickness;

  _INFO("font[%d].size=%d", fonts.size(), font.size);
  _INFO("font[%d].height=%d", fonts.size(), font.height);
  _INFO("font[%d].units_per_EM=%d", fonts.size(), font.units_per_EM);
  _INFO("font[%d].bboxMin=%d", fonts.size(), font.bboxMin);
  _INFO("font[%d].bboxMax=%d", fonts.size(), font.bboxMax);
  _INFO("font[%d].ascender=%d", fonts.size(), font.ascender);
  _INFO("font[%d].descender=%d", fonts.size(), font.descender);
  _INFO("font[%d].underline_position=%d", fonts.size(), font.underline_position);
  _INFO("font[%d].underline_thickness=%d", fonts.size(), font.underline_thickness);
  _INFO("font[%d].max_advance.x=%d", fonts.size(), font.max_advance.x);
  _INFO("font[%d].max_advance.y=%d", fonts.size(), font.max_advance.y);
  _INFO("font[%d].max_descend=%d", fonts.size(), font.max_descend);
  _INFO("font[%d].max_bearingx=%d", fonts.size(), font.max_bearingx);

  fonts.push_back(font);
  FT_Done_Face(ftFace);
  return fonts.size() - 1;
}

float Text::getScale(const std::pair<int, int> &size, int fontId, const std::pair<int, int> &viewport) { // returns scale value for resizing from viewport px size (e.g. 1920x1080) to [0.0, 2.0] OGL size based on base and requested font height
  if(viewport.second == 0 || fonts[fontId].height == 0)
    return 1.0;
  return 2.0f * (static_cast<float>(size.second) / static_cast<float>(fonts[fontId].height)) / static_cast<float>(viewport.second);
}

std::pair<float, float> Text::getTextSize(const std::string &text, const std::pair<int, int> &size, int fontId, const std::pair<int, int> &viewport) { // returns text size in range [0.0, 2.0]
  float scale = getScale(size, fontId, viewport);
  return getTextSize(text, size, fontId, scale);
}

std::pair<float, float> Text::getTextSize(const std::string &text, const std::pair<int, int> &size, int fontId, float scale) { // returns text size in range [0.0, 2.0]
  std::string t = text;
  breakLines(t, fontId, size.first, scale);
  return getTextSize(t, fontId, scale);
}

std::pair<float, float> Text::getTextSize(const std::string &text, int fontId, float scale) {
  std::pair<float, float> position;
  float maxWidth = 0;
  for(std::string::const_iterator c = text.begin(); c != text.end(); ++c) {
    advance(position, *c, fontId, scale);
    maxWidth = std::max(maxWidth, position.first); // new lines reset position.first value
  }
  advance(position, '\n', fontId, scale);
  position.first = maxWidth;
  position.second = std::fabs(position.second); // we want size, not offset
  return position;
}

void Text::renderToTexture(const std::string &text, const std::pair<int, int> &size, const std::pair<int, int> &viewport, int fontId) {
}

void Text::advance(std::pair<float, float> &position, char character, int fontId, float scale) {
  if(character < 0 || character >= 128)
    return;
  if(character == '\n') {
    position.second -= fonts[fontId].height * scale;
    position.first = 0;
    return;
  }
  position.first += static_cast<float>(fonts[fontId].ch[character].advance.x >> 6) * scale; // advance is in 1/64px
}

void Text::breakLines(std::string &text, int fontId, float w, float scale) {
  // TODO: cache?

  if(w == 0)
    return;

  std::pair<float, float> position = {0, 0};
  std::pair<float, float> lastSpaceP = {0, 0};
  int lastSpaceI = -1;

  for(int i = 0, n = static_cast<int>(text.size()); i < n; ++i) {
    std::pair<float, float> newPosition = position;
    advance(newPosition, text[i], fontId, scale);
    if(newPosition.first >= w && (i > 0 && !isspace(text[i - 1]))) {
      if(lastSpaceI != -1) {
        text[lastSpaceI] = '\n';
        i = lastSpaceI;
      }
      else {
        text.insert(static_cast<size_t>(i), 1, '\n');
        ++n;
      }
      lastSpaceI = -1;
      position = lastSpaceP;
    }
    else if(isspace(text[i])) {
      lastSpaceI = i;
      lastSpaceP = position;
    }
    advance(position, text[i], fontId, scale);
  }
}

#ifdef CACHED_RENDERING
void Text::render(std::string text, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, int fontId, std::vector<float> color, bool cache) {
#else
void Text::render2(std::string text, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, int fontId, std::vector<float> color, bool cache) {
#endif
  if(color.size() >= 4 && color[3] == 0.0f) // if the text is fully transparent, we don't have to render it
    return;

  float scale = getScale(size, fontId, viewport);
  float textMaxWidth = size.first <= 0 ? 0 : 2.0 * static_cast<float>(size.first) / static_cast<float>(viewport.first) - fonts[fontId].max_bearingx * scale;
  if(size.first > 0)
    breakLines(text, fontId, textMaxWidth, scale);

  TextTexture textTexture = getTextTexture(text, fontId, cache);

  renderTextTexture(textTexture, position, size, viewport, color);

  if(!cache)
    glDeleteTextures(1, &textTexture.textureId);
}

Text::TextTexture Text::getTextTexture(const std::string &text, int fontId, bool cache) {
  TextKey tk {text, fontId};
  if(generatedTextures.count(tk))
    return generatedTextures.at(tk);

  float scale = 1.0;
  bool alignedToOrigin = false;
  std::pair<float, float> startingPos = {fonts[fontId].max_bearingx * scale, (alignedToOrigin ? 0 : fonts[fontId].max_descend) * scale};
  //std::pair<float, float> tSize = getTextSize(text, size, fontId, scale);
  std::pair<float, float> tSize = getTextSize(text, fontId, scale);
  //std::pair<GLuint, GLuint> texSize = {tSize.first * static_cast<float>(viewport.first) / 2.0f, tSize.second * static_cast<float>(viewport.second) / 2.0f};
  std::pair<GLuint, GLuint> texSize = {tSize.first / 2.0f, tSize.second / 2.0f};

  glActiveTexture(GL_TEXTURE0);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  GLuint framebuffer;
  GLuint depthRenderbuffer;
  GLuint texture;

  glGenFramebuffers(1, &framebuffer);
  glGenRenderbuffers(1, &depthRenderbuffer);
  glGenTextures(1, &texture);

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA,
      texSize.first,
      texSize.second,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      NULL
  );

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, texSize.first, texSize.second);

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

  GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status == GL_FRAMEBUFFER_COMPLETE) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO 1. set fbo texture shader and uniforms
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO 2. render the text glyph by glyph
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO

    std::pair<float, float> startingPos = {fonts[fontId].max_bearingx * scale, fonts[fontId].max_descend * scale};

    glUseProgram(programObject);

    std::pair<float, float> pos = {0.0f, 0.0f};
    std::string::const_iterator c;
    for(c = text.begin(); c != text.end(); ++c) {
      if(*c < 0 || *c >= 128)
        continue;

      if(isprint(*c)) {
        Character ch = fonts[fontId].ch[*c];

        float xpos = (pos.first + startingPos.first) + static_cast<float>(ch.bearing.x) * scale;
        float ypos = (pos.second + startingPos.second) - static_cast<float>(ch.size.y - ch.bearing.y) * scale;
        float w = static_cast<float>(ch.size.x) * scale;
        float h = static_cast<float>(ch.size.y) * scale;

        float vertices[] = {
          xpos,     ypos + h, 0.0f,
          xpos,     ypos,     0.0f,
          xpos + w, ypos,     0.0f,
          xpos,     ypos + h, 0.0f,
          xpos + w, ypos,     0.0f,
          xpos + w, ypos + h, 0.0f
        };

        float tex[] = {
          0.0, 0.0,
          0.0, 1.0,
          1.0, 1.0,
          0.0, 0.0,
          1.0, 1.0,
          1.0, 0.0
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glUniform1i(samplerLoc, 0);
  
        glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);
        glUniform1f(opaLoc, 1.0f);

        glEnableVertexAttribArray(posLoc);
        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

        glEnableVertexAttribArray(texLoc);
        glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, tex);

        glDrawArrays(GL_TRIANGLES, 0, 6);
      }
      advance(pos, *c, fontId, scale);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO 1. set fbo texture shader and uniforms
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO 2. render the text glyph by glyph
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO
    // TODO

  }
  else {
    _INFO("--- CREATING FRAMEBUFFER FOR TEXT RENDERING HAS FAILED! ---");
    switch(status) {
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        _INFO("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
        _INFO("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS");
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        _INFO("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
        break;
      case GL_FRAMEBUFFER_UNSUPPORTED:
        _INFO("GL_FRAMEBUFFER_UNSUPPORTED");
        break;
      case GL_INVALID_ENUM:
        _INFO("GL_INVALID_ENUM");
        break;
      case GL_INVALID_OPERATION:
        _INFO("GL_INVALID_OPERATION");
        break;
      default:
        _INFO("UNKNOWN ERROR: %d", status);
        break;
    }
  }

  glDeleteRenderbuffers(1, &depthRenderbuffer);
  glDeleteFramebuffers(1, &framebuffer);
  //glDeleteTextures(1, &texture);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  TextTexture tt {texture, texSize.first, texSize.second};
  if(cache)
    generatedTextures.insert(std::make_pair(tk, tt));
  return tt;
}

void Text::renderTextTexture(TextTexture textTexture, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, std::vector<float> color) {
  if(textTexture.textureId == GL_INVALID_VALUE) {
    _INFO("INVALID TEXT TEXTURE");
    return;
  }

  float zoom = 1.0;
  float w = textTexture.width; // TODO: scale???
  float h = textTexture.height; // TODO: scale???
  float xPos = position.first;
  float yPos = position.second;
  float leftPx = xPos - (w / 2.0) * (zoom - 1.0);
  float rightPx = (xPos + w) + (w / 2.0) * (zoom - 1.0);
  float downPx = yPos - (h / 2.0) * (zoom - 1.0);
  float topPx = (yPos + h) + (h / 2.0) * (zoom - 1.0);
  float left = (leftPx / viewport.first) * 2.0 - 1.0;
  float right = (rightPx / viewport.first) * 2.0 - 1.0;
  float down = (downPx / viewport.second) * 2.0 - 1.0;
  float top = (topPx / viewport.second) * 2.0 - 1.0;

  _INFO("Rendering text texture: left=%f, right=%f, up=%f, down=%f", left, right, top, down);

  float vertices[] = {
    left,  top,  0.0f,
    left,  down, 0.0f,
    right, down, 0.0f,
    left,  top,  0.0f,
    right, down, 0.0f,
    right, top,  0.0f
  };

  float tex[] = {
    0.0, 0.0,
    0.0, 1.0,
    1.0, 1.0,
    0.0, 0.0,
    1.0, 1.0,
    1.0, 0.0
  };

  glUseProgram(programObject);


  glBindTexture(GL_TEXTURE_2D, textTexture.textureId);
  glUniform1i(samplerLoc, 0);

  if(color.size() < 3)
    glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);
  else
    glUniform3f(colLoc, color[0], color[1], color[2]);

  if(color.size() < 4)
    glUniform1f(opaLoc, 1.0f);
  else
    glUniform1f(opaLoc, color[3]);

  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

  glEnableVertexAttribArray(texLoc);
  glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, tex);


  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
}

#ifdef CACHED_RENDERING
void Text::render2(std::string text, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, int fontId, std::vector<float> color, bool cache) {
#else
void Text::render(std::string text, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, int fontId, std::vector<float> color, bool cache) {
#endif
  bool alignedToOrigin = false; // TODO: add as a function parameter

  if(color.size() >= 4 && color[3] == 0.0f) // if the text is fully transparent, we don't have to render it
    return;

  std::pair<float, float> startingPos = {static_cast<float>(position.first) / static_cast<float>(viewport.first) * 2.0f - 1.0f, static_cast<float>(position.second) / static_cast<float>(viewport.second) * 2.0f - 1.0f};

  float scale = getScale(size, fontId, viewport);

  startingPos.first += fonts[fontId].max_bearingx * scale;
  startingPos.second += (alignedToOrigin ? 0 : fonts[fontId].max_descend) * scale;

  float textMaxWidth = size.first <= 0 ? 0 : 2.0 * static_cast<float>(size.first) / static_cast<float>(viewport.first) - fonts[fontId].max_bearingx * scale;
  if(size.first > 0)
    breakLines(text, fontId, textMaxWidth, scale);

  glUseProgram(programObject);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RENDERING TO TEXTURE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  std::pair<float, float> tSize = getTextSize(text, size, fontId, scale);
  _INFO("Rendering text texture %fx%f: %s", tSize.first, tSize.second, text.c_str());
  std::pair<GLuint, GLuint> texSize = {tSize.first * static_cast<float>(viewport.first) / 2.0f, tSize.second * static_cast<float>(viewport.second) / 2.0f};
  _INFO("Rendering text texture %ux%u: %s", texSize.first, texSize.second, text.c_str());

  glActiveTexture(GL_TEXTURE0);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  GLuint framebuffer;
  GLuint depthRenderbuffer;
  GLuint texture;

  glGenFramebuffers(1, &framebuffer);
  glGenRenderbuffers(1, &depthRenderbuffer);
  glGenTextures(1, &texture);

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA,
      texSize.first,
      texSize.second,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      NULL
  );

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, texSize.first, texSize.second);

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

  GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status == GL_FRAMEBUFFER_COMPLETE) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: set fbo texture shader and uniforms
    // TODO: draw
  }
  else {
    _INFO("--- CREATING FRAMEBUFFER FOR TEXT RENDERING HAS FAILED! ---");
    switch(status) {
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        _INFO("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
        _INFO("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS");
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        _INFO("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
        break;
      case GL_FRAMEBUFFER_UNSUPPORTED:
        _INFO("GL_FRAMEBUFFER_UNSUPPORTED");
        break;
      case GL_INVALID_ENUM:
        _INFO("GL_INVALID_ENUM");
        break;
      case GL_INVALID_OPERATION:
        _INFO("GL_INVALID_OPERATION");
        break;
      default:
        _INFO("UNKNOWN ERROR: %d", status);
        break;
    }
  }

  glDeleteRenderbuffers(1, &depthRenderbuffer);
  glDeleteFramebuffers(1, &framebuffer);
  glDeleteTextures(1, &texture); // TODO: save it instead of deleting it
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // TODO: restore state
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  std::pair<float, float> pos = {0.0f, 0.0f};
  std::string::const_iterator c;
  for(c = text.begin(); c != text.end(); ++c) {
    if(*c < 0 || *c >= 128)
      continue;

    if(isprint(*c)) {
      Character ch = fonts[fontId].ch[*c];

      float xpos = (pos.first + startingPos.first) + static_cast<float>(ch.bearing.x) * scale;
      float ypos = (pos.second + startingPos.second) - static_cast<float>(ch.size.y - ch.bearing.y) * scale;
      float w = static_cast<float>(ch.size.x) * scale;
      float h = static_cast<float>(ch.size.y) * scale;

      float vertices[] = {
        xpos,     ypos + h, 0.0f,
        xpos,     ypos,     0.0f,
        xpos + w, ypos,     0.0f,
        xpos,     ypos + h, 0.0f,
        xpos + w, ypos,     0.0f,
        xpos + w, ypos + h, 0.0f
      };

      float tex[] = {
        0.0, 0.0,
        0.0, 1.0,
        1.0, 1.0,
        0.0, 0.0,
        1.0, 1.0,
        1.0, 0.0
      };

      glBindTexture(GL_TEXTURE_2D, ch.TextureID);
      glUniform1i(samplerLoc, 0);
/*
      bool outline = true;
      if(outline) {
        float outlineOffset = 2.0 * scale;

        if(color.size() < 3)
          glUniform3f(colLoc, 0.0f, 0.0f, 0.0f);
        else if(color[0] + color[1] + color[2] < 1.5f)
          glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);
        else
          glUniform3f(colLoc, 0.0f, 0.0f, 0.0f);

        if(color.size() < 4)
          glUniform1f(opaLoc, 1.0f);
        else
          glUniform1f(opaLoc, color[3]);

        glEnableVertexAttribArray(texLoc);
        glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, tex);

        int dir[] = {
           0,  1,
          -1,  1,
          -1,  0,
          -1, -1,
           0, -1,
           1, -1,
           1,  0,
           1,  1
        };
        for(int i = 0; i < 8; ++i) {
          float outlineV[6 * 3];
          for(int j = 0; j < 6 * 3; ++j) {
            switch(j % 3) {
              case 0:
                outlineV[j] = vertices[j] + dir[2 * i] * outlineOffset;
                break;
              case 1:
                outlineV[j] = vertices[j] + dir[2 * i + 1] * outlineOffset;
                break;
              case 2:
                outlineV[j] = vertices[j];
                break;
            }
          }

          glEnableVertexAttribArray(posLoc);
          glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, outlineV);

          glDrawArrays(GL_TRIANGLES, 0, 6);
        }
      }
*/
      if(color.size() < 3)
        glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);
      else
        glUniform3f(colLoc, color[0], color[1], color[2]);

      if(color.size() < 4)
        glUniform1f(opaLoc, 1.0f);
      else
        glUniform1f(opaLoc, color[3]);

      glEnableVertexAttribArray(posLoc);
      glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

      glEnableVertexAttribArray(texLoc);
      glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, tex);

      glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    advance(pos, *c, fontId, scale);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
}

void Text::checkShaderCompileError(GLuint shader) {
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


#endif
