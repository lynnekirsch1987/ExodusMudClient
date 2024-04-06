
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "merc.h"
#include "tables.h"
#include "interp.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"
#include "obj_trig.h"

int check_entangle args ((CHAR_DATA * ch));
bool room_Class_check args ((CHAR_DATA * ch, ROOM_INDEX_DATA * location));
bool room_race_check args((CHAR_DATA * ch, ROOM_INDEX_DATA * to_room));
bool br_vnum_check args ((int vnum));
bool can_move_char args ((CHAR_DATA * ch, ROOM_INDEX_DATA * to_room, bool follow, bool show_messages));
bool can_move_char_door args((CHAR_DATA * ch, int door, bool follow, bool show_messages));
bool can_move_char_door_new args((CHAR_DATA * ch, int door, bool follow, bool show_messages, int position));
OBJ_DATA *get_obj_vnum_room args((ROOM_INDEX_DATA* rid, int vnum));
void call_storm args((CHAR_DATA *ch, char *argument));
	

//bool br_vnum_check args((int vnum));
void falcon_alert args ((CHAR_DATA * ch));
void check_skeletal_mutation args((CHAR_DATA* ch, bool on_round));
bool check_clasp args((CHAR_DATA *ch));
extern bool battle_royale;
extern bool no_check_skeletal;

/* command procedures needed */
DECLARE_DO_FUN (do_look);
DECLARE_DO_FUN (do_recall);
DECLARE_DO_FUN (do_stand);
bool move_fast = FALSE;
bool is_sliding = FALSE;
const char * dir_name[] = { "north", "east", "south", "west", "up", "down" };
const sh_int rev_dir[] = { 2, 3, 0, 1, 5, 4 };
const sh_int movement_loss[SECT_MAX] = { 1, 1, 2, 3, 4, 6, 2, 4, 6, 8, 6, 5, 2, 6, 6, 4, 5 };
int find_door args ((CHAR_DATA * ch, char *arg));
long int level_cost args ((int level));
extern MEMORY_DATA *memories;

// Minax 7-13-02 Added a cook command to cook food over a campfire.
void do_cook (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *food;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("What do you want to cook?\n\r", ch);
      return;
    }
  if ((food = get_obj_carry (ch, arg)) == NULL)
    {
      send_to_char ("You do not have that item.\n\r", ch);
      return;
    }
  if (food->item_type != ITEM_FOOD)
    {
      send_to_char ("You can only cook food.\n\r", ch);
      return;
    }
  if (food->condition == 69)
    {
      send_to_char ("It's already cooked.\n\r", ch);
      return;
    }
  for (obj = ch->in_room->contents; obj; obj = obj->next_content)
    if (obj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE1
	|| obj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE2
	|| obj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE3)
      {
	act ("$n cooks $p over $P.", ch, food, obj, TO_ROOM);
	act ("You cook $p over $P.", ch, food, obj, TO_CHAR);
	food->condition = 69;
	food->value[0] *= 2;	/* doubles fullness benefit */
	food->value[1] *= 2;	/* doubles hunger benefit */
	food->value[2] *= 2;	/* doubles the time till spoilage */
	food->value[3] = 0;	// removes the poison
	return;
      }
  send_to_char ("You can't cook without a campfire.\n\r", ch);
  return;
}


// Minax 7-24-02 New skill - hone
void do_hone (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int hone_skill, percent, damage;
  OBJ_DATA *obj;
  AFFECT_DATA af, *paf, *paf_next;
  argument = one_argument (argument, arg);
  hone_skill = get_skill (ch, gsn_hone);
  if (hone_skill < 1)
    {
      send_to_char ("You wouldn't even know where to start.\n\r", ch);
      return;
    }
  if (arg[0] == '\0')
    {
      send_to_char ("What do you want to hone?\n\r", ch);
      return;
    }
  if ((obj = get_obj_carry (ch, arg)) == NULL)
    {
      send_to_char ("You do not have that item.\n\r", ch);
      return;
    }
  if ((obj->value[0] != WEAPON_SWORD) &&
      (obj->value[0] != WEAPON_AXE) &&
      (obj->value[0] != WEAPON_DAGGER) &&
      (obj->value[0] != WEAPON_POLEARM) && (obj->value[0] != WEAPON_ARROW))
    {
      act ("You cannot sharpen $p.", ch, obj, NULL, TO_CHAR);
      return;
    }
  if (IS_WEAPON_STAT (obj, WEAPON_SHARP))
    {
      act ("$p is as sharp as it can get.", ch, obj, NULL, TO_CHAR);
      return;
    }
  if (obj->condition == 0)
    obj->condition = 100;
  damage = (obj->condition - (125 - hone_skill));
  if (damage < 0)
    {
      act ("$n destroys $p in the process of trying to sharpen it.", ch,
	   obj, NULL, TO_ROOM);
      act ("$p cannot take the stress of sharpening and breaks.", ch,
	   obj, NULL, TO_CHAR);
      extract_obj (obj);
      return;
    }
// For Warriors
  if (IS_CLASS (ch, PC_CLASS_WARRIOR))
    {
      if ((str_cmp (obj->material, "steel") == 0) ||
	  (str_cmp (obj->material, "iron") == 0) ||
	  (str_cmp (obj->material, "metal") == 0) ||
	  (str_cmp (obj->material, "adamantite") == 0) ||
	  (str_cmp (obj->material, "platinum") == 0) ||
	  (str_cmp (obj->material, "silver") == 0) ||
	  (str_cmp (obj->material, "bronze") == 0) ||
	  (str_cmp (obj->material, "brass") == 0) ||
	  (str_cmp (obj->material, "copper") == 0))
	{
	  OBJ_DATA *grinder;
	  for (grinder = ch->in_room->contents; grinder;
	       grinder = grinder->next_content)
	    if (grinder->pIndexData->vnum == OBJ_VNUM_GRINDER)
	      {
		percent = number_percent ();
		if (percent < hone_skill)
		  {
		    af.where = TO_WEAPON;
		    af.type = gsn_hone;
		    af.level = ch->level;
		    af.duration = ch->level;
		    af.location = 0;
		    af.modifier = 0;
		    af.bitvector = WEAPON_SHARP;
		    af.permaff = FALSE;
		    af.composition = FALSE;
		    af.comp_name = str_dup ("");
		    affect_to_obj (obj, &af);
		    act ("$n grinds $p to a razor edge.", ch, obj,
			 NULL, TO_ROOM);
		    act ("You grind $p to a razor edge.", ch, obj,
			 NULL, TO_CHAR);
		    check_improve (ch, gsn_hone, TRUE, 3);
		    WAIT_STATE (ch, skill_table[gsn_hone].beats);
		    obj->condition -= (125 - hone_skill);
		    return;
		  }

		else
		  {
		    act
		      ("$n holds $p to the grinder too long, causing damage.",
		       ch, obj, NULL, TO_ROOM);
		    act
		      ("You hold $p to the grinder too long, causing damage.",
		       ch, obj, NULL, TO_CHAR);
		    obj->condition -= (135 - hone_skill);
		    WAIT_STATE (ch, skill_table[gsn_hone].beats);
		    return;
		  }
	      }
	  send_to_char
	    ("This task requires the use of a grinding stone.\n\r", ch);
	  return;
	}
      send_to_char ("This weapon's material is unfamiliar to you.\n\r", ch);
      return;
    }
//  For Rangers
  if (IS_CLASS (ch, PC_CLASS_RANGER))
    {
      if (str_cmp (obj->material, "wood") == 0)
	{
	  OBJ_DATA *fire;
	  for (fire = ch->in_room->contents; fire; fire = fire->next_content)
	    if (fire->pIndexData->vnum == OBJ_VNUM_CAMPFIRE1
		|| fire->pIndexData->vnum == OBJ_VNUM_CAMPFIRE2
		|| fire->pIndexData->vnum == OBJ_VNUM_CAMPFIRE3)
	      {
		percent = number_percent ();
		if (obj->value[4] & WEAPON_POISON)
		  {
		    obj->value[4] = obj->value[4] & !WEAPON_POISON;
		    for (paf = obj->affected; paf != NULL; paf = paf_next)
		      {
			paf_next = paf->next;
			if (paf->type == gsn_poison)
			  affect_remove_obj (obj, paf);
		      }
		  }
		if (percent < hone_skill)
		  {
		    af.where = TO_WEAPON;
		    af.type = gsn_hone;
		    af.level = ch->level;
		    af.duration = ch->level;
		    af.location = 0;
		    af.modifier = 0;
		    af.bitvector = WEAPON_SHARP;
		    af.permaff = FALSE;
		    af.composition = FALSE;
		    af.comp_name = str_dup ("");
		    affect_to_obj (obj, &af);
		    act ("$n hardens $p over $P.", ch, obj, fire, TO_ROOM);
		    act ("You harden $p over $P.", ch, obj, fire, TO_CHAR);
		    check_improve (ch, gsn_hone, TRUE, 3);
		    WAIT_STATE (ch, skill_table[gsn_hone].beats);
		    obj->condition -= (125 - hone_skill);
		    return;
		  }

		else
		  {
		    act ("$n holds $p over $P too long.", ch, obj,
			 fire, TO_ROOM);
		    act ("You hold $p over $P too long.", ch, obj,
			 fire, TO_CHAR);
		    obj->condition -= (135 - hone_skill);
		    WAIT_STATE (ch, skill_table[gsn_hone].beats);
		    if (number_percent () < 21)
		      {
			af.where = TO_WEAPON;
			af.type = gsn_hone;
			af.level = ch->level;
			af.duration = ch->level;
			af.location = 0;
			af.modifier = 0;
			af.bitvector = WEAPON_FLAMING;
			af.permaff = FALSE;
			af.composition = FALSE;
			af.comp_name = str_dup ("");
			affect_to_obj (obj, &af);
			act ("$p gets hot and catches fire!", ch, obj,
			     NULL, TO_ROOM);
			act ("$p gets hot and catches fire!", ch, obj,
			     NULL, TO_CHAR);
			obj->timer = 3;
			return;
		      }
		    return;
		  }
	      }
	  send_to_char
	    ("You need a campfire to harden the weapon over.\n\r", ch);
	  return;
	}
      send_to_char ("Rangers may only hone wooden weapons.\n\r", ch);
      return;
    }
  send_to_char ("You can't hone sh*t.\n\r", ch);
}

void do_camp (CHAR_DATA * ch, char *argument)
{
  int camp_skill;
  OBJ_DATA *obj;
  camp_skill = get_skill (ch, gsn_camping);
  if (camp_skill < 1)
    {
      send_to_char ("You don't know anything about camping.\n\r", ch);
      return;
    }
  if (ch->move < 50)
    {
      send_to_char ("You are too tired to create a campfire.\n\r", ch);
      return;
    }
  if (ch->in_room->sector_type != SECT_HILLS
      && ch->in_room->sector_type != SECT_MOUNTAIN
      && ch->in_room->sector_type != SECT_DESERT
      && ch->in_room->sector_type != SECT_FIELD
      && ch->in_room->sector_type != SECT_FOREST
      && ch->in_room->sector_type != SECT_GRASSLAND)
    {
      send_to_char ("You cannot create a campfire here.\n\r", ch);
      return;
    }
  if (weather_info.sky > SKY_RAINING)
    {
      send_to_char
	("It's raining too hard to start a campfire right now.\n\r", ch);
      return;
    }
  for (obj = ch->in_room->contents; obj; obj = obj->next_content)
    if (obj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE1
	|| obj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE2
	|| obj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE3)
      {
	send_to_char ("A campfire is already burning here.\n\r", ch);
	return;
      }
  if (camp_skill < number_percent ())
    {
      send_to_char
	("A combination of wind and damp wood hinders you as you fail at your attempt to start a fire.\n\r",
	 ch);
      act
	("A combination of wind and damp wood hinders $n as $e fails at $s attempt to start a fire.",
	 ch, NULL, NULL, TO_ROOM);
      check_improve (ch, gsn_camping, FALSE, 4);
      return;
    }
  ch->move /= 2;
  if (ch->move < 0)
    ch->move = 0;
  obj = create_object (get_obj_index (OBJ_VNUM_CAMPFIRE1), 0);
  obj->level = ch->level;
  obj->timer = ch->level / 8;
  if (obj->timer < 1)
    obj->timer = 1;
  obj_to_room (obj, ch->in_room);
  send_to_char
    ("Covering a few heavy logs with brittle pieces of kindling, you strike up a radiant campfire.\n\r",
     ch);
  act
    ("Covering a few heavy logs with brittle pieces of kindling, $n strikes up a radiant campfire.",
     ch, NULL, NULL, TO_ROOM);
  check_improve (ch, gsn_camping, TRUE, 4);
}


/* Set a single bit in the array */
void setbit (char *explored, int index)
{

  /* 1 << number = two to the power of number */
  explored[index / 8] |= 1 << (index % 8);
  return;
}

void unsetbit (char *explored, int index)
{

  /* 1 << number = two to the power of number */
  explored[index / 8] &= ~(1 << (index % 8));
  return;
}


/* Get a single bit from the array */
int getbit (char *explored, int index)
{

  /* IS_SET doesn't just return zero or one, so the != 0 is needed */
  return (IS_SET (explored[index / 8], 1 << (index % 8)) != 0);
}


/* Count the number of set bits in a byte */
int bitcount (char ch)
{
  int bit, count = 0;

  /* Search bits from highest to lowest */
  for (bit = 1 << 7; bit > 0; bit >>= 1)
    {
      if (ch & bit)
	count++;
    }
  return count;
}


/* Count the total number of rooms visited */
int roomcount (CHAR_DATA * ch)
{
  int index = 0, count = 0;

  /* NPCs don't have this */
  if (IS_NPC (ch))
    return 0;

  /* MAX_EXPLORE is 8192 */
  for (index = 0; index < MAX_EXPLORE; index++)
    {
      count += bitcount (ch->pcdata->explored[index]);
    }
  return count;
}


/* Count rooms visited in current area */
int areacount (CHAR_DATA * ch)
{
  int index = 0, count = 0;
  if (IS_NPC (ch))
    return 0;

  /* If you aren't in an area, you can't use this */
  if ((ch->in_room == NULL) || (ch->in_room->area == NULL))
    return 0;
  for (index = ch->in_room->area->min_vnum;
       index <= ch->in_room->area->max_vnum; index++)
    {
      count += getbit (ch->pcdata->explored, index);
    }
  return count;
}


/* Count used vnums in current areas range */
int arearooms (CHAR_DATA * ch)
{
  int count = 0;
  int index = 0;
  if (IS_NPC (ch))
    return 0;
  if ((ch->in_room == NULL) || (ch->in_room->area == NULL))
    return 0;
  for (index = ch->in_room->area->min_vnum;
       index <= ch->in_room->area->max_vnum; index++)
    {
      if (get_room_index (index) != NULL)
	count++;
    }
  return count;
}

void do_call (CHAR_DATA * ch, char *argument)
{
  char object[MAX_INPUT_LENGTH];
  CHAR_DATA *mob, *mch;
  bool mount_type;		//0 for mount, 1 for behemoth
  int chance;
  int mana_table[] =
    { 40, 40, 40, 40, 40, 40, 40, 40, 50, 50, 50, 50, 50, 50, 50, 50,
    50, 50, 60, 60, 60, 60, 60, 60, 60, 60, 60, 70, 70, 70, 70, 70, 70,
    80, 80,
    80, 80, 80, 80, 80, 80, 80, 100, 100, 100, 100, 100, 100, 100, 100,
    100, 100,
    100, 100, 100, 100, 100, 150, 150, 150, 150, 150, 150, 150, 150,
    150, 150,
    150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150
  };
  int level = 0;
  one_argument (argument, object);
  if (object[0] == '\0')
    {
      send_to_char ("What would you like to call for?\n\r", ch);
      return;
    }
  //Iblis - short circuit for the nidae skill "call storm"
  if (!str_cmp (object, "storm"))
  { 
    call_storm(ch,argument);
    return;
  }
  if (!str_cmp (object, "mount") || !str_cmp (object, "beast"))
    {
      if (!str_cmp (object, "mount"))
	mount_type = 0;

      else
	mount_type = 1;
      if (!mount_type)
	{
	  if ((chance = get_skill (ch, gsn_summon_mount)) == 0)
	    {
	      send_to_char
		("You attempt to call for a mount, but none appear.\n\r", ch);
	      return;
	    }
	}

      else if ((chance = get_skill (ch, gsn_summon_beast)) == 0)
	{
	  send_to_char
	    ("You attempt to call for a behemoth, but none appear.\n\r", ch);
	  return;
	}
      if (!mount_type)
	level = Class_level (ch, 6);

      else
	{
	   if (ch->in_room->sector_type == SECT_CITY ||  !IS_OUTSIDE(ch))
	    {
	      send_to_char
		("Beasts do not travel to your current location.\n\r", ch);
	      return;
	    }

	  else
	    level = Class_level (ch, 4);
	}
      if (level == 0)
	level = 1;
      if (IS_IMMORTAL (ch))
	level = ch->level;
      if (ch->mana < mana_table[level - 23])
	{
	  send_to_char
	    ("You don't have enough mana to call for a mount.\n\r", ch);
	  return;
	}
      if (number_percent () > chance)
	{
	  if (!mount_type)
	    {
	      send_to_char
		("You attempt to call for a mount, but none appear.\n\r", ch);
	      check_improve (ch, gsn_summon_mount, FALSE, 4);
	    }

	  else
	    {
	      send_to_char
		("You attempt to call for a behemoth, but none appear.\n\r",
		 ch);
	      check_improve (ch, gsn_summon_beast, FALSE, 4);
	    }
	  ch->mana -= (mana_table[level - 23] / 2);
	  return;
	}
      for (mch = char_list; mch; mch = mch->next)
	{
	  if (!IS_NPC (mch))
	    continue;
	  if (mch->riders == ch || mch->mounter == ch)
	    {
	      if (!IS_NPC (ch) && ch->pcdata->boarded == mch)
		{
		  send_to_char
		    ("But you already have a mount boarded at a stable.\n\r",
		     ch);
		  return;
		}
	      if (mch->in_room == ch->in_room)
		{
		  send_to_char ("But your mount is already here.\n\r", ch);

		  return;
		}

	      else
		{
		  char_from_room (mch);
		  char_to_room (mch, ch->in_room);
		  if (ch->race == PC_RACE_NERIX)
		  {
		    add_follower (mch, ch);
		    mch->leader = ch;
		  }
		  
		  if (!mount_type)
		    {
		      act ("You summon forth your trusty steed.", ch,
			   NULL, NULL, TO_CHAR);
		      act ("$n summons forth $s trusty steed.", ch, NULL,
			   NULL, TO_ROOM);
		    }

		  else
		    {
		      act ("You summon forth your behemoth.", ch, NULL,
			   NULL, TO_CHAR);
		      act ("$n summons forth $s behemoth.", ch, NULL,
			   NULL, TO_ROOM);
		    }
		  return;
		}
	    }
	  if (mch->afkmsg != NULL && mch->afkmsg[0] != '\0'
	      && !str_cmp (mch->afkmsg, ch->name))
	    {
	      if (!IS_NPC (ch) && ch->pcdata->boarded == mch)
		{
		  send_to_char
		    ("But you already have a mount boarded at a stable.\n\r",
		     ch);
		  return;
		}
	      if (mch->in_room == ch->in_room)
		{
		  send_to_char ("But your mount is already here.\n\r", ch);

		  return;
		}

	      else
		{
		  char_from_room (mch);
		  char_to_room (mch, ch->in_room);
		  if (ch->race == PC_RACE_NERIX)
		  {
		    add_follower (mch, ch);
		    mch->leader = ch;
		  }
		                           
		  if (!mount_type)
		    {
		      act ("You summon forth your trusty steed.", ch,
			   NULL, NULL, TO_CHAR);
		      act ("$n summons forth $s trusty steed.", ch, NULL,
			   NULL, TO_ROOM);
		    }

		  else
		    {
		      act ("You summon forth your behemoth.", ch, NULL,
			   NULL, TO_CHAR);
		      act ("$n summons forth $s behemoth.", ch, NULL,
			   NULL, TO_ROOM);
		    }
		  return;
		}
	    }
	}
      if (!mount_type)
	{
	  if (level < 31)
	    mob = create_mobile (get_mob_index (MOB_VNUM_SUMMON_1));

	  else if (level < 41)
	    mob = create_mobile (get_mob_index (MOB_VNUM_SUMMON_2));

	  else if (level < 50)
	    mob = create_mobile (get_mob_index (MOB_VNUM_SUMMON_3));

	  else if (level < 56)
	    mob = create_mobile (get_mob_index (MOB_VNUM_SUMMON_4));

	  else if (level < 65)
	    mob = create_mobile (get_mob_index (MOB_VNUM_SUMMON_5));

	  else if (level < 80)
	    mob = create_mobile (get_mob_index (MOB_VNUM_SUMMON_6));

	  else
	    mob = create_mobile (get_mob_index (MOB_VNUM_SUMMON_7));
	}

      else
	{
	  mob = create_mobile (get_mob_index (MOB_VNUM_BEAST));
	  mob->max_hit = ch->max_hit;
	  mob->hit = ch->hit;
	  mob->max_move = ch->max_move;
	  mob->move = ch->move;
	  mob->level = ch->level;
	}
      char_to_room (mob, ch->in_room);
      mob->leader = ch;
      mob->mounter = ch;
      mob->afkmsg = str_dup (ch->name);
      SET_BIT (mob->act, ACT_SENTINEL);
      if (ch->race == PC_RACE_NERIX)
      {
	add_follower (mob, ch);
        mob->leader = ch;
      }
      add_follower (mob, ch);
      act ("You summon forth $N.", ch, NULL, mob, TO_CHAR);
      act ("$n summons forth $N.", ch, NULL, mob, TO_ROOM);
      if (!mount_type)
	check_improve (ch, gsn_summon_mount, TRUE, 4);

      else
	check_improve (ch, gsn_summon_beast, TRUE, 4);
      ch->mana -= mana_table[level - 23];
    }

  else
    {
      send_to_char ("You can't call for that.\n\r", ch);
      return;
    }
}
void do_showmem (CHAR_DATA * ch, char *argument)
{
  MEMORY_DATA *memd;
  char buf[MAX_STRING_LENGTH];
  for (memd = memories; memd; memd = memd->next)
    {
      sprintf (buf, "Mob: %s, Mem: %s\n\r", memd->mob->short_descr,
	       memd->player->name);
      send_to_char (buf, ch);
    }
}
void do_conceal (CHAR_DATA * ch, char *argument)
{
  SCENT_DATA *sc;
  int number, skillper, cnt;
  number = number_percent ();
  skillper = get_skill (ch, gsn_conceal);

  if (number > skillper)
    {
      check_improve (ch, gsn_conceal, FALSE, 1);
      return;
    }
  if (skillper < 33)
    cnt = 1;

  else if (skillper < 66)
    cnt = 2;

  else
    cnt = 3;
  check_improve (ch, gsn_conceal, TRUE, 1);
  for (sc = ch->in_room->scents; sc; sc = sc->next_in_room)
    if (is_name (ch->name, sc->player->name))
      {
	sc->scent_level -= cnt;
	if (sc->scent_level <= 0)
	  {
	    sc->scent_level = SCENT_WEAK;
	    extract_scent (sc);
	  }
	break;
      }
}
void do_track (CHAR_DATA * ch, char *argument)
{
  char to_track[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  one_argument (argument, to_track);
  if (get_skill (ch, gsn_track) < 1)
    {
      send_to_char ("You don't know how to track.\n\r", ch);
      return;
    }
  if (to_track[0] == '\0')
    {
      if (ch->tracking[0] != '\0')
	{
	  sprintf (tcbuf, "You are currently tracking: %s.\n\r",
		   ch->tracking);
	  send_to_char (tcbuf, ch);
	}

      else
	send_to_char ("Track who?\n\r", ch);
      return;
    }

  else if (!str_cmp (to_track, "off"))
    {
      free_string (ch->tracking);
      ch->tracking = str_dup ("");
      send_to_char ("Tracking turned off.\n\r", ch);
    }

  else
    {
      sprintf (tcbuf, "Tracking %s.\n\r", to_track);
      send_to_char (tcbuf, ch);
      free_string (ch->tracking);
      ch->tracking = str_dup (to_track);
    }
  return;
}

void display_track (CHAR_DATA * ch)
{
  char tracking[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int door_dir, found;
  SCENT_DATA *sc;
  int dirs[6] = { 0, 0, 0, 0, 0, 0 };
  int slevels[4] = { 0, 0, 0, 0 };
  ROOM_INDEX_DATA *location;
  if (number_percent () < get_skill (ch, gsn_track))
    {

      for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
	{
	  if (ch->in_room->exit[door_dir] == NULL)
	    continue;

	  location = ch->in_room->exit[door_dir]->u1.to_room;
	  if (location == NULL)
	    continue;
	  for (sc = location->scents; sc; sc = sc->next_in_room)
	    {
	      if (is_name (ch->tracking, sc->player->name))
		{
		  switch (sc->scent_level)
		    {
		    case SCENT_STRONG:
		      dirs[door_dir] = SCENT_STRONG;
		      slevels[SCENT_STRONG]++;
		      if (!IS_NPC (sc->player))
			strcpy (tracking, sc->player->name);

		      else
			strcpy (tracking, sc->player->short_descr);
		      break;
		    case SCENT_NOTICEABLE:
		      dirs[door_dir] = SCENT_NOTICEABLE;
		      slevels[SCENT_NOTICEABLE]++;
		      if (!IS_NPC (sc->player))
			strcpy (tracking, sc->player->name);

		      else
			strcpy (tracking, sc->player->short_descr);
		      break;
		    case SCENT_WEAK:
		      dirs[door_dir] = SCENT_WEAK;
		      slevels[SCENT_WEAK]++;
		      if (!IS_NPC (sc->player))
			strcpy (tracking, sc->player->name);

		      else
			strcpy (tracking, sc->player->short_descr);
		      break;
		    }
		  break;
		}
	    }
	}
      found = 0;
      sprintf (buf, "The strong scent of %s can be found", tracking);
      for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
	{
	  if (dirs[door_dir] == SCENT_STRONG)
	    {
	      if (found != slevels[SCENT_STRONG] - 1 && found != 0)
		strcat (buf, ",");
	      strcat (buf, " ");
	      if (door_dir == 4)
		strcat (buf, "upwards");

	      else if (door_dir == 5)
		strcat (buf, "downwards");

	      else
		strcat (buf, dir_name[door_dir]);
	      found++;
	      if (found == slevels[SCENT_STRONG] - 1
		  && slevels[SCENT_STRONG] != 1)
		strcat (buf, " and");
	    }
	}
      if (found > 0)
	{
	  strcat (buf, ".\n\r");
	  send_to_char (buf, ch);
	}
      found = 0;
      sprintf (buf, "You sense the scent of %s", tracking);
      for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
	{
	  if (dirs[door_dir] == SCENT_NOTICEABLE)
	    {
	      if (found != slevels[SCENT_NOTICEABLE] - 1 && found != 0)
		strcat (buf, ",");
	      strcat (buf, " ");
	      if (door_dir == 4)
		strcat (buf, "upwards");

	      else if (door_dir == 5)
		strcat (buf, "downwards");

	      else
		strcat (buf, dir_name[door_dir]);
	      found++;
	      if (found == slevels[SCENT_NOTICEABLE] - 1
		  && slevels[SCENT_NOTICEABLE] != 1)
		strcat (buf, " and");
	    }
	}
      if (found > 0)
	{
	  strcat (buf, ".\n\r");
	  send_to_char (buf, ch);
	}
      found = 0;
      sprintf (buf, "You catch the faint whiff of %s", tracking);
      for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
	{
	  if (dirs[door_dir] == SCENT_WEAK)
	    {
	      if (found != slevels[SCENT_WEAK] - 1 && found != 0)
		strcat (buf, ",");
	      strcat (buf, " ");
	      if (door_dir == 4)
		strcat (buf, "upwards");

	      else if (door_dir == 5)
		strcat (buf, "downwards");

	      else
		strcat (buf, dir_name[door_dir]);
	      found++;
	      if (found == slevels[SCENT_WEAK] - 1
		  && slevels[SCENT_WEAK] != 1)
		strcat (buf, " and");
	    }
	}
      if (found > 0)
	{
	  strcat (buf, ".\n\r");
	  send_to_char (buf, ch);
	}
      check_improve (ch, gsn_track, TRUE, 2);
    }

  else
    check_improve (ch, gsn_track, FALSE, 2);
}

int wall_damage (CHAR_DATA * ch, OBJ_DATA * wall)
{
  short chance, dam;
  dam = wall->level - get_curr_stat (ch, STAT_CON);
  if (ch->hit - dam <= 15 || dam <= 0)
    dam = 1;
  if (!IS_NPC (ch) && ch->pcdata->loner == FALSE)
    return FALSE;
  act
    ("A surge of pain passes through you as your skin is ripped by tiny thorns.",
     ch, NULL, NULL, TO_CHAR);
  act ("$n's skin is ripped by tiny thorns.", ch, NULL, NULL, TO_ROOM);
  ch->hit -= dam;
  if (wall->level < 50)
    chance = 5;

  else if (wall->level < 70)
    chance = 10;

  else if (wall->level < 80)
    chance = 12;

  else if (wall->level < 90)
    chance = 15;

  else
    chance = 20;
  if (number_percent () < chance)
    return (TRUE);
  return (FALSE);
}

bool is_campsite (ROOM_INDEX_DATA * location)
{
  OBJ_DATA *tObj;
  for (tObj = location->contents; tObj; tObj = tObj->next_content)
    if (tObj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE1
	|| tObj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE2)
      return (TRUE);
  return (FALSE);
}

void move_char (CHAR_DATA * ch, int door, bool follow)
{
  CHAR_DATA *fch;
  CHAR_DATA *fch_next;
  ROOM_INDEX_DATA *in_room;
  ROOM_INDEX_DATA *to_room;
  EXIT_DATA *pexit;
  char tcbuf[MAX_STRING_LENGTH];
  OBJ_DATA *tObj;
  short counter;
  int mount_reset;		// Mathew 7/6/99: For dealing with moving mounts.
  CHAR_DATA *blocker;		// Adeon 7/17/03 -- for blocking mobs
  CHAR_DATA *fch_mount_next;
  if (door < 0 || door > 5)
    {
      bug ("Do_move: bad door %d.", door);
      return;
    }
  if (!IS_NPC(ch) && ch->pcdata->cg_state != 0)
  {
    send_to_char("You can't move while playing a card game.\n\r",ch);
    return;
  }
  if (IS_NPC (ch) && ch->riders != NULL)
    return;
  if (IS_SET(ch->in_room->room_flags2,ROOM_MIRROR))
  {
    switch(door)
    {
      case DIR_NORTH: door = DIR_SOUTH;break;
      case DIR_SOUTH: door = DIR_NORTH;break;
      case DIR_EAST: door = DIR_WEST;break;
      case DIR_WEST: door = DIR_EAST;break;
      case DIR_UP: door = DIR_DOWN;break;
      case DIR_DOWN: door = DIR_UP;break;
    }
  }
  if (IS_NPC (ch) && IS_SET(ch->act2,ACT_PUPPET))
  {
    if((ch->pIndexData->vnum <= MOB_VNUM_WARPED_OWL && ch->pIndexData->vnum >= MOB_VNUM_FROG) || is_shapechanged_druid(ch))
    {
      if(ch->desc && ch->desc->original && ch->in_room->exit[door] && ch->in_room->exit[door]->u1.to_room)
      {
        if (!can_move_char(ch->desc->original,ch->in_room->exit[door]->u1.to_room,follow,TRUE))
	{
	  return;
	}
      }
    }
  }
  if (ch->position == POS_TETHERED)
    return;
  if (!check_web (ch))
    return;
  if (!check_entangle (ch))
    return;
  if (!check_clasp (ch))
    return;
  if (ch->position < POS_FIGHTING || ch->position >= POS_FEIGNING_DEATH)
    return;
  in_room = ch->in_room;
  if ((pexit = in_room->exit[door]) == NULL
      || (to_room = pexit->u1.to_room) == NULL
      || !can_see_room (ch, pexit->u1.to_room))
    {
      send_to_char ("Alas, you cannot go that way.\n\r", ch);
      if (!IS_NPC (ch))
	ch->pcdata->speedlen = 0;
      return;
    }

  if (is_affected (ch, gsn_disorientation) && number_percent() < 34)
    {
      for (door = number_range (0, 5), counter = 0; counter < 6;
	   counter++, door++)
	{
	  if (door > 5)
	    door = door % 6;
	  if ((pexit = in_room->exit[door]) == NULL
	      || (to_room = pexit->u1.to_room) == NULL
	      || !can_see_room (ch, pexit->u1.to_room))
	    continue;

	  else
	    break;
	}
      if (counter > 5)
	{
	  send_to_char ("Alas, you cannot go that way.\n\r", ch);
	  return;
	}
    }
  if (IS_SET(to_room->area->area_flags,AREA_SOLO) && (to_room->area->nplayer > 0) &&
      to_room->area != in_room->area)
    {
	send_to_char("That area is occupied!\n\r",ch);
      return;
    }

  if (IS_SET(to_room->room_flags2,ROOM_SOLO) && !IS_NPC(ch))
    {
      CHAR_DATA *mch;
      for (mch = to_room->people;mch != NULL; mch = mch->next_in_room)
        {
          if (!IS_NPC(mch))
	  {
	    send_to_char("That room is occupied!\n\r",ch);
	    return;
	  }
        }
    }
  if (!br_vnum_check (to_room->vnum))
    {
      send_to_char
	("That room is off limits during the `iBattle Royale``.\r\n", ch);
      return;
    }
  for (blocker = ch->in_room->people; blocker != NULL;
       blocker = blocker->next_in_room)
    {
      if (!IS_SET (blocker->act2, ACT_BLOCK_EXIT)
	  || blocker->blocks_exit == -1)
	continue;
      if (door != blocker->blocks_exit)
	continue;
      if (IS_IMMORTAL (ch) || IS_SET(ch->act2,ACT_NO_BLOCK))
      {
        sprintf (tcbuf, "%s would have blocked your way %s, except you're an Immortal.\n\r",
			blocker->short_descr, dir_name[blocker->blocks_exit]);
	send_to_char(tcbuf,ch);
	break;
      }
      sprintf (tcbuf, "%s blocks your way %s...\n\r",
	       blocker->short_descr, dir_name[blocker->blocks_exit]);
      send_to_char (tcbuf, ch);
      return;
    }
  if (ch->pcdata && ch->pcdata->fishing)
    {
      send_to_char ("You can't move while you're fishing!.\n\r", ch);
      return;
    }
  // Mathew 4/16/99: Fixed the logic of the closed door check to make
  // more sense.
  if (IS_SET (pexit->exit_info, EX_CLOSED) &&
      (!IS_AFFECTED (ch, AFF_PASS_DOOR)
       || IS_SET (pexit->exit_info, EX_NOPASS)) && !IS_IMMORTAL(ch)
    )
    {
      act ("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
      return;
    }
  if (IS_AFFECTED (ch, AFF_CHARM)
      && ch->master != NULL && in_room == ch->master->in_room)
    {
      send_to_char ("What?  And leave your beloved master?\n\r", ch);
      return;
    }
  if (ch->mount != NULL)
    if (ch->mount->position != POS_STANDING
	&& ch->mount->position != POS_FIGHTING)
      {
	sprintf (tcbuf,
		 "%s will have to stand before you can ride it any further.\n\r",
		 fcapitalize (ch->mount->short_descr));
	send_to_char (tcbuf, ch);
	return;
      }
  if (IS_SET (to_room->room_flags, ROOM_NOMOUNT) && ch->mount != NULL
      && !IS_AFFECTED (ch->mount, AFF_FLYING)
      && !IS_AFFECTED (ch->mount, AFF_SWIM))
    {
      send_to_char ("You may not ride mounts in there.\n\r", ch);
      return;
    }
  if (IS_SET (to_room->room_flags, ROOM_NOSWIMMOUNT)
      && ch->mount != NULL && IS_AFFECTED (ch->mount, AFF_SWIM))
    {
      send_to_char ("You may not swim with mounts in there.\n\r", ch);
      return;
    }
  if (IS_SET (to_room->room_flags, ROOM_NOFLYMOUNT)
      && ch->mount != NULL && IS_AFFECTED (ch->mount, AFF_FLYING))
    {
      send_to_char ("You may not fly mounts in there.\n\r", ch);
      return;
    }
  if ((ch->riders != NULL || ch->mounter != NULL)
      && IS_SET (to_room->room_flags,
		 ROOM_NOMOUNT) && (!IS_AFFECTED (ch, AFF_FLYING)
				   && !IS_AFFECTED (ch, AFF_SWIM)))
    {
      send_to_char ("You can't go in there, mount.\n", ch);
      return;
    }
  if ((ch->riders != NULL || ch->mounter != NULL)
      && IS_SET (to_room->room_flags, ROOM_NOFLYMOUNT)
      && IS_AFFECTED (ch, AFF_FLYING))
    {
      send_to_char ("You can't fly in there, mount.\n", ch);
      return;
    }
  if ((ch->riders != NULL || ch->mounter != NULL)
      && IS_SET (to_room->room_flags, ROOM_NOSWIMMOUNT)
      && IS_AFFECTED (ch, AFF_SWIM))
    {
      send_to_char ("You can't swim in there, mount.\n", ch);
      return;
    }
  if (to_room->max_level != 0 && ch->level > to_room->max_level
      && !IS_IMMORTAL (ch))
    {
      send_to_char ("You aren't allowed in there.\n\r", ch);
      return;
    }
// Minax 2-13-03 Keep charmies out of no_charmie rooms.
  if (IS_AFFECTED (ch, AFF_CHARM)
      && (IS_SET (to_room->room_flags, ROOM_NO_CHARMIE)))
    {
      send_to_char
	("You aren't allowed in there because you are charmed.\n\r", ch);
      return;
    }
  if (to_room->Class_flags != 0 && !IS_IMMORTAL (ch))
    {

      if (!room_Class_check (ch, to_room))
	{
	  send_to_char ("You aren't allowed in there.\n\r", ch);
	  return;
	}
    }
  if ((to_room->clan != CLAN_BOGUS) && (ch->clan != to_room->clan)
      && !IS_IMMORTAL (ch) && !IS_SET (ch->act, ACT_PET) && !IS_SET(ch->act2,ACT_PUPPET))
    {
      send_to_char ("You aren't allowed in there.\n\r", ch);
      return;
    }
  if (to_room->race_flags != 0)
  {
    if (!room_race_check(ch,to_room))
    {
      send_to_char ("You aren't allowed in there.\n\r", ch);
      return;
    }
  }
  if (!is_room_owner (ch, to_room) && room_is_private (to_room))
    {
      if (to_room->max_message[0] == '\0')
	send_to_char ("That room is private right now.\n\r", ch);

      else
	{
	  send_to_char (to_room->max_message, ch);
	  send_to_char ("\n\r", ch);
	}
      return;
    }
  //Iblis 6/21/03 - Battle Royale room off limit checking
  if (!br_vnum_check (to_room->vnum))
    {
      send_to_char
	("That room is off limits during `iBattle Royale``\r\n", ch);
      return;
    }
  if (!IS_IMMORTAL (ch) && IS_SET (to_room->room_flags, ROOM_IMP_ONLY))
    {
      send_to_char ("Sorry, you can't go in there.\n\r", ch);
      return;
    }
  if (IS_SET (to_room->room_flags, ROOM_SPECTATOR))
    {
      OBJ_DATA *obj, *obj_next;
      bool found = FALSE;
      if (IS_IMMORTAL (ch))
	found = TRUE;

      else
	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	  {
	    obj_next = obj->next_content;
	    if (obj->pIndexData->vnum == OBJ_VNUM_ATICKET)
	      {
		found = TRUE;
		extract_obj (obj);
		break;
	      }
	  }
      if (!found)
	{
	  send_to_char
	    ("A burly bouncer smirks and says 'Trying to sneak in here without a ticket?  I don't think so.'\n\r",
	     ch);
	  return;
	}
    }
  if (to_room->vnum == 4334)
    {
      OBJ_DATA *obj, *obj_next;
      bool found = FALSE;
      if (IS_IMMORTAL (ch))
	found = TRUE;

      else
	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	  {
	    obj_next = obj->next_content;
	    if (obj->pIndexData->vnum == OBJ_VNUM_ZTICKET)
	      {
		found = TRUE;
		extract_obj (obj);
		break;
	      }
	  }
      if (!found)
	{
	  send_to_char
	    ("A zoo attendant stops you and says 'Sorry, you need a ticket to get in there.'\n\r",
	     ch);
	  return;
	}
    }
  if (!IS_NPC (ch))
    {
      int iClass, iGuild;
      int move;
      for (iClass = 0; iClass < MAX_CLASS; iClass++)
	{
	  for (iGuild = 0; iGuild < MAX_GUILD; iGuild++)
	    {
	      if (!(IS_CLASS (ch, iClass))
		  && to_room->vnum == Class_table[iClass].guild[iGuild])
		{
		  send_to_char ("You aren't allowed in there.\n\r", ch);
		  return;
		}
	    }
	}
      if (IS_SET (to_room->room_flags, ROOM_NO_FLY))
	{
	  if (ch->mount == NULL)
	    {
	      if (IS_AFFECTED (ch, AFF_FLYING) && !IS_IMMORTAL (ch))
		{
		  send_to_char ("You can't fly there.\n\r", ch);
		  return;
		}
	    }

	  else
	    {
	      if (IS_AFFECTED (ch->mount, AFF_FLYING))
		{
		  send_to_char ("You can't fly your mount there.\n\r", ch);
		  return;
		}
	    }
	}
      if (in_room->sector_type == SECT_AIR
	  || to_room->sector_type == SECT_AIR)
	{
	  if (ch->mount == NULL)
	    {
	      if (!IS_AFFECTED (ch, AFF_FLYING) && !IS_IMMORTAL (ch))
		{
		  send_to_char ("You can't fly.\n\r", ch);
		  return;
		}
	    }

	  else
	    {
	      if (!IS_AFFECTED (ch->mount, AFF_FLYING))
		{
		  send_to_char ("Your mount cannot fly.\n\r", ch);
		  return;
		}
	    }
	}

      //Iblis 10/30/04 - Ice Room Check
/*      if (!IS_AFFECTED(ch,AFF_FLYING) && IS_SET(to_room->room_flags2,ROOM_ICE))
      {
        char_from_room(ch);
	char_to_room(ch,to_room);
	do_look(ch,"auto");
	if (can_move_char_door(ch,door,follow,FALSE))
        {
          send_to_char("The slickness of the ice on the ground causes you to slide into the next room!\n\r",ch);
	  if (is_sliding) //this means they slid in from the previous room
	    act("$n slides into the room.",ch,NULL,NULL,TO_ROOM);	  
	  else {

	  }
	  act("The slickness of the ice on the ground causes $n to slide $T!",ch,NULL,dir_name[door],TO_ROOM);
	   is_sliding = TRUE;
	   move_char(ch,door,follow);
	   if (is_sliding) //this means this is their final resting place
  	     act("$n slides into the room.",ch,NULL,NULL,TO_ROOM);
	   is_sliding = FALSE;
	   //return;
	 }
	
	// Move a group if ch was the group leader. 4/14/99 Mathew
	mount_reset = 0;
	fch_mount_next = NULL;
	for (fch = in_room->people; fch != NULL; fch = fch_next)
	  {
	    
	    // Mathew 4/14/99: Skip mounts with riders. They are moved with the
	    // rider.
	    // Mathew 7/6/99: Fix mount problem
	    if (mount_reset)
	      {
		mount_reset = 0;      // Reset the flag.
		fch = fch_mount_next;
	      }
	    if (fch != NULL)
	      {
		fch_next = fch->next_in_room;
	      }
	    
	    else
	      {
		fch_next = NULL;      // Mathew 7/6/99: Skip NULL fch's due to mount resets
		continue;
	      }
	    if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
		&& fch->position < POS_STANDING)
	      do_stand (fch, "");
	    if ((fch->master == ch || fch->stalking == ch)
		&& fch->position >= POS_STANDING && fch->position < POS_FEIGNING_DEATH && fch->in_room != ch->
		in_room)
	      {
		if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
		    && (IS_NPC (fch) && is_aggro (fch)))
		  {
		    act ("You can't bring $N into the city.", ch, NULL, fch,
			 TO_CHAR);
		    act ("You aren't allowed in the city.", fch, NULL, NULL,
			 TO_CHAR);
		    
		    continue;
		  }
		//5-10-03 Iblis - Check to remove camoflauge/hide if following someone
		if (IS_AFFECTED (fch, AFF_CAMOUFLAGE))
		  REMOVE_BIT (fch->affected_by, AFF_CAMOUFLAGE);
		if (IS_AFFECTED (fch, AFF_HIDE))
		  REMOVE_BIT (fch->affected_by, AFF_HIDE);
		act ("You follow $N.", fch, NULL, ch, TO_CHAR);
		if (fch_next != NULL && fch->mount != NULL)
		  {
		    mount_reset++;    // Mathew 7/6/99: Reset to top person in
		    // old room if a rider and a mount move.
		    if (fch->next_in_room != NULL
			&& fch->next_in_room == fch->mount)
		      {
			fch_mount_next = fch->mount->next_in_room;    // Skip the mount.
		      }
		    
		    else
		      {
			fch_mount_next = fch->next_in_room;   // Preserve old list.
		      }
		  }
		move_char (fch, door, TRUE);
	      }
	  }
	
*/	
/*	if (can_move_char_door(ch,door,follow,FALSE))
	{
	  send_to_char("The slickness of the ice on the ground causes you to slide into the next room!\n\r",ch);
	  act("The slicksness of the ice on the ground causes $n to slide $T!",ch,NULL,dir_name[door],TO_ROOM);
	  is_sliding = TRUE;
	  move_char(ch,door,follow);
	  is_sliding = FALSE;
	  return;
	}*/
/*	return;
      }*/

      /* BEGIN UNDERWATER */
      if ((in_room->sector_type == SECT_UNDERWATER
	   || to_room->sector_type == SECT_UNDERWATER)
	  && !IS_AFFECTED (ch, AFF_HASTE))
	{
	  if (!IS_IMMORTAL (ch) && ch->race != PC_RACE_NIDAE)
	    WAIT_STATE (ch, 10);
	}
      if ((in_room->sector_type == SECT_WATER_NOSWIM
	   || to_room->sector_type == SECT_WATER_NOSWIM
	   || to_room->sector_type == SECT_WATER_OCEAN
	   || in_room->sector_type == SECT_WATER_OCEAN)
	  && !IS_AFFECTED (ch, AFF_FLYING))
	{
	  bool found = FALSE;
	  OBJ_DATA *mObj;
	  if (ch->mount != NULL && IS_AFFECTED (ch->mount, AFF_FLYING))
	    found = TRUE;

	  else if (IS_IMMORTAL (ch))
	    found = TRUE;

	  else
	    for (mObj = ch->carrying; mObj; mObj = mObj->next_content)
	      {
		if (mObj->item_type == ITEM_RAFT)
		  {
		    found = TRUE;
		    break;
		  }
	      }

	}
      move = movement_loss[UMIN (SECT_MAX - 1, in_room->sector_type)]
	+ movement_loss[UMIN (SECT_MAX - 1, to_room->sector_type)];
      move /= 2;		/* i.e. the average */
      if (to_room->sector_type == SECT_UNDERWATER
	  && IS_AFFECTED (ch, AFF_AQUA_BREATHE))
	move = 4;

      if ((to_room->sector_type == SECT_WATER_OCEAN
          || to_room->sector_type == SECT_WATER_SWIM
          || to_room->sector_type == SECT_WATER_NOSWIM
	   || to_room->sector_type == SECT_UNDERWATER)
          &&
	  (ch->in_room->sector_type == SECT_WATER_OCEAN
            || ch->in_room->sector_type == SECT_WATER_SWIM
            || ch->in_room->sector_type == SECT_WATER_NOSWIM
	   || ch->in_room->sector_type == SECT_UNDERWATER))
	{
	  if (!IS_AFFECTED (ch, AFF_AQUA_BREATHE) && !IS_IMMORTAL (ch) && !(get_skill(ch,gsn_swim) > number_range(1,100)) && ch->move > 0 && !(get_skill(ch,gsn_swim) == 100) && !IS_SET(ch->imm_flags,IMM_DROWNING))
	    {
	      OBJ_DATA *obj;
	      bool found;

	      found = FALSE;

	      for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		{
		  if (obj->item_type == ITEM_GILLS
		      && obj->wear_loc != WEAR_NONE)
		    {
		      found = TRUE;
		      break;
		    }
		}

	      if (!found)
		{
		  ch->mana -= ch->max_mana/15.0 + (number_range(1,10)-5);
		  ch->move -= ch->max_move/15.0 + (number_range(1,10)-5);

		  send_to_char ("Your lungs fill with water.\n\r", ch);
		  act ("$n turns purple floundering for air.", ch, NULL,
		       NULL, TO_ROOM);
		  //Iblis 10/29/04 - Changed to prevent crashing when mobs drowned on move
		  if (ch->hit <= ch->max_hit/15.0)
		  {
		    ch->hit = 1;
		    if (ch->move > 0)
		      ch->move = -1;
		  }
		  else ch->hit -= ch->max_hit/15.0;
		}
	    }
	
      //12-30-03 Fail this swimming check, and you go off in a random direction
      if (get_skill(ch,gsn_swim) < number_range(1,100) && !(get_skill(ch,gsn_swim) == 100))
      {
        int door=0;
        for (door = number_range (0, 5), counter = 0; counter < 6;
           counter++, door++)
        {
          if (door > 5)
            door = door % 6;
          if ((pexit = in_room->exit[door]) == NULL
              || (to_room = pexit->u1.to_room) == NULL
              || !can_see_room (ch, pexit->u1.to_room)
              || (IS_SET (pexit->exit_info, EX_CLOSED) &&
                 (!IS_AFFECTED (ch, AFF_PASS_DOOR)|| IS_SET (pexit->exit_info, EX_NOPASS))))
            continue;

          else
            break;
        }
      if (counter > 5)
        {
          send_to_char ("Alas, you cannot go that way.\n\r", ch);
          return;
        }      
      }
	}
    

      /* conditional effects */
      if (get_carry_weight (ch) > can_carry_w (ch))
        move *= 2;
      if (IS_AFFECTED (ch, AFF_FLYING) || IS_AFFECTED (ch, AFF_HASTE))
	move /= 2;
      if (IS_AFFECTED (ch, AFF_SLOW))
	move *= 2;
      if (!IS_NPC (ch) && ch->pcdata->speedlen != 0)
	move *= 2;
      if (!IS_NPC (ch) && ch->race == PC_RACE_NIDAE && ch->in_room &&
	    (ch->in_room->sector_type == SECT_UNDERWATER 
	       || ch->in_room->sector_type == SECT_WATER_OCEAN))
	      move = 1;
      if (ch->mount == NULL && ch->move < move && !is_sliding)
	{
	  send_to_char ("You are too exhausted.\n\r", ch);
	  if (!IS_NPC (ch))
	    ch->pcdata->speedlen = 0;
	  return;
	}
      if (!move_fast)
        WAIT_STATE (ch, 1);
      if (ch->mount == NULL)
	ch->move -= move;
    }
  for (tObj = ch->in_room->contents; tObj; tObj = tObj->next_content)
    if (tObj->pIndexData->vnum == OBJ_VNUM_WALL_THORNS
	&& tObj->value[0] == door)
      {
	if (wall_damage (ch, tObj))
	  {
	    send_to_char
	      ("You are unable to break through the wall of thorns.\n\r", ch);
	    return;
	  }
	break;
      }
  if (ch->mount != NULL)
    {
      char tcbuf[MAX_STRING_LENGTH];
      int move;
      move = movement_loss[UMIN (SECT_MAX - 1, in_room->sector_type)]
	+ movement_loss[UMIN (SECT_MAX - 1, to_room->sector_type)];
      move /= 2;		/* i.e. the average */
      if (to_room->sector_type == SECT_UNDERWATER
	  && IS_AFFECTED (ch, AFF_AQUA_BREATHE))
	move = 4;

      if ((to_room->sector_type == SECT_WATER_OCEAN
          || to_room->sector_type == SECT_WATER_SWIM
          || to_room->sector_type == SECT_WATER_NOSWIM
	   || to_room->sector_type == SECT_UNDERWATER)
          &&
	  (ch->in_room->sector_type == SECT_WATER_OCEAN
            || ch->in_room->sector_type == SECT_WATER_SWIM
            || ch->in_room->sector_type == SECT_WATER_NOSWIM
	   || ch->in_room->sector_type == SECT_UNDERWATER))
	{
	  if (!IS_AFFECTED (ch, AFF_AQUA_BREATHE) && !IS_IMMORTAL (ch) && !(get_skill(ch,gsn_swim) > number_range(1,100)) && !IS_SET(ch->imm_flags,IMM_DROWNING) && ch->move > 0)
	  {
	    OBJ_DATA *obj;
	    bool found;

	    found = FALSE;

	    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	      {
		if (obj->item_type == ITEM_GILLS
		    && obj->wear_loc != WEAR_NONE)
		  {
		    found = TRUE;
		    break;
		  }
	      }

	    if (!found)
	      {
		ch->mana -= ch->max_mana/15.0 + (number_range(1,10)-5);
		ch->move -= ch->max_move/15.0 + (number_range(1,10)-5);

		send_to_char ("Your lungs fill with water.\n\r", ch);
		act ("$n turns purple floundering for air.", ch, NULL, NULL, TO_ROOM);
		//Iblis 10/29/04 - Changed to prevent crashing when mobs drowned on move
		if (ch->hit <= ch->max_hit/15.0)
		{
			ch->hit = 1;
			if (ch->move > 0)
				ch->move = -1;
		}
		else ch->hit -= ch->max_hit/15.0;
	      }
	  }
	
      //12-30-03 Fail this swimming check, and you go off in a random direction
      if (get_skill(ch,gsn_swim) < number_range(1,100))
      {
        int door=0;
        for (door = number_range (0, 5), counter = 0; counter < 6;
           counter++, door++)
        {
          if (door > 5)
            door = door % 6;
          if ((pexit = in_room->exit[door]) == NULL
              || (to_room = pexit->u1.to_room) == NULL
              || !can_see_room (ch, pexit->u1.to_room)
	      || (IS_SET (pexit->exit_info, EX_CLOSED) &&
		 (!IS_AFFECTED (ch, AFF_PASS_DOOR)|| IS_SET (pexit->exit_info, EX_NOPASS))))		  
            continue;

          else
            break;
        }
      if (counter > 5)
        {
          send_to_char ("Alas, you cannot go that way.\n\r", ch);
          return;
        }
      }
	}

      /* conditional effects */
      if (get_carry_weight (ch) > can_carry_w (ch))
        move *= 2;
      if (IS_AFFECTED (ch, AFF_FLYING) || IS_AFFECTED (ch, AFF_HASTE))
	move /= 2;
      if (IS_AFFECTED (ch, AFF_SLOW))
	move *= 2;
      if (!IS_NPC (ch) && ch->pcdata->speedlen != 0)
	move *= 2;
      if (!ch->mount)
        return;
      if (ch->mount->move < move && !is_sliding)
	{
	  send_to_char ("Your mount is too exhausted.\n\r", ch);
	  if (!IS_NPC (ch))
	    ch->pcdata->speedlen = 0;
	  return;
	}
      if (ch->mount->hit <= 0 && !is_sliding)
	{
	  send_to_char ("Your mount is in no shape to move around.\n\r", ch);
	  if (!IS_NPC (ch))
	    ch->pcdata->speedlen = 0;
	  return;
	}
      ch->mount->move -= move;
      if (!is_sliding)
      {
        sprintf (tcbuf, "You ride $T on %s.", ch->mount->short_descr);
        act (tcbuf, ch, NULL, dir_name[door], TO_CHAR);
      }
    }

  if (!IS_AFFECTED (ch, AFF_SNEAK) && ch->stalking == NULL
      && ch->invis_level < LEVEL_HERO
      && !(ch->mount && ch->mount->pIndexData->vnum == MOB_VNUM_BEAST)
      && !(ch->pIndexData && ch->pIndexData->vnum == MOB_VNUM_BEAST)
      && !(ch->race == PC_RACE_KALIAN && (time_info.hour > 18 || time_info.hour < 5))
      && !is_sliding
       )
    {
      if (ch->in_room->exit_msg[0] != '\0')
      {
        if (IS_SET(ch->act2,ACT_IMAGINARY))
          act (ch->in_room->exit_msg, ch, NULL, dir_name[door], TO_CJIR);
        else act (ch->in_room->exit_msg, ch, NULL, dir_name[door], TO_ROOM);
      }

      else if (ch->in_room->sector_type == SECT_UNDERWATER)
      {
        if (IS_SET(ch->act2,ACT_IMAGINARY))
  	  act ("$n swims $T.", ch, NULL, dir_name[door], TO_CJIR);
	else act ("$n swims $T.", ch, NULL, dir_name[door], TO_ROOM);
      }

      else if (IS_AFFECTED (ch, AFF_FLYING))
      {
	if (IS_SET(ch->act2,ACT_IMAGINARY))
	  act ("$n flies $T.", ch, NULL, dir_name[door], TO_CJIR);
	else act ("$n flies $T.", ch, NULL, dir_name[door], TO_ROOM);
      }

      else if (ch->mount != NULL)
	{
	  char tcbuf[MAX_STRING_LENGTH];
	  sprintf (tcbuf, "$n leaves $T, riding %s.", ch->mount->short_descr);
	  if (IS_SET(ch->act2,ACT_IMAGINARY))
  	    act (tcbuf, ch, NULL, dir_name[door], TO_CJIR);
	  else act (tcbuf, ch, NULL, dir_name[door], TO_ROOM);
	}

      else
      {
        if (IS_SET(ch->act2,ACT_IMAGINARY))
  	  act ("$n leaves $T.", ch, NULL, dir_name[door], TO_CJIR);
	else act ("$n leaves $T.", ch, NULL, dir_name[door], TO_ROOM);

      }
    }
  else 
    {
      if (!is_sliding)
      {
        act ("$n sneaks $T.",ch,NULL,dir_name[door],TO_IIR);
        if (ch->race == PC_RACE_KALIAN && (time_info.hour > 18 || time_info.hour < 5))
          {
            send_to_char("You slide unnoticed through the shadows to the ",ch);
            send_to_char(dir_name[door],ch);
            send_to_char(".\n\r",ch);
          }
      }
    }

  /* Move the Mount */
  if (ch->mount != NULL)
    {
      char_from_room (ch->mount);
      char_to_room (ch->mount, to_room);
    }
  if (!IS_NPC(ch) && ch->race == PC_RACE_LITAN && ch->pcdata->flaming
	  && weather_info.sky != SKY_RAINING && weather_info.sky != SKY_LIGHTNING
	  && ch->in_room->sector_type != SECT_WATER_SWIM
	  && ch->in_room->sector_type != SECT_WATER_NOSWIM
	  && ch->in_room->sector_type != SECT_WATER_OCEAN
	  && ch->in_room->sector_type != SECT_SWAMP
	  && ch->in_room->sector_type != SECT_UNDERWATER)
  {
	  OBJ_DATA* obj;
	  char tcbuf[MAX_STRING_LENGTH];
	  obj = get_obj_vnum_room (ch->in_room, OBJ_VNUM_LITAN_FOOTSTEPS);
	  if (obj)
	  {
	    obj_from_room(obj);
	    extract_obj(obj);
	  }
	  obj = create_object(get_obj_index(OBJ_VNUM_LITAN_FOOTSTEPS),0);
	  obj->timer = 1;
          sprintf (tcbuf, obj->description, dir_name[door]);
          free_string (obj->description);
          obj->description = str_dup (tcbuf);
	  obj_to_room(obj,ch->in_room);
  }
  if (IS_SET(ch->in_room->room_flags2,ROOM_SNOW) && !IS_SET(ch->affected_by,AFF_FLYING))
  {
    OBJ_DATA* obj;
    char tcbuf[MAX_STRING_LENGTH];
    obj = get_obj_vnum_room (ch->in_room, OBJ_VNUM_LITAN_FOOTSTEPS);
    if (obj)
    {
      obj_from_room(obj);
      extract_obj(obj);
    }
    obj = create_object(get_obj_index(OBJ_VNUM_LITAN_FOOTSTEPS),0);
    obj->timer = 1;
    sprintf (tcbuf, "`hFootsteps in the snow lead off to the %s.``",dir_name[door]);
    free_string (obj->description);
    obj->description = str_dup (tcbuf);
    sprintf (tcbuf, "`ha footstep in the snow``");
    free_string (obj->short_descr);
    obj->short_descr = str_dup (tcbuf);
    obj->value[0] = 1;
    obj_to_room(obj,ch->in_room);
  }
  /* Move the Character */
  char_from_room (ch);
  char_to_room (ch, to_room);

  if (!IS_NPC(ch) && ch->race == PC_RACE_NIDAE && ch->in_room)
  {
    if (ch->in_room->sector_type == SECT_UNDERWATER
		    || ch->in_room->sector_type == SECT_WATER_OCEAN)
    {
      if (IS_AFFECTED(ch,AFF_SLOW))
      {
	 if (IS_AFFECTED(ch,AFF_SLOW))
         {
           AFFECT_DATA *paf;
           AFFECT_DATA *paf_next;
           for (paf = ch->affected; paf != NULL; paf = paf_next)
           {
             paf_next = paf->next;
             if (paf->type == gsn_dehydration)
              {
                send_to_char (skill_table[paf->type].msg_off, ch);
                send_to_char ("\n\r", ch);
                affect_remove (ch, paf);
              }
	   }
	 }
      }
      ch->pcdata->oow_ticks = 0;
    }
    else if (!IS_AFFECTED(ch,AFF_SLOW))
    {
      AFFECT_DATA af;
      ch->perm_stat[STAT_CHA] -= 9;
      ch->perm_stat[STAT_DEX] -= 5;
      af.where = TO_AFFECTS;
      af.type = gsn_dehydration;
      af.level = ch->level;
      af.duration = 5;  //Doesn't matter, its perm til they're in water again
      af.location = APPLY_DEX;
      af.modifier = 0;
      af.bitvector = AFF_SLOW;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (ch, &af);
      send_to_char ("Your skin begins to dry out and you feel less agile.\n\r", ch);
    }
  }

  /* BEGIN ARENA */
  if (is_dueling (ch))
    {
      char buf[MAX_STRING_LENGTH];
      CHAR_DATA *vict, *v_next;
      if (IS_AFFECTED (ch, AFF_SNEAK)
	  || (ch->mount && ch->mount->pIndexData->vnum == MOB_VNUM_BEAST) 
	  || (ch->pIndexData && ch->pIndexData->vnum == MOB_VNUM_BEAST)
          || (ch->race == PC_RACE_KALIAN && time_info.hour > 18 && time_info.hour < 5))
	sprintf (buf, "%s moves stealthily into %s.\n\r", ch->name,
		 to_room->name);

      else if (IS_AFFECTED (ch, AFF_FLYING))
	sprintf (buf, "%s flies %s into %s.\n\r", ch->name,
		 dir_name[door], to_room->name);

      else
	sprintf (buf, "%s moves %s into %s.\n\r", ch->name,
		 dir_name[door], to_room->name);
      arena_report (buf);
      for (vict = ch->in_room->people; vict != NULL; vict = v_next)
	{
	  v_next = vict->next_in_room;
	  if (vict != ch)
	    {
	      if (!IS_AFFECTED (ch, AFF_SNEAK)
		  && ch->invis_level < LEVEL_HERO
		  && !(ch->mount && ch->mount->pIndexData->vnum == MOB_VNUM_BEAST) 
		  && !(ch->pIndexData  && ch->pIndexData->vnum == MOB_VNUM_BEAST)
   	          && !(ch->race == PC_RACE_KALIAN && time_info.hour > 18 && time_info.hour < 5))
		{
		  sprintf (buf,
			   "%s prepares, noticing %s's approach.\n\r",
			   vict->name, ch->name);
		  arena_report (buf);
		}

	      else
		{
		  sprintf (buf, "%s escapes %s's notice!\n\r", ch->name,
			   vict->name);
		  arena_report (buf);
		}
	    }
	}
    }

  /* END ARENA */
  for (tObj = ch->in_room->contents; tObj; tObj = tObj->next_content)
    if (tObj->pIndexData->vnum == OBJ_VNUM_WALL_THORNS
	&& tObj->value[0] == rev_dir[door])
      {
	wall_damage (ch, tObj);
	break;
      }
  if (!IS_AFFECTED (ch, AFF_SNEAK) && ch->stalking == NULL
      && ch->invis_level < LEVEL_HERO
      && !(ch->mount && ch->mount->pIndexData->vnum == MOB_VNUM_BEAST)
      && !(ch->pIndexData && ch->pIndexData->vnum == MOB_VNUM_BEAST)
      && !(ch->race == PC_RACE_KALIAN && (time_info.hour > 18 || time_info.hour < 5))
      && !is_sliding
      )
    {
      if (ch->in_room->enter_msg[0] != '\0')
	{
	  char tmp_buf[MAX_STRING_LENGTH];
	  char *cptr;
	  if ((cptr = strstr (ch->in_room->enter_msg, "$w")) != NULL)
	    {
	      *cptr = '\0';
	      strcpy (tmp_buf, ch->in_room->enter_msg);
	      strcat (tmp_buf, dir_name[rev_dir[door]]);
	      strcat (tmp_buf, cptr + 2);
	      *cptr = '$';
	      if (IS_SET(ch->act2,ACT_IMAGINARY))
		      act (tmp_buf, ch, NULL, NULL, TO_CJIR);
	      else act (tmp_buf, ch, NULL, NULL, TO_ROOM);
	    }

	  else 
	  {
	    if (IS_SET(ch->act2,ACT_IMAGINARY))
	      act (ch->in_room->enter_msg, ch, NULL, NULL, TO_CJIR);
	    else  act (ch->in_room->enter_msg, ch, NULL, NULL, TO_ROOM);
	  }
	}

      else if (ch->in_room->sector_type == SECT_UNDERWATER)
      {
        if (IS_SET(ch->act2,ACT_IMAGINARY))
          act (ch->in_room->enter_msg, ch, NULL, NULL, TO_CJIR);
	else act ("$n swims into view.", ch, NULL, NULL, TO_ROOM);
      }

      else if (IS_AFFECTED (ch, AFF_FLYING))
      {
        if (IS_SET(ch->act2,ACT_IMAGINARY))
           act ("$n flies into the room.", ch, NULL, NULL, TO_CJIR);
	else act ("$n flies into the room.", ch, NULL, NULL, TO_ROOM);
      }

      else if (ch->mount != NULL)
	{
	  char tcbuf[MAX_STRING_LENGTH];
	  sprintf (tcbuf, "$n arrives, riding %s.", ch->mount->short_descr);
	  if (IS_SET(ch->act2,ACT_IMAGINARY))
	    act (tcbuf, ch, NULL, NULL, TO_CJIR);
	  else  act (tcbuf, ch, NULL, NULL, TO_ROOM);
	}

      else
        {
	  if (IS_SET(ch->act2,ACT_IMAGINARY))
	    act ("$n has arrived.", ch, NULL, NULL, TO_CJIR);
	  else act ("$n has arrived.", ch, NULL, NULL, TO_ROOM);
	}
	  
    }
  else if (!is_sliding)
	  act ("$n sneaks into the room.",ch,NULL,dir_name[door],TO_IIR);
  do_look (ch, "auto");

  //Iblis 10/30/04 - Ice Room Check
  if (!IS_AFFECTED(ch,AFF_FLYING) && IS_SET(to_room->room_flags2,ROOM_ICE))
    {
      if (can_move_char_door(ch,door,follow,FALSE))
        {
          send_to_char("The slickness of the ice on the ground causes you to slide into the next room!\n\r",ch);
          if (is_sliding) //this means they slid in from the previous room
            act("$n slides into the room.",ch,NULL,NULL,TO_ROOM);
          act("The slickness of the ice on the ground causes $n to slide $T!",ch,NULL,dir_name[door],TO_ROOM);
	  is_sliding = TRUE;
	  move_char(ch,door,FALSE); //do NOT follow this person if they are sliding
	  if (is_sliding) //this means this is their final resting place
	    act("$n slides into the room.",ch,NULL,NULL,TO_ROOM);
	  is_sliding = FALSE;
	  //return;
	}
      // Move a group if ch was the group leader. 4/14/99 Mathew
      mount_reset = 0;
      fch_mount_next = NULL;
      for (fch = in_room->people; fch != NULL; fch = fch_next)
	{

	  // Mathew 4/14/99: Skip mounts with riders. They are moved with the
	  // rider.
	  // Mathew 7/6/99: Fix mount problem
	  if (mount_reset)
	    {
	      mount_reset = 0;      // Reset the flag.
	      fch = fch_mount_next;
	    }
	  if (fch != NULL)
	    {
	      fch_next = fch->next_in_room;
	    }

            else
              {
                fch_next = NULL;      // Mathew 7/6/99: Skip NULL fch's due to mount resets
                continue;
              }
	  if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
	      && fch->position < POS_STANDING)
	    do_stand (fch, "");
	  if ((fch->master == ch || fch->stalking == ch)
                && fch->position >= POS_STANDING && fch->position < POS_FEIGNING_DEATH && fch->in_room != ch->in_room)
	    {
	      if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
		  && (IS_NPC (fch) && is_aggro (fch)))
		{
		  act ("You can't bring $N into the city.", ch, NULL, fch,
		       TO_CHAR);
		  act ("You aren't allowed in the city.", fch, NULL, NULL,
		       TO_CHAR);

		  continue;
		}

	      //5-10-03 Iblis - Check to remove camoflauge/hide if following someone
	      if (IS_AFFECTED (fch, AFF_CAMOUFLAGE))
		REMOVE_BIT (fch->affected_by, AFF_CAMOUFLAGE);
	      if (IS_AFFECTED (fch, AFF_HIDE))
		REMOVE_BIT (fch->affected_by, AFF_HIDE);
	      act ("You follow $N.", fch, NULL, ch, TO_CHAR);
	      if (fch_next != NULL && fch->mount != NULL)
		{
		  mount_reset++;    // Mathew 7/6/99: Reset to top person in
		  // old room if a rider and a mount move.
		  if (fch->next_in_room != NULL
		      && fch->next_in_room == fch->mount)
		    {
		      fch_mount_next = fch->mount->next_in_room;    // Skip the mount.
		    }

                    else
                      {
                        fch_mount_next = fch->next_in_room;   // Preserve old list.
                      }
		}
	      move_char (fch, door, TRUE);
	    }
	}
      return;
    }


  /* Move Aggie Mobs */
  for (fch = in_room->people; fch != NULL; fch = fch->next_in_room)
    {
      if (IS_NPC (fch) && is_hunting (fch, ch) && !IS_IMMORTAL (ch)
          && get_position(fch) > POS_SLEEPING && (IS_SET (fch->act, ACT_HUNT)
						  || IS_SET (fch->act,
							     ACT_SMART_HUNT))
          && can_see (fch, ch)
          && number_percent () < 55 && fch->in_room->exit[door] != NULL
          && !is_campsite (fch->in_room->exit[door]->u1.to_room)
          && !IS_SET (fch->in_room->exit[door]->u1.to_room->room_flags,
                      ROOM_NO_MOB))
        {
          if (fch->position < POS_STANDING)
            do_stand (fch, "");
          WAIT_STATE (fch, 1 * PULSE_VIOLENCE);
          act ("You hunt $N", fch, NULL, ch, TO_CHAR);
          move_char (fch, door, TRUE);
        }
    }


  trap_check(ch,"room",ch->in_room,NULL);  
  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  sprintf(tcbuf,"%d",ch->in_room->vnum);
  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
  
  if (HAS_SCRIPT (ch))
    {
      TRIGGER_DATA *pTrig;
      for (pTrig = ch->triggers; pTrig != NULL; pTrig = pTrig->next)
	{
	  if (pTrig->trigger_type == TRIG_MOVES
	      && pTrig->current == NULL && !IS_SET (ch->act, ACT_HALT))
	    {
	      act_trigger (ch, pTrig->name, NULL, NAME (ch), NULL);
	      pTrig->bits = SCRIPT_ADVANCE;
	      pTrig->current = pTrig->script;
	    }
	}
    }
  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
    {

      CHAR_DATA *bitch;
      fch_next = fch->next_in_room;
      if (!IS_NPC (ch) && IS_NPC (fch) && fch->level >= 60
	  && number_range (0, 35) == 0
	  && fch->spec_fun == spec_lookup ("spec_cast_cleric"))
	{
	  for (bitch = fch->in_room->people; bitch != NULL;
	       bitch = bitch->next_in_room)
	    {
	      if (is_same_group (bitch, fch) || bitch == fch)
		{
		  int sn;
		  if ((sn = skill_lookup ("sanctuary")) >= 0)
		    (*skill_table[sn].spell_fun)
		      (sn, fch->level, fch, bitch, TARGET_CHAR);
		  break;
		}
	    }
	}
      if (!IS_NPC (ch) && IS_NPC (fch) && number_range (0, 13) == 0
	  && fch->spec_fun == spec_lookup ("spec_cast_mage"))
	{
	  int sn;
	  if ((sn = skill_lookup ("armor")) >= 0)
	    (*skill_table[sn].spell_fun)
	      (sn, fch->level, fch, fch, TARGET_CHAR);
	}
      if (!IS_NPC (ch) && IS_NPC (fch) && number_range (0, 20) == 0
	  && fch->spec_fun == spec_lookup ("spec_cast_mage"))
	{
	  int sn;
	  if ((sn = gsn_fireshield) >= 0)
	    (*skill_table[sn].spell_fun)
	      (sn, fch->level, fch, fch, TARGET_CHAR);
	}
      // Check for Aggie Mob attack. Mathew 4/14/99
      if (IS_NPC (fch) && (IS_SET (fch->act, ACT_HUNT)
			   || IS_SET (fch->act, ACT_REMEMBER)
			   || IS_SET (fch->act, ACT_SMART_HUNT))
	  && is_hunting (fch, ch) && can_see (fch, ch)
	  && !IS_AFFECTED (ch, AFF_CAMOUFLAGE) && !IS_IMMORTAL (ch)
	  && get_position(fch) > POS_SLEEPING)
	{
	  if (fch->position < POS_STANDING)
	    do_stand (fch, "");
	  do_kill (fch, NAME (ch));
	}
      if (IS_NPC (ch)
	  && (IS_SET (ch->act, ACT_HUNT)
	      || IS_SET (ch->act, ACT_REMEMBER)
	      || IS_SET (ch->act, ACT_SMART_HUNT)) && is_hunting (ch, fch)
	  && !IS_AFFECTED (fch, AFF_CAMOUFLAGE) && !IS_IMMORTAL (fch)
	  && can_see (ch, fch) &&  get_position(ch) > POS_SLEEPING)
	{
	  if (ch->position < POS_STANDING)
	    do_stand (ch, "");
	  do_kill (ch, NAME (fch));
	}
    }
  if (ch->mount != NULL)
    {
      int chknum;
      if (!IS_NPC (ch))
	chknum = get_skill (ch, gsn_riding) + 5;

      else
	chknum = 99;
      if (number_percent () < chknum)
	{
	  check_improve (ch, gsn_riding, TRUE, 7);
	}

      else
	{
	  char tcbuf[MAX_STRING_LENGTH];
	  sprintf (tcbuf,
		   "You fall off of %s, landing hard on the ground.\n\r",
		   ch->mount->short_descr);
	  send_to_char (tcbuf, ch);
	  sprintf (tcbuf,
		   "$n falls off of %s, landing hard on the ground.",
		   ch->mount->short_descr);
	  act (tcbuf, ch, NULL, NULL, TO_ROOM);
	  ch->position = POS_SITTING;
	  WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
	  ch->mount->riders = NULL;
	  ch->mount = NULL;
	}
    }
//Minax 7-10-02 Check for a tripwire in the room.
  for (tObj = ch->in_room->contents; tObj; tObj = tObj->next_content)
    {
      if (tObj->pIndexData->vnum == OBJ_VNUM_TRIPWIRE
	  && ch->level < 92 && !IS_NPC (ch))
	{
	  if ((number_percent () / 2) > get_curr_stat (ch, STAT_INT)
	      && ch->position > POS_RESTING)
	    {
	      if (ch->mount != NULL)
		{
		  sprintf (tcbuf,
			   "%s trips over $p and $n falls off, landing flat on $s face.",
			   ch->mount->short_descr);
		  act (tcbuf, ch, tObj, NULL, TO_ROOM);
		  sprintf (tcbuf,
			   "%s trips over $p and you falls off, landing flat on your face.",
			   ch->mount->short_descr);
		  act (tcbuf, ch, tObj, NULL, TO_CHAR);
		  ch->mount->riders = NULL;
		  ch->mount = NULL;
		  ch->position = POS_RESTING;
		}

	      else
		{
		  sprintf (tcbuf,
			   "You trip over $p and fall flat on your face!");
		  act (tcbuf, ch, tObj, NULL, TO_CHAR);
		  sprintf (tcbuf,
			   "$n trips over $p and falls flat on $s face!");
		  act (tcbuf, ch, tObj, NULL, TO_ROOM);
		  ch->position = POS_RESTING;
		}
	      WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
	      if (number_percent () < (ch->level / 3))
		{
		  sprintf (tcbuf,
			   "$p is ripped out of the ground and destroyed.");
		  act (tcbuf, ch, tObj, NULL, TO_CHAR);
		  sprintf (tcbuf,
			   "$p tripwire is ripped out of the ground and destroyed.");
		  act (tcbuf, ch, tObj, NULL, TO_ROOM);
		  extract_obj (tObj);
		}
	    }
	}
    }
  if (IS_SET (to_room->room_flags, ROOM_SPECTATOR))
    {
      if (IS_IMMORTAL (ch))
	send_to_char ("The usher politely welcomes you.\n\r", ch);

      else
	send_to_char
	  ("The usher rudely grabs your ticket, ripping it in half.\n\r", ch);
    }
  if (in_room == to_room)	// no circular follows
  {
    if (!no_check_skeletal)
    {
    	check_aggression(ch);
    	check_skeletal_mutation(ch,FALSE);
    }
    return;
  }

  // Move a group if ch was the group leader. 4/14/99 Mathew
  mount_reset = 0;
  fch_mount_next = NULL;
  for (fch = in_room->people; fch != NULL; fch = fch_next)
    {

      // Mathew 4/14/99: Skip mounts with riders. They are moved with the
      // rider.
      // Mathew 7/6/99: Fix mount problem
      if (mount_reset)
	{
	  mount_reset = 0;	// Reset the flag.
	  fch = fch_mount_next;
	}
      if (fch != NULL)
	{
	  fch_next = fch->next_in_room;
	}

      else
	{
	  fch_next = NULL;	// Mathew 7/6/99: Skip NULL fch's due to mount resets
	  continue;
	}
      if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
	  && fch->position < POS_STANDING)
	do_stand (fch, "");
      if ((fch->master == ch || fch->stalking == ch)
	  && fch->position >= POS_STANDING && fch->position < POS_FEIGNING_DEATH && fch->in_room != ch->in_room)
	{
	  if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
	      && (IS_NPC (fch) && is_aggro (fch)))
	    {
	      act ("You can't bring $N into the city.", ch, NULL, fch,
		   TO_CHAR);
	      act ("You aren't allowed in the city.", fch, NULL, NULL,
		   TO_CHAR);
	      continue;
	    }
	  //5-10-03 Iblis - Check to remove camoflauge/hide if following someone
	  if (IS_AFFECTED (fch, AFF_CAMOUFLAGE))
	    REMOVE_BIT (fch->affected_by, AFF_CAMOUFLAGE);
	  if (IS_AFFECTED (fch, AFF_HIDE))
	    REMOVE_BIT (fch->affected_by, AFF_HIDE);
	  act ("You follow $N.", fch, NULL, ch, TO_CHAR);
	  if (fch_next != NULL && fch->mount != NULL)
	    {
	      mount_reset++;	// Mathew 7/6/99: Reset to top person in
	      // old room if a rider and a mount move.
	      if (fch->next_in_room != NULL
		  && fch->next_in_room == fch->mount)
		{
		  fch_mount_next = fch->mount->next_in_room;	// Skip the mount.
		}

	      else
		{
		  fch_mount_next = fch->next_in_room;	// Preserve old list.
		}
	    }
	  move_char (fch, door, TRUE);
	}
    }
  if (ch->tracking[0] != '\0')
    display_track (ch);
  if (!IS_SET (ch->comm, COMM_NOINFO))
    {
      if (in_room && to_room && in_room->area != to_room->area)
      {
         if (to_room->area->llev > ch->level)
         {
           send_to_char("`iWarning : `bThis area is OOL for you`i!!!``\n\r",ch);
         }
      }
    }
  if (!no_check_skeletal)
  {
  	check_aggression(ch);
        check_skeletal_mutation(ch,FALSE);
  }
}

void do_mount (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  CHAR_DATA *mount;
  int chknum;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Mount what?\n\r", ch);
      return;
    }
  if ((mount = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (ch->race == PC_RACE_NERIX)
  {
    send_to_char("You cannot ride or mount.\n\r",ch);
    return;
  }
  if (!IS_NPC (mount))
    {
      send_to_char ("You can't mount players.\n\r", ch);
      return;
    }
  if (!IS_SET (mount->act, ACT_MOUNT))
    {
      send_to_char ("Sorry, that mobile isn't mountable.\n\r", ch);
      return;
    }
  if (mount->position == POS_TETHERED)
    {
      send_to_char
	("You must untether that creature before you can mount it.\n\r", ch);
      return;
    }
  if (mount->level - 8 > ch->level)
    {
      send_to_char
	("Sorry, that mount is too powerful for you to handle.\n\r", ch);
      return;
    }
  if (ch->mount != NULL || ch->position == POS_MOUNTED)
    {
      send_to_char ("You are already riding a mount.\n\r", ch);
      return;
    }
  if (mount->riders != NULL)
    {
      send_to_char ("Sorry, someone is already riding that mobile.\n\r", ch);
      return;
    }
  if (mount->mounter != NULL && mount->mounter != ch)
    {
      send_to_char ("Sorry, that mount belongs to someone else.\n\r", ch);
      return;
    }
  if (mount->afkmsg != NULL && mount->afkmsg[0] != '\0'
      && str_cmp (mount->afkmsg, ch->name))
    {
      send_to_char ("Sorry, that mount belongs to someone else.\n\r", ch);
      return;
    }
  if (mount->position != POS_STANDING && mount->position != POS_FIGHTING)
    {
      sprintf (tcbuf,
	       "Sorry, you'll have to get %s to stand before you can mount it.\n\r",
	       mount->short_descr);
      send_to_char (tcbuf, ch);
      return;
    }
  if (get_char_weight (ch) > mount->pIndexData->max_weight)
    {
      sprintf (tcbuf,
	       "You're too damn heavy, you'll break %s's back!\n\r",
	       mount->short_descr);
      send_to_char (tcbuf, ch);
      return;
    }
  if (!IS_NPC (ch))
    chknum = get_skill (ch, gsn_riding);

  else
    chknum = 98;
  if (number_percent () < chknum)
    {
      sprintf (tcbuf, "You skillfully mount %s.\n\r", mount->short_descr);
      send_to_char (tcbuf, ch);
      sprintf (tcbuf, "$n skillfully mounts %s.", mount->short_descr);
      act (tcbuf, ch, NULL, NULL, TO_ROOM);
      check_improve (ch, gsn_riding, TRUE, 1);
      new_master (ch, mount);
      ch->position = POS_MOUNTED;
    }

  else
    {
      sprintf (tcbuf,
	       "You attempt to mount %s, but fall to the ground.\n\r",
	       mount->short_descr);
      send_to_char (tcbuf, ch);
      sprintf (tcbuf,
	       "$n attempts to mount %s, but falls to the ground.",
	       mount->short_descr);
      act (tcbuf, ch, NULL, NULL, TO_ROOM);
      ch->position = POS_SITTING;
      check_improve (ch, gsn_riding, FALSE, 1);
    }
}
void do_tether (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  CHAR_DATA *mount;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Tether what?\n\r", ch);
      return;
    }
  if ((mount = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (!IS_NPC (mount))
    {
      send_to_char ("You can't tether players.\n\r", ch);
      return;
    }
  if (!IS_SET (mount->act, ACT_MOUNT))
    {
      send_to_char ("Sorry, that mobile cannot be tethered.\n\r", ch);
      return;
    }
  if (mount->riders != NULL)
    {
      send_to_char ("Sorry, someone is riding that mobile.\n\r", ch);
      return;
    }
  if (mount->afkmsg != NULL && mount->afkmsg[0] != '\0'
      && str_cmp (mount->afkmsg, ch->name))
    {
      send_to_char ("Sorry, that mount belongs to someone else.\n\r", ch);
      return;
    }
  if (get_position(mount) == POS_SLEEPING)
    {
      sprintf (tcbuf, "%s must be awake before you can tether it.\n\r",
	       mount->short_descr);
      send_to_char (tcbuf, ch);
      return;
    }
  sprintf (tcbuf, "You tether %s.\n\r", mount->short_descr);
  send_to_char (tcbuf, ch);
  sprintf (tcbuf, "$n tethers %s.", mount->short_descr);
  act (tcbuf, ch, NULL, NULL, TO_ROOM);
  mount->mounter = NULL;
  mount->leader = NULL;
  mount->position = POS_TETHERED;
}

void do_untether (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  CHAR_DATA *mount;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Untether what?\n\r", ch);
      return;
    }
  if ((mount = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (mount->position != POS_TETHERED)
    {
      send_to_char ("That mobile is not tethered.\n\r", ch);
      return;
    }
  if (mount->afkmsg != NULL && mount->afkmsg[0] != '\0'
      && str_cmp (mount->afkmsg, ch->name))
    {
      send_to_char ("Sorry, that mount belongs to someone else.\n\r", ch);
      return;
    }
  mount->position = POS_STANDING;
  sprintf (tcbuf, "You untether %s.\n\r", mount->short_descr);
  send_to_char (tcbuf, ch);
  sprintf (tcbuf, "$n untethers %s.", mount->short_descr);
  act (tcbuf, ch, NULL, NULL, TO_ROOM);
  mount->leader = ch;
  mount->mounter = ch;
}

void new_master (CHAR_DATA * ch, CHAR_DATA * mount)
{
  ch->mount = mount;
  mount->riders = ch;
  mount->leader = ch;
  mount->mounter = ch;
  mount->master = ch;
}

void do_stable (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  CHAR_DATA *keeper, *mount;
  if (IS_NPC (ch))
    return;
  one_argument (argument, arg);
  if (!IS_SET (ch->in_room->room_flags, ROOM_STABLE))
    {
      send_to_char ("But this isn't a stable!\n\r", ch);
      return;
    }
  if ((keeper = find_keeper (ch)) == NULL)
    {
      send_to_char
	("The stable keeper seems to have stepped out for a bit.\n\r", ch);
      return;
    }
  if ((mount = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("That mobile is not here.\n\r", ch);
      return;
    }
  if (!IS_SET (mount->act, ACT_MOUNT) || mount->mounter != ch)
    {
      send_to_char ("That mobile is not yours to board.\n\r", ch);
      return;
    }
  if (mount->riders != NULL)
    {
      send_to_char ("You might want to dismount that mobile first.\n\r", ch);
      return;
    }
  mount->wiznet = time (NULL);
  mount->next_in_board = ch->pcdata->boarded;
  mount->lines = ch->in_room->vnum;
  mount->leader = NULL;
  ch->pcdata->boarded = mount;
  ch->mount = NULL;
  char_from_room (mount);
  char_to_room (mount, get_room_index (ROOM_VNUM_SLIMBO));
  sprintf (tcbuf,
	   "The stablekeeper takes %s and leads it back into the stalls.\n\r",
	   mount->short_descr);
  send_to_char (tcbuf, ch);
}

void do_retrieve (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  CHAR_DATA *keeper, *mount, *prev_mount = NULL;
  bool found = FALSE;
  if (IS_NPC (ch))
    return;
  one_argument (argument, arg);
  if (!IS_SET (ch->in_room->room_flags, ROOM_STABLE))
    {
      send_to_char ("But this isn't a stable!\n\r", ch);
      return;
    }
  if ((keeper = find_keeper (ch)) == NULL)
    {
      send_to_char
	("The stable keeper seems to have stepped out for a bit.\n\r", ch);
      return;
    }
  for (mount = ch->pcdata->boarded; mount != NULL;
       mount = mount->next_in_board)
    {
      if (mount->lines == ch->in_room->vnum && is_name (arg, mount->name))
	{
	  int price = UMAX ((time (NULL) - mount->wiznet) / 86400, 1)
	    * mount->level * 40;
	  if ((ch->silver + 100 * ch->gold) < price)
	    {
	      send_to_char
		("The stablekeeper says 'You can't afford to retrieve your mount right now'.\n\r",
		 ch);
	      return;
	    }
	  deduct_cost (ch, price);
	  sprintf (tcbuf,
		   "The stablekeeper goes to the stalls, and leads %s out for you.\n\r",
		   mount->short_descr);
	  send_to_char (tcbuf, ch);
	  found = TRUE;
	  if (prev_mount != NULL)
	    prev_mount->next_in_board = mount->next_in_board;

	  else
	    ch->pcdata->boarded = ch->pcdata->boarded->next_in_board;
	  char_from_room (mount);
	  char_to_room (mount, ch->in_room);
	  break;
	}
      prev_mount = mount;
    }
  if (!found)
    send_to_char ("No such mount has been boarded here.\n\r", ch);
}

void do_tame (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  if (IS_NPC (ch))
    return;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Tame what?\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (!IS_NPC (victim))
    {
      send_to_char ("You can't tame players.\n\r", ch);
      return;
    }
  if (IS_SET (victim->act, ACT_MOUNT))
    {
      send_to_char ("That mount is already tamed.\n\r", ch);
      return;
    }
  if (!IS_SET (victim->act, ACT_TAME))
    {
      send_to_char ("You may not tame this target.\n\r", ch);
      return;
    }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (IS_NPC (victim) && (IS_SET (victim->act, ACT_TRAIN)
			  || IS_SET (victim->act, ACT_PRACTICE)
			  || IS_SET (victim->act, ACT_IS_HEALER)))
    {
      send_to_char ("I don't think so.\n\r", ch);
      return;
    }
  if (IS_AFFECTED (victim, AFF_CHARM))
    {
      if (victim->master == ch)
	send_to_char ("That mobile is currently your pet.\n\r", ch);

      else
	send_to_char ("That mobile is someone else's pet.\n\r", ch);
      return;
    }
  sprintf (tcbuf, "You attempt to tame %s.", victim->short_descr);
  act (tcbuf, ch, NULL, victim, TO_CHAR);
  sprintf (tcbuf, "$n attempts to tame %s.", victim->short_descr);
  act (tcbuf, ch, NULL, victim, TO_ROOM);
  if (get_skill (ch, gsn_tame) == 0)
    {
      if (number_percent () < 75)
	{
	  sprintf (tcbuf, "%s eyes you warily and backs away.",
		   fcapitalize (victim->short_descr));
	  act (tcbuf, ch, NULL, victim, TO_CHAR);
	  sprintf (tcbuf, "%s eyes $n warily and backs away.",
		   fcapitalize (victim->short_descr));
	  act (tcbuf, ch, NULL, victim, TO_ROOM);
	  if (number_percent () < 50)
	    {
	      int attempt;
	      for (attempt = 0; attempt < 6; attempt++)
		{
		  EXIT_DATA *pexit;
		  int door;
		  door = number_door ();
		  if ((pexit = victim->in_room->exit[door]) == 0
		      || pexit->u1.to_room == NULL
		      || IS_SET (pexit->exit_info, EX_CLOSED)
		      || number_range (0, victim->daze) != 0
		      || (IS_NPC (ch) &&
			  (IS_SET
			   (pexit->u1.to_room->room_flags,
			    ROOM_NO_MOB) || is_campsite (pexit->u1.
							 to_room))
			  && !IS_SET (ch->act, ACT_SMART_HUNT)))
		    continue;
		  move_char (victim, door, TRUE);
		}
	    }
	  return;
	}

      else
	{
	  sprintf (tcbuf, "%s becomes agitated and attacks.",
		   capitalize (victim->short_descr));
	  act (tcbuf, victim, NULL, NULL, TO_ROOM);

	  /*      set_fighting(victim, ch); */
	  multi_hit (victim, ch, TYPE_UNDEFINED);
	  return;
	}
    }
  if (number_percent () < get_skill (ch, gsn_tame)
      && victim->level <= ch->level)
    {
      if (number_percent () < 20)
	{
	  sprintf (tcbuf, "%s takes a liking to you.\n\r",
		   capitalize (victim->short_descr));
	  send_to_char (tcbuf, ch);
	  sprintf (tcbuf, "%s takes a liking to $n.",
		   capitalize (victim->short_descr));
	  act (tcbuf, ch, NULL, victim, TO_ROOM);
	  check_improve (ch, gsn_tame, TRUE, 1);
	  victim->master = ch;
	  victim->mounter = ch;
	  return;
	}

      else
	{
	  REMOVE_BIT (victim->act, ACT_AGGRESSIVE);
	  SET_BIT (victim->act, ACT_MOUNT);
	  sprintf (tcbuf, "You tame %s.\n\r", victim->short_descr);
	  send_to_char (tcbuf, ch);
	  sprintf (tcbuf, "$n tames %s.", victim->short_descr);
	  act (tcbuf, ch, NULL, NULL, TO_ROOM);
	  victim->master = ch;
	  victim->mounter = ch;
	  return;
	}
    }

  else
    {
      if (number_percent () < 75)
	{
	  sprintf (tcbuf, "%s eyes you warily and backs away.",
		   fcapitalize (victim->short_descr));
	  act (tcbuf, ch, NULL, victim, TO_CHAR);
	  sprintf (tcbuf, "%s eyes $n warily and backs away.",
		   fcapitalize (victim->short_descr));
	  act (tcbuf, ch, NULL, victim, TO_ROOM);
	  if (number_percent () < 50)
	    {
	      int attempt;
	      for (attempt = 0; attempt < 6; attempt++)
		{
		  EXIT_DATA *pexit;
		  int door;
		  door = number_door ();
		  if ((pexit = victim->in_room->exit[door]) == 0
		      || pexit->u1.to_room == NULL
		      || IS_SET (pexit->exit_info, EX_CLOSED)
		      || number_range (0, victim->daze) != 0
		      || (IS_NPC (ch)
			  &&
			  ((IS_SET
			    (pexit->u1.to_room->room_flags,
			     ROOM_NO_MOB) || is_campsite (pexit->u1.
							  to_room))
			   && !IS_SET (ch->act, ACT_SMART_HUNT))))
		    continue;
		  move_char (victim, door, TRUE);
		}
	    }
	}

      else
	{
	  sprintf (tcbuf, "%s becomes agitated and attacks.",
		   capitalize (victim->short_descr));
	  act (tcbuf, victim, NULL, NULL, TO_ROOM);
	  set_fighting (victim, ch);
	}
      check_improve (ch, gsn_tame, FALSE, 1);
    }
}
void do_dismount (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_STRING_LENGTH];
  if (ch->mount == NULL)
    {
      send_to_char ("You don't currently have a mount.\n\r", ch);
      return;
    }
  sprintf (tcbuf, "You dismount from %s.\n\r", ch->mount->short_descr);
  send_to_char (tcbuf, ch);
  sprintf (tcbuf, "$n dismounts %s.", ch->mount->short_descr);
  act (tcbuf, ch, NULL, NULL, TO_ROOM);
  ch->mount->riders = NULL;
  ch->mount = NULL;
  if (ch->fighting != NULL)
    {
      ch->position = POS_FIGHTING;
    }

  else
    {
      ch->position = POS_STANDING;
    }
}
void do_speedwalk (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char revarg[MAX_INPUT_LENGTH];
  char dirstr[2];
  int xcnt, ycnt;
  if (IS_NPC (ch))
    {
      send_to_char ("Mobiles can not speedwalk.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Which way(s) do you want to go?\n\r", ch);
      return;
    }

  for (xcnt = strlen (arg) - 1, ycnt = 0; xcnt > -1; xcnt--, ycnt++)
    {
      switch (arg[xcnt])
	{
	default:
	  send_to_char ("Directional string not understood.\n\r", ch);
	  return;
	case 'n':
	case 'N':
	case 's':
	case 'S':
	case 'e':
	case 'E':
	case 'w':
	case 'W':
	case 'd':
	case 'D':
	case 'u':
	case 'U':
	  break;
	}
      revarg[ycnt] = arg[xcnt];
    }
  dirstr[1] = '\0';
  dirstr[0] = revarg[ycnt - 1];
  interpret (ch, dirstr);
  revarg[ycnt - 1] = '\0';
  if (revarg[0] != '\0')
    {
      free_string (ch->pcdata->speedwalk);
      ch->pcdata->speedwalk = str_dup (revarg);
      ch->pcdata->speedlen = strlen (revarg);
    }
}
void do_gallop (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char revarg[MAX_INPUT_LENGTH];
  char dirstr[2];
  int xcnt, ycnt;
  if (IS_NPC (ch))
    {
      send_to_char ("Mobiles can not gallop.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Which way(s) do you want to gallop?\n\r", ch);
      return;
    }
  if (ch->mount == NULL)
    {
      send_to_char ("You must have a mount to gallop.\n\r", ch);
      return;
    }
  for (xcnt = strlen (arg) - 1, ycnt = 0; xcnt > -1; xcnt--, ycnt++)
    {
      switch (arg[xcnt])
	{
	default:
	  send_to_char ("Gallop string not understood.\n\r", ch);
	  return;
	case 'n':
	case 'N':
	case 's':
	case 'S':
	case 'e':
	case 'E':
	case 'w':
	case 'W':
	case 'd':
	case 'D':
	case 'u':
	case 'U':
	  break;
	}
      revarg[ycnt] = arg[xcnt];
    }
  dirstr[1] = '\0';
  dirstr[0] = revarg[ycnt - 1];
  interpret (ch, dirstr);
  revarg[ycnt - 1] = '\0';
  if (revarg[0] != '\0')
    {
      free_string (ch->pcdata->speedwalk);
      ch->pcdata->speedwalk = str_dup (revarg);
      ch->pcdata->speedlen = strlen (revarg);
    }
}
void do_north (CHAR_DATA * ch, char *argument)
{
  if (ch->position > POS_FIGHTING && ch->position < POS_FEIGNING_DEATH && ch->fighting == NULL)
    move_char (ch, DIR_NORTH, FALSE);
  return;
}

void do_east (CHAR_DATA * ch, char *argument)
{

  //IBLIS 6/28/03 - Yes a VERY VERY CHEAP fix to stop people from going back into the safe rooms
  //near the altar during BR..so sue me, its late and this works
  if (!(battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner)))
      || (ch->in_room != NULL && ch->in_room->vnum != 4149
	  && ch->in_room->vnum != 4405 && ch->in_room->vnum != 4406
	  && ch->in_room->vnum != 4404))
    {
      if (ch->position > POS_FIGHTING && ch->fighting == NULL && ch->position < POS_FEIGNING_DEATH)
	move_char (ch, DIR_EAST, FALSE);

      else if (ch->in_room != NULL)
	{
	  send_to_char ("You can't go back there now.\r\n", ch);
	}
    }
  return;
}

void do_south (CHAR_DATA * ch, char *argument)
{
  if (ch->position > POS_FIGHTING && ch->fighting == NULL  && ch->position < POS_FEIGNING_DEATH)
    move_char (ch, DIR_SOUTH, FALSE);
  return;
}

void do_west (CHAR_DATA * ch, char *argument)
{
  if (ch->position > POS_FIGHTING && ch->fighting == NULL  && ch->position < POS_FEIGNING_DEATH)
    move_char (ch, DIR_WEST, FALSE);
  return;
}

void do_up (CHAR_DATA * ch, char *argument)
{
  if (ch->position > POS_FIGHTING && ch->fighting == NULL  && ch->position < POS_FEIGNING_DEATH)
    move_char (ch, DIR_UP, FALSE);
  return;
}

void do_down (CHAR_DATA * ch, char *argument)
{
  if (ch->position > POS_FIGHTING && ch->fighting == NULL  && ch->position < POS_FEIGNING_DEATH)
    move_char (ch, DIR_DOWN, FALSE);
  return;
}

void do_doorbash (CHAR_DATA * ch, char *argument)
{
  char door_name[MAX_INPUT_LENGTH];
  EXIT_DATA *pexit = NULL;
  short x, door = 0, chance;
  CHAR_DATA *vch;
  OBJ_DATA *obj;
  one_argument (argument, door_name);
  if (door_name[0] == '\0')
    {
      send_to_char ("Bash which door?\n\r", ch);
      return;
    }
  if (!str_cmp (door_name, "n") || !str_cmp (door_name, "north"))
    {
      door = 0;
      pexit = ch->in_room->exit[0];
    }

  else if (!str_cmp (door_name, "e") || !str_cmp (door_name, "east"))
    {
      door = 1;
      pexit = ch->in_room->exit[1];
    }

  else if (!str_cmp (door_name, "s") || !str_cmp (door_name, "south"))
    {
      door = 2;
      pexit = ch->in_room->exit[2];
    }

  else if (!str_cmp (door_name, "w") || !str_cmp (door_name, "west"))
    {
      door = 3;
      pexit = ch->in_room->exit[3];
    }

  else if (!str_cmp (door_name, "u") || !str_cmp (door_name, "up"))
    {
      door = 4;
      pexit = ch->in_room->exit[4];
    }

  else if (!str_cmp (door_name, "d") || !str_cmp (door_name, "down"))
    {
      door = 5;
      pexit = ch->in_room->exit[5];
    }
  if (pexit == NULL)
    for (x = 0; x < MAX_DIR; x++)
      if (ch->in_room->exit[x] != NULL
	  && ch->in_room->exit[x]->keyword != NULL
	  && ch->in_room->exit[x]->keyword[0] != '\0')
	if (!str_cmp (ch->in_room->exit[x]->keyword, door_name))
	  {
	    door = x;
	    pexit = ch->in_room->exit[x];
	    break;
	  }
  chance = get_skill (ch, gsn_doorbash) - 24;
  if (pexit != NULL)
    {
      if (!IS_SET (pexit->rs_flags, EX_ISDOOR)
	  && !IS_SET (pexit->exit_info, EX_ISDOOR))
	{
	  send_to_char ("There isn't a door there.\n\r", ch);
	  return;
	}
      if (!IS_SET (pexit->exit_info, EX_CLOSED))
	{
	  if (pexit->keyword[0] == '\0')
	    send_to_char ("That door isn't closed.\n\r", ch);

	  else
	    act ("The $T isn't closed.", ch, NULL, pexit->keyword, TO_CHAR);
	  return;
	}
      if (IS_SET (pexit->rs_flags, EX_NOBASH)
	  || IS_SET (pexit->exit_info, EX_NOBASH)
	  || chance + get_curr_stat (ch, STAT_STR) < number_percent ())
	{
	  if (pexit->keyword[0] != '\0')
	    {
	      act ("You attempt to smash down the $T, but to no avail.",
		   ch, NULL, pexit->keyword, TO_CHAR);
	      act ("$n attempts to smash down the $T, but to no avail.",
		   ch, NULL, pexit->keyword, TO_ROOM);
	      if (pexit->u1.to_room !=
		  NULL && pexit->u1.to_room->exit[rev_dir[door]] !=
		  NULL && pexit->u1.to_room->exit[rev_dir[door]]->u1.
		  to_room == ch->in_room)
		for (vch = pexit->u1.to_room->people; vch;
		     vch = vch->next_in_room)
		  act ("You hear someone banging against the $T.",
		       vch, NULL, pexit->keyword, TO_CHAR);
	    }

	  else
	    {
	      act
		("You attempt to smash down the door $T from here, but to no avail.",
		 ch, NULL, dir_name[door], TO_CHAR);
	      act
		("$n attempts to smash down the door $T from here, but to no avail.",
		 ch, NULL, dir_name[door], TO_ROOM);
	      if (pexit->u1.to_room != NULL
		  && pexit->u1.to_room->exit[rev_dir[door]] != NULL
		  && pexit->u1.to_room->exit[rev_dir[door]]->u1.to_room ==
		  ch->in_room)
		for (vch = pexit->u1.to_room->people; vch;
		     vch = vch->next_in_room)
		  act
		    ("You hear someone banging against the door $T from here.",
		     vch, NULL, dir_name[rev_dir[door]], TO_CHAR);
	    }
	  check_improve (ch, gsn_doorbash, FALSE, 1);
	  return;
	}

      else
	{
	  REMOVE_BIT (pexit->exit_info, EX_ISDOOR);
	  REMOVE_BIT (pexit->exit_info, EX_CLOSED);
	  REMOVE_BIT (pexit->exit_info, EX_LOCKED);
	  SET_BIT (pexit->exit_info, EX_BASHED);
	  if (pexit->u1.to_room->exit[rev_dir[door]] != NULL)
	    {
	      REMOVE_BIT (pexit->u1.to_room->exit[rev_dir[door]]->
			  exit_info, EX_ISDOOR);
	      REMOVE_BIT (pexit->u1.to_room->exit[rev_dir[door]]->
			  exit_info, EX_CLOSED);
	      REMOVE_BIT (pexit->u1.to_room->exit[rev_dir[door]]->
			  exit_info, EX_LOCKED);
	      SET_BIT (pexit->u1.to_room->exit[rev_dir[door]]->exit_info,
		       EX_BASHED);
	    }
	  if (pexit->keyword[0] != '\0')
	    {
	      act ("Using your brute strength, you smash down the $T.",
		   ch, NULL, pexit->keyword, TO_CHAR);
	      act ("Using $s brute strength, $n smashes down the $T.",
		   ch, NULL, pexit->keyword, TO_ROOM);
	      if (pexit->u1.to_room !=
		  NULL && pexit->u1.to_room->exit[rev_dir[door]] !=
		  NULL && pexit->u1.to_room->exit[rev_dir[door]]->u1.
		  to_room == ch->in_room)
		for (vch = pexit->u1.to_room->people; vch;
		     vch = vch->next_in_room)
		  act ("The $T is smashed from the other side.", vch,
		       NULL, pexit->keyword, TO_CHAR);
	      if (get_eq_char (ch, WEAR_HEAD) == NULL)
		{
		  float hurts;
		  act
		    ("Your bare head smashes through the $T, causing a great deal of pain.",
		     ch, NULL, pexit->keyword, TO_CHAR);
		  hurts = ch->hit * 0.10;
		  damage_old (ch, ch, (int) hurts, 0, DAM_OTHER, FALSE);
		}
	    }

	  else
	    {
	      act
		("Using your brute strength, you smash down the door $T from here.",
		 ch, NULL, dir_name[door], TO_CHAR);
	      act
		("Using $s brute strength, $n smashes down the door $T from here.",
		 ch, NULL, dir_name[door], TO_ROOM);
	      if (pexit->u1.to_room != NULL
		  && pexit->u1.to_room->exit[rev_dir[door]] != NULL
		  && pexit->u1.to_room->exit[rev_dir[door]]->u1.to_room ==
		  ch->in_room)
		for (vch = pexit->u1.to_room->people; vch;
		     vch = vch->next_in_room)
		  act
		    ("The door $T from here is smashed from the other side.",
		     vch, NULL, dir_name[rev_dir[door]], TO_CHAR);
	      if (get_eq_char (ch, WEAR_HEAD) == NULL)
		{
		  float hurts;
		  act
		    ("Your bare head smashes through the door $T from here, causing you great pain.",
		     ch, NULL, dir_name[door], TO_CHAR);
		  hurts = ch->hit * 0.10;
		  damage_old (ch, ch, (int) hurts, 0, DAM_OTHER, FALSE);
		}
	    }
	  check_improve (ch, gsn_doorbash, TRUE, 1);
	  return;
	}
    }

  else
    {
      ROOM_INDEX_DATA *location;
      if ((obj = get_obj_here (ch, door_name)) == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
      if (obj->item_type != ITEM_CTRANSPORT)
	{
	  send_to_char ("You can't bash that.\n\r", ch);
	  return;
	}
      if (!IS_SET (obj->value[5], CONT_CLOSED))
	{
	  act ("$p is not closed.", ch, obj, NULL, TO_CHAR);
	  return;
	}
      location = get_room_index (obj->value[2]);
      if (IS_SET (obj->value[5], CONT_NOBASH)
	  || chance + get_curr_stat (ch, STAT_STR) < number_percent ())
	{
	  act
	    ("You attempt to smash open the entrance to $p, but to no avail.",
	     ch, obj, NULL, TO_CHAR);
	  act
	    ("$n attempts to smash open the entrance to $p, but to no avail.",
	     ch, obj, NULL, TO_ROOM);
	  if (location != NULL)
	    for (vch = location->people; vch; vch = vch->next_in_room)
	      act ("You hear someone banging against the entrance to $p.",
		   vch, obj, NULL, TO_CHAR);
	}

      else
	{
	  REMOVE_BIT (obj->value[5], CONT_CLOSEABLE);
	  REMOVE_BIT (obj->value[5], CONT_CLOSED);
	  REMOVE_BIT (obj->value[5], CONT_LOCKED);
	  act ("You smash open the entrance to $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n smashes open the entrance to $p.", ch, obj, NULL, TO_ROOM);
	  if (location != NULL)
	    for (vch = location->people; vch; vch = vch->next_in_room)
	      act ("Someone smashes open the entrance to $p.", vch,
		   obj, NULL, TO_CHAR);
	  if (get_eq_char (ch, WEAR_HEAD) == NULL)
	    {
	      float hurts;
	      act
		("Your bare head smashes through the entrance to $p, causing you great pain.",
		 ch, obj, NULL, TO_CHAR);
	      hurts = ch->hit * 0.10;
	      damage_old (ch, ch, (int) hurts, 0, DAM_OTHER, FALSE);
	    }
	}
      return;
    }
}
int find_door (CHAR_DATA * ch, char *arg)
{
  EXIT_DATA *pexit;
  int door;
  if (!str_cmp (arg, "n") || !str_cmp (arg, "north"))
    door = 0;

  else if (!str_cmp (arg, "e") || !str_cmp (arg, "east"))
    door = 1;

  else if (!str_cmp (arg, "s") || !str_cmp (arg, "south"))
    door = 2;

  else if (!str_cmp (arg, "w") || !str_cmp (arg, "west"))
    door = 3;

  else if (!str_cmp (arg, "u") || !str_cmp (arg, "up"))
    door = 4;

  else if (!str_cmp (arg, "d") || !str_cmp (arg, "down"))
    door = 5;

  else
    {
      for (door = 0; door <= 5; door++)
	{
	  if ((pexit = ch->in_room->exit[door]) != NULL
	      && IS_SET (pexit->exit_info, EX_ISDOOR)
	      && pexit->keyword != NULL && is_name (arg, pexit->keyword))
	    return door;
	}
      act ("You see no $T here.", ch, NULL, arg, TO_CHAR);
      return -1;
    }
  if ((pexit = ch->in_room->exit[door]) == NULL)
    {
      act ("You see no door $T here.", ch, NULL, arg, TO_CHAR);
      return -1;
    }
  if (!IS_SET (pexit->exit_info, EX_ISDOOR))
    {
      send_to_char ("You can't do that.\n\r", ch);
      return -1;
    }
  return door;
}

void do_coax (CHAR_DATA * ch, char *argument)
{
  char cmd[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  char mount_name[MAX_INPUT_LENGTH];
  CHAR_DATA *mnt;
  argument = one_argument (argument, mount_name);
  argument = one_argument (argument, cmd);
  if (mount_name[0] == '\0' || cmd[0] == '\0')
    {
      send_to_char ("Coax who into what?\n\r", ch);
      return;
    }
  if ((mnt = get_char_room (ch, mount_name)) == NULL)
    {
      sprintf (buf, "I don't see any %s here.\n\r", mount_name);
      send_to_char (buf, ch);
      return;
    }
  if (mnt->riders != ch && mnt->mounter != ch)
    {
      act ("$N seems to be ignoring you.", ch, NULL, mnt, TO_CHAR);
      return;
    }
  if (!str_cmp (cmd, "eat"))
    {
      do_eat (mnt, argument);
      send_to_char ("Ok.\n\r", ch);
    }

  else if (!str_cmp (cmd, "drink"))
    {
      do_drink (mnt, argument);
      send_to_char ("Ok.\n\r", ch);
    }

  else if (!str_cmp (cmd, "sleep"))
    {
      do_sleep (mnt, argument);
      send_to_char ("Ok.\n\r", ch);
    }

  else if (!str_cmp (cmd, "rest"))
    {
      do_rest (mnt, argument);
      send_to_char ("Ok.\n\r", ch);
    }

  else if (!str_cmp (cmd, "sit"))
    {
      do_sit (mnt, argument);
      send_to_char ("Ok.\n\r", ch);
    }

  else if (!str_cmp (cmd, "stand"))
    {
      do_stand (mnt, argument);
      send_to_char ("Ok.\n\r", ch);
    }

  else
    act ("$N doesn't seem to understand.", ch, NULL, mnt, TO_CHAR);
}

void do_open (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int door;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Open what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_here (ch, arg)) != NULL)
    {
//      trip_triggers(ch, OBJ_TRIG_OPEN, obj, NULL, OT_SPEC_NONE);
      /* open portal */
      if (obj->item_type == ITEM_PORTAL)
	{
	  if (!IS_SET (obj->value[1], EX_ISDOOR))
	    {
	      send_to_char ("You can't do that.\n\r", ch);
	      return;
	    }
	  if (!IS_SET (obj->value[1], EX_CLOSED))
	    {
	      send_to_char ("It's already open.\n\r", ch);
	      return;
	    }
	  if (IS_SET (obj->value[1], EX_LOCKED))
	    {
	      send_to_char ("It's locked.\n\r", ch);
	      return;
	    }
	  REMOVE_BIT (obj->value[1], EX_CLOSED);
	  act ("You open $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n opens $p.", ch, obj, NULL, TO_ROOM);
	  trip_triggers(ch, OBJ_TRIG_OPEN, obj, NULL, OT_SPEC_NONE);
	  return;
	}
      if (obj->item_type == ITEM_CTRANSPORT)
	{
	  if (!IS_SET (obj->value[5], CONT_CLOSED))
	    {
	      send_to_char ("It's already open.\n\r", ch);
	      return;
	    }
	  if (!IS_SET (obj->value[5], CONT_CLOSEABLE))
	    {
	      send_to_char ("You can't do that.\n\r", ch);
	      return;
	    }
	  if (IS_SET (obj->value[5], CONT_LOCKED))
	    {
	      send_to_char ("It's locked.\n\r", ch);
	      return;
	    }
	  REMOVE_BIT (obj->value[5], CONT_CLOSED);
	  act ("You open $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n opens $p.", ch, obj, NULL, TO_ROOM);
	  trip_triggers(ch, OBJ_TRIG_OPEN, obj, NULL, OT_SPEC_NONE);
	  return;
	}

      /* 'open object' */
      if (obj->item_type != ITEM_CONTAINER
	  && obj->item_type != ITEM_PACK && obj->item_type != ITEM_QUIVER)
	{
	  send_to_char ("That's not a container.\n\r", ch);
	  return;
	}
      if (!IS_SET (obj->value[1], CONT_CLOSED))
	{
	  send_to_char ("It's already open.\n\r", ch);
	  return;
	}
      if (!IS_SET (obj->value[1], CONT_CLOSEABLE))
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if (IS_SET (obj->value[1], CONT_LOCKED))
	{
	  send_to_char ("It's locked.\n\r", ch);
	  return;
	}
      REMOVE_BIT (obj->value[1], CONT_CLOSED);
      act ("You open $p.", ch, obj, NULL, TO_CHAR);
      act ("$n opens $p.", ch, obj, NULL, TO_ROOM);
      trip_triggers(ch, OBJ_TRIG_OPEN, obj, NULL, OT_SPEC_NONE);
      trap_check(ch,"object",NULL,obj);
      return;
    }

  else if (IS_SET (ch->in_room->room_flags, ROOM_TRANSPORT))
    {
      for (obj = object_list; obj; obj = obj->next)
	{
	  if (obj->item_type != ITEM_CTRANSPORT)
	    continue;
	  if (obj->value[2] == ch->in_room->vnum && is_name (arg, obj->name))
	    {
	      if (!IS_SET (obj->value[5], CONT_CLOSED))
		{
		  send_to_char ("It's already open.\n\r", ch);
		  return;
		}
	      if (!IS_SET (obj->value[5], CONT_CLOSEABLE))
		{
		  send_to_char ("You can't do that.\n\r", ch);
		  return;
		}
	      if (IS_SET (obj->value[5], CONT_LOCKED))
		{
		  send_to_char ("It's locked.\n\r", ch);
		  return;
		}
	      REMOVE_BIT (obj->value[5], CONT_CLOSED);
	      act ("You open $p.", ch, obj, NULL, TO_CHAR);
	      act ("$n opens $p.", ch, obj, NULL, TO_ROOM);
	      trip_triggers(ch, OBJ_TRIG_OPEN, obj, NULL, OT_SPEC_NONE);
	      return;
	    }
	}
    }
  if ((door = find_door (ch, arg)) >= 0)
    {

      /* 'open door' */
      ROOM_INDEX_DATA *to_room;
      EXIT_DATA *pexit;
      EXIT_DATA *pexit_rev;
      pexit = ch->in_room->exit[door];
      if (!IS_SET (pexit->exit_info, EX_CLOSED))
	{
	  send_to_char ("It's already open.\n\r", ch);
	  return;
	}
      if (IS_SET (pexit->exit_info, EX_LOCKED))
	{
	  send_to_char ("It's locked.\n\r", ch);
	  return;
	}
      REMOVE_BIT (pexit->exit_info, EX_CLOSED);
      act ("$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM);
      send_to_char ("Ok.\n\r", ch);
      trap_check(ch,flag_string (dir_flags,door),ch->in_room,NULL);

      /* open the other side */
      if ((to_room = pexit->u1.to_room) != NULL
	  && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
	  && pexit_rev->u1.to_room == ch->in_room)
	{
	  CHAR_DATA *rch;
	  REMOVE_BIT (pexit_rev->exit_info, EX_CLOSED);
	  for (rch = to_room->people; rch != NULL; rch = rch->next_in_room)
	    act ("The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR);
          trap_check(ch,flag_string (dir_flags,rev_dir[door]),to_room,NULL);
	}
      trip_triggers(ch, OBJ_TRIG_OPEN, obj, NULL, OT_SPEC_NONE);
    }
  return;
}

void do_close (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int door;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Close what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_here (ch, arg)) != NULL)
    {
      trip_triggers(ch, OBJ_TRIG_CLOSE, obj, NULL, OT_SPEC_NONE);
      /* portal stuff */
      if (obj->item_type == ITEM_PORTAL)
	{
	  if (!IS_SET (obj->value[1], EX_ISDOOR)
	      || IS_SET (obj->value[1], EX_NOCLOSE))
	    {
	      send_to_char ("You can't do that.\n\r", ch);
	      return;
	    }
	  if (IS_SET (obj->value[1], EX_CLOSED))
	    {
	      send_to_char ("It's already closed.\n\r", ch);
	      return;
	    }
	  SET_BIT (obj->value[1], EX_CLOSED);
	  act ("You close $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n closes $p.", ch, obj, NULL, TO_ROOM);
	  return;
	}
      if (obj->item_type == ITEM_CTRANSPORT)
	{
	  if (IS_SET (obj->value[5], CONT_CLOSED))
	    {
	      send_to_char ("It's already closed.\n\r", ch);
	      return;
	    }
	  if (!IS_SET (obj->value[5], CONT_CLOSEABLE))
	    {
	      send_to_char ("You can't do that.\n\r", ch);
	      return;
	    }
	  SET_BIT (obj->value[5], CONT_CLOSED);
	  act ("You close $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n closes $p.", ch, obj, NULL, TO_ROOM);
	  return;
	}

      /* 'close object' */
      if (obj->item_type != ITEM_CONTAINER
	  && obj->item_type != ITEM_PACK && obj->item_type
	  && obj->item_type != ITEM_QUIVER)
	{
	  send_to_char ("That's not a container.\n\r", ch);
	  return;
	}
      if (IS_SET (obj->value[1], CONT_CLOSED))
	{
	  send_to_char ("It's already closed.\n\r", ch);
	  return;
	}
      if (!IS_SET (obj->value[1], CONT_CLOSEABLE))
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      SET_BIT (obj->value[1], CONT_CLOSED);
      act ("You close $p.", ch, obj, NULL, TO_CHAR);
      act ("$n closes $p.", ch, obj, NULL, TO_ROOM);
      return;
    }

  else if (IS_SET (ch->in_room->room_flags, ROOM_TRANSPORT))
    {
      for (obj = object_list; obj; obj = obj->next)
	{
	  if (obj->item_type != ITEM_CTRANSPORT)
	    continue;
	  if (obj->value[2] == ch->in_room->vnum && is_name (arg, obj->name))
	    {
	      if (IS_SET (obj->value[5], CONT_CLOSED))
		{
		  send_to_char ("It's already closed.\n\r", ch);
		  return;
		}
	      if (!IS_SET (obj->value[5], CONT_CLOSEABLE))
		{
		  send_to_char ("You can't do that.\n\r", ch);
		  return;
		}
	      SET_BIT (obj->value[5], CONT_CLOSED);
	      act ("You close $p.", ch, obj, NULL, TO_CHAR);
	      act ("$n closes $p.", ch, obj, NULL, TO_ROOM);
	      return;
	    }
	}
    }
  if ((door = find_door (ch, arg)) >= 0)
    {

      /* 'close door' */
      ROOM_INDEX_DATA *to_room;
      EXIT_DATA *pexit;
      EXIT_DATA *pexit_rev;
      pexit = ch->in_room->exit[door];
      if (IS_SET (pexit->exit_info, EX_CLOSED))
	{
	  send_to_char ("It's already closed.\n\r", ch);
	  return;
	}
      SET_BIT (pexit->exit_info, EX_CLOSED);
      act ("$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM);
      send_to_char ("Ok.\n\r", ch);

      /* close the other side */
      if ((to_room = pexit->u1.to_room) != NULL
	  && (pexit_rev = to_room->exit[rev_dir[door]]) != 0
	  && pexit_rev->u1.to_room == ch->in_room)
	{
	  CHAR_DATA *rch;
	  SET_BIT (pexit_rev->exit_info, EX_CLOSED);
	  for (rch = to_room->people; rch != NULL; rch = rch->next_in_room)
	    act ("The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR);
	}
    }
  return;
}

bool has_key (CHAR_DATA * ch, int key)
{
  OBJ_DATA *obj;
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if (obj->pIndexData->vnum == key)
	return TRUE;
    }
  return FALSE;
}

bool has_ct_key (CHAR_DATA * ch, OBJ_DATA * tObj)
{
  OBJ_DATA *obj;
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if (obj->pIndexData->vnum == tObj->value[6]
	  && !str_cmp (obj->owner, tObj->owner))
	return TRUE;
    }
  return FALSE;
}

void do_lock (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int door;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Lock what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_here (ch, arg)) != NULL)
    {

      /* portal stuff */
      if (obj->item_type == ITEM_PORTAL)
	{
	  if (!IS_SET (obj->value[1], EX_ISDOOR)
	      || IS_SET (obj->value[1], EX_NOCLOSE))
	    {
	      send_to_char ("You can't do that.\n\r", ch);
	      return;
	    }
	  if (!IS_SET (obj->value[1], EX_CLOSED))
	    {
	      send_to_char ("It's not closed.\n\r", ch);
	      return;
	    }
	  if (obj->value[4] < 0 || IS_SET (obj->value[1], EX_NOLOCK))
	    {
	      send_to_char ("It can't be locked.\n\r", ch);
	      return;
	    }
	  if (!has_key (ch, obj->value[4]))
	    {
	      send_to_char ("You lack the key.\n\r", ch);
	      return;
	    }
	  if (IS_SET (obj->value[1], EX_LOCKED))
	    {
	      send_to_char ("It's already locked.\n\r", ch);
	      return;
	    }
	  SET_BIT (obj->value[1], EX_LOCKED);
	  act ("You lock $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n locks $p.", ch, obj, NULL, TO_ROOM);
	  trap_check(ch,"portal",NULL,obj);
	  {
	    OBJ_DATA *kobj;
	    for (kobj = ch->carrying; kobj != NULL; kobj = kobj->next_content)
	      {
		if (kobj->pIndexData->vnum == obj->value[4])
		  {
		    if (kobj->item_type == ITEM_ROOM_KEY)
		      kobj->value[0]++;
		    if (kobj->value[0] >= 2)
		      {
			act ("$p crumbles into dust.", ch, kobj, NULL,
			     TO_CHAR);
			extract_obj (kobj);
		      }
		    break;
		  }
	      }
	  }
	  return;
	}
      if (obj->item_type == ITEM_CTRANSPORT)
	{
	  if (!IS_SET (obj->value[5], CONT_CLOSED))
	    {
	      send_to_char ("It's not closed.\n\r", ch);
	      return;
	    }
	  if (obj->value[5] < 0)
	    {
	      send_to_char ("It can't be locked.\n\r", ch);
	      return;
	    }
	  if (!has_key (ch, obj->value[6]))
	    {
	      send_to_char ("You lack the key.\n\r", ch);
	      return;
	    }
	  if (IS_SET (obj->value[5], CONT_LOCKED))
	    {
	      send_to_char ("It's already locked.\n\r", ch);
	      return;
	    }
	  SET_BIT (obj->value[5], CONT_LOCKED);
	  act ("You lock $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n locks $p.", ch, obj, NULL, TO_ROOM);
	  return;
	}

      /* 'lock object' */
      if (obj->item_type != ITEM_CONTAINER)
	{
	  send_to_char ("That's not a container.\n\r", ch);
	  return;
	}
      if (!IS_SET (obj->value[1], CONT_CLOSED))
	{
	  send_to_char ("It's not closed.\n\r", ch);
	  return;
	}
      if (obj->value[2] < 0)
	{
	  send_to_char ("It can't be locked.\n\r", ch);
	  return;
	}
      if (!has_key (ch, obj->value[2]))
	{
	  send_to_char ("You lack the key.\n\r", ch);
	  return;
	}
      if (IS_SET (obj->value[1], CONT_LOCKED))
	{
	  send_to_char ("It's already locked.\n\r", ch);
	  return;
	}
      SET_BIT (obj->value[1], CONT_LOCKED);
      act ("You lock $p.", ch, obj, NULL, TO_CHAR);
      act ("$n locks $p.", ch, obj, NULL, TO_ROOM);
      trap_check(ch,"object",NULL,obj);
      {
	OBJ_DATA *kobj;
	for (kobj = ch->carrying; kobj != NULL; kobj = kobj->next_content)
	  {
	    if (kobj->pIndexData->vnum == obj->value[2])
	      {
		if (kobj->item_type == ITEM_ROOM_KEY)
		  kobj->value[0]++;
		if (kobj->value[0] >= 2)
		  {
		    act ("$p crumbles into dust.", ch, kobj, NULL, TO_CHAR);
		    extract_obj (kobj);
		  }
		break;
	      }
	  }
      }
      return;
    }

  else if (IS_SET (ch->in_room->room_flags, ROOM_TRANSPORT))
    {
      for (obj = object_list; obj; obj = obj->next)
	{
	  if (obj->item_type != ITEM_CTRANSPORT)
	    continue;
	  if (obj->value[2] == ch->in_room->vnum && is_name (arg, obj->name))
	    {
	      if (!IS_SET (obj->value[5], CONT_CLOSED))
		{
		  send_to_char ("It's not closed.\n\r", ch);
		  return;
		}
	      if (obj->value[5] < 0)
		{
		  send_to_char ("It can't be locked.\n\r", ch);
		  return;
		}
	      if (!has_key (ch, obj->value[6]))
		{
		  send_to_char ("You lack the key.\n\r", ch);
		  return;
		}
	      if (IS_SET (obj->value[5], CONT_LOCKED))
		{
		  send_to_char ("It's already locked.\n\r", ch);
		  return;
		}
	      SET_BIT (obj->value[5], CONT_LOCKED);
	      act ("You lock $p.", ch, obj, NULL, TO_CHAR);
	      act ("$n locks $p.", ch, obj, NULL, TO_ROOM);
	      return;
	    }
	}
    }
  if ((door = find_door (ch, arg)) >= 0)
    {

      /* 'lock door' */
      ROOM_INDEX_DATA *to_room;
      EXIT_DATA *pexit;
      EXIT_DATA *pexit_rev;
      pexit = ch->in_room->exit[door];
      if (!IS_SET (pexit->exit_info, EX_CLOSED))
	{
	  send_to_char ("It's not closed.\n\r", ch);
	  return;
	}
      if (pexit->key < 0)
	{
	  send_to_char ("It can't be locked.\n\r", ch);
	  return;
	}
      if (!has_key (ch, pexit->key))
	{
	  send_to_char ("You lack the key.\n\r", ch);
	  return;
	}
      if (IS_SET (pexit->exit_info, EX_LOCKED))
	{
	  send_to_char ("It's already locked.\n\r", ch);
	  return;
	}
      SET_BIT (pexit->exit_info, EX_LOCKED);
      send_to_char ("*Click*\n\r", ch);
      act ("$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM);
      trap_check(ch,"door",NULL,obj);

      /* lock the other side */
      if ((to_room = pexit->u1.to_room) != NULL
	  && (pexit_rev = to_room->exit[rev_dir[door]]) != 0
	  && pexit_rev->u1.to_room == ch->in_room)
	{
	  SET_BIT (pexit_rev->exit_info, EX_LOCKED);
	}
      {
	OBJ_DATA *kobj;
	for (kobj = ch->carrying; kobj != NULL; kobj = kobj->next_content)
	  {
	    if (kobj->pIndexData->vnum == pexit->key)
	      {
		if (kobj->item_type == ITEM_ROOM_KEY)
		  kobj->value[0]++;
		if (kobj->value[0] >= 2)
		  {
		    act ("$p crumbles into dust.", ch, kobj, NULL, TO_CHAR);
		    extract_obj (kobj);
		  }
		break;
	      }
	  }
      }
    }
  return;
}

void do_unlock (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int door;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Unlock what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_here (ch, arg)) != NULL)
    {

      /* portal stuff */
      if (obj->item_type == ITEM_PORTAL)
	{
	  if (IS_SET (obj->value[1], EX_ISDOOR))
	    {
	      send_to_char ("You can't do that.\n\r", ch);
	      return;
	    }
	  if (!IS_SET (obj->value[1], EX_CLOSED))
	    {
	      send_to_char ("It's not closed.\n\r", ch);
	      return;
	    }
	  if (obj->value[4] < 0)
	    {
	      send_to_char ("It can't be unlocked.\n\r", ch);
	      return;
	    }
	  if (!has_key (ch, obj->value[4]))
	    {
	      send_to_char ("You lack the key.\n\r", ch);
	      return;
	    }
	  if (!IS_SET (obj->value[1], EX_LOCKED))
	    {
	      send_to_char ("It's already unlocked.\n\r", ch);
	      return;
	    }
	  REMOVE_BIT (obj->value[1], EX_LOCKED);
	  act ("You unlock $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n unlocks $p.", ch, obj, NULL, TO_ROOM);
	  trap_check(ch,"portal",NULL,obj);
	  {
	    OBJ_DATA *kobj;
	    for (kobj = ch->carrying; kobj != NULL; kobj = kobj->next_content)
	      {
		if (kobj->pIndexData->vnum == obj->value[4])
		  {
		    if (kobj->item_type == ITEM_ROOM_KEY)
		      kobj->value[0]++;
		    if (kobj->value[0] >= 2)
		      {
			act ("$p crumbles into dust.", ch, kobj, NULL,
			     TO_CHAR);
			extract_obj (kobj);
		      }
		    break;
		  }
	      }
	  }
	  return;
	}
      if (obj->item_type == ITEM_CTRANSPORT)
	{
	  if (!IS_SET (obj->value[5], CONT_CLOSED))
	    {
	      send_to_char ("It's not closed.\n\r", ch);
	      return;
	    }
	  if (obj->value[5] < 0)
	    {
	      send_to_char ("It can't be unlocked.\n\r", ch);
	      return;
	    }
	  if (!has_ct_key (ch, obj))
	    {
	      send_to_char ("You lack the key.\n\r", ch);
	      return;
	    }
	  if (!IS_SET (obj->value[5], CONT_LOCKED))
	    {
	      send_to_char ("It's already unlocked.\n\r", ch);
	      return;
	    }
	  REMOVE_BIT (obj->value[5], CONT_LOCKED);
	  act ("You unlock $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n unlocks $p.", ch, obj, NULL, TO_ROOM);
	  return;
	}

      /* 'unlock object' */
      if (obj->item_type != ITEM_CONTAINER)
	{
	  send_to_char ("That's not a container.\n\r", ch);
	  return;
	}
      if (!IS_SET (obj->value[1], CONT_CLOSED))
	{
	  send_to_char ("It's not closed.\n\r", ch);
	  return;
	}
      if (obj->value[2] < 0)
	{
	  send_to_char ("It can't be unlocked.\n\r", ch);
	  return;
	}
      if (!has_key (ch, obj->value[2]))
	{
	  send_to_char ("You lack the key.\n\r", ch);
	  return;
	}
      if (!IS_SET (obj->value[1], CONT_LOCKED))
	{
	  send_to_char ("It's already unlocked.\n\r", ch);
	  return;
	}
      REMOVE_BIT (obj->value[1], CONT_LOCKED);
      act ("You unlock $p.", ch, obj, NULL, TO_CHAR);
      act ("$n unlocks $p.", ch, obj, NULL, TO_ROOM);
      trap_check(ch,"object",NULL,obj);
      {
	OBJ_DATA *kobj;
	for (kobj = ch->carrying; kobj != NULL; kobj = kobj->next_content)
	  {
	    if (kobj->pIndexData->vnum == obj->value[2])
	      {
		if (kobj->item_type == ITEM_ROOM_KEY)
		  kobj->value[0]++;
		if (kobj->value[0] >= 2)
		  {
		    act ("$p crumbles into dust.", ch, kobj, NULL, TO_CHAR);
		    extract_obj (kobj);
		  }
		break;
	      }
	  }
      }
      return;
    }

  else if (IS_SET (ch->in_room->room_flags, ROOM_TRANSPORT))
    {
      for (obj = object_list; obj; obj = obj->next)
	{
	  if (obj->item_type != ITEM_CTRANSPORT)
	    continue;
	  if (obj->value[2] == ch->in_room->vnum && is_name (arg, obj->name))
	    {
	      if (!IS_SET (obj->value[5], CONT_CLOSED))
		{
		  send_to_char ("It's not closed.\n\r", ch);
		  return;
		}
	      if (obj->value[5] < 0)
		{
		  send_to_char ("It can't be unlocked.\n\r", ch);
		  return;
		}
	      if (!has_ct_key (ch, obj))
		{
		  send_to_char ("You lack the key.\n\r", ch);
		  return;
		}
	      if (!IS_SET (obj->value[5], CONT_LOCKED))
		{
		  send_to_char ("It's already unlocked.\n\r", ch);
		  return;
		}
	      REMOVE_BIT (obj->value[5], CONT_LOCKED);
	      act ("You unlock $p.", ch, obj, NULL, TO_CHAR);
	      act ("$n unlocks $p.", ch, obj, NULL, TO_ROOM);
	      return;
	    }
	}
    }
  if ((door = find_door (ch, arg)) >= 0)
    {

      /* 'unlock door' */
      ROOM_INDEX_DATA *to_room;
      EXIT_DATA *pexit;
      EXIT_DATA *pexit_rev;
      pexit = ch->in_room->exit[door];
	if (!IS_SET (pexit->exit_info, EX_CLOSED))
	{
	  send_to_char ("It's not closed.\n\r", ch);
	  return;
	}
      if (pexit->key < 0)
	{
	  send_to_char ("It can't be unlocked.\n\r", ch);
	  return;
	}
      if (!has_key (ch, pexit->key))
	{
	  send_to_char ("You lack the key.\n\r", ch);
	  return;
	}
      if (!IS_SET (pexit->exit_info, EX_LOCKED))
	{
	  send_to_char ("It's already unlocked.\n\r", ch);
	  return;
	}
      REMOVE_BIT (pexit->exit_info, EX_LOCKED);
      send_to_char ("*Click*\n\r", ch);
      act ("$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM);
      trap_check(ch,flag_string (dir_flags,door),ch->in_room,NULL);


      /* unlock the other side */
      if ((to_room = pexit->u1.to_room) != NULL
	  && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
	  && pexit_rev->u1.to_room == ch->in_room)
	{
	  REMOVE_BIT (pexit_rev->exit_info, EX_LOCKED);
          trap_check(ch,flag_string (dir_flags,rev_dir[door]),to_room,NULL);
	}
      {
	OBJ_DATA *kobj;
	for (kobj = ch->carrying; kobj != NULL; kobj = kobj->next_content)
	  {
	    if (kobj->pIndexData->vnum == pexit->key)
	      {
		if (kobj->item_type == ITEM_ROOM_KEY)
		  kobj->value[0]++;
		if (kobj->value[0] >= 2)
		  {
		    act ("$p crumbles into dust.", ch, kobj, NULL, TO_CHAR);
		    extract_obj (kobj);
		  }
		break;
	      }
	  }
      }
    }
  return;
}

void do_pick (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *gch;
  OBJ_DATA *obj;
  int door;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Pick what?\n\r", ch);
      return;
    }
  if (IS_NPC (ch) && ch->master != NULL)
    {
      if (ch->master->pet == ch)
	return;
    }
  WAIT_STATE (ch, skill_table[gsn_pick_lock].beats);

  /* look for guards */
  for (gch = ch->in_room->people; gch; gch = gch->next_in_room)
    {
      if (IS_NPC (gch) && IS_AWAKE (gch) && ch->level + 5 < gch->level)
	{
	  act ("$N is standing too close to the lock.", ch, NULL, gch,
	       TO_CHAR);
	  return;
	}
    }
  if (!IS_NPC (ch) && number_percent () > get_skill (ch, gsn_pick_lock))
    {
      send_to_char ("You failed.\n\r", ch);
      check_improve (ch, gsn_pick_lock, FALSE, 2);
      return;
    }
  if ((obj = get_obj_here (ch, arg)) != NULL)
    {

      /* portal stuff */
      if (obj->item_type == ITEM_PORTAL)
	{
	  if (!IS_SET (obj->value[1], EX_ISDOOR))
	    {
	      send_to_char ("You can't do that.\n\r", ch);
	      return;
	    }
	  if (!IS_SET (obj->value[1], EX_CLOSED))
	    {
	      send_to_char ("It's not closed.\n\r", ch);
	      return;
	    }
	  if (obj->value[4] < 0)
	    {
	      send_to_char ("It can't be unlocked.\n\r", ch);
	      return;
	    }
	  if (IS_SET (obj->value[1], EX_PICKPROOF))
	    {
	      send_to_char ("You failed.\n\r", ch);
	      return;
	    }
	  REMOVE_BIT (obj->value[1], EX_LOCKED);
	  act ("You pick the lock on $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n picks the lock on $p.", ch, obj, NULL, TO_ROOM);
	  check_improve (ch, gsn_pick_lock, TRUE, 2);
	  return;
	}

      /* 'pick object' */
      if (obj->item_type != ITEM_CONTAINER)
	{
	  send_to_char ("That's not a container.\n\r", ch);
	  return;
	}
      if (!IS_SET (obj->value[1], CONT_CLOSED))
	{
	  send_to_char ("It's not closed.\n\r", ch);
	  return;
	}
      if (obj->value[2] < 0)
	{
	  send_to_char ("It can't be unlocked.\n\r", ch);
	  return;
	}
      if (!IS_SET (obj->value[1], CONT_LOCKED))
	{
	  send_to_char ("It's already unlocked.\n\r", ch);
	  return;
	}
      if (IS_SET (obj->value[1], CONT_PICKPROOF))
	{
	  send_to_char ("You failed.\n\r", ch);
	  return;
	}
      REMOVE_BIT (obj->value[1], CONT_LOCKED);
      act ("You pick the lock on $p.", ch, obj, NULL, TO_CHAR);
      act ("$n picks the lock on $p.", ch, obj, NULL, TO_ROOM);
      check_improve (ch, gsn_pick_lock, TRUE, 2);
      return;
    }
  if ((door = find_door (ch, arg)) >= 0)
    {

      /* 'pick door' */
      ROOM_INDEX_DATA *to_room;
      EXIT_DATA *pexit;
      EXIT_DATA *pexit_rev;
      pexit = ch->in_room->exit[door];
      if (!IS_SET (pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL (ch))
	{
	  send_to_char ("It's not closed.\n\r", ch);
	  return;
	}
      if (pexit->key < 0 && !IS_IMMORTAL (ch))
	{
	  send_to_char ("It can't be picked.\n\r", ch);
	  return;
	}
      if (!IS_SET (pexit->exit_info, EX_LOCKED))
	{
	  send_to_char ("It's already unlocked.\n\r", ch);
	  return;
	}
      if (IS_SET (pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL (ch))
	{
	  send_to_char ("You failed.\n\r", ch);
	  return;
	}
      if (IS_SET (pexit->exit_info, EX_HARD) && !IS_IMMORTAL (ch) && number_percent() != 47)
      {
        send_to_char ("You failed.\n\r", ch);
        return;
      }
      if (IS_SET (pexit->exit_info, EX_INFURIATING) && !IS_IMMORTAL (ch) 
		      && number_percent() != 47 && number_percent() != 93)
      {
        send_to_char ("You failed.\n\r", ch);
        return;
      }
      REMOVE_BIT (pexit->exit_info, EX_LOCKED);
      send_to_char ("*Click*\n\r", ch);
      act ("$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM);
      check_improve (ch, gsn_pick_lock, TRUE, 2);

      /* pick the other side */
      if ((to_room = pexit->u1.to_room) != NULL
	  && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
	  && pexit_rev->u1.to_room == ch->in_room)
	{
	  REMOVE_BIT (pexit_rev->exit_info, EX_LOCKED);
	}
    }
  return;
}

void do_drag (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int door, chroll, vicroll;
  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Who do you want to drag?\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (!IS_NPC(ch) && current_time - ch->pcdata->last_pkdeath < (60*MULTIKILL_WAIT_TIME))
  {
    send_to_char("You must be declared killable again, first. (type `kkillable``)\n\r",ch);
    return;
  }
  if (!IS_NPC(victim) && current_time - victim->pcdata->last_pkdeath < (60*MULTIKILL_WAIT_TIME)) 
  {
    act("$N is protected by a divine light, you dare not approach.",ch,NULL,victim,TO_CHAR);
    return;
  }
  if (victim == ch)
    {
      send_to_char ("You can't drag yourself!\n\r", ch);
      return;
    }
  if (victim->position == POS_TETHERED)
    {
      send_to_char ("You can't drag tethered mobiles.\n\r", ch);
      return;
    }
  if (victim->position == POS_MOUNTED)
    {
      send_to_char ("You can't drag mounted players.\n\r", ch);
      return;
    }
  if (victim->position == POS_STANDING)
    {
      act ("$N is standing. Try pushing $M.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (ch->position == POS_FIGHTING || ch->fighting != NULL)
    {
      send_to_char ("Maybe you should stop fighting first?\n\r", ch);
      return;
    }
  if (ch->position == POS_FIGHTING || ch->fighting != NULL)
    {
      send_to_char ("You can't drag fighting players.\n\r", ch);
      return;
    }
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (ch->position >= POS_FEIGNING_DEATH)
  {
    act("$N can't be dragged while doing that.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if (IS_NPC (victim) && (IS_SET (victim->act2, ACT_ILLUSION)))
    {
      act ("You try to drag $N to no avail.",ch,NULL,victim,TO_CHAR);
      act("$n attempts to drag $N, but $s hands go right through $S.",ch,NULL,victim,TO_ROOM);
      return;
    }

  if (IS_SET (ch->in_room->room_flags, ROOM_NO_PUSH))
    {
      send_to_char ("This place is safe from bullies like you.\n\r", ch);
      return;
    }

   if (IS_SET (victim->act2, ACT_NOMOVE) || IS_SET (victim->act2, ACT_BLOCK_EXIT))
   {
	   send_to_char ("That mob's power blocks your dragging efforts.\n\r",ch);
	   return;
   }
  
  if (!IS_NPC (victim))
    {
      if (IS_SET (ch->in_room->room_flags, ROOM_SAFE))
	if (!(time (NULL) - victim->pcdata->last_fight < 120))
	  {
	    send_to_char
	      ("This place is safe from bullies like you.\n\r", ch);
	    return;
	  }
      if (victim->desc != NULL && victim->desc->pEdit != NULL)
	{
	  send_to_char ("You may not drag a person who is building.\n\r", ch);
	  return;
	}
      if (victim->pcdata->cg_state != 0)
        {
          send_to_char ("Be nice to people playing Miverblis!\n\r",ch);
          return;
        }
      
    }

  else
    {
      if (IS_SET (ch->in_room->room_flags, ROOM_SAFE))
	{
	  send_to_char ("This place is safe from bullies like you.\n\r", ch);
	  return;
	}
    }
  argument = one_argument (argument, arg);
  if (!str_cmp (arg, "n") || !str_cmp (arg, "north"))
    door = 0;

  else if (!str_cmp (arg, "e") || !str_cmp (arg, "east"))
    door = 1;

  else if (!str_cmp (arg, "s") || !str_cmp (arg, "south"))
    door = 2;

  else if (!str_cmp (arg, "w") || !str_cmp (arg, "west"))
    door = 3;

  else if (!str_cmp (arg, "u") || !str_cmp (arg, "up"))
    door = 4;

  else if (!str_cmp (arg, "d") || !str_cmp (arg, "down"))
    door = 5;

  else
    {
      send_to_char ("You must specify a direction to drag in.\n\r", ch);
      return;
    }
  chroll =
    get_curr_stat (ch, STAT_STR) + get_curr_stat (ch,
						  STAT_DEX) +
    (ch->level / 5);
  vicroll =
    get_curr_stat (victim, STAT_STR) + get_curr_stat (victim,
						      STAT_DEX) +
    (victim->level / 10);
  if (chroll + number_range (1, 5) >= vicroll + number_range (1, 5))
    drag_char (ch, victim, door, FALSE);

  else
    {
      act ("$n tries unsuccessfully to drag you.", ch, NULL, victim, TO_VICT);
      act ("$n tries unsuccessfully to drag $N.", ch, NULL, victim,
	   TO_NOTVICT);
      act ("You try unsuccessfully to drag $N.", ch, NULL, victim, TO_CHAR);
    }
  return;
}

void drag_char (CHAR_DATA * ch, CHAR_DATA * victim, int door, bool follow)
{
  CHAR_DATA *fch, *blocker;
  CHAR_DATA *fch_next;
  ROOM_INDEX_DATA *in_room;
  ROOM_INDEX_DATA *to_room;
  EXIT_DATA *pexit;
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *tObj;
  if (is_affected (ch, skill_lookup ("fear")))
    {
      send_to_char
	("You attempt to leave the room, but your fear holds you tight.\n\r",
	 ch);
      act ("$n struggles vainly against the fear which holds $m in place.",
	   ch, NULL, NULL, TO_ROOM);
      return;
    }
  if (door < 0 || door > 5)
    {
      bug ("Do_move: bad door %d.", door);
      return;
    }
  in_room = ch->in_room;
  if ((pexit = in_room->exit[door]) == NULL
      || (to_room = pexit->u1.to_room) == NULL
      || !can_see_room (ch, pexit->u1.to_room))
    {
      send_to_char ("There is not an exit in that direction.\n\r", ch);
      return;
    }
  if (pexit->u1.vnum == 0 || pexit->u1.to_room == NULL)
    {
      send_to_char
	("For some reason you dream that your head is being slammed into a wall!\n\r",
	 victim);
      send_to_char ("There is not an exit in that direction.\n\r", ch);
      return;
    }
//IBLIS 5/31/03 - Added the below checks for dragging into a clan hall, Class specific room
//                so we can finally get rid of the vast majority of the !#@$ing nobully rooms
  if ((to_room->clan != CLAN_BOGUS)
      && (victim->clan != to_room->clan) && !IS_SET (victim->act, ACT_PET))
    {
      act ("You can't seem to drag $N in there.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (!can_move_char (victim, to_room, FALSE, FALSE)
      && !IS_AFFECTED (victim, AFF_CHARM))
    {
      act ("You can't seem to drag $N in that direction.", ch, NULL,
	   victim, TO_CHAR);
      return;
    }


  for (blocker = ch->in_room->people; blocker != NULL;
		  blocker = blocker->next_in_room)
  { 
	  if (!IS_SET (blocker->act2, ACT_BLOCK_EXIT)
			  || blocker->blocks_exit == -1)
		  continue;
	  if (door != blocker->blocks_exit)
		  continue;
	  if (IS_IMMORTAL (ch) || IS_SET(ch->act2,ACT_NO_BLOCK))
	  {
		  sprintf (buf, "%s would have blocked your way %s, except you're an Immortal.\n\r",
				  blocker->short_descr, dir_name[blocker->blocks_exit]);
			  send_to_char(buf,ch);
		  break;
	  }
	  sprintf (buf, "%s blocks your way %s...\n\r",
			  blocker->short_descr, dir_name[blocker->blocks_exit]);
		  send_to_char (buf, ch);
	  return ;
  }
  
  
  if (pexit->u1.to_room->Class_flags != 0)
      if (!room_Class_check (victim, pexit->u1.to_room))
	{
	  act ("You can't seem to drag $N in that direction.", ch, NULL,
	       victim, TO_CHAR);
	  return;
	}
  if (pexit->u1.to_room->race_flags != 0)
  {
    if (!room_race_check(ch,pexit->u1.to_room))
    {
      act ("You can't seem to drag $N in that direction.", ch, NULL, victim, TO_CHAR);
      return;
    }
  }
// Minax 2-13-03 Cannot drag charmed mobs into no_charmie rooms
  if (IS_AFFECTED (victim, AFF_CHARM)
      && (IS_SET (to_room->room_flags, ROOM_NO_CHARMIE)))
    {
      send_to_char ("Charmed folks cannot enter that room.\n\r", ch);
      return;
    }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (IS_NPC (victim)
      && (IS_SET (victim->act, ACT_TRAIN)
	  || IS_SET (victim->act, ACT_PRACTICE)
	  || IS_SET (victim->act, ACT_IS_HEALER)
	  || IS_SET (victim->act, ACT_AGGRESSIVE)))
    {
      send_to_char ("I don't think so.\n\r", ch);
      return;
    }
  // Mathew 4/16/99: Added EX_NOPASS door check so PC's/NPC's cannot be
  // dragged through doors that are NOPASS.
  if (IS_SET (pexit->exit_info, EX_CLOSED) &&
      (!IS_AFFECTED (ch, AFF_PASS_DOOR)
       || IS_SET (pexit->exit_info, EX_NOPASS)))
    {
      act ("The $d is closed, you slam against it.", victim, NULL,
	   pexit->keyword, TO_CHAR);
      act ("You slam $N against the $t!", ch, pexit->keyword, victim,
	   TO_CHAR);
      return;
    }
  // Mathew 4/28/99: Added EX_NOPASS door check so victim PC's/NPC's cannot be
  // dragged through doors that are NOPASS.
  if (IS_SET (pexit->exit_info, EX_CLOSED) &&
      (!IS_AFFECTED (victim, AFF_PASS_DOOR)
       || IS_SET (pexit->exit_info, EX_NOPASS)))
    {
      act ("The $d is closed, you are dragged against it.", victim, NULL,
	   pexit->keyword, TO_CHAR);
      act ("$N cannot pass through $t!", ch, pexit->keyword, victim, TO_CHAR);
      return;
    }
  if (room_is_private (to_room))
    {
      if (to_room->max_message[0] == '\0')
	send_to_char ("That room is private right now.\n\r", ch);

      else
	{
	  send_to_char (to_room->max_message, ch);
	  send_to_char ("\n\r", ch);
	}
      return;
    }
  //Iblis 6/21/03 - Battle Royale room off limit checking
  if (!br_vnum_check (to_room->vnum))
    {
      send_to_char
	("That room is off limits during `iBattle Royale``\r\n", ch);
      return;
    }
  if (in_room->sector_type == SECT_AIR || to_room->sector_type == SECT_AIR)
    {
      if (!IS_AFFECTED (ch, AFF_FLYING) && !IS_IMMORTAL (ch))
	{
	  send_to_char
	    ("You are dragged into the air and fall down.\n\r", victim);
	  return;
	}
    }
  WAIT_STATE (ch, 1);
  for (tObj = ch->in_room->contents; tObj; tObj = tObj->next_content)
    if (tObj->pIndexData->vnum == OBJ_VNUM_WALL_THORNS
	&& tObj->value[0] == door)
      {
	if (wall_damage (ch, tObj))
	  {
	    act ("The wall of thorns stops you from dragging $N any further.",
		 ch, NULL, victim, TO_CHAR);
	    sprintf (buf,
		     "$n attempts to drag $N %s, but is stopped by the wall of thorns!",
		     dir_name[door]);
	    act (buf, ch, NULL, victim, TO_NOTVICT);
	    sprintf (buf,
		     "$n attempts to drag you %s, but is stopped by the wall of thorns!",
		     dir_name[door]);
	    act (buf, ch, NULL, victim, TO_VICT);
	    return;
	  }
	break;
      }
  char_from_room (ch);
  char_to_room (ch, to_room);
  if (tObj != NULL && wall_damage (victim, tObj))
    {
      sprintf (buf, "The wall of thorns stops $n from dragging you %s.",
	       dir_name[door]);
      act (buf, ch, NULL, victim, TO_VICT);
      act
	("The wall of thorns rips $N from your grip as it pierces you with tiny thorns.",
	 ch, NULL, victim, TO_CHAR);
      sprintf (buf, "The wall of thorns stops $N from dragging $n %s.",
	       dir_name[door]);
      act (buf, victim, NULL, ch, TO_ROOM);
      act ("$n has arrived.", ch, NULL, NULL, TO_ROOM);
      do_look (ch, "auto");
      return;
    }
  sprintf (buf, "Summoning all of your strength, you drag $N %s!",
	   dir_name[door]);
  act (buf, ch, NULL, victim, TO_CHAR);
  do_look (ch, "auto");
  sprintf (buf, "$n drags you %s!", dir_name[door]);
  act (buf, ch, NULL, victim, TO_VICT);
  sprintf (buf, "$N drags $n %s!", dir_name[door]);
  act (buf, victim, NULL, ch, TO_NOTVICT);
  if (!can_move_char_door_new(victim,door,FALSE,TRUE,POS_DEAD))
  {
	  send_to_char("Circumstances make you leave your victim behind.\n\r",ch);
	  send_to_char("Circumstances make your assailant leave you behind.\n\r",victim);
	  return;
  }
  char_from_room (victim);
  char_to_room (victim, to_room);
  act ("$n drags $N into the room.", ch, NULL, victim, TO_NOTVICT);
  do_look (victim, "auto");
  trap_check(victim,"room",victim->in_room,NULL);  
  trip_triggers(victim, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  sprintf(buf,"%d",victim->in_room->vnum);
  trip_triggers_arg(victim, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,buf);
  check_aggression(victim);
  if (in_room == to_room)	/* no circular follows */
    return;
  for (fch = in_room->people; fch != NULL; fch = fch_next)
    {
      fch_next = fch->next_in_room;
      if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
	  && fch->position < POS_STANDING)
	do_stand (fch, "");
      if (fch->master == ch && fch->position == POS_STANDING)
	{
	  if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
	      && (IS_NPC (fch) && is_aggro (fch)))
	    {
	      act ("You can't bring $N into the city.", ch, NULL, fch,
		   TO_CHAR);
	      act ("You aren't allowed in the city.", fch, NULL, NULL,
		   TO_CHAR);
	      return;
	    }
	  REMOVE_BIT (fch->affected_by, AFF_HIDE);
	  REMOVE_BIT (ch->affected_by, AFF_HIDE);
	  REMOVE_BIT (fch->affected_by, AFF_CAMOUFLAGE);
	  REMOVE_BIT (ch->affected_by, AFF_CAMOUFLAGE);
	  act ("You follow $N.", fch, NULL, ch, TO_CHAR);
	  move_char (fch, door, TRUE);
	}
    }
  return;
}

void do_push (CHAR_DATA * ch, char *argument)
{
  push_char (ch, argument, FALSE);
}

void push_char (CHAR_DATA * ch, char *argument, short always)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim, *blocker;
  EXIT_DATA *pexit;
  ROOM_INDEX_DATA *in_room, *to_room;
  int door=0, chroll, vicroll;
  OBJ_DATA *tObj;
  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Who do you want to push?\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (!IS_NPC(ch) && current_time - ch->pcdata->last_pkdeath < (60*MULTIKILL_WAIT_TIME))
  {
    send_to_char("You must be declared killable again, first.  (type `kkillable``)\n\r",ch);
    return;
  }

  if (!IS_NPC(victim) && current_time - victim->pcdata->last_pkdeath < (60*MULTIKILL_WAIT_TIME))
    {
	    act("$N is protected by a divine light, you dare not approach.",ch,NULL,victim,TO_CHAR);
	    return;
    }
  
  if (ch->in_room->sector_type == SECT_WATER_SWIM
      || ch->in_room->sector_type == SECT_WATER_NOSWIM
      || ch->in_room->sector_type == SECT_UNDERWATER
      || ch->in_room->sector_type == SECT_WATER_OCEAN)
    {
      send_to_char
	("Your efforts are resisted by the water currents.\n\r", ch);
      return;
    }
  if (IS_SET (ch->in_room->room_flags, ROOM_NO_PUSH) && !always)
    {
      send_to_char ("This room is safe from bullies like you.\n\r", ch);
      return;
    }

  if (IS_SET (victim->act2, ACT_NOMOVE) || IS_SET (victim->act2, ACT_BLOCK_EXIT))
  {
    send_to_char ("That mob's power blocks your pushing efforts.\n\r",ch);
    return;
  }

  

  for (blocker = ch->in_room->people; blocker != NULL;
		  blocker = blocker->next_in_room)
  { 
	  if (!IS_SET (blocker->act2, ACT_BLOCK_EXIT)
			  || blocker->blocks_exit == -1)
		  continue;
	  if (door != blocker->blocks_exit)
		  continue;
	  if (IS_IMMORTAL (ch) || IS_SET(ch->act2,ACT_NO_BLOCK))
	  {
		  sprintf (buf, "%s would have blocked your way %s, except you're an Immortal.\n\r",
				  blocker->short_descr, dir_name[blocker->blocks_exit]);
			  send_to_char(buf,ch);
		  break;
	  }
	  sprintf (buf, "%s blocks your way %s...\n\r",
			  blocker->short_descr, dir_name[blocker->blocks_exit]);
		  send_to_char (buf, ch);
	  return ;
  }
  
  if (!IS_NPC (victim))
    if (IS_SET (ch->in_room->room_flags, ROOM_NO_PUSH)
	&& !(time (NULL) - victim->pcdata->last_fight < 120) && !always)
      {
	send_to_char ("This room is safe from bullies like you.\n\r", ch);
	return;
      }
  if (IS_IMMORTAL (victim) && !IS_IMMORTAL (ch))
    {
      send_to_char ("I don't think so.\n\r", ch);
      return;
    }
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (victim->position == POS_TETHERED)
    {
      send_to_char ("You can't push tethered mounts.\n\r", ch);
      return;
    }
  if (!IS_NPC (victim))
    {
      if (IS_SET (ch->in_room->room_flags, ROOM_SAFE))
	if (!(time (NULL) - victim->pcdata->last_fight < 120) && !always)
	  {
	    send_to_char
	      ("This place is safe from bullies like you.\n\r", ch);
	    return;
	  }
      if (victim->desc != NULL && victim->desc->pEdit != NULL)
	{
	  send_to_char ("You may not push a person who is building.\n\r", ch);
	  return;
	}
      if (victim->pcdata->cg_state != 0)
      {
        send_to_char ("Be nice to people playing Miverblis!\n\r",ch);
        return;
      }
    }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (IS_NPC (victim)
      && (IS_SET (victim->act, ACT_TRAIN)
	  || IS_SET (victim->act, ACT_PRACTICE)
	  || IS_SET (victim->act, ACT_IS_HEALER)))
    {
      send_to_char ("I don't think so.\n\r", ch);
      return;
    }

  if (IS_NPC (victim) && (IS_SET (victim->act2, ACT_ILLUSION)))
  {
	  act ("You try to push $N to no avail.",ch,NULL,victim,TO_CHAR);
	  act("$n attempts to push $N, but $s hands go right through $S.",ch,NULL,victim,TO_ROOM);
	  return;
  }
  
  if (victim->position == POS_MOUNTED)
    {
      send_to_char ("You can't push mounted players.\n\r", ch);
      return;
    }
  if (victim->position == POS_RESTING || victim->position == POS_SITTING)
    {
      act ("$N is resting. Try dragging $M.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (!IS_AWAKE (victim))
    {
      act ("$N is asleep. Try dragging $M.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (ch->position == POS_FIGHTING || ch->fighting != NULL)
    {
      send_to_char ("Maybe you should stop fighting first?\n\r", ch);
      return;
    }
  if (victim->position == POS_FIGHTING || victim->fighting != NULL)
    {
      act ("$N is fighting. Wait your turn!", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (victim->position >= POS_FEIGNING_DEATH)
  {
	 act("$N can't be pushed while doing that.",ch,NULL,victim,TO_CHAR);
	 return;
  }
  argument = one_argument (argument, arg);
  if (!str_cmp (arg, "n") || !str_cmp (arg, "north"))
    door = 0;

  else if (!str_cmp (arg, "e") || !str_cmp (arg, "east"))
    door = 1;

  else if (!str_cmp (arg, "s") || !str_cmp (arg, "south"))
    door = 2;

  else if (!str_cmp (arg, "w") || !str_cmp (arg, "west"))
    door = 3;

  else if (!str_cmp (arg, "u") || !str_cmp (arg, "up"))
    door = 4;

  else if (!str_cmp (arg, "d") || !str_cmp (arg, "down"))
    door = 5;

  else
    {
      send_to_char ("You must specify a direction to push in.\n\r", ch);
      return;
    }
  in_room = ch->in_room;
  if ((pexit = in_room->exit[door]) == NULL
      || (to_room = pexit->u1.to_room) == NULL
      || !can_see_room (ch, pexit->u1.to_room))
    {
      send_to_char ("There is no exit in that direction.\n\r", ch);
      return;
    }
  if ((to_room->clan != CLAN_BOGUS) && (victim->clan != to_room->clan)
      && !IS_SET (victim->act, ACT_PET))
    {
      act ("You can't push $N in there.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (!can_move_char (victim, to_room, FALSE, FALSE)
      && !IS_AFFECTED (victim, AFF_CHARM))
    {
      act ("You can't push $N in that direction.", ch, NULL, victim,
	   TO_CHAR);
      return;
    }
  if (victim == ch)
    {
      send_to_char ("You can't push yourself.\n\r", ch);
      return;
    }
  if (pexit->u1.to_room->Class_flags != 0)
    {


      if (!room_Class_check (victim, pexit->u1.to_room))
	{
	  act ("You can't push $N in that direction.", ch, NULL,
	       victim, TO_CHAR);
	  return;
	}

    }
  if (to_room->race_flags != 0)
  {
    if (!room_race_check(ch,pexit->u1.to_room))
    {
      act ("You can't push $N in that direction.", ch, NULL, victim, TO_CHAR);
      return;
    }
  }
  // Mathew 4/16/99: Added EX_NOPASS door check so PC's/NPC's cannot be
  // pushed through doors that are NOPASS.
  if (IS_SET (pexit->exit_info, EX_CLOSED) &&
      (!IS_AFFECTED (victim, AFF_PASS_DOOR)
       || IS_SET (pexit->exit_info, EX_NOPASS)) && !IS_IMMORTAL(ch))
    {
      act ("The $d is closed, you are slammed into it.", victim, NULL,
	   pexit->keyword, TO_CHAR);
      act ("You slam $N into the $t!", ch, pexit->keyword, victim, TO_CHAR);
      return;
    }
  if (IS_SET (pexit->u1.to_room->room_flags, ROOM_NOMOUNT)
      && IS_SET (victim->act, ACT_MOUNT))
    {
      act ("You can't push $N in that direction.", ch, NULL,
	   victim, TO_CHAR);
      return;
    }
  if ((victim->level - 10) >= ch->level)
    {
      act ("$n tries unsuccessfully to push you.", ch, NULL, victim, TO_VICT);
      act ("$n tries unsuccessfully to push $N.", ch, NULL, victim,
	   TO_NOTVICT);
      act ("$N looks at you with contempt and ignores you.", ch, NULL,
	   victim, TO_CHAR);
      return;
    }
// Minax 2-13-03 Cannot push charmed mobs into no_charmie rooms
  if (IS_AFFECTED (victim, AFF_CHARM)
      && (IS_SET (to_room->room_flags, ROOM_NO_CHARMIE)))
    {
      send_to_char ("Charmed folks cannot enter that room.\n\r", ch);
      return;
    }
  chroll =
    get_curr_stat (ch, STAT_STR) + get_curr_stat (ch,
						  STAT_DEX) +
    (ch->level / 25) + get_carry_weight (ch);
  if (victim->riders != NULL)
    vicroll =
      get_curr_stat (victim, STAT_STR) + get_curr_stat (victim,
							STAT_DEX) +
      (victim->level / 10) + get_carry_weight (victim) +
      get_carry_weight (victim->riders);

  else
    vicroll =
      get_curr_stat (victim, STAT_STR) + get_curr_stat (victim,
							STAT_DEX) +
      (victim->level / 10) + get_carry_weight (victim);
  if (IS_NPC (victim) && IS_SET (victim->act, ACT_AGGRESSIVE))
    vicroll *= 3;

  //Iblis - Semicheap fix for hoarding mobs into rooms via push, while not rendering push completely useless
  if (!IS_NPC(ch) && IS_NPC(victim) && (dice(3,5) < ch->pcdata->pushed_mobs_counter))
  {
    act ("$N growls at you and attacks.", ch, NULL, victim, TO_CHAR);
    act ("$N growls at $n and attacks.", ch, NULL, victim, TO_ROOM);
    act ("You growl at $n and attack.", ch, NULL, victim, TO_VICT);
    do_kill (victim, ch->name);
    WAIT_STATE(ch,4);
    return;
  }
  
  if (chroll + number_range (1, 3) >= vicroll + number_range (1, 5) || always)
    {
      if (victim->riders == NULL)
	{
	  sprintf (buf, "$n slams into you, pushing you %s!", dir_name[door]);
	  act (buf, ch, NULL, victim, TO_VICT);
	  sprintf (buf, "$n slams into $N, pushing $M %s!", dir_name[door]);
	  act (buf, ch, NULL, victim, TO_NOTVICT);
	  sprintf (buf, "You slam into $N, pushing $M %s!", dir_name[door]);
	  act (buf, ch, NULL, victim, TO_CHAR);
	}

      else
	{
	  sprintf (buf, "$n slams into %s, pushing you %s!",
		   NAME (victim), dir_name[door]);
	  act (buf, ch, NULL, victim->riders, TO_VICT);
	  sprintf (buf, "$n slams into %s, pushing it and $N %s!",
		   NAME (victim), dir_name[door]);
	  act (buf, ch, NULL, victim->riders, TO_NOTVICT);
	  sprintf (buf, "You slam into $N, pushing $M and %s %s!",
		   NAME (victim->riders), dir_name[door]);
	  act (buf, ch, NULL, victim, TO_CHAR);
	}
      //IBLIS 8/27/03 - Fixed the bug here and all other places for wall of thorns.
      //  previously it was tObj->next, it needed to be tObj->next_content.  Bug
      //  has been here forever.  I previously fixed this for tripwire but did not
      //  notice wall of thorns until players began complaining recently

      for (tObj = ch->in_room->contents; tObj; tObj = tObj->next_content)
	if (tObj->pIndexData->vnum == OBJ_VNUM_WALL_THORNS
	    && tObj->value[0] == door)
	  {
	    if (wall_damage (victim, tObj))
	      {
		act ("The wall of thorns stops you from being pushed $T.",
		     victim, NULL, dir_name[door], TO_CHAR);
		return;
	      }
	    break;
	  }
      char_from_room (victim);
      char_to_room (victim, to_room);
      if (victim->riders != NULL)
	{
	  char nuf[MAX_STRING_LENGTH];
	  char_from_room (victim->riders);
	  char_to_room (victim->riders, to_room);
	  do_look (victim->riders, "auto");
	  sprintf (nuf, "$n comes flying into the room, carrying %s.",
		   victim->riders->name);
	  act (nuf, victim, NULL, victim->riders, TO_NOTVICT);
	}
      do_look (victim, "auto");
      act ("$n comes flying into the room!", victim, NULL, NULL, TO_ROOM);
      if (!IS_NPC(ch) && IS_NPC(victim) && ch->pcdata->pushed_mobs_counter < 10000)
      {
	      ++ch->pcdata->pushed_mobs_counter;
      }
      trip_triggers(victim, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(victim,"room",victim->in_room,NULL);
      sprintf(buf,"%d",victim->in_room->vnum);
      trip_triggers_arg(victim, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,buf);
      check_aggression(victim);
      if (number_range (1, 10) == 5 && !always && can_use_room (ch, to_room) && can_move_char_door(ch,door, FALSE, FALSE))
	{

	  // Mathew 4/16/99: Added EX_NOPASS door check so PC's/NPC's cannot
	  // fall through doors that are NOPASS.
	  if (IS_SET (pexit->exit_info, EX_CLOSED) &&
	      (!IS_AFFECTED (ch, AFF_PASS_DOOR)
	       || IS_SET (pexit->exit_info, EX_NOPASS)) && !IS_IMMORTAL(ch))
	    {
	      sprintf (buf,
		       "$n loses $s balance, bounces off the door, and embraces the floor face first.");
	      act (buf, ch, NULL, NULL, TO_ROOM);
	      sprintf (buf,
		       "You lose your balance, bounce off the door, and embrace the floor face first.");
	      act (buf, ch, NULL, victim, TO_CHAR);
	      ch->position = POS_RESTING;
	      return;
	    }			// end if
	  sprintf (buf, "$n loses $s balance, falling %s.", dir_name[door]);
	  act (buf, ch, NULL, NULL, TO_ROOM);
	  sprintf (buf, "You lose your balance and fall %s.", dir_name[door]);
	  act (buf, ch, NULL, victim, TO_CHAR);
	  if (tObj != NULL && wall_damage (ch, tObj))
	    {
	      act ("The wall of thorns stop you from falling $T.", ch,
		   NULL, dir_name[door], TO_CHAR);
	      return;
	    }
	  char_from_room (ch);
	  char_to_room (ch, to_room);
	  ch->position = POS_RESTING;
	  do_look (ch, "auto");
  	trap_check(ch,"room",ch->in_room,NULL);  
          trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
          sprintf(buf,"%d",ch->in_room->vnum);
          trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,buf);
 	  check_aggression(ch);
	  act ("$n comes stumbling into the room and falls over!", ch,
	       NULL, NULL, TO_ROOM);
	}
    }

  else
    {
      int chance = 0;

      chance += (40 / 22.0) * 25 - ch->mod_stat[STAT_CHA];
      chance +=
	(40 / 2000) * 2000 - (victim->alignment <
			      0) ? 1000 +
	victim->alignment : victim->alignment + 1000;

      if (IS_NPC (victim) && number_percent () < chance)
	{
	  act ("$N growls at you and attacks.", ch, NULL, victim, TO_CHAR);
	  act ("$N growls at $n and attacks.", ch, NULL, victim, TO_ROOM);
	  act ("You growl at $n and attack.", ch, NULL, victim, TO_VICT);
	  do_kill (victim, ch->name);
	}

      else
	{
	  act ("$n tries unsuccessfully to push you.", ch, NULL, victim,
	       TO_VICT);
	  act ("$n tries unsuccessfully to push $N.", ch, NULL, victim,
	       TO_NOTVICT);
	  act ("$N looks at you with contempt and ignores you.", ch,
	       NULL, victim, TO_CHAR);
	}
    }
  return;
}

void do_stand (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj = NULL;
  if (ch->race == PC_RACE_SWARM)
  {
	  send_to_char("Huh?\n\r",ch);
	  return;
  }
  if (ch->position == POS_MOUNTED)
    {
      send_to_char ("You must dismount before you can stand.\n\r", ch);
      return;
    }
  if (argument[0] != '\0' && argument[0] != '*')
    {
      if (ch->position == POS_FIGHTING || ch->fighting != NULL)
	{
	  send_to_char ("Maybe you should finish fighting first?\n\r", ch);
	  return;
	}
      obj = get_obj_list (ch, argument, ch->in_room->contents);
      if (obj == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
      if ((obj->item_type != ITEM_FURNITURE && obj->item_type != ITEM_RAFT)
	  || (!IS_SET (obj->value[2], STAND_AT)
	      && !IS_SET (obj->value[2], STAND_ON)
	      && !IS_SET (obj->value[2], STAND_IN)))
	{
	  send_to_char ("You can't seem to find a place to stand.\n\r", ch);
	  return;
	}
      if (ch->on != obj && count_users (obj) >= obj->value[0])
	{
	  act_new ("There's no room to stand on $p.", ch, obj, NULL,
		   TO_CHAR, POS_DEAD);
	  return;
	}
      else ch->on = obj;
    }
  if (ch->position == POS_STANDING && ch->fighting != NULL)
    ch->position = POS_FIGHTING;
  switch (get_position(ch))
    {
    case POS_SLEEPING:
      if (IS_AFFECTED (ch, AFF_SLEEP)
	  || is_affected (ch, skill_lookup ("Ballad of Ole Rip"))
	  || (!IS_NPC(ch) && ch->pcdata->knock_time))
	{
	  send_to_char ("You can't wake up!\n\r", ch);
	  return;
	}
      if (obj == NULL)
	{
	  send_to_char ("You wake and stand up.\n\r", ch);
	  act ("$n wakes and stands up.", ch, NULL, NULL, TO_ROOM);
	  ch->on = NULL;
	}

      else if (IS_SET (obj->value[2], STAND_AT))
	{
	  act_new ("You wake and stand at $p.", ch, obj, NULL, TO_CHAR,
		   POS_DEAD);
	  act ("$n wakes and stands at $p.", ch, obj, NULL, TO_ROOM);
	}

      else if (IS_SET (obj->value[2], STAND_ON))
	{
	  act_new ("You wake and stand on $p.", ch, obj, NULL, TO_CHAR,
		   POS_DEAD);
	  act ("$n wakes and stands on $p.", ch, obj, NULL, TO_ROOM);
	}

      else
	{
	  if (obj->pIndexData->vnum == OBJ_VNUM_HEXAGRAM)
	    {
	      act("The vision of $n quivers as $e steps into the hexagram.",ch,NULL,NULL,TO_ROOM);
	    }
          else act ("$n wakes and stands in $p.", ch, obj, NULL, TO_ROOM);
          act_new ("You wake and stand in $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);

	}
      ch->position = POS_STANDING;
      do_look (ch, "auto");
      break;
    case POS_RESTING:
    case POS_SITTING:
      if (obj == NULL)
	{
	  send_to_char ("You stand up.\n\r", ch);
	  act ("$n stands up.", ch, NULL, NULL, TO_ROOM);
	  ch->on = NULL;
	}

      else if (IS_SET (obj->value[2], STAND_AT))
	{
	  act ("You stand at $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n stands at $p.", ch, obj, NULL, TO_ROOM);
	}

      else if (IS_SET (obj->value[2], STAND_ON))
	{
	  act ("You stand on $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n stands on $p.", ch, obj, NULL, TO_ROOM);
	}

      else
	{
	  if (obj->pIndexData->vnum == OBJ_VNUM_HEXAGRAM)
	    {
	      act("The vision of $n quivers as $e steps into the hexagram.",ch,NULL,NULL,TO_ROOM);
	    }
          else act ("$n stands in $p.", ch, obj, NULL, TO_ROOM);
          act ("You stand in $p.", ch, obj, NULL, TO_CHAR);

	}
      ch->position = POS_STANDING;
      break;
    case POS_STANDING:
      if (obj == NULL)
        {
	  send_to_char ("You are already standing.\n\r", ch);
        }

      else if (IS_SET (obj->value[2], STAND_AT))
        {
          act ("You stand at $p.", ch, obj, NULL, TO_CHAR);
          act ("$n stands at $p.", ch, obj, NULL, TO_ROOM);
        }

      else if (IS_SET (obj->value[2], STAND_ON))
        {
          act ("You stand on $p.", ch, obj, NULL, TO_CHAR);
          act ("$n stands on $p.", ch, obj, NULL, TO_ROOM);
        }

      else
        {
	  if (obj->pIndexData->vnum == OBJ_VNUM_HEXAGRAM)
	  {
	    act("The vision of $n quivers as $e steps into the hexagram.",ch,NULL,NULL,TO_ROOM);
	  }
	  else act ("$n stands in $p.", ch, obj, NULL, TO_ROOM);
          act ("You stand in $p.", ch, obj, NULL, TO_CHAR);
        }
      break;
    case POS_FIGHTING:
      send_to_char ("You are already fighting!\n\r", ch);
      break;
    }
  ch->bashed = FALSE;
  if ((ch->fighting != NULL) && (ch->position < POS_FIGHTING))
    ch->position = POS_FIGHTING;
  return;
}

void do_rest (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj = NULL;
  if (ch->race == PC_RACE_SWARM)
    {
      send_to_char("Huh?\n\r",ch);
      return;
    }

  if (ch->position == POS_FIGHTING || ch->fighting != NULL)
    {
      send_to_char ("You are already fighting!\n\r", ch);
      return;
    }
  if (ch->position == POS_MOUNTED)
    {
      send_to_char ("You must dismount before you can rest.\n\r", ch);
      return;
    }

  /* okay, now that we know we can rest, find an object to rest on */
  if (argument[0] != '\0')
    {
      obj = get_obj_list (ch, argument, ch->in_room->contents);
      if (obj == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
    }

  else
    obj = ch->on;
  if (obj != NULL)
    {
      if ((!IS_SET (obj->item_type, ITEM_FURNITURE) && !IS_SET(obj->item_type,ITEM_RAFT))
	  || (!IS_SET (obj->value[2], REST_ON)
	      && !IS_SET (obj->value[2], REST_IN)
	      && !IS_SET (obj->value[2], REST_AT)))
	{
	  send_to_char ("You can't rest on that.\n\r", ch);
	  return;
	}
      if (obj != NULL && ch->on != obj && count_users (obj) >= obj->value[0])
	{
	  act_new ("There's no more room on $p.", ch, obj, NULL, TO_CHAR,
		   POS_DEAD);
	  return;
	}
      ch->on = obj;
    }
  switch (get_position(ch))
    {
    case POS_SLEEPING:
      if (IS_AFFECTED (ch, AFF_SLEEP)
	  || is_affected (ch, skill_lookup ("Ballad of Ole Rip"))
	  || (!IS_NPC(ch) && ch->pcdata->knock_time))
	{
	  send_to_char ("You can't wake up!\n\r", ch);
	  return;
	}
      if (obj == NULL)
	{
	  send_to_char ("You wake up and start resting.\n\r", ch);
	  act ("$n wakes up and starts resting.", ch, NULL, NULL, TO_ROOM);
	}

      else if (IS_SET (obj->value[2], REST_AT))
	{
	  act_new ("You wake up and rest at $p.", ch, obj, NULL,
		   TO_CHAR, POS_SLEEPING);
	  act ("$n wakes up and rests at $p.", ch, obj, NULL, TO_ROOM);
	}

      else if (IS_SET (obj->value[2], REST_ON))
	{
	  act_new ("You wake up and rest on $p.", ch, obj, NULL,
		   TO_CHAR, POS_SLEEPING);
	  act ("$n wakes up and rests on $p.", ch, obj, NULL, TO_ROOM);
	}

      else
	{
	  act_new ("You wake up and rest in $p.", ch, obj, NULL,
		   TO_CHAR, POS_SLEEPING);
	  act ("$n wakes up and rests in $p.", ch, obj, NULL, TO_ROOM);
	}
      ch->position = POS_RESTING;
      break;
    case POS_RESTING:
      send_to_char ("You are already resting.\n\r", ch);
      break;
    case POS_STANDING:
      if (obj == NULL)
	{
	  send_to_char ("You rest.\n\r", ch);
	  act ("$n sits down and rests.", ch, NULL, NULL, TO_ROOM);
	}

      else if (IS_SET (obj->value[2], REST_AT))
	{
	  act ("You sit down at $p and rest.", ch, obj, NULL, TO_CHAR);
	  act ("$n sits down at $p and rests.", ch, obj, NULL, TO_ROOM);
	}

      else if (IS_SET (obj->value[2], REST_ON))
	{
	  act ("You sit on $p and rest.", ch, obj, NULL, TO_CHAR);
	  act ("$n sits on $p and rests.", ch, obj, NULL, TO_ROOM);
	}

      else
	{
	  act ("You rest in $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n rests in $p.", ch, obj, NULL, TO_ROOM);
	}
      ch->position = POS_RESTING;
      break;
    case POS_SITTING:
      if (obj == NULL)
	{
	  send_to_char ("You rest.\n\r", ch);
	  act ("$n rests.", ch, NULL, NULL, TO_ROOM);
	}

      else if (IS_SET (obj->value[2], REST_AT))
	{
	  act ("You rest at $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n rests at $p.", ch, obj, NULL, TO_ROOM);
	}

      else if (IS_SET (obj->value[2], REST_ON))
	{
	  act ("You rest on $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n rests on $p.", ch, obj, NULL, TO_ROOM);
	}

      else
	{
	  act ("You rest in $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n rests in $p.", ch, obj, NULL, TO_ROOM);
	}
      ch->position = POS_RESTING;
      break;
    }
  trip_triggers(ch, OBJ_TRIG_CHAR_POS_CHANGE, NULL, NULL, OT_SPEC_NONE);
  return;
}

void do_sit (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj = NULL;
  if (ch->race == PC_RACE_SWARM)
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }  
  if (ch->position == POS_FIGHTING || ch->fighting != NULL)
    {
      send_to_char ("Maybe you should finish this fight first?\n\r", ch);
      return;
    }
  if (ch->position == POS_MOUNTED)
    {
      send_to_char ("You are already sitting on your mount.\n\r", ch);
      return;
    }

  /* okay, now that we know we can sit, find an object to sit on */
  if (argument[0] != '\0')
    {
      obj = get_obj_list (ch, argument, ch->in_room->contents);
      if (obj == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
    }

  else
    obj = ch->on;
  if (obj != NULL)
    {
      if ((!IS_SET (obj->item_type, ITEM_FURNITURE) && !IS_SET (obj->item_type,ITEM_RAFT))
	  || (!IS_SET (obj->value[2], SIT_ON)
	      && !IS_SET (obj->value[2], SIT_IN)
	      && !IS_SET (obj->value[2], SIT_AT)))
	{
	  send_to_char ("You can't sit on that.\n\r", ch);
	  return;
	}
      if (obj != NULL && ch->on != obj && count_users (obj) >= obj->value[0])
	{
	  act_new ("There's no more room on $p.", ch, obj, NULL, TO_CHAR,
		   POS_DEAD);
	  return;
	}
      ch->on = obj;
    }
  switch (get_position(ch))
    {
    case POS_SLEEPING:
      if (IS_AFFECTED (ch, AFF_SLEEP)
	  || is_affected (ch, skill_lookup ("Ballad of Ole Rip"))
	  || (!IS_NPC(ch) && ch->pcdata->knock_time))
	{
	  send_to_char ("You can't wake up!\n\r", ch);
	  return;
	}
      if (obj == NULL)
	{
	  send_to_char ("You wake and sit up.\n\r", ch);
	  act ("$n wakes and sits up.", ch, NULL, NULL, TO_ROOM);
	}

      else if (IS_SET (obj->value[2], SIT_AT))
	{
	  act_new ("You wake and sit at $p.", ch, obj, NULL, TO_CHAR,
		   POS_DEAD);
	  act ("$n wakes and sits at $p.", ch, obj, NULL, TO_ROOM);
	}

      else if (IS_SET (obj->value[2], SIT_ON))
	{
	  act_new ("You wake and sit on $p.", ch, obj, NULL, TO_CHAR,
		   POS_DEAD);
	  act ("$n wakes and sits at $p.", ch, obj, NULL, TO_ROOM);
	}

      else
	{
	  act_new ("You wake and sit in $p.", ch, obj, NULL, TO_CHAR,
		   POS_DEAD);
	  act ("$n wakes and sits in $p.", ch, obj, NULL, TO_ROOM);
	}
      ch->position = POS_SITTING;
      break;
    case POS_RESTING:
      if (obj == NULL)
	send_to_char ("You stop resting.\n\r", ch);

      else if (IS_SET (obj->value[2], SIT_AT))
	{
	  act ("You sit at $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n sits at $p.", ch, obj, NULL, TO_ROOM);
	}

      else if (IS_SET (obj->value[2], SIT_ON))
	{
	  act ("You sit on $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n sits on $p.", ch, obj, NULL, TO_ROOM);
	}
      ch->position = POS_SITTING;
      break;
    case POS_SITTING:
      send_to_char ("You are already sitting down.\n\r", ch);
      break;
    case POS_STANDING:
      if (obj == NULL)
	{
	  send_to_char ("You sit down.\n\r", ch);
	  act ("$n sits down on the ground.", ch, NULL, NULL, TO_ROOM);
	}

      else if (IS_SET (obj->value[2], SIT_AT))
	{
	  act ("You sit down at $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n sits down at $p.", ch, obj, NULL, TO_ROOM);
	}

      else if (IS_SET (obj->value[2], SIT_ON))
	{
	  act ("You sit on $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n sits on $p.", ch, obj, NULL, TO_ROOM);
	}

      else
	{
	  act ("You sit down in $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n sits down in $p.", ch, obj, NULL, TO_ROOM);
	}
      ch->position = POS_SITTING;
      break;
    }

  trip_triggers(ch, OBJ_TRIG_CHAR_POS_CHANGE, NULL, NULL, OT_SPEC_NONE);
  return;
}

void do_sleep (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj = NULL;
  if (ch->race == PC_RACE_SWARM)
   {
     send_to_char("Huh?\n\r",ch);
     return;
   }  
  if (is_affected (ch, skill_lookup ("Dance of Shadows")))
    {
      send_to_char ("Your nightmares keep you from falling asleep.\n\r", ch);
      return;
    }
  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are WAY too scared to sleep right now!\n\r",ch);
    return;
  }
  if (ch->fighting != NULL)
    {
      send_to_char ("You are WAY too busy to sleep now!\n\r", ch);
      return;
    }
  switch (get_position(ch))
    {
    case POS_SLEEPING:
      send_to_char ("You are already sleeping.\n\r", ch);
      break;
    case POS_MOUNTED:
      send_to_char ("You must dismount before you can sleep.\n\r", ch);
      return;
    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:
      if (argument[0] == '\0' && ch->on == NULL)
	{
	  send_to_char ("You go to sleep.\n\r", ch);
	  act ("$n goes to sleep.", ch, NULL, NULL, TO_ROOM);
	  ch->position = POS_SLEEPING;
	}

      else
	/* find an object and sleep on it */
	{
	  if (argument[0] == '\0')
	    obj = ch->on;

	  else
	    obj = get_obj_list (ch, argument, ch->in_room->contents);
	  if (obj == NULL)
	    {
	      send_to_char ("You don't see that here.\n\r", ch);
	      return;
	    }
	  if ((obj->item_type != ITEM_FURNITURE && obj->item_type != ITEM_RAFT)
	      || (!IS_SET (obj->value[2], SLEEP_ON)
		  && !IS_SET (obj->value[2], SLEEP_IN)
		  && !IS_SET (obj->value[2], SLEEP_AT)))
	    {
	      send_to_char ("You can't sleep on that!\n\r", ch);
	      return;
	    }
	  if (ch->on != obj && count_users (obj) >= obj->value[0])
	    {
	      act_new ("There is no room on $p for you.", ch, obj, NULL,
		       TO_CHAR, POS_DEAD);
	      return;
	    }
	  ch->on = obj;
	  if (IS_SET (obj->value[2], SLEEP_AT))
	    {
	      act ("You go to sleep at $p.", ch, obj, NULL, TO_CHAR);
	      act ("$n goes to sleep at $p.", ch, obj, NULL, TO_ROOM);
	    }

	  else if (IS_SET (obj->value[2], SLEEP_ON))
	    {
	      act ("You go to sleep on $p.", ch, obj, NULL, TO_CHAR);
	      act ("$n goes to sleep on $p.", ch, obj, NULL, TO_ROOM);
	    }

	  else
	    {
	      act ("You go to sleep in $p.", ch, obj, NULL, TO_CHAR);
	      act ("$n goes to sleep in $p.", ch, obj, NULL, TO_ROOM);
	    }
	  ch->position = POS_SLEEPING;
	}
      break;
    case POS_FIGHTING:
      send_to_char ("You are WAY too busy to sleep now!\n\r", ch);
      break;
    }

  trip_triggers(ch, OBJ_TRIG_CHAR_POS_CHANGE, NULL, NULL, OT_SPEC_NONE);
  return;
}

void do_wake (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  one_argument (argument, arg);
  if (arg[0] == '\0' || arg[0] == '*')
    {
      do_stand (ch, argument);
      return;
    }
  if (!IS_AWAKE (ch))
    {
      send_to_char ("You are asleep yourself!\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_AWAKE (victim))
    {
      act ("$N is already awake.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (IS_AFFECTED (victim, AFF_SLEEP)
      || is_affected (victim, skill_lookup ("Ballad of Ole Rip"))
      || (!IS_NPC(victim) && victim->pcdata->knock_time))
    {
      act ("You can't wake $M!", ch, NULL, victim, TO_CHAR);
      return;
    }
  act_new ("$n wakes you.", ch, NULL, victim, TO_VICT, POS_SLEEPING);
  do_stand (victim, "");
  return;
}

void do_sneak (CHAR_DATA * ch, char *argument)
{
  AFFECT_DATA af;
   if (!IS_NPC(ch) && ch->pcdata->flaming)
   {
      send_to_char ("Its hard to sneak while flaming!\n\r",ch);
      return;
    }
    
  send_to_char ("You attempt to move silently.\n\r", ch);
  affect_strip (ch, gsn_sneak);
  if (IS_AFFECTED (ch, AFF_SNEAK))
    return;
  if (number_percent () < get_skill (ch, gsn_sneak))
    {
      check_improve (ch, gsn_sneak, TRUE, 3);
      af.where = TO_AFFECTS;
      af.type = gsn_sneak;
      af.level = ch->level;
      af.duration = ch->level;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = AFF_SNEAK;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (ch, &af);
    }

  else
    check_improve (ch, gsn_sneak, FALSE, 3);
  return;
}

void do_hide (CHAR_DATA * ch, char *argument)
{

// Minax 6-30-02 No hiding while fighting!
  if (ch->position == POS_FIGHTING || ch->fighting != NULL)
    {
      send_to_char ("No way! You are fighting.\n\r", ch);
      return;
    }
   if (!IS_NPC(ch) && ch->pcdata->flaming)
    {
      send_to_char ("Its hard to hide while flaming!\n\r",ch);
      return;
    }
    
  send_to_char ("You attempt to hide.\n\r", ch);
  if (IS_AFFECTED (ch, AFF_HIDE))
    REMOVE_BIT (ch->affected_by, AFF_HIDE);
  if (number_percent () < get_skill (ch, gsn_hide))
    {
      SET_BIT (ch->affected_by, AFF_HIDE);
      check_improve (ch, gsn_hide, TRUE, 3);
    }

  else
    check_improve (ch, gsn_hide, FALSE, 3);
  return;
}

int check_entangle (CHAR_DATA * ch)
{
  int chance, remdur, breakfree = FALSE, dx;
  AFFECT_DATA *entaff, af;
  if (!is_affected (ch, gsn_entangle))
    return (1);
  entaff = affect_find (ch->affected, gsn_entangle);
  if (entaff == NULL)
    return (1);
  dx = get_curr_stat (ch, STAT_DEX);
  if (is_affected (ch, gsn_fireshield))
    dx += 2;
  switch (get_curr_stat (ch, STAT_DEX))
    {
    default:
      chance = number_range (1, 4);
      remdur = entaff->duration * (number_range (0, 50) * 0.01);
    case 20:
    case 21:
    case 22:
    case 23:
      chance = number_range (1, 3);
      remdur = entaff->duration * (number_range (0, 75) * 0.01);
      break;
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
      chance = number_range (1, 2);
      remdur = entaff->duration * (number_range (0, 100) * 0.01);
      break;
    }
  if (chance == 2)
    {
      remdur = UMAX (remdur, 1);
      af.where = TO_AFFECTS;
      af.type = entaff->type;
      af.level = entaff->level;
      af.duration = 0 - remdur;
      af.modifier = 0;
      af.location = 0;
      af.bitvector = 0;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_join (ch, &af);
      if (af.duration <= 0)
	{
	  send_to_char
	    ("The bonds holding you break and fall away.\n\r\n\r", ch);
	  act ("The bonds holding $n break and fall away.", ch, NULL,
	       NULL, TO_ROOM);
	  affect_strip (ch, skill_lookup ("entangle"));
	  breakfree = TRUE;
	}

      else if (remdur < 5)
	{
	  send_to_char
	    ("You struggle weakly with the entanglement holding you, it seems to loosen a bit.\n\r",
	     ch);
	  act
	    ("$n struggles weakly with the entanglement holding $m, it seems to loosen a bit.",
	     ch, NULL, NULL, TO_ROOM);
	}

      else
	{
	  send_to_char
	    ("You tear at the restraints holding you, large chunks break free.\n\r",
	     ch);
	  act
	    ("$n tears at the restraints holding $m, large chunks break free.",
	     ch, NULL, NULL, TO_ROOM);
	}
    }

  else
    {
      send_to_char ("The bonds holding you will not budge.\n\r", ch);
      act ("$n struggles ineffectually with the bonds entrapping $m.",
	   ch, NULL, NULL, TO_ROOM);
    }
  {
    int rmv;
    rmv = number_range (32, 52) - get_curr_stat (ch, STAT_DEX);
    rmv /= 2;
    if (rmv < 0)
      rmv = 3;
    if (ch->move - rmv >= 0)
      ch->move -= rmv;
  }
  return (breakfree);
}

void do_entangle (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  AFFECT_DATA af, *entaff;
  int percent;
  one_argument (argument, arg1);
  if (arg1[0] == '\0')
    {
      send_to_char ("Who would you like to entangle?\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg1)) == NULL)
    {
      send_to_char ("They are not here.\n\r", ch);
      return;
    }
  if (ch->fighting != NULL)
    {
      send_to_char ("You can't do that while you are fighting.\n\r", ch);
      return;
    }
  if (!IS_CLASS (ch, PC_CLASS_ASSASSIN) && !IS_NPC(ch))
    {
      if (!
	  (ch->in_room->sector_type == SECT_HILLS
	   || ch->in_room->sector_type == SECT_FIELD
	   || ch->in_room->sector_type == SECT_SWAMP
	   || ch->in_room->sector_type == SECT_FOREST
	   || ch->in_room->sector_type == SECT_GRASSLAND))
	{
	  send_to_char ("You can't do that here.\n\r", ch);
	  return;
	}
    }
  if (is_affected (victim, gsn_entangle))
    {
      act ("$N is already entangled.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (!IS_CLASS (ch, PC_CLASS_ASSASSIN) && ch->mana < 25)
    {
      send_to_char ("You don't have enough mana.\n\r", ch);
      return;
    }
  if (IS_CLASS (ch, PC_CLASS_ASSASSIN) && ch->move < 50)
    {
      send_to_char ("You don't have enough move.\n\r", ch);
      return;
    }
  if (is_safe (ch, victim))
    {
      send_to_char ("Not on that target", ch);
      return;
    }
  if (!IS_CLASS (ch, PC_CLASS_ASSASSIN))
    ch->mana -= 25;

  else
    ch->move /= 2;
  percent = number_percent ();
  if (victim->level > ch->level + 4)
    percent /= 2;
  if (IS_SET (victim->res_flags, RES_ENTANGLE))
    percent *= 2;
  if (IS_SET (victim->imm_flags, IMM_ENTANGLE))
    percent = 10000;
  if (percent < get_skill (ch, gsn_entangle) && (!IS_CLASS (ch, PC_CLASS_ASSASSIN)) && !IS_NPC(ch))
    {
      act ("You manage to entangle $N using nearby plant life.", ch,
	   NULL, victim, TO_CHAR);
      act ("$n has entangled you, using nearby plant life.", ch, NULL,
	   victim, TO_VICT);
      act ("$n has entangled $N using nearby plant life.", ch, NULL,
	   victim, TO_NOTVICT);
      check_improve (ch, gsn_entangle, TRUE, 3);
      af.where = TO_AFFECTS;
      af.type = gsn_entangle;
      af.level = ch->level;
      af.duration = ch->level / 3;
      af.modifier = 0;
      af.location = 0;
      af.bitvector = 0;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      entaff = affect_find (ch->affected, skill_lookup ("entangle"));
      if (entaff != NULL)
	{
	  if (entaff->duration + af.duration > 20)
	    {
	      affect_strip (ch, skill_lookup ("entangle"));
	      af.duration = 20;
	    }
	}

      else if (af.duration > 20)
	af.duration = 20;
      affect_join (victim, &af);
      return;
    }
// Minax 7-7-02 Entangle message should not have forest references for Assassins
  if (percent < get_skill (ch, gsn_entangle) && IS_CLASS (ch, PC_CLASS_ASSASSIN))
    {
      act ("You manage to entangle $N using various ropes and materials.",
	   ch, NULL, victim, TO_CHAR);
      act ("$n has entangled you, using various ropes and materials.", ch,
	   NULL, victim, TO_VICT);
      act ("$n has entangled $N using various ropes and materials.", ch,
	   NULL, victim, TO_NOTVICT);
      check_improve (ch, gsn_entangle, TRUE, 3);
      af.where = TO_AFFECTS;
      af.type = gsn_entangle;
      af.level = ch->level;
      af.duration = ch->level / 3;
      af.modifier = 0;
      af.location = 0;
      af.bitvector = 0;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      entaff = affect_find (ch->affected, skill_lookup ("entangle"));
      if (entaff != NULL)
	{
	  if (entaff->duration + af.duration > 20)
	    {
	      affect_strip (ch, skill_lookup ("entangle"));
	      af.duration = 20;
	    }
	}

      else if (af.duration > 20)
	af.duration = 20;
      affect_join (victim, &af);
      return;
    }

  else
    {
      act ("$N manages to evade your entanglement attempt.", ch, NULL,
	   victim, TO_CHAR);
      check_improve (ch, gsn_entangle, FALSE, 3);
    }
  return;
}

void do_camouflage (CHAR_DATA * ch, char *argument)
{

// Minax 6-30-02 Can't camouflage while fighting.  Also, if you camouflage while
// following someone, you will stop following them.  This prevents people from
// using camouflage in the woods, then being led into town, where camouflage is
// inappropriate.
// Iblis 5-10-03 In order to fix the bug that allowed any char to break out of charm
// by attempting to camoflauge (and then were immune to charm until it wore off),
// It has been changed so you CAN camoflauge while following someone, but as soon as
// you follow them into another room, you become visible.  Thus I commented out Minax's
// changed below.
  if (ch->position == POS_FIGHTING || ch->fighting != NULL)
    {
      send_to_char ("No way! You are fighting.\n\r", ch);
      return;
    }
  if (!IS_NPC(ch) && ch->pcdata->flaming)
  {
	  send_to_char ("Its hard to camoflauge while flaming!\n\r",ch);
	  return;
  }

  send_to_char ("You attempt to camouflage yourself.\n\r", ch);
  if (IS_AFFECTED (ch, AFF_CAMOUFLAGE))
    REMOVE_BIT (ch->affected_by, AFF_CAMOUFLAGE);
  if (number_percent () < get_skill (ch, gsn_camouflage) &&
      ((ch->in_room->sector_type == SECT_HILLS
       || ch->in_room->sector_type == SECT_FIELD
       || ch->in_room->sector_type == SECT_FOREST
       || ch->in_room->sector_type == SECT_GRASSLAND)))
    {
      SET_BIT (ch->affected_by, AFF_CAMOUFLAGE);
      check_improve (ch, gsn_camouflage, TRUE, 3);
    }

  else
    check_improve (ch, gsn_camouflage, FALSE, 3);
  return;
}

void do_stalk (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Stalk whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (victim == ch)
    {
      if (ch->stalking != NULL)
	{
	  act ("You stop stalking $N.", ch, NULL, ch->stalking, TO_CHAR);
	  ch->stalking = NULL;
	}
      return;
    }
  if (IS_IMMORTAL (victim))
    {
      send_to_char ("You can't stalk an immortal.\n\r", ch);
      return;
    }
  if (ch->mount != NULL)
    {
      send_to_char ("You can't stalk someone when you are mounted.\n\r", ch);
      return;
    }
  if (number_percent () > get_skill (ch, gsn_stalk) && !(IS_NPC (ch)))
    {
      send_to_char ("You failed.\n\r", ch);
      check_improve (ch, gsn_stalk, FALSE, 2);
      return;
    }
  act ("You are now stalking $N.", ch, NULL, victim, TO_CHAR);
  check_improve (ch, gsn_stalk, TRUE, 2);
  ch->stalking = victim;
}

void do_visible (CHAR_DATA * ch, char *argument)
{
  affect_strip (ch, gsn_invis);
  affect_strip (ch, gsn_astral_body);
  affect_strip (ch, gsn_mass_invis);
  affect_strip (ch, gsn_heavenly_cloak);
  affect_strip (ch, gsn_sneak);
  affect_strip (ch, gsn_vanish);
  REMOVE_BIT (ch->affected_by, AFF_HIDE);
  REMOVE_BIT (ch->affected_by, AFF_CAMOUFLAGE);
  REMOVE_BIT (ch->affected_by, AFF_INVISIBLE);
  REMOVE_BIT (ch->affected_by, AFF_SNEAK);
  send_to_char ("Ok.\n\r", ch);
  return;
}

void do_fly (CHAR_DATA * ch, char *argument)
{
  if (ch->race != PC_RACE_SIDHE && ch->race != PC_RACE_KALIAN && ch->race != PC_RACE_NERIX)
    {
      send_to_char ("You don't have the power to fly at will.\n\r", ch);
      return;
    }
  //12-29-03 Iblis - By Minax's request, if you're overweight you can't fly
  if (get_carry_weight (ch) > can_carry_w (ch))
  {
    send_to_char("You are too heavy to fly.\n\r",ch);
    return;
  }
  if (IS_SET (ch->affected_by, AFF_FLYING))
    {
      send_to_char ("You are already flying.\n\r", ch);
      return;
    }
  SET_BIT (ch->affected_by, AFF_FLYING);
  if (ch->race == PC_RACE_SIDHE)
  {
    send_to_char ("You raise your wings and gently float into the air.\n\r", ch);
    act ("$n raises $s wings and gently floats into the air.", ch, NULL, NULL, TO_ROOM);
  }
  else if (ch->race == PC_RACE_KALIAN)
  {
    send_to_char ("You slowly levitate into the air.\n\r",ch);
    act ("$n slowly levitates into the air.\n\r",ch,NULL,NULL, TO_ROOM);
  }
  else 
  {
    send_to_char ("With powerful strokes from your humongus wings, you become airborne.\n\r",ch);
    act ("With powerful strokes from $s humongous wings, $n becomes airborne.",ch,NULL,NULL,TO_ROOM);
  }
}

void do_land (CHAR_DATA * ch, char *argument)
{
  AFFECT_DATA *aff, *aff_next;
  bool found = FALSE;
  for (aff = ch->affected; aff != NULL; aff = aff_next)
    {
      aff_next = aff->next;
      if (aff->bitvector == AFF_FLYING)
	{
	  affect_remove (ch, aff);
	  found = TRUE;
	}
    }
  if (IS_SET (ch->affected_by, AFF_FLYING))
    {
      found = TRUE;
      REMOVE_BIT (ch->affected_by, AFF_FLYING);
    }
  if (found)
  {
    send_to_char ("You float slowly to the ground.\n\r", ch);
    act("$n floats slowly to the ground.",ch,NULL,NULL,TO_ROOM);
  }

  else
    send_to_char ("Your feet are already firmly on the ground.\n\r", ch);
  return;
}

void do_recall (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *location;
  if (ch->level > 10)
    {
      send_to_char ("You need to find another way back to town.\n\r", ch);
      return;
    }
  if (ch->dueler || (!IS_NPC(ch) && IS_SET (ch->act, PLR_JAILED)) || (ch->in_room && ch->in_room->vnum == 23055))
  {
    send_to_char("Nuh uh!\n\r",ch);
    return;
  }
  if (IS_NPC (ch) && !IS_SET (ch->act, ACT_PET))
    {
      send_to_char ("Only players can recall.\n\r", ch);
      return;
    }
  act ("$n prays for transportation!", ch, 0, 0, TO_ROOM);
  if ((location = get_room_index (ROOM_VNUM_TEMPLE)) == NULL)
    {
      send_to_char ("You are completely lost.\n\r", ch);
      return;
    }
  if (ch->in_room == location)
    {
      send_to_char ("You are already there.\n\r", ch);
      return;
    }
  if (ch->in_room == NULL)
    return;
  if (!IS_NPC (ch) && ch->level > 10)
    if ((time (NULL) - ch->pcdata->last_fight < 120) && (!IS_IMMORTAL(ch)))
      {
        send_to_char ("Nice try.  You can't recall so easily.\n\r", ch);
        return;
      }
  if ((victim = ch->fighting) != NULL)
    {
      int lose;
      if (number_percent () < 20)
	{
	  WAIT_STATE (ch, 4);
	  sprintf (buf, "You failed!.\n\r");
	  send_to_char (buf, ch);
	  return;
	}
      lose = level_cost (ch->level) / 8.0;
      gain_exp (ch, 0 - lose);
      sprintf (buf, "You recall from combat!  You lose %d exps.\n\r", lose);
      send_to_char (buf, ch);
      stop_fighting (ch, TRUE);
    }
  if (ch->pcdata)
    ch->pcdata->fishing = 0;
  ch->move /= 2;
  act ("$n disappears.", ch, NULL, NULL, TO_ROOM);
  char_from_room (ch);
  char_to_room (ch, location);
  act ("$n appears in the room.", ch, NULL, NULL, TO_ROOM);
  do_look (ch, "auto");
  trap_check(ch,"room",ch->in_room,NULL);  
  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  sprintf(buf,"%d",ch->in_room->vnum);
  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,buf);
  check_aggression(ch);
  if (ch->pet != NULL)
    do_recall (ch->pet, "");

  trip_triggers(ch, OBJ_TRIG_CHAR_RECALLS, NULL, NULL, OT_SPEC_NONE);
  return;
}

void do_train (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *mob;
  sh_int stat = -1;
  char *pOutput = NULL;
  int cost;
  if (IS_NPC (ch))
    return;

  /*
   * Check for trainer.
   */
  for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
    {
      if (IS_NPC (mob) && IS_SET (mob->act, ACT_TRAIN))
	break;
    }
  if (mob == NULL)
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
    }
  if (argument[0] == '\0')
    {
      sprintf (buf, "You have %d training sessions.\n\r", ch->train);
      send_to_char (buf, ch);
      argument = "foo";
    }
  cost = 1;

  //Iblis 6/07/03 - It seems this Class_table stuff is useless, so I commented it out
  if (!str_cmp (argument, "str"))
    {
      stat = STAT_STR;
      pOutput = "strength";
    }

  else if (!str_cmp (argument, "int"))
    {
      stat = STAT_INT;
      pOutput = "intelligence";
    }

  else if (!str_cmp (argument, "wis"))
    {
      stat = STAT_WIS;
      pOutput = "wisdom";
    }

  else if (!str_cmp (argument, "dex"))
    {
      stat = STAT_DEX;
      pOutput = "dexterity";
    }

  else if (!str_cmp (argument, "con"))
    {
      stat = STAT_CON;
      pOutput = "constitution";
    }

  else if (!str_cmp (argument, "cha"))
    {
      stat = STAT_CHA;
      pOutput = "charisma";
    }

  else
    {
      strcpy (buf, "You can train:");
      if (ch->perm_stat[STAT_STR] < get_max_train (ch, STAT_STR))
	strcat (buf, " str");
      if (ch->perm_stat[STAT_INT] < get_max_train (ch, STAT_INT))
	strcat (buf, " int");
      if (ch->perm_stat[STAT_WIS] < get_max_train (ch, STAT_WIS))
	strcat (buf, " wis");
      if (ch->perm_stat[STAT_DEX] < get_max_train (ch, STAT_DEX))
	strcat (buf, " dex");
      if (ch->perm_stat[STAT_CON] < get_max_train (ch, STAT_CON))
	strcat (buf, " con");
      if (ch->perm_stat[STAT_CHA] < get_max_train (ch, STAT_CHA))
	strcat (buf, " cha");

      if (buf[strlen (buf) - 1] != ':')
	{
	  strcat (buf, ".\n\r");
	  send_to_char (buf, ch);
	}

      else
	{

	  /*
	   * This message dedicated to Jordan ... you big stud!
	   */
	  act ("You have nothing left to train, you $T!", ch, NULL,
	       ch->sex == SEX_MALE ? "big stud" : ch->sex ==
	       SEX_FEMALE ? "hot babe" : "wild thing", TO_CHAR);
	}
      return;
    }
  if (ch->perm_stat[stat] >= get_max_train (ch, stat))
    {
      act ("Your $T is already at maximum.", ch, NULL, pOutput, TO_CHAR);
      return;
    }
  if (cost > ch->train)
    {
      send_to_char ("You don't have enough training sessions.\n\r", ch);
      return;
    }
  ch->train -= cost;
  ch->perm_stat[stat] += 1;
  act ("Your $T increases!", ch, NULL, pOutput, TO_CHAR);
  act ("$n's $T increases!", ch, NULL, pOutput, TO_ROOM);
  return;
}

bool can_use_room (CHAR_DATA * ch, ROOM_INDEX_DATA * location)
{
  if (IS_IMMORTAL (ch))
    return (TRUE);
  if (location->Class_flags != 0)
    {

      if (!room_Class_check (ch, location))
	{
	  return FALSE;
	}
    }
  if (location->race_flags != 0)
  {
    if (!room_race_check(ch,location))
      {
        return FALSE;
      }
  }
  return (TRUE);
}

bool can_move_char_door (CHAR_DATA * ch, int door, bool follow, bool show_messages)
{
	return can_move_char_door_new(ch,door,follow,show_messages,POS_FIGHTING);
}
	

//IBLIS 6/1/03 - A simple cut/paste/comment out of move_char.
//This version sends NO messages to the char, and returns either
//a false or a true if the character is allowed to move the
//door direction.  This is used particularly in the monk dive skill,
//but could be used in other places (I may modify it later to make
//it more useful).
bool
can_move_char_door_new (CHAR_DATA * ch, int door, bool follow, bool show_messages, int position)
{

  ROOM_INDEX_DATA *in_room;
  ROOM_INDEX_DATA *to_room;
  EXIT_DATA *pexit;
  CHAR_DATA* blocker;
  char tcbuf[MAX_STRING_LENGTH];

  if (door < 0 || door > 5)
    {
      bug ("Do_move: bad door %d.", door);
      return FALSE;
    }
  if (IS_NPC (ch) && ch->riders != NULL)
    return FALSE;
  if (ch->position == POS_TETHERED)
    return FALSE;
  if (!check_web (ch))
    return FALSE;
  if (!check_entangle (ch))
    return FALSE;
  if (!check_clasp (ch))
    return FALSE;  
  if (ch->position < position || ch->position >= POS_FEIGNING_DEATH)
    return FALSE;
  in_room = ch->in_room;
  if ((pexit = in_room->exit[door]) == NULL
      || (to_room = pexit->u1.to_room) == NULL
      || !can_see_room (ch, pexit->u1.to_room)
      || !br_vnum_check (to_room->vnum))
    {
      if (!IS_NPC (ch))
	ch->pcdata->speedlen = 0;
      return FALSE;
    }

  for (blocker = ch->in_room->people; blocker != NULL;
       blocker = blocker->next_in_room)
    {
      if (!IS_SET (blocker->act2, ACT_BLOCK_EXIT)
          || blocker->blocks_exit == -1)
        continue;
      if (door != blocker->blocks_exit)
        continue;
      if (IS_IMMORTAL (ch) || IS_SET(ch->act2,ACT_NO_BLOCK))
	{
	  sprintf (tcbuf, "%s would have blocked your way %s, except you're an Immortal.\n\r",
		   blocker->short_descr, dir_name[blocker->blocks_exit]);
	  if (show_messages)
	    send_to_char(tcbuf,ch);
	  break;
	}
      sprintf (tcbuf, "%s blocks your way %s...\n\r",
               blocker->short_descr, dir_name[blocker->blocks_exit]);
      if (show_messages)
	send_to_char (tcbuf, ch);
      return FALSE;
    }



  // Mathew 4/16/99: Fixed the logic of the closed door check to make^M
  //   // more sense.^M
  if (IS_SET (pexit->exit_info, EX_CLOSED) &&
      (!IS_AFFECTED (ch, AFF_PASS_DOOR)
       || IS_SET (pexit->exit_info, EX_NOPASS)) && !IS_IMMORTAL(ch)
    )
    {
      if (show_messages)
	act ("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
      return FALSE;
    }
  return can_move_char (ch, to_room, follow, show_messages);
}

//Iblis - a function that is much more useful than cutting and pasting this whole goddamn mess of crap
//into each function you want to check if a player can move into a room.  This allows allows one 
//command to be used to check all reasons players cannot enter rooms, fixing numerous buggy ways
//to get around room restrctions that have been present since the dawn of Exodus.  You can either
//have it send the character the proper message as to why they cannot enter the room, or just handle
//the message in the function that calls this
bool can_move_char (CHAR_DATA * ch, ROOM_INDEX_DATA * to_room, bool follow, bool show_messages)
{

  ROOM_INDEX_DATA *in_room;

  char tcbuf[MAX_STRING_LENGTH];
  if (IS_NPC (ch) && ch->riders != NULL)
    return FALSE;
  if (ch->position == POS_TETHERED)
    return FALSE;
  if (!check_web (ch))
    return FALSE;
  if (!check_entangle (ch))
    return FALSE;
  if (!check_clasp (ch))
    return FALSE;
  if (IS_SET(to_room->area->area_flags,AREA_IMP_ONLY) && get_trust(ch) < MAX_LEVEL)
    return FALSE;
  if (ch->position >= POS_FEIGNING_DEATH && ch->position != POS_COMA)
    return FALSE;

  if (IS_NPC (ch) && IS_SET(ch->act2,ACT_PUPPET))
  {
    if(ch->pIndexData->vnum <= MOB_VNUM_WARPED_OWL && ch->pIndexData->vnum >= MOB_VNUM_FROG)
    {
      if(ch->desc && ch->desc->original && ch->in_room)
      {
	 return can_move_char(ch->desc->original,ch->in_room,follow,show_messages);
      }
    }
  }
  

  in_room = ch->in_room;

  if (IS_SET(to_room->area->area_flags,AREA_SOLO) && (to_room->area->nplayer > 0) &&
		  to_room->area != in_room->area)
  {
    if (show_messages)
      send_to_char("That area is occupied!\n\r",ch);
    return FALSE;
  }

  if (IS_SET(to_room->room_flags2,ROOM_SOLO) && !IS_NPC(ch))
  {
    CHAR_DATA *mch;
	for (mch = to_room->people;mch != NULL; mch = mch->next_in_room)
	{
  	  if (!IS_NPC(mch))
	  {
 	    if (show_messages)
  	      send_to_char("That room is occupied!\n\r",ch);
	    return FALSE;
	  }
	}
  }
			
  
  if (!br_vnum_check (to_room->vnum))
    {
      if (show_messages)
	send_to_char
	  ("That room is off limits during the `iBattle Royale``.\r\n", ch);
      return FALSE;
    }
  if (ch->pcdata && ch->pcdata->fishing)
    {
      if (show_messages)
	send_to_char ("You can't move while you're fishing!.\n\r", ch);
      return FALSE;
    }
  if (IS_AFFECTED (ch, AFF_CHARM)
      && ch->master != NULL && in_room == ch->master->in_room && !IS_NPC (ch))
    {
      if (show_messages)
	send_to_char ("What?  And leave your beloved master?\n\r", ch);
      return FALSE;
    }
  if (ch->mount != NULL)
    if (ch->mount->position != POS_STANDING
	&& ch->mount->position != POS_FIGHTING)
      {
	if (show_messages)
	  {
	    sprintf (tcbuf,
		     "%s will have to stand before you can ride it any further.\n\r",
		     fcapitalize (ch->mount->short_descr));
	    send_to_char (tcbuf, ch);
	  }
	return FALSE;
      }
  if (IS_SET (to_room->room_flags, ROOM_NOMOUNT) && ch->mount != NULL
      && !IS_AFFECTED (ch->mount, AFF_FLYING)
      && !IS_AFFECTED (ch->mount, AFF_SWIM))
    {
      if (show_messages)
	{
	  send_to_char ("You may not ride mounts in there.\n\r", ch);
	}
      return FALSE;
    }
  if (IS_SET (to_room->room_flags, ROOM_NOSWIMMOUNT)
      && ch->mount != NULL && IS_AFFECTED (ch->mount, AFF_SWIM))
    {
      if (show_messages)
	send_to_char ("You may not swim with mounts in there.\n\r", ch);
      return FALSE;
    }
  if (IS_SET (to_room->room_flags, ROOM_NOFLYMOUNT)
      && ch->mount != NULL && IS_AFFECTED (ch->mount, AFF_FLYING))
    {
      if (show_messages)
	send_to_char ("You may not fly mounts in there.\n\r", ch);
      return FALSE;
    }
  if ((ch->riders != NULL || ch->mounter != NULL)
      && IS_SET (to_room->room_flags,
		 ROOM_NOMOUNT) && (!IS_AFFECTED (ch, AFF_FLYING)
				   && !IS_AFFECTED (ch, AFF_SWIM)))
    {
      if (show_messages)
	send_to_char ("You can't go in there, mount.\n", ch);
      return FALSE;
    }
  if ((ch->riders != NULL || ch->mounter != NULL)
      && IS_SET (to_room->room_flags, ROOM_NOFLYMOUNT)
      && IS_AFFECTED (ch, AFF_FLYING))
    {
      if (show_messages)
	send_to_char ("You can't fly in there, mount.\n", ch);
      return FALSE;
    }
  if ((ch->riders != NULL || ch->mounter != NULL)
      && IS_SET (to_room->room_flags, ROOM_NOSWIMMOUNT)
      && IS_AFFECTED (ch, AFF_SWIM))
    {
      if (show_messages)
	send_to_char ("You can't swim in there, mount.\n", ch);
      return FALSE;
    }
  if (to_room->max_level != 0 && ch->level > to_room->max_level
      && !IS_IMMORTAL (ch))
    {
      if (show_messages)
	send_to_char ("You aren't allowed in there.\n\r", ch);
      return FALSE;
    }
// Minax 2-13-03 Keep charmies out of no_charmie rooms.
  if (IS_AFFECTED (ch, AFF_CHARM)
      && (IS_SET (to_room->room_flags, ROOM_NO_CHARMIE)))
    {
      if (show_messages)
	send_to_char
	  ("You aren't allowed in there because you are charmed.\n\r", ch);
      return FALSE;
    }
  if (to_room->Class_flags != 0 && !IS_IMMORTAL (ch))
    {
      if (!room_Class_check (ch, to_room))
	{
	  if (show_messages)
	    send_to_char ("You aren't allowed in there.\n\r", ch);
	  return FALSE;
	}
    }
  if ((to_room->clan != CLAN_BOGUS) && (ch->clan != to_room->clan)
      && !IS_IMMORTAL (ch) && !IS_SET (ch->act, ACT_PET) && !IS_SET (ch->act2, ACT_PUPPET))
    {
      if (show_messages)
	send_to_char ("You aren't allowed in there.\n\r", ch);
      return FALSE;
    }
  if (to_room->race_flags != 0)
  {
    if (!room_race_check(ch,to_room))
    {
    if (show_messages)
      send_to_char ("You aren't allowed in there.\n\r", ch);
    return FALSE;
    }
  }
  if (IS_SET(to_room->room_flags2,ROOM_PKONLY) && !IS_NPC(ch) && ch->pcdata->loner == FALSE && ch->clan == CLAN_BOGUS)
  {
    if (show_messages)
      send_to_char("NoPK's aren't allowed in there.\n\r",ch);
    return FALSE;
  }
  if (!is_room_owner (ch, to_room) && room_is_private (to_room))
    {
      if (show_messages)
	{
	  if (to_room->max_message[0] == '\0')
	    send_to_char ("That room is private right now.\n\r", ch);

	  else
	    {
	      send_to_char (to_room->max_message, ch);
	      send_to_char ("\n\r", ch);
	    }
	}
      return FALSE;
    }
  //Iblis 6/21/03 - Battle Royale room off limit checking
  if (!br_vnum_check (to_room->vnum))
    {
      if (show_messages)
	send_to_char
	  ("That room is off limits during `iBattle Royale``\r\n", ch);
      return FALSE;
    }
  if (!IS_IMMORTAL (ch) && IS_SET (to_room->room_flags, ROOM_IMP_ONLY))
    {
      if (show_messages)
	send_to_char ("Sorry, you can't go in there.\n\r", ch);
      return FALSE;
    }
  if (IS_SET (to_room->room_flags, ROOM_SPECTATOR))
    {
      OBJ_DATA *obj, *obj_next;
      bool found = FALSE;
      if (IS_IMMORTAL (ch))
	found = TRUE;

      else
	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	  {
	    obj_next = obj->next_content;
	    if (obj->pIndexData->vnum == OBJ_VNUM_ATICKET)
	      {
		found = TRUE;
		break;
	      }
	  }
      if (!found)
	{
	  if (show_messages)
	    {
	      send_to_char
		("A burly bouncer smirks and says 'Trying to sneak in here without a ticket?  I don't think so.'\n\r",
		 ch);
	    }
	  return FALSE;
	}
    }
  if (to_room->vnum == 4334)
    {
      OBJ_DATA *obj, *obj_next;
      bool found = FALSE;
      if (IS_IMMORTAL (ch))
	found = TRUE;

      else
	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	  {
	    obj_next = obj->next_content;
	    if (obj->pIndexData->vnum == OBJ_VNUM_ZTICKET)
	      {
		found = TRUE;
		break;
	      }
	  }
      if (!found)
	{
	  if (show_messages)
	    {
	      send_to_char
		("A zoo attendant stops you and says 'Sorry, you need a ticket to get in there.'\n\r",
		 ch);
	    }
	  return FALSE;
	}
    }
  if (!IS_NPC (ch))
    {
      int iClass, iGuild;
      int move;
      for (iClass = 0; iClass < MAX_CLASS; iClass++)
	{
	  for (iGuild = 0; iGuild < MAX_GUILD; iGuild++)
	    {
	      if (!IS_CLASS (ch, iClass)
		  && to_room->vnum == Class_table[iClass].guild[iGuild])
		{
		  if (show_messages)
		    send_to_char ("You aren't allowed in there.\n\r", ch);
		  return FALSE;
		}
	    }
	}
      if (IS_SET (to_room->room_flags, ROOM_NO_FLY))
	{
	  if (ch->mount == NULL)
	    {
	      if (IS_AFFECTED (ch, AFF_FLYING) && !IS_IMMORTAL (ch))
		{
		  if (show_messages)
		    send_to_char ("You can't fly there.\n\r", ch);
		  return FALSE;
		}
	    }

	  else
	    {
	      if (IS_AFFECTED (ch->mount, AFF_FLYING))
		{
		  if (show_messages)
		    send_to_char ("You can't fly your mount there.\n\r", ch);
		  return FALSE;
		}
	    }
	}
      if (in_room->sector_type == SECT_AIR
	  || to_room->sector_type == SECT_AIR)
	{
	  if (ch->mount == NULL)
	    {
	      if (!IS_AFFECTED (ch, AFF_FLYING) && !IS_IMMORTAL (ch))
		{
		  if (show_messages)
		    send_to_char ("You can't fly.\n\r", ch);
		  return FALSE;
		}
	    }

	  else
	    {
	      if (!IS_AFFECTED (ch->mount, AFF_FLYING))
		{
		  if (show_messages)
		    send_to_char ("Your mount cannot fly.\n\r", ch);
		  return FALSE;
		}
	    }
	}

      if ((in_room->sector_type == SECT_WATER_NOSWIM
	   || to_room->sector_type == SECT_WATER_NOSWIM
	   || to_room->sector_type == SECT_WATER_OCEAN
	   || in_room->sector_type == SECT_WATER_OCEAN)
	  && !IS_AFFECTED (ch, AFF_FLYING))
	{
	  bool found = FALSE;
	  OBJ_DATA *mObj;
	  if (ch->mount != NULL && IS_AFFECTED (ch->mount, AFF_FLYING))
	    found = TRUE;

	  else if (IS_IMMORTAL (ch))
	    found = TRUE;

	  else
	    for (mObj = ch->carrying; mObj; mObj = mObj->next_content)
	      {
		if (mObj->item_type == ITEM_RAFT)
		  {
		    found = TRUE;
		    if (to_room->sector_type == SECT_WATER_OCEAN)
		      {
			short chance;
			switch (weather_info.sky)
			  {
			  case SKY_RAINING:
			  case SKY_LIGHTNING:
			    chance = 75;
			    break;
			  case SKY_CLOUDY:
			    chance = 50;
			    break;
			  default:
			    chance = 25;
			    break;
			  }

		      }
		    break;
		  }
	      }
	}
      move = movement_loss[UMIN (SECT_MAX - 1, in_room->sector_type)]
	+ movement_loss[UMIN (SECT_MAX - 1, to_room->sector_type)];
      move /= 2;		/* i.e. the average */
      if (to_room->sector_type == SECT_UNDERWATER
	  && IS_AFFECTED (ch, AFF_AQUA_BREATHE))
	move = 4;

      /*Iblis 12-29-03 - Check swimming everytime you move from a water room to a water room*/
       if ((to_room->sector_type == SECT_WATER_OCEAN
          || to_room->sector_type == SECT_WATER_SWIM
          || to_room->sector_type == SECT_UNDERWATER) 
          &&
	   (ch->in_room->sector_type == SECT_WATER_OCEAN
	    || ch->in_room->sector_type == SECT_WATER_NOSWIM
	    || ch->in_room->sector_type == SECT_UNDERWATER))
       {
	 if (!IS_AFFECTED (ch, AFF_AQUA_BREATHE) && !IS_IMMORTAL (ch) && !(get_skill(ch,gsn_swim) > number_range(1,100)) && !IS_SET(ch->imm_flags,IMM_DROWNING) && ch->move > 0)
	   {
	     OBJ_DATA *obj;
	     bool found;

	     found = FALSE;

	     for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	       {
		 if (obj->item_type == ITEM_GILLS
		     && obj->wear_loc != WEAR_NONE)
		   {
		     found = TRUE;
		     break;
		   }
	       }

	     if (!found)
	       {
		 ch->mana -= ch->max_mana/15.0 + (number_range(1,10)-5);
		 ch->move -= ch->max_move/15.0 + (number_range(1,10)-5);

		 send_to_char ("Your lungs fill with water.\n\r", ch);
		 act ("$n turns purple floundering for air.", ch, NULL, NULL, TO_ROOM);
		 //Iblis 10/29/04 - Changed to prevent crashing when mobs drowned on move
		 if (ch->hit <= ch->max_hit/15.0)
		 {
			 ch->hit = 1;
			 if (ch->move > 0)
				 ch->move = -1;
		 }
		 else ch->hit -= ch->max_hit/15.0;
	       }
	   }
       //12-30-03 Fail this swimming check, and you go off in a random direction
       if (get_skill(ch,gsn_swim) < number_range(1,100))
       {
         int door=0, counter;
	 EXIT_DATA *pexit;
         for (door = number_range (0, 5), counter = 0; counter < 6;
 	    counter++, door++)
	 {
	   if (door > 5)
	     door = door % 6;
	   if ((pexit = in_room->exit[door]) == NULL
	       || (to_room = pexit->u1.to_room) == NULL
	       || !can_see_room (ch, pexit->u1.to_room)
	       || (IS_SET (pexit->exit_info, EX_CLOSED) &&
	           (!IS_AFFECTED (ch, AFF_PASS_DOOR)|| IS_SET (pexit->exit_info, EX_NOPASS))))
	       
	     continue;
	   if (IS_SET (pexit->exit_info, EX_CLOSED) &&
              (!IS_AFFECTED (ch, AFF_PASS_DOOR)
               || IS_SET (pexit->exit_info, EX_NOPASS)))
	      continue;

          
            break;
	 }
       if (counter > 5)
	 {
           if (show_messages)
  	     send_to_char ("Alas, you cannot go that way.\n\r", ch);
	   return FALSE;
	 }
       }
    }

      
      /* conditional effects */
      if (get_carry_weight (ch) > can_carry_w (ch))
        move *= 2;
      if (IS_AFFECTED (ch, AFF_FLYING) || IS_AFFECTED (ch, AFF_HASTE))
	move /= 2;
      if (IS_AFFECTED (ch, AFF_SLOW))
	move *= 2;
      if (!IS_NPC (ch) && ch->pcdata->speedlen != 0)
	move *= 2;
      if (ch->mount == NULL && ch->move < move)
	{
	  if (show_messages)
	    send_to_char ("You are too exhausted.\n\r", ch);
	  if (!IS_NPC (ch))
	    ch->pcdata->speedlen = 0;
	  return FALSE;
	}
       } 

  if (ch->mount != NULL)
    {

      int move;
      move = movement_loss[UMIN (SECT_MAX - 1, in_room->sector_type)]
	+ movement_loss[UMIN (SECT_MAX - 1, to_room->sector_type)];
      move /= 2;		/* i.e. the average */
      if (to_room->sector_type == SECT_UNDERWATER
	  && IS_AFFECTED (ch, AFF_AQUA_BREATHE))
	move = 4;

      if ((to_room->sector_type == SECT_WATER_OCEAN
          || to_room->sector_type == SECT_WATER_SWIM
          || to_room->sector_type == SECT_WATER_NOSWIM
	   || to_room->sector_type == SECT_UNDERWATER)
          &&
	  (ch->in_room->sector_type == SECT_WATER_OCEAN
            || ch->in_room->sector_type == SECT_WATER_SWIM
            || ch->in_room->sector_type == SECT_WATER_NOSWIM
	   || ch->in_room->sector_type == SECT_UNDERWATER))
	{
	  if (!IS_AFFECTED (ch, AFF_AQUA_BREATHE) && !IS_IMMORTAL (ch)&& !(get_skill(ch,gsn_swim) > number_range(1,100)) && !IS_SET(ch->imm_flags,IMM_DROWNING) && ch->move > 0)
	    {
	      OBJ_DATA *obj;
	      bool found;

	      found = FALSE;

	      for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		{
		  if (obj->item_type == ITEM_GILLS
		      && obj->wear_loc != WEAR_NONE)
		    {
		      found = TRUE;
		      break;
		    }
		}

	      if (!found)
		{
		  ch->mana -= ch->max_mana/15.0 + (number_range(1,10)-5);
		  ch->move -= ch->max_move/15.0 + (number_range(1,10)-5);

		  send_to_char ("Your lungs fill with water.\n\r", ch);
		  act ("$n turns purple floundering for air.", ch, NULL, NULL, TO_ROOM);
		  //Iblis 10/29/04 - Changed to prevent crashing when mobs drowned on move
		  if (ch->hit <= ch->max_hit/15.0)
		  {
			  ch->hit = 1;
			  if (ch->move > 0)
				  ch->move = -1;
		  }
		  else ch->hit -= ch->max_hit/15.0;
		}
	    }
	
      //12-30-03 Fail this swimming check, and you go off in a random direction
      if (get_skill(ch,gsn_swim) < number_range(1,100))
       {
        int door=0, counter=0;
	EXIT_DATA *pexit;
        for (door = number_range (0, 5), counter = 0; counter < 6;
           counter++, door++)
        {
          if (door > 5)
            door = door % 6;
          if ((pexit = in_room->exit[door]) == NULL
              || (to_room = pexit->u1.to_room) == NULL
              || !can_see_room (ch, pexit->u1.to_room)
              || (IS_SET (pexit->exit_info, EX_CLOSED) &&
                 (!IS_AFFECTED (ch, AFF_PASS_DOOR)|| IS_SET (pexit->exit_info, EX_NOPASS))))
            continue;

          else
            break;
        }
      if (counter > 5)
        {
          if (show_messages)
	    send_to_char ("Alas, you cannot go that way.\n\r", ch);
          return FALSE;
        }
       }
	}


      /* conditional effects */
      if (get_carry_weight (ch) > can_carry_w (ch))
        move *= 2;
      if (IS_AFFECTED (ch, AFF_FLYING) || IS_AFFECTED (ch, AFF_HASTE))
	move /= 2;
      if (IS_AFFECTED (ch, AFF_SLOW))
	move *= 2;
      if (!IS_NPC (ch) && ch->pcdata->speedlen != 0)
	move *= 2;
      if (ch->mount->move < move)
	{
	  if (show_messages)
	    send_to_char ("Your mount is too exhausted.\n\r", ch);
	  if (!IS_NPC (ch))
	    ch->pcdata->speedlen = 0;
	  return FALSE;
	}
      if (ch->mount->hit <= 0)
	{
	  if (show_messages)
	    send_to_char
	      ("Your mount is in no shape to move around.\n\r", ch);
	  if (!IS_NPC (ch))
	    ch->pcdata->speedlen = 0;
	  return FALSE;
	}
    }
  return TRUE;
}


//IBLIS 6/20/03 - Should have done this a long time ago.  A much simpler way to handle checking
//                if a char can enter a room of a certain Class
bool room_Class_check (CHAR_DATA * ch, ROOM_INDEX_DATA * to_room)
{
  bool can_enter = FALSE;
  if (IS_SET(ch->act2,ACT_PUPPET))
    return TRUE;
  if (IS_SET (to_room->Class_flags, CLASS_MAGE)
      && (IS_CLASS (ch, PC_CLASS_MAGE) || (IS_SET (ch->act, ACT_PET))))
    {

      can_enter = TRUE;
    }
  if (IS_SET (to_room->Class_flags, CLASS_CLERIC) && IS_CLASS (ch, PC_CLASS_CLERIC))
    {
      can_enter = TRUE;
    }
  if (IS_SET (to_room->Class_flags, CLASS_THIEF) && IS_CLASS (ch, PC_CLASS_THIEF))
    {
      can_enter = TRUE;
    }
  if (IS_SET (to_room->Class_flags, CLASS_WARRIOR) && IS_CLASS (ch, PC_CLASS_WARRIOR))
    {
      can_enter = TRUE;
    }
  if (IS_SET (to_room->Class_flags, CLASS_RANGER) && IS_CLASS (ch, PC_CLASS_RANGER))
    {
      can_enter = TRUE;
    }
  if (IS_SET (to_room->Class_flags, CLASS_BARD) && IS_CLASS (ch, PC_CLASS_BARD))
    {
      can_enter = TRUE;
    }
  if (IS_SET (to_room->Class_flags, CLASS_PALADIN) && IS_CLASS (ch, PC_CLASS_PALADIN))
    {
      can_enter = TRUE;
    }
  if (IS_SET (to_room->Class_flags, CLASS_ASSASSIN) && IS_CLASS (ch, PC_CLASS_ASSASSIN))
    {
      can_enter = TRUE;
    }
  if (IS_SET (to_room->Class_flags, CLASS_REAVER) && IS_CLASS (ch, PC_CLASS_REAVER))
    {
      can_enter = TRUE;
    }
  if (IS_SET (to_room->Class_flags, CLASS_MONK) && IS_CLASS (ch, PC_CLASS_MONK))
    {
      can_enter = TRUE;
    }
  if (IS_SET (to_room->Class_flags, CLASS_NECROMANCER) && IS_CLASS (ch, PC_CLASS_NECROMANCER))
    {
      can_enter = TRUE;
    }
  if (IS_SET (to_room->Class_flags, CLASS_SWARM) && IS_CLASS (ch, PC_CLASS_SWARM))
     can_enter = TRUE;
  if (IS_SET (to_room->Class_flags, CLASS_GOLEM) && IS_CLASS (ch, PC_CLASS_GOLEM))
     can_enter = TRUE;
  if (IS_SET (to_room->Class_flags, CLASS_CHAOS_JESTER) && IS_CLASS (ch, PC_CLASS_CHAOS_JESTER))
    can_enter = TRUE;
  if (IS_SET (to_room->Class_flags, CLASS_ADVENTURER) && IS_CLASS (ch, PC_CLASS_ADVENTURER))
     can_enter = TRUE;
  if (IS_SET (to_room->Class_flags, CLASS_DRUID) && IS_CLASS (ch, PC_CLASS_DRUID))
     can_enter = TRUE;
  if (!can_enter)
    {
      return FALSE;
    }
  return TRUE;
}

//Iblis - A much simpler way (requiring much less cutting and pasting of code) to check if a player's race
//allows them into the room
bool room_race_check (CHAR_DATA * ch, ROOM_INDEX_DATA * to_room)
{
  bool can_enter=TRUE;
  long flag1 = 0;
  flag1 = to_room->race_flags;
  flag1 &= ~(ROOM_PLAYERSTORE + ROOM_NOTRANSPORT + ROOM_NOTELEPORT);
  if (flag1 == 0)
    return TRUE;
  
      if (!IS_IMMORTAL (ch)
      && !IS_SET(ch->act2,ACT_PUPPET))
    {
	    can_enter = FALSE;
      switch (ch->race)
        {
        case PC_RACE_HUMAN:     /* Human */
          if (IS_SET (to_room->race_flags, RACE_HUMAN))
              can_enter=TRUE;
          break;
        case PC_RACE_ELF:       /* Elf */
          if (IS_SET (to_room->race_flags, RACE_ELF))
              can_enter=TRUE;
          break;
        case PC_RACE_DWARF:     /* Dwarf */
          if (IS_SET (to_room->race_flags, RACE_DWARF))
              can_enter=TRUE;
          break;
        case PC_RACE_CANTHI:    /* Canthi */
          if (IS_SET (to_room->race_flags, RACE_CANTHI))
              can_enter=TRUE;
          break;
	case PC_RACE_VROATH:    /* Vro'ath */
          if (IS_SET (to_room->race_flags, RACE_VROATH))
              can_enter=TRUE;
          break;
        case PC_RACE_SYVIN:     /* Syvin */
          if (IS_SET (to_room->race_flags, RACE_SYVIN))
              can_enter=TRUE;
          break;
        case PC_RACE_SIDHE:     /* Sidhe */
          if (IS_SET (to_room->race_flags, RACE_SIDHE))
              can_enter=TRUE;
          break;
        case PC_RACE_LITAN:             /* Litan */
          if (IS_SET (to_room->race_flags, RACE_LITAN))
              can_enter=TRUE;
          break;
        case PC_RACE_KALIAN:             /* Ja'dor */
          if (IS_SET (to_room->race_flags, RACE_KALIAN))
              can_enter=TRUE;
          break;
	case PC_RACE_LICH:             /* Lich */
          if (IS_SET (to_room->race_flags, RACE_LICH))
              can_enter=TRUE;
	  break;
	case PC_RACE_NERIX:             /* Nerix */
	  if (IS_SET (to_room->race_flags, RACE_NERIX))
	      can_enter=TRUE;
	  break;
	case PC_RACE_AVATAR:            /* Avatar */
	  if (IS_SET (to_room->race_flags, RACE_AVATAR))
    	      can_enter=TRUE;
          break;

        }
    }
      return can_enter;
}

//Iblis - A cheap function that checks areas a player can enter in battle royale
bool br_vnum_check (int vnum)
{
  if (!battle_royale)
    return TRUE;

  else if ((vnum >= 25000 && vnum <= 26999)
	   || (vnum >= 4000 && vnum <= 4599) || (vnum >= 12714
						 && vnum <= 12913)
	   || (vnum >= 5023 && vnum <= 5098) || (vnum >= 12915
						 && vnum <= 13415)
	   || (vnum >= 21052 && vnum <= 21500) || (vnum >= 20951
						   && vnum <= 21051))
    return TRUE;

  else
    return FALSE;
}

//Iblis - Lich defile skill
void do_defile(CHAR_DATA *ch, char *argument)
{
  int chance;
  if (IS_NPC(ch) || ch->in_room == NULL)
    return;
  if ((chance = get_skill (ch, gsn_defile)) < 1)
    {
      send_to_char ("You would defile yourself, but...yea....\n\r", ch);
      return;
    }
  if (ch->in_room->sector_type != SECT_GRASSLAND &&
      ch->in_room->sector_type != SECT_FOREST &&
      ch->in_room->sector_type != SECT_FIELD)
    {
      send_to_char("You are clueless as to how to defile here.\n\r",ch);
      return;
    }

  if (ch->mana < 100)
  {
    send_to_char("You do not have enough mana.\n\r",ch);
    return;
  }
  ch->mana -= 100;

  WAIT_STATE (ch, skill_table[gsn_defile].beats);
  if (chance > number_percent())
    {
      OBJ_DATA *obj;
      act("$n defiles the surrounding area, turning it into a stench filled bog.",ch,NULL,NULL,TO_ROOM);
      send_to_char("You defile the surrounding area, turning it into a stench filled bog.\n\r",ch);
      obj = create_object(get_obj_index(OBJ_VNUM_DEFILE),0);
      obj->timer = 24;
      obj->value[1] = ch->in_room->sector_type;
      obj->value[0] = ch->in_room->vnum;
      ch->in_room->sector_type = SECT_SWAMP;
      obj_to_room(obj,get_room_index(ROOM_VNUM_DV_LIMBO));
      return;
    }
 else
   {
     act("$n attempts to defile the surrounding area, but fails miserably.",ch,NULL,NULL,TO_ROOM);
     send_to_char("You attempt to defile the surrounding area, but fail miserably.\n\r",ch);
     return;
   }
}

//Iblis - Nerix swoop skill
void do_swoop(CHAR_DATA *ch, char* argument)
{
  int chance;
  if (!IS_SET(ch->affected_by,AFF_FLYING))
    {
      send_to_char ("You have to be flying to swoop.\n\r",ch);
      return;
    }
  if ((chance = get_skill(ch,gsn_swoop)) < 1)
    {
      send_to_char ("Swoop there it is!\n\r",ch);
      return;
    }
  else
  {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    char *check;
    int number;
    int count;
    if ((wch = get_char_room (ch, argument)) != NULL)
    {
	    send_to_char("You realize they're in the same room as you and feel sheepish.\n\r",ch);
	    return;
    }
    number = number_argument (argument, arg);
    count = 0;
    for (wch = char_list; wch != NULL; wch = wch->next)
      {
	if (IS_NPC(wch))
	  continue;
	if (is_affected (wch, gsn_mask) && wch->mask != NULL)
	  check = wch->mask;

      else
        check = wch->name;
	if (wch->in_room == NULL || !can_see (ch, wch) || !is_name (arg, check))
	  continue;
	if (++count == number)
	  break;
      }
    if (count == number && ch->in_room && wch->in_room)
    {
      if (wch->in_room->area == ch->in_room->area && IS_OUTSIDE(wch) && IS_OUTSIDE(ch) 
	  && chance > number_percent() && can_move_char(ch,wch->in_room,FALSE,FALSE))
      {
	act("You shoot straight up, flying at incredible speeds towards $N.",ch,NULL,wch,TO_CHAR);
	act("$n shoots straight up, and with a quick puff of air is gone.",ch,NULL,wch,TO_ROOM);
	char_from_room(ch);
	char_to_room(ch,wch->in_room);
	act("You land with a loud crash beside $N.",ch,NULL,wch,TO_CHAR);
	act("A flash of light is seen, and then suddenly $n is standing in the room, as a stiff breeze chills you slightly.",ch,NULL,wch,TO_ROOM);
	WAIT_STATE(ch,skill_table[gsn_swoop].beats);
	return;
      }
      else
	{
	  act("You shoot straight up, but don't know where to find $N, so you quickly land.",ch,NULL,wch,TO_CHAR);
	  act("$n shoots straight up, but looks instantly confused as $e hits the ground with a thud.",ch,NULL,NULL,TO_ROOM);
	  WAIT_STATE(ch,skill_table[gsn_swoop].beats);
	  return;
	}
    }
    else
      {
	send_to_char("There is not a player on by that name at the moment.\n\r",ch);
	return;
      }
  }
}

//Iblis - Nidae locate skill
void do_locate(CHAR_DATA *ch, char* argument)
{
  int chance;
  if ((chance = get_skill(ch,gsn_locate)) < 1)
    {
      send_to_char ("Hum diddly dum.  Nope, you still can't locate sh*t!\n\r",ch);
      return;
    }
  else
    {
      char arg[MAX_INPUT_LENGTH];
      CHAR_DATA *wch;
      char *check;
      int number;
      int count;
      if (argument[0] == '\0')
      {
	      send_to_char("Locate whom?\n\r",ch);
	      return;
      }
      if ((wch = get_char_room (ch, argument)) != NULL)
	{
	  send_to_char("You realize they're in the same room as you and feel sheepish.\n\r",ch);
	  return;
	}
      if (ch->mana < skill_table[gsn_locate].min_mana)
      {
	      send_to_char("You do not have enough energy to locate anything.\n\r",ch);
	      return;
      }
      else ch->mana -= skill_table[gsn_locate].min_mana;
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
	    break;
	}
      if (count == number && ch->in_room && wch->in_room)
	{
	  WAIT_STATE(ch,skill_table[gsn_locate].beats);
	  if ((wch->in_room->sector_type == SECT_WATER_OCEAN 
		  || wch->in_room->sector_type == SECT_UNDERWATER) 
	      && (ch->in_room->sector_type == SECT_WATER_OCEAN
                  || ch->in_room->sector_type == SECT_UNDERWATER)
	      && chance > number_percent() && can_move_char(ch,wch->in_room,FALSE,FALSE))
	    {
	      send_to_char("As you dive through the depts of the murky waters you notice someone at the surface and swim up towards them.\n\r",ch);
	      act("You feel a small rush as $n swims rapidly away.",ch,NULL,NULL,TO_ROOM);
	      char_from_room(ch);
	      char_to_room(ch,wch->in_room);
	      act("The water bulges and a strange creature eyes you suspiciously.",ch,NULL,wch,TO_ROOM);
	      return;
	    }
	  else
	  {
            send_to_char("You dive down and hit your head on the bottom, instantly forgetting what you were diving for.\n\r",ch);
	    act("You watch $n shoot down, then slowly drift back up, swimming around in circles.",ch,NULL,NULL,TO_ROOM);
	    return;
	  }

	}
      send_to_char("You have a feeling they are not worth searching for.\n\r",ch);
    }
}

//Iblis - Nidae find skill
void do_find(CHAR_DATA *ch, char* argument)
{
  int chance,bac=0;
  ROOM_INDEX_DATA *pRoomIndex;
  if ((chance = get_skill(ch,gsn_find_land)) < 1 || (battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner))))
    {
      send_to_char ("You search and search but you can't find land.  Where did you last see it?\n\r",ch);
      return;
    }
  if (ch->move < skill_table[gsn_find_land].min_mana)
    {
      send_to_char("You do not have enough energy to find anything.\n\r",ch);
      return;
    }
  else ch->move -= skill_table[gsn_find_land].min_mana;
  if (chance > number_percent())
    {
      pRoomIndex = get_random_room (ch);
      while (IS_SET (pRoomIndex->room_flags, ROOM_ARENA)
	     || pRoomIndex->area->construct
	     || IS_SET (pRoomIndex->race_flags, ROOM_NOTELEPORT)
	     || pRoomIndex->sector_type != SECT_SHORE)
      {
	if (++bac > 10000)
	  {
	    send_to_char("You can't seem to find any land, no matter how hard you search.\n\r",ch);
	    act("$n swims in a couple circles, then stops, appearing dizzy.",ch,NULL,NULL,TO_ROOM);
	    WAIT_STATE (ch, skill_table[gsn_find_land].beats);
	    return;
	  }
	pRoomIndex = get_random_room (ch);
      }
      WAIT_STATE (ch, skill_table[gsn_find_land].beats);
      send_to_char("You dart through the water and take off towards shore.\n\r",ch);
      act ("$n darts through the water.  $e must be going some place important!", ch, NULL, NULL, TO_ROOM);
      char_from_room (ch);
      char_to_room (ch, pRoomIndex);
      send_to_char("You fly out of the water and land on the shore, creating a giant THUD!\n\r",ch);
      act ("$n flies out of the water at an incredible speed, landing on the ground beside you.", ch, NULL, NULL, TO_ROOM);
    }
  else
    {
      WAIT_STATE (ch, skill_table[gsn_find_land].beats);
      send_to_char("You can't seem to find any land, no matter how hard you search.\n\r",ch);
      act("$n swims in a couple circles, then stops, appearing dizzy.",ch,NULL,NULL,TO_ROOM);
      return;
    }
}
      
//Iblis - Nidae swim skill
void do_swim(CHAR_DATA *ch, char* argument)
{
  if (ch->race != PC_RACE_NIDAE)
  {
    send_to_char ("Huh?\n\r", ch);
    return;
  }
  if (!ch->in_room)
    return;
  if (ch->in_room->sector_type != SECT_UNDERWATER
         && ch->in_room->sector_type != SECT_WATER_OCEAN)
     {
	     send_to_char("There is not enough water to swim here!\n\r",ch);
	     return;
     }
  if (argument[0] != '\0')
    {
      move_fast = TRUE;
      if (!str_cmp (argument, "n") || !str_cmp (argument, "north"))
	{
	  move_char (ch, DIR_NORTH, FALSE);
	  move_fast = FALSE;
	  if (ch->in_room->sector_type == SECT_UNDERWATER
              || ch->in_room->sector_type == SECT_WATER_OCEAN)
	    move_char (ch, DIR_NORTH, FALSE);
	  else 
	  {
	    WAIT_STATE(ch,1);
	    send_to_char("You can't swim anymore once you hit land.\n\r",ch);
	    return;
	  }
	}
      
      
      else if (!str_cmp (argument, "e") || !str_cmp (argument, "east"))
	{
	  move_char (ch, DIR_EAST, FALSE);
	  move_fast = FALSE;
	  if (ch->in_room->sector_type == SECT_UNDERWATER
              || ch->in_room->sector_type == SECT_WATER_OCEAN)
	    move_char (ch, DIR_EAST, FALSE);
	  else 
	  {
	    WAIT_STATE(ch,1);
	    send_to_char("You can't swim anymore once you hit land.\n\r",ch);
	    return;
	  }
	  
	}

      else if (!str_cmp (argument, "s") || !str_cmp (argument, "south"))
	{
	  move_char (ch, DIR_SOUTH, FALSE);
	  move_fast = FALSE;
	  if (ch->in_room->sector_type == SECT_UNDERWATER
              || ch->in_room->sector_type == SECT_WATER_OCEAN)
	    move_char (ch, DIR_SOUTH, FALSE);
	  else 
	  {
	    WAIT_STATE(ch,1);
	    send_to_char("You can't swim anymore once you hit land.\n\r",ch);
	    return;
	  }
	}
      
      else if (!str_cmp (argument, "w") || !str_cmp (argument, "west"))
	{
	  move_char (ch, DIR_WEST, FALSE);
	  move_fast = FALSE;
	  if (ch->in_room->sector_type == SECT_UNDERWATER
              || ch->in_room->sector_type == SECT_WATER_OCEAN)
	    move_char (ch, DIR_WEST, FALSE);
	  else 
	  {
	    WAIT_STATE(ch,1);
	    send_to_char("You can't swim anymore once you hit land.\n\r",ch);
	    return;
	  }
	}

      else if (!str_cmp (argument, "u") || !str_cmp (argument, "up"))
	{
	  move_char (ch, DIR_UP, FALSE);
	  move_fast = FALSE;
	  if (ch->in_room->sector_type == SECT_UNDERWATER
              || ch->in_room->sector_type == SECT_WATER_OCEAN)
	    move_char (ch, DIR_UP, FALSE);
	  else 
	  {
            WAIT_STATE(ch,1);
	    send_to_char("You can't swim anymore once you hit land.\n\r",ch);
	    return;
	  }
	}

      else if (!str_cmp (argument, "d") || !str_cmp (argument, "down"))
	{
	  move_char (ch, DIR_DOWN, FALSE);
	  move_fast = FALSE;
	  if (ch->in_room->sector_type == SECT_UNDERWATER 
	      || ch->in_room->sector_type == SECT_WATER_OCEAN)
	    move_char (ch, DIR_DOWN, FALSE);
	  else 
	  {
  	    WAIT_STATE(ch,1);
	    send_to_char("You can't swim anymore once you hit land.\n\r",ch);
	    return;
	  }
	}
      return;
    }
  else send_to_char("Swim in which direction?\n\r",ch);
}
      
