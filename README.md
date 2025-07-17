Boomwhacker Assigner
Authored by Ben Wu for UT-Austin's Society of Unconventional Drummers (Texas SOUnD)
Supplemental programs created by: Santiago Marroquin

This program assigns and colors Boomwhacker parts to an ensemble of players. 
It outputs a Musescore file and a dictionary mapping the players to each of their boomwhackers
TODO: It will eventually output a seating order based on a flattened adjacency graph that represents when players need to 'borrow' each others' notes.

TODO: Eventually, this program will accept scores that have already been assigned, either in the form of a CSV configuration file or a previously
colored Musescore file. Then, it will output a dictionary and allow for changes to be made.

Configurable Settings:
File: The Musescore file to be colored. The program will consider ONLY a singular piano part, as determined by convention.
Players: The number of players in the ensemble.
Hold limit: The number of boomwhackers someone can hold at once. Can be specified per player. (Default 2)
Whackers per pitch: The number of boomwhackers for each pitch. Can be specified per pitch. Ex. 4 Low G's. (Default 2)
Switch time (in seconds): The time it takes for a player to switch notes. Can be specified per player. (Default 2)
TODO: Caps: The number of caps available.

Tip: The difficulty of a part can generally be controlled by altering the hold limit and switch time of a player. 
A higher switch time means that a person will tend to have less switches, and changing the hold limit to one can force the player
to only have to play one boomwhacker at a time, with 'switch_time' seconds until their next played note. The inverse also applies.

How it works:
This program does nothing smart. It simply colors as a human would, just with a faster memory and higher levels of recursion for offloading.
Essentially, this is treated similar to a dynamic programming problem with greedy option picking and exchanges of optimal solutions. It assigns each note sequentially, considering the best option for who should receive it.

That being said, the steps it takes are given as follows.
1. Attempts to assign it to players in the order of how recently they have played it. The note is not assigned to them if doing so would cause a conflict while switching boomwhackers.
2. Assuming that one of the players who has previously played this note has a conflict with the note to be assigned, it attempts to offload one of the conflicting notes to another player.
3. Attempts to find a boomwhacker available to allocate to a new, non-conflicting player.

If the note was not assigned by any of the attempts, then the note is marked as conflicting and is left uncolored.