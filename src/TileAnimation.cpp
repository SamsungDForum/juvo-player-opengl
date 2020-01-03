#include "TileAnimation.h"

#include <chrono>

TileAnimation::TileAnimation() : active(false) {}

TileAnimation::TileAnimation(AnimationParameters<std::pair<int, int>> position,
                             AnimationParameters<float> zoom,
                             AnimationParameters<std::pair<int, int>> size,
                             AnimationParameters<float> opacity)
  :
          start(std::chrono::steady_clock::now()),
          position(position),
          zoom(zoom),
          size(size),
          opacity(opacity),
          active(position.duration > std::chrono::duration_values<std::chrono::milliseconds>::zero() ||
                 zoom.duration > std::chrono::duration_values<std::chrono::milliseconds>::zero() ||
                 size.duration > std::chrono::duration_values<std::chrono::milliseconds>::zero() ||
                 opacity.duration > std::chrono::duration_values<std::chrono::milliseconds>::zero()) {
}

void TileAnimation::update(std::pair<int, int> &position, float &zoom, std::pair<int, int> &size, float &opacity) {
  //auto [position2, zoom2, size2, opacity2] = update(); // i'm compiling this under GCC 6.2.1, so structured bindings are not yet available...
  std::tuple<std::pair<int, int>, float, std::pair<int, int>, float> out = update();
  position = std::get<0>(out);
  zoom = std::get<1>(out);
  size = std::get<2>(out);
  opacity = std::get<3>(out);
}

std::tuple<std::pair<int, int>, float, std::pair<int, int>, float> TileAnimation::update() {
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

std::pair<int, int> TileAnimation::interpolate(const std::chrono::time_point<std::chrono::steady_clock> &now, const AnimationParameters<std::pair<int, int>> &value) {
  float fraction = value.fraction(now, start);
  if(fraction <= 0.0f)
    return value.source;
  if(fraction > fractionThreshold)
    return value.target;
  float interpolation = Animation::ease(fraction, value.easing);
  return { value.source.first + (value.target.first - value.source.first) * interpolation, value.source.second + (value.target.second - value.source.second) * interpolation };
}

