#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "merc.h"
#include "db.h"
#include "recycle.h"
#include "music.h"
#include "lookup.h"
#include "tables.h"
#include "olc.h"
void do_remfake (CHAR_DATA * ch, char *argument)
{
  FAKE_DATA *fake_ch, *prev = NULL;
  char name[MAX_INPUT_LENGTH];
  one_argument (argument, name);
  if (name[0] == '\0')
    {
      send_to_char ("Syntax: remfake <name>\n\r", ch);
      return;
    }
  for (fake_ch = fake_list; fake_ch; fake_ch = fake_ch->next)
    {
      if (!str_cmp (fake_ch->name, name))
	{
	  if (prev == NULL)
	    fake_list = fake_ch->next;

	  else
	    prev->next = fake_ch->next;
	  free_fake (fake_ch);
	  send_to_char ("Ok.\n\r", ch);
	  return;
	}
      prev = fake_ch;
    }
  send_to_char ("They aren't here (not even faked).\n\r", ch);
}

void do_addfake (CHAR_DATA * ch, char *argument)
{
  FAKE_DATA *fake_ch;
  int level, race, Class;
  char name2[MAX_INPUT_LENGTH], title[MAX_INPUT_LENGTH];
  char name[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  argument = one_argument (argument, name2);
  strcpy (name, capitalize (name2));
  argument = one_argument (argument, buf);
  level = atoi (buf);
  argument = one_argument (argument, buf);
  Class = Class_lookup (buf);
  argument = one_argument (argument, buf);
  race = race_lookup (buf);
  strcpy (title, argument);
  if (name[0] == '\0' || level == 0 || Class == -1 || race <= 0
      || title[0] == '\0')
    {
      send_to_char
	("Syntax: addfake <name> <level> <Class> <race> <title>\n\r", ch);
      return;
    }
  fake_ch = new_fake ();
  fake_ch->name = str_dup (name);
  fake_ch->race = race;
  fake_ch->level = level;
  fake_ch->Class = Class;
  fake_ch->title = str_dup (title);
  fake_ch->descriptor = descriptor_list->descriptor;
  fake_ch->next = fake_list;
  fake_list = fake_ch;
  send_to_char ("Ok.\n\r", ch);
}

CHAR_DATA *create_mobile (MOB_INDEX_DATA * pMobIndex)
{
  CHAR_DATA *mob;
  int i;
  AFFECT_DATA af;
  TRIGGER_DATA *trigger, *prev_trig = NULL, *new_trig = NULL;
  SCRIPT_DATA *scr, *new_scr = NULL, *prev_scr = NULL;
  mobile_count++;
  if (pMobIndex == NULL)
    {
      bug ("Create_mobile: NULL pMobIndex.", 0);
      exit (1);
    }
  mob = new_char ();
  mob->pIndexData = pMobIndex;
  mob->name = str_dup (pMobIndex->player_name);
  mob->id = get_mob_id ();
  mob->short_descr = str_dup (pMobIndex->short_descr);
  mob->long_descr = str_dup (pMobIndex->long_descr);
  mob->description = str_dup (pMobIndex->description);
  mob->spec_fun = pMobIndex->spec_fun;
  mob->prompt = NULL;
  mob->next_in_board = NULL;
  mob->mood = pMobIndex->default_mood;
  mob->variables = NULL;
  mob->hunt_memory = NULL;

  /*    mob->triggers       = pMobIndex->triggers;     */
  if (pMobIndex->triggers == NULL)
    mob->triggers = NULL;

  else
    {
      for (trigger = pMobIndex->triggers; trigger != NULL;
	   trigger = trigger->next)
	{
	  new_trig = malloc (sizeof (TRIGGER_DATA));
//	  new_trig = reinterpret_cast<TRIGGER_DATA *>(malloc (sizeof (TRIGGER_DATA)));
	  if (prev_trig == NULL)
	    mob->triggers = new_trig;

	  else
	    prev_trig->next = new_trig;
	  new_trig->tracer = 0;
	  new_trig->waiting = 0;
	  new_trig->bits = 0;
	  new_trig->trigger_type = trigger->trigger_type;
	  new_trig->keywords = str_dup (trigger->keywords);
	  new_trig->name = str_dup (trigger->name);
	  new_trig->current = NULL;
	  if (trigger->script != NULL)
	    {
	      prev_scr = NULL;
	      for (scr = trigger->script; scr != NULL; scr = scr->next)
		{
		  new_scr = malloc (sizeof (SCRIPT_DATA));
//		  new_scr = reinterpret_cast<SCRIPT_DATA *>(malloc (sizeof (SCRIPT_DATA)));
		  if (prev_scr == NULL)
		    new_trig->script = new_scr;

		  else
		    prev_scr->next = new_scr;
		  if (scr->command == NULL)
		    new_scr->command = NULL;

		  else
		    new_scr->command = str_dup (scr->command);
		  prev_scr = new_scr;
		}
	      new_scr->next = NULL;
	    }
	  prev_trig = new_trig;
	}
      new_trig->next = NULL;
    }
  if (pMobIndex->wealth == 0)
    {
      mob->silver = 0;
      mob->gold = 0;
    }
  else
    {
      long wealth;
      wealth =
	number_range (pMobIndex->wealth / 2, 3 * pMobIndex->wealth / 2);
      mob->gold = number_range (wealth / 200, wealth / 100);
      mob->silver = wealth - (mob->gold * 100);
    }
  mob->group = pMobIndex->group;
  mob->act = pMobIndex->act;
  mob->act2 = pMobIndex->act2;
  mob->blocks_exit = pMobIndex->blocks_exit;
  mob->number_of_attacks = pMobIndex->number_of_attacks;
  mob->comm = COMM_NOCHANNELS;	/* |COMM_NOSHOUT */
  mob->affected_by = pMobIndex->affected_by;
  if (!IS_SET (mob->affected_by, AFF_SWIM)
      && IS_SET (mob->affected_by, AFF_AQUA_BREATHE))
    SET_BIT (mob->affected_by, AFF_SWIM);
  mob->alignment = pMobIndex->alignment;
  mob->level = pMobIndex->level;
  mob->hitroll = pMobIndex->hitroll;
  mob->damroll = pMobIndex->damage[DICE_BONUS];
  mob->max_hit =
    dice (pMobIndex->hit[DICE_NUMBER],
	  pMobIndex->hit[DICE_TYPE]) + pMobIndex->hit[DICE_BONUS];
  mob->hit = mob->max_hit;
  mob->max_mana =
    dice (pMobIndex->mana[DICE_NUMBER],
	  pMobIndex->mana[DICE_TYPE]) + pMobIndex->mana[DICE_BONUS];
  mob->mana = mob->max_mana;
  mob->max_move = pMobIndex->move;
  mob->move = mob->max_move;
  mob->damage[DICE_NUMBER] = pMobIndex->damage[DICE_NUMBER];
  mob->damage[DICE_TYPE] = pMobIndex->damage[DICE_TYPE];
  mob->dam_type = pMobIndex->dam_type;
  if (mob->dam_type == 0)
    switch (number_range (1, 3))
      {
      case (1):
	mob->dam_type = 3;
	break;			/* slash */
      case (2):
	mob->dam_type = 7;
	break;			/* pound */
      case (3):
	mob->dam_type = 11;
	break;			/* pierce */
      }
  for (i = 0; i < 4; i++)
    mob->armor[i] = pMobIndex->ac[i];
  mob->off_flags = pMobIndex->off_flags;
  mob->imm_flags = pMobIndex->imm_flags;
  mob->res_flags = pMobIndex->res_flags;
  mob->vuln_flags = pMobIndex->vuln_flags;
  mob->start_pos = pMobIndex->start_pos;
  mob->default_pos = pMobIndex->default_pos;
  mob->sex = pMobIndex->sex;
  if (mob->sex == 3)		/* random sex */
    mob->sex = number_range (1, 2);
  mob->race = pMobIndex->race;
  mob->form = pMobIndex->form;
  mob->parts = pMobIndex->parts;
  mob->size = pMobIndex->size;
  if (pMobIndex->material != NULL)
    mob->material = str_dup (pMobIndex->material);

  else
    mob->material = str_dup ("");
  //Iblis 8/29/04 - New, setable stats
  for (i = 0; i < MAX_STATS; i++)
    mob->perm_stat[i] = pMobIndex->perm_stat[i] + (number_range(0,4)-2);
  //Iblis 8/29/04 - Setable stats make below not needed
/*  if (IS_SET (mob->act, ACT_WARRIOR))
    {
      mob->perm_stat[STAT_STR] += 3;
      mob->perm_stat[STAT_INT] -= 1;
      mob->perm_stat[STAT_CON] += 2;
    }
  if (IS_SET (mob->act, ACT_THIEF))
    {
      mob->perm_stat[STAT_DEX] += 3;
      mob->perm_stat[STAT_INT] += 1;
      mob->perm_stat[STAT_WIS] -= 1;
    }
  if (IS_SET (mob->act, ACT_CLERIC))
    {
      mob->perm_stat[STAT_WIS] += 3;
      mob->perm_stat[STAT_DEX] -= 1;
      mob->perm_stat[STAT_STR] += 1;
    }
  if (IS_SET (mob->act, ACT_MAGE))
    {
      mob->perm_stat[STAT_INT] += 3;
      mob->perm_stat[STAT_STR] -= 1;
      mob->perm_stat[STAT_DEX] += 1;
    }
  if (IS_SET (mob->act, ACT_BARD))
    {
      mob->perm_stat[STAT_DEX] += 2;
      mob->perm_stat[STAT_STR] -= 1;
      mob->perm_stat[STAT_CHA] += 3;
    }
  if (IS_SET (mob->act, ACT_RANGER))
    {
      mob->perm_stat[STAT_STR] += 2;
      mob->perm_stat[STAT_INT] += 1;
      mob->perm_stat[STAT_CON] += 1;
    }*/
  if (IS_SET (mob->off_flags, OFF_FAST))
    mob->perm_stat[STAT_DEX] += 2;
  /*mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
  mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) / 2;*/
  mob->position = mob->start_pos;
  mob->recruit_flags = mob->pIndexData->recruit_flags;
  mob->recruit_value[0] = mob->pIndexData->recruit_value[0];
  mob->recruit_value[1] = mob->pIndexData->recruit_value[1];
  mob->recruit_value[2] = mob->pIndexData->recruit_value[2];
  if (mob->level >= 70)
    SET_BIT (mob->imm_flags, IMM_CHARM);
  mob->next = char_list;
  char_list = mob;
  pMobIndex->count++;
  if (IS_SET (mob->affected_by, AFF_PLAGUE))
    {
      af.where = TO_AFFECTS;
      af.type = gsn_plague;
      af.level = mob->level;
      af.duration = -1;
      af.location = APPLY_STR;
      af.modifier = -5;
      af.bitvector = 0;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (mob, &af);
    }
  if (IS_SET (mob->affected_by, AFF_SWIM))
    {
      af.where = TO_AFFECTS;
      af.type = gsn_swim;
      af.level = mob->level;
      af.duration = -1;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = 0;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (mob, &af);
    }
  if (IS_AFFECTED (mob, AFF_POISON))
    {
      af.where = TO_AFFECTS;
      af.type = gsn_poison;
      af.level = mob->level;
      af.duration = -1;
      af.location = APPLY_STR;
      af.modifier = -2;
      af.bitvector = 0;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (mob, &af);
    }
  if (IS_AFFECTED (mob, AFF_SANCTUARY))
    {
      af.where = TO_AFFECTS;
      af.type = gsn_sanctuary;
      af.level = mob->level;
      af.duration = -1;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = 0;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (mob, &af);
    }
  if (IS_AFFECTED (mob, AFF_HASTE))
    {
      af.where = TO_AFFECTS;
      af.type = skill_lookup ("haste");
      af.level = mob->level;
      af.duration = -1;
      af.location = APPLY_DEX;
      af.modifier = 1 + (mob->level >= 18) + (mob->level >= 25) +
	(mob->level >= 32);
      af.bitvector = 0;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (mob, &af);
    }
  if (IS_AFFECTED (mob, AFF_PROTECT_EVIL))
    {
      af.where = TO_AFFECTS;
      af.type = skill_lookup ("protection evil");
      af.level = mob->level;
      af.duration = -1;
      af.location = APPLY_SAVES;
      af.modifier = -1;
      af.bitvector = 0;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (mob, &af);
    }
  if (IS_AFFECTED (mob, AFF_PROTECT_GOOD))
    {
      af.where = TO_AFFECTS;
      af.type = skill_lookup ("protection good");
      af.level = mob->level;
      af.duration = -1;
      af.location = APPLY_SAVES;
      af.modifier = -1;
      af.bitvector = 0;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (mob, &af);
    }
  return mob;
}
