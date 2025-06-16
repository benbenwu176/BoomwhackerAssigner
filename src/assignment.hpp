#pragma once

#include "note.hpp"
#include "player.hpp"
#include "boomwhacker.hpp"
#include "graph.hpp"

class Assignment {
public:
  std::vector<Note> notes;
  std::vector<Boomwhacker*> whackers;
  std::vector<Player> players;
  std::vector<std::vector<Note*>> mrp;
  Graph *adjacency_graph;

  Assignment(std::vector<int> &pitches, std::vector<double> &times);
  void init_notes(std::vector<int> &pitches, std::vector<double> &times);
  void init_players();
  void init_whackers();
  void init_mrp();
  void init_graph();
  void write();
  void assign();
};