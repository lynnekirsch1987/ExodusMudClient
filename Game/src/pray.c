#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"
#include "pray.h"
#include "recycle.h"
#include "tables.h"
#include "obj_trig.h"

long int low_gain_exp args ((CHAR_DATA * ch, long int gain));
bool br_vnum_check args ((int vnum));
extern bool exploration_tracking;
extern int getbit args ((char *explored, int index));
extern bool battle_royale;
extern bool nogate;
void spell_create_food args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
long int level_cost args ((int level));


char *condition_noun args ((int condition));

char *pray_target_name;

void say_prayer (CHAR_DATA * ch, int sn)
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
    char *newsyl;
  };

  static const struct syl_type syl_table[] = {
    {" ", " "},
    {"ar", "abra"},
    {"au", "kada"},
    {"bless", "fido"},
    {"blind", "krow"},
    {"bur", "sume"},
    {"cu", "lodi"},
    {"de", "krona"},
    {"en", "unso"},
    {"light", "dies"},
    {"lo", "hi"},
    {"mor", "zak"},
    {"move", "sido"},
    {"ness", "lacri"},
    {"ning", "illa"},
    {"per", "duda"},
    {"ra", "reo"},
    {"fresh", "mdo"},
    {"re", "bendi"},
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
    {"w", "x"},
    {"x", "n"},
    {"y", "l"},
    {"z", "k"},
    {"", ""}
  };

  buf[0] = '\0';
  for (pName = skill_table[sn].name; *pName != '\0'; pName += length)
    {
      for (iSyl = 0; (length = strlen (syl_table[iSyl].old)) != 0; iSyl++)
	{
	  if (!str_prefix (syl_table[iSyl].old, pName))
	    {
	      strcat (buf, syl_table[iSyl].newsyl);
	      break;
	    }
	}

      if (length == 0)
	length = 1;
    }

  sprintf (buf2, "$n orates the prayer, '%s'.", buf);
  sprintf (buf, "$n orates the prayer, '%s'.", skill_table[sn].name);

  for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
    {
      if (rch != ch)
	act (IS_CLASS (rch, PC_CLASS_CLERIC) ? buf : buf2, ch, NULL, rch, TO_VICT);
    }

  if (is_dueling (ch))
    {
      rid = get_room_index (ROOM_VNUM_SPECTATOR);

      for (rch = rid->people; rch; rch = rch->next_in_room)
	act (IS_CLASS (rch, PC_CLASS_CLERIC) ? buf : buf2, ch, NULL, rch, TO_VICT);
    }

  return;
}


void do_prayers (CHAR_DATA * ch, char *argument)
{
  char pray_list[MAX_LEVEL][MAX_STRING_LENGTH];
  char pray_columns[MAX_LEVEL];
  int sn, lev, mana;
  bool found = FALSE;
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC (ch))
    return;


  if (!has_skills (ch, SKILL_PRAY))
    {
      send_to_char ("You don't know any prayers.\n\r", ch);
      return;
    }

  if (ch->level > LEVEL_HERO)
    {
      for (lev = 0; lev < MAX_LEVEL; lev++)
	{
	  pray_columns[lev] = 0;
	  pray_list[lev][0] = '\0';
	}

      for (sn = 0; sn < MAX_SKILL; sn++)
	{
	  if (skill_table[sn].name == NULL)
	    break;

	  if (get_skill(ch,sn) > 0
	      && skill_table[sn].type == SKILL_PRAY)
	    {
	      found = TRUE;
	      lev = level_for_skill (ch, sn);
	      if (ch->level < lev)
		sprintf (buf, "%-18s  n/a      ", skill_table[sn].name);
	      else
		{
		  mana = UMAX (skill_table[sn].min_mana,
			       100 / (2 + ch->level - lev));
		  sprintf (buf, "%-18s  %3d mana  ", skill_table[sn].name,
			   mana);
		}

	      if (pray_list[lev][0] == '\0')
		sprintf (pray_list[lev], "\n\rLevel %2d: %s", lev, buf);
	      else
		/* append */
		{
		  if (++pray_columns[lev] % 2 == 0)
		    strcat (pray_list[lev], "\n\r          ");
		  strcat (pray_list[lev], buf);
		}
	    }
	}
    }
  else
    {

      for (lev = 0; lev < LEVEL_HERO; lev++)
	{
	  pray_columns[lev] = 0;
	  pray_list[lev][0] = '\0';
	}

      for (sn = 0; sn < MAX_SKILL; sn++)
	{
	  if (skill_table[sn].name == NULL)
	    break;

	  if (level_for_skill(ch,sn) < LEVEL_HERO
	      && skill_table[sn].type == SKILL_PRAY
	      && (ch->pcdata->learned[sn] > 0 || ch->pcdata->mod_learned[sn] > 0))
	    {
	      found = TRUE;
	      lev = level_for_skill (ch, sn);
	      if (ch->level < lev)
		sprintf (buf, "%-18s  n/a      ", skill_table[sn].name);
	      else
		{
		  mana = UMAX (skill_table[sn].min_mana,
			       100 / (2 + ch->level - lev));
		  sprintf (buf, "%-18s  %3d mana  ", skill_table[sn].name,
			   mana);
		}

	      if (pray_list[lev][0] == '\0')
		sprintf (pray_list[lev], "\n\rLevel %2d: %s", lev, buf);
	      else
		/* append */
		{
		  if (++pray_columns[lev] % 2 == 0)
		    strcat (pray_list[lev], "\n\r          ");
		  strcat (pray_list[lev], buf);
		}
	    }
	}
    }
  /* return results */

  if (!found)
    {
      send_to_char ("You know no prayers.\n\r", ch);
      return;
    }

  if (ch->level > LEVEL_HERO)
    {
      for (lev = 0; lev < MAX_LEVEL; lev++)
	if (pray_list[lev][0] != '\0')
	  send_to_char (pray_list[lev], ch);
    }
  else
    {
      for (lev = 0; lev < LEVEL_HERO; lev++)
	{
	  if (lev == 1)
	    if (ch->Class == 1 || ch->Class == 6)
	      {
		send_to_char ("\r\n`a:`b---------------------`a:``\r\n", ch);
		sprintf (buf, "`o%10s Prayers\r\n",
			 capitalize (Class_table[ch->Class].name));
		send_to_char (buf, ch);
		send_to_char ("`a:`b---------------------`a:``\r\n", ch);
	      }
	  if (lev == 31 && ch->level > 30 && ch->Class2 != ch->Class)
	    if (ch->Class != ch->Class2
		&& (ch->Class2 == 1 || ch->Class2 == 6))
	      {
		send_to_char ("\r\n`a:`b---------------------`a:``\r\n", ch);
		sprintf (buf, "`o%10s Prayers\r\n",
			 capitalize (Class_table[ch->Class2].name));
		send_to_char (buf, ch);
		send_to_char ("`a:`b---------------------`a:``\r\n", ch);
	      }
	  if (lev == 61 && ch->level > 60 && ch->Class2 != ch->Class3)
	    if (ch->Class2 != ch->Class3
		&& (ch->Class3 == 1 || ch->Class3 == 6)
		&& !(ch->Class2 != 1 && ch->Class2 != 6
		     && ch->Class == ch->Class3))
	      {
		send_to_char ("\r\n`a:`b---------------------`a:``\r\n", ch);
		sprintf (buf, "`o%10s Prayers\r\n",
			 capitalize (Class_table[ch->Class3].name));
		send_to_char (buf, ch);
		send_to_char ("`a:`b---------------------`a:``\r\n", ch);
	      }
	  if (pray_list[lev][0] != '\0')
	    send_to_char (pray_list[lev], ch);
	}
    }
  send_to_char ("\n\r", ch);
}

void do_pray (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  void *vo;
  int mana;
  int sn;
  int target;
  char buf[20];

  /* if (IS_NPC(ch) && ch->desc == NULL)
     return;
   */

  if (!has_skills (ch, SKILL_PRAY) && !IS_NPC (ch))
    {
      send_to_char ("You have not the faith to pray effectively.\n\r", ch);
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


  pray_target_name = one_argument (argument, arg1);

  one_argument (pray_target_name, arg2);

  if (arg1[0] == '\0')
    {
      send_to_char ("Pray what? where?\n\r", ch);
      return;
    }

  if ((sn = find_spell (ch, arg1)) < 0
      || (!IS_NPC (ch) && get_skill(ch,sn) < 1))
    {
      send_to_char ("You don't know any prayers of that name.\n\r", ch);
      return;
    }

  if (IS_SET(ch->act2,ACT_FAMILIAR) && get_skill(ch,sn) < 1)
      return;

  
  if (skill_table[sn].type != SKILL_PRAY)
    {
      send_to_char ("You don't know any prayers of that name.\n\r", ch);
      return;
    }

  if (ch->position < skill_table[sn].minimum_position
      || (ch->fighting != NULL
	  && skill_table[sn].minimum_position > POS_FIGHTING))
    {
      send_to_char ("You are not prepared to pray.\n\r", ch);
      return;
    }

  if (is_affected (ch, gsn_entangle))
    {
      send_to_char ("You are too constrained to use any of your mystical pow\
ers.\n\r", ch);
      return;
    }

  if (is_affected (ch, gsn_fear))
  {
	  send_to_char ("You are way too scared to orate that prayer right now.\n\r", ch);
	  return;
  }
  
  if (is_affected (ch, skill_lookup ("Quiet Mountain Lake"))
      && (skill_table[sn].target == TAR_CHAR_OFFENSIVE
	  || skill_table[sn].target == TAR_AREA_OFF
	  || skill_table[sn].target == TAR_OBJ_CHAR_OFF))
    {
      send_to_char
	("You are too peaceful to orate that prayer right now.\n\r", ch);
      return;
    }

  if (ch->level + 2 == level_for_skill (ch, sn))
    mana = 50;
  else
    mana = UMAX (skill_table[sn].min_mana,
		 100 / (2 + ch->level - level_for_skill (ch, sn)));

  if (is_affected (ch, gsn_mirror))
    {
      act ("The mirror arround you is destroyed by your outgoing prayer.",
	   ch, NULL, NULL, TO_CHAR);
      act ("The mirror around $n is destroyed by $s outgoing prayer.",
	   ch, NULL, NULL, TO_ROOM);
      affect_strip (ch, gsn_mirror);
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
      bug ("Do_pray: bad target for sn %d.", sn);
      return;

    case TAR_IGNORE:
    case TAR_AREA_OFF:
      break;

    case TAR_CHAR_OFFENSIVE:
      if (arg2[0] == '\0')
	{
	  if ((victim = ch->fighting) == NULL)
	    {
	      send_to_char
		("You must name a recipient for this prayer.\n\r", ch);
	      return;
	    }
	}
      else
	{
	  if ((victim = get_char_room (ch, pray_target_name)) == NULL)
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
	  if ((victim = get_char_room (ch, pray_target_name)) == NULL)
	    {
	      send_to_char ("They aren't here.\n\r", ch);
	      return;
	    }
	}

      vo = (void *) victim;
      target = TARGET_CHAR;
      break;

    case TAR_CHAR_SELF:
      if (arg2[0] != '\0' && !is_name (pray_target_name, ch->name))
	{
	  send_to_char
	    ("You may not direct this prayer towards another.\n\r", ch);
	  return;
	}

      vo = (void *) ch;
      target = TARGET_CHAR;
      break;

    case TAR_OBJ_INV:
      if (arg2[0] == '\0')
	{
	  send_to_char ("What should be the object of this prayer?\n\r", ch);
	  return;
	}

      if ((obj = get_obj_carry (ch, pray_target_name)) == NULL)
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
	      send_to_char
		("At who or what should this prayer be directed?\n\r", ch);
	      return;
	    }

	  target = TARGET_CHAR;
	}
      else if ((victim = get_char_room (ch, pray_target_name)) != NULL)
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
	      send_to_char ("You can't do that on your own follower.\n\r",
			    ch);
	      return;
	    }

	  vo = (void *) victim;
	}
      else if ((obj = get_obj_here (ch, pray_target_name)) != NULL)
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

    case TAR_OBJ_CHAR_DEF:
      if (arg2[0] == '\0')
	{
	  vo = (void *) ch;
	  target = TARGET_CHAR;
	}
      else if ((victim = get_char_room (ch, pray_target_name)) != NULL)
	{
	  vo = (void *) victim;
	  target = TARGET_CHAR;
	}
      else if ((obj = get_obj_carry (ch, pray_target_name)) != NULL)
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

  if (!IS_NPC (ch) && ch->mana < mana)
    {
      send_to_char ("You don't have enough mana.\n\r", ch);
      return;
    }
  if (!((IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_FAKIE)))
    say_prayer (ch, sn);

  if (!IS_IMMORTAL (ch))
    WAIT_STATE (ch, skill_table[sn].beats);

  if (number_percent () > get_skill (ch, sn))
    {
      send_to_char ("You lost your concentration.\n\r", ch);
      check_improve (ch, sn, FALSE, 1);
      ch->mana -= mana / 2;
    }
  else
    {
      CHAR_DATA *perp=NULL;
      AFFECT_DATA *af=NULL;
      ch->mana -= mana;
      if (IS_NPC (ch) || mana_using_Class (ch))
	/* Class has spells */
	{

	 (*skill_table[sn].spell_fun) (sn, ch->level, ch, vo, target);
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
          //      (*skill_table[sn].spell_fun) (sn, 3 * ch->level / 4, ch, vo,
          //                    target);
        }
	//	(*skill_table[sn].spell_fun) (sn, ch->level, ch, vo, target);
      else
	{
         (*skill_table[sn].spell_fun) (sn, 3 * ch->level / 4, ch, vo, target);
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
          //      (*skill_table[sn].spell_fun) (sn, 3 * ch->level / 4, ch, vo,
          //                    target);
        }
      //(*skill_table[sn].spell_fun) (sn, 3 * ch->level / 4, ch, vo, target);

      sprintf(buf, "%d", sn);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_USES_SPELL, NULL, victim, OT_TYPE_PRAYER, buf);
      
      check_improve (ch, sn, TRUE, 1);
    }

  if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
       || (skill_table[sn].target == TAR_OBJ_CHAR_OFF
	   && target == TARGET_CHAR)) && victim != ch && victim->master != ch)
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      for (vch = ch->in_room->people; vch; vch = vch_next)
	{
	  vch_next = vch->next_in_room;
	  if (victim == vch && victim->fighting == NULL &&
	      get_position(victim) > POS_SLEEPING)
	    {
	      multi_hit (ch, victim, TYPE_UNDEFINED);
	      break;
	    }
	}
    }

  return;
}

void prayer_renewal (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  victim->move = UMIN (victim->move + level, victim->max_move);
  if (victim->max_move == victim->move)
    send_to_char ("You feel fully renewed!\n\r", victim);
  else
    send_to_char ("You feel less tired.\n\r", victim);
  if (ch != victim)
    send_to_char ("Ok.\n\r", ch);
  return;
}

void
prayer_purification (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  bool found = FALSE;

  level += 2;

  if ((!IS_NPC (ch) && IS_NPC (victim) &&
       !(IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)) ||
      (IS_NPC (ch) && !IS_NPC (victim)))
    {
      send_to_char ("You failed..\n\r", ch);
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

  if (victim->race != PC_RACE_LITAN && 
		  check_dispel (level, victim, gsn_fireshield))
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
      act ("The white aura around $n's body vanishes.",
	   victim, NULL, NULL, TO_ROOM);
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


  if (check_dispel (level, victim, skill_lookup ("weaken")))
    {
      act ("$n looks stronger.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (found)
    send_to_char ("Ok.\n\r", ch);
  else
    send_to_char ("Prayer failed.\n\r", ch);
}

void prayer_nexus (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *portal, *stone;

  if (nogate || (!IS_NPC(ch) && ch->pcdata->nogate))
  {
	send_to_char("The gods are not allowing that at the moment.\n\r",ch);
	return;
  }
  
  stone = get_eq_char (ch, WEAR_HOLD);

  if (!IS_IMMORTAL (ch))
    {
      if (stone == NULL || (stone->item_type != ITEM_WARP_STONE &&
			    stone->pIndexData->vnum != OBJ_VNUM_RUNE_CONV))
	{
	  send_to_char
	    ("You lack the proper component for this prayer.\n\r", ch);
	  return;
	}
    }



  if (IS_SET (ch->in_room->race_flags, ROOM_NOTRANSPORT))
    {
      send_to_char ("You cannot leave this realm via ordinary magic.\n\r",
		    ch);
      return;
    }

  if (battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner)) && !br_vnum_check (ch->in_room->vnum))
    {
      send_to_char
	("The area you are trying to gate to is now off limits until the end of Battle Royale.\n\r",
	 ch);
      return;
    }


  if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
      act ("You draw upon the power of $p.", ch, stone, NULL, TO_CHAR);
      act ("It flares brightly and vanishes!", ch, stone, NULL, TO_CHAR);
      extract_obj (stone);
    }

  if (stone != NULL && stone->pIndexData->vnum == OBJ_VNUM_RUNE_CONV)
    act ("You draw upon the power of $p.", ch, stone, NULL, TO_CHAR);

  if ((victim = get_char_world (ch, pray_target_name)) == NULL
      || victim->in_room == NULL
/*      || !can_see_room(ch, victim->in_room) */
      || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
      || IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL)
      || IS_SET(victim->in_room->area->area_flags,AREA_SOLO)
      || IS_SET(victim->in_room->room_flags2,ROOM_SOLO)
      || (victim->in_room->area->construct)
      || (IS_SET(victim->in_room->area->area_flags,AREA_IMP_ONLY) && get_trust(ch) < MAX_LEVEL)
      || (!IS_NPC (victim) && victim->level >= LEVEL_IMMORTAL
	  && victim->level > ch->level)
      || (IS_NPC (victim) && IS_SET (victim->imm_flags, IMM_SUMMON)))
  //    || (IS_NPC (victim) && saves_spell (level - 2, victim, DAM_MENTAL)))
    {
      portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
      portal->timer = 1 + level / 25;
      portal->value[3] = PORTAL_FUCKED;

      free_string (portal->name);
      portal->name = str_dup ("nexus gate portal");

      free_string (portal->description);
      portal->description =
	str_dup
	("A shimmering black nexus rises from the ground, leading to parts unknown.");

      act ("$n makes arcane gestures and a glowing nexus appears!",
	   ch, NULL, NULL, TO_ROOM);
      send_to_char ("A nexus opens but seems unstable.\n\r", ch);

      obj_to_room (portal, ch->in_room);
      return;
    }

    if (exploration_tracking)
       if (getbit (ch->pcdata->explored, victim->in_room->vnum) == 0)
         {
           send_to_char ("Your know nothing about the location you are trying to reach\n\r", ch);
           return;         
         }


  if (victim->in_room == ch->in_room)
    {
      send_to_char ("Are you blind?\n\r", ch);
      return;
    }

  if (room_is_private (victim->in_room))
    {
      send_to_char ("A nexus fluxuates but never solidifies.\n\r", ch);
      return;
    }

  if (IS_SET (victim->in_room->race_flags, ROOM_NOTRANSPORT))
    {
      send_to_char ("You cannot travel there via ordinary magic.\n\r", ch);
      return;
    }

  portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
  portal->timer = 1 + level / 15;
  portal->value[3] = victim->in_room->vnum;
  portal->value[0] = -1;

  free_string (portal->description);
  portal->description =
    str_dup
    ("A shimmering black nexus rises from the ground, leading to parts unknown.");

  free_string (portal->name);
  portal->name = str_dup ("nexus gate portal");

  act ("$p rises up from the ground.", ch, portal, NULL, TO_ROOM);
  act ("$p rises up before you.", ch, portal, NULL, TO_CHAR);
  obj_to_room (portal, ch->in_room);


  portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
  portal->timer = 1 + level / 25;
  portal->value[3] = ch->in_room->vnum;
  portal->value[0] = -1;

  free_string (portal->name);
  portal->name = str_dup ("nexus gate portal");

  free_string (portal->description);
  portal->description =
    str_dup
    ("A shimmering black nexus rises from the ground, leading to parts unknown.");

  act ("$p rises up from the ground.", victim, portal, NULL, TO_ROOM);
  act ("$p rises up before you.", victim, portal, NULL, TO_CHAR);
  obj_to_room (portal, victim->in_room);
  return;
}


void
prayer_conservancy (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ((is_affected (victim, sn))
      || is_affected (victim, skill_lookup ("armor")))
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
  send_to_char ("You feel your faith protecting you.\n\r", victim);
  if (ch != victim)
    act ("$N is protected by your faith.", ch, NULL, victim, TO_CHAR);
  return;
}

void
prayer_divine_protection (int sn, int level, CHAR_DATA * ch, void *vo,
			  int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected (ch, sn))
    {
      if (victim == ch)
	send_to_char ("A heavenly force already protects you.\n\r", ch);
      else
	act ("$N is already protected by a heavenly force.", ch, NULL,
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
  af.location = APPLY_SAVING_SPELL;
  af.modifier = -2;
  affect_to_char (victim, &af);
  act ("$n is surrounded by a heavenly force.", victim, NULL, NULL, TO_ROOM);
  send_to_char ("A heavenly force surrounds you.\n\r", victim);
  return;
}

void
prayer_damnation (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;
  victim = (CHAR_DATA *) vo;

  if (IS_AFFECTED (victim, AFF_CURSE)
      || saves_spell (level, victim, DAM_HOLY))
    return;
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level/8;
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

  send_to_char ("You feel forsaken.\n\r", victim);
  if (ch != victim)
    act ("$N screams suddenly, feeling the void of being forsaken.", ch,
	 NULL, victim, TO_CHAR);
  return;
}

void prayer_halos (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;
  victim = (CHAR_DATA *) vo;

  if (ch->alignment < 500)
    {
      act ("Your god deems you unworthy of the power of the seraphs.", ch,
	   NULL, NULL, TO_VICT);
      return;
    }

  if (is_affected (victim, sn))
    {
      act ("You already possess the power of the seraphs.", ch, NULL,
	   victim, TO_VICT);
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 4;
  af.location = APPLY_CHA;
  af.modifier = 1;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (victim, &af);

  send_to_char ("You are granted the power of the seraphs.\n\r", victim);
  act ("$N's eyes begin to twinkle a sparkling gold color.", ch, NULL,
       victim, TO_ROOM);

  return;
}

void
prayer_illumination (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;
  victim = (CHAR_DATA *) vo;


  if (is_affected (victim, sn))
    {
      if (ch == victim)
	act ("You already possess divine wisdom.", ch, NULL, victim, TO_CHAR);
      else
	{
	  act ("$N already possesses divine wisdom.", ch, NULL, victim,
	       TO_CHAR);
	  act ("You already possess divine wisdom.", ch, NULL, victim,
	       TO_VICT);
	}
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 4;
  af.location = APPLY_WIS;
  af.modifier = 1;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (victim, &af);

  send_to_char ("You are granted divine wisdom.\n\r", victim);

  if (ch != victim)
    act ("$N's eyes begin to glow, as if $e had been suddenly inspired.", ch,
	 NULL, victim, TO_CHAR);

  return;
}

void
prayer_edification (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj, *obj_next;
  victim = (CHAR_DATA *) vo;
  if (ch->race == PC_RACE_SIDHE || ch->race == PC_RACE_LICH || ch->race == PC_RACE_NERIX)
  {
    send_to_char("You are too evil to pray that.\n\r",ch);
    return;
  }		


  send_to_char ("Your god guides you onto the path of righteousness.\n\r",
		victim);

  act ("$n is guided onto the path of righteousness.", victim, NULL,
       NULL, TO_ROOM);

  change_alignment(victim,level);

   if (IS_CLASS (victim, PC_CLASS_REAVER) && victim->alignment > 0
     && (get_eq_char (victim, WEAR_WIELD_L) != NULL
     || get_eq_char (victim, WEAR_WIELD_R) != NULL))
   {
     act ("`iYou are struck down as a traitor by the God of evil!``", victim, NULL, NULL, TO_CHAR);
     act ("`i$n is struck down as a traitor by the God of evil!``", victim, NULL, NULL, TO_ROOM);
     raw_kill (victim, victim);
     return;
   }


  for (obj = victim->carrying; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      if (obj->wear_loc == WEAR_NONE)
	continue;

      if ((victim->race != race_lookup ("avatar"))
	  && ((IS_OBJ_STAT (obj, ITEM_ANTI_EVIL) && IS_EVIL (victim))
	      || (IS_OBJ_STAT (obj, ITEM_ANTI_GOOD) && IS_GOOD (victim))
	      || (IS_OBJ_STAT (obj, ITEM_ANTI_NEUTRAL)
		  && IS_NEUTRAL (victim))))
	{
	  act ("You are zapped by $p.", victim, obj, NULL, TO_CHAR);
	  act ("$n is zapped by $p.", victim, obj, NULL, TO_ROOM);

	  obj_from_char (obj);
	  obj_to_room (obj, victim->in_room);
	}
    }

  return;
}

void
prayer_deconsecrate (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj, *obj_next;
  victim = (CHAR_DATA *) vo;

  if (ch->race == PC_RACE_SIDHE || ch->race == PC_RACE_LICH || ch->race == PC_RACE_NERIX)
  {
    send_to_char("You have no need for that prayer.\n\r",ch);
    return;
  }
  


  send_to_char ("You are drawn toward the darkness in your soul.\n\r",
		victim);
  act ("$n is drawn toward the darkness in $s soul.", victim, NULL, NULL,
       TO_ROOM);

  change_alignment(victim,0-level);

  if (IS_CLASS (victim, PC_CLASS_REAVER) && victim->alignment > 0
     && (get_eq_char (victim, WEAR_WIELD_L) != NULL
     || get_eq_char (victim, WEAR_WIELD_R) != NULL))
  {
      act ("`iYou are struck down as a traitor by the God of evil!``", victim, NULL, NULL, TO_CHAR);
      act ("`i$n is struck down as a traitor by the God of evil!``", victim, NULL, NULL, TO_ROOM);
      raw_kill (victim, victim);
      return;
  }
   

  for (obj = victim->carrying; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      if (obj->wear_loc == WEAR_NONE)
	continue;

      if ((victim->race != race_lookup ("avatar"))
	  && ((IS_OBJ_STAT (obj, ITEM_ANTI_EVIL) && IS_EVIL (victim))
	      || (IS_OBJ_STAT (obj, ITEM_ANTI_GOOD) && IS_GOOD (victim))
	      || (IS_OBJ_STAT (obj, ITEM_ANTI_NEUTRAL)
		  && IS_NEUTRAL (victim))))
	{
	  act ("You are zapped by $p.", victim, obj, NULL, TO_CHAR);
	  act ("$n is zapped by $p.", victim, obj, NULL, TO_ROOM);

	  obj_from_char (obj);
	  obj_to_room (obj, victim->in_room);
	}
    }

  return;
}

void prayer_penance (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  victim = (CHAR_DATA *) vo;

  if (victim->hit <= 100)
    {
      send_to_char
	("You aren't currently healthy enough to pray for penance.\n\r", ch);
      return;
    }

  send_to_char
    ("Your god answers your prayer by increasing your spirit at the cost of your physical health.\n\r",
     victim);
  act
    ("$n's burst of confidence seems to contradict $s sudden decrease in health.",
     victim, NULL, NULL, TO_ROOM);

  victim->mana = UMIN (victim->mana + 59, victim->max_mana);
  victim->hit -= 100;

  return;
}

void
prayer_sustenance (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  spell_create_food (sn,level,ch,vo,target);
  /*OBJ_DATA *mushroom;

  mushroom = create_object (get_obj_index (OBJ_VNUM_MUSHROOM), 0);
  mushroom->value[0] = level / 2;
  mushroom->value[1] = level;
  if (ch->in_room->sector_type == SECT_UNDERWATER)
    {

      act ("$p suddenly appears then disintegrates to mush.", ch,
	   mushroom, NULL, TO_ROOM);
      act ("$p suddenly appears then disintegrates to mush.", ch,
	   mushroom, NULL, TO_CHAR);
      return;
    }

  obj_to_room (mushroom, ch->in_room);
  act ("$p suddenly appears.", ch, mushroom, NULL, TO_ROOM);
  act ("$p suddenly appears.", ch, mushroom, NULL, TO_CHAR);
  return;*/
}

void
prayer_holy_water (int sn, int level, CHAR_DATA * ch, void *vo, int target)
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


void
prayer_soul_fire (int sn, int level, CHAR_DATA * ch, void *vo, int target)
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


void
prayer_levitation (int sn, int level, CHAR_DATA * ch, void *vo, int target)
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

  if (IS_AFFECTED (victim, AFF_FLYING))
    {
      if (victim == ch)
	send_to_char ("You are already airborne.\n\r", ch);
      else
	act ("$N doesn't need your help to fly.", ch, NULL, victim, TO_CHAR);
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

void
prayer_revelation (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  spell_identify(sn,level,ch,vo,target);
  return;
}


void
prayer_evocation (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *portal;

  if (nogate || (!IS_NPC(ch) && ch->pcdata->nogate))
  {
	send_to_char("The gods are not allowing that at the moment.\n\r",ch);
	return;
  } 
  
  // Akamai 4/17/99 - Fixed evocation so that clerics can not summon
  // characters to the ocean - added test to check that ch not in ocean 
  if ((victim = get_char_world (ch, pray_target_name)) == NULL
      || victim->in_room == NULL
      || victim->in_room == ch->in_room
      || (ch->in_room->sector_type == SECT_WATER_OCEAN)
      || (victim->in_room->sector_type == SECT_SWAMP)
      || IS_SET (victim->in_room->room_flags, ROOM_SAFE)
      || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
      || IS_NPC (victim)
      || victim->level >= LEVEL_IMMORTAL
      || (IS_SET(ch->in_room->area->area_flags,AREA_IMP_ONLY) && get_trust(victim) < MAX_LEVEL)
      || victim->level < 8
      || IS_SET(ch->in_room->area->area_flags,AREA_SOLO)
      || IS_SET(ch->in_room->room_flags2,ROOM_SOLO)
      || (!IS_NPC(victim) && victim->pcdata->nogate)
      || (!(battle_royale && (victim->level > 19 || (!IS_NPC(victim) && victim->pcdata->loner))) && IS_SET (victim->act, PLR_NOSUMMON))
      || victim->desc == NULL || victim->desc->pEdit != NULL)
    {

      send_to_char ("You failed.\n\r", ch);
      return;
    }

  if (IS_SET (ch->in_room->race_flags, ROOM_NOTRANSPORT))
    {
      act ("$N cannot enter this realm via ordinary magic.\n\r", ch, NULL,
	   victim, TO_CHAR);
      return;
    }

  portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
  portal->timer = 1 + level / 25;
  portal->value[0] = 1;
  portal->value[3] = ch->in_room->vnum;

  obj_to_room (portal, victim->in_room);

  act ("$p appears and beckons to you.", victim, portal, NULL, TO_CHAR);
  act ("$p appears with a pop.", victim, portal, NULL, TO_ROOM);

  if (saves_spell (level - 2, victim, DAM_MENTAL))
    act ("Your portal opens, but $N resists your compulsion to enter.",
	 ch, NULL, victim, TO_CHAR);
  else
    {
      enter_portal (victim, portal->name, FALSE);

      if (victim->in_room == ch->in_room)
	{
	  if (!is_affected (ch, gsn_blindness))
	    act ("$n has summoned you!", ch, NULL, victim, TO_VICT);

	  act ("You succeed in summoning $N.", ch, NULL, victim, TO_CHAR);
	}
      else
	act ("$N seems unable to enter your portal.", ch, NULL, victim,
	     TO_CHAR);
    }

  return;
}

void
prayer_celestial_sight (int sn, int level, CHAR_DATA * ch, void *vo,
			int target)
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
	  || !is_name_no_abbrev (pray_target_name, obj->name)
	  || IS_OBJ_STAT (obj, ITEM_NOLOCATE)
	  || obj->item_type == ITEM_CARD
	  || number_percent () > 2 * level || ch->level < obj->level-10)
	continue;


      for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);

      //IBLIS 6/19/03 - Made it so mortals can't locate objs on imms
      if (in_obj->carried_by != NULL && !IS_IMMORTAL (ch)
	  && IS_IMMORTAL (in_obj->carried_by) && !IS_NPC (in_obj->carried_by))
	continue;

      found = TRUE;
      number++;

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
		     in_obj->in_room == NULL
		     ? "somewhere" : in_obj->in_room->name);
	}

      buf[0] = UPPER (buf[0]);
      add_buf (buffer, buf);

      if (number >= max_found)
	break;
    }

  if (!found)
    send_to_char ("Nothing like that in heaven or earth.\n\r", ch);
  else
    page_to_char (buf_string (buffer), ch);

  free_buf (buffer);

  return;
}


void
prayer_heavenly_cloak (int sn, int level, CHAR_DATA * ch, void *vo,
		       int target)
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


void
prayer_hallowed_vision (int sn, int level, CHAR_DATA * ch, void *vo,
			int target)
{
  CHAR_DATA *ich;

  act ("$n has called forth a brilliant light.", ch, NULL, NULL, TO_ROOM);
  send_to_char ("You call forth a brilliant light.\n\r", ch);

  for (ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room)
    {
      if (ich->invis_level > 0)
	continue;
      if (ich == ch || saves_spell (level, ich, DAM_HOLY))
	continue;

      affect_strip (ich, gsn_invis);
      affect_strip (ich, gsn_mass_invis);
      affect_strip (ich, gsn_heavenly_cloak);
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


void prayer_bless (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;

  /* deal with the object case first */
  if (target == TARGET_OBJ)
    {
      obj = (OBJ_DATA *) vo;
      if (IS_OBJ_STAT (obj, ITEM_BLESS))
	{
	  act ("$p is already blessed.", ch, obj, NULL, TO_CHAR);
	  return;
	}

      if (IS_OBJ_STAT (obj, ITEM_EVIL))
	{
	  AFFECT_DATA *paf;

	  paf = affect_find (obj->affected, gsn_curse);
	  if (!saves_dispel (level, paf != NULL ? paf->level : obj->level, 0))
	    {
	      if (paf != NULL)
		affect_remove_obj (obj, paf);
	      act ("$p glows a pale blue.", ch, obj, NULL, TO_ALL);
	      REMOVE_BIT (obj->extra_flags[0], ITEM_EVIL);
	      return;
	    }
	  else
	    {
	      act ("The evil of $p is too powerful for you to overcome.",
		   ch, obj, NULL, TO_CHAR);
	      return;
	    }
	}

      af.where = TO_OBJECT;
      af.type = sn;
      af.level = level;
      af.duration = 6 + level;
      af.location = APPLY_SAVES;
      af.modifier = -1;
      af.bitvector = ITEM_BLESS;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup ("");

      affect_to_obj (obj, &af);

      act ("$p glows with a holy aura.", ch, obj, NULL, TO_ALL);
      return;
    }

  /* character target */
  victim = (CHAR_DATA *) vo;


  if (victim->position == POS_FIGHTING || ch->fighting != NULL
      || is_affected (victim, sn))
    {
      if (victim == ch)
	send_to_char ("You are already blessed.\n\r", ch);
      else
	act ("$N already has divine favor.", ch, NULL, victim, TO_CHAR);
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 6 + level;
  af.location = APPLY_HITROLL;
  af.modifier = level / 13.0 + .1;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (victim, &af);

  af.location = APPLY_SAVING_SPELL;
  af.modifier = 0 - level / 8;
  affect_to_char (victim, &af);
  send_to_char ("You feel righteous.\n\r", victim);
  if (ch != victim)
    act ("You grant $N the favor of your god.", ch, NULL, victim, TO_CHAR);
  return;
}

void prayer_calm (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  int mlevel = 0;
  int count = 0;
  int high_level = 0;
  int chance;
  AFFECT_DATA af;

  send_to_char ("Ok.\n\r", ch);
  /* get sum of all mobile levels in the room */
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (vch->position == POS_FIGHTING || vch->fighting != NULL)
	{
	  count++;
	  if (IS_NPC (vch))
	    mlevel += vch->level;
	  else
	    mlevel += vch->level / 2;
	  high_level = UMAX (high_level, vch->level);
	}
    }

  /* compute chance of stopping combat */
  chance = 4 * level - high_level + 2 * count;

  if (IS_IMMORTAL (ch))		/* always works */
    mlevel = 0;

  if (number_range (0, chance) >= mlevel)
    {				/* hard to stop large fights */
      for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	  if (IS_NPC (vch)
	      && (IS_SET (vch->imm_flags, IMM_MAGIC)
		  || IS_SET (vch->act, ACT_UNDEAD)))
	    return;

	  if (IS_AFFECTED (vch, AFF_CALM) || IS_AFFECTED (vch, AFF_BERSERK)
	      || is_affected (vch, skill_lookup ("frenzy")))
	    return;

	  send_to_char ("A wave of calm passes over you.\n\r", vch);

	  if (vch->fighting || vch->position == POS_FIGHTING)
	    stop_fighting (vch, FALSE);


	  af.where = TO_AFFECTS;
	  af.type = sn;
	  af.level = level;
	  af.duration = level / 5;
	  af.location = APPLY_HITROLL;
	  af.permaff = FALSE;
	  af.composition = FALSE;
	  af.comp_name = str_dup ("");
	  if (!IS_NPC (vch))
	    af.modifier = -5;
	  else
	    af.modifier = -2;
	  af.bitvector = AFF_CALM;
	  affect_to_char (vch, &af);

	  af.location = APPLY_DAMROLL;
	  affect_to_char (vch, &af);
	}
    }
}

void prayer_frenzy (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected (victim, sn) || IS_AFFECTED (victim, AFF_BERSERK) || is_affected (ch, gsn_berserk))
    {
      if (victim == ch)
	send_to_char ("You are already in a frenzy.\n\r", ch);
      else
	act ("$N is already in a frenzy.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (is_affected (victim, skill_lookup ("calm")))
    {
      if (victim == ch)
	send_to_char ("Why don't you just relax for a while?\n\r", ch);
      else
	act ("$N doesn't look like $e wants to fight anymore.",
	     ch, NULL, victim, TO_CHAR);
      return;
    }

  if ((IS_GOOD (ch) && !IS_GOOD (victim)) ||
      (IS_NEUTRAL (ch) && !IS_NEUTRAL (victim)) ||
      (IS_EVIL (ch) && !IS_EVIL (victim)))
    {
      act ("Your god doesn't seem to like $N", ch, NULL, victim, TO_CHAR);
      return;
    }

  af.permaff = FALSE;
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 3;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  if (is_affected (victim, skill_lookup ("Falkentynes Fury")))
    af.modifier = level / 15;
  else
    af.modifier = level / 10;
  af.bitvector = 0;

  af.location = APPLY_HITROLL;
  affect_to_char (victim, &af);

  af.location = APPLY_DAMROLL;
  affect_to_char (victim, &af);

  if (is_affected (victim, skill_lookup ("Falkentynes Fury")))
    af.modifier = 5 * (level / 15);
  else
    af.modifier = 10 * (level / 12);
  af.location = APPLY_AC;
  affect_to_char (victim, &af);

  send_to_char ("You are filled with holy wrath!\n\r", victim);
  act ("$n gets a wild look in $s eyes!", victim, NULL, NULL, TO_ROOM);
}

void
prayer_holy_word (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  int bless_num, curse_num, frenzy_num;

  bless_num = skill_lookup ("bless");
  curse_num = skill_lookup ("curse");
  frenzy_num = skill_lookup ("frenzy");

  act ("$n utters a word of divine power!", ch, NULL, NULL, TO_ROOM);
  send_to_char ("You utter a word of divine power.\n\r", ch);

  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;

      if ((IS_GOOD (ch) && IS_GOOD (vch)) ||
	  (IS_EVIL (ch) && IS_EVIL (vch)) ||
	  (IS_NEUTRAL (ch) && IS_NEUTRAL (vch)))
	{
	  send_to_char ("You feel full more powerful.\n\r", vch);
	  prayer_frenzy (frenzy_num, level, ch, (void *) vch, TARGET_CHAR);
	  prayer_bless (bless_num, level, ch, (void *) vch, TARGET_CHAR);
	}

      else if ((IS_GOOD (ch) && IS_EVIL (vch)) ||
	       (IS_EVIL (ch) && IS_GOOD (vch)))
	{
	  if (!is_safe_spell (ch, vch, TRUE))
	    {
	      spell_curse (curse_num, level, ch, (void *) vch, TARGET_CHAR);
	      send_to_char ("You are struck down!\n\r", vch);
	      dam = dice (level, 6);
	      damage_old (ch, vch, dam, sn, DAM_ENERGY, TRUE);
	    }
	}

      else if (IS_NEUTRAL (ch))
	{
	  if (!is_safe_spell (ch, vch, TRUE))
	    {
	      spell_curse (curse_num, level / 2, ch, (void *) vch,
			   TARGET_CHAR);
	      send_to_char ("You are struck down!\n\r", vch);
	      dam = dice (level, 4);
	      damage_old (ch, vch, dam, sn, DAM_ENERGY, TRUE);
	    }
	}
    }

  send_to_char ("You feel drained.\n\r", ch);
  ch->move = 0;
  ch->hit /= 2;
}

void
prayer_remove_curse (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  bool found = FALSE;

  /* do object cases first */
  if (target == TARGET_OBJ)
    {
      obj = (OBJ_DATA *) vo;

      if (IS_OBJ_STAT (obj, ITEM_NODROP) || IS_OBJ_STAT (obj, ITEM_NOREMOVE))
	{
	  if (!IS_OBJ_STAT (obj, ITEM_NOUNCURSE)
	      && !saves_dispel (level + 2, obj->level, 0))
	    {
	      REMOVE_BIT (obj->extra_flags[0], ITEM_NODROP);
	      REMOVE_BIT (obj->extra_flags[0], ITEM_NOREMOVE);
	      act ("$p glows blue.", ch, obj, NULL, TO_ALL);
	      return;
	    }

	  act ("The curse on $p is beyond your power.", ch, obj, NULL,
	       TO_CHAR);
	  return;
	}

      act ("$p does not seem to be cursed.", ch, obj, NULL, TO_CHAR);
      return;
    }

  /* characters */
  victim = (CHAR_DATA *) vo;

  if (check_dispel (level, victim, gsn_curse))
    {
      send_to_char ("You feel better.\n\r", victim);
      act ("$n looks more relaxed.", victim, NULL, NULL, TO_ROOM);
    }

  for (obj = victim->carrying; (obj != NULL && !found);
       obj = obj->next_content)
    {
      if ((IS_OBJ_STAT (obj, ITEM_NODROP)
	   || IS_OBJ_STAT (obj, ITEM_NOREMOVE))
	  && !IS_OBJ_STAT (obj, ITEM_NOUNCURSE))
	{			/* attempt to remove curse */
	  if (!saves_dispel (level, obj->level, 0))
	    {
	      found = TRUE;
	      REMOVE_BIT (obj->extra_flags[0], ITEM_NODROP);
	      REMOVE_BIT (obj->extra_flags[0], ITEM_NOREMOVE);
	      act ("Your $p glows blue.", victim, obj, NULL, TO_CHAR);
	      act ("$n's $p glows blue.", victim, obj, NULL, TO_ROOM);
	    }
	}
    }
}

void prayer_haven (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *pobj;
  ROOM_INDEX_DATA *location = NULL;
  bool found = FALSE;
  int vnum;


  if (ch->position == POS_FIGHTING || ch->fighting != NULL)
    {
      send_to_char
	("The gods do not grant haven to those who are fighting.\n\r", ch);
      return;
    }

  if (!str_cmp (ch->in_room->area->name, "Haven"))
    {
      send_to_char ("The gods have already granted you a haven.\n\r", ch);
      return;
    }

  for (vnum = ROOM_VNUM_HAVEN_START; vnum < ROOM_VNUM_HAVEN_END; vnum++)
    {
      location = get_room_index (vnum);

      if (location == NULL || location->owner[0] != '\0')
	continue;

      found = TRUE;
      break;
    }

  if (!found)
    {
      send_to_char ("Sorry, the gods have no room for you.\n\r", ch);
      return;
    }

  if (IS_CLASS (ch, PC_CLASS_PALADIN) && ch->alignment < 1000)
    {
      send_to_char
	("The gods do not deem you holy enough to deserve a haven.\n\r", ch);
      return;
    }


  pobj = create_object (get_obj_index (OBJ_VNUM_PORTAL), ch->level);

  free_string (pobj->description);
  pobj->description = str_dup ("");

  free_string (pobj->short_descr);
  pobj->short_descr = str_dup ("a portal to haven");

  free_string (pobj->name);
  pobj->name = str_dup ("haven");

  pobj->timer = 5;

  if (ch->level < 45)
    pobj->timer = 1;
  else if (ch->level < 60)
    pobj->timer = 2;
  else if (ch->level < 75)
    pobj->timer = 3;
  else if (ch->level < 90)
    pobj->timer = 4;

  pobj->value[0] = -1;
  pobj->value[2] = GATE_SHOWEXIT;
  pobj->value[3] = vnum;
  pobj->value[4] = PACT_ENTER;

  obj_to_room (pobj, ch->in_room);

  pobj = create_object (get_obj_index (OBJ_VNUM_PORTAL), ch->level);

  free_string (pobj->description);
  pobj->description = str_dup ("");

  free_string (pobj->short_descr);
  pobj->short_descr = str_dup ("a portal from haven");

  free_string (pobj->name);
  pobj->name = str_dup ("haven");

  pobj->value[0] = -1;
  pobj->value[2] = GATE_SHOWEXIT;
  pobj->value[3] = ch->in_room->vnum;
  pobj->value[4] = PACT_EXIT;

  obj_to_room (pobj, location);

  free_string (location->owner);
  location->owner = str_dup (ch->name);

  send_to_char ("The gods grant you a temporary haven.\n\r", ch);
  act ("The gods have granted $n a temporary haven.", ch, NULL, NULL,
       TO_ROOM);
}

void
prayer_heroes_feast (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *gch;

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (is_same_group (gch, ch) && !IS_NPC (gch) && gch->race != PC_RACE_KALIAN)
	{
	  gch->pcdata->condition[COND_HUNGER] = 48;
	  gch->pcdata->condition[COND_THIRST] = 48;
	  gch->pcdata->condition[COND_FULL] = 48;
	  send_to_char
	    ("A sumptuous feast worthy of heroes appears and satiates your appetite.\n\r",
	     gch);
	}
    }

}

void
prayer_group_sanctuary (int sn, int level, CHAR_DATA * ch, void *vo,
			int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;

  for (victim = char_list; victim != NULL; victim = victim->next)
    {
      if (is_same_group (victim, ch) && !IS_NPC (victim))
	{
	  if (IS_AFFECTED (victim, AFF_SANCTUARY))
	    {
	      if (victim == ch)
		send_to_char ("You are already in sanctuary.\n\r", ch);
	      else
		act ("$N is already in sanctuary.", ch, NULL, victim,
		     TO_CHAR);
	      continue;
	    }

	  af.where = TO_AFFECTS;
	  af.type = sn;
	  af.level = level;
	  if (!IS_IMMORTAL (ch))
	    af.duration = level / 10;
	  else
	    af.duration = level;
	  af.location = APPLY_NONE;
	  af.modifier = 0;
	  af.bitvector = AFF_SANCTUARY;
	  af.permaff = FALSE;
	  af.composition = FALSE;
	  af.comp_name = str_dup ("");

	  affect_to_char (victim, &af);
	  act ("$n is surrounded by a white aura.", victim, NULL, NULL,
	       TO_ROOM);
	  send_to_char ("You are surrounded by a white aura.\n\r", victim);
	}
    }
}

void
prayer_mass_levitation (int sn, int level, CHAR_DATA * ch, void *vo,
			int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;

  for (victim = char_list; victim != NULL; victim = victim->next)
    {
      if (is_same_group (victim, ch) && !IS_NPC (victim))
	{
	  if (IS_AFFECTED (victim, AFF_FLYING))
	    {
	      if (victim == ch)
		send_to_char ("You are already airborne.\n\r", ch);
	      else
		act ("$N doesn't need your help to fly.", ch, NULL,
		     victim, TO_CHAR);
	      continue;
	    }
	   if (!IS_NPC(victim) && victim->race == PC_RACE_NIDAE && number_percent() <= 50)
	    {
	      act ("$N is unaffected.\n\r",ch,NULL,victim,TO_CHAR);
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
	  act ("$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM);
	  send_to_char ("Your feet rise off the ground.\n\r", victim);
	}
    }
}

void
prayer_sanctuary (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected (victim, gsn_soul_link))
  {
    if (victim == ch)
      send_to_char("You cannot be in sanctuary if you are sharing a soul.\n\r",ch);
    else
      act("$N cannot be in sanctuary because $N is sharing a soul.",ch,NULL,victim,TO_CHAR);
  }
  
  if (IS_AFFECTED (victim, AFF_SANCTUARY))
    {
      if (victim == ch)
	send_to_char ("You are already in sanctuary.\n\r", ch);
      else
	act ("$N is already in sanctuary.", ch, NULL, victim, TO_CHAR);
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  if (!IS_IMMORTAL (ch) || IS_NPC (ch))
    af.duration = level / 10;
  else
    af.duration = level;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_SANCTUARY;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (victim, &af);
  act ("$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM);
  send_to_char ("You are surrounded by a white aura.\n\r", victim);
  return;
}


void
prayer_continual_light (int sn, int level, CHAR_DATA * ch, void *vo,
			int target)
{
  OBJ_DATA *light;

  if (pray_target_name[0] != '\0')
    {				/* do a glow on some object */
      light = get_obj_carry (ch, pray_target_name);

      if (light == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}

      if (IS_OBJ_STAT (light, ITEM_GLOW))
	{
	  act ("$p is already glowing.", ch, light, NULL, TO_CHAR);
	  return;
	}

      SET_BIT (light->extra_flags[0], ITEM_GLOW);
      act ("$p glows with a white light.", ch, light, NULL, TO_ALL);
      return;
    }

  light = create_object (get_obj_index (OBJ_VNUM_LIGHT_BALL), 0);
  obj_to_room (light, ch->in_room);
  act ("$n twiddles $s thumbs and $p appears.", ch, light, NULL, TO_ROOM);
  act ("You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR);
  return;
}

void
prayer_earthquake (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  send_to_char ("The earth trembles beneath your feet!\n\r", ch);
  act ("$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);

  for (vch = char_list; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next;

      if (vch->in_room == NULL)
	continue;


      if (vch->in_room == ch->in_room)
	{
	  if (is_same_group (ch, vch))
	    continue;

	  if (vch != ch && !is_safe_spell (ch, vch, TRUE))
	    {
	      if (IS_AFFECTED (vch, AFF_FLYING) || (vch->mount != NULL &&
						    IS_AFFECTED (vch->
								 mount,
								 AFF_FLYING)))
		damage_old (ch, vch, 0, sn, DAM_BASH, TRUE);
	      else
		damage_old (ch, vch, level + dice (2, 8), sn, DAM_BASH, TRUE);
	    }
	  continue;
	}

      if (vch->in_room->area == ch->in_room->area)
	send_to_char ("The earth trembles and shivers.\n\r", vch);
    }

  return;
}

void
prayer_true_sight (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED (victim, AFF_DETECT_INVIS))
    {
      if (victim == ch)
	send_to_char ("You already possess true sight.\n\r", ch);
      else
	act ("$N already possesses true sight.", ch, NULL, victim, TO_CHAR);
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

//Iblis - new prayer to allow a player to recover some of their lost xp, with the cleric also getting
//a share
void prayer_holy_ceremony (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int xp=0;
  if (IS_NPC (victim))
    {
      send_to_char
	("This prayer is only useful when prayed on players.\r\n", ch);
      return;
    }

  if (ch == victim)
    {
      send_to_char ("This prayer can not be cast on yourself.\r\n", ch);
      return;
    }
  if (victim->pcdata->last_death_counter)
    {
      act ("Your holy ceremony reunites $N with $S wandering spirit.", ch,
	   NULL, victim, TO_CHAR);
      act ("$n's holy ceremony reunites you with your wandering spirit.",
	   ch, NULL, victim, TO_VICT);
      xp = victim->pcdata->xp_last_death * .25;
      if (xp > level_cost (ch->level) * .40)
        xp = level_cost (ch->level) * .40;
      low_gain_exp (ch, xp);
      low_gain_exp (victim, victim->pcdata->xp_last_death * .5);
      victim->pcdata->xp_last_death = 0;
      victim->pcdata->last_death_counter = 0;
    }
  else
    act ("You can no longer help $N with this prayer", ch, NULL, victim,
	 TO_CHAR);
  return;
}
