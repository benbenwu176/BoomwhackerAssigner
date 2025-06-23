#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare" // Ignore int to vector size warnings

#include <algorithm>
#include <iostream>
#include "globals.hpp"
#include "note.hpp"
#include "player.hpp"
#include "boomwhacker.hpp"
#include "graph.hpp"


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

  Assignment(std::vector<int> &pitches, std::vector<double> &times);
  void init_notes(std::vector<int> &pitches, std::vector<double> &times);
  void init_players();
  void init_whackers();
  void init_mrp();
  void init_graph();
  void write();
  void assign();
  int add_note(Note* note, add_flags flags);
  Boomwhacker* find_whacker(int pitch);
};

int random_player();