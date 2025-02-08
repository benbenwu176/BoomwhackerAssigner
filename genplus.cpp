#include <iostream>
#include "genplus.h"
#include <cstdint>
#include <cstdio>  // For setvbuf
#include <cstdlib> // For malloc and free

Config cfg;

/**
 * @brief Generates an array of notes
 * 
 * @param midis The array of MIDI values
 * @param times The array of time values
 * 
 * @returns An array of pointers to notes
*/
std::vector<Note> generate_note_array(std::vector<int>& pitches, std::vector<float>& times) {
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
    std::vector<int> nums(3072);
    std::cin.read(reinterpret_cast<char*>(nums.data()), 3072 * sizeof(int));
    for (int val : nums) {
        std::cout << val << " ";
    }
    return 0;
    /*
    // Read n, p, r from stdin
    int n, p, r;

    std::cin.read(reinterpret_cast<char*>(&n), sizeof(int));
    std::cin.read(reinterpret_cast<char*>(&p), sizeof(int));
    std::cin.read(reinterpret_cast<char*>(&r), sizeof(int));

    // Read the integer array of size n
    std::vector<int> int_array_n(n);
    std::cin.read(reinterpret_cast<char*>(int_array_n.data()), n * sizeof(int));

    // Read the float array of size n
    std::vector<float> float_array_n(n);
    std::cin.read(reinterpret_cast<char*>(float_array_n.data()), n * sizeof(float));

    // Read the integer array of size p
    std::vector<int> int_array_p(p);
    std::cin.read(reinterpret_cast<char*>(int_array_p.data()), p * sizeof(int));

    // Read the float array of size r
    std::vector<float> float_array_r(r);
    std::cin.read(reinterpret_cast<char*>(float_array_r.data()), r * sizeof(float));

    // Verify that all data was read successfully
    // if (!std::cin) {
    //     std::cerr << "Error reading input data!" << std::endl;
    //     return 1;
    // }

    // Print the received data for verification
    std::cout << "n: " << n << ", p: " << p << ", r: " << r << std::endl;

    std::cout << "int_array_n: ";
    for (int val : int_array_n) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    std::cout << "float_array_n: ";
    for (float val : float_array_n) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    std::cout << "int_array_p: ";
    for (int val : int_array_p) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    std::cout << "float_array_r: ";
    for (float val : float_array_r) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    return 0;
    */
}