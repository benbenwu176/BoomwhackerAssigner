import com.univocity.parsers.common.record.Record;
import com.univocity.parsers.tsv.*;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.PriorityQueue;
import java.util.Scanner;
import java.util.Set;
import java.util.TreeMap;

public class MsczReader {
    public static void main(String[] args) throws IOException {
        // Sets the line separator character to \n
        TsvParserSettings settings = new TsvParserSettings();
        settings.getFormat().setLineSeparator("\n");

        // creates a TSV parser
        TsvParser msParser = new TsvParser(settings);

        // Retrieves the TSV file of notes
        String inputTag = "Input the name of your note tsv file: ";
        String fileName = getFileName(inputTag);
        // test.notes.tsv
        // 1.1-MarioMedley.notes.tsv
        File noteFile = new File("./notes/" + fileName);

        // Parses the file of notes and creates a list of records
        List<Record> fileNotes = msParser.parseAllRecords(noteFile);
        fileNotes.remove(0); // Removes the first row of labels

        // Creates a TreeMap containing a Boomwhacker and all instances in which it is played
        TreeMap<Boomwhacker, ArrayList<Note>> bwMap = generateBoomwhackerMap(fileNotes);
        generateEdges(bwMap);

        // Assigns the boomwhackers to players
        int numPlayers = 8;
        Assigner asnr = new Assigner(numPlayers);
        asnr.assignWhackers(new PriorityQueue<>(bwMap.keySet()));
        System.out.println(asnr);
    }

    // Retrieves the name of a file from user input
    private static String getFileName(String inputTag) {
        Scanner input = new Scanner(System.in);
        System.out.print(inputTag);
        String fileName = input.nextLine();
        System.out.println();
        input.close();
        return fileName;
    }

    /*
     * Creates a TreeMap containing each unique note played in the piece, represented as
     * boomwhacker keys, and every instance that each of those boomwhackers is played in
     * a value of ArrayLists of notes.
     */
    private static TreeMap<Boomwhacker, ArrayList<Note>> generateBoomwhackerMap(List<Record> fileNotes) {
        TreeMap<Boomwhacker, ArrayList<Note>> bwMap = new TreeMap<>();
        for (Record fileNote : fileNotes) {
            int pitch = fileNote.getInt("midi");
            if (pitch == 81) {
                // TODO: this is a temporary fix to not process the high A in the main mario theme
                continue;
            }
            int measure = fileNote.getInt("mc");
            double beat = Music.parseFraction(fileNote.getString("quarterbeats"));
            Note sheetNote = new Note(pitch, measure, beat);
            Boomwhacker bw = new Boomwhacker(pitch);
            if (!bwMap.containsKey(bw)) {
                // Inserts a boomwhacker entry of an ArrayList of all times it's played
                ArrayList<Note> bwNotes = new ArrayList<>();
                bwNotes.add(sheetNote);
                bwMap.put(bw, bwNotes);
            } else {
                // Adds an instance of a note to a boomwhacker entry if already in the map
                bwMap.get(bw).add(sheetNote);
            }
        }
        return bwMap;
    }

    /*
     * Updates each boomwhacker in the map with the lowest time interval between playing
     * two boomwhackers
     */
    private static void generateEdges(TreeMap<Boomwhacker, ArrayList<Note>> bwMap) {
        Set<Boomwhacker> whackers = bwMap.keySet();
        for (Boomwhacker whacker : whackers) {
            // The list of notes of the current boomwhacker
            ArrayList<Note> curNotes = bwMap.get(whacker);
            for (Boomwhacker other : whackers) {
                if (!whacker.equals(other)) {
                    // Generates edges between notes for every other whacker besides the current.
                    ArrayList<Note> otherNotes = bwMap.get(other);
                    double minInterval = Double.POSITIVE_INFINITY;
                    for (int i = 0; i < curNotes.size(); i++) {
                        Note curNote = curNotes.get(i);
                        for (int j = 0; j < otherNotes.size(); j++) {
                            // Replaces minInterval with the current interval between notes if less
                            Note otherNote = otherNotes.get(j);
                            double interval = curNote.getTimeInterval(otherNote);
                            // TODO: Implement directionality with the sign of the interval
                            double tempInterval = Math.abs(interval);
                            if (tempInterval < minInterval && tempInterval > 0) {
                                minInterval = tempInterval;
                            }
                        }
                    }
                    whacker.addEdge(new Edge(minInterval, other));
                    other.addEdge(new Edge(minInterval, whacker));
                }
            }
        }
    }
}