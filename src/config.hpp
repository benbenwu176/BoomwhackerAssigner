#pragma once

#include <vector>

class Config {
public:
  // Note data
  int num_notes; // number of notes in the piece

  // Parameter data
  int num_players; // The number of players in the ensemble
  int whacker_overflow_limit; // The max # of boomwhackers that can be played at once
  int whackers_per_pitch; // The number of boomwhackers per pitch

  // Rate data
  double switch_time; // The time, in seconds, it takes a player to switch boomwhackers

  Config(int num_notes, const std::vector<int> &params, const std::vector<double> &rates);
};