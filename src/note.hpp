#pragma once

class Boomwhacker;
class Player;

typedef struct Note {
  double time;          // The time, in seconds, the note occurs at in the piece
  double duration;      // How long this note lasts (only set if rolled)
  Boomwhacker* whacker; // The boomwhacker that is used for this note
  Player* player;       // The player (ptr) that plays this note
  int id;               // Index of the note within the note array
  int pitch;            // MIDI pitch value
  bool capped;          // Whether the note is played with a capped boomwhacker
  bool proximate;       // Whether the note is proximate
  bool conflicting;     // Whether the note creates a switch conflict
  
  Note(int id, int pitch, double time, double duration);
} Note;

inline Note::Note(int id, int pitch, double time, double duration) {
  this->time = time;
  this->duration = duration;
  whacker = nullptr;
  player = nullptr;
  this->id = id;
  this->pitch = pitch;
  capped = false;
  proximate = false;
  conflicting = false;
}