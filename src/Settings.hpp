#ifndef SETTINGS_HPP
#define SETTINGS_HPP

class Settings {
public:
  enum class RefreshRate {
    Hz60 = 60,
    Hz144 = 144,
    Hz240 = 240,
  };

  struct DifficultyValues {
    float jumpImpulse;
    float gapSize;
    float obstacleSpacing;
    float scrollSpeed;
    float gravity;
    float spawnRate;
  };

  Settings();

  void setRefreshRate(RefreshRate refreshRate);
  void setRefreshRate(int hz);

  int targetHz() const;
  float hzScale() const;
  float fixedDeltaSeconds() const;
  float centroidSmoothingAlpha() const;

  DifficultyValues scaledDifficulty() const;

private:
  RefreshRate refreshRate_;

  float baseJumpImpulse_;
  float baseGapSize_;
  float baseObstacleSpacing_;
  float baseScrollSpeed_;
  float baseGravity_;
  float baseSpawnRate_;
  float centroidSmoothingAlpha_;
};

#endif
