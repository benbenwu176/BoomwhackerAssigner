#pragma once

#define C2_MIDI 36 
#define C3_MIDI 48
#define C4_MIDI 60
#define G5_MIDI 79 
#define NUM_UNIQUE_PITCHES 44 // The number of unique pitches that are playable
#define OCTAVE_INTERVAL 12 // MIDI range of an octave
#define NUM_WHACKER_PITCHES 32 // The number of boomwhackers from G5-C3

#define PITCH_TO_INDEX(pitch) (pitch - C2_MIDI)

typedef enum {
    NO_RECURSE = 0 << 0, // Disables recursive offloading
    RECURSE = 1 << 0, // Allows recursive offloading
    ALLOCATE = 1 << 1, // Allows allocation of new boomwhackers
    PROXIMATE = 1 << 2, // Allows proximate notes to be played by nearby players
    ROOT = 1 << 3, // Denotes the root of the recursive offloading tree
    LAST_RESORT = RECURSE | ALLOCATE | PROXIMATE // Allows all options to be used
} add_flags;