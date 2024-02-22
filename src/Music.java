/**
 * A class containing particularly useful methods that translate musical notation
 * into usable data that can be parsed.
 */
public class Music {
    /**
     * Returns a string of the actual note that corresponds to this pitch
     * @param pitch the pitch of the note
     * @return the note that corresponds to this pitch
     */
    public static String pitchToNote(int pitch) {
        StringBuilder s = new StringBuilder();
        // Determines the octave of the boomwhacker
        if (pitch < 48) {
            s.append("Capped* low ");
        } else if (pitch < 60) {
            s.append("Low ");
        } else if (pitch < 72) {
            s.append("Middle ");
        } else {
            s.append("High ");
        }
        // Determines the tone of the boomwhacker
        switch (pitch % 12) {
            case 0:
                s.append("C");
                break;
            case 1:
                s.append("C#");
                break;
            case 2:
                s.append("D");
                break;
            case 3:
                s.append("D#");
                break;
            case 4:
                s.append("E");
                break;
            case 5:
                s.append("F");
                break;
            case 6:
                s.append("F#");
                break;
            case 7:
                s.append("G");
                break;
            case 8:
                s.append("G#");
                break;
            case 9:
                s.append("A");
                break;
            case 10:
                s.append("A#");
                break;
            case 11:
                s.append("B");
                break;
            default:
                break;
        }
        return s.toString();
    }

    /**
     * Returns a double representing the double representation of a fraction.
     * Precondition: In the context of music, fractions are never negative.
     * @param fraction the fraction to be parsed
     * @return d the double representation of the fraction
     */
    public static double parseFraction(String fraction) {
        String[] f = fraction.split("/");
        if (f.length == 1) {
            return Double.parseDouble(f[0]);
        } else {
            return Double.parseDouble(f[0]) / Double.parseDouble(f[1]);
        }
    }
}