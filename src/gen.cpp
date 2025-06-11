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
#include <iostream>
#include <io.h>
#include <random>
#include <thread>
#include <fcntl.h>

#include "gen.hpp"

Config* cfg;
Assignment* assignment;

/**
 * @brief Returns a random player number. Thread-safe.
 */
int random_player() {
  thread_local std::mt19937 generator(std::random_device{}());
  std::uniform_int_distribution<int> distribution(0, cfg->num_players - 1);
  return distribution(generator);
}

/**
 * @brief Iterates through all notes and tries to assign them
 */
void assign() {

  for (int i = 0; i < NUM_UNIQUE_PITCHES; i++) {
    int rplayer = random_player();
    for (int j = 0; j < cfg->num_notes; j++) {
      if (i + C2_MIDI == assignment->notes[j].pitch) {
        assignment->notes[j].player = rplayer;
      }
    }
  }
  for (int i = 0; i < cfg->num_notes; i++) {
    // add_note(&assignment.notes[i], FINAL_RESORT);
  }

  assignment->write();
}

/**
 * @brief Print the data from stdin containing pitches, times, parameters, and rates.
 *
 * @param n The number of notes.
 * @param p The number of parameters.
 * @param r The number of rates.
 * @param pitches The pitches of the notes.
 * @param times The times of the notes.
 * @param params The integer parameters.
 * @param rates The rate parameters.
 */
void check_params(int n, int p, int r, std::vector<int> &pitches, std::vector<double> &times,
                  std::vector<int> &params, std::vector<double> &rates) {
  std::cerr << "Number of notes: " << n << std::endl;
  std::cerr << "Number of parameters: " << p << std::endl;
  std::cerr << "Number of rates: " << r << std::endl;
  std::cerr << "Pitches: ";
  for (int i = 0; i < n; i++)
  {
    std::cerr << pitches[i] << " ";
  }
  std::cerr << std::endl;
  std::cerr << "Times: ";
  for (int i = 0; i < n; i++)
  {
    std::cerr << times[i] << " ";
  }
  std::cerr << std::endl;
  std::cerr << "Parameters: ";
  for (int i = 0; i < p; i++)
  {
    std::cerr << params[i] << " ";
  }
  std::cerr << std::endl;
  std::cerr << "Rates: ";
  for (int i = 0; i < r; i++)
  {
    std::cerr << rates[i] << " ";
  }
  std::cerr << std::endl;
}

/**
 * @brief Main function. Reads array lengths from args and uses a char buffer to read
 * in the array data from stdin. NOTE: This buffer must be padded to the nearest page size,
 * otherwise the read will fail and data will be corrupted.
 *
 * @return 0 on success, 1 on failure
 */
int main(int argc, char *argv[]) {
  // Read n, p, r from args
  int n = atoi(argv[1]);
  int p = atoi(argv[2]);
  int r = atoi(argv[3]);

  // Read pitches, times, parameters, and rates from stdin
  int pitches_size = n * sizeof(int);
  if (n % 2 == 1)
  {
    pitches_size += sizeof(int);
  }
  // Add padding to the int array to round it up to a multiple of 8
  int times_size = n * sizeof(double);
  int params_size = p * sizeof(int);
  if (p % 2 == 1)
  {
    params_size += sizeof(int);
  }
  // Add padding to the int array to round it up to a multiple of 8
  int rates_size = r * sizeof(double);
  int data_size = pitches_size + times_size + params_size + rates_size;
  int padding = 4096 - (data_size) % 4096;
  char *buffer = new char[data_size + padding];
  std::cin.read(reinterpret_cast<char *>(buffer), data_size + padding);

  std::vector<int> pitches(n);
  std::vector<double> times(n);
  std::vector<int> params(p);
  std::vector<double> rates(r);

  memcpy(pitches.data(), buffer, n * sizeof(int));
  memcpy(times.data(), buffer + pitches_size, n * sizeof(double));
  memcpy(params.data(), buffer + pitches_size + times_size, p * sizeof(int));
  memcpy(rates.data(), buffer + pitches_size + times_size + params_size, r * sizeof(double));
  delete[] buffer;

  check_params(n, p, r, pitches, times, params, rates);

  cfg = new Config(n, params, rates);
  assignment = new Assignment(pitches, times);

  assign();

  return 0;
}