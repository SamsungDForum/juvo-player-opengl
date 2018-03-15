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
    int height;
    glm::ivec2 max_advance;
    int underline_position;
    int underline_thickness;
  };

private:
  GLuint programObject;
  FT_Library ftLibrary;
  FT_Face ftFace;
  std::vector<Font> fonts;

  std::unordered_map<TextKey, GLuint, TextKey> generatedTextures;

private:
  void prepareShaders();
  void checkShaderCompileError(GLuint shader);

public:
  Text();
  ~Text();
  int AddFont(char *data, int size, int fontsize);
  void render(std::string text, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, int fontId, std::vector<float> color, bool cache);
  std::pair<int, int> getTextSize(std::string text, std::pair<int, int> size, int fondId);
  GLuint getCachedTexture(std::string text, int fontId);
  std::pair<int, int> getTextSize(std::string text, int fontId);
  void draw(std::string text, int fontId, std::vector<float> color);
  void advance(std::pair<float, float> &position, char character, int fontId);
  void breakLines(std::string &text, int fontId, float w);
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

  // TODO: when alpha is not 0.0 and not 1.0, instead of blending with background image, font blends with video...

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
  FT_Error error = FT_New_Memory_Face(ftLibrary, reinterpret_cast<FT_Byte*>(data), size, 0, &ftFace); // FT_Byte* is an alias for unsigned char*
  if(error)
    return -1;

  Font font;

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
    font.max_advance.x = std::max(font.max_advance.x, character.advance.x);
    font.max_advance.y = std::max(font.max_advance.y, character.advance.y);
    font.ch.insert(std::pair<GLchar, Character>(c, character));
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

  fonts.push_back(font);
  FT_Done_Face(ftFace);
  return fonts.size() - 1;
}

GLuint Text::getCachedTexture(std::string text, int fontId) {
  if(generatedTextures.count(TextKey{text, 0}))
    return generatedTextures.at(TextKey{text, 0});

  GLuint framebuffer;
  GLuint texture;
  // GLuint depthRenderbuffer;

  glGenFramebuffers(1, &framebuffer);
  //glGenRenderbuffers(1, &depthRenderbuffer);
  glGenTextures(1, &texture);

  glBindTexture(GL_TEXTURE_2D, texture);
  std::pair<int, int> size = getTextSize(text, fontId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.first, size.second, 0, GL_RGB, GL_UNSIGNED_BYTE/*GL_UNSIGNED_SHORT_5_6_5*/, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  //glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
  //glRenderbufferStprage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size.first, size.second);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
  //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status == GL_FRAMEBUFFER_COMPLETE) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT/* | GL_DEPTH_BUFFER_BIT*/);
    draw(text, fontId, {1.0f, 1.0f, 1.0f, 1.0f}); // set shaders and uniforms and draw to framebuffer
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0); // set rendering back to system framebuffer; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &result);
  //glDeleteRenderbuffers(1, &depthRenderbuffer);
  glDeleteFramebuffers(1, &framebuffer);
  //glDeleteTextures(1, &texture);

  generatedTextures.insert({TextKey{text, 0}, texture});
  return texture;
}

std::pair<int, int> Text::getTextSize(std::string text, std::pair<int, int> size, int fontId) {
  int width = 0;
  GLfloat fontH = 1;
  for(std::string::const_iterator c = text.begin(); c != text.end(); ++c)
    fontH = std::max(static_cast<float>(fontH), static_cast<float>(fonts[fontId].ch[*c].size.y));
  GLfloat scale = size.second / fontH;
  for(std::string::const_iterator c = text.begin(); c != text.end(); ++c) {
    Character ch = fonts[fontId].ch[*c];
    width += (ch.advance.x >> 6);
  }
  return {width * scale, size.second};

 /* std::pair<int, int> _size = getTextSize(text, fontId);
  GLfloat scale = size.second / fonts[fontId].height;
  return {_size.first * scale, _size.second};*/
}

std::pair<int, int> Text::getTextSize(std::string text, int fontId) {
  std::pair<float, float> position;
  for(std::string::const_iterator c = text.begin(); c != text.end(); ++c)
    advance(position, *c, fontId);
  advance(position, '\n', fontId);
  return static_cast<std::pair<int, int>>(position);
}

void Text::advance(std::pair<float, float> &position, char character, int fontId) {
  if(character < 0 || character >= 128)
    return;
  if(character == '\n') {
    position.second += fonts[fontId].height;
    position.first = 0;
    return;
  }
  Character ch = fonts[fontId].ch[character];
  position.first += static_cast<float>(ch.advance.x >> 6); // advance is in 1/64px
}

void Text::breakLines(std::string &text, int fontId, float w) {
  std::pair<float, float> position = {0, 0};
  std::pair<float, float> lastSpaceP = {0, 0};
  int lastSpaceI = -1;
  for(int i = 0; i != static_cast<int>(text.size()); ++i) {
    if(position.first >= w && lastSpaceI != -1) {
      text[lastSpaceI] = '\n';
      i = lastSpaceI;
      lastSpaceI = -1;
      position = lastSpaceP;
    }
    else if(text[i] == ' ') {
      lastSpaceI = i;
      lastSpaceP = position;
    }
    advance(position, text[i], fontId);
  }
}

void Text::draw(std::string text, int fontId, std::vector<float> color) {
  // TODO: Implement
}

void Text::render(std::string text, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, int fontId, std::vector<float> color, bool cache) {

  if(color.size() >= 4 && color[3] == 0.0f)
    return;

  float _vW = viewport.first;
  float _vH = viewport.second;
  float _h = size.second;
  float _xPos = position.first;
  float _yPos = position.second;
  float _left = _xPos / _vW * 2 - 1;
  float _down = _yPos / _vH * 2 - 1;

  if(size.first > 0)
    breakLines(text, fontId, size.first * 2.0);

  GLfloat x = _left;
  GLfloat y = _down;
  GLfloat fontH = 1;
  for(std::string::const_iterator c = text.begin(); c != text.end(); ++c)
    fontH = std::max(static_cast<float>(fontH), static_cast<float>(fonts[fontId].ch[*c].size.y));
  GLfloat scale = _h / fontH;

  glUseProgram(programObject);
  //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
  //glEnable(GL_BLEND);

  std::string::const_iterator c;
  for(c = text.begin(); c != text.end(); ++c) {
    if(*c < 0 || *c >= 128)
      continue;
    if(*c == '\n') {
      y -= fonts[fontId].height / _vH * scale * 2.0;
      x = _left;
      continue;
    }

    Character ch = fonts[fontId].ch[*c];

    GLfloat xpos = x + static_cast<float>(ch.bearing.x) / _vW * scale * 2.0;
    GLfloat ypos = y - static_cast<float>(ch.size.y - ch.bearing.y) / _vH * scale * 2.0;
    GLfloat w = static_cast<float>(ch.size.x) / _vW * scale * 2.0;
    GLfloat h = static_cast<float>(ch.size.y) / _vH * scale * 2.0;

    GLfloat vertices[] = {
      xpos,     ypos + h, 0.0f,
      xpos,     ypos,     0.0f,
      xpos + w, ypos,     0.0f,
      xpos,     ypos + h, 0.0f,
      xpos + w, ypos,     0.0f,
      xpos + w, ypos + h, 0.0f
    };

    GLfloat tex[] = {
      0.0, 0.0,
      0.0, 1.0,
      1.0, 1.0,
      0.0, 0.0,
      1.0, 1.0,
      1.0, 0.0
    };

    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
    glUniform1i(samplerLoc, 0);

    bool outline = true;
    if(outline) {
      GLfloat outlineOffset = 2.0 / _vW * scale * 2.0;

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
        GLfloat outlineV[6 * 3];
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

    x += static_cast<float>(ch.advance.x >> 6) / _vW * scale * 2.0; // advance is in 1/64px
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  glUseProgram(0);

}

/*void Text::render(std::string text) {
  return;

  GLfloat x = -0.9;
  GLfloat y = 0.8;
  GLfloat scale = 0.001;

  glUseProgram(programObject);
  //glActiveTexture(GL_TEXTURE0);

  std::string::const_iterator c;
  for(c = text.begin(); c != text.end(); ++c) {
    Character ch = fonts[0].ch[*c];

    GLfloat xpos = x + ch.bearing.x * scale;
    GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;
    GLfloat w = ch.size.x * scale;
    GLfloat h = ch.size.y * scale;

    GLfloat vertices[] = {
      xpos,     ypos + h, 0.0f,
      xpos,     ypos,     0.0f,
      xpos + w, ypos,     0.0f,
      xpos,     ypos + h, 0.0f,
      xpos + w, ypos,     0.0f,
      xpos + w, ypos + h, 0.0f
    };

    GLfloat tex[] = {
      0.0, 0.0,
      0.0, 1.0,
      1.0, 1.0,
      0.0, 0.0,
      1.0, 1.0,
      1.0, 0.0
    };

    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
    GLuint samplerLoc = glGetUniformLocation(programObject, "s_texture");
    glUniform1i(samplerLoc, 0);

    GLuint colLoc = glGetUniformLocation(programObject, "u_color");
    glUniform3f(colLoc, 0.0f, 1.0f, 0.0f);
    GLuint opaLoc = glGetUniformLocation(programObject, "u_opacity");
    glUniform1f(opaLoc, 1.0f);

    GLint posLoc = glGetAttribLocation(programObject, "a_position");
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

    GLint texLoc = glGetAttribLocation(programObject, "a_texCoord");
    glEnableVertexAttribArray(texLoc);
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, tex);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    x += (ch.advance >> 6) * scale; // advance is in 1/64px
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  glUseProgram(0);
}*/

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
