#pragma once

#include "note.hpp"
#include "globals.hpp"

class Boomwhacker {
public:
  int pitch; // The pitch of the boomwhacker
  bool used; // Whether the boomwhacker has been used
  bool capped; // Whether the boomwhacker is capped
  std::vector<Note*> notes; // The notes that are played by this boomwhacker

  Boomwhacker(int pitch);
  void cap();
  void uncap();
  void alloc();
  void dealloc();
  void add_note(Note* note);
  int get_real_pitch();
};