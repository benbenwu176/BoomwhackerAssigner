#pragma once

#include "note.hpp"
#include "boomwhacker.hpp"

class Bucket {
public:
  int capacity;
  double switch_time;
  std::vector<Note*> data;

  Bucket(int hold_limit, double switch_time);
  bool try_add(Note* note, bool force);
  bool bucket_conflict();
};

class Player {
public:
  int id;
  int hold_limit;
  double switch_time;
  std::vector<Boomwhacker*> whackers; // The boomwhackers that the player has
  std::vector<Note*> notes; // The notes that the player has played
  Bucket* bucket; // The player's bucket of notes. Refer to documentation for details.

  Player(int id, int hold_limit, double switch_time);
};