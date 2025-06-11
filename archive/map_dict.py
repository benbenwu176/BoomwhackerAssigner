import pandas as pd

df = pd.readcsv('notes.csv')
numPlayers = df["player"].max()+1
players = [[] for  in range(numPlayers)]
midiNotes = ["C","Db","D","Eb","E","F","Gb","G","Ab","A","Bb","B"]

def noteToString(midi, capped):
    string= ""
    if midi >= 36 and midi < 60:
        string+= "Low "
    elif midi >= 60 and midi < 72:
        string+= "Mid "
    else:
        string+= "High "
    string+= midiNotes[midi%12]
    if capped:
        string+= "*"
    return string

for index, row in df.iterrows():
    playerIndex = row["player"]
    midi = row["midi"]
    capped = row["capped"]
    note = noteToString(midi,capped)
    if note not in players[playerIndex]:
        players[playerIndex].append(note)

for index, player in enumerate(players):
    string= "Player " + str(index) + ": "
    for i in range(len(player)-1):
        string+= player[i] + ", "
    if len(player) >= 1:
        string += player[len(player)-1]
    print(string) 