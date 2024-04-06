#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include <ctype.h>
#include "chant.h"
#include "obj_trig.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"


char *chant_target_name;
OBJ_DATA *get_obj_vnum_room args((ROOM_INDEX_DATA* rid, int vnum));
OBJ_DATA *get_obj_vnum_container args((OBJ_DATA* rid, int vnum));
long int low_gain_exp args ((CHAR_DATA * ch, long int gain));
void update_aggressor args ((CHAR_DATA * ch, CHAR_DATA * victim));
void do_echo args((CHAR_DATA * ch, char *argument));
void do_at args((CHAR_DATA * ch, char *argument));
void do_look args((CHAR_DATA * ch, char *argument));
void do_follow args((CHAR_DATA * ch, char *argument));
void do_group args((CHAR_DATA * ch, char *argument));
OBJ_DATA *get_obj_vnum_char args((CHAR_DATA* ch, int vnum));	
void spell_invis args((int sn, int level, CHAR_DATA * ch, void *vo, int target));	
void delete_player_corpse args((OBJ_DATA * pObj));
void save_player_corpse args((OBJ_DATA * pObj));
bool can_move_char args ((CHAR_DATA * ch, ROOM_INDEX_DATA * to_room, bool follow, bool show_messages));
bool check_nonpk args ((CHAR_DATA * ch, CHAR_DATA * victim));
void do_stand args((CHAR_DATA * ch, char *argument));
extern bool crit_strike_possible;
extern bool ignore_save;
extern struct clan_type clan_table[MAX_CLAN];

long int level_cost args ((int level));


bool check_torture args ((CHAR_DATA *ch));
	

extern bool nogate;
extern bool battle_royale;
	
	

void say_chant (CHAR_DATA * ch, int sn)
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
    char *New;
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
    {"sen", "gharnu"},
    {"tect", "infra"},
    {"tri", "cula"},
    {"ven", "nofo"},
    {"a", "i"},
    {"b", "r"},
    {"c", "z"},
    {"d", "q"},
    {"e", "h"},
    {"f", "y"},
    {"g", "p"},
    {"h", "g"},
    {"i", "x"},
    {"j", "o"},
    {"k", "f"},
    {"l", "w"},
    {"m", "n"},
    {"n", "t"},
    {"o", "a"},
    {"p", "m"},
    {"q", "d"},
    {"r", "u"},
    {"s", "l"},
    {"t", "c"},
    {"u", "e"},
    {"v", "k"},
    {"w", "b"}, {"x", "s"}, {"y", "j"}, {"z", "v"}, {"", ""}
  };
  buf[0] = '\0';
  for (pName = skill_table[sn].name; *pName != '\0'; pName += length)
    {
      for (iSyl = 0; (length = strlen (syl_table[iSyl].old)) != 0; iSyl++)
	{
	  if (!str_prefix (syl_table[iSyl].old, pName))
	    {
	      strcat (buf, syl_table[iSyl].New);
	      break;
	    }
	}
      if (length == 0)
	length = 1;
    }
  sprintf (buf2, "$n utters the dark chant, '%s'.", buf);
  sprintf (buf, "$n utters the dark chant, '%s'.", skill_table[sn].name);
  for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
    {
      if (rch != ch)
	act ((IS_CLASS (rch, PC_CLASS_REAVER)|| IS_CLASS (rch, PC_CLASS_NECROMANCER)) ? buf : buf2, ch, NULL, rch, TO_VICT);
    }
  if (is_dueling (ch))
    {
      rid = get_room_index (ROOM_VNUM_SPECTATOR);
      for (rch = rid->people; rch; rch = rch->next_in_room)
	act (IS_CLASS (rch, PC_CLASS_REAVER) ? buf : buf2, ch, NULL, rch, TO_VICT);
    }
  return;
}

void do_chants (CHAR_DATA * ch, char *argument)
{
  char chant_list[MAX_LEVEL][MAX_STRING_LENGTH];
  char chant_columns[MAX_LEVEL];
  int sn, lev, mana;
  bool found = FALSE;
  char buf[MAX_STRING_LENGTH];
  if (IS_NPC (ch))
    return;
  if (ch->level > LEVEL_HERO)
    {
      for (lev = 0; lev < MAX_LEVEL; lev++)
	{
	  chant_columns[lev] = 0;
	  chant_list[lev][0] = '\0';
	}
      for (sn = 0; sn < MAX_SKILL; sn++)
	{
	  if (skill_table[sn].name == NULL)
	    break;
	  if (level_for_skill(ch,sn) < MAX_LEVEL && ch->pcdata->learned[sn] > 0
			  && skill_table[sn].type == SKILL_CHANT)
	    {
	      found = TRUE;
	      lev = level_for_skill (ch, sn);
	      if (ch->level < lev)
		sprintf (buf, "%-18s  n/a      ", skill_table[sn].name);

	      else
		{
	           if (sn == skill_lookup("seizure"))
		     mana = ch->level/2;
		   else if (sn == skill_lookup("bone craft"))
		     mana = ch->max_mana/2;
		   else if (sn == skill_lookup("drain aura"))
		     mana = skill_table[sn].min_mana;
		   else     
		   {
		    mana =
		      UMAX (skill_table[sn].min_mana,
		  	  100 / (2 + ch->level - lev));
		   }
		  sprintf (buf, "%-18s  %3d mana  ",
			   skill_table[sn].name, mana);
		}
	      if (chant_list[lev][0] == '\0')
		sprintf (chant_list[lev], "\n\rLevel %2d: %s", lev, buf);

	      else
		/* append */
		{
		  if (++chant_columns[lev] % 2 == 0)
		    strcat (chant_list[lev], "\n\r          ");
		  strcat (chant_list[lev], buf);
		}
	    }
	}
    }

  else
    {
      for (lev = 0; lev < LEVEL_HERO; lev++)
	{
	  chant_columns[lev] = 0;
	  chant_list[lev][0] = '\0';
	}
      for (sn = 0; sn < MAX_SKILL; sn++)
	{
	  if (skill_table[sn].name == NULL)
	    break;
	  if (level_for_skill(ch,sn) < LEVEL_HERO && skill_table[sn].type == SKILL_CHANT
			  && ch->pcdata->learned[sn] > 0)
	    {
	      found = TRUE;
	      lev = level_for_skill (ch, sn);
	      if (ch->level < lev)
		sprintf (buf, "%-18s  n/a      ", skill_table[sn].name);

	      else
		{
		  if (sn == skill_lookup("seizure"))
	            mana = ch->level/2;
	          else if (sn == skill_lookup("bone craft"))
	            mana = ch->max_mana/2;
				     
		  else
		  {
		    mana =
		       UMAX (skill_table[sn].min_mana,
		 	  100 / (2 + ch->level - lev));
		  }
		     sprintf (buf, "%-18s  %3d mana  ",
		  	   skill_table[sn].name, mana);
		   
			 
		}
	      if (chant_list[lev][0] == '\0')
		sprintf (chant_list[lev], "\n\rLevel %2d: %s", lev, buf);

	      else
		/* append */
		{
		  if (++chant_columns[lev] % 2 == 0)
		    strcat (chant_list[lev], "\n\r          ");
		  strcat (chant_list[lev], buf);
		}
	    }
	}
    }

  /* return results */
  if (!found)
    {
      send_to_char ("You know no chants.\n\r", ch);
      return;
    }
  if (ch->level > LEVEL_HERO)
    {
      for (lev = 0; lev < MAX_LEVEL; lev++)
	if (chant_list[lev][0] != '\0')
	  send_to_char (chant_list[lev], ch);
    }

  else
    {
      for (lev = 0; lev < LEVEL_HERO; lev++)
	if (chant_list[lev][0] != '\0')
	  send_to_char (chant_list[lev], ch);
    }
  send_to_char ("\n\r", ch);
}

void do_chant (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  void *vo;
  int mana;
  int sn;
  int target;
  bool wait = TRUE;
  /*  if (IS_NPC(ch) && ch->desc == NULL)
     return;
   */
  if (!has_skills (ch, SKILL_CHANT) && !IS_NPC (ch))
    {
      send_to_char ("You lack the ability to chant.\n\r", ch);
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
  //05-10-03 Iblis - This attempts to rememdy the problem of the actual blade/burst spells store in the blade
  // vs what blade/burst spells SHOULD be stored
  if (IS_CLASS (ch, PC_CLASS_REAVER))
    update_sword (ch, ch->sword);
  chant_target_name = one_argument (argument, arg1);
  one_argument (chant_target_name, arg2);
  if (arg1[0] == '\0')
    {
      send_to_char ("Chant what? where?\n\r", ch);
      return;
    }
  if ((sn = find_spell (ch, arg1)) < 0
      || (!IS_NPC (ch) && get_skill(ch,sn) < 1))
    {
      send_to_char ("You don't know any chants of that name.\n\r", ch);
      return;
    }

  if (IS_SET(ch->act2,ACT_FAMILIAR) && get_skill(ch,sn) < 1)
      return;
  
  
  if (skill_table[sn].type != SKILL_CHANT)
    {
      send_to_char ("You don't know any chants of that name.\n\r", ch);
      return;
    }
  if (ch->position < skill_table[sn].minimum_position
      || (ch->fighting != NULL
	  && skill_table[sn].minimum_position > POS_FIGHTING))
    {
      send_to_char ("You are not prepared to chant.\n\r", ch);
      return;
    }

  if (is_affected (ch, gsn_fear))
  {
	  send_to_char ("You are too scared to chant that right now.\n\r", ch);
	  return;
  }
  
  if (is_affected (ch, skill_lookup ("Quiet Mountain Lake"))
      && (skill_table[sn].target == TAR_CHAR_OFFENSIVE
	  || skill_table[sn].target == TAR_AREA_OFF
	  || skill_table[sn].target == TAR_OBJ_CHAR_OFF))
    {
      send_to_char ("You are too peaceful to chant that right now.\n\r", ch);
      return;
    }
  if (sn == skill_lookup("seizure"))
	  mana = ch->level/2;
  else if (sn == skill_lookup("bone craft"))
	  mana = ch->max_mana/2;
  else if (sn == skill_lookup("drain aura"))
	  mana = skill_table[sn].min_mana;
  else
  {
    if (ch->level + 2 == level_for_skill (ch, sn))
      mana = 50;

    else
      mana =
        UMAX (skill_table[sn].min_mana,
  	    100 / (2 + ch->level - level_for_skill (ch, sn)));
  }
  if (is_affected (ch, gsn_mirror))
    {
      act ("The mirror arround you is destroyed by your outgoing chant.",
	   ch, NULL, NULL, TO_CHAR);
      act ("The mirror around $n is destroyed by $s outgoing chant.",
	   ch, NULL, NULL, TO_ROOM);
      affect_strip (ch, gsn_mirror);
    }
/*
  if (IS_AFFECTED (ch, AFF_INVISIBLE) )
    {
      affect_strip (ch, gsn_astral_body);	    
      affect_strip (ch, gsn_invis);
      affect_strip (ch, gsn_mass_invis);
      affect_strip (ch, gsn_heavenly_cloak);
      REMOVE_BIT (ch->affected_by, AFF_INVISIBLE);
      act ("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
    }
*/
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
      bug ("Do_chant: bad target for sn %d.", sn);
      return;
    case TAR_IGNORE:
      break;
    case TAR_CHAR_OFFENSIVE:
      if (arg2[0] == '\0')
	{
	  if ((victim = ch->fighting) == NULL)
	    {
	      send_to_char ("Who would you like to chant this to?\n\r", ch);
	      return;
	    }
	  if (check_shopkeeper_attack (ch, victim))
	    return;
	   
	}

      else
	{
	  if ((victim = get_char_room (ch, chant_target_name)) == NULL)
	    {
	      send_to_char ("They aren't here.\n\r", ch);
	      return;
	    }
	}
      if (!IS_NPC (ch))
	{
	  if (is_safe (ch, victim) && victim != ch)
	    {

//            send_to_char ("Not on that target.\n\r", ch);
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
	  if ((victim = get_char_room (ch, chant_target_name)) == NULL)
	    {
	      send_to_char ("They aren't here.\n\r", ch);
	      return;
	    }
	}
      vo = (void *) victim;
      target = TARGET_CHAR;
      break;
    case TAR_CHAR_SELF:
      if (arg2[0] != '\0' && !is_name (chant_target_name, ch->name))
	{
	  send_to_char ("You cannot chant this to another.\n\r", ch);
	  return;
	}
      vo = (void *) ch;
      target = TARGET_CHAR;
      break;
    case TAR_OBJ_INV:
      if (arg2[0] == '\0')
	{
	  send_to_char ("What should this be chanted to?\n\r", ch);
	  return;
	}
      if ((obj = get_obj_carry (ch, chant_target_name)) == NULL)
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
	      send_to_char ("Chant to whom or what?\n\r", ch);
	      return;
	    }
	  target = TARGET_CHAR;
	}

      else if ((victim = get_char_room (ch, chant_target_name)) != NULL)
	{
	  target = TARGET_CHAR;
	}
      if (target == TARGET_CHAR)
	{			/* check the sanity of the attack */
	  if (is_safe_spell (ch, victim, FALSE) && victim != ch)
	    {

//            send_to_char ("Not on that target.\n\r", ch);
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

      else if ((obj = get_obj_here (ch, chant_target_name)) != NULL)
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
      
      else if ((victim = get_char_room (ch, chant_target_name)) != NULL)
	{
	  vo = (void *) victim;
	  target = TARGET_CHAR;
	}
      
      else if ((obj = get_obj_carry (ch, chant_target_name)) != NULL)
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
      case TAR_OBJ_PENTAGRAM:
	{
	  wait = FALSE;
	  obj = get_obj_vnum_room(ch->in_room,OBJ_VNUM_PENTAGRAM);
	  if (obj)
	    obj = get_obj_list (ch, chant_target_name, obj->contains);
	  else
	  {
  	    send_to_char ("You see no pentagram here.\n\r",ch);
	    return;
	  }
		    
	  if (obj)
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
    case TAR_CHAR_HEXAGRAM:
      { 
	wait = FALSE;
	obj = get_obj_vnum_room(ch->in_room,OBJ_VNUM_HEXAGRAM);
	if (!obj)
	  {
	    send_to_char ("A hexagram is required for this chant.\n\r",ch);
	    return;
	  }
	if (arg2[0] == '\0')
	   victim = ch;
	else if (!(victim = get_char_room (ch, chant_target_name)))
	  {
	    send_to_char ("You do not see them here.\n\r",ch);
	    return;
	  }
	if (victim->on != obj)
	  {
	    send_to_char ("That target is not standing in the hexagram.\n\r",ch);
	    return;
	  }
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;
      }
    case TAR_ROOM_SEPTAGRAM:
      {
	if (sn != skill_lookup("bone hail"))
		wait = FALSE;
        obj = get_obj_vnum_room(ch->in_room,OBJ_VNUM_SEPTAGRAM);
        if (!obj)
          {
            send_to_char ("A septagram is required for this chant.\n\r",ch);
            return;
          }
	target = TARGET_NONE;
	break;
      }
    case TAR_CHAR_WORLD:
      if (arg2[0] == '\0')
        {
          send_to_char ("Who would you like to chant this on?\n\r",ch);
	  return;
        }

      else
        {
          if ((victim = get_char_world(ch, chant_target_name)) == NULL)
            {
              send_to_char ("They aren't here.\n\r", ch);
              return;
            }
        }
      vo = (void *) victim;
      target = TARGET_CHAR;
      break;
    case TAR_STRING:
      if (chant_target_name[0] == '\0')
        vo = NULL;
      else vo = (void *) chant_target_name;
      target = TARGET_NONE;
      break;	      
    }
  if (!IS_NPC (ch) && ch->mana < mana)
    {
      send_to_char ("You don't have enough mana.\n\r", ch);
      return;
    }
  if (!IS_IMMORTAL (ch) && wait)
    WAIT_STATE (ch, skill_table[sn].beats);
  if (number_percent () > get_skill (ch, sn))
    {
      send_to_char ("You lost your concentration.\n\r", ch);
      check_improve (ch, sn, FALSE, 1);
      ch->mana -= mana / 2;
    }

  else
    {
      char buf[20];
      CHAR_DATA *perp=NULL;
      AFFECT_DATA *af=NULL;
      ch->mana -= mana;

      //5-11-03 Iblis - Make reavers say something when they chant
      if (!((IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_FAKIE)))
      {
        say_chant (ch, sn);
      }
      if (IS_NPC (ch) || mana_using_Class (ch))
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
			  && ch->in_room == perp->in_room && perp->fighting == ch)
	    {
	      //simulated 60% success rate
	      if (number_percent() >= 60)
		{
		  send_to_char("You lose your concentration.\n\r",perp);
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

	/* Class has spells */
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
			  && ch->in_room == perp->in_room && perp->fighting == ch)
            {
              //simulated 60% success rate
              if (number_percent() >= 60)
                {
                  send_to_char("You lose your concentration.\n\r",perp);
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

      if (IS_CLASS(ch,PC_CLASS_NECROMANCER))
	change_alignment(ch,0-skill_table[sn].skill_level[10]/5.0);
      sprintf(buf, "%d", sn);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_USES_SPELL, NULL, victim, OT_TYPE_CHANT, buf);
      
      check_improve (ch, sn, TRUE, 1);
    }
  if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
       || (skill_table[sn].target == TAR_OBJ_CHAR_OFF
	   && target == TARGET_CHAR)) && victim != ch && victim->master != ch)
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

//      set_fighting (victim, ch);
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
  if (skill_table[sn].target == TAR_OBJ_PENTAGRAM 
		  || skill_table[sn].target == TAR_STRING
		  || sn == skill_lookup("death vision"))
    {
      obj = get_obj_vnum_room(ch->in_room,OBJ_VNUM_PENTAGRAM);	
      if (obj != NULL)
	{
	  obj->value[5]--;
	  if (obj->value[5] <= 0 && obj->in_room)
	    {
	      if (obj->in_room->people)
		{
		  act("The blood that the pentagram was drawn with seeps into the earth.",
		      obj->in_room->people, NULL, NULL, TO_ALL);
		}
	      obj_from_room (obj);
	      extract_obj (obj);
	    }
	}
    }
  if (skill_table[sn].target == TAR_CHAR_HEXAGRAM)
    {
      obj = get_obj_vnum_room(ch->in_room,OBJ_VNUM_HEXAGRAM);
      if (obj != NULL)
        {
          obj->value[5]--;
          if (obj->value[5] <= 0 && obj->in_room)
            {
              if (obj->in_room->people)
                {
                  act("The blood that the hexagram was drawn with seeps into the earth.",
                      obj->in_room->people, NULL, NULL, TO_ALL);
                }
              obj_from_room (obj);
              extract_obj (obj);
            }
        }
    }
  if (skill_table[sn].target == TAR_ROOM_SEPTAGRAM && sn != skill_lookup("bone hail"))
    {
      obj = get_obj_vnum_room(ch->in_room,OBJ_VNUM_SEPTAGRAM);
      if (obj != NULL)
        {
	  if (sn != skill_lookup("bone hail"))
	          obj->value[5]--;
          if (obj->value[5] <= 0 && obj->in_room)
            {
              if (obj->in_room->people)
                {
                  act("The blood that the septagram was drawn with seeps into the earth.",
                      obj->in_room->people, NULL, NULL, TO_ALL);
                }
              obj_from_room (obj);
              extract_obj (obj);
            }
        }
    }


  
  return;
}

void
chant_shocking_blade (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *sword;
  if (!IS_SET (ch->in_room->room_flags, ROOM_SHRINE))
    {
      send_to_char
	("This chant may only be uttered at a dark shrine.\n\r", ch);
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
      send_to_char ("You do not possess your blade.\n\r", ch);
      return;
    }
  if (sword->bs_charges[BLADE_SPELL_SHOCKING] ==
      sword->bs_capacity[BLADE_SPELL_SHOCKING])
    {
      send_to_char
	("Your blade cannot hold any more charges of shocking blade.\n\r",
	 ch);
      return;
    }
  ch->sword->bs_charges[BLADE_SPELL_SHOCKING]++;
  update_sword (ch, ch->sword);

  //act("$n utters the dark chant 'ulikwa shakrah'", ch, sword, NULL, TO_ROOM);
  act ("$n's buzzes with electric energy for a moment.", ch, sword,
       NULL, TO_ROOM);
  send_to_char
    ("`nElectrical currents pulse violently through your sword awaiting invocation.``\n\r",
     ch);
  return;
}

void
chant_frost_blade (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *sword;
  if (!IS_SET (ch->in_room->room_flags, ROOM_SHRINE))
    {
      send_to_char
	("This chant may only be uttered at a dark shrine.\n\r", ch);
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
      send_to_char ("You do not possess your blade.\n\r", ch);
      return;
    }
  if (sword->bs_charges[BLADE_SPELL_FROST] ==
      sword->bs_capacity[BLADE_SPELL_FROST])
    {
      send_to_char
	("Your blade cannot hold any more charges of frost blade.\n\r", ch);
      return;
    }
  ch->sword->bs_charges[BLADE_SPELL_FROST]++;
  update_sword (ch, ch->sword);

  //act("$n utters the dark chant 'ulikwa faliin'", ch, sword, NULL, TO_ROOM);
  act ("$n's blade frosts over briefly.", ch, sword, NULL, TO_ROOM);
  send_to_char
    ("`nA bitter frost coats your sword, awaiting invocation.``\n\r", ch);
  return;
}

void
chant_flaming_blade (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *sword;
  if (!IS_SET (ch->in_room->room_flags, ROOM_SHRINE))
    {
      send_to_char
	("This chant may only be uttered at a dark shrine.\n\r", ch);
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
      send_to_char ("You do not possess your blade.\n\r", ch);
      return;
    }
  if (sword->bs_charges[BLADE_SPELL_FLAMING] ==
      sword->bs_capacity[BLADE_SPELL_FLAMING])
    {
      send_to_char
	("Your blade cannot hold any more charges of flaming blade.\n\r", ch);
      return;
    }
  ch->sword->bs_charges[BLADE_SPELL_FLAMING]++;
  update_sword (ch, ch->sword);

  // act("$n utters the dark chant 'ulikwa chadros'", ch, sword, NULL, TO_ROOM);
  act ("$n's blade glows with a red flame, then fades.", ch, sword,
       NULL, TO_ROOM);
  send_to_char
    ("`iA scorching flame envelopes your sword, awaiting invocation.``\n\r",
     ch);
  return;
}

void
chant_chaotic_blade (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *sword;
  if (!IS_SET (ch->in_room->room_flags, ROOM_SHRINE))
    {
      send_to_char
	("This chant may only be uttered at a dark shrine.\n\r", ch);
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
      send_to_char ("You do not possess your blade.\n\r", ch);
      return;
    }
  if (sword->bs_charges[BLADE_SPELL_CHAOTIC] ==
      sword->bs_capacity[BLADE_SPELL_CHAOTIC])
    {
      send_to_char
	("Your blade cannot hold any more charges of chaotic blade.\n\r", ch);
      return;
    }
  ch->sword->bs_charges[BLADE_SPELL_CHAOTIC]++;
  update_sword (ch, ch->sword);

  //act("$n utters the dark chant 'ulikwa siiradhan'", ch, sword, NULL, TO_ROOM);
  act ("$n's blades edge glows with sharpened intensity, then fades.", ch,
       sword, NULL, TO_ROOM);
  send_to_char
    ("`hYour blades edge glows with a chaotic fury as it awaits invocation``\n\r",
     ch);
  return;
}

void
chant_venomous_blade (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *sword;
  if (!IS_SET (ch->in_room->room_flags, ROOM_SHRINE))
    {
      send_to_char
	("This chant may only be uttered at a dark shrine.\n\r", ch);
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
      send_to_char ("You do not possess your blade.\n\r", ch);
      return;
    }
  if (sword->bs_charges[BLADE_SPELL_VENEMOUS] ==
      sword->bs_capacity[BLADE_SPELL_VENEMOUS])
    {
      send_to_char
	("Your blade cannot hold any more charges of venomous blade.\n\r",
	 ch);
      return;
    }
  ch->sword->bs_charges[BLADE_SPELL_VENEMOUS]++;
  update_sword (ch, ch->sword);

  //act("$n utters the dark chant 'ulikwa dusabril'", ch, sword, NULL, TO_ROOM);
  act ("$n's blade glows with a poisonous aura.", ch, sword, NULL, TO_ROOM);
  send_to_char
    ("`fA deadly venom is stored in your blade, awaiting invocation.``\n\r",
     ch);
  return;
}

void
chant_vampiric_blade (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *sword;
  if (!IS_SET (ch->in_room->room_flags, ROOM_SHRINE))
    {
      send_to_char
	("This chant may only be uttered at a dark shrine.\n\r", ch);
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
      send_to_char ("You do not possess your blade.\n\r", ch);
      return;
    }
  if (sword->bs_charges[BLADE_SPELL_VAMPIRIC] ==
      sword->bs_capacity[BLADE_SPELL_VAMPIRIC])
    {
      send_to_char
	("Your blade cannot hold any more charges of vampiric blade.\n\r",
	 ch);
      return;
    }
  ch->sword->bs_charges[BLADE_SPELL_VAMPIRIC]++;
  update_sword (ch, ch->sword);

  // act("$n utters the dark chant 'ulikwa vhellos'", ch, sword, NULL, TO_ROOM);
  act ("$n's blade teems with bloodthirst.", ch, sword, NULL, TO_ROOM);
  send_to_char
    ("`aYour blade gets bloodthirsty and awaits invocation.``\n\r", ch);
  return;
}

void
chant_apathy_blade (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *sword;
  if (!IS_SET (ch->in_room->room_flags, ROOM_SHRINE))
    {
      send_to_char
        ("This chant may only be uttered at a dark shrine.\n\r", ch);
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
      send_to_char ("You do not possess your blade.\n\r", ch);
      return;
    }
  if (sword->bs_charges[BLADE_SPELL_APATHY] ==
      sword->bs_capacity[BLADE_SPELL_APATHY])
    {
      send_to_char
        ("Your blade cannot hold any more charges of apathy blade.\n\r",
         ch);
      return;
    }
  ch->sword->bs_charges[BLADE_SPELL_APATHY]++;
  update_sword (ch, ch->sword);

  // act("$n utters the dark chant 'ulikwa vhellos'", ch, sword, NULL, TO_ROOM);
  act ("$n's blade seethes with darkness.", ch, sword, NULL, TO_ROOM);
  send_to_char
    ("`aYour blade seethes with darkness, awaiting invocation.``\n\r", ch);
  return;
}

void
chant_summon_sword (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *sword;
  int i;
  if (get_eq_char (ch, WEAR_WIELD_L) != NULL
      || get_eq_char (ch, WEAR_WIELD_R) != NULL)
    {
      send_to_char ("You already possess your sword!\n\r", ch);
      return;
    }
  if (!IS_SET (ch->in_room->room_flags, ROOM_SHRINE))
    {
      send_to_char
	("This chant may only be uttered at a dark shrine.\n\r", ch);
      return;
    }
  if (ch->alignment > 0)
    {
      send_to_char
	("You may not summon your blade until you return to the path of evil.\n\r",
	 ch);
      return;
    }
  if (get_eq_char (ch, WEAR_SHIELD) != NULL
      && get_eq_char (ch, WEAR_HOLD) != NULL)
    {
      send_to_char ("You can't summon a sword with your hands full.\n\r", ch);
      return;
    }
  if (ch->sword == NULL)
    {
      send_to_char
	("Your sword has gone missing.  Contact an immortal.\n\r", ch);
      return;
    }
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

  //act("$n utters the dark chant 'vhallen raghosh'", ch, sword, NULL, TO_ROOM);
  act ("$n summons $s reaver blade.", ch, sword, NULL, TO_ROOM);
  send_to_char ("You succeed in summoning your sword.\n\r", ch);
  return;
}

void
chant_lightning_burst (int sn, int level, CHAR_DATA * ch, void *vo,
		       int target)
{
  OBJ_DATA *sword;
  if (!IS_SET (ch->in_room->room_flags, ROOM_SHRINE))
    {
      send_to_char
	("This chant may only be uttered at a dark shrine.\n\r", ch);
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
      send_to_char ("You do not possess your blade.\n\r", ch);
      return;
    }
  if (sword->bb_charges[BURST_LIGHTNING] ==
      sword->bb_capacity[BURST_LIGHTNING])
    {
      send_to_char
	("Your blade cannot hold any more charges of lightning burst.\n\r",
	 ch);
      return;
    }
  ch->sword->bb_charges[BURST_LIGHTNING]++;
  update_sword (ch, ch->sword);

  // act("$n utters the dark chant 'gharnu viergha'", ch, sword, NULL, TO_ROOM);
  act
    ("$n's blade becomes momentarily electrified with a pulse of lightning.",
     ch, sword, NULL, TO_ROOM);
  send_to_char ("`nYour blade yearns for an electrical maelstrom.``\n\r", ch);
  return;
}

void chant_ice_burst (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *sword;
  if (!IS_SET (ch->in_room->room_flags, ROOM_SHRINE))
    {
      send_to_char
	("This chant may only be uttered at a dark shrine.\n\r", ch);
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
      send_to_char ("You do not possess your blade.\n\r", ch);
      return;
    }
  if (sword->bb_charges[BURST_ICE] == sword->bb_capacity[BURST_ICE])
    {
      send_to_char
	("Your blade cannot hold any more charges of ice burst.\n\r", ch);
      return;
    }
  ch->sword->bb_charges[BURST_ICE]++;
  update_sword (ch, ch->sword);

  //act("$n utters the dark chant 'gharnu shimihna'", ch, sword, NULL, TO_ROOM);
  act ("$n's blade is embittered with a harsh frost.", ch, sword,
       NULL, TO_ROOM);
  send_to_char ("`nYour blade yearns for a devilish blizzard.``\n\r", ch);
  return;
}

void
chant_fire_burst (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *sword;
  if (!IS_SET (ch->in_room->room_flags, ROOM_SHRINE))
    {
      send_to_char
	("This chant may only be uttered at a dark shrine.\n\r", ch);
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
      send_to_char ("You do not possess your blade.\n\r", ch);
      return;
    }
  if (sword->bb_charges[BURST_FIRE] == sword->bb_capacity[BURST_FIRE])
    {
      send_to_char
	("Your blade cannot hold any more charges of fire burst.\n\r", ch);
      return;
    }
  ch->sword->bb_charges[BURST_FIRE]++;
  update_sword (ch, ch->sword);

  //act("$n utters the dark chant 'gharnu fharga'", ch, sword, NULL, TO_ROOM);
  act ("$n's blade becomes wreathed with searing flames, then fades.",
       ch, sword, NULL, TO_ROOM);
  send_to_char ("`iYour blade yearns for a scorching firestorm.``\n\r", ch);
  return;
}

void
chant_unholy_burst (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *sword;
  if (!IS_SET (ch->in_room->room_flags, ROOM_SHRINE))
    {
      send_to_char
	("This chant may only be uttered at a dark shrine.\n\r", ch);
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
      send_to_char ("You do not possess your blade.\n\r", ch);
      return;
    }
  if (sword->bb_charges[BURST_UNHOLY] == sword->bb_capacity[BURST_UNHOLY])
    {
      send_to_char
	("Your blade cannot hold any more charges of unholy burst.\n\r", ch);
      return;
    }
  ch->sword->bb_charges[BURST_UNHOLY]++;
  update_sword (ch, ch->sword);

  //act("$n utters the dark chant 'gharnu ungholvv'", ch, sword, NULL, TO_ROOM);
  act ("$n's blade gives off a disturbing vibe of discontent.", ch,
       sword, NULL, TO_ROOM);
  send_to_char ("`aYour blade yearns for brutal war.``\n\r", ch);
  return;
}

void
chant_hellscape_burst (int sn, int level, CHAR_DATA * ch, void *vo,
		       int target)
{
  OBJ_DATA *sword;
  if (!IS_SET (ch->in_room->room_flags, ROOM_SHRINE))
    {
      send_to_char
	("This chant may only be uttered at a dark shrine.\n\r", ch);
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
      send_to_char ("You do not possess your blade.\n\r", ch);
      return;
    }
  if (sword->bb_charges[BURST_HELLSCAPE] ==
      sword->bb_capacity[BURST_HELLSCAPE])
    {
      send_to_char
	("Your blade cannot hold any more charges of hellscape.\n\r", ch);
      return;
    }
  ch->sword->bb_charges[BURST_HELLSCAPE]++;
  update_sword (ch, ch->sword);

  //act("$n utters the dark chant 'gharnu utumo-dhar'", ch, sword, NULL, TO_ROOM);
  act ("Hell's choir sings an odeous dirge to $n.", ch, sword, NULL, TO_ROOM);
  send_to_char
    ("`aYour blade becomes enthralled with demonic pleasures.``\n\r", ch);
  return;
}

// Iblis 1/1/04 Necromancer Stuff
void chant_death_palm (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, hpgain;
  dam = dice (1,30) + 110;
  hpgain = 15 + dice(1,10);
  if (dam < hpgain)
    hpgain = dam;
  if (saves_spell (level, victim, DAM_NEGATIVE))
  {
    dam /= 2;
    hpgain /= 2;
  }
  send_to_char("Fanged mouths appear in the palm of your hand.\n\r",ch);
  act("You press your open hand against $N's face and life streams through your veins.",ch,NULL,victim,TO_CHAR);
  act("A muffled scream is heard as $n grabs $N's head.",ch,NULL,victim,TO_NOTVICT);
  act("Tiny mouths on the inside of $n's hand sucks your life away as $e grabs your throat.",ch,NULL,victim,TO_VICT);
  if (damage_old (ch, victim, dam, sn, DAM_NEGATIVE, FALSE))
    ch->hit += hpgain;
  return;
}

void chant_decay (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  act("$p is covered by an inky membrane.",ch,obj,NULL,TO_ALL);
  obj->extra_flags[0] |= ITEM_ROT_DEATH;
}
  
void chant_knowledge_of_the_dead (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA* fch;
  for (fch = ch->in_room->people;fch != NULL;fch = fch->next_in_room)
    {
      spell_identify(sn,level,fch,vo,target);
    }
}

void chant_vampirism (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA af;
  if (obj->item_type != ITEM_WEAPON)
    {
      send_to_char("You may only chant this on weapons.\n\r",ch);
      return;
    }
  if (IS_WEAPON_STAT (obj, WEAPON_VAMPIRIC))
  {
    send_to_char("That weapon is already vampiric.\n\r",ch);
    return;
  }
  act("$p is enveloped in a dark red smoke.",ch,obj,NULL,TO_ALL);
  af.where = TO_WEAPON;
  af.level = ch->level;
  af.duration = 20;
  af.location = 0;
  af.modifier = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  af.type = 1;
  af.bitvector = WEAPON_VAMPIRIC;
  affect_to_obj (obj, &af);
}

void chant_exoskeleton (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  if (is_affected (ch, sn))
    {
      if (victim == ch)
        send_to_char ("Your skin is already made of bone.\n\r", ch);

      else
        act ("$N already has skin made of bone.", ch, NULL, victim, TO_CHAR);
      return;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.location = APPLY_AC;
  af.modifier = -12;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (victim, &af);
  act ("$n's skin turns into white bone.", victim, NULL, NULL, TO_ROOM);
  send_to_char ("Your skin turns into white bone.\n\r", victim);
  return;
}


void chant_unholy_ceremony (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int xp;
  if (IS_NPC (victim))
    {
      send_to_char
        ("This chant is only useful when chanted on players.\r\n", ch);
      return;
    }

  if (ch == victim)
    {
      send_to_char ("This chant can not be chanted on yourself.\r\n", ch);
      return;
    }

  if (check_nonpk (ch, victim))
  return;
  
  if (victim->pcdata->last_death_counter)
    {
      act("Just as $N's spirit is about to reunite with $S body, you absorb it into your own.",
	  ch,NULL,victim,TO_CHAR);
      act("$n absorbs your wandering spirit.",ch,NULL,victim,TO_VICT);
      xp = victim->pcdata->xp_last_death * .25;
      if (xp > level_cost (ch->level) * .40)
        xp = level_cost (ch->level) * .40;	    
      low_gain_exp (ch, xp);
      victim->pcdata->xp_last_death = 0;
      victim->pcdata->last_death_counter = 0;
    }
  else
    act ("You can no longer reap the benefits of this chant from $N", ch, NULL, victim,
         TO_CHAR);
  return;
}

void chant_intoxicate (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  if (IS_NPC (victim))
    {
      send_to_char
        ("This chant is only useful when chanted on players.\r\n", ch);
      return;
    }
  if (!victim->pcdata->loner && victim->clan == CLAN_NONE)
    {
      send_to_char ("This chant can not be used on noclans.\n\r",ch);
      return;
    }
  if (victim->pcdata->condition[COND_DRUNK] > 30 || is_affected (victim, sn))
    {
      send_to_char("Your victim is too drunk already.\n\r",ch);
      return;
    }
  victim->pcdata->condition[COND_DRUNK] = 30;
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 20;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (victim, &af);
  update_aggressor (ch, victim);
  send_to_char("It is suddenly hard to walk, concentrate and do everything else that requires thinking.\n\r",victim);
  act("$N staggers and blinks a few times, trying to focus.",ch,NULL,victim,TO_NOTVICT);
  if (ch != victim)
    act("$N's head is blanketed in invisible and toxic fumes.",ch,NULL,victim,TO_CHAR);
}

void chant_skeletal_mutation (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  act("$n recoils in pain as bone fragments puncture $s skin.",victim,NULL,NULL,TO_ROOM);
  send_to_char("Your bones crackle and twist while fragments puctures your skin.\n\r",victim);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 5;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup (ch->name);
  affect_join (victim, &af);
  damage (ch, victim, ch->level, sn, DAM_ACID, FALSE);
}

void check_skeletal_mutation (CHAR_DATA* ch, bool on_round)
{

  if (is_affected(ch,gsn_skeletal_mutation))
    {
	    crit_strike_possible = FALSE;
      act("$n recoils in pain as bone fragments puncture $s skin.",ch,NULL,NULL,TO_ROOM);
      if (on_round)
	{
  	  send_to_char("Your bones crackle and twist while fragments puctures your skin!\n\r",ch);
	  damage_old(ch,ch,25,gsn_skeletal_mutation,DAM_ACID,FALSE);
	}
      else 
	{
  	  send_to_char("Your insides are torn apart by your deformed bones!\n\r",ch);
	  damage_old(ch,ch,10,gsn_skeletal_mutation,DAM_ACID,FALSE);
	}
      crit_strike_possible = TRUE;
    }
}
      
void chant_pox (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  if (saves_spell (level, victim, DAM_DISEASE) ||
      (IS_NPC (victim) && IS_SET (victim->act, ACT_UNDEAD))
      || is_affected(victim,gsn_immunity))
    {
      if (ch == victim)
        send_to_char ("You feel momentarily ill, but it passes.\n\r", ch);

      else
        act ("$N seems to be unaffected.", ch, NULL, victim, TO_CHAR);
      return;
    }
  act("Hideous yellow pox appears all over $n's body.",victim,NULL,NULL,TO_ROOM);
  send_to_char("Your skin prickles with yellow pox.\n\r",victim);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = ch->level/4;
  af.location = APPLY_CHA;
  af.modifier = -6;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup (ch->name);
  affect_join (victim, &af);
}

void chant_drain_aura (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, hpgain, managain;
  ch->mana += 1;
  if (ch->hit <= 20)
  {
    send_to_char("You do not have enough energy.\n\r",ch);
    return;
  }
  ch->hit -= 20;
  dam = dice (1, 150) + 10;
  hpgain = 15 + dice(1,10);
  managain = 25 + dice(1,10);
  if (dam < hpgain)
    dam = hpgain;
  if (saves_spell (level, victim, DAM_NEGATIVE))
    {
      dam /= 2;
      hpgain /= 2;
    }
  act("Breathing in, you feel $N's energy seep into you.",ch,NULL,victim,TO_CHAR);
  act("$n grabs your wrist and you feel your energy abandon you.",ch,NULL,victim,TO_VICT);
  act("$n grabs $N's wrist, and suddenly you can see energy flowing to $n.",ch,NULL,victim,TO_NOTVICT);
  if (damage_old (ch, victim, dam, sn, DAM_NEGATIVE, FALSE))
    {
      if (victim->mana < managain)
	{
	  if (victim->mana > 0)
	    managain = victim->mana;
	  else managain = 0;
      }
      ch->mana += managain;
      victim->mana -= managain;
    }
  return;
}

void chant_immunity (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  if (is_affected(victim,gsn_immunity))
  {
    if (victim != ch)
      send_to_char("They already have immunity!\n\r",ch);
    else send_to_char("You're already have immunity!\n\r",victim);
    return;
  }
  act("$n's skin turns grey.",victim,NULL,NULL,TO_ROOM);
  send_to_char("Your skin turns grey.\n\r",victim);
  affect_strip(victim,gsn_pox);
  if (is_affected(victim,gsn_brain_blisters))
  {
    affect_strip(victim,gsn_brain_blisters);
    affect_strip(victim,skill_lookup("disorientation"));
  }  
    REMOVE_BIT (victim->affected_by, AFF_PLAGUE);
    affect_strip (victim, gsn_plague);
  affect_strip(victim,gsn_vomit);
  affect_strip(victim,gsn_atrophy);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 22;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (victim, &af);
}
 
void chant_brain_blisters (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  if (saves_spell (level, victim, DAM_DISEASE) ||
      (IS_NPC (victim) && IS_SET (victim->act, ACT_UNDEAD))
      || is_affected(victim,gsn_immunity))
    {
      if (ch == victim)
        send_to_char ("You feel momentarily ill, but it passes.\n\r", ch);

      else
        act ("$N seems to be unaffected.", ch, NULL, victim, TO_CHAR);
      return;
    }
  act("$n grabs $s head as blisters burst on $s scalp!",victim,NULL,NULL,TO_ROOM);
  send_to_char("Pus oozes down into your eyes as the blisters on your scalp burst!\n\r",victim);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 44;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup (ch->name);
  affect_to_char (victim, &af);

  af.where = TO_AFFECTS;
  af.type = gsn_disorientation;
  af.level = level;
  af.duration = 44;
  af.location = APPLY_HITROLL;
  af.modifier = -4;
  af.permaff = FALSE;
  af.bitvector = 0;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_join (victim, &af);

}

void chant_aura_rot (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  if ((saves_spell (level, victim, DAM_MENTAL) && number_percent() <= 10)
	|| is_affected(victim,gsn_immunity))
    {
      if (ch == victim)
        send_to_char ("You feel momentarily ill, but it passes.\n\r", ch);

      else
        act ("$N seems to be unaffected.", ch, NULL, victim, TO_CHAR);
      return;
    }
  act("The light in $n's eye begins to fade.\n\r",victim,NULL,NULL,TO_ROOM);
  send_to_char("You feel your energy begin to fade\n\r",victim);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 6;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup (ch->name);
  affect_to_char (victim, &af);
  return;
}



void chant_vomit (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  if (saves_spell (level, victim, DAM_DISEASE) ||
      (IS_NPC (victim) && IS_SET (victim->act, ACT_UNDEAD))
      || is_affected(victim,gsn_immunity))
    {
      if (ch == victim)
        send_to_char ("You feel momentarily ill, but it passes.\n\r", ch);

      else
        act ("$N seems to be unaffected.", ch, NULL, victim, TO_CHAR);
      return;
    }
  act("$n vomits violently!",victim,NULL,NULL,TO_ROOM);
  send_to_char("It feels like you are turned inside out as you vomit blood.\n\r",victim);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = ch->level/4;
  af.location = APPLY_CON;
  af.modifier = -1;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup (ch->name);
  affect_join (victim, &af);
}

void chant_atrophy (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  if (saves_spell (level, victim, DAM_DISEASE) ||
      (IS_NPC (victim) && IS_SET (victim->act, ACT_UNDEAD))
      || is_affected(victim,gsn_immunity))
    {
      if (ch == victim)
        send_to_char ("You feel momentarily ill, but it passes.\n\r", ch);

      else
        act ("$N seems to be unaffected.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (is_affected(ch,gsn_atrophy))
    {
      send_to_char("Leave that poor atrophied character alone!\n\r",ch);
      return;
    }
  act("$n's body withers before your eyes.",victim,NULL,NULL,TO_ROOM);
  send_to_char("The muscles in your body grow limp and useless.\n\r",victim);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = ch->level/4;
  af.location = APPLY_DEX;
  af.modifier = -3;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (victim, &af);
}

void chant_greymantle (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  if ((saves_spell (level, victim, DAM_MENTAL) && number_percent() <= 10)
      || IS_SET (victim->imm_flags, IMM_MAGIC))
    {
      if (victim != ch)
        send_to_char ("Nothing seemed to happen.\n\r", ch);
      send_to_char ("You feel momentarily weird.\n\r", victim);
      return;
    }
  act("$n's shadow fades.",victim,NULL,NULL,TO_ROOM);
  send_to_char("A feeling of unease settles over you as your shadow is nowhere to be seen.\n\r",victim);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 10;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_join (victim, &af);
}

void reverse_heal(CHAR_DATA* ch, CHAR_DATA* victim, int dam, int sn)
{
  if (victim->hit < 1)
    {
      send_to_char("Your healers powers are rendered useless.\n\r",ch);
      return;
    }
  if (victim->hit-dam < 1)
    dam = victim->hit-1;
  damage_old (ch, victim, adjust_damage(dam), sn, DAM_UNIQUE, TRUE);
}

void chant_lich_eyes (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  if (victim != NULL && victim != ch)
    {
      send_to_char("This chant can only be uttered on yourself.\n\r",ch);
      return;
    }
  if (is_affected(ch,skill_lookup("lich eyes")))
    {
      send_to_char("You're already have lich eyes!\n\r",ch);
      return;
    }
  affect_strip(ch,skill_lookup("detect invis"));
  affect_strip(ch,skill_lookup("detect hidden"));
  act("$n screams as $s eyes rot!",ch,NULL,NULL,TO_ROOM);
  send_to_char("Terrible pain courses through you as your eyes rot in their sockets.\n\r",victim);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 50;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_DETECT_HIDDEN;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (ch, &af);
  af.bitvector = AFF_DETECT_INVIS;
  affect_to_char (ch, &af);
}

void chant_seizure (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  dam = dice(ch->level/3.0,3) + ch->level;
  dam = adjust_damage(dam);
  if (saves_spell (level, victim, DAM_DISEASE))
    {
      dam /= 2;
    }
  if (check_immune (victim, DAM_DISEASE) != IS_IMMUNE && !IS_SET(victim->act,ACT_UNDEAD))
  {
    send_to_char("`kThe ground smashes into you as you convulse with a seizure!``\n\r",victim);
    act("`k$N convulses in seizure as you burn $S mind with unholy fire!``",ch,NULL,victim,TO_CHAR);
    act("`k$n grabs $N's head, who convulses with a seizure!``",ch,NULL,victim,TO_NOTVICT);
  }
  else
  {
    act("You grab $N, but fail to cause a seizure.",ch,NULL,victim,TO_CHAR);
    act("$n grabs $N, but it appears to have no affect.",ch,NULL,victim,TO_NOTVICT);
    act("$n grabs you, but it appears to have no affect.",ch,NULL,victim,TO_VICT);
  }
  damage_old (ch, victim, dam, sn, DAM_DISEASE, FALSE);
  return;
}

void chant_ghost_form (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  AFFECT_DATA af;
  if (is_affected(ch,skill_lookup("ghost form")))
    {
      send_to_char("You are already in the limbo between life and death\n\r",ch);
      return;
    }
  if (is_affected (ch, gsn_soul_link))
    {
	send_to_char("You cannot move into the limbo between life and death if you are sharing a soul.\n\r",ch);
    }
  act("$n turns translucent.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You step into the limbo between life and death.\n\r",ch);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 8;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (ch, &af);
}

void chant_eldritch_miasma (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  AFFECT_DATA af;
  if (is_affected(ch,skill_lookup("eldritch miasma")))
    {
      send_to_char("You are already surrounded by a green fog.\n\r",ch);
      return;
    }
  act("$n starts twitching and a heavy smoke seeps of out $s pores.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You start twitching and a heavy smoke seeps of out your pores.\n\r",ch);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 44;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (ch, &af);
}

bool check_eldritch_miasma (CHAR_DATA * ch, CHAR_DATA * victim)
{
  int chance;
  if (!IS_AWAKE (victim))
    return FALSE;
  if (IS_NPC(victim))
    return FALSE;
  if (!is_affected(victim,skill_lookup("eldritch miasma")))
    return FALSE;
  chance = get_skill (victim, skill_lookup("eldritch miasma")) / 2;
  
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
  if (number_percent () >= chance)
    return FALSE;

  act ("The green fog surrounding $N solidifies and absorbs your attack.", ch,
       NULL, victim, TO_CHAR);
  act ("The green fog enveloping you solidifies to absorb $n's attack.", ch,
       NULL, victim, TO_VICT);
  act ("The green fog surrounding $N solidifies and absorbs $n's attack.",
       ch,NULL,victim,TO_NOTVICT);
  return TRUE;
}

void chant_nearsight (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int chance=0;
  if (is_affected(victim,skill_lookup("nearsight")))
    {
      send_to_char("They already have trouble seeing.\n\r",ch);
      return;
    }
  if (victim->saving_throw > 30)
    chance = 90;
  else if (victim->saving_throw > 20)
    chance = 50 + (victim->saving_throw-20)*4;
  else if (victim->saving_throw > 5)
    chance = 40 + (victim->saving_throw-5)*2;
  else chance = victim->saving_throw*4;
  if (ignore_save)
    chance = 0;
  if (chance < number_percent())
    {
      act("$n screams as $s eyes boil!",victim,NULL,NULL,TO_ROOM);
      send_to_char("Your eyes boil!\n\r",victim);
      af.where = TO_AFFECTS;
      af.type = sn;
      af.level = level;
      af.duration = 30;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = 0;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (victim, &af);
    }
  else
    {
      send_to_char("Your efforts are foiled.\n\r",ch);
      return;
    }
}
  
void chant_clasp (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int duration;
  AFFECT_DATA af;
  if (ch->in_room->sector_type == SECT_AIR)
    {
      send_to_char("That will not work here.\n\r",ch);
      return;
    }
  if (is_affected(victim,gsn_clasp))
    {
      send_to_char("They are already restrained.\n\r",ch);
      return;
    }
  switch(get_curr_stat(victim,STAT_STR))
    {
    case 25:duration = 2;break;
    case 24:duration = 5;break;
    case 23:duration = 6;break;
    case 22:duration = 7;break;
    case 21:
    case 20:
    case 19:duration = 10;break;
    case 18:
    case 17:
    case 16:duration = 12;break;
    case 15:duration = 14;break;
    case 14:duration = 15;break;
    case 3:
    case 2:
    case 1:
    case 0:duration = 25;break;
    default:duration = 20;break;
    }
  act("Skeletal hands burst out of the ground and grab $n's legs!",victim,NULL,NULL,TO_ROOM);
  send_to_char("Skeletal hands burst out of the ground and grab your ankles!\n\r",victim);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = duration;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (victim, &af);
}

bool check_clasp (CHAR_DATA *ch)
{
  AFFECT_DATA *af, af2;
  if (!check_torture(ch))
    return FALSE;
  if (!is_affected(ch,gsn_clasp))
    return TRUE;
  for (af = ch->affected; af != NULL; af = af->next)
    {
      if (af->type == gsn_clasp)
	break;
    }
  act("$n struggles, trying to break free from the skeletal hands.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You try to break free from the skeletal hands clasping your legs!\n\r",ch);
  if (af->duration-1 == 0)
  {
    affect_strip(ch,gsn_clasp);
    send_to_char (skill_table[af->type].msg_off, ch);
    send_to_char ("\n\r", ch);
    return TRUE;
  }
  else 
    {
      af2.where = af->where;
      af2.type = af->type;
      af2.level = af->level;
      af2.duration = -1;
      af2.location = af->location;
      af2.modifier = 0;
      af2.permaff = FALSE;
      af2.bitvector = 0;
      af2.composition = FALSE;
      af2.comp_name = str_dup ("");
      affect_join (ch, &af2);
    }
  return FALSE;
}

void chant_corpse_walk (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  if (IS_NPC(victim))
    {
      send_to_char("This only works on players.\n\r",ch);
      return;
    }
  sprintf(buf,"the corpse of %s",victim->name);
  for (obj = object_list;obj != NULL;obj = obj->next)
    {
      if (obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC)
	{
	  if (!str_cmp(buf,obj->short_descr))
	    break;
	}
    }
  if (obj == NULL || obj->timer < 4 || victim->pcdata->corpse != NULL || obj->carried_by || (obj->in_room == NULL))
    {
      act("There's an anticipated feeling like something is going to happen...but it's doubtful it ever will.",ch,NULL,NULL,TO_ALL);
      return;
    }
  act("There's an anticipated feeling like something is going to happen...",ch,NULL,NULL,TO_ALL);
  victim->pcdata->corpse = obj;
  victim->pcdata->corpse_timer = 3;
  victim->pcdata->corpse_to_room = victim->in_room;
  if (obj->in_room && obj->in_room->people)
    act ("$p rises from the ground and stumbles away.",obj->in_room->people,obj,NULL,TO_ALL);
  delete_player_corpse(obj);
  obj_from_room(obj);
  obj_to_room(obj,get_room_index(ROOM_VNUM_DV_LIMBO));
  save_player_corpse(obj);
}

void chant_jinx (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int chance=0;
  if (is_affected(victim,skill_lookup("jinx")))
    {
      send_to_char("That poor soul is already jinxed!\n\r",ch);
      return;
    }
  if (victim->saving_throw > 30)
    chance = 90;
  else if (victim->saving_throw > 20)
    chance = 50 + (victim->saving_throw-20)*4;
  else if (victim->saving_throw > 5)
    chance = 40 + (victim->saving_throw-5)*2;
  else chance = victim->saving_throw*4;
  if (ignore_save)
    chance = 0;
  if (chance < number_percent())
    {
      act("$n stares at $N intently and mutters a curse under $s breath.",ch,NULL,victim,TO_NOTVICT);
      act("$n stares at you with hate in $s eyes while muttering a curse under $s breath.",ch,NULL,victim,TO_VICT);
      act("You place a jinx of bad luck on $N.",ch,NULL,victim,TO_CHAR);
      af.where = TO_SKILL;
      af.type = sn;
      af.level = level;
      af.duration = 20;
      af.location = 0;
      af.modifier = -10;
      af.bitvector = 0;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (victim, &af);
    }
  else
    {
      act("$n stares at $N intently and mutters something under $s breath.",
	  ch,NULL,victim,TO_NOTVICT);
      act("$n stares at you with hate in $s eyes while muttering something under $s breath.",
	  ch,NULL,victim,TO_VICT);
      act("You attempt to place a jinx of bad luck on $N, but fail.",ch,NULL,victim,TO_CHAR);
    }
}

void chant_demonic_possession (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  if (is_affected(victim,skill_lookup("demonic possession")))
    {
      send_to_char("They are already possessed.\n\r",ch);
      return;
    }
  if (victim->alignment > -800)
    {
      send_to_char("They are not demonic enough.\n\r",ch);
      return;
    }

  act("$n clenches $s teeth as a demonic being takes over $s body.",victim,NULL,NULL,TO_ROOM);
  send_to_char("Your will is stripped away and replaced by an unearthly rage!\n\r",victim);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = ch->level/5;
  af.location = APPLY_DAMROLL;
  af.modifier = 10;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (victim, &af);
  af.location = APPLY_HITROLL;
  affect_to_char (victim, &af);
  af.location = gsn_berserk;
  af.where = TO_SKILL;
  af.modifier = 60;
  affect_to_char (victim, &af);
}

void chant_beacon_of_the_damned (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj;
  CHAR_DATA *fch;
  AFFECT_DATA af;
  OBJ_INDEX_DATA *oid;
  if (ch->alignment > -800)
    {
      send_to_char("You are not demonic enough.\n\r",ch);
       ch->mana += UMAX (skill_table[sn].min_mana, 100 / (2 + ch->level - level_for_skill (ch, sn))); 
      return;
    }
  oid = get_obj_index (OBJ_VNUM_BEACON);
  if (oid->count > 0)
    {
      send_to_char("There is already a beacon in this world.\n\r",ch);
       ch->mana += UMAX (skill_table[sn].min_mana, 100 / (2 + ch->level - level_for_skill (ch, sn))); 
      return;
    }
  act("Verdant fire vomits from the septagram leaving the sky ablaze with a nameless evil!",ch,NULL,NULL,TO_ALL);
  do_echo(ch,"`jA green pillar pierces the heavens as the harbinger of death lends aid to all who will ride with him.``");
  obj = create_object(oid,0);
  obj->timer = ch->level/10;
  obj_to_room(obj,ch->in_room);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = ch->level/10;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  for (fch = char_list;fch != NULL;fch = fch->next)
    {
      if (!IS_NPC(fch) && fch->alignment < -400)
	{ 
	  af.location = APPLY_SAVES;
	  af.modifier = -5;
	  affect_to_char (fch, &af);
	  af.location = APPLY_AC;
	  if (IS_SET(fch->act,ACT_UNDEAD))
	    af.modifier = -20;
	  else af.modifier = -10;
          affect_to_char (fch, &af);
	  af.modifier = 10;
	  af.location = APPLY_DAMROLL;
	  affect_to_char (fch, &af);
	  af.location = APPLY_HITROLL;
	  affect_to_char (fch, &af);
	  af.location = APPLY_STR;
	  af.modifier = 1;
	  affect_to_char (fch, &af);
	  af.location = APPLY_INT;
	  affect_to_char (fch, &af);
	  af.location = APPLY_WIS;
          affect_to_char (fch, &af);
	  af.location = APPLY_DEX;
          affect_to_char (fch, &af);
	  af.location = APPLY_CON;
          affect_to_char (fch, &af);
	  af.location = APPLY_CHA;
          affect_to_char (fch, &af);
	  send_to_char("Your whole being shivers with a surge of power.\n\r",fch);
	}
    }
}

void chant_swell (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  OBJ_DATA *obj;
  AFFECT_DATA af;
  if (is_affected(ch,skill_lookup("swell")))
    {
      send_to_char("They are already swollen.\n\r",ch);
      return;
    }
  act("The hexagram burns with a green fire.",ch,NULL,NULL,TO_ALL);
  act("$n's skin bubbles and swells!",ch,NULL,victim,TO_NOTVICT);
  act("With a surprised look at you, $N starts to swell.",ch,NULL,victim,TO_CHAR);
  send_to_char("Terrible pain shoots through you as your skin bubbles and swells.\n\r",victim);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = ch->level/5;
  af.location = APPLY_HIT;
  af.modifier = victim->max_hit*.15;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (victim, &af);
  victim->hit += af.modifier;
  af.where = TO_VULN;
  af.bitvector = VULN_PIERCE;
  af.location = 0;
  af.modifier = 0;
  affect_to_char (victim, &af);
  obj = get_eq_char (victim, WEAR_BODY);
  if (obj != NULL)
  {
    obj_from_char(obj);
    obj_to_room(obj,ch->in_room);
    act("$p pop off of $n and falls to the ground.",victim,obj,NULL,TO_ROOM);
    act("$p pop off of you and falls to the ground.",victim,obj,NULL,TO_CHAR);
  }
  obj = get_eq_char (victim, WEAR_LEGS);
  if (obj != NULL)
  {
    obj_from_char(obj);
    obj_to_room(obj,ch->in_room);
    act("$p pop off of $n and falls to the ground.",victim,obj,NULL,TO_ROOM);
    act("$p pop off of you and falls to the ground.",victim,obj,NULL,TO_CHAR);
  }
}

void chant_death_vision (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo, *ch2,*ch2_next;
  OBJ_DATA *obj, *obj2;
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *on;
  ROOM_INDEX_DATA *old_room;
  obj = get_obj_vnum_room(ch->in_room,OBJ_VNUM_PENTAGRAM);
  if (!obj)
    {
      send_to_char ("You see no pentagram here.\n\r",ch);
      return;
    }
  sprintf(buf,"The eye of`b %s``",victim->name);
  for (obj2 = obj->contains;obj2 != NULL;obj2 = obj2->next_content)
    {
      if (obj2->pIndexData->vnum == OBJ_VNUM_EYE)
	{
	  if (!str_cmp(buf,obj2->short_descr))
	    break;
	}
    }
  if (obj2 == NULL || !victim->in_room)
  {
    send_to_char("You are missing the proper component.\n\r",ch);
    return;
  }
  if (victim->in_room == ch->in_room)
  {
    send_to_char("You realize you can just type look to see what room they're in, and slap yourself in the face for your stupidity.\n\r",ch);
    return;
  }
  act("$p explodes! In the smoke you see a vision...",ch,obj2,NULL,TO_ALL);
  obj_from_obj(obj2);
  extract_obj(obj2);
  sprintf(buf,"%s look",victim->name);
  old_room = ch->in_room;
  for (ch2 = ch->in_room->people;ch2 != NULL;ch2 = ch2_next)
    {
	ch2_next = ch2->next_in_room;
        on = ch2->on;
        char_from_room(ch2);
        char_to_room(ch2,victim->in_room);
	do_look(ch2,"auto");
	//Iblis - What a horrible hack..but i couldn't think of a 
	//        much better way to do it
	char_from_room(ch2);
	char_to_room(ch2,get_room_index(ROOM_VNUM_DV_LIMBO));
	ch2->on = on;
    }
  for (ch2 = ch->in_room->people;ch2 != NULL;ch2 = ch2_next)
  {
    ch2_next = ch2->next_in_room;
    on = ch2->on;
    char_from_room(ch2);
    char_to_room(ch2,old_room);
    ch2->on = on;
  }
}

void chant_bone_craft (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  char *ownmessage = (char*) vo, *name=NULL;
  OBJ_DATA *obj,*obj2,*obj3,*obj4;
  short armcount=0, legcount=0,i;
  char buf[MAX_STRING_LENGTH];
  AFFECT_DATA *paf;
  CHAR_DATA *mount;
  obj = get_obj_vnum_room(ch->in_room,OBJ_VNUM_PENTAGRAM);
  if (!obj)
    {
      send_to_char ("You see no pentagram here.\n\r",ch);
      return;
    }
  obj2 = get_obj_vnum_container(obj,OBJ_VNUM_EYE);
  obj3 = get_obj_vnum_container(obj,OBJ_VNUM_SINEW);
  if (obj2 && obj3)
    {
      obj_from_obj(obj2);
      obj_from_obj(obj3);
      obj4 = create_object(get_obj_index(OBJ_VNUM_BC_NECKLACE),0);
      if (strlen(obj2->short_descr) > 15)
	name = obj2->short_descr+13;
      sprintf (buf, obj4->short_descr, name);
      free_string (obj4->short_descr);
      obj4->short_descr = str_dup (buf);
      obj_to_obj(obj4,obj);
      //MAKE NECKLACE
      act("You construct a $p out of various materials.",ch,obj4,NULL,TO_CHAR);
      act("$n constructs a $p out of various materials.",ch,obj4,NULL,TO_ROOM);
      extract_obj(obj2);
      extract_obj(obj3);
      return;
    }
  obj2 = get_obj_vnum_container(obj,OBJ_VNUM_SKIN);
  if (obj2 && obj3)
    {
      obj_from_obj(obj2);
      obj_from_obj(obj3);
      obj4 = create_object(get_obj_index(OBJ_VNUM_BC_SADDLEBAGS),0);
      if (ownmessage != NULL)
        sprintf (buf, obj4->short_descr, ownmessage);
      else sprintf(buf, obj4->short_descr, "I'm forgetful");
      free_string (obj4->short_descr);
      obj4->short_descr = str_dup (buf);
      obj_to_obj(obj4,obj);
      act("You construct a $p out of various materials.",ch,obj4,NULL,TO_CHAR);
      act("$n constructs a $p out of various materials.",ch,obj4,NULL,TO_ROOM);
      extract_obj(obj2);
      extract_obj(obj3);
      return;
      //MAKE SADDLEBAGS
    }
  obj2 = get_obj_vnum_container(obj,OBJ_VNUM_RIB_BONE);
  if (obj2 && obj3)
    {
      obj_from_obj(obj2);
      obj_from_obj(obj3);
      obj4 = create_object(get_obj_index(OBJ_VNUM_BC_DAGGER),0);
      //MAKE DAGGER
      paf = new_affect ();
      paf->where = TO_OBJECT;
      paf->type = -1;
      paf->level = level;
      paf->duration = -1;
      paf->location = APPLY_DAMROLL;
      paf->modifier = ch->level/30+1;
      paf->bitvector = 0;
      paf->permaff = FALSE;
      paf->composition = FALSE;
      paf->comp_name = str_dup ("");
      paf->next = obj4->affected;
      obj4->affected = paf;

      paf = new_affect ();
      paf->type = -1;
      paf->where = TO_OBJECT;
      paf->level = level;
      paf->duration = -1;
      paf->location = APPLY_HITROLL;
      paf->modifier = ch->level/30+1;
      paf->bitvector = 0;
      paf->permaff = FALSE;
      paf->composition = FALSE;
      paf->comp_name = str_dup ("");
      paf->next = obj4->affected;
      obj4->affected = paf;

      obj_to_obj(obj4,obj);
      act("You construct a $p out of various materials.",ch,obj4,NULL,TO_CHAR);
      act("$n constructs a $p out of various materials.",ch,obj4,NULL,TO_ROOM);
      extract_obj(obj2);
      extract_obj(obj3);
      return;
    }
  obj3 = get_obj_vnum_container(obj,OBJ_VNUM_THIGH_BONE);
  if (obj3)
    obj_from_obj(obj3);
  if (obj2 && obj3 && (obj4 = get_obj_vnum_container(obj,OBJ_VNUM_THIGH_BONE)))
    {
      obj_from_obj(obj2);
      obj_from_obj(obj4);
      extract_obj(obj2);
      extract_obj(obj3);
      extract_obj(obj4);
      obj4 = create_object(get_obj_index(OBJ_VNUM_BC_SCYTHE),0);
      if (ownmessage != NULL)
	sprintf (buf, obj4->short_descr, ownmessage);
      else sprintf (buf, obj4->short_descr, "I'm forgetful");
      free_string (obj4->short_descr);
      obj4->short_descr = str_dup (buf);
      paf = new_affect ();
      paf->where = TO_OBJECT;
      paf->type = -1;
      paf->level = level;
      paf->duration = -1;
      paf->location = APPLY_DAMROLL;
      paf->modifier = ch->level/20+2;
      paf->bitvector = 0;
      paf->permaff = FALSE;
      paf->composition = FALSE;
      paf->comp_name = str_dup ("");
      paf->next = obj4->affected;
      obj4->affected = paf;
      paf = new_affect ();
      paf->type = -1;
      paf->where = TO_OBJECT;
      paf->level = level;
      paf->duration = -1;
      paf->location = APPLY_HITROLL;
      paf->modifier = ch->level/20+2;
      paf->bitvector = 0;
      paf->permaff = FALSE;
      paf->composition = FALSE;
      paf->comp_name = str_dup ("");
      paf->next = obj4->affected;
      obj4->affected = paf;

      obj_to_obj(obj4,obj);
      act("You construct a $p out of various materials.",ch,obj4,NULL,TO_CHAR);
      act("$n constructs a $p out of various materials.",ch,obj4,NULL,TO_ROOM);
      return;
    }
  else if (obj3)
    obj_to_obj(obj3,obj);
  for (obj2 = obj->contains;obj2 != NULL;obj2 = obj2->next_content)
    {
      if (obj2->pIndexData->vnum == OBJ_VNUM_SLICED_LEG)
        ++legcount;
      if (obj2->pIndexData->vnum == OBJ_VNUM_SLICED_ARM)
	++armcount;
    }
  if (armcount >= 4 && legcount >= 4)
    {
      for (i = 0;i<8;i++)
	{
	  if (i < 4)
	    obj2 = get_obj_vnum_container(obj,OBJ_VNUM_SLICED_LEG);
	  else obj2 = get_obj_vnum_container(obj,OBJ_VNUM_SLICED_ARM);
	  obj_from_obj(obj2);
	  extract_obj(obj2);
	}
      mount = create_mobile (get_mob_index(MOB_VNUM_BC_MOUNT));
      if (ch->max_hit < 15000)
      {
        mount->max_hit = ch->max_hit*2;
        mount->hit = ch->max_hit*2;
      }
      else
      {
        mount->max_hit = 30000;
        mount->hit = 30000;
      }
      mount->level = ch->level;
      mount->afkmsg = str_dup (ch->name);
      char_to_room(mount,ch->in_room);
      SET_BIT (mount->act, ACT_SENTINEL);
      act("You construct $N out of various materials.",ch,NULL,mount,TO_CHAR);
      act("$n constructs $N out of various materials.",ch,NULL,mount,TO_ROOM);
      return;
    }
  send_to_char("There were not enough materials to construct anything!\n\r",ch);
}

void chant_darkhounds (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  int chance = number_percent();
  CHAR_DATA* mob;
  for (mob = char_list;mob != NULL;mob = mob->next)
  {
    if (IS_NPC(mob) && mob->pIndexData->vnum == MOB_VNUM_DARKHOUNDS
		    && !str_cmp(mob->afkmsg,ch->name) && is_same_group(mob,ch))
    {
      send_to_char("You already have some darkhounds.\n\r",ch);
      return;
    }
  }
  if (chance > 90)
    {
      act("$n summons forth the hounds of the underworld.",ch,NULL,NULL,TO_ROOM);
      send_to_char("You summon forth the hounds of the underworld.\n\r",ch);		  
      mob = create_mobile(get_mob_index(MOB_VNUM_DARKHOUNDS));
      char_to_room(mob,ch->in_room);
      mob->afkmsg = str_dup (ch->name);
      mob->ticks_remaining = ch->level/9;
      add_follower (mob, ch);
      mob->leader = ch;
    }
  if (chance > 60)
    {
      if (chance <= 90)
      {
        act("$n summons forth the hounds of the underworld.",ch,NULL,NULL,TO_ROOM);
        send_to_char("You summon forth the hounds of the underworld.\n\r",ch);
      }
      mob = create_mobile(get_mob_index(MOB_VNUM_DARKHOUNDS));
      char_to_room(mob,ch->in_room);
      mob->afkmsg = str_dup (ch->name);
      mob->ticks_remaining = ch->level/9;
      add_follower (mob, ch);
      mob->leader = ch;
    }
  else 
    {
      act("$n summons forth a hound of the underworld.",ch,NULL,NULL,TO_ROOM);
      send_to_char("You summon forth a hound of the underworld.\n\r",ch);
    }

  mob = create_mobile(get_mob_index(MOB_VNUM_DARKHOUNDS));
  char_to_room(mob,ch->in_room);
  mob->afkmsg = str_dup (ch->name);
  mob->ticks_remaining = ch->level/9;
  add_follower (mob, ch);
  mob->leader = ch;
}


void chant_animate_dead (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA* mob;
  OBJ_DATA* obj, *prev, *prev_next;
  char buf[MAX_STRING_LENGTH];
  if (IS_NPC(ch))
    return;
  if ((obj = get_obj_vnum_room(ch->in_room,OBJ_VNUM_CORPSE_NPC)) == NULL
      && ((obj = get_obj_vnum_char(ch,OBJ_VNUM_CORPSE_NPC)) == NULL))
    {
      send_to_char("A corpse is required for this chant.\n\r",ch);
      ch->mana += UMAX (skill_table[sn].min_mana, 100 / (2 + ch->level - level_for_skill (ch, sn)));
      
      return;
    }
  if (IS_SET (ch->in_room->room_flags, ROOM_NO_CHARMIE))
    {
      send_to_char ("This room is resistant to your animation.\n\r", ch);
      return;
    }
  if (ch->pcdata->familiar != NULL)
  {
    send_to_char("You already have a familiar in the realms.\n\r",ch);
    return;
  }
  if (obj->carried_by)
    {
      for (prev = obj->contains; prev != NULL; prev = prev_next)
        {
          prev_next = prev->next_content;
          obj_from_obj(prev);
          obj_to_room(prev,obj->carried_by->in_room);
          sprintf (buf, "$p falls from %s.", obj->short_descr);
          act (buf, ch, prev, NULL, TO_ROOM);
          act (buf, ch, prev, NULL, TO_CHAR);
        }
      obj_from_char(obj);
    }
  else
    {
      for (prev = obj->contains; prev != NULL; prev = prev_next)
        {
          prev_next = prev->next_content;
          obj_from_obj(prev);
          obj_to_room(prev,obj->in_room);
          sprintf (buf, "$p falls from %s.", obj->short_descr);
          act (buf, ch, prev, NULL, TO_ROOM);
          act (buf, ch, prev, NULL, TO_CHAR);
        }
      obj_from_room(obj);
    }
  extract_obj(obj);
  //CREATE THE FUCKING MINION HERE
  if (ch->pcdata->familiar_type == FAMILIAR_SKELETON)
    {
      mob = create_mobile(get_mob_index(MOB_VNUM_FAM_SKELETON));
      mob->perm_stat[STAT_STR] = 20;
      mob->perm_stat[STAT_INT] = 6;
      mob->perm_stat[STAT_WIS] = 3;
      mob->perm_stat[STAT_DEX] = 15;
      mob->perm_stat[STAT_CON] = 18;
      mob->perm_stat[STAT_CHA] = 3;
      mob->mod_stat[STAT_STR] = 0;
      mob->mod_stat[STAT_INT] = 0;
      mob->mod_stat[STAT_WIS] = 0;
      mob->mod_stat[STAT_DEX] = 0;
      mob->mod_stat[STAT_CON] = 0;
      mob->mod_stat[STAT_CHA] = 0;
      act("A skeleton breaks loose from the fleshy confines of a rotting corpse.",ch,NULL,NULL,TO_ALL);
    }
  else if (ch->pcdata->familiar_type == FAMILIAR_BANSHEE)
    {
      mob = create_mobile(get_mob_index(MOB_VNUM_FAM_BANSHEE));
      mob->perm_stat[STAT_STR] = 16;
      mob->perm_stat[STAT_INT] = 13;
      mob->perm_stat[STAT_WIS] = 3;
      mob->perm_stat[STAT_DEX] = 15;
      mob->perm_stat[STAT_CON] = 15;
      mob->perm_stat[STAT_CHA] = 3;
      mob->mod_stat[STAT_STR] = 0;
      mob->mod_stat[STAT_INT] = 0;
      mob->mod_stat[STAT_WIS] = 0;
      mob->mod_stat[STAT_DEX] = 0;
      mob->mod_stat[STAT_CON] = 0;
      mob->mod_stat[STAT_CHA] = 0;
      act("A black mist rises from a nearby corpse.",ch,NULL,NULL,TO_ALL);
    }
  else
    {
      mob = create_mobile(get_mob_index(MOB_VNUM_FAM_ZOMBIE));
      mob->perm_stat[STAT_STR] = 15;
      mob->perm_stat[STAT_INT] = 6;
      mob->perm_stat[STAT_WIS] = 3;
      mob->perm_stat[STAT_DEX] = 18;
      mob->perm_stat[STAT_CON] = 20;
      mob->perm_stat[STAT_CHA] = 3;
      mob->mod_stat[STAT_STR] = 0;
      mob->mod_stat[STAT_INT] = 0;
      mob->mod_stat[STAT_WIS] = 0;
      mob->mod_stat[STAT_DEX] = 0;
      mob->mod_stat[STAT_CON] = 0;
      mob->mod_stat[STAT_CHA] = 0;
      act("With a groan, a corpse shambles to its feet.",ch,NULL,NULL,TO_ALL);
    }

  mob->exp = ch->pcdata->familiar_exp;
  mob->max_hit = ch->pcdata->familiar_max_hit;
  mob->hit = ch->pcdata->familiar_max_hit/2;
  mob->max_mana = ch->pcdata->familiar_max_mana;
  mob->mana = ch->pcdata->familiar_max_mana;
  mob->max_move = ch->pcdata->familiar_max_move;
  mob->move = ch->pcdata->familiar_max_move;
  mob->level = ch->pcdata->familiar_level;
  sprintf (buf, "%s %s", mob->name, ch->pcdata->familiar_name);
  free_string (mob->name);
  mob->name = str_dup (buf);
  mob->afkmsg = str_dup (ch->name);
  SET_BIT(mob->act2,ACT_FAMILIAR);
  SET_BIT(mob->affected_by,AFF_CHARM);
  char_to_room(mob,ch->in_room);
  add_follower (mob, ch);
  ch->pcdata->familiar = mob;
  mob->leader = ch;

}

void chant_astral_body (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  spell_invis(sn,level,ch,vo,target);
  if (!IS_NPC(victim) && victim->pcdata->familiar != NULL)
    spell_invis(sn,level,ch,(void *)victim->pcdata->familiar,target);
} 

void chant_calling (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  OBJ_DATA *obj;
  // the victim is a real char - lets be nice
  if (IS_NPC(ch))
    return;
  obj = get_obj_vnum_room(ch->in_room,OBJ_VNUM_HEXAGRAM);
  if (obj == NULL)
    {
      send_to_char ("A hexagram is required for this chant.\n\r",ch);
      return;
    }
  if (count_users (obj) >= obj->value[0])
    {
      send_to_char ("The hexagram is too full for a calling to work.\n\r",ch);
      return;
    }
  if (nogate || (!IS_NPC(victim) && victim->pcdata->nogate) || (!IS_NPC(ch) && ch->pcdata->nogate))
    {
      send_to_char("The gods are not allowing that at the moment.\n\r",ch);
      return;
    }
  if ((victim->in_room == NULL) || (victim->level >= LEVEL_IMMORTAL))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  // the victim is a real victim, but for various reasons the mage
  // might fail, in these cases check for failure and maybe summon_nasty
  if (victim != ch->pcdata->familiar)
    {
      if (IS_SET (victim->in_room->room_flags, ROOM_SAFE)
	  || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
	  || (ch->in_room->sector_type == SECT_WATER_OCEAN)
	  || (victim->in_room == ch->in_room)
	  || (victim->fighting != NULL)
	  || (victim->in_room->sector_type == SECT_SWAMP)
	  || (IS_NPC (victim)
	  || (IS_SET(ch->in_room->area->area_flags,AREA_IMP_ONLY) && get_trust(victim) < MAX_LEVEL)
          || (ch->in_room->owner[0] != '\0')
	  || IS_SET(ch->in_room->area->area_flags,AREA_SOLO)
          || IS_SET(ch->in_room->room_flags2,ROOM_SOLO)
          || !(can_move_char(victim,ch->in_room,FALSE,FALSE))
	      || (victim->level < 8) || (!(battle_royale && (victim->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner)))
					 && IS_SET (victim->act, PLR_NOSUMMON))))
	{
          send_to_char ("You failed.\n\r", ch);
          return;
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
      send_to_char("A glowing hexagram appears under your feet!\n\r",victim);
      act("A glowing hexagram appears under $n's feet!",victim,NULL,NULL,TO_ROOM);
      if (saves_spell (level + 10, victim, DAM_MENTAL))
	{
	  ch->mana += .8*UMAX (skill_table[sn].min_mana, 100 / (2 + ch->level - level_for_skill (ch, sn)));
	  send_to_char ("Your demonic powers cannot quite grasp them.\n\r",ch);
	  return;
	}
    }
  send_to_char("The world flickers before you.\n\r",victim);
  act("With a green flash, $n is gone!",victim,NULL,NULL,TO_ROOM);
  char_from_room(victim);
  char_to_room(victim,ch->in_room);
  act("$n appears in the middle of the hexagram.",victim,NULL,NULL,TO_ROOM);
  act("The blood that the hexagram was drawn with seeps into the earth.",
		  obj->in_room->people, NULL, NULL, TO_ALL);
  extract_obj(obj);
}



void chant_bone_hail (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  if (ch->in_room == NULL)
    return;
  dam = 250 + number_range(1,250);
  if (!IS_NPC(ch) && ch->pcdata->familiar_type == FAMILIAR_SKELETON && ch->pcdata->familiar
      && ch->pcdata->familiar->in_room == ch->in_room)
    {
      act("`o$N explodes into a storm of bone shards, tearing the life from your enemies!``",ch,NULL,ch->pcdata->familiar,TO_CHAR);
      dam += 100;
      familiar_poof(ch);
    }
  else send_to_char ("`oYou conjure a storm of splintered bones that tears your enemies apart!``\n\r", ch);
  dam = adjust_damage(dam);
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;
      if (is_safe_spell (ch, vch, TRUE)
          || (IS_NPC (ch) && IS_NPC (vch)
              && (ch->fighting == vch || vch->fighting == ch)))
        continue;
      send_to_char ("`oA storm of splintered bones tears you apart!``\n\r", vch);
      /*if (saves_spell (level, vch, DAM_PIERCE))
          damage_old (ch, vch, dam / 2, sn, DAM_PIERCE, FALSE);
      else*/
          damage_old (ch, vch, dam, sn, DAM_PIERCE, FALSE);
    }
}

void chant_wind_of_death (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam, attempt;
  AFFECT_DATA af;
  ROOM_INDEX_DATA *was_in;
  send_to_char ("`hYou summon forth the screaming wind of hell!``\n\r", ch);
  dam = number_range(80,120);
  dam = adjust_damage(dam);
  if (!IS_NPC(ch) && ch->pcdata->familiar_type == FAMILIAR_BANSHEE && ch->pcdata->familiar
      && ch->pcdata->familiar->in_room == ch->in_room)
    {
      dam += 50;
    }
  act ("`hA screaming wind slashes your skin into thin strips!``", ch, NULL, NULL, TO_ROOM);
  af.where = TO_AFFECTS;
  af.type = gsn_fear;
  af.level = ch->level;
  af.duration = (ch->level > 75)?2:1;
  af.location = APPLY_HITROLL;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  af.bitvector = AFF_CALM;

  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;
      if (vch == ch)
	      continue;
      if (is_safe_spell (ch, vch, TRUE)
          || (IS_NPC (ch) && IS_NPC (vch)
              && (ch->fighting == vch || vch->fighting == ch)))
        continue;
      if (IS_SET(vch->affected_by,AFF_FLYING))
	damage_old (ch, vch, dam*3, sn, DAM_WIND, TRUE);
      else 
      damage_old (ch, vch, dam, sn, DAM_WIND, TRUE);
      if (vch->hit > 1) //IE they just died)..if they didn't die and hp is 1, I can live with that
      {
        if (number_percent() <= 20)
	{

	  if (get_position(vch) <= POS_SLEEPING || vch->position == POS_MOUNTED)
	    continue;

	  if (IS_AFFECTED (vch, AFF_CALM))
	    continue;

	  if (IS_NPC (vch) && (vch->pIndexData->pShop != NULL ||
				  IS_SET (vch->act, ACT_TRAIN)
				  || IS_SET (vch->act, ACT_PRACTICE)
				  || IS_SET (vch->act, ACT_IS_HEALER)
				  || IS_SET (vch->act2, ACT_NOMOVE)))
	    continue;


	  if (saves_spell (ch->level, vch, DAM_MENTAL))
	    continue;

	  if (ch->fighting != vch && vch->fighting != ch)
	    continue;

	  if (vch->position == POS_RESTING || vch->position == POS_SITTING)
	    do_stand (vch, "");
	  
	  was_in = vch->in_room;
	  for (attempt = 0; attempt < 6; attempt++)
	    {
	      EXIT_DATA *pexit;
	      int door;
	      door = attempt;//number_door ();
	      if ((pexit = was_in->exit[door]) == 0
              || pexit->u1.to_room == NULL
		  || IS_SET (pexit->exit_info, EX_CLOSED)
		  || (IS_NPC (vch) && IS_SET (pexit->u1.to_room->room_flags, ROOM_NO_MOB)))
		continue;
	      send_to_char("The intensity of the wind drives you from the room in fright!\n\r",vch);
	      move_char (vch, door, FALSE);
	      if (vch->in_room != was_in)
		break;
	    }
	  if (!IS_NPC (vch))
	    af.modifier = -5;
	  else
	    af.modifier = -2;
	  if (vch->in_room == was_in)
	    send_to_char("The intensity of the wind frightens you into submission.\n\r",vch);
	  affect_to_char (vch, &af);
	  if (vch->in_room != was_in)
	    stop_fighting (vch, TRUE);
	}
      }
    }
}

void chant_festering_boils (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  AFFECT_DATA af;
  act("You hear a subtle popping sound.",ch,NULL,NULL,TO_ROOM);
  send_to_char("Subtle popping sounds are heard as grey boils appear all over your body.\n\r",ch);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 20;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_join (ch, &af);
}

void chant_soul_link (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  if (IS_NPC(ch))
	  return;
  if (IS_NPC(victim))
  {
    send_to_char("Not on NPC's.\n\r",ch);
    return;
  }
  if (is_affected(victim,skill_lookup("ghost form")) || is_affected(ch,skill_lookup("ghost form")))
    {
      send_to_char("Neither you or your victim can be in between worlds.\n\r",ch);
      return;
    }
  if (is_affected(victim,skill_lookup("sanctuary")) || is_affected(ch,skill_lookup("sanctuary")))
    {
      send_to_char("Neither you or your victim can in sanctuary and share your soul.\n\r",ch);
      return;
    }

  act("The hexagram flares up around $n.",victim,NULL,NULL,TO_ROOM);
  send_to_char("The hexagram flares up around you.\n\r",ch);
  act("Your soul is now linked to $N's.",ch,NULL,victim,TO_CHAR);
  act("Your soul is now linked to $n's.",ch,NULL,victim,TO_VICT);
  act("Your life is $S life, $S life is your life.",ch,NULL,victim,TO_CHAR);
  act("Your life is $s life, $s life is your life.",ch,NULL,victim,TO_VICT);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 20;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (ch, &af);
  affect_to_char (victim,&af);
  ch->pcdata->soul_link = victim;
  victim->pcdata->soul_link = ch;
}

void chant_death_shroud (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int chance = 100;
  
  if (is_affected(victim,skill_lookup("death shroud")))
    {
      send_to_char("They are already shrouded in a stench.\n\r",ch);
      return;
    }
  if (!ignore_save)
  {
    if (victim->saving_throw > -5)
      chance -= victim->saving_throw*4;
    else chance -= 20 + (victim->saving_throw-5)*2;
  }
  if (chance > number_percent())
    {
      
      act("$n is shrouded in a stinking cloud.",victim,NULL,NULL,TO_ROOM);
      send_to_char("You are shrouded in a stench of fresh blood.\n\r",victim);
      af.where = TO_AFFECTS;
      af.type = sn;
      af.level = level;
      af.duration = 44;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = 0;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_to_char (victim, &af);
    }
  else send_to_char("You failed.\n\r",ch);
}

void chant_bone_puppet (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj,*obj2;//,*obj3,*obj4;
  int bonecount=0,i=0;
  CHAR_DATA *mob;
  if (ch->in_room == NULL)
    return;
  for (mob = char_list;mob != NULL;mob = mob->next)
    {
      if (IS_NPC(mob) && mob->pIndexData->vnum == MOB_VNUM_BONE_PUPPET
	  && !str_cmp(mob->afkmsg,ch->name))
	{
	  send_to_char("You already have a bone puppet in the realm.\n\r",ch);
	  return;
	}
    }

  obj = get_obj_vnum_room(ch->in_room,OBJ_VNUM_PENTAGRAM);
  if (!obj)
    {
      send_to_char ("You see no pentagram here.\n\r",ch);
      return;
    }
  for (obj2 = obj->contains;obj2 != NULL;obj2 = obj2->next_content)
    {
      if (obj2->pIndexData->vnum == OBJ_VNUM_THIGH_BONE)
        ++bonecount;
      if (obj2->pIndexData->vnum == OBJ_VNUM_RIB_BONE)
        ++bonecount;
    }
  if (bonecount >= 4)
    {
      for (i = 0;i<4;i++)
	{
	  obj2 = get_obj_vnum_container(obj,OBJ_VNUM_THIGH_BONE);
	  if (obj2 == NULL)
	    obj2 = get_obj_vnum_container(obj,OBJ_VNUM_RIB_BONE);
	  obj_from_obj(obj2);
	  extract_obj(obj2);
	}
    }
  else
    {
      send_to_char("You do not have enough bone pieces for a bone puppet.\n\r",ch);
      return;
    }
  mob = create_mobile(get_mob_index(MOB_VNUM_BONE_PUPPET));
  mob->max_hit = 1;
  mob->hit = 1;
  mob->afkmsg = str_dup (ch->name);
  mob->max_move = ch->level;
  mob->move = ch->level;
  mob->ticks_remaining = ch->level/10;
  char_to_room(mob,ch->in_room);
  SET_BIT(mob->act2,ACT_PUPPET);
  add_follower (mob, ch);
  act("A puppet made of bones twitches to life inside the pentagram.",ch,NULL,NULL,TO_ALL);

}

void chant_dark_sphere (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj;
  OBJ_INDEX_DATA *oid;
  if (ch->in_room == NULL)
    return;
  if (IS_SET(ch->in_room->room_flags2,ROOM_ALWAYS_DARK))
    {
      send_to_char("This room is already in darkness.\n\r",ch);
      return;
    }
  oid = get_obj_index (OBJ_VNUM_DARK_SPHERE);
  if (oid->count > 0)
    {
      for (obj = object_list;obj != NULL;obj = obj->next)
	{
	  if (obj->pIndexData->vnum == OBJ_VNUM_DARK_SPHERE)
	    {
	      if (!str_cmp(obj->plr_owner,ch->name))
		{
		  send_to_char("You may only have one dark sphere in the realm at a time.\n\r",ch);
		  return;
		}
	    }
	}
    }
  obj = create_object(oid,0);
  obj->plr_owner = str_dup(ch->name);
  obj->value[0] = ch->in_room->vnum;   //For redundancy, may not use it
  obj_to_room(obj,get_room_index(ROOM_VNUM_DV_LIMBO));
  act("The septagram starts glowing with a green light.",ch,NULL,NULL,TO_ALL);
  send_to_char("You conjure a sphere of complete darkness.\n\r",ch);
  act("$n conjures a sphere of complete darkness.",ch,NULL,NULL,TO_ROOM);
  SET_BIT(ch->in_room->room_flags2,ROOM_ALWAYS_DARK);
  obj->timer = 10;
}

//Iblis - End Necromancer Stuff


