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
#include <random>
#include <thread>
#include <io.h>
#include <fcntl.h>
#include "gen.h"

Config cfg;
Assignment assignment;

/**
 * @brief Returns a random player number. Thread-safe.
 */
int random_player()
{
  thread_local std::mt19937 generator(std::random_device{}());
  std::uniform_int_distribution<int> distribution(0, cfg.num_players - 1);
  return distribution(generator);
}

Graph init_graph()
{
  Graph graph;
  graph.nodes = std::vector<Node>(cfg.num_players);
  for (int i = 0; i < cfg.num_players; i++)
  {
    graph.nodes[i].player = i;
    graph.nodes[i].edges = std::vector<Edge>();
  }
  return std::move(graph);
}

std::vector<std::vector<Note *>> init_mrp()
{
  std::vector<std::vector<Note *>> mrp(NUM_UNIQUE_PITCHES);
  for (int i = 0; i < NUM_UNIQUE_PITCHES; i++)
  {
    mrp[i] = std::vector<Note *>();
  }
  return std::move(mrp);
}

/**
 * @brief Attempts to add a note to a player's bucket. Returns the inverse of the maximum time differential
 * between notes. Successfully adds if the note is able to be played within the player's switch time.
 *
 */
double Player::attempt_add(Note *note, bool force)
{
  int dupe_index = -1;
  for (int i = 0; i < bucket.size(); i++)
  {
    if (bucket[i]->pitch == note->pitch && bucket[i]->capped == note->capped)
    {
      dupe_index = i;
      break;
    }
  }
  return 0;
}

std::vector<Player> init_players()
{
  std::vector<Player> players(cfg.num_players);
  for (int i = 0; i < cfg.num_players; i++)
  {
    players[i].id = i;
    players[i].whackers = std::vector<Boomwhacker *>();
    players[i].notes = std::vector<Note *>();
    players[i].bucket = std::vector<Note *>();
  }
  return std::move(players);
}

std::vector<Boomwhacker *> init_whackers()
{
  std::vector<Boomwhacker *> whackers(NUM_WHACKER_PITCHES * cfg.whackers_per_pitch);
  for (int i = 0; i < NUM_WHACKER_PITCHES; i++)
  {
    Boomwhacker *array = new Boomwhacker[cfg.whackers_per_pitch];
    for (int j = 0; j < cfg.whackers_per_pitch; j++)
    {
      array[j].pitch = i;
      array[j].used = false;
      array[j].capped = false;
      array[j].notes = std::vector<Note *>();
    }
    whackers[i] = array;
  }
  return std::move(whackers);
}

std::vector<Note> init_notes(std::vector<int> &pitches, std::vector<double> &times)
{
  std::vector<Note> notes(cfg.num_notes);
  for (int i = 0; i < cfg.num_notes; i++)
  {
    notes[i].time = times[i];
    notes[i].id = i;
    notes[i].player = -1;
    notes[i].pitch = pitches[i];
    notes[i].whacker_index = -1;
    notes[i].capped = false;
    notes[i].proximate = false;
    notes[i].conflicting = false;
  }
  return std::move(notes);
}

std::vector<int> flatten_graph()
{
  std::vector<int> flattened_graph(cfg.num_players);
  for (int i = 0; i < cfg.num_players; i++)
  {
    flattened_graph[i] = random_player();
  }
  return std::move(flattened_graph);
}

void write_assignment()
{
  Note *notes = assignment.notes.data();
  _setmode(_fileno(stdout), O_BINARY);
  // Write # of conflicts in the final assignment
  int num_conflicts = 0;
  for (int i = 0; i < cfg.num_notes; i++)
  {
    if (assignment.notes[i].conflicting)
    {
      num_conflicts++;
    }
  }
  std::cout.write(reinterpret_cast<const char *>(&num_conflicts), sizeof(int));
  // Write flattened graph of the final assignment
  std::vector<int> flattened_graph = flatten_graph();
  std::cout.write(reinterpret_cast<const char *>(flattened_graph.data()), sizeof(int) * cfg.num_players);
  // Write the notes in the final assignment
  std::cout.write(reinterpret_cast<const char *>(notes), sizeof(Note) * cfg.num_notes);
}

bool add_note(Note *note, int mode)
{
  if (note->pitch >= C2_MIDI)
  {
    for (int i = 0; i < assignment.mrp[note->pitch].size(); i++)
    {
      if (assignment.mrp[note->pitch][i] != nullptr)
      {
        // Add the note to the player's bucket
      }
    }
  }

  return true;
}

void init_assignment()
{
  assignment.notes = std::vector<Note>(cfg.num_notes);
  assignment.whackers = std::vector<Boomwhacker *>(NUM_WHACKER_PITCHES * cfg.whackers_per_pitch);
  assignment.players = std::vector<Player>(cfg.num_players);
  assignment.mrp = std::vector<std::vector<Note *>>(NUM_UNIQUE_PITCHES);
  assignment.adjacency_graph = Graph();
}

void assign(std::vector<int> &pitches, std::vector<double> &times)
{
  init_assignment();

  for (int i = 0; i < NUM_UNIQUE_PITCHES; i++) {
    int rplayer = random_player();
    for (int j = 0; j < cfg.num_notes; j++) {
      if (i + C2_MIDI == pitches[j]) {
        assignment.notes[j].player = rplayer;
      }
    }
  }
  // for (int i = 0; i < cfg.num_notes; i++)
  // {
  //   // add_note(&assignment.notes[i], FINAL_RESORT);
  //   assignment.notes[i].player = random_player();
  // }

  write_assignment();
}

/**
 * @brief Print the data from stdin containing pitches, times, parameters, and rates.
 *
 * @param n The number of notes.
 * @param p The number of parameters.
 * @param r The number of rates.
 * @param pitches The pitches of the notes.
 * @param times The times of the notes.
 * @param parameters The integer parameters.
 * @param rates The rate parameters.
 */
void check_params(int n, int p, int r, std::vector<int> &pitches, std::vector<double> &times,
                  std::vector<int> &parameters, std::vector<double> &rates)
{
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
    std::cerr << parameters[i] << " ";
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
 * @brief Initialize the config struct with the given parameters and rates.
 *
 * @param n The number of notes.
 * @param parameters The integer parameters.
 * @param rates The rate parameters.
 *
 * IMPORTANT: Maintain this with the order of the parameters and rates.
 */
void init_cfg(int n, const std::vector<int> &parameters, const std::vector<double> &rates)
{
  // Note data
  cfg.num_notes = n;

  // Parameter data
  cfg.num_players = parameters[0];
  cfg.whacker_overflow_limit = parameters[1];
  cfg.whackers_per_pitch = parameters[2];

  // Rate data
  cfg.switch_time = rates[0] + 0.0001; // Add by small amount to avoid floating point errors
}

/**
 * @brief Main function. Reads array lengths from args and uses a char buffer to read
 * in the array data from stdin. NOTE: This buffer must be padded to the nearest page size,
 * otherwise the read will fail and data will be corrupted.
 *
 * @return 0 on success, 1 on failure
 */
int main(int argc, char *argv[])
{
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
  int parameters_size = p * sizeof(int);
  if (p % 2 == 1)
  {
    parameters_size += sizeof(int);
  }
  // Add padding to the int array to round it up to a multiple of 8
  int rates_size = r * sizeof(double);
  int data_size = pitches_size + times_size + parameters_size + rates_size;
  int padding = 4096 - (data_size) % 4096;
  char *buffer = new char[data_size + padding];
  std::cin.read(reinterpret_cast<char *>(buffer), data_size + padding);

  std::vector<int> pitches(n);
  std::vector<double> times(n);
  std::vector<int> parameters(p);
  std::vector<double> rates(r);

  memcpy(pitches.data(), buffer, n * sizeof(int));
  memcpy(times.data(), buffer + pitches_size, n * sizeof(double));
  memcpy(parameters.data(), buffer + pitches_size + times_size, p * sizeof(int));
  memcpy(rates.data(), buffer + pitches_size + times_size + parameters_size, r * sizeof(double));
  delete[] buffer;

  check_params(n, p, r, pitches, times, parameters, rates);

  init_cfg(n, parameters, rates);

  assign(pitches, times);

  // for (int i = 0; i < cfg.num_notes; i++) {
  //     std::cerr << assignment.notes[i].player << " ";
  // }

  return 0;
}