#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "obj_trig.h"
DECLARE_DO_FUN (do_scan);
extern char *target_name;
extern bool exploration_tracking;
extern int getbit args ((char *explored, int index));
extern bool nogate;
extern bool ignore_save;
extern bool crit_strike_possible;
extern bool battle_royale;
extern bool costs_mana;
extern const char *dir_desc[];

void do_look args((CHAR_DATA * ch, char *argument));
void one_hit args ((CHAR_DATA * ch, CHAR_DATA * victim, int dt));
void do_split args((CHAR_DATA * ch, char *argument));
bool has_quiver args((CHAR_DATA * ch));
void do_zecho args((CHAR_DATA * ch, char *argument));
void do_return args((CHAR_DATA * ch, char *argument));
int summon_goes_wrong args((CHAR_DATA * ch));
void summon_nasty args((CHAR_DATA * ch));
bool can_move_char args ((CHAR_DATA * ch, ROOM_INDEX_DATA * to_room, bool follow, bool show_messages));

void chant_seizure args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
void chant_wind_of_death args((int sn, int level, CHAR_DATA * ch, void *vo, int target));

void spell_farsight (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  if (IS_AFFECTED (ch, AFF_BLIND))
    {
      send_to_char ("Maybe it would help if you could see?\n\r", ch);
      return;
    }
  do_scan (ch, target_name);
}

void spell_portal (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *portal, *stone;
  stone = get_eq_char (ch, WEAR_HOLD);
  if (nogate || (!IS_NPC(ch) && ch->pcdata->nogate))
  {
	  send_to_char("The gods are not allowing that at the moment.\n\r",ch);
	  return;
  }
  if (!IS_IMMORTAL (ch))
    {
      if (stone == NULL
	  || (stone->item_type != ITEM_WARP_STONE
	      && stone->pIndexData->vnum != OBJ_VNUM_RUNE_CONV))
	{
	  send_to_char
	    ("You lack the proper component for this spell.\n\r", ch);
	  return;
	}
    }
  if (!IS_NPC (ch))
    if ((time (NULL) - ch->pcdata->last_fight < 60))
      {
	send_to_char
	  ("You cannot muster the concentration to travel the realms in that way right now.\n\r",
	   ch);
	return;
      }
  if (IS_SET (ch->in_room->race_flags, ROOM_NOTRANSPORT))
    {
      send_to_char
	("You cannot leave this realm via ordinary magic.\n\r", ch);
      return;
    }
  if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
      act ("You draw upon the power of $p.", ch, stone, NULL, TO_CHAR);
      act ("It flares brightly and vanishes!", ch, stone, NULL, TO_CHAR);
      extract_obj (stone);
    }
  if (stone != NULL && stone->pIndexData->vnum == OBJ_VNUM_RUNE_CONV)
    act ("You draw upon the power of $p.", ch, stone, NULL, TO_CHAR);
  if ((victim = get_char_world (ch, target_name)) == NULL
      || victim->in_room == NULL || !can_see_room (ch, victim->in_room)
      || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
      || IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL)
      || (!IS_NPC (victim) && victim->level >= LEVEL_IMMORTAL)
      || (victim->in_room->area->construct)
      || IS_SET(victim->in_room->area->area_flags,AREA_SOLO)
      || IS_SET(victim->in_room->room_flags2,ROOM_SOLO)
      || (IS_SET(victim->in_room->area->area_flags,AREA_IMP_ONLY) && get_trust(victim) < MAX_LEVEL)
      || (IS_NPC (victim) && IS_SET (victim->imm_flags, IMM_SUMMON)))
    {
      portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
      portal->timer = 1 + level / 25;
      portal->value[3] = PORTAL_FUCKED;
      act ("$n makes arcane gestures and a glowing portal appears!",
	   ch, NULL, NULL, TO_ROOM);
      send_to_char ("A portal opens but seems unstable.\n\r", ch);
      obj_to_room (portal, ch->in_room);
      return;
    }

    if (exploration_tracking)
       if (getbit (ch->pcdata->explored, victim->in_room->vnum) == 0)
         {
           send_to_char ("Your know nothing about the location you are trying to reach\n\r", ch);
           return;         
         }


  if (victim->in_room == ch->in_room)
    {
      send_to_char ("Are you blind?\n\r", ch);
      return;
    }
  if (room_is_private (victim->in_room))
    {
      send_to_char ("A portal fluxuates but never solidifies.\n\r", ch);
      return;
    }
  if (IS_SET (victim->in_room->race_flags, ROOM_NOTRANSPORT))
    {
      send_to_char ("You cannot travel there via ordinary magic.\n\r", ch);
      return;
    }
  portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
  portal->timer = 1 + level / 25;
  portal->value[3] = victim->in_room->vnum;
  act ("$p rises up from the ground.", ch, portal, NULL, TO_ROOM);
  act ("$p rises up before you.", ch, portal, NULL, TO_CHAR);

// Minax 6-30-02 Groundwork is in place for people affected by 'detect magic'
//   get a message when someone opens a portal to them.
  if (IS_AFFECTED (victim, AFF_DETECT_MAGIC))
    {
      act ("You sense a magical doorway opening from far away.", victim,
	   portal, NULL, TO_CHAR);
      //return;
    }
  obj_to_room (portal, ch->in_room);
  return;
}

//Iblis - Ranger spider vine spell
void spell_spider_vine (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj;
  AFFECT_DATA af;
  if (is_dueling (ch))
  {
	  send_to_char("Please do not cast this while in the normal Arena.\n\r",ch);
	  return;
  }
  if (!is_affected (ch, gsn_bark_skin))
  {
    send_to_char("Your skin is not bark, thus the spell fails miserably.\n\r",ch);
    return;
  }
  if (ch->hit <= 10)
  {
    send_to_char("You don't now have enough life for this spell.\n\r",ch);
    return;
  }
  else ch->hit -= 10;
  if (!(ch->in_room->sector_type == SECT_HILLS
        || ch->in_room->sector_type == SECT_FIELD
        || ch->in_room->sector_type == SECT_SWAMP
        || ch->in_room->sector_type == SECT_FOREST
        || ch->in_room->sector_type == SECT_GRASSLAND))
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
    }
  
  send_to_char("`cTendrils of vine bore into your arms and become a part of you.``\n\r",ch);
  act("`cVines crawl up $n's body and attach to $s arms.``",ch,NULL,NULL,TO_ROOM);
  obj = get_eq_char (ch, WEAR_WRIST_L);
  if (obj != NULL)
    unequip_char(ch,obj);
  obj = get_eq_char (ch, WEAR_WRIST_R);
  if (obj != NULL)
    unequip_char(ch,obj);
  obj = get_eq_char (ch, WEAR_ARMS);
  if (obj != NULL)
    unequip_char(ch,obj);
  obj = create_object (get_obj_index (OBJ_VNUM_SPIDER_VINE_ARMS), 0);
  obj_to_char(obj,ch);
  equip_char (ch, obj, WEAR_ARMS);
  obj = create_object (get_obj_index (OBJ_VNUM_SPIDER_VINE_WRISTS), 0);
  obj_to_char(obj,ch);
  equip_char (ch, obj, WEAR_WRIST_L);
  obj = create_object (get_obj_index (OBJ_VNUM_SPIDER_VINE_WRISTS), 0);
  obj_to_char(obj,ch);
  equip_char (ch, obj, WEAR_WRIST_R);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 10+((level-62)/2.8);
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (ch, &af);
}

//Iblis - Lich despoil skill
void do_despoil(CHAR_DATA *ch, char *argument)
{
  int chance=0;
  AFFECT_DATA af;
  if (IS_NPC(ch) || ch->in_room == NULL)
    return;
  if ((chance = get_skill (ch, gsn_despoil)) < 1)
    {
      send_to_char ("You know nothing about despoiling.\n\r", ch);
      return;
    }
  if (ch->il_ticks > 0)
    {
      send_to_char ("You cannot do that again yet.\n\r",ch);
      return;
    }
  if (ch->mana < 5)
    {
      send_to_char("You do not have enough mana.\n\r",ch);
      return;
    }
  ch->mana -= 5;
  if (IS_AFFECTED (ch, AFF_PROTECT_GOOD)
      || IS_AFFECTED (ch, AFF_PROTECT_EVIL))
    {
        send_to_char ("You are already protected.\n\r", ch);
      return;
    }
  WAIT_STATE (ch, skill_table[gsn_despoil].beats);
  if (chance > number_percent())
  {
  af.where = TO_AFFECTS;
  af.type = gsn_despoil;
  af.level = ch->level;
  af.duration = dice(5,ch->level/8);
  af.location = APPLY_SAVING_SPELL;
  af.modifier = (ch->level/15 * -1);
  af.bitvector = AFF_PROTECT_GOOD;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (ch, &af);
  send_to_char ("You feel aligned with darkness.\n\r", ch);
  ch->il_ticks = 24;
  return;
  }
  else
  {
	  send_to_char("You failed to despoil yourself.\n\r",ch);
	  return;
  }
}

//Iblis - Lich contaminate skill
void do_contaminate(CHAR_DATA *ch, char *argument)
{
  int chance=0;
  AFFECT_DATA af;
  CHAR_DATA *victim;
  if (IS_NPC(ch))
    return;
  if ((chance = get_skill (ch, gsn_contaminate)) < 1)
    {
      send_to_char ("You contaminate your pinky finger, but it does no good.\n\r", ch);
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
  if (ch == victim)
    {
      send_to_char("Contaminating yourself could be bad for your health.\n\r",ch);
      return;
    }
  if (is_safe (ch, victim))
    return;
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
      return;
    }
     

  if (ch->mana < 30)
    {
      send_to_char("You do not have enough mana.\n\r",ch);
      return;
    }
  ch->mana -= 30;


  if (is_affected(victim,gsn_contaminate))
    {
          act ("$N is already contaminated.", ch, NULL, victim, TO_CHAR);
      return;
    }

  WAIT_STATE (ch, skill_table[gsn_contaminate].beats);
  if (chance > number_percent())
    {
      af.where = TO_AFFECTS;
      af.type = gsn_contaminate;
      af.level = ch->level;
      af.duration = number_range((ch->level/9.0)-2, (ch->level/9.0)+2);
      if (af.duration < 1)
        af.duration = 1;
      af.location = APPLY_AC;
      af.modifier = ch->level/10.0;
      af.bitvector = 0;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup (ch->name);
      affect_to_char (victim, &af);
      af.location = APPLY_HITROLL;
      af.modifier = 0 - ch->level/10.0;
      affect_to_char (victim, &af);
      victim->contaminator = ch;
      act("$n contaminates you with $s impurities.",ch,NULL,victim,TO_VICT);
      act("You contaminate $N with your impurities.",ch,NULL,victim,TO_CHAR);
      act("$n contaminates $N with $s impurities.",ch,NULL,victim,TO_NOTVICT);
      damage(ch,victim,0,gsn_contaminate,DAM_BASH,FALSE);
      return;
    }
  else
    {
      send_to_char("You failed to contaminate them.\n\r",ch);
      damage(ch,victim,0,gsn_contaminate,DAM_BASH,FALSE);
      return;
    }

}

//Iblis - Litan flame command, to set themselves on fire (giving them flameshield, changing their short
//desc, blinding people looking at them, and burning some of their shit)
void do_flame(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj, *obj_next;
  if (IS_NPC(ch) || ch->race != PC_RACE_LITAN)
	  return;
  if (ch->pcdata->flaming)
  {
	  ch->pcdata->flaming = 0;
	  if (!((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL
	          && obj->item_type == ITEM_LIGHT
	          && obj->value[2] != 0 && ch->in_room != NULL))
	     ch->in_room->light--;
	  
	  if (is_affected(ch,gsn_fireshield))
    	    affect_strip(ch,gsn_fireshield);
	  send_to_char("The flame surrounding you dies down.\n\r",ch);
	  return;
  }
  else 
  {
	  if (IS_OUTSIDE(ch) && (weather_info.sky == SKY_RAINING ||
			  weather_info.sky == SKY_LIGHTNING))
	  {
	    send_to_char("The rain prevents your scales from igniting.\n\r",ch);
	    return;
	  }

	  if (ch->in_room->sector_type == SECT_SWAMP 
			  || ch->in_room->sector_type == SECT_WATER_OCEAN
			  || ch->in_room->sector_type == SECT_WATER_NOSWIM
			  || ch->in_room->sector_type == SECT_WATER_SWIM
			  || ch->in_room->sector_type == SECT_UNDERWATER)
	  {
	    send_to_char("The water here prevents your scales from igniting.\n\r",ch);
	    return;
	  }

	  if (IS_SET(ch->in_room->room_flags2,ROOM_SNOW))
	  {
	    send_to_char("The snow here causes your flame to fizzle out instantly.\n\r",ch);
	    return;
	  }
	         affect_strip (ch, gsn_sneak);
		   REMOVE_BIT (ch->affected_by, AFF_HIDE);
		     REMOVE_BIT (ch->affected_by, AFF_CAMOUFLAGE);
		         REMOVE_BIT (ch->affected_by, AFF_SNEAK);
			   
	  ch->pcdata->flaming = 1;
	  if (is_affected(ch,gsn_fireshield))
	    affect_strip(ch,gsn_fireshield);
	  if (!is_affected(ch,gsn_fireshield))
	  {
	    AFFECT_DATA af;
	    af.where = TO_AFFECTS;
	    af.type = gsn_fireshield;
	    af.level = ch->level;
	    af.duration = 24;
	    af.modifier = -15;
	    af.location = APPLY_AC;
	    af.bitvector = 0;
	    af.permaff = FALSE;
	    af.composition = FALSE;
	    af.comp_name = str_dup ("");
	    affect_to_char (ch, &af);
	  }
	 if (!((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL
	       && obj->item_type == ITEM_LIGHT
               && obj->value[2] != 0 && ch->in_room != NULL))
	    ch->in_room->light++;
	  send_to_char("The slimy secrete covering your scales bursts into flames!\n\r",ch);
	  for (obj = ch->carrying;obj != NULL;obj = obj_next)
	    {
	      OBJ_DATA *t_obj, *n_obj;
	      char *msg;
	      obj_next = obj->next_content;
	      if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF)
		  || IS_OBJ_STAT (obj, ITEM_NOPURGE))
	      continue;
	      switch (obj->item_type)
		{
		case ITEM_CONTAINER:
		case ITEM_QUIVER:
		  msg = "$p ignites and burns!";
		  break;
		case ITEM_POTION:
		  msg = "$p bubbles and boils!";
		  break;
		case ITEM_SCROLL:
		  msg = "$p crackles and burns!";
		  break;
		case ITEM_STAFF:
		  msg = "$p smokes and chars!";
	      break;
		case ITEM_WAND:
		  msg = "$p sparks and sputters!";
	      break;
		case ITEM_FOOD:
		  msg = "$p blackens and crisps!";
		  break;
		case ITEM_PILL:
		  msg = "$p melts and drips!";
		  break;
		case ITEM_PARCHMENT:
		  msg = "$p is burned into waste.";
		  break;
		default :
		  continue;
		}
	      if (obj->carried_by != NULL)
		act (msg, obj->carried_by, obj, NULL, TO_ALL);
	      
	      else if (obj->in_room != NULL && obj->in_room->people != NULL)
		act (msg, obj->in_room->people, obj, NULL, TO_ALL);
	      if (obj->contains)
		{
		  
		  for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
		    {
		      n_obj = t_obj->next_content;
		      obj_from_obj (t_obj);
		      act("$p falls to the floor.",ch,t_obj,NULL,TO_ALL);
		      if (obj->in_room != NULL)
			obj_to_room (t_obj, obj->in_room);
		      
		      else if (obj->carried_by != NULL)
			obj_to_room (t_obj, obj->carried_by->in_room);
		      
		      else
			{
			  extract_obj (t_obj);
			  continue;
			}
		    }
		}
	      extract_obj (obj);
	    }
     if (is_affected (ch, gsn_entangle))
       {
         send_to_char ("The plants holding you break and fall away.\n\r", ch);
         act ("The plants holding $n break and fall away.", ch, NULL, NULL, TO_ROOM);
         affect_strip (ch, skill_lookup ("entangle"));
       }
  }
	  return;
}

//Iblis - Litan flare skill	
void do_flare(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;
  int chance;
  if (IS_NPC(ch))
    return;
  if ((chance = get_skill (ch, gsn_flare)) < 1)
    {
      send_to_char ("You flare your nostrils!\n\r", ch);
      act("$n flares $s nostrils!\n\r",ch,NULL,NULL,TO_ROOM);
      return;
    }
      victim = ch->fighting;
      if (victim == NULL)
        {
          send_to_char ("But you aren't fighting anyone!\n\r", ch);
          return;
        }
  if (is_safe (ch, victim))
    return;
  if (ch->pcdata->flaming == FALSE)
    {
      send_to_char("You have no magical flame surrounding you to flare!\n\r",ch);
      return;
  }
  if (ch->mana < 50)
  {
    send_to_char("You do not have enough mana.\n\r",ch);
    return;
  }
  ch->mana -= 50;
  WAIT_STATE(ch,skill_table[gsn_flare].beats);
  if (number_percent () < chance)
    {
      if (IS_AFFECTED (victim, AFF_BLIND)
	  || IS_SET(victim->imm_flags, IMM_BLIND)
	  || victim->race == PC_RACE_LITAN)
	{
	  send_to_char ("You failed.\n\r", ch);
	  return;
	}
      if (victim->saving_throw > 10)
	      chance = 50;
      else chance = victim->saving_throw*5;
      if (ignore_save)
        chance = 0;
      if (number_percent() < chance)
      {
        send_to_char ("You failed.\n\r", ch);
        return;
       }
      
      act("You have been blinded by the flaring of $n's flame!", ch,NULL,victim
           ,TO_ROOM);
      af.where = TO_AFFECTS;
      af.type = gsn_flare;
      af.level = ch->level;
      af.location = APPLY_HITROLL;
      af.modifier = -4;
      af.duration = 0;
      af.bitvector = AFF_BLIND;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (victim, &af);
      act ("$N fumbles awkwardly as a dazzlingly bright flame flares in front of them.",ch, NULL, victim, TO_NOTVICT);
      act ("$N fumbles awkwardly as the magical flame surrounding you flares brightly.",ch,NULL,victim,TO_CHAR);
    }
  else
    {
      send_to_char("You attempt to induce the magical flame surrounding you to flare, but you fail miserably.\n\r",ch);
    }
}

//Iblis - Nerix lightningbreath skill
void do_lightningbreath (CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  int dam, chance;

  if ((chance = get_skill(ch,gsn_lightningbreath)) < 1)
    {
      send_to_char("You cannot breathe lightning.\n\r",ch);
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

  if (ch->mana < 100)
    {
      send_to_char ("You do not have enough mana.\n\r",ch);
      return;
    }
  ch->mana -= 100;

  /* BEGIN UNDERWATER */
  if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
      send_to_char ("Your lightning breath is useless underwater.\n\r", ch);
      return;
    }

  /* END UNDERWATER */
  act ("$n breathes a bolt of lightning at $N.", ch, NULL, victim,
       TO_NOTVICT);
  act ("$n breathes a bolt of lightning at you!", ch, NULL, victim, TO_VICT);
  act ("You breathe a bolt of lightning at $N.", ch, NULL, victim, TO_CHAR);
  dam = number_range(ch->level,ch->level*2.5);
  dam = adjust_damage(dam);
  if (saves_spell (ch->level, victim, DAM_LIGHTNING))
    {
      shock_effect (victim, ch->level / 2, dam / 4, TARGET_CHAR);
      damage_old (ch, victim, dam / 2, gsn_lightningbreath, DAM_LIGHTNING, TRUE);
    }

  else
    {
      shock_effect (victim, ch->level, dam, TARGET_CHAR);
      damage_old (ch, victim, dam, gsn_lightningbreath, DAM_LIGHTNING, TRUE);
    }
  WAIT_STATE (ch, skill_table[gsn_lightningbreath].beats);
}

//Iblis - Nidae bubble skill
void do_bubble (CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;
  int chance;
  CHAR_DATA *victim;
  if ((chance = get_skill(ch,gsn_bubble)) < 1)
  {
    send_to_char("You concentrate hard, but can't even muster a spit bubble.\n\r",ch);
    return;
  }
  if (argument[0] == '\0')
  {
	  send_to_char ("Surround who with a bubble?\n\r",ch);
	  return;
  }
  if ((victim = get_char_room (ch, argument)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (!(ch->in_room && (ch->in_room->sector_type == SECT_UNDERWATER || ch->in_room->sector_type == SECT_WATER_OCEAN)))
  {
	  send_to_char("You must be in the water to do that.\n\r",ch);
	  return;
  }
  if (ch->mana < skill_table[gsn_bubble].min_mana)
  {
	  send_to_char("You cannot muster the power to draw a bubble at the moment.\n\r",ch);
	  return;
  }
  else ch->mana -= skill_table[gsn_bubble].min_mana;
  if (IS_AFFECTED (ch, AFF_AQUA_BREATHE) || IS_SET(ch->imm_flags,IMM_DROWNING))
    {
      if (victim == ch)
        send_to_char ("Your lungs already take the form of gills.\n\r", ch);

      else
        act ("$N already swims like a fish.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (chance > number_percent())
  {
    af.where = TO_AFFECTS;
    af.type = gsn_bubble;
    af.level = ch->level;
    af.duration = 8 + dice(1,4);
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_AQUA_BREATHE;
    af.permaff = FALSE;
    af.composition = FALSE;
    af.comp_name = str_dup ("");
    affect_to_char (victim, &af);
    act ("You raise your hands and draw a circle around $N, seperating the air from the water and form a bubble around $M.",ch,NULL,victim,TO_CHAR);
    act ("As $n raises $s hands and draws a circle around your body, the area around you suddenly seems drier and you realize that you can breathe perfectly fine.",ch,NULL,victim,TO_VICT);
    act ("As $n raises $s hands, drawing a bubble around $N, you notice that $E is suddenly surrounded by a bubble.",ch,NULL,victim,TO_NOTVICT);
    WAIT_STATE (ch, skill_table[gsn_bubble].beats);
    return;
  }
  act ("$n waves his finger in the air, but it does no good.",ch,NULL,victim,TO_ROOM);
  send_to_char("You wave your finger in the air, but it does no good.",ch);
  WAIT_STATE (ch, skill_table[gsn_bubble].beats);
}

//Iblis - Nidae call storm skill
void call_storm(CHAR_DATA *ch, char *argument)
{
  int chance;
  CHAR_DATA *vch;
  if ((chance = get_skill(ch,gsn_call_storm)) < 1)
  {
    send_to_char("You call \"Storm, STORM!\" but you get nothing.\n\r",ch);
    return;
  }
  if (!ch->in_room || !IS_OUTSIDE(ch) || ch->in_room->sector_type == SECT_UNDERWATER
		  || ch->in_room->sector_type == SECT_WATER_OCEAN)
  {
    send_to_char("This room is not accepting storms at the moment, please try back NEVER.\n\r",ch);
    return;
  }
  if (ch->mana < skill_table[gsn_call_storm].min_mana)
  {
    send_to_char("You do not have enough energy to call anything.\n\r",ch);
    return;
  }
  else ch->mana -= skill_table[gsn_call_storm].min_mana;
	
  WAIT_STATE(ch,skill_table[gsn_call_storm].beats);
  if (chance > number_percent())
  {
    send_to_char("You call forth a roaring rainstorm!\n\r",ch);
    act("$n makes some wierd gestures, mumbles a bit and then you are drenched by a roaring rainstorm.",ch,NULL,NULL,TO_ROOM);
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (IS_NPC(vch) || vch->race != PC_RACE_NIDAE)
	      continue;
      vch->pcdata->oow_ticks = 0;
      send_to_char("You feel replenished as you absorb the water from above.\n\r",vch);
    }
    return;
	    
  }
  else
  {
    send_to_char("You call forth absolutely nothing!\n\r",ch);
    act("$n makes some weird gestures, then looks up and shakes $s fist at the sky.",ch,NULL,NULL,TO_ROOM);
  }
} 

//Iblis - Nidae transform skill
void do_transform(CHAR_DATA *ch, char* argument)
{
  CHAR_DATA *victim;
  int chance;
  AFFECT_DATA af;
  if ((chance = get_skill(ch,gsn_transform)) < 1)
    {
      send_to_char ("You try not to transform yourself into a fool, but its too late.\n\r",ch);
      return;
    }
  if (argument[0] == '\0')
    {
      if (IS_SET(ch->act2,ACT_TRANSFORMER))
      {
	send_to_char("You return to your true form.\n\r",ch);
	act("$n fades away...",ch,NULL,NULL,TO_ROOM);
	affect_strip(ch,gsn_transform);
	act("and $n fades into existence.",ch,NULL,NULL,TO_ROOM);
        return;
      }  
      send_to_char ("Transform into who/what?\n\r",ch);
      return;
    }
  if ((victim = get_char_room (ch, argument)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (!IS_NPC(victim))
    {
      send_to_char ("You are not powerful enough to transform into players.\n\r",ch);
      return;
    }
  if (ch->hit <= 30)
    {
      send_to_char ("You do not have the strength to transform.\n\r",ch);
      return;
    }
  else ch->hit -= 30;
   WAIT_STATE (ch, skill_table[gsn_transform].beats);
  if (chance > number_percent())
  {
    af.where = TO_AFFECTS;
    af.type = gsn_transform;
    af.level = ch->level;
    af.duration = 10;
    af.location = APPLY_STR;
    af.modifier = 0;
    af.bitvector = 0;
    af.permaff = FALSE;
    af.composition = FALSE;
    af.comp_name = str_dup ("");
    affect_to_char (ch, &af);
    act("$n fades away slowly, with $N returning in $s place.",ch,NULL,victim,TO_ROOM);
    SET_BIT(ch->act2,ACT_TRANSFORMER);
    free_string(ch->long_descr);
    ch->long_descr = str_dup(victim->long_descr);
    free_string(ch->short_descr);
    ch->short_descr = str_dup(victim->short_descr);
    act("Your liquid body ripples as it changes into a perfect replica of $N.",ch,NULL,victim,TO_CHAR);
    return;
  }
  else
  {
    act("You close your eyes, and think of the body of $N.  You wait for the change to come, but nothing happens so you look down, frowning when you see notice you're still you.",ch,NULL,victim,TO_CHAR);
    act("Some weird garbles sounds come from $n as $e rearranges $s body into some odd positions, but the end results appears to be absolutely nothing.",ch,NULL,victim,TO_ROOM);
  }
  
}

//Iblis - Mage's floating letters spell, Idea by minax, to advertise for new player stores
void spell_floating_letters (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];
  if (!ch->in_room)
    return;
  if (get_obj_vnum_room(ch->in_room,OBJ_VNUM_FLOATING_LETTERS))
    {
      send_to_char("This room already has floating letters.\n\r",ch);
      return;
    }
  else obj = create_object(get_obj_index(OBJ_VNUM_FLOATING_LETTERS),0);
  sprintf(buf,obj->description,(char*)vo);
  free_string(obj->description);
  obj->description = str_dup(buf);
  obj_to_room(obj,ch->in_room);
  obj->timer = number_range(1,ch->level);
  sprintf(buf,"Magical floating letters appear, spelling out `a'``%s`a'``.",(char*)vo);
  act(buf,ch,NULL,NULL,TO_ALL);
}



//Iblis 9/26/04 - Chaos Jester spells

void spell_vermin_skin (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  if (costs_mana)
  {
    if (ch->mana < 50-ch->perm_stat[STAT_CON])
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= 50-ch->perm_stat[STAT_CON];
  }
  if (victim != ch)
    {
      send_to_char("You can only cast that on yourself.\n\r",ch);
      return;
    }
    if (is_affected (victim, sn))
    {
        send_to_char ("You already have vermin crawling all over you.\n\r", ch);
      return;
      }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = ch->perm_stat[STAT_CON];
  af.modifier = 0-ch->perm_stat[STAT_DEX];
  af.location = APPLY_AC;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (victim, &af);
  send_to_char ("You smile as you imagine a swarm of vermin crawling all over your skin.\n\r", ch);
  /*  if (ch != victim)
      act ("$N is protected by your magic.", ch, NULL, victim, TO_CHAR);*/
  act ("Buzzing insects crawl out of the widened pores of $n's skin.",ch,NULL,NULL,TO_ROOM);
  return;
}


void spell_maelstrom (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int mana = 60-ch->perm_stat[STAT_INT]-ch->perm_stat[STAT_WIS];
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }
  if (ch->in_room->sector_type != SECT_WATER_OCEAN
      && ch->in_room->sector_type != SECT_WATER_SWIM
      && ch->in_room->sector_type != SECT_UNDERWATER
      && ch->in_room->sector_type != SECT_WATER_NOSWIM)
    {
      send_to_char ("There is no water here for you to mess with.\n\r", ch);
      return;
    }
  send_to_char ("You charge the surrounding water with chaotic magic, creating a maelstrom!\n\r",ch);
  act("The water around your starts swirling and you are sucked into a maelstrom!",ch,NULL,NULL,TO_ROOM);

  for (vch = ch->in_room->people; vch != NULL;vch = vch_next)
    {
      vch_next = vch->next_in_room;
      if (vch == ch)
	continue;
      if (!is_safe_spell (ch, vch, FALSE))
	{
	  int dam;
	  dam = ch->perm_stat[STAT_INT]*11;
	  if (vch->size > SIZE_MEDIUM)
 	    dam += (vch->size - SIZE_MEDIUM)*50;
	  if (saves_spell (level, vch, DAM_DROWNING))
	    dam /= 2;
	  damage_old (ch, vch, adjust_damage(dam), sn, DAM_DROWNING, TRUE);
	}

    }
}

void spell_tornado (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int mana = 35-ch->perm_stat[STAT_INT];
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }
  if (ch->in_room->sector_type != SECT_AIR)
    {
      send_to_char ("There is not enough air here for you to mess with.\n\r", ch);
      return;
    }
  send_to_char ("You conjure a tornado!\n\r",ch);
  act("The sky blackens as $n summons a howling tornado!",ch,NULL,NULL,TO_ROOM);

  for (vch = ch->in_room->people; vch != NULL;vch = vch_next)
    {
      vch_next = vch->next_in_room;
      if (vch == ch)
        continue;
      if (!is_safe_spell (ch, vch, FALSE))
        {
          int dam;
          dam = ch->perm_stat[STAT_STR]*9;
	  if (vch->race == PC_RACE_NERIX)
	  {
	    dam += ch->perm_stat[STAT_STR] + ch->perm_stat[STAT_INT];
	    act("The tornado tears feathers off your body!",ch,NULL,vch,TO_VICT);
	  }
          if (saves_spell (level, vch, DAM_WIND))
            dam /= 2;
          damage_old (ch, vch, adjust_damage(dam), sn, DAM_WIND, TRUE);
        }

    }
}

void spell_vortex (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA *af,af2,*first_aff=NULL,*af_next;
  int mana = 50-ch->perm_stat[STAT_INT],counter=0;
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
   else ch->mana -= mana;
  }
  if (victim->perm_stat[STAT_WIS]/2.0 > number_percent())
  {
    send_to_char("You failed!\n\r",ch);
    return;
  }
  first_aff = victim->affected;
  for (af = victim->affected; (af != victim->affected || counter < 1) && af != NULL; af = af_next)
    {
      af_next = af->next;
      ++counter;
      if (af->permaff 
            || (af->type == gsn_jalknation) || (af->type == gsn_jurgnation)
	    || (af->type == gsn_aquatitus) || (af->type == gsn_dehydration)
	    || (af->type == gsn_death_spasms) 
	    || (!IS_NPC(victim) && victim->pcdata->flaming && af->type == gsn_fireshield))
        continue;
      
      af2.where = af->where;	    
      af2.type = af->type;
      af2.level = af->level;
      af2.duration = 0 - (af->duration * ((80-victim->perm_stat[STAT_INT])/100.0));
      af2.location = af->location;
      af2.modifier = 0;
      af2.bitvector = af->bitvector;
      af2.permaff = af->permaff;
      af2.composition = af->composition;
      af2.comp_name = str_dup (af->comp_name);
      affect_join(victim,&af2);
    }
  act("You drain the life force of $N.",ch,NULL,victim,TO_CHAR);
  send_to_char("A swirling vortex of colors saps your energy.\n\r",victim);
  act("$N cries out as $E surges with chaotic magic.",ch,NULL,victim,TO_NOTVICT);
  return;
}

void spell_locust_wings (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;
  int mana = 30-ch->perm_stat[STAT_INT];
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }
  if (target == TARGET_OBJ)
    {
      obj = (OBJ_DATA *) vo;
          af.where = TO_OBJECT2;
          af.type = sn;
          af.level = level / 2;
          af.duration = level / 8;
          af.location = 0;
          af.modifier = 0;
          af.bitvector = ITEM_WINGED;
          af.permaff = FALSE;
          af.composition = FALSE;
          af.comp_name = str_dup ("");
          obj_from_char(obj);
          affect_to_obj (obj, &af);
	  act("You decide that $p would look better with wings.",ch,obj,NULL,TO_CHAR);
          act ("$p grows two pairs of insect wings!", ch, obj, NULL, TO_ROOM);
	  obj_to_char(obj,ch);
          return;
    }
  victim = (CHAR_DATA *) vo;
  /* BEGIN UNDERWATER */
  if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
      if (victim == ch)
        send_to_char ("You are already floating in the water.\n\r", ch);

      else
        act ("$N is already floating in the water.", ch, NULL, victim,
             TO_CHAR);
      return;
    }

  /* END UNDERWATER */

  //12-29-03 Iblis - By Minax's request, if you're overweight you can't fly
  if (get_carry_weight (victim) > can_carry_w (victim))
    {
      send_to_char("They are too heavy to fly.\n\r",ch);
      return;
    }

  if (IS_AFFECTED (victim, AFF_FLYING))
    {
      if (victim == ch)
        send_to_char ("You are already airborne.\n\r", ch);

      else
        act ("$N doesn't need your help to fly.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (!IS_NPC(victim) && victim->race == PC_RACE_NIDAE && number_percent() <= 50)
    {
      send_to_char ("You failed.\n\r",ch);
      return;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 70+ch->perm_stat[STAT_CON];
  af.location = 0;
  af.modifier = 0;
  af.bitvector = AFF_FLYING;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_join (victim, &af);
  send_to_char ("Translucent insect wings break out of the skin of your back!\n\r", victim);
  act ("$n gurgles in delight as translucent insect wings break out of the skin of $s back", victim, NULL, NULL, TO_ROOM);
  return;
}

void spell_torture (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int duration;
  AFFECT_DATA af;
  int mana = 50-ch->perm_stat[STAT_INT];
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }
  if (ch->in_room->sector_type == SECT_AIR)
    {
      send_to_char("That will not work here.\n\r",ch);
      return;
    }
  if (is_affected(victim,skill_lookup("torture")))
    {
      send_to_char("They are already restrained.\n\r",ch);
      return;
    }
  switch(get_curr_stat(victim,STAT_STR))
    {
    case 25:duration = 2;break;
    case 24:duration = 5;break;
    case 23:duration = 6;break;
    case 22:duration = 7;break;
    case 21:
    case 20:
    case 19:duration = 10;break;
    case 18:
    case 17:
    case 16:duration = 12;break;
    case 15:duration = 14;break;
    case 14:duration = 15;break;
    case 3:
    case 2:
    case 1:
    case 0:duration = 25;break;
    default:duration = 20;break;
    }
  act("You snap your fingers and barbed chains encircles $N.",ch,NULL,victim,TO_CHAR);
  send_to_char("You scream as barbed chains encircle you!\n\r",victim);
  act("$n snaps $s fingers and suddenly $N is constricted by barbed chains!",ch,NULL,victim,TO_NOTVICT);
  //  send_to_char("Skeletal hands burst out of the ground and grab your ankles!\n\r",victim);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = duration;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup (ch->name);
  affect_to_char (victim, &af);
}


bool check_torture (CHAR_DATA *ch)
{
  AFFECT_DATA *af, af2;
  CHAR_DATA *perp=NULL;
  int dam=0;
//  int dt=0;
  //  if (!check_torture(ch))
  //  return FALSE;
  if (!is_affected(ch,gsn_torture))
    return TRUE;
  for (af = ch->affected; af != NULL; af = af->next)
    {
      if (af->type == gsn_torture)
        break;
    }
  if (af->comp_name != NULL && str_cmp(af->comp_name,""))
    perp = get_exact_pc_world(ch,af->comp_name);
  send_to_char("You try to get loose from the chains but the barbs only cut deeper!\n\r",ch);
  dam = number_range(2,10);
  if (dam > ch->hit)
    dam = ch->hit;
  if (perp != NULL && perp->in_room && perp->in_room == ch->in_room)
    {
      act("$n struggles to get free, but the barbed chains only cut deeper.",ch,NULL,perp,TO_NOTVICT);
      act("You feel better just by watching $n's suffering.",ch,NULL,perp,TO_VICT);
      //      dam = number_range(2,10);
      perp->hit += dam;
    }
  else  act("$n struggles to get free, but the barbed chains only cut deeper.",ch,NULL,NULL,TO_ROOM);
  if (perp == NULL)
    perp = ch;
//  damage(perp,ch,dam,dt,DAM_SLASH,FALSE);
  if (af->duration-1 == 0)
    {
      affect_strip(ch,gsn_torture);
      send_to_char (skill_table[af->type].msg_off, ch);
      send_to_char ("\n\r", ch);
       crit_strike_possible = FALSE;
//      damage(perp,ch,dam,dt,DAM_SLASH,FALSE);
       crit_strike_possible = TRUE;
      if (ch->hit <= 1)
	return FALSE;
      return TRUE;
    }
  else
    {
      af2.where = af->where;
      af2.type = af->type;
      af2.level = af->level;
      af2.duration = -1;
      af2.location = af->location;
      af2.modifier = 0;
      af2.permaff = FALSE;
      af2.bitvector = 0;
      af2.composition = FALSE;
      af2.comp_name = str_dup (af->comp_name);
      affect_join (ch, &af2);
      crit_strike_possible = FALSE;
//      damage(perp,ch,dam,dt,DAM_SLASH,FALSE);
      crit_strike_possible = TRUE;
    }
  return FALSE;
}

void spell_chaos_warp (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  //  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int mana = 60-ch->perm_stat[STAT_INT];
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }
  if (is_affected (ch, sn))
    {
      //  if (victim == ch)
	send_to_char ("You are already filled with chaotic energies.\n\r", ch);
      
	//else
	// act ("$N is already .", ch, NULL, victim, TO_CHAR);
      return;
    }
  af.where = TO_ACT2;
  af.type = sn;
  af.level = level;
  af.duration = 20+ch->perm_stat[STAT_STR];
  af.modifier = 0;
  af.location = 0;
  af.bitvector = ACT_WARPED;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (ch, &af);
  af.where = TO_AFFECTS;
  af.location = APPLY_HITROLL;
  af.bitvector = 0;
  af.modifier = 7;
  affect_to_char (ch, &af);
  send_to_char("Your skin ripples and contorts as your allow the chaotic energies fill you.\n\r",ch);
  act("The air sparkles with multicolored pulses of magic as $n's body contorts to something otherworldly.",ch,NULL,NULL,TO_ROOM);
  /*  send_to_char ("You feel someone protecting you.\n\r", victim);
  if (ch != victim)
  act ("$N is protected by your magic.", ch, NULL, victim, TO_CHAR);*/
  return;
}

void spell_warped_space (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj;
  OBJ_INDEX_DATA *oid;
  int mana = ch->max_mana*((90 - ch->perm_stat[STAT_INT])/100.0);
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }
  if (ch->in_room == NULL)
    return;
  if (IS_SET(ch->in_room->room_flags2,ROOM_WARPED))
    {
      send_to_char("This room is already in warped space.\n\r",ch);
      return;
    }
  oid = get_obj_index (OBJ_VNUM_WARPED_SPACE);
  if (oid == NULL)
    {
      send_to_char("ERROR!! TELL AN IMM WARPED SPACE FAILED!\n\r",ch);
      return;
    }
  if (oid->count > 0)
    {
      for (obj = object_list;obj != NULL;obj = obj->next)
        {
          if (obj->pIndexData->vnum == OBJ_VNUM_WARPED_SPACE)
            {
              if (!str_cmp(obj->plr_owner,ch->name))
                {
                  send_to_char("You may only have one warped space room in the realm at a time.\n\r",ch);
                  return;
                }
            }
        }
    }
  obj = create_object(oid,0);
  obj->plr_owner = str_dup(ch->name);
  obj->value[0] = ch->in_room->vnum;   //For redundancy, may not use it
  obj_to_room(obj,get_room_index(ROOM_VNUM_DV_LIMBO));
  //  act("The septagram starts glowing with a green light.",ch,NULL,NULL,TO_ALL);
  act("The air around $n twists and bends with chaotic magic.",ch,NULL,NULL,TO_ROOM);
  send_to_char("The world around you twists and bends to match your psyche.\n\r",ch);
  //  act("$n conjures a sphere of complete darkness.",ch,NULL,NULL,TO_ROOM);
  SET_BIT(ch->in_room->room_flags2,ROOM_WARPED);
  obj->timer = 4;
}

void spell_malevolence (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  short done = 0;
  int mana = 50-ch->perm_stat[STAT_INT];
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }
  act("You imagine bad things happening to $N, and smile.",ch,NULL,victim,TO_CHAR);
  act("With a dreamy look on $s face $n smiles wickedly at you.",ch,NULL,victim,TO_VICT);
  act("Goosebumps cover your skin as you see $n's wicked smile.",ch,NULL,victim,TO_NOTVICT);
  if (IS_AFFECTED (victim, AFF_HASTE) && number_percent() <= 50 && !is_affected(victim,skill_lookup("slow")))
    {
      done++;
      spell_slow(find_spell(ch,"slow"),ch->level,ch,(void*)victim, 0);
    }
  if ((IS_AFFECTED (victim, AFF_FLYING) || is_affected(victim,skill_lookup("giant strength")))
      && !is_affected(victim,skill_lookup("weaken")) && number_percent() <= 50)
    {
      done++;
      spell_weaken(find_spell(ch,"weaken"),ch->level,ch,(void*)victim, 0);
    }
  if (IS_AFFECTED (victim,AFF_SANCTUARY) && done < 2 && number_percent() <= 75 
		  && !is_affected(victim,gsn_poison))
    {
      ++done;
      spell_poison(gsn_poison,ch->level,ch,(void*)victim, 0);
    }
  if (is_affected (victim,gsn_fireshield) && done < 2 && number_percent() <= 75
		  && !is_affected(victim,gsn_curse))
    {
      ++done;
      spell_poison(gsn_curse,ch->level,ch,(void*)victim, 0);
    }
  if ((is_affected(victim,skill_lookup("armor")) || is_affected(victim,skill_lookup("conservancy")) ||
      is_affected(victim,skill_lookup("shield")) || is_affected(victim,skill_lookup("exoskeleton")))
      && done < 2)
    {
      ++done;
      spell_faerie_fire(find_spell(ch,"faerie fire"),ch->level,ch,(void*)victim, 0);
    }


  return;
}

void spell_meat_storm (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int vnum;
  int num_people=-1;
  int counter=0;
  int rand_person=0;
  const char *vp;
  OBJ_DATA *obj, *obj_next;
  int mana = 30-ch->perm_stat[STAT_CON];
  if (costs_mana)
  {
    if (ch->mana < mana) 
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }
  send_to_char ("You let loose the chaotic energies within!\n\r",ch);
  act("The room sizzles with chaotic energies!",ch,NULL,NULL,TO_ROOM);
  for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      vnum = obj->pIndexData->vnum;
      if (vnum <= OBJ_VNUM_BRAINS && vnum >= OBJ_VNUM_BUTCHER_MEAT &&
	  vnum != OBJ_VNUM_CORPSE_PC && vnum != OBJ_VNUM_CORPSE_NPC)
	{
	  if (num_people == -1)
	    {
	      for (vch = ch->in_room->people; vch != NULL;vch = vch_next)
		{
		  vch_next = vch->next_in_room;
		  if (vch == ch)
		    continue;
		  if (is_same_group (vch, ch))
			  continue;
		  if (is_safe_spell (ch, vch, FALSE))
		    continue;
		  if (num_people == -1)
		    num_people = 1;
		  else ++num_people;
		}
	    }
	  if (num_people < 1)
	    return;
	  counter = 0;
	  rand_person = number_range(1,num_people);
	  for (vch = ch->in_room->people; vch != NULL;vch = vch_next)
	    {
	      vch_next = vch->next_in_room;
	      if (vch == ch)
		continue;
	      if (is_same_group (vch, ch))
		      continue;
	      if (!is_safe_spell (ch, vch, FALSE))
		{
		  int dam=0;
		  if (++counter != rand_person)
		    continue;
		  dam = number_range(10,30);
		  /*		  if (vch->size > SIZE_MEDIUM)
		    dam += (vch->size - SIZE_MEDIUM)*50;
		  if (saves_spell (level, vch, DAM_DROWNING))
		  dam /= 2;*/
		  damage_old (ch, vch, adjust_damage(dam), sn, DAM_BASH, FALSE);
		  if (dam == 0)
		    {
		      vp = "misses";
		    }

		  else if (dam <= 8)
		    {
		      vp = "barely touches";
		    }

		  else if (dam <= 15)
		    {
		      vp = "nicks";
		    }


		  else if (dam <= 20)
		    {
		      vp = "scratches";
		    }

		  else if (dam <= 27)
		    {
		      vp = "hits";
		    }

		  else if (dam <= 34)
		    {
		      vp = "injures";
		    }

		  else if (dam <= 41)
		    {
		      vp = "wounds";
		    }

		  else if (dam <= 48)
		    {
		      vp = "mauls";
		    }

		  else if (dam <= 55)
		    {
		      vp = "decimates";
		    }

		  else if (dam <= 62)
		    {
		      vp = "maims";
		    }

		  else if (dam <= 69)
		    {
		      vp = "MUTILATES";
		    }

		  else if (dam <= 76)
		    {
		      vp = "DISEMBOWELS";
		    }
		  else if (dam <= 83)
		    {
		      vp = "DISMEMBERS";
		    }

		  else if (dam <= 90)
		    {
		      vp = "MASSACRES";
		    }

		  else if (dam <= 97)
		    {
		      vp = "MANGLES";
		    }

		  else if (dam <= 104)
		    {
		      vp = "*** DEMOLISHES ***";
		    }

		  else if (dam <= 115)
		    {
		      vp = "*** DEVASTATES ***";
		    }

		  else if (dam <= 125)
		    {
		      vp = "=== OBLITERATES ===";
		    }

		  else if (dam <= 135)
		    {
		      vp = ">>> ANNIHILATES <<<";
		    }

		  else if (dam <= 150)
		    {
		      vp = "<<< ERADICATES >>>";
		    }

		  else if (dam <= 170)
		    {
		      vp = "--- DESTROYS ---";
		    }

		  else if (dam <= 190)
		    {
		      vp = "--- PULVERIZES ---";
		    }

		  else if (dam <= 250)
		    {
		      vp = "does UNSPEAKABLE things to";
		    }

		  else if (dam <= 350)
		    {
		      vp = "~~~ LIQUIFIES ~~~";
		    }
		  else if (dam <= 450)
		    {
		      vp = "~~~ VAPORIZES ~~~";
		    }

		  else
		    {
		      vp = "does UNGODLY DAMAGE to";
		    }
		  act ("$p `B$T`` you.",vch,obj,vp,TO_CHAR);
		  obj_from_room(obj);
		  extract_obj(obj);
		  break;


		}
	    }
	  
	}
    }
}

void spell_imaginary_garb (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj;
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  char *argument;
  int mana = 44-ch->perm_stat[STAT_INT];
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }
  if (vo == NULL)
  {
	  send_to_char("c 'imaginary garb' WEARLOCATION name of item\n\r",ch);
	  return;
  }
  argument = one_argument((char*)vo,arg);
  if (argument[0] == '\0')
  {
	  send_to_char("c 'imaginary garb' WEARLOCATION name of item\n\r",ch);
	  return;
  }
  if (get_obj_index(OBJ_VNUM_IMAGINARY_GARB))
    obj = create_object(get_obj_index(OBJ_VNUM_IMAGINARY_GARB),0);
  else {
    send_to_char("ERRRROOOORRRRRRR!\n\r",ch);
    return;
  }
  obj->item_type = ITEM_ARMOR;
  if (!str_prefix(arg,"finger"))
    {
      obj->wear_flags = ITEM_WEAR_FINGER;
    }
  else if (!str_prefix(arg,"neck"))
    {
      obj->wear_flags = ITEM_WEAR_NECK;
    }
  else if (!str_prefix(arg,"torso"))
    {
      obj->wear_flags = ITEM_WEAR_BODY;
    }
  else if (!str_prefix(arg,"head"))
    {
      obj->wear_flags = ITEM_WEAR_HEAD;
    }
  else if (!str_prefix(arg,"legs"))
    {
      obj->wear_flags = ITEM_WEAR_LEGS;
    }
  else if (!str_prefix(arg,"feet"))
    {
      obj->wear_flags = ITEM_WEAR_FEET;
    }
  else if (!str_prefix(arg,"hands"))
    {
      obj->wear_flags = ITEM_WEAR_HANDS;
    }
  else if (!str_prefix(arg,"arms"))
    {
      obj->wear_flags = ITEM_WEAR_ARMS;
    }
  else if (!str_prefix(arg,"shield"))
    {
      obj->wear_flags = ITEM_WEAR_SHIELD;
    }
  else if (!str_prefix(arg,"body"))
    {
      obj->wear_flags = ITEM_WEAR_ABOUT;
    }
  else if (!str_prefix(arg,"waist"))
    {
      obj->wear_flags = ITEM_WEAR_WAIST;
    }
  else if (!str_prefix(arg,"wrist"))
    {
      obj->wear_flags = ITEM_WEAR_WRIST;
    }
  else if (!str_prefix(arg,"held"))
    {
      obj->wear_flags = ITEM_HOLD;
    }
  else if (!str_prefix(arg,"face"))
    {
      obj->wear_flags = ITEM_WEAR_FACE;
    }
  /*  else if (!str_prefix(arg,"floating"))
    {
      obj->wear_flags = ITEM_WEAR_FLOATING;
      }*/
  else if (!str_prefix(arg,"light"))
    {
      obj->item_type = ITEM_LIGHT;
    }
  else if (!str_prefix(arg, "clan"))
  {
    obj->item_type = ITEM_WEAR_CLAN_MARK;
  }
  else 
  {
	  send_to_char(arg,ch);
	  send_to_char("\n\r",ch);
    send_to_char("Create an item to wear on what slot?\n\r",ch);
    return;
  }
  obj->wear_flags |= ITEM_TAKE;
  if (obj->item_type == ITEM_ARMOR)
    {
      obj->value[0] = 4;
      obj->value[1] = 4;
      obj->value[2] = 4;
      obj->value[3] = 4;
    }
  else //it's a light
    {
      extract_obj(obj);
      if (get_obj_index(OBJ_VNUM_IMAGINARY_LIGHT))
	obj = create_object(get_obj_index(OBJ_VNUM_IMAGINARY_LIGHT),0);
    }
  free_string(obj->short_descr);
  obj->short_descr = str_dup(argument);
  free_string(obj->name);
  sprintf(buf,"%s %s",arg,argument);
  obj->name = str_dup(buf);
  if (obj->plr_owner != NULL)
    free_string(obj->plr_owner);
  obj->plr_owner = str_dup(ch->name);
  obj_to_char(obj,ch);
  act("You will $p into existance.",ch,obj,NULL,TO_CHAR);
  act("$n wills $p into existance.",ch,obj,NULL,TO_ROOM);
}


void spell_gravity (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  //  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  OBJ_DATA *obj, *obj_next;
  CHAR_DATA *gch;
  int members = 0;
  char buffer[MAX_STRING_LENGTH];
  int mana = 40-ch->perm_stat[STAT_INT];
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }
  if (is_affected (ch, sn))
    {
        send_to_char ("You are already filled with power.\n\r", ch);
      return;
    }
  af.where = TO_AFFECTS;
  af.type = gsn_gravity;
  af.level = level;
  af.duration = ch->perm_stat[STAT_STR]+20;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (ch, &af);
  send_to_char ("Red pulsing waves of energy start emanating from your skin.\n\r", ch);
  act("You feel a strong pull towards $n.",ch,NULL,NULL,TO_ROOM);
  if (!ch->in_room)
    return;
  for (obj = ch->in_room->contents;obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      if (!IS_SET(obj->wear_flags,ITEM_TAKE))
	continue;
      if (IS_SET(obj->extra_flags[1],ITEM_WIZI) && !IS_IMMORTAL(ch))
        continue;
      if (!obj->in_room)
        continue;
      for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
        if (gch->on == obj)
            continue;
      if ((!obj->in_obj || obj->in_obj->carried_by != ch)
	  && (get_carry_weight (ch) + get_obj_weight (obj) > can_carry_w (ch)))
	  continue;
      if (IS_SET(obj->extra_flags[0],ITEM_NOMOB) && IS_NPC(ch))
	continue;
      if (obj->plr_owner != NULL)
	if (str_cmp (ch->name, obj->plr_owner) && !IS_IMMORTAL(ch))
	  continue;
      if (ch->carry_number + get_obj_number (obj) > can_carry_n (ch))
	  continue;
      if (obj->item_type == ITEM_QUIVER && has_quiver (ch))
	  continue;
      if (!IS_NPC (ch))
	{
	  if (ch->mount != NULL)
	    if (get_char_weight (ch) + get_obj_weight (obj) >
		ch->mount->pIndexData->max_weight)
	      {
		//act ("$d: your mount can't carry that much weight.", ch,
		//    NULL, obj->name, TO_CHAR);
		continue;
	      }
	}
      if (obj->item_type == ITEM_MONEY)
	{
	  ch->silver += obj->value[0];
	  ch->gold += obj->value[1];
	  act("$p is pulled into your inventory.",ch,obj,NULL,TO_CHAR);
	  act("$p levitates into $n's hands.",ch,obj,NULL,TO_ROOM);
	  if (IS_SET (ch->act, PLR_AUTOSPLIT))
	    {                       /* AUTOSPLIT code */
	      members = 0;
	      for (gch = ch->in_room->people; gch != NULL;
		   gch = gch->next_in_room)
		{

		  /*
		   * Mathew: Added !IS_NPC(gch) to be sure uncharmed NPC's and mounts
		   * do not get a share of the money. 4/13/99
		   */
		  if (is_same_group (gch, ch)
		      && !IS_AFFECTED (gch, AFF_CHARM) && !IS_NPC (gch))
		    members++;
		}
	      if (members > 1 && (obj->value[0] > 1 || obj->value[1]))
		{
		  sprintf (buffer, "%d %d", obj->value[0], obj->value[1]);
		  do_split (ch, buffer);
		}
	    }
	  extract_obj (obj);
	}
      else
	{
	  act("$p is pulled into your inventory.",ch,obj,NULL,TO_CHAR);
	  act("$p levitates into $n's hands.",ch,obj,NULL,TO_ROOM);
	  obj_from_room (obj);
	  obj_to_char (obj, ch);
	  // if (obj != NULL && obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC)
	  //	save_player_corpse (obj);
	  trip_triggers(ch, OBJ_TRIG_GET, obj, NULL, OT_SPEC_NONE);
	}

      //check weight
   
    }
  //  if (ch != victim)
  //    act ("$N is protected by your magic.", ch, NULL, victim, TO_CHAR);
  return;
}

void check_gravity_char (CHAR_DATA *ch)
{
  OBJ_DATA *obj,*obj_next;
  CHAR_DATA *gch;
  int members=0;
  char buffer[MAX_STRING_LENGTH];
  //  for (ch = people; ch != NULL; ch = people->next_in_room)
  // {
  if (is_affected(ch,gsn_gravity))
    {
      for (obj = ch->in_room->contents;obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if (!IS_SET(obj->wear_flags,ITEM_TAKE))
	    continue;
	  if (IS_SET(obj->extra_flags[1],ITEM_WIZI) && !IS_IMMORTAL(ch))
	    continue;
	  if (!obj->in_room)
  	    continue;
	  for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	    if (gch->on == obj)
	      continue;
	  if ((!obj->in_obj || obj->in_obj->carried_by != ch)
	      && (get_carry_weight (ch) + get_obj_weight (obj) > can_carry_w (ch)))
	    continue;
	  if (IS_SET(obj->extra_flags[0],ITEM_NOMOB) && IS_NPC(ch))
	    continue;
	  if (obj->plr_owner != NULL)
	    if (str_cmp (ch->name, obj->plr_owner) && !IS_IMMORTAL(ch))
		  continue;
	  if (ch->carry_number + get_obj_number (obj) > can_carry_n (ch))
	    continue;
	  if (obj->item_type == ITEM_QUIVER && has_quiver (ch))
	    continue;
	  if (!IS_NPC (ch))
	    {
	      if (ch->mount != NULL)
		if (get_char_weight (ch) + get_obj_weight (obj) >
		    ch->mount->pIndexData->max_weight)
		  {
		    //act ("$d: your mount can't carry that much weight.", ch,
		    //    NULL, obj->name, TO_CHAR);
		    continue;
		  }
	    }
	  if (obj->item_type == ITEM_MONEY)
	    {
	      act("$p is pulled into your inventory.",ch,obj,NULL,TO_CHAR);
	      act("$p levitates into $n's hands.",ch,obj,NULL,TO_ROOM);
	      ch->silver += obj->value[0];
	      ch->gold += obj->value[1];
	      if (IS_SET (ch->act, PLR_AUTOSPLIT))
		{                       /* AUTOSPLIT code */
		  members = 0;
		  for (gch = ch->in_room->people; gch != NULL;
		       gch = gch->next_in_room)
		    {
		      
		      /*
		       * Mathew: Added !IS_NPC(gch) to be sure uncharmed NPC's and mounts
		       * do not get a share of the money. 4/13/99
		       */
		      if (is_same_group (gch, ch)
			  && !IS_AFFECTED (gch, AFF_CHARM) && !IS_NPC (gch))
			members++;
		    }
		  if (members > 1 && (obj->value[0] > 1 || obj->value[1]))
		    {
		      sprintf (buffer, "%d %d", obj->value[0], obj->value[1]);
		      do_split (ch, buffer);
		    }
		}
	      extract_obj (obj);
	    }
	  else
	    {
	      act("$p is pulled into your inventory.",ch,obj,NULL,TO_CHAR);
	      act("$p levitates into $n's hands.",ch,obj,NULL,TO_ROOM);
	      obj_from_room(obj);
	      obj_to_char (obj, ch);
	      // if (obj != NULL && obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC)
	      //    save_player_corpse (obj);
	      trip_triggers(ch, OBJ_TRIG_GET, obj, NULL, OT_SPEC_NONE);
	    }
	  //	  act("$p is pulled into your inventory.",ch,obj,NULL,TO_CHAR);
	  
	}
    }
}


void check_gravity_room_obj (CHAR_DATA *people,OBJ_DATA *obj)
{
  CHAR_DATA *ch, *gch;
  int members = 0;
  char buffer[MAX_STRING_LENGTH];
  log_string(obj->name);
  log_string(obj->in_room->name);
  log_string(people->name);
  for (ch = people; ch != NULL; ch = people->next_in_room)
    {
      if (is_affected(ch,gsn_gravity))
        {
	  /*          for (obj = ch->in_room->contents;obj != NULL; obj = obj_next)
            {
	    obj_next = obj->next_content;*/
              if (!IS_SET(obj->wear_flags,ITEM_TAKE))
                continue;
	      if (IS_SET(obj->extra_flags[1],ITEM_WIZI) && !IS_IMMORTAL(ch))
                continue;
              for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
                if (gch->on == obj)
                  return;
              if ((!obj->in_obj || obj->in_obj->carried_by != ch)
                  && (get_carry_weight (ch) + get_obj_weight (obj) > can_carry_w (ch)))
                continue;
              if (IS_SET(obj->extra_flags[0],ITEM_NOMOB) && IS_NPC(ch))
                continue;
              if (obj->plr_owner != NULL)
                if (str_cmp (ch->name, obj->plr_owner) && !IS_IMMORTAL(ch))
                  continue;
              if (ch->carry_number + get_obj_number (obj) > can_carry_n (ch))
                continue;
              if (obj->item_type == ITEM_QUIVER && has_quiver (ch))
                continue;
              if (!IS_NPC (ch))
                {
                  if (ch->mount != NULL)
                    if (get_char_weight (ch) + get_obj_weight (obj) >
                        ch->mount->pIndexData->max_weight)
                      {
                        //act ("$d: your mount can't carry that much weight.", ch,
                        //    NULL, obj->name, TO_CHAR);
                        continue;
                      }
                }
              if (obj->item_type == ITEM_MONEY)
                {
                  ch->silver += obj->value[0];
                  ch->gold += obj->value[1];
		  act("$p is pulled into your inventory.",ch,obj,NULL,TO_CHAR);
		  act("$p levitates into $n's hands.",ch,obj,NULL,TO_ROOM);
                  if (IS_SET (ch->act, PLR_AUTOSPLIT))
                    {                       /* AUTOSPLIT code */
                      members = 0;
                      for (gch = ch->in_room->people; gch != NULL;
                           gch = gch->next_in_room)
                        {

                          /*
                           * Mathew: Added !IS_NPC(gch) to be sure uncharmed NPC's and mounts
                           * do not get a share of the money. 4/13/99
                           */
                          if (is_same_group (gch, ch)
                              && !IS_AFFECTED (gch, AFF_CHARM) && !IS_NPC (gch))
                            members++;
                        }
                      if (members > 1 && (obj->value[0] > 1 || obj->value[1]))
                        {
                          sprintf (buffer, "%d %d", obj->value[0], obj->value[1]);
                          do_split (ch, buffer);
                        }
                    }
                  extract_obj (obj);
                }
              else
                {
		  act("$p is pulled into your inventory.",ch,obj,NULL,TO_CHAR);
		  act("$p levitates into $n's hands.",ch,obj,NULL,TO_ROOM);
		  obj_from_room(obj);
                  obj_to_char (obj, ch);
		  // if (obj != NULL && obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC)
                  //    save_player_corpse (obj);
                  trip_triggers(ch, OBJ_TRIG_GET, obj, NULL, OT_SPEC_NONE);
                }

	}
        
    }
}

CHAR_DATA *get_gravity_char_room(ROOM_INDEX_DATA *rid)
{
  CHAR_DATA *mch;
  if (!rid->people)
    return NULL;
  for (mch = rid->people;mch != NULL; mch = mch->next_in_room)
    {
      if (is_affected(mch,gsn_gravity))
	return mch;
    }
  return NULL;
}

void spell_chaos_bolt (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  int mana = (number_range(20,40))-ch->perm_stat[STAT_INT];
  if (costs_mana)
  {
    if (mana < 1)
      mana = 1;
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;  
  }
  switch (number_range(1,14))
  {
  case 1: spell_fireball (skill_lookup("fireball"), level,ch, vo,target);break;
  case 2: spell_magic_missile (skill_lookup("magic missile"), level,ch, vo,target);break;
  case 3: spell_acid_blast (skill_lookup("acid blast"), level,ch, vo,target);break;
  case 4: spell_ray_of_truth (skill_lookup("ray of truth"), level,ch, vo,target);break;
  case 5: spell_demonfire (skill_lookup("demonfire"), level,ch, vo,target);break;
  case 6: spell_chain_lightning (skill_lookup("chain lightning"), level,ch, vo,target);break;
  case 7: spell_chill_touch (skill_lookup("chill touch"), level,ch, vo,target);break;
  case 8: spell_burning_hands (skill_lookup("burning hands"), level,ch, vo,target);break;
  case 9: spell_harm (skill_lookup("harm"), level,ch, vo,target);break;
  case 10: spell_firestorm (skill_lookup("firestorm"), level,ch, vo,target);break;
  case 11: chant_wind_of_death (skill_lookup("wind of death"), level,ch, vo,target);break;
  case 12: chant_seizure (skill_lookup("seizure"), level,ch, vo,target);break;
  case 13: spell_cause_critical (skill_lookup("cause critical"), level,ch, vo,target);break;
  case 14: spell_heal (skill_lookup("heal"), level,ch, vo,target);break;
  }

  return;
}

void spell_spider_body (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
//  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int mana = 50-ch->perm_stat[STAT_INT];
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }
  if (is_affected (ch, sn))
  {
      send_to_char ("You already have a spider body.\n\r", ch);
      return;
  }
  if (is_affected (ch,skill_lookup("scramble")))
  {
    send_to_char ("You are too scrambled to grow a spider body.\n\r",ch);
    return;
  }
  //IF NOT AFFECTED BY SCRAMBLE..
  af.where = TO_SKILL;
  af.type = sn;
  af.level = level;
  af.duration = 10 + ch->perm_stat[STAT_CON];
  af.modifier = 80;
  af.location = skill_lookup("web");
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  //80% web skill
  affect_to_char (ch, &af);
  af.modifier = 100;
  af.location = gsn_escape;
  affect_to_char (ch, &af);
  if (!IS_SET(ch->affected_by,AFF_HASTE))
    {
      af.where = TO_AFFECTS;
      af.location = APPLY_DEX;
      af.modifier = 1 + (level >= 75);
      af.bitvector = AFF_HASTE;
      //haste
      affect_to_char (ch, &af);
    }
  if (!IS_SET(ch->vuln_flags,VULN_BASH))
    {
      af.where = TO_VULN;
      af.location = 0;
      af.modifier = 0;
      af.bitvector = VULN_BASH;
      //VULN BASH
      affect_to_char (ch, &af);
    }

  send_to_char ("Your lower body bloats into the black and red body of a spider.\n\r", ch);
  //  if (ch != victim)
  act ("Poking $s legs curiously, $n watches as $s lower body turns into that of a spider's.", ch, NULL, NULL, TO_ROOM);
  return;
}

void spell_meteor_swarm (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af, *paf;
  int mana = 200-ch->perm_stat[STAT_INT];
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }
  for (paf = victim->affected; paf != NULL; paf = paf->next)
  {
    if (paf->type == gsn_meteor_swarm && paf->comp_name != NULL && !str_cmp(paf->comp_name,ch->name))
    {
      send_to_char("They have enough meteors after them already.\n\r",ch);
      return;
    }
  }
  act("Pointing a finger at $N, you let the meteors know where to land.",ch,NULL,victim,TO_CHAR);
  act("A feeling of impending doom settles on you as $n points a finger at you and mumbles something at the sky.",ch,NULL,victim,TO_VICT);
  act("A distant whistling sound can be heard overhead.",ch,NULL,victim,TO_NOTVICT);
  af.where = TO_IGNORE;
  af.type = sn;
  af.level = level;
  af.duration = 1;
  af.modifier = 0;
  af.location = 0;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup (ch->name);
  affect_to_char (victim, &af);
  return;
}

void meteor_swarm(CHAR_DATA *ch, CHAR_DATA *victim)
{
  short i = 0, num = number_range(3,5);
  if (victim->in_room == NULL || !IS_OUTSIDE(victim))
    return;
  do_zecho(victim,"A burning rain of meteors shakes the earth on impact!");

  for (i = 0; i < num; ++i)
    {
      send_to_char("A meteor crushes your body to a pulp!\n\r",victim);
      if (i == 0)
	act("A swarm of burning meteors slam $n into the ground!",victim,NULL,NULL,TO_ROOM);
      damage_old (ch,victim,100,gsn_meteor_swarm,(number_percent()<=50)?DAM_BASH:DAM_FIRE,FALSE);
      //A very cheap and inaccurate way of determining if someone just died
      //if (victim->hit == 1)
      //  return;
      if (!is_affected(victim,gsn_meteor_swarm))
	return;
      /*      if (victim->hit == 1)
	      return;*/
    }
}

void spell_scramble (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  AFFECT_DATA af;
  int total;
  int mana = 100-ch->perm_stat[STAT_INT];
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }
  if (is_affected(ch,skill_lookup("spider body")))
    {
      send_to_char("You have too many body parts to scramble.\n\r",ch);
      return;
    }

  if (is_affected (ch, sn))
    {
        send_to_char ("You couldn't get more scrambled if you were an egg!\n\r", ch);
      return;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 15 + ch->perm_stat[STAT_CON];
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (ch, &af);
  send_to_char ("Your arms and legs are ripped out of their sockets, just to be reattached in random places on your body a moment later.\n\r", ch);
  act("$n giggles as $s limbs switch places.",ch,NULL,NULL,TO_ROOM);
  total = ch->max_move + ch->max_mana + ch->max_hit;
  switch(number_range(1,6))
    {
    case 1:
      ch->max_hit = number_range(1,total);
      total -= ch->max_hit;
      ch->max_mana = number_range(1,total);
      ch->max_move = total-ch->max_mana;
      break;
    case 2:
      ch->max_hit = number_range(1,total);
      total -= ch->max_hit;
      ch->max_move = number_range(1,total);
      ch->max_mana = total-ch->max_move;
      break;
    case 3:
      ch->max_mana = number_range(1,total);
      total -= ch->max_mana;
      ch->max_hit = number_range(1,total);
      ch->max_move = total-ch->max_hit;
      break;
    case 4:
      ch->max_mana = number_range(1,total);
      total -= ch->max_mana;
      ch->max_move = number_range(1,total);
      ch->max_hit = total-ch->max_move;
      break;
    case 5:
      ch->max_move = number_range(1,total);
      total -= ch->max_move;
      ch->max_hit = number_range(1,total);
      ch->max_mana = total-ch->max_hit;
      break;
    case 6:
      ch->max_move = number_range(1,total);
      total -= ch->max_move;
      ch->max_mana = number_range(1,total);
      ch->max_hit = total-ch->max_mana;
      break;
    }
  return;

}


void spell_vivify (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  CHAR_DATA *mob;
  char buf[MAX_STRING_LENGTH];
  int i=0, counter=0;
  int mana = 35-ch->perm_stat[STAT_CHA];
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }
  if (obj->item_type == ITEM_BINDER)
  {
    send_to_char("Nuh uh!\n\r",ch);
    return;
  }
  //CHECK TO MAKE SURE THERE ARE LESS THAN 10 SO FAR
  for (mob = char_list;mob != NULL;mob = mob->next)
    {
      if (IS_NPC(mob) && mob->pIndexData->vnum == MOB_VNUM_VIVIFY
	  && !str_cmp(mob->afkmsg,ch->name))
	{
	  if (++counter >= 10)
	    {
	      send_to_char("You already have enough vivified objects.\n\r",ch);
	      return;
	    }
	}
    }
  if (get_mob_index(MOB_VNUM_VIVIFY) != NULL)
    mob = create_mobile(get_mob_index(MOB_VNUM_VIVIFY));
  else return;
  char_to_room(mob,ch->in_room);
  mob->afkmsg = str_dup (ch->name);
  mob->ticks_remaining = ch->perm_stat[STAT_CHA];
  mob->hit = ch->perm_stat[STAT_CON]+20;
  mob->max_hit = ch->perm_stat[STAT_CON]+20;
  mob->hitroll = ch->perm_stat[STAT_DEX];
  for (i=0;i<4;++i)
    mob->armor[i] = 0-ch->perm_stat[STAT_DEX];
  free_string(mob->name);
  mob->name = str_dup("spiderlegs");

  free_string(mob->short_descr);
  mob->short_descr = str_dup(obj->short_descr);

  sprintf(buf,mob->long_descr,obj->short_descr);
  free_string(mob->long_descr);
  mob->long_descr = str_dup(buf);
  switch (obj->item_type)
    {
    case ITEM_WEAPON:
      mob->hitroll += 20;
      break;
    default :
      if (IS_SET(obj->wear_flags,ITEM_WEAR_SHIELD))
      {
        mob->hit += 20;
        mob->max_hit += 20;
        SET_BIT(mob->off_flags,OFF_DODGE);
      }
      break;
    case ITEM_CARD:
      for (i=0;i<MAX_STATS;++i)
	mob->perm_stat[i] = 3;
      mob->hit = 1;
      mob->max_hit = 1;
      mob->hitroll = -20;
      mob->level = 1;
      break;
    case ITEM_TREASURE:
      if (obj->pIndexData->vnum != OBJ_VNUM_RUNE_CONV)
	break;
      mob->level = 90;
      mob->max_hit = ch->perm_stat[STAT_CON]*200;
      mob->hit = mob->max_hit;
      mob->number_of_attacks = 4;
      mob->damage[0] = 20;
      mob->damage[1] = 20;
      mob->damage[1] = 20;
      for (i=0;i<MAX_STATS;++i)
	mob->perm_stat[i] = 20;
      mob->hitroll = 100;
      mob->size = SIZE_LARGE;
      for (i=0;i<4;++i)
	mob->armor[i] = mob->perm_stat[STAT_DEX]*-3;
      mob->res_flags |= RES_FIRE;
      mob->imm_flags |= IMM_ENTANGLE;
      mob->act2 |= ACT_WARPED;
      mob->affected_by |= AFF_SANCTUARY | AFF_AQUA_BREATHE;
      free_string(mob->name);
      mob->name = str_dup("Sh'tar shtar");

      free_string(mob->long_descr);
      sprintf(buf," `kSh'tar`b, a demon from the warped space, has answered the summons of %s.``\n\r",ch->name);
      mob->long_descr = str_dup(buf);

      //      sprintf(buf,mob->long_descr,obj->short_descr);
      free_string(mob->short_descr);
      mob->short_descr = str_dup("`kSh'tar``");
      mob->ticks_remaining = -1;
      SET_BIT (mob->act, ACT_PET);
      ch->pet = mob;
      SET_BIT (mob->affected_by, AFF_CHARM);
      

    }
  /*  free_string(mob->name);
  mob->name = str_dup(obj->name);

  free_string(mob->short_descr);
  mob->short_descr = str_dup(obj->short_descr);

  sprintf(buf,mob->long_descr,obj->short_descr);
  free_string(mob->long_descr);
  mob->long_descr = str_dup(buf);*/
  add_follower (mob, ch);
  mob->leader = ch;
  if (mob->hitroll > 99)
  {
    act("$n opens a portal to another plane and Sh'tar steps out.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You summon a being from another plane.\n\r",ch);
  }
  else act("Hairy spider legs wiggle out of $p.",ch,obj,NULL,TO_ALL);
  if (obj->contains)
    {
      OBJ_DATA *obj2=NULL,*obj2_next=NULL;
      for (obj2 = obj->contains; obj2; obj2 = obj2_next)
        {
          obj2_next = obj2->next_content;
	  //          unequip_char (victim, obj);
          obj_from_obj (obj2);
          obj_to_room (obj2,ch->in_room);
          act ("$p falls to the ground", ch, obj2, NULL, TO_ALL);
        }
    }  
  obj_from_char(obj);
  extract_obj(obj);
}


void spell_wielded_enemy (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int mana = 27-ch->perm_stat[STAT_INT];
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }

  act("You imagine $N's weapon turning on $M...",ch,NULL,victim,TO_CHAR);
  if ((get_eq_char (victim, WEAR_WIELD_R) == NULL) &&
      (get_eq_char (victim, WEAR_WIELD_L) == NULL))
    {
      act("This would be easier to do if $N had a weapon wielded.",ch,NULL,victim,TO_CHAR);
      return;
    }
  one_hit(victim,victim,-5);
  return;
}

void spell_erratic_bolt (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *rch = NULL;
  ROOM_INDEX_DATA *last_room, *to_room=NULL;
  int dam,i=0,num=0;
  int total = 0, counter = 0, ptotal = 0, door=0;
  EXIT_DATA *pexit;
  int mana = 120-ch->perm_stat[STAT_INT]-ch->perm_stat[STAT_WIS];
  if (costs_mana)
  {
    if (ch->mana < mana)
    {
      send_to_char("You don't have enough mana.\n\r",ch);
      return;
    }
    else ch->mana -= mana;
  }
  
  /* BEGIN UNDERWATER */
  if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
      act ("$n attempts to source lightning and gets shocked!", ch, NULL, NULL, TO_ROOM);
      send_to_char ("Your attempt to source lightning shocks you!\n\r", ch);
      dam = dice (level, 2);
      damage_old (ch, ch, adjust_damage(dam), sn, DAM_LIGHTNING, TRUE);
      return;
    }
  
  dam = 150;
  last_room = victim->in_room;
  act("A globe of swirling energy slams into $n",victim,NULL,NULL,TO_ROOM);
  act("A globe of swirling energy slams into you!",victim,NULL,NULL,TO_CHAR);
  if (saves_spell (level, victim, DAM_LIGHTNING))
    damage_old (ch, victim, adjust_damage(dam), sn, DAM_LIGHTNING, TRUE);
  else damage_old(ch, victim, adjust_damage(dam), sn, DAM_LIGHTNING, TRUE);
  //last_vict = victim;
  if (number_percent() > 90)
    num = 15;
  else num = 10;
  for (i = 0; i < num; ++i)
    {
      dam -= 15;                   /* decrement damage */
      if (dam < 10)
	dam = 10;
      for (door = number_range (0, 5), counter = 0; counter < 6;
           counter++, door++)
        {
          if (door > 5)
            door = door % 6;
          if ((pexit = last_room->exit[door]) == NULL
              || (to_room = pexit->u1.to_room) == NULL
	      || (IS_SET (pexit->exit_info, EX_CLOSED)))
	    continue;
	  if (!to_room->people)
	    continue;
	  total = 0;
	  for (rch = to_room->people;rch != NULL;rch = rch->next_in_room)
	    {
	    if (IS_NPC(rch) || rch->pcdata->loner)
	      {
		total = -1;
		break;
	      }
	    }
	  if (total != -1)
	    continue;
	  break;
        }
      if (counter > 5)
	{
	  to_room = last_room;
	}
      else
	{
	  if (last_room->people)
	    act("The erratic bolt bounces off $t.",last_room->people,dir_desc[door],NULL,TO_ALL);
	}
      total = 0;
      ptotal = 0;
      for (rch = to_room->people;rch != NULL;rch = rch->next_in_room)
	{
	  if (!IS_NPC(rch) && !rch->pcdata->loner)
	    continue;
	  ++total;
	  if (!IS_NPC(rch))
  	    ++ptotal;
	}
      if (total < 1)
	return;
      if (ptotal > 0)
        total = number_range(1,ptotal);
      else total = number_range(1,total);
      counter = 0;
      for (rch = to_room->people;rch != NULL;rch = rch->next_in_room)
        {
          if (!IS_NPC(rch) && !rch->pcdata->loner)
            continue;
	  if (ptotal > 0)
	  {
  	    if (!IS_NPC(rch))
	      ++counter;
	  }
	  else ++counter;
          if (counter == total)
	    {
	      last_room = rch->in_room;
	      act("A globe of swirling energy slams into $n",rch,NULL,NULL,TO_ROOM);
	      act("A globe of swirling energy slams into you!",rch,NULL,NULL,TO_CHAR);
	      if (saves_spell (level, rch, DAM_LIGHTNING))
		damage_old (ch, rch, adjust_damage(dam), sn, DAM_LIGHTNING, TRUE);
	      else damage_old(ch, rch, adjust_damage(dam), sn, DAM_LIGHTNING, TRUE);
	      //	      damage_old (ch, rch, dam, sn, DAM_LIGHTNING, TRUE);
	    }
        }
    }
}

void turn_into(CHAR_DATA *ch, int vnum_to_turn_into)
{
  CHAR_DATA *mob;
  char buf[MAX_STRING_LENGTH];
  if (get_mob_index(vnum_to_turn_into))
    mob = create_mobile(get_mob_index(vnum_to_turn_into));
  else return;
/*  mob->max_hit = 1;
  mob->hit = 1;*/
/*  mob->afkmsg = str_dup (ch->name);
  mob->max_move = ch->level;
  mob->move = ch->level;*/
  mob->ticks_remaining = ch->level/10;
  switch (vnum_to_turn_into)
  {
    case MOB_VNUM_FROG: mob->ticks_remaining = ch->perm_stat[STAT_INT]/2.0;break;
    case MOB_VNUM_WARPED_CAT: mob->ticks_remaining = 10;break;
    case MOB_VNUM_WARPED_OWL: mob->ticks_remaining = ch->perm_stat[STAT_WIS];break;
  }
  char_to_room(mob,ch->in_room);
  ch->desc->character = mob;
  ch->desc->original = ch;
  mob->desc = ch->desc;

  ch->desc = NULL;

  sprintf (buf, "$N switches into %s",
	   IS_NPC (mob) ? mob->short_descr : mob->name);
  wiznet (buf, ch, NULL, WIZ_SWITCHES, WIZ_SECURE, get_trust (ch));

  /* change communications to match */
  if (ch->prompt != NULL)
    mob->prompt = str_dup (ch->prompt);
  mob->comm = ch->comm;
  mob->lines = ch->lines;
  char_from_room(ch);
  char_to_room(ch,get_room_index(ROOM_VNUM_SWITCHED_LIMBO));
  SET_BIT(mob->act2,ACT_PUPPET);
//  SET_BIT(ch->act2,ACT_PUPPETEER);
  SET_BIT(ch->act2,ACT_SWITCHED);
}

void spell_frog (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  if (ch->in_room == NULL)
    return;
  send_to_char("Confused you look around you... Everything and everyone looks terribly big compared to you.\n\r",ch);
  act("$n disappears in a puff of purple smoke.",ch,NULL,NULL,TO_ROOM);
  act("As the smoke settles you notice a cute little frog.",ch,NULL,NULL,TO_ROOM);
  if (ch->desc)
  {
    send_to_char("You can now cast: ice beam.\n\r",ch->desc->character);
    send_to_char("You gain the immunity: summon.\n\r",ch->desc->character);
    send_to_char("You can now breath underwater.\n\r",ch->desc->character);
  }
  turn_into(ch,MOB_VNUM_FROG);
}

void spell_warped_cat (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  if (ch->in_room == NULL)
    return;
  send_to_char("The voices turn you into a cat!\n\r",ch);
  act("$n disappears in a puff of purple smoke.",ch,NULL,NULL,TO_ROOM);
  act("As the smoke settles a furless cat leaps out of sight.",ch,NULL,NULL,TO_ROOM);
  if (ch->desc)
  {
    send_to_char("You gain the skills: dodge, avoid traps.\n\r",ch->desc->character);
    send_to_char("You are affected by: sneak, invis, detect hidden.\n\r",ch->desc->character);
  }
  turn_into(ch,MOB_VNUM_WARPED_CAT);
}

void spell_warped_owl (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  if (ch->in_room == NULL)
    return;
  send_to_char("Hoooooooot are you...Hoot hoot..Hoot hoot.\n\r",ch);
  act("$n disappears in a puff of purple smoke.",ch,NULL,NULL,TO_ROOM);
  act("As the smoke settles you notice an ugly little owl sitting on the ground.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You gain the skills: swoop.\n\r",ch);
  send_to_char("You are affected by: flying, pass door, detect invis.\n\r",ch);
  turn_into(ch,MOB_VNUM_WARPED_OWL);
}

void switched_return(CHAR_DATA *mob, ROOM_INDEX_DATA *location)
{
  CHAR_DATA *ch;
  if (mob->desc == NULL)
    return;
  if (mob->desc->original == NULL)
    {
//      send_to_char ("You aren't switched.\n\r", mob);
      return;
    }
 
  if (!IS_SET(mob->desc->original->act2,ACT_SWITCHED))
	  return;
  
  //IF they die while void, I think
  if (location == NULL)
    location = get_room_index(ROOM_VNUM_ALTAR);
	  
  ch = mob->desc->original;
  /*  send_to_char
    ("You return to your original body. Type replay to see any missed tells.\n\r",
    ch);*/

  /*
  if (mob->prompt != NULL)
    {
      free_string (mob->prompt);
      mob->prompt = NULL;
    }
  sprintf (buf, "$N returns from %s.", mob->short_descr);
  wiznet (buf, mob->desc->original, 0, WIZ_SWITCHES, WIZ_SECURE,
          get_trust (mob));
  mob->desc->character = mob->desc->original;
  mob->desc->original = NULL;
  mob->desc->character->desc = mob->desc;
  mob->desc = NULL;*/
  act("The carcass of $n slowly transforms into the corpse of $N.",mob,NULL,ch,TO_ROOM);
  char_from_room(ch);
  char_to_room(ch,location);
  send_to_char("Your death in this form results in the death of your true form.\n\r",mob);
  raw_kill(ch,ch);
  do_return(mob,"");
  REMOVE_BIT(ch->act2,ACT_SWITCHED);
}
//  make_corpse(ch);
  

void spell_switch (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *location;
  victim = get_char_world (ch, target_name);

  if (target_name == NULL || target_name[0] == '\0')
  {
	  send_to_char("Switch with whom?\n\r",ch);
	  return;
  }
  /*if (ch->mana < 200)
    {
      send_to_char("You do not have enough mana.\n\r",ch);
      return;
    }
  ch->mana -= 200;*/

  // special case, if the mage tries to summon a character who is not
  // here then they automatically risk summoning a nasty
  if ((victim == NULL))
    {
      if (summon_goes_wrong (ch))
	{
          summon_nasty (ch);
          return;
        }
      
      else
      {
	send_to_char ("You failed.\n\r", ch);
	return;
      }
    }
  // the victim is a real char - lets be nice
  if ((victim->in_room == NULL) || (victim->level >= LEVEL_IMMORTAL))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }

  /*  if (ch->mana < 200)
    {
      send_to_char("You do not have enough mana.\n\r",ch);
      return;
    }
    ch->mana -= 200;*/
  /*  if (victim->mana < 200)
    {
      send_to_char("They do not have enough mana.\n\r",ch);
      return;
    }
    victim->mana -= 200;*/

  if (nogate || (!IS_NPC(ch) && ch->pcdata->nogate) || (!IS_NPC(victim) && victim->pcdata->nogate))
    {
      send_to_char("The gods are not allowing that at the moment.\n\r",ch);
      return;
    }


  // the victim is a real victim, but for various reasons the mage
  // might fail, in these cases check for failure and maybe summon_nasty
  if (IS_SET (victim->in_room->room_flags, ROOM_SAFE)
      || IS_SET (ch->in_room->room_flags,ROOM_SAFE)
      || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
      || IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL)
      || (victim->in_room->sector_type == SECT_SWAMP)
      || (ch->in_room->sector_type == SECT_SWAMP)
      || (ch->in_room->sector_type == SECT_WATER_OCEAN)
      || (victim->in_room->sector_type == SECT_WATER_OCEAN)
      || (victim->in_room == ch->in_room)
      || IS_NPC (victim)
      || IS_NPC (ch)
      || victim->fighting != NULL
      || (IS_SET(ch->in_room->area->area_flags,AREA_IMP_ONLY) && get_trust(victim) < MAX_LEVEL)
      || (victim->level < 8) 
      || !can_move_char(ch,victim->in_room,FALSE,FALSE)
      || !can_move_char(victim,ch->in_room,FALSE,FALSE)
      || (!(battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner)))
	  && (!IS_SET (victim->act, PLR_SWITCHOK))))
    /*    ||  (!(battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner)))
	  && !IS_SET (victim->act, PLR_SWITCHOK)))*/
    {
      if (summon_goes_wrong (ch))
        {
          summon_nasty (ch);
          return;
        }

      else
      {
          send_to_char ("You failed.\n\r", ch);
          return;
        }
    }
  // just fail if the victim is editing - just being nice
  if (victim->desc != NULL && victim->desc->pEdit != NULL)
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }


  if (IS_SET (ch->in_room->race_flags, ROOM_NOTRANSPORT))
    {
      act ("$N cannot enter this realm via ordinary magic.\n\r", ch,
           NULL, victim, TO_CHAR);
      return;
    }

  if (IS_SET (victim->in_room->race_flags, ROOM_NOTRANSPORT))
    {
      act ("You cannot enter that realm via ordinary magic.\n\r", ch,
           NULL, victim, TO_CHAR);
      return;
    }

  //This check is last so a CJ can't just cast this on someone to drain their mana if the switch
  // is not going to take place
  /*if (victim->mana < 200)
    {
      send_to_char("They do not have enough mana.\n\r",ch);
      return;
    }
  victim->mana -= 200;*/
  act("You decide to trade places with $N.",ch,NULL,victim,TO_CHAR);
  act("$n has decided to trade places with you.",ch,NULL,victim,TO_VICT);
  act("$n disappears in a puff of purple smoke.",ch,NULL,NULL,TO_ROOM);
  act("As the smoke settles $N is there instead.",ch,NULL,victim,TO_ROOM);
  act("$n disappears in a puff of purple smoke.",victim,NULL,NULL,TO_ROOM);
  act("As the smoke settles $N is there instead.",victim,NULL,ch,TO_ROOM);
  location = ch->in_room;
  char_from_room(ch);
  char_to_room(ch,victim->in_room);
  char_from_room(victim);
  char_to_room(victim,location);
  do_look (ch, "auto");
  do_look (victim, "auto");
  return;
}

void spell_evil_twin (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;
  if (costs_mana)
  {
    if (ch->mana < 100-ch->perm_stat[STAT_INT])
    {
	  send_to_char("You do not have enough mana.\n\r",ch);
	  return;
    }
    else ch->mana -= (100-ch->perm_stat[STAT_INT]);
  }
  victim = get_char_world (ch, target_name);
  if (victim == NULL || IS_NPC(victim) || ch == victim || is_affected (victim, gsn_evil_twin))
    /*      || saves_spell (level, victim, DAM_OTHER)
	    || IS_SET(victim->imm_flags, IMM_BLIND))*/
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  af.where = TO_AFFECTS;
  af.type = gsn_evil_twin;
  af.level = level;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.duration = 7;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup (ch->name);
  affect_to_char (victim, &af);
  act("$n mutters something about`a '`kI am $N`a'`` and nods to $mself.",ch,NULL,victim,TO_ROOM);
  act("You are $N, no matter what someone else might have to say about it.",ch,NULL,victim,TO_CHAR);
  //  act ("$n fumbles awkwardly, and appears to be blinded.", victim, NULL,
  //    NULL, TO_ROOM);
  return;
}
