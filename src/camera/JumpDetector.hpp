#ifndef JUMP_DETECTOR_HPP
#define JUMP_DETECTOR_HPP

#include <SDL3/SDL.h>
#include <optional>

class JumpDetector {
public:
  JumpDetector();

  bool update(float trackedCentroidY, float trackedHeight, Uint64 nowNS);
  void reset();

private:
  static constexpr float filterAlpha = 0.4f;
  static constexpr float velocitySmoothing = 0.5f;
  static constexpr float noiseSmoothing = 0.05f;
  static constexpr float baseAscentThreshold = 0.008f;
  static constexpr float baseDescentThreshold = 0.006f;
  static constexpr float baseLandingThreshold = 0.004f;
  static constexpr float minDisplacementThreshold = 0.02f;
  static constexpr float ascentNoiseScale = 2.0f;
  static constexpr float descentNoiseScale = 1.5f;
  static constexpr float landingNoiseScale = 1.2f;
  static constexpr float displacementNoiseScale = 6.0f;
  static constexpr Uint64 minJumpDurationMS = 70;
  static constexpr Uint64 maxJumpDurationMS = 700;

  std::optional<float> filteredY_;
  float filteredVelocity_;
  float noiseFloor_;
  int phase_;
  Uint64 jumpStartTimeNS_;
  float jumpStartY_;
  float peakY_;
};

#endif
