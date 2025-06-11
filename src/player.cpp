#include "gen.hpp"
#include "player.hpp"

/**
 * @brief Attempts to add a note to a player's bucket. Returns the inverse of the maximum time differential
 * between notes. Successfully adds if the note is able to be played within the player's switch time.
 *
 */
double Player::attempt_add(Note *note, bool force)
{
  int dupe_index = -1;
  for (int i = 0; i < bucket.size(); i++)
  {
    if (bucket[i]->pitch == note->pitch && bucket[i]->capped == note->capped)
    {
      dupe_index = i;
      break;
    }
  }
  return 0;
}

/**
 * @brief Adds a note to this player?
 */
bool Player::add_note(Note *note, int mode)
{
  if (note->pitch >= C2_MIDI)
  {
    for (int i = 0; i < assignment->mrp[note->pitch].size(); i++)
    {
      if (assignment->mrp[note->pitch][i] != nullptr)
      {
        // Add the note to the player's bucket
      }
    }
  }

  return true;
}