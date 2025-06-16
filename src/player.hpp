#pragma once

#include "note.hpp"
#include "boomwhacker.hpp"

class Player {
public:
  int id;
  int hold_limit;
  double switch_time;
  std::vector<Boomwhacker*> whackers; // The boomwhackers that the player has
  std::vector<Note*> notes; // The notes that the player has played
  std::vector<Note*> bucket; // The player's bucket of notes. Refer to documentation for details.

  Player(int id, int hold_limit, double switch_time);
  bool add_note(Note *note, int mode);
  double attempt_add(Note* note, bool force);
  double force_add(Note* note);
  bool bucket_add(Note* note);
};