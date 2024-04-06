#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "merc.h"
#include "obj_trig.h"

int superduel_ticks;
bool superduel_teams;
extern bool battle_royale;
void do_echo (CHAR_DATA * ch, char *argument);
void do_goto (CHAR_DATA * ch, char *argument);
void do_transfer (CHAR_DATA * ch, char *argument);
extern char str_empty[1];
struct duel_data *current_duel = NULL;
extern bool make_backup_duel;
ROOM_INDEX_DATA *find_location args ((CHAR_DATA * ch, char *arg));
void do_ooc args((CHAR_DATA * ch, char *argument));
void arena_equip (CHAR_DATA *, int);
void arena_chan args((char* text));
void unlink_duel_backups (CHAR_DATA * loser, CHAR_DATA * winner)
{
  char strsave[MAX_STRING_LENGTH];
  sprintf (strsave, "%s%c/%s%s", PLAYER_DIR, toupper (loser->name[0]),
	   capitalize (loser->name), PFILE_BACKUP_DUEL);
  unlink (strsave);
  sprintf (strsave, "%s%c/%s%s", PLAYER_DIR, toupper (winner->name[0]),
	   capitalize (winner->name), PFILE_BACKUP_DUEL);
  unlink (strsave);
  return;
}

void do_stripaffects args((CHAR_DATA *ch,char* argument));
	

/* This function is run when user uses the 'duel' command */
void award_item (CHAR_DATA * winner, CHAR_DATA * loser)
{
  OBJ_DATA *some_obj;
  int total_stuff = 0, xx, rndnum;
  char tcbuf[MAX_INPUT_LENGTH];
  if (loser->carrying == NULL)
    {
      send_to_char
	("Unfortunately there are no items to be won from the defeated.\n\r",
	 winner);

      /*
         send_to_char("In lieu of an item from the defeated, you are awarded 5000 silver.\n\r", winner);
         winner->silver += 5000; */
      send_to_char ("Luckily, you have no equipment to lose.\n\r", loser);
      return;
    }
  total_stuff = 0;
  some_obj = loser->carrying;
  for (some_obj = loser->carrying; some_obj != NULL;
       some_obj = some_obj->next_content)
    total_stuff++;
  rndnum = number_range (1, total_stuff);
  xx = 1;
  for (some_obj = loser->carrying; xx < rndnum && some_obj != NULL;
       some_obj = some_obj->next_content)
    xx++;
  if (some_obj == NULL)
    {
      send_to_char ("OBJIS NULL BUG!\n\r", winner);
      send_to_char ("OBJIS NULL BUG!\n\r", loser);
    }
  if (some_obj->wear_loc != WEAR_NONE)
    unequip_char (loser, some_obj);
  obj_from_char (some_obj);
  obj_to_char (some_obj, winner);
  sprintf (tcbuf,
	   "You have also won %s as tribute from the defeated.\n\r",
	   some_obj->short_descr);
  send_to_char (tcbuf, winner);
  sprintf (tcbuf,
	   "Shamefully defeated, you are stripped of %s, which has been awarded to the victor.\n\r",
	   some_obj->short_descr);
  send_to_char (tcbuf, loser);
}

void do_duel (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_STRING_LENGTH];	/* Temporary string variable */
  CHAR_DATA *to_duel, *tch_next, *tch;	/* Some character structures */
  int tch_count = 0;		/* Integer counting variable */
  ROOM_INDEX_DATA *location;	/* Room location structure */
  int level_avg;
  OBJ_DATA *obj, *obj_next;
  if (!str_cmp (argument, "cancel"))
    {
      if (IS_TRUSTED (ch, GOD))
	{
	  if (current_duel == NULL)
	    {
	      send_to_char ("There is no duel in progress!\n\r", ch);
	      return;
	    }
	  else
	    {
	      send_to_char ("Cancelling current duel...\n\r", ch);
	      duel_cancel (ch);
	      return;
	    }
	}
      else
	{
	  send_to_char ("Sorry, you can't do that.\n\r", ch);
	  return;
	}
    }
  if (!IS_NPC (ch))
    if ((time (NULL) - ch->pcdata->last_fight < 120))
      {
	send_to_char ("You can't duel right now.\n\r", ch);
	return;
      }

  /* Check to make sure user is in an arena registration room */
  if (!IS_SET (ch->in_room->room_flags, ROOM_ARENA_REGISTRATION)
      && argument[0] != '\0')
    {
      send_to_char
	("You must be in the Hall of Legends to use this command!\n\r", ch);
      return;
    }
  if (argument[0] != '\0')
    {				/* Did user supply an argument to duel? */
      if (!str_cmp (argument, "reject"))
	{			/* If argument was 'reject'... */

	  /* Loop through the character list looking for anyone dueling
	   * this character.  If any are found, cancel the duel 
	   * requests and inform both parties */
	  for (tch = char_list; tch != NULL; tch = tch_next)
	    {
	      tch_next = tch->next;
	      if (tch->duel == ch->name)
		{
		  sprintf (tcbuf,
			   "%s has rejected your duel request!\n\r",
			   ch->name);
		  send_to_char (tcbuf, tch);
		  tch->duel = &str_empty[0];
		  tch_count++;
		  break;
		}
	    }
	  if (tch_count == 0)	/* if 0, no one was dueling this character */
	    send_to_char ("No one has requested a duel with you.\n\r", ch);

	  else
	    {
	      sprintf (tcbuf, "Rejecting duel request from %s.\n\r",
		       tch->name);
	      send_to_char (tcbuf, ch);
	    }
	  return;		/* We're done here, return out of function */
	}
      if (!str_cmp (argument, "withdraw"))
	{			/* If argument is 'withdraw'... */

	  /* Remove duel request from character structure... */
	  if (ch->duel[0] != '\0')
	    {
	      send_to_char ("Cancelling current duel request.\n\r", ch);
	      if (ch->duel != ch->name)
		if ((to_duel = get_char_world (ch, ch->duel)) != NULL)
		  {
		    act ("$n has withdrawn $S duel request.", ch, NULL,
			 to_duel, TO_VICT);
		  }
	      ch->duel = &str_empty[0];
	    }
	  else
	    send_to_char ("No current duel request exists.\n\r", ch);
	  return;
	}
      if (!str_cmp (argument, "add"))
	{			/* If add, add to the 'public' list */
	  send_to_char ("Adding you to the public duel list.\n\r", ch);
	  ch->duel = ch->name;
	  return;
	}

      /* Make sure named character exists and is online */
      if ((to_duel = get_char_world (ch, argument)) == NULL)
	{
	  send_to_char ("They aren't here.\n\r", ch);
	  return;		/* If not, do nothing. */
	}
      if (((ch->level < to_duel->level - 10)
	   || (ch->level > to_duel->level + 10))
	  && !IS_TRUSTED (ch, DEMIGOD) && !IS_TRUSTED (to_duel, DEMIGOD))
	{
	  send_to_char
	    ("Only fair and honorable fights may take place in the Arena!\n\r",
	     ch);
	  return;
	}
      if (to_duel == ch)
	{			/* Can't duel yourself! */
	  send_to_char ("Don't be silly!\n\r", ch);
	  return;
	}
      if (IS_NPC (to_duel))
	{			/* can't duel NPCs! */
	  send_to_char ("You can't duel a non-player character!\n\r", ch);
	  return;
	}
      if (ch->duel[0] != '\0' && ch->duel != ch->name)
	{			/* If a duel already exists, inform character, then return */
	  sprintf (tcbuf,
		   "You are alreading awaiting a duel with %s.  Type 'duel withdraw' to withdraw your request.\n\r",
		   ch->duel);
	  send_to_char (tcbuf, ch);
	  return;
	}
      if (to_duel->duel == ch->name)
	{
	  if (current_duel != NULL)
	    {
	      send_to_char
		("You must wait until the current duel has ended.\n\r", ch);
	      return;
	    }
	  if (!to_duel->in_room || to_duel->in_room != ch->in_room)
	  {
		  send_to_char("They must also be in this room for the duel to begin.\n\r",ch);
		  return;
	  }
	  if (!IS_NPC (to_duel))
	     if ((time (NULL) - to_duel->pcdata->last_fight < 120))
	     {
	        send_to_char ("They have fought too recently to join the duel.\n\r", ch);
	        return;
	     }
	  
	  if (to_duel->dueler)
	  {
		  send_to_char("You must wait until they are done super dueling.\n\r",ch);
		  return;
	  }
	  send_to_char ("THE DUEL BEGINS!  Entering the arena...\n\r", ch);
	  send_to_char ("THE DUEL BEGINS! Entering the arena...\n\r",
			to_duel);
	  ch->duel = &str_empty[0];
	  to_duel->duel = &str_empty[0];
	  sprintf (tcbuf,
		   "%s and %s have been transported to the Arena for a duel to the death!\n\r",
		   ch->name, to_duel->name);
	  arena_report (tcbuf);
	  act
	    ("$n and $N have been transported to the Arena for a duel to the death!\n\r",
	     ch, NULL, to_duel, TO_NOTVICT);
	  make_backup_duel = TRUE;
	  save_char_obj (ch);
	  make_backup_duel = TRUE;
	  save_char_obj (to_duel);
	  location = get_random_arena (ch);
	  char_from_room (ch);
	  char_to_room (ch, location);
	  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	  trap_check(ch,"room",ch->in_room,NULL);
	  sprintf(tcbuf,"%d",ch->in_room->vnum);
	  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
	  check_aggression(ch);
	  location = get_random_arena (to_duel);
	  char_from_room (to_duel);
	  char_to_room (to_duel, location);
	  trip_triggers(to_duel, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	  trap_check(to_duel,"room",to_duel->in_room,NULL);
	  sprintf(tcbuf,"%d",to_duel->in_room->vnum);
	  trip_triggers_arg(to_duel, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
	  check_aggression(to_duel);
	  current_duel =
	    (struct duel_data *) malloc (sizeof (struct duel_data));
	  current_duel->old_carry_1 = NULL;
	  current_duel->old_carry_2 = NULL;
	  current_duel->in_duel_1 = ch;
	  current_duel->old_hit_1 = ch->hit;
	  current_duel->old_mana_1 = ch->mana;
	  current_duel->old_move_1 = ch->move;
	  for (obj = ch->carrying; obj; obj = obj_next)
	    {
	      obj_next = obj->next_content;
	      obj->prev_wear_loc = obj->wear_loc;
	      unequip_char (ch, obj);
	      obj_from_char (obj);
	      obj->next_content = current_duel->old_carry_1;
	      current_duel->old_carry_1 = obj;
	    }
	  current_duel->in_duel_2 = to_duel;
	  current_duel->old_hit_2 = to_duel->hit;
	  current_duel->old_mana_2 = to_duel->mana;
	  current_duel->old_move_2 = to_duel->move;
	  for (obj = to_duel->carrying; obj; obj = obj_next)
	    {
	      obj_next = obj->next_content;
	      obj->prev_wear_loc = obj->wear_loc;
	      unequip_char (to_duel, obj);
	      obj_from_char (obj);
	      obj->next_content = current_duel->old_carry_2;
	      current_duel->old_carry_2 = obj;
	    }
	  ch->hit = ch->max_hit;
	  ch->mana = ch->max_mana;
	  ch->move = ch->max_move;
	  to_duel->hit = to_duel->max_hit;
	  to_duel->mana = to_duel->max_mana;
	  to_duel->move = to_duel->max_move;
	  level_avg = (ch->level + to_duel->level) / 2;
	  arena_equip (ch, level_avg);
	  arena_equip (to_duel, level_avg);
	  return;
	}
      for (tch = char_list; tch != NULL; tch = tch_next)
	{
	  tch_next = tch->next;
	  if (tch->duel == to_duel->name && tch->duel != tch->name)
	    {
	      sprintf (tcbuf,
		       "Sorry, %s has already been challenged to a duel!\n\r",
		       to_duel->name);
	      send_to_char (tcbuf, ch);
	      return;
	    }
	}
      sprintf (tcbuf, "You have challenged %s to a duel!\n\r",
	       capitalize (to_duel->name));
      send_to_char (tcbuf, ch);
      sprintf (tcbuf,
	       "%s has challenged you to a duel, type 'duel %s' in the Hall of Legends to accept!\n\r",
	       capitalize (ch->name), capitalize (ch->name));
      send_to_char (tcbuf, to_duel);
      ch->duel = to_duel->name;
    }
  else
    {
      send_to_char ("\n\r", ch);
      send_to_char
	("     `c/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\``\n\r",
	 ch);
      send_to_char
	("    `c|                                                 |    |``\n\r",
	 ch);
      send_to_char
	("    `c|       `n-`k=    `oCURRENT DUEL INFORMATION   `k=`n-       `c|___/``\n\r",
	 ch);
      send_to_char
	("    `c|                                                 |\n\r", ch);
      send_to_char
	("    `c|                                                 |``\n\r",
	 ch);
      if (current_duel != NULL)
	{

	  /*      sprintf(tcbuf, "Duel in progress: %13s vs %-13s!\n\r", 
	     current_duel->in_duel_1->name, 
	     current_duel->in_duel_2->name);
	   */
	  send_to_char
	    ("    `c|``               `o* In Progress * ``                  `c|``\n\r",
	     ch);
	  sprintf (tcbuf,
		   "    `c|``        `k%13s vs %-13s``           `c|``\n\r",
		   current_duel->in_duel_1->name,
		   current_duel->in_duel_2->name);
	  send_to_char (tcbuf, ch);
	  send_to_char
	    ("    `c|``                                                 `c|``\n\r",
	     ch);
	}
      for (tch = char_list; tch != NULL; tch = tch_next)
	{
	  tch_next = tch->next;
	  if (!IS_NPC (tch))
	    {
	      if (tch->duel[0] != '\0')
		{
		  if (tch->duel == tch->name)
		    sprintf (tcbuf,
			     "    `c|```n%13s is awaiting any challenger.``        `c|``\n\r",
			     capitalize (tch->name));

		  else
		    sprintf (tcbuf,
			     "    `c|``    `n%13s is awaiting `n%-13s``      `c|``\n\r",
			     tch->name, tch->duel);
		  send_to_char (tcbuf, ch);
		  tch_count++;
		}
	    }
	}
      if (tch_count == 0)
	{
	  send_to_char
	    ("    `c|                                                 |``\n\r",
	     ch);
	  send_to_char
	    ("    `c|``      `jNo players are awaiting challengers.       `c|``\n\r",
	     ch);
	  send_to_char
	    ("    `c|``                                                 `c|``\n\r",
	     ch);
	  send_to_char
	    ("    `c|``                                                 `c|``\n\r",
	     ch);
	}
      send_to_char
	("    `c|``                                                 `c|``\n\r",
	 ch);
      send_to_char
	("    `c|``                                                 `c|``\n\r",
	 ch);
      send_to_char
	("    `c|``                                                 `c|``\n\r",
	 ch);

      /*      for(tch = char_list; tch != NULL; tch = tch_next)
         {
         tch_next = tch->next;
         if(!IS_NPC(tch) && tch != ch)
         if(tch->duel == ch->name)
         {
         sprintf(tcbuf, "`n%s has challenged you.``\n\r", capitalize(tch->name));
         send_to_char(tcbuf, ch);
         break;
         }
         } 

         if(ch->duel[0] == '\0')
         strcpy(tcbuf, "`c|``    `jYou are currently challenging no one.``      `c|``\n\r");
         else if(ch->duel == ch->name)
         strcpy(tcbuf, "`c|``    `jYou are on the public duel list.       `c|``\n\r");
         else
         sprintf(tcbuf, "`c|``    `nYou are currently challenging `n%s.     `c|``\n\r\n\r", ch->duel);

         send_to_char(tcbuf, ch); */
      send_to_char
	("  `c/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |\n\r", ch);
      send_to_char
	(" `c|                                               |    |\n\r", ch);
      send_to_char
	("  `c\\_______________________________________________\\__/``\n\r",
	 ch);
    }
  return;
}

void arena_report (char *spectator_stuff)
{
  DESCRIPTOR_DATA *ps;
  for (ps = descriptor_list; ps != NULL; ps = ps->next)
    {
      CHAR_DATA *victim;
      victim = ps->original ? ps->original : ps->character;
      if (ps->connected == CON_PLAYING
	  && IS_SET (victim->in_room->room_flags, ROOM_SPECTATOR))
	{
	  send_to_char (spectator_stuff, victim);
	}
    }
}
void arena_equip (CHAR_DATA * ch, int level_avg)
{
  OBJ_DATA *arena_obj;
  int best, objvnum, curr;
  if (IS_NPC (ch))
    return;
  best = 0;
  objvnum = OBJ_VNUM_APOLEARM;
  if ((curr = get_skill (ch, gsn_polearm)) > best)
    best = curr;
  if ((curr = get_skill (ch, gsn_staff)) > best)
    {
      best = curr;
      objvnum = OBJ_VNUM_ASTAFF;
    }
  if ((curr = get_skill (ch, gsn_mace)) > best)
    {
      best = curr;
      objvnum = OBJ_VNUM_AMACE;
    }
  if ((curr = get_skill (ch, gsn_dagger)) > best)
    {
      best = curr;
      objvnum = OBJ_VNUM_ADAGGER;
    }
  if ((curr = get_skill (ch, gsn_sword)) > best || IS_CLASS (ch, PC_CLASS_REAVER))
    {
      best = curr;
      objvnum = OBJ_VNUM_ASWORD;
    }
  if ((curr = get_skill (ch, gsn_hand_to_hand)) <= best
      && !(IS_CLASS (ch, PC_CLASS_MONK)))
    {
      arena_obj = create_object (get_obj_index (objvnum), level_avg);
      arena_obj->level = level_avg;
      obj_to_char (arena_obj, ch);
      if (ch->pcdata->primary_hand == HAND_RIGHT
	  || ch->pcdata->primary_hand == HAND_AMBIDEXTROUS)
	equip_char (ch, arena_obj, WEAR_WIELD_R);

      else
	equip_char (ch, arena_obj, WEAR_WIELD_L);
    }
  if (!IS_CLASS (ch, PC_CLASS_ASSASSIN))
    {
      arena_obj = create_object (get_obj_index (OBJ_VNUM_ASHIELD), level_avg);
      arena_obj->level = level_avg;
      obj_to_char (arena_obj, ch);
      equip_char (ch, arena_obj, WEAR_SHIELD);
    }
  arena_obj = create_object (get_obj_index (OBJ_VNUM_ABOOTS), level_avg);
  arena_obj->level = level_avg;
  obj_to_char (arena_obj, ch);
  equip_char (ch, arena_obj, WEAR_FEET);
  arena_obj = create_object (get_obj_index (OBJ_VNUM_AGLOVES), level_avg);
  arena_obj->level = level_avg;
  obj_to_char (arena_obj, ch);
  equip_char (ch, arena_obj, WEAR_HANDS);
  arena_obj = create_object (get_obj_index (OBJ_VNUM_ALIGHT), level_avg);
  arena_obj->level = level_avg;
  obj_to_char (arena_obj, ch);
  equip_char (ch, arena_obj, WEAR_LIGHT);
  arena_obj = create_object (get_obj_index (OBJ_VNUM_ASLEEVES), level_avg);
  arena_obj->level = level_avg;
  obj_to_char (arena_obj, ch);
  equip_char (ch, arena_obj, WEAR_ARMS);
  arena_obj = create_object (get_obj_index (OBJ_VNUM_ABELT), level_avg);
  arena_obj->level = level_avg;
  obj_to_char (arena_obj, ch);
  equip_char (ch, arena_obj, WEAR_WAIST);
  if (IS_CLASS (ch, PC_CLASS_BARD))
    {
      arena_obj = create_object (get_obj_index (OBJ_VNUM_AINST), level_avg);
      arena_obj->level = level_avg;
      obj_to_char (arena_obj, ch);
      equip_char (ch, arena_obj, WEAR_HOLD);
    }
  arena_obj = create_object (get_obj_index (OBJ_VNUM_ALEGGINGS), level_avg);
  arena_obj->level = level_avg;
  obj_to_char (arena_obj, ch);
  equip_char (ch, arena_obj, WEAR_LEGS);
}

int is_dueling (CHAR_DATA * chk_duel)
{
  if (current_duel != NULL)
    if (chk_duel == current_duel->in_duel_1
	|| chk_duel == current_duel->in_duel_2)
      return (1);
  return (0);
}

void duel_cancel (CHAR_DATA * who_god)
{
  char tcbuf[MAX_STRING_LENGTH];
  OBJ_DATA *obj, *obj_next;
  ROOM_INDEX_DATA *location;
  sprintf (tcbuf, "The duel has been cancelled by %s.\n\r", who_god->name);
  send_to_char (tcbuf, current_duel->in_duel_1);
  send_to_char (tcbuf, current_duel->in_duel_2);
  for (obj = current_duel->in_duel_1->carrying; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      unequip_char (current_duel->in_duel_1, obj);
      obj_from_char (obj);
      extract_obj (obj);
    }
  for (obj = current_duel->in_duel_2->carrying; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      unequip_char (current_duel->in_duel_2, obj);
      obj_from_char (obj);
      extract_obj (obj);
    }
  current_duel->in_duel_1->hit = current_duel->old_hit_1;
  current_duel->in_duel_1->mana = current_duel->old_mana_1;
  current_duel->in_duel_1->move = current_duel->old_move_1;
  current_duel->in_duel_1->position = POS_STANDING;
  for (obj = current_duel->old_carry_1; obj; obj = obj_next)
    {
      obj_next = obj->next_content;
      obj_to_char (obj, current_duel->in_duel_1);
      equip_char_slot_new (current_duel->in_duel_1, obj, obj->prev_wear_loc);
    }
  current_duel->in_duel_2->hit = current_duel->old_hit_2;
  current_duel->in_duel_2->mana = current_duel->old_mana_2;
  current_duel->in_duel_2->move = current_duel->old_move_2;
  current_duel->in_duel_2->position = POS_STANDING;
  for (obj = current_duel->old_carry_2; obj; obj = obj_next)
    {
      obj_next = obj->next_content;
      obj_to_char (obj, current_duel->in_duel_2);
      equip_char_slot_new (current_duel->in_duel_2, obj, obj->prev_wear_loc);
    }
  location = get_room_index (ROOM_VNUM_SPECTATOR);
  char_from_room (current_duel->in_duel_1);
  char_to_room (current_duel->in_duel_1, location);
  trip_triggers(current_duel->in_duel_1, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  trap_check(current_duel->in_duel_1,"room",current_duel->in_duel_1->in_room,NULL);
  sprintf(tcbuf,"%d",current_duel->in_duel_1->in_room->vnum);
  trip_triggers_arg(current_duel->in_duel_1, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
  check_aggression(current_duel->in_duel_1);
  location = get_room_index (ROOM_VNUM_HALL_LEGENDS);
  char_from_room (current_duel->in_duel_2);
  char_to_room (current_duel->in_duel_2, location);
  trip_triggers(current_duel->in_duel_2, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  trap_check(current_duel->in_duel_2,"room",current_duel->in_duel_2->in_room,NULL);
  sprintf(tcbuf,"%d",current_duel->in_duel_2->in_room->vnum);
  trip_triggers_arg(current_duel->in_duel_2, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
  check_aggression(current_duel->in_duel_2);
  free (current_duel);
  current_duel = NULL;
}

void duel_ends (CHAR_DATA * loser)
{
  CHAR_DATA *winner, *spectator;
  ROOM_INDEX_DATA *location;
  OBJ_DATA *obj, *obj_next;
  char tcbuf[MAX_STRING_LENGTH];
  sh_int xx;
  stop_fighting (loser, TRUE);
  send_to_char ("The duel is over!\n\r", current_duel->in_duel_1);
  send_to_char ("The duel is over!\n\r", current_duel->in_duel_2);
  if (current_duel->in_duel_1 == loser)
    winner = current_duel->in_duel_2;

  else
    winner = current_duel->in_duel_1;
  send_to_char ("You are the WINNER!\n\r", winner);
  send_to_char ("You are the LOSER!\n\r", loser);

  /* 09-18-03 Iblis - Added new kc elements */

  winner->pcdata->has_killed[ARENA_KILL]++;
  loser->pcdata->been_killed[ARENA_KILL]++;
  
  for (obj = winner->carrying; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      unequip_char (winner, obj);
      obj_from_char (obj);
      extract_obj (obj);
    }
  for (obj = loser->carrying; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      unequip_char (loser, obj);
      obj_from_char (obj);
      extract_obj (obj);
    }
  current_duel->in_duel_1->hit = current_duel->old_hit_1;
  current_duel->in_duel_1->mana = current_duel->old_mana_1;
  current_duel->in_duel_1->move = current_duel->old_move_1;
  current_duel->in_duel_1->position = POS_STANDING;
  location = get_room_index (ROOM_VNUM_SPECTATOR);
  spectator = location->people;
  char_from_room (winner);
  char_to_room (winner, location);
  trip_triggers(winner, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  trap_check(winner,"room",winner->in_room,NULL);
  sprintf(tcbuf,"%d",winner->in_room->vnum);
  trip_triggers_arg(winner, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
  check_aggression(winner);
  location = get_room_index (ROOM_VNUM_HALL_LEGENDS);
  char_from_room (loser);
  char_to_room (loser, location);
  trip_triggers(loser, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  trap_check(loser,"room",loser->in_room,NULL);
  sprintf(tcbuf,"%d",loser->in_room->vnum);
  trip_triggers_arg(loser, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
  check_aggression(loser);
  for (obj = current_duel->old_carry_1; obj; obj = obj_next)
    {
      obj_next = obj->next_content;
      obj_to_char (obj, current_duel->in_duel_1);
      equip_char_slot_new (current_duel->in_duel_1, obj, obj->prev_wear_loc);
    }
  current_duel->in_duel_2->hit = current_duel->old_hit_2;
  current_duel->in_duel_2->mana = current_duel->old_mana_2;
  current_duel->in_duel_2->move = current_duel->old_move_2;
  current_duel->in_duel_2->position = POS_STANDING;
  for (obj = current_duel->old_carry_2; obj; obj = obj_next)
    {
      obj_next = obj->next_content;
      obj_to_char (obj, current_duel->in_duel_2);
      equip_char_slot_new (current_duel->in_duel_2, obj, obj->prev_wear_loc);
    }
  current_duel->old_carry_1 = NULL;
  current_duel->old_carry_2 = NULL;
  free (current_duel);
  current_duel = NULL;
  duel_record (winner->name, loser->name);
  for (xx = 0; spectator != NULL; xx++)
    spectator = spectator->next_in_room;
  winner->gold += (xx * 2);
  sprintf (tcbuf,
	   "You are awarded your cut of the ticket sales, %d gold coins!\n\r",
	   xx * 2);
  send_to_char (tcbuf, winner);
  send_to_char
    ("You are instantly transported to the arena spectator's room.\n\r",
     winner);
  act ("$n arrives, $s fists raised in victory.", winner, NULL, NULL,
       TO_ROOM);
  send_to_char
    ("You are instantly transported to the Hall of Legends.\n\r", loser);
  act ("$n arrives, hanging $s head in shame.", loser, NULL, NULL, TO_ROOM);
  award_item (winner, loser);
  unlink_duel_backups (loser, winner);
  do_stripaffects(loser,loser->name);
  do_stripaffects(winner,winner->name);
  if (!IS_NPC(loser))
    loser->pcdata->aggression = 0;
  if (!IS_NPC(winner))
     winner->pcdata->aggression = 0;
	  
}

ROOM_INDEX_DATA *get_random_arena (CHAR_DATA * ch)
{
  ROOM_INDEX_DATA *room;
  for (;;)
    {
      room = get_room_index (number_range (23001, 23020));
      if (room != NULL)
//      if (IS_SET (room->room_flags, ROOM_ARENA))
	break;
    }
  return room;
}

int duel_compare (const struct duel_info *i, const struct duel_info *j)
//int duel_compare (void *i, void *j)   
{
  int icnt = 0, jcnt = 0;
  if ((i->wins * (i->wins + i->losses)) -
      (i->losses * (i->wins + i->losses)) >
      (j->wins * (j->wins + j->losses)) - (j->losses * (j->wins + j->losses)))
    icnt += 3;
  if ((i->wins * (i->wins + i->losses)) -
      (i->losses * (i->wins + i->losses)) <
      (j->wins * (j->wins + j->losses)) - (j->losses * (j->wins + j->losses)))
    jcnt += 3;
  if (i->per > j->per)
    icnt += 2;
  if (i->per < j->per)
    jcnt += 2;
  if (i->per == 0)
    icnt -= 2;
  if (j->per == 0)
    jcnt -= 2;
  if (icnt > jcnt)
    return (1);
  if (icnt < jcnt)
    return (-1);
  return (0);
}

void show_duel_record (CHAR_DATA * ch)
{
  struct duel_info *duel_array;
  struct stat sbuf;
  int afd, xx = 0, cnt = 1;
  char tcbuf[MAX_STRING_LENGTH];
  if (stat ("duel_record", &sbuf) == -1)
    {
      send_to_char
	("               `i___________________   ___________________``\n\r",
	 ch);
      send_to_char
	("           `b.-/`i|  `e45   `l~`g~`k**`g~`l~      `m\\ /``      `l~`g~`k**`g~`l~   `e46  `i|`b\\-.``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i| `l<`g= `kRank & Hero `g=`l>  `b: `l<`g= `kWins & Losses `g=`l>`i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b||`m/`i====================`m\\`b:`m/`i====================`m\\`b||``\n\r",
	 ch);
      send_to_char
	("            `m==`b-------------------`m~`b___`m~`b--------------------`m=``\n\r",
	 ch);
      return;
    }
  if (sbuf.st_size <= 0)
    return;
  duel_array = (struct duel_info *) malloc (sbuf.st_size);
  if (duel_array == NULL)
    return;
  afd = open ("duel_record", O_RDONLY);
  if (afd < 1)
    {
      send_to_char
	("               `i___________________   ___________________``\n\r",
	 ch);
      send_to_char
	("           `b.-/`i|  `e45   `l~`g~`k**`g~`l~      `m\\ /``      `l~`g~`k**`g~`l~   `e46  `i|`b\\-.``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i| `l<`g= `kRank & Hero `g=`l>  `b: `l<`g= `kWins & Losses `g=`l>`i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
	 ch);
      send_to_char
	("           `b|||`i|                    `b:                    `i|`b|||\n\r",
	 ch);
      send_to_char
	("           `b||`m/`i====================`m\\`b:`m/`i====================`m\\`b||``\n\r",
	 ch);
      send_to_char
	("          `m==`b--------------------`m~`b___`m~`b--------------------`m==``\n\r",
	 ch);
      return;
    }
  while (read (afd, &duel_array[xx], sizeof (struct duel_info)) > 0)
    xx++;
  qsort ((struct duel_info *) duel_array, xx, sizeof (struct duel_info),
	 (int(*)(const void *,const void *)) duel_compare);
  send_to_char
    ("               `i___________________   ___________________``\n\r", ch);
  send_to_char
    ("           `b.-/`i|  `e45   `l~`g~`k**`g~`l~      `m\\ /``      `l~`g~`k**`g~`l~   `e46  `i|`b\\-.``\n\r",
     ch);
  send_to_char
    ("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
     ch);
  send_to_char
    ("           `b|||`i| `l<`g= `kRank & Hero `g=`l>  `b: `l<`g= `kWins & Losses `g=`l>`i|`b|||``\n\r",
     ch);
  send_to_char
    ("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
     ch);
  for (xx--; xx >= 0; xx--)
    {
      if (cnt == 10)
	sprintf (tcbuf,
		 "           `b|||`i| `g{`o%d`g}`` `o%-13s`` `b:``    `l%3d   %3d       `i|`b|||``\n\r",
		 cnt, duel_array[xx].name, duel_array[xx].wins,
		 duel_array[xx].losses);

      else
	sprintf (tcbuf,
		 "           `b|||`i| `g{`o%d`g}``  `o%-13s`` `b:``    `l%3d   %3d       `i|`b|||``\n\r",
		 cnt, duel_array[xx].name, duel_array[xx].wins,
		 duel_array[xx].losses);
      send_to_char (tcbuf, ch);
      if (cnt == 10)
	break;
      cnt++;
    }
  for (; cnt < 6; cnt++)
    send_to_char
      ("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
       ch);
  send_to_char
    ("           `b|||`i|                    `b:                    `i|`b|||``\n\r",
     ch);
  send_to_char
    ("           `b||`m/`i====================`m\\`b:`m/`i====================`m\\`b||\n\r",
     ch);
  send_to_char
    ("           `m==`b--------------------`m~`b___`m~`b--------------------`m==``\n\r",
     ch);
  close (afd);
}

void duel_record (char *winner, char *loser)
{
  bool win_find = FALSE, lose_find = FALSE;
  struct duel_info dinfo;
  int afd;
  afd = open ("duel_record", O_CREAT | O_RDWR, 00644);
  if (afd < 1)
    return;
  while (read (afd, &dinfo, sizeof (struct duel_info)) > 0)
    {
      if (win_find == FALSE && !str_cmp (dinfo.name, winner))
	{
	  lseek (afd, (0 - sizeof (struct duel_info)), SEEK_CUR);
	  dinfo.wins++;
	  if (dinfo.losses == 0)
	    dinfo.per = 100.0;

	  else if (dinfo.wins == 0)
	    dinfo.per = 0.0;

	  else
	    dinfo.per =
	      ((float) dinfo.wins /
	       ((float) dinfo.losses + (float) dinfo.wins)) * 100.000;
	  write (afd, &dinfo, sizeof (struct duel_info));
	  win_find = TRUE;
	}
      else if (lose_find == FALSE && !str_cmp (dinfo.name, loser))
	{
	  lseek (afd, (0 - sizeof (struct duel_info)), SEEK_CUR);
	  dinfo.losses++;
	  if (dinfo.losses == 0)
	    dinfo.per = 100.0;

	  else if (dinfo.wins == 0)
	    dinfo.per = 0.0;

	  else
	    dinfo.per =
	      ((float) dinfo.wins /
	       ((float) dinfo.losses + (float) dinfo.wins)) * 100.0;
	  write (afd, &dinfo, sizeof (struct duel_info));
	  lose_find = TRUE;
	}
      if (win_find && lose_find)
	break;
    }
  lseek (afd, 0L, SEEK_END);
  if (!win_find)
    {
      strcpy (dinfo.name, winner);
      dinfo.wins = 1;
      dinfo.losses = 0;
      dinfo.per = 100.0;
      write (afd, &dinfo, sizeof (struct duel_info));
    }
  if (!lose_find)
    {
      strcpy (dinfo.name, loser);
      dinfo.wins = 0;
      dinfo.losses = 1;
      dinfo.per = 0.0;
      write (afd, &dinfo, sizeof (struct duel_info));
    }
  close (afd);
}

//Iblis - New Dueling function, "Super Duel"
void do_superduel(CHAR_DATA* ch, char* argument)
{
  char buf[MAX_STRING_LENGTH];
  if (ch->level < 10)
  {
    send_to_char ("You are too little to super duel!\n\r",ch);
    return;
  }
  if (IS_IMMORTAL(ch))
  {
    send_to_char ("BAD IMM NO!\n\r",ch);
    sprintf(buf,"Super duel tick counter = %d\n\r",superduel_ticks);
    send_to_char(buf,ch);
    return;
  }
  if (IS_NPC(ch))
  {
    send_to_char ("BAD NPC NO!\n\r",ch);
    return;
  }
  if (ch->in_room->vnum != ROOM_VNUM_SD_START)
  {
    send_to_char ("The entrance to the super duel arena is not here..\n\r",ch);
    return;
  }

  if (ch->position == POS_MOUNTED)
  {
    send_to_char ("Please dismount before entering the super duel arena.\n\r",ch);
    return;
  }
	        
  if ((time (NULL) - ch->pcdata->last_fight < 120))
  {
    send_to_char ("You can't enter the super duel right now.\n\r", ch);
    return;
  }
  
  if (superduel_ticks < 0 || superduel_ticks > 3)
  {
    send_to_char("A super duel is already in progress, please wait until it is over.\n\r",ch);
    return;
  }
  if (battle_royale)
  {
    send_to_char("No super dueling during Battle Royale.  There is enough carnage already.\n\r",ch);
    return;
  }
  //IF FIGHT_LAG, don't let them do it
//  do_goto(ch,"23028");

    ch->pcdata->sd_team = SD_NO_TEAM;
    if (!strcmp(argument,"red"))
      ch->pcdata->sd_team = SD_TEAM_RED;
    else if (!strcmp(argument,"blue"))
      ch->pcdata->sd_team = SD_TEAM_BLUE;
    else if (!strcmp(argument,"green"))
      ch->pcdata->sd_team = SD_TEAM_GREEN;
    else if (!strcmp(argument,"yellow"))
      ch->pcdata->sd_team = SD_TEAM_YELLOW;
    if (superduel_ticks != 0 && superduel_teams && ch->pcdata->sd_team == SD_NO_TEAM)
    {
       send_to_char("`oThis is a team superduel.  You must join a team via `ksuperduel TEAM`` where TEAM is\n\r",ch);
       send_to_char("`bred`o, `eblue`o, `cgreen`o, or `kyellow`o.\n\r",ch);
       return;
    }
       
  act ("$n is transported to the waiting room, awaiting $s chance to fight in a super duel.", ch, NULL, NULL, TO_ROOM);
  char_from_room (ch);
  char_to_room (ch, find_location (ch, "23055"));
  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  trap_check(ch,"room",ch->in_room,NULL);
  sprintf(buf,"%d",ch->in_room->vnum);
  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,buf);
//  check_aggression(ch);
  //MAKE THE ABOVE room noquit, norecall, noteleport, and have a down path to DSC
  //set some flag saying they're waiting to go in
  //changed that..made it so after 5 ticks everyone in the room is transfered into the arena..
  //If only one person is transfered in, they are made fun of on OOC
  if (superduel_ticks == 0)
  {
    superduel_ticks = 3;
    if (ch->pcdata->sd_team != SD_NO_TEAM)
    {
      superduel_teams = TRUE;
      sprintf(buf,"A new TEAM superduel has been announced by `l%s``!",ch->name);
      arena_chan(buf);
    }
    else 
    {
      superduel_teams = FALSE;
      sprintf(buf,"A new superduel has been announced by `l%s``!",ch->name);
      arena_chan(buf);
    } 
  }
  else if (ch->pcdata->sd_team == SD_NO_TEAM) 
  {
    sprintf(buf,"`l%s`o has joined the superduel.",ch->name);
    arena_chan(buf);
  }
  if (ch->pcdata->sd_team != SD_NO_TEAM)
  {
    switch (ch->pcdata->sd_team)
    {
      case SD_TEAM_BLUE:
        sprintf(buf, "`l%s`o has joined the superduel for team `eBlue`o.",ch->name);
	break;
      case SD_TEAM_YELLOW:
        sprintf(buf, "`l%s`o has joined the superduel for team `kYellow`o.",ch->name);
        break;
      case SD_TEAM_RED:
        sprintf(buf, "`l%s`o has joined the superduel for team `bRed`o.",ch->name);
        break;
      case SD_TEAM_GREEN:
        sprintf(buf, "`l%s`o has joined the superduel for team `cGreen`o.",ch->name);
        break;
    }
    arena_chan(buf);
  }
  sprintf(buf,"There are %i ticks remaining til the superduel begins.\n\r",superduel_ticks);
  send_to_char(buf,ch);
}

//Iblis - Called when someone super dueling gets killed
void superduel_ends(CHAR_DATA *winner, CHAR_DATA *loser)
{
  char buf[MAX_STRING_LENGTH];
  char lteam[MAX_STRING_LENGTH], wteam[MAX_STRING_LENGTH];
  CHAR_DATA* chpointer;
  bool teams_done;
  int counter;
  if (superduel_teams)
  {
    if (!IS_NPC(loser))
    {
      switch (loser->pcdata->sd_team)
      {
        case SD_TEAM_BLUE: sprintf(lteam,"`e%s of team Blue",loser->name);break;
        case SD_TEAM_RED: sprintf(lteam,"`b%s of team Red",loser->name);break;
        case SD_TEAM_YELLOW: sprintf(lteam,"`k%s of team Yellow",loser->name);break;
        case SD_TEAM_GREEN: sprintf(lteam,"`c%s of team Green",loser->name);break;
      }
    }
    else sprintf(lteam, "`f%s of team NPC",loser->short_descr);

    if (!IS_NPC(winner))
    {
      switch (winner->pcdata->sd_team)
      {
        case SD_TEAM_BLUE: sprintf(wteam,"`e%s of team Blue",winner->name);break;
        case SD_TEAM_RED: sprintf(wteam,"`b%s of team Red",winner->name);break;
        case SD_TEAM_YELLOW: sprintf(wteam,"`k%s of team Yellow",winner->name);break;
        case SD_TEAM_GREEN: sprintf(wteam,"`c%s of team Green",winner->name);break;
      }
    }
    else sprintf(wteam, "`f%s of team NPC",winner->short_descr);
  }
  else
  {
    sprintf(lteam,"%s",(IS_NPC(loser))?loser->short_descr:loser->name);
    sprintf(wteam,"%s",(IS_NPC(winner))?winner->short_descr:winner->name);
  }
/*  counter = number_range(1,6);
  switch (counter)
  {
    case 1 : sprintf(buf,"`l%s `ojust `bslaughtered `a%s `oduring a super duel!``",wteam,lteam); break;
    case 2 : sprintf(buf,"`l%s `ohas just been `btoasted`o by `a%s `oduring a super duel!``",lteam,wteam); break;
    case 3 : sprintf(buf,"`l%s `ojust `bmurdered `a%s `oduring a super duel!``",wteam,lteam); break;
    case 4 : sprintf(buf,"`oSeek vengeance! `l%s `ojust `bdestroyed `a%s `oduring a super duel!``",wteam,lteam); break;
    case 5 : sprintf(buf,"`oALAS!  `l%s `ohas just removed `a%s `ofrom the super duel!``",wteam,lteam); break;
    default : sprintf(buf,"`l%s `ojust got `bcreamed`o by `a%s `oduring a super duel!``",lteam,wteam); break;
  }
  counter = 0;*/
  if (winner->hit >= winner->max_hit)
    sprintf(buf,"`oSeek vengeance!  `l%s `ojust performed a FLAWLESS victory over `a%s`o in a super duel!``",wteam,lteam);
  else if (winner->hit >= winner->max_hit*.75)
    sprintf(buf,"`l%s `ojust `bcreamed `a%s`o during a super duel!``",wteam,lteam);
  else if (winner->hit >= winner->max_hit*.5)
    sprintf(buf,"`l%s `ojust `bslaughtered `a%s`o during a super duel!``",wteam,lteam);
  else if (winner->hit >= winner->max_hit*.4)
    sprintf(buf,"`l%s `ohas `bdestroyed `a%s`o during a super duel!``",wteam,lteam);	   
  else if (winner->hit >= winner->max_hit*.3)
    sprintf(buf,"`l%s `ojust `bmurdered `a%s`o during a super duel!``",wteam,lteam);
  else if (winner->hit >= winner->max_hit*.2)
    sprintf(buf,"`l%s `ohas toasted `a%s`o during a super duel!``",wteam,lteam);
  else if (winner->hit >= winner->max_hit*.1)
    sprintf(buf,"`l%s `ohas just removed `a%s`o from the super duel!``",wteam,lteam);
  else sprintf(buf,"`l%s `ojust barely beat `a%s`o during a super duel.``",wteam,lteam);
  counter = 0; 
  arena_chan(buf);
  if (IS_NPC(loser))
  {
    loser->hit = loser->max_hit;
    loser->mana = loser->max_mana;
    loser->move = loser->max_move;
  }
  else
  {
    loser->pcdata->sd_team = SD_NO_TEAM;
    loser->hit = loser->pcdata->backup_hit;
    loser->mana = loser->pcdata->backup_mana;
    loser->move = loser->pcdata->backup_move;
  }
   
  loser->position = POS_RESTING;
  if (!IS_NPC(loser))
  {
    if (loser->pcdata->condition[COND_HUNGER] < 0)
      loser->pcdata->condition[COND_HUNGER] = 0;
    if (loser->pcdata->condition[COND_THIRST] < 0)
      loser->pcdata->condition[COND_THIRST] = 0;
    if (loser->pcdata->familiar)
    {
	    char_from_room(loser->pcdata->familiar);
            char_to_room(loser->pcdata->familiar,winner->in_room);
	    familiar_poof(loser);
    }
    loser->pcdata->aggression = 0;
  }
  do_stripaffects(loser,loser->name);
  sprintf(buf,"%d",ROOM_VNUM_SD_END);
  do_goto(loser,buf);
  loser->dueler = FALSE;
  teams_done = TRUE;
  winner = NULL;
  for (chpointer = char_list;chpointer != NULL;chpointer = chpointer->next)
    {
      if (!IS_NPC(chpointer))
      {
        if (chpointer->dueler)
	{
	  if (superduel_teams && winner != NULL && chpointer->pcdata->sd_team != winner->pcdata->sd_team)
	  {
            teams_done = FALSE;
	  }
  	  winner = chpointer;
//	  do_ooc(chpointer,"MMMMMEEEEE!");
//	  log_string(chpointer->name);
  	  counter++;
	}
      }
    }
  if (superduel_teams && teams_done)
  {
    //extract and clean all the winners
    if (!IS_NPC(winner))
    {
      switch (winner->pcdata->sd_team)
      {
        case SD_TEAM_BLUE: sprintf(wteam,"`eTeam Blue");break;
        case SD_TEAM_RED: sprintf(wteam,"`bTeam Red");break;
        case SD_TEAM_YELLOW: sprintf(wteam,"`kTeam Yellow");break;
        case SD_TEAM_GREEN: sprintf(wteam,"`cTeam Green");break;
      }
    }
    else sprintf(wteam,"`fTeam NPC");
    sprintf(buf,"`oAnd the grand winner of the most recent super duel is -> `b%s`o!",wteam);
    arena_chan(buf);
    if (IS_NPC(winner))
      return;
    for (chpointer = char_list;chpointer != NULL;chpointer = chpointer->next)
    {
      if (!IS_NPC(chpointer))
      {
	if (chpointer->dueler)
	{
          sprintf(buf,"`l%s `owas still alive for %s`o.",chpointer->name,wteam);
	  arena_chan(buf);
	  sprintf(buf,"%d",ROOM_VNUM_SD_END);
	  do_goto(chpointer,buf);
	  superduel_ticks = 0;
	  chpointer->pcdata->events_won[TEAM_SD_WON]++;
          chpointer->hit = chpointer->pcdata->backup_hit;
          chpointer->mana = chpointer->pcdata->backup_mana;
	  chpointer->move = chpointer->pcdata->backup_move;
          if (chpointer->pcdata->condition[COND_HUNGER] < 0)
            chpointer->pcdata->condition[COND_HUNGER] = 0;
          if (chpointer->pcdata->condition[COND_THIRST] < 0)
            chpointer->pcdata->condition[COND_THIRST] = 0;
          if (chpointer->pcdata->familiar)
          {
		  char_from_room(chpointer->pcdata->familiar);
                  char_to_room(chpointer->pcdata->familiar,winner->in_room);
		  familiar_poof(chpointer);
          }
          chpointer->pcdata->aggression = 0;
	  do_stripaffects(chpointer,chpointer->name);
	  if (chpointer != winner)
   	    chpointer->pcdata->sd_team = SD_NO_TEAM;
	  chpointer->dueler = FALSE;
	}
      }
    }
    winner->pcdata->sd_team = SD_NO_TEAM;
  }
  else if (!superduel_teams && counter == 1)
  {
    winner->dueler = FALSE;
    if (!IS_NPC(winner))
      winner->pcdata->sd_team = SD_NO_TEAM;
    sprintf(buf,"%d",ROOM_VNUM_SD_END);
    do_goto(winner,buf);
    sprintf(buf,"`oAnd the grand winner of the most recent super duel is -> `b%s`o!",
		    IS_NPC(winner)?winner->short_descr:winner->name);
    arena_chan(buf);
    superduel_ticks = 0;
    if (!IS_NPC(winner) && winner->pcdata)
      winner->pcdata->events_won[SD_WON]++;
    if (IS_NPC(winner))
    {
      winner->hit = winner->max_hit;
      winner->mana = winner->max_mana;
      winner->move = winner->max_move;
    }
    else
    {
      winner->hit = winner->pcdata->backup_hit;
      winner->mana = winner->pcdata->backup_mana;
      winner->move = winner->pcdata->backup_move;
    }
    if (!IS_NPC(winner))
     {
       if (winner->pcdata->condition[COND_HUNGER] < 0)
         winner->pcdata->condition[COND_HUNGER] = 0;
       if (winner->pcdata->condition[COND_THIRST] < 0)
         winner->pcdata->condition[COND_THIRST] = 0;
       if (winner->pcdata->familiar)
         {
		 char_from_room(winner->pcdata->familiar);
		 char_to_room(winner->pcdata->familiar,winner->in_room);
	   familiar_poof(winner);
         }
       winner->pcdata->aggression = 0;
       
     }
    /*if (IS_AFFECTED (winner, AFF_POISON))
     {
       REMOVE_BIT (winner->affected_by, AFF_POISON);
       affect_strip (winner, gsn_poison);
     }
    if (IS_AFFECTED (winner, AFF_PLAGUE))
     {
       REMOVE_BIT (winner->affected_by, AFF_PLAGUE);
       affect_strip (winner, gsn_plague);
     }
    if (IS_AFFECTED (winner, AFF_BLIND))
     {
       REMOVE_BIT (winner->affected_by, AFF_BLIND);
       affect_strip (winner, gsn_blindness);
     }
    affect_strip(winner,skill_lookup("pox"));
    if (is_affected(winner,skill_lookup("brain blisters")))
    {
      affect_strip(winner,skill_lookup("brain blisters"));
      affect_strip(winner,skill_lookup("disorientation"));
    }
    affect_strip(winner,skill_lookup("vomit"));
    affect_strip(winner,skill_lookup("atrophy"));*/
    do_stripaffects(winner,winner->name);
	    
  }
}

	  
