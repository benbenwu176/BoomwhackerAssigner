#pragma once

class Boomwhacker;

typedef struct Note {
  double time;          // The time, in seconds, the note occurs at in the piece
  Boomwhacker* whacker; // The index of the boomwhacker that is used for this note
  int id;               // Index of the note within the note array
  int player;           // The player that this note is assigned to
  int pitch;            // MIDI pitch value
  bool capped;          // Whether the note is played with a capped boomwhacker
  bool proximate;       // Whether the note is proximate
  bool conflicting;     // Whether the note creates a switch conflict
  
  Note(int id, int pitch, double time);
} Note;

inline Note::Note(int id, int pitch, double time) {
  this->time = time;
  whacker = nullptr;
  this->id = id;
  player = -1;
  this->pitch = pitch;
  capped = false;
  proximate = false;
  conflicting = false;
}