#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h> // Threading functions
#include <time.h> // TODO: implement thread-safe random number generator
#include "gen.h"    

int numGens; // The number of generations to run the genetic algorithm for
int populationSize; // The number of subjects to run the genetic algorithm on
int numPlayers; // The number of players in the ensemble
int numParents; // The number of parents to use in the genetic algorithm
int keepParents; // How many parents to keep throughout generations
bool allowProximates; // whether to allow offloading to nearby players
int WHACKER_OVERFLOW_LIMIT; // 1 + the max # of boomwhackers that can be played at once
int whackersPerPitch; // The number of boomwhackers per pitch
double switchTime; // The time, in seconds, it takes a player to switch boomwhackers
double mutationRate; // placeholder rate for random note assignment
double offloadRate; // chance to offload a conflicting note to another player
int numNotes; // number of notes in the piece

/**
 * @brief Generates an array of notes
 * 
 * @param midis The array of MIDI values
 * @param times The array of time values
 * 
 * @returns An array of pointers to notes
*/
Note** generateNoteArray(int* midis, double* times) {
    Note** noteArray = (struct Note **) calloc(numNotes, sizeof(struct Note*));
    Note* notes = (struct Note*) calloc(numNotes, sizeof(struct Note));
    for (int i = 0; i < numNotes; i++) {
        noteArray[i] = &notes[i];
        notes[i].time = times[i];
        notes[i].id = i;
        notes[i].pitch = midis[i];
        notes[i].whackerIndex = 0; // Assumes that the first whacker used is at index 0
        if (notes[i].pitch < C2_MIDI + OCTAVE_INTERVAL) {
            // TODO: handle boomwhacker usage
            notes[i].capped = true; // cap if lowest octave
        }
    }
    return noteArray;
}

/** 
 * @brief Gets an array of boomwhackers
 * 
 * Creates a hash table of notes so that they are indexable by pitch
 * 
 * @pre: notes is sorted by time and is initialized with note data, pitches are within range
 */
Note** generateWhackerTable(Note** noteArray) {
    Note** whackerTable = (struct Note**) calloc(NUM_UNIQUE_PITCHES, sizeof(struct Note*));
    // Traverse the noteArray and construct linked lists in the whackerTable
    for (int i = 0; i < numNotes; i++) {
        Note* note = noteArray[i];
        int index = note->pitch - C2_MIDI;
        Note* cur = whackerTable[index];
        if (cur == NULL) {
            whackerTable[index] = note;
        } else {
            while(cur->next != NULL) {
                cur = cur->next;
            }
            cur->next = note;
        }
    }
    return whackerTable;
}

/**
 * @brief Generates an array of boomwhacker statuses
 * 
 * @param whackerTable A pointer to an array of Note heads by pitch
 * 
 * @returns An array of pointers to boomwhacker statuses
*/
Boomwhacker** generateWhackerStatuses(Note** whackerTable) {
    Boomwhacker** statuses = (struct Boomwhacker**) calloc(NUM_WHACKER_PITCHES, sizeof(struct Boomwhacker*));
    Boomwhacker* whackers = (struct Boomwhacker*) calloc(whackersPerPitch, sizeof(struct Boomwhacker));
    // TODO: manage boomwhacker usage when you use a capped low and low, i.e. initializing with C2 and C3
    for (int i = 0; i < NUM_WHACKER_PITCHES; i++) {
        statuses[i] = &whackers[i];
        whackers[i].used = false;
        whackers[i].capped = false;
    }
    return statuses;
}

/**
 * Prints the contents of a note array.
 * 
 * @param noteArray A pointer to an array of Note pointers.
 * 
 * @return None
 */
void printNoteArray(Note** noteArray) {
    for (int i = 0; i < numNotes; i++) {
        Note* note = noteArray[i];
        printf("pitch: %d\t player: %d\t capped: %d\t proximate: %d\t id: %-4d\t time: %f\n", note->pitch, note->player, note->capped, note->proximate, note->id, note->time);
    }
}

/**
 * Prints the contents of a whacker table, which is a hash table of notes indexed by pitch.
 * 
 * @param whackerTable A pointer to an array of Note pointers, where each Note represents a whacker.
 * 
 * @return None
 */
void printWhackerTable(Note** whackerTable) {
    for (int i = 0; i < NUM_UNIQUE_PITCHES; i++) {
        Note* cur = whackerTable[i];
        while (cur != NULL) {
            printf("pitch: %d\t player: %d\t capped: %d\t proximate: %d\t id: %-4d\t time: %f\n", cur->pitch, cur->player, cur->capped, cur->proximate, cur->id, cur->time);
            cur = cur->next;
        }
    }
}

/**
 * @brief Prints an array of assignments
 * 
 * @param assignments The array of assignments to be printed
 * @param opt The print option
 * 
 * @return None
 */
void debug(Assignment** assignments, int opt) {
    for (int i = 0; i < populationSize; i++) {
        printf("Assignment %d:\t pScore: %d\n", i, assignments[i]->pScore);
        switch (opt) {
            case 1:
                printNoteArray(assignments[i]->noteArray);
                printf("\n\n");
                break;
            case 2:
                printWhackerTable(assignments[i]->whackerTable);
                printf("\n\n");
                break;
            case 0:
            default:
                break;
        }
    }
}

/**
 * @brief Creates a deep copy of the given assignment.
 * 
 * This function allocates new memory for the copied assignment and its note array.
 * 
 * @param assignment The assignment to be copied.
 * 
 * @return A pointer to the copied assignment.
 */
void copyAssignment(Assignment* assignment, Assignment* copy) {
    copy->pScore = assignment->pScore; // Copy pscore
    // Copy noteArray (raw note data)
    copy->noteArray = (Note**) calloc(numNotes, sizeof(Note*));
    Note* notes = calloc(numNotes, sizeof(Note));
    for (int i = 0; i < numNotes; i++) {
        copy->noteArray[i] = &notes[i];
        notes[i] = *assignment->noteArray[i];
    }
    // Copy whackerTable (Note* Next pointer)
    copy->whackerTable = (Note**) calloc(NUM_UNIQUE_PITCHES, sizeof(Note*));
    for (int i = 0; i < NUM_UNIQUE_PITCHES; i++) {
        Note* assnCur = assignment->whackerTable[i];
        if (assnCur == NULL) {
            copy->whackerTable[i] = NULL;
        } else {
            Note* copyCur = copy->noteArray[assnCur->id]; // Find head of linked list
            copy->whackerTable[i] = copyCur;
            // Construct linked list
            while (assnCur->next != NULL) {
                copyCur->next = copy->noteArray[assnCur->next->id];
                copyCur = copyCur->next;
                assnCur = assnCur->next;
            }
        }
    }
}

/**
 * @brief Frees the memory used by an assignment
 * 
 * @param assignment The assignment to be freed
 * 
 * @pre: numNotes > 0
 * @return None
*/
void freeAssignment(Assignment* assignment) {
    free(assignment->noteArray[0]);
    free(assignment->noteArray);
    free(assignment->whackerTable);
}

/**
 * @brief Checks if a bucket has a timing conflict
 * 
 * A timing conflict occurs if the time difference between the notes at index 2 and 0 is less than SECOND_DELTA.
 * This represents a player not being able to switch notes in time.
 * 
 * @param bucket The bucket to be checked
 * 
 * @pre the bucket is full
 * 
 * @return 1 if a timing conflict has occurred, otherwise 0
*/
bool bucketHasConflict(Note** bucket) {
    // TODO: alter conditional to factor in whacker usage?? i.e. two of the same boomwhacker in one hand but that should rarely happen
    // Calculate the time difference between the notes at index 2 and 0
    double timeDiff = bucket[WHACKER_OVERFLOW_LIMIT - 1]->time - bucket[0]->time;

    // Return 1 if the time difference is less than SECOND_DELTA, otherwise return 0
    return (timeDiff < switchTime) ? 1 : 0;
    // Cases in which adding a note conflicts with both notes will still return that one
    // conflict has occurred instead of two. Change? Maybe...
}

/**
 * @brief Adds a note to a bucket
 * 
 * The bucket is initially initialized with NULL pointers. 
 * Upon adding a note to the bucket, attempt to fill the first slot that is empty. 
 * The bucket should always contain notes with distinct pitches. 
 * When trying to add a note with the same pitch as a note already contained within the bucket, 
 * set the entry in the array with the duplicate pitch to NULL. 
 * Then, shift the remaining elements to the start of the array. 
 * Then, continue to add to the bucket.The bucket should always be sorted by time, 
 * and the note to be added will always have a time value greater or equal to any of 
 * the notes already contained in the bucket. Once the note is added, check if the bucket is full. 
 * If the bucket is full, find the difference between the time values of the notes at index 2 and 0. 
 * If the difference between them is less than SECOND_DELTA, then return 1. Otherwise, return 0.
 * 
 * @param bucket The bucket to be added to
 * @param note The note to be added
 * 
 * @return If a timing conflict has occurred, return 1. Otherwise, return 0.
*/
bool addToBucket(Note** bucket, Note* note) {
    // Find the first empty slot in the bucket and index of duplicate note if they exist
    int nullIndex = -1;
    int dupeIndex = -1;
    for (int i = 0; i < WHACKER_OVERFLOW_LIMIT; i++) {
        if (bucket[i] != NULL) {
            if (bucket[i]->pitch == note->pitch) {
                dupeIndex = i;
            }
        } else {
            nullIndex = i;
            break;
        }
    }
    bool bucketFull = nullIndex == -1; // No null space found
    bool dupeFound = dupeIndex != -1; // Duplicate index found
    if (!bucketFull && !dupeFound) {
        bucket[nullIndex] = note; // Simple add, no duplicates and bucket has space
        if (nullIndex == WHACKER_OVERFLOW_LIMIT - 1) {
            return note->conflicting = bucketHasConflict(bucket);
        }
        return false;
    } else if (dupeFound) {
        bucket[dupeIndex] = NULL; // Set the duplicate note at the dupeIndex to NULL
    } else {
        bucket[0] = NULL; // No duplicate found and bucket is full, remove the first note in the bucket
    }
    // Decrement nullIndex to account for the removed note
    nullIndex--;
    // Shift the remaining elements to the start of the array
    for (int i = 0; i < WHACKER_OVERFLOW_LIMIT - 1; i++) {
        Note* next = bucket[i + 1];
        if (next != NULL) {
            bucket[i] = next;
        }
    }
    if (bucketFull) {
        bucket[WHACKER_OVERFLOW_LIMIT - 1] = note;
        return note->conflicting = bucketHasConflict(bucket); // TODO: be on the lookout for logic errors with this
    } else {
        bucket[nullIndex] = note;
        return false;
    }
}

/**
 * @brief Updates the playability score of an assignment
 * 
 * @param assignment The assignment to be updated
 * @return None
*/
void updatePScore(Assignment* assignment) {
    assignment->pScore = 0;
    // Create player buckets
    Note*** playerBuckets = (Note ***) calloc(numPlayers, sizeof(Note**));
    for (int i = 0; i < numPlayers; i++) {
        playerBuckets[i] = (Note **) calloc(WHACKER_OVERFLOW_LIMIT, sizeof(Note*));
    }

    for (int i = 0; i < numNotes; i++) {
        Note* note = assignment->noteArray[i];
        int player = note->player;
        assignment->pScore += addToBucket(playerBuckets[player], note);
    }
}

/**
 * @brief Compares two assignments by playability score
 * 
 * @param a The first assignment
 * @param b The second assignment
 * 
 * @returns 1 if a > b, -1 if a < b, 0 if a == b
*/
int compareAssignments(const void *a, const void *b) {
    Assignment *assignmentA = *(Assignment **)a;
    Assignment *assignmentB = *(Assignment **)b;

    // Sort in descending order (highest to lowest)
    if (assignmentA->pScore > assignmentB->pScore) {
        return 1;
    } else if (assignmentA->pScore < assignmentB->pScore) {
        return -1;
    } else {
        return 0;
    }
}

/**
 * @brief Creates a new assignment of notes to players
 * 
 * Uses the assignments of the parents to create a new assignment for a child. In this, 
 * assignments are prone to mutation.
 * Notes can be assigned in a few different ways:
 * - A note assignment is randomly chosen from any of the parents
 * - A note assignment is completely randomized to any player (mutation)
 * - Conflicting notes are offloaded to another player (mutation)
 * 
 * @param parents The array of parents
 * @param child The child assignment
 * 
 * @return None
*/
void birth(Assignment** parents, Assignment* child) {
    // TODO: implement and make a multithreaded version
}

/**
 * @brief Breeds the population
 * 
 * Creates a new generation of assignments from the most fit parents of the old generation.
 * 
 * @param assignments The array of assignments
 * 
 * @return None
*/
void breed(Assignment** assignments) {
    // Calculate playability score of each assignment
    for (int i = 0; i < populationSize; i++) {
        updatePScore(assignments[i]); // TODO: multithread
    }
    // Sort the assignments by playability score
    qsort(assignments, populationSize, sizeof(Assignment *), &compareAssignments);
    // Create deep copies of numParents assignments
    Assignment** parents = (Assignment**) calloc(numParents, sizeof(Assignment*));
    Assignment* prnts = (Assignment*) calloc(numParents, sizeof(Assignment));
    for (int i = 0; i < numParents; i++) {
        parents[i] = &prnts[i];
        copyAssignment(assignments[i], parents[i]);
    }
    // For each assignment, for each whacker, 
    // choose a random parent to take their note data from (player assignment, capped status, proximate status)
    // TODO: multithread the assignments of the children
    // TODO: implement mutation rates
    // TODO: decrement pScore of assignment for each conflicting note satisfied, or just recalculate for testing
    for (int i = keepParents; i < populationSize; i++) {
        Assignment* assignment = assignments[i];
        for (int j = 0; j < NUM_UNIQUE_PITCHES; j++) {
            int mutate = rand() % 100 == 0;
            if (mutate < 20) {
                // Select a random player assignment for this note
                int randPlayer = rand() % numPlayers;
                Note* cur = assignment->whackerTable[i];
                while (cur != NULL) {
                    cur->player = randPlayer;
                    cur = cur->next;
                }
            } else {
                int randParent = rand() % numParents;
                Assignment* parent = parents[randParent];
                // Given a random parent, copy the player assignments for this note
                Note* cur = assignment->whackerTable[j];
                Note* parentCur = parent->whackerTable[j];
                while (cur != NULL) {
                    cur->player = parentCur->player;
                    cur->capped = parentCur->capped;
                    cur->proximate = parentCur->proximate;
                    cur = cur->next;
                    parentCur = parentCur->next;
                }
            }
        }
    }
    // Free parents_tmp memory
    for (int i = 0; i < numParents; i++) {
        freeAssignment(parents[i]);
    }
    free(parents);
}

/**
 * @brief Initializes a population of assignments with the given size and number of players.
 * 
 * @param assignments A pointer to an array of Assignment pointers to be initialized.
 * @param init The initial assignment to use as a starting point.
 * 
 * @return None
 */
void initPopulation(Assignment** assignments, Assignment* init) {
    // Allocate/copy note and whacker tables to all other assignments in the array
    Assignment* assnp = calloc(populationSize, sizeof(Assignment));
    for (int i = 0; i < populationSize; i++) {
        assignments[i] = &assnp[i];
        copyAssignment(init, &assnp[i]);

        Assignment assignment = assnp[i];
        // Assign random player values to each whacker
        for (int i = 0; i < NUM_UNIQUE_PITCHES; i++) {
            int randPlayer = rand() % numPlayers;
            Note* cur = assignment.whackerTable[i];
            while (cur != NULL) {
                cur->player = randPlayer;
                cur = cur->next;
            }
        }
    }
    freeAssignment(init); // Free the initial assignment
}

/**
 * @brief Generates a set of assignments based on the given parameters and initial assignment.
 * 
 * @param init The initial assignment to use as a starting point.
 * 
 * @return A pointer to an array of Assignment pointers, representing the most fit assignment generated.
 */
Assignment* generateAssignments(Assignment* init) {
    Assignment** assignments = (Assignment**) calloc(populationSize, sizeof(Assignment*));
    initPopulation(assignments, init); 
    for (int i = 0; i < numGens; i++) {
        breed(assignments);
    }
    int returnIndex = 0; // Will hold the assignment with the lowest pScore
    Assignment* final = malloc(sizeof(Assignment));
    copyAssignment(assignments[returnIndex], final);
    for (int i = 0; i < populationSize; i++) {
        freeAssignment(assignments[i]);
    }
    free(assignments);
    return final;
}

/**
 * @brief Generates an integer array of player assignments to each note
 * 
 * @param noteArray The array of notes
 * 
 * @return An integer array of player values
*/
int* getPlayerValues(Note** noteArray) {
    int* playerValues = (int*) calloc(numNotes, sizeof(int)); // TODO: Free memory

    // Initialize the array with player values
    for (int i = 0; i < numNotes; i++) {
        playerValues[i] = noteArray[i]->player;
    }
    return playerValues;
}

/**
 * @brief Assigns player values to the notes
 * 
 * Processes the serialized data and returns the player values of the most genetically fit assignment.
 * 
 * @param param_data The serialized int array
 * @param rate_data The serialized double array
 * @param midi_data The serialized array of midi values
 * @param time_data The serialized array of time values
 * @param numParams The number of parameters for the model
 * @param numRates The number of rates for the model
 * @param numNotes The number of notes
 * 
 * @pre: param_data, rate_data, midi_data, and time_data are initialized and of the correct size
 * 
 * @return An int array of player assignments to each note
*/
int* assign(void* param_data, void* rate_data, void* midi_data, void* time_data,
            int numParams, int numRates, int n) {
    srand(time(NULL)); // Initialize random number generator
    // srand(1); // Seeded RNG for testing purposes

    // Initialize variables
    int* params = (int*) param_data; // Deserialize the int array
    double* rates = (double*) rate_data; // Deserialize the double array
    numGens = params[0];
    populationSize = params[1];
    numPlayers = params[2];
    numParents = params[3];
    keepParents = params[4];
    allowProximates = params[5];
    WHACKER_OVERFLOW_LIMIT = params[6] + 1;
    whackersPerPitch = params[7];
    mutationRate = rates[0];
    offloadRate = rates[1];
    switchTime = rates[2] + 0.001; // Add 0.001 to avoid floating point errors
    numNotes = n;

    numGens = 10; // TODO: remove testing code
    
    // Deserialize the Note array
    int* midis = (int*) midi_data;
    double* times = (double*) time_data;

    Assignment* init = malloc(sizeof(Assignment));
    init->noteArray = generateNoteArray(midis, times);
    init->whackerTable = generateWhackerTable(init->noteArray);
    init->whackerStatuses = generateWhackerStatuses(init->whackerTable); // TODO: handle whacker statuses
    init->pScore = 0;
    Assignment* final = generateAssignments(init);
    printf("Final pScore: %d\n", final->pScore);
    int* playerValues = getPlayerValues(final->noteArray);
    freeAssignment(final);
    return playerValues; // Return an array of player assignments for each note
}

/**
 * @brief Main function
 * 
 * @return 0 on success, 1 on failure
*/
int main() {
    FILE* file = fopen("data.bin", "rb");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }
    int p, r, n;
    fread(&p, sizeof(int), 1, file);  // Read int p
    fread(&r, sizeof(int), 1, file);  // Read int r
    fread(&n, sizeof(int), 1, file);  // Read int n

    int* params = (int*)malloc(p * sizeof(int));
    double* rates = (double*)malloc(r * sizeof(double));
    int* midis = (int*)malloc(n * sizeof(int));
    double* times = (double*)malloc(n * sizeof(double));
    fread(params, sizeof(int), p, file);  // Read int array p
    fread(rates, sizeof(double), r, file);  // Read double array r
    fread(midis, sizeof(int), n, file);  // Read int array n
    fread(times, sizeof(double), n, file);  // Read double array

    assign(params, rates, midis, times, p, r, n);

    // Free allocated memory and close the file
    free(params);
    free(rates);
    free(midis);
    free(times);
    fclose(file);
    return 0;
}

// TODO: create threads beforehand so that we dont have to create new ones for each generation