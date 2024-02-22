public class Edge implements Comparable<Edge> {
    // A time interval representing a difference of an eighth-note
    final double eighthNoteInterval = 0.5;

    // The interval of time between playing two boomwhackers
    private double timeInterval;

    // The boomwhacker that is connected
    private Boomwhacker bw;

    /**
     * Creates a new Edge with the lowest time interval between playing this and another
     * boomwhacker
     * @param timeInterval the time between playing this boomwhacker and another
     * @param bw the boomwhacker that is played after this
     */
    public Edge(double timeInterval, Boomwhacker bw) {
        this.timeInterval = timeInterval;
        this.bw = bw;
    }

    /**
     * Returns the boomwhacker that is played
     * @return bw
     */
    public Boomwhacker getBw() {
        return bw;
    }

    /**
     * Returns the time between playing this boomwhacker and another
     * @return timeInterval the time between playing notes
     */
    public double getTimeInterval() {
        return timeInterval;
    }

    /**
     * Returns if these two boomwhackers are played in succession. To be successive,
     * The time interval between them must be below an eighth-note interval.
     * @return if these two boomwhackers are played in succession.
     */
    public boolean isSuccessive() {
        return (0 < timeInterval && timeInterval < eighthNoteInterval);
    }

    /**
     * Compares the time interval between two edges
     * @param e the object to be compared.
     * @return the difference between two time intervals
     */
    @Override
    public int compareTo(Edge e) {
        return Double.compare(timeInterval, e.getTimeInterval());
    }

    /**
     * Returns a string containing information about this edge
     * @return a string containing information about this edge
     */
    @Override
    public String toString() {
        return "Edge{" +
                "timeInterval=" + timeInterval +
                ", bw=" + bw +
                '}';
    }
}