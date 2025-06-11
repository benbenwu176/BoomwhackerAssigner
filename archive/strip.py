import os
import ms3
import warnings
warnings.simplefilter(action='ignore', category=FutureWarning)

filename = 'colortest'
score = ms3.Score(f'./BoomwhackerAssigner/data/{filename}.mscz').mscx    
score_bs4 = score.parsed

notes_df = score.notes()

def rgb2html(r, g, b):
    return '#{:02x}{:02x}{:02x}'.format(int(r), int(g), int(b))

for mc, staves in score_bs4.tags.items():
    for staff, voices in staves.items():
        for voice, onsets in voices.items():
            for onset, tag_dicts in onsets.items():
                for tag_dict in tag_dicts:
                    if tag_dict['name'] != 'Chord':
                        continue
                    for note_tag in tag_dict['tag'].find_all("Note"):
                        idx = 0
                        for item in note_tag.contents:
                            if item.name == 'color':
                                del note_tag.contents[idx]
                            idx += 1
                                

output_file = f'./BoomwhackerAssigner/data/{filename}_stripped.mscx'

score.store_score(output_file)
