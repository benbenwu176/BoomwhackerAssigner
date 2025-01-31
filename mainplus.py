import ms3
import numpy as np
import pandas as pd
import struct
import subprocess
from fractions import Fraction

filename = 'spook'
score = ms3.Score(f'./data/{filename}.mscz').mscx
score_bs4 = score.parsed

# red, green, blue, purple, orange, light blue, pink, brown, black
colors = ['#FF0000', '#00AA00', '#0000FF', '#8000D0', '#EE8000', '#00A0FF', '#FF90FF', '#AA5500', '#000000']
# TODO: Add more colors

notes_df = score.notes()
chords_df = score.chords()
measures_df = score.measures()
# chords_df.to_csv('./data/chords.tsv', sep='\t', index=False) # for debugging

chords_df['quarterbeats'] = chords_df['quarterbeats'].apply(float)
chords_df.sort_values(by=['quarterbeats'], inplace=True)
chords_df['quarterbeats'] = chords_df['quarterbeats'].apply(Fraction)

def compute_chord_timings():
    # Initialize variables for time calculation
    current_time = Fraction(0) # start at the beginning of the piece
    current_qpm = 120 # default to 120
    SECONDS_PER_MINUTE = 60 # seconds per minute do some math
    WHOLE_NOTE_DURATION = 4 # quarter notes per whole note
    # Create new fields in the dataframe
    measures_df['time_start'] = Fraction(0)
    measures_df['time_end'] = Fraction(0)
    chords_df['time'] = Fraction(0)
    # Initialize chord iterator
    chord_iter = chords_df.iterrows()
    has_next = True
    
    try: # bad code here this makes you try/except N times, N = # of chords
        i, c = next(chord_iter)
    except StopIteration:
        has_next = False
    # Iterate over measures
    for index, row in measures_df.iterrows():
        # Set measure start time
        if (index == 0): # Beginning of the piece if first measure
            measures_df.at[index, 'time_start'] = Fraction(0)
        else: # End time of previous measure if not first
            measures_df.at[index, 'time_start'] = measures_df.iloc[index - 1]['time_end']

        remaining_duration = Fraction(row['duration_qb'])
        while c['mc'] == row['mc'] and has_next:
            if c['event'] == 'Chord': # Calculate time offset from measure start time and update chord times
                mc_qb_onset = c['mc_onset'] * WHOLE_NOTE_DURATION
                # current time = measure start time + time offset (normalized to quarter notes)
                time_offset = Fraction(Fraction(SECONDS_PER_MINUTE / current_qpm) * mc_qb_onset)
                current_time = measures_df.iloc[index]['time_start'] + time_offset
                remaining_duration = row['duration_qb'] - mc_qb_onset
                chords_df.at[i, 'time'] = current_time
            elif c['event'] == 'Tempo': # update current tempo
                current_qpm = c['qpm']
            try: 
                i, c = next(chord_iter)
            except StopIteration:
                has_next = False
        # set measure end time and update current time
        measures_df.at[index, 'time_end'] = Fraction(current_time + Fraction(SECONDS_PER_MINUTE / current_qpm) * remaining_duration)
        current_time = measures_df.iloc[index]['time_end']

def update_note_timings():
    # Update note timings to match the timing of the chord they belong to
    notes_df['time'] = Fraction(0) # initialize time field
    for index, row in notes_df.iterrows():
        note_chord_id = row['chord_id']
        chord_time = chords_df.loc[chords_df['chord_id'] == note_chord_id, 'time'].values[0]
        notes_df.at[index, 'time'] = chord_time

def time_init():
    compute_chord_timings()
    update_note_timings()

def print_params(pvalues, note_pitches, note_times, num_elements):
    print(pvalues)
    print(note_pitches)
    for time in note_times:
        print(time, end = ", ")
    print(len(pvalues))
    print(num_elements)
    
# Run the genetic algorithm to get assignments
def gen():
    params = {
        'generations': 10000, # number of generations to run the genetic algorithm for
        'population_size': 12, # number of subjects to run the genetic algorithm on
        'num_players': 5, # number of players in the ensemble
        'num_parents': 3, # number of parents to use in the genetic algorithm
        'keep_parents': 1, # functions as a boolean and quantitative number of parents to keep, must be <= num_parents
        'allow_proximates': 1, # whether to allow offloading to nearby players
        'max_whackers': 2, # maximum number of whackers someone can play at once
        'whackers_per_pitch': 2 # the number of whackers available for each pitch, we are poor so we have 2
    }
    rates = {
        'mutation_rate': 0.10, # placeholder rate for random note assignment
        'offload_rate': 0.33, # chance to offload a conflicting note to another player
        'switch_time': 2.0, # time, in seconds, it takes a player to switch boomwhackers
    }
    pvalues = list(params.values())
    rvalues = list(rates.values())
    note_pitches = notes_df['midi'].values
    note_times = notes_df['time'].apply(float).values
    num_elements = len(notes_df)
    
    # Create a byte stream of # of notes, params, rates then add note pitches and times
    stream = struct.pack('i', num_elements) + struct.pack('i'*len(pvalues), *pvalues) + struct.pack('d'*len(rvalues), *rvalues)
    
    proc = subprocess.run(['./genplus.exe'], input = stream, check = True, capture_output=True)
    # IMPORTANT: Maintain this struct format with the corresponding Note struct in genplus.h
    note_struct_format = "dPiiii???"
    
    
    
# recolor the notes with the generated assignments
def recolor():
    # Only relevant in rare edge case of bad writing where whacker notes are written longer than they should be and on top of each other
    STUPID_PEOPLE_BUF = 0.001
    for index, row in notes_df.iterrows():
        score_bs4.color_notes(row['mc'], row['mc_onset'], row['mc'], row['mc_onset'] + STUPID_PEOPLE_BUF,
                              midi=[row['midi']], color_html = colors[row['player']])
    score.store_score(f'./data/{filename}.mscx')
    

time_init()
gen()
# recolor() # IMPORTANT: Notes that are already colored in the original mscz file will NOT be recolored.