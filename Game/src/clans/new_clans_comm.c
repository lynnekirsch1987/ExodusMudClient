//
// new_clans_comm.c
//
// Programmer: Akamai
// Date: 7/10/98
// Version: 1.0
//

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../merc.h"
#include "../olc.h"
#include "new_clans.h"
#include "new_clans_comm.h"
#include "new_clans_io.h"
#include "new_clans_util.h"

//
// some globals used by the rest of the clan code
//
int _clan_ident_ = 1;
int clan_number = 0;
struct clan_type clan_table[MAX_CLAN];

//
// Private functions that implement clan commands
//
void do_clan_imm (CHAR_DATA * ch);
void do_clan_dispatch (CHAR_DATA * ch, char *arg, char *argument);
void clan_who (CHAR_DATA * ch, char *argument);
void clan_list (CHAR_DATA * ch, char *argument);
void clan_help (CHAR_DATA * ch, char *argument);
void clan_loner (CHAR_DATA * ch);
void clan_apply (CHAR_DATA * ch, char *argument);
void clan_defect (CHAR_DATA * ch);
void clan_ally (CHAR_DATA * ch, char *argument);
void clan_enemy (CHAR_DATA * ch, char *argument);
void clan_declare (CHAR_DATA * ch, char *argument);
void clan_last (CHAR_DATA * ch);
void clan_wars (CHAR_DATA * ch);
void clan_info (CHAR_DATA * ch, char *argument);
void clan_accept (CHAR_DATA * ch, char *argument);
void clan_deny (CHAR_DATA * ch, char *argument);
void clan_prospects (CHAR_DATA * ch, char *argument);
void clan_expel (CHAR_DATA * ch, char *argument);
void clan_deputy (CHAR_DATA * ch, char *argument);
void clan_establish (CHAR_DATA * ch, char *argument);
void add_clan (CHAR_DATA * ch, char *argument);
void clan_donate (CHAR_DATA * ch, char *argument);
void clan_disband (CHAR_DATA * ch, char *argument);
void clan_leader (CHAR_DATA * ch, char *argument);
void clan_symbol (CHAR_DATA * ch, char *argument);
void clan_bye (CHAR_DATA * ch);
void clan_short_status (CHAR_DATA * ch, char *argument);
void clan_activate (CHAR_DATA * ch, char *argument);
void clan_list_imm (CHAR_DATA * ch, char *argument);
bool clan_list_status (CHAR_DATA * ch, int status);
void clan_altar (CHAR_DATA * ch, char *argument);
void list_allies (CHAR_DATA * ch, int slot);
void list_enemies (CHAR_DATA * ch, int slot);
void remove_clan_relations (int ident);

void cj_self args((CHAR_DATA *ch, char* argument));

//
// This const simplifies indexing though the different
// clan status values. Clan statuses are not contiguous
// numbers -- so this works as an indirect index table.
//
const int clan_statuses[CLAN_STATUSES] =
  { CLAN_INACTIVE, CLAN_DISBAND, CLAN_DISPERSED, CLAN_HISTORICAL,
  CLAN_PROPOSED, CLAN_RESTRICTED, CLAN_ACTIVE
};

// ==============================================================
//
//   Public Function Entry to clan Code
//
// ==============================================================
//
// This is a big dispatch for most of the clan commands
//
void do_clan (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  argument = one_argument (argument, arg);

  // this might need to change if we allow mob clans
  //act(ch->name,ch,NULL,NULL,TO_ROOM);
  //act(act_bit_name (ch->act),ch,NULL,NULL,TO_ROOM);
//  send_to_char(act_bit_name (ch->act),ch);
  if (IS_NPC (ch))
    {

      //act("HE's a FCUKING NPC!",ch,NULL,NULL,TO_ROOM);
      return;
    }
  if (IS_IMMORTAL (ch) && (arg[0] == '\0'))
    {

//           act("bitch-ass imm",ch,NULL,NULL,TO_ROOM);
      do_clan_imm (ch);
      return;
    }

  else if (arg[0] == '\0')
    {

      //    act("listing clan shit",ch,NULL,NULL,TO_ROOM);
      send_to_char ("Clan Syntax:\n\r", ch);
      send_to_char ("\tclan list\n\r", ch);
      send_to_char ("\tclan loner\n\r", ch);
      send_to_char ("\tclan wars\n\r", ch);
      if (((ch->clan == CLAN_BOGUS) && !is_any_clan_applicant (ch)))
	{
	  send_to_char
	    ("\tclan establish <clan name> <clan symbol> [pk/nopk]\r\n", ch);
	  send_to_char ("\tclan apply <clan name>\n\r", ch);
	}
      if (is_any_clan_applicant (ch) || (ch->clan != CLAN_BOGUS))
	{
	  send_to_char ("\tclan last\n\r", ch);
	  send_to_char ("\tclan defect\n\r", ch);
	  send_to_char ("\tclan donate\n\r", ch);
	}
      if (is_clan_leader (ch) || is_clan_deputy (ch))
	{
	  send_to_char ("\tclan ally [clan name]\n\r", ch);
	  send_to_char ("\tclan enemy [clan name]\n\r", ch);
	  send_to_char ("\tclan declare <clan name>\n\r", ch);
	  send_to_char ("\tclan prospects\n\r", ch);
	  send_to_char ("\tclan accept <player>\n\r", ch);
	  send_to_char ("\tclan deny <player>\r\n", ch);
	  send_to_char ("\tclan expel <player>\n\r", ch);
	}
      if (is_clan_leader (ch))
	{
	  send_to_char ("\tclan deputy <player>\n\r", ch);
	  send_to_char ("\tclan leader <player>\n\r", ch);
	  send_to_char ("\tclan symbol <symbol>\n\r", ch);
	  send_to_char ("\tclan disband\n\r", ch);
	  send_to_char ("\tclan altar\n\r", ch);
	  send_to_char ("\tclan help [clan name]\n\r", ch);
	}
      send_to_char ("\tclan info <clan name>\n\r", ch);
      send_to_char ("\tclan who <clan name>\n\r", ch);
      return;
    }
  do_clan_dispatch (ch, arg, argument);
}

void do_clan_imm (CHAR_DATA * ch)
{
  send_to_char ("Clan Syntax:\n\r", ch);
  send_to_char ("\tclan list [hidden] | [all]\n\r", ch);
  send_to_char ("\tclan loner\n\r", ch);
  send_to_char ("\tclan wars\n\r", ch);
  send_to_char ("\tclan cancel <war number>\n\r", ch);
  send_to_char ("\tclan establish <clan name> <clan symbol>\r\n", ch);
  send_to_char ("\tclan apply <clan name>\n\r", ch);
  send_to_char ("\tclan last\n\r", ch);
  send_to_char ("\tclan defect\n\r", ch);
  send_to_char ("\tclan donate\n\r", ch);
  send_to_char ("\tclan ally [clan name]\n\r", ch);
  send_to_char ("\tclan enemy [clan name]\n\r", ch);
  send_to_char ("\tclan declare <clan name>\n\r", ch);
  send_to_char ("\tclan prospects <clan name>\n\r", ch);
  send_to_char ("\tclan accept <player> <clan name>\n\r", ch);
  send_to_char ("\tclan deny <player>\r\n", ch);
  send_to_char ("\tclan expel <player> <clan name>\n\r", ch);
  send_to_char ("\tclan deputy <player>\n\r", ch);
  send_to_char ("\tclan leader <player> <clan name>\n\r", ch);
  send_to_char ("\tclan symbol <symbol>\n\r", ch);
  send_to_char ("\tclan disband <clan name>\n\r", ch);
  send_to_char ("\tclan altar <clan name>\n\r", ch);
  send_to_char ("\tclan help [clan name]\n\r", ch);

//   send_to_char("\tclan activate <clan name> [leader]\n\r",ch);
  send_to_char ("\tclan activate <clan name>\n\r", ch);
  send_to_char ("\tclan info <clan name>\n\r", ch);
  send_to_char ("\tclan who <clan name>\n\r", ch);
  send_to_char ("Related Commands:\n\r", ch);
  send_to_char ("\tsanctify <clan name>\n\r", ch);
  send_to_char ("\tdesecrate\n\r", ch);
  return;
}
void do_clan_dispatch (CHAR_DATA * ch, char *arg, char *argument)
{

//       act("clan dispatching on yo AZZ!",ch,NULL,NULL,TO_ROOM);
  if (!str_cmp (arg, "who"))
    {
      clan_who (ch, argument);
      return;
    }
  if (!str_cmp (arg, "list"))
    {
      clan_list (ch, argument);
      return;
    }
  if (!str_cmp (arg, "help"))
    {
      clan_help (ch, argument);
      return;
    }
  if (!str_cmp (arg, "info"))
    {
      clan_info (ch, argument);
      return;
    }
  if (!str_cmp (arg, "wars"))
    {
      clan_wars (ch);
      return;
    }
  if (!str_cmp (arg, "ally"))
    {
      clan_ally (ch, argument);
      return;
    }
  if (!str_cmp (arg, "last"))
    {
      clan_last (ch);
      return;
    }
  if (!str_cmp (arg, "deny"))
    {
      clan_deny (ch, argument);
      return;
    }
  if (!str_cmp (arg, "apply"))
    {
      if (ch->race == PC_RACE_AVATAR && ch->pcdata && 
		      (ch->pcdata->avatar_type != 2 && ch->pcdata->avatar_type != 4))
        {
           send_to_char("Sorry, you're a nopk avatar.  If you wish to loner, reincarnate as a pk avatar.\n\r",ch);
           return;				          
	}
      
      clan_apply (ch, argument);
      return;
    }
  if (!str_cmp (arg, "loner"))
    {
      if (ch->race == PC_RACE_AVATAR && ch->pcdata 
		      && (ch->pcdata->avatar_type != 2 && ch->pcdata->avatar_type != 4))
      {
        send_to_char("Sorry, you're a nopk avatar.  If you wish to loner, reincarnate as a pk avatar.\n\r",ch);
        return;
      }
      clan_loner (ch);
      return;
    }
  if (!str_cmp (arg, "enemy"))
    {
      clan_enemy (ch, argument);
      return;
    }
  if (!str_cmp (arg, "expel"))
    {
      clan_expel (ch, argument);
      return;
    }
  if (!str_cmp (arg, "donate"))
    {
      clan_donate (ch, argument);
      return;
    }
  if (!str_cmp (arg, "symbol"))
    {
      clan_symbol (ch, argument);
      return;
    }
  if (!str_cmp (arg, "accept"))
    {
      clan_accept (ch, argument);
      return;
    }
  if (!str_cmp (arg, "defect"))
    {
      clan_defect (ch);
      return;
    }
  if (!str_cmp (arg, "deputy"))
    {
      clan_deputy (ch, argument);
      return;
    }
  if (!str_cmp (arg, "leader"))
    {
      if (!is_clan_leader (ch) && !IS_IMMORTAL (ch))
        {
          send_to_char ("You are not the leader of your clan.\n\r", ch);
          return;
        }
	    
      if (!IS_NPC (ch))
        if ((time (NULL) - ch->pcdata->last_fight < 120) && (!IS_IMMORTAL(ch)))
        {
          send_to_char ("Hold your horses, punk.\n\r", ch);
          return;
        }
      clan_leader (ch, argument);
      return;
    }
  if (!str_cmp (arg, "declare"))
    {
      clan_declare (ch, argument);
      return;
    }
  if (!str_cmp (arg, "disband"))
    {
      clan_disband (ch, argument);
      return;
    }
  if (!str_cmp (arg, "altar"))
    {
      clan_altar (ch, argument);
      return;
    }
  if (!str_cmp (arg, "activate"))
    {
      clan_activate (ch, argument);
      return;
    }
  if (!str_cmp (arg, "prospects"))
    {
      clan_prospects (ch, argument);
      return;
    }
  if (!str_cmp (arg, "establish"))
    {
      if (ch->race == PC_RACE_AVATAR && ch->pcdata && 
		      (ch->pcdata->avatar_type != 2 && ch->pcdata->avatar_type != 4))
      {
        send_to_char("Sorry, you're a nopk avatar.  If you wish to loner, reincarnate as a pk avatar.\n\r",ch);
        return;
      }
      clan_establish (ch, argument);
      return;
    }
  // if we do not recognize any of the commands generate the message
  do_clan (ch, "");
}


//
// Allow a clan member to remove another clan's claim on an area
//
void do_desecrate (CHAR_DATA * ch, char *argument)
{

#ifdef VERBOSE_CLANS
  char buf[MAX_STRING_LENGTH];

#endif /*  */
  OBJ_DATA *obj;
  CHAR_DATA *lch;
  int area_clan;

#ifdef VERBOSE_CLANS
  log_player_command ("s:do_descecrate", ch, argument);

#endif /*  */

#ifdef VERBOSE_CLANS
  sprintf (buf, "desecrate: %s attempts to descecrate area %s",
	   ch->name, ch->in_room->area->name);
  log_string (buf);

#endif /*  */
  for (obj = ch->in_room->contents; obj; obj = obj->next_content)
    if (obj->pIndexData->vnum == OBJ_VNUM_CLAN_SYMBOL)
      break;
  if (obj == NULL)
    {
      send_to_char
	("No symbols declaring this area held exist here.\n\r", ch);
      return;
    }
  if (IS_IMMORTAL (ch))
    {

#ifdef VERBOSE_CLANS
      log_string ("desecrate: by immortal, bypassing checks");

#endif /*  */
    }

  else
    {

#ifdef VERBOSE_CLANS
      log_string ("desecrate: making sure char has a rune");

#endif /*  */
      if (!has_key (ch, OBJ_VNUM_RUNE_CONV))
	{
	  send_to_char
	    ("The desecration of this clan's domination over this area requires the mystical powers of a Rune.\n\r",
	     ch);
	  return;
	}
      lch = char_list;
      area_clan = ch->in_room->area->clan;

#ifdef VERBOSE_CLANS
      log_string ("desecrate: making sure area is clear");

#endif /*  */
      while (lch)
	{
	  if ((area_clan > CLAN_BOGUS) &&	// area is owned by some clan
	      (lch != ch) &&	// char is not same as me
	      (lch->clan == area_clan) &&	// char clan is same as this area clan
	      (lch->in_room->area == ch->in_room->area))
	    {
	      send_to_char
		("You must drive all clan members from this area before you can free it.\n\r",
		 ch);
	      return;
	    }
	  if (!IS_NPC (lch) || !lch->in_room)
	    {
	      lch = lch->next;
	      continue;
	    }
	  if ((lch->in_room->area == ch->in_room->area)
	      && !IS_SET (lch->in_room->room_flags, ROOM_SAFE)
	      && !IS_SET (lch->act, ACT_TRAIN)
	      && !IS_SET (lch->act, ACT_IS_HEALER)
	      && (lch->pIndexData->pShop == NULL))
	    {
	      send_to_char
		("You must first rid this area of all its creatures.\n\r",
		 ch);
	      return;
	    }
	  lch = lch->next;
	}			// end while loop   
    }				// end immortal test
  act
    ("$n traces arcane symbols in the air, desecrating the area in the name of $s clan.",
     ch, NULL, NULL, TO_ROOM);
  send_to_char
    ("You trace arcane symbols in the air, desecrating the area in the name of your clan.\n\r",
     ch);

  // this removes the object that contains the clan symbol from the area.
#ifdef VERBOSE_CLANS
  log_string ("desecrate: removing the symbol and resets");

#endif /*  */
  remove_symbol_resets (ch->in_room->area);
  extract_obj (obj);

  // only take runes from mortal chars
  if (!IS_IMMORTAL (ch))
    {

#ifdef VERBOSE_CLANS
      log_string ("desecrate: taking the rune from the char");

#endif /*  */
      for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	{
	  if (obj->pIndexData->vnum == OBJ_VNUM_RUNE_CONV)
	    {
	      extract_obj (obj);
	      break;
	    }
	}
    }
  // remove the clan number
  ch->in_room->area->clan = CLAN_BOGUS;
  save_area (ch->in_room->area);

#ifdef VERBOSE_CLANS
  log_player_command ("f:do_descecrate", ch, argument);

#endif /*  */
}


//
// Allow a clan member to claim an area for that clan
//
void do_sanctify (CHAR_DATA * ch, char *argument)
{
  char clanstr[MAX_STRING_LENGTH];

#ifdef VERBOSE_CLANS
  char buf[MAX_STRING_LENGTH];

#endif /*  */
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;
  CHAR_DATA *lch;
  RESET_DATA *pReset;
  int slot;
  int area_clan;

#ifdef VERBOSE_CLANS
  log_player_command ("s:do_sanctify", ch, argument);

#endif /*  */

#ifdef VERBOSE_CLANS
  sprintf (buf, "sanctify: %s attempts to sanctify area %s",
	   ch->name, ch->in_room->area->name);
  log_string (buf);

#endif /*  */
  if (ch->in_room->area->noclan)
    {
      send_to_char
	("You may not declare dominion over this area, for it is under the conservancy of the gods.\n\r",
	 ch);
      return;
    }
  if (IS_IMMORTAL (ch))
    {
      argument = one_argument (argument, clanstr);

      // the immortal eiter must be in the clan or supply a clan name
      if (clanstr[0] == '\0')
	{
	  slot = ch->clan;
	}

      else
	{
	  slot = clanname_to_slot (clanstr);
	}

      // validate the slot
      if (slot == CLAN_BOGUS)
	{
	  send_to_char
	    ("You must be in a clan or name a valid clan.\n\r", ch);
	  return;
	}
      // don't set two symbols in this area
      area_clan = clanident_to_slot (ch->in_room->area->clan);
      if (area_clan != CLAN_BOGUS)
	{
	  send_to_char
	    ("This area is already under another clan's dominion.\n\r", ch);
	  return;
	}
#ifdef VERBOSE_CLANS
      log_string ("Sanctify: by immortal, bypassing remaining checks");

#endif /*  */
    }

  else
    {
      slot = (ch->clan);
      if ((slot == CLAN_BOGUS) || (clan_table[slot].status < CLAN_PROPOSED))
	{
	  send_to_char ("You do not belong to a clan.\n\r", ch);
	  return;
	}
      if (!has_key (ch, OBJ_VNUM_RUNE_CONV))
	{
	  send_to_char
	    ("The assertion of your clan's dominance over this area requires the mystical energies of a Rune.\n\r",
	     ch);
	  return;
	}
      area_clan = clanident_to_slot (ch->in_room->area->clan);
      if (area_clan != CLAN_BOGUS)
	{
	  send_to_char
	    ("This area is already under another clan's dominion.\n\r", ch);
	  return;
	}
#ifdef VERBOSE_CLANS
      log_string ("Sanctify: checking that area is empty.");

#endif /*  */
      for (lch = char_list; lch; lch = lch->next)
	{
	  if (lch == NULL)
	    {
	      log_string ("Sanctify: char_list null?");
	      return;
	    }
	  if (!IS_NPC (lch) || !lch->in_room)
	    continue;
	  if ((lch->in_room->area == ch->in_room->area) &&
	      (!IS_SET (lch->in_room->room_flags, ROOM_SAFE)) &&
	      (!IS_SET (lch->in_room->room_flags, ROOM_TRANSPORT)) &&
	      (!IS_SET (lch->act, ACT_TRAIN)) &&
	      (!IS_SET (lch->act, ACT_IS_CHANGER)) &&
	      (!IS_SET (lch->in_room->room_flags, ROOM_BANK)) &&
	      (!IS_SET (lch->act, ACT_IS_HEALER)) &&
	      (lch->pIndexData->pShop == NULL))
	    {
	      send_to_char
		("You must first rid this area of all its creatures.\n\r",
		 ch);
	      return;
	    }
	}			// end for loop
    }				// end immortal test
  if ((pObjIndex = get_obj_index (OBJ_VNUM_CLAN_SYMBOL)) == NULL)
    {
      bug ("Sanctify: no clan symbol object - %d", OBJ_VNUM_CLAN_SYMBOL);
      return;
    }
#ifdef VERBOSE_CLANS
  log_string ("Sanctify: setting area clan slot");

#endif /*  */
  ch->in_room->area->clan = slot;
  obj = create_object (pObjIndex, 0);
  obj_to_room (obj, ch->in_room);

  // this object is the symbol that shows up when a player goes
  // into a clan marked area. This symbol stays in the room until
  // the room is desecrated by another clan. The area is saved later.
  pReset = new_reset_data ();
  pReset->command = 'O';
  pReset->arg1 = OBJ_VNUM_CLAN_SYMBOL;
  pReset->arg2 = 0;
  pReset->arg3 = ch->in_room->vnum;
  pReset->arg4 = -1;

#ifdef VERBOSE_CLANS
  log_string ("Sanctify: adding symbol object and reset to area");

#endif /*  */
  add_reset (ch->in_room, pReset, 0);
  act
    ("$n chants unintelligible words, sanctifying the area in the name of $s clan.",
     ch, NULL, NULL, TO_ROOM);
  send_to_char
    ("You chant unintelligible words, sanctifying the area in the name of your clan.\n\r",
     ch);

  // take the rune from mortal players only
  if (!IS_IMMORTAL (ch))
    {

#ifdef VERBOSE_CLANS
      log_string ("Sanctify: taking rune from character");

#endif /*  */
      for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	{
	  if (obj->pIndexData->vnum == OBJ_VNUM_RUNE_CONV)
	    {
	      extract_obj (obj);
	      break;
	    }
	}
    }
  // don't really save until we have the rune
  save_area (ch->in_room->area);

  // let them know it is finished
  send_to_char
    ("You have declared ascendancy over this area for your clan.\n\r", ch);
  act ("$n has declared ascendancy over this area for $s clan.", ch,
       NULL, NULL, TO_ROOM);

#ifdef VERBOSE_CLANS
  log_player_command ("f:do_sanctify", ch, argument);

#endif /*  */
}


//
// Send a message on a clan war channel
//
void do_clanwar (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int slot;

#ifdef VERBOSE_CLANS
  log_player_command ("s:do_clanwar", ch, argument);

#endif /*  */

  // ok, make sure they are in a clan
  slot = (ch->clan);
  if (slot == CLAN_BOGUS)
    {
      send_to_char ("You are not in a clan.\n\r", ch);
      return;
    }
  // make sure that they are in a mortal visible clan
  if (clan_table[slot].status < CLAN_PROPOSED)
    {
      send_to_char ("You are not in a clan.\n\r", ch);
      return;
    }
  // their clan better be an active clan
  if (clan_table[slot].status < CLAN_ACTIVE)
    {
      send_to_char ("Your clan is not a full clan.\n\r", ch);
      return;
    }
  // what is the text
  if (argument[0] == '\0')
    {
      send_to_char ("What would you like to say to your allies?\n\r", ch);
      return;
    }

  // they better have allies
  if (!clan_has_allies (slot))
    {
      send_to_char ("But, your clan has no allies.\n\r", ch);
      return;
    }
  if (IS_SET (ch->comm, COMM_NOCHANNELS))
    {
      send_to_char ("The gods have revoked your channel privileges.\n\r", ch);
      return;
    }
  if (IS_SET (ch->comm2, COMM_RANTONLY))
  {
	  send_to_char("The gods have decreed you may only use the Rant Channel.\n\r", ch);
	  return;
  }
  if (ch->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch))
	  cj_self(ch,argument);
  else
  {
    sprintf (buf, "[CLANWAR] %s `OYou`a: '`O%s`a'``\n\r",get_clan_symbol(ch->clan), argument);
    send_to_char (buf, ch);
  }

  send_allies_clanwar (ch, argument);

#ifdef VERBOSE_CLANS
  log_player_command ("f:do_clanwar", ch, argument);

#endif /*  */
}


//
// checks the status of any outstanding clan wars and
// if they have gone on long enough declares a winner.
//
void update_wars ()
{
  RELATIONS *us;
  RELATIONS *them;
  char us_buf[MAX_STRING_LENGTH];
  char tm_buf[MAX_STRING_LENGTH];
  time_t now;
  int slot, ident;
  int enemy_slot;
  bool waswar = FALSE;
  bool warring = FALSE;

#ifdef VERBOSE_CLANS
  log_string ("s:update_wars");

#endif /*  */
  now = time (NULL);

  // check to see if *any* clan war has ended
  for (slot = CLANS_START; slot < clan_number; slot++)
    {
      if (clan_table[slot].status < CLAN_ACTIVE)
	continue;
      us = clan_table[slot].relations;
      ident = clan_table[slot].ident;
      while (us)
	{

	  // if the war has lasted long enough then we need to declare a winner
	  if ((us->status >= CLAN_WAR) &&
	      ((now - (us->wartime)) >= CLAN_WAR_TIME))
	    {

	      // do not be fooled (us->ident) is really *their* identifier
	      enemy_slot = clanident_to_slot (us->ident);

	      // there was a war on when we started this routine
	      waswar = TRUE;

	      // if this clan/clanwar is one we have not yet processed
	      if (enemy_slot > slot)
		{

		  // look up their relationship with us
		  them =
		    find_relation (clan_table[enemy_slot].relations, ident);

		  // now declare a winner
		  if ((us->points) > (them->points))
		    {

		      // we *win* - they lost
		      if (us->status == CLAN_WAR_AGGR)
			{

			  // we were the agressor
			  sprintf (us_buf,
				   "Your clans aggressive diplomacy proved useful.\n\rYour clan wins the war as clan %s`` sues for peace.\n\r",
				   clan_table[enemy_slot].name);
			  sprintf (tm_buf,
				   "Alas, your clan has been beaten by the aggressive clan %s``.\n\rYour clan sues for peace.\n\r",
				   clan_table[slot].name);
			}

		      else
			{

			  // they were the agressor
			  sprintf (us_buf,
				   "Your clan proved victorious.\n\rClan %s`` sues for peace with your clan.\n\r",
				   clan_table[enemy_slot].name);
			  sprintf (tm_buf,
				   "Your clan's aggressive diplomacy proved disastrous.\n\rYour clan sues for peace with clan %s``.\n\r",
				   clan_table[slot].name);
			}

		      // give credit
		      // DO NOT reset kills or points
		      // they are needed by has_defeated_clan() function
		      (us->wins)++;
		      (them->loss)++;

		      // mark the end time of the war
		      us->wartime = now;
		      them->wartime = now;

		      // set them up as enemies
		      us->status = CLAN_ENEMY;
		      them->status = CLAN_ENEMY;
		      us->acttime = now;
		      them->acttime = now;
		    }

		  else if ((us->points) < (them->points))
		    {

		      // we *lost* - they win
		      if (us->status == CLAN_WAR_AGGR)
			{

			  // we were the agressor
			  sprintf (us_buf,
				   "Your clans aggressive diplomacy proved disastrous.\n\rYour clan sues for peace with clan %s``.\n\r",
				   clan_table[enemy_slot].name);
			  sprintf (tm_buf,
				   "Your clan proved victorious.\n\rClan %s`` sues for peace with your clan.\n\r",
				   clan_table[slot].name);
			}

		      else
			{

			  // they were the agressor
			  sprintf (us_buf,
				   "Alas, your clan has been beaten by the aggressive clan %s.``\n\rYour clan sues for peace.\n\r",
				   clan_table[enemy_slot].name);
			  sprintf (tm_buf,
				   "Your clans aggressive diplomacy proved useful.\n\rYour clan wins the war as clan %s`` sues for peace.\n\r",
				   clan_table[slot].name);
			}

		      // give credit
		      // DO NOT reset kills or points
		      // they are needed by has_defeated_clan() function
		      (us->loss)++;
		      (them->wins)++;

		      // mark the end time of the war
		      us->wartime = now;
		      them->wartime = now;

		      // set them up as enemies
		      us->status = CLAN_ENEMY;
		      them->status = CLAN_ENEMY;
		      us->acttime = now;
		      them->acttime = now;
		    }

		  else
		    {

		      // the fight was a stalemate
		      sprintf (us_buf,
			       "A tense armistice ensues between your clan and clan %s``.\n\r",
			       clan_table[enemy_slot].name);

		      // the fight was a stalemate
		      sprintf (tm_buf,
			       "A tense armistice ensues between your clan and clan %s``.\n\r",
			       clan_table[slot].name);

		      // mark the end time of the war
		      us->wartime = now;
		      them->wartime = now;

		      // set them up as enemies
		      us->status = CLAN_ENEMY;
		      them->status = CLAN_ENEMY;

		      // they can go to war very quickly, only 3/4 of the normal time
		      us->acttime =
			now - (CLAN_ENEMY_TIME - (CLAN_ENEMY_TIME / 4));
		      them->acttime =
			now - (CLAN_ENEMY_TIME - (CLAN_ENEMY_TIME / 4));
		    }

		  // save us and them
		  save_clan (slot);
		  save_clan (enemy_slot);

		  // notify the appropriate clans
		  send_clan_members (slot, us_buf);
		  send_clan_members (enemy_slot, tm_buf);
		}		// enemy_slot > slot
	    }			// end clan war test
	  us = us->next;
	}			// end the while( us )
    }

  // if there was a war and there are no more wars
  // declare a type of peace across the lands
  if (waswar)
    {
      slot = CLANS_START;
      while ((slot < clan_number) && !warring)
	{
	  warring = clan_is_warring (slot);
	  slot++;
	}
      if (!warring)
	{
	  send_all_players (NULL,
			    "\n\rA peace spreads across the realms...\n\r\n\r");
	}
    }
#ifdef VERBOSE_CLANS
  log_string ("f:update_wars");

#endif /*  */
}


//
// Allow a player with a rune, who defeats a clan leader to dispers a clan
//
void do_disperse (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  char item[MAX_INPUT_LENGTH];
  int clan;
  int vclan;
  OBJ_DATA *obj;
  MEMBER *m;
  CHAR_DATA *c;

#ifdef VERBOSE_CLANS
  log_player_command ("s:do_disperse", ch, argument);

#endif /*  */
  clan = clanname_to_slot (argument);
  if (clan == CLAN_BOGUS)
    {
      sprintf (buf, "There is no clan named %s``.\n\r",
	       capitalize (argument));
      send_to_char (buf, ch);
      return;
    }
  if (!has_key (ch, OBJ_VNUM_RUNE_CONV))
    {
      send_to_char
	("The mystical energies of a Rune are required for this.\n\r", ch);
      return;
    }
  vclan = ch->clan;
  if (vclan == CLAN_BOGUS)
    {
      send_to_char
	("You must be in a clan to disperse another clan.\n\r", ch);
      return;
    }
  if (!is_clan_leader (ch) && !IS_IMMORTAL (ch))
    {
      send_to_char ("You must be a clan leader to disperse.\n\r", ch);
      return;
    }
  // they can not still be at war
  if (is_warring (vclan, clan))
    {
      sprintf (buf, "Your clan is still at war with clan %s``.\n\r",
	       clan_table[clan].name);
      send_to_char (buf, ch);
      send_to_char ("And the tides of war can change quickly.\n\r", ch);
      return;
    }
  // the clan of the person doing the disperse must be victorious
  // over the clan being dispersed
  if (!has_defeated_clan (get_clan_ident (vclan), get_clan_ident (clan)))
    {
      sprintf (buf, "Your clan has not recently defeated clan %s``.\n\r",
	       clan_table[clan].name);
      send_to_char (buf, ch);
      return;
    }
  // the person must be standing over the dead corpse of the
  // other clans clan leader
  sprintf (item, "corpse of %s", clan_table[clan].leader->name);

#ifdef VERBOSE_CLANS
  sprintf (buf, "do_disperse: need item - \"%s\"", item);
  log_string (buf);

#endif /*  */
  obj = ch->in_room->contents;
  while (obj != NULL)
    {

#ifdef VERBOSE_CLANS
      sprintf (buf, "do_disperse: room item - \"%s\"", obj->name);
      log_string (buf);

#endif /*  */
      if (is_name (item, obj->name))
	break;
      obj = obj->next_content;
    }
  if (obj == NULL)
    {
      send_to_char ("You need the corpse of the clan leader.\n\r", ch);
      return;
    }
  // take the rune from the character
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if (obj->pIndexData->vnum == OBJ_VNUM_RUNE_CONV)
	{
	  extract_obj (obj);
	  break;
	}
    }
  sprintf (buf,
	   "You call upon the power of the Rune to disperse clan %s``.\n\r",
	   clan_table[clan].name);
  send_to_char (buf, ch);
  sprintf (buf,
	   "$n calls upon the power of the Rune to disperse clan %s``.",
	   clan_table[clan].name);
  act (buf, ch, NULL, NULL, TO_ROOM);

  // update the clans status... now it's dispersed
  clan_table[clan].status = CLAN_DISPERSED;

  // remove every other clan's relations with us
  remove_clan_relations (clan_table[clan].ident);

  // for now a disperse does not remove any clan symbols
  // remove_clan_symbols();
  save_clan (clan);

  // notify the membership that they were dispersed
  sprintf (buf, "Your clan has been dispersed by %s.\n\r", ch->name);
  send_clan_members (clan, buf);

  // update the members to think they are not in the clan
  m = clan_table[clan].members;
  while (m)
    {
      c = find_character (m->name);
      if (c)
	c->clan = CLAN_BOGUS;
      m = m->next;
    }

#ifdef VERBOSE_CLANS
  log_player_command ("f:do_disperse", ch, argument);

#endif /*  */
}


//
// this use to rebuild the clan table, not the same now
//
void do_reclan (CHAR_DATA * ch, char *argument)
{
  send_to_char ("reclan is not necessary in the new_clan model.\n\r", ch);
}

// ==============================================================
//
//   Private Functions implement Clans
//
// ==============================================================

//
// List off who is in the clan
//
void clan_who (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  MEMBER *m;
  int slot = 0;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_who", ch, argument);

#endif /*  */
  argument = one_argument (argument, arg);

  // trap for an empty clan
  if (arg[0] == '\0')
    {
      send_to_char ("Syntax: clan who <clan name>\n\r", ch);
      return;
    }
  // look up the clan by the clan name
  slot = clanname_to_slot (arg);
  if (slot == CLAN_BOGUS)
    {
      sprintf (buf, "There is no clan named %s``.\n\r", capitalize (arg));
      send_to_char (buf, ch);
      return;
    }
  // check that the clan is a visible status
  if ((clan_table[slot].status < CLAN_PROPOSED) && !IS_IMMORTAL (ch))
    {
      sprintf (buf, "There is no clan named %s``.\n\r", capitalize (arg));
      send_to_char (buf, ch);
      return;
    }
  // for immortals list the real clan status
  if (IS_IMMORTAL (ch))
    clan_short_status (ch, arg);

  // list off the members, list the leader first
  send_to_char ("Clan Members:\n\r", ch);
  sprintf (buf, "\t%-12s (Leader)\n\r", clan_table[slot].leader->name);
  send_to_char (buf, ch);
  m = clan_table[slot].members;
  while (m)
    {
      if (m != clan_table[slot].leader)
	{
	  if (m->status >= CLAN_DEPUTY)
	    {
	      sprintf (buf, "\t%-12s (Deputy)\n\r", m->name);
	    }

	  else
	    {
	      sprintf (buf, "\t%-12s\n\r", m->name);
	    }
	  send_to_char (buf, ch);
	}
      m = m->next;
    }

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_who", ch, argument);

#endif /*  */
}


//
// List all the current and proposed clans
//
void clan_list (CHAR_DATA * ch, char *argument)
{
  int i = 0;
  bool found = FALSE;
  char buf[MAX_INPUT_LENGTH];

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_list", ch, argument);

#endif /*  */
  if (IS_IMMORTAL (ch) && (argument[0] != '\0'))
    {
      clan_list_imm (ch, argument);
      return;
    }
  // set up the table
  if (CLANS_START < clan_number)
    {
      if (IS_IMMORTAL (ch))
	{
	  sprintf (buf,
		   "Name         Symbol       Leader       PK/NonPK    Status       Levels Slot\n\r");
	}

      else
	{
	  sprintf (buf,
		   "Name         Symbol       Leader       PK/NonPK    Status\n\r");
	}
      send_to_char (buf, ch);
    }

  else
    {
      send_to_char ("None.\n\r", ch);
      return;
    }
  for (i = HIDDEN_STATUSES; i < CLAN_STATUSES; i++)
    {
      if (clan_list_status (ch, clan_statuses[i]))
	found = TRUE;
    }
  if (!found)
    send_to_char ("None.\n\r", ch);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_list", ch, argument);

#endif /*  */
}


//
// Edit the clan help message
//
void clan_help (CHAR_DATA * ch, char *argument)
{
  int slot = 0;
  char name[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_help", ch, argument);

#endif /*  */
  argument = one_argument (argument, name);
  if (IS_IMMORTAL (ch))
    {
      if (name[0] == '\0')
	{
	  send_to_char ("Please be more specific, which clan?\n\r", ch);
	  return;
	}
      slot = clanname_to_slot (name);
      if (slot == CLAN_BOGUS)
	{
	  sprintf (buf, "There is no clan named %s``.\n\r",
		   capitalize (name));
	  send_to_char (buf, ch);
	  return;
	}
      // check that the clan is a visible status
      if (clan_table[slot].status < CLAN_PROPOSED)
	{
	  sprintf (buf, "Clan %s is currently in an inactive state.\n\r",
		   capitalize (name));
	  send_to_char (buf, ch);
	  return;
	}
    }

  else
    {
      slot = ch->clan;
      if (slot == CLAN_BOGUS)
	{
	  send_to_char ("You are not currently a clan member.\n\r", ch);
	  return;
	}
      // check that the clan is a visible status
      if (clan_table[slot].status < CLAN_PROPOSED)
	{
	  send_to_char ("No such clan exists.\n\r", ch);
	  return;
	}
      if (!is_clan_leader (ch))
	{
	  send_to_char ("You do not lead your clan.\n\r", ch);
	  return;
	}
    }

  // if they make it this far, then they get to edit the clan help
  // reset the help name
  if (ch->desc->help_name != NULL)
    free_string (ch->desc->help_name);
  ch->desc->help_name = str_dup (clan_table[slot].name);

  // reset the help description
  if (ch->desc->help_info != NULL)
    free_string (ch->desc->help_info);
  ch->desc->help_info =
    read_current_help (clan_table[slot].name, "clans.hlp");

  // now allow them to edit
  send_to_char
    ("Please use the Exodus Editor to enter your clan's help information.\n\r\n\r",
     ch);
  ch->desc->editor = SAVE_CLAN_HELP;
  edit_string (ch, &ch->desc->help_info);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_help", ch, argument);

#endif /*  */
}


//
// Allow a player to mark himself a loner
//
void clan_loner (CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];
  int slot;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_loner", ch, ((char *) NULL));

#endif /*  */

  // Check for level requirement added by Morgan on June 29. 2000
  if (ch->level < 9)
    {
      send_to_char ("You are too young to become a loner.\n\r", ch);
      return;
    }
  slot = (ch->clan);

  // if their clan is not a bogus clan then they are in one already
  if ((slot != CLAN_BOGUS) && (clan_table[slot].status >= CLAN_PROPOSED))
    {
      sprintf (buf,
	       "You are already in clan %s``, you may become a loner by defecting.\n\r",
	       clan_table[slot].name);
      send_to_char (buf, ch);
      return;
    }
  // if they are an applicant, then they are almost in the clan
  if (is_clan_applicant (ch, slot)
      && (clan_table[slot].status >= CLAN_PROPOSED))
    {
      sprintf (buf,
	       "You have already applied to clan %s``, you must defect in order to become a loner.\n\r",
	       clan_table[slot].name);
      send_to_char (buf, ch);
      return;
    }
  if (ch->pcdata->loner == TRUE)
    {
      send_to_char ("You have already been marked as a loner.\n\r", ch);
      ch->clan = CLAN_BOGUS;
      ch->pcdata->deputy = FALSE;
      return;
    }
  send_to_char ("You are now marked as a loner.\n\r", ch);
  ch->pcdata->loner = TRUE;
  ch->pcdata->deputy = FALSE;
  ch->clan = CLAN_BOGUS;

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_loner", ch, ((char *) NULL));

#endif /*  */
}

//
// Allow a player to apply for clan membership
//
void clan_apply (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *leader;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int slot, oldslot;
  MEMBER *m;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_apply", ch, argument);

#endif /*  */
  argument = one_argument (argument, arg);

  // do they have the right syntax
  if (arg[0] == '\0')
    {
      send_to_char ("Apply to which clan?\n\r", ch);
      return;
    }
  // Check for level requirement added by Morgan on June 29. 2000
  if (ch->level < 9)
    {
      send_to_char ("You are too young to apply to a clan.\n\r", ch);
      return;
    }
  // did they give a valid clan name
  slot = clanname_to_slot (arg);
  if (slot == CLAN_BOGUS)
    {
      sprintf (buf, "There is no clan named %s``.\n\r", capitalize (arg));
      send_to_char (buf, ch);
      return;
    }
  // check that the clan is a visible status
  if (clan_table[slot].status < CLAN_PROPOSED)
    {
      sprintf (buf, "There is no clan named %s``.\n\r", capitalize (arg));
      send_to_char (buf, ch);
      return;
    }
  // check that the clan is a visible status
  if (ch->level < MIN_MEMBER_LEVEL)
    {
      sprintf (buf, "You must be at least level %d to join a clan.\n\r",
	       MIN_MEMBER_LEVEL);
      send_to_char (buf, ch);
      return;
    }
  if (ch->pcdata->loner && clan_table[slot].ctype == CLAN_TYPE_EXPL)
    {
      send_to_char ("Loners can't join nopk clans.\n\r", ch);
      return;
    }
  // are they already in a clan
  oldslot = (ch->clan);
  if ((oldslot != CLAN_BOGUS)
      && (clan_table[oldslot].status >= CLAN_PROPOSED))
    {
      sprintf (buf,
	       "You are already in clan %s``, you must defect in order to apply.\n\r",
	       clan_table[oldslot].name);
      send_to_char (buf, ch);
      return;
    }
  // check that they are not applying to another clan too
  oldslot = is_any_clan_applicant (ch);
  if ((oldslot != CLAN_BOGUS)
      && (clan_table[oldslot].status >= CLAN_PROPOSED))
    {
      sprintf (buf,
	       "You applied to clan %s``, you must defect to apply to another clan.\n\r",
	       clan_table[oldslot].name);
      send_to_char (buf, ch);
      return;
    }
  // a just in case fix of the player record
  ch->clan = CLAN_BOGUS;
  ch->pcdata->deputy = FALSE;
  save_char_obj (ch);

  // allocated their entry into the applicant list
  m = new_member_elt ();
  strcpy ((m->name), (ch->name));
  m->status = CLAN_APPLIED;
  m->levels = ch->level;
  m->align = ch->alignment;
  m->race = ch->race;
  m->Class = ch->Class;
  m->initiative = CLAN_BOGUS;
  clan_table[slot].applicants =
    append_member (clan_table[slot].applicants, m);

  // now save the clan
  save_clan (slot);

  // notify player
  sprintf (buf, "You have applied for membership to clan %s``.\n\r",
	   clan_table[slot].name);
  send_to_char (buf, ch);

  // attempt notify the leader
  leader = find_character (clan_table[slot].leader->name);
  if (leader != NULL)
    {
      sprintf (buf, "%s has applied for membership into clan %s``.\n\r",
	       ch->name, clan_table[slot].name);
      send_to_char (buf, leader);
    }

  else
    {

      // This is where we should notify the leader by a note
    }

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_apply", ch, argument);

#endif /*  */
}


//
//  Allow a PC to remove themselves from a clan
//
void clan_defect (CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];
  MEMBER *m;
  bool fixup = FALSE;
  int slot;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_defect", ch, ((char *) NULL));

#endif /*  */

  // can't defect while fighting
  if (ch->position == POS_FIGHTING)
    {
      send_to_char ("No way! You are fighting.\n\r", ch);
      return;
    }
  // check the amount of fight lag
  if ((time (NULL) - ch->pcdata->last_fight) < FIGHT_LAG)
    {
      send_to_char ("Nice try.  You can't defect so easily.\n\r", ch);
      return;
    }
  /*if (ch->clan != CLAN_BOGUS)
  {
    if ((clan_table[ch->clan].relations)->status >= CLAN_WAR)
    {
      send_to_char ("You may not defect during a war.  That is LAME.\n\r",ch);
      return;
    }
  }*/
  
  // can't defect if they are the clan leader
  if (is_clan_leader (ch))
    {
      send_to_char
	("Clan leaders may not defect, you must disband your clan.\n\r", ch);
      return;
    }
  // save the clan slot number
  slot = (ch->clan);

  // send the PC a message if they were in a bogus clan
  if (slot == CLAN_BOGUS)
    {
      slot = is_any_clan_applicant (ch);
      if (slot == CLAN_BOGUS)
	{
	  send_to_char
	    ("You are not currently in or applying to any clan.\n\r", ch);
	  return;
	}
    }
  // set the PC clan flags as appropriate and save them
  ch->clan = CLAN_BOGUS;
  ch->pcdata->deputy = FALSE;
  save_char_obj (ch);

  // send the PC a message if they were in a defunct clan
  if (clan_table[slot].status < CLAN_PROPOSED)
    {
      fixup = TRUE;
      send_to_char
	("You are not currently in or applying to any clan.\n\r", ch);

      // do not return in this case, because what we really want to do is remove
      // them from that clans membership list just below.
      //return;
    }
  // find the member record and remove it from the linked list
  m = find_member (clan_table[slot].members, ch->name);
  if (m == NULL)
    {
      m = find_member (clan_table[slot].applicants, ch->name);
      if (m == NULL)
	{
	  if (!fixup)
	    send_to_char
	      ("You are not currently in or applying to any clan.\n\r", ch);
	  return;
	}

      else
	{
	  clan_table[slot].applicants =
	    delete_member (clan_table[slot].applicants, ch->name);
	  if (!fixup)
	    {
	      sprintf (buf,
		       "You withdraw your application to clan %s``.\n\r",
		       clan_table[slot].name);
	      send_to_char (buf, ch);
	    }
	  save_clan (slot);

#ifdef VERBOSE_CLANS
	  log_player_command ("f:clan_defect", ch, "*just an applicant*");

#endif /*  */
	  return;
	}
    }

  else
    {
      clan_table[slot].members =
	delete_member (clan_table[slot].members, ch->name);
    }
  if (fixup)
    {

#ifdef VERBOSE_CLANS
      log_player_command ("f:clan_defect", ch, "*fixup defunct clan*");

#endif /*  */
      return;
    }
  // Now notify the membership that the player has defected
  sprintf (buf, "%s has defected from clan %s``.\n\r", ch->name,
	   clan_table[slot].name);
  send_clan_members (slot, buf);

  // m is not null, that means we found the player
  // this clan now loses all the levels of the PC, effective as of the
  // time they joined the clan. This clan may now have a status change
  clan_table[slot].levels -= m->levels;
  if ((clan_table[slot].levels < FULL_CLAN_LEVELS))
    {

      // if this clan has lost too many levels then we restrict 
      // it like a proposed clan
      clan_table[slot].status = CLAN_RESTRICTED;
      sprintf (buf,
	       "Clan %s`` has lost too many members and is now restricted.\n\r",
	       clan_table[slot].name);
      send_clan_members (slot, buf);
    }
  save_clan (slot);

  // notify the player of the action
  sprintf (buf, "You defect from clan %s``.\n\r", clan_table[slot].name);
  send_to_char (buf, ch);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_defect", ch, ((char *) NULL));

#endif /*  */
}

//
//  Allow a clan leader to ally this clan with another clan
//
void clan_ally (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char wbuf[MAX_STRING_LENGTH];
  RELATIONS *ally = NULL;
  int slot, allies, levels;
  int ally_slot;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_ally", ch, argument);

#endif /*  */

  // validate clan identifier
  slot = (ch->clan);
  if (slot == CLAN_BOGUS)
    {
      send_to_char ("You are not in a clan.\n\r", ch);
      return;
    }
  // check that their clan is not dispersed or disbanded
  if (clan_table[slot].status < CLAN_PROPOSED)
    {
      send_to_char ("You are not in a clan.\n\r", ch);
      return;
    }
  // check clan leadership
  if (!is_clan_leader (ch) && !is_clan_deputy (ch) && !IS_IMMORTAL (ch))
    {
      send_to_char ("You do not lead your clan.\n\r", ch);
      return;
    }
  // check that their clan is not restricted
  if (clan_table[slot].status < CLAN_ACTIVE)
    {
      send_to_char ("Your clan is not currently a full clan.\n\r", ch);
      return;
    }
  // if they gave us no argument then list off the allies of this clan
  if (argument[0] == '\0')
    {
      list_allies (ch, slot);
      return;
    }
  // lookup this clan name
  ally_slot = clanname_to_slot (argument);
  if (ally_slot == CLAN_BOGUS)
    {
      sprintf (buf, "There is no clan named %s``.\n\r",
	       capitalize (argument));
      send_to_char (buf, ch);
      return;
    }
  // validate that it is a mortal visible clan
  if (clan_table[ally_slot].status < CLAN_PROPOSED)
    {
      sprintf (buf, "There is no clan named %s``.\n\r",
	       capitalize (argument));
      send_to_char (buf, ch);
      return;
    }
  // do not let them do something dumb
  if (ally_slot == slot)
    {
      send_to_char ("You can not mark your own clan as an ally.\n\r", ch);
      return;
    }
  // check that this clan is active
  if (clan_table[ally_slot].status < CLAN_ACTIVE)
    {
      send_to_char
	("You may not make allies of proposed or restricted clans.\n\r", ch);
      return;
    }
  // first if they are an ally then this works as a toggle and unsets
  // the ally flag. First find the record of the relations between
  // the players clan and the proposed ally clan.
  ally =
    find_relation (clan_table[slot].relations, clan_table[ally_slot].ident);

  // if there is no ally entry then we are neutral to them
  // allocate a relation record and set it to neutral, fix up below
  if (ally == NULL)
    {
      ally = new_relations_elt ();
      strcpy ((ally->name), clan_table[ally_slot].name);
      ally->ident = clan_table[ally_slot].ident;
      ally->status = CLAN_NEUTRAL;

      // add them to the relations list
      clan_table[slot].relations =
	append_relation (clan_table[slot].relations, ally);
    }
  // ok, only worry about the issue that they are trying to add a new ally
  if (ally->status != CLAN_ALLY)
    {

      // make sure that this clan does not exceed a hard value
      allies = count_clan_allies (slot);
      if (allies >= CLAN_ALLIES_MAX)
	{
	  send_to_char
	    ("Your clan cannot manage any more alliances.\n\r", ch);
	  return;
	}
      // validate that this clan has enough levels to add an ally,
      // every clan will be able to have at least one ally
      levels = clan_levels (slot);
      if (levels < (allies * LEVELS_PER_ALLY))
	{
	  send_to_char
	    ("Your clan is not powerful enough to add another ally.\n\r", ch);
	  return;
	}
    }
  // now given the current ally status shift to another status
  switch ((ally->status))
    {
    case CLAN_ALLY:
      ally->status = CLAN_NEUTRAL;
      ally->acttime = time (NULL);
      save_clan (slot);
      break;
    case CLAN_NEUTRAL:
      ally->status = CLAN_ALLY;
      ally->acttime = time (NULL);
      save_clan (slot);
      break;
    case CLAN_ENEMY:
      sprintf (buf,
	       "Clan %s`` can not be marked both an ally and an enemy.\n\r",
	       clan_table[ally_slot].name);
      send_to_char (buf, ch);
      return;
      break;
    case CLAN_WAR:
    case CLAN_WAR_AGGR:
    case CLAN_WAR_VICT:
      sprintf (buf, "But you are currently at war with %s``!\n\r",
	       clan_table[ally_slot].name);
      send_to_char (buf, ch);
      return;
      break;
    default:
      ally->status = CLAN_ALLY;
      ally->acttime = time (NULL);
      sprintf (buf,
	       "clan_ally() unrecognized clan status '%d' - clan '%s' tried to ally clan '%s'",
	       (ally->status), clan_table[slot].name,
	       clan_table[ally_slot].name);
      bug (buf, 0);
      save_clan (slot);
      break;
    }

  // set up a string for message notification
  if (is_ally (ally_slot, slot))
    {
      sprintf (buf,
	       "%s secures an alliance between clan %s`` and clan %s``.\n\r",
	       NAME (ch), clan_table[slot].name, clan_table[ally_slot].name);

      // notify allies if we are improving the relation ship
      send_clan_members (ally_slot, buf);
    }

  else if (is_requested_ally (slot, ally_slot))
    {
      sprintf (buf,
	       "%s petitions for an alliance between clan %s`` and clan %s``.\n\r",
	       NAME (ch), clan_table[slot].name, clan_table[ally_slot].name);

      // notify the other clan if we are attempting to improve the relationship
      send_clan_members (ally_slot, buf);
    }

  else
    {
      sprintf (buf,
	       "%s has worsened relations between clan %s`` and clan %s``.\n\r",
	       NAME (ch), clan_table[slot].name, clan_table[ally_slot].name);
    }

  // notify the clan membership
  send_clan_members_ch (ch, buf);

  // notify the wiznet subscribers of the player action
  sprintf (wbuf, "--> %s", buf);
  wiznet (wbuf, NULL, NULL, WIZ_CLAN, 0, 0);
  if (is_ally (ally_slot, slot))
    {
      sprintf (buf, "You secure an alliance with clan %s``.\n\r",
	       clan_table[ally_slot].name);
    }

  else if (is_requested_ally (slot, ally_slot))
    {
      sprintf (buf,
	       "You petition for clan %s`` to ally with your clan.\n\r",
	       clan_table[ally_slot].name);
    }

  else
    {				// neutral is all that is left
      sprintf (buf,
	       "You worsen your clans relations with clan %s``.\n\r",
	       clan_table[ally_slot].name);
    }
  send_to_char (buf, ch);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_ally", ch, argument);

#endif /*  */
}


//
// Allow a clan leader to make a clan an enemy
//
void clan_enemy (CHAR_DATA * ch, char *argument)
{
  char wbuf[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  RELATIONS *enemy = NULL;
  int slot, enemies, levels;
  int enemy_slot;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_enemy", ch, argument);

#endif /*  */
  slot = (ch->clan);
  if (slot == CLAN_BOGUS)
    {
      send_to_char ("You are not in a clan.\n\r", ch);
      return;
    }
  // validate that it is a mortal visible clan
  if (clan_table[slot].status < CLAN_PROPOSED)
    {
      send_to_char ("You are not in a clan.\n\r", ch);
      return;
    }
  if (!is_clan_leader (ch) && !is_clan_deputy (ch) && !IS_IMMORTAL (ch))
    {
      send_to_char
	("You must lead your clan in order to mark clan enemies.\n\r", ch);
      return;
    }
  // check that their clan is not restricted
  if (clan_table[slot].status < CLAN_ACTIVE)
    {
      send_to_char ("Your clan is not currently a full clan.\n\r", ch);
      return;
    }
  // if they gave us no clan name then list our enemies
  if (argument[0] == '\0')
    {
      list_enemies (ch, slot);
      return;
    }
  enemy_slot = clanname_to_slot (argument);
  if (enemy_slot == CLAN_BOGUS)
    {
      sprintf (buf, "There is no clan named %s.\n\r", capitalize (argument));
      send_to_char (buf, ch);
      return;
    }
  // validate that it is a mortal visible clan
  if (clan_table[enemy_slot].status < CLAN_PROPOSED)
    {
      sprintf (buf, "There is no clan named %s.\n\r", capitalize (argument));
      send_to_char (buf, ch);
      return;
    }
  if (enemy_slot == slot)
    {
      send_to_char ("You can not mark your own clan as an enemy.\n\r", ch);
      return;
    }
  if (is_requested_ally (slot, enemy_slot))
    {
      sprintf (buf,
	       "Clan %s`` may not be marked as both an ally and an enemy.\n\r",
	       clan_table[enemy_slot].name);
      send_to_char (buf, ch);
      return;
    }
  if (clan_table[enemy_slot].status < CLAN_ACTIVE)
    {
      send_to_char
	("You may not make enemies out of proposed or restricted clans.\n\r",
	 ch);
      return;
    }
  if (is_warring (slot, enemy_slot))
    {
      sprintf (buf, "You are already warring with clan %s.\n\r",
	       clan_table[enemy_slot].name);
      send_to_char (buf, ch);
      return;
    }
  // first if they are an enemy then this works
  // as a toggle and unsets the enemy status.
  enemy =
    find_relation (clan_table[slot].relations, clan_table[enemy_slot].ident);

  // if there is no enemy entry then we are neutral to them
  // allocate a relation record and set it to neutral, fix up below
  if (enemy == NULL)
    {
      enemy = new_relations_elt ();
      strcpy ((enemy->name), clan_table[enemy_slot].name);
      enemy->ident = clan_table[enemy_slot].ident;
      enemy->status = CLAN_NEUTRAL;

      // add them to the relations list
      clan_table[slot].relations =
	append_relation (clan_table[slot].relations, enemy);
    }
  // Check that they are not trying to add a new enemy
  if (enemy->status != CLAN_ENEMY)
    {

      // make sure that this clan does not exceed a hard value
      enemies = count_clan_enemies (slot);
      if (enemies >= CLAN_ENEMIES_MAX)
	{
	  send_to_char ("Your clan cannot manage any more enemies.\n\r", ch);
	  return;
	}
      // validate that this clan has enough levels to add an enemy,
      // every clan will be able to have at least one enemy
      levels = clan_levels (slot);
      if (levels < (enemies * LEVELS_PER_ENEMY))
	{
	  send_to_char
	    ("Your clan is not powerful enough to declare another enemy.\n\r",
	     ch);
	  return;
	}
    }
  // now given the current ally status shift to another status
  switch ((enemy->status))
    {
    case CLAN_ALLY:
      sprintf (buf,
	       "Clan %s`` can not be marked both an ally and an enemy.\n\r",
	       clan_table[enemy_slot].name);
      send_to_char (buf, ch);
      return;
      break;
    case CLAN_NEUTRAL:
      enemy->status = CLAN_ENEMY;
      enemy->acttime = time (NULL);
      save_clan (slot);
      break;
    case CLAN_ENEMY:
      enemy->status = CLAN_NEUTRAL;
      enemy->acttime = time (NULL);
      save_clan (slot);
      break;
    case CLAN_WAR:
    case CLAN_WAR_AGGR:
    case CLAN_WAR_VICT:
      sprintf (buf, "But you are currently at war with %s!\n\r",
	       clan_table[enemy_slot].name);
      send_to_char (buf, ch);
      return;
      break;
    default:
      enemy->status = CLAN_ENEMY;
      enemy->acttime = time (NULL);
      sprintf (buf,
	       "clan_enemy() unrecognized clan status %d - clan %s tried to enemy clan %s",
	       (enemy->status), clan_table[slot].name,
	       clan_table[enemy_slot].name);
      bug (buf, 0);
      save_clan (slot);
      break;
    }

  // set up a string for message notification
  if (is_enemy (enemy_slot, slot))
    {
      sprintf (buf, "%s declares clan %s`` an enemy of clan %s``.\n\r",
	       NAME (ch), clan_table[slot].name, clan_table[enemy_slot].name);
    }

  else if (is_requested_enemy (slot, enemy_slot))
    {
      sprintf (buf,
	       "%s worsens relations between clan %s`` and clan %s``.\n\r",
	       NAME (ch), clan_table[slot].name, clan_table[enemy_slot].name);
    }

  else
    {
      sprintf (buf,
	       "%s makes overtures to improve relations between clan %s`` and clan %s``.\n\r",
	       NAME (ch), clan_table[slot].name, clan_table[enemy_slot].name);

      // only notify this clan membership if they are attempting
      // to improve the relationship
      send_clan_members (enemy_slot, buf);
    }

  // notify the clan membership
  send_clan_members_ch (ch, buf);

  // notify the wiznet subscribers of the player action
  sprintf (wbuf, "--> %s", buf);
  wiznet (wbuf, NULL, NULL, WIZ_CLAN, 0, 0);
  if (is_enemy (enemy_slot, slot))
    {
      sprintf (buf, "You have made your clan an enemy of clan %s``.\n\r",
	       clan_table[enemy_slot].name);
    }

  else if (is_requested_enemy (slot, enemy_slot))
    {
      sprintf (buf,
	       "You worsen your clans relations with clan %s``.\n\r",
	       clan_table[enemy_slot].name);
    }

  else
    {				// neutral is all that is left
      sprintf (buf,
	       "You make overtures to improve relations with clan %s``.\n\r",
	       clan_table[enemy_slot].name);
    }
  send_to_char (buf, ch);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_enemy", ch, argument);

#endif /*  */
}


//
// Allow a declaration of war
//
void clan_declare (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  RELATIONS *us;
  RELATIONS *them;
  int slot, enemy_slot;
  time_t now;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_declare", ch, argument);

#endif /*  */

  // validate syntax
  if (argument[0] == '\0')
    {
      send_to_char ("Which clan would you like to declare war with?\n\r", ch);
      return;
    }
  // check that this player is in a  clan
  slot = (ch->clan);
  if (slot == CLAN_BOGUS)
    {
      send_to_char ("You must be in a clan to start a war.\n\r", ch);
      return;
    }
  // make sure its still a valid clan
  if (clan_table[slot].status < CLAN_PROPOSED)
    {
      send_to_char ("You must be in a clan to start a war.\n\r", ch);
      return;
    }
  // check that they are the leader of this clan
  if (!is_clan_leader (ch) && !IS_IMMORTAL (ch))
    {
      send_to_char
	("You must be the leader of your clan to start a war.\n\r", ch);
      return;
    }
  // check that their clan is not restricted
  if (clan_table[slot].status < CLAN_ACTIVE)
    {
      send_to_char ("Your clan is not currently a full clan.\n\r", ch);
      return;
    }
  // check that the clan they named is a valid clan
  enemy_slot = clanname_to_slot (argument);
  if (enemy_slot == CLAN_BOGUS)
    {
      sprintf (buf, "There is no clan by the name of %s``.\n\r", argument);
      send_to_char (buf, ch);
      return;
    }
  // check that the clan they named is a valid clan
  if (clan_table[enemy_slot].status < CLAN_PROPOSED)
    {
      sprintf (buf, "There is no clan by the name of %s``.\n\r", argument);
      send_to_char (buf, ch);
      return;
    }
  // check that the clan they named is an active clan
  if (clan_table[enemy_slot].status < CLAN_ACTIVE)
    {
      send_to_char
	("You cannot declare war on proposed or restricted clans.\n\r", ch);
      return;
    }

  if (enemy_slot == clanname_to_slot ("guttersnipes"))
  {
	  send_to_char("You're just a big meanie now.\n\r",ch);
	  return;
  }
  // check that the clan is a PK clan
  if (clan_table[slot].ctype == CLAN_TYPE_EXPL)
    {
      sprintf (buf, "That would make your clan a PK clan``.\n\r");
      send_to_char (buf, ch);
      return;
    }
  // check that the clan they named is a PK clan
  if (clan_table[enemy_slot].ctype == CLAN_TYPE_EXPL)
    {
      sprintf (buf, "You can't declare war against %s``.\n\r", argument);
      send_to_char (buf, ch);
      return;
    }
  // do not let them do something dumb
  if (enemy_slot == slot)
    {
      send_to_char ("You can not start a war with your own clan.\n\r", ch);
      return;
    }
  // make sure we have some kind of relation with them
  them =
    find_relation (clan_table[slot].relations, clan_table[enemy_slot].ident);
  if (them == NULL)
    {
      sprintf (buf,
	       "Your clan does not have any current relationship with clan %s``.\n\r",
	       clan_table[enemy_slot].name);
      send_to_char (buf, ch);
      return;
    }
  // make sure they have some kind of relation with us
  us = find_relation (clan_table[enemy_slot].relations,
		      clan_table[slot].ident);
  if (us == NULL)
    {
      us = new_relations_elt ();
      strcpy ((us->name), clan_table[slot].name);
      us->ident = clan_table[slot].ident;
      us->status = CLAN_NEUTRAL;
      clan_table[enemy_slot].relations =
	append_relation (clan_table[enemy_slot].relations, us);
    }
  // can't go to war twice
  if (is_warring (slot, enemy_slot))
    {
      sprintf (buf, "You are already warring with %s``.\n\r",
	       clan_table[enemy_slot].name);
      send_to_char (buf, ch);
      return;
    }
  // ok, can't move directly from ally to war
  if (is_requested_ally (slot, enemy_slot))
    {
      sprintf (buf,
	       "You can not war with %s`` as they are marked an ally.\n\r",
	       clan_table[enemy_slot].name);
      send_to_char (buf, ch);
      return;
    }
  // they better be marked as an enemy
  if (!is_requested_enemy (slot, enemy_slot))
    {
      sprintf (buf, "You must first mark clan %s`` as an enemy.\n\r",
	       clan_table[enemy_slot].name);
      send_to_char (buf, ch);
      return;
    }
  // check that the time of the last action happend long enough in the past
  now = time (NULL);
  if ((now - (them->acttime)) < CLAN_ENEMY_TIME)
    {
      sprintf (buf,
	       "Clan %s`` has not been your enemy long enough to start a war.\n\r",
	       clan_table[enemy_slot].name);
      send_to_char (buf, ch);
      return;
    }
  // in this case, us is their relations with us
  (us->warcount)++;
  us->status = CLAN_WAR_VICT;
  us->wartime = now;
  us->kills = 0;
  us->points = 0;

  // and them is our relations with them
  (them->warcount)++;
  them->status = CLAN_WAR_AGGR;
  them->wartime = now;
  them->kills = 0;
  them->points = 0;

  // better save both clans
  save_clan (enemy_slot);
  save_clan (slot);

  // notify all of the players of a war
  sprintf (buf, "War breaks out and rages across the realms...\n\r");
  send_all_players (ch, buf);

  // notify the wiznet subscribers of the player action
  sprintf (buf, "--> Clan %s`` declares war on clan %s``.\n\r",
	   clan_table[slot].name, clan_table[enemy_slot].name);
  wiznet (buf, NULL, NULL, WIZ_CLAN, 0, 0);

  // notify the members of this clan that their leader did the deed
  sprintf (buf,
	   "\n\rYour clan has declared war on clan %s``!\n\rMay the innocent be safe.\n\r",
	   clan_table[enemy_slot].name);
  send_clan_members_ch (ch, buf);

  // notify the individual
  sprintf (buf,
	   "\n\rYou have declared war on clan %s``!\n\rMay the innocent be safe.\n\r",
	   clan_table[enemy_slot].name);
  send_to_char (buf, ch);

  // let the members of the other clan know that things have gone sour
  sprintf (buf,
	   "\n\rClan %s`` has declared war on your clan!\n\rMay this act of treachery be avenged.\n\r",
	   clan_table[slot].name);
  send_clan_members (enemy_slot, buf);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_declare", ch, argument);

#endif /*  */
}


//
//  Simply list off the last wins or losses
//
void clan_last (CHAR_DATA * ch)
{
  RELATIONS *them;
  RELATIONS *us;
  char buf[MAX_STRING_LENGTH];
  char logbuf[MAX_STRING_LENGTH];
  time_t now;
  int ident;
  int slot;
  int enemy_slot;
  bool found = FALSE;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_last", ch, ((char *) NULL));

#endif /*  */

  // if this is an immortal checking, do a quick check of the existing wars
  if (IS_IMMORTAL (ch))
    update_wars ();
  now = time (NULL);
  send_to_char ("Recent clan wars:\n\r", ch);

  // run through all of the known clans
  for (slot = CLANS_START; slot < clan_number; slot++)
    {
      if (clan_table[slot].status < CLAN_ACTIVE)
	{

#ifdef VERBOSE_CLANS
	  sprintf (logbuf, "clan_last: skipping %s [%d] not active",
		   clan_table[slot].name, slot);
	  log_string (logbuf);

#endif /*  */
	  continue;
	}
      us = clan_table[slot].relations;
      ident = clan_table[slot].ident;
      while (us)
	{

	  // do not be fooled (us->ident) is really *their* identifier
	  enemy_slot = clanident_to_slot (us->ident);

	  // if the war was recent enough to count and they are not still at war
#ifdef VERBOSE_CLANS
	  sprintf (logbuf, "clan_last: %s [%d] relation with %s [%d]",
		   clan_table[slot].name, slot,
		   clan_table[enemy_slot].name, enemy_slot);
	  log_string (logbuf);
	  sprintf (logbuf, "clan_last: status: %d recent war: %ld < %d",
		   us->status, (now - (us->wartime)), RECENT_WAR_TIME);
	  log_string (logbuf);

#endif /*  */
	  if (((now - (us->wartime)) < RECENT_WAR_TIME) &&
	      (us->status < CLAN_WAR))
	    {

	      // if this clan is one we have not yet processed
	      if (enemy_slot > slot)
		{

		  // look up their relationship with us
		  them =
		    find_relation (clan_table[enemy_slot].relations, ident);
		  if (them != NULL)
		    {

		      // They better have a relation with us
		      if ((us->points) > (them->points))
			{
			  sprintf (buf,
				   "Clan %s`` (%d points in %d kill%s) defeated clan %s`` (%d points in %d kill%s).\n\r",
				   clan_table[slot].name, (us->points),
				   (us->kills),
				   (us->kills) == 1 ? "" : "s",
				   clan_table[enemy_slot].name,
				   (them->points), (them->kills),
				   (them->kills) == 1 ? "" : "s");
			}

		      else if ((us->points) < (them->points))
			{
			  sprintf (buf,
				   "Clan %s`` (%d points in %d kill%s) defeated clan %s`` (%d points in %d kill%s).\n\r",
				   clan_table[enemy_slot].name,
				   (them->points), (them->kills),
				   (them->kills) == 1 ? "" : "s",
				   clan_table[slot].name, (us->points),
				   (us->kills), (us->kills) == 1 ? "" : "s");
			}

		      else
			{
			  sprintf (buf,
				   "Clan %s`` fought to a stalemate with clan %s``.\n\r",
				   clan_table[slot].name,
				   clan_table[enemy_slot].name);
			}
		      send_to_char (buf, ch);
		      found = TRUE;
		    }

		  else
		    {

		      // uh, oh - the them relation is screwed
		      sprintf (logbuf,
			       "clan_last: %s [%d] relation with %s [%d]",
			       clan_table[slot].name, slot,
			       clan_table[enemy_slot].name, enemy_slot);
		      bug (logbuf, 0);
		      bug ("clan_last: them relation is NULL", 0);
		      sprintf (logbuf,
			       "clan_last: now: %ld us->wartime: %ld status: %d",
			       now, us->wartime, us->status);
		      bug (logbuf, 0);
		    }
		}
	    }
	  us = us->next;
	}
    }
  if (!found)
    send_to_char ("None.\n\r", ch);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_last", ch, ((char *) NULL));

#endif /*  */
}

//
// List off the active clan wars
//
void clan_wars (CHAR_DATA * ch)
{
  char logbuf[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  RELATIONS *us;
  RELATIONS *them;
  int slot;
  int enemy_slot;
  int ident;
  time_t timeleft;
  long hr, min, sec;
  bool found = FALSE;
  int war_number;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_wars", ch, ((char *) NULL));

#endif /*  */

  // if this is an immortal checking, do a quick check of the existing wars
  if (IS_IMMORTAL (ch))
    update_wars ();
  send_to_char ("Current clan wars: \n\r", ch);

  // run through all of the known clans
  for (slot = CLANS_START; slot < clan_number; slot++)
    {
      if (clan_table[slot].status < CLAN_ACTIVE)
	continue;
      us = clan_table[slot].relations;
      ident = clan_table[slot].ident;
      while (us)
	{

	  // if we are in a clan war
	  if ((us->status >= CLAN_WAR))
	    {

	      // do not be fooled (us->ident) is really *their* identifier
	      enemy_slot = clanident_to_slot (us->ident);

	      // if this clan is one we have not yet processed
	      if (enemy_slot > slot)
		{
		  them =
		    find_relation (clan_table[enemy_slot].relations, ident);
		  if (them != NULL)
		    {
		      war_number = (slot * 100) + enemy_slot;
		      sprintf (buf,
			       "Clan %s %d points (%d kill%s) vs. Clan %s %d points (%d kill%s)",
			       clan_table[slot].name, us->points,
			       us->kills, us->kills == 1 ? "" : "s",
			       clan_table[enemy_slot].name,
			       them->points, them->kills,
			       them->kills == 1 ? "" : "s");
		      if (IS_IMMORTAL (ch))
			{
			  timeleft =
			    ((us->wartime + CLAN_WAR_TIME) - time (NULL));
			  if (timeleft < 0)
			    timeleft = 0;
			  min = timeleft / 60;
			  sec = timeleft % 60;
			  hr = min / 60;
			  min = min % 60;
			  sprintf (buf + strlen (buf),
				   " (%ld:%ld:%ld) (War Number: %d) \n\r",
				   hr, min, sec, war_number);
			}

		      else
			{
			  sprintf (buf + strlen (buf), "\n\r");
			}
		      send_to_char (buf, ch);
		      found = TRUE;
		    }

		  else
		    {

		      // uh, oh - the them relationship is null
		      sprintf (logbuf,
			       "clan_wars: %s [%d] relation with %s [%d]",
			       clan_table[slot].name, slot,
			       clan_table[enemy_slot].name, enemy_slot);
		      bug (logbuf, 0);
		      bug ("clan_wars: them relation is NULL", 0);
		    }
		}
	    }
	  us = us->next;
	}
    }
  if (!found)
    send_to_char ("\tNone.\n\r", ch);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_wars", ch, ((char *) NULL));

#endif /*  */
}

//
//  List off a bunch of simple things about this clan
//
void clan_info (CHAR_DATA * ch, char *argument)
{
  MEMBER *m;
  RELATIONS *r;
  int twins = 0;
  int tloss = 0;
  int slot;
  int cnt;
  int members = 0;
  bool found = FALSE;
  char outbuf[MAX_STRING_LENGTH * 4];

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_info", ch, argument);

#endif /*  */

  // validate syntax
  if (argument[0] == '\0')
    {
      send_to_char ("Which clan would you like information about?\n\r", ch);
      return;
    }
  // validate the slot
  slot = clanname_to_slot (argument);
  if (slot == CLAN_BOGUS)
    {
      sprintf (outbuf, "There is no clan named %s``.\n\r",
	       capitalize (argument));
      send_to_char (outbuf, ch);
      return;
    }
  // validate clan status
  if ((clan_table[slot].status < CLAN_PROPOSED) && !IS_IMMORTAL (ch))
    {
      sprintf (outbuf, "There is no clan named %s``.\n\r",
	       capitalize (argument));
      send_to_char (outbuf, ch);
      return;
    }
  // the basics
  sprintf (outbuf, "Clan Name: %s``\n\rClan Symbol: %s``\n\r",
	   clan_table[slot].name, clan_table[slot].symbol);

  // count the number of members
  m = clan_table[slot].members;
  while (m)
    {
      members++;
      m = m->next;
    }
  sprintf (outbuf + strlen (outbuf), "Clan Members: %d\n\r", members);

  // run through their relation list and get the total wins and losses
  r = clan_table[slot].relations;
  twins = 0;
  tloss = 0;
  while (r)
    {
      twins += (r->wins);
      tloss += (r->loss);
      r = r->next;
    }
  sprintf (outbuf + strlen (outbuf),
	   "Clan War Wins: %d\tClan War Losses: %d\n\r", twins, tloss);
  page_to_char (outbuf, ch);

  // further information is reserved for clan members and immortals
  if ((!IS_IMMORTAL (ch)) && (slot != ch->clan))
    {

#ifdef VERBOSE_CLANS
      log_player_command ("f:clan_info", ch, "not a member exit");

#endif /*  */
      return;
    }
  if (IS_IMMORTAL (ch))
    clan_short_status (ch, argument);

  // list off the allies
  list_allies (ch, slot);

  // list off the enemies of this clan
  list_enemies (ch, slot);
  sprintf (outbuf, "Recently defeated by clan %s``:\n\r",
	   clan_table[slot].name);
  found = FALSE;
  r = clan_table[slot].relations;
  cnt = 0;
  while (r)
    {
      if (has_defeated_clan (get_clan_ident (slot), r->ident))
	{
	  sprintf (outbuf + strlen (outbuf), "%-12s``", r->name);
	  if (cnt % 3 == 0)
	    strcat (outbuf, "\n\r");
	  cnt++;
	  found = TRUE;
	}
      r = r->next;
    }
  if (!found)
    strcat (outbuf, "None.");
  strcat (outbuf, "\n\r");
  page_to_char (outbuf, ch);
  sprintf (outbuf, "Recent losses by clan %s``:\n\r", clan_table[slot].name);
  found = FALSE;
  r = clan_table[slot].relations;
  cnt = 0;
  while (r)
    {
      if (has_defeated_clan (r->ident, get_clan_ident (slot)))
	{
	  sprintf (outbuf + strlen (outbuf), "%-12s``", r->name);
	  if (cnt % 3 == 0)
	    strcat (outbuf, "\n\r");
	  cnt++;
	  found = TRUE;
	}
      r = r->next;
    }
  if (!found)
    strcat (outbuf, "None.");
  strcat (outbuf, "\n\r");
  page_to_char (outbuf, ch);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_info", ch, argument);

#endif /*  */
}


//
// Allow a deputy or leader to accept another player into the clan
//
void clan_accept (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char clanstr[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *vic;
  MEMBER *newm;
  MEMBER *oldm;
  int slot;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_accept", ch, argument);

#endif /*  */
  argument = one_argument (argument, arg);

  // check the syntax
  if (arg[0] == '\0')
    {
      send_to_char ("Accept which player into your clan?\n\r", ch);
      return;
    }
  if (IS_IMMORTAL (ch))
    {
      argument = one_argument (argument, clanstr);

      // the immortal eiter must be in the clan or supply a clan name
      if (clanstr[0] == '\0')
	{
	  slot = ch->clan;
	}

      else
	{
	  slot = clanname_to_slot (clanstr);
	}

      // validate the slot
      if (slot == CLAN_BOGUS)
	{
	  send_to_char
	    ("You must be in a clan or name a valid clan.\n\r", ch);
	  return;
	}
    }

  else
    {

      // check that we have a valid clan
      slot = ch->clan;
      if (slot == CLAN_BOGUS)
	{
	  send_to_char ("You are not in a clan.\n\r", ch);
	  return;
	}
      // offically if the clan is some status less than propsed then
      // it does not exist to players
      if (clan_table[slot].status < CLAN_PROPOSED)
	{
	  send_to_char ("You are not in a clan.\n\r", ch);
	  return;
	}
      // they need to be a leader or deputy
      if (!is_clan_leader (ch) && !is_clan_deputy (ch))
	{
	  send_to_char ("You do not lead a clan.\n\r", ch);
	  return;
	}
    }

  // look up this name in the applicant list
  oldm = find_member (clan_table[slot].applicants, arg);
  if (oldm == NULL)
    {
      sprintf (buf, "No player %s has applied to your clan.\n\r",
	       capitalize (arg));
      send_to_char (buf, ch);
      return;
    }
  vic = find_character (arg);
  if (vic == NULL)
    {
      send_to_char ("That character is not online.\n\r", ch);
      return;
    }
  if (vic->level < 9)
    {
      send_to_char ("That character is not level 9 or more.\n\r", ch);
      return;
    }
  // copy from the old member rec to the new one
  // unfortunate side effect of the way I coded the
  // the member list insert, delete and find functions
  newm = new_member_elt ();
  strcpy (newm->name, oldm->name);
  newm->levels = oldm->levels;
  newm->align = oldm->align;
  newm->status = CLAN_ACCEPTED;
  newm->initiative = CLAN_BOGUS;

  // if player is on line, update the victim's player record
  // notify the victim that they are now part of the clan
  vic = find_character (arg);
  if (vic != NULL)
    {
      if (clan_table[slot].ctype == CLAN_TYPE_EXPL)
	vic->pcdata->loner = FALSE;

      else
	vic->pcdata->loner = TRUE;
      vic->pcdata->deputy = FALSE;
      vic->clan = slot;
      save_char_obj (vic);
      sprintf (buf, "You have been accepted into clan %s``.\n\r",
	       clan_table[slot].name);
      send_to_char (buf, vic);
    }
  // notify membership
  sprintf (buf, "%s has been accepted into clan %s``.\n\r",
	   capitalize (arg), clan_table[slot].name);
  send_clan_members_ch (ch, buf);

  // finally delete the oldm record
  clan_table[slot].applicants =
    delete_member (clan_table[slot].applicants, arg);
  oldm = NULL;

  // append the new member record
  clan_table[slot].members = append_member (clan_table[slot].members, newm);

  // now check if the addition of this member changes the clan status
  clan_table[slot].levels += newm->levels;
/*  if (//((clan_table[slot].status == CLAN_PROPOSED) ||
       (clan_table[slot].status == CLAN_RESTRICTED)//)
      && clan_table[slot].levels >= FULL_CLAN_LEVELS)
    {
      clan_table[slot].status = CLAN_ACTIVE;
      sprintf (buf, "Clan %s`` has been promoted to a full clan.\n\r",
	       clan_table[slot].name);
      send_clan_members (slot, buf);
    }*/
  // save all of these updates
  save_clan (slot);
  sprintf (buf, "You accepted %s into clan %s``.\n\r", capitalize (arg),
	   clan_table[slot].name);
  send_to_char (buf, ch);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_accept", ch, argument);

#endif /*  */
}


//
// Allow a deputy or a leader to deny another player membership
//
void clan_deny (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int clan;
  MEMBER *applicant;
  CHAR_DATA *vic;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_deny", ch, argument);

#endif /*  */
  argument = one_argument (argument, arg);

  // validate the syntax
  if (arg[0] == '\0')
    {
      send_to_char ("Deny which player membership to your clan?\n\r", ch);
      return;
    }
  // check that we have a valid clan
  clan = ch->clan;
  if (clan == CLAN_BOGUS)
    {
      send_to_char ("You are not in a clan.\n\r", ch);
      return;
    }
  // offically if the clan is some status less than propsed then
  // it does not exist to players
  if (clan_table[clan].status < CLAN_PROPOSED)
    {
      send_to_char ("You are not in a clan.\n\r", ch);
      return;
    }
  // they need to be a leader or deputy
  if (!is_clan_leader (ch) && !is_clan_deputy (ch))
    {
      send_to_char ("You do not lead a clan.\n\r", ch);
      return;
    }
  applicant = find_member (clan_table[clan].applicants, arg);
  if (applicant == NULL)
    {
      sprintf (buf, "No one named %s has applied to your clan.\n\r", arg);
      send_to_char (buf, ch);
      return;
    }
  // this is safe because if the applicant wasn't in the
  // list we wouldn't fall through to this section
  // wow, way shorter than it used to be, only two lines
  clan_table[clan].applicants =
    delete_member (clan_table[clan].applicants, arg);
  save_clan (clan);

  // they do not have to be online, but if they are they should be notified
  vic = find_character (arg);
  if (vic != NULL)
    {
      sprintf (buf, "You have been denied entry to clan %s``.\n\r",
	       clan_table[clan].name);
      send_to_char (buf, vic);
    }
  // let the person know their action was taken
  sprintf (buf, "You denied %s entry into clan %s``.\n\r",
	   capitalize (arg), clan_table[clan].name);
  send_to_char (buf, ch);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_deny", ch, argument);

#endif /*  */
}


//
//  Allow a deputy or a clan leader to check the applicants
//
void clan_prospects (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char name[MAX_STRING_LENGTH];
  MEMBER *a;
  int slot;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_prospects", ch, argument);

#endif /*  */
  if (IS_IMMORTAL (ch))
    {
      argument = one_argument (argument, name);
      if (name[0] == '\0')
	{
	  send_to_char ("You need to specify a clan.\n\r", ch);
	  return;
	}
      slot = clanname_to_slot (name);
      if (slot == CLAN_BOGUS)
	{
	  sprintf (buf, "There is no clan named %s``.\n\r",
		   capitalize (name));
	  send_to_char (buf, ch);
	  return;
	}
      if (clan_table[slot].status < CLAN_PROPOSED)
	{
	  send_to_char
	    ("This clan is currently in an inactive state.\n\r", ch);
	}
    }

  else
    {
      slot = ch->clan;
      if (slot == CLAN_BOGUS)
	{
	  send_to_char ("You are not in a clan.\n\r", ch);
	  return;
	}
      if (clan_table[slot].status < CLAN_PROPOSED)
	{
	  send_to_char ("Your clan no longer exists.\n\r", ch);
	  return;
	}
      if (!is_clan_leader (ch) && !is_clan_deputy (ch))
	{
	  send_to_char ("You do not lead your clan.\n\r", ch);
	  return;
	}
    }

  // check that there are some applicants
  a = clan_table[slot].applicants;
  if (a == NULL)
    {
      send_to_char ("No players are applying to your clan.\n\r", ch);
      return;
    }
  send_to_char ("Players currently applying to your clan:\n\r", ch);
  while (a)
    {
      sprintf (buf, "\t%-12s [%3d] [%5d]\n\r", a->name, a->levels, a->align);
      send_to_char (buf, ch);
      a = a->next;
    }

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_prospects", ch, argument);

#endif /*  */
}


//
// Allow a deputy or a clan leader to expel a clan member
//
void clan_expel (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char clanstr[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *vic;
  MEMBER *oldm;
  int slot;
  int levels;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_expel", ch, argument);

#endif /*  */
  argument = one_argument (argument, arg);

  // check the syntax
  if (arg[0] == '\0')
    {
      send_to_char ("Expel which player from your clan?\n\r", ch);
      return;
    }
  /*if (ch->clan != CLAN_BOGUS)
  {
   if ((clan_table[ch->clan].relations)->status >= CLAN_WAR)
     {
       send_to_char ("You may not expel during a war.  That is LAME.\n\r",ch);
       return;
     }
  }*/
   
  if (IS_IMMORTAL (ch))
    {
      argument = one_argument (argument, clanstr);

      // the immortal eiter must be in the clan or supply a clan name
      if (clanstr[0] == '\0')
	{
	  slot = ch->clan;
	}

      else
	{
	  slot = clanname_to_slot (clanstr);
	}

      // validate the slot
      if (slot == CLAN_BOGUS)
	{
	  send_to_char
	    ("You must be in a clan or name a valid clan.\n\r", ch);
	  return;
	}
    }

  else
    {

      // validate the clan
      slot = ch->clan;
      if (slot == CLAN_BOGUS)
	{
	  send_to_char ("You are not in a clan.\n\r", ch);
	  return;
	}
      // make sure it's active
      if (clan_table[slot].status < CLAN_PROPOSED)
	{
	  send_to_char ("That clan does not exist.\n\r", ch);
	  return;
	}
      // validate that they are an appropriate type player
      if (!is_clan_leader (ch) && !is_clan_deputy (ch))
	{
	  send_to_char ("You do not lead a clan.\n\r", ch);
	  return;
	}
    }

  // if the names are the same, can't expel yourself
  if (!str_cmp (ch->name, arg))
    {
      send_to_char
	("You can not expel yourself, try defecting instead.\n\r", ch);
      return;
    }
  // Make sure there is someone named that
  oldm = find_member (clan_table[slot].members, arg);
  if (oldm == NULL)
    {
      sprintf (buf, "%s is not in your clan.\n\r", arg);
      send_to_char (buf, ch);
      return;
    }
  // ok, do not throw out the leader....
  if ((oldm->status == CLAN_LEADER))
    {
      send_to_char ("You can not expel the clan leader.\n\r", ch);
      return;
    }
  // special privilidges for a deputy, immortals and leaders only expel
  if (!IS_IMMORTAL (ch) &&
      (oldm->status >= CLAN_DEPUTY) && !is_clan_leader (ch))
    {
      send_to_char ("Only clan leaders may expel deputies.\n\r", ch);
      return;
    }
  // save the levels of this character
  levels = oldm->levels;

  // if the player is on line then fix their record and tell them
  vic = find_character (arg);
  if (vic != NULL)
    {
      if (clan_table[vic->clan].ctype == CLAN_TYPE_EXPL)
	vic->pcdata->loner = FALSE;

      else
	vic->pcdata->loner = TRUE;
      vic->clan = CLAN_BOGUS;
      vic->pcdata->deputy = FALSE;
      save_char_obj (vic);
      sprintf (buf, "You have been expelled from clan %s``.\n\r",
	       clan_table[ch->clan].name);
      send_to_char (buf, vic);
    }
  // Do the deletion
  clan_table[slot].members = delete_member (clan_table[slot].members, arg);
  save_clan (slot);

  // Notify the clan
  sprintf (buf, "%s has been expelled from clan %s`` by %s.\n\r",
	   capitalize (arg), clan_table[slot].name, ch->name);
  send_clan_members_ch (ch, buf);

  // subtract off the number of levels that this player
  // contributed to the establishment of the clan
  clan_table[slot].levels -= levels;
  if (clan_table[slot].levels < FULL_CLAN_LEVELS)
    {
      clan_table[slot].status = CLAN_RESTRICTED;
      sprintf (buf,
	       "Clan %s`` has lost too many levels and is now restricted.\n\r",
	       clan_table[slot].name);
      send_clan_members (slot, buf);
      save_clan (slot);
    }
  // Notify the player
  sprintf (buf, "You expelled %s from clan %s``.\n\r",
	   capitalize (arg), clan_table[slot].name);
  send_to_char (buf, ch);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_expel", ch, argument);

#endif /*  */
}


//
// Allow the clan leader to promote/demote a player to deputy
//
void clan_deputy (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *vic;
  MEMBER *m;
  int clan;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_deputy", ch, argument);

#endif /*  */
  one_argument (argument, arg);

  // validate the syntax
  if (arg[0] == '\0')
    {
      send_to_char ("Promote which player to deputy?\n\r", ch);
      return;
    }
  // validate the clan slot
  clan = ch->clan;
  if (clan == CLAN_BOGUS)
    {
      send_to_char ("You are not in a clan.\n\r", ch);
      return;
    }
  // if their clan has been dispersed, disbanded or deactivated
  if ((clan_table[clan].status < CLAN_PROPOSED))
    {
      send_to_char ("You are not in a clan.\n\r", ch);
      return;
    }
  // they better be leader to make a deputy
  if (!is_clan_leader (ch))
    {
      send_to_char ("You do not lead your clan.\n\r", ch);
      return;
    }
  // the player better be on line too
  m = find_member (clan_table[clan].members, arg);
  if (m == NULL)
    {
      send_to_char ("They are not in your clan.", ch);
      return;
    }
  // well m had better not be null by the time we get here
  if (m->status < CLAN_DEPUTY)
    {

      // promote them
      m->status = CLAN_DEPUTY;
      save_clan (clan);

      // if the new deputy is on line let him know
      vic = find_character (arg);
      if (vic != NULL)
	{

	  // this deputy flag should not be used, always false now
	  vic->pcdata->deputy = FALSE;
	  save_char_obj (vic);
	  sprintf (buf, "You are now a deputy of clan %s``.\n\r",
		   clan_table[clan].name);
	  send_to_char (buf, vic);
	}
      // tell the player of his action
      sprintf (buf, "You have made %s a deputy of clan %s``.\n\r",
	       capitalize (arg), clan_table[clan].name);
      send_to_char (buf, ch);
    }

  else
    {

      // demote them
      m->status = CLAN_ACCEPTED;
      save_clan (clan);
      vic = find_character (arg);
      if (vic != NULL)
	{

	  // this deputy flag should not be used, always false now
	  vic->pcdata->deputy = FALSE;
	  save_char_obj (vic);
	  sprintf (buf, "You are no longer a deputy of clan %s``.\n\r",
		   clan_table[clan].name);
	  send_to_char (buf, vic);
	}
      // tell the player of his action
      sprintf (buf,
	       "You have revoked %s clan %s`` deputy privileges.\n\r",
	       capitalize (arg), clan_table[clan].name);
      send_to_char (buf, ch);
    }

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_deputy", ch, argument);

#endif /*  */
}


//
// Allow a player to attempt to establish a clan
//
void clan_establish (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  int slot, oldslot;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_establish", ch, argument);

#endif /*  */

//  if (!IS_IMMORTAL (ch))
  //   {
  //    send_to_char ("You must contact the immortal staff if you wish to create a new clan.\n\r", ch);
  //   return;
  //}

//  if (!IS_IMMORTAL (ch))
  //  {
  if (is_clan_leader (ch))
    {
      send_to_char ("You are already the leader of a clan.\n\r", ch);
      return;
    }
  slot = ch->clan;
  if (slot != CLAN_BOGUS)
    {
      sprintf (buf,
	       "You are a member of clan %s``, you must defect to establish clan.\n\r",
	       clan_table[slot].name);
      send_to_char (buf, ch);
      return;
    }
  oldslot = is_any_clan_applicant (ch);
  if ((oldslot != CLAN_BOGUS)
      && (clan_table[oldslot].status >= CLAN_PROPOSED))
    {
      sprintf (buf,
	       "You are already in clan %s``, you must defect in order to establish a clan.\n\r",
	       clan_table[oldslot].name);
      send_to_char (buf, ch);
      return;
    }
  if (ch->level < MIN_CLAN_LEVEL)
    {
      sprintf (buf,
	       "You must be at least level %d to establish a clan.\n\r",
	       MIN_CLAN_LEVEL);
      send_to_char (buf, ch);
      return;
    }
  // }
  if (clan_number >= (MAX_CLAN - 1))
    {
      send_to_char
	("Sorry, there are already too many clans in play.\n\r", ch);
      return;
    }
  add_clan (ch, argument);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_establish", ch, argument);

#endif /*  */
}


//
//  Finish the establishment functions
//
void add_clan (CHAR_DATA * ch, char *argument)
{
  MEMBER *m = NULL;
  char buf[MAX_INPUT_LENGTH];
  char name[MAX_INPUT_LENGTH];
  char symbol[MAX_INPUT_LENGTH];
  char status[MAX_INPUT_LENGTH];
  int i = 0;
  int slot = 0;

#ifdef VERBOSE_CLANS
  log_player_command ("s:add_clan", ch, argument);

#endif /*  */
  argument = one_argument_nl (argument, name);
  argument = one_argument_nl (argument, symbol);
  argument = one_argument_nl (argument, status);
  if (name[0] == '\0' || symbol[0] == '\0' || status[0] == '\0')
    {
      send_to_char
	("You must supply a clan name, a symbol and pk status.\r\n", ch);
      return;
    }
  if (color_strlen (name) > MAX_NAME_CH)
    {
      sprintf (buf, "The clan name may not exceed %d characters.\n\r",
	       MAX_NAME_CH);
      send_to_char (buf, ch);
      return;
    }
  if (strlen (name) > MAX_NAME)
    {
      send_to_char ("Try using less colors in your clan name.\n\r", ch);
      return;
    }
  if (color_strlen (symbol) > MAX_SYMB_CH)
    {
      sprintf (buf, "The clan symbol may not exceed %d characters.\n\r",
	       MAX_SYMB_CH);
      send_to_char (buf, ch);
      return;
    }
  if (strlen (symbol) > MAX_SYMB)
    {
      send_to_char ("Try using fewer colors in your clan symbol.\n\r", ch);
      return;
    }
  if (clan_lookup (name) != CLAN_BOGUS)
    {
      send_to_char ("That clan name is reserved or in use.\n\r", ch);
      return;
    }
  if (clan_symbol_lookup (symbol) != CLAN_BOGUS)
    {
      send_to_char ("That clan symbol is reserved or in use.\n\r", ch);
      return;
    }
  // names must be alphanumeric with no spaces
  for (i = 0; name[i] != '\0'; i++)
    {
      if (!isalnum ((int) name[i]) || isspace ((int) name[i]))
	{
	  send_to_char ("That is an illegal clan name.\n\r", ch);
	  return;
	}
    }

  // clan symbol with no spaces
  for (i = 0; symbol[i] != '\0'; i++)
    {
      if (isspace ((int) symbol[i]))
	{
	  send_to_char ("That is an illegal clan symbol.\n\r", ch);
	  return;
	}
    }

  // check that they are not trying to spoof the special clan symbol
  if (is_name_no_abbrev (symbol, CLAN_BOGUS_SYMB))
    {
      send_to_char ("That is an illegal clan symbol.\n\r", ch);
      return;
    }
  if (!IS_IMMORTAL (ch))
    {
      if (is_name_no_abbrev
	  (name,
	   "all auto withdraw immortal self someone something the you demise balance circle loner honor fuck satan angel whore christ bastard clan bogus"))
	{
	  send_to_char ("That is an illegal clan name.\n\r", ch);
	  return;
	}
    }
  // check that they are not trying to spoof the special clan name
  if (is_name_no_abbrev (name, CLAN_BOGUS_NAME))
    {
      send_to_char ("That is an illegal clan name.\n\r", ch);
      return;
    }
  // Akamai 02/21/00 - Clans named after players. BAD
  // check that they are not trying name their clan after an
  // existing player - send a simple error message.
  if (is_player_name (name))
    {
      send_to_char ("It is a bad idea to name clans after players.\n\r", ch);
      return;
    }
  // check if they have decided upon a life of the clan,
  // pk or nopk, only immortals can change that afterwards
  if (!str_cmp (status, "pk") && !str_cmp (status, "nopk"))
    {
      send_to_char
	("You must decide if your clan is a PK or a NoPK clan.\n\r", ch);
      return;
    }
  // Making a nopk clan when you are already a loner is a no no
  if (ch->pcdata->loner && (str_cmp (status, "pk")))
    {
      send_to_char
	("You can't make a nopk clan when you are a loner.\n\r", ch);
      return;
    }
  // establish this clan as a proposed clan
  slot = clan_number;

  // we now have one more clan to consider
  clan_number++;

  // fill in the clan data
  strcpy (clan_table[slot].name, name);
  strcpy (clan_table[slot].symbol, symbol);
  m = new_member_elt ();
  m->status = CLAN_LEADER;
  strcpy (m->name, ch->name);
  m->align = ch->alignment;
  m->levels = ch->level;
  clan_table[slot].members = m;
  clan_table[slot].leader = m;
  clan_table[slot].hall = ROOM_VNUM_ALTAR;
  clan_table[slot].levels = ch->level;
  if (IS_IMMORTAL (ch))
    {
      clan_table[slot].status = CLAN_ACTIVE;
    }

  else
    {

      // Ok, ok, this may seem absurd but for testing we check that a
      // new clan might actually start out as a full clan
      if (clan_table[slot].levels > FULL_CLAN_LEVELS)
	{
	  clan_table[slot].status = CLAN_ACTIVE;
	}

      else
	{
	  clan_table[slot].status = CLAN_PROPOSED;
	}
    }
  clan_table[slot].ident = CLAN_PC_IDENT_MIN + _clan_ident_;
  if (!str_cmp (status, "pk"))
    clan_table[slot].ctype = CLAN_TYPE_PC;

  else
    clan_table[slot].ctype = CLAN_TYPE_EXPL;
  _clan_ident_++;

  // save the clan info
  save_clan (slot);
  save_clanindex ();

  // update this as if he joined a clan
  if (!IS_IMMORTAL (ch))
    {
      if (clan_table[slot].ctype == CLAN_TYPE_EXPL)
	ch->pcdata->loner = FALSE;

      else
	ch->pcdata->loner = TRUE;
      ch->pcdata->deputy = FALSE;
      ch->clan = slot;
      save_char_obj (ch);
    }

  else
    {
      ch->clan = slot;
    }
  sprintf (buf, "You are now the leader of clan %s``.\r\n",
	   clan_table[slot].name);
  send_to_char (buf, ch);

#ifdef VERBOSE_CLANS
  log_player_command ("f:add_clan", ch, argument);

#endif /*  */
}


//
//  Allow a clan member to donate an item to a clan box
//
void clan_donate (CHAR_DATA * ch, char *argument)
{

#ifdef VERBOSE_CLANS
  char buf[MAX_INPUT_LENGTH];

#endif /*  */
  char arg1[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  OBJ_DATA *obj, *container;
  int slot;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_donate", ch, argument);

#endif /*  */
  slot = ch->clan;
  if (slot == CLAN_BOGUS)
    {
      send_to_char ("You don't currently belong to a clan.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg1);
  if (arg1[0] == '\0')
    {
      send_to_char ("Donate what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_carry (ch, arg1)) == NULL)
    {
      send_to_char ("You do not have that item.\n\r", ch);
      return;
    }
  if (obj->wear_loc != -1)
    {
      send_to_char
	("You will have to remove that before you can donate it.\n\r", ch);
      return;
    }
  if (ch->fighting != NULL)
  {
    send_to_char("You can't clan donate while fighting.  That is LAME.\n\r",ch);
    return;
  }
  // Adeon 7/18/03 -- can't donate nosave eq
  if (IS_OBJ_STAT (obj, ITEM_NOSAVE) || IS_SET(obj->extra_flags[1],ITEM_NODONATE)
		  || obj->item_type == ITEM_CARD)
    {
      send_to_char ("Strong magic forces prevent you...\n\r", ch);
      return;
    }
  if (IS_OBJ_STAT (obj, ITEM_NODROP))
    {
      send_to_char ("Sorry, you can't donate that.\n\r", ch);
      return;
    }
  // What the hell are these magic numbers... This is a straight copy
  // from the previous code... sheesh
  if (obj->pIndexData->vnum >= 23000 && obj->pIndexData->vnum <= 23100)
    {
      send_to_char ("Sorry, you can't donate that.\n\r", ch);
      return;
    }
#ifdef VERBOSE_CLANS
  sprintf (buf, "clan_donate: %s is allowed to donate %s", ch->name,
	   obj->name);
  log_string (buf);

#endif /*  */
  container = clan_table[slot].clan_box;

  // if the clan box is not yet set, look for it
  if (container == NULL)
    {

#ifdef VERBOSE_CLANS
      sprintf (buf,
	       "clan_donate: searching all objects for clan donation box");
      log_string (buf);

#endif /*  */
      container = object_list;
      while (container != NULL)
	{
	  if (((container->item_type == ITEM_CLAN_DONATION) ||
	       (container->item_type == ITEM_NEWCLANS_DBOX)) &&
	      (container->value[1] == slot))
	    break;
	  container = container->next;
	}

      // if we don't find the clan box then bail
      if (container == NULL)
	{
	  send_to_char ("Your clan does not have a donation box.\n\r", ch);
	  return;
	}
      // found it so set it, we won't have to do this again
      clan_table[slot].clan_box = container;
    }
  if (container == NULL)
    {
      send_to_char ("You clan does not have a donation box.\n\r", ch);
      return;
    }
  if (container->value[0] != -1)
    {
      if (get_obj_number (container) >= container->value[0])
	{
	  send_to_char ("Sorry, the clan donation box is full.\n\r", ch);
	  return;
	}
    }
#ifdef VERBOSE_CLANS
  sprintf (buf, "clan_donate: box is found and has room for the new item");
  log_string (buf);

#endif /*  */
  if (ch->in_room->vnum == container->in_room->vnum)
    {
      act ("$n puts $p in the clan donation box.", ch, obj, NULL, TO_ROOM);
      act ("You put $p in the clan donation box.", ch, obj, NULL, TO_CHAR);
    }

  else
    {
      location = container->in_room;
      if (location == NULL)
	return;
      act ("You send $p to your clan donation box.", ch, obj, container,
	   TO_CHAR);
      act
	("Something materializes from thin air and falls into the clan donation box.",
	 location->people, NULL, NULL, TO_ALL);
    }
  if ((container != NULL) &&
      ((container->item_type == ITEM_CLAN_DONATION) ||
       (container->item_type == ITEM_NEWCLANS_DBOX)))
    {
      obj->cost = 0;
      obj_from_char (obj);
      obj_to_obj (obj, container);

#ifdef VERBOSE_CLANS
      sprintf (buf, "clan_donate: item placed in box, saving box");
      log_string (buf);

#endif /*  */
      save_clan_box (container);
      save_char_obj (ch);
    }
#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_donate", ch, argument);

#endif /*  */
}


//
//  Allow the clan leader to disband a clan
//
void clan_disband (CHAR_DATA * ch, char *argument)
{
  MEMBER *m;
  CHAR_DATA *c;
  char buf[MAX_INPUT_LENGTH];
  char name[MAX_INPUT_LENGTH];
  int slot;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_disband", ch, argument);

#endif /*  */
  if (IS_IMMORTAL (ch))
    {
      argument = one_argument (argument, name);
      if (name[0] == '\0')
	{
	  send_to_char ("Disband which clan?\n\r", ch);
	  return;
	}
      slot = clanname_to_slot (name);
      if (slot == CLAN_BOGUS)
	{
	  sprintf (buf, "There is no clan named %s``.\n\r",
		   capitalize (name));
	  send_to_char (buf, ch);
	  return;
	}
      if (clan_table[slot].status < CLAN_PROPOSED)
	{
	  sprintf (buf,
		   "Clan %s`` is already in a deactivated state.\n\r",
		   capitalize (name));
	  send_to_char (buf, ch);
	  return;
	}
    }

  else
    {

      // ok not an immortal so validate
      slot = ch->clan;
      if (slot == CLAN_BOGUS)
	{
	  send_to_char ("You are not in a clan.\n\r", ch);
	  return;
	}
      if (clan_table[slot].status < CLAN_PROPOSED)
	{
	  send_to_char ("You are not in a clan.\n\r", ch);
	  return;
	}
      if (!is_clan_leader (ch))
	{
	  send_to_char ("You are not the leader of your clan.\n\r", ch);
	  return;
	}
    }

  // if the clan is in a war, don't allow a disband right now
  if (clan_is_warring (slot))
    {
      sprintf (buf,
	       "Clan %s`` is currently at war and cannot be disbanded.\r\n",
	       clan_table[slot].name);
      send_to_char (buf, ch);
      return;
    }
  // notify the membership that they were disbanded
  sprintf (buf, "Clan %s`` has been disbanded by %s.\r\n",
	   clan_table[slot].name, ch->name);
  if (IS_IMMORTAL (ch))
    {
      send_clan_members (slot, buf);
    }

  else
    {
      send_clan_members_ch (ch, buf);
    }

  // notify the player of his action
  sprintf (buf, "Clan %s`` was disbanded.\r\n", clan_table[slot].name);
  send_to_char (buf, ch);
  clan_table[slot].status = CLAN_DISBAND;

  // remove every other clans relation with us and save them
  remove_clan_relations (clan_table[slot].ident);

  // for now a disband does not remove any clan symbols
  // remove_clan_symbols();
  save_clan (slot);

  // update the members to think they are not in the clan
  m = clan_table[slot].members;
  while (m)
    {
      c = find_character (m->name);
      if (c)
	{
	  if (clan_table[c->clan].ctype == CLAN_TYPE_PC)
	    c->pcdata->loner = TRUE;

	  else
	    c->pcdata->loner = FALSE;
	  c->clan = CLAN_BOGUS;
	}
      m = m->next;
    }

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_disband", ch, argument);

#endif /*  */
}


//
// Allow a leader or an immortal to name a new clan leader
//
void clan_leader (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  MEMBER *m;
  int slot = 0;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_leader", ch, argument);

#endif /*  */
  argument = one_argument (argument, arg);
  one_argument (argument, arg1);

  // validate syntax
  if (arg[0] == '\0')
    {
      send_to_char ("Make whom the clan leader?\r\n", ch);
      return;
    }
  // check for a valid clan
  if (arg1[0] != '\0')
    {
      slot = clanname_to_slot (arg1);
      if (slot == CLAN_BOGUS)
	{
	  send_to_char ("No such clan exists.\n\r", ch);
	  return;
	}
    }

  else
    {
      slot = ch->clan;
      if (slot == CLAN_BOGUS)
	{
	  send_to_char ("You are not in a clan.\n\r", ch);
	  return;
	}
    }

  // is this character a leader or an imm
  if (!is_clan_leader (ch) && !IS_IMMORTAL (ch))
    {
      send_to_char ("You are not the leader of your clan.\n\r", ch);
      return;
    }
/*  if (!IS_NPC (ch))
    if ((time (NULL) - ch->pcdata->last_fight < 120) && (!IS_IMMORTAL(ch)))
      {
        send_to_char ("Hold your horses, punk.\n\r", ch);
        return;
      }*/
  
  m = find_member (clan_table[slot].members, arg);
  if (m == NULL)
    {
      sprintf (buf, "%s is not a member of clan %s``.\n\r", arg,
	       clan_table[slot].name);
      send_to_char (buf, ch);
      return;
    }
  // this is safe, because we already checked that this
  // player is in this clan
  clan_table[slot].leader->status = CLAN_DEPUTY;
  clan_table[slot].leader = m;
  clan_table[slot].leader->status = CLAN_LEADER;

  // save this update
  save_clan (slot);

  // notify the membership of their new leader
  sprintf (buf, "Clan %s`` leader is now %s.\r\n",
	   clan_table[slot].name, clan_table[slot].leader->name);
  if (IS_IMMORTAL (ch))
    {
      send_clan_members (slot, buf);
    }

  else
    {
      send_clan_members_ch (ch, buf);
    }

  // notify the individual of their demotion
  sprintf (buf,
	   "You have made %s leader of clan %s``.\r\nYou are no longer leader.\n\r",
	   clan_table[slot].leader->name, clan_table[slot].name);
  send_to_char (buf, ch);
  victim = find_character (arg);
  if (victim != NULL)
    {
      sprintf (buf, "You are now the leader of clan %s``.\n\r",
	       clan_table[slot].name);
      send_to_char (buf, victim);
    }
#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_leader", ch, argument);

#endif /*  */
}


//
// Allow a clan leader to define a new clan symbol
//
void clan_symbol (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  int slot;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_symbol", ch, argument);

#endif /*  */
  argument = one_argument (argument, arg);
  one_argument (argument, arg1);

  // check for valid syntax
  if (arg[0] == '\0')
    {
      send_to_char ("Change your clan symbol to what?\n\r", ch);
      return;
    }
  // check for a valid clan
  if (arg1[0] != '\0')
    {
      slot = clanname_to_slot (arg1);
      if (slot == CLAN_BOGUS)
	{
	  send_to_char ("No such clan exists.\n\r", ch);
	  return;
	}
    }

  else
    {
      slot = ch->clan;
      if (slot == CLAN_BOGUS)
	{
	  send_to_char ("You are not in a clan.\n\r", ch);
	  return;
	}
    }

  // check for leadership rights the character must either be the clan
  // leader or it better be an imm
  if (!is_clan_leader (ch) && !IS_IMMORTAL (ch))
    {
      send_to_char ("You are not the leader of your clan.\n\r", ch);
      return;
    }
  // Abusive behaviour has made it necessary to restrict changing of clan
  // Symbols to immortals only
/*  if (!IS_IMMORTAL (ch))
    {
      send_to_char ("Talk to an immortal to make a change in clan symbol.\n\r", ch);
      return;
    }*/
  if (color_strlen (arg) > MAX_SYMB_CH)
    {
      sprintf (buf,
	       "The clan symbol must be less than %d characters.\n\r",
	       MAX_SYMB_CH);
      send_to_char (buf, ch);
      return;
    }
  if (strlen (arg) > MAX_SYMB)
    {
      send_to_char ("Try using less colors in your clan symbol.\n\r", ch);
      return;
    }
  strcpy (clan_table[slot].symbol, arg);
  save_clan (slot);
  sprintf (buf, "Clan %s`` symbol is now %s``.\r\n",
	   clan_table[slot].name, clan_table[slot].symbol);
  send_to_char (buf, ch);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_symbol", ch, argument);

#endif /*  */
}


// removes a member record
void clan_bye (CHAR_DATA * ch)
{
  MEMBER *m;
  int slot;
  slot = ch->clan;
  if (slot != CLAN_BOGUS)
    {
      m = clan_table[slot].members;
      clan_table[slot].members = delete_member (m, ch->name);
    }
}


//
// List clans all and hidded for immortals
//
void clan_list_imm (CHAR_DATA * ch, char *argument)
{
  int i = 0;
  int statuses = CLAN_STATUSES;
  bool found = FALSE;
  char buf[MAX_INPUT_LENGTH];

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_list_imm", ch, argument);

#endif /*  */
  if (!IS_IMMORTAL (ch))
    return;
  if (!str_cmp (argument, "hidden"))
    {
      send_to_char ("Clan list hidden:\n\r", ch);
      statuses = HIDDEN_STATUSES;
    }

  else if (!str_cmp (argument, "all"))
    {
      send_to_char ("Clan list all:\n\r", ch);
      statuses = CLAN_STATUSES;
    }

  else
    {
      send_to_char ("Options to clan list are \"hidden\" or \"all\"\n\r", ch);
      return;
    }
  if (CLANS_START < clan_number)
    {
      sprintf (buf,
	       "Name         Symbol       Leader       PK/NonPK    Status       Levels Slot\n\r");
      send_to_char (buf, ch);
    }

  else
    {
      send_to_char ("None.\n\r", ch);
      return;
    }

  // do the list in status order - see the clan_statuses const table
  for (i = 0; i < statuses; i++)
    {
      if (clan_list_status (ch, clan_statuses[i]))
	found = TRUE;
    }
  if (!found)
    send_to_char ("None.\n\r", ch);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_list_imm", ch, argument);

#endif /*  */
}
bool clan_list_status (CHAR_DATA * ch, int status)
{
  int i = 0;
  bool found = FALSE;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char symb[MAX_STRING_LENGTH];

  // now run through the clans putting stuff in the table
  for (i = CLANS_START; i < clan_number; i++)
    {
      if (clan_table[i].status == status)
	{
	  found = TRUE;
	  format_clan_symbol (i, symb, 12);
	  sprintf (buf, "%-12s`` %-12s`` %-12s %-12s",
		   clan_table[i].name, symb, clan_table[i].leader->name,
		   clan_table[i].ctype == CLAN_TYPE_PC ? "PK" : "NonPK");
	  switch (clan_table[i].status)
	    {
	    case CLAN_INACTIVE:
	      strcat (buf, "Inactive    ");
	      break;
	    case CLAN_DISBAND:
	      strcat (buf, "Disbanded   ");
	      break;
	    case CLAN_DISPERSED:
	      strcat (buf, "Dispersed   ");
	      break;
	    case CLAN_PROPOSED:
	      strcat (buf, "Proposed    ");
	      break;
	    case CLAN_RESTRICTED:
	      strcat (buf, "Restricted  ");
	      break;
	    case CLAN_HISTORICAL:
	      strcat (buf, "Historical  ");
	      break;
	    case CLAN_ACTIVE:
	      strcat (buf, "Active      ");
	      break;
	    default:
	      strcat (buf, "Unknown     ");
	      break;
	    }
	  if (IS_IMMORTAL (ch))
	    {
	      sprintf (buf2, "%s %6d [%2d]\n\r", buf,
		       clan_table[i].levels, i);
	    }

	  else
	    {

	      // regular characters do not get the extra status information
	      sprintf (buf2, "%s\n\r", buf);
	    }
	  send_to_char (buf2, ch);
	}
    }
  return (found);
}


//
// Allow immortals to reactivate a clan that was deactivated
//
void clan_activate (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  char clan_name[MAX_INPUT_LENGTH];
  MEMBER *m = NULL;
  MEMBER *list = NULL;
  CHAR_DATA *c = NULL;
  bool found = FALSE;
  int i = 0;
  int slot = 0;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_activate", ch, argument);

#endif /*  */
  if (!IS_IMMORTAL (ch))
    return;
  argument = one_argument (argument, clan_name);

  // check for a valid clan
  if (clan_name[0] == '\0')
    {
      send_to_char ("You must specify a clan to reactivate.\n\r", ch);
      return;
    }
  slot = clanname_to_slot (clan_name);
  if (slot == CLAN_BOGUS)
    {
      sprintf (buf, "There is no clan named %s``.\n\r",
	       capitalize (clan_name));
      send_to_char (buf, ch);
      return;
    }
  if (clan_table[slot].status < CLAN_PROPOSED)
    {

      // ok, make the clan active - but because the membership has not
      // been managed, we need to check the consistency of the membership
      // First, we'll just dump all applicants
      list = clan_table[slot].applicants;
      clan_table[slot].applicants = NULL;
      m = list;
      while (m)
	{
	  list = m->next;
	  m->next = NULL;
	  free (m);
	  m = list;
	}

      // scan the members list and see if any of them are members in another
      // clan - if they are members else where then remove them from this
      // clans membership list
      list = clan_table[slot].members;
      clan_table[slot].members = NULL;
      m = list;
      while (m)
	{
	  list = m->next;
	  m->next = NULL;

	  // check all of the existing clans to see if this guy is a member
	  // of any of them -- ignore the clan we are currently fixing
	  i = CLANS_START;
	  while ((i < clan_number) && m)
	    {
	      if (i != slot)
		{
		  if (find_member (clan_table[i].members, m->name))
		    {

		      // ok, m has already switched to another clan
		      free (m);
		      m = NULL;
		    }
		}
	      i++;
	    }

	  // if m != NULL then they guy is *not* in another clan,
	  // so add him to the members list of this clan
	  if (m)
	    {
	      clan_table[slot].members =
		append_member (clan_table[slot].members, m);
	    }
	  m = list;
	}

      // now make one of the clan members the leader
      clan_table[slot].leader = NULL;
      m = clan_table[slot].members;
      while (m && !found)
	{
	  if (m->status == CLAN_LEADER)
	    {
	      found = TRUE;
	    }

	  else
	    {
	      m = m->next;
	    }
	}

      // if we found the clan leader great, if we didn't find him then he
      // joined another clan, just pick the first member in the list and make
      // that guy the clan leader -- an imm can fix it using the leader command
      if (found)
	{
	  clan_table[slot].leader = m;
	}

      else
	{
	  clan_table[slot].leader = clan_table[slot].members;
	  clan_table[slot].leader->status = CLAN_LEADER;
	}

      // finally, don't actually make the clan active yet, keep it restricted
      // and hope they accept a new member or something to fix up the levels
      clan_table[slot].status = CLAN_RESTRICTED;
      save_clan (slot);

      // run through all of the members, if we find one on-line update them
      m = clan_table[slot].members;
      while (m)
	{
	  c = find_character (m->name);
	  if (c)
	    c->clan = slot;
	  m = m->next;
	}
      sprintf (buf,
	       "Clan %s`` was resurected to a restricted status.\n\r",
	       clan_table[slot].name);
      send_to_char (buf, ch);
      sprintf (buf, "Member %s was made leader of clan %s``.\n\r",
	       clan_table[slot].leader->name, clan_table[slot].name);
      send_to_char (buf, ch);
    }

  else if (clan_table[slot].status < CLAN_ACTIVE)
    {

      // the membership has already been under appropriate restrictions
      // will allow the imms to promote a clan temporarily for some reason
      clan_table[slot].status = CLAN_ACTIVE;
      sprintf (buf,
	       "Clan %s`` was temporarily promoted to active status.\n\r",
	       clan_table[slot].name);
      send_to_char (buf, ch);
    }

  else
    {
      sprintf (buf, "Clan %s`` is already an active clan.\n\r",
	       clan_table[slot].name);
      send_to_char (buf, ch);
    }

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_activate", ch, argument);

#endif /*  */
}
void clan_altar (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  char name[MAX_INPUT_LENGTH];
  int slot;

#ifdef VERBOSE_CLANS
  log_player_command ("s:clan_altar", ch, argument);

#endif /*  */
  if (IS_IMMORTAL (ch))
    {
      argument = one_argument (argument, name);

      // check for a valid clan
      if (name[0] == '\0')
	{
	  send_to_char
	    ("You must specify a clan to set their altar.\n\r", ch);
	  return;
	}
      slot = clanname_to_slot (name);
      if (slot == CLAN_BOGUS)
	{
	  sprintf (buf, "There is no clan named %s``.\n\r",
		   capitalize (name));
	  send_to_char (buf, ch);
	  return;
	}
      if (clan_table[slot].status < CLAN_PROPOSED)
	{
	  sprintf (buf, "Clan %s`` is not an active clan.\n\r",
		   clan_table[slot].name);
	  send_to_char (buf, ch);
	  return;
	}
    }

  else
    {
      slot = ch->clan;
      if (slot == CLAN_BOGUS)
	{
	  send_to_char ("You are not currently in a clan.\n\r", ch);
	  return;
	}
      if (!is_clan_leader (ch))
	{
	  send_to_char
	    ("You must be a clan leader to set the clan altar.\n\r", ch);
	  return;
	}
      if (clan_table[slot].hall != ROOM_VNUM_ALTAR)
	{
	  send_to_char ("You have already set your clan altar.\n\r", ch);
	  return;
	}
      if (ch->in_room->clan != slot)
	{
	  sprintf (buf, "This room is not owned by clan %s``.",
		   clan_table[slot].name);
	  send_to_char (buf, ch);
	  return;
	}
    }

  // get the vnum of this room and save the clan
  clan_table[slot].hall = ch->in_room->vnum;
  save_clan (slot);
  sprintf (buf, "You invoke spirits, opening a passage to the netherworld.");
  send_to_char (buf, ch);
  sprintf (buf, "Your make %s`` your clan altar.", ch->in_room->name);
  send_to_char (buf, ch);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_altar", ch, argument);

#endif /*  */
}
void clan_short_status (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  char name[MAX_INPUT_LENGTH];
  int slot;
  argument = one_argument (argument, name);

  // check for a valid clan
  if (name[0] == '\0')
    {
      slot = ch->clan;
      if (slot == CLAN_BOGUS)
	{
	  send_to_char ("You are not currently in a clan.\n\r", ch);
	  return;
	}
    }

  else
    {
      slot = clanname_to_slot (name);
      if (slot == CLAN_BOGUS)
	{
	  sprintf (buf, "There is no clan named %s``.\n\r",
		   capitalize (name));
	  send_to_char (buf, ch);
	  return;
	}
    }
  switch (clan_table[slot].status)
    {
    case CLAN_INACTIVE:
      sprintf (buf, "Clan %s`` is currently inactive.\n\r",
	       clan_table[slot].name);
      break;
    case CLAN_DISBAND:
      sprintf (buf, "Clan %s`` is currently disbanded.\n\r",
	       clan_table[slot].name);
      break;
    case CLAN_DISPERSED:
      sprintf (buf, "Clan %s`` is currently dispersed.\n\r",
	       clan_table[slot].name);
      break;
    case CLAN_PROPOSED:
      sprintf (buf, "Clan %s`` is currently proposed.\n\r",
	       clan_table[slot].name);
      break;
    case CLAN_RESTRICTED:
      sprintf (buf, "Clan %s`` is currently restricted.\n\r",
	       clan_table[slot].name);
      break;
    case CLAN_HISTORICAL:
      sprintf (buf, "Clan %s`` is a historically significant clan.\n\r",
	       clan_table[slot].name);
      break;
    case CLAN_ACTIVE:
      sprintf (buf, "Clan %s`` is currently active.\n\r",
	       clan_table[slot].name);
      break;
    default:
      sprintf (buf, "Clan %s`` is currently <status_unknown>.\n\r",
	       clan_table[slot].name);
      break;
    }
  send_to_char (buf, ch);
}
void list_allies (CHAR_DATA * ch, int slot)
{
  RELATIONS *r;
  int cnt = 0;
  bool found = FALSE;
  char outbuf[MAX_STRING_LENGTH * 2];

  // list off the allies
  sprintf (outbuf, "Allies of clan %s``:\n\r", clan_table[slot].name);
  r = clan_table[slot].relations;
  while (r)
    {
      if (r->status == CLAN_ALLY)
	{

	  // ok, we marked them as an ally, did they also mark us as an ally?
	  if (is_requested_ally (clanident_to_slot (r->ident), slot))
	    {

	      // if then are in the clan, let them know if the other clan is
	      // a full ally or just a friend
	      if ((ch->clan == slot) || IS_IMMORTAL (ch))
		{
		  sprintf (outbuf + strlen (outbuf), "A: %-12s``", r->name);
		}

	      else
		{
		  sprintf (outbuf + strlen (outbuf), "%-15s``", r->name);
		}
	      cnt++;
	      found = TRUE;

	      // every three clans go to a new line
	      if (cnt % 3 == 0)
		strcat (outbuf, "\n\r");
	    }

	  else
	    {

	      // only tell friend status to people in this clan
	      if ((ch->clan == slot) || IS_IMMORTAL (ch))
		{
		  sprintf (outbuf + strlen (outbuf), "F: %-12s``", r->name);
		  cnt++;
		  found = TRUE;

		  // every three clans go to a new line
		  if (cnt % 3 == 0)
		    strcat (outbuf, "\n\r");
		}
	    }
	}
      r = r->next;
    }
  if (!found)
    strcat (outbuf, "None.");
  strcat (outbuf, "\n\r");
  page_to_char (outbuf, ch);
}
void list_enemies (CHAR_DATA * ch, int slot)
{
  RELATIONS *r;
  int cnt = 0;
  bool found = FALSE;
  char outbuf[MAX_STRING_LENGTH * 2];

  // list off the enemies of this clan
  sprintf (outbuf, "Enemies of clan %s``:\n\r", clan_table[slot].name);
  r = clan_table[slot].relations;
  while (r)
    {
      if (r->status == CLAN_ENEMY)
	{

	  // we have marked them as an enemy of us
	  if (is_requested_enemy (clanident_to_slot (r->ident), slot))
	    {

	      // they also marked us as an enemy of them
	      if ((ch->clan == slot) || IS_IMMORTAL (ch))
		{
		  sprintf (outbuf + strlen (outbuf), "E: %-12s``", r->name);
		}

	      else
		{
		  sprintf (outbuf + strlen (outbuf), "%-15s``", r->name);
		}
	      cnt++;
	      found = TRUE;

	      // every three clans go to a new line
	      if (cnt % 3 == 0)
		strcat (outbuf, "\n\r");
	    }

	  else
	    {

	      // only tell hostile status to people in this clan
	      if ((ch->clan == slot) || IS_IMMORTAL (ch))
		{
		  sprintf (outbuf + strlen (outbuf), "H: %-12s``", r->name);
		  cnt++;
		  found = TRUE;

		  // every three clans go to a new line
		  if (cnt % 3 == 0)
		    strcat (outbuf, "\n\r");
		}
	    }
	}

      else if (r->status >= CLAN_WAR)
	{

	  // actually it's worse there is a war going on
	  if ((ch->clan == slot) || IS_IMMORTAL (ch))
	    {

	      // only be explicit to members of this clan
	      sprintf (outbuf + strlen (outbuf), "W: %-12s``", r->name);
	    }

	  else
	    {

	      // not a member then you just see a name
	      sprintf (outbuf + strlen (outbuf), "%-15s``", r->name);
	    }
	  cnt++;
	  found = TRUE;

	  // every three clans go to a new line
	  if (cnt % 3 == 0)
	    strcat (outbuf, "\n\r");
	}
      r = r->next;
    }
  if (!found)
    strcat (outbuf, "None.");
  strcat (outbuf, "\n\r");
  page_to_char (outbuf, ch);
}


// remove this clans relations with all the other clans
void remove_clan_relations (int ident)
{
  int i;
  for (i = CLANS_START; i < clan_number; i++)
    {
      clan_table[i].relations =
	delete_relation (clan_table[i].relations, ident);
      save_clan (i);
    }
}
