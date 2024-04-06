// Object Triggers started by Adeon, largely completed by Iblis.  These are pretty complicated, so
// I largely suggested you DO NOT FUCKING TOUCH THESE and let me handle them.
// obj_trig.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include "merc.h"
#include "interp.h"
#include "obj_trig.h"
#include "olc.h"
#include "recycle.h"
#include "tables.h"

#define OTEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )

void set_trigger_flags(OBJ_TRIG_DATA *pObjTrig);
void check_trigger(CHAR_DATA *ch, OBJ_TRIG_DATA *ot, long trig_type,
				   OBJ_DATA *obj, CHAR_DATA *victim, int special, char *arg);
void activate_trigger(OBJ_TRIG *ot, OBJ_DATA *obj, CHAR_DATA *ch, CHAR_DATA *victim);
void activate_trigger_statement(int statement_on, OBJ_TRIG *ot, OBJ_DATA *obj, CHAR_DATA *ch, CHAR_DATA *victim);
void interp_action(char *action, OBJ_TRIG_DATA *ot, OBJ_DATA *obj,
				   CHAR_DATA *ch, CHAR_DATA *victim);
char *translate_var(char *action, OBJ_DATA *obj, CHAR_DATA *ch,
					CHAR_DATA *victim);


void disguise_fakie(OBJ_TRIG_DATA *ot, OBJ_DATA *obj);
void revert_fakie(OBJ_TRIG_DATA *ot);

bool contains_text(char *string, char *text);

void free_obj_trig(OBJ_TRIG_DATA *ot);
const char *obj_trig_bit_name (int obj_trig_flags);
OBJ_TRIG_DATA *new_obj_trig(void);
extern OBJ_TRIG *ot_list;
//extern OBJ_TRIG_DATA *obj_trig_list;
extern OBJ_TRIG *giant_ot_list;
extern OBJ_TRIG *on_tick_ot_list;
extern CHAR_DATA *GLOBAL_ot_ch;
extern OBJ_DATA *GLOBAL_ot_obj;
void free_ot(OBJ_TRIG *ot);
void ot_set_bits(OBJ_TRIG_DATA* ot, char *argument);
inline void variable_substitution_new (CHAR_DATA *ch, CHAR_DATA *victim, CHAR_DATA *wearer, CHAR_DATA *extra, char *string);
bool obj_trig_in_progress = FALSE;
void check_gravity_char args((CHAR_DATA *ch));
	
	



//sets trigger flags based on the contents of the trigger string
void set_trigger_flags(OBJ_TRIG_DATA *pObjTrig)
{
	char arg1[MAX_STRING_LENGTH];
	char *targ;
	char *trigger;

	pObjTrig->trig_flags = 0; //reset them

	trigger = str_dup(pObjTrig->trigger);

	targ = one_argument(trigger, arg1);

/*	if(!str_cmp(arg1, "$when_worn"))
          SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_EXTRA_FLAGS);
	if(!str_cmp(arg1, "$when_carries"))
          SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_EXTRA_FLAGS);
	if(!str_cmp(arg1, "$when_on_ground"))
          SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_EXTRA_FLAGS);
	if(!str_cmp(arg1, "$when_worn_or_carries"))
          SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_EXTRA_FLAGS);
	if(!str_cmp(arg1, "$when_any"))
          SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_EXTRA_FLAGS);
	if (IS_SET(pObjTrig->trig_flags, OBJ_TRIG_EXTRA_FLAGS))
	  one_argument(targ, arg1);*/
	if(!str_cmp(arg1, "$entersroom"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_ENTER_ROOM);
	  ot_set_bits(pObjTrig, "worn carried on_ground");
	}
	if(!str_cmp(arg1, "$getsobj"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_GET);
	  ot_set_bits(pObjTrig, NULL);
	}
	if(!str_cmp(arg1, "$wearsobj"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_WEAR);
	  ot_set_bits(pObjTrig, NULL);
	}
	if(!str_cmp(arg1, "$char_hp_pct"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_CHAR_HP_PCT);
	  ot_set_bits(pObjTrig, "worn");
	}
	if(!str_cmp(arg1, "$vict_hp_pct"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_VICT_HP_PCT);
	  ot_set_bits(pObjTrig, "worn");
	}
	if(!str_cmp(arg1, "$mana_pct"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_MANA_PCT);
	  ot_set_bits(pObjTrig, "worn");
	}
	if(!str_cmp(arg1, "$move_pct"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_MOVE_PCT);
	  ot_set_bits(pObjTrig, "worn");
	}
	if(!str_cmp(arg1, "$chance_on_hit"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_CHANCE_ON_HIT);
	  ot_set_bits(pObjTrig, "worn");
	}
	if(!str_cmp(arg1, "$chance_on_round"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_CHANCE_ON_ROUND);
	  ot_set_bits(pObjTrig, "worn");
	}
	if(!str_cmp(arg1, "$whenremoved"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_REMOVE);
	  ot_set_bits(pObjTrig, NULL);
	}
	if(!str_cmp(arg1, "$whendropped"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_DROP);
	  ot_set_bits(pObjTrig, NULL);
	}
	if(!str_cmp(arg1, "$whenput"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_PUT);
	  ot_set_bits(pObjTrig, NULL);
	}
	if(!str_cmp(arg1, "$obj_given_char"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_OBJ_GIVEN_CHAR);
	  ot_set_bits(pObjTrig, NULL);
	}
	if(!str_cmp(arg1, "$whenopened"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_OPEN);
	  ot_set_bits(pObjTrig, NULL);
	}
	if(!str_cmp(arg1, "$whenclosed"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_CLOSE);
	  ot_set_bits(pObjTrig, NULL);
	}
	if(!str_cmp(arg1, "$whenactivated") ||
	   !str_cmp(arg1, "$whenentered") ||
	   !str_cmp(arg1, "$whenexited") ||
	   !str_cmp(arg1, "$whencrawled") ||
	   !str_cmp(arg1, "$whenclimbed") ||
	   !str_cmp(arg1, "$whendescended") ||
	   !str_cmp(arg1, "$whenscaled") ||
	   !str_cmp(arg1, "$whenjumped") ||
	   !str_cmp(arg1, "$whentugged") ||
	   !str_cmp(arg1, "$whenrung") ||
	   !str_cmp(arg1, "$whenshoved") ||
	   !str_cmp(arg1, "$whensmashed") ||
	   !str_cmp(arg1, "$whenpressed") ||
	   !str_cmp(arg1, "$whenpulled") ||
	   !str_cmp(arg1, "$whenboarded") ||
	   !str_cmp(arg1, "$whendug")
	   )
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_ACTION_WORD);
	  ot_set_bits(pObjTrig, NULL);
	}
	if(!str_cmp(arg1, "$char_enters_room"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_CHAR_TO_ROOM);
	  ot_set_bits(pObjTrig, "worn");
	}
	if(!str_cmp(arg1, "$char_says") ||
	   !str_cmp(arg1, "$char_yells") ||
	   !str_cmp(arg1, "$char_shouts"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_CHAR_VOCALIZE);
	  ot_set_bits(pObjTrig, "worn carried on_ground");
	}
	if(!str_cmp(arg1, "$char_attacks"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_CHAR_ATTACKS);
	  ot_set_bits(pObjTrig, "worn");
	}
	if(!str_cmp(arg1, "$char_is_attacked"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_CHAR_IS_ATTACKED);
	  ot_set_bits(pObjTrig, "worn");
	}
	if(!str_cmp(arg1, "$whenquaffed"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_QUAFFED);
	  ot_set_bits(pObjTrig, NULL);
	}
	if(!str_cmp(arg1, "$wheneaten"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_EATEN);
	  ot_set_bits(pObjTrig, NULL);
	}
	if(!str_cmp(arg1, "$char_flees"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_CHAR_FLEES);
	  ot_set_bits(pObjTrig, "worn");
	}
	if(!str_cmp(arg1, "$vict_flees"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_VICT_FLEES);
	  ot_set_bits(pObjTrig, "worn");
	}
	if(!str_cmp(arg1, "$char_sleeps") ||
	   !str_cmp(arg1, "$char_sits") ||
	   !str_cmp(arg1, "$char_rests"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_CHAR_POS_CHANGE);
	  ot_set_bits(pObjTrig, "worn");
	}
	if(!str_cmp(arg1, "$char_recalls"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_CHAR_RECALLS);
	  ot_set_bits(pObjTrig, "worn carried");
	}
	if(!str_cmp(arg1, "$char_uses_skill"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_CHAR_USES_SKILL);
	  ot_set_bits(pObjTrig, "worn");
	}
	if(!str_cmp(arg1, "$char_uses_spell"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_CHAR_USES_SPELL);
	  ot_set_bits(pObjTrig, "worn");
	}
	if(!str_cmp(arg1, "$char_invokes"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_CHAR_INVOKES);
	  ot_set_bits(pObjTrig, "worn");
	}
	if(!str_cmp(arg1, "$on_tick"))
	{
	  SET_BIT(pObjTrig->trig_flags, OBJ_TRIG_ON_TICK);
	  ot_set_bits(pObjTrig, "worn carried on_ground");
	}
}


void trip_triggers(CHAR_DATA *ch, long trig_type, OBJ_DATA *obj,
		                                   CHAR_DATA *victim, int special)
{
	trip_triggers_arg(ch, trig_type, obj, victim, special,(char *) NULL);
} 

//check_trigger: makes sure all conditions for the various trigger types
// are true, and also sets neccesary variables to pass to activate_triggers
bool should_be_tripped(CHAR_DATA *ch, long trig_type, OBJ_TRIG_DATA *otd, int special, char *arg, CHAR_DATA *victim)
{
  
  char word[MAX_STRING_LENGTH];
  char *dupe;
  int value1;
  
  if(IS_NPC(ch) && !IS_SET(otd->extra_flags,OT_EXTRA_ON_NPC))
	  return FALSE;
  if(!IS_NPC(ch) && !IS_SET(otd->extra_flags,OT_EXTRA_ON_PC))
	  return FALSE;
  
  if(trig_type == OBJ_TRIG_CHAR_POS_CHANGE)
    {
      if(ch->fighting) //don't want position changes due to
	return FALSE;      //aggression to affect this trigger type
      
      if(get_position(ch) == POS_SITTING &&
	 !str_cmp(otd->trigger, "$char_sits"))
	return TRUE;
      else return FALSE;
      
      if(ch->position == POS_RESTING &&
	 !str_cmp(otd->trigger, "$char_rests"))
	return TRUE;
      else return FALSE;
      if(get_position(ch) == POS_SLEEPING &&
	 !str_cmp(otd->trigger, "$char_sleeps"))
	return TRUE;
      else return FALSE;
    }
  else if(trig_type == OBJ_TRIG_ACTION_WORD)
    {
				//$whenactivated looks for any action
      if(!str_cmp(otd->trigger, "$whenactivated"))
	return TRUE;
      
      switch(special)
	{
	case OT_SPEC_ENTER:
	  if(!str_cmp(otd->trigger, "$whenentered"))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_SPEC_EXIT:
	  if(!str_cmp(otd->trigger, "$whenexited"))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_SPEC_CRAWL:
	  if(!str_cmp(otd->trigger, "$whencrawled"))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_SPEC_CLIMB:
	  if(!str_cmp(otd->trigger, "$whenclimbed"))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_SPEC_DESCEND:
	  if(!str_cmp(otd->trigger, "$whendescended"))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_SPEC_SCALE:
	  if(!str_cmp(otd->trigger, "$whenscaled"))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_SPEC_JUMP:
	  if(!str_cmp(otd->trigger, "$whenjumped"))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_SPEC_TUG:
	  if(!str_cmp(otd->trigger, "$whentugged"))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_SPEC_RING:
	  if(!str_cmp(otd->trigger, "$whenrung"))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_SPEC_SHOVE:
	  if(!str_cmp(otd->trigger, "$whenshoved"))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_SPEC_SMASH:
	  if(!str_cmp(otd->trigger, "$whensmashed"))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_SPEC_PRESS:
	  if(!str_cmp(otd->trigger, "$whenpressed"))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_SPEC_PULL:
	  if(!str_cmp(otd->trigger, "$whenpulled"))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_SPEC_BOARD:
	  if(!str_cmp(otd->trigger, "$whenboarded"))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_SPEC_DIG:
          if(!str_cmp(otd->trigger, "$whendug"))
            return TRUE;
          else return FALSE;
         break;
	}
      
    }
  
  else if(trig_type == OBJ_TRIG_CHAR_VOCALIZE)
    {
      //first strip the trigger type
      dupe = one_argument(otd->trigger, word);
      //now we need to match the type of vocalization
      switch(special)
	{
	case OT_VOC_SAY:
	  if(str_cmp(word, "$char_says"))
	    return FALSE;
	  break;
	case OT_VOC_SHOUT:
	  if(str_cmp(word, "$char_shouts"))
	    return FALSE;
	  break;
	case OT_VOC_YELL:
	  if(str_cmp(word, "$char_yells"))
	    return FALSE;
	  break;
	default:
	  bug("check_triggers: bad voc type!", 0);
	  return FALSE;
	}
      
      if(contains_text(arg, dupe))
	return TRUE;
      else return FALSE;
      
    }
  else if(trig_type == OBJ_TRIG_CHAR_HP_PCT ||
	  trig_type == OBJ_TRIG_VICT_HP_PCT ||
	  trig_type == OBJ_TRIG_MANA_PCT ||
	  trig_type == OBJ_TRIG_MOVE_PCT)
    {
      dupe = one_argument(otd->trigger, word);
      word[0] = '\0';
      dupe = one_argument(dupe, word);
      
      // < hp/mana/move amount
      value1 = atoi(word);
      
      if(atoi(arg) < value1)// && value2 >= roll)
	return TRUE;
      else return FALSE;
    }
  else if (trig_type == OBJ_TRIG_OBJ_GIVEN_CHAR)
  {
	  dupe = one_argument(otd->trigger, word);
	  word[0] = '\0';
	  dupe = one_argument(dupe,word);
	  if (word[0] != '\0')
	  {
  	    if (!IS_NPC(ch))
	      return FALSE;
	    if (ch->pIndexData->vnum == atoi(word))
	      return TRUE;
	    else return FALSE;
	  }
	  return TRUE;
  }

  else if(trig_type == OBJ_TRIG_CHAR_USES_SPELL)
    {
      //first strip the trigger type
      dupe = one_argument(otd->trigger, word);
      switch(special)
	{
	case OT_TYPE_SPELL:
	  if (str_cmp(word, "$char_uses_spell"))
	    return FALSE;
	  if ((!strcmp(dupe,"spell")) 
	      || (is_number(arg) && atoi(arg) == skill_lookup(dupe)))
	    return TRUE;
	  else return FALSE;
		 break;
	  
	case OT_TYPE_SONG:
	  if (str_cmp(word, "$char_uses_spell"))
	    return FALSE;
	  if ((!strcmp(dupe,"song"))
	      || (is_number(arg) && atoi(arg) == skill_lookup(dupe)))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_TYPE_PRAYER:
	  if (str_cmp(word, "$char_uses_spell"))
	    return FALSE;
	  if ((!strcmp(dupe,"prayer"))
	      || (is_number(arg) && atoi(arg) == skill_lookup(dupe)))
	    return TRUE;
	  else return FALSE;
	  break;
	case OT_TYPE_CHANT:
	  if (str_cmp(word, "$char_uses_spell"))
	    return FALSE;
	  if ((!strcmp(dupe,"chant"))
	      || (is_number(arg) && atoi(arg) == skill_lookup(dupe)))
	    return TRUE;
	  else return FALSE;
	  break;
	default: bug("check_triggers: bad voc type!", 0);
	  return FALSE;
	}
    }
  
  else if(trig_type == OBJ_TRIG_CHAR_INVOKES)
    {
      int bs = -1;
      dupe = one_argument(otd->trigger, word);
      if(!str_cmp(dupe, "shocking"))
	{
	  bs = BLADE_SPELL_SHOCKING;
	}
      else if(!str_cmp(dupe, "frost"))
	{
	  bs = BLADE_SPELL_FROST;
	}
      else if(!str_cmp(dupe, "flaming"))
	{
	  bs = BLADE_SPELL_FLAMING;
	}
      else if(!str_cmp(dupe, "chaotic"))
	{
	  bs = BLADE_SPELL_CHAOTIC;
	}
      else if(!str_cmp(dupe, "venomous"))
	{
	  bs = BLADE_SPELL_VENEMOUS;
	}
      else if(!str_cmp(dupe, "vampiric"))
	{
	  bs = BLADE_SPELL_VAMPIRIC;
	}
      else if(!str_cmp(dupe, "apathy"))
	{
	  bs = BLADE_SPELL_APATHY;
	}
      else if(!str_cmp(dupe, "lightning"))
	{
	  bs = BURST_LIGHTNING+100;
	}
      else if(!str_cmp(dupe, "ice"))
	{
	  bs = BURST_ICE+100;
	}
      else if(!str_cmp(dupe, "fire"))
	{
	  bs = BURST_FIRE+100;
	}
      else if(!str_cmp(dupe, "unholy"))
	{
	  bs = BURST_UNHOLY+100;
	}
      else if(!str_cmp(dupe, "hellscape"))
	{
	  bs = BURST_HELLSCAPE+100;
	}
      if ((!strcmp(dupe,"invocation"))
	  || (is_number(arg) && atoi(arg) == bs))
	return TRUE;
      else return FALSE;
    }
  
  
  else if(trig_type == OBJ_TRIG_CHAR_USES_SKILL)			
    {
      
      dupe = one_argument(otd->trigger, word);
      if ((!strcmp(dupe,"skill"))
	  || (is_number(arg) && atoi(arg) == skill_lookup(dupe)))
	return TRUE;
      else return FALSE;
    }
  
  
  else if(trig_type == OBJ_TRIG_CHAR_TO_ROOM)
    {
      dupe = one_argument(otd->trigger, word);
      if (!str_cmp(dupe,arg))
	return TRUE;
      else return FALSE;
      
    }
  return TRUE;
}

void trip_triggers_arg(CHAR_DATA *ch, long trig_type, OBJ_DATA *obj, CHAR_DATA *victim, int special, char *arg)
     
{
  int i=0;
  CHAR_DATA* tch;
  if (IS_NPC(ch) && (victim == NULL || IS_NPC(victim)) && special != OT_SPEC_PULL)
    return;
  if (obj)
    {
      for (i=0;i<MAX_OBJ_TRIGS;i++)
      {
	if(obj->objtrig[i] && IS_SET(obj->objtrig[i]->pIndexData->trig_flags, trig_type))
  	  if (should_be_tripped(ch,trig_type,obj->objtrig[i]->pIndexData,special,arg,victim))
	    activate_trigger(obj->objtrig[i], obj, ch, victim);
      }
    }
  
  if (ch == NULL || ch->in_room == NULL)
	  return;
  for(obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
    {
      for (i=0;i<MAX_OBJ_TRIGS;i++)
	{
	  if(obj->objtrig[i] 
	     && IS_SET(obj->objtrig[i]->pIndexData->trig_flags, trig_type)
	     && IS_SET(obj->objtrig[i]->pIndexData->extra_flags, OT_EXTRA_WHEN_ON_GROUND))
	    if (should_be_tripped(ch,trig_type,obj->objtrig[i]->pIndexData,special,arg,victim))
	      activate_trigger(obj->objtrig[i], obj, ch, victim);
	  
	}
    }
  for(tch = ch->in_room->people; tch != NULL; tch = tch->next_in_room)
    {
      for (obj = tch->carrying; obj != NULL; obj = obj->next_content)
	{
	  for (i=0;i<MAX_OBJ_TRIGS;i++)
	    {
	      if(obj->objtrig[i] && IS_SET(obj->objtrig[i]->pIndexData->trig_flags, trig_type))
		{
		//  if(get_eq_char(tch, obj->wear_loc) == obj && obj->wear_loc != WEAR_NONE)
		if (obj->carried_by == tch && obj->wear_loc != WEAR_NONE)
		    {
		      if(IS_SET(obj->objtrig[i]->pIndexData->extra_flags, OT_EXTRA_WHEN_WORN))
		      {
		        if (should_be_tripped(ch,trig_type,obj->objtrig[i]->pIndexData,special,arg,victim))
			  activate_trigger(obj->objtrig[i], obj, ch, tch);
		      }
		      else if(IS_SET(obj->objtrig[i]->pIndexData->extra_flags, OT_EXTRA_WHEN_TRIGGERER_WORN)
		          && tch == ch)
		      {
		        if (should_be_tripped(ch,trig_type,obj->objtrig[i]->pIndexData,special,arg,victim))
 		          activate_trigger(obj->objtrig[i], obj, ch, victim);
		      }
		    }
		  else if(IS_SET(obj->objtrig[i]->pIndexData->extra_flags, OT_EXTRA_WHEN_CARRIED))
		  {
 		    if (should_be_tripped(ch,trig_type,obj->objtrig[i]->pIndexData,special,arg,victim))
		      activate_trigger(obj->objtrig[i], obj, ch, tch);
		  }
		  else if(IS_SET(obj->objtrig[i]->pIndexData->extra_flags, OT_EXTRA_WHEN_TRIGGERER_CARRIED)
		      && tch == ch)
		  {
		    if (should_be_tripped(ch,trig_type,obj->objtrig[i]->pIndexData,special,arg,victim))
 		      activate_trigger(obj->objtrig[i], obj, ch, victim);
		  }
		}
	    }
	}
    }
  if (trig_type == OBJ_TRIG_ENTER_ROOM)
   check_gravity_char(ch);
}





void trip_triggers_special(CHAR_DATA *ch, long trig_type, OBJ_DATA *obj, 
		CHAR_DATA *victim, int special, OBJ_TRIG* ot)
{
  CHAR_DATA *tch;
  ROOM_INDEX_DATA *inroom = NULL;
  if(trig_type == OBJ_TRIG_ON_TICK)
  {
    if (ot->obj_on && ot->obj_on->in_room)
      inroom = ot->obj_on->in_room;
    else if (ot->obj_on->carried_by && ot->obj_on->carried_by->in_room)
      inroom = ot->obj_on->carried_by->in_room;
    if (inroom)
    {
    
      for (tch = inroom->people; tch != NULL; tch = tch->next_in_room)
      {
        if ((!IS_NPC(tch) && IS_SET(ot->pIndexData->extra_flags,OT_EXTRA_ON_PC))
	   || (IS_NPC(tch) && IS_SET(ot->pIndexData->extra_flags,OT_EXTRA_ON_NPC)))
        {
	  if (ot->obj_on->carried_by)
	  {
 	    if ((ot->obj_on->carried_by == tch && ot->obj_on->wear_loc != WEAR_NONE))
	    {
 	      if (IS_SET(ot->pIndexData->extra_flags, OT_EXTRA_WHEN_WORN))
                activate_trigger(ot, ot->obj_on, ot->obj_on->carried_by, tch);
	    }	    
	    else if (IS_SET(ot->pIndexData->extra_flags, OT_EXTRA_WHEN_CARRIED))
 	      activate_trigger(ot, ot->obj_on, ot->obj_on->carried_by, tch);
	  }
	  else if (IS_SET(ot->pIndexData->extra_flags, OT_EXTRA_WHEN_ON_GROUND))
	    activate_trigger(ot, ot->obj_on, tch, tch);
	  break;
	}
      }
    }
  }
}

void activate_trigger(OBJ_TRIG *ot, OBJ_DATA *obj, CHAR_DATA *ch, CHAR_DATA *victim)
{
	OBJ_TRIG_DATA *otd=ot->pIndexData;
	if ((number_percent() > otd->overall_chance && otd->overall_chance != 100) || ot->seconds_left > 0 || ot->statement_on != 0)
  	  return;
	if (otd->delay[0] > 0)
	{
	  ot->statement_on = 0;
	  ot->seconds_left = otd->delay[0];
	  ot->next = ot_list;
	  ot_list = ot;
	  ot->ch = ch;
	  ot->victim = victim;
	}
	else activate_trigger_statement(0,ot,obj,ch,victim);
	return;
}

void activate_trigger_statement(int statement_on, OBJ_TRIG *ot, OBJ_DATA *obj, CHAR_DATA *ch, CHAR_DATA *victim)
{
	int i=0, pos=0;
        OBJ_TRIG_DATA *otd=ot->pIndexData;
	OBJ_TRIG *otp = NULL;
	disguise_fakie(otd, obj);
	pos = ch->position;
	if(otd->action[statement_on] != NULL && number_percent() < otd->chance[statement_on]+1)
	  interp_action(otd->action[statement_on], otd, obj, ch, victim);
        for (i = statement_on+1;i<16;i++)
        {
	  if (otd->delay[i] > 0)
	  {
	    if (ot->statement_on == 0)
	    {
	      ot->next = ot_list;
	      ot_list = ot;
	    }
	    ot->statement_on = i;
	    ot->seconds_left = otd->delay[i];
	    ot->ch = ch;
	    ot->victim = victim;
	    if (ch->fighting == NULL)
	    {
	      if (pos == POS_MOUNTED && ch->mount == NULL)
 	        ch->position = ch->position; //do nothing
	      else if (pos != POS_FIGHTING)
   	        ch->position = pos;
	      else ch->position = POS_STANDING;
	      
	    }
	     
	    break;
	  }
          if(otd->action[i] != NULL && number_percent() < otd->chance[i]+1)
            interp_action(otd->action[i], otd, obj, ch, victim);
          if (ch->fighting == NULL)
	  {
  	    if (pos == POS_MOUNTED && ch->mount == NULL)
	      ch->position = ch->position; //do nothing
	    else if (pos != POS_FIGHTING)
  	      ch->position = pos;
	    else ch->position = POS_STANDING;
	  }
        }
	if (i >= 16 && ot_list != NULL)
	  {
	    if (ot_list == ot)
              ot_list = ot->next;
	    else if (ot_list->next == ot)
	      ot_list->next = ot->next;
	    else
	    {
	      for (otp = ot_list;otp->next != NULL;otp = otp->next)
	        {
	          if (otp->next == ot)
	            {
	              otp->next = ot->next;
	              break;
	            }
	         }
	    }
	    ot->statement_on = 0;
	  }
        revert_fakie(otd);
}

void interp_action(char *action, OBJ_TRIG_DATA *ot, OBJ_DATA *obj,
				   CHAR_DATA *ch, CHAR_DATA *victim)
{
	char *raw_action;
	char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
	if ((obj->carried_by == NULL || obj->carried_by->in_room == NULL) && obj->in_room == NULL)
  	  return;
        if (obj_trig_in_progress)  	//A pitiful attempt at function isolation/synchronization
         return;
        else obj_trig_in_progress = TRUE;
	GLOBAL_ot_ch = ch;
	GLOBAL_ot_obj = obj;
	raw_action = NULL;
	strcpy(buf2,action);
	raw_action = buf2;
	variable_substitution_new(ch,ch->fighting,obj->carried_by,victim,raw_action);
	if(obj->carried_by == NULL) 
	{
  	  if (obj->in_room)
	    char_to_room(ot->fakie, obj->in_room);
	}
	else
	{
	  if (obj->carried_by->in_room)
	    char_to_room(ot->fakie, obj->carried_by->in_room);
	}
        sprintf(buf, "%s", raw_action);
        interpret(ot->fakie, buf);
	char_from_room(ot->fakie);
	GLOBAL_ot_ch = NULL;
	GLOBAL_ot_obj = NULL;
	obj_trig_in_progress = FALSE;
}



char *translate_var(char *action, OBJ_DATA *obj, CHAR_DATA *ch,
					CHAR_DATA *victim)
{
	char *raw_action;
	raw_action = str_dup(action);
	variable_substitution(ch,raw_action);
	return raw_action;
}

void disguise_fakie(OBJ_TRIG_DATA *ot, OBJ_DATA *obj)
{
	free_string(ot->fakie->short_descr);
	ot->fakie->short_descr = str_dup(obj->short_descr);
	ot->fakie->level = obj->level;
	ot->fakie->trust = 100;
	SET_BIT (ot->fakie->comm, COMM_TRUE_TRUST);
	return;
}

void revert_fakie(OBJ_TRIG_DATA *ot)
{
	return;
	free_string(ot->fakie->short_descr);
	ot->fakie->short_descr = str_dup("The Fakie");
	return;
}

// find a obj trigger with desired vnum
OBJ_TRIG_DATA *get_obj_trig(int vnum)
{
	OBJ_TRIG_DATA *pObjTrig;
	AREA_DATA *pArea;
	if ( (pArea = get_vnum_area (vnum)) == NULL)
	{
	  //send_to_char("That vnum is not assigned an area.\n\r",ch);
	  return FALSE;
	}
	
	for (pObjTrig = pArea->ot_first; pObjTrig != NULL; pObjTrig = pObjTrig->next)
	{
          if (pObjTrig->vnum == vnum)
	    return pObjTrig;
        } 

	return NULL;
}

bool contains_text(char *string, char *text)
{
	char strword[MAX_STRING_LENGTH];
	char textword[MAX_STRING_LENGTH];
	
	text = one_argument(text, textword);

	do
	{
		string = one_argument(string, strword);

		if(!str_cmp(strword, textword))
		{
			// if it successfully evaluates every word in text then it
			//    does indeed contain the text.
			if(text[0] == '\0')
				return TRUE;
			text = one_argument(text, textword);
		}
			
	} while(string[0] != '\0');

	return FALSE;
}


// OLC Obj Trig Editor functions
OTEDIT (otedit_create)
{
  //     MOB_INDEX_DATA *pMob;
      AREA_DATA *pArea;
      OBJ_TRIG_DATA *pObjTrig, *ot;
      int value;
      bool inserted = FALSE;
      //int iHash;
      //int i=0;
      value = atoi (argument);

      /* OLC 1.1b */
      if (argument[0] == '\0' || value <= 0 || value >= INT_MAX)
	{
	  char output[MAX_STRING_LENGTH];
	  sprintf (output, "Syntax:  create [0 < vnum < %d]\n\r", INT_MAX);
	  send_to_char (output, ch);
	  return FALSE;
	}
      pArea = get_vnum_area (value);
      if (!pArea)
	{
	  send_to_char ("That vnum is not assigned an area.\n\r", ch);
	  return FALSE;
	}
      if (!is_builder (ch, pArea))
	{
	  send_to_char ("Vnum in an area you cannot build in.\n\r", ch);
	  return FALSE;
	}

	if(get_obj_trig(value))
	{
		send_to_char("Object Trigger vnum already in use, choose another.\n\r", ch);
		return FALSE;
	}

	pObjTrig = new_obj_trig();
	pObjTrig->vnum = value;

	pObjTrig->fakie = create_mobile(get_mob_index(MOB_VNUM_FAKIE));
	
	if (pArea->ot_first == NULL)
	{
	  pObjTrig->next = pArea->ot_first;
	   pArea->ot_first = pObjTrig;
	}
	else if (pArea->ot_first->next == NULL)
	{
		if (pObjTrig->vnum <  pArea->ot_first->vnum)
		{
			pObjTrig->next =  pArea->ot_first;
			pArea->ot_first = pObjTrig;
		}
		else
		{
			pObjTrig->next = pArea->ot_first->next;
			pArea->ot_first->next = pObjTrig;
		}
	}
	else
	{
		if (pObjTrig->vnum < pArea->ot_first->vnum)
                {
                        pObjTrig->next = pArea->ot_first;
                        pArea->ot_first = pObjTrig;
                }
		else
		{
		  for (ot = pArea->ot_first;ot->next != NULL;ot = ot->next)
		  {
			  if (pObjTrig->vnum < ot->next->vnum)
			  {
				  pObjTrig->next = ot->next;
				  ot->next = pObjTrig;
				  inserted = TRUE;
				  break;
			  }
		  }
			  if (!inserted)
			  {
				  ot->next = pObjTrig;
				  pObjTrig->next = NULL;
			  }
			  


		}
	}
	

	//Iblis 11/07/03 - Ok his hash implementation was NOT finished (IE saving/loading
	//only load from the list, which only contains the MOST RECENT trigger), so I'm like
	//fuck it, comment out hash shit for now
/*	iHash = value % MAX_KEY_HASH;
	pObjTrig->next = obj_trig_hash[iHash];
	obj_trig_hash[iHash] = pObjTrig;*/
	ch->desc->pEdit = (void *) pObjTrig;

	send_to_char("Object Trigger Created.\n\r", ch);
	return TRUE;
}

OTEDIT (otedit_show)
{
	OBJ_TRIG_DATA *pObjTrig;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	bool found = FALSE;

	int i;

	buf2[0] = '\0';

	EDIT_OBJ_TRIG(ch, pObjTrig);
	
	sprintf(buf, "Name: [%s]\n\r", pObjTrig->name);
	send_to_char(buf, ch);
	sprintf(buf, "Vnum: [%d]\n\r", pObjTrig->vnum);
	send_to_char(buf, ch);
	sprintf(buf, "Chance of trigger being tripped: [%d percent]\n\r", pObjTrig->overall_chance);
	send_to_char(buf,ch);
	sprintf(buf, "Trigger Flags: [%s]\n\r", obj_trig_bit_name(pObjTrig->trig_flags));
	send_to_char(buf, ch);
	sprintf(buf, "Trigger String: [%s]\n\r", pObjTrig->trigger);
	send_to_char(buf, ch);
	if (pObjTrig->extra_flags != 0)
  	  sprintf(buf, "Extra Flags: [%s]\n\r\n\r",flag_string (ot_extra_flags, pObjTrig->extra_flags));
	else sprintf(buf, "Extra Flags: [object_only]\n\r\n\r");
	
	send_to_char(buf,ch);
	send_to_char("Action Strings:\n\r", ch);
	send_to_char("No | Chnc | Dly | Action String\n\r",ch);
	for (i = 0;i<16;i++)
	{
	  if(pObjTrig->action[i] != NULL)
	  {
	    sprintf(buf, "%d) %d %d - %s\n\r", i, pObjTrig->chance[i], pObjTrig->delay[i], pObjTrig->action[i]);
	    send_to_char(buf, ch);
	    found = TRUE;
	  }
	}
	
	if(!found)
		send_to_char("*** No Actions Defined\n\r", ch);
	

	return FALSE;
}

OTEDIT (otedit_chance)
{
	OBJ_TRIG_DATA *pObjTrig;
	int value;

	EDIT_OBJ_TRIG(ch, pObjTrig);
	
	if(argument == '\0' || !is_number(argument))
	{
		send_to_char("Syntax: chance %\n\r", ch);
		return FALSE;
	}

	value = atoi(argument);
	if (value < 0 || value > 100)
	{
	  send_to_char("Syntax: chance % (a % is between 0 and 100 genius)\n\r", ch);
	  return FALSE;
	}
	pObjTrig->overall_chance = value;
	send_to_char("Chance successfully set.\n\r", ch);

	return TRUE;
}


OTEDIT (otedit_name)
{
	OBJ_TRIG_DATA *pObjTrig;

	EDIT_OBJ_TRIG(ch, pObjTrig);

	if(argument[0] == '\0')
	{
		send_to_char("Syntax: name <name of trigger>\n\r", ch);
		return FALSE;
	}

	free_string(pObjTrig->name);
	pObjTrig->name = str_dup(argument);

	send_to_char("Trigger name set.\n\r", ch);
	return TRUE;
}

OTEDIT (otedit_trigger)
{
	OBJ_TRIG_DATA *pObjTrig;
	bool on_tick=FALSE;

	EDIT_OBJ_TRIG(ch, pObjTrig);

	if(argument[0] == '\0')
	{
		send_to_char("Syntax: trigger <trigger script string>\n\r", ch);
		return FALSE;
	}

	free_string(pObjTrig->trigger);
	pObjTrig->trigger = str_dup(argument);
        if (IS_SET(pObjTrig->trig_flags,OBJ_TRIG_ON_TICK))
	  on_tick = TRUE;
	set_trigger_flags(pObjTrig);
	if (IS_SET(pObjTrig->trig_flags,OBJ_TRIG_ON_TICK))
	  {
	    if (!on_tick)
	      {
		      
		OBJ_TRIG* otp;
		for (otp = giant_ot_list;otp != NULL;otp = otp->next_ot)
		{
			if (otp->pIndexData == pObjTrig)
			{
			  otp->next_on_tick = on_tick_ot_list;
		          on_tick_ot_list = otp;
			}
		}
	      }
	  }
	else if (on_tick)
	{
  	  OBJ_TRIG *otp,*ot;
	  for (ot = giant_ot_list;ot != NULL;ot = ot->next_ot)
	  {
	    if (ot->pIndexData == pObjTrig)
	    {
	      if (on_tick_ot_list == ot)
	      {
		on_tick_ot_list = ot->next_on_tick;
		ot->next_on_tick = NULL;
	      }
	      else if (on_tick_ot_list && on_tick_ot_list->next_on_tick == ot)
	      {
		on_tick_ot_list->next_on_tick = ot->next_on_tick;
		ot->next_on_tick = NULL;
	      }
	      else
		{
		bool found = FALSE;
		  for (otp = on_tick_ot_list;otp->next_on_tick != NULL;otp = otp->next_on_tick)
		    {
		      if (otp->next_on_tick == ot)
			{
			  otp->next_on_tick = ot->next_on_tick;
			  ot->next_on_tick = NULL;
			  found = TRUE;
			  break;
			}
		  }
		  if (!found)
		    {
		    bug("object trigger to be freed NOT FOUND on on_tick_ot_list.",0);
		    return TRUE;
		    }
		}
	    }
	  }
	}
	    
	    
	send_to_char("Trigger script string set.\n\r", ch);
	return TRUE;
}

OTEDIT (otedit_addaction)
{
	OBJ_TRIG_DATA *pObjTrig;
	bool found = FALSE;
	char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	short argnum, argnum2, i;

	EDIT_OBJ_TRIG(ch, pObjTrig);

	if(argument[0] == '\0')
	{
		send_to_char("Syntax: addaction %ofhitting delay <action script string>\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument,arg);
	argument = one_argument(argument,arg2);
	if (!is_number(arg) || argument[0] == '\0')
        {
	        send_to_char("Syntax: addaction %ofhitting delay <action script string>\n\r", ch);
	        return FALSE;
	}
	argnum = atoi(arg);
	if (argnum < 0 || argnum > 100)
	{
		send_to_char("The chance of hitting must be between 0 and 100.\n\r",ch);
		return FALSE;
	}
	argnum2 = atoi(arg2);
	if (argnum2 < 0 || argnum2 > 999)
	{
	        send_to_char("The seconds delay must be between 0 and 999.\n\r",ch);
		return FALSE;
	}
	for (i=0;i<16;i++)
	{
	  if(pObjTrig->action[i] == NULL)
          {
            pObjTrig->action[i] = str_dup(argument);
            pObjTrig->chance[i] = argnum;
	    pObjTrig->delay[i] = argnum2;
            found = TRUE;
	    break;
          }

	}

	if(!found)
	{
		send_to_char("Maximum number of actions already defined.\n\r", ch);
		return FALSE;
	}

	send_to_char("Action added successfully.\n\r", ch);
	return TRUE;
}

OTEDIT (otedit_remaction)
{
	OBJ_TRIG_DATA *pObjTrig;
	int value, i;
	bool found = FALSE;

	EDIT_OBJ_TRIG(ch, pObjTrig);

	if(argument[0] == '\0' || !is_number(argument))
	{
		send_to_char("Syntax: remaction <action string #>\n\r", ch);
		return FALSE;
	}

	value = atoi(argument);


	for (i=value;i<16;i++)
	{
		if (pObjTrig->action[i] != NULL)
		  found = TRUE;
		if (i==value)
		  free_string(pObjTrig->action[i]);
		if (i == 15)
		{
		  pObjTrig->action[i] = NULL;
		  pObjTrig->chance[i] = 0;
		  pObjTrig->delay[i] = 0;
		}
		else
		{
		  pObjTrig->action[i] = pObjTrig->action[i+1];
		  pObjTrig->chance[i] = pObjTrig->chance[i+1];
		  pObjTrig->delay[i] = pObjTrig->delay[i+1];
		}
	}
	

	if(!found)
	{
		send_to_char("That is not a valid action string number.\n\r", ch);
		return FALSE;
	}

	send_to_char("Action removed successfully.\n\r", ch);
	return TRUE;
}

void do_otlist(CHAR_DATA* ch, char* argument)
{
  OBJ_TRIG_DATA* pObjTrig;
  char buf[MAX_STRING_LENGTH], tcbuf[MAX_STRING_LENGTH*20];
  strcpy (tcbuf, "Obj Trigger Listing:\n\r");
  for (pObjTrig = ch->in_room->area->ot_first; pObjTrig != NULL; pObjTrig = pObjTrig->next)
  {
    sprintf(buf,"[%d] - %s\n\r",pObjTrig->vnum, pObjTrig->name);
    strcat(tcbuf,buf);
  }
  page_to_char (tcbuf, ch);
}

OTEDIT (otedit_purge)
{
  OBJ_TRIG_DATA *pObjTrig, *ot;
  int value;
  bool inserted = FALSE;
  AREA_DATA *pArea;

  value = atoi (argument);

  if(argument[0] == '\0' || value <= 0 || value >= INT_MAX)
    {
      char buf[MAX_STRING_LENGTH];

      sprintf(buf, "Syntax: purge <vnum 1 - %d>\n\r", INT_MAX);
      send_to_char(buf, ch);
      return FALSE;
    }

  if(!get_obj_trig(value))
    {
      send_to_char("Object Trigger does not currently exist.\n\r", ch);
      return FALSE;
    }

  if (ch->desc->pEdit == (void *) get_obj_trig(value))
    edit_done (ch);

  if ((pArea = get_vnum_area(value)) == NULL)
  {  
    send_to_char ("That vnum is not assigned an area.\n\r", ch);
    return FALSE;
  }



  if (pArea->ot_first == NULL)
    {
      send_to_char("Object Trigger does not currently exist.\n\r", ch);
      return FALSE;
    }
  else if (pArea->ot_first->next == NULL)
    {
      if (value == pArea->ot_first->vnum)
	{
		OBJ_TRIG *ot2, *ot2_next;
		pObjTrig = pArea->ot_first;
		for (ot2 = giant_ot_list;ot2 != NULL;ot2 = ot2_next)
		{
  		  ot2_next = ot2->next_ot;
		  if (ot2->pIndexData == pObjTrig)
 		    free_ot(ot2);
		}
		free_obj_trig(pObjTrig);
		pArea->ot_first = NULL;
		inserted = TRUE;
	}
      else
	{
	  send_to_char("Object Trigger does not currently exist.\n\r", ch);
	  return FALSE;
	}
    }
  else
    {
      if (value == pArea->ot_first->vnum)
	{
  	  OBJ_TRIG *ot2, *ot2_next;
	  pObjTrig = pArea->ot_first;
	  for (ot2 = giant_ot_list;ot2 != NULL;ot2 = ot2_next)
	  {
	    ot2_next = ot2->next_ot;
	    if (ot2->pIndexData == pObjTrig)
	      free_ot(ot2);
	  }
	  pArea->ot_first = pArea->ot_first->next;
	  free_obj_trig(pObjTrig);
	  inserted = TRUE;
	}
      else
	{
	  for (ot = pArea->ot_first;ot->next != NULL;ot = ot->next)
	    {
	      if (value == ot->next->vnum)
		{
		  OBJ_TRIG *ot2, *ot2_next;
		  pObjTrig = ot->next;
		  for (ot2 = giant_ot_list;ot2 != NULL;ot2 = ot2_next)
		  {
		    ot2_next = ot2->next_ot;
		    if (ot2->pIndexData == pObjTrig)
		      free_ot(ot2);
		  }
		    
		  ot->next = ot->next->next;
		  free_obj_trig(pObjTrig);
		  inserted = TRUE;
		  break;
		}
	    }
	  if (!inserted)
	    {
	      send_to_char("Object Trigger does not currently exist.\n\r", ch);
              return FALSE;
	    }
	  
	  
	  
	}
    }
  
  
  //Iblis 11/07/03 - Ok his hash implementation was NOT finished (IE saving/loading
  //only load from the list, which only contains the MOST RECENT trigger), so I'm like
  //fuck it, comment out hash shit for now
  /*      iHash = value % MAX_KEY_HASH;
        pObjTrig->next = obj_trig_hash[iHash];
        obj_trig_hash[iHash] = pObjTrig;*/
//  ch->desc->pEdit = (void *) pObjTrig;

  send_to_char("Object Trigger Successfully Purged.\n\r", ch);
  send_to_char("MAKE SURE YOU REMOVED OR HAVE REMOVED all objects with this vnum as their object trigger.\n\r",ch);
  return TRUE;
}


// OLC Obj Trig Editor functions
OTEDIT (otedit_extra)
 {
   
   OBJ_TRIG_DATA *pObjTrig;
   int value;

   EDIT_OBJ_TRIG(ch, pObjTrig);

   if(argument[0] == '\0')
     {
       send_to_char("Syntax: extra EXTRAFLAG\n\r", ch);
       send_to_char("   where EXTRAFLAG = \n\r",ch);
       send_to_char("worn carried on_ground triggerer_worn triggerer_carried\n\r",ch);
       send_to_char("on_npc on_pc\n\r",ch);
       return FALSE;
     }

   if ((value = flag_value (ot_extra_flags, argument)) != NO_FLAG)
     {
       TOGGLE_BIT (pObjTrig->extra_flags, value);
       send_to_char ("Extra flag toggled.\n\r", ch);
       return TRUE;
     }
     send_to_char("Syntax: extra EXTRAFLAG\n\r", ch);
     send_to_char("   where EXTRAFLAG = \n\r",ch);
     send_to_char("worn carried on_ground triggerer_worn triggerer_carried\n\r",ch);
     send_to_char("on_pc on_npc\n\r",ch);
     return FALSE;
 }
   
	   

void ot_set_bits(OBJ_TRIG_DATA* ot, char *argument)
{
  int value = 0;
  if (argument != NULL)
    value = flag_value (ot_extra_flags, argument);
  REMOVE_BIT(ot->extra_flags,OT_EXTRA_WHEN_WORN);
  REMOVE_BIT(ot->extra_flags,OT_EXTRA_WHEN_CARRIED);
  REMOVE_BIT(ot->extra_flags,OT_EXTRA_WHEN_ON_GROUND);
  REMOVE_BIT(ot->extra_flags,OT_EXTRA_WHEN_TRIGGERER_CARRIED);
  REMOVE_BIT(ot->extra_flags,OT_EXTRA_WHEN_TRIGGERER_WORN);
  REMOVE_BIT(ot->extra_flags,OT_EXTRA_ON_NPC);
  SET_BIT(ot->extra_flags,OT_EXTRA_ON_PC);
  TOGGLE_BIT (ot->extra_flags, value);
}
	

void do_otlistall(CHAR_DATA* ch, char* argument)
{
  OBJ_TRIG_DATA* pObjTrig;
  char buf[MAX_STRING_LENGTH], tcbuf[MAX_STRING_LENGTH*20];
  int i=0;
  //  AREA_DATA *pArea;
  strcpy (tcbuf, "Obj Trigger Listing:\n\r");
  //Iblis 9/13/04 - Can't find the MAx vnum, so set it to 65,000.  So sue me.
  for (i = 0; i < 65000; ++i)
    {
      //      for (pObjTrig = pArea->ot_first; pObjTrig != NULL; pObjTrig = pObjTrig->next)
	pObjTrig = get_obj_trig(i);
      if (pObjTrig == NULL)
	continue;
	  sprintf(buf,"[%d] - %s\n\r",pObjTrig->vnum, pObjTrig->name);
	  strcat(tcbuf,buf);
	  //	}
    }
  page_to_char (tcbuf, ch);
}
