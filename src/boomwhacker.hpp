#pragma once

#include "note.hpp"

class Boomwhacker {
public:
  int pitch; // The pitch of the boomwhacker
  bool used; // Whether the boomwhacker has been used
  bool capped; // Whether the boomwhacker is capped
  std::vector<Note*> notes; // The notes that are played by this boomwhacker

  Boomwhacker(int pitch);
};

inline Boomwhacker::Boomwhacker(int pitch) {
  this->pitch = pitch;
  used = false;
  capped = false;
}