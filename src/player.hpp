#pragma once

#include "note.hpp"
#include "boomwhacker.hpp"
#include "globals.hpp"
#include <unordered_set>
#include <iterator>
#include <algorithm>
#include <type_traits>
#include <cmath>

class Player {
public:
  int id;
  int hold_limit;
  double switch_time;
  bool one_handed_roll;
  std::vector<Boomwhacker*> whackers; // The boomwhackers that the player has
  std::vector<Note*> notes; // The notes that the player has played

  Player(int id, int hold_limit, double switch_time, bool one_handed_roll);
  bool conflicts(Note* before, Note* after);
  std::vector<Note*> conflicts_back(std::vector<Note*>::iterator end, Note* note);
  std::vector<Note*> conflicts_front(std::vector<Note*>::iterator end, Note* note);
  void add_whacker(Boomwhacker* whacker);
  void add_note(Note* note);
  void show_whackers();
  std::vector<Boomwhacker*>::iterator get_whacker(int pitch);
};