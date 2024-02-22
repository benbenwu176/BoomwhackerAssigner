public class Note implements Comparable<Note> {
    // The pitch, numerically, of this note. Middle C = 60.
    private int pitch;

    // The measure that this note occurs in
    private int measure;

    // The beat that this note occurs on
    private double beat;

    /**
     * Creates a Note with pitch and time. Used to represent a note played in the actual music.
     * @param pitch the pitch of the note
     * @param measure the measure the note is in
     * @param beat the time that the note occurs in the music
     */
    public Note (int pitch, int measure, double beat) {
        this.pitch = pitch;
        this.measure = measure;
        this.beat = beat;
    }

    /**
     * Returns the pitch of this note.
     * @return pitch
     */
    public int getPitch() {
        return pitch;
    }

    /**
     * Returns the measure this note takes place in
     * @return
     */
    public int getMeasure() {
        return measure;
    }

    /**
     * Returns the beat that this note takes place on
     * @return beat
     */
    public double getBeat() {
        return beat;
    }

    public double getTimeInterval(Note n) {
        if (n == null) {
            throw new IllegalArgumentException("Note cannot be null");
        }
        return beat - n.getBeat();
    }

    /**
     * Compares the pitch of this note to that of another. Higher notes will have greater
     * value than lower notes. Lower notes will have "higher priority" than higher notes.
     * @param n the object to be compared.
     * @return a comparison of the two notes' timing
     */
    @Override
    public int compareTo(Note n) {
        if (n == null) {
            throw new IllegalArgumentException("Note cannot be null.");
        }
        return Double.compare(beat, n.getBeat());
    }

    /**
     * Returns the note that this pitch correlates to
     * @return the note of this pitch
     */
    @Override
    public String toString() {
        StringBuilder s = new StringBuilder();
        s.append("Pitch: ").append(Music.pitchToNote(pitch));
        s.append(", beat: ").append(beat);
        s.append(", measure: ").append(measure);
        return s.toString();
    }
}