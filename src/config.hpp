#pragma once

#include <vector>

typedef struct Config {
  // Note data
  int num_notes; // number of notes in the piece

  // Parameter data
  int num_players; // The number of players in the ensemble
  int whacker_overflow_limit; // The max # of boomwhackers that can be played at once
  int whackers_per_pitch; // The number of boomwhackers per pitch
  int seed; // Optional RNG seed

  // Rate data
  double switch_time; // The time, in seconds, it takes a player to switch boomwhackers

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
  whacker_overflow_limit = params[1];
  whackers_per_pitch = params[2];
  seed = params[3];

  // Rate data
  switch_time = rates[0] + 0.0001; // Add by small amount to avoid floating point errors
}