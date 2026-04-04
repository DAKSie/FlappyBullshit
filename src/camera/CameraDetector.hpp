#ifndef CAMERA_DETECTOR_HPP
#define CAMERA_DETECTOR_HPP

#include "PersonTracker.h"

#include <SDL3/SDL.h>

#include <optional>
#include <vector>

class CameraDetector {
public:
  CameraDetector();

  void update(SDL_Surface* frame, Uint64 nowNS, bool submitForDetection, float smoothingAlpha);
  bool consumeJump();

  const std::vector<SDL_FRect>& trackedRects() const;
  std::optional<float> smoothedCentroidY() const;
  std::optional<SDL_FPoint> centroidDot() const;

  SDL_FRect triggerZoneRect(int frameHeight) const;
  bool isCentroidInZone(float centroidY) const;

private:
  // Zone-based jump trigger: pixels from top of screen where centroid enters/exits trigger zone.
  static constexpr float ZONE_START_Y = 120.0f;
  static constexpr float ZONE_END_Y = 240.0f;
  // ZONE_TOGGLE: true = jump on entry, false = jump on exit.
  static constexpr bool ZONE_TOGGLE = true;

  PersonTracker tracker_;
  std::vector<SDL_FRect> trackedRects_;
  std::optional<float> smoothedCentroidX_;
  std::optional<float> smoothedCentroidY_;
  bool wasCentroidInZone_;
  bool jumpPending_;
};

#endif
