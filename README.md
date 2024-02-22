Instructions on how to use:
1. Import ms3, the python Musescore parsing file. (https://ms3.readthedocs.io/en/latest/) This can be done with the command:
python3 -m pip install ms3
import ms3
in a command terminal.

2. Put your sheet music Musescore (.mscz) files into the sheetmusic directory. Using the ms3 library, open a command terminal in sheetmusic and run the command:
ms3 extract -N -a
This will extract notes regardless of whether there is metadata into a file called notes, by default is ../notes but can be changed.

3. In the notes folder, you will now find a (or several) tsv files. Take note of the file that corresponds to the sheet music that you want to parse.
  
4. MsczReader.java is the main file. Run it and input the name of the tsv file that you just extracted the notes from. (ex. test.notes.tsv)
