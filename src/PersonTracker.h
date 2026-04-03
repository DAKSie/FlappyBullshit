#ifndef PERSON_TRACKER_H
#define PERSON_TRACKER_H

#include <SDL3/SDL.h>
#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <optional>
#include <string>
#include <vector>

struct TrackedPerson {
  SDL_FRect estimatedRect;
  SDL_FRect lastDetectedRect;
  Uint64 lastSeenTime;
  bool hasValidDetection;
  float velocityX;
  float velocityY;
};

class PersonTracker {
public:
  PersonTracker();
  ~PersonTracker();

  void submitFrame(SDL_Surface* frame, Uint64 nowNS);
  std::optional<TrackedPerson> latestTrackedPerson() const;
  std::vector<SDL_FRect> trackedRects() const;

private:
  static cv::CascadeClassifier faceCascade_;
  static bool cascadeLoaded_;
  static constexpr Uint64 trackingTimeoutNS = 500000000ULL;
  static int workerMain(void* userdata);

  static bool tryLoadCascade(const std::string& path);
  static void ensureCascadeLoaded();
  int runWorker();
  std::optional<TrackedPerson> processFrame(const cv::Mat& rgbaFrame, Uint64 nowNS);

  SDL_Mutex* mutex_;
  SDL_Thread* workerThread_;
  bool running_;
  bool hasPendingFrame_;
  cv::Mat pendingFrame_;
  Uint64 pendingTimeNS_;
  TrackedPerson trackedPerson_;
  cv::Mat previousGrayFrame_;
  Uint64 lastUpdateTimeNS_;
};

#endif
