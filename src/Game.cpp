#include "Game.hpp"

#include <array>
#include <cmath>
#include <string>

Game::Game()
    : bird_(),
      obstacles_(),
      score_(0),
      worldWidth_(0.0f),
      worldHeight_(0.0f),
      worldInitialized_(false),
      jumpQueued_(false),
      gameOver_(false),
      restarting_(false),
      restartCountdownSeconds_(0.0f) {}

void Game::setWorldSize(float width, float height) {
  if (!worldInitialized_ || std::fabs(width - worldWidth_) > 0.5f || std::fabs(height - worldHeight_) > 0.5f) {
    worldWidth_ = width;
    worldHeight_ = height;
    worldInitialized_ = true;
    resetRound();
  }
}

void Game::requestJump() {
  jumpQueued_ = true;
}

void Game::fixedUpdate(float fixedDeltaSeconds, const Settings::DifficultyValues& difficulty) {
  if (!worldInitialized_) {
    return;
  }

  if (restarting_) {
    restartCountdownSeconds_ -= fixedDeltaSeconds;
    if (restartCountdownSeconds_ <= 0.0f) {
      resetRound();
    }
    return;
  }

  obstacles_.setGapSize(difficulty.gapSize);

  if (jumpQueued_) {
    if (gameOver_) {
      startRestart();
    } else {
      bird_.jump(difficulty.jumpImpulse);
    }
    jumpQueued_ = false;
  }

  if (gameOver_) {
    return;
  }

  bird_.update(fixedDeltaSeconds, difficulty.gravity);

  obstacles_.update(fixedDeltaSeconds, worldWidth_, worldHeight_, difficulty.scrollSpeed,
                    difficulty.spawnRate, difficulty.obstacleSpacing);

  const SDL_FRect birdRect = bird_.bounds();
  if (birdRect.y <= 0.0f || birdRect.y + birdRect.h >= worldHeight_) {
    gameOver_ = true;
    return;
  }

  if (obstacles_.collides(birdRect, worldHeight_)) {
    gameOver_ = true;
    return;
  }

  score_ += obstacles_.consumePassedCount(bird_.x());
}

void Game::render(SDL_Renderer* renderer) const {
  if (!worldInitialized_) {
    return;
  }

  SDL_SetRenderDrawColor(renderer, 60, 210, 90, 165);
  obstacles_.render(renderer, worldHeight_);

  bird_.render(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 225);
  const std::string scoreText = std::to_string(score_);
  float scoreX = 18.0f;
  for (char c : scoreText) {
    drawDigit(renderer, c - '0', scoreX, 18.0f, 2.7f);
    scoreX += 16.0f * 2.7f;
  }

  if (gameOver_) {
    const SDL_FRect panel = {worldWidth_ * 0.22f, worldHeight_ * 0.35f, worldWidth_ * 0.56f,
                             worldHeight_ * 0.3f};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 255, 90, 90, 230);
    SDL_RenderRect(renderer, &panel);

    const SDL_FRect button = restartButtonRect();
    SDL_SetRenderDrawColor(renderer, 100, 200, 100, 200);
    SDL_RenderFillRect(renderer, &button);
    SDL_SetRenderDrawColor(renderer, 150, 255, 150, 255);
    SDL_RenderRect(renderer, &button);
  }

  if (restarting_) {
    const int countdownValue = static_cast<int>(std::ceil(restartCountdownSeconds_));
    drawDigit(renderer, countdownValue, worldWidth_ * 0.5f - 12.0f, worldHeight_ * 0.5f - 24.0f, 4.0f);
  }
}


void Game::drawDigit(SDL_Renderer* renderer, int digit, float x, float y, float scale) {
  if (digit < 0 || digit > 9) {
    return;
  }

  const float thickness = 2.0f * scale;
  const float segmentWidth = 8.0f * scale;
  const float segmentHeight = 12.0f * scale;

  SDL_FRect segments[7] = {
      SDL_FRect{x + thickness, y, segmentWidth, thickness},
      SDL_FRect{x + thickness + segmentWidth, y + thickness, thickness,
                segmentHeight * 0.5f - thickness},
      SDL_FRect{x + thickness + segmentWidth, y + segmentHeight * 0.5f, thickness,
                segmentHeight * 0.5f - thickness},
      SDL_FRect{x + thickness, y + segmentHeight, segmentWidth, thickness},
      SDL_FRect{x, y + segmentHeight * 0.5f, thickness, segmentHeight * 0.5f - thickness},
      SDL_FRect{x, y + thickness, thickness, segmentHeight * 0.5f - thickness},
      SDL_FRect{x + thickness, y + segmentHeight * 0.5f - thickness * 0.5f, segmentWidth,
                thickness},
  };

  static constexpr std::array<int, 10> masks = {
      0b0111111,
      0b0000110,
      0b1011011,
      0b1001111,
      0b1100110,
      0b1101101,
      0b1111101,
      0b0000111,
      0b1111111,
      0b1101111,
  };

  for (int i = 0; i < 7; ++i) {
    if ((masks[digit] & (1 << i)) != 0) {
      SDL_RenderFillRect(renderer, &segments[i]);
    }
  }
}

bool Game::isGameOver() const {
  return gameOver_;
}

bool Game::isRestarting() const {
  return restarting_;
}

float Game::restartCountdown() const {
  return restartCountdownSeconds_;
}

SDL_FRect Game::restartButtonRect() const {
  const float buttonWidth = 120.0f;
  const float buttonHeight = 50.0f;
  const float centerX = worldWidth_ * 0.5f - buttonWidth * 0.5f;
  const float centerY = worldHeight_ * 0.62f;
  return SDL_FRect{centerX, centerY, buttonWidth, buttonHeight};
}

void Game::handleRestartClick() {
  if (gameOver_ && !restarting_) {
    startRestart();
  }
}

void Game::startRestart() {
  restarting_ = true;
  restartCountdownSeconds_ = 3.0f;
}

void Game::resetRound() {
  bird_.reset(worldWidth_ * 0.22f, worldHeight_ * 0.5f);
  obstacles_.reset();
  score_ = 0;
  jumpQueued_ = false;
  gameOver_ = false;
  restarting_ = false;
  restartCountdownSeconds_ = 0.0f;
}
