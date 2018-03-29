#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <chrono>
#include <vector>
#include <cmath>

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
  std::chrono::time_point<std::chrono::high_resolution_clock> animationStart;
  std::chrono::milliseconds animationDuration;
  std::chrono::milliseconds animationDelay;
  std::vector<double> sourceValues;
  std::vector<double> targetValues;
  Easing easing;
  bool active;

  static constexpr double pi() { return std::atan(1) * 4; }
  float ease(float fraction, Easing easing);

public:
  Animation();
  Animation(std::chrono::time_point<std::chrono::high_resolution_clock> animationStart,
            std::chrono::milliseconds animationDuration,
            std::chrono::milliseconds animationDelay,
            const std::vector<double> sourceValues,
            const std::vector<double> targetValues,
            Easing easing);
  ~Animation() = default;
  std::vector<double> update();
  bool isActive();
  bool isBounce();
};

#endif // _ANIMATION_H_
