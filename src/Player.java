import java.util.ArrayList;
import java.util.PriorityQueue;
import java.util.Stack;

public class Player implements Comparable<Player> {
    // The maximum number of boomwhackers a player shall have
    private final int NOTE_LIMIT = 8;

    // The name of this Player
    private String name;

    // The boomwhackers this player will play
    private Stack<Boomwhacker> boomwhackers;

    /**
     * Initializes a new PLayer object with an empty list of notes
     */
    public Player(String name) {
        this.name = name;
        boomwhackers = new Stack<>();
    }

    /**
     * Attempts to add a note to this player's existing list of notes.
     * If a note was replaced in an attempt to add the original note, returns the
     * replaced note to be assigned. Otherwise, it returns the original note if a simple add
     * was successful or null if the note could not be added.
     * @param bw the note to be added
     * @return the original note, replaced note, or null as described above
     */
    public Boomwhacker addWhacker(Boomwhacker bw) {
//        if (boomwhackers.size() >= NOTE_LIMIT) {
//            return null;
//        }
        boomwhackers.add(bw);
        return bw;
    }

    /**
     * Returns the number of notes this player has
     * @return the number of notes this player has
     */
    public int numWhackers() {
        return boomwhackers.size();
    }

    /**
     * Takes a boomwhacker and returns the edge with the lowest time interval between any of
     * this player's boomwhackers, i.e. the most "restricting" edge.
     * @param bw the boomwhacker to look for edges with
     * @return the most restricting edge associated with this boomwhacker
     */
    public Edge getLowestEdge(Boomwhacker bw) {
        if (bw == null) {
            throw new IllegalArgumentException("Boomwhacker cannot be null");
        }
        double minInterval = Double.POSITIVE_INFINITY;
        Edge minEdge = null;
        // Finds the edge with the lowest time interval in this player's boomwhackers
        for (Boomwhacker other : boomwhackers) {
            Edge otherEdge = bw.getEdge(other);
            if (otherEdge.getTimeInterval() < minInterval) {
                minEdge = otherEdge;
            }
        }
        return minEdge;
    }

    public double getLowestInterval() {
        double minInterval = Double.POSITIVE_INFINITY;
        for (Boomwhacker bw : boomwhackers) {
            for (Boomwhacker other : boomwhackers) {
                if (!bw.equals(other)) {
                    double timeInterval = bw.getEdge(other).getTimeInterval();
                    if (timeInterval < minInterval) {
                        minInterval = timeInterval;
                    }
                }
            }
        }
        return minInterval;
    }

    /**
     * Gets the most recently added boomwhacker to this player's
     * @return A popped boomwhacker
     */
    public Boomwhacker getMostRecentAdded() {
        return boomwhackers.pop();
    }

    /**
     * Compares this Player to that of another. Players with fewer notes than another
     * will be considered "higher priority" than players with more notes.
     * @param p the object to be compared.
     * @return
     */
    @Override
    public int compareTo(Player p) {
        return boomwhackers.size() - p.numWhackers();
    }

    /**
     * Returns a string of all notes assigned to this player
     * @return a string of all notes assigned to this player
     */
    @Override
    public String toString() {
        StringBuilder s = new StringBuilder();
        s.append(name).append("\'s boomwhackers:\n");
        for (Boomwhacker bw : boomwhackers) {
            s.append("Note: ").append(bw.toString()).append('\n');
        }
        s.append("Lowest interval: ").append(getLowestInterval()).append('\n');
        return s.toString();
    }
}