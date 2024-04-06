#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include "merc.h"
void check_travel_point (CHAR_DATA * ch)
{
  FILE *fp;
  char filename[1024], line[64], expStr[16], *cptr;
  long realxp;
  if (IS_NPC (ch) || IS_IMMORTAL (ch))
    return;
  mkdir ("travelpts", 0755);
  sprintf (filename, "travelpts/%d", ch->in_room->vnum);
  if ((fp = fopen (filename, "r")) != NULL)
    {
      while (fgets (line, 64, fp) != NULL)
	{
	  if ((cptr = strchr (line, '\n')) != NULL)
	    *cptr = '\0';
	  if (!strcmp (ch->name, line))
	    {
	      fclose (fp);
	      return;
	    }
	}
      fclose (fp);
    }
  if (ch->in_room->tp_exp > 1000000)
    ch->in_room->tp_exp = 1000000;
  realxp = ch->in_room->tp_exp * 100;
  sprintf (expStr, "%ld", realxp);
  if ((fp = fopen (filename, "a")) != NULL)
    {
      if (ch->in_room->tp_msg == NULL || ch->in_room->tp_msg[0] == '\0')
	act
	  ("You have earned $t experience for your courageous explorations.",
	   ch, expStr, NULL, TO_CHAR);

      else
	act (ch->in_room->tp_msg, ch, expStr, NULL, TO_CHAR);
      gain_exp (ch, realxp);
      fprintf (fp, "%s\n", ch->name);
      fclose (fp);
    }
}
