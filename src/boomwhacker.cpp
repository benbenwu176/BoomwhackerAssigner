#include "gen.hpp"
#include "boomwhacker.hpp"

Boomwhacker::Boomwhacker(int pitch) {
  this->pitch = pitch;
  used = false;
  capped = false;
}

void Boomwhacker::cap() {
  this->capped = true;
}

void Boomwhacker::uncap() {
  this->capped = false;
}

void Boomwhacker::alloc() {
  used = true;
}

void Boomwhacker::dealloc() {
  used = false;
  capped = false;
}

void Boomwhacker::add_note(Note* note) {
  note->whacker_idx = notes.size();
  notes.push_back(note);
}

int Boomwhacker::get_real_pitch() {
  if (capped) {
    return pitch - OCTAVE_INTERVAL;
  } else {
    return pitch;
  }
}