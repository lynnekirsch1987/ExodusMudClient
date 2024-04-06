/****************************************************************************
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


/****************************************************************************
 *       ROM 2.4 is copyright 1993-1998 Russ Taylor                         *
 *       ROM has been brought to you by the ROM consortium                  *
 *           Russ Taylor (rtaylor@hypercube.org)                            *
 *           Gabrielle Taylor (gtaylor@hypercube.org)                       *
 *           Brian Moore (zump@rom.org)                                     *
 *       By using this code, you have agreed to follow the terms of the     *
 *       ROM license, in the file Rom24/doc/rom.license                     *
 ****************************************************************************/

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
#include "obj_trig.h"

//Iblis - Created 9/29/04 to hold all of the Chaos Jester class skills

/*OBJ_DATA *get_obj_vnum_room args((ROOM_INDEX_DATA* rid, int vnum));
OBJ_DATA *get_obj_vnum_char args((CHAR_DATA* ch, int vnum));
void affect_strip_skills args((CHAR_DATA * ch));
long int level_cost args ((int level));
extern int arearooms args ((CHAR_DATA * ch));
extern int areacount args ((CHAR_DATA * ch));
extern int roomcount args ((CHAR_DATA * ch));*/

int quiver_holding args((OBJ_DATA * obj));
void save_player_box args((OBJ_DATA * pObj));
void show_list_to_char args ((OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing));

bool can_move_char args ((CHAR_DATA * ch, ROOM_INDEX_DATA * to_room, bool follow, bool show_messages));
void one_hit args ((CHAR_DATA * ch, CHAR_DATA * victim, int dt));
void prevent_escape args ((CHAR_DATA * ch, CHAR_DATA * victim));
int check_entangle args ((CHAR_DATA * ch));
bool check_clasp args((CHAR_DATA *ch));
void one_hit_throw args((CHAR_DATA * ch, CHAR_DATA * victim, int dt, OBJ_DATA * wield));
CHAR_DATA *get_gravity_char_room args((ROOM_INDEX_DATA *rid));
void flick_card args((CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, int sn));
void friend_poof args((CHAR_DATA *ch));
	
 bool check_flicker (CHAR_DATA * ch, CHAR_DATA * victim)
{
  int chance;
  if (!IS_AWAKE (victim))
    return FALSE;

  chance = get_skill (victim, gsn_flicker) / 2;

  if (!can_see (victim, ch))
    {
      if (number_percent () < get_skill (victim, gsn_blindfighting))
        check_improve (victim, gsn_blindfighting, TRUE, 2);

      else
        {
          check_improve (victim, gsn_blindfighting, FALSE, 2);
          chance /= 2;
        }
    }
  // Minax 12-20-02 Dodge depends on skill, not level
  //  if (number_percent () >= chance + victim->level - ch->level)
  if (number_percent () >= chance)
    return FALSE;

  act ("$N flickers out of reality and your weapon passes right through $M.", ch, NULL, victim, TO_CHAR);
  act ("You flicker right out of reality as $n swings at you.", ch, NULL, victim, TO_VICT);
  check_improve (victim, gsn_flicker, TRUE, 6);
  return TRUE;
}

//Warp touch skill
void do_warptouch (CHAR_DATA *ch, char *argument)
{
  int chance;
  CHAR_DATA *victim;
  if ((chance = get_skill(ch,gsn_warp_touch)) < 1)
    {
      send_to_char("Warp speed ahead!\n\r",ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char("Warptouch whom?\n\r",ch);
      return;
      /*      victim = ch->fighting;dd

      if (victim == NULL)
        {
          send_to_char ("But you aren't fighting anyone!\n\r", ch);
          return;
	  }*/
    }

  if ((victim = get_char_room (ch, argument)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (is_safe (ch, victim))
    return;
  if (ch == victim)
  {
	  send_to_char("When you think about you, you touch yourself.\n\r",ch);
	  return;
  }
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
  if (ch->position < POS_FIGHTING)
  {
    send_to_char("Get off your ass, doofus!\n\r",ch);
    return;
  }
  if (ch->fighting != NULL)
  {
    send_to_char("No way!  You are still fighting!\n\r",ch);
    return;
  }
  /*if (ch->hit < 15)
    {
      send_to_char("You do not have enough within you to create a spear.\n\r",ch);
      return;
    }
    else ch->hit -= 15;*/
  if (chance > number_percent ())
    {
      send_to_char("Your hand sizzles with tickling energy that makes you giggle to yourself.\n\r",ch);
      act("You slap $N in the face, sending $M to the ground.",ch,NULL,victim,TO_CHAR);
      act("Sizzling energy courses through your spasming body as $n touches you!",ch,NULL,victim,TO_VICT);
      act("Red and yellow light explodes as $n touches $N!",ch,NULL,victim,TO_NOTVICT);
      damage (ch, victim, adjust_damage(ch->damroll+(ch->perm_stat[STAT_STR]*2)), gsn_warp_touch, number_range(1,DAM_WIND), TRUE);
    }
  else
    {
      send_to_char("Your hand sizzles with tickling energy that makes you giggle to yourself.\n\r",ch);
      act("You attempt to slap $N in the face, miss $m completely.",ch,NULL,victim,TO_CHAR);
      act("$n's hand comes out in an attempt to slap you in the face, but $e misses completely.",ch,NULL,victim,TO_VICT);
      act("$n's hand comes out in an attempt to slap $N in the face, but $e misses completely.",ch,NULL,victim,TO_NOTVICT);
      damage (ch, victim, 0, gsn_warp_touch, number_range(1,DAM_WIND), TRUE);
    }
}

void do_madrush (CHAR_DATA *ch, char *argument)
{
  int chance, dam;
  short s1=0;
  CHAR_DATA *victim;
  if ((chance = get_skill(ch,gsn_mad_rush)) < 1)
    {
      send_to_char("You rush forward like mad!\n\r",ch);
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
  if (ch == victim)
  {
    send_to_char("You make a mad rush at yourself, but collapse in a heap.\n\r",ch);
    return;
  }
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
  s1=100*(ch->hit/(ch->max_hit*1.0));
  if (ch->move < s1)
    {
      send_to_char("The only sort of rushing you have the energy to do is towards bed.\n\r",ch);
      return;
    }
  else ch->move -= s1;
  WAIT_STATE (ch, skill_table[gsn_mad_rush].beats);
  if (chance > number_percent ())
    {
      act("Having nothing better to do, you rush headfirst into $N.",ch,NULL,victim,TO_CHAR);
      act("Suddenly $n decides to run into you!",ch,NULL,victim,TO_VICT);
      act("$n runs and crashes headfirst into $N!",ch,NULL,victim,TO_NOTVICT);
      dam = ch->max_hit/10.0;
      dam = 150 + ((10 - ch->hit/dam)*10.0);
      damage (ch, victim, adjust_damage(dam), gsn_mad_rush, DAM_BASH, TRUE);
      return;
    }
 else
   {
     act("Having nothing better to do, you rush headfirst into $N.",ch,NULL,victim,TO_CHAR);
     send_to_char("You fall flat on your face!\n\r",ch);
     act("$n falls flat on $s face!",ch,NULL,victim,TO_ROOM);
     /*
     act("You summon your strength to form a spear made of pure ice and throw it at $N.",ch,NULL,victim,TO_CHAR);
     act("$n seems to get a little shorter as $e forms a spear of ice and throw it at $N.",ch,NULL,victim,TO_NOTVICT);
     act("$n seems to get a little shorter as $e forms a spear of ice and throw it at you.",ch,NULL,victim,TO_VICT);*/
     damage (ch, victim, 0, gsn_mad_rush, DAM_BASH, FALSE);
     ch->position = POS_RESTING;
     if (IS_NPC (ch))
       ch->color = 42;
     return;
   }
}


void do_coma (CHAR_DATA * ch, char *argument)
{
  int chance;
  if ((chance = get_skill(ch,gsn_coma)) < 1)
    {
          send_to_char("You can only get that from a hit on the head.",ch);
          return;
    }
  if (ch->race == PC_RACE_SWARM)
    {
      send_to_char("Huh?\n\r",ch);
      return;
    }
  if (is_affected (ch, skill_lookup ("Dance of Shadows")))
    {
      send_to_char ("Your nightmares keep you from slipping into a coma.\n\r", ch);
      return;
    }
  if (is_affected (ch, gsn_fear))
    {
      send_to_char ("You are WAY too scared to slip into a coma right now!\n\r",ch);
      return;
    }
  if (ch->fighting != NULL)
    {
      send_to_char ("You are WAY too busy to slip into a coma now!\n\r", ch);
      return;
    }
  if (chance > number_percent())
{
  switch (ch->position)
    {
    case POS_SLEEPING:
      send_to_char ("You are already sleeping.\n\r", ch);
      break;
    case POS_MOUNTED:
      send_to_char ("You must dismount before you can slip into a coma.\n\r", ch);
      return;
    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:
      send_to_char ("You slip into a coma.\n\r", ch);
      act ("$n lies down and stops breathing completely.", ch, NULL, NULL, TO_ROOM);
      ch->position = POS_COMA;
      trip_triggers(ch, OBJ_TRIG_CHAR_POS_CHANGE, NULL, NULL, OT_SPEC_NONE);
      ch->on = NULL; //Can't go comatose on any object
      break;
    case POS_FIGHTING:
      send_to_char ("You are WAY too busy to slip into a coma now!\n\r", ch);
      break;
    }
} else
     send_to_char ("One of the voices incessant yapping prevents you from doing so.\n\r",ch);

  return;
}


//Iblis - Vanish skill
void do_vanish (CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;
  int chance;
  if ((chance = get_skill(ch,gsn_vanish)) < 1)
    {
      send_to_char("You close your eyes and the world vanishes!\n\r",ch);
      return;
    }
  if (ch->move < 100)
    {
     send_to_char("You cannot muster the strength to vanish.\n\r",ch);
      return;
    }
  else ch->move -= ch->move*.95;
  if (IS_AFFECTED (ch, AFF_INVISIBLE))
    {
        send_to_char ("Your are already invisible.\n\r", ch);
      return;
    }
  if (chance > number_percent())
    {
      af.where = TO_AFFECTS;
      af.type = gsn_vanish;
      af.level = ch->level;
      af.duration = 40 + ch->perm_stat[STAT_WIS];
      af.location = 0;
      af.modifier = 0;
      af.bitvector = AFF_INVISIBLE;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (ch, &af);
      send_to_char("You slap your hands across your eyes and pretend that you are invisible.\n\r",ch);
      act ("$n crawls into $s navel and vanishes!",ch,NULL,NULL,TO_ROOM);
      WAIT_STATE (ch, skill_table[gsn_vanish].beats);
      return;
    }

    WAIT_STATE (ch, skill_table[gsn_vanish].beats);
    send_to_char("You slap your hands across your eyes, but you can't help peeking so it does no good.\n\r",ch);
}


void do_persuade (CHAR_DATA *ch, char *argument)
{
  int chance, amount,amount2;
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  if ((chance = get_skill(ch,gsn_persuade)) < 1)
    {
      send_to_char("You couldn't persuade a starving man to eat.\n\r",ch);
      return;
    }

  if ((victim = get_char_room (ch, argument)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (is_safe (ch, victim))
    return;
  if (chance > number_percent() && get_curr_stat(ch,STAT_CHA) > get_curr_stat(victim,STAT_CHA))
    {
      act("You persuade $N to give you the money that should have been yours in the first place.",ch,NULL,victim,TO_CHAR);
      if (ch->race != PC_RACE_SIDHE && ch->race != PC_RACE_LITAN)
      {
        switch (number_range(1,8))
	  {
	  case 1: do_say(ch,"Give me your money because: I deserve to be rich");break;
	  case 2: do_say(ch,"Give me your money because: I need them");break;
	  case 3: do_say(ch,"Give me your money because: I'm such a nice person");break;
	  case 4: do_say(ch,"Give me your money because: I'm better than you");break;
	  case 5: do_say(ch,"Give me your money, or else!");break;
	  case 6: do_say(ch,"Some extra money would be nice right about now");break;
	  case 7: do_say(ch,"I'm here to collect the tax for king Athanmere");break;
	  case 8: do_say(ch,"You owe me money. You know, from that time at the place?");break;
	  }
      }
      act("$n persuades $N to give $m money.",ch,NULL,victim,TO_NOTVICT);
      WAIT_STATE(ch,skill_table[gsn_persuade].beats);
      if (get_curr_stat(ch,STAT_CHA) > 10)
        chance = number_range(10,get_curr_stat(ch,STAT_CHA));
      else chance = number_range(get_curr_stat(ch,STAT_CHA),10);
      amount = victim->silver * (chance/100.0);
      ch->silver += amount;
      victim->silver -= amount;
      amount2 = victim->gold * (chance/100.0);
      ch->gold += amount2;
      victim->gold -= amount2;
      sprintf(buf,"You give $n %d gold and %d silver.",amount2,amount);
      act(buf,ch,NULL,victim,TO_VICT);
    }
  else
    {
      act("You try to persuade $N to give you money but you don't do that by slapping them in the face.",ch,NULL,victim,TO_CHAR);
      act("$n slaps you in the face, and you don't take it very well.",ch,NULL,victim,TO_VICT);
      act("$n slaps $N in the face.",ch,NULL,victim,TO_NOTVICT);
      damage(ch,victim,0,gsn_persuade,DAM_UNIQUE,FALSE);
      WAIT_STATE(ch,skill_table[gsn_persuade].beats);
    }
}

//Iblis - Mad Dance skill
void do_maddance (CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;
  int chance;
  //  CHAR_DATA *victim;
  if ((chance = get_skill(ch,gsn_mad_dance)) < 1)
    {
      send_to_char("You dances like a madman, but only succeed in making people think you're mad.\n\r",ch);
      return;
    }
  if (is_affected(ch,skill_lookup("spider body")))
  {
    send_to_char("Spiders can't dance.  Spiders can't walk.  The only thing about them is the way they talk.\n\r",ch);
    return;
  }
  if (ch->mana < ch->max_mana*.25 || ch->move < ch->max_move*((80 - ch->perm_stat[STAT_CON])/100.0) || ch->move < ch->max_move*.50)
    {
      send_to_char("You are too exhausted to dance.\n\r",ch);
      return;
    }
  ch->mana -= ch->max_mana*.25;
  ch->move -= ch->max_move*((80 - ch->perm_stat[STAT_CON])/100.0);
  if (is_affected (ch, gsn_mad_dance))
    {
      send_to_char ("Your are already affected by a mad dance.\n\r", ch);
      return;
    }
  if (chance > number_percent())
    {
      af.where = TO_SKILL;
      af.type = gsn_mad_dance;
      af.level = ch->level;
      af.duration = ch->perm_stat[STAT_DEX];
      af.location = gsn_second_attack;
      af.modifier = 100;
      af.bitvector = 0;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (ch, &af);
      af.location = gsn_third_attack;
      affect_to_char(ch, &af);
      send_to_char("You start dancing and notice the envious stares of everyone else.\n\r",ch);
      act("You look at $n incredulously as $e starts dancing like a madman with two left feet.",ch,NULL,NULL,TO_ROOM);
      WAIT_STATE (ch, skill_table[gsn_mad_dance].beats);
      return;
    }

  WAIT_STATE (ch, skill_table[gsn_mad_dance].beats);
  send_to_char("You start dancing, but after kicking yourself in the ass a few times you stop.\n\r",ch);
  act("You look at $n incredulously as $e starts kicking $mself in the ass.",ch,NULL,NULL,TO_ROOM);
}

//Iblis - Fire Dance skill
void do_firedance (CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;
  int chance;
  //  CHAR_DATA *victim;
  if ((chance = get_skill(ch,gsn_fire_dance)) < 1)
    {
      send_to_char("You dances fast and furiously, but the only thing on fire is people's feet as they run quickly away.\n\r",ch);
      return;
    }
  if (is_affected(ch,skill_lookup("spider body")))
    {
      send_to_char("Spiders can't dance.  Spiders can't walk.  The only thing about them is the way they talk.\n\r",ch);
      return;
    }
  if (ch->mana < ch->max_mana*.30 || ch->move < ch->max_move*((50 - ch->perm_stat[STAT_CON])/100.0) || ch->move < ch->max_move*.50)
    {
      send_to_char("You are too exhausted to dance.\n\r",ch);
      return;
    }
  ch->mana -= ch->max_mana*.30;
  ch->move -= ch->max_move*((50 - ch->perm_stat[STAT_CON])/100.0);
  if (is_affected (ch, gsn_fire_dance))
    {
      send_to_char ("Your are already affected by a fire dance.\n\r", ch);
      return;
    }
  if (chance > number_percent())
    {
      af.where = TO_AFFECTS;
      af.type = gsn_fire_dance;
      af.level = ch->level;
      af.duration = ch->perm_stat[STAT_DEX];
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = 0;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (ch, &af);
      send_to_char("As you start dancing, multicolored flames envelop your hands.\n\r",ch);
      act("As $n starts dancing, $s hands light up with multicolored fire!",ch,NULL,NULL,TO_ROOM);
      WAIT_STATE (ch, skill_table[gsn_fire_dance].beats);
      return;
    }
  WAIT_STATE (ch, skill_table[gsn_fire_dance].beats);
  send_to_char("You start dancing, and your ass catches on fire! You shake your booty until it stops.\n\r",ch);
  act("You look at $n incredulously as smoke starts coming from $s ass.",ch,NULL,NULL,TO_ROOM);
}


void do_escape (CHAR_DATA *ch, char *argument)
{
  ROOM_INDEX_DATA *pRoomIndex;
  int chance, counter=0;
  if ((chance = get_skill(ch,gsn_escape)) < 1 || ch->fighting == NULL)
    {
      send_to_char("You escape into the world you currently exist.\n\r",ch);
      return;
    }
  if (!is_affected(ch,skill_lookup("spider body")))
    {
      send_to_char("You don't have enough legs to escape!\n\r",ch);
      return;
    }
  if (ch->hit < 60 - ch->perm_stat[STAT_CON])
    {
      send_to_char("You can't pull off an escape and live.\n\r",ch);
      return;
    }
  if (ch->position < POS_FIGHTING)
    {
      send_to_char("You need to escape your ass from the ground first.\n\r",ch);
      return;
    }
  if (is_affected(ch,skill_lookup("demonic possession")))
    {
      send_to_char("The demonic being inside you does not want to flee.\n\r",ch);
      return;
    }
  if (!check_web (ch))
    return;
  if (!check_entangle (ch))
    return;
  if (!check_clasp (ch))
    return;
  if (((!IS_NPC (ch->fighting) && ch->fighting->pcdata->prevent_escape) 
			   || IS_SET(ch->fighting->act2,ACT_MONK))
           && number_percent () < (get_skill (ch->fighting, gsn_prevent_escape)* .60))
   {
     prevent_escape (ch->fighting, ch);
     return;
   }
  ch->hit -= 60 - ch->perm_stat[STAT_CON];
  if (chance > number_percent() && !IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL) 
      && !IS_SET (ch->in_room->race_flags, ROOM_NOTRANSPORT) && ch->in_room)
    {
      WAIT_STATE (ch, skill_table[gsn_escape].beats);
      send_to_char("Using your eight legs, you run away from battle faster than anyone ever has before.\n\r",ch);
      act("$n scuttles away on his spider legs, giggling and bumping into people.",ch,NULL,NULL,TO_ROOM);
      pRoomIndex = get_random_room (ch);
      while (IS_SET (pRoomIndex->room_flags, ROOM_ARENA)
	     || IS_SET (pRoomIndex->room_flags, ROOM_SILENT)
	     || IS_SET (pRoomIndex->room_flags, ROOM_NOMAGIC)
         || pRoomIndex->area->construct
         || pRoomIndex->sector_type == SECT_WATER_OCEAN
	     || IS_SET (pRoomIndex->race_flags, ROOM_NOTRANSPORT)
	     || IS_SET (pRoomIndex->race_flags, ROOM_NOTELEPORT)
	     || (IS_SET(pRoomIndex->area->area_flags,AREA_IMP_ONLY) && get_trust(ch) < MAX_LEVEL)
	     || (pRoomIndex->vnum >= ROOM_VNUM_PLAYER_START && pRoomIndex->vnum <= ROOM_VNUM_PLAYER_END)
	     || (pRoomIndex->area != ch->in_room->area)
	     || !can_move_char(ch,pRoomIndex,FALSE,FALSE)
	     ) 
	{
	  pRoomIndex = get_random_room (ch);
	  if (++counter > 10000)
	    {
	      send_to_char("You try to escape through the ground, but alas, it is not bending to your will.\n\r",ch);
	      act("$n spins around in circles a few times then dives furiously at the ground, rising with a strange look on $s face.",ch,NULL,NULL,TO_ROOM);
	      return;
	    }
	}
       stop_fighting (ch, TRUE);
        // Shinowlan 8/30/98 -- Check person fled from to see if we
        // need to clear their pending output.  This was put in
        // to deal with kai, flee, reenter abuse.
	if (ch->fighting)
	{
	  if (!IS_NPC (ch->fighting) && (ch->fighting->wait > 0))
	  {
	    ch->fighting->desc->inbuf[0] = '\0';    //clear queued input
  	  }
	}
      char_from_room(ch);
      char_to_room(ch,pRoomIndex);
    }
  else
    {
      send_to_char("You try to escape through the ground, but alas, it is not bending to your will.\n\r",ch);
      act("$n spins around in circles a few times then dives furiously at the ground, rising with a strange look on $s face.",ch,NULL,NULL,TO_ROOM);
      WAIT_STATE (ch, skill_table[gsn_escape].beats);
    }
}

void death_spasms(CHAR_DATA *ch)
{
  AFFECT_DATA af;
  af.where = TO_AFFECTS;
  af.type = gsn_death_spasms;
  af.level = ch->level;
  af.duration = 1;
  af.modifier = -20;
  af.location = APPLY_AC;
  if (!IS_SET(ch->affected_by,AFF_HASTE))
    af.bitvector = AFF_HASTE;
  else af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (ch, &af);
  af.bitvector = 0;
  af.location = APPLY_DAMROLL;
  af.modifier = get_skill(ch,gsn_death_spasms)/10.0;
  affect_to_char (ch, &af);
  af.location = APPLY_HITROLL;
  affect_to_char (ch, &af);
  send_to_char ("You feel a rush of strength as sweet death draws closer.\n\r", ch);
  act ("$n fights back with renewed energy!",ch,NULL,NULL,TO_ROOM);
}

void do_flick (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim = NULL;
  char flicking_card[MAX_INPUT_LENGTH];
  OBJ_DATA *obj, *tobj;
  short  chance, hands = 0;
  chance = get_skill (ch, gsn_flick);
  if (chance <= 0)
    {
      send_to_char ("You flick everyone off.\n\r", ch);
      return;
    }
  argument = one_argument (argument, flicking_card);
  if (flicking_card[0] == '\0')
    {
      send_to_char ("Flock what card at whom?\n\r", ch);
      return;
    }
  if ((obj = get_obj_carry (ch, flicking_card)) == NULL)
    {
      send_to_char ("You don't seem to be carrying that card.\n\r", ch);
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
  if (obj->item_type != ITEM_CARD)
    {
      act ("$p is not a card.", ch, obj, NULL, TO_CHAR);
      return;
    }
  for (tobj = ch->carrying; tobj; tobj = tobj->next_content)
    {
      if ((tobj->wear_loc == WEAR_WIELD_R)
          || (tobj->wear_loc == WEAR_WIELD_L) || (tobj->wear_loc ==
                                                  WEAR_HOLD)
          || (tobj->wear_loc == WEAR_SHIELD))
        {
          if ((tobj->item_type == ITEM_WEAPON
               && (IS_WEAPON_STAT (tobj, WEAPON_TWO_HANDS) && ch->size < SIZE_LARGE))
              || (tobj->item_type == ITEM_INSTRUMENT
                  && IS_SET (tobj->value[0], A)))
            hands += 2;

          else
            hands++;
        }
    }
  if (hands > 1)
    {
      act ("You have no free hand with which to flick $p.", ch, obj,
           NULL, TO_CHAR);
      return;
    }
  if (IS_SET (ch->in_room->room_flags, ROOM_SAFE))
    {
      send_to_char ("You cannot flick a card in a safe room.\n\r", ch);
      return;
    }
  if (argument[0] == '\0')
    {
      if (ch->fighting != NULL)
        victim = ch->fighting;
      else
        {
          send_to_char ("Flick what at whom?\n\r", ch);
          return;
        }
    }
  else victim = get_char_room (ch, argument);

  if (victim == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (victim->in_room)
    {
      CHAR_DATA *tmp_vict = get_gravity_char_room(victim->in_room);
      if (tmp_vict)
        victim = tmp_vict;
    }
  if (victim == ch || is_affected(ch,gsn_gravity))
    {
      send_to_char ("You can't flick a card at yourself.\n\r", ch);
      return;
    }
  if (is_safe (ch, victim))
    return;
  WAIT_STATE (ch, skill_table[gsn_flick].beats);
  act ("With a snap of $s wrist, $n flicks a card at you.", ch, obj, victim, TO_VICT);
  act ("With a snap of $s wrist, $n flicks a card at $N.", ch, obj, victim, TO_NOTVICT);
  act ("You flick $p at $N.", ch, obj, victim, TO_CHAR);
  flick_card(ch,victim,obj,gsn_flick);
}


void flick_card(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, int sn)
{
  int chance;
  chance = get_skill (ch, gsn_flick);
  obj_from_char (obj);
  if (!IS_NPC(ch))
    save_char_obj (ch);
      if (victim->in_room->sector_type == SECT_UNDERWATER)
	{
	  act ("$p floats into the murky depths of the water.", ch, obj,
	       NULL, TO_CHAR);
	  obj_to_room (obj, victim->in_room);
	  act ("$p floats gently into the water.", victim, obj, NULL, TO_ROOM);
	  return;
	}
      if (ch->in_room->sector_type == SECT_UNDERWATER)
	{
	  act ("$p floats into the murky depths of the water.", ch, obj,
	       NULL, TO_CHAR);
	  obj_to_room (obj, ch->in_room);
	  act ("$p floats gently into the water.", ch, obj, NULL, TO_ROOM);
	  return;
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
      chance += get_curr_stat (ch, STAT_DEX) / 3;
      chance -= get_curr_stat (victim, STAT_DEX) / 5;
      chance += victim->size;

      //chance += GET_AC (victim, AC_PIERCE) / 4;
      if (IS_NPC (victim))
	chance += GET_AC (victim, AC_PIERCE) / 5;

  else
    chance += GET_AC (victim, AC_PIERCE) / 6;
      if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
	chance += 5;
      if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
	chance -= 20;

      //  chance += (ch->level - victim->level);
      if (number_percent () < chance)
	{
	  if (!IS_AWAKE (victim))
	    victim->position = POS_RESTING;
	  if (victim && !IS_NPC(victim))
	    save_char_obj (victim);
 	  if (sn == gsn_flick && number_percent() >= 50)
	    obj_to_char(obj,victim);
	  one_hit_throw (ch, victim, sn, obj);
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
	}
      else 
      {
	act("Your flick missed $N.",ch,NULL,victim,TO_CHAR);
	act("$n's flick misses $N.",ch,NULL,victim,TO_NOTVICT);
	act("$n's flick misses you.",ch,NULL,victim,TO_VICT);
	obj_to_room(obj,ch->in_room);
        if ((get_curr_stat (ch, STAT_DEX) < 21
              || get_curr_stat (ch, STAT_CHA) < 14) && number_percent () <= 25)
        {
           damage (ch, victim, 0, gsn_flick, DAM_NONE, FALSE);
        }
//        update_aggressor (ch, victim);
      }
}

void do_shuffle(CHAR_DATA *ch, char* argument)
{
  OBJ_DATA *obj, *obj_next, *tobj;
  CHAR_DATA *rch;
  int counter=0,max,total=0,counter2=0,hands=0,chance;
  if ((chance = get_skill(ch,gsn_shuffle)) < 1)
    {
      if (ch->sex == SEX_MALE)
	{
	  send_to_char("You reach down and shuffle your twig and berries around.  It feels kind of good!\n\r",ch);
	  act("$n reaches down and shuffles $s twig and berries around.  A smile appears on $s face.",ch,NULL,NULL,TO_ROOM);
	  return;
	}
      //Damn women, I can't think of an interesting message for them
      send_to_char("You reach up and shuffle your hair around, trying to look available to any onlookers.\n\r",ch);
      act("$n reaches up and shuffles $s hair around, trying to look available to any onlookers.",ch,NULL,NULL,TO_ROOM);
      return;
    }

  if (ch->move < 200-(ch->perm_stat[STAT_DEX]*2))
    {
      send_to_char("You're too tired to shuffle your cards right now.\n\r",ch);
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
  for (tobj = ch->carrying; tobj; tobj = tobj->next_content)
    {
      if ((tobj->wear_loc == WEAR_WIELD_R)
          || (tobj->wear_loc == WEAR_WIELD_L) || (tobj->wear_loc ==
                                                  WEAR_HOLD)
          || (tobj->wear_loc == WEAR_SHIELD))
        {
          if ((tobj->item_type == ITEM_WEAPON
               && (IS_WEAPON_STAT (tobj, WEAPON_TWO_HANDS) && ch->size < SIZE_LARGE))
              || (tobj->item_type == ITEM_INSTRUMENT
                  && IS_SET (tobj->value[0], A)))
            hands += 2;

          else
            hands++;
        }
    }
  if (hands > 1)
    {
      act ("You have no free hand with which to shuffle.", ch,NULL,NULL, TO_CHAR);
      return;
    }

  max = number_range(5,10);
  max *= (chance/100.0);
  if (max < 1)
    {
      send_to_char("Alas, your skill at shuffling is so poor you actually shuffle your deck without spilling any cards.\n\r",ch);
      return;
    }
  WAIT_STATE (ch, skill_table[gsn_shuffle].beats);
  send_to_char("You try to shuffle your deck but fail! Cards fly in every direction!\n\r",ch);
  act("`bFace contorted in concentration, $n tries to shuffle $n deck but ends up sending cards flying in every direction!``",ch,NULL,NULL,TO_ROOM);
  for (obj = ch->carrying;obj != NULL && counter <= max;obj = obj_next)
    {
      obj_next = obj->next_content;
      if (obj->item_type != ITEM_CARD)
	continue;
      total = 0;
      for (rch = ch->in_room->people;rch != NULL;rch = rch->next_in_room)
	{
  	  if (rch == ch)
	    continue;
	  if (is_safe_spell(ch,rch,FALSE))
	    continue;
	  if (is_same_group(ch,rch))
   	    continue;
	  ++total;
	}
      total = number_range(1,total);
      counter2 = 0;
      for (rch = ch->in_room->people;rch != NULL;rch = rch->next_in_room)
        {
  	  if (rch == ch)
	    continue;
          if (is_safe_spell(ch,rch,FALSE))
            continue;
 	  if (is_same_group(ch,rch))
            continue;
          if (++counter2 == total)
	    {
	      flick_card(ch,rch,obj,gsn_shuffle);
	      ++counter;
	      break;
	    }
        }
    }
}


void do_luckythrow (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;
  one_argument (argument, arg);
  if ((chance = get_skill (ch, gsn_lucky_throw)) == 0)
    {
      send_to_char ("You throw your arms into the air.  That wasn't very lucky.\n\r", ch);
      return;
    }
  victim = ch->fighting;
  if (victim == NULL)
    {
      send_to_char ("But you aren't in combat!\n\r",ch);
      return;
    }
  if (ch->mana < 80-ch->perm_stat[STAT_DEX])
    {
      send_to_char ("You throw your arms up in despair.  You don't have enough strength to be lucky.\n\r",ch);
      return;
    }
  ch->mana -= 80-ch->perm_stat[STAT_DEX];

  if ((get_eq_char(ch,WEAR_WIELD_R) != NULL && !(get_eq_char(ch,WEAR_WIELD_R)->value[0] == WEAPON_DICE))
        || (get_eq_char(ch,WEAR_WIELD_L) != NULL && !(get_eq_char(ch,WEAR_WIELD_L)->value[0] == WEAPON_DICE)))
  {
	  send_to_char("You must be wielding dice for this to work.\n\r",ch);
	  return;
  }

  WAIT_STATE(ch,skill_table[gsn_lucky_throw].beats);
  if (chance > number_percent())
    {
      one_hit(ch,victim,-1);
      one_hit(ch,victim,-1);
      one_hit(ch,victim,-1);
    }
  else send_to_char("Your luck SUCKS!\n\r",ch);

}

void do_friend(CHAR_DATA *ch, char* argument)
{
  CHAR_DATA *mob, *mch;
    char buf[MAX_STRING_LENGTH];
 
  if (IS_NPC(ch))
    return;
 

	for (mch = char_list;mch != NULL;mch = mch->next)
	{
	  if (IS_NPC(mch))
	    continue;
	  if (!mch->in_room)
  	    continue;
		if (IS_IMMORTAL(mch))
		break;
		if (mch->Class == PC_CLASS_CHAOS_JESTER)
		break;
		
		 if (ch->Class != PC_CLASS_CHAOS_JESTER)
			{
			  send_to_char("You attempt to will a friend into existence, but you can't seem to do it.\n\r",ch);
			  return;
			}
			

			
		  if (ch->in_room->area->nplayer > 1)
			{
			  send_to_char("Does your imaginary friend even really exist? Others do not seem to think so.\n\r",ch);
			  return;
			}
			 }
  if (ch->pcdata->familiar)
    {
      familiar_poof(ch);
      return;
    }
  mob = create_mobile(get_mob_index(MOB_VNUM_IMAGINARY_FRIEND));
  mob->perm_stat[STAT_STR] = ch->perm_stat[STAT_STR]-3;
  mob->perm_stat[STAT_INT] = ch->perm_stat[STAT_INT]-3;
  mob->perm_stat[STAT_WIS] = ch->perm_stat[STAT_WIS]-3;
  mob->perm_stat[STAT_DEX] = ch->perm_stat[STAT_DEX]-3;
  mob->perm_stat[STAT_CON] = ch->perm_stat[STAT_CON]-3;
  mob->perm_stat[STAT_CHA] = ch->perm_stat[STAT_CHA]-3;
  mob->mod_stat[STAT_STR] = 0;
  mob->mod_stat[STAT_INT] = 0;
  mob->mod_stat[STAT_WIS] = 0;
  mob->mod_stat[STAT_DEX] = 0;
  mob->mod_stat[STAT_CON] = 0;
  mob->mod_stat[STAT_CHA] = 0;
  act("Your imaginary friend appears, honks your nose, and then giggles a little.",ch,NULL,NULL,TO_ALL);
  sprintf (buf, "%s %s", mob->name, ch->pcdata->familiar_name);
  free_string (mob->name);
  mob->name = str_dup (buf);
  if (str_cmp(ch->pcdata->familiar_name,""))
    sprintf (buf, "Your friend %s", ch->pcdata->familiar_name);
  else sprintf (buf, "Your friend");
  free_string (mob->short_descr);
  mob->short_descr = str_dup(buf);
  mob->exp = ch->pcdata->familiar_exp;
  mob->max_hit = ch->pcdata->familiar_max_hit;
  mob->hit = ch->pcdata->familiar_max_hit/2;
  mob->max_mana = ch->pcdata->familiar_max_mana;
  mob->mana = ch->pcdata->familiar_max_mana;
  mob->max_move = ch->pcdata->familiar_max_move;
  mob->move = ch->pcdata->familiar_max_move;
  mob->level = ch->pcdata->familiar_level;
  //  sprintf (buf, "%s %s", mob->name, ch->pcdata->familiar_name);
  // free_string (mob->name);
  // mob->name = str_dup (buf);
  mob->afkmsg = str_dup (ch->name);
  SET_BIT(mob->act2,ACT_FAMILIAR);
  SET_BIT(mob->affected_by,AFF_CHARM);
  if (mob->level >= 90)
	  mob->number_of_attacks = 3;
  else if (mob->level >= 45)
	  mob->number_of_attacks = 2;
  else mob->number_of_attacks = 1;
  mob->hitroll = mob->level/10 * 10;
  mob->damroll = mob->level/10 * 7;
  char_to_room(mob,ch->in_room);
  add_follower (mob, ch);
  ch->pcdata->familiar = mob;
  mob->leader = ch;
}

void familiar_poof(CHAR_DATA *ch)
{
  OBJ_DATA *obj,*obj_next;
  if (!ch->pcdata->familiar)
  {
	  log_string("Familiar poof called on with no familiar -> ");
          log_string(ch->name);
	  return;
  }
  if (!ch->pcdata->familiar->in_room)
  {
	  log_string("Familiar poof called on familiar not in room for -> ");
          log_string(ch->name);
	  return;
  }
      for (obj = ch->pcdata->familiar->carrying; obj; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  unequip_char (ch->pcdata->familiar, obj);
	  obj_from_char (obj);
	  obj_to_room (obj,ch->pcdata->familiar->in_room);
	  act ("$p falls to the ground", ch->pcdata->familiar, obj, NULL, TO_ALL);
	}
  
  extract_char(ch->pcdata->familiar,TRUE);
  ch->pcdata->familiar = NULL;
}

void do_box(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *box=NULL, *obj=NULL;
  char arg[MAX_STRING_LENGTH];
  if (ch->Class != PC_CLASS_CHAOS_JESTER || IS_NPC(ch))
    {
      send_to_char("Huh?\n\r",ch);
      return;
  }
  if (argument[0] == '\0')
    {
      send_to_char("Syntax: box look\n\r            get/put OBJ\n\r",ch);
      return;
    }
  argument = one_argument(argument,arg);
  box = get_obj_vnum_room(get_room_index(ROOM_VNUM_SWITCHED_LIMBO),OBJ_VNUM_CJ_SHARED_BOX);
  if (box == NULL)
    {
      send_to_char("No box for you!\n\r",ch);
      return;
    }
  if (!str_cmp(arg,"get"))
    {
      obj = get_obj_list (ch, argument, box->contains);
      if (obj == NULL)
	{
	  act ("You see nothing like that in the $p.", ch, box, NULL,
	       TO_CHAR);
	  return;
	}
      get_obj (ch, obj, box);
      trap_check(ch,"object",NULL,box);
      //Iblis - 3/11/04 - Antiduping Code
      if (!IS_NPC(ch))
	save_char_obj (ch);
      return;
    }
  if (!str_cmp(arg,"put"))
    {
      if ((obj = get_obj_carry (ch, argument)) == NULL)
        {
          send_to_char ("You do not have that item.\n\r", ch);
          return;
        }
      if (obj == box)
        {
          send_to_char ("You can't fold it into itself.\n\r", ch);
          return;
        }
      // Adeon 7/18/03 -- cant put nosave stuff in containers
      if (IS_OBJ_STAT (obj, ITEM_NOSAVE))
        {
          send_to_char ("Strong magic forces prevent you...\n\r", ch);
          return;
        }
      if (!can_drop_obj (ch, obj))
        {
          send_to_char ("You can't let go of it.\n\r", ch);
          return;
        }
      if (WEIGHT_MULT (obj) != 100 || obj->item_type == ITEM_QUIVER || WEIGHT_MULTP (obj) != 100)
        {
          send_to_char
            ("You have a feeling that would be a bad idea.\n\r", ch);
          return;
        }
      if (box->item_type == ITEM_QUIVER)
        {
          if (obj->item_type != ITEM_WEAPON || obj->value[0] != WEAPON_ARROW)
            {
              send_to_char ("Quivers can only hold arrows.\n\r", ch);
              return;
            }
          if (box->value[0] <= quiver_holding (box))
            {
              send_to_char ("That quiver is full.\n\r", ch);
              return;
            }
        }
      if (box->item_type != ITEM_CLAN_DONATION
          && box->item_type != ITEM_NEWCLANS_DBOX
          && box->item_type != ITEM_PLAYER_DONATION
          && box->item_type != ITEM_QUIVER)
        {
          if (get_obj_weight (obj) +
              get_true_weight (box) > (box->value[0] *
                                             10) || get_obj_weight (obj)
              > (box->value[3] * 10))
            {
              send_to_char ("It won't fit.\n\r", ch);
              return;
            }
        }

      obj_from_char (obj);
      obj_to_obj (obj, box);
      if (box != NULL && box->item_type == ITEM_PLAYER_DONATION)
        save_player_box (box);
      //Iblis - 3/11/04 - Antiduping Code
      if (!IS_NPC(ch))
        save_char_obj (ch);
      act ("$n puts $p in $P.", ch, obj, box, TO_ROOM);
      act ("You put $p in $P.", ch, obj, box, TO_CHAR);
    
      trip_triggers(ch, OBJ_TRIG_PUT, obj, NULL, OT_SPEC_NONE);
      trap_check(ch,"object",NULL,box);
      return;
    }
  if (!str_cmp(arg,"look"))
    {
      act ("$p holds:", ch, box, NULL, TO_CHAR);
      show_list_to_char (box->contains, ch, TRUE, TRUE);
      return;
    }
  do_box(ch,"");
}

void do_friendxp (CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch) || ch->Class != PC_CLASS_CHAOS_JESTER)
  {
    send_to_char("Huh?\n\r",ch);	  
    return;
  }
      // we have a mob thing
      if (ch->pcdata->all_xp_familiar)
        {
          send_to_char ("Your familiar will now only get 50% of the xp.\n\r", ch);
	  ch->pcdata->all_xp_familiar = FALSE;
          //REMOVE_BIT (ch->imm_flags, IMM_SUMMON);
        }

      else
        {
          send_to_char ("Your familiar will now get 100% of the xp.\n\r", ch);
	  ch->pcdata->all_xp_familiar = TRUE;
	  //          SET_BIT (ch->imm_flags, IMM_SUMMON);
        }
    
}

void do_revert (CHAR_DATA * ch, char *argument) {
    if ((!IS_NPC(ch) && (ch->Class != PC_CLASS_CHAOS_JESTER) && (ch->Class != PC_CLASS_DRUID) && !IS_IMMORTAL(ch))) {
        send_to_char("Huh?\n\r",ch);
        return;
    }
    
    if (!IS_NPC(ch) || !IS_SET(ch->act2, ACT_PUPPET)) {
        send_to_char("You aren't currently switched to another form.\n\r",ch);
        return; 
    } else {
        ch->ticks_remaining = 0;
        if ((ch->pIndexData->vnum == MOB_VNUM_FROG) || (ch->pIndexData->vnum == MOB_VNUM_WARPED_CAT) || (ch->pIndexData->vnum == MOB_VNUM_WARPED_OWL)) {
            act("$n dissappears in a puff of purple smoke, but another being appears in $s place.", ch, NULL, NULL, TO_ROOM);
            send_to_char("You choose to return to your former self.\n\r",ch);
            
            if (ch != NULL) {
                OBJ_DATA *obj,*obj_next;
                for (obj = ch->carrying; obj; obj = obj_next) {
                    obj_next = obj->next_content;
                    unequip_char(ch, obj);
                    obj_from_char(obj);
                        obj_to_room (obj, ch->in_room);
                        act("$p falls to the ground", ch, obj, NULL, TO_ALL);
                    }
            }
                
            if (ch->desc && ch->desc->original) {
                char_from_room(ch->desc->original);
                char_to_room(ch->desc->original, ch->in_room);
            }
        } else if (is_shapechanged_druid(ch)) {
            shapechange_return(ch, 1);
        }        

        extract_char(ch, TRUE);    
    }
}
