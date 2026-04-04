#ifndef SCOREBOARD_HPP
#define SCOREBOARD_HPP

#include <string>

class Scoreboard {
public:
  Scoreboard();
  
  bool loadOrCreateProfile(const std::string& playerName);
  bool updateScore(int newScore);
  
  const std::string& playerName() const;
  int bestScore() const;

private:
  static const char* scoreboardFilename;
  
  std::string playerName_;
  int bestScore_;
  
  bool readScoreboard();
  bool writeScoreboard();
};

#endif
