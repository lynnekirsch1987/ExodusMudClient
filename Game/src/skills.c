#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include <ctype.h>
#include <sys/stat.h>
#include "magic.h"
#include "heros.h"
#include "obj_trig.h"


/* command procedures needed */
DECLARE_DO_FUN (do_help);
DECLARE_DO_FUN (do_say);

long int level_cost (int level);


bool has_skills (CHAR_DATA * ch, int cmdskill_type)
{
	int skill_count;

	// if it's a Paladin and they lost their prayers then no-skill
	if (IS_CLASS (ch, PC_CLASS_PALADIN) &&
		(ch->pcdata->lost_prayers) && (cmdskill_type == SKILL_PRAY))
	{
		return (FALSE);
	}

	for (skill_count = 0; command_table[skill_count].type != 0; skill_count++)
	{
		if (command_table[skill_count].type == cmdskill_type)
		{
			if (level_for_command (ch, skill_count) <= ch->level)
			{
				return (TRUE);
			}
			else
			{
				return (FALSE);
			}
		}
	}
	return (FALSE);
}


/* used to get new skills */
void do_gain (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *trainer;
	//int gn = 0, sn = 0;
	long int levelcost = 0;

	if (IS_NPC (ch))
		return;

	/* find a trainer */
	for (trainer = ch->in_room->people;
		trainer != NULL; trainer = trainer->next_in_room)
		if (IS_NPC (trainer) && IS_SET (trainer->act, ACT_GAIN))
			break;

	if (trainer == NULL || !can_see (ch, trainer))
	{
		send_to_char ("You can't do that here.\n\r", ch);
		return;
	}

	one_argument (argument, arg);

	if (arg[0] == '\0')
	{
		do_say (trainer, "Pardon me?");
		return;
	}
	//IBLIS 5/18/03 - Added new leveling command for the new xp system
	//
	if (!str_prefix (arg, "level"))
	{
		if (ch->level >= 90)
		{
			send_to_char ("Alas, you can level no more.\n\r", ch);
			return;
		}
		if (ch->exp > (levelcost = level_cost (ch->level)))
		{
			ch->exp = ch->exp - levelcost;
			advance_level (ch);
			save_char_obj (ch);
			return;
		}
		else
		{
			sprintf (buf,
				"You do not have the %li experience required for the next level\n\r",
				levelcost);
			send_to_char (buf, ch);
		}
		return;
	}
	//IBLIS 5/18/03 - Added new gain hp, gain mana, and gain move commands
	if (!str_prefix (arg, "hp"))
	{
		if (ch->exp >= 1000000)
		{
			if (ch->race == PC_RACE_KALIAN)
			{
				if (ch->pcdata->hp_gained >= 100
					|| (ch->pcdata->mana_gained + ch->pcdata->move_gained +
					ch->pcdata->hp_gained) >= 150)
				{
					send_to_char ("You've gained all the hp possible.\n\r", ch);
					return;
				}
			}
			else if (ch->pcdata->hp_gained >= 300
				|| (ch->pcdata->mana_gained + ch->pcdata->move_gained +
				ch->pcdata->hp_gained) >= 400)
			{
				send_to_char ("You've gained all the hp possible.\n\r", ch);
				return;
			}
			ch->exp = ch->exp - 1000000;
			ch->pcdata->perm_hit += 1;
			ch->max_hit += 1;
			ch->hit += 1;
			ch->pcdata->hp_gained += 1;
			act ("Your durability increases!", ch, NULL, NULL, TO_CHAR);
			act ("$n's durability increases!", ch, NULL, NULL, TO_ROOM);
		}
		else
			send_to_char ("It costs 1000000 experience to gain hp.\n\r", ch);
		return;
	}

	if (!str_prefix (arg, "move"))
	{
		if (ch->exp >= 1000000)
		{
			if (ch->race == PC_RACE_KALIAN)
			{
				if (ch->pcdata->move_gained >= 100
					|| (ch->pcdata->mana_gained + ch->pcdata->move_gained +
					ch->pcdata->hp_gained) >= 150)
				{
					send_to_char ("You've gained all the move possible.\n\r", ch);
					return;
				}
			}
			else if (ch->pcdata->move_gained >= 300
				|| (ch->pcdata->mana_gained + ch->pcdata->move_gained +
				ch->pcdata->hp_gained) >= 400)
			{
				send_to_char ("You've gained all the move possible.\n\r", ch);
				return;
			}
			ch->exp = ch->exp - 1000000;
			ch->pcdata->perm_move += 1;
			ch->max_move += 1;
			ch->move += 1;
			ch->pcdata->move_gained += 1;
			act ("Your mobility increases!", ch, NULL, NULL, TO_CHAR);
			act ("$n's mobility increases!", ch, NULL, NULL, TO_ROOM);
		}
		else
			send_to_char ("It costs 1000000 experience to gain move.\n\r", ch);
		return;
	}

	if (!str_prefix (arg, "mana"))
	{
		if (ch->exp >= 1000000)
		{
			if (ch->race == PC_RACE_KALIAN)
			{
				if (ch->pcdata->mana_gained >= 100
					|| (ch->pcdata->mana_gained + ch->pcdata->move_gained +
					ch->pcdata->hp_gained) >= 150)
				{
					send_to_char ("You've gained all the mana possible.\n\r", ch);
					return;
				}
			}
			else if (ch->pcdata->mana_gained >= 300
				|| (ch->pcdata->mana_gained + ch->pcdata->move_gained +
				ch->pcdata->hp_gained) >= 400)
			{
				send_to_char ("You've gained all the mana possible.\n\r", ch);
				return;
			}
			ch->exp = ch->exp - 1000000;
			ch->pcdata->perm_mana += 1;
			ch->max_mana += 1;
			ch->mana += 1;
			ch->pcdata->mana_gained += 1;
			act ("Your power increases!", ch, NULL, NULL, TO_CHAR);
			act ("$n's power increases!", ch, NULL, NULL, TO_ROOM);
		}
		else
			send_to_char ("It costs 1000000 experience to gain mana.\n\r", ch);
		return;
	}


	act ("$N tells you 'I do not understand...'", ch, NULL, trainer, TO_CHAR);
}



void do_spells (CHAR_DATA * ch, char *argument)
{
	char spell_list[MAX_LEVEL][MAX_STRING_LENGTH];
	char spell_columns[MAX_LEVEL];
	int sn, lev, mana;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC (ch))
		return;


	if (ch->level > LEVEL_HERO)
	{
		for (lev = 0; lev < MAX_LEVEL; lev++)
		{
			spell_columns[lev] = 0;
			spell_list[lev][0] = '\0';
		}

		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].name == NULL)
				break;

			if (skill_table[sn].spell_fun != spell_null &&
				get_skill(ch,sn) > 0 && skill_table[sn].type == SKILL_CAST)
			{
				found = TRUE;
				lev = level_for_skill (ch, sn);
				if (ch->level < lev)
					sprintf (buf, "%-18s  n/a      ", skill_table[sn].name);
				else
				{
					if (sn != gsn_mirror)
					{
						if (skill_table[sn].min_mana == 0)
						{
							sprintf(buf,"%-18s   `eSurprise!``",skill_table[sn].name);
						}
						else 
						{
							mana = UMAX (skill_table[sn].min_mana,
								100 / (2 + ch->level - lev));
							sprintf (buf, "%-18s  %3d mana  ",
								skill_table[sn].name, mana);
						}
					}
					else
					{
						if (ch->mana < 100)
							sprintf (buf, "%-18s  n/a      ",
							skill_table[sn].name);
						else
						{
							mana = ch->mana / 2;
							sprintf (buf, "%-18s - %3d - %3d mana  ",
								skill_table[sn].name, get_skill(ch,sn), mana);
						}
					}
				}

				if (spell_list[lev][0] == '\0')
					sprintf (spell_list[lev], "\n\rLevel %2d: %s", lev, buf);
				else
					/* append */
				{
					//if (++spell_columns[lev] % 2 == 0)
					strcat (spell_list[lev], "\n\r          ");
					strcat (spell_list[lev], buf);
				}
			}
		}
	}
	else
	{
		for (lev = 0; lev < LEVEL_HERO; lev++)
		{
			spell_columns[lev] = 0;
			spell_list[lev][0] = '\0';
		}

		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].name == NULL)
				break;

			if (level_for_skill(ch,sn) < LEVEL_HERO &&
				skill_table[sn].spell_fun != spell_null
				&& skill_table[sn].type == SKILL_CAST
				&& (ch->pcdata->learned[sn] > 0 || ch->pcdata->mod_learned[sn] > 0))
			{
				found = TRUE;
				lev = level_for_skill (ch, sn);
				if (ch->level < lev)
					sprintf (buf, "%-18s  n/a      ", skill_table[sn].name);
				else
				{
					if (sn != gsn_mirror)
					{
						if (skill_table[sn].min_mana == 0)
						{
							sprintf(buf,"%-18s  `eSurprise!``",skill_table[sn].name);
						}
						else
						{
							mana = UMAX (skill_table[sn].min_mana,
								100 / (2 + ch->level - lev));
							sprintf (buf, "%-18s  %3d mana  ",
								skill_table[sn].name, mana);
						}
					}
					else
					{
						if (ch->mana < 100)
							sprintf (buf, "%-18s  n/a      ",
							skill_table[sn].name);
						else
						{
							mana = ch->mana / 2;
							sprintf (buf, "%-18s  %3d mana  ",
								skill_table[sn].name, mana);
						}
					}
				}

				if (spell_list[lev][0] == '\0')
					sprintf (spell_list[lev], "\n\rLevel %2d: %s", lev, buf);
				else
					/* append */
				{
					if (++spell_columns[lev] % 2 == 0)
						strcat (spell_list[lev], "\n\r          ");
					strcat (spell_list[lev], buf);
				}
			}
		}

	}
	if (!found)
	{
		send_to_char ("You know no spells.\n\r", ch);
		return;
	}

	if (ch->level > LEVEL_HERO)
	{
		for (lev = 0; lev < MAX_LEVEL; lev++)
			if (spell_list[lev][0] != '\0')
				send_to_char (spell_list[lev], ch);
	}
	else
	{
		for (lev = 0; lev < LEVEL_HERO; lev++)
		{


			if (lev == 1)
				if (ch->Class == 0 || ch->Class == 4)
				{
					send_to_char ("\r\n`a:`b---------------------`a:``\r\n", ch);
					sprintf (buf, "`o%11s Spells\r\n",
						capitalize (Class_table[ch->Class].name));
					send_to_char (buf, ch);
					send_to_char ("`a:`b---------------------`a:``\r\n", ch);
				}
				if (lev == 31 && ch->level > 30 && ch->Class != ch->Class2)
					if (ch->Class != ch->Class2
						&& (ch->Class2 == 0 || ch->Class2 == 4))
					{
						send_to_char ("\r\n`a:`b---------------------`a:``\r\n", ch);
						sprintf (buf, "`o%11s Spells\r\n",
							capitalize (Class_table[ch->Class2].name));
						send_to_char (buf, ch);
						send_to_char ("`a:`b---------------------`a:``\r\n", ch);
					}
					if (lev == 61 && ch->level > 60 && ch->Class2 != ch->Class3)
						if (ch->Class2 != ch->Class3
							&& (ch->Class3 == 0 || ch->Class3 == 4)
							&& !(ch->Class2 != 0 && ch->Class2 != 4
							&& ch->Class == ch->Class3))
						{
							send_to_char ("\r\n`a:`b---------------------`a:``\r\n", ch);
							sprintf (buf, "`o%11s Spells\r\n",
								capitalize (Class_table[ch->Class3].name));
							send_to_char (buf, ch);
							send_to_char ("`a:`b---------------------`a:``\r\n", ch);
						}

						if (spell_list[lev][0] != '\0')
							send_to_char (spell_list[lev], ch);
		}
	}
	send_to_char ("\n\r", ch);
}

void do_skills (CHAR_DATA * ch, char *argument)
{
	char skill_list[MAX_LEVEL][MAX_STRING_LENGTH];
	char skill_columns[MAX_LEVEL];
	int sn, lev;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC (ch))
		return;

	if (ch->level > LEVEL_HERO)
	{
		for (lev = 0; lev < MAX_LEVEL; lev++)
		{
			skill_columns[lev] = 0;
			skill_list[lev][0] = '\0';
		}

		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].name == NULL)
				break;

			if (get_skill(ch,sn) > 0 &&
				skill_table[sn].spell_fun == spell_null)
			{
				found = TRUE;
				lev = level_for_skill (ch, sn);
				if (ch->level < lev)
					sprintf (buf, "%-18s n/a      ", skill_table[sn].name);
				else
					sprintf (buf, "%-18s %3d%%      ", skill_table[sn].name,
					get_skill(ch,sn));

				if (skill_list[lev][0] == '\0')
					sprintf (skill_list[lev], "\n\rLevel %2d: %s", lev, buf);
				else
					/* append */
				{
					if (++skill_columns[lev] % 2 == 0)
						strcat (skill_list[lev], "\n\r          ");
					strcat (skill_list[lev], buf);
				}
			}
		}
	}
	else
	{
		for (lev = 0; lev < LEVEL_HERO; lev++)
		{
			skill_columns[lev] = 0;
			skill_list[lev][0] = '\0';
		}

		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].name == NULL)
				break;


			if (level_for_skill(ch,sn) < LEVEL_HERO &&
				skill_table[sn].spell_fun == spell_null
				&& (ch->pcdata->learned[sn] > 0 || ch->pcdata->mod_learned[sn] > 0))
			{
				found = TRUE;
				lev = level_for_skill (ch, sn);
				if (ch->level < lev)
					sprintf (buf, "%-18s n/a      ", skill_table[sn].name);
				else
					sprintf (buf, "%-18s %3d%%      ", skill_table[sn].name,
					get_skill(ch,sn));

				if (skill_list[lev][0] == '\0')
					sprintf (skill_list[lev], "\n\rLevel %2d: %s", lev, buf);
				else
					/* append */
				{
					if (++skill_columns[lev] % 2 == 0)
						strcat (skill_list[lev], "\n\r          ");
					strcat (skill_list[lev], buf);
				}
			}
		}
	}


	if (!found)
	{
		send_to_char ("You know no skills.\n\r", ch);
		return;

	}

	if (ch->level > LEVEL_HERO)
	{
		for (lev = 0; lev < MAX_LEVEL; lev++)
			if (skill_list[lev][0] != '\0')
				send_to_char (skill_list[lev], ch);
	}
	else
		for (lev = 0; lev < LEVEL_HERO; lev++)
		{
			if (lev == 1)
			{
				send_to_char ("\r\n`a:`b---------------------`a:``\r\n", ch);
				sprintf (buf, "`o%10s Skills\r\n",
					capitalize (Class_table[ch->Class].name));
				send_to_char (buf, ch);
				send_to_char ("`a:`b---------------------`a:``\r\n", ch);
			}
			if (lev == 31 && ch->level > 30 && ch->Class != ch->Class2)
			{
				send_to_char ("\r\n`a:`b---------------------`a:``\r\n", ch);
				sprintf (buf, "`o%10s Skills\r\n",
					capitalize (Class_table[ch->Class2].name));
				send_to_char (buf, ch);
				send_to_char ("`a:`b---------------------`a:``\r\n", ch);
			}
			if (lev == 61 && ch->level > 60 && ch->Class2 != ch->Class3)
			{
				send_to_char ("\r\n`a:`b---------------------`a:``\r\n", ch);
				sprintf (buf, "`o%10s Skills\r\n",
					capitalize (Class_table[ch->Class3].name));
				send_to_char (buf, ch);
				send_to_char ("`a:`b---------------------`a:``\r\n", ch);
			}

			if (skill_list[lev][0] != '\0')
				send_to_char (skill_list[lev], ch);
		}

		send_to_char ("\n\r", ch);
}



/* checks for skill improvement */
void check_improve (CHAR_DATA * ch, int sn, bool success, int multiplier)
{
	int chance, chandiv;
	char buf[100];
	bool canimprove = 0;

	if (IS_NPC (ch))
		return;

	if (ch->level < level_for_skill (ch, sn))
		if (ch->level < 91)
			return;			/* skill is not known */

	if (ch->pcdata->learned[sn] == 0 || ch->pcdata->learned[sn] == 100)
		return;
	//IBLIS 5/18/03 - Cheap way to only improve automatic (IE not spammable skills)
	//Tedious because case won't work with the way gsn_blah is defined.

	if (sn == gsn_axe)
		canimprove = 1;
	if (sn == gsn_dagger)
		canimprove = 1;
	if (sn == gsn_flail)
		canimprove = 1;
	if (sn == gsn_mace)
		canimprove = 1;
	if (sn == gsn_polearm)
		canimprove = 1;
	if (sn == gsn_shield_block)
		canimprove = 1;
	if (sn == gsn_staff)
		canimprove = 1;
	if (sn == gsn_dice)
		canimprove = 1;
	if (sn == gsn_sword)
		canimprove = 1;
	if (sn == gsn_dual_wield)
		canimprove = 1;
	if (sn == gsn_dodge)
		canimprove = 1;
	if (sn == gsn_fisticuffery)
		canimprove = 1;
	if (sn == gsn_enhanced_damage)
		canimprove = 1;
	if (sn == gsn_critical_strike)
		canimprove = 1;
	if (sn == gsn_swim)
		canimprove = 1;
	if (sn == gsn_hand_to_hand)
		canimprove = 1;
	if (sn == gsn_parry)
		canimprove = 1;
	if (sn == gsn_leadership)
		canimprove = 1;
	if (sn == gsn_endurance)
		canimprove = 1;
	if (sn == gsn_conceal)
		canimprove = 1;
	if (sn == gsn_second_attack)
		canimprove = 1;
	if (sn == gsn_third_attack)
		canimprove = 1;
	if (sn == gsn_blindfighting)
		canimprove = 1;
	if (sn == gsn_fast_healing)
		canimprove = 1;
	if (sn == gsn_meditation)
		canimprove = 1;
	if (sn == gsn_split_vision)
		canimprove = 1;
	if (sn == gsn_sense_life)
		canimprove = 1;
	if (sn == gsn_dirty_tactics)
		canimprove = 1;
	if (sn == gsn_counter)
		canimprove = 1;
	if (sn == gsn_block)
		canimprove = 1;
	if (sn == gsn_aquatic_sense)
		canimprove = 1;
	if (sn == gsn_woodworking || sn == gsn_fletchery)
		canimprove = 1;
	if (sn == gsn_lumberjacking 
		&& ((is_Class_tiers(ch,PC_CLASS_RANGER,3) && get_skill(ch,gsn_lumberjacking) < 50)
		|| (is_Class_tiers(ch,PC_CLASS_RANGER,2) && get_skill(ch,gsn_lumberjacking) < 30)
		|| (is_Class_tiers(ch,PC_CLASS_RANGER,1) && get_skill(ch,gsn_lumberjacking) < 20)
		|| (get_skill(ch,gsn_lumberjacking) < 10)))
		canimprove = 1;
	if (sn == gsn_nature_sense)
		canimprove = 1;
	if (sn == gsn_shillelagh)
		canimprove = 1;
	if (!canimprove)
	{
		if (success)
		{
			sprintf(buf, "%d", sn);
			trip_triggers_arg(ch, OBJ_TRIG_CHAR_USES_SKILL, NULL, NULL, OT_SPEC_NONE, buf);
		}
		return;
	}


	/* check to see if the character has a chance to learn */
	chance = 10 * int_app[get_curr_stat (ch, STAT_INT)].learn;


	chandiv = (multiplier * rating_for_skill (ch, sn) * 4);
	chandiv = UMAX (chandiv, 3);

	chance /= chandiv;
	chance += ch->level;

	if (number_range (1, 1000) > chance)
		return;

	/* now that the character has a CHANCE to learn, see if they really have */

	if (success)
	{
		chance = URANGE (5, 100 - ch->pcdata->learned[sn], 95);
		if (number_percent () < chance)
		{
			sprintf (buf, "You have become better at %s!\n\r",
				skill_table[sn].name);
			send_to_char (buf, ch);
			ch->pcdata->learned[sn]++;
			gain_exp (ch, 100 * rating_for_skill (ch, sn) * ch->level);
		}
	}
}

//A simple function that improves a skill 1%.  Used mainly for trade skills, which check if the skill
//should be allowed to increase a specific way 
void improve_skill (CHAR_DATA * ch, int sn)
{
	char buf[100];

	if (IS_NPC (ch))
		return;

	if (ch->level < level_for_skill (ch, sn))
		if (ch->level < 91)
			return;                   /* skill is not known */

	if (ch->pcdata->learned[sn] == 0 || ch->pcdata->learned[sn] == 100)
		return;

	sprintf (buf, "You have become better at %s!\n\r",
		skill_table[sn].name);
	send_to_char (buf, ch);
	ch->pcdata->learned[sn]++;
	gain_exp (ch, 100 * rating_for_skill (ch, sn) * ch->level);

}

/* returns a group index number given the name */
int group_lookup (const char *name)
{
	int gn;

	for (gn = 0; gn < MAX_GROUP; gn++)
	{
		if (group_table[gn].name == NULL)
			break;
		if (LOWER (name[0]) == LOWER (group_table[gn].name[0])
			&& !str_prefix (name, group_table[gn].name))
			return gn;
	}

	return -1;
}

/* recursively adds a group given its number -- uses group_add */
void gn_add (CHAR_DATA * ch, int gn)
{
	int i;

	ch->pcdata->group_known[gn] = TRUE;
	for (i = 0; i < MAX_IN_GROUP; i++)
	{
		if (group_table[gn].spells[i] == NULL)
			break;
		group_add (ch, group_table[gn].spells[i], FALSE);
	}
}

/* use for processing a skill or group for addition  */
void group_add_all (CHAR_DATA * ch)
{
	int gn,ti;
	if (IS_NPC (ch))
		return;
	for (gn = 0; gn < MAX_SKILL; gn++)
	{
		//Temp hack to fix skills that you have an aptitude in via eq
		//from being gained permanently
		ti = ch->pcdata->mod_learned[gn];
		ch->pcdata->mod_learned[gn] = 0;
		if (skill_table[gn].name != NULL && level_for_skill (ch, gn) < 91)
			group_add (ch, skill_table[gn].name, FALSE);
		ch->pcdata->mod_learned[gn] = ti;
		if (ch->pcdata->mod_learned[gn] == -999)
			ch->pcdata->mod_learned[gn] = 0;
	}
	return;
}

void group_add (CHAR_DATA * ch, const char *name, bool deduct)
{
	int sn, gn;

	if (IS_NPC (ch))		/* NPCs do not have skills */
		return;
	sn = exact_skill_lookup (name);
	//IBLIS 5/18/03 - To give mask to only those who deserve it
	if (sn == gsn_mask && ch->race != PC_RACE_SIDHE && (!IS_CLASS (ch, PC_CLASS_ASSASSIN)))
		return;
	if (sn == gsn_swim && ch->race == PC_RACE_DWARF)
		return;

	if (sn != -1)
	{
		if (ch->pcdata->learned[sn] <= 0)
		{			/* i.e. not known */
			ch->pcdata->learned[sn] = 1;
		}
		return;
	}

	/* now check groups */

	gn = group_lookup (name);

	if (gn != -1)
	{
		if (ch->pcdata->group_known[gn] == FALSE)
		{
			ch->pcdata->group_known[gn] = TRUE;
		}
		gn_add (ch, gn);		/* make sure all skills in the group are known */
	}
}

void do_mask (CHAR_DATA * ch, char *argument)
{
	int chance;
	AFFECT_DATA af;
	char mask_to[MAX_INPUT_LENGTH];


	if ((chance = get_skill (ch, gsn_mask)) == 0 || IS_NPC (ch))
	{
		send_to_char ("You know nothing of the ways of subterfuge.\n\r", ch);
		return;
	}

	if (is_affected (ch, gsn_mask))
	{
		send_to_char
			("Your disguise is discarded as you reclaim your true identity.\n\r",
			ch);
		affect_strip (ch, gsn_mask);
		return;
	}

	if (ch->mask != NULL)
	{
		free_string (ch->mask);
		ch->mask = NULL;
	}

	one_argument (argument, mask_to);

	if (mask_to[0] == '\0')
	{
		send_to_char ("Who would you like to mask yourself as?\n\r", ch);
		return;
	}


	if (strlen (mask_to) > 13)
	{
		send_to_char ("That name is too long to use as a mask.\n\r", ch);
		return;
	}

	if (strlen (mask_to) < 3)
	{
		send_to_char ("That name is too short to use as a mask.\n\r", ch);
		return;
	}

	{
		CHAR_DATA *lch;
		char strsave[MAX_STRING_LENGTH];
		struct stat statBuf;


		sprintf (strsave, "%s%c/%s", PLAYER_DIR, toupper (mask_to[0]),
			capitalize (mask_to));

		if (!check_parse_name (mask_to) || stat (strsave, &statBuf) != -1)
		{
			send_to_char
				("Sorry, that name cannot be used for your mask.\n\r", ch);
			return;
		}

		for (lch = char_list; lch; lch = lch->next)
		{
			if (lch->name != NULL)
				if (!str_cmp (lch->name, mask_to))
				{
					send_to_char
						("Sorry, that name cannot be used for your mask.\n\r", ch);
					return;
				}

				if (lch->mask != NULL)
					if (!str_cmp (lch->mask, mask_to))
					{
						send_to_char
							("Sorry, that name cannot be used for your mask.\n\r", ch);
						return;
					}
		}
	}

	if (ch->mana >= ch->level * 3)
		ch->mana -= ch->level * 3;
	else
	{
		send_to_char ("You do not have enough mana to mask your appearance."
			"\n\r", ch);
		return;
	}

	if ((number_percent () < chance) && (ch->race == PC_RACE_SIDHE))
	{
		af.where = TO_AFFECTS;
		af.type = gsn_mask;
		af.level = ch->level;
		af.duration = ch->level / 6;
		af.modifier = 0;
		af.location = APPLY_NONE;
		af.bitvector = 0;
		af.permaff = FALSE;
		af.composition = FALSE;
		af.comp_name = str_dup ("");
		affect_to_char (ch, &af);

		send_to_char
			("You slip briefly into the realm of the ethereal and emerge a new person.\n\r",
			ch);
		act
			("$n fades and slides quickly out of phase, returning under a different guise.",
			ch, NULL, NULL, TO_ROOM);
		ch->mask = str_dup (capitalize (mask_to));
		return;
	}

	if ((number_percent () < chance) && (ch->race != PC_RACE_SIDHE))
	{
		af.where = TO_AFFECTS;
		af.type = gsn_mask;
		af.level = ch->level;
		af.duration = ch->level / 6;
		af.modifier = 0;
		af.location = APPLY_NONE;
		af.bitvector = 0;
		af.permaff = FALSE;
		af.composition = FALSE;
		af.comp_name = str_dup ("");
		affect_to_char (ch, &af);

		send_to_char
			("You step off to the side and quickly don a new disguise.\n\r", ch);
		act ("$n turns away for a moment and returns with a different face.",
			ch, NULL, NULL, TO_ROOM);
		ch->mask = str_dup (capitalize (mask_to));
		check_improve (ch, gsn_mask, TRUE, 2);
		return;
	}

	if ((number_percent () >= chance) && (ch->race == PC_RACE_SIDHE))
	{
		send_to_char
			("You slip briefly into the realm of the ethereal and emerge unchanged.\n\r",
			ch);
		act ("$n fades and slides quickly out of phase and emerges unchanged.",
			ch, NULL, NULL, TO_ROOM);
		check_improve (ch, gsn_mask, FALSE, 2);
		return;
	}

	if ((number_percent () >= chance) && (ch->race != PC_RACE_SIDHE))
	{
		send_to_char
			("You try to disguise yourself, but fail miserably.\n\r", ch);
		act
			("$n tries to put on a disguise, but just ends up returning with a silly moustache.",
			ch, NULL, NULL, TO_ROOM);
		return;
	}
}

//IBLIS 5/18/03 - Returns the level cost needed for a player at (level) to reach the next level
// base on Minax's original new experience suggestion (www.devinbrown.com/experience.txt)
long int level_cost (int level)
{
	switch (level)
	{
	case 1:
		return 500;
	case 2:
		return 1000;
	case 3:
		return 1521;
	case 4:
		return 2282;
	case 5:
		return 3424;
	case 6:
		return 5137;
	case 7:
		return 7706;
	case 8:
		return 11560;
	case 9:
		return 17340;
	case 10:
		return 26011;
	case 11:
		return 39017;
	case 12:
		return 58526;
	case 13:
		return 83895;
	case 14:
		return 131686;
	case 15:
		return 197530;
	case 16:
		return 296296;
	case 17:
		return 444444;
	case 18:
		return 666666;
	case 19:
		return 1000000;
	case 20:
		return 1500000;
	case 21:
		return 2100000;
	case 22:
		return 2200000;
	case 23:
		return 2300000;
	case 24:
		return 2400000;
	case 25:
		return 2500000;
	case 26:
		return 2600000;
	case 27:
		return 2700000;
	case 28:
		return 2800000;
	case 29:
		return 2900000;
	case 30:
		return 3000000;
	case 31:
		return 3100000;
	case 32:
		return 3200000;
	case 33:
		return 3300000;
	case 34:
		return 3400000;
	case 35:
		return 3500000;
	case 36:
		return 3600000;
	case 37:
		return 3700000;
	case 38:
		return 3800000;
	case 39:
		return 3900000;
	case 40:
		return 8000000;
	case 41:
		return 8200000;
	case 42:
		return 8400000;
	case 43:
		return 8600000;
	case 44:
		return 8800000;
	case 45:
		return 9000000;
	case 46:
		return 9200000;
	case 47:
		return 9400000;
	case 48:
		return 9600000;
	case 49:
		return 9800000;
	case 50:
		return 10000000;
	case 51:
		return 10200000;
	case 52:
		return 10400000;
	case 53:
		return 10600000;
	case 54:
		return 10800000;
	case 55:
		return 11000000;
	case 56:
		return 11200000;
	case 57:
		return 11400000;
	case 58:
		return 11600000;
	case 59:
		return 11800000;
	case 60:
		return 12000000;
	case 61:
		return 12200000;
	case 62:
		return 12400000;
	case 63:
		return 12600000;
	case 64:
		return 12800000;
	case 65:
		return 13000000;
	case 66:
		return 13500000;
	case 67:
		return 14000000;
	case 68:
		return 14500000;
	case 69:
		return 15000000;
	case 70:
		return 15500000;
	case 71:
		return 16000000;
	case 72:
		return 16500000;
	case 73:
		return 17000000;
	case 74:
		return 18000000;
	case 75:
		return 20000000;
	case 76:
		return 25000000;
	case 77:
		return 27500000;
	case 78:
		return 30000000;
	case 79:
		return 35000000;
	case 80:
		return 40000000;
	case 81:
		return 40500000;
	case 82:
		return 41000000;
	case 83:
		return 41500000;
	case 84:
		return 42000000;
	case 85:
		return 85000000;
	case 86:
		return 86000000;
	case 87:
		return 87000000;
	case 88:
		return 88000000;
	case 89:
		return 89000000;
	case 90:
		return 90000000;
		//case default: return 0;
	}
	return 100000000;
}
