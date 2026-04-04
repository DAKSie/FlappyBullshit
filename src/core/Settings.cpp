#include "Settings.hpp"

Settings::Settings()
    : refreshRate_(RefreshRate::Hz60),
      baseJumpImpulse_(250.0f),
      baseGapSize_(180.0f),
      baseObstacleSpacing_(500.0f),
      baseScrollSpeed_(400.0f),
      baseGravity_(400.0f),
      baseSpawnRate_(0.85f),
      centroidSmoothingAlpha_(0.35f) {}

void Settings::setRefreshRate(RefreshRate refreshRate) {
  refreshRate_ = refreshRate;
}

void Settings::setRefreshRate(int hz) {
  if (hz >= static_cast<int>(RefreshRate::Hz240)) {
    refreshRate_ = RefreshRate::Hz240;
    return;
  }
  if (hz >= static_cast<int>(RefreshRate::Hz144)) {
    refreshRate_ = RefreshRate::Hz144;
    return;
  }
  refreshRate_ = RefreshRate::Hz60;
}

int Settings::targetHz() const {
  return static_cast<int>(refreshRate_);
}

float Settings::hzScale() const {
  return static_cast<float>(targetHz()) / 60.0f;
}

float Settings::fixedDeltaSeconds() const {
  return 1.0f / static_cast<float>(targetHz());
}

float Settings::centroidSmoothingAlpha() const {
  return centroidSmoothingAlpha_;
}

Settings::DifficultyValues Settings::scaledDifficulty() const {
  const float scale = hzScale();

  Settings::DifficultyValues values;

  // Difficulty tuning: jump height when a detected jump triggers bird impulse.
  values.jumpImpulse = baseJumpImpulse_ * scale;
  // Difficulty tuning: vertical gap size between top and bottom obstacle blocks.
  values.gapSize = baseGapSize_;
  // Difficulty tuning: horizontal spawn offset for new obstacle pairs.
  values.obstacleSpacing = baseObstacleSpacing_;
  // Difficulty tuning: obstacle/world horizontal movement speed.
  values.scrollSpeed = baseScrollSpeed_ * scale;
  // Physics tuning: downward acceleration applied to the bird each update.
  values.gravity = baseGravity_ * scale;
  // Spawn tuning: obstacle pair spawn frequency in pairs per second.
  values.spawnRate = baseSpawnRate_ * scale;

  return values;
}
