#include "Text.h"
#include "ProgramBuilder.h"
#include "Settings.h"

void Text::render(std::string text, std::pair<int, int> position, std::pair<int, int> size, int fontId, std::vector<float> color, bool cache) {
  if(validFontId(fontId))
    return;
  if(!renderingMode)
    renderDirect(text, position, size, fontId, color, cache);
  else
    renderCached(text, position, size, fontId, color, cache);
}

Text::Text()
  : textureGCTimeout(1000) {
  /*FT_Error error = */FT_Init_FreeType(&ftLibrary); // TODO(g.skowinski): Handle error
  prepareShaders();
}

Text::~Text() {
  FT_Done_FreeType(ftLibrary);
  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
  if(programObject2 != GL_INVALID_VALUE)
    glDeleteProgram(programObject2);
}

void Text::prepareShaders() {

  const GLchar* vShaderTexStr =
#include "shaders/textRenderer.vert"
;

  const GLchar* fShaderTexStr =
    "precision mediump float;                                              \n"
    "uniform vec3 u_color;                                                 \n"
    "varying vec2 v_texCoord;                                              \n"
    "uniform sampler2D s_texture;                                          \n"
    "uniform float u_opacity;                                              \n"
    "void main()                                                           \n"
    "{                                                                     \n"
    "  gl_FragColor = vec4(1.0, 1.0, 1.0,                                  \n"
    "                      texture2D(s_texture, v_texCoord).r * u_opacity) \n"
    "                 * vec4(u_color, 1.0);                                \n"
    "}                                                                     \n";

  programObject = ProgramBuilder::buildProgram(vShaderTexStr, fShaderTexStr);

  samplerLoc = glGetUniformLocation(programObject, "s_texture");
  colLoc = glGetUniformLocation(programObject, "u_color");
  opaLoc = glGetUniformLocation(programObject, "u_opacity");
  posLoc = glGetAttribLocation(programObject, "a_position");
  texLoc = glGetAttribLocation(programObject, "a_texCoord");

  const GLchar* fShaderTexStr2 =
    "precision mediump float;                          \n"
    "uniform vec3 u_color;                             \n"
    "varying vec2 v_texCoord;                          \n"
    "uniform sampler2D s_texture;                      \n"
    "uniform float u_opacity;                          \n"
    "void main()                                       \n"
    "{                                                 \n"
    "  gl_FragColor = texture2D(s_texture, v_texCoord) \n"
    "                 * vec4(u_color, u_opacity);      \n"
    "}                                                 \n";

  programObject2 = ProgramBuilder::buildProgram(vShaderTexStr, fShaderTexStr2);

  samplerLoc2 = glGetUniformLocation(programObject2, "s_texture");
  colLoc2 = glGetUniformLocation(programObject2, "u_color");
  opaLoc2 = glGetUniformLocation(programObject2, "u_opacity");
  posLoc2 = glGetAttribLocation(programObject2, "a_position");
  texLoc2 = glGetAttribLocation(programObject2, "a_texCoord");
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

float Text::getScale(const std::pair<int, int> &size, int fontId, std::pair<int, int> viewport) { // returns scale value for resizing from viewport px size (e.g. 1920x1080) to [0.0, 2.0] OGL size based on base and requested font height
  if(validFontId(fontId))
    return 1.0;
  if(Settings::viewport.second == 0 || fonts[fontId].height == 0)
    return 1.0;
  return 2.0f * (static_cast<float>(size.second) / static_cast<float>(fonts[fontId].height)) / static_cast<float>(Settings::viewport.second);
}

std::pair<float, float> Text::getTextSize(const std::string &text, const std::pair<int, int> &size, int fontId) { // returns text size in range [0.0, 2.0]
  if(validFontId(fontId))
    return {0.0f, 0.0f};
  float scale = getScale(size, fontId, Settings::viewport);
  float textMaxWidth = size.first <= 0 ? 0 : 2.0 * static_cast<float>(size.first) / static_cast<float>(Settings::viewport.first) - fonts[fontId].max_bearingx * scale;
  std::string t = text;
  breakLines(t, fontId, textMaxWidth, scale); 
  return getTextSize(t, fontId, scale);
}

std::pair<float, float> Text::getTextSize(const std::string &text, int fontId, float scale) {
  if(validFontId(fontId))
    return {0.0f, 0.0f};
  std::pair<float, float> position;
  float maxWidth = 0;
  for(std::string::const_iterator c = text.begin(); c != text.end(); ++c) {
    advance(position, *c, fontId, scale);
    maxWidth = std::max(maxWidth, position.first); // new lines reset position.first value
  }
  advance(position, '\n', fontId, scale);
  position.first = maxWidth + fonts[fontId].max_bearingx * scale;
  position.second = std::fabs(position.second); // we want size, not offset
  return position;
}

void Text::advance(std::pair<float, float> &position, char character, int fontId, float scale, bool invertVerticalAdvance) {
  if(validFontId(fontId))
    return;
  if(character < charRange.first || character >= charRange.second)
    return;
  if(character == '\n') {
    position.second -= fonts[fontId].height * scale * (invertVerticalAdvance ? -1.0 : 1.0);
    position.first = 0.0f;
    return;
  }
  position.first += static_cast<float>(fonts[fontId].ch[character].advance.x >> 6) * scale; // advance is in 1/64px
}

void Text::breakLines(std::string &text, int fontId, float w, float scale) {
  if(validFontId(fontId))
    return;
  if(w == 0)
    return;

  std::pair<float, float> position = {0, 0};
  std::pair<float, float> lastSpacePosition = {0, 0};
  int lastSpaceIndex = -1;

  for(int i = 0, n = static_cast<int>(text.size()); i < n; ++i) {
    std::pair<float, float> newPosition = position;
    advance(newPosition, text[i], fontId, scale);
    if(newPosition.first >= w && (i > 0 && !isspace(text[i - 1]))) {
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
    advance(position, text[i], fontId, scale);
  }
}

void Text::renderCached(std::string text, std::pair<int, int> position, std::pair<int, int> size, int fontId, std::vector<float> color, bool cache) {
  if(validFontId(fontId))
    return;
  if(color.size() >= 4 && color[3] == 0.0f) // if the text is fully transparent, we don't have to render it
    return;

  float scale = getScale(size, fontId, Settings::viewport);
  float textMaxWidth = size.first <= 0 ? 0 : 2.0 * static_cast<float>(size.first) / static_cast<float>(Settings::viewport.first) - fonts[fontId].max_bearingx * scale;
  if(size.first > 0)
    breakLines(text, fontId, textMaxWidth, scale);

  TextTexture textTexture = getTextTexture(text, fontId, cache);

  renderTextTexture(textTexture, position, size, color);

  if(!cache)
    glDeleteTextures(1, &textTexture.textureId);

  deleteUnusedTextures();
}

bool Text::removeFromCache(std::string text, int fontId) {
  if(validFontId(fontId))
    return false;
  TextKey tk {text, fontId};
  if(generatedTextures.count(tk)) {
    generatedTextures.erase(tk);
    return true;
  }
  return false;
}

Text::TextTexture Text::getTextTexture(const std::string &text, int fontId, bool cache) {
  if(validFontId(fontId))
    return TextTexture{};
  TextKey tk {text, fontId};
  if(generatedTextures.count(tk)) {
    generatedTextures.at(tk).lastUsed = std::chrono::high_resolution_clock::now();
    return generatedTextures.at(tk);
  }

  float scale = 1.0;
  std::pair<float, float> texSize = getTextSize(text, fontId, scale);

  //_INFO("Generating texture of size {%f, %f} for text \"%s\"", texSize.first, texSize.second, text.c_str());

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

    std::pair<float, float> startingPos = {static_cast<float>(fonts[fontId].max_bearingx),
                                           -static_cast<float>(fonts[fontId].max_descend)};

    glUseProgram(programObject);
    glViewport(0, 0, texSize.first, texSize.second);

    std::pair<float, float> pos = {0.0f, 0.0f};
    std::string::const_iterator c;
    for(c = text.begin(); c != text.end(); ++c) {
      if(*c < charRange.first || *c >= charRange.second)
        continue;

      if(isprint(*c)) {
        Character ch = fonts[fontId].ch[*c];

        float xpos = (pos.first + startingPos.first) + static_cast<float>(ch.bearing.x);
        float ypos = (pos.second + startingPos.second) + (static_cast<float>(fonts[fontId].height) - static_cast<float>(ch.size.y)) + static_cast<float>(ch.size.y - ch.bearing.y);
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

        if(shadowMode != Shadow::None) {
          std::pair<int, int> scaleSize = {0, fonts[fontId].height};
          float scale = getScale(scaleSize, fontId, texSize);
          float aspectRatio = texSize.second / texSize.first;
          std::pair<float, float> outlineOffset = {1.0 * scale * aspectRatio, 2.0 * scale};

          float dir[] = {
             0.0f,  1.0f,
            -1.0f,  1.0f,
            -1.0f,  0.0f,
            -1.0f, -1.0f,
             0.0f, -1.0f,
             1.0f, -1.0f,
             1.0f,  0.0f,
             1.0f,  1.0f
          };

          std::pair<int, int> dirs = {0, 0};
          switch(shadowMode) {
            case Shadow::Single:
              dirs = {3, 4};
              break;
            case Shadow::Outline:
              dirs = {0, 8};
              break;
            default:
              dirs = {0, 0};
              break;
          }

          for(int i = dirs.first; i < dirs.second; ++i) {
            float outlineV[4 * 3];
            for(int j = 0; j < 4 * 3; ++j) {
              switch(j % 3) {
                case 0:
                  outlineV[j] = vVertices[j] + dir[2 * i] * outlineOffset.first;
                  break;
                case 1:
                  outlineV[j] = vVertices[j] + dir[2 * i + 1] * outlineOffset.second * -1.0f;
                  break;
                case 2:
                  outlineV[j] = vVertices[j];
                  break;
              }
            }

            glUniform3f(colLoc, 0.0f, 0.0f, 0.0f);
            glUniform1f(opaLoc, 1.0f);
            glEnableVertexAttribArray(posLoc);
            glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, outlineV);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
          }
        }

        glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);
        glUniform1f(opaLoc, 1.0f);
        glEnableVertexAttribArray(posLoc);
        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

      }
      advance(pos, *c, fontId, scale, true);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);


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
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, Settings::viewport.first, Settings::viewport.second); // restore previous viewport

  TextTexture tt {.textureId = texture,
                  .width = static_cast<GLuint>(texSize.first),
                  .height = static_cast<GLuint>(texSize.second),
                  .fontId = static_cast<GLuint>(fontId),
                  .lastUsed = std::chrono::high_resolution_clock::now()};
  if(cache)
    generatedTextures.insert(std::make_pair(tk, tt));
  return tt;
}

void Text::renderTextTexture(TextTexture textTexture, std::pair<int, int> position, std::pair<int, int> size, std::vector<float> color) {
  if(textTexture.textureId == GL_INVALID_VALUE) {
    _INFO("INVALID TEXT TEXTURE");
    return;
  }

  std::pair<float, float> startingPos = {static_cast<float>(position.first) / static_cast<float>(Settings::viewport.first) * 2.0f - 1.0f,
                                         static_cast<float>(position.second) / static_cast<float>(Settings::viewport.second) * 2.0f - 1.0f};
  float scale = getScale(size, textTexture.fontId, Settings::viewport);
  float lineHeight = -fonts[textTexture.fontId].height * scale;

  float left = startingPos.first;
  float top = startingPos.second - lineHeight;
  float down = top - textTexture.height * scale * 1.0f;
  float right = left + textTexture.width * scale * 1.0f;

  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
  float texCoord[] = { 0.0f, 0.0f,    0.0f, 1.0f,
                       1.0f, 1.0f,    1.0f, 0.0f };

  glUseProgram(programObject2);
  glBindTexture(GL_TEXTURE_2D, textTexture.textureId);
  glUniform1i(samplerLoc2, 0);
  if(color.size() < 3)
    glUniform3f(colLoc2, 1.0f, 1.0f, 1.0f);
  else
    glUniform3f(colLoc2, color[0], color[1], color[2]);

  if(color.size() < 4)
    glUniform1f(opaLoc2, 1.0f);
  else
    glUniform1f(opaLoc2, color[3]);
  glEnableVertexAttribArray(posLoc2);
  glVertexAttribPointer(posLoc2, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
  glEnableVertexAttribArray(texLoc2);
  glVertexAttribPointer(texLoc2, 2, GL_FLOAT, GL_FALSE, 0, texCoord);
  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
}

void Text::renderDirect(std::string text, std::pair<int, int> position, std::pair<int, int> size, int fontId, std::vector<float> color, bool cache) {
  if(validFontId(fontId))
    return;
  bool alignedToOrigin = false; // can be used as a function parameter

  if(color.size() >= 4 && color[3] == 0.0f) // if the text is fully transparent, we don't have to render it
    return;

  std::pair<float, float> startingPos = {static_cast<float>(position.first) / static_cast<float>(Settings::viewport.first) * 2.0f - 1.0f,
                                         static_cast<float>(position.second) / static_cast<float>(Settings::viewport.second) * 2.0f - 1.0f};

  float scale = getScale(size, fontId, Settings::viewport);
  startingPos.first += fonts[fontId].max_bearingx * scale;
  startingPos.second += (alignedToOrigin ? 0 : fonts[fontId].max_descend) * scale;

  float textMaxWidth = size.first <= 0 ? 0 : 2.0 * static_cast<float>(size.first) / static_cast<float>(Settings::viewport.first) - fonts[fontId].max_bearingx * scale;
  if(size.first > 0)
    breakLines(text, fontId, textMaxWidth, scale);

  glUseProgram(programObject);

  std::pair<float, float> pos = {0.0f, 0.0f};
  std::string::const_iterator c;
  for(c = text.begin(); c != text.end(); ++c) {
    if(*c < charRange.first || *c >= charRange.second)
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

      if(shadowMode != Shadow::None) {
        float aspectRatio = static_cast<float>(Settings::viewport.second) / static_cast<float>(Settings::viewport.first);
        std::pair<float, float> outlineOffset = {2.0 * scale * aspectRatio, 2.0 * scale};


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

        float dir[] = {
           0.0f,  1.0f,
          -1.0f,  1.0f,
          -1.0f,  0.0f,
          -1.0f, -1.0f,
           0.0f, -1.0f,
           1.0f, -1.0f,
           1.0f,  0.0f,
           1.0f,  1.0f
        };
        std::pair<int, int> dirs = {0, 0};
        switch(shadowMode) {
          case Shadow::Single:
            dirs = {3, 4};
            break;
          case Shadow::Outline:
            dirs = {0, 8};
            break;
          default:
            dirs = {0, 0};
            break;
        }
        for(int i = dirs.first; i < dirs.second; ++i) {
          float outlineV[6 * 3];
          for(int j = 0; j < 6 * 3; ++j) {
            switch(j % 3) {
              case 0:
                outlineV[j] = vertices[j] + dir[2 * i] * outlineOffset.first;
                break;
              case 1:
                outlineV[j] = vertices[j] + dir[2 * i + 1] * outlineOffset.second;
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
    }
    advance(pos, *c, fontId, scale);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
}

int Text::getFontDefaultSize(int fontId) {
  if(validFontId(fontId))
    return fonts[fontId].size;
  return 1;
}

int Text::getFontDefaultHeight(int fontId) {
  if(validFontId(fontId))
    return fonts[fontId].height;
  return 1;
}

void Text::deleteUnusedTextures() {
  std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
  for(auto it = generatedTextures.cbegin(); it != generatedTextures.cend(); ) {
    if(std::chrono::duration_cast<std::chrono::duration<double>>(now - it->second.lastUsed) >= textureGCTimeout) {
      glDeleteTextures(1, &it->second.textureId);
      generatedTextures.erase(it++);
    }
    else {
      ++it;
    }
  }
}
