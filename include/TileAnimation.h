#ifndef _TILEANIMATION_H_
#define _TILEANIMATION_H_

#include <chrono>
#include <utility>
#include <cmath>
#include <tuple>

#include "Animation.h"
#include "Utility.h"

class TileAnimation {
public:
  template<typename T>
  struct AnimationParameters {
    std::chrono::milliseconds duration;
    std::chrono::milliseconds delay;
    T source;
    T target;
    Animation::Easing easing;

    AnimationParameters() : AnimationParameters(T{}) {}

    AnimationParameters(T staticValue) : AnimationParameters(
      std::chrono::duration_values<std::chrono::milliseconds>::zero(),
      std::chrono::duration_values<std::chrono::milliseconds>::zero(),
      staticValue,
      staticValue,
      Animation::Easing::Linear) {
    }

    AnimationParameters(std::chrono::milliseconds duration, std::chrono::milliseconds delay, T source, T target, Animation::Easing easing) :
      duration(duration),
      delay(delay),
      source(source),
      target(target),
      easing(easing) {
    }


    float fraction(const std::chrono::time_point<std::chrono::steady_clock> &now, const std::chrono::time_point<std::chrono::steady_clock> &start) const {
      return isDurationPositive(duration) ?
        std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(now - start - delay).count() / duration.count() :
        1.0f;
    }
  };

  struct UpdatedValues {
    Position<int> position;
    float zoom;
    Size<int> size;
    float opacity;
  };

private:
  std::chrono::time_point<std::chrono::steady_clock> start;
  AnimationParameters<Position<int>> position;
  AnimationParameters<float> zoom;
  AnimationParameters<Size<int>> size;
  AnimationParameters<float> opacity;
  bool active;
  static constexpr float fractionThreshold = 0.999f;

  static bool isDurationPositive(std::chrono::milliseconds duration) {
    return duration > std::chrono::duration_values<std::chrono::milliseconds>::zero();
  }

public:
  TileAnimation(AnimationParameters<Position<int>> position,
                AnimationParameters<float> zoom,
                AnimationParameters<Size<int>> size,
                AnimationParameters<float> opacity);
  TileAnimation(Position<int> position, float zoom, Size<int> size, float opacity);
  TileAnimation();
  ~TileAnimation() = default;
  void update(Position<int> &position, float &zoom, Size<int> &size, float &opacity);
  UpdatedValues update();
  float fraction(const std::chrono::time_point<std::chrono::steady_clock> &now, const std::chrono::milliseconds &duration, const std::chrono::milliseconds &delay);

  bool isActive() { return active; }
  Position<int> getSourcePosition() { return position.source; }
  Position<int> getTargetPosition() { return position.target; }
  float getSourceZoom() { return zoom.source; }
  float getTargetZoom() { return zoom.target; }
  Size<int> getSourceSize() { return size.source; }
  Size<int> getTargetSize() { return size.target; }
  float getSourceOpacity() { return opacity.source; }
  float getTargetOpacity() { return opacity.target; }
  float interpolate(const std::chrono::time_point<std::chrono::steady_clock> &now, const AnimationParameters<float> &value);
  Position<int> interpolate(const std::chrono::time_point<std::chrono::steady_clock> &now, const AnimationParameters<Position<int>> &value);
  Size<int> interpolate(const std::chrono::time_point<std::chrono::steady_clock> &now, const AnimationParameters<Size<int>> &value);
};


#endif // _TILEANIMATION_H_
