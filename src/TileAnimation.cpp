#include "TileAnimation.h"

#include <chrono>

TileAnimation::TileAnimation() : active(false) {}

TileAnimation::TileAnimation(AnimationParameters<Position<int>> position,
                             AnimationParameters<float> zoom,
                             AnimationParameters<Size<int>> size,
                             AnimationParameters<float> opacity)
  :
          start(std::chrono::steady_clock::now()),
          position(position),
          zoom(zoom),
          size(size),
          opacity(opacity),
          active(isDurationPositive(position.duration) ||
                 isDurationPositive(zoom.duration) ||
                 isDurationPositive(size.duration) ||
                 isDurationPositive(opacity.duration)) {
}

void TileAnimation::update(Position<int> &position, float &zoom, Size<int> &size, float &opacity) {
  UpdatedValues out = update();
  position = out.position;
  zoom = out.zoom;
  size = out.size;
  opacity = out.opacity;
}

TileAnimation::UpdatedValues TileAnimation::update() {
  if(!active)
    return { position.target, zoom.target, size.target, opacity.target };

  std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();

  if(position.fraction(now, start) > fractionThreshold &&
     zoom.fraction(now, start) > fractionThreshold &&
     size.fraction(now, start) > fractionThreshold &&
     opacity.fraction(now, start) > fractionThreshold)
    active = false;

  return {
    interpolate(now, position),
    interpolate(now, zoom),
    interpolate(now, size),
    interpolate(now, opacity)
  };
}

float TileAnimation::interpolate(const std::chrono::time_point<std::chrono::steady_clock> &now, const AnimationParameters<float> &value) {
  float fraction = value.fraction(now, start);
  if(fraction <= 0.0f)
    return value.source;
  if(fraction > fractionThreshold)
    return value.target;
  return value.source + (value.target - value.source) * Animation::ease(fraction, value.easing);
}

Position<int> TileAnimation::interpolate(const std::chrono::time_point<std::chrono::steady_clock> &now, const AnimationParameters<Position<int>> &value) {
  float fraction = value.fraction(now, start);
  if(fraction <= 0.0f)
    return value.source;
  if(fraction > fractionThreshold)
    return value.target;
  float interpolation = Animation::ease(fraction, value.easing);
  return {
    static_cast<int>(value.source.x + (value.target.x - value.source.x) * interpolation),
    static_cast<int>(value.source.y + (value.target.y - value.source.y) * interpolation)
  };
}

Size<int> TileAnimation::interpolate(const std::chrono::time_point<std::chrono::steady_clock> &now, const AnimationParameters<Size<int>> &value) {
  float fraction = value.fraction(now, start);
  if(fraction <= 0.0f)
    return value.source;
  if(fraction > fractionThreshold)
    return value.target;
  float interpolation = Animation::ease(fraction, value.easing);
  return {
    static_cast<int>(value.source.width + (value.target.width - value.source.width) * interpolation),
    static_cast<int>(value.source.height + (value.target.height - value.source.height) * interpolation)
  };
}

