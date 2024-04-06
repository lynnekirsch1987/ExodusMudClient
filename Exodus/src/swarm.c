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

//Swarm.c
//Iblis - Began 2/20/04, everything but swarm_heal done 9/17/04


void swarm_heal(CHAR_DATA *ch, CHAR_DATA *victim, int sn)
{
  char *attack;
  if (sn >= 0 && sn < MAX_SKILL)
    attack = skill_table[sn].noun_damage;
  else attack = "buggy heal";
//  sprintf(buf,"Your $T has no affect on $N.",attack);
  act("Your $T has no affect on $N.",ch,attack,victim,TO_CHAR);
//  sprintf(buf,"$n's $T has no affect on you.",attack);
  act("$n's $T has no affect on you.",ch,attack,victim,TO_VICT);
  act("$n's $T has no affect on $n.",ch,attack,victim,TO_NOTVICT);
}

//Iblis 9/17/04
/*void do_smother(CHAR_DATA *ch, char* argument)
{
   CHAR_DATA *victim;
   if (get_skill(ch,gsn_smother) <= 0)
   {
     send_to_char("Are you sure you want to smother them with kisses?\n\r",ch);
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

   if (victim->level+30 <= ch->level || number_percent() <= get_skill(ch,gsn_smother))
     {
       act("The swarm smothers $N by crawling into $s nose and mouth.",ch,NULL,victim,TO_NOTVICT);
       act("The swarm smothers your by crawling into your nose and mouth.",ch,NULL,victim,TO_VICT);
       act("You smother $N by crawling into $s nose and mouth.",ch,NULL,victim,TO_CHAR);
       raw_kill(victim,ch);
       WAIT_STATE (ch, skill_table[gsn_smother].beats);
     }
   else 
     {
       act("$n's attempt to smother $N fails miserably.",ch,NULL,victim,TO_NOTVICT);
       act("Your attempt to smother $N fails miserably.",ch,NULL,victim,TO_CHAR);
       act("$n's attempt to smother you fails miserably.",ch,NULL,victim,TO_VICT);
       damage (ch, victim, 0, gsn_smother, DAM_NONE, TRUE);
       WAIT_STATE (ch, skill_table[gsn_smother].beats);
     }
}

void do_attack (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;
  one_argument (argument, arg);
  if ((chance = get_skill (ch, gsn_attack)) == 0)
    {
//      send_to_char ("You get your feet dirty.\n\r", ch);
      do_kill(ch,argument);
      return;
    }
  if (arg[0] == '\0')
    {
      victim = ch->fighting;
      if (victim == NULL)
        {
          send_to_char ("But you aren't in combat!\n\r", ch);
          return;
        }
    }

  else if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_AFFECTED (victim, AFF_BLIND))
    {
      act ("$E's already been blinded.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (victim == ch)
    {
      send_to_char ("Very funny.\n\r", ch);
      return;
    }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (is_safe (ch, victim))
    return;
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("But $N is such a good friend!", ch, NULL, victim, TO_CHAR);
      return;
    }


  chance += get_curr_stat (ch, STAT_DEX);
  chance -= 2 * get_curr_stat (victim, STAT_DEX);

  if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
    chance += 10;
  if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
    chance -= 25;

  chance += (ch->level - victim->level) * 2;

  if (is_affected (victim, gsn_mirror))
    chance = -1;

  if (number_percent () < chance)
    {
      AFFECT_DATA af;
      act ("The swarm attacks $n's eyes!", ch, NULL, victim, TO_NOTVICT);
      act ("The swarm attacks your eyes!", ch, NULL, victim, TO_VICT);
      act ("You attack $n's eyes!", ch, NULL, victim, TO_CHAR);
      
      damage (ch, victim, number_range (2, 5), gsn_attack, DAM_NONE, FALSE);
      send_to_char ("You can't see a thing!\n\r", victim);
      WAIT_STATE (ch, skill_table[gsn_attack].beats);
      af.where = TO_AFFECTS;
      af.type = gsn_attack;
      af.level = ch->level;
      af.duration = 0;
      af.location = APPLY_HITROLL;
      af.modifier = 0;
      af.permaff = FALSE;
      af.bitvector = AFF_BLIND;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (victim, &af);
    }

  else
    {
      act ("The swarm attempts to attack $n's eyes!", ch, NULL, victim, TO_NOTVICT);
      act ("The swarm attempts to attack your eyes!", ch, NULL, victim, TO_VICT);
      act ("You attack $n's eyes, but fail to blind $m!", ch, NULL, victim, TO_CHAR);
      damage (ch, victim, 0, gsn_attack, DAM_NONE, TRUE);
      WAIT_STATE (ch, skill_table[gsn_attack].beats);
    }
}
*/
