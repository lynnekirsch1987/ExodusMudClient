#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include "merc.h"
#include "music.h"
#include "interp.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"
#include "obj_trig.h"
#include "Druid.h"
#include "Utilities.h"

DECLARE_DO_FUN (do_quit);
extern bool half;
extern bool quad;
extern bool doub;
extern bool battle_royale;
extern bool crit_strike_possible;
extern bool player_rewarding;
extern CHAR_DATA *br_leader;
extern CHAR_DATA *last_br_kill;
extern CHAR_DATA *last_br_victim;
extern int last_br_points;
extern short br_death_counter;
extern int half_timer;
extern int quad_timer;
extern int double_timer;
extern int battle_royale_timer;
extern const char *dir_name[];
extern struct clan_type clan_table[MAX_CLAN];
extern char imm_who_msg[];
extern int superduel_ticks;
extern bool superduel_teams;
extern short poll_ticks;
extern char poll_question[];
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
long int level_cost args ((int level));
bool imm_advance = FALSE;
void pk_chan args((char* text));
void arena_chan args((char* text));
void activate_trigger_statement args((int statement_on, OBJ_TRIG *ot, OBJ_DATA *obj, CHAR_DATA *ch, CHAR_DATA *victim));
void delete_player_corpse args((OBJ_DATA * pObj));
void save_player_corpse args((OBJ_DATA * pObj));
extern CHAR_DATA *load_char_obj2 (char *name);
void free_char args ((CHAR_DATA * ch));
void unhire args((ROOM_INDEX_DATA *location));

bool is_player_in_group args((CHAR_DATA * ch));

bool check_social args((CHAR_DATA * ch, char *command, char *argument));

extern bool is_linkloading;
void ppurge args((CHAR_DATA *ch, CHAR_DATA* victim));
void death_spasms args((CHAR_DATA *ch));
void check_gravity_char args((CHAR_DATA *ch));

extern OBJ_TRIG *ot_list;
void superduel_ends args((CHAR_DATA *winner, CHAR_DATA *loser));
extern OBJ_TRIG *on_tick_ot_list;

void spell_haste args((int sn, int level, CHAR_DATA * ch, void *vo, int target));

ROOM_INDEX_DATA *find_location args ((CHAR_DATA * ch, char *arg));
int hit_gain args ((CHAR_DATA * ch));
int mana_gain args ((CHAR_DATA * ch));
int move_gain args ((CHAR_DATA * ch));
void mobile_update args ((void));
void weather_update args ((void));
void char_update args ((void));
void obj_update args ((void));
void obj_trig_tick_update args ((void));
void room_update args ((void));
void aggr_update args ((void));
//IBLIS - Allows high cha characters a chance to not be attacked by aggie mobs
bool resist_aggro args ((CHAR_DATA * victim, CHAR_DATA * ch));
// Morgan, June 26. 2000  For the automated timed reboot function
void check_reboot args ((void));
void check_shutdown args ((void));
void br_transfer args ((CHAR_DATA * ch));
// Reaver chants
void chant_update args ((void));
// Fight.c stuff
bool check_clasp args((CHAR_DATA *ch));
int check_entangle args ((CHAR_DATA * ch));


void knock_update args((void));

void obj_trig_second_update args((void));

void update_aggressor args ((CHAR_DATA * ch, CHAR_DATA * victim));

//Iblis 7/26/03 - Declare in fishing.c (to keep all trophy fishing in one file)
void update_reeling args ((void));
void update_fishing args ((void));

//Iblis 4/05/04 - Declared in trade_skills.c (to keep all that stuff in one file)
void update_trade_skills args ((void));

//bIlis 6/15/04 - Declared in cardgame.c (to keep all that stuff in one file)
void cardgame_update args ((void));
void update_cardgame_round args ((void));

void meteor_swarm args((CHAR_DATA *ch, CHAR_DATA *victim));

// Iblis 6/29/03 - Falconry object deliver skill
void falcon_update args ((void));

//Iblis 6/28/03 - Aggressor timer flag
void aggressor_update args ((void));

// Adeon 6/30/03 -- Check status of character sinking in room
void sink_update args ((void));



int save_number = 0;
int info_tick = 0;

void multiClass args ((DESCRIPTOR_DATA * d, char *argument));
void free_aggressor_list args ((AGGRESSOR_LIST * b));
void delivery_part3 args ((CHAR_DATA * ch));
void delivery_part2 args ((CHAR_DATA * ch));

void sink_update (void)
{
	CHAR_DATA *sch;
	ROOM_INDEX_DATA *sinkhole;
	char buf[MAX_STRING_LENGTH];
	int half_time;

	for (sch = char_list; sch != NULL; sch = sch->next)
	{
		if (IS_IMMORTAL (sch))
			continue;
		//Mobs with nosink flags don't sink!
		if (IS_NPC (sch) && IS_SET (sch->act2, ACT_NOSINK))
			continue;
		if ((sinkhole = sch->in_room) == NULL)
			continue;

		if (!IS_SET (sinkhole->room_flags2, ROOM_SINKING))
		{
			sch->sink_timer = -1;
			continue;
		}
		if (sch->sink_timer == 0)
		{
			if (sinkhole->sink_msg != NULL  && str_cmp(sinkhole->sink_msg,"(null)"))
			{
				sprintf (buf, "%s\n\r", sinkhole->sink_msg);
				//	      act ("$n has vanished...  You hear $s muffled cries from somewhere below.", sch, NULL, NULL, TO_ROOM);
				send_to_char (buf, sch);
			}
			//Iblis 8/29/04 - Added new sink msg that OTHERS see
			if (sinkhole->sink_msg_others != NULL && str_cmp(sinkhole->sink_msg_others,"(null)"))
				act (sinkhole->sink_msg_others, sch, NULL, NULL, TO_ROOM);
			char_from_room (sch);
			char_to_room (sch, get_room_index (sinkhole->sink_dest));
			do_look (sch, "");
			trip_triggers(sch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
			trap_check(sch,"room",sch->in_room,NULL);
			sprintf(buf,"%d",sch->in_room->vnum);
			trip_triggers_arg(sch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,buf);
			check_aggression(sch);
		}
		half_time = (sinkhole->sink_timer / 2) - (sinkhole->sink_timer % 2);
		if (sch->sink_timer == half_time && half_time > 0)
		{
			if (sinkhole->sink_warning != NULL && str_cmp(sinkhole->sink_warning,"(null)"))
			{
				sprintf (buf, "%s\n\r", sinkhole->sink_warning);
				send_to_char (buf, sch);
			}
		}
		sch->sink_timer--;
	}

	return;
}



void auction_update (void)
{
	char buf[MAX_STRING_LENGTH];
	int silver, gold;

	if (auction->item != NULL)
		if (--auction->pulse <= 0)
		{
			auction->pulse = PULSE_AUCTION;
			switch (++auction->going)
			{
			case 1:

			case 2:
				if (auction->bid > 0)
					sprintf (buf, "%s: going %s for `o%d`N.",
					auction->item->short_descr,
					((auction->going == 1) ? "once" : "twice"),
					auction->bid);
				else
					sprintf (buf, "%s: going %s (no bid received yet).",
					auction->item->short_descr,
					((auction->going == 1) ? "once" : "twice"));

				talk_auction (buf);
				break;

			case 3:
				/*          if(auction->bid > 0 && (auction->bid > 
				(auction->buyer->silver + (100 *
				auction->buyer->gold))))
				{
				act("The auctioneer appears before you, realizes you cannot afford $p, and disappears just as quickly.", auction->buyer, auction->item, NULL, TO_CHAR);
				act("The auctioneer appears before $n, realizes $e cannot afford $p, and disappears just as quickly.", auction->buyer, auction->item, NULL, TO_ROOM);
				sprintf(buf, "Auction of %s unsucessessful due to %s's lack of funds.", auction->item->short_descr, IS_NPC(auction->buyer) ? auction->buyer->short_descr : auction->buyer->name);
				talk_auction(buf);

				act("The auctioneer appears before you to return $p to you.",
				auction->seller, auction->item, NULL, TO_CHAR);
				act("The auctioneer appears before $n to return $p to $m.",
				auction->seller, auction->item, NULL, TO_ROOM);

				obj_to_char(auction->item, auction->seller);
				auction->item = NULL;
				}
				else */
				if (auction->bid > 0)
				{
					sprintf (buf, "%s sold to %s for `o%d`N.",
						auction->item->short_descr,
						IS_NPC (auction->buyer) ? auction->buyer->
short_descr : auction->buyer->name, auction->bid);
					talk_auction (buf);
					if (auction->buyer->carry_number + get_obj_number (auction->item) > can_carry_n (auction->buyer)
						|| get_carry_weight (auction->buyer) + get_obj_weight (auction->item) 
				 > can_carry_w (auction->buyer))
					{
						act("The auctioneer appears before you in a puff of smoke and drops $p at your feet, since you are unable to carry it.", auction->buyer, auction->item, NULL, TO_CHAR);
						act("The auctioneer appears before $n, and drops $p at $n's feet.",
							auction->buyer, auction->item, NULL, TO_ROOM);
						obj_to_room(auction->item,auction->buyer->in_room);
					}
					else
					{
						obj_to_char (auction->item, auction->buyer);
						act
							("The auctioneer appears before you in a puff of smoke and hands you $p.",
							auction->buyer, auction->item, NULL, TO_CHAR);
						act ("The auctioneer appears before $n, and hands $m $p",
							auction->buyer, auction->item, NULL, TO_ROOM);
					}

					// Let the seller have gold instead of silver
					gold = auction->bid / 100;
					silver = auction->bid - (gold * 100);
					auction->seller->gold += gold;
					auction->seller->silver += silver;

					//              deduct_cost(auction->seller, 0 - auction->bid);
					auction->item = NULL;
				}
				else
				{
					sprintf (buf,
						"No bids received for %s - object has been removed.",
						auction->item->short_descr);
					talk_auction (buf);

					if (number_percent () < 0)
					{
						ROOM_INDEX_DATA *location;
						OBJ_DATA *pit_obj;

						location = get_room_index (ROOM_VNUM_PIT);
						for (pit_obj = object_list; pit_obj != NULL;
							pit_obj = pit_obj->next)
						{
							if (pit_obj->pIndexData->vnum == OBJ_VNUM_PIT)
								break;
						}

						if (pit_obj != NULL)
						{
							act ("$p has been placed into the donation box.",
								auction->seller, auction->item, NULL, TO_CHAR);
							act
								("Something materializes from thin air and falls into the donation box.",
								location->people, NULL, NULL, TO_ALL);

							obj_to_obj (auction->item, pit_obj);
						}
						else
							extract_obj (auction->item);
					}
					else
					{
						if (auction->seller->carry_number + get_obj_number (auction->item) > can_carry_n (auction->seller)
							|| get_carry_weight (auction->seller) + get_obj_weight (auction->item) >
							can_carry_w (auction->seller))
						{
							act("The auctioneer appears before you and returns $p to the ground before you.",
								auction->seller, auction->item,NULL,TO_CHAR);
							act("The auctioneer appears before $n and returns $p to the ground before $m.",
								auction->seller, auction->item, NULL, TO_ROOM);
							obj_to_room(auction->item, auction->seller->in_room);
						}
						else
						{
							act
								("The auctioneer appears before you to return $p to you.",
								auction->seller, auction->item, NULL, TO_CHAR);
							act
								("The auctioneer appears before $n to return $p to $m.",
								auction->seller, auction->item, NULL, TO_ROOM);

							obj_to_char (auction->item, auction->seller);
						}
					}
					auction->item = NULL;
				}
			}
		}
}


void advance_level (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH];
	int add_hp;
	int add_mana;
	int add_move;
	DESCRIPTOR_DATA *d;
	//int add_prac;

	if (!imm_advance && ((ch->level == 30 || ch->level == 60) 
		&& ch->Class != PC_CLASS_NECROMANCER && ch->Class != PC_CLASS_CHAOS_JESTER
		&& (!((ch->race == PC_RACE_NERIX || ch->race == PC_RACE_KALIAN) && !ch->pcdata->has_reincarnated))
		&& !(ch->race == PC_RACE_AVATAR && ch->pcdata->avatar_type >= 3)))
	{
		for (d = descriptor_list; d != NULL; d = d->next)
		{
			if (d->character == ch)
			{
				if (d->connected != CON_FINISH_MULTICLASS
					&& d->connected != CON_START_MULTICLASS)
				{
					d->connected = CON_START_MULTICLASS;
					multiClass (d, "");
					return;
				}
				if (d->connected == CON_FINISH_MULTICLASS)
				{
					d->connected = CON_PLAYING;
					break;
				}
			}
		}
		// it's not a bug if this happens to be a familiar (An npc that can level)
		if (!IS_NPC(ch) || (IS_NPC(ch) && !IS_SET(ch->act2,ACT_FAMILIAR)))
			bug ("advance_level : descriptor not found", 0);
	}


	if (ch->level > 10)
		SET_BIT (ch->comm, COMM_NOINFO);

	ch->pcdata->last_level = (ch->played + (int) (current_time - ch->logon)) / 3600;


	add_hp = (3 * number_range(hp_gains[current_Class(ch)][ch->perm_stat[STAT_CON]][0], hp_gains[current_Class(ch)][ch->perm_stat[STAT_CON]][1])
		+ number_range(hp_gains[current_Class(ch)][ch->perm_stat[STAT_STR]][0], hp_gains[current_Class(ch)][ch->perm_stat[STAT_STR]][1])) / 4;

	add_mana = (
		3 * number_range( 
			mana_gains[current_Class(ch)][ch->perm_stat[STAT_INT]][0],
			mana_gains[current_Class(ch)][ch->perm_stat[STAT_INT]][1]) 
		+
		number_range(
			mana_gains[current_Class(ch)][ch->perm_stat[STAT_WIS]][0],
			mana_gains[current_Class(ch)][ch->perm_stat[STAT_WIS]][1])
		) / 4;


	add_move = number_range ((ch->perm_stat[STAT_DEX] +
		ch->perm_stat[STAT_CON]) / 8,
		(2 * ch->perm_stat[STAT_DEX] +
		ch->perm_stat[STAT_CON]) / 7);

	//IBLIS 5/28/03 - commented out for new xp system
	//add_prac = URANGE (1, wis_app[ch->perm_stat[STAT_WIS]].practice +
	//           prim_app[ch->perm_stat
	//                    [Class_table[ch->Class].attr_prime]].practice,
	//                   7);
	//IBLIS 6/01/04 - Jesus christ I originally put this in wrong back in back when the whore still 
	//lived with me, I then discovered it was wrong maybe in November '03? but I wasn't sure how to compensate
	//current vro'aths.  This needs to be put in but I'm too lazy to do it now
	//Mike 2/19/07 - Fixed Vroath hp bonus...and that's not nice Iblis...
	add_hp = UMAX (2, add_hp);
	add_mana = UMAX (2, add_mana);
	add_move = UMAX (1, add_move);
	if (ch->race == PC_RACE_VROATH)
	{
		add_hp++;
	}
	if (ch->race == PC_RACE_AVATAR)
	{
		if (ch->pcdata->avatar_type != 2)
		{
			add_hp = (ch->perm_stat[STAT_CON] - 3);
			add_mana = (ch->perm_stat[STAT_INT] - 3);
			add_move = (ch->perm_stat[STAT_DEX] - 3);
		}
		else
		{
			add_hp += 2;
			add_mana += 2;
			add_move += 2;
		}
	}
	if(ch->race == PC_RACE_SWARM)
		add_hp = 1;

	// Base A druids mana solely on Wisdom!
	if (ch->Class == PC_CLASS_DRUID) {
		add_mana = (ch->perm_stat[STAT_WIS] - 5);

		if (ch->perm_stat[STAT_INT] < 10)
			add_mana += (ch->perm_stat[STAT_INT] - 10);
	}

	ch->max_hit += add_hp;
	ch->max_mana += add_mana;
	ch->max_move += add_move;
	//IBLIS 5/28/03 - commented out for new xp system
	// ch->practice += add_prac;

	ch->pcdata->perm_hit += add_hp;
	ch->pcdata->perm_mana += add_mana;
	ch->pcdata->perm_move += add_move;

	//IBLIS 5/28/03 - removed prac info for new xp system
	sprintf (buf,
		"`bYou gain: %d/%d hp, %d/%d m, %d/%d mv",
		add_hp, ch->max_hit,
		add_mana, ch->max_mana, add_move, ch->max_move);

	if (ch->level < 10)
	{
		if (ch->race == 8)
			ch->train += 1;
		ch->train += 1;
		if (ch->race != 8)
			sprintf (buf + strlen (buf), " 1/%d train", ch->train);
		else
			sprintf (buf + strlen (buf), " 2/%d train", ch->train);
	}

	strcat (buf, ".``\n\r");

	send_to_char (buf, ch);
	// IBLIS 5/18/03 - Added the actual level inreasing and assassin checking to this function
	// since the data items needed for the Assassin check are declared in this file
	ch->level++;
	if (ch->Class == PC_CLASS_NECROMANCER || ch->Class == PC_CLASS_CHAOS_JESTER)
		group_add_all (ch);


	if (ch->level == 15)
	{
		send_to_char("`qWARNING : `jYou can now `qSTARVE `jand `qDEHYDRATE and die.  Don't forget to eat and drink.``\n\r",ch);
	}
	if (ch->level == 45 && ch->clan == clanname_to_slot("guttersnipes"))
	{
		int slot=0;
		MEMBER *m;
		bool fixup = FALSE;
		slot = (ch->clan);
		// set the PC clan flags as appropriate and save them
		ch->clan = CLAN_BOGUS;
		ch->pcdata->deputy = FALSE;
		save_char_obj (ch);

		// send the PC a message if they were in a defunct clan
		if (clan_table[slot].status < CLAN_PROPOSED)
		{
			fixup = TRUE;
			send_to_char
				("You are not currently in or applying to any clan.\n\r", ch);

			// do not return in this case, because what we really want to do is remove
			// them from that clans membership list just below.
			//return;
		}
		// find the member record and remove it from the linked list
		m = find_member (clan_table[slot].members, ch->name);
		if (m == NULL)
		{
			m = find_member (clan_table[slot].applicants, ch->name);
			if (m == NULL)
			{
				if (!fixup)
					send_to_char
					("You are not currently in or applying to any clan.\n\r", ch);
				return;
			}

			else
			{
				clan_table[slot].applicants =
					delete_member (clan_table[slot].applicants, ch->name);
				if (!fixup)
				{
					sprintf (buf,
						"You withdraw your application to clan %s``.\n\r",
						clan_table[slot].name);
					send_to_char (buf, ch);
				}
				save_clan (slot);

#ifdef VERBOSE_CLANS
				log_player_command ("f:clan_defect", ch, "*just an applicant*");

#endif /*  */
				return;
			}
		}

		else
		{
			clan_table[slot].members =
				delete_member (clan_table[slot].members, ch->name);
		}
		if (fixup)
		{

#ifdef VERBOSE_CLANS
			log_player_command ("f:clan_defect", ch, "*fixup defunct clan*");

#endif /*  */
			return;
		}
		// Now notify the membership that the player has defected
		sprintf (buf, "%s has defected from clan %s``.\n\r", ch->name,
			clan_table[slot].name);
		send_clan_members (slot, buf);
		clan_table[slot].levels -= m->levels;
		/*      if ((clan_table[slot].levels < FULL_CLAN_LEVELS))
		{

		// if this clan has lost too many levels then we restrict
		// it like a proposed clan
		clan_table[slot].status = CLAN_RESTRICTED;
		sprintf (buf,
		"Clan %s`` has lost too many members and is now restricted.\n\r",
		clan_table[slot].name);
		send_clan_members (slot, buf);
		}*/
		save_clan (slot);

		// notify the player of the action
		sprintf (buf, "You defect from clan %s``.\n\r", clan_table[slot].name);
		send_to_char (buf, ch);

#ifdef VERBOSE_CLANS
		log_player_command ("f:clan_defect", ch, ((char *) NULL));

#endif /*  */
	}	  
	sprintf (buf, "$N has attained level %d!", ch->level);
	wiznet (buf, ch, NULL, WIZ_LEVELS, 0, 0);
	// Minax 7-6-02 Assassins who are unclanned at level 10 are forced to go loner.
	if (ch->level == 10 && IS_CLASS (ch, PC_CLASS_ASSASSIN)
		&& clan_table[ch->clan].ctype != CLAN_TYPE_PC
		&& ch->pcdata->loner != TRUE)
	{
		send_to_char
			("`oYou are no longer protected.  As a level 10 Assassin, you are now a LONER!``\n\r",
			ch);
		ch->pcdata->loner = TRUE;
	}
	return;
}

void decrease_level (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH];
	int add_hp;
	int add_mana;
	int add_move;

	ch->pcdata->last_level =
		(ch->played + (int) (current_time - ch->logon)) / 3600;


	add_hp =
		(3 *
		number_range (hp_gains[current_Class (ch)][ch->perm_stat[STAT_CON]]
	[0],
		hp_gains[current_Class (ch)][ch->
		perm_stat[STAT_CON]][1])
		+
		number_range (hp_gains[current_Class (ch)][ch->perm_stat[STAT_STR]]
	[0],
		hp_gains[current_Class (ch)][ch->
		perm_stat[STAT_STR]][1])) / 4;

	add_mana = (3 * number_range (mana_gains[current_Class (ch)]
	[ch->perm_stat[STAT_INT]][0],
		mana_gains[current_Class (ch)][ch->
		perm_stat
		[STAT_INT]]
	[1]) +
		number_range (mana_gains[current_Class (ch)]
	[ch->perm_stat[STAT_WIS]][0],
		mana_gains[current_Class (ch)][ch->
		perm_stat
		[STAT_WIS]]
	[1])) / 4;


	add_move = number_range ((ch->perm_stat[STAT_DEX] +
		ch->perm_stat[STAT_CON]) / 8,
		(2 * ch->perm_stat[STAT_DEX] +
		ch->perm_stat[STAT_CON]) / 7);
	//IBLIS 5/28/03 - commented out for new xp system
	//  add_prac = URANGE (1, wis_app[ch->perm_stat[STAT_WIS]].practice +
	//                   prim_app[ch->perm_stat
	//                            [Class_table[ch->Class].attr_prime]].practice,
	//                   7);

	add_hp = UMAX (2, add_hp);
	add_mana = UMAX (2, add_mana);
	add_move = UMAX (1, add_move);


	if (ch->race == PC_RACE_AVATAR)
	{
		if (ch->pcdata->avatar_type != 2)
		{
			add_hp = (ch->perm_stat[STAT_CON] - 3);
			add_mana = (ch->perm_stat[STAT_INT] - 3);
			add_move = (ch->perm_stat[STAT_DEX] - 3);
		}
		else
		{
			add_hp += 2;
			add_mana += 2;
			add_move += 2;
		}
	}
	if(ch->race == PC_RACE_SWARM)
		add_hp = 1;
	ch->max_hit -= add_hp;
	ch->max_mana -= add_mana;
	ch->max_move -= add_move;
	//IBLIS 5/28/03 - commented out for new xp system
	//  ch->practice -= add_prac;

	if (ch->hit > ch->max_hit)
		ch->hit = ch->max_hit;
	if (ch->mana > ch->max_mana)
		ch->mana = ch->max_mana;
	if (ch->move > ch->max_move)
		ch->move = ch->max_move;

	ch->pcdata->perm_hit -= add_hp;
	ch->pcdata->perm_mana -= add_mana;
	ch->pcdata->perm_move -= add_move;

	//IBLIS 5/28/03 - removed prac info for new xp system
	sprintf (buf,
		"`bYou lose: -%d/%d hp, -%d/%d m, -%d/%d mv",
		add_hp, ch->max_hit,
		add_mana, ch->max_mana, add_move, ch->max_move);
	ch->level--;
	if (ch->level < 10)
	{
		ch->train -= 1;
		if (ch->race == 8)
		{
			ch->train -= 1;
			sprintf (buf + strlen (buf), " -2/%d train", ch->train);
		}
		else
			sprintf (buf + strlen (buf), " -1/%d train", ch->train);
	}

	strcat (buf, ".``\n\r");
	if ( !(ch->race == PC_RACE_AVATAR && ch->pcdata->avatar_type >= 3))
		ch->pcdata->totalxp = ch->pcdata->totalxp - level_cost (ch->level);

	send_to_char (buf, ch);
	return;
}

//Iblis - To deal with necromancer's familiars and gaining levels
void familiar_gain_level(CHAR_DATA *ch, CHAR_DATA *fam)
{
	int Class,add_hp,add_mana,add_move;
	if (ch->pcdata->familiar_type == FAMILIAR_BANSHEE)
	Class = 1;
	else if (ch->pcdata->familiar_type == FAMILIAR_SKELETON)
	Class = 3;
	else Class = 2;

	if (ch->pcdata->familiar_type == FAMILIAR_IMAGINARY_FRIEND)
	{
		add_hp = 1;
		add_mana = 1;
		add_move = 10;
		if (fam->level == 45)
			fam->number_of_attacks = 2;
		else if (fam->level == 90)
			fam->number_of_attacks = 3;
		if (fam->level%10 == 0)
		{
			fam->damroll += 7;
			fam->hitroll += 10;
		}
	}
	else
	{
		add_hp = (3 * number_range (hp_gains[Class][fam->perm_stat[STAT_CON]] [0],
			hp_gains[Class][fam->perm_stat[STAT_CON]][1])
			+ number_range (hp_gains[Class][fam->perm_stat[STAT_STR]] [0],
			hp_gains[Class][fam->perm_stat[STAT_STR]][1])) / 4;
		add_mana = (3 * number_range (mana_gains[Class] [fam->perm_stat[STAT_INT]][0],
			mana_gains[Class][fam->perm_stat [STAT_INT]] [1]) +
			number_range (mana_gains[Class] [fam->perm_stat[STAT_WIS]][0],
			mana_gains[Class][fam->perm_stat[STAT_WIS]][1])) / 4;
		add_move = number_range ((fam->perm_stat[STAT_DEX] + fam->perm_stat[STAT_CON]) / 8,
			(2 * fam->perm_stat[STAT_DEX] + fam->perm_stat[STAT_CON]) / 7);
		add_hp = UMAX (2, add_hp);
		add_mana = UMAX (2, add_mana);
		add_move = UMAX (1, add_move);
	}

	fam->max_hit += add_hp;
	fam->max_mana += add_mana;
	fam->max_move += add_move;
	ch->pcdata->familiar_max_hit += add_hp;
	ch->pcdata->familiar_max_mana += add_mana;
	ch->pcdata->familiar_max_move += add_move;
	fam->level++;
	ch->pcdata->familiar_level++;
	act("$n seems more powerful.",fam,NULL,NULL,TO_ALL);
}






long int gain_exp (CHAR_DATA * ch, long int gain)
{
	unsigned int maxxp;
	short wis = 0;
	long famgain = 0;
	CHAR_DATA *gch;
	char buf[MAX_STRING_LENGTH];

	/* The workaround below is to make the compiler accept 4000000000
	* as an unsigned integer without giving a warning about it, Morgan
	*/

	maxxp = 4000;
	maxxp *= 1000;
	maxxp *= 1000;

	if (IS_NPC (ch))              // || ch->level > LEVEL_HERO)
		return 0;
	if (gain > 0)
	{
		if (IS_CLASS (ch, PC_CLASS_REAVER) && (get_eq_char (ch, WEAR_WIELD_L) ==  NULL && get_eq_char (ch, WEAR_WIELD_R) == NULL))
			gain = 0;
		for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
		{
			if (IS_SET(gch->act2,ACT_FAMILIAR) && !str_cmp(gch->afkmsg,ch->name))
			{
				if (!IS_NPC(ch) && ch->pcdata->all_xp_familiar)
					famgain = gain;
				else famgain = gain / 2;
				if (famgain > ((level_cost (gch->level)) / 10.0))
					famgain = level_cost (gch->level) * .10;
				wis = gch->perm_stat[STAT_WIS];
				if (famgain > 0 && wis > 13)
					famgain = famgain + (famgain * (wis - 13) / 100.0);
				if (famgain > 0 && wis < 13)
					famgain = famgain - (famgain * (13 - wis) / 100.0);
				sprintf(buf,"Your familiar received %ld experience.\n\r",famgain);
				send_to_char(buf,ch);
				gch->exp = gch->exp + famgain;
				ch->pcdata->familiar_exp += famgain;
				while (gch->exp >= level_cost(gch->level) && gch->level < 90)
				{
					ch->pcdata->familiar_exp -= level_cost(gch->level);	  
					gch->exp -= level_cost(gch->level);
					familiar_gain_level(ch,gch);
				}
				if (!IS_NPC(ch) && ch->pcdata->all_xp_familiar)
					gain = 0;
				else gain -= famgain;
			}
		} 
	}



	wis = ch->perm_stat[STAT_WIS];
	//IBLIS 5/23/03 - Give more xp for more Wis
	if (gain > 0 && wis > 13)
		gain = gain + (gain * (wis - 13) / 100.0);
	if (gain > 0 && wis < 13)
		gain = gain - (gain * (13 - wis) / 100.0);

	if (gain < 0 && 0 - gain > ch->exp)
	{
		gain = 0 - ch->exp;
	}
	if (IS_AFFECTED (ch, AFF_CHARM) && gain > 0)
	{
		gain /= 20;
	}
	if (ch->race == PC_RACE_KALIAN)
		gain /= 2;

	if (!(ch->race == PC_RACE_AVATAR && ch->pcdata->avatar_type >= 3))
	{
		ch->pcdata->totalxp = ch->pcdata->totalxp + gain;
		if (ch->pcdata->totalxp > maxxp)
			ch->pcdata->totalxp = maxxp;
	}

//	if ((ch->Class == PC_CLASS_DRUID) && !is_char_in_natural_sector(ch))
//		gain /= 3;

	if (((ch->level == PC_CLASS_DRUID) || ((ch->leader != NULL) && (ch->leader->Class == PC_CLASS_DRUID))) && is_char_in_natural_sector(ch))
		gain *= 2;

	if (gain >= 0)
	{
		sprintf (buf, "You receive %ld experience points.\n\r", gain);
		send_to_char (buf, ch);
	}
	else
	{
		sprintf (buf, "You lost %ld exp.\n\r", 0 - gain);
		send_to_char (buf, ch);
	}
	ch->exp = ch->exp + gain;
	if (ch->exp > maxxp)
		ch->exp = maxxp;
	return gain;
}

void group_gain_exp (CHAR_DATA * ch, long int gain)
{

	if (IS_NPC (ch) || ch->level >= 92)
		return;
	//IBLIS 5/17/03 - Removed autoleveling for the new xp system
	if (ch->pcdata->group_exp + gain < 0)
		ch->pcdata->group_exp = 0;
	else
		ch->pcdata->group_exp += gain;
	return;
}


long int low_gain_exp (CHAR_DATA * ch, long int gain)
{
	char buf[MAX_STRING_LENGTH];
	int wis = 0;
	long famgain=0;
	CHAR_DATA *gch;

	if (IS_NPC (ch))		// || ch->level > LEVEL_HERO)
		return 0;
	if (gain > 0 && !player_rewarding)
	{
		for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
		{
			if (IS_SET(gch->act2,ACT_FAMILIAR) && !str_cmp(gch->afkmsg,ch->name))
			{
				if (!IS_NPC(ch) && ch->pcdata->all_xp_familiar)
					famgain = gain;
				else famgain = gain / 2;
				if (famgain > ((level_cost (gch->level)) / 10.0))
					famgain = level_cost (gch->level) * .10;
				wis = ch->perm_stat[STAT_WIS];
				if (famgain > 0 && wis > 13)
					famgain = famgain + (famgain * (wis - 13) / 100.0);
				if (famgain > 0 && wis < 13)
					famgain = famgain - (famgain * (13 - wis) / 100.0);
				gch->exp = gch->exp + famgain;
				ch->pcdata->familiar_exp += famgain;
				while (gch->exp >= level_cost(gch->level) && gch->level < 90)
				{
					ch->pcdata->familiar_exp -= level_cost(gch->level);
					gch->exp -= level_cost(gch->level);
					familiar_gain_level(ch,gch);
				}  
				if (!IS_NPC(ch) && ch->pcdata->all_xp_familiar)
					gain = 0;
				else gain -= famgain;
			}
		}
	}

	if (!player_rewarding)
	{
		wis = get_curr_stat (ch, STAT_WIS);
		if (gain > 0 && wis > 13)
			gain = gain + (gain * (wis - 13) / 100.0);
		if (gain > 0 && wis < 13)
			gain = gain - (gain * (13 - wis) / 100.0);
	}
	if (gain < 0 && 0 - gain > ch->exp)
	{
		gain = 0 - ch->exp;
	}

	if (IS_AFFECTED (ch, AFF_CHARM) && gain > 0 && !player_rewarding)
	{
		gain /= 20;
	}
	if (ch->race == PC_RACE_KALIAN)
		gain /= 2;
	if (!(ch->race == PC_RACE_AVATAR && ch->pcdata->avatar_type >= 3))
		ch->pcdata->totalxp = ch->pcdata->totalxp + gain;

//	if ((ch->Class == PC_CLASS_DRUID) && !is_char_in_natural_sector(ch))
//		gain /= 3;

	if (((ch->Class == PC_CLASS_DRUID) || ((ch->leader != NULL) && (ch->leader->Class == PC_CLASS_DRUID))) && is_char_in_natural_sector(ch))
		gain *= 2;

	if (gain >= 0)
	{
		sprintf (buf, "You receive %ld experience points.\n\r", gain);
		send_to_char (buf, ch);
	}
	else
	{
		sprintf (buf, "You lost %ld exp.\n\r", 0 - gain);
		send_to_char (buf, ch);
	}
	ch->exp = ch->exp + gain;


	return gain;
}

int hit_gain (CHAR_DATA * ch)
{
	int gain = 0;
	int number = 101;
	OBJ_DATA *tObj;


	if (ch->in_room == NULL || ch->race == PC_RACE_SWARM)
		return 0;


	if (ch->position == POS_STANDING && ch->fighting != NULL)
		ch->position = POS_FIGHTING;
	if (IS_NPC (ch))
	{
		gain += 5 + ch->level;
		if (IS_AFFECTED (ch, AFF_REGENERATION))
			gain *= 2;
		if (is_affected (ch,gsn_festering_boils))
			gain += gain*.20;

			

		switch (ch->position)
		{
		default:
			gain /= 2;
			break;
		case POS_SLEEPING:
			gain *= 2;
			break;
		case POS_SITTING:
		case POS_RESTING:
			gain = 3 * gain / 2;
			break;
		case POS_COMA:
			gain = 0;
			break;
		case POS_FIGHTING:
			gain /= 3;
			break;
		}
	}
	else
	{
		gain = UMAX (3, get_curr_stat (ch, STAT_CON) - 3 + ch->level / 2);
		gain += Class_table[current_Class (ch)].hp_max - 10;
		number = number_percent ();

		if (IS_AFFECTED (ch, AFF_REGENERATION))
			gain *= 2;
			
			// Duiwel - Increased positional gain benefits 6/28/2012
			

		switch (ch->position)
		{
		default:
			gain /= 2;
			break;
		case POS_SLEEPING:
			gain *= 2;
			break;
		case POS_RESTING:
			gain = 3 * gain / 2;
			break;
		case POS_SITTING:
			gain *= 1;
			break;
		case POS_COMA:
			gain = 0;
			break;
		case POS_FIGHTING:
			gain /= 6;
			break;
		}
		
		//Mike 3/10/08 Boost regen a bunch when not blagged
		//Duiwel 6/28/2012  Mike's Code was in the wrong spot, I've moved it, it was also badly written, I've cleaned it up so it works.
		if ((time (NULL) - ch->pcdata->last_fight > 120)) {
			gain += ch->max_hit / 25; 
		}
	
		if (ch->pcdata->condition[COND_HUNGER] == 0)
			gain /= 2;

		if (ch->pcdata->condition[COND_THIRST] == 0)
			gain /= 2;

	}
	
	gain = gain * ch->in_room->heal_rate / 100;

	if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
		gain = gain * ch->on->value[3] / 100;

	if (number < get_skill (ch, gsn_fast_healing))
	{
		gain += number * abs (gain) / 100;
		if (!(ch->in_room->sector_type == SECT_HILLS ||
			ch->in_room->sector_type == SECT_FIELD ||
			ch->in_room->sector_type == SECT_SWAMP ||
			ch->in_room->sector_type == SECT_FOREST ||
			ch->in_room->sector_type == SECT_GRASSLAND))
		{
			if (IS_CLASS (ch, PC_CLASS_RANGER))
				gain *= 2;
		}
		if (ch->hit < ch->max_hit)
			check_improve (ch, gsn_fast_healing, TRUE, 8);
	}

	if (IS_AFFECTED (ch, AFF_POISON))
		if (gain >= 0)
			gain /= 4;

	if (IS_AFFECTED (ch, AFF_PLAGUE))
	{
		if (gain >= 0)
			gain /= 8;
		else
			gain *= 2;
	}
	
	if (is_affected(ch, gsn_insect_plague))
	{
		if (gain >= 0)
		gain /= 8;
	}

	if (IS_AFFECTED (ch, AFF_HASTE) || IS_AFFECTED (ch, AFF_SLOW))
		if (gain >= 0)
			gain /= 2;
	if (ch->race == PC_RACE_LITAN)
	{
		if (IS_OUTSIDE(ch) && (weather_info.sky == SKY_RAINING || weather_info.sky == SKY_LIGHTNING))
			gain /= 2;
		if (IS_OUTSIDE(ch) && weather_info.sky == SKY_CLOUDLESS)
			gain *= 2;
	}


	//
	// Akamai - One of our 'hang' problems happens right here. Seems the
	// objects in the room end up in circular list. Yikes!
	//
	for (tObj = ch->in_room->contents; tObj != NULL; tObj = tObj->next_content)
	{
		if (tObj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE1
			|| tObj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE2)
		{
			act ("The campfire's warmth soothes you.", ch, NULL, NULL, TO_CHAR);
			if (ch->race == PC_RACE_LITAN)
				gain *= 4;
			else gain += (gain / 3);
			//        gain += UMAX (tObj->level / 12, 1);
			// Minax 7-13-02 ^^^ I commented out the original gains above.  Now you gain
			// an extra 1/3 on top of normal gains if a campfire is burning in the room.
			break;
		}
	}
	if (battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner)) && ch->hit >= ch->max_hit && !IS_NPC (ch)
		&& ch->in_room->vnum == 4200 && !IS_IMMORTAL (ch))
		br_transfer (ch);
	return UMIN (gain, ch->max_hit - ch->hit);
}



int mana_gain (CHAR_DATA * ch)
{
	OBJ_DATA *tObj;
	int gain = 0;
	int number = 0;

	if (ch->in_room == NULL)
		return 0;
	// if (ch->pcdata->last_fight > 200) gain += ch->max_mana/10; //Mike 3/10/08 Boost to regen while not blagged.
	if (ch->position == POS_STANDING && ch->fighting != NULL)
		ch->position = POS_FIGHTING;

	if (IS_NPC (ch))
	{
		gain += 5 + ch->level;
		switch (ch->position)
		{
		default:
			gain /= 2;
			break;
		case POS_SLEEPING:
			gain = 3 * gain / 2;
			break;
		case POS_RESTING:
			break;
		case POS_COMA:
			gain = 3*(3*gain/2);
			break;
		case POS_FIGHTING:
			gain /= 3;
			break;
		}
	}
	else
	{
		gain = (get_curr_stat (ch, STAT_WIS)
			+ get_curr_stat (ch, STAT_INT) + ch->level) / 2;
		number = number_percent ();
		if (number < get_skill (ch, gsn_meditation))
		{
			gain += number * gain / 100;
			if (ch->mana < ch->max_mana)
				check_improve (ch, gsn_meditation, TRUE, 8);
		}
		if (!mana_using_Class (ch))
			gain /= 2;

		if (IS_AFFECTED (ch, AFF_REGENERATION))
			gain *= 2;
			
		// Duiwel - Increased positional gain benefits 6/28/2012

		switch (ch->position)
		{
		default:
			gain /= 2;
			break;
		case POS_SLEEPING:
			gain *= 2;
			break;
		case POS_RESTING:
			gain = 3 * gain / 2;
			break;
		case POS_SITTING:
			gain *= 1;
			break;
		case POS_COMA:
			gain *= 3;
			break;
		case POS_FIGHTING:
			gain /= 6;
			break;
		}

		//Mike 3/10/08 Boost to regen while not blagged.
		//Duiwel 6/28/2012  Mike's Code was in the wrong spot, I've moved it, it was also badly written, I've cleaned it up so it works.
		if ((time (NULL) - ch->pcdata->last_fight > 120)) {
			gain += ch->max_mana / 25; 
		}
		
		if (ch->pcdata->condition[COND_HUNGER] == 0)
			gain /= 2;

		if (ch->pcdata->condition[COND_THIRST] == 0)
			gain /= 2;

	}

	gain = gain * ch->in_room->mana_rate / 100;

	for (tObj = ch->in_room->contents; tObj; tObj = tObj->next_content)
	{
		if (tObj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE1 ||
			tObj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE2)
		{
			gain += UMAX (tObj->level / 12, 1);
			break;
		}
	}

	if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
		gain = gain * ch->on->value[4] / 100;

	if (IS_AFFECTED (ch, AFF_POISON))
		gain /= 4;

	if (IS_AFFECTED (ch, AFF_PLAGUE))
		gain /= 8;

	if (IS_AFFECTED (ch, AFF_HASTE) || IS_AFFECTED (ch, AFF_SLOW))
		gain /= 2;

	return UMIN (gain, ch->max_mana - ch->mana);
}



int move_gain (CHAR_DATA * ch)
{
	int gain = 0;
	int number = 0;

	if (ch->in_room == NULL)
		return 0;

	if (IS_NPC (ch))
	{
		gain += ch->level;
	}
	else
	{
		gain = UMAX (25, ch->level);

		number = number_percent ();
		if (number < get_skill (ch, gsn_endurance))
		{
			gain += number * gain / 100;
			if (ch->move < ch->max_move)
				check_improve (ch, gsn_endurance, TRUE, get_curr_stat (ch,
				STAT_CON)
				- 5);
		}


		if (IS_AFFECTED (ch, AFF_REGENERATION))
			gain *= 2;
			
		// Duiwel - Increased positional gain benefits 6/28/2012

		switch (ch->position)
		{
		case POS_SLEEPING:
			gain += get_curr_stat (ch, STAT_DEX) * 2;
			break;
		case POS_RESTING:
			gain += get_curr_stat (ch, STAT_DEX);
			break;
		case POS_SITTING:
			gain *= 1;
			break;
		case POS_COMA:
			gain = 0;
			break;
		}
		
		//Mike 3/10/08 Boost Regen while not blagged.
		//Duiwel 6/28/2012  Mike's Code was in the wrong spot, I've moved it, it was also badly written, I've cleaned it up so it works.
		if ((time (NULL) - ch->pcdata->last_fight > 120)) {
			gain += ch->max_move / 25; 
		}


		if (ch->pcdata->condition[COND_HUNGER] == 0)
			gain /= 2;

		if (ch->pcdata->condition[COND_THIRST] == 0)
			gain /= 2;
	}

	gain = gain * ch->in_room->heal_rate / 100;

	if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
		gain = gain * ch->on->value[3] / 100;

	if (IS_AFFECTED (ch, AFF_POISON))
		gain /= 4;

	if (IS_AFFECTED (ch, AFF_PLAGUE))
		gain /= 8;
	
	if (IS_AFFECTED (ch, AFF_PLAGUE))
		gain /= 8;
		
	if (is_affected(ch, gsn_insect_plague))
		gain /= 8;
		
	if (IS_AFFECTED (ch, AFF_HASTE) || IS_AFFECTED (ch, AFF_SLOW))
		gain /= 2;

	return UMIN (gain, ch->max_move - ch->move);
}


void gain_condition (CHAR_DATA * ch, int iCond, int value)
{
	int condition;

	// Akamai 4/15/99 - Fixed this test so that Level 91/Heros
	// are now subject to starvation - there will be collateral damage
	if (value == 0 || IS_NPC (ch) || ch->level >= 92)
	{
		return;
	}

	if (iCond == COND_PILLS)
	{
		ch->pcdata->condition[iCond] += value;
		if (ch->pcdata->condition[iCond] < 0)
			ch->pcdata->condition[iCond] = 0;
		return;
	}

	//deals with heroes who are eating/drinking something other than pills
	if ( (ch->level >= 91 || ch->race == PC_RACE_LICH) && iCond != COND_DRUNK)
		return;

	if (ch->in_room != NULL && ch->in_room->vnum == ROOM_VNUM_LIMBO)
	{
		return;
	}

	if (ch->desc == NULL)
	{
		return;
	}

	condition = ch->pcdata->condition[iCond];

	ch->pcdata->condition[iCond] = URANGE (-16, condition + value, 48);

	if ((ch->level < LEVEL_BEGIN_STARVING) && (ch->pcdata->condition[iCond] < -4))
	{
		ch->pcdata->condition[iCond] = -4;
	}

	if (ch->pcdata->condition[iCond] < 0 && ch->pcdata->condition[iCond] >= -5)
	{
		switch (iCond)
		{
		case COND_HUNGER:
			send_to_char
				("Your stomach rumbles as you notice you are hungry.\n\r", ch);
			break;

		case COND_THIRST:
			send_to_char
				("Your dry throat aches for liquid refreshment.\n\r", ch);
			break;

		case COND_DRUNK:
			if (condition != 0 && ch->pcdata->condition[iCond] == 0)
				send_to_char ("Unfortunately, you are sober.\n\r", ch);
			break;
		}
	}

	if (ch->pcdata->condition[iCond] < -5)
	{
		switch (iCond)
		{
		case COND_HUNGER:
			send_to_char ("`yYou are starving!\n\r``", ch);
			if (ch->pcdata->condition[iCond] <= -16)
			{
				send_to_char
					("You convulse violently, and die of starvation!\n\r", ch);
				act ("$n convulses violently and dies of starvation!", ch,
					NULL, NULL, TO_ROOM);
				raw_kill (ch, ch);
				if (is_dueling (ch))
				{
					duel_ends (ch);
					// return FALSE;
				}

				if (ch->dueler)
				{
					superduel_ends(ch,ch);

				}

			}
			break;

		case COND_THIRST:
			send_to_char ("`yYou are dehydrating!\n\r``", ch);
			if (ch->pcdata->condition[iCond] <= -16)
			{
				send_to_char ("You have died of dehydration!\n\r", ch);
				act ("$n dies of dehydration!", ch, NULL, NULL, TO_ROOM);
				raw_kill (ch, ch);
				if (is_dueling (ch))
				{
					duel_ends (ch);
					//  return FALSE;
				}

				if (ch->dueler)
				{
					superduel_ends(ch,ch);
					//return FALSE;
				}

			}
			break;
		}
	}

	return;
}

inline void speedwalk_update (void)
{
	CHAR_DATA *sch;
	char dirstr[2];

	dirstr[1] = '\0';

	for (sch = char_list; sch != NULL; sch = sch->next)
	{
		if (IS_NPC (sch))
			continue;

		if (sch->pcdata->speedlen == 0)
			continue;

		sch->pcdata->speedlen--;
		dirstr[0] = sch->pcdata->speedwalk[sch->pcdata->speedlen];
		interpret (sch, dirstr);
		sch->pcdata->speedwalk[sch->pcdata->speedlen] = '\0';
	}
}

void elevator_update (void)
{
	OBJ_DATA *eObj;
	ELEVATOR_DEST_LIST *edest;
	ROOM_INDEX_DATA *from_room, *to_room, *elev_room, *location;
	CHAR_DATA *vch;

	for (eObj = object_list; eObj != NULL; eObj = eObj->next)
	{
		if (eObj->item_type != ITEM_ELEVATOR)
			continue;

		if (eObj->dest_list != NULL)
		{
			if (eObj->in_room->vnum == ROOM_VNUM_ELIMBO)
			{
				elev_room = get_room_index (eObj->value[0]);

				from_room = eObj->in_room;
				if ((to_room = get_room_index (eObj->dest_list->vnum)) == NULL)
					continue;

				obj_from_room (eObj);

				if (from_room != to_room)
					for (vch = to_room->people; vch; vch = vch->next_in_room)
						act (eObj->dest_list->arrival_msg_o, vch, NULL,
						NULL, TO_CHAR);

				if (elev_room != NULL)
					for (vch = elev_room->people; vch; vch = vch->next_in_room)
						act (eObj->dest_list->arrival_msg_i, vch, NULL,
						NULL, TO_CHAR);

				obj_to_room (eObj, to_room);
				edest = eObj->dest_list;
				eObj->dest_list = eObj->dest_list->next;
				free (edest);
			}
			else
			{
				elev_room = get_room_index (eObj->value[0]);

				if (elev_room == NULL)
					continue;

				location = get_room_index (ROOM_VNUM_ELIMBO);

				if (location != NULL)
				{
					from_room = eObj->in_room;
					if ((to_room =
						get_room_index (eObj->dest_list->vnum)) == NULL)
						continue;

					if (from_room != to_room)
						for (vch = from_room->people; vch;
							vch = vch->next_in_room)
							act (eObj->dest_list->departure_msg_o, vch,
							NULL, NULL, TO_CHAR);

					if (elev_room != NULL)
						for (vch = elev_room->people; vch;
							vch = vch->next_in_room)
							act (eObj->dest_list->departure_msg_i, vch,
							NULL, NULL, TO_CHAR);

					obj_from_room (eObj);
					obj_to_room (eObj, location);
				}
			}
		}
	}
}


void mob_trig_update (void)
{
	CHAR_DATA *ch, *ch_next;


	for (ch = char_list; ch != NULL; ch = ch_next)
	{
		ch_next = ch->next;

		if (!IS_NPC (ch) || ch->in_room == NULL || IS_AFFECTED (ch, AFF_CHARM))
			continue;

		if (HAS_SCRIPT (ch))
		{
			script_update (ch, TRIG_EACH_PULSE);
		}
	}
}

void mobile_update (void)
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;
	EXIT_DATA *pexit;
	MEMORY_DATA *memd, *memd_next;
	int door;
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *huntguy = NULL;//, *victim;
	int scentfound = FALSE;
	ROOM_INDEX_DATA *sRoom;
	SCENT_DATA *sc;
	int xx;
	int dir = 0;

	for (ch = char_list; ch != NULL; ch = ch_next)
	{
		ch_next = ch->next;
		if (IS_NPC (ch) && ch->wait > 0 && ch->fighting == NULL && ch->position != POS_FIGHTING)
		{
			ch->wait = UMAX (0, ch->wait - PULSE_VIOLENCE);
		}
		if (!IS_NPC(ch))
		{
			check_gravity_char(ch);
		}
		if (!IS_NPC (ch) || ch->in_room == NULL || IS_AFFECTED (ch, AFF_CHARM) || ch->stunned > 0)
			continue;

		if (ch->in_room->area->empty)
			continue;
		ch->wait = UMAX (0, ch->wait-1);
		/*      if (IS_SET (ch->act, ACT_SMART_HUNT) && ch->in_room != NULL &&
		IS_SET (ch->in_room->room_flags, ROOM_SAFE))
		{
		for (victim = ch->in_room->people; victim; victim =
		victim->next_in_room)
		{
		if (is_hunting (ch, victim) && !IS_AFFECTED (victim,
		AFF_CAMOUFLAGE))
		{
		dir = number_range (0, 5);
		if (ch->in_room->exit[dir] &&
		!IS_SET (ch->in_room->exit[dir]->exit_info, EX_CLOSED))
		{
		sprintf (buf, "%s %s", victim->name, dir_name[dir]);
		do_push (ch, buf);
		break;
		}
		}
		}
		}*/ 
		/* Above is commented out because you can't push in safe rooms anyways */
		/* Iblis - 2/09/04 - Updated some logicality here.  Mobs can't walk out
		* of battle, can't wander around when entangled/slept/claspsed/webbed etc */

		if (ch->fighting == NULL && (check_clasp(ch)) 
			&& (check_entangle(ch)) && (check_web(ch)))
		{
			if (ch->position != POS_STANDING && ch->wait <= 0)
			{
				if (ch->start_pos == POS_STANDING)
					do_stand(ch,"");
				if (ch->position == POS_SLEEPING && ch->start_pos != POS_SLEEPING)
					do_wake(ch,"");
				if (ch->position == POS_COMA && ch->start_pos != POS_COMA)
					do_wake(ch,"");
				if (ch->position != POS_STANDING)
					continue;
			}
			if (ch->huntto == 0 && (IS_SET (ch->act, ACT_HUNT) ||
				IS_SET (ch->act, ACT_SMART_HUNT)))
			{
				for (memd = ch->hunt_memory; memd; memd = memd_next)
				{
					memd_next = memd->next_memory;

					if (memd->player->in_room != NULL && ch->in_room != NULL)
						if (memd->player->in_room->area == ch->in_room->area)
						{
							if (memd->player->in_room != ch->in_room)
							{
								ch->huntto = memd->player->in_room->vnum;
								break;
							}
							else
								ch->huntto = 0;
						}
				}
			}

			if (ch->huntto > 0)
			{
				ROOM_INDEX_DATA *pRoom;

				if ((pRoom = get_room_index (ch->huntto)) != NULL)
				{
					dir = find_first_step (ch->in_room, pRoom, ch);
					if (ch->in_room->exit[dir])
					{
						sRoom = ch->in_room->exit[dir]->u1.to_room;

						if (sRoom != NULL)
							for (sc = sRoom->scents; sc; sc = sc->next_in_room)
							{
								if (is_hunting (ch, sc->player))
								{
									scentfound = TRUE;
									break;
								}
							}
					}
				}

				if (!scentfound)
				{
					for (xx = 0; xx < MAX_DIR; xx++)
					{
						if (!ch->in_room->exit[xx])
							continue;

						sRoom = ch->in_room->exit[xx]->u1.to_room;

						if (sRoom != NULL)
							for (sc = sRoom->scents; sc; sc = sc->next_in_room)
							{
								if (is_hunting (ch, sc->player))
								{
									scentfound = TRUE;
									dir = xx;
									break;
								}
							}

							if (scentfound)
								break;
					}
				}

				if (!scentfound)
					dir = number_range (0, MAX_DIR - 1);

				if (dir >= 0)
				{
					if (ch->in_room->exit[dir])
					{
						if (IS_SET (ch->in_room->exit[dir]->exit_info,
							EX_CLOSED) &&
							IS_SET (ch->act, ACT_SMART_HUNT))
						{
							sprintf (buf, "unlock %s", dir_name[dir]);
							interpret (ch, buf);
							sprintf (buf, "open %s", dir_name[dir]);
							interpret (ch, buf);
						}
						if (!(IS_SET(ch->act2,ACT_STAY_SECTOR) && ch->in_room->exit[dir]->u1.to_room &&
							ch->in_room->exit[dir]->u1.to_room->sector_type != ch->in_room->sector_type))
							move_char (ch, dir, FALSE);
					}
					if (ch->in_room == pRoom)
						ch->huntto = 0;

					if (huntguy != NULL && huntguy->in_room == ch->in_room)
						ch->huntto = 0;
				}
				else
					ch->huntto = 0;
			}


			if (ch->walkto > 0)
			{
				int dir;
				ROOM_INDEX_DATA *pRoom;


				if ((pRoom = get_room_index (ch->walkto)) != NULL)
				{
					dir = find_first_step (ch->in_room, pRoom, ch);

					if (dir < 0)
					{
						do_emote (ch, "has left.");
						char_from_room (ch);
						char_to_room (ch, pRoom);
						ch->walkto = 0;
					}
					else
					{

						if (!(IS_SET(ch->act2,ACT_STAY_SECTOR) && ch->in_room && ch->in_room->exit[dir] && ch->in_room->exit[dir]->u1.to_room &&
							ch->in_room->exit[dir]->u1.to_room->sector_type != ch->in_room->sector_type))
							move_char (ch, dir, FALSE);
						if (ch->in_room == pRoom)
							ch->walkto = 0;
					}
				}
				else
					ch->walkto = 0;
			}
		}

		if (ch->spec_fun != 0)
		{
			if ((*ch->spec_fun) (ch))
				continue;
		}

		if (ch->pIndexData->pShop != NULL)
			if ((ch->gold * 100 + ch->silver) < ch->pIndexData->wealth)
			{
				ch->gold +=
					ch->pIndexData->wealth * number_range (1, 20) / 5000000;
				ch->silver +=
					ch->pIndexData->wealth * number_range (1, 20) / 50000;
			}

			if (ch->position != POS_STANDING || ch->fighting != NULL)
				continue;

			if (IS_SET (ch->act, ACT_SCAVENGER)
				&& ch->in_room->contents != NULL && number_bits (6) == 0)
			{
				OBJ_DATA *obj;
				OBJ_DATA *obj_best;
				int max;

				max = 1;
				obj_best = 0;
				for (obj = ch->in_room->contents; obj; obj = obj->next_content)
				{
					if (CAN_WEAR (obj, ITEM_TAKE) && can_loot (ch, obj)
						&& obj->cost > max && obj->cost > 0)
					{
						if ((obj->item_type == ITEM_FURNITURE || obj->item_type == ITEM_RAFT) &&
							count_users (obj) > 0)
							continue;
						if (obj->item_type == ITEM_CTRANSPORT)
							continue;
						obj_best = obj;
						max = obj->cost;
					}
				}

				if (obj_best && obj_best->in_room != NULL)
				{
					obj_from_room (obj_best);
					obj_to_char (obj_best, ch);
					act ("$n gets $p.", ch, obj_best, NULL, TO_ROOM);
				}
			}

			if (!IS_SET (ch->act, ACT_SENTINEL)
				&& ch->position != POS_TETHERED
				&& number_bits (3) == 0
				&& (door = number_bits (5)) <= 5
				&& (pexit = ch->in_room->exit[door]) != NULL
				&& pexit->u1.to_room != NULL
				&& !IS_SET (pexit->exit_info, EX_CLOSED)
				&& !IS_SET (pexit->u1.to_room->room_flags, ROOM_NO_MOB)
				&& ch->position >= POS_STANDING
				&& !is_campsite (pexit->u1.to_room)
				&& (!IS_SET (ch->act, ACT_STAY_AREA)
				|| pexit->u1.to_room->area == ch->in_room->area)
				&& (!IS_SET (ch->act, ACT_OUTDOORS)
				|| !IS_SET (pexit->u1.to_room->room_flags, ROOM_INDOORS))
				&& (check_clasp(ch))
				&& (check_entangle(ch))
				&& (check_web(ch))
				&& (!IS_SET (ch->act, ACT_INDOORS)
				|| IS_SET (pexit->u1.to_room->room_flags, ROOM_INDOORS)))
			{
				if (!(IS_SET(ch->act2,ACT_STAY_SECTOR) && pexit->u1.to_room &&
					ch->in_room && pexit->u1.to_room->sector_type != ch->in_room->sector_type))
					move_char (ch, door, FALSE);
			}
	}

	return;
}


void scent_update (void)
{
	SCENT_DATA *sc, *sc_next;

	for (sc = scents; sc; sc = sc_next)
	{
		sc_next = sc->next;

		if (sc->player->in_room != sc->in_room)
			switch (sc->scent_level)
		{
			default:
				extract_scent (sc);
				break;
			case SCENT_STRONG:
			case SCENT_NOTICEABLE:
				sc->scent_level--;
		}
	}
}

void weather_update (void)
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	int diff;
	CHAR_DATA *ch;

	buf[0] = '\0';

	switch (++time_info.hour)
	{
	case 5:
		weather_info.sunlight = SUN_LIGHT;
		strcat (buf,
			"The subtle lightening of the sky's canvas of colors marks the beginning of another day.\n\r");
		for (ch = char_list; ch != NULL; ch = ch->next)
		{
			if (IS_NPC(ch) || !ch->in_room)
				continue;
			if (ch->race == PC_RACE_KALIAN)
			{
				if (is_affected(ch,gsn_jurgnation))
				{
					affect_strip (ch, gsn_jurgnation);
					send_to_char("Night approaches, and you feel less weary.\n\r", ch);
				}
				if (is_affected(ch,gsn_jalknation))
				{
					affect_strip (ch, gsn_jalknation);
					send_to_char ("Dawn approaches, and your invigoration subsides.\n\r",ch);
				}
			}
		}
		break;

	case 6:
		weather_info.sunlight = SUN_RISE;
		strcat (buf,
			"From the east, the rays of the sun unfold to encompass the horizon.\n\r");
		for (ch = char_list; ch != NULL; ch = ch->next)
		{
			if (IS_NPC(ch) || !ch->in_room)
				continue;
			if (ch->race == PC_RACE_KALIAN)
			{
				AFFECT_DATA af;
				af.where = TO_AFFECTS;
				af.type = gsn_jurgnation;
				af.level = 4;
				af.duration = 1;
				af.location = APPLY_STR;
				af.modifier = -3;
				af.bitvector = 0;
				af.permaff = FALSE;
				af.composition = FALSE;
				af.comp_name = str_dup ("");
				affect_to_char (ch, &af);
				send_to_char("The daytime begins to wear you down.\n\r",ch);
			}
		}
		break;

	case 12:
		strcat (buf,
			"The prominent noon rays almost stifle you with their permeating density.\n\r");
		break;

	case 19:
		weather_info.sunlight = SUN_SET;
		strcat (buf,
			"Rich magenta streaks inflamed patterns in the sky as the sun slowly fades into the horizon.\n\r");
		for (ch = char_list; ch != NULL; ch = ch->next)
		{
			if (IS_NPC(ch) || !ch->in_room)
				continue;
			if (ch->race == PC_RACE_KALIAN)
			{
				if (is_affected(ch,gsn_jurgnation))
				{
					affect_strip (ch, gsn_jurgnation);
					send_to_char("Night approaches, and you feel less weary.\n\r", ch);
				}
				if (is_affected(ch,gsn_jalknation))
				{
					affect_strip (ch, gsn_jalknation);
					send_to_char("Dawn approaches, and your invigoration subsides.\n\r",ch);
				}
			}
		}
		break;

	case 20:
		weather_info.sunlight = SUN_DARK;
		strcat (buf,
			"Twilight fades to ink as the blanket of night ripples across the land.\n\r");
		break;


	case 24:
		time_info.hour = 0;
		time_info.day++;
		for (ch = char_list;ch != NULL; ch = ch->next)
		{
			if (IS_NPC(ch) || !ch->in_room)
				continue;
			if (ch->race == PC_RACE_KALIAN)
			{
				AFFECT_DATA af;
				af.where = TO_AFFECTS;
				af.type = gsn_jalknation;
				af.level = 4;
				af.duration = 1;
				af.location = APPLY_DAMROLL;
				af.modifier = ch->level/10;
				af.bitvector = 0;
				af.permaff = FALSE;
				af.composition = FALSE;
				af.comp_name = str_dup ("");
				affect_to_char (ch, &af);
				af.location = APPLY_HITROLL;
				affect_to_char (ch, &af);
				send_to_char ("You feel invigorated by the night.\n\r", ch);
			}
		}
		break;
	}

	if (time_info.day >= 35)
	{
		time_info.day = 0;
		time_info.month++;
	}

	if (time_info.month >= 17)
	{
		time_info.month = 0;
		time_info.year++;
	}

	if (time_info.month >= 9 && time_info.month <= 16)
		diff = weather_info.mmhg > 985 ? -2 : 2;
	else
		diff = weather_info.mmhg > 1015 ? -2 : 2;

	weather_info.change += diff * dice (1, 4) + dice (2, 6) - dice (2, 6);
	weather_info.change = UMAX (weather_info.change, -12);
	weather_info.change = UMIN (weather_info.change, 12);

	weather_info.mmhg += weather_info.change;
	weather_info.mmhg = UMAX (weather_info.mmhg, 960);
	weather_info.mmhg = UMIN (weather_info.mmhg, 1040);

	switch (weather_info.sky)
	{
	default:
		bug ("Weather_update: bad sky %d.", weather_info.sky);
		weather_info.sky = SKY_CLOUDLESS;
		break;

	case SKY_CLOUDLESS:
		if (weather_info.mmhg < 990
			|| (weather_info.mmhg < 1010 && number_bits (2) == 0))
		{
			strcat (buf,
				"Grey patches of clouds grow larger as they converge in the sky just above.\n\r");
			weather_info.sky = SKY_CLOUDY;
		}
		break;

	case SKY_CLOUDY:
		if (weather_info.mmhg < 970
			|| (weather_info.mmhg < 990 && number_bits (2) == 0))
		{
			strcat (buf,
				"Ominous clouds move swiftly across the landscape and let loose a torrential downpour upon the lands.\n\r");
			weather_info.sky = SKY_RAINING;
			for (ch = char_list;ch != NULL; ch = ch->next)
			{
				if (IS_NPC(ch))
					continue;
				if (ch->race != PC_RACE_LITAN)
					continue;
				if (is_affected (ch, gsn_aquatitus))
				{
					affect_strip(ch,gsn_aquatitus);
					send_to_char("As the rain blocks out the sun, you feel yourself slow down.\n\r",ch);
				}
				if (ch->pcdata->flaming && IS_OUTSIDE(ch))
				{
					OBJ_DATA *obj;
					ch->pcdata->flaming = 0;
					if (!((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL
						&& obj->item_type == ITEM_LIGHT
						&& obj->value[2] != 0 && ch->in_room != NULL))
						ch->in_room->light--;

					if (is_affected(ch,gsn_fireshield))
						affect_strip(ch,gsn_fireshield);
					send_to_char("The flame surrounding you fizzles out as the rain pours down.\n\r",ch);
				}

			}
		}

		if (weather_info.mmhg > 1030 && number_bits (2) == 0)
		{
			strcat (buf,
				"Silver streaks of light break through the dark blanket of clouds as the sky clears up.\n\r");
			weather_info.sky = SKY_CLOUDLESS;
		}
		break;

	case SKY_RAINING:
		if (weather_info.mmhg < 970 && number_bits (2) == 0)
		{
			strcat (buf, "Lightning flashes in the sky.\n\r");
			weather_info.sky = SKY_LIGHTNING;
		}
		if (weather_info.mmhg > 1030
			|| (weather_info.mmhg > 1010 && number_bits (2) == 0))
		{
			strcat (buf, "The rain stopped.\n\r");
			weather_info.sky = SKY_CLOUDY;
		}
		break;

	case SKY_LIGHTNING:
		if (weather_info.mmhg > 1010
			|| (weather_info.mmhg > 990 && number_bits (2) == 0))
		{
			strcat (buf, "The lightning has stopped.\n\r");
			weather_info.sky = SKY_RAINING;
			break;
		}
		break;
	}

	if (buf[0] != '\0')
	{
		for (d = descriptor_list; d != NULL; d = d->next)
		{
			if (d->connected == CON_PLAYING && IS_OUTSIDE (d->character)
				&& IS_AWAKE (d->character) 
				&& d->character->in_room && d->character->in_room->sector_type != SECT_UNDERWATER &&
				d->pEdit == NULL && d->editor == 0)
				send_to_char (buf, d->character);
		}
	}

	return;
}

void chant_update (void)
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;

	for (ch = char_list; ch != NULL; ch = ch_next)
	{
		ch_next = ch->next;

		// if they are in some non-room skip them
		if (ch->in_room == NULL)
			continue;

		if (!IS_NPC (ch) && IS_CLASS (ch, PC_CLASS_REAVER))
		{
			if (ch->pcdata->reaver_chant != -1)
				if (ch->pcdata->chant_wait-- <= 0)
				{
					perform_chant (ch);
					ch->pcdata->reaver_chant = -1;
					ch->pcdata->chant_wait = 0;
				}
		}
	}

}

//Iblis - A tiny update function to deal with new-style knock skill
void knock_update (void)
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;

	for (ch = char_list; ch != NULL; ch = ch_next)
	{
		ch_next = ch->next;

		// if they are in some non-room skip them
		if (ch->in_room == NULL)
			continue;

		if (!IS_NPC (ch) && ch->pcdata->knock_time)
			--ch->pcdata->knock_time;
	}

}

//Iblis - an object trigger update function to deal with lag on object triggers. 
void obj_trig_second_update (void)
{
	OBJ_TRIG *ot;
	OBJ_TRIG *ot_next;
	for (ot = ot_list; ot != NULL; ot = ot_next)
	{
		ot_next = ot->next;
		if (--ot->seconds_left < 1)
		{
			if (ot->statement_on == 0)
			{
				ot->statement_on = 1;
				activate_trigger_statement(0,ot,ot->obj_on,ot->ch,ot->victim);
			}
			else activate_trigger_statement(ot->statement_on,ot,ot->obj_on,ot->ch,ot->victim); 
		}	
	}
}

//Iblis - the update function to deal with falcon delivery
void falcon_update (void)
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;

	for (ch = char_list; ch != NULL; ch = ch_next)
	{
		ch_next = ch->next;
		// if they are in some non-room skip them
		if (ch->in_room == NULL)
			continue;

		if (!IS_NPC (ch) && ch->pcdata)
		{
			if (ch->pcdata->falcon_wait == 0)
				continue;
			if (ch->pcdata->falcon_wait > 1)
				ch->pcdata->falcon_wait--;
			else if (ch->pcdata->falcon_wait < -1)
				ch->pcdata->falcon_wait++;
			else if (ch->pcdata->falcon_wait == 1)
				delivery_part2 (ch);
			else if (ch->pcdata->falcon_wait == -1)
			{
				delivery_part3 (ch);
				ch->pcdata->falcon_wait = 0;
			}
		}
	}

}

void char_update (void)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;
	CHAR_DATA *ch_quit;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;
	AFFECT_DATA af;
	int tickdam, level=0;

	ch_quit = NULL;

	save_number++;

	if (save_number > 29)
		save_number = 0;

	for (ch = char_list; ch != NULL; ch = ch_next)
	{
		ch_next = ch->next;

		// if they are in some non-room skip them
		if (ch->in_room == NULL)
			continue;

		// Akamai 5/7/99 - Players who are wanted/jailed often go link-dead
		// as a means of serving their time. This code will no longer update
		// chars who are link-dead - no starvation, no thirst, no nothing
		// when link-dead.
		//
		// Currently the code in comm.c function close_socket() sets the
		// characters ch->desc to NULL when it detects a lost link
		//
		// This is getting commented out by me, Morgan cause sucking LD's
		// into void is important.
		// if(!IS_NPC(ch) && (ch->desc == NULL) ) continue;

		if (IS_SET (ch->in_room->room_flags, ROOM_SAFE) && ch->fighting != NULL)
			stop_fighting (ch, TRUE);

		if (IS_NPC (ch))
		{
			if ((is_affected (ch, skill_lookup ("web")) ||
				is_affected (ch, gsn_entangle)) && (number_range (0, 1) == 0))
				move_char (ch, number_range (0, 5), FALSE);

			if (IS_SET (ch->act, ACT_WIMPY) && ch->wimpy != 0)
			{
				ROOM_INDEX_DATA *loc;
				CHAR_DATA *cl;
				int x, dont;

				for (x = 0; x < 6; x++)
				{
					if (ch->in_room->exit[x] == NULL)
						continue;

					if ((loc = ch->in_room->exit[x]->u1.to_room) == NULL)
						continue;

					dont = 0;

					if (ch->last_fought != NULL)
						for (cl = loc->people; cl != NULL; cl = cl->next_in_room)
						{
							if (cl == ch->last_fought)
							{
								dont = 1;
								break;
							}
						}

						if (dont)
							continue;

						move_char (ch, x, FALSE);
						ch->wimpy--;
				}
			}

			if (IS_SET (ch->act2, ACT_ILLUSION))
			{
				if (--ch->ticks_remaining < 0)
				{
					act("An illusion dissapates.",ch,NULL,NULL,TO_ROOM);
					raw_kill (ch, ch);
				}
			}

			if (IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_DARKHOUNDS)
			{
				if (--ch->ticks_remaining < 0)
				{
					act("`aA darkhound returns to the underworld.``",ch,NULL,NULL,TO_ROOM);
					extract_char (ch, TRUE);
				}
			}
			if (IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_VIVIFY)
			{
				if (ch->ticks_remaining > 0)
				{
					if (--ch->ticks_remaining <= 0)
					{
						act("And with that, $n is gone.",ch,NULL,NULL,TO_ROOM);
						extract_char (ch, TRUE);
					}
				}
			}

			if (IS_SET(ch->act2, ACT_PUPPET))
			{
				if (--ch->ticks_remaining < 0)
				{
					if (IS_NPC(ch))
					{
						if (ch->pIndexData->vnum == MOB_VNUM_BONE_PUPPET)
							act("A bone puppet returns to the ground.",ch,NULL,NULL,TO_ALL);
						else if (ch->pIndexData->vnum == MOB_VNUM_FROG
							|| ch->pIndexData->vnum == MOB_VNUM_WARPED_CAT
							|| ch->pIndexData->vnum == MOB_VNUM_WARPED_OWL)
						{
							act("$n dissappears in a puff of purple smoke, but another being appears in $s place.",ch,NULL,NULL,TO_ROOM);
							send_to_char("Your spell can't last forever, and you are returned to your former self.\n\r",ch);
							if (ch != NULL)
							{
								OBJ_DATA *obj,*obj_next;
								for (obj = ch->carrying; obj; obj = obj_next)
								{
									obj_next = obj->next_content;
									unequip_char (ch, obj);
									obj_from_char (obj);
									obj_to_room (obj,ch->in_room);
									act ("$p falls to the ground", ch, obj, NULL, TO_ALL);
								}
							}
							if (ch->desc && ch->desc->original)
							{
								char_from_room(ch->desc->original);
								char_to_room(ch->desc->original,ch->in_room);
							}
						}
						else if (is_shapechanged_druid(ch))
							shapechange_return(ch, 0);
						
					}
					extract_char(ch,TRUE);  
				}
			}


			//Iblis - 4/16/04 - Modified this greatly, completely changing how hiring is done
			if (IS_SET (ch->recruit_flags, RECRUIT_KEEPER))
			{
				if (ch->recruit_value[2] != -1)
					ch->recruit_value[2]--;
				ch->recruit_value[1]--;
				//This below, rather than a single value counting every 10 ticks, will hopefully
				//stagger the reading from pfiles to subtract wages, hopefully lessenning lag
				if (ch->recruit_value[1] % 10 == 0)
				{
					ch->recruit_value[1] = 9;
					interpret(ch,"oocnote post");
				}


				if (ch->recruit_value[2] == 0)
				{
					act ("$n exits the store, $s shift is over.",
						ch, NULL, NULL, TO_ROOM);
					interpret(ch,"oocnote post");
					close_store(ch->in_room);
					extract_char (ch, TRUE);
					continue;
				}
			}
		}



		if (IS_NPC (ch) && ch->color == 42 && ch->wait <= 0)
		{
			do_stand (ch, "");

			if (ch->position == POS_STANDING)
				ch->color = 0;
		}


		if (IS_NPC (ch) && ch->mood != 8)
		{
			if (ch->mood < ch->pIndexData->default_mood)
				ch->mood++;
			else if (ch->mood > ch->pIndexData->default_mood)
				ch->mood--;
		}
		// Check is shutup should be up
		if (!IS_NPC (ch) && IS_SET(ch->comm2,COMM_SHUTUP))
		{
			//  char buf[MAX_STRING_LENGTH];
			//sprintf(buf,"%s - %ld",ch->name,ch->pcdata->nochan_ticks);
			if (ch->pcdata->shutup_ticks > 1)
			{
				ch->pcdata->shutup_ticks = ch->pcdata->shutup_ticks - 1;
			}
			else if (ch->pcdata->shutup_ticks == 1)
			{
				do_shutup(ch,ch->name);
			}
		}
		//Iblis 10-30-03 Check if Nochan should be up
		if (!IS_NPC (ch) && IS_SET(ch->comm,COMM_NOCHANNELS))
		{
			//  char buf[MAX_STRING_LENGTH];
			//sprintf(buf,"%s - %ld",ch->name,ch->pcdata->nochan_ticks);
			if (ch->pcdata->nochan_ticks > 1)
			{
				ch->pcdata->nochan_ticks = ch->pcdata->nochan_ticks - 1;
			}
			else if (ch->pcdata->nochan_ticks == 1)
			{
				do_nochannels(ch,ch->name);
			}
		}

		//Iblis AutoQuest stuff
		if (!IS_NPC(ch) && ch->pcdata->quest_ticks != 0)
		{
			if (ch->pcdata->quest_ticks > 0)
			{
				--ch->pcdata->quest_ticks;
				if (ch->pcdata->quest_ticks == 0)
				{
					if (ch->pcdata->quest_obj != NULL)
					{
						send_to_char("You realize that you have not finished your quest in time, and feel dejected.\n\r",ch);
						extract_obj(ch->pcdata->quest_obj);
						ch->pcdata->quest_obj = NULL;
						//ch->pcdata->nogate = FALSE;
					}
					if (ch->pcdata->quest_mob != NULL)
					{
						if (ch->pcdata->quest_mob != ch)
							extract_char(ch->pcdata->quest_mob,TRUE);
						send_to_char("You realize that you have not finished your quest in time, and feel dejected.\n\r",ch);
						ch->pcdata->quest_mob = NULL;
						ch->pcdata->quest_obj = NULL;
						ch->pcdata->quest_ticks = -10;
						if (IS_IMMORTAL(ch))
							ch->pcdata->quest_ticks = 0;
						//ch->pcdata->nogate = FALSE;
						ch->pcdata->qtype = 0;
					}                  
				}
			}
			else ++ch->pcdata->quest_ticks;
		}

		// Druid Satiate skill
		if (!IS_NPC(ch) && (ch->Class == PC_CLASS_DRUID) && is_char_in_natural_sector(ch)) {
			if (check_satiate(ch)) {
				ch->pcdata->condition[COND_HUNGER] = 48;
				ch->pcdata->condition[COND_THIRST] = 48;
				ch->pcdata->condition[COND_FULL] = 48;
				send_to_char("Nature gives you the gift of sustainment.\n\r",ch);
			}
		}

		if (!IS_NPC (ch) && ch->race == PC_RACE_KALIAN && time_info.hour >= 5 && time_info.hour < 19 &&
			IS_OUTSIDE(ch))
		{
			if (number_range(1,20) == 14 && get_position(ch) > POS_SLEEPING && !is_affected(ch,gsn_likobe))
			{
				AFFECT_DATA af;
				af.where = TO_AFFECTS;
				af.type = gsn_likobe;
				af.level = 50;
				af.duration = 2;
				af.location = APPLY_DEX;
				af.modifier = -2;
				af.bitvector = 0;
				af.permaff = FALSE;
				af.composition = FALSE;
				af.comp_name = str_dup ("");
				affect_to_char (ch, &af);
				send_to_char("The sunlight burns you and slows you down.\n\r",ch);
			}
		}

		//12-29-03 Iblis - By Minax's request, if you're overweight you can't fly
		if (IS_AFFECTED (ch, AFF_FLYING) && get_carry_weight (ch) > can_carry_w (ch))
		{
			send_to_char("Your fat ass can't fly any more.\n\r",ch);
			do_land(ch,"");
			continue;
		}


		if (!IS_NPC(ch) && ch->race == PC_RACE_NIDAE)
		{
			if (ch->in_room && (ch->in_room->sector_type != SECT_UNDERWATER 
				&& ch->in_room->sector_type != SECT_WATER_OCEAN))
			{
				if (!IS_AFFECTED(ch,AFF_SLOW))
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

				if (++ch->pcdata->oow_ticks % 10 == 0 && ch->pcdata->oow_ticks != 10)
				{
					crit_strike_possible = FALSE;
					damage(ch, ch, 50, 0, DAM_UNIQUE, FALSE);
					crit_strike_possible = TRUE;
					send_to_char("You feel your life drain away as the wind dries your skin.\n\r",ch);
				}
			}
			else if (ch->in_room && (ch->in_room->sector_type == SECT_UNDERWATER
				|| ch->in_room->sector_type == SECT_WATER_OCEAN))
			{
				ch->pcdata->oow_ticks = 0;
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

		}

		if (!IS_NPC(ch) && ch->pcdata->corpse_timer > 0)
		{
			if (--ch->pcdata->corpse_timer == 0)
			{
				delete_player_corpse(ch->pcdata->corpse);
				obj_from_room(ch->pcdata->corpse);
				obj_to_room(ch->pcdata->corpse, ch->pcdata->corpse_to_room);
				save_player_corpse(ch->pcdata->corpse);
				if (ch->pcdata->corpse_to_room->people)
					act("$p stumbles up to you and falls down in a heap.",
					ch->pcdata->corpse_to_room->people,ch->pcdata->corpse,NULL,TO_ALL);
				ch->pcdata->corpse = NULL;
				ch->pcdata->corpse_to_room = NULL;
			}
		}



		if (!IS_NPC (ch) && IS_CLASS (ch, PC_CLASS_PALADIN) && ch->alignment < 200
			&& !ch->pcdata->lost_prayers)
		{
			send_to_char ("Your god has forsaken you.\n\r", ch);
			ch->pcdata->lost_prayers = TRUE;
		}
		// Minax--If a reaver goes good, he's fucked!

		if (!IS_NPC (ch) && IS_CLASS (ch, PC_CLASS_REAVER) && ch->alignment > 0
			&& (get_eq_char (ch, WEAR_WIELD_L) != NULL
			|| get_eq_char (ch, WEAR_WIELD_R) != NULL))
		{
			act ("`iYou are struck down as a traitor by the God of evil!``",
				ch, NULL, NULL, TO_CHAR);
			act ("`i$n is struck down as a traitor by the God of evil!``",
				ch, NULL, NULL, TO_ROOM);
			raw_kill (ch, ch);
			if (is_dueling (ch))
			{
				duel_ends (ch);
			}
			if (ch->dueler)
			{
				superduel_ends(ch,ch);
			}

			continue;
		}

		//Iblis 10/04/04 - Chaos Jester on tick stuff
		if (!IS_NPC (ch) && ch->Class == PC_CLASS_CHAOS_JESTER)
		{
			bool random_done=FALSE;
			int randint = 0;
			CHAR_DATA *mch=NULL;
			//	  if (ch->Class == PC_CLASS_CHAOS_JESTER)
			//{
			int chance = get_skill(ch,gsn_death_spasms);
			if (is_affected(ch,gsn_death_spasms))
			{
				if (ch->hit >= ch->max_hit*.20)
				{
					send_to_char("You feel weaker.\n\r",ch);
					affect_strip(ch,gsn_death_spasms);
				}
			}
			else
			{
				if (ch->hit <= ch->max_hit *.10*(chance/100.0))
					death_spasms(ch);
			}
			//	    }


			//Personality change
			if (number_percent() == 43)
			{
				randint = number_range(1,3);
				while (randint == ch->pcdata->personality)
					randint = number_range(1,3);
				ch->pcdata->personality = randint;
				switch (ch->pcdata->personality)
				{
				case PERS_NORMAL: send_to_char("You feel like your \"normal\" self again.\n\r",ch);break;
				case PERS_AGGRESSIVE: 
					send_to_char("You let the angry side of you take control.\n\r",ch);

					break;
				case PERS_PASSIVE: send_to_char("You let the peaceful side of you take control.\n\r",ch);break;
				}
			}

			//Lower their alignment
			change_alignment(ch,-10);

			//Random forcing to do things
			//ATTACK PLAYER
			if (!random_done && (number_percent() <= (3+(ch->pcdata->personality==PERS_AGGRESSIVE)?2:((ch->pcdata->personality==PERS_PASSIVE)?-1:0) + ((is_player_in_group(ch))?3:0))))
			{
				mch = get_random_player_room(ch);
				if (mch != NULL)
				{
					send_to_char("The voices force you to attack!\n\r",ch);
					if (ch->position < POS_FIGHTING)
						do_stand(ch,"");
					if (ch->fighting && !is_safe(ch,mch))
						ch->fighting = mch;
					else do_kill(ch,mch->name);
					random_done = TRUE;
				}
			}

			//ATTACK MOB
			if (!random_done && (number_percent() <= (5+(ch->pcdata->personality==PERS_AGGRESSIVE)?2:((ch->pcdata->personality==PERS_PASSIVE)?-2:0))))
			{
				mch = get_random_mob_room(ch);
				if (mch != NULL)
				{
					send_to_char("The voices force you to attack!\n\r",ch);
					if (ch->position < POS_FIGHTING)
						do_stand(ch,"");
					if (ch->fighting && !is_safe(ch,mch))
						ch->fighting = mch;
					else do_kill(ch,mch->name);
					random_done = TRUE;
				}
			}

			//STEAL PLAYER
			if (!random_done && (number_percent() <= (2 + ((is_player_in_group(ch))?3:0))))
			{
				//	    char buf[MAX_STRING_LENGTH];
				mch = get_random_player_room(ch);
				if (mch != NULL)
				{
					if (number_range(1,5) == 2)
						sprintf(buf,"coins %s",mch->name);
					else sprintf(buf,"%d. %s",number_range(1,25),mch->name);
					random_done = TRUE;
					if (ch->position < POS_FIGHTING)
						do_stand(ch,"");
					send_to_char("The voices fill you with envy.\n\r",ch);
					do_steal(ch,buf);
				}
			}

			//MOVE PLAYER
			if (!random_done && number_percent() <= 4)
			{
				int door=0,counter=0;
				EXIT_DATA *pexit;
				ROOM_INDEX_DATA *to_room;
				for (door = number_range (0, 5), counter = 0; counter < 6;
					counter++, door++)
				{
					if (door > 5)
						door = door % 6;
					if ((pexit = ch->in_room->exit[door]) == NULL
						|| (to_room = pexit->u1.to_room) == NULL
						|| !can_see_room (ch, pexit->u1.to_room))
						continue;

					else
						break;
				}
				if (counter > 5)
				{
					//		send_to_char ("Alas, you cannot go that way.\n\r", ch);
					//return;
				}
				else
				{
					send_to_char("The voices decide that it is time to go.\n\r",ch);
					if (ch->position < POS_FIGHTING)
						do_stand(ch,"");
					if (ch->fighting)
					{
						do_flee(ch,"");
						random_done = TRUE;
						break;
					}
					else
					{
						switch (door)
						{
						case DIR_NORTH : do_north(ch,"");break;
						case DIR_EAST : do_east(ch,"");break;
						case DIR_SOUTH : do_south(ch,"");break;
						case DIR_WEST : do_west(ch,"");break;
						case DIR_DOWN : do_down(ch,"");break;
						case DIR_UP : do_up(ch,"");break;
							//		  case DIR_NORTH : do_north(ch,"");break;
						}
						random_done = TRUE;
						break;
					}
				}
			}

			//SOCIALS
			if (!random_done && number_percent() <= 5)
			{
				mch = get_random_player_room(ch);
				randint = number_range(1,18);
				if (mch == NULL)
					randint = number_range(9,18);
				if (get_position(ch) == POS_SLEEPING)
					check_social(ch,"snore","");
				else {
					switch(randint)
					{
					case 1: check_social(ch,"accuse",mch->name);break;
					case 2: check_social(ch,"glare",mch->name);break;
					case 3: check_social(ch,"pinch",mch->name);break;
					case 4: check_social(ch,"hug",mch->name);break;
					case 5: check_social(ch,"cry",mch->name);break;
					case 6: check_social(ch,"lust",mch->name);break;
					case 7: check_social(ch,"nibble",mch->name);break;
					case 8: check_social(ch,"hump",mch->name);break;
					case 9: check_social(ch,"smile","");break;
					case 10: check_social(ch,"nod","self");break;
					case 11: check_social(ch,"fume","");break;
					case 12: check_social(ch,"scream","");break;
					case 13: check_social(ch,"cough","");break;
					case 14: check_social(ch,"giggle","");break;
					case 15: check_social(ch,"babble","");break;
					case 16: check_social(ch,"howl","");break;
					case 17: check_social(ch,"blush","");break;
					case 18: check_social(ch,"whine","");break;
					}
				}
			}





		}

		if (!IS_NPC (ch) && ch->lh_ticks != 0)
			ch->lh_ticks--;

		if (!IS_NPC (ch) && ch->ex_ticks != 0)
			ch->ex_ticks--;

		if (!IS_NPC(ch) && ch->pcdata->pushed_mobs_counter > 0)
			--ch->pcdata->pushed_mobs_counter;

		if (!IS_NPC (ch) && ch->il_ticks != 0)
			ch->il_ticks--;

		if (!IS_NPC (ch) && ch->pcdata && ch->pcdata->last_pkdeath != 0)
		{
			if (current_time - ch->pcdata->last_pkdeath > 60*MULTIKILL_WAIT_TIME)
			{
				sprintf(buf,"`l%s's `kprotective aura fades.",ch->name);
				pk_chan(buf);
				WAIT_STATE (ch, PULSE_VIOLENCE*5);
				ch->pcdata->last_pkdeath = 0;
			}
		}

		if (!IS_NPC (ch) && ch->race == PC_RACE_NERIX && !IS_IMMORTAL(ch))
		{
			if ((1 + ((ch->played + (int) (current_time - ch->logon)) - ch->pcdata->nplayed)/36000) 
					> ch->pcdata->nage)
			{
				ch->pcdata->nage = 1 + ((ch->played + (int) (current_time - ch->logon)) 
					- ch->pcdata->nplayed)/36000;
				if (ch->pcdata->nage > 10)  /* Nerix dies */
				{
					send_to_char("You die of old age, but are quickly born again!",ch);
					act("$n dies of old age.",ch,NULL,NULL,TO_ROOM);
					raw_kill(ch,ch);
					decrease_level (ch);
					ch->pcdata->nage = 1;
					ch->pcdata->nplayed = ch->played + (int) (current_time - ch->logon);
					ch->perm_stat[STAT_WIS] -= 5;
					ch->perm_stat[STAT_INT] -= 5;
					ch->perm_stat[STAT_STR] += 5;
					ch->perm_stat[STAT_CON] += 5;
				}
				else
				{
					send_to_char("You age another year.  Happy Birthday!\n\r",ch);
					if (ch->pcdata->nage == 10)
						send_to_char("You are beginning the death march.\n\r",ch);
					if (ch->pcdata->nage % 2 == 0)
					{
						ch->perm_stat[STAT_WIS]++;
						ch->perm_stat[STAT_INT]++;
						ch->perm_stat[STAT_STR]--;
						ch->perm_stat[STAT_CON]--;
					}

				}
			}
		}


		if (ch->timer > 30)
			ch_quit = ch;

		//IBLIS 6/07/03 - Here is the updates on last_death_counter (for use in Holy Ceremony prayer)
		if (!IS_NPC (ch) && ch->pcdata->last_death_counter)
		{
			if (--ch->pcdata->last_death_counter == 0)
			{
				send_to_char
					("`hYou no longer feel the distant memories of your wandering spirit.``\r\n",
					ch);
				ch->pcdata->xp_last_death = 0;
			}
		} 
		if (ch->in_room == NULL)
			continue;
		if (ch->position >= POS_STUNNED)
		{
			if (IS_NPC (ch) && ch->zone != NULL
				&& ch->zone != ch->in_room->area && ch->desc == NULL
				&& ch->fighting == NULL && ch->riders == NULL
				&& ch->position != POS_TETHERED && ch->mounter == NULL
				&& !IS_AFFECTED (ch, AFF_CHARM) && !is_druid_follower(ch) && number_percent () < 5)
			{
				RESET_DATA *pReset;
				ROOM_INDEX_DATA *pRoom;
				//Iblis - This was fixed so that when a mob wanders home, it is simply transfered home,
				//rather than poofing and repopping.  This prevents players from pushing mobs to force
				//repop, and fixed the problem that lost Yorl (though we had few complaints from anyone)
				//of a mob having objects on them when they wander home.  
				for (pReset = ch->zone->reset_first;pReset != NULL;pReset = pReset->next)
				{
					if (pReset->command != 'M')
						continue;
					if (pReset->arg1 != ch->pIndexData->vnum)
						continue;
					pRoom = get_room_index (pReset->arg3);
					if (pRoom == NULL)
						continue;
					act ("$n wanders on home.", ch, NULL, NULL, TO_ROOM);
					char_from_room(ch);
					char_to_room(ch,pRoom);
					act ("$n wanders into the room.",ch,NULL,NULL,TO_ROOM);
				}
				if (pReset != NULL)
					continue;
			}


			// Minax 8-3-02 No hp regen if poisoned (newbies are spared)
			if ((!IS_AFFECTED (ch, AFF_POISON) || ch->level < 9) 
				&& !is_affected (ch, gsn_pox)
				&& !is_affected (ch, gsn_brain_blisters)
				&& !is_affected (ch, gsn_vomit)
				&& !is_affected (ch, gsn_contaminate)
				&& !is_affected (ch, gsn_larva)
				&& ch->position != POS_FEIGNING_DEATH
				&& (!(!IS_NPC(ch) && ch->pcdata->oow_ticks > 19)))
			{
				if (IS_NPC(ch))
					ch->hit += hit_gain (ch);
				else
					ch->hit += hit_gain (ch) + (ch->pcdata->condition[COND_DRUNK]*(10/300))*hit_gain(ch);
		
				if (ch->position == POS_FEIGNING_DEATH)
					damage (ch, ch, 10, gsn_feign_death, DAM_UNIQUE, FALSE);
			}
			if (ch->hit < 0)
			{ 
				raw_kill (ch, ch);
				if (is_dueling (ch))
				{
					duel_ends (ch);
				}

				if (ch->dueler)
				{
					superduel_ends(ch,ch);
				}

				continue;
			}

			if (ch->hit > ch->max_hit)
				ch->hit = ch->max_hit;

			// Minax 8-3-02 No mana regen if poisoned (newbies are spared)
			// No mana gain is aura rot is on
			if ((!is_affected(ch,gsn_aura_rot) || ch->level < 9 ))
			{
				if (ch->mana < ch->max_mana)
				{
					if (IS_NPC(ch))
						ch->mana += (mana_gain (ch));
					else
						ch->mana += (mana_gain (ch) - (ch->pcdata->condition[COND_DRUNK]*(10/600))*mana_gain(ch));
				}
				else
					ch->mana = ch->max_mana;
			}
			if (ch != NULL && is_affected(ch, gsn_insect_plague))
			{
				send_to_char ("`aThe insects continue to devour your flesh``.\n\r", ch);
				act("`aInsects continue to devour $n's body``.", ch, NULL, NULL, TO_ROOM);
				if (IS_NPC(ch))
				ch->hit -= ch->max_hit * number_range(2, 5) / 200;
				else
				ch->hit -= ch->max_hit * number_range(3, 6) / 100;
			}
			if (!is_affected(ch,gsn_atrophy))
			{
				if (ch->move < ch->max_move)
					ch->move += move_gain (ch);
				else
					ch->move = ch->max_move;
			}
			else
			{
				AFFECT_DATA* af;
				for (af = ch->affected; af != NULL; af = af->next)
				{
					if (af->type == gsn_atrophy)
						break;
				}
				if (af)
					ch->move -= (++af->bitvector < 6)?(af->bitvector*15) + 15:100;
			}


		}

		if (ch->position == POS_STUNNED)
			update_pos (ch);


		if (!is_affected (ch, gsn_swim))
		{
			if (ch->in_room == NULL)
				continue;


			if (ch->in_room->sector_type == SECT_UNDERWATER &&
				!IS_AFFECTED (ch, AFF_AQUA_BREATHE) && !IS_IMMORTAL (ch) 
				&& !IS_SET(ch->imm_flags,IMM_DROWNING))
			{
				int dam;
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
					if (ch->drown_damage < 1)
						ch->drown_damage = 1;

					dam = (ch->hit * number_range (10 *
						ch->drown_damage,
						20 * ch->drown_damage)) /
						100;
					ch->drown_damage += 2;

					ch->mana -= dam;
					ch->move -= dam;

					send_to_char ("Your lungs fill with water.\n\r", ch);
					act ("$n turns purple floundering for air.", ch, NULL,
						NULL, TO_ROOM);
					damage_old (ch, ch, dam, gsn_poison, DAM_DROWNING, FALSE);
				}
			}


			if (ch->in_room == NULL)
				continue;

			if ((ch->in_room->sector_type == SECT_WATER_NOSWIM ||
				ch->in_room->sector_type == SECT_WATER_OCEAN)
				&& !IS_AFFECTED (ch, AFF_AQUA_BREATHE) && !IS_SET(ch->imm_flags,IMM_DROWNING)
				&& !IS_IMMORTAL (ch) && !IS_AFFECTED (ch, AFF_SWIM))
			{
				int dam;
				OBJ_DATA *obj;
				bool found;

				found = FALSE;

				if (ch->on != NULL && ch->on->item_type == ITEM_RAFT)
					found = TRUE;
				for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
					if ((obj->item_type == ITEM_GILLS
						&& obj->wear_loc != WEAR_NONE))
					{
						found = TRUE;
						break;
					}

					if (ch->mount != NULL && IS_AFFECTED (ch->mount, AFF_SWIM))
						found = TRUE;

					if (!found)
					{
						ch->last_swim_update++;

						if (ch->last_swim_update > 15)
							ch->last_swim_update = 15;

						if (!((ch->in_room->sector_type == SECT_WATER_NOSWIM ||
							ch->in_room->sector_type == SECT_WATER_OCEAN) &&
							ch->last_swim_update < 2))
						{
							if (number_percent () > get_skill (ch, gsn_swim) ||
								ch->last_swim_update >= 10)
							{
								if (ch->drown_damage < 1)
									ch->drown_damage = 1;

								dam =
									(ch->hit *
									number_range (10 * ch->drown_damage,
									20 * ch->drown_damage)) / 100;
								ch->drown_damage += 2;

								ch->mana -= dam;
								ch->move -= dam;

								send_to_char ("Your lungs fill with water.\n\r",
									ch);
								act ("$n turns purple floundering for air.", ch,
									NULL, NULL, TO_ROOM);
								damage_old (ch, ch, dam, gsn_poison,
									DAM_DROWNING, FALSE);
							}
							else
								check_improve (ch, gsn_swim, TRUE, 2);
						}
					}
					else
						ch->last_swim_update = 0;
			}
			else
				ch->last_swim_update = 0;
		}

		if (ch->in_room == NULL)
			continue;

		if (!IS_NPC (ch) && ch->level < LEVEL_IMMORTAL)
		{
			OBJ_DATA *obj;

			if ((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL
				&& obj->item_type == ITEM_LIGHT && obj->value[2] > 0)
			{
				if (--obj->value[2] == 0 && ch->in_room != NULL)
				{
					--ch->in_room->light;
					act ("$p goes out.", ch, obj, NULL, TO_ROOM);
					act ("$p flickers and goes out.", ch, obj, NULL, TO_CHAR);
					extract_obj (obj);
				}
				else if (obj->value[2] <= 5 && ch->in_room != NULL)
					act ("$p flickers.", ch, obj, NULL, TO_CHAR);
			}
			// Here, the check for linkdead characters gets readded
			// as the real issue is that people use linkdead state
			// to escape online jail time.
			//
			// changed/added by Morgan 6/20/2000
			//

			if ((ch->want_jail_time != 0) && (ch->desc != NULL))
			{
				if (ch->want_jail_time == 1)
					unjail (ch);
				ch->want_jail_time--;

				if (!IS_NPC (ch) && IS_SET (ch->act, PLR_JAILED) &&
					(ch->want_jail_time % 3 == 0))
				{
					send_to_char
						("A prison guard enters and forces you to eat a meal of stale bread and rusty water.\n\r",
						ch);
					sprintf (buf,
						"The prison guard taunts you, 'Haha, you better eat since you have to stay here for %d more ticks.'\n\r",
						ch->want_jail_time);
					send_to_char (buf, ch);
					act
						("A prison guard enters and forces $n to eat a meal of stale bread and rusty water.\n\r",
						ch, NULL, NULL, TO_ROOM);
					if (ch->race != PC_RACE_KALIAN)
						ch->pcdata->condition[COND_HUNGER] += 4;
					ch->pcdata->condition[COND_THIRST] += 4;
					if (ch->race == PC_RACE_VROATH)
					{
						ch->pcdata->condition[COND_THIRST] += 6;
						ch->pcdata->condition[COND_HUNGER] += 6;
					}
				}
			}
			if (IS_IMMORTAL (ch))
				ch->timer = 0;

			if (++ch->timer >= 12)
			{
				if (ch->was_in_room == NULL && ch->in_room != NULL)
				{
					ch->was_in_room = ch->in_room;
					if (ch->fighting != NULL)
						stop_fighting (ch, TRUE);
					act ("$n disappears into the void.",
						ch, NULL, NULL, TO_ROOM);
					send_to_char ("You disappear into the void.\n\r", ch);
					if (ch->level > 1)
						save_char_obj (ch);
					char_from_room (ch);
					char_to_room (ch, get_room_index (ROOM_VNUM_LIMBO));
				}
			}

			//  Paarshad's boredom incarnate
			//  Being drunk makes you do,see and hear wierd shit now
			//  hehe
			if (number_percent () < (ch->pcdata->condition[COND_DRUNK]*(5/2)))
			{
				CHAR_DATA *mch;
				AFFECT_DATA af;
				OBJ_DATA *rose;
				int randint;
				gain_condition (ch, COND_DRUNK, -2);
				randint = number_range(1,5);
				if (ch->pcdata->condition[COND_DRUNK] < 10)
					switch(randint)
				{
					case 1: 
						act("`oA `fpink elephant `otells you `a'`kDrink up $n, there is always room for more.`a'``\n\r",ch,NULL,NULL,TO_CHAR); 
						act("`o$n raises $s cup and slurs, `a'`kCHEERS!`a'``\n\r",ch,NULL,NULL,TO_ROOM) ;
						break;
					case 2:
						act("`oA `fpink elephant `otells you to touch the torch, it won't hurt.``\n\r",ch,NULL,NULL,TO_CHAR); 
						act("`o$n reaches tentatively for $s light.``\n\r",ch,NULL,NULL,TO_ROOM) ;
						break;
					case 3:
					case 4:
					default:
						act("`oA pink elephant gracefully pirouettes towards you, and you decide to join in.``\n\r",ch,NULL,NULL,TO_CHAR); 
						act("`o$n spins around stupidly, bumping into everything and everyone around $m.``\n\r",ch,NULL,NULL,TO_ROOM) ;
						break;
				}
				else if (ch->pcdata->condition[COND_DRUNK] < 20)
					switch(randint)
				{
					case 1: 
						act("`oYou feel `vINVINCIBLE!``\n\r",ch,NULL,NULL,TO_CHAR); 
						act("$n begins flexing for no apparent reason.``\n\r",ch,NULL,NULL,TO_ROOM) ;

						af.where = TO_AFFECTS;
						af.type = exact_skill_lookup("giant strength");
						af.level = (ch->pcdata->condition[COND_DRUNK]*(5/3));
						af.duration = 5;
						af.location = APPLY_STR;
						af.modifier = 1 + ((ch->pcdata->condition[COND_DRUNK]*(10/3)) >= 75);
						af.bitvector = 0;
						af.permaff = FALSE;
						af.composition = FALSE;
						af.comp_name = str_dup ("");
						affect_join (ch, &af);
						send_to_char ("Your muscles surge with heightened power!\n\r", ch);
						break;
					case 2:
						act("`oYou stomach starts to feel `jqueasy`o.``\n\r",ch,NULL,NULL,TO_CHAR); 
						act("`o$n turns slightly green.``\n\r",ch,NULL,NULL,TO_ROOM) ;
						damage (ch, ch, 15, 0 , DAM_CLAWING, FALSE);
						break;
					case 3: 
						act("`oYou pull a `brose `ofrom behind your back.  You're so clever.``\n\r",ch,NULL,NULL,TO_CHAR); 
						act("`o$n is now holding a `brose``.  Where did that come from?``\n\r",ch,NULL,NULL,TO_ROOM) ;
						rose = create_object (get_obj_index (OBJ_VNUM_ROSE), 0);
						obj_to_char (rose, ch);
						break;
					case 4:
					default:
						act("`oLittle`j green`o men start running around and you decide to sit down for a chat.``\n\r",ch,NULL,NULL,TO_CHAR); 
						act("`o$n looks slightly dizzy and sits down.``\n\r",ch,NULL,NULL,TO_ROOM) ;
						ch->position = POS_SITTING;
						break;
				}	
				else if (ch->pcdata->condition[COND_DRUNK] < 30)
					switch(randint)
				{
					case 1: 
						act("`oYou feel lighter than `gair`o.``\n\r",ch,NULL,NULL,TO_CHAR); 
						act("`o$n amazingly begins floating infront of you.``\n\r",ch,NULL,NULL,TO_ROOM) ;
						/* fly baby fly */
						af.where = TO_AFFECTS;
						af.type = exact_skill_lookup("fly");
						af.level = (ch->pcdata->condition[COND_DRUNK]*(5/3));
						af.duration = 2;
						af.location = 0;
						af.modifier = 0;
						af.bitvector = AFF_FLYING;
						af.permaff = FALSE;
						af.composition = FALSE;
						af.comp_name = str_dup ("");
						affect_join (ch, &af);

						break;
					case 2:
						act("`oThe world `ablurs`o and starts to `gs`cp`di`fn`o.``\n\r",ch,NULL,NULL,TO_CHAR); 
						act("`o$n's eyes glaze over.``\n\r",ch,NULL,NULL,TO_ROOM) ;
						af.where = TO_AFFECTS;
						af.type = gsn_disorientation;
						af.level = (ch->pcdata->condition[COND_DRUNK]*(5/3));
						af.duration = 2;
						af.location = APPLY_HITROLL;
						af.modifier = -4;
						af.permaff = FALSE;
						af.bitvector = 0;
						af.composition = FALSE;
						af.comp_name = str_dup ("");
						affect_join (ch, &af);

						break;
					case 3:
						act("`oYou don't take shit like that from `banyone`o!``\n\r",ch,NULL,NULL,TO_CHAR); 
						act("`b$n goes crazy and attacks!``\n\r",ch,NULL,NULL,TO_ROOM) ;
						mch = get_random_mob_room(ch);
						if (mch == NULL)
							mch = get_random_player_room(ch);
						if (mch != NULL)
						{
							if (ch->position < POS_FIGHTING)
								do_stand(ch,"");
							if (ch->fighting && !is_safe(ch,mch))
								ch->fighting = mch;
							else do_kill(ch,mch->name);
						}

						break;
					case 4:
					default:
						act("`oThe world fades to `ablack.``\n\r",ch,NULL,NULL,TO_CHAR); 
						act("`o$n falls hard and hits the ground with a resounding THUD.``\n\r",ch,NULL,NULL,TO_ROOM) ;
						ch->pcdata->knock_time = 24;
						stop_fighting (ch, TRUE);
						ch->position = POS_SLEEPING;
						break;
				}	
				else 
					switch(randint)
				{
					case 1: 
					case 2:
						act("`oMaybe you shouldn't have had that last one.``\n\r",ch,NULL,NULL,TO_CHAR); 
						act("`o$n pukes all over the floor.``\n\r",ch,NULL,NULL,TO_ROOM) ;
						af.where = TO_AFFECTS;
						af.type = gsn_vomit;
						af.level = (ch->pcdata->condition[COND_DRUNK]*(5/3));
						af.duration = 2;
						af.location = APPLY_CON;
						af.modifier = -1;
						af.bitvector = 0;
						af.permaff = FALSE;
						af.composition = FALSE;
						af.comp_name = str_dup("");
						affect_join (ch, &af);

						break;
					case 3:
					case 4:
					default:
						act("`bUh Oh!``\n\r",ch,NULL,NULL,TO_CHAR); 
						act("`o$n looks like $e should be rushed to the nearest healer immediately!``\n\r",ch,NULL,NULL,TO_ROOM) ;
						af.where = TO_AFFECTS;
						af.type = gsn_poison;
						af.level = (ch->pcdata->condition[COND_DRUNK]*(5/3));
						af.duration = 7;
						af.location = APPLY_STR;
						af.modifier = -2;
						af.bitvector = AFF_POISON;
						af.permaff = FALSE;
						af.composition = FALSE;
						af.comp_name = str_dup ("");
						affect_join (ch, &af);
						send_to_char ("You feel very sick.\n\r", ch);

						break;
				}	
			} else gain_condition (ch, COND_DRUNK, -1);
			gain_condition(ch,COND_PILLS,-1);
			if (number_percent () + 5 < get_skill (ch, gsn_endurance))
				check_improve (ch, gsn_endurance, TRUE, get_curr_stat (ch, STAT_CON) - 5);
			else
			{
				if (!IS_NPC(ch) && (ch->pcdata->cg_state == 0 || number_percent() <= 2))
				{
					//	        if (!is_affected(ch,gsn_intoxicate))
					gain_condition (ch, COND_FULL,
						ch->size > SIZE_MEDIUM ? -4 : -2);
					if (ch->race != PC_RACE_NIDAE || 
						!(ch->in_room && 
						(ch->in_room->sector_type == SECT_UNDERWATER ||
						ch->in_room->sector_type == SECT_WATER_OCEAN)))
						gain_condition (ch, COND_THIRST, -1);
					if (ch->race != PC_RACE_KALIAN)
						gain_condition (ch, COND_HUNGER,
						ch->size > SIZE_MEDIUM ? -2 : -1);
				}
			}
		}

		//
		// Akamai 11/4/98 - This is where the code checks a players affs
		// the time is decremented by going through this player's list of
		// affs and decrementing the aff duration
		//
		for (paf = ch->affected; paf != NULL; paf = paf_next)
		{
			paf_next = paf->next;
			if (paf->duration > 0)
			{
				// duration is greater than zero so decrement it and then
				// do some checks
				// NOTE: IF YOU UPDATE HERE, MAKE SURE TO UPDATE IN MAGIC2.C, the vortex spell
				if (!(paf->type == gsn_jalknation 
					|| paf->type == gsn_jurgnation 
					|| paf->type == gsn_aquatitus 
					|| paf->type == gsn_dehydration
					|| paf->type == gsn_death_spasms
					|| (!IS_NPC(ch) && ch->pcdata->flaming && paf->type == gsn_fireshield)))
					paf->duration--;

				if ((number_range (0, 4) == 0) && (paf->level > 0))
					paf->level--;
			}
			else if (paf->duration == 0)
			{
				// This aff is going to be gone in this tick
				if ((paf_next == NULL) || (paf_next->type != paf->type)
					|| (paf_next->duration > 0))
				{
					if ((paf->type > 0) && (skill_table[paf->type].msg_off)
						&& (!paf->composition))
					{
						if (paf->type == gsn_meteor_swarm)
						{
							if (paf->comp_name != NULL && str_cmp(paf->comp_name,""))
							{
								CHAR_DATA *attacker;
								if ((attacker = get_exact_pc_world(ch,paf->comp_name)) == NULL)
									attacker = ch;
								meteor_swarm(attacker,ch);
							}
						}
						else 
						{
							send_to_char (skill_table[paf->type].msg_off, ch);
							send_to_char ("\n\r", ch);
							if (paf->type == skill_lookup("nearsight"))
							{
								act("$n blinks a few times and then grunts in satisfaction.", ch,NULL,NULL,TO_ROOM);
							}

						}
					}
					if (paf->composition && paf->type >= 0)
					{
						send_to_char (comp_table[paf->type].wear_off_string,
							ch);
						send_to_char ("\n\r", ch);
					}
				}
				affect_remove (ch, paf);
			}
			else if (IS_NPC(ch) || paf->duration < -1)
			{
				// ok, somehow this aff got to be less than zero - just silently
				// remove the aff from the player aff list
				affect_remove (ch, paf);
			}
		}
		if (ch != NULL && is_affected (ch, gsn_vomit))
		{
			crit_strike_possible = FALSE;
			damage(ch,ch,60-(get_curr_stat(ch,STAT_CON)),gsn_vomit,DAM_DISEASE,FALSE);
			send_to_char("It feels like you are turned inside out as you vomit blood.\n\r",ch);
			act("$n vomits violently.",ch,NULL,NULL,TO_ROOM);
			crit_strike_possible = TRUE;
		}

		if (ch != NULL && is_affected (ch, gsn_festering_boils))
		{
			if (get_position(ch) == POS_SLEEPING)
				act("$n turns in $s sleep as the boils swell.",ch,NULL,NULL,TO_ROOM);
			else 
			{
				act("The black boils on $n's neck fester.",ch,NULL,NULL,TO_ROOM);
				send_to_char("The boils that cover your body creak as they rub against each other when you move.\n\r",ch);
			}

		}

		if (ch->race == PC_RACE_LITAN
			&& weather_info.sky == SKY_CLOUDLESS
			&& !IS_AFFECTED(ch,AFF_HASTE)
			&& IS_OUTSIDE(ch)
			&& time_info.hour >= 5 && time_info.hour < 19)
			spell_haste(gsn_aquatitus,ch->level,ch,(void *)ch,TARGET_CHAR);
		if (ch->race == PC_RACE_LITAN
			&& (weather_info.sky != SKY_CLOUDLESS || time_info.hour >= 19
			|| time_info.hour < 5))
			affect_strip(ch,gsn_aquatitus);


		/*      if (ch != NULL && is_affected (ch, gsn_contaminate))
		{
		send_to_char("You feel part of your life slip away.\n\r",ch);
		ch->mana -= 10;
		ch->move -= 10;
		if (ch->hit > 10)
		ch->hit -= 10;
		else damage (ch,ch,10,gsn_contaminate,DAM_UNIQUE,FALSE);
		if (ch->contaminator)
		{
		ch->contaminator->mana += 10;
		ch->contaminator->move += 10;
		ch->contaminator->hit += 10;
		}
		}*/

		if (ch != NULL && is_affected (ch, gsn_aura_rot))
		{
			send_to_char("The aura surrounding your body continues to crumble away.\n\r",ch);
			act("The light in $n's eyes dims further.",ch,NULL,NULL,TO_ROOM);
			ch->mana -= ch->max_mana*number_range(10,15)/100;

		}

		if (ch != NULL && is_affected (ch,gsn_larva))
		{
			AFFECT_DATA *af;
			for (af = ch->affected; af != NULL; af = af->next)
			{
				if (af->type == gsn_larva)
					break;
			}
			if (number_percent() == 97)
			{
				send_to_char("You choke as the larvae crawl inside your lung.\n\r",ch);
				damage_old(ch,ch,50,gsn_larva,DAM_UNIQUE,FALSE);
			}
			else
			{
				send_to_char("The larvae eat your flesh!\n\r",ch);
				damage_old(ch,ch,af->modifier,gsn_larva,DAM_UNIQUE,FALSE);
			}
			act("$n shudders and you notice some kind of worm moving under $s skin.",ch,NULL,NULL,TO_ROOM);

		}

		if (ch != NULL && is_affected (ch, gsn_pox))
		{
			AFFECT_DATA *af, plague;
			CHAR_DATA *vch;
			for (af = ch->affected; af != NULL; af = af->next)
			{
				if (af->type == gsn_pox)
					break;
			}
			send_to_char("Pox burst as you move.\n\r",ch);
			act("A snot-like substance leaks out of $n's pox.",ch,NULL,NULL,TO_ROOM);
			crit_strike_possible = FALSE;
			damage(ch,ch,30-(get_curr_stat(ch,STAT_CON)),gsn_pox,DAM_DISEASE,FALSE);
			crit_strike_possible = TRUE;
			plague.where = TO_AFFECTS;
			plague.type = gsn_pox;
			plague.level = af->level-1;
			plague.duration = number_range (1, 2 * plague.level);
			plague.location = APPLY_CHA;
			plague.modifier = -6;
			plague.bitvector = 0;
			plague.composition = FALSE;
			plague.comp_name = str_dup ("");
			if (ch->in_room)
			{
				for (vch = ch->in_room->people; vch != NULL;
					vch = vch->next_in_room)
				{
					if (IS_NPC(vch))
						continue;
					if (!saves_spell (plague.level - 2, vch, DAM_DISEASE)
						&& !IS_IMMORTAL (vch)
						&& IS_NPC(vch)
						&& !is_affected(vch,gsn_immunity)
						&& !is_affected (vch, gsn_pox) && number_bits (4) == 0)
					{
						send_to_char ("Your skin prickles with yellow pox.\n\r", vch);
						act ("Hideous yellow pox appears all over $n's body.", vch, NULL, NULL,
							TO_ROOM);
						affect_join (vch, &plague);
					}
				}
			}

		}

		if (ch != NULL && is_affected (ch, gsn_brain_blisters))
		{
			AFFECT_DATA *af, plague,af2;
			CHAR_DATA *vch;
			int duration;
			for (af = ch->affected; af != NULL; af = af->next)
			{
				if (af->type == gsn_brain_blisters)
					break;
			}
			send_to_char("Pus oozes down into your eyes as the blisters on your scalp burst!\n\r",ch);
			act("$n grabs $s head as blisters burst on $s scalp!",ch,NULL,NULL,TO_ROOM);
			if (get_curr_stat(ch,STAT_CON) < 25)
			{
				crit_strike_possible = FALSE;
				damage(ch,ch,40-(get_curr_stat(ch,STAT_CON)),gsn_brain_blisters,DAM_DISEASE,FALSE);
				crit_strike_possible = TRUE;
			}
			duration =  number_range (1, 2 * plague.level);
			plague.where = TO_AFFECTS;
			plague.type = gsn_brain_blisters;
			plague.level = af->level-1;
			plague.duration = duration;
			plague.location = APPLY_CHA;
			plague.modifier = -6;
			plague.bitvector = 0;
			plague.composition = FALSE;
			plague.comp_name = str_dup ("");
			af2.where = TO_AFFECTS;
			af2.type = gsn_disorientation;
			af2.level = af->level-1;
			af2.duration = duration;
			af2.location = APPLY_HITROLL;
			af2.modifier = -4;
			af2.permaff = FALSE;
			af2.bitvector = 0;
			af2.composition = FALSE;
			af2.comp_name = str_dup ("");
			if (ch->in_room)
			{
				for (vch = ch->in_room->people; vch != NULL;
					vch = vch->next_in_room)
				{
					//	      if (IS_NPC(vch))
					//continue;
					if (!saves_spell (plague.level - 2, vch, DAM_DISEASE)
						&& !IS_IMMORTAL (vch)
						&& IS_NPC(vch)
						&& !is_affected(vch,gsn_immunity)
						&& !is_affected (vch, gsn_brain_blisters) && number_bits (4) == 0)
					{
						send_to_char ("Pus oozes down into your eyes as the blisters on your scalp burst!\n\r", vch);
						act ("$n grabs $s head as blisters burst on $s scalp!", vch, NULL, NULL,TO_ROOM);
						affect_join (vch, &plague);
						affect_join (vch, &af2);
					}
				}
			}

		}

		if (ch != NULL && is_affected (ch,gsn_vomit))
		{
			AFFECT_DATA *af, af2;
			for (af = ch->affected; af != NULL; af = af->next)
			{
				if (af->type == gsn_vomit)
					break;
			}
			if (get_curr_stat(ch,STAT_CON) > 6)
			{
				af2.where = af->where;
				af2.type = af->type;
				af2.level = af->level;
				af2.duration = 0;
				af2.location = af->location;
				af2.modifier = -1;
				af2.permaff = FALSE;
				af2.bitvector = 0;
				af2.composition = FALSE;
				af2.comp_name = str_dup(af->comp_name);
				affect_join (ch, &af2);
			}
		}


		if (ch != NULL && is_affected (ch, gsn_plague))
		{
			AFFECT_DATA *af, plague;
			CHAR_DATA *vch;
			int dam;

			if (ch->in_room == NULL)
				continue;

			act ("$n writhes in agony as plague sores erupt from $s skin.",
				ch, NULL, NULL, TO_ROOM);
			send_to_char ("You writhe in agony from the plague.\n\r", ch);
			for (af = ch->affected; af != NULL; af = af->next)
			{
				if (af->type == gsn_plague)
					break;
			}

			if (af == NULL)
			{
				REMOVE_BIT (ch->affected_by, AFF_PLAGUE);
				continue;
			}

			if (af->level == 1)
				continue;

			plague.where = TO_AFFECTS;
			plague.type = gsn_plague;
			plague.level = af->level - 1;
			plague.duration = number_range (1, 2 * plague.level);
			plague.location = APPLY_STR;
			plague.modifier = -5;
			plague.bitvector = AFF_PLAGUE;
			plague.composition = FALSE;
			plague.comp_name = str_dup ("");

			for (vch = ch->in_room->people; vch != NULL;
				vch = vch->next_in_room)
			{
				if (!saves_spell (plague.level - 2, vch, DAM_DISEASE)
					&& !IS_IMMORTAL (vch)
					&& IS_NPC(vch)
					&& !is_affected(vch,gsn_immunity)
					&& !(IS_NPC (vch) && IS_SET (vch->act, ACT_UNDEAD))
					&& !IS_AFFECTED (vch, AFF_PLAGUE) && number_bits (4) == 0)
				{
					send_to_char ("You feel hot and feverish.\n\r", vch);
					act ("$n shivers and looks very ill.", vch, NULL, NULL,
						TO_ROOM);
					affect_join (vch, &plague);
				}
			}

			dam = UMIN (ch->level, af->level / 5 + 1);
			ch->mana -= dam;
			ch->move -= dam;
			crit_strike_possible = FALSE;
			damage_old (ch, ch, dam, gsn_plague, DAM_DISEASE, FALSE);
			crit_strike_possible = TRUE;
		}

		else if (IS_AFFECTED (ch, AFF_POISON) && ch != NULL
			&& !IS_AFFECTED (ch, AFF_SLOW))
		{
			AFFECT_DATA *poison;

			poison = affect_find (ch->affected, gsn_poison);

			if (poison != NULL)
			{
				act ("$n shivers and suffers.", ch, NULL, NULL, TO_ROOM);
				send_to_char ("You shiver and suffer.\n\r", ch);
				// Minax 8-2-02 Here's where the poison wackiness begins.
				tickdam = number_range (30, 50);
				tickdam -= get_curr_stat (ch, STAT_CON);
				crit_strike_possible = FALSE;
				damage_old (ch, ch, tickdam, gsn_poison, DAM_POISON, FALSE);
				crit_strike_possible = TRUE;
				// Poison can make you go blind.
				if (!IS_NPC (ch) && number_percent () < 4 && ch->level > 9)
				{
					if (IS_AFFECTED (ch, AFF_BLIND)
						|| saves_spell (level, ch, DAM_OTHER))
						continue;

					af.where = TO_AFFECTS;
					af.type = gsn_blindness;
					af.level = ch->level / 7;
					af.location = APPLY_HITROLL;
					af.modifier = -4;
					af.duration = 3;
					af.bitvector = AFF_BLIND;
					af.permaff = FALSE;
					af.composition = FALSE;
					af.comp_name = str_dup ("");

					affect_to_char (ch, &af);

					send_to_char
						("The poison in your veins makes the world go dark.\n\r",
						ch);
					act ("$n fumbles awkwardly, and appears to be blinded.",
						ch, NULL, NULL, TO_ROOM);
				}
				// Poison can make your legs give out.
				if (!IS_NPC (ch) && number_percent () < 11 && ch->level > 9
					&& get_position(ch) != POS_SLEEPING
					&& get_position(ch) != POS_RESTING
					&& get_position(ch) != POS_SITTING)
				{
					send_to_char
						("Your legs give out and you fall to the ground.\n\r",
						ch);
					ch->position = POS_RESTING;
				}
				// Poison can kill a person outright (similar to bee swarm)
				if (!IS_NPC (ch)
					&& number_range (1,
					100 + get_curr_stat (ch, STAT_CON) * 2)
					== 50 && ch->level < 92 && ch->level > 9)
				{
					send_to_char
						("The poison in your body causes your heart to give out.\n\r",
						ch);
					act
						("$n gasps for air, but can no longer resist the poison in $s body.",
						ch, NULL, NULL, TO_ROOM);
					raw_kill (ch, ch);
					if (is_dueling (ch))
					{
						duel_ends (ch);
					}

					if (ch->dueler)
					{
						superduel_ends(ch,ch);
					}

				}
			}
		}


		else if (ch->position == POS_INCAP && number_range (0, 1) == 0)
		{
			damage (ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE);
		}
		else if (ch->position == POS_MORTAL)
		{
			damage (ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE);
		}
	}

	for (ch = char_list; ch != NULL; ch = ch_next)
	{
		ch_next = ch->next;

		if (ch->desc != NULL && ch->level > 1 &&
			ch->desc->descriptor % 30 == save_number)
			save_char_obj (ch);

		if (ch == ch_quit)
			do_quit (ch, "");
	}

	return;
}

//Iblis - Object trigger update to deal with the object triggers that perform actions on tick
void obj_trig_tick_update (void)
{
	OBJ_TRIG *ot, *ot_next;
	for (ot = on_tick_ot_list; ot != NULL; ot = ot_next)
	{
		ot_next = ot->next_on_tick;
		if(IS_SET(ot->pIndexData->trig_flags, OBJ_TRIG_ON_TICK))
		{
			trip_triggers_special(NULL, OBJ_TRIG_ON_TICK, NULL, NULL, OT_SPEC_NONE, ot);
		}
	}
}



void obj_update (void)
{
	OBJ_DATA *obj, *nobj;
	OBJ_DATA *obj_next;
	AFFECT_DATA *paf, *paf_next;
	ROOM_INDEX_DATA *inroom;
	int oldlevel;

	for (obj = object_list; obj != NULL; obj = obj_next)
	{
		CHAR_DATA *rch;
		char *message="";

		obj_next = obj->next;

		for (paf = obj->affected; paf != NULL; paf = paf_next)
		{
			paf_next = paf->next;
			if (paf->duration > 0)
			{
				paf->duration--;
				if (number_range (0, 4) == 0 && paf->level > 0)
					paf->level--;
			}
			else if (paf->duration < 0);
			else
			{
				if (paf_next == NULL
					|| paf_next->type != paf->type || paf_next->duration > 0)
				{
					if (paf->type > 0 && skill_table[paf->type].msg_obj)
					{
						if (obj->carried_by != NULL)
						{
							rch = obj->carried_by;
							act (skill_table[paf->type].msg_obj,
								rch, obj, NULL, TO_CHAR);
							if (paf->type == 1
								&& paf->bitvector == WEAPON_SHARP)
							{
								send_to_char
									("The glimmering aura around your blade fades away.\r\n",
									rch);
							}
							if (paf->type == 1
								&& paf->bitvector == WEAPON_VAMPIRIC)
							{
								send_to_char
									("Your blade's thirst for blood subsides.\r\n",
									rch);
							}
							if (paf->type == 1
								&& paf->bitvector == WEAPON_APATHY)
							{
								send_to_char
									("Your blade's apathy wears off.\r\n",
									rch);
							}

						}
						if (obj->in_room != NULL
							&& obj->in_room->people != NULL)
						{
							rch = obj->in_room->people;
							act (skill_table[paf->type].msg_obj,
								rch, obj, NULL, TO_ALL);
						}
					}
				}

				affect_remove_obj (obj, paf);
			}
		}

		if (obj->item_type == ITEM_WRITING_PAPER && obj->value[0] >= 0)
		{
			if (obj->value[0] > 0)
				obj->value[0]--;

			if (obj->value == 0)
			{
				EXTRA_DESCR_DATA *ed;

				for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
					if (!str_cmp (ed->keyword, "papermsg"))
					{
						free_extra_descr (ed);
						break;
					}

					obj->value[0] = obj->pIndexData->value[0];
			}
		}

		if (obj->timer <= 0 || --obj->timer > 0)
			continue;

		if (obj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE1)
		{
			inroom = obj->in_room;
			if (inroom != NULL)
			{
				act
					("Stretching out as if searching for unused wood, the licks of fire before you begin to slowly die.",
					inroom->people, NULL, NULL, TO_ALL);
				oldlevel = obj->level;
				obj_from_room (obj);
				extract_obj (obj);
				nobj = create_object (get_obj_index (OBJ_VNUM_CAMPFIRE2), 0);
				nobj->timer = (oldlevel / 8) / 3;
				if (nobj->timer < 1)
					nobj->timer = 1;
				nobj->level = oldlevel;
				obj_to_room (nobj, inroom);
				continue;
			}
		}
		else if (obj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE2)
		{
			inroom = obj->in_room;
			if (inroom != NULL)
			{
				act
					("With a penultimate flare and a final, impotent crackle, the flames sink low into the blackened wood.",
					inroom->people, NULL, NULL, TO_ALL);
				oldlevel = obj->level;
				obj_from_room (obj);
				extract_obj (obj);
				nobj = create_object (get_obj_index (OBJ_VNUM_CAMPFIRE3), 0);
				nobj->timer = oldlevel / 8;
				if (nobj->timer < 1)
					nobj->timer = 1;
				nobj->level = oldlevel;
				obj_to_room (nobj, inroom);
				continue;
			}
		}
		else if (obj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE3)
		{
			inroom = obj->in_room;
			if (inroom != NULL)
			{
				act
					("The blackened crisps of tinder and fuel break apart and scatter as slow winds buffet them.",
					inroom->people, NULL, NULL, TO_ALL);
				obj_from_room (obj);
				extract_obj (obj);
				continue;
			}
		}
		else if (obj->pIndexData->vnum == OBJ_VNUM_PENTAGRAM)
		{
			inroom = obj->in_room;
			if (inroom != NULL)
			{
				act("The blood that the pentagram was drawn with seeps into the earth.",
					inroom->people, NULL, NULL, TO_ALL);
				obj_from_room (obj);
				extract_obj (obj);
				continue;
			}
		}
		else if (obj->pIndexData->vnum == OBJ_VNUM_HEXAGRAM)
		{
			inroom = obj->in_room;
			if (inroom != NULL)
			{
				act("The blood that the hexagram was drawn with seeps into the earth.",
					inroom->people, NULL, NULL, TO_ALL);
				obj_from_room (obj);
				extract_obj (obj);
				continue;
			}
		}
		else if (obj->pIndexData->vnum == OBJ_VNUM_SEPTAGRAM)
		{
			inroom = obj->in_room;
			if (inroom != NULL)
			{
				act("The blood that the septagram was drawn with seeps into the earth.",
					inroom->people, NULL, NULL, TO_ALL);
				obj_from_room (obj);
				extract_obj (obj);
				continue;
			}
		}


		switch (obj->item_type)
		{
		default:
			message = "$p crumbles into dust.";
			break;
		case ITEM_FOUNTAIN:
			message = "$p dries up.";
			break;
		case ITEM_CORPSE_NPC:
			message = "$p decays into dust.";
			break;
		case ITEM_CORPSE_PC:
			message = "$p decays into dust.";
			break;
		case ITEM_FOOD:
			message = "$p decomposes.";
			break;
		case ITEM_POTION:
			message = "$p has evaporated from disuse.";
			break;
		case ITEM_PORTAL:
			message = "$p fades out of existence.";
			break;
		case ITEM_WEAPON:
			//Iblis 7/20/03 - Decaying arrows in Quivers fix
			if (obj->value[0] == WEAPON_ARROW
				&& obj->in_obj->item_type == ITEM_QUIVER)
				if (obj->in_obj->carried_by)
					obj->in_obj->carried_by->carry_number++;
			break;


		case ITEM_QUIVER:
		case ITEM_CONTAINER:
			if (CAN_WEAR (obj, ITEM_WEAR_FLOAT))
				if (obj->contains)
					message =
					"$p flickers and vanishes, spilling its contents on the floor.";
				else
					message = "$p flickers and vanishes.";
			else
				message = "$p crumbles into dust.";
			break;
		}

		if (obj && obj->pIndexData && obj->pIndexData->vnum == OBJ_VNUM_LITAN_FOOTSTEPS)
		{
			if (obj->value[0] != 1)
				message = "$p dies down and disappears.";
			else continue;  //if it's a snow footstep, display no message
		}
		if (obj->carried_by != NULL)
		{
			if (IS_NPC (obj->carried_by)
				&& obj->carried_by->pIndexData->pShop != NULL)
				obj->carried_by->silver += obj->cost / 5;
			else
			{
				act (message, obj->carried_by, obj, NULL, TO_CHAR);
				if (obj->wear_loc == WEAR_FLOAT)
					act (message, obj->carried_by, obj, NULL, TO_ROOM);
			}
		}
		else if (obj->in_room != NULL && (rch = obj->in_room->people) != NULL)
		{
			if (!
				(obj->in_obj
				&& obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
				&& !CAN_WEAR (obj->in_obj, ITEM_TAKE)))
			{
				act (message, rch, obj, NULL, TO_ROOM);
				act (message, rch, obj, NULL, TO_CHAR);
			}
		}

		// Here Paarshad .... tell Bella that i am right :)


		if ((obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC
			||CAN_WEAR (obj, ITEM_WEAR_FLOAT)) /*obj->wear_loc == WEAR_FLOAT)*/ && obj->contains)
		{
			OBJ_DATA *t_obj, *next_obj;

			for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
			{
				next_obj = t_obj->next_content;

				//	      if (t_obj->pIndexData->vnum == OBJ_VNUM_RUNE_CONV)
				//		{
				//		  obj_from_obj (t_obj);
				//		  extract_obj (t_obj);
				//		  continue;
				//		}

				obj_from_obj (t_obj);

				if (obj->in_obj)
					obj_to_obj (t_obj, obj->in_obj);
				else if (obj->carried_by)
					if (CAN_WEAR (obj, ITEM_WEAR_FLOAT))
					{
						if (obj->carried_by->in_room == NULL)
							extract_obj (t_obj);
						else
							obj_to_room (t_obj, obj->carried_by->in_room);
					}
					else
						obj_to_char (t_obj, obj->carried_by);

				else if (obj->in_room == NULL)
					extract_obj (t_obj);

				else
					obj_to_room (t_obj, obj->in_room);

			}
		}

		extract_obj (obj);
	}

	return;
}

inline bool is_aggro (CHAR_DATA * ch)
{
	if (IS_SET (ch->act, ACT_AGGRESSIVE) || ch->mood == 1 || IS_SET (ch->act2, ACT_AGGIE_EVIL)
		|| IS_SET (ch->act2, ACT_AGGIE_NEUTRAL) || IS_SET (ch->act2, ACT_AGGIE_GOOD))
		return (1);

	return (0);
}

void aggr_update (void)
{
	CHAR_DATA *wch;
	CHAR_DATA *wch_next;
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	CHAR_DATA *victim;


	for (wch = char_list; wch != NULL; wch = wch_next)
	{
		wch_next = wch->next;

		if (IS_NPC (wch) || wch->level >= LEVEL_IMMORTAL
			|| wch->in_room == NULL || wch->in_room->area->empty)
			continue;

		for (ch = wch->in_room->people; ch != NULL; ch = ch_next)
		{
			int count;

			ch_next = ch->next_in_room;

			if (!IS_NPC (ch)
				|| !is_aggro (ch)
				|| IS_SET (ch->in_room->room_flags, ROOM_SAFE)
				|| IS_AFFECTED (ch, AFF_CALM)
				|| is_affected (ch, skill_lookup ("Quiet Mountain Lake"))
				|| ch->fighting != NULL || IS_AFFECTED (ch, AFF_CHARM)
				|| !IS_AWAKE (ch)
				|| (IS_SET (ch->act, ACT_WIMPY) && IS_AWAKE (wch))
				|| !can_see (ch, wch) || number_bits (1) == 0)
				continue;

			count = 0;
			victim = NULL;
			for (vch = wch->in_room->people; vch != NULL; vch = vch_next)
			{
				vch_next = vch->next_in_room;

				if (!IS_NPC (vch) &&
					(vch->clan ==
					CLAN_BOGUS) ? 1 : (vch->in_room->area->clan !=
					vch->clan)
					&& (vch->level < LEVEL_IMMORTAL)
					&& (IS_SET(ch->act,ACT_AGGRESSIVE) 
					|| (IS_SET(ch->act2,ACT_AGGIE_GOOD) && vch->alignment > 350)
					|| (IS_SET(ch->act2,ACT_AGGIE_NEUTRAL) && vch->alignment >= -350 && vch->alignment <= 350)
					|| (IS_SET(ch->act2,ACT_AGGIE_EVIL) && vch->alignment < -350))

					&& (!(IS_SET(ch->act2,ACT_IMAGINARY) && vch->Class == PC_CLASS_CHAOS_JESTER))
					&& ((ch->level >= (vch->level - 5))
					|| IS_SET (ch->imm_flags, IMM_LEVELS)) 
					&& (!IS_SET (ch->act, ACT_WIMPY) || !IS_AWAKE (vch))
					&& can_see (ch, vch))
				{
					if (!(IS_NPC (vch)) && vch != ch && !resist_aggro (vch, ch))
					{
						if (number_range (0, count) == 0)
						{
							victim = vch;
							count++;
						}
					}
				}
			}

			if (victim == NULL || IS_AFFECTED (victim, AFF_CAMOUFLAGE))
				continue;

			// Druid addition.
			if (IS_NPC(ch) && is_animal(ch) &&
				!IS_NPC(victim) && (victim->Class == PC_CLASS_DRUID) &&
					check_animal_empathy(victim, ch))
				continue;

			if (IS_IMMORTAL (victim))
				continue;

			multi_hit (ch, victim, TYPE_UNDEFINED);
			if (ch->fighting == NULL)
				set_fighting (victim, ch);
			else update_aggressor(ch,victim);
		}
	}

	return;
}

bool resist_aggro (CHAR_DATA * victim, CHAR_DATA * ch)
{
	int cha;
	cha = get_curr_stat (victim, STAT_CHA);
	if (IS_SET (ch->imm_flags, IMM_LEVELS) || cha - 20 < 0)
		return 0;
	else
		return (number_percent () <= (cha - 20) * 5);
}


void StripLeadingBlanks (char *str)
{
	char *cPtr, *cBuf;


	if (strlen (str) < 1)
		return;

	cBuf = (char *) malloc (strlen (str) + 2);
	strcpy (cBuf, str);

	cPtr = cBuf;

	while (*cPtr != '\0' && isspace (*cPtr))
		cPtr++;

	strcpy (str, cPtr);

	free (cBuf);
}

void StripTrailingBlanks (char *str)
{
	char *cPtr;
	unsigned char found = FALSE;

	if (strlen (str) < 1)
		return;

	cPtr = str;

	/* Check string for non-space characters */
	while (*cPtr != '\0')
	{
		if (!isspace (*cPtr))
		{
			found = TRUE;
			break;
		}

		cPtr++;
	}

	if (!found)
		return;

	cPtr = str + strlen (str) - 1;

	while (isspace (*cPtr))
	{
		*cPtr = '\0';
		cPtr--;
	}
}

struct infochan
{
	char *text;
	struct infochan *next;
};


struct infochan *infochan_now = NULL;
struct infochan *infochan_list = NULL;


void show_infochan ()
{
	CHAR_DATA *fch;
	struct infochan *new_infochan, *new_infochan_next;
	char lineBuf[512], *cPtr;
	FILE *fp;


	if (NULL == infochan_now) {
		for (new_infochan = infochan_list; new_infochan != NULL; new_infochan = new_infochan_next) {
			new_infochan_next = new_infochan->next;

			free_string (new_infochan->text);
			free (new_infochan);
		}

		infochan_list = NULL;

		if (NULL == (fp = fopen ("info.txt", "r")))
			return;

		while ((fgets (lineBuf, 512, fp)) != NULL) {
			if (lineBuf[0] != '\0' && lineBuf[0] != '\n')
				StripLeadingBlanks (lineBuf);
			if (lineBuf[0] != '\0')
				StripTrailingBlanks (lineBuf);

			if ((cPtr = strchr (lineBuf, '\n')) != NULL)
				*cPtr = '\0';

			if (lineBuf[0] == '\0')
				continue;

			if ((new_infochan = malloc (sizeof (struct infochan))) == NULL) {
//			if (NULL == (new_infochan = (malloc(sizeof(struct infochan))))) {
				infochan_now = NULL;
				fclose (fp);
				return;
			}

			new_infochan->text = str_dup (lineBuf);
			new_infochan->next = infochan_list;
			infochan_list = new_infochan;
		}

		infochan_now = infochan_list;
		fclose (fp);
	}

	if (infochan_now != NULL) {
		for (fch = char_list; fch != NULL; fch = fch->next) {
			if (IS_NPC (fch) || IS_SET (fch->comm, COMM_NOINFO))
				continue;

			send_to_char("`b[Info]`` ", fch);
			send_to_char(infochan_now->text, fch);
			send_to_char("\n\r", fch);
		}

		infochan_now = infochan_now->next;
	}
}

void quad_update ()
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d, *d_next;

	if (quad) {
		if (quad_timer > 0) {
			quad_timer -= 1;
		} else {
			for (d = descriptor_list; d != NULL; d = d_next) {
				d_next = d->next;
				if (d->connected == CON_PLAYING) {
					sprintf (buf, "`kYour experience gain normalizes.``\n\r");
					send_to_char (buf, d->character);
				}
			}
			quad = FALSE;
		}
	}
	return;
}

void double_update ()
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d, *d_next;

	if (doub)
	{
		if (double_timer > 0)
			double_timer -= 1;
		else
		{
			for (d = descriptor_list; d != NULL; d = d_next)
			{
				d_next = d->next;
				if (d->connected == CON_PLAYING)
				{
					sprintf (buf, "`jYour experience gain normalizes.``\n\r");
					send_to_char (buf, d->character);
				}
			}
			doub = FALSE;
		}
	}
	return;
}


void half_update ()
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d, *d_next;

	if (half)
	{
		if (half_timer > 0)
			half_timer -= 1;
		else
		{
			for (d = descriptor_list; d != NULL; d = d_next)
			{
				d_next = d->next;
				if (d->connected == CON_PLAYING)
				{
					sprintf (buf, "`aYour experience gain normalizes.``\n\r");
					send_to_char (buf, d->character);
				}
			}
			half = FALSE;
		}
	}
	return;
}

//Iblis - Update for starting the super duel
void superduel_update ()
{
	bool fix_fp = FALSE;
	char tcbuf[15];
	if (!superduel_ticks)
		return;
	if (--superduel_ticks <= 0)
	{
		ROOM_INDEX_DATA* start_room;
		int counter = 0;
		short last_team=-1;
		bool same_team=FALSE;
		CHAR_DATA* peeps, *first_person;
		superduel_ticks = 999;
		//Transfer everyone into the arena
		start_room = get_room_index(23055);
		if (!start_room->people)
		{
			arena_chan("The super duel has been canceled due to lack of participation.");
			return;
		}
		first_person = start_room->people;
		if (superduel_teams)
		{
			for (peeps = start_room->people; peeps != NULL; peeps = peeps->next_in_room)
			{
				if (IS_NPC(peeps))
					continue;
				if (!IS_IMMORTAL(peeps) && !IS_NPC(peeps))
					counter++;
				if (last_team != -1 && last_team != peeps->pcdata->sd_team)
					break;
				last_team = peeps->pcdata->sd_team;
			}
			if (peeps == NULL && counter > 1)
			{
				arena_chan("The super duel has been canceled due to everyone chosing the same team.");
				for (peeps = start_room->people; peeps != NULL; peeps = start_room->people, counter++)
				{
					send_to_char("You're being transfered back to the sign-up room.\n\r",peeps);
					sprintf(tcbuf,"%d",ROOM_VNUM_SD_START);
					do_goto(first_person,tcbuf);
				}
			}


		}
		counter = 0;
		for (peeps = start_room->people; peeps != NULL; peeps = start_room->people, counter++)
		{
			if (!IS_NPC(peeps))
			{
				peeps->pcdata->backup_hit = peeps->hit;
				peeps->pcdata->backup_mana = peeps->mana;
				peeps->pcdata->backup_move = peeps->move;
			}
			peeps->hit = peeps->max_hit;
			peeps->mana = peeps->max_mana;
			peeps->move = peeps->max_move;
			if (!IS_IMMORTAL(peeps) && !IS_NPC(peeps))
			{
				peeps->dueler = TRUE;
				if (superduel_teams)
					peeps->pcdata->events_won[TEAM_SD_IN]++;
				else peeps->pcdata->events_won[SD_IN]++;
			}
			else 
			{
				counter--;
				if (first_person == peeps)
					fix_fp = TRUE;
			}
			if (superduel_teams)
			{
				if (last_team != peeps->pcdata->sd_team)
					same_team = FALSE;
				else last_team = peeps->pcdata->sd_team;
			}
			act ("$n is transported into the `bS`iU`bP`iE`bR`i D`bU`iE`bL`i!``", peeps, NULL, NULL, TO_ROOM);
			char_from_room (peeps);
			char_to_room (peeps, get_room_index(number_range(23027,23054)));
			stop_follower(peeps);
			trip_triggers(peeps, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
			trap_check(peeps,"room",peeps->in_room,NULL);
			sprintf(tcbuf,"%d",peeps->in_room->vnum);
			trip_triggers_arg(peeps, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
			check_aggression(peeps);
			if (fix_fp)
			{
				fix_fp = FALSE;
				first_person = start_room->people;
			}
		}
		if (counter == 1)
		{
			sprintf(tcbuf,"%d",ROOM_VNUM_SD_START);
			do_goto(first_person,tcbuf);
			if (!IS_NPC(first_person))
			{
				first_person->hit = first_person->pcdata->backup_hit;
				first_person->mana = first_person->pcdata->backup_mana;
				first_person->move = first_person->pcdata->backup_move;
			}

			send_to_char("Apparently no one wanted to join you in the super duel.\n\r",first_person);
			superduel_ticks = 0;
			if (first_person)
				first_person->dueler = FALSE;
			if (first_person && first_person->pcdata)
			{
				if (superduel_teams)
					first_person->pcdata->events_won[TEAM_SD_IN]--;
				else  first_person->pcdata->events_won[SD_IN]--;
			}
			arena_chan("The super duel has been canceled due to lack of participation.");
			return;
		}
		else if (counter < 1)
		{
			//Some stupid fucking imm or mob started the super_duel;
			return;
		}
		arena_chan("The super duel begins!");
	}
	else if (superduel_ticks != 998)
	{
		char buf[MAX_STRING_LENGTH];
		if (superduel_ticks == 1)
			sprintf(buf,"There is %i tick remaining til the next superduel begins.",superduel_ticks);
		else  sprintf(buf,"There are %i ticks remaining til the next superduel begins.",superduel_ticks);
		arena_chan(buf);
	}
	else superduel_ticks = 999;
}

//Iblis 4/4/4 - Added to allow trees
void room_update ()
{
	ROOM_INDEX_DATA *pRoomIndex;
	OBJ_DATA *tree;
	char buf[MAX_STRING_LENGTH];
	int counter=0, error_check=0, base=0;
	for (counter=0;counter < MAX_KEY_HASH;counter++)
	{
		error_check = 0;
		for (pRoomIndex = room_index_hash[counter]; pRoomIndex != NULL; pRoomIndex = pRoomIndex->next)
		{
			if (++error_check > 10000)
			{
				bug("Error in hash %d in room_update.",counter);
				break;
			}
			if (pRoomIndex->sector_type != SECT_FOREST)
				continue;
			//Do tree shit here
			//value[0] = Type of tree
			//value[1] = Ticks in existence
			//value[2] = type (sapling/small/large/huge/dead)
			if ((tree = get_obj_vnum_room(pRoomIndex,OBJ_VNUM_TREE)) != NULL)
			{
				//Over time, trees grow back so previous chops go away
				if (tree->value[3] > 0)
				{
					if (number_percent() < 3)
						--tree->value[3];
				}

				if (IS_SET(tree->value[4],TREE_VALUE_FALLEN) && !IS_SET(tree->value[4],TREE_VALUE_ROTTEN))
				{
					if (++tree->value[1] >= 100)
					{
						SET_BIT(tree->value[4],TREE_VALUE_ROTTEN);
						sprintf(buf,"A rotten dead %s tree lies here.",tree_table[tree->value[0]].name);
						free_string(tree->description);
						tree->description = str_dup(buf);
						sprintf(buf,"a rotten dead %s tree",tree_table[tree->value[0]].name);
						free_string(tree->short_descr);
						tree->short_descr = str_dup(buf);
						tree->value[1] = 500;
					}
				}

				if (++tree->value[1] < 100 && tree->value[1]%24 == 2)
				{
					if (number_percent() >= 98)
					{
						obj_from_room(tree);
						extract_obj(tree);
					}
				}
				else if (tree->value[1] == 100)
				{
					tree->value[2] = TREE_SMALL;
					sprintf(buf,"A small %s tree grows here.",tree_table[tree->value[0]].name);
					free_string(tree->description);
					tree->description = str_dup(buf);
					sprintf(buf,"a small %s tree",tree_table[tree->value[0]].name);
					free_string(tree->short_descr);
					tree->short_descr = str_dup(buf);
				}
				else if (tree->value[1] == 200)
				{
					tree->value[2] = TREE_MEDIUM;
					sprintf(buf,"A medium-sized %s tree grows here.",tree_table[tree->value[0]].name);
					free_string(tree->description);
					tree->description = str_dup(buf);
					sprintf(buf,"a medium-sized %s tree",tree_table[tree->value[0]].name);
					free_string(tree->short_descr);
					tree->short_descr = str_dup(buf);
				}
				else if (tree->value[1] == 300)
				{
					tree->value[2] = TREE_LARGE;
					sprintf(buf,"A large %s tree grows here.",tree_table[tree->value[0]].name);
					free_string(tree->description);
					tree->description = str_dup(buf);
					sprintf(buf,"a large %s tree",tree_table[tree->value[0]].name);
					free_string(tree->short_descr);
					tree->short_descr = str_dup(buf);
				}
				else if (tree->value[1] == 400)
				{
					tree->value[2] = TREE_HUGE;
					sprintf(buf,"A huge %s tree grows here.",tree_table[tree->value[0]].name);
					free_string(tree->description);
					tree->description = str_dup(buf);
					sprintf(buf,"a huge %s tree",tree_table[tree->value[0]].name);
					free_string(tree->short_descr);
					tree->short_descr = str_dup(buf);
				}
				else if (tree->value[1] == 500)
				{
					if (pRoomIndex->people != NULL)
					{
						sprintf(buf,"A loud crackling noise nearby makes you jump as %s falls to the ground.",
							tree->short_descr);
						do_recho(pRoomIndex->people,buf);
					}
					SET_BIT(tree->value[4],TREE_VALUE_FALLEN);
					SET_BIT(tree->value[4],TREE_VALUE_ROTTEN);
					sprintf(buf,"A rotten dead %s tree lies here.",tree_table[tree->value[0]].name);
					free_string(tree->description);
					tree->description = str_dup(buf);
					sprintf(buf,"a rotten dead %s tree",tree_table[tree->value[0]].name);
					free_string(tree->short_descr);
					tree->short_descr = str_dup(buf);
				}
				else if (tree->value[1] == 600)
				{
					obj_from_room(tree);
					extract_obj(tree);
				}
			}
			else 
			{
				//Chance of growing a sapling
				if (number_percent() == 69)
				{
					if (pRoomIndex->vnum >= ROOM_VNUM_PLAYER_START && pRoomIndex->vnum <= ROOM_VNUM_PLAYER_END)
						continue;
					if (IS_SET(pRoomIndex->area->area_flags,AREA_NO_TREE))
						continue;
					tree = create_object(get_obj_index(OBJ_VNUM_TREE),0);
					tree->value[2] = TREE_SAPLING;
					tree->value[1] = 0;
					do
					{
						tree->value[0] = number_range(0,MAX_TREES-1);
						switch (tree_table[tree->value[0]].rarity)
						{
						case RARITY_VERY_RARE:
							base = 3;
							break;
						case RARITY_RARE:
							base = 10;
							break;
						case RARITY_UNCOMMON:
							base = 25;
							break;
						default:
						case RARITY_SOMEWHAT_COMMON:
							base = 50;
							break;
						case RARITY_COMMON:
							base = 75;
							break;
						}
					} while (number_percent() >= base 
						|| (str_cmp(tree_table[tree->value[0]].area,"all") && str_cmp(tree_table[tree->value[0]].area, pRoomIndex->area->name)));

					sprintf(buf,"An %s sapling grows here.",tree_table[tree->value[0]].name);
					free_string(tree->description);
					tree->description = str_dup(buf);
					sprintf(buf,"an %s sapling",tree_table[tree->value[0]].name);
					free_string(tree->short_descr);
					tree->short_descr = str_dup(buf);
					sprintf(buf,"tree %s",tree_table[tree->value[0]].name);
					free_string(tree->name);
					tree->name = str_dup(buf);
					obj_to_room(tree,pRoomIndex);
				}
			}

		}
	}


}

//Iblis - Update function for the poll channel
void poll_update ()
{
	if (poll_ticks == 0)
		return;
	else if (--poll_ticks > 0)
	{
		char buf[MAX_STRING_LENGTH];
		DESCRIPTOR_DATA *d;
		for (d = descriptor_list; d != NULL; d = d->next)
		{
			CHAR_DATA *victim;
			victim = d->original ? d->original : d->character;
			if (d->connected == CON_PLAYING
				&& !IS_SET (victim->comm2, COMM_NOPOLL)
				&& !IS_SET (victim->comm, COMM_QUIET))
			{
				sprintf(buf,"`a[`hPoll`a]`a:`a %d %s remaining to vote on '`o$t`a'``",poll_ticks,
					(poll_ticks == 1)?"tick":"ticks");
				act_new (buf,d->character, poll_question, NULL, TO_CHAR, POS_SLEEPING);
			}
		}
	}
	else
	{
		CHAR_DATA *wch;
		int votes_yes=0,votes_no=0;
		DESCRIPTOR_DATA *d;
		char buf[MAX_STRING_LENGTH];
		for (wch = char_list; wch != NULL; wch = wch->next)
		{
			if (IS_NPC(wch))
				continue;
			if (wch->pcdata->voted == VOTED_YES)
				++votes_yes;
			else if (wch->pcdata->voted == VOTED_NO)
				++votes_no;
			wch->pcdata->voted = VOTED_NOT;
		}
		sprintf(buf,"`a[`hPoll Results`a]`a: Yes:(`g%d`a) No:(`g%d`a) `hQuestion was: `o%s",
			votes_yes,votes_no,poll_question);
		for (d = descriptor_list; d != NULL; d = d->next)
		{
			CHAR_DATA *victim;
			victim = d->original ? d->original : d->character;
			if (d->connected == CON_PLAYING
				&& !IS_SET (victim->comm2, COMM_NOPOLL)
				&& !IS_SET (victim->comm, COMM_QUIET))
			{
				act_new (buf,d->character, NULL, NULL, TO_CHAR, POS_SLEEPING);
			}
		}
		strcpy(poll_question,"");
	}
}

//Iblis - Update function for Battle Royale    
void battle_royale_update ()
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d, *d_next;

	if (battle_royale)
	{
		if (battle_royale_timer > 0)
		{
			battle_royale_timer -= 1;
			if (battle_royale_timer < 5)
			{
				sprintf (buf,
					"`iThere are `g%d `iticks of Battle Royale remaining.``\r\n",
					battle_royale_timer);
				for (d = descriptor_list; d; d = d->next)
				{
					if (d->connected == CON_PLAYING && d->character)
					{
						send_to_char (buf, d->character);
					}
				}
			}
			for (d = descriptor_list; d != NULL; d = d_next)
			{
				d_next = d->next;
				if (d->character == NULL || d->connected != CON_PLAYING)
					continue;
				if (!IS_IMMORTAL (d->character)
					&& d->character->level > 19
					&& d->character->in_room->vnum > 24999
					&& d->character->in_room->vnum < 27000)
					br_transfer (d->character);
			}

		}
		else
		{
			if (last_br_kill != NULL)
			{
				last_br_kill->pcdata->br_points += last_br_points;
				send_to_char
					("`gFor getting the last kill of the Battle Royale, you just earned double points for it!\r\n",
					last_br_kill);
				last_br_victim->pcdata->br_death_points += last_br_points;
				send_to_char
					("`gFor getting the last kill of the Battle Royale, your last killer just earned double points for it!\r\n",
					last_br_victim);
				if (last_br_kill->pcdata->br_points >
					br_leader->pcdata->br_points)
					br_leader = last_br_kill;
			}
			if (br_leader != NULL)
			{
				sprintf (buf,
					"`gAnd the winner of the Battle Royale is -> `b%s `g with `b%d `g points.\r\n",
					br_leader->name, br_leader->pcdata->br_points);
				br_leader->pcdata->events_won[BR_WON]++;
				do_echo (br_leader, buf);
				send_to_char
					("`gYou will be rewarded when I think of something good to give you.\r\n",
					br_leader);
				do_oocnote (br_leader, "clear");
				do_oocnote (br_leader, "to all");
				do_oocnote (br_leader,
					"subject  `d[`h/`d]`d[`h\\`d]`d[`h/`d]`d[`h\\`d] `iBattle Royale Results `d[`h/`d]`d[`h\\`d]`d[`h/`d]`d[`h\\`d]``");
				do_oocnote (br_leader,
					"+ `d[`h/`d]`d[`h\\`d]`d[`h/`d]`d[`h\\`d] `iBattle Royale Results `d[`h/`d]`d[`h\\`d]`d[`h/`d]`d[`h\\`d]``");
				do_oocnote (br_leader, "+ ``");
				sprintf (buf,
					"+ `oAnd the winner is -> `i%s `owith `b%d `opoints!``",
					br_leader->name, br_leader->pcdata->br_points);
				do_oocnote (br_leader, buf);
				do_oocnote (br_leader, "+ ``");
				do_oocnote (br_leader,
					"+ `bName         `` - `iPoints Received`` - `lPoints Given");
				do_oocnote (br_leader, "+ ``");
			}
			for (d = descriptor_list; d != NULL; d = d_next)
			{
				d_next = d->next;
				if (d->connected == CON_PLAYING && d->character)
				{
					sprintf (buf,
						"`k-+-+- `oThe drums of war fall silent as `iBattle Royale `ocomes to an end. `k-+-+-``\n\r");
					send_to_char (buf, d->character);
					if (!IS_IMMORTAL (d->character)
						&& (d->character->level > 19 
						|| (!IS_NPC(d->character) && d->character->pcdata->loner))
						&& (d->character->in_room->vnum >
						ROOM_VNUM_JAIL_END
						|| d->character->in_room->vnum <
						ROOM_VNUM_JAIL_START))
					{
						sprintf (buf, "%s 21442", d->character->name);

						d->character->fighting = NULL;
						do_transfer (d->character, buf);
						do_stripaffects(d->character,d->character->name);
						d->character->pcdata->events_won[BR_IN]++;
						d->character->position = POS_RESTING;
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
					}
				}
			}
			//Iblis 8/3/03
			//Sorting and Automated note posting
			if (br_leader)
			{
				CHAR_DATA *br_char_list[80];	//Yes Morgan, this is hardcoded.  Assail me if you must.
				short br_char_counter = 0, next_highest_index=0;
				int next_highest = -29999;
				bool found = TRUE;
				do_restore (br_leader, "all");
				for (br_char_counter = 0; br_char_counter < 80;
					br_char_counter++)
					br_char_list[br_char_counter] = NULL;
				br_char_counter = 0;
				for (d = descriptor_list; d != NULL; d = d_next)
				{
					d_next = d->next;
					if (d->connected == CON_PLAYING && d->character
						&& d->character->pcdata && !IS_IMMORTAL (d->character))
					{
						if (br_char_counter > 79)
						{
							bug ("Too many goddamn players for Battle Royale",
								0);
							break;
						}
						br_char_list[br_char_counter++] = d->character;
					}
				}
				while (found)
				{
					found = FALSE;
					next_highest = -29999;
					for (br_char_counter = 0; br_char_counter < 80;
						br_char_counter++)
					{
						if (br_char_list[br_char_counter] != NULL)
						{
							found = TRUE;
							if (br_char_list[br_char_counter]->pcdata->
								br_points > next_highest)
							{
								next_highest =
									br_char_list[br_char_counter]->pcdata->
									br_points;
								next_highest_index = br_char_counter;
							}
						}
					}
					if (found)
					{
						sprintf (buf, "+ `b%-13s`` - `i%9d``       - `l%7d",
							br_char_list[next_highest_index]->name,
							br_char_list[next_highest_index]->pcdata->
							br_points,
							br_char_list[next_highest_index]->pcdata->
							br_death_points);
						if (br_leader == NULL)
							bug ("br_leader == NULL", 0);
						bug (buf, 0);
						do_oocnote (br_leader, buf);
						br_char_list[next_highest_index]->pcdata->br_points = 0;
						br_char_list[next_highest_index] = NULL;
					}
				}



			}


			if (br_leader)
				do_oocnote (br_leader, "post");
			br_leader = NULL;
			last_br_kill = NULL;
			last_br_victim = NULL;
			br_death_counter = 0;
			battle_royale = FALSE;
			strcpy (imm_who_msg, "remove");
		}
	}
	return;
}


void shutdown_graceful ()
{
	extern bool merc_down;
	FILE *fp = NULL;
	DESCRIPTOR_DATA *d, *d_next;
	CHAR_DATA *player;

	// touch the shutdown.txt
	fclose (fpReserve);
	fp = fopen (SHUTDOWN_FILE, "a");
	if (fp == NULL)
	{
		perror (SHUTDOWN_FILE);
	}
	else
	{
		fprintf (fp, "Automated shutdown");
		fclose (fp);
	}
	fpReserve = fopen (NULL_FILE, "r");

	// save the player, let them know we were nice
	player = char_list;
	while (player)
	{
		if (!IS_NPC (player))
		{
			save_char_obj (player);
			send_to_char ("Your character was saved.\n\r", player);
		}
		player = player->next;
	}

	// close all of the socket descriptors
	merc_down = TRUE;
	for (d = descriptor_list; d != NULL; d = d_next)
	{
		d_next = d->next;
		close_socket (d);
	}
	return;
}


void update_handler (void)
{
	static int pulse_area;
	static int pulse_mobile;
	static int pulse_violence;
	static int pulse_point;
	static int pulse_speed;
	static int pulse_clanwar;
	static int pulse_reboot;
	static int pulse_shutdown;
	static int pulse_bet;
	static int pulse_second;
	static int pulse_5_seconds = 0;
	static int pulse_15_seconds = 0;
	static int pulse_30_seconds = 0;
	if (--pulse_second <= 0)
	{
		pulse_second = PULSE_PER_SECOND;
		chant_update ();
		sink_update ();
		falcon_update ();
		knock_update ();
		obj_trig_second_update ();
		if (++pulse_5_seconds > 5)
		{
			update_reeling ();
			pulse_5_seconds = 0;
			update_cardgame_round();
		}
		if (++pulse_15_seconds > 15)
		{
			update_shillelagh();
			update_trade_skills();
			//update_cardgame_round();
			pulse_15_seconds = 0;
		}
		if (++pulse_30_seconds > 30)
		{
			update_fishing ();
			pulse_30_seconds = 0;
		}
	}

	if (--pulse_speed <= 0)
	{
		pulse_speed = PULSE_SPEED;
		speedwalk_update ();
		mob_trig_update ();
	}

	if (--pulse_area <= 0)
	{
		pulse_area = PULSE_AREA;
		area_update ();
	}

	/*
	if (--pulse_music <= 0)
	{
	pulse_music = PULSE_MUSIC;
	song_update ();
	}
	*/
	if (--pulse_mobile <= 0)
	{
		pulse_mobile = PULSE_MOBILE;
		mobile_update ();
		elevator_update ();
	}

	if (--pulse_violence <= 0)
	{
		pulse_violence = PULSE_VIOLENCE;
		violence_update ();
	}

	if (--pulse_clanwar <= 0)
	{
		pulse_clanwar = PULSE_CLANWAR;
		update_wars ();
	}
	// Morgan, June 26. 2000  For the automated timed reboot function
	if (--pulse_reboot <= 0)
	{
		pulse_reboot = PULSE_REBOOT;
		check_reboot ();
	}
	// Morgan, July 21. 2000  For the automated timed shutdown function
	if (--pulse_shutdown <= 0)
	{
		pulse_shutdown = PULSE_SHUTDOWN;
		check_shutdown ();
	}

	if (--pulse_bet <= 0)
	{
		pulse_bet = PULSE_BET;
		bet_update ();
	}

	if (--pulse_point <= 0)
	{
		CHAR_DATA *mob;

		wiznet ("TICK!", NULL, NULL, WIZ_TICKS, 0, 0);

		for (mob = char_list; mob != NULL; mob = mob->next)
		{
			if (IS_NPC (mob) && HAS_SCRIPT (mob))
			{
				script_update (mob, TRIG_TICK_PULSE);
			}
		}
		pulse_point = PULSE_TICK;
		scent_update ();
		weather_update ();
		char_update ();
		obj_update ();
		obj_trig_tick_update ();	     
		half_update ();
		quad_update ();
		double_update ();
		battle_royale_update ();
		//Iblis 6/15/04
		cardgame_update ();
		superduel_update();
		aggressor_update ();
		poll_update();
		room_update();

		info_tick++;
		if (info_tick == 3)
		{
			show_infochan ();
			info_tick = 0;
		}
	}				// end pulse_point
	auction_update ();
	aggr_update ();
	return;
}

//Iblis - New command added to force a tick.  This was most useful for forced growing of trees,
//but can be useful for other things
void do_forcetick(CHAR_DATA* ch,char* argument)
{
	scent_update ();
	weather_update ();
	char_update ();
	obj_update ();
	obj_trig_tick_update ();
	half_update ();
	quad_update ();
	double_update ();
	battle_royale_update ();
	//Iblis 6/15/04
	cardgame_update();
	superduel_update();
	aggressor_update ();
	poll_update();
	room_update();
}

// Morgan, June 26. 2000  For the automated timed reboot function
void check_reboot (void)
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;

	switch (reboot_counter)
	{
	case -1:
		break;
	case 0:
		// Timer is 0 ?? well, then reboot.
		reboot_rom ();
		return;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 10:
	case 15:
		// Setup message for people that mud will reboot
		sprintf (buf, "\007Exodus will reboot in %d minute%s.\007\n\r",
			reboot_counter, reboot_counter > 1 ? "s" : "");
		// Send to all descriptors
		for (d = descriptor_list; d != NULL; d = d->next)
			write_to_buffer (d, buf, 0);
	default:
		// decrement reboot counter if result not found
		reboot_counter--;
		break;
	}
}

// Morgan, July 21. 2000  For the automated timed shutdown function
void check_shutdown (void)
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;

	switch (shutdown_counter)
	{
	case -1:
		break;
	case 0:
		// Timer is 0 ?? well, then shutdown.
		shutdown_graceful ();
		return;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 10:
	case 15:
		// Setup message for people that mud will reboot
		sprintf (buf, "\007Exodus will shut down in %d minute%s.\007\n\r",
			shutdown_counter, shutdown_counter > 1 ? "s" : "");
		// Send to all descriptors
		for (d = descriptor_list; d != NULL; d = d->next)
			write_to_buffer (d, buf, 0);
	default:
		// decrement shutdown counter if result not found
		shutdown_counter--;
		break;
	}
}

//Iblis - I finally coded my dream..a chance to retaliate against fuckers who hit you then run off,
//without penalty. 
void aggressor_update (void)
{
	DESCRIPTOR_DATA *d, *d_next;
	AGGRESSOR_LIST *a, *a_next, *a_prev=NULL;


	for (d = descriptor_list; d != NULL; d = d_next)
	{
		d_next = d->next;
		if (d->character == NULL || d->character->pcdata == NULL)
			continue;
		if ((a = d->character->pcdata->agg_list) != NULL)
		{
			for (; a != NULL; a = a_next)
			{
				a_next = a->next;
				if (a->ch != d->character->fighting
					&& d->character != a->ch->fighting)
				{
					if (--a->ticks_left < 0)
					{
						if (!a->silent)
						{
							act
								("Your time for free revenge against $N has come to an end.",
								d->character, NULL, a->ch, TO_CHAR);
							act
								("$N's times for free revenge against you has come to an end.",
								a->ch, NULL, d->character, TO_CHAR);
						}
						if (a == d->character->pcdata->agg_list)
							d->character->pcdata->agg_list = a_next;
						else
							(a_prev->next = a_next);
						free_aggressor_list (a);
					}
					else
						a_prev = a;
				}
			}
		}
	}
}

//Iblis - The function that loads trees
void load_trees()
{

	ROOM_INDEX_DATA *pRoomIndex;
	OBJ_DATA *tree;
	char buf[MAX_STRING_LENGTH];
	int counter=0, error_check=0, base=0;
	for (counter=0;counter < MAX_KEY_HASH;counter++)
	{
		error_check = 0;
		for (pRoomIndex = room_index_hash[counter]; pRoomIndex != NULL; pRoomIndex = pRoomIndex->next)
		{
			if (++error_check > 10000)
			{
				bug("Error in hash %d in room_update.",counter);
				break;
			}
			if (pRoomIndex->sector_type != SECT_FOREST)
				continue;
			if (number_percent() <= 30)
			{
				if (pRoomIndex->vnum >= ROOM_VNUM_PLAYER_START && pRoomIndex->vnum <= ROOM_VNUM_PLAYER_END)
					continue;
				if (IS_SET(pRoomIndex->area->area_flags,AREA_NO_TREE))
					continue;
				tree = create_object(get_obj_index(OBJ_VNUM_TREE),0);
				tree->value[1] = number_range(0,600);
				do
				{
					tree->value[0] = number_range(0,MAX_TREES-1);
					switch (tree_table[tree->value[0]].rarity)
					{
					case RARITY_VERY_RARE:
						base = 3;
						break;
					case RARITY_RARE:
						base = 10;
						break;
					case RARITY_UNCOMMON:
						base = 25;
						break;
					default:
					case RARITY_SOMEWHAT_COMMON:
						base = 50;
						break;
					case RARITY_COMMON:
						base = 75;
						break;
					}
				} while (number_percent() > base 
					|| (str_cmp(tree_table[tree->value[0]].area,"all") && str_cmp(tree_table[tree->value[0]].area, pRoomIndex->area->name)));

				sprintf(buf,"tree %s",tree_table[tree->value[0]].name);
				free_string(tree->name);
				tree->name = str_dup(buf);
				obj_to_room(tree,pRoomIndex);
				if (tree->value[1] < 100)
				{
					tree->value[2] = TREE_SAPLING;
					sprintf(buf,"An %s sapling grows here.",tree_table[tree->value[0]].name);
					free_string(tree->description);
					tree->description = str_dup(buf);
					sprintf(buf,"an %s sapling",tree_table[tree->value[0]].name);
					free_string(tree->short_descr);
					tree->short_descr = str_dup(buf);
				}
				if (tree->value[1] < 200)
				{
					tree->value[2] = TREE_SMALL;
					sprintf(buf,"A small %s tree grows here.",tree_table[tree->value[0]].name);
					free_string(tree->description);
					tree->description = str_dup(buf);
					sprintf(buf,"a small %s tree",tree_table[tree->value[0]].name);
					free_string(tree->short_descr);
					tree->short_descr = str_dup(buf);
				}
				else if (tree->value[1] < 300)
				{
					tree->value[2] = TREE_MEDIUM;
					sprintf(buf,"A medium-sized %s tree grows here.",tree_table[tree->value[0]].name);
					free_string(tree->description);
					tree->description = str_dup(buf);
					sprintf(buf,"a medium-sized %s tree",tree_table[tree->value[0]].name);
					free_string(tree->short_descr);
					tree->short_descr = str_dup(buf);
				}
				else if (tree->value[1] < 400)
				{
					tree->value[2] = TREE_LARGE;
					sprintf(buf,"A large %s tree grows here.",tree_table[tree->value[0]].name);
					free_string(tree->description);
					tree->description = str_dup(buf);
					sprintf(buf,"a large %s tree",tree_table[tree->value[0]].name);
					free_string(tree->short_descr);
					tree->short_descr = str_dup(buf);
				}
				else if (tree->value[1] < 500)
				{
					tree->value[2] = TREE_HUGE;
					sprintf(buf,"A huge %s tree grows here.",tree_table[tree->value[0]].name);
					free_string(tree->description);
					tree->description = str_dup(buf);
					sprintf(buf,"a huge %s tree",tree_table[tree->value[0]].name);
					free_string(tree->short_descr);
					tree->short_descr = str_dup(buf);
				}
				else if (tree->value[1] < 600)
				{
					if (pRoomIndex->people != NULL)
					{
						sprintf(buf,"A loud crackling noise nearby makes you jump as %s falls to the ground.",
							tree->short_descr);
						do_recho(pRoomIndex->people,buf);
					}
					SET_BIT(tree->value[4],TREE_VALUE_FALLEN);
					SET_BIT(tree->value[4],TREE_VALUE_ROTTEN);
					sprintf(buf,"A rotten dead %s tree lies here.",tree_table[tree->value[0]].name);
					free_string(tree->description);
					tree->description = str_dup(buf);
					sprintf(buf,"a rotten dead %s tree",tree_table[tree->value[0]].name);
					free_string(tree->short_descr);
					tree->short_descr = str_dup(buf);
				}


			}
		}
	}
}
