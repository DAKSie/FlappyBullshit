#ifndef GAME_HPP
#define GAME_HPP

#include "Bird.hpp"
#include "Obstacle.hpp"
#include "Settings.hpp"

#include <SDL3/SDL.h>

class Game {
public:
  Game();

  void setWorldSize(float width, float height);
  void requestJump();
  void fixedUpdate(float fixedDeltaSeconds, const Settings::DifficultyValues& difficulty);
  void render(SDL_Renderer* renderer) const;

  bool isGameOver() const;
  bool isRestarting() const;
  float restartCountdown() const;
  SDL_FRect restartButtonRect() const;
  void handleRestartClick();

private:
  static void drawDigit(SDL_Renderer* renderer, int digit, float x, float y, float scale);

  void resetRound();
  void startRestart();

  Bird bird_;
  ObstacleManager obstacles_;
  int score_;
  float worldWidth_;
  float worldHeight_;
  bool worldInitialized_;
  bool jumpQueued_;
  bool gameOver_;
  bool restarting_;
  float restartCountdownSeconds_;
};

#endif
