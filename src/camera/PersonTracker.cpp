#include "PersonTracker.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <utility>
#include <windows.h>

cv::CascadeClassifier PersonTracker::faceCascade_;
bool PersonTracker::cascadeLoaded_ = false;

namespace {
struct SurfaceDeleter {
  void operator()(SDL_Surface* ptr) const {
    if (ptr) {
      SDL_DestroySurface(ptr);
    }
  }
};

SDL_FRect clampRect(const SDL_FRect& rect, int frameWidth, int frameHeight) {
  SDL_FRect clamped = rect;

  if (clamped.w < 1.0f) {
    clamped.w = 1.0f;
  }
  if (clamped.h < 1.0f) {
    clamped.h = 1.0f;
  }

  if (clamped.w > static_cast<float>(frameWidth)) {
    clamped.w = static_cast<float>(frameWidth);
  }
  if (clamped.h > static_cast<float>(frameHeight)) {
    clamped.h = static_cast<float>(frameHeight);
  }

  if (clamped.x < 0.0f) {
    clamped.x = 0.0f;
  }
  if (clamped.y < 0.0f) {
    clamped.y = 0.0f;
  }

  if (clamped.x + clamped.w > static_cast<float>(frameWidth)) {
    clamped.x = static_cast<float>(frameWidth) - clamped.w;
  }
  if (clamped.y + clamped.h > static_cast<float>(frameHeight)) {
    clamped.y = static_cast<float>(frameHeight) - clamped.h;
  }

  if (clamped.x < 0.0f) {
    clamped.x = 0.0f;
  }
  if (clamped.y < 0.0f) {
    clamped.y = 0.0f;
  }

  return clamped;
}

SDL_FRect blendRect(const SDL_FRect& previousRect, const SDL_FRect& newRect) {
  constexpr float newWeight = 0.7f;
  constexpr float previousWeight = 0.3f;

  SDL_FRect blended;
  blended.x = newRect.x * newWeight + previousRect.x * previousWeight;
  blended.y = newRect.y * newWeight + previousRect.y * previousWeight;
  blended.w = newRect.w * newWeight + previousRect.w * previousWeight;
  blended.h = newRect.h * newWeight + previousRect.h * previousWeight;
  return blended;
}

void updateVelocity(TrackedPerson* trackedPerson, const SDL_FRect& previousRect,
                    const SDL_FRect& currentRect, float deltaSeconds) {
  if (!trackedPerson || deltaSeconds <= 0.0f) {
    return;
  }

  const float previousCenterX = previousRect.x + previousRect.w * 0.5f;
  const float previousCenterY = previousRect.y + previousRect.h * 0.5f;
  const float currentCenterX = currentRect.x + currentRect.w * 0.5f;
  const float currentCenterY = currentRect.y + currentRect.h * 0.5f;

  trackedPerson->velocityX = (currentCenterX - previousCenterX) / deltaSeconds;
  trackedPerson->velocityY = (currentCenterY - previousCenterY) / deltaSeconds;
}

SDL_FRect predictRect(const TrackedPerson& trackedPerson, float deltaSeconds) {
  SDL_FRect predicted = trackedPerson.estimatedRect;
  predicted.x += trackedPerson.velocityX * deltaSeconds;
  predicted.y += trackedPerson.velocityY * deltaSeconds;

  const float centerX = predicted.x + predicted.w * 0.5f;
  const float centerY = predicted.y + predicted.h * 0.5f;
  predicted.w *= 1.01f;
  predicted.h *= 1.01f;
  predicted.x = centerX - predicted.w * 0.5f;
  predicted.y = centerY - predicted.h * 0.5f;

  return predicted;
}

std::optional<SDL_FRect> findLargestMotionRect(const cv::Mat& previousGrayFrame,
                                               const cv::Mat& currentGrayFrame) {
  if (previousGrayFrame.empty() || currentGrayFrame.empty() ||
      previousGrayFrame.size() != currentGrayFrame.size()) {
    return std::nullopt;
  }

  cv::Mat diff;
  cv::absdiff(currentGrayFrame, previousGrayFrame, diff);

  cv::Mat blur;
  cv::GaussianBlur(diff, blur, cv::Size(5, 5), 0.0);

  cv::Mat thresh;
  cv::threshold(blur, thresh, 25, 255, cv::THRESH_BINARY);
  cv::dilate(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 2);

  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  double bestArea = 0.0;
  cv::Rect bestRect;
  for (const std::vector<cv::Point>& contour : contours) {
    const double area = cv::contourArea(contour);
    if (area < 400.0) {
      continue;
    }

    if (area > bestArea) {
      bestArea = area;
      bestRect = cv::boundingRect(contour);
    }
  }

  if (bestArea <= 0.0) {
    return std::nullopt;
  }

  return SDL_FRect{static_cast<float>(bestRect.x), static_cast<float>(bestRect.y),
                   static_cast<float>(bestRect.width), static_cast<float>(bestRect.height)};
}
}

bool PersonTracker::tryLoadCascade(const std::string& path) {
  return faceCascade_.load(path);
}

void PersonTracker::ensureCascadeLoaded() {
  if (cascadeLoaded_) {
    return;
  }

  std::vector<std::string> cascadePaths;

  wchar_t exePath[MAX_PATH];
  if (GetModuleFileNameW(nullptr, exePath, MAX_PATH)) {
    std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
    cascadePaths.push_back((exeDir / "haarcascade_frontalface_default.xml").string());
  }

  cascadePaths.insert(cascadePaths.end(), {
      "C:/msys64/ucrt64/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
      "C:/opencv/build/etc/haarcascades/haarcascade_frontalface_default.xml",
      "C:/Program Files/OpenCV/etc/haarcascades/haarcascade_frontalface_default.xml",
      "haarcascade_frontalface_default.xml"});

  for (const std::string& path : cascadePaths) {
    if (tryLoadCascade(path)) {
      cascadeLoaded_ = true;
      return;
    }
  }

  throw std::runtime_error("Failed to load haarcascade_frontalface_default.xml");
}

PersonTracker::PersonTracker()
    : mutex_(nullptr),
      workerThread_(nullptr),
      running_(true),
      hasPendingFrame_(false),
      pendingFrame_{},
      pendingTimeNS_(0),
      trackedPerson_{},
      previousGrayFrame_{},
      lastUpdateTimeNS_(0) {
  trackedPerson_.estimatedRect = SDL_FRect{0.0f, 0.0f, 0.0f, 0.0f};
  trackedPerson_.lastDetectedRect = SDL_FRect{0.0f, 0.0f, 0.0f, 0.0f};
  trackedPerson_.lastSeenTime = 0;
  trackedPerson_.hasValidDetection = false;
  trackedPerson_.velocityX = 0.0f;
  trackedPerson_.velocityY = 0.0f;

  mutex_ = SDL_CreateMutex();
  if (!mutex_) {
    throw std::runtime_error(std::string("Failed to create tracker mutex: ") + SDL_GetError());
  }

  ensureCascadeLoaded();

  workerThread_ = SDL_CreateThread(PersonTracker::workerMain, "FaceTrackerWorker", this);
  if (!workerThread_) {
    SDL_DestroyMutex(mutex_);
    mutex_ = nullptr;
    throw std::runtime_error(std::string("Failed to create tracker thread: ") + SDL_GetError());
  }
}

PersonTracker::~PersonTracker() {
  if (mutex_) {
    SDL_LockMutex(mutex_);
    running_ = false;
    SDL_UnlockMutex(mutex_);
  }

  if (workerThread_) {
    SDL_WaitThread(workerThread_, nullptr);
    workerThread_ = nullptr;
  }

  if (mutex_) {
    SDL_DestroyMutex(mutex_);
    mutex_ = nullptr;
  }
}

int PersonTracker::workerMain(void* userdata) {
  if (!userdata) {
    return -1;
  }
  return static_cast<PersonTracker*>(userdata)->runWorker();
}

int PersonTracker::runWorker() {
  while (true) {
    cv::Mat frame;
    Uint64 nowNS = 0;

    SDL_LockMutex(mutex_);
    const bool shouldRun = running_;
    if (hasPendingFrame_) {
      frame = std::move(pendingFrame_);
      nowNS = pendingTimeNS_;
      hasPendingFrame_ = false;
    }
    SDL_UnlockMutex(mutex_);

    if (!shouldRun) {
      break;
    }

    if (frame.empty()) {
      SDL_Delay(1);
      continue;
    }

    processFrame(frame, nowNS);
  }

  return 0;
}

void PersonTracker::submitFrame(SDL_Surface* frame, Uint64 nowNS) {
  if (!frame) {
    return;
  }

  SDL_Surface* rgbaFrame = frame;
  std::unique_ptr<SDL_Surface, SurfaceDeleter> convertedFrame;

  if (frame->format != SDL_PIXELFORMAT_RGBA32) {
    convertedFrame.reset(SDL_ConvertSurface(frame, SDL_PIXELFORMAT_RGBA32));
    if (!convertedFrame) {
      return;
    }
    rgbaFrame = convertedFrame.get();
  }

  cv::Mat cvFrame(rgbaFrame->h, rgbaFrame->w, CV_8UC4, rgbaFrame->pixels, rgbaFrame->pitch);
  cv::Mat frameCopy = cvFrame.clone();
  if (frameCopy.empty()) {
    return;
  }

  SDL_LockMutex(mutex_);
  pendingFrame_ = std::move(frameCopy);
  pendingTimeNS_ = nowNS;
  hasPendingFrame_ = true;
  SDL_UnlockMutex(mutex_);
}

std::optional<TrackedPerson> PersonTracker::processFrame(const cv::Mat& rgbaFrame, Uint64 nowNS) {
  if (rgbaFrame.empty()) {
    return std::nullopt;
  }

  ensureCascadeLoaded();

  TrackedPerson localTracked;
  cv::Mat localPreviousGray;
  Uint64 localLastUpdate = 0;

  SDL_LockMutex(mutex_);
  localTracked = trackedPerson_;
  localPreviousGray = previousGrayFrame_.clone();
  localLastUpdate = lastUpdateTimeNS_;
  SDL_UnlockMutex(mutex_);

  cv::Mat cvFrame = rgbaFrame;
  cv::Mat grayFrame;
  cv::cvtColor(cvFrame, grayFrame, cv::COLOR_RGBA2GRAY);
  cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
  clahe->apply(grayFrame, grayFrame);
  cv::fastNlMeansDenoising(grayFrame, grayFrame, 10, 7, 21);
  grayFrame.convertTo(grayFrame, CV_8U, 1.2, 30);

  const float deltaSeconds =
      (localLastUpdate == 0 || nowNS <= localLastUpdate)
          ? (1.0f / 30.0f)
          : std::max(0.0001f, static_cast<float>(nowNS - localLastUpdate) / 1000000000.0f);

  double faceScale = 1.0;
  cv::Mat detectGrayFrame;
  if (grayFrame.cols > 320) {
    faceScale = static_cast<double>(grayFrame.cols) / 320.0;
    cv::resize(grayFrame, detectGrayFrame, cv::Size(), 1.0 / faceScale, 1.0 / faceScale,
               cv::INTER_LINEAR);
  } else {
    detectGrayFrame = grayFrame;
  }

  std::vector<cv::Rect> smallFaces;
  faceCascade_.detectMultiScale(detectGrayFrame, smallFaces, 1.1, 3, 0, cv::Size(24, 24));

  bool detectedFace = false;
  SDL_FRect detectedRect = {0.0f, 0.0f, 0.0f, 0.0f};
  float bestArea = -1.0f;

  for (const cv::Rect& smallFace : smallFaces) {
    cv::Rect faceRect(static_cast<int>(std::lround(smallFace.x * faceScale)),
                      static_cast<int>(std::lround(smallFace.y * faceScale)),
                      static_cast<int>(std::lround(smallFace.width * faceScale)),
                      static_cast<int>(std::lround(smallFace.height * faceScale)));

    faceRect &= cv::Rect(0, 0, grayFrame.cols, grayFrame.rows);
    if (faceRect.width <= 1 || faceRect.height <= 1) {
      continue;
    }

    SDL_FRect faceBox = {static_cast<float>(faceRect.x), static_cast<float>(faceRect.y),
                         static_cast<float>(faceRect.width), static_cast<float>(faceRect.height)};
    const float area = faceBox.w * faceBox.h;
    if (area > bestArea) {
      bestArea = area;
      detectedRect = faceBox;
      detectedFace = true;
    }
  }

  if (detectedFace) {
    if (localTracked.hasValidDetection) {
      const SDL_FRect previousRect = localTracked.estimatedRect;
      localTracked.estimatedRect =
          clampRect(blendRect(localTracked.estimatedRect, detectedRect), grayFrame.cols,
                    grayFrame.rows);
      updateVelocity(&localTracked, previousRect, localTracked.estimatedRect, deltaSeconds);
    } else {
      localTracked.estimatedRect = clampRect(detectedRect, grayFrame.cols, grayFrame.rows);
      localTracked.velocityX = 0.0f;
      localTracked.velocityY = 0.0f;
    }

    localTracked.lastDetectedRect = detectedRect;
    localTracked.lastSeenTime = nowNS;
    localTracked.hasValidDetection = true;
  } else if (localTracked.hasValidDetection) {
    const Uint64 unseenDuration = nowNS - localTracked.lastSeenTime;
    if (unseenDuration > trackingTimeoutNS) {
      localTracked = TrackedPerson{};
    } else {
      const SDL_FRect previousRect = localTracked.estimatedRect;
      SDL_FRect predictedRect = predictRect(localTracked, deltaSeconds);
      std::optional<SDL_FRect> motionRect = findLargestMotionRect(localPreviousGray, grayFrame);
      if (motionRect.has_value()) {
        predictedRect = blendRect(predictedRect, motionRect.value());
      }
      localTracked.estimatedRect = clampRect(predictedRect, grayFrame.cols, grayFrame.rows);
      updateVelocity(&localTracked, previousRect, localTracked.estimatedRect, deltaSeconds);
    }
  }

  SDL_LockMutex(mutex_);
  trackedPerson_ = localTracked;
  previousGrayFrame_ = grayFrame.clone();
  lastUpdateTimeNS_ = nowNS;
  SDL_UnlockMutex(mutex_);

  if (!localTracked.hasValidDetection) {
    return std::nullopt;
  }

  return localTracked;
}

std::vector<SDL_FRect> PersonTracker::trackedRects() const {
  SDL_LockMutex(mutex_);
  const TrackedPerson localTracked = trackedPerson_;
  SDL_UnlockMutex(mutex_);

  if (!localTracked.hasValidDetection) {
    return {};
  }

  return {localTracked.estimatedRect};
}

std::optional<TrackedPerson> PersonTracker::latestTrackedPerson() const {
  SDL_LockMutex(mutex_);
  const TrackedPerson localTracked = trackedPerson_;
  SDL_UnlockMutex(mutex_);

  if (!localTracked.hasValidDetection) {
    return std::nullopt;
  }

  return localTracked;
}
