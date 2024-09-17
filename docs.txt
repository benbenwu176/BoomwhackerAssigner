Group of n parents
A parent is an assignment of boomwhackers
Choose k most fit parents to reproduce
Crossover the boomwhackers (superset of notes) themselves, not necessarily the assignments to players. Subsequent assignment can either be completely random or based on the parents.
Implementing greater genetic diversity can prevent premature convergence to suboptimal solutions
Possibly implement elitism to retain best solutions so far


Parameters:
n number or parents
k most fit parents chosen to reproduce - start with two
Chance of mutation to migrate or bud
Time determined to be "unplayable"; window size when screening
If optimal fitness is not met, number of generations to run the program for


75/25 chance of migrating or budding
migrating - move the assigned boomwhacker to a different player
budding - split off incompatible notes into another whacker that goes to a different player. other player is random?

Positions matter - players have access to the notes of the players surrounding them

Boomwhacker flags:
Borrowed - used in positioning to determine whether a note is real
Capped - used to signify a whacker's usage in budding


Fitness - will be measured inversely; a lower "fitness" score means the individual is more fit.

Will be determined by the total # of instances of unplayable sections of music

Criteria - Fitness of 0; means that every section is playable


Functions of the selection process:
Initialize: Randomly assign boomwhackers in full to players
Screen for unplayable sections
Choose top two (TBD) assignments based on fitness
"Breed", or mix the two assignments. Randomly choose whether to completely randomly assign boomwhackers or to mix and match based on parental input
Repeat until optimal


File Parsing:
Parse notes into boomwhacker objects, maybe use hash table? Handle low capped BWs

Convert time window parameter to beat based on measure number
Use note objects as parsed, and also parse measures for tempo for each measure


Theoretically, coloring sequentially for each note should just work, no need to convert timings as long as arrays match up



Front-end side:
Web UI to input mscz files, parameters, and (optionally) custom player coloring
Run python script to generate a CSV file, TBD to choose what organizational form such as notes or measures
Run assigner with boomwhacker
Choose most optimal assignment, generate player objects with assigned notes
Recolor mscz file based on assignments
(optionally, also generate a file with assignments called bwconfig.txt or something and output that so that assignments can be saved)

Steps:
import ms3
In the directory with the sheet music you want to parse, run the following command:
ms3 extract -N -a
a file with name FILENAME.notes.tsv should be created. take note of this for file input when running the assigner
dependencies: univocity-parsers 2.9.1
