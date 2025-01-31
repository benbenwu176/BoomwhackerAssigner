#include <iostream>
#include "genplus.h"

Config cfg;

/**
 * @brief Generates an array of notes
 * 
 * @param midis The array of MIDI values
 * @param times The array of time values
 * 
 * @returns An array of pointers to notes
*/
std::vector<Note> generate_note_array(std::vector<int>& pitches, std::vector<double>& times) {
    std::vector<Note> notes(cfg.num_notes);
    for (int i = 0; i < cfg.num_notes; i++) {
        notes[i].time = times[i];
        notes[i].pitch = pitches[i];
        notes[i].id = i;
        notes[i].whacker_index = 0; // Assumes that the first whacker used is at index 0
        if (notes[i].pitch < C2_MIDI + OCTAVE_INTERVAL) {
            // TODO: handle boomwhacker usage
            notes[i].capped = true; // cap if lowest octave
        }
    }
    return notes;
}

int main(int argc, char* argv[]) {
    
    
}