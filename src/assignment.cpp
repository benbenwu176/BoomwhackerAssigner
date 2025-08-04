#include "gen.hpp"
#include "assignment.hpp"
#include <thread>
#include <io.h>
#include <fcntl.h>

template<typename T>
typename std::vector<T*>::iterator
find_by_id(std::vector<T*>& vec, int targetId) {
    auto it = std::lower_bound(
        vec.begin(), vec.end(),       // search range
        targetId,                      // value to compare against
        [](T* obj, int val) {          // comparator: obj->id < val
          return obj->id < val;
        }
    );

    // check for an exact match
    if (it != vec.end() && (*it)->id == targetId) {
        return it;
    }
    throw std::runtime_error("Note not found in list.");
}

/**
 * @brief Creates an Assignment object with no assigned notes.
 */
Assignment::Assignment(std::vector<int> &pitches, std::vector<double> &times) {
  init_notes(pitches, times);
  init_players();
  init_whackers();
  adjacency_graph = new Graph();
}

/**
 * @brief Initialize the array of notes representing the piece.
 */
void Assignment::init_notes(std::vector<int> &pitches, std::vector<double> &times) {
  notes.reserve(cfg->num_notes);
  for (int i = 0; i < cfg->num_notes; i++) {
    notes.emplace_back(i, pitches[i], times[i]);
  }
}

/**
 * @brief Initialize an array of players.
 */
void Assignment::init_players() {
  players.reserve(cfg->num_players);
  for (int i = 0; i < cfg->num_players; i++) {
    players.push_back(new Player(i, cfg->hold_limits[i], cfg->switch_times[i]));
  }
}

/**
 * @brief Initialize the boomwhacker table.
 */
void Assignment::init_whackers() {
  whacker_table = std::vector<std::vector<Boomwhacker *>>(NUM_WHACKER_PITCHES);
  for (int i = 0; i < NUM_WHACKER_PITCHES; i++) {
    std::vector<Boomwhacker*>& whackers = whacker_table[i];
    int whacker_quantity = cfg->whacker_quantities[i];
    for (int j = 0; j < whacker_quantity; j++) {
      int pitch = i + C3_MIDI;
      whackers.push_back(new Boomwhacker(pitch, j));
    }
  }
}

/**
 * @brief Initialize the player adjacency graph.
 */
void Assignment::init_graph() {
  adjacency_graph = new Graph();
}

/**
 * @brief Writes the player assignments to STDOUT
 */
void Assignment::write()
{
  // Print # of conflicts in the final assignment
  int num_conflicts = 0;
  for (int i = 0; i < cfg->num_notes; i++) {
    if (notes[i].conflicting) {
      num_conflicts++;
    }
  }
  log_line(); log("Number of conflicts:", num_conflicts, "\n"); log_line();

  // Print whacker data
  for (int i = 0; i < whacker_table.size(); i++) {
    log_line();
    log("Whacker", i);
    for (int j = 0; j < cfg->whacker_quantities[i]; j++) {
      log(whacker_table[i][j]->used);
    }
  }
  std::string scale[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  std::string octaves[4] = {"Low", "Low", "Mid", "High"};
  for (int i = 0; i < players.size(); i++) {
    Player* player = players[i];
    log_line();
    log("Player", i);
    for (int j = 0; j < players[i]->whackers.size(); j++) {
      Boomwhacker* whacker = player->whackers[j];
      int pitch = whacker->pitch;
      std::cout << (whacker->capped ? "*" : "") << octaves[(pitch - C2_MIDI) / 12] << " " << scale[pitch % 12] << " ";
    }
  }
  log_line();

  // Write relevant note fields to binary
  std::filesystem::path recolor_data_path = cfg->tmp_dir / "recolor_data.bin";
  std::ofstream ofs(recolor_data_path, std::ios::binary | std::ios::trunc);
  for (int i = 0; i < notes.size(); i++) {
    Note note = notes[i];
    int player_id = note.player != nullptr ? note.player->id : -1;
    ofs.write(reinterpret_cast<const char*>(&note.time), sizeof(double));
    ofs.write(reinterpret_cast<const char*>(&player_id), sizeof(int));
    ofs.write(reinterpret_cast<const char*>(&note.capped), sizeof(bool));
    ofs.write(reinterpret_cast<const char*>(&note.conflicting), sizeof(bool));
  }
  std::cout.flush();

  // TODO: write entire assignment struct
}

// Find and return all used whackers of a given pitch
std::vector<Boomwhacker*> Assignment::find_used_whackers(int pitch) {
  std::vector<Boomwhacker*> ret;
  int table_index = pitch - C3_MIDI;
  int table_index_capped = table_index + OCTAVE_INTERVAL;

  // Find all natural used whackers
  if (table_index >= 0 && table_index < NUM_WHACKER_PITCHES) {
    std::vector<Boomwhacker*> uncapped_whackers = whacker_table[table_index];
    for (auto& w : uncapped_whackers) {
      if (w->used && w->get_real_pitch() == pitch) {
        ret.push_back(w);
      }
    }
  }

  // Find all capped used whackers
  if (table_index_capped >= 0 && table_index_capped < NUM_WHACKER_PITCHES) {
    std::vector<Boomwhacker*> capped_whackers = whacker_table[table_index_capped];
    for (auto&w : capped_whackers) {
      if (w->used && w->get_real_pitch() == pitch) {
        ret.push_back(w);
      }
    }
  }
  return ret;
}

// Find note in note array at or before the given time
Note* find_closest_before(const std::vector<Note*> notes, double target) {
  auto comp = [](double value, const Note* note) {
    return value < note->time;
  };
  auto it = std::upper_bound(notes.begin(), notes.end(), target, comp);
  if (it == notes.begin()) {
    return nullptr;
  } else {
    --it;
    return *it;
  }
}

// Find note in note array at or after the given time
Note* find_closest_after(const std::vector<Note*> notes, double target) {
  auto comp = [](const Note* note, double value) {
    return value > note->time;
  };
  auto it = std::lower_bound(notes.begin(), notes.end(), target, comp);
  if (it == notes.end()) {
    return nullptr;
  } else {
    return *it;
  }
}

// Returns a list of the players who most recently played this note pitch at or before a given time.
std::vector<Note*> Assignment::get_mrp_queue(int pitch, double time) {
  // Conjure up a list of used whackers at pitch
  std::vector<Boomwhacker*> used_whackers = find_used_whackers(pitch);

  // Binary search whacker note list by time to find the nearest notes
  std::vector<Note*> ret;
  for (auto& whacker : used_whackers) {
    std::vector<Note*> whacker_notes = whacker->notes;
    Note* nearest = find_closest_before(whacker_notes, time);
    if (nearest != nullptr) {
      // Insert to return vector
      int i = 0;
      while (i < ret.size() && nearest->time < ret[i]->time) {
        i++;
      }
      ret.insert(ret.begin() + i, nearest);
    }
  }

  // Return list of most recently played notes in increasing order of closeness to target time
  return ret;
}

// Whacker search cond
bool unused(Boomwhacker* w) {
  return !w->used;
}

/**
 * @brief Attempts to find an available boomwhacker that can play this pitch
 * 
 * Boomwhacker allocation policy:
 * Allocate whichever boomwhacker has more available. 
 */
Boomwhacker* Assignment::find_whacker(int pitch) {
  int table_index = pitch - C3_MIDI;
  int table_index_capped = table_index + OCTAVE_INTERVAL;
  std::vector<Boomwhacker*> uncapped_whackers;
  std::vector<Boomwhacker*> capped_whackers;
  std::vector<Boomwhacker*>::iterator uncapped_whacker;
  std::vector<Boomwhacker*>::iterator capped_whacker;
  bool uncapped_found = false;
  bool capped_found = false;
  
  // Try to find boomwhacker of unaltered pitch
  if (table_index >= 0 && table_index < NUM_WHACKER_PITCHES) {
    uncapped_whackers = whacker_table[table_index];
    uncapped_whacker = std::find_if(uncapped_whackers.begin(), uncapped_whackers.end(), unused);
    uncapped_found = uncapped_whacker != uncapped_whackers.end();
  }
  // Try to find boomwhacker that can be capped
  if (table_index_capped >= 0 && table_index_capped < NUM_WHACKER_PITCHES) {
    capped_whackers = whacker_table[table_index_capped];
    capped_whacker = std::find_if(capped_whackers.begin(), capped_whackers.end(), unused);
    capped_found = capped_whacker != capped_whackers.end();
  }

  if (uncapped_found && capped_found) {
    // Uncapped and capped options available, calculate remaining whackers for both options
    int uncapped_index = uncapped_whacker - uncapped_whackers.begin();
    int capped_index = capped_whacker - capped_whackers.begin();
    int remaining_uncapped = cfg->whacker_quantities[table_index] - (uncapped_index + 1);
    int remaining_capped = cfg->whacker_quantities[table_index_capped] - (capped_index + 1);

    // Pick whichever has more whackers left
    if (remaining_uncapped >= remaining_capped) {
      return *uncapped_whacker;
    } else {
      (*capped_whacker)->cap();
      return *capped_whacker;
    }
  } else if (uncapped_found) {
    // Return the only found whacker (uncapped)
    return *uncapped_whacker;
  } else if (capped_found) {
    // Return the only found whacker (capped)
    (*capped_whacker)->cap();
    return *capped_whacker;
  } else {
    // No whacker found
    return nullptr;
  }

}

/**
 * @brief Attempts to locate a player within the MRP that
 * can play this note without causing conflict.
 * 
 * @return A list of all conflicting notes to be used for offloading
 */
std::optional<std::vector<Note*>> Assignment::add_existing(Note* note) {
  std::vector<Note*> queue = get_mrp_queue(note->pitch, note->time);
  std::vector<Note*> all_conflicts;
  for (int i = 0; i < queue.size(); i++) {
    // Search for non-conflicting player in the MRP to add this to
    Player* player = queue[i]->player;
    std::vector<Note*> conflicts = player->conflicts_back(player->notes.end(), note);
    // Assign to first player with no conflicts
    if (conflicts.empty()) {
      // Assign note
      assign_note(note, *(player->get_whacker(note->pitch)), player, false);
      log("MRP");

      // Return successful add (nullopt)
      return std::nullopt;
    } else {
      for (Note* conflict : conflicts) {
        if (conflict->player != player) {
          log_line();
          log(conflict->id, conflict->player->id, conflict->whacker->id, conflict->whacker->get_real_pitch());
          for (Player* p : players) {
            log_line();
            log("Player", p->id);
            p->show_whackers();
          }
          throw std::runtime_error("Player mismatch.");
        }
      }
      all_conflicts.insert(all_conflicts.end(), conflicts.begin(), conflicts.end());
    }
  }
  // Return all notes that conflict
  return all_conflicts;
}

bool comp_time(Note* note, double target) {
  return target < note->time;
}

bool comp_id(Note* a, Note* b) {
  return a->id < b->id;
}

/*
 * Attempt to offload to other players that have this note
 *
 * Tries a breadth-first search of the options at the first level,
 * then resorts to depth-first if the previous attempts failed.
 * 
 */
int Assignment::add_offload(Option* opt, add_flags flags) {
  Note* note = opt->note;
  const std::vector<Note*>& conflicts = opt->conflicts;
  std::vector<Option*> options;
  options.reserve(conflicts.size());

  // Try to find a conflicting note that can be offloaded
  for (int i = 0; i < conflicts.size(); i++) {
    Note* con = conflicts[i];
    Player* con_old_player = con->player;
    options.push_back(new Option(con));
    // Look for other players that can play the conflicting note
    std::vector<Note*> others = get_mrp_queue(con->pitch, con->time);
    for (Note* other : others) {
      Player* other_player = other->player;
      if (other_player == con->player) {
        // Prevent from checking the same note
        continue;
      }

      // Offload the longest non-conflicting run of notes
      int num_offloaded = 0;
      std::vector<Note*>& con_whacker_notes = con->whacker->notes;
      std::vector<Note*>& con_player_notes = con->player->notes;
      auto con_whacker_loc = find_by_id(con_whacker_notes, con->id);
      int con_whacker_idx = con_whacker_loc - con_whacker_notes.begin();
      for (int j = con_whacker_idx; j >= 0; j--) {
        Note* cur = con_whacker_notes[j];
        std::vector<Note*> conflicts;

        // Find conflicts before
        Note* to_check = find_closest_before(other_player->notes, cur->time);
        assert(to_check != nullptr);
        std::vector<Note*>::iterator player_loc = find_by_id(other_player->notes, to_check->id);
        std::vector<Note*>::iterator insert_loc = player_loc;
        std::vector<Note*> back_conflicts = other_player->conflicts_back(player_loc + 1, cur);
        conflicts.insert(conflicts.end(), back_conflicts.begin(), back_conflicts.end());

        // Find conflicts after
        to_check = find_closest_after(other_player->notes, cur->time);
        if (to_check != nullptr) {
          std::vector<Note*>::iterator player_loc = find_by_id(other_player->notes, to_check->id);
          std::vector<Note*> front_conflicts = other_player->conflicts_front(player_loc, cur);
          conflicts.insert(conflicts.end(), front_conflicts.begin(), front_conflicts.end());
        }
        
        if (conflicts.empty()) {
          // Remove cur from conflicting player and add to new player
          con_whacker_notes.erase(con_whacker_notes.begin() + j);
          auto cur_notes_it = find_by_id(con_player_notes, cur->id);
          con_player_notes.erase(cur_notes_it);

          // Tiebreaker for notes that have the same time but different pitches
          if (insert_loc != other_player->notes.begin()) {
            if ((*insert_loc)->time == cur->time) {
              if ((*insert_loc)->pitch < cur->pitch) {
                ++insert_loc;
              }
            } else {
              ++insert_loc;
            }
          }
          other_player->notes.insert(insert_loc, cur);
          
          std::vector<Note*>& other_whacker_notes = other->whacker->notes;
          auto it = std::lower_bound(other_whacker_notes.begin(), other_whacker_notes.end(), cur, comp_id);
          other_whacker_notes.insert(it, cur);

          cur->player = other_player;
          cur->whacker = other->whacker;
          num_offloaded++;
          log("Offloaded", cur->id, "Player", other_player->id);
        } else {
          // Add conflicts to corresponding option
          options[i]->conflicts.insert(options[i]->conflicts.end(), back_conflicts.begin(), back_conflicts.end());
          break;
        }
      }
      if (num_offloaded > 0) {
        auto it = con_old_player->get_whacker(note->pitch);
        if (con_whacker_notes.empty()) {
          // Remove whacker from old con player
          log("Dealloc from player", con_old_player->id);
          con_old_player->whackers.erase(it);
        }

        // Add note to new now non-conflicting whacker/player
        Boomwhacker* new_whacker = *con_old_player->get_whacker(note->pitch);
        assign_note(note, new_whacker, con_old_player, false);
        return 0;
      }
    }
  }

  // Try to allocate a boomwhacker to cover the conflicting note
  // if ((flags & ALLOCATE) != 0) {
  //   for (int i = 0; i < conflicts.size(); i++) {
  //     Note* con = conflicts[i];
  //     if (add_new_whacker(con) == 0) {
  //       return 0;
  //     } // else, continue
  //   }
  // }  

  // Attempt to recursively offload
  // if ((flags & RECURSE) != 0) {
  //   for (Option* opt : options) {
  //     if (add_offload(opt, flags) == 0) {
  //       return 0;
  //     }
  //     delete opt;
  //   }
  // }

  return 1;
}

// Sort in increasing order of BWs
bool whackers_asc(const Player* a, const Player* b) {
  return a->whackers.size() < b->whackers.size();
}

/**
 * @brief Attempts to allocate a new boomwhacker for this note
 * and assign it to a non-conflicting player.
 */
int Assignment::add_new_whacker(Note* note) {
  Boomwhacker* whacker = find_whacker(note->pitch);
  if (whacker == nullptr) {
    return 1;
  }  

  // Find player that has no conflicts with this note
  auto it = find_if(players.begin(), players.end(), [note](Player* p){
    return (p != note->player) && p->conflicts_back(p->notes.end(), note).empty();
  });
  if (it != players.end()) {
    // Successful add
    Player* player = *it;
    assign_note(note, whacker, player, true);
    log("Whacker", whacker->pitch);
    if (whacker->capped) {
      log("capped", whacker->get_real_pitch());
    } else {
      log("uncapped");
    }

    // Resort to assign notes to players giving priority to whoever has the least
    std::sort(players.begin(), players.end(), whackers_asc);

    return 0;
  } else {
    // Reset whacker
    whacker->dealloc();
    return 1;
  }
}

// Skip assignment of this note
void Assignment::skip(Note* note) {
  log("Conflicting");
  note->player = nullptr;
  note->conflicting = true;
}

void Assignment::assign_note(Note* note, Boomwhacker* whacker, Player* player, bool new_whacker) {
  // Assign note whacker, player, and capped status
  note->whacker = whacker;
  note->player = player;
  note->capped = whacker->capped;

  // Set whacker as used and add note to vector of played notes
  whacker->alloc();
  whacker->add_note(note);

  // Add player note and whacker if new
  if (new_whacker) {
    player->add_whacker(whacker);
  }
  player->add_note(note);

  log("Note assigned to Player", player->id);
}

// Adds a note to the assignment
int Assignment::add_note(Note* note) {
  if (note->pitch < C2_MIDI || note->pitch > G5_MIDI) {
    skip(note);
    return 1;
  }

  log_line();
  log("Try MRP", note->pitch, note->id, note->time);
  // Try to add to MRP
  auto conflicts = add_existing(note);
  bool success = conflicts == std::nullopt;
  if (success) return 0;

  log("Try Offload Basic");
  Option* opt = new Option(note, *conflicts);
  if (add_offload(opt, BASIC) == 0) return 0;
  log("Try Whacker");
  // Try to allocate new BW
  if (add_new_whacker(note) == 0) return 0;
  // log("Try Offload LR");
  if (add_offload(opt, LAST_RESORT) == 0) return 0;

  // All attempts to add have failed thus far. get fucked.
  skip(note);
  delete opt;
  return 1;
}

/**
 * @brief Iterates through all notes and tries to assign them
 */
void Assignment::assign() {
  for (int i = 0; i < cfg->num_notes; i++) {
    // notes[i].player = random_player();
    add_note(&notes[i]);
  }
}