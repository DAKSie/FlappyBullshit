#include "Bird.hpp"

Bird::Bird() : x_(0.0f), y_(0.0f), velocityY_(0.0f) {}

void Bird::reset(float x, float y) {
  x_ = x;
  y_ = y;
  velocityY_ = 0.0f;
}

void Bird::jump(float impulse) {
  velocityY_ = -impulse;
}

void Bird::update(float deltaSeconds, float gravity) {
  velocityY_ += gravity * deltaSeconds;
  y_ += velocityY_ * deltaSeconds;
}

void Bird::render(SDL_Renderer* renderer) const {
  SDL_Vertex vertices[3];
  vertices[0].position = SDL_FPoint{x_ + size_, y_};
  vertices[1].position = SDL_FPoint{x_ - size_, y_ - size_ * 0.75f};
  vertices[2].position = SDL_FPoint{x_ - size_, y_ + size_ * 0.75f};

  vertices[0].color = SDL_FColor{1.0f, 0.86f, 0.2f, 0.95f};
  vertices[1].color = SDL_FColor{1.0f, 0.65f, 0.1f, 0.95f};
  vertices[2].color = SDL_FColor{1.0f, 0.65f, 0.1f, 0.95f};

  vertices[0].tex_coord = SDL_FPoint{0.0f, 0.0f};
  vertices[1].tex_coord = SDL_FPoint{0.0f, 0.0f};
  vertices[2].tex_coord = SDL_FPoint{0.0f, 0.0f};

  SDL_RenderGeometry(renderer, nullptr, vertices, 3, nullptr, 0);
}

SDL_FRect Bird::bounds() const {
  return SDL_FRect{x_ - size_, y_ - size_ * 0.75f, size_ * 2.0f, size_ * 1.5f};
}

float Bird::x() const {
  return x_;
}
