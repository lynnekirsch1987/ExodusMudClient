#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"
#include "heros.h"
#include "obj_trig.h"

/* command procedures needed */
DECLARE_DO_FUN (do_rstat);
DECLARE_DO_FUN (do_mstat);
DECLARE_DO_FUN (do_ostat);
DECLARE_DO_FUN (do_rset);
DECLARE_DO_FUN (do_mset);
DECLARE_DO_FUN (do_oset);
DECLARE_DO_FUN (do_sset);
DECLARE_DO_FUN (do_mfind);
DECLARE_DO_FUN (do_ofind);
DECLARE_DO_FUN (do_slookup);
DECLARE_DO_FUN (do_mload);
DECLARE_DO_FUN (do_oload);
DECLARE_DO_FUN (do_force);
DECLARE_DO_FUN (do_quit);
DECLARE_DO_FUN (do_save);
DECLARE_DO_FUN (do_look);
DECLARE_DO_FUN (do_stand);
extern bool ch_is_quitting;
extern bool isorder;
extern bool imm_advance;
extern bool crit_strike_possible;
void decrease_level args ((CHAR_DATA * ch));
long int low_gain_exp args ((CHAR_DATA * ch, long int gain));
void do_noshout args ((CHAR_DATA * ch, char *argument));
void do_setclan args ((CHAR_DATA * ch, char *argument));
void shutdown_graceful args ((void));
void br_transfer args ((CHAR_DATA * ch));
bool br_vnum_check args ((int vnum));
bool load_char_obj_new args((DESCRIPTOR_DATA * d, char *name, char* filename));
DESCRIPTOR_DATA *new_descriptor args ((void));
extern struct clan_type clan_table[MAX_CLAN];
extern AREA_DATA *area_first;
extern bool slowns;
extern bool battle_royale;
extern char imm_who_msg[];
int half_timer = 0;
int quad_timer = 0;
int double_timer = 0;
int battle_royale_timer = 0;
short br_death_counter = 0;
short battle_royale_dead_amount = 5;
bool deathrestore = FALSE;
bool nogate = FALSE;
bool is_linkloading = FALSE;
bool player_rewarding = FALSE;
long gold_toll;
long damage_toll;
char *get_bs_name (int bs);
char *get_bb_name (int burst);
long get_next_cost (OBJ_DATA * sword, int value);
long get_slot_cost (OBJ_DATA * sword, int value);
long get_burst_cost (OBJ_DATA * sword, int value);
void remove_aggressor args ((CHAR_DATA * ch, CHAR_DATA * victim));
void free_ot args((OBJ_TRIG *ot));
OBJ_TRIG *new_ot args((int vnum));
extern CHAR_DATA *GLOBAL_ot_ch;
extern OBJ_DATA *GLOBAL_ot_obj;
extern OBJ_TRIG *giant_ot_list;
void affect_strip_skills args((CHAR_DATA * ch));
bool show_help args((CHAR_DATA * ch, char *argument));


int adjust_damage args((int dam_to_do));
bool ignore_hitroll = FALSE;//Iblis 3/27/04 - To test hitroll affects
bool ignore_ac = FALSE;//Iblis 3/27/04 - To test ac affects
bool ignore_save = FALSE;
bool new_ac = TRUE;

//IBLIS 6/20/03 - need for Battle Royale Update on char quit
extern CHAR_DATA *br_leader;
extern CHAR_DATA *last_br_kill;

void unsetbit args((char *explored, int index));
int getbit args((char *explored, int index));
void setbit args((char *explored, int index));


/*
nnn * Local functions.
*/
void do_swordstat args ((CHAR_DATA * ch, CHAR_DATA * victim));
void do_affectstat args((CHAR_DATA * imm, CHAR_DATA *ch));

void ppurge args((CHAR_DATA *ch, CHAR_DATA* victim));


void do_prayerstat (CHAR_DATA * ch, CHAR_DATA * victim)
{
	char prayer_list[MAX_LEVEL][MAX_STRING_LENGTH];
	char prayer_columns[MAX_LEVEL];
	int sn, lev;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	if (IS_NPC (victim))
	{
		send_to_char ("Mobiles don't have prayers.\n\r", ch);
		return;
	}
	if (victim->level > LEVEL_HERO)
	{
		for (lev = 0; lev < MAX_LEVEL; lev++)
		{
			prayer_columns[lev] = 0;
			prayer_list[lev][0] = '\0';
		}
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].name == NULL)
				break;
			if (get_skill(victim,sn) > 1 && skill_table[sn].spell_fun != spell_null
				&& skill_table[sn].type == SKILL_PRAY)
			{
				found = TRUE;
				lev = level_for_skill (victim, sn);
				if (victim->level < lev)
					sprintf (buf, "%-18s  n/a      ", skill_table[sn].name);

				else
					sprintf (buf, "%-18s  %3d%%   ", skill_table[sn].name,
					get_skill(victim,sn));
				if (prayer_list[lev][0] == '\0')
					sprintf (prayer_list[lev], "\n\rLevel %2d: %s", lev, buf);

				else
				{
					if (++prayer_columns[lev] % 2 == 0)
						strcat (prayer_list[lev], "\n\r          ");
					strcat (prayer_list[lev], buf);
				}
			}
		}
	}
	else
	{
		for (lev = 0; lev < LEVEL_HERO; lev++)
		{
			prayer_columns[lev] = 0;
			prayer_list[lev][0] = '\0';
		}
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].name == NULL)
				break;
			if (level_for_skill(victim,sn) < LEVEL_HERO && skill_table[sn].spell_fun != spell_null
				&& skill_table[sn].type == SKILL_PRAY
				&& (victim->pcdata->learned[sn] > 0 || victim->pcdata->mod_learned[sn] > 0))
			{
				found = TRUE;
				lev = level_for_skill (victim, sn);
				if (victim->level < lev)
					sprintf (buf, "%-18s  n/a      ", skill_table[sn].name);

				else
					sprintf (buf, "%-18s  %3d%%  ", skill_table[sn].name,
					get_skill(victim,sn));
				if (prayer_list[lev][0] == '\0')
					sprintf (prayer_list[lev], "\n\rLevel %2d: %s", lev, buf);

				else
				{
					if (++prayer_columns[lev] % 2 == 0)
						strcat (prayer_list[lev], "\n\r          ");
					strcat (prayer_list[lev], buf);
				}
			}
		}
	}
	if (!found)
	{
		send_to_char ("This character knows no prayers.\n\r", ch);
		return;
	}
	if (victim->level > LEVEL_HERO)
	{
		for (lev = 0; lev < MAX_LEVEL; lev++)
			if (prayer_list[lev][0] != '\0')
				send_to_char (prayer_list[lev], ch);
	}
	else
	{
		for (lev = 0; lev < LEVEL_HERO; lev++)
			if (prayer_list[lev][0] != '\0')
				send_to_char (prayer_list[lev], ch);
	}
	send_to_char ("\n\r", ch);
}

void do_chantstat (CHAR_DATA * ch, CHAR_DATA * victim)
{
	char prayer_list[MAX_LEVEL][MAX_STRING_LENGTH];
	char prayer_columns[MAX_LEVEL];
	int sn, lev;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	if (IS_NPC (victim))
	{
		send_to_char ("Mobiles don't have chants.\n\r", ch);
		return;
	}
	if (victim->level > LEVEL_HERO)
	{
		for (lev = 0; lev < MAX_LEVEL; lev++)
		{
			prayer_columns[lev] = 0;
			prayer_list[lev][0] = '\0';
		}
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].name == NULL)
				break;
			if (get_skill(victim,sn) && skill_table[sn].spell_fun != spell_null
				&& skill_table[sn].type == SKILL_CHANT)
			{
				found = TRUE;
				lev = level_for_skill (victim, sn);
				if (victim->level < lev)
					sprintf (buf, "%-18s  n/a      ", skill_table[sn].name);

				else
					sprintf (buf, "%-18s  %3d%%   ", skill_table[sn].name,
					get_skill(victim,sn));
				if (prayer_list[lev][0] == '\0')
					sprintf (prayer_list[lev], "\n\rLevel %2d: %s", lev, buf);

				else
					/* append */
				{
					if (++prayer_columns[lev] % 2 == 0)
						strcat (prayer_list[lev], "\n\r          ");
					strcat (prayer_list[lev], buf);
				}
			}
		}
	}
	else
	{
		for (lev = 0; lev < LEVEL_HERO; lev++)
		{
			prayer_columns[lev] = 0;
			prayer_list[lev][0] = '\0';
		}
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].name == NULL)
				break;
			if (level_for_skill(victim,sn) < LEVEL_HERO && skill_table[sn].spell_fun != spell_null
				&& skill_table[sn].type == SKILL_CHANT
				&& (victim->pcdata->learned[sn] > 0 || victim->pcdata->mod_learned[sn] > 0))
			{
				found = TRUE;
				lev = level_for_skill (victim, sn);
				if (victim->level < lev)
					sprintf (buf, "%-18s  n/a      ", skill_table[sn].name);

				else
					sprintf (buf, "%-18s  %3d%%  ", skill_table[sn].name,
					get_skill(victim,sn));
				if (prayer_list[lev][0] == '\0')
					sprintf (prayer_list[lev], "\n\rLevel %2d: %s", lev, buf);

				else
					/* append */
				{
					if (++prayer_columns[lev] % 2 == 0)
						strcat (prayer_list[lev], "\n\r          ");
					strcat (prayer_list[lev], buf);
				}
			}
		}
	}
	if (!found)
	{
		send_to_char ("This character knows no chants.\n\r", ch);
		return;
	}
	if (victim->level > LEVEL_HERO)
	{
		for (lev = 0; lev < MAX_LEVEL; lev++)
			if (prayer_list[lev][0] != '\0')
				send_to_char (prayer_list[lev], ch);
	}
	else
	{
		for (lev = 0; lev < LEVEL_HERO; lev++)
			if (prayer_list[lev][0] != '\0')
				send_to_char (prayer_list[lev], ch);
	}
	send_to_char ("\n\r", ch);
}

void do_songstat (CHAR_DATA * ch, CHAR_DATA * victim)
{
	char prayer_list[MAX_LEVEL][MAX_STRING_LENGTH];
	char prayer_columns[MAX_LEVEL];
	int sn, lev;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	if (IS_NPC (victim))
	{
		send_to_char ("Mobiles don't have songs.\n\r", ch);
		return;
	}
	if (victim->level > LEVEL_HERO)
	{
		for (lev = 0; lev < MAX_LEVEL; lev++)
		{
			prayer_columns[lev] = 0;
			prayer_list[lev][0] = '\0';
		}
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].name == NULL)
				break;
			if (get_skill(victim,sn) > 1 && skill_table[sn].spell_fun != spell_null
				&& skill_table[sn].type == SKILL_SING)
			{
				found = TRUE;
				lev = level_for_skill (victim, sn);
				if (victim->level < lev)
					sprintf (buf, "%-18s  n/a      ", skill_table[sn].name);

				else
					sprintf (buf, "%-18s  %3d%%   ", skill_table[sn].name,
					get_skill(victim,sn));
				if (prayer_list[lev][0] == '\0')
					sprintf (prayer_list[lev], "\n\rLevel %2d: %s", lev, buf);

				else
					/* append */
				{
					if (++prayer_columns[lev] % 2 == 0)
						strcat (prayer_list[lev], "\n\r          ");
					strcat (prayer_list[lev], buf);
				}
			}
		}
	}
	else
	{
		for (lev = 0; lev < LEVEL_HERO; lev++)
		{
			prayer_columns[lev] = 0;
			prayer_list[lev][0] = '\0';
		}
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].name == NULL)
				break;
			if (level_for_skill(victim,sn) < LEVEL_HERO && skill_table[sn].spell_fun != spell_null
				&& skill_table[sn].type == SKILL_SING
				&& (victim->pcdata->learned[sn] > 0 || victim->pcdata->mod_learned[sn] > 0))
			{
				found = TRUE;
				lev = level_for_skill (victim, sn);
				if (victim->level < lev)
					sprintf (buf, "%-18s  n/a      ", skill_table[sn].name);

				else
					sprintf (buf, "%-18s  %3d%%  ", skill_table[sn].name,
					get_skill(victim,sn));
				if (prayer_list[lev][0] == '\0')
					sprintf (prayer_list[lev], "\n\rLevel %2d: %s", lev, buf);

				else
					/* append */
				{
					if (++prayer_columns[lev] % 2 == 0)
						strcat (prayer_list[lev], "\n\r          ");
					strcat (prayer_list[lev], buf);
				}
			}
		}
	}
	if (!found)
	{
		send_to_char ("This character knows no prayers.\n\r", ch);
		return;
	}
	if (victim->level > LEVEL_HERO)
	{
		for (lev = 0; lev < MAX_LEVEL; lev++)
			if (prayer_list[lev][0] != '\0')
				send_to_char (prayer_list[lev], ch);
	}
	else
	{
		for (lev = 0; lev < LEVEL_HERO; lev++)
			if (prayer_list[lev][0] != '\0')
				send_to_char (prayer_list[lev], ch);
	}
	send_to_char ("\n\r", ch);
}

void do_equipstat (CHAR_DATA * ch, CHAR_DATA * victim)
{
	return;
}

void do_invenstat (CHAR_DATA * ch, CHAR_DATA * victim)
{
	return;
}

void do_spellstat (CHAR_DATA * ch, CHAR_DATA * victim)
{
	char spell_list[MAX_LEVEL][MAX_STRING_LENGTH];
	char spell_columns[MAX_LEVEL];
	int sn, lev;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	if (IS_NPC (victim))
	{
		send_to_char ("Mobiles don't have spells.\n\r", ch);
		return;
	}
	if (victim->level > LEVEL_HERO)
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
			if (get_skill(victim,sn) > 1 && skill_table[sn].spell_fun != spell_null
				&& skill_table[sn].type == SKILL_CAST)
			{
				found = TRUE;
				lev = level_for_skill (victim, sn);
				if (victim->level < lev)
					sprintf (buf, "%-18s  n/a      ", skill_table[sn].name);

				else
					sprintf (buf, "%-18s  %3d%%   ", skill_table[sn].name,
					get_skill(victim,sn));
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
			if (level_for_skill(victim,sn) < LEVEL_HERO && skill_table[sn].spell_fun != spell_null
				&& skill_table[sn].type == SKILL_CAST
				&& (victim->pcdata->learned[sn] > 0 || victim->pcdata->mod_learned[sn] > 0))
			{
				found = TRUE;
				lev = level_for_skill (victim, sn);
				if (victim->level < lev)
					sprintf (buf, "%-18s  n/a      ", skill_table[sn].name);

				else
					sprintf (buf, "%-18s  %3d%%  ", skill_table[sn].name,
					get_skill(victim,sn));
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
		send_to_char ("This character knows no spells.\n\r", ch);
		return;
	}
	if (victim->level > LEVEL_HERO)
	{
		for (lev = 0; lev < MAX_LEVEL; lev++)
			if (spell_list[lev][0] != '\0')
				send_to_char (spell_list[lev], ch);
	}
	else
	{
		for (lev = 0; lev < LEVEL_HERO; lev++)
			if (spell_list[lev][0] != '\0')
				send_to_char (spell_list[lev], ch);
	}
	send_to_char ("\n\r", ch);
}

void do_skillstat (CHAR_DATA * ch, CHAR_DATA * victim)
{
	char skill_list[MAX_LEVEL][MAX_STRING_LENGTH];
	char skill_columns[MAX_LEVEL];
	int sn, lev;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	if (IS_NPC (victim))
	{
		send_to_char ("Mobiles don't have skills.\n\r", ch);
		return;
	}
	if (victim->level > LEVEL_HERO)
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
			if (get_skill(victim,sn) > 1 && skill_table[sn].spell_fun == spell_null)
			{
				found = TRUE;
				lev = level_for_skill (victim, sn);
				if (victim->level < lev)
					sprintf (buf, "%-18s n/a      ", skill_table[sn].name);

				else
					sprintf (buf, "%-18s %3d%%      ",
					skill_table[sn].name, get_skill(victim,sn));
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
			if (level_for_skill(victim,sn) < LEVEL_HERO && skill_table[sn].spell_fun == spell_null
				&& (victim->pcdata->learned[sn] > 0 || victim->pcdata->mod_learned[sn] > 0))
			{
				found = TRUE;
				lev = level_for_skill (victim, sn);
				if (victim->level < lev)
					sprintf (buf, "%-18s n/a      ", skill_table[sn].name);

				else
					sprintf (buf, "%-18s %3d%%      ",
					skill_table[sn].name, get_skill(victim,sn));
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
		send_to_char ("This character know no skills.\n\r", ch);
		return;
	}
	if (victim->level > LEVEL_HERO)
	{
		for (lev = 0; lev < MAX_LEVEL; lev++)
			if (skill_list[lev][0] != '\0')
				send_to_char (skill_list[lev], ch);
	}
	else
		for (lev = 0; lev < LEVEL_HERO; lev++)
			if (skill_list[lev][0] != '\0')
				send_to_char (skill_list[lev], ch);
	send_to_char ("\n\r", ch);
}

void do_zapchain (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Reset whose reply chain ?\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (get_trust (victim) >= get_trust (ch))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	victim->reply = NULL;
	sprintf (buf, "%s's reply chain reset.\n\r", victim->name);
	send_to_char (buf, ch);
	return;
}


/* shows all groups, or the sub-members of a group */
void do_groupstat (CHAR_DATA * ch, CHAR_DATA * victim)
{
	char buf[100];
	int gn, /* sn, */ col;
	if (IS_NPC (victim))
	{
		send_to_char ("Mobiles don't have groups.\n\r", ch);
		return;
	}
	col = 0;
	for (gn = 0; gn < MAX_GROUP; gn++)
	{
		if (group_table[gn].name == NULL)
			break;
		if (victim->pcdata->group_known[gn])
		{
			sprintf (buf, "%-20s ", group_table[gn].name);
			send_to_char (buf, ch);
			if (++col % 3 == 0)
				send_to_char ("\n\r", ch);
		}
	}
	if (col % 3 != 0)
		send_to_char ("\n\r", ch);
	sprintf (buf, "Creation points: %d\n\r", victim->pcdata->points);
	send_to_char (buf, ch);
	return;
}

void do_zonepurge (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *lch, *lch_next;
	OBJ_DATA *obj, *obj_next;
	if (ch->level < 92)
	{
		send_to_char ("Huh?\n\r", ch);
		return;
	}
	if (ch->level < 99 && str_cmp (ch->name, ch->in_room->area->creator)
		&& str_cmp (ch->name, ch->in_room->area->helper))
	{
		send_to_char ("Sorry, you can't do that here.\n\r", ch);
		return;
	}
	for (lch = char_list; lch != NULL; lch = lch_next)
	{
		lch_next = lch->next;
		if (!IS_NPC (lch) || !lch->in_room)
			continue;
		if (lch->in_room->area == ch->in_room->area)
		{
			extract_char (lch, TRUE);
		}
	}
	for (obj = object_list; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next;
		if (obj->in_room == NULL)
			continue;
		if (obj->in_room->area == ch->in_room->area)
			extract_obj (obj);
	}
	send_to_char ("Ok.\n\r", ch);
}

void do_remhelp (CHAR_DATA * ch, char *argument)
{
	if (argument[0] == '\0')
	{
		send_to_char ("What help topic would you like to remove?\n\r", ch);
		return;
	}
	remove_help (argument, "online.hlp");
	send_to_char ("Ok.\n\r", ch);
}

/* Super-AT command:

FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>


Executes action several times, either on ALL players (not including yourself),
MORTALS (including trusted characters), GODS (characters with level higher than
L_HERO), MOBS (Not recommended) or every room (not recommended either!)

If you insert a # in the action, it will be replaced by the name of the target.

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room containg
at least one target, but only once per room. # cannot be used with FOR EVERY-
WHERE. # can be anywhere in the action.

Example: 

FOR ALL SMILE -> you will only smile once in a room with 2 players.
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

Destroying the characters this command acts upon MAY cause it to fail. Try to
avoid something like FOR MOBS PURGE (although it actually works at my MUD).

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
though :)

The command works by transporting the character to each of the rooms with 
target in them. Private rooms are not violated.

*/

/* Expand the name of a character into a string that identifies THAT
character within a room. E.g. the second 'guard' -> 2. guard
*/
const char *name_expand (CHAR_DATA * ch)
{
	int count = 1;
	CHAR_DATA *rch;
	char name[MAX_INPUT_LENGTH];	/*  HOPEFULLY no mob has a name longer than THAT */

	static char outbuf[MAX_INPUT_LENGTH];

	if (!IS_NPC (ch))
		return ch->name;

	one_argument (ch->name, name);	/* copy the first word into name */

	if (!name[0])			/* weird mob .. no keywords */
	{
		strcpy (outbuf, "");	/* Do not return NULL, just an empty buffer */
		return outbuf;
	}

	for (rch = ch->in_room->people; rch && (rch != ch); rch = rch->next_in_room)
		if (is_name (name, rch->name))
			count++;


	sprintf (outbuf, "%d.%s", count, name);
	return outbuf;
}

extern ROOM_INDEX_DATA *room_index_hash[];

void do_for (CHAR_DATA * ch, char *argument)
{
	char range[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	bool fGods = FALSE, fMortals = FALSE, fMobs = FALSE, fEverywhere =
		FALSE, found;
	ROOM_INDEX_DATA *room, *old_room;
	CHAR_DATA *p, *p_next;
	int i;

	argument = one_argument (argument, range);

	if (!range[0] || !argument[0])	/* invalid usage? */
	{
		do_help (ch, "cmd_for");
		return;
	}

	if (!str_prefix ("quit", argument))
	{
		send_to_char ("Are you trying to crash the MUD or something?\n\r", ch);
		return;
	}


	if (!str_cmp (range, "all"))
	{
		fMortals = TRUE;
		fGods = TRUE;
	}
	else if (!str_cmp (range, "gods"))
		fGods = TRUE;
	else if (!str_cmp (range, "mortals"))
		fMortals = TRUE;
	else if (!str_cmp (range, "mobs"))
		fMobs = TRUE;
	else if (!str_cmp (range, "everywhere"))
		fEverywhere = TRUE;
	else
		do_help (ch, "for");	/* show syntax */

	/* do not allow # to make it easier */
	if (fEverywhere && strchr (argument, '#'))
	{
		send_to_char ("Cannot use FOR EVERYWHERE with the # thingie.\n\r", ch);
		return;
	}

	if (strchr (argument, '#'))	/* replace # ? */
	{
		for (p = char_list; p; p = p_next)
		{
			p_next = p->next;	/* In case someone DOES try to AT MOBS SLAY # */
			found = FALSE;

			if (!(p->in_room) || room_is_private (p->in_room) || (p == ch))
				continue;

			if (IS_NPC (p) && fMobs)
				found = TRUE;
			else if (!IS_NPC (p) && p->level >= LEVEL_IMMORTAL && fGods)
				found = TRUE;
			else if (!IS_NPC (p) && p->level < LEVEL_IMMORTAL && fMortals)
				found = TRUE;

			/* It looks ugly to me.. but it works :) */
			if (found)		/* p is 'appropriate' */
			{
				char *pSource = argument;	/* head of buffer to be parsed */
				char *pDest = buf;	/* parse into this */

				while (*pSource)
				{
					if (*pSource == '#')	/* Replace # with name of target */
					{
						const char *namebuf = name_expand (p);

						if (namebuf)	/* in case there is no mob name ?? */
							while (*namebuf)	/* copy name over */
								*(pDest++) = *(namebuf++);

						pSource++;
					}
					else
						*(pDest++) = *(pSource++);
				}		/* while */
				*pDest = '\0';	/* Terminate */

				/* Execute */
				old_room = ch->in_room;
				char_from_room (ch);
				char_to_room (ch, p->in_room);
				interpret (ch, buf);
				char_from_room (ch);
				char_to_room (ch, old_room);

			}			/* if found */
		}			/* for every char */
	}
	else				/* just for every room with the appropriate people in it */
	{
		for (i = 0; i < MAX_KEY_HASH; i++)	/* run through all the buckets */
			for (room = room_index_hash[i]; room; room = room->next)
			{
				found = FALSE;

				/* Anyone in here at all? */
				if (fEverywhere)	/* Everywhere executes always */
					found = TRUE;
				else if (!room->people)	/* Skip it if room is empty */
					continue;


				/* Check if there is anyone here of the requried type */
				/* Stop as soon as a match is found or there are no more ppl in room */
				for (p = room->people; p && !found; p = p->next_in_room)
				{

					if (p == ch)	/* do not execute on oneself */
						continue;

					if (IS_NPC (p) && fMobs)
						found = TRUE;
					else if (!IS_NPC (p) && (p->level >= LEVEL_IMMORTAL) && fGods)
						found = TRUE;
					else if (!IS_NPC (p) && (p->level <= LEVEL_IMMORTAL)
						&& fMortals)
						found = TRUE;
				}			/* for everyone inside the room */

				if (found && !room_is_private (room))	/* Any of the required type here AND room not private? */
				{
					/* This may be ineffective. Consider moving character out of old_room
					once at beginning of command then moving back at the end.
					This however, is more safe?
					*/

					old_room = ch->in_room;
					char_from_room (ch);
					char_to_room (ch, room);
					interpret (ch, argument);
					char_from_room (ch);
					char_to_room (ch, old_room);
				}			/* if found */
			}			/* for every room in a bucket */
	}				/* if strchr */
}				/* do_for */


//Iblis 11/05/03 - Finally completed this ccmmand.  It appears to work
void do_deathrestore (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	DESCRIPTOR_DATA* d, *d_next;
	char name[MAX_STRING_LENGTH];
	if (argument[0] != '\0')
	{
		send_to_char("Tell Iblis to ACTUALLY fix this function this time.\n\r",ch);
		return;
		if ((victim = get_char_world (ch, argument)) != NULL)
		{
			if (!IS_NPC (victim))
			{
				for (d = descriptor_list; d != NULL; d = d_next)
				{
					if (d->character == victim)
						break;
				}
				if (d == NULL)
				{
					send_to_char("ERROR not found.\n\r",ch);
					return;
				}
				strcpy(name,victim->name);
				ch_is_quitting = TRUE;
				extract_char (victim, TRUE);
				ch_is_quitting = FALSE;
				load_char_obj_new(d,name,PFILE_BACKUP_DEATH);
				if (d->character->pcdata->condition[COND_HUNGER] < 0)
					d->character->pcdata->condition[COND_HUNGER] = 0;
				if (d->character->pcdata->condition[COND_THIRST] < 0)
					d->character->pcdata->condition[COND_THIRST] = 0;
				if (IS_AFFECTED (d->character, AFF_POISON))
				{
					REMOVE_BIT (d->character->affected_by, AFF_POISON);
					affect_strip (d->character, gsn_poison);
				}
				if (IS_AFFECTED (d->character, AFF_PLAGUE))
				{
					REMOVE_BIT (d->character->affected_by, AFF_PLAGUE);
					affect_strip (d->character, gsn_plague);
				}
				if (IS_AFFECTED (d->character, AFF_BLIND))
				{
					REMOVE_BIT (d->character->affected_by, AFF_BLIND);
					affect_strip (d->character, gsn_blindness);
				}
				d->character->hit = d->character->max_hit;
				d->character->mana = d->character->max_mana;
				d->character->move = d->character->max_move;
				d->character->position = POS_STANDING;


				// clannies that have declared their clan hall get
				// to be resurected in their clan hall
				if (get_clan_hall_ch (d->character) != CLAN_BOGUS)
				{
					ROOM_INDEX_DATA *rid;
					rid = get_room_index (get_clan_hall_ch (d->character));
					if (rid != NULL)
						char_to_room (d->character, rid);
					else char_to_room (d->character, get_room_index (ROOM_VNUM_ALTAR));
				}
				else
					char_to_room (d->character, get_room_index (ROOM_VNUM_ALTAR));

				send_to_char ("An imm has taken pity on you and death restored you.\n\r",d->character);
				send_to_char ("Player restored.\n\r", ch);
				return;
			}
			else
			{
				send_to_char
					("That character is a mob and cannot be death restored.\n\r",
					ch);
				return;
			}
		}
		else
		{
			send_to_char ("That character is NOT online.\n\r", ch);
			return;
		}
	}
	else
	{
		send_to_char ("Syntax: drestore <player>\n\r", ch);
		return;
	}
}

void do_edithelp (CHAR_DATA * ch, char *argument)
{
	if (argument[0] == '\0')
	{
		send_to_char ("What help topic would you like to edit?\n\r", ch);
		return;
	}
	if (ch->desc->help_name != NULL)
		free_string (ch->desc->help_name);
	ch->desc->help_name = str_dup (argument);
	if (ch->desc->help_info != NULL)
		free_string (ch->desc->help_info);
	ch->desc->help_info = read_current_help (argument, "online.hlp");
	ch->desc->editor = SAVE_ANY_HELP;
	edit_string (ch, &ch->desc->help_info);
}

void do_slowns (CHAR_DATA * ch, char *argument)
{
	slowns = !slowns;
	if (slowns)
		send_to_char ("Name service lookup turned off.\n\r", ch);

	else
		send_to_char ("Name service lookup turned on.\n\r", ch);
	return;
}

void do_checksafe (CHAR_DATA * ch, char *argument)
{
	char big_buf[MAX_STRING_LENGTH * 15];
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *location;
	bool notfound;
	int vnum;
	big_buf[0] = '\0';
	for (pArea = area_first; pArea; pArea = pArea->next)
	{
		if (!str_cmp (pArea->name, "Haven")
			|| !str_cmp (pArea->filename, "ocean.are"))
			continue;
		vnum = pArea->min_vnum;
		notfound = FALSE;
		while ((location = get_room_index (vnum)) == NULL)
		{
			if (vnum == pArea->max_vnum)
			{
				notfound = TRUE;
				break;
			}
			if (notfound)
				continue;
			vnum++;
		}
		if (location == NULL)
			continue;
		for (; location; location = location->next)
			if (IS_SET (location->room_flags, ROOM_SAFE))
			{
				sprintf (big_buf + strlen (big_buf), "[%5d]  %s\t(%s)\n\r",
					location->vnum, location->name, location->area->name);
			}
	}
	page_to_char (big_buf, ch);
}

void do_checkbuild (CHAR_DATA * ch, char *argument)
{
	char big_buf[MAX_STRING_LENGTH * 15];
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *location;
	bool notfound;
	int vnum;
	big_buf[0] = '\0';
	for (pArea = area_first; pArea; pArea = pArea->next)
	{
		if (!str_cmp (pArea->filename, "ocean.are"))
			continue;
		vnum = pArea->min_vnum;
		notfound = FALSE;
		while ((location = get_room_index (vnum)) == NULL)
		{
			if (vnum == pArea->max_vnum)
			{
				notfound = TRUE;
				break;
			}
			if (notfound)
				continue;
			vnum++;
		}
		if (location == NULL)
			continue;
		for (; location; location = location->next)
			if (IS_SET (location->room_flags, ROOM_BUILD))
			{
				sprintf (big_buf + strlen (big_buf), "[%5d]  %s\t(%s)\n\r",
					location->vnum, location->name, location->area->name);
			}
	}
	page_to_char (big_buf, ch);
}

void do_checkcity (CHAR_DATA * ch, char *argument)
{
	char big_buf[MAX_STRING_LENGTH * 15];
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *location;
	bool notfound;
	int vnum;
	big_buf[0] = '\0';
	for (pArea = area_first; pArea; pArea = pArea->next)
	{
		if (!str_cmp (pArea->filename, "ocean.are"))
			continue;
		vnum = pArea->min_vnum;
		notfound = FALSE;
		while ((location = get_room_index (vnum)) == NULL)
		{
			if (vnum == pArea->max_vnum)
			{
				notfound = TRUE;
				break;
			}
			if (notfound)
				continue;
			vnum++;
		}
		if (location == NULL)
			continue;
		for (; location; location = location->next)
			if (location->sector_type == SECT_CITY)
			{
				sprintf (big_buf + strlen (big_buf), "[%5d]  %s\t(%s)\n\r",
					location->vnum, location->name, location->area->name);
			}
	}
	page_to_char (big_buf, ch);
}

ROOM_INDEX_DATA *find_location args ((CHAR_DATA * ch, char *arg));
void do_fakehost (CHAR_DATA * ch, char *argument)
{
	char tcbuf[MAX_STRING_LENGTH];
	if (IS_NPC (ch))
		return;
	if (argument[0] == '\0')
	{
		send_to_char ("Syntax:\n\r", ch);
		send_to_char ("fakehost [hostname to fake]\n\r", ch);
		send_to_char ("fakehost off\n\r", ch);
		return;
	}
	if (!str_cmp (argument, "off"))
	{
		send_to_char ("Fakehosting turned off.\n\r", ch);
		free_string (ch->desc->host);
		ch->desc->host = str_dup (ch->desc->realhost);
		return;
	}
	sprintf (tcbuf, "Faking your hostname as '%s'.\n\r", argument);
	send_to_char (tcbuf, ch);
	free_string (ch->desc->host);
	ch->desc->host = str_dup (argument);
	return;
}

void do_permaff (CHAR_DATA * ch, char *argument)
{
	extern int top_affect;
	OBJ_DATA *obj;
	int affby, level;
	AFFECT_DATA *paf;
	char level_string[MAX_INPUT_LENGTH], item_string[MAX_INPUT_LENGTH];
	char affect_string[MAX_INPUT_LENGTH];
	argument = one_argument (argument, item_string);
	argument = one_argument (argument, affect_string);
	argument = one_argument (argument, level_string);
	if (item_string[0] == '\0' || affect_string[0] == '\0'
		|| level_string[0] == '\0')
	{
		send_to_char ("Syntax: permaff <object> <affect> <level>\n\r", ch);
		send_to_char ("Affect may be one of:\n\r", ch);
		show_help(ch, "? affect");
		/*
		send_to_char
		("      haste          invisible          detect_evil          detect_invis\n\r",
		ch);
		send_to_char
		("      detect_magic   detect_hidden      detect_good          faerie_fire\n\r",
		ch);
		send_to_char
		("      sanctuary      infrared           curse                poison\n\r",
		ch);
		send_to_char
		("      sneak          hide               aqua_breathe         flying\n\r",
		ch);
		send_to_char
		("      pass_door      plague             weaken               slow\n\r",
		ch);*/
		return;
	}
	if ((obj = get_obj_here (ch, item_string)) == NULL)
	{
		send_to_char ("You do not see that here.\n\r", ch);
		return;
	}
	level = atol (level_string);
	if ((affby = flag_value (affect_flags, affect_string)) != NO_FLAG)
	{
		paf = alloc_perm (sizeof (*paf));
//		paf = reinterpret_cast<AFFECT_DATA *>(alloc_perm (sizeof (*paf)));
		paf->where = TO_AFFECTS;
		paf->level = level;
		paf->duration = -1;
		paf->bitvector = affby;
		switch (affby)
		{
		default:
			free_affect (paf);
			send_to_char ("Huh?\n\r", ch);
			return;
			break;
		case AFF_INVISIBLE:
			send_to_char ("Adding invis affect to object.\n\r", ch);
			paf->type = skill_lookup ("invis");
			paf->location = APPLY_NONE;
			paf->modifier = 0;
			break;
		case AFF_DETECT_EVIL:
			send_to_char ("Adding detect evil affect to object.\n\r", ch);
			paf->type = skill_lookup ("detect evil");
			paf->location = APPLY_NONE;
			paf->modifier = 0;
			break;
		case AFF_DETECT_INVIS:
			send_to_char ("Adding detect invis affect to object.\n\r", ch);
			paf->type = skill_lookup ("detect invis");
			paf->location = APPLY_NONE;
			paf->modifier = 0;
			break;
		case AFF_DETECT_MAGIC:
			send_to_char ("Adding detect magic affect to object.\n\r", ch);
			paf->type = skill_lookup ("detect magic");
			paf->location = APPLY_NONE;
			paf->modifier = 0;
			break;
		case AFF_DETECT_HIDDEN:
			send_to_char ("Adding detect hidden affect to object.\n\r", ch);
			paf->type = skill_lookup ("detect hidden");
			paf->location = APPLY_NONE;
			paf->modifier = 0;
			break;
		case AFF_DETECT_GOOD:
			send_to_char ("Adding detect good affect to object.\n\r", ch);
			paf->type = skill_lookup ("detect good");
			paf->location = APPLY_NONE;
			paf->modifier = 0;
			break;
		case AFF_FAERIE_FIRE:
			send_to_char ("Adding faerie fire affect to object.\n\r", ch);
			paf->type = skill_lookup ("faerie fire");
			paf->location = APPLY_AC;
			paf->modifier = 2 * level;
			break;
		case AFF_INFRARED:
			send_to_char ("Adding infrared affect to object.\n\r", ch);
			paf->type = skill_lookup ("infravision");
			paf->location = APPLY_NONE;
			paf->modifier = 0;
			break;
		case AFF_CURSE:
			send_to_char ("Adding curse affect to object.\n\r", ch);
			paf->type = skill_lookup ("curse");
			paf->location = APPLY_HITROLL;
			paf->modifier = -1 * (level / 8);
			break;
		case AFF_POISON:
			send_to_char ("Adding poison affect to object.\n\r", ch);
			paf->type = skill_lookup ("poison");
			paf->location = APPLY_STR;
			paf->modifier = -2;
			break;
		case AFF_SNEAK:
			send_to_char ("Adding sneak affect to object.\n\r", ch);
			paf->type = skill_lookup ("sneak");
			paf->location = 0;
			paf->modifier = 0;
			break;
		case AFF_HIDE:
			send_to_char ("Adding hide affect to object.\n\r", ch);
			paf->type = skill_lookup ("hide");
			paf->location = 0;
			paf->modifier = 0;
			break;
		case AFF_CAMOUFLAGE:
			send_to_char ("Adding camouflage affect to object.\n\r", ch);
			paf->type = skill_lookup ("camouflage");
			paf->location = 0;
			paf->modifier = 0;
			break;
		case AFF_FLYING:
			send_to_char ("Adding fly affect to object.\n\r", ch);
			paf->type = skill_lookup ("fly");
			paf->location = 0;
			paf->modifier = 0;
			break;
		case AFF_PASS_DOOR:
			send_to_char ("Adding pass door affect to object.\n\r", ch);
			paf->type = skill_lookup ("pass door");
			paf->location = APPLY_NONE;
			paf->modifier = 0;
			break;
		case AFF_PLAGUE:
			send_to_char ("Adding plague affect to object.\n\r", ch);
			paf->type = skill_lookup ("plague");
			paf->location = APPLY_STR;
			paf->modifier = -5;
			break;
		case AFF_WEAKEN:
			send_to_char ("Adding weaken affect to object.\n\r", ch);
			paf->type = skill_lookup ("weaken");
			paf->location = APPLY_STR;
			paf->modifier = -1 * (level / 5);
			break;
		case AFF_SLOW:
			send_to_char ("Adding slow affect to object.\n\r", ch);
			paf->type = skill_lookup ("slow");
			paf->location = APPLY_DEX;
			paf->modifier = -1 - (level >= 18) - (level >= 25) - (level >= 32);
			break;
		case AFF_AQUA_BREATHE:
			send_to_char ("Adding aqua breathe affect to object.\n\r", ch);
			paf->type = skill_lookup ("aqua breathe");
			paf->location = 0;
			paf->modifier = 0;
			break;
		case AFF_SANCTUARY:
			send_to_char ("Adding sanctuary affect to object.\n\r", ch);
			paf->type = skill_lookup ("sanctuary");
			paf->location = APPLY_NONE;
			paf->modifier = 0;
			paf->bitvector = AFF_SANCTUARY;
			break;
		case AFF_HASTE:
			send_to_char ("Adding haste affect to object.\n\r", ch);
			paf->type = skill_lookup ("haste");
			paf->location = APPLY_DEX;
			paf->modifier = 1 + (level >= 18) + (level >= 25) + (level >= 32);
			break;
		case AFF_PROTECT_EVIL:
			send_to_char ("Adding protect evil affect to object.\n\r", ch);
			paf->type = skill_lookup ("protection evil");
			paf->location = APPLY_SAVES;
			paf->modifier = -1;
			break;
		case AFF_PROTECT_GOOD:
			send_to_char ("Adding protect good affect to object.\n\r", ch);
			paf->type = skill_lookup ("protection good");
			paf->location = APPLY_SAVES;
			paf->modifier = -1;
			break;
		}
		paf->location = APPLY_SPELL_AFFECT;
		paf->composition = FALSE;
		paf->comp_name = str_dup ("");
		paf->next = obj->affected;
		obj->affected = paf;
		top_affect++;
		send_to_char ("Ok.\n\r", ch);
	}
	else
	{
		send_to_char ("Unknown affect name used.\n\r", ch);
		send_to_char ("Affect may be one of:\n\r", ch);
		send_to_char
			("      haste          invisible          detect_evil          detect_invis\n\r",
			ch);
		send_to_char
			("      detect_magic   detect_hidden      detect_good          faerie_fire\n\r",
			ch);
		send_to_char
			("      sanctuary      infrared           curse                poison\n\r",
			ch);
		send_to_char
			("      sneak          hide               aqua_breathe         flying\n\r",
			ch);
		send_to_char
			("      pass_door      plague             weaken               slow\n\r",
			ch);
	}
}
void do_whoinfo (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
	{
		send_to_char ("Mobiles can't have a whoinfo you dolt!\r\n", ch);
		return;
	}
	if (argument[0] == '\0')
	{
		send_to_char ("Syntax: whoinfo <who information>\n\r", ch);
		return;
	}
	if (color_strlen (argument) > 22)
	{
		send_to_char ("Whoinfo can't be more than 22 chars.\n\r", ch);
		return;
	}
	smash_tilde (argument);
	free_string (ch->pcdata->whoinfo);
	ch->pcdata->whoinfo = str_dup (argument);
	send_to_char ("Who information changed.\n\r", ch);
}

void do_wiznet (CHAR_DATA * ch, char *argument)
{
	int flag;
	char buf[MAX_STRING_LENGTH];
	if (argument[0] == '\0')
	{
		if (IS_SET (ch->wiznet, WIZ_ON))
		{
			send_to_char ("Signing off of Wiznet.\n\r", ch);
			REMOVE_BIT (ch->wiznet, WIZ_ON);
		}
		else
		{
			send_to_char ("Welcome to Wiznet!\n\r", ch);
			SET_BIT (ch->wiznet, WIZ_ON);
		}
		return;
	}
	if (!str_prefix (argument, "on"))
	{
		send_to_char ("Welcome to Wiznet!\n\r", ch);
		SET_BIT (ch->wiznet, WIZ_ON);
		return;
	}
	if (!str_prefix (argument, "off"))
	{
		send_to_char ("Signing off of Wiznet.\n\r", ch);
		REMOVE_BIT (ch->wiznet, WIZ_ON);
		return;
	}

	/* show wiznet status */
	if (!str_prefix (argument, "status"))
	{
		buf[0] = '\0';
		if (!IS_SET (ch->wiznet, WIZ_ON))
			strcat (buf, "off ");
		for (flag = 0; wiznet_table[flag].name != NULL; flag++)
			if (IS_SET (ch->wiznet, wiznet_table[flag].flag))
			{
				strcat (buf, wiznet_table[flag].name);
				strcat (buf, " ");
			}
			strcat (buf, "\n\r");
			send_to_char ("Wiznet status:\n\r", ch);
			send_to_char (buf, ch);
			return;
	}
	if (!str_prefix (argument, "show"))

		/* list of all wiznet options */
	{
		buf[0] = '\0';
		for (flag = 0; wiznet_table[flag].name != NULL; flag++)
		{
			if (wiznet_table[flag].level <= get_trust (ch))
			{
				strcat (buf, wiznet_table[flag].name);
				strcat (buf, " ");
			}
		}
		strcat (buf, "\n\r");
		send_to_char ("Wiznet options available to you are:\n\r", ch);
		send_to_char (buf, ch);
		return;
	}
	flag = wiznet_lookup (argument);
	if (flag == -1 || get_trust (ch) < wiznet_table[flag].level)
	{
		send_to_char ("No such option.\n\r", ch);
		return;
	}
	if (IS_SET (ch->wiznet, wiznet_table[flag].flag))
	{
		sprintf (buf, "You will no longer see %s on wiznet.\n\r",
			wiznet_table[flag].name);
		send_to_char (buf, ch);
		REMOVE_BIT (ch->wiznet, wiznet_table[flag].flag);
		return;
	}
	else
	{
		sprintf (buf, "You will now see %s on wiznet.\n\r",
			wiznet_table[flag].name);
		send_to_char (buf, ch);
		SET_BIT (ch->wiznet, wiznet_table[flag].flag);
		return;
	}
}
void
wiznet (char *string, CHAR_DATA * ch, OBJ_DATA * obj, long flag,
		long flag_skip, int min_level)
{
	DESCRIPTOR_DATA *d;
	char buffer[10];
	char buf[MAX_STRING_LENGTH];
	time_t curtime;
	struct tm *loctime;
	curtime = time (NULL);
	loctime = localtime (&curtime);
	strftime (buffer, 10, "%H:%M:%S", loctime);
	for (d = descriptor_list; d != NULL; d = d->next)
	{
		if (d->connected == CON_PLAYING && IS_IMMORTAL (d->character)
			&& IS_SET (d->character->wiznet, WIZ_ON)
			&& (!flag || IS_SET (d->character->wiznet, flag))
			&& (!flag_skip || !IS_SET (d->character->wiznet, flag_skip))
			&& get_trust (d->character) >= min_level && d->character != ch)
		{
			if (flag == WIZ_LINKS && !can_see (d->character, ch))
				continue;
			if (IS_SET (d->character->wiznet, WIZ_PREFIX))
				send_to_char ("\r`j--> ", d->character);
			if (IS_SET (d->character->wiznet, WIZ_TSTAMP))
			{
				sprintf (buf, "Current time: %s>> ", buffer);
				send_to_char (buf, d->character);
			}
			act_new (string, d->character, obj, ch, TO_CHAR, POS_DEAD);
			send_to_char (" ``", d->character);
		}
	}
	return;
}


/* equips a character */
void do_outfit (CHAR_DATA * ch, char *argument)
{
	OBJ_DATA *obj, *obj_next;
	int i, sn, vnum, wield_hand;
	if (ch->level > 5 || IS_NPC (ch))
	{
		send_to_char ("Find it yourself!\n\r", ch);
		return;
	}
	if (ch->pcdata->primary_hand == HAND_LEFT)
		wield_hand = WEAR_WIELD_L;

	else
		wield_hand = WEAR_WIELD_R;
	sn = 0;
	vnum = OBJ_VNUM_SCHOOL_SWORD;	/* just in case! */
	for (i = 0; weapon_table[i].name != NULL; i++)
	{
		if (ch->pcdata->learned[sn] < ch->pcdata->learned[*weapon_table[i].gsn])
		{
			sn = *weapon_table[i].gsn;
			vnum = weapon_table[i].vnum;
		}
	}
	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;
		if (vnum != 0 && obj->pIndexData->vnum == vnum)
			equip_char (ch, obj, wield_hand);
		if (obj->pIndexData->vnum == OBJ_VNUM_MAP)
			extract_obj (obj);
		if (obj->pIndexData->vnum == OBJ_VNUM_SCHOOL_BANNER)
			equip_char (ch, obj, WEAR_LIGHT);
		if (obj->pIndexData->vnum == OBJ_VNUM_SCHOOL_VEST)
			equip_char (ch, obj, WEAR_BODY);
		if (obj->pIndexData->vnum == OBJ_VNUM_SCHOOL_SHIELD)
			equip_char (ch, obj, WEAR_SHIELD);
		if (obj->pIndexData->vnum == OBJ_VNUM_SCHOOL_BOOTS)
			equip_char (ch, obj, WEAR_FEET);
		if (obj->pIndexData->vnum == OBJ_VNUM_SCHOOL_LEGS)
			equip_char (ch, obj, WEAR_LEGS);
	}
	if ((obj = get_eq_char (ch, WEAR_LIGHT)) == NULL)
	{
		obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_BANNER), 0);
		obj->cost = 0;
		obj_to_char (obj, ch);
		equip_char (ch, obj, WEAR_LIGHT);
	}
	if ((obj = get_eq_char (ch, WEAR_LEGS)) == NULL)
	{
		obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_LEGS), 0);
		obj->cost = 0;
		obj_to_char (obj, ch);
		equip_char (ch, obj, WEAR_LEGS);
	}
	if ((obj = get_eq_char (ch, WEAR_FEET)) == NULL)
	{
		obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_BOOTS), 0);
		obj->cost = 0;
		obj_to_char (obj, ch);
		equip_char (ch, obj, WEAR_FEET);
	}
	if ((obj = get_eq_char (ch, WEAR_BODY)) == NULL)
	{
		obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_VEST), 0);
		obj->cost = 0;
		obj_to_char (obj, ch);
		equip_char (ch, obj, WEAR_BODY);
	}

	/* do the weapon thing */
	if (!IS_CLASS (ch, PC_CLASS_REAVER) && (obj = get_eq_char (ch, wield_hand)) == NULL)
	{
		if (vnum != 0)
		{
			obj = create_object (get_obj_index (vnum), 0);
			obj_to_char (obj, ch);
			equip_char (ch, obj, wield_hand);
		}
	}
	obj_to_char (create_object (get_obj_index (OBJ_VNUM_MAP), 0), ch);
	if (((obj = get_eq_char (ch, wield_hand)) == NULL
		|| !IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS))
		&& (obj = get_eq_char (ch, WEAR_SHIELD)) == NULL && !(IS_CLASS (ch, PC_CLASS_ASSASSIN)))
	{
		obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_SHIELD), 0);
		obj->cost = 0;
		obj_to_char (obj, ch);
		equip_char (ch, obj, WEAR_SHIELD);
	}
	send_to_char ("You have been equipped with newbie gear.\n\r", ch);
}


/* shutup command, for those really annoying spammers */
// Added 6/19/04 makes it so a player can still use channels but no one hears them
void do_shutup (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	long ticklength;
	argument =  one_argument (argument, arg);
	one_argument (argument, arg2);
	if (arg[0] == '\0')
	{
		send_to_char ("Shutup X (where X is the tick amount) whom?", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (get_trust (victim) > get_trust (ch))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (arg2 != NULL)
		ticklength = is_number (arg2) ? atol (arg2) : 0;
	else ticklength = 0;
	if (IS_SET (victim->comm2, COMM_SHUTUP))
	{
		REMOVE_BIT (victim->comm2, COMM_SHUTUP);
		if (IS_SET (victim->comm, COMM_NOSHOUT))
			do_noshout(ch,arg); 
		sprintf (buf, "$N unshuts up  %s", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
		send_to_char ("SHUTUP remove.\n\r",ch);
	}
	else
	{
		SET_BIT (victim->comm2, COMM_SHUTUP);
		//     if (!IS_SET (victim->comm, COMM_NOSHOUT))
		//       do_noshout(ch,arg);
		send_to_char ("SHUTUP set.\n\r", ch);
		if (!IS_NPC(victim))
			victim->pcdata->shutup_ticks = ticklength;
		sprintf (buf, "$N makes %s shutup.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	return;
}
/* RT nochannels command, for those spammers */
void do_nochannels (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	long ticklength;
	argument =  one_argument (argument, arg);
	one_argument (argument, arg2);
	if (arg[0] == '\0')
	{
		send_to_char ("Nochannel X (where X is the tick amount) whom?", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (get_trust (victim) > get_trust (ch))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (arg2 != NULL)
		ticklength = is_number (arg2) ? atol (arg2) : 0;
	else ticklength = 0;
	if (IS_SET (victim->comm, COMM_NOCHANNELS))
	{
		REMOVE_BIT (victim->comm, COMM_NOCHANNELS);
		if (IS_SET (victim->comm, COMM_NOSHOUT))
			do_noshout(ch,arg);
		send_to_char
			("The gods have restored your channel priviliges.\n\r", victim);
		send_to_char ("NOCHANNELS removed.\n\r", ch);
		sprintf (buf, "$N restores channels to %s", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else
	{
		SET_BIT (victim->comm, COMM_NOCHANNELS);
		if (!IS_SET (victim->comm, COMM_NOSHOUT))
			do_noshout(ch,arg);
		send_to_char ("The gods have revoked your channel priviliges.\n\r", victim);
		send_to_char ("NOCHANNELS set.\n\r", ch);
		if (!IS_NPC(victim))
			victim->pcdata->nochan_ticks = ticklength;
		sprintf (buf, "$N revokes %s's channels.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	return;
}

void do_smote (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *vch;
	char *letter, *name;
	char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
	int matches = 0;
	if (!IS_NPC (ch) && IS_SET (ch->comm, COMM_NOEMOTE))
	{
		send_to_char ("You can't show your emotions.\n\r", ch);
		return;
	}
	if (argument[0] == '\0')
	{
		send_to_char ("Emote what?\n\r", ch);
		return;
	}
	if (!IS_NPC (ch))
		if (strstr (argument, ch->name) == NULL)
		{
			send_to_char ("You must include your name in an smote.\n\r", ch);
			return;
		}
		send_to_char (argument, ch);
		send_to_char ("\n\r", ch);
		for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
		{
			if (vch->desc == NULL || vch == ch)
				continue;
			if ((letter = strstr (argument, vch->name)) == NULL)
			{
				send_to_char (argument, vch);
				send_to_char ("\n\r", vch);
				continue;
			}
			strcpy (temp, argument);
			temp[strlen (argument) - strlen (letter)] = '\0';
			last[0] = '\0';
			name = vch->name;
			for (; *letter != '\0'; letter++)
			{
				if (*letter == '\'' && matches == strlen (vch->name))
				{
					strcat (temp, "r");
					continue;
				}
				if (*letter == 's' && matches == strlen (vch->name))
				{
					matches = 0;
					continue;
				}
				if (matches == strlen (vch->name))
				{
					matches = 0;
				}
				if (*letter == *name)
				{
					matches++;
					name++;
					if (matches == strlen (vch->name))
					{
						strcat (temp, "you");
						last[0] = '\0';
						name = vch->name;
						continue;
					}
					strncat (last, letter, 1);
					continue;
				}
				matches = 0;
				strcat (temp, last);
				strncat (temp, letter, 1);
				last[0] = '\0';
				name = vch->name;
			}
			send_to_char (temp, vch);
			send_to_char ("\n\r", vch);
		}
		return;
}

void do_bamfin (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	if (!IS_NPC (ch))
	{
		smash_tilde (argument);
		if (argument[0] == '\0')
		{
			sprintf (buf, "Your poofin is %s\n\r", ch->pcdata->bamfin);
			send_to_char (buf, ch);
			return;
		}
		if (strstr (argument, ch->name) == NULL)
		{
			send_to_char ("You must include your name.\n\r", ch);
			return;
		}
		free_string (ch->pcdata->bamfin);
		ch->pcdata->bamfin = str_dup (argument);
		sprintf (buf, "Your poofin is now %s\n\r", ch->pcdata->bamfin);
		send_to_char (buf, ch);
	}
	return;
}

void do_bamfout (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	if (!IS_NPC (ch))
	{
		smash_tilde (argument);
		if (argument[0] == '\0')
		{
			sprintf (buf, "Your poofout is %s\n\r", ch->pcdata->bamfout);
			send_to_char (buf, ch);
			return;
		}
		if (strstr (argument, ch->name) == NULL)
		{
			send_to_char ("You must include your name.\n\r", ch);
			return;
		}
		free_string (ch->pcdata->bamfout);
		ch->pcdata->bamfout = str_dup (argument);
		sprintf (buf, "Your poofout is now %s\n\r", ch->pcdata->bamfout);
		send_to_char (buf, ch);
	}
	return;
}

void do_deny (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Deny whom?\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Not on NPC's.\n\r", ch);
		return;
	}
	if (get_trust (victim) >= get_trust (ch))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (IS_SET (victim->act, PLR_DENY))
	{
		REMOVE_BIT (victim->act, PLR_DENY);
		send_to_char ("You are allowed access.\n\r", victim);
		sprintf(buf, "You allow access to %s.\n\r", victim->name);
		send_to_char (buf , ch);
		sprintf (buf, "$N allows access to %s", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
		send_to_char ("OK.\n\r", ch);
		save_char_obj (victim);
		stop_fighting (victim, TRUE);
		do_quit (victim, "");
	}
	else
	{
		SET_BIT (victim->act, PLR_DENY);
		send_to_char ("You are denied access!\n\r", victim);
		sprintf (buf, "$N denies access to %s", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
		send_to_char ("OK.\n\r", ch);
		save_char_obj (victim);
		stop_fighting (victim, TRUE);
		do_quit (victim, "");
	}
	return;
}

void do_disconnect (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Disconnect whom?\n\r", ch);
		return;
	}
	if (is_number (arg))
	{
		int desc;
		desc = atol (arg);
		for (d = descriptor_list; d != NULL; d = d->next)
		{
			if (d->descriptor == desc)
			{
				close_socket (d);
				send_to_char ("Ok.\n\r", ch);
				return;
			}
		}
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (victim->desc == NULL)
	{
		act ("$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR);
		return;
	}
	for (d = descriptor_list; d != NULL; d = d->next)
	{
		if (d == victim->desc)
		{
			if (get_trust (victim) >= get_trust (ch))
			{
				send_to_char ("Don't mess with people stronger than you!\n\r", ch);
				return;
			}
			close_socket (d);
			send_to_char ("Ok.\n\r", ch);
			return;
		}
	}
	bug ("Do_disconnect: desc not found.", 0);
	send_to_char ("Descriptor not found!\n\r", ch);
	return;
}

void do_jail (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *location;
	argument = one_argument (argument, arg1);
	if (arg1[0] == '\0')
	{
		send_to_char ("Syntax: jail <character>.\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Not on NPC's.\n\r", ch);
		return;
	}
	//  if (victim == ch)
	//   {
	//    send_to_char ("You can't jail yourself.\n\r", ch);
	//   return;
	//}
	send_to_char ("Ok.\n\r", ch);
	location = ch->in_room;
	ch->in_room = victim->in_room;
	arrest (ch, victim);
	ch->in_room = location;
}

void do_pardon (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	argument = one_argument (argument, arg1);
	if (arg1[0] == '\0')
	{
		send_to_char ("Syntax: pardon <character>.\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Not on NPC's.\n\r", ch);
		return;
	}
	if (IS_SET (victim->act, PLR_WANTED) || IS_SET (victim->act, PLR_JAILED))
	{
		unjail (victim);
		send_to_char ("Ok.\n\r", ch);
		send_to_char ("You are no longer WANTED.\n\r", victim);
		return;
	}
	send_to_char ("That player is not wanted.\n\r", ch);
	return;
}

void do_echo (CHAR_DATA * ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	char tcbuf[MAX_STRING_LENGTH];
	if (argument[0] == '\0')
	{
		send_to_char ("Global echo what?\n\r", ch);
		return;
	}
	if (ch != NULL)
		sprintf (tcbuf, "global(%s)> ", ch->name);

	else
		sprintf (tcbuf, "global(NULL)> ");
	for (d = descriptor_list; d; d = d->next)
	{
		if (d->connected == CON_PLAYING)
		{
			send_to_char (argument, d->character);
			send_to_char ("\n\r", d->character);
		}
	}
	return;
}

void do_recho (CHAR_DATA * ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	if (argument[0] == '\0')
	{
		send_to_char ("Local echo what?\n\r", ch);
		return;
	}
	for (d = descriptor_list; d; d = d->next)
	{
		if (d->connected == CON_PLAYING && d->character->in_room == ch->in_room)
		{
			send_to_char (argument, d->character);
			send_to_char ("\n\r", d->character);
		}
	}
	return;
}

void do_trecho (CHAR_DATA * ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if (argument[0] == '\0')
	{
		send_to_char ("Target Room echo who what?\n\r", ch);
		return;
	}
	argument = one_argument (argument, arg);
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("Target not found.\n\r", ch);
		return;
	}

	for (d = descriptor_list; d; d = d->next)
	{
		if (d->connected == CON_PLAYING && d->character->in_room == ch->in_room
			&& d->character != victim)
		{
			send_to_char (argument, d->character);
			send_to_char ("\n\r", d->character);
		}
	}
	send_to_char ("targeted room> ", ch);
	send_to_char (argument, ch);
	send_to_char ("\n\r", ch);

	return;
}


void do_zecho (CHAR_DATA * ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	if (argument[0] == '\0')
	{
		send_to_char ("Zone echo what?\n\r", ch);
		return;
	}
	for (d = descriptor_list; d; d = d->next)
	{
		if (d->connected == CON_PLAYING
			&& d->character->in_room != NULL && ch->in_room != NULL
			&& d->character->in_room->area == ch->in_room->area)
		{
			send_to_char (argument, d->character);
			send_to_char ("\n\r", d->character);
		}
	}
}
void do_pecho (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	argument = one_argument (argument, arg);
	if (argument[0] == '\0' || arg[0] == '\0')
	{
		send_to_char ("Personal echo what?\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("Target not found.\n\r", ch);
		return;
	}
	send_to_char (argument, victim);
	send_to_char ("\n\r", victim);
	send_to_char ("personal> ", ch);
	send_to_char (argument, ch);
	send_to_char ("\n\r", ch);
}

ROOM_INDEX_DATA *find_location (CHAR_DATA * ch, char *arg)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	if (is_number (arg))
		return get_room_index (atol (arg));
	if ((victim = get_char_world (ch, arg)) != NULL)
		return victim->in_room;
	if ((obj = get_obj_world (ch, arg)) != NULL)
		return obj->in_room;
	return NULL;
}

void do_transfer (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char tcbuf[15];
	ROOM_INDEX_DATA *location;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim, *rch;
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	if (arg1[0] == '\0')
	{
		send_to_char ("Transfer whom (and where)?\n\r", ch);
		return;
	}
	if (!str_cmp (arg1, "all"))
	{
		for (d = descriptor_list; d != NULL; d = d->next)
		{
			if (d->connected == CON_PLAYING
				&& d->character != ch
				&& d->character->in_room != NULL && can_see (ch, d->character))
			{
				char buf[MAX_STRING_LENGTH];
				sprintf (buf, "%s %s", d->character->name, arg2);
				do_transfer (ch, buf);
			}
		}
		return;
	}

	/*
	* Thanks to Grodyn for the optional location parameter.
	*/
	if (arg2[0] == '\0')
	{
		location = ch->in_room;
	}
	else
	{
		if ((location = find_location (ch, arg2)) == NULL)
		{
			send_to_char ("No such location.\n\r", ch);
			return;
		}
		if (!is_room_owner (ch, location) && room_is_private (location)
			&& get_trust (ch) < MAX_LEVEL)
		{
			send_to_char ("That room is private right now.\n\r", ch);
			return;
		}
	}
	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (!IS_NPC (victim))
		if (victim->desc != NULL)
			if (victim->desc->pEdit != NULL)
			{
				send_to_char ("They are building a room.\n\r", ch);
				return;
			}
			if (IS_NPC (victim) && victim->riders != NULL)
			{
				send_to_char ("Not that target.\n\r", ch);
				return;
			}
			if (victim->in_room == NULL)
			{
				send_to_char ("They are in limbo.\n\r", ch);
				return;
			}
			if (IS_SET(location->area->area_flags,AREA_IMP_ONLY) && get_trust(victim) < MAX_LEVEL)
			{
				send_to_char ("You failed.\n\r",ch);
				return;
			}

			if (victim->fighting != NULL)
				stop_fighting (victim, TRUE);
			if (victim->pcdata && victim->pcdata->fishing)
				victim->pcdata->fishing = 0;
			if (!battle_royale && (!IS_NPC(ch) || ch->pIndexData->vnum != MOB_VNUM_FAKIE))
				for (rch = victim->in_room->people; rch != NULL; rch = rch->next_in_room)
				{
					if (get_trust (rch) >= victim->invis_level)
					{
						act ("$n disappears in a mushroom cloud.", victim, NULL,
							rch, TO_VICT);
					}
				}
				char_from_room (victim);
				char_to_room (victim, location);
				if (!battle_royale && (!IS_NPC(ch) || ch->pIndexData->vnum != MOB_VNUM_FAKIE))
				{
					for (rch = victim->in_room->people; rch != NULL; rch = rch->next_in_room)
					{
						if (get_trust (rch) >= victim->invis_level)
						{
							act ("$n arrives from a puff of smoke.", victim, NULL, rch,
								TO_VICT);
						}
					}
				}
				if (ch != victim && (!IS_NPC(ch) || ch->pIndexData->vnum != MOB_VNUM_FAKIE))
					act ("$n has transferred you.", ch, NULL, victim, TO_VICT);
				do_look (victim, "auto");
				trip_triggers(victim, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
				trap_check(victim,"room",victim->in_room,NULL);
				sprintf(tcbuf,"%d",victim->in_room->vnum);
				trip_triggers_arg(victim, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
				check_aggression(victim);
				if (!battle_royale)
					send_to_char ("Ok.\n\r", ch);
}

void do_at (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *original;
	OBJ_DATA *on;
	CHAR_DATA *wch;
	argument = one_argument (argument, arg);
	if (arg[0] == '\0' || argument[0] == '\0')
	{
		send_to_char ("At where what?\n\r", ch);
		return;
	}
	if ((location = find_location (ch, arg)) == NULL)
	{
		send_to_char ("No such location.\n\r", ch);
		return;
	}
	if (!is_room_owner (ch, location) && room_is_private (location)
		&& get_trust (ch) < MAX_LEVEL)
	{
		send_to_char ("That room is private right now.\n\r", ch);
		return;
	}
	original = ch->in_room;
	on = ch->on;
	char_from_room (ch);
	char_to_room (ch, location);
	interpret (ch, argument);

	/*
	* See if 'ch' still exists before continuing!
	* Handles 'at XXXX quit' case.
	*/
	for (wch = char_list; wch != NULL; wch = wch->next)
	{
		if (wch == ch)
		{
			char_from_room (ch);
			char_to_room (ch, original);
			ch->on = on;
			break;
		}
	}
	return;
}


// do_linkload, loads an offline playerfile and brings it to the immortals
// location, one can then manipulate stats, freeze, raise levels or other things
// Added by Morgan June 20. 2000 .... Still not entirely satisfied with how it works
// this is an ongoing project.....
void do_linkload (CHAR_DATA * ch, char *argument)
{
	DESCRIPTOR_DATA d;
	bool isChar;
	char name[MAX_INPUT_LENGTH];
	char buffer[MAX_STRING_LENGTH];
	if (argument[0] == '\0')
	{
		send_to_char ("Load who?\n\r", ch);
		return;
	}
	argument[0] = UPPER (argument[0]);
	argument = one_argument (argument, name);

	// Dont want to load a second copy of a player who's already online! 
	if (get_char_world (ch, name) != NULL)
	{
		send_to_char ("That person is already connected!\n\r", ch);
		return;
	}
	name[0] = UPPER (name[0]);
	is_linkloading = TRUE;
	isChar = load_char_obj (&d, name);	// char pfile exists?*
	is_linkloading = FALSE;
	if (!isChar)
	{
		sprintf (buffer, "That character name (%s) is unknown.\n\r", name);
		send_to_char (buffer, ch);
		return;
	}
	d.character->desc = NULL;
	d.character->next = char_list;
	char_list = d.character;
	d.connected = CON_PLAYING;
	reset_char (d.character);

	// bring player to me  
	if (d.character->in_room != NULL)
	{
		d.character->was_in_room = d.character->in_room;

		// char_from_room(d.character);
		char_to_room (d.character, ch->in_room);	// put in room imm is in *
	}
	act ("$n has created $N.", ch, NULL, d.character, TO_ROOM);
	sprintf (buffer, "Loading %s.\n\r", d.character->name);
	send_to_char (buffer, ch);
	if (d.character->pet != NULL)
	{
		char_to_room (d.character->pet, d.character->in_room);
		act ("$n has entered the game.", d.character->pet, NULL, NULL, TO_ROOM);
	}
}


// do_lunload, unloads an offline playerfile, it is supposed to return the char
// to the location the char was in at linkload time, this part of it does not work
// Added by Morgan June 20. 2000 .... Still not entirely satisfied with how it works
// this is an ongoing project.....
// command yanked on live port till unload part works satisfactory
void do_lunload (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	char who[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	argument = one_argument (argument, who);
	if ((victim = get_char_world (ch, who)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	/** Person is legitimately logged on... was not loaded.
	*/
	if (victim->desc != NULL)
	{
		send_to_char ("Do you want to piss the players off?\n\r", ch);
		return;
	}
	if (victim->in_room->vnum == ROOM_VNUM_LIMBO)
	{
		send_to_char
			("Safety precaution, this character is in void and cannot be saved out.\n\r",
			ch);
		return;
	}
	if (victim->was_in_room != NULL)
	{				/* return player and pet to orig room */
		char_from_room (victim);
		char_to_room (victim, victim->was_in_room);
		if (victim->pet != NULL)
		{
			char_from_room (victim->pet);
			char_to_room (victim->pet, victim->was_in_room);
		}
	}
	save_char_obj (victim);
	is_linkloading = TRUE;
	do_quit (victim, "");
	is_linkloading = FALSE;
	sprintf (buf, "%s has destroyed %s.", ch->name, victim->name);
	act (buf, ch, NULL, NULL, TO_ROOM);
}

void do_goto (CHAR_DATA * ch, char *argument)
{
	ROOM_INDEX_DATA *location;
	CHAR_DATA *rch;
	char tcbuf[15];
	int count = 0;
	if (argument[0] == '\0')
	{
		send_to_char ("Goto where?\n\r", ch);
		return;
	}
	if ((location = find_location (ch, argument)) == NULL)
	{
		send_to_char ("No such location.\n\r", ch);
		return;
	}
	count = 0;
	for (rch = location->people; rch != NULL; rch = rch->next_in_room)
		count++;
	if (!is_room_owner (ch, location) && room_is_private (location)
		&& (count > 1 || get_trust (ch) < MAX_LEVEL))
	{
		send_to_char ("That room is private right now.\n\r", ch);
		return;
	}
	if (IS_SET(location->area->area_flags,AREA_IMP_ONLY) && get_trust(ch) < MAX_LEVEL)
	{
		send_to_char ("You failed.\n\r",ch);
		return;
	}
	if (ch->fighting != NULL)
		stop_fighting (ch, TRUE);
	if (ch->pcdata && ch->pcdata->fishing)
		ch->pcdata->fishing = 0;
	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
	{
		if (get_trust (rch) >= ch->invis_level)
		{
			if (!IS_NPC (ch) && ch->pcdata != NULL
				&& ch->pcdata->bamfout[0] != '\0')
				act ("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);

			else
				act ("$n leaves in a swirling mist.", ch, NULL, rch, TO_VICT);
		}
	}
	char_from_room (ch);
	char_to_room (ch, location);
	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
	{
		if (get_trust (rch) >= ch->invis_level)
		{
			if (!IS_NPC (ch) && ch->pcdata != NULL
				&& ch->pcdata->bamfin[0] != '\0')
				act ("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);

			else
				act ("$n appears in a swirling mist.", ch, NULL, rch, TO_VICT);
		}
	}
	do_look (ch, "auto");
	trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	trap_check(ch,"room",ch->in_room,NULL);
	sprintf(tcbuf,"%d",ch->in_room->vnum);
	trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
	check_aggression(ch);
	return;
}

void do_violate (CHAR_DATA * ch, char *argument)
{
	ROOM_INDEX_DATA *location;
	CHAR_DATA *rch;
	char tcbuf[15];
	int failed = FALSE;
	if (argument[0] == '\0')
	{
		send_to_char ("Goto where?\n\r", ch);
		return;
	}
	if ((location = find_location (ch, argument)) == NULL)
	{
		send_to_char ("No such location.\n\r", ch);
		return;
	}
	if (IS_SET (location->room_flags, ROOM_IMP_ONLY) && ch->level < MAX_LEVEL)
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (IS_SET(location->area->area_flags,AREA_IMP_ONLY) && get_trust(ch) < MAX_LEVEL)
	{
		send_to_char ("You failed.\n\r",ch);
		return;
	}

	if (ch->fighting != NULL)
		stop_fighting (ch, TRUE);
	for (rch = location->people; rch != NULL; rch = rch->next_in_room)
		if (get_trust (rch) > get_trust (ch))
		{
			act ("$N tried to violate this room, but failed.", rch, NULL,
				ch, TO_CHAR);
			failed = TRUE;
		}
		if (failed)
		{
			send_to_char ("You failed.\n\r", ch);
			return;
		}
		for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
		{
			if (get_trust (rch) >= ch->invis_level)
			{
				if (!IS_NPC (ch) && ch->pcdata != NULL
					&& ch->pcdata->bamfout[0] != '\0')
					act ("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);

				else
					act ("$n leaves in a swirling mist.", ch, NULL, rch, TO_VICT);
			}
		}
		char_from_room (ch);
		char_to_room (ch, location);
		for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
		{
			if (get_trust (rch) >= ch->invis_level)
			{
				if (!IS_NPC (ch) && ch->pcdata != NULL
					&& ch->pcdata->bamfin[0] != '\0')
					act ("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);

				else
					act ("$n appears in a swirling mist.", ch, NULL, rch, TO_VICT);
			}
		}
		do_look (ch, "auto");
		trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
		trap_check(ch,"room",ch->in_room,NULL);
		sprintf(tcbuf,"%d",ch->in_room->vnum);
		trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
		check_aggression(ch);
		return;
}


/* RT to replace the 3 stat commands */
void do_stat (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char *string;
	string = one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Syntax:\n\r", ch);
		send_to_char ("  stat obj <name>\n\r", ch);
		send_to_char ("  stat mob <name>\n\r", ch);
		send_to_char ("  stat char <name> <option>\n\r", ch);
		send_to_char ("  stat room <number>\n\r", ch);
		return;
	}
	if (!str_cmp (arg, "room"))
	{
		do_rstat (ch, string);
		return;
	}
	if (!str_cmp (arg, "obj"))
	{
		do_ostat (ch, string);
		return;
	}
	if (!str_cmp (arg, "char") || !str_cmp (arg, "mob"))
	{
		do_mstat (ch, string);
		return;
	}
	do_stat (ch, "");
	return;

	/* Here used to be a link to the old way of statting items,
	however, to educate people, and to avoid confusion, that
	has been taken out, now you have to name the type of stats
	you want       (Morgan, Feb, 24. 2001                   
	*/
}

void do_rstat (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char doorname[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	OBJ_DATA *obj;
	CHAR_DATA *rch;
	SCENT_DATA *sc;
	int door;
	one_argument (argument, arg);
	location = (arg[0] == '\0') ? ch->in_room : find_location (ch, arg);
	if (location == NULL)
	{
		send_to_char ("No such location.\n\r", ch);
		return;
	}
	if (!is_room_owner (ch, location) && ch->in_room != location
		&& room_is_private (location) && !IS_TRUSTED (ch, OVERLORD))
	{
		send_to_char ("That room is private right now.\n\r", ch);
		return;
	}
	sprintf (buf, "`jName:`` '%s'\n\r`jArea:`` '%s'\n\r", location->name,
		location->area->name);
	send_to_char (buf, ch);
	sprintf (buf,
		"`jVnum:`` %d  `jSector:`` %d  `jLight:`` %d  `jHealing:`` %d `jMana:`` %d\n\r",
		location->vnum, location->sector_type, location->light,
		location->heal_rate, location->mana_rate);
	send_to_char (buf, ch);
	sprintf (buf, "`jRoom flags:`` %s %s.\n\r`jDescription:``\n\r%s",
		room_bit_name (location->room_flags),
		room2_bit_name (location->room_flags2), location->description);
	send_to_char (buf, ch);
	sprintf (buf,
		"`jExploration Points:`` %ld  `jExploration Max. Level:`` %d\n\r",
		location->tp_exp, location->tp_level);
	send_to_char (buf, ch);
	if (location->extra_descr != NULL)
	{
		EXTRA_DESCR_DATA *ed;
		send_to_char ("`jExtra description keywords:`` '", ch);
		for (ed = location->extra_descr; ed; ed = ed->next)
		{
			send_to_char (ed->keyword, ch);
			if (ed->next != NULL)
				send_to_char (" ", ch);
		}
		send_to_char ("'.\n\r", ch);
	}
	send_to_char ("`jCharacters:``", ch);
	for (rch = location->people; rch; rch = rch->next_in_room)
	{
		if (can_see (ch, rch))
		{
			send_to_char (" ", ch);
			one_argument (rch->name, buf);
			send_to_char (buf, ch);
		}
	}
	send_to_char (".\n\r`jObjects:``   ", ch);
	for (obj = location->contents; obj; obj = obj->next_content)
	{
		send_to_char (" ", ch);
		one_argument (obj->name, buf);
		send_to_char (buf, ch);
	}
	send_to_char (".\n\r", ch);
	send_to_char ("`jScents:`` \n\r", ch);
	for (sc = location->scents; sc; sc = sc->next_in_room)
	{
		sprintf (buf, "`jPlayer:`` %s\t`jLevel:`` ", sc->player->name);
		switch (sc->scent_level)
		{
		default:
			strcat (buf, "Gone.\n\r");
			break;
		case SCENT_STRONG:
			strcat (buf, "Strong.\n\r");
			break;
		case SCENT_NOTICEABLE:
			strcat (buf, "Noticeable.\n\r");
			break;
		case SCENT_WEAK:
			strcat (buf, "Weak.\n\r");
			break;
		}
		send_to_char (buf, ch);
	}
	for (door = 0; door <= 5; door++)
	{
		EXIT_DATA *pexit;
		if (door == 0)
			sprintf (doorname, "North");

		else if (door == 1)
			sprintf (doorname, "East");

		else if (door == 2)
			sprintf (doorname, "South");

		else if (door == 3)
			sprintf (doorname, "West");

		else if (door == 4)
			sprintf (doorname, "Up");

		else if (door == 5)
			sprintf (doorname, "Down");

		else
			sprintf (doorname, "none");
		if ((pexit = location->exit[door]) != NULL)
		{
			sprintf (buf,
				"`jDoor:`` %s.  `jTo:`` %d.  `jKey:`` %d.  `jExit flags:`` %d.\n\r`jKeyword:`` '%s'.  `jDescription:`` %s",
				doorname,
				(pexit->u1.to_room ==
				NULL ? -1 : pexit->u1.to_room->vnum), pexit->key,
				pexit->exit_info, pexit->keyword,
				pexit->description[0] !=
				'\0' ? pexit->description : "(none).\n\r");
			send_to_char (buf, ch);
		}
	}
	if (IS_SET (location->race_flags, ROOM_PLAYERSTORE)
		&& location->owner[0] != '\0')
	{
		char tcbuf[MAX_STRING_LENGTH];
		sprintf (tcbuf, "`jRoom is a player-store owned by`` %s.  ",
			location->owner);
		send_to_char (tcbuf, ch);
		if (store_closed (location))
			send_to_char ("`jThe store is currently closed.``\n\r", ch);

		else
			send_to_char ("`jThe store is currently open.``\n\r", ch);
	}
	if (IS_SET (location->race_flags, ROOM_NOTELEPORT)
		&& location->owner[0] != '\0')
	{
		char tcbuf[MAX_STRING_LENGTH];
		sprintf (tcbuf,
			"`j%s has bought the ``no_teleport `jflag for this room.\n\r",
			location->owner);
		send_to_char (tcbuf, ch);
	}
	return;
}

void do_ostat (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	short i=0;
	bool found=FALSE;
	AFFECT_DATA *paf;
	OBJ_DATA *obj;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Stat what?\n\r", ch);
		return;
	}
	if ((obj = get_obj_world (ch, argument)) == NULL)
	{
		send_to_char ("Nothing like that in hell, earth, or heaven.\n\r", ch);
		return;
	}
	if (obj->item_type == ITEM_PORTAL)
		adjust_portal_weight (obj);
	sprintf (buf, "`jName(s):`` %s\n\r", obj->name);
	send_to_char (buf, ch);
	sprintf (buf,
		"`jVnum:`` %d  `jFormat:`` %s  `jType:`` %s  `jResets:`` %d\n\r",
		obj->pIndexData->vnum, "new", item_type_name (obj),
		obj->pIndexData->reset_num);
	send_to_char (buf, ch);
	sprintf (buf,
		"`jShort description:`` %s\n\r`jLong description:`` %s\n\r",
		obj->short_descr, obj->description);
	send_to_char (buf, ch);
	sprintf (buf, "`jWear bits:`` %s\n\r`jExtra bits:`` %s\n\r",
		wear_bit_name (obj->wear_flags),
		extra_bit_name (obj->extra_flags));
	send_to_char (buf, ch);

	// Akamai 4/30/99 - support Class/race object restrictions
	sprintf (buf, "`jClasses Allowed:`` [%s]\n\r",
		crflag_string (Class_flags, obj->Class_flags));
	send_to_char (buf, ch);
	sprintf (buf, "`jRaces Allowed:``   [%s]\n\r",
		crflag_string (pcrace_flags, obj->race_flags));
	send_to_char (buf, ch);
	sprintf (buf, "`jClans Allowed:``   [%s]\n\r",
		crflag_string (clan_flags, obj->clan_flags));
	send_to_char (buf, ch);
	for (i=0;i<MAX_OBJ_TRIGS;i++)
	{
		if (obj->objtrig[i])
		{
			sprintf(buf, "`jObj Trigger %d Vnum:``  [%d]\n\r", i, obj->objtrig[i]->pIndexData->vnum);
			send_to_char (buf,ch);
			found = TRUE;
		}
	}
	if (!found)
	{
		sprintf(buf, "`jObj Trigger Vnum:``  No Object Trigger\n\r");
		send_to_char (buf,ch);
	}

	sprintf (buf,
		"`jNumber:`` %d/%d  `jWeight:`` %d/%d/%d `j(10th pounds)``\n\r",
		1, get_obj_number (obj), obj->weight, get_obj_weight (obj),
		get_true_weight (obj));
	send_to_char (buf, ch);
	sprintf (buf,
		"`jLevel:`` %d  `jCost:`` %d  `jCondition:`` %d  `jTimer:`` %d `jOwner:`` %s\n\r",
		obj->level, obj->cost, obj->condition, obj->timer, (obj->plr_owner == NULL)?"none":capitalize(obj->plr_owner));
	send_to_char (buf, ch);
	sprintf (buf,
		"`jIn room:`` %d  `jIn object:`` %s  `jCarried by:`` %s `jWear_loc:`` %ld\n\r",
		obj->in_room == NULL ? 0 : obj->in_room->vnum,
		obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr,
		obj->carried_by == NULL ? "(none)" : can_see (ch,
		obj->
		carried_by) ?
		obj->carried_by->name : "someone", obj->wear_loc);
	send_to_char (buf, ch);
	if (obj->item_type == ITEM_CARD)
	{
		sprintf (buf, "`jValues:`` %d %d %d %d %d %d %d %d %d %d %d %d %d\n\r", 
			obj->value[0], obj->value[1], obj->value[2],
			obj->value[3], obj->value[4], obj->value[5], obj->value[6], obj->value[7], obj->value[8],
			obj->value[9], obj->value[10], obj->value[11], obj->value[12]);
	}
	else sprintf (buf, "`jValues:`` %d %d %d %d %d %d %d\n\r", obj->value[0],
		obj->value[1], obj->value[2], obj->value[3], obj->value[4], obj->value[5], obj->value[6]);
	send_to_char (buf, ch);

	/* now give out vital statistics as per identify */
	switch (obj->item_type)
	{
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		sprintf (buf, "`jLevel`` %d `jspells of:``", obj->value[0]);
		send_to_char (buf, ch);
		if (obj->value[1] >= 0 && obj->value[1] < MAX_SKILL)
		{
			send_to_char (" '", ch);
			send_to_char (skill_table[obj->value[1]].name, ch);
			send_to_char ("'", ch);
		}
		if (obj->value[2] >= 0 && obj->value[2] < MAX_SKILL)
		{
			send_to_char (" '", ch);
			send_to_char (skill_table[obj->value[2]].name, ch);
			send_to_char ("'", ch);
		}
		if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
		{
			send_to_char (" '", ch);
			send_to_char (skill_table[obj->value[3]].name, ch);
			send_to_char ("'", ch);
		}
		if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
		{
			send_to_char (" '", ch);
			send_to_char (skill_table[obj->value[4]].name, ch);
			send_to_char ("'", ch);
		}
		send_to_char (".\n\r", ch);
		break;
	case ITEM_WAND:
	case ITEM_STAFF:
		sprintf (buf, "`jHas`` %d(%d) `jcharges of level`` %d",
			obj->value[1], obj->value[2], obj->value[0]);
		send_to_char (buf, ch);
		if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
		{
			send_to_char (" '", ch);
			send_to_char (skill_table[obj->value[3]].name, ch);
			send_to_char ("'", ch);
		}
		send_to_char (".\n\r", ch);
		break;
	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
		if (obj->value[2] < 0)
			send_to_char
			("Can't stat this ITEM_DRINK_CON because it's liquid type isn't set properly.  Please change it.\n\r",
			ch);

		else
		{
			sprintf (buf, "`jIt holds`` %s-colored %s.\n\r",
				liq_table[obj->value[2]].liq_color,
				liq_table[obj->value[2]].liq_name);
			send_to_char (buf, ch);
		}
		break;
	case ITEM_WEAPON:
		send_to_char ("`jWeapon type is`` ", ch);
		switch (obj->value[0])
		{
		case (WEAPON_EXOTIC):
			send_to_char ("exotic\n\r", ch);
			break;
		case (WEAPON_SWORD):
			send_to_char ("sword\n\r", ch);
			break;
		case (WEAPON_DAGGER):
			send_to_char ("dagger\n\r", ch);
			break;
		case (WEAPON_STAFF):
			send_to_char ("staff/spear\n\r", ch);
			break;
		case (WEAPON_MACE):
			send_to_char ("mace/club\n\r", ch);
			break;
		case (WEAPON_AXE):
			send_to_char ("axe\n\r", ch);
			break;
		case (WEAPON_FLAIL):
			send_to_char ("flail\n\r", ch);
			break;
		case (WEAPON_POLEARM):
			send_to_char ("polearm\n\r", ch);
			break;
		case (WEAPON_SHORTBOW):
			send_to_char ("shortbow\n\r", ch);
			break;
		case (WEAPON_LONGBOW):
			send_to_char ("longbow\n\r", ch);
			break;
		case (WEAPON_ARROW):
			send_to_char ("arrow\n\r", ch);
			break;
		case (WEAPON_ARROWHEAD):
			send_to_char ("arrowhead\n\r", ch);
			break;
		case (WEAPON_DICE):
			send_to_char ("dice\n\r",ch);
			break;
		default:
			send_to_char ("unknown\n\r", ch);
			break;
		}
		sprintf (buf, "`jDamage is`` %dd%d `j(average`` %d`j)``\n\r",
			obj->value[1], obj->value[2],
			(1 + obj->value[2]) * obj->value[1] / 2);
		send_to_char (buf, ch);
		sprintf (buf, "`jDamage noun is`` %s.\n\r",
			attack_table[obj->value[3]].noun);
		send_to_char (buf, ch);
		if (obj->value[4])
		{			/* weapon flags */
			sprintf (buf, "`jWeapons flags:`` %s\n\r",
				weapon_bit_name (obj->value[4]));
			send_to_char (buf, ch);
		}
		break;
	case ITEM_ARMOR:
		sprintf (buf,
			"`jArmor Class is`` %d `jpierce,`` %d `jbash,`` %d `jslash, and`` %d `jvs. magic.``\n\r",
			obj->value[0], obj->value[1], obj->value[2], obj->value[3]);
		send_to_char (buf, ch);
		break;
	case ITEM_QUIVER:
		sprintf (buf, "`jCapacity:`` %d\n\r", obj->value[0]);
		send_to_char (buf, ch);
		break;
	case ITEM_POLE:
		sprintf (buf, "`jBaited:`` %d\n\r", obj->value[1]);
		send_to_char (buf, ch);
		break;
	case ITEM_PACK:
	case ITEM_CONTAINER:
		sprintf (buf,
			"`jCapacity:`` %d#  `jMaximum weight:`` %d#  `jflags:`` %s\n\r",
			obj->value[0], obj->value[3], cont_bit_name (obj->value[1]));
		send_to_char (buf, ch);
		if (obj->value[4] != 100)
		{
			sprintf (buf, "`jWeight multiplier:`` %d%%\n\r", obj->value[4]);
			send_to_char (buf, ch);
		}
		break;
	}
	if (obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL)
	{
		EXTRA_DESCR_DATA *ed;
		send_to_char ("`jExtra description keywords:`` '", ch);
		for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
		{
			send_to_char (ed->keyword, ch);
			if (ed->next != NULL)
				send_to_char (" ", ch);
		}
		if (obj->item_type != ITEM_WRITING_PAPER)
			for (ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next)
			{
				send_to_char (ed->keyword, ch);
				if (ed->next != NULL)
					send_to_char (" ", ch);
			}
			send_to_char ("'\n\r", ch);
	}
	for (paf = obj->affected; paf != NULL; paf = paf->next)
	{
		if (paf->where != TO_AFFECTS)
		{

			if (paf->where == TO_SKILL)
			{
				if(paf->location == 0)
					sprintf(buf,"`jAffects ``all abilities `jby ``%d",paf->modifier);
				else sprintf(buf,"`jAffects ``%s `jby ``%d", skill_table[paf->location].name,paf->modifier);
				send_to_char (buf,ch);
				send_to_char ("%.\n\r",ch);
			}
			else
			{

				sprintf (buf, "`jAffects`` %s `jby`` %d`j, level`` %d",
					affect_loc_name (paf->location), paf->modifier,
					paf->level);
				send_to_char (buf, ch);
				if (paf->duration > -1)
					sprintf (buf, "`j, ``%d `jhours.``\n\r", paf->duration);

				else
					sprintf (buf, ".\n\r");
				send_to_char (buf, ch);
			}
		}
		else
		{
			sprintf (buf, "`jAffects [``%s`j] to level`` %d `jfor``",
				flag_string (affect_flags, paf->bitvector), paf->level);
			send_to_char (buf, ch);
			if (paf->duration > -1)
				sprintf (buf, " %d `jhours.``\n\r", paf->duration);

			else
				strcpy (buf, "`jever``.\n\r");
		}
		if (paf->bitvector)
		{
			switch (paf->where)
			{
			case TO_AFFECTS:
				break;

			case TO_WEAPON:
				sprintf (buf, "`jAdds`` %s `jweapon flags.``\n",
					weapon_bit_name (paf->bitvector));
				break;
			case TO_OBJECT2:
			case TO_OBJECT:
				sprintf (buf, "`jAdds`` %s `jobject flag.``\n",
					extra_bit_name_old (paf->bitvector));
				break;
			case TO_IMMUNE:
				sprintf (buf, "`jAdds immunity to`` %s.\n",
					imm_bit_name (paf->bitvector));
				break;
			case TO_RESIST:
				sprintf (buf, "`jAdds resistance to`` %s.\n\r",
					imm_bit_name (paf->bitvector));
				break;
			case TO_VULN:
				sprintf (buf, "`jAdds vulnerability to`` %s.\n\r",
					imm_bit_name (paf->bitvector));
				break;
			default:
				sprintf (buf, "`jUnknown bit`` %d: %d\n\r", paf->where,
					paf->bitvector);
				break;
			}
			send_to_char (buf, ch);
		}
	}
	if (!obj->enchanted)
		for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
		{
			if (paf->location == APPLY_SPELL_AFFECT)
				continue;  //Iblis 4/04/04 needed to stop double affects since permaffects are ALSO actually
			//set on actual objects (in order to function properly)
			if (paf->where == TO_SKILL)
			{
				if(paf->location == 0)
					sprintf(buf,"`jAffects ``all abilities `jby ``%d",paf->modifier);
				else sprintf(buf,"`jAffects ``%s `jby ``%d", skill_table[paf->location].name,paf->modifier);
				send_to_char (buf,ch);
				send_to_char ("%.\n\r",ch);
			}
			else
			{
				sprintf (buf, "`jAffects`` %s `jby`` %d, `jlevel`` %d.\n\r",
					affect_loc_name (paf->location), paf->modifier, paf->level);
				send_to_char (buf, ch);
				if (paf->bitvector)
				{
					switch (paf->where)
					{
					case TO_AFFECTS:
						sprintf (buf, "`jAdds`` %s `jaffect.``\n",
							affect_bit_name (paf->bitvector));
						break;
					case TO_OBJECT2:
					case TO_OBJECT:
						sprintf (buf, "`jAdds`` %s `jobject flag.``\n",
							extra_bit_name_old (paf->bitvector));
						break;
					case TO_IMMUNE:
						sprintf (buf, "`jAdds immunity to`` %s.\n",
							imm_bit_name (paf->bitvector));
						break;
					case TO_RESIST:
						sprintf (buf, "`jAdds resistance to`` %s.\n\r",
							imm_bit_name (paf->bitvector));
						break;
					case TO_VULN:
						sprintf (buf, "`jAdds vulnerability to`` %s.\n\r",
							imm_bit_name (paf->bitvector));
						break;
					default:
						sprintf (buf, "`jUnknown bit`` %d: %d\n\r", paf->where,
							paf->bitvector);
						break;
					}
					send_to_char (buf, ch);
				}
			}
		}
		return;
}

void do_mstat (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	CHAR_DATA *victim;
	argument = one_argument (argument, arg);
	argument = one_argument (argument, arg2);
	if (arg[0] == '\0')
	{
		send_to_char ("Stat whom?\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (!str_cmp (arg2, "skills"))
	{
		sprintf (buf, "%s's skills:\n\r", victim->name);
		send_to_char (buf, ch);
		do_skillstat (ch, victim);
		return;
	}
	if (!str_cmp (arg2, "spells"))
	{
		sprintf (buf, "%s's spells:\n\r", victim->name);
		send_to_char (buf, ch);
		do_spellstat (ch, victim);
		return;
	}
	if (!str_cmp (arg2, "prayers"))
	{
		sprintf (buf, "%s's prayers:\n\r", victim->name);
		send_to_char (buf, ch);
		do_prayerstat (ch, victim);
		return;
	}
	if (!str_cmp (arg2, "chants"))
	{
		sprintf (buf, "%s's chants:\n\r", victim->name);
		send_to_char (buf, ch);
		do_chantstat (ch, victim);
		return;
	}
	if (!str_cmp (arg2, "songs"))
	{
		sprintf (buf, "%s's songs:\n\r", victim->name);
		send_to_char (buf, ch);
		do_songstat (ch, victim);
		return;
	}
	if (!str_cmp (arg2, "groups"))
	{
		sprintf (buf, "%s's groups:\n\r", victim->name);
		send_to_char (buf, ch);
		do_groupstat (ch, victim);
		return;
	}
	if (!str_cmp (arg2, "sword"))
	{
		if (!IS_CLASS (victim, PC_CLASS_REAVER))
		{
			send_to_char ("You can't stat a sword on a non-Reaver!\r\n", ch);
			return;
		}
		sprintf (buf, "%s's sword status:\n\r", victim->name);
		send_to_char (buf, ch);
		do_swordstat (ch, victim);
		return;
	}
	if (!str_cmp (arg2, "affects"))
	{
		sprintf (buf, "%s's affects:\n\r", victim->name);
		send_to_char (buf, ch);
		do_affectstat (ch, victim);
		return;
	}

	if (victim->Class2 == -1)
		victim->Class2 = victim->Class;
	if (victim->Class3 == -1)
		victim->Class3 = victim->Class;
	sprintf (buf, "`jName:`` %s\n\r", victim->name);
	send_to_char (buf, ch);
	if (victim->sex < 0)
		victim->sex = number_range (1, 2);
	sprintf (buf,
		"`jVnum:`` %d  `jFormat:`` %s  `jRace:`` %s  `jGroup:`` %d `jSex:``  %s `jRoom:`` %d\n\r",
		IS_NPC (victim) ? victim->pIndexData->vnum : 0, "new",
		race_table[victim->race].name,
		IS_NPC (victim) ? victim->group : 0,
		sex_table[victim->sex].name,
		victim->in_room == NULL ? 0 : victim->in_room->vnum);
	send_to_char (buf, ch);
	if (IS_NPC (victim))
	{
		sprintf (buf, "`jCount:`` %d  `jKilled:`` %d\n\r",
			victim->pIndexData->count, victim->pIndexData->killed);
		send_to_char (buf, ch);
	}
	sprintf (buf,
		"`jStr:`` %d(%d)  `jInt:`` %d(%d)  `jWis:`` %d(%d)  `jDex:`` %d(%d) `jCon:`` %d(%d) `jCha:`` %d(%d)\n\r",
		victim->perm_stat[STAT_STR], get_curr_stat (victim,
		STAT_STR),
		victim->perm_stat[STAT_INT], get_curr_stat (victim,
		STAT_INT),
		victim->perm_stat[STAT_WIS], get_curr_stat (victim,
		STAT_WIS),
		victim->perm_stat[STAT_DEX], get_curr_stat (victim,
		STAT_DEX),
		victim->perm_stat[STAT_CON], get_curr_stat (victim,
		STAT_CON),
		victim->perm_stat[STAT_CHA], get_curr_stat (victim, STAT_CHA));
	send_to_char (buf, ch);
	sprintf (buf,
		"`jHp:`` %d/%d  `jMana:`` %d/%d  `jMove:`` %d/%d  `jTrains:`` %d\n\r",
		victim->hit, victim->max_hit, victim->mana,
		victim->max_mana, victim->move, victim->max_move,
		IS_NPC (ch) ? 0 : victim->train);
	send_to_char (buf, ch);
	if (!IS_NPC (victim))
	{
		sprintf (buf,
			"`jLv:`` %d  `jClass:`` %s`a/``%s`a/``%s  `jAlign:`` %d  `jGold:`` %ld `jSilver:`` %ld `jExp:`` %lu\n\r",
			victim->level,
			IS_NPC (victim) ? "mobile" : Class_table[victim->
		Class].name,
			IS_NPC (victim) ? "mobile" : Class_table[victim->
			Class2].name,
			IS_NPC (victim) ? "mobile" : Class_table[victim->
			Class3].name,
			victim->alignment, victim->gold, victim->silver, victim->exp);
		if (IS_SET (victim->comm, COMM_NOCHANNELS) && ch->pcdata)
		{
			send_to_char(buf,ch);
			if (victim->pcdata->nochan_ticks >= 1)
				sprintf(buf,"`jPlayer NOCHANNED for ``%ld `jticks.\n\r",victim->pcdata->nochan_ticks);
			else sprintf(buf,"`jPlayer NOCHANNED permanently.\n\r");
		}


		if (IS_SET (victim->act, PLR_WANTED))
			strcat (buf, "`jPlayer WANTED ``");
		if (IS_SET (victim->act, PLR_JAILED))
			strcat (buf, "`jPlayer JAILED ``");
		if ((IS_SET (victim->act, PLR_WANTED)
			|| IS_SET (victim->act, PLR_JAILED))
			&& victim->want_jail_time != 0)
		{
			if (victim->want_jail_time == 999999)
			{
				strcat (buf, "for infinite more ticks.\n\r");
			}
			else
			{
				sprintf (buf + strlen (buf), "for %d more ticks.\n\r",
					victim->want_jail_time);
			}
		}
		if ((victim->pcdata->hero) || (victim->pcdata->loner) ||
			(victim->clan != CLAN_BOGUS))
		{
			strcat (buf, "`jPlayer ``");
			if (victim->pcdata->hero)
				strcat (buf, "is HERO ");
			if (victim->clan != CLAN_BOGUS)
			{
				sprintf (buf2, "`jin clan`` %s`` %s``\n\r",
					get_clan_name_ch (victim),
					get_clan_symbol_ch (victim));
				strcat (buf, buf2);
			}
			else
			{
				if (victim->pcdata->loner)
				{
					strcat (buf, "is LONER\n\r");
				}
				else
				{
					strcat (buf, "\n\r");
				}
			}
		}
	}
	else
		sprintf (buf,
		"`jLv:`` %d  `jClass:`` %s  `jAlign:`` %d  `jGold:`` %ld  `jSilver:`` %ld  `jExp:`` %lu\n\r",
		victim->level,
		IS_NPC (victim) ? "mobile" : Class_table[victim->Class].
		name, victim->alignment, victim->gold, victim->silver,
		victim->exp);
	send_to_char (buf, ch);
	if (!IS_NPC (victim))
	{
		AGGRESSOR_LIST *a;
		sprintf (buf, "`jTotal Experience:`` %lu `j%s``\n\r",
			victim->pcdata->totalxp,
			(victim->pcdata->
			new_style) ? "Converted Character" : "Unconverted Character");
		send_to_char (buf, ch);
		sprintf (buf,
			"`jGained Hp:`` %d `jGained Mana:`` %d `jGained Move:`` %d``\r\n",
			victim->pcdata->hp_gained, victim->pcdata->mana_gained,
			victim->pcdata->move_gained);
		send_to_char (buf, ch);
		if (victim->pcdata->avatar_type >= 3)
		{
			sprintf (buf, "`jOld Race: ``%s `jAvatar Type:`` %s `jRp Points:`` %ld `jNh Points:`` %ld `jQuest Points:`` %ld\n\r",
				(victim->pcdata->old_race != -1)?race_table[victim->pcdata->old_race].name:"N/A", 
				(victim->pcdata->avatar_type == 3)?"NoPkRemort":"PkRemort",victim->pcdata->rppoints,victim->pcdata->nhpoints,victim->pcdata->qpoints);
		}
		else
		{
			sprintf (buf, "`jOld Race: ``%s `jAvatar Type:`` %s `jRp Points:`` %ld `jNh Points:`` %ld `jQuest Points:`` %ld\n\r",
				(victim->pcdata->old_race != -1)?race_table[victim->pcdata->old_race].name:"N/A", 
				(victim->pcdata->avatar_type)?((victim->pcdata->avatar_type == 1)?"NoPk":"Pk"):"Old",
				victim->pcdata->rppoints,victim->pcdata->nhpoints,victim->pcdata->qpoints);
		}
		send_to_char (buf, ch);
		sprintf (buf, "`jCan Retaliate Against -> ``");
		send_to_char (buf, ch);
		for (a = victim->pcdata->agg_list; a != NULL; a = a->next)
		{
			sprintf (buf, "%s `jfor ``%d `jticks | ", a->ch->name,
				a->ticks_left);
			send_to_char (buf, ch);
		}

		//strcat(buf," ");
		send_to_char ("\r\n", ch);
	}
	if (IS_NPC (victim))
	{
		sprintf (buf, "`jMood:`` %d\n\r", victim->mood);
		send_to_char (buf, ch);
	}
	if (!IS_NPC (victim) && victim->pcdata->falcon_wait != 0)
	{
		sprintf (buf,
			"`jFalconry - ``%d `jticks, object ``%s `jto ``%s\r\n",
			ch->pcdata->falcon_wait,
			(ch->pcdata->falcon_object ? ch->pcdata->falcon_object->
short_descr : "Obj Delivered"),
			  (ch->pcdata->falcon_recipient ? ch->pcdata->
			  falcon_recipient->name : "Char Left"));
		send_to_char (buf, ch);
	}
	// Added June 22. 2000 By Morgan
	// reflects players email address in stat.
	// For security reasons, changed so that lower level imms can't
	// gain access to higher level imms email addresses.
	if ((!IS_NPC (victim)) && (ch->level >= victim->level))
	{
		sprintf (buf, "`jEmail address:`` %s\n\r", victim->pcdata->email_addr);
		send_to_char (buf, ch);
	}
	//Iblis 5/13/03 - Added below to stop the stating of linkdead players from crashing the mud
	if (!IS_NPC (victim) && victim->desc == NULL)
		send_to_char ("`jThis player is currently linkdead.\n\r", ch);

	else if ((!IS_NPC (victim)) && (ch->level >= 99)
		&& (victim->desc->connected == CON_PLAYING))
	{
		sprintf (buf, "`jReal Host:`` %s\n\r", victim->desc->realhost);
		send_to_char (buf, ch);
	}
	if (!IS_NPC (victim))
	{
		sprintf (buf, "`jLast room saved:`` %d\n\r",
			victim->pcdata->last_saved_room);
		send_to_char (buf, ch);
	}
	sprintf (buf,
		"`jArmor: pierce:`` %d  `jbash:`` %d  `jslash:`` %d  `jmagic:`` %d\n\r",
		GET_AC (victim, AC_PIERCE), GET_AC (victim, AC_BASH),
		GET_AC (victim, AC_SLASH), GET_AC (victim, AC_EXOTIC));
	send_to_char (buf, ch);
	sprintf (buf,
		"`jHit:`` %d  `jDam:`` %d  `jSaves:`` %d  `jSize:`` %s  `jPosition:`` %s  `jWimpy:`` %d\n\r",
		get_hitroll (victim), get_damroll (victim),
		victim->saving_throw, size_table[victim->size].name,
		position_table[victim->position].name, victim->wimpy);
	send_to_char (buf, ch);
	if (IS_NPC (victim))
	{
		sprintf (buf, "`jDamage:`` %dd%d  `jMessage:``  %s\n\r",
			victim->damage[DICE_NUMBER], victim->damage[DICE_TYPE],
			attack_table[victim->dam_type].noun);
		send_to_char (buf, ch);
	}
	sprintf (buf, "`jFighting:`` %s",
		victim->fighting ? victim->fighting->name : "(none)");
	if (IS_NPC (victim))
		strcat (buf, "\n\r");

	else
		sprintf (buf + strlen (buf), "   `jWanted:`` %d `jtimes``\n\r",
		victim->times_wanted);
	send_to_char (buf, ch);
	if (!IS_NPC (victim))
	{
		sprintf (buf,
			"`jThirst:`` %d  `jHunger:`` %d  `jFull:`` %d  `jDrunk:`` %d  `jPills:`` %d\n\r",
			victim->pcdata->condition[COND_THIRST],
			victim->pcdata->condition[COND_HUNGER],
			victim->pcdata->condition[COND_FULL],
			victim->pcdata->condition[COND_DRUNK],
			victim->pcdata->condition[COND_PILLS]);
		send_to_char (buf, ch);
	}
	sprintf (buf, "`jCarry number:`` %d  `jCarry weight:`` %ld\n\r",
		victim->carry_number, get_carry_weight (victim) / 10);
	send_to_char (buf, ch);
	if (!IS_NPC (victim))
	{
		sprintf (buf,
			"`jAge:`` %d  `jPlayed:`` %d  `jLast Level:`` %d  `jTimer:`` %d\n\r",
			get_age (victim),
			(int) (victim->played + current_time -
			victim->logon) / 3600,
			victim->pcdata->last_level, victim->timer);
		send_to_char (buf, ch);
	}
	sprintf (buf, "`jAct:`` %s %s\n\r", act_bit_name (victim->act),
		act2_bit_name (victim->act2));
	send_to_char (buf, ch);
	if (victim->comm)
	{
		sprintf (buf, "`jComm:`` %s %s\n\r", comm_bit_name (victim->comm), comm2_bit_name(victim->comm2));
		send_to_char (buf, ch);
	}
	if (IS_NPC (victim) && victim->off_flags)
	{
		sprintf (buf, "`jOffense:`` %s\n\r", off_bit_name (victim->off_flags));
		send_to_char (buf, ch);
	}
	if (victim->imm_flags)
	{
		sprintf (buf, "`jImmune:`` %s\n\r", imm_bit_name (victim->imm_flags));
		send_to_char (buf, ch);
	}
	if (victim->res_flags)
	{
		sprintf (buf, "`jResist:`` %s\n\r", imm_bit_name (victim->res_flags));
		send_to_char (buf, ch);
	}
	if (victim->vuln_flags)
	{
		sprintf (buf, "`jVulnerable:`` %s\n\r",
			imm_bit_name (victim->vuln_flags));
		send_to_char (buf, ch);
	}
	if (victim->sink_timer > -1)
	{
		sprintf (buf, "`jSink Timer:`` %d\n\r", victim->sink_timer);
		send_to_char (buf, ch);
	}
	sprintf (buf, "`jForm:`` %s\n\r`jParts:`` %s\n\r",
		form_bit_name (victim->form), part_bit_name (victim->parts));
	send_to_char (buf, ch);
	if (victim->affected_by)
	{
		sprintf (buf, "`jAffected by`` %s\n\r",
			affect_bit_name (victim->affected_by));
		send_to_char (buf, ch);
	}
	if (IS_NPC(victim) && victim->number_of_attacks != -1)
	{
		sprintf (buf, "`jNumber of Attacks:`` %d\n\r",victim->number_of_attacks);
		send_to_char(buf,ch);
	}
	if (IS_NPC(victim) && victim->walkto > 0)
	{
		sprintf (buf, "`jWalk to:`` %d\n\r",victim->walkto);
		send_to_char(buf,ch);
	}
	sprintf (buf, "`jMaster:`` %s  `jLeader:`` %s  `jPet:`` %s\n\r",
		victim->master ? victim->master->name : "(none)",
		victim->leader ? victim->leader->name : "(none)",
		victim->pet ? victim->pet->name : "(none)");
	send_to_char (buf, ch);
	if (!IS_NPC (victim))
	{				/* OLC */
		sprintf (buf, "`jSecurity:`` %d.\n\r", victim->pcdata->security);
		send_to_char (buf, ch);
	}
	if (!IS_NPC (victim))
	{				/* OLC */
		sprintf (buf, "`jBuildAll:`` %d.\n\r", victim->pcdata->buildall);
		send_to_char (buf, ch);
	}
	sprintf (buf,
		"`jShort description:`` %s\n\r`jLong  description:`` %s",
		victim->short_descr,
		victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r");
	send_to_char (buf, ch);
	if (IS_NPC (victim) && victim->spec_fun != 0)
	{
		sprintf (buf, "`jMobile has special procedure`` %s.\n\r",
			spec_name (victim->spec_fun));
		send_to_char (buf, ch);
	}
	for (paf = victim->affected; paf != NULL; paf = paf->next)
	{
		if (!paf->composition)
			sprintf (buf,
			"`jSpell:`` '%s' `jmodifies`` %s `jby`` %d `jfor`` %d `jhours with bits`` %s, `jlevel`` %d.\n\r",
			skill_table[(int) paf->type].name,
			affect_loc_name (paf->location), paf->modifier,
			paf->duration, affect_bit_name (paf->bitvector), paf->level);

		else
		{
			if (!IS_NPC (victim))
			{
				sprintf (buf,
					"`jSong:`` '%s' `jmodifies`` %s `jby`` %d `jfor`` %d `jhours with bits`` %s, `jlevel`` %d.\n\r",
					paf->comp_name, affect_loc_name (paf->location),
					paf->modifier, paf->duration,
					affect_bit_name (paf->bitvector), paf->level);
				send_to_char (buf, ch);
			}
		}
	}
	return;
}


/* ofind and mfind replaced with vnum, vnum skill also added */
void do_vnum (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char *string;
	string = one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Syntax:\n\r", ch);
		send_to_char ("  vnum obj <name>\n\r", ch);
		send_to_char ("  vnum mob <name>\n\r", ch);
		send_to_char ("  vnum skill <skill or spell>\n\r", ch);
		return;
	}
	if (!str_cmp (arg, "obj"))
	{
		do_ofind (ch, string);
		return;
	}
	if (!str_cmp (arg, "mob") || !str_cmp (arg, "char"))
	{
		do_mfind (ch, string);
		return;
	}
	if (!str_cmp (arg, "skill") || !str_cmp (arg, "spell"))
	{
		do_slookup (ch, string);
		return;
	}

	/* do both */
	do_mfind (ch, argument);
	do_ofind (ch, argument);
}

void do_mfind (CHAR_DATA * ch, char *argument)
{
	extern int top_mob_index;
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	int vnum;
	int nMatch;
	bool fAll;
	bool found;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Find whom?\n\r", ch);
		return;
	}
	fAll = FALSE;			/* !str_cmp( arg, "all" ); */
	found = FALSE;
	nMatch = 0;

	/*
	* Yeah, so iterating over all vnum's takes 10,000 loops.
	* Get_mob_index is fast, and I don't feel like threading another link.
	* Do you?
	* -- Furey
	*/
	for (vnum = 0; nMatch < top_mob_index; vnum++)
	{
		if (vnum > 40000)
		{
			char log_buf[MAX_STRING_LENGTH];
			sprintf (log_buf,
				"Impossibly large vnum, top_mob_index(%d), nMatch(%d), vnum(%d).",
				top_mob_index, nMatch, vnum);
			log_string (log_buf);
			return;
		}
		if ((pMobIndex = get_mob_index (vnum)) != NULL)
		{
			nMatch++;
			if (fAll || is_name (argument, pMobIndex->player_name))
			{
				found = TRUE;
				sprintf (buf, "[%5d] %s\n\r", pMobIndex->vnum,
					pMobIndex->short_descr);
				send_to_char (buf, ch);
			}
		}
	}
	if (!found)
		send_to_char ("No mobiles by that name.\n\r", ch);
	return;
}

void do_ofind (CHAR_DATA * ch, char *argument)
{
	extern int top_obj_index;
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	int vnum;
	int nMatch;
	bool fAll;
	bool found;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Find what?\n\r", ch);
		return;
	}
	fAll = FALSE;			/* !str_cmp( arg, "all" ); */
	found = FALSE;
	nMatch = 0;
	for (vnum = 0; nMatch < top_obj_index; vnum++)
	{
		if (vnum > 40000)
		{
			char log_buf[MAX_STRING_LENGTH];
			sprintf (log_buf,
				"Impossibly large vnum, top_obj_index(%d), nMatch(%d), vnum(%d).",
				top_obj_index, nMatch, vnum);
			log_string (log_buf);
			return;
		}
		if ((pObjIndex = get_obj_index (vnum)) != NULL)
		{
			nMatch++;
			if (fAll || is_name (argument, pObjIndex->name))
			{
				found = TRUE;
				sprintf (buf, "[%5d] %s\n\r", pObjIndex->vnum,
					pObjIndex->short_descr);
				send_to_char (buf, ch);
			}
		}
	}
	if (!found)
		send_to_char ("No objects by that name.\n\r", ch);
	return;
}

void do_owhere (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	BUFFER *buffer;
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	bool found;
	int number = 0, max_found;
	found = FALSE;
	number = 0;
	max_found = 200;
	buffer = new_buf ();
	if (argument[0] == '\0')
	{
		send_to_char ("Find what?\n\r", ch);
		free_buf (buffer);
		return;
	}
	// run through all of the objects in the world
	for (obj = object_list; obj != NULL; obj = obj->next)
	{

		// if the object can not be seen, or the name does not match
		// or the char is too low a level, then just skip this object
		if (!can_see_obj (ch, obj) || !is_name (argument, obj->name)
			|| (ch->level < obj->level))
			continue;
		found = TRUE;
		number++;

		// retrace this objects containment hierarchy
		in_obj = obj;
		while (in_obj->in_obj != NULL)
		{
			in_obj = in_obj->in_obj;
		}
		if ((in_obj->carried_by != NULL))
		{

			// the object is carried by someone
			if (can_see (ch, in_obj->carried_by))
			{

				// We can see them
				if (in_obj->carried_by->in_room != NULL)
				{
					sprintf (buf, "%3d) %s is carried by %s [Room %d]\n\r",
						number, obj->short_descr,
						PERS (in_obj->carried_by, ch),
						in_obj->carried_by->in_room->vnum);
				}
				else
				{
					sprintf (buf,
						"%3d) %s is carried by %s [Room <NULL>]\n\r",
						number, obj->short_descr,
						PERS (in_obj->carried_by, ch));
				}
			}
			else
			{

				// we can't see the someone
				if (in_obj->carried_by->in_room != NULL)
				{
					sprintf (buf,
						"%3d) %s is carried by someone [Room %d]\n\r",
						number, obj->short_descr,
						in_obj->carried_by->in_room->vnum);
				}
				else
				{
					sprintf (buf,
						"%3d) %s is carried by someone [Room <NULL>]\n\r",
						number, obj->short_descr);
				}
			}
		}
		else
		{

			// the object is not carried by a character, maybe it's in a room
			if (in_obj->in_room != NULL)
			{
				if (can_see_room (ch, in_obj->in_room))
				{
					sprintf (buf, "%3d) %s is in %s [Room %d]\n\r",
						number, obj->short_descr,
						in_obj->in_room->name, in_obj->in_room->vnum);
				}
				else
				{

					// it's in a room we just can't see the room
					sprintf (buf,
						"%3d) %s is in a room hidden from your view.\n\r",
						number, obj->short_descr);
				}
			}
			else
			{

				// Huh? It's not in any room and it's not carried by anyone?
				sprintf (buf, "%3d) %s is FUBAR\n\r", number, obj->short_descr);
			}
		}
		add_buf (buffer, buf);
		if (number >= max_found)
			break;
	}				// end of object_list loop
	if (!found)
	{
		send_to_char ("Nothing like that in heaven or earth.\n\r", ch);
	}
	else
	{
		page_to_char (buf_string (buffer), ch);
	}
	free_buf (buffer);
}


// Akamai 6/24/99 - Starting work on memory leak - looking at
// how to manage all of the 'FUBAR' objects with some garbage
// collection
void do_garbage_collection (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	char obj_name[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	OBJ_DATA *next_obj;
	bool count_only = FALSE;
	bool find_all = FALSE;
	bool in_something = FALSE;
	int number = 0;
	if (argument[0] == '\0')
	{
		send_to_char ("gc <object> [count]\n\r", ch);
		send_to_char
			("   count only counts objects needing garbage collection\r\n", ch);
		send_to_char ("   object \"all\" will count/free all objects\r\n", ch);
		return;
	}
	// set the name of the object to find
	argument = one_argument (argument, obj_name);
	if (!str_cmp (obj_name, "all"))
	{
		find_all = TRUE;
	}
	// check to see if this is just a counting operation
	if (argument[0] != '\0')
	{
		if (!str_cmp (argument, "count"))
		{
			count_only = TRUE;
		}
		else
		{
			do_garbage_collection (ch, "");
			return;
		}
	}
	// run through all of the objects in the world
	obj = object_list;
	if (object_list != NULL)
	{
		next_obj = obj->next;
	}
	else
	{
		next_obj = NULL;
	}
	while (obj != NULL)
	{

		// if we are finding all - just skip this
		if (find_all || is_name (obj_name, obj->name))
		{

			// retrace this object containment hierarchy
			in_obj = obj;
			in_something = FALSE;
			while (in_obj->in_obj != NULL)
			{
				in_obj = in_obj->in_obj;
				in_something = TRUE;
			}

			// ok, it's not being carried by anyone
			if ((in_obj->carried_by == NULL))
			{

				// it's not in any room either
				if (in_obj->in_room == NULL)
				{

					// these are the 'FUBAR' things in the 'owhere' command
					number++;
					if (!count_only)
					{
						if (in_something)
						{
							sprintf (buf, "GC[%d](i): %s``", number,
								obj->short_descr);
						}
						else
						{
							sprintf (buf, "GC[%d]: %s``", number,
								obj->short_descr);
						}
						log_string (buf);

						// this does the free/recycle
						extract_obj (obj);
						sprintf (buf, "%s\n\r", buf);
						send_to_char (buf, ch);
					}
				}
			}
		}			// end of test for object's name, or count all

		// move both pointers forward
		obj = next_obj;
		if (next_obj != NULL)
		{
			next_obj = next_obj->next;
		}
	}				// end of while (obj != NULL) loop

	// Ok, now a little feed back on the command
	if (number == 0)
	{
		if (!find_all)
		{
			sprintf (buf,
				"There were no \"%s\" found needing garbage collecting.\n\r",
				obj_name);
			send_to_char (buf, ch);
		}
		else
		{
			sprintf (buf,
				"There were no objects found needing garbage collecting.\n\r");
			send_to_char (buf, ch);
		}
	}
	else
	{
		if (!count_only)
		{
			if (!find_all)
			{
				sprintf (buf, "There were %d %s garbage collected.\n\r",
					number, obj_name);
				send_to_char (buf, ch);
			}
			else
			{
				sprintf (buf,
					"There were %d objects garbage collected.\n\r",
					number);
				send_to_char (buf, ch);
			}
		}
		else
		{
			if (!find_all)
			{
				sprintf (buf, "There are %d %s in need of collection.\n\r",
					number, obj_name);
				send_to_char (buf, ch);
			}
			else
			{
				sprintf (buf,
					"There are %d objects in need of collection.\n\r",
					number);
				send_to_char (buf, ch);
			}
		}
	}				// end if(number == 0)
}
void do_mwhere (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	BUFFER *buffer;
	CHAR_DATA *victim;
	bool found;
	int count = 0;
	if (argument[0] == '\0')
	{
		DESCRIPTOR_DATA *d;

		/* show characters logged */
		buffer = new_buf ();
		for (d = descriptor_list; d != NULL; d = d->next)
		{
			if (d->character != NULL && d->connected == CON_PLAYING
				&& d->character->in_room != NULL
				&& can_see (ch, d->character) && can_see_room (ch,
				d->
				character->
				in_room))
			{
				if (d->original != NULL && !can_see (ch, d->original))
					continue;
				victim = d->character;
				count++;
				if (d->original != NULL)
					sprintf (buf,
					"%3d) %s (in the body of %s) is in %s [%d]\n\r",
					count, d->original->name,
					victim->short_descr, victim->in_room->name,
					victim->in_room->vnum);

				else
					sprintf (buf, "%3d) %s is in %s [%d]\n\r", count,
					victim->name, victim->in_room->name,
					victim->in_room->vnum);
				add_buf (buffer, buf);
			}
		}
		page_to_char (buf_string (buffer), ch);
		free_buf (buffer);
		return;
	}
	found = FALSE;
	buffer = new_buf ();
	for (victim = char_list; victim != NULL; victim = victim->next)
	{
		if (victim->in_room != NULL
			&& is_name (argument, victim->name) && can_see (ch, victim)
			&& can_see_room (ch, victim->in_room))
		{
			found = TRUE;
			count++;
			sprintf (buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
				IS_NPC (victim) ? victim->pIndexData->vnum : 0,
				IS_NPC (victim) ? victim->short_descr : victim->name,
				victim->in_room->vnum, victim->in_room->name);
			add_buf (buffer, buf);
		}
	}
	if (!found)
		act ("You didn't find any $T.", ch, NULL, argument, TO_CHAR);

	else
		page_to_char (buf_string (buffer), ch);
	free_buf (buffer);
	return;
}

void do_reboo (CHAR_DATA * ch, char *argument)
{
	send_to_char ("If you want to REBOOT, spell it out.\n\r", ch);
	return;
}

void do_reboot (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	argument = one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Usage: reboot now\n\r", ch);
		send_to_char ("Usage: reboot <minutes to reboot>\n\r", ch);
		send_to_char ("Usage: reboot cancel\n\r", ch);
		send_to_char ("Usage: reboot status\n\r", ch);
		return;
	}
	if (is_name (arg, "cancel"))
	{
		reboot_counter = -1;
		send_to_char ("Reboot cancelled.\n\r", ch);
		return;
	}
	if (is_name (arg, "now"))
	{
		reboot_rom ();
		return;
	}
	if (is_name (arg, "status"))
	{
		if (reboot_counter == -1)
			sprintf (buf, "Automatic rebooting is inactive.\n\r");

		else
			sprintf (buf, "Reboot in %i minutes.\n\r", reboot_counter);
		send_to_char (buf, ch);
		return;
	}
	if (is_number (arg))
	{
		reboot_counter = atoi (arg);
		sprintf (buf, "Exodus will reboot in %i minutes.\n\r", reboot_counter);
		send_to_char (buf, ch);
		return;
	}
	do_reboot (ch, "");
}

void do_shutdown (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	argument = one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Usage: shutdown now\n\r", ch);
		send_to_char ("Usage: shutdown <minutes to reboot>\n\r", ch);
		send_to_char ("Usage: shutdown cancel\n\r", ch);
		send_to_char ("Usage: shutdown status\n\r", ch);
		return;
	}
	if (is_name (arg, "cancel"))
	{
		shutdown_counter = -1;
		send_to_char ("Shutdown cancelled.\n\r", ch);
		return;
	}
	if (is_name (arg, "now"))
	{
		shutdown_rom ();
		return;
	}
	if (is_name (arg, "status"))
	{
		if (shutdown_counter == -1)
			sprintf (buf, "Automatic shutdown is inactive.\n\r");

		else
			sprintf (buf, "Shutdown in %i minutes.\n\r", shutdown_counter);
		send_to_char (buf, ch);
		return;
	}
	if (is_number (arg))
	{
		shutdown_counter = atoi (arg);
		sprintf (buf, "Exodus will shut down in %i minutes.\n\r",
			shutdown_counter);
		send_to_char (buf, ch);
		return;
	}
	do_shutdown (ch, "");
}

void reboot_rom (void)
{
	extern bool merc_down;
	DESCRIPTOR_DATA *d, *d_next;
	sprintf (log_buf, "Rebooting the mud.");
	log_string (log_buf);
	merc_down = TRUE;
	for (d = descriptor_list; d != NULL; d = d_next)
	{
		d_next = d->next;
		if (d->character != NULL)
			do_save (d->character, "");
		write_to_buffer (d, "Exodus is going down for reboot NOW!", 0);
		close_socket (d);
	}
	unlink ("ctransports.txt");
	return;
}

void shutdown_rom (void)
{
	DESCRIPTOR_DATA *d, *d_next;
	sprintf (log_buf, "Shutting the mud down.");
	log_string (log_buf);
	for (d = descriptor_list; d != NULL; d = d_next)
	{
		d_next = d->next;
		if (d->character != NULL)
			do_save (d->character, "");
		write_to_buffer (d, "Exodus is shutting down NOW!", 0);
		close_socket (d);
	}
	unlink ("ctransports.txt");
	shutdown_graceful ();
	return;
}

void do_shutdow (CHAR_DATA * ch, char *argument)
{
	send_to_char ("If you want to SHUTDOWN, spell it out.\n\r", ch);
	return;
}

void do_protect (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	if (argument[0] == '\0')
	{
		send_to_char ("Protect whom from snooping?\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, argument)) == NULL)
	{
		send_to_char ("You can't find them.\n\r", ch);
		return;
	}
	if (IS_SET (victim->comm, COMM_SNOOP_PROOF))
	{
		act_new ("$N is no longer snoop-proof.", ch, NULL, victim,
			TO_CHAR, POS_DEAD);
		send_to_char ("Your snoop-proofing was just removed.\n\r", victim);
		REMOVE_BIT (victim->comm, COMM_SNOOP_PROOF);
	}
	else
	{
		act_new ("$N is now snoop-proof.", ch, NULL, victim, TO_CHAR, POS_DEAD);
		send_to_char ("You are now immune to snooping.\n\r", victim);
		SET_BIT (victim->comm, COMM_SNOOP_PROOF);
	}
}
void do_snoop (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Snoop whom?\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (victim->desc == NULL)
	{
		send_to_char ("No descriptor to snoop.\n\r", ch);
		return;
	}
	if (victim == ch)
	{
		send_to_char ("Cancelling all snoops.\n\r", ch);
		wiznet ("$N stops being such a snoop.", ch, NULL, WIZ_SNOOPS,
			WIZ_SECURE, get_trust (ch));
		for (d = descriptor_list; d != NULL; d = d->next)
		{
			if (d->snoop_by == ch->desc)
				d->snoop_by = NULL;
		}
		return;
	}
	if (victim->desc->snoop_by != NULL)
	{
		send_to_char ("Busy already.\n\r", ch);
		return;
	}
	if (!is_room_owner (ch, victim->in_room)
		&& ch->in_room != victim->in_room
		&& room_is_private (victim->in_room) && !IS_TRUSTED (ch, OVERLORD))
	{
		send_to_char ("That character is in a private room.\n\r", ch);
		return;
	}
	if (get_trust (victim) >= get_trust (ch)
		|| IS_SET (victim->comm, COMM_SNOOP_PROOF))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (ch->desc != NULL)
	{
		for (d = ch->desc->snoop_by; d != NULL; d = d->snoop_by)
		{
			if (d->character == victim || d->original == victim)
			{
				send_to_char ("No snoop loops.\n\r", ch);
				return;
			}
		}
	}
	victim->desc->snoop_by = ch->desc;
	sprintf (buf, "$N starts snooping on %s",
		(IS_NPC (ch) ? victim->short_descr : victim->name));
	wiznet (buf, ch, NULL, WIZ_SNOOPS, WIZ_SECURE, get_trust (ch));
	send_to_char ("Ok.\n\r", ch);
	return;
}

void do_switch (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Switch into whom?\n\r", ch);
		return;
	}
	if (ch->desc == NULL)
		return;
	if (ch->desc->original != NULL)
	{
		send_to_char ("You are already switched.\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (victim == ch)
	{
		send_to_char ("Ok.\n\r", ch);
		return;
	}

	if (!IS_NPC (victim))
	{
		send_to_char ("Not into players.\n\r", ch);
		return;
	}
	if (IS_SET (victim->act, PLR_FREEZE))
	{
		send_to_char
			("You wouldn't be able to return, when switching into a frozen player!\n\r",
			ch);
		return;
	}
	if (!is_room_owner (ch, victim->in_room)
		&& ch->in_room != victim->in_room
		&& room_is_private (victim->in_room) && !IS_TRUSTED (ch, OVERLORD))
	{
		send_to_char ("That character is in a private room.\n\r", ch);
		return;
	}
	if (IS_SET(victim->in_room->area->area_flags,AREA_IMP_ONLY) && get_trust(ch) < MAX_LEVEL)
	{
		send_to_char("You failed.\n\r",ch);
		return;
	}
	if (victim->desc != NULL)
	{
		send_to_char ("Character in use.\n\r", ch);
		return;
	}
	sprintf (buf, "$N switches into %s",
		IS_NPC (victim) ? victim->short_descr : victim->name);
	wiznet (buf, ch, NULL, WIZ_SWITCHES, WIZ_SECURE, get_trust (ch));
	ch->desc->character = victim;
	ch->desc->original = ch;
	victim->desc = ch->desc;

	//  ch->switched = victim;
	ch->desc = NULL;

	/* change communications to match */
	if (ch->prompt != NULL)
		victim->prompt = str_dup (ch->prompt);
	victim->comm = ch->comm;
	victim->lines = ch->lines;
	send_to_char ("Ok.\n\r", victim);
	return;
}

void do_return (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	if (ch->desc == NULL)
		return;
	if (ch->desc->original == NULL)
	{
		send_to_char ("You aren't switched.\n\r", ch);
		return;
	}
	if (!IS_SET(ch->desc->original->act2,ACT_SWITCHED))
	{

		send_to_char
			("You return to your original body. Type replay to see any missed tells.\n\r",
			ch);
	}
	else REMOVE_BIT(ch->desc->original->act,ACT_SWITCHED);
	if (ch->prompt != NULL)
	{
		free_string (ch->prompt);
		ch->prompt = NULL;
	}
	sprintf (buf, "$N returns from %s.", ch->short_descr);
	wiznet (buf, ch->desc->original, 0, WIZ_SWITCHES, WIZ_SECURE,
		get_trust (ch));
	ch->desc->character = ch->desc->original;
	ch->desc->original = NULL;
	ch->desc->character->desc = ch->desc;
	ch->desc = NULL;

	//  ch->switched = NULL;
	return;
}


/* trust levels for load and clone */
bool obj_check (CHAR_DATA * ch, OBJ_DATA * obj)
{
	if (IS_TRUSTED (ch, GOD)
		|| (IS_TRUSTED (ch, GOD) && obj->level <= 20
		&& obj->cost <= 1000) || (IS_TRUSTED (ch, DEMIGOD)
		&& obj->level <= 10
		&& obj->cost <= 500)
		|| (IS_TRUSTED (ch, DEMIGOD) && obj->level <= 5
		&& obj->cost <= 250) || (IS_TRUSTED (ch, DEMIGOD)
		&& obj->level == 0 && obj->cost <= 100))
		return TRUE;

	else
		return FALSE;
}


/* for clone, to insure that cloning goes many levels deep */
void recursive_clone (CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * clone)
{
	OBJ_DATA *c_obj, *t_obj;
	for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
	{
		if (obj_check (ch, c_obj))
		{
			t_obj = create_object (c_obj->pIndexData, 0);
			clone_object (c_obj, t_obj);
			obj_to_obj (t_obj, clone);
			recursive_clone (ch, c_obj, t_obj);
		}
	}
}
void do_clist (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	COMP_DATA *comp;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("View whose compose list?\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Mobiles can't compose.\n\r", ch);
		return;
	}
	sprintf (buf, "Compose list for %s:\n\r", victim->name);
	send_to_char (buf, ch);
	for (comp = victim->compositions; comp; comp = comp->next)
	{
		sprintf (buf, "Name: %s\tType: %s\t", comp->name,
			comp_string (comp->type));
		sprintf (buf + strlen (buf), "Title: %s\n\r", comp->title);
		send_to_char (buf, ch);
	}
}


/* command that is similar to load */
void do_clone (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char *rest;
	CHAR_DATA *mob;
	OBJ_DATA *obj;
	rest = one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Clone what?\n\r", ch);
		return;
	}
	if (!str_prefix (arg, "object"))
	{
		mob = NULL;
		obj = get_obj_here (ch, rest);
		if (obj == NULL)
		{
			send_to_char ("You don't see that here.\n\r", ch);
			return;
		}
	}
	else if (!str_prefix (arg, "mobile") || !str_prefix (arg, "character"))
	{
		obj = NULL;
		mob = get_char_room (ch, rest);
		if (mob == NULL)
		{
			send_to_char ("You don't see that here.\n\r", ch);
			return;
		}
	}
	else
		/* find both */
	{
		mob = get_char_room (ch, argument);
		obj = get_obj_here (ch, argument);
		if (mob == NULL && obj == NULL)
		{
			send_to_char ("You don't see that here.\n\r", ch);
			return;
		}
	}

	/* clone an object */
	if (obj != NULL)
	{
		OBJ_DATA *clone;
		if (obj->pIndexData->item_type == ITEM_CTRANSPORT
			|| obj->pIndexData->item_type == ITEM_ELEVATOR)
		{
			send_to_char
				("You can't have more than one of this object in the world.\n\r",
				ch);
			return;
		}
		if (!obj_check (ch, obj))
		{
			send_to_char
				("Your powers are not great enough for such a task.\n\r", ch);
			return;
		}
		clone = create_object (obj->pIndexData, 0);
		clone_object (obj, clone);
		if (obj->carried_by == NULL || (IS_NPC(ch) && ch->pIndexData->vnum != MOB_VNUM_FAKIE))
			obj_to_room (clone, ch->in_room);
		else
			obj_to_char (clone, ch);
		recursive_clone (ch, obj, clone);
		act ("$n has created $p.", ch, clone, NULL, TO_ROOM);
		act ("You clone $p.", ch, clone, NULL, TO_CHAR);
		wiznet ("$N clones $p.", ch, clone, WIZ_LOAD, WIZ_SECURE,
			get_trust (ch));
		return;
	}
	else if (mob != NULL)
	{
		CHAR_DATA *clone;
		OBJ_DATA *new_obj;
		char buf[MAX_STRING_LENGTH];
		if (!IS_NPC (mob))
		{
			send_to_char ("You can only clone mobiles.\n\r", ch);
			return;
		}
		if ((mob->level > 20 && !IS_TRUSTED (ch, GOD))
			|| (mob->level > 10 && !IS_TRUSTED (ch, GOD))
			|| (mob->level > 5 && !IS_TRUSTED (ch, DEMIGOD))
			|| (mob->level > 0 && !IS_TRUSTED (ch, DEMIGOD))
			|| !IS_TRUSTED (ch, DEMIGOD))
		{
			send_to_char
				("Your powers are not great enough for such a task.\n\r", ch);
			return;
		}
		clone = create_mobile (mob->pIndexData);
		clone_mobile (mob, clone);
		for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
		{
			if (obj_check (ch, obj))
			{
				new_obj = create_object (obj->pIndexData, 0);
				clone_object (obj, new_obj);
				recursive_clone (ch, obj, new_obj);
				obj_to_char (new_obj, clone);
				new_obj->wear_loc = obj->wear_loc;
			}
		}
		char_to_room (clone, ch->in_room);
		act ("$n has created $N.", ch, NULL, clone, TO_ROOM);
		act ("You clone $N.", ch, NULL, clone, TO_CHAR);
		sprintf (buf, "$N clones %s.", clone->short_descr);
		wiznet (buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, get_trust (ch));
		return;
	}
}
void do_seize (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char ("Seize what from whom?\n\r", ch);
		return;
	}
	if ((victim = get_char_room (ch, arg2)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (victim == ch)
	{
		send_to_char ("That's pointless.\n\r", ch);
		return;
	}

	if (get_trust(ch) <= victim->level)
	{
		send_to_char ("You failed.\n\r", ch);
		sprintf (buf, "%s tried to seize an object from you.\n\r", ch->name);
		send_to_char (buf, victim);
		return;
	}
	if ((obj = get_obj_carry (victim, arg1)) == NULL)
	{
		if ((obj = get_obj_wear (victim, arg1)) == NULL)
		{
			send_to_char ("You can't find it.\n\r", ch);
			return;
		}
	}
	if (obj->wear_loc != -1)
		unequip_char (victim, obj);
	obj_from_char (obj);
	obj_to_char (obj, ch);
	//Iblis - 3/11/04 - Antiduping Code
	if (!IS_NPC(ch))
		save_char_obj (ch);
	if (!IS_NPC(victim))
		save_char_obj (victim);

	send_to_char ("Got it!\n\r", ch);
	return;
}

void do_load (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Syntax:\n\r", ch);
		send_to_char ("  load mob <vnum>\n\r", ch);
		send_to_char ("  load obj <vnum> <level>\n\r", ch);
		return;
	}
	if (!str_cmp (arg, "mob") || !str_cmp (arg, "char"))
	{
		do_mload (ch, argument);
		return;
	}
	if (!str_cmp (arg, "obj"))
	{
		do_oload (ch, argument);
		return;
	}

	/* echo syntax */
	do_load (ch, "");
}

void do_mload (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	one_argument (argument, arg);
	if (arg[0] == '\0' || !is_number (arg))
	{
		send_to_char ("Syntax: load mob <vnum>.\n\r", ch);
		return;
	}
	if ((pMobIndex = get_mob_index (atol (arg))) == NULL)
	{
		send_to_char ("No mob has that vnum.\n\r", ch);
		return;
	}
	victim = create_mobile (pMobIndex);
	char_to_room (victim, ch->in_room);

	if (!((IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_FAKIE)))
		act ("$n has created $N!", ch, NULL, victim, TO_ROOM);
	sprintf (buf, "$N loads %s.", victim->short_descr);
	wiznet (buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, get_trust (ch));
	sprintf (buf, "You have created %s\n\r", victim->short_descr);
	send_to_char (buf, ch);
	return;
}

void do_oload (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH];
	int level;
	CHAR_DATA *fch, *fch_next;
	argument = one_argument (argument, arg1);
	one_argument (argument, arg2);
	if (arg1[0] == '\0' || !is_number (arg1))
	{
		send_to_char ("Syntax: load obj <vnum> <level>.\n\r", ch);
		return;
	}
	level = get_trust (ch);	/* default */
	if (arg2[0] != '\0')
	{				/* load with a level */
		if (!is_number (arg2))
		{
			send_to_char ("Syntax: load obj <vnum> <level>.\n\r", ch);
			return;
		}
		level = atol (arg2);
		if (level < 0 || level > get_trust (ch))
		{
			send_to_char ("Level must be be between 0 and your level.\n\r", ch);
			return;
		}
	}
	if ((pObjIndex = get_obj_index (atol (arg1))) == NULL)
	{
		send_to_char ("No object has that vnum.\n\r", ch);
		return;
	}
	/*  if (pObjIndex->vnum == OBJ_VNUM_CLAN_SYMBOL)
	{
	send_to_char ("Sorry, you can't load that object.\n\r", ch);
	return;
	}
	*/  if (pObjIndex->item_type == ITEM_CTRANSPORT
		|| pObjIndex->item_type == ITEM_ELEVATOR)
		for (obj = object_list; obj != NULL; obj = obj->next)
			if (obj->pIndexData == pObjIndex)
			{
				send_to_char
					("You can't have more than one of this object in the world.\n\r",
					ch);
				return;
			}
			obj = create_object (pObjIndex, level);
			if (!CAN_WEAR (obj, ITEM_TAKE) || (IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_FAKIE))
				obj_to_room (obj, ch->in_room);
			else
				obj_to_char (obj, ch);
			for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
			{
				fch_next = fch->next_in_room;
				if (can_see (fch, ch) && !((IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_FAKIE)))
					act ("$n has created $p!", ch, obj, fch, TO_VICT);
			}

			//act ("$n has created $p!", ch, obj, NULL, TO_ROOM);
			wiznet ("$N loads $p.", ch, obj, WIZ_LOAD, WIZ_SECURE, get_trust (ch));
			sprintf (buf, "You have created %s\n\r", obj->short_descr);
			send_to_char (buf, ch);
			return;
}

void do_opurge (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj=NULL;
	bool found=FALSE;
	if (ch->level < 92 && !(IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_FAKIE))
	{
		send_to_char ("Huh?\n\r", ch);
		return;
	}
	one_argument (argument, arg);
	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		if (can_see_obj (ch, obj))
			if (is_name (arg, obj->name))
			{
				found = TRUE;
				break;
			}
			if (!found)
			{
				if ((obj = get_obj_here (ch, arg)) != NULL)
					found = TRUE;
				if (!found)
				{
					send_to_char ("You do not see that here.\n\r", ch);
					return;
				}
			}
			extract_obj (obj);
			send_to_char("Object purged.\n\r",ch);
			return;
}





void do_purge (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	//  char buf[100];
	CHAR_DATA *victim;		//, *gch, *gch_next, *vch, *vch_next;
	OBJ_DATA *obj;
	//  DESCRIPTOR_DATA *d;
	short level;
	one_argument (argument, arg);
	level = get_trust (ch);
	if (level < 92)
	{
		send_to_char ("Huh?\n\r", ch);
		return;
	}
	if (arg[0] == '\0')
	{

		/* 'purge' */
		CHAR_DATA *vnext;
		OBJ_DATA *obj_next;
		if (level < 95
			&& str_cmp (ch->in_room->area->creator, ch->name) != 0
			&& str_cmp (ch->in_room->area->helper, ch->name) != 0)
		{
			send_to_char ("Sorry, you can't do that here.\n\r", ch);
			return;
		}
		for (victim = ch->in_room->people; victim != NULL; victim = vnext)
		{
			vnext = victim->next_in_room;
			if (IS_NPC (victim) && !IS_SET (victim->act, ACT_NOPURGE)
				&& victim != ch /* safety precaution */ )
				extract_char (victim, TRUE);
		}
		for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			if (!IS_OBJ_STAT (obj, ITEM_NOPURGE))
				extract_obj (obj);
		}
		act ("$n purges the room!", ch, NULL, NULL, TO_ROOM);
		send_to_char ("You purged the room.\n\r", ch);
		return;
	}

	if (level < 95)
	{
		send_to_char ("Sorry, you can't do that.\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (!IS_NPC (victim))
	{
		ppurge(ch,victim);
	}
	extract_char (victim, TRUE);
	return;
}

void do_advance (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int level;
	int iLevel;
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number (arg2))
	{
		send_to_char ("Syntax: advance <char> <level>.\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("That player is not here.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Not on NPC's.\n\r", ch);
		return;
	}
	if ((level = atol (arg2)) < 1 || level > MAX_LEVEL)
	{
		send_to_char ("Level must be 1 to 100\n\r", ch);
		return;
	}
	if (level > get_trust (ch))
	{
		send_to_char ("Limited to your trust level.\n\r", ch);
		return;
	}
	if (get_trust (ch) <= get_trust (victim))
	{
		send_to_char ("I wouldn't do that if i were you.\n\r", ch);
		return;
	}
	if (level <= victim->level)
	{
		send_to_char ("Lowering a player's level!\n\r", ch);
		send_to_char ("**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim);
		for (iLevel = victim->level; iLevel > level; iLevel--)
		{
			send_to_char ("You lose a level!!  ", victim);
			decrease_level (victim);
		}
		save_char_obj (victim);
	}
	else
	{
		send_to_char ("Raising a player's level!\n\r", ch);
		send_to_char ("**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim);
		for (iLevel = victim->level; iLevel < level; iLevel++)
		{
			send_to_char ("You raise a level!!  ", victim);
			imm_advance = TRUE;
			advance_level (victim);
			imm_advance = FALSE;
			victim->exp += 0;
		}
		victim->trust = 0;
		save_char_obj (victim);
	}
	return;
}

void do_rpreward (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	long points;
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number (arg2))
	{
		send_to_char ("Syntax: rpreward <char> <rp points> <message>.\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("That player is not here.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Not on NPC's.\n\r", ch);
		return;
	}
	points = atol (arg2);
	if (victim->level > 91)
	{
		send_to_char
			("Sorry, that character cannot be rewarded in this way.\n\r", ch);
		return;
	}
	act (argument, victim, NULL, NULL, TO_CHAR);
	victim->pcdata->rppoints += points;
	send_to_char ("Ok.\n\r", ch);
	save_char_obj (victim);
}

void do_nhreward (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	long points;
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number (arg2))
	{
		send_to_char ("Syntax: nhreward <char> <nh points> <message>.\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("That player is not here.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Not on NPC's.\n\r", ch);
		return;
	}
	points = atol (arg2);
	if (victim->level > 91)
	{
		send_to_char
			("Sorry, that character cannot be rewarded in this way.\n\r", ch);
		return;
	}
	act (argument, victim, NULL, NULL, TO_CHAR);
	victim->pcdata->nhpoints += points;
	send_to_char ("Ok.\n\r", ch);
	save_char_obj (victim);
}

void do_qreward (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	long points;
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number (arg2))
	{
		send_to_char ("Syntax: qreward <char> <quest points> <message>.\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("That player is not here.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Not on NPC's.\n\r", ch);
		return;
	}
	points = atol (arg2);
	if (victim->level > 91)
	{
		send_to_char
			("Sorry, that character cannot be rewarded in this way.\n\r", ch);
		return;
	}
	act (argument, victim, NULL, NULL, TO_CHAR);
	victim->pcdata->qpoints += points;
	send_to_char ("Ok.\n\r", ch);
	save_char_obj (victim);
}

void do_reward (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	long exp;
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number (arg2))
	{
		send_to_char ("Syntax: reward <char> <exp points> <message>.\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("That player is not here.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Not on NPC's.\n\r", ch);
		return;
	}
	exp = atol (arg2);
	if (victim->level > 91)
	{
		send_to_char
			("Sorry, that character cannot be rewarded in this way.\n\r", ch);
		return;
	}
	act (argument, victim, NULL, NULL, TO_CHAR);
	if (!IS_NPC(ch))
		player_rewarding = TRUE;
	low_gain_exp (victim, exp);
	if (!IS_NPC(ch))
		player_rewarding = FALSE;
	send_to_char ("Ok.\n\r", ch);
	save_char_obj (victim);
	return;
}

void do_trust (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int level;
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number (arg2))
	{
		send_to_char ("Syntax: trust <char> <level>.\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("That player is not here.\n\r", ch);
		return;
	}
	if ((level = atol (arg2)) < 0 || level > MAX_LEVEL)
	{
		send_to_char ("Level must be 0 (reset) or 1 to 100.\n\r", ch);
		return;
	}
	if (level > get_trust (ch))
	{
		send_to_char ("Limited to your trust.\n\r", ch);
		return;
	}
	victim->trust = level;
	return;
}

void do_restoremsg (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char *cptr;
	if (IS_NPC (ch))
		return;
	one_argument (argument, arg);
	if (arg[0] != '\0')
		if (!str_cmp (arg, "reset"))
		{
			free_string (ch->pcdata->restoremsg);
			ch->pcdata->restoremsg = str_dup ("");
		}
		else
		{
			free_string (ch->pcdata->restoremsg);
			ch->pcdata->restoremsg = str_dup (argument);
			if ((cptr = strchr (ch->pcdata->restoremsg, '\n')) != NULL)
				*cptr = '\0';
			if ((cptr = strchr (ch->pcdata->restoremsg, '\r')) != NULL)
				*cptr = '\0';
			save_char_obj (ch);
			send_to_char ("Restore Message Updated.\n\r", ch);
			save_char_obj (ch);
		}
	else
	{
		send_to_char
			("syntax: restoremsg <Message to use>\n\r        restoremsg reset\n\r",
			ch);
		sprintf (buf, "\n\rYour restore message is set to: \n\r%s\n\r",
			ch->pcdata->restoremsg[0] !=
			'\0' ? ch->pcdata->restoremsg : "<default>");
		send_to_char (buf, ch);
	}
	return;
}

void do_restore (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *vch;
	DESCRIPTOR_DATA *d;
	one_argument (argument, arg);
	if (arg[0] == '\0' || !str_cmp (arg, "room"))
	{

		/* cure room */
		for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
		{
			affect_strip (vch, gsn_plague);
			affect_strip (vch, gsn_poison);
			affect_strip (vch, gsn_blindness);
			affect_strip (vch, gsn_sleep);
			affect_strip (vch, skill_lookup ("Ballad of Ole Rip"));
			vch->hit = vch->max_hit;
			vch->mana = vch->max_mana;
			vch->move = vch->max_move;
			update_pos (vch);
			if (!IS_NPC (ch) && (ch->pcdata->restoremsg[0] != '\0'))
				sprintf (buf, "%s", ch->pcdata->restoremsg);

			else
				sprintf (buf, "$n has restored you.");
			act (buf, ch, NULL, vch, TO_VICT);
		}
		sprintf (buf, "$N restored room %d.", ch->in_room->vnum);
		wiznet (buf, ch, NULL, WIZ_RESTORE, WIZ_SECURE, get_trust (ch));
		send_to_char ("Room restored.\n\r", ch);
		return;
	}
	if (!str_cmp (arg, "all"))
	{

		/* cure all */
		for (d = descriptor_list; d != NULL; d = d->next)
		{
			victim = d->character;
			if (victim == NULL || IS_NPC (victim))
				continue;
			affect_strip (victim, gsn_plague);
			affect_strip (victim, gsn_poison);
			affect_strip (victim, gsn_blindness);
			affect_strip (victim, gsn_sleep);
			affect_strip (victim, skill_lookup ("Ballad of Ole Rip"));
			victim->hit = victim->max_hit;
			victim->mana = victim->max_mana;
			victim->move = victim->max_move;
			update_pos (victim);
			if (victim->in_room != NULL)
			{
				if (!IS_NPC (ch) && (ch->pcdata->restoremsg[0] != '\0'))
					sprintf (buf, "%s", ch->pcdata->restoremsg);

				else
					sprintf (buf, "$n has restored you.");
			}
			act (buf, ch, NULL, victim, TO_VICT);
		}
		send_to_char ("All active players restored.\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	affect_strip (victim, gsn_plague);
	affect_strip (victim, gsn_poison);
	affect_strip (victim, gsn_blindness);
	affect_strip (victim, gsn_sleep);

	affect_strip (victim, skill_lookup ("Ballad of Ole Rip"));
	victim->hit = victim->max_hit;
	victim->mana = victim->max_mana;
	victim->move = victim->max_move;
	update_pos (victim);
	if (!IS_NPC (ch) && (ch->pcdata->restoremsg[0] != '\0'))
		sprintf (buf, "%s", ch->pcdata->restoremsg);

	else
		sprintf (buf, "$n has restored you.");
	act (buf, ch, NULL, victim, TO_VICT);
	sprintf (buf, "$N restored %s",
		IS_NPC (victim) ? victim->short_descr : victim->name);
	wiznet (buf, ch, NULL, WIZ_RESTORE, WIZ_SECURE, get_trust (ch));
	send_to_char ("Ok.\n\r", ch);
	return;
}

void do_freeze (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Freeze whom?\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Not on NPC's.\n\r", ch);
		return;
	}
	if (get_trust (victim) >= get_trust (ch))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (IS_SET (victim->act, PLR_FREEZE))
	{
		send_to_char ("This character is already frozen.\n\r", ch);
		return;
	}
	else
	{
		SET_BIT (victim->act, PLR_FREEZE);
		send_to_char ("You can't do ANYthing!\n\r", victim);
		send_to_char ("FREEZE set.\n\r", ch);
		sprintf (buf, "$N puts %s in the deep freeze.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	save_char_obj (victim);
	return;
}

void do_unfreeze (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Unfreeze whom?\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Not on NPC's.\n\r", ch);
		return;
	}
	if (get_trust (victim) >= get_trust (ch))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (IS_SET (victim->act, PLR_FREEZE))
	{
		REMOVE_BIT (victim->act, PLR_FREEZE);
		send_to_char ("You can play again.\n\r", victim);
		send_to_char ("FREEZE removed.\n\r", ch);
		sprintf (buf, "$N thaws %s.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else
	{
		send_to_char ("But that character is not even frozen?!?\n\r", ch);
		return;
	}
	save_char_obj (victim);
	return;
}

void do_log (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Log whom?\n\r", ch);
		return;
	}
	if (!str_cmp (arg, "all"))
	{
		if (fLogAll)
		{
			fLogAll = FALSE;
			send_to_char ("Log ALL off.\n\r", ch);
		}
		else
		{
			fLogAll = TRUE;
			send_to_char ("Log ALL on.\n\r", ch);
		}
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Not on NPC's.\n\r", ch);
		return;
	}

	/*
	* No level check, gods can log anyone.
	*/
	if (IS_SET (victim->act, PLR_LOG))
	{
		REMOVE_BIT (victim->act, PLR_LOG);
		send_to_char ("LOG removed.\n\r", ch);
	}
	else
	{
		SET_BIT (victim->act, PLR_LOG);
		send_to_char ("LOG set.\n\r", ch);
	}
	return;
}

void do_noemote (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Noemote whom?\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (get_trust (victim) >= get_trust (ch))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (IS_SET (victim->comm, COMM_NOEMOTE))
	{
		REMOVE_BIT (victim->comm, COMM_NOEMOTE);
		send_to_char ("You can emote again.\n\r", victim);
		send_to_char ("NOEMOTE removed.\n\r", ch);
		sprintf (buf, "$N restores emotes to %s.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else
	{
		SET_BIT (victim->comm, COMM_NOEMOTE);
		send_to_char ("You can't emote!\n\r", victim);
		send_to_char ("NOEMOTE set.\n\r", ch);
		sprintf (buf, "$N revokes %s's emotes.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	return;
}

void do_noshout (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Noshout whom?\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Not on NPC's.\n\r", ch);
		return;
	}
	if (get_trust (victim) >= get_trust (ch) && ch != victim)
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (IS_SET (victim->comm, COMM_NOSHOUT))
	{
		REMOVE_BIT (victim->comm, COMM_NOSHOUT);
		send_to_char ("You can shout again.\n\r", victim);
		send_to_char ("NOSHOUT removed.\n\r", ch);
		sprintf (buf, "$N restores shouts to %s.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else
	{
		SET_BIT (victim->comm, COMM_NOSHOUT);
		send_to_char ("You can't shout!\n\r", victim);
		send_to_char ("NOSHOUT set.\n\r", ch);
		sprintf (buf, "$N revokes %s's shouts.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	return;
}

void do_notell (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Notell whom?", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (get_trust (victim) >= get_trust (ch))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	// Added by Morgan on June 25. 2000
	// Extra check for setting notell on mobs
	if (IS_NPC (victim))
	{
		send_to_char ("Setting NOTELL on mobs is pointless.\n\r", ch);
		return;
	}
	if (IS_SET (victim->comm, COMM_NOTELL))
	{
		REMOVE_BIT (victim->comm, COMM_NOTELL);
		send_to_char ("You can tell again.\n\r", victim);
		send_to_char ("NOTELL removed.\n\r", ch);
		sprintf (buf, "$N restores tells to %s.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else
	{
		SET_BIT (victim->comm, COMM_NOTELL);
		send_to_char ("You can't tell!\n\r", victim);
		send_to_char ("NOTELL set.\n\r", ch);
		sprintf (buf, "$N revokes %s's tells.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	return;
}


// Added notitle as penalty command for immortals
// Morgan, June 25. 2000
void do_notitle (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Notitle who?", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (get_trust (victim) >= get_trust (ch))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Setting NOTITLE on mobs is pointless.\n\r", ch);
		return;
	}
	if (IS_SET (victim->comm, COMM_NOTITLE))
	{
		REMOVE_BIT (victim->comm, COMM_NOTITLE);
		send_to_char ("You can change your title again.\n\r", victim);
		send_to_char ("NOTITLE removed.\n\r", ch);
		sprintf (buf, "$N restores ability to change titles to %s.",
			victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else
	{
		SET_BIT (victim->comm, COMM_NOTITLE);
		send_to_char
			("Your ability to change titles have been revoked!\n\r", victim);
		send_to_char ("NOTITLE set.\n\r", ch);
		sprintf (buf, "$N revokes %s's titles.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	return;
}

bool check_parse_name (char *name);	/* comm.c */

void do_rename (CHAR_DATA * ch, char *argument)
{
	char old_name[MAX_INPUT_LENGTH],
		new_name[MAX_INPUT_LENGTH], strsave[MAX_INPUT_LENGTH];

	CHAR_DATA *victim;
	FILE *file;

	argument = one_argument (argument, old_name);	/* find new/old name */
	one_argument (argument, new_name);

	/* Trivial checks */
	if (!old_name[0])
	{
		send_to_char ("Rename who?\n\r", ch);
		return;
	}

	victim = get_char_world (ch, old_name);

	if (!victim)
	{
		send_to_char ("There is no such a person online.\n\r", ch);
		return;
	}

	if (IS_NPC (victim))
	{
		send_to_char ("You cannot use Rename on NPCs.\n\r", ch);
		return;
	}

	/* allow rename self new_name,but otherwise only lower level */
	if ((victim != ch) && (get_trust (victim) >= get_trust (ch)))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}

	if (!victim->desc || (victim->desc->connected != CON_PLAYING))
	{
		send_to_char
			("This player has lost his link or is inside a pager or the like.\n\r",
			ch);
		return;
	}

	if (!new_name[0])
	{
		send_to_char ("Rename to what new name?\n\r", ch);
		return;
	}

	/* Insert check for clan here!! */
	/*

	if (victim->clan)
	{
	send_to_char ("This player is member of a clan, remove him from there first.\n\r",ch);
	return;
	}
	*/

	if (!check_parse_name (new_name))
	{
		send_to_char ("The new name is illegal.\n\r", ch);
		return;
	}

	/* First, check if there is a player named that off-line */
	sprintf (strsave, "%s%c/%s", PLAYER_DIR, toupper (new_name[0]),
		capitalize (new_name));

	fclose (fpReserve);		/* close the reserve file */
	file = fopen (strsave, "r");	/* attempt to to open pfile */
	if (file)
	{
		send_to_char ("A player with that name already exists!\n\r", ch);
		fclose (file);
		fpReserve = fopen (NULL_FILE, "r");	/* is this really necessary these days? */
		return;
	}
	fpReserve = fopen (NULL_FILE, "r");	/* reopen the extra file */

	if (get_char_world (ch, new_name))	/* check for playing level-1 non-saved */
	{
		send_to_char
			("A player with the name you specified already exists!\n\r", ch);
		return;
	}

	/* Save the filename of the old name */

	sprintf (strsave, "%s%c/%s", PLAYER_DIR, toupper (victim->name[0]),
		capitalize (victim->name));


	/* Rename the character and save him to a new file */
	/* NOTE: Players who are level 1 do NOT get saved under a new name */

	free_string (victim->name);
	victim->name = str_dup (capitalize (new_name));

	save_char_obj (victim);

	/* unlink the old file */
	unlink (strsave);		/* unlink does return a value.. but we do not care */

	/* That's it! */

	send_to_char ("Character renamed.\n\r", ch);

	victim->position = POS_STANDING;	/* I am laaazy */
	act ("$n has renamed you to $N!", ch, NULL, victim, TO_VICT);

}				/* do_rename */



void do_peace (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *rch;
	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
	{
		if (rch->fighting != NULL)
			stop_fighting (rch, TRUE);
		if (IS_NPC (rch) && is_aggro (rch))
		{
			REMOVE_BIT (rch->act, ACT_AGGRESSIVE);
			REMOVE_BIT (rch->act2, ACT_AGGIE_EVIL);
			REMOVE_BIT (rch->act2, ACT_AGGIE_NEUTRAL);
			REMOVE_BIT (rch->act2, ACT_AGGIE_GOOD);
			if (rch->mood == 1)
				rch->mood = 2;
		}
	}
	send_to_char ("Ok.\n\r", ch);
	return;
}

void do_grant (CHAR_DATA * ch, char *argument)
{
	char player[MAX_STRING_LENGTH], tcbuf[MAX_STRING_LENGTH];
	CHAR_DATA *pch;
	argument = one_argument (argument, player);
	if (player[0] == '\0')
	{
		send_to_char ("Who do you want to grant this too?\n\r", ch);
		return;
	}
	pch = get_char_world (ch, player);
	if (pch == NULL)
	{
		send_to_char ("No such user found.\n\r", ch);
		return;
	}
	if (IS_NPC (pch))
	{
		send_to_char ("You can only grant privledges to real players.\n\r", ch);
		return;
	}
	pch->pcdata->security = !pch->pcdata->security;
	if (pch->pcdata->security)
	{
		sprintf (tcbuf, "%s has been granted builder status.\n\r", pch->name);
		send_to_char (tcbuf, ch);
		sprintf (tcbuf, "%s has granted you builder status.\n\r", ch->name);
		send_to_char (tcbuf, pch);
	}
	else
	{
		sprintf (tcbuf, "Builder status removed from %s.\n\r", pch->name);
		send_to_char (tcbuf, ch);
		sprintf (tcbuf, "Your builder status has been removed by %s.\n\r",
			ch->name);
		send_to_char (tcbuf, pch);
	}
	return;
}


// Added an imm toggle to make NonPK's safe from attacks
// from PK players, Morgan on June 29. 2000
void do_safe_nopk (CHAR_DATA * ch, char *argument)
{
	extern bool safe_nopk;
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument (argument, arg);
	if (!str_cmp (arg, "confirm"))
	{
		if (safe_nopk)
		{
			send_to_char ("The NonPK safety switch is now turned OFF.\n\r", ch);
			safe_nopk = FALSE;
			return;
		}
		else
		{
			send_to_char ("The NonPK safety switch is now turned ON.\n\r", ch);
			safe_nopk = TRUE;
			return;
		}
	}
	send_to_char
		("This toggles the NonPK safety switch.\n\rTo toggle nonpk safety, type \"safe_nopk confirm\".\n\r",
		ch);
	return;
}

void do_badsummon (CHAR_DATA * ch, char *argument)
{
	extern bool bad_summoning_on;
	if (bad_summoning_on)
	{
		send_to_char ("bad_summoning is now turned OFF.\n\r", ch);
		bad_summoning_on = FALSE;
	}
	else
	{
		send_to_char ("bad_summoning is now turned ON.\n\r", ch);
		bad_summoning_on = TRUE;
	}
	return;
}
void do_tractivate (CHAR_DATA * ch, char *argument)
{
	extern bool exploration_tracking;
	if (exploration_tracking)
	{
		send_to_char ("Exploration Tracking is now turned OFF.\n\r", ch);
		exploration_tracking = FALSE;
	}
	else
	{
		send_to_char ("Exploration Tracking is now turned ON.\n\r", ch);
		exploration_tracking = TRUE;
	}
	return;
}

void do_half_xp (CHAR_DATA * ch, char *argument)
{
	extern bool doub;
	extern bool quad;
	extern bool half;
	int time;
	if (doub)
	{
		send_to_char ("Not while double experience is on..\n\r", ch);
		return;
	}
	if (quad)
	{
		send_to_char ("Not while quadruple experience is on..\n\r", ch);
		return;
	}
	if (half)
	{
		send_to_char
			("No, let the Half experience time out by itself..\n\r", ch);
		return;
	}
	else
	{
		send_to_char ("Half Experience is now turned ON.\n\r", ch);
		do_echo (ch,
			"`aThe anger of the Gods shower upon you, you gain less experience in your adventures.``\n\r");
		half = TRUE;
		if (is_number (argument))
		{
			time = atol (argument);
			half_timer = time;
		}
		else
			half_timer = 45;
	}
	return;
}

void do_double_xp (CHAR_DATA * ch, char *argument)
{
	extern bool doub;
	extern bool quad;
	extern bool half;
	int time;
	if (half)
	{
		send_to_char
			("People here deserve the time with only half gain..\n\r", ch);
		return;
	}
	if (quad)
	{
		send_to_char ("Not while quadruple experience is on..\n\r", ch);
		return;
	}
	if (doub)
	{
		send_to_char
			("No, let the Double experience time out by itself..\n\r", ch);
		return;
	}
	else
	{
		send_to_char ("Double Experience is now turned ON.\n\r", ch);
		do_echo (ch,
			"`jThe blessings of the Gods shower upon you, you gain more experience in your adventures.``\n\r");
		doub = TRUE;
		if (is_number (argument))
		{
			time = atol (argument);
			double_timer = time;
		}
		else
			double_timer = 45;
	}
	return;
}

void do_quad_xp (CHAR_DATA * ch, char *argument)
{
	extern bool quad;
	extern bool doub;
	extern bool half;
	int time;
	if (half)
	{
		send_to_char
			("People here deserve the time with only half gain..\n\r", ch);
		return;
	}
	if (doub)
	{
		send_to_char ("Not while double experience is on..\n\r", ch);
		return;
	}
	if (quad)
	{
		send_to_char
			("No, let the Quad experience time out by itself..\n\r", ch);
		return;
	}
	else
	{
		send_to_char ("Quadruple Experience is now turned ON.\n\r", ch);
		do_echo (ch,
			"`kThe blessings of the Gods shower upon you, you gain more experience in your adventures.``\n\r");
		quad = TRUE;
		if (is_number (argument))
		{
			time = atol (argument);
			quad_timer = time;
		}
		else
			quad_timer = 45;
	}
	return;
}

void do_cancel_xp (CHAR_DATA * ch, char *argument)
{
	extern bool quad;
	extern bool doub;
	extern bool half;
	if (quad)
	{
		quad_timer = 0;
		send_to_char ("Quad Experience canelled.\n\r", ch);
		return;
	}
	if (doub)
	{
		double_timer = 0;
		send_to_char ("Double experience cancelled.\n\r", ch);
		return;
	}
	if (half)
	{
		half_timer = 0;
		send_to_char ("Half Experience cancelled.\n\r", ch);
		return;
	}
	return;
}

void do_wizlock (CHAR_DATA * ch, char *argument)
{
	FILE *nfp;
	extern bool wizlock;
	wizlock = !wizlock;
	if (wizlock)
	{
		wiznet ("$N has wizlocked the game.", ch, NULL, 0, 0, 0);
		send_to_char ("Game wizlocked.\n\r", ch);
		if ((nfp = fopen ("wizlock", "w")) != NULL)
		{
			fprintf (nfp, "%s\n", ch->name);
			fclose (nfp);
		}
	}
	else
	{
		wiznet ("$N removes wizlock.", ch, NULL, 0, 0, 0);
		send_to_char ("Game un-wizlocked.\n\r", ch);
		unlink ("wizlock");
	}
	return;
}


/* RT anti-newbie code */
void do_newlock (CHAR_DATA * ch, char *argument)
{
	FILE *nfp;
	extern bool newlock;
	newlock = !newlock;
	if (newlock)
	{
		wiznet ("$N locks out new characters.", ch, NULL, 0, 0, 0);
		send_to_char ("New characters have been locked out.\n\r", ch);
		if ((nfp = fopen ("newlock", "w")) != NULL)
		{
			fprintf (nfp, "%s\n", ch->name);
			fclose (nfp);
		}
	}
	else
	{
		wiznet ("$N allows new characters back in.", ch, NULL, 0, 0, 0);
		send_to_char ("Newlock removed.\n\r", ch);
		unlink ("newlock");
	}
	return;
}

void do_slookup (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int sn;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Lookup which skill or spell?\n\r", ch);
		return;
	}
	if (!str_cmp (arg, "all"))
	{
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].name == NULL)
				break;
			sprintf (buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r", sn,
				skill_table[sn].slot, skill_table[sn].name);
			send_to_char (buf, ch);
		}
	}
	else
	{
		if ((sn = skill_lookup (arg)) < 0)
		{
			send_to_char ("No such skill or spell.\n\r", ch);
			return;
		}
		sprintf (buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r", sn,
			skill_table[sn].slot, skill_table[sn].name);
		send_to_char (buf, ch);
	}
	return;
}


/* RT set replaces sset, mset, oset, and rset */
void do_set (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Syntax:\n\r", ch);
		send_to_char ("  set mob   <name> <field> <value>\n\r", ch);
		send_to_char ("  set obj   <name> <field> <value>\n\r", ch);
		send_to_char ("  set room  <room> <field> <value>\n\r", ch);
		send_to_char ("  set skill <name> <spell or skill> <value>\n\r", ch);
		send_to_char ("  set clan  <name> [pk/nopk]\n\r", ch);
		return;
	}
	if (!str_prefix (arg, "mobile") || !str_prefix (arg, "character"))
	{
		do_mset (ch, argument);
		return;
	}
	if (!str_prefix (arg, "skill") || !str_prefix (arg, "spell"))
	{
		do_sset (ch, argument);
		return;
	}
	if (!str_prefix (arg, "object"))
	{
		do_oset (ch, argument);
		return;
	}
	if (!str_prefix (arg, "room"))
	{
		do_rset (ch, argument);
		return;
	}
	if (!str_prefix (arg, "clan"))
	{
		do_setclan (ch, argument);
		return;
	}

	/* echo syntax */
	do_set (ch, "");
}

void make_noble (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *who;
	argument = one_argument (argument, arg1);
	smash_tilde (argument);
	strcpy (arg2, argument);
	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char ("Syntax: noble <player> <whatever/none>\n\r", ch);
		return;
	}
	if ((who = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("Thar player is not connected.\n\r", ch);
		return;
	}
	if (IS_NPC (who))
	{
		send_to_char ("Mobiles don't have pretitles.\n\r", ch);
		return;
	}
	if (!str_cmp (argument, "none"))
	{
		free_string (who->pcdata->pretitle);
		who->pcdata->pretitle = str_dup ("");
		send_to_char ("Nobility removed.\n\r", ch);
		return;
	}
	else
	{
		strcat (argument, "``");
		free_string (who->pcdata->pretitle);
		who->pcdata->pretitle = str_dup (argument);
		send_to_char ("Nobility set.\n\r", ch);
		return;
	}
	send_to_char ("Syntax: noble <player> <whatever/none>\n\r", ch);
	return;
}

void do_setclan (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int clan;
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char ("Syntax: set clan <clan name> [pk/nopk]\n\r", ch);
		return;
	}
	clan = clan_lookup (arg1);
	if (clan == CLAN_BOGUS)
	{
		send_to_char ("That clan does not exist.\n\r", ch);
		return;
	}
	if (!str_cmp (arg2, "pk"))
	{
		clan_table[clan].ctype = CLAN_TYPE_PC;
		sprintf (buf, "clan %s is now a PK clan.\n\r", arg1);
		send_to_char (buf, ch);
		save_clans ();
		return;
	}
	else if (!str_cmp (arg2, "nopk"))
	{
		clan_table[clan].ctype = CLAN_TYPE_EXPL;
		sprintf (buf, "clan %s is now a NonPK clan.\n\r", arg1);
		send_to_char (buf, ch);
		save_clans ();
		return;
	}
	else
	{
		send_to_char ("Should the clan be set as PK or NonPK ?\n\r", ch);
		return;
	}
}
void do_sset (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int value;
	int sn;
	bool fAll;
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	argument = one_argument (argument, arg3);
	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
	{
		send_to_char ("Syntax:\n\r", ch);
		send_to_char ("  set skill <name> <spell or skill> <value>\n\r", ch);
		send_to_char ("  set skill <name> all <value>\n\r", ch);
		send_to_char ("   (use the name of the skill, not the number)\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Not on NPC's.\n\r", ch);
		return;
	}
	fAll = !str_cmp (arg2, "all");
	sn = 0;
	if (!fAll && (sn = skill_lookup (arg2)) < 0)
	{
		send_to_char ("No such skill or spell.\n\r", ch);
		return;
	}

	/*
	* Snarf the value.
	*/
	if (!is_number (arg3))
	{
		send_to_char ("Value must be numeric.\n\r", ch);
		return;
	}
	value = atol (arg3);
	if (value < 0 || value > 100)
	{
		send_to_char ("Value range is 0 to 100.\n\r", ch);
		return;
	}
	affect_strip_skills(victim);
	if (fAll)
	{
		for (sn = 0; sn < MAX_SKILL; sn++)
		{

			//Iblis 5/14/04 - Set so careless imms won't set all and give nonsidhe/nonassassins mask
			if (skill_table[sn].name != NULL)
			{
				victim->pcdata->learned[sn] = value;
			}
		}
	}
	else
	{
		victim->pcdata->learned[sn] = value;
	}
	return;
}

void do_mset (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char buf[100];
	CHAR_DATA *victim;
	CHAR_DATA *hunted;
	int value;
	smash_tilde (argument);
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	strcpy (arg3, argument);
	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
	{
		send_to_char ("Syntax:\n\r", ch);
		send_to_char ("  set char <name> <field> <value>\n\r", ch);
		send_to_char ("  Field being one of:\n\r", ch);
		send_to_char ("    str int wis dex con cha sex Class level mood jailed totalxp\n\r",
			ch);
		send_to_char ("    race group gold silver hp mana move prac wanted age experience\n\r",
			ch);
		send_to_char ("    align train thirst hunger drunk full hand loner hero souls oldrace\n\r",
			ch);
		send_to_char ("    Class1 Class2 Class3 gainedhp gainedmove gainedmana totalsouls\n\r", ch);
		send_to_char ("    numattacks\n\r",ch);
		return;
	}
	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	/* clear zones for mobs */
	victim->zone = NULL;

	/*
	* Snarf the value (which need not be numeric).
	*/
	value = is_number (arg3) ? atol (arg3) : -1;

	/*
	* Set something.
	*/
	if (!str_cmp (arg2, "str"))
	{
		if (value < 3 || value > get_max_train (victim, STAT_STR))
		{
			sprintf (buf, "Strength range is 3 to %d\n\r.",
				get_max_train (victim, STAT_STR));
			send_to_char (buf, ch);
			return;
		}
		victim->perm_stat[STAT_STR] = value;
		return;
	}
	if (!str_cmp (arg2, "loner"))
	{
		if (IS_NPC (victim))
		{
			send_to_char ("You can not set the loner flag on mobiles.\n\r", ch);
			return;
		}
		if (!str_cmp (arg3, "true"))
			victim->pcdata->loner = TRUE;

		else if (!str_cmp (arg3, "false"))
			victim->pcdata->loner = FALSE;

		else
			send_to_char
			("Valid values for the loner flag are 'true' and 'false'\n\r", ch);
		return;
	}
	if (!str_cmp (arg2, "hero"))
	{
		if (IS_NPC (victim))
		{
			send_to_char ("You can not set the hero flag on mobiles.\n\r", ch);
			return;
		}
		if (!str_cmp (arg3, "true"))
		{
			victim->pcdata->hero = TRUE;
		}
		else if (!str_cmp (arg3, "false"))
		{
			victim->pcdata->hero = FALSE;
		}
		else
		{
			send_to_char
				("Valid values for the hero flag are 'true' and 'false'\n\r", ch);
		}
		return;
	}

	if (!str_cmp (arg2, "hunt"))
	{
		if (!IS_NPC (victim))
		{
			send_to_char ("Setting players to hunt? What do YOU think?\n\r", ch);
			return;
		}

		if ((hunted = get_char_world (ch, arg3)) == NULL)
		{
			send_to_char ("Sorry, that player is not online right now?!?\n\r", ch);
			return;
		}

		if (IS_NPC (hunted))
		{
			send_to_char ("Letting mobs hunt mobs? What do YOU think?\n\r", ch);
			return;
		}

		add_hunting(victim, hunted);
		SET_BIT (victim->act, ACT_SMART_HUNT);
		sprintf(buf,"%s now hunts %s!\n\r", victim->short_descr, hunted->name);
		send_to_char(buf, ch);
		return;
	}

	if (!str_cmp (arg2, "jailed"))
	{
		if (value > 0)
		{
			if (!IS_SET (victim->act, PLR_JAILED))
				arrest (ch, victim);
		}
		victim->want_jail_time = value;
		return;
	}
	if (!str_cmp (arg2, "wanted"))
	{
		if (value > 0)
			SET_BIT (victim->act, PLR_WANTED);
		victim->want_jail_time = value;
		return;
	}
	if (!str_cmp (arg2, "experience"))
	{
		if (value > -1)
			victim->exp = value;
		return;
	}
	if (!str_cmp (arg2, "int"))
	{
		if (value < 3 || value > get_max_train (victim, STAT_INT))
		{
			sprintf (buf, "Intelligence range is 3 to %d.\n\r",
				get_max_train (victim, STAT_INT));
			send_to_char (buf, ch);
			return;
		}
		victim->perm_stat[STAT_INT] = value;
		return;
	}
	if (!str_cmp (arg2, "wis"))
	{
		if (value < 3 || value > get_max_train (victim, STAT_WIS))
		{
			sprintf (buf, "Wisdom range is 3 to %d.\n\r",
				get_max_train (victim, STAT_WIS));
			send_to_char (buf, ch);
			return;
		}
		victim->perm_stat[STAT_WIS] = value;
		return;
	}
	if (!str_cmp (arg2, "dex"))
	{
		if (value < 3 || value > get_max_train (victim, STAT_DEX))
		{
			sprintf (buf, "Dexterity ranges is 3 to %d.\n\r",
				get_max_train (victim, STAT_DEX));
			send_to_char (buf, ch);
			return;
		}
		victim->perm_stat[STAT_DEX] = value;
		return;
	}
	if (!str_cmp (arg2, "cha"))
	{
		if (value < 3 || value > get_max_train (victim, STAT_CHA))
		{
			sprintf (buf, "Charisma range is 3 to %d.\n\r",
				get_max_train (victim, STAT_CHA));
			send_to_char (buf, ch);
			return;
		}
		victim->perm_stat[STAT_CHA] = value;
		return;
	}
	if (!str_cmp (arg2, "con"))
	{
		if (value < 3 || value > get_max_train (victim, STAT_CON))
		{
			sprintf (buf, "Constitution range is 3 to %d.\n\r",
				get_max_train (victim, STAT_CON));
			send_to_char (buf, ch);
			return;
		}
		victim->perm_stat[STAT_CON] = value;
		return;
	}
	if (!str_prefix (arg2, "sex"))
	{
		if (value < 0 || value > 2)
		{
			send_to_char ("Sex range is 0 to 2.\n\r", ch);
			return;
		}
		victim->sex = value;
		if (!IS_NPC (victim))
			victim->pcdata->true_sex = value;
		return;
	}
	if (!str_prefix (arg2, "Class"))
	{
		int Class;
		if (IS_NPC (victim))
		{
			send_to_char ("Mobiles have no Class.\n\r", ch);
			return;
		}
		Class = Class_lookup (arg3);
		if (Class == -1)
		{
			char buf[MAX_STRING_LENGTH];
			strcpy (buf, "Possible Classes are: ");
			for (Class = 0; Class < MAX_CLASS; Class++)
			{
				if (Class > 0)
					strcat (buf, " ");
				strcat (buf, Class_table[Class].name);
			}
			strcat (buf, ".\n\r");
			send_to_char (buf, ch);
			return;
		}
		//Iblis 5/13/03 - This is to prevent crashes.  Ideally, when someone is set to be a reaver,
		//the format of their pfile should properly change, but this works for now
		if (Class == 8)
		{
			send_to_char
				("You can't set a character to be a Reaver (this crashes the mud)\n\r",
				ch);
			return;
		}
		victim->Class = Class;
		victim->Class2 = Class;
		victim->Class3 = Class;
		return;
	}
	if (!str_prefix (arg2, "Class1"))
	{
		int Class;
		if (IS_NPC (victim))
		{
			send_to_char ("Mobiles have no Class.\n\r", ch);
			return;
		}
		Class = Class_lookup (arg3);
		if (Class == -1)
		{
			char buf[MAX_STRING_LENGTH];
			strcpy (buf, "Possible Classes are: ");
			for (Class = 0; Class < MAX_CLASS; Class++)
			{
				if (Class > 0)
					strcat (buf, " ");
				strcat (buf, Class_table[Class].name);
			}
			strcat (buf, ".\n\r");
			send_to_char (buf, ch);
			return;
		}
		//Iblis 5/13/03 - This is to prevent crashes.  Ideally, when someone is set to be a reaver,
		//the format of their pfile should properly change, but this works for now
		if (Class == 8)
		{
			send_to_char ("You can't set a character to be a Reaver (this crashes the mud)\n\r",
				ch);
			return;
		}
		victim->Class = Class;
		return;
	}
	if (!str_prefix (arg2, "Class2"))
	{
		int Class;
		if (IS_NPC (victim))
		{
			send_to_char ("Mobiles have no Class.\n\r", ch);
			return;
		}
		Class = Class_lookup (arg3);
		if (Class == -1)
		{
			char buf[MAX_STRING_LENGTH];
			strcpy (buf, "Possible Classes are: ");
			for (Class = 0; Class < MAX_CLASS; Class++)
			{
				if (Class > 0)
					strcat (buf, " ");
				strcat (buf, Class_table[Class].name);
			}
			strcat (buf, ".\n\r");
			send_to_char (buf, ch);
			return;
		}
		//Iblis 5/13/03 - This is to prevent crashes.  Ideally, when someone is set to be a reaver,
		//the format of their pfile should properly change, but this works for now
		if (Class == 8)
		{
			send_to_char
				("You can't set a character to be a Reaver (this crashes the mud)\n\r",
				ch);
			return;
		}
		victim->Class2 = Class;
		return;
	}
	if (!str_prefix (arg2, "Class3"))
	{
		int Class;
		if (IS_NPC (victim))
		{
			send_to_char ("Mobiles have no Class.\n\r", ch);
			return;
		}
		Class = Class_lookup (arg3);
		if (Class == -1)
		{
			char buf[MAX_STRING_LENGTH];
			strcpy (buf, "Possible Classes are: ");
			for (Class = 0; Class < MAX_CLASS; Class++)
			{
				if (Class > 0)
					strcat (buf, " ");
				strcat (buf, Class_table[Class].name);
			}
			strcat (buf, ".\n\r");
			send_to_char (buf, ch);
			return;
		}
		//Iblis 5/13/03 - This is to prevent crashes.  Ideally, when someone is set to be a reaver,
		//the format of their pfile should properly change, but this works for now
		if (Class == 8)
		{
			send_to_char
				("You can't set a character to be a Reaver (this crashes the mud)\n\r",
				ch);
			return;
		}
		victim->Class3 = Class;
		return;
	}
	if (!str_prefix (arg2, "hand"))
	{
		int hand;
		if (IS_NPC (victim))
		{
			send_to_char ("Mobiles have no primary hand.\n\r", ch);
			return;
		}
		hand = hand_lookup (arg3);
		if (hand == -1)
		{
			send_to_char
				("Possible primary hands are: left right ambidextrous.\n\r", ch);
			return;
		}
		victim->pcdata->primary_hand = hand;
		return;
	}
	if (!str_prefix (arg2, "level"))
	{

		if (value < 0 || value > 100)
		{
			send_to_char ("Level range is 0 to 100.\n\r", ch);
			return;
		}
		//5-11-03 Iblis - Fixed this so you can't set chars to a higher level than you are
		if ((ch->level < value) && (!IS_NPC (victim)))
			send_to_char
			("You can't set players to a higher level than you are.\n\r", ch);
		else
			victim->level = value;
		return;
	}
	if (!str_prefix (arg2, "mood"))
	{
		if (!IS_NPC (victim))
		{
			send_to_char ("Not on PC's.\n\r", ch);
			return;
		}
		if (value < 0 || value > 8)
		{
			send_to_char ("Mood range is 0 to 8.\n\r", ch);
			return;
		}
		victim->mood = value;
		return;
	}
	if (!str_prefix (arg2, "gold"))
	{
		victim->gold = value;
		return;
	}
	if (!str_prefix (arg2, "silver"))
	{
		victim->silver = value;
		return;
	}
	if (!str_prefix (arg2, "hp"))
	{
		if (value < -10 || value > 30000)
		{
			send_to_char ("Hp range is -10 to 30,000 hit points.\n\r", ch);
			return;
		}
		victim->max_hit = value;
		if (!IS_NPC (victim))
			victim->pcdata->perm_hit = value;
		return;
	}
	if (!str_prefix (arg2, "mana"))
	{
		if (value < 0 || value > 30000)
		{
			send_to_char ("Mana range is 0 to 30,000 mana points.\n\r", ch);
			return;
		}
		victim->max_mana = value;
		if (!IS_NPC (victim))
			victim->pcdata->perm_mana = value;
		return;
	}
	if (!str_prefix (arg2, "move"))
	{
		if (value < 0 || value > 30000)
		{
			send_to_char ("Move range is 0 to 30,000 move points.\n\r", ch);
			return;
		}
		victim->max_move = value;
		if (!IS_NPC (victim))
			victim->pcdata->perm_move = value;
		return;
	}
	if (!str_prefix (arg2, "train"))
	{
		if (value < 0 || value > 50)
		{
			send_to_char
				("Training session range is 0 to 50 sessions.\n\r", ch);
			return;
		}
		victim->train = value;
		return;
	}
	if (!str_prefix (arg2, "align"))
	{
		if (value < -1000 || value > 1000)
		{
			send_to_char ("Alignment range is -1000 to 1000.\n\r", ch);
			return;
		}
		victim->alignment = value;
		return;
	}
	if (!str_prefix (arg2, "thirst"))
	{
		if (IS_NPC (victim))
		{
			send_to_char ("Not on NPC's.\n\r", ch);
			return;
		}
		if (value < -1 || value > 100)
		{
			send_to_char ("Thirst range is -1 to 100.\n\r", ch);
			return;
		}
		victim->pcdata->condition[COND_THIRST] = value;
		return;
	}
	if (!str_prefix (arg2, "age"))
	{
		if (IS_NPC (victim))
		{
			send_to_char ("Not on NPC's.\n\r", ch);
			return;
		}
		if (value < 17)
		{
			send_to_char ("Players here can't be younger than 17.\n\r", ch);
			return;
		}
		value = value - 17;
		value = value * 72000;
		victim->played = value;
		return;
	}
	if (!str_prefix (arg2, "drunk"))
	{
		if (IS_NPC (victim))
		{
			send_to_char ("Not on NPC's.\n\r", ch);
			return;
		}
		if (value < -1 || value > 100)
		{
			send_to_char ("Drunk range is -1 to 100.\n\r", ch);
			return;
		}
		victim->pcdata->condition[COND_DRUNK] = value;
		return;
	}
	if (!str_prefix (arg2, "full"))
	{
		if (IS_NPC (victim))
		{
			send_to_char ("Not on NPC's.\n\r", ch);
			return;
		}
		if (value < -1 || value > 100)
		{
			send_to_char ("Full range is -1 to 100.\n\r", ch);
			return;
		}
		victim->pcdata->condition[COND_FULL] = value;
		return;
	}
	if (!str_prefix (arg2, "hunger"))
	{
		if (IS_NPC (victim))
		{
			send_to_char ("Not on NPC's.\n\r", ch);
			return;
		}
		if (value < -1 || value > 100)
		{
			send_to_char ("Full range is -1 to 100.\n\r", ch);
			return;
		}
		victim->pcdata->condition[COND_HUNGER] = value;
		return;
	}
	if (!str_prefix (arg2, "race"))
	{
		int race;
		race = race_lookup (arg3);
		if (race == 0)
		{
			send_to_char ("That is not a valid race.\n\r", ch);
			return;
		}
		if (!IS_NPC (victim) && !race_table[race].pc_race)
		{
			send_to_char ("That is not a valid player race.\n\r", ch);
			return;
		}
		victim->race = race;
		return;
	}
	if (!str_prefix (arg2, "oldrace"))
	{
		int race;
		if (IS_NPC (victim))
		{
			send_to_char ("Not on NPC's.\n\r", ch);
			return;
		}
		if (is_number(arg3))
		{
			race = atoi(arg3);
			if (race < 0 || race > 10)
			{
				send_to_char("That is not a valid race.\n\r",ch);
				return;
			}
			victim->pcdata->old_race = race;
			return;
		}
		race = race_lookup (arg3);
		if (race == 0)
		{
			send_to_char ("That is not a valid race.\n\r", ch);
			return;
		}
		if (!IS_NPC (victim) && !race_table[race].pc_race)
		{
			send_to_char ("That is not a valid player race.\n\r", ch);
			return;
		}
		victim->pcdata->old_race = race;
		return;
	}
	if (!str_cmp (arg2, "security"))
	{				/* OLC */
		if (IS_NPC (victim))
		{
			send_to_char ("Not on NPC's.\n\r", ch);
			return;
		}
		if (value > ch->pcdata->security || value < 0)
		{
			if (ch->pcdata->security != 0)
			{
				sprintf (buf, "Valid security is 0-%d.\n\r",
					ch->pcdata->security);
				send_to_char (buf, ch);
			}
			else
			{
				send_to_char ("Valid security is 0 only.\n\r", ch);
			}
			return;
		}
		victim->pcdata->security = value;
		return;
	}
	//Shinowlan -- 12-13-2001 -- buildall is a pcdata field that determines if a pc character can
	// edit any and all rooms, mobs, object, rooms, etc.  It is like a super "security" and a value
	// of 1 supercedes a character's security level.
	if (!str_cmp (arg2, "buildall"))
	{				/* OLC */
		if (IS_NPC (victim))
		{
			send_to_char ("Not on NPC's.\n\r", ch);
			return;
		}
		if (value > ch->pcdata->buildall || value < 0)
		{
			if (ch->pcdata->buildall != 0)
			{
				sprintf (buf, "Valid buildall is 0-%d.\n\r",
					ch->pcdata->buildall);
				send_to_char (buf, ch);
			}
			else
			{
				send_to_char ("Valid buildall is 0 only.\n\r", ch);
			}
			return;
		}
		victim->pcdata->buildall = value;
		return;
	}
	if (!str_prefix (arg2, "group"))
	{
		if (!IS_NPC (victim))
		{
			send_to_char ("Only on NPCs.\n\r", ch);
			return;
		}
		victim->group = value;
		return;
	}
	if (!str_prefix (arg2, "souls"))
	{
		if (IS_NPC (victim))
		{
			send_to_char ("Only on players...\n\r", ch);
			return;
		}
		victim->pcdata->souls = value;
		return;
	}
	if (!str_prefix (arg2, "totalsouls"))
	{
		if (IS_NPC (victim))
		{
			send_to_char ("Only on players...\n\r", ch);
			return;
		}
		victim->pcdata->totalsouls = value;
		return;
	}

	if (!str_prefix (arg2, "numattacks"))
	{
		if (!IS_NPC(victim))
		{
			send_to_char("Only on NPC's...\n\r",ch);
			return;
		}
		if (value <-1 || value > 100)
		{
			send_to_char("Values -1 to 100 acceptable (-1 = ignore this and do it the normal way.\n\r",ch);
			return;
		}
		victim->number_of_attacks = value;
		return;
	}

	if (!str_prefix (arg2, "totalxp"))
	{
		if (IS_NPC (victim))
		{
			send_to_char ("Only on players...\n\r", ch);
			return;
		}
		if (value < -1)
		{
			send_to_char ("Not a valid value...\n\r", ch);
			return;
		}
		victim->pcdata->totalxp = value;
		return;
	}
	if (!str_prefix (arg2, "gainedhp"))
	{
		if (value < 0 || value > 400)
		{
			send_to_char ("Gained hp range is 0 to 400.\n\r", ch);
			return;
		}
		if (!IS_NPC (victim))
			victim->pcdata->hp_gained = value;
		else send_to_char("Only on players...\n\r",ch);
		return;
	}
	if (!str_prefix (arg2, "gainedmana"))
	{
		if (value < 0 || value > 400)
		{
			send_to_char ("Gained mana range is 0 to 400.\n\r", ch);
			return;
		}
		if (!IS_NPC (victim))
			victim->pcdata->mana_gained = value;
		else send_to_char("Only on players...\n\r",ch);
		return;
	}
	if (!str_prefix (arg2, "gainedmove"))
	{
		if (value < 0 || value > 400)
		{
			send_to_char ("Gained move range is 0 to 400.\n\r", ch);
			return;
		}
		if (!IS_NPC (victim))
			victim->pcdata->move_gained = value;
		else send_to_char("Only on players...\n\r",ch);
		return;
	}

	/*
	* Generate usage message.
	*/
	do_mset (ch, "");
	return;
}

void do_string (CHAR_DATA * ch, char *argument)
{
	char type[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	if (ch->level < 91)
	{
		send_to_char("Huh?\n\r",ch);
		return;
	}
	smash_tilde (argument);
	argument = one_argument (argument, type);
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	strcpy (arg3, argument);
	if (type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0'
		|| arg3[0] == '\0')
	{
		send_to_char ("Syntax:\n\r", ch);
		send_to_char ("  string obj  <name> <field> <string>\n\r", ch);
		send_to_char ("    fields: name short long extended\n\r", ch);
		if (IS_IMMORTAL(ch))
		{
			send_to_char ("  string char <name> <field> <string>\n\r", ch);
			send_to_char ("    fields: name short long desc title spec\n\r", ch);
		}
		return;
	}
	if ((!str_prefix (type, "character") || !str_prefix (type, "mobile")) && IS_IMMORTAL(ch))
	{
		if ((victim = get_char_world (ch, arg1)) == NULL)
		{
			send_to_char ("They aren't here.\n\r", ch);
			return;
		}

		/* clear zone for mobs */
		victim->zone = NULL;

		/* string something */
		if (!str_prefix (arg2, "name"))
		{
			if (!IS_NPC (victim))
			{
				send_to_char ("Not on PC's.\n\r", ch);
				return;
			}
			free_string (victim->name);
			victim->name = str_dup (arg3);
			return;
		}
		if (!str_prefix (arg2, "description"))
		{
			free_string (victim->description);
			victim->description = str_dup (arg3);
			return;
		}
		if (!str_prefix (arg2, "short"))
		{
			free_string (victim->short_descr);
			victim->short_descr = str_dup (arg3);
			return;
		}
		if (!str_prefix (arg2, "long"))
		{
			free_string (victim->long_descr);
			strcat (arg3, "\n\r");
			victim->long_descr = str_dup (arg3);
			return;
		}
		if (!str_prefix (arg2, "title"))
		{
			if (IS_NPC (victim))
			{
				send_to_char ("Not on NPC's.\n\r", ch);
				return;
			}
			set_title (victim, arg3);
			return;
		}
		if (!str_prefix (arg2, "spec"))
		{
			if (!IS_NPC (victim))
			{
				send_to_char ("Not on PC's.\n\r", ch);
				return;
			}
			if ((victim->spec_fun = spec_lookup (arg3)) == 0)
			{
				send_to_char ("No such spec fun.\n\r", ch);
				return;
			}
			return;
		}
	}
	if (!str_prefix (type, "object"))
	{

		/* string an obj */
		if (ch->level > 90)
		{
			if ((obj = get_obj_here (ch, arg1)) == NULL)
			{
				send_to_char ("Nothing like that in this room.\n\r", ch);
				return;
			}
		}
		else
		{
			if ((obj = get_obj_carry (ch, arg1)) == NULL)
			{
				send_to_char ("You have nothing like that on you.\n\r", ch);
				return;
			}
		}
		if (obj->item_type == ITEM_BINDER || obj->pIndexData->vnum == OBJ_VNUM_ANTE_BAG)
		{
			send_to_char ("You cannot restring that.\n\r",ch);
			return;
		}
		if (!str_prefix (arg2, "name"))
		{
			free_string (obj->name);
			obj->name = str_dup (arg3);
			if (!IS_IMMORTAL(ch) && obj->plr_owner == NULL)
				obj->plr_owner = str_dup(ch->name);
			return;
		}
		if (!str_prefix (arg2, "short"))
		{
			free_string (obj->short_descr);
			obj->short_descr = str_dup (arg3);
			if (!IS_IMMORTAL(ch) && obj->plr_owner == NULL)
				obj->plr_owner = str_dup(ch->name);
			return;
		}
		if (!str_prefix (arg2, "long"))
		{
			free_string (obj->description);
			obj->description = str_dup (arg3);
			if (!IS_IMMORTAL(ch) && obj->plr_owner == NULL)
				obj->plr_owner = str_dup(ch->name);
			return;
		}
		if (!str_prefix (arg2, "ed") || !str_prefix (arg2, "extended"))
		{
			EXTRA_DESCR_DATA *ed;
			argument = one_argument (argument, arg3);
			if (argument == NULL)
			{
				send_to_char
					("Syntax: oset <object> ed <keyword> <string>\n\r", ch);
				return;
			}
			strcat (argument, "\n\r");
			ed = new_extra_descr ();
			ed->keyword = str_dup (arg3);
			ed->description = str_dup (argument);
			ed->next = obj->extra_descr;
			obj->extra_descr = ed;
			if (!IS_IMMORTAL(ch) && obj->plr_owner == NULL)
				obj->plr_owner = str_dup(ch->name);
			return;
		}
	}

	/* echo bad use message */
	do_string (ch, "");
}

void
rset_stock (CHAR_DATA * ch, ROOM_INDEX_DATA * location, int vnum, int cost)
{
	OBJ_DATA *obj, *shelf, *obj_walk;
	OBJ_INDEX_DATA *obj_index;
	int found = FALSE;
	if (location->owner == NULL || location->owner[0] == '\0'
		|| !IS_SET (location->race_flags, ROOM_PLAYERSTORE))
	{
		send_to_char ("That room is not a player store.\n\r", ch);
		return;
	}
	obj_index = get_obj_index (vnum);
	if (obj_index == NULL)
	{
		send_to_char ("Object vnum does not exist.\n\r", ch);
		return;
	}
	obj = create_object (obj_index, 0);
	if (obj == NULL)
	{
		send_to_char ("Sorry, the store's shelves seem to be broken.\n\r", ch);
		return;
	}
	obj_index = get_obj_index (OBJ_VNUM_SHELF);
	if (obj_index == NULL)
	{
		send_to_char ("Sorry, the store's shelves seem to be broken.\n\r", ch);
		return;
	}
	shelf = create_object (obj_index, 0);
	if (shelf == NULL)
	{
		send_to_char ("Sorry, the store's shelves seem to be broken.\n\r", ch);
		return;
	}
	load_shelf (shelf, location->vnum);
	for (obj_walk = shelf->contains; obj_walk != NULL;
		obj_walk = obj_walk->next_content)
	{
		if (obj_walk->pIndexData->vnum == vnum)
		{
			found = TRUE;
			break;
		}
	}
	if (found)
	{
		obj_from_obj (obj_walk);
		extract_obj (obj_walk);
		send_to_char ("Object has been removed from store inventory.\n\r", ch);
	}
	else
	{
		if (!IS_SET (obj->extra_flags[0], ITEM_PERMSTOCK))
			SET_BIT (obj->extra_flags[0], ITEM_PERMSTOCK);
		if (cost != -1)
			obj->cost = cost;
		obj_to_obj (obj, shelf);
		send_to_char ("Object has been added to store inventory.\n\r", ch);
	}
	save_shelf (shelf, location->vnum);
	extract_obj (obj);
	extract_obj (shelf);
}

void save_shelf (OBJ_DATA * shelf, int vnum)
{
	char tcbuf[MAX_STRING_LENGTH];
	FILE *fp;
	sprintf (tcbuf, "stock/%d", vnum);
	if (shelf->contains == NULL)
	{
		unlink (tcbuf);
		return;
	}
	mkdir ("stock/", 0755);
	if ((fp = fopen (tcbuf, "w")) == NULL)
		return;
	fwrite_objdb (shelf->contains, fp, 0);
	fclose (fp);
}

void load_shelf (OBJ_DATA * shelf, int vnum)
{
	char tcbuf[MAX_STRING_LENGTH];
	FILE *fp;
	sprintf (tcbuf, "stock/%d", vnum);
	if ((fp = fopen (tcbuf, "r")) == NULL)
		return;
	while (!feof (fp) && !str_cmp (fread_word (fp), "#O"))
		fread_objdb (shelf, fp);
	fclose (fp);
}

void do_rset (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	int value;
	smash_tilde (argument);
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	argument = one_argument (argument, arg3);

	/*   strcpy(arg3, argument); */
	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
	{
		send_to_char ("Syntax:\n\r", ch);
		send_to_char
			("  set room <location> <field> <value> [stock-cost]\n\r", ch);
		send_to_char ("  Field being one of:\n\r", ch);
		send_to_char ("    flags sector stock\n\r", ch);
		return;
	}
	if ((location = find_location (ch, arg1)) == NULL)
	{
		send_to_char ("No such location.\n\r", ch);
		return;
	}
	if (!is_room_owner (ch, location) && ch->in_room != location
		&& room_is_private (location) && !IS_TRUSTED (ch, OVERLORD))
	{
		send_to_char ("That room is private right now.\n\r", ch);
		return;
	}

	/*
	* Snarf the value.
	*/
	if (!is_number (arg3))
	{
		send_to_char ("Value must be numeric.\n\r", ch);
		return;
	}
	value = atol (arg3);

	/*
	* Set something.
	*/
	if (!str_prefix (arg2, "flags"))
	{
		location->room_flags = value;
		return;
	}
	if (!str_prefix (arg2, "sector"))
	{
		location->sector_type = value;
		return;
	}
	if (!str_prefix (arg2, "stock"))
	{
		char myarg[MAX_INPUT_LENGTH];
		int value2;
		one_argument (argument, myarg);
		if (myarg[0] == '\0')
			value2 = -1;

		else
			value2 = atol (myarg);
		rset_stock (ch, location, value, value2);
		return;
	}

	/*
	* Generate usage message.
	*/
	do_rset (ch, "");
	return;
}

void do_sockets (CHAR_DATA * ch, char *argument)
{
	char buf[2 * MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	int count;
	count = 0;
	buf[0] = '\0';
	one_argument (argument, arg);
	for (d = descriptor_list; d != NULL; d = d->next)
	{
		if (d->character != NULL && can_see (ch, d->character)
			&& (arg[0] == '\0' || is_name (arg, d->character->name)
			|| (d->original && is_name (arg, d->original->name))))
		{
			count++;
			sprintf (buf + strlen (buf), "[%3d %7d %2d] %s@%s\n\r",
				d->descriptor, d->port, d->connected,
				d->original ? d->original->name : d->character ? d->
				character->name : "(none)", d->host);
		}
	}
	if (count == 0)
	{
		send_to_char ("No one by that name is connected.\n\r", ch);
		return;
	}
	sprintf (buf2, "%d user%s\n\r", count, count == 1 ? "" : "s");
	strcat (buf, buf2);
	page_to_char (buf, ch);
	return;
}


// New Sockets command, Added by Morgan on
// June 24. 2000
void do_newsockets (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *vch;
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	int count;
	char *st;
	char s[100];
	char idle[10];
	count = 0;
	buf[0] = '\0';
	buf2[0] = '\0';
	strcat (buf2, "\n\r[Num Connected_State Login@ Idl] Player Name Host\n\r");
	strcat (buf2,
		"--------------------------------------------------------------------------\n\r");
	for (d = descriptor_list; d; d = d->next)
	{
		if (d->character && can_see (ch, d->character))
		{

			/* NB: You may need to edit the CON_ values */
			switch (d->connected)
			{
			case CON_PLAYING:
				st = "    Playing    ";
				break;
			case CON_GET_NAME:
				st = "   Get Name    ";
				break;
			case CON_GET_OLD_PASSWORD:
				st = "Get Old Passwd ";
				break;
			case CON_CONFIRM_NEW_NAME:
				st = " Confirm Name  ";
				break;
			case CON_GET_NEW_PASSWORD:
				st = "Get New Passwd ";
				break;
			case CON_CONFIRM_NEW_PASSWORD:
				st = "Confirm Passwd ";
				break;
			case CON_GET_NEW_RACE:
				st = "  Get New Race ";
				break;
			case CON_GET_NEW_SEX:
				st = "  Get New Sex  ";
				break;
			case CON_GET_NEW_CLASS:
				st = " Get New Class ";
				break;
			case CON_GET_ALIGNMENT:
				st = " Get Alignment ";
				break;
			case CON_DEFAULT_CHOICE:
				st = " Choose Groups ";
				break;
			case CON_GEN_GROUPS:
				st = " Choose Groups ";
				break;
			case CON_PICK_WEAPON:
				st = " Choose weapon ";
				break;
			case CON_READ_IMOTD:
				st = " Reading IMOTD ";
				break;
			case CON_READ_MOTD:
				st = "  Reading MOTD ";
				break;
			case CON_BREAK_CONNECT:
				st = "Broken Connect ";
				break;
			case CON_GET_EMAIL:
				st = " Getting Email ";
				break;
			case CON_GET_ANSI:
				st = " Choosing Ansi ";
				break;
			case CON_PICK_HAND:
				st = " Picking Hand  ";
				break;
			case CON_CONFIRM_STATS:
				st = " Choosing Stats";
				break;
			default:
				st = "   !UNKNOWN!   ";
				break;
			}
			count++;

			/* Format "login" value... */
			vch = d->original ? d->original : d->character;
			strftime (s, 100, "%I:%M%p", localtime (&vch->logon));
			if (vch->level <= LEVEL_HERO && vch->timer > 0)
				sprintf (idle, "%-2d", vch->timer);

			else
				sprintf (idle, "  ");
			sprintf (buf, "[%3d %s %7s %2s] %-12s %-32.32s\n\r",
				d->descriptor, st, s, idle,
				(d->original) ? d->original->name : (d->
				character) ?
				d->character->name : "(None!)", d->host);
			strcat (buf2, buf);
		}
	}
	sprintf (buf, "\n\r%d user%s\n\r", count, count == 1 ? "" : "s");
	strcat (buf2, buf);
	send_to_char (buf2, ch);
	return;
}


/*
* Thanks to Grodyn for pointing out bugs in this function.
*/
void do_force (CHAR_DATA * ch, char *argument)
{

	/*   DESCRIPTOR_DATA *original; */
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	if (IS_NPC (ch) && ch->pIndexData->vnum != MOB_VNUM_FAKIE)
		return;
	if (!IS_NPC(ch) &&  ch->desc == NULL)
		return;
	argument = one_argument (argument, arg);
	if (arg[0] == '\0' || argument[0] == '\0')
	{
		send_to_char ("Force whom to do what?\n\r", ch);
		if (!IS_NPC(ch))
			ch->desc->incomm[0] = '\0';
		return;
	}
	one_argument (argument, arg2);
	if (!str_cmp (arg2, "delete"))
	{
		send_to_char ("That will NOT be done.\n\r", ch);
		if (!IS_NPC(ch))
			ch->desc->incomm[0] = '\0';
		return;
	}
	sprintf (buf, "$n forces you to '%s'.", argument);
	if (!str_cmp (arg, "all"))
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;
		if (get_trust (ch) < MAX_LEVEL - 3)
		{
			send_to_char ("Not at your level!\n\r", ch);
			if (!IS_NPC(ch))
				ch->desc->incomm[0] = '\0';
			return;
		}
		for (vch = char_list; vch != NULL; vch = vch_next)
		{
			vch_next = vch->next;
			if (!IS_NPC (vch) && get_trust (vch) < get_trust (ch))
			{
				if ((!IS_NPC(ch) || ch->pIndexData->vnum != MOB_VNUM_FAKIE) && !IS_IMMORTAL(vch))
					act (buf, ch, NULL, vch, TO_VICT);
				interpret (vch, argument);
			}
		}
	}
	else if (!str_cmp (arg, "players"))
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;
		if (get_trust (ch) < MAX_LEVEL - 2)
		{
			send_to_char ("Not at your level!\n\r", ch);
			if (!IS_NPC(ch))
				ch->desc->incomm[0] = '\0';
			return;
		}
		for (vch = char_list; vch != NULL; vch = vch_next)
		{
			vch_next = vch->next;
			if (!IS_NPC (vch) && get_trust (vch) < get_trust (ch)
				&& vch->level < LEVEL_HERO)
			{
				if ((!IS_NPC(ch) || ch->pIndexData->vnum != MOB_VNUM_FAKIE) && !IS_IMMORTAL(vch))
					act (buf, ch, NULL, vch, TO_VICT);
				interpret (vch, argument);
			}
		}
	}
	else if (!str_cmp (arg, "gods"))
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;
		if (get_trust (ch) < MAX_LEVEL - 2)
		{
			send_to_char ("Not at your level!\n\r", ch);
			if (!IS_NPC(ch))
				ch->desc->incomm[0] = '\0';
			return;
		}
		for (vch = char_list; vch != NULL; vch = vch_next)
		{
			vch_next = vch->next;
			if (!IS_NPC (vch) && get_trust (vch) < get_trust (ch)
				&& vch->level >= LEVEL_HERO)
			{
				if ((!IS_NPC(ch) || ch->pIndexData->vnum != MOB_VNUM_FAKIE) && !IS_IMMORTAL(vch))
					act (buf, ch, NULL, vch, TO_VICT);
				interpret (vch, argument);
			}
		}
	}
	else
	{
		CHAR_DATA *victim;
		if ((victim = get_char_world (ch, arg)) == NULL)
		{
			send_to_char ("They aren't here.\n\r", ch);
			if (!IS_NPC(ch))
				ch->desc->incomm[0] = '\0';
			return;
		}
		if (victim == ch)
		{
			send_to_char ("Aye aye, right away!\n\r", ch);
			if (!IS_NPC(ch))
				ch->desc->incomm[0] = '\0';
			return;
		}
		if (!is_room_owner (ch, victim->in_room)
			&& ch->in_room != victim->in_room
			&& room_is_private (victim->in_room) && !IS_TRUSTED (ch, OVERLORD))
		{
			send_to_char ("That character is in a private room.\n\r", ch);
			if (!IS_NPC(ch))
				ch->desc->incomm[0] = '\0';
			return;
		}
		if (get_trust (victim) >= get_trust (ch))
		{
			sprintf (buf, "$n tries to force you to '%s'.", argument);
			act (buf, ch, NULL, victim, TO_VICT);
			send_to_char ("Do it yourself!\n\r", ch);
			if (!IS_NPC(ch))
				ch->desc->incomm[0] = '\0';
			return;
		}
		if (!IS_NPC (victim) && get_trust (ch) < MAX_LEVEL - 3)
		{
			send_to_char ("Not at your level!\n\r", ch);
			if (!IS_NPC(ch))
				ch->desc->incomm[0] = '\0';
			return;
		}
		if ((!IS_NPC(ch) || ch->pIndexData->vnum != MOB_VNUM_FAKIE) && !IS_IMMORTAL(victim))
			act (buf, ch, NULL, victim, TO_VICT);
		interpret (victim, argument);
	}
	if (!IS_NPC(ch))
		ch->desc->incomm[0] = '\0';
	send_to_char ("Ok.\n\r", ch);
	return;
}


/*
* New routines by Dionysos.
*/
void do_invis (CHAR_DATA * ch, char *argument)
{
	int level;
	char arg[MAX_STRING_LENGTH];

	/* RT code for taking a level argument */
	one_argument (argument, arg);
	if (arg[0] == '\0')

		/* take the default path */
		if (ch->invis_level)
		{
			ch->invis_level = 0;
			act ("$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM);
			send_to_char ("You slowly fade back into existence.\n\r", ch);
		}
		else
		{
			act ("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
			send_to_char ("You slowly vanish into thin air.\n\r", ch);
			ch->invis_level = get_trust (ch);
		}
	else
		/* do the level thing */
	{
		level = atol (arg);
		if (level < 2 || level > get_trust (ch))
		{
			send_to_char
				("Invis level must be between 2 and your level.\n\r", ch);
			return;
		}
		else
		{

			/* This function needs changed badly */
			act ("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
			send_to_char ("You slowly vanish into thin air.\n\r", ch);
			ch->reply = NULL;
			ch->invis_level = level;
		}
	}
	return;
}

void do_incognito (CHAR_DATA * ch, char *argument)
{
	int level;
	char arg[MAX_STRING_LENGTH];

	/* RT code for taking a level argument */
	one_argument (argument, arg);
	if (arg[0] == '\0')

		/* take the default path */
		if (ch->incog_level)
		{
			ch->incog_level = 0;
			act ("$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM);
			send_to_char ("You are no longer cloaked.\n\r", ch);
		}
		else
		{
			ch->incog_level = get_trust (ch);
			act ("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
			send_to_char ("You cloak your presence.\n\r", ch);
		}
	else
		/* do the level thing */
	{
		level = atol (arg);
		if (level < 2 || level > get_trust (ch))
		{
			send_to_char
				("Incog level must be between 2 and your level.\n\r", ch);
			return;
		}
		else
		{
			ch->reply = NULL;
			ch->incog_level = level;
			act ("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
			send_to_char ("You cloak your presence.\n\r", ch);
		}
	}
	return;
}

void do_hero (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim = NULL;
	char arg[MAX_INPUT_LENGTH];
	if (IS_NPC (ch))
		return;
	argument = one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Make who a hero?\n\r", ch);
		return;
	}
	victim = get_char_world (ch, arg);
	if (victim == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	do_make_hero (ch, victim);
	return;
}

void do_unhero (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim = NULL;
	char arg[MAX_INPUT_LENGTH];
	if (IS_NPC (ch))
		return;
	argument = one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Demote which hero?\n\r", ch);
		return;
	}
	victim = get_char_world (ch, arg);
	if (victim == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	do_unmake_hero (ch, victim);
	return;
}

void do_holylight (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;
	if (IS_SET (ch->act, PLR_HOLYLIGHT))
	{
		REMOVE_BIT (ch->act, PLR_HOLYLIGHT);
		send_to_char ("Holy light mode off.\n\r", ch);
	}
	else
	{
		SET_BIT (ch->act, PLR_HOLYLIGHT);
		send_to_char ("Holy light mode on.\n\r", ch);
	}
	return;
}


/* prefix command: it will put the string typed on each line typed */
void do_prefi (CHAR_DATA * ch, char *argument)
{
	send_to_char ("You cannot abbreviate the prefix command.\r\n", ch);
	return;
}

void do_prefix (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	if (argument[0] == '\0')
	{
		if (ch->prefix[0] == '\0')
		{
			send_to_char ("You have no prefix to clear.\r\n", ch);
			return;
		}
		send_to_char ("Prefix removed.\r\n", ch);
		free_string (ch->prefix);
		ch->prefix = str_dup ("");
		return;
	}
	if (ch->prefix[0] != '\0')
	{
		sprintf (buf, "Prefix changed to %s.\r\n", argument);
		free_string (ch->prefix);
	}
	else
	{
		sprintf (buf, "Prefix set to %s.\r\n", argument);
	}
	ch->prefix = str_dup (argument);
}

void do_auto_shutdown (int signo)
{
	FILE *fp;
	extern bool merc_down;
	DESCRIPTOR_DATA *d, *d_next;
	merc_down = TRUE;

	/* This is to write to the file. */
	fclose (fpReserve);
	if ((fp = fopen (LAST_COMMAND, "a")) == NULL)
		bug ("Error in do_auto_save opening last_command.txt", 0);
	fprintf (fp, "Last Command: %s\n", last_command);
	fclose (fp);
	fpReserve = fopen (NULL_FILE, "r");
	for (d = descriptor_list; d != NULL; d = d_next)
	{
		if (d->connected == CON_PLAYING && d->character != NULL)
		{

			//         send_to_char
			//          ("Sorry, we encountered a crash bug, relog in in a few minutes.\n\r",
			//          d->character);
			//        do_save (d->character, "");
			close_socket (d);
		}
		d_next = d->next;

		//     for (1;0==1;);
	}
	exit (-1);
	return;
}

void do_whomsg (CHAR_DATA * ch, char *argument)
{
	if (strlen (argument) > 0)
	{
		if (!strcmp (argument, "remove"))
		{
			strcpy (imm_who_msg, "remove");
			send_to_char ("The immortal who message has been removed.\r\n", ch);
		}
		else
		{

			if (strlen (argument) > MAX_STRING_LENGTH - 8)
				strncpy (imm_who_msg, argument, MAX_STRING_LENGTH - 8);	//the 8 is the "[/] " and " [\]"
			else
				strcpy (imm_who_msg, argument);

			send_to_char
				("The immortal who message has been changed as requested.\r\n",
				ch);
		}
	}
	else
	{
		send_to_char
			("Set the immortal who message to what? (whomsg remove to remove it)\r\n",
			ch);
	}
}

//Iblis - allows one to stat a character's reaver sword via 'stat char sword'
void do_swordstat (CHAR_DATA * ch, CHAR_DATA * victim)
{
	char buf[MAX_STRING_LENGTH];
	int i;
	OBJ_DATA *obj;
	long cost_power;
	long cost_hatred;
	long cost_fury;
	long cost_strength;
	long cost_shocking_blade;
	long cost_frost_blade;
	long cost_flaming_blade;
	long cost_chaotic_blade;
	long cost_venemous_blade;
	long cost_vampiric_blade;
	long cost_lightning_burst;
	long cost_ice_burst;
	long cost_fire_burst;
	long cost_unholy_burst;
	long cost_hellscape_burst;

	if (victim->pcdata->souls == 1)
	{				/*I'm a perfectionist ok? */
		send_to_char
			("`aThere is currently `b1 `asoul energy stored in the blade.``\n\r",
			ch);
		return;
	}
	sprintf (buf,
		"`aThere are currently `b%ld `asoul energies stored in the blade.``\n\r",
		victim->pcdata->souls);
	send_to_char (buf, ch);
	for (i = 0; i < MAX_BLADE_SPELLS; i++)
	{
		if (victim->sword->bs_capacity[i] > 0)
		{
			sprintf (buf,
				"`aThey have `b%d `aout of a possible `b%d `aspells of %s blade.``\n\r",
				victim->sword->bs_charges[i],
				victim->sword->bs_capacity[i], get_bs_name (i));
			send_to_char (buf, ch);
		}
	}
	for (i = 0; i < MAX_BURST; i++)
	{
		if (victim->sword->bb_capacity[i] > 0)
		{
			sprintf (buf,
				"`aThey have `b%d `aout of a possible `b%d `aspells of %s.``\n\r",
				victim->sword->bb_charges[i],
				victim->sword->bb_capacity[i], get_bb_name (i));
			send_to_char (buf, ch);
		}
	}
	cost_power = get_next_cost (victim->sword, BLADE_POWER);
	cost_hatred = get_next_cost (victim->sword, BLADE_HATRED);
	cost_fury = get_next_cost (victim->sword, BLADE_FURY);
	cost_strength = get_next_cost (victim->sword, BLADE_STRENGTH);
	cost_shocking_blade = get_slot_cost (victim->sword, BLADE_SPELL_SHOCKING);
	cost_frost_blade = get_slot_cost (victim->sword, BLADE_SPELL_FROST);
	cost_flaming_blade = get_slot_cost (victim->sword, BLADE_SPELL_FLAMING);
	cost_chaotic_blade = get_slot_cost (victim->sword, BLADE_SPELL_CHAOTIC);
	cost_venemous_blade = get_slot_cost (victim->sword, BLADE_SPELL_VENEMOUS);
	cost_vampiric_blade = get_slot_cost (victim->sword, BLADE_SPELL_VAMPIRIC);
	cost_lightning_burst = get_burst_cost (victim->sword, BURST_LIGHTNING);
	cost_ice_burst = get_burst_cost (victim->sword, BURST_ICE);
	cost_fire_burst = get_burst_cost (victim->sword, BURST_FIRE);
	cost_unholy_burst = get_burst_cost (victim->sword, BURST_UNHOLY);
	cost_hellscape_burst = get_burst_cost (victim->sword, BURST_HELLSCAPE);
	send_to_char ("\r\n`aUpgrades available       Cost``\n\r", ch);
	sprintf (buf, "  `ePower               `b%6ld`hSE``\n\r", cost_power);
	send_to_char (buf, ch);
	sprintf (buf, "  `eHatred              `b%6ld`hSE``\n\r", cost_hatred);
	send_to_char (buf, ch);
	sprintf (buf, "  `eFury                `b%6ld`hSE``\n\r", cost_fury);
	send_to_char (buf, ch);
	sprintf (buf, "  `eStrength            `b%6ld`hSE``\n\r", cost_strength);
	send_to_char (buf, ch);
	if (victim->level >= 40 && !IS_WEAPON_STAT (victim->sword, WEAPON_LANCE))
	{
		sprintf (buf, "  `eKnightsblade        `b%6d`hSE``\n\r", 10000);
		send_to_char (buf, ch);
	}
	if (victim->level >= 7)
	{
		sprintf (buf, "  `eShocking Blade      `b%6ld`hSE``\n\r",
			cost_shocking_blade);
		send_to_char (buf, ch);
	}
	if (victim->level >= 21)
	{
		sprintf (buf, "  `eFrost Blade         `b%6ld`hSE``\n\r",
			cost_frost_blade);
		send_to_char (buf, ch);
	}
	if (victim->level >= 35)
	{
		sprintf (buf, "  `eFlaming Blade       `b%6ld`hSE``\n\r",
			cost_flaming_blade);
		send_to_char (buf, ch);
	}
	if (victim->level >= 49)
	{
		sprintf (buf, "  `eChaotic Blade       `b%6ld`hSE``\n\r",
			cost_chaotic_blade);
		send_to_char (buf, ch);
	}
	if (victim->level >= 63)
	{
		sprintf (buf, "  `eVenomous Blade      `b%6ld`hSE``\n\r",
			cost_venemous_blade);
		send_to_char (buf, ch);
	}
	if (victim->level >= 70)
	{
		sprintf (buf, "  `eVampiric Blade      `b%6ld`hSE``\n\r",
			cost_vampiric_blade);
		send_to_char (buf, ch);
	}
	if (victim->level >= 14)
	{
		sprintf (buf, "  `eLightning Burst     `b%6ld`hSE``\n\r",
			cost_lightning_burst);
		send_to_char (buf, ch);
	}
	if (victim->level >= 28)
	{
		sprintf (buf, "  `eIce Burst           `b%6ld`hSE``\n\r",
			cost_ice_burst);
		send_to_char (buf, ch);
	}
	if (victim->level >= 42)
	{
		sprintf (buf, "  `eFire Burst          `b%6ld`hSE``\n\r",
			cost_fire_burst);
		send_to_char (buf, ch);
	}
	if (victim->level >= 56)
	{
		sprintf (buf, "  `eUnholy Burst        `b%6ld`hSE``\n\r",
			cost_unholy_burst);
		send_to_char (buf, ch);
	}
	if (victim->level >= 77)
	{
		sprintf (buf, "  `eHellscape           `b%6ld`hSE``\n\r",
			cost_hellscape_burst);
		send_to_char (buf, ch);
	}
	obj = victim->sword;
	sprintf (buf, "\r\n`jName(s):`` %s\n\r", obj->name);
	send_to_char (buf, ch);
	sprintf (buf, "`jDamage is`` %dd%d `j(average`` %d`j)``\n\r",
		obj->value[1], obj->value[2],
		(1 + obj->value[2]) * obj->value[1] / 2);
	send_to_char (buf, ch);
	sprintf (buf,
		"`jShort description:`` %s\n\r`jLong description:`` %s\n\r",
		obj->short_descr, obj->description);
	send_to_char (buf, ch);
	sprintf (buf, "`jDamage noun is`` %s.\n\r",
		attack_table[obj->value[3]].noun);
	send_to_char (buf, ch);
	if (obj->value[4])
	{				/* weapon flags */
		sprintf (buf, "`jWeapons flags:`` %s\n\r",
			weapon_bit_name (obj->value[4]));
		send_to_char (buf, ch);
	}
	if (obj->value[0] == WEAPON_DICE)
	{
		sprintf (buf, "`jWeapon spell:`` %s\n\r",skill_table[obj->value[5]].name);
		send_to_char(buf,ch);
	}
}

//Iblis - Starts Battle Royale, a giant free-for-all PK event
//        Beware, everyone under level 20, clanned or not, is drawn into the event.  
//        The event is restricted to only a few areas, such as thesden, prarie, forest, etc.
//        Restricted areas (besides player rooms) cannot be entered for the duration.  Players
//        in these areas upon the start are transfered to a room surrounding DSC.
//        Players may enter player rooms, but on tick are transfered to a room surrounding
//        DSC.  
//        At the conclusion, the victor is announced, note posted, and players are transfered to
//        a safe room.
void do_battle_royale (CHAR_DATA * ch, char *argument)
{

	extern bool battle_royale;
	int time;
	DESCRIPTOR_DATA *d_next;
	DESCRIPTOR_DATA *d;
	char arg1[MAX_STRING_LENGTH];
	argument = one_argument(argument,arg1);

	if (arg1[0] == '\0' || argument[0] == '\0' || !is_number(argument) || !is_number(arg1))
	{
		send_to_char("Syntax: Battle_Royale TICKLENGTH number_of_people_dead_before_Altar_restore\n\r",ch);
		return;
	}
	send_to_char ("Battle Royale is now turned ON.\n\r", ch);
	do_echo (ch,
		"`k -+-+- `oThe drums of war sound all around you.  `iBATTLE ROYALE`o has been declared! `k-+-+-``\n\r");
	battle_royale = TRUE;
	strcpy (imm_who_msg,
		"`iBattle Royale`o is going on so you might be attacked.  `kHELP BATTLE ROYALE``");
	time = atol (arg1);
	battle_royale_timer = time;
	battle_royale_dead_amount = atol (argument);
	do_restore (ch, "all");

	for (d = descriptor_list; d != NULL; d = d_next)
	{
		d_next = d->next;
		if (d->character == NULL || d->connected != CON_PLAYING)
			continue;
		if (is_dueling (d->character))
			duel_ends (d->character);
		if (d == NULL || d->character == NULL)
			continue;
		if (d->character->master != NULL)
			stop_follower (d->character);
		d->character->dueler = FALSE;
		if (!IS_IMMORTAL (d->character)
			&& (d->character->level > 19 || (!IS_NPC(d->character) || d->character->pcdata->loner))
			&&
			((d->character->in_room->vnum > 24999
			&& d->character->in_room->vnum < 27000)
			|| (!(br_vnum_check (d->character->in_room->vnum)))))
			br_transfer (d->character);
	}
}

//Iblis - Simple function that transfers a person to a room surrounding DSC
void br_transfer (CHAR_DATA * ch)
{
	int dvnum;
	char buf[MAX_STRING_LENGTH];

	//currently tele-transfers you to a square in devon square center
	dvnum = number_range (0, 8) + 4048;
	sprintf (buf, "%s %d", ch->name, dvnum);
	do_transfer (ch, buf);
}

//Iblis - An outdated function used when we changed it so people could not gain 300 hp/m/mv, but only
//400 total, 300 max in a particular one.  This function fixed their hp/m/mv and reimbed them xp.
void do_newfix (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	long value;
	CHAR_DATA *victim;
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	strcpy (arg3, argument);
	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
	{
		send_to_char ("Syntax:\n\r", ch);
		send_to_char ("  newfix <charname> <field> <value>\n\r", ch);
		send_to_char ("  Field being one of:\n\r", ch);
		send_to_char ("   hp mana move\n\r", ch);
		send_to_char
			("  Field is the amount of mana/hp/move to properly SUBTRACT from perm_hp, max_hp, and hp_gained) etc\n\r",
			ch);
	}
	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("NOT ON MOBILES YOU #@$#ing IDIOT.\n\r", ch);
		return;
	}
	value = is_number (arg3) ? atol (arg3) : 0;
	if (value == 0)
	{
		send_to_char
			("You need to put in a number (and a number that is not 0).\n\r", ch);
		return;
	}
	if (!str_prefix (arg2, "mana"))
	{
		if (value < 0 || value > 300)
		{
			send_to_char ("Mana range is 0 to 300 mana points.\n\r", ch);
			return;
		}
		victim->max_mana -= value;
		victim->pcdata->perm_mana -= value;
		victim->pcdata->mana_gained -= value;
		do_restore (ch, victim->name);
		victim->exp += value * 1000000;
		return;
	}
	if (!str_prefix (arg2, "hp"))
	{
		if (value < 0 || value > 300)
		{
			send_to_char ("HP range is 0 to 300 mana points.\n\r", ch);
			return;
		}
		victim->max_hit -= value;
		victim->pcdata->perm_hit -= value;
		victim->pcdata->hp_gained -= value;
		victim->exp += value * 1000000;
		do_restore (ch, victim->name);
		return;
	}
	if (!str_prefix (arg2, "move"))
	{
		if (value < 0 || value > 300)
		{
			send_to_char ("Move range is 0 to 300 mana points.\n\r", ch);
			return;
		}
		victim->max_move -= value;
		victim->pcdata->perm_move -= value;
		victim->pcdata->move_gained -= value;
		do_restore (ch, victim->name);
		victim->exp += value * 1000000;
		return;
	}
	send_to_char ("Syntax:\n\r", ch);
	send_to_char ("  newfix char <name> <field> <value>\n\r", ch);
	send_to_char ("  Field being one of:\n\r", ch);
	send_to_char ("   hp mana move\n\r", ch);
	send_to_char
		("  Field is the amount of mana/hp/move to properly SUBTRACT from perm_hp, max_hp, and hp_gained) etc\n\r",
		ch);
}

//Iblis - Turns on nogate for the ENTIRE mud (useful for quests, etc)
void do_nogateall(CHAR_DATA* ch, char* argument)
{
	if (argument[0] == '\0')
	{
		send_to_char("Nogate/teleport/portal/summon/evocation is `b",ch);
		send_to_char( (nogate ? "on" : "off"), ch);
		send_to_char("``.\n\rIf you want to toggle it, type \"nogate putsomethinghere\"\n\r",ch);
		return;
	}
	if (nogate)
		nogate = 0;
	else nogate = 1;
	send_to_char("Nogate/teleport/portal/summon/evocation is now ",ch);
	send_to_char((nogate?"`ion.``\n\r":"`ioff.``\n\r"),ch);
}

//Iblis - Turns on nogate for a specific players
void do_nogate(CHAR_DATA* ch, char* argument)
{
	CHAR_DATA *victim;
	if (argument[0] == '\0')
	{
		send_to_char("Nogate whom? ",ch);
		return;
	}
	if ((victim = get_char_world (ch, argument)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC(victim))
	{
		send_to_char("You can't nogate an NPC Mr. Smarty Pants.\n\r",ch);
		return;
	}
	if (victim->pcdata->nogate)
		victim->pcdata->nogate = FALSE;
	else victim->pcdata->nogate = TRUE;
	send_to_char("Nogate for them is not set to ",ch);
	send_to_char((nogate?"`ion.``\n\r":"`ioff.``\n\r"),ch);
}




//Iblis 11/14/03 - Project 36 
//Damages a player/mob a specific amount, echoing the specific message to them
void do_damage(CHAR_DATA* ch, char* argument)
{
	char arg1[MAX_INPUT_LENGTH],  arg2[MAX_INPUT_LENGTH];//,  arg3[MAX_INPUT_LENGTH];
	int dam = 0;
	bool all = FALSE;
	CHAR_DATA *jch, *victim=NULL;
	if (!IS_NPC(ch) && !IS_IMMORTAL(ch))
	{
		send_to_char("Huh?\n\r",ch);
		return;
	}
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("Syntax : damage target amount damagemessage\n\r",ch);
		return;
	}

	if (!str_cmp(arg1,"all"))
		all = TRUE;
	else if ((victim = get_char_room (ch, arg1)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (!is_number(arg2))
	{
		send_to_char("Syntax : damage target amount damagemessage\n\r",ch);
		return;
	}
	crit_strike_possible = FALSE;
	dam = atoi(arg2);
	dam = adjust_damage(dam);
	if (!all)
	{
		send_to_char(argument,victim);
		send_to_char("\n\r",victim);
		damage(victim, victim, dam, 0, DAM_UNIQUE, FALSE);
	}
	else
	{
		if (!ch->in_room)
		{
			send_to_char("ERROR you're not in a room!\n\r",ch);
			crit_strike_possible = TRUE;
			return;
		}
		for (jch = ch->in_room->people;jch != NULL; jch = jch->next_in_room)
		{
			if (IS_IMMORTAL(jch) || jch == ch)
				continue;
			send_to_char(argument,jch);
			send_to_char("\n\r",jch);
			damage(jch,jch, dam, 0, DAM_UNIQUE, FALSE);
		}
	}
	crit_strike_possible = TRUE;
}

//Iblis - Target player is nochanned essentially, while access to Rant is still allowed
void do_rantonly (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Rantonly whom?", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (get_trust (victim) >= get_trust (ch))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	// Added by Morgan on June 25. 2000
	// Extra check for setting notell on mobs
	if (IS_NPC (victim))
	{
		send_to_char ("Setting Rantonly on mobs is pointless.\n\r", ch);
		return;
	}
	if (IS_SET (victim->comm2, COMM_RANTONLY))
	{
		REMOVE_BIT (victim->comm2, COMM_RANTONLY);
		send_to_char ("You can do more than Rant now.\n\r", victim);
		send_to_char ("Rantonly removed.\n\r", ch);
		sprintf (buf, "$N removes %s's Rantonly.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else
	{
		SET_BIT (victim->comm2, COMM_RANTONLY);
		send_to_char ("You can only use Rant now!\n\r", victim);
		send_to_char ("Rantonly set.\n\r", ch);
		sprintf (buf, "$N sets %s's Rantonly.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	return;
}

//Iblis - allows a mortal to start double experience for 3 nh points
void do_startdouble(CHAR_DATA *ch,char *argument)
{
	char buf[MAX_STRING_LENGTH];
	extern bool doub;
	extern bool quad;
	extern bool half;

	if (IS_NPC(ch))
		return;
	if (ch->pcdata->nhpoints < 3)
	{
		send_to_char("It costs 3 nh points to initiate a double.  You do not have enough.\n\r",ch);
		return;
	}
	if (half || doub || quad)
	{
		send_to_char("Not while another experience modifier is currently running.\n\r",ch);
		return;
	}
	ch->pcdata->nhpoints -= 3;
	do_double_xp(ch,"3");
	sprintf(buf,"This double brought to you by -> %s",ch->name);
	do_echo(ch,buf);
}

//Iblis - Allows a mortal to contribute to the length of double with 1-3 nh points
void do_contribute(CHAR_DATA *ch,char *argument)
{
	int time;
	extern bool doub; 
	if (!doub)
	{
		send_to_char("You cannot contribute because double experience is not currently on!\n\r",ch);
		return;
	}
	if (!is_number (argument))
	{
		send_to_char("Syntax : contribute X    where X = 1,2, or 3\n\r",ch);
		return;
	}
	time = atol (argument);
	if (time < 1 || time > 3)
	{
		send_to_char("Invalid amount of time.  You may only add 1, 2 or 3 ticks.\n\r",ch);
		return;
	}
	if (ch->pcdata->nhpoints < time)
	{
		send_to_char("You do not have that many nh points!\n\r",ch);
		return;
	}
	ch->pcdata->nhpoints -= time;
	double_timer += time;
}

//Iblis - Quest flags a mortal (so they have pk protection).  Appears on who for easy viewing of questors.
void do_questor(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	if (argument[0] == '\0')
	{
		send_to_char("Syntax : Questor Person\n\r",ch);
		return;
	}
	if ((victim = get_char_world (ch, argument)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC(victim) || !victim->pcdata)
	{
		send_to_char("MORON! Don't do it on NPC's.\n\r",ch);
		return;
	}
	if (victim->pcdata->questing == 0)
	{
		act("$N is now set as a questor.",ch,NULL,victim,TO_CHAR);
		send_to_char("You are now a questor.\n\r",victim);
		victim->pcdata->questing = 1;
	}
	else
	{
		act("$N is no longer set as a questor.",ch,NULL,victim,TO_CHAR);
		send_to_char("You are no longer a questor.\n\r",victim);
		victim->pcdata->questing = 0;
	}
}
//Copied Iblis' code, this adds the 'Elder' flag to Kalians.
void do_elder(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	if (argument[0] == '\0')
	{
		send_to_char("Syntax : Adept Person\n\r",ch);
		return;
	}
	if ((victim = get_char_world (ch, argument)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC(victim) || !victim->pcdata)
	{
		send_to_char("MORON! Don't do it on NPC's.\n\r",ch);
		return;
	}
	if (victim->pcdata->elder == 0)
	{
		act("$N is now set as an Elder",ch,NULL,victim,TO_CHAR);
		send_to_char("You are now an Elder.\n\r",victim);
		victim->pcdata->elder = 1;
	}
	else
	{
		act("$N is no longer set as an Elder.",ch,NULL,victim,TO_CHAR);
		send_to_char("You are no longer an Elder.\n\r",victim);
		victim->pcdata->elder = 0;
	}
}

//Iblis - Strips non-permanent and non-racial affects from target
void do_stripaffects(CHAR_DATA *ch,char* argument)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;
	CHAR_DATA *victim;

	if ((victim = get_char_room (ch, argument)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	for (paf = victim->affected; paf != NULL; paf = paf_next)
	{
		paf_next = paf->next;
		if (paf->type == gsn_fireshield && !IS_NPC(ch) && ch->pcdata->flaming)
			continue;
		if (paf->type == gsn_aquatitus || paf->type == gsn_likobe || paf->type == gsn_jalknation
			|| paf->type == gsn_jurgnation || paf->type == gsn_dehydration || paf->type == skill_lookup("beacon of the damned"))
			continue;
		if (!paf->permaff && paf->duration != -1)
			affect_remove (victim, paf);
	}
	if (ch != victim)
		send_to_char("You cleared them of all non-perma affects.\n\r",ch);
	send_to_char("You have been cleared of all non-perma affects.\n\r",victim);
	return;
}

//Iblis - Sets a target's comm so they cannot use gemote
void do_nogemote (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("NoGemote whom?\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (get_trust (victim) >= get_trust (ch))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (IS_SET (victim->comm, COMM_IMM_NOGEMOTE))
	{
		REMOVE_BIT (victim->comm2, COMM_IMM_NOGEMOTE);
		send_to_char ("You can Gemote again.\n\r", victim);
		send_to_char ("NOGEMOTE removed.\n\r", ch);
		sprintf (buf, "$N restores Gemotes to %s.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else
	{
		SET_BIT (victim->comm2, COMM_IMM_NOGEMOTE);
		send_to_char ("You can't Gemote!\n\r", victim);
		send_to_char ("NOGEMOTE set.\n\r", ch);
		sprintf (buf, "$N revokes %s's Gemotes.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	return;
}

//Iblis - Makes it so the target player cannot delete 
void do_nodelete (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Nodelete whom?\n\r", ch);
		return;
	}
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (get_trust (victim) >= get_trust (ch))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (IS_SET (victim->comm2, COMM_NODELETE))
	{
		REMOVE_BIT (victim->comm2, COMM_NODELETE);
		send_to_char ("NODELETE removed.\n\r", ch);
		sprintf (buf, "$N restores deletion ability to %s.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else
	{
		SET_BIT (victim->comm2, COMM_NODELETE);
		send_to_char ("NODELETE set.\n\r", ch);
		sprintf (buf, "$N revokes %s's deletion ability.", victim->name);
		wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	return;
}

//Iblis - Allows viewing of a player's affects via "stat char bob affects"
void do_affectstat (CHAR_DATA * imm, CHAR_DATA *ch)
{
	AFFECT_DATA *paf, *paf_last = NULL;
	char buf[MAX_STRING_LENGTH];
	bool affectie = FALSE;
	if (ch->affected != NULL)
	{
		affectie = TRUE;
		send_to_char ("They are affected by the following:\n\r", imm);
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
				{
					switch (skill_table[paf->type].type)
					{
					case SKILL_NORMAL:
						if (*skill_table[paf->type].pgsn != gsn_uppercut &&
							*skill_table[paf->type].pgsn != gsn_dehydration)
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
			}
			send_to_char (buf, imm);
			if (ch->level >= 20)
			{
				if (paf->where == TO_SKILL)
				{
					if(paf->location == 0)
						sprintf(buf,": modifies all abilities by %d",paf->modifier);
					else sprintf(buf,": modifies %s by %d",
						skill_table[paf->location].name,paf->modifier);
					send_to_char (buf,imm);
					send_to_char ("% ",imm);
				}
				else
				{
					sprintf (buf, ": modifies %s by %d ",
						affect_loc_name (paf->location), paf->modifier);
					send_to_char (buf, imm);
				}
				if (paf->duration == -1)
				{
					sprintf (buf, "permanently");
					send_to_char (buf, imm);
				}

				else if (paf->type != gsn_jurgnation && paf->type != gsn_jalknation
					&& paf->type != skill_lookup("beacon of the damned")
					&& paf->type != gsn_aquatitus
					&& paf->type != gsn_dehydration
					&& !(paf->type == gsn_fireshield && ch->race == PC_RACE_LITAN))
				{
					sprintf (buf, "for %d hours", paf->duration);
					send_to_char (buf, imm);
				}
			}
			send_to_char ("\n\r", imm);
			if (!paf->composition && str_cmp(paf->comp_name,""))
			{
				sprintf (buf, "                                Affect perpetrated by : %-15s\n\r", paf->
					comp_name);
				send_to_char(buf,imm);
			}
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
				send_to_char ("They are affected by the following:\n\r", imm);
			}
			send_to_char("Racial Affect: detect magic   : modifies none by 0 permanently\n\r",imm);
		}
		if ((race_table[ch->race].aff & AFF_DETECT_HIDDEN) && (ch->affected_by & AFF_DETECT_HIDDEN))
		{
			if (!affectie)
			{
				affectie = TRUE;
				send_to_char ("They are affected by the following:\n\r", imm);
			}
			send_to_char("Racial Affect: detect hidden  : modifies none by 0 permanently\n\r",imm);
		}
		if ((race_table[ch->race].aff & AFF_INFRARED) && (ch->affected_by & AFF_INFRARED))
		{
			if (!affectie)
			{
				affectie = TRUE;
				send_to_char ("They are affected by the following:\n\r", imm);
			}
			send_to_char("Racial Affect: infravision    : modifies none by 0 permanently\n\r",imm);
		}
		if ((race_table[ch->race].aff & AFF_FLYING) && (ch->affected_by & AFF_FLYING))
		{
			if (!affectie)
			{
				affectie = TRUE;
				send_to_char ("Then are affected by the following:\n\r", imm);
			}
			send_to_char("Racial Affect: fly            : modifies none by 0 permanently\n\r",imm);
		}
		if ((race_table[ch->race].aff & AFF_AQUA_BREATHE) && (ch->affected_by & AFF_AQUA_BREATHE))
		{
			if (!affectie)
			{
				affectie = TRUE;
				send_to_char ("Then are affected by the following:\n\r", imm);
			}
			send_to_char("Racial Affect: aqua breathe   : modifies none by 0 permanently\n\r",imm);
		}
		if ((race_table[ch->race].aff & AFF_PASS_DOOR) && (ch->affected_by & AFF_PASS_DOOR))
		{
			if (!affectie)
			{
				affectie = TRUE;
				send_to_char ("Then are affected by the following:\n\r", imm);
			}
			send_to_char("Racial Affect: pass door      : modifies none by 0 permanently\n\r",imm);
		}
		if ((race_table[ch->race].aff & AFF_DETECT_GOOD) && (ch->affected_by & AFF_DETECT_GOOD))
		{
			if (!affectie)
			{
				affectie = TRUE;
				send_to_char ("They are affected by the following:\n\r", imm);
			}
			send_to_char("Racial Affect: detect good    : modifies none by 0 permanently\n\r",imm);
		}


	}


	if (!affectie)
		send_to_char ("They are not affected by any magic.\n\r", imm);
	return;
}

//Iblis - A toggle for setting the toll, Minax's suggestion to punish annoying mortals, by
//having it cost either an amount of gold or deal an amount of hp to send tell/telepath to an imm
void do_toll (CHAR_DATA *ch,char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	argument = one_argument (argument, arg);
	argument = one_argument (argument, arg2);
	if (arg[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("Syntax: Toll goldamount damageamount\n\r",ch);
		return;
	}
	if (!is_number(arg2) || !is_number(arg))
	{
		send_to_char("Syntax: Toll goldamount damageamount\n\r",ch);
		return;
	}
	gold_toll = atol(arg);
	damage_toll = atol(arg2);
	send_to_char("Toll set.\n\r",ch);
}

//Iblis - Turns off the affects of hitroll (ONLY for OLD ac system)
void do_ignorehitroll(CHAR_DATA *ch, char *argument)
{
	if (!ignore_hitroll)
	{
		send_to_char("Hitroll affects are now turned OFF.\n\r",ch);
		ignore_hitroll = TRUE;
	}
	else
	{
		send_to_char("Hitroll affects are now turned ON.\n\r",ch);
		ignore_hitroll = FALSE;
	}
	return;
}

//Iblis - Turns off the affects of ac (ONLY for OLD ac system)
void do_ignoreac(CHAR_DATA *ch, char *argument)
{
	if (!ignore_ac)
	{
		send_to_char("AC affects are now turned OFF.\n\r",ch);
		ignore_ac = TRUE;
	}
	else
	{
		send_to_char("AC affects are now turned ON.\n\r",ch);
		ignore_ac = FALSE;
	}
	return;
}

//Iblis - Turns off the affects of save vs spell
void do_ignoresave(CHAR_DATA *ch, char *argument)
{
	if (!ignore_save)
	{
		send_to_char("Save affects are now turned OFF.\n\r",ch);
		ignore_save = TRUE;
	}
	else
	{
		send_to_char("Save affects are now turned ON.\n\r",ch);
		ignore_save = FALSE;
	}
	return;
}

//Iblis - Flags an object with a +%bonus to a particular skill
void do_skillaff (CHAR_DATA *ch, char* argument)
{
	extern int top_affect;
	OBJ_DATA *obj;
	int value;
	//  OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *pAf;
	char loc[MAX_STRING_LENGTH];
	char mod[MAX_STRING_LENGTH];
	char item_string[MAX_STRING_LENGTH];
	argument = one_argument (argument, item_string);
	argument = one_argument (argument, loc);
	one_argument (argument, mod);
	if (loc[0] == '\0' || mod[0] == '\0' || item_string[0] == '\0' || !is_number (mod))
	{
		send_to_char ("Syntax:  skillaffect <object> <skill> <%bonus>\n\r", ch);
		return;
	}
	if ((value = skill_lookup(loc)) == -1)
	{                           /* Hugin */
		send_to_char ("That is not a valid skill.\n\r", ch);
		return;
	}
	if ((obj = get_obj_here (ch, item_string)) == NULL)
	{
		send_to_char ("You do not see that here.\n\r", ch);
		return;
	}

	pAf = alloc_perm (sizeof (*pAf));
//	pAf = reinterpret_cast<AFFECT_DATA *>(alloc_perm(sizeof(*pAf)));
	pAf->where = TO_SKILL;
	pAf->location = value;
	pAf->modifier = atoi (mod);
	pAf->type = value;
	pAf->duration = -1;
	pAf->bitvector = 0;
	pAf->composition = FALSE;
	pAf->comp_name = str_dup ("");
	pAf->next = obj->affected;
	obj->affected = pAf;
	send_to_char ("Affect added.\n\r", ch);
	top_affect++;
	return;
}

//Iblis - Toggles the new/old ac system.  This is ON by default now.
void do_newac(CHAR_DATA *ch, char *argument)
{
	if (!new_ac)
	{
		send_to_char("New ac affects are now turned ON.\n\r",ch);
		new_ac = TRUE;
	}
	else
	{
		send_to_char("New ac affects are now turned OFF.\n\r",ch);
		new_ac = FALSE;
	}
	return;
}

//Iblis - An Iverath requested (and Iverath named) command that strips battle lag
//from the target character
void do_blagbgone(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	if (argument[0] == '\0')
		victim = ch;
	else if ((victim = get_char_room (ch, argument)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC(victim))
	{
		send_to_char ("Not on mobs, dumbass.\n\r",ch);
		return;
	}

	victim->pcdata->last_fight = time(NULL)-121;
	send_to_char("Blag b gone!\n\r",ch);
	return;
}

//Iblis - player purge.  Separated out due to original new implementation of player stores
//(where shopkeepers were hired for an hourly rate)
void ppurge(CHAR_DATA *ch, CHAR_DATA* victim)
{
	char buf[100];
	CHAR_DATA *vch;              
	OBJ_DATA *obj, *obj_next;
	DESCRIPTOR_DATA *d;

	if (!is_linkloading)
	{
		if (ch == victim)
		{
			send_to_char ("Ho ho ho.\n\r", ch);
			return;
		}
		if (get_trust (ch) <= get_trust (victim))
		{
			send_to_char ("Maybe that wasn't a good idea...\n\r", ch);
			sprintf (buf, "%s tried to purge you!\n\r", ch->name);
			send_to_char (buf, victim);
			return;
		}
	}
	if (!IS_NPC(ch) && !is_linkloading)
	{
		act ("$n disintegrates $N.", ch, 0, victim, TO_NOTVICT);
		act ("$n purges you, and you are forced to leave the game.", ch, 0, victim, TO_VICT);
		act ("You purge $N.",ch,0,victim,TO_CHAR);
	}

	//IBLIS 6/29/03 - Falconry fix
	for (d = descriptor_list; d != NULL; d = d->next)
	{
		if (d->character == NULL || IS_NPC (d->character)
			|| d->character->pcdata == NULL || victim->pcdata == NULL)
			continue;
		if (d->character->pcdata->falcon_wait != 0)
			if (d->character->pcdata->falcon_recipient == victim)
				d->character->pcdata->falcon_recipient = NULL;
	}

	if (victim->pcdata->corpse != NULL)
		obj_to_room(victim->pcdata->corpse,get_room_index(1));

	if (victim->pcdata->flaming)
	{
		OBJ_DATA *obj;
		victim->pcdata->flaming = 0;
		if (!((obj = get_eq_char (victim, WEAR_LIGHT)) != NULL
			&& obj->item_type == ITEM_LIGHT
			&& obj->value[2] != 0 && victim->in_room != NULL))
			victim->in_room->light--;

		if (is_affected(victim,gsn_fireshield))
			affect_strip(victim,gsn_fireshield);
	}

	//IBLIS 6/20/03 - Battle Royale fixes
	if (battle_royale && br_leader == victim)
	{

		br_leader = NULL;
		for (d = descriptor_list; d != NULL; d = d->next)
		{
			if (d->character == NULL || d->character == victim
				|| IS_NPC (d->character)
				|| d->character->pcdata == NULL || victim->pcdata == NULL)
				continue;
			if (br_leader == NULL && d->character->pcdata->br_points > 0)
				br_leader = d->character;

			else if (br_leader != NULL
				&& d->character->pcdata->br_points >
				br_leader->pcdata->br_points)
				br_leader = d->character;
		}
		if (br_leader != NULL)
		{
			char ttbuf[MAX_STRING_LENGTH];
			send_to_char
				("`gDue to the previous Battle Royal leader leaving, you are the new Battle Royal Leader!``\r\n",
				victim);
			sprintf (ttbuf,
				"`gAnd we have a new Battle Royal Leader -> %s``\r\n",
				br_leader->name);
			do_echo (br_leader, ttbuf);
		}
	}
	if (last_br_kill == victim)
		last_br_kill = NULL;

	//6/29/03 Adeon - objects flagged as NOSAVE drop to the ground on quit
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;
		if (IS_OBJ_STAT (obj, ITEM_NOSAVE))
		{
			if (obj->wear_loc != WEAR_NONE)
				unequip_char (victim, obj);
			obj_from_char (obj);
			if (victim->in_room)
				obj_to_room (obj, victim->in_room);
			else extract_obj(obj);
			act ("$n loses possesion of $p.", victim, obj, NULL, TO_ROOM);
			act ("You lose possesion of $p.", victim, obj, NULL, TO_CHAR);
			if (IS_OBJ_STAT (obj, ITEM_MELT_DROP))
			{
				act ("$p dissolves into smoke.", victim, obj, NULL, TO_CHAR);
				extract_obj (obj);
			}

		}
	}
	//Iblis 6/28/03 - Aggressor list fixed
	for (d = descriptor_list; d != NULL; d = d->next)
	{
		if (d->character == NULL || victim == d->character
			|| IS_NPC (d->character) || d->character->pcdata == NULL
			|| victim->pcdata == NULL)
			continue;
		remove_aggressor (victim, d->character);
		remove_aggressor (d->character, victim);
	}

	if (victim->race == PC_RACE_KALIAN)
	{
		affect_strip (victim, gsn_jurgnation);
		affect_strip (victim, gsn_jalknation);
	}

	for (vch = char_list;vch != NULL;vch = vch->next)
	{
		if (vch->contaminator && vch->contaminator == ch)
			vch->contaminator = NULL;
	}


	if (IS_AFFECTED (victim, AFF_CHARM))
	{
		REMOVE_BIT (victim->affected_by, AFF_CHARM);
		affect_strip (victim, gsn_charm_person);
		affect_strip (victim, skill_lookup ("The Pipers Melody"));
	}

	if (is_affected(victim,gsn_soul_link))
	{
		affect_strip (victim,gsn_soul_link);
		if (victim->pcdata->soul_link != NULL)
		{
			affect_strip (victim->pcdata->soul_link,gsn_soul_link);
			victim->pcdata->soul_link->pcdata->soul_link = NULL;
		}
		victim->pcdata->soul_link = NULL;
	}

	if (victim->pcdata->quest_obj != NULL)
		extract_obj(victim->pcdata->quest_obj);
	if (victim->pcdata->quest_mob != NULL && victim->pcdata->quest_mob != victim)
		extract_char(victim->pcdata->quest_mob,TRUE);




	if (victim->level > 1)
		save_char_obj (victim);
	if (is_linkloading && victim->was_in_room != NULL)
	{
		char_from_room(victim);
		char_to_room(victim,victim->was_in_room);
	}
	d = victim->desc;
	extract_char (victim, TRUE);
	if (d != NULL)
		close_socket (d);
	return;
}

void do_qbitset(CHAR_DATA *ch, char* argument)
{
	CHAR_DATA *victim;
	int bit;
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument (argument, arg);
	victim = get_char_room (ch, arg);
	if (victim == NULL)
		victim = get_char_world (ch,arg);
	if (victim == NULL)
	{
		send_to_char("They are not here.\n\r",ch);
		return;
	}
	if (IS_NPC(victim))
	{
		send_to_char("NOT ON NPCS!\n\r",ch);
		return;
	}
	if (!is_number(argument) || (bit = atoi(argument)) < 0 || bit > MAX_QUEST_BITS)
	{
		send_to_char("That's an invalid quest bit.\n\r",ch);
		return;
	}
	setbit(victim->pcdata->qbits,bit);
	//  victim->pcdata->qbits[bit] = TRUE;
	send_to_char("Quest bit set.\n\r",ch);
}

void do_qbitunset(CHAR_DATA *ch, char* argument)
{
	CHAR_DATA *victim;
	int bit;
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument (argument, arg);
	victim = get_char_room (ch, arg);
	if (victim == NULL)
		victim = get_char_world (ch,arg);
	if (victim == NULL)
	{
		send_to_char("They are not here.\n\r",ch);
		return;
	}
	if (IS_NPC(victim))
	{
		send_to_char("NOT ON NPCS!\n\r",ch);
		return;
	}
	if (!is_number(argument) || (bit = atoi(argument)) < 0 || bit > MAX_QUEST_BITS)
	{
		send_to_char("That's an invalid quest bit.\n\r",ch);
		return;
	}
	unsetbit(victim->pcdata->qbits,bit);
	send_to_char("Quest bit unset.\n\r",ch);
}

void do_qbitget(CHAR_DATA *ch, char* argument)
{
	CHAR_DATA *victim;
	int bit;
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument (argument, arg);
	victim = get_char_room (ch, arg);
	if (victim == NULL)
		victim = get_char_world (ch,arg);
	if (victim == NULL)
	{
		send_to_char("They are not here.\n\r",ch);
		return;
	}
	if (IS_NPC(victim))
	{
		send_to_char("NOT ON NPCS!\n\r",ch);
		return;
	}
	if (!is_number(argument) || (bit = atoi(argument)) < 0 || bit > MAX_QUEST_BITS)
	{
		send_to_char("That's an invalid quest bit.\n\r",ch);
		return;
	}
	if (getbit(victim->pcdata->qbits,bit))
		send_to_char("Quest bit IS set.\n\r",ch);
	else send_to_char("Quest bit is NOT set.\n\r",ch);
}

void do_checknplayers(CHAR_DATA *ch, char* argument)
{
	char buf[MAX_STRING_LENGTH];
	if (ch->in_room && ch->in_room->area)
	{
		sprintf(buf,"Number of players in this area now is -> %d.\n\r",ch->in_room->area->nplayer);
		send_to_char(buf,ch);
	}
}


void do_relevel (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];

	// Max levels
	if (    !str_cmp(ch->name,"Morgan")
		||  !str_cmp(ch->name,"Minax"))
	{
		ch->level = MAX_LEVEL;
		ch->trust = MAX_LEVEL;
		ch->pcdata->security = 9;
		sprintf(buf, "%s has been restored to max level.\n\r", ch->name);
		send_to_char(buf, ch);
		return;
	}

	else
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	return;
}

void do_delevel (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	char arg1[MAX_STRING_LENGTH];
	int level;

	argument = one_argument (argument, arg1);

	if (arg1[0] == '\0' || !is_number (arg1))
	{
		send_to_char ("Syntax: delevel <level>.\n\r", ch);
		return;
	}

	if ((level = atoi (arg1)) < 1 || level > 90)
	{
		sprintf (buf, "Level must be 1 to %d.\n\r", 90);
		send_to_char (buf, ch);
		return;
	}

	ch->level = level;
	ch->trust = level;
	ch->pcdata->security = 0;
	sprintf(buf, "%s has been restored to level %d.\n\r", ch->name, level);
	send_to_char(buf, ch);
	send_to_char("Your security has been removed.\n\r", ch);

	return;
}
