#include "gen.hpp"
#include "player.hpp"

Player::Player(int id, int hold_limit, double switch_time) {
  this->id = id;
  this->hold_limit = hold_limit;
  this->switch_time = switch_time;
  whackers = std::vector<Boomwhacker *>();
  notes = std::vector<Note *>();
  bucket = std::vector<Note *>();
}

/**
 * @brief Attempts to add a note to a player's bucket. Returns the inverse of the maximum time differential
 * between notes. Successfully adds if the note is able to be played within the player's switch time.
 *
 */
double Player::attempt_add(Note *note, bool force) {
  int dupe_index = -1;
  for (int i = 0; i < bucket.size(); i++) {
    if (bucket[i]->pitch == note->pitch && bucket[i]->capped == note->capped) {
      dupe_index = i;
      break;
    }
  }
  return 0;
}

/**
 * @brief Adds a note to this player?
 */
bool Player::add_note(Note *note, int mode) {
  if (note->pitch >= C2_MIDI) {
    for (int i = 0; i < assignment->mrp[note->pitch].size(); i++)
    {
      if (assignment->mrp[note->pitch][i] != nullptr)
      {
        // Add the note to the player's bucket
      }
    }
  }

  return true;
}

/**
 * @brief Forcibly adds a note to this player's list of notes.
 */
double Player::force_add(Note* note) {

}

/**
 * @brief Add a note to this player's bucket. Returns if the added note results in a conflict.
 * 
 */
bool Player::bucket_add(Note* note) {

}