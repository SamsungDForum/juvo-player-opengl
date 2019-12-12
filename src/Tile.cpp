#include "Tile.h"
#include "ProgramBuilder.h"
#include "Settings.h"

int Tile::staticTileObjectCount = 0;
GLuint Tile::programObject    = GL_INVALID_VALUE;
GLuint Tile::tileSizeLoc      = GL_INVALID_VALUE;
GLuint Tile::tilePositionLoc  = GL_INVALID_VALUE;
GLuint Tile::samplerLoc       = GL_INVALID_VALUE;
GLuint Tile::posLoc           = GL_INVALID_VALUE;
GLuint Tile::texLoc           = GL_INVALID_VALUE;
GLuint Tile::opacityLoc       = GL_INVALID_VALUE;
GLuint Tile::viewportLoc      = GL_INVALID_VALUE;
GLuint Tile::scaleLoc         = GL_INVALID_VALUE;
GLuint Tile::storytileRectLoc = GL_INVALID_VALUE;

Tile::Tile(int tileId, std::pair<int, int> position, std::pair<int, int> size, float zoom, float opacity, std::string name, std::string description, char *texturePixels, std::pair<int, int> textureSize, GLuint textureFormat)
          : id(tileId),
            position(position),
            size(size),
            zoom(zoom),
            opacity(opacity),
            name(name),
            description(description),
            runningPreview(false),
            previewReady(false),
            previewTextureId(GL_INVALID_VALUE),
            textureId(GL_INVALID_VALUE) {
  initGL();
  initTextures();
  setTexture(texturePixels, textureSize, textureFormat);
  ++staticTileObjectCount;
}

Tile::Tile(int tileId, std::pair<int, int> position, std::pair<int, int> size, float zoom, float opacity, std::string name, std::string description)
          : id(tileId),
            position(position),
            size(size),
            zoom(zoom),
            opacity(opacity),
            name(name),
            description(description),
            runningPreview(false),
            previewReady(false),
            previewTextureId(GL_INVALID_VALUE),
            textureId(GL_INVALID_VALUE) {
  initGL();
  initTextures();
  ++staticTileObjectCount;
}

Tile::Tile(int tileId)
          : id(tileId),
          textureId(GL_INVALID_VALUE) {
  initGL();
  initTextures();
  ++staticTileObjectCount;
}

void Tile::initTextures() {
  if(textureId == GL_INVALID_VALUE)
    glGenTextures(1, &textureId);
  if(previewTextureId == GL_INVALID_VALUE)
    glGenTextures(1, &previewTextureId);
}

void Tile::initGL() {
  if(programObject != GL_INVALID_VALUE)
    return;

  const GLchar* vShaderTexStr =
#include "shaders/tile.vert"
;
 
  const GLchar* fShaderTexStr =
#include "shaders/tile.frag"
;

  programObject = ProgramBuilder::buildProgram(vShaderTexStr, fShaderTexStr);

  tileSizeLoc = glGetUniformLocation(programObject, "u_tileSize");
  tilePositionLoc = glGetUniformLocation(programObject, "u_tilePosition");
  samplerLoc = glGetUniformLocation(programObject, "s_texture");
  posLoc = glGetAttribLocation(programObject, "a_position");
  texLoc = glGetAttribLocation(programObject, "a_texCoord");
  opacityLoc = glGetUniformLocation(programObject, "u_opacity");
  viewportLoc = glGetUniformLocation(programObject, "u_viewport");
  scaleLoc = glGetUniformLocation(programObject, "u_scale");
  storytileRectLoc = glGetUniformLocation(programObject, "u_storytileRect");
}

Tile::Tile(Tile &&other) { // update this move constructor when adding new members!
  if(this != &other) {
    id = other.id;
    position = other.position;
    size = other.size;
    zoom = other.zoom;
    opacity = other.opacity;
    name = other.name;
    description = other.description;

    animation = other.animation;

    runningPreview = other.runningPreview;
    storyboardPreviewStartTimePoint = other.storyboardPreviewStartTimePoint;
    storyboardBitmap = other.storyboardBitmap;
    storytileRect = other.storytileRect;
    previewReady = other.previewReady;
    previewTextureId = other.previewTextureId;
    storyboardBitmap = other.storyboardBitmap;
    bitmapHash = other.bitmapHash;
    getStoryboardDataCallback = other.getStoryboardDataCallback;

    textureId = other.textureId;
    textureFormat = other.textureFormat;

    ++staticTileObjectCount; // prevent destructor of the object we moved from from deleting OpenGL objects with ids keept in static fields
    programObject = other.programObject;
    tileSizeLoc = other.tileSizeLoc;
    tilePositionLoc = other.tilePositionLoc;
    samplerLoc = other.samplerLoc;
    posLoc = other.posLoc;
    texLoc = other.texLoc;
    opacityLoc = other.opacityLoc;
    viewportLoc = other.viewportLoc;
    scaleLoc = other.scaleLoc;
    storytileRectLoc = other.storytileRectLoc;

    other.textureId = GL_INVALID_VALUE; // prevent destructor of the object we moved from from deleting the texture
    other.previewTextureId = GL_INVALID_VALUE; // prevent destructor of the object we moved from from deleting the texture
  }
}

Tile::~Tile() {
  if(textureId != GL_INVALID_VALUE) {
    glDeleteTextures(1, &textureId);
    textureId = GL_INVALID_VALUE;
  }
  if(previewTextureId != GL_INVALID_VALUE) {
    glDeleteTextures(1, &previewTextureId);
    previewTextureId = GL_INVALID_VALUE;
  }
  if(staticTileObjectCount == 1 && programObject != GL_INVALID_VALUE) {
    glDeleteProgram(programObject);
    programObject = GL_INVALID_VALUE;

    tileSizeLoc      = GL_INVALID_VALUE;
    tilePositionLoc  = GL_INVALID_VALUE;
    samplerLoc       = GL_INVALID_VALUE;
    posLoc           = GL_INVALID_VALUE;
    texLoc           = GL_INVALID_VALUE;
    opacityLoc       = GL_INVALID_VALUE;
    viewportLoc      = GL_INVALID_VALUE;
    scaleLoc         = GL_INVALID_VALUE;
    storytileRectLoc = GL_INVALID_VALUE;
  }

  --staticTileObjectCount;
}

void Tile::setTexture(char *pixels, std::pair<int, int> size, GLuint format) {
  if(textureId == GL_INVALID_VALUE)
    initTextures();

  textureFormat = format;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, format, size.first, size.second, 0, format, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, GL_INVALID_VALUE);
}

void Tile::render() {
  if(textureId == GL_INVALID_VALUE)
    return;

  animation.update(position, zoom, size, opacity);

  float leftPx = position.first - (size.first / 2.0) * (zoom - 1.0);
  float rightPx = (position.first + size.first) + (size.first / 2.0) * (zoom - 1.0);
  float downPx = position.second - (size.second / 2.0) * (zoom - 1.0);
  float topPx = (position.second + size.second) + (size.second / 2.0) * (zoom - 1.0);
  float left = (leftPx / Settings::instance().viewport.first) * 2.0 - 1.0;
  float right = (rightPx / Settings::instance().viewport.first) * 2.0 - 1.0;
  float down = (downPx / Settings::instance().viewport.second) * 2.0 - 1.0;
  float top = (topPx / Settings::instance().viewport.second) * 2.0 - 1.0;

  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
  float texCoord[] = { 0.0f, 0.0f,    0.0f, 1.0f,
                       1.0f, 1.0f,    1.0f, 0.0f };

  glUseProgram(programObject);

  glUniform2f(tileSizeLoc, static_cast<float>(rightPx - leftPx), static_cast<float>(topPx - downPx));
  glUniform2f(tilePositionLoc, static_cast<float>(leftPx), static_cast<float>(downPx));
  glUniform1f(opacityLoc, static_cast<GLfloat>(opacity));
  glUniform2f(viewportLoc, static_cast<GLfloat>(Settings::instance().viewport.first), static_cast<GLfloat>(Settings::instance().viewport.second));
  glUniform1f(scaleLoc, static_cast<GLfloat>(zoom));

  // GetTextureId() updates texture data and metadata, so it should be called before setting texture metadata for the pipeline (before setting storytileRectLoc vec4 values)
  glBindTexture(GL_TEXTURE_2D, getCurrentTextureId());
  glUniform1i(samplerLoc, 0);

  if(runningPreview && previewReady)
    glUniform4f(storytileRectLoc, storytileRect.left / storyboardBitmap.bitmapWidth, storytileRect.top / storyboardBitmap.bitmapHeight, storytileRect.width() / storyboardBitmap.bitmapWidth, storytileRect.height() / storyboardBitmap.bitmapHeight);
  else
    glUniform4f(storytileRectLoc, 0.0f, 0.0f, 1.0f, 1.0f);

  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
  glEnableVertexAttribArray(texLoc);
  glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoord);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(posLoc);
  glDisableVertexAttribArray(texLoc);
  glBindTexture(GL_TEXTURE_2D, GL_INVALID_VALUE);
  glUseProgram(GL_INVALID_VALUE);
}

void Tile::moveTo(std::pair<int, int> position, float zoom, std::pair<int, int> size, float opacity, std::chrono::milliseconds duration, std::chrono::milliseconds delay, int bounce) {
  TileAnimation::Easing positionEasing = animation.isActive() ? TileAnimation::Easing::CubicOut : TileAnimation::Easing::CubicInOut;
  if(bounce && !animation.isActive())
    positionEasing = bounce < 0 ? TileAnimation::Easing::BounceLeft : TileAnimation::Easing::BounceRight;

  TileAnimation::Easing zoomEasing = animation.isActive() ? TileAnimation::Easing::CubicOut : TileAnimation::Easing::CubicInOut;
  TileAnimation::Easing sizeEasing = animation.isActive() ? TileAnimation::Easing::CubicOut : TileAnimation::Easing::CubicInOut;
  TileAnimation::Easing opacityEasing = animation.isActive() ? TileAnimation::Easing::CubicOut : TileAnimation::Easing::CubicInOut;
  animation = TileAnimation(std::chrono::high_resolution_clock::now(),
                            std::chrono::milliseconds(duration),
                            std::chrono::milliseconds(delay),
                            this->position,
                            position,
                            positionEasing,
                            this->zoom,
                            zoom,
                            zoomEasing,
                            this->size,
                            size,
                            sizeEasing,
                            this->opacity,
                            opacity,
                            opacityEasing);
}

void Tile::runPreview(bool run) {
  if(!runningPreview && run) {
    storyboardPreviewStartTimePoint = std::chrono::high_resolution_clock::now();
    previewReady = false;
  }
  runningPreview = run;
}

GLuint Tile::getCurrentTextureId() {

  if(!runningPreview) // avoid callback call most of the time
    return textureId;

  std::chrono::milliseconds delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - storyboardPreviewStartTimePoint);
  StoryboardExternData storyboardData = getStoryboardData(delta - std::chrono::milliseconds(storyboardData.duration), id);

  if(!storyboardData.isStoryboardReaderReady || (runningPreview && delta > std::chrono::milliseconds(storyboardData.duration))) { // no preview or preview just finished
    runningPreview = false;
    return textureId;
  }

  if(delta < Settings::instance().tilePreviewDelay) // still during delay
    return textureId;

  if(!storyboardData.isFrameReady) { // frame not ready
    if(previewReady) // if it's not a first frame, let's use last one
      return previewTextureId;
    return textureId;
  }

  if(storyboardData.frame.bitmapHash != bitmapHash) { // new storyboard
    storyboardBitmap = storyboardData.frame;
    setPreviewTexture(storyboardData.frame);
  }
  storytileRect = Rect { // update frame rectangle metadata
    .left = storyboardData.frame.rectLeft,
    .right = storyboardData.frame.rectRight,
    .top = storyboardData.frame.rectTop,
    .bottom = storyboardData.frame.rectBottom
  };

  previewReady = true; // we're here, so frame must be ready and preview must be running
  return previewTextureId;
}

void Tile::setPreviewTexture(SubBitmapExtern frame) {
  if(previewTextureId == GL_INVALID_VALUE)
    initTextures();

  GLuint textureFormat = ConvertFormat(frame.bitmapInfoColorType);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, previewTextureId);
  glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, frame.bitmapWidth, frame.bitmapHeight, 0, textureFormat, GL_UNSIGNED_BYTE, frame.bitmapBytes);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, GL_INVALID_VALUE);
}

StoryboardExternData Tile::getStoryboardData(std::chrono::duration<double> position, int tileId) {
  if(position < std::chrono::milliseconds(0))
    position = std::chrono::milliseconds(0);
  return getStoryboardDataCallback(static_cast<long long>(std::chrono::duration_cast<std::chrono::milliseconds>(position).count()), id);
}


void Tile::setStoryboardCallback(StoryboardExternData (*getStoryboardDataCallback)(long long position, int tileId)) {
  this->getStoryboardDataCallback = getStoryboardDataCallback;
}
