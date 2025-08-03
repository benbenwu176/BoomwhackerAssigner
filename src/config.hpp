#pragma once

#include <vector>

typedef struct Config {
  // Note data
  int num_notes; // number of notes in the piece

  // Parameter data
  int num_players; // The number of players in the ensemble
  int hold_limit; // The max # of boomwhackers that can be played at once
  int seed; // Optional RNG seed

  // Rate data
  double switch_time; // The time, in seconds, it takes a player to switch boomwhackers

  bool enable_whacker_priority = true;
  
  // std::vector<int> whacker_quantities = {
  //   2,3,5,3,5,5,3,5,3,5,3,5,
  //   7,4,4,4,4,4,4,5,4,4,4,4,
  //   2,2,2,2,2,2,2,2
  // };
  std::vector<int> whacker_quantities = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2
  };

  std::vector<double> switch_times = {
    2, 2, 2, 
    2, 2, 2,
    2, 2, 2
  };

  Config(int num_notes, const std::vector<int> &params, const std::vector<double> &rates);
} Config;

/**
 * @brief Initialize the config struct with the given parameters and rates.
 *
 * @param n The number of notes.
 * @param params The integer parameters.
 * @param rates The rate parameters.
 *
 * IMPORTANT: Maintain this with the order of the parameters and rates.
 */
inline Config::Config(int num_notes, const std::vector<int> &params, const std::vector<double> &rates) {
  // Note data
  this->num_notes = num_notes;

  // Parameter data
  num_players = params[0];
  hold_limit = params[1];
  seed = params[2];


  // Rate data
  switch_time = rates[0] - 0.0001; // Subtract by small amount to avoid floating point errors
}