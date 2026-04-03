#ifndef CAMERA_CAPTURE_H
#define CAMERA_CAPTURE_H

#include <SDL3/SDL.h>
#include <memory>
#include <optional>
#include <vector>

class CameraCapture {
public:
  struct FrameData {
    SDL_Surface* frame;
    Uint64 timestampNS;
    Uint64 nowNS;
  };

  CameraCapture();
  ~CameraCapture();

  std::optional<FrameData> acquireFrame();
  void releaseFrame(SDL_Surface* frame);
  bool shouldDetect(Uint64 nowNS);
  void updateTexture(SDL_Surface* frame);
  void beginRender();
  void present();
  SDL_Renderer* renderer() const;
  SDL_Window* window() const;
  void setTargetFrameRate(int hz);
  void setTargetDetectRate(int hz);
  void setVSync(bool enabled);
  int frameWidth() const;
  int frameHeight() const;
  void render(const std::vector<SDL_FRect>& rects);

private:
  struct WindowDeleter {
    void operator()(SDL_Window* ptr) const;
  };

  struct RendererDeleter {
    void operator()(SDL_Renderer* ptr) const;
  };

  struct CameraDeleter {
    void operator()(SDL_Camera* ptr) const;
  };

  struct TextureDeleter {
    void operator()(SDL_Texture* ptr) const;
  };

  static constexpr int defaultWidth = 640;
  static constexpr int defaultHeight = 480;
  static constexpr int defaultFPS = 60;
  static constexpr SDL_Color backgroundColor = {0x99, 0x99, 0x99, SDL_ALPHA_OPAQUE};
  static constexpr SDL_Color rectColor = {0x00, 0xFF, 0x00, SDL_ALPHA_OPAQUE};

  std::unique_ptr<SDL_Window, WindowDeleter> window_;
  std::unique_ptr<SDL_Renderer, RendererDeleter> renderer_;
  std::unique_ptr<SDL_Camera, CameraDeleter> camera_;
  std::unique_ptr<SDL_Texture, TextureDeleter> texture_;
  int frameWidth_;
  int frameHeight_;
  Uint64 targetFrameTimeNS_;
  Uint64 targetDetectTimeNS_;
  Uint64 lastFrameTimeNS_;
  Uint64 lastDetectTimeNS_;
};

#endif
