#include "TileAnimation.h"

float TileAnimation::doEasing(float fraction, Easing easing) {
  float t  = fraction;
  switch(easing) {
    case Easing::QuintInOut:
      return t < 0.5 ? 16 * t * t * t * t * t : 1 + 16 * (t - 1) * (t - 1) * (t - 1) * (t - 1) * (t - 1);
    case Easing::QuintOut:
      return 1 + (t - 1) * (t - 1)  * (t - 1) * (t - 1) * (t - 1);
    case Easing::QuintIn:
      return t * t * t * t * t;
    case Easing::QuartInOut:
      return t < 0.5 ? 8 * t * t * t * t : 1 - 8 * (t - 1) * (t - 1) * (t - 1) * (t - 1);
    case Easing::QuartOut:
      return 1 - (t - 1) * (t - 1) * (t - 1) * (t - 1);
    case Easing::QuartIn:
      return t * t * t * t;
    case Easing::CubicInOut:
      return t < 0.5 ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
    case Easing::CubicOut:
      return (t - 1) * (t - 1) * (t - 1) + 1;
    case Easing::CubicIn:
      return t * t * t;
    case Easing::QuadInOut:
      return t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t;
    case Easing::QuadOut:
      return t * (2 - t);
    case Easing::QuadIn:
      return t * t;
    case Easing::BounceLeft:
      return std::sin(t * M_PI * 2) * (1.0 - t) * 20.0 * -1;
    case Easing::BounceRight:
      return std::sin(t * M_PI * 2) * (1.0 - t) * 20.0;
    case Easing::Linear:
    default:
      return t;
  }
}

TileAnimation::TileAnimation() : active(false) {}
TileAnimation::TileAnimation(std::chrono::time_point<std::chrono::high_resolution_clock> animationStart,
          std::chrono::milliseconds animationDuration,
          std::chrono::milliseconds animationDelay,
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
          animationDelay(animationDelay),
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
          active(animationDuration > std::chrono::milliseconds(0) ? true : false) {
}

void TileAnimation::update(std::pair<int, int> &position, float &zoom, std::pair<int, int> &size, float &opacity) {
  if(!active) {
    position = targetPosition;
    zoom = targetZoom;
    size = targetSize;
    opacity = targetOpacity;
    return;
  }

  std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();

  //if(time_span.count() <= 0) {
  if(now - animationStart < animationDelay) {
    position = sourcePosition;
    zoom = sourceZoom;
    size = sourceSize;
    opacity = sourceOpacity;
    return;
  }
  std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(now - animationStart - animationDelay);

  std::chrono::duration<double> target = std::chrono::duration_cast<std::chrono::duration<double>>(animationDuration);
  float fraction = target.count() ? time_span.count() / target.count() : 1.0;
  switch(positionEasingType) {
    case Easing::BounceLeft:
    case Easing::BounceRight:
      position.first = targetPosition.first + doEasing(fraction, positionEasingType);
      position.second = targetPosition.second;// + doEasing(fraction, positionEasingType);
      break;
    default:
      position.first = sourcePosition.first + (targetPosition.first - sourcePosition.first) * doEasing(fraction, positionEasingType);
      position.second = sourcePosition.second + (targetPosition.second - sourcePosition.second) * doEasing(fraction, positionEasingType);
  }
  zoom = sourceZoom + (targetZoom - sourceZoom) * doEasing(fraction, zoomEasingType);
  size.first = sourceSize.first + (targetSize.first - sourceSize.first) * doEasing(fraction, sizeEasingType);
  size.second = sourceSize.second + (targetSize.second - sourceSize.second) * doEasing(fraction, sizeEasingType);
  opacity = sourceOpacity + (targetOpacity - sourceOpacity) * doEasing(fraction, opacityEasingType);
  if(fraction > 0.999) {
    active = false;
    return;
  }
}

