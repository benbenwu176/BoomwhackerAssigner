#include "gen.hpp"
#include "boomwhacker.hpp"
#include <cassert>

Boomwhacker::Boomwhacker(int pitch, int id) {
  this->pitch = pitch;
  this->id = id;
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
  assert(note->pitch == get_real_pitch());
  notes.push_back(note);
}

int Boomwhacker::get_real_pitch() {
  if (capped) {
    return pitch - OCTAVE_INTERVAL;
  } else {
    return pitch;
  }
}