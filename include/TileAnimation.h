#ifndef _TILEANIMATION_H_
#define _TILEANIMATION_H_

#include <chrono>
#include <utility>
#include <cmath>
#include <tuple>

#include "Animation.h"

class TileAnimation {
public:
  template<typename T>
  struct AnimationParameters {
    std::chrono::milliseconds duration;
    std::chrono::milliseconds delay;
    T source;
    T target;
    Animation::Easing easing;

    float fraction(const std::chrono::time_point<std::chrono::steady_clock> &now, const std::chrono::time_point<std::chrono::steady_clock> &start) const {
      return duration > std::chrono::duration_values<std::chrono::milliseconds>::zero() ?
        std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(now - start - delay).count() / duration.count() :
        1.0f;
    }
  };

private:
  std::chrono::time_point<std::chrono::steady_clock> start;
  AnimationParameters<std::pair<int, int>> position;
  AnimationParameters<float> zoom;
  AnimationParameters<std::pair<int, int>> size;
  AnimationParameters<float> opacity;
  bool active;
  static constexpr float fractionThreshold = 0.999f;

public:
  TileAnimation(AnimationParameters<std::pair<int, int>> position,
                AnimationParameters<float> zoom,
                AnimationParameters<std::pair<int, int>> size,
                AnimationParameters<float> opacity);
  TileAnimation();
  ~TileAnimation() = default;
  void update(std::pair<int, int> &position, float &zoom, std::pair<int, int> &size, float &opacity);
  std::tuple<std::pair<int, int>, float, std::pair<int, int>, float> update();
  float fraction(const std::chrono::time_point<std::chrono::steady_clock> &now, const std::chrono::milliseconds &duration, const std::chrono::milliseconds &delay);

  bool isActive() { return active; }
  std::pair<int, int> getSourcePosition() { return position.source; }
  std::pair<int, int> getTargetPosition() { return position.target; }
  float getSourceZoom() { return zoom.source; }
  float getTargetZoom() { return zoom.target; }
  std::pair<int, int> getSourceSize() { return size.source; }
  std::pair<int, int> getTargetSize() { return size.target; }
  float getSourceOpacity() { return opacity.source; }
  float getTargetOpacity() { return opacity.target; }
  float interpolate(const std::chrono::time_point<std::chrono::steady_clock> &now, const AnimationParameters<float> &value);
  std::pair<int, int> interpolate(const std::chrono::time_point<std::chrono::steady_clock> &now, const AnimationParameters<std::pair<int, int>> &value);
};


#endif // _TILEANIMATION_H_
