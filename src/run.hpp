#pragma once

#include "note.hpp"
#include "boomwhacker.hpp"
#include "globals.hpp"
#include <vector>

class Run {
public:
  int pitch;
  std::vector<Note*> data;

  Run(int pitch);
  Run(Note* seed);
  int get_pitch();
  Note* start();
  Note* end();
  void push(Note* note);
  void queue(Note* note);
};