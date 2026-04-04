#include "Obstacle.hpp"

#include <algorithm>

ObstacleManager::ObstacleManager()
    : obstacles_(),
      rng_(std::random_device{}()),
      obstacleWidth_(82.0f),
      gapSize_(180.0f),
      spawnAccumulator_(0.0f) {}

void ObstacleManager::reset() {
  for (ObstaclePair& obstacle : obstacles_) {
    obstacle.active = false;
    obstacle.passed = false;
  }
  spawnAccumulator_ = 0.0f;
}

void ObstacleManager::setGapSize(float gapSize) {
  gapSize_ = gapSize;
}

void ObstacleManager::update(float deltaSeconds, float worldWidth, float worldHeight,
                             float scrollSpeed, float spawnRate, float obstacleSpacing) {
  for (ObstaclePair& obstacle : obstacles_) {
    if (!obstacle.active) {
      continue;
    }

    obstacle.x -= scrollSpeed * deltaSeconds;
    if (obstacle.x + obstacle.width < 0.0f) {
      obstacle.active = false;
      obstacle.passed = false;
    }
  }

  spawnAccumulator_ += spawnRate * deltaSeconds;
  while (spawnAccumulator_ >= 1.0f) {
    spawnAccumulator_ -= 1.0f;
    spawn(worldWidth, worldHeight, obstacleSpacing);
  }
}

bool ObstacleManager::collides(const SDL_FRect& birdRect, float worldHeight) const {
  for (const ObstaclePair& obstacle : obstacles_) {
    if (!obstacle.active) {
      continue;
    }

    if (intersects(birdRect, topRect(obstacle)) || intersects(birdRect, bottomRect(obstacle, worldHeight))) {
      return true;
    }
  }
  return false;
}

int ObstacleManager::consumePassedCount(float birdX) {
  int passed = 0;
  for (ObstaclePair& obstacle : obstacles_) {
    if (!obstacle.active || obstacle.passed) {
      continue;
    }

    if (obstacle.x + obstacle.width < birdX) {
      obstacle.passed = true;
      ++passed;
    }
  }
  return passed;
}

void ObstacleManager::render(SDL_Renderer* renderer, float worldHeight) const {
  for (const ObstaclePair& obstacle : obstacles_) {
    if (!obstacle.active) {
      continue;
    }

    const SDL_FRect top = topRect(obstacle);
    const SDL_FRect bottom = bottomRect(obstacle, worldHeight);
    SDL_RenderFillRect(renderer, &top);
    SDL_RenderFillRect(renderer, &bottom);
  }
}

bool ObstacleManager::intersects(const SDL_FRect& a, const SDL_FRect& b) {
  return !(a.x + a.w <= b.x || b.x + b.w <= a.x || a.y + a.h <= b.y || b.y + b.h <= a.y);
}

SDL_FRect ObstacleManager::topRect(const ObstaclePair& obstacle) const {
  const float halfGap = obstacle.gapSize * 0.5f;
  const float topHeight = std::max(0.0f, obstacle.gapCenterY - halfGap);
  return SDL_FRect{obstacle.x, 0.0f, obstacle.width, topHeight};
}

SDL_FRect ObstacleManager::bottomRect(const ObstaclePair& obstacle, float worldHeight) const {
  const float halfGap = obstacle.gapSize * 0.5f;
  const float bottomY = std::min(worldHeight, obstacle.gapCenterY + halfGap);
  const float bottomHeight = std::max(0.0f, worldHeight - bottomY);
  return SDL_FRect{obstacle.x, bottomY, obstacle.width, bottomHeight};
}

void ObstacleManager::spawn(float worldWidth, float worldHeight, float obstacleSpacing) {
  ObstaclePair& obstacle = allocatePair();

  const float margin = 35.0f;
  const float halfGap = gapSize_ * 0.5f;
  const float minCenter = margin + halfGap;
  const float maxCenter = worldHeight - margin - halfGap;

  float gapCenter = worldHeight * 0.5f;
  if (maxCenter > minCenter) {
    std::uniform_real_distribution<float> distribution(minCenter, maxCenter);
    gapCenter = distribution(rng_);
  }

  obstacle.x = worldWidth + obstacleSpacing;
  obstacle.width = obstacleWidth_;
  obstacle.gapCenterY = gapCenter;
  obstacle.gapSize = gapSize_;
  obstacle.passed = false;
  obstacle.active = true;
}

ObstacleManager::ObstaclePair& ObstacleManager::allocatePair() {
  for (ObstaclePair& obstacle : obstacles_) {
    if (!obstacle.active) {
      return obstacle;
    }
  }

  obstacles_.push_back(ObstaclePair{0.0f, obstacleWidth_, 0.0f, gapSize_, false, false});
  return obstacles_.back();
}
