#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "merc.h"
#include "interp.h"
#include "Empath.h"
#include "obj_trig.h"
#include "recycle.h"
#include "Utilities.h"

extern char *target_name

/*
 * Empath Command to Remort Player into Empath.
 */
void do_makeempath(CHAR_DATA *imm, char *argument)
{
	CHAR_DATA *player_being_converted;
	player_being_convert = get_char_world(imm, argument);

	// If player is not logged on.
	if (player_being_converted == NULL)
	{
		send_to_char("That player isn't logged in.\n\r", imm);
		return;
	}

	// If imm's trust is not high enough (if imm_level is to low)
	if (get_trust(player_being_converted) >= get_trust(imm))
	{
		send_to_char("You failed.\n\r", imm);
		return;
	}

	// If the player being remorted is not level 90.
	if (player_being_converted->level < 90)
	{
		send_to_char("You are not high enough level to remort.\n\r", player_being_converted);
		send_to_char("That player is not high enough level to remort.\n\r", imm);
		return;
	}

	// If the player is not cleric, deny remort, checks made for full cleric.
	if (!is_Class_tiers(player_being_converted, PC_CLASS_CLERIC, 3))
	{
		send_to_char("You must be a full Cleric to remort into a Empath.\n\r", player_being_converted);
		send_to_char("That player must be a full Cleric to remort into a Empath.\n\r", imm);
		return;
	}

	// If the player is not of a correct race, deny remort.
	if ((player_being_converted->race == PC_RACE_SIDHE) && (player_being_converted->race == PC_RACE_LICH))
	{
		send_to_char("This race cannot remort into an Empath.\n\r", player_being_converted);
		send_to_char("Sidhe and Lich's cannot become an Empath.\n\r", imm);
		return;
	}

	// Start conversion process.

	// Save player's old class before changing into the new one.
	player_being_converted->pcdata->old_Class = -1;
	// Set player's level back down to level 1.
	player_being_converted->level = 1;
	// Set player's title to:
	set_title(player_being_converted, "``the `oEmpath``.");
	// Set player's Experience back to 0.
	player_being_converted->exp = 0;
	// Set player's Total Experience back to 0.
	player_being_converted->pcdata->totalxp = 0;
	// Force player to reroll once and show it on screen.
	do_reroll(player_being_converted, "");

	// Remove all objects player is wearing and place it into their inventory.
	OBJ_DATA *obj, *obj_next;
	for (obj = player_being_converted->carrying; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;

		if (obj->wear_loc != WEAR_NONE)
			unequip_char(player_being_converted, obj);
	}

	// Set player's new Class, 3 checks to make them PURE Empath only.
	player_being_converted->Class = PC_CLASS_EMPATH;
	player_being_converted->Class = PC_CLASS_EMPATH;
	player_being_converted->Class = PC_CLASS_EMPATH;
	// Give all skills/spells related to Class to Player.
	group_add_all(player_being_converted);
	// The next three lines make's it so the player doesn't GAIN any upon remorting.
	player_being_converted->pcdata->hp_gained = 0;
	player_being_converted->pcdata->mana_gained = 0;
	player_being_converted->pcdata->move_gained = 0;
	// Remove all affects currently on the player.
	affect_strip_skills(player_being_converted);
	// Set player's new starting hp/mana/move. (2 checks for each, one to set the actual max, the other to set the current max)
	player_being_converted->max_hit = 100;
	player_being_converted->hit = 100;
	player_being_converted->max_mana = 100;
	player_being_converted->mana = 100;
	player_being_converted->max_move = 100;
	player_being_converted->move = 100;
	// The next three checks will set the perm setting for hp/mana/move for level 1.
	player_being_converted->pcdata->perm_hit = 100;
	player_being_converted->pcdata->perm_mana = 100;
	player_being_converted->pcdata->perm_move = 100;
	// Send the player a greeting for their new remort.
	send_to_char("`oWelcome young Empath.``\n\r", player_being_converted);
	// Send the IMM who did the remort a message saying it worked.
	send_to_char("Successfully converted Empath.``\n\r", imm);
	return;
	// END of makeempath command
}

/*
 * Add Empath based skills/commands below here.
 */

/*
 * Add Empath based spells below here.
 */