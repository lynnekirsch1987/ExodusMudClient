#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "merc.h"
#include "obj_trig.h"

extern char str_empty[1];

/* command procedures needed */
DECLARE_DO_FUN (do_look);
DECLARE_DO_FUN (do_stand);

//IBLIS 6/20/03 - Needed to entering of a portal easier
bool room_Class_check args ((CHAR_DATA * ch, ROOM_INDEX_DATA * location));
bool room_race_check args((CHAR_DATA * ch, ROOM_INDEX_DATA * to_room));
bool br_vnum_check args ((int vnum));
bool can_move_char
args ((CHAR_DATA * ch, ROOM_INDEX_DATA * to_room, bool follow,
       bool show_messages));
ROOM_INDEX_DATA *find_location args ((CHAR_DATA * ch, char *arg));

/* random room generation procedure */
ROOM_INDEX_DATA *get_random_room (CHAR_DATA * ch)
{
  ROOM_INDEX_DATA *room;
  for (;;)
    {
      room = get_room_index (number_range (0, 65535));
      if (room != NULL)
	if (can_see_room (ch, room)
	    && !room_is_private (room)
	    && br_vnum_check (room->vnum)
	    && !IS_SET (room->room_flags, ROOM_PRIVATE)
	    && !IS_SET (room->room_flags, ROOM_SOLITARY)
	    && !IS_SET (room->room_flags, ROOM_SAFE)
            && (IS_NPC (ch) || IS_SET (ch->act, ACT_AGGRESSIVE)
                            || !IS_SET (room->room_flags, ROOM_LAW)))
	  break;
    }
  return room;
}

void enter_elevator (CHAR_DATA * ch, OBJ_DATA * elevator)
{
  ROOM_INDEX_DATA *location, *old_room;
  int pnum;
  CHAR_DATA *vch, *vch_next, *fch, *fch_next;
  char tcbuf[15];
  location = get_room_index (elevator->value[0]);
  if ((old_room = ch->in_room) == NULL)
    return;
  if (location == NULL
      /* || location == ch->in_room */
      || !can_see_room (ch, location)
      || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
    {
      act ("$p doesn't seem to go anywhere.", ch, elevator, NULL, TO_CHAR);
      return;
    }
  //Iblis 6/21/03 - Battle Royale room off limit checking
  if (!br_vnum_check (location->vnum))
    {
      send_to_char
	("That room is off limits during `iBattle Royale``\r\n", ch);
      return;
    }
// Minax 2-13-03 Charmies cannot enter into no_charmie rooms
  if (IS_AFFECTED (ch, AFF_CHARM)
      && (IS_SET (location->room_flags, ROOM_NO_CHARMIE)))
    {
      send_to_char ("Charmed folks cannot enter that room.\n\r", ch);
      return;
    }
  if (!can_move_char (ch, location, FALSE, FALSE))
    {
      send_to_char ("Something prevents you from entering.\n\r", ch);
      return;
    }
  adjust_elevator_weight (elevator);
  if (elevator->weight + get_char_weight (ch) + get_carry_weight (ch) >
      elevator->value[2])
    {
      act ("Sorry, $p can't support your weight.", ch, elevator, NULL,
	   TO_CHAR);
      return;
    }
  for (pnum = 0, vch = location->people; vch; vch = vch->next_in_room)
    pnum++;
  if (pnum >= elevator->value[1])
    {
      act ("Sorry, $p is too crowded.", ch, elevator, NULL, TO_CHAR);
      return;
    }
  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
    {
      fch_next = fch->next_in_room;
      if (can_see (fch, ch))
	act ("$n steps into $p.", ch, elevator, fch, TO_VICT);
    }
  act ("You enter $p.", ch, elevator, NULL, TO_CHAR);
  if (ch->mount != NULL)
    {
      char_from_room (ch->mount);
      char_to_room (ch->mount, location);
    }
  char_from_room (ch);
  char_to_room (ch, location);
  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
    {
      fch_next = fch->next_in_room;
      if (can_see (fch, ch))
	act ("$n has arrived.", ch, NULL, fch, TO_VICT);
    }
  do_look (ch, "auto");
  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  trap_check(ch,"room",ch->in_room,NULL);

  sprintf(tcbuf,"%d",ch->in_room->vnum);
  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
  check_aggression(ch);

   trip_triggers(ch, OBJ_TRIG_ACTION_WORD, elevator, NULL, OT_SPEC_ENTER);
  
  for (vch = old_room->people; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;
      if (elevator == NULL)
	continue;
      if (vch->master == ch && IS_AFFECTED (vch, AFF_CHARM)
	  && vch->position < POS_STANDING)
	do_stand (vch, "");
      if (vch->master == ch && vch->position == POS_STANDING)
	{
	  if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
	      && (IS_NPC (vch) && 
		      (IS_SET (vch->act, ACT_AGGRESSIVE)
		       || IS_SET (vch->act2, ACT_AGGIE_GOOD)
		       || IS_SET (vch->act2, ACT_AGGIE_NEUTRAL)
		       || IS_SET (vch->act2, ACT_AGGIE_EVIL))
		       ))
	    {
	      act ("You can't bring $N into the city.", ch, NULL, vch,
		   TO_CHAR);
	      act ("You aren't allowed in the city.", vch, NULL, NULL,
		   TO_CHAR);
	      continue;
	    }
	  //5-10-03 Iblis - Check to remove camoflauge/hide if following someone
	  if (IS_AFFECTED (vch, AFF_CAMOUFLAGE))
	    REMOVE_BIT (vch->affected_by, AFF_CAMOUFLAGE);
	  if (IS_AFFECTED (vch, AFF_HIDE))
	    REMOVE_BIT (vch->affected_by, AFF_HIDE);
	  act ("You follow $N.", vch, NULL, ch, TO_CHAR);
	  enter_elevator (vch, elevator);
	}
    }
}
void enter_ctransport (CHAR_DATA * ch, OBJ_DATA * ctransport)
{
  ROOM_INDEX_DATA *location, *old_room;
  CHAR_DATA *vch, *vch_next, *fch, *fch_next;
  char tcbuf[15];
  location = get_room_index (ctransport->value[2]);
  if ((old_room = ch->in_room) == NULL)
    return;
  if (location == NULL
      /*      || location == ch->in_room */
      || !can_see_room (ch, location)
      || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
    {
      act ("$p doesn't seem to go anywhere.", ch, ctransport, NULL, TO_CHAR);
      return;
    }
  if (IS_SET (ctransport->value[5], CONT_CLOSED))
    {
      send_to_char ("It is closed.\n\r", ch);
      return;
    }
  //Iblis 6/21/03 - Battle Royale room off limit checking
  if (!br_vnum_check (location->vnum))
    {
      send_to_char
	("That room is off limits during `iBattle Royale``\r\n", ch);
      return;
    }
  if (!can_move_char (ch, location, FALSE, FALSE))
    {
      send_to_char ("Something prevents you from entering.\n\r", ch);
      return;
    }
  if (get_char_weight (ch) +
      get_transport_weight (ctransport->value[2]) > ctransport->value[1])
    {
      act ("$p can't support your weight.", ch, ctransport, NULL, TO_CHAR);
      return;
    }
  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
    {
      fch_next = fch->next_in_room;
      if (can_see (fch, ch))
	act ("$n steps into $p.", ch, ctransport, fch, TO_VICT);
    }
  act ("You enter $p.", ch, ctransport, NULL, TO_CHAR);
  if (ch->mount != NULL)
    {
      char_from_room (ch->mount);
      char_to_room (ch->mount, location);
    }
  char_from_room (ch);
  char_to_room (ch, location);
  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
    {
      fch_next = fch->next_in_room;
      if (can_see (fch, ch))
	act ("$n has arrived.", ch, NULL, fch, TO_VICT);
    }
  do_look (ch, "auto");
  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  trap_check(ch,"room",ch->in_room,NULL);

  sprintf(tcbuf,"%d",ch->in_room->vnum);
  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
  check_aggression(ch);

   trip_triggers(ch, OBJ_TRIG_ACTION_WORD, ctransport, NULL, OT_SPEC_ENTER);
  
  for (vch = old_room->people; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;
      if (vch->master == ch && IS_AFFECTED (vch, AFF_CHARM)
	  && vch->position < POS_STANDING)
	do_stand (vch, "");
      if (vch->master == ch && vch->position == POS_STANDING)
	{
	  if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
	      && (IS_NPC (vch) && 
		      (IS_SET (vch->act, ACT_AGGRESSIVE)
		        || IS_SET (vch->act2, ACT_AGGIE_GOOD)
		        || IS_SET (vch->act2, ACT_AGGIE_NEUTRAL)
		        || IS_SET (vch->act2, ACT_AGGIE_EVIL))
		                             ))
		  
	    {
	      act ("You can't bring $N into the city.", ch, NULL, vch,
		   TO_CHAR);
	      act ("You aren't allowed in the city.", vch, NULL, NULL,
		   TO_CHAR);
	      continue;
	    }
	  //5-10-03 Iblis - Check to remove camoflauge/hide if following someone
	  if (IS_AFFECTED (vch, AFF_CAMOUFLAGE))
	    REMOVE_BIT (vch->affected_by, AFF_CAMOUFLAGE);
	  if (IS_AFFECTED (vch, AFF_HIDE))
	    REMOVE_BIT (vch->affected_by, AFF_HIDE);
	  act ("You follow $N.", vch, NULL, ch, TO_CHAR);
	  enter_ctransport (vch, ctransport);
	}
    }
}
void do_board (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Board what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_here (ch, arg)) == NULL)
    {
      send_to_char ("You don't see that here.\n\r", ch);
      return;
    }
  if (obj->item_type != ITEM_CTRANSPORT
      || !(IS_SET (obj->value[4], TACT_SAIL)
	   || IS_SET (obj->value[4], TACT_PADDLE)
	   || IS_SET (obj->value[4], TACT_ROW)))
    {
      send_to_char ("You can't board that.\n\r", ch);
      return;
    }
  board_boat (ch, obj);
}

void board_boat (CHAR_DATA * ch, OBJ_DATA * ctransport)
{
  ROOM_INDEX_DATA *location, *old_room;
  CHAR_DATA *vch, *vch_next, *fch, *fch_next;
  char tcbuf[15];
  location = get_room_index (ctransport->value[2]);
  if ((old_room = ch->in_room) == NULL)
    return;
  if (location == NULL || !can_see_room (ch, location)
      || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
    {
      act ("$p doesn't seem to go anywhere.", ch, ctransport, NULL, TO_CHAR);
      return;
    }
  if (IS_SET (ctransport->value[5], CONT_CLOSED))
    {
      send_to_char ("It is closed.\n\r", ch);
      return;
    }
  //Iblis 6/21/03 - Battle Royale room off limit checking
  if (!br_vnum_check (location->vnum))
    {
      send_to_char
	("That room is off limits during `iBattle Royale``\r\n", ch);
      return;
    }
  if (!can_move_char (ch, location, FALSE, FALSE))
    {
      send_to_char ("Something prevents you from boarding.\n\r", ch);
      return;
    }
  if (get_char_weight (ch) +
      get_transport_weight (ctransport->value[2]) > ctransport->value[1])
    {
      act ("$p can't support your weight.", ch, ctransport, NULL, TO_CHAR);
      return;
    }
  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
    {
      fch_next = fch->next_in_room;
      if (can_see (fch, ch))
	act ("$n boards $p.", ch, ctransport, fch, TO_VICT);
    }
  act ("You board $p.", ch, ctransport, NULL, TO_CHAR);
  if (ch->mount != NULL)
    {
      char_from_room (ch->mount);
      char_to_room (ch->mount, location);
    }
  char_from_room (ch);
  char_to_room (ch, location);
  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
    {
      fch_next = fch->next_in_room;
      if (can_see (fch, ch))
	act ("$n has arrived.", ch, NULL, fch, TO_VICT);
    }
  do_look (ch, "auto");

  trip_triggers(ch, OBJ_TRIG_ACTION_WORD, ctransport, NULL, OT_SPEC_BOARD);
  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  trap_check(ch,"room",ch->in_room,NULL);

  sprintf(tcbuf,"%d",ch->in_room->vnum);
  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
  check_aggression(ch);
 
  for (vch = old_room->people; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;
      if (vch->master == ch && IS_AFFECTED (vch, AFF_CHARM)
	  && vch->position < POS_STANDING)
	do_stand (vch, "");
      if (vch->master == ch && vch->position == POS_STANDING)
	{
	  if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
	      && (IS_NPC (vch) && 
		      (IS_SET (vch->act, ACT_AGGRESSIVE)
		        || IS_SET (vch->act2, ACT_AGGIE_GOOD)
		        || IS_SET (vch->act2, ACT_AGGIE_NEUTRAL)
		        || IS_SET (vch->act2, ACT_AGGIE_EVIL))))
		  
	    {
	      act ("You can't bring $N into the city.", ch, NULL, vch,
		   TO_CHAR);
	      act ("You aren't allowed in the city.", vch, NULL, NULL,
		   TO_CHAR);
	      continue;
	    }
	  //5-10-03 Iblis - Check to remove camoflauge/hide if following someone
	  if (IS_AFFECTED (vch, AFF_CAMOUFLAGE))
	    REMOVE_BIT (vch->affected_by, AFF_CAMOUFLAGE);
	  if (IS_AFFECTED (vch, AFF_HIDE))
	    REMOVE_BIT (vch->affected_by, AFF_HIDE);
	  act ("You follow $N.", vch, NULL, ch, TO_CHAR);
	  board_boat (vch, ctransport);
	}
    }
}
void enter_portal_book (CHAR_DATA * ch, OBJ_DATA * portal)
{
  CHAR_DATA *fch, *fch_next;
  ROOM_INDEX_DATA *location;
  char tcbuf[15];
  if (IS_SET (ch->act, PLR_JAILED))
    return;
  if (portal->item_type != ITEM_PORTAL_BOOK)
    {
      send_to_char ("You can't seem to find a way in.\n\r", ch);
      return;
    }
  if (IS_SET (ch->in_room->race_flags, ROOM_NOTRANSPORT))
    {
      send_to_char
	("You cannot leave this place via ordinary magic.\n\r", ch);
      return;
    }
  if ((location = get_room_index (portal->value[1])) == NULL)
    {
      act ("$p doesn't seem to go anywhere.", ch, portal, NULL, TO_CHAR);
      return;
    }

  //Iblis 6/21/03 - Battle Royale room off limit checking
  if (!br_vnum_check (location->vnum))
    {
      send_to_char
	("That room is off limits during `iBattle Royale``\r\n", ch);
      return;
    }
  if (IS_NPC (ch) && 
		  (IS_SET (ch->act, ACT_AGGRESSIVE)
		    || IS_SET (ch->act2, ACT_AGGIE_GOOD)
                    || IS_SET (ch->act2, ACT_AGGIE_NEUTRAL)
                    || IS_SET (ch->act2, ACT_AGGIE_EVIL))
      && IS_SET (location->room_flags, ROOM_LAW))
    {
      send_to_char ("Something prevents you from entering...\n\r", ch);
      return;
    }
  if (!can_move_char (ch, location, FALSE, FALSE))
    {
      send_to_char ("Something prevents you from entering...\n\r", ch);
      return;
    }
  if (portal->value[0] != -1)
    {
      act ("Sigils upon $p flare to open a gateway that devours $n.", ch,
	   portal, NULL, TO_ROOM);
      act ("Sigils upon $p flare to open a gateway that devours you...",
	   ch, portal, NULL, TO_CHAR);
      portal->value[1] = ch->in_room->vnum;
    }
  char_from_room (ch);
  char_to_room (ch, location);
  if (portal->value[0] != -1)
    {
      portal->value[0]--;
      if (portal->value[0] == -1)
	portal->value[0] = 0;
    }
  if (portal->value[0] == 0)
    {
      act ("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
      extract_obj (portal);
    }
  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
    {
      fch_next = fch->next_in_room;
      if (can_see (fch, ch))
	act ("$n has arrived.", ch, portal, fch, TO_VICT);
    }
  do_look (ch, "auto");

  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  trap_check(ch,"room",ch->in_room,NULL);

  sprintf(tcbuf,"%d",ch->in_room->vnum);
  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
  check_aggression(ch);

  trip_triggers(ch, OBJ_TRIG_ACTION_WORD, portal, NULL, OT_SPEC_ENTER);
}

void enter_portal (CHAR_DATA * ch, char *argument, bool mount_follow)
{
  ROOM_INDEX_DATA *location = NULL,*prev;
  OBJ_DATA *obj;
  char tcbuf[15];

  if (ch->fighting != NULL || IS_SET (ch->act, PLR_JAILED))
    return;

  /* nifty portal stuff */
  if (argument[0] != '\0')
    {
      ROOM_INDEX_DATA *old_room;
      OBJ_DATA *portal;
      CHAR_DATA *fch, *fch_next;
      old_room = ch->in_room;
      portal = get_obj_list (ch, argument, ch->in_room->contents);
      if (portal == NULL)
	{
	  portal = get_obj_list (ch, argument, ch->carrying);
	  if (portal == NULL)
	    {
	      send_to_char ("You don't see that here.\n\r", ch);
	      return;
	    }
	  if (portal->item_type == ITEM_PORTAL_BOOK &&
	      (IS_SET (ch->in_room->race_flags, ROOM_NOTRANSPORT)))
	    {
	      send_to_char
		("You cannot leave this place via ordinary magic.\n\r", ch);
	      return;
	    }
          if (ch->dueler)
            {
              send_to_char ("None of that while super dueling.\n\r", ch);
              return;
            }
	  if (portal->item_type == ITEM_PORTAL_BOOK)
	    {
	      enter_portal_book (ch, portal);
	      return;
	    }
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if (portal->item_type == ITEM_ELEVATOR)
	{
	  enter_elevator (ch, portal);
	  return;
	}
      if (portal->item_type == ITEM_CTRANSPORT)
	{
	  enter_ctransport (ch, portal);
	  return;
	}
      if (!IS_SET (portal->value[4], PACT_ENTER))
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if (portal->item_type != ITEM_PORTAL
	  || (IS_SET (portal->value[1], EX_CLOSED)
	      && !IS_TRUSTED (ch, DEMIGOD)))
	{
	  send_to_char ("You can't seem to find a way in.\n\r", ch);
	  return;
	}
      if (!IS_TRUSTED (ch, DEMIGOD)
	  && !IS_SET (portal->value[2], GATE_NOCURSE)
	  && (IS_AFFECTED (ch, AFF_CURSE)))
	{
	  send_to_char ("Something prevents you from leaving.\n\r", ch);
	  return;
	}
      if (portal->value[3] == PORTAL_FUCKED)
	{

	  send_to_char
	    ("As you enter the portal you feel the molecules of your flesh ripped apart.\n\r",
	     ch);
	  act
	    ("As $n steps into $p $s body is violently shattered and pulled into the void.",
	     ch, portal, NULL, TO_ROOM);

	  sprintf (tcbuf, "%d", number_range (0, 50) + 400);
	  char_from_room (ch);
	  char_to_room (ch, find_location (ch,tcbuf));
	  do_look(ch,"auto");
	  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	  trap_check(ch,"room",ch->in_room,NULL);

	  sprintf(tcbuf,"%d",ch->in_room->vnum);
	  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
	  check_aggression(ch);

	  return;
	}
      if (IS_SET (portal->value[2], GATE_TRANSPORT))
	{
	  for (obj = object_list; obj != NULL; obj = obj->next)
	    {
	      if (!can_see_obj (ch, obj))
		continue;
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->in_room != NULL)
		{
		  location = get_room_index (obj->in_room->vnum);
		  break;
		}
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->carried_by != NULL)
		{
		  location = obj->carried_by->in_room;
		  break;
		}
	    }
	}

      else if (IS_SET (portal->value[2], GATE_RANDOM)
	       || portal->value[3] == -1)
	{
	  location = get_random_room (ch);
	  portal->value[3] = location->vnum;	/* for record keeping :) */
	}

      else if (IS_SET (portal->value[2], GATE_BUGGY)
	       && (number_percent () < 5))
	location = get_random_room (ch);

      else
	location = get_room_index (portal->value[3]);
      if (location == NULL || !can_see_room (ch, location)
	  || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	{
	  act ("$p doesn't seem to go anywhere.", ch, portal, NULL, TO_CHAR);
	  return;
	}
      //Iblis 6/21/03 - Battle Royale room off limit checking
      if (!br_vnum_check (location->vnum))
	{
	  send_to_char
	    ("That room is off limits during `iBattle Royale``\r\n", ch);
	  return;
	}
// Minax 2-13-03 Charmies cannot enter into no_charmie rooms
      if (IS_AFFECTED (ch, AFF_CHARM)
	  && (IS_SET (location->room_flags, ROOM_NO_CHARMIE)))
	{
	  send_to_char ("Charmed folks cannot enter that room.\n\r", ch);
	  return;
	}
      
      if (!is_room_owner (ch, location) && room_is_private (location))
        {
            if (location->max_message[0] == '\0')
              send_to_char ("That room is private right now.\n\r", ch);
            else
            {
              send_to_char (location->max_message, ch);
              send_to_char ("\n\r", ch);
            }
	 return;
	}
      
      if (location->race_flags != 0)
      {
        if (!room_race_check(ch,location))
        {
          act ("You are unable to enter $p.", ch, portal, NULL, TO_CHAR);
	  return;
        }
      }
      if (location->Class_flags != 0)
	{
	  if (!room_Class_check (ch, location))
	    {
	      act ("You are unable to enter $p.", ch, portal, NULL, TO_CHAR);
	      return;
	    }
	}

      /* For haven portals */
      if (location->owner[0] != '\0')
	{
	  CHAR_DATA *owner;
	  owner = get_char_world (ch, location->owner);
	  if ((owner == NULL || !is_same_group (owner, ch))
	      && portal->pIndexData->vnum == 25)
	    {
	      send_to_char
		("You aren't allowed in there since it is not yours.\n\r",
		 ch);
	      return;
	    }
	  save_char_obj (ch);
	}
      if (IS_NPC (ch) && 
		      (IS_SET (ch->act, ACT_AGGRESSIVE)
		        || IS_SET (ch->act2, ACT_AGGIE_GOOD)
                        || IS_SET (ch->act2, ACT_AGGIE_NEUTRAL)
                        || IS_SET (ch->act2, ACT_AGGIE_EVIL))
	  && IS_SET (location->room_flags, ROOM_LAW))
	{
	  send_to_char ("Something prevents you from leaving.\n\r", ch);
	  return;
	}

/*** NEW STUFF ***/
      if (portal->pIndexData->string1[0] != '\0')
	{
	  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	    {
	      fch_next = fch->next_in_room;
	      if (can_see (fch, ch))
		act (portal->pIndexData->string1, ch, portal, fch, TO_VICT);
	    }
	}

      else
	{
	  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	    {
	      fch_next = fch->next_in_room;
	      if (can_see (fch, ch))
		act ("$n steps into $p.", ch, portal, fch, TO_VICT);
	    }
	}
      if (portal->pIndexData->string2[0] != '\0')
	{
	  act (portal->pIndexData->string2, ch, portal, NULL, TO_CHAR);
	}

      else
	{
	  if (IS_SET (portal->value[2], GATE_NORMAL_EXIT))
	    act ("You enter $p.", ch, portal, NULL, TO_CHAR);

	  else
	    act ("You walk through $p.", ch, portal, NULL, TO_CHAR);
	}
      if (ch->mount != NULL && mount_follow)
	{
	  char_from_room (ch->mount);
	  char_to_room (ch->mount, location);
	}
      prev = ch->in_room;
      char_from_room (ch);
      char_to_room (ch, location);
      if (IS_SET (portal->value[2], GATE_GOWITH))
	{
	  obj_from_room (portal);
	  obj_to_room (portal, location);
	}
      if (portal->pIndexData->string3[0] != '\0')
	{
	  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	    {
	      fch_next = fch->next_in_room;
	      if (can_see (fch, ch))
		act (portal->pIndexData->string3, ch, portal, fch, TO_VICT);
	    }
	}

      else
	{
	  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	    {
	      fch_next = fch->next_in_room;
	      if (can_see (fch, ch))
		act ("$n has arrived.", ch, portal, fch, TO_VICT);
	    }
	}
      if (portal->pIndexData->string4[0] != '\0');
      act (portal->pIndexData->string4, ch, portal, NULL, TO_CHAR);

/*** NEW STUFF ***/
      do_look (ch, "auto");

      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(ch,"room",ch->in_room,NULL);
	trap_check(ch,"enter",prev,NULL);
      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      check_aggression(ch);

      trip_triggers(ch, OBJ_TRIG_ACTION_WORD, portal, NULL, OT_SPEC_ENTER);
      
      if (portal->value[0] != -1)
	{
	  portal->value[0]--;
	  if (portal->value[0] == -1)
	    portal->value[0] = 0;
	}

      /* protect against circular follows */
      if (old_room == location)
	return;
      for (fch = old_room->people; fch != NULL; fch = fch_next)
	{
	  fch_next = fch->next_in_room;
	  if (portal == NULL || portal->value[0] == 0 || !can_see_obj(fch,portal))

	    /* no following through dead portals */
	    continue;
	  if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
	      && fch->position < POS_STANDING)
	    do_stand (fch, "");
	  if (fch->master == ch && fch->position == POS_STANDING)
	    {
	      if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
		  && (IS_NPC (fch) && 
			  (IS_SET (fch->act, ACT_AGGRESSIVE)
			   || IS_SET (fch->act2, ACT_AGGIE_GOOD)
			   || IS_SET (fch->act2, ACT_AGGIE_NEUTRAL)
			   || IS_SET (fch->act2, ACT_AGGIE_EVIL))))
			  


		{
		  act ("You can't bring $N into the city.", ch, NULL,
		       fch, TO_CHAR);
		  act ("You aren't allowed in the city.", fch, NULL,
		       NULL, TO_CHAR);
		  continue;
		}
	      //5-10-03 Iblis - Check to remove camoflauge/hide if following someone
	      if (IS_AFFECTED (fch, AFF_CAMOUFLAGE))
		REMOVE_BIT (fch->affected_by, AFF_CAMOUFLAGE);
	      if (IS_AFFECTED (fch, AFF_HIDE))
		REMOVE_BIT (fch->affected_by, AFF_HIDE);
	      act ("You follow $N.", fch, NULL, ch, TO_CHAR);
	      do_enter (fch, argument);
	    }
	}
      if (portal != NULL && portal->value[0] == 0)
	{
	  act ("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
	  if (ch->in_room == old_room)
	    act ("$p fades out of existence.", ch, portal, NULL, TO_ROOM);

	  else if (old_room->people != NULL)
	    {
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_CHAR);
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_ROOM);
	    }
	  extract_obj (portal);
	}
      return;
    }
  send_to_char ("You can't do that.\n\r", ch);
  return;
}

void do_enter (CHAR_DATA * ch, char *argument)
{
  enter_portal (ch, argument, TRUE);
}

void do_scale (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location = NULL,*prev;
  OBJ_DATA *obj;
  char tcbuf[15];
  if (ch->fighting != NULL)
    return;

  /* nifty portal stuff */
  if (argument[0] != '\0')
    {
      ROOM_INDEX_DATA *old_room;
      OBJ_DATA *portal;
      CHAR_DATA *fch, *fch_next;
      old_room = ch->in_room;
      portal = get_obj_list (ch, argument, ch->in_room->contents);
      if (portal == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
      if (!IS_SET (portal->value[4], PACT_SCALE))
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if (portal->item_type != ITEM_PORTAL
	  || (IS_SET (portal->value[1], EX_CLOSED)
	      && !IS_TRUSTED (ch, DEMIGOD)))
	{
	  send_to_char ("You can't seem to find a way up.\n\r", ch);
	  return;
	}
      if (!IS_TRUSTED (ch, DEMIGOD)
	  && !IS_SET (portal->value[2], GATE_NOCURSE)
	  && (IS_AFFECTED (ch, AFF_CURSE)))
	{
	  send_to_char ("Something prevents you from scaling.\n\r", ch);
	  return;
	}
      if (IS_SET (portal->value[2], GATE_TRANSPORT))
	{
	  for (obj = object_list; obj != NULL; obj = obj->next)
	    {
	      if (!can_see_obj (ch, obj))
		continue;
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->in_room != NULL)
		{
		  location = get_room_index (obj->in_room->vnum);
		  break;
		}
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->carried_by != NULL)
		{
		  location = obj->carried_by->in_room;
		  break;
		}
	    }
	}

      else if (IS_SET (portal->value[2], GATE_RANDOM)
	       || portal->value[3] == -1)
	{
	  location = get_random_room (ch);
	  portal->value[3] = location->vnum;	/* for record keeping :) */
	}

      else if (IS_SET (portal->value[2], GATE_BUGGY)
	       && (number_percent () < 5))
	location = get_random_room (ch);

      else
	location = get_room_index (portal->value[3]);
      if (location == NULL
	  || !can_see_room (ch, location)
	  || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	{
	  act ("$p doesn't seem to go anywhere.", ch, portal, NULL, TO_CHAR);
	  return;
	}
      //Iblis 6/21/03 - Battle Royale room off limit checking
      if (!br_vnum_check (location->vnum))
	{
	  send_to_char
	    ("That room is off limits during `iBattle Royale``\r\n", ch);
	  return;
	}
      if (IS_NPC (ch) && 
		      (IS_SET (ch->act, ACT_AGGRESSIVE)
		       || IS_SET (ch->act2, ACT_AGGIE_GOOD)
                       || IS_SET (ch->act2, ACT_AGGIE_NEUTRAL)
                       || IS_SET (ch->act2, ACT_AGGIE_EVIL))
	  && IS_SET (location->room_flags, ROOM_LAW))
	{
	  send_to_char ("Something prevents you from scaling.\n\r", ch);
	  return;
	}
      if (!can_move_char (ch, location, FALSE, FALSE))
	{
	  send_to_char ("Something prevents you from scaling.\n\r", ch);
	  return;
	}

/*** NEW STUFF ***/
      if (portal->pIndexData->string1[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string1, ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n scales $p.", ch, portal, fch, TO_VICT);
	  }
      if (portal->pIndexData->string2[0] != '\0')
	act (portal->pIndexData->string2, ch, portal, NULL, TO_CHAR);

      else
	{
	  if (IS_SET (portal->value[2], GATE_NORMAL_EXIT))
	    act ("You scale $p.", ch, portal, NULL, TO_CHAR);

	  else
	    act ("You scale your way up $p.", ch, portal, NULL, TO_CHAR);
	}
      if (ch->mount != NULL)
	{
	  char_from_room (ch->mount);
	  char_to_room (ch->mount, location);
	}
      prev = ch->in_room;
      char_from_room (ch);
      char_to_room (ch, location);
      if (IS_SET (portal->value[2], GATE_GOWITH))
	{
	  obj_from_room (portal);
	  obj_to_room (portal, location);
	}
      if (portal->pIndexData->string3[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string3, ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n has arrived.", ch, portal, fch, TO_VICT);
	  }
      if (portal->pIndexData->string4[0] != '\0');
      act (portal->pIndexData->string4, ch, portal, NULL, TO_CHAR);
/*** NEW STUFF ***/
      do_look (ch, "auto");

      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(ch,"room",ch->in_room,NULL);
      trap_check(ch,"scale",prev,NULL);

      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      check_aggression(ch);
      trip_triggers(ch, OBJ_TRIG_ACTION_WORD, portal, NULL, OT_SPEC_SCALE);
      
      if (portal->value[0] != -1)
	{
	  portal->value[0]--;
	  if (portal->value[0] == -1)
	    portal->value[0] = 0;
	}

      /* protect against circular follows */
      if (old_room == location)
	return;
      for (fch = old_room->people; fch != NULL; fch = fch_next)
	{
	  fch_next = fch->next_in_room;
	  if (portal == NULL || portal->value[0] == 0)

	    /* no following through dead portals */
	    continue;
	  if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
	      && fch->position < POS_STANDING)
	    do_stand (fch, "");
	  if (fch->master == ch && fch->position == POS_STANDING)
	    {
	      if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
		  && (IS_NPC (fch) && 
			  (IS_SET (fch->act, ACT_AGGRESSIVE)
			   || IS_SET (fch->act2, ACT_AGGIE_GOOD)
                           || IS_SET (fch->act2, ACT_AGGIE_NEUTRAL)
                           || IS_SET (fch->act2, ACT_AGGIE_EVIL))))
		{
		  act ("You can't bring $N into the city.", ch, NULL,
		       fch, TO_CHAR);
		  act ("You aren't allowed in the city.", fch, NULL,
		       NULL, TO_CHAR);
		  continue;
		}
	      act ("You scale after $N.", fch, NULL, ch, TO_CHAR);
	      do_scale (fch, argument);
	    }
	}
      if (portal != NULL && portal->value[0] == 0)
	{
	  act ("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
	  if (ch->in_room == old_room)
	    act ("$p fades out of existence.", ch, portal, NULL, TO_ROOM);

	  else if (old_room->people != NULL)
	    {
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_CHAR);
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_ROOM);
	    }
	  extract_obj (portal);
	}
      return;
    }
  send_to_char ("You can't do that.\n\r", ch);
  return;
}

void do_crawl (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location = NULL,*prev;
  OBJ_DATA *obj;
  char tcbuf[15];
  if (ch->fighting != NULL)
    return;

  /* nifty portal stuff */
  if (argument[0] != '\0')
    {
      ROOM_INDEX_DATA *old_room;
      OBJ_DATA *portal;
      CHAR_DATA *fch, *fch_next;
      old_room = ch->in_room;
      portal = get_obj_list (ch, argument, ch->in_room->contents);
      if (portal == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
      if (!IS_SET (portal->value[4], PACT_CRAWL))
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if (portal->item_type != ITEM_PORTAL
	  || (IS_SET (portal->value[1], EX_CLOSED)
	      && !IS_TRUSTED (ch, DEMIGOD)))
	{
	  send_to_char ("You can't seem to find a way in.\n\r", ch);
	  return;
	}
      if (!IS_TRUSTED (ch, DEMIGOD)
	  && !IS_SET (portal->value[2], GATE_NOCURSE)
	  && (IS_AFFECTED (ch, AFF_CURSE)))
	{
	  send_to_char ("Something prevents you from crawling.\n\r", ch);
	  return;
	}
      if (IS_SET (portal->value[2], GATE_TRANSPORT))
	{
	  for (obj = object_list; obj != NULL; obj = obj->next)
	    {
	      if (!can_see_obj (ch, obj))
		continue;
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->in_room != NULL)
		{
		  location = get_room_index (obj->in_room->vnum);
		  break;
		}
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->carried_by != NULL)
		{
		  location = obj->carried_by->in_room;
		  break;
		}
	    }
	}

      else if (IS_SET (portal->value[2], GATE_RANDOM)
	       || portal->value[3] == -1)
	{
	  location = get_random_room (ch);
	  portal->value[3] = location->vnum;	/* for record keeping :) */
	}

      else if (IS_SET (portal->value[2], GATE_BUGGY)
	       && (number_percent () < 5))
	location = get_random_room (ch);

      else
	location = get_room_index (portal->value[3]);
      if (location == NULL
	  || !can_see_room (ch, location)
	  || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	{
	  act ("$p doesn't seem to go anywhere.", ch, portal, NULL, TO_CHAR);
	  return;
	}
      //Iblis 6/21/03 - Battle Royale room off limit checking
      if (!br_vnum_check (location->vnum))
	{
	  send_to_char
	    ("That room is off limits during `iBattle Royale``\r\n", ch);
	  return;
	}
      if (IS_NPC (ch) && 
		      (IS_SET (ch->act, ACT_AGGRESSIVE)
		       || IS_SET (ch->act2, ACT_AGGIE_GOOD)
                       || IS_SET (ch->act2, ACT_AGGIE_NEUTRAL)
                       || IS_SET (ch->act2, ACT_AGGIE_EVIL))
	  && IS_SET (location->room_flags, ROOM_LAW))
	{
	  send_to_char ("Something prevents you from crawling...\n\r", ch);
	  return;
	}
      if (!can_move_char (ch, location, FALSE, FALSE))
	{
	  send_to_char ("Something prevents you from crawling...\n\r", ch);
	  return;
	}

/*** NEW STUFF ***/
      if (portal->pIndexData->string1[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string1, ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n crawls to $p.", ch, portal, fch, TO_VICT);
	  }
      if (portal->pIndexData->string2[0] != '\0')
	act (portal->pIndexData->string2, ch, portal, NULL, TO_CHAR);

      else
	{
	  act ("You crawl $p.", ch, portal, NULL, TO_CHAR);
	}
      if (ch->mount != NULL)
	{
	  char_from_room (ch->mount);
	  char_to_room (ch->mount, location);
	}
      prev = ch->in_room;
      char_from_room (ch);

      char_to_room (ch, location);
      if (IS_SET (portal->value[2], GATE_GOWITH))
	{
	  obj_from_room (portal);
	  obj_to_room (portal, location);
	}
      if (portal->pIndexData->string3[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string3, ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n has arrived.", ch, portal, fch, TO_VICT);
	  }
      if (portal->pIndexData->string4[0] != '\0');
      act (portal->pIndexData->string4, ch, portal, NULL, TO_CHAR);

/*** NEW STUFF ***/
      do_look (ch, "auto");

      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(ch,"room",ch->in_room,NULL);
      trap_check(ch,"crawl",prev,NULL);
      
      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      check_aggression(ch);
      trip_triggers(ch, OBJ_TRIG_ACTION_WORD, portal, NULL, OT_SPEC_CRAWL);
      
      if (portal->value[0] != -1)
	{
	  portal->value[0]--;
	  if (portal->value[0] == -1)
	    portal->value[0] = 0;
	}

      /* protect against circular follows */
      if (old_room == location)
	return;
      for (fch = old_room->people; fch != NULL; fch = fch_next)
	{
	  fch_next = fch->next_in_room;
	  if (portal == NULL || portal->value[0] == 0)

	    /* no following through dead portals */
	    continue;
	  if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
	      && fch->position < POS_STANDING)
	    do_stand (fch, "");
	  if (fch->master == ch && fch->position == POS_STANDING)
	    {
	      if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
		  && (IS_NPC (fch) && 
			  (IS_SET (fch->act, ACT_AGGRESSIVE)
			   || IS_SET (fch->act2, ACT_AGGIE_GOOD)
                           || IS_SET (fch->act2, ACT_AGGIE_NEUTRAL)
                           || IS_SET (fch->act2, ACT_AGGIE_EVIL))))
		{
		  act ("You can't bring $N into the city.", ch, NULL,
		       fch, TO_CHAR);
		  act ("You aren't allowed in the city.", fch, NULL,
		       NULL, TO_CHAR);
		  continue;
		}
	      act ("You crawl after $N.", fch, NULL, ch, TO_CHAR);
	      do_crawl (fch, argument);
	    }
	}
      if (portal != NULL && portal->value[0] == 0)
	{
	  act ("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
	  if (ch->in_room == old_room)
	    act ("$p fades out of existence.", ch, portal, NULL, TO_ROOM);

	  else if (old_room->people != NULL)
	    {
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_CHAR);
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_ROOM);
	    }
	  extract_obj (portal);
	}
      return;
    }
  send_to_char ("You can't do that.\n\r", ch);
  return;
}

void do_descend (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location = NULL,*prev;
  OBJ_DATA *obj;
  char tcbuf[15];
  if (ch->fighting != NULL)
    return;

  /* nifty portal stuff */
  if (argument[0] != '\0')
    {
      ROOM_INDEX_DATA *old_room;
      OBJ_DATA *portal;
      CHAR_DATA *fch=NULL, *fch_next;
      old_room = ch->in_room;
      portal = get_obj_list (ch, argument, ch->in_room->contents);
      if (portal == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
      if (!IS_SET (portal->value[4], PACT_DESCEND))
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if (portal->item_type != ITEM_PORTAL
	  || (IS_SET (portal->value[1], EX_CLOSED)
	      && !IS_TRUSTED (ch, DEMIGOD)))
	{
	  send_to_char ("You can't seem to find a way down.\n\r", ch);
	  return;
	}
      if (!IS_TRUSTED (ch, DEMIGOD)
	  && !IS_SET (portal->value[2], GATE_NOCURSE)
	  && (IS_AFFECTED (ch, AFF_CURSE)))
	{
	  send_to_char ("Something prevents you from descending...\n\r", ch);
	  return;
	}
      if (IS_SET (portal->value[2], GATE_TRANSPORT))
	{
	  for (obj = object_list; obj != NULL; obj = obj->next)
	    {
	      if (!can_see_obj (ch, obj))
		continue;
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->in_room != NULL)
		{
		  location = get_room_index (obj->in_room->vnum);
		  break;
		}
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->carried_by != NULL)
		{
		  location = obj->carried_by->in_room;
		  break;
		}
	    }
	}

      else if (IS_SET (portal->value[2], GATE_RANDOM)
	       || portal->value[3] == -1)
	{
	  location = get_random_room (ch);
	  portal->value[3] = location->vnum;	/* for record keeping :) */
	}

      else if (IS_SET (portal->value[2], GATE_BUGGY)
	       && (number_percent () < 5))
	location = get_random_room (ch);

      else
	location = get_room_index (portal->value[3]);
      if (location == NULL
	  || !can_see_room (ch, location)
	  || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	{
	  act ("$p doesn't seem to go anywhere.", ch, portal, NULL, TO_CHAR);
	  return;
	}
      //Iblis 6/21/03 - Battle Royale room off limit checking
      if (!br_vnum_check (location->vnum))
	{
	  send_to_char
	    ("That room is off limits during `iBattle Royale``\r\n", ch);
	  return;
	}
      if (IS_NPC (ch) && 
 	      (IS_SET (ch->act, ACT_AGGRESSIVE)
 	       || IS_SET (ch->act2, ACT_AGGIE_GOOD)
               || IS_SET (ch->act2, ACT_AGGIE_NEUTRAL)
               || IS_SET (ch->act2, ACT_AGGIE_EVIL))
	  && IS_SET (location->room_flags, ROOM_LAW))
	{
	  send_to_char ("Something prevents you from descending...\n\r", ch);
	  return;
	}
      if (!can_move_char (ch, location, FALSE, FALSE))
	{
	  send_to_char ("Something prevents you from descending...\n\r", ch);
	  return;
	}

/*** NEW STUFF ***/
      if (portal->pIndexData->string1[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string1, ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n descends $p.", ch, portal, fch, TO_VICT);
	  }
      if (portal->pIndexData->string2[0] != '\0')
	act (portal->pIndexData->string2, ch, portal, NULL, TO_CHAR);

      else
	{
	  act ("You descend $p.", ch, portal, NULL, TO_CHAR);
	}
      if (ch->mount != NULL)
	{
	  char_from_room (ch->mount);
	  char_to_room (ch->mount, location);
	}
      prev = ch->in_room;
      char_from_room (ch);
      char_to_room (ch, location);
      if (IS_SET (portal->value[2], GATE_GOWITH))
	{
	  obj_from_room (portal);
	  obj_to_room (portal, location);
	}
      if (portal->pIndexData->string3[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string3, ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n has arrived, climbing down $p.", ch, portal,
		   fch, TO_VICT);
	  }
      if (portal->pIndexData->string4[0] != '\0');
      act (portal->pIndexData->string4, ch, portal, NULL, TO_CHAR);

/*** NEW STUFF ***/
      do_look (ch, "auto");

      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(ch,"room",ch->in_room,NULL);
      trap_check(ch,"descend",prev,NULL);
      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      check_aggression(ch);
      trip_triggers(ch, OBJ_TRIG_ACTION_WORD, portal, NULL, OT_SPEC_DESCEND);
      
      if (portal->value[0] != -1)
	{
	  portal->value[0]--;
	  if (portal->value[0] == -1)
	    portal->value[0] = 0;
	}

      /* protect against circular follows */
      if (old_room == location)
	return;
      for (fch = old_room->people; fch != NULL; fch = fch_next)
	{
	  fch_next = fch->next_in_room;
	  if (portal == NULL || portal->value[0] == 0)

	    /* no following through dead portals */
	    continue;
	  if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
	      && fch->position < POS_STANDING)
	    do_stand (fch, "");
	  if (fch->master == ch && fch->position == POS_STANDING)
	    {
	      if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
		  && (IS_NPC (fch) && 
			  (IS_SET (fch->act, ACT_AGGRESSIVE)
			   || IS_SET (fch->act2, ACT_AGGIE_GOOD)
                           || IS_SET (fch->act2, ACT_AGGIE_NEUTRAL)
                           || IS_SET (fch->act2, ACT_AGGIE_EVIL))))
		{
		  act ("You can't bring $N into the city.", ch, NULL,
		       fch, TO_CHAR);
		  act ("You aren't allowed in the city.", fch, NULL,
		       NULL, TO_CHAR);
		  continue;
		}
	      act ("You descend after $N.", fch, NULL, ch, TO_CHAR);
	      do_descend (fch, argument);
	    }
	}
      if (portal != NULL && portal->value[0] == 0)
	{
	  act ("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
	  if (ch->in_room == old_room)
	    act ("$p fades out of existence.", ch, portal, NULL, TO_ROOM);

	  else if (old_room->people != NULL)
	    {
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_CHAR);
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_ROOM);
	    }
	  extract_obj (portal);
	}
      return;
    }
  send_to_char ("You can't do that.\n\r", ch);
  return;
}

void do_climb (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location = NULL,*prev;
  OBJ_DATA *obj;
  char tcbuf[15];
  if (ch->fighting != NULL)
    return;

  /* nifty portal stuff */
  if (argument[0] != '\0')
    {
      ROOM_INDEX_DATA *old_room;
      OBJ_DATA *portal;
      CHAR_DATA *fch, *fch_next;
      old_room = ch->in_room;
      portal = get_obj_list (ch, argument, ch->in_room->contents);
      if (portal == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
      if (!IS_SET (portal->value[4], PACT_CLIMB))
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if (portal->item_type != ITEM_PORTAL
	  || (IS_SET (portal->value[1], EX_CLOSED)
	      && !IS_TRUSTED (ch, DEMIGOD)))
	{
	  send_to_char ("You can't seem to find a way up.\n\r", ch);
	  return;
	}
      if (!IS_TRUSTED (ch, DEMIGOD)
	  && !IS_SET (portal->value[2], GATE_NOCURSE)
	  && (IS_AFFECTED (ch, AFF_CURSE)))
	{
	  send_to_char ("Something prevents you from climbing...\n\r", ch);
	  return;
	}
      if (IS_SET (portal->value[2], GATE_TRANSPORT))
	{
	  for (obj = object_list; obj != NULL; obj = obj->next)
	    {
	      if (!can_see_obj (ch, obj))
		continue;
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->in_room != NULL)
		{
		  location = get_room_index (obj->in_room->vnum);
		  break;
		}
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->carried_by != NULL)
		{
		  location = obj->carried_by->in_room;
		  break;
		}
	    }
	}

      else if (IS_SET (portal->value[2], GATE_RANDOM)
	       || portal->value[3] == -1)
	{
	  location = get_random_room (ch);
	  portal->value[3] = location->vnum;	/* for record keeping :) */
	}

      else if (IS_SET (portal->value[2], GATE_BUGGY)
	       && (number_percent () < 5))
	location = get_random_room (ch);

      else
	location = get_room_index (portal->value[3]);
      if (location == NULL
	  || !can_see_room (ch, location)
	  || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	{
	  act ("$p doesn't seem to go anywhere.", ch, portal, NULL, TO_CHAR);
	  return;
	}
      //Iblis 6/21/03 - Battle Royale room off limit checking
      if (!br_vnum_check (location->vnum))
	{
	  send_to_char
	    ("That room is off limits during `iBattle Royale``\r\n", ch);
	  return;
	}
      if (IS_NPC (ch) && 
		      (IS_SET (ch->act, ACT_AGGRESSIVE)
		       || IS_SET (ch->act2, ACT_AGGIE_GOOD)
                       || IS_SET (ch->act2, ACT_AGGIE_NEUTRAL)
                       || IS_SET (ch->act2, ACT_AGGIE_EVIL))
	  && IS_SET (location->room_flags, ROOM_LAW))
	{
	  send_to_char ("Something prevents you from climbing...\n\r", ch);
	  return;
	}
      if (!can_move_char (ch, location, FALSE, FALSE))
	{
	  send_to_char ("Something prevents you from climbing...\n\r", ch);
	  return;
	}
      if (portal->pIndexData->string1[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string1, ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n climbs up $p.", ch, portal, fch, TO_VICT);
	  }
      if (portal->pIndexData->string2[0] != '\0')
	act (portal->pIndexData->string2, ch, portal, NULL, TO_CHAR);

      else
	{
	  act ("You climb up $p.", ch, portal, NULL, TO_CHAR);
	}
      if (ch->mount != NULL)
	{
	  char_from_room (ch->mount);
	  char_to_room (ch->mount, location);
	}
      prev = ch->in_room;
      char_from_room (ch);
      char_to_room (ch, location);
      if (IS_SET (portal->value[2], GATE_GOWITH))
	{
	  obj_from_room (portal);
	  obj_to_room (portal, location);
	}
      if (portal->pIndexData->string3[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string3, ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n has arrived.", ch, portal, fch, TO_VICT);
	  }
      if (portal->pIndexData->string4[0] != '\0');
      act (portal->pIndexData->string4, ch, portal, NULL, TO_CHAR);
      do_look (ch, "auto");

      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(ch,"room",ch->in_room,NULL);
      trap_check(ch,"climb",prev,NULL);	
      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      check_aggression(ch);
      trip_triggers(ch, OBJ_TRIG_ACTION_WORD, portal, NULL, OT_SPEC_CLIMB);
      
      if (portal->value[0] != -1)
	{
	  portal->value[0]--;
	  if (portal->value[0] == -1)
	    portal->value[0] = 0;
	}

      /* protect against circular follows */
      if (old_room == location)
	return;
      for (fch = old_room->people; fch != NULL; fch = fch_next)
	{
	  fch_next = fch->next_in_room;
	  if (portal == NULL || portal->value[0] == 0)

	    /* no following through dead portals */
	    continue;
	  if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
	      && fch->position < POS_STANDING)
	    do_stand (fch, "");
	  if (fch->master == ch && fch->position == POS_STANDING)
	    {
	      if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
		  && (IS_NPC (fch) && 
			  (IS_SET (fch->act, ACT_AGGRESSIVE)
			   || IS_SET (fch->act2, ACT_AGGIE_GOOD)
                           || IS_SET (fch->act2, ACT_AGGIE_NEUTRAL)
                           || IS_SET (fch->act2, ACT_AGGIE_EVIL))))
		{
		  act ("You can't bring $N into the city.", ch, NULL,
		       fch, TO_CHAR);
		  act ("You aren't allowed in the city.", fch, NULL,
		       NULL, TO_CHAR);
		  continue;
		}
	      act ("You climb after $N.", fch, NULL, ch, TO_CHAR);
	      do_climb (fch, argument);
	    }
	}
      if (portal != NULL && portal->value[0] == 0)
	{
	  act ("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
	  if (ch->in_room == old_room)
	    act ("$p fades out of existence.", ch, portal, NULL, TO_ROOM);

	  else if (old_room->people != NULL)
	    {
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_CHAR);
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_ROOM);
	    }
	  extract_obj (portal);
	}
      return;
    }
  send_to_char ("You can't do that.\n\r", ch);
  return;
}


int exit_elevator (CHAR_DATA * ch)
{
  OBJ_DATA *eObj;
  CHAR_DATA *fch, *fch_next;
  ROOM_INDEX_DATA *location, *old_room;
  char tcbuf[15];
  if (!IS_SET (ch->in_room->room_flags, ROOM_ELEVATOR))
    return (0);
  for (eObj = object_list; eObj; eObj = eObj->next)
    {
      if (eObj->item_type != ITEM_ELEVATOR)
	continue;
      if (eObj->value[0] == ch->in_room->vnum)
	{
	  location = eObj->in_room;
	  if (location == NULL
	      || location == ch->in_room || !can_see_room (ch, location)
	      || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	    {
	      act ("You can't seem to get out that way.", ch, NULL, NULL,
		   TO_CHAR);
	      return (1);
	    }
	  //Iblis 6/21/03 - Battle Royale room off limit checking
	  if (!br_vnum_check (location->vnum))
	    {
	      send_to_char
		("That room is off limits during `iBattle Royale``\r\n", ch);
	      return (1);
	    }
	  if (!can_move_char (ch, location, FALSE, FALSE))
	    {
	      send_to_char ("Something prevents you from exiting.\n\r", ch);
	      return (1);
	    }
	  if (location->vnum == ROOM_VNUM_ELIMBO)
	    {
	      send_to_char ("You can't exit here.\n\r", ch);
	      return (1);
	    }
	  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	    {
	      fch_next = fch->next_in_room;
	      if (can_see (fch, ch))
		act ("$n steps out of $p.", ch, eObj, fch, TO_VICT);
	    }
	  act ("You exit $p.", ch, eObj, NULL, TO_CHAR);
	  old_room = ch->in_room;
	  if (ch->mount != NULL)
	    {
	      char_from_room (ch->mount);
	      char_to_room (ch->mount, location);
	    }
	  char_from_room (ch);
	  char_to_room (ch, location);
	  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	    {
	      fch_next = fch->next_in_room;
	      if (can_see (fch, ch))
		act ("$n steps out of $p.", ch, eObj, fch, TO_VICT);
	    }
	  do_look (ch, "auto");
	  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	  trap_check(ch,"room",ch->in_room,NULL);

	  sprintf(tcbuf,"%d",ch->in_room->vnum);
	  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
	  check_aggression(ch);

	  for (fch = old_room->people; fch != NULL; fch = fch_next)
	    {
	      fch_next = fch->next_in_room;
	      if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
		  && fch->position < POS_STANDING)
		do_stand (fch, "");
	      if (fch->master == ch && fch->position == POS_STANDING)
		{
		  if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
		      && (IS_NPC (fch) && 
			      (IS_SET (fch->act, ACT_AGGRESSIVE)
			       || IS_SET (fch->act2, ACT_AGGIE_GOOD)
                               || IS_SET (fch->act2, ACT_AGGIE_NEUTRAL)
                               || IS_SET (fch->act2, ACT_AGGIE_EVIL))))
		    {
		      act ("You can't bring $N into the city.", ch,
			   NULL, fch, TO_CHAR);
		      act ("You aren't allowed in the city.", fch, NULL,
			   NULL, TO_CHAR);
		      continue;
		    }
		  //5-10-03 Iblis - Check to remove camoflauge/hide if following someone
		  if (IS_AFFECTED (fch, AFF_CAMOUFLAGE))
		    REMOVE_BIT (fch->affected_by, AFF_CAMOUFLAGE);
		  if (IS_AFFECTED (fch, AFF_HIDE))
		    REMOVE_BIT (fch->affected_by, AFF_HIDE);
		  act ("You follow $N.", fch, NULL, ch, TO_CHAR);
		  exit_elevator (fch);
		}
	    }
	  return (1);
	}
    }
  return (0);
}

int exit_transport (CHAR_DATA * ch)
{
  OBJ_DATA *eObj;
  CHAR_DATA *fch, *fch_next;
  ROOM_INDEX_DATA *location, *old_room;
  char tcbuf[15];
  if (!IS_SET (ch->in_room->room_flags, ROOM_TRANSPORT))
    return (0);
  for (eObj = object_list; eObj; eObj = eObj->next)
    {
      if (eObj->item_type != ITEM_CTRANSPORT || eObj->in_room == NULL)
	continue;
      if (eObj->value[2] == ch->in_room->vnum)
	{
	  if (IS_SET (eObj->value[5], CONT_CLOSED))
	    {
	      send_to_char ("It is closed.\n\r", ch);
	      return (1);
	    }
	  location = eObj->in_room;
	  if (location == NULL || location == ch->in_room
	      || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	    {
	      act ("You can't seem to get out that way.", ch, NULL, NULL,
		   TO_CHAR);
	      return (1);
	    }
	  //Iblis 6/21/03 - Battle Royale room off limit checking
	  if (!br_vnum_check (location->vnum))
	    {
	      send_to_char
		("That room is off limits during `iBattle Royale``\r\n", ch);
	      return (1);
	    }
	  if (!can_move_char (ch, location, FALSE, FALSE))
	    {
	      send_to_char ("Something prevents you from exiting.\n\r", ch);
	      return (1);
	    }
	  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	    {
	      fch_next = fch->next_in_room;
	      if (can_see (fch, ch))
		act ("$n steps out of $p.", ch, eObj, fch, TO_VICT);
	    }
	  act ("You exit $p.", ch, eObj, NULL, TO_CHAR);
	  old_room = ch->in_room;
	  if (ch->mount != NULL)
	    {
	      char_from_room (ch->mount);
	      char_to_room (ch->mount, location);
	    }
	  char_from_room (ch);
	  char_to_room (ch, location);
	  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	    {
	      fch_next = fch->next_in_room;
	      if (can_see (fch, ch))
		act ("$n steps out of $p.", ch, eObj, fch, TO_VICT);
	    }
	  do_look (ch, "auto");
          trap_check(ch,"room",ch->in_room,NULL);
	  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	  sprintf(tcbuf,"%d",ch->in_room->vnum);
	  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
	  check_aggression(ch);
	  for (fch = old_room->people; fch != NULL; fch = fch_next)
	    {
	      fch_next = fch->next_in_room;
	      if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
		  && fch->position < POS_STANDING)
		do_stand (fch, "");
	      if (fch->master == ch && fch->position == POS_STANDING)
		{
		  if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
		      && (IS_NPC (fch) && 
			      (IS_SET (fch->act, ACT_AGGRESSIVE)
			       || IS_SET (fch->act2, ACT_AGGIE_GOOD)
                               || IS_SET (fch->act2, ACT_AGGIE_NEUTRAL)
                               || IS_SET (fch->act2, ACT_AGGIE_EVIL))))
		    {
		      act ("You can't bring $N into the city.", ch,
			   NULL, fch, TO_CHAR);
		      act ("You aren't allowed in the city.", fch, NULL,
			   NULL, TO_CHAR);
		      continue;
		    }
		  //5-10-03 Iblis - Check to remove camoflauge/hide if following someone
		  if (IS_AFFECTED (fch, AFF_CAMOUFLAGE))
		    REMOVE_BIT (fch->affected_by, AFF_CAMOUFLAGE);
		  if (IS_AFFECTED (fch, AFF_HIDE))
		    REMOVE_BIT (fch->affected_by, AFF_HIDE);
		  act ("You follow $N.", fch, NULL, ch, TO_CHAR);
		  exit_transport (fch);
		}
	    }
	  return (1);
	}
    }
  return (0);
}

void do_pexit (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location = NULL,*prev;
  OBJ_DATA *obj;
  char tcbuf[15];
  if (ch->fighting != NULL)
    return;
  if (ch->position < POS_STANDING)
  {
    send_to_char("You need to stand to do that.\n\r",ch);
    return;
  }

  /* nifty portal stuff */
  if (argument[0] != '\0')
    {
      ROOM_INDEX_DATA *old_room;
      OBJ_DATA *portal;
      CHAR_DATA *fch, *fch_next;
      old_room = ch->in_room;
      portal = get_obj_list (ch, argument, ch->in_room->contents);
      if (portal == NULL)
	{
	  if (!exit_elevator (ch))
	    if (!exit_transport (ch))
	      send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
      if (!IS_SET (portal->value[4], PACT_EXIT))
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if (portal->item_type != ITEM_PORTAL
	  || (IS_SET (portal->value[1], EX_CLOSED)
	      && !IS_TRUSTED (ch, DEMIGOD)))
	{
	  send_to_char ("You can't seem to find a way out.\n\r", ch);
	  return;
	}
      if (!IS_TRUSTED (ch, DEMIGOD)
	  && !IS_SET (portal->value[2], GATE_NOCURSE)
	  && (IS_AFFECTED (ch, AFF_CURSE)))
	{
	  send_to_char ("Something prevents you from leaving...\n\r", ch);
	  return;
	}
      if (IS_SET (portal->value[2], GATE_TRANSPORT))
	{
	  for (obj = object_list; obj != NULL; obj = obj->next)
	    {
	      if (!can_see_obj (ch, obj))
		continue;
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->in_room != NULL)
		{
		  location = get_room_index (obj->in_room->vnum);
		  break;
		}
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->carried_by != NULL)
		{
		  location = obj->carried_by->in_room;
		  break;
		}
	    }
	}

      else if (IS_SET (portal->value[2], GATE_RANDOM)
	       || portal->value[3] == -1)
	{
	  location = get_random_room (ch);
	  portal->value[3] = location->vnum;	/* for record keeping :) */
	}

      else if (IS_SET (portal->value[2], GATE_BUGGY)
	       && (number_percent () < 5))
	location = get_random_room (ch);

      else
	location = get_room_index (portal->value[3]);
      if (location == NULL
	  || !can_see_room (ch, location)
	  || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	{
	  act ("$p doesn't seem to go anywhere.", ch, portal, NULL, TO_CHAR);
	  return;
	}
      //Iblis 6/21/03 - Battle Royale room off limit checking
      if (!br_vnum_check (location->vnum))
	{
	  send_to_char
	    ("That room is off limits during `iBattle Royale``\r\n", ch);
	  return;
	}
      if (IS_NPC (ch) && 
		      (IS_SET (ch->act, ACT_AGGRESSIVE)
		       || IS_SET (ch->act2, ACT_AGGIE_GOOD)
                       || IS_SET (ch->act2, ACT_AGGIE_NEUTRAL)
                       || IS_SET (ch->act2, ACT_AGGIE_EVIL))
	  && IS_SET (location->room_flags, ROOM_LAW))
	{
	  send_to_char ("Something prevents you from leaving...\n\r", ch);
	  return;
	}
      if (!can_move_char (ch, location, FALSE, FALSE))
	{
	  send_to_char ("Something prevents you from leaving...\n\r", ch);
	  return;
	}

/*** NEW STUFF ***/
      if (portal->pIndexData->string1[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string1, ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n steps out of $p.", ch, portal, fch, TO_VICT);
	  }
      if (portal->pIndexData->string2[0] != '\0')
	act (portal->pIndexData->string2, ch, portal, NULL, TO_CHAR);

      else
	{
	  if (IS_SET (portal->value[2], GATE_NORMAL_EXIT))
	    act ("You exit $p.", ch, portal, NULL, TO_CHAR);

	  else
	    act ("You walk through $p and find yourself...", ch,
		 portal, NULL, TO_CHAR);
	}
      if (ch->mount != NULL)
	{
	  char_from_room (ch->mount);
	  char_to_room (ch->mount, location);
	}
      prev = ch->in_room;
      char_from_room (ch);
      char_to_room (ch, location);
      if (IS_SET (portal->value[2], GATE_GOWITH))
	{
	  obj_from_room (portal);
	  obj_to_room (portal, location);
	}
      if (portal->pIndexData->string3[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string3, ch, portal, fch, TO_VICT);
	  }

      else if (IS_SET (portal->value[2], GATE_NORMAL_EXIT))
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n has arrived.", ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n has arrived through $p.", ch, portal, fch, TO_VICT);
	  }
      if (portal->pIndexData->string4[0] != '\0');
      act (portal->pIndexData->string4, ch, portal, NULL, TO_CHAR);

/*** NEW STUFF ***/
      do_look (ch, "auto");
      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
     trap_check(ch,"exit",prev,NULL);
       trap_check(ch,"room",ch->in_room,NULL);
      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      check_aggression(ch);
      if (portal->value[0] != -1)
	{
	  portal->value[0]--;
	  if (portal->value[0] == -1)
	    portal->value[0] = 0;
	}

      /* protect against circular follows */
      if (old_room == location)
	return;
      for (fch = old_room->people; fch != NULL; fch = fch_next)
	{
	  fch_next = fch->next_in_room;
	  if (portal == NULL || portal->value[0] == 0)

	    /* no following through dead portals */
	    continue;
	  if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
	      && fch->position < POS_STANDING)
	    do_stand (fch, "");
	  if (fch->master == ch && fch->position == POS_STANDING)
	    {
	      if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
		  && (IS_NPC (fch) && 
			  (IS_SET (fch->act, ACT_AGGRESSIVE)
			   || IS_SET (fch->act2, ACT_AGGIE_GOOD)
                           || IS_SET (fch->act2, ACT_AGGIE_NEUTRAL)
                           || IS_SET (fch->act2, ACT_AGGIE_EVIL))))
		{
		  act ("You can't bring $N into the city.", ch, NULL,
		       fch, TO_CHAR);
		  act ("You aren't allowed in the city.", fch, NULL,
		       NULL, TO_CHAR);
		  continue;
		}
	      //5-10-03 Iblis - Check to remove camoflauge/hide if following someone
	      if (IS_AFFECTED (fch, AFF_CAMOUFLAGE))
		REMOVE_BIT (fch->affected_by, AFF_CAMOUFLAGE);
	      if (IS_AFFECTED (fch, AFF_HIDE))
		REMOVE_BIT (fch->affected_by, AFF_HIDE);
	      act ("You follow $N.", fch, NULL, ch, TO_CHAR);
	      do_pexit (fch, argument);
	    }
	}
      if (portal != NULL && portal->value[0] == 0)
	{
	  act ("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
	  if (ch->in_room == old_room)
	    act ("$p fades out of existence.", ch, portal, NULL, TO_ROOM);

	  else if (old_room->people != NULL)
	    {
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_CHAR);
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_ROOM);
	    }
	  extract_obj (portal);
	}
      return;
    }
  send_to_char ("Nope, can't do it.\n\r", ch);
  return;
}

void do_jump (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location = NULL,*prev;
  OBJ_DATA *obj;
  char tcbuf[15];
  if (ch->fighting != NULL)
    return;
  if (argument[0] != '\0')
    {
      ROOM_INDEX_DATA *old_room;
      OBJ_DATA *portal;
      CHAR_DATA *fch, *fch_next;
      old_room = ch->in_room;
      portal = get_obj_list (ch, argument, ch->in_room->contents);
      if (portal == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
      if (!IS_SET (portal->value[4], PACT_JUMP))
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if (portal->item_type != ITEM_PORTAL
	  || (IS_SET (portal->value[1], EX_CLOSED)
	      && !IS_TRUSTED (ch, DEMIGOD)))
	{
	  send_to_char
	    ("You can't seem to find a good jumping point.\n\r", ch);
	  return;
	}
      if (!IS_TRUSTED (ch, DEMIGOD)
	  && !IS_SET (portal->value[2], GATE_NOCURSE)
	  && (IS_AFFECTED (ch, AFF_CURSE)))
	{
	  send_to_char ("Something prevents you from jumping.\n\r", ch);
	  return;
	}
      if (IS_SET (portal->value[2], GATE_TRANSPORT))
	{
	  for (obj = object_list; obj != NULL; obj = obj->next)
	    {
	      if (!can_see_obj (ch, obj))
		continue;
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->in_room != NULL)
		{
		  location = get_room_index (obj->in_room->vnum);
		  break;
		}
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->carried_by != NULL)
		{
		  location = obj->carried_by->in_room;
		  break;
		}
	    }
	}

      else if (IS_SET (portal->value[2], GATE_RANDOM)
	       || portal->value[3] == -1)
	{
	  location = get_random_room (ch);
	  portal->value[3] = location->vnum;
	}

      else if (IS_SET (portal->value[2], GATE_BUGGY)
	       && (number_percent () < 5))
	location = get_random_room (ch);

      else
	location = get_room_index (portal->value[3]);
      if (location == NULL
	  || location == old_room || !can_see_room (ch, location)
	  || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	{
	  act ("Jumping $p seems too dangerous.", ch, portal, NULL, TO_CHAR);
	  return;
	}
      //Iblis 6/21/03 - Battle Royale room off limit checking
      if (!br_vnum_check (location->vnum))
	{
	  send_to_char
	    ("That room is off limits during `iBattle Royale``\r\n", ch);
	  return;
	}
      if (IS_NPC (ch) && 
		      (IS_SET (ch->act, ACT_AGGRESSIVE)
		       || IS_SET (ch->act2, ACT_AGGIE_GOOD)
                       || IS_SET (ch->act2, ACT_AGGIE_NEUTRAL)
                       || IS_SET (ch->act2, ACT_AGGIE_EVIL))
	  && IS_SET (location->room_flags, ROOM_LAW))
	{
	  send_to_char ("Something prevents you from jumping.\n\r", ch);
	  return;
	}
      if (!can_move_char (ch, location, FALSE, FALSE))
	{
	  send_to_char ("Something prevents you from jumping.\n\r", ch);
	  return;
	}

/*** NEW STUFF ***/
      if (portal->pIndexData->string1[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string1, ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n jumps $p.", ch, portal, fch, TO_VICT);
	  }
      if (portal->pIndexData->string2[0] != '\0')
	act (portal->pIndexData->string2, ch, portal, NULL, TO_CHAR);

      else
	{
	  if (IS_SET (portal->value[2], GATE_NORMAL_EXIT))
	    act ("You jump $p.", ch, portal, NULL, TO_CHAR);

	  else
	    act ("You jump $p and land....", ch, portal, NULL, TO_CHAR);
	}
      if (ch->mount != NULL)
	{
	  char_from_room (ch->mount);
	  char_to_room (ch->mount, location);
	}
      prev = ch->in_room;
      char_from_room (ch);
      char_to_room (ch, location);
      if (IS_SET (portal->value[2], GATE_GOWITH))
	{
	  obj_from_room (portal);
	  obj_to_room (portal, location);
	}
      if (portal->pIndexData->string3[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string3, ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n has arrived from the sky.", ch, portal, fch, TO_VICT);
	  }
      if (portal->pIndexData->string4[0] != '\0');
      act (portal->pIndexData->string4, ch, portal, NULL, TO_CHAR);

/*** NEW STUFF ***/
      do_look (ch, "auto");

      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(ch,"jump",prev,NULL);
      trap_check(ch,"room",ch->in_room,NULL);
      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      check_aggression(ch);
      trip_triggers(ch, OBJ_TRIG_ACTION_WORD, portal, NULL, OT_SPEC_JUMP);
      
      if (portal->value[0] != -1)
	{
	  portal->value[0]--;
	  if (portal->value[0] == -1)
	    portal->value[0] = 0;
	}
      if (old_room == location)
	return;
      for (fch = old_room->people; fch != NULL; fch = fch_next)
	{
	  fch_next = fch->next_in_room;
	  if (portal == NULL || portal->value[0] == 0)

	    /* no following through dead portals */
	    continue;
	  if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
	      && fch->position < POS_STANDING)
	    do_stand (fch, "");
	  if (fch->master == ch && fch->position == POS_STANDING)
	    {
	      if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
		  && (IS_NPC (fch) && 
			  (IS_SET (fch->act, ACT_AGGRESSIVE)
			   || IS_SET (fch->act2, ACT_AGGIE_GOOD)
                           || IS_SET (fch->act2, ACT_AGGIE_NEUTRAL)
                           || IS_SET (fch->act2, ACT_AGGIE_EVIL))))
		{
		  act ("You can't bring $N into the city.", ch, NULL,
		       fch, TO_CHAR);
		  act ("You aren't allowed in the city.", fch, NULL,
		       NULL, TO_CHAR);
		  continue;
		}
	      act ("Like the fool you are, you jump after $N.", fch,
		   NULL, ch, TO_CHAR);
	      do_jump (fch, argument);
	    }
	}
      if (portal != NULL && portal->value[0] == 0)
	{
	  act ("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
	  if (ch->in_room == old_room)
	    act ("$p fades out of existence.", ch, portal, NULL, TO_ROOM);

	  else if (old_room->people != NULL)
	    {
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_CHAR);
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_ROOM);
	    }
	  extract_obj (portal);
	}
      return;
    }
  send_to_char ("You can't do that.\n\r", ch);
  return;
}

void do_tug (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location = NULL,*prev;
  OBJ_DATA *obj;
  char tcbuf[15];
  if (ch->fighting != NULL)
    return;
  if (argument[0] != '\0')
    {
      ROOM_INDEX_DATA *old_room;
      OBJ_DATA *portal;
      CHAR_DATA *fch, *fch_next;
      old_room = ch->in_room;
      portal = get_obj_list (ch, argument, ch->in_room->contents);
      if (portal == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
      if (!IS_SET (portal->value[4], PACT_TUG))
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if (portal->item_type != ITEM_PORTAL
	  || (IS_SET (portal->value[1], EX_CLOSED)
	      && !IS_TRUSTED (ch, DEMIGOD)))
	{
	  send_to_char ("You can't seem to get a good enough grip.\n\r", ch);
	  return;
	}
      if (!IS_TRUSTED (ch, DEMIGOD)
	  && !IS_SET (portal->value[2], GATE_NOCURSE)
	  && (IS_AFFECTED (ch, AFF_CURSE)))
	{
	  send_to_char ("Something prevents you from tugging.\n\r", ch);
	  return;
	}
      if (IS_SET (portal->value[2], GATE_TRANSPORT))
	{
	  for (obj = object_list; obj != NULL; obj = obj->next)
	    {
	      if (!can_see_obj (ch, obj))
		continue;
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->in_room != NULL)
		{
		  location = get_room_index (obj->in_room->vnum);
		  break;
		}
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->carried_by != NULL)
		{
		  location = obj->carried_by->in_room;
		  break;
		}
	    }
	}

      else if (IS_SET (portal->value[2], GATE_RANDOM)
	       || portal->value[3] == -1)
	{
	  location = get_random_room (ch);
	  portal->value[3] = location->vnum;
	}

      else if (IS_SET (portal->value[2], GATE_BUGGY)
	       && (number_percent () < 5))
	location = get_random_room (ch);

      else
	location = get_room_index (portal->value[3]);
      if (location == NULL
	  || location == old_room || !can_see_room (ch, location)
	  || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	{
	  act ("You tug at $p.", ch, portal, NULL,
	       TO_CHAR);
	  return;
	}
      //Iblis 6/21/03 - Battle Royale room off limit checking
      if (!br_vnum_check (location->vnum))
	{
	  send_to_char
	    ("That room is off limits during `iBattle Royale``\r\n", ch);
	  return;
	}
      if (IS_NPC (ch) && 
		      (IS_SET (ch->act, ACT_AGGRESSIVE)
		       || IS_SET (ch->act2, ACT_AGGIE_GOOD)
                       || IS_SET (ch->act2, ACT_AGGIE_NEUTRAL)
                       || IS_SET (ch->act2, ACT_AGGIE_EVIL))
	  && IS_SET (location->room_flags, ROOM_LAW))
	{
	  send_to_char ("Something prevents you from tugging.\n\r", ch);
	  return;
	}
      if (!can_move_char (ch, location, FALSE, FALSE))
	{
	  send_to_char ("Something prevents you from tugging.\n\r", ch);
	  return;
	}

/*** NEW STUFF ***/
      if (portal->pIndexData->string1[0] != '\0')
	act (portal->pIndexData->string1, ch, portal, NULL, TO_ROOM);

      else
	act ("$n tugs at $p.", ch, portal, NULL, TO_ROOM);
      if (portal->pIndexData->string2[0] != '\0')
	act (portal->pIndexData->string2, ch, portal, NULL, TO_CHAR);

      else
	{
	  act ("You tug at $p.", ch, portal, NULL, TO_CHAR);
	}
      if (ch->mount != NULL)
	{
	  char_from_room (ch->mount);
	  char_to_room (ch->mount, location);
	}
      prev = ch->in_room;
      char_from_room (ch);
      char_to_room (ch, location);
      if (IS_SET (portal->value[2], GATE_GOWITH))
	{
	  obj_from_room (portal);
	  obj_to_room (portal, location);
	}
      if (portal->pIndexData->string3[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string3, ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n has arrived.", ch, portal, fch, TO_VICT);
	  }
      if (portal->pIndexData->string4[0] != '\0');
      act (portal->pIndexData->string4, ch, portal, NULL, TO_CHAR);

/*** NEW STUFF ***/
      do_look (ch, "auto");

      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(ch,"room",ch->in_room,NULL);
      trap_check(ch,"tug",prev,NULL);
      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      check_aggression(ch);
      trip_triggers(ch, OBJ_TRIG_ACTION_WORD, portal, NULL, OT_SPEC_TUG);
      
      if (portal->value[0] != -1)
	{
	  portal->value[0]--;
	  if (portal->value[0] == -1)
	    portal->value[0] = 0;
	}
      if (old_room == location)
	return;
      for (fch = old_room->people; fch != NULL; fch = fch_next)
	{
	  fch_next = fch->next_in_room;
	  if (portal == NULL || portal->value[0] == 0)

	    /* no following through dead portals */
	    continue;
	  if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
	      && fch->position < POS_STANDING)
	    do_stand (fch, "");
	  if (fch->master == ch && fch->position == POS_STANDING)
	    {
	      if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
		  && (IS_NPC (fch) && 
			  (IS_SET (fch->act, ACT_AGGRESSIVE)
			   || IS_SET (fch->act2, ACT_AGGIE_GOOD)
                           || IS_SET (fch->act2, ACT_AGGIE_NEUTRAL)
                           || IS_SET (fch->act2, ACT_AGGIE_EVIL))))
		{
		  act ("You can't bring $N into the city.", ch, NULL,
		       fch, TO_CHAR);
		  act ("You aren't allowed in the city.", fch, NULL,
		       NULL, TO_CHAR);
		  continue;
		}
	      act ("Following $N's lead, you tug at $p.", fch, portal,
		   ch, TO_CHAR);
	      do_tug (fch, argument);
	    }
	}
      if (portal != NULL && portal->value[0] == 0)
	{
	  act ("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
	  if (ch->in_room == old_room)
	    act ("$p fades out of existence.", ch, portal, NULL, TO_ROOM);

	  else if (old_room->people != NULL)
	    {
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_CHAR);
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_ROOM);
	    }
	  extract_obj (portal);
	}
      return;
    }
  send_to_char ("You can't do that.\n\r", ch);
  return;
}

void do_ring (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location = NULL,*prev;
  OBJ_DATA *obj;
  char tcbuf[15];
  if (ch->fighting != NULL)
    return;
  if (argument[0] != '\0')
    {
      ROOM_INDEX_DATA *old_room;
      OBJ_DATA *portal;
      CHAR_DATA *fch, *fch_next;
      old_room = ch->in_room;
      portal = get_obj_list (ch, argument, ch->in_room->contents);
      if (portal == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
      if (!IS_SET (portal->value[4], PACT_RING))
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if (portal->item_type != ITEM_PORTAL
	  || (IS_SET (portal->value[1], EX_CLOSED)
	      && !IS_TRUSTED (ch, DEMIGOD)))
	{
	  send_to_char ("You can't ring that.\n\r", ch);
	  return;
	}
      if (!IS_TRUSTED (ch, DEMIGOD)
	  && !IS_SET (portal->value[2], GATE_NOCURSE)
	  && (IS_AFFECTED (ch, AFF_CURSE)))
	{
	  send_to_char ("Something prevents you from ringing that.\n\r", ch);
	  return;
	}
      if (IS_SET (portal->value[2], GATE_TRANSPORT))
	{
	  for (obj = object_list; obj != NULL; obj = obj->next)
	    {
	      if (!can_see_obj (ch, obj))
		continue;
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->in_room != NULL)
		{
		  location = get_room_index (obj->in_room->vnum);
		  break;
		}
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->carried_by != NULL)
		{
		  location = obj->carried_by->in_room;
		  break;
		}
	    }
	}

      else if (IS_SET (portal->value[2], GATE_RANDOM)
	       || portal->value[3] == -1)
	{
	  location = get_random_room (ch);
	  portal->value[3] = location->vnum;
	}

      else if (IS_SET (portal->value[2], GATE_BUGGY)
	       && (number_percent () < 5))
	location = get_random_room (ch);

      else
	location = get_room_index (portal->value[3]);
      if (location == NULL
	  || location == old_room || !can_see_room (ch, location)
	  || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	{
	  act ("You ring $p.", ch, portal,
	       NULL, TO_CHAR);
	  return;
	}
      //Iblis 6/21/03 - Battle Royale room off limit checking
      if (!br_vnum_check (location->vnum))
	{
	  send_to_char
	    ("That room is off limits during `iBattle Royale``\r\n", ch);
	  return;
	}
      if (IS_NPC (ch) && 
		      (IS_SET (ch->act, ACT_AGGRESSIVE)
		       || IS_SET (ch->act2, ACT_AGGIE_GOOD)
                       || IS_SET (ch->act2, ACT_AGGIE_NEUTRAL)
                       || IS_SET (ch->act2, ACT_AGGIE_EVIL))
	  && IS_SET (location->room_flags, ROOM_LAW))
	{
	  send_to_char ("Something prevents you from ringing that.\n\r", ch);
	  return;
	}
      if (!can_move_char (ch, location, FALSE, FALSE))
	{
	  send_to_char ("Something prevents you from ringing that.\n\r", ch);
	  return;
	}

/*** NEW STUFF ***/
      if (portal->pIndexData->string1[0] != '\0')
	act (portal->pIndexData->string1, ch, portal, NULL, TO_ROOM);

      else
	act ("$n rings $p.", ch, portal, NULL, TO_ROOM);
      if (portal->pIndexData->string2[0] != '\0')
	act (portal->pIndexData->string2, ch, portal, NULL, TO_CHAR);

      else
	{
	  act ("You ring $p.", ch, portal, NULL, TO_CHAR);
	}
      if (ch->mount != NULL)
	{
	  char_from_room (ch->mount);
	  char_to_room (ch->mount, location);
	}
      prev = ch->in_room;
      char_from_room (ch);
      char_to_room (ch, location);
      if (IS_SET (portal->value[2], GATE_GOWITH))
	{
	  obj_from_room (portal);
	  obj_to_room (portal, location);
	}
      if (portal->pIndexData->string3[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string3, ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n has arrived.", ch, portal, fch, TO_VICT);
	  }
      if (portal->pIndexData->string4[0] != '\0');
      act (portal->pIndexData->string4, ch, portal, NULL, TO_CHAR);
      do_look (ch, "auto");

      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(ch,"room",ch->in_room,NULL);
	trap_check(ch,"ring",prev,NULL);
      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      check_aggression(ch);
      trip_triggers(ch, OBJ_TRIG_ACTION_WORD, portal, NULL, OT_SPEC_RING);
      
      if (portal->value[0] != -1)
	{
	  portal->value[0]--;
	  if (portal->value[0] == -1)
	    portal->value[0] = 0;
	}
      if (old_room == location)
	return;
      for (fch = old_room->people; fch != NULL; fch = fch_next)
	{
	  fch_next = fch->next_in_room;
	  if (portal == NULL || portal->value[0] == 0)

	    /* no following through dead portals */
	    continue;
	  if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
	      && fch->position < POS_STANDING)
	    do_stand (fch, "");
	  if (fch->master == ch && fch->position == POS_STANDING)
	    {
	      if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
		  && (IS_NPC (fch) && 
			  (IS_SET (fch->act, ACT_AGGRESSIVE)
			   || IS_SET (fch->act2, ACT_AGGIE_GOOD)
                           || IS_SET (fch->act2, ACT_AGGIE_NEUTRAL)
                           || IS_SET (fch->act2, ACT_AGGIE_EVIL))))
		{
		  act ("You can't bring $N into the city.", ch, NULL,
		       fch, TO_CHAR);
		  act ("You aren't allowed in the city.", fch, NULL,
		       NULL, TO_CHAR);
		  continue;
		}
	      act ("Following $N's lead, you ring $p too.", fch, portal,
		   ch, TO_CHAR);
	      do_ring (fch, argument);
	    }
	}
      if (portal != NULL && portal->value[0] == 0)
	{
	  act ("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
	  if (ch->in_room == old_room)
	    act ("$p fades out of existence.", ch, portal, NULL, TO_ROOM);

	  else if (old_room->people != NULL)
	    {
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_CHAR);
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_ROOM);
	    }
	  extract_obj (portal);
	}
      return;
    }
  send_to_char ("You can't do that.\n\r", ch);
  return;
}

void do_smash (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location = NULL,*prev;
  OBJ_DATA *obj;
  char tcbuf[15];
  if (ch->fighting != NULL || IS_SET (ch->act, PLR_JAILED))
    return;
  if (argument[0] != '\0')
    {
      ROOM_INDEX_DATA *old_room;
      OBJ_DATA *portal;
      CHAR_DATA *fch, *fch_next;
      old_room = ch->in_room;
      portal = get_obj_list (ch, argument, ch->in_room->contents);
      if (portal == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
      if (!IS_SET (portal->value[4], PACT_SMASH))
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if (ch->dueler)
        {
          send_to_char ("None of that while super dueling.\n\r", ch);
	  return;
        }
      if (portal->item_type != ITEM_PORTAL
	  || (IS_SET (portal->value[1], EX_CLOSED)
	      && !IS_TRUSTED (ch, DEMIGOD)))
	{
	  send_to_char ("You can't seem to smash that.\n\r", ch);
	  return;
	}
      if (!IS_TRUSTED (ch, DEMIGOD)
	  && !IS_SET (portal->value[2], GATE_NOCURSE)
	  && (IS_AFFECTED (ch, AFF_CURSE)))
	{
	  send_to_char ("Something prevents you from smashing that.\n\r", ch);
	  return;
	}
      if (IS_SET (portal->value[2], GATE_TRANSPORT))
	{
	  for (obj = object_list; obj != NULL; obj = obj->next)
	    {
	      if (!can_see_obj (ch, obj))
		continue;
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->in_room != NULL)
		{
		  location = get_room_index (obj->in_room->vnum);
		  break;
		}
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->carried_by != NULL)
		{
		  location = obj->carried_by->in_room;
		  break;
		}
	    }
	}

      else if (IS_SET (portal->value[2], GATE_RANDOM)
	       || portal->value[3] == -1)
	{
	  location = get_random_room (ch);
	  portal->value[3] = location->vnum;
	}

      else if (IS_SET (portal->value[2], GATE_BUGGY)
	       && (number_percent () < 5))
	location = get_random_room (ch);

      else
	location = get_room_index (portal->value[3]);
      if (location == NULL
	  || location == old_room || !can_see_room (ch, location)
	  || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	{
	  act ("You can't seem to smash $p right now.", ch, portal, NULL,
	       TO_CHAR);
	  return;
	}
      //Iblis 6/21/03 - Battle Royale room off limit checking
      if (!br_vnum_check (location->vnum))
	{
	  send_to_char
	    ("That room is off limits during `iBattle Royale``\r\n", ch);
	  return;
	}
      if (IS_NPC (ch) && 
		      (IS_SET (ch->act, ACT_AGGRESSIVE)
		       || IS_SET (ch->act2, ACT_AGGIE_GOOD)
                       || IS_SET (ch->act2, ACT_AGGIE_NEUTRAL)
                       || IS_SET (ch->act2, ACT_AGGIE_EVIL))
	  && IS_SET (location->room_flags, ROOM_LAW))
	{
	  send_to_char ("Something prevents you from smashing that.\n\r", ch);
	  return;
	}
      if (!can_move_char (ch, location, FALSE, FALSE))
	{
	  send_to_char ("Something prevents you from smashing that.\n\r", ch);
	  return;
	}

/*** NEW STUFF ***/
      if (portal->pIndexData->string1[0] != '\0')
	act (portal->pIndexData->string1, ch, portal, NULL, TO_ROOM);

      else
	act ("$n smashes $p.", ch, portal, NULL, TO_ROOM);
      if (portal->pIndexData->string2[0] != '\0')
	act (portal->pIndexData->string2, ch, portal, NULL, TO_CHAR);

      else
	{
	  act ("You smash $p.", ch, portal, NULL, TO_CHAR);
	}
      if (ch->mount != NULL)
	{
	  char_from_room (ch->mount);
	  char_to_room (ch->mount, location);
	}
      prev = ch->in_room;
      char_from_room (ch);
      char_to_room (ch, location);
      if (IS_SET (portal->value[2], GATE_GOWITH))
	{
	  obj_from_room (portal);
	  obj_to_room (portal, location);
	}
      if (portal->pIndexData->string3[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string3, ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n has arrived.", ch, portal, fch, TO_VICT);
	  }
      if (portal->pIndexData->string4[0] != '\0');
      act (portal->pIndexData->string4, ch, portal, NULL, TO_CHAR);

/*** NEW STUFF ***/
      do_look (ch, "auto");

      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(ch,"room",ch->in_room,NULL);
	trap_check(ch,"smash",prev,NULL);
      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      check_aggression(ch);
      trip_triggers(ch, OBJ_TRIG_ACTION_WORD, portal, NULL, OT_SPEC_SMASH);
      
      if (portal->value[0] != -1)
	{
	  portal->value[0]--;
	  if (portal->value[0] == -1)
	    portal->value[0] = 0;
	}
      if (old_room == location)
	return;
      for (fch = old_room->people; fch != NULL; fch = fch_next)
	{
	  fch_next = fch->next_in_room;
	  if (portal == NULL || portal->value[0] == 0)

	    /* no following through dead portals */
	    continue;
	  if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
	      && fch->position < POS_STANDING)
	    do_stand (fch, "");
	  if (fch->master == ch && fch->position == POS_STANDING)
	    {
	      if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
		  && (IS_NPC (fch) && 
			  (IS_SET (fch->act, ACT_AGGRESSIVE)
			   || IS_SET (fch->act2, ACT_AGGIE_GOOD)
                           || IS_SET (fch->act2, ACT_AGGIE_NEUTRAL)
                           || IS_SET (fch->act2, ACT_AGGIE_EVIL))))
		  
		{
		  act ("You can't bring $N into the city.", ch, NULL,
		       fch, TO_CHAR);
		  act ("You aren't allowed in the city.", fch, NULL,
		       NULL, TO_CHAR);
		  continue;
		}
	      act ("Feeling that $N hasn't done enough, you smash $p too.",
		   fch, portal, ch, TO_CHAR);
	      do_smash (fch, argument);
	    }
	}
      if (portal != NULL && portal->value[0] == 0)
	{
	  act ("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
	  if (ch->in_room == old_room)
	    act ("$p fades out of existence.", ch, portal, NULL, TO_ROOM);

	  else if (old_room->people != NULL)
	    {
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_CHAR);
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_ROOM);
	    }
	  extract_obj (portal);
	}
      return;
    }
  send_to_char ("You can't do that.\n\r", ch);
  return;
}

void do_shove (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location = NULL,*prev;
  OBJ_DATA *obj;
  char tcbuf[15];
  if (ch->fighting != NULL)
    return;
  if (argument[0] != '\0')
    {
      ROOM_INDEX_DATA *old_room;
      OBJ_DATA *portal;
      CHAR_DATA *fch, *fch_next;
      old_room = ch->in_room;
      portal = get_obj_list (ch, argument, ch->in_room->contents);
      if (portal == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
      if (!IS_SET (portal->value[4], PACT_SHOVE))
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if (portal->item_type != ITEM_PORTAL
	  || (IS_SET (portal->value[1], EX_CLOSED)
	      && !IS_TRUSTED (ch, DEMIGOD)))
	{
	  send_to_char
	    ("You can't seem to locate a good shoving position.\n\r", ch);
	  return;
	}
      if (!IS_TRUSTED (ch, DEMIGOD)
	  && !IS_SET (portal->value[2], GATE_NOCURSE)
	  && (IS_AFFECTED (ch, AFF_CURSE)))
	{
	  send_to_char ("Something prevents you from shoving.\n\r", ch);
	  return;
	}
      if (IS_SET (portal->value[2], GATE_TRANSPORT))
	{
	  for (obj = object_list; obj != NULL; obj = obj->next)
	    {
	      if (!can_see_obj (ch, obj))
		continue;
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->in_room != NULL)
		{
		  location = get_room_index (obj->in_room->vnum);
		  break;
		}
	      if (obj->pIndexData->vnum == portal->value[3]
		  && obj->carried_by != NULL)
		{
		  location = obj->carried_by->in_room;
		  break;
		}
	    }
	}

      else if (IS_SET (portal->value[2], GATE_RANDOM)
	       || portal->value[3] == -1)
	{
	  location = get_random_room (ch);
	  portal->value[3] = location->vnum;
	}

      else if (IS_SET (portal->value[2], GATE_BUGGY)
	       && (number_percent () < 5))
	location = get_random_room (ch);

      else
	location = get_room_index (portal->value[3]);
      if (location == NULL
	  || location == old_room || !can_see_room (ch, location)
	  || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	{
	  act ("Shoving $p seems too dangerous.", ch, portal, NULL, TO_CHAR);
	  return;
	}
      //Iblis 6/21/03 - Battle Royale room off limit checking
      if (!br_vnum_check (location->vnum))
	{
	  send_to_char
	    ("That room is off limits during `iBattle Royale``\r\n", ch);
	  return;
	}
      if (IS_NPC (ch) && 
		      (IS_SET (ch->act, ACT_AGGRESSIVE)
		       || IS_SET (ch->act2, ACT_AGGIE_GOOD)
                       || IS_SET (ch->act2, ACT_AGGIE_NEUTRAL)
                       || IS_SET (ch->act2, ACT_AGGIE_EVIL))
	  && IS_SET (location->room_flags, ROOM_LAW))
	{
	  send_to_char ("Something prevents you from shoving.\n\r", ch);
	  return;
	}
      if (!can_move_char (ch, location, FALSE, FALSE))
	{
	  send_to_char ("Something prevents you from shoving.\n\r", ch);
	  return;
	}

/*** NEW STUFF ***/
      if (portal->pIndexData->string1[0] != '\0')
	act (portal->pIndexData->string1, ch, portal, NULL, TO_ROOM);

      else
	act ("$n shoves $p.", ch, portal, NULL, TO_ROOM);
      if (portal->pIndexData->string2[0] != '\0')
	act (portal->pIndexData->string2, ch, portal, NULL, TO_CHAR);

      else
	{
	  act ("You shove $p.", ch, portal, NULL, TO_CHAR);
	}
      if (ch->mount != NULL)
	{
	  char_from_room (ch->mount);
	  char_to_room (ch->mount, location);
	}
      prev = ch->in_room;
      char_from_room (ch);
      char_to_room (ch, location);
      if (IS_SET (portal->value[2], GATE_GOWITH))
	{
	  obj_from_room (portal);
	  obj_to_room (portal, location);
	}
      if (portal->pIndexData->string3[0] != '\0')
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act (portal->pIndexData->string3, ch, portal, fch, TO_VICT);
	  }

      else
	for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
	  {
	    fch_next = fch->next_in_room;
	    if (can_see (fch, ch))
	      act ("$n has arrived.", ch, portal, fch, TO_VICT);
	  }
      if (portal->pIndexData->string4[0] != '\0');
      act (portal->pIndexData->string4, ch, portal, NULL, TO_CHAR);

/*** NEW STUFF ***/
      do_look (ch, "auto");

      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(ch,"room",ch->in_room,NULL);
	trap_check(ch,"shove",prev,NULL);
      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      check_aggression(ch);
      trip_triggers(ch, OBJ_TRIG_ACTION_WORD, portal, NULL, OT_SPEC_SHOVE);
      
      if (portal->value[0] != -1)
	{
	  portal->value[0]--;
	  if (portal->value[0] == -1)
	    portal->value[0] = 0;
	}
      if (old_room == location)
	return;
      for (fch = old_room->people; fch != NULL; fch = fch_next)
	{
	  fch_next = fch->next_in_room;
	  if (portal == NULL || portal->value[0] == 0)

	    /* no following through dead portals */
	    continue;
	  if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
	      && fch->position < POS_STANDING)
	    do_stand (fch, "");
	  if (fch->master == ch && fch->position == POS_STANDING)
	    {
	      if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
		  && (IS_NPC (fch) && 
			  (IS_SET (fch->act, ACT_AGGRESSIVE)
			    || IS_SET (fch->act2, ACT_AGGIE_GOOD)
                            || IS_SET (fch->act2, ACT_AGGIE_NEUTRAL)
                            || IS_SET (fch->act2, ACT_AGGIE_EVIL))))
		{
		  act ("You can't bring $N into the city.", ch, NULL,
		       fch, TO_CHAR);
		  act ("You aren't allowed in the city.", fch, NULL,
		       NULL, TO_CHAR);
		  continue;
		}
	      act ("Following $N's lead, you shove $p too.", fch,
		   portal, ch, TO_CHAR);
	      do_shove (fch, argument);
	    }
	}
      if (portal != NULL && portal->value[0] == 0)
	{
	  act ("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
	  if (ch->in_room == old_room)
	    act ("$p fades out of existence.", ch, portal, NULL, TO_ROOM);

	  else if (old_room->people != NULL)
	    {
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_CHAR);
	      act ("$p fades out of existence.", old_room->people,
		   portal, NULL, TO_ROOM);
	    }
	  extract_obj (portal);
	}
      return;
    }
  send_to_char ("You can't do that.\n\r", ch);
  return;
}

void do_dig (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location = NULL;
  OBJ_DATA *obj;
  char tcbuf[15];
  if (ch->fighting != NULL)
    return;
  if (argument[0] != '\0')
    {
      ROOM_INDEX_DATA *old_room;
      OBJ_DATA *portal;
      CHAR_DATA *fch, *fch_next;
      old_room = ch->in_room;
      portal = get_obj_list (ch, argument, ch->in_room->contents);
      if (portal == NULL)
        {
          send_to_char ("You don't see that here.\n\r", ch);
          return;
        }
      if (!IS_SET (portal->value[4], PACT_DIG))
        {
          send_to_char ("You can't do that.\n\r", ch);
          return;
        }
      if (portal->item_type != ITEM_PORTAL
          || (IS_SET (portal->value[1], EX_CLOSED)
              && !IS_TRUSTED (ch, DEMIGOD)))
        {
          send_to_char ("You can't seem to get a good enough grip.\n\r", ch);
          return;
        }
      if (!IS_TRUSTED (ch, DEMIGOD)
          && !IS_SET (portal->value[2], GATE_NOCURSE)
          && (IS_AFFECTED (ch, AFF_CURSE)))
        {
          send_to_char ("Something prevents you from tugging.\n\r", ch);
          return;
        }
      if (IS_SET (portal->value[2], GATE_TRANSPORT))
        {
          for (obj = object_list; obj != NULL; obj = obj->next)
            {
              if (!can_see_obj (ch, obj))
                continue;
              if (obj->pIndexData->vnum == portal->value[3]
                  && obj->in_room != NULL)
                {
                  location = get_room_index (obj->in_room->vnum);
                  break;
                }
              if (obj->pIndexData->vnum == portal->value[3]
                  && obj->carried_by != NULL)
                {
                  location = obj->carried_by->in_room;
                  break;
                }
            }
        }
      else if (IS_SET (portal->value[2], GATE_RANDOM)
               || portal->value[3] == -1)
        {
          location = get_random_room (ch);
          portal->value[3] = location->vnum;
        }

      else if (IS_SET (portal->value[2], GATE_BUGGY)
               && (number_percent () < 5))
        location = get_random_room (ch);

      else
        location = get_room_index (portal->value[3]);
      if (location == NULL
          || location == old_room || !can_see_room (ch, location)
          || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
        {
          act ("You dig into $p.", ch, portal, NULL,
               TO_CHAR);
          return;
        }
      //Iblis 6/21/03 - Battle Royale room off limit checking
      if (!br_vnum_check (location->vnum))
        {
          send_to_char
            ("That room is off limits during `iBattle Royale``\r\n", ch);
          return;
        }
      if (IS_NPC (ch) &&
	  (IS_SET (ch->act, ACT_AGGRESSIVE)
	   || IS_SET (ch->act2, ACT_AGGIE_GOOD)
	   || IS_SET (ch->act2, ACT_AGGIE_NEUTRAL)
	   || IS_SET (ch->act2, ACT_AGGIE_EVIL))
          && IS_SET (location->room_flags, ROOM_LAW))
        {
          send_to_char ("Something prevents you from digging.\n\r", ch);
          return;
        }
      if (!can_move_char (ch, location, FALSE, FALSE))
        {
          send_to_char ("Something prevents you from digging.\n\r", ch);
          return;
        }

      /*** NEW STUFF ***/
      if (portal->pIndexData->string1[0] != '\0')
        act (portal->pIndexData->string1, ch, portal, NULL, TO_ROOM);

      else
        act ("$n dig into $p.", ch, portal, NULL, TO_ROOM);
      if (portal->pIndexData->string2[0] != '\0')
        act (portal->pIndexData->string2, ch, portal, NULL, TO_CHAR);

      else
        {
          act ("You dig into $p.", ch, portal, NULL, TO_CHAR);
        }  if (ch->mount != NULL)
	  {
	    char_from_room (ch->mount);
	    char_to_room (ch->mount, location);
	  }
      char_from_room (ch);
      char_to_room (ch, location);
      if (IS_SET (portal->value[2], GATE_GOWITH))
        {
          obj_from_room (portal);
          obj_to_room (portal, location);
        }
      if (portal->pIndexData->string3[0] != '\0')
        for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
          {
            fch_next = fch->next_in_room;
            if (can_see (fch, ch))
              act (portal->pIndexData->string3, ch, portal, fch, TO_VICT);
          }

      else
        for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
          {
            fch_next = fch->next_in_room;
            if (can_see (fch, ch))
              act ("$n has arrived.", ch, portal, fch, TO_VICT);
          }
      if (portal->pIndexData->string4[0] != '\0');
      act (portal->pIndexData->string4, ch, portal, NULL, TO_CHAR);

      /*** NEW STUFF ***/
      do_look (ch, "auto");

      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(ch,"room",ch->in_room,NULL);
      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      check_aggression(ch);
      trip_triggers(ch, OBJ_TRIG_ACTION_WORD, portal, NULL, OT_SPEC_DIG);

      if (portal->value[0] != -1)
        {
          portal->value[0]--;
          if (portal->value[0] == -1)
            portal->value[0] = 0;
        }
      if (old_room == location)
        return;
      for (fch = old_room->people; fch != NULL; fch = fch_next)
        {
          fch_next = fch->next_in_room;
          if (portal == NULL || portal->value[0] == 0)

            /* no following through dead portals */
            continue;
          if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
              && fch->position < POS_STANDING)
            do_stand (fch, "");
	  if (fch->master == ch && fch->position == POS_STANDING)
            {
              if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
                  && (IS_NPC (fch) &&
		      (IS_SET (fch->act, ACT_AGGRESSIVE)
		       || IS_SET (fch->act2, ACT_AGGIE_GOOD)
		       || IS_SET (fch->act2, ACT_AGGIE_NEUTRAL)
		       || IS_SET (fch->act2, ACT_AGGIE_EVIL))))
                {
                  act ("You can't bring $N into the city.", ch, NULL,
                       fch, TO_CHAR);
                  act ("You aren't allowed in the city.", fch, NULL,
                       NULL, TO_CHAR);
                  continue;
                }
              act ("Following $N's lead, you tug at $p.", fch, portal,
                   ch, TO_CHAR);
              do_tug (fch, argument);
            }
        }
      if (portal != NULL && portal->value[0] == 0)
        {
          act ("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
          if (ch->in_room == old_room)
            act ("$p fades out of existence.", ch, portal, NULL, TO_ROOM);

          else if (old_room->people != NULL)
            {
              act ("$p fades out of existence.", old_room->people,
                   portal, NULL, TO_CHAR);
              act ("$p fades out of existence.", old_room->people,
                   portal, NULL, TO_ROOM);
            }
          extract_obj (portal);
        }
      return;
    }
  send_to_char ("You can't do that.\n\r", ch);
  return;
}
