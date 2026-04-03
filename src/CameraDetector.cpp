#include "CameraDetector.hpp"

#include <algorithm>
#include <cstdio>

CameraDetector::CameraDetector()
    : tracker_(),
      trackedRects_(),
      smoothedCentroidX_(std::nullopt),
      smoothedCentroidY_(std::nullopt),
      wasCentroidInZone_(false),
      jumpPending_(false) {}

void CameraDetector::update(SDL_Surface* frame, Uint64 nowNS, bool submitForDetection, float smoothingAlpha) {
  if (frame && submitForDetection) {
    tracker_.submitFrame(frame, nowNS);
  }

  const std::optional<TrackedPerson> trackedPerson = tracker_.latestTrackedPerson();
  trackedRects_ = tracker_.trackedRects();

  if (trackedPerson.has_value()) {
    const float centroidX = trackedPerson->estimatedRect.x + trackedPerson->estimatedRect.w * 0.5f;
    const float centroidY = trackedPerson->estimatedRect.y + trackedPerson->estimatedRect.h * 0.5f;
    const float alpha = std::clamp(smoothingAlpha, 0.01f, 1.0f);

    if (smoothedCentroidX_.has_value()) {
      smoothedCentroidX_ = alpha * centroidX + (1.0f - alpha) * smoothedCentroidX_.value();
    } else {
      smoothedCentroidX_ = centroidX;
    }

    if (smoothedCentroidY_.has_value()) {
      smoothedCentroidY_ = alpha * centroidY + (1.0f - alpha) * smoothedCentroidY_.value();
    } else {
      smoothedCentroidY_ = centroidY;
    }

    const SDL_FPoint dot = {smoothedCentroidX_.value(), smoothedCentroidY_.value()};
    const bool centroidInZone = isCentroidInZone(dot.y);
    const bool zoneEntered = !wasCentroidInZone_ && centroidInZone;
    const bool zoneExited = wasCentroidInZone_ && !centroidInZone;

    if (ZONE_TOGGLE && zoneEntered) {
      jumpPending_ = true;
      std::printf("JUMP: Y=%.2f (zone entry)\n", dot.y);
    } else if (!ZONE_TOGGLE && zoneExited) {
      jumpPending_ = true;
      std::printf("JUMP: Y=%.2f (zone exit)\n", dot.y);
    } else {
      std::printf("Face Y: %.2f (in_zone=%s)\n", dot.y, centroidInZone ? "true" : "false");
    }

    wasCentroidInZone_ = centroidInZone;
  } else {
    smoothedCentroidX_.reset();
    smoothedCentroidY_.reset();
    wasCentroidInZone_ = false;
    std::printf("Face Y: n/a\n");
  }
}

bool CameraDetector::consumeJump() {
  if (!jumpPending_) {
    return false;
  }
  jumpPending_ = false;
  return true;
}

const std::vector<SDL_FRect>& CameraDetector::trackedRects() const {
  return trackedRects_;
}

std::optional<float> CameraDetector::smoothedCentroidY() const {
  return smoothedCentroidY_;
}

std::optional<SDL_FPoint> CameraDetector::centroidDot() const {
  if (!smoothedCentroidX_.has_value() || !smoothedCentroidY_.has_value()) {
    return std::nullopt;
  }
  return SDL_FPoint{smoothedCentroidX_.value(), smoothedCentroidY_.value()};
}

SDL_FRect CameraDetector::triggerZoneRect(int frameHeight) const {
  (void)frameHeight;
  return SDL_FRect{0.0f, ZONE_START_Y, 10000.0f, ZONE_END_Y - ZONE_START_Y};
}

bool CameraDetector::isCentroidInZone(float centroidY) const {
  return centroidY >= ZONE_START_Y && centroidY <= ZONE_END_Y;
}
