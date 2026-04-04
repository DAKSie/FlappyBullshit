#include "Scoreboard.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

const char* Scoreboard::scoreboardFilename = "scoreboard.txt";

Scoreboard::Scoreboard() : playerName_(""), bestScore_(0) {}

bool Scoreboard::loadOrCreateProfile(const std::string& playerName) {
  playerName_ = playerName;
  bestScore_ = 0;

  if (!readScoreboard()) {
    return writeScoreboard();
  }
  return true;
}

bool Scoreboard::updateScore(int newScore) {
  if (newScore > bestScore_) {
    bestScore_ = newScore;
    return writeScoreboard();
  }
  return true;
}

const std::string& Scoreboard::playerName() const {
  return playerName_;
}

int Scoreboard::bestScore() const {
  return bestScore_;
}

bool Scoreboard::readScoreboard() {
  std::ifstream file(scoreboardFilename);
  if (!file.is_open()) {
    return false;
  }

  std::string line;
  std::string filePlayerName;
  int fileScore = 0;

  if (std::getline(file, line)) {
    size_t colonPos = line.find(':');
    if (colonPos != std::string::npos) {
      filePlayerName = line.substr(0, colonPos);
      try {
        fileScore = std::stoi(line.substr(colonPos + 1));
      } catch (...) {
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }

  if (filePlayerName == playerName_) {
    bestScore_ = fileScore;
    return true;
  }

  return false;
}

bool Scoreboard::writeScoreboard() {
  std::ofstream file(scoreboardFilename);
  if (!file.is_open()) {
    return false;
  }

  file << playerName_ << ":" << bestScore_ << "\n";
  file.close();

  return true;
}
