#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <chrono>
#include <utility>
#include <cmath>

class TileAnimation {
public:
  typedef enum {
    QuintInOut, QuintOut, QuintIn,
    CubicInOut, CubicOut, CubicIn,
    QuartInOut, QuartOut, QuartIn,
    QuadInOut, QuadOut, QuadIn,
    BounceLeft, BounceRight,
    Linear
  } Easing;

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> animationStart;
  std::chrono::milliseconds animationDuration;
  std::chrono::milliseconds animationDelay;
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

public:
  TileAnimation();
  TileAnimation(std::chrono::time_point<std::chrono::high_resolution_clock> animationStart,
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
            Easing opacityEasingType);
  ~TileAnimation() = default;
  float doEasing(float fraction, Easing easing);
  void update(std::pair<int, int> &position, float &zoom, std::pair<int, int> &size, float &opacity);

  bool isActive() { return active; }
  bool isBounce() { return positionEasingType == Easing::BounceLeft || positionEasingType == Easing::BounceRight; }
  std::pair<int, int> getSourcePosition() { return sourcePosition; }
  std::pair<int, int> getTargetPosition() { return targetPosition; }
  float getSourceZoom() { return sourceZoom; }
  float getTargetZoom() { return targetZoom; }
  std::pair<int, int> getSourceSize() { return sourceSize; }
  std::pair<int, int> getTargetSize() { return targetSize; }
  float getSourceOpacity() { return sourceOpacity; }
  float getTargetOpacity() { return targetOpacity; }
};


#endif // _ANIMATION_H_
