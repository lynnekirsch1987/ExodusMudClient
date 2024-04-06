#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "pray.h"
#include "magic.h"

void do_say args((CHAR_DATA * ch, char *argument));
void do_tell args((CHAR_DATA * ch, char *argument));
void do_telepath args((CHAR_DATA * ch, char *argument));

void do_cash (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *mob;
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *stone;
  int pracs;
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
    {
      if (IS_NPC (mob) && IS_SET (mob->act, ACT_IS_HEALER))
	break;
    }
  if (mob == NULL)
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
    }
  if (!str_cmp (arg1, "rune")) //&& !str_cmp (arg2, "in"))
    {
      for (stone = ch->carrying;stone != NULL;stone = stone->next_content)
      {
	if (stone->wear_loc != -1)
  	  continue;
	if (stone->pIndexData->vnum != OBJ_VNUM_RUNE_CONV)
  	  continue;
	break;
      }
      
      if (stone == NULL || stone->pIndexData->vnum != OBJ_VNUM_RUNE_CONV)
	{
	  send_to_char ("You have no rune in your inventory to cash in.\n\r", ch);
	  return;
	}

      pracs = number_range (5,15);
      if (!str_cmp (arg2, "hp"))
      {
        sprintf(buf,"You cash your rune in for %d hp.\n\r",pracs);
        send_to_char(buf,ch);
        sprintf(buf,"%s cashes a rune (%d) in for %d hp.",ch->name,stone->value[3],pracs);
        wiznet (buf, ch, NULL, WIZ_SECURE, 0, 0);
	ch->max_hit += pracs;
	ch->pcdata->perm_hit += pracs;
	extract_obj (stone);
      }
      else if (!str_cmp (arg2, "mana"))
      {
        sprintf(buf,"You cash your rune in for %d mana.\n\r",pracs);
        send_to_char(buf,ch);
        sprintf(buf,"%s cashes a rune (%d) in for %d mana.",ch->name,stone->value[3],pracs);
        wiznet (buf, ch, NULL, WIZ_SECURE, 0, 0);
        ch->max_mana += pracs;
        ch->pcdata->perm_mana += pracs;
	extract_obj (stone);
      }
      else if (!str_cmp (arg2, "move"))
      {
        sprintf(buf,"You cash your rune in for %d move.\n\r",pracs);
	send_to_char(buf,ch);
	sprintf(buf,"%s cashes a rune (%d) in for %d move.",ch->name,stone->value[3],pracs);
	wiznet (buf, ch, NULL, WIZ_SECURE, 0, 0);
        send_to_char(buf,ch);
        ch->max_move += pracs;
        ch->pcdata->perm_move += pracs;
	extract_obj (stone);
      }
      else send_to_char("Syntax : Cash rune (hp)(mana)(move).\n\r",ch);
      
      return;
    }
  else
    {
      send_to_char
	("Syntax : Cash rune (hp)(mana)(move).\n\r",
	 ch);
      return;
    }
  return;
}

void do_heal (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *mob;
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  int cost, sn;
  SPELL_FUN *spell;
  char *words;

  /* check for healer */
  for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
    {
      if (IS_NPC (mob) && IS_SET (mob->act, ACT_IS_HEALER))
	break;
    }
  if (mob == NULL)
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
    }
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {

      /* display price list */
      act ("$N says 'I offer the following spells:'", ch, NULL, mob, TO_CHAR);
      send_to_char ("  light:    cure light wounds      200 silver\n\r", ch);
      send_to_char ("  serious:  cure serious wounds    400 silver\n\r", ch);
      send_to_char ("  critic:   cure critical wounds   550 silver\n\r", ch);
      send_to_char ("  heal:     healing spell         1000 silver\n\r", ch);
      send_to_char ("  blind:    cure blindness         200 silver\n\r", ch);
      send_to_char ("  disease:  cure disease           250 silver\n\r", ch);
      send_to_char ("  poison:   cure poison            250 silver\n\r", ch);
      send_to_char ("  uncurse:  remove curse           500 silver\n\r", ch);
      send_to_char ("  refresh:  restore movement       150 silver\n\r", ch);
      send_to_char ("  mana:     restore mana           600 silver\n\r", ch);
      send_to_char ("  ceremony: holy ceremony        50000 silver\n\r", ch);
      send_to_char (" Type heal <type> to be healed.\n\r", ch);
      return;
    }
  if (!str_prefix (arg, "light"))
    {
      if (!IS_NPC (ch))
	if ((time (NULL) - ch->pcdata->last_fight < 100))
	  {
	    if (is_affected (ch, gsn_mask) && ch->mask != NULL)
  	      sprintf(buf,"%s `cWash up a little you blood-smeared bastard, THEN I will help you!",ch->mask);
	    else
	      sprintf(buf,"%s `cWash up a little you blood-smeared bastard, THEN I will help you!",ch->name);
	    if (ch->race != PC_RACE_SIDHE)
	      do_tell(mob,buf);
	    else do_telepath(mob,buf);
	    return;
	  }
      spell = spell_cure_light;
      sn = skill_lookup ("cure light");
      words = "judicandus dies";
      cost = 200;
    }

  else if (!str_prefix (arg, "serious"))
    {
      if (!IS_NPC (ch))
	if ((time (NULL) - ch->pcdata->last_fight < 100))
	  {
	    if (is_affected (ch, gsn_mask) && ch->mask != NULL)
              sprintf(buf,"%s `cWash up a little you blood-smeared bastard, THEN I will help you!",ch->mask);
            else
              sprintf(buf,"%s `cWash up a little you blood-smeared bastard, THEN I will help you!",ch->name);
            if (ch->race != PC_RACE_SIDHE) 
              do_tell(mob,buf);
	    else do_telepath(mob,buf);
	    return;
	  }
      spell = spell_cure_serious;
      sn = skill_lookup ("cure serious");
      words = "judicandus gzfuajg";
      cost = 400;
    }

  else if (!str_prefix (arg, "critical"))
    {
      if (!IS_NPC (ch))
	if ((time (NULL) - ch->pcdata->last_fight < 100))
	  {
	    if (is_affected (ch, gsn_mask) && ch->mask != NULL)
              sprintf(buf,"%s `cWash up a little you blood-smeared bastard, THEN I will help you!",ch->mask);
            else
              sprintf(buf,"%s `cWash up a little you blood-smeared bastard, THEN I will help you!",ch->name);
	    if (ch->race != PC_RACE_SIDHE)
              do_tell(mob,buf);
	    else do_telepath(mob,buf);
	    return;
	  }
      spell = spell_cure_critical;
      sn = skill_lookup ("cure critical");
      words = "judicandus qfuhuqar";
      cost = 550;
    }

  else if (!str_prefix (arg, "heal"))
    {
      if (!IS_NPC (ch))
	if ((time (NULL) - ch->pcdata->last_fight < 100))
	  {
	    if (is_affected (ch, gsn_mask) && ch->mask != NULL)
              sprintf(buf,"%s `cWash up a little you blood-smeared bastard, THEN I will help you!",ch->mask);
            else
              sprintf(buf,"%s `cWash up a little you blood-smeared bastard, THEN I will help you!",ch->name);
	    if (ch->race != PC_RACE_SIDHE)
              do_tell(mob,buf);
	    else do_telepath(mob,buf);
	    return;
	  }
      spell = spell_heal;
      sn = skill_lookup ("heal");
      words = "pzar";
      cost = 1000;
    }

  else if (!str_prefix (arg, "blindness"))
    {
      spell = spell_cure_blindness;
      sn = skill_lookup ("cure blindness");
      words = "judicandus noselacri";
      cost = 200;
    }

  else if (!str_prefix (arg, "disease"))
    {
      spell = spell_cure_disease;
      sn = skill_lookup ("cure disease");
      words = "judicandus eugzagz";
      cost = 250;
    }

  else if (!str_prefix (arg, "poison"))
    {
      spell = spell_cure_poison;
      sn = skill_lookup ("cure poison");
      words = "judicandus sausabru";
      cost = 250;
    }

  else if (!str_prefix (arg, "uncurse") || !str_prefix (arg, "curse"))
    {
      spell = prayer_remove_curse;
      sn = skill_lookup ("remove curse");
      words = "candussido judifgz";
      cost = 500;
    }

  else if (!str_prefix (arg, "ceremony"))
    {
      spell = prayer_holy_ceremony;
      sn = skill_lookup ("holy ceremony");
      words = "parl qzbendiwail";
      cost = 50000;
    }

  else if (!str_prefix (arg, "mana") || !str_prefix (arg, "energize"))
    {
      if (!IS_NPC (ch))
	if ((time (NULL) - ch->pcdata->last_fight < 100))
	  {
	    if (is_affected (ch, gsn_mask) && ch->mask != NULL)
              sprintf(buf,"%s `cWash up a little you blood-smeared bastard, THEN I will help you!",ch->mask);
            else
              sprintf(buf,"%s `cWash up a little you blood-smeared bastard, THEN I will help you!",ch->name);
	    if (ch->race != PC_RACE_SIDHE)
              do_tell(mob,buf);
	    else do_telepath(mob,buf);
	    return;
	  }
      spell = NULL;
      sn = -1;
      words = "energizer";
      cost = 150;
    }

  else if (!str_prefix (arg, "refresh") || !str_prefix (arg, "moves"))
    {
      if (!IS_NPC (ch))
	if ((time (NULL) - ch->pcdata->last_fight < 100))
	  {
	    if (is_affected (ch, gsn_mask) && ch->mask != NULL)
              sprintf(buf,"%s `cWash up a little you blood-smeared bastard, THEN I will help you!",ch->mask);
            else
              sprintf(buf,"%s `cWash up a little you blood-smeared bastard, THEN I will help you!",ch->name);
	    if (ch->race != PC_RACE_SIDHE)
              do_tell(mob,buf);
	    else do_telepath(mob,buf);
	    return;
	  }
      spell = spell_refresh;
      sn = skill_lookup ("refresh");
      words = "candusima";
      cost = 600;
    }

  else
    {
      act ("$N says 'Type 'heal' for a list of spells.'", ch, NULL, mob,
	   TO_CHAR);
      return;
    }
  if (cost > (ch->gold * 100 + ch->silver))
    {
      act ("$N says 'You do not have enough gold for my services.'", ch,
	   NULL, mob, TO_CHAR);
      return;
    }
  WAIT_STATE (ch, PULSE_VIOLENCE);
  deduct_cost (ch, cost);
  mob->silver += cost;
  act ("$n utters the words '$T'.", mob, NULL, words, TO_ROOM);
  if (spell == NULL)
    {				/* restore mana trap...kinda hackish */
      ch->mana += dice (2, 8) + mob->level / 3;
      ch->mana = UMIN (ch->mana, ch->max_mana);
      send_to_char ("A warm glow passes through you.\n\r", ch);
      return;
    }
  if (sn == -1)
    return;
  spell (sn, mob->level, mob, ch, TARGET_CHAR);
}
