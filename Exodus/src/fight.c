#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include "merc.h"
#include "interp.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"
#include "obj_trig.h"
#include "Utilities.h"

extern struct clan_type clan_table[MAX_CLAN];
extern char *color_table[];
extern const char *dir_name[];
extern bool half;
extern bool quad;
extern bool doub;
extern bool battle_royale;
extern bool killed_by_chain; //A lousy fix for a mob chaining itself to death
extern bool ignore_hitroll;  //Used to test hitroll affects
extern bool ignore_ac;
extern bool new_ac;
bool no_check_skeletal=FALSE;
bool missed_attack = FALSE;

// Iblis 1/05/04 for Necromancers
void feign_death args((CHAR_DATA* ch, char* argument));
void feed_death args((CHAR_DATA* victim, CHAR_DATA* killer));
void check_skeletal_mutation args((CHAR_DATA* ch, bool on_round));
bool check_clasp args((CHAR_DATA *ch));
bool damage2 args((CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type, bool show));
bool damage_old2 args((CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type, bool show));

void stun_char args((CHAR_DATA *ch, CHAR_DATA *victim));
void reset_race args((CHAR_DATA* ch));
void death_spasms args((CHAR_DATA *ch));
void switched_return args((CHAR_DATA *mob, ROOM_INDEX_DATA *location));
CHAR_DATA *get_gravity_char_room args((ROOM_INDEX_DATA *rid));

bool equip_char_slot_new args((CHAR_DATA * ch, OBJ_DATA * obj, long slot));


void larva_hit args((CHAR_DATA *ch,CHAR_DATA *victim));

//IBLIS 6/20/03 - Needed for Battle Royale;
CHAR_DATA *br_leader = NULL;
CHAR_DATA *last_br_kill = NULL;
CHAR_DATA *last_br_victim = NULL;
int last_br_points = 0;
bool is_tail_attack=FALSE; // Cheap fix for Nerix's tail attacks

//Iblis 11/15/03 - Used to prevent critical strike on
//  weapon FLAG hits
bool crit_strike_possible = TRUE;

extern short br_death_counter;
extern short battle_royale_dead_amount;
bool nodam_message = FALSE;
extern AREA_DATA *area_first;
extern const sh_int movement_loss[SECT_MAX];

// Variable, holding the status of the safe-nopk flag
// Morgan on June 29. 2000
extern bool safe_nopk;
extern bool make_backup;
extern bool isorder;
int realxp;

#define MAX_DAMAGE_MESSAGE 45

//external function
bool can_move_char_door
args ((CHAR_DATA * ch, int door, bool follow, bool show_messages));
int wall_damage args ((CHAR_DATA * ch, OBJ_DATA * wall));
void display_track args ((CHAR_DATA * ch));
AGGRESSOR_LIST *new_aggressor args ((void));
void free_aggressor_list args ((AGGRESSOR_LIST * b));
void superduel_ends args((CHAR_DATA *winner, CHAR_DATA *loser));
void pk_chan args((char* text));

long int level_cost args ((int level));
void group_gain_exp args ((CHAR_DATA * ch, long int gain));
long int low_gain_exp args ((CHAR_DATA * ch, long int gain));
void check_assist args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_dodge args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_eldritch_miasma args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_parry args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_shield_block args ((CHAR_DATA * ch, CHAR_DATA * victim));
void dam_message
args ((CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, bool immune));
void death_cry args ((CHAR_DATA * ch, bool not_mob_killer));
bool group_gain args ((CHAR_DATA * ch, CHAR_DATA * victim));
int align_compute args ((CHAR_DATA * gch, CHAR_DATA * victim));

bool is_safe args ((CHAR_DATA * ch, CHAR_DATA * victim));
void make_corpse args ((CHAR_DATA * ch));
void one_hit args ((CHAR_DATA * ch, CHAR_DATA * victim, int dt));
void mob_hit args ((CHAR_DATA * ch, CHAR_DATA * victim, int dt));
void disarm args ((CHAR_DATA * ch, CHAR_DATA * victim));
void clan_leader args ((CHAR_DATA * ch, char *argument));
int check_entangle args ((CHAR_DATA * ch));
void do_murder args ((CHAR_DATA * ch, char *argument));
void clan_defect args ((CHAR_DATA * ch));
bool check_flicker args((CHAR_DATA * ch, CHAR_DATA * victim));

//IBLIS 5/30/03 - calculate and returns xp including bonuses for act,imm,res,dam,ac
long add_xp_bonuses args ((long xp, CHAR_DATA * victim));
void save_player_corpse args((OBJ_DATA * pObj));


//IBLIS 5/31/03 - monk's dirty tactic functions
void do_eyegouge args ((CHAR_DATA * ch, CHAR_DATA * victim));
void do_dirtytrip args ((CHAR_DATA * ch, CHAR_DATA * victim));

//IBLIS 5/31/03 - monk's functions
bool check_block args ((CHAR_DATA * ch, CHAR_DATA * victim));
void counter_attack args ((CHAR_DATA * ch, CHAR_DATA * victim));
void do_feign args ((CHAR_DATA * ch, char *argument));
void do_dive args ((CHAR_DATA * ch, char *argument));
void prevent_escape args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool awareness args ((CHAR_DATA * victim, CHAR_DATA * ch));
bool hear_the_wind
args ((CHAR_DATA * victim, CHAR_DATA * ch, OBJ_DATA * obj));
bool insta_kill
args ((CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type,
      bool show, bool magic));
void calculate_br_points args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool IS_AGGRESSOR args ((CHAR_DATA * ch, CHAR_DATA * victim));
void update_aggressor args ((CHAR_DATA * ch, CHAR_DATA * victim));
void update_aggressor2 args((CHAR_DATA * ch, CHAR_DATA * victim, bool show_messages));
void remove_aggressor args ((CHAR_DATA * ch, CHAR_DATA * victim));
void decrease_level args ((CHAR_DATA * ch));
OBJ_DATA *get_obj_vnum_char args((CHAR_DATA* ch, int vnum));

int adjust_damage args((int dam_to_do));

//Iblis 10/19/03 - Needed for Kalian corpse eating bonuses
void spell_giant_strength args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
void spell_aqua_breathe args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
void spell_pass_door args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
void spell_haste args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
void prayer_sanctuary args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
void prayer_frenzy args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
void spell_fireshield args((int sn, int level, CHAR_DATA * ch, void *vo, int target));

void spell_heal args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
void spell_plague args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
void spell_bark_skin args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
void spell_protection_evil args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
void spell_protection_good args((int sn, int level, CHAR_DATA * ch, void *vo, int target));


void set_wanted (CHAR_DATA * ch)
{
  if (ch->Class == PC_CLASS_CHAOS_JESTER)
    return;
  if (!IS_SET (ch->act, PLR_WANTED) && !IS_IMMORTAL (ch))
  {
    send_to_char ("*** You are now WANTED.\n\r", ch);
    SET_BIT (ch->act, PLR_WANTED);
    wiznet ("$N has become wanted.", ch, NULL, WIZ_PENALTIES, 0, 0);
    if (ch->times_wanted < 65)
    {
      ch->times_wanted++;
      ch->want_jail_time = 11 + (5 * ch->times_wanted) - ch->times_wanted;
    }

    else
      ch->want_jail_time = 999999;
  }
}
void unjail (CHAR_DATA * ch)
{
  char tcbuf[15];
  remove_wanted (ch);
  REMOVE_BIT (ch->comm, COMM_NOCHANNELS);
  if (IS_IMMORTAL (ch))
    REMOVE_BIT (ch->comm, COMM_NOWIZ);
  if (ch->in_room->vnum < 4255 || ch->in_room->vnum > 4266)
    return;
  send_to_char
    ("A prison guard enters and drags you gruffly into the guard room.\n\r",
    ch);
  act ("A prison guard enters and drags $n gruffly into the guard room.",
    ch, NULL, NULL, TO_ROOM);
  wiznet ("$N has been released from jail.", ch, NULL, WIZ_PENALTIES, 0, 0);
  char_from_room (ch);
  char_to_room (ch, get_room_index (4023));
  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  trap_check(ch,"room",ch->in_room,NULL);
  sprintf(tcbuf,"%d",ch->in_room->vnum);
  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
  check_aggression(ch);
}

void arrest (CHAR_DATA * ch, CHAR_DATA * victim)
{
  char buf[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;

  //OBJ_DATA *obj, *obj_next, *pit_obj;
  bool placed = FALSE;
  int vnum;
  if (ch->in_room != victim->in_room)
    return;
  act ("$n grabs $N and hauls $M off to jail.", ch, NULL, victim, TO_NOTVICT);
  act ("$n grabs you and hauls you off to jail.", ch, NULL, victim, TO_VICT);
  act ("You grab $N and haul $M off to jail.", ch, NULL, victim, TO_CHAR);
  sprintf (buf, "%s has been arrested by $N.", victim->name);
  wiznet (buf, ch, NULL, WIZ_PENALTIES, 0, 0);
  char_from_room (victim);
  for (vnum = ROOM_VNUM_JAIL_START; vnum < ROOM_VNUM_JAIL_END; vnum++)
  {
    location = get_room_index (vnum);
    if (location->people == NULL)
    {
      char_to_room (victim, location);
      placed = TRUE;
      break;
    }
  }
  if (placed == FALSE)
  {
    location =
      get_room_index (number_range
      (ROOM_VNUM_JAIL_START, ROOM_VNUM_JAIL_END));
    char_to_room (victim, location);
  }

  /*  if (victim->carrying != NULL)
  {
  for (pit_obj = object_list; pit_obj; pit_obj = pit_obj->next)
  if (pit_obj->pIndexData->vnum == OBJ_VNUM_PIT)
  break;

  if (pit_obj != NULL)
  {
  act
  ("$N drops a prisoner's confiscated items into the donation box.",
  pit_obj->in_room->people, NULL, ch, TO_ALL);

  for (obj = victim->carrying; obj; obj = obj_next)
  {
  obj_next = obj->next_content;
  unequip_char (victim, obj);
  obj_from_char (obj);
  obj_to_obj (obj, pit_obj);
  }
  }
  }*/
  if (IS_NPC (ch))
    extract_char (ch, TRUE);
  REMOVE_BIT (victim->act, PLR_WANTED);
  SET_BIT (victim->act, PLR_JAILED);
  SET_BIT (victim->comm, COMM_NOCHANNELS);
  if (!IS_NPC(victim))
    victim->pcdata->nochan_ticks = 0;
  victim->want_jail_time =
    UMAX (11 + (5 * victim->times_wanted) - victim->times_wanted, 15);
  if (victim->want_jail_time > 250)
    victim->want_jail_time = 250;
  save_char_obj (victim);
  trip_triggers(victim, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  trap_check(victim,"room",victim->in_room,NULL);
  sprintf(buf,"%d",victim->in_room->vnum);
  trip_triggers_arg(victim, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,buf);
  check_aggression(victim);
}

void remove_wanted (CHAR_DATA * ch)
{
  REMOVE_BIT (ch->act, PLR_WANTED);
  REMOVE_BIT (ch->act, PLR_JAILED);
  ch->want_jail_time = 0;
}

void check_saddle (CHAR_DATA * ch)
{
  int chance;
  if (IS_NPC (ch))
    return;
  if (ch->mount == NULL)
    return;
  if (get_eq_char (ch->mount, WEAR_ABOUT) != NULL)
    return;
  chance = get_skill (ch, gsn_riding) / 2 + 10;
  chance += get_curr_stat (ch, STAT_DEX);
  chance += ch->mount->pIndexData->defbonus;
  if (number_percent () > chance)
  {
    char tcbuf[MAX_STRING_LENGTH];
    sprintf (tcbuf,
      "You lose your balance and fall precariously off of %s.\n\r",
      ch->mount->short_descr);
    send_to_char (tcbuf, ch);
    sprintf (tcbuf,
      "$n loses $s balance and falls precariously off of %s.",
      ch->mount->short_descr);
    act (tcbuf, ch, NULL, NULL, TO_ROOM);
    ch->mount->riders = NULL;
    ch->mount = NULL;
    ch->position = POS_SITTING;
  }
}

/*
* Control the fights going on.
* Called periodically by update_handler.
*/
void violence_update (void)
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  CHAR_DATA *victim;
  for (ch = char_list; ch != NULL; ch = ch->next)
  {
    ch_next = ch->next;
    if ((victim = ch->fighting) == NULL || ch->in_room == NULL)
      continue;
    if (IS_NPC (ch) && HAS_SCRIPT (ch))
    {
      TRIGGER_DATA *pTrig;
      for (pTrig = ch->triggers; pTrig != NULL; pTrig = pTrig->next)
      {
        if (pTrig->trigger_type == TRIG_COMBAT
          && pTrig->current == NULL && !IS_SET (ch->act, ACT_HALT))
        {
          act_trigger (ch, pTrig->name, NULL, NAME (victim), NULL);
          pTrig->current = pTrig->script;
          pTrig->bits = SCRIPT_ADVANCE;
        }
      }
    }
    if (IS_NPC (ch) && ch->color == 42 && ch->wait <= 0)
    {
      do_stand (ch, "");
      if (ch->position == POS_STANDING)
        ch->color = 0;
    }
    if (IS_AWAKE (ch) && ch->in_room == victim->in_room)
      multi_hit (ch, victim, TYPE_UNDEFINED);

    else
      stop_fighting (ch, FALSE);
    if ((victim = ch->fighting) == NULL)
      continue;

    /*
    * Fun for the whole family!
    */
    check_assist (ch, victim);
  }
  return;
}

bool should_assist (CHAR_DATA * rch, CHAR_DATA * victim)
{

  /*  if (!IS_SET (rch->act, PLR_AUTOASSIST) || IS_NPC (rch))
  { 
  SET_BIT (rch->act, PLR_AUTOASSIST);
  rch->pcdata->autoassist_level = AASSIST_MOBS;
  }|*/
  //  if (rch->pcdata->autoassist_level == AASSIST_PLAYERS)
  //   rch->pcdata->autoassist_level = AASSIST_ALL;
  // Minax 12-20-02 Anyone who does not autoassist  will now.  Set to mobs for safety.
  // Iblis 6/29/03 Fixed it so autoassist mobs was not set everytime this function was ran,
  // thus overriding autoassist ALL or autoassist players.  
  // Iblis 6/29/03 (1.5 hours later) removed it to deal with MUCH less hassle with aggressor flags/etc
  if (IS_SET(rch->act2,ACT_FAMILIAR))
    return TRUE;
  // Iblis 10/24/03 - Charmies no longer autoassist against players..tis only fair..
  if (!IS_NPC (victim))
    return FALSE;
  if (IS_NPC (rch))
    return TRUE;

  if (IS_NPC (victim) && !(IS_SET(rch->pcdata->autoassist_level,AASSIST_NONE)))
    return TRUE;

  /*  {
  if (rch->pcdata->autoassist_level == AASSIST_MOBS ||
  rch->pcdata->autoassist_level == AASSIST_ALL)
  return (TRUE);
  else
  return (FALSE); */
  /*    }
  else
  {
  if (rch->pcdata->autoassist_level == AASSIST_PLAYERS ||
  rch->pcdata->autoassist_level == AASSIST_ALL)
  return (TRUE);
  else
  return (FALSE);
  }*/
  return (FALSE);
}

void check_assist (CHAR_DATA * ch, CHAR_DATA * victim)
{
  CHAR_DATA *rch, *rch_next;
  //Iblis - 10-24-03 - Quick fix, if victim = player, no assists period
  //  if (!IS_NPC(victim))
  //    return;

  for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
  {
    rch_next = rch->next_in_room;
    if (IS_AWAKE (rch) && rch->fighting == NULL)
    {

      /* quick check for ASSIST_PLAYER */
      if (!IS_NPC (ch) && IS_NPC (rch)
        && IS_SET (rch->off_flags, ASSIST_PLAYERS)
        && rch->level + 6 > victim->level)
      {
        do_emote (rch, "screams and attacks!");
        multi_hit (rch, victim, TYPE_UNDEFINED);
        continue;
      }
      if (rch->riders == ch)
      {
        do_emote (rch, "screams and attacks!");
        multi_hit (rch, victim, TYPE_UNDEFINED);
        continue;
      }
      if (ch->race == PC_RACE_NERIX && IS_SET(rch->act,ACT_MOUNT) 
        && !str_cmp(rch->afkmsg,ch->name))
      {
        do_emote (rch, "screams and attacks!");
        multi_hit (rch, victim, TYPE_UNDEFINED);
        continue;
      }

	// Druid's animals should attack.
	if ((ch->Class == PC_CLASS_DRUID) && (rch->master == ch) && is_animal(rch) && IS_NPC(rch)) {
		do_emote(rch, "screams and attacks!");
		multi_hit(rch, victim, TYPE_UNDEFINED);
		continue;
	}

      if (IS_NPC(rch) && rch->pIndexData->vnum == MOB_VNUM_DARKHOUNDS
        && !str_cmp(rch->afkmsg,ch->name))
      {
        do_emote (rch, "screams and attacks!");
        multi_hit (rch, victim, TYPE_UNDEFINED);
        continue;
      }

      if (IS_NPC(rch) && rch->pIndexData->vnum == MOB_VNUM_VIVIFY
        && !str_cmp(rch->afkmsg,ch->name))
      {
        // do_emote (rch, "screams and attacks!");
        multi_hit (rch, victim, TYPE_UNDEFINED);
        continue;
      }

      if ((rch->group != 0 && (rch->group == ch->group)) || 
        (is_same_group (ch, rch) && !IS_NPC(ch) && ch->pcdata->familiar == rch))
      {
        do_emote (rch, "screams and attacks!");
        multi_hit (rch, victim, TYPE_UNDEFINED);
        continue;
      }
      //	  if (!IS_NPC(victim))
      //  continue;

      /* PCs next */
      if ((!IS_NPC (ch) || IS_AFFECTED (ch, AFF_CHARM)))
      {
        if (((!IS_NPC (rch) && should_assist (rch, victim))
          || IS_AFFECTED (rch, AFF_CHARM) || (!IS_NPC(ch) && ch->pcdata->familiar == rch))
          && is_same_group (ch, rch) && !is_safe (rch, victim) && IS_NPC(victim))
          multi_hit (rch, victim, TYPE_AUTOASSIST);
        continue;
      }

      if (IS_NPC (rch) && IS_NPC(ch) && 
        IS_SET(rch->off_flags,ASSIST_EXACT_ALIGN) && rch->alignment == ch->alignment)
      {
        do_emote (rch, "screams and attacks!");
        multi_hit (rch, victim, TYPE_UNDEFINED);
        continue;
      }


      /* now check the NPC cases */
      if (IS_NPC (ch) && !IS_AFFECTED (ch, AFF_CHARM))
      {
        if ((IS_NPC (rch) && IS_SET (rch->off_flags, ASSIST_ALL))
          || (IS_NPC (rch) && IS_SET(rch->off_flags,ASSIST_EXACT_ALIGN) && 
          rch->alignment == ch->alignment)
          || (IS_NPC (rch) && rch->group
          && rch->group == ch->group) || (IS_NPC (rch)
          && rch->race ==
          ch->
          race
          && IS_SET (rch->
          off_flags,
          ASSIST_RACE))
          || (IS_NPC (rch)
          && IS_SET (rch->off_flags,
          ASSIST_ALIGN) && ((IS_GOOD (rch)
          && IS_GOOD (ch))
          || (IS_EVIL (rch)
          && IS_EVIL (ch))
          || (IS_NEUTRAL (rch)
          &&
          IS_NEUTRAL (ch))))
          || (rch->pIndexData ==
          ch->pIndexData && IS_SET (rch->off_flags, ASSIST_VNUM)))
        {
          CHAR_DATA *vch;
          CHAR_DATA *target;
          int number;
          if (number_bits (1) == 0)
            continue;
          target = NULL;
          number = 0;
          for (vch = ch->in_room->people; vch; vch = vch->next)
          {
            if (can_see (rch, vch))
            {
              if (is_same_group (vch, victim)
                && number_range (0, number) == 0)
              {
                target = vch;
                number++;
              }
            }

            else
            {
              if (number_percent () <
                get_skill (rch, gsn_blindfighting))
              {
                if (is_same_group (vch, victim)
                  && number_range (0, number) == 0)
                {
                  target = vch;
                  number++;
                }
                check_improve (ch, gsn_blindfighting, TRUE, 2);
              }

              else
                check_improve (ch, gsn_blindfighting, FALSE, 2);
            }
          }
          if (target != NULL)
          {
            do_emote (rch, "screams and attacks!");
            multi_hit (rch, target, TYPE_UNDEFINED);
          }
        }
      }
    }
  }
}

inline bool leads_group (CHAR_DATA * ch)
{
  CHAR_DATA *gch;
  for (gch = char_list; gch; gch = gch->next)
  {
    if (IS_NPC (gch))
      continue;
    if (gch->leader == ch)
      return (TRUE);
  }
  return (FALSE);
}


//IBLIS 5/20/03 - Changed leadership to make it a lot better
inline int adjust_hitroll (CHAR_DATA * ch)
{
  int hr_adj = 0, charisma;
  CHAR_DATA *leader = NULL;
  if ((leader = ch->leader) == NULL)
  {
    leader = ch;
    if (!leads_group (leader))
      return (0);
  }
  if (number_percent () < get_skill (leader, gsn_leadership))
  {
    hr_adj = leader->level / 10 + (leader->level == 90 ? 3 : 0);

  }
  charisma = get_curr_stat (leader, STAT_CHA);
  if (charisma < 16)
    hr_adj -= 2;

  else if (charisma < 20);

  else if (charisma < 23)
    hr_adj += 1;

  else if (charisma < 25)
    hr_adj += 2;

  else
    hr_adj += 3;
  if (hr_adj > 0)
  {
    check_improve (leader, gsn_leadership, TRUE, 6);
    if (!ignore_hitroll)
      return (hr_adj);
    else return 0;
  }

  else
  {
    check_improve (leader, gsn_leadership, FALSE, 6);
    return (0);
  }
}
inline int adjust_damroll (CHAR_DATA * ch)
{
  int dm_adj = 0, charisma;
  CHAR_DATA *leader = NULL;
  if ((leader = ch->leader) == NULL)
  {
    leader = ch;
    if (!leads_group (leader))
      return (0);
  }
  if (number_percent () < get_skill (leader, gsn_leadership))
  {
    dm_adj = leader->level / 10 + (leader->level == 90 ? 3 : 0);

  }
  charisma = get_curr_stat (leader, STAT_CHA);
  if (charisma < 16)
    dm_adj -= 2;

  else if (charisma < 20);

  else if (charisma < 23)
    dm_adj += 1;

  else if (charisma < 25)
    dm_adj += 2;

  else
    dm_adj += 3;
  if (dm_adj > 0)
    return (dm_adj);

  else
    return (0);
}
inline int get_hitroll2 (CHAR_DATA * ch)
{
  int hroll = 0;
  hroll = get_hitroll(ch);
  //  hroll = (ch)->hitroll + str_app[get_curr_stat (ch, STAT_STR)].tohit;
  if (!IS_NPC (ch))
    if (ch->mount != NULL)
      hroll += ch->mount->pIndexData->attackbonus;
  if (!ignore_hitroll)
    return (hroll);
  else return 0;
}


/*
* Do one group of attacks.
*/
void multi_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
  int chance;
  char buf[20];
  OBJ_DATA *weapon_obj;
  if (ch == victim && IS_NPC (ch))
    return;
  if (IS_SET(ch->act2, ACT_ILLUSION) || IS_SET(victim->act2, ACT_ILLUSION) || IS_SET(victim->act2, ACT_NO_KILL)
    || IS_SET(ch->act2, ACT_NO_KILL))
  {
    return;
  }
  if (IS_SET (ch->in_room->room_flags, ROOM_SAFE))
  {
    send_to_char ("This room is safe from bullies like you.\n\r", ch);
    stop_fighting (ch, TRUE);
    return;
  }

  /* decrement the wait */
  if (ch->desc == NULL)
    ch->wait = UMAX (0, ch->wait - PULSE_VIOLENCE);
  if (ch->desc == NULL)
    ch->daze = UMAX (0, ch->daze - PULSE_VIOLENCE);

  /* no attacks for stunnies -- just a check */
  if (ch->position < POS_RESTING)
    return;
  if (IS_NPC (ch))
  {
    mob_hit (ch, victim, dt);
    return;
  }
  if (ch->stunned > 0)
  {
    ch->stunned--;
    return;
  }
  //Iblis 10/02/04
  if (IS_SET(ch->act2,ACT_WARPED))
  {
    long res_flag1=0,res_flag2=0;
    int i=0;

    //Makes sure an added resistance doesn't overright a vuln/imm
    //Also makes sure they don't have every flag set, so we only check up to 25 times
    for (i=0; i < 25;++i)
    {
      res_flag1 = 1<<number_range(1,25);
      if ((ch->vuln_flags & res_flag1) || (ch->imm_flags & res_flag1))
        continue;
      break;
    }
    if (i >= 25)
      res_flag1 = 0;
    for (i=0; i < 25;++i)
    {
      res_flag2 = 1<<number_range(1,25);
      if ((ch->vuln_flags & res_flag1) || (ch->imm_flags & res_flag1))
        continue;
      break;
    }
    if (i >= 25)
      res_flag2 = 0;
    ch->dam_type = number_range(1,MAX_DAMAGE_MESSAGE);
    ch->res_flags = (res_flag1 + res_flag2 + race_table[ch->race].res);
  }
  chance = get_skill (ch, gsn_stun);
  if (number_percent() < chance*.03)
  {
    stun_char(ch,victim);
  }
 /* if (ch->race == PC_RACE_SWARM)
  {
    if (damage(ch, victim, ch->hit - dice(1,20), gsn_likobe, DAM_NEGATIVE, FALSE))
    {
      act("The swarm feeds on $N!",ch,NULL,victim,TO_CHAR);
      send_to_char("`bThe black cloud of flies is eating you alive!``\n\r",victim);
      act("$N swings at the buzzing swarm of flies as it consumes $m!",ch,NULL,victim,TO_VICT);
      if (number_percent() <= 3 && !saves_spell (ch->level / 2, victim, DAM_POISON))
      {
        AFFECT_DATA af;
        send_to_char
          ("You feel poison coursing through your veins.", victim);
        act ("$n is poisoned by the venom of the swarm.", victim, NULL,
          NULL, TO_ROOM);
        af.where = TO_AFFECTS;
        af.type = gsn_poison;
        af.level = ch->level * 3 / 4;
        af.duration = ch->level / 4;
        af.location = APPLY_STR;
        af.modifier = -1;
        af.bitvector = AFF_POISON;
        af.permaff = FALSE;
        af.composition = FALSE;
        af.comp_name = str_dup (ch->name);
        affect_join (victim, &af);
      }

    }
    else
    {
      act("The swarm attempts to feed on $N, but it does no good!",ch,NULL,victim,TO_CHAR);
      send_to_char("`bThe black cloud of flies flies can't seem to hurt you!``\n\r",victim);
      act("$N swings at the buzzing swarm of flies annoying $m!",ch,NULL,victim,TO_VICT);
    }
    return;
  }*/
  one_hit (ch, victim, dt);
  if (ch->fighting != victim)
    return;
  if (dt == gsn_throwing)
    return;
  if (IS_AFFECTED (ch, AFF_HASTE))
    one_hit (ch, victim, dt);
  if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle || dt == gsn_shadowslash)
    return;
  chance = get_skill (ch, gsn_second_attack) / 2;
  if (IS_AFFECTED (ch, AFF_SLOW))
    chance /= 2;
  if (number_percent () < chance)
  {
    one_hit (ch, victim, dt);
    check_improve (ch, gsn_second_attack, TRUE, 5);
    if (ch->fighting != victim)
      return;
  }
  chance = get_skill (ch, gsn_third_attack) / 4;
  if (IS_AFFECTED (ch, AFF_SLOW))
    chance = 0;
  if (number_percent () < chance)
  {
    one_hit (ch, victim, dt);
    check_improve (ch, gsn_third_attack, TRUE, 6);
    if (ch->fighting != victim)
      return;
  }
  //Iblis 10/31/03 - Extra attack for Kalians at night
  if (ch->race == PC_RACE_KALIAN && (time_info.hour < 5 || time_info.hour >= 19))
  {
    one_hit (ch, victim, dt);
    if (ch->fighting != victim)
      return;
  }
  //Iblis 2/18/04 - Extra attack for Nidaes in the water
  if (ch->race == PC_RACE_NIDAE && ch->in_room
    && (ch->in_room->sector_type == SECT_UNDERWATER
    || ch->in_room->sector_type == SECT_WATER_OCEAN))
  {
    one_hit (ch, victim, dt);
    if (ch->fighting != victim)
      return;
  }
  trip_triggers(ch, OBJ_TRIG_CHANCE_ON_ROUND, NULL, victim, OT_SPEC_NONE);
  //This is just so the innards checking of this doesn't happen very often
  if (ch->Class == PC_CLASS_CHAOS_JESTER)
  {
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
  }
  check_skeletal_mutation(ch, TRUE);
  if (ch->fighting == NULL)
    return;
  if (ch->max_hit > 0)
  {
    sprintf(buf, "%d", (ch->hit * 100 / ch->max_hit));
    trip_triggers_arg(ch, OBJ_TRIG_CHAR_HP_PCT, NULL, victim, OT_SPEC_NONE, buf);
  }
  if (victim->max_hit > 0)
  {
    sprintf(buf, "%d", (victim->hit * 100 / victim->max_hit));
    trip_triggers_arg(ch, OBJ_TRIG_VICT_HP_PCT, NULL, victim, OT_SPEC_NONE, buf);
  }
  if (ch->max_mana > 0)
  {
    sprintf(buf, "%d", (ch->mana * 100 / ch->max_mana));
    trip_triggers_arg(ch, OBJ_TRIG_MANA_PCT, NULL, victim, OT_SPEC_NONE, buf);
  }
  if (ch->max_move > 0)
  {
    sprintf(buf, "%d", (ch->move * 100 / ch->max_move));
    trip_triggers_arg(ch, OBJ_TRIG_MOVE_PCT, NULL, victim, OT_SPEC_NONE, buf);
  }

  chance = get_skill (ch, gsn_dirty_tactics) / 20.0;
  if (IS_AFFECTED (ch, AFF_SLOW))
    chance /= 2;
  if (number_percent () < chance /*&& !IS_NPC (ch)*/)
  {

    chance = number_percent ();
    if (chance < 20)
      do_dirtytrip (ch, victim);

    else if (chance < 30)
      do_eyegouge (ch, victim);

    //Choose stomp or headbutt IN damage
    else
      damage (ch, victim,
      50 * (get_skill (ch, gsn_dirty_tactics) / 100.0),
      gsn_dirty_tactics, DAM_BASH, TRUE);

    check_improve (ch, gsn_dirty_tactics, TRUE, 6);
    if (ch->fighting != victim)
      return;
  }
  chance = get_skill (ch, gsn_dual_wield);
  if (is_Class_tiers(ch,PC_CLASS_MONK,1) || ch->race == PC_RACE_NERIX)
    chance = 100;
  if (ch->race == PC_RACE_NERIX && get_eq_char (ch, WEAR_HOLD))
    chance = 0;
  weapon_obj = get_eq_char (ch, WEAR_WIELD_R);
  if (weapon_obj == NULL)
    weapon_obj = get_eq_char (ch, WEAR_WIELD_L);
  if (weapon_obj != NULL)
    if (IS_SET (weapon_obj->value[4], WEAPON_TWO_HANDS) && (ch->race != PC_RACE_VROATH))
      return;
  if (weapon_obj != NULL && get_eq_char (ch, WEAR_SHIELD) != NULL
    && !(IS_CLASS (ch, PC_CLASS_MONK)))
    return;
  if (number_percent () < chance)
  {
    one_dual_hit (ch, victim, dt);
    if (!IS_CLASS (ch, PC_CLASS_MONK) && ch->race != PC_RACE_NERIX)
      check_improve (ch, gsn_dual_wield, TRUE, 8);
    if (ch->fighting != victim)
      return;
  }
  if (ch->race == PC_RACE_NERIX)
  {
    is_tail_attack = TRUE;
    one_dual_hit (ch, victim, dt);
    is_tail_attack = FALSE;
    if (ch->fighting != victim)
      return;
  }
  chance = get_skill (ch, gsn_falconry) / 10.0;
  if (number_percent () < chance && !IS_NPC (ch) && IS_OUTSIDE (ch)
    && ch->pcdata->falcon_wait == 0)
  {
    act
      ("`hYour falcon swoops down and claws $N with its talons before flying off.``",
      ch, NULL, victim, TO_CHAR);
    act
      ("`h$n's falcon swoops down and claws you with its talons before flying off.``",
      ch, NULL, victim, TO_VICT);
    act
      ("`h$n's falcon swoops down and claws $N with its talons before flying off.``",
      ch, NULL, victim, TO_NOTVICT);
    damage (ch, victim, 50, gsn_falconry, DAM_CLAWING, FALSE);
    if (ch->fighting != victim)
      return;

  }
  if (is_affected (ch, gsn_spider_vine) && number_percent() > 50)
  {
    int dam;
    act("`bThe vines on $n's arms come to life and tear into your flesh!``",
      ch,NULL,victim,TO_VICT);
    act("`bThe vines on $n's arms come to life and tear into $N's flesh!``",
      ch,NULL,victim,TO_NOTVICT);
    act("`bThe vines on your arms come to life and tear into $N's flesh!``",
      ch,NULL,victim,TO_CHAR);
    dam = dice(3,30);
    damage (ch, victim, dam, 0, DAM_CLAWING, FALSE);
    dam = number_range (1, dam / 5 + 1);
    damage_old (ch, victim, dam, 0, DAM_NEGATIVE, FALSE);
    ch->hit += dam / 2;
    if (!saves_spell (ch->level / 2, victim, DAM_POISON))
    {
      AFFECT_DATA af;
      send_to_char
        ("You feel poison coursing through your veins.", victim);
      act ("$n is poisoned by the venom on $N's vines.", victim, NULL,
        ch, TO_NOTVICT);
      act ("$n is poisoned by the venom on your vines.", victim, NULL, ch, TO_CHAR);
      af.where = TO_AFFECTS;
      af.type = gsn_poison;
      af.level = ch->level * 3 / 4;
      af.duration = ch->level / 2;
      af.location = APPLY_STR;
      af.modifier = -1;
      af.bitvector = AFF_POISON;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup (ch->name);
      affect_join (victim, &af);
    }
    if (ch->fighting != victim)
      return;
  } 
  if (ch->Class == PC_CLASS_CHAOS_JESTER)
  {
    ++ch->pcdata->pain_points;
    if (ch->fighting && IS_NPC(ch->fighting) && ch->pcdata->pain_points > 15)
      ch->pcdata->pain_points = 15;

  }
  if (!IS_NPC(ch) && !IS_NPC(victim) && !IS_IMMORTAL(victim) && ch->pcdata->aggression)
  {
    if ((60 - ch->pcdata->aggression*10) > number_percent())
    {
      one_dual_hit (ch, victim, dt);
      if (ch->fighting != victim)
        return;
    }
    if (ch->pcdata->aggression < 4)
      ++ch->pcdata->aggression;
  }
  return;
}

void mob_throw (CHAR_DATA * ch)
{
  OBJ_DATA *obj;
  for (obj = ch->carrying; obj; obj = obj->next_content)
  {
    if (obj->item_type != ITEM_WEAPON)
      continue;
    if (IS_SET (obj->value[4], WEAPON_THROWING))
    {
      do_throw (ch, obj->name);
      return;
    }
  }
}


/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
  int chance, number;
  CHAR_DATA *vch, *vch_next;
  char buf[30];
  if (ch->stunned > 0)
  {
    ch->stunned--;
    return;
  }
 /* if (ch->race == PC_RACE_SWARM)
  {
    if (damage(ch, victim, ch->hit - dice(1,20), gsn_likobe, DAM_NEGATIVE, FALSE))
    {
      act("The swarm feeds on $N!",ch,NULL,victim,TO_CHAR);
      send_to_char("`bThe black cloud of flies is eating you alive!``\n\r",victim);
      act("$N swings at the buzzing swarm of flies as it consumes $m!",ch,NULL,victim,TO_VICT);
      if (number_percent() <= 3 && !saves_spell (ch->level / 2, victim, DAM_POISON))
      {
        AFFECT_DATA af;
        send_to_char
          ("You feel poison coursing through your veins.", victim);
        act ("$n is poisoned by the venom of the swarm.", victim, NULL,
          NULL, TO_ROOM);
        af.where = TO_AFFECTS;
        af.type = gsn_poison;
        af.level = ch->level * 3 / 4;
        af.duration = ch->level / 4;
        af.location = APPLY_STR;
        af.modifier = -1;
        af.bitvector = AFF_POISON;
        af.permaff = FALSE;
        af.composition = FALSE;
        af.comp_name = str_dup (ch->name);
        affect_join (victim, &af);
      }
    }
    else
    {
      act("The swarm attempts to feed on $N, but it does no good!",ch,NULL,victim,TO_CHAR);
      send_to_char("`bThe black cloud of flies flies can't seem to hurt you!``\n\r",victim);
      act("$N swings at the buzzing swarm of flies annoying $m!",ch,NULL,victim,TO_VICT);
    }
    return;
  }*/

  //Iblis 9/13/04
  if (IS_SET(ch->act2,ACT_WARPED))
  {
    long res_flag1=0,res_flag2=0;
    int i=0;

    //Makes sure an added resistance doesn't overright a vuln/imm
    //Also makes sure they don't have every flag set, so we only check up to 25 times
    for (i=0; i < 25;++i)
    {
      res_flag1 = 1<<number_range(1,25);
      if ((ch->vuln_flags & res_flag1) || (ch->imm_flags & res_flag1))
        continue;
      break;
    }
    if (i >= 25)
      res_flag1 = 0;
    for (i=0; i < 25;++i)
    {
      res_flag2 = 1<<number_range(1,25);
      if ((ch->vuln_flags & res_flag1) || (ch->imm_flags & res_flag1))
        continue;
      break;
    }
    if (i >= 25)
      res_flag2 = 0;
    ch->dam_type = number_range(1,MAX_DAMAGE_MESSAGE);
    ch->res_flags = (res_flag1 + res_flag2 + ch->pIndexData->res_flags);
  }


  if (ch->number_of_attacks > -1)
  {
    int i;
    for (i = 0;i < ch->number_of_attacks;i++) 
    {
      one_hit (ch, victim, dt);
      if (ch->fighting != victim)
        return;
    }
    if (IS_SET (ch->off_flags, OFF_AREA_ATTACK))
    {
      for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
      {
        vch_next = vch->next;
        if ((vch != victim && vch->fighting == ch))
          one_hit (ch, vch, dt);
      }
    }
  }
  else
  {
    one_hit (ch, victim, dt);
    if (ch->fighting != victim)
      return;

    if (IS_SET (ch->off_flags, OFF_AREA_ATTACK))
    {
      for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
      {
        vch_next = vch->next;
        if ((vch != victim && vch->fighting == ch))
          one_hit (ch, vch, dt);
      }
    }
    if (IS_AFFECTED (ch, AFF_HASTE)
      || (IS_SET (ch->off_flags, OFF_FAST) && !IS_AFFECTED (ch, AFF_SLOW)))
      one_hit (ch, victim, dt);
    if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_shadowslash)
      return;
    chance = get_skill (ch, gsn_second_attack) / 2;
    if (IS_AFFECTED (ch, AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
      chance /= 2;
    if (number_percent () < chance)
    {
      one_hit (ch, victim, dt);
      if (ch->fighting != victim)
        return;
    }
    chance = get_skill (ch, gsn_third_attack) / 4;
    if (IS_AFFECTED (ch, AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
      chance = 0;
    if (number_percent () < chance)
    {
      one_hit (ch, victim, dt);
      if (ch->fighting != victim)
        return;
    }
  }
  trip_triggers(ch, OBJ_TRIG_CHANCE_ON_ROUND, NULL, victim, OT_SPEC_NONE);
  //This is just so the innards checking of this doesn't happen very often
  if (ch->Class == PC_CLASS_CHAOS_JESTER)
  {
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
  }
  check_skeletal_mutation(ch, TRUE);
  if (ch->fighting == NULL)
    return;

  if (ch->max_hit > 0)
  {
    sprintf(buf, "%d", (ch->hit * 100 / ch->max_hit));
    trip_triggers_arg(ch, OBJ_TRIG_CHAR_HP_PCT, NULL, victim, OT_SPEC_NONE, buf);
  }
  if (victim->max_hit > 0)
  {
    sprintf(buf, "%d", (victim->hit * 100 / victim->max_hit));
    trip_triggers_arg(ch, OBJ_TRIG_VICT_HP_PCT, NULL, victim, OT_SPEC_NONE, buf);
  }
  if (ch->max_mana > 0)
  {
    sprintf(buf, "%d", (ch->mana * 100 / ch->max_mana));
    trip_triggers_arg(ch, OBJ_TRIG_MANA_PCT, NULL, victim, OT_SPEC_NONE, buf);
  }
  if (ch->max_move > 0)
  {
    sprintf(buf, "%d", (ch->move * 100 / ch->max_move));
    trip_triggers_arg(ch, OBJ_TRIG_MOVE_PCT, NULL, victim, OT_SPEC_NONE, buf);
  }

  /* oh boy!  Fun stuff! */
  if (ch->wait > 0)
    return;
  number = number_range (0, 2);
  if (number == 1 && IS_SET (ch->act, ACT_MAGE))
  {
    number = number_range (0,10);
    switch (number)
    {
    case (0) : if (skill_table[skill_lookup("dispel magic")].skill_level[PC_CLASS_MAGE] <= ch->level)
                 interpret (ch,"cast dispel");
      break;
    case (1) : if (skill_table[skill_lookup("blindess")].skill_level[PC_CLASS_MAGE] <= ch->level)
                 interpret (ch,"cast blindness");
      break;
    case (2) : if (skill_table[skill_lookup("weakness")].skill_level[PC_CLASS_MAGE] <= ch->level)
                 interpret (ch,"cast weakness");
      break;
    case (3) : if (skill_table[skill_lookup("curse")].skill_level[PC_CLASS_MAGE] <= ch->level)
                 interpret (ch,"cast curse");
      break;
    case (4) : if (skill_table[skill_lookup("web")].skill_level[PC_CLASS_MAGE] <= ch->level)
                 interpret (ch,"cast web");
      break;
    case (5) : if (skill_table[skill_lookup("fireshield")].skill_level[PC_CLASS_MAGE] <= ch->level)
                 interpret (ch,"cast fireshield");break;
      break;
    case (6) : if (skill_table[skill_lookup("heat metal")].skill_level[PC_CLASS_MAGE] <= ch->level)
                 interpret (ch,"cast 'heat metal'");break;
    case (7) :
    case (8) :
    case (9) :
    case (10) :if (skill_table[skill_lookup("energy drain")].skill_level[PC_CLASS_MAGE] > ch->level)
               {
                 if (number%2 == 1)
                   interpret (ch,"cast 'magic missle'");
                 else interpret (ch,"cast 'colour spray'");
               }
               else if (skill_table[skill_lookup("fireball")].skill_level[PC_CLASS_MAGE] > ch->level)
               {
                 if (number%2 == 1)
                   interpret (ch,"cast 'colour spray'");
                 else interpret (ch,"cast 'energy drain'");
               }
               else if (skill_table[skill_lookup("acid blast")].skill_level[PC_CLASS_MAGE] > ch->level)
               {
                 if (number%2 == 1)
                   interpret (ch,"cast 'energy drain'");
                 else interpret (ch,"cast 'acid blast'");
               }
               else 
               {
                 if (number%2 == 1)
                   interpret (ch,"cast 'acid blast'");
                 else interpret (ch,"cast 'firestorm'");
               }
               return;
    }

    /*  { mob_cast_mage(ch,victim); return; } */ ;
  }
  if (number == 1 && IS_SET (ch->act2, ACT_MONK))
  {
    number = number_range (0, 5);
    switch (number)
    {
    case (0) : interpret (ch, "strike");break;
    case (1) : interpret (ch, "kick");break;
    case (2) : interpret (ch, "whirlwind");break;
    case (3) : interpret (ch, "flee");
      sprintf(buf, "dive %s",victim->name);
      interpret (ch,buf);
      break;
    case (4) : interpret (ch, "kick");break;

    }
    return;
  }

  if (number == 2 && IS_SET (ch->act2, ACT_NECROMANCER))   
  {
    number = number_range (0, 13);
    switch (number)
    {
    case (0) :  if (skill_table[skill_lookup("nearsight")].skill_level[PC_CLASS_NECROMANCER] <= ch->level)
                  interpret (ch, "chant nearsight");break;
    case (1) : if (skill_table[skill_lookup("pox")].skill_level[PC_CLASS_NECROMANCER] <= ch->level)
                 interpret (ch, "chant pox");break;
    case (2) : if (skill_table[skill_lookup("jinx")].skill_level[PC_CLASS_NECROMANCER] <= ch->level)
                 interpret (ch, "chant jinx");break;
    case (3) : if (skill_table[skill_lookup("atrophy")].skill_level[PC_CLASS_NECROMANCER] <= ch->level)
                 interpret (ch, "chant atrophy");break;
    case (4) : if (skill_table[skill_lookup("brain blisters")].skill_level[PC_CLASS_NECROMANCER] <= ch->level)
                 interpret (ch, "chant 'brain blisters'");break;
    case (5) : if (skill_table[skill_lookup("intoxicate")].skill_level[PC_CLASS_NECROMANCER] <= ch->level)
                 interpret (ch, "chant intoxicate");break;
    case (6) : if (skill_table[skill_lookup("vomit")].skill_level[PC_CLASS_NECROMANCER] <= ch->level)
                 interpret (ch, "chant vomit");break;
    case (7) : if (skill_table[skill_lookup("clasp")].skill_level[PC_CLASS_NECROMANCER] <= ch->level)
                 interpret (ch, "chant clasp");break;
    case (8) : if (skill_table[skill_lookup("greymantle")].skill_level[PC_CLASS_NECROMANCER] <= ch->level)
                 interpret (ch, "chant greymantle");break;
    case (9) : if (skill_table[skill_lookup("death shroud")].skill_level[PC_CLASS_NECROMANCER] <= ch->level)
                 interpret (ch, "chant 'death shroud'");break;
    case (10) : if (skill_table[skill_lookup("drain aura")].skill_level[PC_CLASS_NECROMANCER] <= ch->level) 
                  interpret (ch, "chant 'drain aura'");break;
    case (11) : if (skill_table[skill_lookup("wind of death")].skill_level[PC_CLASS_NECROMANCER] <= ch->level)
                  interpret (ch, "chant 'wind of death'");break;
    case (12) : if (skill_table[skill_lookup("death palm")].skill_level[PC_CLASS_NECROMANCER] <= ch->level)
                  interpret (ch, "chant 'death palm'");break;
    case (13) : if (skill_table[skill_lookup("corrupt")].skill_level[PC_CLASS_NECROMANCER] <= ch->level)
                {
                  CHAR_DATA *vch, *vch_next;
                  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
                  {
                    vch_next = vch->next_in_room;
                    if (!IS_AFFECTED (vch, AFF_CHARM))
                      continue;
                    if (vch->master == NULL)
                      continue;
                    if (vch->master->fighting != ch)
                      continue;
                    break;
                  }
                  if (vch != NULL)
                  {
                    char buf[MAX_STRING_LENGTH];
                    sprintf(buf,"corrupt %s",vch->name);
                    interpret(ch,buf);
                  }
                }
                break;

    }
    return;
  }

  if (number == 2 && IS_SET (ch->act, ACT_CLERIC))
  {

    /* { mob_cast_cleric(ch,victim); return; } */ ;
  }

  /* now for the skills */
  number = number_range (0, 100);
  if (IS_SET (ch->off_flags, OFF_TRAMPLE) && number < 20)
    do_trample (ch, "");
  number = number_range (0, 11);
  switch (number)
  {
  case (0):
    if (IS_SET (ch->off_flags, OFF_BASH))
      interpret (ch, "bash");
    break;
  case (1):
    if (IS_SET (ch->off_flags, OFF_BERSERK)
      && !IS_AFFECTED (ch, AFF_BERSERK))
      interpret (ch, "berserk");
    break;
  case (2):
    if (IS_SET (ch->off_flags, OFF_DISARM)
      || (get_weapon_sn (ch, WEAR_WIELD_R) != gsn_hand_to_hand
      && (IS_SET (ch->act, ACT_WARRIOR)
      || IS_SET (ch->act, ACT_THIEF))))
      interpret (ch, "disarm");
    break;
  case (3):
    if (IS_SET (ch->off_flags, OFF_KICK))
      interpret (ch, "kick");
    break;
  case (4):
    if (IS_SET (ch->off_flags, OFF_KICK_DIRT))
      interpret (ch, "dirt");
    break;
    //  case (5):

    /*      if (IS_SET(ch->off_flags, OFF_TAIL))
    {
    do_tail(ch,"")  ;
    } */
    //      break;
  case (6):
    if (IS_SET (ch->off_flags, OFF_TRIP))
      interpret (ch, "trip");
    break;
    //    case (7):

    /*      if (IS_SET(ch->off_flags, OFF_CRUSH))
    {
    do_crush(ch,"")  ;
    } */
    //    break;
  case (8):
    if (IS_SET (ch->off_flags, OFF_BACKSTAB))
    {
      interpret (ch, "backstab");
    }
    break;
  case (9):
    if (IS_SET (ch->off_flags, OFF_CHARGE))
    {
      interpret (ch, "charge");
    }
    break;
  case (10):
    if (IS_SET (ch->off_flags, OFF_SWEEP))
    {
      interpret (ch, "sweep");
    }
    break;
  case (11):
    if (IS_SET (ch->off_flags, OFF_THROWING))
      mob_throw (ch);
    break;
  case (7):
    if (IS_SET (ch->off_flags, OFF_KAI))
      interpret (ch, "kai");
    break;
  case (5):
    if (IS_SET (ch->off_flags, OFF_HELLBREATH))
      interpret (ch, "hellbreath");
    break;

  }
}


/*
* Hit one guy once.
*/
void one_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
  OBJ_DATA *wield;
  int victim_ac;
  int thac0;
  int thac0_00;
  int thac0_32;
  int dam;
  int diceroll;
  int sn, skill;
  int dam_type = DAM_BASH, wield_type;
  bool result=TRUE;
  missed_attack = FALSE;
  sn = -1;
  if (ch == victim && IS_NPC (ch) && dt != -5) 
    return;
  if (victim == ch->mount)
  {
    act ("$N bucks wildly, throwing you to the ground.", ch, NULL,
      victim, TO_CHAR);
    act ("You buck wildly, throwing $n to the ground.", ch, NULL,
      victim, TO_VICT);
    act ("$N bucks wildly, throwing $n to the ground.", ch, NULL,
      victim, TO_NOTVICT);
    ch->mount->riders = NULL;
    ch->mount = NULL;
    if (!ch->bashed)
      ch->position = POS_FIGHTING;
  }
  if ((victim == ch && dt != -5) || ch == NULL || victim == NULL) 
    return;
  if (dt == -5)
    dt = TYPE_UNDEFINED;
  if (IS_NPC (ch))
    wield_type = WEAR_WIELD_R;

  else
  {
    if (ch->pcdata->primary_hand == HAND_RIGHT
      || ch->pcdata->primary_hand == HAND_AMBIDEXTROUS)
      wield_type = WEAR_WIELD_R;
    else
      wield_type = WEAR_WIELD_L;
  }
  if (victim->position == POS_DEAD ||
    (ch->in_room != victim->in_room && dt != gsn_throwing))
    return;
  wield = get_eq_char (ch, wield_type);
  if (dt == TYPE_UNDEFINED || dt == gsn_whirlwind)
  {
    dt = TYPE_HIT;
    if (wield != NULL && wield->item_type == ITEM_WEAPON)
      dt += wield->value[3];
    else
      dt += ch->dam_type;
  }
  if (!IS_NPC (ch) && ch->mount != NULL)
  {
    if (wield != NULL)
    {
      if (!IS_SET (wield->value[4], WEAPON_LANCE)
        && (wield->value[0] != WEAPON_POLEARM))
      {
        act ("Your $p is useless while mounted.", ch, wield, NULL, TO_CHAR);
        damage (ch, victim, 0, dt, dam_type, FALSE);
        return;
      }
    }
    else
    {
      send_to_char ("You cannot punch from your mount.\n\r", ch);
      damage (ch, victim, 0, dt, dam_type, FALSE);
      return;
    }
  }
  if (victim->mount != NULL && wield == NULL && !IS_NPC (ch) && ch->race != PC_RACE_NERIX)
  {
    act ("You swing your arms wildly, but you fail to reach $N.", ch,
      NULL, victim, TO_CHAR);
    act ("$n swings $s arms wildly, but fails to reach $N.", ch, NULL,
      victim, TO_NOTVICT);
    act ("$n swings $s arms wildly, but fails to reach you.", ch,
      NULL, victim, TO_VICT);
    return;
  }
  if (wield && wield->item_type == ITEM_WEAPON && wield->value[0] == WEAPON_DICE)
  {
    wield->value[3] = number_range(1,MAX_DAMAGE_MESSAGE);
  }
  if (dt < TYPE_HIT)
  {
    if (wield != NULL)
      dam_type = attack_table[wield->value[3]].damage;
    else
      dam_type = attack_table[ch->dam_type].damage;
  }
  else
  {
    if ((dt - TYPE_HIT) < 0 || (dt - TYPE_HIT) >  MAX_DAMAGE_MESSAGE)
    {
      char logbuf[MAX_STRING_LENGTH];
      dam_type = DAM_BASH;
      sprintf (logbuf, "DAMTYPE/attack_table error: %s", ch->name);
      if (IS_NPC (ch))
        sprintf (logbuf + strlen (logbuf), " (vnum: %d)",
        ch->pIndexData->vnum);
      log_string (logbuf);
    }
    else
      dam_type = attack_table[dt - TYPE_HIT].damage;
  }
  if (dam_type == -1)
    dam_type = DAM_BASH;
  if (ch->race == PC_RACE_NERIX)
    skill = 120;
  else 
  {
    sn = get_weapon_sn (ch, wield_type);
    skill = 20 + get_weapon_skill (ch, sn);
  }

  if (!new_ac)
  {
    /*
    * Calculate to-hit-armor-Class-0 versus armor.
    */
    if (IS_NPC (ch))
    {
      thac0_00 = 20;
      thac0_32 = -4;		/* as good as a thief */
      if (IS_SET (ch->act, ACT_WARRIOR))
        thac0_32 = -10;
      else if (IS_SET (ch->act, ACT_THIEF))
        thac0_32 = -4;
      else if (IS_SET (ch->act, ACT_CLERIC))
        thac0_32 = 2;
      else if (IS_SET (ch->act, ACT_MAGE))
        thac0_32 = 6;
    }
    else
    {
      thac0_00 = Class_table[current_Class (ch)].thac0_00;
      thac0_32 = Class_table[current_Class (ch)].thac0_32;
    }
    thac0 = interpolate (ch->level, thac0_00, thac0_32);
    if (thac0 < 0)
      thac0 = thac0 / 2;
    if (thac0 < -5)
      thac0 = -5 + (thac0 + 5) / 2;
    thac0 -= (get_hitroll2 (ch) + adjust_hitroll (ch)) * skill / 100;
    thac0 += 5 * (100 - skill) / 100;
    if (dt == gsn_backstab)
      thac0 -= 10 * (100 - get_skill (ch, gsn_backstab));
	if (dt == gsn_shadowslash)
      thac0 -= 10 * (100 - get_skill (ch, gsn_shadowslash));
    else if (dt == gsn_eviscerate)
      thac0 -= 10 * (100 - get_skill (ch, gsn_eviscerate));
    else if (dt == gsn_throwing)
      thac0 -= 7 * (100 - get_skill (ch, gsn_assassinate));
    else if (dt == gsn_circle)
      thac0 -= 7 * (100 - get_skill (ch, gsn_circle));
    switch (dam_type)
    {
      case (DAM_PIERCE):
        victim_ac = GET_AC (victim, AC_PIERCE) / 10;
        break;
      case (DAM_BASH):
        victim_ac = GET_AC (victim, AC_BASH) / 10;
        break;
      case (DAM_SLASH):
        victim_ac = GET_AC (victim, AC_SLASH) / 10;
        break;
      default:
        victim_ac = GET_AC (victim, AC_EXOTIC) / 10;
        break;
    };
    if (victim_ac < -15)
      victim_ac = (victim_ac + 15) / 5 - 15;
    if (!can_see (ch, victim))
    {
      if (number_percent () < get_skill (ch, gsn_blindfighting))
        check_improve (ch, gsn_blindfighting, TRUE, 2);
      else
      {
        check_improve (ch, gsn_blindfighting, FALSE, 2);
        victim_ac -= 4;
      }
    }
    if (victim->position < POS_FIGHTING)
      victim_ac += 4;
    if (victim->position < POS_RESTING)
      victim_ac += 6;
    if (ignore_ac)
      victim_ac = 0;

    /*
    * The moment of excitement!
    */
    while ((diceroll = number_bits (5)) >= 20);
    if (diceroll == 0 || (diceroll != 19 && diceroll < thac0 - victim_ac))
    {
      damage (ch, victim, 0, dt, dam_type, TRUE);
      if (!missed_attack || !(wield != NULL && ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_WINDSLASH)))
      {
        return;
        //act("A force of air slashes into $n!",victim,NULL,ch,TO_ROOM);
        //act("A force of air slashes into you!",victim,NULL,ch,TO_CHAR);
        //damage(ch,victim,dam/3,dt,DAM_WIND,FALSE);
      }
      //return;
    }
  } 
  else
  {
    int hitroll = get_hitroll2 (ch) + adjust_hitroll (ch);
    int chance = 0,i,Class;
    switch (dam_type)
    {
      case (DAM_PIERCE):
        victim_ac = GET_AC (victim, AC_PIERCE);
        break;
      case (DAM_BASH):
        victim_ac = GET_AC (victim, AC_BASH);
        break;
      case (DAM_SLASH):
        victim_ac = GET_AC (victim, AC_SLASH);
        break;
      default:
        victim_ac = GET_AC (victim, AC_EXOTIC);
        break;
    }

    if (hitroll >= 100)
      chance = 50 + (hitroll-100)/20.0;
    else if (hitroll > 10)
      chance = 30+((hitroll-10)*(20/90.0));
    else if (hitroll == 10)
      chance = 30;
    else if (hitroll > -20)
      chance = (hitroll+20)*(30/30.0);
    else chance = 1;
    if (victim_ac > 500)
      chance += 50;
    else if (victim_ac > 0)
      chance += 30+(victim_ac*(20/500.0));
    else if (victim_ac == 0)
      chance += 30;
    else if (victim_ac > -600)
      chance += 30 - (-victim_ac*(30/600.0));
    else chance += 1;
    for (i = 0, Class=0; i<3; i++)
    {
      if (i==0)
      Class = ch->Class;
      else if (i==1)
      Class = ch->Class2;
      else Class = ch->Class3;
      if (i==1 && ch->level < 31)
        break;
      if (i==2 && ch->level < 61)
        break;
      switch (Class)
      {
        case PC_CLASS_MAGE:
        case PC_CLASS_CLERIC:
        case PC_CLASS_NECROMANCER:
        case PC_CLASS_BARD:
	case PC_CLASS_DRUID:
        default:
          break;
        case PC_CLASS_THIEF:
        case PC_CLASS_ASSASSIN:
          chance += 1; break;
        case PC_CLASS_WARRIOR:
          chance += 5; break;
        case PC_CLASS_PALADIN:
        case PC_CLASS_REAVER:
        case PC_CLASS_RANGER:
        case PC_CLASS_MONK:
          chance += 3; break;
      }
    }

	// Druid Wild Invigoration.
	if (is_Class_tiers(ch, PC_CLASS_DRUID, 1) && is_char_in_natural_sector(ch)) {
		chance += ch->level / 9;

	if (number_percent() > 75) {
		send_to_char("`cYou are invigorated by your natural surroundings and strike quickly at your enemy!``\n\r", ch);
		damage(ch, victim, ((ch->level / 2) + number_percent()), dt, dam_type, TRUE);
	}
    }

    if (number_percent() >= chance)
    {
      damage (ch, victim, 0, dt, dam_type, TRUE);
      if (!missed_attack || !(wield != NULL && ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_WINDSLASH)))
      {
        return;
      }
    }
  }



  if (IS_NPC (ch) && (wield == NULL))
  {
    if (!IS_SET(ch->act2,ACT_FAMILIAR))
      dam = dice (ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);
    else dam = number_range (1 + 4 * skill / 100, 2 * ch->level / 3 * skill / 100);  
  }

  else
  {
    if (sn != -1)
      check_improve (ch, sn, TRUE, 5);
    if (wield != NULL)
    {
      dam = dice (wield->value[1], wield->value[2]) * skill / 100;
      if (get_eq_char (ch, WEAR_SHIELD) == NULL)	/* no shield = more */
        dam = dam * 11 / 10;

      /* sharpness! */
      if (IS_WEAPON_STAT (wield, WEAPON_SHARP))
      {
        int percent;
        if ((percent = number_percent ()) <= (skill / 8))
          dam = 2 * dam + (dam * 2 * percent / 100);
      }
    }

    else
    {
      if (ch->race == PC_RACE_NERIX)
      {
        int percent;
        //Need a way to differentiate normal / tail attacks
        if (is_tail_attack)
        {
          dam = (dice (ch->level/2 + 15,1) + 2*(ch->level/10))/ 2;
        }
        else
        {
          dam = dice (ch->level/2 + 15,1); /* Normal claw attack */
          dam += 2*(ch->level/10);
          if ((percent = number_percent ()) <= (skill / 8))
            dam = 2 * dam + (dam * 2 * percent / 100);     /* Simulated sharp claw */
        }
      }
      else dam = number_range (1 + 4 * skill / 100, 2 * ch->level / 3 * skill / 100);
    }
  }

  /*
  * Bonuses.
  */
  if (get_skill (ch, gsn_enhanced_damage) > 0)
  {
    diceroll = number_percent ();
    if (diceroll <= get_skill (ch, gsn_enhanced_damage))
    {
      check_improve (ch, gsn_enhanced_damage, TRUE, 6);
      dam += 2 * (dam * diceroll / 300);
    }
  }

  if (!IS_NPC(ch) && ch->Class == PC_CLASS_CHAOS_JESTER)
  {
    dam += ch->pcdata->pain_points;
    if (ch->pcdata->pain_points > 4)
    {
      send_to_char("Sweet pain inspires you to hit harder!\n\r",ch);
      act("Pain inspires $n to hit harder.",ch,NULL,NULL,TO_ROOM);
    }
  }

  if (get_eq_char (ch, WEAR_WIELD_R) == NULL
    && get_eq_char (ch, WEAR_WIELD_L) == NULL
    && get_skill (ch, gsn_fisticuffery) > 0)
  {
    diceroll = number_percent ();
    if (diceroll <= get_skill (ch, gsn_fisticuffery))
    {
      check_improve (ch, gsn_fisticuffery, TRUE, 6);
      dam += 2 * (dam * diceroll / 300);
    }
  }
  if (!IS_AWAKE (victim))
    dam *= 2;

  else if (victim->position < POS_FIGHTING)
    dam = dam * 3 / 2;
  if (dt == gsn_uppercut || dt == gsn_strike)
  {
    if (victim->mount != NULL)
    {
      act
        ("You reach back to deliver a mighty blow, but your hand never makes contact with $N.",
        ch, NULL, victim, TO_CHAR);
      act
        ("$n reaches back to deliver a mighty blow, but $s hand never makes contact with $N.",
        ch, NULL, victim, TO_NOTVICT);
      act
        ("$n reaches back to deliver a mighty blow, but $s hand never makes contact with you.",
        ch, NULL, victim, TO_VICT);
      return;
    }
    dam = ch->level + (get_curr_stat (ch, STAT_STR) * number_range (1, 4));
    dam = dam / 2;
    dam += (get_skill (ch, gsn_hand_to_hand) / 8);
    dam += (get_skill (ch, gsn_fisticuffery) / 6);
  }
  if (dt == gsn_backstab  && wield != NULL)
  {
    if (wield->value[0] != 2)
      dam = ((dam * 7) + (ch->level / 9)) / 3;

    else
      dam = ((dam * 7) + (ch->level / 7)) / 3;
  }
  dam += (get_damroll (ch) + adjust_damroll (ch)) * UMIN (100, skill) / 100;
  if (dam <= 0)
    dam = 1;
  if (is_affected (victim, gsn_mirror) && dam_type != DAM_ACID
    && dam_type != DAM_ENERGY && dam_type != DAM_HOLY
    && dam_type != DAM_LIGHTNING && dam_type != DAM_FIRE
    && dam_type != DAM_COLD && dam_type != DAM_NEGATIVE
    && dam_type != DAM_MENTAL && dam_type != DAM_LIGHT
    && dam_type != DAM_CHARM && dam_type != DAM_DISEASE
    && dam_type != DAM_SOUND && dam_type != DAM_HARM)
  {
    if (ch->level >= (victim->level + 10) && number_percent () > 50)
    {
      char *attack;
      char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
      char buf3[MAX_STRING_LENGTH];
      if (dt >= 0 && dt < MAX_SKILL)
        attack = skill_table[dt].noun_damage;

      else if (dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
        attack = attack_table[dt - TYPE_HIT].noun;

      else
      {
        bug ("Dam_message: bad dt %d.", dt);
        dt = TYPE_HIT;
        attack = attack_table[0].name;
      }
      if (is_tail_attack)
        attack = "lightning tail";
      sprintf (buf1,
        "$n's %s is absorbed by the magic of $N's mirror.",
        attack);
      sprintf (buf2,
        "Your %s is absorbed by the magic of $N's mirror.",
        attack);
      sprintf (buf3,
        "$n's %s is absorbed by the magic of your mirror.",
        attack);
      act (buf1, ch, NULL, victim, TO_NOTVICT);
      act (buf2, ch, NULL, victim, TO_CHAR);
      act (buf3, ch, NULL, victim, TO_VICT);
      return;
    }
    victim = ch;
  }

  //Iblis 10/11/04 - dealing with wood/silver/iron shit
  if (wield)
  {
    if (!str_cmp(wield->material,"wood"))
    {
      if (IS_SET(victim->vuln_flags,VULN_WOOD))
        dam += dam*.10;
      if (IS_SET(victim->res_flags,RES_WOOD))
        dam -= dam*.10;
      //this should generate the "Your weapon is powerless" message but this is good enough for now
      if (IS_SET(victim->imm_flags,IMM_WOOD))
        dam = 0;
    }
    else if (!str_cmp(wield->material,"silver"))
    {
      if (IS_SET(victim->vuln_flags,VULN_SILVER))
        dam += dam*.10;
      if (IS_SET(victim->res_flags,RES_SILVER))
        dam -= dam*.10;
      if (IS_SET(victim->imm_flags,IMM_SILVER))
        dam = 0;
    }
    else if (!str_cmp(wield->material,"iron") || !str_cmp(wield->material,"steel"))
    {
      if (IS_SET(victim->vuln_flags,VULN_IRON))
        dam += dam*.10;
      if (IS_SET(victim->res_flags,RES_IRON))
        dam -= dam*.10;
      if (IS_SET(victim->imm_flags,IMM_IRON))
        dam = 0;
    }
  } 

  if (!missed_attack)
  {
    if (dt != gsn_strike)		// && dt != gsn_counter)
      result = damage (ch, victim, dam, dt, dam_type, TRUE);

    else
      result = damage (ch, victim, dam, dt, dam_type, FALSE);
  }
  //Iblis 10/11/04
  else
  {
    missed_attack = FALSE;
    crit_strike_possible = FALSE;
    if (wield != NULL && ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_WINDSLASH))
    {
      act("A force of air slashes into $n!",victim,NULL,ch,TO_ROOM);
      act("A force of air slashes into you!",victim,NULL,ch,TO_CHAR);
      damage(ch,victim,dam/3,dt,DAM_WIND,FALSE);    
      result = FALSE;
      wield = NULL;
    }
  }
  crit_strike_possible = FALSE;
  if (result && IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_DARKHOUNDS)
  {
    act("`aA darkhound returns to the underworld.``",ch,NULL,NULL,TO_ROOM);
    extract_char (ch, TRUE);
    return;
  }
  if (result && is_affected (victim, gsn_festering_boils) && number_percent() <= 3)
  {
    spell_plague(gsn_plague,ch->level,ch,(void *)ch,TARGET_CHAR);
  }
  if (result && is_affected (victim, gsn_fireshield))
    if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
    {
      damage (victim, ch, dam / 2, 0, DAM_FIRE, FALSE);
      act ("$N's scorching shield of flame burns you as you attack.",
        ch, NULL, victim, TO_CHAR);
      act ("$n is burnt by your scorching shield of flame.", ch,
        NULL, victim, TO_VICT);
    }

    if (ch->fighting == victim && ch->race == PC_RACE_NERIX && result)
    {
      dam = number_range (1, dam / 10 + 2);
      if (!is_tail_attack)
      {
        act ("$n is struck by lightning from $N's lightning claw.", victim, NULL, ch, TO_ROOM);
        act ("You are shocked by $N's lightning claw.", victim, NULL, ch, TO_CHAR);
      }
      else
      {
        act ("$n is struck by lightning from $N's lightning tail.", victim, NULL, ch, TO_ROOM);
        act ("You are shocked by $N's lightning tail.", victim, NULL, ch, TO_CHAR);
      }
      shock_effect (victim, ch->level / 2, dam, TARGET_CHAR);
      damage (ch, victim, dam, 0, DAM_LIGHTNING, FALSE);
    }
    crit_strike_possible = TRUE;

    /* but do we have a funky weapon? */
    //Iblis 6/13/04 - For Liches who have contaminated the victim
    if (result && wield == NULL && ch->fighting == victim && (victim->contaminator != NULL && victim->contaminator == ch))
    {
      dam = number_range (1, dam / 5 + 1);
      act ("Life is drawn from $n.", victim, NULL, NULL, TO_ROOM);
      act ("You feel $p drawing your life away.", victim, wield, NULL, TO_CHAR);
      damage_old (ch, victim, dam, 0, DAM_NEGATIVE, FALSE);
      if (!IS_CLASS (ch, PC_CLASS_REAVER))
        change_alignment(ch,-1);
      ch->hit += dam / 2;
    }

    //Iblis 9/29/04 - For Chaos Jesters with Fire Dance
    if (result && wield == NULL && ch->fighting == victim && is_affected(ch,gsn_fire_dance))
    {
      dam = number_range (1, dam / 4 + 1);
      act ("$n is burned.", victim, NULL, NULL, TO_ROOM);
      act ("Your flesh is seared.", victim, NULL, NULL, TO_CHAR);
      fire_effect ((void *) victim, ch->level / 2, dam, TARGET_CHAR);
      damage (ch, victim, dam, 0, DAM_FIRE, FALSE);
    }

    if (result && wield != NULL)
    {
      crit_strike_possible = FALSE;
      //5-11-03 Iblis - We need the old value of dam, so we don't create a new dam now
      if (dam > 100)
        dam = 100;		//to set dam to a more reasonable value

      //10/11/04 - New HOLY weapon flag
      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_HOLY) && number_percent() <= 3 && !IS_WEAPON_STAT(wield,WEAPON_HOLY_ACTIVATED))
      {
        SET_BIT(wield->value[4],WEAPON_HOLY_ACTIVATED);
        act ("$p bursts into white flames!",ch,wield,NULL,TO_ALL);
      }

      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_HOLY_ACTIVATED))
      {
        act ("$p scorches $n with holy flames!",victim,wield,NULL,TO_ROOM);
        act ("$p scorches you with holy flames!",victim,wield,NULL,TO_CHAR);
        damage (ch, victim, 10, 0, DAM_HOLY, FALSE);
      }

      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_POISON))
      {
        int level;
        AFFECT_DATA *poison, af;
        if ((poison = affect_find (wield->affected, gsn_poison)) == NULL)
          level = wield->level;

        else
          level = poison->level;
        if (!saves_spell (level / 2, victim, DAM_POISON))
        {
          send_to_char
            ("You feel poison coursing through your veins.", victim);
          act ("$n is poisoned by the venom on $p.", victim, wield,
            NULL, TO_ROOM);
          af.where = TO_AFFECTS;
          af.type = gsn_poison;
          af.level = level * 3 / 4;
          af.duration = level / 2;
          af.location = APPLY_STR;
          af.modifier = -1;
          af.bitvector = AFF_POISON;
          af.permaff = FALSE;
          af.composition = FALSE;
          af.comp_name = str_dup (ch->name);
          affect_join (victim, &af);
        }

      }
      if (ch->fighting == victim && (IS_WEAPON_STAT (wield, WEAPON_VAMPIRIC) || (victim->contaminator != NULL && victim->contaminator == ch)))
      {

        //Iblis 5-11-03 In attempting to fix the fact that vampiric does nothing
        //when activated on a reaver weapon, it was determined that was because
        //reaver weapons are level 0.  Iverath and I decided extra flags should
        //do damage based on the damage your attack does.
        dam = number_range (1, dam / 5 + 1);

        act ("$p draws life from $n.", victim, wield, NULL, TO_ROOM);
        act ("You feel $p drawing your life away.", victim, wield,
          NULL, TO_CHAR);
        damage_old (ch, victim, dam, 0, DAM_NEGATIVE, FALSE);

        // Minax 1-17-03 Reavers do not get alignment lowered by vampiric use
        if (!IS_CLASS (ch, PC_CLASS_REAVER)) 
          change_alignment(ch,-1);
        ch->hit += dam / 2;
      }
      if (ch->fighting == victim && IS_OBJ_STAT (wield, ITEM_BLESS))
        change_alignment(ch,1);
      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_FLAMING))
      {

        //5-11-03 Iblis - Change the damage due to depend on the damage of the weapon
        //attack instead of the weapon's level
        dam = number_range (1, dam / 4 + 1);
        act ("$n is burned by $p.", victim, wield, NULL, TO_ROOM);
        act ("$p sears your flesh.", victim, wield, NULL, TO_CHAR);
        fire_effect ((void *) victim, wield->level / 2, dam, TARGET_CHAR);
        damage (ch, victim, dam, 0, DAM_FIRE, FALSE);
      }
      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_FROST))
      {

        //5-11-03 Iblis - Change the damage due to depend on the damage of the weapon
        //attack instead of the weapon's level
        dam = number_range (1, dam / 6 + 2);
        act ("$p freezes $n.", victim, wield, NULL, TO_ROOM);
        act ("The cold touch of $p surrounds you with ice.", victim,
          wield, NULL, TO_CHAR);
        cold_effect (victim, wield->level / 2, dam, TARGET_CHAR);
        damage (ch, victim, dam, 0, DAM_COLD, FALSE);
      }
      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_SHOCKING))
      {

        //5-11-03 Iblis - Change the damage due to depend on the damage of the weapon
        //attack instead of the weapon's level
        dam = number_range (1, dam / 5 + 2);
        act ("$n is struck by lightning from $p.", victim, wield, NULL,
          TO_ROOM);
        act ("You are shocked by $p.", victim, wield, NULL, TO_CHAR);
        shock_effect (victim, wield->level / 2, dam, TARGET_CHAR);
        damage (ch, victim, dam, 0, DAM_LIGHTNING, FALSE);
      }
      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_LARVA) && number_percent() <= 10)
      {
        act("As $p hits you, a larva burrows into your skin.",victim,wield,NULL,TO_CHAR);
        act("As $p hits $n, a larva burrows into $s skin.",victim,wield,NULL,TO_ROOM);
        larva_hit(ch,victim);
      }
      crit_strike_possible = TRUE;
    }

    //Iblis 10/03/04 - dice weapon's spell hit
    if (ch->fighting == victim && wield && wield->item_type == ITEM_WEAPON && wield->value[0] == WEAPON_DICE && number_percent() <= 5
      && wield->value[5] > 0 && wield->value[5] < MAX_SKILL && skill_table[wield->value[5]].name != NULL
      && skill_table[wield->value[5]].type != SKILL_NORMAL && get_skill(ch,gsn_dice) > 1)
    {
      (*skill_table[wield->value[5]].spell_fun) (wield->value[5], ch->level, ch, (void*)victim, TARGET_CHAR);
    }
    trip_triggers(ch, OBJ_TRIG_CHANCE_ON_HIT, NULL, victim, OT_SPEC_NONE);

    return;
}

void
one_hit_throw (CHAR_DATA * ch, CHAR_DATA * victim, int dt, OBJ_DATA * wield)
{
  int victim_ac;
  int thac0;
  int thac0_00;
  int thac0_32;
  int dam;
  int diceroll;
  int sn;
  int dam_type = DAM_BASH;
  bool result;
  sn = -1;

  /* just in case */
  if (victim == ch || ch == NULL || victim == NULL)
    return;
  if (victim->position == POS_DEAD)
    return;
  if (dt == TYPE_UNDEFINED)
  {
    dt = TYPE_HIT;
    if (wield != NULL && wield->item_type == ITEM_WEAPON)
      dt += wield->value[3];

    else
      dt += ch->dam_type;
  }
  if (dt < TYPE_HIT)
    if (wield != NULL)
      dam_type = attack_table[wield->value[3]].damage;

    else
      dam_type = attack_table[ch->dam_type].damage;

  else
  {
    if ((dt - TYPE_HIT) < 0 || (dt - TYPE_HIT) >  MAX_DAMAGE_MESSAGE)
    {
      char logbuf[MAX_STRING_LENGTH];
      dam_type = DAM_BASH;
      sprintf (logbuf, "DAMTYPE/attack_table error: %s", ch->name);
      if (IS_NPC (ch))
        sprintf (logbuf + strlen (logbuf), " (vnum: %d)",
        ch->pIndexData->vnum);
      log_string (logbuf);
    }

    else
      dam_type = attack_table[dt - TYPE_HIT].damage;
  }
  if (dam_type == -1)
    dam_type = DAM_BASH;

  /*
  * Calculate to-hit-armor-Class-0 versus armor.
  */
  if (IS_NPC (ch))
  {
    thac0_00 = 20;
    thac0_32 = -4;		/* as good as a thief */
    if (IS_SET (ch->act, ACT_WARRIOR))
      thac0_32 = -10;

    else if (IS_SET (ch->act, ACT_THIEF))
      thac0_32 = -4;

    else if (IS_SET (ch->act, ACT_CLERIC))
      thac0_32 = 2;

    else if (IS_SET (ch->act, ACT_MAGE))
      thac0_32 = 6;
  }

  else
  {
    thac0_00 = Class_table[current_Class (ch)].thac0_00;
    thac0_32 = Class_table[current_Class (ch)].thac0_32;
  }
  thac0 = interpolate (ch->level, thac0_00, thac0_32);
  if (thac0 < 0)
    thac0 = thac0 / 2;
  if (thac0 < -5)
    thac0 = -5 + (thac0 + 5) / 2;
  thac0 -= get_hitroll2 (ch);
  thac0 += 5 * 0 / 100;
  if (dt == gsn_backstab)
    thac0 -= 10 * (100 - get_skill (ch, gsn_backstab));
  if (dt == gsn_shadowslash)
    thac0 -= 10 * (100 - get_skill (ch, gsn_shadowslash));
  if (dt == gsn_eviscerate)
    thac0 -= 10 * (100 - get_skill (ch, gsn_eviscerate));
  switch (dam_type)
  {
  case (DAM_PIERCE):
    victim_ac = GET_AC (victim, AC_PIERCE) / 10;
    break;
  case (DAM_BASH):
    victim_ac = GET_AC (victim, AC_BASH) / 10;
    break;
  case (DAM_SLASH):
    victim_ac = GET_AC (victim, AC_SLASH) / 10;
    break;
  default:
    victim_ac = GET_AC (victim, AC_EXOTIC) / 10;
    break;
  };
  if (victim_ac < -15)
    victim_ac = (victim_ac + 15) / 5 - 15;
  if (!can_see (ch, victim))
  {
    if (number_percent () < get_skill (ch, gsn_blindfighting))
      check_improve (ch, gsn_blindfighting, TRUE, 2);

    else
    {
      check_improve (ch, gsn_blindfighting, FALSE, 2);
      victim_ac -= 4;
    }
  }
  if (victim->position < POS_FIGHTING)
    victim_ac += 4;
  if (victim->position < POS_RESTING)
    victim_ac += 6;

  /*
  * Hit.
  * Calc damage.
  */
  if (IS_NPC (ch) && (wield == NULL))
    dam = dice (ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);

  else
  {
    if (sn != -1)
      check_improve (ch, sn, TRUE, 5);
    if (wield != NULL)
    {

      dam = dice (wield->value[1], wield->value[2]);
      if (get_eq_char (ch, WEAR_SHIELD) == NULL)	/* no shield = more */
        dam = dam * 11 / 10;

      /* sharpness! */
      if (IS_WEAPON_STAT (wield, WEAPON_SHARP))
      {
        int percent;
        if ((percent = number_percent ()) <= (100 / 8))
          dam = 2 * dam + (dam * 2 * percent / 100);
      }
    }

    else
    {
      if (ch->race == PC_RACE_NERIX)
      {
        int percent;
        //Need a way to differentiate normal / tail attacks
        if (is_tail_attack)
        {
          dam = (dice (ch->level/2 + 15,1) + 2*(ch->level/10))/ 2;
        }
        else
        {
          dam = dice (ch->level/2 + 15,1); /* Normal claw attack */
          dam += 2*(ch->level/10);
          if ((percent = number_percent ()) <= (100 / 8))
            dam = 2 * dam + (dam * 2 * percent / 100);     /* Simulated sharp claw */
        }
      }
      else dam = number_range (1 + 4, 2 * ch->level / 3);
    }
  }

  /*
  * Bonuses.
  */
  if (get_skill (ch, gsn_enhanced_damage) > 0)
  {
    diceroll = number_percent ();
    if (diceroll <= get_skill (ch, gsn_enhanced_damage))
    {
      check_improve (ch, gsn_enhanced_damage, TRUE, 6);
      dam += 2 * (dam * diceroll / 300);
    }
  }
  if (!IS_AWAKE (victim))
    dam *= 2;

  else if (victim->position < POS_FIGHTING)
    dam = dam * 3 / 2;
  if (dt == gsn_uppercut || dt == gsn_strike)
  {
    dam = ch->level + (get_curr_stat (ch, STAT_STR) * number_range (1, 4));
    dam += (get_skill (ch, gsn_hand_to_hand) / 3);
  }
  if (dt == gsn_backstab && wield != NULL)
  {
    if (wield->value[0] != 2)
      dam *= 2 + (ch->level / 10);

    else
      dam *= 2 + (ch->level / 8);
  }

  else if (dt == gsn_assassinate)
    //Iblis 6/22/03 -changed below a bit to compensate for other asssassinate changes
    dam *= 3 + (90 / 2);

  else if (dt == gsn_throwing)
    dam *= 2 + (ch->level / 16);
  dam += get_damroll (ch) * UMIN (100, 100) / 100;
  if (dam <= 0)
    dam = 1;
  if (dt == gsn_assassinate)
  {
    short counter1 = 0, denom = 0;
    if (ch->Class == 7 || ch->Class == 2)
      counter1++;
    if (ch->level > 30 && (ch->Class2 == 7 || ch->Class2 == 2))
      counter1++;
    if (ch->level > 60 && (ch->Class3 == 7 || ch->Class3 == 2))
      counter1++;
    dam = dam * (counter1 / 3.0);
    dam = dam * (500.0 / 950);

    denom = number_range (1, 100);
    if (((counter1 == 2 && denom == 46)
      || (counter1 == 3 && denom == 47)) 
      && !IS_IMMORTAL(ch) && check_immune (victim, dam_type) != IS_IMMUNE)
    {
      insta_kill (ch, victim, dam, dt, dam_type, TRUE, FALSE);
      return;
    }
  }
  //Iblis 10/03/04 - For Flick and Shuffle skill
  if (wield != NULL && wield->item_type == ITEM_CARD)
  {
    dam = dice(wield->value[2],wield->value[3]) + wield->value[4];
    dam_type = DAM_SLASH;
  }

  //Iblis 10/11/04 - dealing with wood/silver/iron shit
  if (wield)
  {
    if (!str_cmp(wield->material,"wood"))
    {
      if (IS_SET(victim->vuln_flags,VULN_WOOD))
        dam += dam*.10;
      if (IS_SET(victim->res_flags,RES_WOOD))
        dam -= dam*.10;
      //this should generate the "Your weapon is powerless" message but this is good enough for now
      if (IS_SET(victim->imm_flags,IMM_WOOD))
        dam = 0;
    }
    else if (!str_cmp(wield->material,"silver"))
    {
      if (IS_SET(victim->vuln_flags,VULN_SILVER))
        dam += dam*.10;
      if (IS_SET(victim->res_flags,RES_SILVER))
        dam -= dam*.10;
      if (IS_SET(victim->imm_flags,IMM_SILVER))
        dam = 0;
    }
    else if (!str_cmp(wield->material,"iron") || !str_cmp(wield->material,"steel"))
    {
      if (IS_SET(victim->vuln_flags,VULN_IRON))
        dam += dam*.10;
      if (IS_SET(victim->res_flags,RES_IRON))
        dam -= dam*.10;
      if (IS_SET(victim->imm_flags,IMM_IRON))
        dam = 0;
    }
  }

  result = damage (ch, victim, dam, dt, dam_type, TRUE);
  if (is_affected (victim, gsn_mirror) && dam_type != DAM_ACID
    && dam_type != DAM_ENERGY && dam_type != DAM_HOLY
    && dam_type != DAM_LIGHTNING && dam_type != DAM_FIRE
    && dam_type != DAM_COLD && dam_type != DAM_NEGATIVE
    && dam_type != DAM_MENTAL && dam_type != DAM_LIGHT
    && dam_type != DAM_CHARM && dam_type != DAM_DISEASE
    && dam_type != DAM_SOUND && dam_type != DAM_HARM)
  {
    if (ch->level >= (victim->level + 10) && number_percent () > 50)
    {
      char *attack;
      char buf1[MAX_STRING_LENGTH];
      char buf2[MAX_STRING_LENGTH];
      char buf3[MAX_STRING_LENGTH];
      if (dt >= 0 && dt < MAX_SKILL)
        attack = skill_table[dt].noun_damage;

      else if (dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
        attack = attack_table[dt - TYPE_HIT].noun;

      else
      {
        bug ("Dam_message: bad dt %d.", dt);
        dt = TYPE_HIT;
        attack = attack_table[0].name;
      }
      if (is_tail_attack)
        attack = "lightning tail";
      sprintf (buf1,
        "$n's %s is absorbed by the magic of $N's mirror.",
        attack);
      sprintf (buf2,
        "Your %s is absorbed by the magic of $N's mirror.",
        attack);
      sprintf (buf3,
        "$n's %s is absorbed by the magic of your mirror.",
        attack);
      act (buf1, ch, NULL, victim, TO_NOTVICT);
      act (buf2, ch, NULL, victim, TO_CHAR);
      act (buf3, ch, NULL, victim, TO_VICT);
      return;
    }
    victim = ch;
  }
  //Iblis 6/13/04 - For Liches who have contaminated the victims
  if (result && wield == NULL && ch->fighting == victim && (victim->contaminator != NULL && victim->contaminator == ch))
  {
    dam = number_range (1, dam / 5 + 1);
    act ("Life is drawn from $n.", victim, NULL, NULL, TO_ROOM);
    act ("You feel $p drawing your life away.", victim, wield, NULL, TO_CHAR);
    damage_old (ch, victim, dam, 0, DAM_NEGATIVE, FALSE);
    if (!IS_CLASS (ch, PC_CLASS_REAVER))
      change_alignment(ch,-1);
    ch->hit += dam / 2;
  }

  //Iblis 9/29/04 - For Chaos Jesters with Fire Dance
  if (result && wield == NULL && ch->fighting == victim && is_affected(ch,gsn_fire_dance))
  {
    dam = number_range (1, dam / 4 + 1);
    act ("$n is burned.", victim, NULL, NULL, TO_ROOM);
    act ("Your flesh is seared.", victim, NULL, NULL, TO_CHAR);
    fire_effect ((void *) victim, ch->level / 2, dam, TARGET_CHAR);
    damage (ch, victim, dam, 0, DAM_FIRE, FALSE);
  }

  if (result && wield != NULL && wield->item_type != ITEM_CARD)
  {
    crit_strike_possible = FALSE;
    //5-11-03 Iblis - We need the old value of dam, so we don't create a new dam now
    if (dam > 100)
      dam = 100;		//to set dam to a more reasonable value
    if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_POISON))
    {
      int level;
      AFFECT_DATA *poison, af;
      if ((poison = affect_find (wield->affected, gsn_poison)) == NULL)
        level = wield->level;

      else
        level = poison->level;
      if (!saves_spell (level / 2, victim, DAM_POISON))
      {
        send_to_char
          ("You feel poison coursing through your veins.", victim);
        act ("$n is poisoned by the venom on $p.", victim, wield,
          NULL, TO_ROOM);
        af.where = TO_AFFECTS;
        af.type = gsn_poison;
        af.level = level * 3 / 4;
        af.duration = level / 2;
        af.location = APPLY_STR;
        af.modifier = -1;
        af.bitvector = AFF_POISON;
        af.permaff = FALSE;
        af.composition = FALSE;
        af.comp_name = str_dup (ch->name);
        affect_join (victim, &af);
      }

    }
    if (ch->fighting == victim && (IS_WEAPON_STAT (wield, WEAPON_VAMPIRIC)  || (victim->contaminator != NULL && victim->contaminator == ch)))
    {

      //Iblis 5-11-03 In attempting to fix the fact that vampiric does nothing
      //when activated on a reaver weapon, it was determined that was because
      //reaver weapons are level 0.  Iverath and I decided extra flags should
      //do damage based on the damage your attack does.
      dam = number_range (1, dam / 5 + 1);
      act ("$p draws life from $n.", victim, wield, NULL, TO_ROOM);
      act ("You feel $p drawing your life away.", victim, wield,
        NULL, TO_CHAR);
      damage_old (ch, victim, dam, 0, DAM_NEGATIVE, FALSE);

      // Minax 1-17-03 Reavers do not get alignment lowered by vampiric use
      if (!IS_CLASS (ch, PC_CLASS_REAVER)) 
        change_alignment(ch,-1);
      ch->hit += dam / 2;
    }
    if (ch->fighting == victim && IS_OBJ_STAT (wield, ITEM_BLESS))
      change_alignment(ch,1);
    if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_FLAMING))
    {

      //5-11-03 Iblis - Change the damage due to depend on the damage of the weapon
      //attack instead of the weapon's level
      dam = number_range (1, dam / 4 + 1);
      act ("$n is burned by $p.", victim, wield, NULL, TO_ROOM);
      act ("$p sears your flesh.", victim, wield, NULL, TO_CHAR);
      fire_effect ((void *) victim, wield->level / 2, dam, TARGET_CHAR);
      damage (ch, victim, dam, 0, DAM_FIRE, FALSE);
    }
    if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_FROST))
    {

      //5-11-03 Iblis - Change the damage due to depend on the damage of the weapon
      //attack instead of the weapon's level
      dam = number_range (1, dam / 6 + 2);
      act ("$p freezes $n.", victim, wield, NULL, TO_ROOM);
      act ("The cold touch of $p surrounds you with ice.", victim,
        wield, NULL, TO_CHAR);
      cold_effect (victim, wield->level / 2, dam, TARGET_CHAR);
      damage (ch, victim, dam, 0, DAM_COLD, FALSE);
    }
    if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_SHOCKING))
    {

      //5-11-03 Iblis - Change the damage due to depend on the damage of the weapon
      //attack instead of the weapon's level
      dam = number_range (1, dam / 5 + 2);
      act ("$n is struck by lightning from $p.", victim, wield, NULL,
        TO_ROOM);
      act ("You are shocked by $p.", victim, wield, NULL, TO_CHAR);
      shock_effect (victim, wield->level / 2, dam, TARGET_CHAR);
      damage (ch, victim, dam, 0, DAM_LIGHTNING, FALSE);
    }
    if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_LARVA) && number_percent() <= 10)
    {
      act("As $p hits you, a larva burrows into your skin.",victim,wield,NULL,TO_CHAR);
      act("As $p hits $n, a larva burrows into $s skin.",victim,wield,NULL,TO_ROOM);
      larva_hit(ch,victim);
    }
    crit_strike_possible = TRUE;
  }

  trip_triggers(ch, OBJ_TRIG_CHANCE_ON_HIT, NULL, victim, OT_SPEC_NONE);

  return;
}

void one_dual_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
  OBJ_DATA *wield;
  int victim_ac;
  int thac0;
  int thac0_00;
  int thac0_32;
  int dam;
  int diceroll;
  int sn, skill;
  int dam_type = DAM_BASH, wield_type;
  bool result=TRUE;
  missed_attack = FALSE;
  sn = -1;
  if (victim == ch->mount)
  {
    act ("$N bucks wildly, throwing you to the ground.", ch, NULL,
      victim, TO_CHAR);
    act ("You buck wildly, throwing $n to the ground.", ch, NULL,
      victim, TO_VICT);
    act ("$N bucks wildly, throwing $n to the ground.", ch, NULL,
      victim, TO_NOTVICT);
    ch->mount->riders = NULL;
    ch->mount = NULL;
    if (!ch->bashed)
      ch->position = POS_FIGHTING;
  }
  if (IS_NPC (ch))
    wield_type = WEAR_WIELD_L;

  else
  {
    if (ch->pcdata->primary_hand == HAND_LEFT)
      wield_type = WEAR_WIELD_R;

    else
      wield_type = WEAR_WIELD_L;
  }

  /* just in case */
  if (victim == ch || ch == NULL || victim == NULL)
    return;

  /*
  * Can't beat a dead char!
  * Guard against weird room-leavings.
  */
  if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
    return;

  /*
  * Figure out the type of damage message.
  */
  wield = get_eq_char (ch, wield_type);
  if (dt == TYPE_UNDEFINED)
  {
    dt = TYPE_HIT;
    if (wield != NULL && wield->item_type == ITEM_WEAPON)
      dt += wield->value[3];

    else
      dt += ch->dam_type;
  }
  if (!IS_NPC (ch) && ch->mount != NULL)
  {
    if (wield != NULL)
    {
      if (!IS_SET (wield->value[4], WEAPON_LANCE)
        && (wield->value[0] != WEAPON_POLEARM))
      {
        char tcbuf[MAX_STRING_LENGTH];
        sprintf (tcbuf, "Your %s is useless while mounted.\n\r",
          wield->name);
        send_to_char (tcbuf, ch);
        damage (ch, victim, 0, dt, dam_type, FALSE);
        return;
      }
    }

    else
    {
      send_to_char ("You cannot punch from your mount.\n\r", ch);
      damage (ch, victim, 0, dt, dam_type, FALSE);
      return;
    }
  }
  if (victim->mount != NULL && wield == NULL && !IS_NPC (ch) && ch->race != PC_RACE_NERIX)
  {
    act ("You swing your arms wildly, but you fail to reach $N.", ch,
      NULL, victim, TO_CHAR);
    act ("$n swings $s arms wildly, but fails to reach $N.", ch, NULL,
      victim, TO_NOTVICT);
    act ("$n swings $s arms wildly, but fails to reach you.", ch,
      NULL, victim, TO_VICT);
    return;
  }
  if (dt < TYPE_HIT)
    if (wield != NULL)
      dam_type = attack_table[wield->value[3]].damage;

    else
      dam_type = attack_table[ch->dam_type].damage;

  else
    dam_type = attack_table[dt - TYPE_HIT].damage;
  if (dam_type == -1)
    dam_type = DAM_BASH;

  if (ch->race == PC_RACE_NERIX)
    skill = 120;
  else
  {
    /* get the weapon skill */
    sn = get_weapon_sn (ch, wield_type);
    skill = 20 + get_weapon_skill (ch, sn);
  }

  if (!new_ac)
  {
    /*
    * Calculate to-hit-armor-Class-0 versus armor.
    */
    if (IS_NPC (ch))
    {
      thac0_00 = 20;
      thac0_32 = -4;		/* as good as a thief */
      if (IS_SET (ch->act, ACT_WARRIOR))
        thac0_32 = -10;

      else if (IS_SET (ch->act, ACT_THIEF))
        thac0_32 = -4;

      else if (IS_SET (ch->act, ACT_CLERIC))
        thac0_32 = 2;

      else if (IS_SET (ch->act, ACT_MAGE))
        thac0_32 = 6;
    }

    else
    {
      thac0_00 = Class_table[current_Class (ch)].thac0_00;
      thac0_32 = Class_table[current_Class (ch)].thac0_32;
    }
    thac0 = interpolate (ch->level, thac0_00, thac0_32);
    if (thac0 < 0)
      thac0 = thac0 / 2;
    if (thac0 < -5)
      thac0 = -5 + (thac0 + 5) / 2;
    thac0 -= (get_hitroll2 (ch) + adjust_hitroll (ch)) * skill / 100;
    thac0 += 5 * (100 - skill) / 100;
    if (dt == gsn_backstab)
      thac0 -= 10 * (100 - get_skill (ch, gsn_backstab));
	 if (dt == gsn_shadowslash)
      thac0 -= 10 * (100 - get_skill (ch, gsn_shadowslash));
    if (dt == gsn_eviscerate)
      thac0 -= 10 * (100 - get_skill (ch, gsn_eviscerate));
    switch (dam_type)
    {
    case (DAM_PIERCE):
      victim_ac = GET_AC (victim, AC_PIERCE) / 10;
      break;
    case (DAM_BASH):
      victim_ac = GET_AC (victim, AC_BASH) / 10;
      break;
    case (DAM_SLASH):
      victim_ac = GET_AC (victim, AC_SLASH) / 10;
      break;
    default:
      victim_ac = GET_AC (victim, AC_EXOTIC) / 10;
      break;
    };
    if (victim_ac < -15)
      victim_ac = (victim_ac + 15) / 5 - 15;
    if (!can_see (ch, victim))
    {
      if (number_percent () < get_skill (ch, gsn_blindfighting))
        check_improve (ch, gsn_blindfighting, TRUE, 2);

      else
      {
        check_improve (ch, gsn_blindfighting, FALSE, 2);
        victim_ac -= 4;
      }
    }
    if (victim->position < POS_FIGHTING)
      victim_ac += 4;
    if (victim->position < POS_RESTING)
      victim_ac += 6;
    if (ignore_ac)
      victim_ac = 0;

    /*
    * The moment of excitement!
    */
    while ((diceroll = number_bits (5)) >= 20);
    if (diceroll == 0 || (diceroll != 19 && diceroll < thac0 - victim_ac))
    {

      /* Miss. */
      damage (ch, victim, 0, dt, dam_type, TRUE);
      if (!(wield != NULL && ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_WINDSLASH)))
        return;
    }
  }
  else
  {
    int hitroll = get_hitroll2 (ch) + adjust_hitroll (ch);
    int chance=0,i,Class;
    switch (dam_type)
    {
    case (DAM_PIERCE):
      victim_ac = GET_AC (victim, AC_PIERCE);
      break;
    case (DAM_BASH):
      victim_ac = GET_AC (victim, AC_BASH);
      break;
    case (DAM_SLASH):
      victim_ac = GET_AC (victim, AC_SLASH);
      break;
    default:
      victim_ac = GET_AC (victim, AC_EXOTIC);
      break;
    }
    if (hitroll >= 100)
      chance = 50 + (hitroll-100)/20.0;
    else if (hitroll > 10)
      chance = 30+((hitroll-10)*(20/90.0));
    else if (hitroll == 10)
      chance = 30;
    else if (hitroll > -20)
      chance = (hitroll+20)*(30/30.0);
    else chance = 1;
    if (victim_ac > 500)
      chance += 50;
    else if (victim_ac > 0)
      chance += 30+(victim_ac*(20/500.0));
    else if (victim_ac == 0)
      chance += 30;
    else if (victim_ac > -600)
      chance += 30 - (-victim_ac*(30/600.0));
    else chance += 1;
    for (i = 0,Class=0;i<3;i++)
    {
      if (i==0)
      Class = ch->Class;
      else if (i==1)
      Class = ch->Class2;
      else Class = ch->Class3;
      if (i==1 && ch->level < 31)
        break;
      if (i==2 && ch->level < 61)
        break;
      switch (Class)
      {
      case PC_CLASS_MAGE:
      case PC_CLASS_CLERIC:
      case PC_CLASS_NECROMANCER:
      case PC_CLASS_BARD:
      case PC_CLASS_DRUID:
      default:
        break;
      case PC_CLASS_THIEF:
      case PC_CLASS_ASSASSIN:
        chance += 1;break;
      case PC_CLASS_WARRIOR:
        chance += 5;break;
      case PC_CLASS_PALADIN:
      case PC_CLASS_REAVER:
      case PC_CLASS_RANGER:
      case PC_CLASS_MONK:
        chance += 3;break;
      }
    }

    if (number_percent() >= chance)
    {
      damage (ch, victim, 0, dt, dam_type, TRUE);
      if (!(wield != NULL && ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_WINDSLASH)))
        return;
    }
  }
  /*
  * Hit.
  * Calc damage.
  */
  if (IS_NPC (ch) && (wield == NULL))
    dam = dice (ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);

  else
  {
    if (sn != -1)
      check_improve (ch, sn, TRUE, 5);
    if (wield != NULL)
    {
      dam = dice (wield->value[1], wield->value[2]) * skill / 100;
      if (get_eq_char (ch, WEAR_SHIELD) == NULL)	/* no shield = more */
        dam = dam * 11 / 10;

      /* sharpness! */
      if (IS_WEAPON_STAT (wield, WEAPON_SHARP))
      {
        int percent;
        if ((percent = number_percent ()) <= (skill / 8))
          dam = 2 * dam + (dam * 2 * percent / 100);
      }
    }

    else
    {
      if (ch->race == PC_RACE_NERIX)
      {
        int percent;
        //Need a way to differentiate normal / tail attacks
        if (is_tail_attack)
        {
          dam = (dice (ch->level/2 + 15,1) + 2*(ch->level/10))/ 2;
        }
        else
        {
          dam = dice (ch->level/2 + 15,1); /* Normal claw attack */
          dam += 2*(ch->level/10);
          if ((percent = number_percent ()) <= (skill / 8))
            dam = 2 * dam + (dam * 2 * percent / 100);     /* Simulated sharp claw */
        }
      }
      else dam = number_range (1 + 4 * skill / 100, 2 * ch->level / 3 * skill / 100);
    }
  }
  if (!IS_AWAKE (victim))
    dam *= 2;

  else if (victim->position < POS_FIGHTING)
    dam = dam * 3 / 2;
  dam += (get_damroll (ch) + adjust_damroll (ch)) * UMIN (100, skill) / 100;
  if (dam <= 0)
    dam = 1;

  //Iblis 10/11/04 - dealing with wood/silver/iron shit
  if (wield)
  {
    if (!str_cmp(wield->material,"wood"))
    {
      if (IS_SET(victim->vuln_flags,VULN_WOOD))
        dam += dam*.10;
      if (IS_SET(victim->res_flags,RES_WOOD))
        dam -= dam*.10;
      //this should generate the "Your weapon is powerless" message but this is good enough for now
      if (IS_SET(victim->imm_flags,IMM_WOOD))
        dam = 0;
    }
    else if (!str_cmp(wield->material,"silver"))
    {
      if (IS_SET(victim->vuln_flags,VULN_SILVER))
        dam += dam*.10;
      if (IS_SET(victim->res_flags,RES_SILVER))
        dam -= dam*.10;
      if (IS_SET(victim->imm_flags,IMM_SILVER))
        dam = 0;
    }
    else if (!str_cmp(wield->material,"iron") || !str_cmp(wield->material,"steel"))
    {
      if (IS_SET(victim->vuln_flags,VULN_IRON))
        dam += dam*.10;
      if (IS_SET(victim->res_flags,RES_IRON))
        dam -= dam*.10;
      if (IS_SET(victim->imm_flags,IMM_IRON))
        dam = 0;
    }
  }

  if (!missed_attack)
    result = damage (ch, victim, dam, dt, dam_type, TRUE);
  else
  {
    crit_strike_possible = FALSE;
    missed_attack = FALSE;
    if (wield != NULL && ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_WINDSLASH))
    {
      act("A force of air slashes into $n!",victim,NULL,ch,TO_ROOM);
      act("A force of air slashes into you!",victim,NULL,ch,TO_CHAR);
      damage(ch,victim,dam/3,dt,DAM_WIND,FALSE);
      result = FALSE;
      wield = NULL;
    }
  }

  if (is_affected (victim, gsn_mirror) && dam_type != DAM_ACID
    && dam_type != DAM_ENERGY && dam_type != DAM_HOLY
    && dam_type != DAM_LIGHTNING && dam_type != DAM_FIRE
    && dam_type != DAM_COLD && dam_type != DAM_NEGATIVE
    && dam_type != DAM_MENTAL && dam_type != DAM_LIGHT
    && dam_type != DAM_CHARM && dam_type != DAM_DISEASE
    && dam_type != DAM_SOUND && dam_type != DAM_HARM)
  {
    if (ch->level >= (victim->level + 10) && number_percent () > 50)
    {
      char *attack;
      char buf1[MAX_STRING_LENGTH];
      char buf2[MAX_STRING_LENGTH];
      char buf3[MAX_STRING_LENGTH];
      if (dt >= 0 && dt < MAX_SKILL)
        attack = skill_table[dt].noun_damage;

      else if (dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
        attack = attack_table[dt - TYPE_HIT].noun;

      else
      {
        bug ("Dam_message: bad dt %d.", dt);
        dt = TYPE_HIT;
        attack = attack_table[0].name;
      }
      if (is_tail_attack)
        attack = "lightning tail";
      sprintf (buf1,
        "$n's %s is absorbed by the magic of $N's mirror.",
        attack);
      sprintf (buf2,
        "Your %s is absorbed by the magic of $N's mirror.",
        attack);
      sprintf (buf3,
        "$n's %s is absorbed by the magic of your mirror.",
        attack);
      act (buf1, ch, NULL, victim, TO_NOTVICT);
      act (buf2, ch, NULL, victim, TO_CHAR);
      act (buf3, ch, NULL, victim, TO_VICT);
      return;
    }
    victim = ch;
  }
  crit_strike_possible = FALSE;
  if (result && is_affected (victim, gsn_fireshield))
    if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
    {
      damage (victim, ch, dam / 2, 0, DAM_FIRE, FALSE);
      act ("$N's scorching shield of flame burns you as you attack.",
        ch, NULL, victim, TO_CHAR);
      act ("$n is burnt by your scorching shield of flame.", ch,
        NULL, victim, TO_VICT);
    }

    if (ch->fighting == victim && ch->race == PC_RACE_NERIX && result)
    {
      dam = number_range (1, dam / 10 + 2);
      if (!is_tail_attack)
      {
        act ("$n is struck by lightning from $N's lightning claw.", victim, NULL, ch, TO_ROOM);
        act ("You are shocked by $N's lightning claw.", victim, NULL, ch, TO_CHAR);
      }
      else
      {
        act ("$n is struck by lightning from $N's lightning tail.", victim, NULL, ch, TO_ROOM);
        act ("You are shocked by $N's lightning tail.", victim, NULL, ch, TO_CHAR);
      }
      shock_effect (victim, ch->level / 2, dam, TARGET_CHAR);
      damage (ch, victim, dam, 0, DAM_LIGHTNING, FALSE);
    }
    crit_strike_possible = TRUE;
    /* but do we have a funky weapon? */
    //Iblis 6/13/04 - For Liches who have contaminated the victim
    if (result && wield == NULL && ch->fighting == victim && (victim->contaminator != NULL && victim->contaminator == ch))
    {
      dam = number_range (1, dam / 5 + 1);
      act ("Life is drawn from $n.", victim, NULL, NULL, TO_ROOM);
      act ("You feel $p drawing your life away.", victim, wield, NULL, TO_CHAR);
      damage_old (ch, victim, dam, 0, DAM_NEGATIVE, FALSE);
      if (!IS_CLASS (ch, PC_CLASS_REAVER))
        change_alignment(ch,-1);
      ch->hit += dam / 2;
    }

    //Iblis 9/29/04 - For Chaos Jesters with Fire Dance
    if (result && wield == NULL && ch->fighting == victim && is_affected(ch,gsn_fire_dance))
    {
      dam = number_range (1, dam / 4 + 1);
      act ("$n is burned.", victim, NULL, NULL, TO_ROOM);
      act ("Your flesh is seared.", victim, NULL, NULL, TO_CHAR);
      fire_effect ((void *) victim, ch->level / 2, dam, TARGET_CHAR);
      damage (ch, victim, dam, 0, DAM_FIRE, FALSE);
    }

    if (result && wield != NULL)
    {
      crit_strike_possible = FALSE;
      //5-11-03 Iblis - We need the old value of dam, so we don't create a new dam now
      if (dam > 100)
        dam = 100;		//to set dam to a more reasonable value

      //10/11/04 - New HOLY weapon flag
      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_HOLY) && number_percent() <= 3 && !IS_WEAPON_STAT(wield,WEAPON_HOLY_ACTIVATED))
      {
        SET_BIT(wield->value[4],WEAPON_HOLY_ACTIVATED);
        act ("$p bursts into white flames!",ch,wield,NULL,TO_ALL);
      }

      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_HOLY_ACTIVATED))
      {
        act ("$p scorches $n with holy flames!",victim,wield,NULL,TO_ROOM);
        act ("$p scorches you with holy flames!",victim,wield,NULL,TO_CHAR);
        damage (ch, victim, 10, 0, DAM_HOLY, FALSE);
      }

      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_POISON))
      {
        int level;
        AFFECT_DATA *poison, af;
        if ((poison = affect_find (wield->affected, gsn_poison)) == NULL)
          level = wield->level;

        else
          level = poison->level;
        if (!saves_spell (level / 2, victim, DAM_POISON))
        {
          send_to_char
            ("You feel poison coursing through your veins.", victim);
          act ("$n is poisoned by the venom on $p.", victim, wield,
            NULL, TO_ROOM);
          af.where = TO_AFFECTS;
          af.type = gsn_poison;
          af.level = level * 3 / 4;
          af.duration = level / 2;
          af.location = APPLY_STR;
          af.modifier = -1;
          af.bitvector = AFF_POISON;
          af.permaff = FALSE;
          af.composition = FALSE;
          af.comp_name = str_dup (ch->name);
          affect_join (victim, &af);
        }

      }
      if (ch->fighting == victim && (IS_WEAPON_STAT (wield, WEAPON_VAMPIRIC)  || (victim->contaminator != NULL && victim->contaminator == ch)))
      {

        //Iblis 5-11-03 In attempting to fix the fact that vampiric does nothing
        //when activated on a reaver weapon, it was determined that was because
        //reaver weapons are level 0.  Iverath and I decided extra flags should
        //do damage based on the damage your attack does.
        dam = number_range (1, dam / 5 + 1);
        act ("$p draws life from $n.", victim, wield, NULL, TO_ROOM);
        act ("You feel $p drawing your life away.", victim, wield,
          NULL, TO_CHAR);
        damage_old (ch, victim, dam, 0, DAM_NEGATIVE, FALSE);

        // Minax 1-17-03 Reavers do not get alignment lowered by vampiric use
        if (!IS_CLASS (ch, PC_CLASS_REAVER)) 
          change_alignment(ch,-1);
        ch->hit += dam / 2;
      }
      if (ch->fighting == victim && IS_OBJ_STAT (wield, ITEM_BLESS))
        change_alignment(ch,1);
      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_FLAMING))
      {

        //5-11-03 Iblis - Change the damage due to depend on the damage of the weapon
        //attack instead of the weapon's level
        dam = number_range (1, dam / 4 + 1);
        act ("$n is burned by $p.", victim, wield, NULL, TO_ROOM);
        act ("$p sears your flesh.", victim, wield, NULL, TO_CHAR);
        fire_effect ((void *) victim, wield->level / 2, dam, TARGET_CHAR);
        damage (ch, victim, dam, 0, DAM_FIRE, FALSE);
      }
      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_FROST))
      {

        //5-11-03 Iblis - Change the damage due to depend on the damage of the weapon
        //attack instead of the weapon's level
        dam = number_range (1, dam / 6 + 2);
        act ("$p freezes $n.", victim, wield, NULL, TO_ROOM);
        act ("The cold touch of $p surrounds you with ice.", victim,
          wield, NULL, TO_CHAR);
        cold_effect (victim, wield->level / 2, dam, TARGET_CHAR);
        damage (ch, victim, dam, 0, DAM_COLD, FALSE);
      }
      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_SHOCKING))
      {

        //5-11-03 Iblis - Change the damage due to depend on the damage of the weapon
        //attack instead of the weapon's level
        dam = number_range (1, dam / 5 + 2);
        act ("$n is struck by lightning from $p.", victim, wield, NULL,
          TO_ROOM);
        act ("You are shocked by $p.", victim, wield, NULL, TO_CHAR);
        shock_effect (victim, wield->level / 2, dam, TARGET_CHAR);
        damage (ch, victim, dam, 0, DAM_LIGHTNING, FALSE);
      }
      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_LARVA) && number_percent() <= 10)
      {
        act("As $p hits you, a larva burrows into your skin.",victim,wield,NULL,TO_CHAR);
        act("As $p hits $n, a larva burrows into $s skin.",victim,wield,NULL,TO_ROOM);
        larva_hit(ch,victim);
      }
      crit_strike_possible = TRUE;
    }

    trip_triggers(ch, OBJ_TRIG_CHANCE_ON_HIT, NULL, victim, OT_SPEC_NONE);

    return;
}

//Iblis 1/09/04 - Intercept function for the soul link spell
bool damage (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type, bool show)
{
  if (victim->race == PC_RACE_SWARM)
  {
    if (victim->level > ch->level+50)
      dam = dice(1,2);
    else dam = dice(5,3);
    if (dam_type == DAM_FIRE)
    {
      dam *= 2;
      dam_type = DAM_UNIQUE;  //So that vuln fire won't come into affect
    }
  }

  return damage2(ch,victim,dam,dt,dam_type,show);
}

/*
* Inflict damage from a hit.
*/
bool
damage2 (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type,
         bool show)
{
  bool mirror_attack = FALSE;
  OBJ_DATA *corpse;
  bool immune;
  CHAR_DATA *tch=NULL, *anothertch=NULL;
  missed_attack = FALSE;
  if (victim->position == POS_DEAD)
    return FALSE;
  if (ch != victim && ch != NULL && victim != NULL)
  {
    if (!IS_NPC (ch))
      ch->pcdata->last_fight = time (NULL);
    if (!IS_NPC (victim))
      victim->pcdata->last_fight = time (NULL);
  }
  if (dam > 120000 && dt >= TYPE_HIT)
  {

    dam = 120000;
    if (!IS_IMMORTAL (ch))
    {
      OBJ_DATA *obj;
      obj = get_eq_char (ch, WEAR_WIELD_R);
      send_to_char ("You really shouldn't cheat.\n\r", ch);
      if (obj != NULL)
        extract_obj (obj);
      obj = get_eq_char (ch, WEAR_WIELD_L);
      if (obj != NULL)
        extract_obj (obj);
    }
  }
  //Iblis 1/13/04 Paladins do 2x damage to all undead
  if (dt != gsn_soul_link)
  {
    if (ch->Class == 6 && IS_SET(victim->act,ACT_UNDEAD))
      dam *= 2;
    if ((dam_type == DAM_NEGATIVE || dam_type == DAM_DISEASE || dam_type == DAM_POISON)
      && is_affected(victim,gsn_death_shroud))
    {
      dam *= 2;
    }
    //Iblis 9/29/04 - Dwarves do more and take less damage underground
    if (ch->race == PC_RACE_DWARF && ch->in_room && ch->in_room->sector_type == SECT_UNDERGROUND)
    {
      dam += .2*dam;
    }
    if (victim->race == PC_RACE_DWARF && victim->in_room && victim->in_room->sector_type == SECT_UNDERGROUND)
    {
      dam -= .2*dam;
    }

    //Iblis 10/26/04 - Vuln Cold mobs/players take 10% more damage in snow/ice rooms
    if (IS_SET(ch->vuln_flags,VULN_COLD) && (IS_SET(victim->in_room->room_flags,ROOM_SNOW) 
      || IS_SET(victim->in_room->room_flags,ROOM_ICE)))
      dam += .1*dam;

    //Iblis 9/28/04 - Wind damage does 3x to flying people
    //     if (dam_type == DAM_WIND && IS_SET(victim->affected_by,AFF_FLYING))
    //       dam *= 3;

    //Iblis 9/28/04 - Chaos Jesters take 1/3 less damage from ACT_WARPED mobs
    if (IS_SET(ch->act2,ACT_WARPED) && !IS_NPC(victim) && victim->Class == PC_CLASS_CHAOS_JESTER)
      dam = (dam / 3.0) * 2;

    //Iblis 9/29/04 - Not act warped mobs/players get 2 times damage in warped rooms
    if (victim->in_room && IS_SET(victim->in_room->room_flags2,ROOM_WARPED) && !IS_SET(victim->act2,ACT_WARPED))
      dam *= 2;

    //Iblis 1/17/04 Litans get 2x damage if in water
    if (victim->race == PC_RACE_LITAN && victim->in_room &&
      (victim->in_room->sector_type == SECT_WATER_SWIM
      || victim->in_room->sector_type == SECT_WATER_NOSWIM
      || victim->in_room->sector_type == SECT_WATER_OCEAN
      || victim->in_room->sector_type == SECT_SWAMP
      || victim->in_room->sector_type == SECT_UNDERWATER))
      dam *= 2;
    if ((dt != gsn_exorcism) && (dt != gsn_hellscape) && !(ch->Class == PC_CLASS_REAVER && dt==1000+DAM_SLASH))
    {
      if (dam > 35)
        dam = (dam - 35) / 2 + 35;
      if (dam > 80)
        dam = (dam - 80) / 2 + 80;
    }
    if (ch->Class == PC_CLASS_REAVER && dt==1000+DAM_SLASH)
    {
      if (dam > 35)
        dam = (dam - 35) * .6 + 35;
      if (dam > 80)
        dam = (dam - 80) * .6 + 80;
    }
  }
  if (victim != ch)
  {

    /*
    * Certain attacks are forbidden.
    * Most other attacks are returned.
    */
    if (is_safe (ch, victim))
      return FALSE;
    if (!IS_NPC (ch) && !ch->fighting && dt != gsn_throwing && dt != gsn_assassinate && dt != gsn_archery && dt != gsn_trapdamage)
      if (ch->pcdata->battlecry[0] != '\0')
      {
        char tcbuf[MAX_STRING_LENGTH];
        sprintf (tcbuf, "$n %s", ch->pcdata->battlecry);
        act (tcbuf, ch, NULL, NULL, TO_ROOM);
      }
      if (victim->position > POS_STUNNED)
      {
        if (victim->fighting == NULL && dt != gsn_trapdamage)
        {
          if (dt == TYPE_AUTOASSIST || dam_type == DAM_NONE)
            set_fighting_auto (victim, ch);

          else
            set_fighting (victim, ch);
        }
      }
      if (victim->position > POS_STUNNED)
      {
        if (ch->fighting == NULL && dt != gsn_trapdamage)
        {
          if (dt == TYPE_AUTOASSIST || dam_type == DAM_NONE)
          {
            update_aggressor(ch,victim);
            set_fighting_auto (ch, victim);
          }
          else
          {
            update_aggressor(ch,victim);
            set_fighting (ch, victim);
          }
        }
      }

      /*
      * More charm stuff.
      */
      if (victim->master == ch)
        stop_follower (victim);
  }

  /*
  * Inviso attacks ... not.
  */
  if (IS_AFFECTED (ch, AFF_INVISIBLE) && dt != gsn_trapdamage)
  {
    affect_strip (ch, gsn_astral_body);
    affect_strip (ch, gsn_invis);
    affect_strip (ch, gsn_mass_invis);
    affect_strip (ch, gsn_heavenly_cloak);
    affect_strip (ch, gsn_vanish);
    REMOVE_BIT (ch->affected_by, AFF_INVISIBLE);
    act ("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
  }
  //5-10-03 Iblis - Setting it so if you are attacked, you become un-hidden and un-camoed
  if (dt != gsn_trapdamage)
  {
    if (IS_AFFECTED (ch, AFF_CAMOUFLAGE))
      REMOVE_BIT (ch->affected_by, AFF_CAMOUFLAGE);
    if (IS_AFFECTED (ch, AFF_HIDE))
      REMOVE_BIT (ch->affected_by, AFF_HIDE);
    if (IS_SET(ch->act2,ACT_TRANSFORMER))
      affect_strip(ch,gsn_transform);
  } 

  /*
  * Damage modifiers.
  */
  if (dt != gsn_soul_link)
  {
    if (dam > 1 && !IS_NPC (victim)
      && victim->pcdata->condition[COND_DRUNK] > 10)
      dam = 9 * dam / 10;
    if (dam > 1 && IS_AFFECTED (victim, AFF_SANCTUARY))
      dam /= 2;
    if (dam > 1 && is_affected (victim, skill_lookup("ghost form")))
      dam *= (2.0/3);
    if (dam > 1 && is_affected (ch, skill_lookup("ghost form")))
      dam /= 2;
    if (dam > 1 && ((IS_AFFECTED (victim, AFF_PROTECT_EVIL) && IS_EVIL (ch))
      || (IS_AFFECTED (victim, AFF_PROTECT_GOOD)
      && IS_GOOD (ch))))
      dam -= dam / 4;
    //Iblis 1/04/04 Death ward allows a person to take half damage from undeads
    if (IS_SET(ch->act,ACT_UNDEAD) && is_affected(victim,gsn_death_ward))
      dam /= 2;

	if ((victim->Class == PC_CLASS_DRUID) && is_affected(victim, skill_lookup("plant shell"))) {
		dam -= ((victim->level / 270) * dam);
	}
  }

  immune = FALSE;

  /*
  * Check for parry, and dodge.
  */
  if (dt >= TYPE_HIT && ch != victim
    && (IS_NPC (ch) || (!IS_NPC (ch) && !ch->pcdata->feigned))
    && dt != gsn_whirlwind && dt != gsn_trapdamage)
  {
    if (check_parry (ch, victim))
      return FALSE;
    if (!IS_NPC (victim) && check_block (ch, victim))
      return FALSE;
    if (check_dodge (ch, victim))
      return FALSE;
    if (check_flicker(ch,victim))
      return FALSE;
    if (check_eldritch_miasma (ch, victim))
      return FALSE;
    if (check_shield_block (ch, victim))
      return FALSE;
  }
  switch (check_immune (victim, dam_type))
  {
  case (IS_IMMUNE):
    immune = TRUE;
    dam = 0;
    break;
  case (IS_RESISTANT):
    dam -= dam / 3;
    break;
  case (IS_VULNERABLE):
    dam += dam / 2;
    break;
  }


  if (dt == TYPE_AUTOASSIST)
    dt = TYPE_HIT;
  if (is_affected (victim, gsn_mirror) && dam_type != DAM_ACID
    && dam_type != DAM_ENERGY && dam_type != DAM_HOLY
    && dam_type != DAM_LIGHTNING && dam_type != DAM_FIRE
    && dam_type != DAM_COLD && dam_type != DAM_NEGATIVE
    && dam_type != DAM_MENTAL && dam_type != DAM_LIGHT
    && dam_type != DAM_CHARM && dam_type != DAM_DISEASE
    && dam_type != DAM_SOUND && dam_type != DAM_HARM && dt != gsn_trapdamage)
  {
    if (ch->level >= (victim->level + 10) && number_percent () > 50)
    {
      char *attack;
      char buf1[MAX_STRING_LENGTH];
      char buf2[MAX_STRING_LENGTH];
      char buf3[MAX_STRING_LENGTH];
      if (dt >= 0 && dt < MAX_SKILL)
        attack = skill_table[dt].noun_damage;

      else if (dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
        attack = attack_table[dt - TYPE_HIT].noun;

      else
      {
        bug ("Dam_message: bad dt %d.", dt);
        dt = TYPE_HIT;
        attack = attack_table[0].name;
      }
      if (is_tail_attack)
        attack = "lightning tail";
      sprintf (buf1,
        "$n's %s is absorbed by the magic of $N's mirror.",
        attack);
      sprintf (buf2,
        "Your %s is absorbed by the magic of $N's mirror.",
        attack);
      sprintf (buf3,
        "$n's %s is absorbed by the magic of your mirror.",
        attack);
      act (buf1, ch, NULL, victim, TO_NOTVICT);
      act (buf2, ch, NULL, victim, TO_CHAR);
      act (buf3, ch, NULL, victim, TO_VICT);
      return FALSE;
    }
    mirror_attack = TRUE;
  }
  if (ch->race == PC_RACE_VROATH && ch != victim)
  {				/* Vroath? */
    if (ch->alignment >= 500)
      dam -= (dam * 2 / 3);

    else if (ch->alignment >= 0)
      dam -= dam / 3;
  }

  // Minax 7-6-02 Added info below for Critical Strike skill. (triple damage)
  // Iblis 5/12/03 - Made Critical Strike a little better if berserk/frenzied
  // Mike 3/9/08 - Made Critical Strike do half damage, but hit twice as much.
  if (dam != 0 && crit_strike_possible && victim->race != PC_RACE_SWARM && dt != gsn_soul_link
    && ((!IS_AFFECTED (ch, AFF_BERSERK) && number_percent () > 90)
    || ((!IS_NPC (ch)) && ch->pcdata->feigned)
    || (IS_AFFECTED (ch, AFF_BERSERK) && number_percent () > 85))
    && (get_skill (ch, gsn_critical_strike) > number_percent ()))
  {
    act ("`hYou land a `j*** CRITICAL STRIKE ***`h on $N!``", ch, NULL,
      victim, TO_CHAR);
    act ("`b$n lands a `i*** CRITICAL STRIKE ***`b on you!``", ch,
      NULL, victim, TO_VICT);
    act ("`h$n lands a `k*** CRITICAL STRIKE ***`h on $N!``", ch, NULL,
      victim, TO_NOTVICT);
    check_improve (victim, gsn_critical_strike, TRUE, 6);
    dam *= 1.5;
    if (!IS_NPC (ch))
      ch->pcdata->feigned = FALSE;
  }


  if (!IS_NPC(victim) && is_affected(victim,gsn_soul_link) && dt != gsn_soul_link && dam > 0)
  {
    if(victim->pcdata->soul_link == NULL)
    {
      bug("soul_link == NULL, but affect still present",0);
    }
    else
    {
      if (dam_type != DAM_HOLY)
        dam /= 2;
      damage2(victim->pcdata->soul_link,victim->pcdata->soul_link,dam,gsn_soul_link,DAM_UNIQUE,show);
    }
  }



  if (show)
  {
    if (!mirror_attack)
      dam_message (ch, victim, dam, dt, immune);

    else
      dam_message (ch, victim, dam, dt, 3);
  }
  if (dam == 0)
  {
    missed_attack = TRUE; //Iblis 10/11/04 - Hack to deal with windslash
    return FALSE;
  }

  /*
  * Hurt the victim   * Inform the victim of his new state.
  */
  if (mirror_attack)
    victim = ch;

  victim->hit -= dam;
  if (!IS_NPC (victim) && victim->level >= LEVEL_IMMORTAL && victim->hit < 1)
    victim->hit = 1;
  check_saddle (victim);
  update_pos (victim);
  switch (victim->position)
  {
  case POS_MORTAL:
    act ("$n is mortally wounded, and will die soon, if not aided.",
      victim, NULL, NULL, TO_ROOM);
    send_to_char
      ("`CYou are mortally wounded, and will die soon, if not aided.``\n\r",
      victim);
    break;
  case POS_INCAP:
    act ("$n is incapacitated and will slowly die, if not aided.",
      victim, NULL, NULL, TO_ROOM);
    send_to_char
      ("`CYou are incapacitated and will slowly die, if not aided.``\n\r",
      victim);
    break;
  case POS_STUNNED:
    act ("$n is stunned, but will probably recover.", victim, NULL,
      NULL, TO_ROOM);
    send_to_char
      ("`CYou are stunned, but will probably recover.``\n\r", victim);
    break;
  case POS_DEAD:
    victim->position = POS_STANDING;
    if (HAS_SCRIPT (victim))
    {
      TRIGGER_DATA *pTrig;
      for (pTrig = victim->triggers; pTrig != NULL; pTrig = pTrig->next)
        if (pTrig->trigger_type == TRIG_DIES)
        {
          act_trigger (victim, pTrig->name, NULL, NAME (ch),
            NAME (victim));
          pTrig->current = pTrig->script;
          while (pTrig->current != NULL)
          {
            pTrig->bits = SCRIPT_ADVANCE;
            script_interpret (victim, pTrig);
            if (pTrig->current != NULL)
              pTrig->current = pTrig->current->next;

            else
              pTrig->tracer = 0;
          }
        }
    }
    victim->position = POS_DEAD;
    if (!(IS_SET(victim->act,PLR_WANTED) && IS_NPC (ch) && ch->spec_fun != 0 
      && (!str_cmp("spec_executioner",spec_name (ch->spec_fun)) 
      || !str_cmp("spec_guard",spec_name (ch->spec_fun)))))
    {
      act ("$n is DEAD!!", victim, 0, 0, TO_ROOM);
      send_to_char ("`CAlas, you have been KILLED!!``\n\r\n\r", victim);
    }
    if (dt == gsn_pox || dt == gsn_plague || dt == gsn_poison || dt==gsn_aura_rot || dt == gsn_brain_blisters
      || dt == gsn_vomit || dt == gsn_contaminate || dt == gsn_skeletal_mutation )
    {
      AFFECT_DATA *af;//, plague;
      for (af = victim->affected; af != NULL; af = af->next)
      {
        if (af->type == dt)
          break;
      }
      if (victim == ch && af != NULL && af->comp_name != NULL)
      {
        if  (str_cmp(af->comp_name,""))
        {
          if ((ch = get_exact_pc_world(ch,af->comp_name)) == NULL)
          {
            ch = victim;
          }
        }
      }
      victim->position = POS_DEAD;
    }
    // Swap the mob and the character who set the trap temporily to properly inact justice	       

    if (IS_NPC(ch) && IS_SET(ch->act2, ACT_TRAPMOB))
    {
      if ((tch = get_exact_pc_world(tch,ch->name)) != NULL)
      {
        anothertch=ch;
        ch=tch;
      }
    }
    if ((ch->clan != CLAN_BOGUS) && (victim->clan != CLAN_BOGUS) && !battle_royale && !ch->dueler)
    {
      clanwar_kill (ch, victim);
    }
    if (victim != ch)
      if (!IS_NPC (ch) && !IS_NPC (victim) && (ch->aggres == NULL)
        && IS_AGGRESSOR (ch, victim))
      {
        if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
          && !IS_SET (victim->act, PLR_WANTED)
          && !(victim->race == PC_RACE_LICH)
          && !(victim->Class == PC_CLASS_CHAOS_JESTER)
          && !is_warring (ch->clan, victim->clan)
          && !is_dueling (ch) && !battle_royale && !ch->dueler && ch->clan != clanname_to_slot("guttersnipes"))
          set_wanted (ch);
        if (!ch->pcdata->loner &&
          (ch->clan == CLAN_BOGUS) && !is_dueling (ch)
          && !battle_royale && !ch->dueler)
        {
          ch->pcdata->loner = TRUE;
          send_to_char
            ("For your crimes against humanity, you are now marked a loner.\n\r",
            ch);
        }
      }
      //IBLIS 6/20/03 - Calculate Battle Royale points
      if (!IS_NPC (ch) && !IS_NPC (victim))
        if (battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner)))
          calculate_br_points (ch, victim);
      if (!IS_NPC (ch) && !IS_NPC (victim) &&
        (ch->clan != CLAN_BOGUS) &&
        !battle_royale &&
        !ch->dueler && !victim->dueler &&
        (ch->clan == victim->clan) && is_clan_leader (victim))
        clan_leader (victim, ch->name);
      if (!IS_NPC (ch))
      {
        if (IS_NPC (victim))
          ch->pcdata->has_killed[MOB_KILL]++;
        else if (ch != victim)
        {
          if (battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner)))
            ch->pcdata->has_killed[BR_KILL]++;
          else if (ch->dueler)
            ch->pcdata->has_killed[SD_KILL]++;
          else ch->pcdata->has_killed[PLAYER_KILL]++;
        }
      }
      if (!IS_NPC (victim))
      {
        if (IS_NPC (ch))
          victim->pcdata->been_killed[MOB_KILL]++;
        else if (ch != victim)
        {
          if (battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner)))
            victim->pcdata->been_killed[BR_KILL]++;
          else if (victim->dueler)
            victim->pcdata->been_killed[SD_KILL]++;
          else victim->pcdata->been_killed[PLAYER_KILL]++;
        }
      }

      /* BEGIN ARENA */
      if (is_dueling (victim))
      {
        duel_ends (victim);
        return FALSE;
      }

      if (victim->dueler)
      { 
        superduel_ends (ch,victim);
        return FALSE;
      }

      /* END ARENA */
      break;
  default:
    if (dam > victim->max_hit / 4)
      send_to_char
      ("`CYou reel in shock from the tremendous pain!``\n\r", victim);
    if (victim->hit < victim->max_hit / 4)
      send_to_char ("`CYou sure are BLEEDING!``\n\r", victim);
    break;
  }

  /*
  * Sleep spells and extremely wounded folks.
  */
  if (!IS_AWAKE (victim))
    stop_fighting (victim, FALSE);

  /*
  * Payoff for killing things.
  */
  if (victim->position == POS_DEAD)
  {
    if (!is_dueling (victim) && !victim->dueler)
      //Iblis 6/29/03
      //Needed to fix Avatar Reavers who die from > 0 align
      if (group_gain (ch, victim))
      {
        raw_kill (victim, victim);
        return TRUE;
      }
      if (!IS_NPC (victim)
        && !((IS_SET(victim->act,PLR_WANTED) && IS_NPC (ch) && ch->spec_fun != 0
        && (!str_cmp("spec_executioner",spec_name (ch->spec_fun))
        || !str_cmp("spec_guard",spec_name (ch->spec_fun))))))
      {
        sprintf (log_buf, "%s killed by %s at %d", victim->name,
          (IS_NPC (ch) ? ch->short_descr : ch->name),
          ch->in_room->vnum);
        log_string (log_buf);
        if (!is_dueling (victim) && !(battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner))) && !victim->dueler)
        {

          //You only lose max 1/10 xp needed for next level
          //
          if (ch->level > 1)
          {
            long xploss;
            if (IS_NPC (ch))
              xploss = (victim->exp / 2.0);

            else
              xploss = (victim->exp / 4.0);
            if (victim->level > 60)
            {
              if (xploss > (level_cost (victim->level) / 2.0))
                xploss = (level_cost (victim->level) / 2.0);
            }
            else
            {
              if (xploss > (level_cost (victim->level) / 5.0))
                xploss = (level_cost (victim->level) / 5.0);
            }
            if (xploss > (level_cost (victim->level) / 5.0))
              xploss = (level_cost (victim->level) / 5.0);
            if (victim->race != PC_RACE_NERIX)
            {
              victim->pcdata->xp_last_death = xploss;
              victim->pcdata->last_death_counter = victim->perm_stat[STAT_CON];
            }
            if (victim->race != PC_RACE_NERIX)
              gain_exp (victim, 0 - xploss);
            else gain_exp (victim, 0 - (xploss/2));
          }

          else
            send_to_char
            ("Since you are level 1, you lose no experience.\n\r", ch);
        }
      }

      /* Make the head for a bountied PC */
      if (!IS_NPC (victim) && !IS_NPC (ch) && is_bountied (victim)
        && !battle_royale)
      {
        char buf[MAX_STRING_LENGTH];
        char *name;
        OBJ_DATA *obj;
        name = str_dup (victim->name);
        obj = create_object (get_obj_index (OBJ_VNUM_SEVERED_HEAD), 0);
        obj->extra_flags[0] |= ITEM_NODROP | ITEM_NOUNCURSE;
        sprintf (buf, "%s %s", "head", name);
        free_string (obj->name);
        obj->name = str_dup (buf);
        sprintf (buf, obj->short_descr, name);
        free_string (obj->short_descr);
        obj->short_descr = str_dup (buf);
        sprintf (buf, obj->description, name);
        free_string (obj->description);
        obj->description = str_dup (buf);
        obj_to_char (obj, ch);
        free_string (name);
      }
      raw_kill (victim, ch);
      if (anothertch != NULL)
      {
        ch=anothertch;
        anothertch= NULL;
      }

      if (IS_NPC(ch) && IS_SET (ch->act, ACT_LOOTER))
      {
        do_get (ch, "all corpse");
        do_get (ch, "all 2.corpse");
        do_wear (ch, "all");
      }
      if (ch != victim && !IS_NPC (ch) && !is_same_clan (ch, victim)
        && victim->times_wanted < 65)
        remove_wanted (victim);
      if (!IS_NPC (ch) && IS_NPC (victim))
      {
        OBJ_DATA *coins;
        corpse = get_obj_list (ch, "corpse", ch->in_room->contents);
        if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse && corpse->contains)	/* exists and not empty */
          do_get (ch, "all corpse");
        if (IS_SET (ch->act, PLR_AUTOGOLD) && corpse && corpse->contains &&	/* exists and not empty */
          !IS_SET (ch->act, PLR_AUTOLOOT))
          if ((coins =
            get_obj_list (ch, "gcash", corpse->contains)) != NULL)
            do_get (ch, "all.gcash corpse");
        if (get_skill(ch,gsn_butcher) > 0
          && (IS_SET (ch->act, PLR_AUTOBUTCHER)))
          do_butcher (ch, "corpse");
        if (IS_SET (ch->act, PLR_AUTOSAC))
        {
          if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse
            && corpse->contains)
            return TRUE;	/* leave if corpse has treasure */

          else
            do_sacrifice (ch, "corpse");
        }
      }
      return TRUE;
  }
  if (victim == ch)
    return TRUE;

  /*
  * Take care of link dead people.
  */
  if (!IS_NPC (victim) && victim->desc == NULL)
  {
    if (number_range (0, victim->wait) == 0)
    {
      do_recall (victim, "");
      return TRUE;
    }
  }

  if (anothertch != NULL)
  {
    ch=anothertch;
    anothertch= NULL;
  }
  /*
  * Wimp out?
  */
  if (IS_NPC (victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
  {
    if ((IS_SET (victim->act, ACT_WIMPY) && number_bits (2) == 0
      && victim->hit < victim->max_hit / 5)
      || (IS_AFFECTED (victim, AFF_CHARM) && victim->master != NULL
      && victim->master->in_room != victim->in_room))
    {
      if (victim->position > POS_RESTING)
      {
        victim->wimpy = number_range (0, 4);
        do_flee (victim, "");
      }
    }
  }
  if (!IS_NPC (victim)
    && victim->hit > 0
    && victim->hit <= victim->wimpy && victim->wait < PULSE_VIOLENCE / 2)
  {
    if (victim->position <= POS_RESTING)
    {
      send_to_char ("You try to flee, but you aren't prepared.\n\r",
        victim);
    }

    else
    {
      do_flee (victim, "");
    }
  }
  return TRUE;
}


//Iblis 1/09/04 - Intercept function for the soul link spell
bool damage_old (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type, bool show)
{
  if (victim->race == PC_RACE_SWARM)
  {
    if (victim->level > ch->level+50)
      dam = dice(1,2);
    else dam = dice(5,3);
    if (dam_type == DAM_FIRE)
    {
      dam *= 2;
      dam_type = DAM_UNIQUE;  //So that vuln fire won't come into affect
    }

  }
  return damage_old2(ch,victim,dam,dt,dam_type,show);
}




/*
* Inflict damage from a hit.
*/
bool
damage_old2 (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
             int dam_type, bool show)
{
  bool mirror_attack = FALSE;
  OBJ_DATA *corpse;
  bool immune;
  long xploss;
  if (IS_NPC(ch) && IS_SET(ch->act2,ACT_TRAPMOB))
    return	damage(ch,victim,dam,dt,dam_type,show);
  if (victim->position == POS_DEAD)
    return FALSE;
  if (ch != victim && ch != NULL && victim != NULL)
  {
    if (!IS_NPC (ch))
      ch->pcdata->last_fight = time (NULL);
    if (!IS_NPC (victim))
      victim->pcdata->last_fight = time (NULL);
  }
  if (dam > 1200 && dt >= TYPE_HIT)
  {

    dam = 1200;
    if (!IS_IMMORTAL (ch))
    {
      OBJ_DATA *obj;
      obj = get_eq_char (ch, WEAR_WIELD_R);
      if (obj != NULL)
        extract_obj (obj);
      obj = get_eq_char (ch, WEAR_WIELD_L);
      if (obj != NULL)
        extract_obj (obj);
      send_to_char ("You really shouldn't cheat.\n\r", ch);
    }
  }
  if (dt != gsn_soul_link)
  {
    //Iblis 1/13/04 Paladins do 2x damage to all undead
    if (ch->Class == 6 && IS_SET(victim->act,ACT_UNDEAD))
      dam *= 2;
    if ((dam_type == DAM_NEGATIVE || dam_type == DAM_DISEASE || dam_type == DAM_POISON)
      && is_affected(victim,gsn_death_shroud))
    {
      dam *= 2;
    }

    //Iblis 9/29/04 - Dwarves do more and take less damage underground
    if (ch->race == PC_RACE_DWARF && ch->in_room && ch->in_room->sector_type == SECT_UNDERGROUND)
    {
      dam += .2*dam;
    }
    if (victim->race == PC_RACE_DWARF && victim->in_room && victim->in_room->sector_type == SECT_UNDERGROUND)
    {
      dam -= .2*dam;
    }

    //Iblis 10/26/04 - Vuln Cold mobs/players take 10% more damage in snow/ice rooms
    if (IS_SET(ch->vuln_flags,VULN_COLD) && (IS_SET(victim->in_room->room_flags,ROOM_SNOW)
      || IS_SET(victim->in_room->room_flags,ROOM_ICE)))
      dam += .1*dam;

    //Iblis 9/28/04 - Wind damage does 3x to flying people
    //    if (dam_type == DAM_WIND && IS_SET(victim->affected_by,AFF_FLYING))
    //     dam *= 3;

    //Iblis 9/28/04 - Chaos Jesters take 1/3 less damage from ACT_WARPED mobs
    if (IS_SET(ch->act2,ACT_WARPED) && !IS_NPC(victim) && victim->Class == PC_CLASS_CHAOS_JESTER)
      dam = (dam / 3.0) * 2;

    //Iblis 9/29/04 - Not act warped mobs/players get 2 times damage in warped rooms
    if (victim->in_room && IS_SET(victim->in_room->room_flags2,ROOM_WARPED) && !IS_SET(victim->act2,ACT_WARPED))
      dam *= 2;

    //Iblis 1/17/04 Litan's get dealts 2x damage when in water
    if (victim->race == PC_RACE_LITAN && victim->in_room &&
      (victim->in_room->sector_type == SECT_WATER_SWIM
      || victim->in_room->sector_type == SECT_WATER_NOSWIM
      || victim->in_room->sector_type == SECT_WATER_OCEAN
      || victim->in_room->sector_type == SECT_SWAMP
      || victim->in_room->sector_type == SECT_UNDERWATER))
      dam *= 2;

    // damage reduction
    if (dam > 35)
      dam = (dam - 35) / 2 + 35;
    if (dam > 80)
      dam = (dam - 80) / 2 + 80;
  }
  if (victim != ch)
  {

    // Certain attacks are forbidden.
    // Most other attacks are returned.
    if (is_safe (ch, victim))
      return FALSE;
    if (!IS_NPC (ch) && !ch->fighting)
      if (ch->pcdata->battlecry[0] != '\0')
      {
        char buf[MAX_STRING_LENGTH];
        sprintf (buf, "$n %s", ch->pcdata->battlecry);
        act (buf, ch, NULL, NULL, TO_ROOM);
      }
      if (victim->position > POS_STUNNED)
      {
        /* Iblis 08/28/03 - Aggessor fix */	
        update_aggressor(ch,victim);
        if (victim->fighting == NULL)
          set_fighting (victim, ch);
        if (ch->fighting == NULL)
          set_fighting (ch, victim);

        // If victim is charmed, ch might attack victim's master.
        if (IS_NPC (ch)
          && IS_NPC (victim)
          && IS_AFFECTED (victim, AFF_CHARM)
          && victim->master != NULL
          && victim->master->in_room == ch->in_room
          && number_bits (3) == 0)
        {
          stop_fighting (ch, FALSE);
          multi_hit (ch, victim->master, TYPE_UNDEFINED);
          return FALSE;
        }
      }			// end if(victim->position > POS_STUNNED)

      // if you go after your own charmie, then they are released
      if (victim->master == ch)
        stop_follower (victim);
  }

  /*
  * Inviso attacks ... not.
  */
  if (IS_AFFECTED (ch, AFF_INVISIBLE))
  {
    affect_strip (ch, gsn_astral_body);
    affect_strip (ch, gsn_invis);
    affect_strip (ch, gsn_mass_invis);
    affect_strip (ch, gsn_heavenly_cloak);
    affect_strip (ch, gsn_vanish);
    REMOVE_BIT (ch->affected_by, AFF_INVISIBLE);
    act ("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
  }
  //5-10-03 Iblis - Setting it so if you are attacked, you become un-hidden and un-camoed
  if (IS_AFFECTED (ch, AFF_CAMOUFLAGE))
    REMOVE_BIT (ch->affected_by, AFF_CAMOUFLAGE);
  if (IS_AFFECTED (ch, AFF_HIDE))
    REMOVE_BIT (ch->affected_by, AFF_HIDE);
  if (IS_SET(ch->act2,ACT_TRANSFORMER))
    affect_strip(ch,gsn_transform);


  /*
  * Damage modifiers.
  */
  if (dt != gsn_soul_link)
  {
    if (dam > 1 && !IS_NPC (victim)
      && victim->pcdata->condition[COND_DRUNK] > 10)
      dam = 9 * dam / 10;
    if (dam > 1 && IS_AFFECTED (victim, AFF_SANCTUARY))
      dam /= 2;
    if (dam > 1 && is_affected (victim, skill_lookup("ghost form")))
      dam *= (2.0/3);
    if (dam > 1 && is_affected (ch, skill_lookup("ghost form")))
      dam /= 2;
    if (dam > 1 && ((IS_AFFECTED (victim, AFF_PROTECT_EVIL) && IS_EVIL (ch))
      || (IS_AFFECTED (victim, AFF_PROTECT_GOOD)
      && IS_GOOD (ch))))
      dam -= dam / 4;
    //Iblis 1/04/04 Death ward allows a person to take half damage from undeads
    if (IS_SET(ch->act,ACT_UNDEAD) && is_affected(victim,gsn_death_ward))
      dam /= 2;
  }
  immune = FALSE;

  /*
  * Check for parry, and dodge.
  */
  if (dt >= TYPE_HIT && ch != victim)
  {
    if (check_parry (ch, victim))
      return FALSE;
    if (!IS_NPC (victim) && check_block (ch, victim))
      return FALSE;
    if (check_dodge (ch, victim))
      return FALSE;
    if (check_flicker(ch,victim))
      return FALSE;
    if (check_eldritch_miasma (ch, victim))
      return FALSE;      
    if (check_shield_block (ch, victim))
      return FALSE;
  }
  switch (check_immune (victim, dam_type))
  {
  case (IS_IMMUNE):
    immune = TRUE;
    dam = 0;
    break;
  case (IS_RESISTANT):
    dam -= dam / 3;
    break;
  case (IS_VULNERABLE):
    dam += dam / 2;
    break;
  }


  //IBLIS 5/29/03 - I could NOT find, and I tried, any reference to the vulnerabily checking for wood,iron,and silver
  //in this fight.c mentioned in handler.c, so I added my own checks.
  //Oh geez this will be complicated..I will add it later 
  if (is_affected (victim, gsn_mirror) && dam_type != DAM_ACID
    && dam_type != DAM_ENERGY && dam_type != DAM_HOLY
    && dam_type != DAM_LIGHTNING && dam_type != DAM_FIRE
    && dam_type != DAM_COLD && dam_type != DAM_NEGATIVE
    && dam_type != DAM_MENTAL && dam_type != DAM_LIGHT
    && dam_type != DAM_CHARM && dam_type != DAM_DISEASE
    && dam_type != DAM_SOUND && dam_type != DAM_HARM)
  {
    if (ch->level >= (victim->level + 10) && number_percent () > 50)
    {
      char *attack;
      char buf1[MAX_STRING_LENGTH];
      char buf2[MAX_STRING_LENGTH];
      char buf3[MAX_STRING_LENGTH];
      if (dt >= 0 && dt < MAX_SKILL)
        attack = skill_table[dt].noun_damage;

      else if (dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
        attack = attack_table[dt - TYPE_HIT].noun;

      else
      {
        bug ("Dam_message: bad dt %d.", dt);
        dt = TYPE_HIT;
        attack = attack_table[0].name;
      }
      if (is_tail_attack)
        attack = "lightning tail";
      sprintf (buf1,
        "$n's %s is absorbed by the magic of $N's mirror.",
        attack);
      sprintf (buf2,
        "Your %s is absorbed by the magic of $N's mirror.",
        attack);
      sprintf (buf3,
        "$n's %s is absorbed by the magic of your mirror.",
        attack);
      act (buf1, ch, NULL, victim, TO_NOTVICT);
      act (buf2, ch, NULL, victim, TO_CHAR);
      act (buf3, ch, NULL, victim, TO_VICT);
      return FALSE;
    }
    mirror_attack = TRUE;
  }
  if (ch->race == PC_RACE_VROATH)
  {				/* Vroath? */
    if (ch->alignment >= 500)
      dam -= (dam * 2 / 3);

    else if (ch->alignment >= 0)
      dam -= dam / 3;
  }

  if (!IS_NPC(victim) && is_affected(victim,gsn_soul_link) && dt != gsn_soul_link &&dam > 0)
  {
    if(victim->pcdata->soul_link == NULL)
    {
      bug("soul_link == NULL, but affect still present",0);
    }
    else
    {
      if (dam_type != DAM_HOLY)
        dam /= 2;
      damage_old2(victim->pcdata->soul_link,victim->pcdata->soul_link,dam,gsn_soul_link,DAM_UNIQUE,show);
    }
  }  


  if (show)
  {
    if (!mirror_attack)
      dam_message (ch, victim, dam, dt, immune);

    else
      dam_message (ch, victim, dam, dt, 3);
  }
  if (dam == 0)
    return FALSE;

  /*
  * Hurt the victim.
  * Inform the victim of his new state.
  */
  if (mirror_attack)
    victim = ch;
  victim->hit -= dam;
  if (!IS_NPC (victim) && victim->level >= LEVEL_IMMORTAL && victim->hit < 1)
    victim->hit = 1;
  check_saddle (victim);
  update_pos (victim);
  switch (victim->position)
  {
  case POS_MORTAL:
    act ("$n is mortally wounded, and will die soon, if not aided.",
      victim, NULL, NULL, TO_ROOM);
    send_to_char
      ("You are mortally wounded, and will die soon, if not aided.\n\r",
      victim);
    break;
  case POS_INCAP:
    act ("$n is incapacitated and will slowly die, if not aided.",
      victim, NULL, NULL, TO_ROOM);
    send_to_char
      ("You are incapacitated and will slowly die, if not aided.\n\r",
      victim);
    break;
  case POS_STUNNED:
    act ("$n is stunned, but will probably recover.", victim, NULL,
      NULL, TO_ROOM);
    send_to_char ("You are stunned, but will probably recover.\n\r",
      victim);
    break;
  case POS_DEAD:
    victim->position = POS_STANDING;
    if (HAS_SCRIPT (victim))
    {
      TRIGGER_DATA *pTrig;
      for (pTrig = victim->triggers; pTrig != NULL; pTrig = pTrig->next)
        if (pTrig->trigger_type == TRIG_DIES)
        {
          act_trigger (victim, pTrig->name, NULL, NAME (ch),
            NAME (victim));
          pTrig->current = pTrig->script;
          while (pTrig->current != NULL)
          {
            pTrig->bits = SCRIPT_ADVANCE;
            script_interpret (victim, pTrig);
            if (pTrig->current != NULL)
              pTrig->current = pTrig->current->next;

            else
              pTrig->tracer = 0;
          }
        }
    }

    victim->position = POS_DEAD;
    if (victim == ch && dt == skill_lookup("chain lightning"))
    {
      killed_by_chain = TRUE;
    }
    if (!(IS_SET(victim->act,PLR_WANTED) && IS_NPC (ch) && ch->spec_fun != 0
      && (!str_cmp("spec_executioner",spec_name (ch->spec_fun))
      || !str_cmp("spec_guard",spec_name (ch->spec_fun)))))
    {

      act ("$n is DEAD!!", victim, 0, 0, TO_ROOM);
      if (dt == skill_lookup("seizure"))
      {
        char buf[MAX_STRING_LENGTH];
        if (IS_NPC(ch))
          sprintf(buf,"`kThe laughing face of %s is the last thing you see as death takes you.``\n\r",ch->short_descr);
        else sprintf(buf,"`kThe laughing face of %s is the last thing you see as death takes you.``\n\r",ch->name);
        send_to_char(buf,victim);
      }
      send_to_char ("You have been KILLED!!\n\r\n\r", victim);
    }
    if (victim != ch)
      if (!IS_NPC (ch) && !IS_NPC (victim) && ch->aggres == NULL
        && IS_AGGRESSOR (ch, victim))
      {
        if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
          && !IS_SET (victim->act, PLR_WANTED)
          && !(victim->race == PC_RACE_LICH)
          && !(victim->Class == PC_CLASS_CHAOS_JESTER)
          && !is_warring (ch->clan, victim->clan)
          && !is_dueling (ch) && !battle_royale && !ch->dueler && ch->clan != clanname_to_slot("guttersnipes"))
          set_wanted (ch);
        if (!ch->pcdata->loner &&
          (ch->clan == CLAN_BOGUS) && !is_dueling (ch)
          && !battle_royale && !ch->dueler)
        {
          ch->pcdata->loner = TRUE;
          send_to_char
            ("For your crimes against humanity, you are now marked a loner.\n\r",
            ch);
        }
      }
      if (dt == gsn_pox || dt == gsn_plague || dt == gsn_poison || dt == gsn_aura_rot || dt == gsn_brain_blisters
        || dt == gsn_vomit || dt == gsn_contaminate || dt == gsn_skeletal_mutation)
      {
        AFFECT_DATA *af;//, plague;
        for (af = victim->affected; af != NULL; af = af->next)
        {
          if (af->type == dt)
            break;
        }
        if (victim == ch && af != NULL && af->comp_name != NULL)
        {
          if  (str_cmp(af->comp_name,""))
          {
            if ((ch = get_exact_pc_world(ch,af->comp_name)) == NULL)
            {
              ch = victim;
            }
          }

        }
        victim->position = POS_DEAD;
      }
      //IBLIS 6/20/03 - Calculate Battle Royale points
      if (!IS_NPC (ch) && !IS_NPC (victim))
        if (battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner)))
          calculate_br_points (ch, victim);
      if (!IS_NPC (ch) && !IS_NPC (victim) &&
        (ch->clan != CLAN_BOGUS) &&
        !battle_royale && 
        (ch->clan == victim->clan) && is_clan_leader (victim)
        && victim != ch)
        clan_leader (victim, ch->name);
      if ((ch->clan != CLAN_BOGUS) && (victim->clan != CLAN_BOGUS) && !battle_royale && !ch->dueler)
        clanwar_kill (ch, victim);
      if (!IS_NPC (ch))
      {
        if (IS_NPC (victim))
          ch->pcdata->has_killed[MOB_KILL]++;

        else if (ch != victim)
        {
          if (battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner)))
            ch->pcdata->has_killed[BR_KILL]++;
          else if (ch->dueler)
            ch->pcdata->has_killed[SD_KILL]++;
          else ch->pcdata->has_killed[PLAYER_KILL]++;
        }
      }
      if (!IS_NPC (victim))
      {
        if (IS_NPC (ch))
          victim->pcdata->been_killed[MOB_KILL]++;

        else if (ch != victim)
        {
          if (battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner)))
            victim->pcdata->been_killed[BR_KILL]++;
          else if (victim->dueler)
            victim->pcdata->been_killed[SD_KILL]++;
          else victim->pcdata->been_killed[PLAYER_KILL]++;
        }
      }

      /* BEGIN ARENA */
      if (is_dueling (victim))
      {
        duel_ends (victim);
        return FALSE;
      }

      if (victim->dueler)
      {
        superduel_ends(ch,victim);
        return FALSE;
      }

      /* END ARENA */
      break;
  default:
    if (dam > victim->max_hit / 4)
      send_to_char
      ("You reel in shock from the tremendous pain!\n\r", victim);
    if (victim->hit < victim->max_hit / 4)
      send_to_char ("You sure are BLEEDING!\n\r", victim);
    break;
  }

  /*
  * Sleep spells and extremely wounded folks.
  */
  if (!IS_AWAKE (victim))
    stop_fighting (victim, FALSE);
  if (victim->position == POS_DEAD)
  {
    if (!is_dueling (victim) && !victim->dueler)
      //Iblis 6/29/03
      //Needed to fix Avatar Reavers who die from > 0 align
      if (group_gain (ch, victim))
      {
        raw_kill (victim, victim);
        return TRUE;
      }

      if (!IS_NPC (victim) 
        && !((IS_SET(victim->act,PLR_WANTED) && IS_NPC (ch) && ch->spec_fun != 0
        && (!str_cmp("spec_executioner",spec_name (ch->spec_fun))
        || !str_cmp("spec_guard",spec_name (ch->spec_fun))))))
      {
        sprintf (log_buf, "%s killed by %s at %d", victim->name,
          (IS_NPC (ch) ? ch->short_descr : ch->name),
          ch->in_room->vnum);
        log_string (log_buf);
        if (!is_dueling (victim) && !(battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner))) && !victim->dueler)
        {

          if (ch->level > 1)
          {
            if (IS_NPC (ch))
              xploss = (victim->exp / 2.0);

            else
              xploss = (victim->exp / 4.0);
            if (victim->level > 60)
            {
              if (xploss > (level_cost (victim->level) / 2.0))
                xploss = (level_cost (victim->level) / 2.0);
            }
            else 
            {
              if (xploss > (level_cost (victim->level) / 5.0))
                xploss = (level_cost (victim->level) / 5.0);
            }
            if (victim->race != PC_RACE_NERIX)
            {
              victim->pcdata->xp_last_death = xploss;
              victim->pcdata->last_death_counter = victim->perm_stat[STAT_CON];
            }
            if (victim->race != PC_RACE_NERIX)
              gain_exp (victim, 0 - xploss);
            else gain_exp (victim, 0 - (xploss/2));
          }

          else
            send_to_char
            ("Since you are level 1, you lose no experience.\n\r", ch);
        }
      }
      raw_kill (victim, ch);
      if (IS_NPC(ch) && IS_SET (ch->act, ACT_LOOTER))
      {
        do_get (ch, "all corpse");
        do_get (ch, "all 2.corpse");
        do_wear (ch, "all");
      }
      if (ch != victim && !IS_NPC (ch) && !is_same_clan (ch, victim)
        && victim->times_wanted < 65)
        remove_wanted (victim);
      if (!IS_NPC (ch) && IS_NPC (victim))
      {
        OBJ_DATA *coins;
        corpse = get_obj_list (ch, "corpse", ch->in_room->contents);
        if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse && corpse->contains)	/* exists and not empty */
          do_get (ch, "all corpse");
        if (IS_SET (ch->act, PLR_AUTOGOLD) && corpse && corpse->contains &&	/* exists and not empty */
          !IS_SET (ch->act, PLR_AUTOLOOT))
          if ((coins =
            get_obj_list (ch, "gcash", corpse->contains)) != NULL)
            do_get (ch, "all.gcash corpse");
        if (get_skill(ch,gsn_butcher) > 0
          && (IS_SET (ch->act, PLR_AUTOBUTCHER)))
          do_butcher (ch, "corpse");
        if (IS_SET (ch->act, PLR_AUTOSAC))
        {
          if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse
            && corpse->contains)
            return TRUE;	/* leave if corpse has treasure */

          else
            do_sacrifice (ch, "corpse");
        }
      }
      return TRUE;
  }
  if (victim == ch)
    return TRUE;

  /*
  * Take care of link dead people.
  */
  if (!IS_NPC (victim) && victim->desc == NULL)
  {
    if (number_range (0, victim->wait) == 0)
    {
      do_recall (victim, "");
      return TRUE;
    }
  }

  /*
  * Wimp out?
  */
  if (IS_NPC (victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
  {
    if ((IS_SET (victim->act, ACT_WIMPY) && number_bits (2) == 0
      && victim->hit < victim->max_hit / 5)
      || (IS_AFFECTED (victim, AFF_CHARM) && victim->master != NULL
      && victim->master->in_room != victim->in_room))
    {
      victim->wimpy = number_range (0, 4);
      do_flee (victim, "");
    }
  }
  if (!IS_NPC (victim)
    && victim->hit > 0
    && victim->hit <= victim->wimpy && victim->wait < PULSE_VIOLENCE / 2)
  {
    do_flee (victim, "");
  }
  return TRUE;
}

bool check_nonpk (CHAR_DATA * ch, CHAR_DATA * victim)
{
  char buf[MAX_STRING_LENGTH];

  // Here, checks for PC vs. PC fights are added for nopk
  // protection, disregard if it's a duel
  if (ch == victim)
    return FALSE;
  if (!IS_NPC (ch) && !IS_NPC (victim))
  {
    if (IS_IMMORTAL (ch))
    {
      if (is_warring (ch->clan, victim->clan))
        return FALSE;
      if (is_dueling (ch) && is_dueling (victim))
        return FALSE;
      if (ch->dueler && victim->dueler)
        return FALSE;
      return FALSE;
      //send_to_char
      //("Immortals can't go around attacking players.\n\r", ch);
      //	  return FALSE;
    }
    if (IS_IMMORTAL (victim))
    {
      if (is_warring (ch->clan, victim->clan))
        return FALSE;
      if (is_dueling (ch) && is_dueling (victim))
        return FALSE;
      if (ch->dueler && victim->dueler)
        return FALSE;
      //	  if (can_see (ch, victim))
      //	    send_to_char ("You want to attack an immortal ?!?!\n\r", ch);
      //	  return FALSE;
    }
    if (ch->level < 9)
    {
      send_to_char
        ("You are too young to worry about playerkilling.\n\r", ch);
      return TRUE;
    }
    if (victim->level < 9)
    {
      send_to_char
        ("You really don't want to attack this character.\n\r", ch);
      return TRUE;
    }
    if (is_dueling (ch) && is_dueling (victim))
      return FALSE;
    if (ch->dueler && victim->dueler)
      return FALSE;
    if (victim->pcdata->questing)
    {
      sprintf(buf, "%s is protected by the God of Questing.\n\r",victim->name);
      send_to_char (buf,ch);
      return TRUE;
    }
    if (ch->pcdata->questing)
    {
      send_to_char("The God of Questing prohibits you from doing that.\n\r",ch);
      return TRUE;
    }


    if (clan_table[ch->clan].ctype == CLAN_TYPE_PC)
    {
      if (clan_table[victim->clan].ctype == CLAN_TYPE_EXPL)
      {
        sprintf (buf, "%s is protected by the God of Peace.\n\r",
          victim->name);
        send_to_char (buf, ch);
        return TRUE;
      }
      //	  if (clan_table[victim->clan].ctype == CLAN_TYPE_PC)
      //	    return FALSE;
      if (!victim->pcdata->loner && !(clan_table[victim->clan].ctype == CLAN_TYPE_PC))
      {
        sprintf (buf, "%s is protected by the God of Peace.\n\r",
          victim->name);
        send_to_char (buf, ch);
        return TRUE;
      }
      if (current_time - victim->pcdata->last_pkdeath < (60*MULTIKILL_WAIT_TIME) )
      {
        sprintf (buf, "%s is protected by a divine light, you dare not approach.\n\r",victim->name);
        send_to_char (buf, ch);
        return TRUE;
      }
      if (current_time - ch->pcdata->last_pkdeath < (60*MULTIKILL_WAIT_TIME) )
      {
        send_to_char("You must be declared killable again, first. (type `kkillable``)\n\r",ch);
        return TRUE;
      }
      return FALSE;
    }
    if (clan_table[ch->clan].ctype == CLAN_TYPE_EXPL)
    {
      send_to_char
        ("You have to defect, then type 'clan loner' or join a PK clan to do that.\n\r",
        ch);
      return TRUE;
    }
    if (ch->pcdata->loner)
    {
      if (clan_table[victim->clan].ctype == CLAN_TYPE_EXPL)
      {
        sprintf (buf, "%s is protected by the God of Peace.\n\r",
          victim->name);
        send_to_char (buf, ch);
        return TRUE;
      }
      //if (clan_table[victim->clan].ctype == CLAN_TYPE_PC )
      //	  if (!victim->pcdata->loner && !(clan_table[victim->clan].ctype == CLAN_TYPE_PC))
      //	    return FALSE;
      if (!victim->pcdata->loner && !(clan_table[victim->clan].ctype == CLAN_TYPE_PC))
      {
        sprintf (buf, "%s is protected by the God of Peace.\n\r",
          victim->name);
        send_to_char (buf, ch);
        return TRUE;
      }
      if (current_time - victim->pcdata->last_pkdeath < (60*MULTIKILL_WAIT_TIME) ) 
      {
        sprintf (buf, "%s is protected by a divine light, you dare not approach.\n\r",victim->name);
        send_to_char (buf, ch);
        return TRUE;
      }
      if (current_time - ch->pcdata->last_pkdeath < (60*MULTIKILL_WAIT_TIME) )
      {
        send_to_char("You must be declared killable again, first. (type `kkillable``)\n\r",ch);
        return TRUE;
      }
      return FALSE;
    }
    if (!ch->pcdata->loner)
    {
      send_to_char
        ("You have to type 'clan loner' or join a PK clan to do that.\n\r",
        ch);
      return TRUE;
    }
    if (IS_SET (victim->act, PLR_WANTED))
      return TRUE;
  }
  return FALSE;
}

bool is_safe (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (victim->in_room == NULL || ch->in_room == NULL)
    return TRUE;
  if (victim->fighting == ch || victim == ch)
    return FALSE;
  if (check_shopkeeper_attack (ch, victim))
    return (TRUE);
  //  if (IS_SET (victim->act, PLR_WANTED) && (IS_NPC(ch) || (!IS_NPC(ch) && (ch->pcdata->loner || ch->clan != CLAN_BOGUS))))
  //    return FALSE;
  if (victim->dueler && ch->dueler)
    return FALSE;
  //if (isorder && !IS_NPC(victim) && ch != victim)
  //  return TRUE;
  if (isorder && !IS_NPC(victim) && ch != victim)
  {
    send_to_char("Luckily, you are smart enough to refuse that order.\n\r",ch);
    return TRUE;
  }

  /* killing mobiles */
  if (IS_NPC (victim))
  {

    /* safe room? */
    if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
    {
      send_to_char ("Not in this room.\n\r", ch);
      stop_fighting (ch, TRUE);
      return TRUE;
    }
    if (victim->pIndexData->pShop != NULL)
    {
      send_to_char ("The shopkeeper wouldn't like that.\n\r", ch);
      return TRUE;
    }

    /* no killing healers, trainers, etc */
    if (IS_SET (victim->act, ACT_IS_HEALER)
      //        || IS_SET (victim->act, ACT_PRACTICE)
      //        || IS_SET (victim->act, ACT_TRAIN)
      || IS_SET (victim->act, ACT_IS_CHANGER))
    {
      send_to_char ("I don't think the gods would approve.\n\r", ch);
      return TRUE;
    }

    if (IS_SET (victim->act2, ACT_ILLUSION))
    {
      send_to_char ("Try as you might, you cannot attack an illusion.\n\r",ch);
      return TRUE;
    }
    if (IS_SET(victim->act2,ACT_IMAGINARY) && (ch->Class != PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch)))
      return TRUE;
    if (IS_SET(victim->act2,ACT_IMAGINARY) && victim->in_room && victim->in_room->area->nplayer > 1)
    {
      act("Does $N even exist?  Others here do not think so.",ch,NULL,victim,TO_CHAR);
      return TRUE;
    }



    if (!IS_NPC (ch))
    {

      /* no pets */
      if (IS_SET (victim->act, ACT_PET))
      {
        act ("But $N looks so cute and cuddly...", ch, NULL,
          victim, TO_CHAR);
        return TRUE;
      }

      /* no charmed creatures unless owner */
      if (IS_AFFECTED (victim, AFF_CHARM) && ch != victim->master && !IS_SET(victim->act2,ACT_FAMILIAR))
      {
        send_to_char ("You don't own that creature.\n\r", ch);
        return TRUE;
      }

      if (IS_SET(victim->act2,ACT_PUPPET) && !ch->pcdata->loner)
      {
        send_to_char ("You can't kill that if you're nopk.\n\r",ch);
        return TRUE;
      }
    }
  }

  /* killing players */
  else
  {

    /* NPC doing the killing */
    if (IS_NPC (ch))
    {

      /* safe room check */
      if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
      {
        send_to_char ("Not in this room.\n\r", ch);
        stop_fighting (ch, TRUE);
        return TRUE;
      }

      /* charmed mobs and pets cannot attack players while owned */
      if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL
        && ch->master->fighting != victim)
      {
        send_to_char ("Players are your friends!\n\r", ch);
        return TRUE;
      }

      if (IS_SET (ch->act2,ACT_PUPPET) && !IS_NPC(victim) && !victim->pcdata->loner)
      {
        char buf[MAX_STRING_LENGTH];
        sprintf (buf, "%s is protected by the God of Peace.\n\r", victim->name);
        send_to_char (buf, ch);
        return TRUE;
      }
    }

    /* player doing the killing */
    else
    {
      if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
      {
        send_to_char ("Not in this room.\n\r", ch);
        stop_fighting (ch, TRUE);
        return TRUE;
      }
      if (safe_nopk && !(battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner)) && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner))))
        if (check_nonpk (ch, victim))
          return TRUE;
    }
  }
  return FALSE;
}


//
// Akamai: 11/2/98 - Looking at why Spells don't protect immortals
// and why area affects don't quite work correctly
//
bool is_safe_spell (CHAR_DATA * ch, CHAR_DATA * victim, bool area)
{

  // if victim or the char are not in any room (huh?) then
  // the victim is *safe* from the spell
  if (victim->in_room == NULL || ch->in_room == NULL)
    return TRUE;

  // if the victim is ch *and* it's an area attack then the
  // victim is *safe* from the spell
  if (victim == ch && area)
    return TRUE;

  // if the victim is already fighting the cast(er) or the victim
  // is the caster then *not safe* from spell
  if (victim->fighting == ch || victim == ch)
    return FALSE;

  // if the caster is an immortal and the spell is not an area attack
  // then the victim is *not safe* 
  if (IS_IMMORTAL (ch) && !area)
    return FALSE;

  // safe rooms are safe
  if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
    return TRUE;

  /* killing mobiles */
  if (IS_NPC (victim))
  {

    // in mob shop owner
    if (victim->pIndexData->pShop != NULL)
      return TRUE;

    // imaginary mobs
    if (IS_SET(victim->act2,ACT_IMAGINARY) && ((ch->Class != PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(ch)
      && victim->in_room) || victim->in_room->area->nplayer > 1))
      return TRUE;

    /* no killing healers, trainers, etc */
    if (IS_SET (victim->act, ACT_IS_HEALER) ||
      //        IS_SET (victim->act, ACT_PRACTICE) ||
      //        IS_SET (victim->act, ACT_TRAIN) ||
      IS_SET (victim->act, ACT_IS_CHANGER))
      return TRUE;
    if (!IS_NPC (ch))
    {

      /* no pets - pets are safe from player spells */
      if (IS_SET (victim->act, ACT_PET))
        return TRUE;

      /* no charmed creatures unless owner */
      if (IS_AFFECTED (victim, AFF_CHARM) &&
        (area || ch != victim->master))
        return TRUE;

      /* legal kill? -- cannot hit mob fighting non-group member 
      if( victim->fighting != NULL &&
      !is_same_group(ch, victim->fighting)) return TRUE; */
    }

    else
    {

      /* area effect spells do not hit other mobs */
      // area attack only affect the group you are fighting
      if (area && !is_same_group (victim, ch->fighting))
        return TRUE;
    }
  }

  else
  {

    /* killing players */
    if (area && IS_IMMORTAL (victim))
      return TRUE;

    /* NPC doing the killing */
    if (IS_NPC (ch))
    {

      /* charmed mobs and pets cannot attack players while owned */
      if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL
        && ch->master->fighting != victim)
        return TRUE;

      /* legal kill? -- mobs only hit players grouped with opponent */
      if (ch->fighting != NULL && !is_same_group (ch->fighting, victim))
        return TRUE;
    }

    else
    {
      if (safe_nopk && !(battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner)) && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner))))
        if (check_nonpk (ch, victim))
          return TRUE;

      /* player doing the killing */
      if (ch->level > (victim->level + PLAYER_LEVEL_DIFF))
        return TRUE;
    }
  }
  return FALSE;
}


/*
* Check for parry.
*/
bool check_parry (CHAR_DATA * ch, CHAR_DATA * victim)
{
  int chance;
  OBJ_DATA *bow;
  if (!IS_AWAKE (victim))
    return FALSE;
  chance = get_skill (victim, gsn_parry) / 3 + 5 + get_curr_stat(victim, STAT_DEX);
  if (get_eq_char (victim, WEAR_WIELD_R) == NULL
    && get_eq_char (victim, WEAR_WIELD_L) == NULL)
  {
    if (IS_NPC (victim))
      chance /= 2;

    else
      return FALSE;
  }
  if (!can_see (ch, victim))
  {
    if (number_percent () < get_skill (ch, gsn_blindfighting))
      check_improve (ch, gsn_blindfighting, TRUE, 2);

    else
    {
      check_improve (ch, gsn_blindfighting, FALSE, 2);
      chance /= 2;
    }
  }
  // Minax 12-20-02 Parry depends on skill, not level
  //  if (number_percent () >= chance + victim->level - ch->level)
  if (number_percent () >= chance)
    return FALSE;
  bow = get_eq_char (victim, WEAR_WIELD_R);
  if (bow == NULL)
    bow = get_eq_char (victim, WEAR_WIELD_L);
  if (bow != NULL
    && (bow->value[0] == WEAPON_LONGBOW
    || bow->value[0] == WEAPON_SHORTBOW) && number_percent () > 80)
    bow->condition--;
  act ("You raise your weapon to parry $n's attack.", ch, NULL, victim,
    TO_VICT);
  act ("$N parries your attack with $S weapon.", ch, NULL, victim, TO_CHAR);
  check_improve (victim, gsn_parry, TRUE, 6);
  return TRUE;
}


/*
* Check for shield block.
*/
bool check_shield_block (CHAR_DATA * ch, CHAR_DATA * victim)
{
  int chance;
  if (!IS_AWAKE (victim))
    return FALSE;
  if (!IS_NPC(victim) && victim->race == PC_RACE_NERIX)
  {
    chance = 9 + get_curr_stat(victim, STAT_STR);
    if (number_percent () >= chance)
      return FALSE;
    act ("You block $n's attack with your humongous wings.", ch, NULL, victim, TO_VICT);
    act ("$N blocks your attack with $s humongous wings.", ch, NULL, victim, TO_CHAR);
    return TRUE;
  }

  else
  {
    chance = get_skill (victim, gsn_shield_block) / 10 + get_curr_stat(victim, STAT_STR);
    if (get_eq_char (victim, WEAR_SHIELD) == NULL)
    {
      return FALSE;
    }
  }

  // Minax 12-20-02 Shield Block depends on skill, not level
  //  if (number_percent () >= chance + victim->level - ch->level)
  if (number_percent () >= chance)
    return FALSE;
  act ("You block $n's attack with your shield.", ch, NULL, victim, TO_VICT);
  act ("$N blocks your attack with a shield.", ch, NULL, victim, TO_CHAR);
  check_improve (victim, gsn_shield_block, TRUE, 4);
  return TRUE;
}


/*
* Check for dodge.
*/
bool check_dodge (CHAR_DATA * ch, CHAR_DATA * victim)
{
  int chance, some_number;
  short fighting = 0;
  if (!IS_AWAKE (victim))
    return FALSE;

  chance = get_skill (victim, gsn_dodge) / 3 + 5  + get_curr_stat(victim, STAT_DEX);
  if (victim->race == PC_RACE_NERIX && IS_SET (ch->affected_by, AFF_FLYING)
    && IS_SET (victim->affected_by, AFF_FLYING))
    chance += get_skill (victim, gsn_dodge) / 5;

  //IBLIS 5/31/03 - Split Vision skill for monks
  if (number_percent () < get_skill (victim, gsn_split_vision))
  {
    CHAR_DATA *gch;
    for (gch = victim->in_room->people; gch != NULL;
      gch = gch->next_in_room)
    {
      if (gch->fighting == victim)
        if (fighting++)
          break;
    }
    if (fighting > 1)
    {
      chance += get_curr_stat (victim, STAT_DEX);
      check_improve (victim, gsn_split_vision, TRUE, 2);
    }
  }
  if (!can_see (victim, ch))
  {
    if (number_percent () < get_skill (victim, gsn_blindfighting))
      check_improve (victim, gsn_blindfighting, TRUE, 2);

    else
    {
      check_improve (victim, gsn_blindfighting, FALSE, 2);
      chance /= 2;
    }
  }
  // Minax 12-20-02 Dodge depends on skill, not level
  //  if (number_percent () >= chance + victim->level - ch->level)
  if (number_percent () >= chance)
    return FALSE;

  //IBLIS 5/31/03 - Added new dodge messages for monks
  chance = (get_skill (victim, gsn_counter)) * .35;
  if (get_eq_char (ch, WEAR_WIELD_R) == NULL
    && get_eq_char (ch, WEAR_WIELD_L) == NULL && chance > 0)
    chance += 20;
  if (/*!IS_NPC (victim) &&*/ number_percent () < chance)
  {
    counter_attack (victim, ch);
    return TRUE;
  }
  if (!IS_CLASS (victim, PC_CLASS_MONK) && !IS_SET(victim->act2,ACT_MONK))
  {
    act ("You quickly step out of the way, dodging $n's attack.", ch,
      NULL, victim, TO_VICT);
    act ("$N quickly steps out of the way, dodging your attack.", ch,
      NULL, victim, TO_CHAR);
  }

  else
  {
    some_number = number_percent ();
    if (some_number < 20)
    {
      act ("Tilting your head to the side you avoid $n's strike.",
        ch, NULL, victim, TO_VICT);
    }

    else if (some_number < 40)
    {
      act
        ("You drop down to the ground as you narrowly avoid $n's weapon.",
        ch, NULL, victim, TO_VICT);
    }

    else if (some_number < 60)
    {
      act
        ("$n falls for your feint and you avoid their attack with lots of room to spare.",
        ch, NULL, victim, TO_VICT);
    }

    else if (some_number < 80)
    {
      act ("You dive over $n's swing and land in a roll.", ch, NULL,
        victim, TO_VICT);
    }

    else
      act ("You jump to evade the swing aimed for your legs.", ch,
      NULL, victim, TO_VICT);
    some_number = number_percent ();
    if (some_number < 25)
      act ("You swing your weapon at $N, who casually avoids the attack.",
      ch, NULL, victim, TO_CHAR);

    else if (some_number < 50)
      act ("$N drops to the side, rolling away from your attack.",
      ch, NULL, victim, TO_CHAR);

    else if (some_number < 75)
      act
      ("$N moves into the blind spot of your swordsarm to avoid your strike.",
      ch, NULL, victim, TO_CHAR);

    else
      act
      ("As if all your moves were predicted, $N gracefully eludes your attempts to hit $m.",
      ch, NULL, victim, TO_CHAR);
  }
  check_improve (victim, gsn_dodge, TRUE, 6);
  return TRUE;
}


/*
* Set position of a victim.
*/
void update_pos (CHAR_DATA * victim)
{
  if (victim->hit > 0)
  {
    if (victim->position <= POS_STUNNED)
      victim->position = POS_STANDING;
    return;
  }
  if (IS_NPC (victim) && victim->hit < 1)
  {
    victim->position = POS_DEAD;
    return;
  }
  if (victim->hit <= -11)
  {
    victim->position = POS_DEAD;
    return;
  }
  if (victim->hit <= -6)
    victim->position = POS_MORTAL;

  else if (victim->hit <= -3)
    victim->position = POS_INCAP;

  else
    victim->position = POS_STUNNED;
  return;
}

void add_hunting (CHAR_DATA * hunter, CHAR_DATA * hunted)
{
  MEMORY_DATA *memd;
  for (memd = hunter->hunt_memory; memd; memd = memd->next_memory)
    if (memd->player == hunted)
      return;
  memd = new_memory ();
  memd->mob = hunter;
  memd->player = hunted;
  memd->next_memory = hunter->hunt_memory;
  hunter->hunt_memory = memd;
  memd->next = memories;
  memories = memd;
}

void extract_memory (MEMORY_DATA * memd)
{
  MEMORY_DATA *prev;
  if (memd == memories)
  {
    memories = memd->next;
  }

  else
  {
    for (prev = memories; prev; prev = prev->next)
      if (prev->next == memd)
      {
        prev->next = memd->next;
        break;
      }
  }
  if (memd == memd->mob->hunt_memory)
  {
    memd->mob->hunt_memory = memd->next_memory;
  }

  else
  {
    for (prev = memd->mob->hunt_memory; prev; prev = prev->next_memory)
    {
      if (prev->next_memory == memd)
      {
        prev->next_memory = memd->next_memory;
        break;
      }
    }
  }
  free_memory (memd);
}


/*
* Start fights.
*/
void set_fighting (CHAR_DATA * ch, CHAR_DATA * victim)
{

  /*
  char buf[MAX_STRING_LENGTH];
  if (ch->fighting != NULL)
  {
  sprintf(buf, "Set_fighting: %s is already fighting %s", 
  !IS_NPC(ch) ? ch->name : ch->short_descr, 
  !IS_NPC(victim) ? victim->name : victim->short_descr );
  bug (buf, 0);
  return;
  }
  */
  if (IS_SET(ch->act2, ACT_ILLUSION) || IS_SET(victim->act2, ACT_ILLUSION) 
    || IS_SET(victim->act2,ACT_NO_KILL) || IS_SET(ch->act2,ACT_NO_KILL))
    return;
  ch->last_fought = victim;
  victim->last_fought = ch;
  update_aggressor (victim, ch);
  ch->aggres = NULL;
  if (victim != ch)
    victim->aggres = ch;
  if (IS_AFFECTED (ch, AFF_SLEEP))
    affect_strip (ch, gsn_sleep);
  if (is_affected (ch, skill_lookup ("Ballad of Ole Rip")))
    affect_strip (ch, skill_lookup ("Ballad of Ole Rip"));
  if (!IS_NPC (ch))
  {
    ch->pcdata->knock_time = 0;
    ch->pcdata->last_fight = time (NULL);
    if (IS_NPC (victim) && (IS_SET (victim->act, ACT_HUNT)
      || IS_SET (victim->act, ACT_REMEMBER)
      || IS_SET (victim->act, ACT_SMART_HUNT)))
    {
      add_hunting (victim, ch);
    }
  }
  if (!IS_NPC (victim))
  {
    if (IS_NPC (ch) &&
      (IS_SET (ch->act, ACT_HUNT) || IS_SET (ch->act, ACT_REMEMBER)
      || IS_SET (ch->act, ACT_SMART_HUNT)))
    {
      add_hunting (ch, victim);
    }
    victim->pcdata->knock_time = 0;
  }
  ch->fighting = victim;
  ch->on = NULL;
  victim->on = NULL;
  if (!ch->bashed)
    ch->position = POS_FIGHTING;
  return;
}

void set_fighting_auto (CHAR_DATA * ch, CHAR_DATA * victim)
{

  /*
  char buf[MAX_STRING_LENGTH];
  if (ch->fighting != NULL)
  {
  sprintf(buf, "Set_fighting: %s is already fighting %s", 
  !IS_NPC(ch) ? ch->name : ch->short_descr, 
  !IS_NPC(victim) ? victim->name : victim->short_descr );
  bug (buf, 0);
  return;
  }
  */
  if (IS_SET(ch->act2, ACT_ILLUSION) || IS_SET(victim->act2, ACT_ILLUSION)
    || IS_SET(victim->act2,ACT_NO_KILL) || IS_SET(ch->act2,ACT_NO_KILL))	  
    return;
  if (ch != victim)
    ch->aggres = victim;
  update_aggressor (victim, ch);
  if (IS_AFFECTED (ch, AFF_SLEEP))
    affect_strip (ch, gsn_sleep);
  if (is_affected (ch, skill_lookup ("Ballad of Ole Rip")))
    affect_strip (ch, skill_lookup ("Ballad of Ole Rip"));
  if (!IS_NPC (ch))
  {
    ch->pcdata->knock_time = 0;
    ch->pcdata->last_fight = time (NULL);
    if (IS_NPC (victim)
      && (IS_SET (victim->act, ACT_HUNT)
      || IS_SET (victim->act,
      ACT_REMEMBER) || IS_SET (victim->act,
      ACT_SMART_HUNT)))
    {
      add_hunting (victim, ch);
    }
  }
  if (!IS_NPC (victim))
  {
    if (IS_NPC (ch)
      && (IS_SET (ch->act, ACT_HUNT)
      || IS_SET (ch->act, ACT_REMEMBER)
      || IS_SET (ch->act, ACT_SMART_HUNT)))
      add_hunting (ch, victim);
    victim->pcdata->knock_time = 0;
  }
  ch->fighting = victim;
  ch->on = NULL;
  victim->on = NULL;
  if (!ch->bashed)
    ch->position = POS_FIGHTING;
  return;
}


/*
* Stop fights.
*/
void stop_fighting (CHAR_DATA * ch, bool fBoth)
{
  CHAR_DATA *fch;
  OBJ_DATA *wield;
  if (!IS_NPC(ch) && ch->pcdata->aggression)
    ch->pcdata->aggression = 1;
  if (!IS_NPC(ch) && ch->Class == PC_CLASS_CHAOS_JESTER)
    ch->pcdata->pain_points = 0;
  if (((wield = get_eq_char (ch, WEAR_WIELD_R)) != NULL) && IS_WEAPON_STAT(wield,WEAPON_HOLY_ACTIVATED))
    REMOVE_BIT(wield->value[4],WEAPON_HOLY_ACTIVATED);
  if (((wield = get_eq_char (ch, WEAR_WIELD_L)) != NULL) && IS_WEAPON_STAT(wield,WEAPON_HOLY_ACTIVATED))
    REMOVE_BIT(wield->value[4],WEAPON_HOLY_ACTIVATED);
  //AFFECT_DATA *af;
  /*  if (ch->fighting != NULL && is_affected(ch->fighting,gsn_evil_twin))
  {
  for (af = ch->fighting->affected; af != NULL; af = af->next)
  {
  if (af->type == gsn_evil_twin)
  break;
  }
  if (af != NULL && af->comp_name != NULL && !str_cmp(ch->name,af->comp_name))
  affect_remove(ch->fighting,af);
  }*/
  for (fch = char_list; fch != NULL; fch = fch->next)
  {
    if (fch == ch || (fBoth && fch->fighting == ch))
    {
      /*	  if (fch->fighting != NULL && is_affected(fch->fighting,gsn_evil_twin))
      {
      for (af = fch->fighting->affected; af != NULL; af = af->next)
      {
      if (af->type == gsn_evil_twin)
      break;
      }
      if (af != NULL && af->comp_name != NULL && !str_cmp(fch->name,af->comp_name))
      affect_remove(fch->fighting,af);
      }*/
      fch->fighting = NULL;
      fch->stunned = 0;
      if (!IS_NPC(fch) && fch->pcdata->aggression)
        fch->pcdata->aggression = 1;
      if (!IS_NPC(fch) && fch->Class == PC_CLASS_CHAOS_JESTER)
        fch->pcdata->pain_points = 0;
      if (((wield = get_eq_char (fch, WEAR_WIELD_R)) != NULL) && IS_WEAPON_STAT(wield,WEAPON_HOLY_ACTIVATED))
        REMOVE_BIT(wield->value[4],WEAPON_HOLY_ACTIVATED);
      if (((wield = get_eq_char (fch, WEAR_WIELD_L)) != NULL) && IS_WEAPON_STAT(wield,WEAPON_HOLY_ACTIVATED))
        REMOVE_BIT(wield->value[4],WEAPON_HOLY_ACTIVATED);
      if (IS_NPC (fch))
        fch->position = fch->default_pos;

      else
      {
        if (fch->mount != NULL && fch->mount->riders == fch)
          fch->position = POS_MOUNTED;

        else
          fch->position = POS_STANDING;
      }
      update_pos (fch);
    }
  }
  return;
}


/*
* Make a corpse out of a character.
*/
void make_corpse (CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *corpse;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  char *name;

  if (IS_SET(ch->act2,ACT_PUPPET))
    return;

  if (IS_NPC (ch))
  {
    name = ch->short_descr;
    corpse = create_object (get_obj_index (OBJ_VNUM_CORPSE_NPC), 0);
    corpse->timer = number_range (3, 6);
  }

  else
  {
    name = ch->name;
    corpse = create_object (get_obj_index (OBJ_VNUM_CORPSE_PC), 0);
    corpse->timer = number_range (29, 31);
    corpse->owner = NULL;
    corpse->value[5] = ch->pcdata->been_killed[PLAYER_KILL] + ch->pcdata->been_killed[MOB_KILL]
    + ch->pcdata->other_deaths;
    if (corpse->plr_owner)
      free_string(corpse->plr_owner);
    corpse->plr_owner = str_dup(ch->name);
  }
  if (ch->gold > 0 || ch->silver > 0)
  {
    if (ch->race == PC_RACE_NIDAE || IS_SET(ch->form,FORM_INSTANT_DECAY))
      obj_to_room(create_money (ch->gold, ch->silver), ch->in_room);
    else obj_to_obj (create_money (ch->gold, ch->silver), corpse);
    ch->gold = 0;
    ch->silver = 0;
  }
  corpse->cost = 0;
  corpse->level = ch->level;
  sprintf (buf, corpse->short_descr, name);
  free_string (corpse->short_descr);
  corpse->short_descr = str_dup (buf);
  sprintf (buf, corpse->description, name);
  free_string (corpse->description);
  corpse->description = str_dup (buf);
  corpse->value[6] = ch->size;
  for (obj = ch->carrying; obj != NULL; obj = obj_next)
  {
    bool floating = FALSE;
    obj_next = obj->next_content;
    if (IS_SET(obj->extra_flags[1],ITEM_TATTOO))
      continue;
    //      if (IS_SET(obj->wear_loc,WEAR_FLOAT))
    //	floating = TRUE;
    unequip_char (ch, obj);
    obj_from_char (obj);
    if (obj->item_type == ITEM_POTION)
      obj->timer = number_range (500, 1000);
    if (obj->item_type == ITEM_SCROLL)
      obj->timer = number_range (1000, 2500);
    if (!IS_NPC (ch) && IS_SET (obj->extra_flags[0], ITEM_ROT_DEATH))
    {
      extract_obj (obj);
      continue;
    }
    if (IS_SET (obj->extra_flags[0], ITEM_VIS_DEATH))
      REMOVE_BIT (obj->extra_flags[0], ITEM_VIS_DEATH);
    if (IS_SET (obj->extra_flags[0], ITEM_INVENTORY))
      extract_obj (obj);

    else if ((floating && ch->race != PC_RACE_AVATAR) || ch->race == PC_RACE_NIDAE)
    {
      act ("$p falls to the floor.", ch, obj, NULL, TO_ROOM);
      obj_to_room (obj, ch->in_room);
    }

    else if (IS_SET(ch->form,FORM_INSTANT_DECAY))
    {
      act ("$p falls to the floor.", ch, obj, NULL, TO_ROOM);
      obj_to_room(obj,ch->in_room);
    }

    else
      obj_to_obj (obj, corpse);
  }
  if (number_percent () == 1 && number_percent () <= 10
    && number_percent () <= 20 && ch->level > 1 && IS_NPC(ch))
  {
    OBJ_DATA *robj;
    robj = create_object (get_obj_index (OBJ_VNUM_RUNE_CONV), 0);
    robj->value[3] = number_range(1,30000);
    obj_to_obj (robj, corpse);
  }
  if (ch->race == PC_RACE_AVATAR || IS_SET(ch->act2, ACT_ILLUSION) || ch->race == PC_RACE_NIDAE
    || IS_SET(ch->form,FORM_INSTANT_DECAY))
    extract_obj(corpse);
  else 
  {
    obj_to_room (corpse, ch->in_room);
    if (!IS_NPC(ch))
      save_player_corpse(corpse);

  }
  if (!IS_NPC(ch) && IS_CLASS(ch,PC_CLASS_REAVER) && IS_CLASS(ch, PC_CLASS_DRUID))
    reset_char (ch);
  if (!IS_NPC(ch) && ch->race == PC_RACE_NIDAE)
  {
    corpse = create_object(get_obj_index(OBJ_VNUM_ICE_STATUE),0);
    obj_to_room(corpse,ch->in_room);
    corpse->timer = 100;
  }
  return;
}


/*
* Improved Death_cry contributed by Diavolo.
*/
void death_cry (CHAR_DATA * ch, bool not_mob_killer)
{
  ROOM_INDEX_DATA *was_in_room;
  char *msg;
  int door;
  int vnum;
  vnum = 0;
  msg = "$n's chilling death cry sends a shiver up your spine.";
  if (IS_SET(ch->act2,ACT_SWITCHED))
    return;
  switch (number_range (0, 7))
  {
  case 0:
    msg = "$n hits the ground with a thud, DEAD.";
    break;
  case 1:
    if (ch->material == 0)
    {
      msg = "$n splatters blood on your armor.";
      break;
    }
  case 2:
    if (IS_SET (ch->parts, PART_GUTS))
    {
      msg = "$n spills $s guts all over the floor.";
      vnum = OBJ_VNUM_GUTS;
    }
    break;
  case 3:
    if (IS_SET (ch->parts, PART_HEAD))
    {
      msg = "$n's severed head plops on the ground.";
      vnum = OBJ_VNUM_SEVERED_HEAD;
    }
    break;
  case 4:
    if (IS_SET (ch->parts, PART_HEART))
    {
      msg = "$n's heart is torn from $s chest.";
      vnum = OBJ_VNUM_TORN_HEART;
    }
    break;
  case 5:
    if (IS_SET (ch->parts, PART_ARMS))
    {
      msg = "$n's arm is sliced from $s dead body.";
      vnum = OBJ_VNUM_SLICED_ARM;
    }
    break;
  case 6:
    if (IS_SET (ch->parts, PART_LEGS))
    {
      msg = "$n's leg is sliced from $s dead body.";
      vnum = OBJ_VNUM_SLICED_LEG;
    }
    break;
  case 7:
    if (IS_SET (ch->parts, PART_BRAINS))
    {
      msg = "$n's head is shattered, and $s brains splash all over you.";
      vnum = OBJ_VNUM_BRAINS;
    }
  }
  if ((number_range (1, 2) == 1) && (IS_SET (ch->parts, PART_FEATHERS)))
  {
    msg = "The feather of $n is plucked from $s corpse.";
    vnum = OBJ_VNUM_FEATHER;
  }
  if (!IS_NPC (ch) && not_mob_killer && !battle_royale)
  {
    if (IS_SET (ch->parts, PART_EYE))
    {
      msg = "The eye of $n pops from $s head and bounces off your foot.";
      vnum = OBJ_VNUM_EYE;
    }
  }
  act (msg, ch, NULL, NULL, TO_ROOM);
  if (vnum != 0)
  {
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    char *name;
    name = IS_NPC (ch) ? ch->short_descr : ch->name;
    obj = create_object (get_obj_index (vnum), 0);
    if (obj->item_type != ITEM_GEM && obj->item_type != ITEM_TREASURE && obj->item_type != ITEM_FEATHER)
      obj->timer = number_range (4, 7);
    sprintf (buf, obj->short_descr, name);
    free_string (obj->short_descr);
    obj->short_descr = str_dup (buf);
    sprintf (buf, obj->description, name);
    free_string (obj->description);
    obj->description = str_dup (buf);
    if (obj->pIndexData->vnum == OBJ_VNUM_EYE)
    {
      sprintf(buf,"%s %s",obj->name,name);
      free_string(obj->name);
      obj->name = str_dup(buf);
    }
    if (obj->item_type == ITEM_FOOD)
    {
      if (IS_SET (ch->form, FORM_POISON))
        obj->value[3] = 1;

      else if (!IS_SET (ch->form, FORM_EDIBLE))
        obj->item_type = ITEM_TRASH;
    }
    obj_to_room (obj, ch->in_room);
  }
  if (IS_NPC (ch))
    msg = "You hear something's death cry.";

  else
    msg = "You hear the tortured cry of someone dying nearby.";
  was_in_room = ch->in_room;
  for (door = 0; door <= 5; door++)
  {
    EXIT_DATA *pexit;
    if ((pexit = was_in_room->exit[door]) != NULL
      && pexit->u1.to_room != NULL && pexit->u1.to_room != was_in_room)
    {
      ch->in_room = pexit->u1.to_room;
      act (msg, ch, NULL, NULL, TO_ROOM);
    }
  }
  ch->in_room = was_in_room;
  return;
}

void raw_kill (CHAR_DATA * victim, CHAR_DATA * killer)
{
  int i;
  CHAR_DATA *vch;
  DESCRIPTOR_DATA *d;
  MEMORY_DATA *memd, *memd_next;
  char strsave[MAX_INPUT_LENGTH];
  int soulgain = 0;
  char buf[MAX_STRING_LENGTH];

  if (victim == killer && !IS_NPC(victim))
    victim->pcdata->other_deaths++;

  if (!(battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner))) && !victim->dueler)
  {
    if (IS_SET(victim->act,PLR_WANTED) && IS_NPC (killer) && killer->spec_fun != 0
      && (!str_cmp("spec_executioner",spec_name (killer->spec_fun))
      || !str_cmp("spec_guard",spec_name (killer->spec_fun))))
    {
      do_peace(victim,"");
      victim->hit = 1;
      victim->position = POS_RESTING;
      arrest(killer,victim);
      act("$n beats you to unconsciousness.  You awake to find yourself behind bars.",
        killer,NULL, victim,TO_VICT);
      return;
    }
  }


  sprintf (buf, "%s got toasted by %s at %s [room %d]",
    (IS_NPC (victim) ? victim->short_descr : victim->name),
    (IS_NPC (killer) ? killer->short_descr : killer->name),
    killer->in_room->name, killer->in_room->vnum);

  if (IS_NPC (victim))
    wiznet (buf, NULL, NULL, WIZ_MOBDEATHS, 0, 0);

  else
    wiznet (buf, NULL, NULL, WIZ_DEATHS, 0, 0);

  if (!IS_NPC(victim) && victim->pcdata->soul_link)
  {
    CHAR_DATA *tempc;
    tempc = victim->pcdata->soul_link;
    send_to_char("Your soul linked partner has died, drawing you into death.\n\r",victim->pcdata->soul_link);
    send_to_char("Your soul link draws your partner into death.\n\r",victim);
    affect_strip (victim,gsn_soul_link);
    affect_strip (tempc,gsn_soul_link);
    raw_kill(tempc,tempc);
  }

  feed_death(victim,killer);
  if (!IS_NPC(victim))
  {
    for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->connected == CON_PLAYING && IS_AWAKE (d->character) &&
        d->pEdit == NULL && d->editor == 0 && !IS_NPC(d->character) &&
        get_skill(d->character,gsn_sense_death) > number_percent())
      {
        send_to_char ("Your pulse quickens as a mortal life is taken from the world.\n\r", d->character);
        check_improve (d->character, gsn_sense_death, TRUE, 2);
      }
    }
  }


  if (!IS_NPC (victim))
  {
    if (IS_NPC (killer))
    {

      //          kill_table[URANGE(0, killer->level, MAX_LEVEL - 1)].kills++;
      //          killer->pIndexData->kills++;
    }
    else
    {
      if (victim != killer && !(battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner))))
      {
        OBJ_DATA *binder, *tobj;
        int total=0,counter=0;
        victim->pcdata->last_pkdeath = current_time;
        sprintf(buf,"`k%s `lis surrounded by a protective light.``",victim->name);
        pk_chan(buf);
        binder = get_obj_vnum_char(victim, OBJ_VNUM_BINDER);
        if (binder && binder->contains) 
        {
          for (tobj = binder->contains; tobj != NULL; tobj = tobj->next_content)
            ++total;
          total = number_range(1,total);
          for (tobj = binder->contains; tobj != NULL; tobj = tobj->next_content)
          {
            if (++counter == total)
            {
              obj_from_obj(tobj);
              obj_to_char(tobj,killer);
              act("You win $p from $N due to your victory over $M.",killer,tobj,victim,TO_CHAR);
              act_new("You lose $p to $n due to $s victory over you.",killer,tobj,victim,TO_VICT,POS_DEAD);
              break;
            }
          }
        }
      }

      if (killer->race == PC_RACE_KALIAN && victim != killer  && victim->in_room == killer->in_room && !IS_SET(victim->form,FORM_INSTANT_DECAY))
      {
        act("`iYou pounce onto the corpse of $N`i, sink your teeth into $s neck and feed.``",killer,NULL,victim,TO_CHAR);
        act("`iYou watch in horror as $n leaps onto the corpse of $N`i and feeds upon it.``",killer,NULL,victim,TO_ROOM);
        killer->pcdata->condition[COND_THIRST] += number_range(4,8);
        if (killer->pcdata->condition[COND_THIRST] > 45)
          killer->pcdata->condition[COND_THIRST] = 45;
        switch (victim->race)
        {
        case PC_RACE_DWARF: spell_giant_strength (find_spell (killer, "giant strength"), 
                              victim->level/9,killer, (void*)killer,0);
          break;
        case PC_RACE_CANTHI: spell_aqua_breathe (find_spell (killer, "aqua breathe"), victim->level/10,killer, (void*)killer,0);
          break;
        case PC_RACE_SIDHE: spell_pass_door (find_spell (killer, "pass door"), victim->level/4,killer, (void*)killer,0);
          break;
        case PC_RACE_ELF: spell_haste (find_spell (killer, "haste"), victim->level/6,killer, (void*)killer,0);
          break;
        case PC_RACE_SYVIN: spell_bark_skin (gsn_bark_skin, victim->level/10, killer, (void*)killer,0);
          break;
        case PC_RACE_AVATAR: prayer_sanctuary (find_spell (killer, "sanctuary"), victim->level/2, killer, (void*)killer,0);
          break;
        case PC_RACE_VROATH: prayer_frenzy (find_spell (killer, "frenzy"), victim->level/5, killer, (void*)killer,0);
          break;
        case PC_RACE_HUMAN: spell_heal (find_spell (killer, "heal"), victim->level/5, killer, (void*)killer,0);
          break;
        case PC_RACE_LITAN: spell_fireshield (find_spell (killer, "fireshield"), victim->level, killer, (void*)killer,0);
          break;
        case PC_RACE_KALIAN: send_to_char("Kalian's blood is harmful to your health.\n\r",killer);
          spell_plague (find_spell (killer, "plague"), victim->level/8, killer, (void*)killer,0);
          break;
        case PC_RACE_LICH: spell_protection_evil (find_spell (killer, ""), victim->level, killer, (void*)killer,0);
          break;
        case PC_RACE_NERIX: spell_protection_good (find_spell (killer, ""), victim->level, killer, (void*)killer,0);
          break;
        }
      }

    }
  }
  if (IS_NPC (victim))
  {
    if (IS_SET(victim->act2,ACT_QUESTMOB))
    {
      if (!IS_NPC(killer) && killer->pcdata->quest_mob == victim)
        killer->pcdata->quest_mob = killer;
      else
      {
        CHAR_DATA *wch;
        for (wch = char_list; wch != NULL; wch = wch->next)
        {
          if (IS_NPC(wch))
            continue;
          if (wch->pcdata->quest_mob == victim)
          {
            wch->pcdata->quest_mob = NULL;
            send_to_char("Alas, someone ELSE has killed the mob you were hunting for before you could reach it.\n\r",wch);
            send_to_char("Unfortunately, that means you have failed in your quest.\n\r",wch);
            wch->pcdata->quest_ticks = -10;
            break;
          }
        }
      }
    }

    if (!IS_NPC (killer))
    {
      if (!(battle_royale && (killer->level > 19 || (!IS_NPC(killer) && killer->pcdata->loner))))
      {
        if (IS_CLASS (killer, PC_CLASS_REAVER)
          && !IS_SET(killer->act,PLR_CARDKILL)
          && (get_eq_char (killer, WEAR_WIELD_L) !=
          NULL || get_eq_char (killer, WEAR_WIELD_R) != NULL))
        {
          soulgain = compute_soul_gain (killer, victim);
          sprintf (buf,
            "`aYou mercilessly stab %s `ainto the corpse of %s`a, reaping its energy.``\n\r",
            killer->sword->short_descr, victim->short_descr);
          send_to_char (buf, killer);
          act
            ("`aYou watch in horror as $n reaps the soul of $s victim with a stab of $s sword.``",
            killer, killer, killer, TO_ROOM);
          if (soulgain == 1)
            sprintf (buf, "`aYou gain `b1 `asoul energy.``\n\r");

          else
            sprintf (buf,
            "`aYou gain `b%d `asoul energies.``\n\r",
            soulgain);
          send_to_char (buf, killer);
          killer->pcdata->souls += soulgain;
          killer->pcdata->totalsouls += soulgain;
        }
      }
      if (killer->race == PC_RACE_KALIAN && victim->in_room == killer->in_room && !IS_SET(victim->form,FORM_INSTANT_DECAY))
      {
        act("`iYou pounce onto the corpse of $N`i, sink your teeth into $s neck and feed.``",killer,NULL,victim,TO_CHAR);
        act("`iYou watch in horror as $n leaps onto the corpse of $N`i and feeds upon $M.``",killer,NULL,victim,TO_ROOM);
        killer->pcdata->condition[COND_THIRST] += number_range(4,8);
        if (killer->pcdata->condition[COND_THIRST] > 45)
          killer->pcdata->condition[COND_THIRST] = 45;
      }
      //	  else
      //	    {

      //          kill_table[URANGE(0, killer->level, MAX_LEVEL - 1)].kills++;
      //          killer->pIndexData->kills++;
      //	    }
    }
    //      victim->pIndexData->deaths++;
    //      kill_table[URANGE(0, victim->level, MAX_LEVEL - 1)].deaths++;
    //
  }
  //  Minax 12-24-02  When Avatars die, they are gone forever
  if (!IS_NPC (victim) && !IS_IMMORTAL (victim) &&
    (victim->race == PC_RACE_AVATAR) && !(battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner))))
  {
    if (victim->pcdata->avatar_type < 2)
    {
      for (d = descriptor_list; d; d = d->next)
      {
        if (d->connected == CON_PLAYING)
        {
          vch = d->character;
          act
            ("`oAn odeous howl permeates the air as $N the Avatar becomes nothing more than a memory.",
            vch, NULL, victim, TO_CHAR);
        }
      }
      send_to_char
        ("`oYou shiver as your soul is ripped from your body.  Your mortal life has expired forever.``\n\r",
        victim);
      sprintf (strsave, "%s%c/%s", PLAYER_DIR, toupper (victim->name[0]),
        capitalize (victim->name));
      wiznet ("Avatar $N died and is being deleted.", victim, NULL, 0, 0, 0);
      if (victim->clan != CLAN_BOGUS)
        clan_defect (victim);
      do_hard_quit (victim, "", TRUE);
      unlink (strsave);
      return;
    }
    else //pk Avatar
    {
      send_to_char ("`oThe gods have reclaimed your corpse in order to hide all proof of your death.``\n\r", victim);
      wiznet ("Avatar $N died and their corpse went POOF!.", victim, NULL, 0, 0, 0);
    }
  }
  if (!IS_NPC (victim))
  {
    make_backup = TRUE;
    save_char_obj (victim);
    if (victim->pcdata->fishing)
      victim->pcdata->fishing = 0;
  }
  if (!IS_SET(victim->act2, ACT_ILLUSION))
    death_cry (victim, ((IS_NPC (killer) || killer == victim) ? FALSE : TRUE));
  if ((!(battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner))) || IS_NPC (victim)) && !victim->dueler)
    if (!(IS_SET(victim->act,PLR_WANTED) && IS_NPC (killer) && killer->spec_fun != 0
      && (!str_cmp("spec_executioner",spec_name (killer->spec_fun))
      || !str_cmp("spec_guard",spec_name (killer->spec_fun)))))
    {
      make_corpse (victim);
      if (!IS_NPC(victim) && victim->pcdata->avatar_type >= 3 && victim->race == PC_RACE_AVATAR)
      {
        reset_race(victim);
        send_to_char("You have died, and thus are no longer an Avatar.\n\r",victim);
      }
    }

    stop_fighting (victim, TRUE);
    if (is_dueling (victim))
    {
      duel_ends (victim);
      return;
    }
    if (victim->dueler)
    { 
      superduel_ends (killer,victim);
      return;
    }
    for (memd = memories; memd; memd = memd_next)
    {
      memd_next = memd->next;
      if (memd->player == victim || memd->mob == victim)
        extract_memory (memd);
    }
    if (IS_NPC (victim))
    {
      victim->pIndexData->killed++;
      kill_table[URANGE (0, victim->level, MAX_LEVEL - 1)].killed++;
      if (HAS_SCRIPT (victim))
      {
        TRIGGER_DATA *pTrig;
        for (pTrig = victim->triggers; pTrig != NULL; pTrig = pTrig->next)
        {
          pTrig->current = NULL;
          pTrig->tracer = 0;
        }
      }
      if (victim->mount != NULL)
      {
        victim->mount->riders = NULL;
        victim->mount->mounter = NULL;
        victim->mount = NULL;
        victim->position = POS_STANDING;
      }
      if (victim->riders != NULL)
      {
        send_to_char ("Alas, your trusty mount has died.\n\r",
          victim->riders);
        victim->riders->mount = NULL;
        victim->riders->position = POS_STANDING;
      }
      if (IS_SET(victim->act2,ACT_PUPPET)) {
	if (is_shapechanged_druid(victim))
		shapechange_return(victim, 2);
	else
        switched_return(victim,victim->in_room);
      }
      extract_char (victim, TRUE);
      return;
    }
    if (!(battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner))) && !victim->dueler)
    {
      if (IS_SET(victim->act,PLR_WANTED) && IS_NPC (killer) && killer->spec_fun != 0 
        && (!str_cmp("spec_executioner",spec_name (killer->spec_fun)) 
        || !str_cmp("spec_guard",spec_name (killer->spec_fun))))
      {
        do_peace(victim,"");
        victim->hit = 1;
        victim->position = POS_RESTING;
        arrest(killer,victim);
        act("$n beats you to unconsciousness.  You awake to find yourself behind bars.", killer,NULL, victim,TO_VICT);
        return;
      }
      if (IS_SET(victim->act2,ACT_PUPPET)) {
	if (is_shapechanged_druid(victim))
		shapechange_return(victim, 2);
	else
        switched_return(victim,victim->in_room);
      }
      extract_char (victim, FALSE);
    }

    else				//Do the most important things from extract char here
    {
      SCENT_DATA *sc, *sc_next;

      // clannies that have declared their clan hall get
      // to be resurected in their clan hall
      sprintf (buf, "%s %d", victim->name, ROOM_VNUM_ALTAR);
      if (!IS_SET (victim->act, PLR_JAILED))
      {
        do_transfer (victim, buf);
        if (++br_death_counter >= battle_royale_dead_amount)
        {
          do_restore (victim, "");
          br_death_counter = 0;
        }
      }
      for (sc = scents; sc; sc = sc_next)
      {
        sc_next = sc->next;
        if (sc->player == victim)
          extract_scent (sc);
      }
    }
    while (victim->affected)
      affect_remove (victim, victim->affected);
    victim->affected_by = race_table[victim->race].aff;
    if (battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner)))
    {
      OBJ_DATA *obj, *obj_next;
      long wear_long;
      for (obj = victim->carrying; obj != NULL; obj = obj_next)
      {
        obj_next = obj->next_content;
        if (obj->wear_loc == WEAR_NONE)
          continue;
        wear_long = obj->wear_loc;
        if (obj->affected)
        {
          unequip_char (victim, obj);
          equip_char_slot_new (victim, obj, wear_long);
        }
      }
    }
    if (!(battle_royale && (victim->level > 19 ||  (!IS_NPC(victim) && victim->pcdata->loner))))
    {
      for (i = 0; i < 4; i++)
        victim->armor[i] = 100;
    }
    victim->position = POS_RESTING;
    victim->hit = 1;
    victim->mana = 1;
    if (!(battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner))))
      victim->move = 1;

    else
      victim->move = -250;
    victim->pcdata->condition[COND_HUNGER] = 24;
    victim->pcdata->condition[COND_THIRST] = 24;
    victim->pcdata->condition[COND_DRUNK] = 0;

    // Shinowlan -- 6/25/98 Bug: #7 Death frees a jailed person 
    // this goes with fix in handler.c

    // Shinowlan -- 7/21/98 Added GRACE_WANTED constant to merc.h
    // This constant controls the number of times a player can be wanted
    // before wanted flags persist (are not removed) when a player dies.
    if (!IS_NPC (victim) && (victim->times_wanted < GRACE_WANTED)
      && !IS_SET (victim->act, PLR_JAILED))
      remove_wanted (victim);

    //Iblis 6/28/03
    //Remove all aggressor stuff between the two chars
    remove_aggressor (killer, victim);
    remove_aggressor (victim, killer);
    save_char_obj (victim);
    return;
}

void port_kill (CHAR_DATA * victim)
{
  int i;
  MEMORY_DATA *memd, *memd_next;
  OBJ_DATA *obj, *obj_next;
  stop_fighting (victim, TRUE);
  for (memd = memories; memd; memd = memd_next)
  {
    memd_next = memd->next;
    if (memd->player == victim || memd->mob == victim)
      extract_memory (memd);
  }
  for (obj = victim->carrying; obj; obj = obj_next)
  {
    obj_next = obj->next_content;
    unequip_char (victim, obj);
    extract_obj (obj);
  }
  if (IS_NPC (victim))
  {
    victim->pIndexData->killed++;
    kill_table[URANGE (0, victim->level, MAX_LEVEL - 1)].killed++;
    if (victim->mount != NULL)
    {
      victim->mount->riders = NULL;
      victim->mount->mounter = NULL;
      victim->mount = NULL;
      victim->position = POS_STANDING;
    }
    if (victim->riders != NULL)
    {
      send_to_char ("Alas, your trusty mount has died.\n\r",
        victim->riders);
      victim->riders->mount = NULL;
      victim->riders->position = POS_STANDING;
    }
    extract_char (victim, TRUE);
    return;
  }
  if (IS_SET(victim->act2,ACT_PUPPET)) {
  if (is_shapechanged_druid(victim))
	shapechange_return(victim, 2);
  else
    switched_return(victim,victim->in_room);
  }
  extract_char (victim, FALSE);
  while (victim->affected)
    affect_remove (victim, victim->affected);
  victim->affected_by = race_table[victim->race].aff;
  for (i = 0; i < 4; i++)
    victim->armor[i] = 100;
  victim->position = POS_RESTING;
  victim->hit = 1;
  victim->mana = 1;
  victim->move = 1;
  victim->pcdata->condition[COND_HUNGER] = 24;
  victim->pcdata->condition[COND_THIRST] = 24;
  save_char_obj (victim);
  return;
}


//Iblis 6/29/03 - Made it boolean to fix Avatar reaver bug..
//It returns TRUE if an avatar reaver died, false otherwise
bool group_gain (CHAR_DATA * ch, CHAR_DATA * victim)
{

  CHAR_DATA *gch;
  CHAR_DATA *lch;
  int xp;
  int num_npcs = 0, num_pcs = 0;
  int npc_group_levels = 0;
  int group_levels = 0;
  float grouplevels = 0;
  int highest_npc_level = 0;
  bool cardkill = FALSE;
  char buf[MAX_STRING_LENGTH];
  OBJ_INDEX_DATA *objid;
  OBJ_DATA *obj;

  /*
  * Monsters don't get kill xp's or alignment changes.
  * P-killing doesn't help either.
  * Dying of mortal wounds or poison doesn't give xp to anyone!
  */
  if (victim == ch)
    return FALSE;
  if (!IS_NPC(ch) && IS_SET(ch->act,PLR_CARDKILL))
  {
    int value=0;
    cardkill = TRUE;
    if (IS_NPC(victim) && !IS_SET(victim->act,ACT_PET) && !IS_SET(victim->act2,ACT_FAMILIAR) && victim->pIndexData->vnum != MOB_VNUM_VIVIFY)
    {
      if(victim->pIndexData->card_vnum != 0 && (objid = get_obj_index(victim->pIndexData->card_vnum)))
      {
        switch(objid->rarity)
        {
        case RARITY_ALWAYS: value = 110;break;
        case RARITY_COMMON: value = 75;break;
        case RARITY_SOMEWHAT_COMMON: value = 50;break;
        case RARITY_UNCOMMON: value = 25; break;
        case RARITY_RARE: value = 10; break;
        case RARITY_VERY_RARE: value = 3; break;
        default : value = 110;
        }

        if (number_percent() <= value)
          //It pops
        {
          obj = create_object(objid,0);
          obj_to_char(obj,ch);
          act("You take a small card from the corpse.",ch,obj,NULL,TO_CHAR);
          act("$n takes a small card from the corpse.",ch,obj,NULL,TO_ROOM);
        }
      }
      else if (number_percent() <= 80 || ch->Class == PC_CLASS_CHAOS_JESTER)
        //A commong card pops
      {
        if (number_percent() >  (1 + (victim->level*(39/90.0)) ) )
        {
          obj = create_object(get_obj_index(OBJ_VNUM_COMMON_CARD),0);
          //Random Element
          obj->value[11] = number_range(0,4);
          //Random Offence
          value = 1 + (victim->level/10) + ((victim->level > 80)?1:0) + ((victim->level == 100)?1:0);
          obj->value[8] = number_range(0,value);
          //Random Defence
          obj->value[9] = value - obj->value[8];
          if (obj->value[9] > 9)
          {
            obj->value[8] += obj->value[9] - 9;
            obj->value[9] = 9;
          }
          if (obj->value[8] > 9)
          {
            value += obj->value[8] - 9;
            obj->value[8] = 9;
          }

          //Nemesis
          if (number_percent() <= 5)
            obj->value[12] = number_range(1,9);
          //HP
          obj->value[7] = (victim->level/10) - 2;
          if (obj->value[7] < 1)
            obj->value[7] = 1; 
          if (victim->race == orace_lookup ("human")
            || victim->race ==orace_lookup ("elf")
            || victim->race ==orace_lookup ("vro'ath")
            || victim->race ==orace_lookup ("dwarf")
            || victim->race ==orace_lookup ("syvin")
            || victim->race ==orace_lookup ("litan")
            || victim->race ==orace_lookup ("orc")
            || victim->race ==orace_lookup ("goblin")
            || victim->race ==orace_lookup ("hobgoblin"))
            obj->value[10] = CARD_RACE_HUMANOID;
          else if (victim->race ==orace_lookup ("lich")
            || victim->race ==orace_lookup ("kalian"))
            obj->value[10] = CARD_RACE_UNDEAD;
          else if (victim->race ==orace_lookup ("dragon")
            || victim->race ==orace_lookup ("lizard"))
            obj->value[10] = CARD_RACE_DRAGON;
          else if (victim->race ==orace_lookup ("sidhe")
            || victim->race ==orace_lookup ("unique")
            || victim->race ==orace_lookup ("avatar"))
            obj->value[10] = CARD_RACE_MAGIC;
          else if (victim->race ==orace_lookup ("cloudgiant")
            || victim->race ==orace_lookup ("school monster")
            || victim->race ==orace_lookup ("troll")
            || victim->race ==orace_lookup ("giant")
            || victim->race ==orace_lookup ("modron")
            || victim->race ==orace_lookup ("kobold"))
            obj->value[10] = CARD_RACE_MONSTER;
          else if (victim->race ==orace_lookup ("wyvern")
            || victim->race ==orace_lookup ("nerix")
            || victim->race ==orace_lookup ("song bird")
            || victim->race ==orace_lookup ("bat"))
            obj->value[10] = CARD_RACE_BIRD;
          else if (victim->race ==orace_lookup ("swarm")
            || victim->race ==orace_lookup ("centipede"))
            obj->value[10] = CARD_RACE_INSECT;
          else if (victim->race ==orace_lookup ("canthi")
            || victim->race ==orace_lookup ("water fowl")
            || victim->race == orace_lookup ("fish"))
            obj->value[10] = CARD_RACE_FISH;
          else obj->value[10] = CARD_RACE_ANIMAL;

          if (IS_SET(victim->act,ACT_UNDEAD))
            obj->value[10] = CARD_RACE_UNDEAD;
          switch (obj->value[11])
          {
          default: sprintf(buf,"`h[`oCard`h] `h[`aMob`h] `a[`h0`a] `a[``%s`a]``",victim->short_descr);break;
          case CARD_ELEMENT_WATER: sprintf(buf,"`h[`oCard`h] `h[`aMob`h] `a[`eW`a] `a[``%s`a]``",victim->short_descr);break;
          case CARD_ELEMENT_LIGHTNING: sprintf(buf,"`h[`oCard`h] `h[`aMob`h] `a[`kL`a] `a[``%s`a]``",victim->short_descr);break;
          case CARD_ELEMENT_FIRE: sprintf(buf,"`h[`oCard`h] `h[`aMob`h] `a[`iF`a] `a[``%s`a]``",victim->short_descr);break;
          case CARD_ELEMENT_STONE: sprintf(buf,"`h[`oCard`h] `h[`aMob`h] `a[`dS`a] `a[``%s`a]``",victim->short_descr);break;
          }
          free_string(obj->short_descr);
          obj->short_descr = str_dup(buf);
          sprintf(buf,"card %s",victim->name);
          free_string(obj->name);
          obj->name = str_dup(buf);
          obj_to_char(obj,ch);
          act("You take a small card from the corpse.",ch,obj,NULL,TO_CHAR);
          act("$n takes a small card from the corpse.",ch,obj,NULL,TO_ROOM);
        }

        //rune cards
        else
        {
          int random_number=0, counter=0, total_cards=0;
          RUNE_CARD_DATA *rc;
          for (rc = rune_card_list;rc != NULL;rc = rc->next)
            ++total_cards;
          random_number = number_range(1,total_cards);
          counter = 0;
          //  do_echo(ch,"`lA rune card should have popped.``");
          for (rc = rune_card_list;rc != NULL;rc = ((rc==NULL)?rune_card_list:rc->next))
          {
            //		      send_to_char("!",ch);
            if (++counter == random_number)
            {
              if (!(objid = get_obj_index(rc->vnum)))
              {
                bug("Error, vnum %d on rune card list does not exist!",rc->vnum);
                break;
              }
              switch(objid->rarity)
              {
              case RARITY_ALWAYS: value = 46;break;
              case RARITY_COMMON: value = 25;break;
              case RARITY_SOMEWHAT_COMMON: value = 15;break;
              case RARITY_UNCOMMON: value = 10; break;
              case RARITY_RARE: value = 3; break;
              case RARITY_VERY_RARE: value = 1; break;
              default : value = 46;
              }
              if (number_percent() <= value && ch->level >= objid->level)
              {
                obj = create_object(get_obj_index(rc->vnum),0);
                obj_to_char(obj,ch);
                //  do_echo(ch,"`kA rune card DID pop!``");
                act("You take a small card from the corpse.",ch,obj,NULL,TO_CHAR);
                act("$n takes a small card from the corpse.",ch,obj,NULL,TO_ROOM);
                break;
              }
              //Restart the loop if no card is given
              else 
              {
                random_number = number_range(1,total_cards);
                counter = 0;
                rc = NULL;
                //		      send_to_char("#",ch);
              }
            }
          }
        }	    
      }
    }
  }

  // run through the list of members in the group, counting the members
  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
  {

    // Mathew 4/12/99 -- fix group membership so that mounts and
    // uncharmed npcs no longer effect experience calculation
    if (is_same_group (gch, ch) && !IS_NPC (gch))
    {

      num_pcs++;
      group_levels += gch->level;
    }
    //IBLIS 5/18/03
    //grouplevels is used to see if groupxp should be incremented
    grouplevels = group_levels;

    //IBLIS 5/18/03 - New Charmie Penalty
    if (is_same_group (gch, ch) && IS_NPC (gch)
      && !IS_SET (gch->act, ACT_PET) && !IS_SET (gch->act2, ACT_FAMILIAR)
      && gch->pIndexData->vnum != MOB_VNUM_VIVIFY)
    {
      num_npcs++;
      if (num_npcs > 1)
      {
        if (gch->level > highest_npc_level)
        {
          npc_group_levels = npc_group_levels + highest_npc_level;
          highest_npc_level = gch->level;
        }

        else
          npc_group_levels += gch->level;
      }

      else
        highest_npc_level = gch->level;
    }
  }

  lch = (ch->leader != NULL) ? ch->leader : ch;
  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
  {
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *stone;
    if (!is_same_group (gch, ch) || IS_NPC (gch))
      continue;
    if (!IS_NPC (victim) && gch->level - victim->level > 10
      && gch == ch && /*gch->aggres == NULL && */ !is_warring (gch->clan,
      victim->
      clan)
      && !IS_CLASS (victim, PC_CLASS_ASSASSIN) && !(battle_royale && 
      (gch->level > 19 || (!IS_NPC(gch) && gch->pcdata->loner)))
      && IS_AGGRESSOR (gch, victim) && !IS_IMMORTAL(gch))
    {

      send_to_char ("`bYou LOSE a level!``\n\r", gch);

      //IBLIS 5/19/03 - Make sure their total exp is docked for this
      gch->pcdata->totalxp = gch->pcdata->totalxp - level_cost (gch->level);

      decrease_level (gch);
      continue;
    }
    //IBLIS 5/29/03 - Group xp was too damn low.  Made it much better
    if (!IS_NPC (victim))
    {
      stone = get_eq_char (gch, WEAR_HOLD);
      if ((stone == NULL || (stone->pIndexData->vnum != OBJ_VNUM_RUNE_CONV))
        && !(IS_CLASS (gch, PC_CLASS_ASSASSIN)))
        xp = 1;

      else
      {

        //IBLIS 5/19/03 - If you are holding a rune (or are an ass) and you kill a real player,
        //you get 1/4th of the xp they lost, minus group penalities, EVEN
        //if the rest of your group are charmies OR they aren't holding runes
        xp = victim->exp / 5.0;
        if (xp > level_cost (victim->level))
          xp = level_cost (victim->level) * .20;
        xp /= 4;
      }
    }

    else
    {
      xp = victim->max_hit * victim->level;
      xp = add_xp_bonuses (xp, victim);
    }
    if (!(battle_royale && (gch->level > 19 || (!IS_NPC(gch) && gch->pcdata->loner))))
    {
      if (group_levels - gch->level > 0)
      {
        xp -=
          xp *
          (((group_levels - gch->level) / (gch->level * 1.0)) * .10);
        if (xp < ((victim->max_hit * victim->level) * .50))
          xp = (victim->max_hit * victim->level) * .50;
      }
      xp -= xp * ((npc_group_levels / (gch->level * 1.0)) * .29);
      if (IS_NPC (victim)
        && xp < ((victim->max_hit * victim->level) * .11))
        xp = ((victim->max_hit * victim->level) * .11);
      if (victim->pIndexData != NULL
        && ((victim->pIndexData)->affected_by & AFF_SANCTUARY))
        xp = xp * 1.5;
      if (xp < 1)
        xp = 1;
      if (quad)
        xp = xp * 4;
      if (ch == gch && !IS_NPC(gch) && gch->pcdata->quest_mob == victim)
      {
        if (ch->pcdata->qtype == 3)
          xp = xp * 6;
        else
          xp = xp * 4;
      }
      if (doub)
        xp = xp * 2;
      if (half)
        xp = xp / 2;
      if (lch == gch)
        xp +=  xp*(.02*num_pcs);
      if (gch->race == PC_RACE_NIDAE && 
        (gch->in_room->sector_type == SECT_UNDERWATER
        || gch->in_room->sector_type == SECT_WATER_OCEAN))
        xp = xp * 2;

      if (xp > ((level_cost (gch->level)) / 10.0))
        xp = level_cost (gch->level) * .10;
      if (!IS_NPC (victim) && xp == 1)
        xp = 0;

      if ((gch->clan != CLAN_BOGUS)
        && (gch->clan == gch->in_room->area->clan)
        && (gch->in_room->area->points != 0))
      {
        xp += .07 * xp;
      }
      if (IS_SET(victim->act2,ACT_FAMILIAR))
        xp = 0;
      if (IS_NPC(gch) || (!IS_SET(gch->act,PLR_CARDKILL) && !cardkill))
      {
        xp = gain_exp (gch, xp);
        if (grouplevels > gch->level)
          group_gain_exp (gch, xp);
      }

      // Minax--If a reaver goes good, he's fucked!
    }

    if (!(!IS_NPC(victim) && !IS_NPC(gch) && battle_royale))
      align_compute (gch, victim);

    if (IS_CLASS (gch, PC_CLASS_REAVER) && gch->alignment > 0
      && (get_eq_char (gch, WEAR_WIELD_L) !=
      NULL || get_eq_char (gch, WEAR_WIELD_R) != NULL))
    {
      act ("`iYou are struck down as a traitor by the God of evil!``",
        gch, NULL, NULL, TO_CHAR);
      act ("`i$n is struck down as a traitor by the God of evil!``",
        gch, NULL, NULL, TO_ROOM);
      raw_kill (gch, gch);
      return TRUE;
    }
    for (obj = gch->carrying; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      if (obj->wear_loc == WEAR_NONE)
        continue;
      if ((gch->race != PC_RACE_AVATAR)
        && ((IS_OBJ_STAT (obj, ITEM_ANTI_EVIL) && IS_EVIL (gch))
        || (IS_OBJ_STAT (obj, ITEM_ANTI_GOOD) && IS_GOOD (gch))
        || (IS_OBJ_STAT (obj, ITEM_ANTI_NEUTRAL)
        && IS_NEUTRAL (gch))))
      {
        act ("You are zapped by $p.", gch, obj, NULL, TO_CHAR);
        act ("$n is zapped by $p.", gch, obj, NULL, TO_ROOM);
        obj_from_char (obj);
        obj_to_room (obj, gch->in_room);
      }
    }
  }
  return FALSE;
}

long add_xp_bonuses (long xp, CHAR_DATA * victim)
{
  int ac_total;

  if (IS_SET (victim->act, ACT_AGGRESSIVE))
    xp += xp * .05;
  if (IS_SET (victim->act, ACT_REMEMBER))
    xp += xp * .05;
  if (IS_SET (victim->act, ACT_HUNT))
    xp += xp * .05;
  if (IS_SET (victim->act, ACT_WIMPY))
    xp += xp * .05;
  if (IS_SET (victim->act, ACT_SMART_HUNT))
    xp += xp * .07;
  if (IS_SET (victim->act, ACT_LOOTER))
    xp += xp * .01;
  if (IS_SET (victim->act, ACT_RANGER))
    xp += xp * .05;
  if (IS_SET (victim->act, ACT_BARD))
    xp += xp * .05;
  if (IS_SET (victim->act, ACT_CLERIC))
    xp += xp * .05;
  if (IS_SET (victim->act, ACT_MAGE))
    xp += xp * .05;
  if (IS_SET (victim->act, ACT_THIEF))
    xp += xp * .05;
  if (IS_SET (victim->act, ACT_WARRIOR))
    xp += xp * .05;
  if (IS_SET (victim->act2, ACT_MONK))
    xp += xp * .30;
  if (IS_SET (victim->act, ACT_UNDEAD))
    xp += xp * .03;
  if (IS_SET (victim->off_flags, OFF_AREA_ATTACK))
    xp += xp * .20;
  if (IS_SET (victim->off_flags, OFF_BACKSTAB))
    xp += xp * .02;
  if (IS_SET (victim->off_flags, OFF_BASH))
    xp += xp * .05;
  if (IS_SET (victim->off_flags, OFF_BERSERK))
    xp += xp * .02;
  if (IS_SET (victim->off_flags, OFF_DISARM))
    xp += xp * .05;
  if (IS_SET (victim->off_flags, OFF_DODGE))
    xp += xp * .05;
  if (IS_SET (victim->off_flags, OFF_FADE))
    xp += xp * .20;
  if (victim->number_of_attacks != -1)
    xp += xp * (.05*victim->number_of_attacks);
  else if (IS_SET (victim->off_flags, OFF_FAST))
    xp += xp * .03;
  if (IS_SET (victim->off_flags, OFF_KICK))
    xp += xp * .03;
  if (IS_SET (victim->off_flags, OFF_KICK_DIRT))
    xp += xp * .03;
  if (IS_SET (victim->off_flags, OFF_PARRY))
    xp += xp * .05;
  if (IS_SET (victim->off_flags, OFF_TRIP))
    xp += xp * .05;
  if (IS_SET (victim->off_flags, OFF_CHARGE))
    xp += xp * .01;
  if (IS_SET (victim->off_flags, OFF_SWEEP))
    xp += xp * .01;
  if (IS_SET (victim->off_flags, OFF_THROWING))
    xp += xp * .05;
  if (IS_SET (victim->off_flags, OFF_TRAMPLE))
    xp += xp * .01;
  if (IS_SET (victim->imm_flags, IMM_WEAPON))
    xp += xp * 1.00;
  if (IS_SET (victim->imm_flags, IMM_MAGIC))
    xp += xp * .15;
  if (IS_SET (victim->imm_flags, IMM_BASH))
    xp += xp * .10;
  if (IS_SET (victim->imm_flags, IMM_PIERCE))
    xp += xp * .10;
  if (IS_SET (victim->imm_flags, IMM_SLASH))
    xp += xp * .10;
  if (IS_SET (victim->res_flags, RES_WEAPON))
    xp += xp * .50;
  if (IS_SET (victim->res_flags, RES_MAGIC))
    xp += xp * .05;
  if (IS_SET (victim->res_flags, RES_BASH))
    xp += xp * .05;
  if (IS_SET (victim->res_flags, RES_PIERCE))
    xp += xp * .05;
  if (IS_SET (victim->res_flags, RES_SLASH))
    xp += xp * .05;
  ac_total =
    victim->armor[0] + victim->armor[1] + victim->armor[2] + victim->armor[3];
  if (ac_total < 0)
    xp += xp * .01 * (abs ((ac_total)) / 20);
  xp +=
    xp *
    (((((victim->damage[0] * victim->damage[1]) +
    victim->damage[0]) / 2) + victim->damage[2]) / 100);
  return xp;
}

int get_affluence (CHAR_DATA * ch)
{
  int total = 0;
  AREA_DATA *pArea;
  for (pArea = area_first; pArea; pArea = pArea->next)
  {
    if ((ch->clan != CLAN_BOGUS) && (pArea->clan == ch->clan))
      total += pArea->points;
  }
  return (total);
}


/*
* Compute xp for a kill.
*/

//IBLIS 5/20/03 - Calculates Alignment
int align_compute (CHAR_DATA * gch, CHAR_DATA * victim)
{

  //
  // alignment calculation
  //

  // Minax new alignment system
  int alignment=0;
  int align_cutter=0;  // 0 = nothing, 1 = 1/2 alignment change, 2 = 1/3 alignment change, 3 = no alignment change
  OBJ_DATA *wield;
  if (gch->race == PC_RACE_SIDHE || IS_SET(gch->act2, ACT_FAMILIAR) || gch->race == PC_RACE_NERIX || IS_SET(victim->act2,ACT_NO_ALIGN))
    return 0;
  if ((wield = get_eq_char (gch, WEAR_WIELD_L)) != NULL ||
    (wield = get_eq_char (gch, WEAR_WIELD_R)) != NULL)
  {
    if (IS_WEAPON_STAT(wield, WEAPON_APATHY))
    {
      if (gch->level < 31)
        align_cutter = 1;
      else if (gch->level < 61)
        align_cutter = 2;
      else align_cutter = 3;
    }
  }

  if (gch->alignment > 0)	// Player is good
  {
    if (victim->alignment == 0)
      alignment = 0 - ((abs (gch->alignment) * victim->level) / 200);

    else if (victim->alignment < 0)
    {
      if (abs (victim->alignment) > abs (gch->alignment))
        alignment =
        abs ((abs (gch->alignment) -
        abs (victim->alignment) * victim->level)) / 200;

      else if (abs (victim->alignment) < abs (gch->alignment))
        alignment =
        0 -
        (abs
        ((abs (gch->alignment) -
        abs (victim->alignment) * victim->level)) / 200);
    }

    else if (victim->alignment > 0)
    {
      if (abs (victim->alignment) > abs (gch->alignment))
        alignment =
        0 -
        (abs
        ((abs (gch->alignment) -
        abs (victim->alignment) * victim->level)) / 200);

      else if (abs (victim->alignment) < abs (gch->alignment))
        alignment =
        0 -
        (abs
        ((abs (gch->alignment) -
        abs (victim->alignment) * victim->level)) / 200);
    }
  }

  else if (gch->alignment == 0)
    alignment = 0 - ((victim->alignment * victim->level) / 200);

  else if (gch->alignment < 0)	// Player is evil
  {
    if (victim->alignment == 0)
      alignment = (abs (gch->alignment) * victim->level) / 200;

    else if (victim->alignment < 0)
    {
      if (abs (victim->alignment) > abs (gch->alignment))
        alignment =
        abs ((abs (gch->alignment) -
        abs (victim->alignment) * victim->level)) / 200;

      else if (abs (victim->alignment) < abs (gch->alignment))
        alignment =
        abs ((abs (gch->alignment) -
        abs (victim->alignment) * victim->level)) / 200;
    }

    else if (victim->alignment > 0)
    {
      if (abs (victim->alignment) > abs (gch->alignment))
        alignment =
        0 -
        (abs
        ((abs (gch->alignment) -
        abs (victim->alignment) * victim->level)) / 200);

      else if (abs (victim->alignment) < abs (gch->alignment))
        alignment =
        abs ((abs (gch->alignment) -
        abs (victim->alignment) * victim->level)) / 200;
    }
  }
  if (alignment > 8)
    alignment = 8;
  if (alignment < -8)
    alignment = -8;
  if (align_cutter == 1)
    alignment /= 2;
  else if (align_cutter == 2)
    alignment = alignment * (1.0/3.0);
  else if (align_cutter == 3)
    alignment = alignment * (1.0/5.0);
  change_alignment(gch,alignment);
  /*  gch->alignment += alignment;
  if (gch->alignment < -1000)
  gch->alignment = -1000;
  if (gch->alignment > 1000)
  gch->alignment = 1000;
  if (gch->race == PC_RACE_LICH && gch->alignment > -250)
  gch->alignment = -250;*/

  return (0);
}


/*
* Disarm a creature.
* Caller must check for successful attack.
*/
void disarm (CHAR_DATA * ch, CHAR_DATA * victim)
{
  OBJ_DATA *obj;
  if ((obj = get_eq_char (victim, WEAR_WIELD_R)) == NULL)
    if ((obj = get_eq_char (victim, WEAR_WIELD_L)) == NULL)
      return;
  if (IS_OBJ_STAT (obj, ITEM_NOREMOVE))
  {
    act ("Alas, you cannot pry $N's weapon away from $M!", ch, NULL,
      victim, TO_CHAR);
    act ("$n tries to disarm you, but your weapon won't budge!", ch,
      NULL, victim, TO_VICT);
    act ("$n tries to disarm $N, but fails.", ch, NULL, victim, TO_NOTVICT);
    return;
  }
  act ("`B$n DISARMS you and sends your weapon flying!``", ch, NULL,
    victim, TO_VICT);
  act ("`AYou disarm $N!``", ch, NULL, victim, TO_CHAR);
  act ("$n disarms $N!", ch, NULL, victim, TO_NOTVICT);
  obj_from_char (obj);
  if (IS_OBJ_STAT (obj, ITEM_NODROP) || IS_OBJ_STAT (obj, ITEM_INVENTORY) || (battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner))) || victim->dueler)
    obj_to_char (obj, victim);

  else
  {
    if (victim && !IS_NPC(victim))
      save_char_obj (victim);
    obj_to_room (obj, victim->in_room);
    if (IS_NPC (victim) && victim->wait == 0 && can_see_obj (victim, obj))
      get_obj (victim, obj, NULL);
  }
  return;
}

void do_berserk (CHAR_DATA * ch, char *argument)
{
  int chance, hp_percent;
  if ((chance = get_skill (ch, gsn_berserk)) == 0
    || ((IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_BERSERK) && !IS_SET (ch->act2, ACT_FAMILIAR))))
  {
    send_to_char ("You turn red in the face, but nothing happens.\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_BERSERK) || is_affected (ch, gsn_berserk)
    || is_affected (ch, skill_lookup ("frenzy")))
  {
    send_to_char ("You get a little madder.\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CALM)
    || is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char ("You are feeling too mellow to go berserk.\n\r", ch);
    return;
  }
  if (ch->mana < 50)
  {
    send_to_char ("You can't get up enough energy.\n\r", ch);
    return;
  }

  /* modifiers */

  /* fighting */
  if (ch->position == POS_FIGHTING || ch->fighting != NULL)
    chance += 10;

  /* damage -- below 50% of hp helps, above hurts */
  hp_percent = 100 * ch->hit / ch->max_hit;
  chance += 25 - hp_percent / 2;
  if (number_percent () < chance)
  {
    AFFECT_DATA af;
    WAIT_STATE (ch, PULSE_VIOLENCE);
    ch->mana -= 50;
    ch->move /= 2;

    /* heal a little damage */
    ch->hit += ch->level * 2;
    ch->hit = UMIN (ch->hit, ch->max_hit);
    send_to_char ("Your pulse races as you are consumed by rage!\n\r", ch);
    act ("$n gets a wild look in $s eyes.", ch, NULL, NULL, TO_ROOM);
    check_improve (ch, gsn_berserk, TRUE, 2);
    af.where = TO_AFFECTS;
    af.type = gsn_berserk;
    af.level = ch->level;
    af.permaff = FALSE;
    af.composition = FALSE;
    af.comp_name = str_dup ("");
    af.duration = number_fuzzy (ch->level / 8);
    if (is_affected (ch, skill_lookup ("Falkentynes Fury")))
      af.modifier = ch->level / 11;

    else
      af.modifier = ch->level / 7;
    af.bitvector = 0;
    af.location = APPLY_HITROLL;
    affect_to_char (ch, &af);
    af.location = APPLY_DAMROLL;
    affect_to_char (ch, &af);

    // Minax 6-29-02 Berserk improves saving vs. magic by the same
    // degree is improves hitroll and damroll
    if (is_affected (ch, skill_lookup ("Falkentynes Fury")))
      af.modifier = 0 - (ch->level / 11);

    else
      af.modifier = 0 - (ch->level / 7);
    af.location = APPLY_SAVING_SPELL;
    affect_to_char (ch, &af);
    if (is_affected (ch, skill_lookup ("Falkentynes Fury")))
      af.modifier = 5 * (ch->level / 10);

    else
      af.modifier = 10 * (ch->level / 10);
    af.location = APPLY_AC;
    affect_to_char (ch, &af);
  }

  else
  {
    WAIT_STATE (ch, 3 * PULSE_VIOLENCE);
    ch->mana -= 25;
    ch->move /= 2;
    send_to_char ("Your pulse speeds up, but nothing happens.\n\r", ch);
    check_improve (ch, gsn_berserk, FALSE, 2);
  }
}
void do_bash (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;
  int ti=0;
  one_argument (argument, arg);
  if ((chance = get_skill (ch, gsn_bash)) == 0
    || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_BASH)))
  {
    send_to_char ("Bashing? What's that?\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_entangle))
  {
    send_to_char ("You are too constrained to initiate combat.\n\r", ch);
    return;
  }

  if (is_affected (ch, skill_lookup("clasp")))
  {
    send_to_char ("Your feet are to restrained to do that.\n\r", ch);
    return;
  }
  if (is_affected (ch, skill_lookup ("web")))
  {
    send_to_char
      ("The webs binding you make movement difficult and bashing impossible.\n\r",
      ch);
    return;
  }
  if (arg[0] == '\0')
  {
    victim = ch->fighting;
    if (victim == NULL)
    {
      send_to_char ("But you aren't fighting anyone!\n\r", ch);
      return;
    }
  }

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (victim->position < POS_FIGHTING)
  {
    act ("You'll have to let $M get back up first.", ch, NULL, victim,
      TO_CHAR);
    return;
  }
  if (victim == ch)
  {
    send_to_char
      ("You try to bash your brains out, but fail, sadly enough.\n\r", ch);
    return;
  }
  if (ch->mount != NULL)
  {
    send_to_char ("You cannot bash while mounted.\n\r", ch);
    return;
  }
  if (victim->mount != NULL)
  {
    send_to_char ("You cannot bash someone who is mounted.\n\r", ch);
    return;
  }
  if (is_safe (ch, victim))
    return;
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
  {
    act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
    return;
  }

  /* modifiers */

  /* size  and weight
  chance += ch->carry_weight / 250;
  chance -= victim->carry_weight / 200;
  */
  if (ch->size < victim->size)
    chance += (ch->size - victim->size) * 15;

  else
    chance += (ch->size - victim->size) * 10;

  /* stats */
  chance += get_curr_stat (ch, STAT_STR);
  chance -= (get_curr_stat (victim, STAT_DEX) * 4) / 3;

  //Iblis 4/12/04 - New ac thingie for bash
  if (GET_AC(victim,AC_BASH) < 0)
    ti = -GET_AC (victim, AC_BASH) / 100;
  ti -= get_hitroll2 (ch)/12;
  if (ti > 0)
    chance -= ti*2;



  /* speed */
  if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
    chance += 10;
  if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
    chance -= 30;

  /* level */
  //  chance += (ch->level - victim->level);
  if (!IS_NPC (victim) && chance < get_skill (victim, gsn_dodge))
  {				/*
          act("$n tries to bash you, but you dodge it.",ch,NULL,victim,TO_VICT);
          act("$N dodges your bash, you fall flat on your face.",ch,NULL,victim,TO_CHAR);
          WAIT_STATE(ch,skill_table[gsn_bash].beats);
          return; */
    chance -= 3 * (get_skill (victim, gsn_dodge) - chance);
  }

  /* now the attack */
  if (number_percent () < chance && victim->race != PC_RACE_SWARM)
  {
    if (awareness (ch, victim))
      return;
    act ("`B$n sends you sprawling with a powerful bash!``", ch, NULL,
      victim, TO_VICT);
    if (!is_affected (victim, gsn_mirror))
    {
      act ("`AYou slam into $N, and send $M flying!``", ch, NULL,
        victim, TO_CHAR);
      act ("$n sends $N sprawling with a powerful bash.", ch, NULL,
        victim, TO_NOTVICT);
      check_improve (ch, gsn_bash, TRUE, 1);
    }
    WAIT_STATE (victim, 3 * PULSE_VIOLENCE);
    WAIT_STATE (ch, skill_table[gsn_bash].beats);
    victim->position = POS_RESTING;
    victim->bashed = TRUE;
    if (IS_NPC (victim))
      victim->color = 42;
    damage (ch, victim,
      number_range (2, 2 + 2 * ch->size + chance / 20),
      gsn_bash, DAM_BASH, FALSE);
    if (!is_affected (victim, gsn_mirror)
      && is_affected (victim, gsn_fireshield))
      if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
      {
        damage (victim, ch,
          number_range (2,
          2 + 2 * ch->size + chance / 20) / 2,
          0, DAM_FIRE, FALSE);
        act ("$N's scorching shield of flame burns you as you attack.",
          ch, NULL, victim, TO_CHAR);
        act ("$n is burnt by your scorching shield of flame.", ch, NULL,
          victim, TO_VICT);
      }
  }

  else
  {
    damage (ch, victim, 0, gsn_bash, DAM_BASH, FALSE);
    act ("You fall flat on your face!", ch, NULL, victim, TO_CHAR);
    act ("$n falls flat on $s face.", ch, NULL, victim, TO_NOTVICT);
    act ("You evade $n's bash, causing $m to fall flat on $s face.",
      ch, NULL, victim, TO_VICT);
    check_improve (ch, gsn_bash, FALSE, 1);
    ch->position = POS_RESTING;
    if (IS_NPC (ch))
      ch->color = 42;
    WAIT_STATE (ch, skill_table[gsn_bash].beats * 3 / 2);
  }
}

void do_charge (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim, *next_victim;
  int chance, dam;
  bool hits = FALSE;
  if ((chance = get_skill (ch, gsn_charge)) == 0
    || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_CHARGE)))
  {
    send_to_char ("Charging? What's that?\n\r", ch);
    return;
  }
  if (IS_SET (ch->in_room->room_flags, ROOM_SAFE))
  {
    send_to_char ("This room is safe from bullies like you.\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CALM) || is_affected (ch, skill_lookup
    ("Quiet Mountain Lake")))
  {
    send_to_char ("You are feeling too peaceful to do that now.\n\r", ch);
    return;
  }
  if (is_affected (ch, skill_lookup("clasp")))
  {
    send_to_char ("The skeletal hands prevent you from doing that.\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_entangle))
  {
    send_to_char ("You are too constrained to initiate combat.\n\r", ch);
    return;
  }
  if (is_affected (ch, skill_lookup ("web")))
  {
    send_to_char
      ("The webs binding you make movement difficult and charging impossible.\n\r",
      ch);
    return;
  }
  for (victim = ch->in_room->people; victim; victim = next_victim)
  {
    next_victim = victim->next_in_room;
    if (victim == ch)
      continue;
    if (victim->invis_level >= LEVEL_HERO)
      continue;
    if (is_same_group (ch, victim))
      continue;
    if (victim->position < POS_STANDING)
      continue;
    if (is_safe (ch, victim))
      continue;
    if (check_shopkeeper_attack (ch, victim))
      continue;
    if (victim->mount != NULL && ch->mount == NULL)
      continue;
    if (IS_NPC(ch) && IS_NPC(victim))
      continue;

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;
    if (ch->size < victim->size)
      chance += (ch->size - victim->size) * 15;

    else
      chance += (ch->size - victim->size) * 10;

    /* stats */
    chance += get_curr_stat (ch, STAT_STR);
    chance -= (get_curr_stat (victim, STAT_DEX) * 4) / 3;

    //Iblis 4/12/04 - New ac thingie for bash
    /*      if (GET_AC(victim,AC_BASH) < 0)
    ti = -GET_AC (victim, AC_BASH) / 100;
    ti -= get_hitroll (ch)/12;
    if (ti > 0)
    chance -= ti*2;*/

    /* speed */
    if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
      chance += 10;
    if (IS_SET (victim->off_flags, OFF_FAST)
      || IS_AFFECTED (victim, AFF_HASTE))
      chance -= 30;

    /* level */
    chance += (ch->level - victim->level);
    if (!IS_NPC (victim) && chance < get_skill (victim, gsn_dodge))
      chance -= 3 * (get_skill (victim, gsn_dodge) - chance);

    if (chance < 0)
      chance = 0;
    else if (chance > 100)
      chance = 100;
    /* now the attack */
    if (number_percent () < chance)
    {
      if (awareness (ch, victim))
        continue;
      if (!is_affected (victim, gsn_mirror))
      {
        act ("`B$n's charge sends you to the ground!``", ch, NULL,
          victim, TO_VICT);
        act ("`AYou charge into $N, and send $M to the ground!``",
          ch, NULL, victim, TO_CHAR);
        act ("$n's charge leaves $N floundering on the ground.",
          ch, NULL, victim, TO_NOTVICT);
        check_improve (ch, gsn_charge, TRUE, 1);
        hits = TRUE;
      }
      dam = (number_range (2, 2 + 2 * ch->size + chance / 20) * 5);
      if (victim->position < POS_FIGHTING)
        dam += dam / 2;
      if (dam < 0)
        dam = 1;
      else if (dam > 200)
        dam = 200;
      damage (ch, victim, dam, gsn_charge, DAM_BASH, FALSE);
      victim->position = POS_RESTING;
      if (IS_NPC (victim))
        victim->color = 42;
      WAIT_STATE (victim, 3 * PULSE_VIOLENCE);
      if (!is_affected (victim, gsn_mirror)
        && is_affected (victim, gsn_fireshield))
        if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
        {
          damage (victim, ch,
            number_range (2,
            2 + 2 * ch->size +
            chance / 20) / 2, 0, DAM_FIRE, FALSE);
          act
            ("$N's scorching shield of flame burns you as you attack.",
            ch, NULL, victim, TO_CHAR);
          act ("$n is burnt by your scorching shield of flame.", ch,
            NULL, victim, TO_VICT);
        }
    }
  }
  if (!hits)
    send_to_char ("You missed everyone.\n\r", ch);
  WAIT_STATE (ch, skill_table[gsn_charge].beats);
}

void do_sweep (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim, *next_victim;
  int chance;
  if ((chance = get_skill (ch, gsn_sweep)) == 0
    || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_SWEEP)))
  {
    send_to_char ("Sweeping? What's that?\n\r", ch);
    return;
  }
  if (IS_SET (ch->in_room->room_flags, ROOM_SAFE))
  {
    send_to_char ("This room is safe from bullies like you.\n\r", ch);
    return;
  }
  if (ch->mount != NULL)
  {
    send_to_char ("You can't perform a sweep while mounted.\n\r", ch);
    return;
  }
  if (is_affected (ch, skill_lookup("clasp")))
  {
    send_to_char ("Your feet are to restrained to do that.\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_entangle))
  {
    send_to_char ("You are too constrained to initiate combat.\n\r", ch);
    return;
  }
  if (is_affected (ch, skill_lookup ("web")))
  {
    send_to_char
      ("The webs binding you make movement difficult and sweeping impossible.\n\r",
      ch);
    return;
  }
  WAIT_STATE (ch, skill_table[gsn_sweep].beats);
  send_to_char("You sweep your legs around the room, trying to hit everyone.\n\r",ch);
  for (victim = ch->in_room->people; victim; victim = next_victim)
  {
    next_victim = victim->next_in_room;
    if (victim == ch || is_same_group (ch, victim))
      continue;
    if (is_safe (ch, victim))
      continue;
    if (awareness (ch, victim))
      continue;
    if (victim->mount != NULL)
      continue;
    if (check_shopkeeper_attack (ch, victim))
      return;
    if (IS_NPC(ch) && IS_NPC(victim))
      continue;

    if (number_percent () < get_skill (ch, gsn_sweep))
    {
      check_improve (ch, gsn_sweep, TRUE, 1);
      if (number_percent () < get_skill (ch, gsn_kick))
        damage (ch, victim, number_range (1, ch->level), gsn_sweep,
        DAM_BASH, TRUE);

      else
        damage (ch, victim, UMAX ((ch->level / 4), ch->level),
        gsn_sweep, DAM_BASH, TRUE);
      if (!is_affected (victim, gsn_mirror)
        && is_affected (victim, gsn_fireshield))
        if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
        {
          damage (victim, ch,
            number_range (2,
            2 + 2 * ch->size +
            chance / 20) / 2, 0, DAM_FIRE, FALSE);
          act
            ("$N's scorching shield of flame burns you as you attack.",
            ch, NULL, victim, TO_CHAR);
          act ("$n is burnt by your scorching shield of flame.", ch,
            NULL, victim, TO_VICT);
        }
    }
  }
}

void do_dirt (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;
  if (ch->in_room->sector_type == SECT_UNDERWATER
    || ch->in_room->sector_type == SECT_WATER_OCEAN
    || ch->in_room->sector_type == SECT_WATER_NOSWIM
    || ch->in_room->sector_type == SECT_WATER_SWIM)
  {
    send_to_char ("You kick your feet about madly.\n\r", ch);
    return;
  }
  one_argument (argument, arg);
  if ((chance = get_skill (ch, gsn_dirt)) == 0
    || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_KICK_DIRT)))
  {
    send_to_char ("You get your feet dirty.\n\r", ch);
    return;
  }
  if (arg[0] == '\0')
  {
    victim = ch->fighting;
    if (victim == NULL)
    {
      send_to_char ("But you aren't in combat!\n\r", ch);
      return;
    }
  }

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (IS_AFFECTED (victim, AFF_BLIND))
  {
    act ("$E's already been blinded.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("Very funny.\n\r", ch);
    return;
  }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (is_safe (ch, victim))
    return;
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
  {
    act ("But $N is such a good friend!", ch, NULL, victim, TO_CHAR);
    return;
  }

  /* modifiers */

  /* dexterity */
  chance += get_curr_stat (ch, STAT_DEX);
  chance -= 2 * get_curr_stat (victim, STAT_DEX);

  /* speed  */
  if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
    chance += 10;
  if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
    chance -= 25;

  /* level */
  chance += (ch->level - victim->level) * 2;

  /* sloppy hack to prevent false zeroes */
  if (chance % 5 == 0)
    chance += 1;

  /* terrain */
  switch (ch->in_room->sector_type)
  {
  case (SECT_INSIDE):
    chance -= 20;
    break;
  case (SECT_CITY):
    chance -= 10;
    break;
  case (SECT_FIELD):
    chance += 5;
    break;
  case (SECT_FOREST):
    break;
  case (SECT_HILLS):
    break;
  case (SECT_MOUNTAIN):
    chance -= 10;
    break;
  case (SECT_WATER_SWIM):
    chance = 0;
    break;
  case (SECT_WATER_NOSWIM):
    chance = 0;
    break;
  case (SECT_AIR):
    chance = 0;
    break;
  case (SECT_DESERT):
    chance += 10;
    break;
  }
  if (chance == 0)
  {
    send_to_char ("There isn't any dirt to kick.\n\r", ch);
    return;
  }
  if (is_affected (victim, gsn_mirror))
    victim = ch;

  /* now the attack */
  if (number_percent () < chance)
  {
    AFFECT_DATA af;
    act ("$n is blinded by the dirt in $s eyes!", victim, NULL, NULL,
      TO_ROOM);
    if (victim != ch)
      act ("$n kicks dirt in your eyes!", ch, NULL, victim, TO_VICT);

    else
      act ("The dirt is reflected back into your eyes!", ch, NULL,
      victim, TO_VICT);
    damage (ch, victim, number_range (2, 5), gsn_dirt, DAM_NONE, FALSE);
    send_to_char ("You can't see a thing!\n\r", victim);
    check_improve (ch, gsn_dirt, TRUE, 2);
    WAIT_STATE (ch, skill_table[gsn_dirt].beats);
    af.where = TO_AFFECTS;
    af.type = gsn_dirt;
    af.level = ch->level;
    af.duration = 0;
    af.location = APPLY_HITROLL;
    af.modifier = -4;
    af.permaff = FALSE;
    af.bitvector = AFF_BLIND;
    af.composition = FALSE;
    af.comp_name = str_dup ("");
    affect_to_char (victim, &af);
  }

  else
  {
    damage (ch, victim, 0, gsn_dirt, DAM_NONE, TRUE);
    check_improve (ch, gsn_dirt, FALSE, 2);
    WAIT_STATE (ch, skill_table[gsn_dirt].beats);
  }
}

void do_redirect (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  one_argument (argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char ("Redirect at whom?\n\r", ch);
    return;
  }
  if (ch->fighting == NULL)
  {
    send_to_char ("But you aren't fighting anyone.\n\r", ch);
    return;
  }
  if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("You aren't as flexible as you think.\n\r", ch);
    return;
  }
  if (victim->fighting != ch)
  {
    act ("But you aren't fighting $N.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (ch->fighting == victim)
  {
    act ("You're already aiming for $N.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (number_percent () > get_skill (ch, gsn_redirect))
  {
    send_to_char ("You failed.\n\r", ch);
    check_improve (ch, gsn_redirect, FALSE, 1);
    return;
  }
  send_to_char
    ("Deftly repositioning yourself, you redirect your combat efforts successfully.\n\r",
    ch);
  ch->fighting = victim;
  check_improve (ch, gsn_redirect, TRUE, 1);
}

void do_throw (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim = NULL, *vch, *vfightnow = NULL, *cfightnow = NULL;
  char throw_weapon[MAX_INPUT_LENGTH];
  OBJ_DATA *obj, *tobj;
  short x, chance, door = 0, hands = 0;
  if (IS_NPC (ch) && IS_SET (ch->off_flags, OFF_THROWING))
    chance = 90;

  else
    chance = get_skill (ch, gsn_throwing);
  if (chance <= 0)
  {
    send_to_char ("Throwing? What's that?\n\r", ch);
    return;
  }
  argument = one_argument (argument, throw_weapon);
  if (throw_weapon[0] == '\0')
  {
    send_to_char ("Throw what at whom?\n\r", ch);
    return;
  }
  if ((obj = get_obj_carry (ch, throw_weapon)) == NULL)
  {
    send_to_char ("You don't seem to be carrying that weapon.\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CALM) || is_affected (ch, skill_lookup
    ("Quiet Mountain Lake")))
  {
    send_to_char ("You are feeling too peaceful to do that now.\n\r", ch);
    return;
  }
  if (obj->item_type != ITEM_WEAPON)
  {
    act ("$p is not a weapon.", ch, obj, NULL, TO_CHAR);
    return;
  }
  for (tobj = ch->carrying; tobj; tobj = tobj->next_content)
  {
    if ((tobj->wear_loc == WEAR_WIELD_R)
      || (tobj->wear_loc == WEAR_WIELD_L) || (tobj->wear_loc ==
      WEAR_HOLD)
      || (tobj->wear_loc == WEAR_SHIELD))
    {
      if ((tobj->item_type == ITEM_WEAPON
        && (IS_WEAPON_STAT (tobj, WEAPON_TWO_HANDS) && ch->size < SIZE_LARGE))
        || (tobj->item_type == ITEM_INSTRUMENT
        && IS_SET (tobj->value[0], A)))
        hands += 2;

      else
        hands++;
    }
  }
  if (hands > 1)
  {
    act ("You have no free hand with which to throw $p.", ch, obj,
      NULL, TO_CHAR);
    return;
  }
  if (!IS_SET (obj->value[4], WEAPON_THROWING))
  {
    act ("$p is not suitable for throwing.", ch, obj, NULL, TO_CHAR);
    return;
  }
  if (IS_SET (ch->in_room->room_flags, ROOM_SAFE))
  {
    send_to_char ("You cannot throw weapons in a safe room.\n\r", ch);
    return;
  }
  if (argument[0] == '\0')
  {
    if (ch->fighting != NULL)
      victim = ch->fighting;

    else
    {
      send_to_char ("Throw what at whom?\n\r", ch);
      return;
    }
  }

  else if ((victim = get_char_room (ch, argument)) == NULL)
  {
    for (x = 0; x < MAX_DIR; x++)
      if (ch->in_room->exit[x] != NULL
        && ch->in_room->exit[x]->u1.to_room != NULL
        && !IS_SET (ch->in_room->exit[x]->exit_info, EX_CLOSED))
      {
        for (vch = ch->in_room->exit[x]->u1.to_room->people; vch;
          vch = vch->next_in_room)
          if (is_name (argument, vch->name) && can_see_hack (ch, vch))
          {
            victim = vch;
            door = x;
            break;
          }
          if (victim != NULL)
            break;
      }
  }
  if (victim == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (victim->in_room)
  {
    CHAR_DATA *tmp_vict = get_gravity_char_room(victim->in_room);
    if (tmp_vict)
      victim = tmp_vict;
  }
  if (victim == ch)
  {
    send_to_char ("You can't throw a weapon at yourself.\n\r", ch);
    return;
  }
  if (is_safe (ch, victim))
    return;

  // if (check_shopkeeper_attack (ch, victim))
  //   return;
  if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
  {
    send_to_char ("You cannot throw weapons in a safe room.\n\r", ch);
    return;
  }
  // Akamai 6/9/98 -- Bug: #12 Throw needs to enforce 20 level use
  // restrictions like all other equipment
  if (ch->level < (obj->level - EQUIP_LEVEL_DIFF))
  {

    // make it look like the character released the weapon
    obj_from_char (obj);
    //Iblis - 3/11/04 - Antiduping Code
    if (!IS_NPC(ch))
      save_char_obj (ch);
    if (ch->in_room == victim->in_room)
    {
      act ("$n poorly lobs $p in your direction.", ch, obj, victim,
        TO_VICT);
      act ("$n poorly lobs $p in $N's direction.", ch, obj, victim,
        TO_NOTVICT);
      act ("You poorly lob $p in $N's direction.", ch, obj, victim,
        TO_CHAR);
      act ("$p clatters harmlessly to the ground.", ch, obj, victim,
        TO_ROOM);
      act ("$p lands nearly at your feet.", ch, obj, victim, TO_VICT);
    }

    else
    {
      act ("You poorly lob $p $T.", ch, obj, dir_name[door], TO_CHAR);
      act ("$n poorly lobs $p $T.", ch, obj, dir_name[door], TO_ROOM);
      act ("You hear $p clatter harmlessly to the ground.", ch, obj,
        NULL, TO_ROOM);
      act ("$p clatters harmlessly to the ground.", victim, obj,
        NULL, TO_ROOM);
      act ("$p lands nearly at your feet.", victim, obj, NULL, TO_CHAR);
    }
    obj_to_room (obj, victim->in_room);
    WAIT_STATE (ch, 20);
    return;
  }
  if (hear_the_wind (ch, victim, obj))
    return;
  obj_from_char (obj);
  //Iblis - 3/11/04 - Antiduping Code
  if (!IS_NPC(ch))
    save_char_obj (ch);
  if (ch->in_room == victim->in_room)
  {
    act ("$n throws $p in your direction.", ch, obj, victim, TO_VICT);
    act ("$n throws $p in $N's direction.", ch, obj, victim, TO_NOTVICT);
    act ("You throw $p in $N's direction.", ch, obj, victim, TO_CHAR);
  }

  else
  {
    act ("You throw $p $T.", ch, obj, dir_name[door], TO_CHAR);
    act ("$n throws $p $T.", ch, obj, dir_name[door], TO_ROOM);
    act ("$p comes flying out of nowhere.", victim, obj, NULL, TO_ROOM);
    act ("$p comes flying out of nowhere.", victim, obj, NULL, TO_CHAR);
  }
  WAIT_STATE (ch, 20);
  if (victim->in_room->sector_type == SECT_UNDERWATER)
  {
    act ("$p floats into the murky depths of the water.", ch, obj,
      NULL, TO_CHAR);
    obj_to_room (obj, victim->in_room);
    act ("$p floats gently into the water.", victim, obj, NULL, TO_ROOM);
    return;
  }
  if (ch->in_room->sector_type == SECT_UNDERWATER)
  {
    act ("$p floats into the murky depths of the water.", ch, obj,
      NULL, TO_CHAR);
    obj_to_room (obj, ch->in_room);
    act ("$p floats gently into the water.", ch, obj, NULL, TO_ROOM);
    return;
  }
  if (is_affected (victim, gsn_mirror))
  {
    obj_to_room (obj, victim->in_room);
    act
      ("$p bounces off of your invisible mirror and clatters to the ground.",
      victim, obj, NULL, TO_CHAR);
    act
      ("$p bounces off of an invisible mirror and clatters to the ground.",
      victim, obj, victim, TO_NOTVICT);
    return;
  }
  chance += get_curr_stat (ch, STAT_DEX) / 3;
  chance -= get_curr_stat (victim, STAT_DEX) / 5;
  chance += victim->size;

  //chance += GET_AC (victim, AC_PIERCE) / 4;
  if (IS_NPC (victim))
    chance += GET_AC (victim, AC_PIERCE) / 5;

  else
    chance += GET_AC (victim, AC_PIERCE) / 6;
  if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
    chance += 5;
  if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
    chance -= 20;

  //  chance += (ch->level - victim->level);
  if (number_percent () < chance)
  {
    if (!IS_AWAKE (victim))
      victim->position = POS_RESTING;
    act ("$p hits you squarely in the torso, causing you to wince in pain.",
      victim, obj, NULL, TO_CHAR);
    act ("You aim perfectly, and $p hits $N soundly.", ch, obj, victim,
      TO_CHAR);
    act ("$N aims perfectly, and hits $n soundly with $p.", victim, obj, ch,
      TO_NOTVICT);
    obj_to_char (obj, victim);
    //Iblis - 3/11/04 - Antiduping Code
    if (victim && !IS_NPC(victim))
      save_char_obj (victim);
    check_improve (ch, gsn_throwing, TRUE, 1);
    if (ch->in_room != victim->in_room)
    {
      nodam_message = TRUE;
      vfightnow = victim->fighting;
      cfightnow = ch->fighting;
    }
    update_aggressor (ch, victim);
    one_hit_throw (ch, victim, gsn_throwing, obj);
    if (ch->in_room != victim->in_room)
    {
      victim->fighting = vfightnow;
      ch->fighting = cfightnow;
    }
    if (ch->fighting == NULL)
    {
      if (ch->mount != NULL && ch->mount->riders != NULL)
        ch->position = POS_MOUNTED;

      else
        ch->position = POS_STANDING;
    }
    if (victim->fighting == NULL)
    {
      if (victim->mount != NULL && victim->mount->riders != NULL)
        victim->position = POS_MOUNTED;

      else
        victim->position = POS_STANDING;
    }
    nodam_message = FALSE;
  }

  else
  {
    act ("$p sails right past $n and falls with a clatter to the ground.",
      victim, obj, NULL, TO_NOTVICT);
    if (ch->in_room != victim->in_room)
      act ("$p sails right past $N and falls with a clatter to the ground.",
      ch, obj, victim, TO_CHAR);
    act ("$p comes whizzing by and falls with a clatter to the ground.",
      victim, obj, NULL, TO_CHAR);
    obj_to_room (obj, victim->in_room);
    check_improve (ch, gsn_throwing, FALSE, 1);
    if ((get_curr_stat (ch, STAT_DEX) < 21
      || get_curr_stat (ch, STAT_CHA) < 14) && number_percent () <= 25)
    {
      damage (ch, victim, 0, gsn_throwing, DAM_NONE, FALSE);

      //update_aggressor(ch,victim);
    }
    update_aggressor (ch, victim);
  }
}

void do_assassinate (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim = NULL, *vch, *vfightnow, *cfightnow;
  char vict_str[MAX_INPUT_LENGTH];
  OBJ_DATA *obj, *weapon, *tobj;
  short x, chance, door = 0, hands = 0;
  chance = get_skill (ch, gsn_assassinate);
  if (chance <= 0)
  {
    send_to_char ("Assassination? What's that?\n\r", ch);
    return;
  }
  if (ch->move < 50)
  {
    send_to_char
      ("You are currently too tired to assassinate anyone.\n\r", ch);
    return;
  }
  argument = one_argument (argument, vict_str);
  if (argument[0] == '\0')
  {
    send_to_char ("Assassinate whom with what?\n\r", ch);
    return;
  }
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    if (is_name (argument, obj->name) && can_see_obj (ch, obj))
      break;
  if (obj == NULL)
  {
    send_to_char ("You don't seem to be carrying that weapon.\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CALM) || is_affected (ch, skill_lookup
    ("Quiet Mountain Lake")))
  {
    send_to_char ("You are feeling too peaceful to do that now.\n\r", ch);
    return;
  }
  if (obj->item_type != ITEM_WEAPON)
  {
    act ("$p is not a weapon.", ch, obj, NULL, TO_CHAR);
    return;
  }
  weapon = get_eq_char (ch, WEAR_WIELD_R);
  if (weapon != NULL && weapon != obj
    && IS_WEAPON_STAT (weapon, WEAPON_TWO_HANDS))
  {
    act ("You have no hand free with which to throw $p.", ch, obj,
      NULL, TO_CHAR);
    return;
  }
  weapon = get_eq_char (ch, WEAR_WIELD_L);
  if (weapon != NULL && weapon != obj
    && IS_WEAPON_STAT (weapon, WEAPON_TWO_HANDS))
  {
    act ("You have no hand free with which to throw $p.", ch, obj,
      NULL, TO_CHAR);
    return;
  }
  for (tobj = ch->carrying; tobj; tobj = tobj->next_content)
    if ((tobj->wear_loc == WEAR_HOLD
      || tobj->wear_loc == WEAR_WIELD_R || tobj->wear_loc ==
      WEAR_WIELD_L) && tobj != obj)
      hands++;
  if (hands > 1)
  {
    act ("You have no hand free with which to throw $p.", ch, obj,
      NULL, TO_CHAR);
    return;
  }
  // Minax 7-7-02 Modified the code below to allow assassins to assassinate
  // with any weapon, not just piercing weapons.
  if ((obj->value[3] != 11 && !IS_CLASS (ch, PC_CLASS_ASSASSIN))
    || (obj->value[0] == WEAPON_ARROW))
  {				/* Weapon PIERCE? */
    act ("$p cannot be used for assassination.", ch, obj, NULL, TO_CHAR);
    return;
  }
  if (IS_SET (ch->in_room->room_flags, ROOM_SAFE))
  {
    send_to_char ("You cannot assassinate in a safe room.\n\r", ch);
    return;
  }
  if (argument[0] == '\0')
  {
    send_to_char ("Assassinate whom with what?\n\r", ch);
    return;
  }
  if ((victim = get_char_room (ch, vict_str)) != NULL)
  {
    act ("You are too close to $N to perform an assassination.", ch,
      NULL, victim, TO_CHAR);
    return;
  }
  for (x = 0; x < MAX_DIR; x++)
  {
    if (ch->in_room->exit[x] != NULL
      && ch->in_room->exit[x]->u1.to_room != NULL
      && !IS_SET (ch->in_room->exit[x]->exit_info, EX_CLOSED))
    {
      for (vch = ch->in_room->exit[x]->u1.to_room->people; vch;
        vch = vch->next_in_room)
        if (is_name (vict_str, vch->name) && can_see_hack (ch, vch))
        {
          victim = vch;
          door = x;
          break;
        }
        if (victim != NULL)
          break;
        if (ch->in_room->exit[x]->u1.to_room->exit[x] != NULL
          && ch->in_room->exit[x]->u1.to_room->exit[x]->u1.
          to_room !=
          NULL && !IS_SET (ch->in_room->exit[x]->u1.to_room->
          exit[x]->exit_info, EX_CLOSED))
        {
          for (vch =
            ch->in_room->exit[x]->u1.to_room->exit[x]->
            u1.to_room->people; vch; vch = vch->next_in_room)
            if (is_name (vict_str, vch->name) && can_see_hack (ch, vch))
            {
              victim = vch;
              door = x;
              break;
            }
            if (victim != NULL)
              break;
        }
    }
  }
  if (victim == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (victim->in_room)
  {
    CHAR_DATA *tmp_vict = get_gravity_char_room(victim->in_room);
    if (tmp_vict)
      victim = tmp_vict;
  }
  if (victim == ch)
  {
    send_to_char ("You can't assassinate yourself.\n\r", ch);
    return;
  }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (victim->in_room == ch->in_room)
  {
    send_to_char
      ("You can't assassinate someone in the same room.\n\r", ch);
    return;
  }
  if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
  {
    send_to_char ("You cannot assassinate in a safe room.\n\r", ch);
    return;
  }
  if (is_safe (ch, victim))
    return;

  if (ch->in_room->sector_type == SECT_UNDERWATER && ch->race != PC_RACE_CANTHI)
  {				/* Canthi */
    act ("$p doesn't make for a good tool of assassination underwater.",
      ch, obj, NULL, TO_CHAR);
    return;
  }
  ch->move /= 2;

  //unequip_char (ch, obj);
  //obj_from_char (obj);
  act ("You fling $p $T.", ch, obj, dir_name[door], TO_CHAR);
  act ("$p comes hurling out of nowhere.", victim, obj, NULL, TO_ROOM);
  act ("$p comes hurling out of nowhere.", victim, obj, NULL, TO_CHAR);
  if (get_skill (ch, gsn_assassinate) >= 75)
    WAIT_STATE (ch, skill_table[gsn_assassinate].beats - PULSE_VIOLENCE);

  else
    WAIT_STATE (ch, skill_table[gsn_assassinate].beats);
  if (is_affected (victim, gsn_mirror))
  {
    obj_to_room (obj, victim->in_room);
    //Iblis - 3/11/04 - Antiduping Code
    if (!IS_NPC(ch))
      save_char_obj (ch);
    act
      ("$p bounces off of your invisible mirror and clatters to the ground.",
      victim, obj, NULL, TO_CHAR);
    act
      ("$p bounces off of an invisible mirror and clatters to the ground.",
      victim, obj, victim, TO_NOTVICT);
    return;
  }
  if (victim->in_room->sector_type == SECT_UNDERWATER)
  {
    act ("$p floats into the murky depths of the water.", ch, obj,
      NULL, TO_CHAR);
    obj_to_room (obj, victim->in_room);
    //Iblis - 3/11/04 - Antiduping Code
    if (!IS_NPC(ch))
      save_char_obj (ch);
    act ("$p floats gently into the water.", victim, obj, NULL, TO_ROOM);
    return;
  }
  chance += get_curr_stat (ch, STAT_DEX) / 3;
  chance -= get_curr_stat (victim, STAT_DEX) / 5;
  chance += victim->size;
  chance += GET_AC (victim, AC_PIERCE) / 4;
  if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
    chance += 5;
  if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
    chance -= 20;

  //  chance += (ch->level - victim->level);
  if (number_percent () < chance && victim->race != PC_RACE_SWARM)
  {
    if (hear_the_wind (ch, victim, obj))
      return;
    unequip_char (ch, obj);
    obj_from_char (obj);
    if (!IS_AWAKE (victim))
      victim->position = POS_RESTING;
    act
      ("Gasping for breath, you double over in pain as $p finds you in the back.",
      victim, obj, NULL, TO_CHAR);
    act
      ("$N gasps for breath and doubles over in pain as $p hits $M in the back.",
      ch, obj, victim, TO_CHAR);
    act
      ("$N gasps for breath and doubles over in pain as $p hits $M in the back.",
      victim, obj, victim, TO_NOTVICT);
    check_improve (ch, gsn_assassinate, TRUE, 1);
    nodam_message = TRUE;
    vfightnow = victim->fighting;
    cfightnow = ch->fighting;
    update_aggressor2 (ch, victim, TRUE);
    one_hit_throw (ch, victim, gsn_assassinate, obj);

    /* BREAK THE WEAPON HERE IN THE FUTURE */
    extract_obj (obj);
    victim->fighting = vfightnow;
    ch->fighting = cfightnow;
    if (ch->fighting == NULL)
    {
      if (ch->mount != NULL && ch->mount->riders != NULL)
        ch->position = POS_MOUNTED;

      else
        ch->position = POS_STANDING;
    }
    if (victim->fighting == NULL)
    {
      if (victim->mount != NULL && victim->mount->riders != NULL)
        victim->position = POS_MOUNTED;

      else
        victim->position = POS_STANDING;
    }
    nodam_message = FALSE;
  }

  else
  {
    unequip_char (ch, obj);
    obj_from_char (obj);
    obj_to_room (obj, victim->in_room);
    //Iblis - 3/11/04 - Antiduping Code
    if (!IS_NPC(ch))
      save_char_obj (ch);
    act ("$p misses $n and falls to the ground.", victim, obj, NULL,
      TO_NOTVICT);
    act ("$p misses $N and falls to the ground.", ch, obj, victim, TO_CHAR);
    act ("$p comes hurling by and falls to the ground.", victim, obj,
      NULL, TO_CHAR);
    check_improve (ch, gsn_assassinate, FALSE, 1);
    update_aggressor2 (ch, victim, TRUE);
  }
}

void do_trip (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim, *oldvict = NULL;
  int chance;
  one_argument (argument, arg);

  if (is_affected (ch, skill_lookup("clasp")))
  {
    send_to_char ("Your feet are to restrained to do that.\n\r", ch);
    return;
  }
  /* BEGIN UNDERWATER */
  if (ch->in_room->sector_type == SECT_UNDERWATER)
  {
    send_to_char ("You can't trip someone underwater!\n\r", ch);
    return;
  }

  /* END UNDERWATER */
  if ((chance = get_skill (ch, gsn_trip)) == 0
    || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_TRIP)))
  {
    send_to_char ("Tripping?  What's that?\n\r", ch);
    return;
  }
  if (arg[0] == '\0')
  {
    victim = ch->fighting;
    if (victim == NULL)
    {
      send_to_char ("But you aren't fighting anyone!\n\r", ch);
      return;
    }
  }

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (ch->mount != NULL)
  {
    send_to_char ("You cannot trip while mounted.\n\r", ch);
    return;
  }
  if (victim->mount != NULL)
  {
    send_to_char ("You cannot trip someone who is mounted.\n\r", ch);
    return;
  }
  if (is_safe (ch, victim))
    return;
  if (IS_AFFECTED (victim, AFF_FLYING))
  {
    act ("$S feet aren't on the ground.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (victim->race == PC_RACE_SWARM)
  {
    act ("You can't figure out how to trip $m.", ch, NULL, victim,TO_CHAR);
    return;
  }
  if (victim->position < POS_FIGHTING)
  {
    act ("$N is already down.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("You can't trip yourself!\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
  {
    act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
    return;
  }

  /* modifiers */

  /* size */
  if (ch->size < victim->size)
    chance += (ch->size - victim->size) * 10;	/* bigger = harder to trip */

  /* dex */
  chance += get_curr_stat (ch, STAT_DEX);
  chance -= get_curr_stat (victim, STAT_DEX) * 3 / 2;

  /* speed */
  if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
    chance += 10;
  if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
    chance -= 20;

  /* level */
  chance += (ch->level - victim->level) * 2;
  if (is_affected (victim, gsn_mirror))
  {
    oldvict = victim;
    victim = ch;
  }

  /* now the attack */
  if (number_percent () < chance)
  {
    if (awareness (ch, victim))
      return;
    if (victim != ch)
    {
      act ("`B$n trips you and you go down!``", ch, NULL, victim,
        TO_VICT);
      act ("`AYou trip $N and $E goes down!``", ch, NULL, victim,
        TO_CHAR);
      act ("$n trips $N, sending $M to the ground.", ch, NULL,
        victim, TO_NOTVICT);
    }

    else
    {
      act
        ("`BYour mirror deflects $n's trip, sending $m to the ground!``",
        ch, NULL, oldvict, TO_VICT);
      act ("`A$N's mirror deflects your trip, causing you to fall!``", ch,
        NULL, oldvict, TO_CHAR);
      act
        ("$n's is sent to the ground as $s trip is deflected off of $N's mirror.",
        ch, NULL, oldvict, TO_NOTVICT);
    }
    check_improve (ch, gsn_trip, TRUE, 1);

    //    DAZE_STATE (victim, 2 * PULSE_VIOLENCE);
    WAIT_STATE (victim, 2 * PULSE_VIOLENCE);

    //    WAIT_STATE (ch, skill_table[gsn_trip].beats);
    WAIT_STATE (ch, 2 * PULSE_VIOLENCE);
    victim->position = POS_RESTING;
    victim->bashed = TRUE;
    if (IS_NPC (victim))
      victim->color = 42;
    damage (ch, victim, number_range (2, 2 + 2 * victim->size),
      gsn_trip, DAM_BASH, TRUE);
    if (!is_affected (victim, gsn_mirror)
      && is_affected (victim, gsn_fireshield))
      if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
      {
        damage (victim, ch,
          number_range (2, 2 + 2 * victim->size) / 2, 0,
          DAM_FIRE, FALSE);
        act ("$N's scorching shield of flame burns you as you attack.",
          ch, NULL, victim, TO_CHAR);
        act ("$n is burnt by your scorching shield of flame.", ch, NULL,
          victim, TO_VICT);
      }
  }

  else
  {
    damage (ch, victim, 0, gsn_trip, DAM_BASH, TRUE);
    WAIT_STATE (ch, skill_table[gsn_trip].beats * 2 / 3);
    check_improve (ch, gsn_trip, FALSE, 1);
  }
}

void do_butcher (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj, *objin, *objnext;
  char buf[MAX_STRING_LENGTH];
  int number, skillper, num, cnt;
  one_argument (argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char ("Butcher what?\n\r", ch);
    return;
  }
  if ((obj = get_obj_here (ch, arg)) == NULL)
  {
    act ("You see no $T here.", ch, NULL, arg, TO_CHAR);
    return;
  }
  if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
  {
    act ("You can't butcher $T.", ch, NULL, arg, TO_CHAR);
    return;
  }
  if (get_skill (ch, gsn_butcher) == 0)
  {
    send_to_char ("You don't know the first thing about butchery.\n\r", ch);
    return;
  }
  if (obj->item_type == ITEM_CORPSE_PC)
  {
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    sprintf(buf,"the corpse of %s1",ch->name);
    strcpy(buf2,obj->short_descr);
    strcat(buf2,"1");
    if (/*IS_NPC(ch) ||*/ strcmp(buf,buf2) && !IS_IMMORTAL(ch))
    {
      send_to_char("If you attempt to butcher a player corpse other than your own, you shall face the wrath of the gods.\n\r",ch);
      return;
    }
  }

  skillper = get_skill (ch, gsn_butcher);
  number = number_percent ();
  if (number > skillper)
  {
    act ("You botch the job of hacking $p into pieces.", ch, obj, NULL,
      TO_CHAR);
    act ("$n botches the job of hacking $p into pieces.", ch, obj,
      NULL, TO_ROOM);
    check_improve (ch, gsn_butcher, FALSE, 1);
    for (objin = obj->contains; objin; objin = objnext)
    {
      objnext = objin->next_content;
      obj_from_obj (objin);
      obj_to_room (objin, ch->in_room);
      sprintf (buf, "$p falls from %s.", obj->short_descr);
      act (buf, ch, objin, NULL, TO_ROOM);
      act (buf, ch, objin, NULL, TO_CHAR);
      if (objin->pIndexData->vnum <= 5 && objin->pIndexData->vnum >= 1)
      {
        int silver = 0;
        int gold = 0;
        OBJ_DATA *tobj, *tobj_next;
        for (tobj = ch->in_room->contents; tobj != NULL; tobj = tobj_next)
        {
          tobj_next = tobj->next_content;
          switch (tobj->pIndexData->vnum)
          {
          case OBJ_VNUM_SILVER_ONE:
            silver += 1;
            extract_obj (tobj);
            break;
          case OBJ_VNUM_GOLD_ONE:
            gold += 1;
            extract_obj (tobj);
            break;
          case OBJ_VNUM_SILVER_SOME:
            silver += tobj->value[0];
            extract_obj (tobj);
            break;
          case OBJ_VNUM_GOLD_SOME:
            gold += tobj->value[1];
            extract_obj (tobj);
            break;
          case OBJ_VNUM_COINS:
            silver += tobj->value[0];
            gold += tobj->value[1];
            extract_obj (tobj);
            break;
          }
        }
        obj_to_room (create_money (gold, silver), ch->in_room);

      }  
    }
    extract_obj (obj);
    return;
  }
  act ("You butcher $p, hacking it into raw bloody chunks.", ch, obj,
    NULL, TO_CHAR);
  act ("$n butchers $p, hacking it into raw bloody chunks.", ch, obj,
    NULL, TO_ROOM);
  check_improve (ch, gsn_butcher, TRUE, 1);
  if (skillper < 25 || obj->value[6] == SIZE_TINY)
    num = 1;

  else if (skillper < 50 || obj->value[6] == SIZE_SMALL)
    num = 2;

  else if (skillper < 75)
    num = 3;

  else
    num = 4;
  for (objin = obj->contains; objin; objin = objnext)
  {
    objnext = objin->next_content;
    obj_from_obj (objin);
    obj_to_room (objin, ch->in_room);
    sprintf (buf, "$p falls from %s.", obj->short_descr);
    act (buf, ch, objin, NULL, TO_ROOM);
    act (buf, ch, objin, NULL, TO_CHAR);
    if (objin->pIndexData->vnum <= 5 && objin->pIndexData->vnum >= 1)
    {
      int silver = 0;
      int gold = 0;
      OBJ_DATA *tobj, *tobj_next;
      for (tobj = ch->in_room->contents; tobj != NULL; tobj = tobj_next)
      {
        tobj_next = tobj->next_content;
        switch (tobj->pIndexData->vnum)
        {
        case OBJ_VNUM_SILVER_ONE:
          silver += 1;
          extract_obj (tobj);
          break;
        case OBJ_VNUM_GOLD_ONE:
          gold += 1;
          extract_obj (tobj);
          break;
        case OBJ_VNUM_SILVER_SOME:
          silver += tobj->value[0];
          extract_obj (tobj);
          break;
        case OBJ_VNUM_GOLD_SOME:
          gold += tobj->value[1];
          extract_obj (tobj);
          break;
        case OBJ_VNUM_COINS:
          silver += tobj->value[0];
          gold += tobj->value[1];
          extract_obj (tobj);
          break;
        }
      }
      obj_to_room (create_money (gold, silver), ch->in_room);

    }
  }
  extract_obj (obj);
  for (cnt = 0; cnt < num; cnt++)
  {
    obj = create_object (get_obj_index (OBJ_VNUM_BUTCHER_MEAT), 0);
    obj_to_room (obj, ch->in_room);
  }
}

void do_trample (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];//, *ps;
  CHAR_DATA *victim;		//, *lch;
  int chance = 0;
  one_argument (argument, arg);
  if (!IS_NPC (ch))
  {
    chance = get_skill (ch, gsn_trample);
    if (chance == 0)
    {
      send_to_char ("Trample?  What's that?\n\r", ch);
      return;
    }
  }
  if (arg[0] == '\0')
  {
    victim = ch->fighting;
    if (victim == NULL)
    {
      send_to_char ("But you aren't fighting anyone!\n\r", ch);
      return;
    }
  }

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (is_safe (ch, victim))
    return;
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (IS_AFFECTED (victim, AFF_FLYING))
  {
    send_to_char ("You can't trample someone who is flying.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("You can't trample yourself!\n\r", ch);
    return;
  }
  if (ch->mount == NULL)
  {
    send_to_char ("You must have a mount to trample someone.\n\r", ch);
    return;
  }
  if (victim->mount != NULL)
  {
    send_to_char ("You can't trample someone who is mounted.\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
  {
    act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
    return;
  }

  /* modifiers */
  chance += ch->mount->pIndexData->attackbonus;
  if (ch->size < victim->size)
    chance += (ch->size - victim->size) * 10;	/* bigger = harder to trip */

  /* dex */
  chance -= get_curr_stat (victim, STAT_DEX) * 3 / 2;
  if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
    chance -= 20;

  /* level */
  chance += (ch->level - victim->level) * 2;
  if (is_affected (victim, gsn_mirror))
  {
    act ("Your mount is bounced back by the magical mirror surrounding $N.",
      ch, NULL, victim, TO_CHAR);
    act ("$n's attempt to trample you is deflected by your magical mirror.",
      ch, NULL, victim, TO_VICT);
    act ("$n's attempt to trample $N is deflected by $N's magical mirror.",
      ch, NULL, victim, TO_NOTVICT);
    return;
  }

  /* now the attack */
  if (number_percent () < chance && victim->race != PC_RACE_SWARM)
  {
    sprintf (buf,
      "`B%s's mount flares $s nostrils, rears back wrathfully and tramples you.``",
      ch->name);
    act (buf, ch->mount, NULL, victim, TO_VICT);

    act ("`AYour mount rears back wrathfully and tramples $N.``",
      ch, NULL, victim, TO_CHAR);
    act ("$n's mount rears back wrathfully and tramples $N.", ch,
      NULL, victim, TO_NOTVICT);

    check_improve (ch, gsn_trample, TRUE, 1);
    DAZE_STATE (victim, 24);
    WAIT_STATE (ch, skill_table[gsn_trample].beats);
    victim->position = POS_SITTING;
    damage (ch, victim, number_range (2, 2 +
      (6 * ch->mount->size *
      ch->mount->pIndexData->
      attackbonus)), gsn_trample,
      DAM_BASH, TRUE);
    if (!is_affected (victim, gsn_mirror)
      && is_affected (victim, gsn_fireshield))
      if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
      {
        damage (victim, ch,
          number_range (2, 2 + 2 * victim->size) / 2, 0,
          DAM_FIRE, FALSE);
        act ("$N's scorching shield of flame burns you as you attack.",
          ch, NULL, victim, TO_CHAR);
        act ("$n is burnt by your scorching shield of flame.", ch, NULL,
          victim, TO_VICT);
      }
  }

  else
  {
    damage (ch, victim, 0, gsn_trample, DAM_BASH, TRUE);
    WAIT_STATE (ch, skill_table[gsn_trample].beats * 2 / 3);
    check_improve (ch, gsn_trample, FALSE, 1);
  }
}

void do_killwipe (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  one_argument (argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char ("Syntax: killwipe [player]\n\r", ch);
    return;
  }
  if ((victim = get_char_world (ch, arg)) != NULL)
  {
    if (IS_NPC (victim))
    {
      send_to_char
        ("Non-player characters do not have a kill count.\n\r", ch);
      return;
    }
    victim->pcdata->has_killed[PLAYER_KILL] = 0;
    victim->pcdata->has_killed[MOB_KILL] = 0;
    victim->pcdata->has_killed[BR_KILL] = 0;
    victim->pcdata->has_killed[SD_KILL] = 0;
    victim->pcdata->been_killed[PLAYER_KILL] = 0;
    victim->pcdata->been_killed[MOB_KILL] = 0;
    victim->pcdata->been_killed[BR_KILL] = 0;
    victim->pcdata->been_killed[SD_KILL] = 0;
    victim->pcdata->other_deaths = 0;
    send_to_char ("Ok.\n\r", ch);
  }

  else
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
}

void do_killcount (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], *nameptr, buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    victim = ch;

  else if ((victim = get_char_world (ch, arg)) != NULL)
  {
    if (IS_NPC (victim))
    {
      send_to_char
        ("Non-player characters do not have a kill count.\n\r", ch);
      return;
    }
  }

  else
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch)
    nameptr = "You";

  else
  {
    if (is_affected (victim, gsn_mask) && victim->mask != NULL)
      nameptr = victim->mask;

    else
      nameptr = victim->name;
  }
  if (victim->pcdata->been_killed[PLAYER_KILL] +
    victim->pcdata->been_killed[MOB_KILL] <= 0)
    sprintf (buf, "%s %s never died.\n\r", nameptr,
    victim == ch ? "have" : "has");

  else
  {
    sprintf (buf, "%s %s died %d time%s,", nameptr,
      victim == ch ? "have" : "has",
      victim->pcdata->been_killed[PLAYER_KILL] +
      victim->pcdata->been_killed[MOB_KILL],
      (victim->pcdata->been_killed[PLAYER_KILL] +
      victim->pcdata->been_killed[MOB_KILL] == 1) ? "" : "s");
    if (victim->pcdata->been_killed[PLAYER_KILL] == 0)
      strcat (buf, " but never due to player killing.\n\r");

    else if (victim->pcdata->been_killed[PLAYER_KILL] == 1
      && victim->pcdata->been_killed[MOB_KILL] == 0)
      strcat (buf, " due to player killing.\n\r");

    else if (victim->pcdata->been_killed[MOB_KILL] == 0)
      strcat (buf, " always due to player killing.\n\r");

    else
      sprintf (buf + strlen (buf),
      " %d times due to player killing.\n\r",
      victim->pcdata->been_killed[PLAYER_KILL]);
  }
  if (victim->pcdata->has_killed[PLAYER_KILL] +
    victim->pcdata->has_killed[MOB_KILL] <= 0)
    sprintf (strlen (buf) + buf, "%s %s never killed anyone.\n\r",
    nameptr, victim == ch ? "have" : "has");

  else
  {
    sprintf (strlen (buf) + buf, "%s %s killed %d time%s,", nameptr,
      victim == ch ? "have" : "has",
      victim->pcdata->has_killed[PLAYER_KILL] +
      victim->pcdata->has_killed[MOB_KILL],
      (victim->pcdata->has_killed[PLAYER_KILL] +
      victim->pcdata->has_killed[MOB_KILL] == 1) ? "" : "s");
    if (victim->pcdata->has_killed[PLAYER_KILL] == 0)
      strcat (buf, " but never another player.\n\r");

    else if (victim->pcdata->has_killed[PLAYER_KILL] == 1
      && victim->pcdata->has_killed[MOB_KILL] == 0)
      strcat (buf, " the victim being an unlucky player.\n\r");

    else if (victim->pcdata->has_killed[PLAYER_KILL] == 0)
      strcat (buf, " each victim a player.\n\r");

    else
      sprintf (buf + strlen (buf),
      " %d of these victims being players.\n\r",
      victim->pcdata->has_killed[PLAYER_KILL]);
  }
  send_to_char (buf, ch);
  sprintf(buf, "%s has killed %d people and has been killed %d times in the Arena.\n\r",
    victim->name, victim->pcdata->has_killed[ARENA_KILL], victim->pcdata->been_killed[ARENA_KILL]);
  send_to_char (buf,ch);
  sprintf(buf, "%s has killed %d people and has been killed %d times during Battle Royale.\n\r", 
    victim->name, victim->pcdata->has_killed[BR_KILL], victim->pcdata->been_killed[BR_KILL]);
  send_to_char (buf,ch);
  sprintf(buf, "%s has been the supreme victor in %d out of %d Battle Royales.\n\r",victim->name, victim->pcdata->events_won[BR_WON], victim->pcdata->events_won[BR_IN]);
  send_to_char (buf,ch);
  sprintf(buf, "%s has killed %d people and has been killed %d times during Super Duels.\n\r",
    victim->name, victim->pcdata->has_killed[SD_KILL], victim->pcdata->been_killed[SD_KILL]);
  send_to_char (buf,ch);
  sprintf(buf, "%s has been the last man standing in %d out of %d Super Duels.\n\r",victim->name, victim->pcdata->events_won[SD_WON], victim->pcdata->events_won[SD_IN]);
  send_to_char (buf,ch);
  sprintf(buf, "%s has been on the winning team in %d out of %d Team Super Duels.\n\r",victim->name, victim->pcdata->events_won[TEAM_SD_WON], victim->pcdata->events_won[TEAM_SD_IN]);
  send_to_char (buf,ch);
  sprintf(buf, "%s has committed suicide %d times.\n\r",victim->name,victim->pcdata->other_deaths);
  send_to_char (buf,ch);
}

void do_kill (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  one_argument (argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char ("Kill whom?\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char ("You are too peaceful to fight right now.\n\r", ch);
    return;
  }
  if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("You hit yourself.  Ouch!\n\r", ch);
    multi_hit (ch, ch, TYPE_UNDEFINED);
    return;
  }
  if (!IS_NPC (victim) && !IS_NPC (ch))
  {

    do_murder (ch, victim->name);

    /*      send_to_char ("Sorry, you need to MURDER other players.\n\r", ch); */
    return;
  }
  if (is_safe (ch, victim))
    return;
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
  {
    act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (ch->position == POS_FIGHTING || ch->fighting != NULL)
  {
    send_to_char ("You do the best you can!\n\r", ch);
    return;
  }
  if (awareness (ch, victim))
    return;
  WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
  if (!IS_NPC(ch) && ch->pcdata->aggression)
    ch->pcdata->aggression = 1;
  if (!IS_NPC(ch) && ch->Class == PC_CLASS_CHAOS_JESTER)
    ch->pcdata->pain_points = 0;
  /*if (((wield = get_eq_char (ch, WEAR_WIELD_R)) != NULL) && IS_WEAPON_STAT(wield,WEAPON_HOLY_ACTIVATED))
  REMOVE_BIT(wield->value[4],WEAPON_HOLY_ACTIVATED);
  if (((wield = get_eq_char (ch, WEAR_WIELD_L)) != NULL) && IS_WEAPON_STAT(wield,WEAPON_HOLY_ACTIVATED))
  REMOVE_BIT(wield->value[4],WEAPON_HOLY_ACTIVATED);
  if (((wield = get_eq_char (victim, WEAR_WIELD_R)) != NULL) && IS_WEAPON_STAT(wield,WEAPON_HOLY_ACTIVATED))
  REMOVE_BIT(wield->value[4],WEAPON_HOLY_ACTIVATED);
  if (((wield = get_eq_char (victim, WEAR_WIELD_L)) != NULL) && IS_WEAPON_STAT(wield,WEAPON_HOLY_ACTIVATED))
  REMOVE_BIT(wield->value[4],WEAPON_HOLY_ACTIVATED);*///

  trip_triggers(ch, OBJ_TRIG_CHAR_ATTACKS, NULL, victim, OT_SPEC_NONE);
  trip_triggers(victim, OBJ_TRIG_CHAR_IS_ATTACKED, NULL, ch, OT_SPEC_NONE);
  multi_hit (ch, victim, TYPE_UNDEFINED);
  return;
}

void do_murde (CHAR_DATA * ch, char *argument)
{
  send_to_char ("If you want to MURDER, spell it out.\n\r", ch);
  return;
}

bool check_shopkeeper_attack (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (is_affected (ch, gsn_entangle))
  {
    send_to_char ("You can't move your arms away from your sides.\n\r", ch);
    return (TRUE);
  }
  if (IS_NPC (victim))
  {
    if (victim->pIndexData->pShop != NULL)
    {
      send_to_char
        ("No acts of violence may be committed against that target.\n\r",
        ch);
      return (TRUE);
    }
    if (IS_SET (victim->recruit_flags, RECRUIT_KEEPER))
    {
      send_to_char
        ("No acts of violence may be committed against that target.\n\r",
        ch);
      return (TRUE);
    }
    if (IS_SET (victim->act2, ACT_NO_KILL))
    {
      send_to_char("No acts of violence may be committed against that target.\n\r",ch);
      return (TRUE);
    }
  }
  return (FALSE);
}

void do_murder (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  one_argument (argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char ("Murder whom?\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char ("You are too peaceful to fight right now.\n\r", ch);
    return;
  }
  if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("You hit yourself.  Ouch!\n\r", ch);
    multi_hit (ch, ch, TYPE_UNDEFINED);
    return;
  }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (is_safe (ch, victim))
    return;
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
  {
    act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (ch->position == POS_FIGHTING || ch->fighting != NULL)
  {
    send_to_char ("You do the best you can!\n\r", ch);
    return;
  }
  if (awareness (ch, victim))
    return;
  WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
  if (!IS_NPC(ch) && ch->pcdata->aggression)
    ch->pcdata->aggression = 1;
  if (!IS_NPC(ch) && ch->Class == PC_CLASS_CHAOS_JESTER)
    ch->pcdata->pain_points = 0;
  /*  if (((wield = get_eq_char (ch, WEAR_WIELD_R)) != NULL) && IS_WEAPON_STAT(wield,WEAPON_HOLY_ACTIVATED))
  REMOVE_BIT(wield->value[4],WEAPON_HOLY_ACTIVATED);
  if (((wield = get_eq_char (ch, WEAR_WIELD_L)) != NULL) && IS_WEAPON_STAT(wield,WEAPON_HOLY_ACTIVATED))
  REMOVE_BIT(wield->value[4],WEAPON_HOLY_ACTIVATED);
  if (((wield = get_eq_char (victim, WEAR_WIELD_R)) != NULL) && IS_WEAPON_STAT(wield,WEAPON_HOLY_ACTIVATED))
  REMOVE_BIT(wield->value[4],WEAPON_HOLY_ACTIVATED);
  if (((wield = get_eq_char (victim, WEAR_WIELD_L)) != NULL) && IS_WEAPON_STAT(wield,WEAPON_HOLY_ACTIVATED))
  REMOVE_BIT(wield->value[4],WEAPON_HOLY_ACTIVATED);*/
  if (IS_NPC (ch))
    sprintf (buf, "Help! I am being attacked by %s!", ch->short_descr);

  else
    sprintf (buf, "Help!  I am being attacked by %s!", ch->name);
  do_yell (victim, buf);
  trip_triggers(ch, OBJ_TRIG_CHAR_ATTACKS, NULL, victim, OT_SPEC_NONE);
  trip_triggers(victim, OBJ_TRIG_CHAR_IS_ATTACKED, NULL, ch, OT_SPEC_NONE);
  multi_hit (ch, victim, TYPE_UNDEFINED);
  return;
}

void do_backstab (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int backstab_skill;
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  one_argument (argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char ("Backstab whom?\n\r", ch);
    return;
  }
  if (get_skill (ch, gsn_backstab) < 1)
  {
    send_to_char ("Backstabbing, how ?!?\n\r", ch);
    return;
  }
  /*if (ch->move < 100)
  {
  send_to_char
  ("You are currently too tired to backstab anyone.\n\r", ch);
  return;
  }*/
  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char ("You are too peaceful to backstab right now.\n\r", ch);
    return;
  }
  if (ch->fighting != NULL)
  {
    send_to_char ("You're facing the wrong end.\n\r", ch);
    return;
  }

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("How can you sneak up on yourself?\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
  {
    act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (is_safe (ch, victim))
    return;
  if ((obj = get_eq_char (ch, WEAR_WIELD_R)) == NULL)
    if ((obj = get_eq_char (ch, WEAR_WIELD_L)) == NULL)
    {
      send_to_char ("You need to wield a weapon to backstab.\n\r", ch);
      return;
    }
    if (victim->hit < (victim->max_hit / 2))
    {
      act ("$N is hurt and suspicious ... you can't sneak up.", ch,
        NULL, victim, TO_CHAR);
      return;
    }
    backstab_skill = get_skill (ch, gsn_backstab);
    if (backstab_skill >= 75)
      WAIT_STATE (ch, skill_table[gsn_backstab].beats - PULSE_VIOLENCE);

    else
      WAIT_STATE (ch, skill_table[gsn_backstab].beats);
    if ((number_percent () < backstab_skill
      || (backstab_skill >= 2 && !IS_AWAKE (victim))) && victim->race != PC_RACE_SWARM)
    {
      if (awareness (ch, victim))
        return;
      check_improve (ch, gsn_backstab, TRUE, 1);

      //      ch->move -= victim->level;
      multi_hit (ch, victim, gsn_backstab);
      if (!is_affected (victim, gsn_mirror)
        && is_affected (victim, gsn_fireshield))
        if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
        {
          damage (victim, ch, 8, 0, DAM_FIRE, FALSE);
          act ("$N's scorching shield of flame burns you as you attack.",
            ch, NULL, victim, TO_CHAR);
          act ("$n is burnt by your scorching shield of flame.", ch, NULL,
            victim, TO_VICT);
        }
    }

    else
    {
      //      ch->move -= victim->level;
      check_improve (ch, gsn_backstab, FALSE, 1);
      if (number_percent () < 2 && IS_WEAPON_STAT (obj, WEAPON_VORPAL))
      {
        act ("Vorpal damage goes here.", ch, NULL, victim, TO_ROOM);
      }
      damage (ch, victim, 0, gsn_backstab, DAM_NONE, TRUE);
    }
    return;
}

void do_shadowslash (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int shadowslash_skill;
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  one_argument (argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char ("Who would you like to slash?\n\r", ch);
    return;
  }
  if (get_skill (ch, gsn_shadowslash) < 1)
  {
    send_to_char ("You don't know the first thing about Reaver dark arts.\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char ("You are too peaceful to slash at someone right now.\n\r", ch);
    return;
  }
  if (ch->fighting != NULL)
  {
    send_to_char ("You can't concentrate enough.\n\r", ch);
    return;
  }

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("How the hell do you except to do that?\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
  {
    act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (is_safe (ch, victim))
    return;
  if ((obj = get_eq_char (ch, WEAR_WIELD_R)) == NULL)
    if ((obj = get_eq_char (ch, WEAR_WIELD_L)) == NULL)
    {
      send_to_char ("You need to wield a weapon to slash someone. Duh. You're a Reaver for crying out loud.\n\r", ch);
      return;
    }
	if (victim->hit < (victim->max_hit / 2))
    {
      act ("There's not much of $N left to slash...", ch,
        NULL, victim, TO_CHAR);
      return;
    }
    shadowslash_skill = get_skill (ch, gsn_shadowslash);
    if (shadowslash_skill >= 75)
      WAIT_STATE (ch, skill_table[gsn_shadowslash].beats - PULSE_VIOLENCE);

    else
      WAIT_STATE (ch, skill_table[gsn_shadowslash].beats);
    if ((number_percent () < shadowslash_skill
      || (shadowslash_skill >= 2 && !IS_AWAKE (victim))) && victim->race != PC_RACE_SWARM)
    {
      if (awareness (ch, victim))
        return;
      check_improve (ch, gsn_shadowslash, TRUE, 1);

      //      ch->move -= victim->level;
      multi_hit (ch, victim, gsn_shadowslash);
      if (!is_affected (victim, gsn_mirror)
        && is_affected (victim, gsn_fireshield))
        if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
        {
          damage (victim, ch, 8, 0, DAM_FIRE, FALSE);
          act ("$N's scorching shield of flame burns you as you attack.",
            ch, NULL, victim, TO_CHAR);
          act ("$n is burnt by your scorching shield of flame.", ch, NULL,
            victim, TO_VICT);
        }
    }

    else
    {
      //      ch->move -= victim->level;
      check_improve (ch, gsn_shadowslash, FALSE, 1);
      if (number_percent () < 2 && IS_WEAPON_STAT (obj, WEAPON_VORPAL))
      {
        act ("Vorpal damage goes here.", ch, NULL, victim, TO_ROOM);
      }
      damage (ch, victim, 0, gsn_shadowslash, DAM_NONE, TRUE);
    }
    return;
}

void do_eviscerate (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int eviscerate_skill;
  CHAR_DATA *victim;
  one_argument (argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char ("eviscerate whom?\n\r", ch);
    return;
  }
  if (get_skill (ch, gsn_eviscerate) < 1)
  {
    send_to_char ("You wouldn't even know where to begin.\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char
      ("You are too peaceful to eviscerate anyone right now.\n\r", ch);
    return;
  }
  if (ch->fighting != NULL)
  {
    send_to_char ("You'll never pull it off in combat.\n\r", ch);
    return;
  }

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("...and your guts would be all over the floor.\n\r", ch);
    return;
  }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (is_safe (ch, victim))
    return;
  if (get_eq_char (ch, WEAR_WIELD_R) == NULL ||
    get_eq_char (ch, WEAR_WIELD_L) == NULL)
  {
    send_to_char ("You need to wield two weapons to eviscerate.\n\r", ch);
    return;
  }
  if (!IS_SET (get_eq_char(ch,WEAR_WIELD_R)->value[4], WEAPON_SHARP)
    || !IS_SET (get_eq_char(ch,WEAR_WIELD_L)->value[4], WEAPON_SHARP))
  {
    act ("Only the sharpest weapons will work.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (victim->hit < (victim->max_hit - victim->max_hit / 20))
  {
    act ("$N is hurt and suspicious ... you can't sneak up.", ch,
      NULL, victim, TO_CHAR);
    return;
  }
  eviscerate_skill = get_skill (ch, gsn_eviscerate);
  if ((number_percent () + 5 < eviscerate_skill || (eviscerate_skill >= 2 && !IS_AWAKE (victim)))
    && victim->race != PC_RACE_SWARM)
  {
    if (awareness (ch, victim))
      return;
    check_improve (ch, gsn_eviscerate, TRUE, 1);
    WAIT_STATE (ch, skill_table[gsn_eviscerate].beats);
    if (victim->hit > 10000)
    {
      victim->hit -= 5000;
      act
        ("`nYou pounce upon $N`n and `iEVISCERATE`n $M with incredible fury!``",
        ch, NULL, victim, TO_CHAR);
      act
        ("`n$n `npounces upon you and `iEVISCERATES`n you with incredible fury!``",
        ch, NULL, victim, TO_VICT);
      act
        ("`n$n `npounces upon $N`n and `iEVISCERATES`n $M with incredible fury!``",
        ch, NULL, victim, TO_NOTVICT);
    }

    else
    {
      victim->hit -= ((victim->hit) / 2);
      act
        ("`nYou pounce upon $N`n and `iEVISCERATE`n $M with incredible fury!``",
        ch, NULL, victim, TO_CHAR);
      act
        ("`n$n `npounces upon you and `iEVISCERATES`n you with incredible fury!``",
        ch, NULL, victim, TO_VICT);
      act
        ("`n$n `npounces upon $N`n and `iEVISCERATES`n $M with incredible fury!``",
        ch, NULL, victim, TO_NOTVICT);
    }
    damage(ch,victim,0,gsn_eviscerate,DAM_NONE,FALSE);
    //      set_fighting (victim, ch);
    if (!is_affected (victim, gsn_mirror)
      && is_affected (victim, gsn_fireshield))
      if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
      {
        damage (victim, ch, 8, 0, DAM_FIRE, FALSE);
        act ("$N's scorching shield of flame burns you as you attack.",
          ch, NULL, victim, TO_CHAR);
        act ("$n is burnt by your scorching shield of flame.", ch, NULL,
          victim, TO_VICT);
      }
  }

  else
  {
    act ("You fail miserably in your attempt to eviscerate $N.", ch,
      NULL, victim, TO_CHAR);
    WAIT_STATE (ch, 4 * PULSE_VIOLENCE);
    check_improve (ch, gsn_eviscerate, FALSE, 1);
    damage (ch, victim, 0, gsn_eviscerate, DAM_NONE, TRUE);
  }
  return;
}

void do_sap (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int chance;
  CHAR_DATA *victim;
  OBJ_DATA *sap;
  one_argument (argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char ("sap whom?\n\r", ch);
    return;
  }
  if (get_skill (ch, gsn_sap) < 1)
  {
    send_to_char ("You'd better leave that to assassins.\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char ("You are too peaceful to do that right now.\n\r", ch);
    return;
  }
  if (ch->fighting != NULL)
  {
    send_to_char ("Your victim sees you coming.\n\r", ch);
    return;
  }

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("You knock yourself silly.\n\r", ch);
    return;
  }
  if (IS_NPC (victim))
  {
    send_to_char ("You may only attempt to sap players.\n\r", ch);
    return;
  }
  if (IS_NPC (ch))
  {
    send_to_char ("Mobiles can't sap\r\n", ch);
    return;
  }
  if (ch->pcdata->primary_hand == HAND_AMBIDEXTROUS
    || ch->pcdata->primary_hand == HAND_RIGHT)
    sap = get_eq_char (ch, WEAR_WIELD_R);

  else
    sap = get_eq_char (ch, WEAR_WIELD_L);
  if (sap == NULL || (sap->pIndexData->vnum != OBJ_VNUM_SAP))
  {
    send_to_char ("A sap must be your primary weapon.\n\r", ch);
    return;
  }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (is_safe (ch, victim))
    return;
  if (victim->hit < (victim->max_hit - victim->max_hit / 20))
  {
    act ("$N is hurt and suspicious ... you can't sneak up.", ch,
      NULL, victim, TO_CHAR);
    return;
  }
  chance = get_skill (ch, gsn_sap);
  if (ch->level < victim->level)
    chance -= ((victim->level - ch->level) * 3);
  if ((number_percent () < chance || (chance >= 2 && !IS_AWAKE (victim))) && !IS_SET(victim->imm_flags, IMM_SLEEP) && !IS_SET(victim->act,ACT_UNDEAD))
  {
    if (awareness (ch, victim))
      return;
    act ("You sap $N squarely on the skull, knocking $M out cold!",
      ch, NULL, victim, TO_CHAR);
    act ("You feel a knock to the back of your head and you pass out.",
      ch, NULL, victim, TO_VICT);
    act ("$n saps $N, knocking $M unconscious.", ch, NULL, victim,
      TO_NOTVICT);
    send_to_char ("The sap breaks apart in your hands.\n\r", ch);
    extract_obj (sap);
    check_improve (ch, gsn_sap, TRUE, 1);
    WAIT_STATE (victim, 4 * PULSE_VIOLENCE);
    WAIT_STATE (ch, skill_table[gsn_bash].beats);
    victim->position = POS_SLEEPING;
    if (!is_affected (victim, gsn_mirror)
      && is_affected (victim, gsn_fireshield))
      if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
      {
        damage (victim, ch, 8, 0, DAM_FIRE, FALSE);
        act ("$N's scorching shield of flame burns you as you attack.",
          ch, NULL, victim, TO_CHAR);
        act ("$n is burnt by your scorching shield of flame.", ch, NULL,
          victim, TO_VICT);
      }
  }

  else
  {
    act
      ("You move in behind $N, but are caught before you can land the blow!",
      ch, NULL, victim, TO_CHAR);
    WAIT_STATE (ch, 4 * PULSE_VIOLENCE);
    if (IS_SET (ch->in_room->room_flags, ROOM_LAW) && ch->Class != PC_CLASS_CHAOS_JESTER && 
      !(battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner))))
    {
      send_to_char ("*** You are now WANTED.\n\r", ch);
      SET_BIT (ch->act, PLR_WANTED);
      wiznet ("$N has become wanted.", ch, NULL, WIZ_PENALTIES, 0, 0);
      ch->want_jail_time = 10;
    }
    send_to_char ("The sap breaks apart in your hands.\n\r", ch);
    extract_obj (sap);
    check_improve (ch, gsn_sap, FALSE, 1);
    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
      return;
    }

    damage (ch, victim, 0, gsn_sap, DAM_NONE, TRUE);
  }
  return;
}

void do_circle (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  int circle_skill;
  OBJ_DATA *obj;
  if (ch->fighting == NULL)
  {
    send_to_char ("But you aren't fighting anyone.\n\r", ch);
    return;
  }
  victim = ch->fighting;
  if (victim->fighting == ch)
  {
    act ("How do you expect to circle behind $M?", ch, NULL, victim,
      TO_CHAR);
    return;
  }
  if ((obj = get_eq_char (ch, WEAR_WIELD_R)) == NULL)
    if ((obj = get_eq_char (ch, WEAR_WIELD_L)) == NULL)
    {
      send_to_char ("You need to wield a weapon to circle attack.\n\r", ch);
      return;
    }
    circle_skill = get_skill (ch, gsn_circle);
    if (circle_skill >= 75)
      WAIT_STATE (ch, skill_table[gsn_circle].beats - PULSE_VIOLENCE);

    else
      WAIT_STATE (ch, skill_table[gsn_circle].beats);
    if (number_percent () < circle_skill || (circle_skill >= 2))
    {
      check_improve (ch, gsn_circle, TRUE, 1);
      if (IS_NPC(ch))
      {
        one_hit(ch,victim,gsn_circle);
        if (IS_AFFECTED(ch,AFF_HASTE))
          one_hit(ch,victim,gsn_circle);
      }
      else multi_hit (ch, victim, gsn_circle);
      if (!is_affected (victim, gsn_mirror)
        && is_affected (victim, gsn_fireshield))
        if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
        {
          damage (victim, ch, 8, 0, DAM_FIRE, FALSE);
          act ("$N's scorching shield of flame burns you as you attack.",
            ch, NULL, victim, TO_CHAR);
          act ("$n is burnt by your scorching shield of flame.", ch, NULL,
            victim, TO_VICT);
        }
    }

    else
    {
      check_improve (ch, gsn_circle, FALSE, 1);
      act ("You fail in your attempt to circle around $N.", ch, NULL,
        victim, TO_CHAR);
    }
    return;
}

void do_uppercut (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *weapon_l, *weapon_r;
  one_argument (argument, arg);
  if (get_skill (ch, gsn_uppercut) < 1 || (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_UPPERCUT) && !IS_SET(ch->act2,ACT_FAMILIAR)))
  {
    send_to_char ("You don't know how to uppercut!\n\r", ch);
    return;
  }
  if (arg[0] == '\0')
  {
    if (ch->fighting == NULL)
    {
      send_to_char ("Uppercut whom?\n\r", ch);
      return;
    }
    victim = ch->fighting;
  }

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("How can you uppercut yourself?\n\r", ch);
    return;
  }
  if (is_safe (ch, victim))
  {
    send_to_char ("You can't do that here.\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
  {
    act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
    return;
  }
  weapon_l = get_eq_char (ch, WEAR_WIELD_L);
  weapon_r = get_eq_char (ch, WEAR_WIELD_R);
  if (weapon_l != NULL && weapon_r != NULL)
  {
    send_to_char ("You can't uppercut while wielding two weapons.\n\r", ch);
    return;
  }
  if ((weapon_l != NULL && IS_WEAPON_STAT (weapon_l, WEAPON_TWO_HANDS))
    || (weapon_r != NULL && IS_WEAPON_STAT (weapon_r, WEAPON_TWO_HANDS)))
  {
    send_to_char
      ("You can't uppercut while holding a two-handed weapon.\n\r", ch);
    return;
  }
  if ((weapon_r != NULL || weapon_l != NULL) &&
    (get_eq_char (ch, WEAR_HOLD) != NULL
    || get_eq_char (ch, WEAR_SHIELD) != NULL))
  {
    send_to_char
      ("You don't have a free hand with which to uppercut.\n\r", ch);
    return;
  }

  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CALM)
    || is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char
      ("You are way too sedate to consider attacking anyone.\n\r", ch);
    return;
  }
  WAIT_STATE (ch, skill_table[gsn_uppercut].beats);
  if (number_percent () < get_skill (ch, gsn_uppercut)
    || (get_skill (ch, gsn_uppercut) >= 2 && !IS_AWAKE (victim)))
  {
    if (awareness (ch, victim))
      return;
    check_improve (ch, gsn_uppercut, TRUE, 1);
    one_hit (ch, victim, gsn_uppercut);
    if (!is_affected (victim, gsn_mirror)
      && is_affected (victim, gsn_fireshield))
      if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
      {
        damage (victim, ch, 8, 0, DAM_FIRE, FALSE);
        act ("$N's scorching shield of flame burns you as you attack.",
          ch, NULL, victim, TO_CHAR);
        act ("$n is burnt by your scorching shield of flame.", ch, NULL,
          victim, TO_VICT);
      }
  }

  else
  {
    AFFECT_DATA af;
    af.where = TO_AFFECTS;
    af.type = gsn_uppercut;
    af.level = ch->level;
    af.duration = 3;
    af.location = APPLY_STR;
    af.modifier = -1;
    af.bitvector = 0;
    af.permaff = FALSE;
    af.composition = FALSE;
    af.comp_name = str_dup ("");
    affect_join (ch, &af);
    check_improve (ch, gsn_uppercut, FALSE, 1);
    damage (ch, victim, 0, gsn_uppercut, DAM_NONE, TRUE);
  }
  return;
}

void do_kai (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int kai_skill;
  time_t kai_passed;
  kai_skill = get_skill (ch, gsn_kai);
  if (kai_skill <= 1 || (IS_NPC (ch) && !IS_SET(ch->act2,ACT_FAMILIAR)))
  {
    send_to_char ("You have no idea what kai is.\n\r", ch);
    return;
  }
  one_argument (argument, arg);
  if (arg[0] == '\0')
  {
    if (ch->fighting == NULL)
    {
      send_to_char ("Kai attack whom?\n\r", ch);
      return;
    }
    victim = ch->fighting;
  }

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("How can you kai attack yourself?\n\r", ch);
    return;
  }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (is_safe (ch, victim))
  {
    send_to_char ("You can't do that here.\n\r", ch);
    return;
  }

  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  } 

  if (IS_AFFECTED (ch, AFF_CALM)
    || is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char ("You're feeling too mellow to frighten ANYONE.\n\r", ch);
    return;
  }
  if (ch->fighting == NULL)
    set_fighting (victim, ch);
  else update_aggressor(ch,victim);
  if (kai_skill >= 75)
    WAIT_STATE (ch, 8);

  else
    WAIT_STATE (ch, 12);

  //Shinowlan 3/2/99 Prevent Kai spamming 
  //if the time passed between kai attempts is less than the lag imposed 
  //on kai victims, then the kai fails.  This ensures that a player can not
  // be "locked" in kai spam.
  if (!IS_NPC(ch))
    kai_passed = (current_time - ch->pcdata->last_kai);
  else
    kai_passed = 1000;
  if (kai_passed < ((skill_table[gsn_kai].beats / PULSE_PER_SECOND) + 1))
  {
    send_to_char ("Alas, you are mentally exhausted.\n\r", ch);
    return;
  }
  if (!IS_NPC(ch))
    ch->pcdata->last_kai = current_time;
  if (number_percent () < kai_skill || (kai_skill >= 2 && !IS_AWAKE (victim)))
  {
    if (is_affected (victim, gsn_mirror))
    {
      if (number_percent () < get_skill (ch, gsn_kai) / 2)
      {
        act
          ("The mirror around $N shatters into a million slivers of glass.",
          ch, NULL, victim, TO_CHAR);
        act
          ("The mirror around $N shatters into a million slivers of glass.",
          ch, NULL, victim, TO_NOTVICT);
        act
          ("The mirror around you shatters into a million slivers of glass.",
          ch, NULL, victim, TO_VICT);
        affect_strip (victim, gsn_mirror);
        return;
      }
      act
        ("Your scream, echoed back via $N's mirror, paralyzes you with fear.",
        ch, NULL, victim, TO_CHAR);
      act
        ("$n's scream, echoed back via your mirror, paralyzes $m with fear.",
        ch, NULL, victim, TO_VICT);
      act
        ("$n's scream, echoed back via $N's mirror, paralyzes $m with fear.",
        ch, NULL, victim, TO_NOTVICT);
      WAIT_STATE (ch, skill_table[gsn_kai].beats);
      return;
    }
    act ("You scream at $N, paralyzing $M with fear.", ch, NULL,
      victim, TO_CHAR);
    act ("$n screams at you, paralyzing you with fear.", ch, NULL,
      victim, TO_VICT);
    act ("$n screams at $N, paralyzing $M with fear.", ch, NULL,
      victim, TO_NOTVICT);
    check_improve (ch, gsn_kai, TRUE, 1);
    WAIT_STATE (victim, skill_table[gsn_kai].beats);
  }

  else
  {
    check_improve (ch, gsn_kai, FALSE, 1);
    damage (ch, victim, 0, gsn_kai, DAM_NONE, TRUE);
  }
  return;
}

void do_flee (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *was_in;
  ROOM_INDEX_DATA *now_in;
  CHAR_DATA *victim, *fch;
  int attempt;
  long exp_lost;

  if ((victim = ch->fighting) == NULL)
  {
    if (ch->position == POS_FIGHTING)
      ch->position = POS_STANDING;
    send_to_char ("You aren't fighting anyone.\n\r", ch);
    return;
  }
  if (ch->stunned > 0)
    return;
  if (is_affected(ch,skill_lookup("demonic possession")))
  {
    send_to_char("The demonic being inside you does not want to flee.\n\r",ch);
    return;
  }
  if (ch->position < POS_FIGHTING)
  {
    send_to_char ("You try to flee, but you have to stand first\n\r.", ch);
    return;
  }
  if (!check_web (ch))
    return;
  if (!check_entangle (ch))
    return;
  if (!check_clasp (ch))
    return;
  if (((!IS_NPC (ch->fighting) && ch->fighting->pcdata->prevent_escape) || IS_SET(ch->fighting->act2,ACT_MONK))
    && number_percent () < (get_skill (ch->fighting, gsn_prevent_escape)* .60))
  {
    prevent_escape (ch->fighting, ch);
    return;
  }
  was_in = ch->in_room;
  if (get_skill (ch, gsn_elusiveness) > number_percent ())
  {
    if (argument[0] != '\0')
    {
      no_check_skeletal = TRUE;
      //stop_fighting (ch, TRUE);
      if (!str_cmp (argument, "n") || !str_cmp (argument, "north"))
        move_char (ch, DIR_NORTH, FALSE);

      else if (!str_cmp (argument, "e") || !str_cmp (argument, "east"))
        move_char (ch, DIR_EAST, FALSE);

      else if (!str_cmp (argument, "s") || !str_cmp (argument, "south"))
        move_char (ch, DIR_SOUTH, FALSE);

      else if (!str_cmp (argument, "w") || !str_cmp (argument, "west"))
        move_char (ch, DIR_WEST, FALSE);

      else if (!str_cmp (argument, "u") || !str_cmp (argument, "up"))
        move_char (ch, DIR_UP, FALSE);

      else if (!str_cmp (argument, "d") || !str_cmp (argument, "down"))
        move_char (ch, DIR_DOWN, FALSE);

      else
      {
        argument[2] = '\0';
        do_speedwalk (ch, argument);
      }
      no_check_skeletal = FALSE;

      //return;
    }
    if ((now_in = ch->in_room) != was_in)
    {
      CHAR_DATA *cfighting=NULL;
      if (ch->fighting == NULL)
        return;
      if (!IS_NPC(ch) && (IS_SET(ch->fighting->act,ACT_MAGE) || IS_SET(ch->fighting->act2,ACT_NECROMANCER)))
        cfighting = ch->fighting;
      stop_fighting (ch, TRUE);

      // Shinowlan 8/30/98 -- Check person fled from to see if we
      // need to clear their pending output.  This was put in
      // to deal with kai, flee, reenter abuse.
      if (!IS_NPC (victim) && (victim->wait > 0))
      {
        victim->desc->inbuf[0] = '\0';	//clear queued input
      }
      check_improve (ch, gsn_elusiveness, TRUE, 2);
      send_to_char ("You flee from combat!\n\r", ch);
      ch->in_room = was_in;
      act ("$n has fled!", ch, NULL, NULL, TO_ROOM);
      ch->in_room = now_in;
      trip_triggers(ch, OBJ_TRIG_CHAR_FLEES, NULL, victim, OT_SPEC_NONE);
      trip_triggers(victim, OBJ_TRIG_VICT_FLEES, NULL, ch, OT_SPEC_NONE);
      check_aggression(ch);
      check_skeletal_mutation(ch,FALSE);
      if (cfighting && cfighting->fighting == NULL && ch->hit > 1)
      {
        if (IS_SET(cfighting->act,ACT_MAGE))
        {
          if (number_percent() < 31)
          {
            char buf[MAX_STRING_LENGTH];
            sprintf(buf,"cast summon %s",ch->name);
            interpret (cfighting, buf);
          }
        }
        else
        {
          if (number_percent() < 50)
            interpret (cfighting, "chant 'ghost form'");
          if (number_percent() > 50)
            interpret (cfighting, "chant darkhounds");
        }
      }
      return;
    }
  }
  for (attempt = 0; attempt < 6; attempt++)
  {
    EXIT_DATA *pexit;
    int door;
    CHAR_DATA *cfighting=NULL;
    door = number_door ();
    if ((pexit = was_in->exit[door]) == 0
      || pexit->u1.to_room == NULL
      || IS_SET (pexit->exit_info, EX_CLOSED)
      || number_range (0, ch->daze) != 0 || (IS_NPC (ch)
      && IS_SET (pexit->u1.
      to_room->
      room_flags,
      ROOM_NO_MOB)))
      continue;

    //
    // stop_fighting (ch, TRUE);
    /////   move_char (ch, door, FALSE);
    //act ("$n has fled!", ch, NULL, NULL, TO_ROOM);
    //ch->in_room = now_in;
    exp_lost = (ch->fighting->level * ch->fighting->max_hit) / 20.0;
    no_check_skeletal = TRUE;
    move_char (ch, door, FALSE);
    no_check_skeletal = FALSE;
    if ((now_in = ch->in_room) == was_in)
      continue;


    /*    switch (door) 
    {
    case 0: move_char (ch,1);break;
    case 1: move_char (ch,0);break;
    case 2: move_char (ch,3);break;
    case 3: move_char (ch,2);break;
    case 4: move_char (ch,5);break;
    case 5: move_char (ch 4);break;
    default :move_char (ch,0);break;
    } */
    //      move_char (ch, 
    ch->in_room = was_in;
    act ("$n has fled!", ch, NULL, NULL, TO_ROOM);
    ch->in_room = now_in;
    if (ch->mount != NULL)
    {
      int mod;
      int new_move;
      switch (get_curr_stat (ch, STAT_DEX))
      {
      default:
        mod = 6;
        break;
      case 16:
      case 17:
      case 18:
        mod = 7;
        break;
      case 19:
      case 20:
      case 21:
      case 22:
        mod = 8;
        break;
      case 23:
      case 24:
      case 25:
        mod = 9;
        break;
      }
      new_move = ch->move * mod / 10;
      ch->move = UMAX (1, new_move);
    }
    //ch->in_room = was_in;
    // Shinowlan -- 8/30/98 Prevent victims queued commands from
    // being executed after this character flees. This is esp.
    // relevant for the kai skill.
    if (!IS_NPC (victim) && (victim->wait > 0))
    {
      victim->desc->inbuf[0] = '\0';	//clear queued input
    }
    //act ("$n has fled!", ch, NULL, NULL, TO_ROOM);
    //ch->in_room = now_in;
    if (!IS_NPC (ch))
    {
      send_to_char ("You flee from combat!\n\r", ch);
      if (get_skill (ch, gsn_elusiveness) > number_percent ())
        send_to_char ("You snuck away safely.\n\r", ch);

      else if (!(battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner))))
      {

        //            exp_lost = (ch->fighting->level * ch->fighting->max_hit) / 20.0;
        if (exp_lost < 1)
          exp_lost = 1;
        if (ch->level < 2)
          exp_lost = 0;
        if (ch->exp + exp_lost < 0)
          exp_lost = 0 - ch->exp;

        //sprintf(buf, "You lost %ld exp.\n\r", exp_lost);
        //send_to_char (buf, ch);
        gain_exp (ch, -exp_lost);
      }
    }
    //stop_fighting (ch, TRUE);
    //move_char (ch, door, FALSE);    
    //      cfighting = ch->fighting;
    if (ch->fighting == NULL)
      return;
    if (!IS_NPC(ch) && (IS_SET(ch->fighting->act,ACT_MAGE) || IS_SET(ch->fighting->act2,ACT_NECROMANCER)))
      cfighting = ch->fighting;
    stop_fighting (ch, TRUE);
    trip_triggers(ch, OBJ_TRIG_CHAR_FLEES, NULL, victim, OT_SPEC_NONE);
    trip_triggers(victim, OBJ_TRIG_VICT_FLEES, NULL, ch, OT_SPEC_NONE);
    for (fch = ch->in_room->people; fch; fch = fch->next_in_room)
    {
      if (IS_NPC (fch) && is_hunting (fch, ch))
      {
        fch->position = POS_STANDING;
        do_kill (fch, ch->name);
      }
    }
    check_aggression(ch);
    check_skeletal_mutation(ch,FALSE);	       
    if (cfighting && cfighting->fighting == NULL && ch->hit > 1)
    {
      if (IS_SET(cfighting->act,ACT_MAGE))
      {
        if (number_percent() < 31)
        {
          char buf[MAX_STRING_LENGTH];
          sprintf(buf,"cast summon %s",ch->name);
          interpret (cfighting, buf);
        }
      }
      else
      {
        if (number_percent() < 50)
          interpret (cfighting, "chant 'ghost form'");
        if (number_percent() > 50)
          interpret (cfighting, "chant darkhounds");
      }
    }
    return;
  }
  send_to_char ("PANIC! You couldn't escape!\n\r", ch);
  return;
}

void do_retreat (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *was_in;
  ROOM_INDEX_DATA *now_in;
  CHAR_DATA *victim, *fch;
  int attempt;
  long exp_lost;

  //  char buf[MAX_STRING_LENGTH];
  if ((victim = ch->fighting) == NULL)
  {
    if (ch->position == POS_FIGHTING)
      ch->position = POS_STANDING;
    send_to_char ("You aren't fighting anyone.\n\r", ch);
    return;
  }
  if (is_affected(ch,skill_lookup("demonic possession")))
  {
    send_to_char("The demonic being inside you does not want to flee.\n\r",ch);
    return;
  }

  if (!check_web (ch))
    return;
  if (!check_entangle (ch))
    return;
  if (!check_clasp (ch))
    return;  
  was_in = ch->in_room;
  if (get_skill (ch, gsn_retreat) <= 0)
  {
    send_to_char ("You don't know how to retreat.\n\r", ch);
    return;
  }
  if (!((!IS_NPC (ch->fighting)
    && !ch->fighting->pcdata->
    prevent_escape) || number_percent () >= get_skill (ch->fighting,
    gsn_prevent_escape)
    * .60))
  {
    prevent_escape (ch->fighting, ch);
    return;
  }
  if (get_skill (ch, gsn_retreat) > number_percent ())
  {
    if (argument[0] != '\0')
    {
      no_check_skeletal = TRUE;
      if (!str_cmp (argument, "n") || !str_cmp (argument, "north"))
        move_char (ch, DIR_NORTH, FALSE);

      else if (!str_cmp (argument, "e") || !str_cmp (argument, "east"))
        move_char (ch, DIR_EAST, FALSE);

      else if (!str_cmp (argument, "s") || !str_cmp (argument, "south"))
        move_char (ch, DIR_SOUTH, FALSE);

      else if (!str_cmp (argument, "w") || !str_cmp (argument, "west"))
        move_char (ch, DIR_WEST, FALSE);

      else if (!str_cmp (argument, "u") || !str_cmp (argument, "up"))
        move_char (ch, DIR_UP, FALSE);

      else if (!str_cmp (argument, "d") || !str_cmp (argument, "down"))
        move_char (ch, DIR_DOWN, FALSE);
      no_check_skeletal = FALSE;
    }
    if ((now_in = ch->in_room) != was_in)
    {
      CHAR_DATA *cfighting=NULL;
      if (ch->fighting == NULL)
        return;
      if (!IS_NPC(ch) && (IS_SET(ch->fighting->act,ACT_MAGE) || IS_SET(ch->fighting->act2,ACT_NECROMANCER)))
        cfighting = ch->fighting;
      stop_fighting (ch, TRUE);

      // IBLIS 5/31/03 - Added below check to retreat
      // Shinowlan 8/30/98 -- Check person fled from to see if we
      // need to clear their pending output.  This was put in
      // to deal with kai, flee, reenter abuse.
      if (!IS_NPC (victim) && (victim->wait > 0))
      {
        victim->desc->inbuf[0] = '\0';	//clear queued input
      }
      check_improve (ch, gsn_retreat, TRUE, 2);
      now_in = ch->in_room;
      send_to_char ("You retreat from combat!\n\r", ch);
      ch->in_room = was_in;
      act ("$n has retreated!", ch, NULL, NULL, TO_ROOM);
      ch->in_room = now_in;
      trip_triggers(ch, OBJ_TRIG_CHAR_FLEES, NULL, victim, OT_SPEC_NONE);
      trip_triggers(victim, OBJ_TRIG_VICT_FLEES, NULL, ch, OT_SPEC_NONE);
      check_aggression(ch);
      check_skeletal_mutation(ch,FALSE);
      //}
      // }
      //else
      //  {
      // /   if (number_percent() < 50)
      //     interpret (ch, "chant 'ghost form'");
      //if (number_percent() > 50)
      //      interpret (ch, "chant darkhounds");
      //  }
      //}
      if (cfighting && cfighting->fighting == NULL && ch->hit > 1)
      {
        if (IS_SET(cfighting->act,ACT_MAGE))
        {
          if (number_percent() < 31)
          {
            char buf[MAX_STRING_LENGTH];
            sprintf(buf,"cast summon %s",ch->name);
            interpret (cfighting, buf);
          }
        }
        else
        {
          if (number_percent() < 50)
            interpret (cfighting, "chant 'ghost form'");
          if (number_percent() > 50)
            interpret (cfighting, "chant darkhounds");
        }
      }

      return;
    }
  }
  for (attempt = 0; attempt < 6; attempt++)
  {
    EXIT_DATA *pexit;
    int door;
    CHAR_DATA *cfighting = NULL;
    door = number_door ();
    if ((pexit = was_in->exit[door]) == 0
      || pexit->u1.to_room == NULL
      || IS_SET (pexit->exit_info, EX_CLOSED)
      || number_range (0, ch->daze) != 0 || (IS_NPC (ch)
      && IS_SET (pexit->u1.
      to_room->
      room_flags,
      ROOM_NO_MOB)))
      continue;
    exp_lost = (ch->fighting->level * ch->fighting->max_hit) / 20.0;

    //move_char (ch, door, FALSE);
    if ((now_in = ch->in_room) == was_in)
      continue;
    ch->in_room = was_in;
    act ("$n has fled!", ch, NULL, NULL, TO_ROOM);
    ch->in_room = now_in;
    if (!IS_NPC (ch))
    {
      send_to_char ("You flee from combat!\n\r", ch);
      if (get_skill (ch, gsn_elusiveness) > number_percent ())
        send_to_char ("You snuck away safely.\n\r", ch);

      else if (!(battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner))))
      {

        //            exp_lost = (ch->fighting->level * ch->fighting->max_hit) / 20.0;
        if (exp_lost < 1)
          exp_lost = 1;
        if (ch->level < 2)
          exp_lost = 0;
        if (ch->exp + exp_lost < 0)
          exp_lost = 0 - ch->exp;

        //sprintf(buf, "You lost %ld exp.\n\r", exp_lost);
        //send_to_char (buf, ch);
        gain_exp (ch, -exp_lost);
      }
    }
    if (ch->fighting == NULL)
      return;
    if (!IS_NPC(ch) && (IS_SET(ch->fighting->act,ACT_MAGE) || IS_SET(ch->fighting->act2,ACT_NECROMANCER)))
      cfighting = ch->fighting;
    stop_fighting (ch, TRUE);
    no_check_skeletal = TRUE;
    move_char (ch, door, FALSE);
    no_check_skeletal = FALSE;
    trip_triggers(ch, OBJ_TRIG_CHAR_FLEES, NULL, victim, OT_SPEC_NONE);
    trip_triggers(victim, OBJ_TRIG_VICT_FLEES, NULL, ch, OT_SPEC_NONE);
    for (fch = ch->in_room->people; fch; fch = fch->next_in_room)
    {
      if (IS_NPC (fch) && is_hunting (fch, ch))
      {
        fch->position = POS_STANDING;
        do_kill (fch, ch->name);
      }
    }
    check_aggression(ch);
    check_skeletal_mutation(ch,FALSE);
    if (cfighting && cfighting->fighting == NULL && ch->hit > 1)
    {
      if (IS_SET(cfighting->act,ACT_MAGE))
      {
        if (number_percent() < 31)
        {
          char buf[MAX_STRING_LENGTH];
          sprintf(buf,"cast summon %s",ch->name);
          interpret (cfighting, buf);
        }
      }
      else
      {
        if (number_percent() < 50)
          interpret (cfighting, "chant 'ghost form'");
        if (number_percent() > 50)
          interpret (cfighting, "chant darkhounds");
      }
    }

    return;
  }
  send_to_char ("PANIC! You couldn't escape!\n\r", ch);
  return;
}

void do_rescue (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *fch;
  one_argument (argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char ("Rescue whom?\n\r", ch);
    return;
  }
  if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("What about fleeing instead?\n\r", ch);
    return;
  }
  if (victim->race == PC_RACE_SWARM)
  {
    send_to_char ("You can't figure out how to rescue a swarm.\n\r",ch);
    return;
  }
  if (!IS_NPC (ch) && IS_NPC (victim) && !is_same_group (ch, victim))
  {
    send_to_char ("Doesn't need your help!\n\r", ch);
    return;
  }
  if (ch->fighting == victim)
  {
    send_to_char ("Too late.\n\r", ch);
    return;
  }
  if ((fch = victim->fighting) == NULL)
  {
    send_to_char ("That person is not fighting right now.\n\r", ch);
    return;
  }
  if (fch->fighting != victim)
  {
    send_to_char ("You cannot rescue a player that is not tanking.\n\r", ch);
    return;
  }
  if (!IS_NPC(fch) && (!IS_NPC(ch) && !ch->pcdata->loner && ch->clan == CLAN_BOGUS))
  {
    send_to_char ("You cannot interfere with pk.\n\r",ch);
    return;
  }

  WAIT_STATE (ch, skill_table[gsn_rescue].beats);
  if (number_percent () > get_skill (ch, gsn_rescue))
  {
    send_to_char ("You fail the rescue.\n\r", ch);
    check_improve (ch, gsn_rescue, FALSE, 1);
    return;
  }
  act ("You rescue $N!", ch, NULL, victim, TO_CHAR);
  act ("$n rescues you!", ch, NULL, victim, TO_VICT);
  act ("$n rescues $N!", ch, NULL, victim, TO_NOTVICT);
  check_improve (ch, gsn_rescue, TRUE, 1);
  stop_fighting (fch, FALSE);
  stop_fighting (victim, FALSE);
  update_aggressor(ch,fch);
  set_fighting (ch, fch);
  set_fighting (fch, ch);
  return;
}

void do_kick (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  if (!IS_NPC (ch) && ch->level < level_for_skill (ch, gsn_kick))
  {
    send_to_char ("You better leave the martial arts to fighters.\n\r", ch);
    return;
  }
  if (get_skill (ch, gsn_kick) < 1)
  {
    send_to_char ("You don't know how to kick!\n\r", ch);
    return;
  }
  if (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_KICK))
    return;
  if ((victim = ch->fighting) == NULL)
  {
    send_to_char ("You aren't fighting anyone.\n\r", ch);
    return;
  }
  WAIT_STATE (ch, skill_table[gsn_kick].beats);
  if (get_skill (ch, gsn_kick) > number_percent ())
  {
    if (ch->race == PC_RACE_NERIX)
      damage (ch, victim, number_range (1, ch->level) + ch->level/2, gsn_kick, DAM_BASH, TRUE);
    else damage (ch, victim, ((!IS_CLASS (ch, PC_CLASS_MONK)) ? number_range (1, ch->level)
      : (number_range (1, ch->level)) * 2), gsn_kick, DAM_BASH, TRUE);
    check_improve (ch, gsn_kick, TRUE, 1);
    if (!is_affected (victim, gsn_mirror)
      && is_affected (victim, gsn_fireshield))
      if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
      {
        damage (victim, ch, number_range (1, ch->level) / 2, 0,
          DAM_FIRE, FALSE);
        act ("$N's scorching shield of flame burns you as you attack.",
          ch, NULL, victim, TO_CHAR);
        act ("$n is burnt by your scorching shield of flame.", ch, NULL,
          victim, TO_VICT);
      }
  }

  else
  {
    damage (ch, victim, 0, gsn_kick, DAM_BASH, TRUE);
    check_improve (ch, gsn_kick, FALSE, 1);
  }
  return;
}

void do_disarm (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int chance, hth, ch_weapon, vict_weapon, ch_vict_weapon;
  int wield_type = WEAR_WIELD_R, cwield_type = WEAR_WIELD_R;
  hth = 0;
  if ((chance = get_skill (ch, gsn_disarm)) == 0)
  {
    send_to_char ("You don't know how to disarm opponents.\n\r", ch);
    return;
  }
  if (get_eq_char (ch, WEAR_WIELD_R) == NULL && ch->race != PC_RACE_NERIX)
  {
    cwield_type = WEAR_WIELD_L;
    if (get_eq_char (ch, WEAR_WIELD_L) == NULL
      && ((hth = get_skill (ch, gsn_hand_to_hand)) == 0
      || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_DISARM))))
    {
      send_to_char ("You must wield a weapon to disarm.\n\r", ch);
      return;
    }
  }
  if ((victim = ch->fighting) == NULL)
  {
    send_to_char ("You aren't fighting anyone.\n\r", ch);
    return;
  }
  if ((obj = get_eq_char (victim, WEAR_WIELD_R)) == NULL)
  {
    wield_type = WEAR_WIELD_L;
    if ((obj = get_eq_char (victim, WEAR_WIELD_L)) == NULL)
    {
      send_to_char ("Your opponent is not wielding a weapon.\n\r", ch);
      return;
    }
  }

  /* find weapon skills */
  ch_weapon = get_weapon_skill (ch, get_weapon_sn (ch, cwield_type));
  vict_weapon = get_weapon_skill (victim, get_weapon_sn (victim, wield_type));
  ch_vict_weapon = get_weapon_skill (ch, get_weapon_sn (victim, wield_type));

  if (ch->race == PC_RACE_NERIX)
    ch_weapon = number_range(50,100);
  /* modifiers */

  /* skill */
  if (get_eq_char (ch, WEAR_WIELD_R) == NULL)
  {
    if (get_eq_char (ch, WEAR_WIELD_L) == NULL)
      chance = chance * hth / 150;

    else
      chance = chance * ch_weapon / 100;
  }
  chance += (ch_vict_weapon / 2 - vict_weapon) / 2;

  /*Iblis 12-30-03 Added to make disarm work a little better*/
  if (get_skill (ch, gsn_disarm) == 100)
    chance += number_range(1,20);

  /* dex vs. strength */
  chance += get_curr_stat (ch, STAT_DEX);
  chance -= 2 * get_curr_stat (victim, STAT_STR);

  /* level */
  chance += (ch->level - victim->level) * 2;
  if (is_affected (victim, gsn_mirror))
  {
    act
      ("Your attempt to disarm $N is deflected by the magic mirror surrounding $M.",
      ch, NULL, victim, TO_CHAR);
    act
      ("$n's attempt to disarm you is deflected by the magic mirror surrounding you.",
      ch, NULL, victim, TO_VICT);
    act
      ("$n's attempt to disarm $N is deflected by the magic mirror surrounding $M.",
      ch, NULL, victim, TO_NOTVICT);
    return;
  }

  /* and now the attack */
  if (number_percent () < chance && !IS_IMMORTAL(victim))
  {
    WAIT_STATE (ch, skill_table[gsn_disarm].beats);
    disarm (ch, victim);
    check_improve (ch, gsn_disarm, TRUE, 1);
  }

  else
  {
    WAIT_STATE (ch, skill_table[gsn_disarm].beats);
    act ("You fail to disarm $N.", ch, NULL, victim, TO_CHAR);
    act ("$n tries to disarm you, but fails.", ch, NULL, victim, TO_VICT);
    act ("$n tries to disarm $N, but fails.", ch, NULL, victim, TO_NOTVICT);
    check_improve (ch, gsn_disarm, FALSE, 1);
  }
  return;
}

void do_sla (CHAR_DATA * ch, char *argument)
{
  send_to_char ("If you want to SLAY, spell it out.\n\r", ch);
  return;
}

void do_slay (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  one_argument (argument, arg);
  bug ("**** SLAY DEBUG ****",0);
  bug (arg,0);

  if (arg[0] == '\0')
  {
    send_to_char ("Slay whom?\n\r", ch);
    return;
  }
  if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    bug ("get_char_room returned null",0);
    return;
  }
  if (ch == victim)
  {
    send_to_char ("Suicide is a mortal sin.\n\r", ch);
    return;
  }
  if (!IS_NPC (victim) && victim->level >= get_trust (ch))
  {
    send_to_char ("You failed.\n\r", ch);
    return;
  }
  act ("You slay $M in cold blood!", ch, NULL, victim, TO_CHAR);
  act ("$n slays you in cold blood!", ch, NULL, victim, TO_VICT);
  act ("$n slays $N in cold blood!", ch, NULL, victim, TO_NOTVICT);
  raw_kill (victim, victim);
  return;
}

//Iblis - changed damage message via Iverath's request
void
dam_message (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, bool immune)
{
  char buf1[256], buf2[256], buf3[256];
  const char *vs;
  const char *vp;
  const char *str;
  const char *attack;
  char punct;
  if (ch == NULL || victim == NULL || nodam_message)
    return;
  if (dam == 0)
  {
    vs = "miss";
    vp = "misses";
  }

  else if (dam <= 8)
  {
    vs = "barely touch";
    vp = "barely touches";
  }

  else if (dam <= 15)
  {
    vs = "nick";
    vp = "nicks";
  }


  else if (dam <= 20)
  {
    vs = "scratch";
    vp = "scratches";
  }

  else if (dam <= 27)
  {
    vs = "hit";
    vp = "hits";
  }

  else if (dam <= 34)
  {
    vs = "injure";
    vp = "injures";
  }

  else if (dam <= 41)
  {
    vs = "wound";
    vp = "wounds";
  }

  else if (dam <= 48)
  {
    vs = "maul";
    vp = "mauls";
  }

  else if (dam <= 55)
  {
    vs = "decimate";
    vp = "decimates";
  }

  else if (dam <= 62)
  {
    vs = "maim";
    vp = "maims";
  }

  else if (dam <= 69)
  {
    vs = "MUTILATE";
    vp = "MUTILATES";
  }

  else if (dam <= 76)
  {
    vs = "DISEMBOWEL";
    vp = "DISEMBOWELS";
  }

  else if (dam <= 83)
  {
    vs = "DISMEMBER";
    vp = "DISMEMBERS";
  }

  else if (dam <= 90)
  {
    vs = "MASSACRE";
    vp = "MASSACRES";
  }

  else if (dam <= 97)
  {
    vs = "MANGLE";
    vp = "MANGLES";
  }

  else if (dam <= 104)
  {
    vs = "*** DEMOLISH ***";
    vp = "*** DEMOLISHES ***";
  }

  else if (dam <= 115)
  {
    vs = "*** DEVASTATE ***";
    vp = "*** DEVASTATES ***";
  }

  else if (dam <= 125)
  {
    vs = "=== OBLITERATE ===";
    vp = "=== OBLITERATES ===";
  }

  else if (dam <= 135)
  {
    vs = ">>> ANNIHILATE <<<";
    vp = ">>> ANNIHILATES <<<";
  }

  else if (dam <= 150)
  {
    vs = "<<< ERADICATE >>>";
    vp = "<<< ERADICATES >>>";
  }

  else if (dam <= 170)
  {
    vs = "--- `iDESTROY`` ---";
    vp = "--- `iDESTROYS`` ---";
  }

  else if (dam <= 190)
  {
    vs = "--- `iPULVERIZE`` ---";
    vp = "--- `iPULVERIZES`` ---";
  }

  else if (dam <= 250)
  {
    vs = "do UNSPEAKABLE things to";
    vp = "does UNSPEAKABLE things to";
  }

  else if (dam <= 350)
  {
    vs = "~~~ LIQUIFY ~~~";
    vp = "~~~ LIQUIFIES ~~~";
  }

  else if (dam <= 450)
  {
    vs = "~~~ VAPORIZE ~~~";
    vp = "~~~ VAPORIZES ~~~";
  }

  else 
  {
    vs = "do UNGODLY DAMAGE to";
    vp = "does UNGODLY DAMAGE to";
  }





  punct = (dam <= 24) ? '.' : '!';
  if (immune == 3 && dam != 0)
  {
    if (dt == TYPE_HIT)
    {
      sprintf (buf1,
        "$n %s $mself as $s attack reflects off of $N's mirror%c",
        vp, punct);
      sprintf (buf2,
        "Your attack reflects off of $N's mirror and `A%s`` you%c",
        vp, punct);
      sprintf (buf3,
        "$n's attack reflects off of your mirror and `B%s`` $m%c",
        vp, punct);
    }

    else
    {
      if (dt >= 0 && dt < MAX_SKILL)
        attack = skill_table[dt].noun_damage;

      else if (dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
        attack = attack_table[dt - TYPE_HIT].noun;

      else
      {
        bug ("Dam_message: bad dt %d.", dt);
        dt = TYPE_HIT;
        attack = attack_table[0].name;
      }
      if (is_tail_attack)
        attack = "lightning tail";
      sprintf (buf1,
        "$n %s $mself as $s %s reflects off of $N's mirror%c",
        vp, attack, punct);
      sprintf (buf2,
        "Your %s reflects off of $N's mirror and `A%s`` you%c",
        attack, vp, punct);
      sprintf (buf3,
        "$n's %s reflects off of your mirror and `B%s`` $m%c",
        attack, vp, punct);
    }
    if (ch == victim)
    {
      act (buf1, ch, NULL, victim, TO_ROOM);
      act (buf2, ch, NULL, victim, TO_CHAR);
    }

    else
    {
      act (buf1, ch, NULL, victim, TO_NOTVICT);
      act (buf2, ch, NULL, victim, TO_CHAR);
      act (buf3, ch, NULL, victim, TO_VICT);
    }
    return;
  }
  if (dt == TYPE_HIT)
  {
    if (ch == victim)
    {
      sprintf (buf1, "$n %s $melf%c", vp, punct);
      sprintf (buf2, "You %s yourself%c", vs, punct);
    }

    else
    {
      sprintf (buf1, "$n %s $N%c", vp, punct);
      sprintf (buf2, "You `A%s`` $N%c", vs, punct);
      sprintf (buf3, "$n `B%s`` you%c", vp, punct);
    }
  }

  else if (dt != gsn_counter)
  {
    if (dt >= 0 && dt < MAX_SKILL)
      attack = skill_table[dt].noun_damage;

    else if (dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
      attack = attack_table[dt - TYPE_HIT].noun;

    else
    {
      bug ("Dam_message: bad dt %d.", dt);
      dt = TYPE_HIT;
      attack = attack_table[0].name;
    }
    if (is_tail_attack)
      attack = "lightning tail";
    if (immune)
    {
      if (ch == victim)
      {
        sprintf (buf1, "$n is unaffected by $s own %s.", attack);
        sprintf (buf2, "Luckily, you are immune to that.");
      }

      else
      {
        sprintf (buf1, "$N is unaffected by $n's %s!", attack);
        sprintf (buf2, "$N is unaffected by your %s!", attack);
        sprintf (buf3, "$n's %s is powerless against you.", attack);
      }
    }

    else
    {
      if (ch == victim)
      {
        sprintf (buf1, "$n's %s %s $m%c", attack, vp, punct);
        sprintf (buf2, "Your %s %s you%c", attack, vp, punct);
      }

      else
      {
        sprintf (buf1, "$n's %s %s $N%c", attack, vp, punct);
        sprintf (buf2, "Your %s `A%s`` $N%c", attack, vp, punct);
        sprintf (buf3, "$n's %s `B%s`` you%c", attack, vp, punct);
      }
    }
  }

  else
  {
    if (number_percent () < 50)
      sprintf (buf1,
      "You avoid the attack and your counter attack `A%s`` $N%c",
      vp, punct);

    else
      sprintf (buf1,
      "You block $N's attack and your counter attack `A%s`` $N%c",
      vp, punct);
    act (buf1, ch, NULL, victim, TO_CHAR);
    return;
  }
  if (ch == victim)
  {
    act (buf1, ch, NULL, NULL, TO_ROOM);
    act (buf2, ch, NULL, NULL, TO_CHAR);
  }

  else
  {

    //IBLIS 5/31/03 - Monk's kick send specific messages
    if ((IS_CLASS (ch, PC_CLASS_MONK) || IS_SET(ch->act2,ACT_MONK))&& dt == gsn_kick)
    {
      int some_number = 0;
      some_number = number_percent ();
      if (some_number < 41)
      {
        act ("$n whirls and slams $s foot into $N's face.", ch,
          NULL, victim, TO_NOTVICT);
        act
          ("$n spins and as you turn to block the incoming kick you are greeted by a foot in the face.",
          ch, NULL, victim, TO_VICT);
      }

      else if (some_number < 81)
      {
        act
          ("$n swings $s foot high up in the air and with a snap $e drops $s heel down on $N's collar bone.",
          ch, NULL, victim, TO_NOTVICT);
        act
          ("$n swings $s foot high up in the air and with a snap $e drops $s heel down on your collar bone.",
          ch, NULL, victim, TO_VICT);
      }

      else
      {
        act ("With a loud crack $n kicks the inside of $N's knee.",
          ch, NULL, victim, TO_NOTVICT);
        act
          ("One of your legs is rendered useless as $n kicks the inside of your knee.",
          ch, NULL, victim, TO_VICT);
        victim->move -= victim->move * .4;
      }
      act (buf2, ch, NULL, victim, TO_CHAR);
    }

    else if (dt == gsn_dirty_tactics)
    {
      if (number_percent () < 72)	//Headbutt
      {
        act
          ("You headbutt $N in the face, leaving $M uglier than before.",
          ch, NULL, victim, TO_CHAR);
        act ("$n grabs $N's hair and violently headbutts $S face!", ch,
          NULL, victim, TO_NOTVICT);
        act
          ("$n quickly steps inside your defenses and smashes your face with $s forehead!",
          ch, NULL, victim, TO_VICT);
      }

      else			//Stomp 
      {
        act
          ("You stomp $N on the foot, the cracking sound putting a smile on your face.",
          ch, NULL, victim, TO_CHAR);
        act ("$n feints a swing to $N's face and stomps on $S foot.",
          ch, NULL, victim, TO_NOTVICT);
        act
          ("A stabbing pain shoots up your leg as $n crushes your foot!.",
          ch, NULL, victim, TO_VICT);
        victim->move -= victim->move * .5;
      }
    }

    else
    {
      act (buf1, ch, NULL, victim, TO_NOTVICT);
      act (buf2, ch, NULL, victim, TO_CHAR);
      act (buf3, ch, NULL, victim, TO_VICT);
    }

    //act (buf2, ch, NULL, victim, TO_CHAR);
  }
  return;
}

//Iblis - one of the dirty tactics commands, done automatically based on a random chance
//during battle
void do_eyegouge (CHAR_DATA * ch, CHAR_DATA * victim)
{

  AFFECT_DATA af;
  if (IS_AFFECTED (victim, AFF_BLIND))
  {
    act
      ("You attempt to poke $N in the eyes, but $E's already been blinded.",
      ch, NULL, victim, TO_CHAR);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("Very funny.\n\r", ch);
    return;
  }
  /* level */
  if (is_affected (victim, gsn_mirror))
    victim = ch;

  /* now the attack */
  act
    ("$n's hand turns into a blur as $e uses an opening in $N's defense to gouge $S eyes.",
    ch, NULL, victim, TO_NOTVICT);
  if (victim != ch)
    act ("The last thing you see is two fingers coming at your eyes.",
    ch, NULL, victim, TO_VICT);

  else
    act ("The mirror causes you to poke yourself in the eyes!", ch,
    NULL, victim, TO_VICT);
  send_to_char ("You can't see a thing!\n\r", victim);
  act ("You gouge $N's eyes out! Well, at least you temporarily blinded $M.",
    ch, NULL, victim, TO_CHAR);
  af.where = TO_AFFECTS;
  af.type = gsn_dirty_tactics;
  af.level = ch->level;
  af.duration = 1;
  af.location = APPLY_HITROLL;
  af.modifier = -4;
  af.permaff = FALSE;
  af.bitvector = AFF_BLIND;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (victim, &af);
}

//Iblis - one of the dirty tactics commands, done automatically based on a random chance
//during battle
void do_dirtytrip (CHAR_DATA * ch, CHAR_DATA * victim)
{

  CHAR_DATA *oldvict = NULL;
  int chance;

  /* BEGIN UNDERWATER */
  if (ch->in_room->sector_type == SECT_UNDERWATER)
  {
    send_to_char ("You can't trip someone underwater!\n\r", ch);
    return;
  }

  /* END UNDERWATER */
  if (ch->mount != NULL)
  {
    send_to_char ("You cannot trip while mounted.\n\r", ch);
    return;
  }
  if (victim->mount != NULL)
  {
    send_to_char ("You cannot trip someone who is mounted.\n\r", ch);
    return;
  }
  if (IS_AFFECTED (victim, AFF_FLYING))
  {
    act ("$S feet aren't on the ground.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (victim->position < POS_FIGHTING)
  {
    act ("$N is already down.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("You can't trip yourself!\n\r", ch);
    return;
  }

  /* modifiers */
  chance = get_skill (ch, gsn_trip);

  /* size */
  if (ch->size < victim->size)
    chance += (ch->size - victim->size) * 10;	/* bigger = harder to trip */

  /* dex */
  chance += get_curr_stat (ch, STAT_DEX);
  chance -= get_curr_stat (victim, STAT_DEX) * 3 / 2;

  /* speed */
  if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
    chance += 10;
  if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
    chance -= 20;

  /* level */
  chance += (ch->level - victim->level) * 2;
  if (is_affected (victim, gsn_mirror))
  {
    oldvict = victim;
    victim = ch;
  }

  /* now the attack */
  if (number_percent () < chance)
  {
    if (victim != ch)
    {
      act ("`B$n trips you with a low sweep.``", ch, NULL, victim,
        TO_VICT);
      act
        ("`AYou trip the clumsy fool in front of you with a beautiful low sweep.``",
        ch, NULL, victim, TO_CHAR);
      act ("$n trips $N with a low sweep of $s leg.", ch, NULL, victim,
        TO_NOTVICT);
    }

    else
    {
      act
        ("`BYour mirror deflects $n's trip, sending $m to the ground!``",
        ch, NULL, oldvict, TO_VICT);
      act ("`A$N's mirror deflects your trip, causing you to fall!``", ch,
        NULL, oldvict, TO_CHAR);
      act
        ("$n's is sent to the ground as $s trip is deflected off of $N's mirror.",
        ch, NULL, oldvict, TO_NOTVICT);
    }
    victim->position = POS_RESTING;
    victim->bashed = TRUE;
  }

  else
  {				//MISS
    act
      ("$n attempts to trip you, but you jump out of the way just in time.",
      ch, NULL, victim, TO_VICT);
    act ("`B$N jumps out of the way just in time, avoiding your trip.``",
      ch, NULL, victim, TO_CHAR);
    act ("$N jumps out of the way just in time to avoid $n's trip", ch,
      NULL, victim, TO_NOTVICT);
  }
}


/*
* Iblis - Check for Block.
*/
bool check_block (CHAR_DATA * ch, CHAR_DATA * victim)
{
  int chance;
  if (!IS_AWAKE (victim))
    return FALSE;
  chance = get_skill (victim, gsn_block) / 2;
  if (number_percent () >= chance)
    return FALSE;

  if (get_eq_char (victim, WEAR_WIELD_R) != NULL
    || get_eq_char (victim, WEAR_WIELD_L) != NULL)
  {
    return FALSE;
  }
  if (!can_see (ch, victim))
  {
    if (number_percent () < get_skill (ch, gsn_blindfighting))
      check_improve (ch, gsn_blindfighting, TRUE, 2);

    else
    {
      check_improve (ch, gsn_blindfighting, FALSE, 2);
      chance /= 2;
    }
  }
  chance = number_percent ();
  if (/*!IS_NPC (victim)
      &&*/ number_percent () < (get_skill (victim, gsn_counter) * .45))
  {
    counter_attack (victim, ch);
  }

  else
  {
    if (chance < 16)
      act ("$n slaps your weapon aside with the palm of $s hand.",
      victim, NULL, ch, TO_VICT);

    else if (chance < 33)
      act ("You are left dumbfounded as $n blocks your attack with a foot.",
      victim, NULL, ch, TO_VICT);

    else if (chance < 50)
      act
      ("As you charge at $n, $e stops your momentum with a kick in the chest.",
      victim, NULL, ch, TO_VICT);

    else if (chance < 66)
      act
      ("$n catches your weapon between the palms of $s hands, stopping your attack effectively.",
      victim, NULL, ch, TO_VICT);

    else if (chance < 83)
      act ("$n nudges your thrusting weapon to the side with a finger.",
      victim, NULL, ch, TO_VICT);

    else
      act
      ("Your weapon swings in a wide and harmless arc as $n strikes at your arm.",
      victim, NULL, ch, TO_VICT);
    act ("You slap $N's weapon aside with the palm of your hand.", victim,
      NULL, ch, TO_CHAR);
  }
  check_improve (victim, gsn_block, TRUE, 6);
  return TRUE;
}

void counter_attack (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (number_percent () < 50)
  {
    act
      ("$n easily slaps $N's weapon aside and whips $s hand out in a lightning-quick counter.",
      ch, NULL, victim, TO_NOTVICT);
    act
      ("Catching your weapon in $s hand $n throws a punch at your solar plexus, leaving you breathless.",
      ch, NULL, victim, TO_VICT);
  }

  else
  {
    act
      ("Dodging $N's attack, $n grins and strikes at the back of $S head.",
      ch, NULL, victim, TO_NOTVICT);
    act ("$n darts past your attack and places an elbow in your back!", ch,
      NULL, victim, TO_VICT);
  }
  one_hit (ch, victim, gsn_counter);
  check_improve (victim, gsn_counter, TRUE, 5);
}

//Iblis - Monk knock skill
void do_knock (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int chance;
  CHAR_DATA *victim = NULL;
  one_argument (argument, arg);
  if (arg[0] == '\0' && (victim = ch->fighting) == NULL)
  {
    send_to_char ("Knock whom?\n\r", ch);
    return;
  }
  if (IS_NPC (ch))
    return;
  if (get_skill (ch, gsn_knock) < 1)
  {
    send_to_char ("You'd better leave that to the monks.\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char
      ("You are too peaceful to knock anyone right now.\n\r", ch);
    return;
  }
  if ((victim == NULL) && ((victim = get_char_room (ch, arg)) == NULL))
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (ch->fighting != NULL)
  {
    send_to_char ("You can't find a way to creep up behind them in the middle of a fight.\n\r",ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("You knock yourself silly.\n\r", ch);
    return;
  }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (is_safe (ch, victim))
    return;
  if (victim->position <= POS_SITTING)
    chance = get_skill (ch, gsn_knock) * .80;

  else if (victim->fighting != NULL)
    chance = 0;

  else
    chance = get_skill (ch, gsn_knock) * .50;
  if (ch->level < victim->level)
    chance -= ((victim->level - ch->level) * 3);
  if ((number_percent () < chance || (chance >= 2 && !IS_AWAKE (victim))) &&  !IS_SET(victim->imm_flags, IMM_SLEEP) && !IS_SET(victim->act,ACT_UNDEAD))
  {
    if (awareness (ch, victim))
      return;
    act ("You render $N unconscious with a knock on the head.", ch,
      NULL, victim, TO_CHAR);
    act ("You feel a knock to the back of your head and you pass out.",
      ch, NULL, victim, TO_VICT);
    act ("$n renders $N unconscious with a knock on the head.", ch,
      NULL, victim, TO_NOTVICT);
    check_improve (ch, gsn_knock, TRUE, 1);
    //WAIT_STATE (victim, 4 * PULSE_VIOLENCE);
    if (!IS_NPC(victim))
      victim->pcdata->knock_time = 12;
    WAIT_STATE (ch, skill_table[gsn_knock].beats);

    //victim->position = POS_SLEEPING;
    if (victim->fighting == ch)
      stop_fighting (victim, TRUE);
    victim->position = POS_SLEEPING;
    if (!is_affected (victim, gsn_mirror)
      && is_affected (victim, gsn_fireshield))
      if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
      {
        damage (victim, ch, 8, 0, DAM_FIRE, FALSE);
        act ("$N's scorching shield of flame burns you as you attack.",
          ch, NULL, victim, TO_CHAR);
        act ("$n is burnt by your scorching shield of flame.", ch, NULL,
          victim, TO_VICT);
      }
  }

  else if (!nodam_message)
  {
    act
      ("You move in behind $N, but are caught before you can land the blow!",
      ch, NULL, victim, TO_CHAR);
    WAIT_STATE (ch, skill_table[gsn_knock].beats);
    check_improve (ch, gsn_knock, FALSE, 1);
    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
      return;
    }
    damage (ch, victim, 0, gsn_knock, DAM_NONE, TRUE);
  }
  return;
}

//Iblis - Monk feign death skill
void do_feign (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;
  short vi;
  one_argument (argument, arg);
  if (IS_NPC (ch))
    return;
  if (IS_CLASS(ch,PC_CLASS_NECROMANCER))
  {
    feign_death(ch,argument);
    return;
  }
  if (get_skill (ch, gsn_feign) < 1)
  {
    send_to_char("You attempt to feign a smile. :/\n\r",ch);
    return;
  }

  if (arg[0] == '\0')
  {
    victim = ch->fighting;
    if (victim == NULL)
    {
      send_to_char ("But you aren't fighting anyone!\n\r", ch);
      return;
    }
  }

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (is_safe (ch, victim))
    return;
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
  {
    act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
    return;
  }
  vi = get_curr_stat (victim, STAT_INT);
  if (vi < 7)
    chance = .9 * get_skill (victim, gsn_feign);

  else if (vi < 12)
    chance = .5 * get_skill (victim, gsn_feign);

  else if (vi < 15)
    chance = .3 * get_skill (victim, gsn_feign);

  else if (vi < 18)
    chance = .2 * get_skill (victim, gsn_feign);

  else if (vi < 25)
    chance = .1 * get_skill (victim, gsn_feign);

  else
    chance = 0;
  act ("You point and gasp `a'`kLook behind you! A three-headed monkey!`a'``",
    ch, NULL, victim, TO_CHAR);
  act
    ("$n gasps and points at something behind you. `a'`kLook behind you! A three-headed monkey!`a'``",
    ch, NULL, victim, TO_ROOM);
  if (number_percent () < chance && victim->race != PC_RACE_SWARM)
  {
    act ("$N turns around ``'`kReally?!`a'``", ch, NULL, victim, TO_CHAR);
    act ("$N turns around ``'`kReally?!`a'``", ch, NULL, victim,
      TO_NOTVICT);
    act ("You turn around to gaze at the marvel. Big mistake, stupid.",
      ch, NULL, victim, TO_VICT);
    ch->pcdata->feigned = TRUE;

    //cheap way to initiate battle
    damage (ch, victim, 0, gsn_feign, DAM_BASH, FALSE);
    check_improve (ch, gsn_feign, TRUE, 5);
    WAIT_STATE (ch, skill_table[gsn_feign].beats);
  }

  else
  {
    if (ch->race == PC_RACE_CANTHI && !IS_IMMORTAL (ch))
    {
      act ("$N gurgles `a'`kHah! I won't fall for that! (again)`a'``",
        ch, NULL, victim, TO_ROOM);
      act ("You gurgles `a'`kHah! I won't fall for that! (again)`a'``",
        ch, NULL, victim, TO_VICT);
      act ("$N gurgles `a'`kHah! I won't fall for that! (again)`a'``", ch,
        NULL, victim, TO_NOTVICT);
      return;
    }
    if (ch->race == PC_RACE_VROATH && !IS_IMMORTAL (ch))
    {
      act ("$N grunts `a'`kHah! I won't fall for that! (again)`a'``",
        ch, NULL, victim, TO_ROOM);
      act ("You grunts `a'`kHah! I won't fall for that! (again)`a'``",
        ch, NULL, victim, TO_VICT);
      act ("$N grunts `a'`kHah! I won't fall for that! (again)`a'``",
        ch, NULL, victim, TO_NOTVICT);
      return;
    }
    if (ch->race == PC_RACE_SIDHE && !IS_IMMORTAL (ch))
    {
      act
        ("$N telepaths to you `a'`kHah! I won't fall for that! (again)`a'``",
        ch, NULL, victim, TO_CHAR);
      act
        ("You telepath to $n `a'`kHah! I won't fall for that! (again)`a'``",
        ch, NULL, victim, TO_VICT);
      act
        ("$N telepaths to all `a'`kHah! I won't fall for that! (again)`a'``",
        ch, NULL, victim, TO_NOTVICT);
      return;
    }
    if ((ch->race != PC_RACE_CANTHI && ch->race != PC_RACE_VROATH && ch->race != PC_RACE_SIDHE)
      || IS_IMMORTAL (ch))
    {
      act ("$N says `a'`kHah! I won't fall for that! (again)`a'``",
        ch, NULL, victim, TO_CHAR);
      act ("You says `a'`kHah! I won't fall for that! (again)`a'``",
        ch, NULL, victim, TO_VICT);
      act ("$N says `a'`kHah! I won't fall for that! (again)`a'``",
        ch, NULL, victim, TO_NOTVICT);
    }
    check_improve (ch, gsn_feign, FALSE, 5);
    WAIT_STATE (ch, skill_table[gsn_feign].beats);
  }
}

//Iblis - Monk prevent escape skill, called from flee/retreat
void prevent_escape (CHAR_DATA * ch, CHAR_DATA * victim)
{
  act ("`hYou intercept $N as $E tries to escape!``", ch, NULL, victim,
    TO_CHAR);
  act ("`h$n is in the way! Panic!``", ch, NULL, victim, TO_VICT);
  act ("`h$n skillfully prevents $N's attempt to flee.``", ch, NULL,
    victim, TO_NOTVICT);
  damage (victim, ch, get_curr_stat (victim, STAT_STR),
    gsn_prevent_escape, DAM_BASH, FALSE);
}

//Iblis - Monk dive skill
void do_dive (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim, *vch, *fch, *fch_next, *blocker;
  int chance, x;	
  char arg[MAX_INPUT_LENGTH];
  char tcbuf[MAX_INPUT_LENGTH];

  short door = 0;
  ROOM_INDEX_DATA *in_room = NULL;
  ROOM_INDEX_DATA *to_room = NULL;

  OBJ_DATA *tObj;
  EXIT_DATA *pexit;
  one_argument (argument, arg);
  if ((chance = get_skill (ch, gsn_dive)) <= 0 || IS_SET(ch->act2,ACT_NOMOVE))
  {
    send_to_char ("Diving? What's that?\n\r", ch);
    return;
  }
  if (arg[0] == '\0')
  {
    send_to_char ("Dive at whom?\r\n", ch);
    return;
  }
  if ((victim = get_char_room (ch, arg)) != NULL)
  {
    send_to_char ("You cannot dive at someone in the same room.\n\r", ch);
    return;
  }

  else
  {
    for (x = 0; x < MAX_DIR; x++)
      if (ch->in_room->exit[x] != NULL
        && ch->in_room->exit[x]->u1.to_room != NULL
        && !IS_SET (ch->in_room->exit[x]->exit_info, EX_CLOSED))
      {
        for (vch = ch->in_room->exit[x]->u1.to_room->people; vch;
          vch = vch->next_in_room)
          if (is_name (argument, vch->name) && can_see_hack (ch, vch))
          {
            victim = vch;
            door = x;
            break;
          }
          if (victim != NULL)
            break;
      }
  }
  if (victim == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (ch->fighting != NULL)
  {
    send_to_char ("You can't do that while fighting.\r\n", ch);
    return;
  }
  if (IS_SET (ch->in_room->room_flags, ROOM_SAFE))
  {
    send_to_char ("You cannot dive from a safe room.\n\r", ch);
    return;
  }
  if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
  {
    send_to_char ("You cannot dive into a safe room.\n\r", ch);
    return;
  }

  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  } 

  if (IS_AFFECTED (ch, AFF_CALM)
    || is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char ("You are feeling too peaceful to do that now.\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_clasp))
  {
    send_to_char ("Your feet are to restrained to do that.\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_entangle))
  {
    send_to_char ("You are too constrained to initiate combat.\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CHARM)
    && ch->master != NULL && in_room == ch->master->in_room)
  {
    send_to_char ("What?  And leave your beloved master?\n\r", ch);
    return;
  }
  if (is_affected (ch, skill_lookup ("web")))
  {
    send_to_char
      ("The webs binding you make movement difficult and diving impossible.\n\r",
      ch);
    return;
  }
  if (ch->mount != NULL)
  {
    send_to_char ("You cannot dive while mounted.\n\r", ch);
    return;
  }
  if (victim->mount != NULL)
  {
    send_to_char ("You cannot dive at someone who is mounted.\n\r", ch);
    return;
  }
  if (is_safe (ch, victim))
    return;
  if (check_shopkeeper_attack (ch, victim))
    return;

  for (blocker = ch->in_room->people; blocker != NULL;
    blocker = blocker->next_in_room)
  {
    if (!IS_SET (blocker->act2, ACT_BLOCK_EXIT)
      || blocker->blocks_exit == -1)
      continue;
    if (door != blocker->blocks_exit)
      continue;
    if (IS_IMMORTAL (ch))
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

  if (!IS_NPC(ch) && ch->pcdata && ch->pcdata->fishing)
  {
    send_to_char ("You can't move while you're fishing!.\n\r", ch);
    return;
  }


  if (!can_move_char_door (ch, door, FALSE, FALSE))
  {
    send_to_char ("You cannot dive in the direction of that char.\r\n", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char
      ("You attempt to dive at yourself, but you see yourself coming and step aside.\n\r",
      ch);
    return;
  }
  act ("You dive at $N.", ch, NULL, victim, TO_CHAR);

  act ("$n runs and leaps away to the $T.", ch, NULL, dir_name[door],
    TO_ROOM);

  in_room = ch->in_room;
  pexit = in_room->exit[door];
  to_room = pexit->u1.to_room;
  if (pexit == NULL || to_room == NULL)
    send_to_char ("uh oh\r\n", ch);

  /* Move the Character */
  char_from_room (ch);
  char_to_room (ch, to_room);
  do_look (ch, "auto");

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
  for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
  {

    fch_next = fch->next_in_room;

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
      && can_see (ch, fch) && get_position(ch) > POS_SLEEPING)
    {
      if (ch->position < POS_STANDING)
        do_stand (ch, "");
      do_kill (ch, NAME (fch));
    }
  }

  //Minax 7-10-02 Check for a tripwire in the room.
  for (tObj = ch->in_room->contents; tObj; tObj = tObj->next_content)
    if (tObj->pIndexData->vnum == OBJ_VNUM_TRIPWIRE
      && ch->level < 92 && !IS_NPC (ch))
    {
      if ((number_percent () / 2) > get_curr_stat (ch, STAT_INT)
        && ch->position > POS_RESTING)
      {
        sprintf (tcbuf,
          "You trip over the tripwire and fall flat on your face, missing your dive completely!\n\r");
        send_to_char (tcbuf, ch);
        sprintf (tcbuf,
          "$n trips over a tripwire and falls flat on $s face!");
        act (tcbuf, ch, NULL, NULL, TO_ROOM);
        ch->position = POS_RESTING;
        WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
        if (number_percent () < (ch->level / 3))
        {
          sprintf (tcbuf,
            "The tripwire is ripped out of the ground and destroyed.\n\r");
          send_to_char (tcbuf, ch);
          sprintf (tcbuf,
            "The tripwire is ripped out of the ground and destroyed.");
          act (tcbuf, ch, NULL, NULL, TO_ROOM);
          extract_obj (tObj);
        }
        return;
      }
    }
    for (tObj = ch->in_room->contents; tObj; tObj = tObj->next_content)
      if (tObj->pIndexData->vnum == OBJ_VNUM_WALL_THORNS
        && tObj->value[0] == rev_dir[door])
      {
        wall_damage (ch, tObj);
        break;
      }
      if (ch->tracking[0] != '\0')
        display_track (ch);

      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(ch,"room",ch->in_room,NULL);
      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      if (check_aggression(ch))
        return;
      if (number_percent () < chance)
      {
        act
          ("$N flies feet first into the room, planting a kick squarely on $n, sending $m to the ground.",
          victim, NULL, ch, TO_NOTVICT);
        act
          ("$N comes flying into the room and lands with both feet in your chest!",
          victim, NULL, ch, TO_CHAR);
        act
          ("You roll and launch yourself into the air, planting both your feet in the chest of $N.",
          ch, NULL, victim, TO_CHAR);

        damage (ch, victim, adjust_damage(.5 * ch->level), gsn_dive, DAM_BASH, FALSE);
        victim->position = POS_RESTING;
        check_improve (ch, gsn_dive, TRUE, 2);
        WAIT_STATE (ch, skill_table[gsn_dive].beats);
        WAIT_STATE (victim, 12);
      }

      else
      {
        act
          ("$N flies feet first into the room, missing $n and landing hard on the ground.",
          victim, NULL, ch, TO_NOTVICT);
        act
          ("$N flies feet first into the room, missing you and landing hard on the ground.",
          victim, NULL, ch, TO_CHAR);
        act
          ("You roll and launch yourself into the air, narrowly missing $N, and land hard on the ground.",
          ch, NULL, victim, TO_CHAR);
        damage (ch, victim, 0, gsn_dive, DAM_BASH, FALSE);
        ch->position = POS_RESTING;
        check_improve (ch, gsn_dive, FALSE, 2);
        WAIT_STATE (ch, skill_table[gsn_dive].beats);
      }
}

//Iblis - Monk toss skill
void do_toss (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char tcbuf[15];
  CHAR_DATA *victim = NULL;
  int chance;
  short dir;
  EXIT_DATA *pexit;
  ROOM_INDEX_DATA *to_room = NULL;
  argument = one_argument (argument, arg);
  one_argument (argument, arg2);

  /* BEGIN UNDERWATER */
  if (ch->in_room->sector_type == SECT_UNDERWATER)
  {
    send_to_char ("You can't toss someone underwater!\n\r", ch);
    return;
  }

  /* END UNDERWATER */
  if ((chance = get_skill (ch, gsn_toss)) <= 0 || (IS_NPC (ch)))
  {
    send_to_char ("Tossing?  What's that?\n\r", ch);
    return;
  }
  if (arg[0] == '\0')
  {
    victim = ch->fighting;
    if (victim == NULL)
    {
      send_to_char ("But you aren't fighting anyone!\n\r", ch);
      return;
    }
  }

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (ch->mount != NULL)
  {
    send_to_char ("You cannot toss someone while mounted.\n\r", ch);
    return;
  }
  if (victim->mount != NULL)
  {
    send_to_char ("You cannot toss someone who is mounted.\n\r", ch);
    return;
  }
  if (is_safe (ch, victim))
    return;
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (IS_SET (ch->in_room->room_flags, ROOM_NO_PUSH))
  {
    send_to_char ("This room is safe from bullies like you.\n\r", ch);
    return;
  }

  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char ("You are too peaceful to sap anyone right now.\n\r", ch);
    return;
  }

  if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (IS_SET(victim->act2,ACT_NOMOVE))
  {
    send_to_char ("That mob's power blocks your tossing efforts.\n\r",ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char
      ("You attempt to toss yourself, but you cannot defy the laws of gravity in that fashion.\n\r",
      ch);
    return;
  }
  if (ch->fighting != NULL
    && (ch->fighting != victim || victim->fighting != ch))
  {
    act ("You can't toss $N while fighting someone else.", ch, NULL,
      victim, TO_CHAR);
  }
  if (!str_cmp (arg2, "n") || !str_cmp (arg2, "north"))
    dir = 0;

  else if (!str_cmp (arg2, "e") || !str_cmp (arg2, "east"))
    dir = 1;

  else if (!str_cmp (arg2, "s") || !str_cmp (arg2, "south"))
    dir = 2;

  else if (!str_cmp (arg2, "w") || !str_cmp (arg2, "west"))
    dir = 3;

  else if (!str_cmp (arg2, "u") || !str_cmp (arg2, "up"))
    dir = 4;

  else if (!str_cmp (arg2, "d") || !str_cmp (arg2, "down"))
    dir = 5;

  else
  {
    send_to_char ("You must specify a direction to toss in.\n\r", ch);
    return;
  }
  if (!IS_NPC (victim)
    && get_curr_stat (ch,
    STAT_STR) <
    ((get_curr_stat (victim, STAT_STR) +
    get_curr_stat (victim, STAT_CON)) / 2))
  {
    act ("You're not strong enough to toss $N anywhere.", ch, NULL,
      victim, TO_CHAR);
    return;
  }
  if (can_move_char_door (victim, dir, FALSE, FALSE))
  {

    if (!IS_NPC(ch) && IS_NPC(victim) && (dice(3,5) < ch->pcdata->pushed_mobs_counter))
    {
      act ("$N growls at you and attacks.", ch, NULL, victim, TO_CHAR);
      act ("$N growls at $n and attacks.", ch, NULL, victim, TO_ROOM);
      act ("You growl at $n and attack.", ch, NULL, victim, TO_VICT);
      do_kill (victim, ch->name);
      WAIT_STATE(ch,4);
      return;
    }


    if (number_percent () <
      ((ch->fighting) ? (.5 * get_skill (ch, gsn_toss)) : (.3 *
      get_skill
      (ch,
      gsn_toss))))
    {
      if (awareness (ch, victim))
        return;

      pexit = ch->in_room->exit[dir];
      to_room = pexit->u1.to_room;
      if (pexit == NULL || to_room == NULL)
        send_to_char ("uh oh\r\n", ch);
      char_from_room (victim);
      char_to_room (victim, to_room);
      trip_triggers(victim, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(victim,"room",victim->in_room,NULL);
      sprintf(tcbuf,"%d",victim->in_room->vnum);
      trip_triggers_arg(victim, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      check_aggression(victim);
      act
        ("You place your foot in $N's chest and roll backwards, launching $M into the next room.",
        ch, NULL, victim, TO_CHAR);
      act
        ("$n grabs you by the neck and with a twist of $S hip $E and throws you into the next room!",
        ch, NULL, victim, TO_VICT);
      act
        ("$n places a foot in $N's chest and roll backwards, launching $M into the next room.",
        ch, NULL, victim, TO_ROOM);
      act ("$n comes flying into the room and lands in a heap.", victim,
        NULL, NULL, TO_ROOM);
      damage (ch, victim, .25 * ch->carry_weight, gsn_toss, DAM_BASH,
        FALSE);
      victim->position = POS_RESTING;
      if (!IS_NPC(ch) && IS_NPC(victim) && ch->pcdata->pushed_mobs_counter < 10000)
      {
        ++ch->pcdata->pushed_mobs_counter;
      }
    }

    else
    {
      act ("You fail to get a good grip on $N.", ch, NULL, victim,
        TO_CHAR);
    }
    WAIT_STATE (ch, skill_table[gsn_toss].beats);
  }

  else
  {
    act ("You can not toss $N in that direction.", ch, NULL, victim,
      TO_CHAR);
    return;
  }
}

//Iblis - Monk strike skill
void do_strike (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *weapon_l, *weapon_r;
  one_argument (argument, arg);
  if (get_skill (ch, gsn_strike) < 1)
  {
    send_to_char ("You don't know how to strike!\n\r", ch);
    return;
  }
  if (arg[0] == '\0')
  {
    if (ch->fighting == NULL)
    {
      send_to_char ("Strike whom?\n\r", ch);
      return;
    }
    victim = ch->fighting;
  }

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("How can you strike yourself?\n\r", ch);
    return;
  }
  if (is_safe (ch, victim))
  {
    send_to_char ("You can't do that here.\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
  {
    act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
    return;
  }
  weapon_l = get_eq_char (ch, WEAR_WIELD_L);
  weapon_r = get_eq_char (ch, WEAR_WIELD_R);
  if (weapon_l != NULL || weapon_r != NULL)
  {
    send_to_char ("You can't strike while wielding a weapon.\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CALM)
    || is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char
      ("You are way too sedate to consider attacking anyone.\n\r", ch);
    return;
  }
  WAIT_STATE (ch, skill_table[gsn_strike].beats);
  if (number_percent () < get_skill (ch, gsn_strike)
    || (get_skill (ch, gsn_strike) >= 2 && !IS_AWAKE (victim)))
  {
    if (awareness (ch, victim))
      return;
    check_improve (ch, gsn_strike, TRUE, 1);

    switch (ch->dam_type)
    {
    case 3:
      act ("$n's Wing of the Crane slash strikes against your neck!",
        ch, NULL, victim, TO_VICT);
      act ("Your Wing of the Crane slash strikes against $N neck!",
        ch, NULL, victim, TO_CHAR);
      act ("$n's Wing of the Crane slash strikes against $N neck!",
        ch, NULL, victim, TO_NOTVICT);
      break;
    case 5:
      act ("$n's Tiger Claw strike tears into your chest!", ch, NULL,
        victim, TO_VICT);
      act ("Your Tiger Claw strike tears into $N's chest!", ch, NULL,
        victim, TO_CHAR);
      act ("$n's Tiger Claw strike tears into $N's chest!", ch, NULL,
        victim, TO_NOTVICT);
      break;
    case 11:
      act
        ("$n's Snake's Head piercing attack slices into the soft parts of your neck!",
        ch, NULL, victim, TO_VICT);
      act
        ("Your Snake's Head piercing attack slices into the soft parts of $N's neck!",
        ch, NULL, victim, TO_CHAR);
      act
        ("$n's Snake's Head piercing attack slices into the soft parts of $N's neck!",
        ch, NULL, victim, TO_NOTVICT);
      break;
    case 17:
      act ("$n's Panther Fists slam down on your collar bones!", ch,
        NULL, victim, TO_VICT);
      act ("Your Panther Fists slam down on $N's collar bones!", ch,
        NULL, victim, TO_CHAR);
      act ("$n's Panther Fists slam down on $N's collar bones!", ch,
        NULL, victim, TO_NOTVICT);
      break;
    case 18:
      act
        ("$n's Dragon's Breath attack almost tears your arms out of their sockets!",
        ch, NULL, victim, TO_VICT);
      act
        ("Your Dragon's Breath attack almost tears $N's arms out of their sockets!",
        ch, NULL, victim, TO_CHAR);
      act
        ("$n's Dragon's Breath attack almost tears $N's arms out of their sockets!",
        ch, NULL, victim, TO_NOTVICT);
      break;
    }
    one_hit (ch, victim, gsn_strike);

  }

  else
  {

    check_improve (ch, gsn_strike, FALSE, 1);
    damage (ch, victim, 0, gsn_strike, ch->dam_type, TRUE);
  }
  return;
}

//Iblis - Monk whirlwind skill
void do_whirlwind (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim = NULL,
    *next_victim;
  int j, chance;
  bool hits = FALSE;

  if ( (chance = get_skill(ch, gsn_whirlwind)) == 0)
  {
    send_to_char("Whirlwind? What's that?\n\r", ch);
    return;
  }
  if (IS_SET (ch->in_room->room_flags, ROOM_SAFE))
  {
    send_to_char("This room is safe from bullies like you.\n\r", ch);
    return;
  }
  if (is_affected(ch, gsn_fear))
  {
    send_to_char("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CALM)
    || is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char("You are feeling too peaceful to do that now.\n\r", ch);
    return;
  }
  if (is_affected(ch, gsn_entangle))
  {
    send_to_char("You are too constrained to initiate combat.\n\r", ch);
    return;
  }
  if (is_affected(ch, skill_lookup ("web")))
  {
    send_to_char("The webs binding you make movement difficult and charging impossible.\n\r", ch);
    return;
  }
  act
    ("`b$n whirls around the room striking down everything in $s path, leaving a trail of terrible devastation.``",
    ch, NULL, victim, TO_ROOM);
  act ("`hYou execute the feared whirlwind attack!``", ch, NULL, victim,
    TO_CHAR);
  for (victim = ch->in_room->people; victim != NULL; victim = next_victim)
  {
    next_victim = victim->next_in_room;
    if (victim == ch)
      continue;
    if (victim->invis_level >= LEVEL_HERO)
      continue;
    if (is_same_group(ch, victim))
      continue;
    if (is_safe(ch, victim))
      continue;
    if (check_shopkeeper_attack(ch, victim))
      continue;
    if (victim->mount != NULL && ch->mount == NULL)
      continue;

    chance = (get_skill (ch, gsn_whirlwind));

    for (j = 0; j < 2; j++)
    {

      /* now the attack */
      if (number_percent () < chance)
      {
        check_improve(ch, gsn_whirlwind, TRUE, 1);
        hits = TRUE;
        act("one_hit", ch, NULL, victim, TO_ROOM);
        one_hit(ch, victim, gsn_whirlwind);
        if (IS_NPC(victim))
          victim->color = 42;
      }
    }
  }
  if (!hits)
    send_to_char("But alas, you missed everyone.\n\r", ch);

  WAIT_STATE (ch, skill_table[gsn_whirlwind].beats);
}

//Iblis - Monk fists skill
void do_fists (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int dam;
  CHAR_DATA *victim;

  one_argument (argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char ("Use your fists of fury on whom?\n\r", ch);
    return;
  }
  if (get_skill (ch, gsn_fists_of_fury) < 1)
  {
    send_to_char
      ("So you want to use fists of fury without the skill?!?\n\r", ch);
    return;
  }
  if (ch->mana < 75)
  {
    send_to_char ("You don't have enough mana.\n\r", ch);
    return;
  }
  if (is_affected (ch, gsn_fear))
  {
    send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
    return;
  }
  if (is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
  {
    send_to_char
      ("You are too peaceful to use your fists on anything right now.\n\r",
      ch);
    return;
  }
  if (ch->fighting != NULL)
  {
    send_to_char
      ("Your fists of fury need time to warm up, try it when you're not already fighting.\n\r",
      ch);
    return;
  }

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
  {
    act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if (victim == ch)
  {
    send_to_char
      ("Try as you might, you can't force yourself to hit..yourself.\n\r",
      ch);
    return;
  }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (is_safe (ch, victim))
    return;

  ch->mana -= 75;
  if (number_percent () < get_skill (ch, gsn_fists_of_fury))
  {
    if (awareness (ch, victim))
      return;
    check_improve (ch, gsn_fists_of_fury, TRUE, 1);

    dam = number_range(200,300) + ch->perm_stat[STAT_DEX] + ch->perm_stat[STAT_STR];
    dam = adjust_damage(dam);
    act ("`iYou attack $N with an incredible fury!``", ch, NULL, victim,
      TO_CHAR);
    act
      ("`iThe very air seems to slash at you when $n attacks with a barrage of punches!``",
      ch, NULL, victim, TO_VICT);
    act
      ("`iAttacking $N with an incredible fury $n's hands slash like knives!``",
      ch, NULL, victim, TO_NOTVICT);
    damage (ch, victim, dam, gsn_fists_of_fury, 21, FALSE);   //WRATH ATTACK
    if (!is_affected (victim, gsn_mirror)
      && is_affected (victim, gsn_fireshield))
      if (!saves_spell (ch->level + 2, ch, DAM_FIRE))
      {
        damage (victim, ch, 8, 0, DAM_FIRE, FALSE);
        act ("$N's scorching shield of flame burns you as you attack.",
          ch, NULL, victim, TO_CHAR);
        act ("$n is burnt by your scorching shield of flame.", ch, NULL,
          victim, TO_VICT);
      }
      WAIT_STATE (ch, skill_table[gsn_fists_of_fury].beats);
      if (number_percent () < get_skill (ch, gsn_fists_of_fury) * .07)
      {
        stop_fighting(ch,FALSE);
        nodam_message = TRUE;
        do_knock (ch, arg);
        nodam_message = FALSE;
      }
  }

  else
  {
    check_improve (ch, gsn_fists_of_fury, FALSE, 1);
    damage (ch, victim, 0, gsn_fists_of_fury, DAM_NONE, FALSE);
    act ("You cannot summon the fury to execute the attack.", ch, NULL,
      victim, TO_CHAR);
    act ("$n attempts to drive $mself into a fury, but fails.", ch,
      NULL, victim, TO_ROOM);

  }
  return;
}

//Iblis - Monk bandage skill
void do_bandage (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int chance, heal_amount;
  CHAR_DATA *victim;
  OBJ_DATA *bandage;
  one_argument (argument, arg);
  if (get_skill (ch, gsn_bandage) < 1)
  {
    send_to_char ("You'd better leave that to the monks.\n\r", ch);
    return;
  }
  if (arg[0] == '\0')
    victim = ch;

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if ((bandage = get_obj_carry (ch, "bandage")) == NULL)
  {
    send_to_char
      ("Your attempt to will a bandage into existence, but alas you cannot.\n\r",
      ch);
    return;
  }
  if (victim->hit >= victim->max_hit)
  {
    act ("Your bandages are of no use to $N.", ch, NULL, victim, TO_CHAR);
    return;
  }
  chance = get_skill (ch, gsn_bandage);
  if (number_percent () < chance)
  {
    if (victim->race == PC_RACE_SWARM)
    {
      swarm_heal(ch,victim,gsn_bandage);
      return;
    }
    act ("You bandage $N's wounds.", ch, NULL, victim, TO_CHAR);
    act ("$n carefully bandages your wounds.", ch, NULL, victim, TO_VICT);
    act ("$n binds $N's wounds.", ch, NULL, victim, TO_NOTVICT);

    heal_amount =
      ((10 + get_curr_stat (ch, STAT_DEX)) / 100.0) * victim->max_hit;
    if (heal_amount > 200)
      heal_amount = 200;
    if (!IS_NPC(victim) && victim->pcdata->soul_link)
    {
      victim->hit = UMIN (victim->hit + heal_amount/2, victim->max_hit);
      victim->pcdata->soul_link->hit = UMIN (victim->pcdata->soul_link->hit + heal_amount/2,
        victim->pcdata->soul_link->max_hit);
      update_pos (victim->pcdata->soul_link);
      send_to_char ("A healing force courses through your body!\n\r", victim->pcdata->soul_link);
    }
    else 
    {
      victim->hit += heal_amount;
      if (victim->hit > victim->max_hit)
        victim->hit = victim->max_hit;
    }

    extract_obj (bandage);
    check_improve (ch, gsn_bandage, TRUE, 1);
    WAIT_STATE (ch, skill_table[gsn_bandage].beats);
  }

  else
  {
    act
      ("You attempt to bandage $N's wounds, but apparently your skills could use some improving.",
      ch, NULL, victim, TO_CHAR);
    act ("$n wastes a bandage trying to bandage your wounds.", ch, NULL,
      victim, TO_VICT);
    act ("$n attempts to bind $N's wounds, with no success.", ch, NULL,
      victim, TO_NOTVICT);
    WAIT_STATE (ch, skill_table[gsn_bandage].beats);
    extract_obj (bandage);
    check_improve (ch, gsn_bandage, FALSE, 1);
  }
  return;
}

//Iblis - Monk awareness skill.  Function called in act_info.c in do_look
bool awareness (CHAR_DATA * victim, CHAR_DATA * ch)
{
  if (ch->fighting != NULL)
    return FALSE;
  if (number_percent () < (get_skill (ch, gsn_awareness) * .10))
  {
    act
      ("You see $N coming from a mile away and easily sidestep $S attack.",
      ch, NULL, victim, TO_CHAR);
    act
      ("The instant before your attack hits, $n steps aside with a yawn, making you look like a clumsy fool.",
      ch, NULL, victim, TO_VICT);
    act
      ("$n sidesteps $N's attack with an obvious lack of interest or difficulty.",
      ch, NULL, victim, TO_NOTVICT);
    check_improve (ch, gsn_awareness, TRUE, 1);
    return TRUE;
  }

  else
    return FALSE;
}

//Iblis - Monk hear the wind skill.  Function called in do_throw and do_assassinate
bool hear_the_wind (CHAR_DATA * victim, CHAR_DATA * ch, OBJ_DATA * obj)
{
  char buf[MAX_STRING_LENGTH];
  if (number_percent () < (get_skill (ch, gsn_hear_the_wind) * .10))
  {
    act
      ("You look up just in time to see $p flying at your head. With a grunt you catch it and throw it back.",
      ch, obj, victim, TO_CHAR);

    act
      ("$n looks up quickly, catches $p and throws it back to where it came from, proving it's all in the reflexes.",
      ch, obj, victim, TO_ROOM);
    one_argument (obj->name, buf);
    if (is_affected (victim, gsn_mask) && victim->mask != NULL)
      sprintf (buf, "%s %s", buf, victim->mask);	//nameptr = victim->mask;
    else
      sprintf (buf, "%s %s", buf, victim->name);	//MASK CHECK
    unequip_char (victim, obj);
    obj_from_char (obj);
    obj_to_char (obj, ch);
    do_throw (ch, buf);
    check_improve (ch, gsn_hear_the_wind, TRUE, 5);
    return TRUE;
  }

  else
    return FALSE;
}

//Iblis - Insta-kill function, called in do_assassinate and magic.c's spell_bee_swarm
bool
insta_kill (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
            int dam_type, bool show, bool magical_attack)
{
  bool mirror_attack = FALSE;
  OBJ_DATA *corpse;

  //bool immune;
  if (victim->position == POS_DEAD)
    return FALSE;
  if (victim != ch)
  {

    /*
    * Certain attacks are forbidden.
    * Most other attacks are returned.
    */
    if (is_safe (ch, victim))
      return FALSE;
    if (victim->position > POS_STUNNED)
    {
      if (victim->fighting == NULL)
      {
        if (dt == TYPE_AUTOASSIST || dam_type == DAM_NONE)
          set_fighting_auto (victim, ch);

        else
          set_fighting (victim, ch);
      }
    }
    if (victim->position > POS_STUNNED)
    {
      if (ch->fighting == NULL)
      {
        if (dt == TYPE_AUTOASSIST || dam_type == DAM_NONE)
        {
          /* Iblis 08/28/03 - To properly set aggressor on someone
          * already fighting */
          update_aggressor(ch,victim);
          set_fighting_auto (ch, victim);
        }

        else
        {
          update_aggressor(ch,victim);
          set_fighting (ch, victim);
        }
      }
    }

    /*
    * More charm stuff.
    */
    if (victim->master == ch)
      stop_follower (victim);
  }

  /*
  * Inviso attacks ... not.
  */
  if (IS_AFFECTED (ch, AFF_INVISIBLE))
  {
    affect_strip (ch, gsn_astral_body);  
    affect_strip (ch, gsn_invis);
    affect_strip (ch, gsn_mass_invis);
    affect_strip (ch, gsn_heavenly_cloak);
    affect_strip (ch, gsn_vanish);
    REMOVE_BIT (ch->affected_by, AFF_INVISIBLE);
    act ("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
  }
  //5-10-03 Iblis - Setting it so if you are attacked, you become un-hidden and un-camoed
  if (IS_AFFECTED (ch, AFF_CAMOUFLAGE))
    REMOVE_BIT (ch->affected_by, AFF_CAMOUFLAGE);
  if (IS_SET(ch->act2,ACT_TRANSFORMER))
    affect_strip(ch,gsn_transform);
  if (IS_AFFECTED (ch, AFF_HIDE))
    REMOVE_BIT (ch->affected_by, AFF_HIDE);
  if (is_affected (victim, gsn_mirror) && dam_type != DAM_ACID
    && dam_type != DAM_ENERGY && dam_type != DAM_HOLY
    && dam_type != DAM_LIGHTNING && dam_type != DAM_FIRE
    && dam_type != DAM_COLD && dam_type != DAM_NEGATIVE
    && dam_type != DAM_MENTAL && dam_type != DAM_LIGHT
    && dam_type != DAM_CHARM && dam_type != DAM_DISEASE
    && dam_type != DAM_SOUND && dam_type != DAM_HARM && !magical_attack)
  {
    if (ch->level >= (victim->level + 10) && number_percent () > 50)
    {
      char *attack;
      char buf1[MAX_STRING_LENGTH];
      char buf2[MAX_STRING_LENGTH];
      char buf3[MAX_STRING_LENGTH];
      if (dt >= 0 && dt < MAX_SKILL)
        attack = skill_table[dt].noun_damage;

      else if (dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
        attack = attack_table[dt - TYPE_HIT].noun;

      else
      {
        bug ("Dam_message: bad dt %d.", dt);
        dt = TYPE_HIT;
        attack = attack_table[0].name;
      }
      if (is_tail_attack)
        attack = "lightning tail";
      sprintf (buf1,
        "$n's %s is absorbed by the magic of $N's mirror.",
        attack);
      sprintf (buf2,
        "Your %s is absorbed by the magic of $N's mirror.",
        attack);
      sprintf (buf3,
        "$n's %s is absorbed by the magic of your mirror.",
        attack);
      act (buf1, ch, NULL, victim, TO_NOTVICT);
      act (buf2, ch, NULL, victim, TO_CHAR);
      act (buf3, ch, NULL, victim, TO_VICT);

      //return FALSE;
    }
    mirror_attack = TRUE;
  }
  if (mirror_attack)
  {
    if (show)
      dam_message (ch, victim, dam, dt, 3);
    return FALSE;
  }

  /*
  * Hurt the victim   * Inform the victim of his new state.
  */
  victim->hit = -50;
  if (!IS_NPC (victim) && victim->level >= LEVEL_IMMORTAL && victim->hit < 1)
    victim->hit = 1;
  check_saddle (victim);
  update_pos (victim);
  switch (victim->position)
  {
  case POS_MORTAL:
    act ("$n is mortally wounded, and will die soon, if not aided.",
      victim, NULL, NULL, TO_ROOM);
    send_to_char
      ("`CYou are mortally wounded, and will die soon, if not aided.``\n\r",
      victim);
    break;
  case POS_INCAP:
    act ("$n is incapacitated and will slowly die, if not aided.",
      victim, NULL, NULL, TO_ROOM);
    send_to_char
      ("`CYou are incapacitated and will slowly die, if not aided.``\n\r",
      victim);
    break;
  case POS_STUNNED:
    act ("$n is stunned, but will probably recover.", victim, NULL,
      NULL, TO_ROOM);
    send_to_char
      ("`CYou are stunned, but will probably recover.``\n\r", victim);
    break;
  case POS_DEAD:
    victim->position = POS_STANDING;
    if (HAS_SCRIPT (victim))
    {
      TRIGGER_DATA *pTrig;
      for (pTrig = victim->triggers; pTrig != NULL; pTrig = pTrig->next)
        if (pTrig->trigger_type == TRIG_DIES)
        {
          act_trigger (victim, pTrig->name, NULL, NAME (ch),
            NAME (victim));
          pTrig->current = pTrig->script;
          while (pTrig->current != NULL)
          {
            pTrig->bits = SCRIPT_ADVANCE;
            script_interpret (victim, pTrig);
            if (pTrig->current != NULL)
              pTrig->current = pTrig->current->next;

            else
              pTrig->tracer = 0;
          }
        }
    }
    victim->position = POS_DEAD;
    if (dt == gsn_assassinate)
    {
      send_to_char
        ("`bYou are hit in the heart and die instantly.\r\n``", victim);
      act ("$n is DEAD!!", victim, NULL, NULL, TO_ROOM);
      act ("You hit $N in the heart, causing $M to die instantly.",
        ch, NULL, victim, TO_CHAR);
    }
    if (magical_attack)
    {
      send_to_char
        ("You have a violent allergic reaction to your bee stings and die!\n\r",
        victim);
      act
        ("$n has a violent allergic reaction to $s bee stings and dies!",
        victim, NULL, NULL, TO_ROOM);
    }
    if ((ch->clan != CLAN_BOGUS) && (victim->clan != CLAN_BOGUS) && !(battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner))) && !ch->dueler)
      clanwar_kill (ch, victim);
    if (victim != ch)
      if (!IS_NPC (ch) && !IS_NPC (victim)
        && /*(ch->aggres == NULL) && */ IS_AGGRESSOR (ch, victim))
      {
        if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
          && !IS_SET (victim->act, PLR_WANTED)
          && !victim->race == PC_RACE_LICH
          && !(victim->Class == PC_CLASS_CHAOS_JESTER)
          && !is_warring (ch->clan, victim->clan)
          && !is_dueling (ch) && !battle_royale && !ch->dueler && ch->clan != clanname_to_slot("guttersnipes"))
          set_wanted (ch);
        if (!ch->pcdata->loner &&
          (ch->clan == CLAN_BOGUS) && !is_dueling (ch)
          && !battle_royale && !ch->dueler)
        {
          ch->pcdata->loner = TRUE;
          send_to_char
            ("For your crimes against humanity, you are now marked a loner.\n\r",
            ch);
        }
      }
      if (!IS_NPC(victim) && IS_NPC(ch) && battle_royale)
        calculate_br_points (ch, victim);
      //IBLIS 6/20/03 - Calculate Battle Royale points
      if (!IS_NPC (ch) && !IS_NPC (victim))
        if (battle_royale && ((!IS_NPC(ch) && ch->pcdata->loner) || ch->level > 19) && victim->level > 19)
          calculate_br_points (ch, victim);
      if (!IS_NPC (ch) && !IS_NPC (victim) &&
        (ch->clan != CLAN_BOGUS) &&
        !battle_royale &&
        (ch->clan == victim->clan) && is_clan_leader (victim))
        clan_leader (victim, ch->name);
      if (!IS_NPC (ch))
      {
        if (IS_NPC (victim))
          ch->pcdata->has_killed[MOB_KILL]++;

        else if (ch != victim)
        {
          if (battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner)))
            ch->pcdata->has_killed[BR_KILL]++;
          else if (ch->dueler)
            ch->pcdata->has_killed[SD_KILL]++;
          else ch->pcdata->has_killed[PLAYER_KILL]++;
        }
      }
      if (!IS_NPC (victim))
      {
        if (IS_NPC (ch))
          victim->pcdata->been_killed[MOB_KILL]++;

        else if (ch != victim)
        {
          if (battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner)))
            victim->pcdata->been_killed[BR_KILL]++;
          else if (victim->dueler)
            victim->pcdata->been_killed[SD_KILL]++;
          else victim->pcdata->been_killed[PLAYER_KILL]++;
        }
      }

      /* BEGIN ARENA */
      if (is_dueling (victim))
      {
        duel_ends (victim);
        return FALSE;
      }

      if (victim->dueler)
      {
        superduel_ends (ch,victim);
        return FALSE;
      }

      /* END ARENA */
      break;
  default:
    if (dam > victim->max_hit / 4)
      send_to_char
      ("`CYou reel in shock from the tremendous pain!``\n\r", victim);
    if (victim->hit < victim->max_hit / 4)
      send_to_char ("`CYou sure are BLEEDING!``\n\r", victim);
    break;
  }

  /*
  * Sleep spells and extremely wounded folks.
  */
  if (!IS_AWAKE (victim))
    stop_fighting (victim, FALSE);

  /*
  * Payoff for killing things.
  */
  if (victim->position == POS_DEAD)
  {
    if (!is_dueling (victim) && !victim->dueler)
      //Iblis 6/29/03
      //Needed to fix Avatar Reavers who die from > 0 align
      if (group_gain (ch, victim))
      {
        raw_kill (victim, victim);
        return TRUE;
      }

      if (!IS_NPC (victim) 
        && !((IS_SET(victim->act,PLR_WANTED) && IS_NPC (ch) && ch->spec_fun != 0
        && (!str_cmp("spec_executioner",spec_name (ch->spec_fun))
        || !str_cmp("spec_guard",spec_name (ch->spec_fun))))))
      {
        sprintf (log_buf, "%s killed by %s at %d", victim->name,
          (IS_NPC (ch) ? ch->short_descr : ch->name),
          ch->in_room->vnum);
        log_string (log_buf);
        if (!is_dueling (victim) && !(battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner))) && !victim->dueler)
        {

          //You only lose max 1/10 xp needed for next level
          //
          if (ch->level > 1)
          {
            long xploss;
            if (IS_NPC (ch))
              xploss = (victim->exp / 2.0);

            else
              xploss = (victim->exp / 4.0);
            if (victim->level > 60)
            {
              if (xploss > (level_cost (victim->level) / 2.0))
                xploss = (level_cost (victim->level) / 2.0);
            }
            else 
            {
              if (xploss > (level_cost (victim->level) / 5.0))
                xploss = (level_cost (victim->level) / 5.0);
            }
            if (victim->race != PC_RACE_NERIX)
            {
              victim->pcdata->xp_last_death = xploss;
              victim->pcdata->last_death_counter = victim->perm_stat[STAT_CON];
            }
            if (victim->race != PC_RACE_NERIX)
              gain_exp (victim, 0 - xploss);
            else gain_exp (victim, 0 - (xploss/2)); 
          }

          else
            send_to_char
            ("Since you are level 1, you lose no experience.\n\r", ch);
        }
      }

      /* Make the head for a bountied PC */
      if (!IS_NPC (victim) && !IS_NPC (ch) && is_bountied (victim))
      {
        char buf[MAX_STRING_LENGTH];
        char *name;
        OBJ_DATA *obj;
        name = str_dup (victim->name);
        obj = create_object (get_obj_index (OBJ_VNUM_SEVERED_HEAD), 0);
        obj->extra_flags[0] |= ITEM_NODROP | ITEM_NOUNCURSE;
        sprintf (buf, "%s %s", "head", name);
        free_string (obj->name);
        obj->name = str_dup (buf);
        sprintf (buf, obj->short_descr, name);
        free_string (obj->short_descr);
        obj->short_descr = str_dup (buf);
        sprintf (buf, obj->description, name);
        free_string (obj->description);
        obj->description = str_dup (buf);
        obj_to_char (obj, ch);
        free_string (name);
      }
      raw_kill (victim, ch);
      if (IS_NPC(ch) && IS_SET (ch->act, ACT_LOOTER))
      {
        do_get (ch, "all corpse");
        do_get (ch, "all 2.corpse");
        do_wear (ch, "all");
      }
      if (ch != victim && !IS_NPC (ch) && !is_same_clan (ch, victim)
        && victim->times_wanted < 65)
        remove_wanted (victim);
      if (!IS_NPC (ch) && IS_NPC (victim))
      {
        OBJ_DATA *coins;
        corpse = get_obj_list (ch, "corpse", ch->in_room->contents);
        if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse && corpse->contains)	/* exists and not empty */
          do_get (ch, "all corpse");
        if (IS_SET (ch->act, PLR_AUTOGOLD) && corpse && corpse->contains &&	/* exists and not empty */
          !IS_SET (ch->act, PLR_AUTOLOOT))
          if ((coins =
            get_obj_list (ch, "gcash", corpse->contains)) != NULL)
            do_get (ch, "all.gcash corpse");
        if (get_skill(ch,gsn_butcher) > 0
          && (IS_SET (ch->act, PLR_AUTOBUTCHER)))
          do_butcher (ch, "corpse");
        if (IS_SET (ch->act, PLR_AUTOSAC))
        {
          if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse
            && corpse->contains)
            return TRUE;	/* leave if corpse has treasure */

          else
            do_sacrifice (ch, "corpse");
        }
      }
      return TRUE;
  }
  if (victim == ch)
    return TRUE;

  /*
  * Take care of link dead people.
  */
  if (!IS_NPC (victim) && victim->desc == NULL)
  {
    if (number_range (0, victim->wait) == 0)
    {
      do_recall (victim, "");
      return TRUE;
    }
  }
  return TRUE;
}

//Iblis - function to calculate battle royale points for each kill.  Called when a player is killed during
//Battle Royale
void calculate_br_points (CHAR_DATA * ch, CHAR_DATA * victim)
{
  int gain = 0;
  char ttbuf[MAX_STRING_LENGTH];
  if (IS_IMMORTAL (ch) || IS_IMMORTAL (victim))
    return;
  if (ch == victim)
    gain = -victim->level;

  else
    gain = victim->level;
  if (IS_NPC(ch))
    gain = ch->level/3.0;
  else if (victim->level > ch->level)
    gain += (victim->level - ch->level) * 2;
  if (last_br_kill == NULL)
  {
    last_br_kill = ch;
    last_br_victim = victim;
    gain += 25;
    last_br_points = gain;
  }

  else
  {
    if (br_leader == victim)
      gain += 50;
    last_br_kill = ch;
    last_br_victim = victim;
    last_br_points = gain;
  }
  if (!IS_NPC(ch))
  {
    ch->pcdata->br_points += gain;
    sprintf (ttbuf, "`gYou recieve `b%d `gpoints for that kill.``\r\n", gain);
    send_to_char (ttbuf, ch);

    sprintf (ttbuf, "`b%s `igot creamed by `b%s`i, who earned `b%d`i points for it.``", victim->name, ch->name, gain);
  }
  else sprintf (ttbuf, "`b%s `igot creamed by `b%s`i and lost `b%d`i points for it.``", victim->name, ch->name, 0-gain);
  do_echo(ch,ttbuf);
  sprintf (ttbuf,
    "`gYour opponent received `b%d `gpoints for that kill.``\r\n",
    gain);
  send_to_char (ttbuf, victim);
  victim->pcdata->br_death_points += gain;
  if (!IS_NPC(ch))
  {
    if (br_leader == NULL
      || br_leader->pcdata->br_points < ch->pcdata->br_points)
    {
      br_leader = ch;
      send_to_char
        ("`gAnd with that kill, you are the new Battle Royal Leader!``\r\n",
        ch);
      sprintf (ttbuf,
        "`gAnd we have a new Battle Royal Leader -> `b%s``\r\n",
        ch->name);
      do_echo (ch, ttbuf);
    }
  }
}

//Iblis - function to tell if a player is on an aggessor list
bool IS_AGGRESSOR (CHAR_DATA * ch, CHAR_DATA * victim)
{
  AGGRESSOR_LIST *a, *a_next;
  if (IS_NPC (ch) || IS_NPC (victim))
    return FALSE;
  for (a = ch->pcdata->agg_list; a != NULL; a = a_next)
  {
    a_next = a->next;
    if (a->ch == victim)
      return FALSE;
  }
  return TRUE;
}

//Iblis - old update aggressor function, which simply calls the new function without
//making the action silent
void update_aggressor (CHAR_DATA * ch, CHAR_DATA * victim)
{
  update_aggressor2 (ch,victim,FALSE);
}

//Iblis - Sets an aggressor of victim to be CH, since ch is attacking victim
void update_aggressor2 (CHAR_DATA * ch, CHAR_DATA * victim, bool silent)
{
  AGGRESSOR_LIST *a, *a_next, *a_prev=NULL;
  bool found = FALSE;
  if (IS_NPC (ch) || IS_NPC (victim) || battle_royale || ch->dueler || victim->dueler)
    return;
  if (IS_IMMORTAL(ch) || IS_IMMORTAL(victim))
    return;
  if (!IS_AGGRESSOR (ch, victim))
    return;
  if (is_dueling (ch) || ch->dueler || victim->dueler)
    return;
  for (a = victim->pcdata->agg_list; a != NULL; a = a_next)
  {
    a_next = a->next;
    a_prev = a;
    if (a->ch == ch)
    {
      a->ticks_left = 2;
      found = TRUE;
      break;
    }
  }
  if (!found)
  {

    if (!silent)
    {
      act
        ("You have been provoked!  You are now free to get revenge against $N!",
        victim, NULL, ch, TO_CHAR);
      act ("You have provoked $N, so $E is now free to get revenge!", ch,
        NULL, victim, TO_CHAR);
    }
    a = new_aggressor ();
    if (victim->pcdata->agg_list)
      a_prev->next = a;

    else
      victim->pcdata->agg_list = a;
    a->next = NULL;
    a->ch = ch;
    a->silent = silent;
    a->ticks_left = 2;
  }
}


//Iblis 6/28/03
//(removes agressor ch from victim)
void remove_aggressor (CHAR_DATA * ch, CHAR_DATA * victim)
{
  AGGRESSOR_LIST *a, *a_next, *a_prev=NULL;
  char buf[MAX_STRING_LENGTH];
  if (IS_NPC (ch) || IS_NPC (victim) || ch->pcdata == NULL
    || victim->pcdata == NULL)
    return;
  for (a = victim->pcdata->agg_list; a != NULL; a = a_next)
  {
    a_next = a->next;
    sprintf (buf, "INFO - Aggressor - %s, Char - %s, Victim - %s",
      a->ch->name, ch->name, victim->name);
    log_string (buf);
    if (a->ch == ch)
    {
      if (ch != NULL && !a->silent)
      {
        act
          ("Your time for free revenge against $N has come to an end.",
          victim, NULL, ch, TO_CHAR);
        act
          ("$N's time for free revenge against you has come to an end.",
          ch, NULL, victim, TO_CHAR);
      }
      if (a == victim->pcdata->agg_list)
        victim->pcdata->agg_list = a_next;

      else
        (a_prev->next = a_next);
      free_aggressor_list (a);
      a_next = NULL;
    }
    a_prev = a;
  }
}

//Iblis - Ranger Mount pounce skill
void do_pounce (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim, *vch, *fch, *fch_next;
  int chance, x;		
  char arg[MAX_INPUT_LENGTH];
  char tcbuf[MAX_INPUT_LENGTH];

  short door = 0;
  ROOM_INDEX_DATA *in_room = NULL;
  ROOM_INDEX_DATA *to_room = NULL;

  OBJ_DATA *tObj;
  EXIT_DATA *pexit;
  one_argument (argument, arg);
  if (IS_NPC (ch))
    return;
  if ((chance = get_skill (ch, gsn_pounce)) <= 0)
  {
    send_to_char ("Pouncing? What's that?\n\r", ch);
    return;
  }
  if (arg[0] == '\0')
  {
    send_to_char ("Pounce on whom?\r\n", ch);
    return;
  }
  if (!ch->mount || !ch->mount->pIndexData
    || ch->mount->pIndexData->vnum != MOB_VNUM_BEAST)
  {
    send_to_char
      ("You cannot pounce unless you are riding the `dWoodland `bBehemoth``.\n\r",
      ch);
    return;
  }
  if ((victim = get_char_room (ch, arg)) != NULL)
  {
    send_to_char ("You need a running start to pounce on someone.\n\r", ch);
    return;
  }

  else
  {
    for (x = 0; x < MAX_DIR; x++)
      if (ch->in_room->exit[x] != NULL
        && ch->in_room->exit[x]->u1.to_room != NULL
        && !IS_SET (ch->in_room->exit[x]->exit_info, EX_CLOSED))
      {
        for (vch = ch->in_room->exit[x]->u1.to_room->people; vch;
          vch = vch->next_in_room)
          if (is_name (argument, vch->name) && can_see_hack (ch, vch))
          {
            victim = vch;
            door = x;
            break;
          }
          if (victim != NULL)
            break;
      }
  }
  if (victim == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (ch->mount != NULL)
    if (ch->mount->position != POS_STANDING
      && ch->mount->position != POS_FIGHTING)
    {
      sprintf (tcbuf,
        "%s will have to stand before it can pounce.\n\r",
        fcapitalize (ch->mount->short_descr));
      send_to_char (tcbuf, ch);
      return;
    }
    if (IS_AFFECTED (ch, AFF_CHARM)
      && ch->master != NULL && in_room == ch->master->in_room)
    {
      send_to_char ("What?  And leave your beloved master?\n\r", ch);
      return;
    }
    if (ch->fighting != NULL)
    {
      send_to_char ("You can't do that while fighting.\r\n", ch);
      return;
    }
    if (!victim->in_room)
    {
      bug ("NULL victim->in_room in pounce", 0);
      send_to_char ("Nuh uh.\n\r", ch);
      return;
    }
    if (IS_SET (ch->in_room->room_flags, ROOM_SAFE))
    {
      send_to_char ("You cannot pounce from a safe room.\n\r", ch);
      return;
    }
    if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
    {
      send_to_char ("You cannot pounce into a safe room.\n\r", ch);
      return;
    }
    if (is_affected (ch, gsn_fear))
    {
      send_to_char ("You are way too scared to consider attacking anyone.\n\r", ch);
      return;
    }
    if (IS_AFFECTED (ch, AFF_CALM)
      || is_affected (ch, skill_lookup ("Quiet Mountain Lake")))
    {
      send_to_char ("You are feeling too peaceful to do that now.\n\r", ch);
      return;
    }
    if (is_affected (ch, gsn_entangle))
    {
      send_to_char ("You are too constrained to initiate combat.\n\r", ch);
      return;
    }
    if (is_affected (ch, skill_lookup ("web")))
    {
      send_to_char
        ("The webs binding you make movement difficult and pouncing impossible.\n\r",
        ch);
      return;
    }

    if (is_safe (ch, victim))
      return;
    if (check_shopkeeper_attack (ch, victim))
      return;

    if (!can_move_char_door (ch, door, FALSE, FALSE))
    {
      send_to_char ("You cannot dive in the direction of that char.\r\n", ch);
      return;
    }
    if (victim == ch)
    {
      send_to_char
        ("You attempt to dive at yourself, but you see yourself coming and step aside.\n\r",
        ch);
      return;
    }
    act ("Your `dWoodland `bBehemoth ``takes off running to pounce on $N.",
      ch, NULL, victim, TO_CHAR);

    act ("$n's `dWoodland `bBehemoth ``takes off running to the $T.",
      ch, NULL, dir_name[door], TO_ROOM);

    in_room = ch->in_room;
    pexit = in_room->exit[door];
    to_room = pexit->u1.to_room;
    if (pexit == NULL || to_room == NULL)
      send_to_char ("uh oh\r\n", ch);

    /* Move the Mount */
    if (ch->mount != NULL)
    {
      char_from_room (ch->mount);
      char_to_room (ch->mount, to_room);
    }

    else
    {
      bug ("Null mount in Pounce", 0);
      return;
    }

    /* Move the Character */
    char_from_room (ch);
    char_to_room (ch, to_room);
    do_look (ch, "auto");

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
    for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
    {

      fch_next = fch->next_in_room;

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
        && can_see (ch, fch) && get_position(ch) > POS_SLEEPING)
      {
        if (ch->position < POS_STANDING)
          do_stand (ch, "");
        do_kill (ch, NAME (fch));
      }
    }
    //Ignore tripwires for this skill, cause I'm too lazy to deal with them
    for (tObj = ch->in_room->contents; tObj; tObj = tObj->next_content)
      if (tObj->pIndexData->vnum == OBJ_VNUM_WALL_THORNS
        && tObj->value[0] == rev_dir[door])
      {
        wall_damage (ch, tObj);
        break;
      }
      if (ch->tracking[0] != '\0')
        display_track (ch);

      if (victim->mount != NULL)
        chance -= 15;
      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(ch,"room",ch->in_room,NULL);
      sprintf(tcbuf,"%d",ch->in_room->vnum);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      if (check_aggression(ch))
        return;
      if (number_percent () < chance)
      {
        if (victim->mount)
        {
          sprintf (tcbuf,
            "A `dWoodland `bBehemoth`` comes flying into into the room, knocking $N off %s.",
            victim->mount->short_descr);
          act (tcbuf, victim, NULL, ch, TO_NOTVICT);
          sprintf (tcbuf,
            "A `dWoodland `bBehemoth`` comes flying into the room and knocks you off %s!",
            victim->mount->short_descr);
          act (tcbuf, victim, NULL, ch, TO_CHAR);
          sprintf (tcbuf,
            "Your `dWoodland `bBehemoth`` leaps into the air and lands on $N, knocking $M off %s.",
            victim->mount->short_descr);
          act (tcbuf, ch, NULL, victim, TO_CHAR);
          victim->mount->riders = NULL;
          victim->mount = NULL;
        }

        else
        {
          act
            ("A `dWoodland `bBehemoth`` comes flying into into the room, landing on $N.",
            victim, NULL, ch, TO_NOTVICT);
          act
            ("A `dWoodland `bBehemoth`` comes flying into the room and lands on top of you!",
            victim, NULL, ch, TO_CHAR);
          act
            ("Your `dWoodland `bBehemoth`` leaps into the air and lands squarely on $N.",
            ch, NULL, victim, TO_CHAR);
        }

        damage (ch, victim, 1.5 * ch->level, gsn_pounce, DAM_BASH, FALSE);
        victim->position = POS_RESTING;
        check_improve (ch, gsn_pounce, TRUE, 2);
        WAIT_STATE (ch, skill_table[gsn_pounce].beats);
      }

      else
      {
        act
          ("A `dWoodland `bBehemoth`` comes flying into the room, missing $n and causing $N to fall off.",
          victim, NULL, ch, TO_NOTVICT);
        act
          ("Your `dWoodland `bBehemoth`` leaps into the air, but misses $N, causing you to fall off.",
          ch, NULL, victim, TO_CHAR);
        act
          ("A `dWoodland `bBehemoth`` comes flying into the room, missing you and causing #N to fall off.",
          victim, NULL, ch, TO_CHAR);
        ch->mount->riders = NULL;
        ch->mount = NULL;
        damage (ch, victim, 0, gsn_pounce, DAM_BASH, FALSE);
        ch->position = POS_RESTING;
        check_improve (ch, gsn_pounce, FALSE, 2);
        WAIT_STATE (ch, skill_table[gsn_pounce].beats);
      }
}

//Iblis - Ranger's Mount's disorient skill
void do_disorient (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;
  if ((chance = get_skill (ch, gsn_disorientation)) == 0)
  {
    send_to_char ("You attempt to disorient yourself.\n\r", ch);
    return;
  }
  //Iblis - 10/27/04 - Largely requested by other imms
  if (IS_IMMORTAL(ch))
  {
    int ticks = 0;
    AFFECT_DATA af;
    argument = one_argument (argument, arg);
    if (arg[0] == '\0')
    {
      victim = ch->fighting;
      if (victim == NULL)
      {
        send_to_char ("But you aren't in combat!\n\r", ch);
        return;
      }
    }

    else if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
    ticks = atoi(argument);
    if (ticks <= 0 || ticks > 32000)
      ticks = 10;
    af.where = TO_AFFECTS;
    af.type = gsn_disorientation;
    af.level = ch->level;
    af.duration = ticks;
    af.location = APPLY_HITROLL;
    af.modifier = -4;
    af.permaff = FALSE;
    af.bitvector = 0;
    af.composition = FALSE;
    af.comp_name = str_dup ("");
    affect_join (victim, &af);
    send_to_char ("You suddenly feel disoriented!\n\r", victim);
    sprintf(arg,"You disorient $N for %d ticks.",ticks);
    act(arg,ch,NULL,victim,TO_CHAR);
    return;
  }
  if (!ch->mount || !ch->mount->pIndexData
    || ch->mount->pIndexData->vnum != MOB_VNUM_BEAST)
  {
    send_to_char
      ("You cannot disorient someone unless you are riding the `dWoodland `bBehemoth``.\n\r",
      ch);
    return;
  }
  if (!ch->in_room)
  {
    bug ("null ch->in_room in disorient", 0);
    send_to_char ("Nuh uh.\n\r", ch);
    return;
  }
  if (ch->in_room->sector_type == SECT_CITY ||  !IS_OUTSIDE(ch))
  {
    send_to_char
      ("Your `dWoodland `bBehemoth`` refuses your disorient request.\n\r",
      ch);
    return;
  }
  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
  {
    victim = ch->fighting;
    if (victim == NULL)
    {
      send_to_char ("But you aren't in combat!\n\r", ch);
      return;
    }
  }

  else if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  /*  if (IS_IMMORTAL(ch))
  {
  int ticks = 0;
  AFFECT_DATA af;
  ticks = atoi(argument);
  if (ticks < 0 || ticks > 32000)
  ticks = 10;
  af.where = TO_AFFECTS;
  af.type = gsn_disorientation;
  af.level = ch->level;
  af.duration = ticks;
  af.location = APPLY_HITROLL;
  af.modifier = -4;
  af.permaff = FALSE;
  af.bitvector = 0;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (victim, &af);
  send_to_char ("You suddenly feel disoriented!\n\r", victim);
  return;
  }*/
  if (IS_AFFECTED (victim, AFF_BLIND))
  {
    act ("$E's already been blinded.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (victim == ch)
  {
    send_to_char ("Very funny.\n\r", ch);
    return;
  }
  if (check_shopkeeper_attack (ch, victim))
    return;
  if (is_safe (ch, victim))
    return;
  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
  {
    act ("But $N is such a good friend!", ch, NULL, victim, TO_CHAR);
    return;
  }

  /* dexterity */
  chance += get_curr_stat (ch, STAT_DEX);
  chance -= 2 * get_curr_stat (victim, STAT_DEX);

  /* speed  */
  if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
    chance += 10;
  if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
    chance -= 25;

  /* sloppy hack to prevent false zeroes */
  if (chance % 5 == 0)
    chance += 1;
  if (is_affected (victim, gsn_mirror))
    victim = ch;

  /* now the attack */
  if (number_percent () < chance)
  {
    AFFECT_DATA af;
    act ("$n is disoriented by the spit in $s eyes!", victim, NULL,
      NULL, TO_ROOM);
    if (victim != ch)
      act ("$n's `dWooden `bBehemoth`` spits in your eyes!", ch,
      NULL, victim, TO_VICT);

    else
      act ("The spit is reflected back into your eyes!", ch, NULL,
      victim, TO_VICT);
    af.where = TO_AFFECTS;
    af.type = gsn_disorientation;
    af.level = ch->level;
    af.duration = 1;
    af.location = APPLY_HITROLL;
    af.modifier = -4;
    af.permaff = FALSE;
    af.bitvector = 0;
    af.composition = FALSE;
    af.comp_name = str_dup ("");
    affect_to_char (victim, &af);
    damage (ch, victim, number_range (2, 5), gsn_disorientation,
      DAM_NONE, FALSE);
    send_to_char ("You suddenly feel disoriented!\n\r", victim);
    check_improve (ch, gsn_disorientation, TRUE, 2);
    WAIT_STATE (ch, skill_table[gsn_disorientation].beats);
  }

  else
  {
    damage (ch, victim, 0, gsn_disorientation, DAM_NONE, TRUE);
    check_improve (ch, gsn_disorientation, FALSE, 2);
    WAIT_STATE (ch, skill_table[gsn_disorientation].beats);
  }
}

//
