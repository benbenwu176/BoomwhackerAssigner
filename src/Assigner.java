import java.util.ArrayList;
import java.util.HashSet;
import java.util.PriorityQueue;
import java.util.Set;

public class Assigner {
    // The number of players in the ensemble
    private int numPlayers;

    // A list of players in the ensemble
    private ArrayList<Player> players;

    /**
     * Creates a new Assigner object, initializing players in numerical order.
     * @param numPlayers the number of players in the ensemble
     */
    public Assigner(int numPlayers) {
        players = new ArrayList<>();
        assignPlayers(numPlayers);
    }

    /**
     * Creates a new Assigned object, initializing a list of players with specific names.
     * @param names a list of names of the players
     */
    public Assigner(String[] names) {
        players = new ArrayList<>();
        assignPlayers(names);
    }

    /*
     * Fills the queue with new numbered players.
     */
    private void assignPlayers(int numPlayers) {
        for (int i = 1; i <= numPlayers; i++) {
            players.add(new Player("Player " + i));
        }
    }

    /*
     * Fills the queue with new players with specific names.
     */
    private void assignPlayers(String[] names) {
        for (String name : names) {
            players.add(new Player(name));
        }
    }

    /**
     * Assigns boomwhackers to each player.
     * Pre: boomwhackers contains queue of all notes played in the piece, != null,
     *      notes != null
     * @param boomwhackers a PQ of boomwhackers
     */
    public void assignWhackers(PriorityQueue<Boomwhacker> boomwhackers) {
        if (boomwhackers == null) {
            return;
        }
        while (!boomwhackers.isEmpty()) {
            attemptAdd(boomwhackers);
        }

        // TODO: Recursive add method
//        if (add(boomwhackers, boomwhackers.poll()) == false) {
//            throw new IllegalStateException("Error: Failed to add all boomwhackers. " +
//                    "Either manually assign or change music.");
//        }
    }

    /*
     * Attempts to give a boomwhacker to a player.
     */
    private void attemptAdd(PriorityQueue<Boomwhacker> boomwhackers) {
        Boomwhacker bw = boomwhackers.poll();

        // Adds a boomwhacker to the player least restricted by adding this boomwhacker
        int maxIndex = -1;
        double maxInterval = Double.NEGATIVE_INFINITY;
        // Player with the largest intervals between playing their boomwhackers
        // will be assigned the current boomwhacker
        for (int i = 0; i < players.size(); i++) {
            Player player = players.get(i);
            Edge playerLowestEdge = player.getLowestEdge(bw);
            if (playerLowestEdge == null) {
                maxIndex = i;
                break;
            }
            if (playerLowestEdge.getTimeInterval() > maxInterval) {
                maxInterval = playerLowestEdge.getTimeInterval();
                maxIndex = i;
            }
        }
        // Adds boomwhacker to the least restricted player, and moves them to the end of the list
         if (players.get(maxIndex).addWhacker(bw) == null) {
             boomwhackers.offer(bw);
         }
//        // Attempt to add note
//        Boomwhacker newNote = player.addWhacker(bw);
//        while (newNote == null) {
//            // Add unsuccessful, try new player
//            failedPlayers.add(player);
//            player = players.poll();
//            if (player == null) {
//                return false;
//            }
//            newNote = player.addWhacker(bw);
//        }
//        if (!newNote.equals(bw)) {
//            // Add successful with replacement. Add old note to new player
//            boomwhackers.offer(newNote);
//        } // Else, add was successful with no replacement needed.
//
//        return true;
    }

    /*
     * Recursively attempts to give a boomwhacker to a player.
     */
    /*
    private boolean add(PriorityQueue<Boomwhacker> boomwhackers, Boomwhacker bw) {
        if (bw == null) {
            return true;
        }
        Player player = players.poll();
        Set<Player> failedPlayers = new HashSet<>();
        // Attempt to add note
        Boomwhacker newNote = player.addWhacker(bw);
        while (newNote == null) {
            // Add unsuccessful, try new player
            failedPlayers.add(player);
            player = players.poll();
            if (player == null) {
                return false;
            }
            newNote = player.addWhacker(bw);
        }
        if (!newNote.equals(bw)) {
            // Add successful with replacement. Add old note to new player
            boomwhackers.offer(newNote);
        } // Else, add was successful with no replacement needed.

        // Add back players that failed to add this note
        for (Player p : failedPlayers) {
            players.offer(p);
        }
        return true;
    }
    */

    @Override
    public String toString() {
        StringBuilder s = new StringBuilder();
        for (Player p : players) {
            s.append(p.toString()).append('\n');
        }
        return s.toString();
    }
}