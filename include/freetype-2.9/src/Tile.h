#ifndef _TILE_H_
#define _TILE_H_

#include <string>
#include <chrono>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "../include/freetype-2.9/include/ft2build.h"

// ---------------
// Animation class
// ---------------
class TileAnimation {
  public:
    typedef enum {
      Linear,
      Quad,
      Cubic,
      Quint
    } Easing;

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> animationStart;
    std::chrono::milliseconds animationDuration;
    std::pair<int, int> sourcePosition;
    std::pair<int, int> targetPosition;
    Easing positionEasingType;
    float sourceZoom;
    float targetZoom;
    Easing zoomEasingType;
    std::pair<int, int> sourceSize;
    std::pair<int, int> targetSize;
    Easing sizeEasingType;
    float sourceOpacity;
    float targetOpacity;
    Easing opacityEasingType;
    bool active;

    static constexpr double pi() { return std::atan(1) * 4; }

    float zoomEasing(float fraction) {
      return doEasing(fraction, zoomEasingType);
    }

    float positionEasing(float fraction) {
      return doEasing(fraction, positionEasingType);
    }

    float sizeEasing(float fraction) {
      return doEasing(fraction, sizeEasingType);
    }

    float opacityEasing(float fraction) {
      return doEasing(fraction, opacityEasingType);
    }

    float doEasing(float fraction, Easing easing) {
      switch(easing) {
        case Easing::Quint:
          return quintEasing(fraction);
        case Easing::Cubic:
          return cubicEasing(fraction);
        case Easing::Quad:
          return quadEasing(fraction);
        case Easing::Linear:
        default:
          return linearEasing(fraction);
      }

    }

    float linearEasing(float t) {
      return t;
    }

    float quadEasing(float t) {
      return t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t;
    }

    float cubicEasing(float t) {
      return t < 0.5 ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
    }

    float quintEasing(float t) {
      return t < 0.5 ? 16 * t * t * t * t * t : 1 + 16 * (t - 1) * (t - 1) * (t - 1) * (t - 1) * (t - 1);
    }

  public:
    TileAnimation() : active(false) {}
    TileAnimation(std::chrono::time_point<std::chrono::high_resolution_clock> animationStart,
              std::chrono::milliseconds animationDuration,
              std::pair<int, int> sourcePosition,
              std::pair<int, int> targetPosition,
              Easing positionEasingType,
              float sourceZoom,
              float targetZoom,
              Easing zoomEasingType,
              std::pair<int, int> sourceSize,
              std::pair<int, int> targetSize,
              Easing sizeEasingType,
              float sourceOpacity,
              float targetOpacity,
              Easing opacityEasingType)
      :
              animationStart(animationStart),
              animationDuration(animationDuration),
              sourcePosition(sourcePosition),
              targetPosition(targetPosition),
              positionEasingType(positionEasingType),
              sourceZoom(sourceZoom),
              targetZoom(targetZoom),
              zoomEasingType(zoomEasingType),
              sourceSize(sourceSize),
              targetSize(targetSize),
              sizeEasingType(sizeEasingType),
              sourceOpacity(sourceOpacity),
              targetOpacity(targetOpacity),
              opacityEasingType(opacityEasingType),
              active(true) {}

    void update(std::pair<int, int> &position, float &zoom, std::pair<int, int> &size, float &opacity) {
      if(!active)
        return;
      std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(now - animationStart);
      std::chrono::duration<double> target = std::chrono::duration_cast<std::chrono::duration<double>>(animationDuration);
      float fraction = time_span.count() / target.count();
      if(fraction >= 1.0) {
        active = false;
        return;
      }
      position.first = sourcePosition.first + (targetPosition.first - sourcePosition.first) * positionEasing(fraction);
      position.second = sourcePosition.second + (targetPosition.second - sourcePosition.second) * positionEasing(fraction);
      zoom = sourceZoom + (targetZoom - sourceZoom) * zoomEasing(fraction);
      size.first = sourceSize.first + (targetSize.first - sourceSize.first) * sizeEasing(fraction);
      size.second = sourceSize.second + (targetSize.second - sourceSize.second) * sizeEasing(fraction);
      opacity = sourceOpacity + (targetOpacity - sourceOpacity) * opacityEasing(fraction);
    }

    bool isActive() { return active; }

    ~TileAnimation() = default;
};

// ---------
// Tile class
// ---------

class Tile {
  private:
    int id;
    GLuint programObject = GL_INVALID_VALUE;
    GLuint textureFormat = GL_INVALID_VALUE;
    int x;
    int y;
    int width;
    int height;
    int viewportWidth;
    int viewportHeight;
    float zoom;
    float opacity;
    std::string name;
    std::string description;

    TileAnimation animation;
    GLuint textureId = GL_INVALID_VALUE;
/*    GLuint samplerLoc;
    GLint posLoc;
    GLint texLoc;
    GLuint opacityLoc;*/

    void initialize();

  public:
    Tile(int tileId, GLuint programObject, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, float zoom, float opacity, std::string name, std::string description, char *texturePixels, std::pair<int, int> textureSize, GLuint textureFormat);
    Tile(int tileId, GLuint programObject);
    ~Tile();
    Tile(Tile &) = delete; // no copy constructor
    Tile(const Tile &) = delete; // no copy constructor
    //Tile(Tile &&) = default; // default move constructor
    Tile& operator=(Tile&&) = delete;
    Tile(Tile &&other);

    void render();
    void setTexture(char *pixels, int width, int heigth, GLuint format);
    void moveTo(std::pair<int, int> position, float zoom, std::pair<int, int> size, float opacity, std::chrono::milliseconds duration);

    void setId(int id) { this->id = id; }
    int  getId() { return id; }
    void setSize(int width, int height) { this->width = width; this->height = height; }
    void setSize(const std::pair<int, int> &size) { width = size.first; height = size.second; }
    std::pair<int, int> getSize() { return std::make_pair(width, height); }
    int getWidth() { return width; }
    int getHeight() { return height; }
    void setPosition(int x, int y) { this->x = x; this->y = y; }
    void setPosition(const std::pair<int, int> &position) { x = position.first; y = position.second; }
    int getX() { return x; }
    int getY() { return y; }
    std::pair<int, int> getPosition() { return std::make_pair(x, y); }
    void setName(const std::string &name) { this->name = name; }
    void setDescription(const std::string &description) { this->description = description; }
    void setViewportSize(const std::pair<int, int> &viewportSize) { viewportWidth = viewportSize.first; viewportHeight = viewportSize.second; }
    void setViewportSize(int viewportWidth, int viewportHeight) { this->viewportWidth = viewportWidth; this->viewportHeight = viewportHeight; }
    int getTextureId() { return textureId; }
    void setZoom(float zoom) { this->zoom = zoom; }
    void setOpacity(float opacity) { this->opacity = opacity; }
    float getZoom() { return zoom; }
    float getOpacity() { return opacity; }
};

Tile::Tile(int tileId, GLuint programObject, std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, float zoom, float opacity, std::string name, std::string description, char *texturePixels, std::pair<int, int> textureSize, GLuint textureFormat)
          : id(tileId),
            programObject(programObject),
            x(position.first),
            y(position.second),
            width(size.first),
            height(size.second),
            viewportWidth(viewport.first),
            viewportHeight(viewport.second),
            zoom(zoom),
            opacity(opacity),
            name(name),
            description(description) {
  initialize();
  setTexture(texturePixels, textureSize.first, textureSize.second, textureFormat);
}

Tile::Tile(int tileId, GLuint programObject)
          : id(tileId),
          programObject(programObject),
          textureId(GL_INVALID_VALUE) {
  initialize();
}

void Tile::initialize() {
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &textureId);
/*  glUseProgram(programObject);
  samplerLoc = glGetUniformLocation(programObject, "s_texture");
  posLoc = glGetAttribLocation(programObject, "a_position");
  texLoc = glGetAttribLocation(programObject, "a_texCoord");
  opacityLoc = glGetUniformLocation(programObject, "u_opacity"); */
}

Tile::Tile(Tile &&other) {
  if(this != &other) {
    id = other.id;
    width = other.width;
    height = other.height;
    x = other.x;
    y = other.y;
    viewportWidth = other.viewportWidth;
    viewportHeight = other.viewportHeight;
    textureId = other.textureId;
    textureFormat = other.textureFormat;
    programObject = other.programObject;
    name = other.name;
    description = other.description;
    zoom = other.zoom;
    opacity = other.opacity;
    animation = other.animation;

    other.textureId = GL_INVALID_VALUE; // prevent destructor of the object we moved from from deleting the texture
  }
}

Tile::~Tile() {
  if(textureId != GL_INVALID_VALUE)
    glDeleteTextures(1, &textureId);
}

void Tile::moveTo(std::pair<int, int> position, float zoom, std::pair<int, int> size, float opacity, std::chrono::milliseconds duration) {
  animation = TileAnimation(std::chrono::high_resolution_clock::now(),
                            std::chrono::milliseconds(duration),
                            {x, y},
                            position,
                            TileAnimation::Easing::Cubic,
                            this->zoom,
                            zoom,
                            TileAnimation::Easing::Cubic,
                            {width, height},
                            size,
                            TileAnimation::Easing::Cubic,
                            this->opacity,
                            opacity,
                            TileAnimation::Easing::Cubic);
  x = position.first;
  y = position.second;
}

void Tile::render() { // TODO(g.skowinski): Implement opacity handling.
  std::pair<int, int> position {x, y};
  std::pair<int, int> size {width, height};
  animation.update(position, zoom, size, opacity);

  float vW = viewportWidth;
  float vH = viewportHeight;
  float w = width;
  float h = height;
  float xPos = position.first;
  float yPos = position.second;
  float left = xPos / vW * 2 - 1;
  float down = yPos / vH * 2 - 1;
  float right = (xPos + w) / vW * 2 - 1;
  float top = (yPos + h) / vH * 2 - 1;

  //GLint colLoc = glGetUniformLocation(programObject, "u_color");
  //glUniform4f(colLoc, 0.0f, 0.0f, 0.0f, 1.0f);

  left -= (w / vW) * (zoom - 1.0);
  right += (w / vW) * (zoom - 1.0);
  down -= (h / vH) * (zoom - 1.0);
  top += (h / vH) * (zoom - 1.0);
  

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
}

void Tile::setTexture(char *pixels, int width, int height, GLuint format) {
  textureFormat = format;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR/*GL_NEAREST*/);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR/*GL_NEAREST*/);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFlush();
}

#endif // _TILE_H_
