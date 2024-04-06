/***************************************************************************
 * *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,         *
 * *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.    *
 * *                                                                          *
 * *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael           *
 * *  Chastain, Michael Quan, and Mitchell Tse.                               *
 * *                                                                          *
 * *  In order to use any part of this Merc Diku Mud, you must comply with    *
 * *  both the original Diku license in 'license.doc' as well the Merc        *
 * *  license in 'license.txt'.  In particular, you may not remove either of  *
 * *  these copyright notices.                                                *
 * *                                                                          *
 * *  Much time and thought has gone into this software and you are           *
 * *  benefitting.  We hope that you share your changes too.  What goes       *
 * *  around, comes around.                                                   *
 * ****************************************************************************/


/***************************************************************************
 * *       ROM 2.4 is copyright 1993-1998 Russ Taylor                         *
 * *       ROM has been brought to you by the ROM consortium                  *
 * *           Russ Taylor (rtaylor@hypercube.org)                            *
 * *           Gabrielle Taylor (gtaylor@hypercube.org)                       *
 * *           Brian Moore (zump@rom.org)                                     *
 * *       By using this code, you have agreed to follow the terms of the     *
 * *       ROM license, in the file Rom24/doc/rom.license                     *
 * ****************************************************************************/

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include "merc.h"
#include "interp.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"

//Iblis - Created beginning of 2004 to hold all of the Necromancer Class skills

OBJ_DATA *get_obj_vnum_room args((ROOM_INDEX_DATA* rid, int vnum));
OBJ_DATA *get_obj_vnum_char args((CHAR_DATA* ch, int vnum));
void affect_strip_skills args((CHAR_DATA * ch));
long int level_cost args ((int level));
extern int arearooms args ((CHAR_DATA * ch));
extern int areacount args ((CHAR_DATA * ch));
extern int roomcount args ((CHAR_DATA * ch));

	

void do_leeching (CHAR_DATA* ch, char* argument)
{
  CHAR_DATA *victim;//, *victim_next;
  int chance;

  if ((chance = get_skill(ch,gsn_leeching)) < 1)
    {
      send_to_char("Leeches? The people who take all your money but give nothing?  None here.\n\r",ch);
      return;
    }

  if ((victim = get_char_room (ch, argument)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  

  if (!is_affected (victim, gsn_poison))
    {
      if (victim == ch)
        send_to_char
          ("You do not appear to be poisoned at the moment.\n\r", ch);

      else
        act ("$N doesn't appear to be poisoned.", ch, NULL, victim, TO_CHAR);
      return;
    }
  WAIT_STATE (ch, skill_table[gsn_leeching].beats);
  if (check_dispel (ch->level, victim, gsn_poison) || chance < number_percent())
    {
      act ("You attach a few leeches to $N's neck to suck the poison out.",ch,NULL,victim,TO_CHAR);
      act ("$n attaches a few leeches to your neck that sucks the poison out..\n\r", ch,NULL,victim,TO_VICT);
      act ("$n attaches a few leeches to $N's neck and removes them soon afterwards.",
           ch, NULL, victim, TO_NOTVICT);
    }

  else
  {
    send_to_char ("Alas, the leeches failed.\n\r", ch);
    act ("$n attaches a few leeches on $N's neck and removes them soon afterwards.", ch, NULL, victim, TO_NOTVICT);
    act ("$n attaches a few leeches on your neck, but they fail to suck all the poison out.\n\r", ch,NULL,victim,TO_VICT);
  }
    
  
}


// obj->value[5] will store the amount of uses possible for each -gram
// obj->value[6] will store the total number of reinforcements
void do_draw (CHAR_DATA* ch, char* argument)
{
  int chance;
  OBJ_DATA* obj;
  char arg1[MAX_STRING_LENGTH];
  OBJ_DATA* stone;
  argument = one_argument(argument,arg1);
  
  if ((chance = get_skill(ch,gsn_draw)) < 1)
    {
      send_to_char("The best you can draw is stick figures.\n\r",ch);
      return;
    }
  if (arg1[0] == '\0')
    {
      send_to_char("Syntax : Draw pentagram \n\r",ch);
      if (ch->level >= 23)
	send_to_char ("         Draw hexagram \n\r",ch);
      if (ch->level >= 34)
        send_to_char ("         Draw death ward \n\r",ch);    
      if (ch->level >= 50)
	send_to_char ("         Draw septagram \n\r",ch);
      return;
    }
  stone = get_eq_char (ch, WEAR_HOLD);
  if (stone == NULL || (stone->pIndexData->vnum != OBJ_VNUM_RUNE_CONV))
    stone = NULL;

  if ((ch->in_room->sector_type == SECT_AIR 
      || ch->in_room->sector_type == SECT_WATER_OCEAN
      || ch->in_room->sector_type == SECT_SWAMP
      || ch->in_room->sector_type == SECT_UNDERWATER
      || ch->in_room->sector_type == SECT_WATER_SWIM
      || ch->in_room->sector_type == SECT_WATER_NOSWIM) 
      && !str_cmp(argument,"ward")	) 
    {
      send_to_char("You cannot draw here.\n\r",ch);
      return;
    }
  if (!str_prefix(arg1,"pentagram"))
    {
      if (!(obj = get_obj_vnum_room(ch->in_room,OBJ_VNUM_PENTAGRAM)))
	if (get_obj_vnum_room(ch->in_room,OBJ_VNUM_HEXAGRAM)
	    || get_obj_vnum_room(ch->in_room,OBJ_VNUM_SEPTAGRAM))
	  {
	    send_to_char("There is already a drawing in this room.\n\r",ch);
	      return;
	  }
      if (obj && str_cmp(obj->plr_owner,ch->name))
	{
	  send_to_char("You may not reinforce another necromancer's drawing.\n\r",ch);
	  return;
	}
      if (ch->hit <= (stone?5:10))
	{
	  send_to_char("You cannot muster the strength to draw.\n\r",ch);
	  return;
	}
      ch->hit -= (stone?5:10);
      WAIT_STATE (ch, skill_table[gsn_draw].beats);
      act("$n bites $s index finger and blood seeps forth.",ch,NULL,NULL,TO_ROOM);
      send_to_char("You bite your index finger and blood seeps forth.\n\r",ch);
      if (!obj)
        {
	  act("Whispers of the underworld resound as $n draws a pentagram with $s own blood.",ch,NULL,NULL,TO_ROOM);
	  send_to_char("Whispers of the underworld resound as you draw a pentagram with your own blood.\n\r",ch);
	  obj = create_object(get_obj_index(OBJ_VNUM_PENTAGRAM), ch->level);
	  obj_to_room(obj,ch->in_room);
	  obj->timer = 2;
	  obj->value[5] = 1;
	  obj->plr_owner = str_dup(ch->name);
	}
      else 
	{
	  act("$n reinforces the pentagram with unholy runes.",ch,NULL,NULL,TO_ROOM);
	  send_to_char("You reinforce the pentagram with unholy runes.\n\r",ch);
	  obj->timer += 2;
	  obj->value[5]++;
	}
    }
  else if (!str_prefix(arg1,"hexagram") && ch->level >= 23)
    {
      if (!(obj = get_obj_vnum_room(ch->in_room,OBJ_VNUM_HEXAGRAM)))
        if (get_obj_vnum_room(ch->in_room,OBJ_VNUM_PENTAGRAM)
            || get_obj_vnum_room(ch->in_room,OBJ_VNUM_SEPTAGRAM))
          {
            send_to_char("There is already a drawing in this room.\n\r",ch);
	    return;
          }
      if (obj && str_cmp(obj->plr_owner,ch->name))
        {
          send_to_char("You may not reinforce another necromancer's drawing.\n\r",ch);
          return;
        }
      if (ch->hit <= (stone?10:20))
        {
          send_to_char("You cannot muster the strength to draw.\n\r",ch);
          return;
        }
      ch->hit -= (stone?10:20);
      WAIT_STATE (ch, skill_table[gsn_draw].beats);
      act("$n bites $s index finger and blood seeps forth.",ch,NULL,NULL,TO_ROOM);
      send_to_char("You bite your index finger and blood seeps forth.\n\r",ch);
      if (!obj)
        {
	  act("Whispers of the underworld resound as $n draws a hexagram with $s own blood.",ch,NULL,NULL,TO_ROOM);
	  send_to_char("Whispers of the underworld resound as you draw a hexagram with your own blood.\n\r",ch);
	  obj = create_object(get_obj_index(OBJ_VNUM_HEXAGRAM), ch->level);
	  obj_to_room(obj,ch->in_room);
	  obj->timer = 5;
	  obj->value[5] = 1;
	  obj->value[6] = 0;
	  obj->plr_owner = str_dup(ch->name);
	}
      else if (obj->value[6] < 2)
	{
	  act("$n reinforces the hexagram with unholy runes.",ch,NULL,NULL,TO_ROOM);
          send_to_char("You reinforce the hexagram with unholy runes.\n\r",ch);
	  obj->value[5]++;
	  obj->value[6]++;
	  obj->timer += 5;
	}
      else
      {
        send_to_char("You cannot reinforce the hexagram any more.\n\r",ch);
	return;
      }
      
    }
  else if (!str_prefix(arg1,"septagram") && ch->level >= 50)
    {
      if (!(obj = get_obj_vnum_room(ch->in_room,OBJ_VNUM_SEPTAGRAM)))
        if (get_obj_vnum_room(ch->in_room,OBJ_VNUM_HEXAGRAM)
            || get_obj_vnum_room(ch->in_room,OBJ_VNUM_PENTAGRAM))
          {
            send_to_char("There is already a drawing in this room.\n\r",ch);
	    return;
          }
      if (obj && str_cmp(obj->plr_owner,ch->name))
        {
          send_to_char("You may not reinforce another necromancer's drawing.\n\r",ch);
          return;
        }
      if (ch->hit <= (stone?15:30))
        {
          send_to_char("You cannot muster the strength to draw.\n\r",ch);
          return;
        }
      ch->hit -= (stone?15:30);
      WAIT_STATE (ch, skill_table[gsn_draw].beats);
      act("$n bites $s index finger and blood seeps forth.",ch,NULL,NULL,TO_ROOM);
      send_to_char("You bite your index finger and blood seeps forth.\n\r",ch);
      if (!obj)
        {
	  act("Whispers of the underworld resound as $n draws a septagram with $s own blood.",ch,NULL,NULL,TO_ROOM);
	  send_to_char("Whispers of the underworld resound as you draw a septagram with your own blood.\n\r",ch);
	  obj = create_object(get_obj_index(OBJ_VNUM_SEPTAGRAM), ch->level);
	  obj_to_room(obj,ch->in_room);
	  obj->timer = 7;
	  obj->value[5] = 1;
	  obj->value[6] = 0;
	  obj->plr_owner = str_dup(ch->name);
	}
      else if (obj->value[6] < 1)
	{
	  act("$n reinforces the septagram with unholy runes.",ch,NULL,NULL,TO_ROOM);
          send_to_char("You reinforce the septagram with unholy runes.\n\r",ch);
	  obj->value[5]++;
	  obj->value[6]++;
	  obj->timer += 7;
	}
      else
        {
          send_to_char("You cannot reinforce the septagram any more.\n\r",ch);
          return;
        }      
    }
  
  else if (!str_cmp(arg1,"death") && !str_cmp(argument,"ward") && get_skill(ch,gsn_death_ward) > 1)
    {
      int chance;
      
      if ((chance = get_skill(ch,gsn_death_ward)) < 1)
	{
	  send_to_char("You couldn't ward off death if he was as frail as your grandma.\nr",ch);
	  return;
	}
      if (ch->hit <= 15)
	{
	  send_to_char("You cannot muster the strength.\n\r",ch);
	  return;
	}
      else ch->hit -= 15;
      WAIT_STATE (ch, skill_table[gsn_death_ward].beats);
      if (chance > number_percent())
	{
	  AFFECT_DATA af;
	  
	  act("$n bites his index finger and blood seeps forth.",ch,NULL,NULL,TO_ROOM);
	  send_to_char("You bite your index finger and blood seeps forth.\n\r",ch);
	  act("$n draws a hideous mask of unholy runes on $s face.",ch,NULL,NULL,TO_ROOM);
	  send_to_char("You draw a hideous mask of unholy runes on your face.\n\r",ch);
	  
	  af.where = TO_AFFECTS;
	  af.type = gsn_death_ward;
	  af.level = ch->level;
	  af.permaff = FALSE;
	  af.composition = FALSE;
	  af.comp_name = str_dup ("");
	  af.duration = 7+((90-ch->level)/7.0);
	  af.bitvector = 0;
	  af.modifier = 0;
	  af.location = APPLY_NONE;
	  affect_to_char (ch, &af);
	}
      else
	{
	  send_to_char ("You bite your finger and smear some blood on your face, but in the end you just look silly.\n\r",ch);
	  act("$n bites $s finger and smears some blood on $s face, but just ends up looking silly." ,ch,NULL,NULL,TO_ROOM);
	}
    }
  //generate the message
  else do_draw(ch,"");
  
  
}


void do_embalm (CHAR_DATA* ch, char* argument)
{
  int chance;
  OBJ_DATA* obj=NULL;

  if ((chance = get_skill(ch,gsn_embalm)) < 1)
    {
      send_to_char("You confuse the terms embalm and napalm.\n\r",ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char("Syntax : Embalm bodypart/corpse \n\r",ch);
      return;
    }
  if ((obj = get_obj_here (ch, argument)) == NULL)
    {
      send_to_char ("You do not see that here.\n\r", ch);
      return;
    }
  switch (obj->pIndexData->vnum)
    {
    case OBJ_VNUM_SEVERED_HEAD:
    case OBJ_VNUM_CORPSE_NPC:
    case OBJ_VNUM_TORN_HEART:
    case OBJ_VNUM_SLICED_ARM:
    case OBJ_VNUM_SLICED_LEG:
    case OBJ_VNUM_GUTS:
    case OBJ_VNUM_BRAINS:break;
    default : send_to_char("You cannot embalm that!\n\r",ch);return;
    }
  WAIT_STATE (ch, skill_table[gsn_embalm].beats);
  if (obj->timer == 0)
  {
	  send_to_char("That is already embalmed!\n\r",ch);
	  return;
  }
  if (chance > number_percent())
  {
    obj->timer = 0;
    obj->value[5] = -1;
    act("$n embalms $p.",ch,obj,NULL,TO_ROOM);
    act("You embalm $p.",ch,obj,NULL,TO_CHAR);
  }
  else 
  {
    act("$n fails to embalm $p.",ch,obj,NULL,TO_ROOM);
    act("You fail to embalm $p.",ch,obj,NULL,TO_CHAR);
  }
}


void do_corrupt (CHAR_DATA* ch, char* argument)
{
  CHAR_DATA *victim, *master;
  int chance;

  if (ch->position < POS_STANDING && !IS_NPC(ch))
  {
    send_to_char("No way, you are still fighting!\n\r",ch);
    return;
  }
		    
  
  if ((chance = get_skill(ch,gsn_corrupt)) < 1)
    {
      send_to_char("You couldn't even corrupt Morian.\n\r",ch);
      return;
    }

  if (argument[0] == '\0')
    {
      send_to_char("Syntax : Corrupt CharmieName \n\r",ch);
      return;
    }


  if ((victim = get_char_room (ch, argument)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (ch->mana < skill_table[gsn_corrupt].min_mana)
  {
    send_to_char ("You don't have enough mana.\n\r",ch);
    return;
  }
  else ch->mana -= skill_table[gsn_corrupt].min_mana;
  if (!IS_AFFECTED (victim, AFF_CHARM))
    {
      send_to_char ("You just attempted to corrupt a noncharmed being.\n\r",ch);
      act("$n whispers something in $N's ear.",ch,NULL,victim,TO_ROOM);
      damage (ch, victim, 0, gsn_corrupt, DAM_NONE,FALSE);
    }


  if (chance-5 > number_percent() && victim->master != NULL)
    {
      act("$n whispers something in $N's ear.",ch,NULL,victim,TO_NOTVICT);
      act("$n whispers something in your ear.",ch,NULL,victim,TO_VICT);
      act("You whisper something in $N's ear.",ch,NULL,victim,TO_CHAR);
      if (victim->master->sex == SEX_MALE)
	act("$n turns on $s master!",victim,NULL,NULL,TO_ROOM);
      else act("$n turns on $s mistress!",victim,NULL,NULL,TO_ROOM);
      master = victim->master;
      REMOVE_BIT (victim->affected_by, AFF_CHARM);
      affect_strip (victim, gsn_charm_person);
      affect_strip (victim, skill_lookup ("The Pipers Melody"));
      victim->master = NULL;
      damage (victim, master, 0, gsn_corrupt, DAM_NONE,FALSE);
      victim->fighting = master;
    }
  else
    {
      act("$n whispers something in $N's ear.",ch,NULL,victim,TO_ROOM);
      act("You whisper something in $N's ear, but you fail to corrupt $m.",ch,NULL,victim,TO_CHAR);
      damage (victim, ch, 0, gsn_corrupt, DAM_NONE,FALSE);
    }
}


void feign_death (CHAR_DATA* ch, char* argument)
{
  int chance;

  if (IS_NPC(ch))
    return;
  if ((chance = get_skill(ch,gsn_feign_death)) < 1)
    {
      send_to_char("You couldn't feign death if your life depended on it.\n\r",ch);
      return;
    }
  if (ch->position < POS_FIGHTING && ch->fighting == NULL)
  {
	  send_to_char("You need to stand up first.\n\r",ch);
	  return;
  }
  if (ch->hit <= 20)
  {
    send_to_char ("You're not strong enough to feign your death.\n\r",ch);
    return;
  }
  ch->hit -= 20;
  
  if (chance > number_percent())
  {
    send_to_char("You feign your death.\n\r",ch);
    act("``$n is DEAD!!",ch,NULL,NULL,TO_ROOM);
    stop_fighting(ch,TRUE);
    ch->position = POS_FEIGNING_DEATH;
  }
  else send_to_char("You fail to feign your death.\n\r",ch);
}

void do_feast (CHAR_DATA* ch, char* argument)
{
  int chance;
  OBJ_DATA* obj=NULL,*prev,*prev_next;
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
    return;
  if (argument[0] == '\0')
    {
      send_to_char ("Feast on what?\n\r", ch);
      return;
    }

  if ((chance = get_skill(ch,gsn_feast)) < 1)
    {
      send_to_char("You feast upon the air.\n\r",ch);
      return;
    }
  if ((obj = get_obj_here (ch, argument)) == NULL)
    {
      act ("You see no $T here.", ch, NULL, argument, TO_CHAR);
      return;
    }
  if (obj->item_type != ITEM_CORPSE_NPC)
   {
      send_to_char("There is no corpse here to feed upon.\n\r",ch);
      return;
    }

  if (ch->mana < skill_table[gsn_feast].min_mana)
    {
      send_to_char ("You don't have enough mana.\n\r",ch);
      return;
    }
  else ch->mana -= skill_table[gsn_feast].min_mana;
   
  if (chance > number_percent())
  {
    act("$n devours $p.",ch,obj,NULL,TO_ROOM);
    act("You devour $p.",ch,obj,NULL,TO_CHAR);
    ch->pcdata->condition[COND_HUNGER] = 48;
    ch->pcdata->condition[COND_THIRST] = 48;
    ch->pcdata->condition[COND_FULL] = 48;
  }
  else
  {
    act ("$n attempts to devour $p, but ends up just making a mess of $mself.",ch,obj,NULL,TO_ROOM);
    act("You attempt to devour $p, but end up just making a mess of yourself.",ch,obj,NULL,TO_CHAR);
  }
  WAIT_STATE (ch, skill_table[gsn_feast].beats);
  if (obj->carried_by)
    {
      for (prev = obj->contains; prev != NULL; prev = prev_next)
	{
	  prev_next = prev->next_content;
	  obj_from_obj(prev);
	  obj_to_room(prev,obj->carried_by->in_room);
	  sprintf (buf, "$p falls from %s.", obj->short_descr);
          act (buf, ch, prev, NULL, TO_ROOM);
          act (buf, ch, prev, NULL, TO_CHAR);
	      if (obj->pIndexData->vnum <= 5 && obj->pIndexData->vnum >= 1)
		{
		  int silver = 0;
		  int gold = 0;
		  OBJ_DATA *tobj, *tobj_next;
		  for (tobj = ch->in_room->contents; tobj != NULL; tobj = tobj_next)
		    {
		      tobj_next = tobj->next_content;
		      switch (tobj->pIndexData->vnum)
			{
			case OBJ_VNUM_SILVER_ONE:
			  silver += 1;
			  extract_obj (tobj);
			  break;
			case OBJ_VNUM_GOLD_ONE:
			  gold += 1;
			  extract_obj (tobj);
			  break;
			case OBJ_VNUM_SILVER_SOME:
			  silver += tobj->value[0];
			  extract_obj (tobj);
			  break;
			case OBJ_VNUM_GOLD_SOME:
			  gold += tobj->value[1];
			  extract_obj (tobj);
			  break;
			case OBJ_VNUM_COINS:
			  silver += tobj->value[0];
			  gold += tobj->value[1];
			  extract_obj (tobj);
			  break;
			}
		    }
		  obj_to_room (create_money (gold, silver), ch->in_room);

		}
	}
      obj_from_char(obj);
    }
  else 
    {
      for (prev = obj->contains; prev != NULL; prev = prev_next)
	{
	  prev_next = prev->next_content;
	  obj_from_obj(prev);
	  obj_to_room(prev,obj->in_room);
	  sprintf (buf, "$p falls from %s.", obj->short_descr);
          act (buf, ch, prev, NULL, TO_ROOM);
          act (buf, ch, prev, NULL, TO_CHAR);
	      if (obj->pIndexData->vnum <= 5 && obj->pIndexData->vnum >= 1)
		{
		  int silver = 0;
		  int gold = 0;
		  OBJ_DATA *tobj, *tobj_next;
		  for (tobj = ch->in_room->contents; tobj != NULL; tobj = tobj_next)
		    {
		      tobj_next = tobj->next_content;
		      switch (tobj->pIndexData->vnum)
			{
			case OBJ_VNUM_SILVER_ONE:
			  silver += 1;
			  extract_obj (tobj);
			  break;
			case OBJ_VNUM_GOLD_ONE:
			  gold += 1;
			  extract_obj (tobj);
			  break;
			case OBJ_VNUM_SILVER_SOME:
			  silver += tobj->value[0];
			  extract_obj (tobj);
			  break;
			case OBJ_VNUM_GOLD_SOME:
			  gold += tobj->value[1];
			  extract_obj (tobj);
			  break;
			case OBJ_VNUM_COINS:
			  silver += tobj->value[0];
			  gold += tobj->value[1];
			  extract_obj (tobj);
			  break;
			}
		    }
		  obj_to_room (create_money (gold, silver), ch->in_room);

		}
	}
      obj_from_room(obj);
    }
  extract_obj(obj);
}


void do_hellbreath (CHAR_DATA* ch, char* argument)
{
  int chance;
  CHAR_DATA* victim;
  
  if ((chance = get_skill(ch,gsn_hellbreath)) < 1)
    {
      send_to_char("Your breath may smell like hell, but you can't hellbreathe.\n\r",ch);
      return;
    }

  if (argument[0] == '\0')
    {
      victim = ch->fighting;
      if (victim == NULL)
        {
          send_to_char ("But you aren't fighting anyone!\n\r", ch);
          return;
        }
    }

  else if ((victim = get_char_room (ch, argument)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (is_safe (ch, victim))
    return;
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (ch->mana < skill_table[gsn_hellbreath].min_mana)
    {
      send_to_char ("You don't have enough mana.\n\r",ch);
      return;
    }
  else ch->mana -= skill_table[gsn_hellbreath].min_mana;
  if (ch->hit <= 10)
    {
      send_to_char("You don't have enough strength.\n\r",ch);
      return;
    }
  else ch->hit -= 10;
  WAIT_STATE (ch, skill_table[gsn_hellbreath].beats);
  if (number_percent() < chance)
    {
      act("You breathe green fire at $N!",ch,NULL,victim,TO_CHAR);
      act("$n scorches you with $s hellish breath!",ch,NULL,victim,TO_VICT);
      act("$n scorches $N with $s hellish breath!",ch,NULL,victim,TO_NOTVICT);
      damage_old (ch, victim, (ch->level + dice(2,25)), gsn_hellbreath, DAM_NEGATIVE, TRUE);
    }
  else
    {
      act("You breathe green fire at $N, but MISS!",ch,NULL,victim,TO_CHAR);
      act("$n scorches you with $s hellish breath, but MISSES!",ch,NULL,victim,TO_VICT);
      act("$n scorches $N with $s hellish breath, but MISSES!",ch,NULL,victim,TO_NOTVICT);
      damage_old (ch, victim, 0, gsn_hellbreath, DAM_NEGATIVE, TRUE);
    }
}
  


void feed_death(CHAR_DATA* victim, CHAR_DATA* killer)
{
  DESCRIPTOR_DATA* d;
  
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->connected == CON_PLAYING && IS_AWAKE (d->character) 
	  && d->pEdit == NULL && d->editor == 0 && d->character->in_room 
	  && get_skill(d->character,gsn_feed_death) > number_percent()
	  && d->character->in_room->area == victim->in_room->area)
	{
	  if (killer == d->character)
	    act("You feel your victim's life course through your veins as $e gasps $s last breath.",d->character,NULL,victim,TO_CHAR);
	  else if (victim->in_room == d->character->in_room)
		send_to_char("You watch the formation of the unseeing stare, and you feel the rush of new energy!\n\r",d->character);
	  else send_to_char("The death of a nearby creature sends thrills through your body as it replenishes you.\n\r",d->character);
	  act("$n gets a brief look of extacy on $s face as $e drinks the rapture of a nearby death.",d->character,NULL,NULL,TO_ROOM);
	  check_improve (d->character, gsn_feed_death, FALSE, 2);
	  if (IS_NPC(victim))
	    d->character->hit += (2*(victim->level/5.0));
	  else d->character->hit += (2*(victim->level));
	}
    }
}

void corpse_carve(CHAR_DATA* ch, char* argument)
{
  int chance=0,i;
  OBJ_DATA *corpse, *obj, *weapon, *prev, *prev_next;
  char *msg, *name;
  int vnum;
  short both=2; //0 = no need to sprintf in short and long, 1 = short only, 2 = both
  char buf[MAX_STRING_LENGTH];
  if ((chance = get_skill (ch, gsn_corpse_carving)) < 1)
    {
      send_to_char ("You know nothing about carving corpses.\n\r", ch);
      return;
    }
  weapon = get_eq_char (ch, WEAR_WIELD_R);
  if (weapon == NULL || (weapon->value[0] != WEAPON_DAGGER))
    weapon = get_eq_char (ch, WEAR_WIELD_L);
  if (weapon == NULL || (weapon->value[0] != WEAPON_DAGGER))
    {
              send_to_char("You need to be wielding a dagger to properly carve.\n\r", ch);
              return;
    }
  if (argument[0] == '\0')
    {
      send_to_char("Syntax : carve corpse\n\r",ch);
      return;
    }
  if (!str_prefix(argument,"corpse"))
    {
      if ((corpse = get_obj_vnum_room(ch->in_room,OBJ_VNUM_CORPSE_NPC)) == NULL
	  && ((corpse = get_obj_vnum_char(ch,OBJ_VNUM_CORPSE_NPC)) == NULL))
	{
	  send_to_char("There is no corpse here to carve.\n\r",ch);
	  return;
	}
      if (chance > number_percent())
	{
	  vnum = 0;
	  chance = dice (1,2);
	  act("$n systematically dissects $p, removing limbs and digging around for bits of muscle and tissue.",ch,corpse,NULL,TO_ROOM);
	  act("You dissect $p, removing what parts you can.",ch,corpse,NULL,TO_CHAR);
	  for (prev = corpse->contains; prev != NULL; prev = prev_next)
	    {
	      prev_next = prev->next_content;
	      obj_from_obj(prev);
	      obj_to_room(prev,ch->in_room);
	      sprintf (buf, "$p falls from %s.", corpse->short_descr);
              act (buf, ch, prev, NULL, TO_ROOM);
              act (buf, ch, prev, NULL, TO_CHAR);
	    }
	  for (i=0;i<chance;i++)
	    {
	      both = 2;
	      switch(number_range(1,10))
		{
		case 1:
		  msg =  "$n's arm is sliced from $s dead body.";
		  vnum = OBJ_VNUM_SLICED_ARM;
		  break;
		case 2:
		  vnum = OBJ_VNUM_SLICED_LEG;break;
		case 3:
		  vnum = OBJ_VNUM_SEVERED_HEAD;break;
		case 4:
		  vnum = OBJ_VNUM_GUTS;break;
		case 5:
		  vnum = OBJ_VNUM_TORN_HEART;break;
		case 6:
		  vnum = OBJ_VNUM_BRAINS;break;
		case 7:
		  vnum = OBJ_VNUM_RIB_BONE;both=0;break;
		case 8:
		  vnum = OBJ_VNUM_THIGH_BONE;both=0;break;
		case 9:
		  vnum = OBJ_VNUM_SINEW;both=0;break;
		default:
		  vnum = OBJ_VNUM_SKIN;both=1;break;
		}
	      name = strstr (corpse->short_descr,"of");
	      if (strlen(name) >= 4)
		name += 3;
	      else return;
	      obj = create_object (get_obj_index (vnum), 0);
	      if (obj->pIndexData->vnum != OBJ_VNUM_RIB_BONE && obj->pIndexData->vnum != OBJ_VNUM_THIGH_BONE)
		obj->timer = number_range (4, 7);
	      if (both)
	      {
	        sprintf (buf, obj->short_descr, name);
	        free_string (obj->short_descr);
	        obj->short_descr = str_dup (buf);
	      }
	      if (both==2)
	      {
	        sprintf (buf, obj->description, name);
	        free_string (obj->description);
	        obj->description = str_dup (buf);
	      }
	      if (obj->item_type == ITEM_FOOD)
		{
		  if (IS_SET (ch->form, FORM_POISON))
		    obj->value[3] = 1;

		  else if (!IS_SET (ch->form, FORM_EDIBLE))
		    obj->item_type = ITEM_TRASH;
		}
	      obj_to_room (obj, ch->in_room);
	      if (vnum == OBJ_VNUM_SINEW || vnum == OBJ_VNUM_SKIN)
	      {
	        obj->timer = 180;
	      }
	    }
	  obj_from_room(corpse);
	  extract_obj(corpse);
	}
      else
	{
	  act("$n attempts to systematically dissect $p, but fails miserably.",ch,corpse,NULL,TO_ROOM);
	  act("You attempt to systematically dissect $p, but fail miserably.",ch,corpse,NULL,TO_CHAR);
	  for (prev = corpse->contains; prev != NULL; prev = prev_next)
            {
              prev_next = prev->next_content;
              obj_from_obj(prev);
              obj_to_room(prev,ch->in_room);
	      sprintf (buf, "$p falls from %s.", corpse->short_descr);
	      act (buf, ch, prev, NULL, TO_ROOM);
	      act (buf, ch, prev, NULL, TO_CHAR);
            }
	  obj_from_room(corpse);
          extract_obj(corpse);
	}

    }
  //generat usage message
  else corpse_carve(ch,"");
}

void do_unsummon(CHAR_DATA* ch, char* argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;
  one_argument (argument, arg);
  if (IS_NPC(ch))
    return;
  if ((chance = get_skill (ch, gsn_unsummon)) < 1)
    {
      send_to_char ("Unsummun...Unsumon..Unsmmon..You can't even spell it much less do it.\n\r", ch);
      return;
    }
  if (arg[0] == '\0')
    {
          send_to_char ("Unsummon who?\n\r", ch);
          return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (!IS_SET(victim->act2,ACT_FAMILIAR))
    {
      send_to_char ("You cannot unsummon that.\n\r",ch);
      return;
    }
  if (chance > number_percent())
  {
    act("$n sends $N back to the underworld.",ch,NULL,victim,TO_ROOM);
    act("You send $N back to the world of the dead.",ch,NULL,victim,TO_CHAR);
      if (victim != NULL)
      {
        OBJ_DATA *obj,*obj_next;
        for (obj = victim->carrying; obj; obj = obj_next)
        {
          obj_next = obj->next_content;
          unequip_char (victim, obj);
          obj_from_char (obj);
          obj_to_room (obj,ch->in_room);
          act ("$p falls to the ground", ch, obj, NULL, TO_ALL);
        }
      }

    extract_char(victim,TRUE);
    if (chance*.50 > number_percent())
    {
	  if (ch->mana + 60 <= ch->max_mana)
 	    ch->mana += 60;
	  else ch->mana = ch->max_mana;
    }
  }
  else 
  {
    send_to_char("Your unsummoning efforts fail.\n\r",ch);
    act("$n attempts to send $N back to the underworld, but fails.",ch,NULL,victim,TO_ROOM);
  }
}

void do_makenecro(CHAR_DATA *imm, char *argument)
{
  CHAR_DATA *ch;
  OBJ_DATA *obj,*obj_next, *obj3;
  short tempb=0;
  ch = get_char_world(imm,argument);
  if (ch == NULL)
  {
    send_to_char("That person isn't logged in or you can't spell for shit.\n\r",imm);
    return;
  }
  if (get_trust (ch) >= get_trust (imm))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  if (ch->Class != 0)
	  ++tempb;
  if (ch->Class2 != 0)
	  ++tempb;
  if (ch->Class3 != 0)
	  ++tempb;
  if (tempb > 1)
  {
	  send_to_char("You must be at least 2/3rds mage to remort into Necromancer.\n\r",ch);
	  return;
  }
  if (ch->level < 90)
  {
	  send_to_char("You are not high enough level to remort.\n\r",ch);
	  return;
  }
	  
  if (((obj = get_obj_vnum_char(ch,OBJ_VNUM_NECRO1)) == NULL)
         || ((obj_next = get_obj_vnum_char(ch,OBJ_VNUM_NECRO2)) == NULL)
         || ((obj3 = get_obj_vnum_char(ch,OBJ_VNUM_NECRO3)) == NULL))
  {
     send_to_char("You do not have all of the desired objects.\n\r",ch);
     return;
  }
  extract_obj(obj);
  extract_obj(obj_next);
  extract_obj(obj3);
	
  if (ch->Class != 0)
    ch->pcdata->old_Class = ch->Class;
  else if (ch->Class2 != 0)
    ch->pcdata->old_Class = ch->Class2;
  else ch->pcdata->old_Class = ch->Class3;
  ch->level = 1;
  set_title (ch, "``the `kNecromancer``.");
  ch->exp = 0;
  ch->pcdata->totalxp = 0;
  do_reroll(ch,"");
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
  ch->Class = 10;
  ch->Class2 = 10;
  ch->Class3 = 10;
  group_add_all (ch);
  ch->pcdata->hp_gained = 0;
  ch->pcdata->mana_gained = 0;
  ch->pcdata->move_gained = 0;
  affect_strip_skills(ch);
  ch->max_hit = 20;
  ch->hit = 20;
  ch->max_mana = 100;
  ch->mana = 100;
  ch->max_move = 100;
  ch->move = 100;
  ch->pcdata->perm_hit = 20;
  ch->pcdata->perm_mana = 100;
  ch->pcdata->perm_move = 100;
  send_to_char("While you are level 1, you may change your familiar type.\n\r",ch);
  send_to_char("It defaults to a Banshee.  You may also chose a Skeleton or a Zombie.\n\r",ch);
  send_to_char("You may chose by typing `kfamiliar TYPE NAME``, where TYPE is either\n\r",ch);
  send_to_char("skeleton, zombie, or banshee, and NAME is the permanent name you wish your\n\r",ch);
  send_to_char("familiar to have.  This name is very important as it is used FOREVER.\n\r",ch);
  send_to_char("You do not have to give your familiar a name if you do not want to, but it is\n\r",ch);
  send_to_char("HIGHLY SUGGESTED that you do so.\n\r",ch);
  send_to_char("You are now a necromancer.  Go forth and prosper.\n\r",ch);
  return;
}

void do_familiar(CHAR_DATA *ch, char *argument)
{
  char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];
  if ((IS_NPC(ch) || ch->Class != PC_CLASS_NECROMANCER || ch->level != 1) && !IS_IMMORTAL(ch))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  argument = one_argument(argument,arg1);
  one_argument(argument,arg2);
  if (arg1[0] == '\0')
  {
	  send_to_char("Syntax : familiar TYPE NAME, where TYPE = zombie, skeleton, banshee, and \n\r",ch);
	  send_to_char("         NAME = the name you wish to give them.\n\r",ch);
	  return;
  }
  if (!str_cmp(arg1,"skeleton"))
  {
    ch->pcdata->familiar_type = FAMILIAR_SKELETON;
    send_to_char("Familiar type set to Skeleton.\n\r",ch);
  }
  else if (!str_cmp(arg1,"zombie"))
  {
    ch->pcdata->familiar_type = FAMILIAR_ZOMBIE;
    send_to_char("Familiar type set to Zombie.\n\r",ch);
  }
  else if (!str_cmp(arg1,"banshee"))
  {
    ch->pcdata->familiar_type = FAMILIAR_BANSHEE;
    send_to_char("Familiar type set to Banshee.\n\r",ch);
  }
  else 
  {
    do_familiar(ch,"");
    return;
  }
  if (arg2[0] != '\0')
  {
     char strsave[MAX_STRING_LENGTH];
     struct stat statBuf;
     sprintf (strsave, "%s%c/%s", PLAYER_DIR, toupper (arg2[0]), capitalize (arg2));
     if (!check_parse_name (arg2) || stat (strsave, &statBuf) != -1)
     {
       send_to_char ("Sorry, that name cannot be used for your familiar.\n\r", ch);
       return;
     }									   
    free_string(ch->pcdata->familiar_name);
    ch->pcdata->familiar_name = str_dup(arg2);
    send_to_char (ch->pcdata->familiar_name,ch);
    send_to_char (" set as the name of your familiar.\n\r",ch);
  }
}
  
	  
void do_undeadscore (CHAR_DATA *mch, char *argument)
{
  char buf[MAX_STRING_LENGTH], bigbuf[MAX_STRING_LENGTH], *cptr;
  char skill_list[MAX_LEVEL][MAX_STRING_LENGTH];
  char skill_columns[MAX_LEVEL];
  bool found = FALSE;
  int tcnt = 0,sn=0,lev=0;
  AFFECT_DATA *paf, *paf_last = NULL;
  bool affectie = FALSE;
	
  CHAR_DATA *ch;
  if (mch->in_room == NULL || IS_NPC(mch))
	  return;
  if (!mch->pcdata->familiar || mch->pcdata->familiar->in_room != mch->in_room)
  { 
    if (mch->Class != PC_CLASS_CHAOS_JESTER)
      send_to_char ("Your familiar is not in the room.\n\r",mch);
    else send_to_char ("Your imaginary friend is not in the room.\n\r",mch);
    return;
  }
  ch = mch->pcdata->familiar;
  send_to_char
    ("`b+-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+``\n\r",
     mch);
  send_to_char
    ("`b|`d                  _____________________________________                  `b|``\n\r",
     mch);
  send_to_char
    ("`b|`d         ________|                                     |_______          `b|``\n\r",
     mch);
  sprintf (buf,
           "`b|`d         \\       |" "               `o%-13s  "
           "       `d|      /          `b|``\n\r", ch->name);
  send_to_char (buf, mch);
  if ((ch->clan != CLAN_BOGUS) && is_full_clan (ch))
    sprintf (buf,
             "`b|`d          \\      |           `oof Clan %-16s``  `d|     /           `b|``\n\r",
             get_clan_name_ch (ch));

  else
    strcpy (buf,
            "`b|`d          \\      |                                     |     /           `b|``\n\r");
  send_to_char (buf, mch);
  send_to_char
    ("`b|`d          /      |_____________________________________|     \\           `b|``\n\r",
     mch);
  send_to_char
    ("`b|`d         /__________)                               (_________\\          `b|``\n\r",
     mch);
  if (!IS_NPC (ch))
    for (cptr = ch->pcdata->title; *cptr != '\0'; cptr++)
      {
        if (*cptr == '`')
          {
            tcnt++;
            cptr++;
          }
	if (*cptr == '\0')
          break;
      }
  sprintf (buf, "`b|`a  Title: `g%%-%ds `b|``\n\r", 63 + (tcnt * 2));
  sprintf (bigbuf, buf, IS_NPC (ch) ? "" : ch->pcdata->title);
  send_to_char (bigbuf, mch);

  sprintf (buf, "`b|`a  Level: `g%-8d", ch->level);
  send_to_char (buf, mch);
  if (get_trust (ch) != ch->level)
    {
      sprintf (buf, "  `aTrust: `g%-2d", get_trust (ch));
      send_to_char (buf, mch);
    }
  sprintf (buf, "  `aAge: `g%-3d  `aHours: `g%-5d", get_age (ch),
           (ch->played + (int) (current_time - ch->logon)) / 3600);
  send_to_char (buf, mch);
  if (!IS_NPC (ch))
    {
      switch (ch->pcdata->primary_hand)
        {
        default:
        case HAND_RIGHT:
          sprintf (buf, "`aHand: `g%-12s", "Right");
          break;
        case HAND_LEFT:
          sprintf (buf, "`aHand: `g%-12s", "Left");
          break;
        case HAND_AMBIDEXTROUS:
          sprintf (buf, "`aHand: `g%-12s", "Ambidextrous");
          break;
        }
      send_to_char (buf, mch);
    }
else
  {
    sprintf (buf, "%19s", " ");
    send_to_char (buf, mch);
  }
  if (get_trust (ch) == ch->level)
    sprintf (buf, "%13s`b|``\n\r", "");

  else
    strcpy (buf, " `b|``\n\r");
  send_to_char (buf, mch);
  sprintf (buf,
           "`b|  `aRace: `g%-7s `aGender: `g%-7s  `aClass: `g%-8s`a/ `g%-8s`a/ `g%-8s     `b|``\n\r",
           (ch->pIndexData->vnum == 28)?"Imaginary":"Undead",
           ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
           IS_NPC (ch) ? "mobile" : Class_table[ch->Class].name,
           IS_NPC (ch) ? "mobile" : ((ch->level > 30) ?
                                     Class_table[ch->
						Class2].name :
                                     "???"),
           IS_NPC (ch) ? "mobile" : ((ch->level > 60) ?
                                     Class_table[ch->Class3].name : "???"));
  send_to_char (buf, mch);
  sprintf (buf,
           "`b|  `aExperience: `g%-10ld  `aTo Level: `g%-9ld  `aTrains: `g%-4d              `b|``\n\r",
           ch->exp, level_cost (ch->level), ch->train);
  send_to_char (buf, mch);
  sprintf (buf,
           "`b|  `aHP: `g%5d/%-5d `aMana: `g%5d/%-5d `aMove: `g%5d/%-5d `aWimpy: `g%-5d       `b|``\n\r",
           ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
           ch->max_move, ch->wimpy);
  send_to_char (buf, mch);
  sprintf (buf,
           "`b|      -=[ `oAttributes `b]=-                       -=[ `oArmor `b]=-             |``\n\r");
  send_to_char (buf, mch);
  sprintf (buf,
           "`b|    `aStr: `g%02d(%02d)   `aDex: `g%02d(%02d)             `aPierce: `g%-4d   `aExotic: `g%-4d    `b|``\n\r",
           ch->perm_stat[STAT_STR], get_curr_stat (ch, STAT_STR),
	   ch->perm_stat[STAT_DEX], get_curr_stat (ch, STAT_DEX),
           GET_AC (ch, AC_PIERCE), GET_AC (ch, AC_EXOTIC));
  send_to_char (buf, mch);
  sprintf (buf,
           "`b|    `aInt: `g%02d(%02d)   `aCon: `g%02d(%02d)    `aSave: `g%-3d  `aBash: `g%-4d    `aSlash: `g%-4d    `b|``\n\r",
           ch->perm_stat[STAT_INT], get_curr_stat (ch, STAT_INT),
           ch->perm_stat[STAT_CON], get_curr_stat (ch, STAT_CON),
           ch->saving_throw, GET_AC (ch, AC_BASH), GET_AC (ch, AC_SLASH));
  send_to_char (buf, mch);
  sprintf (buf,
           "`b|    `aWis: `g%02d(%02d)   `aCha: `g%02d(%02d)            `aHitroll: `g%-3d   `aDamroll: `g%-3d     `b|``\n\r",
           ch->perm_stat[STAT_WIS], get_curr_stat (ch, STAT_WIS),
           ch->perm_stat[STAT_CHA], get_curr_stat (ch, STAT_CHA),
           get_hitroll (ch), get_damroll (ch));
  send_to_char (buf, mch);
  sprintf (buf,
           "`b|  `aItems: `g%4d/%-4d    `aWeight: `g%7ld/%-7d                            `b|``\n\r",
           ch->carry_number, can_carry_n (ch),
           get_carry_weight (ch) / 10, can_carry_w (ch) / 10);
  send_to_char (buf, mch);
  sprintf (buf, "`b|  `aGold: `g%-8ld `aSilver: `g%-8ld ", ch->gold,
           ch->silver);
  strcpy (bigbuf, buf);
  if (ch->level >= 10)
    {
      sprintf (buf, "`aAlignment: `g%5d.  ", ch->alignment);
      strcat (bigbuf, buf);
    }

  else
    strcat (bigbuf, "                   ");
  strcat (bigbuf, "`gThey are ");

  // Akamai 6/4/98 -- Remove magic constants, see merc.h
  if (ALIGN_IS_ANGELIC (ch))
    strcat (bigbuf, "angelic.");

  else if (ALIGN_IS_SAINTLY (ch))
    strcat (bigbuf, "saintly.");

  else if (ALIGN_IS_GOOD (ch))
    strcat (bigbuf, "good.   ");

  else if (ALIGN_IS_KIND (ch))
    strcat (bigbuf, "kind.   ");

  else if (ALIGN_IS_NEUTRAL (ch))
    strcat (bigbuf, "neutral.");

  else if (ALIGN_IS_MEAN (ch))
    strcat (bigbuf, "mean.   ");

  else if (ALIGN_IS_EVIL (ch))
    strcat (bigbuf, "evil.   ");

  else if (ALIGN_IS_DEMONIC (ch))
    strcat (bigbuf, "demonic.");

  else
    strcat (bigbuf, "satanic.");
  sprintf (buf, "%s   `b|``\n\r", bigbuf);
  send_to_char (buf, mch);
  if (!IS_NPC(ch))
    {
      sprintf (buf, "`b|  `aQuest Points: `g%-7ld `aRP Points: `g%-7ld `aNewbie Helper Points: `g%-7ld `b|\n\r", ch->pcdata->qpoints, ch->pcdata->rppoints, ch->pcdata->nhpoints);
      send_to_char (buf, mch);
    }
  if (IS_IMMORTAL (ch))
    {
      strcpy (bigbuf, "`b|`a  Holy Light: ");
      if (IS_SET (ch->act, PLR_HOLYLIGHT))
        sprintf (buf, "%-5s", "`gon");

      else
        sprintf (buf, "%-5s", "`goff");
      strcat (bigbuf, buf);
      if (ch->invis_level)
	{
          sprintf (buf, "  `aInvisible: `glevel %-8d", ch->invis_level);
          strcat (bigbuf, buf);
        }

      else
        {
          sprintf (buf, "%27s", " ");
          strcat (bigbuf, buf);
        }
      if (ch->incog_level)
        {
          sprintf (buf, "  `aIncognito: `glevel %-8d", ch->incog_level);
          strcat (bigbuf, buf);
        }

      else
        {
          sprintf (buf, "%27s", " ");
          strcat (bigbuf, buf);
        }
      sprintf (buf, "%s  `b|``\n\r", bigbuf);
      send_to_char (buf, mch);
    }
  strcpy (bigbuf, "`b|``  `a");
  if (!IS_NPC (ch))
    {
      if (ch->pcdata->condition[COND_DRUNK] > 10)
        strcat (bigbuf, "They are drunk.  ");
      if (ch->pcdata->condition[COND_THIRST] <= 0)
        strcat (bigbuf, "They are thirsty.  ");
      if (ch->pcdata->condition[COND_HUNGER] <= 0)
        strcat (bigbuf, "They are hungry.  ");
    }
  if (ch->position == POS_STANDING && ch->fighting != NULL)
    ch->position = POS_FIGHTING;
  switch (ch->position)
    {
    case POS_DEAD:
      strcat (bigbuf, "They are DEAD!!");
      break;
    case POS_MORTAL:
      strcat (bigbuf, "They are mortally wounded.");
      break;
    case POS_INCAP:
      strcat (bigbuf, "They are incapacitated.");
      break;
    case POS_STUNNED:
      strcat (bigbuf, "They are stunned.");
      break;
    case POS_COMA:
    case POS_SLEEPING:
      strcat (bigbuf, "They are sleeping.");
      break;
    case POS_RESTING:
      strcat (bigbuf, "They are resting.");
      break;
    case POS_SITTING:
      strcat (bigbuf, "They are sitting.");
      break;
    case POS_STANDING:
      strcat (bigbuf, "They are standing.");
      break;
    case POS_FIGHTING:
      strcat (bigbuf, "They are fighting.");
      break;
    case POS_MOUNTED:
      strcat (bigbuf, "They are riding a mount.");
      break;
    }
  sprintf (buf, "%-80s`b|``\n\r", bigbuf);
  send_to_char (buf, mch);
  send_to_char
    ("`b+-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+``\n\r",
     mch);

  for (lev = 0; lev < LEVEL_HERO; lev++)
    {
      skill_columns[lev] = 0;
      skill_list[lev][0] = '\0';
    }


  lev = 0;
  for (sn = 0;sn < MAX_SKILL;sn++)
  {
    if(get_skill(ch,sn) > 1)
      {
	found = TRUE;
	lev++;
	  sprintf (buf, "%-18s      ", skill_table[sn].name);

	if (skill_list[lev][0] == '\0')
	{
	  sprintf (skill_list[lev], "\n\r%s",  buf);
	}
        else
          /* append */
          {
            if (++skill_columns[lev] % 2 == 0)
              strcat (skill_list[lev], "\n\r          ");
            strcat (skill_list[lev], buf);
          }
      }
  }
  if (!found)
    {
      send_to_char ("They know no skills.\n\r", mch);
      return;
    }
  else
    {
      if (mch->Class != PC_CLASS_CHAOS_JESTER)
        send_to_char("\n\rYour familiar can use the following skills\n\r",mch);
      else send_to_char("\n\rYour imaginary friend can use the following skills\n\r",mch);
				 
      for (lev = 0; lev < LEVEL_HERO; lev++)
	{
	  if (skill_list[lev][0] != '\0')
	    send_to_char (skill_list[lev], mch);
	}
    }

send_to_char("\n\r\n\r",mch);
  if (ch->affected != NULL)
    {
      affectie = TRUE;
      
      if (mch->Class != PC_CLASS_CHAOS_JESTER)
	send_to_char ("Your familiar is affected by the following:\n\r", mch);
      else send_to_char("Your imaginary friend is affected by the following:\n\r",mch);
      for (paf = ch->affected; paf != NULL; paf = paf->next)
        {
          if (paf_last != NULL && paf->type == paf_last->type
              && !paf_last->composition)
            if (ch->level >= 20)
              sprintf (buf, "                              ");

            else
              continue;

          else
            {
              if (paf->composition == TRUE)
                sprintf (buf, "Song: %-24s", paf->comp_name);

              else
                switch (skill_table[paf->type].type)
                  {
                  case SKILL_NORMAL:
                    if (*skill_table[paf->type].pgsn != gsn_uppercut)
                      sprintf (buf, "Skill: %-23s",
                               skill_table[paf->type].name);

                    else
                      sprintf (buf, "Penalty: %-21s",
                               skill_table[paf->type].name);
                    break;
                  case SKILL_CAST:
                    sprintf (buf, "Spell: %-23s",
                             skill_table[paf->type].name);
                    break;
                  case SKILL_PRAY:
                    sprintf (buf, "Prayer: %-22s",
                             skill_table[paf->type].name);
                    break;
                  case SKILL_CHANT:
                    sprintf (buf, "Chant: %-23s",
                             skill_table[paf->type].name);
                    break;
                  case SKILL_SING:
                    sprintf (buf, "Song: %-24s", skill_table[paf->type].name);
                    break;
                  }
            }
          send_to_char (buf, mch);
          if (ch->level >= 20)
            {
              if (paf->where == TO_SKILL)
		{
		  if(paf->location == 0)
		    sprintf(buf,": modifies all abilities by %d",paf->modifier);
		  else sprintf(buf,": modifies %s by %d",
			       skill_table[paf->location].name,paf->modifier);
		  send_to_char (buf,mch);
		  send_to_char ("% ",mch);
		}
              else
		{
		  sprintf (buf, ": modifies %s by %d ",
			   affect_loc_name (paf->location), paf->modifier);
		  send_to_char (buf, mch);
		}
              if (paf->duration == -1)
		{
		  sprintf (buf, "permanently");
		  send_to_char (buf, mch);
		}

              else if (paf->type != gsn_jurgnation && paf->type != gsn_jalknation
		       && paf->type != skill_lookup("beacon of the damned"))
		{
		  sprintf (buf, "for %d hours", paf->duration);
		  send_to_char (buf, mch);
		}
            }
          send_to_char ("\n\r", mch);
          paf_last = paf;
        }
    }
  if (!IS_NPC(ch))
    {
      if ((race_table[ch->race].aff & AFF_DETECT_MAGIC) && (ch->affected_by & AFF_DETECT_MAGIC))
        {
          if (!affectie)
            {
              affectie = TRUE;
	      if (mch->Class != PC_CLASS_CHAOS_JESTER)
                send_to_char ("Your familiar is affected by the following:\n\r", mch);
	      else send_to_char ("Your imaginary friend is affected by the following:\n\r",mch);
            }
          send_to_char("Racial Affect: detect magic   : modifies none by 0 permanently\n\r",mch);
        }
      if ((race_table[ch->race].aff & AFF_DETECT_HIDDEN) && (ch->affected_by & AFF_DETECT_HIDDEN))
        {
          if (!affectie)
            {
              affectie = TRUE;
	      if (mch->Class != PC_CLASS_CHAOS_JESTER)
              send_to_char ("Your familiar is affected by the following:\n\r", mch);
	      else send_to_char ("Your imaginary friend is affected by the following:\n\r",mch);
            }
          send_to_char("Racial Affect: detect hidden  : modifies none by 0 permanently\n\r",mch);
        }
      if ((race_table[ch->race].aff & AFF_INFRARED) && (ch->affected_by & AFF_INFRARED))
        {
          if (!affectie)
            {
              affectie = TRUE;
	      if (mch->Class != PC_CLASS_CHAOS_JESTER)
              send_to_char ("You familiar is affected by the following:\n\r", mch);
	      else send_to_char ("Your imaginary friend is affected by the following:\n\r",mch);
            }
	  send_to_char("Racial Affect: infravision    : modifies none by 0 permanently\n\r",mch);
        }
      if ((race_table[ch->race].aff & AFF_FLYING) && (ch->affected_by & AFF_FLYING))
        {
          if (!affectie)
            {
              affectie = TRUE;
	      if (mch->Class != PC_CLASS_CHAOS_JESTER)
              send_to_char ("Your familiar is affected by the following:\n\r", mch);
	      else send_to_char ("Your imaginary friend is affected by the following:\n\r",mch);
            }
          send_to_char("Racial Affect: fly            : modifies none by 0 permanently\n\r",mch);
        }
      if ((race_table[ch->race].aff & AFF_AQUA_BREATHE) && (ch->affected_by & AFF_AQUA_BREATHE))
        {
          if (!affectie)
            {
              affectie = TRUE;
	      if (mch->Class != PC_CLASS_CHAOS_JESTER)
              send_to_char ("Your familiar is affected by the following:\n\r", mch);
	      else send_to_char ("Your imaginary friend is affected by the following:\n\r",mch);
            }
          send_to_char("Racial Affect: aqua breathe   : modifies none by 0 permanently\n\r",mch);
        }
      if ((race_table[ch->race].aff & AFF_PASS_DOOR) && (ch->affected_by & AFF_PASS_DOOR))
        {
          if (!affectie)
            {
              affectie = TRUE;
	      if (mch->Class != PC_CLASS_CHAOS_JESTER)
              send_to_char ("Your familiar is affected by the following:\n\r", mch);
	      else send_to_char ("Your imaginary friend is affected by the following:\n\r",mch);
            }
          send_to_char("Racial Affect: pass door      : modifies none by 0 permanently\n\r",mch);
        }

    }


  if (!affectie)
  {
    if (mch->Class != PC_CLASS_CHAOS_JESTER)
      send_to_char ("Your familiar is not affected by any magic.\n\r", mch);
    else send_to_char("Your imaginary friend is not affected by any magic.\n\r",mch);
  }
  return;   
}

void do_undeadgain (CHAR_DATA *ch, char *argument)
{
  if (ch->in_room == NULL || IS_NPC(ch))
    return;
  if (!ch->pcdata->familiar || ch->pcdata->familiar->in_room != ch->in_room || ch->pcdata->familiar_type == FAMILIAR_IMAGINARY_FRIEND)
    {
      send_to_char ("Your familiar is not in the room.\n\r",ch);
      return;
    }
  if (ch->pcdata->familiar_gained_hp + ch->pcdata->familiar_gained_mana 
      + ch->pcdata->familiar_gained_move >= 400)
    {
      send_to_char ("Your familiar has gained all that is possible.\n\r",ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax : undeadgain hp/mana/move.\n\r",ch);
      return;
    }
  if (ch->pcdata->familiar_exp < 1000000)
    {
      send_to_char ("Your familiar does not have enough experience to gain anything.\n\r",ch);
      return;
    }
  if (!str_cmp(argument,"hp"))
    {
      if (ch->pcdata->familiar_gained_hp >= 300)
	{
	  send_to_char("Your familiar has gained all the hp possible.\n\r",ch);
	  return;
	}
      else
	{
	  ch->pcdata->familiar_gained_hp++;
	  ch->pcdata->familiar_max_hit++;
	  ch->pcdata->familiar->max_hit++;
	  ch->pcdata->familiar->hit++;
	  ch->pcdata->familiar->exp -= 1000000;
	  ch->pcdata->familiar_exp -= 1000000;
	  send_to_char("Your familiar gains 1 hp.\n\r",ch);
	}
      return;
    }
  if (!str_cmp(argument,"mana"))
    {
      if (ch->pcdata->familiar_gained_mana >= 300)
        {
          send_to_char("Your familiar has gained all the mana possible.\n\r",ch);
          return;
        }
      else
        {
          ch->pcdata->familiar_gained_mana++;
          ch->pcdata->familiar_max_mana++;
          ch->pcdata->familiar->max_mana++;
          ch->pcdata->familiar->mana++;
	  ch->pcdata->familiar->exp -= 1000000;
          ch->pcdata->familiar_exp -= 1000000;
          send_to_char("Your familiar gains 1 mana.\n\r",ch);
        }
      return;
    }
  if (!str_cmp(argument,"move"))
    {
      if (ch->pcdata->familiar_gained_move >= 300)
        {
          send_to_char("Your familiar has gained all the move possible.\n\r",ch);
          return;
        }
      else
        {
          ch->pcdata->familiar_gained_move++;
          ch->pcdata->familiar_max_move++;
          ch->pcdata->familiar->max_move++;
          ch->pcdata->familiar->move++;
	  ch->pcdata->familiar->exp -= 1000000;
          ch->pcdata->familiar_exp -= 1000000;
          send_to_char("Your familiar gains 1 move.\n\r",ch);
        }
      return;
    }
  else do_undeadgain(ch,""); //generate usage message
}

void do_puppet(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  if (get_skill(ch,skill_lookup("bone puppet")) < 1)
  {
	  send_to_char("Huh?",ch);
    return;
  }
  if (ch->desc == NULL || ch->in_room == NULL)
    return;
  if (ch->desc->original != NULL && !IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_BONE_PUPPET)
    {
      send_to_char
	("You return to your original body. Type replay to see any missed tells.\n\r",
	 ch);
      if (ch->prompt != NULL)
	{
	  free_string (ch->prompt);
	  ch->prompt = NULL;
	}
      sprintf (buf, "$N returns from %s.", ch->short_descr);
      wiznet (buf, ch->desc->original, 0, WIZ_SWITCHES, WIZ_SECURE,
	      get_trust (ch));
      ch->desc->character = ch->desc->original;
      REMOVE_BIT(ch->desc->character->act2,ACT_PUPPETEER);
      ch->desc->original = NULL;
      ch->desc->character->desc = ch->desc;
      ch->desc = NULL;
      return;
    }
  else
    {
      for (victim = ch->in_room->people;victim != NULL;victim = victim->next_in_room)
	 {
	   if (IS_NPC(victim) && victim->pIndexData->vnum == MOB_VNUM_BONE_PUPPET
	       && !str_cmp(ch->name,victim->afkmsg))
	     break;
	 }
       if (victim == NULL)
	 {
	   send_to_char("You need a bone puppet to switch into!\n\r",ch);
	   return;
	 }
       ch->desc->character = victim;
       ch->desc->original = ch;
       victim->desc = ch->desc;
       
       ch->desc = NULL;
       
       sprintf (buf, "$N switches into %s",
		IS_NPC (victim) ? victim->short_descr : victim->name);
       wiznet (buf, ch, NULL, WIZ_SWITCHES, WIZ_SECURE, get_trust (ch));

       /* change communications to match */
       if (ch->prompt != NULL)
	 victim->prompt = str_dup (ch->prompt);
       victim->comm = ch->comm;
       victim->lines = ch->lines;
       SET_BIT(ch->act2,ACT_PUPPETEER);
       send_to_char ("You are now a bone puppet.\n\r", victim);
       
     }
   
}


