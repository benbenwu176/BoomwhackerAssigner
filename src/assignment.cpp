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
  Note *note_data = notes.data();
  _setmode(_fileno(stdout), O_BINARY);
  // Write # of conflicts in the final assignment
  int num_conflicts = 0;
  for (int i = 0; i < cfg->num_notes; i++)
  {
    if (notes[i].conflicting)
    {
      num_conflicts++;
    }
  }
  std::cerr << "\nNumber of conflicts: " << num_conflicts << std::endl;

  std::cout.write(reinterpret_cast<const char *>(&num_conflicts), sizeof(int));
  // Write flattened graph of the final assignment
  std::vector<int> flattened_graph = adjacency_graph->flatten();
  std::cout.write(reinterpret_cast<const char *>(flattened_graph.data()), sizeof(int) * cfg->num_players);
  // Write the notes in the final assignment
  std::cout.write(reinterpret_cast<const char *>(note_data), sizeof(Note) * cfg->num_notes);
}

/**
 * @brief Whacker search condition
 */
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
    
  // Return whatever is left
  } else if (uncapped_found) {
    return *uncapped_whacker;
  } else if (capped_found) {
    (*capped_whacker)->cap();
    return *capped_whacker;
  } else {
    return nullptr;
  }

}

// Assign notes to players giving priority to whoever has the least
bool whackers_asc(const Player* a, const Player* b) {
  return a->whackers.size() < b->whackers.size();
}

/**
 * @brief Attempts to locate a player within the MRP that
 * can play this note without causing conflict.
 * 
 * @return A list of all conflicting notes to be used for offloading
 */
std::vector<Note*> Assignment::add_existing(Note* note) {
  std::vector<Player*> queue = mrp->get_queue(note->pitch);
  std::vector<Note*> all_conflicts = {note};
  for (int i = 0; i < queue.size(); i++) {
    // Search for non-conflicting player in the MRP to add this to
    Player* player = queue[i];
    std::vector<Note*> conflicts = player->conflicts(player->notes.end(), note);
    if (conflicts.empty()) {
      player->notes.push_back(note);
      note->player = player->id;
      mrp->add(player, note);
      player->add_note_to_whacker(note);
      std::cerr << "MRP ";
      return conflicts;
    } else {
      all_conflicts.insert(all_conflicts.end(), conflicts.begin(), conflicts.end());
    }
  }
  return all_conflicts;
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
  // TODO: insert player with new bw in sorted place in list instead of sorting every time
  std::sort(players.begin(), players.end(), whackers_asc);
  // Find player who should play this note
  auto p = find_if(players.begin(), players.end(), [note](Player* p){
    return p->conflicts(p->notes.end(), note).empty();
  });
  if (p != players.end()) {
    // On successful add, Add note to whacker & player, whacker to player, player to MRP
    Player* player = *p;
    note->whacker = whacker;
    note->player = player->id;
    note->capped = whacker->capped;
    whacker->used = true;
    whacker->notes.push_back(note);
    player->notes.push_back(note);
    player->whackers.push_back(whacker);
    mrp->add(player, note);
    std::cerr << "Whacker " << whacker->pitch << " " << (whacker->capped ? "capped":"uncapped");
    return 0;
  } else {
    whacker->used = false;
    whacker->capped = false;
    return 1;
  }
}


/**
 * @brief Adds a note to the assignment
 */
int Assignment::add_note(Note* note) {
  if (note->pitch < C2_MIDI || note->pitch > G5_MIDI) {
    // goto skip;
    throw std::runtime_error("Pitch not within range: " + std::to_string(note->pitch));
  }

  std::cerr << "\nStart " << note->pitch << " " << note->id << " ";
  // Try to add to MRP
  std::vector<Note*> all_conflicts = add_existing(note);
  int success = all_conflicts.empty();
  if (success) return 0;
  std::cerr << "Middle ";
  all_conflicts.erase(all_conflicts.begin());
  // Try to offload a conflicting note of players in MRP
  // if (add_offload(note) == 0) return 0;
  std::cerr << "End ";
  // Try to allocate new BW
  if (add_new_whacker(note) == 0) return 0;
  
  // All attempts to add have failed thus far. get fucked.
skip:
  std::cerr << "Conflicting ";
  note->player = -1;
  note->conflicting = true;
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