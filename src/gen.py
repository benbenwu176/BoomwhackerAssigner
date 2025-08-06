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
from typing import Optional

print("Starting...")
args = sys.argv[1:]
tmp_dir = args[0]
score_path = args[1]
params_path = args[2]
gen_path = args[3]

root_dir = os.path.dirname(os.path.dirname(__file__))
gen_out_path_debug = os.path.join(root_dir, 'frontend', 'gen_output.log')
gen_out_path = os.path.join(tmp_dir, 'gen_output.log')
data_out_path = os.path.join(tmp_dir, 'data.bin')
recolor_data_path = os.path.join(tmp_dir, 'recolor_data.bin')
notes_out_path = os.path.join(tmp_dir, 'notes.csv')
chords_out_path = os.path.join(tmp_dir, 'chords.csv')
rests_out_path = os.path.join(tmp_dir, 'rests.csv')
measures_out_path = os.path.join(tmp_dir, 'measures.csv')

start_time = time.perf_counter()
score = ms3.Score(score_path).mscx
score_bs4 = score.parsed
build_time = time.perf_counter()
show_conflicting = True

colors = ['#FF0000', '#006622', '#0000FF', "#720AD3", '#EE8000', '#00A0FF', '#FF90FF', '#AA5500', '#33DD00']

notes_df = score.notes()
chords_df = score.chords()
rests_df = score.rests()
measures_df = score.measures()

chords_df.to_csv(chords_out_path)
rests_df.to_csv(rests_out_path)
measures_df.to_csv(measures_out_path)

if 'tremolo' in chords_df.columns:
  chord_roll_dict = {
    chord_id: True
    for chord_id in chords_df.loc[chords_df['tremolo'].notna(), 'chord_id']
  }
else:
  chord_roll_dict = {}
  print("No rolls detected. May need to update bs4_parser.py")

def rgba2html(
  r_str: Optional[str],
  g_str: Optional[str],
  b_str: Optional[str],
  a_str: Optional[str],
) -> Optional[str]:
  """
  From R,G,B,A each as strings '0'–'255', return '#RRGGBB' (if alpha==255)
  or 'rgba(r, g, b, a)' (with a in [0,1]), or None if any input is missing/invalid.
  """
  try:
      r = int(r_str)
      g = int(g_str)
      b = int(b_str)
      a_int = int(a_str)
  except (TypeError, ValueError):
      # one of them was None, nan, or not an integer-string
      return None

  # clamp to valid ranges
  r = max(0, min(255, r))
  g = max(0, min(255, g))
  b = max(0, min(255, b))
  a_int = max(0, min(255, a_int))

  a = a_int / 255.0
  if a_int >= 255:
      return f"#{r:02X}{g:02X}{b:02X}"
  return f"rgba({r}, {g}, {b}, {a:.3f})"

# Append 'color' column to notes_df
notes_df['color'] = notes_df.apply(lambda row: rgba2html(row.get('r'), row.get('g'), row.get('b'), row.get('a')), axis=1)

def process_bs4():
  def iter_tag_dicts():
    for staves in score_bs4.tags.values():
      for voices in staves.values():
        for onsets in voices.values():
          for tag_list in onsets.values():
            yield from tag_list

  for tag_dict in iter_tag_dicts():
    if tag_dict.get('name') != 'Chord':
      continue

    # remove all <color> children inside each <Note>
    bs_tag = tag_dict['tag']
    for note in bs_tag.find_all('Note'):
      for color in note.find_all('color'):
        color.decompose()

def get_chord_timings():
  global notes_df
  global chords_df
  df = chords_df.sort_values('quarterbeats').copy()

  # Make a column that holds the tempo QPM only on Tempo rows, then ffill
  df['tempo_qpm'] = df['qpm'].where(df['event']=='Tempo')
  df['qpm_used']  = df['tempo_qpm'].ffill().fillna(120)

  # Compute the quarterbeat increments
  df['delta_qb'] = df['quarterbeats'].diff().fillna(df['quarterbeats'])

  # Compute the time *increments*
  df['time_inc'] = df['delta_qb'] / df['qpm_used'] * 60

  # Cumulative‐sum → absolute time
  df['time'] = df['time_inc'].cumsum()

  # Only keep time for Chord rows
  df.loc[df['event']!='Chord', 'time'] = None

  # Compute durations in one shot
  # default 0, then for rolled chords use qb‐to‐time formula
  df['dur'] = 0
  mask = (df['event']=='Chord') & df['chord_id'].map(lambda cid: chord_roll_dict.get(cid) is not None)
  df.loc[mask, 'dur'] = df.loc[mask, 'duration_qb'] / df.loc[mask, 'qpm_used'] * 60

  # Drop helper columns
  df.drop(columns=['tempo_qpm','qpm_used','delta_qb','time_inc'], inplace=True)

  # Replace with altered copy and set note times
  chords_df = df
  notes_df = notes_df.merge(
    chords_df[['chord_id','time', 'dur']],
    on='chord_id',
    how='left',
    suffixes=('','_chord')
  )

# Run the algorithm to generate the assignments
def gen():
  # Write note data to file 'data.bin'
  pitches_arr = notes_df['midi'].to_numpy(dtype=np.int32)
  times_arr = notes_df['time'].to_numpy(dtype=np.float64)
  dur_arr = notes_df['dur'].to_numpy(dtype=np.float64)
  with open(data_out_path, 'wb') as f:
    pitches_arr.tofile(f)
    times_arr.tofile(f)
    dur_arr.tofile(f)
    # f.write(pitches_arr.tobytes(order='C'))
    # f.write(times_arr.tobytes(order='C'))
    # f.write(dur_arr.tobytes(order='C'))

  # Run the assignment generation program
  print("Number of notes:" + str(len(notes_df)))
  cmd = [gen_path, tmp_dir, params_path, data_out_path, str(len(notes_df))]
  print(f"Running {cmd}")
  proc = subprocess.run(cmd, capture_output=True)

  # Write the stderr from the subprocess to an output file for debugging
  current_datetime = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
  with open(gen_out_path, 'w') as f:
    f.write(f"Date and Time: {current_datetime}\n")
  with open(gen_out_path, 'ab') as f:
    f.write(proc.stdout.decode('utf-8').encode())
    f.write(proc.stderr.decode('utf-8').encode())

  # Write to local gen_out log for quick debug
  with open(gen_out_path_debug, 'w') as f:
    f.write(f"Date and Time: {current_datetime}\n")
  with open(gen_out_path_debug, 'ab') as f:
    f.write(proc.stdout.decode('utf-8').encode())
    f.write(proc.stderr.decode('utf-8').encode())
    
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

def recolor(notes):
  # 1) Extract all note attributes in one pass
  times       = [n.time        for n in notes]
  players     = [n.player      for n in notes]
  capped      = [n.capped      for n in notes]
  conflicting = [n.conflicting for n in notes]
  mcs     = notes_df['mc'].tolist()
  onsets  = notes_df['mc_onset'].tolist()
  midis   = notes_df['midi'].tolist()

  # 2) Bulk-assign to your DataFrame and write CSV once
  notes_df['time']        = times
  notes_df['player']      = players
  notes_df['capped']      = capped
  notes_df['conflicting'] = conflicting
  notes_df.to_csv(notes_out_path, index=False)

  # 3) Prep locals for the paint loop
  buf           = 0.001
  max_player    = len(colors) - 1
  show_conf     = show_conflicting
  paint         = score_bs4.color_notes
  log_lines     = []
  num_conflicts = 0

  # 4) Iterate over Python data, not the DataFrame
  for i, (p, mc, onset, midi, conf, t) in enumerate(
      zip(players, mcs, onsets, midis, conflicting, times)):
    # pick color
    if p < 0 or p > max_player:
      color = "#000000"
      if p > max_player:
        print("Error: Player index out of range")
    else:
      color = colors[p]

    # paint the note
    paint(mc, onset, mc, onset + buf, midi=[midi], color_html=color)

    # accumulate conflict log
    if show_conf and conf:
      num_conflicts += 1
      m, s = divmod(t, 60)
      log_lines.append(
        f"Conflicting note {midi} in measure {mc}, "
        f"time: {int(m)}:{s:05.2f}"
      )

  # 5) write all logs at once
  if log_lines:
    with open(gen_out_path, 'a') as f:
      f.write("\n".join(log_lines) + "\n")

  print("Number of conflicts:", num_conflicts)

  # 6) save recolored score
  root, _ = os.path.splitext(score_path)
  score.store_score(f"{root}_colored.mscx")

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

process_bs4()
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