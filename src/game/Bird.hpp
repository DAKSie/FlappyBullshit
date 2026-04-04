#ifndef BIRD_HPP
#define BIRD_HPP

#include <SDL3/SDL.h>

class Bird {
public:
  Bird();

  void reset(float x, float y);
  void jump(float impulse);
  void update(float deltaSeconds, float gravity);
  void render(SDL_Renderer* renderer) const;

  SDL_FRect bounds() const;
  float x() const;

private:
  static constexpr float size_ = 20.0f;

  float x_;
  float y_;
  float velocityY_;
};

#endif
