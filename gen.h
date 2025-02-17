#ifndef GEN_H
#define GEN_H

#include <vector>

#define C2_MIDI 36 
#define C4_MIDI 60
#define G5_MIDI 79 
#define NUM_UNIQUE_PITCHES 44 // The number of unique pitches that are playable
#define OCTAVE_INTERVAL 12 // MIDI range of an octave
#define NUM_WHACKER_PITCHES 32 // The number of boomwhackers from G5-C3

typedef struct Config {
    // Note data
    int num_notes; // number of notes in the piece

    // Parameter data
    int num_players; // The number of players in the ensemble
    int whacker_overflow_limit; // 1 + the max # of boomwhackers that can be played at once
    int whackers_per_pitch; // The number of boomwhackers per pitch

    // Rate data
    double switch_time; // The time, in seconds, it takes a player to switch boomwhackers

} Config;

// Define the Note struct
typedef struct Note {
    double time;        // The time, in seconds, the note occurs at in the piece
    int next;           // Index of the next note of the same pitch
    int id;             // Index of the note within the note array
    int player;         // The player that this note is assigned to
    int pitch;          // MIDI pitch value
    int whacker_index;  // Index of the whacker being used for this note
    bool capped;        // Whether the note is played with a capped boomwhacker
    bool proximate;     // Whether the note is proximate
    bool conflicting;   // Whether the note creates a switch conflict
    
} Note;

typedef struct Boomwhacker {
    bool used; // Whether the boomwhacker has been used
    bool capped; // Whether the boomwhacker is capped
} Boomwhacker;

typedef struct Bucket {
    Note** bucket; // Array of pointers to notes
} Bucket;

#endif  // GEN_H