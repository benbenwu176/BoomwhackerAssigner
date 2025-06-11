#include "config.hpp"

/**
 * @brief Initialize the config struct with the given parameters and rates.
 *
 * @param n The number of notes.
 * @param params The integer parameters.
 * @param rates The rate parameters.
 *
 * IMPORTANT: Maintain this with the order of the parameters and rates.
 */
Config::Config(int num_notes, const std::vector<int> &params, const std::vector<double> &rates) {
  // Note data
  this->num_notes = num_notes;

  // Parameter data
  this->num_players = params[0];
  this->whacker_overflow_limit = params[1];
  this->whackers_per_pitch = params[2];

  // Rate data
  this->switch_time = rates[0] + 0.0001; // Add by small amount to avoid floating point errors
}