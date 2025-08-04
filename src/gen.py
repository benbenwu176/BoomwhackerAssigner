import numpy as np
import pandas as pd
import warnings
warnings.simplefilter(action='ignore', category=FutureWarning)
import struct
import subprocess
import time
import os
from fractions import Fraction
from collections import namedtuple
from datetime import datetime
import sys
import ms3

print("Starting...")
args = sys.argv[1:]
tmp_dir = args[0]
score_path = args[1]
params_path = args[2]
gen_path = args[3]

gen_out_path = os.path.join(tmp_dir, 'gen_output.log')
data_out_path = os.path.join(tmp_dir, 'data.bin')
recolor_data_path = os.path.join(tmp_dir, 'recolor_data.bin')
notes_out_path = os.path.join(tmp_dir, 'notes.csv')
chords_out_path = os.path.join(tmp_dir, 'chords.csv')
measures_out_path = os.path.join(tmp_dir, 'measures.csv')

print("Args parsed")

start_time = time.perf_counter()
score = ms3.Score(score_path).mscx
score_bs4 = score.parsed
build_time = time.perf_counter()
show_conflicting = True

colors = ['#FF0000', '#006622', '#0000FF', "#720AD3", '#EE8000', '#00A0FF', '#FF90FF', '#AA5500', '#33DD00']

notes_df = score.notes()
chords_df = score.chords()
measures_df = score.measures()

notes_df.to_csv(notes_out_path)
measures_df.to_csv(chords_out_path)
measures_df.to_csv(measures_out_path)

print("Score parsed")

def to_fraction(val):
    if pd.isna(val) or val == '':
        return None
    try:
        return Fraction(val)
    except ValueError:
        # Fallback for float strings
        return Fraction(float(val))

chords_df['quarterbeats'] = chords_df['quarterbeats'].apply(float)
chords_df.sort_values(by=['quarterbeats'], inplace=True)
# chords_df['quarterbeats'] = chords_df['quarterbeats'].apply(Fraction)
chords_df['quarterbeats'] = chords_df['quarterbeats'].map(to_fraction)

def get_chord_timings():
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

  # Update note timings to match the timing of the chord they belong to
  notes_df['time'] = Fraction(0) # initialize time field
  for index, row in notes_df.iterrows():
    note_chord_id = row['chord_id']
    chord_time = chords_df.loc[chords_df['chord_id'] == note_chord_id, 'time'].values[0]
    notes_df.at[index, 'time'] = chord_time
  
# Run the genetic algorithm to get assignments
def gen():
  # Write note data to file 'data.bin'
  pitches_arr = notes_df['midi'].to_numpy(dtype=np.int32)
  times_arr = notes_df['time'].to_numpy(dtype=np.float64)
  with open(data_out_path, 'wb') as f:
    f.write(pitches_arr.tobytes(order='C'))
    f.write(times_arr.tobytes(order='C'))

  # Run the assignment generation program
  cmd = [gen_path, tmp_dir, params_path, data_out_path, str(len(notes_df))]
  print(f"Running {cmd}")
  proc = subprocess.run(cmd, capture_output=True)

  # Write the stderr from the subprocess to an output file for debugging
  current_datetime = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
  with open(gen_out_path, 'w') as f:
    f.write(f"Date and Time: {current_datetime}\n")
  with open(gen_out_path, 'ab') as f:
    f.write(proc.stdout.decode('utf-8').encode())
    
  if (proc.returncode != 0):
    raise Exception("Assignment generation failed.")

  # IMPORTANT: Maintain this struct format with written fields in assignment.cpp
  Note = namedtuple('Note', ['time', 'player', 'capped', 'conflicting'])
  note_struct_format = '<di??'
  note_struct_size = struct.calcsize(note_struct_format)
  notes = []

  # Read in note data from recolor data path
  with open(recolor_data_path, 'rb') as f:
    while True:
      chunk = f.read(note_struct_size)
      if (len(chunk) < note_struct_size):
        break
      unpacked = struct.unpack(note_struct_format, chunk)
      notes.append(Note(*unpacked))
  
  return notes

# Recolor the notes with the generated assignments
def recolor(notes):
  # Output assignment configuration to a CSV file
  notes_df['time'] = [note.time for note in notes]
  notes_df['player'] = [note.player for note in notes]
  notes_df['capped'] = [note.capped for note in notes]
  notes_df['conflicting'] = [note.conflicting for note in notes]
  notes_df.to_csv(notes_out_path)

  # Only relevant in rare edge case of bad writing where whacker notes are written longer than they should be and durations overlap
  STUPID_PEOPLE_BUF = 0.001
  num_conflicting = 0
  # Recolor notes to their assigned player
  with open(gen_out_path, 'a') as f:
    for index, row in notes_df.iterrows():
      color = ''
      if (notes[index].player == -1):
        color = "#000000"
      else:
        color = colors[notes[index].player]
      score_bs4.color_notes(row['mc'], row['mc_onset'], row['mc'], row['mc_onset'] + STUPID_PEOPLE_BUF,
                  midi=[row['midi']], color_html = color)
      if (show_conflicting and row['conflicting'] == True):
        num_conflicting += 1
        minutes, seconds = divmod(row['time'], 60)
        f.write(f"Conflicting note {row['midi']} in measure {row['mc']}, time: {int(minutes)}:{seconds:05.2f}\n")
  
  print("Number of conflicts: " + str(num_conflicting))
  # Write recolored assignment to file
  root, ext = os.path.splitext(score_path)
  mscx_path = root + "_colored" + ".mscx"
  score.store_score(mscx_path)

# TODO: create a list of all the colors of each colored note in the piece
# TODO: strip color tags from original musescore
# TODO: strip other instruments besides piano from original musescore
# TODO: remove ties
# TODO: remove trills (see macabre.mscx)
# TODO: count number of caps used
# TODO: fix time calculations to not jump an extra partial when switching time mid measure, and reformat to handle note duration/trills
# TODO: when conflicting, color with optimal player and add cross notehead
# TODO: handle repeats (volta in runaway)
# TODO: Add note heads
# TODO: add optimize for highest global switch time
# TODO: return file config csv filename_timestamp_hash (6 character hash based on filename, timestamp, and crypto rand)
# TODO: MRP creation from time using used whacker binary search indexing
# TODO: Add more colors

get_chord_timings()
chords_time = time.perf_counter()
notes = gen()
if (notes == None):
  exit(1)
gen_time = time.perf_counter()
recolor(notes)
color_time = time.perf_counter()

print(f"MSCX construction time: {build_time - start_time:.6f}")
print(f"Chord computation time: {chords_time - build_time:.6f}")
print(f"Assignment generation time: {gen_time - chords_time:.6f}")
print(f"Recoloring time: {color_time - gen_time:.6f}")