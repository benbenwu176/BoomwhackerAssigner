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
    players.push_back(new Player(i, cfg->hold_limit, cfg->switch_time));
  }
}

/**
 * @brief Initialize the boomwhacker table.
 */
void Assignment::init_whackers() {
  whacker_table = std::vector<std::vector<Boomwhacker *>>(NUM_WHACKER_PITCHES * cfg->whackers_per_pitch);
  for (int i = 0; i < NUM_WHACKER_PITCHES; i++)
  {
    std::vector<Boomwhacker*>& whackers = whacker_table[i];
    for (int j = 0; j < cfg->whackers_per_pitch; j++) {
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
 * @brief Attempts to find an available boomwhacker that can play this pitch
 */
Boomwhacker* Assignment::find_whacker(int pitch) {
  // Look for boomwhacker at natural pitch
  int table_index = pitch - C3_MIDI;
  if (table_index >= 0 && table_index < NUM_WHACKER_PITCHES) {
    std::vector<Boomwhacker*>& whackers = whacker_table[table_index];
    for (int i = 0; i < whackers.size(); i++) {
      if (whackers[i]->used == false) {
        // Unused whacker found, return it
        return whackers[i];
      }
    }
  }

  // Look an octave above for a boomwhacker to cap
  table_index += OCTAVE_INTERVAL;
  if (table_index >= 0 && table_index < NUM_WHACKER_PITCHES) {
    std::vector<Boomwhacker*>& whackers = whacker_table[table_index];
    for (int i = 0; i < whackers.size(); i++) {
      if (whackers[i]->used == false) {
        // Whacker found, cap and return it
        whackers[i]->capped = true;
        return whackers[i];
      }
    }
  }
  // No suitable whackers have been found, return nullptr
  return nullptr;
}

// Assign notes to players giving priority to whoever has the least
bool whackers_asc(const Player* a, const Player* b) {
  return a->whackers.size() < b->whackers.size();
}

/**
 * @brief Attempts to locate a player within the MRP that
 * can play this note without causing conflict.
 */
int Assignment::add_existing(Note* note) {
  std::vector<Player*> queue = mrp->get_queue(note->pitch);
  for (int i = 0; i < queue.size(); i++) {
    Player* player = queue[i];
    bool note_added = player->bucket->try_add(note);
    if (note_added) {
      // On successful add, update MRP and Note player
      note->player = player->id;
      mrp->add(player, note);
      // Add note to boomwhacker
      std::vector<Boomwhacker*>& player_whackers = player->whackers;
      for (int j = 0; j < player_whackers.size(); j++) {
        if (player_whackers[j]->pitch == note->pitch) {
          player_whackers[j]->notes.push_back(note);
          note->capped = player_whackers[j]->capped;
        }
      }
      std::cerr << "MRP ";
      return 0;
    }
  }
  return 1;
}


/**
 * @brief Adds a note to the assignment
 */
int Assignment::add_note(Note* note) {
  std::cerr << "\nStart " << note->pitch << " " << note->id << " ";
  if (note->pitch < C2_MIDI || note->pitch > G5_MIDI) {
    throw std::runtime_error("Pitch not within range: " + std::to_string(note->pitch));
  }

  int success = add_existing(note);
  if (success == 0) {
    return 0;
  }
  
  std::cerr << "Middle ";
  // TODO: Offload

  // Get queue of players in note MRP
  // in order of mrp, try to offload one of mrp's **conflicting** boomwhackers
  // for the pitch of each conflicting boomwhacker (last 2 in bucket)
  // look for player in mrp[conflicting pitch] that can play this note
  // switch all until conflict occurs
  // OR if none others in MRP can play, but spare boomwhacker: allocate bw and add to non-conflicting player


  std::cerr << "End ";
  // MRP and offloading failed, try to allocate new BW
  Boomwhacker* whacker = find_whacker(note->pitch);
  if (whacker) {
    std::sort(players.begin(), players.end(), whackers_asc);
    // Find player who should play this note
    for (int i = 0; i < players.size(); i++) {
      Player* player = players[i];
      bool note_added = player->bucket->try_add(note);
      if (note_added) {
        // On successful add, Add note to whacker & player, whacker to player, player to MRP
        note->whacker = whacker;
        note->player = player->id;
        note->capped = whacker->capped;
        whacker->used = true;
        player->whackers.push_back(whacker);
        mrp->add(player, note);
        std::cerr << "Whacker " << whacker->pitch << " " << (whacker->capped ? "capped":"uncapped");
        return 0;
      } else {
        // Reset whacker
        whacker->used = false;
        whacker->capped = false;
      }
    }
  }
  
  // All attempts to add have failed thus far. get fucked.
  
  std::cerr << "Conflicting ";
  note->player = -1;
  note->conflicting = true;
  return 0;
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