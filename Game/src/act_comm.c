#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <crypt.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"
#include "obj_trig.h"
#include "Druid.h"

#define ANSI_CLSHOME "[2J[1;1H"
bool ch_is_quitting = FALSE;
void clan_defect args ((CHAR_DATA * ch));
bool nowhere = FALSE;
bool isorder = FALSE;
short poll_ticks = 0;
char poll_question[MAX_STRING_LENGTH] = "";
extern long gold_toll;
extern long damage_toll;
extern bool is_linkloading;

/* command procedures needed */
DECLARE_DO_FUN (do_quit);
extern struct clan_type clan_table[MAX_CLAN];
extern void roll_stats (CHAR_DATA * ch);
extern void do_sset (CHAR_DATA * ch, char *argument);
extern void do_mset (CHAR_DATA * ch, char *argument);
extern void do_pardon (CHAR_DATA * ch, char *argument);
extern void do_transfer (CHAR_DATA * ch, char *argument);
extern void do_echo (CHAR_DATA * ch, char *argument);
extern long genraces[];
extern bool battle_royale;

//Morgan 12/23/03 Ignore function
extern bool is_ignoring(CHAR_DATA *ch, CHAR_DATA *victim);

//IBLIS 6/20/03 - needed for Battle Royale Update on char quit
extern CHAR_DATA *br_leader;
extern CHAR_DATA *last_br_kill;

//IBLIS 5/31/03 - makes a string argument into a drunkified string
void drunkify args((CHAR_DATA * ch, char *argument));
void superduel_ends args((CHAR_DATA *winner, CHAR_DATA *loser));
void aggressor_update args ((void));
void free_aggressor_list args ((AGGRESSOR_LIST * b));
void remove_aggressor args ((CHAR_DATA * ch, CHAR_DATA * victim));
void do_auction args ((CHAR_DATA * ch, char *argument));
void do_shout (CHAR_DATA * ch, char *argument);
void reset_race args((CHAR_DATA* ch));
char* nidaeify args((CHAR_DATA *ch, char* bufn, char *argument));
char* anti_nidaeify args((CHAR_DATA *ch, char* bufn, char *argument));
void nuke_boarded_mounts args((CHAR_DATA *ch));

void cj_self args((CHAR_DATA *ch, char* argument));
void cj_channel args((CHAR_DATA *ch, char* argument));
void do_switchtalk args((CHAR_DATA *ch,char* argument));

void affect_strip_skills args((CHAR_DATA * ch));

struct swap_list
{
  CHAR_DATA *member;
  struct swap_list *next;
};
void talk_auction (char *argument)
{
  DESCRIPTOR_DATA *d;
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *original;
  sprintf (buf, "`a[`NAuction`a] `N%s``", argument);
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      original = d->original ? d->original : d->character;	/* if switched */
      if ((d->connected == CON_PLAYING)
	  && !IS_SET (original->comm, COMM_NOAUCTION))
	act_new (buf, original, NULL, NULL, TO_CHAR, POS_SLEEPING);
    }
}


// Minax 7-8-02 Silly little command that replaces the old social.
void do_pat (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Pat whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (ch->level >= victim->level)
    {
      act ("You pat $N on the head.", ch, NULL, victim, TO_CHAR);
      act ("$n pats you on the head.", ch, NULL, victim, TO_VICT);
      act ("$n pats $N on the head.", ch, NULL, victim, TO_NOTVICT);
      return;
    }
  if (ch->level < victim->level)
    {
      act ("You kneel before $N, who pats you on the head.", ch, NULL,
	   victim, TO_CHAR);
      act ("$n kneels before you.  You pat $m on the head.", ch, NULL,
	   victim, TO_VICT);
      act ("$n kneels before $N, who pats $m on the head.", ch, NULL,
	   victim, TO_NOTVICT);
      return;
    }
}


// Minax 7-9-02 Dice command lets a player roll a dice for the room to see.
void do_dice (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int die;
  die = number_range (1, 100);
  sprintf (buf, "`oYou throw a 100-sided die and roll a `j%d`o!``\n\r", die);
  send_to_char (buf, ch);
  sprintf (buf,
	   "`o-> `k$n `othrows a 100-sided die and rolls a `j%d`o!``", die);
  act (buf, ch, NULL, NULL, TO_ROOM);
  return;
}

void do_cls (CHAR_DATA * ch, char *argument)
{
  send_to_char (ANSI_CLSHOME, ch);
  return;
}

void do_delet (CHAR_DATA * ch, char *argument)
{
  send_to_char ("You must type the full command to delete yourself.\n\r", ch);
}

void do_delete (CHAR_DATA * ch, char *argument)
{
  char strsave[MAX_INPUT_LENGTH];
  char pword[MAX_INPUT_LENGTH];
  if (IS_NPC (ch))
    return;
  if IS_SET(ch->comm2,COMM_NODELETE)
  {
    send_to_char("Nice try.  You can't escape that easily.\n\r",ch);
    return;
  }
  one_argument_nl (argument, pword);
  if (ch->pcdata->confirm_delete)
    {
      if (argument[0] == '\0')
	{
	  send_to_char ("Delete status removed.\n\r", ch);
	  ch->pcdata->confirm_delete = FALSE;
	  return;
	}

      else
	{
	  if (strcmp (ch->pcdata->pwd, crypt (pword, ch->pcdata->pwd)) == 0)
	    {
	      sprintf (strsave, "%s%c/%s", PLAYER_DIR,
		       toupper (ch->name[0]), capitalize (ch->name));
	      wiznet ("$N turns $Mself into line noise.", ch, NULL, 0, 0, 0);
	      if (ch->clan != CLAN_BOGUS)
		clan_defect (ch);
	      do_hard_quit (ch, "", FALSE);
	      unlink (strsave);
	      return;
	    }

	  else
	    {
	      send_to_char ("Wrong password.\n\r", ch);
	      ch->pcdata->confirm_delete = FALSE;
	      return;
	    }
	}
    }
  if (argument[0] == '\0')
    {
      send_to_char
	("Type delete <password> to remove this character.\n\r", ch);
      return;
    }
  if (strcmp (ch->pcdata->pwd, crypt (pword, ch->pcdata->pwd)) == 0)
    {
      send_to_char
	("Type delete <password> again to confirm this command.\n\r", ch);
      send_to_char ("WARNING: this command is irreversible.\n\r", ch);
      send_to_char
	("Typing delete with no argument will undo delete status.\n\r", ch);
      ch->pcdata->confirm_delete = TRUE;
      wiznet ("$N is contemplating deletion.", ch, NULL, 0, 0,
	      get_trust (ch));
    }

  else
    send_to_char ("Wrong password.\n\r", ch);
  return;
}


/* RT code to display channel status */
void do_channels (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];

  /* lists all channels and their status */
  send_to_char ("`a[`fchannel`a]    [`fstatus`a]``\n\r", ch);
  send_to_char ("`a---------------------``\n\r", ch);
  send_to_char ("`lgossip``         ", ch);
  if (!IS_SET (ch->comm, COMM_NOGOSSIP))
    send_to_char ("`kON``\n\r", ch);

  else
    send_to_char ("`aOFF``\n\r", ch);
  send_to_char ("`lauction``        ", ch);
  if (!IS_SET (ch->comm, COMM_NOAUCTION))
    send_to_char ("`kON``\n\r", ch);

  else
    send_to_char ("`aOFF``\n\r", ch);
  send_to_char ("`lmusic``          ", ch);
  if (!IS_SET (ch->comm, COMM_NOMUSIC))
    send_to_char ("`kON``\n\r", ch);

  else
    send_to_char ("`aOFF``\n\r", ch);
  send_to_char ("`lQ/A``            ", ch);
  if (!IS_SET (ch->comm, COMM_NOQUESTION))
    send_to_char ("`kON``\n\r", ch);

  else
    send_to_char ("`aOFF``\n\r", ch);
  send_to_char ("`lOOC``            ", ch);
  if (!IS_SET (ch->comm, COMM_NOOOC))
    send_to_char ("`kON``\n\r", ch);

  else
    send_to_char ("`aOFF``\n\r", ch);
  send_to_char ("`lgrats``          ", ch);
  if (!IS_SET (ch->comm, COMM_NOGRATS))
    send_to_char ("`kON``\n\r", ch);

  else
    send_to_char ("`aOFF``\n\r", ch);
  send_to_char ("`linfo``           ", ch);
  if (!IS_SET (ch->comm, COMM_NOINFO))
    send_to_char ("`kON``\n\r", ch);

  else
    send_to_char ("`aOFF``\n\r", ch);

  //Iblis added channels below to next comment
  
  send_to_char ("`larena``          ", ch);
  if (!IS_SET (ch->comm, COMM_NOSUPERDUEL))
    send_to_char ("`kON``\n\r", ch);
  else send_to_char ("`aOFF``\n\r", ch);
  send_to_char ("`lpk``             ", ch);
  if (!IS_SET (ch->comm, COMM_NOPK))
    send_to_char ("`kON``\n\r", ch);
  else send_to_char ("`aOFF``\n\r", ch);
  send_to_char ("`lrant``           ", ch);
  if (!IS_SET (ch->comm2, COMM_NORANT))
     send_to_char ("`kON``\n\r", ch);
  else send_to_char ("`aOFF``\n\r", ch);
  send_to_char ("`lpoll``           ", ch);
  if (!IS_SET (ch->comm2, COMM_NOPOLL))
      send_to_char ("`kON``\n\r", ch);
  else send_to_char ("`aOFF``\n\r", ch);
  send_to_char ("`lgemote``         ", ch);
  if (!IS_SET (ch->comm2, COMM_NOGEMOTE))
      send_to_char ("`kON``\n\r", ch);
  else send_to_char ("`aOFF``\n\r", ch);

  //End of Iblis added channels
      
    
  if (IS_IMMORTAL (ch))
    {
      send_to_char ("`lgod channel``    ", ch);
      if (!IS_SET (ch->comm, COMM_NOWIZ))
	send_to_char ("`kON``\n\r", ch);

      else
	send_to_char ("`aOFF``\n\r", ch);
    }

  /*
   * Admin channel, added by Morgan
   * June 24. 2000, for level 100's
   * to communicate in "private"
   */
  if (IS_IMMORTAL (ch) && ch->level > 99)
    {
      send_to_char ("`lAdmin channel``  ", ch);
      if (!IS_SET (ch->comm, COMM_NOADMIN))
	send_to_char ("`kON``\n\r", ch);

      else
	send_to_char ("`aOFF``\n\r", ch);
    }
  // hearclan, added by Morgan
  // June 24. 2000, for spying on clans
  // without having clan flag
  if (IS_IMMORTAL (ch) && ch->level > 94)
    {
      send_to_char ("`lhearclan``       ", ch);
      if (!IS_SET (ch->comm, COMM_HEARCLAN))
	send_to_char ("`kON``\n\r", ch);

      else
	send_to_char ("`aOFF``\n\r", ch);
    }
  send_to_char ("`lshouts``         ", ch);
  if (!IS_SET (ch->comm, COMM_SHOUTSOFF))
    send_to_char ("`kON``\n\r", ch);

  else
    send_to_char ("`aOFF``\n\r", ch);
  send_to_char ("`ltells``          ", ch);
  if (!IS_SET (ch->comm, COMM_DEAF))
    send_to_char ("`kON``\n\r", ch);

  else
    send_to_char ("`aOFF``\n\r", ch);
  send_to_char ("`lquiet mode``     ", ch);
  if (IS_SET (ch->comm, COMM_QUIET))
    send_to_char ("`kON``\n\r", ch);

  else
    send_to_char ("`aOFF``\n\r", ch);
  if (IS_SET (ch->comm, COMM_AFK))
    send_to_char ("You are AFK.\n\r", ch);
  if (IS_SET (ch->comm, COMM_SNOOP_PROOF))
    send_to_char ("You are immune to snooping.\n\r", ch);
  if (ch->lines != PAGELEN)
    {
      if (ch->lines)
	{
	  sprintf (buf, "You display %d lines of scroll.\n\r", ch->lines + 2);
	  send_to_char (buf, ch);
	}

      else
	send_to_char ("Scroll buffering is off.\n\r", ch);
    }
  if (ch->prompt != NULL)
    {
      sprintf (buf, "Your current prompt is: %s\n\r", ch->prompt);
      send_to_char (buf, ch);
    }
  if (IS_SET (ch->comm, COMM_NOSHOUT))
    send_to_char ("You cannot shout.\n\r", ch);
  if (IS_SET (ch->comm, COMM_NOTELL))
    send_to_char ("You cannot use tell.\n\r", ch);
  if (IS_SET (ch->comm, COMM_NOCHANNELS))
    send_to_char ("You cannot use channels.\n\r", ch);
  if (IS_SET (ch->comm, COMM_NOEMOTE))
    send_to_char ("You cannot show emotions.\n\r", ch);
  if (IS_SET (ch->comm2, COMM_RANTONLY))
    send_to_char ("You can only use the Rant channel.\n\r",ch);	 
}


/* RT deaf blocks out all shouts */
void do_deaf (CHAR_DATA * ch, char *argument)
{
  if (IS_SET (ch->comm, COMM_DEAF))
    {
      send_to_char ("You can now hear tells again.\n\r", ch);
      REMOVE_BIT (ch->comm, COMM_DEAF);
    }

  else
    {
      send_to_char ("From now on, you won't hear tells.\n\r", ch);
      SET_BIT (ch->comm, COMM_DEAF);
    }
}


/* RT quiet blocks out all communication */
void do_quiet (CHAR_DATA * ch, char *argument)
{
  if (IS_SET (ch->comm, COMM_QUIET))
    {
      send_to_char ("Quiet mode removed.\n\r", ch);
      REMOVE_BIT (ch->comm, COMM_QUIET);
    }

  else
    {
      send_to_char
	("From now on, you will only hear says and emotes.\n\r", ch);
      SET_BIT (ch->comm, COMM_QUIET);
    }
}


/* afk command */
void do_afk (CHAR_DATA * ch, char *argument)
{
  if (IS_SET (ch->comm, COMM_AFK))
    {
      send_to_char ("AFK mode removed. Type 'replay' to see tells.\n\r", ch);
      REMOVE_BIT (ch->comm, COMM_AFK);
    }

  else
    {
      send_to_char ("You are now in AFK mode.\n\r", ch);
      SET_BIT (ch->comm, COMM_AFK);
      free_string (ch->afkmsg);
      ch->afkmsg = str_dup (argument);
    }
}

void do_replay (CHAR_DATA * ch, char *argument)
{
  if (IS_NPC (ch))
    {
      send_to_char ("You can't replay.\n\r", ch);
      return;
    }
  if (buf_string (ch->pcdata->buffer)[0] == '\0')
    {
      send_to_char ("You have no tells to replay.\n\r", ch);
      return;
    }
  page_to_char (buf_string (ch->pcdata->buffer), ch);
  clear_buf (ch->pcdata->buffer);
}

void do_info (CHAR_DATA * ch, char *argument)
{
  if (IS_SET (ch->comm, COMM_NOINFO))
    {
      send_to_char ("Info channel is now ON.\n\r", ch);
      REMOVE_BIT (ch->comm, COMM_NOINFO);
    }

  else
    {
      send_to_char ("Info channel is now OFF.\n\r", ch);
      SET_BIT (ch->comm, COMM_NOINFO);
    }
}


/* RT chat replaced with ROM gossip */
void do_gossip (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH], bufn[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  if (argument[0] == '\0')
    {
      if (IS_SET (ch->comm, COMM_NOGOSSIP))
	{
	  send_to_char ("Gossip channel is now ON.\n\r", ch);
	  REMOVE_BIT (ch->comm, COMM_NOGOSSIP);
	}

      else
	{
	  send_to_char ("Gossip channel is now OFF.\n\r", ch);
	  SET_BIT (ch->comm, COMM_NOGOSSIP);
	}
    }

  else
    /* gossip message sent, turn gossip on if it isn't already */
    {
      if (IS_SET (ch->comm, COMM_QUIET))
	{
	  send_to_char ("You must turn off quiet mode first.\n\r", ch);
	  return;
	}
      if (IS_SET (ch->comm, COMM_NOCHANNELS))
	{
	  send_to_char
	    ("The gods have revoked your channel priviliges.\n\r", ch);
	  return;
	}
      if (IS_SET (ch->comm2, COMM_RANTONLY))
        {
          send_to_char
            ("The gods have decreed you may only use the Rant Channel.\n\r", ch);
          return;
        }
       
      REMOVE_BIT (ch->comm, COMM_NOGOSSIP);
      if (ch->level < 3)
	{
	  send_to_char
	    ("Sorry, you are too young to project your voice across the lands in that manner.\n\r",
	     ch);
	  return;
	}
      if (ch->race == 7 && !IS_IMMORTAL (ch))
	{
	  act
	    ("The fragile structure of your vocal system prevents you from creating anything other than a high-pitched whine.",
	     ch, NULL, NULL, TO_CHAR);
	  return;
	}
      if (ch->race == PC_RACE_LITAN && !IS_IMMORTAL (ch))
      {
	 do_shout(ch,argument);
	 return;
      }
      //IBLIS 5/31/03 - Added Drunk Talk to gossip
      if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	drunkify (ch, argument);
      if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
        cj_self(ch,argument);
      else 
      {
        sprintf (buf, "`a[`MGossip`a] `lYou`a: '`M%s`a'``\n\r", argument);     
        send_to_char (buf, ch);
      }
      
      if (ch->race == PC_RACE_NIDAE)
        nidaeify(ch,bufn,argument);
      else anti_nidaeify(ch,bufn,argument);
      
      if (!IS_SET (ch->comm2, COMM_SHUTUP))
      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  CHAR_DATA *victim;
	  victim = d->original ? d->original : d->character;
	  if (d->connected == CON_PLAYING && d->character != ch
	      && !IS_SET (victim->comm, COMM_NOGOSSIP)
	      && !IS_SET (victim->comm, COMM_QUIET))
	    {
	      if (is_ignoring(victim, ch)) 
              continue;
	      if ((d->character->race == PC_RACE_NIDAE && !IS_IMMORTAL(victim))|| (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(victim)))
	      {
		if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(d->character))
		  cj_channel(d->character,bufn);
		else act_new ("`a[`MGossip`a]`l $n`a:`a '`M$t`a'``", ch,
				bufn, d->character, TO_VICT, POS_SLEEPING);
	      }
	      else {
 	        if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(d->character))
		  cj_channel(d->character,argument);
		else act_new ("`a[`MGossip`a]`l $n`a:`a '`M$t`a'``", ch,
		     argument, d->character, TO_VICT, POS_SLEEPING);
	      }
	    }
	}
    }
}

void do_grats (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH], bufn[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  if (argument[0] == '\0')
    {
      if (IS_SET (ch->comm, COMM_NOGRATS))
	{
	  send_to_char ("Grats channel is now ON.\n\r", ch);
	  REMOVE_BIT (ch->comm, COMM_NOGRATS);
	}

      else
	{
	  send_to_char ("Grats channel is now OFF.\n\r", ch);
	  SET_BIT (ch->comm, COMM_NOGRATS);
	}
    }

  else
    /* grats message sent, turn grats on if it isn't already */
    {
      if (IS_SET (ch->comm, COMM_QUIET))
	{
	  send_to_char ("You must turn off quiet mode first.\n\r", ch);
	  return;
	}
      if (IS_SET (ch->comm, COMM_NOCHANNELS))
	{
	  send_to_char
	    ("The gods have revoked your channel priviliges.\n\r", ch);
	  return;
	}
      if (IS_SET (ch->comm2, COMM_RANTONLY))
      {
	      send_to_char("The gods have decreed you may only use the Rant Channel.\n\r", ch);
	      return;
      }
      REMOVE_BIT (ch->comm, COMM_NOGRATS);
      if (ch->level < 3)
	{
	  send_to_char
	    ("Sorry, you are too young to project your voice across the lands in that manner.\n\r",
	     ch);
	  return;
	}
      if (ch->race == 7 && !IS_IMMORTAL (ch))
	{
	  act
	    ("The fragile structure of your vocal system prevents you from creating anything other than a high-pitched whine.",
	     ch, NULL, NULL, TO_CHAR);
	  return;
	}

        if (ch->race == PC_RACE_LITAN && !IS_IMMORTAL (ch))
	{
	  do_shout(ch,argument);
	  return;
	}
	
      
      //IBLIS 5/31/03 - Added Drunk Talk to grats
      if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	drunkify (ch, argument);
      if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
        cj_self(ch,argument);
      else {
        sprintf (buf, "`a[`MGrats`a] `lYou`a: '`M%s`a'``\n\r", argument);
        send_to_char (buf, ch);
      }
      
      if (ch->race == PC_RACE_NIDAE)
         nidaeify(ch,bufn,argument);
      else anti_nidaeify(ch,bufn,argument);
      
      if (!IS_SET (ch->comm2, COMM_SHUTUP))
      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  CHAR_DATA *victim;
	  victim = d->original ? d->original : d->character;
	  if (d->connected == CON_PLAYING && d->character != ch
	      && !IS_SET (victim->comm, COMM_NOGRATS)
	      && !IS_SET (victim->comm, COMM_QUIET))
	    {
	      if (is_ignoring(victim, ch)) 
              continue;
	      if ((d->character->race == PC_RACE_NIDAE  && !IS_IMMORTAL(victim))
		      || (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(victim)))
	      {
 	        if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
	          cj_channel(d->character,bufn);
		else act_new ("`a[`MGrats`a]`l $n`a: `a'`M$t`a'``", ch,
	           bufn, d->character, TO_VICT, POS_SLEEPING);
	      }
	      else
	      {
 	        if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
	          cj_channel(d->character,argument);
		else act_new ("`a[`MGrats`a]`l $n`a: `a'`M$t`a'``", ch,
		       argument, d->character, TO_VICT, POS_SLEEPING);
	      }
	    }
	}
    }
}

//Iblis 10-22-03 Made it so the only someones on OOC are immortals
void do_ooc (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH], buf1[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  if (argument[0] == '\0')
    {
      if (IS_SET (ch->comm, COMM_NOOOC))
	{
	  send_to_char ("OOC channel is now ON.\n\r", ch);
	  REMOVE_BIT (ch->comm, COMM_NOOOC);
	}

      else
	{
	  send_to_char ("OOC channel is now OFF.\n\r", ch);
	  SET_BIT (ch->comm, COMM_NOOOC);
	}
    }

  else
    /* OOC message sent, turn OOC on if it isn't already */
    {
      if (IS_SET (ch->comm, COMM_QUIET))
	{
	  send_to_char ("You must turn off quiet mode first.\n\r", ch);
	  return;
	}
      if (IS_SET (ch->comm, COMM_NOCHANNELS))
	{
	  send_to_char
	    ("The gods have revoked your channel priviliges.\n\r", ch);
	  return;
	}
      if (IS_SET (ch->comm2, COMM_RANTONLY))
      {
	      send_to_char("The gods have decreed you may only use the Rant Channel.\n\r", ch);
	      return;
      }
      REMOVE_BIT (ch->comm, COMM_NOOOC);
      sprintf (buf, "`a[`MOOC`a] `lYou`a: '`M%s`a'``\n\r", argument);
      send_to_char (buf, ch);
      if (!IS_SET (ch->comm2, COMM_SHUTUP))
      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  CHAR_DATA *victim;
	  victim = d->original ? d->original : d->character;
	  if (d->connected == CON_PLAYING && d->character != ch
	      && !IS_SET (victim->comm, COMM_NOOOC)
	      && !IS_SET (victim->comm, COMM_QUIET))
	    {
	      if (IS_IMMORTAL(ch) && 
                  ((get_trust (d->character) < ch->invis_level) 
		     || (get_trust (d->character) < ch->incog_level && d->character->in_room != ch->in_room)))
	      {
	        strcpy(buf1, "`oImmortal");
	      }
	      else if (IS_NPC(ch))
	        strcpy(buf1, ch->short_descr);
	      else strcpy(buf1, ch->name);
	      if (is_ignoring(victim, ch)) 
              continue;
	      sprintf(buf,"`a[`MOOC`a] `l%s`a: '`M%s`a'``\n\r",buf1, argument);
	      send_to_char(buf,d->character);
	    }
	}
    }
}

void do_question (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH], bufn[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  if (argument[0] == '\0')
    {
      if (IS_SET (ch->comm, COMM_NOQUESTION))
	{
	  send_to_char ("Q/A channel is now ON.\n\r", ch);
	  REMOVE_BIT (ch->comm, COMM_NOQUESTION);
	}

      else
	{
	  send_to_char ("Q/A channel is now OFF.\n\r", ch);
	  SET_BIT (ch->comm, COMM_NOQUESTION);
	}
    }

  else
    /* question sent, turn Q/A on if it isn't already */
    {
      if (IS_SET (ch->comm, COMM_QUIET))
	{
	  send_to_char ("You must turn off quiet mode first.\n\r", ch);
	  return;
	}
      if (IS_SET (ch->comm, COMM_NOCHANNELS))
	{
	  send_to_char
	    ("The gods have revoked your channel priviliges.\n\r", ch);
	  return;
	}
      if (IS_SET (ch->comm2, COMM_RANTONLY))
      {
	      send_to_char("The gods have decreed you may only use the Rant Channel.\n\r", ch);
	      return;
      }
      REMOVE_BIT (ch->comm, COMM_NOQUESTION);
      if (ch->level < 3)
	{
	  send_to_char
	    ("Sorry, you are too young to project your voice across the lands in that manner.\n\r",
	     ch);
	  return;
	}
      if (ch->race == 7 && !IS_IMMORTAL (ch))
	{
	  act
	    ("The fragile structure of your vocal system prevents you from creating anything other than a high-pitched whine.",
	     ch, NULL, NULL, TO_CHAR);
	  return;
	}
      if (ch->race == PC_RACE_LITAN && !IS_IMMORTAL (ch))
        {
  	  do_shout(ch,argument);
          return;		
	}

      //IBLIS 5/31/03 - Added Drunk Talk to question
      if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        drunkify (ch, argument);
      
      if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
        cj_self(ch,argument);
      else
      {
        sprintf (buf, "`a[`MQuestion`a] `lYou`a: '`M%s`a'``\n\r", argument);
        send_to_char (buf, ch);
      }

      if (ch->race == PC_RACE_NIDAE)
        nidaeify(ch,bufn,argument);
      else anti_nidaeify(ch,bufn,argument);
      
      if (!IS_SET (ch->comm2, COMM_SHUTUP))
      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  CHAR_DATA *victim;
	  victim = d->original ? d->original : d->character;
	  if (d->connected == CON_PLAYING && d->character != ch
	      && !IS_SET (victim->comm, COMM_NOQUESTION)
	      && !IS_SET (victim->comm, COMM_QUIET))
	    {
	      if (is_ignoring(victim, ch)) 
              continue;
	      if ((d->character->race == PC_RACE_NIDAE && !IS_IMMORTAL(victim))|| (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(victim)))
	      {
	       if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
	         cj_channel(d->character,bufn);	     
	       else act_new ("`a[`MQuestion`a]`l $n`a: '`M$t`a'``", ch,
	         bufn, d->character, TO_VICT, POS_SLEEPING);
	      }
	      else 
	      {
		if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
                  cj_channel(d->character,argument);
		else act_new ("`a[`MQuestion`a]`l $n`a: '`M$t`a'``", ch,
		  argument, d->character, TO_VICT, POS_SLEEPING);
	      }
		 
	    }
	}
    }
}


/* RT answer channel - uses same line as questions */
void do_answer (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH], bufn[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  if (argument[0] == '\0')
    {
      if (IS_SET (ch->comm, COMM_NOQUESTION))
	{
	  send_to_char ("Q/A channel is now ON.\n\r", ch);
	  REMOVE_BIT (ch->comm, COMM_NOQUESTION);
	}

      else
	{
	  send_to_char ("Q/A channel is now OFF.\n\r", ch);
	  SET_BIT (ch->comm, COMM_NOQUESTION);
	}
    }

  else
    /* answer sent, turn Q/A on if it isn't already */
    {
	     
      if (IS_SET (ch->comm, COMM_QUIET))
	{
	  send_to_char ("You must turn off quiet mode first.\n\r", ch);
	  return;
	}
      if (IS_SET (ch->comm, COMM_NOCHANNELS))
	{
	  send_to_char
	    ("The gods have revoked your channel priviliges.\n\r", ch);
	  return;
	}
      if (IS_SET (ch->comm2, COMM_RANTONLY))
      {
	      send_to_char("The gods have decreed you may only use the Rant Channel.\n\r", ch);
	      return;
      }
      REMOVE_BIT (ch->comm, COMM_NOQUESTION);
      if (ch->level < 3)
	{
	  send_to_char
	    ("Sorry, you are too young to project your voice across the lands in that manner.\n\r",
	     ch);
	  return;
	}
      if (ch->race == 7 && !IS_IMMORTAL (ch))
	{
	  act
	    ("The fragile structure of your vocal system prevents you from creating anything other than a high-pitched whine.",
	     ch, NULL, NULL, TO_CHAR);
	  return;
	}
       if (ch->race == PC_RACE_LITAN && !IS_IMMORTAL (ch))
       {
	 do_shout(ch,argument);
         return;
        }

      //IBLIS 5/31/03 - Added Drunk Talk to question
      if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        drunkify (ch, argument);
	
      if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
       cj_self(ch,argument);
      else
      {
        sprintf (buf, "`a[`MAnswer`a] `lYou`a: '`M%s`a'``\n\r", argument);
        send_to_char (buf, ch);
      }

      if (ch->race == PC_RACE_NIDAE)
        nidaeify(ch,bufn,argument);
      else anti_nidaeify(ch,bufn,argument);
      
      if (!IS_SET (ch->comm2, COMM_SHUTUP))
      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  CHAR_DATA *victim;
	  victim = d->original ? d->original : d->character;
	  if (d->connected == CON_PLAYING && d->character != ch
	      && !IS_SET (victim->comm, COMM_NOQUESTION)
	      && !IS_SET (victim->comm, COMM_QUIET))
	    {
	      if (is_ignoring(victim, ch)) 
              continue;
	      if ((d->character->race == PC_RACE_NIDAE && !IS_IMMORTAL(victim))|| (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(victim)))
	      {
 	        if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
	          cj_channel(d->character,bufn);
		else act_new ("`a[`MAnswer`a]`l $n`a: '`M$t`a'``", ch,
		       bufn, d->character, TO_VICT, POS_SLEEPING);
	      }
	      else
	      {
		if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
                  cj_channel(d->character,argument);
		else  act_new ("`a[`MAnswer`a]`l $n`a: '`M$t`a'``", ch,
				argument, d->character, TO_VICT, POS_SLEEPING);
	      }
		      
	    }
	}
    }
}

void do_anonymous (CHAR_DATA * ch, char *argument)
{
  if (IS_SET (ch->comm, COMM_ANONYMOUS))
    {
      REMOVE_BIT (ch->comm, COMM_ANONYMOUS);
      send_to_char ("Anonymous mode removed.\n\r", ch);
    }

  else
    {
      if (ch->race != 7 && ch->level < 30)
	{
	  send_to_char
	    ("You are not high enough level to become anonymous.\n\r", ch);
	  return;
	}
      SET_BIT (ch->comm, COMM_ANONYMOUS);
      send_to_char ("You are now anonymous.\n\r", ch);
    }
}


    // The bit toggled for spying on clan channels
    // Added by Morgan on June 24. 2000
void do_hearclan (CHAR_DATA * ch, char *argument)
{
  if (IS_SET (ch->comm, COMM_HEARCLAN))
    {
      send_to_char ("Hearclan is now ON.\n\r", ch);
      REMOVE_BIT (ch->comm, COMM_HEARCLAN);
    }

  else
    {
      send_to_char ("Hearclan is now OFF.\n\r", ch);
      SET_BIT (ch->comm, COMM_HEARCLAN);
    }
}

void do_music (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH], bufn[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  if (argument[0] == '\0')
    {
      if (IS_SET (ch->comm, COMM_NOMUSIC))
	{
	  send_to_char ("Music channel is now ON.\n\r", ch);
	  REMOVE_BIT (ch->comm, COMM_NOMUSIC);
	}

      else
	{
	  send_to_char ("Music channel is now OFF.\n\r", ch);
	  SET_BIT (ch->comm, COMM_NOMUSIC);
	}
    }

  else
    /* music sent, turn music on if it isn't already */
    {
      if (IS_SET (ch->comm, COMM_QUIET))
	{
	  send_to_char ("You must turn off quiet mode first.\n\r", ch);
	  return;
	}
      if (IS_SET (ch->comm, COMM_NOCHANNELS))
	{
	  send_to_char
	    ("The gods have revoked your channel priviliges.\n\r", ch);
	  return;
	}
      if (IS_SET (ch->comm2, COMM_RANTONLY))
      {
	      send_to_char("The gods have decreed you may only use the Rant Channel.\n\r", ch);
	      return;
      }
      if (!IS_CLASS (ch, PC_CLASS_BARD) && !IS_IMMORTAL (ch) && ch->race != PC_RACE_NIDAE)
	{
	  send_to_char
	    ("Only the melodic voices of bards are dulcet enough to be projected to such a wide audience.\n\r",
	     ch);
	  return;
	}
      REMOVE_BIT (ch->comm, COMM_NOMUSIC);
      if (ch->level < 3)
	{
	  send_to_char
	    ("Sorry, you are too young to project your voice across the lands in that manner.\n\r",
	     ch);
	  return;
	}
      if (ch->race == 7 && !IS_IMMORTAL (ch))
	{
	  act
	    ("The fragile structure of your vocal system prevents you from creating anything other than a high-pitched whine.",
	     ch, NULL, NULL, TO_CHAR);
	  return;
	}
      if (ch->race == PC_RACE_LITAN && !IS_IMMORTAL (ch))
      {
        do_shout(ch,argument);
        return;
      }
	
      //IBLIS 5/31/03 - Added Drunk Talk to music
      if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	drunkify (ch, argument);
      if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
        cj_self(ch,argument);
      else
      {
        sprintf (buf, "`a[`QMusic`a] `lYou`a: '`Q%s`a'``\n\r", argument);
        send_to_char (buf, ch);
      }

     if (ch->race == PC_RACE_NIDAE)
       nidaeify(ch,bufn,argument);
     else anti_nidaeify(ch,bufn,argument);
      
     if (!IS_SET (ch->comm2, COMM_SHUTUP))
      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  CHAR_DATA *victim;
	  victim = d->original ? d->original : d->character;
	  if (d->connected == CON_PLAYING && d->character != ch
	      && !IS_SET (victim->comm, COMM_NOMUSIC)
	      && !IS_SET (victim->comm, COMM_QUIET))
	    {
	      if (is_ignoring(victim, ch)) 
              continue;
	      if ((d->character->race == PC_RACE_NIDAE && !IS_IMMORTAL(victim))
		      || (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(victim)))
	      {
		if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
		  cj_channel(d->character,bufn);
		else act_new ("`a[`QMusic`a] `l$n`a: `a'`Q$t`a'``", ch,
	                bufn, d->character, TO_VICT, POS_SLEEPING);
	      }
	      else 
	      {
	        if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
	          cj_channel(d->character,argument);
		else act_new ("`a[`QMusic`a] `l$n`a: `a'`Q$t`a'``", ch,
	          argument, d->character, TO_VICT, POS_SLEEPING);
	      }
	    }
	}
    }
}

void do_elite (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  if (strcmp (ch->name, "Murk") != 0 && strcmp (ch->name, "Cailet") != 0
      && strcmp (ch->name, "Ragnar") != 0
      && strcmp (ch->name, "Taerna") != 0 && strcmp (ch->name, "Nekom") != 0
      && strcmp (ch->name, "Iblis") != 0)  //Sorry to infiltrate your code guys (and gals),
  					   //but you are all gone, so I will just use
					   //this as a channel to talk to myself on
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("You don't want to be lame, do you?\n\r", ch);
      return;
    }
  sprintf (buf, "`a[`bEliTe`a] `lYou`a: '`b%s`a'``\n\r", argument);
  send_to_char (buf, ch);
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      CHAR_DATA *victim;
      victim = d->original ? d->original : d->character;
      if (d->connected == CON_PLAYING && d->character != ch
	  && !IS_NPC (d->character)
	  && (!strcmp (d->character->name, "Cailet")
	      || !strcmp (d->character->name, "Ragnar")
	      || !strcmp (d->character->name, "Nekom")
	      || !strcmp (d->character->name, "Taerna")
	      || !strcmp (d->character->name, "Murk")
	      || !strcmp (d->character->name, "Iblis")))
	{
	  act_new ("`a[`bEliTe`a] `l$n`a: `a'`b$t`a'``", ch, argument,
		   d->character, TO_VICT, POS_SLEEPING);
	}
    }
}


/* clan channels */
void do_clantalk (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH], bufn[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  if (!is_clan (ch))
    {
      send_to_char ("You aren't in a clan.\n\r", ch);
      return;
    }
  if (!is_full_clan (ch))
    {
      send_to_char
	("Sorry, your clan has not yet reached full-clan status.\n\r", ch);
      return;
    }
  if (argument[0] == '\0')
    {
      if (IS_SET (ch->comm, COMM_NOCLAN))
	{
	  send_to_char ("Clan channel is now ON\n\r", ch);
	  REMOVE_BIT (ch->comm, COMM_NOCLAN);
	}

      else
	{
	  send_to_char ("Clan channel is now OFF\n\r", ch);
	  SET_BIT (ch->comm, COMM_NOCLAN);
	}
      return;
    }
  if (IS_SET (ch->comm, COMM_NOCHANNELS))
    {
      send_to_char ("The gods have revoked your channel privileges.\n\r", ch);
      return;
    }
  REMOVE_BIT (ch->comm, COMM_NOCLAN);
  if (IS_SET (ch->comm2, COMM_RANTONLY))
  {
	  send_to_char("The gods have decreed you may only use the Rant Channel.\n\r", ch);
	  return;
  }

  //IBLIS 5/31/03 - Added Drunk Talk to clan talk
  if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    drunkify (ch, argument);
  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
	  cj_self(ch,argument);
  else
  {
    sprintf (buf, "%s `OYou`a: '`O%s`a'``\n\r", get_clan_symbol_ch (ch),
	   argument);
    send_to_char (buf, ch);
  }
  
  if (ch->race == PC_RACE_NIDAE)
    nidaeify(ch,bufn,argument);
  else anti_nidaeify(ch,bufn,argument);
  
  sprintf (buf, "%s `O$n`a: '`O$t`a'``", get_clan_symbol_ch (ch));

  // Changed on June 24. 2000 to support the command/channel hearclan
  if (!IS_SET (ch->comm2, COMM_SHUTUP))
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if ((d->connected == CON_PLAYING && d->character != ch
	   && is_same_clan (ch, d->character)
	   && !IS_SET (d->character->comm, COMM_NOCLAN)
	   && !IS_SET (d->character->comm, COMM_QUIET))
	  || (d->connected == CON_PLAYING && d->character != ch
	      && !IS_SET (d->character->comm, COMM_HEARCLAN)
	      && !IS_SET (d->character->comm, COMM_QUIET)
	      && IS_IMMORTAL (d->character) && d->character->level > 94))
	{
	      if (is_ignoring(d->character, ch)) 
              continue;
          if ((d->character->race == PC_RACE_NIDAE && !IS_IMMORTAL(d->character))
		  || (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(d->character)))
	  {
  	    if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(d->character))
	      cj_channel(d->character,bufn);
	    else act_new (buf, ch, bufn, d->character, TO_VICT, POS_DEAD);
	  }
	  else 
	  {
	    if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(d->character))
	      cj_channel(d->character,argument);
	    else act_new (buf, ch, argument, d->character, TO_VICT, POS_DEAD);
	  }
	}
    }
  return;
}


// This command is used for sending messages on clan channels
// Added by Morgan on June 26. 2000
void do_sendclan (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  char arg1[MAX_INPUT_LENGTH];
  int clan;

  // snarf first argument and look it up in the clan table
  argument = one_argument (argument, arg1);
  clan = clan_lookup (arg1);

  /* if it doesn't find a matching clan, tell the user */
  if (clan == CLAN_BOGUS)
    {
      send_to_char ("Sorry! No such clan exists!!!\n\r", ch);
      return;
    }

  /*
   * set up the message that needs to be sent to the players in the clan
   * echo it to yourself to see yourself writing..
   */
  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
	  cj_self(ch,argument);
  else
  {
    sprintf (buf, "%s `OYou`a: '`O%s`a'``\n\r", get_clan_symbol (clan),
	   argument);
    send_to_char (buf, ch);
  }

  // set up the message that goes to others
  sprintf (buf, "%s `O$n`a: '`O$t`a'``", get_clan_symbol (clan));

/*
 *  look around the world for targets and send it to them.
 *  sendclan is special in that way, because as an immortal
 *  we don't care about the status of the player, as long
 *  as they are members of the targeted clan
 */

  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if ((d->connected == CON_PLAYING && d->character != ch
	   && is_clan_member (d->character, clan)
	   && !IS_SET (d->character->comm, COMM_NOCLAN)
	   && !IS_SET (d->character->comm, COMM_QUIET))
	  || (d->connected == CON_PLAYING && d->character != ch
	      && !IS_SET (d->character->comm, COMM_HEARCLAN)
	      && !IS_SET (d->character->comm, COMM_QUIET)
	      && IS_IMMORTAL (d->character) && d->character->level > 94))
	{
  	  if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(d->character))
	    cj_channel(d->character,argument);
	  else act_new (buf, ch, argument, d->character, TO_VICT, POS_DEAD);
	}
    }
  return;
}



//
// Send a message on a clan war channel as a specific clan member
//
void do_sendcw (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int slot;
  char arg1[MAX_INPUT_LENGTH];
  CHAR_DATA *player;

  // snarf first argument and look it up in the clan table
  argument = one_argument (argument, arg1);
  slot = clan_lookup (arg1);

  /* if it doesn't find a matching clan, tell the user */
  if (slot == CLAN_BOGUS)
    {
      send_to_char ("Sorry! No such clan exists!!!\n\r", ch);
      return;
    }

  // their clan better be an active clan
  if (clan_table[slot].status < CLAN_ACTIVE)
    {
      send_to_char ("That clan is not a full clan.\n\r", ch);
      return;
    }
  // what is the text
  if (argument[0] == '\0')
    {
      send_to_char ("What would you like to say to the allies?\n\r", ch);
      return;
    }

  // they better have allies
  if (!clan_has_allies (slot))
    {
      send_to_char ("But, that clan has no allies.\n\r", ch);
      return;
    }

  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
	  cj_self(ch,argument);
  else
  {
    sprintf (buf, "[CLANWAR] %s `OYou`a: '`O%s`a'``\n\r",get_clan_symbol(slot), argument);
    send_to_char (buf, ch);
  }

  sprintf (buf, "[CLANWAR] %s `O$n`a: '`O$t`a'``", get_clan_symbol(slot));
  player = char_list;
  while (player)
    {
      if ((!IS_NPC (player) &&
	   (ch != player) && ((slot == player->clan)
			      || is_ally (slot, player->clan)))
	  || (!IS_NPC (player)
	      && (IS_IMMORTAL (player)
		  && !IS_SET (player->comm, COMM_HEARCLAN))))
	{
  	  if (player->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(player))
	    cj_channel(player,argument);
	  else act_new (buf, ch, argument, player, TO_VICT, POS_DEAD);
	}
      player = player->next;
    }
}


void do_immtalk (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  if (argument[0] == '\0')
    {
      if (IS_SET (ch->comm, COMM_NOWIZ))
	{
	  send_to_char ("Immortal channel is now ON\n\r", ch);
	  REMOVE_BIT (ch->comm, COMM_NOWIZ);
	}

      else
	{
	  send_to_char ("Immortal channel is now OFF\n\r", ch);
	  SET_BIT (ch->comm, COMM_NOWIZ);
	}
      return;
    }
  REMOVE_BIT (ch->comm, COMM_NOWIZ);
  sprintf (buf, "`l$n`a: `R%s``", argument);
  if (IS_IMMORTAL(ch))
  {
   act_new ("`a[`RImm`a] `l$n`a: `R$t``", ch, argument, NULL, TO_CHAR,
	   POS_DEAD);
  }
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->connected == CON_PLAYING && IS_IMMORTAL (d->character)
	  && !IS_SET (d->character->comm, COMM_NOWIZ))
	{
	  act_new ("`a[`RImm`a] `l$n`a: `R$t``", ch, argument,
		   d->character, TO_VICT, POS_DEAD);
	}
    }
  return;
}

void do_showdamage (CHAR_DATA * ch, char *argument)
{
      if (IS_SET (ch->comm2, COMM_SHOWDAMAGE))
	{
	  send_to_char ("You no longer see damage count!\n\r", ch);
	  REMOVE_BIT (ch->comm2, COMM_SHOWDAMAGE);
	}
      else
	{
	  send_to_char ("You now see damage numbers!\n\r", ch);
	  SET_BIT (ch->comm2, COMM_SHOWDAMAGE);
	}
  return;
}




/* 
 * An imm channel for level 100's to communicate in private
 * added by Morgan on June 24. 2000
 */
void do_admin (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  if (argument[0] == '\0')
    {
      if (IS_SET (ch->comm, COMM_NOADMIN))
	{
	  send_to_char ("Administrator channel is now ON\n\r", ch);
	  REMOVE_BIT (ch->comm, COMM_NOADMIN);
	}

      else
	{
	  send_to_char ("Administrator channel is now OFF\n\r", ch);
	  SET_BIT (ch->comm, COMM_NOADMIN);
	}
      return;
    }
  REMOVE_BIT (ch->comm, COMM_NOADMIN);
  sprintf (buf, "$n: %s", argument);
  act_new ("`a[`SAdm`a] `l$n`a: `S$t``", ch, argument, NULL, TO_CHAR,
	   POS_DEAD);
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->connected == CON_PLAYING && IS_IMMORTAL (d->character)
	  && !IS_SET (d->character->comm, COMM_NOADMIN)
	  && d->character->level > 99)
	{
	  act_new ("`a[`SAdm`a] `l$n`a: `S$t``", ch, argument,
		   d->character, TO_VICT, POS_DEAD);
	}
    }
  return;
}

void do_say (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *vch;
  char talktext[MAX_STRING_LENGTH];
  char bufn[MAX_STRING_LENGTH];

//  int i=0;
  if (IS_SET (ch->in_room->room_flags, ROOM_SILENT) && !IS_IMMORTAL (ch))
    {
      send_to_char
	("This room is shielded from disturbance by powerful wards of magic.\n\r",
	 ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("Say what?\n\r", ch);
      return;
    }
  // End of Minax Changes
  if (ch->race == 7 && !IS_IMMORTAL (ch))
    {
      if (ch->race >= 40 && ch->move >= 10 && ch->mana >= 10)
	{
	  ch->hit = ch->hit - (ch->level / 2);
	  if (ch->hit < 0)
	    {
	      send_to_char
		("Your speaking efforts have exhausted you to the point of death.\n\r",
		 ch);
	      raw_kill (ch, ch);
	    }
	}

      else
	{
	  act
	    ("The fragile structure of your vocal system prevents you from creating anything other than a high-pitched whine.",
	     ch, NULL, NULL, TO_CHAR);
	  return;
	}
    }
   if (ch->race == PC_RACE_LITAN && !IS_IMMORTAL (ch))
   {
     do_shout(ch,argument);
     return;
   }

   if (IS_SET(ch->act2,ACT_PUPPET))
   {
     do_switchtalk(ch,argument);
     return;
   }
    
  // IBLIS 5/31/03 - People wanted drunk talk (why who knows) so I deliver
  if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    {
      drunkify (ch, argument);
    }
  // Minax 6/29/02--When Vro'aths talk, they grunt and when Canthi talk,
  // they gurgle.
  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
    cj_self(ch,argument);
  if (ch->race == 4 && !IS_IMMORTAL (ch))
    {
      strcpy(bufn,"gurgles");
      if (!(ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch)))
        act ("You gurgle `a'`K$T`a'``", ch, NULL, argument, TO_CHAR);
    }

  else if (ch->race == 5 && !IS_IMMORTAL (ch))
    {
      strcpy(bufn,"grunt");
       if (!(ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch)))
      act ("You grunt `a'`K$T`a'``", ch, NULL, argument, TO_CHAR);
    }
  else if (ch->race == PC_RACE_NIDAE && !IS_IMMORTAL (ch))
    {
      strcpy(bufn,"sings");
       if (!(ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch)))
      act ("You sing `a'`K$T`a'``", ch, NULL, argument, TO_CHAR);
    }
  else
    {
      strcpy(bufn,"says");
       if (!(ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch)))
      act ("You say `a'`K$T`a'``", ch, NULL, argument, TO_CHAR);
    }
  sprintf (talktext,"$n %s `a'`K$t`a'``",bufn);
  if (ch->race == PC_RACE_NIDAE)
    nidaeify(ch,bufn,argument);
  else anti_nidaeify(ch,bufn,argument);
  for (vch = ch->in_room->people;vch != NULL;vch = vch->next_in_room)
    {
      if (ch == vch)
        continue;
      if ((vch->race == PC_RACE_NIDAE && !IS_IMMORTAL(vch))
	      || (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(vch)))
      {
        if (vch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(vch))
           cj_channel(vch,bufn);
	else act (talktext, ch, bufn, vch, TO_VICT);
      }
      else 
      {
	if (vch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(vch))
           cj_channel(vch,argument);
	else act (talktext, ch, argument, vch, TO_VICT);
      }
    }
  if (!IS_NPC (ch))
    for (vch = ch->in_room->people; vch; vch = vch->next_in_room)
      {
	if (vch == ch || !IS_NPC (vch) || get_position(vch) == POS_SLEEPING)
	  continue;
	strcpy (talktext, argument);
	mob_talk (ch, vch, talktext, 1);
      }
  trip_triggers_arg(ch, OBJ_TRIG_CHAR_VOCALIZE, NULL, NULL, OT_VOC_SAY, argument);
  return;
}

void drunkify (CHAR_DATA * ch, char *argument)
{
  int i = 0;
  if (IS_NPC (ch))
    return;
  for (i = 0; i < strlen (argument); i++)
    {
      if (number_percent () <
	  (((ch->pcdata->condition[COND_DRUNK] - 10) * .5) + 5))
	{
	  if (argument[i] < 'a' || argument[i] > 'z')
	    argument[i] = 'a';
	  argument[i] = (char) ((argument[i] + number_percent ()) % 26) + 97;
	  if (number_percent () < 10)
	    argument[i] -= 32;
	}

      else if (argument[i] >= 'a' && argument[i] <= 'z'
	       && number_percent () <
	       (ch->pcdata->condition[COND_DRUNK] + 25))
	argument[i] -= 32;
    }
}

char* nidaeify (CHAR_DATA *ch, char *argument, char *bufn)
{
  int i = 0;
  strcpy(argument,bufn);
  if (IS_IMMORTAL(ch))
    return bufn;
//  static char buf[MAX_STRING_LENGTH];
  for (i = 0; i < strlen (argument); i++)
    {
      if (argument[i] != ' ')
	{
	switch(number_range(1,5))
	  {
	  case 1: bufn[i] = 'a';break;
	  case 2: bufn[i] = 'e';break;
	  case 3: bufn[i] = 'i';break;
	  case 4: bufn[i] = 'o';break;
	  case 5: bufn[i] = 'u';break;
	  }
	}
      else bufn[i] = ' ';
    }
  return bufn;
}

char* anti_nidaeify (CHAR_DATA *ch, char *argument, char *bufn)
{
  int i = 0;
  strcpy (argument,bufn);
  if (IS_IMMORTAL(ch))
    return bufn;
  for (i = 0; i < strlen (argument); i++)
    {
      if (number_percent () <
          ((100 * .5) + 5))
        {
          if (argument[i] < 'a' || argument[i] > 'z')
            argument[i] = 'a';
          argument[i] = (char) ((argument[i] + number_percent ()) % 26) + 97;
          if (number_percent () < 10)
            argument[i] -= 32;
        }

      else if (argument[i] >= 'a' && argument[i] <= 'z'
               && number_percent () <
               (80))
        argument[i] -= 32;
    }
  return bufn;
}


void do_shout (CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  char bufn[MAX_STRING_LENGTH];
  if (argument[0] == '\0')
    {
      if (IS_SET (ch->comm, COMM_SHOUTSOFF))
	{
	  send_to_char ("You can hear shouts again.\n\r", ch);
	  REMOVE_BIT (ch->comm, COMM_SHOUTSOFF);
	}

      else
	{
	  send_to_char ("You will no longer hear shouts.\n\r", ch);
	  SET_BIT (ch->comm, COMM_SHOUTSOFF);
	}
      return;
    }
  if (IS_SET (ch->comm, COMM_NOSHOUT))
    {
      send_to_char ("You can't shout.\n\r", ch);
      return;
    }
  if (IS_SET (ch->in_room->room_flags, ROOM_SILENT) && !IS_IMMORTAL (ch))
    {
      send_to_char ("Sssshhhh... Silence is a must in this room\n\r", ch);
      return;
    }
  REMOVE_BIT (ch->comm, COMM_SHOUTSOFF);
  if (ch->level < 3)
    {
      send_to_char
	("Sorry, you are too young to project your voice across the lands in that manner.\n\r",
	 ch);
      return;
    }
  if (ch->race == 7 && !IS_IMMORTAL (ch))
    {
      act
	("The fragile structure of your vocal system prevents you from creating anything other than a high-pitched whine.",
	 ch, NULL, NULL, TO_CHAR);
      return;
    }
  //IBLIS 5/31/03 - Added Drunk Talk to shouting
  if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    drunkify (ch, argument);
  WAIT_STATE (ch, 12);
  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
    cj_self(ch,argument);
  else if (ch->race == PC_RACE_LITAN)
    act ("You shriek '$T'", ch, NULL, argument, TO_CHAR);	  
  else act ("You shout '$T'", ch, NULL, argument, TO_CHAR);
  if (ch->race == PC_RACE_NIDAE)
    nidaeify(ch,bufn,argument);
  else anti_nidaeify(ch,bufn,argument);
  if (!IS_SET (ch->comm2, COMM_SHUTUP))
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      CHAR_DATA *victim;
      victim = d->original ? d->original : d->character;
      if (d->connected == CON_PLAYING && d->character != ch
	  && !IS_SET (victim->comm, COMM_SHOUTSOFF)
	  && !IS_SET (victim->comm, COMM_QUIET))
	{
	      if (is_ignoring(victim, ch)) 
              continue;
          if (ch->race == PC_RACE_LITAN)
	  {
  	    if (d->character->race == PC_RACE_NIDAE)
	    {
 	      if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(d->character))
	        cj_channel(d->character,bufn);
	      else act ("$n shrieks '$t'", ch, bufn, d->character, TO_VICT);
	    }
	    else 
	    {
	      if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(d->character))
	        cj_channel(d->character,argument);
	      else act ("$n shrieks '$t'", ch, argument, d->character, TO_VICT);
	    }
	  }
	  else 
	  {
  	    if ((d->character->race == PC_RACE_NIDAE && !IS_IMMORTAL(victim))
			    || (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(victim)))
	    {
 	      if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(d->character))
	        cj_channel(d->character,bufn);
	      else act ("$n shouts '$t'", ch, bufn, d->character, TO_VICT);
	    }
	    else  
	    {
	      if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(d->character))
   	        cj_channel(d->character,argument);
	      else act ("$n shouts '$t'", ch, argument, d->character, TO_VICT);
	    }
	  }
	}
    }
   trip_triggers_arg(ch, OBJ_TRIG_CHAR_VOCALIZE, NULL, NULL, OT_VOC_SHOUT, argument);
  return;
}

FAKE_DATA *get_fake (char *str)
{
  FAKE_DATA *fake_ch;
  for (fake_ch = fake_list; fake_ch != NULL; fake_ch = fake_ch->next)
    if (!str_cmp (str, fake_ch->name))
      return (fake_ch);
  return (NULL);
}

void do_tell (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  char tcbuf[MAX_INPUT_LENGTH], bigbuf[MAX_STRING_LENGTH];
  char message[MAX_INPUT_LENGTH];
  char *content;
  CHAR_DATA *victim;
  bool last = FALSE;
  strcpy (tcbuf, argument);
  if (IS_SET (ch->comm, COMM_NOTELL) || IS_SET (ch->comm, COMM_DEAF))
    {
      send_to_char ("Your message didn't get through.\n\r", ch);
      return;
    }
  if (IS_SET (ch->comm, COMM_QUIET))
    {
      send_to_char ("You must turn off quiet mode first.\n\r", ch);
      return;
    }
  if (IS_SET (ch->comm, COMM_DEAF))
    {
      send_to_char ("You must turn off deaf mode first.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg);
  if (arg[0] == '\0' || argument[0] == '\0')
    {
      send_to_char ("Tell whom what?\n\r", ch);
      return;
    }
  if (arg[0] == '[')
    {
      char *cptr, *cptr2;
      if ((cptr = strchr (tcbuf, ']')) == NULL)
	{
	  send_to_char ("Didn't understand name list.\n\r", ch);
	  return;
	}
      *cptr = '\0';
      strcpy (message, cptr + 1);
      cptr = tcbuf + 1;
      while (1)
	{
	  cptr2 = strchr (cptr, ',');
	  if (cptr2 == NULL)
	    {
	      last = TRUE;
	      cptr2 = strchr (tcbuf, '\0');
	    }
	  *cptr2 = '\0';

	  //IBLIS 5/31/03 - Added Drunk Talk to tells
	  if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	    drunkify (ch, message);
	  sprintf (bigbuf, "%s %s", cptr, message);
	  do_tell (ch, bigbuf);
	  if (last)
	    return;
	  cptr = cptr2 + 1;
	}
    }

  /*
   * Can tell to PC's anywhere, but NPC's only in same room.
   * -- Furey
   */
  if ((victim = get_char_world (ch, arg)) == NULL
      || (IS_NPC (victim) && victim->in_room != ch->in_room))
    {
      FAKE_DATA *fake_vict;
      if ((fake_vict = get_fake (arg)) != NULL)
	{
	  char fbuf[MAX_STRING_LENGTH];
	  sprintf (fbuf, "You tell %s `a'`L%s`a'``\n\r",
		   fake_vict->name, argument);
	  send_to_char (fbuf, ch);
	  return;
	}
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (victim->race == PC_RACE_SIDHE && !IS_IMMORTAL (victim) && !IS_IMMORTAL (ch) && !IS_SET(ch->act2,ACT_QUESTMASTER))
    {
      act
	("The Sidhe you are trying to speak to cannot be communicated with in this manner.",
	 ch, NULL, NULL, TO_CHAR);
      return;
    }
  if (ch->race == PC_RACE_SIDHE && !IS_IMMORTAL (ch))
    {
      act
	("The fragile structure of your vocal system prevents you from creating anything other than a high-pitched whine.",
	 ch, NULL, NULL, TO_CHAR);
      return;
    }
   if (ch->race == PC_RACE_LITAN && !IS_IMMORTAL (ch) && !IS_IMMORTAL(victim))
    {
         if (ch->hit < (ch->max_hit/15))
	 {
		 send_to_char("You do not have the strength to tell anyone anything.\n\r",ch);
		 return;
	 }
	 else ch->hit -= (ch->max_hit/15);
	 send_to_char("The pain is exquisite, but you manage to send the tell.\n\r",ch);
    }
    
  if (victim->desc == NULL && !IS_NPC (victim))
    {
      act ("$N seems to have misplaced $S link...try again later.", ch,
	   NULL, victim, TO_CHAR);
      if (victim->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
        return;
      sprintf (buf, "%s tells you `a'`L%s`a'``\n\r", PERS (ch, victim),
	       argument);
      buf[0] = UPPER (buf[0]);
      add_buf (victim->pcdata->buffer, buf);
      return;
    }
  if (!(IS_IMMORTAL (ch) && ch->level > LEVEL_IMMORTAL) && !IS_AWAKE (victim))
    {
      act ("$E can't hear you.", ch, 0, victim, TO_CHAR);
      return;
    }
  if (IS_NPC (ch) && !IS_AWAKE (victim))
    {
      act ("$E can't hear you.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if ((IS_SET (victim->comm, COMM_QUIET)
       || IS_SET (victim->comm, COMM_DEAF)) && !IS_IMMORTAL (ch))
    {
      act ("$E is not receiving tells.", ch, 0, victim, TO_CHAR);
      return;
    }
  if (IS_SET (victim->comm, COMM_AFK))
    {
      if (IS_NPC (victim))
	{
	  act ("$E is AFK, and not receiving tells.", ch, NULL, victim,
	       TO_CHAR);
	  return;
	}
      if (!IS_NPC(victim) && IS_IMMORTAL(victim) && !IS_NPC(ch) && gold_toll != 0)
	{
	  bool bdam = FALSE;
	  if (ch->gold >= gold_toll)
	    ch->gold -= gold_toll;
	  else if (ch->silver >= gold_toll*100)
	    ch->silver -= gold_toll*100;
	  else if (ch->pcdata->bank_gold >= gold_toll)
	    ch->pcdata->bank_gold -= gold_toll;
	  else if (ch->pcdata->bank_silver >= gold_toll*100)
	    ch->pcdata->bank_silver -= gold_toll*100;
      else
	{
	  bdam = TRUE;
	  damage (ch,ch,damage_toll,gsn_likobe,DAM_UNIQUE,FALSE);
	  sprintf(buf,"This tell cost you %ld damage.", damage_toll);
	  act (buf, ch, argument, victim, TO_CHAR);
	}
	  if (bdam == FALSE)
	    {
	      sprintf(buf,"This tell cost you %ld gold.", gold_toll);
	      act (buf, ch, argument, victim, TO_CHAR);
	    }
	}

      if (victim->afkmsg[0] == '\0')
	act ("$E is AFK, but your tell will go through when $E returns.", ch,
	     NULL, victim, TO_CHAR);

      else
	{
	  sprintf (buf, "%s is AFK: %s\n\r",
		   (is_affected (victim, gsn_mask)
		    && victim->mask !=
		    NULL) ? victim->mask : victim->name, victim->afkmsg);
	  send_to_char (buf, ch);
	}
      if (victim->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
        return;
      if (ch->race == PC_RACE_NIDAE)
        nidaeify(ch,buf,argument);
      else anti_nidaeify(ch,buf,argument);
      if ((victim->race == PC_RACE_NIDAE && !IS_IMMORTAL(victim))
	      || (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(victim)))
      {
        sprintf (buf, "%s tells you `a'`L%s`a'``\n\r",
	       (is_affected (ch, gsn_mask)
		&& ch->mask != NULL) ? ch->mask : PERS (ch, victim),
	       argument);
      }
      else 
      {
        sprintf (buf, "%s tells you `a'`L%s`a'``\n\r",
               (is_affected (ch, gsn_mask)
                 && ch->mask != NULL) ? ch->mask : PERS (ch, victim),
                argument);
      }
      buf[0] = UPPER (buf[0]);
      add_buf (victim->pcdata->buffer, buf);
      return;
    }

  if (is_ignoring(victim, ch)) 
    {
       sprintf(buf, "%s is ignoring you.\n\r", victim->name);
       send_to_char(buf, ch);
       return;
     }
  if (!IS_NPC(victim) && IS_IMMORTAL(victim) && !IS_NPC(ch) && gold_toll != 0)
    {
      bool bdam = FALSE;
      if (ch->gold >= gold_toll)
        ch->gold -= gold_toll;
      else if (ch->silver >= gold_toll*100)
        ch->silver -= gold_toll*100;
      else if (ch->pcdata->bank_gold >= gold_toll)
        ch->pcdata->bank_gold -= gold_toll;
      else if (ch->pcdata->bank_silver >= gold_toll*100)
	ch->pcdata->bank_silver -= gold_toll*100;
      else
      {
	bdam = TRUE;
	damage (ch,ch,damage_toll,gsn_likobe,DAM_UNIQUE,FALSE);
	if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
  	  cj_self(ch,argument);
	else
	{
	  sprintf(buf,"You tell $N (for %ld damage) `a'`L$t`a'``", damage_toll);
          act (buf, ch, argument, victim, TO_CHAR);
	}
      }
      if (bdam == FALSE)
      {
        if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
          cj_self(ch,argument);
	else
	{
	  sprintf(buf,"You tell $N (for %ld gold) `a'`L$t`a'``", gold_toll);
          act (buf, ch, argument, victim, TO_CHAR);
	}
      }
    }

    else if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
      cj_self(ch,argument);
    else if (!IS_NPC(victim) && victim->desc && victim->desc->editor != 0 && victim->desc->connected != CON_PLAYING)
      act ("You tell $N (who is building) `a'`L$t`a'``", ch, argument, victim, TO_CHAR);
    else if (victim->fighting != NULL)
      act ("You tell $N (who's in combat) `a'`L$t`a'``", ch, argument, victim, TO_CHAR);
    else act ("You tell $N `a'`L$t`a'``", ch, argument, victim, TO_CHAR);
  if (ch->race == PC_RACE_NIDAE)
    nidaeify(ch,buf,argument);
  else anti_nidaeify(ch,buf,argument);
  if ((victim->race == PC_RACE_NIDAE  && !IS_IMMORTAL(victim))
	  || (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(victim)))
    content = buf;
  else content = argument;
  {
    if (victim->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
      cj_channel(victim,content);
    else if (ch->fighting != NULL)
      act_new ("$n (who's in combat) tells you `a'`L$t`a'``", ch, content, victim, TO_VICT, POS_DEAD);
    else if (!IS_NPC(ch) && ch->desc->editor != 0)
      act_new ("$n (who is building) tells you `a'`L$t`a'``", ch, content, victim, TO_VICT, POS_DEAD);
    else act_new ("$n tells you `a'`L$t`a'``", ch, content, victim, TO_VICT, POS_DEAD);
  }
  victim->reply = ch;
  if (IS_NPC (victim) && !IS_NPC (ch))
    {
      if (get_position(victim) == POS_SLEEPING)
	{
	  act ("$N is sleeping, I don't think $E heard you.", ch, NULL,
	       victim, TO_CHAR);
	  return;
	}
      mob_talk (ch, victim, argument, 0);
    }
  return;
}

void do_telepath (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  char tcbuf[MAX_INPUT_LENGTH], bigbuf[MAX_STRING_LENGTH];
  char message[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  bool last = FALSE;
  if (ch->race != 7 && !IS_IMMORTAL (ch))
    {
      send_to_char ("You don't have the power to project thoughts.\n\r", ch);
      return;
    }
  strcpy (tcbuf, argument);
  if (IS_SET (ch->comm, COMM_NOTELL) || IS_SET (ch->comm, COMM_DEAF))
    {
      send_to_char ("Your telepathic message did not get through.\n\r", ch);
      return;
    }
  if (IS_SET (ch->comm, COMM_QUIET))
    {
      send_to_char ("You must turn off quiet mode first.\n\r", ch);
      return;
    }
  if (IS_SET (ch->comm, COMM_DEAF))
    {
      send_to_char ("You must turn off deaf mode first.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg);
  if (arg[0] == '\0' || argument[0] == '\0')
    {
      send_to_char ("Telepath whom what?\n\r", ch);
      return;
    }
  if (arg[0] == '[')
    {
      char *cptr, *cptr2;
      if ((cptr = strchr (tcbuf, ']')) == NULL)
	{
	  send_to_char ("Didn't understand name list.\n\r", ch);
	  return;
	}
      *cptr = '\0';
      strcpy (message, cptr + 1);
      cptr = tcbuf + 1;
      while (1)
	{
	  cptr2 = strchr (cptr, ',');
	  if (cptr2 == NULL)
	    {
	      last = TRUE;
	      cptr2 = strchr (tcbuf, '\0');
	    }
	  *cptr2 = '\0';

	  //IBLIS 5/31/03 - Added Drunk Talk to telepaths
	  if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	    drunkify (ch, message);
	  sprintf (bigbuf, "%s %s", cptr, message);
	  do_tell (ch, bigbuf);
	  if (last)
	    return;
	  cptr = cptr2 + 1;
	}
    }
  if ((victim = get_char_world (ch, arg)) == NULL
      || (IS_NPC (victim) && victim->in_room != ch->in_room))
    {
      FAKE_DATA *fake_vict;
      if ((fake_vict = get_fake (arg)) != NULL)
	{
	  char fbuf[MAX_STRING_LENGTH];
	  sprintf (fbuf, "You telepath to %s `a'`L%s`a'``\n\r",
		   fake_vict->name, argument);
	  send_to_char (fbuf, ch);
	  return;
	}
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (victim->race == PC_RACE_NIDAE && !IS_IMMORTAL(victim))
  {
    send_to_char ("You cannot send telepathic messages to that receiver.\n\r", ch);
    return;
  }
  if (victim->race != PC_RACE_SIDHE && !IS_IMMORTAL (victim) && !IS_NPC (victim))
    {
      if (get_curr_stat (ch, STAT_CHA) >= 20
	  && get_curr_stat (ch, STAT_INT) >= 21 && ch->level >= 30
	  && ch->mana >= 10 && ch->move != 10)
	{
	  ch->hit = ch->hit - (ch->level / 5);
	  if (ch->hit < 0)
	    {
	      send_to_char
		("Your telepathic efforts have exhausted you to the point of death.\n\r",
		 ch);
	      raw_kill (ch, ch);
	    }
	  ch->move = UMAX (ch->move - (ch->level / 5), 0);
	  ch->mana = UMAX (ch->mana - (ch->level / 5), 0);
	}

      else
	{
	  send_to_char
	    ("You cannot send telepathic messages to that receiver.\n\r", ch);
	  return;
	}
    }
  if (victim->desc == NULL && !IS_NPC (victim))
    {
      act ("$N seems to have misplaced $S link...try again later.", ch,
	   NULL, victim, TO_CHAR);
      if (victim->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
        return;
      sprintf (buf, "%s telepaths to you `a'`L%s`a'``\n\r",
	       PERS (ch, victim), argument);
      buf[0] = UPPER (buf[0]);
      add_buf (victim->pcdata->buffer, buf);
      return;
    }
  if (!(IS_IMMORTAL (ch) && ch->level > LEVEL_IMMORTAL) && !IS_AWAKE (victim))
    {
      act ("$E can't hear your telepathic message.", ch, 0, victim, TO_CHAR);
      return;
    }
  if (IS_NPC (ch) && !IS_AWAKE (victim))
    {
      act ("$E can't hear your telepathic message.", ch, NULL, victim,
	   TO_CHAR);
      return;
    }
  if ((IS_SET (victim->comm, COMM_QUIET)
       || IS_SET (victim->comm, COMM_DEAF)) && !IS_IMMORTAL (ch))
    {
      act ("$E is not receiving telepaths.", ch, 0, victim, TO_CHAR);
      return;
    }
  if (IS_SET (victim->comm, COMM_AFK))
    {
      if (IS_NPC (victim))
	{
	  act ("$E is AFK, and not receiving telepaths.", ch, NULL,
	       victim, TO_CHAR);
	  return;
	}
      if (!IS_NPC(victim) && IS_IMMORTAL(victim) && !IS_NPC(ch) && gold_toll != 0)
	{
	  bool bdam = FALSE;
	  if (ch->gold >= gold_toll)
	    ch->gold -= gold_toll;
	  else if (ch->silver >= gold_toll*100)
	    ch->silver -= gold_toll*100;
	  else if (ch->pcdata->bank_gold >= gold_toll)
	    ch->pcdata->bank_gold -= gold_toll;
	  else if (ch->pcdata->bank_silver >= gold_toll*100)
	    ch->pcdata->bank_silver -= gold_toll*100;
	  else
	    {
	      bdam = TRUE;
	      damage (ch,ch,damage_toll,gsn_likobe,DAM_UNIQUE,FALSE);
	      sprintf(buf,"This telepath cost you %ld damage.", damage_toll);
          act (buf, ch, argument, victim, TO_CHAR);
	    }
	  if (bdam == FALSE)
	    {
	      sprintf(buf,"This telepath cost you %ld gold.", gold_toll);
	      act (buf, ch, argument, victim, TO_CHAR);
	    }
	}
      
      if (victim->afkmsg[0] == '\0')
	act ("$E is AFK, but your telepath will go through when $E returns.",
	     ch, NULL, victim, TO_CHAR);

      else
	{
	  sprintf (buf, "%s is AFK: %s\n\r",
		   (is_affected (victim, gsn_mask)
		    && victim->mask !=
		    NULL) ? victim->mask : victim->name, victim->afkmsg);
	  send_to_char (buf, ch);
	}
      if (victim->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
        return;
      sprintf (buf, "%s telepaths to you `a'`L%s`a'``\n\r",
	       (is_affected (ch, gsn_mask)
		&& ch->mask != NULL) ? ch->mask : PERS (ch, victim),
	       argument);
      buf[0] = UPPER (buf[0]);
      if (!IS_NPC (victim))
	{
	  add_buf (victim->pcdata->buffer, buf);
	}
      return;
    }

  if (is_ignoring(victim, ch)) 
    {
       sprintf(buf, "%s is ignoring you.\n\r", victim->name);
       send_to_char(buf, ch);
       return;
     }

  if (!IS_NPC(victim) && IS_IMMORTAL(victim) && !IS_NPC(ch) && gold_toll != 0)
    {
      bool bdam = FALSE;
      if (ch->gold >= gold_toll)
        ch->gold -= gold_toll;
      else if (ch->silver >= gold_toll*100)
        ch->silver -= gold_toll*100;
      else if (ch->pcdata->bank_gold >= gold_toll)
        ch->pcdata->bank_gold -= gold_toll;
      else if (ch->pcdata->bank_silver >= gold_toll*100)
        ch->pcdata->bank_silver -= gold_toll*100;
      else
	{
	  bdam = TRUE;
	  damage (ch,ch,damage_toll,gsn_likobe,DAM_UNIQUE,FALSE);
	  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
	    cj_self(ch,argument);
	  else
	  {
	    sprintf(buf,"You telepath to $N (for %ld damage) `a'`L$t`a'``", damage_toll);
	    act (buf, ch, argument, victim, TO_CHAR);
	  }
	}
      if (bdam == FALSE)
	{
  	  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
	    cj_self(ch,argument);
	  else
	  {
  	    sprintf(buf,"You telepath to $N (for %ld gold) `a'`L$t`a'``", gold_toll);
	    act (buf, ch, argument, victim, TO_CHAR);
	  }
	}
    }
  else if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
	  cj_self(ch,argument);
  else if (!IS_NPC(victim) && victim->desc && victim->desc->editor != 0 && victim->desc->connected != CON_PLAYING)
    act ("You telepath to $N (who is building) `a'`L$t`a'``", ch, argument, victim, TO_CHAR);
  else if (victim->fighting != NULL)
    act ("You telepath to $N (who's in combat) `a'`L$t`a'``", ch, argument, victim, TO_CHAR);
  else act ("You telepath to $N `a'`L$t`a'``", ch, argument, victim, TO_CHAR);
  if (victim->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
    cj_channel(victim, argument);
  else if (!IS_NPC(ch) && ch->desc->editor != 0)
    act_new ("$n (who is building) telepaths to you `a'`L$t`a'``", ch, argument, victim,TO_VICT,POS_DEAD);
  else if (ch->fighting != NULL)
    act_new ("$n (who's in combat) telepaths to you `a'`L$t`a'``", ch, argument, victim,TO_VICT,POS_DEAD);
  else act_new ("$n telepaths to you `a'`L$t`a'``", ch, argument, victim,
	   TO_VICT, POS_DEAD);
  victim->reply = ch;
  if (IS_NPC (victim) && !IS_NPC (ch))
    {
      if (get_position(victim) == POS_SLEEPING)
	{
	  act
	    ("$N is sleeping, I don't think $E heard your telepathic message.",
	     ch, NULL, victim, TO_CHAR);
	  return;
	}
      mob_talk (ch, victim, argument, 0);
    }
  return;
}

void reply_tele (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  if (IS_NPC (ch))
    return;
  if (IS_SET (ch->comm, COMM_NOTELL))
    {
      send_to_char ("Your message didn't get through.\n\r", ch);
      return;
    }
  if ((victim = ch->reply) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (victim->desc == NULL && !IS_NPC (victim))
    {
      act ("$N seems to have misplaced $S link...try again later.", ch,
	   NULL, victim, TO_CHAR);
      sprintf (buf, "%s tells you `a'`L%s`a'``\n\r", PERS (ch, victim), argument);
      buf[0] = UPPER (buf[0]);
      add_buf (victim->pcdata->buffer, buf);
      return;
    }
  if (!IS_IMMORTAL (ch) && !IS_AWAKE (victim))
    {
      act ("$E can't hear you.", ch, 0, victim, TO_CHAR);
      return;
    }
  if ((IS_SET (victim->comm, COMM_QUIET)
       || IS_SET (victim->comm, COMM_DEAF)) && !IS_IMMORTAL (ch)
      && !IS_IMMORTAL (victim))
    {
      act_new ("$E is not receiving tells.", ch, 0, victim, TO_CHAR,
	       POS_DEAD);
      return;
    }
  if (!IS_IMMORTAL (victim) && !IS_AWAKE (ch))
    {
      send_to_char ("In your dreams, or what?\n\r", ch);
      return;
    }
  if (IS_SET (victim->comm, COMM_AFK))
    {
      if (IS_NPC (victim))
	{
	  act_new ("$E is AFK, and not receiving tells.", ch, NULL,
		   victim, TO_CHAR, POS_DEAD);
	  return;
	}
      if (!IS_NPC(victim) && IS_IMMORTAL(victim) && !IS_NPC(ch) && gold_toll != 0)
        {
          bool bdam = FALSE;
          if (ch->gold >= gold_toll)
            ch->gold -= gold_toll;
          else if (ch->silver >= gold_toll*100)
            ch->silver -= gold_toll*100;
          else if (ch->pcdata->bank_gold >= gold_toll)
            ch->pcdata->bank_gold -= gold_toll;
          else if (ch->pcdata->bank_silver >= gold_toll*100)
            ch->pcdata->bank_silver -= gold_toll*100;
          else
            {
              bdam = TRUE;
              damage (ch,ch,damage_toll,gsn_likobe,DAM_UNIQUE,FALSE);
              sprintf(buf,"This telepath cost you %ld damage.", damage_toll);
	      act (buf, ch, argument, victim, TO_CHAR);
            }
          if (bdam == FALSE)
            {
              sprintf(buf,"This telepath cost you %ld gold.", gold_toll);
              act (buf, ch, argument, victim, TO_CHAR);
            }
        }

      if (victim->afkmsg[0] != '\0')
	act_new
	  ("$E is AFK, but your tell will go through when $E returns.",
	   ch, NULL, victim, TO_CHAR, POS_DEAD);

      else
	{
	  sprintf (buf, "%s is AFK: %s\n\r",
		   (is_affected (victim, gsn_mask)
		    && victim->mask !=
		    NULL) ? victim->mask : victim->name, victim->afkmsg);
	  send_to_char (buf, ch);
	}
      if (victim->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
	return;
      sprintf (buf, "%s tells you `a'`L%s`a'``\n\r",
	       (is_affected (ch, gsn_mask)
		&& ch->mask != NULL) ? ch->mask : PERS (ch, victim),argument);
      buf[0] = UPPER (buf[0]);
      add_buf (victim->pcdata->buffer, buf);
      return;
    }


  if (is_ignoring(victim, ch)) 
    {
       sprintf(buf, "%s is ignoring you.\n\r", victim->name);
       send_to_char(buf, ch);
       return;
     }
  if (!IS_NPC(victim) && IS_IMMORTAL(victim) && !IS_NPC(ch) && gold_toll != 0)
    {
      bool bdam = FALSE;
      if (ch->gold >= gold_toll)
        ch->gold -= gold_toll;
      else if (ch->silver >= gold_toll*100)
        ch->silver -= gold_toll*100;
      else if (ch->pcdata->bank_gold >= gold_toll)
        ch->pcdata->bank_gold -= gold_toll;
      else if (ch->pcdata->bank_silver >= gold_toll*100)
        ch->pcdata->bank_silver -= gold_toll*100;
      else
        {
          bdam = TRUE;
          damage (ch,ch,damage_toll,gsn_likobe,DAM_UNIQUE,FALSE);
	  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
		  cj_self(ch,argument);
	  else 
	  {
            sprintf(buf,"You telepath to $N (for %ld damage) `a'`L$t`a'``", damage_toll);
           act (buf, ch, argument, victim, TO_CHAR);
	  }
        }
      if (bdam == FALSE)
        {
  	  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
	    cj_self(ch,argument);
	  else
	  {
            sprintf(buf,"You telepath to $N (for %ld gold) `a'`L$t`a'``", gold_toll);
            act (buf, ch, argument, victim, TO_CHAR);
	  }
        }
    }
  else if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
	  cj_self(ch,argument);
  else if (!IS_NPC(victim) && victim->desc && victim->desc->editor != 0)
    act_new ("You telepath to $N (who is building)`a'`L$t`a'``", ch, argument, victim, TO_CHAR, POS_DEAD);
  else if (victim->fighting != NULL)
    act_new ("You telepath to $N (who's in combat) `a'`L$t`a'``", ch, argument, victim, TO_CHAR, POS_DEAD);
  else act_new ("You telepath to $N `a'`L$t`a'``", ch, argument, victim, TO_CHAR, POS_DEAD);

  if (victim->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
    cj_channel(victim,argument);
  
  else if (!IS_NPC(ch) && ch->desc->editor != 0)
	  act_new ("$n (who is building) telepaths to you `a'`L$t`a'``", ch, argument, victim, TO_VICT, POS_DEAD);
  else if (ch->fighting != NULL)
	  act_new ("$n (who's in combat) telepaths to you `a'`L$t`a'``", ch, argument, victim, TO_VICT, POS_DEAD);
  else act_new ("$n telepaths to you `a'`L$t`a'``", ch, argument, victim, TO_VICT, POS_DEAD);
  victim->reply = ch;
  return;
}

void do_reply (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  char bufn[MAX_STRING_LENGTH];
  char* content;
	
  if (IS_NPC (ch))
    return;
  if (IS_SET (ch->comm, COMM_NOTELL))
    {
      send_to_char ("Your message didn't get through.\n\r", ch);
      return;
    }
  if (ch->race == PC_RACE_SIDHE)
    {
      reply_tele (ch, argument);
      return;
    }

  if ((victim = ch->reply) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (!IS_IMMORTAL (victim) && !IS_AWAKE (ch))
    {
      send_to_char ("In your dreams, or what?\n\r", ch);
      return;
    }
  
  if (ch->race == PC_RACE_LITAN && !IS_IMMORTAL (ch) && !IS_IMMORTAL(victim))
    {
      if (ch->hit < (ch->max_hit/15))
      {
        send_to_char("You do not have the strength to tell anyone anything.\n\r",ch);
        return;
      }
      else ch->hit -= (ch->max_hit/15);
      send_to_char("The pain is exquisite, but you manage to send the tell.\n\r",ch);
    }
   
  
  if (victim->race == PC_RACE_SIDHE && !IS_IMMORTAL (victim) && !IS_IMMORTAL (ch))
    {
      act
	("The Sidhe you are trying to speak to cannot be communicated with in this manner.",
	 ch, NULL, NULL, TO_CHAR);
      return;
    }
  if (victim->desc == NULL && !IS_NPC (victim))
    {
      act ("$N seems to have misplaced $S link...try again later.", ch,
	   NULL, victim, TO_CHAR);
      sprintf (buf, "%s tells you `a'`L%s`a'``\n\r", PERS (ch, victim),
	       argument);
      buf[0] = UPPER (buf[0]);
      add_buf (victim->pcdata->buffer, buf);
      return;
    }
  if (!IS_IMMORTAL (ch) && !IS_AWAKE (victim))
    {
      act ("$E can't hear you.", ch, 0, victim, TO_CHAR);
      return;
    }
  if ((IS_SET (victim->comm, COMM_QUIET)
       || IS_SET (victim->comm, COMM_DEAF)) && !IS_IMMORTAL (ch)
      && !IS_IMMORTAL (victim))
    {
      act_new ("$E is not receiving tells.", ch, 0, victim, TO_CHAR,
	       POS_DEAD);
      return;
    }
  if (IS_SET (victim->comm, COMM_AFK))
    {
      if (IS_NPC (victim))
	{
	  act_new ("$E is AFK, and not receiving tells.", ch, NULL,
		   victim, TO_CHAR, POS_DEAD);
	  return;
	}

      //Code for the Immortal toll command (costing players oney or life to tell an imm)
      if (!IS_NPC(victim) && IS_IMMORTAL(victim) && !IS_NPC(ch) && gold_toll != 0)
        {
          bool bdam = FALSE;
          if (ch->gold >= gold_toll)
            ch->gold -= gold_toll;
          else if (ch->silver >= gold_toll*100)
            ch->silver -= gold_toll*100;
          else if (ch->pcdata->bank_gold >= gold_toll)
            ch->pcdata->bank_gold -= gold_toll;
          else if (ch->pcdata->bank_silver >= gold_toll*100)
            ch->pcdata->bank_silver -= gold_toll*100;
      else
        {
          bdam = TRUE;
          damage (ch,ch,damage_toll,gsn_likobe,DAM_UNIQUE,FALSE);
          sprintf(buf,"This tell cost you %ld damage.", damage_toll);
          act (buf, ch, argument, victim, TO_CHAR);
        }
          if (bdam == FALSE)
            {
              sprintf(buf,"This tell cost you %ld gold.", gold_toll);
              act (buf, ch, argument, victim, TO_CHAR);
            }
        }

      if (victim->afkmsg[0] != '\0')
	act_new
	  ("$E is AFK, but your tell will go through when $E returns.",
	   ch, NULL, victim, TO_CHAR, POS_DEAD);

      else
	{
	  sprintf (buf, "%s is AFK: %s\n\r",
		   (is_affected (victim, gsn_mask)
		    && victim->mask !=
		    NULL) ? victim->mask : victim->name, victim->afkmsg);
	  send_to_char (buf, ch);
	}
      if (victim->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
        return;
      if (ch->race == PC_RACE_NIDAE)
        nidaeify(ch,bufn,argument);
      else anti_nidaeify(ch,bufn,argument);
      if ((victim->race == PC_RACE_NIDAE && !IS_IMMORTAL(victim))
	      || (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(victim)))
      {
        sprintf (buf, "%s tells you `a'`L%s`a'``\n\r", (is_affected (ch, gsn_mask) && ch->mask != NULL) ? ch->mask : PERS (ch, victim),buf);
      }
      else
      {
	  sprintf (buf, "%s tells you `a'`L%s`a'``\n\r",
               (is_affected (ch, gsn_mask)
                && ch->mask != NULL) ? ch->mask : PERS (ch, victim),
              argument);
      }
      buf[0] = UPPER (buf[0]);
      add_buf (victim->pcdata->buffer, buf);
      return;
    }

  if (is_ignoring(victim, ch)) 
    {
       sprintf(buf, "%s is ignoring you.\n\r", victim->name);
       send_to_char(buf, ch);
       return;
     }


  if (!IS_NPC(victim) && IS_IMMORTAL(victim) && !IS_NPC(ch) && gold_toll != 0)
    {
      bool bdam = FALSE;
      if (ch->gold >= gold_toll)
        ch->gold -= gold_toll;
      else if (ch->silver >= gold_toll*100)
        ch->silver -= gold_toll*100;
      else if (ch->pcdata->bank_gold >= gold_toll)
        ch->pcdata->bank_gold -= gold_toll;
      else if (ch->pcdata->bank_silver >= gold_toll*100)
        ch->pcdata->bank_silver -= gold_toll*100;
      else
	{
	  bdam = TRUE;
	  damage (ch,ch,damage_toll,gsn_likobe,DAM_UNIQUE,FALSE);
	  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
		  cj_self(ch,argument);
	  else
	  {
	    sprintf(buf,"You tell $N (for %ld damage) `a'`L$t`a'``", damage_toll);
	    act (buf, ch, argument, victim, TO_CHAR);
	  }
	}
      if (bdam == FALSE)
	{
  	  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
	    cj_self(ch,argument);
	  else
	  {
	    sprintf(buf,"You tell $N (for %ld gold) `a'`L$t`a'``", gold_toll);
	    act (buf, ch, argument, victim, TO_CHAR);
	  }
	}
    }

  else if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
    cj_self(ch,argument);
  else if (!IS_NPC(victim) && victim->desc && victim->desc->editor != 0)
    act_new ("You tell $N (who is building) `a'`L$t`a'``", ch, argument, victim, TO_CHAR, POS_DEAD);
  else if (victim->fighting != NULL)
    act_new ("You tell $N (who's in combat) `a'`L$t`a'``", ch, argument, victim, TO_CHAR, POS_DEAD);
  else act_new ("You tell $N `a'`L$t`a'``", ch, argument, victim, TO_CHAR, POS_DEAD);
 
  if (ch->race == PC_RACE_NIDAE)
    nidaeify(ch,bufn,argument);
  else anti_nidaeify(ch,bufn,argument);
  if ((victim->race == PC_RACE_NIDAE && !IS_IMMORTAL(victim))
         || (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(victim)))
    content = bufn;
  else content = argument;
  
  if (victim->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
    cj_channel(victim, content);
  else if (!IS_NPC(ch) && ch->desc->editor != 0)
    act_new ("$n (who is building) tells you `a'`L$t`a'``", ch, content, victim, TO_VICT,POS_DEAD);
  else if (ch->fighting != NULL)
    act_new ("$n (who's in combat) tells you `a'`L$t`a'``", ch, content, victim, TO_VICT,POS_DEAD);
  else act_new ("$n tells you `a'`L$t`a'``", ch, content, victim, TO_VICT, POS_DEAD);
  victim->reply = ch;
  return;
}

void do_yell (CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  char bufn[MAX_STRING_LENGTH];
  if (IS_SET (ch->in_room->room_flags, ROOM_SILENT) && !IS_IMMORTAL (ch))
    {
      send_to_char ("Ssshhh... Silence is a must here.\n\r", ch);
      return;
    }
  if (IS_SET (ch->comm, COMM_NOSHOUT))
    {
      send_to_char ("You can't yell.\n\r", ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("Yell what?\n\r", ch);
      return;
    }
  if (ch->race == PC_RACE_SIDHE && !IS_IMMORTAL (ch))
    {
      act
	("The fragile structure of your vocal system prevents you from creating anything other than a high-pitched whine.",
	 ch, NULL, NULL, TO_CHAR);
      return;
    }
   if (ch->race == PC_RACE_LITAN && !IS_IMMORTAL (ch))
    {
       do_shout(ch,argument);
       return;
    }
   
/*  if (ch->race > 11 && ch->race < 16)
    return;
  if (ch->race > 16 && ch->race < 19)
    return;
  if (ch->race > 21 && ch->race < 24)
    return;
  if (ch->race > 24 && ch->race < 30)
    return;
  if (ch->race > 30)
    return;*/

  //IBLIS 5/31/03 - Added Drunk Talk to yelling
  if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    drunkify (ch, argument);
  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
	  cj_self(ch,argument);
  else act ("You yell '$t'", ch, argument, NULL, TO_CHAR);

  if (ch->race == PC_RACE_NIDAE)
    nidaeify(ch,bufn,argument);
  else anti_nidaeify(ch,bufn,argument);
  
  if (!IS_SET (ch->comm2, COMM_SHUTUP))
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->connected == CON_PLAYING
	  && d->character != ch
	  && d->character->in_room != NULL
	  && d->character->in_room->area == ch->in_room->area
	  && !IS_SET (d->character->comm, COMM_QUIET))
	{
	      if (is_ignoring(d->character, ch)) 
              continue;
	  if ((d->character->race == PC_RACE_NIDAE && !IS_IMMORTAL(d->character))
				|| (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(d->character)))
	  {
  	    if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(d->character))
	      cj_channel(d->character,bufn);
	    else act ("$n yells '$t'", ch, bufn, d->character, TO_VICT);
  	  }
	    
	  else 
	  {
  	    if (d->character->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(d->character))
	      cj_channel(d->character,argument);
	    else act ("$n yells '$t'", ch, argument, d->character, TO_VICT);
	  }
	    
	}
    }
   trip_triggers_arg(ch, OBJ_TRIG_CHAR_VOCALIZE, NULL, NULL, OT_VOC_YELL, argument);
  return;
}

void do_emote (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *rch;
  if (!IS_NPC (ch) && IS_SET (ch->comm, COMM_NOEMOTE))
    {
      send_to_char ("You can't show your emotions.\n\r", ch);
      return;
    }
  if (IS_SET (ch->in_room->room_flags, ROOM_SILENT) && !IS_IMMORTAL (ch))
    {
      send_to_char
	("This room is shielded from disturbance by powerful wards of magic.\n\r",
	 ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("Emote what?\n\r", ch);
      return;
    }
	// Avariel 9/22/12 - Fuck this shit! 
 // if (ch->race == PC_RACE_LITAN && !IS_IMMORTAL (ch))
   // {
    //  send_to_char("You are not smart enough to know how to emote things, you can only shriek.\n\r",ch);
   //   return;
   // }
   
  //IBLIS 5/31/03 - Added Drunk Emoting
  if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    drunkify (ch, argument);
  for (rch = ch->in_room->people;rch != NULL;rch = rch->next_in_room)
  {
    if (rch == ch || rch->position < POS_RESTING)
      continue;
    if (!IS_IMMORTAL(rch))
      act("$n $t",ch,argument,rch,TO_VICT);
    else 
    {
      send_to_char("`j*``",rch);
      act("$n $t",ch,argument,rch,TO_VICT);
    }
  }
  //act ("$n $T", ch, NULL, argument, TO_ROOM);
  act ("$n $T", ch, NULL, argument, TO_CHAR);
  return;
}

void do_pmote (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *vch;
  char *letter, *name;
  char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
  int matches = 0;
  if (!IS_NPC (ch) && IS_SET (ch->comm, COMM_NOEMOTE))
    {
      send_to_char ("You can't show your emotions.\n\r", ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("Emote what?\n\r", ch);
      return;
    }
	// Avariel 9/22/12 - Fuck this shit! 
  //if (ch->race == PC_RACE_LITAN && !IS_IMMORTAL (ch))
   // {
   //   send_to_char("You are not smart enough to know how to emote things, you can only shriek.\n\r",ch);
   //   return;
   // }
   
  act ("`N$n $t``", ch, argument, NULL, TO_CHAR);
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (vch->desc == NULL || vch == ch)
	continue;
      if ((letter = strstr (argument, vch->name)) == NULL)
	{
          strcpy(temp,argument);
	  drunkify(ch,temp);
	  act ("`N$N $t``", vch, temp, ch, TO_CHAR);
	  continue;
	}
      strcpy (temp, argument);
      temp[strlen (argument) - strlen (letter)] = '\0';
      last[0] = '\0';
      name = vch->name;
      for (; *letter != '\0'; letter++)
	{
	  if (*letter == '\'' && matches == strlen (vch->name))
	    {
	      strcat (temp, "r");
	      continue;
	    }
	  if (*letter == 's' && matches == strlen (vch->name))
	    {
	      matches = 0;
	      continue;
	    }
	  if (matches == strlen (vch->name))
	    {
	      matches = 0;
	    }
	  if (*letter == *name)
	    {
	      matches++;
	      name++;
	      if (matches == strlen (vch->name))
		{
		  strcat (temp, "you");
		  last[0] = '\0';
		  name = vch->name;
		  continue;
		}
	      strncat (last, letter, 1);
	      continue;
	    }
	  matches = 0;
	  strcat (temp, last);
	  strncat (temp, letter, 1);
	  last[0] = '\0';
	  name = vch->name;
	}
      //IBLIS 5/31/03 - Added Drunk Emoting
      if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        drunkify (ch, temp);
      act ("$N $t", vch, temp, ch, TO_CHAR);
    }
  return;
}


/*
 * All the posing stuff.
 */
struct pose_table_type
{
  char *message[18*2];
};
const struct pose_table_type pose_table[MAX_CLASS] = {
  {
   { //Mage
    "You sizzle with energy.", 
    "$n sizzles with energy.",
    "You turn into a butterfly, then return to your normal shape.",
    "$n turns into a butterfly, then returns to $s normal shape.",
    "Blue sparks fly from your fingers.",
    "Blue sparks fly from $n's fingers.",
    "Little red lights dance in your eyes.",
    "Little red lights dance in $n's eyes.",
    "A slimy green monster appears before you and bows.",
    "A slimy green monster appears before $n and bows.",
    "You turn everybody into a little pink elephant.",
    "You are turned into a little pink elephant by $n.",
    "A small ball of light dances on your fingertips.",
    "A small ball of light dances on $n's fingertips.",
    "Smoke and fumes leak from your nostrils.",
    "Smoke and fumes leak from $n's nostrils.",
    "The light flickers as you rap in magical languages.",
    "The light flickers as $n raps in magical languages.",
    "Your head disappears.", 
    "$n's head disappears.",
    "A fire elemental singes your hair.",
    "A fire elemental singes $n's hair.",
    "The sky changes color to match your eyes.",
    "The sky changes color to match $n's eyes.",
    "The stones dance to your command.",
    "The stones dance to $n's command.",
    "The heavens and grass change colour as you smile.",
    "The heavens and grass change colour as $n smiles.",
    "Everyone's clothes are transparent, and you are laughing.",
    "Your clothes are transparent, and $n is laughing.",
    "A black hole swallows you.", 
    "A black hole swallows $n.",
    "The world shimmers in time with your whistling.",
    "The world shimmers in time with $n's whistling.",
    "A black hole swallows you.",
    "A black hole swallows $n.",
    }},
  {
   {  //Cleric
    "A cool breeze refreshes you.", 
    "A cool breeze refreshes $n.",
    "You feel very holy.",
    "$n looks very holy.",
    "You nonchalantly turn wine into water.",
    "$n nonchalantly turns wine into water."
    "A halo appears over your head.",
    "A halo appears over $n's head.",
    "You recite words of wisdom.", 
    "$n recites words of wisdom.",
    "Deep in prayer, you levitate.", 
    "Deep in prayer, $n levitates.",
    "An angel consults you.", 
    "An angel consults $n.",
    "Your body glows with an unearthly light.",
    "$n's body glows with an unearthly light.",
    "A spot light hits you.", 
    "A spot light hits $n.",
    "Everyone levitates as you pray.", 
    "You levitate as $n prays.",
    "The sun pierces through the clouds to illuminate you.",
    "The sun pierces through the clouds to illuminate $n.",
    "The ocean parts before you.", 
    "The ocean parts before $n.",
    "A thunder cloud kneels to you.",
    "A thunder cloud kneels to $n.",
    "The Burning Man speaks to you.",
    "The Burning Man speaks to $n.",
    "An eye in a pyramid winks at you.",
    "An eye in a pyramid winks at $n.",
    "Valentine Michael Smith offers you a glass of water.",
    "Valentine Michael Smith offers $n a glass of water.",
    "Some big dude in the sky gives you a staff.",
    "Some big dude in the sky gives $n a staff.", 
    "Click.",
    "Click.",
    }},
  {
   { //Thief
    "You perform a small card trick.",
    "$n performs a small card trick.",
    "You wiggle your ears alternately.",
    "$n wiggles $s ears alternately.",
    "You nimbly tie yourself into a knot.",
    "$n nimbly ties $mself into a knot.",
    "You juggle with daggers, apples, and eyeballs.",
    "$n juggles with daggers, apples, and eyeballs.",
    "You steal the underwear off every person in the room.",
    "Your underwear is gone!  $n stole it!",
    "The dice roll ... and you win again.",
    "The dice roll ... and $n wins again.",
    "You count the money in everyone's pockets.",
    "Check your money, $n is counting it.",
    "You balance a pocket knife on your tongue.",
    "$n balances a pocket knife on your tongue.",
    "You produce a coin from everyone's ear.",
    "$n produces a coin from your ear.",
    "Your eyes dance with greed.", 
    "$n's eyes dance with greed.",
    "You deftly steal everyone's weapon.",
    "$n deftly steals your weapon.",
    "The Grey Mouser buys you a beer.",
    "The Grey Mouser buys $n a beer.",
    "Everyone's pocket explodes with your fireworks.",
    "Your pocket explodes with $n's fireworks.",
    "Everyone discovers your dagger a centimeter from their eye.",
    "You discover $n's dagger a centimeter from your eye.",
    "Where did you go?", 
    "Where did $n go?",
    "Click.",
    "Click.",
    "You step behind your shadow.", 
    "$n steps behind $s shadow.",
    "Click.",
    "Click.",
    }},
  {
   { //Warrior
    "You hit your head, and your eyes roll.",
    "$n hits $s head, and $s eyes roll.",
    "You show your bulging muscles.",
    "$n shows $s bulging muscles.",
    "You crack nuts between your fingers.",
    "$n cracks nuts between $s fingers.",
    "You grizzle your teeth and look mean.",
    "$n grizzles $s teeth and looks mean.",
    "Crunch, crunch -- you munch a bottle.",
    "Crunch, crunch -- $n munches a bottle.",
    "... 98, 99, 100 ... you do pushups.",
    "... 98, 99, 100 ... $n does pushups.",
    "Arnold Schwarzenegger admires your physique.",
    "Arnold Schwarzenegger admires $n's physique.",
    "Watch your feet, you are juggling granite boulders.",
    "Watch your feet, $n is juggling granite boulders.",
    "Oomph!  You squeeze water out of a granite boulder.",
    "Oomph!  $n squeezes water out of a granite boulder.",
    "You pick your teeth with a spear.",
    "$n picks $s teeth with a spear.",
    "Everyone is swept off their foot by your hug.",
    "You are swept off your feet by $n's hug.",
    "Your karate chop splits a tree.",
    "$n's karate chop splits a tree.",
    "A strap of your armor breaks over your mighty thews.",
    "A strap of $n's armor breaks over $s mighty thews.",
    "A boulder cracks at your frown.",
    "A boulder cracks at $n's frown.",
    "Mercenaries arrive to do your bidding.",
    "Mercenaries arrive to do $n's bidding.",
    "Four matched Percherons bring in your chariot.",
    "Four matched Percherons bring in $n's chariot.",
    "Atlas asks you to relieve him.",
    "Atlas asks $n to relieve him.",
    "Click.",
    "Click.",
    }},
  {
	  
   { //Ranger
    "You show your bulging muscles.",
    "$n shows $s bulging muscles.",
    "You crack nuts between your fingers.",
    "$n cracks nuts between $s fingers.",
    "You grizzle your teeth and look mean.",
    "$n grizzles $s teeth and looks mean.",
    "You hit your head, and your eyes roll.",
    "$n hits $s head, and $s eyes roll.",
    "Crunch, crunch -- you munch a bottle.",
    "Crunch, crunch -- $n munches a bottle.",
    "... 98, 99, 100 ... you do pushups.",
    "... 98, 99, 100 ... $n does pushups.",
    "Arnold Schwarzenegger admires your physique.",
    "Arnold Schwarzenegger admires $n's physique.",
    "Watch your feet, you are juggling granite boulders.",
    "Watch your feet, $n is juggling granite boulders.",
    "Oomph!  You squeeze water out of a granite boulder.",
    "Oomph!  $n squeezes water out of a granite boulder.",
    "You pick your teeth with a spear.",
    "$n picks $s teeth with a spear.",
    "Everyone is swept off their foot by your hug.",
    "You are swept off your feet by $n's hug.",
    "Your karate chop splits a tree.",
    "$n's karate chop splits a tree.",
    "A strap of your armor breaks over your mighty thews.",
    "A strap of $n's armor breaks over $s mighty thews.",
    "A boulder cracks at your frown.",
    "A boulder cracks at $n's frown.",
    "Mercenaries arrive to do your bidding.",
    "Mercenaries arrive to do $n's bidding.",
    "Four matched Percherons bring in your chariot.",
    "Four matched Percherons bring in $n's chariot.",
    "Atlas asks you to relieve him.",
    "Atlas asks $n to relieve him.",
    "Click.",
    "Click.",
    }},
  {
   {  //Bard
    "You perform a small card trick.",
    "$n performs a small card trick.",
    "You wiggle your ears alternately.",
    "$n wiggles $s ears alternately.",
    "You nimbly tie yourself into a knot.",
    "$n nimbly ties $mself into a knot.",
    "You juggle with daggers, apples, and eyeballs.",
    "$n juggles with daggers, apples, and eyeballs.",
    "You steal the underwear off every person in the room.",
    "Your underwear is gone!  $n stole it!",
    "The dice roll ... and you win again.",
    "The dice roll ... and $n wins again.",
    "You count the money in everyone's pockets.",
    "Check your money, $n is counting it.",
    "You balance a pocket knife on your tongue.",
    "$n balances a pocket knife on your tongue.",
    "You produce a coin from everyone's ear.",
    "$n produces a coin from your ear.",
    "You step behind your shadow.", 
    "$n steps behind $s shadow.",
    "Your eyes dance with greed.", 
    "$n's eyes dance with greed.",
    "You deftly steal everyone's weapon.",
    "$n deftly steals your weapon.",
    "The Grey Mouser buys you a beer.",
    "The Grey Mouser buys $n a beer.",
    "Everyone's pocket explodes with your fireworks.",
    "Your pocket explodes with $n's fireworks.",
    "Everyone discovers your dagger a centimeter from their eye.",
    "You discover $n's dagger a centimeter from your eye.",
    "Where did you go?", 
    "Where did $n go?",
    "Click.",
    "Click.",
    }},
  {
	  
   { //Paladin
     "You hit your head, and your eyes roll.",
     "$n hits $s head, and $s eyes roll.",
     "You show your bulging muscles.",
     "$n shows $s bulging muscles.",
     "You crack nuts between your fingers.",
     "$n cracks nuts between $s fingers.",
     "You grizzle your teeth and look mean.",
     "$n grizzles $s teeth and looks mean.",
     "Crunch, crunch -- you munch a bottle.",
     "Crunch, crunch -- $n munches a bottle.",
     "... 98, 99, 100 ... you do pushups.",
     "... 98, 99, 100 ... $n does pushups.",
     "Arnold Schwarzenegger admires your physique.",
     "Arnold Schwarzenegger admires $n's physique.",
     "Watch your feet, you are juggling granite boulders.",
     "Watch your feet, $n is juggling granite boulders.",
     "Oomph!  You squeeze water out of a granite boulder.",
     "Oomph!  $n squeezes water out of a granite boulder.",
     "You pick your teeth with a spear.",
     "$n picks $s teeth with a spear.",
     "Everyone is swept off their foot by your hug.",
     "You are swept off your feet by $n's hug.",
     "Your karate chop splits a tree.",
     "$n's karate chop splits a tree.",
     "A strap of your armor breaks over your mighty thews.",
     "A strap of $n's armor breaks over $s mighty thews.",
     "A boulder cracks at your frown.",
     "A boulder cracks at $n's frown.",
     "Mercenaries arrive to do your bidding.",
     "Mercenaries arrive to do $n's bidding.",
     "Four matched Percherons bring in your chariot.",
     "Four matched Percherons bring in $n's chariot.",
     "Atlas asks you to relieve him.",
    "Atlas asks $n to relieve him.", 
    "Click.",
    "Click.",
   }},
  {
   { //Assassin
    "You perform a small card trick.",
    "$n performs a small card trick.",
    "You wiggle your ears alternately.",
    "$n wiggles $s ears alternately.",
    "You nimbly tie yourself into a knot.",
    "$n nimbly ties $mself into a knot.",
    "You juggle with daggers, apples, and eyeballs.",
    "$n juggles with daggers, apples, and eyeballs.",
    "You steal the underwear off every person in the room.",
    "Your underwear is gone!  $n stole it!",
    "The dice roll ... and you win again.",
    "The dice roll ... and $n wins again.",
    "You count the money in everyone's pockets.",
    "Check your money, $n is counting it.",
    "You balance a pocket knife on your tongue.",
    "$n balances a pocket knife on your tongue.",
    "You produce a coin from everyone's ear.",
    "$n produces a coin from your ear.",
    "You step behind your shadow.", 
    "$n steps behind $s shadow.",
    "Your eyes dance with greed.", 
    "$n's eyes dance with greed.",
    "You deftly steal everyone's weapon.",
    "$n deftly steals your weapon.",
    "The Grey Mouser buys you a beer.",
    "The Grey Mouser buys $n a beer.",
    "Everyone's pocket explodes with your fireworks.",
    "Your pocket explodes with $n's fireworks.",
    "Everyone discovers your dagger a centimeter from their eye.",
    "You discover $n's dagger a centimeter from your eye.",
    "Where did you go?", 
    "Where did $n go?",
    "Click.",
    "Click.",
    "Click.",
        "Click.",
    }},
  {
	  
   { //Reaver
    "You hit your head, and your eyes roll.",
    "$n hits $s head, and $s eyes roll.",
    "You show your bulging muscles.",
    "$n shows $s bulging muscles.",
    "You crack nuts between your fingers.",
    "$n cracks nuts between $s fingers.",
    "You grizzle your teeth and look mean.",
    "$n grizzles $s teeth and looks mean.",
    "Crunch, crunch -- you munch a bottle.",
    "Crunch, crunch -- $n munches a bottle.",
    "... 98, 99, 100 ... you do pushups.",
    "... 98, 99, 100 ... $n does pushups.",
    "Arnold Schwarzenegger admires your physique.",
    "Arnold Schwarzenegger admires $n's physique.",
    "Watch your feet, you are juggling granite boulders.",
    "Watch your feet, $n is juggling granite boulders.",
    "Oomph!  You squeeze water out of a granite boulder.",
    "Oomph!  $n squeezes water out of a granite boulder.",
    "You pick your teeth with $p.",
    "$n picks $s teeth with $p.",
    "Everyone is swept off their foot by your hug.",
    "You are swept off your feet by $n's hug.",
    "Your karate chop splits a tree.",
    "$n's karate chop splits a tree.",
    "A strap of your armor breaks over your mighty thews.",
    "A strap of $n's armor breaks over $s mighty thews.",
    "A boulder cracks at your frown.",
    "A boulder cracks at $n's frown.",
    "Mercenaries arrive to do your bidding.",
    "Mercenaries arrive to do $n's bidding.",
    "Four matched Percherons bring in your chariot.",
    "Four matched Percherons bring in $n's chariot.",
    "Atlas asks you to relieve him.",
    "Atlas asks $n to relieve him.", 
"Click.",
    "Click.",
	    
   }},
  {
   { //Monk
     "You hit your head, and your eyes roll.",
     "$n hits $s head, and $s eyes roll.",
     "You show your bulging muscles.",
     "$n shows $s bulging muscles.",
     "You crack nuts between your fingers.",
     "$n cracks nuts between $s fingers.",
     "You grizzle your teeth and look mean.",
     "$n grizzles $s teeth and looks mean.",
     "Crunch, crunch -- you munch a bottle.",
     "Crunch, crunch -- $n munches a bottle.",
     "... 98, 99, 100 ... you do pushups.",
     "... 98, 99, 100 ... $n does pushups.",
     "Arnold Schwarzenegger admires your physique.",
     "Arnold Schwarzenegger admires $n's physique.",
     "Watch your feet, you are juggling granite boulders.",
     "Watch your feet, $n is juggling granite boulders.",
     "Oomph!  You squeeze water out of a granite boulder.",
     "Oomph!  $n squeezes water out of a granite boulder.",
     "You pick your teeth with a spear.",
     "$n picks $s teeth with a spear.",
     "Everyone is swept off their foot by your hug.",
     "You are swept off your feet by $n's hug.",
     "Your karate chop splits a tree.",
     "$n's karate chop splits a tree.",
     "A strap of your armor breaks over your mighty thews.",
     "A strap of $n's armor breaks over $s mighty thews.",
     "A boulder cracks at your frown.",
     "A boulder cracks at $n's frown.",
     "Mercenaries arrive to do your bidding.",
     "Mercenaries arrive to do $n's bidding.",
     "Four matched Percherons bring in your chariot.",
     "Four matched Percherons bring in $n's chariot.",
     "Atlas asks you to relieve him.",
     "Atlas asks $n to relieve him.",
   "Click.",
       "Click.",
   }},
  {	
   { //Necromancer
    "You sizzle with energy.",
    "$n sizzles with energy.",
    "You turn into a butterfly, then return to your normal shape.",
    "$n turns into a butterfly, then returns to $s normal shape.",
    "Blue sparks fly from your fingers.",
    "Blue sparks fly from $n's fingers.",
    "Little red lights dance in your eyes.",
    "Little red lights dance in $n's eyes.",
    "A slimy green monster appears before you and bows.",
    "A slimy green monster appears before $n and bows.",
    "You turn everybody into a little pink elephant.",
    "You are turned into a little pink elephant by $n.",
    "A small ball of light dances on your fingertips.",
    "A small ball of light dances on $n's fingertips.",
    "Smoke and fumes leak from your nostrils.",
    "Smoke and fumes leak from $n's nostrils.",
    "The light flickers as you rap in magical languages.",
    "The light flickers as $n raps in magical languages.",
    "Your head disappears.",
    "$n's head disappears.",
    "A fire elemental singes your hair.",
    "A fire elemental singes $n's hair.",
    "The sky changes color to match your eyes.",
    "The sky changes color to match $n's eyes.",
    "The stones dance to your command.",
    "The stones dance to $n's command.",
    "The heavens and grass change colour as you smile.",
    "The heavens and grass change colour as $n smiles.",
    "Everyone's clothes are transparent, and you are laughing.",
    "Your clothes are transparent, and $n is laughing.",
    "A black hole swallows you.",
    "A black hole swallows $n.",
    "The world shimmers in time with your whistling.",
    "The world shimmers in time with $n's whistling.",
    "Click.",
        "Click.",
    }},
  {
   {  //Swarm
    "You sizzle with energy.",
    "$n sizzles with energy.",
    "You turn into a butterfly, then return to your normal shape.",
    "$n turns into a butterfly, then returns to $s normal shape.",
    "Blue sparks fly from your fingers.",
    "Blue sparks fly from $n's fingers.",
    "Little red lights dance in your eyes.",
    "Little red lights dance in $n's eyes.",
    "A slimy green monster appears before you and bows.",
    "A slimy green monster appears before $n and bows.",
    "You turn everybody into a little pink elephant.",
    "You are turned into a little pink elephant by $n.",
    "A small ball of light dances on your fingertips.",
    "A small ball of light dances on $n's fingertips.",
    "Smoke and fumes leak from your nostrils.",
    "Smoke and fumes leak from $n's nostrils.",
    "The light flickers as you rap in magical languages.",
    "The light flickers as $n raps in magical languages.",
    "Your head disappears.",
    "$n's head disappears.",
    "A fire elemental singes your hair.",
    "A fire elemental singes $n's hair.",
    "The sky changes color to match your eyes.",
    "The sky changes color to match $n's eyes.",
    "The stones dance to your command.",
    "The stones dance to $n's command.",
    "The heavens and grass change colour as you smile.",
    "The heavens and grass change colour as $n smiles.",
    "Everyone's clothes are transparent, and you are laughing.",
    "Your clothes are transparent, and $n is laughing.",
    "A black hole swallows you.",
    "A black hole swallows $n.",
    "The world shimmers in time with your whistling.",
    "The world shimmers in time with $n's whistling.",
    "Click.",
        "Click.",
}},
  { 	
   { //Golem
    "You sizzle with energy.",
    "$n sizzles with energy.",
    "You turn into a butterfly, then return to your normal shape.",
    "$n turns into a butterfly, then returns to $s normal shape.",
    "Blue sparks fly from your fingers.",
    "Blue sparks fly from $n's fingers.",
    "Little red lights dance in your eyes.",
    "Little red lights dance in $n's eyes.",
    "A slimy green monster appears before you and bows.",
    "A slimy green monster appears before $n and bows.",
    "You turn everybody into a little pink elephant.",
    "You are turned into a little pink elephant by $n.",
    "A small ball of light dances on your fingertips.",
    "A small ball of light dances on $n's fingertips.",
    "Smoke and fumes leak from your nostrils.",
    "Smoke and fumes leak from $n's nostrils.",
    "The light flickers as you rap in magical languages.",
    "The light flickers as $n raps in magical languages.",
    "Your head disappears.",
    "$n's head disappears.",
    "A fire elemental singes your hair.",
    "A fire elemental singes $n's hair.",
    "The sky changes color to match your eyes.",
    "The sky changes color to match $n's eyes.",
    "The stones dance to your command.",
    "The stones dance to $n's command.",
    "The heavens and grass change colour as you smile.",
    "The heavens and grass change colour as $n smiles.",
    "Everyone's clothes are transparent, and you are laughing.",
    "Your clothes are transparent, and $n is laughing.",
    "A black hole swallows you.",
    "A black hole swallows $n.",
    "The world shimmers in time with your whistling.",
    "The world shimmers in time with $n's whistling.",
    "Click.",
    "Click.",
}},
  {
   { //Chaos Jester
    "You sizzle with energy.",
    "$n sizzles with energy.",
    "You turn into a butterfly, then return to your normal shape.",
    "$n turns into a butterfly, then returns to $s normal shape.",
    "Blue sparks fly from your fingers.",
    "Blue sparks fly from $n's fingers.",
    "Little red lights dance in your eyes.",
    "Little red lights dance in $n's eyes.",
    "A slimy green monster appears before you and bows.",
    "A slimy green monster appears before $n and bows.",
    "You turn everybody into a little pink elephant.",
    "You are turned into a little pink elephant by $n.",
    "A small ball of light dances on your fingertips.",
    "A small ball of light dances on $n's fingertips.",
    "Smoke and fumes leak from your nostrils.",
    "Smoke and fumes leak from $n's nostrils.",
    "The light flickers as you rap in magical languages.",
    "The light flickers as $n raps in magical languages.",
    "Your head disappears.",
    "$n's head disappears.",
    "A fire elemental singes your hair.",
    "A fire elemental singes $n's hair.",
    "The sky changes color to match your eyes.",
    "The sky changes color to match $n's eyes.",
    "The stones dance to your command.",
    "The stones dance to $n's command.",
    "The heavens and grass change colour as you smile.",
    "The heavens and grass change colour as $n smiles.",
    "Everyone's clothes are transparent, and you are laughing.",
    "Your clothes are transparent, and $n is laughing.",
    "A black hole swallows you.",
    "A black hole swallows $n.",
    "The world shimmers in time with your whistling.",
    "The world shimmers in time with $n's whistling.",
    "Click.",
    "Click.",
}},
  {	
   {  //Adventurer
    "You sizzle with energy.",
    "$n sizzles with energy.",
    "You turn into a butterfly, then return to your normal shape.",
    "$n turns into a butterfly, then returns to $s normal shape.",
    "Blue sparks fly from your fingers.",
    "Blue sparks fly from $n's fingers.",
    "Little red lights dance in your eyes.",
    "Little red lights dance in $n's eyes.",
    "A slimy green monster appears before you and bows.",
    "A slimy green monster appears before $n and bows.",
    "You turn everybody into a little pink elephant.",
    "You are turned into a little pink elephant by $n.",
    "A small ball of light dances on your fingertips.",
    "A small ball of light dances on $n's fingertips.",
    "Smoke and fumes leak from your nostrils.",
    "Smoke and fumes leak from $n's nostrils.",
    "The light flickers as you rap in magical languages.",
    "The light flickers as $n raps in magical languages.",
    "Your head disappears.",
    "$n's head disappears.",
    "A fire elemental singes your hair.",
    "A fire elemental singes $n's hair.",
    "The sky changes color to match your eyes.",
    "The sky changes color to match $n's eyes.",
    "The stones dance to your command.",
    "The stones dance to $n's command.",
    "The heavens and grass change colour as you smile.",
    "The heavens and grass change colour as $n smiles.",
    "Everyone's clothes are transparent, and you are laughing.",
    "Your clothes are transparent, and $n is laughing.",
    "A black hole swallows you.",
    "A black hole swallows $n.",
    "The world shimmers in time with your whistling.",
    "The world shimmers in time with $n's whistling.",
    "Click.",
    "Click.",
    }}
};
void do_pose (CHAR_DATA * ch, char *argument)
{

//  int level;
  int Class=0;
  int pose;

  if (IS_NPC (ch))
     return;
//  send_to_char ("There are no posers on this mud!\r\n", ch);

//send_to_char("There are no posers on this mud!\r\n",ch);
  /*send_to_char ("You strike a pose showing off your best assets!\r\n", ch);
  act ("$n strikes a pose showing off $s best assets!\r\n", ch, NULL,
       NULL, TO_ROOM);*/

  //level = UMIN (ch->level, sizeof (pose_table) / sizeof (pose_table[0]) - 1);
  pose = number_range (0, 17);
  if (ch->level < 31)
  {
    Class = ch->Class;
  }
  else if (ch->level < 61)
  {
    if (number_percent() < 50)
	    Class = ch->Class;
    else Class = ch->Class2;
  }
  else 
  {
    if (number_percent() >= 66)
	    Class = ch->Class;
    else if (number_percent() < 50)
	    Class = ch->Class2;
    else Class = ch->Class3;
  }

		    
  if (Class == PC_CLASS_REAVER && !IS_NPC(ch) && ch->sword)
  {
    act (pose_table[Class].message[pose*2], ch, ch->sword, NULL, TO_CHAR);
    act (pose_table[Class].message[pose*2+1], ch, ch->sword, NULL, TO_ROOM);
  }
  else 
  {
    if (Class == PC_CLASS_REAVER)
      Class = PC_CLASS_WARRIOR;
    act (pose_table[Class].message[pose*2], ch, NULL, NULL, TO_CHAR);
    act (pose_table[Class].message[pose*2+1], ch, NULL, NULL, TO_ROOM);
  }


  return;
}

void do_bug (CHAR_DATA * ch, char *argument)
{
  append_file (ch, BUG_FILE, argument);
  send_to_char ("Bug logged.\n\r", ch);
  return;
}

void do_typo (CHAR_DATA * ch, char *argument)
{
  append_file (ch, TYPO_FILE, argument);
  send_to_char ("Typo logged.\n\r", ch);
  return;
}

void do_rent (CHAR_DATA * ch, char *argument)
{
  send_to_char ("There is no rent here.  Just save and quit.\n\r", ch);
  return;
}

void do_qui (CHAR_DATA * ch, char *argument)
{
  send_to_char ("If you want to QUIT, you have to spell it out.\n\r", ch);
  return;
}

void do_quit (CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d, *d_next;
  int id;
  CHAR_DATA *vch, *vch_next;	//,*ch2,*ch2_next;
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj, *obj_next;
  if (IS_NPC (ch))
    return;
  if (is_dueling (ch))
    {
      send_to_char ("No way! You are in the middle of a duel!\n\r", ch);
      return;
    }
  if (ch->dueler)
    {
      send_to_char ("No way! You are in the middle of a super duel!\n\r", ch);
      return;
    }
  if (ch->position == POS_FIGHTING || ch->fighting != NULL)
    {
      send_to_char ("No way! You are fighting.\n\r", ch);
      return;
    }
  if (current_time - ch->pcdata->last_pkdeath < (60*MULTIKILL_WAIT_TIME))
  {
    send_to_char("You cannot quit while protected.  Type \"killable\" first.\n\r",ch);
    return;
  }
  if (!ch->in_room)
    return;
  if (IS_SET (ch->in_room->room_flags, ROOM_ELEVATOR) ||
		  IS_SET (ch->in_room->area->area_flags,AREA_NO_QUIT) ||
      (ch->in_room->vnum >= ROOM_VNUM_HAVEN_START
       && ch->in_room->vnum <= ROOM_VNUM_HAVEN_END))
    {
      send_to_char ("You will have to leave this room first.\n\r", ch);
      return;
    }
  if (!IS_NPC (ch))
    if ((time (NULL) - ch->pcdata->last_fight < 120) && (!IS_IMMORTAL(ch)))
      {
	send_to_char ("Nice try.  You can't escape so easily.\n\r", ch);
	return;
      }
/*  if (!IS_NPC (ch))
    if ((time (NULL) - ch->pcdata->last_loot < 600) && (!IS_IMMORTAL(ch)))
      {
        send_to_char ("Nice try.  You have not yet shown your face as a Looter long enough.\n\r", ch);
        return;
      }*/
  
  if (ch->position < POS_STUNNED)
    {
      send_to_char ("You're not DEAD yet.\n\r", ch);
      return;
    }
  if (auction->item != NULL
      && ((ch == auction->buyer) || (ch == auction->seller)))
    {
      send_to_char
	("Wait till you have sold/bought the item on auction.\n\r", ch);
      return;
    }
     if (ch->pcdata->familiar != NULL)
    {
      if (ch->Class != PC_CLASS_CHAOS_JESTER)
        act("`o$N returns to the underworld.``",ch,NULL,ch->pcdata->familiar,TO_CHAR);
      else act("`o$N goes POOF!``",ch,NULL,ch->pcdata->familiar,TO_CHAR);
      familiar_poof(ch);
    }
 /* Moved to extract_char 
    for (list = ch->trap_list;list != NULL;list = list->next_trap)
	count++;
    for (;count >0;count--)
    {
	if (ch->trap_list != NULL)
	{
		obj_from_room(ch->trap_list );
		extract_obj(ch->trap_list );
	}
    }
    */
  //6/29/03 Adeon - objects flagged as NOSAVE drop to the ground on quit
  for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      if (IS_OBJ_STAT (obj, ITEM_NOSAVE))
	{
	  if (obj->wear_loc != WEAR_NONE)
	    unequip_char (ch, obj);
	  obj_from_char (obj);
	  obj_to_room (obj, ch->in_room);
	  act ("$n loses possesion of $p.", ch, obj, NULL, TO_ROOM);
	  act ("You lose possesion of $p.", ch, obj, NULL, TO_CHAR);
	  if (IS_OBJ_STAT (obj, ITEM_MELT_DROP))
	  {
	    act ("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
	    extract_obj (obj);
	  }
	   
	}
    }

  if (ch->pcdata->corpse != NULL)
    obj_to_room(ch->pcdata->corpse,get_room_index(1));

  //5-12-03 Iblis - Made it so like it was at one time.  You quit, and you
  //become uncharmed AND lose the affects
  if (IS_AFFECTED (ch, AFF_CHARM))
    {
      REMOVE_BIT (ch->affected_by, AFF_CHARM);
      affect_strip (ch, gsn_charm_person);
      affect_strip (ch, skill_lookup ("The Pipers Melody"));
    }
  if (is_affected(ch,gsn_soul_link))
  {
      affect_strip (ch,gsn_soul_link);
      if (ch->pcdata->soul_link != NULL)
      {
        affect_strip (ch->pcdata->soul_link,gsn_soul_link);
        ch->pcdata->soul_link->pcdata->soul_link = NULL;
      }
      ch->pcdata->soul_link = NULL;
  }
  for (vch = char_list;vch != NULL;vch = vch->next)
  {
    if (vch->contaminator && vch->contaminator == ch)
	    vch->contaminator = NULL;
  }
  if (ch->race == PC_RACE_KALIAN)
  {
    affect_strip (ch, gsn_jurgnation);
    affect_strip (ch, gsn_jalknation);
  }
  if (ch->pcdata->quest_obj != NULL)
    extract_obj(ch->pcdata->quest_obj);
  if (ch->pcdata->quest_mob != NULL && ch->pcdata->quest_mob != ch)
    extract_char(ch->pcdata->quest_mob,TRUE);
   
  send_to_char
    ("The music in my heart I bore, long after it was heard no more.\n\r",
     ch);


  //IBLIS 6/20/03 - Battle Royale fixes
  if (battle_royale && br_leader == ch)
    {
      br_leader = NULL;
      for (d = descriptor_list; d != NULL; d = d_next)
	{
	  d_next = d->next;
	  if (d->character == NULL || d->character == ch
	      || IS_NPC (d->character) || d->character->pcdata == NULL
	      || ch->pcdata == NULL)
	    continue;
	  if (br_leader == NULL && d->character->pcdata->br_points > 0)
	    br_leader = d->character;

	  else if (br_leader != NULL
		   && d->character->pcdata->br_points >
		   br_leader->pcdata->br_points)
	    br_leader = d->character;
	}
      if (br_leader != NULL)
	{
	  char ttbuf[MAX_STRING_LENGTH];
	  send_to_char
	    ("`gDue to the previous Battle Royal leader leaving, you are the new Battle Royal Leader!``\r\n",
	     ch);
	  sprintf (ttbuf,
		   "`gAnd we have a new Battle Royal Leader -> %s``\r\n",
		   br_leader->name);
	  do_echo (br_leader, ttbuf);
	}
    }
  if (last_br_kill == ch)
    last_br_kill = NULL;

//bug("Aggressor list start",0);
  //Iblis 6/28/03 - Aggressor list fixed
  for (d = descriptor_list; d != NULL; d = d_next)
    {

      d_next = d->next;
      if (d->character == NULL || d->character->pcdata == NULL
	  || ch->pcdata == NULL)
	{			
	  continue;
	}
      if (ch == d->character || IS_NPC (d->character))
	{			
	  continue;
	}
      remove_aggressor (ch, d->character);
      remove_aggressor (d->character, ch);
    }

  if (ch->pcdata->flaming)
    {
      OBJ_DATA *obj;
      ch->pcdata->flaming = 0;
      if (!((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL
                  && obj->item_type == ITEM_LIGHT
	    && obj->value[2] != 0 && ch->in_room != NULL))
	ch->in_room->light--;

      if (is_affected(ch,gsn_fireshield))
	affect_strip(ch,gsn_fireshield);
    }


  //Iblis 6/30/03 - Falcon list fixed
  for (d = descriptor_list; d != NULL; d = d_next)
    {
      d_next = d->next;
      if (d->character == NULL || d->character->pcdata == NULL
	  || ch->pcdata == NULL)
	continue;
      if (IS_NPC (d->character))
	continue;
      if (d->character->pcdata->falcon_wait != 0)
	if (d->character->pcdata->falcon_recipient == ch)
	  d->character->pcdata->falcon_recipient = NULL;
    }

  if (!is_linkloading)
  {
   sprintf (buf, "%s has left the game.\n\r", ch->name);
   for (vch = char_list; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next;
      if (vch->in_room == NULL)
	continue;
      if (vch->in_room == ch->in_room && can_see (vch, ch) && ch != vch)
	{
	  send_to_char (buf, vch);
	  continue;
	}
    }
   sprintf (log_buf, "%s has quit.", ch->name);
   log_string (log_buf);
   wiznet ("$N rejoins the real world.", ch, NULL, WIZ_LOGINS, 0,
	  get_trust (ch));
  }
	

  /*
   * After extract_char the ch is no longer valid!
   */
  save_char_obj (ch);
  id = ch->id;
  d = ch->desc;
  ch_is_quitting = TRUE;
  if (!is_linkloading)
  {
    nuke_pets (ch);
    nuke_boarded_mounts (ch);
  }
  for (vch = char_list; vch != NULL; vch = vch->next)
    {
      if (vch->master == ch)
	{
	  stop_follower (vch);
	  if (IS_AFFECTED (vch, AFF_CHARM))
	    {
	      REMOVE_BIT (vch->affected_by, AFF_CHARM);
	      affect_strip (vch, gsn_charm_person);
	      affect_strip (vch, skill_lookup ("The Pipers Melody"));
	    }
	}
      if (vch->leader == ch)
	vch->leader = vch;
    }
  if (IS_AFFECTED (ch, AFF_CHARM))
    {
      REMOVE_BIT (ch->affected_by, AFF_CHARM);
      affect_strip (ch, gsn_charm_person);
      affect_strip (ch, skill_lookup ("The Pipers Melody"));
    }
  extract_char (ch, TRUE);
  ch_is_quitting = FALSE;
  if (d != NULL)
    close_socket (d);
  for (d = descriptor_list; d != NULL; d = d_next)
    {
      CHAR_DATA *tch;
      d_next = d->next;
      tch = d->original ? d->original : d->character;
      if (tch && tch->id == id)
	{
	  extract_char (tch, TRUE);
	  close_socket (d);
	}
    }
  return;
}

void do_hard_quit (CHAR_DATA * ch, char *argument, bool avatar)
{
  DESCRIPTOR_DATA *d, *d_next;
  CHAR_DATA *vch, *vch_next;
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA* obj, *obj_next;
  int id;
  if (IS_NPC (ch))
    return;
  if (is_dueling (ch))
    {
      if (!avatar)
	{
	  send_to_char ("No way! You are in the middle of a duel!\n\r", ch);
	  return;
	}

      else
	duel_ends (ch);
    }

  if (ch->dueler)
  {
    if (!avatar)
    {
      send_to_char("No way! You are in the middle of a super duel!\n\r", ch);
      return;
    }
    else
      superduel_ends(ch,ch);
  }
  if (ch->position == POS_FIGHTING || ch->fighting != NULL)
    stop_fighting (ch, TRUE);
  if (auction->item != NULL
      && ((ch == auction->buyer) || (ch == auction->seller)))
    {
      if (!avatar)
	{
	  send_to_char
	    ("Wait till you have sold/bought the item on auction.\n\r", ch);
	  return;
	}

      else
	{

	  //CHEAAAAAAP FIX
	  ch->level = 92;
	  do_auction (ch, "stop");
	}
    }

  if (ch->pcdata->corpse != NULL)
     obj_to_room(ch->pcdata->corpse,get_room_index(1));
  //IBLIS 6/20/03 - Battle Royale fixes
  if (battle_royale && br_leader == ch)
    {
      DESCRIPTOR_DATA *d;
      br_leader = NULL;
      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  if (d->character == NULL || d->character == ch
	      || IS_NPC (d->character) || d->character->pcdata == NULL
	      || ch->pcdata == NULL)
	    continue;
	  if (br_leader == NULL && d->character->pcdata->br_points > 0)
	    br_leader = d->character;

	  else if (br_leader != NULL
		   && d->character->pcdata->br_points >
		   br_leader->pcdata->br_points)
	    br_leader = d->character;
	}
      if (br_leader != NULL)
	{
	  char ttbuf[MAX_STRING_LENGTH];
	  send_to_char
	    ("`gDue to the previous Battle Royal leader leaving, you are the new Battle Royal Leader!``\r\n",
	     ch);
	  sprintf (ttbuf,
		   "`gAnd we have a new Battle Royal Leader -> %s``\r\n",
		   br_leader->name);
	  do_echo (br_leader, ttbuf);
	}
    }
  if (last_br_kill == ch)
    last_br_kill = NULL;

  if (ch->pcdata->flaming)
    {
      OBJ_DATA *obj;
      ch->pcdata->flaming = 0;
      if (!((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL
                  && obj->item_type == ITEM_LIGHT
	    && obj->value[2] != 0 && ch->in_room != NULL))
	ch->in_room->light--;

      if (is_affected(ch,gsn_fireshield))
	affect_strip(ch,gsn_fireshield);
    }


  //Iblis 6/28/03 - Aggressor list fixed
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->character == NULL || ch == d->character
	  || IS_NPC (d->character) || d->character->pcdata == NULL
	  || ch->pcdata == NULL)
	continue;
      remove_aggressor (ch, d->character);
      remove_aggressor (d->character, ch);
    }

  for (vch = char_list;vch != NULL;vch = vch->next)
    {
      if (vch->contaminator && vch->contaminator == ch)
          vch->contaminator = NULL;
    }
    
  
   //6/29/03 Adeon - objects flagged as NOSAVE drop to the ground on quit
   for (obj = ch->carrying; obj != NULL; obj = obj_next)
     {
       obj_next = obj->next_content;
       if (IS_OBJ_STAT (obj, ITEM_NOSAVE))
       {
         if (obj->wear_loc != WEAR_NONE)
             unequip_char (ch, obj);
         obj_from_char (obj);
         obj_to_room (obj, ch->in_room);
         act ("$n loses possesion of $p.", ch, obj, NULL, TO_ROOM);
         act ("You lose possesion of $p.", ch, obj, NULL, TO_CHAR);
	 if (IS_OBJ_STAT (obj, ITEM_MELT_DROP))
	   {
	     act ("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
	     extract_obj (obj);
	   }
	  
       }
      }
 
   if (ch->pcdata->quest_obj != NULL)
       extract_obj(ch->pcdata->quest_obj);
   if (ch->pcdata->quest_mob != NULL && ch->pcdata->quest_mob != ch)
       extract_char(ch->pcdata->quest_mob,TRUE);
   
  
  //Iblis 6/30/03 - Falcon list fixed
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->character == NULL || IS_NPC (d->character)
	  || d->character->pcdata == NULL || ch->pcdata == NULL)
	continue;
      if (d->character->pcdata->falcon_wait != 0)
	if (d->character->pcdata->falcon_recipient == ch)
	  d->character->pcdata->falcon_recipient = NULL;
    }
  send_to_char
    ("The music in my heart I bore, long after it was heard no more.\n\r",
     ch);
  sprintf (buf, "%s has left the game.\n\r", ch->name);
  for (vch = char_list; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next;
      if (vch->in_room == NULL)
	continue;
      if (vch->in_room == ch->in_room && can_see (ch, vch) && ch != vch)
	{
	  send_to_char (buf, vch);
	  continue;
	}
    }
  sprintf (log_buf, "%s has quit.", ch->name);
  log_string (log_buf);
  wiznet ("$N rejoins the real world.", ch, NULL, WIZ_LOGINS, 0,
	  get_trust (ch));
  id = ch->id;
  d = ch->desc;
  for (vch = char_list; vch != NULL; vch = vch->next)
    {
      if (vch->master == ch)
	{
	  stop_follower (vch);
	  if (IS_AFFECTED (vch, AFF_CHARM))
	    {
	      REMOVE_BIT (vch->affected_by, AFF_CHARM);
	      affect_strip (vch, gsn_charm_person);
	      affect_strip (vch, skill_lookup ("The Pipers Melody"));
	    }
	}
      if (vch->leader == ch)
	vch->leader = vch;
    }
  if (IS_AFFECTED (ch, AFF_CHARM))
    {
      REMOVE_BIT (ch->affected_by, AFF_CHARM);
      affect_strip (ch, gsn_charm_person);
      affect_strip (ch, skill_lookup ("The Pipers Melody"));
    }

  if (is_affected(ch,gsn_soul_link))
    {
      affect_strip (ch,gsn_soul_link);
      if (ch->pcdata->soul_link != NULL)
      {
        affect_strip (ch->pcdata->soul_link,gsn_soul_link);
        ch->pcdata->soul_link->pcdata->soul_link = NULL;
      }
      ch->pcdata->soul_link = NULL;
    }
  

  if (ch->race == PC_RACE_KALIAN)
   {
      affect_strip (ch, gsn_jurgnation);
      affect_strip (ch, gsn_jalknation);
   }
  
  extract_char (ch, TRUE);
  if (d != NULL)
    close_socket (d);

  /* toast evil cheating bastards */
  for (d = descriptor_list; d != NULL; d = d_next)
    {
      CHAR_DATA *tch;
      d_next = d->next;
      tch = d->original ? d->original : d->character;
      if (tch && tch->id == id)
	{
	  extract_char (tch, TRUE);
	  close_socket (d);
	}
    }
  return;
}

void do_save (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char arg4[MAX_INPUT_LENGTH];
  int pos;
  if (IS_NPC (ch))
    return;
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);
  argument = one_argument (argument, arg4);
  if (is_dueling (ch))
    {
      send_to_char ("You may not save while dueling!\n\r", ch);
      return;
    }
  pos = ch->position;
  if (!str_cmp (arg1, "nowhere"))
    {
      nowhere = TRUE;
      sprintf (buf, "Saved %s.\n\r", ch->name);
      send_to_char (buf, ch);
    }

  else
    {
      nowhere = FALSE;
      if (!IS_AFFECTED (ch, AFF_BLIND) && ch->in_room)
	sprintf (buf, "Saved %s at %s.\n\r", ch->name, ch->in_room->name);

      else
	sprintf (buf, "Saved %s at somewhere.\n\r", ch->name);
      send_to_char (buf, ch);
    }
  save_char_obj (ch);
  ch->position = pos;
  return;
}

void do_follow (CHAR_DATA * ch, char *argument)
{

/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  one_argument (argument, arg);
  if ((battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner)))|| ch->dueler)
    {
      send_to_char
	("You don't need to follow anyone.  You should fight alone.\r\n", ch);
      return;
    }
  if (arg[0] == '\0')
    {
      send_to_char ("Follow whom?\n\r", ch);
      return;
    }
  if (is_affected (ch, gsn_mirror))
    {
      act ("You can't follow someone while you are mirrored.", ch, NULL,
	   NULL, TO_CHAR);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (is_affected (victim, gsn_mirror))
    {
      act ("You can't follow someone who is mirrored.", ch, NULL, NULL,
	   TO_CHAR);
      return;
    }
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL)
    {
      act ("But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR);
      return;
    }
  if (victim == ch)
    {
      if (ch->master == NULL)
	{
	  send_to_char ("You already follow yourself.\n\r", ch);
	  return;
	}
      stop_follower (ch);
      return;
    }
  if (!IS_NPC (victim) && IS_SET (victim->act, PLR_NOFOLLOW)
      && !IS_IMMORTAL (ch))
    {
      act ("$N doesn't seem to want any followers.\n\r", ch, NULL,
	   victim, TO_CHAR);
      return;
    }
  REMOVE_BIT (ch->act, PLR_NOFOLLOW);
  if (ch->master != NULL)
    stop_follower (ch);
  add_follower (ch, victim);
  return;
}

void follow_gr (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (victim == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL)
    {
      act ("But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR);
      return;
    }
  if (victim == ch)
    {
      if (ch->master == NULL)
	{
	  send_to_char ("You already follow yourself.\n\r", ch);
	  return;
	}
      stop_follower (ch);
      return;
    }
  if (!IS_NPC (victim) && IS_SET (victim->act, PLR_NOFOLLOW)
      && !IS_IMMORTAL (ch))
    {
      act ("$N doesn't seem to want any followers.\n\r", ch, NULL,
	   victim, TO_CHAR);
      return;
    }
  REMOVE_BIT (ch->act, PLR_NOFOLLOW);
  if (ch->master != NULL)
    stop_follower (ch);
  add_follower (ch, victim);
  return;
}

void do_defect (CHAR_DATA * ch, char *argument)
{
  if (ch->master == NULL)
    {
      send_to_char ("You aren't a follower of a group.\n\r", ch);
      return;
    }
  if (!IS_NPC (ch))
    ch->pcdata->group_exp = 0;
  defect_follower (ch);
  return;
}

void add_follower (CHAR_DATA * ch, CHAR_DATA * master)
{
  //Can have this with Familiars or vivify object and it's not a bug
/*  if (ch->master != NULL)
    {
      bug ("Add_follower: non-null master.", 0);
      return;
    }*/
  ch->master = master;
  ch->leader = NULL;
  if (can_see (master, ch))
    act ("$n now follows you.", ch, NULL, master, TO_VICT);
  act ("You now follow $N.", ch, NULL, master, TO_CHAR);
  return;
}

void defect_follower (CHAR_DATA * ch)
{
  if (ch->master == NULL)
    {
      bug ("defect_follower: null master.", 0);
      return;
    }
  if (IS_AFFECTED (ch, AFF_CHARM))
    {
      send_to_char ("Sorry, you can't do that now.\n\r", ch);
      return;
    }
  if (can_see (ch->master, ch) && ch->in_room != NULL)
    {
      act ("$n has defected from your group.", ch, NULL, ch->master, TO_VICT);
      act ("You defect from $N's group.", ch, NULL, ch->master, TO_CHAR);
    }
  if (ch->master->pet == ch)
    ch->master->pet = NULL;
  ch->master = NULL;
  ch->leader = NULL;
  return;
}

void stop_follower (CHAR_DATA * ch)
{
  bool grouped = 0;
  CHAR_DATA *gch;
  if (ch->master == NULL)
    {
    //  bug ("Stop_follower: null master.", 0);
      return;
    }
  //IBLIS 5/18/03 - need for new group xp thing
  if (!IS_NPC (ch))
    ch->pcdata->group_exp = 0;
  grouped = 0;
  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (is_same_group (gch, ch->master))
	grouped = 1;
    }
  if (!grouped && !IS_NPC (ch->master) && !IS_NPC (ch))
    ch->master->pcdata->group_exp = 0;
  if (IS_AFFECTED (ch, AFF_CHARM))
    {
      REMOVE_BIT (ch->affected_by, AFF_CHARM);
      affect_strip (ch, gsn_charm_person);
      affect_strip (ch, skill_lookup ("The Pipers Melody"));
    }
  if (can_see (ch->master, ch) && ch->in_room != NULL && !ch_is_quitting)
    {
      act ("$n stops following you.", ch, NULL, ch->master, TO_VICT);
      act ("You stop following $N.", ch, NULL, ch->master, TO_CHAR);
    }
  if (ch->master != NULL)
    {
      if (ch->master->pet == ch)
	ch->master->pet = NULL;
    }
//  if (ch->master->pet == ch)
//    ch->master->pet = NULL;
  ch->master = NULL;
  ch->leader = NULL;
  return;
}


/* nukes charmed monsters and pets */
void nuke_pets (CHAR_DATA * ch)
{
  CHAR_DATA *pet;
  OBJ_DATA *obj, *obj_next;
  if ((pet = ch->pet) != NULL)
    {
      stop_follower (pet);
      for (obj = pet->carrying; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  unequip_char (pet, obj);
	}
      if (pet->in_room != NULL)
	act ("$N slowly fades away.", ch, NULL, pet, TO_NOTVICT);
      extract_char (pet, TRUE);
    }
  ch->pet = NULL;
  return;
}

// Iblis 9/13/04 - Added because mounts load new on logon, but never went away, filling up the stable
// with an ungodly number of useless fucking mounts
void nuke_boarded_mounts (CHAR_DATA *ch)
{
  CHAR_DATA *mount, *mount_next;
  for (mount = ch->pcdata->boarded; mount != NULL; mount = mount_next)
  {
    mount_next = mount->next_in_board;
    extract_char (mount,TRUE);
  }
  ch->pcdata->boarded = NULL;
  return;
}

void die_follower (CHAR_DATA * ch)
{
  CHAR_DATA *fch;
  if (ch->master != NULL)
    {
      if (ch->master->pet == ch)
	ch->master->pet = NULL;
      stop_follower (ch);
    }
  ch->leader = NULL;
  for (fch = char_list; fch != NULL; fch = fch->next)
    {
      if (fch->master == ch && !IS_AFFECTED (fch, AFF_CHARM))
	stop_follower (fch);
      if (fch->leader == ch)
	fch->leader = fch;
    }
  return;
}

int disband_follower (CHAR_DATA * ch)
{
  CHAR_DATA *fch;
  int found = FALSE;
  if (ch->master != NULL)
    {
      if (ch->master->pet == ch)
	ch->master->pet = NULL;
      stop_follower (ch);
    }
  ch->leader = NULL;

  //IBLIS 5/18/03 - New group xp thing
  if (!IS_NPC (ch))
    ch->pcdata->group_exp = 0;
  for (fch = char_list; fch != NULL; fch = fch->next)
    {
      if (fch->master == ch)
	{
	  act ("$n has disbanded the group.", ch, NULL, fch, TO_VICT);
	  stop_follower (fch);
	  found = TRUE;

	  //IBLIS 5/18/03 - New group xp thing
	  if (!IS_NPC (fch))
	    fch->pcdata->group_exp = 0;
	  if (IS_AFFECTED (fch, AFF_CHARM))
	    {
	      REMOVE_BIT (fch->affected_by, AFF_CHARM);
	      affect_strip (fch, gsn_charm_person);
	      affect_strip (ch, skill_lookup ("The Pipers Melody"));
	    }
	}
      if (fch->leader == ch)
	fch->leader = fch;
    }
  return (found);
}

void do_order (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;//, *victim2;
  CHAR_DATA *och;
  CHAR_DATA *och_next;
  bool found;
  bool fAll;
  argument = one_argument (argument, arg);
  one_argument(one_argument (argument, arg2),arg3);
  if (!str_prefix (arg2, "yell") || !str_prefix (arg2, "shout")
      || !str_prefix (arg2, "defect") || !str_prefix (arg2, "plan")
      || !str_prefix (arg2, "delete") || !str_prefix (arg2, "clan")
      || !str_prefix (arg2, "steal") || !str_prefix (arg2, "quit")
      || !str_prefix (arg2, "email") || !str_prefix (arg2, "reroll")
      || !str_prefix (arg2, "reincarnate")
      || !str_prefix (arg2, "duel") || !str_prefix (arg2, "superduel")
      || !str_prefix (arg2, "alias") || !str_prefix (arg2, "ooc")
      || !str_prefix (arg2, "ignore"))
    //      !str_prefix (arg2, "enter") ||
    //      !str_prefix (arg2, "exit") ||
    //      !str_prefix (arg2, "climb") ||
    //      !str_prefix (arg2, "play") ||
    //      !str_prefix (arg2, "crawl") ||
    //      !str_prefix (arg2, "descend") ||
    //      !str_prefix (arg2, "scale") ||
    //      !str_prefix (arg2, "jump") ||
    //      !str_prefix (arg2, "tug") ||
    //      !str_prefix (arg2, "ring") ||
    //      !str_prefix (arg2, "shove") ||
    //      !str_prefix (arg2, "smash"))
    {
      send_to_char ("Sorry, you can't order that.\n\r", ch);
      return;
    }
 /* if (!str_prefix (arg2, "rescue"))
    //      || !str_prefix (arg2, "hit") ||
    //      !str_prefix (arg2, "kill"))
    {
      send_to_char ("You may be powerful, but he will not tank.\n\r", ch);
      return;
    }*/
  if (arg[0] == '\0' || argument[0] == '\0')
    {
      send_to_char ("Order whom to do what?\n\r", ch);
      return;
    }
  if (IS_AFFECTED (ch, AFF_CHARM))
    {
      send_to_char ("You feel like taking, not giving, orders.\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "all"))
    {
      fAll = TRUE;
      victim = NULL;
      if (!str_prefix (arg2, "cast") || !str_prefix (arg2, "pray")
	  || !str_prefix (arg2, "chant") || !str_prefix (arg2, "sing"))
	{
	  send_to_char
	    ("Though a powerful master you are, your minions can not perform this bidding.\n\r",
	     ch);
	  return;
	}
    }
  
  else
    {
      fAll = FALSE;
      if ((victim = get_char_room (ch, arg)) == NULL)
	{
	  send_to_char ("They aren't here.\n\r", ch);
	  return;
	}
      
      if (victim == ch)
	{
	  send_to_char ("Aye aye, right away!\n\r", ch);
	  return;
	}
      if (IS_NPC (victim) && (!str_prefix (arg2, "mount")))
	{
	  send_to_char ("Do it yourself!\n\r", ch);
	  return;
	}
      if (!IS_AFFECTED (victim, AFF_CHARM) || victim->master != ch
	  || (IS_IMMORTAL (victim) && victim->trust >= ch->trust))
	{
	  send_to_char ("Do it yourself!\n\r", ch);
	  return;
	}
      if ((!str_prefix (arg2, "cast") || !str_prefix (arg2, "pray")
	   || (!str_prefix (arg2, "berserk") && !IS_SET(victim->act2,ACT_FAMILIAR))
	   || !str_prefix (arg2, "chant")
	   || !str_prefix (arg2, "sing")) && (IS_NPC (victim) && !IS_SET(victim->act2,ACT_FAMILIAR)))
	{
	  send_to_char
	    ("Though a powerful master you are, your minion can not perform this bidding.\n\r",
	     ch);
	  return;
	}
    }
  found = FALSE;
  for (och = ch->in_room->people; och != NULL; och = och_next)
    {
      och_next = och->next_in_room;
      if (IS_NPC (och) && (!str_prefix (arg2, "mount")))
	return;
      if (IS_AFFECTED (och, AFF_CHARM)
	  && och->master == ch && (fAll || och == victim))
	{
	  if (!IS_SET(och->act2,ACT_FAMILIAR))
            isorder = TRUE;
	  if (!str_prefix(arg2,"tell") || !str_prefix(arg2,"telepath"))
	  {
	    CHAR_DATA *tch;
	    tch = get_char_world (ch, arg3);
	    if (tch != NULL && !IS_NPC(tch))
	    {
	      if (is_ignoring(tch,ch))
	      {
  	        WAIT_STATE (ch, PULSE_VIOLENCE);
		sprintf(buf, "%s is ignoring you.\n\r", tch->name);
		send_to_char(buf, ch);
		return;
	      }
	    }
	  }

		      
	  found = TRUE;
	  sprintf (buf, "$n orders you to '%s'.", argument);
	  act (buf, ch, NULL, och, TO_VICT);
	  interpret (och, argument);
	}
    }
  if (found)
    {
      WAIT_STATE (ch, PULSE_VIOLENCE);
      send_to_char ("Ok.\n\r", ch);
    }
  
  else
    send_to_char ("You have no followers here.\n\r", ch);
  return;
}

void do_swap (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_STRING_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  CHAR_DATA *newleader, *gch, *ach;
  struct swap_list *slist = NULL, *tmpswap, *tmpswap_next;
  one_argument (argument, arg);
  if (IS_AFFECTED (ch, AFF_CHARM))
    {
      send_to_char ("Sorry, you can't do that while charmed.\n\r", ch);
      return;
    }
  if (arg[0] == '\0')
    {
      send_to_char ("Who would you like to make group leader?\n\r", ch);
      return;
    }
  if ((newleader = get_char_world (ch, arg)) == NULL
      || !can_see (ch, newleader))
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (!is_same_group (newleader, ch))
    {
      act ("$N is not a member of your group.", ch, NULL, newleader, TO_CHAR);
      return;
    }
  ach = newleader;
  while (ach->leader && ach->leader != ach)
    ach = ach->leader;
  if (ach != ch)
    {
      act ("You aren't group leader of $N.", ch, NULL, newleader, TO_CHAR);
      return;
    }
  sprintf (tcbuf, "%s swaps group leadership with ", capitalize (ch->name));
  sprintf (tcbuf + strlen (tcbuf), "%s.\n\r", capitalize (newleader->name));
  for (gch = char_list; gch != NULL; gch = gch->next)
    if (ch != gch && gch != newleader
	&& is_same_group (gch, newleader) && !IS_NPC (gch))
      {
	tmpswap = (struct swap_list *) malloc (sizeof (struct swap_list));
	tmpswap->next = slist;
	tmpswap->member = gch;
	slist = tmpswap;
      }
  for (tmpswap = slist; tmpswap; tmpswap = tmpswap_next)
    {
      tmpswap_next = tmpswap->next;
      send_to_char (tcbuf, tmpswap->member);
      tmpswap->member->leader = newleader;
      tmpswap->member->master = newleader;
      free (tmpswap);
    }
  ch->leader = newleader;
  ch->master = newleader;
  newleader->leader = NULL;
  newleader->master = NULL;
  act ("$n has made you group leader.", ch, NULL, newleader, TO_VICT);
  act ("You swap group leadership with $N.", ch, NULL, newleader, TO_CHAR);
  return;
}

bool is_Class_in_group (CHAR_DATA * ch, int Class)
{

  // bool found = FALSE; /* unused */
  CHAR_DATA *gleader;
  CHAR_DATA *ach;
  gleader = get_group_leader (ch);
  if (gleader->Class == Class)
    return (TRUE);
  ach = char_list;
  while (ach != NULL)
    {

      // if they are in the same group then check the Class of ach
      if (gleader == get_group_leader (ach))
        {
          if (ach->Class == Class)
            return (TRUE);
        }
      ach = ach->next;
    }
  return (FALSE);
}

bool is_nopk_in_group (CHAR_DATA * ch)
{

  // bool found = FALSE; /* unused */
  CHAR_DATA *gleader;
  CHAR_DATA *ach;
  gleader = get_group_leader (ch);
  if (!IS_NPC(gleader) && !gleader->pcdata->loner)
    return (TRUE);
  ach = char_list;
  while (ach != NULL)
    {

      // if they are in the same group then check the Class of ach
      if (gleader == get_group_leader (ach))
        {
          if (!IS_NPC(ach) && !ach->pcdata->loner)
            return (TRUE);
        }
      ach = ach->next;
    }
  return (FALSE);
}

bool is_player_in_group (CHAR_DATA * ch)
{

  // bool found = FALSE; /* unused */
  CHAR_DATA *gleader;
  CHAR_DATA *ach;
  gleader = get_group_leader (ch);
  if (!IS_NPC(gleader) && ch != gleader)
    return (TRUE);
  ach = char_list;
  while (ach != NULL)
    {

      // if they are in the same group then check the Class of ach
      if (gleader == get_group_leader (ach))
        {
          if (!IS_NPC(ach) && ach != ch)
            return (TRUE);
        }
      ach = ach->next;
    }
  return (FALSE);
}

void do_group (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      CHAR_DATA *gch;
      CHAR_DATA *leader;

      leader = get_group_leader (ch);
      sprintf (buf, "%s's group:\n\r", (is_affected (leader, gsn_mask)
					&& leader->mask !=
					NULL) ? leader->
	       mask : PERS (leader, ch));
      send_to_char (buf, ch);
      for (gch = char_list; gch != NULL; gch = gch->next)
	{
	  if (is_same_group (gch, ch))
	    {
	      sprintf (buf,
		       "[%3d %s/``%s/``%s] %-16s %4d/%4d hp %4d/%4d mana %4d/%4d mv %5ld xp\n\r",
		       gch->level,
		       IS_NPC (gch) ? "Mob" : Class_table[gch->Class].
		       who_name,
		       IS_NPC (gch) ? "Mob" : ((gch->level > 30) ?
					       Class_table[gch->Class2].
					       who_name : "???"),
		       IS_NPC (gch) ? "Mob" : ((gch->level > 60) ?
					       Class_table[gch->Class3].
					       who_name : "???"),
		       (is_affected (gch, gsn_mask) && gch->mask !=
			NULL) ? gch->mask : capitalize (PERS (gch, ch)),
		       gch->hit, gch->max_hit, gch->mana, gch->max_mana,
		       gch->move, gch->max_move,
		       (!IS_NPC (gch)) ? gch->pcdata->group_exp : 0);
	      send_to_char (buf, ch);
	    }
	}
      return;
    }
  victim = get_char_room (ch, arg);

  // make sure victim is not null
  if (victim == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (is_affected (ch, gsn_mirror))
    {
      act ("You can't group with someone when you are mirrored.", ch,
	   NULL, NULL, TO_CHAR);
      return;
    }
  if (is_affected (victim, gsn_mirror))
    {
      act ("You can't group with someone who is mirrored.", ch, NULL,
	   NULL, TO_CHAR);
      return;
    }
  if (ch->master != NULL || (ch->leader != NULL && ch->leader != ch))
    {
      send_to_char ("But you are following someone else!\n\r", ch);
      return;
    }
  // Enact race restrictions. Current race restrictions are:
  //  - Sidhe can only group with Sidhe
  /* Mike 2/19/07 - Remove group restrictions based on race (Tolerance!)
  if (ch->race == PC_RACE_SIDHE || victim->race == PC_RACE_SIDHE)
    {
      if (victim->race != PC_RACE_SIDHE)
	{
	  send_to_char ("You can not group with a non-Sidhe.\n\r", ch);
	  send_to_char ("You can not group with a Sidhe.\n\r", victim);
	  return;
	}
      if (ch->race != PC_RACE_SIDHE)
	{
	  send_to_char ("You can not group with a Sidhe.\n\r", ch);
	  send_to_char ("You can not group with a non-Sidhe.\n\r", victim);
	  return;
	}
    }				// end Sidhe grouping restrictions

  if (ch->Class == PC_CLASS_CHAOS_JESTER || victim->Class == PC_CLASS_CHAOS_JESTER
      || is_Class_in_group (ch,PC_CLASS_CHAOS_JESTER) || is_Class_in_group (victim,PC_CLASS_CHAOS_JESTER))
  {
    if (is_nopk_in_group(ch) || is_nopk_in_group(victim))
      {
	send_to_char("Chaos Jesters may not group with any nopks.\n\r",ch);
	send_to_char("Chaos Jesters may not group with any nopks.\n\r",victim);
	return;
      }
  }

    

  // Akamai 6/4/98 -- Bug: #10 - Vroath grouping restrictions
  // Vroath can not group with Elves, Syvin, Canthi
  // Vroath can only group with Dwarves and other Vroath
  if ((ch->race == PC_RACE_VROATH)
      || (victim->race == PC_RACE_VROATH)
      || is_race_in_group (ch, PC_RACE_VROATH)
      || is_race_in_group (victim, PC_RACE_VROATH))
    {

      // If ch->race is Vroath or if there is Vroath in the group
      // then we check the victim race
      if (victim->race == PC_RACE_ELF)
	{
	  if (ch->race == PC_RACE_VROATH)
	    {
	      send_to_char
		("You can not tolerate a group with an Elf.\n\r", ch);
	    }

	  else
	    {
	      send_to_char
		("Elves and Vro'ath don't get along in a group.\n\r", ch);
	    }
	  send_to_char
	    ("Vro'ath can't tolerate Elves in their group.\n\r", victim);
	  log_activity ("Grouping Restriction: Elf and Vro'ath", ch, victim);
	  return;
	}
      if (is_race_in_group (victim,PC_RACE_ELF))
	{
	  if (ch->race == PC_RACE_VROATH)
            {
              send_to_char
                ("You can not tolerate a group with an Elf.\n\r", ch);
	    }

          else
            {
              send_to_char
                ("Elves and Vro'ath don't get along in a group.\n\r", ch);
            }
          send_to_char
            ("Vro'ath can't tolerate Elves in their group.\n\r", victim);
          log_activity ("Grouping Restriction: Elf and Vro'ath", ch, victim);
          return;
        }


      if (victim->race == PC_RACE_CANTHI)
	{
	  if (ch->race == PC_RACE_VROATH)
	    {
	      send_to_char
		("You can not tolerate a group with a Canthi.\n\r", ch);
	    }

	  else
	    {
	      send_to_char
		("Canthi and Vro'ath don't get along in a group.\n\r", ch);
	    }
	  send_to_char
	    ("Vro'ath can't tolerate Canthi in their group.\n\r", victim);
	  log_activity ("Grouping Restriction: Canthi and Vro'ath", ch,
			victim);
	  return;
	}
      if (is_race_in_group (victim, PC_RACE_CANTHI))
	{
	  if (ch->race == PC_RACE_VROATH)
            {
              send_to_char
                ("You can not tolerate a group with a Canthi.\n\r", ch);
            }

          else
            {
              send_to_char
                ("Canthi and Vro'ath don't get along in a group.\n\r", ch);
            }
          send_to_char
            ("Vro'ath can't tolerate Canthi in their group.\n\r", victim);
          log_activity ("Grouping Restriction: Canthi and Vro'ath", ch,
                        victim);
          return;
        }

      if (victim->race == PC_RACE_NIDAE)
        {
          if (ch->race == PC_RACE_VROATH)
            {
              send_to_char
                ("You can not tolerate a group with a Nidae.\n\r", ch);
            }

          else
            {
              send_to_char
                ("Nidae and Vro'ath don't get along in a group.\n\r", ch);
            }
          send_to_char
            ("Vro'ath can't tolerate Nidae in their group.\n\r", victim);
          log_activity ("Grouping Restriction: Canthi and Vro'ath", ch,
                        victim);
          return;
        }
      if (is_race_in_group (victim, PC_RACE_NIDAE))
        {
          if (ch->race == PC_RACE_VROATH)
            {
              send_to_char
                ("You can not tolerate a group with a Nidae.\n\r", ch);
            }

          else
            {
              send_to_char
                ("Nidae and Vro'ath don't get along in a group.\n\r", ch);
            }
          send_to_char
            ("Vro'ath can't tolerate Nidae in their group.\n\r", victim);
          log_activity ("Grouping Restriction: Nidae and Vro'ath", ch,
                        victim);
          return;
        }


      if (victim->race == PC_RACE_NERIX)
        {
          if (ch->race == PC_RACE_VROATH)
            {
              send_to_char
                ("You can not tolerate a group with a Nerix.\n\r", ch);
            }

          else
            {
              send_to_char
                ("Nerix and Vro'ath don't get along in a group.\n\r", ch);
            }
          send_to_char
            ("Vro'ath can't tolerate Nerix in their group.\n\r", victim);
          log_activity ("Grouping Restriction: Nerix and Vro'ath", ch,
                        victim);
          return;
        }
      if (is_race_in_group (victim, PC_RACE_NERIX))
        {
          if (ch->race == PC_RACE_VROATH)
            {
              send_to_char
                ("You can not tolerate a group with a Nerix.\n\r", ch);
            }

          else
            {
              send_to_char
                ("Nerix and Vro'ath don't get along in a group.\n\r", ch);
            }
          send_to_char
            ("Vro'ath can't tolerate Nerix in their group.\n\r", victim);
          log_activity ("Grouping Restriction: Nerix and Vro'ath", ch,
                        victim);
          return;
        }


      if (victim->race == PC_RACE_SYVIN)
	{
	  if (ch->race == PC_RACE_VROATH)
	    {
	      send_to_char
		("You can not tolerate a group with a Syvin.\n\r", ch);
	    }

	  else
	    {
	      send_to_char
		("Syvin and Vro'ath don't get along in a group.\n\r", ch);
	    }
	  send_to_char
	    ("Vro'ath can't tolerate Syvin in their group.\n\r", victim);
	  log_activity ("Grouping Restriction: Syvin and Vro'ath", ch,
			victim);
	  return;
	}
      if (is_race_in_group(victim,PC_RACE_SYVIN))
        {
	  
	  if (ch->race == PC_RACE_VROATH)
	    {
	  send_to_char
	    ("You can not tolerate a group with a Syvin.\n\r", ch);
	    }
	  
	  else
	    {
              send_to_char
                ("Syvin and Vro'ath don't get along in a group.\n\r", ch);
            }
          send_to_char
            ("Vro'ath can't tolerate Syvin in their group.\n\r", victim);
          log_activity ("Grouping Restriction: Syvin and Vro'ath", ch,
                        victim);
          return;
	}
      
      // If victim->race is Vroath, then we must check the character race
      // *and* that no restricted races are already in the group
      if ((ch->race == PC_RACE_ELF) || is_race_in_group (ch, PC_RACE_ELF))
	{
	  if (ch->race == PC_RACE_ELF)
	    {
	      send_to_char ("Vro'ath will not group with your kind.\n\r", ch);
	    }

	  else
	    {
	      send_to_char
		("Vro'ath and Elves do not get along in a group.\n\r", ch);
	    }
	  send_to_char ("You can not stand to group with an Elf.\n\r",
			victim);
	  log_activity ("Grouping Restriction: Vro'ath and Elf", ch, victim);
	  return;
	}
      if ((ch->race == PC_RACE_CANTHI)
	  || is_race_in_group (ch, PC_RACE_CANTHI))
	{
	  if (ch->race == PC_RACE_CANTHI)
	    {
	      send_to_char ("Vro'ath will not group with your kind.\n\r", ch);
	    }

	  else
	    {
	      send_to_char
		("Vro'ath and Canthi do not get along in a group.\n\r", ch);
	    }
	  send_to_char ("You can not stand to group with a Canthi.\n\r",
			victim);
	  log_activity ("Grouping Restriction: Vro'ath and Canthi", ch,
			victim);
	  return;
	}
      if ((ch->race == PC_RACE_SYVIN) || is_race_in_group (ch, PC_RACE_SYVIN))
	{
	  if (ch->race == PC_RACE_SYVIN)
	    {
	      send_to_char ("Vro'ath will not group with your kind.\n\r", ch);
	    }

	  else
	    {
	      send_to_char
		("Vro'ath and Syvin do not get along in a group.\n\r", ch);
	    }
	  send_to_char ("You can not stand to group with a Syvin.\n\r",
			victim);
	  log_activity ("Grouping Restriction: Vro'ath and Syvin", ch,
			victim);
	  return;
	}
      if ((ch->race == PC_RACE_NIDAE) || is_race_in_group (ch, PC_RACE_NIDAE))
      {
	if (ch->race == PC_RACE_NIDAE)
	  {
	    send_to_char ("Vro'ath will not group with your kind.\n\r", ch);
	  }

          else
            {
              send_to_char
                ("Vro'ath and Nidae do not get along in a group.\n\r", ch);
            }
	send_to_char ("You can not stand to group with a Nidae.\n\r",
		      victim);
	log_activity ("Grouping Restriction: Vro'ath and Nidae", ch,
		      victim);
	return;
      }
      if ((ch->race == PC_RACE_NERIX) || is_race_in_group (ch, PC_RACE_NERIX))
      {
	if (ch->race == PC_RACE_NERIX)
	  {
	    send_to_char ("Vro'ath will not group with your kind.\n\r", ch);
	  }

          else
            {
              send_to_char
                ("Vro'ath and Nerix do not get along in a group.\n\r", ch);
            }
	send_to_char ("You can not stand to group with a Nerix.\n\r",
		      victim);
	log_activity ("Grouping Restriction: Vro'ath and Nerix", ch,
		      victim);
	return;
      }


    }				// end Vroath grouping restrictions
   //Litan grouping restrictions
  if ((ch->race == PC_RACE_LITAN) || (victim->race == PC_RACE_LITAN)
        || is_race_in_group (ch, PC_RACE_LITAN)
        || is_race_in_group (victim, PC_RACE_LITAN))
  {
    if (victim->race == PC_RACE_CANTHI)
      {
	if (ch->race == PC_RACE_LITAN)
	  {
              send_to_char
                ("You can not tolerate a group with a Canthi.\n\r", ch);
	  }

          else
            {
              send_to_char
                ("Canthi and Litans don't get along in a group.\n\r", ch);
            }
          send_to_char
            ("Litans can't tolerate Canthi in their group.\n\r", victim);
          log_activity ("Grouping Restriction: Canthi and Litan", ch,
                        victim);
          return;
      }
    if (is_race_in_group(victim,PC_RACE_CANTHI))
      {
        if (ch->race == PC_RACE_LITAN)
          {
              send_to_char
                ("You can not tolerate a group with a Canthi.\n\r", ch);
          }

          else
            {
              send_to_char
                ("Canthi and Litans don't get along in a group.\n\r", ch);
            }
          send_to_char
            ("Litans can't tolerate Canthi in their group.\n\r", victim);
          log_activity ("Grouping Restriction: Canthi and Litan", ch,
                        victim);
          return;
      }
    if (victim->race == PC_RACE_NIDAE)
      {
        if (ch->race == PC_RACE_LITAN)
          {
              send_to_char
                ("You can not tolerate a group with a Nidae.\n\r", ch);
          }

          else
            {
              send_to_char
                ("Nidae and Litans don't get along in a group.\n\r", ch);
            }
          send_to_char
            ("Litans can't tolerate Nidae in their group.\n\r", victim);
          log_activity ("Grouping Restriction: Nidae and Litan", ch,
                        victim);
          return;
      }
    if (is_race_in_group(victim,PC_RACE_NIDAE))
      {
        if (ch->race == PC_RACE_LITAN)
          {
              send_to_char
                ("You can not tolerate a group with a Nidae.\n\r", ch);
          }

          else
            {
              send_to_char
                ("Nidae and Litans don't get along in a group.\n\r", ch);
            }
          send_to_char
            ("Litans can't tolerate Nidae in their group.\n\r", victim);
          log_activity ("Grouping Restriction: Nidae and Litan", ch,
                        victim);
          return;
      }


    if ((ch->race == PC_RACE_CANTHI)
	|| is_race_in_group (ch, PC_RACE_CANTHI))
      {
	if (ch->race == PC_RACE_CANTHI)
	  {
	    send_to_char ("Litans will not group with your kind.\n\r", ch);
	  }

          else
            {
              send_to_char
                ("Litans and Canthi do not get along in a group.\n\r", ch);
            }
	send_to_char ("You can not stand to group with a Canthi.\n\r",
		      victim);
	log_activity ("Grouping Restriction: Litan and Canthi", ch,
		      victim);
	return;
      }
    if ((ch->race == PC_RACE_NIDAE)
        || is_race_in_group (ch, PC_RACE_NIDAE))
      {
        if (ch->race == PC_RACE_NIDAE)
          {
            send_to_char ("Litans will not group with your kind.\n\r", ch);
          }

          else
            {
              send_to_char
                ("Litans and Nidae do not get along in a group.\n\r", ch);
            }
        send_to_char ("You can not stand to group with a Nidae.\n\r",
                      victim);
        log_activity ("Grouping Restriction: Litan and Nidae", ch,
                      victim);
        return;
      }

  }

  if ((ch->race == PC_RACE_NIDAE) || (victim->race == PC_RACE_NIDAE)
      || is_race_in_group (ch, PC_RACE_NIDAE)
      || is_race_in_group (victim, PC_RACE_NIDAE))
    {
      if (victim->race == PC_RACE_CANTHI)
	{
	  if (ch->race == PC_RACE_NIDAE)
	    {
              send_to_char
                ("You can not tolerate a group with a Canthi.\n\r", ch);
	    }

          else
            {
              send_to_char
                ("Canthi and Nidae don't get along in a group.\n\r", ch);
            }
          send_to_char
            ("Nidae can't tolerate Canthi in their group.\n\r", victim);
          log_activity ("Grouping Restriction: Canthi and Litan", ch,
                        victim);
          return;
	}
      if (is_race_in_group(victim,PC_RACE_CANTHI))
	{
	  if (ch->race == PC_RACE_NIDAE)
	    {
              send_to_char
                ("You can not tolerate a group with a Canthi.\n\r", ch);
	    }

          else
            {
              send_to_char
                ("Canthi and Nidae don't get along in a group.\n\r", ch);
            }
          send_to_char
            ("Nidae can't tolerate Canthi in their group.\n\r", victim);
          log_activity ("Grouping Restriction: Canthi and Litan", ch,
                        victim);
          return;
	}
      if ((ch->race == PC_RACE_CANTHI)
	  || is_race_in_group (ch, PC_RACE_CANTHI))
	{
	  if (ch->race == PC_RACE_CANTHI)
	    {
	      send_to_char ("Nidae will not group with your kind.\n\r", ch);
	    }

          else
            {
              send_to_char
                ("Nidae and Canthi do not get along in a group.\n\r", ch);
            }
	  send_to_char ("You can not stand to group with a Canthi.\n\r",
			victim);
	  log_activity ("Grouping Restriction: Nidae and Canthi", ch,
			victim);
	  return;
	}

    }
*/
	  
  if (victim->master != ch && ch != victim)
    {
      act_new ("$N isn't following you.", ch, NULL, victim, TO_CHAR,
	       POS_SLEEPING);
      return;
    }
  if (IS_AFFECTED (victim, AFF_CHARM))
    {
      send_to_char ("You can't remove charmed mobs from your group.\n\r", ch);
      return;
    }
  if (IS_AFFECTED (ch, AFF_CHARM))
    {
      act_new ("You like your master too much to leave $m!", ch, NULL,
	       victim, TO_VICT, POS_SLEEPING);
      return;
    }
// Level restrictions are not currently in effect. These restrictions are
  // currently handled with wierd experience point allocation when levels are
  // to far out of bounds. 
  /*
     if(victim->level - ch->level > 8)
     {
     send_to_char("They are to high of a level for your group.\n\r", ch);
     return;
     }

     if(victim->level - ch->level < -8)
     {
     send_to_char("They are to low of a level for your group.\n\r", ch);
     return;
     }
   */
  if (is_same_group (victim, ch) && ch != victim)
    {
      if (!IS_NPC (victim))
	victim->pcdata->group_exp = 0;
      victim->leader = NULL;
      act_new ("$n removes $N from $s group.", ch, NULL, victim,
	       TO_NOTVICT, POS_RESTING);
      act_new ("$n removes you from $s group.", ch, NULL, victim,
	       TO_VICT, POS_SLEEPING);
      act_new ("You remove $N from your group.", ch, NULL, victim,
	       TO_CHAR, POS_SLEEPING);
      return;
    }
  // This does not actually fix the bug in the grouping it has just fixed the
  // orgininal alignment testing code. The ALIGN_IN_SPHERE macro has the same
  // alignment bug that has been in the code for quite some time.
  /*
     // Old buggy code is out
     if(!ALIGN_IS_NEUTRAL(victim))
     {
     if(!ALIGN_IN_SPHERE(ch,victim))
     {
     act("Your sphere of beliefs prohibits $N from joining your group.",
     ch, NULL, victim, TO_CHAR);
     act("Your sphere of beliefs prohibits you from joining $n's group.",
     ch, NULL, victim, TO_VICT);
     return;
     }
     }
   */

  // Akamai 6/4/98 -- Bug: #9 Alignment restrictions
  // This is the real fix for alignment restrictions
  // Minax 7-10-02 Removed alignment restrictions on groups because our
  // playerbase is small enough that we should take what we can get...for now.
  /* Mike 2/19/07 - Removed again till pbase picks up! 
  if (!in_group_sphere (ch, victim))
    {
      sprintf (buf, "do_group: %s is prevented from joining %s's group.",
	       victim->name, ch->name);
      log_string (buf);
     
      act ("Your sphere of beliefs prohibits $N from joining your group.",
	   ch, NULL, victim, TO_CHAR);
      act ("Your sphere of beliefs prohibits you from joining $n's group.",
	   ch, NULL, victim, TO_VICT);
      return;
    }
*/
  victim->leader = ch;
  act_new ("$N joins $n's group.", ch, NULL, victim, TO_NOTVICT, POS_RESTING);
  act_new ("You join $n's group.", ch, NULL, victim, TO_VICT, POS_SLEEPING);
  act_new ("$N joins your group.", ch, NULL, victim, TO_CHAR, POS_SLEEPING);
  return;
}

void group_gr (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (victim == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (ch->master != NULL || (ch->leader != NULL && ch->leader != ch))
    {
      send_to_char ("But you are following someone else!\n\r", ch);
      return;
    }
  if (victim->master != ch && ch != victim)
    {
      act ("$N isn't following you.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (IS_AFFECTED (victim, AFF_CHARM))
    {
      send_to_char ("You can't remove charmed mobs from your group.\n\r", ch);
      return;
    }
  if (IS_AFFECTED (ch, AFF_CHARM))
    {
      act ("You like your master too much to leave $m!", ch, NULL,
	   victim, TO_VICT);
      return;
    }

  victim->leader = ch;
  act ("$N joins $n's group.", ch, NULL, victim, TO_NOTVICT);
  act ("You join $n's group.", ch, NULL, victim, TO_VICT);
  act ("$N joins your group.", ch, NULL, victim, TO_CHAR);
  return;
}


/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *gch;
  int members;
  int amount_gold = 0, amount_silver = 0;
  int share_gold, share_silver;
  int extra_gold, extra_silver;
  argument = one_argument (argument, arg1);
  one_argument (argument, arg2);
  if (arg1[0] == '\0')
    {
      send_to_char ("Split how much?\n\r", ch);
      return;
    }
  amount_silver = atoi (arg1);
  if (arg2[0] != '\0')
    amount_gold = atoi (arg2);
  if (amount_gold < 0 || amount_silver < 0)
    {
      send_to_char ("Your group wouldn't like that.\n\r", ch);
      return;
    }
  if (amount_gold == 0 && amount_silver == 0)
    {
      send_to_char ("You hand out zero coins, but no one notices.\n\r", ch);
      return;
    }
  if (ch->gold < amount_gold || ch->silver < amount_silver)
    {
      send_to_char ("You don't have that much to split.\n\r", ch);
      return;
    }
  members = 0;
  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {

/*
 * Mathew: Added !IS_NPC(gch) to be sure uncharmed NPC's and mounts
 * do not get a share of the money. 4/13/99
 */
      if (is_same_group (gch, ch) && !IS_AFFECTED (gch, AFF_CHARM)
	  && !IS_NPC (gch))
	members++;
    }
  if (members < 2)
    {
      send_to_char ("Just keep it all.\n\r", ch);
      return;
    }
  share_silver = amount_silver / members;
  extra_silver = amount_silver % members;
  share_gold = amount_gold / members;
  extra_gold = amount_gold % members;
  if (share_gold == 0 && share_silver == 0)
    {
      send_to_char ("Don't even bother, cheapskate.\n\r", ch);
      return;
    }
  ch->silver -= amount_silver;
  ch->silver += share_silver + extra_silver;
  ch->gold -= amount_gold;
  ch->gold += share_gold + extra_gold;
  if (share_silver > 0)
    {
      sprintf (buf,
	       "You split %d silver coins. Your share is %d silver.\n\r",
	       amount_silver, share_silver + extra_silver);
      send_to_char (buf, ch);
    }
  if (share_gold > 0)
    {
      sprintf (buf,
	       "You split %d gold coins. Your share is %d gold.\n\r",
	       amount_gold, share_gold + extra_gold);
      send_to_char (buf, ch);
    }
  if (share_gold == 0)
    {
      sprintf (buf,
	       "$n splits %d silver coins. Your share is %d silver.",
	       amount_silver, share_silver);
    }

  else if (share_silver == 0)
    {
      sprintf (buf, "$n splits %d gold coins. Your share is %d gold.",
	       amount_gold, share_gold);
    }

  else
    {
      sprintf (buf,
	       "$n splits %d silver and %d gold coins, giving you %d silver and %d gold.\n\r",
	       amount_silver, amount_gold, share_silver, share_gold);
    }
  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {

/*
 * Mathew: Added !IS_NPC(gch) to be sure uncharmed NPC's and mounts
 * do not get a share of the money. 4/13/99
 */
      if (gch != ch && is_same_group (gch, ch)
	  && !IS_AFFECTED (gch, AFF_CHARM) && !IS_NPC (gch))
	{
	  act (buf, ch, NULL, gch, TO_VICT);
	  gch->gold += share_gold;
	  gch->silver += share_silver;
	}
    }
  return;
}

void do_gtell (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH], bufn[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  CHAR_DATA *gch;
  if (argument[0] == '\0')
    {
      send_to_char ("Tell your group what?\n\r", ch);
      return;
    }
  if (IS_SET (ch->comm, COMM_NOTELL))
    {
      send_to_char ("Your message didn't get through!\n\r", ch);
      return;
    }

  /*
   * Note use of send_to_char, so gtell works on sleepers.
   */

  //IBLIS 5/31/03 - Added Drunk Talk to clan talk
  if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    drunkify (ch, argument);
  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
	  cj_self(ch,argument);
  else 
  {
    sprintf (buf, "You tell the group `P'%s'``\n\r", argument);
    send_to_char(buf,ch);
  }

  if (ch->race == PC_RACE_NIDAE)
    nidaeify(ch,bufn,argument);
  else anti_nidaeify(ch,bufn,argument);
  
  sprintf (buf, "%s tells the group `P'%s'``\n\r",
      (is_affected (ch, gsn_mask)
       && ch->mask != NULL) ? ch->mask : ch->name, argument);
  sprintf (buf2, "%s tells the group `P'%s'``\n\r",
           (is_affected (ch, gsn_mask)
            && ch->mask != NULL) ? ch->mask : ch->name, bufn);
  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (is_same_group (gch, ch) && gch != ch)
      {
        if ((gch->race == PC_RACE_NIDAE  && !IS_IMMORTAL(gch))
			|| (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(gch)))
	{
	  if (gch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(gch))
 	    cj_channel(gch,bufn);
	  else send_to_char (buf2,gch);
	}
	else 
	{
  	  if (gch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(gch))
	    cj_channel(gch,argument);
	  else send_to_char (buf, gch);
	}
      }
    }
  return;
}


// Akamai 6/4/98 -- Bug: #9 Alignment restriction fix
// This is the real fix. This function checks the alignment sphere of
// all the people in a group and checks that adding a new person will
// not breach the allowed distance in the sphere.
bool in_group_sphere (CHAR_DATA * ch, CHAR_DATA * victim)
{

//  char buf[MAX_STRING_LENGTH];
  int gmin, gmax, valign;

  // offset all alignments so that they are all nonnegative
  valign = victim->alignment + ALIGN_MAX;
  gmin = get_group_min_align (ch) + ALIGN_MAX;
  gmax = get_group_max_align (ch) + ALIGN_MAX;

  /*
     sprintf (buf, "in_group_sphere: min=%d max=%d new_member=%d sphere=%d",
     gmin, gmax, valign, ALIGN_GROUP_SPHERE);
     log_string (buf);
   */

  // if adding victim is going to expand the current sphere 
  // then adjust the group max and min to reflect the new sphere
  if (valign > gmax)
    gmax = valign;
  if (valign < gmin)
    gmin = valign;

  // finally check that the new sphere is within the tolerance
  if ((gmax - gmin) <= ALIGN_GROUP_SPHERE)
    {
      return (TRUE);
    }

  else
    {
      return (FALSE);
    }
}


// Akamai 6/4/98 -- Bug: #9 Alignment restriction fix
// get the alignment of the character with the maximum alignment in the group
int get_group_max_align (CHAR_DATA * ch)
{
  int max_align = ALIGN_MIN;
  CHAR_DATA *gleader;
  CHAR_DATA *ach;
  gleader = get_group_leader (ch);
  if (gleader->alignment > max_align)
    max_align = gleader->alignment;
  ach = char_list;
  while (ach != NULL)
    {

      // only consider - real players - not pets
      if (!IS_NPC (ach))
	{

	  // if they are in the same group then check the align
	  // if the align is greater than the current max align, store it
	  if (gleader == get_group_leader (ach))
	    {
	      if (ach->alignment > max_align)
		max_align = ach->alignment;
	    }
	}
      ach = ach->next;
    }
  return (max_align);
}


// Akamai 6/4/98 -- Bug: #9 Alignment restriction fix
// get the alignment of the character with the minumum alignment in the group
int get_group_min_align (CHAR_DATA * ch)
{
  int min_align = ALIGN_MAX;
  CHAR_DATA *gleader;
  CHAR_DATA *ach;
  gleader = get_group_leader (ch);
  if (gleader->alignment < min_align)
    min_align = gleader->alignment;
  ach = char_list;
  while (ach != NULL)
    {

      // only consider - real players - not pets
      if (!IS_NPC (ach))
	{

	  // if they are in the same group then check the align
	  // if the align is less than the current min align, store it
	  if (gleader == get_group_leader (ach))
	    {
	      if (ach->alignment < min_align)
		min_align = ach->alignment;
	    }
	}
      ach = ach->next;
    }
  return (min_align);
}


// Akamai 6/4/98 -- Simply return the group leader of the given character
CHAR_DATA *get_group_leader (CHAR_DATA * ch)
{
  int loop = 0;
  if (ch == NULL)
    return (NULL);

  // the field leader is a pointer to a list of character records
  // the character record that has null in the leader field or the
  // pointer to the character record itself is the leader

  // find the leader of ach (the character with a null leader field)
  while (ch->leader && (ch->leader != ch) && (loop < MAX_PC_GROUP))
    {
      loop++;
      ch = ch->leader;
    }

  // There is a subtle bug here that was in the original code. If the loop
  // ends because the loop counter exceeds MAX_PC_GROUP then it is likely
  // that varible ch has a bogus pointer as the group leader
  return (ch);
}


/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group (CHAR_DATA * ach, CHAR_DATA * bch)
{
  if (ach == NULL || bch == NULL)
    return (FALSE);
  if (ach == bch)
    return (TRUE);
  ach = get_group_leader (ach);
  bch = get_group_leader (bch);

  // if they are lead by the same character, then they are in the same group
  return (ach == bch);
}


// Akamai 6/4/98 -- Bug: #10 Race grouping restrictions
// This function simply checks if a given race is currently in the group with
// the character provided.
bool is_race_in_group (CHAR_DATA * ch, int race)
{

  // bool found = FALSE; /* unused */
  CHAR_DATA *gleader;
  CHAR_DATA *ach;
  gleader = get_group_leader (ch);
  if (gleader->race == race)
    return (TRUE);
  ach = char_list;
  while (ach != NULL)
    {

      // if they are in the same group then check the race of ach
      if (gleader == get_group_leader (ach))
	{
	  if (ach->race == race)
	    return (TRUE);
	}
      ach = ach->next;
    }
  return (FALSE);
}

int number_in_group (CHAR_DATA * ch)
{
  CHAR_DATA *ach;
  CHAR_DATA *gleader;
  int num = 0;
  if (ch == NULL)
    return (0);
  gleader = get_group_leader (ch);
  for (ach = char_list; ach; ach = ach->next)
    {
      if (gleader == get_group_leader (ach))
	num++;
    }
  return (num);
}

//Iblis - Allows rerolling of stats at level 1, or "reroll all" to convert old characters to the new xp
//system
void do_reroll (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_STRING_LENGTH];
  long reimb = 0;
  int gn = 0;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  if (IS_IMMORTAL (ch))
    {
      send_to_char ("Are you Insane?!\n\r", ch);
      return;
    }
  if (IS_NPC (ch))
    return;
  if (!strcmp (argument, "all"))
    {
      ch->pcdata->condition[COND_HUNGER] = 40;
      ch->pcdata->condition[COND_THIRST] = 40;
      if (ch->pcdata->new_style)	//They've already converted, so use total exp for their exp
	{
	  send_to_char ("No more free rides!  Use reincarnate!\r\n", ch);
	  return;

	}

      else
	{

	  //First off, calculate reimbursement
	  reimb = ch->level * ch->level * ch->level * 100;
	  for (gn = 0; gn < MAX_SKILL; gn++)
	    {
	      if (!ch->pcdata->hero)
		{
		  if (skill_table[gn].name != NULL
		      && level_for_skill (ch,
					  gn) <
		      91 && ch->pcdata->learned[gn] > 0)
		    {
		      reimb += 1500000.0 * (ch->pcdata->learned[gn] / 100.0);
		      ch->pcdata->learned[gn] = 0;
		      ch->pcdata->mod_learned[gn] = 0;
		    }
		}

	      else
		{
		  if (skill_table[gn].name != NULL
		      && ch->pcdata->learned[gn] > 0)
		    {
		      reimb += 1500000.0 / (ch->pcdata->learned[gn] / 100.0);
		      ch->pcdata->learned[gn] = 0;
		      ch->pcdata->learned[gn] = 0;
		    }
		}
	    }
	}
      if (reimb < 10000)
	reimb = 10000;

      //    reimb += 50000; //to fix the fact they have no weapon at 40%
      sprintf (tcbuf, "%s all 0", ch->name);
      do_sset (ch, tcbuf);
      group_add_all (ch);

      //remove all their items, even no_remove flagged ones
      for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if (IS_CLASS (ch, PC_CLASS_REAVER)
	      && (obj->wear_loc == WEAR_WIELD_L
		  || obj->wear_loc == WEAR_WIELD_R))
	    continue;
	  if (obj->wear_loc != WEAR_NONE)
	    unequip_char (ch, obj);
	}
      sprintf (tcbuf, "%s hp 20", ch->name);
      do_mset (ch, tcbuf);
      sprintf (tcbuf, "%s mana 100", ch->name);
      do_mset (ch, tcbuf);
      sprintf (tcbuf, "%s move 100", ch->name);
      do_mset (ch, tcbuf);

      ch->level = 1;

      ch->exp = reimb;
      ch->pcdata->totalxp = reimb;
      ch->hit = 20;
      ch->mana = 100;
      ch->move = 100;
      ch->train = 2;
      do_pardon (ch, ch->name);

      sprintf (tcbuf, "%s loner false", ch->name);
      do_mset (ch, tcbuf);
      do_reroll (ch, tcbuf);

      //IBLIS 5/19/03 - Signal they are a new style character now
      ch->pcdata->new_style = 1;
      switch (ch->Class)
	{
	case 0:
	  sprintf (tcbuf, "%s Zeryn", ch->name);
	  break;
	case 1:
	  sprintf (tcbuf, "%s Isaac", ch->name);
	  break;
	case 2:
	  sprintf (tcbuf, "%s Ivan", ch->name);
	  break;
	case 3:
	  sprintf (tcbuf, "%s Vedros", ch->name);
	  break;
	case 4:
	  sprintf (tcbuf, "%s Vanya", ch->name);
	  break;
	case 5:
	  sprintf (tcbuf, "%s Aldera", ch->name);
	  break;
	case 6:
	  sprintf (tcbuf, "%s Godfrey", ch->name);
	  break;
	case 7:
	  sprintf (tcbuf, "%s Jericka", ch->name);
	  break;
	case 8:
	  sprintf (tcbuf, "%s Azaran", ch->name);
	  break;
	default:
	  sprintf (tcbuf, "%s 4134", ch->name);
	  break;
	}
      do_transfer (ch, tcbuf);
      return;
    }
  if (ch->level > 1 || (ch->race == PC_RACE_NERIX && ch->pcdata->nage > 1))
    {
      send_to_char ("You've advanced too far to reroll.\n\r", ch);
      return;
    }
  if (ch->race == PC_RACE_NIDAE && ch->in_room && !IS_AFFECTED(ch,AFF_SLOW))
  {
	  send_to_char("Please do not reroll in water.  I know this is a pain. Sorry.\n\r",ch);
	  return;
  }
  roll_stats (ch);

  strcpy (tcbuf,
	  " __^__                                                            __^__\n\r");
  strcat (tcbuf,
	  "( ___ )----------------------------------------------------------( ___ )\n\r");
  strcat (tcbuf,
	  " | / |                                                            | \\ |\n\r");
  strcat (tcbuf,
	  " | / |                          Stats:                            | \\ |\n\r");
  strcat (tcbuf,
	  " | / |------------------------------------------------------------| \\ |\n\r");
  strcat (tcbuf,
	  " | / |                                                            | \\ |\n\r");
  sprintf (tcbuf + strlen (tcbuf),
	   " | / |           Strength: %2d               Dexterity: %2d         | \\ |\n\r",
	   ch->perm_stat[STAT_STR], ch->perm_stat[STAT_DEX]);
  sprintf (tcbuf + strlen (tcbuf),
	   " | / |       Intelligence: %2d            Constitution: %2d         | \\ |\n\r",
	   ch->perm_stat[STAT_INT], ch->perm_stat[STAT_CON]);
  sprintf (tcbuf + strlen (tcbuf),
	   " | / |             Wisdom: %2d                Charisma: %2d         | \\ |\n\r",
	   ch->perm_stat[STAT_WIS], ch->perm_stat[STAT_CHA]);
  strcat (tcbuf,
	  " |___|                                                            |___|\n\r");
  strcat (tcbuf,
	  "(_____)----------------------------------------------------------(_____)\n\r");
  strcat (tcbuf, "\n\r");
  ch->train = 2;
  if (ch->pcdata->br_points != -1)
    {
      strcat (tcbuf,
	      "Type reroll again if these stats are not sufficient. \n\r");
      send_to_char (tcbuf, ch);
    }
}


//Iblis - Deal with MultiClassing at 31 and 61
void multiClass (DESCRIPTOR_DATA * d, char *argument)
{
  short *newClass=NULL;
  char tcbuf[MAX_STRING_LENGTH];
  short iClass;
  if (d->character->level == 30)
    newClass = &d->character->Class2;

  else if (d->character->level == 60)
    newClass = &d->character->Class3;

  else
    {
      if (!d->character->pcdata->new_style)
	{
	  send_to_char
	    ("You must convert to the new system (reroll all) before you can reincarnate.\r\n",
	     d->character);
	  return;
	}
      d->character->level = 1;
      d->character->pcdata->totalxp *= .80;
      d->character->exp = d->character->pcdata->totalxp;
      send_to_char
	("You have reincarnated.  Now you must chose your new Class.\r\n",
	 d->character);
    }

  switch (d->connected)
    {
    case CON_START_MULTICLASS:
      send_to_desc (d, "\n\r");
      send_to_desc (d,
		    " __^__                                                            __^__\n\r");
      send_to_desc (d,
		    "( ___ )----------------------------------------------------------( ___ )\n\r");
      send_to_desc (d,
		    " | / |                                                            | \\ |\n\r");
      send_to_desc (d,
		    " | / |                Please Choose a Class:                      | \\ |\n\r");
      send_to_desc (d,
		    " | / |------------------------------------------------------------| \\ |\n\r");
      send_to_desc (d,
		    " | / |                                                            | \\ |\n\r");
      sprintf (tcbuf,
	       " | / |             %s       %s                    | \\ |\n\r",
	       IS_SET (genraces[d->character->race],
		       CLASS_MAGE) ? "[`oM``]agic-User" :
	       "`aMagic-User``  ", IS_SET (genraces[d->character->race],
					   CLASS_CLERIC) ?
	       "[`oC``]leric" : "`aCleric``  ");
      send_to_desc (d, tcbuf);
      sprintf (tcbuf,
	       " | / |             %s            %s                   | \\ |\n\r",
	       IS_SET (genraces[d->character->race],
		       CLASS_THIEF) ? "[`oT``]hief" : "`aThief``  ",
	       IS_SET (genraces[d->character->race],
		       CLASS_WARRIOR) ? "[`oW``]arrior" : "`aWarrior``  ");
      send_to_desc (d, tcbuf);
      sprintf (tcbuf,
	       " | / |             %s           %s                      | \\ |\n\r",
	       IS_SET (genraces[d->character->race],
		       CLASS_RANGER) ? "[`oR``]anger" : "`aRanger``  ",
	       IS_SET (genraces[d->character->race],
		       CLASS_BARD) ? "[`oB``]ard" : "`aBard``  ");
      send_to_desc (d, tcbuf);
      sprintf (tcbuf,
	       " | / |             %s          %s                  | \\ |\n\r",
	       IS_SET (genraces[d->character->race],
		       CLASS_PALADIN) ? "[`oP``]aladin" :
	       "`aPaladin``  ", IS_SET (genraces[d->character->race],
					CLASS_ASSASSIN) ?
	       "[`oA``]ssassin" : "`aAssassin``  ");
      send_to_desc (d, tcbuf);
      sprintf (tcbuf,
	       " | / |             %s           %s                      | \\ |\n\r",
	       IS_SET (genraces[d->character->race],
		       CLASS_REAVER) ? "[`oRE``]aver" : "`aReaver``  ",
	       IS_SET (genraces[d->character->race],
		       CLASS_MONK) ? "[`oMO``]nk" : "`aMonk``  ");
      send_to_desc (d, tcbuf);
      sprintf (tcbuf,
	      " | / |             %s                                         | \\ |\n\r",
	       IS_SET (genraces[d->character->race],
		       CLASS_DRUID) ? "[`oD``]ruid" : "`aDruid``  ");
      send_to_desc (d, tcbuf);
      send_to_desc (d,
		    " |___|                                                            |___|\n\r");
      send_to_desc (d,
		    "(_____)----------------------------------------------------------(_____)\n\r");
      send_to_desc (d, "\n\rWhat Class would you like to be? ");
      d->connected = CON_FINISH_MULTICLASS;
      break;
    case CON_FINISH_MULTICLASS:
      iClass = Class_lookup (argument);
      if (iClass != -1)
	switch (iClass)
	  {
          default:
	    iClass = -1;
	    break;
	  case 0:
	    if (!IS_SET (genraces[d->character->race], CLASS_MAGE))
	      iClass = -2;
	    break;
	  case 1:
	    if (!IS_SET (genraces[d->character->race], CLASS_CLERIC))
	      iClass = -2;
	    break;
	  case 2:
	    if (!IS_SET (genraces[d->character->race], CLASS_THIEF))
	      iClass = -2;
	    break;
	  case 3:
	    if (!IS_SET (genraces[d->character->race], CLASS_WARRIOR))
	      iClass = -2;
	    break;
	  case 4:
	    if (!IS_SET (genraces[d->character->race], CLASS_RANGER))
	      iClass = -2;
	    break;
	  case 5:
	    if (!IS_SET (genraces[d->character->race], CLASS_BARD))
	      iClass = -2;
	    break;
	  case 6:
	    if (!IS_SET (genraces[d->character->race], CLASS_PALADIN))
	      iClass = -2;
	    if (IS_CLASS (d->character, PC_CLASS_REAVER))
	      iClass = -3;
	    break;
	  case 7:
	    if (!IS_SET (genraces[d->character->race], CLASS_ASSASSIN))
	      iClass = -2;
	    if (d->character->race == PC_RACE_AVATAR && d->character->pcdata->avatar_type != 2)
  	      iClass = -6;
	    break;
	  case 8:

	    if (!IS_CLASS (d->character, PC_CLASS_REAVER))
	      {
		if (!IS_SET (genraces[d->character->race], CLASS_REAVER))
		  iClass = -2;
		if (IS_CLASS (d->character, PC_CLASS_PALADIN))
		  iClass = -3;
		if (!IS_CLASS (d->character, PC_CLASS_REAVER))
		  iClass = -4;
	      }
	    break;

	  case 9:
	    if (!IS_SET (genraces[d->character->race], CLASS_MONK))
	      iClass = -2;
	    if (IS_CLASS (d->character, PC_CLASS_REAVER))
	      iClass = -5;
	    break;

	case PC_CLASS_DRUID:
		if (!IS_SET(genraces[d->character->race], CLASS_DRUID))
			iClass = -2;
	break;
	  }
      if (iClass < 0)
	{
	  if (d->ansi)
	    write_to_buffer (d, ANSI_CLSHOME, 0);
	  send_to_desc (d, "\n\r");
	  send_to_desc (d,
			" __^__                                                            __^__\n\r");
	  send_to_desc (d,
			"( ___ )----------------------------------------------------------( ___ )\n\r");
	  send_to_desc (d,
			" | / |                                                            | \\ |\n\r");
	  send_to_desc (d,
			" | / |                Please Choose a Class:                      | \\ |\n\r");
	  send_to_desc (d,
			" | / |------------------------------------------------------------| \\ |\n\r");
	  send_to_desc (d,
			" | / |                                                            | \\ |\n\r");
	  sprintf (tcbuf,
		   " | / |             %s       %s                    | \\ |\n\r",
		   IS_SET (genraces[d->character->race],
			   CLASS_MAGE) ? "[`oM``]agic-User" :
		   "`aMagic-User``  ",
		   IS_SET (genraces[d->character->race],
			   CLASS_CLERIC) ? "[`oC``]leric" : "`aCleric``  ");
	  send_to_desc (d, tcbuf);
	  sprintf (tcbuf,
		   " | / |             %s            %s                   | \\ |\n\r",
		   IS_SET (genraces[d->character->race],
			   CLASS_THIEF) ? "[`oT``]hief" : "`aThief``  ",
		   IS_SET (genraces[d->character->race],
			   CLASS_WARRIOR) ? "[`oW``]arrior" :
		   "`aWarrior``  ");
	  send_to_desc (d, tcbuf);
	  sprintf (tcbuf,
		   " | / |             %s           %s                      | \\ |\n\r",
		   IS_SET (genraces[d->character->race],
			   CLASS_RANGER) ? "[`oR``]anger" :
		   "`aRanger``  ", IS_SET (genraces[d->character->race],
					   CLASS_BARD) ? "[`oB``]ard" :
		   "`aBard``  ");
	  send_to_desc (d, tcbuf);
	  sprintf (tcbuf,
		   " | / |             %s        %s                    | \\ |\n\r",
		   IS_SET (genraces[d->character->race],
			   CLASS_PALADIN) ? "[`oP``]aladin" :
		   "`aPaladin``  ", IS_SET (genraces[d->character->race],
					    CLASS_ASSASSIN) ?
		   "[`oA``]ssassin" : "`aAssassin``  ");
	  send_to_desc (d, tcbuf);
	  sprintf (tcbuf,
		   " | / |             %s           %s                      | \\ |\n\r",
		   IS_SET (genraces[d->character->race],
			   CLASS_REAVER) ? "[`oRE``]aver" :
		   "`aReaver``  ", IS_SET (genraces[d->character->race],
					   CLASS_MONK) ? "[`oMO``]nk" :
		   "`aMonk``  ");
	  send_to_desc (d, tcbuf);
	  sprintf (tcbuf,
		   " | / |            %s                                         | \\ |\n\r",
		   IS_SET (genraces[d->character->race],
			CLASS_DRUID) ? "[`oD``]ruid" : "`aDruid``  ");
	  send_to_desc (d, tcbuf);
	  send_to_desc (d,
			" |___|                                                            |___|\n\r");
	  send_to_desc (d,
			"(_____)----------------------------------------------------------(_____)\n\r\n\r");
	  if (iClass == -1)
	    send_to_desc (d, "That is not a valid Class.\n\r");

	  else if (iClass == -2)
	    send_to_desc (d,
			  "Your race selection prevents you from choosing that Class.\n\r");

	  else if (iClass == -3)
	    send_to_desc (d, "Paladins and Reavers don't mix well.\n\r");

	  else if (iClass == -4)
	    send_to_desc (d,
			  "If you want to be a Reaver, you have to start a Reaver.\n\r");

	  else if (iClass == -5)
	    send_to_desc (d,
			  "Monk can't be Reavers, since Monks can't use weapons.\n\r");
	  else send_to_desc (d, "Nopk Avatars cannot chose to be Assassins, who are forced to loner.\n\r");
	  send_to_desc (d, "What Class would you like to be? ");
	  return;
	}
      *newClass = iClass;
      if (iClass == 9)
	{
	  OBJ_DATA *weapon;
	  if ((weapon = get_eq_char (d->character, WEAR_WIELD_R)) != NULL)
	    unequip_char (d->character, weapon);
	  if ((weapon = get_eq_char (d->character, WEAR_WIELD_L)) != NULL)
	    unequip_char (d->character, weapon);
	}
      if (iClass == 7)
	{
	  OBJ_DATA *shield;
	  if ((shield = get_eq_char (d->character, WEAR_SHIELD)) != NULL)
	    unequip_char (d->character, shield);
	  if (clan_table[d->character->clan].ctype != CLAN_TYPE_PC
	      && d->character->pcdata->loner != TRUE)
	    {
	      send_to_char
		("`oYou are no longer protected.  As an Assassin, you are now a LONER!``\n\r",
		 d->character);
	      d->character->pcdata->loner = TRUE;
	    }
	}
      ++d->character->level;
      group_add_all (d->character);
      --d->character->level;

      advance_level (d->character);
    }
}


//Iblis 6/20/03 - NOTE NOTE NOTE I use br_points for some things, so you obviously
//cannot reincarnate during Battle Royale.
void do_reincarnate (CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d=NULL;
  OBJ_DATA *obj, *obj_next;
  int sn;

  char pword[MAX_INPUT_LENGTH];
  if (IS_NPC (ch))
    return;

  if (ch->in_room->vnum != ROOM_VNUM_ZANNAH)
    {
      send_to_char
	("Zannah is the only one who can help you with this.\r\n", ch);
      return;
    }
  if (IS_IMMORTAL (ch))
    {
      send_to_char ("Bad Imm. NO!\r\n", ch);
      return;
    }
  if (ch->race == PC_RACE_KALIAN || ch->race == PC_RACE_NERIX)
    {
      if (ch->level < 90)
        {
           send_to_char("You may not reincarnate until you are 90.\n\r",ch);
           return;
         }
    }
								
  if (ch->pcdata->avatar_type >= 3 && ch->race == PC_RACE_AVATAR)
  {
	  send_to_char("You may not reincarnate as an Avatar.\n\r",ch);
	  return;
  }
  if (!ch->pcdata->new_style)
    {
      send_to_char
	("You must convert to the new system (reroll all) before you can reincarnate.\r\n",
	 d->character);
      return;
    }
  if (battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner)))
    {
      send_to_char ("Not during battle royale!\r\n", ch);
      return;
    }
  if (!IS_NPC (ch))
    if ((time (NULL) - ch->pcdata->last_fight < 120) && (!IS_IMMORTAL(ch)))
      {
	send_to_char ("Nice try.  You can't reincarnate so easily.\n\r", ch);
	return;
      }
  one_argument_nl (argument, pword);
  if (ch->pcdata->br_points == -2)
    {
      if (argument[0] == '\0')
	{
	  send_to_char ("Reincarnate status removed.\n\r", ch);
	  ch->pcdata->br_points = 0;
	  return;
	}

      else
	{
	  if (strcmp (ch->pcdata->pwd, crypt (pword, ch->pcdata->pwd)) == 0)
	    {
	    }

	  else
	    {
	      send_to_char ("Wrong password.\n\r", ch);
	      ch->pcdata->br_points = 0;
	      return;
	    }
	}
    }
  if (argument[0] == '\0')
    {
      send_to_char
	("Type reincarnate <password> to reincarnate this character.\n\r",
	 ch);
      return;
    }
  if (strcmp (ch->pcdata->pwd, crypt (pword, ch->pcdata->pwd)) == 0
      && ch->pcdata->br_points != -2)
    {
      send_to_char
	("Type reincarnation <password> again to confirm this command.\n\r",
	 ch);
      send_to_char
	("WARNING: this command is irreversible without xp loss.\n\r", ch);
      send_to_char
	("Typing reincarnation with no argument will undo reincarnation status.\n\r",
	 ch);
      ch->pcdata->br_points = -2;
      wiznet ("$N is contemplating reincarnation.", ch, NULL, 0, 0,
	      get_trust (ch));
      return;
    }

  else if (ch->pcdata->br_points != -2)
    {
      send_to_char ("Wrong password.\n\r", ch);
      return;
    }
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->character == ch)
	{
	  char tcbuf[MAX_STRING_LENGTH];
	  act ("Zannah orates the sacred prayer, 'Noit anr Acnier'.", ch,
	       NULL, NULL, TO_ROOM);
	  send_to_char
	    ("Zannah orates the sacred prayer, 'Noit anr Acnier', and you feel young again.\r\n",
	     ch);
	  if (!((ch->race == PC_RACE_AVATAR && ch->pcdata->avatar_type == 0) || ch->race == PC_RACE_KALIAN 
				  || ch->race == PC_RACE_NERIX))
  	    ch->pcdata->totalxp *= .60;
	  
	  if (ch->race == PC_RACE_KALIAN || ch->race == PC_RACE_NERIX)
	  {
  	    affect_strip (ch, gsn_jurgnation);
	    affect_strip (ch, gsn_jalknation);
	    ch->pcdata->has_reincarnated = TRUE;
	    ch->pcdata->totalxp *= .10;
	  }
	  char_from_room(ch);
	  char_to_room(ch,get_room_index(ROOM_VNUM_REINC_ROOM));
	  ch->exp = ch->pcdata->totalxp;
	  ch->level = 1;
	  ch->pcdata->hp_gained = 0;
	  ch->pcdata->mana_gained = 0;
	  ch->pcdata->move_gained = 0;
	  ch->pcdata->lost_prayers = 0;

	  //remove all their items, even no_remove flagged ones
	  for (obj = ch->carrying; obj != NULL; obj = obj_next)
	    {
	      obj_next = obj->next_content;

	      if (IS_CLASS(ch,PC_CLASS_REAVER) && (obj->wear_loc == WEAR_WIELD_L || obj->wear_loc == WEAR_WIELD_R))
		{
		  unequip_char (ch,obj);
		  obj_from_char (obj);
		  extract_obj (obj);
		}
			
	      if (obj->wear_loc != WEAR_NONE)
		unequip_char (ch, obj);
	    }
      	  if (ch->race == PC_RACE_NERIX) 
	  {
            ch->pcdata->nage = 1;
            ch->pcdata->nplayed = ch->played + (int) (current_time - ch->logon);
	  }
	
	  ch->pcdata->br_points = -1;
	  //To fix pure clerics reincarnating
	  ch->pcdata->autoassist_level = AASSIST_MOBS;
	  affect_strip_skills(ch);
	  for (sn = 0; sn < MAX_SKILL; sn++)
	  {
	    ch->pcdata->learned[sn] = 0;
	    ch->pcdata->mod_learned[sn] = 0;
	  }
	  if (d->ansi)
	    write_to_buffer (d, ANSI_CLSHOME, 0);
	  send_to_desc (d, "\n\r");
	  send_to_desc (d,
			" __^__                                                            __^__\n\r");
	  send_to_desc (d,
			"( ___ )----------------------------------------------------------( ___ )\n\r");
	  send_to_desc (d,
			" | / |                                                            | \\ |\n\r");
	  send_to_desc (d,
			" | / |                Please Choose a Class:                      | \\ |\n\r");
	  send_to_desc (d,
			" | / |------------------------------------------------------------| \\ |\n\r");
	  send_to_desc (d,
			" | / |                                                            | \\ |\n\r");
	  sprintf (tcbuf,
		   " | / |             %s       %s                    | \\ |\n\r",
		   IS_SET (genraces[ch->race],
			   CLASS_MAGE) ? "[`oM``]agic-User" :
		   "`aMagic-User``  ", IS_SET (genraces[ch->race],
					       CLASS_CLERIC) ?
		   "[`oC``]leric" : "`aCleric``  ");
	  send_to_desc (d, tcbuf);
	  sprintf (tcbuf,
		   " | / |             %s            %s                   | \\ |\n\r",
		   IS_SET (genraces[ch->race],
			   CLASS_THIEF) ? "[`oT``]hief" : "`aThief``  ",
		   IS_SET (genraces[ch->race],
			   CLASS_WARRIOR) ? "[`oW``]arrior" :
		   "`aWarrior``  ");
	  send_to_desc (d, tcbuf);
	  sprintf (tcbuf,
		   " | / |             %s           %s                      | \\ |\n\r",
		   IS_SET (genraces[ch->race],
			   CLASS_RANGER) ? "[`oR``]anger" :
		   "`aRanger``  ", IS_SET (genraces[ch->race],
					   CLASS_BARD) ? "[`oB``]ard" :
		   "`aBard``  ");
	  send_to_desc (d, tcbuf);
	  sprintf (tcbuf,
		   " | / |             %s          %s                  | \\ |\n\r",
		   IS_SET (genraces[ch->race],
			   CLASS_PALADIN) ? "[`oP``]aladin" :
		   "`aPaladin``    ", IS_SET (genraces[ch->race],
					      CLASS_ASSASSIN) ?
		   "[`oA``]ssassin" : "`aAssassin``  ");
	  send_to_desc (d, tcbuf);
	  sprintf (tcbuf,
		   " | / |             %s           %s                      | \\ |\n\r",
		   IS_SET (genraces[ch->race],
			   CLASS_REAVER) ? "[`oRE``]aver" :
		   "`aReaver``  ", IS_SET (genraces[ch->race],
					   CLASS_MONK) ? "[`oMO``]nk" :
		   "`aMonk``  ");
	  send_to_desc (d, tcbuf);
	  sprintf (tcbuf,
		   " | / |             %s                                         | \\ |\n\r",
		   IS_SET (genraces[ch->race],
			CLASS_DRUID) ? "[`oD``]ruid" : "`aDruid``  ");
	  send_to_desc (d, tcbuf);
	  send_to_desc (d,
			" |___|                                                            |___|\n\r");
	  send_to_desc (d,
			"(_____)----------------------------------------------------------(_____)\n\r\n\r");
	  ch->level = 0;
	  d->connected = CON_GET_NEW_CLASS;
	}
    }
}

//Iblis - command to turn off annoying colors (to deal with Iverath vs Fairydust war)
void do_noblink (CHAR_DATA * ch, char *argument)
{
  if (IS_NPC (ch) || !ch->pcdata)
    return;
  if (ch->pcdata->noblinking)
    {
      ch->pcdata->noblinking = FALSE;
      send_to_char ("No Blinking is now turned `iOFF``.\n\r", ch);
      return;
    }

  else
    {
      ch->pcdata->noblinking = TRUE;
      send_to_char ("No Blinking is now turned `iON``.\n\r", ch);
      return;
    }
}

//Iblis - the small function that actually sends stuff over the pk channel
void pk_chan(char* text)
{
  CHAR_DATA* fch;
  for (fch = char_list; fch != NULL; fch = fch->next)
  {
    if (IS_NPC (fch) || IS_SET (fch->comm, COMM_NOPK))
      continue;
    send_to_char ("`l[`cPK`l]`` `c", fch);
    send_to_char (text, fch);
    send_to_char ("``\n\r", fch);
  }
}

//Iblis - the small function that actually sends stuff over the arena channel
void arena_chan(char* text)
{
  CHAR_DATA* fch;
  for (fch = char_list; fch != NULL; fch = fch->next)
  {
    if (IS_NPC (fch) || IS_SET (fch->comm, COMM_NOSUPERDUEL))
      continue;
    send_to_char ("`i[`oArena`i]`` `o", fch);
    send_to_char (text, fch);
    send_to_char ("``\n\r", fch);
  }
}

//Iblis - the actual pk channel usage command (the only thing this can do for players
//is toggle it on or off, since the messages sent over the channel are done via the code
//and not by an actual player command)
void do_pk (CHAR_DATA * ch, char *argument)
{
  if (IS_SET (ch->comm, COMM_NOPK))
    {
      send_to_char ("PK channel is now ON.\n\r", ch);
      REMOVE_BIT (ch->comm, COMM_NOPK);
    }

  else
    {
      send_to_char ("PK channel is now OFF.\n\r", ch);
      SET_BIT (ch->comm, COMM_NOPK);
    }
}

//Iblis - the actual arena channel usage command (the only thing this can do for players
////is toggle it on or off, since the messages sent over the channel are done via the code
////and not by an actual player command)
void do_arena (CHAR_DATA * ch, char *argument)
{
  if (IS_SET (ch->comm, COMM_NOSUPERDUEL))
    {
      send_to_char ("Arena channel is now ON.\n\r", ch);
      REMOVE_BIT (ch->comm, COMM_NOSUPERDUEL);
    }

  else
    {
      send_to_char ("Arena channel is now OFF.\n\r", ch);
      SET_BIT (ch->comm, COMM_NOSUPERDUEL);
    }
}


//Iblis - Iverath suggested bitching channel
void do_rant (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  
  if (argument[0] == '\0')
    {
      if (IS_SET (ch->comm2, COMM_NORANT))
        {
          send_to_char ("Rant channel is now ON.\n\r", ch);
          REMOVE_BIT (ch->comm2, COMM_NORANT);
        }

      else
        {
          send_to_char ("Rant channel is now OFF.\n\r", ch);
          SET_BIT (ch->comm2, COMM_NORANT);
        }
    }

  else
    /* gossip message sent, turn gossip on if it isn't already */
    {
	     
      if (IS_SET (ch->comm, COMM_QUIET))
        {
          send_to_char ("You must turn off quiet mode first.\n\r", ch);
          return;
        }
      if (IS_SET (ch->comm, COMM_NOCHANNELS))
        {
          send_to_char
            ("The gods have revoked your channel priviliges.\n\r", ch);
          return;
        }
      REMOVE_BIT (ch->comm2, COMM_NORANT);
      if (ch->level < 3)
        {
          send_to_char
            ("Sorry, you are too young to project your voice across the lands in that manner.\n\r",
             ch);
          return;
        }
      sprintf (buf, "`a[`dRant`a] `lYou`a: '`d%s`a'``\n\r", argument);
      send_to_char (buf, ch);
      if (!IS_SET (ch->comm2, COMM_SHUTUP))
      for (d = descriptor_list; d != NULL; d = d->next)
        {
          CHAR_DATA *victim;
          victim = d->original ? d->original : d->character;
          if (d->connected == CON_PLAYING && d->character != ch
              && !IS_SET (victim->comm2, COMM_NORANT)
              && !IS_SET (victim->comm, COMM_QUIET))
            {
              if (is_ignoring(victim, ch))
		continue;
              act_new ("`a[`dRant`a]`l $n`a:`a '`d$t`a'``", ch,
                       argument, d->character, TO_VICT, POS_SLEEPING);
            }
        }
    }
}

//Iblis - Iverath suggested poll channel, in which imms can pose a poll to the mud, in which all
//can vote
void do_poll (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  argument = one_argument(argument,arg);
  if (IS_NPC(ch))
	  return;
  
  if (arg[0] == '\0')
    {
      if (IS_SET (ch->comm2, COMM_NOPOLL))
        {
          send_to_char ("Poll channel is now ON.\n\r", ch);
          REMOVE_BIT (ch->comm2, COMM_NOPOLL);
        }
      
      else
        {
          send_to_char ("Poll channel is now OFF.\n\r", ch);
          SET_BIT (ch->comm2, COMM_NOPOLL);
        }
    }
  
  else if (!str_cmp(arg,"ask"))
    /* gossip message sent, turn gossip on if it isn't already */
    {
      if (!IS_IMMORTAL(ch))
	{
          send_to_char("Only immortals may start Polls.\n\r",ch);
          return;
	}
      if (poll_question[0] != '\0')
	{
	  send_to_char("There is already a Poll going on.\n\r",ch);
	  return;
	}
      
      if (IS_SET (ch->comm, COMM_QUIET))
	{
	  send_to_char ("You must turn off quiet mode first.\n\r", ch);
	  return;
	}
      if (IS_SET (ch->comm, COMM_NOCHANNELS))
	{
	send_to_char ("The gods have revoked your channel priviliges.\n\r", ch);
	return;
	}
      REMOVE_BIT (ch->comm2, COMM_NOPOLL);
      if (ch->level < 3)
        {
          send_to_char
            ("Sorry, you are too young to project your voice across the lands in that manner.\n\r",
             ch);
          return;
        }

      if (strlen (argument) < 10)
      {
        send_to_char("Less than 10 characters is not a poll!\n\r",ch);
        return;
      }  
      else
      {
      strcpy(poll_question,argument);
      poll_ticks = 3;
      sprintf (buf, "`a[`hPoll`a] `lYou`a: '`o%s`a'``\n\r", argument);
      send_to_char (buf, ch);
      if (!IS_SET (ch->comm2, COMM_SHUTUP))
      for (d = descriptor_list; d != NULL; d = d->next)
        {
          CHAR_DATA *victim;
          victim = d->original ? d->original : d->character;
          if (d->connected == CON_PLAYING && d->character != ch
              && !IS_SET (victim->comm2, COMM_NOPOLL)
              && !IS_SET (victim->comm, COMM_QUIET))
            {
              if (is_ignoring(victim, ch))
                continue;
              act_new ("`a[`hPoll`a]`l $n`a:`a '`o$t`a'``", ch,
                       argument, d->character, TO_VICT, POS_SLEEPING);
            }
        }
     }
    }
  else if (!str_cmp(arg,"vote"))
    {
      if (poll_question[0] == '\0')
	{
	  send_to_char("There is nothing to vote on!\n\r",ch);
	  return;
	}
      sprintf(buf,"`hThe current poll is `a: `o %s``.\n\r",poll_question);
      send_to_char(buf,ch);
      if (!str_cmp(argument,"yes"))
	{
	  if (ch->pcdata->voted == VOTED_YES)
	  {
		  send_to_char("You reaffirm your position.\n\r",ch);
		  return;
	  }
          if (ch->pcdata->voted == VOTED_NO)
	  {
	    send_to_char("`hYou change your vote to `a: `bYES``.\n\r",ch);
	    wiznet ("$N has changed their vote to `a: `bYES``", ch, NULL, WIZ_POLL, 0, get_trust (ch));
	  }
	    
	  else 
	  {
  	    send_to_char("`hYou have voted `a: `bYES``.\n\r",ch);
	    wiznet ("$N has voted `a: `bYES``", ch, NULL, WIZ_POLL, 0, get_trust (ch));
	  }
	  ch->pcdata->voted = VOTED_YES;

	}
      else if (!str_cmp(argument,"no"))
	{
	  if (ch->pcdata->voted == VOTED_NO)
	   {
	     send_to_char("You reaffirm your position.\n\r",ch);
	     return;
	   }
	  if (ch->pcdata->voted == VOTED_YES)
	  {
	    send_to_char("`hYou change your vote to `a: `bNO``.\n\r",ch);
	    wiznet ("$N has changed their vote to `a: `bNO``", ch, NULL, WIZ_POLL, 0, get_trust (ch));
	  }
	  else
	  {
	    send_to_char("`hYou have voted `a: `bNO``.\n\r",ch);
	    wiznet ("$N has voted `a: `bNO``", ch, NULL, WIZ_POLL, 0, get_trust (ch));
	  }
	  ch->pcdata->voted = VOTED_NO;
	}
      else 
	{
	  /*	  sprintf(buf,"`hThe current poll is `a: `o %s``.\n\r",poll_question);
		  send_to_char(buf,ch);*/
	  if (ch->pcdata->voted == VOTED_NOT)
	    send_to_char("You may either vote yes or no.\n\r",ch);
	  else if (ch->pcdata->voted == VOTED_YES)
	    send_to_char("`hYour vote is currently `a: `bYES``.\n\r",ch);
	  else send_to_char("`hYour vote is currently `a: `bNO``.\n\r",ch);
	  return;
	}
    }
  else 
    {
      send_to_char("Syntax : Poll     (toggles channel on/off)\n\r",ch);
      send_to_char("         Poll vote yes/no      (vote yes or no to the current poll)\n\r",ch);
      if (IS_IMMORTAL(ch))
	send_to_char("         Poll ask QUESTION      (start a poll based on QUESTION)\n\r",ch);
      return;
    }

}

//Iblis - Sayto command, basically like say but "Iblis says to Iverath 'Here's your new command.'"
void do_sayto (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *vch, *victim;
  char talktext[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH], bufn[MAX_STRING_LENGTH];


  //  int i=0;
  if (IS_SET (ch->in_room->room_flags, ROOM_SILENT) && !IS_IMMORTAL (ch))
    {
      send_to_char
        ("This room is shielded from disturbance by powerful wards of magic.\n\r",
         ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("Sayto whom what?\n\r", ch);
      return;
    }
  argument = one_argument(argument,arg);
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("Sayto whom what?\n\r", ch);
      return;
    }
  // End of Minax Changes
    if (ch->race == PC_RACE_SIDHE && !IS_IMMORTAL (ch))
    {
      act
	("The fragile structure of your vocal system prevents you from creating anything other than a high-pitched whine.",
	 ch, NULL, NULL, TO_CHAR);
      return;
    }
  if (ch->race == PC_RACE_LITAN && !IS_IMMORTAL (ch))
    {
      do_shout(ch,argument);
      return;
    }

  if (IS_SET(ch->act2,ACT_PUPPET))
    {
      do_switchtalk(ch,argument);
      return;
    }
  
  // IBLIS 5/31/03 - People wanted drunk talk (why who knows) so I deliver
  if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    {
      drunkify (ch, argument);
    }
  // Minax 6/29/02--When Vro'aths talk, they grunt and when Canthi talk,
  // they gurgle.
  /*if (dch->race == 4 && !IS_IMMORTAL (ch))
    {
      sprintf(buf,"$n gurgles to $N `a'`K%s`a'``",argument);
      act (buf, ch, NULL, victim, TO_NOTVICT);
      sprintf(buf,"$n gurgles to you `a'`K%s`a'``",argument);
      act (buf, ch, NULL, victim, TO_VICT);
      sprintf(buf,"You gurgle to $N `a'`K%s`a'``",argument);
      act (buf, ch, NULL, victim, TO_CHAR);
    }

  else if (ch->race == 5 && !IS_IMMORTAL (ch))
    {
      sprintf(buf,"$n grunts to $N `a'`K%s`a'``",argument);
      act (buf, ch, NULL, victim, TO_NOTVICT);
      sprintf(buf,"$n grunts to you `a'`K%s`a'``",argument);
      act (buf, ch, NULL, victim, TO_VICT);
      sprintf(buf,"You grunt to $N `a'`K%s`a'``",argument);
      act (buf, ch, NULL, victim, TO_CHAR);
    }

  else
    {
      sprintf(buf,"$n says to $N `a'`K%s`a'``",argument);
      act (buf, ch, NULL, victim, TO_NOTVICT);
      sprintf(buf,"$n says to you `a'`K%s`a'``",argument);
      act (buf, ch, NULL, victim, TO_VICT);
      sprintf(buf,"You say to $N `a'`K%s`a'``",argument);
      act (buf, ch, NULL, victim, TO_CHAR);
    }
*/
  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
	  cj_self(ch,argument);
  if (ch->race == PC_RACE_CANTHI && !IS_IMMORTAL (ch))
    {
      strcpy(bufn,"gurgles");
       if (!(ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch)))
      act ("You gurgle to $N `a'`K$t`a'``", ch, argument, victim, TO_CHAR);
    }

  else if (ch->race == PC_RACE_VROATH && !IS_IMMORTAL (ch))
    {
      strcpy(bufn,"grunts");
      if (!(ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch)))
      act ("You grunt to $N `a'`K$t`a'``", ch, argument, victim, TO_CHAR);
    }
  else if (ch->race == PC_RACE_NIDAE && !IS_IMMORTAL (ch))
    {
      strcpy(bufn,"sings");
      if (!(ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch)))
      act ("You sing to $N `a'`K$t`a'``", ch, argument, victim, TO_CHAR);
    }
  else
    {
      strcpy(bufn,"says");
      if (!(ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch)))
      act ("You say to $N `a'`K$t`a'``", ch, argument, victim, TO_CHAR);
    }
  sprintf (talktext,"$n %s to %s `a'`K$t`a'``",bufn,((IS_NPC(victim))?victim->short_descr:victim->name));
  sprintf (arg,"$n %s to you `a'`K$t`a'``",bufn);
  if (ch->race == PC_RACE_NIDAE)
    nidaeify(ch,bufn,argument);
  else anti_nidaeify(ch,bufn,argument);
  for (vch = ch->in_room->people;vch != NULL;vch = vch->next_in_room)
    {
      if (ch == vch)
        continue;
      if ((vch->race == PC_RACE_NIDAE && !IS_IMMORTAL(vch))
		      || (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(vch)))
      {
        if (vch == victim)
  	  act (arg, ch, bufn, vch, TO_VICT);
	else 
	{
  	  if (vch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(vch))
	    cj_channel(vch,bufn);
	  else act (talktext, ch, bufn, vch, TO_VICT);
        }
      }
      else 
      {
        if (vch == victim)
          act (arg, ch, argument, vch, TO_VICT);
	else 
	{
	  if (vch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(vch))
  	    cj_channel(vch,argument);
	  else act (talktext, ch, argument, vch, TO_VICT);
	}
      }
    }

  if (!IS_NPC (ch))
    for (vch = ch->in_room->people; vch; vch = vch->next_in_room)
      {
        if (vch == ch || !IS_NPC (vch) || get_position(vch) == POS_SLEEPING ||
	    IS_SET (vch->comm, COMM_DEAF))
          continue;
        strcpy (talktext, argument);
        mob_talk (ch, vch, talktext, 1);
      }
  trip_triggers_arg(ch, OBJ_TRIG_CHAR_VOCALIZE, NULL, NULL, OT_VOC_SAY, argument);
  return;
}

//Iblis - Essentially an inroom tell (though other players in the room can tell who
//you are whispering to, but cannot tell what is said)
void do_whisper (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  char tcbuf[MAX_INPUT_LENGTH], bigbuf[MAX_STRING_LENGTH];
  char message[MAX_INPUT_LENGTH];
  CHAR_DATA *victim, *rch;
  bool last = FALSE;
  strcpy (tcbuf, argument);
  if (IS_SET (ch->comm, COMM_NOTELL) || IS_SET (ch->comm, COMM_DEAF))
    {
      send_to_char ("Your message didn't get through.\n\r", ch);
      return;
    }
  if (IS_SET (ch->comm, COMM_QUIET))
    {
      send_to_char ("You must turn off quiet mode first.\n\r", ch);
      return;
    }
  if (IS_SET (ch->comm, COMM_DEAF))
    {
      send_to_char ("You must turn off deaf mode first.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg);
  if (arg[0] == '\0' || argument[0] == '\0')
    {
      send_to_char ("Tell whom what?\n\r", ch);
      return;
    }
  if (IS_SET(ch->act2,ACT_PUPPET))
    {
      do_switchtalk(ch,argument);
      return;
    }
  if (arg[0] == '[')
    {
      char *cptr, *cptr2;
      if ((cptr = strchr (tcbuf, ']')) == NULL)
        {
          send_to_char ("Didn't understand name list.\n\r", ch);
          return;
        }
      *cptr = '\0';
      strcpy (message, cptr + 1);
      cptr = tcbuf + 1;
      while (1)
        {
          cptr2 = strchr (cptr, ',');
          if (cptr2 == NULL)
            {
              last = TRUE;
              cptr2 = strchr (tcbuf, '\0');
            }
          *cptr2 = '\0';
	  //IBLIS 5/31/03 - Added Drunk Talk to tells
          if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
            drunkify (ch, message);
          sprintf (bigbuf, "%s %s", cptr, message);
          do_whisper (ch, bigbuf);
          if (last)
            return;
          cptr = cptr2 + 1;
        }
    }

  /*
   * Can tell to PC's anywhere, but NPC's only in same room.
   * -- Furey
   */
  if ((victim = get_char_world (ch, arg)) == NULL
      || victim->in_room != ch->in_room)
    {
      FAKE_DATA *fake_vict;
      if ((fake_vict = get_fake (arg)) != NULL)
        {
          char fbuf[MAX_STRING_LENGTH];
          sprintf (fbuf, "You tell %s `a'`L%s`a'``\n\r",
                   fake_vict->name, argument);
          send_to_char (fbuf, ch);
          return;
        }
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (ch->race == PC_RACE_SIDHE && !IS_IMMORTAL (ch))
    {
      act
        ("The fragile structure of your vocal system prevents you from creating anything other than a high-pitched whine.",
         ch, NULL, NULL, TO_CHAR);
      return;
    }
  if (ch->race == PC_RACE_LITAN && !IS_IMMORTAL (ch))
    {
      send_to_char("The odd structures of your vocal cavity make anything other than shrieking impossible.\n\r",ch);
      return;
    }

  if (!(IS_IMMORTAL (ch) && ch->level > LEVEL_IMMORTAL) && !IS_AWAKE (victim))
    {
      act ("$E can't hear you.", ch, 0, victim, TO_CHAR);
      return;
    }
  if (IS_NPC (ch) && !IS_AWAKE (victim))
    {
      act ("$E can't hear you.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if ((IS_SET (victim->comm, COMM_QUIET)
       || IS_SET (victim->comm, COMM_DEAF)) && !IS_IMMORTAL (ch))
    {
      act ("$E is not receiving whispers.", ch, 0, victim, TO_CHAR);
      return;
    }

  if (is_ignoring(victim, ch))
    {
      sprintf(buf, "%s is ignoring you.\n\r", victim->name);
      send_to_char(buf, ch);
      return;
    }

  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
	  cj_self(ch,argument);
  else act ("You whisper to $N `a'`L$t`a'``", ch, argument, victim, TO_CHAR);

  if (ch->race == PC_RACE_NIDAE)
    nidaeify(ch,buf,argument);
  else anti_nidaeify(ch,buf,argument);
 
  if ((victim->race == PC_RACE_NIDAE  && !IS_IMMORTAL(victim))
		  || (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(victim)))
  {
    if (victim->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
      cj_channel(victim,buf);
    else act_new ("$n whispers to you `a'`L$t`a'``", ch, buf, victim, TO_VICT, POS_DEAD);
  }
  else 
  {
    if (victim->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(victim))
      cj_channel(victim,argument);
    else act_new ("$n whispers to you `a'`L$t`a'``", ch, argument, victim, TO_VICT, POS_DEAD);
  }
  sprintf(tcbuf,"%s whispers something to %s.\n\r",(IS_NPC(ch))?ch->short_descr:ch->name,
		  (IS_NPC(victim))?victim->short_descr:victim->name);
  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
  {
    if (rch == victim || rch == ch)
      continue;
    if (rch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(rch))
      send_to_char("The voices whisper something.\n\r",ch);
    else send_to_char(tcbuf,rch);
  }
  victim->reply = ch;
  if (IS_NPC (victim) && !IS_NPC (ch))
    {
      if (get_position(victim) == POS_SLEEPING)
        {
          act ("$N is sleeping, I don't think $E heard you.", ch, NULL,
               victim, TO_CHAR);
          return;
        }
      mob_talk (ch, victim, argument, 0);
    }
  return;
}

//Iblis - Global Pmote channel (basically allows a global pmote on the gemote channel)
void do_gpmote (CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  char *letter, *name;
  char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
  int matches = 0;
      

  if (argument[0] == '\0')
    {
      if (IS_SET (ch->comm2, COMM_NOGEMOTE))
        {
          send_to_char ("Global emotes are now ON.\n\r", ch);
          REMOVE_BIT (ch->comm2, COMM_NOGEMOTE);
        }

      else
        {
          send_to_char ("Global emotes are now OFF.\n\r", ch);
          SET_BIT (ch->comm2, COMM_NOGEMOTE);
        }
      return;
    }

   if ((!IS_NPC (ch) && (IS_SET (ch->comm2, COMM_IMM_NOGEMOTE))) || IS_SET (ch->comm, COMM_NOCHANNELS))
    {
      send_to_char ("You've been banned from globally emoting.\n\r",ch);
      return;
    }
   if (IS_SET (ch->comm2, COMM_RANTONLY))
    {
      send_to_char ("The gods have decreed you may only use the Rant Channel.\n\r", ch);
      return;
    }
   
// Avariel 9/22/12 - Fuck this shit! 
  //if (ch->race == PC_RACE_LITAN && !IS_IMMORTAL (ch))
  //  {
  //    send_to_char("You are not smart enough to know how to emote things, you can only shriek.\n\r"
	//	   ,ch);
  //    return;
  //  }

  act_new ("`n*`g You $t`n *``", ch, argument, NULL, TO_CHAR, POS_SLEEPING);
  if (!IS_SET (ch->comm2, COMM_SHUTUP))
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      CHAR_DATA *victim;
      victim = d->original ? d->original : d->character;
      if (d->connected == CON_PLAYING && d->character != ch
	  && !IS_SET (victim->comm2, COMM_NOGEMOTE)
	  && !IS_SET (victim->comm, COMM_QUIET))
	{
	  if (is_ignoring(victim, ch))
	    continue;
	  if ((letter = strstr (argument, d->character->name)) == NULL)
	    {
	      strcpy (temp,argument);
	      drunkify (ch,temp);
	      act_new ("`n*`g $n $t`n *``", ch, temp, d->character, TO_VICT, POS_SLEEPING);
	      continue;
	    }
	  strcpy (temp, argument);
	  temp[strlen (argument) - strlen (letter)] = '\0';
	  last[0] = '\0';
	  name = d->character->name;
	  for (; *letter != '\0'; letter++)
	    {
	      if (*letter == '\'' && matches == strlen (d->character->name))
		{
		  strcat (temp, "r");
		  continue;
		}
	      if (*letter == 's' && matches == strlen (d->character->name))
		{
		  matches = 0;
		  continue;
		}
	      if (matches == strlen (d->character->name))
		{
		  matches = 0;
		}
	      if (*letter == *name)
		{
		  matches++;
		  name++;
		  if (matches == strlen (d->character->name))
		    {
		      strcat (temp, "you");
		      last[0] = '\0';
		      name = d->character->name;
		      continue;
		    }
		  strncat (last, letter, 1);
		  continue;
		}
	      matches = 0;
	      strcat (temp, last);
	      strncat (temp, letter, 1);
	      last[0] = '\0';
	      name = d->character->name;
	    }
	  //IBLIS 5/31/03 - Added Drunk Emoting
	  if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	    drunkify (ch, temp);
	  act_new ("`n*`g $n $t`n *``", ch,
		   temp, d->character, TO_VICT, POS_SLEEPING);
	}
    }

  return;
}
  
//Iblis - Player suggested global emote channel
void do_gemote (CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d;


  if (argument[0] == '\0')
    {
      if (IS_SET (ch->comm2, COMM_NOGEMOTE))
        {
          send_to_char ("Global emotes are now ON.\n\r", ch);
          REMOVE_BIT (ch->comm2, COMM_NOGEMOTE);
        }

      else
        {
          send_to_char ("Global emotes are now OFF.\n\r", ch);
          SET_BIT (ch->comm2, COMM_NOGEMOTE);
        }
      return;
    }

  if ((!IS_NPC (ch) && (IS_SET (ch->comm2, COMM_IMM_NOGEMOTE))) 
			  || IS_SET (ch->comm, COMM_NOCHANNELS)
			  
		       )
    {
      send_to_char ("You've been banned from globally emoting.\n\r",ch);
      return;
    }

  if (IS_SET (ch->comm2, COMM_RANTONLY))
    {
      send_to_char ("The gods have decreed you may only use the Rant Channel.\n\r", ch);
      return;
    }
  
// Avariel 9/22/12 - Fuck this shit! 
  //if (ch->race == PC_RACE_LITAN && !IS_IMMORTAL (ch))
    //{
   //   send_to_char("You are not smart enough to know how to emote things, you can only shriek.\n\r"
   //                ,ch);
   //   return;
    //}

  //IBLIS 5/31/03 - Added Drunk Emoting
  if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    drunkify (ch, argument);
  act_new ("`n*`g $n $t`n *``", ch, argument, NULL, TO_CHAR, POS_SLEEPING);
  if (!IS_SET (ch->comm2, COMM_SHUTUP))
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      CHAR_DATA *victim;
      victim = d->original ? d->original : d->character;
      if (d->connected == CON_PLAYING && d->character != ch
          && !IS_SET (victim->comm2, COMM_NOGEMOTE)
          && !IS_SET (victim->comm, COMM_QUIET))
        {
          if (is_ignoring(victim, ch))
            continue;
              act_new ("`n*`g $n $t`n *``", ch, argument, d->character, TO_VICT, POS_SLEEPING);
        }
    }

  return;
}

void cj_channel(CHAR_DATA *ch, char* argument)
{
  send_to_char("The voices tell you `a'`L",ch);
  send_to_char(argument,ch);
  send_to_char("`a'``\n\r",ch);
}

void cj_self(CHAR_DATA *ch, char* argument)
{
  send_to_char("You mutter to yourself `a'`L",ch);
  send_to_char(argument,ch);
  send_to_char("`a'``\n\r",ch);
}

void do_switchtalk(CHAR_DATA *ch,char* argument)
{
  if (!IS_NPC(ch))
    return;
  if (ch->pIndexData->vnum == MOB_VNUM_FROG)
  {
    act("The frog ribbits at you.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You ribbit.\n\r",ch);
    return;
  }
  if (ch->pIndexData->vnum == MOB_VNUM_WARPED_CAT)
  {
    act("The warped cat meows about something.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You meow a little.\n\r",ch);
    return;
  }
  if (ch->pIndexData->vnum == MOB_VNUM_WARPED_OWL)
  {
    act("The warped owl hoots its question.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You hoot.\n\r",ch);
    return;
  }
}

  

