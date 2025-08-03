#pragma once

class Boomwhacker;
class Player;

typedef struct Note {
  double time;          // The time, in seconds, the note occurs at in the piece
  Boomwhacker* whacker; // The boomwhacker that is used for this note
  Player* player;       // The player (ptr) that plays this note
  int notes_idx;        // Index of the note within the note array
  int whacker_idx;      // Index of where the note is in the whacker's note array
  int player_idx;       // Index of where the note is in the player's note array
  int pitch;            // MIDI pitch value
  bool capped;          // Whether the note is played with a capped boomwhacker
  bool proximate;       // Whether the note is proximate
  bool conflicting;     // Whether the note creates a switch conflict
  
  Note(int id, int pitch, double time);
} Note;

inline Note::Note(int id, int pitch, double time) {
  this->time = time;
  whacker = nullptr;
  player = nullptr;
  this->notes_idx = id;
  this->whacker_idx = -1;
  this->player_idx = -1;
  this->pitch = pitch;
  capped = false;
  proximate = false;
  conflicting = false;
}