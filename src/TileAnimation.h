#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <chrono>

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

    float doEasing(float fraction, Easing easing) {
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
          return std::sin(t * M_PI * 3) * (1.0 - t) * (192.0 / 5) * -1;
        case Easing::BounceRight:
          return std::sin(t * M_PI * 3) * (1.0 - t) * (192.0 / 5);
        case Easing::Linear:
        default:
          return t;
      }

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
              active(true) {
    }

    void update(std::pair<int, int> &position, float &zoom, std::pair<int, int> &size, float &opacity) {
      if(!active) {
        position = targetPosition;
        zoom = targetZoom;
        size = targetSize;
        opacity = targetOpacity;
        return;
      }
      std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(now - animationStart);
      std::chrono::duration<double> target = std::chrono::duration_cast<std::chrono::duration<double>>(animationDuration);
      float fraction = time_span.count() / target.count();
      if(fraction >= 1.0) {
        active = false;
        return;
      }
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
    }

    bool isActive() {
      return active;
    }

    bool isBounce() {
      return positionEasingType == Easing::BounceLeft || positionEasingType == Easing::BounceRight;
    }

    ~TileAnimation() = default;

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
