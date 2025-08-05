import os
import ms3
import warnings
warnings.simplefilter(action='ignore', category=FutureWarning)

filename = 'test2.mscx'
archive = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(archive)
score_path = os.path.join(root_dir, 'data', filename)
score = ms3.Score(score_path).mscx
score_bs4 = score.parsed

notes_df = score.notes()
chords_df = score.chords()
notes_df['rolls'] = None

def rgb2html(r, g, b):
    return '#{:02x}{:02x}{:02x}'.format(int(r), int(g), int(b))

id = 0
rolls = []

for mc, staves in score_bs4.tags.items():
    for staff, voices in staves.items():
        for voice, onsets in voices.items():
            for onset, tag_dicts in onsets.items():
                for tag_dict in tag_dicts:
                    if tag_dict['name'] != 'Chord':
                        continue
                    rolled = tag_dict['tag'].find('TremoloSingleChord')
                    for note_tag in tag_dict['tag'].find_all('Note'):
                        rolls.append((id, rolled is not None))
                        idx = 0
                        note_tag.contents[:] = [item for item in note_tag.contents if item.name != 'color']
                        id += 1
                                
notes_df['rolled'] = rolls

print(notes_df)

output_file = os.path.join(root_dir, 'output', filename)

score.store_score(output_file)
