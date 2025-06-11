import os
import ms3
import warnings
warnings.simplefilter(action='ignore', category=FutureWarning)

filename = 'humu'
score = ms3.Score(f'./data/{filename}.mscz').mscx  
score_bs4 = score.parsed

notes_df = score.notes()
colors = []

def rgb2html(r, g, b):
  return '#{:02x}{:02x}{:02x}'.format(int(r), int(g), int(b))

# Look for all player colors used in the piece
for mc, staves in score_bs4.tags.items():
  for staff, voices in staves.items():
    for voice, onsets in voices.items():
      for onset, tag_dicts in onsets.items():
        for tag_dict in tag_dicts:
          if tag_dict['name'] != 'Chord':
            continue
          for note_tag in tag_dict['tag'].find_all("Note"):
            # eid = note_tag.find("eid")
            HTML_COLOR = ""
            if note_tag.color is None:
              HTML_COLOR = '#000000' # default to black if no color is specified
            else:
              rgb = note_tag.color.attrs
              HTML_COLOR = rgb2html(rgb['r'], rgb['g'], rgb['b'])
            if HTML_COLOR not in colors:
              colors.append(HTML_COLOR)
       
num_parts = len(colors)
scores = []
for i in range(num_parts):
  s = ms3.Score(f'./data/{filename}.mscz').mscx
  s_bs4 = s.parsed
  scores.append(s_bs4)

# Iterate through the XML tree for each player to isolate their color
# Right now, it makes notes that aren't the current color invisible and creates
# a new mscx file for each isolated part.
for i in range(num_parts):
  for mc, staves in scores[i].tags.items():
    for staff, voices in staves.items():
      for voice, onsets in voices.items():
        for onset, tag_dicts in onsets.items():
          for tag_dict in tag_dicts:
            if tag_dict['name'] != 'Chord':
              continue
            for j in range(len(tag_dict['tag'].contents)):
              if tag_dict['tag'].contents[j].name == 'Note':
                note_tag = tag_dict['tag'].contents[j]
                HTML_COLOR = ""
                if note_tag.color is None:
                  HTML_COLOR = '#000000'
                else:
                  rgb = note_tag.color.attrs
                  HTML_COLOR = rgb2html(rgb['r'], rgb['g'], rgb['b'])
                if HTML_COLOR != colors[i]:
                  # Add invisible tag to note
                  scores[i].new_tag('visible', value=0, before=note_tag.find())
                  # Attempt to completely remove the note tag from the XML file, does not work well
                  # tag_dict['tag'].contents[j].replace_with('') 
                  """
                  # attempt to replace note with rest, does not work
                  if tag_dict['tag'].find("Note") is None:
                    tag_dict['name'] = "Rest"
                    tag_dict['tag']['name'] = "Rest"
                    continue
                  """
            

directory_name = f'./data/{filename}_colored_parts'

try:
  os.mkdir(directory_name)
except FileExistsError:
  print(f"Directory '{directory_name}' already exists.")
except PermissionError:
  print(f"Permission denied: Unable to create '{directory_name}'.")
  exit(1)
except Exception as e:
  print(f"An error occurred: {e}")
  exit(1)

for i in range(num_parts):
  scores[i].store_score(f'{directory_name}/{filename}_{i}.mscx')
  print(f"Stored score {i} with color {colors[i]} in '{directory_name}' directory.")
