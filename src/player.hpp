#pragma once

#include "note.hpp"
#include "boomwhacker.hpp"
#include "globals.hpp"
#include "run.hpp"
#include <unordered_set>
#include <iterator>
#include <algorithm>
#include <type_traits>
#include <cmath>
#include <optional>
#include <utility>

class Player {
public:
  int id;
  int hold_limit;
  double switch_time;
  bool one_handed_roll;
  std::vector<std::pair<double, double>> excluded_ranges;
  std::vector<Boomwhacker*> whackers; // The boomwhackers that the player has
  std::vector<Note*> notes; // The notes that the player has played

  Player(int id, int hold_limit, double switch_time, bool one_handed_roll,
    std::vector<std::pair<double, double>>& excluded_ranges);

  bool excluded(Note* note);
  std::vector<Note*>::iterator find_closest_before(Note* target);
  Note* roll_conflict(Note* note, std::vector<Note*>::iterator before);
  std::optional<std::vector<Run*>> run_conflicts(Note* note, std::vector<Note*>::iterator start);
  std::optional<std::vector<Run*>> conflicts2(Note* note);


  bool conflicts(Note* before, Note* after);
  std::vector<Note*> conflicts_back(std::vector<Note*>::iterator end, Note* note);
  std::vector<Note*> conflicts_front(std::vector<Note*>::iterator end, Note* note);
  void add_whacker(Boomwhacker* whacker);
  void add_note(Note* note);
  void show_whackers();
  std::vector<Boomwhacker*>::iterator get_whacker(int pitch);
};