#pragma once

#include <vector>
#include "json.hpp"
#include <cstdint>
#include <filesystem>
#include <utility>

typedef struct Config {
  // Note data
  int num_notes; // number of notes in the piece
  
  // Player data
  uint32_t num_players;
  std::vector<uint32_t> hold_limits;
  std::vector<double> switch_times;
  std::vector<bool> one_handed_rolls;
  std::vector<std::vector<std::pair<double,double>>> excluded_ranges;

  // Whacker data
  std::vector<uint32_t> whacker_quantities;

  // OS temp directory
  std::filesystem::path tmp_dir;

  Config(int num_notes, const nlohmann::json& params, std::filesystem::path tmp_dir);
} Config;

// Initialize the config struct with the given parameters and rates.
inline Config::Config(int num_notes, const nlohmann::json& params, std::filesystem::path tmp_dir) {
  this->num_notes = num_notes;
  num_players = params["numPlayers"].get<uint32_t>();
  hold_limits = params["playerHoldLimits"].get<std::vector<uint32_t>>();
  switch_times = params["playerSwitchTimes"].get<std::vector<double>>();
  one_handed_rolls = params["playerOneHandedRolls"].get<std::vector<bool>>();
  excluded_ranges = params["playerExcludeRanges"].get<std::vector<std::vector<std::pair<double,double>>>>();
  whacker_quantities = params["whackerQuantities"].get<std::vector<uint32_t>>();
  this->tmp_dir = tmp_dir;
}