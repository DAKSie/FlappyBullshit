#ifndef OBSTACLE_HPP
#define OBSTACLE_HPP

#include <SDL3/SDL.h>

#include <random>
#include <vector>

class ObstacleManager {
public:
  ObstacleManager();

  void reset();
  void setGapSize(float gapSize);

  void update(float deltaSeconds, float worldWidth, float worldHeight, float scrollSpeed,
              float spawnRate, float obstacleSpacing);

  bool collides(const SDL_FRect& birdRect, float worldHeight) const;
  int consumePassedCount(float birdX);
  void render(SDL_Renderer* renderer, float worldHeight) const;

private:
  struct ObstaclePair {
    float x;
    float width;
    float gapCenterY;
    float gapSize;
    bool passed;
    bool active;
  };

  static bool intersects(const SDL_FRect& a, const SDL_FRect& b);
  SDL_FRect topRect(const ObstaclePair& obstacle) const;
  SDL_FRect bottomRect(const ObstaclePair& obstacle, float worldHeight) const;
  void spawn(float worldWidth, float worldHeight, float obstacleSpacing);
  ObstaclePair& allocatePair();

  std::vector<ObstaclePair> obstacles_;
  std::mt19937 rng_;
  float obstacleWidth_;
  float gapSize_;
  float spawnAccumulator_;
};

#endif
