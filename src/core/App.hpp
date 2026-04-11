#ifndef APP_HPP
#define APP_HPP

#include <SDL3/SDL.h>
#include <memory>

#include "Settings.hpp"
#include "../render/RenderManager.hpp"
#include "../camera/CameraDetector.hpp"
#include "../game/Game.hpp"

class Scoreboard;

class App {
public:
  App(std::shared_ptr<Scoreboard> scoreboard, int cameraDeviceIndex = 0);

  SDL_AppResult onEvent(const SDL_Event& event);
  SDL_AppResult onIterate();

private:
  void applyRefreshRate(int hz);

  Settings settings_;
  RenderManager renderManager_;
  CameraDetector cameraDetector_;
  Game game_;
  std::shared_ptr<Scoreboard> scoreboard_;
  Uint64 lastUpdateTimeNS_;
  float accumulatorSeconds_;
  bool worldInitialized_;
};

#endif
