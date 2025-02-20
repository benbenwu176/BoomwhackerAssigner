#include <iostream>
#include <random>
#include <thread>
#include <io.h>
#include <fcntl.h>
#include "genplus.h"

Config cfg;

/**
 * @brief Returns a random player number. Thread-safe.
 */
int random_player() {
    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(0, cfg.num_players - 1);
    return distribution(generator);
}

/*
 * Basic test function for multi-threading
 */
void thread_func(int id) {
    // Print a random player number
    std::cerr << "Thread " << id << ": " << random_player() << std::endl;
}

/**
 * @brief Creates a deep copy of the given assignment
 * 
 * This function allocates new memory for the copied assignment and its note array
 * 
 * @param src The assignment to be copied
 * @param dst The assignment to copy to
 * 
 * @return None
 */
void copy_assignment(Assignment* src, Assignment* dst) {
    if (src == nullptr || dst == nullptr) {
        std::cerr << "Copy assignment: Invalid arguments, src or dst is null" << std::endl;
        return;
    }
    dst->notes = src->notes;
    dst->whacker_table = src->whacker_table;
    dst->bws = src->bws;
    dst->score = src->score;
}

/**
 * @brief Initializes a population of assignments with the given size and number of players.
 * 
 * @param assignments A pointer to an array of Assignment pointers to be initialized.
 * @param init The initial assignment to use as a starting point.
 * 
 * @return None
 */
void init_population(std::vector<Assignment>& assignments, Assignment* init) {
    for (int i = 0; i < assignments.size(); i++) {
        copy_assignment(init, &assignments[i]);
        assignments[i].score = cfg.population_size - i;
    }
}

/**
 * @brief Generates a set of assignments based on the given parameters and initial assignment.
 * 
 * @param init The initial assignment to use as a starting point.
 * 
 * @return A pointer to an array of Assignment pointers, representing the most fit assignment generated.
 */
Assignment* generate_assignments(Assignment* init) {
    std::vector<Assignment> assignments(cfg.population_size);
    init_population(assignments, init);
    for (int i = 0; i < cfg.num_gens; i++) {
        // breed(assignments);
    }
    // Sort the assignments by playability score in ascending order
    return &assignments[0];
}

/**
 * @brief Generates an array of notes
 * 
 * @param pitches The vector of note pitches
 * @param times The vector of note times
 * 
 * @returns A vector of Notes with the given pitches and times
*/
std::vector<Note> generate_note_array(std::vector<int>& pitches, std::vector<double>& times) {
    std::vector<Note> notes(cfg.num_notes);
    for (int i = 0; i < cfg.num_notes; i++) {
        notes[i].time = times[i];
        notes[i].id = i;
        notes[i].pitch = pitches[i];
        notes[i].whacker_index = 0; // Assumes the first whacker used is at index 0
        if (notes[i].pitch < C2_MIDI + OCTAVE_INTERVAL) {
            // TODO: handle boomwhacker usage
            notes[i].capped = true; // Cap if below C3
        }
    }
    return std::move(notes);
}

/** 
 * @brief Generates a hash table of notes that are connected by equal pitch
 * 
 * Creates a hash table of notes so that they are indexable by pitch - the lowest note pitch. Returns a list of pointers representing 
 * the first note of each pitch that occurs in the piece.
 * 
 * @pre: Notes is sorted by time and is initialized with note data, pitches are within range
 * 
 * @param notes The vector of notes
 * 
 * @returns A vector of pointers to notes
 */
std::vector<int> generate_whacker_table(std::vector<Note>& notes) {
    std::vector<int> whacker_table(NUM_UNIQUE_PITCHES);
    for (int i = 0; i < NUM_UNIQUE_PITCHES; i++) {
        whacker_table[i] = -1;
    }
    // Traverse the note vector and construct linked lists in the whacker table
    for (int i = 0; i < cfg.num_notes; i++) {
        int pitch_index = notes[i].pitch - C2_MIDI;
        int head_id = whacker_table[pitch_index];
        if (head_id == -1) {
            // First note of this pitch
            whacker_table[pitch_index] = notes[i].id;
        } else {
            // Add to "linked" list
            int cur_id = head_id;
            while (notes[cur_id].next != -1) {
                cur_id = notes[cur_id].next;
            }
            notes[cur_id].next = notes[i].id;
        }
    }
    return std::move(whacker_table);
}

/**
 * @brief Randomly assigns players to notes
 * 
 * @param assignment The assignment to be randomized
 */
void randomize_player_assignments(Assignment* assignment) {
    // Iterate through each pitch and randomly assign a player
    for (int i = 0; i < NUM_UNIQUE_PITCHES; i++) {
        int player = random_player();
        int cur_id = assignment->whacker_table[i];
        while (cur_id != -1) {
            assignment->notes[cur_id].player = player;
            cur_id = assignment->notes[cur_id].next;
        }
    }
}

/**
 * @brief Initializes the boomwhackers and their starting status
 * 
 * @param whackerTable A hash table of notes by pitch
 * 
 * @returns A vector of boomwhackers
 */
std::vector<Boomwhacker> generate_bws(std::vector<int>& whackerTable) {
    // TODO: implement this
    return std::move(std::vector<Boomwhacker>(NUM_UNIQUE_PITCHES));
}

/**
 * @brief Assigns player values to each note in the piece.
 * 
 * Processes the note data. Initializes the generations and runs the genetic algorithm.
 * 
 * @param pitches The pitches of the notes.
 * @param times The times of the notes.
 */
void assign(std::vector<int>& pitches, std::vector<double>& times) {
    Assignment init;
    init.notes = generate_note_array(pitches, times);
    init.whacker_table = generate_whacker_table(init.notes);
    init.bws = generate_bws(init.whacker_table);
    init.score = 0;

    randomize_player_assignments(&init);
    
    // Assignment* final = generate_assignments(&init);
    Assignment* final = &init; // TODO: remove this line

    // Print the note objects TODO: overload << operator in Note class
    // IMPORTANT: Keep this in sync with the python struct format and struct definition in genplus.h
    Note* notes = final->notes.data();
    _setmode(_fileno(stdout), O_BINARY);
    std::cout.write(reinterpret_cast<const char*>(notes), sizeof(Note) * cfg.num_notes);
    // Note: Don't write anything else to stdout (i.e. newline), or the parsing of the note array will fail
    // Note 2: It is necessary to set cout to binary mode, otherwise write will insert random newlines that offset the data
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
        std::cout << pitches[i] << " ";
    }
    std::cout << std::endl;
    for (int i = 0; i < n; i++) {
        std::cout << times[i] << " ";
    }
    std::cout << std::endl;
    for (int i = 0; i < p; i++) {
        std::cout << parameters[i] << " ";
    }
    std::cout << std::endl;
    for (int i = 0; i < r; i++) {
        std::cout << rates[i] << " ";
    }
    std::cout << std::endl;
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

    // Thread data
    cfg.threads = std::thread::hardware_concurrency();
    
    // Parameter data
    cfg.num_gens = parameters[0];
    cfg.population_size = parameters[1];
    cfg.num_players = parameters[2];
    cfg.num_parents = parameters[3];
    cfg.keep_parents = parameters[4];
    cfg.allow_proximates = parameters[5];
    cfg.whacker_overflow_limit = parameters[6];
    cfg.whackers_per_pitch = parameters[7]; 

    // Rate data
    cfg.switch_time = rates[0] + 0.0001; // Add by small amount to avoid floating point errors
    cfg.mutation_rate = rates[1];
    cfg.offload_rate = rates[2];
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
    
    // check_params(n, p, r, pitches, times, parameters, rates);

    init_cfg(n, parameters, rates);

    assign(pitches, times);

    return 0;
}