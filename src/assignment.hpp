#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare" // Ignore int to vector size warnings

#include <algorithm>
#include <optional>
#include <iostream>
#include "globals.hpp"
#include "note.hpp"
#include "player.hpp"
#include "boomwhacker.hpp"
#include "graph.hpp"
#include <cassert>


/* 
 * The MRP contains a pitch-indexable array of Players who have most recently played this
 * note. The array of players is organized as a pseudo priority queue. The person who most
 * recently played the note is at the front.
 */
class MRP {
public:
  MRP();
  std::vector<Player*>& get_queue(int pitch);
  void add(Player* player, Note* note);

private:
  std::vector<std::vector<Player*>> data;
};

class Assignment {
public:
  std::vector<Note> notes;
  std::vector<std::vector<Boomwhacker*>> whacker_table;
  std::vector<Player*> players;
  MRP* mrp;
  Graph* adjacency_graph;

  // Root functions
  Assignment(std::vector<int> &pitches, std::vector<double> &times);
  void init_notes(std::vector<int> &pitches, std::vector<double> &times);
  void init_players();
  void init_whackers();
  void init_mrp();
  void init_graph();
  void write();
  void assign();

  // Assignment functions
  std::optional<std::vector<Note*>> add_existing(Note* note);
  int add_offload(Note* note, std::vector<Note*> all_conflicts, add_flags flags);
  int add_new_whacker(Note* note);
  void skip(Note* note);
  void assign_note(Note* note, Boomwhacker* whacker, Player* player, bool new_whacker);
  int add_note(Note* note);
  Boomwhacker* find_whacker(int pitch);
  std::vector<Boomwhacker*> find_used_whackers(int pitch);
  std::vector<Note*> get_mrp_queue(int pitch, double time);
};

int random_player();