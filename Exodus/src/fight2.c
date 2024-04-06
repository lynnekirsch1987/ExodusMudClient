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
#include "merc.h"
#include "interp.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"

//Iblis - created this file since fight.c is so fucking big, all new fight stuff goes here

void pk_chan args((char* text));

//Iblis - Kalian fear skill
void do_fear (CHAR_DATA* ch, char* argument)
{
  CHAR_DATA *victim, *victim_next;
  ROOM_INDEX_DATA *was_in, *now_in;
  int attempt;
  
  if (get_skill(ch,gsn_fear) < 1)
  {
    send_to_char("If you dressed up as a mouse, even an elephant wouldn't be scared of you.\n\r",ch);
    return;
  }


  if (is_affected (ch, gsn_fear))
  {
	  send_to_char ("You are way too scared to consider scaring anyone.\n\r", ch);
	  return;
  }
  
  if (IS_AFFECTED (ch, AFF_CALM) || is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char ("You are feeling too mellow to frighten anyone.\n\r", ch);
    return;
  }

  WAIT_STATE (ch, skill_table[gsn_fear].beats);
  send_to_char("You attempt to strike fear into your assailants.\n\r",ch);

  for (victim = ch->in_room->people; victim != NULL; victim = victim_next)
    {
      victim_next = victim->next_in_room;

      if (victim == ch)
        continue;

      if (get_position(victim) <= POS_SLEEPING || victim->position == POS_MOUNTED)
        continue;

      if (IS_AFFECTED (victim, AFF_CALM))
        continue;

      if (!IS_NPC(victim) && victim->Class == PC_CLASS_CHAOS_JESTER)
        continue;
      
      if (IS_NPC (victim) && (victim->pIndexData->pShop != NULL ||
                              IS_SET (victim->act, ACT_TRAIN)
                              || IS_SET (victim->act, ACT_PRACTICE)
                              || IS_SET (victim->act, ACT_IS_HEALER) 
			      || IS_SET (victim->act2, ACT_NOMOVE)))
        continue;


      if (saves_spell (ch->level, victim, DAM_MENTAL))
        continue;
    
      if (get_skill(ch,gsn_fear) < number_percent())
        continue;

      if (ch->fighting != victim && victim->fighting != ch)
        continue;

      if (victim->position == POS_RESTING || victim->position == POS_SITTING)
        do_stand (victim, "");

      was_in = victim->in_room;
      for (attempt = 0; attempt < 6; attempt++)
        {
          EXIT_DATA *pexit;
          int door;
	  AFFECT_DATA af;
          door = attempt;
          if ((pexit = was_in->exit[door]) == 0
              || pexit->u1.to_room == NULL
	      || number_percent() < 50
              || IS_SET (pexit->exit_info, EX_CLOSED)
              || (IS_NPC (victim) && IS_SET (pexit->u1.to_room->room_flags, ROOM_NO_MOB)))
            continue;

          act ("You have struck fear directly into $N's heart, causing $M to flee with terror.",
               ch, NULL, victim, TO_CHAR);
          act ("$N has struck fear directly into $n's heart, causing $m to flee with terror.",
               victim, NULL, ch, TO_NOTVICT);
          act ("$N has struck fear directly into your heart, causing you to you flee for your life.",
               victim, NULL, ch, TO_CHAR);
          move_char (victim, door, FALSE);

	  
	  af.where = TO_AFFECTS;
	  af.type = gsn_fear;
	  af.level = ch->level;
	  af.duration = number_range(0,(ch->level > 75)?2:1);
	  af.location = APPLY_HITROLL;
	  af.permaff = FALSE;
	  af.composition = FALSE;
	  af.comp_name = str_dup ("");
	  if (!IS_NPC (victim))
		    af.modifier = -5;
	  else
		    af.modifier = -2;
	  af.bitvector = AFF_CALM;
	  affect_to_char (victim, &af);

          if ((now_in = victim->in_room) != was_in)
            break;
        }

      if ((now_in = victim->in_room) == was_in)
        continue;

      stop_fighting (victim, TRUE);
    }
}

//Iblis - Kalian illusion skill
void do_illusion (CHAR_DATA* ch, char* argument)
{
  CHAR_DATA *victim;//, *victim_next;
  MOB_INDEX_DATA* pMobIndex;
  char buf[MAX_STRING_LENGTH];
  
  if (get_skill(ch,gsn_illusion) < 1)
  {
    send_to_char("You attempt to make an illusion, but alas, you have NO skill.\n\r",ch);
    return;
  }

  if ((pMobIndex = get_mob_index (MOB_VNUM_ILLUSION)) == NULL)
  {
    send_to_char ("No mob has that vnum.\n\r", ch);
    return;
  }

  if (IS_NPC(ch))
    return;

  if (ch->mana < 150 || ch->il_ticks != 0)
  {
    send_to_char ("You do not have the strength to make an illusion.\n\r",ch);
    return;
  }

  ch->mana -= 150;
  ch->il_ticks = 24;
  victim = create_mobile (pMobIndex);
  SET_BIT(victim->act, ACT_SENTINEL);
  SET_BIT(victim->act2, ACT_ILLUSION);
  SET_BIT(victim->act2, ACT_NOMOVE);
  free_string(victim->name);
  victim->name = str_dup(ch->name);
  free_string(victim->short_descr);
  victim->short_descr = str_dup(ch->name);
  free_string(victim->long_descr);
  strcpy(buf,"`g");
  strcat(buf,capitalize(ch->name));
  strcat(buf,ch->pcdata->title);
  strcat(buf,"`` is here.\n\r");
  victim->long_descr = str_dup(buf);
  victim->ticks_remaining = ch->level/10;
  char_to_room (victim, ch->in_room);
  act("An illusion appears out of thin air.\n\r",ch,NULL,NULL,TO_ROOM);
  send_to_char("You successfully conjure an illusion.\n\r",ch);
  WAIT_STATE (ch, skill_table[gsn_illusion].beats);
  
 
}

//Iblis - sets oneself killable (turns off multikill protection)
void do_killable(CHAR_DATA* ch, char* argument)
{
  char buf[MAX_STRING_LENGTH];
  if (IS_NPC(ch))
    return;
//  if (current_time - ch->pcdata->last_pkdeath < (60*MULTIKILL_WAIT_TIME)) 
  if (ch->pcdata->last_pkdeath != 0)
  {
     ch->pcdata->last_pkdeath = 0;
     send_to_char("You have declared yourself killable to the world.\n\r",ch);
     sprintf(buf,"`l%s's `kprotective aura fades.``",ch->name);
     pk_chan(buf);
     WAIT_STATE (ch, PULSE_VIOLENCE*5);
  }
  else
  {
     send_to_char("The world already knows you can be killed.  Apparently you forgot, though.\n\r",ch);
     return;
  }
}

//Iblis - Kalian suck skill
void do_suck (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  int dam, multiplier;
  if (!IS_NPC (ch) && ch->level < level_for_skill (ch, gsn_suck))
    {
      send_to_char ("You better leave sucking to the Kalians.\n\r", ch);
      return;
    }
  if (get_skill (ch, gsn_suck) < 1)
    {
      send_to_char ("You don't know how to suck (yes you are very stupid)!\n\r", ch);
      return;
    }
  multiplier = 1;
  if (argument[0] == '\0')
  {
    if ((victim = ch->fighting) == NULL)
      {
        send_to_char ("You aren't fighting anyone.\n\r", ch);
        return;
      }
  }
  else if ((victim = get_char_room (ch, argument)) == NULL)
    {
       send_to_char ("They aren't here.\n\r", ch);
       return;
    }
  if (ch == victim)
  {
	  send_to_char("You suck yourself, and you enjoy it fully.\n\r",ch);
	  return;
  }
  if (ch->fighting == NULL && ch->position == POS_STANDING)
    multiplier *= 2;
  WAIT_STATE (ch, skill_table[gsn_suck].beats);
  if (get_skill (ch, gsn_suck) > number_percent ())
    {
      dam = 2*number_range (1, ch->level)*multiplier;
      if (IS_SET(victim->act,ACT_UNDEAD))
        dam=0;
      if (dam > 0 && check_immune (victim, DAM_NEGATIVE) != IS_IMMUNE && !(is_safe (ch, victim)))
      {
        act ("$N's vampiric bite `bdraws`` life from $n.", victim, NULL, ch, TO_NOTVICT);
        act ("Your vampiric bite `bdraws`` life from $n.", victim, NULL, ch, TO_VICT);
	act ("You feel $N's vampiric bite `bdrawing`` your life away.", victim, NULL, ch, TO_CHAR);
	damage_old (ch, victim, dam, gsn_suck, DAM_NEGATIVE, FALSE);
	if ( time_info.hour > 18 && time_info.hour < 5)
          multiplier *= 2;
        ch->hit += (((dam/9) < 5)?5:dam/9*multiplier);
      }
      else 
      {
        act ("You miss $n completely.  Can you even attack them?", victim, NULL, ch, TO_VICT);
      }
      check_improve (ch, gsn_suck, TRUE, 1);
    }

  else
    {
      damage (ch, victim, 0, gsn_suck, DAM_VAMPIRIC, FALSE);
      act ("$N's vampiric bite `imisses`` $n.", victim, NULL, ch, TO_NOTVICT);
      act ("Your vampiric bite `bmisses`` $n.", victim, NULL, ch, TO_VICT);
      act ("You watch as $N's vampiric bite `bmisses ``you.", victim, NULL, ch, TO_CHAR);
      check_improve (ch, gsn_suck, FALSE, 1);
    }
  return;
}

//Iblis - A function to adjust the damage so it does the damage the code says it should
//(IE it makes it high enough so after dam adjusting code in damage() it does
//the correct amound of damage
int adjust_damage(int dam_to_do)
{
  int damage;
  if (dam_to_do < 35)
    return dam_to_do;
  if (dam_to_do <= 80)
  {
    damage = ((dam_to_do-35)*2)+35;
  }
  else
  {
    damage = ( (( ((dam_to_do-80)*2) +45)*2) +35);
  }
  return damage;
}

//Iblis - toggle for the ranger aggression skill
void do_aggression (CHAR_DATA * ch, char *argument)
{
  if (get_skill (ch, gsn_aggression) < 1 || IS_NPC(ch))
    {
      send_to_char ("You don't know how to be aggressive!\n\r", ch);
      return;
    }
  if (!str_cmp(argument,"rage"))
  {
    if (ch->pcdata->aggression)
    {
      send_to_char("You get a little more aggressive.\n\r",ch);
      return;
    }
    else 
    {
      ch->pcdata->aggression = 1;
      send_to_char("`iYou are overcome with the desire to kill!\n\r",ch);
      act("$n's skin turns red as $e works $mself into a frenzy!",ch,NULL,NULL,TO_ROOM);
      WAIT_STATE (ch, skill_table[gsn_aggression].beats);
      return;
    }
  }
  if(!str_cmp(argument,"calm"))
  {
    if (!ch->pcdata->aggression)
    {
      send_to_char("A relaxing massage couldn't get you any calmer.\n\r",ch);
      return;
    }
    else
    {
      ch->pcdata->aggression = 0;
      send_to_char("You close your eyes and take deep breaths to calm yourself down.\n\r",ch);
      act("$n closes $s eyes and takes deep breaths to calm down.\n\r",ch,NULL,NULL,TO_ROOM);
      WAIT_STATE (ch, skill_table[gsn_aggression].beats);
      return;
    }
  }
  send_to_char("You are currently ",ch);
  if (ch->pcdata->aggression)
	  send_to_char("overcome with a desire to kill anything that crosses your path.\n\r",ch);
  else send_to_char("without a care in the world.\n\r",ch);
}

//Iblis - simple function which checks aggression should be triggered when ch enters a room
bool check_aggression(CHAR_DATA *ch)
{
  int i = 0;
  int counter = 0;
  CHAR_DATA *tch=NULL;
  if (IS_NPC(ch))
    return FALSE;
  if (ch->pcdata->aggression && (ch->pcdata->loner || ch->clan != CLAN_BOGUS))
  {
    for(tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
    {
      if (!IS_NPC(tch) && ch != tch && (tch->pcdata->loner || tch->clan != CLAN_BOGUS) && can_see(ch,tch) && !IS_IMMORTAL(tch))
        ++counter;
    }
    if (counter > 0)
    {
      counter = number_range(1,counter);

      for(tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
      {
        if (!IS_NPC(tch) && ch != tch && (tch->pcdata->loner || tch->clan != CLAN_BOGUS) && can_see(ch,tch) && !IS_IMMORTAL(tch))
  	  if (counter == ++i)
	  {
	    act("`i$n snarls and throws $mself at you!``",ch,NULL,tch,TO_VICT);
	    act("`i$n snarls and throws $mself at $N!``",ch,NULL,tch,TO_NOTVICT);
	    do_kill(ch,tch->name);
	    return TRUE;
	  }
      }
			

    }
    else return FALSE;
  }
  else
  {
    for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
      if (!IS_NPC(tch) && tch->pcdata->aggression && (tch->pcdata->loner || tch->clan != CLAN_BOGUS) 
        && tch->fighting == NULL && can_see(tch,ch))
      {
        act("`i$n snarls and throws $mself at you!``",tch,NULL,ch,TO_VICT);
        act("`i$n snarls and throws $mself at $N!``",tch,NULL,ch,TO_NOTVICT);		
        do_kill(tch,ch->name);
	return TRUE;
      }
  }
  return FALSE;
}

/*bool check_gaze(CHAR_DATA *ch)
{
	int counter = 0;
	CHAR_DATA *tch = NULL;

	if (IS_NPC(ch))
		return FALSE;

	// Check for noclans or immortals.
	if ((!ch->pcdata->loner && ch->clan == CLAN_BOGUS) || IS_IMMORTAL(ch))
		return FALSE;

	if (get_skill(ch, gsn_gaze) > 0)
	{
		for (tch = ch->in_room->people; tch != NULL; tch = tch->next_in_room)
		{
			if (isAffectedByGaze(tch) && (ch != tch) && !is_same_group(ch, tch))
			{
				lich_gaze(ch, tch);
				counter++;
			}
		}
	}
	else
	{
		for (tch = ch->in_room->people; tch != NULL; tch->next_in_room)
		{
			if ((get_skill(tch, gsn_gaze) > 0) && isAffectedByGaze(ch) &&
				(ch != tch) && !is_same_group(ch, tch) && !IS_IMMORTAL(tch))
				{
					lich_gaze(tch, ch);
					counter++;
				}
		}
	}

	if (counter > 0)
		return TRUE;
	else
		return FALSE;
}

bool isAffectedByGaze(CHAR_DATA *ch)
{
	if (IS_NPC(ch))
		return FALSE;

	else if (!(ch->pcdata->loner || ch->clan != CLAN_BOGUS))
		return FALSE;
	else if (IS_IMMORTAL(ch))
		return FALSE;
	else if (ch->race == PC_RACE_LICH)
		return FALSE;
	else if (is_affected(ch, gsn_berserk) ||
		 is_affected(ch, skill_lookup("frenzy")) ||
		 is_affected(ch, skill_lookup("demonic possession")))
		return FALSE;

	return TRUE;
}*/

//Iblis - function called in fight.c when a stun hits
void stun_char(CHAR_DATA *ch, CHAR_DATA *victim)
{
	short stunamount;
	if (victim->stunned)
		return;
	stunamount = number_range(1,3);
	if (victim->stunned > 0)
  	  victim->stunned += stunamount;
	else victim->stunned = stunamount;
	WAIT_STATE (victim, 12*stunamount);
        act ("`bYou strike a powerful blow to $N's `bnerve center, stunning $M.``",
          ch, NULL, victim, TO_CHAR);
        act ("`b$n strikes you with a powerful blow that makes you go numb.``", ch,
              NULL, victim, TO_VICT);
        act ("`b$n stuns $N `bwith a powerful strike, temporarily paralyzing $M.``", ch,
              NULL, victim, TO_NOTVICT);
		     
	return;
}

//Iblis - Nidae spear skill
void do_spear (CHAR_DATA *ch, char *argument)
{
  int chance;
  CHAR_DATA *victim;
  if ((chance = get_skill(ch,gsn_spear)) < 1)
    {
      send_to_char("You pretend to get speared through the heart, but no one is buying it.\n\r",ch);
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
  if (IS_AFFECTED (ch, AFF_CALM) || is_affected (ch, skill_lookup
                                                 ("Quiet Mountain Lake")))
    {
      send_to_char ("You are feeling too peaceful to do that now.\n\r", ch);
      return;
    }
  if (ch->hit < 15)
  {
	  send_to_char("You do not have enough within you to create a spear.\n\r",ch);
	  return;
  }
  else ch->hit -= 15;
  WAIT_STATE (ch, skill_table[gsn_spear].beats);
  if (chance > number_percent ())
    {
      act("You summon your strength to form a spear made of pure ice and throw it at $N.",ch,NULL,victim,TO_CHAR);
      act("$n seems to get a little shorter as $e forms a spear of ice and throw it at $N.",ch,NULL,victim,TO_NOTVICT);
      act("$n seems to get a little shorter as $e forms a spear of ice and throw it at you.",ch,NULL,victim,TO_VICT);
      damage (ch, victim, ch->level-14 + dice(1,10), gsn_spear, DAM_COLD, TRUE);
      return;
    }
  else
    {
      act("You summon your strength to form a spear made of pure ice and throw it at $N.",ch,NULL,victim,TO_CHAR);
      act("$n seems to get a little shorter as $e forms a spear of ice and throw it at $N.",ch,NULL,victim,TO_NOTVICT);
      act("$n seems to get a little shorter as $e forms a spear of ice and throw it at you.",ch,NULL,victim,TO_VICT);
      damage (ch, victim, 0, gsn_spear, DAM_COLD, TRUE);
      return;
    }
}

void larva_hit(CHAR_DATA *ch,CHAR_DATA *victim)
{
  AFFECT_DATA *paf, af;
  for (paf = victim->affected; paf != NULL; paf = paf->next)
  {
    if (paf->type == gsn_larva)
      break;
  }
  if (paf == NULL)
    {
      af.where = TO_AFFECTS;
      af.type = gsn_larva;
      af.level = ch->level * 3 / 4;
      af.duration = 10;
      af.location = APPLY_NONE;
      af.modifier = 5;
      af.bitvector = 0;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup (ch->name);
      affect_join (victim, &af);
    }
  else
    {
      af.where = TO_AFFECTS;
      af.type = gsn_larva;
      af.level = ch->level * 3 / 4;
      af.duration = paf->duration;
      af.location = APPLY_NONE;
      af.modifier = paf->modifier;
      af.bitvector = 0;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup (ch->name);
      af.duration += 10;
      if (af.duration > 20)
        af.duration = 20;
      af.modifier += 1;
      if (af.modifier > 10)
	af.modifier = 10;
      affect_remove(victim,paf);
      affect_to_char(victim,&af);
    }
  
}

// Lich Gaze Skill
/*void lich_gaze(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int percent, chance, save, level_difference = ch->level - victim->level;

	percent = get_skill(ch, gsn_gaze);

	if (percent == 100)
		chance = dice(8, 6);
	else if (percent <= 66)
		chance = dice(4, 6);
	else if (percent <= 33)
		chance = dice(2, 6);

	save = victim->saving_throw - dice(1, 20);

	if ((victim->Class == PC_CLASS_PALADIN) && (victim->alignment > 300))
		save -= dice(1, 6);

	if ((victim->Class2 == PC_CLASS_PALADIN) && (victim->level > 30) && (victim->alignment > 300))
		save -= dice(1, 6);

	if ((victim->Class3 == PC_CLASS_PALADIN) && (victim->level > 60) && (victim->alignment > 300))
		save -= dice(1, 6);

	if ((victim->Class == PC_CLASS_CLERIC) && (victim->alignment > 300))
		save -= dice(1, 6);

	if ((victim->Class2 == PC_CLASS_CLERIC) && (victim->level > 30) && (victim->alignment > 300))
		save -= dice(1, 6);

	if ((victim->Class3 == PC_CLASS_CLERIC) && (victim->level > 60) && (victim->alignment > 300))
		save -= dice(1, 6);

	if ((chance + save + level_difference) > 0)
	{
		short paralysis;
		paralysis = number_range(1, 4);
		WAIT_STATE(victim, 5 * paralysis);
		act("`iYour evil gaze strikes terror into $N's mind, leaving $M frozen in horror!``", ch, NULL, victim, TO_CHAR);
		act("`i$n's red eyes send terror through every corner of your mind, leaving you paralyzed with horror!``", ch, NULL, victim, TO_VICT);
		act("`i$n gazes into $N's eyes, leaving $M paralyzed with horror!``", ch, NULL, victim, TO_NOTVICT);
	}
	else
		return;
}*/
