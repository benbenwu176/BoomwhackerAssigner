#include "gen.hpp"
#include "player.hpp"

/**
 * @brief Player class constructor
 */
Player::Player(int id, int hold_limit, double switch_time) {
  this->id = id;
  this->hold_limit = hold_limit;
  this->switch_time = switch_time;
  whackers = std::vector<Boomwhacker *>();
  notes = std::vector<Note *>();
}

/* 
 * Helper method to obtain an ordered set of notes based on their pitch
 */
template<typename Iter, typename Proj>
std::vector<typename std::iterator_traits<Iter>::value_type>
last_n_unique_by(Iter first, Iter last, std::size_t n,
                 typename std::iterator_traits<Iter>::value_type initial, Proj proj) {
  using T   = typename std::iterator_traits<Iter>::value_type;
  using Key = std::invoke_result_t<Proj, T>;

  std::vector<T>          result;
  std::unordered_set<Key> seen;

  // 1) Insert the initial element first:
  Key init_key = proj(initial);
  seen.insert(init_key);
  result.push_back(initial);

  // 2) Now walk backwards through [first, last),
  //    stopping when we've collected n total elements:
  auto rbegin = std::make_reverse_iterator(last);
  auto rend   = std::make_reverse_iterator(first);

  for (auto rit = rbegin; rit != rend && result.size() < n; ++rit) {
    T elem = *rit;
    Key key = proj(elem);
    if (seen.insert(key).second) { 
        result.push_back(elem);
    }
  }

  return result;
}

/**
 * @brief Checks if a note is conflicting with the player at a given point in time
 * 
 * @return A list of conflicting notes, excluding the note to be added. Returns an empty vector if 
 * no conflicts occurred.
 */
std::vector<Note*> Player::conflicts_back(std::vector<Note*>::iterator end, Note* note) {
  // Create ordered set of notes by pitch
  auto bucket = last_n_unique_by(notes.begin(), end, hold_limit + 1, note,
    [](Note* n){return n->pitch;});
  
  // Bucket size = hold capacity (# of hands), no confict
  if (bucket.size() <= hold_limit) {
    bucket.clear();
  } else {
    // Bucket full, check for timing conflict
    double latest_time = bucket.front()->time;
    double earliest_time = bucket.back()->time;
    double delta = std::abs(latest_time - earliest_time);
    if (delta < switch_time) {
      // Conflict between played note and last note, return list of conflicting notes (excluding the note that was attempted to be added)
      bucket.erase(bucket.begin());
    } else {
      // Time to switch, no conflict
      bucket.clear();
    }
  }
  return bucket;

}

template<typename Iter, typename Proj>
std::vector<typename std::iterator_traits<Iter>::value_type>
first_n_unique_by(Iter first, Iter last, std::size_t n, 
                  typename std::iterator_traits<Iter>::value_type initial, Proj proj) {
  using T   = typename std::iterator_traits<Iter>::value_type;
  using Key = std::invoke_result_t<Proj, T>;

  std::vector<T>          result;
  std::unordered_set<Key> seen;

  // 1) Seed with the initial element:
  Key init_key = proj(initial);
  seen.insert(init_key);
  result.push_back(initial);
  // 2) Walk forward through [first, last),
  //    stopping when we've collected n total elements:
  for (auto it = first; it != last && result.size() < n; ++it) {
      T const& elem = *it;
      Key key = proj(elem);
      if (seen.insert(key).second) {
          result.push_back(elem);
      }
  }

  return result;
}

/**
 * @brief Checks if a note is conflicting with the player at a given point in time
 * 
 * @return A list of conflicting notes, excluding the note to be added. Returns an empty vector if 
 * no conflicts occurred.
 */
std::vector<Note*> Player::conflicts_front(std::vector<Note*>::iterator start, Note* note) {
  // Create ordered set of notes by pitch
  auto bucket = first_n_unique_by(start, notes.end(), hold_limit + 1, note,
    [](Note* n){return n->pitch;});

  // Bucket size = hold capacity (# of hands), no confict
  if (bucket.size() <= hold_limit) {
    bucket.clear();
  } else {
    // Bucket full, check for timing conflict
    double latest_time = bucket.front()->time;
    double earliest_time = bucket.back()->time;
    double delta = std::abs(latest_time - earliest_time);
    if (delta < switch_time) {
      // Conflict between played note and last note, return list of conflicting notes (excluding the note that was attempted to be added)
      bucket.erase(bucket.begin());
    } else {
      // Time to switch, no conflict
      bucket.clear();
    }
  }
  return bucket;

}

bool comp_pitch(Boomwhacker* a, Boomwhacker* b) {
  return a->get_real_pitch() < b->get_real_pitch();
}

void Player::show_whackers() {
  for (Boomwhacker* whacker : whackers) {
    log(whacker->get_real_pitch());
  }
}

void Player::add_whacker(Boomwhacker* whacker) {
  auto it = std::lower_bound(whackers.begin(), whackers.end(), whacker, comp_pitch);
  whackers.insert(it, whacker);
}

void Player::add_note(Note* note) {
  notes.push_back(note);
}

template<typename T>
typename std::vector<T*>::iterator
find_by_real_pitch(std::vector<T*>& vec, int target_pitch) {
    auto it = std::lower_bound(vec.begin(), vec.end(), target_pitch, [](T* obj, int val) {
        return obj->get_real_pitch() < val;
      }
    );

  // check for an exact match
  if (it != vec.end() && (*it)->get_real_pitch() == target_pitch) {
    return it;
  }
  return vec.end();
}

// Finds a whacker in this player's inventory that matches the given note's pitch
std::vector<Boomwhacker*>::iterator Player::get_whacker(int pitch) {
  auto it = find_by_real_pitch(whackers, pitch);
  if (it == whackers.end()) {
    std::string s = "Player " + std::to_string(id) + " lacks corresponding whacker of pitch: " + std::to_string(pitch);
    throw std::runtime_error(s);
  }

  return it;
}