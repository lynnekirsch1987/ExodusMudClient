#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include "merc.h"
#include "interp.h"
#include "clans/new_clans.h"
struct vnum_list
{
  int vnum;
  struct vnum_list *next;
};
struct vnum_list *linked_rooms;

extern const char *dir_name[];
//extern const sh_int movement_loss[SECT_MAX];
bool paddling = FALSE;


void do_drive (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location, *new_loc;
  char direction[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  EXIT_DATA *pexit;
  CHAR_DATA *vch;
  OBJ_DATA *eObj;
  sh_int door;
  if (!IS_SET (ch->in_room->room_flags, ROOM_TRANSPORT))
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
    }
  one_argument (argument, direction);
  if (!str_cmp (direction, "n") || !str_cmp (direction, "north"))
    door = 0;

  else if (!str_cmp (direction, "e") || !str_cmp (direction, "east"))
    door = 1;

  else if (!str_cmp (direction, "s") || !str_cmp (direction, "south"))
    door = 2;

  else if (!str_cmp (direction, "w") || !str_cmp (direction, "west"))
    door = 3;

  else if (!str_cmp (direction, "u") || !str_cmp (direction, "up"))
    door = 4;

  else if (!str_cmp (direction, "d") || !str_cmp (direction, "down"))
    door = 5;

  else
    {
      send_to_char ("Drive in which direction?\n\r", ch);
      return;
    }
  for (eObj = object_list; eObj; eObj = eObj->next)
    {
      if (eObj->item_type != ITEM_CTRANSPORT)
	continue;
      if (eObj->value[3] == ch->in_room->vnum)
	{
	  location = eObj->in_room;
	  if (location == NULL
	      || location == ch->in_room || !can_see_room (ch, location)
	      || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	    {
	      act ("You can't seem to drive that way.", ch, NULL, NULL,
		   TO_CHAR);
	      return;
	    }
	  if (!IS_SET (eObj->value[4], TACT_DRIVE))
	    {
	      send_to_char ("You can't do that here.\n\r", ch);
	      return;
	    }
	  if (location->exit[door] == NULL)
	    {
	      send_to_char ("You can't drive that way.\n\r", ch);
	      return;
	    }
	  if ((pexit = location->exit[door]) == NULL)
	    {
	      bug ("Nonexistant exit using transport.", 0);
	      return;
	    }
	  if (IS_SET (pexit->exit_info, EX_CLOSED))
	    {
	      send_to_char ("You drive directly into the door.\n\r", ch);
	      return;
	    }
	  new_loc = location->exit[door]->u1.to_room;
	  if (new_loc == NULL
	      || new_loc == ch->in_room
	      || new_loc->clan != CLAN_BOGUS
	      || !can_see_room (ch, new_loc)
	      || (room_is_private (new_loc) && !IS_TRUSTED (ch, OVERLORD)))
	    {
	      act ("You can't seem to drive that way.", ch, NULL, NULL,
		   TO_CHAR);
	      return;
	    }
	  if (IS_SET (new_loc->room_flags2, ROOM_NOVEHICLE))
	    {
	      send_to_char ("You are not allowed to drive there.\n\r", ch);
	      return;
	    }
	  if (!IS_SET (new_loc->room_flags, ROOM_PIER))
	    switch (new_loc->sector_type)
	      {
	      case SECT_INSIDE:
		if (!IS_SET (eObj->value[0], TSECT_INSIDE))
		  {
		    act ("$p is not capable of travelling indoors.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_CITY:
		if (!IS_SET (eObj->value[0], TSECT_CITY))
		  {
		    act ("$p is not capable of travelling in the city.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_FIELD:
		if (!IS_SET (eObj->value[0], TSECT_FIELD))
		  {
		    act ("$p is not capable of travelling over fields.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_FOREST:
		if (!IS_SET (eObj->value[0], TSECT_FOREST))
		  {
		    act ("$p is not capable of travelling in the forest.", ch,
			 eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_HILLS:
		if (!IS_SET (eObj->value[0], TSECT_HILLS))
		  {
		    act ("$p is not capable of travelling on hills.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_MOUNTAIN:
		if (!IS_SET (eObj->value[0], TSECT_MOUNTAIN))
		  {
		    act ("$p is not capable of travelling on mountains.", ch,
			 eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WATER_SWIM:
		if (!IS_SET (eObj->value[0], TSECT_WATER_SWIM))
		  {
		    act ("$p is not capable of travelling over water.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WATER_NOSWIM:
		if (!IS_SET (eObj->value[0], TSECT_WATER_NOSWIM))
		  {
		    act ("$p is not capable of travelling over water.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_AIR:
		if (!IS_SET (eObj->value[0], TSECT_AIR))
		  {
		    act ("$p is not capable of travelling in the air.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_DESERT:
		if (!IS_SET (eObj->value[0], TSECT_DESERT))
		  {
		    act ("$p is not capable of travelling in the desert.", ch,
			 eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_UNDERWATER:
		if (!IS_SET (eObj->value[0], TSECT_UNDERWATER))
		  {
		    act ("$p is not capable of travelling underwater.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_GRASSLAND:
		if (!IS_SET (eObj->value[0], TSECT_GRASSLAND))
		  {
		    act ("$p is not capable of travelling across grasslands.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_TUNDRA:
		if (!IS_SET (eObj->value[0], TSECT_TUNDRA))
		  {
		    act ("$p is not capable of travelling across the tundra.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_SWAMP:
		if (!IS_SET (eObj->value[0], TSECT_SWAMP))
		  {
		    act ("$p is not capable of travelling in swampy areas.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WATER_OCEAN:
		if (!IS_SET (eObj->value[0], TSECT_WATER_OCEAN))
		  {
		    act ("$p is not capable of travelling over the ocean.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WASTELAND:
		if (!IS_SET (eObj->value[0], TSECT_WASTELAND))
		  {
		    act
		      ("$p is not capable of travelling across the wasteland.",
		       ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_SHORE:
		if (!IS_SET (eObj->value[0], TSECT_SHORE))
		{
			act ("$p is not capable of travelling on the shore.",
					ch, eObj, NULL, TO_CHAR);
			return;
		}
		break;
	      case SECT_UNDERGROUND:
                if (!IS_SET (eObj->value[0], TSECT_UNDERGROUND))
		  {
		    act ("$p is not capable of travelling underground.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
                break;
	      }
	  obj_from_room (eObj);
	  obj_to_room (eObj, new_loc);
	  do_look (ch, "out");

	  /*      sprintf(tcbuf, "Outside of %s, you see...  \n\r",eObj->short_descr);
	     send_to_char(tcbuf, ch);
	     send_to_char(new_loc->description, ch);
	     send_to_char("\n\r", ch);
	     do_exits(ch, "rrcar");
	     show_list_to_char_but(new_loc->contents, ch, FALSE, FALSE, eObj);
	     show_char_to_char(new_loc->people, ch); */
	  sprintf (tcbuf, "%s has arrived.", fcapitalize (eObj->short_descr));
	  for (vch = new_loc->people; vch; vch = vch->next_in_room)
	    act (tcbuf, vch, NULL, NULL, TO_CHAR);
	  sprintf (tcbuf, "%s has left.", fcapitalize (eObj->short_descr));
	  for (vch = location->people; vch; vch = vch->next_in_room)
	    act (tcbuf, vch, NULL, NULL, TO_CHAR);
	  if (eObj->pIndexData->string1[0] != '\0')
	    send_transport_message (eObj->value[3],
				    eObj->pIndexData->string1);
	  return;
	}
    }
  send_to_char ("You can't do that here.\n\r", ch);
}

void do_sail (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location, *new_loc;
  char direction[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  CHAR_DATA *vch;
  OBJ_DATA *eObj;
  sh_int door;
  if (!IS_SET (ch->in_room->room_flags, ROOM_TRANSPORT))
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
    }
  one_argument (argument, direction);
  if (!str_cmp (direction, "n") || !str_cmp (direction, "north"))
    door = 0;

  else if (!str_cmp (direction, "e") || !str_cmp (direction, "east"))
    door = 1;

  else if (!str_cmp (direction, "s") || !str_cmp (direction, "south"))
    door = 2;

  else if (!str_cmp (direction, "w") || !str_cmp (direction, "west"))
    door = 3;

  else if (!str_cmp (direction, "u") || !str_cmp (direction, "up"))
    door = 4;

  else if (!str_cmp (direction, "d") || !str_cmp (direction, "down"))
    door = 5;

  else
    {
      send_to_char ("Sail in which direction?\n\r", ch);
      return;
    }
  for (eObj = object_list; eObj; eObj = eObj->next)
    {
      if (eObj->item_type != ITEM_CTRANSPORT || eObj->in_room == NULL)
	continue;
      if (eObj->value[3] == ch->in_room->vnum)
	{
	  location = eObj->in_room;
	  if (location == NULL || location == ch->in_room
	      /*              || !can_see_room(ch, location) */
	      || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	    {
	      act ("You can't seem to sail that way.", ch, NULL, NULL,
		   TO_CHAR);
	      return;
	    }
	  if (!IS_SET (eObj->value[4], TACT_SAIL))
	    {
	      send_to_char ("You can't do that here.\n\r", ch);
	      return;
	    }
	  if (location->exit[door] == NULL)
	    {
	      send_to_char ("You can't sail that way.\n\r", ch);
	      return;
	    }
	  new_loc = location->exit[door]->u1.to_room;
	  if (new_loc == NULL || new_loc == ch->in_room
	      /*              || !can_see_room(ch, new_loc) */
	      || (room_is_private (new_loc) && !IS_TRUSTED (ch, OVERLORD)))
	    {
	      act ("You can't seem to sail that way.", ch, NULL, NULL,
		   TO_CHAR);
	      return;
	    }
	  if (!IS_SET (new_loc->room_flags, ROOM_PIER))
	    switch (new_loc->sector_type)
	      {
	      case SECT_INSIDE:
		if (!IS_SET (eObj->value[0], TSECT_INSIDE))
		  {
		    act ("$p is not capable of travelling indoors.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_CITY:
		if (!IS_SET (eObj->value[0], TSECT_CITY))
		  {
		    act ("$p is not capable of travelling in the city.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_FIELD:
		if (!IS_SET (eObj->value[0], TSECT_FIELD))
		  {
		    act ("$p is not capable of travelling over fields.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_FOREST:
		if (!IS_SET (eObj->value[0], TSECT_FOREST))
		  {
		    act ("$p is not capable of travelling in the forest.", ch,
			 eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_HILLS:
		if (!IS_SET (eObj->value[0], TSECT_HILLS))
		  {
		    act ("$p is not capable of travelling on hills.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_MOUNTAIN:
		if (!IS_SET (eObj->value[0], TSECT_MOUNTAIN))
		  {
		    act ("$p is not capable of travelling on mountains.", ch,
			 eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WATER_SWIM:
		if (!IS_SET (eObj->value[0], TSECT_WATER_SWIM))
		  {
		    act ("$p is not capable of travelling over water.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WATER_NOSWIM:
		if (!IS_SET (eObj->value[0], TSECT_WATER_NOSWIM))
		  {
		    act ("$p is not capable of travelling over water.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_AIR:
		if (!IS_SET (eObj->value[0], TSECT_AIR))
		  {
		    act ("$p is not capable of travelling in the air.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_DESERT:
		if (!IS_SET (eObj->value[0], TSECT_DESERT))
		  {
		    act ("$p is not capable of travelling in the desert.", ch,
			 eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_UNDERWATER:
		if (!IS_SET (eObj->value[0], TSECT_UNDERWATER))
		  {
		    act ("$p is not capable of travelling underwater.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_GRASSLAND:
		if (!IS_SET (eObj->value[0], TSECT_GRASSLAND))
		  {
		    act ("$p is not capable of travelling across grasslands.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_TUNDRA:
		if (!IS_SET (eObj->value[0], TSECT_TUNDRA))
		  {
		    act ("$p is not capable of travelling across the tundra.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_SWAMP:
		if (!IS_SET (eObj->value[0], TSECT_SWAMP))
		  {
		    act ("$p is not capable of travelling in swampy areas.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WATER_OCEAN:
		if (!IS_SET (eObj->value[0], TSECT_WATER_OCEAN))
		  {
		    act ("$p is not capable of travelling over the ocean.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WASTELAND:
		if (!IS_SET (eObj->value[0], TSECT_WASTELAND))
		  {
		    act
		      ("$p is not capable of travelling across the wasteland.",
		       ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_SHORE:
		if (!IS_SET (eObj->value[0], TSECT_SHORE))
		{
			act ("$p is not capable of travelling on the shore.",
					ch, eObj, NULL, TO_CHAR);
			return;
		}
		break;
	      case SECT_UNDERGROUND:
                if (!IS_SET (eObj->value[0], TSECT_UNDERGROUND))
		  {
		    act ("$p is not capable of travelling underground.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
                break;
	      }
	  obj_from_room (eObj);
	  obj_to_room (eObj, new_loc);
	  do_look (ch, "out");

	  /*
	     sprintf(tcbuf, "Outside of %s, you see...  \n\r",eObj->short_descr);
	     send_to_char(tcbuf, ch);
	     send_to_char(new_loc->description, ch);
	     send_to_char("\n\r", ch);
	     do_exits(ch, "rrcar");
	     show_list_to_char_but(new_loc->contents, ch, FALSE, FALSE, eObj);
	     show_char_to_char(new_loc->people, ch); */
	  sprintf (tcbuf, "%s has arrived.", fcapitalize (eObj->short_descr));
	  for (vch = new_loc->people; vch; vch = vch->next_in_room)
	    act (tcbuf, vch, NULL, NULL, TO_CHAR);
	  sprintf (tcbuf, "%s has left.", fcapitalize (eObj->short_descr));
	  for (vch = location->people; vch; vch = vch->next_in_room)
	    act (tcbuf, vch, NULL, NULL, TO_CHAR);
	  if (eObj->pIndexData->string1[0] != '\0')
	    send_transport_message (eObj->value[3],
				    eObj->pIndexData->string1);
	  return;
	}
    }
  send_to_char ("You can't do that here.\n\r", ch);
}

void do_row (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location, *new_loc;
  char direction[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  CHAR_DATA *vch;
  OBJ_DATA *eObj;
  sh_int door;
  if (!IS_SET (ch->in_room->room_flags, ROOM_TRANSPORT))
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
    }
  one_argument (argument, direction);
  if (!str_cmp (direction, "n") || !str_cmp (direction, "north"))
    door = 0;

  else if (!str_cmp (direction, "e") || !str_cmp (direction, "east"))
    door = 1;

  else if (!str_cmp (direction, "s") || !str_cmp (direction, "south"))
    door = 2;

  else if (!str_cmp (direction, "w") || !str_cmp (direction, "west"))
    door = 3;

  else if (!str_cmp (direction, "u") || !str_cmp (direction, "up"))
    door = 4;

  else if (!str_cmp (direction, "d") || !str_cmp (direction, "down"))
    door = 5;

  else
    {
      send_to_char ("Row in which direction?\n\r", ch);
      return;
    }
  for (eObj = object_list; eObj; eObj = eObj->next)
    {
      if (eObj->item_type != ITEM_CTRANSPORT)
	continue;
      if (eObj->value[3] == ch->in_room->vnum)
	{
	  location = eObj->in_room;
	  if (location == NULL
	      || location == ch->in_room || !can_see_room (ch, location)
	      || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	    {
	      act ("You can't seem to row that way.", ch, NULL, NULL,
		   TO_CHAR);
	      return;
	    }
	  if (!IS_SET (eObj->value[4], TACT_ROW))
	    {
	      send_to_char ("You can't do that here.\n\r", ch);
	      return;
	    }
	  if (location->exit[door] == NULL)
	    {
	      send_to_char ("You can't row that way.\n\r", ch);
	      return;
	    }
	  new_loc = location->exit[door]->u1.to_room;
	  if (new_loc == NULL
	      || new_loc == ch->in_room || !can_see_room (ch, new_loc)
	      || (room_is_private (new_loc) && !IS_TRUSTED (ch, OVERLORD)))
	    {
	      act ("You can't seem to row that way.", ch, NULL, NULL,
		   TO_CHAR);
	      return;
	    }
	  if (!IS_SET (new_loc->room_flags, ROOM_PIER))
	    switch (new_loc->sector_type)
	      {
	      case SECT_INSIDE:
		if (!IS_SET (eObj->value[0], TSECT_INSIDE))
		  {
		    act ("$p is not capable of travelling indoors.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_CITY:
		if (!IS_SET (eObj->value[0], TSECT_CITY))
		  {
		    act ("$p is not capable of travelling in the city.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_FIELD:
		if (!IS_SET (eObj->value[0], TSECT_FIELD))
		  {
		    act ("$p is not capable of travelling over fields.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_FOREST:
		if (!IS_SET (eObj->value[0], TSECT_FOREST))
		  {
		    act ("$p is not capable of travelling in the forest.", ch,
			 eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_HILLS:
		if (!IS_SET (eObj->value[0], TSECT_HILLS))
		  {
		    act ("$p is not capable of travelling on hills.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_MOUNTAIN:
		if (!IS_SET (eObj->value[0], TSECT_MOUNTAIN))
		  {
		    act ("$p is not capable of travelling on mountains.", ch,
			 eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WATER_SWIM:
		if (!IS_SET (eObj->value[0], TSECT_WATER_SWIM))
		  {
		    act ("$p is not capable of travelling over water.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WATER_NOSWIM:
		if (!IS_SET (eObj->value[0], TSECT_WATER_NOSWIM))
		  {
		    act ("$p is not capable of travelling over water.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_AIR:
		if (!IS_SET (eObj->value[0], TSECT_AIR))
		  {
		    act ("$p is not capable of travelling in the air.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_DESERT:
		if (!IS_SET (eObj->value[0], TSECT_DESERT))
		  {
		    act ("$p is not capable of travelling in the desert.", ch,
			 eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_UNDERWATER:
		if (!IS_SET (eObj->value[0], TSECT_UNDERWATER))
		  {
		    act ("$p is not capable of travelling underwater.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_GRASSLAND:
		if (!IS_SET (eObj->value[0], TSECT_GRASSLAND))
		  {
		    act ("$p is not capable of travelling across grasslands.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_TUNDRA:
		if (!IS_SET (eObj->value[0], TSECT_TUNDRA))
		  {
		    act ("$p is not capable of travelling across the tundra.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_SWAMP:
		if (!IS_SET (eObj->value[0], TSECT_SWAMP))
		  {
		    act ("$p is not capable of travelling in swampy areas.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WATER_OCEAN:
		if (!IS_SET (eObj->value[0], TSECT_WATER_OCEAN))
		  {
		    act ("$p is not capable of travelling over the ocean.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WASTELAND:
		if (!IS_SET (eObj->value[0], TSECT_WASTELAND))
		  {
		    act
		      ("$p is not capable of travelling across the wasteland.",
		       ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_SHORE:
		if (!IS_SET (eObj->value[0], TSECT_SHORE))
		{
			act ("$p is not capable of travelling on the shore.",
					ch, eObj, NULL, TO_CHAR);
			return;
		}
		break;
	      case SECT_UNDERGROUND:
                if (!IS_SET (eObj->value[0], TSECT_UNDERGROUND))
		  {
		    act ("$p is not capable of travelling underground.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
                break;
	      }
	  obj_from_room (eObj);
	  obj_to_room (eObj, new_loc);
	  do_look (ch, "out");

	  /*
	     sprintf(tcbuf, "Outside of %s, you see...  \n\r",eObj->short_descr);
	     send_to_char(tcbuf, ch);
	     send_to_char(new_loc->description, ch);
	     send_to_char("\n\r", ch);
	     do_exits(ch, "rrcar");
	     show_list_to_char_but(new_loc->contents, ch, FALSE, FALSE, eObj);
	     show_char_to_char(new_loc->people, ch); */
	  sprintf (tcbuf, "%s has arrived.", fcapitalize (eObj->short_descr));
	  for (vch = new_loc->people; vch; vch = vch->next_in_room)
	    act (tcbuf, vch, NULL, NULL, TO_CHAR);
	  sprintf (tcbuf, "%s has left.", fcapitalize (eObj->short_descr));
	  for (vch = location->people; vch; vch = vch->next_in_room)
	    act (tcbuf, vch, NULL, NULL, TO_CHAR);
	  if (eObj->pIndexData->string1[0] != '\0')
	    send_transport_message (eObj->value[3],
				    eObj->pIndexData->string1);
	  return;
	}
    }
  send_to_char ("You can't do that here.\n\r", ch);
}

/*void do_paddle (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location, *new_loc;
  char direction[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  CHAR_DATA *vch;
  OBJ_DATA *eObj;
  sh_int door;
  if (!IS_SET (ch->in_room->room_flags, ROOM_TRANSPORT))
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
    }
  one_argument (argument, direction);
  if (!str_cmp (direction, "n") || !str_cmp (direction, "north"))
    door = 0;

  else if (!str_cmp (direction, "e") || !str_cmp (direction, "east"))
    door = 1;

  else if (!str_cmp (direction, "s") || !str_cmp (direction, "south"))
    door = 2;

  else if (!str_cmp (direction, "w") || !str_cmp (direction, "west"))
    door = 3;

  else if (!str_cmp (direction, "u") || !str_cmp (direction, "up"))
    door = 4;

  else if (!str_cmp (direction, "d") || !str_cmp (direction, "down"))
    door = 5;

  else
    {
      send_to_char ("Paddle in which direction?\n\r", ch);
      return;
    }
  for (eObj = object_list; eObj; eObj = eObj->next)
    {
      if (eObj->item_type != ITEM_CTRANSPORT)
	continue;
      if (eObj->value[3] == ch->in_room->vnum)
	{
	  location = eObj->in_room;
	  if (location == NULL
	      || location == ch->in_room || !can_see_room (ch, location)
	      || (room_is_private (location) && !IS_TRUSTED (ch, OVERLORD)))
	    {
	      act ("You can't seem to paddle that way.", ch, NULL, NULL,
		   TO_CHAR);
	      return;
	    }
	  if (!IS_SET (eObj->value[4], TACT_PADDLE))
	    {
	      send_to_char ("You can't do that here.\n\r", ch);
	      return;
	    }
	  if (location->exit[door] == NULL)
	    {
	      send_to_char ("You can't paddle that way.\n\r", ch);
	      return;
	    }
	  new_loc = location->exit[door]->u1.to_room;
	  if (new_loc == NULL
	      || new_loc == ch->in_room || !can_see_room (ch, new_loc)
	      || (room_is_private (new_loc) && !IS_TRUSTED (ch, OVERLORD)))
	    {
	      act ("You can't seem to paddle that way.", ch, NULL, NULL,
		   TO_CHAR);
	      return;
	    }
	  if (!IS_SET (new_loc->room_flags, ROOM_PIER))
	    switch (new_loc->sector_type)
	      {
	      case SECT_INSIDE:
		if (!IS_SET (eObj->value[0], TSECT_INSIDE))
		  {
		    act ("$p is not capable of travelling indoors.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_CITY:
		if (!IS_SET (eObj->value[0], TSECT_CITY))
		  {
		    act ("$p is not capable of travelling in the city.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_FIELD:
		if (!IS_SET (eObj->value[0], TSECT_FIELD))
		  {
		    act ("$p is not capable of travelling over fields.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_FOREST:
		if (!IS_SET (eObj->value[0], TSECT_FOREST))
		  {
		    act ("$p is not capable of travelling in the forest.", ch,
			 eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_HILLS:
		if (!IS_SET (eObj->value[0], TSECT_HILLS))
		  {
		    act ("$p is not capable of travelling on hills.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_MOUNTAIN:
		if (!IS_SET (eObj->value[0], TSECT_MOUNTAIN))
		  {
		    act ("$p is not capable of travelling on mountains.", ch,
			 eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WATER_SWIM:
		if (!IS_SET (eObj->value[0], TSECT_WATER_SWIM))
		  {
		    act ("$p is not capable of travelling over water.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WATER_NOSWIM:
		if (!IS_SET (eObj->value[0], TSECT_WATER_NOSWIM))
		  {
		    act ("$p is not capable of travelling over water.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_AIR:
		if (!IS_SET (eObj->value[0], TSECT_AIR))
		  {
		    act ("$p is not capable of travelling in the air.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_DESERT:
		if (!IS_SET (eObj->value[0], TSECT_DESERT))
		  {
		    act ("$p is not capable of travelling in the desert.", ch,
			 eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_UNDERWATER:
		if (!IS_SET (eObj->value[0], TSECT_UNDERWATER))
		  {
		    act ("$p is not capable of travelling underwater.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_GRASSLAND:
		if (!IS_SET (eObj->value[0], TSECT_GRASSLAND))
		  {
		    act ("$p is not capable of travelling across grasslands.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_TUNDRA:
		if (!IS_SET (eObj->value[0], TSECT_TUNDRA))
		  {
		    act ("$p is not capable of travelling across the tundra.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_SWAMP:
		if (!IS_SET (eObj->value[0], TSECT_SWAMP))
		  {
		    act ("$p is not capable of travelling in swampy areas.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WATER_OCEAN:
		if (!IS_SET (eObj->value[0], TSECT_WATER_OCEAN))
		  {
		    act ("$p is not capable of travelling over the ocean.",
			 ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_WASTELAND:
		if (!IS_SET (eObj->value[0], TSECT_WASTELAND))
		  {
		    act
		      ("$p is not capable of travelling across the wasteland.",
		       ch, eObj, NULL, TO_CHAR);
		    return;
		  }
		break;
	      case SECT_SHORE:
		if (!IS_SET (eObj->value[0], TSECT_SHORE))
		{
			act ("$p is not capable of travelling on the shore.",
					ch, eObj, NULL, TO_CHAR);
			return;
		}
		break;
	      }
	  obj_from_room (eObj);
	  obj_to_room (eObj, new_loc);
	  do_look (ch, "out");

	  *
	     sprintf(tcbuf, "Outside of %s, you see...  \n\r",eObj->short_descr);
	     send_to_char(tcbuf, ch);
	     send_to_char(new_loc->description, ch);
	     send_to_char("\n\r", ch);
	     do_exits(ch, "rrcar");
	     show_list_to_char_but(new_loc->contents, ch, FALSE, FALSE, eObj);
	     show_char_to_char(new_loc->people, ch); *
	  sprintf (tcbuf, "%s has arrived.", fcapitalize (eObj->short_descr));
	  for (vch = new_loc->people; vch; vch = vch->next_in_room)
	    act (tcbuf, vch, NULL, NULL, TO_CHAR);
	  sprintf (tcbuf, "%s has left.", fcapitalize (eObj->short_descr));
	  for (vch = location->people; vch; vch = vch->next_in_room)
	    act (tcbuf, vch, NULL, NULL, TO_CHAR);
	  if (eObj->pIndexData->string1[0] != '\0')
	    send_transport_message (eObj->value[3],
				    eObj->pIndexData->string1);
	  return;
	}
    }
  send_to_char ("You can't do that here.\n\r", ch);
}
*/

int have_visited (int room_vnum)
{
	struct vnum_list *this_room;
	for (this_room = linked_rooms; this_room; this_room = this_room->next)
		if (this_room->vnum == room_vnum)
			return (1);
	return (0);
}

void trace_links (CHAR_DATA * ch, ROOM_INDEX_DATA * room_link)
{
	int door_dir;
	struct vnum_list *this_room;
	if (room_link == NULL || have_visited (room_link->vnum))
		return;
	this_room = malloc (sizeof (struct vnum_list));
//	this_room = reinterpret_cast<vnum_list *>(malloc(sizeof(struct vnum_list)));
	if (this_room == NULL)
		return;
	this_room->vnum = room_link->vnum;
	this_room->next = linked_rooms;
	linked_rooms = this_room;
	for (door_dir = 0; door_dir < MAX_DIR; door_dir++) {
		if (room_link->exit[door_dir] != NULL) {
			if (room_link->exit[door_dir]->u1.to_room != NULL) {
				trace_links (ch, room_link->exit[door_dir]->u1.to_room);
			}
		}
	}
}

void set_transport_links (ROOM_INDEX_DATA * room_link, int ct_vnum)
{
	int door_dir;
	struct vnum_list *this_room;
	if (room_link == NULL || have_visited (room_link->vnum))
		return;
	this_room = malloc (sizeof (struct vnum_list));
//	this_room = reinterpret_cast<vnum_list *>(malloc(sizeof(struct vnum_list)));
	if (this_room == NULL)
		return;
	this_room->vnum = room_link->vnum;
	this_room->next = linked_rooms;
	linked_rooms = this_room;
	room_link->ctransport_vnum = ct_vnum;
	if (!IS_SET(room_link->room_flags, ROOM_TRANSPORT))
		SET_BIT (room_link->room_flags, ROOM_TRANSPORT);
	for (door_dir = 0; door_dir < MAX_DIR; door_dir++) {
		if (room_link->exit[door_dir] != NULL) {
			if (room_link->exit[door_dir]->u1.to_room != NULL) {
				set_transport_links (room_link->exit[door_dir]->u1.to_room, ct_vnum);
			}
		}
	}
}

void send_tmessage_links (ROOM_INDEX_DATA * room_link, char *msg, bool first)
{
	int door_dir;
	struct vnum_list *this_room;
	CHAR_DATA *vch;
	if (NULL == room_link || have_visited (room_link->vnum))
		return;
	this_room = malloc (sizeof (struct vnum_list));
//	this_room = reinterpret_cast<vnum_list *>(malloc(sizeof(struct vnum_list)));
	if (NULL == this_room)
		return;
	this_room->vnum = room_link->vnum;
	this_room->next = linked_rooms;
	linked_rooms = this_room;
	if (!first) {
		char tcbuf[MAX_STRING_LENGTH];
		sprintf (tcbuf, "%s\n\r", msg);
		for (vch = room_link->people; vch; vch = vch->next_in_room)
			send_to_char (tcbuf, vch);
	}
	for (door_dir = 0; door_dir < MAX_DIR; door_dir++) {
		if (room_link->exit[door_dir] != NULL) {
			if (room_link->exit[door_dir]->u1.to_room != NULL) {
				send_tmessage_links (room_link->exit[door_dir]->u1.to_room, msg, FALSE);
			}
		}
	}
}

int get_tweight_links (ROOM_INDEX_DATA * room_link)
{
	int door_dir, this_weight = 0;
	struct vnum_list *this_room;
	CHAR_DATA *vch;
	OBJ_DATA *obj;

	if (NULL == room_link || have_visited (room_link->vnum))
		return (0);
	if ((this_room = malloc (sizeof (struct vnum_list))) == NULL)
//	if (NULL == (this_room = reinterpret_cast<vnum_list *>(malloc(sizeof(struct vnum_list)))))
		return (0);

	this_room->vnum = room_link->vnum;
	this_room->next = linked_rooms;
	linked_rooms = this_room;
	for (vch = room_link->people; vch; vch = vch->next_in_room)
		this_weight += get_char_weight(vch);
	for (obj = room_link->contents; obj; obj = obj->next_content)
		this_weight += obj->weight;
	for (door_dir = 0; door_dir < MAX_DIR; door_dir++) {
		if (room_link->exit[door_dir] != NULL) {
			if (room_link->exit[door_dir]->u1.to_room != NULL) {
				this_weight += get_tweight_links (room_link->exit[door_dir]->u1.to_room);
			}
		}
	}
	return (this_weight);
}

void set_transport_flags (int start_vnum, int ct_vnum)
{
  ROOM_INDEX_DATA *start_room;
  linked_rooms = NULL;
  if ((start_room = get_room_index (start_vnum)) != NULL)
    set_transport_links (start_room, ct_vnum);
}

void send_transport_message (int start_vnum, char *msg)
{
  ROOM_INDEX_DATA *start_room;
  linked_rooms = NULL;
  if ((start_room = get_room_index (start_vnum)) != NULL)
    send_tmessage_links (start_room, msg, TRUE);
}

int get_transport_weight (int start_vnum)
{
  ROOM_INDEX_DATA *start_room;
  int weight = 0;
  linked_rooms = NULL;
  if ((start_room = get_room_index (start_vnum)) != NULL)
    weight = get_tweight_links (start_room);
  return (weight);
}

void do_paddle (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location, *new_loc;
  char direction[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  CHAR_DATA *vch, *vch_next;
  //OBJ_DATA *eObj;
  OBJ_DATA *raft;
  sh_int move;
  sh_int door;
  raft = ch->on;
  if (raft == NULL || raft->item_type != ITEM_RAFT || ch->in_room == NULL)
    {
      send_to_char("You must be on a raft to paddle.\n\r",ch);
      return;
    }
  /*  if (!IS_SET (ch->in_room->room_flags, ROOM_TRANSPORT))
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
      }*/
  one_argument (argument, direction);
  if (!str_cmp (direction, "n") || !str_cmp (direction, "north"))
    door = 0;

  else if (!str_cmp (direction, "e") || !str_cmp (direction, "east"))
    door = 1;

  else if (!str_cmp (direction, "s") || !str_cmp (direction, "south"))
    door = 2;

  else if (!str_cmp (direction, "w") || !str_cmp (direction, "west"))
    door = 3;

  else if (!str_cmp (direction, "u") || !str_cmp (direction, "up"))
    door = 4;

  else if (!str_cmp (direction, "d") || !str_cmp (direction, "down"))
    door = 5;

  else
    {
      send_to_char ("Paddle in which direction?\n\r", ch);
      return;
    }
  location = ch->in_room;
  if (location->exit[door] == NULL)
    {
      send_to_char ("You can't paddle that way.\n\r", ch);
      return;
    }
  new_loc = location->exit[door]->u1.to_room;
  if (new_loc == NULL || new_loc == ch->in_room
      /*              || !can_see_room(ch, new_loc) */
      || (room_is_private (new_loc) && !IS_TRUSTED (ch, OVERLORD)))
    {
      act ("You can't seem to paddle that way.", ch, NULL, NULL,
	   TO_CHAR);
      return;
    }
  if (!IS_SET (new_loc->room_flags, ROOM_PIER))
    switch (new_loc->sector_type)
      {
      case SECT_INSIDE:
	if (!IS_SET (raft->value[3], TSECT_INSIDE))
	  {
	    act ("$p is not capable of travelling indoors.",
		 ch, raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_CITY:
	if (!IS_SET (raft->value[3], TSECT_CITY))
	  {
	    act ("$p is not capable of travelling in the city.",
		 ch, raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_FIELD:
	if (!IS_SET (raft->value[3], TSECT_FIELD))
	  {
	    act ("$p is not capable of travelling over fields.",
		 ch, raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_FOREST:
	if (!IS_SET (raft->value[3], TSECT_FOREST))
	  {
	    act ("$p is not capable of travelling in the forest.", ch,
		 raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_HILLS:
	if (!IS_SET (raft->value[3], TSECT_HILLS))
	  {
	    act ("$p is not capable of travelling on hills.",
		 ch, raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_MOUNTAIN:
	if (!IS_SET (raft->value[3], TSECT_MOUNTAIN))
	  {
	    act ("$p is not capable of travelling on mountains.", ch,
		 raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_WATER_SWIM:
	if (!IS_SET (raft->value[3], TSECT_WATER_SWIM))
	  {
	    act ("$p is not capable of travelling over water.",
		 ch, raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_WATER_NOSWIM:
	if (!IS_SET (raft->value[3], TSECT_WATER_NOSWIM))
	  {
	    act ("$p is not capable of travelling over water.",
		 ch, raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_AIR:
	if (!IS_SET (raft->value[3], TSECT_AIR))
	  {
	    act ("$p is not capable of travelling in the air.",
		 ch, raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_DESERT:
	if (!IS_SET (raft->value[3], TSECT_DESERT))
	  {
	    act ("$p is not capable of travelling in the desert.", ch,
		 raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_UNDERWATER:
	if (!IS_SET (raft->value[3], TSECT_UNDERWATER))
	  {
	    act ("$p is not capable of travelling underwater.",
		 ch, raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_GRASSLAND:
	if (!IS_SET (raft->value[3], TSECT_GRASSLAND))
	  {
	    act ("$p is not capable of travelling across grasslands.",
		 ch, raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_TUNDRA:
	if (!IS_SET (raft->value[3], TSECT_TUNDRA))
	  {
	    act ("$p is not capable of travelling across the tundra.",
		 ch, raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_SWAMP:
	if (!IS_SET (raft->value[3], TSECT_SWAMP))
	  {
	    act ("$p is not capable of travelling in swampy areas.",
		 ch, raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_WATER_OCEAN:
	if (!IS_SET (raft->value[3], TSECT_WATER_OCEAN))
	  {
	    act ("$p is not capable of travelling over the ocean.",
		 ch, raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_WASTELAND:
	if (!IS_SET (raft->value[3], TSECT_WASTELAND))
	  {
                    act
                      ("$p is not capable of travelling across the wasteland.",
                       ch, raft, NULL, TO_CHAR);
                    return;
	  }
	break;
      case SECT_SHORE:
	if (!IS_SET (raft->value[3], TSECT_SHORE))
	  {
	    act ("$p is not capable of travelling on the shore.",
		 ch, raft, NULL, TO_CHAR);
	    return;
	  }
	break;
      case SECT_UNDERGROUND:
        if (!IS_SET (raft->value[3], TSECT_UNDERGROUND))
          {
            act ("$p is not capable of travelling underground.",
                 ch, raft, NULL, TO_CHAR);
            return;
          }
        break;
      }
  move = movement_loss[UMIN (SECT_MAX - 1, ch->in_room->sector_type)]
    + movement_loss[UMIN (SECT_MAX - 1, new_loc->sector_type)];
  move /= 2;                /* i.e. the average */
  if (get_carry_weight (ch) > can_carry_w (ch))
    move *= 2;
  if (IS_AFFECTED (ch, AFF_FLYING) || IS_AFFECTED (ch, AFF_HASTE))
    move /= 2;
  if (IS_AFFECTED (ch, AFF_SLOW))
    move *= 2;
  if (ch->move < move)
    {
      send_to_char ("You are too exhausted.\n\r", ch);
      return;
    }
  else ch->move -= move;
  paddling = TRUE;
  obj_from_room (raft);
  obj_to_room (raft, new_loc);
  paddling = FALSE;
  if (new_loc->people)
    act("$p is paddled into view.",new_loc->people,raft,NULL,TO_ALL);
  location = ch->in_room;
  for (vch = ch->in_room->people;vch;vch = vch_next)
  {
    vch_next = vch->next_in_room;
    if (vch->on == raft)
      {
//	act("$n has paddled away in $p.",vch,raft,NULL,TO_ROOM);
	char_from_room(vch);
	char_to_room(vch,new_loc);
	vch->on = raft;
	if (vch == ch)
	{
	  sprintf(tcbuf, "You paddle %s %s.\n\r",raft->short_descr,dir_name[door]);
	  send_to_char(tcbuf,vch);
	  //	  ch->move -= move;
	}
	else
	{
	  sprintf(tcbuf, "$n paddles $p %s.",dir_name[door]);
	  act(tcbuf,ch,raft,vch,TO_VICT);
	}
	do_look(vch,"");
	WAIT_STATE (vch, 1);
//	act("$n has arrived in $p.",vch,raft,NULL,TO_ROOM);
      }
  }
  if (location->people)
    act("$p is paddled away.",location->people,raft,NULL,TO_ALL);
  //obj_from_room (raft);
  //obj_to_room (raft, new_loc);
      
/*  sprintf (tcbuf, "%s has arrived.", fcapitalize (raft->short_descr));
  for (vch = new_loc->people; vch; vch = vch->next_in_room)
    act (tcbuf, vch, NULL, NULL, TO_CHAR);
  sprintf (tcbuf, "%s has left.", fcapitalize (raft->short_descr));
  for (vch = location->people; vch; vch = vch->next_in_room)
    act (tcbuf, vch, NULL, NULL, TO_CHAR);*/
/*  if (raft->pIndexData->string1[0] != '\0')
    send_transport_message (raft->value[3],
			    raft->pIndexData->string1);*/
  return;
}



