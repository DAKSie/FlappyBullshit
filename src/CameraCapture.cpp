#include "CameraCapture.h"

#include <stdexcept>
#include <string>

void CameraCapture::WindowDeleter::operator()(SDL_Window* ptr) const {
  if (ptr) {
    SDL_DestroyWindow(ptr);
  }
}

void CameraCapture::RendererDeleter::operator()(SDL_Renderer* ptr) const {
  if (ptr) {
    SDL_DestroyRenderer(ptr);
  }
}

void CameraCapture::CameraDeleter::operator()(SDL_Camera* ptr) const {
  if (ptr) {
    SDL_CloseCamera(ptr);
  }
}

void CameraCapture::TextureDeleter::operator()(SDL_Texture* ptr) const {
  if (ptr) {
    SDL_DestroyTexture(ptr);
  }
}

CameraCapture::CameraCapture()
    : window_(nullptr),
      renderer_(nullptr),
      camera_(nullptr),
      texture_(nullptr),
    frameWidth_(defaultWidth),
    frameHeight_(defaultHeight),
    targetFrameTimeNS_(1000000000ULL / 60ULL),
    targetDetectTimeNS_(1000000000ULL / 60ULL),
      lastFrameTimeNS_(0),
      lastDetectTimeNS_(0) {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_CAMERA)) {
    throw std::runtime_error(std::string("SDL initialization failed: ") + SDL_GetError());
  }

  SDL_SetAppMetadata("Face Jump Detector", "1.0", "com.example.face-jump-detector");

  SDL_Window* rawWindow = nullptr;
  SDL_Renderer* rawRenderer = nullptr;
  if (!SDL_CreateWindowAndRenderer("Face Jump Detector", defaultWidth, defaultHeight,
                                   SDL_WINDOW_RESIZABLE, &rawWindow, &rawRenderer)) {
    throw std::runtime_error(std::string("Window/Renderer creation failed: ") + SDL_GetError());
  }

  window_.reset(rawWindow);
  renderer_.reset(rawRenderer);

  SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
  SDL_SetRenderVSync(renderer_.get(), 1);

  int devcount = 0;
  std::unique_ptr<SDL_CameraID, void (*)(void*)> devices(SDL_GetCameras(&devcount), SDL_free);
  if (!devices) {
    throw std::runtime_error(std::string("Camera enumeration failed: ") + SDL_GetError());
  }

  if (devcount == 0) {
    throw std::runtime_error("No camera devices found");
  }

  SDL_CameraSpec desiredSpec;
  SDL_zero(desiredSpec);
  desiredSpec.format = SDL_PIXELFORMAT_RGBA32;
  desiredSpec.width = defaultWidth;
  desiredSpec.height = defaultHeight;
  desiredSpec.framerate_numerator = defaultFPS;
  desiredSpec.framerate_denominator = 1;

  SDL_Camera* rawCamera = SDL_OpenCamera(devices.get()[0], &desiredSpec);
  if (!rawCamera) {
    rawCamera = SDL_OpenCamera(devices.get()[0], nullptr);
  }

  if (!rawCamera) {
    throw std::runtime_error(std::string("Camera open failed: ") + SDL_GetError());
  }

  camera_.reset(rawCamera);
}

CameraCapture::~CameraCapture() {
  texture_.reset();
  camera_.reset();
  renderer_.reset();
  window_.reset();
  SDL_Quit();
}

std::optional<CameraCapture::FrameData> CameraCapture::acquireFrame() {
  Uint64 nowNS = SDL_GetTicksNS();

  if (targetFrameTimeNS_ > 0 && lastFrameTimeNS_ != 0 && nowNS - lastFrameTimeNS_ < targetFrameTimeNS_) {
    SDL_DelayNS(targetFrameTimeNS_ - (nowNS - lastFrameTimeNS_));
    nowNS = SDL_GetTicksNS();
  }
  lastFrameTimeNS_ = nowNS;

  Uint64 timestampNS = 0;
  SDL_Surface* frame = SDL_AcquireCameraFrame(camera_.get(), &timestampNS);
  if (!frame) {
    return std::nullopt;
  }

  return FrameData{frame, timestampNS, nowNS};
}

void CameraCapture::releaseFrame(SDL_Surface* frame) {
  if (frame && camera_) {
    SDL_ReleaseCameraFrame(camera_.get(), frame);
  }
}

bool CameraCapture::shouldDetect(Uint64 nowNS) {
  if (targetDetectTimeNS_ == 0 || nowNS - lastDetectTimeNS_ >= targetDetectTimeNS_) {
    lastDetectTimeNS_ = nowNS;
    return true;
  }
  return false;
}

void CameraCapture::updateTexture(SDL_Surface* frame) {
  if (!frame) {
    return;
  }

  if (!texture_) {
    frameWidth_ = frame->w;
    frameHeight_ = frame->h;
    SDL_SetWindowSize(window_.get(), frame->w, frame->h);
    SDL_SetRenderLogicalPresentation(renderer_.get(), frame->w, frame->h,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    SDL_Texture* rawTexture =
        SDL_CreateTexture(renderer_.get(), frame->format, SDL_TEXTUREACCESS_STREAMING, frame->w,
                          frame->h);
    if (!rawTexture) {
      throw std::runtime_error(std::string("Texture creation failed: ") + SDL_GetError());
    }

    texture_.reset(rawTexture);
  }

  SDL_UpdateTexture(texture_.get(), nullptr, frame->pixels, frame->pitch);
}

void CameraCapture::beginRender() {
  SDL_SetRenderDrawColor(renderer_.get(), backgroundColor.r, backgroundColor.g, backgroundColor.b,
                         backgroundColor.a);
  SDL_RenderClear(renderer_.get());

  if (texture_) {
    SDL_RenderTexture(renderer_.get(), texture_.get(), nullptr, nullptr);
  }
}

void CameraCapture::present() {
  SDL_RenderPresent(renderer_.get());
}

SDL_Renderer* CameraCapture::renderer() const {
  return renderer_.get();
}

SDL_Window* CameraCapture::window() const {
  return window_.get();
}

void CameraCapture::setTargetFrameRate(int hz) {
  if (hz <= 0) {
    return;
  }
  targetFrameTimeNS_ = 1000000000ULL / static_cast<Uint64>(hz);
}

void CameraCapture::setTargetDetectRate(int hz) {
  if (hz <= 0) {
    return;
  }
  targetDetectTimeNS_ = 1000000000ULL / static_cast<Uint64>(hz);
}

void CameraCapture::setVSync(bool enabled) {
  SDL_SetRenderVSync(renderer_.get(), enabled ? 1 : 0);
}

int CameraCapture::frameWidth() const {
  return frameWidth_;
}

int CameraCapture::frameHeight() const {
  return frameHeight_;
}

void CameraCapture::render(const std::vector<SDL_FRect>& rects) {
  beginRender();
  if (texture_) {
    SDL_SetRenderDrawColor(renderer_.get(), rectColor.r, rectColor.g, rectColor.b, rectColor.a);
    for (const SDL_FRect& rect : rects) {
      SDL_RenderRect(renderer_.get(), &rect);
    }
  }
  present();
}
