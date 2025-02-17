#ifndef GEN_H
#define GEN_H

#include <vector>

#define C2_MIDI 36 
#define C4_MIDI 60
#define G5_MIDI 79 
#define NUM_UNIQUE_PITCHES 44 // The number of unique pitches that are playable
#define OCTAVE_INTERVAL 12 // MIDI range of an octave
#define NUM_WHACKER_PITCHES 32 // The number of boomwhackers from G5-C3

/* 
 * TODO: Allow whackers per pitch to be unique to each pitch
 * Allow lowest note and highest note to vary
 * Allow different switch times per player for variable difficulty
 * Allow different whacker overflow limits for 4 mallet boomwhacker playing
 * Change NUM_UNIQUE_PITCHES to be a parameter and not a constant
 */

typedef struct Config {
    // Note data
    int num_notes; // number of notes in the piece

    // Thread data
    int threads;

    // Parameter data
    int num_gens; // The number of generations to run the genetic algorithm for
    int population_size; // The number of subjects to run the genetic algorithm on
    int num_players; // The number of players in the ensemble
    int num_parents; // The number of parents to use in the genetic algorithm
    int keep_parents; // How many parents to keep throughout generations
    bool allow_proximates; // whether to allow offloading to nearby players
    int whacker_overflow_limit; // 1 + the max # of boomwhackers that can be played at once
    int whackers_per_pitch; // The number of boomwhackers per pitch

    // Rate data
    double switch_time; // The time, in seconds, it takes a player to switch boomwhackers
    double mutation_rate; // placeholder rate for random note assignment
    double offload_rate; // chance to offload a conflicting note to another player

} Config;

// Define the Note struct
typedef struct Note {
    double time = -1; // The time, in seconds, the note occurs at in the piece
    int next = -1; // Index of the next note of the same pitch
    int id = -1; // Index of the note within the note array
    int player = -1; // The player that this note is assigned to
    int pitch = -1; // MIDI pitch value
    int whacker_index = -1; // Index of the whacker being used for this note
    bool capped = false; // Whether the note is played with a capped boomwhacker
    bool proximate = false; // Whether the note is proximate
    bool conflicting = false; // Whether the note creates a switch conflict
    
} Note;

typedef struct Boomwhacker {
    bool used; // Whether the boomwhacker has been used
    bool capped; // Whether the boomwhacker is capped
} Boomwhacker;

typedef struct Assignment {
    std::vector<Note> notes; // Vector of notes
    std::vector<int> whacker_table; // Hash table of pointers to note heads
    std::vector<Boomwhacker> bws; // Array of pointers to boomwhackers
    int score; // Playability score

    bool operator<(const Assignment& other) const;

    void update_score();
} Assignment;

typedef struct Bucket {
    Note** bucket; // Array of pointers to notes
} Bucket;

#endif  // GEN_H