/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,         *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.    *
 *                                                                          *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael           *
 *  Chastain, Michael Quan, and Mitchell Tse.                               *
 *                                                                          *
 *  In order to use any part of this Merc Diku Mud, you must comply with    *
 *  both the original Diku license in 'license.doc' as well the Merc        *
 *  license in 'license.txt'.  In particular, you may not remove either of  *
 *  these copyright notices.                                                *
 *                                                                          *
 *  Much time and thought has gone into this software and you are           *
 *  benefitting.  We hope that you share your changes too.  What goes       *
 *  around, comes around.                                                   *
 ****************************************************************************/

/***************************************************************************
 *       ROM 2.4 is copyright 1993-1998 Russ Taylor                         *
 *       ROM has been brought to you by the ROM consortium                  *
 *           Russ Taylor (rtaylor@hypercube.org)                            *
 *           Gabrielle Taylor (gtaylor@hypercube.org)                       *
 *           Brian Moore (zump@rom.org)                                     *
 *       By using this code, you have agreed to follow the terms of the     *
 *       ROM license, in the file Rom24/doc/rom.license                     *
 ****************************************************************************/

#if defined(macintosh)
#include <types.h>
#else /*  */
#include <sys/types.h>
#endif /*  */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "clans/new_clans.h"

//Iblis created file, to deal with trophy fishing

void catch_fish args ((CHAR_DATA * ch));
short hotspot_proximity args ((CHAR_DATA * ch));
void check_top_fisher args ((CHAR_DATA * ch, OBJ_DATA * obj));

#define FISHING_NOT            0
#define FISHING_CASTED         1
#define FISHING_ROUND_1        2
#define FISHING_ROUND_2        3
#define FISHING_NOT_REELED     4
#define FISHING_REELED         5
#define FISHING_SLACKEN        6
#define FISHING_HT_NOT_REELED  7
#define FISHING_HT_REELED      8
#define FISHING_HT_SLACKEN     9
#define FISHING_BAIT_GONE      10
int hotspot[250];

//Note - I changed most of the send_to_chars to be acts to deal properly with a player sleeping, 
//since logically you can still fish when you sleep, you will just not notice your pole shaking :P
typedef struct top_fisher TOP_FISHER;
struct top_fisher
{
  short weight;
  char name[20];
  char fish_name[50];
};
TOP_FISHER top_fishers[25];

//Iblis - Baiting a fish (dealing with trophy fishing)
void do_bait (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *pole, *bait;
  if ((pole = get_eq_char (ch, WEAR_HOLD)) == NULL
      || pole->item_type != ITEM_POLE)
    {
      send_to_char
	("You must be holding a fishing pole, or else you have nothing to bait!\n\r",
	 ch);
      return;
    }
  if (ch->pcdata && ch->pcdata->fishing)
    {
      send_to_char
	("How do you expect do bait your hook when its in the water?\n\r",
	 ch);
      return;
    }
  for (bait = ch->carrying; bait != NULL; bait = bait->next_content)
    if (bait->item_type == ITEM_BAIT)
      break;
  if (bait == NULL)
    {
      send_to_char
	("What do you intend to bait that hook with? Fish aren't THAT stupid.\n\r",
	 ch);
      return;
    }
  if (pole->value[1] == 1)
    {
      send_to_char
	("Your pole is already baited.  More bait does not equal bigger fish.\n\r",
	 ch);
      return;
    }
  act ("You bait your hook with $p.", ch, bait, NULL, TO_CHAR);
  act ("$n baits $s hook with $p.", ch, bait, NULL, TO_ROOM);
  obj_from_char (bait);
  extract_obj (bait);

  //Add something for chars that are Master Baiters...
  pole->value[1] = 1;		//Set the pole flag to baited
}

//Iblis - casting (for trophy fishing).  Called in magic.c from do_cast if "cast line" is typed by a player
void cast_line (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *pole;
  if (IS_NPC(ch))
	  return;
  if (ch->in_room->sector_type != SECT_WATER_OCEAN)
    {
      send_to_char ("You can only fish in the ocean.\n\r", ch);
      return;
    }
  if ((pole = get_eq_char (ch, WEAR_HOLD)) == NULL
      || pole->item_type != ITEM_POLE)
    {
      send_to_char
	("You must be holding a fishing pole, or else you have nothing to cast!\n\r",
	 ch);
      return;
    }
  if (ch->pcdata && ch->pcdata->fishing)
    {
      send_to_char
	("You can't cast a line if your line is in the water!\n\r", ch);
      return;
    }
  if (!pole->value[1])
    {
      send_to_char
	("You cast your line, but it's not likely a fish will latch onto your empty hook by accident.\n\r",
	 ch);
      return;
    }
  send_to_char
    ("You cast your line, and wait patiently for those fish to come a bitin!\n\r",
     ch);
  act
    ("$n leans back, and in one fluid motion brings $s pole forward, casting $s line.",
     ch, NULL, NULL, TO_ROOM);
  ch->pcdata->fishing = FISHING_CASTED;
}

//Iblis - the fishing update function used to do deal with the initial hooking of a fish on the line
//Called from update.c
void update_fishing (void)
{
  DESCRIPTOR_DATA *d;
  OBJ_DATA *pole;
  char buf[80];
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->connected != CON_PLAYING || !d->character
	  || !d->character->pcdata || !d->character->pcdata->fishing)
	continue;

      //Do fishing shit here
      if (d->character->pcdata->fishing == FISHING_CASTED)
	{
	  if (number_range (1, 40) <
	      (get_skill (d->character, gsn_fishing) / 8))
	    {
	      d->character->pcdata->fishing_weight = number_range (1, 250);
	      if (IS_IMMORTAL (d->character))
		{
		  sprintf (buf, "Original weight -> %d, HSP -> %d\n\r",
			   d->character->pcdata->fishing_weight,
			   hotspot_proximity (d->character));
		  send_to_char (buf, d->character);
		}
	      d->character->pcdata->
		fishing_weight /= (hotspot_proximity (d->character));
	      if (d->character->pcdata->fishing_weight < 1)
		d->character->pcdata->fishing_weight = 1;
	      if (d->character->pcdata->fishing_weight == 250)
		d->character->pcdata->fishing_weight += number_range (1, 50);
	      act ("You feel a little `gtug`` on the line.",
		   d->character, NULL, NULL, TO_CHAR);
	      if (IS_IMMORTAL (d->character))
		{
		  sprintf (buf, "Fish weighs -> %d\n\r",
			   d->character->pcdata->fishing_weight);
		  send_to_char (buf, d->character);
		}
	      d->character->pcdata->fishing_ll = 40;
	      d->character->pcdata->fishing = FISHING_ROUND_1;
	    }
	}

      else
	{
	  switch (d->character->pcdata->fishing)
	    {
	    case FISHING_ROUND_1:
	      if (d->character->pcdata->fishing_weight > 100)
		{
		  act ("You feel a `gpull`` on the line.", d->character,
		       NULL, NULL, TO_CHAR);
		  act ("$n's pole moves a little.", d->character, NULL,
		       NULL, TO_ROOM);
		  d->character->pcdata->fishing = FISHING_ROUND_2;
		}

	      else if (number_range (1, 100) >
		       d->character->pcdata->fishing_weight)
		{
		  act
		    ("You feel a pull on the line as the fish frees itself and snacks on your bait.",
		     d->character, NULL, NULL, TO_CHAR);
		  act ("$n's pole moves a little, but is quickly still.",
		       d->character, NULL, NULL, TO_ROOM);
		  d->character->pcdata->fishing = FISHING_BAIT_GONE;
		  pole = get_eq_char (d->character, WEAR_HOLD);
		  if (pole != NULL && pole->item_type == ITEM_POLE)
		    pole->value[1] = 0;	//IE NOT BAITED ANY MORE
		  else
		    bug ("No pole in AAA", 0);
		}
	      break;
	    case FISHING_ROUND_2:
	      if (d->character->pcdata->fishing_weight > 200)
		{
		  act ("You feel a strong `gjerk`` on the line!",
		       d->character, NULL, NULL, TO_CHAR);
		  act ("$n's pole starts to shake.", d->character, NULL,
		       NULL, TO_ROOM);
		  d->character->pcdata->fishing = FISHING_NOT_REELED;
		}

	      else if (number_range (1, 200) >
		       d->character->pcdata->fishing_weight)
		{
		  act
		    ("You feel a strong jerk on the line as the fish frees itself and snacks on your bait.\n\r",
		     d->character, NULL, NULL, TO_CHAR);
		  act ("$n's pole starts to shake, but is quickly still.",
		       d->character, NULL, NULL, TO_ROOM);
		  d->character->pcdata->fishing = FISHING_BAIT_GONE;
		  pole = get_eq_char (d->character, WEAR_HOLD);
		  if (pole != NULL && pole->item_type == ITEM_POLE)
		    pole->value[1] = 0;	//IE NOT BAITED ANY MORE
		  else
		    bug ("No pole in AAB", 0);
		}
	    }
	}
    }
}

//Iblis - the fishing update function used to do deal with the reeling in of a fish on the line
////Called from update.c
void update_reeling (void)
{
  DESCRIPTOR_DATA *d;
  OBJ_DATA *pole;
  short chance;
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->connected != CON_PLAYING || !d->character
	  || !d->character->pcdata || d->character->pcdata->fishing < 4)
	continue;
      chance = number_percent ();
      if (d->character->pcdata->fishing == FISHING_REELED)
	{
	  if (d->character->position < POS_RESTING)
	    {
	      if (chance < 80 && d->character->pcdata->fishing_ll < 100)
		{
		  d->character->pcdata->fishing_ll += 10;
		  continue;
		}

	      else
		{
		  d->character->pcdata->fishing = FISHING_BAIT_GONE;
		  pole = get_eq_char (d->character, WEAR_HOLD);
		  if (pole != NULL && pole->item_type == ITEM_POLE)
		    pole->value[1] = 0;	//IE NOT BAITED ANY MORE
		  else
		    bug ("No pole in AAC", 0);
		  continue;
		}
	    }
	  d->character->pcdata->fishing_ll -= 10;
	  if (d->character->pcdata->fishing_ll <= 0)
	    {
	      catch_fish (d->character);
	    }

	  else if (chance < 45)
	    {
	      send_to_char
		("The line zigs through the water.  Line tension: `j***``.\n\r",
		 d->character);
	      d->character->pcdata->fishing = FISHING_NOT_REELED;
	    }

	  else if (chance <
		   (45 +
		    (100 *
		     (1 /
		      (9 -
		       (d->character->pcdata->fishing_weight * 1.0) / 50)))))
	    {
	      send_to_char
		("The line zigs through the water.  Line tension: `i***``.\n\r",
		 d->character);
	      d->character->pcdata->fishing = FISHING_HT_NOT_REELED;
	    }

	  else
	    {
	      send_to_char
		("The line zigs through the water.  Line tension: `j***``.\n\r",
		 d->character);
	      d->character->pcdata->fishing = FISHING_NOT_REELED;
	    }
	}

      else if (d->character->pcdata->fishing == FISHING_HT_REELED)
	{
	  if (d->character->position < POS_RESTING)
	    {
	      if (chance < 7 && d->character->pcdata->fishing_ll < 100)
		{
		  d->character->pcdata->fishing_ll += 10;
		  continue;
		}

	      else
		{
		  d->character->pcdata->fishing = FISHING_BAIT_GONE;
		  pole = get_eq_char (d->character, WEAR_HOLD);
		  if (pole != NULL && pole->item_type == ITEM_POLE)
		    pole->value[1] = 0;	//IE NOT BAITED ANY MORE
		  else
		    bug ("No pole in AAD", 0);
		  continue;
		}
	    }
	  if (chance < 91)
	    {
	      send_to_char
		("You suddenly feel the line slacken, and you notice your bait gone.\n\r",
		 d->character);
	      d->character->pcdata->fishing = FISHING_BAIT_GONE;
	      if ((pole =
		   get_eq_char (d->character,
				WEAR_HOLD)) ==
		  NULL || pole->item_type != ITEM_POLE)
		{
		  bug ("No pole in AAE", 0);
		}

	      else
		pole->value[1] = 0;
	    }

	  else
	    {
	      d->character->pcdata->fishing_ll -= 10;
	      if (d->character->pcdata->fishing_ll <= 0)
		{		//FISH CATCH
		  catch_fish (d->character);
		}

	      else
		{
		  send_to_char
		    ("The line zigs through the water.  Line tension: `j***``.\n\r",
		     d->character);
		  d->character->pcdata->fishing = FISHING_REELED;
		}
	    }
	}

      else if (d->character->pcdata->fishing == FISHING_HT_NOT_REELED)
	{
	  if (d->character->position < POS_RESTING)
	    {
	      if (chance < 10 && d->character->pcdata->fishing_ll < 100)
		{
		  d->character->pcdata->fishing_ll += 10;
		  continue;
		}

	      else
		{
		  d->character->pcdata->fishing = FISHING_BAIT_GONE;
		  pole = get_eq_char (d->character, WEAR_HOLD);
		  if (pole != NULL && pole->item_type == ITEM_POLE)
		    pole->value[1] = 0;	//IE NOT BAITED ANY MORE
		  else
		    bug ("No pole in AAF", 0);
		  continue;
		}
	    }
	  if (chance < 15)
	    {
	      send_to_char
		("The line zigs through the water.  Line tension: `j***``.\n\r",
		 d->character);
	      d->character->pcdata->fishing = FISHING_NOT_REELED;
	    }

	  else if (chance < 50)
	    {
	      send_to_char
		("The line zigs through the water.  Line tension: `i***``.\n\r",
		 d->character);
	      d->character->pcdata->fishing = FISHING_HT_NOT_REELED;
	    }

	  else
	    {
	      send_to_char
		("You suddenly feel the line slacken, and you notice your bait gone.\n\r",
		 d->character);
	      d->character->pcdata->fishing = FISHING_BAIT_GONE;
	      if ((pole =
		   get_eq_char (d->character,
				WEAR_HOLD)) ==
		  NULL || pole->item_type != ITEM_POLE)
		{
		  bug ("No pole in AAG", 0);
		}

	      else
		pole->value[1] = 0;
	    }
	}

      else if (d->character->pcdata->fishing == FISHING_NOT_REELED)
	{
	  if (d->character->position < POS_RESTING)
	    {
	      if (chance < 55 && d->character->pcdata->fishing_ll < 100)
		{
		  d->character->pcdata->fishing_ll += 10;
		  continue;
		}

	      else
		{
		  d->character->pcdata->fishing = FISHING_BAIT_GONE;
		  pole = get_eq_char (d->character, WEAR_HOLD);
		  if (pole != NULL && pole->item_type == ITEM_POLE)
		    pole->value[1] = 0;	//IE NOT BAITED ANY MORE
		  else
		    bug ("No pole in AAH", 0);
		  continue;
		}
	    }
	  if (chance < 60)
	    {
	      send_to_char
		("The line zigs through the water.  Line tension: `j***``.\n\r",
		 d->character);
	      d->character->pcdata->fishing = FISHING_NOT_REELED;
	    }

	  else
	    {
	      send_to_char
		("You suddenly feel the line slacken, and you notice your bait gone.\n\r",
		 d->character);
	      d->character->pcdata->fishing = FISHING_BAIT_GONE;
	      if ((pole =
		   get_eq_char (d->character,
				WEAR_HOLD)) ==
		  NULL || pole->item_type != ITEM_POLE)
		{
		  bug ("No pole in AAI", 0);
		}

	      else
		pole->value[1] = 0;
	    }
	}

      else if (d->character->pcdata->fishing == FISHING_HT_SLACKEN
	       || d->character->pcdata->fishing == FISHING_SLACKEN)
	{
	  if (d->character->position < POS_RESTING)
	    {
	      if (chance < 90 && d->character->pcdata->fishing_ll < 100)
		{
		  d->character->pcdata->fishing_ll += 10;
		  if (d->character->pcdata->fishing_ll > 100)
		    d->character->pcdata->fishing_ll = 100;
		  continue;
		}

	      else
		{
		  d->character->pcdata->fishing = FISHING_BAIT_GONE;
		  pole = get_eq_char (d->character, WEAR_HOLD);
		  if (pole != NULL && pole->item_type == ITEM_POLE)
		    pole->value[1] = 0;	//IE NOT BAITED ANY MORE
		  else
		    bug ("No pole in AAJ", 0);
		  continue;
		}
	    }
	  if (chance < 95)
	    {
	      send_to_char
		("The line zigs through the water.  Line tension: `j***``.\n\r",
		 d->character);
	      d->character->pcdata->fishing = FISHING_NOT_REELED;
	    }

	  else
	    {
	      send_to_char
		("The line zigs through the water.  Line tension: `i***``.\n\r",
		 d->character);
	      d->character->pcdata->fishing = FISHING_HT_NOT_REELED;
	    }
	  d->character->pcdata->fishing_ll +=
	    (10 + d->character->pcdata->fishing_weight / 10);
	  if (d->character->pcdata->fishing_ll > 100)
	    d->character->pcdata->fishing_ll = 100;
	}
    }
}

//Iblis - the reel command (for use in trophy fishing)
void do_reel (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *pole;
  short value;
  if (!ch || !ch->pcdata || !ch->pcdata->fishing)
    {
      if (ch)
	send_to_char
	  ("You can't reel if you have not cast the reel.\n\r", ch);
      return;
    }
  pole = get_eq_char (ch, WEAR_HOLD);
  if (pole == NULL || pole->item_type != ITEM_POLE)
    {
      send_to_char ("You cannot reel without a pole.\n\r", ch);
      return;
    }
  if (!str_cmp (argument, "line"))
    {
      if (ch->pcdata->fishing < FISHING_NOT_REELED)
	{
	  if (ch->pcdata->fishing == FISHING_ROUND_1)
	    value = 100;

	  else if (ch->pcdata->fishing == FISHING_ROUND_2)
	    value = 200;
	  else
	    {

	      send_to_char
		("You reel in a little, hoping to excite the fish.\n\r", ch);
	      act ("$n reels in a little, for no apparent reason.", ch,
		   NULL, NULL, TO_ROOM);
	      return;
	    }
	  if (number_range (1, value) < ch->pcdata->fishing_weight)
	    {
	      if (number_percent () < 60)
		{
		  send_to_char
		    ("Alas, you feel the line slacken.  You notice the bait is still there though.\n\r",
		     ch);
		  ch->pcdata->fishing = FISHING_CASTED;
		}

	      else
		{
		  send_to_char
		    ("Alas, you feel the line slacken, and notice the fish `idining`` on your bait.\n\r",
		     ch);
		  pole = get_eq_char (ch, WEAR_HOLD);
		  if (pole != NULL && pole->item_type == ITEM_POLE)
		    pole->value[1] = 0;	//IE NOT BAITED ANY MORE
		  else
		    bug ("No pole in AAJ", 0);
		  ch->pcdata->fishing = FISHING_BAIT_GONE;
		}
	      return;
	    }

	  else
	    {
	      send_to_char ("You start to reel in the line.\n\r", ch);
	      act ("$n starts to reel in $s line.", ch, NULL, NULL, TO_ROOM);
	      ch->pcdata->fishing = FISHING_REELED;
	      return;
	    }
	}

      else if (ch->pcdata->fishing == FISHING_NOT_REELED
	       || ch->pcdata->fishing == FISHING_HT_NOT_REELED)
	{
	  send_to_char ("You start to reel in the line some more.\n\r", ch);
	  act ("$n starts to reel in $s line.", ch, NULL, NULL, TO_ROOM);
	  ch->pcdata->fishing++;
	}

      else if (ch->pcdata->fishing == FISHING_BAIT_GONE)
	{
	  send_to_char
	    ("You start to reel in the line, but with no bait your fishing is fairly useless.\n\r",
	     ch);
	  return;
	}

      else
	{
	  send_to_char ("You can only reel so much at once!\n\r", ch);
	  return;
	}
    }

  else if (!str_cmp (argument, "slacken"))
    {
      if (ch->pcdata->fishing == FISHING_HT_NOT_REELED)
	{
	  send_to_char ("You start to slacken the line.\n\r", ch);
	  act ("$n starts to slacken $s line.", ch, NULL, NULL, TO_ROOM);
	  ch->pcdata->fishing = FISHING_HT_SLACKEN;
	  return;
	}
      if (ch->pcdata->fishing == FISHING_NOT_REELED)
	{
	  send_to_char ("You start to slacken the line.\n\r", ch);
	  act ("$n starts to slacken $s line.", ch, NULL, NULL, TO_ROOM);
	  ch->pcdata->fishing = FISHING_SLACKEN;
	  return;
	}
      if (ch->pcdata->fishing == FISHING_REELED
	  || ch->pcdata->fishing == FISHING_HT_REELED)
	{
	  send_to_char
	    ("You just started to reel the line.  You can't slacken now!.\n\r",
	     ch);
	  return;
	}
      if (ch->pcdata->fishing == FISHING_SLACKEN
	  || ch->pcdata->fishing == FISHING_HT_SLACKEN)
	{
	  send_to_char
	    ("You just started to slacken the line. Give it time!\n\r", ch);
	  return;
	}
      if (ch->pcdata->fishing == FISHING_BAIT_GONE)
	{
	  send_to_char
	    ("You start to slacken the line, but with no bait your fishing is fairly useless.\n\r",
	     ch);
	  return;
	}
      send_to_char ("You can't slacken the line now.\n\r", ch);
    }

  else if (!str_cmp (argument, "all"))
    {
      if (ch->pcdata->fishing == FISHING_CASTED
	  || ch->pcdata->fishing == FISHING_BAIT_GONE)
	{
	  send_to_char
	    ("You reel in your line completely and decide to start over.\n\r",
	     ch);
	  act ("$n reels in $s line completely.", ch, NULL, NULL, TO_ROOM);
	  ch->pcdata->fishing = FISHING_NOT;
	  return;
	}

      else
	{
	  send_to_char
	    ("You can't reel all, you have a fish on your line!.\n\r", ch);
	  return;
	}
    }

  else
    send_to_char ("Syntax : Reel line, Reel slacken, or Reel all.\n\r", ch);
}

//Iblis - The function called from update_reeling if the fish is determined to be caught
void catch_fish (CHAR_DATA * ch)
{
  char *obj_name, *obj_keywords;
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj, *pole;
  short rn;
  if (!ch || !ch->pcdata)
    return;
  if (ch->pcdata->fishing_weight <= 10)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_TROPHY_BAIT), 0);
      obj->weight = ch->pcdata->fishing_weight * 10;
    }

  else
    {
      obj = create_object (get_obj_index (OBJ_VNUM_TROPHY_FISH), 0);
      obj->weight = ch->pcdata->fishing_weight * 10;
      if (ch->pcdata->fishing_weight <= 25)
	{
	  obj->cost = ch->pcdata->fishing_weight * 100;
	  obj_name = "a small fish";
	  obj_keywords = "small fish";
	}

      else if (ch->pcdata->fishing_weight <= 50)
	{
	  obj->cost = ch->pcdata->fishing_weight * 100;
	  obj_name = "a medium-sized fish";
	  obj_keywords = "medium-sized fish";
	}

      else if (ch->pcdata->fishing_weight <= 100)
	{
	  obj->cost = ch->pcdata->fishing_weight * 100;
	  obj_name = "a large fish";
	  obj_keywords = "large fish";
	}

      else if (ch->pcdata->fishing_weight <= 150)
	{
	  obj->cost = 10 * ch->pcdata->fishing_weight * 100;
	  obj_name = "a very large fish";
	  obj_keywords = "large fish";
	}

      else if (ch->pcdata->fishing_weight <= 200)
	{
	  obj->cost = 10 * ch->pcdata->fishing_weight * 100;
	  obj_name = "a huge fish";
	  obj_keywords = "huge fish";
	}

      else if (ch->pcdata->fishing_weight <= 250)
	{
	  obj->cost = 20 * ch->pcdata->fishing_weight * 100;
	  obj_name = "an enormous fish";
	  obj_keywords = "enormous fish";
	}

      else
	{
	  obj->cost = 1500000;
	  rn = (number_percent ()) % 21;	//Fish at the end will have less of a chance of being chosen
	  switch (rn)
	    {
	    case 0:
	      obj_name = "Dwarfbane, the legendary bull shark";
	      obj_keywords = "Dwarfbane bull shark fish";
	      break;
	    case 1:
	      obj_name = "Razorfin, the legendary marlin";
	      obj_keywords = "Razorfin marlin fish";
	      break;
	    case 2:
	      obj_name = "Sawtooth, the legendary sawfish";
	      obj_keywords = "Sawtooth swafish fish";
	      break;
	    case 3:
	      obj_name = "Bigsplash, the legendary tarpon";
	      obj_keywords = "Bigsplash tarpon fish";
	      break;
	    case 4:
	      obj_name = "Boatsmasher, the legendary swordfish";
	      obj_keywords = "Boatsmasher swordfish fish";
	      break;
	    case 5:
	      obj_name = "Silver Arrow, the legendary sturgeon";
	      obj_keywords = "Silver Arrow sturgeon fish";
	      break;
	    case 6:
	      obj_name = "Flatsnout, the legendary hammerhead shark";
	      obj_keywords = "Flatsnout hammerhead shark fish";
	      break;
	    case 7:
	      obj_name = "Bazkarakk, the legendary giant squid";
	      obj_keywords = "Bazkarakk giant squid fish";
	      break;
	    case 8:
	      obj_name = "Selsatta, the legendary jellyfish";
	      obj_keywords = "Selsatta jellyfish fish";
	      break;
	    case 9:
	      obj_name = "Stoneshell, the legendary sea turtle";
	      obj_keywords = "Stoneshell sea turtle fish";
	      break;
	    case 10:
	      obj_name = "Mungava, the legendary grouper";
	      obj_keywords = "Mungava grouper fish";
	      break;
	    case 11:
	      obj_name = "Snapjaw, the legendary mako shark";
	      obj_keywords = "Snapjaw mako shark fish";
	      break;
	    case 12:
	      obj_name = "Mola Mola, the legendary sunfish";
	      obj_keywords = "Mola sunfish fish";
	      break;
	    case 13:
	      obj_name = "Abarakis, the legendary sea bass";
	      obj_keywords = "Abarakis sea bass fish";
	      break;
	    case 14:
	      obj_name = "Seabolt, the legendary sailfish";
	      obj_keywords = "Seabolt sailfish fish";
	      break;
	    case 15:
	      obj_name = "Ancient, the legendary coelacanth";
	      obj_keywords = "Anceint coelacanth fish";
	      break;
	    case 16:
	      obj_name = "Asmodeus, the legendary devilfish";
	      obj_keywords = "Asmodeus devilfish fish";
	      break;
	    case 17:
	      obj_name = "Ralthagar, the legendary sea horse";
	      obj_keywords = "Ralthagar sea horse fish";
	      break;
	    case 18:
	      obj_name = "Dark Cloud, the legendary mardan garuyan";
	      obj_keywords = "Dark Cloud mardan garuyan fish";
	      break;
	    case 19:
	      obj_name = "Fin Fin, Rild's legendary gift to Minax";
	      obj_keywords = "Fin Rild Minax fish";
	      break;
	    default:
	      obj_name = "Salty, the legendary anchovy";
	      obj_keywords = "Salty anchovy fish";
	      break;
	    }
	}
      free_string (obj->short_descr);
      obj->short_descr = str_dup (obj_name);
      free_string (obj->description);
      obj->description = str_dup (capitalize (obj_name));	//," is here.");
      free_string (obj->name);
      obj->name = str_dup (obj_keywords);
    }
  ch->pcdata->fishing = FISHING_NOT;
  pole = get_eq_char (ch, WEAR_HOLD);
  if (pole == NULL || pole->item_type != ITEM_POLE)
    {
      bug ("Player caught a fish without holding a pole.", 0);
    }

  else
    pole->value[1] = 0;		//Pole is no longer baited
  act
    ("You successfully reel in the last bit, and you see $p on the end of your reel!",
     ch, obj, NULL, TO_CHAR);
  act
    ("$n successfully reels in the last bit, and you see $p on the end of $s reel!",
     ch, obj, NULL, TO_ROOM);
  sprintf (buf, "!!INFO!! - %s has caught %s that weighed %d.", ch->name,
	   obj->short_descr, obj->weight / 10);
  log_string (buf);
  check_top_fisher (ch, obj);
  if (ch->carry_number + get_obj_number (obj) > can_carry_n (ch)
      || get_carry_weight (ch) + get_obj_weight (obj) > can_carry_w (ch))
    {
      act ("You pull $p off the end of your pole and toss it on the deck.",
	   ch, obj, NULL, TO_CHAR);
      act ("$n pulls $p off the end of $s pole and tosses it on the deck.",
	   ch, obj, NULL, TO_ROOM);
      obj_to_room (obj, ch->in_room);
    }

  else
    {
      act
	("You quickly grab $p, and dream about showing it to all your friends soon!",
	 ch, obj, NULL, TO_CHAR);
      act ("$n quickly scoops up $p and puts it among $s things.", ch, obj,
	   NULL, TO_ROOM);
      obj_to_char (obj, ch);
    }
}

//Iblis - function called to determine if the ocean vnum is a hotspot or not
bool is_hotspot (int hs, int x)
{
  short i;
  for (i = 0; i < x; i++)
    if (hotspot[i] == hs)
      return TRUE;
  return FALSE;
}

//Iblis - function called every 30 ticks I think? to update hotspots
void make_hotspots (void)
{
  short i;
  for (i = 0; i < 250; i++)
    hotspot[i] = 0;
  for (i = 0; i < 250; i++)
    {
      while (is_hotspot ((hotspot[i] = number_range (1, 9999) + 28201), i))
	;
    }
}

//Iblis - function that returns a number between 1 and 10 to determine the relativity of the ocean
//room a player is in to a hotspot, used to determine the range of weights for fish 
short hotspot_proximity (CHAR_DATA * ch)
{
  int dir, distance, angle, closest_hotspot;
  ROOM_INDEX_DATA *was_in_room, *was_in_room2;
  if (ch == NULL || ch->in_room == NULL)
    {
      if (ch == NULL)
	bug ("NULL ch in hotspot_proximity", 0);

      else
	bug ("NULL ch->in_room in hotspot_proximity", 0);
      return 10;
    }
  closest_hotspot = 10;
  was_in_room = ch->in_room;
  for (dir = 0; dir <= 3; dir++)
    {
      if (ch->in_room->exit[dir] != NULL)
	{
	  for (distance = 1; distance <= 10; distance++)
	    {
	      EXIT_DATA *pexit;
	      if (((pexit = ch->in_room->exit[dir]) != NULL)
		  && (pexit->u1.to_room != NULL)
		  && (pexit->u1.to_room != was_in_room))
		{

		  /* If the door is closed, stop looking... */
		  if (IS_SET (pexit->exit_info, EX_CLOSED))
		    break;
		  ch->in_room = pexit->u1.to_room;
		  was_in_room2 = ch->in_room;
		  for (angle = 0; angle < 11 - distance; angle++)
		    {
		      if ((angle >= 1) &&
			  (((pexit =
			     ch->in_room->exit[(dir + 1) % 4]) !=
			    NULL) && (pexit->u1.to_room != NULL)
			   && (pexit->u1.to_room != was_in_room)))
			{

			  /* If the door is closed, stop looking... */
			  // Iblis - Yes, I believe this is right even for here
			  if (IS_SET (pexit->exit_info, EX_CLOSED))
			    break;
			  ch->in_room = pexit->u1.to_room;
			}

		      else if (angle >= 1)
			break;

		      //If the room is not ocean, continue looking (even if its past an inside room)
		      if (!ch || !ch->in_room
			  || ch->in_room->sector_type != SECT_WATER_OCEAN)
			continue;
		      if (is_hotspot (ch->in_room->vnum, 250))
			if (closest_hotspot > angle + distance)
			  closest_hotspot = angle + distance;
		    }
		  ch->in_room = was_in_room2;
		}
	    }
	}
      ch->in_room = was_in_room;
    }
  return closest_hotspot;
}

//Iblis - loads the topfisherman data from file
void load_topfishers ()
{
  FILE *fp;

  short i;
  bool nomore;
  char *tempstring;
  if ((fp = fopen (TOP_FISHER_FILE, "r")) == NULL)
    {
      for (i = 0; i < 25; i++)
	{
	  top_fishers[i].weight = 0;
	  strcpy (top_fishers[i].name, "\0");
	  strcpy (top_fishers[i].fish_name, "\0");
	}
      return;
    }
  nomore = FALSE;
  for (i = 0; i < 25; i++)
    {
      tempstring = fread_string (fp);
      if (tempstring == NULL)
	nomore = TRUE;

      if (!nomore)
	{
	  top_fishers[i].weight = fread_number (fp);
	  strcpy (top_fishers[i].fish_name, fread_string (fp));
	  strcpy (top_fishers[i].name, tempstring);
	}

      else
	{
	  strcpy (top_fishers[i].name, "\0");
	  strcpy (top_fishers[i].name, "\0");
	  strcpy (top_fishers[i].fish_name, "\0");
	}
    }
}

//Iblis - saves the topfisherman data to file
void save_topfishers (void)
{
  FILE *fp;
  short i;
  fclose (fpReserve);
  if ((fp = fopen (TOP_FISHER_FILE, "w")) == NULL)
    {
      perror (TOP_FISHER_FILE);
    }
  for (i = 0; i < 25; i++)
    {
      fprintf (fp, "%s~ %d %s~\n", top_fishers[i].name,
	       top_fishers[i].weight, top_fishers[i].fish_name);
    }
  fclose (fp);
  fpReserve = fopen (NULL_FILE, "r");
}

//Iblis - Called when the fish is caught, to determine if it is a record breaking fish or not
void check_top_fisher (CHAR_DATA * ch, OBJ_DATA * obj)
{
  short i, k;
  TOP_FISHER temp_tf[2];
  if (IS_IMMORTAL (ch))
    return;
  for (i = 0; i < 25; i++)
    {
      if (!str_cmp(top_fishers[i].name,ch->name) && top_fishers[i].weight >= obj->weight / 10.0
		      && obj->weight < 2500)
      {
	      if (obj->weight < 2500 && obj->cost > 10000)
		      obj->cost = 10000;
	      
        return;
      }
		
      if (top_fishers[i].weight < obj->weight / 10.0)
	{
	  temp_tf[i % 2].weight = top_fishers[i].weight;
	  strcpy (temp_tf[i % 2].name, top_fishers[i].name);
	  strcpy (temp_tf[i % 2].fish_name, top_fishers[i].fish_name);
	  top_fishers[i].weight = obj->weight / 10.0;
	  strcpy (top_fishers[i].name, ch->name);
	  strcpy (top_fishers[i].fish_name, obj->short_descr);
	  if (!str_cmp(temp_tf[i % 2].name,ch->name) && temp_tf[i%2].weight < 250)
	  {
	     save_topfishers ();
	     send_to_char ("Your fish is a record breaking fish!\n\r", ch);
	     break;
	  }
	  for (k = i + 1; k < 25; k++)
	    {
	      temp_tf[k % 2].weight = top_fishers[k].weight;
	      strcpy (temp_tf[k % 2].name, top_fishers[k].name);
	      strcpy (temp_tf[k % 2].fish_name, top_fishers[k].fish_name);
	      top_fishers[k].weight = temp_tf[(k - 1) % 2].weight;
	      strcpy (top_fishers[k].name, temp_tf[(k - 1) % 2].name);
	      strcpy (top_fishers[k].fish_name,
		      temp_tf[(k - 1) % 2].fish_name);
	      if (!str_cmp(temp_tf[k % 2].name,ch->name) && temp_tf[k%2].weight < 250)
		      break;
	    }
	  send_to_char ("Your fish is a record breaking fish!\n\r", ch);
	  save_topfishers ();
	  break;
	}
    }
    if (obj->weight < 2500 && obj->cost > 10000)
      obj->cost = 10000;
}

//Iblis - The actual command to display the topfisherman to a character
void do_topfishers (CHAR_DATA * ch, char *argument)
{
  short i;
  char buf[MAX_STRING_LENGTH];
  send_to_char
    ("------------------- Top Fisherman -------------------\n\r", ch);
  send_to_char
    ("##. Player Name    Fish Name                                  Fish Weight\n\r\n\r",
     ch);
  for (i = 0; i < 25; i++)
    {
      if (top_fishers[i].weight == 0)
	break;
      sprintf (buf, "`b%2d. `l%-12s   `i%-43s   `h%5d``\n\r", i + 1,
	       top_fishers[i].name, top_fishers[i].fish_name,
	       top_fishers[i].weight);
      send_to_char (buf, ch);
    }
}
