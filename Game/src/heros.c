//
//  heros.c
//  Functions and routines specifically in support of new style hero
//  characters.
//
//  Exodus Consortium - Copyright 1999 all rights reserved.
//
//
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include <ctype.h>
#include <sys/stat.h>
#include "heros.h"

// Akamai 4/26/99 - HERO changes - basically this is the set of routines
// that allow the manipulation of hero'd characters. There are special
// case functions for promoting, demoting and 'training' heros.

// Iblis 6/7/03 - Basically, the simplicity of the new system allows heros to be
// treated more like normal folk.  Most (if not all) of the functions in here are never used, so I deleted them.

// some necessary prototypes
DECLARE_DO_FUN (do_say);

//DECLARE_DO_FUN (do_hard_quit);
DECLARE_SPELL_FUN (spell_null);
//void gain_hero_skills (CHAR_DATA * imm, CHAR_DATA * ch);

//
// Function prototypes that support the do_hero_gain and
// do_hero_practice routines
//
int is_hero_group (CHAR_DATA * hero, int gn);
int get_group_cost (CHAR_DATA * hero, int gn);
int is_hero_skill (CHAR_DATA * hero, int sn);
int get_skill_cost (CHAR_DATA * hero, int sn);
int get_rating (CHAR_DATA * hero, int sn);

// when a character is made a hero, the old player file is backedup
// as <playername>.PFILE_BACKUP_EXT - this can be used to restore the
// old pfile, should it ever be the case that the hero abuses his power
//
//
// Make this caracter a hero in exodus.
//
void do_make_hero (CHAR_DATA * imm, CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];
  char old_fname[MAX_INPUT_LENGTH];
  char new_fname[MAX_INPUT_LENGTH];
  int lev = 0, gn = 0;

  // better be an immortal that does this
  if (!IS_IMMORTAL (imm))
    return;

  // if the caracter is an NPC - they can't be a hero
  if (IS_NPC (ch))
    return;

  // are they already a hero? 
  if (ch->pcdata->hero)
    {
      sprintf (buf, "%s already has hero status.\n\r", ch->name);
      send_to_char (buf, imm);
      return;
    }
  // move the player's pfile (backup incase of demotion)
  sprintf (old_fname, "%s%c/%s", PLAYER_DIR, ch->name[0],
	   capitalize (ch->name));
  sprintf (new_fname, "%s%c/%s%s", PLAYER_DIR, ch->name[0],
	   capitalize (ch->name), PFILE_BACKUP_EXT);
  rename (old_fname, new_fname);

  // set their hero flag
  ch->pcdata->hero = TRUE;

  //IBLIS 5/18/03 - To deal with the new XP system
  // give them every skill in their player Class
//  gain_hero_skills (imm, ch);

  // raise the char to level 91, hero level
  for (lev = ch->level; lev < LEVEL_EXPLORER; lev++)
    {

      //ch->level += 1;
      advance_level (ch);
    }

  //ch->exp = exp_per_level (ch, ch->pcdata->points) * UMAX (1, ch->level);
  ch->trust = 0;

  //Give the hero all skills at 1%
  for (gn = 0; gn < MAX_SKILL; gn++)
    {
      if (skill_table[gn].name != NULL)
	group_add (ch, skill_table[gn].name, FALSE);
    }

  // force a save - save the new hero status
  save_char_obj (ch);

  // give some messages to the character and the immortal
  sprintf (buf, "You have promoted %s to Hero.\n\r", ch->name);
  send_to_char (buf, imm);
  sprintf (buf, "You have been promoted to hero.\n\r");
  send_to_char (buf, ch);
}


//
// Bad caracter is losing his hero status
//
void do_unmake_hero (CHAR_DATA * imm, CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];
  char old_fname[MAX_INPUT_LENGTH];
  char new_fname[MAX_INPUT_LENGTH];

//   DESCRIPTOR_DATA *d;

  // better be an immortal that does this
  if (!IS_IMMORTAL (imm))
    return;

  // if the caracter is an NPC - they can't be a hero or un-heroed
  if (IS_NPC (ch))
    return;

  // They better have been made a hero previously
  if (!(ch->pcdata->hero))
    {
      sprintf (buf, "%s is not currently a hero.\n\r", ch->name);
      send_to_char (buf, imm);
      return;
    }
  // move the player's hero pfile
  sprintf (old_fname, "%s%c/%s", PLAYER_DIR, ch->name[0],
	   capitalize (ch->name));
  sprintf (new_fname, "%s%c/%s%s", PLAYER_DIR, ch->name[0],
	   capitalize (ch->name), HERO_BACKUP_EXT);
  rename (old_fname, new_fname);

  // restore the old saved pfile
  sprintf (old_fname, "%s%c/%s%s", PLAYER_DIR, ch->name[0],
	   capitalize (ch->name), PFILE_BACKUP_EXT);
  sprintf (new_fname, "%s%c/%s", PLAYER_DIR, ch->name[0],
	   capitalize (ch->name));
  rename (old_fname, new_fname);

  // give some messages to the character and the immortal
  sprintf (buf, "You have demoted %s.\n\r", ch->name);
  send_to_char (buf, imm);
  sprintf (buf, "You have been demoted. You are no longer a hero.\n\r");
  send_to_char (buf, ch);
  sprintf (buf,
	   "You will have your regular character on your next login.\n\r");
  send_to_char (buf, ch);

  // force the player to quit - do_hard_quit() is nice because it does
  // not save the player file (which would blow what we just did above)
  do_hard_quit (ch, "", FALSE);
}



