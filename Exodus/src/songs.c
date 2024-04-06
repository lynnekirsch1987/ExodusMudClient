#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"
#include "songs.h"
#include "recycle.h"
#include "tables.h"
#include "interp.h"
#include "obj_trig.h"

char *sing_target_name;
extern bool nogate;
void update_aggressor args ((CHAR_DATA * ch, CHAR_DATA * victim));

/* Composition Name,
   Composition Type (Must be defined in merc.h),
   Mana Required to play this song (2 * this value to compose needed),
   Apply Field (Defined in merc.h),
   Amount to modify Apply Field by,
   Area Affect, or not, (TRUE == AREA, FALSE == SINGLE TARGET),
   Hostile or not, (TRUE == HOSTILE, FALSE == NON-HOSTILE),
   Duration of song affects,
   Level needed to compose this song,
   Base damage done to victims of the song,
   What the character sees when the song hits a victim,
   What the victim sees when the song hits him/her,
   What the victim sees when the song affects wear off,
   What the player sees when they play the song,
   What others see when the player plays the song */

const struct comp_type comp_table[] = {
  {"reserved", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "", "", "", "", ""},

  {"Aria", COMP_ARIA, 20, APPLY_AC, -10, TRUE, FALSE, 20, 1, 0, 0,
   "",
   "The melodies of the aria quicken your mind and reflexes.",
   "The memory of the lighthearted aria leaves your mind.",
   "You raise your voice in melody as you perform '%s' upon $p.",
   "$n sings a quiet aria as the air is filled with the sound of '%s'."},

  {"Ballad", COMP_BALLAD, 30, APPLY_AC, -10, FALSE, FALSE, 20, 1, 0, 0,
   "$N smiles suddenly, enraptured by music.",
   "The romance of song lightens your spirits.",
   "Disremembering the romantic ballad, your spirits sink.",
   "You orate the romantic tune '%s' with swift agility upon $p.",
   "$n sings the haunting romantic tune '%s' while expertly playing $p."},


  {"Battlesong", COMP_BATTLESONG, 30, APPLY_DAMROLL, 2, FALSE, FALSE, 20,
   10, 0, 0,
   "$N growls with an inspired rage!",
   "Your soul erupts in an inspired rage as the song overtakes you!",
   "The riling battlesong slips your memory.",
   "Screaming out in dark tones, you thunder '%s', playing $p fervently!",
   "$n thunders out the dark, rhythmic '%s' upon $p."},

  {"Dirge", COMP_DIRGE, 25, APPLY_AC, 20, TRUE, TRUE, 20, 10, 0, 0,
   "",
   "A deep sense of sadness sinks into your heart.",
   "Summoning your willpower, you cease to remember the mournful elegy.",
   "Raising your voice in elegiac tones, you wail '%s', upon $p.",
   "$n begins to moan the lamentful dirge '%s', accompanying it with $p."},

  {"Sonnet", COMP_SONNET, 25, APPLY_INT, 1, TRUE, FALSE, 20, 1, 0, 0,
   "",
   "Innovative rhymes dance in your head, enlightening you.",
   "Try as you might, you can no longer recall the enlightening sonnet.",
   "You smirk and begin to sing the clever tune, '%s' accenting it with $p.",
   "Piping out a sequence of clever rhymes, $n performs '%s' upon $p."},

  {"Hymn", COMP_HYMN, 50, APPLY_WIS, 3, FALSE, FALSE, 20, 10, 0, 0,
   "$N suddenly bows, deep in thought.",
   "The slow hymn makes you think about the nature of life.",
   "Snapping back to reality, you cease thinking about the nature of life.",
   "You begin to sing '%s', a slow song about a forgotten deity.",
   "$n performs the slow, uplifting hymn '%s' upon $p."},


  {"Serenade", COMP_SERENADE, 20, APPLY_SAVING_SPELL, -1, TRUE, FALSE,
   20,
   1, 0, 0,
   "",
   "Your inner soul glows with ebullience.",
   "The moderate tunes of the serenade fly from your memory.",
   "Smiling slowly, you play a thoughtful, soul resonating tune '%s' upon $p.",
   "$n begins to sing a soulful serenade '%s' while playing $p."},

  {"Melody", COMP_MELODY, 40, APPLY_DEX, 2, FALSE, FALSE, 20, 1, 0, 0,
   "$N begins to dance slowly, speeding up as the song increases in tempo.",
   "You begin to dance quickly, in time with the song.",
   "Panting quietly, you cease your quick dance and move normally.",
   "Plucking out a quick sequence of notes on $p, you play the melody '%s'.",
   "$n plucks out a quick melody called '%s' on $p."},

  {"Satire", COMP_SATIRE, 40, APPLY_AC, 40, FALSE, TRUE, 20, 5, 0, 0,
   "$N grimaces in anger, interpreting the satire.",
   "You growl in anger as you realize the butt of the satire is yourself!",
   "You finally forget the embarrassment of the dreadful satire.",
   "You grin and perform the witty satire '%s', keeping pace with $p.",
   "Grinning derisively, $n sings the wry tune '%s', smirking all the while."},

  {"Symphony", COMP_SYMPHONY, 50, APPLY_STR, 2, FALSE, FALSE, 20, 20, 0,
   0,
   "$N gestures madly, getting caught up in the symphony!",
   "You gesture wildly, getting caught up in the powerful symphony!",
   "You attempt to remember the powerful arrangement of the symphony, and fail.",
   "Arranging melodic components in a rush, the complex symphony '%s' rings out.",
   "$n moves $s hands quickly over $p, performing the complex piece '%s'."},

  {"Jig", COMP_JIG, 25, APPLY_DEX, 1, FALSE, FALSE, 20, 1, 0, 0,
   "$N grins broadly and dances quickly.",
   "Your tempo increases as you dance quickly!",
   "Misstepping, you cease to dance your tiny jig.",
   "You tap your foot quietly as you play the speedy '%s' upon $p.",
   "$n hums as $s performs the quick dance '%s' upon $p in triple time."},

  {"Warsong", COMP_WARSONG, 25, APPLY_DAMROLL, 1, TRUE, FALSE, 20, 15, 2,
   0,
   "",
   "You feel emblazoned with the power of the warsong!",
   "Your spirit's rage smolders and is finally quenched.",
   "Raising your voice in defiance, you cry out '%s', adding beats with $p.",
   "$n screams out the dissentful '%s' as $s manipulates $p."},

  {"Opera", COMP_OPERA, 25, APPLY_HITROLL, 1, TRUE, FALSE, 20, 15, 0, 0,
   "",
   "The emotion of the opera steels your nerve.",
   "Randomly thinking of something else, the power of the opera leaves your mind.",
   "Your face contorted with emotion, you bellow the operatic '%s' upon $p.",
   "$n peruses a range of emotional fields as $s sings the opera '%s'."},
/*

// Akamai: 10/29/98 -- Ok, I'm proposing some new songs.
// I'll need to decide how this can be used to enhance the
// things that have been taken from bards

  {"Lullaby", COMP_LULLABY, 25, APPLY_HITROLL, 1, TRUE, FALSE, 20, 15, 0,
   "", 
   "The emotion of the opera steels your nerve.", 
   "Randomly thinking of something else, the power of the opera leaves your mind.",
   "Your face contorted with emotion, you bellow the operatic '%s' upon $p.",
   "$n peruses a range of emotional fields as $s sings the opera '%s'."},

  {"Waltz", COMP_WALTZ, 25, APPLY_HITROLL, 1, TRUE, FALSE, 20, 15, 0,
   "", 
   "The emotion of the opera steels your nerve.", 
   "Randomly thinking of something else, the power of the opera leaves your mind.",
   "Your face contorted with emotion, you bellow the operatic '%s' upon $p.",
   "$n peruses a range of emotional fields as $s sings the opera '%s'."},

  {"Choral", COMP_CHORAL, 25, APPLY_HITROLL, 1, TRUE, FALSE, 20, 15, 0,
   "", 
   "The emotion of the opera steels your nerve.", 
   "Randomly thinking of something else, the power of the opera leaves your mind.",
   "Your face contorted with emotion, you bellow the operatic '%s' upon $p.",
   "$n peruses a range of emotional fields as $s sings the opera '%s'."},
   
  {"Medley", COMP_MEDLEY, 25, APPLY_HITROLL, 1, TRUE, FALSE, 20, 15, 0,
   "", 
   "The emotion of the opera steels your nerve.", 
   "Randomly thinking of something else, the power of the opera leaves your mind.",
   "Your face contorted with emotion, you bellow the operatic '%s' upon $p.",
   "$n peruses a range of emotional fields as $s sings the opera '%s'."},

  {"Concerto", COMP_CONCERTO, 25, APPLY_HITROLL, 1, TRUE, FALSE, 20, 15, 0,
   "", 
   "The emotion of the opera steels your nerve.", 
   "Randomly thinking of something else, the power of the opera leaves your mind.",
   "Your face contorted with emotion, you bellow the operatic '%s' upon $p.",
   "$n peruses a range of emotional fields as $s sings the opera '%s'."},

  {"Blues", COMP_BLUES, 25, APPLY_HITROLL, 1, TRUE, FALSE, 20, 15, 0,
   "", 
   "The emotion of the opera steels your nerve.", 
   "Randomly thinking of something else, the power of the opera leaves your mind.",
   "Your face contorted with emotion, you bellow the operatic '%s' upon $p.",
   "$n peruses a range of emotional fields as $s sings the opera '%s'."},

  {"Chant", COMP_CHANT, 25, APPLY_HITROLL, 1, TRUE, FALSE, 20, 15, 0,
   "", 
   "The emotion of the opera steels your nerve.", 
   "Randomly thinking of something else, the power of the opera leaves your mind.",
   "Your face contorted with emotion, you bellow the operatic '%s' upon $p.",
   "$n peruses a range of emotional fields as $s sings the opera '%s'."},

  {"Requiem", COMP_REQUIEM, 25, APPLY_HITROLL, 1, TRUE, FALSE, 20, 15, 0,
   "", 
   "The emotion of the opera steels your nerve.", 
   "Randomly thinking of something else, the power of the opera leaves your mind.",
   "Your face contorted with emotion, you bellow the operatic '%s' upon $p.",
   "$n peruses a range of emotional fields as $s sings the opera '%s'."},

  {"Harmony", COMP_HARMONY, 25, APPLY_HITROLL, 1, TRUE, FALSE, 20, 15, 0,
   "", 
   "The emotion of the opera steels your nerve.", 
   "Randomly thinking of something else, the power of the opera leaves your mind.",
   "Your face contorted with emotion, you bellow the operatic '%s' upon $p.",
   "$n peruses a range of emotional fields as $s sings the opera '%s'."},
*/

  {NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "", "", "", "", ""}
};


void do_songs (CHAR_DATA * ch, char *argument)
{
  char song_list[MAX_LEVEL][MAX_STRING_LENGTH];
  char song_columns[MAX_LEVEL];
  int sn, lev, mana;
  bool found = FALSE;
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC (ch))
    return;

  if (ch->level > LEVEL_HERO)
    {
      for (lev = 0; lev < MAX_LEVEL; lev++)
	{
	  song_columns[lev] = 0;
	  song_list[lev][0] = '\0';
	}

      for (sn = 0; sn < MAX_SKILL; sn++)
	{
	  if (skill_table[sn].name == NULL)
	    break;

	  if (get_skill(ch,sn) > 0
	      && skill_table[sn].type == SKILL_SING)
	    {
	      found = TRUE;
	      lev = level_for_skill (ch, sn);
	      if (ch->level < lev)
		sprintf (buf, "%-21s  n/a      ", skill_table[sn].name);
	      else
		{
		  mana = UMAX (skill_table[sn].min_mana,
			       100 / (2 + ch->level - lev));
		  sprintf (buf, "%-21s  %3d mana  ", skill_table[sn].name,
			   mana);
		}

	      if (song_list[lev][0] == '\0')
		sprintf (song_list[lev], "\n\rLevel %2d: %s", lev, buf);
	      else
		/* append */
		{
		  if (++song_columns[lev] % 2 == 0)
		    strcat (song_list[lev], "\n\r          ");
		  strcat (song_list[lev], buf);
		}
	    }
	}
    }
  else
    {
      for (lev = 0; lev < LEVEL_HERO; lev++)
	{
	  song_columns[lev] = 0;
	  song_list[lev][0] = '\0';
	}

      for (sn = 0; sn < MAX_SKILL; sn++)
	{
	  if (skill_table[sn].name == NULL)
	    break;

	  if (level_for_skill(ch,sn) < LEVEL_HERO
	      && skill_table[sn].type == SKILL_SING
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

	      if (song_list[lev][0] == '\0')
		sprintf (song_list[lev], "\n\rLevel %2d: %s", lev, buf);
	      else
		/* append */
		{
		  if (++song_columns[lev] % 2 == 0)
		    strcat (song_list[lev], "\n\r          ");
		  strcat (song_list[lev], buf);
		}
	    }
	}
    }


  if (!found)
    {
      send_to_char ("You know no songs.\n\r", ch);
      return;
    }

  if (ch->level > LEVEL_HERO)
    {
      for (lev = 0; lev < MAX_LEVEL; lev++)
	if (song_list[lev][0] != '\0')
	  send_to_char (song_list[lev], ch);
    }
  else
    {
      for (lev = 0; lev < LEVEL_HERO; lev++)
	if (song_list[lev][0] != '\0')
	  send_to_char (song_list[lev], ch);
    }

  send_to_char ("\n\r", ch);
}

void do_sing (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  void *vo;
  int mana;
  int sn;
  int target;

  /*  if (IS_NPC(ch) && ch->desc == NULL)
     return;
   */

  if (!has_skills (ch, SKILL_SING) && !IS_NPC (ch))
    {
      send_to_char ("You couldn't sing to save your life.\n\r", ch);
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


  sing_target_name = one_argument (argument, arg1);

  one_argument (sing_target_name, arg2);

  if (arg1[0] == '\0')
    {
      send_to_char ("Sing what? where?\n\r", ch);
      return;
    }


  if ((sn = find_spell (ch, arg1)) < 0
      || (!IS_NPC (ch) && get_skill(ch,sn) < 1))
    {
      send_to_char ("You don't know any songs of that name.\n\r", ch);
      return;
    }

  if (IS_SET(ch->act2,ACT_FAMILIAR) && get_skill(ch,sn) < 1)
    return;

  
  if (skill_table[sn].type != SKILL_SING)
    {
      send_to_char ("You don't know any songs of that name.\n\r", ch);
      return;
    }

  if (ch->position < skill_table[sn].minimum_position
      || (ch->fighting != NULL
	  && skill_table[sn].minimum_position > POS_FIGHTING))
    {
      send_to_char ("You are not prepared to sing.\n\r", ch);
      return;
    }


  if (is_affected (ch, gsn_fear) &&
       (skill_table[sn].target == TAR_CHAR_OFFENSIVE ||
        skill_table[sn].target == TAR_AREA_OFF ||
        skill_table[sn].target == TAR_OBJ_CHAR_OFF))
  {
	  send_to_char ("You are way too scared to sings that song right now.\n\r", ch);
	  return;
  }
  if (is_affected (ch, skill_lookup ("Quiet Mountain Lake")) &&
      (skill_table[sn].target == TAR_CHAR_OFFENSIVE ||
       skill_table[sn].target == TAR_AREA_OFF ||
       skill_table[sn].target == TAR_OBJ_CHAR_OFF))
    {
      send_to_char
	("You are too peaceful to sing that song right now.\n\r", ch);
      return;
    }

  if (ch->level + 2 == level_for_skill (ch, sn))
    mana = 50;
  else
    mana = UMAX (skill_table[sn].min_mana,
		 100 / (2 + ch->level - level_for_skill (ch, sn)));

  if (is_affected (ch, gsn_mirror))
    {
      act ("The mirror arround you is destroyed by your outgoing music.",
	   ch, NULL, NULL, TO_CHAR);
      act ("The mirror around $n is destroyed by $s outgoing music.",
	   ch, NULL, NULL, TO_ROOM);
      affect_strip (ch, gsn_mirror);
    }

  if (IS_AFFECTED (ch, AFF_INVISIBLE))
    {
      affect_strip (ch, gsn_invis);
      affect_strip (ch, gsn_mass_invis);
      affect_strip (ch, gsn_heavenly_cloak);
      affect_strip (ch, gsn_astral_body);
      affect_strip (ch, gsn_vanish);
      REMOVE_BIT (ch->affected_by, AFF_INVISIBLE);
      act ("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
    }

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
      bug ("Do_sing: bad target for sn %d.", sn);
      return;

    case TAR_AREA_OFF:
    case TAR_IGNORE:
      break;

    case TAR_CHAR_OFFENSIVE:
      if (arg2[0] == '\0')
	{
	  if ((victim = ch->fighting) == NULL)
	    {
	      send_to_char ("Who would you like to sing this to?\n\r", ch);
	      return;
	    }
	}
      else
	{
	  if ((victim = get_char_room (ch, sing_target_name)) == NULL)
	    {
	      send_to_char ("They aren't here.\n\r", ch);
	      return;
	    }
	}

      // if it is not a player, and they're safe, don't allow
      if (!IS_NPC (ch))
	{
	  if (is_safe (ch, victim) && victim != ch)
	    {
//               send_to_char("Not on that target.\n\r", ch);
	      return;
	    }
	}
      // if the bard is the master, don't allow them to abuse the charmie
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
	  if ((victim = get_char_room (ch, sing_target_name)) == NULL)
	    {
	      send_to_char ("They aren't here.\n\r", ch);
	      return;
	    }
	}

      vo = (void *) victim;
      target = TARGET_CHAR;
      break;

    case TAR_CHAR_SELF:
      if (arg2[0] != '\0' && !is_name (sing_target_name, ch->name))
	{
	  send_to_char ("You cannot sing this song to another.\n\r", ch);
	  return;
	}

      vo = (void *) ch;
      target = TARGET_CHAR;
      break;

    case TAR_OBJ_INV:
      if (arg2[0] == '\0')
	{
	  send_to_char ("What should the song be sung to?\n\r", ch);
	  return;
	}

      if ((obj = get_obj_carry (ch, sing_target_name)) == NULL)
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
	      send_to_char ("Sing the song to whom or what?\n\r", ch);
	      return;
	    }
	  if (check_shopkeeper_attack (ch, victim))
	    return;

	  target = TARGET_CHAR;
	}
      else if ((victim = get_char_room (ch, sing_target_name)) != NULL)
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
      else if ((obj = get_obj_here (ch, sing_target_name)) != NULL)
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
      else if ((victim = get_char_room (ch, sing_target_name)) != NULL)
	{
	  vo = (void *) victim;
	  target = TARGET_CHAR;
	}
      else if ((obj = get_obj_carry (ch, sing_target_name)) != NULL)
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
    }				// end switch


  if (!IS_NPC (ch) && ch->mana < mana)
    {
      send_to_char ("You don't have enough mana.\n\r", ch);
      return;
    }

  if (skill_table[sn].target == TAR_AREA_OFF ||
      skill_table[sn].target == TAR_CHAR_OFFENSIVE ||
      skill_table[sn].target == TAR_OBJ_CHAR_OFF)
    {
      if (victim != NULL && IS_NPC (victim))
	{
	  if (victim->pIndexData != NULL &&
	      (victim->pIndexData->pShop != NULL ||
	       IS_SET (victim->act, ACT_TRAIN) ||
	       IS_SET (victim->act, ACT_PRACTICE) ||
	       IS_SET (victim->act, ACT_IS_HEALER)))
	    {
	      send_to_char ("Not on that target.\n\r", ch);
	      return;
	    }
	}
    }

  sprintf (buf, "$n sings the tune, '%s'.", skill_table[sn].name);
  act (buf, ch, NULL, NULL, TO_ROOM);

  if (!IS_IMMORTAL (ch))
    WAIT_STATE (ch, skill_table[sn].beats);

  if (number_percent () > get_skill (ch, sn))
    {
      // on a failure deduct only half the mana
      ch->mana -= mana / 2;
      send_to_char ("You lost your concentration.\n\r", ch);
      check_improve (ch, sn, FALSE, 1);
      ch->mana -= mana / 2;
    }
  else
    {
      // on success deduct the full cost and check for improvement
      CHAR_DATA *perp=NULL;
      AFFECT_DATA *af=NULL;
      ch->mana -= mana;
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
	  // dispatch through the function named in the skill table
	  //	  (*skill_table[sn].spell_fun) (sn, ch->level, ch, vo, target);
	}
      else
	{
  	  (*skill_table[sn].spell_fun) (sn, 3 * ch->level / 4, ch, vo, target);
	  // dispatch through the function named in the skill table
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
	  //	  (*skill_table[sn].spell_fun) (sn, 3 * ch->level / 4, ch, vo,target);
	}
      sprintf(buf, "%d", sn);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_USES_SPELL, NULL, victim, OT_TYPE_SONG, buf);
      check_improve (ch, sn, TRUE, 1);
    }

  if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE ||
       (skill_table[sn].target == TAR_OBJ_CHAR_OFF
	&& target == TARGET_CHAR)) && victim != ch && victim->master != ch)
    {

      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      for (vch = ch->in_room->people; vch; vch = vch_next)
	{
	  vch_next = vch->next_in_room;
	  if ((victim == vch) && (victim->fighting == NULL)
	      && (get_position(victim) > POS_SLEEPING))
	    {
	      multi_hit (ch, victim, TYPE_UNDEFINED);
	      break;
	    }
	}
    }

  return;
}

int
play_composition (CHAR_DATA * ch, char *comp_name, char *target_name,
		  OBJ_DATA * iobj)
{
  COMP_DATA *comp;
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  AFFECT_DATA af, *paf;
  int modby;

  for (comp = ch->compositions; comp; comp = comp->next)
    {
      if (!str_cmp (comp->name, comp_name))
	{
	  if (comp_table[comp->type].area == FALSE)
	    {

	      if (target_name[0] == '\0')
		{
		  send_to_char ("This composition requires a target.\n\r",
				ch);
		  return (0);
		}

	      victim = get_char_room (ch, target_name);
	      if (victim == NULL)
		{
		  send_to_char ("They aren't here.\n\r", ch);
		  return (0);
		}

	      if (comp_table[comp->type].hostile && ch == victim)
		{
		  send_to_char
		    ("You can't play this composition at yourself.\n\r", ch);
		  return (0);
		}

	      if (IS_NPC (victim) && (victim->pIndexData->pShop != NULL ||
				      IS_SET (victim->act, ACT_TRAIN)
				      || IS_SET (victim->act, ACT_PRACTICE)
				      || IS_SET (victim->act,
						 ACT_IS_HEALER))
		  && comp_table[comp->type].hostile)
		{
		  send_to_char ("Not on that target.\n\r", ch);
		  return (0);
		}
	      sprintf (buf, comp_table[comp->type].play_str_other,
		       comp->title);
	      act (buf, ch, iobj, NULL, TO_ROOM);

	      sprintf (buf, comp_table[comp->type].play_str_ch, comp->title);
	      act (buf, ch, iobj, NULL, TO_CHAR);

	      for (paf = victim->affected; paf != NULL; paf = paf->next)
		{
		  if (paf->composition &&
		      paf->location == comp_table[comp->type].apply)
		    {
		      act
			("$N is already affected by this type of composition.",
			 ch, NULL, victim, TO_CHAR);
		      return (0);
		    }
		}

	      if (ch->mana < comp_table[comp->type].mana)
		{
		  send_to_char ("You don't have enough mana.\n\r", ch);
		  return (0);
		}

	      ch->mana -= comp_table[comp->type].mana;

	      af.where = TO_AFFECTS;
	      af.type = comp->type;
	      af.level = comp->skill;
	      af.duration = comp_table[comp->type].duration
		- ((100 - comp->skill) % comp_table[comp->type].duration) + 4;
	      af.location = comp_table[comp->type].apply;

	      switch (comp->type)
		{
		default:
		  modby = comp_table[comp->type].modify_by;
		  break;

		case COMP_WARSONG:
		case COMP_BATTLESONG:
		case COMP_OPERA:
		  modby = ch->level / 13.0 + .1;
		  modby = UMAX (1, modby);
		  break;

		case COMP_SERENADE:
		  modby = -1 * (ch->level / 20);
		  modby = UMIN (-1, modby);
		  break;
		}

	      af.modifier = modby;
	      af.bitvector = 0;
	      af.permaff = FALSE;
	      af.composition = TRUE;
	      af.comp_name = str_dup (comp->title);

	      affect_to_char (victim, &af);
	      check_improve (ch, gsn_instruments, TRUE, 1);
	      check_improve (ch, gsn_compose, TRUE, 1);

	      act (comp_table[comp->type].vict_string, ch, NULL, victim,
		   TO_VICT);

	      if (ch != victim)
		act (comp_table[comp->type].char_string, ch, NULL,
		     victim, TO_CHAR);

	      if (comp_table[comp->type].hostile)
		multi_hit (victim, ch, TYPE_UNDEFINED);

	      if (comp_table[comp->type].base_damage != 0)
		{
		  int dam;

		  dam = (comp_table[comp->type].base_damage * ch->level);
		  dam += number_range ((0 - ch->level) / 4, ch->level / 4);

		  damage (ch, victim, dam, 0, DAM_OTHER, FALSE);
		}

	      if (IS_NPC (victim) && !comp_table[comp->type].hostile)
		{
		  if (get_skill (ch, gsn_influence) > number_percent ())
		    {
		      int coins;
		      char cbuf[MAX_STRING_LENGTH];

		      coins = number_range (0, ch->level) / 2;
		      coins = UMIN (coins, victim->silver);
		      if (coins > 0)
			{

			  act
			    ("$N smiles softly, enthralled by your sonorous protection of the tune $t.",
			     ch, comp->title, victim, TO_CHAR);
			  act
			    ("$N smiles softly, enthralled by $n's sonorous protection of the tune $t.",
			     ch, comp->title, victim, TO_NOTVICT);
			  act
			    ("You smile softly, enthralled by $n's sonorous protection of the tune $t.",
			     ch, comp->title, victim, TO_VICT);
			  sprintf (cbuf, "%d silver %s", coins, NAME (ch));
			  do_give (victim, cbuf);
			}
		    }
		}

	      return (0);
	    }
	  else
	    {

	      if (ch->mana < comp_table[comp->type].mana)
		{
		  send_to_char ("You don't have enough mana.\n\r", ch);
		  return (0);
		}

	      af.where = TO_AFFECTS;
	      af.type = comp->type;
	      af.level = comp->skill;
	      af.duration = comp_table[comp->type].duration
		- ((100 - comp->skill) % comp_table[comp->type].duration) + 4;
	      af.location = comp_table[comp->type].apply;

	      switch (comp->type)
		{
		default:
		  modby = comp_table[comp->type].modify_by;
		  break;

		case COMP_WARSONG:
		case COMP_BATTLESONG:
		case COMP_OPERA:
		  modby = ch->level / 10;
		  modby = UMAX (1, modby);
		  break;

		case COMP_SERENADE:
		  modby = -1 * (ch->level / 20);
		  modby = UMIN (-1, modby);
		  break;
		}

	      af.modifier = modby;
	      af.bitvector = 0;
	      af.permaff = FALSE;
	      af.composition = TRUE;
	      af.comp_name = str_dup (comp->title);

	      ch->mana -= comp_table[comp->type].mana;

	      sprintf (buf,
		       "$n peforms the original composition '%s' upon $p.",
		       comp->title);
	      act (buf, ch, iobj, NULL, TO_ROOM);

	      sprintf (buf,
		       "You peform the original composition '%s' upon $p.",
		       comp->title);
	      act (buf, ch, iobj, NULL, TO_CHAR);

	      check_improve (ch, gsn_instruments, TRUE, 1);
	      check_improve (ch, gsn_compose, TRUE, 1);

	      for (victim = ch->in_room->people; victim;
		   victim = victim->next_in_room)
		{
		  bool noaff;

		  noaff = FALSE;

		  /*              if(victim == ch)
		     continue; */

		  for (paf = victim->affected; paf != NULL; paf = paf->next)
		    {
		      if (paf->composition
			  && paf->location == comp_table[comp->type].apply)
			{
			  noaff = TRUE;
			  break;
			}
		    }

		  if (noaff)
		    continue;

		  if (comp_table[comp->type].hostile)
		    {
		      if (victim != ch && !is_safe_spell (ch, victim, FALSE))
			set_fighting (victim, ch);
		    }

		  affect_to_char (victim, &af);

		  act (comp_table[comp->type].vict_string, ch, NULL,
		       victim, TO_VICT);

		  if (IS_NPC (victim) && !comp_table[comp->type].hostile)
		    {
		      if (get_skill (ch, gsn_influence) > number_percent ())
			{
			  int coins;
			  char cbuf[MAX_STRING_LENGTH];

			  coins = number_range (0, ch->level) / 2;
			  coins = UMIN (coins, victim->silver);
			  if (coins > 0)
			    {
			      act
				("$N smiles softly, enthralled by your sonorous protection of the tune $t.",
				 ch, comp->title, victim, TO_CHAR);
			      act
				("$N smiles softly, enthralled by $n's sonorous protection of the tune $t.",
				 ch, comp->title, victim, TO_NOTVICT);
			      act
				("You smile softly, enthralled by $n's sonorous protection of the tune $t.",
				 ch, comp->title, victim, TO_VICT);

			      sprintf (cbuf, "%d silver %s",
				       coins, NAME (ch));
			      do_give (victim, cbuf);
			    }
			}
		    }
		}
	      return (0);
	    }
	}
    }
  return (-1);
}

void do_repertoire (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  COMP_DATA *comp;
  int comps = 0;


  for (comp = ch->compositions; comp; comp = comp->next)
    {
      sprintf (buf, "Name: %s\tType: %s\t\t", comp->name,
	       comp_string (comp->type));
      sprintf (buf + strlen (buf), "Title: %s\n\r", comp->title);
      send_to_char (buf, ch);
      comps++;
    }

  sprintf (buf,
	   "You have created %d composition%s out of a maximum of %d.\n\r",
	   comps, (comps == 1) ? "" : "s", max_compositions (ch));
  send_to_char (buf, ch);
}

void do_compose (CHAR_DATA * ch, char *argument)
{
  char namestr[MAX_INPUT_LENGTH];
  char typestr[MAX_INPUT_LENGTH];
  COMP_DATA *comp, *comp_next, *prev = NULL;
  char buf[MAX_STRING_LENGTH];
  int type, comps = 0;

  if (get_skill (ch, gsn_compose) <= 0)
    {
      send_to_char ("You know nothing about composing.\n\r", ch);
      return;
    }

  argument = one_argument (argument, namestr);

  if (namestr[0] == '\0')
    {
      send_to_char
	("Syntax: compose <name>                 (To remove a composition)\n\r",
	 ch);
      send_to_char
	("Syntax: compose <name> <type> <title>  (To create a composition)\n\r",
	 ch);
      return;
    }

  argument = one_argument (argument, typestr);

  if (typestr[0] == '\0')
    {
      for (comp = ch->compositions; comp; comp = comp_next)
	{
	  comp_next = comp->next;

	  if (!str_cmp (comp->name, namestr))
	    {
	      sprintf (buf,
		       "Removing composition %s from your repertoire.\n\r",
		       namestr);
	      send_to_char (buf, ch);

	      if (prev)
		{
		  prev->next = comp->next;
		  free_comp (comp);
		}
	      else
		{
		  ch->compositions = comp->next;
		  free_comp (comp);
		}

	      return;
	    }

	  prev = comp;
	}

      send_to_char
	("You have no such composition in your repertoire.\n\r", ch);
      return;
    }

  if (argument[0] == '\0')
    {
      send_to_char
	("Syntax: compose <name>                 (To remove a composition)\n\r",
	 ch);
      send_to_char
	("Syntax: compose <name> <type> <title>  (To create a composition)\n\r",
	 ch);
      return;
    }

  if ((type = comp_type (typestr)) == 0)
    {
      send_to_char ("Unknown composition type.\n\r", ch);
      return;
    }

  smash_tilde (namestr);


  if (skill_lookup (namestr) != -1)
    {
      send_to_char
	("Sorry, you cannot use that name for your composition.\n\r", ch);
      return;
    }

  comps = 0;
  for (comp = ch->compositions; comp; comp = comp->next)
    {
      if (!str_cmp (comp->name, namestr))
	{
	  send_to_char
	    ("You already have a composition by that name.\n\r", ch);
	  return;
	}
      comps++;
    }

  if (comps >= max_compositions (ch))
    {
      send_to_char ("Your repertoire is already at maximum.\n\r", ch);
      return;
    }

  if (ch->mana < comp_table[type].mana)
    {
      send_to_char ("You don't have enough mana.\n\r", ch);
      return;
    }

  if (ch->level < comp_table[type].level)
    {
      send_to_char
	("You aren't high enough level to compose such a song.\n\r", ch);
      return;
    }

  ch->mana -= comp_table[type].mana;

  comp = new_comp ();
  comp->name = str_dup (namestr);
  comp->title = str_dup (argument);
  smash_tilde (comp->title);
  comp->type = type;
  comp->skill = ch->level;
  // push the new composition onto the front of the list
  comp->next = ch->compositions;
  ch->compositions = comp;

  send_to_char ("Composition added to your repertoire.\n\r", ch);
}

int max_compositions (CHAR_DATA * ch)
{
  int skill_per;

  skill_per = get_skill (ch, gsn_compose);

  if (skill_per == 0)
    return (0);

  if (skill_per < 10)
    return (1);
  else if (skill_per < 20)
    return (2);
  else if (skill_per < 30)
    return (3);
  else if (skill_per < 40)
    return (4);
  else if (skill_per < 50)
    return (5);
  else if (skill_per < 60)
    return (6);
  else if (skill_per < 70)
    return (7);
  else if (skill_per < 80)
    return (8);
  else if (skill_per < 90)
    return (9);
  else
    return (10);
}

char *comp_string (int comp_type)
{
  int index;

  for (index = 1; comp_table[index].name != NULL; index++)
    {
      if (comp_type == comp_table[index].type)
	{
	  return (comp_table[index].name);
	}
    }
  return ("Unknown");
}


int comp_type (char *comp_str)
{
  int index;

  for (index = 1; comp_table[index].name != NULL; index++)
    {
      if (!str_cmp (comp_table[index].name, comp_str))
	{
	  return (comp_table[index].type);
	}
    }
  return (0);
}

void do_play (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  void *vo;
  int mana = 0;
  int level_diff = 0;
  int sn = 0;
  int target = 0;
  int bonus = 0;
  OBJ_DATA *iobj;

  if (get_skill (ch, gsn_instruments) <= 0)
    {
      send_to_char
	("You couldn't play an instrument to save your life.\n\r", ch);
      return;
    }

  if (is_affected (ch, gsn_entangle))
    {
      send_to_char ("You can't move your arms away from your sides.\n\r", ch);
      return;
    }

  iobj = get_eq_char (ch, WEAR_HOLD);

  if (iobj == NULL || iobj->item_type != ITEM_INSTRUMENT)
    {
      send_to_char
	("You must be holding an instrument in order to play a song.\n\r",
	 ch);
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

  sing_target_name = one_argument (argument, arg1);

  one_argument (sing_target_name, arg2);

  if (arg1[0] == '\0')
    {
      send_to_char ("Play what? where?\n\r", ch);
      return;
    }
  // check that this character actually has this song 
  sn = find_spell (ch, arg1);
  if ((sn < 0) ||
      (!IS_NPC (ch) && get_skill(ch,sn) < 1))
    {
      // ok, they don't know a 'natural' song, do they have private
      // composition of their own with this name?
      if (play_composition (ch, arg1, sing_target_name, iobj) == -1)
	{
	  send_to_char ("You don't know any songs of that name.\n\r", ch);
	}
      return;
    }

  if (skill_table[sn].type != SKILL_SING)
    {
      send_to_char ("You don't know any songs of that name.\n\r", ch);
      return;
    }
  // Akamai 6/16/99 - Play/Sing fix preparation. It use to be that
  // 'play' was prevented at the command dispatch level (when the
  // char was fighting). By putting control in this procedure it is
  // now possible to switch as necessary. Also, it would be possible
  // to modify the effectivness of play based on the use of specific
  // instrument use.
  if (ch->position == POS_FIGHTING || ch->fighting != NULL)
    {
      send_to_char ("No Way!  You are still fighting.\n\r", ch);
      return;
    }


  if (ch->position < skill_table[sn].minimum_position
      || (ch->fighting != NULL
	  && skill_table[sn].minimum_position > POS_FIGHTING))
    {
      send_to_char ("You are not prepared to play your instrument.\n\r", ch);
      return;
    }

  if (is_affected (ch, gsn_fear) &&
       (skill_table[sn].target == TAR_CHAR_OFFENSIVE ||
        skill_table[sn].target == TAR_AREA_OFF ||
        skill_table[sn].target == TAR_OBJ_CHAR_OFF))
    {
      send_to_char ("You are way too scared to play that song right now.\n\r", ch);
      return;
    }

  if (is_affected (ch, skill_lookup ("Quiet Mountain Lake"))
      &&
      (skill_table[sn].target == TAR_CHAR_OFFENSIVE ||
       skill_table[sn].target == TAR_AREA_OFF ||
       skill_table[sn].target == TAR_OBJ_CHAR_OFF))
    {
      send_to_char
	("You are too peaceful to play that song right now.\n\r", ch);
      return;
    }

  if (ch->level + 2 == level_for_skill (ch, sn))
    {
      mana = 50;
    }
  else
    {
      level_diff = (2 + ch->level - level_for_skill (ch, sn));
      mana = UMAX (skill_table[sn].min_mana, (100 / level_diff));
    }
  if (is_affected (ch, gsn_mirror))
    {
      act ("The mirror arround you is destroyed by your outgoing music.",
	   ch, NULL, NULL, TO_CHAR);
      act ("The mirror around $n is destroyed by $s outgoing music.",
	   ch, NULL, NULL, TO_ROOM);
      affect_strip (ch, gsn_mirror);
    }
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
      bug ("Do_play: bad target for sn %d.", sn);
      return;

    case TAR_AREA_OFF:
    case TAR_IGNORE:
      break;

    case TAR_CHAR_OFFENSIVE:
      if (arg2[0] == '\0')
	{
	  if ((victim = ch->fighting) == NULL)
	    {
	      send_to_char ("Who would you like to play this to?\n\r", ch);
	      return;
	    }
	}
      else
	{
	  if ((victim = get_char_room (ch, sing_target_name)) == NULL)
	    {
	      send_to_char ("They aren't here.\n\r", ch);
	      return;
	    }
	}

      if (!IS_NPC (ch))
	{

	  if (is_safe (ch, victim) && victim != ch)
	    {
	      send_to_char ("Not on that target.\n\r", ch);
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
	  if ((victim = get_char_room (ch, sing_target_name)) == NULL)
	    {
	      send_to_char ("They aren't here.\n\r", ch);
	      return;
	    }
	}

      vo = (void *) victim;
      target = TARGET_CHAR;
      break;

    case TAR_CHAR_SELF:
      if (arg2[0] != '\0' && !is_name (sing_target_name, ch->name))
	{
	  send_to_char ("You cannot play this song to another.\n\r", ch);
	  return;
	}

      vo = (void *) ch;
      target = TARGET_CHAR;
      break;

    case TAR_OBJ_INV:
      if (arg2[0] == '\0')
	{
	  send_to_char ("What should the song be played to?\n\r", ch);
	  return;
	}

      if ((obj = get_obj_carry (ch, sing_target_name)) == NULL)
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
	      send_to_char ("Play the song to whom or what?\n\r", ch);
	      return;
	    }

	  target = TARGET_CHAR;
	}
      else if ((victim = get_char_room (ch, sing_target_name)) != NULL)
	{
	  target = TARGET_CHAR;
	}

      if (target == TARGET_CHAR)
	{			/* check the sanity of the attack */
	  if (is_safe_spell (ch, victim, FALSE) && victim != ch)
	    {
	      send_to_char ("Not on that target.\n\r", ch);
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
      else if ((obj = get_obj_here (ch, sing_target_name)) != NULL)
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
      else if ((victim = get_char_room (ch, sing_target_name)) != NULL)
	{
	  vo = (void *) victim;
	  target = TARGET_CHAR;
	}
      else if ((obj = get_obj_carry (ch, sing_target_name)) != NULL)
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

  if (skill_table[sn].target == TAR_AREA_OFF ||
      skill_table[sn].target == TAR_CHAR_OFFENSIVE ||
      skill_table[sn].target == TAR_OBJ_CHAR_OFF)
    {
      if (victim != NULL && IS_NPC (victim))
	{
	  if (victim->pIndexData != NULL &&
	      (victim->pIndexData->pShop != NULL ||
	       IS_SET (victim->act, ACT_TRAIN) ||
	       IS_SET (victim->act, ACT_PRACTICE) ||
	       IS_SET (victim->act, ACT_IS_HEALER)))
	    {
	      send_to_char ("Not on that target.\n\r", ch);
	      return;
	    }
	}
    }

  sprintf (buf, "$n plays the tune, '%s' upon $p.", skill_table[sn].name);
  act (buf, ch, iobj, NULL, TO_ROOM);

  if (!IS_IMMORTAL (ch))
    WAIT_STATE (ch, skill_table[sn].beats);

  if (number_percent () > get_skill (ch, sn))
    {
      send_to_char ("You lost your concentration.\n\r", ch);
      check_improve (ch, sn, FALSE, 1);
      check_improve (ch, gsn_instruments, FALSE, 1);
      check_improve (ch, gsn_compose, FALSE, 1);
      ch->mana -= mana / 2;
    }
  else
    {
      ch->mana -= mana;
      if (number_percent () > get_skill (ch, gsn_instruments))
	{
	  send_to_char
	    ("Your inexperience with instruments causes you to fumble a few notes.\n\r",
	     ch);
	  bonus = 0 - iobj->level;
	}
      else
	bonus = iobj->level;

      if (bonus < 0)
	bonus = 0;

      if (IS_NPC (ch) || mana_using_Class (ch))
	/* Class has spells */
	(*skill_table[sn].spell_fun) (sn, ch->level + bonus, ch, vo, target);
      else
	(*skill_table[sn].spell_fun) (sn, (3 * ch->level / 4) + bonus,
				      ch, vo, target);
     
      sprintf(buf, "%d", sn);
      trip_triggers_arg(ch, OBJ_TRIG_CHAR_USES_SPELL, NULL, victim, OT_TYPE_SONG, buf);
      
      check_improve (ch, sn, TRUE, 1);
      check_improve (ch, gsn_instruments, TRUE, 2);
      check_improve (ch, gsn_compose, TRUE, 1);
    }

  if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
       || (skill_table[sn].target == TAR_OBJ_CHAR_OFF
	   && target == TARGET_CHAR)) && victim != ch && victim->master != ch)
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;
      update_aggressor(ch,victim);
      for (vch = ch->in_room->people; vch; vch = vch_next)
	{
	  vch_next = vch->next_in_room;
	  if (victim == vch && victim->fighting == NULL)
	    {
	      multi_hit (victim, ch, TYPE_UNDEFINED);
	      break;
	    }
	}
    }

  return;
}



// ----------------------------------------------------------------------
//
// The default Bard Songs
//
// ----------------------------------------------------------------------


void song_elyteness (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = NULL;
  int dam = 0;

  victim = (CHAR_DATA *) vo;
  dam = dice (level, 12);
  dam = dam * 2 / 3;
  if (saves_spell (level, victim, DAM_SOUND))
    {
      dam /= 2;
    }
  damage_old (ch, victim, dam, sn, DAM_SOUND, TRUE);
  //update_aggressor(ch,victim);
  return;
}

void
song_shattereddream (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = NULL;
  int dam = 0;

  victim = (CHAR_DATA *) vo;
  dam = dice (level, 12);
  dam = dam * 4 / 3;


  if (is_affected (victim, gsn_mirror))
    {
      if (number_percent () < 25)
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
	}
      else
	{
	  act
	    ("The mirror around $N absorbs the high pitched sounds from your song.",
	     ch, NULL, victim, TO_CHAR);
	  act
	    ("The mirror around $N absorbs the high pitched sounds from $n's song.",
	     ch, NULL, victim, TO_NOTVICT);
	  act
	    ("The mirror around you absorbs the high pitched sounds from $n's song.",
	     ch, NULL, victim, TO_VICT);
	  return;
	}

    }

  if (saves_spell (level, victim, DAM_SOUND))
    {
      dam /= 2;
    }
  damage_old (ch, victim, dam, sn, DAM_SOUND, TRUE);
  //update_aggressor(ch,victim);
  return;
}


void song_disperse (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = NULL;
  CHAR_DATA *gch = NULL;
  char tcbuf[MAX_STRING_LENGTH];

  victim = (CHAR_DATA *) vo;
  sprintf (tcbuf, "You are dispersed from %s's group.\n\r",
	   capitalize (victim->name));

  if (!saves_spell (level, victim, DAM_MENTAL))
    {
      if (victim->leader == NULL || victim->leader == victim)
	{
	  for (gch = char_list; gch != NULL; gch = gch->next)
	    {
	      if (is_same_group (gch, victim))
		{
		  if (gch != ch && gch != victim)
		    {
		      gch->leader = NULL;
		      send_to_char (tcbuf, gch);
		    }
		}
	    }
	  send_to_char ("Your group has been dispersed!\n\r", victim);
	  act ("$N's group has been dispersed.", ch, NULL, victim, TO_CHAR);
	}
      else
	{
	  victim->leader = NULL;
	  send_to_char ("You are dispersed from your group!\n\r", victim);
	  act ("$N has been dispersed from $s group.", ch, NULL, victim,
	       TO_CHAR);
	  send_to_char ("You are dispersed from your group!\n\r", victim);
	}
    }
  return;
}


void song_scatter (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = NULL;
  CHAR_DATA *gch = NULL;
  char tcbuf[MAX_STRING_LENGTH];
  ROOM_INDEX_DATA *pRoomIndex;
  int savet;

  if (nogate || (!IS_NPC(ch) && ch->pcdata->nogate))
	  
  {
    send_to_char("The gods are not allowing that at the moment.\n\r",ch);
    return;
  }

  victim = (CHAR_DATA *) vo;

   if (victim->in_room == NULL
       || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL))
     {
      send_to_char ("You failed.\n\r", ch);
      return;
     }

  sprintf (tcbuf, "As a member of %s's group, you are scattered!\n\r",
	   capitalize (victim->name));

  switch (get_curr_stat (ch, STAT_CHA))
    {
    case 25:
    case 24:
      savet = 8;
      break;
    case 23:
    case 22:
      savet = 6;
      break;
    case 21:
    case 20:
      savet = 4;
      break;
    case 19:
    case 18:
      savet = 2;
      break;
    default:
      savet = 0;
      break;
    }

  if (!saves_spell (level + 20 - savet, victim, DAM_MENTAL))
    {
      for (gch = char_list; gch != NULL; gch = gch->next)
	if (is_same_group (gch, victim))
	  if (gch != ch && gch != victim && gch->in_room == ch->in_room)
	    {
	      pRoomIndex = get_random_room (gch);

	      while (IS_SET (pRoomIndex->room_flags, ROOM_ARENA) ||
		     pRoomIndex->area->construct ||
		     pRoomIndex->sector_type == SECT_WATER_OCEAN ||
		     IS_SET (pRoomIndex->race_flags, ROOM_NOTRANSPORT)
		     || IS_SET (pRoomIndex->race_flags, ROOM_NOTELEPORT))
		pRoomIndex = get_random_room (gch);

	      send_to_char (tcbuf, gch);
	      act ("$n disappears to one of the four corners of the world.",
		   gch, NULL, NULL, TO_ROOM);
	      char_from_room (gch);
	      if (gch->mount != NULL)
		char_from_room (gch->mount);
	      char_to_room (gch, pRoomIndex);
	      if (gch->mount != NULL)
		char_to_room (gch->mount, pRoomIndex);
	      act ("$n slowly fades into existence.", gch, NULL, NULL,
		   TO_ROOM);
	      do_look (gch, "auto");
	    }

      send_to_char ("Your group has been scattered!\n\r", victim);
    }

  return;
}


void
song_falkentynes_fury (int sn, int level, CHAR_DATA * ch, void *vo,
		       int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected (victim, sn) || IS_AFFECTED (victim, AFF_BERSERK))
    {
      if (victim == ch)
	send_to_char ("You are already as inspired as you can get.\n\r", ch);
      else
	act ("$N is already possessed with a fury.", ch, NULL, victim,
	     TO_CHAR);
      return;
    }

  if (is_affected (victim, skill_lookup ("calm")))
    {
      if (victim == ch)
	send_to_char ("Why don't you just relax for a while?\n\r", ch);
      else
	act ("$N doesn't look like $e has any inspiration.",
	     ch, NULL, victim, TO_CHAR);
      return;
    }


  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 4;
  if (is_affected (victim, skill_lookup ("frenzy")))
    af.modifier = level / 14;
  else
    af.modifier = level / 8;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.location = APPLY_DAMROLL;
  af.composition = FALSE;
  af.comp_name = str_dup ("");
  affect_to_char (victim, &af);

  af.location = APPLY_STR;
  af.modifier = 1 + (level >= 30) + (level >= 65);
  affect_to_char (victim, &af);

  if (is_affected (victim, skill_lookup ("frenzy")))
    af.modifier = 5 * (level / 10);
  else
    af.modifier = 10 * (level / 10);
  af.location = APPLY_AC;
  affect_to_char (victim, &af);

  send_to_char
    ("The rage of Falkentyne fills you and curdles your blood.\n\r", victim);
  act ("The howl of Falkentyne fills the air.", victim, NULL, NULL, TO_ROOM);
}

void
song_walking_on_clouds (int sn, int level, CHAR_DATA * ch, void *vo,
			int target)
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
	send_to_char
	  ("You are already have the ability to walk amongst the clouds.\n\r",
	   ch);
      else
	act ("$N is already floating mid-air.", ch, NULL, victim, TO_CHAR);
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
  send_to_char ("Your step becomes lighter and your spirit soars.\n\r",
		victim);
  act ("$n's spirit soars to the sky upon hearing this lighthearted melody.",
       victim, NULL, NULL, TO_ROOM);
  return;
}

void
song_good_vibrations (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  send_to_char ("The ground rocks and rolls beneath your feet!\n\r", ch);
  act ("$n's cacophony makes the ground rock and roll beneath your feet.",
       ch, NULL, NULL, TO_ROOM);

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
	      if (IS_AFFECTED (vch, AFF_FLYING) ||
		  (vch->mount != NULL
		   && IS_AFFECTED (vch->mount, AFF_FLYING)))
		act ("$n does not get hit.", ch, NULL, vch, TO_CHAR);
	      damage_old (ch, vch, 0, sn, DAM_BASH, TRUE);
	    }
	  else
	    {
	      act ("$n gets hit.", ch, NULL, vch, TO_CHAR);
	      vch->position = POS_RESTING;
	      damage_old (ch, vch, level + dice (2, 8), sn, DAM_BASH, TRUE);
	    }
	  continue;
	}

      if (vch->in_room->area == ch->in_room->area)
	send_to_char ("The ground hums violently beneath you.\n\r", vch);
    }
}

void song_rip (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  AFFECT_DATA af;

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 12;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");

  // Akamai: 10/29/98 -- This fixes a bug with bard charm so that
  // immortals can no longer be slept. As well this implements
  // level restrictions so that sleep can only work on mobs/chars
  // with in MAGIC_LEVEL_DIFF of the Bard singing the song

  for (vch = char_list; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next;

      // if the victim is not in the same room, go to the next char in list
      if (vch->in_room == NULL)
	continue;

      // if the char is immortal, next character
      if (IS_IMMORTAL (vch))
	continue;

      // if the victim is a real player +- MAGIC_LEVEL_DIFF from the level
      // of the bard then they can't be affected
      if (!IS_NPC (vch) &&
	  ((vch->level >= (ch->level + MAGIC_LEVEL_DIFF)) ||
	   (vch->level <= (ch->level - MAGIC_LEVEL_DIFF))))
	continue;

      // if the victim is a MOB + MAGIC_LEVEL_DIFF from the level of the
      // bard then the MOB is beyond their sleeping ability
      if (IS_NPC (vch) && (vch->level > (ch->level + MAGIC_LEVEL_DIFF)))
	continue;

      // if the vic is in the same room, not in the same group
      if ((vch->in_room == ch->in_room) &&
	  !is_same_group (ch, vch) && (vch != ch))
	{

	  // if the character is protected from the spell next character
	  if (is_safe_spell (ch, vch, TRUE))
	    continue;

	  // if the char is already affected then, next character
	  if (is_affected (vch, skill_lookup ("sleep")) ||
	      is_affected (vch, skill_lookup ("Ballad of Ole Rip")))
	    {
	      act ("$N is already fast asleep.", ch, NULL, vch, TO_CHAR);
	      continue;
	    }
	  // if the char is already affected by a resistant song, next char
	  if (is_affected (vch, skill_lookup ("Dance of Shadows")))
	    {
	      act ("$N's fear of nightmares is too strong.",
		   ch, NULL, vch, TO_CHAR);
	      continue;
	    }

	   if (is_affected (vch, gsn_fear))
	   {
	     act ("$N's fear of everything prevents $S from sleeping right now.",ch,NULL,vch,TO_CHAR);
	     continue;
	   }

	   if (IS_SET(vch->imm_flags, IMM_SOUND) || IS_SET(vch->imm_flags, IMM_SLEEP)
			   || IS_SET(vch->act,ACT_UNDEAD))
             continue;

	  act
	    ("$n's relaxing lullaby soothes you into a peaceful rest ..... zzzzzz.",
	     ch, NULL, vch, TO_VICT);
	  act ("$n goes to sleep.", vch, NULL, NULL, TO_ROOM);
	  vch->position = POS_SLEEPING;
	  affect_join (vch, &af);
	}
    }
  return;
}

void song_dance_shad (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  static const sh_int dam_each[] = {
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    30, 35, 40, 45, 50, 55, 55, 55, 56, 57,
    58, 58, 59, 60, 61, 61, 62, 63, 64, 64,
    65, 66, 67, 67, 68, 69, 70, 70, 71, 72,
    73, 73, 74, 75, 76, 76, 77, 78, 79, 79
  };
  int dam;


  if (is_affected (victim, skill_lookup ("Dance of Shadows")))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }

  if (get_position(victim) > POS_SLEEPING &&
      saves_spell (victim->level + 2, victim, DAM_MENTAL))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }

  if (get_position(victim) == POS_SLEEPING)
    {
      level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
      level = UMAX (0, level);
      dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
      damage_old (ch, victim, dam, sn, DAM_MENTAL, TRUE);
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 12;

  if (ch->level <= 45)
    af.duration = 3;
  else if (ch->level <= 50)
    af.duration = 4;
  else if (ch->level <= 55)
    af.duration = 5;
  else if (ch->level <= 60)
    af.duration = 6;
  else if (ch->level <= 65)
    af.duration = 7;
  else if (ch->level <= 70)
    af.duration = 8;
  else if (ch->level <= 75)
    af.duration = 9;
  else if (ch->level <= 80)
    af.duration = 10;
  else if (ch->level <= 85)
    af.duration = 11;

  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = 0;
  af.permaff = FALSE;

  af.composition = FALSE;
  af.comp_name = str_dup ("");

  affect_to_char (victim, &af);

  send_to_char ("You are afraid to sleep.\n\r", victim);

  act ("The melancholy 'Dance of Shadows' fills the air.", ch, NULL,
       victim, TO_NOTVICT);

  if (ch != victim)
    act ("$N's will have nightmares.", ch, NULL, victim, TO_CHAR);

  return;
}

void song_orpheus (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  char tcbuf[MAX_STRING_LENGTH];
  AFFECT_DATA af;
  int hand, statc, savet;
  OBJ_DATA *obj, *obj2;

  statc = get_curr_stat (victim, STAT_CHA) + get_curr_stat (victim, STAT_INT);

  if (statc > 47)
    savet = 2;
  else if (statc > 40)
    savet = 1;
  else if (statc > 32)
    savet = 0;
  else if (statc > 28)
    savet = -1;
  else
    savet = -2;

  if (is_affected (victim, skill_lookup ("Despair of Orpheus"))
      || saves_spell (level + savet, victim, DAM_MENTAL)
      || IS_SET(victim->imm_flags, IMM_SOUND) 
      || IS_IMMORTAL(ch))
    {
      act ("$N seems unaffected by your song.", ch, NULL, victim, TO_CHAR);
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 3;
  af.location = APPLY_HITROLL;
  af.modifier = -1 * (level / 8);
  af.bitvector = 0;
  af.permaff = FALSE;
  af.composition = FALSE;
  af.comp_name = str_dup ("");

  affect_to_char (victim, &af);

  af.location = APPLY_SAVING_SPELL;
  af.modifier = level / 14;

  affect_to_char (victim, &af);

  send_to_char ("You feel your knees quiver with despair.\n\r", victim);
  act ("The plantive wail of Orpheus fills the air.", ch, NULL, victim,
       TO_NOTVICT);
  if (ch != victim)
    act ("$N's knees quiver with despair.", ch, NULL, victim, TO_CHAR);

  update_aggressor (ch, victim);
  victim->move -= number_range (victim->move / 4, victim->move / 2);

  if (!IS_NPC (victim))
    hand = victim->pcdata->primary_hand;
  else
    hand = HAND_RIGHT;

  switch (hand)
    {
    default:
      obj = get_eq_char (victim, WEAR_WIELD_R);
      obj2 = get_eq_char (victim, WEAR_WIELD_L);
      break;
    case HAND_LEFT:
      obj = get_eq_char (victim, WEAR_WIELD_L);
      obj2 = get_eq_char (victim, WEAR_WIELD_R);
      break;
    }

  if (obj != NULL && can_drop_obj (victim, obj))
    {
      obj_from_char (obj);
      obj_to_room (obj, victim->in_room);
      sprintf (tcbuf, "You drop %s.\n\r", obj->short_descr);
      send_to_char (tcbuf, victim);
      sprintf (tcbuf, "$n drops %s.", obj->short_descr);
      act (tcbuf, victim, NULL, NULL, TO_ROOM);
      if (victim && !IS_NPC(victim))
        save_char_obj (victim);
    }
  else if (obj2 != NULL && can_drop_obj (victim, obj2))
    {
      obj_from_char (obj2);
      obj_to_room (obj2, victim->in_room);
      sprintf (tcbuf, "You drop %s.\n\r", obj2->short_descr);
      send_to_char (tcbuf, victim);
      sprintf (tcbuf, "$n drops %s.", obj2->short_descr);
      act (tcbuf, victim, NULL, NULL, TO_ROOM);
      if (victim && !IS_NPC(victim))
        save_char_obj (victim);
    }

  return;
}

// Shinowlan -- 2/9/99 number_of_charmies()
// Returns the number of charmies a bard currently has control of.
int number_of_charmies (CHAR_DATA * ch)
{
  CHAR_DATA *fch;
  int has_charmed = 0;

  for (fch = char_list; fch; fch = fch->next)
    {
      if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM))
	{
	  has_charmed++;
	}
    }
  // return number of charmies for character, ch.
  return has_charmed;
}



// Shinowlan -- 2/9/99 allowable_charmies()
// Returns the number of charmies a character is allowed to have. This
// procedure was written so as to avoid repeated calls to can_charm for
// in piper's melody. 
int allowable_charmies (CHAR_DATA * ch)
{
  int avg;
  int allowed = 0;

  avg = (get_curr_stat (ch, STAT_CHA) + get_curr_stat (ch, STAT_INT)) / 2;

  // Akamai 4/29/99 - Bard charm fixes
  // small random bonus for certain races - requested by moonlyte
  switch (ch->race)
    {
    case PC_RACE_CANTHI:
      avg = avg + number_range (1, 4);
      break;
/*    case PC_RACE_SYVIN:
      avg = avg + number_range (0, 2);
      break;*/
    default:
      break;
    }

  switch (avg)
    {
    default:
      allowed = 0;
      break;
    case 12:
    case 13:
    case 14:
      allowed = 1;
      break;
    case 15:
    case 16:
      allowed = 2;
      break;
    case 17:
    case 18:
      allowed = 3;
      break;
    case 19:
    case 20:
      allowed = 4;
      break;
    case 21:
      allowed = 5;
      break;
    case 22:
      allowed = 6;
      break;
    case 23:
      allowed = 7;
      break;
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
      allowed = 8;
      break;
    }				// end switch(avg)
  return (allowed);
}


// Shinowlan 2/9/99 --
// modified to avoid using can_charm which searches across char_list.
// Instead, rely on single calls to allowable_charmies() and 
// number_of_charmies().  This results in much faster execution.

void song_piper (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;
  CHAR_DATA *vch_next;
  int charisma = 0;
  int savet, maxaff, afft = 0;
  int max_charmies, current_charmies;
  short level1;

// Minax 2-13-03 Cannot cast charm in a no_charmie room

  if (IS_SET (ch->in_room->room_flags, ROOM_NO_CHARMIE))
    {
      send_to_char ("This room is resistant to your charms.\n\r", ch);
      return;
    }

  send_to_char ("You play a compelling tune on your flute.\n\r", ch);
  act ("A compelling melody unravels from $n's flute.", ch, NULL, NULL,
       TO_ROOM);

  //Iblis 6/11/03 - Made bard charm based on your levels as a bard, not total level
  level1 = Class_level (ch, 5);

  //Morgan 9/3/03 - Imms who were never bards are still able to use the song
  if (IS_IMMORTAL(ch))
      level1 = ch->level;


  // Akamai 3/12/99 - This change gives a small (random) bonus to
  // Sidhe, Canthi and Syvin - change was requested by Moonlyte
  charisma = get_curr_stat (ch, STAT_CHA);
  switch (ch->race)
    {
    case PC_RACE_CANTHI:
    case PC_RACE_SIDHE:
      charisma = charisma + number_range (1, 5);
      break;
    case PC_RACE_SYVIN:
      charisma = charisma + number_range (1, 3);
      break;
    default:
      break;
    }

  // Akamai 3/12/99 - the range of cases was increased to account for
  // the possible bonuses given to Canthi, Sidhe and Syvin bards
  switch (charisma)
    {
    case 30:
      savet = -10;
      maxaff = 8;
      break;
    case 29:
      savet = -8;
      maxaff = 8;
      break;
    case 28:
      savet = -6;
      maxaff = 8;
      break;
    case 27:
      savet = -5;
      maxaff = 8;
      break;
    case 26:
      savet = -4;
      maxaff = 8;
      break;
    case 25:
      savet = -3;
      maxaff = 8;
      break;
    case 24:
      savet = -2;
      maxaff = 7;
      break;
    case 23:
      savet = -1;
      maxaff = 7;
      break;
    case 22:
      savet = -1;
      maxaff = 6;
      break;
    case 21:
      savet = -1;
      maxaff = 5;
      break;
    case 20:
    case 19:
      savet = 0;
      maxaff = 4;
      break;
    case 18:
    case 17:
      savet = 0;
      maxaff = 3;
      break;
    default:
      savet = 0;
      maxaff = 2;
      break;
    }

  // Akamai: 10/27/98 -- There was a bug in the bard charm song that
  // allowed bards to charm 1 additional MOB each time the song was
  // played. The fix limits Bards to a max of 8 charmies, like Mages
  //
  // run through all of the chars in the mud looking for chars/mobs
  // which are in the same room and that can be charmed

  // determine max number of charmies this character can have
  max_charmies = allowable_charmies (ch);

  // determine how many charmies the character currently has
  current_charmies = number_of_charmies (ch);


  for (victim = char_list; victim != NULL; victim = vch_next)
    {

      // if the bard can't charm any more then exit procedure
      //if( !can_charm(ch) ) return;
      if (current_charmies >= max_charmies)
	return;

      // if the bard has affected enough for this singing
      if (afft >= maxaff)
	return;

      // set the next victim, sets up loop on a continue
      vch_next = victim->next;

      // if victim is not in any room go to the next char in list
      if (victim->in_room == NULL)
	continue;

      // if the victim is an immortal, skip them
      if (IS_IMMORTAL (victim))
	continue;

      // if the victim is a real player +- MAGIC_LEVEL_DIFF then
      if (!IS_NPC (victim) &&
	  ((victim->level >= (level1 + 10)) ||
	   (victim->level <= (level1 - MAGIC_LEVEL_DIFF))))
	continue;

      // if the victim is a MOB + MAGIC_LEVEL_DIFF from the level of the
      // bard then the MOB is beyond their piping ability
      if (IS_NPC (victim) && (victim->level > (level1 + 10)))
	continue;

      // if bard and victim are not the same char and they are in the same room
      if (victim != ch && victim->in_room == ch->in_room)
	{

	  // they are not protected
	  if (!is_safe_spell (ch, victim, TRUE))
	    {

	      // the not protected chars may have an exception that keeps them
	      // from being charmed
	      if (IS_AFFECTED (victim, AFF_CHARM) ||
		  is_affected (victim, skill_lookup ("The Pipers Melody"))
		  || is_affected (ch, skill_lookup ("The Pipers Melody"))
		  || IS_AFFECTED (ch, AFF_CHARM)
		  || IS_SET (victim->imm_flags, IMM_CHARM)
		  || IS_SET (victim->act, ACT_UNDEAD)
		  || IS_SET (victim->imm_flags, IMM_SOUND)
/*		  saves_spell (level1 + savet, victim, DAM_CHARM)*/
		)
		{
		  act ("$N is not affected by your song.",
		       ch, NULL, victim, TO_CHAR);
		  continue;
		}		// affs exceptions

	      // ok, the song is successful for this victim
	      if (victim->master)
		stop_follower (victim);

	      // add thie victim
	      add_follower (victim, ch);
	      victim->leader = ch;

	      // initialized the affects for this victim
	      af.where = TO_AFFECTS;
	      af.type = sn;
	      af.level = level1;
	      af.duration = number_fuzzy (level1 / 4);
	      af.location = 0;
	      af.modifier = 0;
	      af.bitvector = AFF_CHARM;
	      af.permaff = FALSE;
	      af.composition = FALSE;
	      af.comp_name = str_dup ("");

	      // assign the affects to victim 
	      affect_to_char (victim, &af);
	      afft++;
	      current_charmies++;

	      act ("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
	      act ("$N looks at you with adoring eyes.", ch, NULL, victim,
		   TO_CHAR);

	    }			// end of the not protected test
	}			// end of the check that they are in the same room
    }				// end of the for loop

  return;
}

void song_qmlake (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  int mlevel = 0;
  int count = 0;
  int high_level = 0;
  int chance;
  AFFECT_DATA af;

  /* get sum of all mobile levels in the room */
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (vch->position == POS_FIGHTING || vch->fighting != NULL)
	{
	  count++;
	  if (IS_NPC (vch))
	    {
	      mlevel += vch->level;
	      if (IS_SET (vch->vuln_flags, VULN_SOUND) &&
		  IS_SET (vch->vuln_flags, VULN_MENTAL))
		mlevel -= vch->level / 2;
	    }
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
	  if (IS_NPC (vch))
	    {
	      if (IS_SET (vch->imm_flags, IMM_MAGIC) ||
		  IS_SET (vch->act, ACT_UNDEAD) ||
		  IS_SET (vch->imm_flags, IMM_MENTAL) ||
		  IS_SET (vch->imm_flags, IMM_SOUND))
		continue;

	      if (IS_SET (vch->res_flags, RES_MENTAL) &&
		  IS_SET (vch->res_flags, RES_SOUND))
		if (number_percent () < 40)
		  continue;
	    }

	  if (is_affected (vch, skill_lookup ("Quiet Mountain Lake"))
	      || IS_AFFECTED (vch, AFF_BERSERK)
	      || is_affected (vch, skill_lookup ("frenzy")))
	    return;


          if (is_safe (ch, vch) && vch != ch)
            {
           //   send_to_char("Not on that target.\n\r", ch);
              continue;
            }
	  
	  send_to_char
	    ("The serenity of a quiet mountain morning is instilled in you.\n\r",
	     vch);

	  if (vch->fighting || vch->position == POS_FIGHTING)
	    stop_fighting (vch, FALSE);

	  af.permaff = FALSE;
	  af.where = TO_AFFECTS;
	  af.type = sn;
	  af.level = level;
	  af.duration = level / 4;
	  af.location = APPLY_HITROLL;
	  if (!IS_NPC (vch))
	    af.modifier = -5;
	  else
	    af.modifier = -2;
	  af.bitvector = 0;

	  af.composition = FALSE;
	  af.comp_name = str_dup ("");

	  affect_to_char (vch, &af);
	  af.location = APPLY_DAMROLL;
	  affect_to_char (vch, &af);
	}
    }
}

void
song_maelstrom_of_nyvenban (int sn, int level, CHAR_DATA * ch, void *vo,
			    int target)
{
  CHAR_DATA *victim, *victim_next;
  ROOM_INDEX_DATA *was_in, *now_in;
  int attempt;

  for (victim = ch->in_room->people; victim != NULL; victim = victim_next)
    {
      victim_next = victim->next_in_room;

      if (victim == ch)
	continue;

      if (get_position(victim) <= POS_SLEEPING || victim->position == POS_MOUNTED)
	continue;

      if (IS_NPC (victim) && (victim->pIndexData->pShop != NULL ||
			      IS_SET (victim->act, ACT_TRAIN)
			      || IS_SET (victim->act, ACT_PRACTICE)
			      || IS_SET (victim->act, ACT_IS_HEALER)
		              || IS_SET (victim->act2, ACT_NOMOVE)))
	continue;


      if (saves_spell (ch->level, victim, DAM_MENTAL))
	continue;

      if (IS_SET(victim->imm_flags, IMM_SOUND))
        continue;

      if (victim->position == POS_RESTING || victim->position == POS_SITTING)
	do_stand (victim, "");

      was_in = victim->in_room;
      for (attempt = 0; attempt < 6; attempt++)
	{
	  EXIT_DATA *pexit;
	  int door;

	  door = number_door ();
	  if ((pexit = was_in->exit[door]) == 0
	      || pexit->u1.to_room == NULL
	      || IS_SET (pexit->exit_info, EX_CLOSED)
	      || number_range (0, victim->daze) != 0 || (IS_NPC (victim)
							 && IS_SET (pexit->
								    u1.
								    to_room->
								    room_flags,
								    ROOM_NO_MOB)))
	    continue;

	  move_char (victim, door, FALSE);

	  if ((now_in = victim->in_room) != was_in)
	    break;
	}

      if ((now_in = victim->in_room) == was_in)
	continue;

      victim->in_room = was_in;
      act ("Assailed with the frenetics of a manic vision, $n flees.",
	   victim, NULL, NULL, TO_ROOM);
      act ("Assailed with the frenetics of a manic vision, you flee.",
	   victim, NULL, NULL, TO_CHAR);
      victim->in_room = now_in;
      stop_fighting (victim, TRUE);
    }
}

void
song_kiss_of_first_love (int sn, int level, CHAR_DATA * ch, void *vo,
			 int target)
{

  CHAR_DATA *victim;

  for (victim = ch->in_room->people; victim != NULL;
       victim = victim->next_in_room)
    {
      if (victim->race == PC_RACE_SWARM)
	      continue;
      if ((IS_NPC (ch) && IS_NPC (victim)) ||
	  (!IS_NPC (ch) && !IS_NPC (victim)))
	{
	  victim->move = UMIN (victim->move + level / 2, victim->max_move);
	  if (!IS_NPC(victim) && victim->pcdata->soul_link)
	  {
		  victim->hit = UMIN (victim->hit + 13, victim->max_hit);
		  victim->pcdata->soul_link->hit = UMIN (victim->pcdata->soul_link->hit + 13,
				  victim->pcdata->soul_link->max_hit);
		  update_pos (victim->pcdata->soul_link);
		  send_to_char ("The sweet remembrance of a first love's kiss rejuvenates you.\n\r", victim->pcdata->soul_link);
	  }
	  else victim->hit = UMIN (victim->hit + 25, victim->max_hit);
	  send_to_char
	    ("The sweet remembrance of a first love's kiss rejuvenates you.\n\r",
	     victim);
	  if (ch != victim)
	    send_to_char ("Ok.\n\r", ch);
	}
    }
}

void
song_ballad_woodie (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const sh_int dam_each[] = {
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    30, 35, 40, 45, 50, 55, 55, 55, 56, 57,
    58, 58, 59, 60, 61, 61, 62, 63, 64, 64,
    65, 66, 67, 67, 68, 69, 70, 70, 71, 72,
    73, 73, 74, 75, 76, 76, 77, 78, 79, 79
  };
  int dam;
  AFFECT_DATA af;

  level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
  level = UMAX (0, level);
  dam = number_range (dam_each[level] / 2, dam_each[level] * 2);

  if (saves_spell (level, victim, DAM_SOUND))
    dam /= 2;

  damage_old (ch, victim, dam, sn, DAM_SOUND, TRUE);
  if (!saves_spell (level, victim, DAM_POISON))
    {
      send_to_char ("You feel poison coursing through your veins.\n\r",
		    victim);
      act ("$n looks very ill.", victim, NULL, NULL, TO_ROOM);

      af.where = TO_AFFECTS;
      af.type = gsn_poison;
      af.level = level;
      af.duration = level / 2;
      af.location = APPLY_STR;
      af.modifier = -1;
      af.bitvector = AFF_POISON;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_join (victim, &af);
    }

  return;
}
