#include "gen.hpp"
#include "assignment.hpp"
#include <io.h>
#include <iostream>
#include <fcntl.h>

/**
 * @brief Creates an Assignment object with no assigned notes.
 */
Assignment::Assignment(std::vector<int> &pitches, std::vector<double> &times) {
  init_notes(pitches, times);
  init_players();
  init_whackers();
  init_mrp();
  init_graph();
}

/**
 * @brief Initialize the array of notes representing the piece.
 */
void Assignment::init_notes(std::vector<int> &pitches, std::vector<double> &times) {
  notes = std::vector<Note>(cfg->num_notes);
  for (int i = 0; i < cfg->num_notes; i++) {
    notes[i].time = times[i];
    notes[i].id = i;
    notes[i].player = -1;
    notes[i].pitch = pitches[i];
    notes[i].whacker_index = -1;
    notes[i].capped = false;
    notes[i].proximate = false;
    notes[i].conflicting = false;
  }
}

/**
 * @brief Initialize an array of players.
 */
void Assignment::init_players() {
  players = std::vector<Player>(cfg->num_players);
  for (int i = 0; i < cfg->num_players; i++) {
    players[i].id = i;
    players[i].whackers = std::vector<Boomwhacker *>();
    players[i].notes = std::vector<Note *>();
    players[i].bucket = std::vector<Note *>();
  }
}

/**
 * @brief Initialize the boomwhacker table.
 */
void Assignment::init_whackers() {
  whackers = std::vector<Boomwhacker *>(NUM_WHACKER_PITCHES * cfg->whackers_per_pitch);
  for (int i = 0; i < NUM_WHACKER_PITCHES; i++)
  {
    Boomwhacker *array = new Boomwhacker[cfg->whackers_per_pitch];
    for (int j = 0; j < cfg->whackers_per_pitch; j++)
    {
      array[j].pitch = i;
      array[j].used = false;
      array[j].capped = false;
      array[j].notes = std::vector<Note *>();
    }
    whackers[i] = array;
  }
}

/**
 * @brief Initialize the Most Recently Played cache.
 */
void Assignment::init_mrp() {
  mrp = std::vector<std::vector<Note *>>(NUM_UNIQUE_PITCHES);
  for (int i = 0; i < NUM_UNIQUE_PITCHES; i++)
  {
    mrp[i] = std::vector<Note *>();
  }
}

/**
 * @brief Initialize the player adjacency graph.
 */
void Assignment::init_graph() {
  adjacency_graph = new Graph();
}

/**
 * @brief Writes the player assignments to STDOUT
 */
void Assignment::write()
{
  Note *note_data = notes.data();
  _setmode(_fileno(stdout), O_BINARY);
  // Write # of conflicts in the final assignment
  int num_conflicts = 0;
  for (int i = 0; i < cfg->num_notes; i++)
  {
    if (notes[i].conflicting)
    {
      num_conflicts++;
    }
  }
  std::cout.write(reinterpret_cast<const char *>(&num_conflicts), sizeof(int));
  // Write flattened graph of the final assignment
  std::vector<int> flattened_graph = adjacency_graph->flatten();
  std::cout.write(reinterpret_cast<const char *>(flattened_graph.data()), sizeof(int) * cfg->num_players);
  // Write the notes in the final assignment
  std::cout.write(reinterpret_cast<const char *>(note_data), sizeof(Note) * cfg->num_notes);
}