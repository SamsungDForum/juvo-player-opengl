#ifndef _TEXT_H_
#define _TEXT_H_

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

#include <glm/vec2.hpp>
#include <map>
#include <string>

#ifndef _FT_FREETYPE_
#define _FT_FREETYPE_
#include <ft2build.h>
#include FT_FREETYPE_H
#endif // _FT_FREETYPE_

class Text {
private:
  GLuint programObject;
  FT_Library ftLibrary;
  FT_Face ftFace;
  struct Character {
    GLuint TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    GLuint Advance;
  };
  std::map<GLchar, Character> Characters;

private:
  void prepareShaders();

public:
  Text();
  ~Text();
  int AddFont(char *data, int size, int fontSize);
  void render(std::string text);
};

Text::Text() {
  FT_Error error = FT_Init_FreeType(&ftLibrary);
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
    "  gl_FragColor                \n"
    "     = vec4(1.0, 1.0, 1.0,    \n"
    "       texture2D(s_texture,   \n"
    "               v_texCoord).r  \n" // we're sampling luminance, so r=g=b,a=1.0
    "               * u_opacity)   \n"
    "       * vec4(u_color, 1.0);  \n"
    "}                             \n";

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vShaderTexStr, NULL);
  glCompileShader(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fShaderTexStr, NULL);
  glCompileShader(fragmentShader);

  programObject = glCreateProgram();
  glAttachShader(programObject, vertexShader);
  glAttachShader(programObject, fragmentShader);
  glLinkProgram(programObject);

  //glBindAttribLocation(programObject, 0, "a_position");
}

int Text::AddFont(char *data, int size, int fontSize) {
  FT_Error error = FT_New_Memory_Face(ftLibrary, reinterpret_cast<FT_Byte*>(data), size, 0, &ftFace); // FT_Byte* is an alias for unsigned char*
  if(error)
    return -1;
  FT_Set_Pixel_Sizes(ftFace, 0, fontSize);

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
      static_cast<GLuint>(ftFace->glyph->advance.x)
    };
    Characters.insert(std::pair<GLchar, Character>(c, character));
  }
  FT_Done_Face(ftFace);
  return 0;
}

void Text::render(std::string text) {
  GLfloat x = -0.9;
  GLfloat y = 0.8;
  GLfloat scale = 0.001;

  glUseProgram(programObject);
  //glActiveTexture(GL_TEXTURE0);

  std::string::const_iterator c;
  for(c = text.begin(); c != text.end(); ++c) {
    Character ch = Characters[*c];

    GLfloat xpos = x + ch.Bearing.x * scale;
    GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
    GLfloat w = ch.Size.x * scale;
    GLfloat h = ch.Size.y * scale;

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

    x += (ch.Advance >> 6) * scale; // Advance is in 1/64px
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  glUseProgram(0);
}

#endif