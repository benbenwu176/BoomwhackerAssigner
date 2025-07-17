#include "gen.hpp"
#include "player.hpp"

/**
 * @brief Bucket class constructor
 */
Bucket::Bucket(int hold_limit, double switch_time) {
  capacity = hold_limit + 1; // Default bucket size = 2 + 1 = 3
  this->switch_time = switch_time;
  data = std::vector<Note*>();
}

/**
 * @brief Checks if a bucket has a timing conflict
 * 
 * A timing conflict occurs if the time difference between the notes at the front and back is less than switch_time.
 * This represents a player not being able to switch notes in time.
 * 
 * @return True if a timing conflict occurs, false otherwise
*/
bool Bucket::bucket_conflict() {
  if (data.size() < capacity) {
    // Bucket has space, no conflict
    return false;
  } else {
    double front_time = data.front()->time;
    double back_time = data.back()->time;
    return (back_time - front_time) < switch_time;
  }
}

/**
 * @brief Attempts to add a note to this bucket
 * 
 * A bucket maintains an ordered set of notes with unique pitches that 
 * this player has most recently played. The set is ordered in increasing chronological
 * order of when the note was played.
 * 
 * If the bucket has a note with a duplicate pitch to the note to be added, remove the old note and add
 * the new note to the end.
 * Once the note is added, check if the bucket is full. 
 * If the bucket is full, find the difference between the time values of the notes at index 2 and 0. 
 * If the difference between them is less than SECOND_DELTA, then return 1. Otherwise, return 0.
 * 
 * @param note The note to add
 * @param force Whether to force a conflict if necessary
 * 
 * @return True if no conflict, false otherwise
*/
bool Bucket::try_add(Note* note) {
  std::vector<Note*> old(data);
  int dupe_index = -1;
  for (int i = 0; i < data.size(); i++) {
    if (data[i]->pitch == note->pitch) {
      dupe_index = i;
      break;
    }
  }

  /* Remove a note if bucket is full already
  If dupe, remove dupe
  If no dupe found and bucket has space, don't remove
  If no dupe found and bucket full, remove data[0] */
  if (dupe_index != -1) {
    data.erase(data.begin() + dupe_index);
  } else {
    if (data.size() >= capacity) {
      data.erase(data.begin());
    }
  }
  /*
  If note causes a conflict but add isn't forced, rollback and return conflicting (false)
  If note causes a conflict but add is forced, add and return conflicting (false)
  If note does not cause a conflict, add and return not conflicting (true)
  */
  data.push_back(note);
  note->conflicting = bucket_conflict();
  
  if (note->conflicting) {
    data = old;
    note->conflicting = false;
    return false;   
  } else {
    return true;
  }
}

/**
 * @brief Player class constructor
 */
Player::Player(int id, int hold_limit, double switch_time) {
  this->id = id;
  this->hold_limit = hold_limit;
  this->switch_time = switch_time;
  whackers = std::vector<Boomwhacker *>();
  notes = std::vector<Note *>();
  bucket = new Bucket(hold_limit, switch_time);
}