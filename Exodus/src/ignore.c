#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

bool
is_ignoring (CHAR_DATA * ch, CHAR_DATA * victim)
{
  int pos;
  CHAR_DATA *rch;


  if (ch->desc == NULL)
    rch = ch;
  else
    rch = ch->desc->original ? ch->desc->original : ch;

  if (IS_NPC (rch))
    return FALSE;

  for (pos = 0; pos < MAX_IGNORE; pos++)
    {
      if (rch->pcdata->ignore[pos] == NULL)
	break;

      if (!str_cmp (rch->pcdata->ignore[pos], victim->name))
	return TRUE;
    }

  return FALSE;
}

void
do_ignore (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim, *rch;
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  int pos;
  bool found = FALSE;

  one_argument (argument, arg);

  if (ch->desc == NULL)
    rch = ch;
  else
    rch = ch->desc->original ? ch->desc->original : ch;

  if (IS_NPC (rch))
    return;

  if (arg[0] == '\0')
    {
      send_to_char ("You are currently ignoring:\n\r", rch);
      send_to_char ("---------------------------\n\r", rch);

      for (pos = 0; pos < MAX_IGNORE; pos++)
	{
	  if (rch->pcdata->ignore[pos] == NULL)
	    {
	      if (pos < 1)
		send_to_char ("No one.\n\r", rch);
	      break;
	    }
	  else
	    {
	      sprintf (buf, "%d: %s\n\r", pos + 1, rch->pcdata->ignore[pos]);
	      send_to_char (buf, rch);
	    }
	}
      return;
    }

  if ((victim = get_char_world (rch, argument)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (IS_NPC (victim))
    {
      send_to_char ("Ignore a mob?  You don't think so.\n\r", ch);
      return;
    }

  if (IS_IMMORTAL (victim))
    {
      send_to_char ("You really don't think that is a good idea.\n\r", ch);
      return;
    }

  if (ch == victim)
    {
      send_to_char ("You don't think you really want to ignore yourself.\n\r",
		    ch);
      return;
    }



  for (pos = 0; pos < MAX_IGNORE; pos++)
    {
      if (rch->pcdata->ignore[pos] == NULL)
	break;

      if (found)
	{
	  rch->pcdata->ignore[pos - 1] = rch->pcdata->ignore[pos];
	  rch->pcdata->ignore[pos] = NULL;
	  continue;
	}

      if (!str_prefix (arg, rch->pcdata->ignore[pos]))
	{
//          if (!IS_IMMORTAL(ch))
  // 	     WAIT_STATE (ch, 3 * PULSE_VIOLENCE);
	  sprintf (buf, "You stop ignoring %s.\n\r", victim->name);
	  send_to_char (buf, ch);
	  //Iblis - people were spamming others with the below message, so it got yanked
//	  sprintf (buf, "%s stops ignoring you.\n\r", ch->name);
//	  send_to_char (buf, victim);
	  free_string (rch->pcdata->ignore[pos]);
	  rch->pcdata->ignore[pos] = NULL;
	  found = TRUE;
	}
    }


  if (pos >= MAX_IGNORE)
    {
      send_to_char ("You can't ignore anymore people\n\r", ch);
      return;
    }
  if (!found)
    {
      //if (!IS_IMMORTAL(ch))
        // WAIT_STATE (ch, 3 * PULSE_VIOLENCE);
      rch->pcdata->ignore[pos] = str_dup (victim->name);
      sprintf (buf, "You now ignore %s.\n\r", victim->name);
      send_to_char (buf, ch);
     
     //Iblis - people were spamming others with the below message, so it got yanked
     // sprintf (buf, "%s ignores you.\n\r", ch->name);
     // send_to_char (buf, victim);
    }
  return;

}
