#include "App.hpp"

#include "Settings.hpp"
#include "../render/RenderManager.hpp"
#include "../camera/CameraDetector.hpp"
#include "../game/Game.hpp"
#include "Scoreboard.hpp"

#include <SDL3/SDL.h>
#include <algorithm>

App::App(std::shared_ptr<Scoreboard> scoreboard)
    : settings_(),
      renderManager_(),
      cameraDetector_(),
      game_(scoreboard),
      scoreboard_(scoreboard),
      lastUpdateTimeNS_(0),
      accumulatorSeconds_(0.0f),
      worldInitialized_(false) {
  applyRefreshRate(settings_.targetHz());
}

SDL_AppResult App::onEvent(const SDL_Event& event) {
  if (event.type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }
  if (event.type == SDL_EVENT_CAMERA_DEVICE_DENIED) {
    return SDL_APP_FAILURE;
  }

  if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
    if (event.key.key == SDLK_1) {
      applyRefreshRate(60);
    } else if (event.key.key == SDLK_2) {
      applyRefreshRate(144);
    } else if (event.key.key == SDLK_3) {
      applyRefreshRate(240);
    }
  }

  if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
    float renderX = event.button.x;
    float renderY = event.button.y;
    SDL_RenderCoordinatesFromWindow(renderManager_.renderer(), event.button.x, event.button.y,
                                    &renderX, &renderY);

    int pickedHz = 0;
    if (renderManager_.pickRefreshRateAt(renderX, renderY, &pickedHz)) {
      applyRefreshRate(pickedHz);
    } else {
      const SDL_FRect restartButton = game_.restartButtonRect();
      if (renderX >= restartButton.x && renderX <= restartButton.x + restartButton.w &&
          renderY >= restartButton.y && renderY <= restartButton.y + restartButton.h) {
        game_.handleRestartClick();
      }
    }
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult App::onIterate() {
  std::optional<RenderManager::FrameData> frameData = renderManager_.acquireFrame();
  Uint64 nowNS = SDL_GetTicksNS();

  if (frameData.has_value()) {
    SDL_Surface* frame = frameData->frame;
    nowNS = frameData->nowNS;

    struct FrameGuard {
      RenderManager& renderManager;
      SDL_Surface* frame;
      ~FrameGuard() {
        renderManager.releaseFrame(frame);
      }
    };

    FrameGuard guard{renderManager_, frame};
    renderManager_.updateCameraTexture(frame);

    game_.setWorldSize(static_cast<float>(frame->w), static_cast<float>(frame->h));
    worldInitialized_ = true;

    cameraDetector_.update(frame, frameData->nowNS,
                           renderManager_.shouldDetect(frameData->nowNS),
                           settings_.centroidSmoothingAlpha());
  } else if (!worldInitialized_) {
    game_.setWorldSize(static_cast<float>(renderManager_.frameWidth()),
                       static_cast<float>(renderManager_.frameHeight()));
    worldInitialized_ = true;
    cameraDetector_.update(nullptr, nowNS, false, settings_.centroidSmoothingAlpha());
  } else {
    cameraDetector_.update(nullptr, nowNS, false, settings_.centroidSmoothingAlpha());
  }

  if (lastUpdateTimeNS_ != 0 && nowNS > lastUpdateTimeNS_) {
    float frameDeltaSeconds = static_cast<float>(nowNS - lastUpdateTimeNS_) / 1000000000.0f;
    frameDeltaSeconds = std::clamp(frameDeltaSeconds, 0.0f, 0.25f);
    accumulatorSeconds_ += frameDeltaSeconds;
  }
  lastUpdateTimeNS_ = nowNS;

  if (cameraDetector_.consumeJump()) {
    game_.requestJump();
  }

  const float fixedDelta = settings_.fixedDeltaSeconds();
  const Settings::DifficultyValues difficulty = settings_.scaledDifficulty();

  while (accumulatorSeconds_ >= fixedDelta) {
    game_.fixedUpdate(fixedDelta, difficulty);
    accumulatorSeconds_ -= fixedDelta;
  }

  renderManager_.beginFrame();

  SDL_Renderer* renderer = renderManager_.renderer();
  game_.render(renderer);

  SDL_SetRenderDrawColor(renderer, 20, 240, 240, 180);
  for (const SDL_FRect& rect : cameraDetector_.trackedRects()) {
    SDL_RenderRect(renderer, &rect);
  }

  const std::optional<SDL_FPoint> centroidDot = cameraDetector_.centroidDot();
  if (centroidDot.has_value()) {
    const SDL_FRect dotRect = {centroidDot->x - 4.0f, centroidDot->y - 4.0f, 8.0f, 8.0f};
    SDL_SetRenderDrawColor(renderer, 255, 60, 60, 230);
    SDL_RenderFillRect(renderer, &dotRect);
  }

  renderManager_.drawZoneOverlay(cameraDetector_.triggerZoneRect(renderManager_.frameHeight()));

  renderManager_.drawRefreshRateMenu(settings_.targetHz());
  renderManager_.endFrame();

  return SDL_APP_CONTINUE;
}

void App::applyRefreshRate(int hz) {
  settings_.setRefreshRate(hz);
  if (!renderManager_.applyRefreshRate(settings_.targetHz())) {
    SDL_Log("Refresh mode switch to %d Hz not fully applied (timing scale still applied)",
            settings_.targetHz());
  } else {
    SDL_Log("Refresh rate switched to %d Hz", settings_.targetHz());
  }
}
