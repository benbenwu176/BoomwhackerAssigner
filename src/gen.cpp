/**
 * @file gen.cpp
 * @brief Main file for the GenPlus program.
 * @version 0.1
 * @date 2/17/2025
 * @author Benjamin Wu
 *
 * TODO:
 * Write # of conflicts and flattened graph to stdout
 * Allow whackers per pitch to be unique to each pitch
 * Allow lowest note and highest note to vary
 * Allow different switch times per player for variable difficulty
 * Allow different whacker overflow limits for 4 mallet boomwhacker playing
 * Change NUM_UNIQUE_PITCHES to be a parameter and not a constant
 * Split functions into multiple files
 */

#include "gen.hpp"

Config* cfg;
Assignment* assignment;

// Print the data from stdin containing pitches, times, parameters, and rates.
void check_params(int num_notes, std::vector<int> pitches, std::vector<double> times, const nlohmann::json& params) {
  log("Number of notes:", num_notes);
  log_line();
  log("Number of players:", params["numPlayers"]);
  log_line();
  log("Parameters:", params.dump(2));
  log_line();
  log("Pitches:");
  for (uint32_t pitch : pitches) {
    log(pitch);
  }
  log_line();
  for (double time : times) {
    log(time);
  }
  log_line();
}

/**
 * @brief Main function. Reads array lengths from args and uses a char buffer to read
 * in the array data from stdin. NOTE: This buffer must be padded to the nearest page size,
 * otherwise the read will fail and data will be corrupted.
 *
 * @return 0 on success, 1 on failure
 */
int main(int argc, char* argv[]) {
  // Initialize error signal handlers
  error_handler::initialize_error_handlers();

  // Read in file path arguments
  if (argc != 5) {
    log(argc, argv[0]);
    log("Usage:", argv[0], "<tmp_dir> <params_path> <data_out_path> <num_notes>");
    return 1;
  }
  std::filesystem::path tmp_dir{argv[1]};
  std::filesystem::path params_path{argv[2]};
  std::filesystem::path data_out_path{argv[3]};
  int num_notes = atoi(argv[4]);

  // Read in JSON parameter object
  std::ifstream params_in(params_path);
  if (!params_in) {
    throw std::runtime_error("Cannot open parameters.");
  }
  nlohmann::json params;
  try {
    params_in >> params;
  } catch (const std::exception &e) {
    log("Error parsing JSON:", e.what());
    return 1;
  }

  // Read in note pitches and times
  std::ifstream bin_in(data_out_path, std::ios::binary);
  if (!bin_in) {
    throw std::runtime_error("Cannot open binary file.");
  }
  std::vector<int> pitches(num_notes);
  std::vector<double> times(num_notes);
  bin_in.read(reinterpret_cast<char*>(pitches.data()), num_notes * sizeof(int));
  bin_in.read(reinterpret_cast<char*>(times.data()), num_notes * sizeof(double));

  // Log the data that was just read
  check_params(num_notes, pitches, times, params);

  cfg = new Config(num_notes, params, tmp_dir);
  assignment = new Assignment(pitches, times);

  try {
    assignment->assign();
    assignment->write();
  } catch (const std::runtime_error& e) {
      std::cerr << "\nError: " << e.what() << std::endl;
      return 1;
  } catch (...) {
    std::cerr << "\nAn unknown error has occurred. " << std::endl;
    return 1;
  }

  return 0;
}