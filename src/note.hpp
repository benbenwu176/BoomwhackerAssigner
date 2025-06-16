#pragma once

typedef struct Note {
  double time;          // The time, in seconds, the note occurs at in the piece
  int id;               // Index of the note within the note array
  int player;           // The player that this note is assigned to
  int pitch;            // MIDI pitch value
  int whacker_index;     // The index of the boomwhacker that is used for this note
  bool capped;          // Whether the note is played with a capped boomwhacker
  bool proximate;       // Whether the note is proximate
  bool conflicting;     // Whether the note creates a switch conflict
  
  Note(int id, int pitch, double time);
} Note;

inline Note::Note(int id, int pitch, double time) {
  this->time = time;
  this->id = id;
  player = -1;
  this->pitch = pitch;
  whacker_index = -1;
  capped = false;
  proximate = false;
  conflicting = false;
}