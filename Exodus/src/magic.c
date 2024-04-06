#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "obj_trig.h"

//
// Tables of nasty mobs to use when summon goes badly
//
bool bad_summoning_on = FALSE;
bool safe_nopk = TRUE;
bool killed_by_chain = FALSE;
extern bool exploration_tracking;
extern bool ignore_save;
void cast_line args ((CHAR_DATA * ch, char *argument));
extern bool nogate;
void prayer_damnation args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
void reverse_heal args((CHAR_DATA* ch, CHAR_DATA* victim, int dam, int sn));
int true_level_for_skill args((CHAR_DATA *ch, int sn));
int getbit args ((char *explored, int index));
void obj_cast_spell2 args((int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, char* argument));

#define NASTY_LIST_LEN 7
const int nasty10[NASTY_LIST_LEN] = { 21114,	// a painted horse, level 14, eastern prarie
20954,			// a strongman, level 16, fulk nerra
553,				// a mean 'ol dog, level 10, teche village
12956,			// a timberjace, level 15, the mill
12928,			// an evil pixie, level 15, forest fierense
12937,			// will-o-whisp, level 15, outer woodlands
21095				// wolf, level 6, eastern prairie
};
const int nasty20[NASTY_LIST_LEN] = { 12923,	// wolverine, level 29, forest firense
4084,				// a bodyguard, level 29, thesden
13418,			// rock troll, level 28, macocaap outlands
20964,			// higgins, level 23, fulk nerra
20967,			// mad anna, level 25, fulk nerra
5300,				// flaming hound, level 30, burning chambers
12927				// moose, level 28, outer woodlands
};
const int nasty30[NASTY_LIST_LEN] = { 2223,	// white dragon, level 37, keep of lesser dragons
4092,				// a knight, level 37, thesden
4091,				// an elder thief, level 36, thesden
8704,				// tomb thief, level 38, ancient pyramid
7911,				// baker, level 32, erion abby
10768,			// black cougar, level 30, silver crescent woods
5517				// syvin hunter, level 35, linden
};
const int nasty40[NASTY_LIST_LEN] = { 8708,	// sandman, level 43, ancient pyramid
11536,			// scruffy pirate, level 45, freeport
11538,			// admiral blacksail, level 48, freeport
7280,				// manipulatii devil, level 48, argent stronghold
2604,				// dwarf brigand, level 40, shale peaks
20387,			// canthi seer, level 43, ebon downs
5333				// chamber guard, level 47, burning chambers
};
const int nasty50[NASTY_LIST_LEN] = { 100,	// knight guardian, level 50, castle dunluce
5334,				// elite chamber guard, level 53, burning chambers
7284,				// mephisto lord, level 54, argent stronghold
18993,			// drunk old man, level 58, city of the sun
11540,			// lord mourning, level 53, freeport
2620,				// shale dragon, level 60, shale peaks
20390				// captain of the canthi, level 52, ebon downs
};
const int nasty60[NASTY_LIST_LEN] = { 7251,	// champion of lance argent, level 63, argent stronghold
19078,			// ocelot knight, level 68, city of the sun
7702,				// mountain barbarian, level 60, ryiascar's rise
11382,			// vroath heavy cavalry, level 60, fields of death
19144,			// sun guard, level 68, city of the sun
19087,			// clan leader, level 65, city of the sun
16303				// a huge ant, level 60, keep of krathen
};
const int nasty70[NASTY_LIST_LEN] = { 7703,	// ular, level 70 ryiascar's rise
7711,				// bronze dragon, level 76, ryaiscar's rise
3677,				// weinCloudstan guard, level 74, weincloudstan city
7265,				// angel seraph, level 79, argent stronghold
16304,			// krathen guard, level 80, keep of krathen
19088,			// king, level 70, city of the sun
0				//
};
const int nasty80[NASTY_LIST_LEN] = { 14,	// silver dragon, level 85, 
7268,				// lord of law, level 85, argent stronghold
11362,			// goltar guardian, level 85, 
3206,				// cloud dragon, level 80, weincloudstan
0,				//
0,				//
0				//
};
const int nasty90[NASTY_LIST_LEN] = { 7060,	// shenlon, level 90, lair of shenlon
0,				//
0,				//
0,				//
0,				//
0,				//
0				//
};

/* command procedures needed */
DECLARE_DO_FUN (do_look);

/*
* Local functions.
*/
void say_spell args ((CHAR_DATA * ch, int sn));

/* imported functions */
bool remove_obj args ((CHAR_DATA * ch, int iWear, bool fReplace));
void wear_obj
args ((CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace, int hand));
bool insta_kill
args ((CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type,
	  bool show, bool magic));
char *condition_noun args ((int condition));

//IBLIS 6/21/03 - Needed for areas ticking off checking in Battle Royale
extern bool battle_royale;
bool br_vnum_check args ((int vnum));

/*
* Lookup a skill by name.
*/
int skill_lookup (const char *name)
{
	int sn;
	for (sn = 0; sn < MAX_SKILL; sn++)
	{
		if (skill_table[sn].name == NULL)
			break;
		if (LOWER (name[0]) == LOWER (skill_table[sn].name[0])
			&& !str_prefix (name, skill_table[sn].name))
			return sn;
	}
	return -1;
}


/*
* Lookup a skill by name.
*/
int exact_skill_lookup (const char *name)
{
	int sn;
	for (sn = 0; sn < MAX_SKILL; sn++)
	{
		if (skill_table[sn].name == NULL)
			break;
		if (LOWER (name[0]) == LOWER (skill_table[sn].name[0])
			&& !str_cmp (name, skill_table[sn].name))
			return sn;
	}
	return -1;
}

int find_spell (CHAR_DATA * ch, const char *name)
{

	/* finds a spell the character can cast if possible */
	int sn, found = -1;
	if (IS_NPC (ch))
		return skill_lookup (name);
	for (sn = 0; sn < MAX_SKILL; sn++)
	{
		if (skill_table[sn].name == NULL)
			break;
		if (LOWER (name[0]) == LOWER (skill_table[sn].name[0])
			&& !str_prefix (name, skill_table[sn].name))
		{
			if (found == -1)
				found = sn;
			if (get_skill(ch,sn) >= 1)
				return sn;
		}
	}
	return found;
}


/*
* Lookup a skill by slot number.
* Used for object loading.
*/
int slot_lookup (int slot)
{
	extern bool fBootDb;
	int sn;
	if (slot <= 0)
		return -1;
	for (sn = 0; sn < MAX_SKILL; sn++)
	{
		if (slot == skill_table[sn].slot)
			return sn;
	}
	if (fBootDb)
	{
		bug ("Slot_lookup: bad slot %d.", slot);
		abort ();
	}
	return -1;
}


/*
* Utter mystical words for an sn.
*/
void say_spell (CHAR_DATA * ch, int sn)
{
	ROOM_INDEX_DATA *rid;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	CHAR_DATA *rch;
	char *pName;
	int iSyl;
	int length;
	struct syl_type
	{
		char *old;
		char *sylnew;
	};
	static const struct syl_type syl_table[] = {
		{" ", " "},
		{"ar", "annen"},
		{"aqua", "sannuo"},
		{"au", "kada"},
		{"bless", "erino"},
		{"blind", "veras"},
		{"breath", "aeindara"},
		{"breathe", "aeindesa"},
		{"bur", "mosa"},
		{"cu", "eruoe"},
		{"de", "oculo"},
		{"en", "unso"},
		{"light", "rus"},
		{"lo", "hi"},
		{"mor", "zak"},
		{"move", "sido"},
		{"ness", "densi"},
		{"ning", "illa"},
		{"per", "duda"},
		{"ra", "gru"},
		{"fresh", "ima"},
		{"re", "nerein"},
		{"son", "sabru"},
		{"tect", "infra"},
		{"tri", "cula"},
		{"ven", "nofo"},
		{"a", "a"},
		{"b", "b"},
		{"c", "q"},
		{"d", "e"},
		{"e", "z"},
		{"f", "y"},
		{"g", "o"},
		{"h", "p"},
		{"i", "u"},
		{"j", "y"},
		{"k", "t"},
		{"l", "r"},
		{"m", "w"},
		{"n", "i"},
		{"o", "a"},
		{"p", "s"},
		{"q", "d"},
		{"r", "f"},
		{"s", "g"},
		{"t", "h"},
		{"u", "j"},
		{"v", "z"},
		{"w", "x"}, {"x", "n"}, {"y", "l"}, {"z", "k"}, {"", ""}
	};
	buf[0] = '\0';
	for (pName = skill_table[sn].name; *pName != '\0'; pName += length)
	{
		for (iSyl = 0; (length = strlen (syl_table[iSyl].old)) != 0; iSyl++)
		{
			if (!str_prefix (syl_table[iSyl].old, pName))
			{
				strcat (buf, syl_table[iSyl].sylnew);
				break;
			}
		}
		if (length == 0)
			length = 1;
	}
	if (ch->Class != PC_CLASS_CHAOS_JESTER)
	{
		sprintf (buf2, "$n intones the sacred words, '%s'.", buf);
		sprintf (buf, "$n intones the sacred words, '%s'.", skill_table[sn].name);
		for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
		{
			if (rch != ch)
				act (IS_CLASS (rch, PC_CLASS_MAGE) ? buf : buf2, ch, NULL, rch, TO_VICT);
		}
		if (is_dueling (ch))
		{
			rid = get_room_index (ROOM_VNUM_SPECTATOR);
			for (rch = rid->people; rch; rch = rch->next_in_room)
				act (IS_CLASS (rch, PC_CLASS_MAGE) ? buf : buf2, ch, NULL, rch, TO_VICT);
		}
	}
	return;
}


/*
* Compute a saving throw.
* Negative apply's make saving throw better.
*/
bool saves_spell (int level, CHAR_DATA * victim, int dam_type)
{
	int save;
	if (!ignore_save)
		save = 50 + ((victim->level - level)/5) - (victim->saving_throw * (50/40));
	else save = 50 + ((victim->level - level)/5);
	//  if (IS_AFFECTED (victim, AFF_BERSERK))
	//    save += victim->level / 2;
	//Little bonus for resist poison
	if (dam_type == DAM_POISON && IS_SET(victim->res_flags,RES_POISON))
		save += 5;
	switch (check_immune (victim, dam_type))
	{
	case IS_IMMUNE:
		return TRUE;
	case IS_RESISTANT:
		save += 5;
		break;
	case IS_VULNERABLE:
		save -= 5;
		break;
	}
	if (!IS_NPC (victim) && !mana_using_Class(victim))
		save = 9 * save / 10;
	save = URANGE (5, save, 98);
	return number_percent () < save;
}

bool saves_spell_old (int level, CHAR_DATA * victim, int dam_type)
{
	int save;
	if (!ignore_save)
		save = 50 + (victim->level - level)*5 - (victim->saving_throw * (50/40));
	else save = 50 + (victim->level - level)*5;
	//  if (IS_AFFECTED (victim, AFF_BERSERK))
	//    save += victim->level / 2;
	//Little bonus for resist poison
	if (dam_type == DAM_POISON && IS_SET(victim->res_flags,RES_POISON))
		save += 5;
	switch (check_immune (victim, dam_type))
	{
	case IS_IMMUNE:
		return TRUE;

	case IS_RESISTANT:
		save += 5;
		break;

	case IS_VULNERABLE:
		save -= 5;
		break;
	}
	if (!IS_NPC (victim) && !mana_using_Class(victim))
		save = 9 * save / 10;
	save = URANGE (5, save, 98);
	return number_percent () < save;
}


/* RT save for dispels */
bool saves_dispel (int dis_level, int spell_level, int duration)
{
	int save;

	/*  if (duration == -1)
	spell_level += 5; 
	return; */
	/* very hard to dispel permanent effects */
	save = 50 + (spell_level - dis_level) * 5;
	save = URANGE (5, save, 95);
	return number_percent () < save;
}


/* co-routine for dispel magic and cancellation */
bool check_dispel (int dis_level, CHAR_DATA * victim, int sn)
{
	AFFECT_DATA *af;
	if (is_affected (victim, sn))
	{
		for (af = victim->affected; af != NULL; af = af->next)
		{
			if (af->duration == -1)
				continue;
			if (af->permaff == TRUE)
				continue;
			if (af->type == sn)
			{
				if (!saves_dispel (dis_level, af->level, af->duration))
				{
					affect_strip (victim, sn);
					if (skill_table[sn].msg_off)
					{
						send_to_char (skill_table[sn].msg_off, victim);
						send_to_char ("\n\r", victim);
					}
					return TRUE;
				}

				else
					af->level--;
			}
		}
	}
	return FALSE;
}


/* for finding mana costs -- temporary version */
int mana_cost (CHAR_DATA * ch, int min_mana, int level)
{
	if (ch->level + 2 == level)
		return 1000;
	return UMAX (min_mana, (100 / (2 + ch->level - level)));
}

void do_layhands (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	one_argument (argument, arg);
	if (get_skill (ch, gsn_layhands) < 1)
	{
		send_to_char ("You have no idea where to lay your hands.\r\n", ch);
		return;
	}
	if (arg[0] == '\0')
	{
		send_to_char ("Lay your hands upon whom?\n\r", ch);
		return;
	}
	if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (ch->mana < ch->level)
	{
		send_to_char ("You do not have enough mana.\n\r", ch);
		return;
	}
	if (ch->lh_ticks != 0)
	{
		send_to_char ("You are not yet ready to do that again.\n\r", ch);
		return;
	}
	ch->mana -= ch->level;
	if (number_percent () < get_skill (ch, gsn_layhands))
	{
		if (IS_SET(victim->act,ACT_UNDEAD))
			reverse_heal(ch,victim,500,gsn_layhands);
		else if (victim->race == PC_RACE_SWARM)
			swarm_heal(ch,victim,gsn_layhands);
		else
		{
			if (!IS_NPC(victim) && victim->pcdata->soul_link)
			{
				victim->hit = UMIN (victim->hit + (3 * ch->level), victim->max_hit);
				victim->pcdata->soul_link->hit = UMIN (victim->pcdata->soul_link->hit + (3 * ch->level),
					victim->pcdata->soul_link->max_hit);
				update_pos (victim->pcdata->soul_link);
				send_to_char ("A healing force courses through your body!\n\r", victim->pcdata->soul_link);
			}
			else victim->hit = UMIN (victim->hit + (6 * ch->level), victim->max_hit);
			act ("You lay your holy, glowing hands upon $N.", ch, NULL, victim,
				TO_CHAR);
			act ("$n lays $s holy, glowing hands upon you.", ch, NULL, victim,
				TO_VICT);
			ch->lh_ticks = 24;
		}
	}

	else
	{
		act
			("You lay your hands upon $N, but your faith in your god is not great enough to invoke the miracle of healing.",
			ch, NULL, victim, TO_CHAR);
		act ("$n lays $s hands upon you, but nothing happens.", ch, NULL,
			victim, TO_VICT);
		ch->lh_ticks = 12;
	}
}
void do_exorcise (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	int dam, sn;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Exorcise whom?\n\r", ch);
		return;
	}
	if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (number_percent () < get_skill (ch, gsn_exorcism))
	{
		if (victim == ch)
		{
			send_to_char ("You cannot exorcise yourself.\n\r", ch);
			return;
		}
		if (!IS_NPC (victim))
			if (IS_IMMORTAL (victim) || victim->level < 8)
			{
				send_to_char
					("You cannot perform an exorcism on that target.\n\r", ch);
				return;
			}
			if (ch->fighting != NULL)
			{
				send_to_char
					("You cannot concentrate enough to perform the exorcism\n\r", ch);
				return;
			}
			if (ch->mana < 400)
			{
				send_to_char
					("You don't have enough mana to perform the exorcism.\n\r", ch);
				return;
			}
			if (ch->move < 200)
			{
				send_to_char
					("You don't have enough movement ability to perform the exorcism.\n\r",
					ch);
				return;
			}
			if (ch->hit < 400)
			{
				send_to_char
					("You don't have enough hitpoints to perform the exorcism.\n\r",
					ch);
				return;
			}
			if (ch->ex_ticks != 0)
			{
				send_to_char ("You are not yet ready to do that again.\n\r", ch);
				return;
			}
			if (ch->alignment != -1000 && ch->alignment != 1000)
			{
				send_to_char
					("Your current alignment doesn't allow you to perform exorcisms.\n\r",
					ch);
				return;
			}
			if (!IS_IMMORTAL (ch))
				WAIT_STATE (ch, 3 * PULSE_VIOLENCE);
			ch->mana /= 2;
			ch->hit /= 2;
			ch->move /= 2;
			act
				("You grab $N's forehead and attempt to shake the spirits from $S soul.",
				ch, NULL, victim, TO_CHAR);
			act
				("$n grabs $N's forehead and attempts to shake the spirits from $S soul.",
				ch, NULL, victim, TO_NOTVICT);
			act
				("$n grabs your forehead and attempts to shake the spirits from your soul.",
				ch, NULL, victim, TO_VICT);
			ch->ex_ticks = 24;
			dam = abs (ch->alignment - victim->alignment) / 2;
			dam = (dam * ch->level) / 100;
			if (IS_SET (victim->res_flags, RES_HOLY))
				dam /= 2;
			if (IS_SET (victim->vuln_flags, VULN_HOLY))
				dam = (dam * 3) / 2;
			if (IS_SET (victim->imm_flags, IMM_HOLY))
				dam = 0;
			if (IS_SET (victim->act, ACT_UNDEAD))
				dam = victim->max_hit;
			sn = skill_lookup ("exorcism");
			if (get_curr_stat (ch, STAT_WIS) < 18 && number_percent () < 25)
			{
				damage (ch, victim, (dam * 3) / 4, sn, DAM_OTHER, TRUE);
				damage (ch, ch, dam / 2, sn, DAM_OTHER, TRUE);
			}

			else
				damage (ch, victim, dam, sn, DAM_OTHER, TRUE);
			if (ch->alignment > 0)
				change_alignment(ch,-300);
			else
				change_alignment(ch,300);

	}

	else
		send_to_char ("You do not know how to perform an exorcism.\n\r", ch);
}


/*
* The kludgy global is for spells who want more stuff from command line.
*/
char *target_name;

//Iblis 10/03/04 - Needed to not subtract mana from people for casting the CJ spells from wands, etc,
//since CJ spells check for mana usage in the actual spell
bool costs_mana=FALSE;


void do_cast (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	void *vo;
	int mana;
	int sn;
	int target;

	/*  if (IS_NPC(ch) && ch->desc == NULL)
	return;
	*/
	if (!str_cmp (argument, "line"))
	{
		cast_line (ch, argument);
		return;
	}
	if (!has_skills (ch, SKILL_CAST) && !IS_NPC (ch))
	{
		send_to_char ("You lack the ability to cast spells.\n\r", ch);
		return;
	}
	if (IS_SET (ch->in_room->room_flags, ROOM_SILENT) && !IS_IMMORTAL (ch))
	{
		send_to_char
			("This room is shielded from disturbance by powerful wards of magic.\n\r",
			ch);
		return;
	}
	// Iblis 9/13/04 - No Magic flag
	if (IS_SET (ch->in_room->room_flags2, ROOM_NOMAGIC) && !IS_IMMORTAL (ch))
	{
		send_to_char("There are no magical forces to draw power from.\n\r",ch);
		return;
	}
	target_name = one_argument (argument, arg1);
	one_argument (target_name, arg2);
	if (arg1[0] == '\0')
	{
		send_to_char ("Cast which what where?\n\r", ch);
		return;
	}
	if ((sn = find_spell (ch, arg1)) < 0
		|| (!IS_NPC (ch) && get_skill(ch,sn) < 1))
	{
		send_to_char ("You don't know any spells of that name.\n\r", ch);
		return;
	}

	if (IS_SET(ch->act2,ACT_FAMILIAR) && get_skill(ch,sn) < 1)
		return;


	if (skill_table[sn].type != SKILL_CAST)
	{
		send_to_char ("You don't know any spells of that name.\n\r", ch);
		return;
	}
	if (ch->position < skill_table[sn].minimum_position
		|| (ch->fighting != NULL
		&& skill_table[sn].minimum_position > POS_FIGHTING))
	{
		send_to_char ("You can't concentrate enough.\n\r", ch);
		return;
	}
	if (is_affected (ch, gsn_entangle))
	{
		send_to_char
			("You are too constrained to use any of your mystical powers.\n\r",
			ch);
		return;
	}
	if (is_affected (ch, gsn_fear))
	{
		send_to_char ("You are way too scared to cast that spell right now.\n\r", ch);
		return;
	}  
	if (is_affected (ch, skill_lookup ("Quiet Mountain Lake"))
		&& (skill_table[sn].target == TAR_CHAR_OFFENSIVE
		|| skill_table[sn].target == TAR_OBJ_CHAR_OFF
		|| skill_table[sn].target == TAR_AREA_OFF))
	{
		send_to_char
			("You are too peaceful to cast that spell right now.\n\r", ch);
		return;
	}

	if (sn != gsn_mirror)
	{
		if (skill_table[sn].min_mana == 0)
			mana = 0;
		else {
			if (ch->level + 2 == level_for_skill (ch, sn))
				mana = 50;

			else
				mana = UMAX (skill_table[sn].min_mana, 100 / (2 + ch->level - level_for_skill (ch, sn)));
		}
	}

	else
	{
		if (ch->mana < 100)
			mana = 100;

		else
			mana = ch->mana / 2;
	}
	if (is_affected (ch, gsn_mirror))
	{
		act ("The mirror arround you is destroyed by your outgoing magic.",
			ch, NULL, NULL, TO_CHAR);
		act ("The mirror around $n is destroyed by $s outgoing magic.",
			ch, NULL, NULL, TO_ROOM);
		affect_strip (ch, gsn_mirror);
	}
	//Kalus: Clarity spell causes all other spells to be half mana

if (is_affected (ch, gsn_clarity))

{

mana = mana / 2;

}
	// Adeon 7/2/03 -- We decided to make invis better, casting spells
	//                  no longer causes you to become visible.

	/*if (IS_AFFECTED (ch, AFF_INVISIBLE))
	{
	affect_strip (ch, gsn_invis);
	affect_strip (ch, gsn_mass_invis);
	affect_strip (ch, gsn_heavenly_cloak);
	REMOVE_BIT (ch->affected_by, AFF_INVISIBLE);
	act ("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
	} */

	/*
	* Locate targets.
	*/
	victim = NULL;
	obj = NULL;
	vo = NULL;
	target = TARGET_NONE;
	switch (skill_table[sn].target)
	{
	default:
		bug ("Do_cast: bad target for sn %d.", sn);
		return;
	case TAR_IGNORE:
	case TAR_AREA_OFF:
		break;
	case TAR_CHAR_OFFENSIVE:
		if (arg2[0] == '\0')
		{
			if ((victim = ch->fighting) == NULL)
			{
				send_to_char ("Cast the spell on whom?\n\r", ch);
				return;
			}
		}

		else
		{
			if ((victim = get_char_room (ch, target_name)) == NULL)
			{
				send_to_char ("They aren't here.\n\r", ch);
				return;
			}
			if (check_shopkeeper_attack (ch, victim))
				return;
		}
		if (!IS_NPC (ch))
		{
			if (is_safe (ch, victim) && victim != ch)
			{

				//            send_to_char("Not on that target.\n\r", ch);
				return;
			}
		}
		if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
		{
			send_to_char ("You can't do that on your own follower.\n\r", ch);
			return;
		}
		vo = (void *) victim;
		target = TARGET_CHAR;
		break;
	case TAR_CHAR_DEFENSIVE:
		if (arg2[0] == '\0')
		{
			victim = ch;
		}

		else
		{
			if ((victim = get_char_room (ch, target_name)) == NULL)
			{
				send_to_char ("They aren't here.\n\r", ch);
				return;
			}
		}
		vo = (void *) victim;
		target = TARGET_CHAR;
		break;
	case TAR_CHAR_SELF:
		if (arg2[0] != '\0' && !is_name (target_name, ch->name))
		{
			send_to_char ("You cannot cast this spell on another.\n\r", ch);
			return;
		}
		vo = (void *) ch;
		target = TARGET_CHAR;
		break;
	case TAR_OBJ_INV:
		if (arg2[0] == '\0')
		{
			send_to_char ("What should the spell be cast upon?\n\r", ch);
			return;
		}
		if ((obj = get_obj_carry (ch, target_name)) == NULL)
		{
			send_to_char ("You are not carrying that.\n\r", ch);
			return;
		}
		vo = (void *) obj;
		target = TARGET_OBJ;
		break;
	case TAR_OBJ_CHAR_OFF:
		if (arg2[0] == '\0')
		{
			if ((victim = ch->fighting) == NULL)
			{
				send_to_char ("Cast the spell on whom or what?\n\r", ch);
				return;
			}
			target = TARGET_CHAR;
		}

		else if ((victim = get_char_room (ch, target_name)) != NULL)
		{
			target = TARGET_CHAR;
		}
		if (target == TARGET_CHAR)
		{			/* check the sanity of the attack */
			if (is_safe_spell (ch, victim, FALSE) && victim != ch)
			{

				//            send_to_char("Not on that target.\n\r", ch);
				return;
			}
			if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
			{
				send_to_char
					("You can't do that on your own follower.\n\r", ch);
				return;
			}
			vo = (void *) victim;
		}

		else if ((obj = get_obj_here (ch, target_name)) != NULL)
		{
			vo = (void *) obj;
			target = TARGET_OBJ;
		}

		else
		{
			send_to_char ("You don't see that here.\n\r", ch);
			return;
		}
		break;
	case TAR_STRING:
		if (target_name[0] == '\0')
			vo = NULL;
		else vo = (void *) target_name;
		target = TARGET_NONE;
		break;

	case TAR_OBJ_CHAR_DEF:
		if (arg2[0] == '\0')
		{
			vo = (void *) ch;
			target = TARGET_CHAR;
		}

		else if ((victim = get_char_room (ch, target_name)) != NULL)
		{
			vo = (void *) victim;
			target = TARGET_CHAR;
		}

		else if ((obj = get_obj_carry (ch, target_name)) != NULL)
		{
			vo = (void *) obj;
			target = TARGET_OBJ;
		}

		else
		{
			send_to_char ("You don't see that here.\n\r", ch);
			return;
		}
		break;
	}
	if (!IS_NPC (ch) && ch->mana < mana && !IS_IMMORTAL (ch))
	{
		send_to_char ("You don't have enough mana.\n\r", ch);
		return;
	}
	if (str_cmp (skill_table[sn].name, "ventriloquate")
		&& (!((IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_FAKIE))))
		say_spell (ch, sn);
	if (!IS_IMMORTAL (ch))
		WAIT_STATE (ch, skill_table[sn].beats);
	if (number_percent () > get_skill (ch, sn))
	{
		send_to_char ("You lost your concentration.\n\r", ch);
		if (sn == skill_lookup ("enchant armor")
			|| sn == skill_lookup ("enchant weapon"))
			check_improve (ch, sn, FALSE, 4);

		else
			check_improve (ch, sn, FALSE, 1);
		ch->mana -= mana / 2;
	}

	else
	{
		char buf[20];
		CHAR_DATA *perp=NULL;
		AFFECT_DATA *af=NULL;
		ch->mana -= mana;
		if (IS_NPC (ch) || mana_using_Class (ch))
		{
			costs_mana = TRUE;
			(*skill_table[sn].spell_fun) (sn, ch->level, ch, vo, target);
			costs_mana = FALSE;
			//Only try to do the spell 90% of the time
			if (number_percent() <= 90)
			{
				for (af = ch->affected; af != NULL; af = af->next)
				{
					if (af->type == gsn_evil_twin)
						break;
				}
			}

			if (af != NULL && af->comp_name != NULL && ((perp = get_exact_pc_world(ch,af->comp_name)) != NULL)
				&& perp->fighting == ch && ch->in_room == perp->in_room)
			{
				//simulated 60% success rate
				if (number_percent() >= 60)
				{
					send_to_char("You lost your concentration.\n\r",perp);
				} 
				else 
				{
					if (target == TARGET_CHAR)
					{
						if (((CHAR_DATA*)vo) == ch)
							(*skill_table[sn].spell_fun) (sn, perp->level, perp, (void*)perp, target);
						else if (((CHAR_DATA*)vo) == perp)
							(*skill_table[sn].spell_fun) (sn, perp->level, perp, (void*)ch, target);
						else (*skill_table[sn].spell_fun) (sn, perp->level, perp, vo, target);
					}
					else if (target == TARGET_NONE || target == TARGET_ROOM)
						(*skill_table[sn].spell_fun) (sn, perp->level, perp, vo, target);
				}

			}
		}

		else
		{
			costs_mana = TRUE;
			(*skill_table[sn].spell_fun) (sn, 3 * ch->level / 4, ch, vo, target);
			costs_mana = FALSE;
			//Only try to do the spell 90% of the time
			if (number_percent() <= 90)
			{
				for (af = ch->affected; af != NULL; af = af->next)
				{
					if (af->type == gsn_evil_twin)
						break;
				}
			}

			//          (*skill_table[sn].spell_fun) (sn, 3 * ch->level / 4, ch, vo, target);
			if (af != NULL && af->comp_name != NULL && ((perp = get_exact_pc_world(ch,af->comp_name)) != NULL)
				&& perp->fighting == ch && ch->in_room == perp->in_room)
			{
				//simulated 60% success rate
				if (number_percent() >= 60)
				{
					send_to_char("You lost your concentration.\n\r",perp);
				}
				else 
				{
					if (target == TARGET_CHAR)
					{
						if (((CHAR_DATA*)vo) == ch)
							(*skill_table[sn].spell_fun) (sn, perp->level, perp, (void*)perp, target);
						else if (((CHAR_DATA*)vo) == perp)
							(*skill_table[sn].spell_fun) (sn, perp->level, perp, (void*)ch, target);
						else (*skill_table[sn].spell_fun) (sn, perp->level, perp, vo, target);
					}
					else if (target == TARGET_NONE || target == TARGET_ROOM)
						(*skill_table[sn].spell_fun) (sn, perp->level, perp, vo, target);
				}

			}
			//	  (*skill_table[sn].spell_fun) (sn, 3 * ch->level / 4, ch, vo,
			//			target);
		}
		sprintf(buf, "%d", sn);
		trip_triggers_arg(ch, OBJ_TRIG_CHAR_USES_SPELL, NULL, victim, OT_TYPE_SPELL, buf);
		if ((sn == skill_lookup ("enchant armor")) ||
			(sn == skill_lookup ("enchant weapon")))
		{
			check_improve (ch, sn, TRUE, 4);
		}

		else
		{
			check_improve (ch, sn, TRUE, 1);
		}
	}
	if (!str_cmp (skill_table[sn].name, "web")
		&& IS_AFFECTED (victim, AFF_CHARM) && victim->master == ch)
		return;
	if (!str_cmp (skill_table[sn].name,"chain lightning"))
		return;
	if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
		|| (skill_table[sn].target == TAR_OBJ_CHAR_OFF
		&& target == TARGET_CHAR)) && victim != ch && victim->master != ch)// && !IS_SET(ch->act2, ACT_ILLUSION))
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;
		for (vch = ch->in_room->people; vch; vch = vch_next)
		{
			vch_next = vch->next_in_room;
			if (victim == vch && victim->fighting == NULL
				&& get_position(victim) > POS_SLEEPING)
			{
				multi_hit (ch, victim, TYPE_UNDEFINED);
				break;
			}
		}
	}
	return;
}


/*
* Cast spells at targets using a magical object.
*/
void obj_cast_spell (int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj)
{
	obj_cast_spell2(sn,level,ch,victim,obj, "");
}

void obj_cast_spell2(int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, char* argument)
{
	void *vo;
	int target = TARGET_NONE;
	char buf[20];
	if (ch == NULL)
	{
		bug ("obj_cast_spell: NULL character (object user)", 0);
		return;
	}
	//verify that the victim is actually in the same room as the character 
	if (victim != NULL)
	{
		if ((ch->in_room) != (victim->in_room))
			return;
	}
	if (sn <= 0)
		return;
	if (sn >= MAX_SKILL || skill_table[sn].spell_fun == 0)
	{
		bug ("Obj_cast_spell: bad sn %d.", sn);
		return;
	}
	switch (skill_table[sn].target)
	{
	default:
		bug ("Obj_cast_spell: bad target for sn %d.", sn);
		return;
	case TAR_AREA_OFF:
	case TAR_IGNORE:
		vo = NULL;
		break;
	case TAR_CHAR_OFFENSIVE:
		if (victim == NULL)
			victim = ch->fighting;
		if (victim == NULL)
		{
			send_to_char ("You can't do that.\n\r", ch);
			return;
		}
		if (is_safe (ch, victim) && ch != victim)
		{
			send_to_char ("Something isn't right...\n\r", ch);
			return;
		}
		vo = (void *) victim;
		target = TARGET_CHAR;
		break;
	case TAR_CHAR_DEFENSIVE:
	case TAR_CHAR_SELF:
		if (victim == NULL)
			victim = ch;
		vo = (void *) victim;
		target = TARGET_CHAR;
		break;
	case TAR_OBJ_INV:
		if (obj == NULL)
		{
			send_to_char ("You can't do that.\n\r", ch);
			return;
		}
		vo = (void *) obj;
		target = TARGET_OBJ;
		break;
	case TAR_OBJ_CHAR_OFF:
		if (victim == NULL && obj == NULL)
		{
			if (ch->fighting != NULL)
				victim = ch->fighting;

			else
			{
				send_to_char ("You can't do that.\n\r", ch);
				return;
			}
		}
		if (victim != NULL)
		{
			if (is_safe_spell (ch, victim, FALSE) && ch != victim)
			{
				send_to_char ("Something isn't right...\n\r", ch);
				return;
			}
			vo = (void *) victim;
			target = TARGET_CHAR;
		}

		else
		{
			vo = (void *) obj;
			target = TARGET_OBJ;
		}
		break;
	case TAR_OBJ_CHAR_DEF:
		if (victim == NULL && obj == NULL)
		{
			vo = (void *) ch;
			target = TARGET_CHAR;
		}

		else if (victim != NULL)
		{
			vo = (void *) victim;
			target = TARGET_CHAR;
		}

		else
		{
			vo = (void *) obj;
			target = TARGET_OBJ;
		}
		break;
	case TAR_STRING:
		if (argument[0] == '\0')
			vo = NULL;
		else vo = (void*)argument;
		target = TARGET_NONE;
		break;
	}
	target_name = "";
	(*skill_table[sn].spell_fun) (sn, level, ch, vo, target);

	if (sn == skill_lookup("penance"))			//Iblis 7/28/03 - Quick fix.  I KNOW Hardcoding penance value is bad
		damage (ch, vo, 0, 1, 16, FALSE);
//		damage (ch, reinterpret_cast<CHAR_DATA *>(vo), 0, 1, 16, FALSE);
	sprintf(buf, "%d", sn);
	trip_triggers_arg(ch, OBJ_TRIG_CHAR_USES_SPELL, NULL, victim, OT_TYPE_SPELL, buf);
	if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
		|| (skill_table[sn].target == TAR_OBJ_CHAR_OFF
		&& target == TARGET_CHAR)) && victim != ch && victim->master != ch)
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;
		for (vch = ch->in_room->people; vch; vch = vch_next)
		{
			vch_next = vch->next_in_room;
			if (victim == vch && victim->fighting == NULL
				&& get_position(victim) > POS_SLEEPING)
			{
				multi_hit (ch, victim, TYPE_UNDEFINED);
				break;
			}
		}
	}
	return;
}


/*
* Spell functions.
*/
void spell_acid_blast (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	if (ch->in_room->sector_type == SECT_UNDERWATER)
	{
		act ("$n's acid blast fizzles out in the water.\n\r", ch, NULL,
			NULL, TO_ROOM);
		send_to_char ("Your acid blast fizzles in steam.\n\r", ch);
		return;
	}
	dam = dice (level, 12);
	dam = dam * 3 / 4;
	if (saves_spell (level, victim, DAM_ACID))
		dam /= 2;
	damage_old (ch, victim, dam, sn, DAM_ACID, TRUE);
	return;
}

void spell_armor (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if ((is_affected (victim, sn))
		|| is_affected (victim, skill_lookup ("conservancy")))
	{
		if (victim == ch)
			send_to_char ("You are already armored.\n\r", ch);

		else
			act ("$N is already armored.", ch, NULL, victim, TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = 24;
	af.modifier = -10;
	af.location = APPLY_AC;
	af.bitvector = 0;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("You feel someone protecting you.\n\r", victim);
	if (ch != victim)
		act ("$N is protected by your magic.", ch, NULL, victim, TO_CHAR);
	return;
}

void spell_fireshield (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	/*  if (victim->race == PC_RACE_LITAN)
	{
	send_to_char("This spell is of no use to you.  You can only get fireshield naturally.\n\r",victim);
	return;
	}*/

	if (victim->in_room->sector_type == SECT_WATER_OCEAN
		|| victim->in_room->sector_type == SECT_WATER_SWIM
		|| victim->in_room->sector_type == SECT_UNDERWATER
		|| IS_SET(victim->in_room->room_flags2,ROOM_SNOW))
	{
		if (victim == ch)
			send_to_char
			("Your shield of fire quickly evaporates into a useless steam.\n\r",
			ch);

		else
			act
			("$N's shield of fire quickly evaporates into a useless steam.\n\r",
			ch, NULL, victim, TO_CHAR);
		return;
	}
	if (is_affected (victim, sn))
	{
		if (victim == ch)
			send_to_char
			("You are already surrounded by a shield of fire.\n\r", ch);

		else
			act ("$N is already surrounded by a shield of fire.", ch, NULL,
			victim, TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = UMAX (level / 10, 3);
	af.modifier = -15;
	af.location = APPLY_AC;
	af.bitvector = 0;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("You are surrounded by a shield of fire.\n\r", victim);
	if (ch != victim)
		act ("$N is surrounded by your shield of fire.", ch, NULL, victim,
		TO_CHAR);
	if (is_affected (ch, gsn_entangle))
	{
		send_to_char ("The plants holding you break and fall away.\n\r", ch);
		act ("The plants holding $n break and fall away.", ch, NULL, NULL,
			TO_ROOM);
		affect_strip (ch, skill_lookup ("entangle"));
	}
	
		if (is_affected (ch, gsn_insect_plague))
	{
		send_to_char ("The dark cloud of insects surrounding you are destroyed by the shield of fire.\n\r", ch);
		act ("The dark cloud of insects surrounding $n are destroyed by the shield of fire.", ch, NULL, NULL,
			TO_ROOM);
		affect_strip (ch, skill_lookup ("insect plague"));
	}
	return;
}

int check_web (CHAR_DATA * ch)
{
	int chance, remdur, breakfree = FALSE;
	AFFECT_DATA *webaff, af;
	if (!is_affected (ch, skill_lookup ("web")))
		return (1);
	webaff = affect_find (ch->affected, skill_lookup ("web"));
	if (webaff == NULL)
		return (1);
	switch (get_curr_stat (ch, STAT_DEX))
	{
	default:
		chance = number_range (1, 4);
		remdur = webaff->duration * (number_range (0, 50) * 0.01);
	case 20:
	case 21:
	case 22:
	case 23:
		chance = number_range (1, 3);
		remdur = webaff->duration * (number_range (0, 75) * 0.01);
		break;
	case 24:
	case 25:
		chance = number_range (1, 2);
		remdur = webaff->duration * (number_range (0, 100) * 0.01);
		break;
	}
	if (chance == 2)
	{
		remdur = UMAX (remdur, 1);
		af.where = TO_AFFECTS;
		af.type = webaff->type;
		af.level = webaff->level;
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
				("The magical webs holding you dissolve and fall away.\n\r\n\r",
				ch);
			act ("The magical webs holding $n dissolve and fall away.", ch,
				NULL, NULL, TO_ROOM);
			affect_strip (ch, skill_lookup ("web"));
			breakfree = TRUE;
		}

		else if (remdur < 5)
		{
			send_to_char
				("You struggle weakly with the webs holding you, they seem to become less sticky.\n\r",
				ch);
			act
				("$n struggles weakly with the webs holding $m, they seem to become less sticky.",
				ch, NULL, NULL, TO_ROOM);
		}

		else
		{
			send_to_char
				("You tear at the webs holding you, ripping away large chunks.\n\r",
				ch);
			act ("$n tears at the webs holding $m, ripping away large chunks.",
				ch, NULL, NULL, TO_ROOM);
		}
	}

	else
	{
		send_to_char ("The webs holding you will not budge.\n\r", ch);
		act ("$n struggles ineffectually with the webs encasing $m.", ch,
			NULL, NULL, TO_ROOM);
	}
	return (breakfree);
}

void spell_web (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af, *webaff;
	if (is_affected (ch, skill_lookup ("web"))
		|| !saves_spell (level - 5, victim, DAM_NEGATIVE))
	{
		if (!is_affected (victim, skill_lookup ("web")))
			send_to_char ("You are held fast by sticky webs.\n\r", victim);

		else
			send_to_char
			("You are held even firmer by even MORE sticky webs.\n\r", victim);
		if (ch != victim)
		{
			if (!is_affected (ch, skill_lookup ("web")))
				act ("$N is covered in sticky webs.", ch, NULL, victim, TO_CHAR);

			else
				act ("$N is covered in a large mass of sticky webs.", ch,
				NULL, victim, TO_CHAR);
		}
		af.where = TO_AFFECTS;
		af.type = sn;
		af.level = level;
		af.duration = level / 6;
		af.modifier = 0;
		af.location = 0;
		af.bitvector = 0;
		af.permaff = FALSE;
		af.composition = FALSE;
		af.comp_name = str_dup ("");
		webaff = affect_find (ch->affected, skill_lookup ("web"));
		if (webaff != NULL)
		{
			if (webaff->duration + af.duration > 20)
			{
				affect_strip (ch, skill_lookup ("web"));
				af.duration = 20;
			}
		}

		else if (af.duration > 20)
			af.duration = 20;
		affect_join (victim, &af);
	}

	else
		act ("$N shrugs off your sticky webs.", ch, NULL, victim, TO_CHAR);
	return;
}

void spell_bark_skin (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (is_affected (ch, sn))
	{
		if (victim == ch)
			send_to_char ("Your skin is already as hard as bark.\n\r", ch);

		else
			act ("$N is already as hard as can be.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (is_affected (ch, skill_lookup ("stone skin")))
	{
		if (victim == ch)
			send_to_char ("Your skin can't get any harder.\n\r", ch);

		else
			act ("$N's skin is already as hard as can be.", ch, NULL,
			victim, TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level;
	af.location = APPLY_AC;
	af.modifier = -15;
	af.bitvector = 0;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	act ("$n's skin hardens into bark.", victim, NULL, NULL, TO_ROOM);
	send_to_char ("Your skin hardens into bark.\n\r", victim);
	return;
}

void spell_blindness (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (IS_AFFECTED (victim, AFF_BLIND)
		|| saves_spell (level, victim, DAM_OTHER)
		|| IS_SET(victim->imm_flags, IMM_BLIND))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.location = APPLY_HITROLL;
	af.modifier = -4;
	af.duration = 1 + level / 7;
	af.bitvector = AFF_BLIND;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("You have been blinded!\n\r", victim);
	act ("$n fumbles awkwardly, and appears to be blinded.", victim, NULL,
		NULL, TO_ROOM);
	return;
}

void spell_burning_hands (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const sh_int dam_each[] =
	{ 0, 0, 0, 0, 0, 14, 17, 20, 23, 26, 29, 29, 29, 30, 30, 31, 31,
	32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39,
	40, 40, 41, 41, 42, 42, 43,
	43, 44, 44, 45, 45, 46, 46, 47, 47, 48, 48
	};
	int dam;

	/* BEGIN UNDERWATER */
	if (ch->in_room->sector_type == SECT_UNDERWATER)
	{
		act ("$n's hands fizzle in steam!", ch, NULL, NULL, TO_ROOM);
		send_to_char ("Your attempt fizzles in steam.\n\r", ch);
		return;
	}

	/* END UNDERWATER */
	level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
	level = UMAX (0, level);
	dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
	if (saves_spell (level, victim, DAM_FIRE))
		dam /= 2;
	damage_old (ch, victim, dam, sn, DAM_FIRE, TRUE);
	return;
}

void spell_bee_sting (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const sh_int dam_each[] =
	{ 0, 0, 0, 0, 0, 14, 17, 20, 23, 26, 29, 29, 29, 30, 30, 31, 31,
	32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39,
	40, 40, 41, 41, 42, 42, 43,
	43, 44, 44, 45, 45, 46, 46, 47, 47, 48, 48
	};
	int dam;

	/* BEGIN UNDERWATER */
	if (ch->in_room->sector_type == SECT_UNDERWATER)
	{
		act ("$n calls forth a bee, which promptly drowns.", ch, NULL,
			NULL, TO_ROOM);
		send_to_char ("You call forth a bee, which promptly drowns.\n\r", ch);
		return;
	}

	/* END UNDERWATER */
	level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
	level = UMAX (0, level);
	dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
	if (saves_spell (level, victim, DAM_HARM))
		dam /= 2;
	if (victim->race == PC_RACE_SWARM)
		dam = 0;
	damage_old (ch, victim, dam, sn, DAM_HARM, TRUE);
	return;
}

void spell_ice_beam (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const sh_int dam_each[] =
	{ 0, 0, 0, 0, 0, 14, 17, 20, 23, 26, 29, 29, 29, 30, 30, 31, 31,
	32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39,
	40, 40, 41, 41, 42, 42, 43,
	43, 44, 44, 45, 45, 46, 46, 47, 47, 48, 48
	};
	int dam;
	if (ch->in_room->sector_type != SECT_UNDERWATER
		&& ch->in_room->sector_type != SECT_WATER_OCEAN
		&& ch->in_room->sector_type != SECT_WATER_SWIM)
	{
		send_to_char ("You have no water to create an ice beam with!\n\r", ch);
		return;
	}
	level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
	level = UMAX (0, level);
	dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
	if (saves_spell (level, victim, DAM_COLD))
		dam /= 2;
	damage_old (ch, victim, dam, sn, DAM_COLD, TRUE);
	return;
}


// Minax 7-3-02 I commented out the original Call Lightning code so that we can
// go back to it if we ever want to.  I changed call lightning to call down a
// single bolt of lightning that can be directed at a target.

/*
void
spell_call_lightning (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
CHAR_DATA *vch;
CHAR_DATA *vch_next;
int dam;

if (!IS_OUTSIDE (ch))
{
send_to_char ("You must be outdoors to use this spell.\n\r", ch);
return;
}

if (weather_info.sky < SKY_RAINING)
{
send_to_char ("You need bad weather.\n\r", ch);
return;
}

dam = dice (level / 2, 8);

send_to_char ("Lightning streaks from the heavens, striking your foes!\n\r",
ch);
act ("$n calls down a shower of lightning to strike $s foes!", ch, NULL,
NULL, TO_ROOM);

for (vch = char_list; vch != NULL; vch = vch_next)
{
vch_next = vch->next;
if (vch->in_room == NULL)
continue;
if (vch->in_room == ch->in_room)
{
if (vch != ch && (IS_NPC (ch) ? !IS_NPC (vch) : IS_NPC (vch)))
damage_old (ch, vch, saves_spell (level, vch, DAM_LIGHTNING)
? dam / 2 : dam, sn, DAM_LIGHTNING, TRUE);
continue;
}

if (vch->in_room->area == ch->in_room->area
&& IS_OUTSIDE (vch) && IS_AWAKE (vch))
send_to_char
("Lightning streaks through the air not far from you, illuminating the sky with mystical powers.\n\r",
vch);
}

return;
}
*/
void spell_call_lightning (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	if (!IS_OUTSIDE (ch))
	{
		send_to_char ("You must be outdoors to use this spell.\n\r", ch);
		return;
	}
	if (weather_info.sky < SKY_RAINING)
	{
		send_to_char
			("Without bad weather, your spell produces nothing but sparks.\n\r",
			ch);
		return;
	}

	/* BEGIN UNDERWATER */
	if (ch->in_room->sector_type == SECT_UNDERWATER)
	{
		act
			("$n's attempt to call a bolt of lightning results in fizzles of steam!",
			ch, NULL, NULL, TO_ROOM);
		send_to_char
			("You try to call a bolt of lightning, but just get a fizzle of steam.\n\r",
			ch);
		return;
	}

	/* END UNDERWATER */
	dam = dice (level, 12);
	dam = dam * 3 / 4;
	if (saves_spell (level, victim, DAM_LIGHTNING))
		dam /= 2;
	damage_old (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
	return;
}

void spell_cancellation (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	bool found = FALSE;
	level += 2;
	if ((!IS_NPC (ch) && IS_NPC (victim)
		&& !(IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim))
		|| (IS_NPC (ch) && !IS_NPC (victim)))
	{
		send_to_char ("You failed, try dispel magic.\n\r", ch);
		return;
	}
	if (saves_spell (level, victim, DAM_OTHER))
	{
		send_to_char ("You feel a brief tingling sensation.\n\r", victim);
		send_to_char ("You failed.\n\r", ch);
		return;
	}

	/* begin running through the spells */
	if (check_dispel (level, victim, skill_lookup ("armor")))
		found = TRUE;
	if (victim->race != PC_RACE_LITAN 
		&& check_dispel (level, victim, gsn_fireshield))
		found = TRUE;
	if (check_dispel (level, victim, gsn_bark_skin))
	{
		act ("$n's skin regains its normal texture.", victim, NULL, NULL,
			TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("bless")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("blindness")))
	{
		found = TRUE;
		act ("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
	}
	if (check_dispel (level, victim, skill_lookup ("calm")))
	{
		found = TRUE;
		act ("$n no longer looks so peaceful...", victim, NULL, NULL, TO_ROOM);
	}
	if (check_dispel (level, victim, skill_lookup ("Quiet Mountain Lake")))
	{
		found = TRUE;
		act ("$n looks less serene.", victim, NULL, NULL, TO_ROOM);
	}
	if (check_dispel (level, victim, skill_lookup ("change sex")))
	{
		found = TRUE;
		act ("$n looks more like $mself again.", victim, NULL, NULL, TO_ROOM);
	}
	if (check_dispel (level, victim, skill_lookup ("charm person")))
	{
		found = TRUE;
		act ("$n regains $s free will.", victim, NULL, NULL, TO_ROOM);
	}
	if (check_dispel (level, victim, skill_lookup ("chill touch")))
	{
		found = TRUE;
		act ("$n looks warmer.", victim, NULL, NULL, TO_ROOM);
	}
	if (check_dispel (level, victim, skill_lookup ("curse")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("detect evil")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("detect good")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("detect hidden")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("detect invis")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("detect hidden")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("detect magic")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("faerie fire")))
	{
		act ("$n's outline fades.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("fly")))
	{
		act ("$n falls to the ground!", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("frenzy")))
	{
		act ("$n no longer looks so wild.", victim, NULL, NULL, TO_ROOM);;
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("giant strength")))
	{
		act ("$n no longer looks so mighty.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("haste")))
	{
		act ("$n is no longer moving so quickly.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("infravision")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("invis")))
	{
		act ("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("mass invis")))
	{
		act ("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("pass door")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("protection evil")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("protection good")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("sanctuary")))
	{
		act ("The white aura around $n's body vanishes.", victim, NULL,
			NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("shield")))
	{
		act ("The shield protecting $n vanishes.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("sleep")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("slow")))
	{
		act ("$n is no longer moving so slowly.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("stone skin")))
	{
		act ("$n's skin regains its normal texture.", victim, NULL, NULL,
			TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("Ballad of Ole Rip")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("damnation")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("soul fire")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("Quiet Mountain Lake")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("weaken")))
	{
		act ("$n looks stronger.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (found)
		send_to_char ("Ok.\n\r", ch);

	else
		send_to_char ("Spell failed.\n\r", ch);
}

void spell_cause_light (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	damage_old (ch, (CHAR_DATA *) vo, dice (1, 8) + level / 3, sn, DAM_HARM,
		TRUE);
	return;
}

void spell_cause_critical (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	damage_old (ch, (CHAR_DATA *) vo, dice (3, 8) + level - 6, sn, DAM_HARM,
		TRUE);
	return;
}

void spell_cause_serious (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	damage_old (ch, (CHAR_DATA *) vo, dice (2, 8) + level / 2, sn, DAM_HARM,
		TRUE);
	return;
}

void spell_chain_lightning (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *tmp_vict, *last_vict, *next_vict;
	bool found;
	int dam;

	/* BEGIN UNDERWATER */
	if (ch->in_room->sector_type == SECT_UNDERWATER)
	{
		act ("$n attempts to source lightning and gets shocked!", ch,
			NULL, NULL, TO_ROOM);
		send_to_char ("Your attempt to source lightning shocks you!\n\r", ch);
		dam = dice (level, 2);
		damage_old (ch, ch, dam, sn, DAM_LIGHTNING, TRUE);
		return;
	}

	/* END UNDERWATER */
	/* first strike */
	act ("A lightning bolt leaps from $n's hand and arcs to $N.", ch,
		NULL, victim, TO_ROOM);
	act ("A lightning bolt leaps from your hand and arcs to $N.", ch,
		NULL, victim, TO_CHAR);
	act ("A lightning bolt leaps from $n's hand and hits you!", ch, NULL,
		victim, TO_VICT);
	dam = dice (level, 6);
	if (saves_spell (level, victim, DAM_LIGHTNING))
		dam /= 3;
	damage_old (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
	last_vict = victim;
	level -= 4;			/* decrement damage */

	/* new targets */
	while (level > 0 && ch && ch->in_room)
	{
		found = FALSE;
		for (tmp_vict = ch->in_room->people; tmp_vict != NULL;
			tmp_vict = next_vict)
		{
			next_vict = tmp_vict->next_in_room;
			if (!is_safe_spell (ch, tmp_vict, TRUE) && tmp_vict != last_vict)
			{
				found = TRUE;
				last_vict = tmp_vict;
				act ("The bolt arcs to $n!", tmp_vict, NULL, NULL, TO_ROOM);
				act ("The bolt hits you!", tmp_vict, NULL, NULL, TO_CHAR);
				dam = dice (level, 6);
				if (saves_spell (level, tmp_vict, DAM_LIGHTNING))
					dam /= 3;
				damage_old (ch, tmp_vict, dam, sn, DAM_LIGHTNING, TRUE);
				level -= 4;	/* decrement damage */
			}
		}			/* end target searching loop */
		if (!found)
		{			/* no target found, hit the caster */
			if (ch == NULL)
				return;
			if (last_vict == ch)
			{			/* no double hits */
				act ("The bolt seems to have fizzled out.", ch, NULL, NULL,
					TO_ROOM);
				act ("The bolt grounds out through your body.", ch, NULL,
					NULL, TO_CHAR);
				return;
			}
			last_vict = ch;
			act ("The bolt arcs to $n...whoops!", ch, NULL, NULL, TO_ROOM);
			send_to_char ("You are struck by your own lightning!\n\r", ch);
			dam = dice (level, 6);
			if (saves_spell (level, ch, DAM_LIGHTNING))
				dam /= 3;
			damage_old (ch, ch, dam, sn, DAM_LIGHTNING, TRUE);
			if (killed_by_chain)
			{
				killed_by_chain = FALSE;
				return;
			}
			level -= 4;		/* decrement damage */
			if (ch == NULL || ch->fighting == NULL)
				return;
		}

		/* now go back and find more targets */
	}
}


// Minax 7-8-02 Sex Change is a permanent toggle.  This not only makes it
// more fun, but it also makes it easier on players who get sick of other
// players casting it on them and having no recourse.  Now two more casts
// get them back to where they started.
void spell_change_sex (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	if (saves_spell (level, victim, DAM_OTHER))
	{
		act ("You feel a tingling down below, but it passes.", ch, NULL,
			NULL, TO_VICT);
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (victim->sex == 0)
	{
		victim->sex = 1;
		send_to_char ("You feel vastly different.\n\r", victim);
		act ("$n doesn't look like $mself anymore...", victim, NULL, NULL,
			TO_ROOM);
		return;
	}
	if (victim->sex == 1)
	{
		victim->sex = 2;
		send_to_char ("You feel vastly different.\n\r", victim);
		act ("$n doesn't look like $mself anymore...", victim, NULL, NULL,
			TO_ROOM);
		return;
	}
	if (victim->sex == 2)
	{
		victim->sex = 0;
		send_to_char ("You feel vastly different.\n\r", victim);
		act ("$n doesn't look like $mself anymore...", victim, NULL, NULL,
			TO_ROOM);
		return;
	}
}


/*  OLD CODE
void
spell_change_sex (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
CHAR_DATA *victim = (CHAR_DATA *) vo;
AFFECT_DATA af;

if (is_affected (victim, sn))
{
if (victim == ch)
send_to_char ("You have already been changed.\n\r", ch);
else
act ("$N has already had $s(?) sex changed.", ch, NULL, victim,
TO_CHAR);
return;
}
if (saves_spell (level, victim, DAM_OTHER))
return;
af.where = TO_AFFECTS;
af.type = sn;
af.level = level;
af.duration = 2 * level;
af.location = APPLY_SEX;
af.permaff = FALSE;
af.composition = FALSE;
af.comp_name = str_dup ("");
af.composition = FALSE;
af.comp_name = str_dup ("");
do
{
af.modifier = number_range (0, 2) - victim->sex;
}
while (af.modifier == 0);
af.bitvector = 0;
affect_to_char (victim, &af);
send_to_char ("You feel vastly different.\n\r", victim);
act ("$n doesn't look like $mself anymore...", victim, NULL, NULL, TO_ROOM);
return;
}
*/
bool can_charm (CHAR_DATA * ch)
{
	CHAR_DATA *fch;
	int has_charmed = 0;
	int ave;
	for (fch = char_list; fch; fch = fch->next)
	{
		if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM))
		{
			has_charmed++;
		}
	}
	if (has_charmed >= 8)
		return (FALSE);
	ave = (get_curr_stat (ch, STAT_CHA) + get_curr_stat (ch, STAT_INT)) / 2;
	switch (ave)
	{
		//Iblis - With the new xp system etc, you should not get 1 free charmie regardless of cha
	default:
		return FALSE;
	case 12:
	case 13:
		if (has_charmed > 0 || get_curr_stat (ch, STAT_CHA) < 6)
		{
			return (FALSE);
		}

		else
		{
			return (TRUE);
		}
		break;
	case 14:
	case 15:
	case 16:
		if (has_charmed > 1)
		{
			return (FALSE);
		}

		else
		{
			return (TRUE);
		}
		break;
	case 17:
	case 18:
		if (has_charmed > 2)
		{
			return (FALSE);
		}

		else
		{
			return (TRUE);
		}
		break;
	case 19:
	case 20:
		if (has_charmed > 3)
		{
			return (FALSE);
		}

		else
		{
			return (TRUE);
		}
		break;
	case 21:
	case 22:
		if (has_charmed > 4)
		{
			return (FALSE);
		}

		else
		{
			return (TRUE);
		}
		break;
	case 23:
		if (has_charmed > 5)
		{
			return (FALSE);
		}

		else
		{
			return (TRUE);
		}
		break;
	case 24:
		if (has_charmed > 6)
		{
			return (FALSE);
		}

		else
		{
			return (TRUE);
		}
		break;
	case 25:
		if (has_charmed > 7)
		{
			return (FALSE);
		}

		else
		{
			return (TRUE);
		}
		break;
	}				// end switch(ave)
	return (FALSE);
}

void spell_charm_person (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	short level2;
	if (is_safe (ch, victim))
		return;

	// Akamai: 10/29/98 -- Fixing Mage charm so that it does not affect
	// immortals and apprpriately covers level differences
	if (IS_IMMORTAL (victim))
	{
		send_to_char
			("You are worth no more than an askance look from an immortal.\n\r",
			ch);
		return;
	}
	if (victim == ch)
	{
		send_to_char ("You like yourself even better!\n\r", ch);
		return;
	}
	level2 = Class_level (ch, 0);
	if (level2 == 0)
		level2 = 1;
	if (IS_IMMORTAL (ch))
		level2 = ch->level;

	// if the victim is a real player +- MAGIC_LEVEL_DIFF from the level
	// of the mage then fail.
	if (!IS_NPC (victim) &&
		((victim->level >= (level2 + MAGIC_LEVEL_DIFF)) ||
		(victim->level <= (level2 - MAGIC_LEVEL_DIFF))))
	{
		send_to_char ("That person is some how beyond your charms.\n\r", ch);
		return;
	}
	// if the victim is a MOB + MAGIC_LEVEL_DIFF from the level of the
	// mage then the MOB is beyond them
	if (IS_NPC (victim) && (victim->level > (level2 + MAGIC_LEVEL_DIFF)))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (IS_AFFECTED (victim, AFF_CHARM)
		|| IS_AFFECTED (ch, AFF_CHARM)
		|| IS_SET (victim->imm_flags, IMM_CHARM)
		|| IS_SET (victim->act, ACT_UNDEAD)
		|| saves_spell_old (level2, victim, DAM_CHARM) || !can_charm (ch))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (IS_SET (victim->in_room->room_flags, ROOM_LAW))
	{
		send_to_char
			("The mayor does not allow charming in the city limits.\n\r", ch);
		return;
	}
	// Minax 2-13-03 Cannot cast charm in a no_charmie room
	if (IS_SET (ch->in_room->room_flags, ROOM_NO_CHARMIE))
	{
		send_to_char ("This room is resistant to your charms.\n\r", ch);
		return;
	}
	if (victim->master)
		stop_follower (victim);
	add_follower (victim, ch);
	victim->leader = ch;
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level2;
	if (!IS_IMMORTAL(ch))
	{
		af.duration = number_fuzzy (level2 / 4);
	}
	else
	{
		af.duration = -1;
	}
	af.location = 0;
	af.modifier = 0;
	af.bitvector = AFF_CHARM;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	act ("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
	act ("$N looks at you with adoring eyes.", ch, NULL, victim, TO_CHAR);
	return;
}

void spell_chill_touch (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const sh_int dam_each[] =
	{ 0, 0, 0, 6, 7, 8, 9, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16,
	16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 21,
	22, 22, 22, 23, 23, 23,
	24, 24, 24, 25, 25, 25, 26, 26, 26, 27
	};
	AFFECT_DATA af;
	int dam;
	level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
	level = UMAX (0, level);
	dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
	if (!saves_spell (level, victim, DAM_COLD))
	{
		act ("$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM);
		af.where = TO_AFFECTS;
		af.type = sn;
		af.level = level;
		af.duration = 6;
		af.location = APPLY_STR;
		af.modifier = -1;
		af.bitvector = 0;
		af.permaff = FALSE;
		af.composition = FALSE;
		af.comp_name = str_dup ("");
		affect_join (victim, &af);
	}

	else
	{
		dam /= 2;
	}
	damage_old (ch, victim, dam, sn, DAM_COLD, TRUE);
	return;
}

void spell_colour_spray (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const sh_int dam_each[] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30, 35, 40, 45, 50, 55, 55, 55,
	56, 57, 58, 58, 59, 60, 61, 61, 62, 63, 64, 64, 65, 66, 67, 67, 68,
	69, 70, 70, 71, 72, 73,
	73, 74, 75, 76, 76, 77, 78, 79, 79
	};
	int dam;
	level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
	level = UMAX (0, level);
	dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
	if (saves_spell (level, victim, DAM_LIGHT))
		dam /= 2;

	else
		spell_blindness (skill_lookup ("blindness"), level / 2, ch,
		(void *) victim, TARGET_CHAR);
	damage_old (ch, victim, dam, sn, DAM_LIGHT, TRUE);
	return;
}

void spell_control_weather (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	if (!str_cmp (target_name, "better"))
		weather_info.change += dice (level, 4);

	else if (!str_cmp (target_name, "worse"))
		weather_info.change -= dice (level, 4);

	else
		send_to_char ("Do you want it to get better or worse?\n\r", ch);
	send_to_char ("Ok.\n\r", ch);
	return;
}

void spell_create_food (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	OBJ_DATA *mushroom;
	mushroom = create_object (get_obj_index (OBJ_VNUM_MUSHROOM), 0);
	mushroom->value[0] = level / 2;
	mushroom->value[1] = level;

	/* BEGIN UNDERWATER */
	if (ch->in_room->sector_type == SECT_UNDERWATER)
	{
		act ("$p suddenly appears then disintegrates to mush.", ch,
			mushroom, NULL, TO_ROOM);
		act ("$p suddenly appears then disintegrates to mush.", ch,
			mushroom, NULL, TO_CHAR);
		return;
	}

	/* END UNDERWATER */
	obj_to_room (mushroom, ch->in_room);
	free_string(mushroom->name);
	free_string(mushroom->short_descr);
	free_string(mushroom->description);
	switch (ch->race)
	{
	case PC_RACE_HUMAN :
		mushroom->description = str_dup("`dA `klemon`d pie topped with fluffy `hwhite meringue`d looks yummy for humans.``");
		mushroom->short_descr = str_dup("`da `kLemon `hMeringue `dpie``");
		mushroom->name = str_dup("pie lemon meringue");
		break;
	case PC_RACE_ELF :
		mushroom->description = str_dup("`dA thin crispy wafer wrapped in a large `cgreen leaf`d indicates that an elf has passed by.``");
		mushroom->short_descr = str_dup("`da crispy wafer in a `cgreen leaf``");
		mushroom->name = str_dup("crispy wafer leaf");
		break;
	case PC_RACE_SYVIN :
		mushroom->description = str_dup("`dA large salad of `cgreen lettuce`d and `kc`io`jl`do`br`kf`iu`jl `dv`be`kg`ie`jt`da`bb`kl`ie`js`d looks like a syvin feast.``");
		mushroom->short_descr = str_dup("`da salad with `cgreen lettuce`d and `kc`io`jl`do`br`kf`iu`jl `dv`be`kg`ie`jt`da`bb`kl`ie`js``");
		mushroom->name = str_dup("salad feast");
		break;
	case PC_RACE_DWARF :
		mushroom->description = str_dup("`dThis is not a `astone`d, it's a hard loaf of dwarf bread, laced with `kgold flakes`d.``");
		mushroom->short_descr = str_dup("`da `astoney`d loaf of dwarf bread with `kgold flakes``");
		mushroom->name = str_dup("loaf bread");
		break;
	case PC_RACE_SIDHE :
		mushroom->description = str_dup("`dYou don't think that this `fwispy cotton candy`d could fill any belly but a sidhe's.``");
		mushroom->short_descr = str_dup("`fwispy cotton candy``");
		mushroom->name = str_dup("wispy cotton candy");
		break;
	case PC_RACE_VROATH :
		mushroom->description = str_dup("`dYou give this huge odorous pile of `bbloody flesh`d a very wide berth.``");
		mushroom->short_descr = str_dup("`da huge smelly pile of `bbloody flesh``");
		mushroom->name = str_dup("bloody flesh pile");
		break;
	case PC_RACE_CANTHI :
		mushroom->description = str_dup("`dThis fillet of `lbluefin tuna`d wrapped in `gfresh seaweed`d came from under the sea.``");
		mushroom->short_descr = str_dup("`da fillet of `lbluefin tuna`d wraped in `gfresh seaweed``");
		mushroom->name = str_dup("fillet bluefin tuna");
		break;
	case PC_RACE_LITAN :
		mushroom->description = str_dup("`dIf you ate this `bred hot`d salsa you would probably burst into `iflames.``");
		mushroom->short_descr = str_dup("`bred hot`d salsa``");
		mushroom->name = str_dup("hot salsa");
		break;
	case PC_RACE_AVATAR :
		mushroom->description = str_dup("`dSomeone has smuggled this `omanna of the gods`d out of the heavens.``");
		mushroom->short_descr = str_dup("`omanna of the gods``");
		mushroom->name = str_dup("manna gods");
		break;
	case PC_RACE_NERIX :
		mushroom->description = str_dup("`dA pile of `kn`cu`dt`ks`d and `cs`de`ke`dd`ks`d sits here attracting a hungry nerix.``");
		mushroom->short_descr = str_dup("`da pile of `kn`cu`dt`ks`d and `cs`de`ke`dd`ks``");
		mushroom->name = str_dup("pile nuts seeds");
		break;
	case PC_RACE_LICH :
		mushroom->description = str_dup("`aA large pile of brains is here, attracting flies and zombies.``");
		mushroom->short_descr = str_dup("`aA large pile of brains.``");
		mushroom->name = str_dup("pile brains");
		break;
	default :
		mushroom->short_descr = str_dup("`nA `mM`fa`gg`ni`mc `fM`gu`ns`mh`fr`go`no`mm``");
		mushroom->description = str_dup("A delicious magic mushroom is here.");
		mushroom->name = str_dup("magic mushroom");
		break;
	}

	if (number_percent() <= 7)
	{
		free_string(mushroom->name);
		free_string(mushroom->short_descr);
		free_string(mushroom->description);
		mushroom->short_descr = str_dup("`nA `mM`fa`gg`ni`mc `fM`gu`ns`mh`fr`go`no`mm``");
		mushroom->description = str_dup("A delicious magic mushroom is here.");
		mushroom->name = str_dup("magic mushroom");
	}
	act ("$p suddenly appears.", ch, mushroom, NULL, TO_ROOM);
	act ("$p suddenly appears.", ch, mushroom, NULL, TO_CHAR);
	return;
}

void spell_create_rose (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	OBJ_DATA *rose;
	rose = create_object (get_obj_index (OBJ_VNUM_ROSE), 0);
	act ("$n has created a beautiful red rose.", ch, rose, NULL, TO_ROOM);
	send_to_char ("You create a beautiful red rose.\n\r", ch);
	obj_to_char (rose, ch);
	return;
}

void spell_create_spring (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	OBJ_DATA *spring;
	spring = create_object (get_obj_index (OBJ_VNUM_SPRING), 0);
	spring->timer = level;

	/* BEGIN UNDERWATER */
	if (ch->in_room->sector_type == SECT_UNDERWATER)
	{
		act
			("$p flows from the ground and is diluted by the surrounding water.",
			ch, spring, NULL, TO_ROOM);
		act
			("$p flows from the ground and is diluted by the surrounding water.",
			ch, spring, NULL, TO_CHAR);
		return;
	}

	/* END UNDERWATER */
	obj_to_room (spring, ch->in_room);
	act ("$p flows from the ground.", ch, spring, NULL, TO_ROOM);
	act ("$p flows from the ground.", ch, spring, NULL, TO_CHAR);
	return;
}

void spell_create_water (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int water;
	if (obj->item_type != ITEM_DRINK_CON)
	{
		send_to_char ("It is unable to hold water.\n\r", ch);
		return;
	}
	if (obj->value[2] != LIQ_WATER && obj->value[1] != 0)
	{
		send_to_char ("It contains some other liquid.\n\r", ch);
		return;
	}
	water =
		UMIN (level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
		obj->value[0] - obj->value[1]);
	if (water > 0)
	{
		obj->value[2] = LIQ_WATER;
		obj->value[1] += water;
		if (!is_name ("water", obj->name))
		{
			char buf[MAX_STRING_LENGTH];
			sprintf (buf, "%s water", obj->name);
			free_string (obj->name);
			obj->name = str_dup (buf);
		}
		act ("$p is filled.", ch, obj, NULL, TO_CHAR);
	}
	return;
}

void spell_cure_blindness (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	if (!is_affected (victim, gsn_blindness))
	{
		if (victim == ch)
			send_to_char ("You can already see perfectly fine.\n\r", ch);

		else
			act ("$N does not appear to be blinded.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (check_dispel (level, victim, gsn_blindness))
	{
		send_to_char
			("With a sudden flash of white light, your vision miraculously returns!\n\r",
			victim);
		act ("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
	}

	else
		send_to_char ("Alas, your spell failed.\n\r", ch);
}

void spell_cure_critical (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;
	heal = dice (3, 8) + level / 3;
	if (is_affected(victim,gsn_greymantle) || IS_SET(victim->act,ACT_UNDEAD))
		reverse_heal(ch,victim,heal,sn);
	else if (victim->race == PC_RACE_SWARM)
		swarm_heal(ch,victim,sn);
	else
	{
		if (!IS_NPC(victim) && victim->pcdata->soul_link)
		{
			victim->hit = UMIN (victim->hit + heal/2, victim->max_hit);
			victim->pcdata->soul_link->hit = UMIN (victim->pcdata->soul_link->hit + heal/2,
				victim->pcdata->soul_link->max_hit);
			update_pos (victim->pcdata->soul_link);
			send_to_char ("A healing force courses through your body!\n\r", victim->pcdata->soul_link);
		}

		else victim->hit = UMIN (victim->hit + heal, victim->max_hit);
		update_pos (victim);
		send_to_char ("A healing force courses through your body!\n\r", victim);
	}
	if (ch != victim)
		send_to_char ("Ok.\n\r", ch);
	return;
}

void spell_herb_lore (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;
	if (ch->in_room->sector_type != SECT_FIELD
		&& ch->in_room->sector_type != SECT_FOREST
		&& ch->in_room->sector_type != SECT_GRASSLAND)
	{
		send_to_char
			("You couldn't find the needed herbs in these surroundings.\n\r", ch);
		return;
	}
	heal = 20;
	if (number_percent () < 6)
		heal = 100;
	if (!IS_NPC(victim) && victim->pcdata->soul_link)
	{
		victim->hit = UMIN (victim->hit + heal/2, victim->max_hit);
		victim->pcdata->soul_link->hit = UMIN (victim->pcdata->soul_link->hit + heal/2,
			victim->pcdata->soul_link->max_hit);
		update_pos (victim->pcdata->soul_link);
		send_to_char ("A healing force courses through your body!\n\r", victim->pcdata->soul_link);
	}
	else if (victim->race == PC_RACE_SWARM)
	{
		swarm_heal(ch,victim,sn);
		return;
	}
	else victim->hit = UMIN (victim->hit + heal, victim->max_hit);
	update_pos (victim);
	if (ch != victim)
	{
		act ("$n's knowledge of herbs allows you to replenish your strength.", ch,NULL,victim,TO_VICT);
		send_to_char ("Ok.\n\r", ch);
	}
	else send_to_char ("Your knowledge of herbs allows you to replenish your strength.\n\r",ch);
	return;
}


/* RT added to cure plague */
void spell_cure_disease (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	bool work = FALSE;
	/*  if (!is_affected (victim, gsn_plague))
	{
	if (victim == ch)
	send_to_char
	("You are not currently stricken with the plague.\n\r", ch);

	else
	act ("$N does not currently appear to be diseased.", ch, NULL,
	victim, TO_CHAR);
	//  return;
	}*/
	if (check_dispel (level, victim, gsn_plague))
		work = TRUE;
	if (check_dispel (level, victim, gsn_pox))
		work = TRUE;
	if (check_dispel (level, victim, gsn_atrophy))
		work = TRUE;
	if (check_dispel (level, victim, gsn_contaminate))
	{
		work = TRUE;
		send_to_char("You feel the impurities leaving your body.",victim);
	}
	if (check_dispel (level, victim, gsn_brain_blisters))
	{
		affect_strip(victim,skill_lookup("disorientation"));
		work = TRUE;
	}
	if (check_dispel (level, victim, gsn_vomit))
		work = TRUE;

	if (work)
	{
		send_to_char ("Your sores vanish.\n\r", victim);
		act ("$n looks relieved as $s sores vanish.", victim, NULL, NULL,
			TO_ROOM);
	}

	else
		send_to_char ("Alas, the spell failed.\n\r", ch);
}

void spell_cure_light (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;
	heal = dice (1, 8) + level / 3;
	if (is_affected(victim,gsn_greymantle) || IS_SET(victim->act,ACT_UNDEAD))
		reverse_heal(ch,victim,heal,sn);
	else if (victim->race == PC_RACE_SWARM)
		swarm_heal(ch,victim,sn);
	else
	{
		if (!IS_NPC(victim) && victim->pcdata->soul_link)
		{
			victim->hit = UMIN (victim->hit + heal/2, victim->max_hit);
			victim->pcdata->soul_link->hit = UMIN (victim->pcdata->soul_link->hit + heal/2, 
				victim->pcdata->soul_link->max_hit);
			update_pos (victim->pcdata->soul_link);
			send_to_char ("A healing force courses through your body!\n\r", victim->pcdata->soul_link);

		}
		else victim->hit = UMIN (victim->hit + heal, victim->max_hit);
		update_pos (victim);
		send_to_char ("You feel better!\n\r", victim);
	}
	if (ch != victim)
		send_to_char ("Ok.\n\r", ch);
	return;
}

void spell_cure_poison (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	if (!is_affected (victim, gsn_poison))
	{
		if (victim == ch)
			send_to_char
			("You do not appear to be poisoned at the moment.\n\r", ch);

		else
			act ("$N doesn't appear to be poisoned.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (check_dispel (level, victim, gsn_poison))
	{
		send_to_char ("A warm feeling runs through your body.\n\r", victim);
		act ("$n visage lightens considerably, and begins to look much better.",
			victim, NULL, NULL, TO_ROOM);
	}

	else
		send_to_char ("Alas, the spell failed.\n\r", ch);
}

void spell_cure_serious (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;
	heal = dice (2, 8) + level / 2;
	if (is_affected(victim,gsn_greymantle) || IS_SET(victim->act,ACT_UNDEAD))
		reverse_heal(ch,victim,heal,sn);
	else if (victim->race == PC_RACE_SWARM)
		swarm_heal(ch,victim,sn);
	else
	{
		if (!IS_NPC(victim) && victim->pcdata->soul_link)
		{
			victim->hit = UMIN (victim->hit + heal/2, victim->max_hit);
			victim->pcdata->soul_link->hit = UMIN (victim->pcdata->soul_link->hit + heal/2,
				victim->pcdata->soul_link->max_hit);
			update_pos (victim->pcdata->soul_link);
			send_to_char ("A healing force courses through your body!\n\r", victim->pcdata->soul_link);

		}
		else victim->hit = UMIN (victim->hit + heal, victim->max_hit);
		update_pos (victim);
		send_to_char ("You feel better!\n\r", victim);
	}
	if (ch != victim)
		send_to_char ("Ok.\n\r", ch);
	return;
}

void spell_curse (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;

	/* deal with the object case first */
	if (target == TARGET_OBJ)
	{
		obj = (OBJ_DATA *) vo;
		if (IS_OBJ_STAT (obj, ITEM_EVIL))
		{
			act ("$p is already filled with evil.", ch, obj, NULL, TO_CHAR);
			return;
		}
		if (IS_OBJ_STAT (obj, ITEM_BLESS))
		{
			AFFECT_DATA *paf;
			paf = affect_find (obj->affected, skill_lookup ("bless"));
			if (!saves_dispel (level, paf != NULL ? paf->level : obj->level, 0))
			{
				if (paf != NULL)
					affect_remove_obj (obj, paf);
				act ("$p glows with a red aura.", ch, obj, NULL, TO_ALL);
				REMOVE_BIT (obj->extra_flags[0], ITEM_BLESS);
				return;
			}

			else
			{
				act ("The holy aura of $p is too powerful for you to overcome.",
					ch, obj, NULL, TO_CHAR);
				return;
			}
		}
		af.where = TO_OBJECT;
		af.type = sn;
		af.level = level;
		af.duration = level/8;
		af.location = APPLY_SAVES;
		af.modifier = +1;
		af.bitvector = ITEM_EVIL;
		af.permaff = FALSE;
		af.composition = FALSE;
		af.comp_name = str_dup ("");
		affect_to_obj (obj, &af);
		SET_BIT (obj->extra_flags[0], ITEM_NODROP);
		act ("$p glows with a malevolent aura.", ch, obj, NULL, TO_ALL);
		return;
	}

	/* character curses */
	victim = (CHAR_DATA *) vo;
	if (IS_AFFECTED (victim, AFF_CURSE)
		|| saves_spell (level, victim, DAM_NEGATIVE))
	{
		send_to_char ("Spell failed.\n\r", ch);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = 2 * level;
	af.location = APPLY_HITROLL;
	af.modifier = -1 * (level / 8);
	af.bitvector = AFF_CURSE;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	af.location = APPLY_SAVING_SPELL;
	af.modifier = level / 8;
	affect_to_char (victim, &af);
	send_to_char ("You feel unclean.\n\r", victim);
	if (ch != victim)
		act ("$N looks very uncomfortable.", ch, NULL, victim, TO_CHAR);
	return;
}

void spell_demonfire (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	if (!IS_NPC (ch) && !IS_EVIL (ch))
	{
		victim = ch;
		send_to_char ("The demons turn upon you!\n\r", ch);
	}
	change_alignment(ch,-50);
	if (victim != ch)
	{
		act ("$n calls forth the demons of Hell upon $N!", ch, NULL,
			victim, TO_ROOM);
		act ("$n has assailed you with the demons of Hell!", ch, NULL,
			victim, TO_VICT);
		send_to_char ("You conjure forth the demons of hell!\n\r", ch);
	}
	dam = dice (level, 10);
	if (saves_spell (level, victim, DAM_NEGATIVE))
		dam /= 2;
	damage_old (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);
	if (victim->hit > 1)
		prayer_damnation (gsn_curse, 3 * level / 4, ch, (void *) victim, TARGET_CHAR);
}

void spell_detect_evil (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (IS_AFFECTED (victim, AFF_DETECT_EVIL))
	{
		if (victim == ch)
			send_to_char ("You can already sense evil.\n\r", ch);

		else
			act ("$N can already detect evil.", ch, NULL, victim, TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_DETECT_EVIL;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("Your eyes tingle.\n\r", victim);
	if (ch != victim)
		send_to_char ("Ok.\n\r", ch);
	return;
}

void spell_detect_good (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (IS_AFFECTED (victim, AFF_DETECT_GOOD))
	{
		if (victim == ch)
			send_to_char ("You can already sense good.\n\r", ch);

		else
			act ("$N can already detect good.", ch, NULL, victim, TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_DETECT_GOOD;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("Your eyes tingle.\n\r", victim);
	if (ch != victim)
		send_to_char ("Ok.\n\r", ch);
	return;
}

void spell_detect_hidden (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (IS_AFFECTED (victim, AFF_DETECT_HIDDEN))
	{
		if (victim == ch)
			send_to_char ("You are already as alert as you can be. \n\r", ch);

		else
			act ("$N can already sense hidden lifeforms.", ch, NULL,
			victim, TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = AFF_DETECT_HIDDEN;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("Your awareness improves.\n\r", victim);
	if (ch != victim)
		send_to_char ("Ok.\n\r", ch);
	return;
}

void spell_detect_invis (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (IS_AFFECTED (victim, AFF_DETECT_INVIS))
	{
		if (victim == ch)
			send_to_char ("You can already see invisible.\n\r", ch);

		else
			act ("$N can already see invisible things.", ch, NULL, victim,
			TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_DETECT_INVIS;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("Your eyes tingle.\n\r", victim);
	if (ch != victim)
		send_to_char ("Ok.\n\r", ch);
	return;
}

void spell_detect_magic (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (IS_AFFECTED (victim, AFF_DETECT_MAGIC))
	{
		if (victim == ch)
			send_to_char ("You can already sense magical auras.\n\r", ch);

		else
			act ("$N can already detect magic.", ch, NULL, victim, TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_DETECT_MAGIC;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("Your eyes glow a brilliant `jgreen``.\n\r", victim);
	if (ch != victim)
		send_to_char ("Ok.\n\r", ch);
	return;
}

void spell_detect_poison (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	if (obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD)
	{
		if (obj->value[3] != 0)
			send_to_char ("You smell poisonous fumes.\n\r", ch);

		else
			send_to_char ("It looks delicious.\n\r", ch);
	}

	else
	{
		send_to_char ("It doesn't appear to be poisoned.\n\r", ch);
	}
	return;
}

void spell_dispel_evil (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	if (!IS_NPC (ch) && IS_EVIL (ch))
		victim = ch;
	if (IS_GOOD (victim))
	{
		act ("$N is protected by the forces of Good.", ch, NULL, victim,
			TO_ROOM);
		return;
	}
	if (IS_NEUTRAL (victim))
	{
		act ("$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (victim->hit > (ch->level * 4))
		dam = dice (level, 4);

	else
		dam = UMAX (victim->hit, dice (level, 4));
	if (saves_spell (level, victim, DAM_HOLY))
		dam /= 2;
	dam = dam * 5 / 4;
	damage_old (ch, victim, dam, sn, DAM_HOLY, TRUE);
	return;
}

void spell_dispel_good (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	if (!IS_NPC (ch) && IS_GOOD (ch))
		victim = ch;
	if (IS_EVIL (victim))
	{
		act ("$N is protected by $S evil.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (IS_NEUTRAL (victim))
	{
		act ("$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (victim->hit > (ch->level * 4))
		dam = dice (level, 4);

	else
		dam = UMAX (victim->hit, dice (level, 4));
	if (saves_spell (level, victim, DAM_NEGATIVE))
		dam /= 2;
	damage_old (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);
	dam = dam * 5 / 4;
	return;
}


/* modified for enhanced use */
void spell_dispel_magic (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	bool found = FALSE;
	if (saves_spell (level, victim, DAM_OTHER))
	{
		send_to_char ("You feel a brief tingling sensation.\n\r", victim);
		send_to_char ("You failed.\n\r", ch);
		return;
	}

	/* begin running through the spells */
	if (check_dispel (level, victim, skill_lookup ("armor")))
		found = TRUE;
	if (victim->race != PC_RACE_LITAN 
		&& check_dispel (level, victim, gsn_fireshield))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("bless")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("blindness")))
	{
		found = TRUE;
		act ("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
	}
	if (check_dispel (level, victim, skill_lookup ("calm")))
	{
		found = TRUE;
		act ("$n no longer looks so peaceful...", victim, NULL, NULL, TO_ROOM);
	}
	if (check_dispel (level, victim, skill_lookup ("Quiet Mountain Lake")))
	{
		found = TRUE;
		act ("$n looks less serene.", victim, NULL, NULL, TO_ROOM);
	}
	if (check_dispel (level, victim, skill_lookup ("change sex")))
	{
		found = TRUE;
		act ("$n looks more like $mself again.", victim, NULL, NULL, TO_ROOM);
	}
	if (check_dispel (level, victim, skill_lookup ("charm person")))
	{
		found = TRUE;
		act ("$n regains $s free will.", victim, NULL, NULL, TO_ROOM);
	}
	if (check_dispel (level, victim, skill_lookup ("chill touch")))
	{
		found = TRUE;
		act ("$n looks warmer.", victim, NULL, NULL, TO_ROOM);
	}
	if (check_dispel (level, victim, skill_lookup ("curse")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("detect evil")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("detect good")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("detect invis")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("detect magic")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("faerie fire")))
	{
		act ("$n's outline fades.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("fly")))
	{
		act ("$n falls to the ground!", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("frenzy")))
	{
		act ("$n no longer looks so wild.", victim, NULL, NULL, TO_ROOM);;
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("giant strength")))
	{
		act ("$n no longer looks so mighty.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("haste")))
	{
		act ("$n is no longer moving so quickly.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("infravision")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("invis")))
	{
		act ("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("mass invis")))
	{
		act ("$n fades into existence.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("pass door")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("protection evil")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("protection good")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("sanctuary")))
	{
		act ("The white aura around $n's body vanishes.", victim, NULL,
			NULL, TO_ROOM);
		found = TRUE;
	}
	if (IS_AFFECTED (victim, AFF_SANCTUARY)
		&& !saves_dispel (level, victim->level, -1)
		&& !is_affected (victim, skill_lookup ("sanctuary")))
	{
		REMOVE_BIT (victim->affected_by, AFF_SANCTUARY);
		act ("The white aura around $n's body vanishes.", victim, NULL,
			NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("shield")))
	{
		act ("The shield protecting $n vanishes.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("sleep")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("Ballad of Ole Rip")))
		found = TRUE;
	if (check_dispel (level, victim, skill_lookup ("slow")))
	{
		act ("$n is no longer moving so slowly.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("stone skin")))
	{
		act ("$n's skin regains its normal texture.", victim, NULL, NULL,
			TO_ROOM);
		found = TRUE;
	}
	if (check_dispel (level, victim, skill_lookup ("weaken")))
	{
		act ("$n looks stronger.", victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
	if (found)
		send_to_char ("Ok.\n\r", ch);

	else
		send_to_char ("Spell failed.\n\r", ch);
	return;
}

void spell_firestorm (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	if (ch->in_room->sector_type == SECT_WATER_OCEAN
		|| ch->in_room->sector_type == SECT_WATER_SWIM
		|| ch->in_room->sector_type == SECT_UNDERWATER)
	{
		send_to_char
			("Your storm of fire quickly evaporates into scalding steam.\n\r",
			ch);
		act
			("$n summons a storm of fire that evaporates into a scalding steam.",
			ch, NULL, NULL, TO_ROOM);
		damage_old (ch, ch, level / 5, sn, DAM_FIRE, TRUE);
		return;
	}
	send_to_char ("A storm of fire rains down from the heavens!\n\r", ch);
	act ("$n summons a storm of fire that rains down from the heavens.",
		ch, NULL, NULL, TO_ROOM);
	for (vch = char_list; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next;
		if (vch->in_room == NULL)
			continue;
		if (vch == ch)
			continue;
		if (vch->in_room == ch->in_room)
		{
			if (!is_safe_spell (ch, vch, FALSE))
			{
				int dam;
				dam = dice (level, 6);
				if (saves_spell (level, vch, DAM_FIRE))
					dam /= 5;
				damage_old (ch, vch, dam, sn, DAM_FIRE, TRUE);
			}
		}

		else if (vch->in_room->area == ch->in_room->area)
			send_to_char
			("In the distance, a storm of fire rains down from the sky.\n\r",
			vch);
	}
	return;
}

void spell_bee_swarm (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	if (ch->in_room->sector_type == SECT_UNDERWATER)
	{
		act ("$n calls forth a swarm of bees, which promptly drown.", ch,
			NULL, NULL, TO_ROOM);
		send_to_char
			("You call forth a swarm of bees, which promptly drown.\n\r", ch);
		return;
	}
	send_to_char ("A swarm of bees emerges!\n\r", ch);
	act ("$n summons a swarm of bees!", ch, NULL, NULL, TO_ROOM);
	for (vch = char_list; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next;
		if (vch->in_room == NULL)
			continue;
		if (ch != vch && vch->in_room == ch->in_room)
		{
			if (!is_safe_spell (ch, vch, FALSE))
			{
				int dam;
				if ((IS_AFFECTED (vch, AFF_CHARM) && vch->leader == ch) || vch->race == PC_RACE_SWARM)
					continue;
				dam = level + dice (3, 6);
				if (saves_spell (level, vch, DAM_HARM))
					dam /= 4;
				damage_old (ch, vch, dam, sn, DAM_HARM, TRUE);
				if (number_range
					(1, 100 + get_curr_stat (vch, STAT_CON) * 2) == 50
					&& vch->level < 92)
					// if (1==1)
				{
					insta_kill (ch, vch, 0, 0, 0, 0, 1);

					//raw_kill (vch, ch);
				}
			}
		}

		else if (vch != ch && vch->in_room->area == ch->in_room->area)
			send_to_char
			("A swarm of bees can be heard buzzing in the distance.\n\r", vch);
	}
	return;
}

void spell_enchant_armor (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf;
	int result, fail;
	int ac_bonus, added;
	bool ac_found = FALSE;
	if (obj->item_type != ITEM_ARMOR)
	{
		send_to_char ("That isn't an armor.\n\r", ch);
		return;
	}
	if (obj->wear_loc != -1)
	{
		send_to_char ("The item must be carried to be enchanted.\n\r", ch);
		return;
	}

	/* this means they have no bonus */
	ac_bonus = 0;
	fail = 35;

	/* find the bonuses */
	if (!obj->enchanted)
		for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
		{
			if (paf->location == APPLY_AC)
			{
				ac_bonus = paf->modifier;
				ac_found = TRUE;
				fail += 10 * (ac_bonus * ac_bonus);
			}

			else			/* things get a little harder */
				fail += 20;
		}
		for (paf = obj->affected; paf != NULL; paf = paf->next)
		{
			if (paf->location == APPLY_AC)
			{
				ac_bonus = paf->modifier;
				ac_found = TRUE;
				fail += 10 * (ac_bonus * ac_bonus);
			}

			else			/* things get a little harder */
				fail += 20;
		}

		/* KLUDGE */
		if (ac_bonus < -6)
			fail += 45;

		/* apply other modifiers */
		fail -= level / 3;
		if (IS_OBJ_STAT (obj, ITEM_BLESS))
			fail -= 15;
		if (IS_OBJ_STAT (obj, ITEM_GLOW))
			fail -= 5;
		fail = URANGE (5, fail, 85);
		result = number_percent ();

		/* the moment of truth */
		if ((result < (fail / 5) || IS_OBJ_STAT (obj, ITEM_NOENCHANT))&& !IS_IMMORTAL (ch))
		{				/* item destroyed */
			act ("$p flares blindingly... and evaporates!", ch, obj, NULL, TO_CHAR);
			act ("$p flares blindingly... and evaporates!", ch, obj, NULL, TO_ROOM);
			extract_obj (obj);
			return;
		}
		if (result < (fail / 3) && !IS_IMMORTAL (ch))
		{				/* item disenchanted */
			AFFECT_DATA *paf_next;
			long vector_to_remove = FALSE;
			int i;
			act ("$p glows brightly, then fades...oops.", ch, obj, NULL, TO_CHAR);
			act ("$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM);
			obj->enchanted = TRUE;

			/* remove all affects */
			for (paf = obj->affected; paf != NULL; paf = paf_next)
			{
				paf_next = paf->next;
				if (paf->bitvector > 0)
					vector_to_remove = paf->bitvector;

				//IBLIS 5/22/03 - Fix it so fading a honed weapon doesn't leave it perm-sharp
				free_affect (paf);
				if (vector_to_remove)
				{
					obj->value[4] = obj->value[4] & !vector_to_remove;
					vector_to_remove = 0;
				}
			}
			obj->affected = NULL;

			/* clear all flags */
			for (i=0;i<MAX_EXTRA_FLAGS;i++)	      
				obj->extra_flags[i] = 0;
			return;
		}
		if (result <= fail && !IS_IMMORTAL (ch))
		{				/* failed, no bad result */
			send_to_char ("Nothing seemed to happen.\n\r", ch);
			return;
		}

		/* okay, move all the old flags into new vectors if we have to */
		if (!obj->enchanted)
		{
			AFFECT_DATA *af_new;
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
				af_new->permaff = paf->permaff;
				af_new->composition = FALSE;
				af_new->comp_name = str_dup ("");
			}
		}
		if (result <= (90 - level / 5))
		{				/* success! */
			act ("$p shimmers with a gold aura.", ch, obj, NULL, TO_CHAR);
			act ("$p shimmers with a gold aura.", ch, obj, NULL, TO_ROOM);
			SET_BIT (obj->extra_flags[0], ITEM_MAGIC);
			added = -1;
		}

		else
			/* exceptional enchant */
		{
			act ("$p glows a brillant gold!", ch, obj, NULL, TO_CHAR);
			act ("$p glows a brillant gold!", ch, obj, NULL, TO_ROOM);
			SET_BIT (obj->extra_flags[0], ITEM_MAGIC);
			SET_BIT (obj->extra_flags[0], ITEM_GLOW);
			added = -2;
		}

		/* now add the enchantments */
		if (obj->level < LEVEL_HERO)
			obj->level = UMIN (LEVEL_HERO - 1, obj->level + 1);
		if (ac_found)
		{
			for (paf = obj->affected; paf != NULL; paf = paf->next)
			{
				if (paf->location == APPLY_AC)
				{
					paf->type = sn;
					paf->modifier += added;
					paf->level = UMAX (paf->level, level);
				}
			}
		}

		else
			/* add a new affect */
		{
			paf = new_affect ();
			paf->where = TO_OBJECT;
			paf->type = sn;
			paf->level = level;
			paf->duration = -1;
			paf->location = APPLY_AC;
			paf->modifier = added;
			paf->permaff = FALSE;
			paf->bitvector = 0;
			paf->next = obj->affected;
			obj->affected = paf;
		}
}
void spell_enchant_weapon (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf;
	int result, fail,i;
	int hit_bonus, dam_bonus, added;
	bool hit_found = FALSE, dam_found = FALSE;
	if (obj->item_type != ITEM_WEAPON)
	{
		send_to_char ("That isn't a weapon.\n\r", ch);
		return;
	}
	if (obj->wear_loc != -1)
	{
		send_to_char ("The item must be carried to be enchanted.\n\r", ch);
		return;
	}

	/* this means they have no bonus */
	hit_bonus = 0;
	dam_bonus = 0;
	fail = 35;

	/* find the bonuses */
	if (!obj->enchanted)
		for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
		{
			if (paf->location == APPLY_HITROLL)
			{
				hit_bonus = paf->modifier;
				hit_found = TRUE;
				fail += 10 * (hit_bonus * hit_bonus);
			}

			else if (paf->location == APPLY_DAMROLL)
			{
				dam_bonus = paf->modifier;
				dam_found = TRUE;
				fail += 10 * (dam_bonus * dam_bonus);
			}

			else			/* things get a little harder */
				fail += 25;
		}
		for (paf = obj->affected; paf != NULL; paf = paf->next)
		{
			if (paf->location == APPLY_HITROLL)
			{
				hit_bonus = paf->modifier;
				hit_found = TRUE;
				fail += 2 * (hit_bonus * hit_bonus);
			}

			else if (paf->location == APPLY_DAMROLL)
			{
				dam_bonus = paf->modifier;
				dam_found = TRUE;
				fail += 2 * (dam_bonus * dam_bonus);
			}

			else			/* things get a little harder */
				fail += 25;
		}

		/* apply other modifiers */
		fail -= level / 2;
		if (IS_OBJ_STAT (obj, ITEM_BLESS))
			fail -= 15;
		if (IS_OBJ_STAT (obj, ITEM_GLOW))
			fail -= 5;
		fail = URANGE (5, fail, 95);
		if (IS_OBJ_STAT (obj, ITEM_NOENCHANT))
			fail = 95;

		result = number_percent ();

		/* the moment of truth */
		if ((result < (fail / 5) || IS_OBJ_STAT (obj, ITEM_NOENCHANT))&& !IS_IMMORTAL (ch))
		{				/* item destroyed */
			act ("$p shivers violently and explodes!", ch, obj, NULL, TO_CHAR);
			act ("$p shivers violently and explodeds!", ch, obj, NULL, TO_ROOM);
			extract_obj (obj);
			return;
		}
		if (result < (fail / 2) && !IS_IMMORTAL (ch))
		{				/* item disenchanted */
			AFFECT_DATA *paf_next;
			act ("$p glows brightly, then fades...oops.", ch, obj, NULL, TO_CHAR);
			act ("$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM);
			obj->enchanted = TRUE;

			/* remove all affects */
			for (paf = obj->affected; paf != NULL; paf = paf_next)
			{
				if (paf->where == TO_WEAPON && paf->permaff == FALSE)
				{
					if (paf->bitvector == WEAPON_SHARP)
						if (obj->value[4] & WEAPON_SHARP)
							obj->value[4] -= WEAPON_SHARP;
					if (paf->bitvector == WEAPON_POISON)
						if (obj->value[4] & WEAPON_POISON)
							obj->value[4] -= WEAPON_POISON;
					if (paf->bitvector == WEAPON_VAMPIRIC)
						if (obj->value[4] & WEAPON_VAMPIRIC)
							obj->value[4] -= WEAPON_VAMPIRIC;
				}
				paf_next = paf->next;
				free_affect (paf);
			}
			obj->affected = NULL;

			/* clear all flags */
			for (i=0;i<MAX_EXTRA_FLAGS;i++)
				obj->extra_flags[i] = 0;
			return;
		}
		if (result <= fail && !IS_IMMORTAL (ch))
		{				/* failed, no bad result */
			send_to_char ("Nothing seemed to happen.\n\r", ch);
			return;
		}

		/* okay, move all the old flags into new vectors if we have to */
		if (!obj->enchanted)
		{
			AFFECT_DATA *af_new;
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
				af_new->permaff = paf->permaff;
				af_new->composition = FALSE;
				af_new->comp_name = str_dup ("");
			}
		}
		if (result <= (100 - level / 5))
		{				/* success! */
			act ("$p glows blue.", ch, obj, NULL, TO_CHAR);
			act ("$p glows blue.", ch, obj, NULL, TO_ROOM);
			SET_BIT (obj->extra_flags[0], ITEM_MAGIC);
			added = 1;
		}

		else
			/* exceptional enchant */
		{
			act ("$p glows a brillant blue!", ch, obj, NULL, TO_CHAR);
			act ("$p glows a brillant blue!", ch, obj, NULL, TO_ROOM);
			SET_BIT (obj->extra_flags[0], ITEM_MAGIC);
			SET_BIT (obj->extra_flags[0], ITEM_GLOW);
			added = 2;
		}

		/* now add the enchantments */
		if (obj->level < LEVEL_HERO - 1)
			obj->level = UMIN (LEVEL_HERO - 1, obj->level + 1);
		if (dam_found)
		{
			for (paf = obj->affected; paf != NULL; paf = paf->next)
			{
				if (paf->location == APPLY_DAMROLL)
				{
					paf->where = TO_OBJECT;
					paf->type = sn;
					paf->modifier += added;
					paf->level = UMAX (paf->level, level);
					if (paf->modifier > 4)
						SET_BIT (obj->extra_flags[0], ITEM_HUM);
				}
			}
		}

		else
			/* add a new affect */
		{
			paf = new_affect ();
			paf->where = TO_OBJECT;
			paf->type = sn;
			paf->level = level;
			paf->duration = -1;
			paf->location = APPLY_DAMROLL;
			paf->modifier = added;
			paf->bitvector = 0;
			paf->permaff = FALSE;
			paf->composition = FALSE;
			paf->comp_name = str_dup ("");
			paf->next = obj->affected;
			obj->affected = paf;
		}
		if (hit_found)
		{
			for (paf = obj->affected; paf != NULL; paf = paf->next)
			{
				if (paf->location == APPLY_HITROLL)
				{
					paf->type = sn;
					paf->where = TO_OBJECT;
					paf->modifier += added;
					paf->level = UMAX (paf->level, level);
					if (paf->modifier > 4)
						SET_BIT (obj->extra_flags[0], ITEM_HUM);
				}
			}
		}

		else
			/* add a new affect */
		{
			paf = new_affect ();
			paf->where = TO_OBJECT;
			paf->type = sn;
			paf->level = level;
			paf->duration = -1;
			paf->location = APPLY_HITROLL;
			paf->modifier = added;
			paf->bitvector = 0;
			paf->permaff = FALSE;
			paf->composition = FALSE;
			paf->comp_name = str_dup ("");
			paf->next = obj->affected;
			obj->affected = paf;
		}
}


/*
* Drain XP, MANA, HP.
* Caster gains HP.
*/
void spell_energy_drain (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	if (victim != ch)
		change_alignment(ch,-50);
	if (saves_spell (level, victim, DAM_NEGATIVE) && number_percent() < 20)
	{
		send_to_char ("You feel a momentary chill.\n\r", victim);
		send_to_char ("You failed.\n\r",ch);
		return;
	}
	if (victim->level <= 2)
	{
		dam = ch->hit + 1;
	}

	else
	{
		//      gain_exp (victim, 0 - number_range (level / 2, 3 * level / 2));
		victim->mana -= victim->mana*.05;
		victim->move -= victim->move*.05;
		dam = dice (1, level);
		ch->hit += dam/10;
	}
	send_to_char ("You feel your life slipping away!\n\r", victim);
	send_to_char ("Wow....what a rush!\n\r", ch);
	damage_old (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);
	return;
}

void spell_fireball (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const sh_int dam_each[] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 30, 34, 39, 45,
	52, 58, 63, 67, 74, 79, 82, 84, 86, 88, 90, 92, 94, 96, 98, 100,
	102, 104, 106, 108, 110,
	112, 114, 116, 118, 120, 122, 124, 126, 128, 130
	};
	int dam;

	/* BEGIN UNDERWATER */
	if (ch->in_room->sector_type == SECT_UNDERWATER)
	{
		act ("$n's attempt to create a fireball fizzles in steam!", ch,
			NULL, NULL, TO_ROOM);
		send_to_char
			("You try to create a fireball, but it fizzles in steam.\n\r", ch);
		return;
	}

	/* END UNDERWATER */
	level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
	level = UMAX (0, level);
	dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
	if (saves_spell (level, victim, DAM_FIRE))
		dam /= 2;
	damage_old (ch, victim, dam, sn, DAM_FIRE, TRUE);
	return;
}

void spell_fireproof (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA af;
	if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF))
	{
		act ("$p is already protected from burning.", ch, obj, NULL, TO_CHAR);
		return;
	}
	af.where = TO_OBJECT;
	af.type = sn;
	af.level = level;
	af.duration = number_fuzzy (level / 4);
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = ITEM_BURN_PROOF;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_obj (obj, &af);
	act ("You protect $p from fire.", ch, obj, NULL, TO_CHAR);
	act ("$p is surrounded by a protective aura.", ch, obj, NULL, TO_ROOM);
}

void spell_flamestrike (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	if (ch->in_room->sector_type == SECT_UNDERWATER
		|| ch->in_room->sector_type == SECT_WATER_OCEAN
		|| ch->in_room->sector_type == SECT_WATER_SWIM)
	{
		act ("$n's attempt at a flamestrike fizzles in steam!", ch, NULL,
			NULL, TO_ROOM);
		send_to_char ("Your flamestrike fizzles in your hands!\n\r", ch);
		return;
	}
	dam = dice (6 + level / 2, 8);
	if (saves_spell (level, victim, DAM_FIRE))
		dam /= 2;
	damage_old (ch, victim, dam, sn, DAM_FIRE, TRUE);
	return;
}

void spell_faerie_fire (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (IS_AFFECTED (victim, AFF_FAERIE_FIRE))
		return;
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level/9;
	af.location = APPLY_AC;
	af.modifier = level;
	af.bitvector = AFF_FAERIE_FIRE;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("You are surrounded by a pink outline.\n\r", victim);
	act ("$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM);
	return;
}

void spell_faerie_fog (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *ich;
	act ("$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM);
	send_to_char ("You conjure a cloud of purple smoke.\n\r", ch);
	for (ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room)
	{
		if (ich->invis_level > 0)
			continue;
		if (ich == ch || saves_spell (level, ich, DAM_OTHER))
			continue;
		affect_strip (ich, gsn_invis);
		affect_strip (ich, gsn_heavenly_cloak);
		affect_strip (ich, gsn_mass_invis);
		affect_strip (ich, gsn_sneak);
		affect_strip (ich, gsn_astral_body);
		affect_strip (ch, gsn_vanish);
		REMOVE_BIT (ich->affected_by, AFF_HIDE);
		REMOVE_BIT (ich->affected_by, AFF_CAMOUFLAGE);
		REMOVE_BIT (ich->affected_by, AFF_INVISIBLE);
		REMOVE_BIT (ich->affected_by, AFF_SNEAK);
		act ("$n is revealed!", ich, NULL, NULL, TO_ROOM);
		send_to_char ("You are revealed!\n\r", ich);
	}
	return;
}

void spell_floating_disc (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	OBJ_DATA *disc, *floating;
	floating = get_eq_char (ch, WEAR_FLOAT);
	if (floating != NULL && IS_OBJ_STAT (floating, ITEM_NOREMOVE))
	{
		act ("You can't remove $p.", ch, floating, NULL, TO_CHAR);
		return;
	}
	disc = create_object (get_obj_index (OBJ_VNUM_DISC), 0);
	disc->value[0] = ch->level * 10;	/* 10 pounds per level capacity */
	disc->value[3] = ch->level * 5;	/* 5 pounds per level max per item */
	disc->timer = ch->level * 2 - number_range (0, level / 2);
	act ("$n has created a floating black disc.", ch, NULL, NULL, TO_ROOM);
	send_to_char ("You create a floating disc.\n\r", ch);
	obj_to_char (disc, ch);
	wear_obj (ch, disc, TRUE, 0);
	return;
}

void spell_fly (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

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
	af.duration = level + 3;
	af.location = 0;
	af.modifier = 0;
	af.bitvector = AFF_FLYING;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("Your feet rise off the ground.\n\r", victim);
	act ("$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM);
	return;
}

void spell_gate (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim;
	char tcbuf[15];
	if (nogate || (!IS_NPC(ch) && ch->pcdata->nogate))
	{
		send_to_char("The gods are not allowing that at the moment.\n\r",ch);
		return;
	}
	if ((victim = get_char_world (ch, target_name)) == NULL
		|| (!IS_NPC (victim))
		|| victim == ch
		|| victim->in_room == NULL || !can_see_room (ch, victim->in_room)
		|| room_is_private (victim->in_room)
		|| (victim->in_room->area->construct)
		|| IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
		|| IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL)
		|| IS_SET (victim->imm_flags, IMM_SUMMON)
		//    || saves_spell (level, victim, DAM_OTHER)
		|| (IS_SET(victim->in_room->area->area_flags,AREA_IMP_ONLY) && get_trust(ch) < MAX_LEVEL)
		)
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (exploration_tracking)
		if (getbit (ch->pcdata->explored, victim->in_room->vnum) == 0)
		{
			send_to_char ("Your know nothing about the location you are trying to reach\n\r", ch);
			return;         
		}


		if (IS_SET (victim->in_room->race_flags, ROOM_NOTRANSPORT))
		{
			send_to_char ("You cannot travel there via ordinary magic.\n\r", ch);
			return;
		}
		if (IS_SET (ch->in_room->race_flags, ROOM_NOTRANSPORT))
		{
			send_to_char
				("You cannot leave this realm via ordinary magic.\n\r", ch);
			return;
		}
		if (!IS_NPC (ch))
			if ((time (NULL) - ch->pcdata->last_fight < 60))
			{
				send_to_char
					("You cannot muster the concentration to travel the realms in that way right now.\n\r",
					ch);
				return;
			}
			if ((battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner))) && !br_vnum_check (victim->in_room->vnum))
			{
				send_to_char
					("The area you are trying to gate to is now off limits until the end of Battle Royale.\n\r",
					ch);
				return;
			}
			if (ch->pcdata && ch->pcdata->fishing)
			{
				send_to_char ("You can't gate anywhere while fishing!.\n\r", ch);
				return;
			}
			act ("$n steps through a gate and vanishes.", ch, NULL, NULL, TO_ROOM);
			send_to_char ("You step through a gate and vanish.\n\r", ch);
			char_from_room (ch);
			char_to_room (ch, victim->in_room);
			act ("$n arrives through a rip etched from the fabric of existence.",
				ch, NULL, NULL, TO_ROOM);
			do_look (ch, "auto");
			trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
			trap_check(ch,"room",ch->in_room,NULL);
			sprintf(tcbuf,"%d",ch->in_room->vnum);
			trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
			check_aggression(ch);
			return;
}

void spell_giant_strength (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (is_affected (victim, sn))
	{
		if (victim == ch)
			send_to_char ("You are already as strong as you can get!\n\r", ch);

		else
			act ("$N can't get any stronger.", ch, NULL, victim, TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level;
	af.location = APPLY_STR;

	//IBLIS 5/19/03 - New stats require this to be weakened
	//  af.modifier = 1 + (level >= 18) + (level >= 25) + (level >= 32);
	af.modifier = 1 + (level >= 75);
	af.bitvector = 0;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("Your muscles surge with heightened power!\n\r", victim);
	act ("$n's muscles surge with heightened power.", victim, NULL, NULL,
		TO_ROOM);
	return;
}

void spell_harm (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	dam = UMAX (20, victim->hit - dice (1, 4));
	if (saves_spell (level, victim, DAM_HARM))
		dam = UMIN (50, dam / 2);
	dam = UMIN (100, dam);
	damage_old (ch, victim, dam, sn, DAM_HARM, TRUE);
	return;
}


/* RT haste spell */
void spell_haste (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (is_affected (victim, sn) || IS_AFFECTED (victim, AFF_HASTE)
		|| IS_SET (victim->off_flags, OFF_FAST))
	{
		if (victim == ch)
			send_to_char ("You can't move any faster!\n\r", ch);

		else
			act ("$N is already moving as fast as $E can.", ch, NULL,
			victim, TO_CHAR);
		return;
	}
	if (IS_AFFECTED (victim, AFF_SLOW))
	{
		if (!check_dispel (level, victim, skill_lookup ("slow")))
		{
			if (victim != ch)
				send_to_char ("Spell failed.\n\r", ch);
			send_to_char ("You feel momentarily faster.\n\r", victim);
			return;
		}
		act ("$n is moving less slowly.", victim, NULL, NULL, TO_ROOM);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	if (victim == ch)
		af.duration = level / 2;

	else
		af.duration = level / 4;
	af.location = APPLY_DEX;

	//IBLIS 5/19/03 - New stats need this to be weakened
	//  af.modifier = 1 + (level >= 18) + (level >= 25) + (level >= 32);
	af.modifier = 1 + (level >= 75);
	af.bitvector = AFF_HASTE;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("You feel yourself moving more quickly.\n\r", victim);
	act ("$n is moving more quickly.", victim, NULL, NULL, TO_ROOM);
	if (ch != victim)
		send_to_char ("Ok.\n\r", ch);
	return;
}

void spell_heal (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	if (is_affected(victim,gsn_greymantle) || IS_SET(victim->act,ACT_UNDEAD))
		reverse_heal(ch,victim,100,sn);
	else if (victim->race == PC_RACE_SWARM)
		swarm_heal(ch,victim,sn);
	else
	{
		if (!IS_NPC(victim) && victim->pcdata->soul_link)
		{
			victim->hit = UMIN (victim->hit + 50, victim->max_hit);
			victim->pcdata->soul_link->hit = UMIN (victim->pcdata->soul_link->hit + 50,
				victim->pcdata->soul_link->max_hit);
			update_pos (victim->pcdata->soul_link);
			send_to_char ("A warm feeling fills your body.\n\r", victim->pcdata->soul_link);
		} 
		else victim->hit = UMIN (victim->hit + 100, victim->max_hit);
		update_pos (victim);
		send_to_char ("A warm feeling fills your body.\n\r", victim);
	}
	if (ch != victim)
		send_to_char ("Ok.\n\r", ch);
	return;
}

void spell_greater_heal (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	if (is_affected(victim,gsn_greymantle) || IS_SET(victim->act,ACT_UNDEAD))
		reverse_heal(ch,victim,250,sn);
	else if (victim->race == PC_RACE_SWARM)
		swarm_heal(ch,victim,sn);
	else
	{ 
		if (!IS_NPC(victim) && victim->pcdata->soul_link)
		{
			victim->hit = UMIN (victim->hit + 125, victim->max_hit);
			victim->pcdata->soul_link->hit = UMIN (victim->pcdata->soul_link->hit + 125,
				victim->pcdata->soul_link->max_hit);
			update_pos (victim->pcdata->soul_link);
			send_to_char ("A warm feeling fills your body.\n\r", victim->pcdata->soul_link);
		}
		else victim->hit = UMIN (victim->hit + 250, victim->max_hit);
		update_pos (victim);
		send_to_char ("A warm feeling fills your body.\n\r", victim);
	}
	if (ch != victim)
		send_to_char ("Ok.\n\r", ch);
	return;
}

void spell_restore (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	if (is_affected(victim,gsn_greymantle) || IS_SET(victim->act,ACT_UNDEAD))
		reverse_heal(ch,victim,500,sn);
	else if (victim->race == PC_RACE_SWARM)
		swarm_heal(ch,victim,sn);
	else
	{
		if (!IS_NPC(victim) && victim->pcdata->soul_link)
		{
			victim->hit = UMIN (victim->hit + 400, victim->max_hit);
			victim->pcdata->soul_link->hit = UMIN (victim->pcdata->soul_link->hit + 400,
				victim->pcdata->soul_link->max_hit);
			update_pos (victim->pcdata->soul_link);
			send_to_char ("A warm feeling fills your body.\n\r", victim->pcdata->soul_link);
		}
		else victim->hit = victim->max_hit;
		update_pos (victim);
		send_to_char ("A warm feeling fills your body.\n\r", victim);
	}
	if (ch != victim)
		send_to_char ("Ok.\n\r", ch);
	return;
}

void spell_heat_metal (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj_lose, *obj_next;
	int dam = 0;
	bool fail = TRUE;
	if (!saves_spell (level + 2, victim, DAM_FIRE)
		&& !IS_SET (victim->imm_flags, IMM_FIRE) && !IS_IMMORTAL(victim))
	{
		for (obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next)
		{
			obj_next = obj_lose->next_content;
			if (number_range (1, 2 * level) > obj_lose->level
				&& !saves_spell (level, victim, DAM_FIRE)
				&& !IS_OBJ_STAT (obj_lose, ITEM_NONMETAL)
				&& !IS_OBJ_STAT (obj_lose, ITEM_BURN_PROOF))
			{
				switch (obj_lose->item_type)
				{
				case ITEM_ARMOR:
					if (obj_lose->wear_loc != -1)
					{		/* remove the item */
						if (can_drop_obj (victim, obj_lose)
							&& (obj_lose->weight / 10) < number_range (1,
							2 *
							get_curr_stat
							(victim,
							STAT_DEX))
							&& remove_obj (victim, obj_lose->wear_loc, TRUE))
						{
							act ("$n yelps and throws $p to the ground!",
								victim, obj_lose, NULL, TO_ROOM);
							act ("You remove and drop $p before it burns you.",
								victim, obj_lose, NULL, TO_CHAR);
							dam += (number_range (1, obj_lose->level) / 3);
							obj_from_char (obj_lose);
							obj_to_room (obj_lose, victim->in_room);
							fail = FALSE;
						}

						else
							/* stuck on the body! ouch! */
						{
							act ("Your skin is seared by $p!", victim,
								obj_lose, NULL, TO_CHAR);
							dam += (number_range (1, obj_lose->level));
							fail = FALSE;
						}
					}

					else
						/* drop it if we can */
					{
						if (can_drop_obj (victim, obj_lose))
						{
							act ("$n yelps and throws $p to the ground!",
								victim, obj_lose, NULL, TO_ROOM);
							act ("You remove and drop $p before it burns you.",
								victim, obj_lose, NULL, TO_CHAR);
							dam += (number_range (1, obj_lose->level) / 6);
							obj_from_char (obj_lose);
							obj_to_room (obj_lose, victim->in_room);
							fail = FALSE;
						}

						else
							/* cannot drop */
						{
							act ("Your skin is seared by $p!", victim,
								obj_lose, NULL, TO_CHAR);
							dam += (number_range (1, obj_lose->level) / 2);
							fail = FALSE;
						}
					}
					break;
				case ITEM_WEAPON:
					if (obj_lose->wear_loc != -1)
					{		/* try to drop it */
						if (IS_WEAPON_STAT (obj_lose, WEAPON_FLAMING))
							continue;
						if (can_drop_obj (victim, obj_lose)
							&& remove_obj (victim, obj_lose->wear_loc, TRUE))
						{
							act
								("$n is burned by $p, and throws it to the ground.",
								victim, obj_lose, NULL, TO_ROOM);
							send_to_char
								("You throw your red-hot weapon to the ground!\n\r",
								victim);
							dam += 1;
							obj_from_char (obj_lose);
							obj_to_room (obj_lose, victim->in_room);
							fail = FALSE;
						}

						else
							/* YOWCH! */
						{
							send_to_char
								("Your weapon sears your flesh!\n\r", victim);
							dam += number_range (1, obj_lose->level);
							fail = FALSE;
						}
					}

					else
						/* drop it if we can */
					{
						if (can_drop_obj (victim, obj_lose))
						{
							act ("$n throws a burning hot $p to the ground!",
								victim, obj_lose, NULL, TO_ROOM);
							act ("You remove and drop $p before it burns you.", victim,
								obj_lose, NULL, TO_CHAR);
							dam += (number_range (1, obj_lose->level) / 6);
							obj_from_char (obj_lose);
							obj_to_room (obj_lose, victim->in_room);
							fail = FALSE;
						}

						else
							/* cannot drop */
						{
							act ("Your skin is seared by $p!", victim,
								obj_lose, NULL, TO_CHAR);
							dam += (number_range (1, obj_lose->level) / 2);
							fail = FALSE;
						}
					}
					break;
				}
			}
		}
	}
	if (fail)
	{
		send_to_char ("Your spell had no effect.\n\r", ch);
		send_to_char ("You feel momentarily warmer.\n\r", victim);
	}

	else
		/* damage! */
	{
		if (saves_spell (level, victim, DAM_FIRE))
			dam = 2 * dam / 3;
		damage_old (ch, victim, dam, sn, DAM_FIRE, TRUE);
	}
}
void spell_identify (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	char buf[MAX_STRING_LENGTH];
	AFFECT_DATA *paf;
	if (IS_OBJ_STAT (obj, ITEM_NOIDENTIFY))
	{
		send_to_char
			("You cannot glean any more information about this object.\n\r", ch);
		return;
	}
	sprintf (buf,
		"Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d, ",
		obj->name, item_type_name (obj),
		extra_bit_name_new (obj->extra_flags,FALSE), obj->weight / 10,
		obj->cost, obj->level);
	send_to_char (buf, ch);
	if (obj->wear_flags > ITEM_TAKE && obj->item_type != ITEM_LIGHT)
	{
		sprintf (buf, "worn: %s\n\r",wear_bit_name (obj->wear_flags - ITEM_TAKE));
		send_to_char (buf, ch);
	}

	// Adeon 7/2/03 -- Finally updating ident for Class/race/clan flags
	/*  sprintf (buf, "   Used by - Races: %s Classes: %s Clans: %s\n\r",
	crflag_string (pcrace_flags, obj->race_flags),
	crflag_string (Class_flags, obj->Class_flags),
	crflag_string (clan_flags, obj->clan_flags));
	send_to_char (buf, ch);*/
	//Iblis - he did it wrong so I fixed it
	sprintf (buf, "   Used by - Races: %s", crflag_string (pcrace_flags, obj->race_flags));
	send_to_char (buf,ch);
	sprintf (buf, " Classes: %s",crflag_string (Class_flags, obj->Class_flags));
	send_to_char (buf,ch);
	sprintf (buf, " Clans: %s\n\r",crflag_string (clan_flags, obj->clan_flags));
	send_to_char (buf, ch);

	//Iblis 7/6/03 - Adding condition as requested
	sprintf (buf, "Condition : %s\n\r", condition_noun (obj->condition));
	send_to_char (buf, ch);

	//Iblis 11/06/03 - Added Owner (if applicable)
	if (obj->plr_owner != NULL)
	{
		sprintf (buf, "Owner : %s\n\r", capitalize(obj->plr_owner));
		send_to_char (buf, ch);
	}

	switch (obj->item_type)
	{
	case ITEM_VIAL:
		if (obj->value[0] > 0 && obj->value[0] < MAX_SKILL)
		{
			sprintf (buf, "Level %d spell of: '%s'\n\r", obj->level,
				skill_table[obj->value[0]].name);
			send_to_char (buf, ch);
		}

		else
			send_to_char ("Vial is empty.\n\r", ch);
		break;
	case ITEM_PARCHMENT:
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		sprintf (buf, "Level %d spells of:", obj->value[0]);
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
		sprintf (buf, "Has %d charges of level %d", obj->value[2],
			obj->value[0]);
		send_to_char (buf, ch);
		if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
		{
			send_to_char (" '", ch);
			send_to_char (skill_table[obj->value[3]].name, ch);
			send_to_char ("'", ch);
		}
		send_to_char (".\n\r", ch);
		break;
	case ITEM_CARD:
		if (obj->value[6] >= 1 && obj->value[6] < MAX_SKILL)
		{
			sprintf (buf, "Has a charge of level %d", obj->value[5]);
			send_to_char (buf,ch);
			send_to_char (" '", ch);
			send_to_char (skill_table[obj->value[6]].name, ch);
			send_to_char (" '.\n\r", ch);
		}
		break;
	case ITEM_DRINK_CON:
		if (obj->value[2] == -1)
			strcpy (buf, "It holds no liquid.\n\r");

		else
			sprintf (buf, "It holds %s-colored %s.\n\r",
			liq_table[obj->value[2]].liq_color,
			liq_table[obj->value[2]].liq_name);
		send_to_char (buf, ch);
		break;
	case ITEM_CONTAINER:
		sprintf (buf, "Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
			obj->value[0], obj->value[3], cont_bit_name (obj->value[1]));
		send_to_char (buf, ch);
		if (obj->value[4] != 100)
		{
			sprintf (buf, "Weight multiplier: %d%%\n\r", obj->value[4]);
			send_to_char (buf, ch);
		}
		break;
	case ITEM_QUIVER:
		sprintf (buf, "Capacity: %d\n\r", obj->value[0]);
		send_to_char (buf, ch);
		break;
		//Iblis - 06/04/04 - Goddamn \% doesn't work to add just a % to sprintf, so I have to do it
		//this bass ackwards hacky way
	case ITEM_FURNITURE:
		sprintf (buf, "HP Healing Bonus: %d",obj->value[3]-100);
		send_to_char (buf, ch);
		send_to_char ("%   ", ch);
		sprintf (buf, "Mana Healing Bonus: %d",obj->value[4]-100);
		send_to_char (buf, ch);
		send_to_char ("%\n\r",ch);
		break;
	case ITEM_WEAPON:
		send_to_char ("Weapon type is ", ch);
		switch (obj->value[0])
		{
		case (WEAPON_EXOTIC):
			send_to_char ("exotic.\n\r", ch);
			break;
		case (WEAPON_SWORD):
			send_to_char ("sword.\n\r", ch);
			break;
		case (WEAPON_DAGGER):
			send_to_char ("dagger.\n\r", ch);
			break;
		case (WEAPON_STAFF):
			send_to_char ("staff/spear.\n\r", ch);
			break;
		case (WEAPON_MACE):
			send_to_char ("mace/club.\n\r", ch);
			break;
		case (WEAPON_AXE):
			send_to_char ("axe.\n\r", ch);
			break;
		case (WEAPON_FLAIL):
			send_to_char ("flail.\n\r", ch);
			break;
		case (WEAPON_POLEARM):
			send_to_char ("polearm.\n\r", ch);
			break;
		case (WEAPON_SHORTBOW):
			send_to_char ("shortbow.\n\r", ch);
			break;
		case (WEAPON_LONGBOW):
			send_to_char ("longbow.\n\r", ch);
			break;
		case (WEAPON_ARROW):
			send_to_char ("arrow.\n\r", ch);
			break;
		case (WEAPON_ARROWHEAD):
			send_to_char ("arrowhead.\n\r", ch);
			break;
		case (WEAPON_DICE):
			send_to_char ("dice.\n\r",ch);
			break;
		default:
			send_to_char ("unknown.\n\r", ch);
			break;
		}
		sprintf (buf, "Damage is %dd%d (average %d).\n\r", obj->value[1],
			obj->value[2], (1 + obj->value[2]) * obj->value[1] / 2);
		send_to_char (buf, ch);
		if (obj->value[4])
		{			/* weapon flags */
			sprintf (buf, "Weapons flags: %s\n\r",
				weapon_bit_name (obj->value[4]));
			send_to_char (buf, ch);
		}
		if (obj->value[0] == WEAPON_DICE)
		{
			sprintf (buf, "`jWeapon spell:`` %s\n\r",skill_table[obj->value[5]].name);
			send_to_char(buf,ch);
		}
		break;
	case ITEM_ARMOR:
		sprintf (buf,
			"Armor Class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r",
			obj->value[0], obj->value[1], obj->value[2], obj->value[3]);
		send_to_char (buf, ch);
		break;
	}
	if (!obj->enchanted)
		for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
		{
			if (paf->location != APPLY_NONE && paf->modifier != 0)
			{
				if (paf->where == TO_SKILL)
				{
					if(paf->location == 0)
						sprintf(buf,"Affects all abilities by %d",paf->modifier);
					else sprintf(buf,"Affects %s by %d", skill_table[paf->location].name,paf->modifier);
					send_to_char (buf,ch);
					send_to_char ("%.\n\r",ch);
				}
				else
				{
					sprintf (buf, "Affects %s by %d.\n\r",affect_loc_name (paf->location), paf->modifier);
					send_to_char (buf, ch);
					if (paf->bitvector)
					{
						switch (paf->where)
						{
						case TO_AFFECTS:
							sprintf (buf, "Adds %s affect.\n",
								affect_bit_name (paf->bitvector));
							break;
						case TO_OBJECT2:
						case TO_OBJECT:
							sprintf (buf, "Adds %s object flag.\n",
								extra_bit_name_old (paf->bitvector));
							break;
						case TO_IMMUNE:
							sprintf (buf, "Adds immunity to %s.\n",
								imm_bit_name (paf->bitvector));
							break;
						case TO_RESIST:
							sprintf (buf, "Adds resistance to %s.\n\r",
								imm_bit_name (paf->bitvector));
							break;
						case TO_VULN:
							sprintf (buf, "Adds vulnerability to %s.\n\r",
								imm_bit_name (paf->bitvector));
							break;
						default:
							sprintf (buf, "Unknown bit %d: %d\n\r",
								paf->where, paf->bitvector);
							break;
						}
						send_to_char (buf, ch);
					}
				}
			}
		}
		for (paf = obj->affected; paf != NULL; paf = paf->next)
		{
			if (paf->where != TO_AFFECTS)
			{
				if (paf->where == TO_SKILL)
				{
					if(paf->location == 0)
						sprintf(buf,"Affects all abilities by %d",paf->modifier);
					else sprintf(buf,"Affects %s by %d", skill_table[paf->location].name,paf->modifier);
					send_to_char (buf,ch);
					send_to_char ("%.\n\r",ch);
				}
				else if (paf->location != APPLY_NONE && paf->modifier != 0)
				{
					sprintf (buf, "Affects %s by %d", affect_loc_name (paf->location), paf->modifier);
					send_to_char (buf, ch);
					if (paf->duration > -1)
						sprintf (buf, ", %d hours.\n\r", paf->duration);
					else
						sprintf (buf, ".\n\r");
					send_to_char (buf, ch);
				}
			}
			else
			{
				sprintf (buf, "Affects [%s] to level %d for",flag_string (affect_flags, paf->bitvector), paf->level);
				send_to_char(buf, ch);
				if (paf->duration > -1)
					sprintf (buf, " %d hours.\n\r", paf->duration);
				else
					sprintf (buf, "ever.\n\r");
				send_to_char (buf, ch);

			}
			if (paf->bitvector)
			{
				switch (paf->where)
				{
				case TO_AFFECTS:
					break;		/*
								sprintf(buf,"Adds %s affect.\n",
								affect_bit_name(paf->bitvector));
								break; */
				case TO_OBJECT2:
				case TO_OBJECT:
					sprintf (buf, "Adds %s object flag.\n",
						extra_bit_name_old (paf->bitvector));
					break;
				case TO_WEAPON:
					sprintf (buf, "Adds %s weapon flags.\n",
						weapon_bit_name (paf->bitvector));
					break;
				case TO_IMMUNE:
					sprintf (buf, "Adds immunity to %s.\n",
						imm_bit_name (paf->bitvector));
					break;
				case TO_RESIST:
					sprintf (buf, "Adds resistance to %s.\n\r",
						imm_bit_name (paf->bitvector));
					break;
				case TO_VULN:
					sprintf (buf, "Adds vulnerability to %s.\n\r",
						imm_bit_name (paf->bitvector));
					break;
				default:
					sprintf (buf, "Unknown bit %d: %d\n\r", paf->where,
						paf->bitvector);
					break;
				}
				send_to_char (buf, ch);
			}
		}
		return;
}

void spell_infravision (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (IS_AFFECTED (victim, AFF_INFRARED))
	{
		if (victim == ch)
			send_to_char ("You can already see in the dark.\n\r", ch);

		else
			act ("$N already has infravision.\n\r", ch, NULL, victim, TO_CHAR);
		return;
	}
	act ("$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM);
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = 2 * level;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = AFF_INFRARED;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("Your eyes glow red.\n\r", victim);
	return;
}

void spell_invis (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;

	/* object invisibility */
	if (target == TARGET_OBJ)
	{
		obj = (OBJ_DATA *) vo;
		if (IS_OBJ_STAT (obj, ITEM_INVIS))
		{
			act ("$p is already invisible.", ch, obj, NULL, TO_CHAR);
			return;
		}
		af.where = TO_OBJECT;
		af.type = sn;
		af.level = level;
		af.duration = level + 12;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = ITEM_INVIS;
		af.permaff = FALSE;
		af.composition = FALSE;
		af.comp_name = str_dup ("");
		affect_to_obj (obj, &af);
		act ("$p fades out of sight.", ch, obj, NULL, TO_ALL);
		return;
	}

	/* character invisibility */
	victim = (CHAR_DATA *) vo;
	if (IS_AFFECTED (victim, AFF_INVISIBLE))
		return;
	act ("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level + 12;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = AFF_INVISIBLE;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("You fade out of existence.\n\r", victim);
	return;
}

void spell_mirror (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim, *fch;
	AFFECT_DATA af;
	bool found = FALSE;
	victim = (CHAR_DATA *) vo;
	if (is_affected (victim, gsn_mirror))
		return;
	if (IS_AFFECTED (ch, AFF_CHARM))
	{
		send_to_char ("You can't get away from your master so easily.\n\r", ch);
		return;
	}
	for (fch = char_list; fch != NULL; fch = fch->next)
	{
		if (fch->master == ch)
		{
			act
				("The strange power of the mirror forces $n to be seperated from your group.",
				ch, NULL, fch, TO_VICT);
			stop_follower (fch);
			found = TRUE;
		}
		if (fch->leader == ch)
		{
			fch->leader = fch;
			found = TRUE;
		}
		if (ch->master != NULL)
		{
			act
				("The strange power of the mirror forces $n to be seperated from your group.",
				ch, NULL, fch, TO_VICT);
			ch->master = NULL;
			found = TRUE;
		}
		if (ch->leader != NULL)
		{
			ch->leader = NULL;
			found = TRUE;
		}
	}
	if (found)
		send_to_char ("You leave your friends behind.\n\r", ch);
	act ("$n is surrounded by a reflective mirror.", victim, NULL, NULL,
		TO_ROOM);
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	if (level < 51)
		af.duration = 1;

	else if (level < 63)
		af.duration = 2;

	else if (level < 72)
		af.duration = 3;

	else
		af.duration = 4;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = 0;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("You are surrounded by a reflective mirror.\n\r", victim);
	return;
}

void spell_know_alignment (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	char *msg;
	int ap;
	ap = victim->alignment;
	if (ap > 700)
		msg = "$N has a pure and good aura.";

	else if (ap > 350)
		msg = "$N is of excellent moral character.";

	else if (ap > 100)
		msg = "$N is often kind and thoughtful.";

	else if (ap > -100)
		msg = "$N doesn't have a firm moral commitment.";

	else if (ap > -350)
		msg = "$N lies to $S friends.";

	else if (ap > -700)
		msg = "$N is a black-hearted murderer.";

	else
		msg = "$N is the embodiment of pure evil!.";
	act (msg, ch, NULL, victim, TO_CHAR);
	return;
}

void spell_lightning_bolt (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const sh_int dam_each[] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 28, 31, 34, 37, 40, 40, 41, 42,
	42, 43, 44, 44, 45, 46, 46, 47, 48, 48, 49, 50, 50, 51, 52, 52,
	53, 54, 54, 55, 56, 56, 57,
	58, 58, 59, 60, 60, 61, 62, 62, 63, 64
	};
	int dam;
	level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
	level = UMAX (0, level);

	/* BEGIN UNDERWATER */
	if (ch->in_room->sector_type == SECT_UNDERWATER)
	{
		send_to_char ("You can't summon a lightning bolt underwater.\n\r", ch);
		return;
	}

	/* END UNDERWATER */
	dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
	if (saves_spell (level, victim, DAM_LIGHTNING))
		dam /= 2;
	damage_old (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
	return;
}

void spell_locate_object (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	char buf[MAX_INPUT_LENGTH];
	BUFFER *buffer;
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	bool found;
	int number = 0, max_found;
	found = FALSE;
	number = 0;
	max_found = IS_IMMORTAL (ch) ? 200 : 2 * level;
	buffer = new_buf ();
	for (obj = object_list; obj != NULL; obj = obj->next)
	{
		if (!can_see_obj (ch, obj)
			|| !is_name_no_abbrev (target_name, obj->name)
			|| IS_OBJ_STAT (obj, ITEM_NOLOCATE)
			|| obj->item_type == ITEM_CARD
			|| number_percent () > 2 * level || ch->level < obj->level-10)
			continue;
		for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);

		////IBLIS 6/19/03 - Made it so mortals can't locate objs on imms
		if (in_obj->carried_by != NULL && !IS_IMMORTAL (ch)
			&& IS_IMMORTAL (in_obj->carried_by) && !IS_NPC (in_obj->carried_by))
			continue;
		found = TRUE;
		number++;
		if (!IS_IMMORTAL (ch) && in_obj->item_type == ITEM_PLAYER_DONATION)
			continue;
		if (!IS_IMMORTAL (ch) && in_obj->item_type == ITEM_CLAN_DONATION)
			continue;
		if (in_obj->carried_by != NULL && can_see (ch, in_obj->carried_by))
		{
			sprintf (buf, "one is carried by %s\n\r",
				PERS (in_obj->carried_by, ch));
		}

		else
		{
			if (IS_IMMORTAL (ch) && in_obj->in_room != NULL)
				sprintf (buf, "one is in %s [Room %d]\n\r",
				in_obj->in_room->name, in_obj->in_room->vnum);

			else
				sprintf (buf, "one is in %s\n\r",
				in_obj->in_room ==
				NULL ? "somewhere" : in_obj->in_room->name);
		}
		buf[0] = UPPER (buf[0]);
		add_buf (buffer, buf);
		if (number >= max_found)
			break;
	}
	if (!found)
		send_to_char ("Nothing like that in heaven or earth.\n\r", ch);

	else
	{
		page_to_char (buf_string (buffer), ch);
		send_to_char ("Ok.\n\r", ch);
	}
	free_buf (buffer);
	return;
}

void spell_magic_missile (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const sh_int dam_each[] =
	{ 0, 3, 3, 4, 4, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8,
	9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 12, 12, 12,
	12, 12, 13, 13, 13, 13,
	13, 14, 14, 14, 14, 14
	};
	int dam;
	level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
	level = UMAX (0, level);
	dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
	if (saves_spell (level, victim, DAM_ENERGY))
		dam /= 2;
	damage_old (ch, victim, dam, sn, DAM_ENERGY, TRUE);
	return;
}

void spell_mass_healing (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *gch;
	int heal_num, refresh_num;
	heal_num = skill_lookup ("heal");
	refresh_num = skill_lookup ("refresh");
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
		if (gch->race == PC_RACE_SWARM)
			continue;
		if ((IS_NPC (ch) && IS_NPC (gch)) || (!IS_NPC (ch) && !IS_NPC (gch)))
		{
			if (is_affected(gch,gsn_greymantle) || IS_SET(gch->act,ACT_UNDEAD))
			{
				reverse_heal(ch,gch,100,sn);
				continue;
			}
			spell_heal (heal_num, level, ch, (void *) gch, TARGET_CHAR);
			spell_refresh (refresh_num, level, ch, (void *) gch, TARGET_CHAR);
		}
	}
}
void spell_mass_invis (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	AFFECT_DATA af;
	CHAR_DATA *gch;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
		if (!is_same_group (gch, ch) || IS_AFFECTED (gch, AFF_INVISIBLE))
			continue;
		act ("$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM);
		send_to_char ("You slowly fade out of existence.\n\r", gch);
		af.where = TO_AFFECTS;
		af.type = sn;
		af.level = level / 2;
		af.duration = 24;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = AFF_INVISIBLE;
		af.permaff = FALSE;
		af.composition = FALSE;
		af.comp_name = str_dup ("");
		affect_to_char (gch, &af);
	}
	send_to_char ("Ok.\n\r", ch);
	return;
}

void spell_null (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	send_to_char ("That's not a spell!\n\r", ch);
	return;
}

void spell_pass_door (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (IS_AFFECTED (victim, AFF_PASS_DOOR))
	{
		if (victim == ch)
			send_to_char ("You are already out of phase.\n\r", ch);

		else
			act ("$N is already shifted out of phase.", ch, NULL, victim,
			TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = number_fuzzy (level / 4);
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = AFF_PASS_DOOR;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	act ("$n turns translucent.", victim, NULL, NULL, TO_ROOM);
	send_to_char ("You turn translucent.\n\r", victim);
	return;
}

void spell_plague (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (saves_spell (level, victim, DAM_DISEASE) ||
		(IS_NPC (victim) && IS_SET (victim->act, ACT_UNDEAD))
		|| is_affected(victim,gsn_immunity)
		|| !IS_NPC(victim))
	{
		if (ch == victim)
			send_to_char ("You feel momentarily ill, but it passes.\n\r", ch);

		else
			act ("$N seems to be unaffected.", ch, NULL, victim, TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level * 3 / 4;
	af.duration = level;
	af.location = APPLY_STR;
	af.modifier = -5;
	af.bitvector = AFF_PLAGUE;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup (ch->name);
	affect_join (victim, &af);
	send_to_char
		("You scream in agony as plague sores erupt from your skin.\n\r", victim);
	act ("$n screams in agony as plague sores erupt from $s skin.",
		victim, NULL, NULL, TO_ROOM);
}

void spell_poison (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;
	if (target == TARGET_OBJ)
	{
		obj = (OBJ_DATA *) vo;
		if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
		{
			if (IS_OBJ_STAT (obj, ITEM_BLESS))
			{
				act ("Your spell fails to corrupt $p.", ch, obj, NULL, TO_CHAR);
				return;
			}
			obj->value[3] = 1;
			act ("$p is infused with poisonous vapors.", ch, obj, NULL, TO_ALL);
			return;
		}
		if (obj->item_type == ITEM_WEAPON)
		{
			if (IS_WEAPON_STAT (obj, WEAPON_FLAMING)
				|| IS_WEAPON_STAT (obj, WEAPON_FROST)
				|| IS_WEAPON_STAT (obj, WEAPON_VAMPIRIC)
				|| IS_WEAPON_STAT (obj, WEAPON_SHARP)
				|| IS_WEAPON_STAT (obj, WEAPON_VORPAL)
				|| IS_WEAPON_STAT (obj, WEAPON_SHOCKING)
				|| IS_OBJ_STAT (obj, ITEM_BLESS))
			{
				act ("You can't seem to envenom $p.", ch, obj, NULL, TO_CHAR);
				return;
			}
			if (IS_WEAPON_STAT (obj, WEAPON_POISON))
			{
				act ("$p is already envenomed.", ch, obj, NULL, TO_CHAR);
				return;
			}
			af.where = TO_WEAPON;
			af.type = sn;
			af.level = level / 2;
			af.duration = level / 8;
			af.location = 0;
			af.modifier = 0;
			af.bitvector = WEAPON_POISON;
			af.permaff = FALSE;
			af.composition = FALSE;
			af.comp_name = str_dup ("");
			affect_to_obj (obj, &af);
			act ("$p is coated with deadly venom.", ch, obj, NULL, TO_ALL);
			return;
		}
		act ("You can't poison $p.", ch, obj, NULL, TO_CHAR);
		return;
	}
	victim = (CHAR_DATA *) vo;
	if (saves_spell (level, victim, DAM_POISON)) 
	{
		act ("$n turns slightly green, but it passes.", victim, NULL, NULL,
			TO_ROOM);
		send_to_char ("You feel momentarily ill, but it passes.\n\r", victim);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level;
	af.location = APPLY_STR;
	af.modifier = -2;
	af.bitvector = AFF_POISON;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup (ch->name);
	affect_join (victim, &af);
	send_to_char ("You feel very sick.\n\r", victim);
	act ("$n looks very ill.", victim, NULL, NULL, TO_ROOM);
	return;
}

void spell_protection_evil (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (IS_AFFECTED (victim, AFF_PROTECT_EVIL)
		|| IS_AFFECTED (victim, AFF_PROTECT_GOOD))
	{
		if (victim == ch)
			send_to_char ("You are already protected.\n\r", ch);

		else
			act ("$N is already protected.", ch, NULL, victim, TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = (ch->level == level)?24:level/9;
	af.location = APPLY_SAVING_SPELL;
	af.modifier = -1;
	af.bitvector = AFF_PROTECT_EVIL;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("You feel holy and pure.\n\r", victim);
	if (ch != victim)
		act ("$N is protected from evil.", ch, NULL, victim, TO_CHAR);
	return;
}

void spell_protection_good (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (IS_AFFECTED (victim, AFF_PROTECT_GOOD)
		|| IS_AFFECTED (victim, AFF_PROTECT_EVIL))
	{
		if (victim == ch)
			send_to_char ("You are already protected.\n\r", ch);

		else
			act ("$N is already protected.", ch, NULL, victim, TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = 24;
	af.location = APPLY_SAVING_SPELL;
	af.modifier = -1;
	af.bitvector = AFF_PROTECT_GOOD;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("You feel aligned with darkness.\n\r", victim);
	if (ch != victim)
		act ("$N is protected from good.", ch, NULL, victim, TO_CHAR);
	return;
}

void spell_ray_of_truth (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam, align;
	if (!IS_GOOD (ch))
	{
		victim = ch;
		send_to_char ("The energy explodes inside you!\n\r", ch);
	}
	if (victim != ch)
	{
		act ("$n raises $s hand, and a blinding ray of light shoots forth!",
			ch, NULL, NULL, TO_ROOM);
		send_to_char
			("You raise your hand and a blinding ray of light shoots forth!\n\r",
			ch);
	}
	if (IS_GOOD (victim))
	{
		act ("$n seems unharmed by the light.", victim, NULL, victim, TO_ROOM);
		send_to_char ("The light seems powerless to affect you.\n\r", victim);
		return;
	}
	dam = dice (level, 10);
	if (saves_spell (level, victim, DAM_HOLY))
		dam /= 2;
	align = victim->alignment;
	align -= 350;
	if (align < -1000)
		align = -1000 + (align + 1000) / 3;
	dam = (dam * align * align) / 1000000;
	damage_old (ch, victim, dam, sn, DAM_HOLY, TRUE);
	if (victim->hit > 1 && number_percent() <= 50)
		spell_blindness (gsn_blindness, 3 * level / 4, ch, (void *) victim,
		TARGET_CHAR);
}

void spell_recharge (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance, percent;
	if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF)
	{
		send_to_char ("That item does not carry charges.\n\r", ch);
		return;
	}
	if (obj->value[0] > ch->level)
	{
		send_to_char ("Your skills are not great enough for that.\n\r", ch);
		return;
	}
	if (obj->value[1] == 0)
	{
		send_to_char ("That item has already been recharged once.\n\r", ch);
		return;
	}
	chance = 40 + 2 * level;
	chance -= obj->value[3];	/* harder to do high-level spells */
	chance -= (obj->value[1] - obj->value[2]) * (obj->value[1] - obj->value[2]);
	chance = UMAX (level / 2, chance);
	percent = number_percent ();
	if (percent < chance / 2)
	{
		act ("$p glows softly.", ch, obj, NULL, TO_CHAR);
		act ("$p glows softly.", ch, obj, NULL, TO_ROOM);
		obj->value[2] = UMAX (obj->value[1], obj->value[2]);
		obj->value[1] = 0;
		return;
	}

	else if (percent <= chance)
	{
		int chargeback, chargemax;
		act ("$p glows softly.", ch, obj, NULL, TO_CHAR);
		act ("$p glows softly.", ch, obj, NULL, TO_ROOM);
		chargemax = obj->value[1] - obj->value[2];
		if (chargemax > 0)
			chargeback = UMAX (1, chargemax * percent / 100);

		else
			chargeback = 0;
		obj->value[2] += chargeback;
		obj->value[1] = 0;
		return;
	}

	else if (percent <= UMIN (95, 3 * chance / 2))
	{
		send_to_char ("Nothing seems to happen.\n\r", ch);
		if (obj->value[1] > 1)
			obj->value[1]--;
		return;
	}

	else
		/* whoops! */
	{
		act ("$p glows brightly and explodes!", ch, obj, NULL, TO_CHAR);
		act ("$p glows brightly and explodes!", ch, obj, NULL, TO_ROOM);
		extract_obj (obj);
	}
}
void spell_refresh (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	victim->move = UMIN (victim->move + level, victim->max_move);
	if (victim->max_move == victim->move)
		send_to_char ("You feel fully refreshed!\n\r", victim);

	else
		send_to_char ("You feel less tired.\n\r", victim);
	if (ch != victim)
		send_to_char ("Ok.\n\r", ch);
	return;
}

void spell_shield (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (is_affected (victim, sn))
	{
		if (victim == ch)
			send_to_char ("You are already shielded from harm.\n\r", ch);

		else
			act ("$N is already protected by a shield.", ch, NULL, victim,
			TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = 8 + level;
	af.location = APPLY_AC;
	af.modifier = -10;
	af.bitvector = 0;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	act ("$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM);
	send_to_char ("You are surrounded by a force shield.\n\r", victim);
	return;
}

void spell_shocking_grasp (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const int dam_each[] =
	{ 0, 0, 0, 0, 0, 0, 0, 20, 25, 29, 33, 36, 39, 39, 39, 40, 40, 41,
	41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47, 48, 48, 49, 49,
	50, 50, 51, 51, 52,
	52, 53, 53, 54, 54, 55, 55, 56, 56, 57, 57
	};
	int dam;
	level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
	level = UMAX (0, level);
	dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
	if (saves_spell (level, victim, DAM_LIGHTNING))
		dam /= 2;
	damage_old (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
	return;
}

void spell_sleep (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	// Akamai: 10/29/98 -- Fixing Mage sleep so that it does not affect
	// immortals and appropriately handles level differences
	if (IS_IMMORTAL (victim))
	{
		send_to_char
			("You are worth no more than an askance look from an immortal.\n\r",
			ch);
		return;
	}
	// if the victim is a real player +- MAGIC_LEVEL_DIFF from the level
	// of the mage then fail.
	if (!IS_NPC (victim) &&
		((victim->level >= (ch->level + MAGIC_LEVEL_DIFF)) ||
		(victim->level <= (ch->level - MAGIC_LEVEL_DIFF))))
	{
		send_to_char ("That person is some how beyond your charms.\n\r", ch);
		return;
	}
	// if the victim is a MOB + MAGIC_LEVEL_DIFF from the level of the
	// mage then the MOB is beyond them
	if (IS_NPC (victim) && (victim->level > (ch->level + MAGIC_LEVEL_DIFF)))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (IS_AFFECTED (victim, AFF_SLEEP)
		|| (IS_NPC (victim) && IS_SET (victim->act, ACT_UNDEAD))
		|| saves_spell (level - 4, victim, DAM_MENTAL)
		|| is_affected (victim, skill_lookup ("Dance of Shadows"))
		|| is_affected (victim, gsn_fear)
		|| IS_SET(victim->imm_flags, IMM_SLEEP)
		|| IS_SET(victim->act, ACT_UNDEAD))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = 12;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = AFF_SLEEP;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_join (victim, &af);
	if (IS_AWAKE (victim))
	{
		send_to_char
			("Suddenly, you are overcome with the need to sleep ..... zzzzzz.\n\r",
			victim);
		act ("$n goes to sleep.", victim, NULL, NULL, TO_ROOM);
		victim->position = POS_SLEEPING;
	}
	return;
}

void spell_slow (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (is_affected (victim, sn) || IS_AFFECTED (victim, AFF_SLOW))
	{
		if (victim == ch)
			send_to_char ("You can't move any slower!\n\r", ch);

		else
			act ("$N can't get any slower than that.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (saves_spell (level, victim, DAM_MENTAL)
		|| IS_SET (victim->imm_flags, IMM_MAGIC)
		|| (!IS_NPC(ch) && ch->race == PC_RACE_NIDAE))
	{
		if (victim != ch)
			send_to_char ("Nothing seemed to happen.\n\r", ch);
		send_to_char ("You feel momentarily lethargic.\n\r", victim);
		return;
	}
	if (IS_AFFECTED (victim, AFF_HASTE))
	{
		if (is_affected(victim,skill_lookup("haste")))
		{
			if (!check_dispel (level, victim, skill_lookup ("haste")))
			{
				if (victim != ch)
					send_to_char ("Spell failed.\n\r", ch);
				send_to_char ("You feel momentarily slower.\n\r", victim);
				return;
			}
		}
		else
		{
			if (!saves_dispel (ch->level, victim->level, -1))
			{
				if (victim != ch)
					send_to_char ("Spell failed.\n\r", ch);
				send_to_char ("You feel momentarily slower.\n\r", victim);
				return;
			}
			REMOVE_BIT(victim->affected_by,AFF_HASTE);
		}
		act ("$n is moving less quickly.", victim, NULL, NULL, TO_ROOM);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level / 2;
	af.location = APPLY_DEX;
	af.modifier = -1 - (level >= 18) - (level >= 25) - (level >= 32);
	af.bitvector = AFF_SLOW;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("You feel yourself slowing d o w n...\n\r", victim);
	act ("$n starts to move in slow motion.", victim, NULL, NULL, TO_ROOM);
	return;
}

void spell_stone_skin (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (is_affected (ch, sn))
	{
		if (victim == ch)
			send_to_char ("Your skin is already as hard as a rock.\n\r", ch);

		else
			act ("$N is already as hard as can be.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (is_affected (ch, gsn_bark_skin))
	{
		if (victim == ch)
			send_to_char ("Your skin can't get any harder.\n\r", ch);

		else
			act ("$N's skin is already as hard as can be.", ch, NULL,
			victim, TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level;
	af.location = APPLY_AC;
	af.modifier = -15;
	af.bitvector = 0;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	act ("$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM);
	send_to_char ("Your skin turns to stone.\n\r", victim);
	return;
}


//
// Akamai 4/5/99 -- Part of the changes to summon. This does a saving
// roll calculation against the mage to see if a summon fails or
// fails *badly*
//
int summon_goes_wrong (CHAR_DATA * ch)
{
	if (!IS_IMMORTAL (ch))
	{
		if (bad_summoning_on)
		{

			// there is an impression that this is happening too
			// frequently - this will make the failure much less frequent
			if (number_percent () >= 25)
			{
				if (saves_spell (ch->level, ch, DAM_MENTAL))
				{
					return (0);
				}

				else
				{
					return (1);
				}
			}

			else
			{
				return (0);
			}
		}

		else
		{

			// when bad summoning is turned off, never fail badly
			return (0);
		}
	}

	else
	{

		// summon should never fail badly for immortals
		return (0);
	}
}


//
// Akamai 4/5/99 -- Part of the changes to summon. This picks a random
// nasty mob, relatively close to the level of the mage, opens a portal
// and brings that mob to the mage. As well, this sets the mob fighting
// flag to the mage so the mob auto attacks the mage.
//
#define MAX_NASTY_TRYS 3
//
void summon_nasty (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *aggie = NULL;
	OBJ_DATA *portal = NULL;
	int attempts = 0;
	int deci_range = 0;
	int seg = 0;
	int item = 0;
	int vnum = 0;
	int found = FALSE;

	// pick a random aggie mob
	deci_range = ch->level / 10;
	while ((aggie == NULL) && (attempts < MAX_NASTY_TRYS))
	{
		seg = number_range (-2, 1);	// three fourths of the time we go easy
		item = number_range (0, (NASTY_LIST_LEN - 1));
		switch (deci_range)
		{
		default:
			attempts = 6;
			break;
		case 0:
		case 1:		// this just can't be
			// fail immediately
			attempts = 6;
			break;
		case 2:
			if (seg <= 0)
			{
				vnum = nasty10[item];
			}

			else
			{
				vnum = nasty20[item];
			}
			break;
		case 3:
			if (seg <= 0)
			{
				vnum = nasty20[item];
			}

			else
			{
				vnum = nasty30[item];
			}
			break;
		case 4:
			if (seg <= 0)
			{
				vnum = nasty30[item];
			}

			else
			{
				vnum = nasty40[item];
			}
			break;
		case 5:
			if (seg <= 0)
			{
				vnum = nasty40[item];
			}

			else
			{
				vnum = nasty50[item];
			}
			break;
		case 6:
			if (seg <= 0)
			{
				vnum = nasty50[item];
			}

			else
			{
				vnum = nasty60[item];
			}
			break;
		case 7:
			if (seg <= 0)
			{
				vnum = nasty60[item];
			}

			else
			{
				vnum = nasty70[item];
			}
			break;
		case 8:
		case 9:
		case 10:
			if (seg <= 0)
			{
				vnum = nasty70[item];
			}

			else
			{
				vnum = nasty80[item];
			}
			break;
		}

		// got a vnum for the monster, now see if it's around
		if (vnum > 0)
		{
			aggie = char_list;
			while ((aggie != NULL) && !found)
			{
				if (IS_NPC (aggie))
				{
					if (aggie->pIndexData->vnum == vnum)
					{
						found = TRUE;
					}

					else
					{
						aggie = aggie->next;
					}
				}

				else
				{
					aggie = aggie->next;
				}
			}
			if (found)
			{
				sprintf (buf, "summon_nasty(): %s summons %s [%d]",
					ch->name, aggie->name, vnum);
				log_string (buf);
			}

			else
			{
				sprintf (buf, "summon_nasty(): could not find mob [%d]", vnum);
				log_string (buf);
			}
		}
		// make sure we don't spend too much time on this
		attempts++;
	}
	if (attempts >= 5)
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	// open a portal to that mob
	portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
	portal->timer = 1 + (ch->level) / 25;
	portal->value[0] = 1;
	portal->value[3] = ch->in_room->vnum;
	obj_to_room (portal, aggie->in_room);
	act ("$p appears and beckons to you.", aggie, portal, NULL, TO_CHAR);
	act ("$p appears with a pop.", aggie, portal, NULL, TO_ROOM);
	enter_portal (aggie, portal->name, FALSE);
	if (aggie->in_room == ch->in_room)
	{
		act ("$n has summoned you!", ch, NULL, aggie, TO_VICT);
		act ("You succeed in summoning $N.", ch, NULL, aggie, TO_CHAR);

		// and boy is that mob going to be mad
		// the mob hunts the mage
		REMOVE_BIT (aggie->act, ACT_AGGRESSIVE);
		SET_BIT (aggie->act, ACT_HUNT);
		SET_BIT (aggie->act, ACT_REMEMBER);
		SET_BIT (aggie->act, ACT_SMART_HUNT);
		set_fighting (ch, aggie);
		add_hunting (aggie, ch);
	}
	return;
}


//
// Akamai 4/5/99 -- Fixing "nosummon" - this routine, spell_summon, was
// not checking the victim->act flags to see if the PLR_NOSUMMON flag
// was set. Now it checks. As well, on a failure, there is a chance that
// the mage will summon something aggressive.
//
void spell_summon (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim;
	OBJ_DATA *portal;
	victim = get_char_world (ch, target_name);

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

	if (nogate || (!IS_NPC(ch) && ch->pcdata->nogate) || (!IS_NPC(victim) && victim->pcdata->nogate))
	{
		send_to_char("The gods are not allowing that at the moment.\n\r",ch);
		return;
	}


	// the victim is a real victim, but for various reasons the mage
	// might fail, in these cases check for failure and maybe summon_nasty
	if (IS_SET (victim->in_room->room_flags, ROOM_SAFE)
		|| IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
		|| (victim->in_room->sector_type == SECT_SWAMP)
		|| (ch->in_room->sector_type == SECT_WATER_OCEAN)
		|| (victim->in_room == ch->in_room)
		|| IS_NPC (victim)
		|| IS_SET(ch->in_room->area->area_flags,AREA_SOLO)
		|| IS_SET(ch->in_room->room_flags2,ROOM_SOLO)
		|| (IS_SET(ch->in_room->area->area_flags,AREA_IMP_ONLY) && get_trust(victim) < MAX_LEVEL)
		|| (victim->level < 8 && !ch->pcdata->has_reincarnated) || (!(battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner)))
		&& IS_SET (victim->act, PLR_NOSUMMON)))
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
	// the victim's save needs to be checked before we open the portal
	if (saves_spell (level - 2, victim, DAM_MENTAL))
	{

		// the victim is strong enough to resist the summon, so check
		// to see if the summon goes very *badly*
		if (summon_goes_wrong (ch))
		{
			summon_nasty (ch);
		}

		else
		{
			portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
			portal->timer = 1 + level / 25;
			portal->value[0] = 1;
			portal->value[3] = ch->in_room->vnum;
			obj_to_room (portal, victim->in_room);
			act ("$p appears and beckons to you.", victim, portal, NULL,
				TO_CHAR);
			act ("$p appears with a pop.", victim, portal, NULL, TO_ROOM);
			act ("Your portal opens, but $N resists your compulsion to enter.",
				ch, NULL, victim, TO_CHAR);
		}
	}

	else
	{

		// the victim is not strong enough to resist the summon
		portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
		portal->timer = 1 + level / 25;
		portal->value[0] = 1;
		portal->value[3] = ch->in_room->vnum;
		obj_to_room (portal, victim->in_room);
		act ("$p appears and beckons to you.", victim, portal, NULL, TO_CHAR);
		act ("$p appears with a pop.", victim, portal, NULL, TO_ROOM);
		act ("Your portal opens, but $N resists your compulsion to enter.",
			ch, NULL, victim, TO_CHAR);
		//enter_portal (victim, portal->name, FALSE);
		//if (victim->in_room == ch->in_room)
		//{
		//	if (!is_affected (ch, gsn_blindness))
		//	{
		//		act ("$n has summoned you!", ch, NULL, victim, TO_VICT);
		//	}
		//	act ("You succeed in summoning $N.", ch, NULL, victim, TO_CHAR);
		//}

		//else
		//{
		//	act ("$N seems unable to enter your portal.", ch, NULL,
		//		victim, TO_CHAR);
		//}
	}
	return;
}

void spell_teleport (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	ROOM_INDEX_DATA *pRoomIndex;
	char tcbuf[15];
	if (is_safe (ch, victim))
		return;
	if (nogate|| (!IS_NPC(ch) && ch->pcdata->nogate))
	{
		send_to_char("The gods are not allowing that at the moment.\n\r",ch);
		return;
	}
	if (victim->in_room == NULL
		|| (!IS_IMMORTAL(ch) && IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL))
		|| (victim != ch && !IS_IMMORTAL(ch) && IS_SET (victim->imm_flags, IMM_SUMMON))
		|| (IS_SET (victim->act2, ACT_NOMOVE))
		|| (!IS_NPC (ch) && !IS_IMMORTAL(ch) && victim->fighting != NULL)
		|| (victim != ch && !IS_IMMORTAL(ch) && (saves_spell (level - 5, victim, DAM_OTHER))))
	{
		send_to_char ("You failed.\n\r", ch);
		return;
	}
	if (IS_SET (ch->in_room->race_flags, ROOM_NOTRANSPORT))
	{
		send_to_char
			("You cannot leave this realm via ordinary magic.\n\r", ch);
		return;
	}
	if (!IS_NPC (victim) && victim->desc != NULL)
		if (victim->desc->pEdit != NULL)
		{
			send_to_char ("You failed.\n\r", ch);
			return;
		}
		// Minax 7-29-02 Teleporting a mob has a 33% chance of making the mob
		// attack the caster.
		if (IS_NPC (victim) && (number_percent () < 34) && !IS_IMMORTAL(ch))
		{
			send_to_char ("You failed.\n\r", ch);
			victim->fighting = ch;
			return;
		}
		pRoomIndex = get_random_room (victim);
		while (IS_SET (pRoomIndex->room_flags, ROOM_ARENA)
			|| IS_SET (pRoomIndex->room_flags, ROOM_SILENT)
			|| IS_SET (pRoomIndex->room_flags, ROOM_NOMAGIC)
			|| pRoomIndex->area->construct
			|| pRoomIndex->sector_type == SECT_WATER_OCEAN
			|| IS_SET (pRoomIndex->race_flags, ROOM_NOTRANSPORT)
			|| IS_SET (pRoomIndex->race_flags, ROOM_NOTELEPORT)
			|| (IS_SET(pRoomIndex->area->area_flags,AREA_SOLO))
			|| IS_SET(pRoomIndex->room_flags2,ROOM_SOLO)
			|| (IS_SET(pRoomIndex->area->area_flags,AREA_IMP_ONLY) && get_trust(ch) < MAX_LEVEL)
			|| (pRoomIndex->vnum >= ROOM_VNUM_PLAYER_START && pRoomIndex->vnum <= ROOM_VNUM_PLAYER_END)	 
			|| (battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner)) && !br_vnum_check (pRoomIndex->vnum))
			)
			pRoomIndex = get_random_room (victim);
		if (victim != ch)
			send_to_char ("You have been teleported!\n\r", victim);
		act ("$n vanishes!", victim, NULL, NULL, TO_ROOM);
		char_from_room (victim);
		char_to_room (victim, pRoomIndex);
		act ("$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM);
		do_look (victim, "auto");
		trip_triggers(victim, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
		trap_check(victim,"room",victim->in_room,NULL);
		sprintf(tcbuf,"%d",victim->in_room->vnum);
		trip_triggers_arg(victim, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
		check_aggression(victim);
		return;
}

void spell_ventriloquate (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char speaker[MAX_INPUT_LENGTH];
	CHAR_DATA *vch;
	int race=0;
	target_name = one_argument (target_name, speaker);

	if (ch->in_room == NULL || speaker[0] == '\0' || IS_NPC(ch))
		return;

	//IBLIS 5/20/03 - Made says look realistic
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
		if (!strcmp (vch->name, target_name))
			race = vch->race;
	}
	if (race == 7)
	{
		send_to_char
			("You can't make a sidhe talk, no matter how hard you try.\n\r", ch);
		return;
	}

	else if (race == 4)
	{
		sprintf (buf1, "``%s gurgles `a'`k%s`a'\n\r", speaker, target_name);
		sprintf (buf2, "Someone makes %s gurgle `a'`k%s`a'\n\r", speaker,
			target_name);
	}

	else if (race == 5)
	{
		sprintf (buf1, "``%s grunts `a'`k%s`a'\n\r", speaker, target_name);
		sprintf (buf2, "Someone makes %s grunt `a'`k%s`a'\n\r", speaker,
			target_name);
	}

	else
	{
		sprintf (buf1, "``%s says `a'`k%s`a'\n\r", speaker, target_name);
		sprintf (buf2, "Someone makes %s say `a'`k%s`a'\n\r", speaker,
			target_name);
	}
	buf1[0] = UPPER (buf1[0]);
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
		if (!is_name (speaker, vch->name))
			send_to_char (saves_spell (level, vch, DAM_OTHER) ? buf2 : buf1, vch);
	}
	return;
}

void spell_weaken (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (is_affected (victim, sn) || saves_spell (level, victim, DAM_OTHER))
		return;
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level / 2;
	af.location = APPLY_STR;
	af.modifier = -1 * (level / 15);
	af.bitvector = AFF_WEAKEN;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("You feel your strength slip away.\n\r", victim);
	act ("$n looks tired and weak.", victim, NULL, NULL, TO_ROOM);
	return;
}


/* RT recall spell is back */
void spell_word_of_recall (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	ROOM_INDEX_DATA *location;
	char tcbuf[15];
	if (IS_NPC (victim))
		return;
	if ((location = get_room_index (ROOM_VNUM_TEMPLE)) == NULL)
	{
		send_to_char ("You are completely lost.\n\r", victim);
		return;
	}
	if (IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
		|| IS_AFFECTED (victim, AFF_CURSE))
	{
		send_to_char ("Spell failed.\n\r", victim);
		return;
	}
	if (victim->fighting != NULL)
		stop_fighting (victim, TRUE);
	ch->move /= 2;
	act ("$n disappears.", victim, NULL, NULL, TO_ROOM);
	char_from_room (victim);
	char_to_room (victim, location);
	act ("$n appears in the room.", victim, NULL, NULL, TO_ROOM);
	do_look (victim, "auto");
	trip_triggers(victim, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	trap_check(victim,"room",victim->in_room,NULL);
	sprintf(tcbuf,"%d",victim->in_room->vnum);
	trip_triggers_arg(victim, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
	check_aggression(victim);
}


/*
* NPC spells.
*/
void spell_acid_breath (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam, hp_dam, dice_dam, hpch;

	/* BEGIN UNDERWATER */
	if (ch->in_room->sector_type == SECT_UNDERWATER)
	{
		send_to_char
			("Your acid breath is rendered useless underwater.\n\r", ch);
		return;
	}

	/* END UNDERWATER */
	act ("$n spits acid at $N.", ch, NULL, victim, TO_NOTVICT);
	act ("$n spits a stream of corrosive acid at you.", ch, NULL, victim,
		TO_VICT);
	act ("You spit acid at $N.", ch, NULL, victim, TO_CHAR);
	hpch = UMAX (12, ch->hit);
	hp_dam = number_range (hpch / 11 + 1, hpch / 6);
	dice_dam = dice (level, 16);
	dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
	dam = dam * 2 / 3;
	dam = dam / 4 * 6;
	if (saves_spell (level, victim, DAM_ACID))
	{
		acid_effect (victim, level / 2, dam / 4, TARGET_CHAR);
		damage_old (ch, victim, dam / 2, sn, DAM_ACID, TRUE);
	}

	else
	{
		acid_effect (victim, level, dam, TARGET_CHAR);
		damage_old (ch, victim, dam, sn, DAM_ACID, TRUE);
	}
}
void spell_fire_breath (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *vch, *vch_next;
	int dam, hp_dam, dice_dam;
	int hpch;

	/* BEGIN UNDERWATER */
	if (ch->in_room->sector_type == SECT_UNDERWATER)
	{
		send_to_char ("Your fire breath is useless underwater.\n\r", ch);
		return;
	}

	/* END UNDERWATER */
	act ("$n breathes forth a cone of fire.", ch, NULL, victim, TO_NOTVICT);
	act ("$n breathes a cone of hot fire over you!", ch, NULL, victim, TO_VICT);
	act ("You breath forth a cone of fire.", ch, NULL, NULL, TO_CHAR);
	hpch = UMAX (10, ch->hit);
	hp_dam = number_range (hpch / 9 + 1, hpch / 5);
	dice_dam = dice (level, 20);
	dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
	dam = dam * 2 / 3;
	dam = dam / 4 * 6;
	fire_effect (victim->in_room, level, dam / 2, TARGET_ROOM);
	for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next_in_room;
		if (is_safe_spell (ch, vch, TRUE)
			|| (IS_NPC (vch) && IS_NPC (ch)
			&& (ch->fighting != vch || vch->fighting != ch)))
			continue;
		if (vch == victim)
		{			/* full damage */
			if (saves_spell (level, vch, DAM_FIRE))
			{
				fire_effect (vch, level / 2, dam / 4, TARGET_CHAR);
				damage_old (ch, vch, dam / 2, sn, DAM_FIRE, TRUE);
			}

			else
			{
				fire_effect (vch, level, dam, TARGET_CHAR);
				damage_old (ch, vch, dam, sn, DAM_FIRE, TRUE);
			}
		}

		else
			/* partial damage */
		{
			if (saves_spell (level - 2, vch, DAM_FIRE))
			{
				fire_effect (vch, level / 4, dam / 8, TARGET_CHAR);
				damage_old (ch, vch, dam / 4, sn, DAM_FIRE, TRUE);
			}

			else
			{
				fire_effect (vch, level / 2, dam / 4, TARGET_CHAR);
				damage_old (ch, vch, dam / 2, sn, DAM_FIRE, TRUE);
			}
		}
	}
}
void spell_frost_breath (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *vch, *vch_next;
	int dam, hp_dam, dice_dam, hpch;
	act ("$n breathes out a freezing cone of frost!", ch, NULL, victim,
		TO_NOTVICT);
	act ("$n breathes a freezing cone of frost over you!", ch, NULL,
		victim, TO_VICT);
	act ("You breath out a cone of frost.", ch, NULL, NULL, TO_CHAR);
	hpch = UMAX (12, ch->hit);
	hp_dam = number_range (hpch / 11 + 1, hpch / 6);
	dice_dam = dice (level, 16);
	dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
	dam = dam * 2 / 3;
	dam = dam / 4 * 6;
	cold_effect (victim->in_room, level, dam / 2, TARGET_ROOM);
	for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next_in_room;
		if (is_safe_spell (ch, vch, TRUE)
			|| (IS_NPC (vch) && IS_NPC (ch)
			&& (ch->fighting != vch || vch->fighting != ch)))
			continue;
		if (vch == victim)
		{			/* full damage */
			if (saves_spell (level, vch, DAM_COLD))
			{
				cold_effect (vch, level / 2, dam / 4, TARGET_CHAR);
				damage_old (ch, vch, dam / 2, sn, DAM_COLD, TRUE);
			}

			else
			{
				cold_effect (vch, level, dam, TARGET_CHAR);
				damage_old (ch, vch, dam, sn, DAM_COLD, TRUE);
			}
		}

		else
		{
			if (saves_spell (level - 2, vch, DAM_COLD))
			{
				cold_effect (vch, level / 4, dam / 8, TARGET_CHAR);
				damage_old (ch, vch, dam / 4, sn, DAM_COLD, TRUE);
			}

			else
			{
				cold_effect (vch, level / 2, dam / 4, TARGET_CHAR);
				damage_old (ch, vch, dam / 2, sn, DAM_COLD, TRUE);
			}
		}
	}
}
void spell_gas_breath (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam, hp_dam, dice_dam, hpch;

	/* BEGIN UNDERWATER */
	if (ch->in_room->sector_type == SECT_UNDERWATER)
	{
		send_to_char ("Your gas breath is useless underwater.\n\r", ch);
		return;
	}

	/* END UNDERWATER */
	act ("$n breathes out a cloud of poisonous gas!", ch, NULL, NULL, TO_ROOM);
	act ("You breath out a cloud of poisonous gas.", ch, NULL, NULL, TO_CHAR);
	hpch = UMAX (16, ch->hit);
	hp_dam = number_range (hpch / 15 + 1, 8);
	dice_dam = dice (level, 12);
	dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
	dam = dam * 2 / 3;
	dam = dam / 4 * 6;
	poison_effect (ch->in_room, level, dam, TARGET_ROOM);
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next_in_room;
		if (is_safe_spell (ch, vch, TRUE)
			|| (IS_NPC (ch) && IS_NPC (vch)
			&& (ch->fighting == vch || vch->fighting == ch)))
			continue;
		if (saves_spell (level, vch, DAM_POISON))
		{
			poison_effect (vch, level / 2, dam / 4, TARGET_CHAR);
			damage_old (ch, vch, dam / 2, sn, DAM_POISON, TRUE);
		}

		else
		{
			poison_effect (vch, level, dam, TARGET_CHAR);
			damage_old (ch, vch, dam, sn, DAM_POISON, TRUE);
		}
	}
}
void spell_lightning_breath (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam, hp_dam, dice_dam, hpch;

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
	hpch = UMAX (10, ch->hit);
	hp_dam = number_range (hpch / 9 + 1, hpch / 5);
	dice_dam = dice (level, 20);
	dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
	dam = dam * 2 / 3;
	dam = dam / 4 * 6;
	if (saves_spell (level, victim, DAM_LIGHTNING))
	{
		shock_effect (victim, level / 2, dam / 4, TARGET_CHAR);
		damage_old (ch, victim, dam / 2, sn, DAM_LIGHTNING, TRUE);
	}

	else
	{
		shock_effect (victim, level, dam, TARGET_CHAR);
		damage_old (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
	}
}
void spell_aqua_breathe (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (IS_AFFECTED (ch, AFF_AQUA_BREATHE) || IS_SET(ch->imm_flags,IMM_DROWNING))
	{
		if (victim == ch)
			send_to_char ("Your lungs already take the form of gills.\n\r", ch);

		else
			act ("$N already swims like a fish.", ch, NULL, victim, TO_CHAR);
		return;
	}
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level + 2;
	af.location = 0;
	af.modifier = 0;
	af.bitvector = AFF_AQUA_BREATHE;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("Your lungs take the form of gills.\n\r", victim);
	act ("$n turns into a fish.", victim, NULL, NULL, TO_ROOM);
	return;
}

void spell_channel_electricity (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam;
	int hpch;

	/* BEGIN UNDERWATER */
	if (ch->in_room->sector_type != SECT_UNDERWATER
		&& ch->in_room->sector_type != SECT_WATER_OCEAN
		&& ch->in_room->sector_type != SECT_WATER_SWIM)
	{
		send_to_char
			("You try to draw the power of electricity, only it singes your flesh.\n\r",
			ch);
		return;
	}

	/* END UNDERWATER */
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next_in_room;
		if (!is_safe_spell (ch, vch, TRUE))
		{
			hpch = UMAX (10, ch->hit);
			dam = number_range (hpch / 12, hpch / 5);
			if (saves_spell (level, vch, DAM_LIGHTNING))
				dam /= 2;
			damage_old (ch, vch, dam, sn, DAM_LIGHTNING, TRUE);
		}
	}
	return;
}

void spell_electrical_charge (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const sh_int dam_each[] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 28, 31, 34, 37, 40, 40, 41, 42,
	42, 43, 44, 44, 45, 46, 46, 47, 48, 48, 49, 50, 50, 51, 52, 52,
	53, 54, 56, 58, 60, 62, 64,
	66, 68, 70, 72, 74, 76, 78, 80, 82, 84
	};
	int dam;
	level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
	level = UMAX (0, level);
	if (ch->in_room->sector_type != SECT_UNDERWATER
		&& ch->in_room->sector_type != SECT_WATER_OCEAN
		&& ch->in_room->sector_type != SECT_WATER_SWIM)
	{
		send_to_char
			("You try to draw the power of electricity and singe yourself!\n\r",
			ch);
		return;
	}
	dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
	if (saves_spell (level, victim, DAM_LIGHTNING))
		dam /= 1.5;
	damage_old (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
	return;
}


/*
* Spells for mega1.are from Glop/Erkenbrand.
*/
void spell_general_purpose (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	dam = number_range (25, 100);
	if (saves_spell (level, victim, DAM_PIERCE))
		dam /= 2;
	damage_old (ch, victim, dam, sn, DAM_PIERCE, TRUE);
	return;
}

void spell_high_explosive (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	dam = number_range (30, 120);
	if (saves_spell (level, victim, DAM_PIERCE))
		dam /= 2;
	damage_old (ch, victim, dam, sn, DAM_PIERCE, TRUE);
	return;
}


/* RANGER SPELLS */
void spell_eyes_of_the_owl (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	if (IS_AFFECTED (victim, AFF_INFRARED))
	{
		if (victim == ch)
			send_to_char ("You can already see in the dark.\n\r", ch);

		else
			act ("$N can already see in the dark.\n\r", ch, NULL, victim,
			TO_CHAR);
		return;
	}
	act ("$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM);
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = 2 * level;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = AFF_INFRARED;
	af.permaff = FALSE;
	af.composition = FALSE;
	af.comp_name = str_dup ("");
	affect_to_char (victim, &af);
	send_to_char ("Your eyes glow red.\n\r", victim);
	return;
}

void do_brew (CHAR_DATA * ch, char *argument)
{
	char vial_name[MAX_INPUT_LENGTH];
	char spell_name[MAX_INPUT_LENGTH];
	char vial_level[MAX_INPUT_LENGTH];
	char tcbuf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	int sn, found = FALSE, level;
	if (get_skill (ch, gsn_brew) < 1)
	{
		send_to_char ("You don't know how to brew potions.\n\r", ch);
		return;
	}
	argument = one_argument (argument, spell_name);
	argument = one_argument (argument, vial_name);
	one_argument (argument, vial_level);
	if (spell_name[0] == '\0' || vial_name[0] == '\0')
	{
		send_to_char ("Brew what into what?\n\r", ch);
		return;
	}
	sn = skill_lookup (spell_name);
	if (sn == -1)
	{
		send_to_char ("You don't know any spells of that name.\n\r", ch);
		return;
	}
	if (skill_table[sn].type == SKILL_NORMAL)
	{
		send_to_char ("That isn't a valid spell.\n\r", ch);
		return;
	}
	if (sn == skill_lookup ("haven")
		|| sn == skill_lookup ("aqua breathe") || sn ==
		skill_lookup ("mirror") || sn == gsn_fireshield)
	{
		if (skill_table[sn].type == SKILL_PRAY)
			send_to_char
			("Sorry, that prayer cannot be brewed into a potion.\n\r", ch);

		else
			send_to_char
			("Sorry, that spell cannot be brewed into a potion.\n\r", ch);
		return;
	}
	if (is_number (vial_level))
		level = atoi (vial_level);

	else
		level = ch->level;
	if (level > ch->level)
	{
		sprintf (tcbuf,
			"You may only brew potions of level %d and below.\n\r",
			ch->level);
		send_to_char (tcbuf, ch);
		return;
	}
	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		if (can_see_obj (ch, obj))
			if (is_name (vial_name, obj->name))
			{
				found = TRUE;
				break;
			}
			if (!found)
			{
				send_to_char ("You do not see that here.\n\r", ch);
				return;
			}
			if (obj->item_type != ITEM_VIAL)
			{
				send_to_char
					("That item is not capable of holding a brewed potion.\n\r", ch);
				return;
			}
			if (obj->value[0] != 0)
			{
				sprintf (tcbuf, "%s already contains a brew.\n\r",
					fcapitalize (obj->pIndexData->short_descr));
				send_to_char (tcbuf, ch);
				return;
			}
			// Akamai 6/5/98 -- Bug: #6 A skill should not improve/happen if
			// the player does not have enough mana
			if (ch->mana < skill_table[sn].min_mana * 4)
			{
				send_to_char
					("You don't have enough mana to brew that potion.\n\r", ch);
				return;
			}
			if ((number_percent () < get_skill (ch, gsn_brew)) &&
				(number_percent () < get_skill (ch, sn)))
			{
				ch->mana = UMAX (0, (ch->mana - (skill_table[sn].min_mana * 4)));
				sprintf (tcbuf, "You brew %s of %s.\n\r", obj->short_descr,
					skill_table[sn].name);
				send_to_char (tcbuf, ch);
				sprintf (tcbuf, "$n brews %s of %s.", obj->short_descr,
					skill_table[sn].name);
				act (tcbuf, ch, NULL, NULL, TO_ROOM);
				if (get_skill (ch, gsn_brew) != 100)
				{
					if (number_percent () < 10)
						obj->value[0] = -1;

					else
						obj->value[0] = sn;
				}

				else
					obj->value[0] = sn;
				obj->level = level;
				sprintf (tcbuf, "%s of %s", obj->short_descr, skill_table[sn].name);
				free_string (obj->short_descr);
				obj->short_descr = str_dup (tcbuf);
				sprintf (tcbuf, "%s %s",obj->name, skill_table[sn].name);
				free_string (obj->name);
				obj->name = str_dup(tcbuf);
				check_improve (ch, gsn_brew, TRUE, 1);
				return;
			}

			else
			{
				ch->mana = UMAX (0, (ch->mana - (skill_table[sn].min_mana * 2)));
				sprintf (tcbuf, "You fail in your attempt to brew %s of %s.\n\r",
					obj->short_descr, skill_table[sn].name);
				send_to_char (tcbuf, ch);
				sprintf (tcbuf, "$n attempts to brew %s of %s, but fails.",
					obj->short_descr, skill_table[sn].name);
				act (tcbuf, ch, NULL, NULL, TO_ROOM);
				check_improve (ch, gsn_brew, FALSE, 1);
				return;
			}
}
void do_scribe (CHAR_DATA * ch, char *argument)
{
	char parchment_name[MAX_INPUT_LENGTH];
	char spell_name[MAX_INPUT_LENGTH];
	char tcbuf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	int sn, found = FALSE;
	bool isnew = FALSE;
	if (get_skill (ch, gsn_scribe) < 1)
	{
		send_to_char ("Huh?\n\r", ch);
		return;
	}
	argument = one_argument (argument, spell_name);
	one_argument (argument, parchment_name);
	if (spell_name[0] == '\0' || parchment_name[0] == '\0')
	{
		send_to_char ("Scribe what onto what?\n\r", ch);
		return;
	}
	if ((sn = skill_lookup (spell_name)) == -1)
	{
		send_to_char ("You don't know any spells of that name.\n\r", ch);
		return;
	}
	if (skill_table[sn].type == SKILL_NORMAL)
	{
		send_to_char ("That isn't a valid spell.\n\r", ch);
		return;
	}
	if (sn == skill_lookup ("haven") || sn == gsn_mirror
		|| sn == skill_lookup ("aqua breathe")
		|| sn == gsn_fireshield
		|| sn == skill_lookup ("bone craft")
		|| sn == skill_lookup ("penance") || sn ==
		skill_lookup ("edification") || sn == skill_lookup ("deconsecrate"))
	{
		if (skill_table[sn].type == SKILL_PRAY)
			send_to_char
			("Sorry, that prayer cannot be scribed onto a scroll.\n\r", ch);

		else
			send_to_char
			("Sorry, that spell cannot be scribed onto a scroll.\n\r", ch);
		return;
	}
	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		if (can_see_obj (ch, obj))
			if (is_name (parchment_name, obj->name))
			{
				found = TRUE;
				break;
			}
			if (!found)
			{
				send_to_char ("You do not see that here.\n\r", ch);
				return;
			}
			if (obj->item_type == ITEM_SCROLL)
			{
				send_to_char
					("That item is not capable of holding another spell.\n\r", ch);
				return;
			}
			if (obj->item_type != ITEM_PARCHMENT)
			{
				send_to_char
					("That item is not capable of holding a scribed spell.\n\r", ch);
				return;
			}
			// Akamai 6/5/98 -- Bug: #6 Scribe works before mana check
			// This mana check should happen before the check to see if the skill succeeds
			if (ch->mana < skill_table[sn].min_mana * 4)
			{
				send_to_char
					("You don't have enough mana to scribe that spell.\n\r", ch);
				return;
			}
			if ((number_percent () < get_skill (ch, gsn_scribe)) &&
				(number_percent () < get_skill (ch, sn)))
			{
				if (obj->value[1] <= 0)
				{
					isnew = TRUE;
					obj->value[1] = sn;
					obj->value[2] = -1;
				}
				else if ((obj->value[2] <= 0))
				{
					if (  ((skill_table[obj->value[1]].type == SKILL_CAST) && ( (skill_table[sn].type == SKILL_PRAY)||(skill_table[sn].type == SKILL_CHANT)))
						||((skill_table[obj->value[1]].type == SKILL_CHANT) && ( (skill_table[sn].type == SKILL_PRAY)||(skill_table[sn].type == SKILL_CAST)))
						||((skill_table[obj->value[1]].type == SKILL_PRAY) && ( (skill_table[sn].type == SKILL_CAST)||(skill_table[sn].type == SKILL_CHANT))) ) 
					{
						send_to_char ("One parchment can not hold such incompatible types of magic.\n\r",ch);	
						return;
					}
					if ( (true_level_for_skill(ch,obj->value[1]) + true_level_for_skill (ch, sn) ) > 90)
					{
						//			sprintf(tcbuf,"level -> %d, true level -> %d",level_for_skill(ch,obj->value[1]), true_level_for_skill (ch, sn));
						//			send_to_char(tcbuf,ch);
						send_to_char("This parchment can not handle such a powerful combination.\n\r",ch);
						return;
					}
					obj->value[2] = sn;
					obj->value[3] = -1;
				}
				else
				{
					send_to_char ("This parchment can be imbued with no more magical etchings.\n\r", ch);
					return;
				}
				ch->mana = UMAX (0, (ch->mana - (skill_table[sn].min_mana * 4)));
				sprintf (tcbuf, "You scribe %s on to %s.\n\r",
					skill_table[sn].name, obj->pIndexData->short_descr);
				send_to_char (tcbuf, ch);
				sprintf (tcbuf, "$n scribes %s on to %s.", skill_table[sn].name,
					obj->pIndexData->short_descr);
				act (tcbuf, ch, NULL, NULL, TO_ROOM);
				obj->value[0] = ch->level;
				if (isnew)
					sprintf (tcbuf, "%s of %s", obj->short_descr, skill_table[sn].name);

				else
					sprintf (tcbuf, "%s and %s", obj->short_descr, skill_table[sn].name);
				obj->value[3] = -1;
				obj->value[4] = -1;
				obj->level = ch->level;

				// ok, the scribe succeeds
				free_string (obj->short_descr);
				obj->short_descr = str_dup (tcbuf);
				sprintf (tcbuf, "%s %s", obj->name, skill_table[sn].name);
				free_string (obj->name);
				obj->name = str_dup(tcbuf);
				check_improve (ch, gsn_scribe, TRUE, 1);
				return;
			}

			else
			{
				ch->mana = UMAX (0, (ch->mana - (skill_table[sn].min_mana * 2)));
				sprintf (tcbuf,
					"You fail in your attempt to scribe %s on to %s.\n\r",
					skill_table[sn].name, obj->pIndexData->short_descr);
				send_to_char (tcbuf, ch);
				sprintf (tcbuf, "$n attempts to scribe %s on to %s, but fails.",
					skill_table[sn].name, obj->pIndexData->short_descr);
				act (tcbuf, ch, NULL, NULL, TO_ROOM);
				check_improve (ch, gsn_scribe, FALSE, 1);
				return;
			}
}
