#include "Animation.h"

Animation::Animation()
  : active(false) {
}

Animation::Animation(std::chrono::milliseconds duration,
                     std::chrono::milliseconds delay,
                     const std::vector<double> source,
                     const std::vector<double> target,
                     Easing easing,
                     std::function<void()> endWith)
  :
                     start(std::chrono::steady_clock::now()),
                     duration(duration),
                     delay(delay),
                     source(source),
                     target(target),
                     easing(easing),
                     active(duration > std::chrono::duration_values<std::chrono::milliseconds>::zero()),
                     endWith(endWith) {
  if(this->source.size() != this->target.size()) {
    size_t minSize = std::min(this->source.size(), this->target.size());
    this->source.resize(minSize);
    this->target.resize(minSize);
  }
  if(source.empty())
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
      return t < 0.5 ? 4 * t * t * t : 1 + 4 * (t - 1) * (t - 1) * (t - 1);
    case Easing::CubicOut:
      return 1 + (t - 1) * (t - 1) * (t - 1);
    case Easing::CubicIn:
      return t * t * t;
    case Easing::QuadInOut:
      return t < 0.5 ? 2 * t * t : 1 - 2 * (t - 1) * (t - 1);
    case Easing::QuadOut:
      return 1 - (t - 1) * (t - 1);
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
    return target;

  if(isDuringDelay())
    return source;

  double fraction = duration > std::chrono::duration_values<std::chrono::milliseconds>::zero() ?
    std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::steady_clock::now() - start - delay).count() / duration.count() :
    1.0;
  updateActivity(fraction);

  std::vector<double> values;
  for(size_t i = 0, sn = source.size(), tn = target.size(); i < sn && i < tn; ++i)
    values.push_back(source[i] + (target[i] - source[i]) * ease(fraction, easing));
  return values;
}

void Animation::updateActivity(double fraction) {
  if(active && fraction > 0.999) {
    active = false;
    if(endWith != nullptr)
      endWith();
  }
}

bool Animation::isActive() {
  return active;
}

bool Animation::isDuringDelay() {
  return active && (std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::steady_clock::now() - start) < delay);
}

