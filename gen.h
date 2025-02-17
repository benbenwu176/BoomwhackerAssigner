#ifndef GEN_H
#define GEN_H

#include <vector>

#define C2_MIDI 36 
#define C4_MIDI 60
#define G5_MIDI 79 
#define NUM_UNIQUE_PITCHES 44 // The number of unique pitches that are playable
#define OCTAVE_INTERVAL 12 // MIDI range of an octave
#define NUM_WHACKER_PITCHES 32 // The number of boomwhackers from G5-C3

enum add_flags {
    NO_RECURSE = 0, // Disables recursive offloading
    RECURSE = 1 << 0, // Allows recursive offloading
    ALLOCATE = 1 << 1, // Allows allocation of new boomwhackers
    PROXIMATE = 1 << 2, // Allows proximate notes to be played by nearby players
    FINAL_RESORT = RECURSE | ALLOCATE | PROXIMATE // Allows all options to be used
};

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
    double time;          // The time, in seconds, the note occurs at in the piece
    int id;               // Index of the note within the note array
    int player;           // The player that this note is assigned to
    int pitch;            // MIDI pitch value
    int whacker_index;     // The index of the boomwhacker that is used for this note
    bool capped;          // Whether the note is played with a capped boomwhacker
    bool proximate;       // Whether the note is proximate
    bool conflicting;     // Whether the note creates a switch conflict
} Note;

typedef struct Boomwhacker {
    int pitch; // The pitch of the boomwhacker
    bool used; // Whether the boomwhacker has been used
    bool capped; // Whether the boomwhacker is capped
    std::vector<Note*> notes; // The notes that are played by this boomwhacker
} Boomwhacker;

typedef struct Bucket {
    std::vector<Note*> notes; // Array of pointers to notes
} Bucket;

typedef struct Player {
    int id;
    std::vector<Boomwhacker*> whackers; // The boomwhackers that the player has
    std::vector<Note*> notes; // The notes that the player has played
    Bucket bucket; // The bucket of notes that the player has played
} Player;

typedef struct Edge{
    int dst; // The destination node
    Note* note; // The note that this edge is connected to
} Edge;

typedef struct Node {
    int player; // The player this node is assigned to
    std::vector<Edge> edges; // The edges that are connected to this node
} Node;

typedef struct Graph {
    std::vector<Node> nodes; // Array of nodes
} Graph;

typedef struct Assignment {
    std::vector<Note> notes;
    std::vector<Boomwhacker*> whackers;
    std::vector<Player> players;
    std::vector<std::vector<Note*>> mrp;
    Graph adjacency_graph;
} Assignment;

#endif  // GEN_H