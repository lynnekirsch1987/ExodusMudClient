//
// version.c
//
// Programmer: Akamai
// Date: 12/07/98
//
// Remember to update the defined constants each time the code is built.
// If we were using real configuration managment tools, we could do this
// automatically.
//
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

#define CVS_ID "$Id: version.c,v 1.49 2004/10/30 19:32:10 dijoist Exp $"

#define VERS_DATE "October 27, 2004"
#define VERS_TIME "12.30 PM PST"
#define VERS_VERS "Exodus 2.6"
//Upped to 2.5 because of cardgame
struct credits_type
{
  char *real_name;
  char *imm_char_name;
  char *alt_char_names;
  char *coding_start;
  char *coding_finish;
  char *changes[10];
};
const struct credits_type version_credits[] = {
  /* 0 */
  {"Jack I. Muramatsu", "Shinowlan", "Shinowlan", "1/1998", "Present",
   {"Bank bug", "And other misc bugs", '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0'},},
  /* 1 */
  {"David W. McDonald", "Akamai   ", "Pulewai, Percy, Arbormists",
   "5/1998", "Present", {"Complete re-write of clan code",
			 "Hero changes, hero training",
			 "And other misc bugs", '\0', '\0', '\0',
			 '\0', '\0', '\0', '\0'},},
  /* 2 */ {"Flemming K. Hansen",
	   "Morgan   ", "Felicia, Christina, Kastagir", "7/2000",
	   "Present ", {"Changed bad_summon to default off",
			"tweaked creation points a bit",
			"Changed update.c to force people into void when ld",
			"added the immcommands linkload and lunload",
			"Added several functions to do_auction, minbid is in",
			"Added hearclan, admin channel, newsockets",
			"added configurable colors to immchannel and adminchannel",
			"pfiles are now version 7",
			"NonPK protection, toggleable by imm with 'safe_nopk'",
			"fixed a few bugs"}},
  /* 3 */ {"Devin Brown",
	   "Minax    ", "Possum", "7/2001", "Present ",
	   {"Added Assassins",
	    "Added Reavers (with substantial outside help)",
	    "Added Avatars",
	    "Numerous other well needed additions", '\0', '\0',
	    '\0', '\0', '\0', '\0'}},
  /* 4 */ {"Nicholas Jackson",
	   "Iblis    ", "Flanch, Dijo, Aenima, Pingo", "5/2003",
	   "Present ", {"Converted to a new experience system",
			"Added Monks, Necros, Liches, new Avatars",
			"Added MultiClassing capabilities, Chaos Jesters",
			"Fixed Reavers to be bugfree",
			"Added Litans, Kalians, and new AC/Hitroll system",
			"Added Super Duel and Battle Royale",
			"Added the Card Game, multi-slot eq",
			"Finished object triggers (and added lag/chance etc)",
			"and various bug fixes", 
			'\0'}},
  /* 5 */ {"Josh Smykal",
	   "Adeon    ", "Cory, Xeroth, Augustus", "7/2003",
	   "9/2003  ", {"Added several new building features",
			"Bug fixes",
			"Added immortal only board 'projects'",
			'\0', '\0', '\0', '\0', '\0', '\0',
			'\0'}},
  /* 6 */ {"Daniel Rife",
	   "????    ", "Paarshad, Sargon, Poe, Ophidias, Psi, ...", "6/2004",
	   "Present ", {"Created Traps",
			"Bug fixes",
			"A bunch of little stuff",
			'\0', '\0', '\0', '\0', '\0', '\0',
			'\0'}},
  /* 7 */ {'\0', '\0', '\0', '\0', '\0',
	   {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	    '\0'}}
};
void do_version (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];

  //   char name[MAX_INPUT_LENGTH];
  int i;
  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      sprintf (buf, "Version: %s\n\rBuild: %s %s\n\r", VERS_VERS,
	       VERS_DATE, VERS_TIME);
      send_to_char (buf, ch);
    }

  else
    {
      sprintf (buf, "Coders:\n\r");
      send_to_char (buf, ch);
      for (i = 0; version_credits[i].imm_char_name != NULL; i++)
	{
	  sprintf (buf, "\t%s\t%s to %s\n\r",
		   version_credits[i].imm_char_name,
		   version_credits[i].coding_start,
		   version_credits[i].coding_finish);
	  send_to_char (buf, ch);
	}
    }
}
