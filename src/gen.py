import ms3
import numpy as np
import warnings
warnings.simplefilter(action='ignore', category=FutureWarning)
import struct
import subprocess
import time
from fractions import Fraction
from collections import namedtuple
from datetime import datetime

print("Starting...")
gen_out_path = 'output\\gen_output.txt'
notes_out_path = 'output\\notes.csv'
chords_out_path = 'output\\chords.csv'
measures_out_path = 'output\\measures.csv'
gen_exe_path = 'build\\gen.exe'

start_time = time.perf_counter()
filename = 'humu'
score = ms3.Score(f'.\\data\\{filename}.mscz').mscx
score_bs4 = score.parsed
build_time = time.perf_counter()
show_conflicting = True

# red, green, blue, purple, orange, light blue, pink, brown, black, TODO: gray?, light green but like different
colors = ['#FF0000', '#006622', '#0000FF', "#720AD3", '#EE8000', '#00A0FF', '#FF90FF', '#AA5500', '#33DD00']
# TODO: Add more colors

notes_df = score.notes()
chords_df = score.chords()
measures_df = score.measures()

chords_df['quarterbeats'] = chords_df['quarterbeats'].apply(float)
chords_df.sort_values(by=['quarterbeats'], inplace=True)
chords_df['quarterbeats'] = chords_df['quarterbeats'].apply(Fraction)

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

def print_params(pvalues, note_pitches, note_times, num_elements):
  print(pvalues)
  print(note_pitches)
  print(note_times)
  for time in note_times:
    print(time, end = ", ")
  print(len(pvalues))
  print(num_elements)
  
# Run the genetic algorithm to get assignments
def gen():
  params = {
    'num_players': 9, # number of players in the ensemble
    'hold_limit': 2, # maximum number of whackers someone can play at once
    'seed': 0, # optional RNG seed
  }
  rates = {
    'switch_time': 2.0, # time, in seconds, it takes a player to switch boomwhackers
  }
  # Convert params, rates, and note times into numpy lists
  pvalues = list(params.values())
  rvalues = np.array(list(rates.values()), dtype = np.float64)
  note_pitches = notes_df['midi'].values
  note_times = np.array([float(f) for f in notes_df['time']])
  n = len(notes_df)
  p = len(pvalues)
  r = len(rvalues)
    
  # Create a byte stream of # of notes, params, rates then add note pitches and times
  data_size = n*4 + n*8 + p*4 + r*8
  padding = 4096 - (data_size) % 4096
  stream_format = ('i' * n) + ('d' * n) + ('i' * p) + ('d' * r) + ('x' * padding)
  stream = struct.pack(stream_format, *note_pitches, *note_times, *pvalues, *rvalues)
  
  args = [str(n), str(p), str(r)]
  exe = [gen_exe_path, *args]
  cmd = exe
  # Run the assignment generation program
  proc = subprocess.run(cmd, input = stream, capture_output=True, shell=True) # Add back check = True
  print(f"Number of notes: {n:d}")
  # Get the current date and time
  current_datetime = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

  # Write the stderr from the subprocess to an output file for debugging
  with open(gen_out_path, 'w') as f:
    f.write(f"Date and Time: {current_datetime}\n")
  with open(gen_out_path, 'ab') as f:
    f.write(proc.stderr.decode('utf-8').encode())
    
  if (proc.returncode != 0):
    raise Exception("Assignment generation failed.")

  # IMPORTANT: Maintain this struct format with the corresponding Note struct in gen.h
  Note = namedtuple('Note', ['time', 'whacker', 'id', 'player', 'pitch', 'capped', 'proximate', 'conflicting'])
  note_struct_format = "dPiii???x" # Double, pointer, integer, bool, padding
  note_struct_size = struct.calcsize(note_struct_format)
  note_struct_parser = struct.Struct(note_struct_format)
  notes = []
  note_data = proc.stdout
  if (len(note_data) == 0):
    raise Exception("No data in stdout.")
  
  # Read num of conflicting notes from the first 4 bytes of the output
  num_conflicting_notes = struct.unpack('i', note_data[:4])[0]
  print("Conflicts: " + str(num_conflicting_notes))
  # Read flattened array of player indexes from the next 4 * num_players bytes of the output
  player_indexes = np.frombuffer(note_data[4:4 + 4 * params['num_players']], dtype=np.int32)
  # Start reading note data from the 4 * num_players + 4 bytes offset
  byte_offset = 4 + 4 * params['num_players']
  for offset in range(byte_offset ,len(proc.stdout), note_struct_size):
    chunk = note_data[offset : offset + note_struct_size]
    if (len(chunk) < note_struct_size):
      # End of data, just padding here
      break
    # Unpack the data into a tuple of Python values
    unpacked = note_struct_parser.unpack(chunk)
    notes.append(Note(*unpacked))
  
  return notes

  
# Recolor the notes with the generated assignments. IMPORTANT: Notes that are already colored in the original mscz file will NOT be recolored.
def recolor(notes):
  # Output assignment configuration to a CSV file
  notes_df['time'] = [note.time for note in notes]
  notes_df['player'] = [note.player for note in notes]
  notes_df['capped'] = [note.capped for note in notes]
  notes_df['conflicting'] = [note.conflicting for note in notes]
  notes_df.to_csv(notes_out_path)
  # measures_df.to_csv(chords_out_path)
  # measures_df.to_csv(measures_out_path)

  # Only relevant in rare edge case of bad writing where whacker notes are written longer than they should be and durations overlap
  STUPID_PEOPLE_BUF = 0.001
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
        minutes, seconds = divmod(row['time'], 60)
        f.write(f"Conflicting note in measure {row['mc']}, time: {int(minutes)}:{seconds:05.2f}\n")
  
  # Write recolored assignment to file
  score.store_score(f'./data/{filename}.mscx')
  

# TODO: create a list of all the colors of each colored note in the piece
# TODO: strip color tags from original musescore
# TODO: strip other instruments besides piano from original musescore
# TODO: remove ties
# TODO: remove trills (see macabre.mscx)
# TODO: count number of caps used
# TODO: fix time calculations to not jump an extra partial when switching time mid measure, and reformat to handle note duration/trills
# TODO: when conflicting, color with optimal player and add cross notehead

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
# TODO: Add note heads