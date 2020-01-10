#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <chrono>
#include <vector>
#include <cmath>
#include <functional>

class Animation {
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
  std::chrono::time_point<std::chrono::steady_clock> start;
  std::chrono::milliseconds duration;
  std::chrono::milliseconds delay;
  std::vector<double> source;
  std::vector<double> target;
  Easing easing;
  bool active;

  std::function<void()> endWith;

  void updateActivity(double fraction);

public:
  Animation();
  Animation(std::chrono::milliseconds duration,
            std::chrono::milliseconds delay,
            const std::vector<double> source,
            const std::vector<double> target,
            Easing easing,
            std::function<void()> endWith = nullptr);
  ~Animation() = default;
  Animation(const Animation&) = delete;
  Animation(Animation&&) = default;
  Animation& operator=(const Animation &other) = default;

  std::vector<double> update();
  bool isActive();
  bool isDuringDelay();
  static float ease(float fraction, Easing easing);
};

#endif // _ANIMATION_H_
