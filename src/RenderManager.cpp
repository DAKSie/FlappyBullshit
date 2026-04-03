#include "RenderManager.hpp"

#include <array>

RenderManager::RenderManager() : camera_(), currentRefreshRate_(60) {
  camera_.setVSync(true);
  SDL_SetRenderDrawBlendMode(camera_.renderer(), SDL_BLENDMODE_BLEND);
}

std::optional<RenderManager::FrameData> RenderManager::acquireFrame() {
  return camera_.acquireFrame();
}

void RenderManager::releaseFrame(SDL_Surface* frame) {
  camera_.releaseFrame(frame);
}

bool RenderManager::shouldDetect(Uint64 nowNS) {
  return camera_.shouldDetect(nowNS);
}

void RenderManager::updateCameraTexture(SDL_Surface* frame) {
  camera_.updateTexture(frame);
}

void RenderManager::beginFrame() {
  camera_.beginRender();
}

void RenderManager::endFrame() {
  camera_.present();
}

SDL_Renderer* RenderManager::renderer() const {
  return camera_.renderer();
}

int RenderManager::frameWidth() const {
  return camera_.frameWidth();
}

int RenderManager::frameHeight() const {
  return camera_.frameHeight();
}

bool RenderManager::applyRefreshRate(int targetHz) {
  currentRefreshRate_ = targetHz;
  camera_.setTargetFrameRate(targetHz);
  camera_.setTargetDetectRate(targetHz);
  camera_.setVSync(true);

  SDL_Window* window = camera_.window();
  if (!window) {
    return false;
  }

  int width = 0;
  int height = 0;
  SDL_GetWindowSize(window, &width, &height);

  const SDL_DisplayID displayID = SDL_GetDisplayForWindow(window);
  if (displayID == 0) {
    return false;
  }

  SDL_DisplayMode mode;
  if (SDL_GetClosestFullscreenDisplayMode(displayID, width, height, static_cast<float>(targetHz),
                                          true, &mode)) {
    return SDL_SetWindowFullscreenMode(window, &mode);
  }

  return false;
}

int RenderManager::currentRefreshRate() const {
  return currentRefreshRate_;
}

bool RenderManager::pickRefreshRateAt(float renderX, float renderY, int* outHz) const {
  if (!outHz) {
    return false;
  }

  const SDL_FRect panel = menuPanelRect();
  if (renderX < panel.x || renderY < panel.y || renderX > panel.x + panel.w || renderY > panel.y + panel.h) {
    return false;
  }

  const int options[3] = {60, 144, 240};
  for (int i = 0; i < 3; ++i) {
    const SDL_FRect box = menuBoxRect(i);
    if (renderX >= box.x && renderX <= box.x + box.w && renderY >= box.y && renderY <= box.y + box.h) {
      *outHz = options[i];
      return true;
    }
  }

  return false;
}

void RenderManager::drawRefreshRateMenu(int targetHz) const {
  SDL_Renderer* render = camera_.renderer();
  if (!render) {
    return;
  }

  const SDL_FRect panel = menuPanelRect();
  SDL_SetRenderDrawColor(render, 10, 10, 10, 120);
  SDL_RenderFillRect(render, &panel);

  const int options[3] = {60, 144, 240};
  for (int i = 0; i < 3; ++i) {
    const bool selected = options[i] == targetHz;

    SDL_FRect box = menuBoxRect(i);
    SDL_SetRenderDrawColor(render, selected ? 255 : 90, selected ? 220 : 90, 60, selected ? 220 : 140);
    SDL_RenderRect(render, &box);

    SDL_SetRenderDrawColor(render, selected ? 255 : 170, selected ? 230 : 170, selected ? 120 : 170,
                           selected ? 230 : 170);

    const int value = options[i];
    if (value >= 100) {
      drawDigit(render, (value / 100) % 10, box.x + 5.0f, box.y + 15.0f, 1.0f);
      drawDigit(render, (value / 10) % 10, box.x + 18.0f, box.y + 15.0f, 1.0f);
      drawDigit(render, value % 10, box.x + 31.0f, box.y + 15.0f, 1.0f);
    } else {
      drawDigit(render, (value / 10) % 10, box.x + 12.0f, box.y + 15.0f, 1.0f);
      drawDigit(render, value % 10, box.x + 25.0f, box.y + 15.0f, 1.0f);
    }
  }
}

SDL_FRect RenderManager::menuPanelRect() {
  return SDL_FRect{12.0f, 70.0f, 180.0f, 66.0f};
}

SDL_FRect RenderManager::menuBoxRect(int index) {
  return SDL_FRect{18.0f + static_cast<float>(index) * 56.0f, 78.0f, 50.0f, 50.0f};
}

void RenderManager::drawDigit(SDL_Renderer* renderer, int digit, float x, float y, float scale) {
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

void RenderManager::drawZoneOverlay(const SDL_FRect& zoneRect) const {
  SDL_Renderer* render = camera_.renderer();
  if (!render) {
    return;
  }

  SDL_SetRenderDrawColor(render, 0, 255, 0, 60);
  SDL_RenderFillRect(render, &zoneRect);

  SDL_SetRenderDrawColor(render, 0, 255, 0, 200);
  SDL_RenderRect(render, &zoneRect);
}
