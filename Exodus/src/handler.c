#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include <stdlib.h>
#include "recycle.h"
#include "tables.h"
#include "interp.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"
#include "obj_trig.h"
#include "olc.h"
#include "Druid.h"
#include "Utilities.h"

DECLARE_DO_FUN (do_return);
extern bool ch_is_quitting;
extern bool paddling;
void falcon_alert args ((CHAR_DATA * ch));
int flag_value args ((const struct flag_type * flag_table, char *argument));
bool IS_CLASS args((CHAR_DATA * ch, short Class));
inline bool is_affected args((CHAR_DATA * ch, int sn));
short level_for_skill_2 (CHAR_DATA * ch, short sn);
void save_player_corpse args((OBJ_DATA* corpse));
bool cardgame_surrender args((CHAR_DATA *ch, bool premature));
void check_gravity_room_obj args((CHAR_DATA *people,OBJ_DATA *obj));
void switched_return args((CHAR_DATA *ch, ROOM_INDEX_DATA *location));
void friend_poof args((CHAR_DATA *ch));
int hands_used args((CHAR_DATA *ch));
int hands_required args((CHAR_DATA *ch, OBJ_DATA *obj));
bool equip_char_slot_new args((CHAR_DATA * ch, OBJ_DATA * obj, long slot));

/*
 * Local functions.
 */
void affect_modify args ((CHAR_DATA * ch, AFFECT_DATA * paf, bool fAdd));
char *one_argument (char *argument, char *arg_first)
{
  char cEnd;
  while (isspace (*argument))
    argument++;
  cEnd = ' ';
  if (*argument == '\'' || *argument == '"')
    cEnd = *argument++;
  while (*argument != '\0')
    {
      if (*argument == cEnd)
	{
	  argument++;
	  break;
	}
      *arg_first = LOWER (*argument);
      arg_first++;
      argument++;
    }
  *arg_first = '\0';
  while (isspace (*argument))
    argument++;
  return argument;
}
char *one_argument_nl (char *argument, char *arg_first)
{
  char cEnd;
  while (isspace (*argument))
    argument++;
  cEnd = ' ';
  if (*argument == '\'' || *argument == '"')
    cEnd = *argument++;
  while (*argument != '\0')
    {
      if (*argument == cEnd)
	{
	  argument++;
	  break;
	}
      *arg_first = *argument;
      arg_first++;
      argument++;
    }
  *arg_first = '\0';
  while (isspace (*argument))
    argument++;
  return argument;
}


/* friend stuff -- for NPC's mostly */
bool is_friend (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (is_same_group (ch, victim))
    return TRUE;
  if (!IS_NPC (ch))
    return FALSE;
  if (!IS_NPC (victim))
    {
      if (IS_SET (ch->off_flags, ASSIST_PLAYERS))
	return TRUE;

      else
	return FALSE;
    }
  if (IS_AFFECTED (ch, AFF_CHARM))
    return FALSE;
  if (IS_SET (ch->off_flags, ASSIST_ALL))
    return TRUE;
  if (ch->group && ch->group == victim->group)
    return TRUE;
  if (IS_SET (ch->off_flags, ASSIST_VNUM)
      && ch->pIndexData == victim->pIndexData)
    return TRUE;
  if (IS_SET (ch->off_flags, ASSIST_RACE) && ch->race == victim->race)
    return TRUE;
  if (IS_SET (ch->off_flags, ASSIST_ALIGN)
      && ((IS_GOOD (ch) && IS_GOOD (victim))
	  || (IS_EVIL (ch) && IS_EVIL (victim))
	  || (IS_NEUTRAL (ch) && IS_NEUTRAL (victim))))
    return TRUE;
  return FALSE;
}


/* returns number of people on an object */
int count_users (OBJ_DATA * obj)
{
  CHAR_DATA *fch;
  int count = 0;
  if (obj->in_room == NULL)
    return 0;
  for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)
    if (fch->on == obj)
      count++;
  return count;
}


/* returns race number */
int race_lookup (const char *name)
{
  int race;
  for (race = 0; race_table[race].name != NULL; race++)
    {
      if (LOWER (name[0]) == LOWER (race_table[race].name[0])
	  && !str_prefix (name, race_table[race].name))
	return race;
    }
  return 0;
}

int orace_lookup (const char *name)
{
  int race;
  for (race = 1; race_table[race].name != NULL; race++)
    {
      if (LOWER (name[0]) == LOWER (race_table[race].name[0])
	  && !str_prefix (name, race_table[race].name))
	return race;
    }
  return 0;
}

int liq_lookup (const char *name)
{
  int liq;
  for (liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
      if (LOWER (name[0]) == LOWER (liq_table[liq].liq_name[0])
	  && !str_prefix (name, liq_table[liq].liq_name))
	return liq;
    }
  return -1;
}

int weapon_lookup (const char *name)
{
  int type;
  for (type = 0; weapon_table[type].name != NULL; type++)
    {
      if (LOWER (name[0]) == LOWER (weapon_table[type].name[0])
	  && !str_prefix (name, weapon_table[type].name))
	return type;
    }
  return -1;
}

int weapon_type (const char *name)
{
  int type;
  for (type = 0; weapon_table[type].name != NULL; type++)
    {
      if (LOWER (name[0]) == LOWER (weapon_table[type].name[0])
	  && !str_prefix (name, weapon_table[type].name))
	return weapon_table[type].type;
    }
  return WEAPON_EXOTIC;
}

int item_lookup (const char *name)
{
  int type;
  for (type = 0; item_table[type].name != NULL; type++)
    {
      if (LOWER (name[0]) == LOWER (item_table[type].name[0])
	  && !str_prefix (name, item_table[type].name))
	return item_table[type].type;
    }
  return -1;
}

char *item_name (int item_type)
{
  int type;
  for (type = 0; item_table[type].name != NULL; type++)
    if (item_type == item_table[type].type)
      return item_table[type].name;
  return "none";
}

char *weapon_name (int weapon_type)
{
  int type;
  for (type = 0; weapon_table[type].name != NULL; type++)
    if (weapon_type == weapon_table[type].type)
      return weapon_table[type].name;
  return "exotic";
}

int attack_lookup (const char *name)
{
  int att;
  for (att = 0; attack_table[att].name != NULL; att++)
    {
      if (LOWER (name[0]) == LOWER (attack_table[att].name[0])
	  && !str_prefix (name, attack_table[att].name))
	return att;
    }
  return 0;
}


/* returns a flag for wiznet */
long wiznet_lookup (const char *name)
{
  int flag;
  for (flag = 0; wiznet_table[flag].name != NULL; flag++)
    {
      if (LOWER (name[0]) == LOWER (wiznet_table[flag].name[0])
	  && !str_prefix (name, wiznet_table[flag].name))
	return flag;
    }
  return -1;
}


/* returns Class number */
int Class_lookup (const char *name)
{
  int Class;
  for (Class = 0; Class < MAX_CLASS; Class++)
    {
	if (Class_table[Class].name != NULL && Class_table[Class].name[0] != '\0')
	{
            if (LOWER (name[0]) == LOWER (Class_table[Class].name[0])
	         && !str_prefix (name, Class_table[Class].name))
	    return Class;
	}
    }
  return -1;
}

int hand_lookup (const char *name)
{
  int hand;
  for (hand = 0; hand < 3; hand++)
    {
      if (!str_cmp (name, "left"))
	return (HAND_LEFT);
      if (!str_cmp (name, "right"))
	return (HAND_RIGHT);
      if (!str_cmp (name, "ambidextrous"))
	return (HAND_AMBIDEXTROUS);
    }
  return -1;
}


/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */
int check_immune (CHAR_DATA * ch, int dam_type)
{
  int immune, def;
  int bit;
  immune = -1;
  def = IS_NORMAL;
  if (dam_type == DAM_NONE)
    return immune;
  if (dam_type == DAM_UNIQUE)
    return 0;
  if (dam_type <= 3)
    {
      if (IS_SET (ch->imm_flags, IMM_WEAPON))
	def = IS_IMMUNE;

      else if (IS_SET (ch->res_flags, RES_WEAPON))
	def = IS_RESISTANT;

      else if (IS_SET (ch->vuln_flags, VULN_WEAPON))
	def = IS_VULNERABLE;
    }

  else if (dam_type != DAM_VAMPIRIC)
    /* magical attack */
    {
      if (IS_SET (ch->imm_flags, IMM_MAGIC))
	def = IS_IMMUNE;

      else if (IS_SET (ch->res_flags, RES_MAGIC))
	def = IS_RESISTANT;

      else if (IS_SET (ch->vuln_flags, VULN_MAGIC))
	def = IS_VULNERABLE;
    }

  /* set bits to check -- VULN etc. must ALL be the same or this will fail */
  switch (dam_type)
    {
    case (DAM_BASH):
      bit = IMM_BASH;
      break;
    case (DAM_PIERCE):
      bit = IMM_PIERCE;
      break;
    case (DAM_SLASH):
      bit = IMM_SLASH;
      break;
    case (DAM_FIRE):
      bit = IMM_FIRE;
      break;
    case (DAM_COLD):
      bit = IMM_COLD;
      break;
    case (DAM_LIGHTNING):
      bit = IMM_LIGHTNING;
      break;
    case (DAM_ACID):
      bit = IMM_ACID;
      break;
    case (DAM_POISON):
      bit = IMM_POISON;
      break;
    case (DAM_NEGATIVE):
      bit = IMM_NEGATIVE;
      break;
    case (DAM_HOLY):
      bit = IMM_HOLY;
      break;
    case (DAM_ENERGY):
      bit = IMM_ENERGY;
      break;
    case (DAM_MENTAL):
      bit = IMM_MENTAL;
      break;
    case (DAM_DISEASE):
      bit = IMM_DISEASE;
      break;
    case (DAM_DROWNING):
      bit = IMM_DROWNING;
      break;
    case (DAM_LIGHT):
      bit = IMM_LIGHT;
      break;
    case (DAM_CHARM):
      bit = IMM_CHARM;
      break;
    case (DAM_SOUND):
      bit = IMM_SOUND;
      break;
    default:
      return def;
    }
  if (IS_SET (ch->imm_flags, bit))
    immune = IS_IMMUNE;

  else if (IS_SET (ch->res_flags, bit) && immune != IS_IMMUNE)
    immune = IS_RESISTANT;

  else if (IS_SET (ch->vuln_flags, bit))
    {
      if (immune == IS_IMMUNE)
	immune = IS_RESISTANT;

      else if (immune == IS_RESISTANT)
	immune = IS_NORMAL;

      else
	immune = IS_VULNERABLE;
    }
  if (immune == -1)
    return def;

 else return immune;
    
}

int get_char_weight (CHAR_DATA * ch)
{
  int weight;
  switch (ch->size)
    {
    case SIZE_TINY:
      weight = 500;
      break;
    case SIZE_SMALL:
      weight = 800;
      break;
    default:
    case SIZE_MEDIUM:
      weight = 1400;
      break;
    case SIZE_LARGE:
      weight = 1750;
      break;
    case SIZE_HUGE:
      weight = 2500;
      break;
    case SIZE_GIANT:
      weight = 4000;
      break;
    }
  weight += get_carry_weight (ch);
  return (weight);
}


/* for returning skill information */
int get_skill (CHAR_DATA * ch, int sn)
{
  int skill=0;
//  char buf[30];
  if (ch->stunned)
    return 0;
  if (sn == -1)
    {				/* shorthand for level based skills */
      skill = ch->level * 5 / 2;
    }

  else if (sn < -1 || sn > MAX_SKILL)
    {
      bug ("Bad sn %d in get_skill.", sn);
      skill = 0;
    }

  else if (!IS_NPC (ch))
    {
      if (ch->level < level_for_skill (ch, sn))
	skill = 0;
      else if (sn == gsn_swim)
      {
        if ((get_carry_weight (ch) / 10) > (can_carry_w (ch) / 10))
          skill = 0;
	else if (ch->race == PC_RACE_CANTHI || ch->race == PC_RACE_NIDAE)
		skill = 100;
	else if (ch->race == PC_RACE_DWARF)
		skill = 0;
	else skill = ch->pcdata->learned[sn];
      }
      else      
	skill = ch->pcdata->learned[sn];
      skill += ch->pcdata->mod_learned[sn];
      if (sn == gsn_dual_wield && skill > 0 && ch->race == PC_RACE_NERIX)
        skill = 100;
      if (sn == gsn_steal && ch->Class == PC_CLASS_CHAOS_JESTER)
        skill = 50;
        
      
    }

  else
    /* mobiles */
    {
      //Iblis - if mob is an obj trigger mob, they have 100% in ALL skills
      if (ch->pIndexData->vnum == MOB_VNUM_FAKIE)
       return 100;
      if (IS_SET(ch->act2,ACT_ALL_SKILLS)) 
      {
	if (!(sn == gsn_dodge || sn == gsn_flicker || sn == gsn_parry || sn == gsn_conceal
		|| sn == gsn_prevent_escape || sn == gsn_awareness || sn == gsn_hear_the_wind
		|| sn == gsn_dirty_tactics || sn == gsn_counter || sn == gsn_feed_death))
 	  return 100;
      }

	// Hunyuk -- deal with Druid mobs here
	if (is_shapechanged_druid(ch)) {
		if (ch->pIndexData->vnum == MOB_VNUM_SHAPECHANGE_MOUSE) {
			if (sn == gsn_hide)
				return 100;
			else if (sn == gsn_sneak)
				return 100;
			else
				return 0;
		}

		if (ch->pIndexData->vnum == MOB_VNUM_SHAPECHANGE_RAVEN) {
			if (sn == gsn_swoop)
				return 100;
			else
				return 0;
		}

		if (ch->pIndexData->vnum == MOB_VNUM_SHAPECHANGE_WOLF) {
			if (sn == gsn_track)
				return 100;
			else
				return 0;
		}

		if (ch->pIndexData->vnum == MOB_VNUM_SHAPECHANGE_BEAR) {
			if (sn == gsn_bash)
				return 100;
			else
				return 0;
		}
	}

      //Iblis - if mob is a CJ created mob, they only have the hardcoded skills below
      if (IS_SET(ch->act2,ACT_PUPPET))
      {
        if (ch->pIndexData->vnum == MOB_VNUM_FROG)
	{
	  if (sn == skill_lookup("ice beam"))
  	    return 100;
	  else return 0;
	}
	if (ch->pIndexData->vnum == MOB_VNUM_WARPED_CAT)
	{
	  if (sn == gsn_dodge)
	    return 100;
	  else if (sn == gsn_avoid_traps)
 	    return 100;
	  else return 0;
	}
	if (ch->pIndexData->vnum == MOB_VNUM_WARPED_OWL)
	{
	  if (sn == gsn_swoop)
	    return 100;
	  else return 0;
	}
      }
	  
      //Iblis - if mob is a necromancer familiar, they only have the hardcoded skills below
      //at 100 (if they are high enough level), otherwise they have the skill at 0%
      if (IS_SET(ch->act2,ACT_FAMILIAR))
	{
	  if (ch->pIndexData->vnum == MOB_VNUM_FAM_ZOMBIE)
	    {
	      if (sn == gsn_kick && ch->level >= 10)
		return 100;
	      else if (sn == gsn_mace)
  	        return 100;
	      else if (sn == gsn_flail)
  	        return 100;
	      else if (sn == gsn_berserk && ch->level >= 30)
		return 100;
	      else if (sn == gsn_parry && ch->level >= 39)
		return 100;
	      else if (sn == gsn_uppercut && ch->level >= 40)
		return 100;
	      else if (sn == gsn_fast_healing && ch->level >= 50)
		return 100;
	      else if (sn == gsn_second_attack && ch->level >= 50)
                return 100;
              else if (sn == gsn_enhanced_damage && ch->level >= 62)
                return 100;
	      else if (sn == gsn_circle && ch->level >= 55)
		return 100;
	      else if (sn == gsn_plague && ch->level >= 60)
		return 100;
	      else if (sn == gsn_fear && ch->level >= 80)
		return 100;
	      else return 0;
	    }
	  if (ch->pIndexData->vnum == MOB_VNUM_FAM_SKELETON)
            {
              if (sn == gsn_throwing && ch->level >= 30)
                return 100;
	      else if (sn == gsn_sword || sn == gsn_axe)
  	        return 100;
              else if (sn == gsn_shield_block && ch->level >= 35)
                return 100;
              else if (sn == gsn_berserk && ch->level >= 40)
                return 100;
              else if (sn == gsn_rescue && ch->level >= 50)
                return 100;
	      else if (sn == gsn_second_attack && ch->level >= 50)
                return 100;
	      else if (sn == gsn_enhanced_damage && ch->level >= 62)
                return 100;
              else if (sn == gsn_hellbreath && ch->level >= 80)
                return 100;
              else return 0;
            }
	  if (ch->pIndexData->vnum == MOB_VNUM_FAM_BANSHEE)
            {
              if (sn == gsn_hand_to_hand)
                return 100;
              else if (sn == gsn_fisticuffery && ch->level >= 30)
                return 100;
	      else if (sn == gsn_second_attack && ch->level >= 50)
                return 100;
              else if (sn == gsn_enhanced_damage && ch->level >= 62)
                return 100;
              else if (sn == gsn_dual_wield && ch->level >= 80)
                return 100;
	      else if (sn == gsn_kai && ch->level >= 40)
                return 100;
              else if (sn == skill_lookup("Ballad of the Cantankerous Woodsman") && ch->level >= 28)
                return 100;
              else if (sn == skill_lookup("Falkentynes Fury") && ch->level >= 13)
                return 100;
              else return 0;
            }
	  if (ch->pIndexData->vnum == MOB_VNUM_IMAGINARY_FRIEND)
	  {
	    if (sn == gsn_sword && ch->level >= 10)
   	      return 100;
	    else if (sn == gsn_flail && ch->level >= 25)
  	      return 100;
	    else if (sn == gsn_enhanced_damage && ch->level >= 30)
	      return 100;
	    else if (sn == gsn_dagger && ch->level >= 45)
	      return 100;
	    else if (sn == gsn_redirect && ch->level >= 50)
   	      return 100;
	    else if (sn == gsn_dirty_tactics && ch->level >= 75)
 	      return 100;
	    else if (sn == gsn_pick_lock && ch->level >= 90)
 	      return 100;
	    else return 0;
	  }
	  return 0;
	}


      if (skill_table[sn].spell_fun != spell_null)
	skill = 40 + 2 * ch->level;

      else if (sn == gsn_sneak || sn == gsn_hide)
	skill = ch->level * 2 + 20;

      else if ((sn == gsn_dodge && IS_SET (ch->off_flags, OFF_DODGE))
	       || (sn == gsn_parry && IS_SET (ch->off_flags, OFF_PARRY)))
	skill = ch->level * 2;

      else if (sn == gsn_shield_block)
	skill = 10 + 2 * ch->level;

      else if (sn == gsn_second_attack && (IS_SET (ch->act, ACT_WARRIOR)
					   || IS_SET (ch->act, ACT_THIEF)))
	skill = 10 + 3 * ch->level;

      else if (sn == gsn_third_attack && IS_SET (ch->act, ACT_WARRIOR))
	skill = 4 * ch->level - 40;

      else if (sn == gsn_hand_to_hand)
	skill = 40 + 2 * ch->level;

      else if (sn == gsn_trip && IS_SET (ch->off_flags, OFF_TRIP))
	skill = 10 + 3 * ch->level;

      else if (sn == gsn_bash && IS_SET (ch->off_flags, OFF_BASH))
	skill = 10 + 3 * ch->level;

      else if (sn == gsn_counter && IS_SET (ch->off_flags, OFF_COUNTER))
        skill = 10 + 3 * ch->level;

      else if (sn == gsn_disarm && (IS_SET (ch->off_flags, OFF_DISARM)
				    || IS_SET (ch->act, ACT_WARRIOR)
				    || IS_SET (ch->act, ACT_THIEF)))
	skill = 20 + 3 * ch->level;

      else if (sn == gsn_berserk && IS_SET (ch->off_flags, OFF_BERSERK))
	skill = 3 * ch->level;

      else if (sn == gsn_kick)
	skill = 10 + 3 * ch->level;

      else if (sn == gsn_backstab && IS_SET (ch->act, ACT_THIEF))
	skill = 20 + 2 * ch->level;
	

      else if (sn == gsn_rescue)
	skill = 40 + ch->level;

      else if (sn == gsn_sword
	       || sn == gsn_dagger
	       || sn == gsn_staff
	       || sn == gsn_dice
	       || sn == gsn_mace
	       || sn == gsn_axe || sn == gsn_flail || sn == gsn_polearm)
	skill = 40 + 5 * ch->level / 2;

      //Iblis - hardcoded skill %'s for mobs set ACT_MONK
      else if (sn == gsn_critical_strike && IS_SET (ch->act2, ACT_MONK))
	      skill = 40 + ch->level;

      else if (sn == gsn_counter && IS_SET (ch->act2, ACT_MONK))
	      skill = 40 + ch->level;

      else if (sn == gsn_dive && IS_SET (ch->act2, ACT_MONK))
	      skill = 40 + ch->level;
      
      else if (sn == gsn_prevent_escape && IS_SET (ch->act2, ACT_MONK))
	       skill = 40 + ch->level;

      else if (sn == gsn_hear_the_wind && IS_SET (ch->act2, ACT_MONK))
               skill = 40 + ch->level;

      else if (sn == gsn_strike && IS_SET (ch->act2, ACT_MONK))
               skill = 40 + ch->level;

      else if (sn == gsn_awareness && IS_SET (ch->act2, ACT_MONK))
               skill = 40 + ch->level;

      else if (sn == gsn_dirty_tactics && (IS_SET (ch->act2, ACT_MONK) || IS_SET (ch->act2, ACT_THIEF)))
               skill = 40 + ch->level;

      else if (sn == gsn_whirlwind && IS_SET (ch->act2, ACT_MONK))
               skill = 40 + ch->level;

      else if (sn == gsn_fists_of_fury && IS_SET (ch->act2, ACT_MONK))
               skill = 40 + ch->level;
     
      //Iblis - hardcoded skill %'s for mobs set ACT_MAGE
      else if (IS_SET (ch->act, ACT_MAGE) && (sn == skill_lookup("dispel magic")
			      || sn == skill_lookup("blindness")
			      || sn == skill_lookup("weaken")
			      || sn == skill_lookup("curse")
			      || sn == skill_lookup("colour spray")
			      || sn == skill_lookup("magic missle")
			      || sn == skill_lookup("acid blast")
			      || sn == skill_lookup("energy drain")
			      || sn == skill_lookup("heat metal")
			      || sn == skill_lookup("web")
			      || sn == skill_lookup("firestorm")
			      || sn == skill_lookup("fireball")))
	      skill = 40 + ch->level;
      else if (IS_SET (ch->act2, ACT_NECROMANCER) && (sn >= skill_lookup("death palm")) 
		      			  && (sn <= skill_lookup("unsummon")))
	      skill = 40 + ch->level;
      else if (IS_SET (ch->act2, ACT_MAGE_SKILLS) && (sn == gsn_staves
			      ||  sn == gsn_brew ||  sn == gsn_scrolls
			      ||  sn == gsn_wands ||  sn == gsn_scribe))
	      skill = 40 + ch->level;
      else if (sn == gsn_charge && IS_SET (ch->off_flags,OFF_CHARGE))
	      skill = 40 + ch->level;
      else if (sn == gsn_sweep && IS_SET (ch->off_flags,OFF_SWEEP))
	      skill = 40 + ch->level;
      else if (sn == gsn_stun && IS_SET (ch->off_flags,OFF_STUN))
	      skill = 40 + ch->level;
      else if (sn == gsn_kai && IS_SET (ch->off_flags,OFF_KAI))
	      skill = 40 + ch->level;
      else if (sn == gsn_hellbreath && IS_SET (ch->off_flags,OFF_HELLBREATH))
	      skill = 10 + 3 * ch->level;

      else
	skill = 0;		//Shinowlan 03/26/2002 -- added to support mob scripts.
      //this way mobs can use all skills as needed.
      //Shinowlan 06/25/2002 - set back to the standard level of zero, was causing crash bugs (kai)
    }
  if (ch->daze > 0)
    {
      if (skill_table[sn].spell_fun != spell_null)
	skill /= 2;

      else
	skill = 2 * skill / 3;
    }
  if (!IS_NPC (ch) && (ch->pcdata->condition[COND_DRUNK] > 10))
    skill = 9 * skill / 10;
  return URANGE (0, skill, 100);
}


/* for returning weapon information */
int get_weapon_sn (CHAR_DATA * ch, int wield_type)
{
  OBJ_DATA *wield;
  int sn;
  wield = get_eq_char (ch, wield_type);
  if (wield == NULL || wield->item_type != ITEM_WEAPON)
    sn = gsn_hand_to_hand;

  else
    switch (wield->value[0])
      {
      default:
	sn = -1;
	break;
      case (WEAPON_SWORD):
	sn = gsn_sword;
	break;
      case (WEAPON_DAGGER):
	sn = gsn_dagger;
	break;
      case (WEAPON_STAFF):
	sn = gsn_staff;
	break;
      case (WEAPON_MACE):
	sn = gsn_mace;
	break;
      case (WEAPON_AXE):
	sn = gsn_axe;
	break;
      case (WEAPON_FLAIL):
	sn = gsn_flail;
	break;
      case (WEAPON_POLEARM):
	sn = gsn_polearm;
	break;
      case (WEAPON_DICE):
	sn = gsn_dice;
	break;
      }
  return sn;
}

int get_weapon_skill (CHAR_DATA * ch, int sn)
{
  int skill;

  /* -1 is exotic */
  if (IS_NPC (ch))
    {
      if (sn == -1)
	skill = 3 * ch->level;
/*  Using get_skill so familiar work properly 
      else if (sn == gsn_hand_to_hand)
	skill = 40 + 2 * ch->level;

      else
	skill = 40 + 5 * ch->level / 2;
	*/
      else skill = get_skill(ch,sn);
    }

  else
    {
      if (sn == -1)
	skill = 3 * ch->level;

      else
	skill = ch->pcdata->learned[sn];
    }
  return URANGE (0, skill, 100);
}


/* used to de-screw characters */
void reset_char (CHAR_DATA * ch)
{
  int loc, mod, stat;
  OBJ_DATA *obj;
  AFFECT_DATA *af;
  int i,k;
  long wear_flags_left=0;
  if (IS_NPC (ch))
    return;
  if (ch->pcdata->perm_hit == 0
      || ch->pcdata->perm_mana == 0
      || ch->pcdata->perm_move == 0 || ch->pcdata->last_level == 0)
    {

      /* do a FULL reset */
//      for (loc = 0; loc < MAX_WEAR; loc++)
//	{
      for (k = 0;wear_conversion_table[k].wear_loc != -2;++k)
      {
        if (wear_conversion_table[k].wear_flag == ITEM_TAKE || wear_conversion_table[k].wear_flag == ITEM_NO_SAC)
          continue;
        if (!IS_SET(wear_flags_left,wear_conversion_table[k].wear_flag))
          continue;
        loc = wear_conversion_table[k].wear_loc;
        obj = get_eq_char (ch, loc);
        if (obj == NULL)
          continue;
        REMOVE_BIT(wear_flags_left,obj->wear_loc);
//	  obj = get_eq_char (ch, loc);
//	  if (obj == NULL)
//	    continue;
	  if (!obj->enchanted)
	    for (af = obj->pIndexData->affected; af != NULL; af = af->next)
	      {
		mod = af->modifier;
		switch (af->location)
		  {
		  case APPLY_SEX:
		    ch->sex -= mod;
		    if (ch->sex < 0 || ch->sex > 2)
		      ch->sex = IS_NPC (ch) ? 0 : ch->pcdata->true_sex;
		    break;
		  case APPLY_MANA:
		    ch->max_mana -= mod;
		    break;
		  case APPLY_HIT:
		    ch->max_hit -= mod;
		    break;
		  case APPLY_MOVE:
		    ch->max_move -= mod;
		    break;
		  }
	      }
	  for (af = obj->affected; af != NULL; af = af->next)
	    {
	      mod = af->modifier;
	      switch (af->location)
		{
		case APPLY_SEX:
		  ch->sex -= mod;
		  break;
		case APPLY_MANA:
		  ch->max_mana -= mod;
		  break;
		case APPLY_HIT:
		  ch->max_hit -= mod;
		  break;
		case APPLY_MOVE:
		  ch->max_move -= mod;
		  break;
		}
	    }
	}

      /* now reset the permanent stats */
      ch->pcdata->perm_hit = ch->max_hit;
      ch->pcdata->perm_mana = ch->max_mana;
      ch->pcdata->perm_move = ch->max_move;
      ch->pcdata->last_level = ch->played / 3600;
      if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
	{
	  if (ch->sex > 0 && ch->sex < 3)
	    ch->pcdata->true_sex = ch->sex;

	  else
	    ch->pcdata->true_sex = 0;
	}
    }

  /* now restore the character to his/her true condition */
  for (stat = 0; stat < MAX_STATS; stat++)
    ch->mod_stat[stat] = 0;
  if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
    ch->pcdata->true_sex = 0;
  ch->sex = ch->pcdata->true_sex;
  if (!is_affected(ch,gsn_scramble))
  {
    ch->max_hit = ch->pcdata->perm_hit;
    ch->max_mana = ch->pcdata->perm_mana;
    ch->max_move = ch->pcdata->perm_move;
  }
  for (i = 0; i < 4; i++)
    ch->armor[i] = 100;
  ch->hitroll = 0;
  ch->damroll = 0;
  ch->saving_throw = 0;

  for (i = 1; wear_conversion_table[i].wear_loc != -2;++i)
    wear_flags_left |= wear_conversion_table[i].wear_flag;
  /* now start adding back the effects */
//  for (loc = 0; loc < MAX_WEAR; loc++)
  for (k = 0;wear_conversion_table[k].wear_loc != -2;++k)
    {
      if (wear_conversion_table[k].wear_flag == ITEM_TAKE || wear_conversion_table[k].wear_flag == ITEM_NO_SAC)
        continue;
      if (!IS_SET(wear_flags_left,wear_conversion_table[k].wear_flag))
        continue;
      loc = wear_conversion_table[k].wear_loc;
      obj = get_eq_char (ch, loc);
      if (obj == NULL)
	continue;
      REMOVE_BIT(wear_flags_left,obj->wear_loc);
      for (i = 0; i < 4; i++)
	ch->armor[i] -= apply_ac_new (obj, i);
      if (!obj->enchanted)
	for (af = obj->pIndexData->affected; af != NULL; af = af->next)
	  {
	    mod = af->modifier;
	    switch (af->location)
	      {
	      case APPLY_STR:
		ch->mod_stat[STAT_STR] += mod;
		break;
	      case APPLY_DEX:
		ch->mod_stat[STAT_DEX] += mod;
		break;
	      case APPLY_INT:
		ch->mod_stat[STAT_INT] += mod;
		break;
	      case APPLY_WIS:
		ch->mod_stat[STAT_WIS] += mod;
		break;
	      case APPLY_CON:
		ch->mod_stat[STAT_CON] += mod;
		break;
	      case APPLY_CHA:
		ch->mod_stat[STAT_CHA] += mod;
		break;
	      case APPLY_SEX:
		ch->sex += mod;
		break;
	      case APPLY_MANA:
		ch->max_mana += mod;
		break;
	      case APPLY_HIT:
		ch->max_hit += mod;
		break;
	      case APPLY_MOVE:
		ch->max_move += mod;
		break;
	      case APPLY_AC:
		for (i = 0; i < 4; i++)
		  ch->armor[i] += mod;
		break;
	      case APPLY_HITROLL:
		ch->hitroll += mod;
		break;
	      case APPLY_DAMROLL:
		ch->damroll += mod;
		break;
	      case APPLY_SAVES:
		ch->saving_throw += mod;
		break;
	      case APPLY_SAVING_ROD:
		ch->saving_throw += mod;
		break;
	      case APPLY_SAVING_PETRI:
		ch->saving_throw += mod;
		break;
	      case APPLY_SAVING_BREATH:
		ch->saving_throw += mod;
		break;
	      case APPLY_SAVING_SPELL:
		ch->saving_throw += mod;
		break;
	      }
	  }
      for (af = obj->affected; af != NULL; af = af->next)
	{
	  mod = af->modifier;
	  switch (af->location)
	    {
	    case APPLY_STR:
	      ch->mod_stat[STAT_STR] += mod;
	      break;
	    case APPLY_DEX:
	      ch->mod_stat[STAT_DEX] += mod;
	      break;
	    case APPLY_INT:
	      ch->mod_stat[STAT_INT] += mod;
	      break;
	    case APPLY_WIS:
	      ch->mod_stat[STAT_WIS] += mod;
	      break;
	    case APPLY_CON:
	      ch->mod_stat[STAT_CON] += mod;
	      break;
	    case APPLY_CHA:
	      ch->mod_stat[STAT_CHA] += mod;
	      break;
	    case APPLY_SEX:
	      ch->sex += mod;
	      break;
	    case APPLY_MANA:
	      ch->max_mana += mod;
	      break;
	    case APPLY_HIT:
	      ch->max_hit += mod;
	      break;
	    case APPLY_MOVE:
	      ch->max_move += mod;
	      break;
	    case APPLY_AC:
	      for (i = 0; i < 4; i++)
		ch->armor[i] += mod;
	      break;
	    case APPLY_HITROLL:
	      ch->hitroll += mod;
	      break;
	    case APPLY_DAMROLL:
	      ch->damroll += mod;
	      break;
	    case APPLY_SAVES:
	      ch->saving_throw += mod;
	      break;
	    case APPLY_SAVING_ROD:
	      ch->saving_throw += mod;
	      break;
	    case APPLY_SAVING_PETRI:
	      ch->saving_throw += mod;
	      break;
	    case APPLY_SAVING_BREATH:
	      ch->saving_throw += mod;
	      break;
	    case APPLY_SAVING_SPELL:
	      ch->saving_throw += mod;
	      break;
	    }
	}
    }

  /* now add back spell effects */
  for (af = ch->affected; af != NULL; af = af->next)
    {
      mod = af->modifier;
      switch (af->location)
	{
	case APPLY_STR:
	  ch->mod_stat[STAT_STR] += mod;
	  break;
	case APPLY_DEX:
	  ch->mod_stat[STAT_DEX] += mod;
	  break;
	case APPLY_INT:
	  ch->mod_stat[STAT_INT] += mod;
	  break;
	case APPLY_WIS:
	  ch->mod_stat[STAT_WIS] += mod;
	  break;
	case APPLY_CON:
	  ch->mod_stat[STAT_CON] += mod;
	  break;
	case APPLY_CHA:
	  ch->mod_stat[STAT_CHA] += mod;
	  break;
	case APPLY_SEX:
	  ch->sex += mod;
	  break;
	case APPLY_MANA:
	  ch->max_mana += mod;
	  break;
	case APPLY_HIT:
	  ch->max_hit += mod;
	  break;
	case APPLY_MOVE:
	  ch->max_move += mod;
	  break;
	case APPLY_AC:
	  for (i = 0; i < 4; i++)
	    ch->armor[i] += mod;
	  break;
	case APPLY_HITROLL:
	  ch->hitroll += mod;
	  break;
	case APPLY_DAMROLL:
	  ch->damroll += mod;
	  break;
	case APPLY_SAVES:
	  ch->saving_throw += mod;
	  break;
	case APPLY_SAVING_ROD:
	  ch->saving_throw += mod;
	  break;
	case APPLY_SAVING_PETRI:
	  ch->saving_throw += mod;
	  break;
	case APPLY_SAVING_BREATH:
	  ch->saving_throw += mod;
	  break;
	case APPLY_SAVING_SPELL:
	  ch->saving_throw += mod;
	  break;
	}
    }

  /* make sure sex is RIGHT!!!! */
  if (ch->sex < 0 || ch->sex > 2)
    ch->sex = ch->pcdata->true_sex;
}


/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust (CHAR_DATA * ch)
{
  if (ch == NULL)
    {
//      bug ("NULL ch in get_trust", 0);
      return 0;
    }
  if (ch->desc != NULL && ch->desc->original != NULL)
    ch = ch->desc->original;
  if (ch == NULL)
  {
	  bug ("NULL ch in get_trust (2)", 0);
	  return 0;
  }
  if (ch->trust != 0 && IS_SET (ch->comm, COMM_TRUE_TRUST))
    return ch->trust;
  if (IS_NPC (ch) && ch->level >= LEVEL_HERO)
    return LEVEL_HERO - 1;

  else
    return ch->level;
}


/*
 * Retrieve a character's age.
 */
int get_age (CHAR_DATA * ch)
{
  if (!IS_NPC(ch) && ch->race == PC_RACE_NERIX)
  {
    return 1 + ((ch->played + (int) (current_time - ch->logon)) - ch->pcdata->nplayed)/36000;
  }
  else return 17 + (ch->played + (int) (current_time - ch->logon)) / 72000;
}

/* command for retrieving stats */
int get_curr_stat (CHAR_DATA * ch, int stat)
{
  int max;
  if (IS_NPC (ch) || ch->level > LEVEL_IMMORTAL)
    max = 25;

  else
    {

//IBLIS 5/19/03 - Changed so the max is the max trainable, 
//or 3 + natural stat, whichever is greater
      if (ch->race == race_lookup ("avatar"))
	max = 25;
      max = pc_race_table[ch->race].max_stats[stat];	// + 4;
      if (Class_table[ch->Class].attr_prime == stat)
	max += 2;

      //if (ch->race == race_lookup ("human"))
      //max += 1;
      max = UMAX (max, ch->perm_stat[stat] + 3);
      max = UMIN (max, 25);
    }
  return URANGE (3, ch->perm_stat[stat] + ch->mod_stat[stat], max);
}


/* command for returning max training score */
int get_max_train (CHAR_DATA * ch, int stat)
{
  int max;
  if (IS_NPC (ch) || ch->level > LEVEL_IMMORTAL)
    return 25;
  if (ch->race == race_lookup ("avatar"))
    max = 25;
  max = pc_race_table[ch->race].max_stats[stat];
  if (Class_table[ch->Class].attr_prime == stat)
    {
      if (ch->race == race_lookup ("human"))
	max += 3;

      else
	max += 2;
    }
  return UMIN (max, 25);
}


/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n (CHAR_DATA * ch)
{
  if (!IS_NPC (ch) && ch->level >= LEVEL_IMMORTAL)
    return 1000;

  /*  if (IS_NPC(ch) && IS_SET(ch->act, ACT_PET))
     return 0; */
  return MAX_WEAR + 2 * get_curr_stat (ch, STAT_DEX) + ch->level;
}


/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w (CHAR_DATA * ch)
{
  if (!IS_NPC (ch) && ch->level >= LEVEL_IMMORTAL)
    return 10000000;

  /*  if (IS_NPC(ch) && IS_SET(ch->act, ACT_PET))
     return 0; */
  return str_app[get_curr_stat (ch, STAT_STR)].carry * 10 + ch->level * 25;
}


/*
 * See if a string is one of the names of an object.
 */
bool is_name (char *str, char *namelist)
{
  char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
  char *list, *string;

  /* fix crash on NULL namelist */
  if (namelist == NULL || namelist[0] == '\0')
    return FALSE;

  string = str;

  /* we need ALL parts of string to match part of namelist */
  for (;;)
    {				/* start parsing string */
      str = one_argument (str, part);
      if (part[0] == '\0')
	return TRUE;

      /* check to see if this is part of namelist */
      list = namelist;
      for (;;)
	{			/* start parsing namelist */
	  list = one_argument (list, name);
	  if (name[0] == '\0')	/* this name was not found */
	    return FALSE;
	  if (!str_prefix (string, name))
	    return TRUE;	/* full pattern match */
	  if (!str_prefix (part, name))
	    break;
	}
    }
}

bool is_name_no_abbrev (char *str, char *namelist)
{
  char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH], arg[MAX_INPUT_LENGTH];
  char *list, *string;
  strcpy (buf, str);
  one_argument (buf, arg);
  if (arg[0] == '\0')
    return (FALSE);
  string = str;

  /* we need ALL parts of string to match part of namelist */
  for (;;)
    {				/* start parsing string */
      str = one_argument (str, part);
      if (part[0] == '\0')
	return TRUE;

      /* check to see if this is part of namelist */
      list = namelist;
      for (;;)
	{			/* start parsing namelist */
	  list = one_argument (list, name);
	  if (name[0] == '\0')	/* this name was not found */
	    return FALSE;
	  if (!str_cmp (string, name))
	    return TRUE;	/* full pattern match */
	  if (!str_cmp (part, name))
	    break;
	}
    }
}


/* enchanted stuff for eq */
void affect_enchant (OBJ_DATA * obj)
{

  /* okay, move all the old flags into new vectors if we have to */
  if (!obj->enchanted)
    {
      AFFECT_DATA *paf, *af_new;
      obj->enchanted = TRUE;
      for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
	{
	  af_new = new_affect ();
	  af_new->next = obj->affected;
	  obj->affected = af_new;
	  af_new->where = paf->where;
	  af_new->type = UMAX (0, paf->type);
	  af_new->level = paf->level;
	  af_new->duration = paf->duration;
	  af_new->location = paf->location;
	  af_new->modifier = paf->modifier;
	  af_new->bitvector = paf->bitvector;
	  af_new->composition = paf->composition;
	  af_new->comp_name = str_dup (paf->comp_name);
	}
    }
}


/*
 * Apply or remove an affect to a character.
 */
void affect_modify (CHAR_DATA * ch, AFFECT_DATA * paf, bool fAdd)
{
  OBJ_DATA *wield;
  int mod, i,sn;
  mod = paf->modifier;
  if (fAdd)
    {
      switch (paf->where)
	{
	case TO_AFFECTS:
	  SET_BIT (ch->affected_by, paf->bitvector);
	  break;
	case TO_IMMUNE:
	  SET_BIT (ch->imm_flags, paf->bitvector);
	  break;
	case TO_RESIST:
	  SET_BIT (ch->res_flags, paf->bitvector);
	  break;
	case TO_VULN:
	  SET_BIT (ch->vuln_flags, paf->bitvector);
	  break;
	case TO_ACT2:
	  SET_BIT (ch->act2, paf->bitvector);
	  
	  break;
	  
	//Iblis - below added for skill and equipment affects that modify a character's skill %
	case TO_SKILL:
	  if (!IS_NPC(ch))
	  {
	    if (paf->location != 0 && skill_table[paf->location].name != NULL)
	    {
	      ch->pcdata->mod_learned[paf->location] += paf->modifier;
	    }
	    else if (paf->location == 0)
	    {
	      for (sn = 0; sn < MAX_SKILL; sn++)
	      {
	        if (skill_table[sn].name != NULL) 
                {
		  ch->pcdata->mod_learned[sn] += paf->modifier;
		}
	      }
	    }
	  }
	  return;
		  
	}
    }

  else
    {
      switch (paf->where)
	{
	case TO_AFFECTS:
	  REMOVE_BIT (ch->affected_by, paf->bitvector);
	  break;
	case TO_IMMUNE:
	  REMOVE_BIT (ch->imm_flags, paf->bitvector);
	  break;
	case TO_RESIST:
	  REMOVE_BIT (ch->res_flags, paf->bitvector);
	  break;
	case TO_VULN:
	  REMOVE_BIT (ch->vuln_flags, paf->bitvector);
	  break;
	case TO_ACT2:
	  REMOVE_BIT (ch->act2, paf->bitvector);
	  if (!IS_NPC(ch) && paf->bitvector == ACT_WARPED)
 	    ch->res_flags = race_table[ch->race].res;
	  break;
	//Iblis - below added for skill and equipment affects that modify a character's skill %
	case TO_SKILL:
          if (!IS_NPC(ch))
	    {
	      if (paf->location != 0 && skill_table[paf->location].name != NULL)
		{
		  ch->pcdata->mod_learned[paf->location] -= paf->modifier;
		}
	      else if (paf->location == 0)
		{
		  for (sn = 0; sn < MAX_SKILL; sn++)
		    {
		      if (skill_table[sn].name != NULL)
			{
			  ch->pcdata->mod_learned[sn] -= paf->modifier;
			}
		    }
		}
	    }
	  return;
	}
      mod = 0 - mod;
    }
  switch (paf->location)
    {
    default:
      bug ("Affect_modify: unknown location %d.", paf->location);
      return;
    case APPLY_NONE:
      break;
    case APPLY_STR:
      ch->mod_stat[STAT_STR] += mod;
      break;
    case APPLY_DEX:
      ch->mod_stat[STAT_DEX] += mod;
      break;
    case APPLY_INT:
      ch->mod_stat[STAT_INT] += mod;
      break;
    case APPLY_WIS:
      ch->mod_stat[STAT_WIS] += mod;
      break;
    case APPLY_CON:
      ch->mod_stat[STAT_CON] += mod;
      break;
    case APPLY_CHA:
      ch->mod_stat[STAT_CHA] += mod;
      break;
    case APPLY_SEX:
      ch->sex += mod;
      break;
    case APPLY_CLASS:
      break;
    case APPLY_LEVEL:
      break;
    case APPLY_AGE:
      break;
    case APPLY_HEIGHT:
      break;
    case APPLY_WEIGHT:
      break;
    case APPLY_MANA:
      ch->max_mana += mod;
      break;
    case APPLY_HIT:
      ch->max_hit += mod;
      break;
    case APPLY_MOVE:
      ch->max_move += mod;
      break;
    case APPLY_GOLD:
      break;
    case APPLY_EXP:
      break;
    case APPLY_AC:
      for (i = 0; i < 4; i++)
	ch->armor[i] += mod;
      break;
    case APPLY_HITROLL:
      ch->hitroll += mod;
      break;
    case APPLY_DAMROLL:
      ch->damroll += mod;
      break;
    case APPLY_SAVES:
      ch->saving_throw += mod;
      break;
    case APPLY_SAVING_ROD:
      ch->saving_throw += mod;
      break;
    case APPLY_SAVING_PETRI:
      ch->saving_throw += mod;
      break;
    case APPLY_SAVING_BREATH:
      ch->saving_throw += mod;
      break;
    case APPLY_SAVING_SPELL:
      ch->saving_throw += mod;
      break;
    case APPLY_SPELL_AFFECT:
      break;
    }

  /*
   * Check for weapon wielding.
   * Guard against recursion (for weapons with affects).
   */
  if (!IS_NPC (ch)
      && (wield =
	  get_eq_char (ch,
		       WEAR_WIELD_R)) !=
      NULL && get_obj_weight (wield) >
      (str_app[get_curr_stat (ch, STAT_STR)].wield * 10))
    {
      static int depth;
      if (depth == 0)
	{
	  depth++;
	  obj_from_char (wield);
	  obj_to_room (wield, ch->in_room);
	  //Iblis - 3/11/04 - Antiduping Code
	  if (ch && !IS_NPC(ch))
            save_char_obj (ch);
	  if (!ch_is_quitting)
	    {
	      act ("You drop $p.", ch, wield, NULL, TO_CHAR);
	      act ("$n drops $p.", ch, wield, NULL, TO_ROOM);
	    }
	  depth--;
	}
    }
  if (!IS_NPC (ch) && (wield = get_eq_char (ch, WEAR_WIELD_L)) != NULL
      && get_obj_weight (wield) >
      (str_app[get_curr_stat (ch, STAT_STR)].wield * 10))
    {
      static int depth;
      if (depth == 0)
	{
	  depth++;
	  obj_from_char (wield);
	  obj_to_room (wield, ch->in_room);
	  if (!ch_is_quitting)
	    {
  	      //Iblis - 3/11/04 - Antiduping Code
       	      if (ch && !IS_NPC(ch))
	        save_char_obj (ch);
	      act ("You drop $p.", ch, wield, NULL, TO_CHAR);
	      act ("$n drops $p.", ch, wield, NULL, TO_ROOM);
	    }
	  depth--;
	}
    }
  return;
}


/* find an effect in an affect list */
AFFECT_DATA *affect_find (AFFECT_DATA * paf, int sn)
{
  AFFECT_DATA *paf_find;
  for (paf_find = paf; paf_find != NULL; paf_find = paf_find->next)
    {
      if (paf_find->type == sn)
	return paf_find;
    }
  return NULL;
}


/* fix object affects when removing one */
void affect_check (CHAR_DATA * ch, int where, int vector)
{
  AFFECT_DATA *paf;
  OBJ_DATA *obj;
  if (where == TO_OBJECT || where == TO_WEAPON || vector == 0 || where == TO_OBJECT2)
    return;
  for (paf = ch->affected; paf != NULL; paf = paf->next)
    if (paf->where == where && paf->bitvector == vector)
      {
	switch (where)
	  {
	  case TO_AFFECTS:
	    SET_BIT (ch->affected_by, vector);
	    break;
	  case TO_IMMUNE:
	    SET_BIT (ch->imm_flags, vector);
	    break;
	  case TO_RESIST:
	    SET_BIT (ch->res_flags, vector);
	    break;
	  case TO_VULN:
	    SET_BIT (ch->vuln_flags, vector);
	    break;
	  case TO_ACT2:
	    SET_BIT (ch->act2,vector);
	    break;
	  }
	return;
      }
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc == -1)
	continue;
      for (paf = obj->affected; paf != NULL; paf = paf->next)
	if (paf->where == where && paf->bitvector == vector)
	  {
	    switch (where)
	      {
	      case TO_AFFECTS:
		SET_BIT (ch->affected_by, vector);
		break;
	      case TO_IMMUNE:
		SET_BIT (ch->imm_flags, vector);
		break;
	      case TO_RESIST:
		SET_BIT (ch->res_flags, vector);
		break;
	      case TO_VULN:
		SET_BIT (ch->vuln_flags, vector);
		break;
	      case TO_ACT2:
		SET_BIT (ch->act2, vector);
		break;
	      }
	    return;
	  }
      if (obj->enchanted)
	continue;
      for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
	if (paf->where == where && paf->bitvector == vector)
	  {
	    switch (where)
	      {
	      case TO_AFFECTS:
		SET_BIT (ch->affected_by, vector);
		break;
	      case TO_IMMUNE:
		SET_BIT (ch->imm_flags, vector);
		break;
	      case TO_RESIST:
		SET_BIT (ch->res_flags, vector);
		break;
	      case TO_VULN:
		SET_BIT (ch->vuln_flags, vector);
		break;
	      case TO_ACT2:
		SET_BIT (ch->act2, vector);
	      }
	    return;
	  }
    }
}
void affect_to_char (CHAR_DATA * ch, AFFECT_DATA * paf)
{
  AFFECT_DATA *paf_new;
		  
		  
  if (IS_SET (ch->affected_by, paf->bitvector))
    return;
  paf_new = new_affect ();
  paf_new->where = paf->where;
  paf_new->type = paf->type;
  paf_new->level = paf->level;
  paf_new->duration = paf->duration;
  paf_new->location = paf->location;
  paf_new->modifier = paf->modifier;
  paf_new->bitvector = paf->bitvector;
  paf_new->permaff = paf->permaff;
  paf_new->composition = paf->composition;
  paf_new->comp_name = str_dup (paf->comp_name);

  /*  *paf_new = *paf; */
  paf_new->next = ch->affected;
  ch->affected = paf_new;
  affect_modify (ch, paf_new, TRUE);
  return;
}


/* give an affect to an object */
void affect_to_obj (OBJ_DATA * obj, AFFECT_DATA * paf)
{
  AFFECT_DATA *paf_new;
  paf_new = new_affect ();
  paf_new->where = paf->where;
  paf_new->type = paf->type;
  paf_new->level = paf->level;
  paf_new->duration = paf->duration;
  paf_new->location = paf->location;
  paf_new->modifier = paf->modifier;
  paf_new->bitvector = paf->bitvector;
  paf_new->permaff = paf->permaff;
  paf_new->composition = paf->composition;
  paf_new->comp_name = str_dup (paf->comp_name);

  /*  *paf_new = *paf; */
  paf_new->next = obj->affected;
  obj->affected = paf_new;

  /* apply any affect vectors to the object's extra_flags */
  if (paf->bitvector)
    switch (paf->where)
      {
      case TO_OBJECT:
	SET_BIT (obj->extra_flags[0], paf->bitvector);
	break;
      case TO_OBJECT2:
	SET_BIT (obj->extra_flags[1], paf->bitvector);
        break;
      case TO_WEAPON:
	if (obj->item_type == ITEM_WEAPON)
	  SET_BIT (obj->value[4], paf->bitvector);
	break;
      }
  return;
}


/*
 * Remove an affect from a char.
 */
void affect_remove (CHAR_DATA * ch, AFFECT_DATA * paf)
{
  int where;
  int vector;
  if (ch->affected == NULL)
    {
      bug ("Affect_remove: no affect.", 0);
      return;
    }
  affect_modify (ch, paf, FALSE);
  where = paf->where;
  vector = paf->bitvector;
  if (paf == ch->affected)
    {
      ch->affected = paf->next;
    }

  else
    {
      AFFECT_DATA *prev;
      for (prev = ch->affected; prev != NULL; prev = prev->next)
	{
	  if (prev->next == paf)
	    {
	      prev->next = paf->next;
	      break;
	    }
	}
      if (prev == NULL)
	{
	  bug ("Affect_remove: cannot find paf.", 0);
	  return;
	}
    }
  if (paf->bitvector == AFF_CHARM && ch->race != PC_RACE_SIDHE
      && ch->leader != NULL && ch->leader->race == PC_RACE_SIDHE)
    {
      act ("Repulsed by $n's very presence, you push $m away in disgust.",
	   ch, NULL, ch->leader, TO_VICT);
      act
	("Your mind pulls free of the grasp of $N's magic, and $E pushes you away with a look of utter disgust.",
	 ch, NULL, ch->leader, TO_CHAR);
      if (ch->master->pet == ch)
	ch->master->pet = NULL;
      ch->master = NULL;
      ch->leader = NULL;
    }

  else if (paf->bitvector == AFF_CHARM && ch->race == PC_RACE_SIDHE
	   && ch->leader != NULL && ch->leader->race != PC_RACE_SIDHE)
    {
      act ("You regain control of your mind and push $N away in contempt.",
	   ch, NULL, ch->leader, TO_CHAR);
      act
	("$n snaps his head to glare balefully at you a second before he escapes your magical grasp and slinks away.",
	 ch, NULL, ch->leader, TO_VICT);
      if (ch->master->pet == ch)
	ch->master->pet = NULL;
      ch->master = NULL;
      ch->leader = NULL;
    }

   if (paf->type == gsn_contaminate && ch->contaminator)
   {
     send_to_char("Someone somewhere has broken free of your contamination.\n\r",ch->contaminator);
/*     ch->contaminator->mana -= 100;
     ch->contaminator->move -= 100;
     damage (ch->contaminator,ch->contaminator,100,gsn_contaminate,DAM_UNIQUE,FALSE);*/
     ch->contaminator = NULL;
   }
   
  if (paf->type == gsn_dehydration && ch->race == PC_RACE_NIDAE)
  {
    ch->perm_stat[STAT_CHA] += 9;
    ch->perm_stat[STAT_DEX] += 5;
  }

  if (paf->type == gsn_transform && !IS_NPC(ch))
  {
    REMOVE_BIT(ch->act2,ACT_TRANSFORMER);
    free_string(ch->long_descr);
    ch->long_descr = str_dup("");
    free_string(ch->short_descr);
    ch->short_descr = str_dup("");
  }
  
  if (paf->type == gsn_spider_vine)
    {
      OBJ_DATA* obj;
      obj = get_eq_char (ch, WEAR_WRIST_L);
      if (obj != NULL)
	unequip_char(ch,obj);
      obj_from_char(obj);
      extract_obj(obj);
      obj = get_eq_char (ch, WEAR_WRIST_R);
      if (obj != NULL)
	unequip_char(ch,obj);
      obj_from_char(obj);
      extract_obj(obj);
      obj = get_eq_char (ch, WEAR_ARMS);
      if (obj != NULL)
	unequip_char(ch,obj);
      obj_from_char(obj);
      extract_obj(obj);
    }

  if (paf->type == gsn_soul_link)
  {
    if (!IS_NPC(ch))
      if (ch->pcdata->soul_link)
      {
	ch->pcdata->soul_link->pcdata->soul_link = NULL;
	affect_strip(ch->pcdata->soul_link,gsn_soul_link);
	ch->pcdata->soul_link = NULL;
      }
  }
/*
  if (paf->type == gsn_intoxicate)
  {
    if (!IS_NPC(ch))
      ch->pcdata->condition[COND_DRUNK] = 0;
  }
*/
  if (paf->type == gsn_bark_skin)
    affect_strip (ch, gsn_spider_vine);

  if (paf->type == gsn_scramble && !IS_NPC(ch))
  {
    OBJ_DATA *obj, *obj_next;
    long wear_long;
    int hit_difference = 0, mana_difference = 0, move_difference = 0;
    ch->max_hit = 1000;
    ch->max_move = 1000;
    ch->max_mana = 1000;
    for (obj = ch->carrying; obj != NULL; obj = obj_next)
      {
	obj_next = obj->next_content;
	if (obj->wear_loc == WEAR_NONE)
	  continue;
	wear_long = obj->wear_loc;
//	if (obj->affected)
//	  {
	    unequip_char (ch, obj);
	    hit_difference += (1000-ch->max_hit);
	    mana_difference += (1000-ch->max_mana);
	    move_difference += (1000-ch->max_move);
	    equip_char_slot_new (ch, obj, wear_long);
//	  }
      }
/*    ch->max_hit = ch->pcdata->perm_hit;
    ch->max_move = ch->pcdata->perm_move;
    ch->max_mana = ch->pcdata->perm_mana;*/
    ch->max_hit = ch->pcdata->perm_hit + hit_difference;
    ch->max_move = ch->pcdata->perm_move + move_difference;
    ch->max_mana = ch->pcdata->perm_mana + mana_difference;
  }
  free_affect (paf);
  affect_check (ch, where, vector);
  return;
}

void affect_remove_obj (OBJ_DATA * obj, AFFECT_DATA * paf)
{
  int where, vector;
  if (obj->affected == NULL)
    {
      bug ("Affect_remove_object: no affect.", 0);
      return;
    }
  if (obj->carried_by != NULL && obj->wear_loc != -1)
    affect_modify (obj->carried_by, paf, FALSE);
  where = paf->where;
  vector = paf->bitvector;

  /* remove flags from the object if needed */
  if (paf->bitvector)
    switch (paf->where)
      {
      case TO_OBJECT:
	REMOVE_BIT (obj->extra_flags[0], paf->bitvector);
	break;
      case TO_OBJECT2:
	REMOVE_BIT (obj->extra_flags[1], paf->bitvector);
	break;
      case TO_WEAPON:
	if (obj->item_type == ITEM_WEAPON)
	  REMOVE_BIT (obj->value[4], paf->bitvector);
	break;
      }
  if (paf == obj->affected)
    {
      obj->affected = paf->next;
    }

  else
    {
      AFFECT_DATA *prev;
      for (prev = obj->affected; prev != NULL; prev = prev->next)
	{
	  if (prev->next == paf)
	    {
	      prev->next = paf->next;
	      break;
	    }
	}
      if (prev == NULL)
	{
	  bug ("Affect_remove_object: cannot find paf.", 0);
	  return;
	}
    }
  free_affect (paf);
  if (obj->carried_by != NULL && obj->wear_loc != -1)
    affect_check (obj->carried_by, where, vector);
  return;
}


/*
 * Strip all affects of a given sn.
 */
void affect_strip (CHAR_DATA * ch, int sn)
{
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;
  for (paf = ch->affected; paf != NULL; paf = paf_next)
    {
      paf_next = paf->next;
      if (paf->type == sn)
	affect_remove (ch, paf);
    }
  return;
}


/*
 * Return true if a char is affected by a spell.
 */
inline bool is_affected (CHAR_DATA * ch, int sn)
{
  AFFECT_DATA *paf;
  if (ch == NULL)
    return FALSE;
  //Iblis 10/30/04 - Bad hack, but saves changing of a LOT of code
  if (sn == gsn_mirror && IS_SET(ch->act2,ACT_ALWAYS_MIRRORED))
    return TRUE;
		  
  for (paf = ch->affected; paf != NULL; paf = paf->next)
    {
      if (paf->type == sn && paf->composition == FALSE)
	return TRUE;
    }
  return FALSE;
}


/*
 * Strip all affects that modify skills.
 */
void affect_strip_skills (CHAR_DATA * ch)
{
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;
  for (paf = ch->affected; paf != NULL; paf = paf_next)
    {
      paf_next = paf->next;
      if (paf->where == TO_SKILL)
        affect_remove (ch, paf);
    }
  return;
}



/*
 * Add or enhance an affect.
 */
void affect_join (CHAR_DATA * ch, AFFECT_DATA * paf)
{
  AFFECT_DATA *paf_old;
  bool found;
  found = FALSE;
  for (paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next)
    {
      if (paf_old->type == paf->type)
	{
	  paf->level = (paf->level += paf_old->level) / 2;
	  paf->duration += paf_old->duration;
	  paf->modifier += paf_old->modifier;
	  affect_remove (ch, paf_old);
	  break;
	}
    }
  affect_to_char (ch, paf);
  return;
}


/*
 * Move a char out of a room.
 */
void char_from_room (CHAR_DATA * ch)
{
  OBJ_DATA *obj;
  CHAR_DATA *fch, *fch_next;
  //char buf[MAX_STRING_LENGTH];
  if (ch->in_room == NULL)
    {
      return;
    }
  if (!IS_NPC (ch) && ch->pcdata && (ch->pcdata->cg_state != 0 || ch->pcdata->cg_ticks != 0))
  {
    if (ch->pcdata->cg_state > CG_ANTE_ACCEPTED)
      cardgame_surrender(ch,TRUE);
    else if (ch->pcdata->cg_state == 0)
      do_cardduel(ch,"quit");
    else do_ante(ch,"quit");
  }
    

  if (!IS_NPC (ch))
  {
    --ch->in_room->area->nplayer;
    //do_echo(ch,"num_players--");
  }
/*  if (ch->in_room->area->nplayer < 0)
  {
    ch->in_room->area->nplayer = 0;
   // do_echo(ch,"num_players < 0 so set to 0");
  }*/
  if ((((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL
      && obj->item_type == ITEM_LIGHT
      && obj->value[2] != 0) || (!IS_NPC(ch) && ch->pcdata->flaming))
	  && ch->in_room->light > 0)
    --ch->in_room->light;
  if (ch == ch->in_room->people)
    {

      /* If top of the people list in the room, set top of
       * list to the next_in_room. Mathew 4/14/99
       */
      ch->in_room->people = ch->next_in_room;
    }

  else
    {

      /* Otherwise, remove ch from the people in room list. Mathew 4/14/99
       */
      CHAR_DATA *prev;
      for (prev = ch->in_room->people; prev; prev = prev->next_in_room)
	{
	  if (prev->next_in_room == ch)
	    {
	      prev->next_in_room = ch->next_in_room;
	      break;
	    }
	}

//      sprintf(buf,"char_from_room:ch found. (%s)",ch->name);
      //    bug (buf,0);
      if (prev == NULL)
	{
//	  sprintf (buf, "Char_from_room:ch not found. (%s)", ch->name);
//	  bug (buf, 0);
	}
    }
  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
    {
      TRIGGER_DATA *pTrig;
      fch_next = fch->next_in_room;
      if (!IS_NPC (fch))
	continue;
      if (!can_see (fch, ch) || !IS_AWAKE (fch))
	continue;
      if (ch == fch)
	continue;
      if (HAS_SCRIPT (fch))
	for (pTrig = fch->triggers; pTrig != NULL; pTrig = pTrig->next)
	  if (pTrig->trigger_type == TRIG_EXIT && pTrig->current ==
	      NULL && !IS_SET (fch->act, ACT_HALT))
	    {
	      act_trigger (fch, pTrig->name, NULL, NAME (ch), NAME (fch));
	      pTrig->current = pTrig->script;
	      pTrig->bits = SCRIPT_ADVANCE;
	    }
    }
  ch->in_room = NULL;
  ch->next_in_room = NULL;
  ch->on = NULL;		/* sanity check! */
  return;
}

void add_scent (CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex)
{
  SCENT_DATA *sc;
  if (IS_SET (ch->act, ACT_NO_SCENT) || ch->level >= LEVEL_IMMORTAL)
    return;
  switch (pRoomIndex->sector_type)
    {
    case SECT_UNDERWATER:
    case SECT_WATER_SWIM:
    case SECT_WATER_NOSWIM:
    case SECT_WATER_OCEAN:
      return;
    }
  for (sc = pRoomIndex->scents; sc; sc = sc->next_in_room)
    {
      if (sc->player == ch)
	{
	  switch (pRoomIndex->sector_type)
	    {
	    default:
	      sc->scent_level = SCENT_STRONG;
	      break;
	    case SECT_SWAMP:
	    case SECT_HILLS:
	    case SECT_MOUNTAIN:
	    case SECT_SHORE:
	      sc->scent_level = SCENT_NOTICEABLE;
	      break;
	    }
	  return;
	}
    }
  sc = new_scent ();
  switch (pRoomIndex->sector_type)
    {
    default:
      sc->scent_level = SCENT_STRONG;
      break;
    case SECT_SWAMP:
    case SECT_HILLS:
    case SECT_MOUNTAIN:
    case SECT_SHORE:
      sc->scent_level = SCENT_NOTICEABLE;
      break;
    }
  sc->in_room = pRoomIndex;
  sc->player = ch;
  sc->next_in_room = pRoomIndex->scents;
  pRoomIndex->scents = sc;
  sc->next = scents;
  scents = sc;
  if (get_skill (ch, gsn_conceal) > 0)
    do_conceal (ch, NULL);
}

bool is_hunting (CHAR_DATA * hunter, CHAR_DATA * hunted)
{
  MEMORY_DATA *hunts;
  if (get_position(hunter) <= POS_SLEEPING)
    return (FALSE);
  for (hunts = hunter->hunt_memory; hunts; hunts = hunts->next_memory)
    if (hunts->player == hunted)
      return (TRUE);
  return (FALSE);
}


/*
 * Move a char into a room.
 */
void char_to_room (CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex)
{
  OBJ_DATA *obj;
  CHAR_DATA *fch, *fch_next;
  bool stop_follow = FALSE;
  long bigasscounter=0;
  
  char buf[MAX_STRING_LENGTH];
  if (pRoomIndex == NULL)
    {
      ROOM_INDEX_DATA *room;
      bug ("Char_to_room: NULL.", 0);
      if ((room = get_room_index (ROOM_VNUM_ALTAR)) != NULL)
	char_to_room (ch, room);
      return;
    }
  ch->in_room = pRoomIndex;
  ch->next_in_room = pRoomIndex->people;
  pRoomIndex->people = ch;

  // druids affected by trackless step leave no scent in natural areas
  if (!(ch->Class == PC_CLASS_DRUID) || !is_affected(ch, skill_lookup("trackless step")) ||
	!is_char_in_natural_sector(ch))
  add_scent (ch, pRoomIndex);

  // Adeon 6/30/03
  if (IS_SET (pRoomIndex->room_flags2, ROOM_SINKING))
    if (!IS_SET (ch->act2, ACT_NOSINK))
      ch->sink_timer = pRoomIndex->sink_timer;

    else
      ch->sink_timer = -1;

  else
    ch->sink_timer = -1;
  if ((ch->in_room->sector_type == SECT_UNDERWATER
       || ch->in_room->sector_type == SECT_WATER_SWIM
       || ch->in_room->sector_type == SECT_WATER_OCEAN
       || ch->in_room->sector_type == SECT_SWAMP
       || ch->in_room->sector_type == SECT_WATER_NOSWIM
       ||  (IS_OUTSIDE(ch) && (weather_info.sky == SKY_RAINING || weather_info.sky == SKY_LIGHTNING)))
       && ch->race == PC_RACE_LITAN)
  {
    if (!IS_NPC(ch) && ch->pcdata->flaming)
      {
	ch->pcdata->flaming = 0;
	if (!((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL
                  && obj->item_type == ITEM_LIGHT
	      && obj->value[2] != 0 && ch->in_room != NULL))
	  ch->in_room->light--;

	if (is_affected(ch,gsn_fireshield))
	  affect_strip(ch,gsn_fireshield);
	send_to_char("The flame surrounding you is fizzles out by the water.\n\r",ch);
      }
  }

  if ((ch->in_room->sector_type == SECT_UNDERWATER
       || ch->in_room->sector_type == SECT_WATER_SWIM
       || ch->in_room->sector_type == SECT_WATER_OCEAN)
      && is_affected (ch, gsn_fireshield))
    {
      send_to_char
	("Your shield of fire quickly evaporates into a useless steam.\n\r",
	 ch);
      act ("$n's shield of fire quickly evaporates into a useless steam.",
	   ch, NULL, NULL, TO_ROOM);
      affect_strip (ch, gsn_fireshield);
    }
  if ((ch->in_room->sector_type != SECT_UNDERWATER
       && ch->in_room->sector_type != SECT_WATER_SWIM
       && ch->in_room->sector_type != SECT_WATER_NOSWIM
       && ch->in_room->sector_type != SECT_WATER_OCEAN)
      || IS_AFFECTED (ch, AFF_AQUA_BREATHE)
      || IS_SET(ch->imm_flags,IMM_DROWNING))
    ch->drown_damage = 1;
  if (pRoomIndex->tp_exp > 0 && ch->level <= pRoomIndex->tp_level
      && !IS_NPC (ch) && !IS_IMMORTAL (ch))
    check_travel_point (ch);
  bigasscounter = 0;
  for (fch = pRoomIndex->people; fch != NULL; fch = fch_next)
    {
      TRIGGER_DATA *pTrig;
      if (bigasscounter++ > 10000)
      {
	 sprintf(buf,"Bigasscounter in char_to_room person -> %s bug name -> %s bug room -> %d",ch->name, pRoomIndex->people->name,pRoomIndex->vnum);
	 bug(buf,0);
	 if (IS_NPC(ch))
	 {
		 bug("ITS AN NPC",0);
		 sprintf(buf,"Vnum = %d",ch->pIndexData->vnum);
		 bug(buf,0);
	 }
	 else
	 {
		 bug("ITS A PC",0);
                 exit(1);
	 }
         		 
	 return;
      }
      fch_next = fch->next_in_room;
      if (ch == fch)
	continue;
      if (!can_see (fch, ch) || !IS_AWAKE (fch))
	continue;
      if (!IS_NPC (fch))
	continue;
      if (IS_AFFECTED (ch, AFF_SNEAK)
	  && !IS_AFFECTED (fch, AFF_DETECT_HIDDEN))
	continue;
      if (ch->group != 0 && !stop_follow && ch->group == fch->group)
	{
	  int numgr;
	  if (!is_same_group (ch, fch))
	    {
	      numgr = number_in_group (ch) - 1;
	      numgr = 100 - (numgr * 40);
	      if (numgr >= number_percent () && fch->master == NULL)
		{
		  add_follower (fch, ch);
		  group_gr (ch, fch);
		}
	    }
	  stop_follow = TRUE;
	}
      if (HAS_SCRIPT (fch))
	for (pTrig = fch->triggers; pTrig != NULL; pTrig = pTrig->next)
	  if (pTrig->trigger_type ==
	      TRIG_ENTER && pTrig->current == NULL
	      && !IS_SET (fch->act, ACT_HALT))
	    {
	      act_trigger (fch, pTrig->name, NULL, NAME (ch), NAME (fch));
	      pTrig->current = pTrig->script;
	      pTrig->bits = SCRIPT_ADVANCE;
	    }
    }

  /*  if(IS_NPC(ch))
     {
     if(ch->riders != NULL)
     {
     if(ch->in_room != ch->riders->in_room)
     {
     ch->riders->mount = NULL;
     ch->riders->position = POS_STANDING;
     ch->riders = NULL;
     }
     }
     } */
  if (!IS_NPC (ch))
    {
      if (ch->mount != NULL)
	{
	  if (ch->in_room != ch->mount->in_room)
	    {
	      ch->mount->riders = NULL;
	      ch->mount = NULL;
	      ch->position = POS_STANDING;
	    }
	}
      if (ch->in_room->area->empty)
	{
	  ch->in_room->area->empty = FALSE;
	  ch->in_room->area->age = 0;
	}
      ++ch->in_room->area->nplayer;
       //do_echo(ch,"num_players++");
      /*{
	      char buf[10];
      sprintf(buf,"NPLAYER - %d\n\r",ch->in_room->area->nplayer);
      send_to_char(buf,ch);
      }*/
      //Iblis 10/04/04 - SCAN TO SEE IF IMAGINARY FRIENDS SHOULD POOF
      if (ch->in_room->area->nplayer > 1 && !IS_IMMORTAL(ch))
      {
	CHAR_DATA *mch;
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
	  if (mch->Class == PC_CLASS_CHAOS_JESTER && mch->in_room->area == ch->in_room->area
			  && mch->pcdata->familiar )
	  {
	    act("$N vanishes into thin air.",mch,NULL,mch->pcdata->familiar,TO_ALL);
	    familiar_poof(mch);
	  }
	}
      }
      
    }
  if (((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL
      && obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
		  || (!IS_NPC(ch) && ch->pcdata->flaming))
    ++ch->in_room->light;
  if (IS_AFFECTED (ch, AFF_PLAGUE))
    {
      AFFECT_DATA *af, plague;
      CHAR_DATA *vch;
      for (af = ch->affected; af != NULL; af = af->next)
	{
	  if (af->type == gsn_plague)
	    break;
	}
      if (af == NULL)
	{
	  REMOVE_BIT (ch->affected_by, AFF_PLAGUE);

	  //Iblis 6/22/03 - Falcon skill
	  falcon_alert (ch);
	  return;
	}
      if (af->level == 1)
	{

	  //Iblis 6/22/03 - Falcon skill
	  falcon_alert (ch);
	  return;
	}
      plague.where = TO_AFFECTS;
      plague.type = gsn_plague;
      plague.level = af->level - 1;
      plague.duration = number_range (1, 2 * plague.level);
      plague.location = APPLY_STR;
      plague.modifier = -5;
      plague.bitvector = AFF_PLAGUE;
      plague.composition = FALSE;
      plague.comp_name = str_dup ("");
      for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	  if (!saves_spell (plague.level - 2, vch, DAM_DISEASE)
	      && !IS_IMMORTAL (vch) && !IS_AFFECTED (vch, AFF_PLAGUE)
              && IS_NPC(vch)
	      && number_bits (6) == 0)
	    {
	      send_to_char ("You feel hot and feverish.\n\r", vch);
	      act ("$n shivers and looks very ill.", vch, NULL, NULL,
		   TO_ROOM);
	      affect_join (vch, &plague);
	    }
	}
    }
  //Iblis 6/22/03 - Falcon skill
  falcon_alert (ch);
  //Iblis 1/02/04 - Fixed enters room and char enters room
/*  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  sprintf(tcbuf,"%d",ch->in_room->vnum);
  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);*/
      
  return;
}


/*
 * Give an obj to a char.
 */
void obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch)
{
  if (!IS_NPC (ch) && obj->item_type == ITEM_FOOD && obj->timer == 0 && obj->value[5] != -1)
    obj->timer = obj->pIndexData->value[2];
  obj->next_content = ch->carrying;
  ch->carrying = obj;
  obj->carried_by = ch;
  obj->in_room = NULL;
  obj->in_obj = NULL;
  ch->carry_number += get_obj_number (obj);
  ch->carry_weight += get_obj_weight (obj);
}


/*
 * Take an obj from its character.
 */
void obj_from_char (OBJ_DATA * obj)
{
  CHAR_DATA *ch;
  if (!obj)
    return;
  if ((ch = obj->carried_by) == NULL)
    {
      bug ("Obj_from_char: null ch.", 0);
      return;
    }
  if (obj->wear_loc != WEAR_NONE)
    unequip_char (ch, obj);
  if (ch->carrying == obj)
    {
      ch->carrying = obj->next_content;
    }

  else
    {
      OBJ_DATA *prev;
      for (prev = ch->carrying; prev != NULL; prev = prev->next_content)
	{
	  if (prev->next_content == obj)
	    {
	      prev->next_content = obj->next_content;
	      break;
	    }
	}
      if (prev == NULL)
	bug ("Obj_from_char: obj not in list.", 0);
    }
  obj->carried_by = NULL;
  obj->next_content = NULL;
  ch->carry_number -= get_obj_number (obj);
  ch->carry_weight -= get_obj_weight (obj);
  return;
}


/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac_new (OBJ_DATA * obj, int type)
{
  if (obj->item_type != ITEM_ARMOR)
    return 0;
  if (IS_SET(obj->wear_flags,ITEM_WEAR_BODY))
    return 3 * obj->value[type];
  if (IS_SET(obj->wear_flags,ITEM_WEAR_ABOUT) || IS_SET(obj->wear_flags,ITEM_WEAR_HEAD) 
		  || IS_SET(obj->wear_flags,ITEM_WEAR_LEGS) || IS_SET(obj->wear_flags, ITEM_WEAR_LEGS))
    return 2 * obj->value[type];
  if (IS_SET(obj->wear_flags,ITEM_WEAR_FEET) || IS_SET(obj->wear_flags,ITEM_WEAR_HANDS)
		  || IS_SET(obj->wear_flags,ITEM_WEAR_ARMS) || IS_SET(obj->wear_flags,ITEM_WEAR_SHIELD)
		  || IS_SET(obj->wear_flags,ITEM_WEAR_NECK) || IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_NECK)
		  || IS_SET(obj->wear_flags,ITEM_WEAR_WRIST) || IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_WRIST)
		  || IS_SET(obj->wear_flags,ITEM_WEAR_WAIST) || IS_SET(obj->wear_flags,ITEM_HOLD)
		  || IS_SET(obj->wear_flags,ITEM_WEAR_FACE))
	  return obj->value[type];
  return 0;
}

/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac (OBJ_DATA * obj, int iWear, int type)
{
  if (obj->item_type != ITEM_ARMOR)
    return 0;
  switch (iWear)
    {
    case WEAR_BODY:
      return 3 * obj->value[type];
    case WEAR_HEAD:
      return 2 * obj->value[type];
    case WEAR_LEGS:
      return 2 * obj->value[type];
    case WEAR_FEET:
      return obj->value[type];
    case WEAR_HANDS:
      return obj->value[type];
    case WEAR_ARMS:
      return obj->value[type];
    case WEAR_SHIELD:
      return obj->value[type];
    case WEAR_FINGER_L:
      return 0;
    case WEAR_FINGER_R:
      return 0;
    case WEAR_NECK_1:
      return obj->value[type];
    case WEAR_NECK_2:
      return obj->value[type];
    case WEAR_ABOUT:
      return 2 * obj->value[type];
    case WEAR_WAIST:
      return obj->value[type];
    case WEAR_WRIST_L:
      return obj->value[type];
    case WEAR_WRIST_R:
      return obj->value[type];
    case WEAR_HOLD:
      return obj->value[type];
    case WEAR_WORN_WINGS:
	return 3 * obj->value[type];
    }
  return 0;
}


/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char_new (CHAR_DATA * ch, long iWear)
{
  OBJ_DATA *obj;
  long bugcounter = 0;
  if (ch == NULL)
    return NULL;
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if (bugcounter++ > 9999999)
	{
	  bug ("ERROR ERROR in get_eq_char for CHARNAME then OBJNAME", 0);
	  bug (ch->name, 0);
	  bug (obj->short_descr, 0);
	  bug (obj->name, 0);
	  return NULL;
	}
      if (obj->wear_loc == -1)
        continue;
      if (IS_SET(obj->wear_loc,iWear))
	return obj;
    }
  return NULL;
}

/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char (CHAR_DATA * ch, int iWear)
{
  OBJ_DATA *obj;
  int i=0;
  long bugcounter = 0, wear_flag=-1;
  if (ch == NULL)
    return NULL;
  for (i=0; wear_conversion_table[i].wear_loc != -2; ++i)
  {
    if (wear_conversion_table[i].wear_loc == iWear)
    {
      wear_flag = wear_conversion_table[i].wear_flag;
      break;
    }
  }
  if (wear_flag == -1)
    return NULL;
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if (bugcounter++ > 9999999)
        {
          bug ("ERROR ERROR in get_eq_char for CHARNAME then OBJNAME", 0);
          bug (ch->name, 0);
          bug (obj->short_descr, 0);
          bug (obj->name, 0);
          return NULL;
        }
      if (obj->wear_loc == -1)
        continue;
      if (IS_SET(obj->wear_loc,wear_flag))
        return obj;
    }
  return NULL;
}

// Iblis 1/04/04 - Gets an object from the room with a specific vnum
// Created for use in necromancer's draw skill
OBJ_DATA *get_obj_vnum_room(ROOM_INDEX_DATA* rid, int vnum)
{
  OBJ_DATA* obj;
  for (obj = rid->contents;obj != NULL;obj = obj->next_content)
  {
    if (obj->pIndexData->vnum == vnum)
     return obj;
  }
  return NULL;
}

// Iblis 1/04/04 - Gets an object from a char with a specific vnum
// Created for use in necromancer's draw skill
OBJ_DATA *get_obj_vnum_char(CHAR_DATA* ch, int vnum)
{
  OBJ_DATA* obj;
  for (obj = ch->carrying;obj != NULL;obj = obj->next_content)
    {
      if (obj->pIndexData->vnum == vnum)
	return obj;
    }
  return NULL;
}

// Iblis 1/07/04 - Gets an object from the room with a specific vnum
// Created for use in necromancer's bone craft skill
OBJ_DATA *get_obj_vnum_container(OBJ_DATA* rid, int vnum)
{
  OBJ_DATA* obj;
  for (obj = rid->contains;obj != NULL;obj = obj->next_content)
    {
      if (obj->pIndexData->vnum == vnum)
	return obj;
    }
  return NULL;
}


//
// Akamai 4/30/99 -- Support race and Class specific objects
// Checks to see if a character can actually hold/wear the piece of
// equipment. If the char can not hold it, then it is forcibly dropped
//
int char_can_wear (CHAR_DATA * ch, OBJ_DATA * obj)
{
  long fval = 0;
  if (!obj)
    return FALSE;

  // Immortals can wear anything
  if (IS_IMMORTAL (ch))
    return (TRUE);

  // NPCs can wear anything too - they're special
  if (IS_NPC (ch))
    return (TRUE);

  // Validate good/neutral/evil 
  if ((IS_OBJ_STAT (obj, ITEM_ANTI_EVIL) && IS_EVIL (ch)) ||
      (IS_OBJ_STAT (obj, ITEM_ANTI_GOOD) && IS_GOOD (ch)) ||
      (IS_OBJ_STAT (obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL (ch)))
    {
      return (FALSE);
    }


  // Iblis - (11/09/03)Validate Clan
  if (!IS_SET(obj->clan_flags,flag_value(clan_flags, capitalize(get_clan_name_ch (ch)))))
  {
	  if (obj->clan_flags != 0)
		  return (FALSE);
  }
  if (flag_value(clan_flags, capitalize(get_clan_name_ch (ch))) == -99 && ch->pcdata->loner == FALSE
		  && !IS_SET(obj->clan_flags,CLAN_NONE))
  {
	   if (obj->clan_flags != 0)
		  return (FALSE);
  }
  if (flag_value(clan_flags, capitalize(get_clan_name_ch (ch))) == -99 && ch->pcdata->loner == TRUE
                  && !IS_SET(obj->clan_flags,CLAN_LONER))
  {
	   if (obj->clan_flags != 0)
 	          return (FALSE);
  }
  
  // Validate race
  // if no flags are set then it is valid for all races
  if (obj->race_flags != 0)
    {
      switch (ch->race)
	{
	case PC_RACE_HUMAN:
	  if (!IS_SET (obj->race_flags, RACE_HUMAN))
	    return (FALSE);
	  break;
	case PC_RACE_ELF:
	  if (!IS_SET (obj->race_flags, RACE_ELF))
	    return (FALSE);
	  break;
	case PC_RACE_DWARF:
	  if (!IS_SET (obj->race_flags, RACE_DWARF))
	    return (FALSE);
	  break;
	case PC_RACE_CANTHI:
	  if (!IS_SET (obj->race_flags, RACE_CANTHI))
	    return (FALSE);
	  break;
	case PC_RACE_VROATH:
	  if (!IS_SET (obj->race_flags, RACE_VROATH))
	    return (FALSE);
	  break;
	case PC_RACE_SYVIN:
	  if (!IS_SET (obj->race_flags, RACE_SYVIN))
	    return (FALSE);
	  break;
	case PC_RACE_SIDHE:
	  if (!IS_SET (obj->race_flags, RACE_SIDHE))
	    return (FALSE);
	  break;
	case PC_RACE_LITAN:
	  if (!IS_SET (obj->race_flags, RACE_LITAN))
            return (FALSE);
          break;
	case PC_RACE_KALIAN:
	   return (TRUE);
           break;
	case PC_RACE_LICH:
	  if (!IS_SET (obj->race_flags, RACE_LICH))
	    return (FALSE);
	  break;
	case PC_RACE_NERIX:
	  if (!IS_SET (obj->race_flags, RACE_NERIX))
	  return (FALSE);
	case PC_RACE_SWARM:
	  if (!IS_SET (obj->race_flags, RACE_SWARM))
 	    return (FALSE);
	default:
	  bug ("char_can_wear() - unrecognized PC race %d", ch->race);
	  return (FALSE);
	  break;
	}
    }
  // Validate Class
  // if no flags are set then it is valid for all Classes
  if (obj->Class_flags != 0)
    {

      // Hero status is a variation of the Class flags
      if (IS_SET (obj->Class_flags, CLASS_HERO) && !(ch->pcdata->hero))
	{
	  return (FALSE);
	}
      // get the flag value from the Class_flags table for this Class
      fval = flag_value (Class_flags, Class_table[ch->Class].name);
      if (fval < 0)
	{
	  bug ("char_can_wear() - unrecognized Class/Class flag", 0);
	  return (FALSE);
	}

      else
	{
	  if (!Class_flagged_obj (obj, ch))
	    return (FALSE);
	}
    }
  return (TRUE);
}

bool char_can_remove_to_wear(CHAR_DATA *ch, OBJ_DATA *obj_to_wear, bool show_messages)
{
  return char_can_remove_to_wear_hand(ch,obj_to_wear,show_messages,0);
}

//Iblis - new function to deal with multislot eq
bool char_can_remove_to_wear_hand(CHAR_DATA *ch, OBJ_DATA *obj_to_wear, bool show_messages, int hand)
{
  long i, hands_needed=0;
  
  OBJ_DATA *obj=NULL;
  for (i = 2; i <= MAX_WEAR_FLAG;i = i<<1)
  {
    if (!IS_SET(obj_to_wear->wear_flags,i) && !(obj_to_wear->item_type == ITEM_LIGHT && i == ITEM_WEAR_LIGHT))
      continue;
    if (i == ITEM_NO_SAC || i == ITEM_TWO_HANDS || i == ITEM_TAKE)
      continue;
    if ((i == ITEM_WEAR_BODY || i == ITEM_WEAR_LEGS) && is_affected(ch,skill_lookup("swell")))
    {
      if (show_messages)
        send_to_char("You're too swollen to wear that.\n\r",ch);
      return FALSE;
    }
    if ((obj = get_eq_char_new (ch, i)) == NULL)
      continue;
    if (i == ITEM_WIELD)
    {
      if (get_skill(ch,gsn_dual_wield) == 0 && hand != HAND_AMBIDEXTROUS)
      {
        if (hand != HAND_LEFT)
	  obj = get_eq_char_new (ch,ITEM_WIELD);
	else obj = get_eq_char_new (ch,ITEM_TWO_HANDS);
	if (obj == NULL)
  	  continue;
	//handle the noremove with the code below
      }
      else
      {
	if (hand != HAND_LEFT)
	{
  	  obj = get_eq_char_new (ch,ITEM_WIELD);
	  if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
	  {
  	    if (show_messages && get_eq_char_new (ch,ITEM_TWO_HANDS) != NULL)
  	      act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
	    obj = get_eq_char_new(ch,ITEM_TWO_HANDS);
	  }
	}
	else
	{
          obj = get_eq_char_new (ch,ITEM_TWO_HANDS);
	  if (obj == NULL)
  	    continue;
	  if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
	  {
	    if (show_messages && get_eq_char_new (ch,ITEM_WIELD) != NULL)
	       act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
	    obj = get_eq_char_new(ch,ITEM_WIELD);
	  }
	}
	if (obj == NULL)
  	  continue;
      }
		 
    }
    if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
    {
      //The following three checks deal with objects that could be in two locations, that do not
      //take up both of those locations.
      if (i == ITEM_WEAR_NECK && !IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_NECK))
      {
        if (show_messages && get_eq_char_new (ch,ITEM_WEAR_ANOTHER_NECK) != NULL)
          act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
        obj = get_eq_char_new (ch,ITEM_WEAR_ANOTHER_NECK);
      }
      else if (i == ITEM_WEAR_FINGER && !IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_FINGER))
      {
        if (show_messages && get_eq_char_new (ch,ITEM_WEAR_ANOTHER_FINGER) != NULL)
          act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
        obj = get_eq_char_new (ch,ITEM_WEAR_ANOTHER_FINGER);
      }
      else if (i == ITEM_WEAR_WRIST && !IS_SET(obj->wear_flags, ITEM_WEAR_ANOTHER_WRIST))
      {
        if (show_messages && get_eq_char_new (ch,ITEM_WEAR_ANOTHER_WRIST) != NULL)
          act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
        obj = get_eq_char_new (ch, ITEM_WEAR_ANOTHER_WRIST);
      }
      if (obj == NULL)
        continue;
      if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
      {
        if (show_messages)
          act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
        return FALSE;
      }
    }
  }
  //Perform the hands check crap
  if ((hands_needed = hands_required(ch,obj_to_wear)) + hands_used(ch) <= 2 
    || (!IS_SET(obj_to_wear->wear_flags,ITEM_WIELD) && hands_needed < 2))
    return TRUE;
  hands_needed = hands_required(ch,obj_to_wear) - (2-hands_used(ch));
  if (hands_needed > 2)
    return FALSE;
  //If this is not a weapon, hands_needed =2 ,and we already know we can remove the slot the item
  //  goes in, so we need to be able to clear one more hand
  //If this is a weapon, they have a weapon slot free they can legally wear it
  if (hands_needed && (obj = get_eq_char_new(ch,ITEM_WEAR_SHIELD)))
  {
     if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
     {
       if (show_messages)
         act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
       return FALSE;
     }
     --hands_needed;
  }
  if (hands_needed && (obj = get_eq_char_new(ch,ITEM_HOLD)))
  {
    if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
    {
      if (show_messages)
        act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
      return FALSE;
    }
    --hands_needed;
    if (obj->item_type == ITEM_INSTRUMENT && IS_SET (obj->value[0], A) && ch->size < SIZE_LARGE)
      --hands_needed;
  }
  if (hands_needed && (obj = get_eq_char_new(ch,ITEM_WIELD)))
  {
    if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
    {
       if (show_messages)
         act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
       return FALSE;
    }
     --hands_needed;
     if (!IS_NPC (ch) && ch->size < SIZE_LARGE && IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS))
       --hands_needed;
  }
  if (hands_needed && (obj = get_eq_char_new(ch,ITEM_WIELD)))
  {
    if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
    {
      if (show_messages)
	act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
      return FALSE;
    }
    --hands_needed;
    if (!IS_NPC (ch) && ch->size < SIZE_LARGE && IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS))
      --hands_needed;
  }
  //if somehow, beyond my understanding, hands_needed != 0, you can't wear it so return false here
  if (hands_needed > 0)
    return FALSE;
  
  return TRUE;
}

//Iblis - new function to deal with multislot eq
bool all_slots_free(CHAR_DATA *ch, OBJ_DATA *obj_to_wear,int hand)
{
  long i;
  OBJ_DATA *obj;
  for (i = 2; i <= MAX_WEAR_FLAG;i = i<<1)
    {
      if (!(obj_to_wear->item_type == ITEM_LIGHT && i == ITEM_WEAR_LIGHT))
      {
        if (!IS_SET(obj_to_wear->wear_flags,i))
          continue;
      }
      if (i == ITEM_NO_SAC || i == ITEM_TWO_HANDS || i == ITEM_TAKE)
        continue;
      if ((i == ITEM_WEAR_BODY || i == ITEM_WEAR_LEGS) && is_affected(ch,skill_lookup("swell")))
	{
	  return FALSE;
	}
      if (i == ITEM_WIELD && get_skill(ch,gsn_dual_wield) > 0)
      {
	if (!get_eq_char_new(ch,ITEM_WIELD) || !get_eq_char_new(ch,ITEM_TWO_HANDS))
  	  continue;
      }
      if (i == ITEM_WIELD && hand == HAND_LEFT)
        i = ITEM_TWO_HANDS;
      if ((obj = get_eq_char_new (ch, i)) != NULL)
      {
       if (i == ITEM_WEAR_NECK && !IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_NECK))
         obj = get_eq_char_new (ch,ITEM_WEAR_ANOTHER_NECK);
       else if (i == ITEM_WEAR_FINGER && !IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_FINGER))
         obj = get_eq_char_new (ch,ITEM_WEAR_ANOTHER_FINGER);
       else if (i == ITEM_WEAR_WRIST && !IS_SET(obj->wear_flags, ITEM_WEAR_ANOTHER_WRIST))
         obj = get_eq_char_new (ch, ITEM_WEAR_ANOTHER_WRIST);
       else if (i == ITEM_WIELD && !IS_NPC(ch) && obj_to_wear->item_type == ITEM_WEAPON && ch->size < SIZE_LARGE && IS_WEAPON_STAT (obj_to_wear, WEAPON_TWO_HANDS))
	 obj = get_eq_char_new (ch, ITEM_TWO_HANDS);
       if (obj != NULL)
	return FALSE;
      }
      /*      if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
	{
	  //The following three checks deal with objects that could be in two locations, that do not
	  //take up both of those locations.
	  if (i == ITEM_WEAR_NECK && !IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_NECK))
	    obj = get_eq_char_new (ch,ITEM_WEAR_ANOTHER_NECK);
	  else if (i == ITEM_WEAR_FINGER && !IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_FINGER))
	    obj = get_eq_char_new (ch,ITEM_WEAR_ANOTHER_FINGER);
	  else if (i == ITEM_WEAR_WRIST && !IS_SET(obj->wear_flags, ITEM_WEAR_ANOTHER_WRIST))
	    obj = get_eq_char_new (ch, ITEM_WEAR_ANOTHER_WRIST);
	  if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
	    {
	      if (show_messages)
		act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
	      return FALSE;
	    }
	    }*/
    }
  return TRUE;
}

/*bool remove_and_equip_char(CHAR_DATA *ch, OBJ_DATA *obj_to_wear, bool show_messages)
{
  remove_and_equip_char_hand(ch,obj_to_wear,show_messages,0);
}*/

bool remove_and_equip_char_hand(CHAR_DATA *ch, OBJ_DATA *obj_to_wear, bool show_messages, int hand)
{
  long i=0,newslot=0;
  OBJ_DATA *obj = NULL;
  int hands_needed=0;
  obj_to_wear->wear_loc = 0;
  for (i = 2; i <= MAX_WEAR_FLAG;i = i<<1)
    {
      if ((i == ITEM_WEAR_BODY || i == ITEM_WEAR_LEGS) && is_affected(ch,skill_lookup("swell")))
	{
	  if (show_messages)
	    send_to_char("You're too swollen to wear that.\n\r",ch);
	  return FALSE;
	}
      if (!IS_SET(obj_to_wear->wear_flags,i) && !(obj_to_wear->item_type == ITEM_LIGHT && i == ITEM_WEAR_LIGHT))
	continue;
      if (i == ITEM_NO_SAC || i == ITEM_TWO_HANDS || i == ITEM_TAKE)
	continue;
      if (i == ITEM_WIELD && hand == HAND_LEFT)
        i = ITEM_TWO_HANDS;
      
      if ((obj = get_eq_char_new (ch, i)) == NULL)
      {
        obj_to_wear->wear_loc |= i;
	if (i == ITEM_WIELD && !IS_NPC(ch) && obj_to_wear->item_type == ITEM_WEAPON && ch->size < SIZE_LARGE && IS_WEAPON_STAT (obj_to_wear, WEAPON_TWO_HANDS))
 	  obj_to_wear->wear_loc |= ITEM_TWO_HANDS;
	if (i == ITEM_TWO_HANDS && !IS_NPC(ch) && obj_to_wear->item_type == ITEM_WEAPON && ch->size < SIZE_LARGE && IS_WEAPON_STAT (obj_to_wear, WEAPON_TWO_HANDS))
  	   obj_to_wear->wear_loc |= ITEM_WIELD;
	continue;
      }
      newslot = -1;
      if (i == ITEM_WEAR_NECK && (get_eq_char_new (ch,ITEM_WEAR_ANOTHER_NECK) == NULL))
      {
	obj_to_wear->wear_loc |= ITEM_WEAR_ANOTHER_NECK;
	continue;
      }
      if (i == ITEM_WEAR_FINGER && (get_eq_char_new (ch,ITEM_WEAR_ANOTHER_FINGER) == NULL))
      {
        obj_to_wear->wear_loc |= ITEM_WEAR_ANOTHER_FINGER;
	continue;
      }
      if (i == ITEM_WEAR_WRIST && (get_eq_char_new (ch,ITEM_WEAR_ANOTHER_WRIST) == NULL))
      {
        obj_to_wear->wear_loc |= ITEM_WEAR_ANOTHER_WRIST;
        continue;
      }
      if ((i == ITEM_WIELD || i == ITEM_TWO_HANDS) && !(!IS_NPC(ch) && obj_to_wear->item_type == ITEM_WEAPON && ch->size < SIZE_LARGE && IS_WEAPON_STAT (obj_to_wear, WEAPON_TWO_HANDS)))
	{
	  if (get_skill(ch,gsn_dual_wield) == 0 && hand != HAND_AMBIDEXTROUS)
	    {
	 //     if (hand == HAND_LEFT)
	//	obj = get_eq_char_new (ch,ITEM_WIELD);
	//      else obj = get_eq_char_new (ch,ITEM_TWO_HANDS);
	      //handle the noremove with the code below
	    }
	  else
	  {
	    if (hand != HAND_LEFT)
	    {
	      //The if below deals with putting it in the other hand if this one is full
	      if (get_eq_char_new(ch,ITEM_TWO_HANDS) == NULL && hands_used(ch) <= 1)
	      {
	        obj_to_wear->wear_loc |= ITEM_TWO_HANDS;
		continue;
	      }
	      /*obj = get_eq_char_new (ch,ITEM_WIELD);
	      if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
		{
		  if (show_messages && get_eq_char_new (ch,ITEM_TWO_HANDS) != NULL)
		    act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
		  obj = get_eq_char_new(ch,ITEM_TWO_HANDS);
		}*/
	    }
            else
	    {
	    //The if below deals with putting it in the other hand if this one is full
	      if (get_eq_char_new(ch,ITEM_WIELD) == NULL && hands_used(ch) <= 1)
	      {
	        obj_to_wear->wear_loc |= ITEM_WIELD;
	        continue;
	      }
/*	    obj = get_eq_char_new (ch,ITEM_TWO_HANDS);
	    if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
	      {
		if (show_messages && get_eq_char_new (ch,ITEM_WIELD) != NULL)
		  act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
		obj = get_eq_char_new(ch,ITEM_WIELD);
	      }*/
	    }
	  }
	}

	
	     
      
      if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
	{
	  //The following three checks deal with objects that could be in two locations, that do not
	  //take up both of those locations.
//	  if (show_messages)
 // 	    act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
	  if (i == ITEM_WEAR_NECK && !IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_NECK))
		  newslot = ITEM_WEAR_ANOTHER_NECK;
	  else if (i == ITEM_WEAR_FINGER && !IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_FINGER))
	    newslot = ITEM_WEAR_ANOTHER_FINGER;
	  else if (i == ITEM_WEAR_WRIST && !IS_SET(obj->wear_flags, ITEM_WEAR_ANOTHER_WRIST))
	    newslot = ITEM_WEAR_ANOTHER_WRIST;
	  //else newslot = -1;
	  if (newslot != -1)
  	    obj = get_eq_char_new(ch,newslot);
	  else 
	  {
  	    if (show_messages)
	      act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
	    return FALSE;
	  }
	  if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
	    {
	      if (show_messages)
		act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
	      return FALSE;
	    }
	}
      unequip_char(ch,obj);
      if (obj->item_type == ITEM_WEAPON && IS_WEAPON_STAT(obj,WEAPON_HOLY_ACTIVATED))
	REMOVE_BIT(obj->value[4],WEAPON_HOLY_ACTIVATED);
      if (obj->item_type == ITEM_POLE)
	obj->value[1] = 0;
      act ("$n stops using $p.", ch, obj, NULL, TO_ROOM);
      act ("You stop using $p.", ch, obj, NULL, TO_CHAR);
      if (i == ITEM_WIELD &&!IS_NPC(ch) && obj->item_type == ITEM_WEAPON && ch->size < SIZE_LARGE 
		      && IS_WEAPON_STAT (obj,WEAPON_TWO_HANDS))
        obj_to_wear->wear_loc |= ITEM_TWO_HANDS;
      if (i == ITEM_TWO_HANDS && !IS_NPC(ch) && obj->item_type == ITEM_WEAPON && ch->size < SIZE_LARGE 
		      && IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS))
        obj_to_wear->wear_loc |= ITEM_WIELD;
      if (newslot == -1)
        obj_to_wear->wear_loc |= i;
      else obj_to_wear->wear_loc |= newslot;
    }

  if (hands_used(ch) <= 2)
    return TRUE;
  hands_needed = hands_used(ch)-2;
  if (hands_needed && (obj = get_eq_char_new(ch,ITEM_WEAR_SHIELD)) && obj != obj_to_wear)
    {
      if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
	{
	  if (show_messages)
	    act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
	  return FALSE;
	}
      --hands_needed;
    }
  if (hands_needed && (obj = get_eq_char_new(ch,ITEM_HOLD)) && obj != obj_to_wear)
    {
      if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
	{
	  if (show_messages)
	    act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
	  return FALSE;
	}
      --hands_needed;
      if (obj->item_type == ITEM_INSTRUMENT && IS_SET (obj->value[0], A) && ch->size < SIZE_LARGE)
	--hands_needed;
    }
  if (hands_needed && (obj = get_eq_char_new(ch,ITEM_WIELD)) && obj != obj_to_wear)
    {
      if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
	{
	  if (show_messages)
	    act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
	  return FALSE;
	}
      --hands_needed;
      if (!IS_NPC (ch) && ch->size < SIZE_LARGE && IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS))
	--hands_needed;
    }
  if (hands_needed && (obj = get_eq_char_new(ch,ITEM_WIELD)) && obj != obj_to_wear)
    {
      if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
	{
	  if (show_messages)
	    act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
	  return FALSE;
	}
      --hands_needed;
      if (!IS_NPC (ch) && ch->size < SIZE_LARGE && IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS))
	--hands_needed;
    }
  //if somehow, beyond my understanding, hands_needed != 0, you can't wear it so return false here
  if (hands_needed > 0)
    return FALSE;


  return TRUE;
}


//
// Equip a char with an obj.
//
void equip_char (CHAR_DATA * ch, OBJ_DATA * obj, long iWear)
{
  AFFECT_DATA *paf;
  int i;
  if (get_eq_char (ch, iWear) != NULL)
    return;

  // support alignment/Class/race specific equipment
  if ((!char_can_wear (ch, obj) && (ch->race != PC_RACE_AVATAR))
      || ((IS_SET(obj->extra_flags[1],ITEM_NOAVATAR)) && ch->race == PC_RACE_AVATAR)
      || ((IS_SET(obj->extra_flags[1],ITEM_NOKALIAN)) && ch->race == PC_RACE_KALIAN))
    {
      act ("You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR);
      act ("$n is zapped by $p and drops it.", ch, obj, NULL, TO_ROOM);
      obj_from_char (obj);
      obj_to_room (obj, ch->in_room);
      obj->wear_loc = -1;
      //Iblis - 3/11/04 - Antiduping Code
      if (ch && !IS_NPC(ch))
        save_char_obj (ch);
      return;
    }

  //DEAL WITH AC PROPERLY!! PICK THE
  for (i = 0; i < 4; i++)
    ch->armor[i] -= apply_ac (obj, iWear, i);
  for (i = 0; wear_conversion_table[i].wear_loc != -2; ++i)
  {
	  if (wear_conversion_table[i].wear_loc == iWear)
	  {
		  obj->wear_loc = wear_conversion_table[i].wear_flag;
		  break;
	  }
  }
//  obj->wear_loc = iWear;
  if (!obj->enchanted)
    for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
      if (paf->location != APPLY_SPELL_AFFECT)
	{
	  affect_modify (ch, paf, TRUE);
	}
  for (paf = obj->affected; paf != NULL; paf = paf->next)
    if (paf->location == APPLY_SPELL_AFFECT)
      {
	paf->permaff = TRUE;
	switch (paf->bitvector)
	  {
	  case A:
	    send_to_char ("You are blinded!\n\r", ch);
	    act ("$n appears to be blinded.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case B:
	    send_to_char ("You fade out of existence.\n\r", ch);
	    act ("$n fades out of existence.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case C:
	  case D:
	  case E:
	  case G:
	    send_to_char ("Your eyes tingle.\n\r", ch);
	    break;
	  case F:
	    send_to_char ("Your awareness improves.\n\r", ch);
	    break;
	  case H:
	    act ("$n is surrounded by a white aura.", ch, NULL, NULL,
		 TO_ROOM);
	    send_to_char ("You are surrounded by a white aura.\n\r", ch);
	    break;
	  case I:
	    send_to_char ("You are surrounded by a pink outline.\n\r", ch);
	    act ("$n is surrounded by a pink outline.", ch, NULL,
		 NULL, TO_ROOM);
	    break;
	  case J:
	    send_to_char ("Your eyes glow red.\n\r", ch);
	    act ("$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM);
	    break;
	  case K:
	    send_to_char ("You feel unclean.\n\r", ch);
	    break;
	  case M:
	    send_to_char ("You feel very sick.\n\r", ch);
	    act ("$n looks very ill.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case N:
	    send_to_char ("You feel holy and pure.\n\r", ch);
	    break;
	  case O:
	    send_to_char ("You feel aligned with darkness.\n\r", ch);
	    break;
	  case P:
	    send_to_char ("You begin to move silently.\n\r", ch);
	    break;
	  case Q:
	    send_to_char ("You hide in the shadows.\n\r", ch);
	    break;
	  case R:
	    send_to_char ("You feel very sleepy ..... zzzzzz.\n\r", ch);
	    act ("$n goes to sleep.", ch, NULL, NULL, TO_ROOM);
	    ch->position = POS_SLEEPING;
	    break;
	  case T:
	    send_to_char ("Your feet rise off the ground.\n\r", ch);
	    act ("$n's feet rise off the ground.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case U:
	    act ("$n turns translucent.", ch, NULL, NULL, TO_ROOM);
	    send_to_char ("You turn translucent.\n\r", ch);
	    break;
	  case V:
	    send_to_char ("You feel yourself moving more quickly.\n\r", ch);
	    act ("$n is moving more quickly.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case X:
            send_to_char
              ("You scream in agony as plague sores erupt from your skin.\n\r",
               ch);
            act ("$n screams in agony as plague sores erupt from $s skin.",
                 ch, NULL, NULL, TO_ROOM);
            break;
	  case Y:
	    send_to_char ("You feel your strength slip away.\n\r", ch);
	    act ("$n looks tired and weak.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case bb:
	    send_to_char ("Your lungs take the form of gills.\n\r", ch);
	    act ("$n turns into a fish.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case dd:
	    send_to_char ("You feel yourself slowing d o w n...\n\r", ch);
	    act ("$n starts to move in slow motion.", ch, NULL, NULL,
		 TO_ROOM);
	    break;
	  case ee:
	    send_to_char ("Your lungs take the form of gills.\n\r", ch);
	    act ("$n turns into a fish.", ch, NULL, NULL, TO_ROOM);
	    break;
	  }
	if (ch->affected != NULL)
	  affect_strip (ch, paf->type);
	REMOVE_BIT (ch->affected_by, paf->bitvector);
	affect_to_char (ch, paf);
      }
  
    else
      affect_modify (ch, paf, TRUE);
  if ((obj->item_type == ITEM_LIGHT
       && obj->value[2] != 0 && ch->in_room != NULL)
      && !(!IS_NPC(ch) && ch->pcdata->flaming))
    ++ch->in_room->light;
  
  trip_triggers(ch, OBJ_TRIG_WEAR, obj, NULL, OT_SPEC_NONE);
  
  return;
}

//
// Equip a char with an obj.
//
bool equip_char_new (CHAR_DATA * ch, OBJ_DATA * obj)
{
  return equip_char_new_hand(ch,obj,0);
}

bool equip_char_new_hand (CHAR_DATA * ch, OBJ_DATA * obj, int hand)
{
  AFFECT_DATA *paf;
  int i;
//  if (get_eq_char_new (ch, iWear) != NULL)
//    return;

  
  // support alignment/Class/race specific equipment
  if ((!char_can_wear (ch, obj) && (ch->race != PC_RACE_AVATAR))
		  || ((IS_SET(obj->extra_flags[1],ITEM_NOAVATAR)) && ch->race == PC_RACE_AVATAR)
		  || ((IS_SET(obj->extra_flags[1],ITEM_NOKALIAN)) && ch->race == PC_RACE_KALIAN))
    {
      act ("You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR);
      act ("$n is zapped by $p and drops it.", ch, obj, NULL, TO_ROOM);
      obj_from_char (obj);
      obj_to_room (obj, ch->in_room);
      obj->wear_loc = -1;
      //Iblis - 3/11/04 - Antiduping Code
      if (ch && !IS_NPC(ch))
        save_char_obj (ch);
      return FALSE;
    }

    remove_and_equip_char_hand(ch,obj,TRUE,hand);

//  obj->wear_loc = 0;
//  obj->wear_loc = iWear;
  
  //THIS DOES NOT DEAL PROPERLY WITH RINGS/NECK/WRIST
  
  /*  obj->wear_loc = obj->wear_flags;
  if (IS_SET(obj->wear_loc,ITEM_TAKE))
    obj->wear_loc -= ITEM_TAKE;
  if (IS_SET(obj->wear_loc,ITEM_NO_SAC))
    obj->wear_loc -= ITEM_NO_SAC;
  if (IS_SET(obj->wear_loc,ITEM_TWO_HANDS))
  obj->wear_loc -= ITEM_TWO_HANDS;*/

  //DEAL WITH AC PROPERLY!! PICK THE 
  for (i = 0; i < 4; i++)
    ch->armor[i] -= apply_ac_new (obj, i);
  //obj->wear_loc = iWear;
  if (!obj->enchanted)
    for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
      if (paf->location != APPLY_SPELL_AFFECT)
	{
	  affect_modify (ch, paf, TRUE);
	}
  for (paf = obj->affected; paf != NULL; paf = paf->next)
    if (paf->location == APPLY_SPELL_AFFECT)
      {
	paf->permaff = TRUE;
	switch (paf->bitvector)
	  {
	  case A:
	    send_to_char ("You are blinded!\n\r", ch);
	    act ("$n appears to be blinded.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case B:
	    send_to_char ("You fade out of existence.\n\r", ch);
	    act ("$n fades out of existence.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case C:
	  case D:
	  case E:
	  case G:
	    send_to_char ("Your eyes tingle.\n\r", ch);
	    break;
	  case F:
	    send_to_char ("Your awareness improves.\n\r", ch);
	    break;
	  case H:
	    act ("$n is surrounded by a white aura.", ch, NULL, NULL,
		 TO_ROOM);
	    send_to_char ("You are surrounded by a white aura.\n\r", ch);
	    break;
	  case I:
	    send_to_char ("You are surrounded by a pink outline.\n\r", ch);
	    act ("$n is surrounded by a pink outline.", ch, NULL,
		 NULL, TO_ROOM);
	    break;
	  case J:
	    send_to_char ("Your eyes glow red.\n\r", ch);
	    act ("$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM);
	    break;
	  case K:
	    send_to_char ("You feel unclean.\n\r", ch);
	    break;
	  case M:
	    send_to_char ("You feel very sick.\n\r", ch);
	    act ("$n looks very ill.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case N:
	    send_to_char ("You feel holy and pure.\n\r", ch);
	    break;
	  case O:
	    send_to_char ("You feel aligned with darkness.\n\r", ch);
	    break;
	  case P:
	    send_to_char ("You begin to move silently.\n\r", ch);
	    break;
	  case Q:
	    send_to_char ("You hide in the shadows.\n\r", ch);
	    break;
	  case R:
	    send_to_char ("You feel very sleepy ..... zzzzzz.\n\r", ch);
	    act ("$n goes to sleep.", ch, NULL, NULL, TO_ROOM);
	    ch->position = POS_SLEEPING;
	    break;
	  case T:
	    send_to_char ("Your feet rise off the ground.\n\r", ch);
	    act ("$n's feet rise off the ground.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case U:
	    act ("$n turns translucent.", ch, NULL, NULL, TO_ROOM);
	    send_to_char ("You turn translucent.\n\r", ch);
	    break;
	  case V:
	    send_to_char ("You feel yourself moving more quickly.\n\r", ch);
	    act ("$n is moving more quickly.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case X:
	    send_to_char
	      ("You scream in agony as plague sores erupt from your skin.\n\r",
	       ch);
	    act ("$n screams in agony as plague sores erupt from $s skin.",
		 ch, NULL, NULL, TO_ROOM);
	    break;
	  case Y:
	    send_to_char ("You feel your strength slip away.\n\r", ch);
	    act ("$n looks tired and weak.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case bb:
	    send_to_char ("Your lungs take the form of gills.\n\r", ch);
	    act ("$n turns into a fish.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case dd:
	    send_to_char ("You feel yourself slowing d o w n...\n\r", ch);
	    act ("$n starts to move in slow motion.", ch, NULL, NULL,
		 TO_ROOM);
	    break;
	  case ee:
	    send_to_char ("Your lungs take the form of gills.\n\r", ch);
	    act ("$n turns into a fish.", ch, NULL, NULL, TO_ROOM);
	    break;
	  }
	if (ch->affected != NULL)
	  affect_strip (ch, paf->type);
	REMOVE_BIT (ch->affected_by, paf->bitvector);
	affect_to_char (ch, paf);
      }

    else
      affect_modify (ch, paf, TRUE);
  if ((obj->item_type == ITEM_LIGHT
      && obj->value[2] != 0 && ch->in_room != NULL)
	  && !(!IS_NPC(ch) && ch->pcdata->flaming))
    ++ch->in_room->light;

  trip_triggers(ch, OBJ_TRIG_WEAR, obj, NULL, OT_SPEC_NONE);
  
  return TRUE;
}

bool equip_char_slot_new (CHAR_DATA * ch, OBJ_DATA * obj, long slot)
{
  AFFECT_DATA *paf;
  int i;
  if (slot == WEAR_NONE)
    return FALSE;
  //  if (get_eq_char_new (ch, iWear) != NULL)
  //    return;

  // support alignment/Class/race specific equipment
  if ((!char_can_wear (ch, obj) && (ch->race != PC_RACE_AVATAR))
      || ((IS_SET(obj->extra_flags[1],ITEM_NOAVATAR)) && ch->race == PC_RACE_AVATAR)
      || ((IS_SET(obj->extra_flags[1],ITEM_NOKALIAN)) && ch->race == PC_RACE_KALIAN))
    {
      act ("You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR);
      act ("$n is zapped by $p and drops it.", ch, obj, NULL, TO_ROOM);
      obj_from_char (obj);
      obj_to_room (obj, ch->in_room);
      obj->wear_loc = -1;
      //Iblis - 3/11/04 - Antiduping Code
      if (ch && !IS_NPC(ch))
        save_char_obj (ch);
      return FALSE;
    }

  //  remove_and_equip_char_hand(ch,obj,TRUE,hand);
  obj->wear_loc = slot;

  //  obj->wear_loc = 0;
  //  obj->wear_loc = iWear;

  //THIS DOES NOT DEAL PROPERLY WITH RINGS/NECK/WRIST

  /*  obj->wear_loc = obj->wear_flags;
  if (IS_SET(obj->wear_loc,ITEM_TAKE))
    obj->wear_loc -= ITEM_TAKE;
  if (IS_SET(obj->wear_loc,ITEM_NO_SAC))
    obj->wear_loc -= ITEM_NO_SAC;
  if (IS_SET(obj->wear_loc,ITEM_TWO_HANDS))
  obj->wear_loc -= ITEM_TWO_HANDS;*/

  //DEAL WITH AC PROPERLY!! PICK THE
  for (i = 0; i < 4; i++)
    ch->armor[i] -= apply_ac_new (obj, i);
  //obj->wear_loc = iWear;
  if (!obj->enchanted)
    for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
      if (paf->location != APPLY_SPELL_AFFECT)
        {
          affect_modify (ch, paf, TRUE);
        }
  for (paf = obj->affected; paf != NULL; paf = paf->next)
    if (paf->location == APPLY_SPELL_AFFECT)
      {
        paf->permaff = TRUE;
        switch (paf->bitvector)
          {
          case A:
            send_to_char ("You are blinded!\n\r", ch);
            act ("$n appears to be blinded.", ch, NULL, NULL, TO_ROOM);
            break;
          case B:
            send_to_char ("You fade out of existence.\n\r", ch);
            act ("$n fades out of existence.", ch, NULL, NULL, TO_ROOM);
            break;
          case C:
          case D:
          case E:
          case G:
            send_to_char ("Your eyes tingle.\n\r", ch);
            break;
          case F:
            send_to_char ("Your awareness improves.\n\r", ch);
            break;
          case H:
            act ("$n is surrounded by a white aura.", ch, NULL, NULL,
                 TO_ROOM);
            send_to_char ("You are surrounded by a white aura.\n\r", ch);
            break;
          case I:
            send_to_char ("You are surrounded by a pink outline.\n\r", ch);
            act ("$n is surrounded by a pink outline.", ch, NULL,
                 NULL, TO_ROOM);
            break;
          case J:
            send_to_char ("Your eyes glow red.\n\r", ch);
            act ("$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM);
            break;
	  case K:
            send_to_char ("You feel unclean.\n\r", ch);
            break;
          case M:
            send_to_char ("You feel very sick.\n\r", ch);
            act ("$n looks very ill.", ch, NULL, NULL, TO_ROOM);
            break;
          case N:
            send_to_char ("You feel holy and pure.\n\r", ch);
            break;
          case O:
            send_to_char ("You feel aligned with darkness.\n\r", ch);
            break;
          case P:
            send_to_char ("You begin to move silently.\n\r", ch);
            break;
          case Q:
            send_to_char ("You hide in the shadows.\n\r", ch);
            break;
          case R:
            send_to_char ("You feel very sleepy ..... zzzzzz.\n\r", ch);
            act ("$n goes to sleep.", ch, NULL, NULL, TO_ROOM);
            ch->position = POS_SLEEPING;
            break;
          case T:
            send_to_char ("Your feet rise off the ground.\n\r", ch);
            act ("$n's feet rise off the ground.", ch, NULL, NULL, TO_ROOM);
            break;
          case U:
            act ("$n turns translucent.", ch, NULL, NULL, TO_ROOM);
            send_to_char ("You turn translucent.\n\r", ch);
            break;
          case V:
            send_to_char ("You feel yourself moving more quickly.\n\r", ch);
            act ("$n is moving more quickly.", ch, NULL, NULL, TO_ROOM);
            break;
          case X:
            send_to_char
              ("You scream in agony as plague sores erupt from your skin.\n\r",
               ch);
            act ("$n screams in agony as plague sores erupt from $s skin.",
                 ch, NULL, NULL, TO_ROOM);
            break;
	  case Y:
            send_to_char ("You feel your strength slip away.\n\r", ch);
            act ("$n looks tired and weak.", ch, NULL, NULL, TO_ROOM);
            break;
          case bb:
            send_to_char ("Your lungs take the form of gills.\n\r", ch);
            act ("$n turns into a fish.", ch, NULL, NULL, TO_ROOM);
            break;
          case dd:
            send_to_char ("You feel yourself slowing d o w n...\n\r", ch);
            act ("$n starts to move in slow motion.", ch, NULL, NULL,
                 TO_ROOM);
            break;
          case ee:
            send_to_char ("Your lungs take the form of gills.\n\r", ch);
	    act ("$n turns into a fish.", ch, NULL, NULL, TO_ROOM);
            break;
          }
        if (ch->affected != NULL)
          affect_strip (ch, paf->type);
        REMOVE_BIT (ch->affected_by, paf->bitvector);
        affect_to_char (ch, paf);
      }

    else
      affect_modify (ch, paf, TRUE);
  if ((obj->item_type == ITEM_LIGHT
       && obj->value[2] != 0 && ch->in_room != NULL)
      && !(!IS_NPC(ch) && ch->pcdata->flaming))
    ++ch->in_room->light;

  //this is getting worn silently to fix something, so don't do obj triggers
//trip_triggers(ch, OBJ_TRIG_WEAR, obj, NULL, OT_SPEC_NONE);

  return TRUE;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char (CHAR_DATA * ch, OBJ_DATA * obj)
{
  AFFECT_DATA *paf = NULL;
  AFFECT_DATA *lpaf = NULL;
  AFFECT_DATA *lpaf_next = NULL;
  int i;
  if (!obj || obj->wear_loc == WEAR_NONE)
    return;
  for (i = 0; i < 4; i++)
    ch->armor[i] += apply_ac_new (obj, i);
  obj->wear_loc = -1;
  if (!obj->enchanted)
    {
      for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
      {
	if (paf->location == APPLY_SPELL_AFFECT)
	  {
	    for (lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next)
	      {
		lpaf_next = lpaf->next;
		if (!lpaf || !paf)
		{
			send_to_char("BUG! please see an imm.\n\r",ch);
			return;
		}
		if ((lpaf->type == paf->type) &&
		    (lpaf->level == paf->level) &&
		    (lpaf->location == APPLY_SPELL_AFFECT))
		  {
		    affect_remove (ch, lpaf);

		    //affect_strip (ch,lpaf->type);
		    lpaf_next = NULL;
		  }
	      }
	  }

	else
	  {
	    affect_modify (ch, paf, FALSE);
	    affect_check (ch, paf->where, paf->bitvector);
	  }
      }
    }
//  if (obj->enchanted)
//    {
      for (paf = obj->affected; paf != NULL; paf = paf->next)
	if (paf->location == APPLY_SPELL_AFFECT)
	  {
	    for (lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next)
	      {
		lpaf_next = lpaf->next;
		if ((lpaf->type == paf->type) &&
		    (lpaf->level == paf->level) &&
		    (lpaf->location == APPLY_SPELL_AFFECT))
		  {
		    if (paf->type > 0 && skill_table[paf->type].msg_off)
		      send_to_char (skill_table[paf->type].msg_off, ch);
		    send_to_char ("\n\r", ch);
		    affect_remove (ch, lpaf);

		    //affect_strip (ch,lpaf->type);
		    lpaf_next = NULL;
		  }
	      }
	  }

	else
	  {
	    affect_modify (ch, paf, FALSE);
	    affect_check (ch, paf->where, paf->bitvector);
	  }
  //  }
  for (paf = obj->affected; paf != NULL; paf = paf->next)
    if (paf->location == APPLY_SPELL_AFFECT)
      {
	paf->permaff = TRUE;
	switch (paf->bitvector)
	  {
	  case A:
	    send_to_char ("You are no longer blinded!\n\r", ch);
	    act ("$n is no longer blinded.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case B:
	    send_to_char ("You fade into existence.\n\r", ch);
	    act ("$n fades into existance.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case C:
	  case D:
	  case E:
	  case G:
	    send_to_char ("You no longer see invisible objects.\n\r", ch);
	    break;
	  case F:
	    send_to_char
	      ("You feel less aware of your surroundings.\n\r", ch);
	    break;
	  case H:
	    act ("The white aura around $n's body vanishes.", ch, NULL,
		 NULL, TO_ROOM);
	    send_to_char
	      ("The white aura around your body vanishes..\n\r", ch);
	    break;
	  case I:
	    send_to_char ("Your outline fades.\n\r", ch);
	    act ("$n's outline fades.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case J:
	    send_to_char ("You no longer see in the dark.\n\r", ch);

	    //act ("$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM);
	    break;
	  case K:
	    send_to_char ("The curse wears off..\n\r", ch);
	    break;
	  case M:
	    send_to_char ("You feel less sick.\n\r", ch);
	    act
	      ("$n visage lightens considerably, and begins to look much better.",
	       ch, NULL, NULL, TO_ROOM);
	    break;
	  case N:

	    //send_to_char ("You feel holy and pure.\n\r", ch);
	    break;
	  case O:

	    //send_to_char ("You feel aligned with darkness.\n\r", ch);
	    break;
	  case P:

	    //send_to_char ("You begin to move silently.\n\r", ch);
	    break;
	  case Q:

	    //send_to_char ("You hide in the shadows.\n\r", ch);
	    break;
	  case R:

	    //send_to_char ("You feel very sleepy ..... zzzzzz.\n\r", ch);
	    //act ("$n goes to sleep.", ch, NULL, NULL, TO_ROOM);
	    ch->position = POS_SLEEPING;
	    break;
	  case T:
	    send_to_char ("You fall to the ground.\n\r", ch);
	    act ("$n falls to the ground!", ch, NULL, NULL, TO_ROOM);
	    break;
	  case U:

	    //act ("$n turns translucent.", ch, NULL, NULL, TO_ROOM);
	    //send_to_char ("You turn translucent.\n\r", ch);
	    break;
	  case V:
	    send_to_char ("You are no longer moving so quickly.\n\r", ch);
	    act ("$n is no longer moving so quickly.", ch, NULL, NULL,
		 TO_ROOM);
	    break;
	  case X:

	    //send_to_char
	    //("You scream in agony as plague sores erupt from your skin.\n\r",  ch);
	    //act ("$n screams in agony as plague sores erupt from $s skin.",
	    //   ch, NULL, NULL, TO_ROOM);
	    break;
	  case Y:
	    send_to_char ("You feel your strength return.\n\r", ch);
	    act ("$n looks stronger.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case bb:

	    //send_to_char ("Your lungs take the form of gills.\n\r", ch);
	    //act ("$n turns into a fish.", ch, NULL, NULL, TO_ROOM);
	    break;
	  case dd:
	    send_to_char ("You are no longer moving so slowly.\n\r", ch);
	    act ("$n is no longer moving so slowly.", ch, NULL, NULL,
		 TO_ROOM);
	    break;
	  case ee:
	    send_to_char ("Your lungs revert back to normal form.\n\r", ch);

	    //act ("$n turns into a fish.", ch, NULL, NULL, TO_ROOM);
	    break;
	  }
	if (ch->affected != NULL)
	  affect_strip (ch, paf->type);
	REMOVE_BIT (ch->affected_by, paf->bitvector);

	//        affect_to_char (ch, paf);
      }
  // else
  //  affect_modify (ch, paf, TRUE);
  if (((obj->item_type == ITEM_LIGHT
      && obj->value[2] != 0 && ch->in_room != NULL) && !(!IS_NPC(ch) && ch->pcdata->flaming))
		  && ch->in_room->light > 0)
    --ch->in_room->light;

  trip_triggers(ch, OBJ_TRIG_REMOVE, obj, NULL, OT_SPEC_NONE);
  
  return;
}


/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list (OBJ_INDEX_DATA * pObjIndex, OBJ_DATA * list)
{
  OBJ_DATA *obj;
  int nMatch;
  nMatch = 0;
  for (obj = list; obj != NULL; obj = obj->next_content)
    {
      if (obj->pIndexData == pObjIndex)
	nMatch++;
    }
  return nMatch;
}


/*
 * Move an obj out of a room.
 */
void obj_from_room (OBJ_DATA * obj)
{
  ROOM_INDEX_DATA *in_room;
  CHAR_DATA *ch;
  OBJ_DATA* prev, *prev_next;
  if ((in_room = obj->in_room) == NULL)
    {
      char bugbuf[MAX_STRING_LENGTH];
      sprintf (bugbuf, "obj_from_room error: name(%s), short(%s).",
	       obj->name, obj->short_descr);
      bug (bugbuf, 0);
      return;
    }
  if (!(obj->item_type == ITEM_RAFT && paddling))
  {
    for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
      if (ch->on == obj)
        ch->on = NULL;
  }
  if (obj == in_room->contents)
    {
      in_room->contents = obj->next_content;
    }

  else
    {
//      OBJ_DATA *prev;
      for (prev = in_room->contents; prev; prev = prev->next_content)
	{
	  if (prev->next_content == obj)
	    {
	      prev->next_content = obj->next_content;
	      break;
	    }
	}
      if (prev == NULL)
	{
	  bug ("Obj_from_room: obj not found.", 0);
	  return;
	}
    }

  if (obj->pIndexData->vnum == OBJ_VNUM_PENTAGRAM)
  {
    for (prev = obj->contains; prev != NULL; prev = prev_next)
    {
      prev_next = prev->next_content;
      obj_from_obj(prev);
      obj_to_room(prev,obj->in_room);
    }
  }
	  
  
  obj->in_room = NULL;
  obj->next_content = NULL;
  return;
}


/*
 * Move an obj into a room.
 */
void obj_to_room (OBJ_DATA * obj, ROOM_INDEX_DATA * pRoomIndex)
{
  obj->next_content = pRoomIndex->contents;
  pRoomIndex->contents = obj;
  obj->in_room = pRoomIndex;
  obj->carried_by = NULL;
  obj->in_obj = NULL;
  //Kind of a pointless check..how else would an object get there?
//  if (descriptor_list && pRoomIndex->people)
//    check_gravity_room_obj(pRoomIndex->people,obj);
  return;
}


/*
 * Move an object into an object.
 */
void obj_to_obj (OBJ_DATA * obj, OBJ_DATA * obj_to)
{
  obj->next_content = obj_to->contains;
  obj_to->contains = obj;
  obj->in_obj = obj_to;
  obj->in_room = NULL;
  obj->carried_by = NULL;
  if (obj_to->pIndexData->vnum == OBJ_VNUM_PIT)
    obj->cost = 0;
  for (; obj_to != NULL; obj_to = obj_to->in_obj)
    {
      if (obj_to->carried_by != NULL)
	{
	  if ((obj->item_type != ITEM_WEAPON
	      || obj->value[0] != WEAPON_ARROW
	      || obj_to->item_type != ITEM_QUIVER)
		  && (obj->item_type != ITEM_CARD || obj_to->item_type != ITEM_BINDER))
	    {
	      obj_to->carried_by->carry_number += get_obj_number (obj);
	      obj_to->carried_by->carry_weight +=
		get_obj_weight (obj) * WEIGHT_MULT (obj_to) / 100;
	    }
	}
    }
  return;
}


/*
 * Move an object into an object into a particular position
 * Useful ONLY for putting cards into binder/decks
 * Pass a -1 as pos to put a card in the next available position
// * It returns TRUE if there are still cards in slots after the card
// * just inserted, and FALSE if not.
 */
bool obj_to_obj_pos (OBJ_DATA * obj, OBJ_DATA * obj_to, int pos)
{
  OBJ_DATA *pobj, *pobj_last=NULL;//, *pobj->next;
  int counter=1;
  //boolean rvalue = FALSE;
  pobj = obj_to->contains;
  //If the obj_to is empty
  if (pobj == NULL)
  {
    obj->next_content = NULL;
    obj_to->contains = obj;
    if (pos == -1)
      obj->bs_capacity[0] = 1;
    else obj->bs_capacity[0] = pos;
  }
  //If there is only one thing in the obj_to
  else if (pobj->next_content == NULL || (pos == -1 && pobj->bs_capacity[0] != 1) || (pos != -1 && pobj->bs_capacity[0] >= pos))
  {
    if (pos == pobj->bs_capacity[0])
      return FALSE;
		  
    //If the one thing is in slot 1	 
    if (pobj->bs_capacity[0] == 1 || (pos != -1 && pobj->bs_capacity[0] < pos))
    {
      obj->next_content = pobj->next_content;
      pobj->next_content = obj;
      if (pos == -1)
        obj->bs_capacity[0] = 2; //Its now the second thing in the obj_to 
      else obj->bs_capacity[0] = pos;
    }
    //the one thing is NOT in slot 1
    else
    {
/*      if (pos == -1 || pos < pobj->bs_capacity[0])
      {*/
        obj_to->contains = obj;
        obj->next_content = pobj;
	if (pos == -1)
          obj->bs_capacity[0] = 1;
	else obj->bs_capacity[0] = pos;
     // }
    }
      
  }	  
  //Otherwise, recurse through the obj_to and put the obj in the next available slot
  else 
  {
    for (pobj = obj_to->contains;pobj != NULL;pobj_last = pobj, pobj = pobj->next_content, ++counter)
    {
      if (pos != -1 && pobj->bs_capacity[0] == pos)
        return FALSE;
      if ((pos != -1 && pobj->bs_capacity[0] > pos) ||
          (pos == -1 && pobj->bs_capacity[0] != counter))
      {
        obj->next_content = pobj;
        pobj_last->next_content = obj;
    //    if (obj->next_content != NULL)
    //      rvalue = TRUE;
        if (pos == -1)
	  obj->bs_capacity[0] = counter;
	else obj->bs_capacity[0] = pos;
        break;
      }
    }
    if (pobj == NULL)
    {
      obj->next_content = NULL;
      pobj_last->next_content = obj;
      if (pos == -1)
        obj->bs_capacity[0] = counter;
      else obj->bs_capacity[0] = pos;
    }

  }
		  
//  obj->next_content = obj_to->contains;
//  obj_to->contains = obj;
  obj->in_obj = obj_to;
  obj->in_room = NULL;
  obj->carried_by = NULL;
  if (obj_to->pIndexData->vnum == OBJ_VNUM_PIT)
    obj->cost = 0;
  for (; obj_to != NULL; obj_to = obj_to->in_obj)
    {
      if (obj_to->carried_by != NULL)
        {
          if ((obj->item_type != ITEM_WEAPON
              || obj->value[0] != WEAPON_ARROW
              || obj_to->item_type != ITEM_QUIVER)
	      && (obj->item_type != ITEM_CARD || obj_to->item_type != ITEM_BINDER))
            {
              obj_to->carried_by->carry_number += get_obj_number (obj);
              obj_to->carried_by->carry_weight +=
                get_obj_weight (obj) * WEIGHT_MULT (obj_to) / 100;
            }
        }
    }
  return TRUE;
}



/*
 * Move an object out of an object.
 */
void obj_from_obj (OBJ_DATA * obj)
{
  OBJ_DATA *obj_from;
  if ((obj_from = obj->in_obj) == NULL)
    {
      bug ("Obj_from_obj: null obj_from.", 0);
      return;
    }
  if (obj == obj_from->contains)
    {
      obj_from->contains = obj->next_content;
    }

  else
    {
      OBJ_DATA *prev;
      for (prev = obj_from->contains; prev; prev = prev->next_content)
	{
	  if (prev->next_content == obj)
	    {
	      prev->next_content = obj->next_content;
	      break;
	    }
	}
      if (prev == NULL)
	{
	  bug ("Obj_from_obj: obj not found.", 0);
	  return;
	}
    }
  obj->next_content = NULL;
  obj->in_obj = NULL;
  for (; obj_from != NULL; obj_from = obj_from->in_obj)
    {
      if (obj_from->carried_by != NULL)
	{
	  if ((obj->item_type != ITEM_WEAPON || obj->value[0] != WEAPON_ARROW || obj_from->item_type != ITEM_QUIVER) &&
	     (obj->item_type != ITEM_CARD || obj_from->item_type != ITEM_BINDER))
	  {
	    obj_from->carried_by->carry_number -= get_obj_number (obj);
	    obj_from->carried_by->carry_weight -= get_obj_weight (obj)
	      * WEIGHT_MULT (obj_from) / 100;
	  }
	}
    }
  return;
}

void extract_scent (SCENT_DATA * sc)
{
  SCENT_DATA *prev;
  if (sc == scents)
    {
      scents = sc->next;
    }

  else
    {
      for (prev = scents; prev; prev = prev->next)
	if (prev->next == sc)
	  {
	    prev->next = sc->next;
	    break;
	  }
    }
  if (sc == sc->in_room->scents)
    {
      sc->in_room->scents = sc->next_in_room;
    }

  else
    {
      for (prev = sc->in_room->scents; prev; prev = prev->next_in_room)
	{
	  if (prev->next_in_room == sc)
	    {
	      prev->next_in_room = sc->next_in_room;
	      break;
	    }
	}
    }
  free_scent (sc);
}


/*
 * Extract an obj from the world.
 */
void extract_obj (OBJ_DATA * obj)
{
  OBJ_DATA *obj_content;
  OBJ_DATA *obj_next;
  char tcbuf[15];


  if (obj == NULL)
  {
    bug("extracting null obj",0);
    return;
  }
  if (obj->set_by != NULL)
  {
	if (obj == obj->set_by->trap_list)
	{
      		obj->set_by->trap_list = obj->next_trap;
    	}
	else
    	{
        OBJ_DATA *prev;
      		for (prev = obj->set_by->trap_list; prev != NULL; prev = prev->next_trap)
		{
  			if (prev->next_trap == obj)
	    		{
		        	prev->next_trap = obj->next_trap;
		                break;
		        }
        	}
                if (prev == NULL)
		{
	  		bug ("Extract_obj: trap not found.", 0);
		  	return;
		}
    	}
  }
  // free up/fix up a portal for removal
  if (obj->item_type == ITEM_PORTAL
      && obj->value[3] >= ROOM_VNUM_HAVEN_START
      && obj->value[3] <= ROOM_VNUM_HAVEN_END)
    {
      ROOM_INDEX_DATA *location;
      CHAR_DATA *vch, *vch_next;
      OBJ_DATA *dobj, *dobj_next;
      location = get_room_index (obj->value[3]);
      if (location != NULL)
	{
	  for (vch = location->people; vch; vch = vch_next)
	    {
	      vch_next = vch->next_in_room;
	      send_to_char ("Your time in haven has come to an end.\n\r",
			    vch);
	      char_from_room (vch);
	      char_to_room (vch, obj->in_room);
	      do_look (vch, "auto");
	      act ("$n has arrived.", vch, NULL, NULL, TO_ROOM);
	      trip_triggers(vch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	      trap_check(vch,"room",vch->in_room,NULL);
	      sprintf(tcbuf,"%d",vch->in_room->vnum);
	      trip_triggers_arg(vch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
	      check_aggression(vch);
	    }
	  for (dobj = location->contents; dobj; dobj = dobj_next)
	    {
	      dobj_next = dobj->next_content;
	      extract_obj (dobj);
	    }
	  free_string (location->owner);
	  location->owner = str_dup ("");
	}			// end if location was null
    }				// end if object was a portal

  if (obj->pIndexData->vnum == OBJ_VNUM_BEACON)
  {
    CHAR_DATA* fch;
    for (fch = char_list;fch != NULL;fch = fch->next)
      {
	if (!IS_NPC(fch) && is_affected(fch,skill_lookup("beacon of the damned")))
	  {
	    affect_strip(fch,skill_lookup("beacon of the damned"));
	    send_to_char("The green pillar of fire on the horizon fades into nothingness.\n\r",fch); 
	  }
      }
  }
  if (obj->pIndexData->vnum == OBJ_VNUM_DARK_SPHERE)
  {
    ROOM_INDEX_DATA *rid;
    rid = get_room_index(obj->value[0]);
    if (rid != NULL)
    {
//	    char buf[MAX_STRING_LENGTH];
//	    sprintf(buf,"%d",obj->value[0]);
            REMOVE_BIT(rid->room_flags2,ROOM_ALWAYS_DARK);
    }
  }

  if (obj->pIndexData->vnum == OBJ_VNUM_DEFILE)
  {
	  ROOM_INDEX_DATA* rid;
	  rid = get_room_index(obj->value[0]);
	  if (rid != NULL)
	    rid->sector_type = obj->value[1];
  }

  if (obj->pIndexData->vnum == OBJ_VNUM_WARPED_SPACE)
  {
     ROOM_INDEX_DATA *rid;
     rid = get_room_index(obj->value[0]);
     if (rid != NULL)
     {
//       char buf[MAX_STRING_LENGTH];
 //      sprintf(buf,"%d",obj->value[0]);
       REMOVE_BIT(rid->room_flags2,ROOM_WARPED);
     }
  }

  //Iblis 4/12/04 - Made to check purging/saccing etc of items someone is currently carving
  //NOTE : It doesn't quit after finding it, because say bob starts carving, gives it to jim,
  //who stats carving, then drops and sacs it.  Complete mediation.
  if (obj->item_type == ITEM_WOODEN_INCONSTRUCTION)
  {
    CHAR_DATA *fch;
    for (fch = char_list;fch != NULL;fch = fch->next)
    {
      if (!IS_NPC(fch) && fch->pcdata && fch->pcdata->trade_skill_obj != NULL)
      {
	if (fch->pcdata->trade_skill_obj == obj)
  	  fch->pcdata->trade_skill_obj = NULL;
      }
    }
  }

    

  /*  // remove the object from where it was
  if (obj->in_room != NULL)
    {
      obj_from_room (obj);
    }

  else if (obj->carried_by != NULL)
    {
      obj_from_char (obj);
    }

  else if (obj->in_obj != NULL)
    {
      obj_from_obj (obj);
    }*/
  // remove anything that is in this object free/recycle contents
  for (obj_content = obj->contains; obj_content; obj_content = obj_next)
    {
      obj_next = obj_content->next_content;
      extract_obj (obj_content);
      }

  if (obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC)
    save_player_corpse(obj);

  // remove the object from where it was
  if (obj->in_room != NULL)
    {
      obj_from_room (obj);
    }

  else if (obj->carried_by != NULL)
    {
      obj_from_char (obj);
    }

  else if (obj->in_obj != NULL)
    {
      obj_from_obj (obj);
    }


  // ok, now remove the item from the object_list
  if (object_list == obj)
    {
      object_list = obj->next;
    }

  else
    {
      OBJ_DATA *prev;
      for (prev = object_list; prev != NULL; prev = prev->next)
	{
	  if (prev->next == obj)
	    {
	      prev->next = obj->next;
	      break;
	    }
	}
      if (prev == NULL)
	{
	  bug ("Extract_obj: obj %d not found.", obj->pIndexData->vnum);
	  return;
	}
    }

  // note that we have one less of these
  --obj->pIndexData->count;

  // ahh, free/recycle the memory
  free_obj (obj);
  return;
}


/*
 * Extract a char from the world.
 */
void extract_char (CHAR_DATA * ch, bool fPull)
{
  CHAR_DATA *wch;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next,*list;
  SCENT_DATA *sc, *sc_next;
  MEMORY_DATA *memd, *memd_next;
  ROOM_INDEX_DATA *location;
  char tcbuf[15];
  int count=0;
  if (ch->in_room == NULL)
    {
      if (ch->name != NULL)
	bug (ch->name, 0);
      bug ("Extract_char: NULL.", 0);
      return;
    }
  nuke_pets (ch);
  ch->pet = NULL;		/* just in case */
    for (list = ch->trap_list;list != NULL;list = list->next_trap)
        count++;
    for (;count >0;count--)
    {
        if (ch->trap_list != NULL)
        {
                obj_from_room(ch->trap_list );
                extract_obj(ch->trap_list );
        }
    }
 
  if (ch->contaminator)
    ch->contaminator = NULL;
  if (IS_SET(ch->act2,ACT_FAMILIAR))
  {
    for (wch = char_list; wch != NULL; wch = wch->next)
    {
      if (IS_NPC(wch))
        continue;
      if (wch->pcdata->familiar == ch)
      {
        wch->pcdata->familiar = NULL;
      }
    }
  }
  if (fPull)
    die_follower (ch);
  stop_fighting (ch, TRUE);
  location = ch->in_room;
  if (location != NULL)
    {
      char_from_room (ch);
      if (!IS_NPC(ch) && ch->pcdata->flaming)
	      ch->pcdata->flaming = 0;
      char_to_room (ch, get_room_index (ROOM_VNUM_LIMBO));
      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(ch,"room",ch->in_room,NULL);
      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
//      check_aggression(ch);
    }
//  if (ch->sword != NULL)
 //   {
  //    log_string("Extracting ch->sword from");
   //   log_string(ch->name);
    //  extract_obj (ch->sword);
   // }
  for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      if (!(!IS_NPC(ch) && fPull == FALSE && IS_SET(obj->extra_flags[1],ITEM_TATTOO)))
       extract_obj (obj);
    }
  if (location != NULL)
    {
      char_from_room (ch);
      char_to_room (ch, location);
      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(ch,"room",ch->in_room,NULL);
      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
 //     check_aggression(ch);
    }
  if (ch->riders != NULL)
    {
      ch->riders->position = POS_STANDING;
      ch->riders->mount = NULL;
    }
  if (ch->mount != NULL)
    {
      ch->mount->riders = NULL;
      ch->mount->mounter = NULL;
      ch->mount->afkmsg[0] = '\0';
    }
  char_from_room (ch);
  if (!fPull)
    {
      ROOM_INDEX_DATA *rid;

      //Iblis 4/04/04 - A quick dirty fix to fix any skill related affect problems upon death
/*      if (!IS_NPC(ch))
      {
         int i=0;
         for (i = 0; i < MAX_SKILL; i++)
         {
	   if (skill_table[i].name != NULL)
	   {
	     ch->pcdata->mod_learned[i] = 0;
	   }
	 }
      }	                                 */

      // Shinowlan 6/12/98 -- Bug: 7 If a PC dies in jail they reappear
      // in the altar.  They *should* reappear in their jail cell.
      // This checks to see if a character died in a jail cell, and if so
      // moves them back to that cell rather than the altar.
      if ((location->vnum >= ROOM_VNUM_JAIL_START) &&
	  (location->vnum <= ROOM_VNUM_JAIL_END))
	{
	  char_to_room (ch, location);
	  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	  trap_check(ch,"room",ch->in_room,NULL);
	  sprintf(tcbuf,"%d",ch->in_room->vnum);
	  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
	 // check_aggression(ch);
	  return;
	}
      // clannies that have declared their clan hall get
      // to be resurected in their clan hall
      if (get_clan_hall_ch (ch) != CLAN_BOGUS)
	{
	  rid = get_room_index (get_clan_hall_ch (ch));
	  if (rid != NULL)
	  {
	    char_to_room (ch, rid);
	    trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	    trap_check(ch,"room",ch->in_room,NULL);
	    sprintf(tcbuf,"%d",ch->in_room->vnum);
	    trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
	  //  check_aggression(ch);
	  }

	  else
	  {
	    char_to_room (ch, get_room_index (ROOM_VNUM_ALTAR));
	    trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	    trap_check(ch,"room",ch->in_room,NULL);
	    sprintf(tcbuf,"%d",ch->in_room->vnum);
	    trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
	    //check_aggression(ch);
	  }
	}

      else
      {
	char_to_room (ch, get_room_index (ROOM_VNUM_ALTAR));
	trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	trap_check(ch,"room",ch->in_room,NULL);
	sprintf(tcbuf,"%d",ch->in_room->vnum);
	trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
//	check_aggression(ch);
      }
      return;
    }
  if (IS_NPC (ch))
    --ch->pIndexData->count;
  if (IS_SET(ch->act2,ACT_QUESTMOB))
    {
      CHAR_DATA *wch;
      for (wch = char_list; wch != NULL; wch = wch->next)
	{
	  if (IS_NPC(wch))
	    continue;
	  if (wch->pcdata->quest_mob == ch)
	    {
	      wch->pcdata->quest_mob = NULL;
	      send_to_char("Alas, someone ELSE has killed the mob you were hunting for before you could reach it.\n\r",wch);
              send_to_char("Unfortunately, that means you have failed in your quest.\n\r",wch);
              wch->pcdata->quest_ticks = -10;
	      break;
	    }
	}
    }
  
  for (sc = scents; sc; sc = sc_next)
    {
      sc_next = sc->next;
      if (sc->player == ch)
	extract_scent (sc);
    }
  for (memd = memories; memd; memd = memd_next)
    {
      memd_next = memd->next;
      if (memd->player == ch || memd->mob == ch)
	extract_memory (memd);
    }
  if (ch->desc != NULL && ch->desc->original != NULL)
    {
      if (IS_SET(ch->desc->original->act2,ACT_SWITCHED))
        REMOVE_BIT(ch->desc->original->act2,ACT_SWITCHED);
/*        switched_return(ch,location);
      else */
        do_return (ch, "");
      ch->desc = NULL;
    }
  for (wch = char_list; wch != NULL; wch = wch->next)
    {
      if (wch->reply == ch)
	wch->reply = NULL;
      if (wch->stalking == ch)
	wch->stalking = NULL;
      if (wch->last_fought == ch)
	wch->last_fought = NULL;
    }
  if (ch == char_list)
    {
      char_list = ch->next;
    }

  else
    {
      CHAR_DATA *prev;
      for (prev = char_list; prev != NULL; prev = prev->next)
	{
	  if (prev->next == ch)
	    {
	      prev->next = ch->next;
	      break;
	    }
	}
      if (prev == NULL)
	{
	  bug ("Extract_char: char not found.", 0);
	  return;
	}
    }
  if (ch->desc != NULL)
    ch->desc->character = NULL;
  free_char (ch);
  return;
}


/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *rch;
  char *check;
  int number;
  int count;

  int count2;

  number = number_argument (argument, arg);
  count = 0;
  if (!str_cmp (arg, "self"))
    return ch;
  if (ch == NULL)
  {
      if ( is_name("farmer jon",  ch->name ))
      {
          bug ("***** DEBUG ****",0);
          bug ("CH IS NULL",0);
      }

    return NULL;
  }

  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if ( is_name("farmer jon",  ch->name ))
        {
           
            bug ("**** DEBUG get_char_room: name loop***",0);
            bug ("Searching for...",0);
            bug (arg,0);
            bug (rch->name,0);
            
            bug ("Isname?",0);
            if (is_name (arg, rch->name))
                bug (" Yep - name match", 0);
            
            if (str_cmp(arg, rch->name))
                bug ("str_cmp matches!",0);


            if (is_name ("kalus", rch->name))
              bug ("Kalus found!",0);
        }



      if (is_affected (rch, gsn_mask) && rch->mask != NULL)
	check = rch->mask;

      else
	check = rch->name;
      if (!can_see (ch, rch) || !is_name (arg, check))
	continue;
      if (++count == number)
	return rch;
    }
    if ( is_name("farmer jon",  ch->name ))
    {
      bug ("***** DEBUG ****",0);
      bug ("Loop finished - no match found",0);
    }
  return NULL;
}


/*
 * Find a char in the room.
 */
//Iblis 07/22/03 - Needed mainly for checking if a storekeeper is in
////the room.  Sidhe were masking as owner + a few chars and being able
////to list contents.  This is NOT good and is now FINALLY fixed.
CHAR_DATA *get_char_room_nomask (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *rch;
  char *check;
  int number;
  int count;
  number = number_argument (argument, arg);
  count = 0;
  if (!str_cmp (arg, "self"))
    return ch;
  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {

      check = rch->name;
      if (!can_see (ch, rch) || !is_name (arg, check))
	continue;
      if (++count == number)
	return rch;
    }
  return NULL;
}


/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *wch;
  char *check;
  int number;
  int count;
  if ((wch = get_char_room (ch, argument)) != NULL)
    return wch;
  number = number_argument (argument, arg);
  count = 0;
  for (wch = char_list; wch != NULL; wch = wch->next)
    {
      if (is_affected (wch, gsn_mask) && wch->mask != NULL)
	check = wch->mask;

      else
	check = wch->name;
      if (wch->in_room == NULL || !can_see (ch, wch) || !is_name (arg, check))
	continue;
      if (++count == number)
	return wch;
    }
  return NULL;
}

//Iblis - same as above, EXCEPT it ignore those blasted masks
CHAR_DATA *get_char_world2 (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *wch;
  int number;
  int count;
  if ((wch = get_char_room (ch, argument)) != NULL)
    return wch;
  number = number_argument (argument, arg);
  count = 0;
  for (wch = char_list; wch != NULL; wch = wch->next)
    {
      if (!is_name (arg, wch->name))
        continue;
      if (++count == number)
        return wch;
    }
  return NULL;
}



/*
 * Iblis - Find a char in the room only if their name matches EXACTLY
 */
CHAR_DATA *get_exact_pc_room (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *rch;
  if (ch == NULL)
    return NULL;
  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
       if (IS_NPC(rch) || str_cmp (argument, rch->name))
        continue;
       return rch;
    }
  return NULL;
}


/*
 * Iblis - Find a char in the world only if their name matches EXACTLY
 */
CHAR_DATA *get_exact_pc_world (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *wch;
  if ((wch = get_exact_pc_room (ch, argument)) != NULL)
    return wch;
  for (wch = char_list; wch != NULL; wch = wch->next)
    {
      if (IS_NPC(wch) || wch->in_room == NULL || str_cmp (argument, wch->name))
        continue;
      return wch;
    }
  return NULL;
}



/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type (OBJ_INDEX_DATA * pObjIndex)
{
  OBJ_DATA *obj;
  for (obj = object_list; obj != NULL; obj = obj->next)
    {
      if (obj->pIndexData == pObjIndex)
	return obj;
    }
  return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list (CHAR_DATA * ch, char *argument, OBJ_DATA * list)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;
  number = number_argument (argument, arg);
  count = 0;
  for (obj = list; obj != NULL; obj = obj->next_content)
    {
      if (can_see_obj (ch, obj) && is_name (arg, obj->name))
	{
	  if (++count == number)
	    return obj;
	}
    }
  return NULL;
}


/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;
  number = number_argument (argument, arg);
  count = 0;
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc == WEAR_NONE
	  && (can_see_obj (ch, obj)) && is_name (arg, obj->name))
	{
	  if (++count == number)
	    return obj;
	}
    }
  return NULL;
}


/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;
  number = number_argument (argument, arg);
  count = 0;
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc != WEAR_NONE
	  && can_see_obj (ch, obj) && is_name (arg, obj->name))
	{
	  if (++count == number)
	    return obj;
	}
    }
  return NULL;
}


/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj;
  if (!ch || !ch->in_room)
    return NULL;
  obj = get_obj_list (ch, argument, ch->in_room->contents);
  if (obj != NULL)
    return obj;
  if ((obj = get_obj_carry (ch, argument)) != NULL)
    return obj;
  if ((obj = get_obj_wear (ch, argument)) != NULL)
    return obj;
  return NULL;
}


/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;
  if ((obj = get_obj_here (ch, argument)) != NULL)
    return obj;
  number = number_argument (argument, arg);
  count = 0;
  for (obj = object_list; obj != NULL; obj = obj->next)
    {
      if (can_see_obj (ch, obj) && is_name (arg, obj->name))
	{
	  if (++count == number)
	    return obj;
	}
    }
  return NULL;
}


/* deduct cost from a character */
void deduct_cost (CHAR_DATA * ch, int cost)
{
  int silver = 0, gold = 0;
  if (IS_IMMORTAL (ch))
    return;
  silver = UMIN (ch->silver, cost);
  if (silver < cost)
    {
      gold = ((cost - silver + 99) / 100);
      silver = cost - 100 * gold;
    }
  ch->gold -= gold;
  ch->silver -= silver;
  if (ch->gold < 0)
    {
      bug ("deduct costs: gold %d < 0", ch->gold);
      ch->gold = 0;
    }
  if (ch->silver < 0)
    {
      bug ("deduct costs: silver %d < 0", ch->silver);
      ch->silver = 0;
    }
}
void deduct_bank (CHAR_DATA * ch, int cost)
{
  int silver = 0, gold = 0;
  if (IS_NPC (ch) && IS_IMMORTAL (ch))
    return;
  silver = UMIN (ch->pcdata->bank_silver, cost);
  if (silver < cost)
    {
      gold = ((cost - silver + 99) / 100);
      silver = cost - 100 * gold;
    }
  ch->pcdata->bank_gold -= gold;
  ch->pcdata->bank_silver -= silver;
  if (ch->gold < 0)
    {
      bug ("deduct bank costs: gold %d < 0", ch->pcdata->bank_gold);
      ch->pcdata->bank_gold = 0;
    }
  if (ch->silver < 0)
    {
      bug ("deduct bank costs: silver %d < 0", ch->pcdata->bank_silver);
      ch->pcdata->bank_silver = 0;
    }
}


/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money (int gold, int silver)
{
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  if (gold < 0 || silver < 0 || (gold == 0 && silver == 0))
    {
      bug ("Create_money: zero or negative money.", UMIN (gold, silver));
      gold = UMAX (1, gold);
      silver = UMAX (1, silver);
    }
  if (gold == 0 && silver == 1)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SILVER_ONE), 0);
    }

  else if (gold == 1 && silver == 0)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_GOLD_ONE), 0);
    }

  else if (silver == 0)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_GOLD_SOME), 0);
      sprintf (buf, obj->short_descr, gold);
      free_string (obj->short_descr);
      obj->short_descr = str_dup (buf);
      obj->value[1] = gold;
      obj->cost = gold;
      obj->weight = gold / 5;
    }

  else if (gold == 0)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SILVER_SOME), 0);
      sprintf (buf, obj->short_descr, silver);
      free_string (obj->short_descr);
      obj->short_descr = str_dup (buf);
      obj->value[0] = silver;
      obj->cost = silver;
      obj->weight = silver / 20;
    }

  else
    {
      obj = create_object (get_obj_index (OBJ_VNUM_COINS), 0);
      sprintf (buf, obj->short_descr, silver, gold);
      free_string (obj->short_descr);
      obj->short_descr = str_dup (buf);
      obj->value[0] = silver;
      obj->value[1] = gold;
      obj->cost = 100 * gold + silver;
      obj->weight = gold / 5 + silver / 20;
    }
  return obj;
}


/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number (OBJ_DATA * obj)
{
  int number;
  if (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_PACK
      || obj->item_type == ITEM_MONEY || obj->item_type == ITEM_GEM
      || obj->item_type == ITEM_JEWELRY
      || obj->item_type == ITEM_CLAN_DONATION
      || obj->item_type == ITEM_PLAYER_DONATION
      || obj->item_type == ITEM_BINDER)
    number = 0;

  else
    number = 1;
  if (obj->item_type == ITEM_QUIVER)
    return 1;
  if (obj->item_type == ITEM_BINDER)
    return 0;
  for (obj = obj->contains; obj != NULL; obj = obj->next_content)
    {
      if (obj == obj->next_content)
	break;
      number += get_obj_number (obj);
    }
  return number;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight (OBJ_DATA * obj)
{
  int weight;
  OBJ_DATA *tobj;
  weight = obj->weight;
  if (IS_SET(obj->extra_flags[1],ITEM_WINGED))
    weight = 0;
  if (obj->item_type == ITEM_BINDER || obj->item_type == ITEM_QUIVER)
    return weight;
  for (tobj = obj->contains; tobj != NULL; tobj = tobj->next_content)
    weight += get_obj_weight (tobj) * WEIGHT_MULT (obj) / 100;
  return weight;
}

int get_true_weight (OBJ_DATA * obj)
{
  int weight;
  weight = obj->weight;
  if (IS_SET(obj->extra_flags[1],ITEM_WINGED))
    weight = 0;
  if (obj->item_type == ITEM_BINDER || obj->item_type == ITEM_QUIVER)
    return weight;
  for (obj = obj->contains; obj != NULL; obj = obj->next_content)
    weight += get_obj_weight (obj);
  return weight;
}


/*
 * True if room is dark.
 */
bool room_is_dark (ROOM_INDEX_DATA * pRoomIndex)
{
  if (IS_SET (pRoomIndex->room_flags2, ROOM_ALWAYS_DARK))
    return TRUE;
  if (pRoomIndex->light > 0)
    return FALSE;
  if (IS_SET (pRoomIndex->room_flags, ROOM_DARK))
    return TRUE;
  if (pRoomIndex->sector_type == SECT_INSIDE
      || pRoomIndex->sector_type == SECT_CITY)
    return FALSE;
  if (weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK)
    return TRUE;
  return FALSE;
}

bool is_room_owner (CHAR_DATA * ch, ROOM_INDEX_DATA * room)
{
  if (room->owner == NULL || room->owner[0] == '\0')
    return FALSE;
  return is_name (ch->name, room->owner);
}


/*
 * True if room is private.
 */
bool room_is_private (ROOM_INDEX_DATA * pRoomIndex)
{
  CHAR_DATA *rch;
  int count;

  /*  if (pRoomIndex->owner != NULL && pRoomIndex->owner[0] != '\0')
     return TRUE; */
  count = 0;
  for (rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room)
    count++;
  if (IS_SET (pRoomIndex->room_flags, ROOM_PRIVATE) && count >= 2)
    return TRUE;
  if (IS_SET (pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1)
    return TRUE;
  if (pRoomIndex->max_in_room != 0 && count >= pRoomIndex->max_in_room)
    return TRUE;
  return FALSE;
}


/* visibility on a room -- for entering and exits */
bool can_see_room (CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex)
{
  if (IS_SET (pRoomIndex->room_flags, ROOM_IMP_ONLY) &&
      (get_trust (ch) < MAX_LEVEL))
    return FALSE;
  if (!IS_IMMORTAL (ch) && IS_SET (pRoomIndex->room_flags, ROOM_GODS_ONLY))
    return FALSE;
  if (IS_SET(pRoomIndex->area->area_flags,AREA_IMP_ONLY) && get_trust(ch) < MAX_LEVEL)
    return FALSE;

  /* 
     if( !IS_IMMORTAL(ch) &&
     IS_SET(pRoomIndex->room_flags, ROOM_HEROES_ONLY) )
     return FALSE;
   */
  if (IS_SET(ch->act2,ACT_PUPPET))
    return TRUE;
  if (!IS_IMMORTAL (ch)
      && IS_SET (pRoomIndex->room_flags, ROOM_NEWBIES_ONLY)
      && (ch->level > 8))
    return FALSE;
  if (!IS_IMMORTAL (ch) && (pRoomIndex->clan != CLAN_BOGUS)
      && (ch->clan != pRoomIndex->clan) && !IS_SET (ch->act, ACT_PET))
    return FALSE;
  return TRUE;
}


/*
 * True if char can see victim.
 */
bool can_see (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (ch == NULL || victim == NULL)
    return FALSE;
  if (ch == victim)
    return TRUE;
  if (ch->in_room == NULL || victim->in_room == NULL)
    return FALSE;
  if (get_trust (ch) < victim->invis_level)
    return FALSE;
  if (get_trust (ch) < victim->incog_level && ch->in_room != victim->in_room)
    return FALSE;
  if ((!IS_NPC (ch) && IS_SET (ch->act, PLR_HOLYLIGHT))
      || (IS_NPC (ch) && IS_IMMORTAL (ch)))
    return TRUE;
  if ((ch->Class == PC_CLASS_DRUID) && check_nature_sense(ch))
    return TRUE;
  if (IS_SET(victim->in_room->room_flags2,ROOM_ALWAYS_DARK)
      && !(IS_AFFECTED(ch,AFF_INFRARED) && !IS_SET(victim->act,ACT_UNDEAD)))
  return FALSE;
  if (IS_AFFECTED (ch, AFF_BLIND))
    return FALSE;
  if (room_is_dark (ch->in_room) && ch->race != PC_RACE_KALIAN)
    {
      if (ch->in_room->sector_type != SECT_INSIDE)
	{
	  if (!(IS_AFFECTED (ch, AFF_DARK_VISION)
		|| (IS_AFFECTED (ch, AFF_INFRARED) && !IS_SET(victim->act,ACT_UNDEAD))))
	    return FALSE;
	}

      else if (!(IS_AFFECTED (ch, AFF_INFRARED) && !IS_SET(victim->act,ACT_UNDEAD)))
	return FALSE;
    }
  if (IS_AFFECTED (victim, AFF_INVISIBLE)
      && !IS_AFFECTED (ch, AFF_DETECT_INVIS))
    return FALSE;
  if (IS_SET(victim->act2,ACT_IMAGINARY) && !IS_IMMORTAL(ch) && ch->Class != PC_CLASS_CHAOS_JESTER)
    return FALSE;

  //Iblis - Sneaking is no longer autohide, after Iverath's insistance on this it was finally done
  /* sneaking */
/*  if (IS_AFFECTED (victim, AFF_SNEAK)
      && !IS_AFFECTED (ch, AFF_DETECT_HIDDEN) && victim->fighting == NULL)
    {
      int chance;
      chance = get_skill (victim, gsn_sneak);
      chance += get_curr_stat (ch, STAT_DEX) * 3 / 2;
      chance -= get_curr_stat (ch, STAT_INT) * 2;
      chance += ch->level - victim->level * 3 / 2;
      if (number_percent () < chance)
	return FALSE;
    }*/
//  if ((victim->mount && victim->mount->pIndexData && victim->mount->pIndexData->vnum == MOB_VNUM_BEAST)
//                         || (victim->pIndexData && victim->pIndexData->vnum == MOB_VNUM_BEAST))
//                  return FALSE;
  if (victim->stalking == ch && !IS_AFFECTED (ch, AFF_DETECT_HIDDEN)
      && victim->fighting == NULL)
    {
      int chance;
      chance = get_skill (victim, gsn_stalk);
      chance += get_curr_stat (ch, STAT_DEX) * 3 / 2;
      chance -= get_curr_stat (ch, STAT_INT) * 2;
      chance += ch->level - victim->level * 3 / 2;
      if (number_percent () < chance)
	return FALSE;
    }
  if (IS_AFFECTED (victim, AFF_CAMOUFLAGE))
    return FALSE;
  if (IS_AFFECTED (victim, AFF_HIDE)
      && !IS_AFFECTED (ch, AFF_DETECT_HIDDEN) && victim->fighting == NULL)
    return FALSE;
  if (victim->stalking == ch && !IS_AFFECTED (ch, AFF_DETECT_HIDDEN)
      && victim->fighting == NULL)
    return FALSE;
  if (victim->race == PC_RACE_NIDAE && IS_OUTSIDE(victim) 
		  && (weather_info.sky == SKY_RAINING || weather_info.sky == SKY_LIGHTNING)
		  && number_percent() < 20)
    return FALSE;
  if (ch->race == PC_RACE_SWARM && IS_SET(victim->act,ACT_UNDEAD))
    return FALSE;
  return TRUE;
}

bool can_see_hack (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (ch == victim)
    return TRUE;
  if (ch->in_room == NULL || victim->in_room == NULL)
    return FALSE;
  if (get_trust (ch) < victim->invis_level)
    return FALSE;
  if (get_trust (ch) < victim->incog_level && ch->in_room != victim->in_room)
    return FALSE;
  if ((!IS_NPC (ch) && IS_SET (ch->act, PLR_HOLYLIGHT))
      || (IS_NPC (ch) && IS_IMMORTAL (ch)))
    return TRUE;
  if (IS_AFFECTED (ch, AFF_BLIND))
    return FALSE;
  if (IS_AFFECTED (victim, AFF_INVISIBLE)
      && !IS_AFFECTED (ch, AFF_DETECT_INVIS))
    return FALSE;
  
   //Iblis - Sneaking is no longer autohide, after Iverath's insistance on this it was finally done
  /* sneaking */
/*  if (IS_AFFECTED (victim, AFF_SNEAK)
      && !IS_AFFECTED (ch, AFF_DETECT_HIDDEN) && victim->fighting == NULL)
    {
      int chance;
      chance = get_skill (victim, gsn_sneak);
      chance += get_curr_stat (ch, STAT_DEX) * 3 / 2;
      chance -= get_curr_stat (ch, STAT_INT) * 2;
      chance += ch->level - victim->level * 3 / 2;
      if (number_percent () < chance)
	return FALSE;
    }*/
//  if ((victim->mount && victim->mount->pIndexData && victim->mount->pIndexData->vnum == MOB_VNUM_BEAST)
//       || (victim->pIndexData && victim->pIndexData->vnum == MOB_VNUM_BEAST))
//        return FALSE;
  if (victim->stalking == ch && !IS_AFFECTED (ch, AFF_DETECT_HIDDEN)
      && victim->fighting == NULL)
    {
      int chance;
      chance = get_skill (victim, gsn_stalk);
      chance += get_curr_stat (ch, STAT_DEX) * 3 / 2;
      chance -= get_curr_stat (ch, STAT_INT) * 2;
      chance += ch->level - victim->level * 3 / 2;
      if (number_percent () < chance)
	return FALSE;
    }
  if (IS_AFFECTED (victim, AFF_CAMOUFLAGE))
    return FALSE;
  if (IS_AFFECTED (victim, AFF_HIDE)
      && !IS_AFFECTED (ch, AFF_DETECT_HIDDEN) && victim->fighting == NULL)
    return FALSE;
  if (victim->stalking == ch && !IS_AFFECTED (ch, AFF_DETECT_HIDDEN)
      && victim->fighting == NULL)
    return FALSE;
  return TRUE;
}


/*
 * True if char can see obj.
 */
bool can_see_obj (CHAR_DATA * ch, OBJ_DATA * obj)
{
  if (!IS_NPC (ch) && IS_SET (ch->act, PLR_HOLYLIGHT))
    return TRUE;
  if ((!IS_IMMORTAL (ch) || (IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_FAKIE))&& IS_SET (obj->extra_flags[1],ITEM_WIZI))
    return FALSE;
  if ((IS_IMMORTAL (ch) || ch->Class == PC_CLASS_CHAOS_JESTER || (IS_NPC(ch) && ch->pIndexData->pShop)) && IS_SET (obj->extra_flags[1],ITEM_IMAGINARY))
    return TRUE;
  if (!IS_IMMORTAL(ch) && ch->Class != PC_CLASS_CHAOS_JESTER && IS_SET (obj->extra_flags[1],ITEM_IMAGINARY))
    return FALSE;
  if (IS_SET (obj->extra_flags[0], ITEM_VIS_DEATH))
    return FALSE;
  if (IS_SET (obj->extra_flags[1], ITEM_PULSATING))
    return TRUE;
  if (IS_AFFECTED (ch, AFF_BLIND) && obj->item_type != ITEM_POTION)
    return FALSE;
  if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
    return TRUE;
  if (IS_SET (obj->extra_flags[0], ITEM_INVIS)
      && !IS_AFFECTED (ch, AFF_DETECT_INVIS))
    return FALSE;
  if (IS_OBJ_STAT (obj, ITEM_GLOW))
    return TRUE;
  if (room_is_dark (ch->in_room) && ch->race != PC_RACE_KALIAN)
    {
      if (ch->in_room->sector_type != SECT_INSIDE)
	{
	  if (!IS_AFFECTED (ch, AFF_INFRARED)
	      && !IS_AFFECTED (ch, AFF_DARK_VISION))
	    return FALSE;
	}

      else if (!IS_AFFECTED (ch, AFF_INFRARED))
	return FALSE;
    }
  return TRUE;
}


/*
 * True if char can drop obj.
 */
bool can_drop_obj (CHAR_DATA * ch, OBJ_DATA * obj)
{
  if (ch->dueler)
    return FALSE;
  if (!IS_SET (obj->extra_flags[0], ITEM_NODROP))
    return TRUE;
  if (!IS_NPC (ch) && ch->level >= LEVEL_IMMORTAL)
    return TRUE;
  if (IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_FAKIE)
    return TRUE;
  return FALSE;
}


/*
 * Return ascii name of an item type.
 */
char *item_type_name (OBJ_DATA * obj)
{
  switch (obj->item_type)
    {
    case ITEM_LIGHT:
      return "light";
    case ITEM_SCROLL:
      return "scroll";
    case ITEM_WAND:
      return "wand";
    case ITEM_STAFF:
      return "staff";
    case ITEM_WEAPON:
      return "weapon";
    case ITEM_SADDLE:
      return "saddle";
    case ITEM_VIAL:
      return "vial";
    case ITEM_TREASURE:
      return "treasure";
    case ITEM_ARMOR:
      return "armor";
    case ITEM_POTION:
      return "potion";
    case ITEM_CLOTHING:
      return "clothing";
    case ITEM_FURNITURE:
      return "furniture";
    case ITEM_TRASH:
      return "trash";
    case ITEM_PARCHMENT:
      return "parchment";
    case ITEM_CONTAINER:
      return "container";
    case ITEM_CLAN_DONATION:
      return "clan donation box";
    case ITEM_NEWCLANS_DBOX:	/* new_clans */
      return "clan donation box";	/* new_clans */
    case ITEM_PLAYER_DONATION:
      return "player box";
    case ITEM_ELEVATOR:
      return "transporter";
    case ITEM_DRINK_CON:
      return "drink container";
    case ITEM_KEY:
      return "key";
    case ITEM_FOOD:
      return "food";
    case ITEM_MONEY:
      return "money";
    case ITEM_EBUTTON:
      return "button";
    case ITEM_RAFT:
      return "raft";
    case ITEM_CORPSE_NPC:
      return "npc corpse";
    case ITEM_CORPSE_PC:
      return "pc corpse";
    case ITEM_FOUNTAIN:
      return "fountain";
    case ITEM_PILL:
      return "pill";
    case ITEM_PROTECT:
      return "protection";
    case ITEM_MAP:
      return "map";
    case ITEM_PORTAL:
      return "portal";
    case ITEM_PORTAL_BOOK:
      return "portal book";
    case ITEM_WARP_STONE:
      return "warp stone";
    case ITEM_ROOM_KEY:
      return "room key";
    case ITEM_GEM:
      return "gem";
    case ITEM_JEWELRY:
      return "jewelry";
    case ITEM_JUKEBOX:
      return "juke box";
    case ITEM_GILLS:
      return "gills";
    case ITEM_PACK:
      return "pack";
    case ITEM_STRANSPORT:
      return "transportation";
    case ITEM_CTRANSPORT:
      return "transportation";
    case ITEM_INSTRUMENT:
      return "instrument";
    case ITEM_WRITING_INSTRUMENT:
      return "writing instrument";
    case ITEM_WRITING_PAPER:
      return "writing paper";
    case ITEM_CTRANSPORT_KEY:
      return "transportation key";
    case ITEM_QUIVER:
      return "quiver";
    case ITEM_FEATHER:
      return "feather";
    case ITEM_BAIT:
      return "bait";
    case ITEM_POLE:
      return "pole";
    case ITEM_TREE:
      return "tree";
    case ITEM_WOOD:
      return "wood";
    case ITEM_SEED:
      return "seed";
    case ITEM_WOODEN_INCONSTRUCTION:
      return "wooden_in_construction";
    case ITEM_CARD:
      return "card";
    case ITEM_BINDER:
      return "binder";
    case ITEM_ROOM_TRAP:
      return "roomtrap";
    case ITEM_PORTAL_TRAP:
      return "portaltrap";
    case ITEM_OBJ_TRAP:
      return "objecttrap";
    case ITEM_RANDOM:
      return "random";
    }
  bug ("Item_type_name: unknown type %d.", obj->item_type);
  return "(unknown)";
}


/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name (int location)
{
  switch (location)
    {
    case APPLY_NONE:
      return "none";
    case APPLY_STR:
      return "strength";
    case APPLY_DEX:
      return "dexterity";
    case APPLY_INT:
      return "intelligence";
    case APPLY_WIS:
      return "wisdom";
    case APPLY_CON:
      return "constitution";
    case APPLY_CHA:
      return "charisma";
    case APPLY_SEX:
      return "sex";
    case APPLY_CLASS:
      return "Class";
    case APPLY_LEVEL:
      return "level";
    case APPLY_AGE:
      return "age";
    case APPLY_MANA:
      return "mana";
    case APPLY_HIT:
      return "hp";
    case APPLY_MOVE:
      return "moves";
    case APPLY_GOLD:
      return "gold";
    case APPLY_EXP:
      return "experience";
    case APPLY_AC:
      return "armor Class";
    case APPLY_HITROLL:
      return "hit roll";
    case APPLY_DAMROLL:
      return "damage roll";
    case APPLY_SAVES:
      return "saves";
    case APPLY_SAVING_ROD:
      return "save vs rod";
    case APPLY_SAVING_PETRI:
      return "save vs petrification";
    case APPLY_SAVING_BREATH:
      return "save vs breath";
    case APPLY_SAVING_SPELL:
      return "save vs spell";
    //case APPLY_SKILL:
    //  return "skill"
    case APPLY_SPELL_AFFECT:
      return "none";
    }
  bug ("Affect_location_name: unknown location %d.", location);
  return "(unknown)";
}


/*
 * Return ascii name of an affect bit vector.
 */
const char *affect_bit_name (int vector)
{
  static char buf[512];
  buf[0] = '\0';
  if (vector & AFF_BLIND)
    strcat (buf, " blind");
  if (vector & AFF_INVISIBLE)
    strcat (buf, " invisible");
  if (vector & AFF_DETECT_EVIL)
    strcat (buf, " detect_evil");
  if (vector & AFF_DETECT_GOOD)
    strcat (buf, " detect_good");
  if (vector & AFF_DETECT_INVIS)
    strcat (buf, " detect_invis");
  if (vector & AFF_DETECT_MAGIC)
    strcat (buf, " detect_magic");
  if (vector & AFF_DETECT_HIDDEN)
    strcat (buf, " detect_hidden");
  if (vector & AFF_SANCTUARY)
    strcat (buf, " sanctuary");
  if (vector & AFF_FAERIE_FIRE)
    strcat (buf, " faerie_fire");
  if (vector & AFF_INFRARED)
    strcat (buf, " infrared");
  if (vector & AFF_CURSE)
    strcat (buf, " curse");
  if (vector & AFF_POISON)
    strcat (buf, " poison");
  if (vector & AFF_PROTECT_EVIL)
    strcat (buf, " prot_evil");
  if (vector & AFF_PROTECT_GOOD)
    strcat (buf, " prot_good");
  if (vector & AFF_SLEEP)
    strcat (buf, " sleep");
  if (vector & AFF_SNEAK)
    strcat (buf, " sneak");
  if (vector & AFF_HIDE)
    strcat (buf, " hide");
  if (vector & AFF_CAMOUFLAGE)
    strcat (buf, " camouflage");
  if (vector & AFF_CHARM)
    strcat (buf, " charm");
  if (vector & AFF_FLYING)
    strcat (buf, " flying");
  if (vector & AFF_PASS_DOOR)
    strcat (buf, " pass_door");
  if (vector & AFF_BERSERK)
    strcat (buf, " berserk");
  if (vector & AFF_CALM)
    strcat (buf, " calm");
  if (vector & AFF_HASTE)
    strcat (buf, " haste");
  if (vector & AFF_SLOW)
    strcat (buf, " slow");
  if (vector & AFF_PLAGUE)
    strcat (buf, " plague");
  if (vector & AFF_DARK_VISION)
    strcat (buf, " dark_vision");
  if (vector & AFF_AQUA_BREATHE)
    strcat (buf, " aqua_breathe");
  if (vector & AFF_SWIM)
    strcat (buf, " swim");
  return (buf[0] != '\0') ? buf + 1 : "none";
}


/*
 * Return ascii name of extra flags vector.
 */

const char *extra_bit_name (long extra_flags[])
{
  return extra_bit_name_new (extra_flags,TRUE);
}

const char *extra_bit_name_new (long extra_flags[], bool show_hidden)
{
  static char buf[1024];  //doubled this do deal with more extra flags
  int i;
  for (i = 0; i < MAX_EXTRA_FLAGS; i++)
    {
      if (i == 0)
	{      
	  buf[0] = '\0';
	  if (extra_flags[i] & ITEM_GLOW)
	    strcat (buf, " glow");
	  if (extra_flags[i] & ITEM_HUM)
	    strcat (buf, " hum");
	  if (extra_flags[i] & ITEM_DARK)
	    strcat (buf, " dark");
	  if (extra_flags[i] & ITEM_LOCK)
	    strcat (buf, " lock");
	  if (extra_flags[i] & ITEM_EVIL)
	    strcat (buf, " evil");
	  if (extra_flags[i] & ITEM_INVIS)
	    strcat (buf, " invis");
	  if (extra_flags[i] & ITEM_MAGIC)
	    strcat (buf, " magic");
	  if (extra_flags[i] & ITEM_NODROP)
	    strcat (buf, " nodrop");
	  if (extra_flags[i] & ITEM_BLESS)
	    strcat (buf, " bless");
	  if (extra_flags[i] & ITEM_ANTI_GOOD)
	    strcat (buf, " anti-good");
	  if (extra_flags[i] & ITEM_ANTI_EVIL)
	    strcat (buf, " anti-evil");
	  if (extra_flags[i] & ITEM_ANTI_NEUTRAL)
	    strcat (buf, " anti-neutral");
	  if (extra_flags[i] & ITEM_NOREMOVE)
	    strcat (buf, " noremove");
	  if (extra_flags[i] & ITEM_INVENTORY)
	    strcat (buf, " inventory");
	  if (extra_flags[i] & ITEM_NOPURGE)
	    strcat (buf, " nopurge");
	  if (extra_flags[i] & ITEM_VIS_DEATH)
	    strcat (buf, " vis_death");
	  if (extra_flags[i] & ITEM_ROT_DEATH)
	    strcat (buf, " rot_death");
	  if (extra_flags[i] & ITEM_NOLOCATE)
	    strcat (buf, " no_locate");
	  if (extra_flags[i] & ITEM_NOSAC)
	    strcat (buf, " no_sac");
	  if (extra_flags[i] & ITEM_NONMETAL)
	    strcat (buf, " nonmetal");
	  if (extra_flags[i] & ITEM_MELT_DROP)
	    strcat (buf, " meltdrop");
	  if (extra_flags[i] & ITEM_HAD_TIMER)
	    strcat (buf, " had_timer");
	  if (extra_flags[i] & ITEM_DONATE)
	    strcat (buf, " donate");
	  if (extra_flags[i] & ITEM_HAD_TIMER)
	    strcat (buf, " had_timer");
	  if (extra_flags[i] & ITEM_PERMSTOCK)
	    strcat (buf, " permstock");
	  if (extra_flags[i] & ITEM_SELL_EXTRACT)
	    strcat (buf, " sell_extract");
	  if (extra_flags[i] & ITEM_BURN_PROOF)
	    strcat (buf, " burn_proof");
	  if (extra_flags[i] & ITEM_NOUNCURSE)
	    strcat (buf, " no_uncurse");
	  if (extra_flags[i] & ITEM_NOIDENTIFY)
	    strcat (buf, " no_identify");
	  if (extra_flags[i] & ITEM_NOSAVE)
	    strcat (buf, " no_save");
	  if (extra_flags[i] & ITEM_NOMOB)
	    strcat (buf, " no_mob");
	  if (extra_flags[i] & ITEM_NOENCHANT) 
	    strcat (buf, " no_enchant");
	}
      else if (i == 1)
	{
	  if (extra_flags[i] & ITEM_NOSTOCK)
            strcat (buf, " no_stock");
	  if (extra_flags[i] & ITEM_NODONATE)
            strcat (buf, " no_donate");
	  if (extra_flags[i] & ITEM_NOAUCTION)
            strcat (buf, " no_auction");
	  if (extra_flags[i] & ITEM_TATTOO)
 	    strcat (buf, " tattoo");
	  if (extra_flags[i] & ITEM_WIZI)
 	    strcat (buf, " wizi");
	  if (extra_flags[i] & ITEM_IMAGINARY)
	    strcat (buf, " imaginary");
	  if (extra_flags[i] & ITEM_WINGED)
   	    strcat (buf, " winged");
	  if (extra_flags[i] & ITEM_NOAVATAR)
  	    strcat (buf, " noavatar");
	  if (extra_flags[i] & ITEM_NOKALIAN)
  	    strcat (buf, " nokalian");
	  if (extra_flags[i] & ITEM_LIGHTNING_PROOF)
  	    strcat (buf, " lightning_proof");
	  if (extra_flags[i] & ITEM_COLD_PROOF)
            strcat (buf, " cold_proof");
	  if (extra_flags[i] & ITEM_ACID_PROOF)
            strcat (buf, " acid_proof");
	  if (extra_flags[i] & ITEM_INVIS_WEAR && show_hidden)
  	    strcat (buf, " invis_wear");
	  if (extra_flags[i] & ITEM_PULSATING)
 	    strcat (buf, " pulsating");
	}
    }
  return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *extra_bit_name_old (int extra_flags)
{
  static char buf[512];
  buf[0] = '\0';
  if (extra_flags & ITEM_GLOW)
    strcat (buf, " glow");
  if (extra_flags & ITEM_HUM)
    strcat (buf, " hum");
  if (extra_flags & ITEM_DARK)
    strcat (buf, " dark");
  if (extra_flags & ITEM_LOCK)
    strcat (buf, " lock");
  if (extra_flags & ITEM_EVIL)
    strcat (buf, " evil");
  if (extra_flags & ITEM_INVIS)
    strcat (buf, " invis");
  if (extra_flags & ITEM_MAGIC)
    strcat (buf, " magic");
  if (extra_flags & ITEM_NODROP)
    strcat (buf, " nodrop");
  if (extra_flags & ITEM_BLESS)
    strcat (buf, " bless");
  if (extra_flags & ITEM_ANTI_GOOD)
    strcat (buf, " anti-good");
  if (extra_flags & ITEM_ANTI_EVIL)
    strcat (buf, " anti-evil");
  if (extra_flags & ITEM_ANTI_NEUTRAL)
    strcat (buf, " anti-neutral");
  if (extra_flags & ITEM_NOREMOVE)
    strcat (buf, " noremove");
  if (extra_flags & ITEM_INVENTORY)
    strcat (buf, " inventory");
  if (extra_flags & ITEM_NOPURGE)
    strcat (buf, " nopurge");
  if (extra_flags & ITEM_VIS_DEATH)
    strcat (buf, " vis_death");
  if (extra_flags & ITEM_ROT_DEATH)
    strcat (buf, " rot_death");
  if (extra_flags & ITEM_NOLOCATE)
    strcat (buf, " no_locate");
  if (extra_flags & ITEM_NOSAC)
    strcat (buf, " no_sac");
  if (extra_flags & ITEM_NONMETAL)
    strcat (buf, " nonmetal");
  if (extra_flags & ITEM_MELT_DROP)
    strcat (buf, " meltdrop");
  if (extra_flags & ITEM_HAD_TIMER)
    strcat (buf, " had_timer");
  if (extra_flags & ITEM_DONATE)
    strcat (buf, " donate");
  if (extra_flags & ITEM_HAD_TIMER)
    strcat (buf, " had_timer");
  if (extra_flags & ITEM_PERMSTOCK)
    strcat (buf, " permstock");
  if (extra_flags & ITEM_SELL_EXTRACT)
    strcat (buf, " sell_extract");
  if (extra_flags & ITEM_BURN_PROOF)
    strcat (buf, " burn_proof");
  if (extra_flags & ITEM_NOUNCURSE)
    strcat (buf, " no_uncurse");
  if (extra_flags & ITEM_NOIDENTIFY)
    strcat (buf, " no_identify");
  if (extra_flags & ITEM_NOSAVE)
    strcat (buf, " no_save");
  if (extra_flags & ITEM_NOMOB)
    strcat (buf, " no_mob");
  if (extra_flags & ITEM_NOENCHANT)
    strcat (buf, " no_enchant");
  return (buf[0] != '\0') ? buf + 1 : "none";
}



/* return ascii name of an act vector */
const char *act_bit_name (int act_flags)
{
  static char buf[512];
  buf[0] = '\0';
  if (IS_SET (act_flags, ACT_IS_NPC))
    {
      strcat (buf, " npc");
      if (act_flags & ACT_SENTINEL)
	strcat (buf, " sentinel");
      if (act_flags & ACT_SCAVENGER)
	strcat (buf, " scavenger");
      if (act_flags & ACT_AGGRESSIVE)
	strcat (buf, " aggressive");
      if (act_flags & ACT_STAY_AREA)
	strcat (buf, " stay_area");
      if (act_flags & ACT_WIMPY)
	strcat (buf, " wimpy");
      if (act_flags & ACT_PET)
	strcat (buf, " pet");
      if (act_flags & ACT_TRAIN)
	strcat (buf, " train");
      if (act_flags & ACT_PRACTICE)
	strcat (buf, " practice");
      if (act_flags & ACT_UNDEAD)
	strcat (buf, " undead");
      if (act_flags & ACT_CLERIC)
	strcat (buf, " cleric");
      if (act_flags & ACT_MAGE)
	strcat (buf, " mage");
      if (act_flags & ACT_THIEF)
	strcat (buf, " thief");
      if (act_flags & ACT_WARRIOR)
	strcat (buf, " warrior");
      if (act_flags & ACT_RANGER)
	strcat (buf, " ranger");
      if (act_flags & ACT_BARD)
	strcat (buf, " bard");
      if (act_flags & ACT_HALT)
	strcat (buf, " halt");
      if (act_flags & ACT_BOUNTY)
	strcat (buf, " bounty");
      if (act_flags & ACT_NOPURGE)
	strcat (buf, " no_purge");
      if (act_flags & ACT_IS_HEALER)
	strcat (buf, " healer");
      if (act_flags & ACT_NO_SCENT)
	strcat (buf, " no_scent");
      if (act_flags & ACT_IS_CHANGER)
	strcat (buf, " changer");
      if (act_flags & ACT_GAIN)
	strcat (buf, " skill_train");
      if (act_flags & ACT_LOOTER)
	strcat (buf, " looter");
      if (act_flags & ACT_TAME)
	strcat (buf, " tame");
    }

  else
    {
      strcat (buf, " player");

/*      if (act_flags & PLR_AUTOASSIST)
	strcat (buf, " autoassist");*/
      if (act_flags & PLR_AUTOEXIT)
	strcat (buf, " autoexit");
      if (act_flags & PLR_AUTOLOOT)
	strcat (buf, " autoloot");
      if (act_flags & PLR_AUTOSAC)
	strcat (buf, " autosac");
      if (act_flags & PLR_AUTOGOLD)
	strcat (buf, " autogold");
      if (act_flags & PLR_AUTOSPLIT)
	strcat (buf, " autosplit");
      if (act_flags & PLR_HOLYLIGHT)
	strcat (buf, " holy_light");
      if (act_flags & PLR_CANLOOT)
	strcat (buf, " loot_corpse");
      if (act_flags & PLR_NOSUMMON)
	strcat (buf, " no_summon");
      if (act_flags & PLR_NOFOLLOW)
	strcat (buf, " no_follow");
      if (act_flags & PLR_FREEZE)
	strcat (buf, " frozen");
      if (act_flags & PLR_WANTED)
	strcat (buf, " wanted");
      if (act_flags & PLR_JAILED)
	strcat (buf, " jailed");
      if (act_flags & PLR_AUTOBUTCHER)
	strcat (buf, " autobutcher");
      if (act_flags & PLR_UNDEAD)
        strcat (buf, " undead");
      if (act_flags & PLR_CARDKILL)
        strcat (buf, " cardkill");
      if (act_flags & PLR_SWITCHOK)
        strcat (buf, " switchok");
    }
  return (buf[0] != '\0') ? buf + 1 : "none";
}


// Adeon 7/1/03 -- Expanding Mob Act Flags!
const char *act2_bit_name (int act_flags2)
{
  static char buf[512];
  buf[0] = '\0';
  if (act_flags2 & ACT_BLOCK_EXIT)
    strcat (buf, " blockexit");
  if (act_flags2 & ACT_NOSINK)
    strcat (buf, " nosink");
  if (act_flags2 & ACT_ILLUSION)
    strcat (buf, " illusion");
  if (act_flags2 & ACT_NOMOVE)
    strcat (buf, " nomove");
  if (act_flags2 & ACT_FAMILIAR)
    strcat (buf, " familiar");
  if (act_flags2 & ACT_PUPPET)
    strcat (buf, " puppet");
  if (act_flags2 & ACT_PUPPETEER)
    strcat (buf, " puppeteer");
  if (act_flags2 & ACT_MONK)
    strcat (buf, " monk");
  if (act_flags2 & ACT_NECROMANCER)
    strcat (buf, " necromancer");  
  if (act_flags2 & ACT_QUESTMASTER)
    strcat (buf, " questmaster");
  if (act_flags2 & ACT_QUESTMOB)
    strcat (buf, " questmob");
  if (act_flags2 & ACT_AGGIE_EVIL)
    strcat (buf, " aggie_evil");
  if (act_flags2 & ACT_AGGIE_NEUTRAL)
    strcat (buf, " aggie_neutral");
  if (act_flags2 & ACT_AGGIE_GOOD)
    strcat (buf, " aggie_good");
  if (act_flags2 & ACT_CROUPIER)
    strcat (buf, " croupier");
  if (act_flags2 & ACT_MAGE_SKILLS)
    strcat (buf, " mageskills");
  if (act_flags2 & ACT_NO_KILL)
    strcat (buf, " nokill");
  if (act_flags2 & ACT_NO_ALIGN)
    strcat (buf, " noalign");
  if (act_flags2 & ACT_TRAPMOB)
    strcat (buf, " trapmob");
  if (act_flags2 & ACT_NO_BLOCK)
    strcat (buf, " noblock");
  if (act_flags2 & ACT_STAY_SECTOR)
    strcat (buf, " staysector");
  if (act_flags2 & ACT_WARPED)
    strcat (buf, " warped");
  if (act_flags2 & ACT_IMAGINARY)
    strcat (buf, " imaginary");
  if (act_flags2 & ACT_SWITCHED)
    strcat (buf, " switched");
  if (act_flags2 & ACT_ALL_SKILLS)
    strcat (buf, " all_skills");
  if (act_flags2 & ACT_ALWAYS_MIRRORED)
    strcat (buf, " always_mirrored");
  
  return (buf[0] != '\0') ? buf + 1 : " ";
}

// Adeon 7/20/03 -- Obj Trigger Bit Names
const char * obj_trig_bit_name (int obj_trig_flags)
{
  static char buf[512];

  buf[0] = '\0';
  if(obj_trig_flags & OBJ_TRIG_ENTER_ROOM)
    strcat(buf, " entersroom");
  if(obj_trig_flags & OBJ_TRIG_GET)
    strcat(buf, " getsobj");
  if(obj_trig_flags & OBJ_TRIG_WEAR)
    strcat(buf, " wearsobj");
  if(obj_trig_flags & OBJ_TRIG_CHAR_HP_PCT)
    strcat(buf, " char_hp_pct");
  if(obj_trig_flags & OBJ_TRIG_VICT_HP_PCT)
    strcat(buf, " vict_hp_pct");
  if(obj_trig_flags & OBJ_TRIG_MANA_PCT)
    strcat(buf, " mana_pct");
  if(obj_trig_flags & OBJ_TRIG_MOVE_PCT)
    strcat(buf, " move_pct");
  if(obj_trig_flags & OBJ_TRIG_CHANCE_ON_HIT)
    strcat(buf, " chance_on_hit");
  if(obj_trig_flags & OBJ_TRIG_CHANCE_ON_ROUND)
    strcat(buf, " chance_on_round");
  if(obj_trig_flags & OBJ_TRIG_REMOVE)
    strcat(buf, " whenremoved");
  if(obj_trig_flags & OBJ_TRIG_DROP)
    strcat(buf, " whendropped");
  if(obj_trig_flags & OBJ_TRIG_PUT)
    strcat(buf, " whenput");
  if(obj_trig_flags & OBJ_TRIG_OBJ_GIVEN_CHAR)
    strcat(buf, " obj_given_char");
  if(obj_trig_flags & OBJ_TRIG_OPEN)
    strcat(buf, " whenopened");
  if(obj_trig_flags & OBJ_TRIG_CLOSE)
    strcat(buf, " whenclosed");
  if(obj_trig_flags & OBJ_TRIG_ACTION_WORD)
    strcat(buf, " whenactivated");
  if(obj_trig_flags & OBJ_TRIG_CHAR_TO_ROOM)
    strcat(buf, " char_enters_room");
  if(obj_trig_flags & OBJ_TRIG_CHAR_VOCALIZE)
    strcat(buf, " char_vocalizes");
  if(obj_trig_flags & OBJ_TRIG_CHAR_ATTACKS)
    strcat(buf, " char_attacks");
  if(obj_trig_flags & OBJ_TRIG_CHAR_IS_ATTACKED)
    strcat(buf, " char_is_attacked");
  if(obj_trig_flags & OBJ_TRIG_QUAFFED)
    strcat(buf, " whenquaffed");
  if(obj_trig_flags & OBJ_TRIG_EATEN)
    strcat(buf, " wheneaten");
  if(obj_trig_flags & OBJ_TRIG_CHAR_FLEES)
    strcat(buf, " char_flees");
  if(obj_trig_flags & OBJ_TRIG_VICT_FLEES)
    strcat(buf, " vict_flees");
  if(obj_trig_flags & OBJ_TRIG_CHAR_POS_CHANGE)
    strcat(buf, " char_pos_change");
  if(obj_trig_flags & OBJ_TRIG_CHAR_RECALLS)
    strcat(buf, " char_recalls");
  if(obj_trig_flags & OBJ_TRIG_CHAR_USES_SKILL)
    strcat(buf, " char_uses_skill");
  if(obj_trig_flags & OBJ_TRIG_CHAR_USES_SPELL)
    strcat(buf, " char_uses_spell");
  if(obj_trig_flags & OBJ_TRIG_CHAR_INVOKES)
    strcat(buf, " char_invokes");
  if(obj_trig_flags & OBJ_TRIG_ON_TICK)
    strcat(buf, " on_tick");
  if(obj_trig_flags & OBJ_TRIG_EXTRA_FLAGS)
    strcat(buf, " extra");

  return (buf[0] != '\0') ? buf + 1 : " ";
}

const char *comm_bit_name (int comm_flags)
{
  static char buf[512];
  buf[0] = '\0';
  if (comm_flags & COMM_QUIET)
    strcat (buf, " quiet");
  if (comm_flags & COMM_DEAF)
    strcat (buf, " deaf");
  if (comm_flags & COMM_NOWIZ)
    strcat (buf, " no_wiz");
  if (comm_flags & COMM_NOAUCTION)
    strcat (buf, " no_auction");
  if (comm_flags & COMM_NOGOSSIP)
    strcat (buf, " no_gossip");
  if (comm_flags & COMM_NOQUESTION)
    strcat (buf, " no_question");
  if (comm_flags & COMM_NOMUSIC)
    strcat (buf, " no_music");
  if (comm_flags & COMM_NOOOC)
    strcat (buf, " no_ooc");
  if (comm_flags & COMM_COMPACT)
    strcat (buf, " compact");
  if (comm_flags & COMM_BRIEF)
    strcat (buf, " brief");
  if (comm_flags & COMM_PROMPT)
    strcat (buf, " prompt");
  if (comm_flags & COMM_COMBINE)
    strcat (buf, " combine");
  if (comm_flags & COMM_NOEMOTE)
    strcat (buf, " no_emote");
  if (comm_flags & COMM_NOSHOUT)
    strcat (buf, " no_shout");
  if (comm_flags & COMM_NOTELL)
    strcat (buf, " no_tell");
  if (comm_flags & COMM_NOCHANNELS)
    strcat (buf, " no_channels");
  return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *comm2_bit_name (int comm2_flags)
{
  static char buf[512];
  buf[0] = '\0';
  if (comm2_flags & COMM_NORANT)
    strcat (buf, " no_rant");
  if (comm2_flags & COMM_RANTONLY)
    strcat (buf, " rant_only");
  if (comm2_flags & COMM_NOPOLL)
    strcat (buf, " nopoll");
  if (comm2_flags & COMM_NOGEMOTE)
    strcat (buf, " nogemote");
  if (comm2_flags & COMM_IMM_NOGEMOTE)
    strcat (buf, " imm_nogemote");
  if (comm2_flags & COMM_NODELETE)
    strcat (buf, " nodelete");
  if (comm2_flags & COMM_SHOWDAMAGE)
    strcat (buf, " showdamage");
  if (comm2_flags & COMM_SHUTUP)
    strcat (buf, " shutup");  
   return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *imm_bit_name (int imm_flags)
{
  static char buf[512];
  buf[0] = '\0';
  if (imm_flags & IMM_SUMMON)
    strcat (buf, " summon");
  if (imm_flags & IMM_CHARM)
    strcat (buf, " charm");
  if (imm_flags & IMM_MAGIC)
    strcat (buf, " magic");
  if (imm_flags & IMM_WEAPON)
    strcat (buf, " weapon");
  if (imm_flags & IMM_BASH)
    strcat (buf, " blunt");
  if (imm_flags & IMM_PIERCE)
    strcat (buf, " piercing");
  if (imm_flags & IMM_SLASH)
    strcat (buf, " slashing");
  if (imm_flags & IMM_FIRE)
    strcat (buf, " fire");
  if (imm_flags & IMM_COLD)
    strcat (buf, " cold");
  if (imm_flags & IMM_LIGHTNING)
    strcat (buf, " lightning");
  if (imm_flags & IMM_ACID)
    strcat (buf, " acid");
  if (imm_flags & IMM_POISON)
    strcat (buf, " poison");
  if (imm_flags & IMM_NEGATIVE)
    strcat (buf, " negative");
  if (imm_flags & IMM_HOLY)
    strcat (buf, " holy");
  if (imm_flags & IMM_ENERGY)
    strcat (buf, " energy");
  if (imm_flags & IMM_MENTAL)
    strcat (buf, " mental");
  if (imm_flags & IMM_DISEASE)
    strcat (buf, " disease");
  if (imm_flags & IMM_DROWNING)
    strcat (buf, " drowning");
  if (imm_flags & IMM_LIGHT)
    strcat (buf, " light");
  if (imm_flags & VULN_IRON)
    strcat (buf, " iron");
  if (imm_flags & VULN_WOOD)
    strcat (buf, " wood");
  if (imm_flags & VULN_SILVER)
    strcat (buf, " silver");
  if (imm_flags & IMM_LEVELS)
    strcat (buf, " levels");
  if (imm_flags & IMM_BLIND)
    strcat (buf, " blind");
  if (imm_flags & IMM_SLEEP)
    strcat (buf, " sleep");
  if (imm_flags & IMM_ARROWS)
    strcat (buf, " arrows");
  if (imm_flags & IMM_ENTANGLE)
    strcat (buf, " entangle");
  if (imm_flags & IMM_WIND)
    strcat (buf, " wind");
  return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *wear_bit_name (long wear_flags)
{
  static char buf[512];
  buf[0] = '\0';
  if (wear_flags & ITEM_TAKE)
    strcat (buf, " take");
  if (wear_flags & ITEM_WEAR_FINGER)
    strcat (buf, " finger");
  if (wear_flags & ITEM_WEAR_NECK)
    strcat (buf, " neck");
  if (wear_flags & ITEM_WEAR_BODY)
    strcat (buf, " torso");
  if (wear_flags & ITEM_WEAR_HEAD)
    strcat (buf, " head");
  if (wear_flags & ITEM_WEAR_LEGS)
    strcat (buf, " legs");
  if (wear_flags & ITEM_WEAR_FEET)
    strcat (buf, " feet");
  if (wear_flags & ITEM_WEAR_HANDS)
    strcat (buf, " hands");
  if (wear_flags & ITEM_WEAR_ARMS)
    strcat (buf, " arms");
  if (wear_flags & ITEM_WEAR_SHIELD)
    strcat (buf, " shield");
  if (wear_flags & ITEM_WEAR_ABOUT)
    strcat (buf, " body");
  if (wear_flags & ITEM_WEAR_WAIST)
    strcat (buf, " waist");
  if (wear_flags & ITEM_WEAR_WRIST)
    strcat (buf, " wrist");
  if (wear_flags & ITEM_WIELD)
    strcat (buf, " wield");
  if (wear_flags & ITEM_HOLD)
    strcat (buf, " hold");
  if (wear_flags & ITEM_WEAR_FLOAT)
    strcat (buf, " float");
  if (wear_flags & ITEM_WEAR_ANOTHER_FINGER)
    strcat (buf, " another_finger");
  if (wear_flags & ITEM_WEAR_ANOTHER_NECK)
    strcat (buf, " another_neck");
  if (wear_flags & ITEM_WEAR_ANOTHER_NECK)
    strcat (buf, " another_wrist");	   
  if (wear_flags & ITEM_WEAR_LIGHT)
    strcat (buf, " light");
  if (wear_flags & ITEM_WEAR_TATTOO)
    strcat (buf, " tattoo");
  if (wear_flags & ITEM_WEAR_FACE)
    strcat (buf, " face");
  if (wear_flags & ITEM_WEAR_EARS)
    strcat (buf, " ears");
  if (wear_flags & ITEM_WEAR_CREST)
    strcat (buf, " crest");
  if (wear_flags & ITEM_WEAR_CLAN_MARK)
    strcat (buf, " clan_mark");
  if (wear_flags & ITEM_WORN_WINGS)
    strcat (buf, " wings");
  return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *room_bit_name (int room_flags)
{
  static char buf[512];
  buf[0] = '\0';
  if (room_flags & ROOM_DARK)
    strcat (buf, " dark");
  if (room_flags & ROOM_ELEVATOR)
    strcat (buf, " elevator");
  if (room_flags & ROOM_NOMOUNT)
    strcat (buf, " no_mount");
  if (room_flags & ROOM_SILENT)
    strcat (buf, " silent");
  if (room_flags & ROOM_NO_MOB)
    strcat (buf, " no_mob");
  if (room_flags & ROOM_TRANSPORT)
    strcat (buf, " transport");
  if (room_flags & ROOM_INDOORS)
    strcat (buf, " indoors");
  if (room_flags & ROOM_PRIVATE)
    strcat (buf, " private");
  if (room_flags & ROOM_STABLE)
    strcat (buf, " stable");
  if (room_flags & ROOM_SAFE)
    strcat (buf, " safe");
  if (room_flags & ROOM_SOLITARY)
    strcat (buf, " solitary");
  if (room_flags & ROOM_PET_SHOP)
    strcat (buf, " pet_shop");
  if (room_flags & ROOM_NO_RECALL)
    strcat (buf, " norecall");
  if (room_flags & ROOM_IMP_ONLY)
    strcat (buf, " imp_only");
  if (room_flags & ROOM_GODS_ONLY)
    strcat (buf, " gods_only");
  if (room_flags & ROOM_NOFLYMOUNT)
    strcat (buf, " noflymount");
  if (room_flags & ROOM_NOSWIMMOUNT)
    strcat (buf, " no_swim_mount");
  if (room_flags & ROOM_NEWBIES_ONLY)
    strcat (buf, " newbies_only");
  if (room_flags & ROOM_LAW)
    strcat (buf, " law");
  if (room_flags & ROOM_NOWHERE)
    strcat (buf, " nowhere");
  if (room_flags & ROOM_ARENA)
    strcat (buf, " arena");
  if (room_flags & ROOM_SPECTATOR)
    strcat (buf, " spectator");
  if (room_flags & ROOM_ARENA_REGISTRATION)
    strcat (buf, " arena_registration");
  if (room_flags & ROOM_NO_PUSH)
    strcat (buf, " nopush");
  if (room_flags & ROOM_BANK)
    strcat (buf, " bank");
  if (room_flags & ROOM_BUILD)
    strcat (buf, " build");
  if (room_flags & ROOM_BLOODBATH)
    strcat (buf, " bloodbath");
  if (room_flags & ROOM_NO_CHARMIE)
    strcat (buf, " no_charmie");
  if (room_flags & ROOM_NO_FLY)
    strcat (buf, " nofly");
  if (room_flags & ROOM_PIER)
    strcat (buf, " pier");
  if (room_flags & ROOM_SHRINE)
    strcat (buf, " shrine");
  return (buf[0] != '\0') ? buf + 1 : "none";
}


// Adeon 6/30/03 -- expanded room flags
const char *room2_bit_name (int room_flags2)
{
  static char buf[512];
  buf[0] = '\0';
  if (room_flags2 & ROOM_SINKING)
    strcat (buf, " sinking");
  if (room_flags2 & ROOM_NOVEHICLE)
    strcat (buf, " novehicle");
  if (room_flags2 & ROOM_ALWAYS_DARK)
    strcat (buf, " alwaysdark");
  if (room_flags2 & ROOM_NOMAGIC)
    strcat (buf, " nomagic");
  if (room_flags2 & ROOM_PKONLY)
    strcat (buf, " pkonly");
  if (room_flags2 & ROOM_WARPED) 
    strcat (buf, " warped");
  if (room_flags2 & ROOM_SOLO)
    strcat (buf, " solo");
  if (room_flags2 & ROOM_MIRROR)
    strcat (buf, " mirror");
  if (room_flags2 & ROOM_ICE)
    strcat (buf, " ice");
  if (room_flags2 & ROOM_SNOW)
    strcat (buf, " snow");
  return (buf[0] != '\0') ? buf + 1 : "";
}

const char *form_bit_name (int form_flags)
{
  static char buf[512];
  buf[0] = '\0';
  if (form_flags & FORM_POISON)
    strcat (buf, " poison");

  else if (form_flags & FORM_EDIBLE)
    strcat (buf, " edible");
  if (form_flags & FORM_MAGICAL)
    strcat (buf, " magical");
  if (form_flags & FORM_INSTANT_DECAY)
    strcat (buf, " instant_decay");
  if (form_flags & FORM_OTHER)
    strcat (buf, " other");
  if (form_flags & FORM_ANIMAL)
    strcat (buf, " animal");
  if (form_flags & FORM_SENTIENT)
    strcat (buf, " sentient");
  if (form_flags & FORM_UNDEAD)
    strcat (buf, " undead");
  if (form_flags & FORM_CONSTRUCT)
    strcat (buf, " construct");
  if (form_flags & FORM_MIST)
    strcat (buf, " mist");
  if (form_flags & FORM_INTANGIBLE)
    strcat (buf, " intangible");
  if (form_flags & FORM_BIPED)
    strcat (buf, " biped");
  if (form_flags & FORM_CENTAUR)
    strcat (buf, " centaur");
  if (form_flags & FORM_INSECT)
    strcat (buf, " insect");
  if (form_flags & FORM_SPIDER)
    strcat (buf, " spider");
  if (form_flags & FORM_CRUSTACEAN)
    strcat (buf, " crustacean");
  if (form_flags & FORM_WORM)
    strcat (buf, " worm");
  if (form_flags & FORM_BLOB)
    strcat (buf, " blob");
  if (form_flags & FORM_MAMMAL)
    strcat (buf, " mammal");
  if (form_flags & FORM_BIRD)
    strcat (buf, " bird");
  if (form_flags & FORM_REPTILE)
    strcat (buf, " reptile");
  if (form_flags & FORM_SNAKE)
    strcat (buf, " snake");
  if (form_flags & FORM_DRAGON)
    strcat (buf, " dragon");
  if (form_flags & FORM_AMPHIBIAN)
    strcat (buf, " amphibian");
  if (form_flags & FORM_FISH)
    strcat (buf, " fish");
  if (form_flags & FORM_COLD_BLOOD)
    strcat (buf, " cold_blooded");
  return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *part_bit_name (int part_flags)
{
  static char buf[512];
  buf[0] = '\0';
  if (part_flags & PART_HEAD)
    strcat (buf, " head");
  if (part_flags & PART_ARMS)
    strcat (buf, " arms");
  if (part_flags & PART_LEGS)
    strcat (buf, " legs");
  if (part_flags & PART_HEART)
    strcat (buf, " heart");
  if (part_flags & PART_BRAINS)
    strcat (buf, " brains");
  if (part_flags & PART_GUTS)
    strcat (buf, " guts");
  if (part_flags & PART_HANDS)
    strcat (buf, " hands");
  if (part_flags & PART_FEET)
    strcat (buf, " feet");
  if (part_flags & PART_FINGERS)
    strcat (buf, " fingers");
  if (part_flags & PART_EAR)
    strcat (buf, " ears");
  if (part_flags & PART_EYE)
    strcat (buf, " eyes");
  if (part_flags & PART_LONG_TONGUE)
    strcat (buf, " long_tongue");
  if (part_flags & PART_EYESTALKS)
    strcat (buf, " eyestalks");
  if (part_flags & PART_TENTACLES)
    strcat (buf, " tentacles");
  if (part_flags & PART_FINS)
    strcat (buf, " fins");
  if (part_flags & PART_WINGS)
    strcat (buf, " wings");
  if (part_flags & PART_TAIL)
    strcat (buf, " tail");
  if (part_flags & PART_CLAWS)
    strcat (buf, " claws");
  if (part_flags & PART_FANGS)
    strcat (buf, " fangs");
  if (part_flags & PART_HORNS)
    strcat (buf, " horns");
  if (part_flags & PART_SCALES)
    strcat (buf, " scales");
  if (part_flags & PART_TUSKS)
    strcat (buf, " tusks");
  if (part_flags & PART_FEATHERS)
    strcat (buf, " feathers");
  if (part_flags & PART_EYE)
    strcat (buf, " eye");
  return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *weapon_bit_name (int weapon_flags)
{
  static char buf[512];
  buf[0] = '\0';
  if (weapon_flags & WEAPON_FLAMING)
    strcat (buf, " flaming");
  if (weapon_flags & WEAPON_FROST)
    strcat (buf, " frost");
  if (weapon_flags & WEAPON_VAMPIRIC)
    strcat (buf, " vampiric");
  if (weapon_flags & WEAPON_SHARP)
    strcat (buf, " sharp");
  if (weapon_flags & WEAPON_VORPAL)
    strcat (buf, " vorpal");
  if (weapon_flags & WEAPON_TWO_HANDS)
    strcat (buf, " two-handed");
  if (weapon_flags & WEAPON_LANCE)
    strcat (buf, " lance");
  if (weapon_flags & WEAPON_SHOCKING)
    strcat (buf, " shocking");
  if (weapon_flags & WEAPON_POISON)
    strcat (buf, " poison");
  if (weapon_flags & WEAPON_THROWING)
    strcat (buf, " throwing");
  if (weapon_flags & WEAPON_APATHY)
    strcat (buf, " apathy");
  if (weapon_flags & WEAPON_WINDSLASH)
    strcat (buf, " windslash");
  if (weapon_flags & WEAPON_LARVA)
    strcat (buf, " larva");
  if (weapon_flags & WEAPON_HOLY)
    strcat (buf, " holy");
  return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *cont_bit_name (int cont_flags)
{
  static char buf[512];
  buf[0] = '\0';
  if (cont_flags & CONT_CLOSEABLE)
    strcat (buf, " closable");
  if (cont_flags & CONT_PICKPROOF)
    strcat (buf, " pickproof");
  if (cont_flags & CONT_CLOSED)
    strcat (buf, " closed");
  if (cont_flags & CONT_LOCKED)
    strcat (buf, " locked");
  return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *off_bit_name (int off_flags)
{
  static char buf[512];
  buf[0] = '\0';
  if (off_flags & OFF_AREA_ATTACK)
    strcat (buf, " area attack");
  if (off_flags & OFF_BACKSTAB)
    strcat (buf, " backstab");
  if (off_flags & OFF_BASH)
    strcat (buf, " bash");
  if (off_flags & OFF_BERSERK)
    strcat (buf, " berserk");
  if (off_flags & OFF_TRAMPLE)
    strcat (buf, " trample");
  if (off_flags & OFF_DISARM)
    strcat (buf, " disarm");
  if (off_flags & OFF_DODGE)
    strcat (buf, " dodge");
  if (off_flags & OFF_FADE)
    strcat (buf, " fade");
  if (off_flags & OFF_FAST)
    strcat (buf, " fast");
  if (off_flags & OFF_KICK)
    strcat (buf, " kick");
  if (off_flags & OFF_KICK_DIRT)
    strcat (buf, " kick_dirt");
  if (off_flags & OFF_PARRY)
    strcat (buf, " parry");
  if (off_flags & OFF_RESCUE)
    strcat (buf, " rescue");
  if (off_flags & OFF_TAIL)
    strcat (buf, " tail");
  if (off_flags & OFF_TRIP)
    strcat (buf, " trip");
  if (off_flags & OFF_CRUSH)
    strcat (buf, " crush");
  if (off_flags & ASSIST_ALL)
    strcat (buf, " assist_all");
  if (off_flags & ASSIST_ALIGN)
    strcat (buf, " assist_align");
  if (off_flags & ASSIST_RACE)
    strcat (buf, " assist_race");
  if (off_flags & ASSIST_PLAYERS)
    strcat (buf, " assist_players");
  if (off_flags & ASSIST_GUARD)
    strcat (buf, " assist_guard");
  if (off_flags & ASSIST_VNUM)
    strcat (buf, " assist_vnum");
  if (off_flags & OFF_UPPERCUT)
    strcat (buf, " uppercut");
  if (off_flags & OFF_COUNTER)
    strcat (buf, " counter");
  if (off_flags & ASSIST_EXACT_ALIGN)
    strcat (buf, " assist_exact_align");
  if (off_flags & OFF_KAI)
    strcat (buf, " kai");
  if (off_flags & OFF_HELLBREATH)
    strcat (buf, " charge");
  if (off_flags & OFF_STUN)
    strcat (buf, " stun");
  return (buf[0] != '\0') ? buf + 1 : "none";
}


//Iblis 6/07/03 - Needed to check Class in our new multiClass system
bool IS_CLASS (CHAR_DATA * ch, short Class)
{
  if (!ch || IS_NPC (ch))
    return FALSE;

  if (ch->level <= 30)
    return (ch->Class == Class);

  else if (ch->level <= 60)
    return (ch->Class == Class || ch->Class2 == Class);

  else
    return (ch->Class == Class || ch->Class2 == Class || ch->Class3 == Class);
}

//Iblis - Returns true if the character is of Class Class of at least tiers tiers
bool is_Class_tiers (CHAR_DATA * ch, short Class, short tiers)
{
  if (!ch || IS_NPC (ch))
    return FALSE;
  if (ch->level <= 30) 
  {
    if (tiers == 1)
      return (ch->Class == Class);
    else return FALSE;
  }
  else if (ch->level <= 60)
  {
    if (tiers == 2)
      return (ch->Class == Class && ch->Class2 == Class);
    else if (tiers == 1)
      return (ch->Class == Class || ch->Class2 == Class);
    else return FALSE;
  }
  else 
  {
    if (tiers == 1)
      return (ch->Class == Class || ch->Class2 == Class || ch->Class3 == Class);
    if (tiers == 3)
      return (ch->Class == Class && ch->Class2 == Class && ch->Class3 == Class);
    if (tiers == 2)
      return ((ch->Class == Class && ch->Class2 == Class)
	      || (ch->Class == Class && ch->Class3 == Class)
	      || (ch->Class2 == Class && ch->Class3 == Class));
    else return FALSE;
  }
}

//Iblis - returns the level needed for a certain skill.  Shortcircuited the original function to deal
//with the special skills necromancers have based on their old Class
short level_for_skill (CHAR_DATA* ch, short sn)
{
  int level = level_for_skill_2(ch,sn);
  if (!IS_NPC(ch) && ch->pcdata->old_Class != -1)
    {
      switch(ch->pcdata->old_Class)
        {
	case PC_CLASS_MAGE: if (sn == gsn_scribe || sn == gsn_scrolls)
	  level =  skill_table[sn].skill_level[ch->pcdata->old_Class];
	break;
	case PC_CLASS_CLERIC: if (sn == gsn_fast_healing)
	  level =  skill_table[sn].skill_level[ch->pcdata->old_Class];
	break;
	case PC_CLASS_THIEF: if (sn == gsn_hide)
	  level =  skill_table[sn].skill_level[ch->pcdata->old_Class];
	break;
	case PC_CLASS_WARRIOR: if (sn == gsn_disarm)
	  level =  skill_table[sn].skill_level[ch->pcdata->old_Class];
	break;
	case PC_CLASS_RANGER: if (sn == gsn_endurance)
	  level =  skill_table[sn].skill_level[ch->pcdata->old_Class];
	break;
	case PC_CLASS_BARD: if (sn == gsn_influence || sn == gsn_palm)
	  level =  skill_table[sn].skill_level[ch->pcdata->old_Class];
	break;
	case PC_CLASS_PALADIN: if (sn == gsn_leadership)
	  level =  skill_table[sn].skill_level[ch->pcdata->old_Class];
	break;
	case PC_CLASS_ASSASSIN: if (sn == gsn_throwing)
	  level =  skill_table[sn].skill_level[ch->pcdata->old_Class];
	break;
	case PC_CLASS_REAVER: if (sn == gsn_sword)
	  level =  skill_table[sn].skill_level[ch->pcdata->old_Class];
	break;
	case PC_CLASS_MONK: if (sn == gsn_sense_life)
	  level =  skill_table[sn].skill_level[ch->pcdata->old_Class];
	break;
	default: level = 92;
        }
    }
  if (sn == gsn_lumberjacking && ch->pcdata->learned[sn] > 0)
    level = 90;
  if (!IS_NPC(ch) && level > ch->level && ch->pcdata->mod_learned[sn] > 0)
    return 1;
  return level;
}

//Iblis - Main function for returning the level of a skill.  I essentially rewrote this for the new
//Class system.  I also added the race specific skills here so they can be dealt with more efficiently
//than mask for sidhe was, and probably easier than adding a new spot for every skill to tell if
//a particular race can use the skill or not
short level_for_skill_2 (CHAR_DATA * ch, short sn)
{
  int level = 0;
  level = skill_table[sn].skill_level[ch->Class];
  if (ch->race == PC_RACE_KALIAN)
  { 
    if (sn == gsn_fear)
      return 30;
    if (sn == gsn_suck)
      return 1;
    if (sn == gsn_illusion)
    {
      if (ch->level > 30)
        return 51;
      else return 92;
    }
  }
  else if (ch->race == PC_RACE_NERIX)
  {
    if (sn == gsn_lightningbreath)
    {
      if (ch->level > 60)
        return 90;
      else return 92;
    }
    if (sn == gsn_swoop)
      return 1;
    if (sn == gsn_uppercut || sn == gsn_riding || sn == gsn_parry || sn == gsn_hand_to_hand
        || sn == gsn_fisticuffery || sn == gsn_axe || sn == gsn_flail || sn == gsn_dagger
	|| sn == gsn_mace || sn == gsn_polearm || sn == gsn_sword || sn == gsn_staff || sn == gsn_dice)
     return 92;
  }
  else if (ch->race == PC_RACE_LICH)
  {
    if (sn == gsn_defile)
      return 10;
    if (sn == gsn_contaminate)
      return 30;
    if (sn == gsn_despoil)
      return 20;
  }
  else if (ch->race == PC_RACE_LITAN)
  {
    if (sn == gsn_flare)
      return 15;
  }
  else if (ch->race == PC_RACE_NIDAE)
  {
    if (sn == gsn_aquatic_sense)
      return 35;
    else if (sn == gsn_bubble)
      return 10;
    else if (sn == gsn_locate)
      return 40;
    else if (sn == gsn_spear)
      return 29;
    else if (sn == gsn_transform)
      return 90;
    else if (sn == gsn_call_storm)
      return 15;
    else if (sn == gsn_find_land)
      return 30;
  }
  if (sn == gsn_mask)
  {
    if (ch->race == PC_RACE_SIDHE || IS_IMMORTAL(ch))
      return 10;
    else if (ch->Class == PC_CLASS_ASSASSIN && is_Class_tiers(ch,PC_CLASS_ASSASSIN,2))
      return 50;
    else if (ch->Class2 == PC_CLASS_ASSASSIN && is_Class_tiers(ch,PC_CLASS_ASSASSIN,2))
      return 80;
    else return 92;
  }
  if (sn == gsn_swim)
  {
    if (ch->race == PC_RACE_DWARF)
      return 92;
    else if (ch->race == PC_RACE_CANTHI || ch->race == PC_RACE_NIDAE)
      return 1;
  }
/*  if (sn == gsn_steal)
  {
    if (ch->Class == PC_CLASS_CHAOS_JESTER)
      return 1;
  }*/

  if (ch->level <= 30)
    {
      if (level <= 30)
	return level;

      else
	return 92;
    }
  if (level <= 30)
    return level;
  if (ch->level <= 60)
    {
      if (ch->Class == ch->Class2)
	{
	  if (level <= 60)
	    return level;

	  else
	    return 92;
	}
      level = skill_table[sn].skill_level[ch->Class2] + 30;
      if (level <= 60)
	return level;

      else
	return 92;
    }
  if (ch->Class == ch->Class2 && ch->Class == ch->Class3)
  {
    if (level < 91)
      return level;
    else
      return 92;
  }
  if (ch->Class == ch->Class2)
    {
      if (level <= 60)
	return level;
      level = skill_table[sn].skill_level[ch->Class3] + 60;
      if (level <= 90)
	return level;

      else
	return 92;
    }
  if (ch->Class == ch->Class3)
    {
      if (level <= 60)
	{
	  if (level + 30 <= skill_table[sn].skill_level[ch->Class2] + 30)
	    return level + 30;

	  else
	    return skill_table[sn].skill_level[ch->Class2] + 30;
	}
      level = skill_table[sn].skill_level[ch->Class2] + 30;
      if (level <= 60)
	return level;

      else
	return 92;
    }
  if (ch->Class3 == ch->Class2)
    {
      level = skill_table[sn].skill_level[ch->Class2] + 30;
      if (level <= 90)
	return level;

      else
	return 92;
    }
  if ((level = skill_table[sn].skill_level[ch->Class2] + 30) <= 60)
    return level;
  if ((level = skill_table[sn].skill_level[ch->Class3] + 60) <= 90)
    return level;

  else
    return 92;
}

/*
 *  Iblis - 7/3/04 - Returns the coded level of the skill, regardless 
 *  of what teirs a person is. This is used for scribing so a mag/mag/war 
 *  can scribe the exact same as a war/mag/mag
 */
int true_level_for_skill(CHAR_DATA *ch, int sn)
{
  int level=92;
  if (skill_table[sn].skill_level[ch->Class] < Class_level(ch,ch->Class))
    if (level > skill_table[sn].skill_level[ch->Class])
      level = skill_table[sn].skill_level[ch->Class];
  if (skill_table[sn].skill_level[ch->Class2] < Class_level(ch,ch->Class2))
    if (level > skill_table[sn].skill_level[ch->Class2])
      level = skill_table[sn].skill_level[ch->Class2];
  if (skill_table[sn].skill_level[ch->Class3] < Class_level(ch,ch->Class3))
    if (level > skill_table[sn].skill_level[ch->Class3])
      level = skill_table[sn].skill_level[ch->Class3];

     return(level);
   }
    

short level_for_command (CHAR_DATA * ch, short sn)
{

  //int level = 100//, level2=100;
  if (ch->level > 60)
    return
      UMIN (UMIN
	    (command_table[sn].skill_level[ch->Class],
	     command_table[sn].skill_level[ch->Class2]),
	    command_table[sn].skill_level[ch->Class3]);

  else if (ch->level > 30)
    return UMIN (command_table[sn].skill_level[ch->Class],
		 command_table[sn].skill_level[ch->Class2]);

  else
    return command_table[sn].skill_level[ch->Class];
}

bool mana_using_Class (CHAR_DATA * ch)
{
  if (ch->level > 60)
    return (Class_table[ch->Class].fMana
	    || Class_table[ch->Class2].fMana
	    || Class_table[ch->Class3].fMana);

  else if (ch->level > 30)
    return (Class_table[ch->Class].fMana || Class_table[ch->Class2].fMana);

  else
    return Class_table[ch->Class].fMana;
}

bool Class_flagged_obj (OBJ_DATA * obj, CHAR_DATA * ch)
{
  short counter1 = 0;
  if (IS_SET
      (obj->Class_flags,
       flag_value (Class_flags, Class_table[ch->Class].name)))
    counter1 += (ch->level > 30) ? 30 : Class_level (ch, ch->Class);
  if (ch->level > 30
      &&
      (IS_SET
       (obj->Class_flags,
	flag_value (Class_flags, Class_table[ch->Class2].name))))
    counter1 += (ch->level > 60) ? 30 : Class_level (ch, ch->Class2);
  if (ch->level > 60
      &&
      (IS_SET
       (obj->Class_flags,
	flag_value (Class_flags, Class_table[ch->Class3].name))))
    counter1 +=
      (Class_level (ch, ch->Class3) > 30) ? 30 : Class_level (ch, ch->Class3);
  if (obj->level - EQUIP_LEVEL_DIFF <= counter1)
    return TRUE;

//  if (obj->level - EQUIP_LEVEL_DIFF <= 60 && counter1 >= 2)
//    return TRUE;
  //if (obj->level - EQUIP_LEVEL_DIFF <= 90 && counter1 >= 3)
  //  return TRUE;
  return FALSE;
}

short rating_for_skill (CHAR_DATA * ch, short sn)
{
  short rvalue = 500, rating = 0, i = 0, current_Class = 0;
  for (i = 0; i < 3; i++)
    {
      if (i == 0)
	current_Class = ch->Class;

      else if (i == 1)
	{
	  if (ch->level > 30)
	    current_Class = ch->Class2;

	  else
	    return rating;
	}

      else if (i == 2)
	{
	  if (ch->level > 60)
	    current_Class = ch->Class3;

	  else
	    return rating;
	}
      rating = skill_table[sn].rating[current_Class];
      if (rating == 0)
	rating = 8;
      if (skill_table[sn].skill_level[current_Class] > 60)
	rating *= 3;

      else if (skill_table[sn].skill_level[current_Class] > 30)
	rating *= 2;
      if (rating < rvalue)
	rvalue = rating;
    }
  return rvalue;
}

//Iblis - returns the Class the character is currently leveling in
short current_Class (CHAR_DATA * ch)
{
  if (IS_NPC (ch))
    return ch->Class;
  if (ch->level < 31)
    return ch->Class;

  else if (ch->level < 61)
    return ch->Class2;

  else
    return ch->Class3;
}

//Iblis - returns the amount of levels a character has of "Class" Class
short Class_level (CHAR_DATA * ch, short Class)
{
  short level = 0;
  if (ch->level <= 30)
    {
      if (ch->Class == Class)
	level = ch->level;
    }

  else
    {
      if (ch->Class == Class)
	level += 30;
      if (ch->level <= 60)
	{
	  if (ch->Class2 == Class)
	    level += (ch->level - 30);
	}

      else
	{
	  if (ch->Class2 == Class)
	    level += 30;
	  if (ch->Class3 == Class)
	    level += (ch->level - 60);
	}
    }
  return level;
}

//Iblis - I finally wrote a fucking change alignment function to deal with special cases 
//(such as sidhe, nerix, familiars, and liches) rather than having to cut/paste the same goddamn
//restrictions EVERY place align changes
void change_alignment(CHAR_DATA *ch,int amount)
{
  if (ch->race == PC_RACE_SIDHE || IS_SET(ch->act2,ACT_FAMILIAR) || ch->race == PC_RACE_NERIX)
    return;
  ch->alignment += amount;
  if (ch->alignment > 1000)
    ch->alignment = 1000;
  else if (ch->alignment < -1000)
    ch->alignment = -1000;
  if (ch->race == PC_RACE_LICH && ch->alignment > -350)
      ch->alignment = -350;
}


CHAR_DATA *get_random_mob_room(CHAR_DATA *ch)
{
  CHAR_DATA *mch=NULL;
  int total=0,counter=0;
  if (!ch->in_room)
    return NULL;
  for (mch = ch->in_room->people;mch != NULL;mch = mch->next_in_room)
  {
    if (!IS_NPC(mch) || mch == ch)
      continue;
    ++total;
  }
  
  total = number_range(1,total);
  for (mch = ch->in_room->people;mch != NULL;mch = mch->next_in_room)
    {
      if (!IS_NPC(mch) || mch == ch)
	continue;
      if (++counter == total)
	return mch;
    }
  return NULL;
}

CHAR_DATA *get_random_player_room(CHAR_DATA *ch)
{
  CHAR_DATA *mch=NULL;
  int total=0,counter=0;
  if (!ch->in_room)
    return NULL;
  for (mch = ch->in_room->people;mch != NULL;mch = mch->next_in_room)
    {
      if (IS_NPC(mch) || mch == ch)
	continue;
      ++total;
    }

  total = number_range(1,total);
  for (mch = ch->in_room->people;mch != NULL;mch = mch->next_in_room)
    {
      if (IS_NPC(mch) || mch == ch)
        continue;
      if (++counter == total)
        return mch;
    }
  return NULL;
}

int get_hitroll(CHAR_DATA *ch)
{
  int hitroll = GET_HITROLL(ch);
  if (!IS_NPC(ch))
  {
    if (ch->pcdata->personality == PERS_AGGRESSIVE)
      hitroll += 5;
    else if (ch->pcdata->personality == PERS_PASSIVE)
      hitroll -= 5;
  }
  return hitroll;
}

int get_damroll(CHAR_DATA *ch)
{
  int damroll = GET_DAMROLL(ch);
  if (!IS_NPC(ch))
    {
      if (ch->pcdata->personality == PERS_AGGRESSIVE)
	damroll += 5;
      else if (ch->pcdata->personality == PERS_PASSIVE)
	damroll -= 5;
    }
  return damroll;
}

int hands_used(CHAR_DATA *ch)
{
  OBJ_DATA *obj;
  int hands = 0;
  if (get_eq_char_new (ch,ITEM_WEAR_SHIELD))
    ++hands;
  if ((obj = get_eq_char_new (ch,ITEM_HOLD)) != NULL)
    ++hands;
  if (obj && obj->item_type == ITEM_INSTRUMENT && IS_SET (obj->value[0], A) && ch->size < SIZE_LARGE) 
    ++hands;
  if ((obj = get_eq_char_new (ch,ITEM_WIELD)) != NULL)
    ++hands;
//  if (obj && ch->size < SIZE_LARGE && IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS))
//    ++hands;
  if ((obj = get_eq_char_new (ch,ITEM_TWO_HANDS)) != NULL)
    ++hands;
//  if (!IS_NPC(ch) && obj && ch->size < SIZE_LARGE && IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS))
//    ++hands;
  return hands;
}

int hands_required(CHAR_DATA *ch, OBJ_DATA *obj)
{
  //  OBJ_DATA *obj;
  int hands = 0;
  if (IS_SET(obj->wear_flags,ITEM_WEAR_SHIELD))
    ++hands;
  if (IS_SET(obj->wear_flags,ITEM_HOLD))
    ++hands;
  if (obj->item_type == ITEM_INSTRUMENT && IS_SET (obj->value[0], A) && ch->size < SIZE_LARGE)
    ++hands;
  if (IS_SET(obj->wear_flags,ITEM_WIELD))
    ++hands;
  if (!IS_NPC(ch) && obj->item_type == ITEM_WEAPON && ch->size < SIZE_LARGE && IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS))
    ++hands;
  /*  if (IS_SET(obj->wear_flags,ITEM_TWO_HANDS))
    ++hands;
  if (obj && ch->size < SIZE_LARGE && IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS))
  ++hands;*/
  return hands;
}


