#include "TextTextureGenerator.h"
#include "ProgramBuilder.h"
#include "Settings.h"
#include "LogConsole.h"
#include "log.h"

#include <stdexcept>
#include <algorithm>

TextTextureGenerator::TextTextureGenerator()
  : textureGCTimeout(1000) {
  FT_Error error = FT_Init_FreeType(&ftLibrary);
  if(error != FT_Err_Ok)
    throw std::runtime_error(getErrorMessage(error));
  prepareShaders();
}

TextTextureGenerator::~TextTextureGenerator() {
  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
  for(auto& texture : generatedTextures) {
    GLuint id = texture.second.getTextureId();
    glDeleteTextures(1, &id);
  }
  for(FT_Face& face : faces)
    FT_Done_Face(face);
  FT_Done_FreeType(ftLibrary);
}

void TextTextureGenerator::prepareShaders() {

  const GLchar* vShaderTexStr =
#include "shaders/textTextureGenerator.vert"
;

  const GLchar* fShaderTexStr =
#include "shaders/textTextureGenerator.frag"
;

  programObject = ProgramBuilder::buildProgram(vShaderTexStr, fShaderTexStr);

  samplerLoc = glGetUniformLocation(programObject, "s_texture");
  colLoc = glGetUniformLocation(programObject, "u_color");
  posLoc = glGetAttribLocation(programObject, "a_position");
  texLoc = glGetAttribLocation(programObject, "a_texCoord");
}

int TextTextureGenerator::addFont(char *data, int size) {
  FT_Face ftFace;
  FT_Error error = FT_New_Memory_Face(ftLibrary, reinterpret_cast<FT_Byte*>(data), size, 0, &ftFace); // FT_Byte* is an alias for unsigned char*

  if(error) {
    LogConsole::instance().log("Cannot create new FT_Face from data", LogConsole::LogLevel::Error);
    return -1;
  }

  faces.push_back(ftFace);
  return faces.size() - 1;
}

const TextTextureGenerator::FontFace TextTextureGenerator::getFontFace(int fontId, int fontSize) {
  FontFaceKey fontFaceKey = FontFaceKey {
    .id = fontId,
    .size = fontSize
  };

  auto search = fonts.find(fontFaceKey);
  if(search != fonts.end())
    return search->second;

  FontFace font = generateFontFace(fontFaceKey);
  fonts.insert({ fontFaceKey, font });
  return fonts.find(fontFaceKey)->second;
}

TextTextureGenerator::FontFace TextTextureGenerator::generateFontFace(FontFaceKey fontFaceKey) {
  if(!TextTextureGenerator::instance().isFontValid(fontFaceKey.id)) {
    LogConsole::instance().log("no such fontId", LogConsole::LogLevel::Error);
    throw std::out_of_range("no such fontId");
  }
  FT_Face ftFace = faces[fontFaceKey.id];
  FontFace font;
  font.max_descend = 0;
  font.max_advance = {0, 0};
  font.max_descend = 0;
  font.max_bearingx = 0;
  FT_Set_Pixel_Sizes(ftFace, 0, fontFaceKey.size);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for(GLubyte c = charRange.first; c < charRange.second; ++c) {
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
    font.max_descend = std::max(font.max_descend, character.size.y - character.bearing.y);
    font.max_bearingx = std::max(font.max_bearingx, character.bearing.x);
  }

  font.id = fonts.size();
  font.size = fontFaceKey.size;
  font.units_per_EM = ftFace->units_per_EM;
  font.bboxMin = {ftFace->bbox.xMin, ftFace->bbox.yMin};
  font.bboxMax = {ftFace->bbox.xMax, ftFace->bbox.yMax};
  font.ascender = ftFace->ascender;
  font.descender = ftFace->descender;
  font.height = ftFace->height * font.size / font.units_per_EM;
  font.underline_position = ftFace->underline_position;
  font.underline_thickness = ftFace->underline_thickness;

  return font;
}

TextTextureGenerator::TextureInfo TextTextureGenerator::getTexture(TextTextureGenerator::TextureKey textureKey) {
  if(!TextTextureGenerator::instance().isFontValid(textureKey.fontId))
    throw std::out_of_range("no such fontId");

  if(generatedTextures.count(textureKey))
    return generatedTextures.at(textureKey);

  gcTextures();
  gcBrokenTextSizes();
  TextureInfo textureInfo = generateTexture(textureKey);
  generatedTextures.insert(std::make_pair(textureKey, textureInfo));

  return textureInfo;
}

TextTextureGenerator::TextureInfo TextTextureGenerator::generateTexture(TextTextureGenerator::TextureKey textureKey) { // TODO: Rasterize to SDFs?
  std::pair<GLuint, GLuint> texSize = getTextSize(textureKey);
  const FontFace& font = getFontFace(textureKey.fontId, textureKey.size.second);

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

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::pair<float, float> startingPos = {static_cast<float>(font.max_bearingx),
                                           -static_cast<float>(font.max_descend)};

    glUseProgram(programObject);
    glViewport(0, 0, texSize.first, texSize.second);

    std::pair<float, float> pos{0.0f, 0.0f};
    std::string::const_iterator c;

    GLuint textMaxWidth = textureKey.size.first > static_cast<GLuint>(font.max_bearingx) ? textureKey.size.first - static_cast<GLuint>(font.max_bearingx) : 0u;
    std::string text = textureKey.text; // we'll be modifying copy
    breakLines(text, font, textMaxWidth);

    for(c = text.begin(); c != text.end(); ++c) {
      if(*c < charRange.first || *c >= charRange.second)
        continue;

      if(isprint(*c)) {
        Character ch = font.ch.at(*c);

        float xpos = (pos.first + startingPos.first) + static_cast<float>(ch.bearing.x);
        float ypos = (pos.second + startingPos.second) + (static_cast<float>(font.height) - static_cast<float>(ch.size.y)) + static_cast<float>(ch.size.y - ch.bearing.y);
        float w = static_cast<float>(ch.size.x);
        float h = static_cast<float>(ch.size.y);
        float left = xpos * 2.0f / texSize.first - 1.0f;
        float right = (xpos + w) * 2.0f / texSize.first - 1.0f;
        float top = ypos * 2.0f / texSize.second - 1.0f;
        float down = (ypos + h) * 2.0f / texSize.second - 1.0f;

        GLfloat vVertices[] = { left,   top,  0.0f,
                                left,   down, 0.0f,
                                right,  down, 0.0f,
                                right,  top,  0.0f
        };
        GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
        float texCoord[] = { 0.0f, 0.0f,    0.0f, 1.0f,
                             1.0f, 1.0f,    1.0f, 0.0f };
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glUniform1i(samplerLoc, 0);
        glEnableVertexAttribArray(texLoc);
        glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoord);

        glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);
        glEnableVertexAttribArray(posLoc);
        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
      }
      advance(pos, *c, font, true);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
  }
  else {
    printFramebufferError(status);
    throw std::runtime_error("Framebuffer error while rasterizing glyphs.");
  }

  glDeleteRenderbuffers(1, &depthRenderbuffer);
  glDeleteFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, Settings::instance().viewport.first, Settings::instance().viewport.second); // restore previous viewport

  return TextureInfo(texture, texSize, textureKey.fontId, font);
}

void TextTextureGenerator::printFramebufferError(const GLuint status) {
    LogConsole::instance().log("--- CREATING FRAMEBUFFER FOR TEXT RENDERING HAS FAILED! ---", LogConsole::LogLevel::Error);
    switch(status) {
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        LogConsole::instance().log("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT", LogConsole::LogLevel::Error);
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
        LogConsole::instance().log("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS", LogConsole::LogLevel::Error);
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        LogConsole::instance().log("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT", LogConsole::LogLevel::Error);
        break;
      case GL_FRAMEBUFFER_UNSUPPORTED:
        LogConsole::instance().log("GL_FRAMEBUFFER_UNSUPPORTED", LogConsole::LogLevel::Error);
        break;
      case GL_INVALID_ENUM:
        LogConsole::instance().log("GL_INVALID_ENUM", LogConsole::LogLevel::Error);
        break;
      case GL_INVALID_OPERATION:
        LogConsole::instance().log("GL_INVALID_OPERATION", LogConsole::LogLevel::Error);
        break;
      default:
        LogConsole::instance().log(std::string("UNKNOWN ERROR: %d" + std::to_string(status)), LogConsole::LogLevel::Error);
        break;
    }
}

void TextTextureGenerator::advance(std::pair<float, float>& position, const char character, const FontFace& font, const bool invertVerticalAdvance) { // TODO: Implement kerning.
  if(character < charRange.first || character >= charRange.second)
    return;
  if(character == '\n') {
    position.second -= font.height * (invertVerticalAdvance ? -1.0 : 1.0);
    position.first = 0.0f;
    return;
  }
  position.first += static_cast<float>(font.ch.at(character).advance.x >> 6); // advance is in 1/64px
}

void TextTextureGenerator::breakLines(std::string &text, const FontFace &font, const float maxWidth) {
  if(maxWidth < 1)
    return;

  std::pair<float, float> position = {0, 0};
  std::pair<float, float> lastSpacePosition = {0, 0};
  int lastSpaceIndex = -1;

  for(int i = 0, n = static_cast<int>(text.size()); i < n; ++i) {
    std::pair<float, float> newPosition = position;
    advance(newPosition, text[i], font);
    if(newPosition.first >= maxWidth && (i > 0 && !isspace(text[i - 1]))) {
      if(lastSpaceIndex != -1) {
        text[lastSpaceIndex] = '\n';
        i = lastSpaceIndex;
      }
      else {
        text.insert(static_cast<size_t>(i), 1, '\n');
        ++n;
      }
      lastSpaceIndex = -1;
      position = lastSpacePosition;
    }
    else if(isspace(text[i])) {
      lastSpaceIndex = i;
      lastSpacePosition = position;
    }
    advance(position, text[i], font);
  }
}

std::pair<GLuint, GLuint> TextTextureGenerator::getTextSize(TextureKey textureKey) {
  if(!isFontValid(textureKey.fontId))
    return std::pair<GLuint, GLuint>({1, 1});

  auto search = brokenTexts.find(textureKey);
  if(search != brokenTexts.end())
    return std::pair<GLuint, GLuint>(search->second.getSize());

  const FontFace& font = getFontFace(textureKey.fontId, textureKey.size.second); 
  GLuint textMaxWidth = textureKey.size.first > static_cast<GLuint>(font.max_bearingx) ? textureKey.size.first - static_cast<GLuint>(font.max_bearingx) : 0u;

  std::string text = textureKey.text;
  breakLines(text, font, textMaxWidth);
  std::pair<GLuint, GLuint> brokenTextSize = getBrokenTextSize(text, textureKey.fontId, textureKey.size.second);
  brokenTexts.insert({ textureKey, BrokenTextValue(text, brokenTextSize) });
  return brokenTextSize;
}

std::pair<GLuint, GLuint> TextTextureGenerator::getBrokenTextSize(const std::string text, int fontId, GLuint fontHeight) {

  const FontFace& font = getFontFace(fontId, fontHeight); 
  std::pair<float, float> position{0.0f, 0.0f};
  float maxWidth = 0.0f;

  for(std::string::const_iterator c = text.begin(); c != text.end(); ++c) {
    advance(position, *c, font);
    maxWidth = std::max(maxWidth, position.first); // new lines reset position.first value
  }
  advance(position, '\n', font);
  position.first = maxWidth + static_cast<float>(font.max_bearingx);
  position.second = std::fabs(position.second); // we want size, not offset
  return static_cast<std::pair<GLuint, GLuint>>(position);
}

void TextTextureGenerator::gcTextures() {
  std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
  auto it = generatedTextures.begin();
  while(it != generatedTextures.end()) {
    if(std::chrono::duration_cast<std::chrono::duration<double>>(now - it->second.getLastTimeAccessed()) >= textureGCTimeout) {
      GLuint id = it->second.getTextureId();
      glDeleteTextures(1, &id);
      it = generatedTextures.erase(it);
    }
    else
      ++it;
  }
}

void TextTextureGenerator::gcBrokenTextSizes() {
  std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
  auto it = brokenTexts.begin();
  while(it != brokenTexts.end()) {
    if(std::chrono::duration_cast<std::chrono::duration<double>>(now - it->second.getLastTimeAccessed()) >= textureGCTimeout)
      it = brokenTexts.erase(it);
    else
      ++it;
  }
}

const char* TextTextureGenerator::getErrorMessage(const FT_Error error) {
  #undef __FTERRORS_H__
  #define FT_ERRORDEF(e, v, s)  case e: return s;
  #define FT_ERROR_START_LIST   switch (error) {
  #define FT_ERROR_END_LIST     }
  #include FT_ERRORS_H
  return "(Unknown error)";
}

bool TextTextureGenerator::isFontValid(int fontId) {
  return static_cast<size_t>(fontId) < faces.size();
}

