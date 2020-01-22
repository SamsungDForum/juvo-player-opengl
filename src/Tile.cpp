#include "Tile.h"
#include "ProgramBuilder.h"
#include "Settings.h"
#include "Utility.h"
#include "TextRenderer.h"
#include "LogConsole.h"

#include<sstream>

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

Tile::Tile(int tileId, Position<int> position, Size<int> size, float zoom, float opacity, std::string name, std::string description, char *texturePixels, Size<int> textureSize, GLuint textureFormat)
          : id(tileId),
            position(position),
            size(size),
            zoom(zoom),
            opacity(opacity),
            name(name),
            description(description),
            animation(TileAnimation(position, zoom, size, opacity)),
            active(false),
            runningPreview(false),
            previewReady(false),
            previewTextureId(0),
            textureId(0) {
  initGL();
  initTextures();
  setTexture(texturePixels, textureSize, textureFormat);
  ++staticTileObjectCount;
}

Tile::Tile(int tileId, Position<int> position, Size<int> size, float zoom, float opacity, std::string name, std::string description)
          : id(tileId),
            position(position),
            size(size),
            zoom(zoom),
            opacity(opacity),
            name(name),
            description(description),
            animation(TileAnimation(position, zoom, size, opacity)),
            active(false),
            runningPreview(false),
            previewReady(false),
            previewTextureId(0),
            textureId(0) {
  initGL();
  initTextures();
  ++staticTileObjectCount;
}

Tile::Tile(int tileId)
          : id(tileId),
            active(false),
            runningPreview(false),
            previewReady(false),
            previewTextureId(0),
            textureId(0) {
  initGL();
  initTextures();
  ++staticTileObjectCount;
}

void Tile::initTextures() {
  assertCurrentEGLContext();

  if(textureId == 0)
    glGenTextures(1, &textureId);
  if(previewTextureId == 0)
    glGenTextures(1, &previewTextureId);

  if(textureId == GL_INVALID_VALUE || previewTextureId == GL_INVALID_VALUE) {
    LogConsole::instance().log("-----===== INVALID VALUES FOR TILE TEXTURES! =====-----", LogConsole::LogLevel::Error);
    throw("-----===== INVALID VALUES FOR TILE TEXTURES! =====-----");
  }
}

void Tile::initGL() {
  assertCurrentEGLContext();

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

    other.textureId = 0; // prevent destructor of the object we moved from from deleting the texture
    other.previewTextureId = 0; // prevent destructor of the object we moved from from deleting the texture
  }
}

Tile::~Tile() {
  assertCurrentEGLContext();

  if(textureId != 0) {
    glDeleteTextures(1, &textureId);
    textureId = 0;
  }
  if(previewTextureId != 0) {
    glDeleteTextures(1, &previewTextureId);
    previewTextureId = 0;
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

void Tile::setTexture(char *pixels, Size<int> size, GLuint format) {
  assertCurrentEGLContext();

  if(textureId == 0)
    initTextures();

  textureFormat = format;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, format, size.width, size.height, 0, format, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void Tile::render() {
  assertCurrentEGLContext();

  if(textureId == 0)
    return;

  animation.update(position, zoom, size, opacity);

  float leftPx = position.x - (size.width / 2.0) * (zoom - 1.0);
  float rightPx = (position.x + size.width) + (size.width / 2.0) * (zoom - 1.0);
  float downPx = position.y - (size.height / 2.0) * (zoom - 1.0);
  float topPx = (position.y + size.height) + (size.height / 2.0) * (zoom - 1.0);
  float left = (leftPx / Settings::instance().viewport.width) * 2.0 - 1.0;
  float right = (rightPx / Settings::instance().viewport.width) * 2.0 - 1.0;
  float down = (downPx / Settings::instance().viewport.height) * 2.0 - 1.0;
  float top = (topPx / Settings::instance().viewport.height) * 2.0 - 1.0;

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
  glUniform2f(viewportLoc, static_cast<GLfloat>(Settings::instance().viewport.width), static_cast<GLfloat>(Settings::instance().viewport.height));
  glUniform1f(scaleLoc, static_cast<GLfloat>(zoom));

  // GetTextureId() updates texture data and metadata, so it should be called before setting texture metadata for the pipeline (before setting storytileRectLoc vec4 values)
  glActiveTexture(GL_TEXTURE0);
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
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);

  if(active)
    renderName();
}

void Tile::renderName() {
  float opacity = (zoom - 1.0f) / (Settings::instance().zoom - 1.0f) * this->opacity;
  float left = position.x + size.width * 0.5f - TextRenderer::instance().getTextSize(name, {0, static_cast<GLuint>(Settings::instance().tileNameFontHeight)}, 0).width * 0.5f;
  TextRenderer::instance().render(
      name,
      { static_cast<int>(left), static_cast<int>(position.y - Settings::instance().tileNameFontHeight - size.height * (Settings::instance().zoom - 1.0f) * 0.5f) },
      { 0, Settings::instance().tileNameFontHeight },
      0,
      { 1.0f, 1.0f, 1.0f, opacity }
  );
}

void Tile::moveTo(Position<int> position, float zoom, Size<int> size, float opacity, std::chrono::milliseconds moveDuration, std::chrono::milliseconds animationDuration, std::chrono::milliseconds delay) {
  Animation::Easing easing = animation.isActive() ? Animation::Easing::CubicOut : Animation::Easing::CubicInOut;

  animation = TileAnimation(
      TileAnimation::AnimationParameters<Position<int>> (
        moveDuration,
        delay,
        this->position,
        position,
        easing
      ),
      TileAnimation::AnimationParameters<float> {
        animationDuration, // TODO: setting this to value lower than move Duration causes an animation artifacts in last stage
        delay,
        this->zoom,
        zoom,
        easing
      },
      TileAnimation::AnimationParameters<Size<int>> {
        moveDuration,
        delay,
        this->size,
        size,
        easing
     },
      TileAnimation::AnimationParameters<float> {
        moveDuration,
        delay,
        this->opacity,
        opacity,
        easing
    }
  );
}

void Tile::runPreview(bool run) {
  if(!runningPreview && run) {
    storyboardPreviewStartTimePoint = std::chrono::steady_clock::now();
    previewReady = false;
  }
  runningPreview = run;
}

GLuint Tile::getCurrentTextureId() {

  if(!runningPreview) // avoid callback call most of the time
    return textureId;

  std::chrono::milliseconds delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - storyboardPreviewStartTimePoint);
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
  assertCurrentEGLContext();

  if(previewTextureId == 0)
    initTextures();

  GLuint textureFormat = ConvertFormat(frame.bitmapInfoColorType);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, previewTextureId);
  glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, frame.bitmapWidth, frame.bitmapHeight, 0, textureFormat, GL_UNSIGNED_BYTE, frame.bitmapBytes);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);
}

StoryboardExternData Tile::getStoryboardData(std::chrono::milliseconds position, int tileId) {
  if(position < std::chrono::duration_values<std::chrono::milliseconds>::zero())
    position = std::chrono::duration_values<std::chrono::milliseconds>::zero();
  return getStoryboardDataCallback(static_cast<long long>(position.count()), id);
}


void Tile::setStoryboardCallback(StoryboardExternData (*getStoryboardDataCallback)(long long position, int tileId)) {
  this->getStoryboardDataCallback = getStoryboardDataCallback;
}
