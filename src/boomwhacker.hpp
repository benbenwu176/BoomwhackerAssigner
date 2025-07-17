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
  int get_real_pitch();
};

inline Boomwhacker::Boomwhacker(int pitch) {
  this->pitch = pitch;
  used = false;
  capped = false;
}

inline int Boomwhacker::get_real_pitch() {
  if (capped) {
    return pitch - OCTAVE_INTERVAL;
  } else {
    return pitch;
  }
}