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

//Archery.c
//Iblis 7/19/03


void update_aggressor args ((CHAR_DATA * ch, CHAR_DATA * victim));
extern const char *dir_name[];
void mob_hit args ((CHAR_DATA * ch, CHAR_DATA * victim, int dt));

//function in Archery
bool damage_object args ((OBJ_DATA * obj, short amount));
char *arrowhead_name args ((char *material));

//Iblis - Function that calculates the chance of firing an arrow succeeding
int archery_chance (CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * bow,
		OBJ_DATA * arrow, int room_distance)
{
  int chance;

  chance = ((ch->move * 1.0) / ch->max_move) * 100;

  if (get_position(victim) == POS_SLEEPING)
    chance += 100;

  else if (victim->position == POS_RESTING)
    chance += 50;

  else if (victim->position == POS_SITTING)
    chance += 33;

  else if (victim->position == POS_FIGHTING || ch->fighting != NULL)
    chance += 20;

  if (ch->race == PC_RACE_ELF)
    chance += 25;

  else if (ch->race == PC_RACE_SYVIN)
    chance += 7;

  if (victim->size == SIZE_TINY)
    chance -= 20;

  else if (victim->size == SIZE_SMALL)
    chance -= 10;

  else if (victim->size == SIZE_LARGE)
    chance += 10;

  else if (victim->size == SIZE_HUGE)
    chance += 20;

  else if (victim->size == SIZE_GIANT)
    chance += 30;
  
  if (get_curr_stat (victim, STAT_DEX) > 18)
    chance -= 5 * (get_curr_stat (victim, STAT_DEX) - 18);

  if (room_distance == 2)
    chance -= 10;

  else if (room_distance == 3)
    chance -= 20;

  else if (room_distance == 4)
    chance -= 40;

  if (IS_OUTSIDE (victim) || IS_OUTSIDE (ch))
    {
      if (weather_info.sky == SKY_RAINING)
	chance -= 10;

      else if (weather_info.sky == SKY_LIGHTNING)
	chance -= 15;
    }
  if (get_curr_stat (victim, STAT_STR) > 20)
    chance += 5 * (get_curr_stat (victim, STAT_STR) - 20);

  
  if (bow->condition < 100)
    chance -= (100 - bow->condition);

  if (arrow->condition < 100)
    chance -= (100 - arrow->condition);

  if (number_percent () >= get_skill (ch, gsn_archery))
    chance = -1;
  if (chance < 0)
    send_to_char ("You might as well have fired blind.\r\n", ch);

  else if (chance < 21)
    send_to_char
      ("You don't have any faith in the quality of the shot.\r\n", ch);

  else if (chance < 41)
    send_to_char ("You have a bad feeling about the shot.\r\n", ch);

  else if (chance < 61)
    send_to_char
      ("Not the worst shot, but it could have been better.\r\n", ch);

  else if (chance < 81)
    send_to_char ("You have a good feeling about the shot.\r\n", ch);

  else if (chance < 101)
    send_to_char ("The shot was right on target.\r\n", ch);

  else
    send_to_char
      ("The gods themselves could not have fired more accurately.\r\n", ch);
  return chance;
}

//Iblis - Fire command (for use in archery skill)
void do_fire (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim = NULL, *vch, *vfightnow, *cfightnow;
  char vict_str[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj, *weapon;
  short x, door = 0, room_distance = 0;
  ROOM_INDEX_DATA *current_room;
  if (get_skill (ch, gsn_archery) <= 0)
    {
      send_to_char
	("You might have heard of firing once, but you are clueless as to how.\n\r",
	 ch);
      return;
    }
  argument = one_argument (argument, vict_str);
  if (argument[0] == '\0')
    {
      for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	if (obj->item_type == ITEM_WEAPON
	    && obj->value[0] == WEAPON_ARROW && can_see_obj (ch, obj))
	  break;
    }
  else
    {
      for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	if (is_name (argument, obj->name) && can_see_obj (ch, obj))
	  break;
    }
  if (obj == NULL)
    {
      send_to_char ("You don't seem to be carrying that arrow.\n\r", ch);
      return;
    }
  if (is_affected (ch, gsn_fear))
  {
	  send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
	  return;
  }
  if (IS_AFFECTED (ch, AFF_CALM) || is_affected (ch, skill_lookup
						 ("Quiet Mountain Lake")))
    {
      send_to_char ("You are feeling too peaceful to do that now.\n\r", ch);
      return;
    }
  if (obj->item_type != ITEM_WEAPON || obj->value[0] != WEAPON_ARROW)
    {
      act ("$p is not a arrow, and you can only fire arrows.", ch, obj,
	   NULL, TO_CHAR);
      return;
    }
  weapon = get_eq_char (ch, WEAR_WIELD_R);
  if (weapon == NULL
      || (weapon->value[0] != WEAPON_SHORTBOW
	  && weapon->value[0] != WEAPON_LONGBOW))
    weapon = get_eq_char (ch, WEAR_WIELD_L);
  if (weapon == NULL
      || (weapon->value[0] != WEAPON_SHORTBOW
	  && weapon->value[0] != WEAPON_LONGBOW))
    {
      send_to_char ("You need to be wielding a bow to fire arrows.\n\r", ch);
      return;
    }
  for (x = 0; x < MAX_DIR; x++)
    {
      if (ch->in_room->exit[x] != NULL
	  && ch->in_room->exit[x]->u1.to_room != NULL
	  && !IS_SET (ch->in_room->exit[x]->exit_info, EX_CLOSED))
	{
	  for (vch = ch->in_room->exit[x]->u1.to_room->people; vch;
	       vch = vch->next_in_room)
	    if (is_name (vict_str, vch->name) && can_see_hack (ch, vch))
	      {
		victim = vch;
		room_distance = 1;
		door = x;
		break;
	      }
	  if (victim != NULL)
	    break;
	  if (ch->in_room->exit[x]->u1.to_room->exit[x] != NULL
	      && ch->in_room->exit[x]->u1.to_room->exit[x]->u1.
	      to_room !=
	      NULL && !IS_SET (ch->in_room->exit[x]->u1.to_room->
			       exit[x]->exit_info, EX_CLOSED))
	    {
	      for (vch =
		   ch->in_room->exit[x]->u1.to_room->exit[x]->
		   u1.to_room->people; vch; vch = vch->next_in_room)
		if (is_name (vict_str, vch->name) && can_see_hack (ch, vch))
		  {
		    victim = vch;
		    room_distance = 2;
		    door = x;
		    break;
		  }
	      if (victim != NULL)
		break;
	      if (weapon->value[0] == WEAPON_SHORTBOW)
		continue;
	      current_room =
		ch->in_room->exit[x]->u1.to_room->exit[x]->u1.to_room;

	      //Room distance = 3
	      if (current_room->exit[x] != NULL
		  && current_room->exit[x]->u1.to_room != NULL
		  && !IS_SET (current_room->exit[x]->exit_info, EX_CLOSED))
		{
		  for (vch = current_room->exit[x]->u1.to_room->people;
		       vch; vch = vch->next_in_room)
		    if (is_name (vict_str, vch->name)
			&& can_see_hack (ch, vch))
		      {
			victim = vch;
			room_distance = 3;
			door = x;
			break;
		      }
		  if (victim != NULL)
		    break;

		  //Room distance = 4
		  if (current_room->exit[x]->u1.to_room->exit[x] !=
		      NULL && current_room->exit[x]->u1.to_room->
		      exit[x]->u1.to_room !=
		      NULL && !IS_SET (current_room->exit[x]->u1.
				       to_room->exit[x]->exit_info,
				       EX_CLOSED))
		    {
		      for (vch =
			   current_room->exit[x]->u1.to_room->exit[x]->
			   u1.to_room->people; vch; vch = vch->next_in_room)
			if (is_name (vict_str, vch->name)
			    && can_see_hack (ch, vch))
			  {
			    victim = vch;
			    room_distance = 4;
			    door = x;
			    break;
			  }
		      if (victim != NULL)
			break;
		    }
		}
	    }
	}
    }
  if (victim == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (victim == ch)
    {
      send_to_char ("You can't fire at yourself.\n\r", ch);
      return;
    }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (victim->in_room == ch->in_room)
    {
      send_to_char ("You can't fire at someone in the same room.\n\r", ch);
      return;
    }
  if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
    {
      send_to_char ("You cannot fire in a safe room.\n\r", ch);
      return;
    }
  if (ch->in_room->sector_type == SECT_UNDERWATER && ch->race != PC_RACE_CANTHI)
    {				
      send_to_char
	("Luckily, you realize before you actually fire than firing underwater is useless.\n\r",
	 ch);
      return;
    }
  if (ch->mana < 20)
    {
      send_to_char
	("You do not the required mana to be able to concentrate enough to fire at anything.\n\r",
	 ch);
      return;
    }
  ch->mana -= 20;
  sprintf (buf, "You fire $p $T using %s.", weapon->short_descr);
  act (buf, ch, obj, dir_name[door], TO_CHAR);
  sprintf (buf, "$n fires $p $T using %s.", weapon->short_descr);
  act (buf, ch, obj, dir_name[door], TO_ROOM);
  if (!IS_IMMORTAL (ch))
    {
      if (weapon->value[0] == WEAPON_LONGBOW)
	WAIT_STATE (ch, 4 * skill_table[gsn_archery].beats);

      else
	WAIT_STATE (ch, skill_table[gsn_archery].beats);
    }
  for (x = 0; x < MAX_DIR; x++)
    {
      if (ch->in_room->exit[x] != NULL
	  && ch->in_room->exit[x]->u1.to_room != NULL
	  && !IS_SET (ch->in_room->exit[x]->exit_info, EX_CLOSED))
	{
	  if (ch->in_room->exit[x]->u1.to_room->people)
	    {
	      act ("You hear the snapping sound of a bowstring nearby.",
		   ch->in_room->exit[x]->u1.to_room->people, NULL, NULL,
		   TO_ROOM);
	      act ("You hear the snapping sound of a bowstring nearby.",
		   ch->in_room->exit[x]->u1.to_room->people, NULL, NULL,
		   TO_CHAR);
	    }
	}
    }
  if (is_affected (victim, gsn_mirror))
    {
      obj_to_room (obj, victim->in_room);
      act
	("$p bounces off of your invisible mirror and clatters to the ground.",
	 victim, obj, NULL, TO_CHAR);
      act
	("$p bounces off of an invisible mirror and clatters to the ground.",
	 victim, obj, victim, TO_NOTVICT);
      return;
    }
  if (victim->in_room->sector_type == SECT_UNDERWATER)
    {
      act ("$p floats into the murky depths of the water.", ch, obj,
	   NULL, TO_CHAR);
      obj_to_room (obj, victim->in_room);
      act ("$p floats gently into the water.", victim, obj, NULL, TO_ROOM);
      return;
    }
  if (number_percent () - 5 < archery_chance (ch, victim, weapon, obj, room_distance)
		  && !IS_SET(victim->imm_flags,IMM_ARROWS))
    {
      short dam = dice (obj->value[1], obj->value[2]);
      short level2;
      /*if (dam < obj->value[1] * obj->value[2])
	dam = ((dam + (obj->value[1] * obj->value[2])) / 2.0) + 1;*/

//      sprintf(buf,"intial arrow diceroll = %d\n\r",dam);
      //     send_to_char(buf,ch);
      dam *= 8;
      if (weapon->value[0] == WEAPON_LONGBOW)
	dam *= 4;

      else
	dam *= 2;

      //    sprintf(buf,"damage after long/short factored in = %d\n\r",dam);
      //        send_to_char(buf,ch);
      //if (weapon->value[0] == WEAPON_SHORTBOW)
      /*else
      {
        if (ch->race == PC_RACE_ELF)
  	  dam *= 8;

        else if (get_curr_stat (ch, STAT_STR) > 18)
  	  dam *= (get_curr_stat (ch, STAT_STR)) - 17;
      }*/

      // sprintf(buf,"damage after str factored in = %d\n\r",dam);
      //            send_to_char(buf,ch);
      dam += dice (weapon->value[1], weapon->value[2]);

//        sprintf(buf,"damage after bow dice added = %d\n\r",dam);
//                   send_to_char(buf,ch);
      level2 = Class_level (ch, 4);
      if (level2 == 0)
	level2 = 1;
      if (IS_IMMORTAL (ch))
	level2 = ch->level;
      if (level2 < 31)
	{
	  dam *= .334;
	}
      else if (level2 < 61)
	{
	  dam *= .667;
	}
      vfightnow = victim->fighting;
      cfightnow = ch->fighting;

      //Iblis 5/13/03 - To get around the darn damage reducer in fight.c
      dam = adjust_damage(dam);
      damage (ch, victim, dam, gsn_archery, obj->value[3], FALSE);
      obj_from_char (obj);
      act
	("You wince in pain as $p flies into the room and sticks into your side.",
	 victim, obj, NULL, TO_CHAR);
      act
	("$n winces in pain as $p flies into the room and sticks into $s's side.",
	 victim, obj, NULL, TO_ROOM);

      //5-11-03 Iblis - We need the old value of dam, so we don't create a new dam now
      if (dam > 100)
	dam = 100;		//to set dam to a more reasonable value
      if (ch->fighting == victim && IS_WEAPON_STAT (obj, WEAPON_POISON))
	{
	  int level;
	  AFFECT_DATA *poison, af;
	  if ((poison = affect_find (obj->affected, gsn_poison)) == NULL)
	    level = obj->level;

	  else
	    level = poison->level;
	  if (!saves_spell (level / 2, victim, DAM_POISON))
	    {
	      send_to_char
		("You feel poison coursing through your veins.", victim);
	      act ("$n is poisoned by the venom on $p.", victim, obj,
		   NULL, TO_ROOM);
	      af.where = TO_AFFECTS;
	      af.type = gsn_poison;
	      af.level = level * 3 / 4;
	      af.duration = level / 2;
	      af.location = APPLY_STR;
	      af.modifier = -1;
	      af.bitvector = AFF_POISON;
	      af.permaff = FALSE;
	      af.composition = FALSE;
	      af.comp_name = str_dup ("");
	      affect_join (victim, &af);
	    }

	}
      if (ch->fighting == victim && IS_WEAPON_STAT (obj, WEAPON_FLAMING))
	{

	  //5-11-03 Iblis - Change the damage due to depend on the damage of the weapon          //attack instead of the weapon's level
	  dam = number_range (1, dam / 4 + 1);
	  act ("$n is burned by $p.", victim, obj, NULL, TO_ROOM);
	  act ("$p sears your flesh.", victim, obj, NULL, TO_CHAR);
	  fire_effect ((void *) victim, obj->level / 2, dam, TARGET_CHAR);
	  damage (ch, victim, dam, 0, DAM_FIRE, FALSE);
	}
      if (ch->fighting == victim && IS_WEAPON_STAT (obj, WEAPON_FROST))
	{

	  //5-11-03 Iblis - Change the damage due to depend on the damage of the weapon
	  //attack instead of the weapon's level
	  dam = number_range (1, dam / 6 + 2);
	  act ("$p freezes $n.", victim, obj, NULL, TO_ROOM);
	  act ("The cold touch of $p surrounds you with ice.", victim,
	       obj, NULL, TO_CHAR);
	  cold_effect (victim, obj->level / 2, dam, TARGET_CHAR);
	  damage (ch, victim, dam, 0, DAM_COLD, FALSE);
	}
      if (ch->fighting == victim && IS_WEAPON_STAT (obj, WEAPON_SHOCKING))
	{

	  //5-11-03 Iblis - Change the damage due to depend on the damage of the weapon          //attack instead of the weapon's level
	  dam = number_range (1, dam / 5 + 2);
	  act ("$n is struck by lightning from $p.", victim, obj, NULL,
	       TO_ROOM);
	  act ("You are shocked by $p.", victim, obj, NULL, TO_CHAR);
	  shock_effect (victim, obj->level / 2, dam, TARGET_CHAR);
	  damage (ch, victim, dam, 0, DAM_LIGHTNING, FALSE);
	}
      extract_obj (obj);
      if (IS_NPC(victim))
      {
        short chance;
        if (victim->hit > victim->max_hit/2)
          chance = 5;
        else if (victim->hit < victim->max_hit/4)
          chance = 85;
        else if (victim->hit < victim->max_hit/8)
          chance = 102;
        else chance = 45;
        if (number_percent() <= chance && victim->position >= POS_FIGHTING)
	{
	   ROOM_INDEX_DATA *old_room;
           old_room = victim->in_room;
	   char_from_room(victim);
	   char_to_room(victim,ch->in_room);
	  act("$n finally notices where $N has been firing from, and runs in for a quick attack on $M.",victim,NULL,ch,TO_ROOM);
          mob_hit (victim, ch, TYPE_UNDEFINED);
	  char_from_room(victim);
	  char_to_room(victim,old_room);
	}
      }
      victim->fighting = vfightnow;
      ch->fighting = cfightnow;
      if (ch->fighting == NULL)
	{
	  if (ch->mount != NULL && ch->mount->riders != NULL)
	    ch->position = POS_MOUNTED;

	  else
	    ch->position = POS_STANDING;
	}
      if (victim->fighting == NULL)
	{
	  if (victim->mount != NULL && victim->mount->riders != NULL)
	    victim->position = POS_MOUNTED;

	  else
	    victim->position = POS_STANDING;
	}
      check_improve (ch, gsn_archery, TRUE, 1);
    }
  else
    {
      unequip_char (ch, obj);
      obj_from_char (obj);
      if (victim->in_room->exit[door] != NULL
	  && victim->in_room->exit[door]->u1.to_room != NULL
	  && !IS_SET (victim->in_room->exit[door]->exit_info, EX_CLOSED))
	{
	  obj_to_room (obj, victim->in_room->exit[door]->u1.to_room);
	  act ("$p zips past $n into the next room.", victim, obj, NULL,
	       TO_ROOM);
	  act ("$p zips past you into the next room.", victim, obj, NULL,
	       TO_CHAR);
	  if (victim->in_room->exit[door]->u1.to_room->people)
	    {
	      act ("$p zips past and sticks in the ground.",
		   victim->in_room->exit[door]->u1.to_room->people, obj,
		   NULL, TO_CHAR);
	      act ("$p zips past and sticks in the ground.",
		   victim->in_room->exit[door]->u1.to_room->people, obj,
		   NULL, TO_ROOM);
	    }
	}
      else
	{
	  obj_to_room (obj, victim->in_room);
	  act ("$p zips past $n and sticks in the ground.", victim, obj,
	       NULL, TO_NOTVICT);
	  act ("$p zips past you and sticks in the ground.", victim,
	       obj, NULL, TO_CHAR);
	}
      obj->condition = 50;
      check_improve (ch, gsn_archery, FALSE, 1);
      update_aggressor (ch, victim);
    }
}

//Iblis - Added with dreams of future functionality
////Return TRUE if the obj should be destroyed
bool damage_object (OBJ_DATA * obj, short amount)
{
  obj->condition -= amount;
  if (obj->condition <= 0)
    {
       return TRUE;
    }
  else
    return FALSE;
}
                        
