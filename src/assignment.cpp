#include "gen.hpp"
#include "assignment.hpp"
#include <thread>
#include <io.h>
#include <iostream>
#include <fcntl.h>

/**
 * @brief Returns a random player number. Thread-safe.
 */
int random_player() {
  return random_utils::randInt(0, cfg->num_players - 1);
}

/**
 * @brief Helper method to log to gen_output
 */
void log(std::string s) {
  std::cerr << s;
}

/**
 * @brief Initialize the Most Recently Played cache.
 */
MRP::MRP() {
  data = std::vector<std::vector<Player*>>(NUM_UNIQUE_PITCHES);
}

/**
 * @brief Get a queue of players who most recently played this pitch.
 */
std::vector<Player*>& MRP::get_queue(int pitch) {
  return data[PITCH_TO_INDEX(pitch)];
}

/**
 * @brief Add a note to the MRP
 * 
 * If the player is already in the MRP, remove it and add it to front
 * Else, add as new to the front
 */
void MRP::add(Player* player, Note* note) {
  std::vector<Player*>& queue = get_queue(note->pitch);
  for (int i = 0; i < queue.size(); i++) {
    if (queue[i] == player) {
      queue.erase(queue.begin() + i);
      break;
    }
  }
  queue.insert(queue.begin(), player);
}


/**
 * @brief Creates an Assignment object with no assigned notes.
 */
Assignment::Assignment(std::vector<int> &pitches, std::vector<double> &times) {
  init_notes(pitches, times);
  init_players();
  init_whackers();
  mrp = new MRP();
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
    players.push_back(new Player(i, cfg->hold_limit, cfg->switch_times[i]));
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
      whackers.push_back(new Boomwhacker(pitch));
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
  _setmode(_fileno(stdout), O_BINARY);
  // Write # of conflicts in the final assignment
  int num_conflicts = 0;
  for (int i = 0; i < cfg->num_notes; i++) {
    if (notes[i].conflicting) {
      num_conflicts++;
    }
  }
  std::cerr << "\nNumber of conflicts: " << num_conflicts << std::endl;

  std::cout.write(reinterpret_cast<const char *>(&num_conflicts), sizeof(int));
  // Write flattened graph of the final assignment
  std::vector<int> flattened_graph = adjacency_graph->flatten();
  std::cout.write(reinterpret_cast<const char *>(flattened_graph.data()), sizeof(int) * cfg->num_players);

  // Write relevant note fields to genpy.py
  for (int i = 0; i < notes.size(); i++) {
    Note note = notes[i];
    int player_id = note.player != nullptr ? note.player->id : -1;
    std::cout.write(reinterpret_cast<const char*>(&note.time), sizeof(double));
    std::cout.write(reinterpret_cast<const char*>(&player_id), sizeof(int));
    std::cout.write(reinterpret_cast<const char*>(&note.capped), sizeof(bool));
    std::cout.write(reinterpret_cast<const char*>(&note.conflicting), sizeof(bool));
  }
  std::cout.flush();
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
Note* Assignment::find_closest_before(const std::vector<Note*> notes, double target) {
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

    if (cfg->enable_whacker_priority) {
      // Pick whichever has more whackers left
      if (remaining_uncapped >= remaining_capped) {
        return *uncapped_whacker;
      } else {
        (*capped_whacker)->cap();
        return *capped_whacker;
      }
    } else {
      return *uncapped_whacker;
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
    std::vector<Note*> conflicts = player->conflicts(player->notes.end(), note);

    // Assign to first player with no conflicts
    if (conflicts.empty()) {
      // Assign note
      assign_note(note, player->get_whacker(note->pitch), player, false);
      std::cerr << "MRP ";

      // Return successful add (nullopt)
      return std::nullopt;
    } else {
      all_conflicts.insert(all_conflicts.end(), conflicts.begin(), conflicts.end());
    }
  }
  // Return all notes that conflict
  return all_conflicts;
}

/*
 * Attempt to offload to other players that have this note
 *
 * Tries a breadth-first search of the options at the first level,
 * then resorts to depth-first if the previous attempts failed.
 */
int Assignment::add_offload(Note* note, std::vector<Note*> all_conflicts) {
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
    return p->conflicts(p->notes.end(), note).empty();
  });
  if (it != players.end()) {
    // On successful add, Add note to whacker & player, whacker to player, player to MRP
    Player* player = *it;
    assign_note(note, whacker, player, true);
    std::cerr << "Whacker " << whacker->pitch << " ";
    if (whacker->capped) {
      std::cerr << "capped " << whacker->get_real_pitch() << " ";
    } else {
      std::cerr << "uncapped" << " ";
    }

    // Resort to assign notes to players giving priority to whoever has the least
    std::sort(players.begin(), players.end(), whackers_asc);

    return 0;
  } else {
    whacker->used = false;
    whacker->capped = false;
    return 1;
  }
}

// Skip assignment of this note
void Assignment::skip(Note* note) {
  std::cerr << "Conflicting ";
  note->player = nullptr;
  note->conflicting = true;
}

void Assignment::assign_note(Note* note, Boomwhacker* whacker, Player* player, bool new_whacker) {
  // Assign note whacker, player, and capped status
  note->whacker = whacker;
  note->player = player;
  note->capped = whacker->capped;

  // Set whacker as used and add note to vector of played notes
  whacker->used = true;
  whacker->add_note(note);

  // Add player note and whacker if new
  if (new_whacker) {
    player->add_whacker(whacker);
  }
  player->add_note(note);

  std::cerr << "P" << player->id << " ";
}

/**
 * @brief Adds a note to the assignment
 */
int Assignment::add_note(Note* note) {
  if (note->pitch < C2_MIDI || note->pitch > G5_MIDI) {
    skip(note);
    return 1;
    // throw std::runtime_error("Pitch not within range: " + std::to_string(note->pitch));
  }

  std::cerr << "\nStart " << note->pitch << " " << note->id << " ";
  // Try to add to MRP
  auto conflicts = add_existing(note);
  bool success = conflicts == std::nullopt;
  if (success) return 0;

  std::cerr << "Middle ";
  std::vector<Note*> all_conflicts = *conflicts;
  // Try to offload a conflicting note of players in MRP
  if (add_offload(note, all_conflicts) == 0) return 0;
  std::cerr << "End ";
  // Try to allocate new BW
  if (add_new_whacker(note) == 0) return 0;
  
  // All attempts to add have failed thus far. get fucked.
  skip(note);
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

/*
TODO: manage fields regarding mrp, note, player, and boomwhacker when a note is properly assigned
*/