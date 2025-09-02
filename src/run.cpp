#include "run.hpp"

Run::Run(int pitch) {
  this->pitch = pitch;
}

Run::Run(Note* seed) {
  this->pitch = seed->pitch;
  data.push_back(seed);
}

int Run::get_pitch() {
  return pitch;
}

Note* Run::start() {
  return data.front();
}

Note* Run::end() {
  return data.back();
}

void Run::push(Note* note) {
  data.push_back(note);
}

void Run::queue(Note* note) {
  data.insert(data.begin(), note);
}