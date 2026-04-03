#include "JumpDetector.h"

#include <algorithm>
#include <cmath>

JumpDetector::JumpDetector()
    : filteredY_(std::nullopt),
      filteredVelocity_(0.0f),
      noiseFloor_(0.0f),
      phase_(0),
      jumpStartTimeNS_(0),
      jumpStartY_(0.0f),
      peakY_(0.0f) {}

void JumpDetector::reset() {
  filteredY_.reset();
  filteredVelocity_ = 0.0f;
  noiseFloor_ = 0.0f;
  phase_ = 0;
  jumpStartTimeNS_ = 0;
  jumpStartY_ = 0.0f;
  peakY_ = 0.0f;
}

bool JumpDetector::update(float trackedCentroidY, float trackedHeight, Uint64 nowNS) {
  if (!filteredY_.has_value()) {
    filteredY_ = trackedCentroidY;
    filteredVelocity_ = 0.0f;
    noiseFloor_ = 0.0f;
    return false;
  }

  const float previousY = filteredY_.value();
  const float filtered = trackedCentroidY * filterAlpha + previousY * (1.0f - filterAlpha);
  const float rawVelocity = filtered - previousY;
  filteredVelocity_ =
      rawVelocity * velocitySmoothing + filteredVelocity_ * (1.0f - velocitySmoothing);
  const float velocity = filteredVelocity_;
  filteredY_ = filtered;

  if (phase_ == 0) {
    noiseFloor_ = std::max(0.0005f,
                           std::fabs(velocity) * noiseSmoothing + noiseFloor_ * (1.0f - noiseSmoothing));
  }

  const float heightScale = trackedHeight > 0.0f ? trackedHeight : 1.0f;
  const float ascentThreshold = -(baseAscentThreshold + noiseFloor_ * ascentNoiseScale);
  const float descentThreshold = baseDescentThreshold + noiseFloor_ * descentNoiseScale;
  const float landingThreshold = baseLandingThreshold + noiseFloor_ * landingNoiseScale;
  const float adaptiveDisplacementThreshold =
      std::max(minDisplacementThreshold * heightScale, noiseFloor_ * displacementNoiseScale);

  if (phase_ == 0) {
    if (velocity < ascentThreshold) {
      phase_ = 1;
      jumpStartTimeNS_ = nowNS;
      jumpStartY_ = filtered;
      peakY_ = filtered;
    }
    return false;
  }

  const Uint64 durationMS = (nowNS - jumpStartTimeNS_) / 1000000ULL;

  if (phase_ == 1) {
    if (filtered < peakY_) {
      peakY_ = filtered;
    }

    if (velocity > descentThreshold) {
      phase_ = 2;
    }

    if (durationMS > maxJumpDurationMS) {
      reset();
    }

    return false;
  }

  if (phase_ == 2) {
    if (filtered < peakY_) {
      peakY_ = filtered;
    }

    if (durationMS > maxJumpDurationMS) {
      reset();
      return false;
    }

    if (std::fabs(velocity) <= landingThreshold) {
      const float displacement = jumpStartY_ - peakY_;
      const bool durationValid = durationMS >= minJumpDurationMS && durationMS <= maxJumpDurationMS;
      const bool displacementValid = displacement >= adaptiveDisplacementThreshold;
      const bool detected = durationValid && displacementValid;
      reset();
      filteredY_ = filtered;
      filteredVelocity_ = 0.0f;
      return detected;
    }
  }

  return false;
}
