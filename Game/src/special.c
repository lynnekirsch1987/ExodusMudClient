#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>
#include "merc.h"
#include "pray.h"
#include "magic.h"
#include "clans/new_clans.h"
#include "clans/new_clans_util.h"

void do_reroll args((CHAR_DATA * ch, char *argument));
void do_goto args((CHAR_DATA * ch, char *argument));
void affect_strip_skills args((CHAR_DATA * ch));
OBJ_DATA *get_obj_vnum_char args((CHAR_DATA* ch, int vnum));
void save_clan args((int clanslot));

extern long genraces[];
extern bool battle_royale;
extern struct clan_type clan_table[MAX_CLAN];	

/* command procedures needed */
DECLARE_DO_FUN (do_yell);
DECLARE_DO_FUN (do_open);
DECLARE_DO_FUN (do_close);
DECLARE_DO_FUN (do_say);
DECLARE_DO_FUN (do_backstab);
DECLARE_DO_FUN (do_flee);
DECLARE_DO_FUN (do_murder);

/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN (spec_bard);
DECLARE_SPEC_FUN (spec_breath_any);
DECLARE_SPEC_FUN (spec_breath_acid);
DECLARE_SPEC_FUN (spec_breath_fire);
DECLARE_SPEC_FUN (spec_breath_frost);
DECLARE_SPEC_FUN (spec_breath_gas);
DECLARE_SPEC_FUN (spec_breath_lightning);
DECLARE_SPEC_FUN (spec_cast_adept);
DECLARE_SPEC_FUN (spec_cast_cleric);
DECLARE_SPEC_FUN (spec_cast_judge);
DECLARE_SPEC_FUN (spec_cast_mage);
DECLARE_SPEC_FUN (spec_cast_healer);
DECLARE_SPEC_FUN (spec_cast_underwater);
DECLARE_SPEC_FUN (spec_cast_undead);
DECLARE_SPEC_FUN (spec_executioner);
DECLARE_SPEC_FUN (spec_fido);
DECLARE_SPEC_FUN (spec_guard);
DECLARE_SPEC_FUN (spec_janitor);
DECLARE_SPEC_FUN (spec_mayor);
DECLARE_SPEC_FUN (spec_poison);
DECLARE_SPEC_FUN (spec_thief);
DECLARE_SPEC_FUN (spec_nasty);
DECLARE_SPEC_FUN (spec_troll_member);
DECLARE_SPEC_FUN (spec_ogre_member);
DECLARE_SPEC_FUN (spec_patrolman);
DECLARE_SPEC_FUN (spec_wanted_checker);
DECLARE_SPEC_FUN (spec_shopkeeper_talker);

/* the function table */
const struct spec_type spec_table[] = {
  {"spec_bard", spec_bard},
  {"spec_breath_any", spec_breath_any},
  {"spec_breath_acid", spec_breath_acid},
  {"spec_breath_fire", spec_breath_fire},
  {"spec_breath_frost", spec_breath_frost},
  {"spec_breath_gas", spec_breath_gas},
  {"spec_breath_lightning", spec_breath_lightning},
  {"spec_cast_adept", spec_cast_adept},
  {"spec_cast_cleric", spec_cast_cleric},
  {"spec_cast_judge", spec_cast_judge},
  {"spec_cast_mage", spec_cast_mage},
  {"spec_cast_healer", spec_cast_healer},
  /* BEGIN UNDERWATER */
  {"spec_cast_underwater", spec_cast_underwater},
  /* END UNDERWATER */
  {"spec_cast_undead", spec_cast_undead},
  {"spec_executioner", spec_executioner},
  {"spec_fido", spec_fido},
  {"spec_guard", spec_guard},
  {"spec_janitor", spec_janitor},
  {"spec_mayor", spec_mayor},
  {"spec_poison", spec_poison},
  {"spec_thief", spec_thief},
  {"spec_nasty", spec_nasty},
  {"spec_troll_member", spec_troll_member},
  {"spec_ogre_member", spec_ogre_member},
  {"spec_patrolman", spec_patrolman}, 
  {"spec_wanted_checker", spec_wanted_checker},
  {"spec_shopkeeper_talker", spec_shopkeeper_talker},
  {NULL, NULL}
};
char *spec_string (SPEC_FUN * fun)
{				/* OLC */
  int cmd;
  for (cmd = 0; spec_table[cmd].function != NULL; cmd++)
    if (fun == spec_table[cmd].function)
      return spec_table[cmd].name;
  return 0;
}

SPEC_FUN *spec_lookup (const char *name)
{				/* OLC */
  int cmd;
  for (cmd = 0; spec_table[cmd].name != NULL; cmd++)
    if (!str_cmp (name, spec_table[cmd].name))
      return spec_table[cmd].function;
  return 0;
}

char *spec_name (SPEC_FUN * function)
{
  int i;
  for (i = 0; spec_table[i].function != NULL; i++)
    {
      if (function == spec_table[i].function)
	return spec_table[i].name;
    }
  return NULL;
}

bool spec_troll_member (CHAR_DATA * ch)
{
  CHAR_DATA *vch, *victim = NULL;
  int count = 0;
  char *message;
  if (!IS_AWAKE (ch) || IS_AFFECTED (ch, AFF_CALM)
      || is_affected (ch, skill_lookup ("Quiet Mountain Lake"))
      || ch->in_room == NULL || IS_AFFECTED (ch, AFF_CHARM)
      || ch->fighting == NULL)
    return FALSE;

  /* find an ogre to beat up */
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (!IS_NPC (vch) || ch == vch)
	continue;
      if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
	return FALSE;
      if (vch->pIndexData->group == GROUP_VNUM_OGRES
	  && ch->level > vch->level - 2 && !is_safe (ch, vch))
	{
	  if (number_range (0, count) == 0)
	    victim = vch;
	  count++;
	}
    }
  if (victim == NULL)
    return FALSE;

  /* say something, then raise hell */
  switch (number_range (0, 6))
    {
    default:
      message = NULL;
      break;
    case 0:
      message = "$n yells 'I've been looking for you, punk!'";
      break;
    case 1:
      message = "With a scream of rage, $n attacks $N.";
      break;
    case 2:
      message =
	"$n says 'What's slimy Ogre trash like you doing around here?'";
      break;
    case 3:
      message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
      break;
    case 4:
      message = "$n says 'There's no cops to save you this time!'";
      break;
    case 5:
      message = "$n says 'Time to join your brother, spud.'";
      break;
    case 6:
      message = "$n says 'Let's rock.'";
      break;
    }
  if (message != NULL)
    act (message, ch, NULL, victim, TO_ALL);
  multi_hit (ch, victim, TYPE_UNDEFINED);
  return TRUE;
}

bool spec_wanted_checker (CHAR_DATA * ch)
{
  CHAR_DATA *vch;
  char buf[MAX_STRING_LENGTH];
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
  {
    if (IS_NPC (vch))
      continue;
    if (!IS_SET(vch->act,PLR_WANTED) || vch->level != 90)
      continue;
    sprintf(buf,"You have passed my test, %s.  You may proceed.",vch->name);
    do_say(ch,buf);
    act("$n disappears with a *POP* and a cloud of smoke.",vch,NULL,NULL,TO_ROOM);
    char_from_room(vch);
    char_to_room(vch,get_room_index(7307));
    return TRUE;
  }
  return FALSE;
}
    
			    

bool spec_ogre_member (CHAR_DATA * ch)
{
  CHAR_DATA *vch, *victim = NULL;
  int count = 0;
  char *message;
  if (!IS_AWAKE (ch) || IS_AFFECTED (ch, AFF_CALM)
      || is_affected (ch, skill_lookup ("Quiet Mountain Lake"))
      || ch->in_room == NULL || IS_AFFECTED (ch, AFF_CHARM)
      || ch->fighting == NULL)
    return FALSE;

  /* find an troll to beat up */
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (!IS_NPC (vch) || ch == vch)
	continue;
      if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
	return FALSE;
      if (vch->pIndexData->group == GROUP_VNUM_TROLLS
	  && ch->level > vch->level - 2 && !is_safe (ch, vch))
	{
	  if (number_range (0, count) == 0)
	    victim = vch;
	  count++;
	}
    }
  if (victim == NULL)
    return FALSE;

  /* say something, then raise hell */
  switch (number_range (0, 6))
    {
    default:
      message = NULL;
      break;
    case 0:
      message = "$n yells 'I've been looking for you, punk!'";
      break;
    case 1:
      message = "With a scream of rage, $n attacks $N.'";
      break;
    case 2:
      message = "$n says 'What's Troll filth like you doing around here?'";
      break;
    case 3:
      message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
      break;
    case 4:
      message = "$n says 'There's no cops to save you this time!'";
      break;
    case 5:
      message = "$n says 'Time to join your brother, spud.'";
      break;
    case 6:
      message = "$n says 'Let's rock.'";
      break;
    }
  if (message != NULL)
    act (message, ch, NULL, victim, TO_ALL);
  multi_hit (ch, victim, TYPE_UNDEFINED);
  return TRUE;
}

bool spec_patrolman (CHAR_DATA * ch)
{
  CHAR_DATA *vch, *victim = NULL;
  OBJ_DATA *obj;
  char *message;
  int count = 0;
  if (!IS_AWAKE (ch) || IS_AFFECTED (ch, AFF_CALM)
      || is_affected (ch, skill_lookup ("Quiet Mountain Lake"))
      || ch->in_room == NULL || IS_AFFECTED (ch, AFF_CHARM)
      || ch->fighting == NULL)
    return FALSE;

  /* look for a fight in the room */
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (vch == ch)
	continue;
      if (vch->fighting != NULL)
	{			/* break it up! */
	  if (number_range (0, count) == 0)
	    victim = (vch->level > vch->fighting->level)
	      ? vch : vch->fighting;
	  count++;
	}
    }
  if (victim == NULL || (IS_NPC (victim) && victim->spec_fun == ch->spec_fun))
    return FALSE;
  if (((obj = get_eq_char (ch, WEAR_NECK_1)) != NULL
       && obj->pIndexData->vnum == OBJ_VNUM_WHISTLE)
      || ((obj = get_eq_char (ch, WEAR_NECK_2)) != NULL
	  && obj->pIndexData->vnum == OBJ_VNUM_WHISTLE))
    {
      act ("You blow down hard on $p.", ch, obj, NULL, TO_CHAR);
      act ("$n blows on $p, ***WHEEEEEEEEEEEET***", ch, obj, NULL, TO_ROOM);
      for (vch = char_list; vch != NULL; vch = vch->next)
	{
	  if (vch->in_room == NULL)
	    continue;
	  if (vch->in_room != ch->in_room
	      && vch->in_room->area == ch->in_room->area)
	    send_to_char ("You hear a shrill whistling sound.\n\r", vch);
	}
    }
  switch (number_range (0, 6))
    {
    default:
      message = NULL;
      break;
    case 0:
      message = "$n yells 'All roit! All roit! break it up!'";
      break;
    case 1:
      message = "$n says 'Society's to blame, but what's a bloke to do?'";
      break;
    case 2:
      message = "$n mumbles 'bloody kids will be the death of us all.'";
      break;
    case 3:
      message = "$n shouts 'Stop that! Stop that!' and attacks.";
      break;
    case 4:
      message = "$n pulls out his billy and goes to work.";
      break;
    case 5:
      message = "$n sighs in resignation and proceeds to break up the fight.";
      break;
    case 6:
      message = "$n says 'Settle down, you hooligans!'";
      break;
    }
  if (message != NULL)
    act (message, ch, NULL, NULL, TO_ALL);
  multi_hit (ch, victim, TYPE_UNDEFINED);
  return TRUE;
}

bool spec_nasty (CHAR_DATA * ch)
{
  CHAR_DATA *victim, *v_next;
  long gold;
  if (!IS_AWAKE (ch))
    {
      return FALSE;
    }
  if (ch->position >= POS_FIGHTING && ch->fighting != NULL)
    {
      for (victim = ch->in_room->people; victim != NULL; victim = v_next)
	{
	  v_next = victim->next_in_room;
	  if (!IS_NPC (victim)
	      && (victim->level > ch->level)
	      && (victim->level < ch->level + 10))
	    {
	      do_backstab (ch, victim->name);
	      if (ch->position >= POS_FIGHTING && ch->fighting == NULL)
		do_murder (ch, victim->name);

	      /* should steal some coins right away? :) */
	      return TRUE;
	    }
	}
      return FALSE;		/*  No one to attack */
    }

  /* okay, we must be fighting.... steal some coins and flee */
  if ((victim = ch->fighting) == NULL)
    return FALSE;		/* let's be paranoid.... */
  switch (number_bits (2))
    {
    case 0:
      act ("$n rips apart your coin purse, spilling your gold!", ch,
	   NULL, victim, TO_VICT);
      act ("You slash apart $N's coin purse and gather his gold.", ch,
	   NULL, victim, TO_CHAR);
      act ("$N's coin purse is ripped apart!", ch, NULL, victim, TO_NOTVICT);
      gold = victim->gold / 10;	/* steal 10% of his gold */
      victim->gold -= gold;
      ch->gold += gold;
      return TRUE;
    case 1:
      do_flee (ch, "");
      return TRUE;
    default:
      return FALSE;
    }
}


/*
 * Core procedure for dragons.
 */
bool dragon (CHAR_DATA * ch, char *spell_name)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  int sn;
  if (ch->position < POS_FIGHTING || ch->fighting == NULL)
    return FALSE;
  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim->fighting == ch && number_bits (3) == 0)
	break;
    }
  if (victim == NULL)
    return FALSE;
  if ((sn = skill_lookup (spell_name)) < 0)
    return FALSE;
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
  return TRUE;
}


/*
 * Special procedures for mobiles.
 */
bool spec_breath_any (CHAR_DATA * ch)
{
  if (ch->position < POS_FIGHTING || ch->fighting == NULL)
    return FALSE;
  switch (number_bits (3))
    {
    case 0:
      return spec_breath_fire (ch);
    case 1:
    case 2:
      return spec_breath_lightning (ch);
    case 3:
      return spec_breath_gas (ch);
    case 4:
      return spec_breath_acid (ch);
    case 5:
    case 6:
    case 7:
      return spec_breath_frost (ch);
    }
  return FALSE;
}

bool spec_breath_acid (CHAR_DATA * ch)
{
  return dragon (ch, "acid breath");
}

bool spec_breath_fire (CHAR_DATA * ch)
{
  return dragon (ch, "fire breath");
}

bool spec_breath_frost (CHAR_DATA * ch)
{
  return dragon (ch, "frost breath");
}

bool spec_breath_gas (CHAR_DATA * ch)
{
  int sn;
  if (ch->position < POS_FIGHTING || ch->fighting == NULL)
    return FALSE;
  if ((sn = skill_lookup ("gas breath")) < 0)
    return FALSE;
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, NULL, TARGET_CHAR);
  return TRUE;
}

bool spec_breath_lightning (CHAR_DATA * ch)
{
  return dragon (ch, "lightning breath");
}

bool spec_cast_adept (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  if (!IS_AWAKE (ch))
    return FALSE;
  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim != ch && can_see (ch, victim) && number_bits (1) == 0
	  && !IS_NPC (victim) && victim->level < 11)
	break;
    }
  if (victim == NULL)
    return FALSE;
  switch (number_bits (4))
    {
    case 0:
      act ("$n utters the word 'abrazak'.", ch, NULL, NULL, TO_ROOM);
      spell_armor (skill_lookup ("armor"), ch->level, ch, victim,
		   TARGET_CHAR);
      return TRUE;
    case 1:
      act ("$n utters the word 'fido'.", ch, NULL, NULL, TO_ROOM);
      prayer_bless (skill_lookup ("bless"), ch->level, ch, victim,
		    TARGET_CHAR);
      return TRUE;
    case 2:
      act ("$n utters the words 'judicandus noselacri'.", ch, NULL, NULL,
	   TO_ROOM);
      spell_cure_blindness (skill_lookup ("cure blindness"), ch->level,
			    ch, victim, TARGET_CHAR);
      return TRUE;
    case 3:
      act ("$n utters the words 'judicandus dies'.", ch, NULL, NULL, TO_ROOM);
      spell_cure_light (skill_lookup ("cure light"), ch->level, ch,
			victim, TARGET_CHAR);
      return TRUE;
    case 4:
      act ("$n utters the words 'judicandus sausabru'.", ch, NULL, NULL,
	   TO_ROOM);
      spell_cure_poison (skill_lookup ("cure poison"), ch->level, ch,
			 victim, TARGET_CHAR);
      return TRUE;
    case 5:
      act ("$n utters the word 'candusima'.", ch, NULL, NULL, TO_ROOM);
      spell_refresh (skill_lookup ("refresh"), ch->level, ch, victim,
		     TARGET_CHAR);
      return TRUE;
    case 6:
      act ("$n utters the words 'judicandus eugzagz'.", ch, NULL, NULL,
	   TO_ROOM);
      spell_cure_disease (skill_lookup ("cure disease"), ch->level, ch,
			  victim, TARGET_CHAR);
    }
  return FALSE;
}

bool spec_cast_cleric (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn, num_do;
  num_do = number_range (0, 2);
  switch (num_do)
    {
    case 0:
      if (number_range (0, 17) != 0)
	return FALSE;
      for (victim = ch->in_room->people; victim != NULL;
	   victim = victim->next_in_room)
	{
	  if ((is_same_group (victim, ch) || victim == ch)
	      && victim->hit <= victim->max_hit / 2)
	    {
	      if (number_range (0, 1) == 0)
		sn = skill_lookup ("heal");

	      else
		sn = skill_lookup ("cure critical");
	      if (sn >= 0)
		(*skill_table[sn].spell_fun) (sn, ch->level, ch,
					      victim, TARGET_CHAR);
	      return TRUE;
	    }
	}
      break;
    case 1:
      if (ch->position < POS_FIGHTING || ch->fighting == NULL)
	return FALSE;
      for (victim = ch->in_room->people; victim != NULL; victim = v_next)
	{
	  v_next = victim->next_in_room;
	  if (victim->fighting == ch && number_bits (2) == 0)
	    break;
	}
      if (victim == NULL)
	return FALSE;
      for (;;)
	{
	  int min_level;
	  switch (number_bits (4))
	    {
	    case 0:
	      min_level = 0;
	      spell = "blindness";
	      break;
	    case 1:
	      min_level = 3;
	      spell = "cause serious";
	      break;
	    case 2:
	      min_level = 7;
	      spell = "earthquake";
	      break;
	    case 3:
	      min_level = 9;
	      spell = "cause critical";
	      break;
	    case 4:
	      min_level = 10;
	      spell = "dispel evil";
	      break;
	    case 5:
	      min_level = 12;
	      spell = "curse";
	      break;
	    case 6:
	      min_level = 12;
	      spell = "change sex";
	      break;
	    case 7:
	      min_level = 13;
	      spell = "flamestrike";
	      break;
	    case 8:
	    case 9:
	    case 10:
	      min_level = 15;
	      spell = "harm";
	      break;
	    case 11:
	      min_level = 15;
	      spell = "plague";
	      break;
	    default:
	      min_level = 16;
	      spell = "dispel magic";
	      break;
	    }
	  if (ch->level >= min_level)
	    break;
	}
      if ((sn = skill_lookup (spell)) < 0)
	return FALSE;
      (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
      return TRUE;
      break;
    }
  return FALSE;
}

bool spec_cast_judge (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;
  if (ch->position < POS_FIGHTING || ch->fighting == NULL)
    return FALSE;
  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim->fighting == ch && number_bits (2) == 0)
	break;
    }
  if (victim == NULL)
    return FALSE;
  spell = "high explosive";
  if ((sn = skill_lookup (spell)) < 0)
    return FALSE;
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
  return TRUE;
}


/* BEGIN UNDERWATER */
bool spec_cast_underwater (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;
  if (ch->position < POS_FIGHTING || ch->fighting == NULL)
    return FALSE;
  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim->fighting == ch && number_bits (2) == 0)
	break;
    }
  if (victim == NULL)
    return FALSE;
  for (;;)
    {
      int min_level;
      switch (number_bits (4))
	{
	case 0:
	  min_level = 0;
	  spell = "blindness";
	  break;
	case 1:
	  min_level = 3;
	  spell = "chill touch";
	  break;
	case 2:
	  min_level = 7;
	  spell = "electrical charge";
	  break;
	case 3:
	  min_level = 8;
	  spell = "teleport";
	  break;
	case 5:
	  min_level = 12;
	  spell = "earthquake";
	  break;
	case 6:
	  min_level = 13;
	  spell = "ice beam";
	  break;
	case 7:
	case 8:
	case 9:
	case 10:
	  min_level = 20;
	  spell = "channel electricity";
	  break;
	default:
	  min_level = 20;
	  spell = "energy drain";
	  break;
	}
      if (ch->level >= min_level)
	break;
    }
  if ((sn = skill_lookup (spell)) < 0)
    return FALSE;
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
  return TRUE;
}


/* END UNDERWATER */
bool spec_cast_mage (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;
  if (ch->position < POS_FIGHTING || ch->fighting == NULL)
    return FALSE;
  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim->fighting == ch && number_bits (2) == 0)
	break;
    }
  if (victim == NULL)
    return FALSE;
  for (;;)
    {
      int min_level;
      switch (number_bits (4))
	{
	case 0:
	  min_level = 4;
	  spell = "chill touch";
	  break;
	case 1:
	  min_level = 6;
	  spell = "faerie fire";
	  break;
	case 2:
	  min_level = 7;
	  spell = "burning hands";
	  break;
	case 3:
	  min_level = 10;
	  spell = "shocking grasp";
	  break;
	case 4:
	  min_level = 10;
	  spell = "sleep";
	  break;
	case 5:
	  min_level = 11;
	  spell = "weaken";
	  break;
	case 6:
	  min_level = 12;
	  spell = "blindness";
	  break;
	case 7:
          min_level = 13;
	  spell = "lightning bolt";
	  break;
	case 8:
          min_level = 13;
	  spell = "teleport";
	  break;
	case 9:
	  min_level = 14;
	  spell = "faerie fog";
	  break;
	case 10:
	  min_level = 16;
	  spell = "colour spray";
	  break;
	case 11:
	  min_level = 16;
	  spell = "dispel magic";
	  break;
	case 12:
	  min_level = 17;
	  spell = "poison";
	  break;
	case 13:
	  min_level = 18;
	  spell = "curse";
	  break;
	case 14:
	  min_level = 19;
	  spell = "energy drain";
	  break;
	case 15:
	  min_level = 22;
	  spell = "fireball";
	  break;
	case 16:
	  min_level = 23;
	  spell = "slow";
	  break;
	case 17:
	  min_level = 25;
	  spell = "change sex";
	  break;
	case 18:
	  min_level = 26;
	  spell = "call lightning";
	  break;
	case 19:
	case 20:
	  min_level = 47;
	  spell = "heat metal";
	  break;
	case 21:
	  min_level = 50;
	  spell = "acid blast";
	  break;
	case 22:
	  min_level = 55;
	  spell = "web";
	  break;
	default:
	  min_level = 50;
	  spell = "acid blast";
	  break;
	}
      if (ch->level >= min_level)
	break;
    }
  if ((sn = skill_lookup (spell)) < 0)
    return FALSE;
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
  return TRUE;
}

bool spec_bard (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;
  char buf[MAX_STRING_LENGTH];
  if (ch->position < POS_FIGHTING || ch->fighting == NULL)
    return FALSE;
  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim->fighting == ch && number_bits (2) == 0)
	break;
    }
  if (victim == NULL)
    return FALSE;
  for (;;)
    {
      int min_level;
      switch (number_bits (4))
	{
	case 0:
	  min_level = 14;
	  sprintf(buf,"$n sings the tune, 'Good Vibrations'.");
	  spell = "good vibrations";
	  break;
	case 1:
	  min_level = 28;
	  sprintf(buf,"$n sings the tune, 'Ballad of the Cantankerous Woodsman'.");
	  spell = "ballad of the cantankerous woodsman";
	  break;
	case 2:
	  min_level = 38;
	  sprintf(buf,"$n sings the tune, 'Maelstrom of Nyvenban'.");
	  spell = "maelstrom of nyvenban";
	  break;
	case 3:
	  min_level = 40;
	  sprintf(buf,"$n sings the tune, 'Dance of Shadows'.");
	  spell = "dance of shadows";
	  break;
	case 4:
	  min_level = 40;
	  sprintf(buf,"$n sings the tune, 'The Piper's Melody'.");
	  spell = "the pipers melody";
	  break;
	case 5:
	  min_level = 42;
	  sprintf(buf,"$n sings the tune, 'To the Four Corners'.");
	  spell = "to the four corners";
	  break;
	case 6:
	  min_level = 42;
	  sprintf(buf,"$n sings the tune, 'Path of the Nomad'.");
	  spell = "path of the nomad";
	  break;
	case 7:
          min_level = 44;
	  sprintf(buf,"$n sings the tune, 'Despair of Orpheus'.");
	  spell = "despair of orpheus";
	  break;
	case 8:
          min_level = 63;
	  sprintf(buf,"$n sings the tune, 'Song of Elyteness'.");
	  spell = "song of elyteness";
	  break;
	case 9:
          min_level = 72;
	  sprintf(buf,"$n sings the tune, 'Ballad of Ole Rip'.");
	  spell = "ballad of ole rip";
	  break;
	case 10:
          min_level = 85;
	  sprintf(buf,"$n sings the tune, 'Song of Shattered Dreams'.");
	  spell = "song of shattered dreams";
	  break;
	default:
	  min_level = 14;
	  sprintf(buf,"$n sings the tune, 'Good Vibrations'.");
	  spell = "good vibrations";
	  break;
	}
      if (ch->level >= min_level)
	break;
    }
  act (buf, ch, NULL, NULL, TO_ROOM);
  if ((sn = skill_lookup (spell)) < 0)
    return FALSE;
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
  return TRUE;
}

bool spec_cast_healer (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;
  if (ch->position < POS_FIGHTING || ch->fighting == NULL)
    return FALSE;
  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim->fighting == ch && number_bits (2) == 0)
	break;
    }
  if (victim == NULL)
    return FALSE;
  for (;;)
    {
      int min_level;
      switch (number_bits (4))
	{
	case 0:
	  min_level = 0;
          act ("$n orates the prayer 'lodibendi dies'.", ch, NULL, NULL, TO_ROOM);
	  spell = "cure light";
	  break;
	case 1:
	  min_level = 20;
          act ("$n orates the prayer 'lodibendi gzfuajg'.", ch, NULL, NULL, TO_ROOM);
	  spell = "cure serious";
	  break;
	case 2:
	  min_level = 40;
          act ("$n orates the prayer 'lodibendi qfuhuqar'.", ch, NULL, NULL, TO_ROOM);
	  spell = "cure critical";
	  break;
	case 3:
	  min_level = 60;
          act ("$n orates the prayer 'pzar'.", ch, NULL, NULL, TO_ROOM);
	  spell = "heal";
	  break;
	case 4:
	  min_level = 80;
          act ("$n orates the prayer 'obendiahzf pzar'.", ch, NULL, NULL, TO_ROOM);
	  spell = "greater heal";
	  break;
	default:
	  min_level = 0;
          act ("$n orates the prayer 'lodibendi dies'.", ch, NULL, NULL, TO_ROOM);
	  spell = "cure light";
	  break;
	}
      if (ch->level >= min_level)
	break;
    }
  if ((sn = skill_lookup (spell)) < 0)
    return FALSE;
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, ch, TARGET_CHAR);
  return TRUE;
}

bool spec_cast_undead (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;
  if (ch->position < POS_FIGHTING || ch->fighting == NULL)
    return FALSE;
  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim->fighting == ch && number_bits (2) == 0)
	break;
    }
  if (victim == NULL)
    return FALSE;
  for (;;)
    {
      int min_level;
      switch (number_bits (4))
	{
	case 0:
	  min_level = 0;
	  spell = "curse";
	  break;
	case 1:
	  min_level = 3;
	  spell = "weaken";
	  break;
	case 2:
	  min_level = 6;
	  spell = "chill touch";
	  break;
	case 3:
	  min_level = 9;
	  spell = "blindness";
	  break;
	case 4:
	  min_level = 12;
	  spell = "poison";
	  break;
	case 5:
	  min_level = 15;
	  spell = "energy drain";
	  break;
	case 6:
	  min_level = 18;
	  spell = "harm";
	  break;
	case 7:
	  min_level = 21;
	  spell = "teleport";
	  break;
	case 8:
	  min_level = 20;
	  spell = "plague";
	  break;
	default:
	  min_level = 18;
	  spell = "harm";
	  break;
	}
      if (ch->level >= min_level)
	break;
    }
  if ((sn = skill_lookup (spell)) < 0)
    return FALSE;
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
  return TRUE;
}

bool spec_executioner (CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  if (!IS_AWAKE (ch) || ch->fighting != NULL || battle_royale)
    return FALSE;
  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (!IS_NPC (victim) && (IS_SET (victim->act, PLR_WANTED) || victim->race == PC_RACE_LICH)
	  && can_see (ch, victim))
	break;
    }
  if (victim == NULL)
    return (FALSE);
  if (victim->race != PC_RACE_LICH)
  {
    sprintf (buf,
	   "It's the notorious criminal %s!! Guards, we must be unfailing in our hunt!",
	   victim->name);
  }
  else sprintf(buf,"Help me!  There's some hideous undead creature over here! Kill it quick!!!");
  REMOVE_BIT (ch->comm, COMM_NOSHOUT);
  do_yell (ch, buf);
  multi_hit (ch, victim, TYPE_UNDEFINED);
  return TRUE;
}

bool spec_fido (CHAR_DATA * ch)
{
  OBJ_DATA *corpse;
  OBJ_DATA *c_next;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  if (!IS_AWAKE (ch))
    return FALSE;
  for (corpse = ch->in_room->contents; corpse != NULL; corpse = c_next)
    {
      c_next = corpse->next_content;
      if (corpse->item_type != ITEM_CORPSE_NPC)
	continue;
      act ("$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM);
      for (obj = corpse->contains; obj; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  obj_from_obj (obj);
	  obj_to_room (obj, ch->in_room);
	}
      extract_obj (corpse);
      return TRUE;
    }
  return FALSE;
}

bool spec_guard (CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim, *v_next;
  bool found = FALSE;
  short door = 0;
  short chance = 85;
  if (!IS_AWAKE (ch) || ch->fighting != NULL || battle_royale)
    return FALSE;
  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (!IS_NPC (victim) && (IS_SET (victim->act, PLR_WANTED) || victim->race == PC_RACE_LICH)
	  && can_see (ch, victim))
	break;
    }
  if (victim != NULL && !IS_SET(victim->act, PLR_WANTED) && victim->race == PC_RACE_LICH)
  {
    sprintf(buf,"Help me!  There's some hideous undead creature over here! Kill it quick!!!");
    REMOVE_BIT (ch->comm, COMM_NOSHOUT);
    do_yell (ch, buf);
    multi_hit (ch, victim, TYPE_UNDEFINED);
    return TRUE;
  }
		  
  if (victim == NULL)
    {
      for (door = 0; door < MAX_DIR; door++)
	{
	  if (ch->in_room->exit[door] != NULL
	      && ch->in_room->exit[door]->u1.to_room != NULL
	      && !IS_SET (ch->in_room->exit[door]->exit_info, EX_CLOSED))
	    {
	      for (victim = ch->in_room->exit[door]->u1.to_room->people;
		   victim; victim = victim->next)
		if (!IS_NPC (victim)
		    && (IS_SET (victim->act, PLR_WANTED)) //|| victim->race == PC_RACE_LICH)
		    && can_see (ch, victim))
		  {
		    found = TRUE;
		    break;
		  }
	      if (found)
		break;
	    }
	}
      chance = 55;
    }
  if (victim != NULL)
    {
      short curr_dex, curr_wis;
      curr_dex = get_curr_stat (victim, STAT_DEX);
      curr_wis = get_curr_stat (victim, STAT_WIS);
      if (curr_dex < 13)
	chance += 25;

      else if (curr_dex > 20 && curr_dex < 24)
	chance -= 10;

      else if (curr_dex > 23)
	chance -= 15;
      if (curr_wis < 16)
	chance += 25;

      else if (curr_wis > 20 && curr_wis < 24)
	chance -= 10;

      else if (curr_wis > 23)
	chance -= 15;
      if (victim->times_wanted < 21);

      else if (victim->times_wanted < 41)
	chance += 5;

      else if (victim->times_wanted < 51)
	chance += 10;

      else if (victim->times_wanted < 76)
	chance += 20;

      else
	chance += 40;
      if (number_percent () < chance)
	{
	  if (ch->in_room != victim->in_room)
	    move_char (ch, door, TRUE);
	//  if (!str_cmp (ch->in_room->area->filename, "thesden.are"))
	//    arrest (ch, victim);

	  else
	    {
	//      if (victim->race != PC_RACE_LICH)
	        sprintf (buf, "The fugitive %s is here!", victim->name);
	  //    else sprintf(buf,"Help me!  There's some hideous undead creature over here! Kill it quick!!!");
	      
	      REMOVE_BIT (ch->comm, COMM_NOSHOUT);
	      do_yell (ch, buf);
	      multi_hit (ch, victim, TYPE_UNDEFINED);
	    }
	  return TRUE;
	}
    }
  return FALSE;
}

bool spec_janitor (CHAR_DATA * ch)
{
  OBJ_DATA *trash;
  OBJ_DATA *trash_next;
  if (!IS_AWAKE (ch))
    return FALSE;
  for (trash = ch->in_room->contents; trash != NULL; trash = trash_next)
    {
      trash_next = trash->next_content;
      if (!IS_SET (trash->wear_flags, ITEM_TAKE) || !can_loot (ch, trash))
	continue;
      if (trash->item_type == ITEM_DRINK_CON
	  || trash->item_type == ITEM_TRASH || trash->cost < 10)
	{
 	  if ((trash->item_type == ITEM_FURNITURE || trash->item_type == ITEM_RAFT) &&
               count_users (trash) > 0)
            continue;
          if (trash->item_type == ITEM_CTRANSPORT)
            continue;
				   
	  act ("$n picks up some trash.", ch, NULL, NULL, TO_ROOM);
	  obj_from_room (trash);
	  obj_to_char (trash, ch);
	  return TRUE;
	}
    }
  return FALSE;
}

bool spec_mayor (CHAR_DATA * ch)
{
  static const char open_path[] =
    "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";
  static const char close_path[] =
    "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";
  static const char *path;
  static int pos;
  static bool move;
  if (!move)
    {
      if (time_info.hour == 6)
	{
	  path = open_path;
	  move = TRUE;
	  pos = 0;
	}
      if (time_info.hour == 20)
	{
	  path = close_path;
	  move = TRUE;
	  pos = 0;
	}
    }
  if (ch->fighting != NULL && ch->position >= POS_FIGHTING)
    return spec_cast_mage (ch);
  if (!move || ch->position < POS_SLEEPING)
    return FALSE;
  switch (path[pos])
    {
    case '0':
    case '1':
    case '2':
    case '3':
      move_char (ch, path[pos] - '0', FALSE);
      break;
    case 'W':
      ch->position = POS_STANDING;
      act ("$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM);
      break;
    case 'S':
      ch->position = POS_SLEEPING;
      act ("$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM);
      break;
    case 'a':
      act ("$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM);
      break;
    case 'b':
      act ("$n says 'What a view!  I must do something about that dump!'",
	   ch, NULL, NULL, TO_ROOM);
      break;
    case 'c':
      act ("$n says 'Vandals!  Youngsters have no respect for anything!'",
	   ch, NULL, NULL, TO_ROOM);
      break;
    case 'd':
      act ("$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM);
      break;
    case 'e':
      act ("$n says 'I hereby declare the city of Midgaard open!'", ch,
	   NULL, NULL, TO_ROOM);
      break;
    case 'E':
      act ("$n says 'I hereby declare the city of Midgaard closed!'",
	   ch, NULL, NULL, TO_ROOM);
      break;
    case 'O':

/*      do_unlock( ch, "gate" ); */
      do_open (ch, "gate");
      break;
    case 'C':
      do_close (ch, "gate");

/*      do_lock( ch, "gate" ); */
      break;
    case '.':
      move = FALSE;
      break;
    }
  pos++;
  return FALSE;
}

bool spec_poison (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  if (ch->position < POS_FIGHTING
      || (victim = ch->fighting) == NULL || number_percent () > 2 * ch->level)
    return FALSE;
  act ("You bite $N!", ch, NULL, victim, TO_CHAR);
  act ("$n bites $N!", ch, NULL, victim, TO_NOTVICT);
  act ("$n bites you!", ch, NULL, victim, TO_VICT);
  spell_poison (gsn_poison, ch->level, ch, victim, TARGET_CHAR);
  return TRUE;
}

bool spec_thief (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  long gold, silver;
  if (ch->position != POS_STANDING)
    return FALSE;
  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (IS_NPC (victim)
	  || victim->level >= LEVEL_IMMORTAL
	  || number_bits (5) != 0
	  || victim->level < 9 || !can_see (ch, victim))
	continue;
      if (IS_AWAKE (victim) && number_range (0, ch->level) == 0)
	{
	  act ("You discover $n's hands in your wallet!", ch, NULL,
	       victim, TO_VICT);
	  act ("$N discovers $n's hands in $S wallet!", ch, NULL,
	       victim, TO_NOTVICT);
	  return TRUE;
	}

      else
	{
	  gold =
	    victim->gold * UMIN (number_range (1, 20), ch->level / 2) / 100;
	  gold = UMIN (gold, ch->level * ch->level * 10);
	  ch->gold += gold;
	  victim->gold -= gold;
	  silver =
	    victim->silver * UMIN (number_range (1, 20), ch->level / 2) / 100;
	  silver = UMIN (silver, ch->level * ch->level * 25);
	  ch->silver += silver;
	  victim->silver -= silver;
	  return TRUE;
	}
    }
  return FALSE;
}

//Iblis - Imm command (and used by the special mobs) to make a player a lich
void do_makelich(CHAR_DATA *imm, char *argument)
{
  CHAR_DATA *ch;
  OBJ_DATA *obj,*obj_next;
  int sn=0;
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
  if (ch->level < 90)
  {
	  send_to_char ("You're not high enough level to remort.\n\r",ch);
	  return;
  }
  set_title (ch, "``the `aLich``.");
  ch->level = 1;
  ch->exp = 0;
  ch->pcdata->totalxp = 0;
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
  ch->pcdata->hp_gained = 0;
  ch->pcdata->mana_gained = 0;
  ch->pcdata->move_gained = 0;
  while (ch->affected)
    affect_remove (ch, ch->affected);

  ch->affected_by ^= ch->affected_by & race_table[ch->race].aff;
  ch->imm_flags ^= ch->imm_flags & race_table[ch->race].imm;
  ch->res_flags ^= ch->res_flags & race_table[ch->race].res;
  ch->vuln_flags ^= ch->vuln_flags & race_table[ch->race].vuln;
  ch->act ^= ch->act & race_table[ch->race].act;
  if (ch->race == PC_RACE_SWARM)
      ch->act2 ^= ch->act2 & ACT_NOMOVE;

  ch->pcdata->old_race = ch->race;
  ch->race = PC_RACE_LICH;
  
  ch->size = pc_race_table[ch->race].size;
  ch->affected_by = ch->affected_by | race_table[ch->race].aff;
  ch->imm_flags = ch->imm_flags | race_table[ch->race].imm;
  ch->res_flags = ch->res_flags | race_table[ch->race].res;
  ch->vuln_flags = ch->vuln_flags | race_table[ch->race].vuln;
  ch->act = ch->act | race_table[ch->race].act;
  if (ch->race == PC_RACE_SWARM)
       ch->act2 = ch->act2 | ACT_NOMOVE;
  ch->form = race_table[ch->race].form;
  ch->parts = race_table[ch->race].parts;
  ch->max_hit = 20;
  ch->hit = 20;
  ch->max_mana = 100;
  ch->mana = 100;
  ch->max_move = 100;
  ch->move = 100;
  ch->pcdata->perm_hit = 20;
  ch->pcdata->perm_mana = 100;
  ch->pcdata->perm_move = 100;
  ch->pcdata->condition[COND_DRUNK] = 5;
  ch->pcdata->condition[COND_FULL] = 5;
  ch->pcdata->condition[COND_THIRST] = 5;
  ch->pcdata->condition[COND_HUNGER] = 5;

  if (ch->Class == PC_CLASS_PALADIN)
    {
      char tcbuf[MAX_INPUT_LENGTH];
      DESCRIPTOR_DATA *d;
      d = ch->desc;
      ch->pcdata->br_points = -1;
      //To fix pure clerics reincarnating
      ch->pcdata->autoassist_level = AASSIST_MOBS;
      affect_strip_skills(ch);
      for (sn = 0; sn < MAX_SKILL; sn++)
      {
        ch->pcdata->learned[sn] = 0;
        ch->pcdata->mod_learned[sn] = 0;
      }
      ch->pcdata->lost_prayers = 0;		 
      send_to_desc (d, "\n\r");
      send_to_desc (d,
		    " __^__                                                            __^__\n\r");
      send_to_desc (d,
		    "( ___ )----------------------------------------------------------( ___ )\n\r");
      send_to_desc (d,
		    " | / |                                                            | \\ |\n\r");
      send_to_desc (d,
		    " | / |                Please Choose a Class:                      | \\ |\n\r");
      send_to_desc (d,
		    " | / |------------------------------------------------------------| \\ |\n\r");
      send_to_desc (d,
		    " | / |                                                            | \\ |\n\r");
      sprintf (tcbuf,
	       " | / |             %s       %s                    | \\ |\n\r",
	       IS_SET (genraces[ch->race],
		       CLASS_MAGE) ? "[`oM``]agic-User" :
	       "`aMagic-User``  ", IS_SET (genraces[ch->race],
					   CLASS_CLERIC) ?
	       "[`oC``]leric" : "`aCleric``  ");
      send_to_desc (d, tcbuf);
      sprintf (tcbuf,
	       " | / |             %s            %s                   | \\ |\n\r",
	       IS_SET (genraces[ch->race],
		       CLASS_THIEF) ? "[`oT``]hief" : "`aThief``  ",
	       IS_SET (genraces[ch->race],
		       CLASS_WARRIOR) ? "[`oW``]arrior" :
	       "`aWarrior``  ");
      send_to_desc (d, tcbuf);
      sprintf (tcbuf,
	       " | / |             %s           %s                      | \\ |\n\r",
	       IS_SET (genraces[ch->race],
		       CLASS_RANGER) ? "[`oR``]anger" :
	       "`aRanger``  ", IS_SET (genraces[ch->race],
				       CLASS_BARD) ? "[`oB``]ard" :
	       "`aBard``  ");
      send_to_desc (d, tcbuf);
      sprintf (tcbuf,
	       " | / |             %s        %s                    | \\ |\n\r",
	       IS_SET (genraces[ch->race],
		       CLASS_PALADIN) ? "[`oP``]aladin" :
	       "`aPaladin``    ", IS_SET (genraces[ch->race],
					  CLASS_ASSASSIN) ?
	       "[`oA``]ssassin" : "`aAssassin``  ");
      send_to_desc (d, tcbuf);
      sprintf (tcbuf,
	       " | / |             %s           %s                      | \\ |\n\r",
	       IS_SET (genraces[ch->race],
		       CLASS_REAVER) ? "[`oRE``]aver" :
	       "`aReaver``  ", IS_SET (genraces[ch->race],
				       CLASS_MONK) ? "[`oMO``]nk" :
	       "`aMonk``  ");
      send_to_desc (d, tcbuf);
      sprintf (tcbuf,
	      " | / |              %s                                        | \\ |\n\r",
	      IS_SET (genraces[ch->race],
		      CLASS_DRUID) ? "[`oD``]ruid " :
	      "`aDruid``  ");
      send_to_desc (d, tcbuf);
      send_to_desc (d,
		    " |___|                                                            |___|\n\r");
      send_to_desc (d,
		    "(_____)----------------------------------------------------------(_____)\n\r\n\r");
      ch->level = 0;
      d->connected = CON_GET_NEW_CLASS;
      send_to_char("Ding.  Lich done.  They are one.\n\r",imm);
      return;
    }

  group_add_all (ch);
  do_reroll(ch,"");
  send_to_char("Ding.  Lich done.  You are one.\n\r",ch);
  send_to_char("Ding.  Lich done.  They are one.\n\r",imm);
  return;
}

//Iblis - Imm command (and used by the special mobs) to make a player a Kalian
void do_makekalian(CHAR_DATA *imm, char *argument)
{
  CHAR_DATA *ch;
  OBJ_DATA *obj,*obj_next;
  ch = get_char_world(imm,argument);
  if (ch == NULL)
    {
      send_to_char("That person isn't logged in or you can't spell for shit.\n\r",imm);
      return;
    }
  if (ch->race == PC_RACE_KALIAN)
  {
	  send_to_char("You're already a Kalian.  *ERROR ERROR*\n\r",ch);
	  return;
  }
  if (ch->level < 90)
  {
	  send_to_char("You're not high enough level to remort!\n\r",ch);
	  return;
  }
  if (get_trust (ch) >= get_trust (imm))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  ch->level = 1;
  set_title (ch, "``the `bKalian``.");
  ch->exp = 0;
  ch->pcdata->totalxp = 0;
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
  ch->pcdata->hp_gained = 0;
  ch->pcdata->mana_gained = 0;
  ch->pcdata->move_gained = 0;
  while (ch->affected)
    affect_remove (ch, ch->affected);

  ch->affected_by ^= ch->affected_by & race_table[ch->race].aff;
  ch->imm_flags ^= ch->imm_flags & race_table[ch->race].imm;
  ch->res_flags ^= ch->res_flags & race_table[ch->race].res;
  ch->vuln_flags ^= ch->vuln_flags & race_table[ch->race].vuln;
  ch->act ^= ch->act & race_table[ch->race].act;
  if (ch->race == PC_RACE_SWARM)
      ch->act2 ^= ch->act2 & ACT_NOMOVE;
  

  ch->pcdata->old_race = ch->race;
  ch->race = PC_RACE_KALIAN;

  ch->size = pc_race_table[ch->race].size;
  ch->affected_by = ch->affected_by | race_table[ch->race].aff;
  ch->imm_flags = ch->imm_flags | race_table[ch->race].imm;
  ch->res_flags = ch->res_flags | race_table[ch->race].res;
  ch->vuln_flags = ch->vuln_flags | race_table[ch->race].vuln;
  ch->act = ch->act | race_table[ch->race].act;
  if (ch->race == PC_RACE_SWARM)
     ch->act2 = ch->act2 | ACT_NOMOVE;
  
  ch->form = race_table[ch->race].form;
  ch->parts = race_table[ch->race].parts;
  ch->max_hit = 20;
  ch->hit = 20;
  ch->max_mana = 100;
  ch->mana = 100;
  ch->max_move = 100;
  ch->move = 100;
  ch->pcdata->perm_hit = 20;
  ch->pcdata->perm_mana = 100;
  ch->pcdata->perm_move = 100;
  if (ch->Class == 10)
  {
    ch->max_hit += 100;
    ch->hit += 100;
    ch->max_mana += 100;
    ch->mana += 100;
    ch->max_move += 100;
    ch->move += 100;
    ch->pcdata->perm_hit += 100;
    ch->pcdata->perm_mana += 100;
    ch->pcdata->perm_move += 100;
    send_to_char("For being a Necromancer you get a *SPECIAL* bonus.\n\r",ch);
  }
  group_add_all (ch);
  do_reroll(ch,"");
  send_to_char("Ding.  Kalian done.  You are one.\n\r",ch);
  send_to_char("Ding.  Kalian done.  They are one.\n\r",imm);
  return;
}

//Imm command (and used by the special mobs) to make a player a new new style Avatar
void do_makeavatar(CHAR_DATA *imm, char *argument)
{
  CHAR_DATA *ch;
  OBJ_DATA *obj,*obj_next;
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
  if (ch->level < 90)
  {
	  send_to_char ("You're not a high enough level to remort.\n\r",ch);
	  return;
  }
  ch->level = 1;
  ch->exp = 0;
  set_title (ch, "``the `hAvatar``.");
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
  ch->pcdata->hp_gained = 0;
  ch->pcdata->mana_gained = 0;
  ch->pcdata->move_gained = 0;
  while (ch->affected)
    affect_remove (ch, ch->affected);

  ch->affected_by ^= ch->affected_by & race_table[ch->race].aff;
  ch->imm_flags ^= ch->imm_flags & race_table[ch->race].imm;
  ch->res_flags ^= ch->res_flags & race_table[ch->race].res;
  ch->vuln_flags ^= ch->vuln_flags & race_table[ch->race].vuln;
  ch->act ^= ch->act & race_table[ch->race].act;
  if (ch->race == PC_RACE_SWARM)
      ch->act2 ^= ch->act2 & ACT_NOMOVE;
  

  ch->pcdata->old_race = ch->race;
  if (ch->pcdata->loner || ch->clan != CLAN_BOGUS)
    ch->pcdata->avatar_type = 4;
  else ch->pcdata->avatar_type = 3;
  ch->race = PC_RACE_AVATAR;

  ch->size = pc_race_table[ch->race].size;
  ch->affected_by = ch->affected_by | race_table[ch->race].aff;
  ch->imm_flags = ch->imm_flags | race_table[ch->race].imm;
  ch->res_flags = ch->res_flags | race_table[ch->race].res;
  ch->vuln_flags = ch->vuln_flags | race_table[ch->race].vuln;
  ch->act = ch->act | race_table[ch->race].act;
  if (ch->race == PC_RACE_SWARM)
     ch->act2 = ch->act2 | ACT_NOMOVE;
  
  ch->form = race_table[ch->race].form;
  ch->parts = race_table[ch->race].parts;
  ch->max_hit = 20;
  ch->hit = 20;
  ch->max_mana = 100;
  ch->mana = 100;
  ch->max_move = 100;
  ch->move = 100;
  ch->pcdata->perm_hit = 20;
  ch->pcdata->perm_mana = 100;
  ch->pcdata->perm_move = 100;
  group_add_all (ch);
  do_reroll(ch,"");
  send_to_char("Ding.  Avatar done.  You are one.\n\r",ch);
  send_to_char("Ding.  Avatar done.  They are one.\n\r",imm);
  return;
}

//Iblis - a function used to reset a players race (Basically the only place I believe this is used
//anymore is when a new new avatar dies)
void reset_race(CHAR_DATA* ch)
{
  OBJ_DATA *obj,*obj_next;
  int sn=0;
  if (ch->level < 41)
    ch->pcdata->totalxp = ch->pcdata->totalxp - (ch->pcdata->totalxp * ((41.0-ch->level)/100) );
  ch->exp = ch->pcdata->totalxp;
  ch->level = 1;
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

  for (sn=0;sn < MAX_SKILL;sn++)
  {
	ch->pcdata->learned[sn] = 0;
  }
  
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
  ch->pcdata->hp_gained = 0;
  ch->pcdata->mana_gained = 0;
  ch->pcdata->move_gained = 0;
  while (ch->affected)
    affect_remove (ch, ch->affected);

  ch->affected_by ^= ch->affected_by & race_table[ch->race].aff;
  ch->imm_flags ^= ch->imm_flags & race_table[ch->race].imm;
  ch->res_flags ^= ch->res_flags & race_table[ch->race].res;
  ch->vuln_flags ^= ch->vuln_flags & race_table[ch->race].vuln;
  ch->act ^= ch->act & race_table[ch->race].act;
  if (ch->race == PC_RACE_SWARM)
     ch->act2 ^= ch->act2 & ACT_NOMOVE;
  

  ch->race = ch->pcdata->old_race;

  ch->size = pc_race_table[ch->race].size;
  ch->affected_by = ch->affected_by | race_table[ch->race].aff;
  ch->imm_flags = ch->imm_flags | race_table[ch->race].imm;
  ch->res_flags = ch->res_flags | race_table[ch->race].res;
  ch->vuln_flags = ch->vuln_flags | race_table[ch->race].vuln;
  ch->act = ch->act | race_table[ch->race].act;
  if (ch->race == PC_RACE_SWARM)
     ch->act2 = ch->act2 | ACT_NOMOVE;
  
  ch->form = race_table[ch->race].form;
  ch->parts = race_table[ch->race].parts;
  ch->max_hit = 20;
  ch->hit = 20;
  ch->max_mana = 100;
  ch->mana = 100;
  ch->max_move = 100;
  ch->move = 100;
  ch->pcdata->perm_hit = 20;
  ch->pcdata->perm_mana = 100;
  ch->pcdata->perm_move = 100;
  group_add_all (ch);
  do_reroll(ch,"");
  return;
}

//Imm command (and used by the special mobs) to make a player a nerix
void do_makenerix(CHAR_DATA *imm, char* argument)
{
  CHAR_DATA *ch;
  OBJ_DATA *obj,*obj_next, *obj3;
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
  if (ch->alignment < 800)
  {
    send_to_char("You are not angelic enough to remort into Nerix.\n\r",ch);
    return;
  }
	  
  if (((ch->played + (int) (current_time - ch->logon)) / 3600) < 30)
  {
    send_to_char("You are too young to remort into Nerix.\n\r",ch);
    return;
  }
  if (ch->level < 90)
  {
    send_to_char("You're not high enough level to remort.\n\r",ch);
    return;
  }
  if ((ch->Class != 6 && ch->Class != 5 && ch->Class != 1 && ch->Class != 3)
     || (ch->Class2 != 6 && ch->Class2 != 5 && ch->Class2 != 1 && ch->Class2 != 3)
     || (ch->Class3 != 6 && ch->Class3 != 5 && ch->Class3 != 1 && ch->Class3 != 3))
  {
    send_to_char("You may not remort to Nerix if you are anything but some combination of Bard, Cleric, Warrior, and Paladin.\n\r",ch);
    return;
  }
  if (((obj = get_obj_vnum_char(ch,OBJ_VNUM_NERIX1)) == NULL)
		  || ((obj_next = get_obj_vnum_char(ch,OBJ_VNUM_NERIX2)) == NULL)
		  || ((obj3 = get_obj_vnum_char(ch,OBJ_VNUM_NERIX3)) == NULL))
  {
	  send_to_char("You do not have all of the desired objects.\n\r",ch);
	  return;
  }
  extract_obj(obj);
  extract_obj(obj_next);
  extract_obj(obj3);
	  
  ch->level = 1;
  ch->exp = 0;
  ch->pcdata->totalxp = 0;
  set_title (ch, "``the `oNerix``.");
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
  ch->pcdata->hp_gained = 0;
  ch->pcdata->mana_gained = 0;
  ch->pcdata->move_gained = 0;
  while (ch->affected)
    affect_remove (ch, ch->affected);

  ch->affected_by ^= ch->affected_by & race_table[ch->race].aff;
  ch->imm_flags ^= ch->imm_flags & race_table[ch->race].imm;
  ch->res_flags ^= ch->res_flags & race_table[ch->race].res;
  ch->vuln_flags ^= ch->vuln_flags & race_table[ch->race].vuln;
  ch->act ^= ch->act & race_table[ch->race].act;
  if (ch->race == PC_RACE_SWARM)
    ch->act2 ^= ch->act2 & ACT_NOMOVE;
  

  ch->pcdata->old_race = ch->race;
  ch->race = PC_RACE_NERIX;

  ch->size = pc_race_table[ch->race].size;
  ch->affected_by = ch->affected_by | race_table[ch->race].aff;
  ch->imm_flags = ch->imm_flags | race_table[ch->race].imm;
  ch->res_flags = ch->res_flags | race_table[ch->race].res;
  ch->vuln_flags = ch->vuln_flags | race_table[ch->race].vuln;
  ch->act = ch->act | race_table[ch->race].act;
  if (ch->race == PC_RACE_SWARM)
     ch->act2 = ch->act2 | ACT_NOMOVE;
  ch->form = race_table[ch->race].form;
  ch->parts = race_table[ch->race].parts;
  ch->max_hit = 100;
  ch->hit = 100;
  ch->max_mana = 200;
  ch->mana = 200;
  ch->max_move = 100;
  ch->move = 100;
  ch->pcdata->perm_hit = 100;
  ch->pcdata->perm_mana = 200;
  ch->pcdata->perm_move = 100;
  ch->alignment = 1000;
  ch->pcdata->nage = 1;
  ch->pcdata->nplayed = ch->played + (int) (current_time - ch->logon);
  ch->dam_type = 41; /*shocking claw*/
  group_add_all (ch);
  do_reroll(ch,"");
  send_to_char("Ding.  Nerix done.  You are one.\n\r",ch);
  send_to_char("Ding.  Nerix done.  They are one.\n\r",imm);
  return;
}

//Imm command (and used by the special mobs) to make a player a swarm
void do_makeswarm(CHAR_DATA *imm, char* argument)
{
  CHAR_DATA *ch;
  OBJ_DATA *obj,*obj_next;
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
  ch->level = 1;
  ch->exp = 0;
  ch->pcdata->totalxp = 0;
  set_title (ch, "``the `jSwarm!``.");
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
  ch->pcdata->hp_gained = 0;
  ch->pcdata->mana_gained = 0;
  ch->pcdata->move_gained = 0;
  while (ch->affected)
    affect_remove (ch, ch->affected);
  ch->affected_by ^= ch->affected_by & race_table[ch->race].aff;
  ch->imm_flags ^= ch->imm_flags & race_table[ch->race].imm;
  ch->res_flags ^= ch->res_flags & race_table[ch->race].res;
  ch->vuln_flags ^= ch->vuln_flags & race_table[ch->race].vuln;
  ch->act ^= ch->act & race_table[ch->race].act;
  if (ch->race == PC_RACE_SWARM)
    ch->act2 ^= ch->act2 & ACT_NOMOVE;


  ch->pcdata->old_race = ch->race;
  ch->race = PC_RACE_SWARM;

  ch->size = pc_race_table[ch->race].size;
  ch->affected_by = ch->affected_by | race_table[ch->race].aff;
  ch->imm_flags = ch->imm_flags | race_table[ch->race].imm;
  ch->res_flags = ch->res_flags | race_table[ch->race].res;
  ch->vuln_flags = ch->vuln_flags | race_table[ch->race].vuln;
  ch->act = ch->act | race_table[ch->race].act;
  if (ch->race == PC_RACE_SWARM)
    ch->act2 = ch->act2 | ACT_NOMOVE;
  ch->form = race_table[ch->race].form;
  ch->parts = race_table[ch->race].parts;
  ch->max_hit = 10;
  ch->hit = 10;
  ch->max_mana = 100;
  ch->mana = 100;
  ch->max_move = 100;
  ch->move = 100;
  ch->pcdata->perm_hit = 10;
  ch->pcdata->perm_mana = 100;
  ch->pcdata->perm_move = 100;
  group_add_all (ch);
  /* SET THEIR GODDAMN STATS HERE */
  send_to_char("Ding.  Swarm done.  You are one.\n\r",ch);
  send_to_char("Ding.  Swarm done.  They are one.\n\r",imm);
  return;
}

//Imm command (and used by the special mobs) to make a player a Nidae
void do_makenidae(CHAR_DATA *imm, char* argument)
{
  CHAR_DATA *ch;
  OBJ_DATA *obj,*obj_next;
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
  ch->level = 1;
  ch->exp = 0;
  ch->pcdata->totalxp = 0;
  set_title (ch, "``the `lNidae``.");
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
  ch->pcdata->hp_gained = 0;
  ch->pcdata->mana_gained = 0;
  ch->pcdata->move_gained = 0;
  while (ch->affected)
    affect_remove (ch, ch->affected);
  ch->affected_by ^= ch->affected_by & race_table[ch->race].aff;
  ch->imm_flags ^= ch->imm_flags & race_table[ch->race].imm;
  ch->res_flags ^= ch->res_flags & race_table[ch->race].res;
  ch->vuln_flags ^= ch->vuln_flags & race_table[ch->race].vuln;
  ch->act ^= ch->act & race_table[ch->race].act;
  if (ch->race == PC_RACE_SWARM)
    ch->act2 ^= ch->act2 & ACT_NOMOVE;


  ch->pcdata->old_race = ch->race;
  ch->race = PC_RACE_NIDAE;

  ch->size = pc_race_table[ch->race].size;
  ch->affected_by = ch->affected_by | race_table[ch->race].aff;
  ch->imm_flags = ch->imm_flags | race_table[ch->race].imm;
  ch->res_flags = ch->res_flags | race_table[ch->race].res;
  ch->vuln_flags = ch->vuln_flags | race_table[ch->race].vuln;
  ch->act = ch->act | race_table[ch->race].act;
  if (ch->race == PC_RACE_SWARM)
    ch->act2 = ch->act2 | ACT_NOMOVE;
  ch->form = race_table[ch->race].form;
  ch->parts = race_table[ch->race].parts;
  ch->max_hit = 20;
  ch->hit = 20;
  ch->max_mana = 100;
  ch->mana = 100;
  ch->max_move = 100;
  ch->move = 100;
  ch->pcdata->perm_hit = 20;
  ch->pcdata->perm_mana = 100;
  ch->pcdata->perm_move = 100;
  group_add_all (ch);
  do_reroll(ch,"");
  send_to_char("Ding.  Nidae done.  You are one.\n\r",ch);
  send_to_char("Ding.  Nidae done.  They are one.\n\r",imm);
  return;
}


void do_makecj(CHAR_DATA *imm, char *argument)
{
  CHAR_DATA *ch;
  OBJ_DATA *obj,*obj_next;//, *obj3;
  char buf[MAX_STRING_LENGTH];
  short Class=0,i=0;
  ch = get_char_world(imm,argument);
  if (ch == NULL)
    {
      send_to_char("That person isn't logged in or you can't spell for shit.\n\r",imm);
      return;
    }
  if (ch->race == PC_RACE_AVATAR || ch->race == PC_RACE_NERIX)
  {
    send_to_char("I don't think so.\n\r",imm);
    send_to_char("You're not crazy enough to become a chaos jester.\n\r",ch);
    return;
  }
  if (get_trust (ch) >= get_trust (imm))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  if (IS_NPC(ch))
    return;
  if (ch->level < 90)
    {
      send_to_char("You are not high enough level to remort.\n\r",ch);
      return;
    }

  /*if (((obj = get_obj_vnum_char(ch,OBJ_VNUM_NECRO1)) == NULL)
      || ((obj_next = get_obj_vnum_char(ch,OBJ_VNUM_NECRO2)) == NULL)
      || ((obj3 = get_obj_vnum_char(ch,OBJ_VNUM_NECRO3)) == NULL))
    {
      send_to_char("You do not have all of the desired objects.\n\r",ch);
      return;
    }
  extract_obj(obj);
  extract_obj(obj_next);
  extract_obj(obj3);*/

/*  if (ch->Class != 0)
    ch->pcdata->old_Class = ch->Class;
  else if (ch->Class2 != 0)
    ch->pcdata->old_Class = ch->Class2;
  else ch->pcdata->old_Class = ch->Class3;*/
  sprintf(buf,"Laugh your ass off at %s, who is now a Chaos Jester.",ch->name);
  wiznet (buf, ch, NULL, WIZ_SECURE, 0, 0);
  ch->level = 1;
  set_title (ch, "``the `fChaos Jester``.");
  ch->exp = 0;
  ch->pcdata->totalxp = 0;
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
  ch->pcdata->learned[gsn_sword] = 1;
  ch->pcdata->learned[gsn_dagger] = 1;
  ch->pcdata->learned[gsn_staff] = 1;
  ch->pcdata->learned[gsn_mace] = 1;
  ch->pcdata->learned[gsn_axe] = 1;
  ch->pcdata->learned[gsn_flail] = 1;
  ch->pcdata->learned[gsn_polearm] = 1;

  for (i = 0; i < 3; ++i)
  {
    if (i == 0)
      Class = ch->Class;
    else if (i == 1)
      Class = ch->Class2;
    else Class = ch->Class3;
    switch (Class)
      {
      case PC_CLASS_MAGE:
      case PC_CLASS_NECROMANCER: 
	ch->pcdata->perm_mana += 33; 
	ch->max_mana += 33;
	ch->mana += 33;
	break;
      case PC_CLASS_MONK:
      case PC_CLASS_WARRIOR:
      case PC_CLASS_REAVER:
	ch->pcdata->perm_hit += 16;
	ch->max_hit += 16;
	ch->hit += 16;
	break;
      case PC_CLASS_PALADIN:
      case PC_CLASS_RANGER:
	ch->pcdata->perm_hit += 15;
        ch->max_hit += 15;
        ch->hit += 15;
	ch->pcdata->perm_mana += 3;
        ch->max_mana += 3;
        ch->mana += 3;
	break;
      case PC_CLASS_BARD:
      case PC_CLASS_CLERIC:
	ch->pcdata->perm_hit += 7;
        ch->max_hit += 7;
        ch->hit += 7;
        ch->pcdata->perm_mana += 7;
        ch->max_mana += 7;
        ch->mana += 7;
        break;
      case PC_CLASS_THIEF:
      case PC_CLASS_ASSASSIN:
	ch->pcdata->perm_hit += 8;
        ch->max_hit += 8;
        ch->hit += 8;
	ch->pcdata->perm_move += 33;
	ch->max_move += 33;
	ch->move += 33;
	break;
      }
  }
  ch->Class = PC_CLASS_CHAOS_JESTER;
  ch->Class2 = PC_CLASS_CHAOS_JESTER;
  ch->Class3 = PC_CLASS_CHAOS_JESTER;
  group_add_all (ch);
  do_reroll(ch,"");
  SET_BIT(ch->act,PLR_SWITCHOK);
  ch->pcdata->familiar_type = FAMILIAR_IMAGINARY_FRIEND;
  if (ch->pcdata->loner == FALSE)
  {
    ch->pcdata->loner = TRUE;
//    ch->pcdata->deputy = FALSE;
    send_to_char("You are now graced with the honor of being pk.\n\r",ch);
    return;
  }
  char_from_room(ch);
  char_to_room(ch,get_room_index(ROOM_VNUM_ALTAR));
  send_to_char("While you are level 1, you may change your imaginary friend's name.\n\r",ch);
  send_to_char("This name is very important as it is used FOREVER.\n\r",ch);
  send_to_char("You do not have to give your friend a name if you do not want to, but it is\n\r",ch);
  send_to_char("HIGHLY SUGGESTED that you do so.\n\r",ch);
  send_to_char("friendname NAME    is a usable command, but only while level 1.\n\r",ch);
  return;
}

void do_friendname(CHAR_DATA *ch, char *argument)
{
  char arg1[MAX_STRING_LENGTH];
  one_argument(argument,arg1);
  if ((IS_NPC(ch) || ch->Class != PC_CLASS_CHAOS_JESTER || ch->level != 1) && !IS_IMMORTAL(ch))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (arg1[0] == '\0')
  {
    send_to_char("Syntax: friendname NAME\n\r",ch);
    return;
  }
  if (arg1[0] != '\0')
    {
      char strsave[MAX_STRING_LENGTH];
      struct stat statBuf;
      sprintf (strsave, "%s%c/%s", PLAYER_DIR, toupper (arg1[0]), capitalize (arg1));
      if (!check_parse_name (arg1) || stat (strsave, &statBuf) != -1)
	{
	  send_to_char ("Sorry, that name cannot be used for your imaginary friend.\n\r", ch);
	  return;
	}
      free_string(ch->pcdata->familiar_name);
      ch->pcdata->familiar_name = str_dup(capitalize(arg1));
      send_to_char (ch->pcdata->familiar_name,ch);
      send_to_char (" set as the name of your imaginary friend.\n\r",ch);
    }
}

void do_makenewbclannie(CHAR_DATA *imm, char* argument)
{
  CHAR_DATA *ch;
  //  OBJ_DATA *obj,*obj_next;//, *obj3;
  char arg1[MAX_STRING_LENGTH];
  //  short Class=0,i=0;
  int slot=0;
  MEMBER *newm;
  MEMBER *oldm;
  argument = one_argument(argument,arg1);
  ch = get_char_world(imm,arg1);
  if (ch == NULL)
    {
      send_to_char("That person isn't logged in or you can't spell for shit.\n\r",imm);
      return;
    }
  if (get_trust (ch) >= get_trust (imm))
    {
      send_to_char ("You failed.\n\r", imm);
      return;
    }
  if (IS_NPC(ch))
    return;
  if (ch->level >= 45)
    {
      send_to_char("They are too high to join the newb clan.\n\r",imm);
      send_to_char("You are too high to join the newb clan.\n\r",ch);
      return;
    }
  if (ch->level < 10)
    {
      send_to_char("They are too young to join the newb clan.\n\r",imm);
      send_to_char("You are too young to join the newb clan.\n\r",ch);
      return;
    }

  argument = "Guttersnipes";
  slot = clanname_to_slot (argument);
  
  
  // validate the slot
  if (slot == CLAN_BOGUS)
    {
      send_to_char ("You must be in a clan or name a valid clan.\n\r", ch);
      return;
    }
  
  //  vic = find_character (argument);
  // copy from the old member rec to the new one
  // unfortunate side effect of the way I coded the
  // the member list insert, delete and find functions
  newm = new_member_elt ();
  strcpy (newm->name, ch->name);
  newm->levels = ch->level;
  newm->align = ch->alignment;
  newm->status = CLAN_ACCEPTED;
  newm->initiative = CLAN_BOGUS;

  //  vic = find_character (argument);
  if (ch != NULL)
    {
      if (clan_table[slot].ctype == CLAN_TYPE_EXPL)
        ch->pcdata->loner = FALSE;

      else
        ch->pcdata->loner = TRUE;
      ch->pcdata->deputy = FALSE;
      ch->clan = slot;
      save_char_obj (ch);
      sprintf (arg1, "You have been accepted into clan %s``.\n\r",
               clan_table[slot].name);
      send_to_char (arg1, ch);
    }

  // finally delete the oldm record
  oldm = find_member (clan_table[slot].applicants, ch->name);
  if (oldm)
  {
    clan_table[slot].applicants = delete_member (clan_table[slot].applicants, ch->name);
  }
  oldm = NULL;

  // notify membership
  sprintf (arg1, "%s has been accepted into clan %s``.\n\r",
           capitalize (ch->name), clan_table[slot].name);
  send_clan_members_ch (ch, arg1);

  // finally delete the oldm record
  clan_table[slot].applicants =
    delete_member (clan_table[slot].applicants, argument);
  oldm = NULL;

  // append the new member record
  clan_table[slot].members = append_member (clan_table[slot].members, newm);

  // now check if the addition of this member changes the clan status
  clan_table[slot].levels += newm->levels;
  /*  if (//((clan_table[slot].status == CLAN_PROPOSED) ||
       (clan_table[slot].status == CLAN_RESTRICTED)//)
      && clan_table[slot].levels >= FULL_CLAN_LEVELS)
    {
      clan_table[slot].status = CLAN_ACTIVE;
      sprintf (buf, "Clan %s`` has been promoted to a full clan.\n\r",
               clan_table[slot].name);
      send_clan_members (slot, buf);
      }*/
  // save all of these updates
  save_clan (slot);
    sprintf (arg1, "You accepted %s into clan %s``.\n\r", capitalize (ch->name),
           clan_table[slot].name);
  send_to_char (arg1, imm);

#ifdef VERBOSE_CLANS
  log_player_command ("f:clan_accept", ch, argument);

#endif /*  */
}

bool spec_shopkeeper_talker (CHAR_DATA * ch)
{
  //  CHAR_DATA *vch;
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];
  int total=0,counter=0;
  if (number_percent() < 25 && ch->carrying)
    {
      for (obj = ch->carrying;obj != NULL;obj = obj->next_content)
	++total;
      total = number_range(1,total);
      for (obj = ch->carrying;obj != NULL;obj = obj->next_content)
	if (++counter == total)
	  break;
      switch (number_range(1,4))
	{
	  case 1: sprintf(buf,"`oI have ``%s`o for sale!",obj->short_descr);
	  case 2: sprintf(buf,"`oI am currently selling ``%s`o, and I KNOW you want to buy it.",obj->short_descr);break;
	  case 3: sprintf(buf,"`oI have ``%s `oin stock and it's priced to move!",obj->short_descr);break;
	  default: sprintf(buf,"%s `ocan be found in my wonderful store.  Won't you please buy one?",obj->short_descr);break;
	}
      do_say(ch,buf);
      return TRUE;
    }
  return FALSE;
      
}
