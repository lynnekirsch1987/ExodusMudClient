#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "merc.h"
#include "interp.h"
#include "Druid.h"
#include "obj_trig.h"
#include "recycle.h"
#include "Utilities.h"


extern char *target_name;

/*****
 * Druid Specific Commands
 *****/
 
void do_makedruid(CHAR_DATA *imm, char *argument) {
    CHAR_DATA *player_being_converted;
    player_being_converted = get_char_world(imm, argument);

    if (player_being_converted == NULL) {
        send_to_char("That player isn't logged in.\n\r", imm);
        return;
    }

    if (get_trust(player_being_converted) >= get_trust(imm)) {
        send_to_char("You failed.\n\r", imm);
        return;
    }

    if (player_being_converted->level < 90) {
        send_to_char("You are not high enough level to remort.\n\r", player_being_converted);
        send_to_char("That player is not high enough level to remort.\n\r", imm);
        return;
    }

    // Class restrictions
    if (!is_Class_tiers(player_being_converted, PC_CLASS_RANGER, 3)) {
        send_to_char("You must be a full Ranger to remort into a Druid.\n\r", player_being_converted);
        send_to_char("That player must be a full Ranger to remort into a Druid.\n\r", imm);
        return;
    }
    
    // Race restrictions    
    if (!(player_being_converted->race == PC_RACE_HUMAN) && 
            !(player_being_converted->race == PC_RACE_ELF) &&
            !(player_being_converted->race == PC_RACE_SYVIN)) {
        send_to_char("Only Syvin, Elves and Humans can remort into a Druid.\n\r", player_being_converted);
        send_to_char("Only Syvin, Elves and Humans can remort into a Druid.\n\r", imm);
        return;
    }

    player_being_converted->pcdata->old_Class = -1;
    player_being_converted->level = 1;
    set_title (player_being_converted, "``the `dDruid``.");
    player_being_converted->exp = 0;
    player_being_converted->pcdata->totalxp = 0;
    do_reroll(player_being_converted, "");
    
    OBJ_DATA *obj, *obj_next;
    for (obj = player_being_converted->carrying; obj != NULL; obj = obj_next) {
        obj_next = obj->next_content;

        if (obj->wear_loc != WEAR_NONE)
            unequip_char(player_being_converted, obj);
    }
    
    player_being_converted->Class = PC_CLASS_DRUID;
    player_being_converted->Class = PC_CLASS_DRUID;
    player_being_converted->Class = PC_CLASS_DRUID;
    group_add_all(player_being_converted);
    player_being_converted->pcdata->hp_gained = 0;
    player_being_converted->pcdata->mana_gained = 0;
    player_being_converted->pcdata->move_gained = 0;
    affect_strip_skills(player_being_converted);
    player_being_converted->max_hit = 100;
    player_being_converted->hit = 100;
    player_being_converted->max_mana = 50;
    player_being_converted->mana = 50;
    player_being_converted->max_move = 100;
    player_being_converted->move = 100;
    player_being_converted->pcdata->perm_hit = 100;
    player_being_converted->pcdata->perm_mana = 50;
    player_being_converted->pcdata->perm_move = 100;
    send_to_char("`dWelcome young druid.``\n\r", player_being_converted);
    send_to_char("Successfully converted druid.``\n\r", imm);
    return;
}

void do_shillelagh(CHAR_DATA *ch, char *argument) {
	OBJ_DATA *obj, *target_tree;
	int found_tree = FALSE;
	obj = char_has_shillelagh(ch);

	if (!IS_CLASS(ch, PC_CLASS_DRUID))
	{
		send_to_char("Only Druids can perform this action.\n\r", ch);
		return;
	}

	if (obj > 1)
	{
		send_to_char("You can't weild anymore staffs!\n\r", ch);
		return;
	}

	for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content) {
		if ((obj->item_type == ITEM_TREE) && !IS_SET(obj->value[4], TREE_VALUE_FALLEN) &&
			!IS_SET(obj->value[4], TREE_VALUE_ROTTEN) && can_see_obj(ch, obj)) {
				found_tree = TRUE;
				target_tree = obj;
		}
	}

	if (!found_tree) {
		send_to_char("There are no living trees here to create your shillelagh from.\n\r", ch);
		return;
	}

	send_to_char("`kYou enter a deep trance and begin the shillelagh creation ritual.``\n\r", ch);
	act("$n enters a deep, trance-like state.", ch, NULL, NULL, TO_ROOM);
	ch->pcdata->shillelagh = 1;
	WAIT_STATE(ch, (PULSE_PER_SECOND * 1));
	create_shillelagh(ch, target_tree);
	return;
}

void create_shillelagh(CHAR_DATA *ch, OBJ_DATA *tree) {
	OBJ_DATA *shillelagh;
	char buf[MAX_STRING_LENGTH];
	AFFECT_DATA *paf;

	int tree_type = tree->value[0] + 1;
	int chance = dice(1,1000);
	int cchance = dice(1,100);

	if (chance == 1)
	{
		shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_SPECIAL), 0);
	}
	else
	{
		switch (tree_type)
		{
			case TREE_TYPE_SLIMEWOOD:
				shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_SLIMEWOOD), 0);
				break;
			case TREE_TYPE_JEWELWOOD:
				shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_JEWELWOOD), 0);
				break;
			case TREE_TYPE_HELLROOT:
				shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_HELLROOT), 0);
				break;
			case TREE_TYPE_BARBVINE:
				shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_BARBVINE), 0);
				break;
			case TREE_TYPE_GREEN_LEECHLEAF:
				shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_LEACHLEAF), 0);
				break;
			case TREE_TYPE_MOONBLOSSOM:
				shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_MOONBLOSSOM), 0);
				break;
			case TREE_TYPE_RISIRIEL:
				shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_RISIRIEL), 0);
				break;
			case TREE_TYPE_IRONWOOD:
				shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_IRONWOOD), 0);
				break;
			default:
					if (cchance > 19)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON1), 0);}
					else if (cchance == 1)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON2), 0);}
					else if (cchance == 2)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON3), 0);}
					else if (cchance == 3)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON4), 0);}
					else if (cchance == 4)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON5), 0);}
					else if (cchance == 5)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON6), 0);}
					else if (cchance == 6)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON7), 0);}
					else if (cchance == 7)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON8), 0);}
					else if (cchance == 8)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON9), 0);}
					else if (cchance == 9)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON10), 0);}
					else if (cchance == 10)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON11), 0);}
					else if (cchance == 11)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON12), 0);}
					else if (cchance == 12)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON13), 0);}
					else if (cchance == 13)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON14), 0);}
					else if (cchance == 14)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON15), 0);}
					else if (cchance == 15)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON16), 0);}
					else if (cchance == 16)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON17), 0);}
					else if (cchance == 17)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON18), 0);}
					else if (cchance == 18)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON19), 0);}
					else if (cchance == 19)
					{shillelagh = create_object(get_obj_index(OBJ_VNUM_SHILLELAGH_COMMON20), 0);}
				break;
		}
	}
	// Set all the statistics/special features of the shillelagh here based on the type of tree, etc.
	shillelagh->plr_owner = str_dup(ch->name);

	shillelagh->value[6] = (int) ch->in_room->vnum;
	obj_to_char(shillelagh, ch);
	switch (ch->pcdata->primary_hand)
	{
		case HAND_LEFT:
			equip_char (ch, shillelagh, WEAR_WIELD_L);
			break;
		default:
			equip_char (ch, shillelagh, WEAR_WIELD_R);
			break;
	}
	save_char_obj (ch);
	return;

}

void do_befriend(CHAR_DATA *ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    char tcbuf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if (get_skill(ch, gsn_animal_friendship) < 1) {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if (ch->mana < 51) {
        send_to_char("You don't have enough mana to befriend an animal.\n\r", ch);
        return;
    }

    if (IS_NPC(ch))
        return;
        
    one_argument(argument, arg);
    
    if (arg[0] == '\0') {
        send_to_char ("What animal did you wish to befriend?\n\r", ch);
        return;
    }
    
    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("That animal is not here.\n\r", ch);
        return;
    }
    
    if (!IS_NPC(victim) || !is_animal(victim)) {
        send_to_char("That is no animal.\n\r", ch);
        return;
    }    
    
    if ((victim->master != NULL) && (victim->master == ch)) {
        sprintf(tcbuf, "You have already befriended %s.", fcapitalize(victim->short_descr));
        act(tcbuf, ch, NULL, victim, TO_CHAR);
        return;
    }

    if (check_shopkeeper_attack(ch, victim))
        return;
        
    if (IS_NPC(victim) && (IS_SET(victim->act, ACT_TRAIN)
        || IS_SET(victim->act, ACT_PRACTICE)
        || IS_SET(victim->act, ACT_IS_HEALER))) {
            send_to_char("`iYou best tell an immortal to change the race of that target!\n\r", ch);
            return;
    }
    
    if (IS_AFFECTED(victim, AFF_CHARM)) {
        if (victim->master == ch)
            send_to_char ("That animal is already your friend.\n\r", ch);

        else
            send_to_char ("That animal is someone else's pet.\n\r", ch);

        return;
    }

    sprintf (tcbuf, "You attempt to befriend %s.", victim->short_descr);
    act (tcbuf, ch, NULL, victim, TO_CHAR);
    sprintf (tcbuf, "$n attempts to befriend %s.", victim->short_descr);
    act (tcbuf, ch, NULL, victim, TO_ROOM);

    if (get_skill(ch, gsn_animal_friendship) < 10) {
        if (number_percent() < 75) {
            sprintf(tcbuf, "%s doesn't seem to like you very much.", fcapitalize(victim->short_descr));
            act(tcbuf, ch, NULL, victim, TO_CHAR);
            sprintf(tcbuf, "%s doesn't seem to like $n all that well.", fcapitalize(victim->short_descr));
            act(tcbuf, ch, NULL, victim, TO_ROOM);
            
            if (number_percent() < 50) {
                int attempt;
                for (attempt = 0; attempt < 6; attempt++) {
                    EXIT_DATA *pexit;
                    int door;
                    door = number_door();
                    if (((pexit = victim->in_room->exit[door]) == 0)
                        || (pexit->u1.to_room == NULL)
                        || IS_SET(pexit->exit_info, EX_CLOSED) || (number_range(0, victim->daze) != 0)
                        || (IS_NPC(ch) && (IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) || is_campsite(pexit->u1.to_room))
                        && !IS_SET(victim->act, ACT_SMART_HUNT)))
                        continue;
                    move_char(victim, door, TRUE);
                }
            }

            return;
        } else {
            sprintf(tcbuf, "%s is not at all friendly and attacks!", capitalize(victim->short_descr));
            act(tcbuf, victim, NULL, NULL, TO_ROOM);
            multi_hit(victim, ch, TYPE_UNDEFINED);
            return;
        }
    }
    
    if (check_animal_friendship(ch, victim)) {
        REMOVE_BIT(victim->act, ACT_AGGRESSIVE);
        SET_BIT(victim->act, ACT_SENTINEL);
        if (victim->master)
            stop_follower(victim);
        add_follower(victim, ch);
        victim->leader = ch;
        victim->master = ch;
        ch->mana -= 50;
        sprintf(tcbuf, "You befriend %s.\n\r", victim->short_descr);
        send_to_char(tcbuf, ch);
        sprintf(tcbuf, "$n befriends %s.", victim->short_descr);
        act(tcbuf, ch, NULL, NULL, TO_ROOM);
        return;
    } else {
        if (number_percent() < 75) {
            sprintf(tcbuf, "%s doesn't seem to like you very much.", fcapitalize(victim->short_descr));
            act(tcbuf, ch, NULL, victim, TO_CHAR);
            sprintf(tcbuf, "%s doesn't seem to like $n all that well.", fcapitalize(victim->short_descr));
            act(tcbuf, ch, NULL, victim, TO_ROOM);
            
            if (number_percent() < 50) {
                int attempt;
                for (attempt = 0; attempt < 6; attempt++) {
                    EXIT_DATA *pexit;
                    int door;
                    door = number_door();
                    if (((pexit = victim->in_room->exit[door]) == 0)
                        || (pexit->u1.to_room == NULL)
                        || IS_SET(pexit->exit_info, EX_CLOSED) || (number_range(0, victim->daze) != 0)
                        || (IS_NPC(ch) && (IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) || is_campsite(pexit->u1.to_room))
                        && !IS_SET(victim->act, ACT_SMART_HUNT)))
                        continue;
                    move_char(victim, door, TRUE);
                }
            }

            return;
        } else {
            sprintf(tcbuf, "%s is not at all friendly and attacks!",capitalize (victim->short_descr));
            act(tcbuf, victim, NULL, NULL, TO_ROOM);
            multi_hit(victim, ch, TYPE_UNDEFINED);
            return;
        }
    }
}

void do_shapechange(CHAR_DATA *ch, char *argument) {
    char command[MAX_INPUT_LENGTH];
    
    if (ch->in_room == NULL)
        return;

    if (ch->mana > 500) {
        if (is_char_in_natural_sector(ch)) {
            if (check_shapechange(ch)) {
                argument = one_argument(argument, command);
                bool not_enough_wis = FALSE;
                short form = 0;
                int i;                
                for(i = 0; command[i]; i++) {
                    command[i] = tolower(command[i]);       
                };     
                
                if (strcmp(command, "mouse") == 0) {
                    form = 1;
                } else if (strcmp(command, "raven") == 0) {
                    form = 2;
                } else if (strcmp(command, "wolf") == 0) {
                    form = 3;
                } else if (strcmp(command, "bear") == 0) {
                    form = 4;
                } else {
                    send_to_char("`cWhat animal form did you want to take?``\n\r", ch);
                    return;
                }

                switch (form) {
                    case(1):
                        if ((ch->perm_stat[STAT_WIS] + ch->mod_stat[STAT_WIS]) >= MIN_WIS_SHAPECHANGE_MOUSE) {
                            ch->mana /= 2;
                            ch->hit /= 2;
                            ch->move /= 2;
                            send_to_char("`cYou draw on your natural surroundings and begin to take animal form.``\n\r", ch);            
                            send_to_char("`aYour body starts to shrink in size and you feel a tail emerging from your backside!``\n\r",ch);
                            act("$n is gone, and only a mouse remains.", ch, NULL, NULL, TO_ROOM);
                            perform_shapechange(ch, MOB_VNUM_SHAPECHANGE_MOUSE);
                        } else {
                            not_enough_wis = TRUE;
                        }
                        
                        break;
                    case(2):
                        if ((ch->perm_stat[STAT_WIS] + ch->mod_stat[STAT_WIS]) >= MIN_WIS_SHAPECHANGE_RAVEN) {
                            ch->mana /= 3;
                            ch->hit /= 3;
                            ch->move /= 3;
                            send_to_char("`cYou draw on your natural surroundings and begin to take animal form.``\n\r", ch);            
                            send_to_char("`aYour skin begins to turn into feathers!``\n\r",ch);
                            act("$n is gone, and only a black raven remains.", ch, NULL, NULL, TO_ROOM);
                            perform_shapechange(ch, MOB_VNUM_SHAPECHANGE_RAVEN);
                        } else {
                            not_enough_wis = TRUE;
                        }
                    case(3):
                        if ((ch->perm_stat[STAT_WIS] + ch->mod_stat[STAT_WIS]) >= MIN_WIS_SHAPECHANGE_WOLF) {
                            ch->mana /= 4;
                            ch->hit /= 4;
                            ch->move /= 4;
                            send_to_char("`cYou draw on your natural surroundings and begin to take animal form.``\n\r", ch);            
                            send_to_char("`aYour teeth ache as large fangs emerge from your gums!``\n\r",ch);
                            act("$n is gone, and only a snarling wolf remains.", ch, NULL, NULL, TO_ROOM);
                            perform_shapechange(ch, MOB_VNUM_SHAPECHANGE_WOLF);
                        } else {
                            not_enough_wis = TRUE;
                        }
                        
                        break;
                    case(4):
                        if ((ch->perm_stat[STAT_WIS] + ch->mod_stat[STAT_WIS]) >= MIN_WIS_SHAPECHANGE_BEAR) {
                            ch->mana = 1;
                            ch->hit = 1;
                            ch->move = 1;
                            send_to_char("`cYou draw on your natural surroundings and begin to take animal form.``\n\r", ch);            
                            send_to_char("`aYour muscles surge with immense strength as thick fur begins to protrude from your skin!``\n\r",ch);
                            act("$n is gone, and only a giant grizzly bear remains.", ch, NULL, NULL, TO_ROOM);
                            perform_shapechange(ch, MOB_VNUM_SHAPECHANGE_BEAR);
                        } else {
                            not_enough_wis = TRUE;
                        }

                        break;
                }
                
                if (not_enough_wis == TRUE) {
                    send_to_char("`aYou lack the wisdom to take that form.``\n\r",ch);
                }                
                
                return;
            } else {
                ch->mana /= 2;
                ch->hit /= 2;
                ch->move /= 2;
                send_to_char("`cYou attempt to draw on your natural surroundings to take animal form, but fail.``\n\r", ch);
                return;
            }
        } else {
            send_to_char("`cYou cannot take animal form here.``\n\r", ch);
            return;
        }    
    } else {
        send_to_char("`cYou don't have enough mana to take animal form.``\n\r", ch);
        return;
    }
}

void perform_shapechange(CHAR_DATA *ch, int shapechange_vnum) {
    CHAR_DATA *mob;
    char buf[MAX_STRING_LENGTH];
    
    if (get_mob_index(shapechange_vnum)) {
        mob = create_mobile(get_mob_index(shapechange_vnum));
    } else {
        send_to_char("`iContact an immortal, that animal doesn't exist.``\n\r", ch);
        return;
    }

    switch (shapechange_vnum) {
        case MOB_VNUM_SHAPECHANGE_MOUSE:
            mob->ticks_remaining = (ch->level / 2);
            mob->max_hit = ch->max_hit / 20;
            break;
        case MOB_VNUM_SHAPECHANGE_RAVEN:
            mob->ticks_remaining = (ch->level / 3);
            mob->max_hit = ch->max_hit / 5;
            break;
        case MOB_VNUM_SHAPECHANGE_WOLF:
            mob->ticks_remaining = (ch->level / 4);
            mob->max_hit = ch->max_hit;
            break;
        case MOB_VNUM_SHAPECHANGE_BEAR:
            mob->max_hit = ch->max_hit * 2;
            mob->ticks_remaining = (ch->level / 5);
            break;
        case MOB_VNUM_SHAPECHANGE_TREE:
            mob->max_hit = ch->max_hit / 10;
            mob->ticks_remaining = (ch->level / 10);
            break;        
    }

    mob->hit = mob->max_hit;    
    char_to_room(mob, ch->in_room);       
    ch->desc->character = mob;
    ch->desc->original = ch;
    mob->desc = ch->desc;

    ch->desc = NULL;

    sprintf(buf, "$N switches into %s", IS_NPC (mob) ? mob->short_descr : mob->name);
    wiznet(buf, ch, NULL, WIZ_SWITCHES, WIZ_SECURE, get_trust(ch));

    // change communications to match
    if (ch->prompt != NULL)
        mob->prompt = str_dup(ch->prompt);

    mob->comm = ch->comm;
    mob->lines = ch->lines;

    char_from_room(ch);
    char_to_room(ch, get_room_index(ROOM_VNUM_SWITCHED_LIMBO));
    SET_BIT(mob->act2, ACT_PUPPET);
    SET_BIT(ch->act2, ACT_SWITCHED);
}

void shapechange_return(CHAR_DATA *ch, int how_returned) {
    CHAR_DATA *original;
    switch(how_returned){
        // 0 - time ran out
        // 1 - revert
        // 2 - death
        case(0):
            act("$n reverts into the form of another being.", ch, NULL, NULL, TO_ROOM);
            send_to_char("`cYour time as an animal has come to an end.``\n\r",ch);


            if (ch->desc && ch->desc->original) {
                char_from_room(ch->desc->original);
                char_to_room(ch->desc->original, ch->in_room);
            }
            
            break;
        case(1):
            act("$n reverts into the form of another being.", ch, NULL, NULL, TO_ROOM);
            send_to_char("`cYou choose to end your time as an animal.``\n\r",ch);
            
            if (ch->desc && ch->desc->original) {
                char_from_room(ch->desc->original);
                char_to_room(ch->desc->original, ch->in_room);
            }

            break;        

        case(2):
            if (ch->desc == NULL)
                return;

            if (ch->desc->original == NULL)
                return;

            if (!IS_SET(ch->desc->original->act2, ACT_SWITCHED))
                return;

            original = ch->desc->original;

            //IF they die while void, I think
            if (original->in_room == NULL)
                original->in_room = get_room_index(ROOM_VNUM_ALTAR);

            act("The carcass of $n slowly transforms into something vastly different.", ch, NULL, NULL, TO_ROOM);
            char_from_room(original);
            char_to_room(original, ch->in_room);
            send_to_char("Your death in this form results in the death of your true form.\n\r", ch);
            raw_kill(original, original);
            do_return(ch, "");
            REMOVE_BIT(original->act2, ACT_SWITCHED);
            
            break;        
    }
}

bool is_druid_follower(CHAR_DATA *ch) {
    if ((ch->master != NULL) && !IS_NPC(ch->master) && (ch->master->Class == PC_CLASS_DRUID) && IS_NPC(ch) && is_animal(ch))
        return TRUE;
        
    return FALSE;
}

bool is_shapechanged_druid(CHAR_DATA *ch) {
    if (!IS_NPC(ch))
        return FALSE;
    
    if ((ch->pIndexData->vnum == MOB_VNUM_SHAPECHANGE_MOUSE) ||
        (ch->pIndexData->vnum == MOB_VNUM_SHAPECHANGE_RAVEN) ||
        (ch->pIndexData->vnum == MOB_VNUM_SHAPECHANGE_WOLF) ||
        (ch->pIndexData->vnum == MOB_VNUM_SHAPECHANGE_BEAR) ||
        (ch->pIndexData->vnum == MOB_VNUM_SHAPECHANGE_TREE))        
            return TRUE;

    return FALSE;
}

bool is_tree_in_room(ROOM_INDEX_DATA *room) {
    OBJ_DATA *obj;
    for (obj = room->contents; obj != NULL; obj = obj->next_content) {    
        if (obj->item_type == ITEM_TREE)
            return TRUE;
    }
    
    return FALSE;
}

OBJ_DATA* char_has_shillelagh(CHAR_DATA *ch) {
    OBJ_DATA *obj;
    for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
        if (obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON1
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON2
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON3
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON4
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON5
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON6
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON7
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON8
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON9
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON10
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON11
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON12
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON13
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON14
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON15
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON16
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON17
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON18
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON19
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON20
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_BARBVINE
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_HELLROOT
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_IRONWOOD 
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_JEWELWOOD 
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_LEACHLEAF
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_MOONBLOSSOM 
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_RISIRIEL
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_SLIMEWOOD
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_SPECIAL
			)
		{
            return obj;
        }
    }
        
    for (obj = ch->on; obj != NULL; obj = obj->next_content) {
        if (obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON1
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON2
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON3
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON4
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON5
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON6
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON7
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON8
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON9
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON10
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON11
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON12
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON13
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON14
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON15
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON16
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON17
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON18
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON19
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_COMMON20
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_BARBVINE
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_HELLROOT
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_IRONWOOD 
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_JEWELWOOD 
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_LEACHLEAF
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_MOONBLOSSOM 
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_RISIRIEL
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_SLIMEWOOD
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_SPECIAL
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_BARBVINE
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_HELLROOT
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_IRONWOOD 
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_JEWELWOOD 
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_LEACHLEAF
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_MOONBLOSSOM 
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_RISIRIEL
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_SLIMEWOOD
			|| obj->pIndexData->vnum == OBJ_VNUM_SHILLELAGH_SPECIAL
			)
		{
			return obj;
        }
    }
    
    return NULL;
}

void update_shillelagh(void) {
    DESCRIPTOR_DATA *d;
    
    for (d = descriptor_list; d != NULL; d = d->next) {
        if ((d->connected != CON_PLAYING) || !d->character || !d->character->pcdata || !(d->character->pcdata->shillelagh > 0))
            continue;

        d->character->pcdata->shillelagh++;
        
        if (d->character->pcdata->shillelagh > 2) {
            d->character->pcdata->shillelagh = 0;
            act ("$n completes the creation ritual of $s shillelagh.", d->character, NULL, NULL, TO_ROOM);
            send_to_char("`kYou successfully create your shillelagh.``\n\r", d->character);
        } else {
            act ("$n draws more energy from a tree into $s shillelagh.", d->character, NULL, NULL, TO_ROOM);
           send_to_char("`kYou draw more energy into your shillelagh.``\n\r", d->character);
        }
    }
}

/*****
 * Druid Skills.
 *****/
 
bool check_nature_sense(CHAR_DATA * ch) {
    if (is_char_in_natural_sector(ch) && (number_percent() < get_skill(ch, gsn_nature_sense))) {
        check_improve(ch, gsn_nature_sense, TRUE, 2);
        return TRUE;
    }
        
    return FALSE;
}

bool check_shillelagh(CHAR_DATA *ch) {    
    if (number_percent() < (get_skill(ch, gsn_nature_sense) - (25 - ch->perm_stat[STAT_CHA])))
        return TRUE;
        
    return FALSE;
}

bool check_animal_empathy(CHAR_DATA *ch, CHAR_DATA *victim) {
    int dice = number_percent();
    int chance = get_skill(ch, gsn_animal_empathy);
    if (victim->level > ch->level)
        chance += ((ch->level - victim->level));
    else 
        chance += ((ch->level - victim->level));
    
    chance -= (25 - (ch->perm_stat[STAT_CHA] + ch->mod_stat[STAT_CHA]));
    
    if (dice > chance)
        return FALSE;
    
    return TRUE;
}

bool check_animal_friendship(CHAR_DATA *ch, CHAR_DATA *victim) {
    int chance = get_skill(ch, gsn_animal_friendship);
    if (victim->level > ch->level)
        chance += ((ch->level - victim->level) * 5);
    
    chance -= (25 - ch->mod_stat[STAT_CHA]);
        
    CHAR_DATA *rch, *rch_next;
    for (rch = ch->in_room->people; rch != NULL; rch = rch_next) {
        rch_next = rch->next_in_room;
        
        if (rch_next != NULL) {
            if (IS_NPC(rch_next) && (rch_next->master != NULL) && (rch->master == ch) && is_animal(rch))
                chance -= 15;
        }
    }
    
    if (number_percent() > chance)
        return FALSE;
    
    return TRUE;
}

bool check_satiate(CHAR_DATA *ch) {
    int chance = get_skill(ch, gsn_satiate);
    if (number_percent() > chance)
        return FALSE;
    
    return TRUE;
}

bool check_wild_invigoration(CHAR_DATA *ch) {
    int chance = get_skill(ch, gsn_wild_invigoration);
    if (number_percent() > chance)
        return FALSE;
    
    return TRUE;
}

bool check_primeval_guidance(CHAR_DATA *ch) {
    int chance = get_skill(ch, gsn_primeval_guidance);
    chance -= (25 - ch->mod_stat[STAT_WIS]);        
    if (number_percent() > chance)
        return FALSE;
    
    return TRUE;
}

bool check_shapechange(CHAR_DATA *ch) {
    int chance = get_skill(ch, gsn_shapechange);    
    chance -= (25 - ch->mod_stat[STAT_WIS]);        
    if (number_percent() > chance)
        return FALSE;
    
    return TRUE;
}

/*****
 * Druid Spells.
 *****/



 void spell_bear_heart(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim != ch)
        return;

    if (is_affected(victim, sn)) {
        send_to_char ("`aYou already have the `iheart `aof a bear!``\n\r", ch);
        return;
    }
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_HIT;
    af.modifier = (level) + 50;
    
    if (af.modifier < 1)
        af.modifier = 1;
    
    af.bitvector = 0;
    af.permaff = FALSE;
    af.composition = FALSE;
    af.comp_name = str_dup("");
    
    if (ch->race == PC_RACE_SYVIN) {
        af.location = APPLY_DAMROLL;
        af.modifier = (level / 10) + 5;
        affect_to_char(victim, &af);        
    }

    if (ch->race == PC_RACE_ELF) {
        af.location = APPLY_HITROLL;
        af.modifier = (level / 10 ) + 5;
        affect_to_char(victim, &af);        
    }

    affect_to_char(victim, &af);

    send_to_char("`aYou call upon the `iheart `aof the bear to increase your physical prowess!``\n\r", victim);
    act("`a$n conjurs a blurred mist resembling a beating `iheart.``", victim, NULL, NULL, TO_ROOM);
    return;
}

void spell_chameleon_shroud(int sn, int level, CHAR_DATA *ch, void *vo, int target) {    
    if (!is_char_in_natural_sector(ch)) {
        send_to_char("A chameleon couldn't even hide itself here.\n\r", ch);
        return;
    }
    
    if (IS_AFFECTED(ch, AFF_CAMOUFLAGE))
        REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
        
    if (number_percent() < get_skill(ch, sn)) {
        send_to_char("`cYou fade into your surroundings like a `dc`ch`da`cm`de`cl`de`co`dn`c.``\n\r", ch);
        SET_BIT(ch->affected_by, AFF_CAMOUFLAGE);
    } else
        send_to_char("`cYou fail to blend into your surroundings.``\n\r", ch);

    return;
}
//Avariel 5/12/09 - Removed the natural sector check.
void spell_neutralize_poison(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
   
    
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    if (!is_affected(victim, gsn_poison)) {
        if (victim == ch)
            send_to_char("`cThere is no poison coursing through your veins.``\n\r", ch);

        else
            act("`c$N is not poisoned.``", ch, NULL, victim, TO_CHAR);
     
        return;
    }

    int chance = get_skill(ch, sn) + (ch->perm_stat[STAT_WIS] - 25);
    
        

		if (check_dispel (level, victim, gsn_poison)) {
            act("`c$N looks much less sick.", victim, NULL, victim, TO_ROOM);
        } else {
            act("`c$N looks much less sick.", victim, NULL, victim, TO_ROOM);
        }

        if (skill_table[sn].msg_off) {
            send_to_char (skill_table[sn].msg_off, victim);
            send_to_char ("\n\r", victim);
        }
    else
        send_to_char("`cAre you sure you gathered the right herbs and berries?``\n\r", ch);

    return;
 }

void spell_tree_stride(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    char tcbuf[15];
    char buf[MAX_STRING_LENGTH];
    bool found_start_tree = FALSE;

    OBJ_DATA *obj, *start_tree;
    for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content) {    
        if ((obj->item_type == ITEM_TREE) && !IS_SET(obj->value[4],TREE_VALUE_FALLEN) && 
            !IS_SET(obj->value[4],TREE_VALUE_ROTTEN) && can_see_obj(ch, obj)) {
            found_start_tree = TRUE;
            start_tree = obj;
            break;
        }
    }
    
    if (!found_start_tree) {
        send_to_char("There are no living trees here.\n\r", ch);
        return;
    }
        
    // Find some trees.
    int counter = 0;
    OBJ_DATA *tree;
    for (tree = object_list; tree != NULL; tree = tree->next) {
        if (tree->item_type == ITEM_TREE) {
            if (!can_see_obj(ch, tree) || (tree->in_room == NULL) ||
                !can_see_room(ch, tree->in_room) || !can_move_char(ch, tree->in_room, FALSE, FALSE) ||
                IS_SET(tree->value[4],TREE_VALUE_FALLEN) || IS_SET(tree->value[4],TREE_VALUE_ROTTEN))
                    continue;

            counter++;
        }
    }

    OBJ_DATA *trees[counter];
    counter = 0;
    for (tree = object_list; tree != NULL; tree = tree->next) {
        if (tree->item_type == ITEM_TREE) {
            if (!can_see_obj(ch, tree) || (tree->in_room == NULL) ||
                !can_see_room(ch, tree->in_room) || !can_move_char(ch, tree->in_room, FALSE, FALSE) ||
                IS_SET(tree->value[4],TREE_VALUE_FALLEN) || IS_SET(tree->value[4],TREE_VALUE_ROTTEN))
                    continue;

            trees[counter] = tree;
            counter++;
        }
    }
    
    int target_tree = dice(1, counter);    
    sprintf(buf, "You are absorbed by %s ``and emerge somewhere else.\n\r", trees[target_tree]->short_descr);
    send_to_char(buf, ch);
    act("$n is absorbed by a tree.", ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, trees[target_tree]->in_room);
    act("$n emerges from a tree.", ch, NULL, NULL, TO_ROOM);
    do_look(ch, "auto");
    trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
    trap_check(ch,"room", ch->in_room, NULL);
    sprintf(tcbuf, "%d", ch->in_room->vnum);
    trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE, tcbuf);
    check_aggression(ch);
    return;
}

void spell_treant(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    CHAR_DATA *treant;
    char buf[MAX_STRING_LENGTH];
    bool found_tree = FALSE;

    OBJ_DATA *obj, *target_tree;
    for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content) {    
        if ((obj->item_type == ITEM_TREE) && !IS_SET(obj->value[4],TREE_VALUE_FALLEN) && 
            !IS_SET(obj->value[4],TREE_VALUE_ROTTEN) && can_see_obj(ch, obj)) {
                found_tree = TRUE;
                target_tree = obj;
                break;
        }
    }

    for (treant = char_list; treant != NULL; treant = treant->next) {
        if (IS_NPC(treant) && treant->pIndexData->vnum == MOB_VNUM_TREANT
            && (str_cmp(treant->afkmsg, ch->name) == 0) && is_same_group(treant, ch)) {
                send_to_char("You already have a treant.\n\r",ch);
                return;
        }
    }
    
    if (!found_tree) {
        send_to_char("There are no living trees here.\n\r", ch);
        return;
    }
    
    int dice = number_percent();
    int chance = get_skill(ch, skill_lookup("treant"));
    chance -= ((25 - ch->perm_stat[STAT_WIS]) * 2);
    chance -= ((25 - (ch->perm_stat[STAT_CHA] + ch->mod_stat[STAT_CHA])) * 2);
    
    
     {
        sprintf(buf, "`cYou watch with pleasure as %s`c stirs and comes to life.``\n\r", target_tree->short_descr);
        send_to_char(buf, ch);
        treant = create_mobile(get_mob_index(MOB_VNUM_TREANT));
        treant->level = ch->level;
        treant->afkmsg = str_dup(ch->name);
        treant->short_descr = str_dup(target_tree->short_descr);
        treant->long_descr = "`cThis tree is alive and seems to be sentient.``\n\r";
        treant->max_hit = ch->max_hit;
        treant->hit = treant->max_hit;
        treant->max_mana = 1;
        treant->mana = 1;
        treant->max_move = ch->max_move;
        treant->move = treant->max_move;
        treant->hitroll = ch->level;
        treant->damroll = (ch->level / 2);
        //treant->ticks_remaining = ch->level / 2;

        // Base the damage of the treant on the character level.
        treant->damage[0] = (ch->level / 9);
        if (treant->damage[0] < 1)
            treant->damage[0] = 1;
        
        treant->damage[1] = 10;
        treant->damage[2] = (ch->level / 3);
        
        // Make these things unique based on the type of tree, the rarer, the better.
        switch (target_tree->value[0] + 1) {
            case TREE_TYPE_SLIMEWOOD: {
                // RARITY_UNCOMMON
                treant->dam_type = find_damage_number_by_noun("slime");
                break;
            } case TREE_TYPE_JEWELWOOD: {
                // RARITY_UNCOMMON
                treant->dam_type = find_damage_number_by_noun("magic");
                break;
            } case TREE_TYPE_HELLROOT: {
                // RARITY_UNCOMMON
                treant->dam_type = find_damage_number_by_noun("flame");
                break;
            } case TREE_TYPE_BARBVINE: {
                // RARITY_UNCOMMON
                treant->dam_type = find_damage_number_by_noun("pierce");
                break;
            } case TREE_TYPE_GREEN_LEECHLEAF: {
                // RARITY_UNCOMMON
                treant->dam_type = find_damage_number_by_noun("suction");
                break;
            } case TREE_TYPE_MOONBLOSSOM: {
                // RARITY_RARE
                treant->dam_type = find_damage_number_by_noun("wrath");
                break;
            } case TREE_TYPE_RISIRIEL: {
                // RARITY_RARE
                treant->dam_type = find_damage_number_by_noun("drain");
                break;
            } case TREE_TYPE_IRONWOOD: {
                // RARITY_RARE
                treant->dam_type = find_damage_number_by_noun("chill");
                break;
			} case TREE_TYPE_ROTSTENCH: {
				// RARITY_COMMON
				treant->dam_type = find_damage_number_by_noun("digestion");
				break;
            } default: {
                // Leave the treant pretty plain jane for the common trees.
                // TREE_TYPE_OAK, TREE_TYPE_MAPLE, TREE_TYPE_BIRCH, TREE_TYPE_PINE
                // TREE_TYPE_WILLOW, TREE_TYPE_ELM, TREE_TYPE_POPLAR, TREE_TYPE_KARRI
                // TREE_TYPE_MARRI, TREE_TYPE_JARRAH, TREE_TYPE_ROTSTENCH
                treant->dam_type = find_damage_number_by_noun("smash");;
                break;
            }
        }
                    
        switch (target_tree->value[2]) {
            case TREE_SAPLING: {
                treant->size = SIZE_TINY;
                break;
            } case TREE_SMALL: {
                treant->size = SIZE_SMALL;
                break;
            } case TREE_MEDIUM: {
                treant->size = SIZE_MEDIUM;
                break;
            } case TREE_LARGE: {
                treant->size = SIZE_LARGE;
                break;
            } case TREE_HUGE: {
                treant->size = SIZE_HUGE;
                break;
            } default: {
                treant->size = SIZE_MEDIUM;
                break;
            }
        }
        
	SET_BIT(treant->affected_by, AFF_CHARM);
        char_to_room(treant, ch->in_room);
        extract_obj(target_tree);
        add_follower(treant, ch);
        treant->leader = ch;
    }

}

void spell_plant_shell(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    if (!is_char_in_natural_sector(ch)) {
        send_to_char("`cYou can only summon forth a shell of plants in a natural place.``\n\r", ch);
        return;
    }

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn)) {
        send_to_char("`cYou are already surrounded by a shell of plants.\n\r", ch);
        return;
    }
    
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = (level / 2);
    af.modifier = -(level / 2);
    
    if (af.duration < 1)
        af.duration = 1;

    if (af.modifier > -1)
        af.modifier = -1;
    
    af.location = APPLY_AC;
    af.bitvector = 0;
    af.permaff = FALSE;
    af.composition = FALSE;
    af.comp_name = str_dup("");
    affect_to_char (victim, &af);

    send_to_char("`cPlants emerge from the ground and encapsulate you in a shell.``\n\r", victim);
    act("`cPlants emerge from the ground and encapsulate $N in a protective shell.``", ch, NULL, victim, TO_ROOM);
    return;
}

void spell_trackless_step(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    if (!is_char_in_natural_sector(ch)) {
        send_to_char("`cYou can only do this in a natural place.``\n\r", ch);
        return;
    }
    
    if (is_affected(ch, sn)) {
        send_to_char("You already have a trackless step.\n\r", ch);
        return;
    }

    AFFECT_DATA af;
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 2;
    af.location = APPLY_NONE;
    af.modifier = 1;
    af.bitvector = 0;
    af.permaff = FALSE;
    af.composition = FALSE;
    af.comp_name = str_dup("");
    affect_to_char(ch, &af);
    send_to_char("`cYou ask the wilds to cover your steps.``\n\r", ch);
    return;
}

void spell_oceanic_wrath(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    if ((ch->in_room) && ((ch->in_room->sector_type == SECT_WATER_SWIM) ||
            (ch->in_room->sector_type == SECT_WATER_NOSWIM) ||
            (ch->in_room->sector_type == SECT_WATER_OCEAN) ||
            (ch->in_room->sector_type == SECT_SHORE))) {
        
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
        send_to_char("`lYou summon forth an enormous wall of water!``\n\r", ch);
        act("$n summons forth an enormous wall of water!", ch, NULL, NULL, TO_ROOM);
        for (vch = char_list; vch != NULL; vch = vch_next) {
            vch_next = vch->next;
            if (vch->in_room == NULL)
                continue;
            if (vch == ch)
                continue;
            if (vch->in_room == ch->in_room) {
                if (!is_safe_spell(ch, vch, FALSE)) {
                    // Scatter them to nearby rooms.
                    int attempt;
                    ROOM_INDEX_DATA *was_in;
                    was_in = vch->in_room;
                    for (attempt = 0; attempt < 6; attempt++) {
                        EXIT_DATA *pexit;
                        int door;
                        door = attempt;
                        if (((pexit = was_in->exit[door]) == 0) || (pexit->u1.to_room == NULL) || 
                            (IS_SET(pexit->exit_info, EX_CLOSED)) || 
                            (IS_NPC(vch) && IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)))
                                continue;
            
                        //act("`lYour wave sends $N sprawling into the water!``", ch, NULL, vch, TO_CHAR);
                        act("`lA huge wave of water sweeps $n away!``",vch, NULL, ch, TO_ROOM);
                        act("`l$N's huge wave of water sweeps you away!``", vch, NULL, ch, TO_CHAR);
                        char_from_room(vch);
                        char_to_room(vch, pexit->u1.to_room);
                        //move_char(vch, door, FALSE);
                        //vch->position = POS_RESTING;
                        break;
                    }
                    
                    int dam;
                    dam = dice(level, 4);
                    damage_old(ch, vch, dam, sn, find_damage_number_by_noun("wrath"), TRUE);
                }
            }
        }

        return;
    } else {
        send_to_char("`lYou must be in or near water to use this spell!``\n\r", ch);
        return;
    }
}

void spell_commune(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer = new_buf();
    CHAR_DATA *victim;
    bool found;
    found = FALSE;
    add_buf(buffer, "Animals near you\n\r");
    add_buf(buffer, "------------------------------------------------------------\n\r");
    for (victim = char_list; victim != NULL; victim = victim->next) {
        if ((victim->in_room != NULL) && can_see(ch, victim) && 
        can_see_room(ch, victim->in_room) && is_animal(victim) && IS_NPC(victim) &&
           (victim->in_room->area == ch->in_room->area)) {
                found = TRUE;
                sprintf(buf, "%s    %s\n\r", victim->in_room->area->name, victim->short_descr);
                add_buf(buffer, buf);
        }
    }
    
    if (!found) {
        act("You didn't find any animals near you.", ch, NULL, NULL, TO_CHAR);
        return;
    } else {
        page_to_char(buf_string(buffer), ch);
    }

    free_buf(buffer);
    return;
}

void spell_spore_eruption(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    CHAR_DATA *victim;
    CHAR_DATA *nextVictim;
    int dam;

    if (is_char_in_natural_sector(ch) && !is_char_in_water_sector(ch)) {        
        act("A thick `dbrown`` cloud of spores erupt from the surrounding fungi!", ch, NULL, NULL, TO_ROOM);
        act("You cause the surrounding fungi to expunge their spores into the air!", ch, NULL, NULL, TO_CHAR);

        dam = dice((ch->level / 9), 6);

        poison_effect(ch->in_room, level, dam, TARGET_ROOM);

        for (victim = ch->in_room->people; victim != NULL; victim = nextVictim) {
            nextVictim = victim->next_in_room;
            if (is_safe_spell(ch, victim, TRUE) || (IS_NPC(ch) && IS_NPC(victim) && (ch->fighting == victim || victim->fighting == ch)))
                continue;
            
            if (saves_spell(level, victim, DAM_POISON)) {
                poison_effect(victim, level / 2, dam / 4, TARGET_CHAR);
                damage_old(ch, victim, dam / 2, sn, DAM_POISON, TRUE);
            } else {
                poison_effect(victim, level, dam, TARGET_CHAR);
                damage_old(ch, victim, dam, sn, DAM_POISON, TRUE);
            }
        }
    } else {
        send_to_char("This spell can only be cast in natural areas.\n\r", ch);
        return;
    }
}

void spell_birth_return(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    OBJ_DATA *shillelagh = char_has_shillelagh(ch);
    char tcbuf[15];
    
    if (shillelagh == NULL) {
        send_to_char("You do not posess your shillelagh.\n\r", ch);
        return;
    } else {
        act("`k$n is sucked into a blinding light and is gone.``", ch, NULL, NULL, TO_ROOM);
        send_to_char("`kYour shillelagh flares brightly as it returns you to its place of birth.``\n\r", ch);
        char_from_room(ch);
        char_to_room(ch, get_room_index(shillelagh->value[6]));
        act("A tree materializes and changes into the form of $n.", ch, NULL, NULL, TO_ROOM);
        do_look(ch, "auto");
        trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
        trap_check(ch, "room", ch->in_room, NULL);
        sprintf(tcbuf, "%d", ch->in_room->vnum);
        trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE, tcbuf);
        check_aggression(ch);
        return;
    }
}

void spell_waterwalk(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    if (!is_char_in_water_sector(ch)) {
        send_to_char("`cYou can only waterwalk in water.``\n\r", ch);
        return;
    }

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn)) {
        send_to_char("`nYou are already waterwalking.``\n\r", ch);
        return;
    }
    
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = (level / 2);
    af.location = APPLY_NONE;
    af.modifier = 1;    
    
    if (af.duration < 1)
        af.duration = 1;
    
	af.bitvector = AFF_AQUA_BREATHE;
    af.permaff = FALSE;
    af.composition = FALSE;
    af.comp_name = str_dup("");
    affect_to_char (victim, &af);

    send_to_char("`nYou rise slowly up to the surface of the water.``\n\r", victim);
    act("`c$N rises up out of the water and stands on its surface.``", victim, NULL, victim, TO_ROOM);
    return;
}

void spell_wild_enchant(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    OBJ_DATA *shillelagh;
    AFFECT_DATA *paf;
    int enhancement;

    if (!is_char_in_natural_sector(ch)) {
        send_to_char("This spell can only be cast in natural areas.\n\r", ch);
        return;
    }

    if ((shillelagh = char_has_shillelagh(ch)) == NULL) {
        send_to_char("You do not posess your shillelagh.\n\r", ch);
        return;
    }
    
    if (shillelagh->value[5] >= 5) {
        send_to_char("Your shillelagh has already reached its maximum enchantment.\n\r", ch);
        return;
    } else {
        enhancement = dice(1, 4);
               
        if (shillelagh->value[5] == 0) {
            // Never Enchanted...
            paf = new_affect();
            paf->where = TO_OBJECT;
            paf->type = sn;
            paf->level = level;
            paf->duration = -1;
            paf->location = APPLY_DAMROLL;
            paf->modifier = enhancement;
            paf->bitvector = 0;
            paf->permaff = FALSE;
            paf->composition = FALSE;
            paf->comp_name = str_dup ("");
            paf->next = shillelagh->affected;
            shillelagh->affected = paf;

            paf = new_affect();
            paf->where = TO_OBJECT;
            paf->type = sn;
            paf->level = level;
            paf->duration = -1;
            paf->location = APPLY_HITROLL;
            paf->modifier = enhancement;
            paf->bitvector = 0;
            paf->permaff = FALSE;
            paf->composition = FALSE;
            paf->comp_name = str_dup ("");
            paf->next = shillelagh->affected;
            shillelagh->affected = paf;
            
            shillelagh->value[5]++;
        } else {
            shillelagh->value[5]++;

            for (paf = shillelagh->affected; paf != NULL; paf = paf->next) {
                if (paf->location == APPLY_DAMROLL) {
                    paf->where = TO_OBJECT;
                    paf->type = sn;
                    paf->modifier += enhancement;
                    paf->level = UMAX(paf->level, level);
                    if (paf->modifier > 4)
                        SET_BIT(shillelagh->extra_flags[0], ITEM_HUM);
                }
            }

            for (paf = shillelagh->affected; paf != NULL; paf = paf->next) {
                if (paf->location == APPLY_HITROLL) {
                    paf->type = sn;
                    paf->where = TO_OBJECT;
                    paf->modifier += enhancement;
                    paf->level = UMAX(paf->level, level);
                    if (paf->modifier > 4)
                        SET_BIT (shillelagh->extra_flags[0], ITEM_HUM);
                }
            }
        }
    }

    send_to_char("You imbue your shillelagh with wild power.\n\r", ch);
		reset_char (ch);
    return;
}

void spell_winter_wrath(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    CHAR_DATA *victim, *nextVictim;
    int dam;

    act("$n summons forth a raging `oblizzard``!", ch, NULL, NULL, TO_ROOM);
    act("You summon forth a raging `oblizzard ``from the heavens!", ch, NULL, NULL, TO_CHAR);

    dam = dice((ch->level / 4), 8);

    cold_effect(ch->in_room, level, (dam / 2), TARGET_ROOM);

    for (victim = ch->in_room->people; victim != NULL; victim = nextVictim) {
        nextVictim = victim->next_in_room;
        
        if (is_safe_spell(ch, victim, TRUE) || (IS_NPC(victim) && IS_NPC(ch) && ((ch->fighting != victim) || (victim->fighting != ch))))
            continue;

        if (saves_spell(level, victim, DAM_COLD)) {
            cold_effect(victim, (level / 2), (dam / 4), TARGET_CHAR);
            damage_old(ch, victim, dam / 2, sn, DAM_COLD, TRUE);
        } else {
            cold_effect(victim, level, dam / 2, TARGET_CHAR);
            damage_old(ch, victim, dam, sn, DAM_COLD, TRUE);
        }
    }
}

void spell_resist_elements(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim != ch)
        return;

    if (is_affected(victim, sn)) {
        send_to_char ("You are already resistant to the elements.\n\r", ch);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = (level / 9);
    af.location = APPLY_NONE;
    af.modifier = 1;    
    af.bitvector = 0;
    af.permaff = FALSE;
    af.composition = FALSE;
    af.comp_name = str_dup("");
    affect_to_char(victim, &af);
    
    send_to_char("Fire in your eyes. Ice in your veins. Electricity in your mind. Air in your lungs.\n\r", victim);
    return;
}

void spell_ironwood(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    if (is_iron(obj)) {
        if ((obj->item_type) && !((obj->item_type == ITEM_WEAPON) || (obj->item_type == ITEM_ARMOR) || (obj->item_type == ITEM_CLOTHING))) {
            send_to_char("That type of item cannot be converted from metal into wood.\n\r", ch);
            return;          
        } else {
            if (number_percent() < get_skill(ch, sn)) {
                obj->extra_flags[0] |= ITEM_NONMETAL;
                if (obj->extra_flags[0] & ITEM_BURN_PROOF)
                    obj->extra_flags[0] ^= ITEM_BURN_PROOF;

                free_string(obj->material);
                obj->material = str_dup("wood");                    
                strcpy(buf, obj->short_descr);
                escape_color(buf);
                sprintf(buf2, "`d%s``", buf);
                free_string(obj->short_descr);
                obj->short_descr = str_dup(buf2);
                strcpy(buf, obj->description);
                escape_color(buf);
                sprintf(buf2, "`d%s``", buf);
                free_string(obj->description);
                obj->description = str_dup(buf2);
                sprintf(buf, "Brown fumes engulf %s.\n\r", obj->short_descr);
                send_to_char(buf, ch);
            } else {
                extract_obj(obj);
                sprintf(buf, "Brown fumes evaporate %s.\n\r", obj->short_descr);
                send_to_char(buf, ch);
            }
        }
    } else  {
        send_to_char("Only objects made from iron or steel can be converted into wood.\n\r", ch);
        return;
    }
}

void spell_insect_plague(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
	if (is_char_in_water_sector(ch)) {
        send_to_char ("You cannot summon insects here.\n\r", ch);
        return;		
	}
	

	
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	
		if (is_affected (victim, gsn_fireshield)) {
		send_to_char ("You summon a dark cloud of insects... which is instantly fried.\n\r", ch);
        return; 
	}

	act("$n `ais enveloped in a dark cloud of insects!``",victim,NULL,NULL,TO_ROOM);
	send_to_char("`aA dark cloud of insects envelops you and tears at your flesh!``\n\r",victim);
	
	if (is_affected(victim, sn)) {
		AFFECT_DATA *af;
		for (af = victim->affected; af != NULL; af = af->next) {
			if (af->type == sn)
				break;
		}
		
		af->duration += (ch->level / 9);
	} else {
		af.where = TO_AFFECTS;
		af.type = gsn_insect_plague;
		af.level = level;
		af.duration = ch->level / 9;
		if (af.duration < 1)
			af.duration = 1;	
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = 0;
		af.permaff = FALSE;
		af.composition = FALSE;
		af.comp_name = str_dup(ch->name);
		affect_to_char(victim, &af);
		
		af.where = TO_AFFECTS;
		af.type = gsn_disorientation;
		af.level = level;
		af.duration = ch->level / 9;
		if (af.duration < 1)
			af.duration = 1;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = 0;
		af.permaff = FALSE;
		af.composition = FALSE;
		af.comp_name = str_dup(ch->name);
		affect_to_char(victim, &af);
	}
	
	return;
}

void spell_desert_wrath(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    CHAR_DATA *victim, *nextVictim;
    int dam;

    act("$n summons forth a furious `dsandstorm``!", ch, NULL, NULL, TO_ROOM);
    act("You summon forth a furious `dsandstorm ``from the heavens!", ch, NULL, NULL, TO_CHAR);

    dam = dice((ch->level / 3), 10);

    for (victim = ch->in_room->people; victim != NULL; victim = nextVictim) {
        nextVictim = victim->next_in_room;
        
        if (is_safe_spell(ch, victim, TRUE) || (IS_NPC(victim) && IS_NPC(ch) && ((ch->fighting != victim) || (victim->fighting != ch))))
            continue;

        if (saves_spell(level, victim, DAM_WIND)) {
            damage_old(ch, victim, dam / 2, sn, DAM_WIND, TRUE);
            if (!IS_NPC(victim) && (victim->pcdata->condition[COND_THIRST] > 0))
                victim->pcdata->condition[COND_THIRST] /= 2;
        } else {
            damage_old(ch, victim, dam, sn, DAM_COLD, TRUE);
            if (!IS_NPC(victim) && (victim->pcdata->condition[COND_THIRST] > 0))
                victim->pcdata->condition[COND_THIRST] = 0;
        }
    }
}

void spell_draw_life(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    dam = (ch->level / 3);
    dam -= dice(1, 6);
    
    if (dam < 1)
        dam = 10;
    
    ch->hit += dam;    
    damage_old(ch, victim, dam, sn, DAM_VAMPIRIC, TRUE);
    return;
}

void spell_force(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam = 30;
    dam += dice(ch->level, 4);
    
    if (saves_spell(level, victim, DAM_PIERCE))
        dam /= 2;
        
    damage_old(ch, victim, dam, sn, DAM_PIERCE, TRUE);
    return;
}

void spell_clarity(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
CHAR_DATA *victim = (CHAR_DATA *) vo;
AFFECT_DATA af;
if (victim != ch)
{
send_to_char ("You cannot cast this spell on another.\n\r", ch);
        return;
}
    if (is_affected(victim, sn)) {
        send_to_char ("You already have clarity in spellcasting.\n\r", ch);
        return;
    }
act("`h$n's eyes glow a bright `jgreen.``",victim,NULL,NULL,TO_ROOM);

send_to_char("`aYou bind your mind's eye to nature, making spells ring with new clarity.``\n\r",victim);


af.where = TO_AFFECTS;
af.type = gsn_clarity;
af.level = level;
af.duration = ch->level / 5;
if (af.duration < 1)
af.duration = 1;
af.location = APPLY_NONE;
af.modifier = 0;
af.bitvector = 0;
af.permaff = FALSE;
af.composition = FALSE;
af.comp_name = str_dup(ch->name);
affect_to_char(victim, &af);
return;
}

void spell_tree_form(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    OBJ_DATA *shillelagh = char_has_shillelagh(ch);
    
    if (is_tree_in_room(ch->in_room)) {
        send_to_char("A tree already grows here.\n\r", ch);
        return;
    } else if (shillelagh == NULL) {
        send_to_char("You may only take tree form when you possess your shillelagh.\n\r", ch);
        return;
    } else if (!is_char_in_natural_sector(ch)) {
	    send_to_char("You cannot take tree form here.\n\r", ch);
	    return;
	} else {
	    send_to_char("`cYou take the form of a tree.``\n\r", ch);            
	    act("$n is gone, and only a tree remains.", ch, NULL, NULL, TO_ROOM);
	    perform_shapechange(ch, MOB_VNUM_SHAPECHANGE_TREE);
	}
}

void spell_serene_grove(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    OBJ_DATA *shillelagh = char_has_shillelagh(ch);

    if (shillelagh == NULL) {
        send_to_char("You cannot return to the serene grove without your shillelagh.\n\r", ch);
        return;
    } else if (!is_char_in_natural_sector(ch)) {
        send_to_char("You cannot return to the serene grove from here.\n\r", ch);
        return;
    } else {      
        act("`c$n is absorbed by the surrounding plantlife.``", ch, NULL, NULL, TO_ROOM);
        send_to_char("`cYou return to the serene grove.``\n\r", ch);
        char_from_room(ch);
        char_to_room(ch, get_room_index(ROOM_VNUM_SERENE_GROVE));
        act("$n has returned to the serene grove.", ch, NULL, NULL, TO_ROOM);
        do_look(ch, "auto");
        trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
        return;
    }
}

void spell_regeneration(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim != ch)
        return;

    if (is_affected(victim, sn)) {
        send_to_char ("You are already regenerating!\n\r", ch);
        return;
    } else if (!is_char_in_natural_sector(ch)) {
        send_to_char ("You can not begin regeneration here!\n\r", ch);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = (level / 10);
    af.location = APPLY_NONE;
    af.modifier = 1;
    af.bitvector = 0;
    af.permaff = FALSE;
    af.composition = FALSE;
    af.comp_name = str_dup("");
    
    if (af.duration < 1)
    	af.duration = 1;
    	
    affect_to_char(victim, &af);

    send_to_char("`cYou draw upon the life force in your natural surroundings.``\n\r", victim);
    return;
}

void spell_arcane_diminish (int sn, int level, CHAR_DATA * ch, void *vo, int target)
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

void spell_moonlance(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam = 50;
    if ((time_info.hour > 18) || (time_info.hour < 5)) {
        dam += dice((ch->level / 2), 4);
        
        if (saves_spell(level, victim, DAM_LIGHT))
            dam /= 2;
            
        damage_old(ch, victim, dam, sn, DAM_LIGHT, TRUE);
    } else {
        send_to_char("You can only use this spell at night.``\n\r", ch);
        return;       
    }
}

void spell_betrayal(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *gch = NULL;
	
	for (gch = char_list; gch != NULL; gch = gch->next) {
		if (is_same_group (gch, victim) && (gch->leader == victim)) {
			if ((gch != ch) && (gch != victim) && (gch->in_room == ch->in_room) && IS_NPC(gch) && is_animal(gch)) {
				stop_follower(gch);
			}
		}
	}

	send_to_char("`bYour followers have betrayed you!``\n\r", victim);
	act("`b$N's followers have betrayed them!``", victim, NULL, victim, TO_ROOM);
	return;
}

void spell_tap_root(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam = 100;
    dam += dice(ch->level, 10);
    
    if (saves_spell(level, victim, DAM_PIERCE))
        dam /= 2;
        
    damage_old(ch, victim, dam, sn, DAM_PIERCE, TRUE);
    return;
}

void spell_scry(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    OBJ_DATA *shillelagh;
    CHAR_DATA *victim = get_char_world(ch, target_name);
    char buf[MAX_STRING_LENGTH];

    if ((shillelagh = char_has_shillelagh(ch)) == NULL) {
        send_to_char("You cannot scry someone without your shillelagh.\n\r", ch);
        return;
    }    

    if (victim == NULL) {
        send_to_char("You failed.\n\r", ch);
        return;
    }
    
    if ((victim->in_room == NULL) || (victim->level >= LEVEL_IMMORTAL)) {
        send_to_char ("You failed.\n\r", ch);
        return;
    }
    
    sprintf(buf, "You see that %s is in %s, %s.", victim->name, victim->in_room->area->name, victim->in_room->name);
    act(buf, ch, NULL, NULL, TO_CHAR);
    return;
}

void spell_lunar_heal(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    
        if (is_affected(victim, gsn_greymantle) || IS_SET(victim->act,ACT_UNDEAD)) {
            reverse_heal(ch, victim, 100, sn);
        } else {
            if (!IS_NPC(victim) && victim->pcdata->soul_link) {
                victim->hit = UMIN(victim->hit + 100, victim->max_hit);
                victim->pcdata->soul_link->hit = UMIN(victim->pcdata->soul_link->hit + 100, victim->pcdata->soul_link->max_hit);
                update_pos(victim->pcdata->soul_link);
                send_to_char("A warm lunar wave rushes over your body.\n\r", victim->pcdata->soul_link);
            } else {
                victim->hit = UMIN(victim->hit + 100, victim->max_hit);
                update_pos(victim);
                send_to_char("A warm lunar wave rushes over your body.\n\r", victim);
            }
        }
        
        if (ch != victim)
            send_to_char ("Ok.\n\r", ch);
        
        return;
    
    }


void spell_life_circle(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    OBJ_DATA *shillelagh;
    CHAR_DATA *gch;

    if ((shillelagh = char_has_shillelagh(ch)) == NULL) {
        send_to_char("You cannot perform the life circle without your shillelagh.\n\r", ch);
        return;
    } else if (ch->mana < 300) {
        send_to_char("You do not have enough mana to perform the life circle.\n\r", ch);
        return;
    }

    act("`k$n's shillelagh emminates a wave of warm radiant energy.``", ch, NULL, NULL, TO_ROOM);
    
    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
        if (gch == ch)
            continue;
        
        if (is_affected(gch, gsn_greymantle) || IS_SET(gch->act, ACT_UNDEAD)) {
            continue;
        }
        
        send_to_char("`kA warm feeling fills your body.``\n\r", gch);        
        gch->hit = gch->max_hit;
        gch->mana = gch->max_mana;
        gch->move = gch->max_move;
    }

    ch->hit = 1;
    ch->mana = 1;
    ch->move = 1;
    send_to_char("`kYour shillelagh emminates a wave of warm radiant energy.``\n\r", ch);
    return;
}

void spell_sprout_tree(int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    OBJ_DATA *tree;
    char buf[MAX_STRING_LENGTH];
    
    if (char_has_shillelagh(ch) == NULL) {
        send_to_char("You are not capable of sprouting a tree without your shillelagh.\n\r", ch);
        return;
    }    

    if (is_char_in_natural_sector(ch) && !IS_SET(ch->in_room->area->area_flags, AREA_NO_TREE) && !is_char_in_water_sector(ch)) {        
        if (is_tree_in_room(ch->in_room)) {
            send_to_char("There is alread a tree here.\n\r", ch);
            return;
        }
        
        send_to_char("You draw on the power of your shillelagh and sprout a small tree.\n\r", ch);
        act("A small tree sprouts out of the ground.", NULL, NULL, NULL, TO_ROOM);
        

        tree = create_object(get_obj_index(OBJ_VNUM_TREE),0);
        tree->value[2] = TREE_SAPLING;
        tree->value[1] = 0;

        int chance = number_percent();
        if (chance < 65) {
            int num = dice(1, 11);
            
            switch (num) {
                case (1):
                    tree->value[0] = TREE_TYPE_OAK;
                    break;
                case (2):
                    tree->value[0] = TREE_TYPE_MAPLE;
                    break;
                case (3):
                    tree->value[0] = TREE_TYPE_BIRCH;
                    break;
                case (4):
                    tree->value[0] = TREE_TYPE_PINE;
                    break;
                case (5):
                    tree->value[0] = TREE_TYPE_WILLOW;
                    break;
                case (6):
                    tree->value[0] = TREE_TYPE_ELM;
                    break;
                case (7):
                    tree->value[0] = TREE_TYPE_POPLAR;
                    break;
                case (8):
                    tree->value[0] = TREE_TYPE_KARRI;
                    break;
                case (9):
                    tree->value[0] = TREE_TYPE_MARRI;
                    break;
                case (10):
                    tree->value[0] = TREE_TYPE_JARRAH;
                    break;
                case (11):
                    tree->value[0] = TREE_TYPE_ROTSTENCH;
                    break;
            }
        } else if (chance < 85) {
            int num = dice(1, 3);
            
            switch (num) {
                case (1):
                    tree->value[0] = TREE_TYPE_MOONBLOSSOM;
                    break;
                case (2):
                    tree->value[0] = TREE_TYPE_RISIRIEL;
                    break;
                case (3):
                    tree->value[0] = TREE_TYPE_IRONWOOD;
                    break;
            }
        } else if (chance < 95) {
            int num = dice(1, 3);
            
            switch (num) {
                case (1):
                    tree->value[0] = TREE_TYPE_HELLROOT;
                    break;
                case (2):
                    tree->value[0] = TREE_TYPE_BARBVINE;
                    break;
                case (3):
                    tree->value[0] = TREE_TYPE_GREEN_LEECHLEAF;
                    break;
            }
        } else {
            if (number_percent() < 51)            
                tree->value[0] = TREE_TYPE_SLIMEWOOD;
            else 
                tree->value[0] = TREE_TYPE_JEWELWOOD;
        }
                

        sprintf(buf,"An %s sapling grows here.",tree_table[tree->value[0]].name);
        free_string(tree->description);
        tree->description = str_dup(buf);
        sprintf(buf,"an %s sapling",tree_table[tree->value[0]].name);
        free_string(tree->short_descr);
        tree->short_descr = str_dup(buf);
        sprintf(buf,"tree %s",tree_table[tree->value[0]].name);
        free_string(tree->name);
        tree->name = str_dup(buf);
        obj_to_room(tree, ch->in_room);
        return;
    } else {
        send_to_char("A tree will not grow here.\n\r", ch);
        return;
    }
}

// Command to dismiss a created shillelagh
void do_shillelaghdismiss (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *shillelagh;
	char buf[MAX_STRING_LENGTH];

	if ((shillelagh = get_obj_wear(ch, "shillelagh")) == NULL)
	{
		send_to_char ("You do not currently possess your shillelagh.\n\r", ch);
		return;
	}

	sprintf (buf, "`a%s `arots away in a cloud of dust.``\n\r", shillelagh->short_descr);
	send_to_char (buf, ch);
	act("`a$n utters the magic words of return, making the shillelagh rot away in a cloud of dust.``", ch, NULL, NULL, TO_ROOM);
	unequip_char (ch, shillelagh);
	obj_from_char (shillelagh);
	extract_obj (shillelagh);
	reset_char (ch);
	return;
}


