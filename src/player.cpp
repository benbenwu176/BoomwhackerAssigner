#include "gen.hpp"
#include "player.hpp"

/**
 * @brief Player class constructor
 */
Player::Player(int id, int hold_limit, double switch_time, bool one_handed_roll, 
    std::vector<std::pair<double, double>>& excluded_ranges) {
  this->id = id;
  this->hold_limit = hold_limit;
  this->switch_time = switch_time;
  this->one_handed_roll = one_handed_roll;
  this->excluded_ranges = excluded_ranges;
  whackers = std::vector<Boomwhacker *>();
  notes = std::vector<Note *>();
}

template<typename T>
typename std::vector<T*>::iterator
find_by_real_pitch(std::vector<T*>& vec, int target_pitch) {
  auto it = std::lower_bound(vec.begin(), vec.end(), target_pitch, [](T* obj, int val) {
    return obj->get_real_pitch() < val;
  });

  // Check for an exact match
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

// Check if note lies within the player's excluded time ranges (exclusive)
bool Player::excluded(Note* note) {
  for (auto range : excluded_ranges) {
    if (range.first < note->time && note->time < range.second) {
      return true;
    }
  }
  return false;
}

bool comp_time(const Note* a, const Note* b) {
  return a->time < b->time;
}

// Find note in note array at or before the given note
std::vector<Note*>::iterator Player::find_closest_before(Note* target) {
  // Find note at or before target
  auto it = std::upper_bound(notes.begin(), notes.end(), target, comp_time);

  if (it == notes.begin()) {
    return it;
  } else {
    return --it;
  }
}

// Check if this player has a roll conflict with this note
Note* Player::roll_conflict(Note* note, std::vector<Note*>::iterator bef) {
  // No roll conflict if player can roll with one hand
  if (one_handed_roll) {
    return;
  }

  // Check if note is within before's roll duration
  Note* before = *bef;
  std::vector<Note*>::iterator aft;
  if (before->time <= note->time) {
    aft = std::next(bef);
    if (before->duration > 0 && 
        before->time <= note->time && note->time <= before->time + before->duration) {
      return before;
    }
  } else {
    aft = bef;
  }

  // Check if after is within note's roll duration
  if (aft != notes.end()) {
    Note* after = *aft;
    if (note->duration > 0 &&
        note->time <= after->time && after->time <= note->time + note->duration) {
      return after;
    }
  }

  // No roll conflict
  return nullptr;
}

// Check if this player has any run conflicts with the note
std::optional<std::vector<Run*>> Player::run_conflicts(Note* insert, std::vector<Note*>::iterator start) {
  const double low  = insert->time - switch_time;
  const double high = insert->time + switch_time;

  // Create a subarray of notes in range [low, high]
  auto L_it = std::lower_bound(notes.begin(), notes.end(), low, 
                               [](const Note* n, double t){return n->time < t;});
  auto R_it = std::upper_bound(L_it, notes.end(), high,
                               [](double t, const Note* n){return t < n->time;});
  
  const size_t L = static_cast<size_t>(L_it - notes.begin());
  const size_t R = static_cast<size_t>(R_it - notes.begin());
  if (L == R) return std::nullopt;

  // Two-pointer sweep on [L, R)
  std::unordered_map<int, int> freq; // pitch -> count in window
  freq.reserve(32);
  int unique_pitches = 0;

  size_t i = L;
  size_t j = L;

  // Helpers
  auto add = [&](int pitch) {
    int &c = freq[pitch];
    if (c++ == 0) ++unique_pitches;
  };

  auto remove = [&](int pitch) {
    auto it = freq.find(pitch);
    if (it != freq.end() && --(it->second) == 0) {
      freq.erase(it);
      --unique_pitches;
    }
  };

  for (; i < R && notes[i]->time <= insert->time; ++i) {

    while (j < R && notes[j]->time <= notes[i]->time + switch_time) {
      add(notes[j]->pitch);
      ++j;
    }

    const bool insert_pitch_present = (freq.find(insert->pitch) != freq.end());
    const int unique_with_insert = unique_pitches + (insert_pitch_present ? 0 : 1);

    if (unique_with_insert > hold_limit) {
      // Conflict! Collect runs from notes in [i, j)

      std::unordered_map<int, std::vector<Note*>> by_pitch;
      by_pitch.reserve(freq.size());

      for (size_t k = i; k < j && notes[k]->time <= insert->time; ++k) {
        by_pitch[notes[k]->pitch].push_back(notes[k]);
      }
    }
  }
}

/*
 * Returns if the player can play this note.
 * nullopt if successful, a list of conflicting notes before
 * the note time otherwise
 */
std::optional<std::vector<Run*>> Player::conflicts2(Note* note) {
  std::vector<Run*> conflicts;

  // Check if within excluded ranges
  if (excluded(note)) {
    return conflicts;
  }

  // Get the note closest at or before this note in the player array
  std::vector<Note*>::iterator before = find_closest_before(note);

  // Check if player has a roll conflict with this note
  Note* rcon = roll_conflict(note, before);
  if (rcon) {
    if (rcon->time <= note->time) {
      conflicts.push_back(new Run(rcon));
    }
    return conflicts;
  }
  
  // Find start of relevant notes
  auto it = before;
  while (it != notes.begin() && (*it)->time >= (note->time - switch_time)) {
    --it;
  }
  auto start = it;
  if (it != notes.begin()) {
    ++start;
  }

  // Find run conflicts
  return run_conflicts(note, start);
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

  // Insert the initial element first
  Key init_key = proj(initial);
  seen.insert(init_key);
  result.push_back(initial);

  // Walk backwards through [first, last), stopping when
  // n total elements have been collected or the end has been reached.
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
bool Player::conflicts(Note* before, Note* after) {
  // assert(after->time >= before->time);
  double delta = after->time - before->time;
  return delta < switch_time;
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
    if (conflicts(bucket.back(), bucket.front())) {
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

  // Seed with the initial element:
  Key init_key = proj(initial);
  seen.insert(init_key);
  result.push_back(initial);
  
  // Walk forward through [first, last), stopping when 
  // n total elements have been collected or the end has been reached.
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
    if (conflicts(bucket.front(), bucket.back())) {
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