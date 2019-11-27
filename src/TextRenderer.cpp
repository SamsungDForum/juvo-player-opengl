#include "TextRenderer.h"
#include "TextTextureGenerator.h"
#include "ProgramBuilder.h"
#include "Settings.h"
#include "LogConsole.h"

TextRenderer::TextRenderer() {
  prepareShaders();
}

TextRenderer::~TextRenderer() {
  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
}

void TextRenderer::prepareShaders() {
  const GLchar* vShaderTexStr =
#include "shaders/textRenderer.vert"
;

  const GLchar* fShaderTexStr =
#include "shaders/textRenderer.frag"
;

  programObject = ProgramBuilder::buildProgram(vShaderTexStr, fShaderTexStr);

  posLoc = glGetAttribLocation(programObject, "a_position");
  texLoc = glGetAttribLocation(programObject, "a_texCoord");
  colLoc = glGetUniformLocation(programObject, "u_color");
  samplerLoc = glGetUniformLocation(programObject, "s_texture");
  shaColLoc = glGetUniformLocation(programObject, "u_shadowColor");
  shaOffLoc = glGetUniformLocation(programObject, "u_shadowOffset");
  opaLoc = glGetUniformLocation(programObject, "u_opacity");
}

int TextRenderer::addFont(char *data, int size) {
  return TextTextureGenerator::instance().addFont(data, size);
}

std::pair<GLuint, GLuint> TextRenderer::getTextSize(const std::string text, std::pair<GLuint, GLuint> size, int fontId) {
  try {
    return TextTextureGenerator::instance().getTexture(TextTextureGenerator::TextureKey {
      .text = text,
      .size = size,
      .fontId = fontId,
    }).getSize();
  } catch(const std::exception &e) {
    LogConsole::instance().log(std::string("Cannot get text size: ") + std::string(e.what()), LogConsole::LogLevel::Error);
  } catch(...) {
    LogConsole::instance().log("Cannot get text size: unknown exception", LogConsole::LogLevel::Error);
  }
  return {1, 1};
}

void TextRenderer::render(std::string text, std::pair<int, int> position, std::pair<int, int> size, int fontId, std::vector<float> color) {
  try {
    if(color.size() >= 4 && color[3] < 0.001f) // if the text is fully transparent, we don't have to render it
      return;

    TextTextureGenerator::TextureInfo textureInfo = TextTextureGenerator::instance().getTexture(TextTextureGenerator::TextureKey {
      .text = text,
      .size = size,
      .fontId = fontId,
    });

    if(textureInfo.getSize().first < 1 || textureInfo.getSize().second < 1) {
      LogConsole::instance().log("textureInfo.size invalid!", LogConsole::LogLevel::Error);
      return;
    }

    float left = static_cast<float>(position.first) / static_cast<float>(Settings::instance().viewport.first) * 2.0f - 1.0f;
    float right = left + static_cast<float>(textureInfo.getSize().first) / static_cast<float>(Settings::instance().viewport.first) * 2.0f;
    float top = (static_cast<float>(position.second) + static_cast<float>(textureInfo.getFont().height)) / static_cast<float>(Settings::instance().viewport.second) * 2.0f - 1.0f;
    float down = top - static_cast<float>(textureInfo.getSize().second) / static_cast<float>(Settings::instance().viewport.second) * 2.0f;

    GLfloat vertices[] = { left,   top,  0.0f,
                           left,   down, 0.0f,
                           right,  down, 0.0f,
                           right,  top,  0.0f
    };
    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
    float texCoord[] = { 0.0f, 0.0f,    0.0f, 1.0f,
                         1.0f, 1.0f,    1.0f, 0.0f };

    glUseProgram(programObject);

    glBindTexture(GL_TEXTURE_2D, textureInfo.getTextureId());
    glUniform1i(samplerLoc, 0);
    glUniform3f(colLoc, color[0], color[1], color[2]);
    glUniform3f(shaColLoc, 0.0f, 0.0f, 0.0f);
    glUniform1f(opaLoc, color[3]);
    glUniform2f(shaOffLoc, -1.0f / textureInfo.getSize().first, -1.0f / textureInfo.getSize().second);
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(texLoc);
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoord);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    glDisableVertexAttribArray(posLoc);
    glDisableVertexAttribArray(texLoc);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
  } catch(const std::exception &e) {
    LogConsole::instance().log(std::string("Text rendering failed: ") + std::string(e.what()), LogConsole::LogLevel::Error);
  } catch(...) {
    LogConsole::instance().log("Text rendering failed: unknown exception", LogConsole::LogLevel::Error);
  }
}

