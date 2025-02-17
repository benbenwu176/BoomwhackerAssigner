/**
 * @file gen.cpp
 * @brief Main file for the GenPlus program.
 * @version 0.1
 * @date 2/17/2025
 * @author Benjamin Wu
 * 
 * TODO: 
 * Write # of conflicts and flattened graph to stdout
 * Allow whackers per pitch to be unique to each pitch
 * Allow lowest note and highest note to vary
 * Allow different switch times per player for variable difficulty
 * Allow different whacker overflow limits for 4 mallet boomwhacker playing
 * Change NUM_UNIQUE_PITCHES to be a parameter and not a constant
 * Split functions into multiple files
 */
#include <iostream>
#include <random>
#include <thread>
#include <io.h>
#include <fcntl.h>
#include "gen.h"

Config cfg;

void assign(std::vector<int>& pitches, std::vector<double>& times) {
    
}

/**
 * @brief Print the data from stdin containing pitches, times, parameters, and rates.
 * 
 * @param n The number of notes.
 * @param p The number of parameters.
 * @param r The number of rates.
 * @param pitches The pitches of the notes.
 * @param times The times of the notes.
 * @param parameters The integer parameters.
 * @param rates The rate parameters.
 */
void check_params(int n, int p, int r, std::vector<int>& pitches, std::vector<double>& times, 
                    std::vector<int>& parameters, std::vector<double>& rates) {
    for (int i = 0; i < n; i++) {
        std::cerr << pitches[i] << " ";
    }
    std::cerr << std::endl;
    for (int i = 0; i < n; i++) {
        std::cerr << times[i] << " ";
    }
    std::cerr << std::endl;
    for (int i = 0; i < p; i++) {
        std::cerr << parameters[i] << " ";
    }
    std::cerr << std::endl;
    for (int i = 0; i < r; i++) {
        std::cerr << rates[i] << " ";
    }
    std::cerr << std::endl;
}

/** 
 * @brief Initialize the config struct with the given parameters and rates.
 * 
 * @param n The number of notes.
 * @param parameters The integer parameters.
 * @param rates The rate parameters.
 * 
 * IMPORTANT: Maintain this with the order of the parameters and rates.
 */
void init_cfg(int n, const std::vector<int>& parameters, const std::vector<double>& rates) {
    // Note data
    cfg.num_notes = n;

    // Parameter data
    cfg.num_players = parameters[0];
    cfg.whacker_overflow_limit = parameters[1];
    cfg.whackers_per_pitch = parameters[2]; 

    // Rate data
    cfg.switch_time = rates[0] + 0.0001; // Add by small amount to avoid floating point errors
}

/**
 * @brief Main function. Reads array lengths from args and uses a char buffer to read 
 * in the array data from stdin. NOTE: This buffer must be padded to the nearest page size,
 * otherwise the read will fail and data will be corrupted.
 * 
 * @return 0 on success, 1 on failure
 */
int main(int argc, char* argv[]) {
    // Read n, p, r from args
    int n = atoi(argv[1]);
    int p = atoi(argv[2]);
    int r = atoi(argv[3]);

    int data_size = n * sizeof(int) + n * sizeof(double); + p * sizeof(int) + r * sizeof(double);
    int padding = 4096 - (data_size) % 4096;
    char* buffer = new char[data_size + padding];
    std::cin.read(reinterpret_cast<char*>(buffer), data_size + padding);
    
    std::vector<int> pitches(n);
    std::vector<double> times(n);
    std::vector<int> parameters(p);
    std::vector<double> rates(r);

    memcpy(pitches.data(), buffer, n * sizeof(int));
    memcpy(times.data(), buffer + n * sizeof(int), n * sizeof(double));
    memcpy(parameters.data(), buffer + n * sizeof(int) + n * sizeof(double), p * sizeof(int));
    memcpy(rates.data(), buffer + n * sizeof(int) + n * sizeof(double) + p * sizeof(int), r * sizeof(double));
    delete[] buffer;
    
    check_params(n, p, r, pitches, times, parameters, rates);

    init_cfg(n, parameters, rates);

    assign(pitches, times);

    return 0;
}