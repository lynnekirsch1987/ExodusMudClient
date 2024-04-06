/* Store Reaver Specific Stuff Here! */
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"
#include "obj_trig.h"

//Iblis - File originally created and the vast majority of stuff done by Adeon, I just fixed a lot of
//the bugs that he left when he vanished after initially coding them

/* Local Functions */
void sword_upgrade (CHAR_DATA * ch, char *argument);
void sword_status (CHAR_DATA * ch);
void sword_dismiss (CHAR_DATA * ch);
//5-12-03 Iblis - Added to allow the affects of the sword to be viewed
void sword_affects (CHAR_DATA * ch);
void sword_respec(CHAR_DATA * ch);
//5-11-03 Iblis - Added to correctly computer hit/damroll directly after upgrading
void sword_reequip (CHAR_DATA * ch);

long get_next_cost (OBJ_DATA * sword, int value);
long get_slot_cost (OBJ_DATA * sword, int value);
long get_burst_cost (OBJ_DATA * sword, int value);
long get_souls_in_sword(OBJ_DATA * sword);
AFFECT_DATA *get_hit_affect (OBJ_DATA * sword);
AFFECT_DATA *get_dam_affect (OBJ_DATA * sword);
void upgrade_blade_spell (CHAR_DATA * sword, int type, long cost, int level);
void upgrade_burst_spell (CHAR_DATA * sword, int type, long cost, int level);
char *get_bs_name (int bs);
char *get_bb_name (int burst);
void create_reaver_sword(CHAR_DATA * ch);

extern bool doub;
extern bool half;
extern bool quad;

/* Burst Invocation Subset */
void invoke_burst (CHAR_DATA * ch, OBJ_DATA * sword, int type);
void invoke_lightning_burst (CHAR_DATA * ch, OBJ_DATA * sword);
void invoke_ice_burst (CHAR_DATA * ch, OBJ_DATA * sword);
void invoke_fire_burst (CHAR_DATA * ch, OBJ_DATA * sword);
void invoke_unholy_burst (CHAR_DATA * ch, OBJ_DATA * sword);
void invoke_hellscape_burst (CHAR_DATA * ch, OBJ_DATA * sword);



/* This determines the number of souls gained per kill */
int compute_soul_gain (CHAR_DATA * ch, CHAR_DATA * victim)
{
	int gain;
	int level_value;
	int align_deviation;
	int align_penalty;
	int vict_align_diff;
	OBJ_DATA *stone;


	if (IS_SET(victim->act2,ACT_NO_ALIGN))
		return 0;
	align_penalty = ch->alignment + 1000;
	vict_align_diff = 1000 - victim->alignment;

	align_deviation = (vict_align_diff + align_penalty);
	level_value = (victim->level);

	gain = level_value - (align_deviation / 30);
	gain += ch->exp % 7;		/*randomize it a bit */

	if (gain < 0)
		gain = 1;			/*1 at least */
	if (doub)
		gain *= 2;
	if (quad)
		gain *= 4;
	if (half)
		gain *= .5;
	stone = get_eq_char (ch, WEAR_HOLD);
	if (stone != NULL && (stone->pIndexData->vnum == OBJ_VNUM_RUNE_CONV))      
	gain *= 2;

	return gain;
}


void do_sword (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];

	if (!IS_CLASS (ch, PC_CLASS_REAVER) || IS_NPC (ch))
	{
		send_to_char ("Only reavers may perform this operation.\n\r", ch);
		return;
	}

	if ( (get_eq_char(ch, WEAR_WIELD_L) == NULL && get_eq_char(ch, WEAR_WIELD_R) == NULL) ||  
		get_obj_wear(ch, "reaver sword") == NULL)
	{
		send_to_char ("You do not currently possess your sword.\n\r", ch);
		return;
	}

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	if (!str_cmp (arg1, "status"))
	{
		sword_status (ch);
		return;
	}

	if (!str_cmp (arg1, "upgrade"))
	{
		sword_upgrade (ch, arg2);
		return;
	}

	if (!str_cmp (arg1, "dismiss"))
	{
		sword_dismiss (ch);
		return;
	}
	//Iblis 5/12/03 - Needed for sword_affects
	if (!str_cmp (arg1, "affects"))
	{
		sword_affects (ch);
		return;
	}

	if (!str_cmp (arg1, "respec"))
	{
		sword_respec(ch);
		return;
	}

	sprintf(buf, "Sword options for %s:\n\r\n\r", ch->sword->short_descr);
	send_to_char(buf, ch);
	send_to_char("  sword status  - check soul energies and blade spells on your weapon\n\r", ch);
	//Iblis 5/12/03 - self-explanatory
	send_to_char("  sword affects - check the chants active on your weapon\n\r", ch);
	send_to_char("  sword upgrade - enhance your sword\n\r", ch);
	send_to_char("  sword dismiss - dismiss your sword for later recall\n\r", ch);
	send_to_char("  sword respec - removes all upgrades from your sword returning all soul energies\n\r", ch);

	return;
}

void sword_upgrade (CHAR_DATA * ch, char *argument)
{
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
	long cost_apathy_blade;
	long cost_lightning_burst;
	long cost_ice_burst;
	long cost_fire_burst;
	long cost_unholy_burst;
	long cost_hellscape_burst;
	char buf[MAX_STRING_LENGTH];

	int level_to_use;

	AFFECT_DATA *paf;

	cost_power = get_next_cost (ch->sword, BLADE_POWER);
	cost_hatred = get_next_cost (ch->sword, BLADE_HATRED);
	cost_fury = get_next_cost (ch->sword, BLADE_FURY);
	cost_strength = get_next_cost (ch->sword, BLADE_STRENGTH);

	cost_shocking_blade = get_slot_cost (ch->sword, BLADE_SPELL_SHOCKING);
	cost_frost_blade = get_slot_cost (ch->sword, BLADE_SPELL_FROST);
	cost_flaming_blade = get_slot_cost (ch->sword, BLADE_SPELL_FLAMING);
	cost_chaotic_blade = get_slot_cost (ch->sword, BLADE_SPELL_CHAOTIC);
	cost_venemous_blade = get_slot_cost (ch->sword, BLADE_SPELL_VENEMOUS);
	cost_vampiric_blade = get_slot_cost (ch->sword, BLADE_SPELL_VAMPIRIC);
	cost_apathy_blade = get_slot_cost (ch->sword, BLADE_SPELL_APATHY);

	cost_lightning_burst = get_burst_cost (ch->sword, BURST_LIGHTNING);
	cost_ice_burst = get_burst_cost (ch->sword, BURST_ICE);
	cost_fire_burst = get_burst_cost (ch->sword, BURST_FIRE);
	cost_unholy_burst = get_burst_cost (ch->sword, BURST_UNHOLY);
	cost_hellscape_burst = get_burst_cost (ch->sword, BURST_HELLSCAPE);

	if (!IS_SET (ch->in_room->room_flags, ROOM_SHRINE))
	{
		send_to_char ("You may only do this at a dark shrine.\n\r", ch);
		return;
	}

	if (!str_cmp (argument, "power"))
	{
		if (ch->pcdata->souls < cost_power)
		{
			send_to_char("`aYou do not possess enough soul energy to upgrade power.``\n\r",	ch);
			return;
		}

		sprintf (buf, "`aYou submit `b%ld `asouls to the dark lord.``\n\r",	cost_power);
		send_to_char (buf, ch);
		send_to_char ("`iYour blade has increased in power!``\n\r", ch);
		ch->pcdata->souls -= cost_power;
		ch->sword->value[2]++;

		//5-10-03 Iblis - One of the lines of another dam/hitroll reaver bug..
		//update_sword(ch,ch->sword);
		//5-11-03 Iblis - Added so that hit/damroll would be computed directly after update
		sword_reequip (ch);
		return;
	}

	if (!str_cmp (argument, "hatred"))
	{
		if (ch->pcdata->souls < cost_hatred)
		{
			send_to_char("`aYou do not possess enough soul energy to upgrade hatred.``\n\r", ch);
			return;
		}

		sprintf (buf, "`aYou submit `b%ld `asouls to the dark lord.``\n\r",	cost_hatred);
		send_to_char (buf, ch);
		send_to_char ("`iYour blade has increased in hatred!``\n\r", ch);
		ch->pcdata->souls -= cost_hatred;
		ch->sword->value[1]++;

		//5-10-03 Iblis - One of the lines of another dam/hitroll reaver bug
		//update_sword(ch, ch->sword);
		//5-11-03 Iblis - Added so that hit/damroll would be computed directly after update
		sword_reequip (ch);
		return;
	}


	if (!str_cmp (argument, "fury"))
	{
		if (ch->pcdata->souls < cost_fury)
		{
			send_to_char("`aYou do not possess enough soul energy to upgrade fury.``\n\r", ch);
			return;
		}

		sprintf (buf, "`aYou submit `b%ld `asouls to the dark lord.``\n\r",	cost_fury);
		send_to_char (buf, ch);
		send_to_char ("`iYour blade has increased in fury!``\n\r", ch);
		ch->pcdata->souls -= cost_fury;


		if ((paf = get_hit_affect (ch->sword)) != NULL)
		{
			paf->type = 1;
			paf->modifier += 1;
			paf->level = UMAX (paf->level, ch->level);
			if (paf->modifier > 4)
				SET_BIT (ch->sword->extra_flags[0], ITEM_HUM);
		}
		else
		{			/* If we don't have any fury yet... */

			paf = new_affect ();
			paf->where = TO_OBJECT;
			paf->type = 1;
			paf->level = ch->level;
			paf->duration = -1;
			paf->location = APPLY_HITROLL;
			paf->modifier = 1;
			paf->bitvector = 0;
			paf->permaff = TRUE;
			paf->composition = FALSE;
			paf->comp_name = str_dup ("");
			paf->next = ch->sword->affected;
			ch->sword->affected = paf;
		}

		//5-10-03 Iblis - One of the lines of anohter dam/hitroll reaver bug
		//update_sword(ch, ch->sword);
		//5-11-03 Iblis - Added so that hit/damroll would be computed directly after update
		sword_reequip (ch);
		return;
	}

	if (!str_cmp (argument, "strength"))
	{
		if (ch->pcdata->souls < cost_strength)
		{
			send_to_char("`aYou do not possess enough soul energy to upgrade strength.``\n\r", ch);
			return;
		}

		sprintf (buf, "`aYou submit `b%ld `asouls to the dark lord.``\n\r",	cost_strength);
		send_to_char (buf, ch);
		send_to_char ("`iYour blade has increased in strength!``\n\r", ch);
		ch->pcdata->souls -= cost_strength;


		if ((paf = get_dam_affect (ch->sword)) != NULL)
		{
			paf->type = 1;
			paf->modifier += 1;
			paf->level = UMAX (paf->level, ch->level);
			if (paf->modifier > 4)
				SET_BIT (ch->sword->extra_flags[0], ITEM_HUM);
		}
		else
		{			/* If we don't have any strength yet... */

			paf = new_affect ();
			paf->where = TO_OBJECT;
			paf->type = 1;
			paf->level = ch->level;
			paf->duration = -1;
			paf->location = APPLY_DAMROLL;
			paf->modifier = 1;
			paf->bitvector = 0;
			paf->permaff = TRUE;
			paf->composition = FALSE;
			paf->comp_name = str_dup ("");
			paf->next = ch->sword->affected;
			ch->sword->affected = paf;
		}

		//5-10-03 Iblis - One of the lines of anohter dam/hitroll reaver bug
		//update_sword(ch, ch->sword);
		//5-11-03 Iblis - Added so that hit/damroll would be computed directly after update
		sword_reequip (ch);
		return;
	}

	if (!str_cmp (argument, "knightsblade"))
	{
		if (ch->level < 40)
		{
			send_to_char("You are not advanced enough to upgrade to a chaotic edge.\n\r", ch);
			return;
		}

		if (IS_SET (ch->sword->value[4], WEAPON_LANCE))
		{
			send_to_char ("You sword is already a KnightsBlade.\n\r", ch);
			return;
		}

		if (ch->pcdata->souls < 10000)
		{
			send_to_char("You do not posses enough soul energy to upgrade to a KnightsBlade.\n\r", ch);
			return;
		}
		//IBlis 7/29/03 Fixed bug that knightsblade is FREE
		ch->pcdata->souls -= 10000;
		SET_BIT (ch->sword->value[4], WEAPON_LANCE);

		send_to_char("`aYou submit `b10000`` `asouls to the dark lord.``\n\r", ch);
		send_to_char("`aYour blade seems to grow in length.``\n\r", ch);
		//5-11-03 Iblis - Added so that the lance flag is immediately added
		sword_reequip (ch);
		//update_sword(ch, ch->sword);

		return;
	}



	if (!str_cmp (argument, "shocking"))
	{
		level_to_use = 7;
		upgrade_blade_spell (ch, BLADE_SPELL_SHOCKING, cost_shocking_blade,	level_to_use);
		update_sword (ch, ch->sword);
		return;
	}

	if (!str_cmp (argument, "frost"))
	{
		level_to_use = 21;
		upgrade_blade_spell (ch, BLADE_SPELL_FROST, cost_frost_blade, level_to_use);
		update_sword (ch, ch->sword);
		return;
	}

	if (!str_cmp (argument, "apathy"))
	{
		level_to_use = 31;
		upgrade_blade_spell (ch, BLADE_SPELL_APATHY, cost_apathy_blade, level_to_use);
		update_sword (ch, ch->sword);
		return;
	}

	if (!str_cmp (argument, "flaming"))
	{
		level_to_use = 35;
		upgrade_blade_spell (ch, BLADE_SPELL_FLAMING, cost_flaming_blade, level_to_use);
		update_sword (ch, ch->sword);
		return;
	}

	if (!str_cmp (argument, "chaotic"))
	{
		level_to_use = 49;
		upgrade_blade_spell (ch, BLADE_SPELL_CHAOTIC, cost_chaotic_blade, level_to_use);
		update_sword (ch, ch->sword);
		return;
	}

	if (!str_cmp (argument, "venomous"))
	{
		level_to_use = 63;
		upgrade_blade_spell (ch, BLADE_SPELL_VENEMOUS, cost_venemous_blade, level_to_use);
		update_sword (ch, ch->sword);
		return;
	}

	if (!str_cmp (argument, "vampiric"))
	{
		level_to_use = 70;
		upgrade_blade_spell (ch, BLADE_SPELL_VAMPIRIC, cost_vampiric_blade, level_to_use);
		update_sword (ch, ch->sword);
		return;
	}

	if (!str_cmp (argument, "lightning"))
	{
		level_to_use = 14;
		upgrade_burst_spell (ch, BURST_LIGHTNING, cost_lightning_burst, level_to_use);
		update_sword (ch, ch->sword);
		return;
	}

	if (!str_cmp (argument, "ice"))
	{
		level_to_use = 28;
		upgrade_burst_spell (ch, BURST_ICE, cost_ice_burst, level_to_use);
		update_sword (ch, ch->sword);
		return;
	}

	if (!str_cmp (argument, "fire"))
	{
		level_to_use = 42;
		upgrade_burst_spell (ch, BURST_FIRE, cost_fire_burst, level_to_use);
		update_sword (ch, ch->sword);
		return;
	}

	if (!str_cmp (argument, "unholy"))
	{
		level_to_use = 56;
		upgrade_burst_spell (ch, BURST_UNHOLY, cost_unholy_burst, level_to_use);
		update_sword (ch, ch->sword);
		return;
	}

	if (!str_cmp (argument, "hellscape"))
	{
		level_to_use = 77;
		upgrade_burst_spell (ch, BURST_HELLSCAPE, cost_hellscape_burst,	level_to_use);
		update_sword (ch, ch->sword);
		return;
	}

	send_to_char ("`aUpgrades available       Cost``\n\r", ch);
	sprintf (buf, "  `ePower               `b%6ld`hSE``\n\r", cost_power);
	send_to_char (buf, ch);
	sprintf (buf, "  `eHatred              `b%6ld`hSE``\n\r", cost_hatred);
	send_to_char (buf, ch);
	sprintf (buf, "  `eFury                `b%6ld`hSE``\n\r", cost_fury);
	send_to_char (buf, ch);
	sprintf (buf, "  `eStrength            `b%6ld`hSE``\n\r", cost_strength);
	send_to_char (buf, ch);
	if (ch->level >= 40 && !IS_WEAPON_STAT (ch->sword, WEAPON_LANCE))
	{
		sprintf (buf, "  `eKnightsblade        `b%6d`hSE``\n\r", 10000);
		send_to_char (buf, ch);
	}
	if (ch->level >= 7)
	{
		sprintf (buf, "  `eShocking Blade      `b%6ld`hSE``\n\r", cost_shocking_blade);
		send_to_char (buf, ch);
	}
	if (ch->level >= 21)
	{
		sprintf (buf, "  `eFrost Blade         `b%6ld`hSE``\n\r", cost_frost_blade);
		send_to_char (buf, ch);
	}
	if (ch->level >= 31 && is_Class_tiers(ch,PC_CLASS_REAVER,2))
	{
		sprintf (buf, "  `eApathy Blade        `b%6ld`hSE``\n\r", cost_apathy_blade);
		send_to_char (buf, ch);
	}
	if (ch->level >= 35 && is_Class_tiers(ch,PC_CLASS_REAVER,2))
	{
		sprintf (buf, "  `eFlaming Blade       `b%6ld`hSE``\n\r", cost_flaming_blade);
		send_to_char (buf, ch);
	}
	if (ch->level >= 49 && is_Class_tiers(ch,PC_CLASS_REAVER,2))
	{
		sprintf (buf, "  `eChaotic Blade       `b%6ld`hSE``\n\r", cost_chaotic_blade);
		send_to_char (buf, ch);
	}
	if (ch->level >= 63 && is_Class_tiers(ch,PC_CLASS_REAVER,3))
	{
		sprintf (buf, "  `eVenomous Blade      `b%6ld`hSE``\n\r", cost_venemous_blade);
		send_to_char (buf, ch);
	}
	if (ch->level >= 70 && is_Class_tiers(ch,PC_CLASS_REAVER,3))
	{
		sprintf (buf, "  `eVampiric Blade      `b%6ld`hSE``\n\r", cost_vampiric_blade);
		send_to_char (buf, ch);
	}
	if (ch->level >= 14)
	{
		sprintf (buf, "  `eLightning Burst     `b%6ld`hSE``\n\r", cost_lightning_burst);
		send_to_char (buf, ch);
	}
	if (ch->level >= 28)
	{
		sprintf (buf, "  `eIce Burst           `b%6ld`hSE``\n\r", cost_ice_burst);
		send_to_char (buf, ch);
	}
	if (ch->level >= 42 && is_Class_tiers(ch,PC_CLASS_REAVER,2))
	{
		sprintf (buf, "  `eFire Burst          `b%6ld`hSE``\n\r", cost_fire_burst);
		send_to_char (buf, ch);
	}
	if (ch->level >= 56 && is_Class_tiers(ch,PC_CLASS_REAVER,2))
	{
		sprintf (buf, "  `eUnholy Burst        `b%6ld`hSE``\n\r", cost_unholy_burst);
		send_to_char (buf, ch);
	}
	if (ch->level >= 77 && is_Class_tiers(ch,PC_CLASS_REAVER,3))
	{
		sprintf (buf, "  `eHellscape           `b%6ld`hSE``\n\r", cost_hellscape_burst);
		send_to_char (buf, ch);
	}
	return;

}

void upgrade_blade_spell (CHAR_DATA * ch, int type, long cost, int level)
{
	char buf[MAX_STRING_LENGTH];

	if (ch->level < level)
	{
		send_to_char("`aYou are not advanced enough to upgrade that blade spell.``\n\r", ch);
		return;
	}

	if (ch->pcdata->souls < cost)
	{
		send_to_char("`aYou do not possess enough soul energy to upgrade that blade spell.``\n\r", ch);
		return;
	}

	sprintf (buf, "`aYou submit `b%ld `asouls to the dark lord.``\n\r", cost);
	send_to_char (buf, ch);

	ch->pcdata->souls -= cost;
	ch->sword->bs_capacity[type]++;

	switch (type)
	{
		case BLADE_SPELL_SHOCKING:
			send_to_char("`nYour blade buzzes with additional electric potential.``\n\r", ch);
			break;
		case BLADE_SPELL_FROST:
			send_to_char ("`nYour blade becomes more cool to the touch.``\n\r", ch);
			break;
		case BLADE_SPELL_FLAMING:
			send_to_char ("`iYour blade flickers with a glowing flame.``\n\r", ch);
			break;
		case BLADE_SPELL_CHAOTIC:
			send_to_char("`aYour blade glints a chaotic gleam in the light.``\n\r", ch);
			break;
		case BLADE_SPELL_VENEMOUS:
			send_to_char("`fThe edges of your blade glisten with a venomous coating.``\n\r", ch);
			break;
		case BLADE_SPELL_VAMPIRIC:
			send_to_char ("`aA horrendous howl eminates from your blade.``\n\r", ch);
			break;
		case BLADE_SPELL_APATHY:
			send_to_char ("`aA dark shroud envelops your blade.``\n\r",ch);
			break;
	}

	return;
}

void upgrade_burst_spell (CHAR_DATA * ch, int type, long cost, int level)
{
	char buf[MAX_STRING_LENGTH];

	if (ch->level < level)
	{
		send_to_char("`aYou are not advanced enough to upgrade that burst spell.``\n\r", ch);
		return;
	}

	if (ch->pcdata->souls < cost)
	{
		send_to_char("`aYou do not possess enough soul energy to upgrade that burst spell.``\n\r", ch);
		return;
	}

	sprintf (buf, "`aYou submit `b%ld `asouls to the dark lord.``\n\r", cost);
	send_to_char (buf, ch);

	ch->pcdata->souls -= cost;
	ch->sword->bb_capacity[type]++;

	switch (type)
	{
		case BURST_LIGHTNING:
			send_to_char ("`nStatic charges crackle along your blade.``\n\r", ch);
			break;
		case BURST_ICE:
			send_to_char ("`nYour blade briefly sparkles with frost.``\n\r", ch);
			break;
		case BURST_FIRE:
			send_to_char ("`iYour balde glows red with fiery power.``\n\r", ch);
			break;
		case BURST_UNHOLY:
			send_to_char ("`aYour blade takes on an eerie unholy glow.``\n\r", ch);
			break;
		case BURST_HELLSCAPE:
			send_to_char ("`aYour blade roars with hells fury!``\n\r", ch);
			break;
	}

	return;
}

void sword_status (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH];
	int i;

	switch(ch->pcdata->souls)
	{
		case 0:
			send_to_char("`aThere are currently `bno `asoul energies stored in your blade.``\n\r", ch);
			break;

		case 1:
			send_to_char("`aThere is currently `b1 `asoul energy stored in your blade.``\n\r", ch);
			break;

		default:
			sprintf(buf, "`aThere are currently `b%ld `asoul energies stored in your blade.``\n\r", ch->pcdata->souls);
			send_to_char (buf, ch);
			break;
	}

	for (i = 0; i < MAX_BLADE_SPELLS; i++)
	{
		if (ch->sword->bs_capacity[i] > 0)
		{
			sprintf (buf, "`aYou have `b%d `aout of a possible `b%d `aspells of %s blade.``\n\r",
				ch->sword->bs_charges[i], ch->sword->bs_capacity[i],
				get_bs_name (i));
			send_to_char (buf, ch);
		}
	}

	for (i = 0; i < MAX_BURST; i++)
	{
		if (ch->sword->bb_capacity[i] > 0)
		{
			sprintf (buf, "`aYou have `b%d `aout of a possible `b%d `aspells of %s.``\n\r",
				ch->sword->bb_charges[i], ch->sword->bb_capacity[i],
				get_bb_name (i));
			send_to_char (buf, ch);
		}
	}

	return;
}

void sword_dismiss (CHAR_DATA * ch)
{
	OBJ_DATA *sword;
	char buf[MAX_STRING_LENGTH];

	if ((sword = get_obj_wear (ch, "reaver sword")) == NULL)
	{
		send_to_char ("You do not currently possess your sword.\n\r", ch);
		return;
	}

	sprintf (buf, "`a%s `adissapears in a puff of black smoke.``\n\r", sword->short_descr);
	send_to_char (buf, ch);
	act("`a$n utters the dark words of return, making the sword vanish in a puff of black smoke.``", ch, NULL, NULL, TO_ROOM);
	unequip_char (ch, sword);
	obj_from_char (sword);
	extract_obj (sword);
	reset_char (ch);
	return;
}


//Iblis 5/12/03 - Added a function that allows a reaver to see the affects on their weapon
//as well as how long the affect lasts for
void sword_affects (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *sword;
	AFFECT_DATA *paf;
	bool affected = 0;
	if ((sword = get_obj_wear (ch, "reaver sword")) == NULL)
	{
		send_to_char ("You do not currently possess your sword.\n\r", ch);
		return;
	}
	if (sword->affected == NULL)
		send_to_char ("`aYour blade currently has no flags on it.\n\r", ch);
	for (paf = sword->affected; paf != NULL; paf = paf->next)
	{
		if (paf->bitvector)
		{
			if (paf->where == TO_WEAPON)
			{
				affected = 1;
				if (paf->duration == 1)
					sprintf (buf, "`aYour blade has the`b %s `aflag for`b %d `ahour.``\n",
					weapon_bit_name (paf->bitvector), paf->duration);
				else
					sprintf (buf, "`aYour blade has the`b %s `aflag for`b %d `ahours.``\n",
					weapon_bit_name (paf->bitvector), paf->duration);
			}
			else
				sprintf (buf, "`aUnknown bit`b %d: %d\n\r", paf->where,
				paf->bitvector);
			send_to_char (buf, ch);
		}
	}
	if (!affected)
		send_to_char ("`aYour blade currently has no flags on it.``\n\r", ch);
}

void sword_respec(CHAR_DATA * ch)
{
	long
		souls_in_sword = 0;
	OBJ_DATA
		*sword = NULL;
	char 
		buf[MAX_STRING_LENGTH];


	if( !IS_SET(ch->in_room->room_flags, ROOM_SHRINE) )
	{
		send_to_char("You may only do this at a dark shrine.\n\r", ch);
		return;
	}

	sword = get_obj_wear(ch, "reaver sword");
	if( sword == NULL )
	{
		send_to_char("You do not currently possess your sword.\n\r", ch);
		return;
	}

	send_to_char("`aYou drive your sword into the ground and kneel to mutter a dark chant.``\n\r", ch);
	act("`a$n drives $s sword into the ground and kneels to mutter a dark chant.``", ch, NULL, NULL, TO_ROOM);

	souls_in_sword = get_souls_in_sword(sword);
	unequip_char(ch, sword);
	obj_from_char(sword);
	extract_obj(sword);
	reset_char(ch);
	create_reaver_sword(ch);
	ch->pcdata->souls += souls_in_sword;

	if( souls_in_sword > 2 )
	{
		sprintf(buf, "`aThe sword begins to pulse as you extract %ld soul energies from it.``\n\r", souls_in_sword);
		send_to_char(buf, ch);
	}
	else
	{
		send_to_char("`aThe sword begins to pulse as you try to extract soul energies from it.``\n\r", ch);
	}
	send_to_char("`aThe sword vanishes in a puff of black smoke.``\n\r", ch);
	act("`a$n's sword begins to pulse with dark energy.  The sword vanishes in a puff of black smoke.``", ch, NULL, NULL, TO_ROOM);

	return;
}

long get_souls_in_sword(OBJ_DATA * sword)
{
	long
		total_souls = 0,
		cost = 0;
	int
		i = 0,
		level = 0;
	AFFECT_DATA 
		*paf = NULL;

	//////////////////////////////////
	// calculate total amount of souls spent on the sword

	// BLADE_POWER cost
	level = sword->value[2];
	cost = 8;
	for(i = 1; i < level; ++i)
	{
		total_souls += cost;
		cost *= 2;
	}

	// BLADE_HATRED cost
	level = sword->value[1];
	cost = 16;
	for(i = 1; i < level; ++i)
	{
		total_souls += cost;
		cost *= 4;
	}

	// BLADE_FURY cost
	level = 0;
	paf = get_hit_affect(sword);
	if(paf != NULL)
		level = paf->modifier;
	cost = 8;
	for(i = 0; i < level; ++i)
	{
		total_souls += cost;
		cost *= 2;
	}

	// BLADE_STRENGTH cost
	level = 0;
	paf = get_dam_affect(sword);
	if(paf != NULL)
		level = paf->modifier;
	cost = 9;
	for(i = 0; i < level; ++i)
	{
		total_souls += cost;
		cost *= 3;
	}

	// BLADE_SPELL_SHOCKING cost
	level = sword->bs_capacity[BLADE_SPELL_SHOCKING];
	cost = 100;
	for(i = 0; i < level; ++i)
	{
		total_souls += cost;
		cost += 200;
	}

	// BLADE_SPELL_FROST cost
	level = sword->bs_capacity[BLADE_SPELL_FROST];
	cost = 200;
	for(i = 0; i < level; ++i)
	{
		total_souls += cost;
		cost += 300;
	}

	// BLADE_SPELL_FLAMING cost
	level = sword->bs_capacity[BLADE_SPELL_FLAMING];
	cost = 300;
	for(i = 0; i < level; ++i)
	{
		total_souls += cost;
		cost += 400;
	}

	// BLADE_SPELL_CHAOTIC cost
	level = sword->bs_capacity[BLADE_SPELL_CHAOTIC];
	cost = 600;
	for(i = 0; i < level; ++i)
	{
		total_souls += cost;
		cost += 500;
	}

	// BLADE_SPELL_VENEMOUS cost
	level = sword->bs_capacity[BLADE_SPELL_VENEMOUS];
	cost = 900;
	for(i = 0; i < level; ++i)
	{
		total_souls += cost;
		cost += 600;
	}

	// BLADE_SPELL_VAMPIRIC cost
	level = sword->bs_capacity[BLADE_SPELL_VAMPIRIC];
	cost = 1000;
	for(i = 0; i < level; ++i)
	{
		total_souls += cost;
		cost += 1000;
	}

	// BLADE_SPELL_APATHY cost
	level = sword->bs_capacity[BLADE_SPELL_APATHY];
	cost = 1000;
	for(i = 0; i < level; ++i)
	{
		total_souls += cost;
		cost += 6000;
	}

	// BURST_LIGHTNING cost
	level = sword->bb_capacity[BURST_LIGHTNING];
	cost = 1000;
	for(i = 0; i < level; ++i)
	{
		total_souls += cost;
		cost += 1000;
	}

	// BURST_ICE cost
	level = sword->bb_capacity[BURST_ICE];
	cost = 2000;
	for(i = 0; i < level; i++)
	{
		total_souls += cost;
		cost += 2000;
	}

	// BURST_FIRE cost
	level = sword->bb_capacity[BURST_FIRE];
	cost = 3500;
	for(i = 0; i < level; i++)
	{
		total_souls += cost;
		cost += 3500;
	}

	// BURST_UNHOLY cost
	level = sword->bb_capacity[BURST_UNHOLY];
	cost = 10000;
	for(i = 0; i < level; i++)
	{
		total_souls += cost;
		cost += 1500;
	}

	// BURST_HELLSCAPE cost
	level = sword->bb_capacity[BURST_HELLSCAPE];
	cost = 20000;
	for(i = 0; i < level; i++)
	{
		total_souls += cost;
		cost += 2500;
	}

	// KnightBlade cost
	if(IS_SET(sword->value[4], WEAPON_LANCE))
	{
		total_souls += 10000;
	}

	return total_souls;
}

long get_next_cost (OBJ_DATA * sword, int value)
{
	long cost;
	int i;
	AFFECT_DATA *paf;
	int mod;

	switch (value)
	{
		case BLADE_POWER:
			cost = 8;			/*initial value */
			for (i = 1; i < sword->value[2]; i++)
				cost *= 2;
			break;

		case BLADE_HATRED:
			cost = 16;
			for (i = 1; i < sword->value[1]; i++)
				cost *= 4;
			break;

		case BLADE_FURY:
			paf = get_hit_affect (sword);
			if (paf == NULL)
				mod = 0;
			else
				mod = paf->modifier;
			cost = 8;
			for (i = 0; i < mod; i++)
				cost *= 2;
			break;

		case BLADE_STRENGTH:
			paf = get_dam_affect (sword);
			if (paf == NULL)
				mod = 0;
			else
				mod = paf->modifier;
			cost = 9;
			for (i = 0; i < mod; i++)
				cost *= 3;
			break;

		default:
			cost = 0;
			break;
	}

	return cost;
}

long get_slot_cost (OBJ_DATA * sword, int value)
{
	long cost=0;
	int i;

	switch (value)
	{
		case BLADE_SPELL_SHOCKING:
			cost = 100;
			for (i = 0; i < sword->bs_capacity[value]; i++)
				cost += 200;
			break;
		case BLADE_SPELL_FROST:
			cost = 200;
			for (i = 0; i < sword->bs_capacity[value]; i++)
				cost += 300;
			break;
		case BLADE_SPELL_FLAMING:
			cost = 300;
			for (i = 0; i < sword->bs_capacity[value]; i++)
				cost += 400;
			break;
		case BLADE_SPELL_CHAOTIC:
			cost = 600;
			for (i = 0; i < sword->bs_capacity[value]; i++)
				cost += 500;
			break;
		case BLADE_SPELL_VENEMOUS:
			cost = 900;
			for (i = 0; i < sword->bs_capacity[value]; i++)
				cost += 600;
			break;
		case BLADE_SPELL_VAMPIRIC:
			cost = 1000;
			for (i = 0; i < sword->bs_capacity[value]; i++)
				cost += 1000;
			break;
		case BLADE_SPELL_APATHY:
			cost = 1000;
			for (i=0; i < sword->bs_capacity[value]; i++)
				cost += 6000;
			break;
	}
	return cost;
}

long get_burst_cost (OBJ_DATA * sword, int value)
{
	long cost=0;
	int i;

	switch (value)
	{
		case BURST_LIGHTNING:
			cost = 1000;
			for (i = 0; i < sword->bb_capacity[value]; i++)
				cost += 1000;
			break;
		case BURST_ICE:
			cost = 2000;
			for (i = 0; i < sword->bb_capacity[value]; i++)
				cost += 2000;
			break;
		case BURST_FIRE:
			cost = 3500;
			for (i = 0; i < sword->bb_capacity[value]; i++)
				cost += 3500;
			break;
		case BURST_UNHOLY:
			cost = 10000;
			for (i = 0; i < sword->bb_capacity[value]; i++)
				cost += 1500;
			break;
		case BURST_HELLSCAPE:
			cost = 20000;
			for (i = 0; i < sword->bb_capacity[value]; i++)
				cost += 2500;
			break;
	}
	return cost;
}


AFFECT_DATA *get_hit_affect (OBJ_DATA * sword)
{
	AFFECT_DATA *paf;

	for (paf = sword->affected; paf != NULL; paf = paf->next)
	{
		if (paf->location == APPLY_HITROLL)
			return paf;
	}

	return NULL;
}



AFFECT_DATA *get_dam_affect (OBJ_DATA * sword)
{
	AFFECT_DATA *paf;

	for (paf = sword->affected; paf != NULL; paf = paf->next)
	{
		if (paf->location == APPLY_DAMROLL)
			return paf;
	}

	return NULL;
}

/*returns the name associated with the blade spell*/
char *get_bs_name (int bs)
{
	char buf[MAX_STRING_LENGTH];

	switch (bs)
	{
		case BLADE_SPELL_SHOCKING:
			sprintf (buf, "shocking");
			break;
		case BLADE_SPELL_FROST:
			sprintf (buf, "frost");
			break;
		case BLADE_SPELL_FLAMING:
			sprintf (buf, "flaming");
			break;
		case BLADE_SPELL_CHAOTIC:
			sprintf (buf, "chaotic");
			break;
		case BLADE_SPELL_VENEMOUS:
			sprintf (buf, "venomous");
			break;
		case BLADE_SPELL_VAMPIRIC:
			sprintf (buf, "vampiric");
			break;
		case BLADE_SPELL_APATHY:
			sprintf (buf, "apathy");
			break;
	}

	return str_dup (buf);
}

char *get_bb_name (int burst)
{
	char buf[MAX_STRING_LENGTH];

	switch (burst)
	{
		case BURST_LIGHTNING:
			sprintf (buf, "lightning burst");
			break;
		case BURST_ICE:
			sprintf (buf, "ice burst");
			break;
		case BURST_FIRE:
			sprintf (buf, "fire burst");
			break;
		case BURST_UNHOLY:
			sprintf (buf, "unholy burst");
			break;
		case BURST_HELLSCAPE:
			sprintf (buf, "hellscape");
			break;
	}

	return str_dup (buf);
}


//5-11-03 Iblis - This is cheap function, consisting of copy/paste of code
//from sword_dimiss and chant summon_sword.  This function allows the updated
//sword to be made and immediately worn by the character.
//ESSENTIALLY this is sword_update plus it remove the old sword, creates an
//updated sword, and make the character wear it.  This is probably what sword_update
//was originally forced to do, but since in several places you can get away with less
//work (IE only the work done in sword_update and not here), I chose to make this a 
//separate function
void sword_reequip (CHAR_DATA * ch)
{
	OBJ_DATA *sword;
	int i;
	sword = get_obj_wear (ch, "reaver sword");
	if (!sword)
	{
		bug ("REAVER SWORD REMOVE BUG",0);
		return;
	}
	unequip_char (ch, sword);
	obj_from_char (sword);
	extract_obj (sword);
	reset_char (ch);
	sword = create_object (get_obj_index (OBJ_VNUM_REAVER_SWORD), 0);
	clone_object (ch->sword, sword);
	for (i = 0; i < MAX_BLADE_SPELLS; i++)
	{
		sword->bs_capacity[i] = ch->sword->bs_capacity[i];
		sword->bs_charges[i] = ch->sword->bs_charges[i];
	}
	obj_to_char (sword, ch);
	switch (ch->pcdata->primary_hand)
	{
		case HAND_LEFT:
			equip_char (ch, sword, WEAR_WIELD_L);
			break;
		default:
			equip_char (ch, sword, WEAR_WIELD_R);
	}
	save_char_obj (ch);
}



void update_sword (CHAR_DATA * ch, OBJ_DATA * sword)
{
	OBJ_DATA *blade;
	int i;

	if (ch->pcdata->primary_hand == HAND_LEFT)
	{
		blade = get_eq_char (ch, WEAR_WIELD_L);
	}
	else
	{
		blade = get_eq_char (ch, WEAR_WIELD_R);
	}

	if (blade == NULL)
		return;

	//5-10-03 Iblis - I believe this to be the line that led to the famount
	//damroll bug, so I'm leaving it in for posterity.  The bug actually is more
	//with clone_object not removing affects from the object you are clonning to.
	//clone_object(sword, blade);

	for (i = 0; i < MAX_BLADE_SPELLS; i++)
	{
		blade->bs_charges[i] = ch->sword->bs_charges[i];
		blade->bs_capacity[i] = ch->sword->bs_capacity[i];
	}
	for (i = 0; i < MAX_BURST; i++)
	{
		blade->bb_charges[i] = ch->sword->bb_charges[i];
		blade->bb_capacity[i] = ch->sword->bb_capacity[i];
	}

	save_char_obj (ch);
}

void do_invoke (CHAR_DATA * ch, char *argument)
{
	OBJ_DATA *sword;
	int skill;
	int i, weapon_number = 1, count = 0;
	short bs;
	char buf[MAX_STRING_LENGTH];
	bool burst = FALSE;

	if (!IS_CLASS (ch, PC_CLASS_REAVER) || IS_NPC (ch))
	{
		send_to_char("Only reavers can practice this art.\n\r", ch);
		return;
	}
	// Iblis 9/13/04 - No Magic flag
	if (IS_SET (ch->in_room->room_flags2, ROOM_NOMAGIC) && !IS_IMMORTAL (ch))
	{
		send_to_char("There are no magical forces to draw power from.\n\r",ch);
		return;
	}


	if (ch->pcdata->primary_hand == HAND_LEFT)
	{
		sword = get_eq_char (ch, WEAR_WIELD_L);
	}
	else
	{
		sword = get_eq_char (ch, WEAR_WIELD_R);
	}

	if (sword == NULL)
	{
		send_to_char ("You do not possess your sword.\n\r", ch);
		return;
	}

	if (ch->alignment > -1)
	{
		send_to_char("You may not invoke the powers of evil when you yourself are not evil.\n\r", ch);
		return;
	}

	if ((skill = get_skill (ch, gsn_invocation)) < 1)
	{
		send_to_char("You are not practiced in that art.\n\r", ch);
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char("Invoke which blade spell?\n\r", ch);
		return;
	}
	//5-10-30 Iblis - Reavers sometimes have a problem with actually have the burst/blade spells on their blade, but
	//needing an update in order to properly allow them to use them.
	update_sword(ch, ch->sword);

	if (ch->pcdata->reaver_chant != -1)
	{
		send_to_char("You can only invoke one thing at a time.\n\r", ch);
		return;
	}

	if (!str_cmp(argument, "shocking"))
	{
		bs = BLADE_SPELL_SHOCKING;
	}
	else if (!str_cmp(argument, "frost"))
	{
		bs = BLADE_SPELL_FROST;
	}
	else if (!str_cmp(argument, "flaming"))
	{
		bs = BLADE_SPELL_FLAMING;
	}
	else if (!str_cmp(argument, "chaotic"))
	{
		bs = BLADE_SPELL_CHAOTIC;
	}
	else if (!str_cmp(argument, "venomous"))
	{
		bs = BLADE_SPELL_VENEMOUS;
	}
	else if (!str_cmp(argument, "vampiric"))
	{
		bs = BLADE_SPELL_VAMPIRIC;
	}
	else if (!str_cmp(argument, "apathy"))
	{
		bs = BLADE_SPELL_APATHY;
	}
	else if (!str_cmp(argument, "lightning"))
	{
		burst = TRUE;
		bs = BURST_LIGHTNING;
	}
	else if (!str_cmp(argument, "ice"))
	{
		burst = TRUE;
		bs = BURST_ICE;
	}
	else if (!str_cmp(argument, "fire"))
	{
		burst = TRUE;
		bs = BURST_FIRE;
	}
	else if (!str_cmp(argument, "unholy"))
	{
		burst = TRUE;
		bs = BURST_UNHOLY;
	}
	else if (!str_cmp(argument, "hellscape"))
	{
		burst = TRUE;
		bs = BURST_HELLSCAPE;
	}
	else
	{
		send_to_char("You know no blade spell by that name.\n\r", ch);
		return;
	}

	if (burst && ch->mana < skill_table[gsn_invocation].min_mana)
	{
		send_to_char("You don't have enough mana.\n\r", ch);
		return;
	}
	else if (!burst && ch->mana < 25)
	{
		send_to_char("You don't have enough mana.\n\r", ch);
		return;
	}


	if (!burst)
	{
		if (sword->bs_charges[bs] < 1)
		{
			sprintf (buf, "You have no charges of %s blade stored in %s.\n\r",
				get_bs_name (bs), sword->short_descr);
			send_to_char (buf, ch);
			return;
		}
	}
	else
	{
		if (sword->bb_charges[bs] < 1)
		{
			sprintf (buf, "You have no charges of %s stored in %s.\n\r",
				get_bb_name (bs), sword->short_descr);
			send_to_char (buf, ch);
			return;
		}
	}

	for (i = 1; i <= REAVER_WEAPON_MAX; i++, weapon_number *= 2)
		if (IS_WEAPON_STAT (sword, weapon_number))
			count++;

	if (count >= 2 && !burst)
	{
		send_to_char("You can have up to two active blades spells at a time.\n\r", ch);
		return;
	}



	act("You raise $p into the air, and begin the arcane ritual of invocation.", ch, sword, NULL, TO_CHAR);
	act("$n raises $p into the air, and begins the arcane ritual of invocation.", ch, sword, NULL, TO_ROOM);

	/* chant to do after wait */
	ch->pcdata->reaver_chant = bs;

	/* seconds to wait */
	ch->pcdata->chant_wait = 3;

	if (burst)
		ch->pcdata->reaver_chant -= 100;
	//IBLIS 6/20/03 - Such an unbelievably cheap fix I can't believe I'm putting it in,
	//but its easier than debuggin and finding out why invoke makes reavers pos = standing
	if (ch->mount != NULL)
		ch->position = POS_MOUNTED;
}

void perform_chant(CHAR_DATA * ch)
{

	OBJ_DATA* sword;
	int percent, 
		skill;
	AFFECT_DATA af;
	short bs = ch->pcdata->reaver_chant;
	bool burst = TRUE;
	short gsn = 0;
	int bitvector = 0;

	if( !IS_CLASS(ch, PC_CLASS_REAVER) || IS_NPC(ch) )
	{
		send_to_char("Only reavers can practice this art.\n\r", ch);
		return;
	}


	if(ch->pcdata->primary_hand == HAND_LEFT)
	{
		sword = get_eq_char(ch, WEAR_WIELD_L);
	}
	else
	{
		sword = get_eq_char(ch, WEAR_WIELD_R);
	}

	if (sword == NULL)
	{
		send_to_char ("You do not possess your sword.\n\r", ch);
		return;
	}

	if (bs < 0)
		bs += 100;
	else
		burst = FALSE;

	skill = get_skill(ch, gsn_invocation);

	if( (percent = number_percent()) < skill )
	{
		char buf[20];

		if(!burst)
		{
			af.where = TO_WEAPON;
			af.level = ch->level;
			af.duration = (int) ((ch->level / 2) * (ch->alignment / -1000.0));
			if(af.duration < 1)
				af.duration = 1;
			af.location = 0;
			af.modifier = 0;
			af.permaff = FALSE;
			af.composition = FALSE;
			af.comp_name = str_dup ("");

			sword->bs_charges[bs]--;
			ch->sword->bs_charges[bs]--;	/* ugly but effective */


			switch(bs)
			{
				case BLADE_SPELL_SHOCKING:
					{	
						if (IS_WEAPON_STAT (sword, WEAPON_SHOCKING))
						{
							act ("$p already has an electrical fury.", ch, sword, NULL, TO_CHAR);
							return;
						}
						gsn = skill_lookup ("shocking grasp");
						bitvector = WEAPON_SHOCKING;
						act ("$n invokes an electrical fury upon $p!", ch, sword, NULL, TO_ROOM);
						act ("You successfully invoke an electrical fury upon $p!",	ch, sword, NULL, TO_CHAR);
					}
					break;

				case BLADE_SPELL_FROST:
					{
						if (IS_WEAPON_STAT (sword, WEAPON_FROST))
						{
							act ("$p already has a deadly chill.", ch, sword, NULL, TO_CHAR);
							return;
						}
						gsn = skill_lookup ("chill touch");
						bitvector = WEAPON_FROST;

						act ("$n invokes a deadly chill upon $p!", ch, sword, NULL, TO_ROOM);
						act ("You successfully invoke a deadly chill upon $p!", ch, sword, NULL, TO_CHAR);
					}
					break;

				case BLADE_SPELL_FLAMING:
					{
						if (IS_WEAPON_STAT (sword, WEAPON_FLAMING))
						{
							act ("$p already has an fiery blaze.", ch, sword, NULL, TO_CHAR);
							return;
						}
						gsn = skill_lookup ("fire breath");
						bitvector = WEAPON_FLAMING;

						act ("$n invokes a fiery blaze upon $p!", ch, sword, NULL, TO_ROOM);
						act ("You successfully invoke a fiery blaze upon $p!", ch, sword, NULL, TO_CHAR);
					}
					break;

				case BLADE_SPELL_CHAOTIC:
					{
						if (IS_WEAPON_STAT (sword, WEAPON_SHARP))
						{
							act ("$p already has a chaotic shine.", ch, sword, NULL, TO_CHAR);
							return;
						}
						gsn = 1;
						bitvector = WEAPON_SHARP;

						act ("$n invokes a chaotic shine upon $p!", ch, sword, NULL, TO_ROOM);
						act ("You successfully invoke a sharpened gleam upon $p!", ch, sword, NULL, TO_CHAR);
					}
					break;

				case BLADE_SPELL_VENEMOUS:
					{
						if (IS_WEAPON_STAT (sword, WEAPON_POISON))
						{
							act ("$p already has a poisonous coating.", ch, sword, NULL, TO_CHAR);
							return;
						}
						gsn = gsn_poison;
						bitvector = WEAPON_POISON;

						act ("$n invokes a poisonous coating upon $p!", ch, sword, NULL, TO_ROOM);
						act ("You successfully invoke a venomous sheath upon $p!", ch, sword, NULL, TO_CHAR);
					}
					break;

				case BLADE_SPELL_VAMPIRIC:
					{
						if (IS_WEAPON_STAT (sword, WEAPON_VAMPIRIC))
						{
							act ("$p already has a life leeching spell.", ch, sword, NULL, TO_CHAR);
							return;
						}
						gsn = 1;
						bitvector = WEAPON_VAMPIRIC;

						act ("$n invokes a life leeching spell upon $p!", ch, sword, NULL, TO_ROOM);
						act ("You successfully invoke vampiric blade upon $p!", ch, sword, NULL, TO_CHAR);
					}
					break;

				case BLADE_SPELL_APATHY:
				{
					if (IS_WEAPON_STAT (sword, WEAPON_APATHY))
					{
						act ("$p already has an alignment nullifying spell.", ch, sword, NULL, TO_CHAR);
						return;
					}
					gsn = 1;
					bitvector = WEAPON_APATHY;
					act ("$n invokes an alignment nullifying spell upon $p!", ch, sword, NULL, TO_ROOM);
					act ("You successfully invoke an alignment nullifying spell upon $p!",ch,sword,NULL,TO_CHAR);
				}
				break;
			}


			af.type = gsn;
			af.bitvector = bitvector;
			affect_to_obj (sword, &af);
		}
		else
		{
			invoke_burst(ch, sword, bs);
			sword->bb_charges[bs]--;
			ch->sword->bb_charges[bs]--;
		}

		sprintf(buf, "%d", (burst) ? bs+100 : bs);
		trip_triggers_arg(ch, OBJ_TRIG_CHAR_INVOKES, NULL, ch, OT_SPEC_NONE, buf);

		check_improve (ch, gsn_invocation, TRUE, 4);
		return;
	}
	else
	{
		send_to_char("You fail to empower your blade into an elemental wrath.\n\r", ch);
		if (burst)
			ch->mana -= skill_table[gsn_invocation].min_mana / 2;
		else ch->mana -= 25;
		check_improve (ch, gsn_invocation, FALSE, 4);
		WAIT_STATE (ch, skill_table[gsn_invocation].beats);

		//5-11-03 Iblis - Added a check to see if the spell missed was a burst so the right charges
		//could be subtracted.  Previously, all missed burst spells were subtracting from the blade
		//charges array.
		if (!burst)
		{
			sword->bs_charges[bs]--;
			ch->sword->bs_charges[bs]--;
		}
		else
		{
			sword->bb_charges[bs]--;
			ch->sword->bb_charges[bs]--;
		}
		return;
	}
}


void invoke_burst (CHAR_DATA * ch, OBJ_DATA * sword, int type)
{
	switch (type)
	{
		case BURST_LIGHTNING:
			invoke_lightning_burst (ch, sword);
			break;
		case BURST_ICE:
			invoke_ice_burst (ch, sword);
			break;
		case BURST_FIRE:
			invoke_fire_burst (ch, sword);
			break;
		case BURST_UNHOLY:
			invoke_unholy_burst (ch, sword);
			break;
		case BURST_HELLSCAPE:
			invoke_hellscape_burst (ch, sword);
			break;
	}
	return;
}

void invoke_lightning_burst (CHAR_DATA * ch, OBJ_DATA * sword)
{
	CHAR_DATA *vch, *vch_next;
	int dam, gsn;

	act ("Bolts of lightning shoot forth from $p in an electric fury!.", ch, sword, NULL, TO_CHAR);
	act ("$n unleashes a furious electric storm from $p!", ch, sword, NULL, TO_ROOM);
	for (vch = ch->in_room->people; vch; vch = vch_next)
	{
		vch_next = vch->next_in_room;
		if (vch != ch)
		{
			if (IS_IMMORTAL (vch) || is_safe_spell (ch, vch, TRUE))
				continue;
			if (vch->alignment < 0)
			{
				act ("$N is unaffected by $n's burst of lightning.", ch, NULL, vch, TO_NOTVICT);
				act ("You are unaffected by $n's burst of lightning", ch, NULL, vch, TO_VICT);
				act ("$N is unaffected by your burst of lightning!", ch, NULL, vch, TO_CHAR);
				continue;
			}
			act ("$n unleashes a burst of lightning at $N.", ch, NULL, vch, TO_NOTVICT);
			act ("$n unleashes a burst of lightning at you!", ch, NULL, vch, TO_VICT);
			act ("You unleash a burst of lightning at $N.", ch, NULL, vch, TO_CHAR);
			//Iblis 5/13/03 - Scaled Lightning Burst to use alignment
			dam = (abs (ch->alignment - vch->alignment) / 20);
			//Iblis 5/13/03 - To get around the darn damage reducer in fight.c
			if (dam > 35)
				dam = dam * 2 - 35;
			if (dam > 80)
				dam = dam * 2 - 80;
			//Iblis 5/13/03 - Changed the save calculation to occur based on the character's level, not
			//the level of the chant
			if (saves_spell (ch->level, vch, DAM_LIGHTNING))
			{
				shock_effect (vch, ch->level / 2, dam / 4, TARGET_CHAR);
				damage_old (ch, vch, dam / 2, skill_lookup ("lightning breath"), DAM_LIGHTNING,	TRUE);
			}
			else
			{
				shock_effect (vch, ch->level, dam, TARGET_CHAR);
				damage_old (ch, vch, dam, skill_lookup ("lightning breath"), DAM_LIGHTNING, TRUE);
			}
		}
	}

	gsn = skill_lookup("lightning burst");
	ch->mana -= skill_table[gsn].min_mana / 10;

	return;

}

void invoke_ice_burst (CHAR_DATA * ch, OBJ_DATA * sword)
{
	CHAR_DATA *vch, *vch_next;
	int dam, gsn;

	act ("A furious blizzard spews forth from $p!.", ch, sword, NULL, TO_CHAR);
	act ("$n unleashes a icy blizzard from $p!", ch, sword, NULL, TO_ROOM);
	for (vch = ch->in_room->people; vch; vch = vch_next)
	{
		vch_next = vch->next_in_room;
		if (vch != ch)
		{
			if (IS_IMMORTAL (vch) || is_safe_spell (ch, vch, TRUE))
				continue;
			if (vch->alignment < 0)
			{
				act ("$N is unaffected by $n's burst of ice.", ch, NULL, vch, TO_NOTVICT);
				act ("You are unaffected by $n's burst of ice", ch, NULL, vch, TO_VICT);
				act ("$N is unaffected by your burst of ice!", ch, NULL, vch, TO_CHAR);
				continue;
			}
			act ("$n unleashes a burst of ice at $N.", ch, NULL, vch, TO_NOTVICT);
			act ("$n unleashes a burst of ice at you!", ch, NULL, vch, TO_VICT);
			act ("You unleash a burst of ice at $N.", ch, NULL, vch, TO_CHAR);

			//Iblis 5/13/03 - Scaled Frost Burst to use alignment
			dam = ((abs (ch->alignment - vch->alignment) / 20) * 2);
			//Iblis 5/13/03 - To get around the darn damage reducer in fight.c
			if (dam > 35)
				dam = dam * 2 - 35;
			if (dam > 80)
				dam = dam * 2 - 80;
			//Iblis 5/13/03 - Changed the save calculation to occur based on the character's level, not
			//the level of the chant
			if (saves_spell (ch->level, vch, DAM_COLD))
			{
				cold_effect (vch, ch->level / 2, dam / 4, TARGET_CHAR);
				damage_old (ch, vch, dam / 2, skill_lookup ("frost breath"), DAM_COLD, TRUE);
			}
			else
			{
				cold_effect (vch, ch->level, dam, TARGET_CHAR);
				damage_old (ch, vch, dam, skill_lookup ("frost breath"), DAM_COLD, TRUE);
			}
		}
	}

	gsn = skill_lookup("ice burst");
	ch->mana -= skill_table[gsn].min_mana / 10;

	return;

}

void invoke_fire_burst (CHAR_DATA * ch, OBJ_DATA * sword)
{
	CHAR_DATA *vch, *vch_next;
	int dam, gsn;

	act ("Searing flames shoot forth from $p creating a blazing inferno!.", ch, sword, NULL, TO_CHAR);
	act ("$n unleashes a blazing inferno from $p!", ch, sword, NULL, TO_ROOM);
	for (vch = ch->in_room->people; vch; vch = vch_next)
	{
		vch_next = vch->next_in_room;
		if (vch != ch)
		{
			if (IS_IMMORTAL (vch) || is_safe_spell (ch, vch, TRUE))
				continue;
			if (vch->alignment < 0)
			{
				act ("$N is unaffected by $n's burst of fire.", ch, NULL, vch, TO_NOTVICT);
				act ("You are unaffected by $n's burst of fire", ch, NULL, vch, TO_VICT);
				act ("$N is unaffected by your burst of fire!", ch, NULL, vch, TO_CHAR);
				continue;
			}
			act ("$n unleashes a burst of fire at $N.", ch, NULL, vch, TO_NOTVICT);
			act ("$n unleashes a burst of fire at you!", ch, NULL, vch, TO_VICT);
			act ("You unleash a burst of fire at $N.", ch, NULL, vch, TO_CHAR);

			//Iblis 5/13/03 - Scaled Fire Burst to use alignment
			dam = ((abs (ch->alignment - vch->alignment) / 20) * 3);
			//Iblis 5/13/03 - To get around the darn damage reducer in fight.c
			if (dam > 35)
				dam = (dam - 35) * 2 + 35;
			if (dam > 80)
				dam = (dam - 80) * 2 + 80;
			//Iblis 5/13/03 - Changed the save calculation to occur based on the character's level, not
			//the level of the chant
			if (saves_spell (ch->level, vch, DAM_FIRE))
			{
				fire_effect (vch, ch->level / 2, dam / 4, TARGET_CHAR);
				damage_old (ch, vch, dam / 2, skill_lookup ("fire breath"), DAM_LIGHTNING, TRUE);
			}
			else
			{
				fire_effect (vch, ch->level, dam, TARGET_CHAR);
				damage_old (ch, vch, dam, skill_lookup ("fire breath"), DAM_LIGHTNING, TRUE);
			}
		}
	}

	gsn = skill_lookup("fire burst");
	ch->mana -= skill_table[gsn].min_mana / 10;

	return;

}

void invoke_unholy_burst (CHAR_DATA * ch, OBJ_DATA * sword)
{
	CHAR_DATA *vch, *vch_next;
	int dam, sn;

	act ("Unholy wrath crackles within $p and is unleashed!.", ch, sword, NULL, TO_CHAR);
	act ("$n unleashes an unholy wrath from $p!", ch, sword, NULL, TO_ROOM);
	for (vch = ch->in_room->people; vch; vch = vch_next)
	{
		vch_next = vch->next_in_room;
		if (vch != ch)
		{
			if (IS_IMMORTAL (vch) || is_safe_spell (ch, vch, TRUE))
				continue;
			if (vch->alignment < 0)
			{
				act ("$N is unaffected by $n's unholy wrath.", ch, NULL, vch, TO_NOTVICT);
				act ("You are unaffected by $n's unholy wrath.", ch, NULL, vch, TO_VICT);
				act ("$N is unaffected by your unholy wrath!", ch, NULL, vch, TO_CHAR);
				continue;
			}
			act ("$n unleashes a burst of unholy wrath at $N.", ch, NULL, vch, TO_NOTVICT);
			act ("$n unleashes a burst of unholy wrath at you!", ch, NULL, vch, TO_VICT);
			act ("You unleash a burst of unholy wrath at $N.", ch, NULL, vch, TO_CHAR);

			sn = skill_lookup ("unholy burst");

			//Iblis 5/13/03 - Scaled Unholy Burst to use alignment
			dam = (abs (ch->alignment - vch->alignment) / 4);
			//Iblis 5/13/03 - To get around the darn damage reducer in fight.c
			if (dam > 35)
				dam = (dam - 35) * 2 + 35;
			if (dam > 80)
				dam = (dam - 80) * 2 + 80;

			//Iblis 5/13/03 - Changed the save calculation to occur based on the character's level, not
			//the level of the chant
			if (saves_spell (ch->level, vch, DAM_OTHER))
			{
				damage (ch, vch, dam / 2, sn, DAM_OTHER, TRUE);
			}
			else
			{
				damage (ch, vch, dam, sn, DAM_OTHER, TRUE);
			}
		}
	}

	ch->mana -= skill_table[sn].min_mana / 10;

	return;

}


void invoke_hellscape_burst (CHAR_DATA * ch, OBJ_DATA * sword)
{
	CHAR_DATA *vch, *vch_next;
	int dam, sn;

	act ("All hell breaks loose.\n\r", ch, sword, NULL, TO_CHAR);
	act ("Hell hath no fury like $n!\n\r", ch, sword, NULL, TO_ROOM);

	for (vch = ch->in_room->people; vch; vch = vch_next)
	{
		vch_next = vch->next_in_room;
		if (vch != ch)
		{

			dam = abs (ch->alignment - vch->alignment);
			dam = dam / 2;
			if (IS_IMMORTAL (vch) || is_safe_spell (ch, vch, TRUE))	// Don't hurt imms
				continue;
			if (vch->alignment < 0)	// Don't hurt evil folks
			{
				act ("$n's explosion of evil hellfire has no effect on $N.", ch, NULL, vch, TO_NOTVICT);
				act ("$n's explosion of evil hellfire has no effect on you.", ch, NULL, vch, TO_VICT);
				act ("Your explosion of evil hellfire has no effect on $N.", ch, NULL, vch, TO_CHAR);
				continue;
			}

			sn = skill_lookup ("hellscape");

			//Iblis 5/13/03 - Changed the save calculation to occur based on the character's level, not
			//the level of the chant
			if (saves_spell (ch->level, vch, DAM_FIRE))
			{
				damage (ch, vch, dam / 2, sn, DAM_FIRE, TRUE);
			}
			else
			{
				damage (ch, vch, dam, sn, DAM_FIRE, TRUE);
			}
		}
	}

	ch->mana -= (ch->mana / 2);
	return;
}







//Iblis - Minax suggestion for a new reaver skill
void do_repulse (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	//if (IS_NPC(ch))
	//              return;


	if (get_skill (ch, gsn_repulse) < 1)
	{
		send_to_char ("No one is repulsed by you.\r\n", ch);
		return;
	}
	one_argument (argument, arg);

	if (ch->position < POS_STANDING)
	{
		send_to_char ("You can't repulse anyone without standing.\r\n", ch);
		return;
	}

	if (ch->fighting != NULL)
	{
		send_to_char ("You're too busy to repulse.\r\n", ch);
		return;
	}

	if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	if (!IS_NPC (victim))
	{
		send_to_char("Players aren't repulsed by you.  They've seen much, much worse.\r\n",	ch);
		return;
	}

	if (check_shopkeeper_attack (ch, victim))
		return;


	if (victim->alignment < ch->alignment)
	{
		act ("$N is too evil to be repulsed by you.", ch, NULL, victim,	TO_CHAR);
		return;
	}

	if (victim->alignment > 0)
	{
		act ("$N is too good to be repulsed by you.", ch, NULL, victim,	TO_CHAR);
		return;
	}

	if (ch->alignment >= 0)
	{
		send_to_char ("You're not evil enough to repulse anyone.\r\n", ch);
		return;
	}
	chance = get_skill (ch, gsn_repulse) / 2.0;
	chance += get_curr_stat (ch, STAT_CHA) * 2;
	if (ch->level < victim->level)
		chance -= (victim->level - ch->level) * (victim->level - ch->level);
	if (chance < 0)
		chance = 0;
	if (get_skill (ch, gsn_repulse) >= 99)
		chance += get_curr_stat (ch, STAT_CHA) - 19;

	if (number_percent () < chance)
	{
		victim->alignment = -victim->alignment;
		act ("$n has repulsed $N so much $E has gone good.", ch, NULL, victim, TO_ROOM);
		act ("You repulse $N so much that $E turns good.", ch, NULL, victim, TO_CHAR);
		WAIT_STATE (ch, skill_table[gsn_repulse].beats);
		return;
	}
	else
	{
		act ("$n attempts to repulse $N, but only succeeds in angering $M.", ch, NULL, victim, TO_ROOM);
		act ("You attempt to repulse $N, but only succeed in angering $M.", ch, NULL, victim, TO_CHAR);
		damage (ch, victim, 0, gsn_repulse, 0, FALSE);
		WAIT_STATE (ch, skill_table[gsn_repulse].beats);
		return;
	}
}

//Iblis - Minax suggested new reaver skill
void do_tempt (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	//if (IS_NPC(ch))
	// return;
	if (get_skill (ch, gsn_tempt) < 1)
	{
		send_to_char("You can't even tempt a little girl to take a lollipop.\r\n", ch);
		return;
	}
	one_argument (argument, arg);

	if (ch->position < POS_STANDING)
	{
		send_to_char("You can't tempt anyone unless standing.\r\n", ch);
		return;
	}

	if (ch->fighting != NULL)
	{
		send_to_char("You're too busy to tempt.\r\n", ch);
		return;
	}

	if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
	{
		act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim == ch)
	{
		send_to_char("You try to tempt yourself, but you're just not that tempting.\r\n", ch);
		return;
	}

	if (is_safe (ch, victim))
		return;

	if (IS_CLASS (victim, PC_CLASS_PALADIN) || victim->race == PC_RACE_NERIX)
	{
		send_to_char ("Paladins are beyond your tempting.\r\n", ch);
		return;
	}

	if (victim->alignment < ch->alignment)
	{
		act ("$N is too evil to be tempted by you.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (ch->alignment >= 0)
	{
		send_to_char ("You're not evil enough to tempt anyone.\r\n", ch);
		return;
	}
	chance = get_skill (ch, gsn_repulse) / 2.0;
	chance += get_curr_stat (ch, STAT_CHA) * 2;
	if (ch->level < victim->level)
		chance -= (victim->level - ch->level) * (victim->level - ch->level);
	if (chance < 0)
		chance = 0;
	if (get_skill (ch, gsn_repulse) >= 99)
		chance += get_curr_stat (ch, STAT_CHA) - 19;

	if (number_percent () < chance)
	{
		victim->alignment -= ch->level;
		if (victim->alignment < ch->alignment)
			victim->alignment = ch->alignment;
		act ("$n has tempted $N to grow more evil.", ch, NULL, victim, TO_ROOM);
		act ("You tempt $N to grow more evil.", ch, NULL, victim, TO_CHAR);
		WAIT_STATE (ch, skill_table[gsn_tempt].beats);
		return;
	}
	else
	{
		act ("$n attempts to tempt $N, but only succeeds in angering $M.", ch, NULL, victim, TO_ROOM);
		act ("You attempt to tempt $N, but only succeed in angering $M.", ch, NULL, victim, TO_CHAR);
		damage (ch, victim, 0, gsn_repulse, 0, FALSE);
		WAIT_STATE (ch, skill_table[gsn_tempt].beats);
		return;
	}
}

//Iblis - aunction to back calculate the total souls for reavers that began collecting them before
//showstats total souls was in.  This is run once per old reaver, then the total souls variable is
//correct and showstats works of that
void backcalc_totalsouls(CHAR_DATA *ch)
{
	unsigned long 
		sls=0;
	int 
		cost,
		mod,
		i;
	AFFECT_DATA 
		*paf;

	sls = ch->pcdata->souls;

	if (ch->sword)
	{
		cost = 8;                 /*initial value */
		// sls += cost;
		for (i = 1; i < ch->sword->value[2]; i++)
		{
			sls += cost;
			cost *= 2;
		}
		cost = 16;
		for (i = 1; i < ch->sword->value[1]; i++)
		{
			sls += cost;
			cost *= 4;
		}
		paf = get_hit_affect (ch->sword);
		if (paf == NULL)
			mod = 0;
		else
			mod = paf->modifier;
		cost = 8;
		for (i = 0; i < mod; i++)
		{
			sls += cost;
			cost *= 2;
		}
		paf = get_dam_affect (ch->sword);
		if (paf == NULL)
			mod = 0;
		else
			mod = paf->modifier;
		cost = 9;
		for (i = 0; i < mod; i++)
		{
			sls += cost;
			cost *= 3;
		}
		//           case BLADE_SPELL_SHOCKING:
		cost = 100;
		for (i = 0; i < ch->sword->bs_capacity[BLADE_SPELL_SHOCKING]; i++)
		{
			sls += cost;
			cost += 200;
		}
		//            case BLADE_SPELL_FROST:
		cost = 200;
		for (i = 0; i < ch->sword->bs_capacity[BLADE_SPELL_FROST]; i++)
		{
			sls += cost;
			cost += 300;
		}
		//            case BLADE_SPELL_FLAMING:
		cost = 300;
		for (i = 0; i < ch->sword->bs_capacity[BLADE_SPELL_FLAMING]; i++)
		{
			sls += cost;
			cost += 400;
		}
		//            case BLADE_SPELL_CHAOTIC:
		cost = 600;
		for (i = 0; i < ch->sword->bs_capacity[BLADE_SPELL_CHAOTIC]; i++)
		{
			sls += cost;
			cost += 500;
		}
		//            case BLADE_SPELL_VENEMOUS:
		cost = 900;
		for (i = 0; i < ch->sword->bs_capacity[BLADE_SPELL_VENEMOUS]; i++)
		{
			sls += cost;
			cost += 600;
		}
		//            case BLADE_SPELL_VAMPIRIC:
		cost = 1000;
		for (i = 0; i < ch->sword->bs_capacity[BLADE_SPELL_VAMPIRIC]; i++)
		{
			cost += 1000;
			sls += cost;
		}
		//            case BLADE_SPELL_APATHY:
		cost = 1000;
		for (i=0; i < ch->sword->bs_capacity[BLADE_SPELL_APATHY]; i++)
		{
			cost += 6000;
			sls += cost;
		}
		//            case BURST_LIGHTNING:
		cost = 1000;
		for (i = 0; i < ch->sword->bb_capacity[BURST_LIGHTNING]; i++)
		{
			sls += cost;
			cost += 1000;
		}
		//case BURST_ICE:
		cost = 2000;
		for (i = 0; i < ch->sword->bb_capacity[BURST_ICE]; i++)
		{
			sls += cost;
			cost += 2000;
		}
		//case BURST_FIRE:
		cost = 3500;
		for (i = 0; i < ch->sword->bb_capacity[BURST_FIRE]; i++)
		{
			cost += 3500;
			sls += cost;
		}
		//case BURST_UNHOLY:
		cost = 10000;
		for (i = 0; i < ch->sword->bb_capacity[BURST_UNHOLY]; i++)
		{
			sls += cost;
			cost += 1500;
		}
		//case BURST_HELLSCAPE:
		cost = 20000;
		for (i = 0; i < ch->sword->bb_capacity[BURST_HELLSCAPE]; i++)
		{
			sls += cost;
			cost += 2500;
		}
		ch->pcdata->totalsouls = sls;
	}
}

void create_reaver_sword(CHAR_DATA * ch)
{
	OBJ_DATA 
		*sword = NULL;
	char
		*short_descr = NULL,
		*descr = NULL;
	int
		i;

	short_descr = str_dup(ch->sword->short_descr);
	descr = str_dup(ch->sword->description);

	sword = create_object (get_obj_index (OBJ_VNUM_REAVER_SWORD), 0);
	sword->short_descr = short_descr;
	sword->description = descr;
	ch->sword->cost = 0;
	for (i = 0; i < MAX_BLADE_SPELLS; i++)
	{
		ch->sword->bs_capacity[i] = 0;
		ch->sword->bs_charges[i] = 0;
	}
	for (i = 0; i < MAX_BURST; i++)
	{
		ch->sword->bb_capacity[i] = 0;
		ch->sword->bb_charges[i] = 0;
	}

	ch->sword = sword;
}
