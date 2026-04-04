#ifndef RENDER_MANAGER_HPP
#define RENDER_MANAGER_HPP

#include "../camera/CameraCapture.hpp"

#include <SDL3/SDL.h>

#include <optional>

class RenderManager {
public:
  using FrameData = CameraCapture::FrameData;

  RenderManager();

  std::optional<FrameData> acquireFrame();
  void releaseFrame(SDL_Surface* frame);
  bool shouldDetect(Uint64 nowNS);
  void updateCameraTexture(SDL_Surface* frame);

  void beginFrame();
  void endFrame();

  SDL_Renderer* renderer() const;
  int frameWidth() const;
  int frameHeight() const;

  bool applyRefreshRate(int targetHz);
  int currentRefreshRate() const;

  bool pickRefreshRateAt(float renderX, float renderY, int* outHz) const;
  void drawRefreshRateMenu(int targetHz) const;
  void drawZoneOverlay(const SDL_FRect& zoneRect) const;

private:
  static void drawDigit(SDL_Renderer* renderer, int digit, float x, float y, float scale);
  static SDL_FRect menuPanelRect();
  static SDL_FRect menuBoxRect(int index);

  CameraCapture camera_;
  int currentRefreshRate_;
};

#endif
