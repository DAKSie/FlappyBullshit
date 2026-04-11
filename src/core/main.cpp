#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <exception>
#include <iostream>
#include <memory>
#include <string>

#include "App.hpp"
#include "Scoreboard.hpp"

static std::unique_ptr<App> app;

static std::string promptPlayerName() {
  std::cout << "Welcome to Flappy Bird!\n";
  std::cout << "Enter your name: ";
  std::cout.flush();

  std::string name;
  if (std::getline(std::cin, name)) {
    if (!name.empty()) {
      return name;
    }
  }

  return "Player";
}

static int promptCameraDevice() {
  if (!SDL_Init(SDL_INIT_CAMERA)) {
    throw std::runtime_error(std::string("SDL initialization failed: ") + SDL_GetError());
  }

  int devcount = 0;
  std::unique_ptr<SDL_CameraID, void (*)(void*)> devices(SDL_GetCameras(&devcount), SDL_free);
  if (!devices) {
    SDL_Quit();
    throw std::runtime_error(std::string("Camera enumeration failed: ") + SDL_GetError());
  }

  if (devcount == 0) {
    SDL_Quit();
    throw std::runtime_error("No camera devices found");
  }

  std::cout << "\nAvailable camera devices:\n";
  for (int i = 0; i < devcount; ++i) {
    const char* deviceName = SDL_GetCameraName(devices.get()[i]);
    if (!deviceName) {
      deviceName = "Unknown Device";
    }
    std::cout << i << ". " << deviceName << "\n";
  }

  SDL_Quit();

  std::cout << "\nSelect a camera device (0-" << (devcount - 1) << "): ";
  std::cout.flush();

  std::string input;
  if (std::getline(std::cin, input)) {
    try {
      int selectedDevice = std::stoi(input);
      if (selectedDevice >= 0 && selectedDevice < devcount) {
        return selectedDevice;
      }
    } catch (...) {
    }
  }

  std::cout << "Invalid selection. Using device 0.\n";
  return 0;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
  try {
    std::string playerName = promptPlayerName();
    int cameraDeviceIndex = promptCameraDevice();
    
    auto scoreboard = std::make_shared<Scoreboard>();
    if (!scoreboard->loadOrCreateProfile(playerName)) {
      SDL_Log("Warning: Could not load or create scoreboard profile");
    }

    app = std::make_unique<App>(scoreboard, cameraDeviceIndex);
    *appstate = app.get();
    return SDL_APP_CONTINUE;
  } catch (const std::exception& error) {
    SDL_Quit();
    SDL_Log("Initialization failed: %s", error.what());
    return SDL_APP_FAILURE;
  }
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  if (!appstate || !event) {
    return SDL_APP_FAILURE;
  }

  try {
    App* instance = static_cast<App*>(appstate);
    return instance->onEvent(*event);
  } catch (const std::exception& error) {
    SDL_Log("Event handling failed: %s", error.what());
    return SDL_APP_FAILURE;
  }
}

SDL_AppResult SDL_AppIterate(void* appstate) {
  if (!appstate) {
    return SDL_APP_FAILURE;
  }

  try {
    App* instance = static_cast<App*>(appstate);
    return instance->onIterate();
  } catch (const std::exception& error) {
    SDL_Log("Iteration failed: %s", error.what());
    return SDL_APP_FAILURE;
  }
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
  app.reset();
}
