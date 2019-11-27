#include "Animation.h"

Animation::Animation()
  : active(false) {
}

Animation::Animation(std::chrono::time_point<std::chrono::high_resolution_clock> animationStart,
                     std::chrono::milliseconds animationDuration,
                     std::chrono::milliseconds animationDelay,
                     const std::vector<double> sourceValues,
                     const std::vector<double> targetValues,
                     Easing easing)
  :
                     animationStart(animationStart),
                     animationDuration(animationDuration),
                     animationDelay(animationDelay),
                     sourceValues(sourceValues),
                     targetValues(targetValues),
                     easing(easing),
                     active(animationDuration > std::chrono::milliseconds(0) ? true : false) {
  if(this->sourceValues.size() != this->targetValues.size()) {
    size_t minSize = std::min(this->sourceValues.size(), this->targetValues.size());
    this->sourceValues.resize(minSize);
    this->targetValues.resize(minSize);
  }
  if(sourceValues.empty())
    active = false;
}

float Animation::ease(float fraction, Easing easing) {
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

std::vector<double> Animation::update() {
  if(!active)
    return targetValues;
  std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
  if(now - animationStart < animationDelay)
    return sourceValues;

  std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(now - animationStart - animationDelay);
  std::chrono::duration<double> target = std::chrono::duration_cast<std::chrono::duration<double>>(animationDuration);
  double fraction = target.count() ? time_span.count() / target.count() : 1.0;
  active = fraction <= 0.999;

  std::vector<double> values;
  for(size_t i = 0, sn = sourceValues.size(), tn = targetValues.size(); i < sn && i < tn; ++i) {
    values.push_back(isBounce() ? targetValues[i] + ease(fraction, easing) : sourceValues[i] + (targetValues[i] - sourceValues[i]) * ease(fraction, easing));
  }
  return values;
}

bool Animation::isActive() {
  return active;
}

bool Animation::isBounce() {
  return easing == Easing::BounceLeft || easing == Easing::BounceRight;
}

