//Traps- Paarshad 
//
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
#include "clans/new_clans.h"

#define MAX_NUM_TRAPS 5

extern bool crit_strike_possible;
// Trap type, Spectator message, Victim message, Num Dice, Dice Size, Dice modified (ie +23 damage), Dam Type 

const struct trap_info_type trapinfo [] = {
 {TRAP_O_PLAIN, 0,
 "A trap goes off.``\n\r",
 "A trap goes off causing you immense pain.``\n\r",
  16,7,10, "slice" //AVG 84
 },
 {TRAP_O_BLADE, 0,
 "`bIn the blink of an eye tiny razor blades fly past you leaving tiny cuts all over $n.``\n\r",
 "`bRazor blades explode towards you, cutting you badly.``\n\r",
  16,7,10, "slice" //AVG 84
 },
 {TRAP_O_NEEDLE, TFLAG_POISON ,
 "From a concealed box, a needle shoots out embedding itself in $n's skin.\n\r",
 "A needle stabs into your skin, causing you to feel ill.\n\r",
  8,7,5, "pierce" // AVG 42
 },
 {TRAP_O_BEE, TFLAG_BEE ,
 "A jar of bees shatters as they swarm around.\n\r",
 "A jar of bees shatters as the begin stinging you.\n\r",
 12,9,15, "bite" //AVG 90
 },
 {TRAP_O_EXPLODING, TFLAG_AREA, 
 "Flames encircle the room burning everything.\n\r",
 "Flames rush towards you searing your flesh.\n\r",
 8,7,10, "flame" // AVG 76
 },
 {TRAP_P_PLAIN, 0,
 "A trap goes off.``\n\r",
 "A trap goes off causing you immense pain.``\n\r",
  16,7,10, "slice" //AVG 84
 },
 {TRAP_P_ROCK, TFLAG_KNOCK, 
 "As $n walks forward, an onslaught of rocks falls on $m.\n\r",
 "A grinding sound is soon accompanied by rocks falling on your head.\n\r",
 16,9,9, "crush" // AVG 98
 },
 {TRAP_P_DART, TFLAG_DISORIENT, 
 "A dart shoots from a corner sticking into $n's neck.\n\r",
 "A dart flies into your neck, causing the world to spin.\n\r",
 10,5,10, "pierce" //AVG 50
 },
 {TRAP_P_EXPLODING, TFLAG_AREA,
 "Flames encircle the room burning everything.\n\r",
 "Flames rush towards you searing your flesh.\n\r",
 18,9,10, "flame" // AVG 110
 },
 {TRAP_P_SPIKES, 0,
 "A pair of masterfully aligned spikes leap from the doorway, impaling %n.\n\r",
 "The taste of blood enters your mouth as two spikes impale you.\n\r",
 20,7,16, "stab" // AVG 112
 },
 {TRAP_P_ALARM,0,"","",0,0,0,"bash"},
 {TRAP_R_PLAIN, 0,
 "A trap goes off.``\n\r",
 "A trap goes off causing you immense pain.``\n\r",
  16,7,10, "slice" //AVG 84
 },
 {TRAP_R_SPIKEPIT, 0,
 "`bThe ground gives way under $n.``\n\r",
 "`bThe ground gives way under your feet and you fall into a bed of spikes.``\n\r",
 20,11,13, "stab" // AVG 146
 },
 {TRAP_R_LOG, 0,
 "`bA huge log slices through the air, hitting $n square in the chest knocking $m back.``\n\r",
 "`bPains grips your entire body as a large log smacks you square in the chest.``\n\r",
 16,13,11, "smash" // AVG  134
 },
 {TRAP_R_LANDMINE, TFLAG_AREA,
 "`bShrapnel flies through the room, barely missing you.``\n\r",
 "`bA subtle click, a second before, warned of the maelstrom of shrapnel flying through you..``\n\r",
 18,9,17, "flame" // AVG 124
 },
 {TRAP_R_SPIKESNARE, TFLAG_SNARE, 
 "`bA thin coil of barbed wire wraps around $n's leg lifting $m into the air.``\n\r",
 "`bThe world turns upside as the flesh on your leg is torn upwards over your head.``\n\r",
 6,5,10, "stab" //AVG 36
 },
 {TRAP_R_ALARM,0,"","",0,0,0,"bash"},
 {-1,0,NULL,NULL,0,0,0,NULL}
};

int trap_info_lookup(int traptype)
{
  int flag;
  for (flag = 0; trapinfo[flag].type != -1; flag++)
  	if (trapinfo[flag].type == traptype)
		return flag;
  return NO_FLAG;
}
char * get_spec_mess(OBJ_DATA *obj)
{
char *smess;
EXTRA_DESCR_DATA *ed;

	 // Get Spectator message or put one in from the trap type
	for (ed = obj->extra_descr; ed; ed = ed->next)
	{
		if (is_name ("specmessage", ed->keyword) )
		break;
	}
	smess = ed->description;
	if (smess == NULL)
	{
		smess = strdup("(smess) HRM Paarshad should really put something here.\n\r");
	}
return smess;	
}
char * get_vict_mess(OBJ_DATA *obj)
{
char *vmess;
EXTRA_DESCR_DATA *ed;

	 // Get Spectator message or put one in from the trap type
	for (ed = obj->extra_descr; ed; ed = ed->next)
	{
		if (is_name ("victmessage", ed->keyword) )
		break;
	}
	vmess = ed->description;
	if (vmess == NULL)
	{
		vmess = strdup("(vmess) HRM Paarshad should really put something here.\n\r");
	}
return vmess;	
}
void spring_trap (CHAR_DATA *victim, OBJ_DATA *obj);

void trap_check(CHAR_DATA *victim,char *type, ROOM_INDEX_DATA *room, OBJ_DATA *obj)
{
OBJ_DATA *thing,*next_thing;
	if (IS_IMMORTAL(victim))
		return;
	if(room != NULL && !strcmp(type,"room"))
	{
	
		for (thing = room->contents; thing != NULL; thing = next_thing)  // BTW WIZI means its set
		{
			next_thing = thing->next_content;
			if ( (thing->item_type == ITEM_ROOM_TRAP) 
			     && IS_SET(thing->extra_flags[1],ITEM_WIZI) 
			     && ((number_percent () > (get_skill (victim, gsn_avoid_traps))*0.75 + victim->mod_stat[STAT_DEX] - thing->value[5])
			     || IS_SET(thing->value[12],TFLAG_UNAVOID) ))
			{		
				spring_trap(victim,thing);
				return;
			}
		}
	}
	if(room != NULL && 
	(
	 !strcmp(type,"door")||!strcmp(type,"north")||!strcmp(type,"east")
	 ||!strcmp(type,"south")||!strcmp(type,"west")||!strcmp(type,"up")
	 ||!strcmp(type,"down")))
	{

		for (thing = room->contents; thing != NULL; thing = next_thing)  // BTW WIZI means its set
		{
			
			next_thing = thing->next_content;
			if ( (  (thing->item_type == ITEM_PORTAL_TRAP))
			     && ( (thing->value[9] == flag_value (dir_flags, type)))
			     && IS_SET(thing->extra_flags[1],ITEM_WIZI)
			     && ((number_percent () > (get_skill (victim, gsn_avoid_traps))*0.75 + victim->mod_stat[STAT_DEX] - thing->value[5])
			     || IS_SET(thing->value[12],TFLAG_UNAVOID) ))
			{		
				spring_trap(victim,thing);
				return;
			}
		}
	
	}
	if(room != NULL && 
	(
	 !strcmp(type,"enter")||!strcmp(type,"exit")
	 ||!strcmp(type,"climb")||!strcmp(type,"crawl")
	 ||!strcmp(type,"descend")||!strcmp(type,"scale")||!strcmp(type,"jump")
	 ||!strcmp(type,"tug")))
	{
	
		for (thing = room->contents; thing != NULL; thing = next_thing)  // BTW WIZI means its set
		{
			
			next_thing = thing->next_content;
			if ( (  (thing->item_type == ITEM_PORTAL_TRAP))
			     && ( (thing->value[7] == flag_value (pact_flags, type)))
			     && IS_SET(thing->extra_flags[1],ITEM_WIZI)
			     && ((number_percent () > (get_skill (victim, gsn_avoid_traps))*0.75 + victim->mod_stat[STAT_DEX] - thing->value[5])
			     || IS_SET(thing->value[12],TFLAG_UNAVOID) ))
			{		
				spring_trap(victim,thing);
				return;
			}
		}
	}
	if (obj != NULL && !strcmp(type,"object"))
	{
		for (thing = obj->contains; thing != NULL; thing= next_thing)
		{
			next_thing = thing->next_content;
			if ( (thing->item_type == ITEM_OBJ_TRAP)  
			     && IS_SET(thing->extra_flags[1],ITEM_WIZI) 
			     && ((number_percent () > (get_skill (victim, gsn_avoid_traps))*0.75 + victim->mod_stat[STAT_DEX] - (thing->value[5]/2))
			     || IS_SET(thing->value[12],TFLAG_UNAVOID) ))
			{
				spring_trap(victim,thing);
				return;
			}
		}
	}
	 
return;
}
void trap_damage (CHAR_DATA *victim, OBJ_DATA *obj)
{
CHAR_DATA *ch = victim;
AFFECT_DATA af;
int dam;
	if (IS_SET(obj->value[12],TFLAG_SNARE))
	{

		af.where = TO_AFFECTS;
		af.type = gsn_entangle;
		af.level = obj->level;
		af.duration = 10;
		af.modifier = 0;
		af.location = 0;
		af.bitvector = 0;
		af.permaff = FALSE;
		af.composition = FALSE;
		af.comp_name = str_dup ("");
		affect_join (victim, &af);
	}
	if (IS_SET(obj->value[12],TFLAG_POISON))
	{
	  if (saves_spell (obj->level, victim, DAM_POISON))
	    {
	      act ("$n turns slightly green, but it passes.", victim, NULL, NULL,
	           TO_ROOM);
	      send_to_char ("You feel momentarily ill, but it passes.\n\r", victim);
	    }
	  else
	  {
	  af.where = TO_AFFECTS;
	  af.type = gsn_poison;
	  af.level = obj->level;
	  af.duration = obj->level;
	  af.location = APPLY_STR;
	  af.modifier = -2;
	  af.bitvector = AFF_POISON;
	  af.permaff = FALSE;
	  af.composition = FALSE;
	  if (obj->set_by != NULL)
		  af.comp_name = str_dup (obj->set_by->name);
	  else
		  af.comp_name = str_dup ("");
	  affect_join (victim, &af);
	  send_to_char ("You feel very sick.\n\r", victim);
	  act ("$n looks very ill.", victim, NULL, NULL, TO_ROOM);
	 }
	}
	if (IS_SET(obj->value[12],TFLAG_DISORIENT))
	{
	  af.where = TO_AFFECTS;
	  af.type = gsn_disorientation;
	  af.level = obj->level;
	  af.duration = 5;
	  af.location = APPLY_HITROLL;
	  af.modifier = -4;
	  af.permaff = FALSE;
	  af.bitvector = 0;
	  af.composition = FALSE;
	  af.comp_name = str_dup ("");
	  send_to_char ("The world starts spinning around you.\n\r", victim);
	  affect_join (victim, &af);
	}
	if (IS_SET(obj->value[12],TFLAG_BEE))
	{
		if (number_range (1, 100 + get_curr_stat (victim, STAT_CON) * 2) == 50 && victim->level < 92)	
		   insta_kill (victim, victim, 0, 0, 0, 0, 1);
	}
	 if (obj->value[6])
	 {
	 CHAR_DATA *mob;
	 MOB_INDEX_DATA * pMobIndex;
		pMobIndex = get_mob_index(obj->value[6]);
			
		if (pMobIndex != NULL)
		{
  	 		mob = create_mobile(pMobIndex);
			char_to_room(mob,victim->in_room);
			multi_hit (mob, victim, TYPE_UNDEFINED);
		}
	 }
	if (obj->set_by == NULL)
            ch = victim;
	else
	    ch = obj->set_by;
        

	crit_strike_possible = FALSE;
	if (IS_SET(obj->value[12],TFLAG_EXCELLENT))
	{
	  dam = (dice(obj->value[1],obj->value[2])+obj->value[3])*1.5;
	}
	else
	  dam = (dice(obj->value[1],obj->value[2])+obj->value[3]);
	dam = adjust_damage(dam);
	damage (ch, victim, dam , 0 ,attack_table[obj->value[8]].damage , FALSE);
	crit_strike_possible = TRUE;
	if (IS_SET(obj->value[12],TFLAG_KNOCK))
	{
	      victim->pcdata->knock_time = 24;
	      stop_fighting (victim, TRUE);
	      victim->position = POS_SLEEPING;
	}
	WAIT_STATE(ch,16);
}







void spring_trap (CHAR_DATA *victim, OBJ_DATA *obj)
{
CHAR_DATA *mob;
char *smess, *vmess;


	 if ( victim == NULL || IS_IMMORTAL(victim) || (obj->item_type != ITEM_ROOM_TRAP  && obj->item_type != ITEM_OBJ_TRAP && obj->item_type != ITEM_PORTAL_TRAP ) || IS_NPC(victim) || (!((victim->pcdata->loner || victim->clan != CLAN_BOGUS)) &&(obj->set_by != NULL)) )
		return;
	 if (obj->set_by != NULL && obj->set_by == victim)
		return;

	 if (obj->value[0] == TRAP_R_SPELL || obj->value[0] == TRAP_O_SPELL|| obj->value[0] == TRAP_P_SPELL)
	 {
	 	if (skill_table[obj->value[1]].spell_fun == NULL)
			return;
	 	mob = create_mobile(get_mob_index(MOB_VNUM_FAKIE));
	 	SET_BIT (mob->act, PLR_HOLYLIGHT);
		SET_BIT(mob->act2, ACT_TRAPMOB);
		if (obj->set_by != NULL)
			mob->name = strdup(obj->set_by->name);
		mob->short_descr = strdup(obj->short_descr);	 
		mob->level = obj->value[2];
	 	char_to_room(mob,victim->in_room);	
	crit_strike_possible = FALSE;
		(*skill_table[obj->value[1]].spell_fun) (obj->value[1], obj->value[2], mob, victim, skill_table[obj->value[1]].target);
	crit_strike_possible = TRUE;
		extract_char(mob,TRUE);	
	 }
	 else
	 {
		smess = get_spec_mess(obj);
		vmess = get_vict_mess(obj);
		// AREA AFFECT TYPES
		if ( IS_SET(obj->value[12], TFLAG_AREA))  
		{
		CHAR_DATA *vch;
	    CHAR_DATA *vch_next;

		      for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
		        {
		            vch_next = vch->next_in_room;
			  		if ( !IS_NPC(vch) && (vch->pcdata->loner || vch->clan != CLAN_BOGUS || obj->set_by == NULL)) 
			  		{
						act (vmess, vch, obj, NULL, TO_CHAR);
						trap_damage(vch,obj);
			  		}
			  		else
						act (smess, vch, obj, NULL, TO_CHAR);
				  
				}
		}else //NON AREA AFFECT
		{
		   act (vmess, victim, obj, NULL, TO_CHAR);
  		   act (smess, victim, obj, NULL, TO_ROOM);
		   trap_damage(victim,obj);
		}
	 }
	 if (obj->item_type == ITEM_ROOM_TRAP || obj->item_type == ITEM_PORTAL_TRAP)
		 obj_from_room (obj);
	 else if (obj->item_type == ITEM_OBJ_TRAP)
		obj_from_obj(obj);
         extract_obj(obj);
	 
return;
}



void do_disarmtrap (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  bool found = FALSE;
  OBJ_DATA *obj=NULL,* container=NULL;
  argument = one_argument (argument, arg);
  argument = one_argument (argument, arg2);
   if ( arg[0] == '\0' || (strcmp(arg,"room") && strcmp(arg,"object") && strcmp(arg,"portal") )
        || ( ( !strcmp(arg,"object") || !strcmp(arg,"portal")) && arg2[0] == '\0') )  
  {
	  send_to_char ("Syntax: disarmtrap room\n\r",ch);
	  send_to_char ("        disarmtrap object [itemname]\n\r",ch);
	  return;
  }
if (!strcmp(arg,"room"))
    for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
    {
      if ( (obj->item_type == ITEM_ROOM_TRAP || obj->item_type == ITEM_PORTAL_TRAP) && IS_SET(obj->extra_flags[1],ITEM_WIZI))
    	break;
    }
else if (!strcmp(arg,"object"))
{
        if ((container = get_obj_carry (ch, arg2)) != NULL)
          found = TRUE;
        if (!found)
        {
                if ((container = get_obj_here (ch, arg2)) != NULL)
                        found = TRUE;
                if (!found)
                {
                        send_to_char ("You do not see that here.\n\r", ch);
                        return;
                }
        }

    for (obj = container->contains; obj != NULL; obj = obj->next_content)
    {
      if ( obj->item_type == ITEM_OBJ_TRAP && IS_SET(obj->extra_flags[1],ITEM_WIZI))
    	break;
    }
}
 if (obj == NULL)
    {
      send_to_char ("You did not find a trap.\n\r", ch);
      return;
    }
if (ch->move < ch->move*0.1)
   {
	   send_to_char ("You do not have enough energy for that.\n\r",ch);
	   return;
   }

if (obj->item_type == ITEM_OBJ_TRAP || obj->item_type == ITEM_ROOM_TRAP || obj->item_type ==
 ITEM_PORTAL_TRAP )
  {
		ch->move -= ch->max_move*.1;
                if (number_percent () < (get_skill (ch, gsn_disarm_traps)*.75+ (get_curr_stat (ch, STAT_DEX)- (obj->value[5]/2))) )
                {
                        REMOVE_BIT(obj->extra_flags[1],ITEM_WIZI);  // Trap is not active 
                        act ("$n disarms $p.", ch, obj, NULL, TO_ROOM);
                        act ("You disarm $p.", ch, obj, NULL, TO_CHAR);
                        if (obj->set_by == NULL)
			{
			         if (obj->item_type == ITEM_ROOM_TRAP)
			                 obj_from_room (obj);
			         else if (obj->item_type == ITEM_OBJ_TRAP)
			                obj_from_obj(obj);

				extract_obj(obj);
			} else  
			{
  				  if (obj->set_by != NULL)
				  {
				        if (obj == obj->set_by->trap_list)
				        {
				                obj->set_by->trap_list = obj->next_trap;
				        }
				        else
				        {
				        OBJ_DATA *prev;
			                for (prev = obj->set_by->trap_list; prev != NULL; prev = prev->next_trap)
			                {
		                        if (prev->next_trap == obj)
		                        {
		                                prev->next_trap = obj->next_trap;
		                                break;
		                        }
			          }
			          if (prev == NULL)
	        	          {
		                        bug ("do_disarmtrap: trap not found.", 0);
	                  	        return;
			          }
		        }
		}

			}

                        WAIT_STATE(ch,16);
                }else
                {
                        send_to_char("You fail and the trap fires!\n\r",ch);
			act (get_vict_mess(obj), ch, obj, NULL, TO_CHAR);
	                act (get_spec_mess(obj), ch, obj, NULL, TO_ROOM);
                       	trap_damage(ch,obj);
                        if (obj->item_type == ITEM_ROOM_TRAP)
 		                obj_from_room (obj);
		        else if (obj->item_type == ITEM_OBJ_TRAP)
                		obj_from_obj(obj);

			extract_obj(obj);
                }


  }

 
  return;
}
void do_install (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  int count=0;
  OBJ_DATA *obj,*list, *container=NULL;
  bool found;
  argument = one_argument (argument, arg);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);

  found = FALSE;
  if (!ch->pcdata->loner)
  {
	send_to_char ("The god of peace prevents you from setting traps.\n\r",ch);
	return;
  }
  if ( arg2[0] == '\0' || (strcmp(arg,"room") && strcmp(arg,"object") && strcmp(arg,"portal") )
	|| ( ( !strcmp(arg,"object") || !strcmp(arg,"portal")) && arg3[0] == '\0') )
    {
      send_to_char ("Syntax: Install room [trapname]\n\r", ch);
      send_to_char ("        Install [portal/object] [trapname] [direction/exitname/itemname]\n\r",ch);
      return;
    }
  
  if ((obj = get_obj_carry (ch, arg2)) == NULL)
    {
      send_to_char ("You do not have that trap.\n\r", ch);
      return;
    }
  if (!strcmp(arg,"object") &&  obj->item_type != ITEM_OBJ_TRAP  )
  {
	send_to_char("That is not an object trap.\n\r",ch);
	return;
  }
  if (!strcmp(arg,"room") &&  obj->item_type != ITEM_ROOM_TRAP )
  {
	send_to_char("That is not a room trap.\n\r",ch);
	return;
  }
  if (!strcmp(arg,"portal"))
  {
	if (obj->item_type != ITEM_PORTAL_TRAP )
	{
		send_to_char("That is not a portal trap.\n\r",ch);
		return;
	}
 	else if( !strcmp(arg3,"door")||strcmp(arg3,"north")||!strcmp(arg3,"east")
         ||!strcmp(arg3,"south")||!strcmp(arg3,"west")||!strcmp(arg3,"up")
         ||!strcmp(arg3,"down"))
		obj->value[9] = flag_value (dir_flags, arg3); 
	else if(!strcmp(arg3,"enter")||!strcmp(arg3,"exit")
            ||!strcmp(arg3,"climb")||!strcmp(arg3,"crawl")
            ||!strcmp(arg3,"descend")||!strcmp(arg3,"scale")||!strcmp(arg3,"jump")
            ||!strcmp(arg3,"tug")) 
	  	obj->value[7] = flag_value(pact_flags,arg3);
	else
	{
		send_to_char("Available exitnames are enter, exit, climb, crawl, descend, scale, jump, and tug.\n\r",ch);
		return;
	}

  }
   
  if (!strcmp(arg,"object"))
{
	if ((container = get_obj_carry (ch, arg3)) != NULL)
          found = TRUE;
  	if (!found)
	{
      		if ((container = get_obj_here (ch, arg3)) != NULL)
        		found = TRUE;
      		if (!found)
        	{
          		send_to_char ("You do not see that here.\n\r", ch);
          		return;
        	}
    	}
  if (container->item_type != ITEM_CONTAINER
      && container->item_type != ITEM_PACK
      && container->item_type != ITEM_PLAYER_DONATION
      && container->item_type != ITEM_NEWCLANS_DBOX
      && container->item_type != ITEM_CLAN_DONATION
      && container->item_type != ITEM_QUIVER)
        {
	send_to_char("You can only trap containers.\n\r",ch);
	return;
  	}
  if ((container->item_type == ITEM_CLAN_DONATION) ||
      (container->item_type == ITEM_NEWCLANS_DBOX))
    {
      if (container->value[0] != -1)
        if (get_obj_number (container) >= container->value[0])
          {
            send_to_char ("Sorry, the clan donation box is full.\n\r", ch);
            return;
          }
    }
  if (container->item_type == ITEM_PLAYER_DONATION)
    {
      if (container->value[0] != -1)
        if (get_obj_number (container) >= container->value[0])
          {
            send_to_char ("Sorry, that box is full.\n\r", ch);
            return;
          }
    }
      if (container->item_type != ITEM_CLAN_DONATION
          && container->item_type != ITEM_NEWCLANS_DBOX
          && container->item_type != ITEM_PLAYER_DONATION
          && container->item_type != ITEM_QUIVER)
        {
          if (get_obj_weight (obj) +
              get_true_weight (container) > (container->value[0] *
                                             10) || get_obj_weight (obj)
              > (container->value[3] * 10))
            {
              send_to_char ("It won't fit.\n\r", ch);
              return;
            }
        }
       if (container->pIndexData->vnum == OBJ_VNUM_PENTAGRAM
           && container->plr_owner && str_cmp(container->plr_owner,ch->name))
       {
          send_to_char("Only the necromancer who drew that pentagram can put objects in it.\n\r", ch);
          return;
       }

  
}
for (list = ch->trap_list;list != NULL;list = list->next_trap)
	count++;

if (count >= MAX_NUM_TRAPS)
{
	send_to_char ("You have reached the limit of traps you may install at once.\n\r",ch);
	return;
}

if (obj->item_type == ITEM_OBJ_TRAP || obj->item_type == ITEM_ROOM_TRAP || obj->item_type == ITEM_PORTAL_TRAP ) 	
  {
	if (IS_SET(obj->extra_flags[1],ITEM_WIZI)) 
	{
		send_to_char("This trap is already set, why are you holding it?\n\r",ch);
	}else
	{
		if (number_percent () < (get_skill (ch, gsn_create_traps)*.75+ (get_curr_stat (ch, STAT_DEX)- (obj->value[5]/2))) )
		{
			
			act ("$n installs $p.", ch, obj, NULL, TO_ROOM);
			act ("You install $p.", ch, obj, NULL, TO_CHAR);
			found = TRUE;
			obj->value[5] = get_curr_stat (ch,STAT_DEX);
			if (number_percent() < 5)
				SET_BIT(obj->value[12],TFLAG_NODISARM);
			
			if (number_percent() < 30)
			{
				SET_BIT(obj->value[12],TFLAG_EXCELLENT);
				obj->value[5] = UMIN(obj->value[5]+3,25);
			}
			if (!strcmp(arg,"room") || !strcmp(arg,"portal"))
			{
				obj_from_char (obj);
				obj_to_room (obj, ch->in_room);
			}
			else if (!strcmp(arg,"object"))
			{
      				obj_from_char (obj);
				obj_to_obj (obj, container);
			}
			SET_BIT(obj->extra_flags[1],ITEM_WIZI);  // Trap is installed
			obj->set_by = ch;
			WAIT_STATE(ch,16);
			if (ch->trap_list==NULL)
			{
				ch->trap_list = obj;
				obj->next_trap = NULL;
			}else
			{
				obj->next_trap = ch->trap_list;
				ch->trap_list = obj;
			}
		}else
		{
			send_to_char("The trap backfires!\n\r",ch);
			act (get_vict_mess(obj), ch, obj, NULL, TO_CHAR);
	                act (get_spec_mess(obj), ch, obj, NULL, TO_ROOM);
			trap_damage(ch,obj);
		}
		
				
	}
} else send_to_char("That is not a trap.\n\r",ch);
    
  return;
}

bool detect_room_trap (CHAR_DATA *ch, ROOM_INDEX_DATA * room)
{
OBJ_DATA *obj;
if (get_skill (ch,gsn_detect_traps) < 1)
	return FALSE;
for (obj = room->contents; obj != NULL; obj = obj->next_content)
    {
      if ( (( obj->item_type == ITEM_ROOM_TRAP 
           || obj->item_type == ITEM_PORTAL_TRAP )) 
	&& IS_SET(obj->extra_flags[1],ITEM_WIZI)
	&& (number_percent () < (get_skill (ch, gsn_detect_traps)*.75+ (get_curr_stat (ch, STAT_INT))) )
	&& !IS_SET(obj->value[12],TFLAG_NODETECT))
		return TRUE;
        	
    }

   return FALSE;	
}

bool detect_obj_trap (CHAR_DATA *ch,OBJ_DATA *obj)
{
OBJ_DATA *thing;
	if (obj != NULL )
	{
		for (thing = obj->contains; thing != NULL; thing= thing->next_content)
		{
			if ( (thing->item_type == ITEM_OBJ_TRAP)  
			     && IS_SET(thing->extra_flags[1],ITEM_WIZI) 
			     && number_percent () < (get_skill (ch, gsn_detect_traps))*0.75 + ch->mod_stat[STAT_INT] )
				return TRUE;
		}
	}
	return FALSE;
}
