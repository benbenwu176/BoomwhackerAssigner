import java.util.ArrayList;
import java.util.Objects;

public class Boomwhacker implements Comparable<Boomwhacker> {
    // The pitch, numerically, of this note.
    // Middle C = 60, half-steps are +- 1, and octaves are +- 12 (i.e. High C = 72).
    private int pitch;

    // If the boomwhacker is capped or not
    private boolean capped;

    // A list of edges containing the time differential between playing this boomwhacker and another
    private ArrayList<Edge> edges;

    /**
     * Creates a Note instance with pitch only. Used to represent boomwhackers
     * in note assignment.
     * @param pitch the pitch of the note
     */
    public Boomwhacker (int pitch) {
        this.pitch = pitch;
        capped = pitch < 48;
        edges = new ArrayList<>();
    }

    /**
     * Returns the pitch of this note.
     * @return pitch
     */
    public int getPitch() {
        return pitch;
    }

    /**
     * Adds an edge to this boomwhacker
     */
    public void addEdge(Edge e) {
        edges.add(e);
    }

    /**
     * Retuns the edge associated with the given boomwhacker
     * @param bw the boomwhacker that the edge is connected to
     * @return the edge between these two boomwhackers
     */
    public Edge getEdge(Boomwhacker bw) {
        if (bw == null) {
            throw new IllegalArgumentException("Boomwhacker cannot be null");
        }
        for (Edge e : edges) {
            if (e.getBw().equals(bw)) {
                // Equal boomwhackers found
                return e;
            }
        }
        // Edge not found
        return null;
    }

    /**
     * Returns the list of edges generated by the parser
     * @return the list of edges generated by the parser
     */
    public ArrayList<Edge> getEdges() {
        return edges;
    }

    /**
     * Returns if two boomwhackers are equal to each other - that is, their pitches are
     * the same.
     * @param o the (boomwhacker) object to be compared
     * @return if two boomwhackers have the same pitch
     */
    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof Boomwhacker that)) return false;
        return getPitch() == that.getPitch();
    }

    /**
     * Compares the pitch of this note to that of another. Higher notes will have greater
     * value than lower notes.
     * @param n the object to be compared.
     * @return
     */
    @Override
    public int compareTo(Boomwhacker n) {
        if (n == null) {
            throw new IllegalArgumentException("Note cannot be null.");
        }
        return pitch - n.getPitch();
    }

    /**
     * Returns the hashCode of a boomwhacker based on its pitch
     * @return
     */
    @Override
    public int hashCode() {
        return Objects.hash(pitch);
    }

    /**
     * Returns the note that this pitch correlates to
     * @return the note of this pitch
     */
    @Override
    public String toString() {
        return Music.pitchToNote(pitch);
    }
}