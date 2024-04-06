/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,         *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.    *
 *                                                                          *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael           *
 *  Chastain, Michael Quan, and Mitchell Tse.                               *
 *                                                                          *
 *  In order to use any part of this Merc Diku Mud, you must comply with    *
 *  both the original Diku license in 'license.doc' as well the Merc        *
 *  license in 'license.txt'.  In particular, you may not remove either of  *
 *  these copyright notices.                                                *
 *                                                                          *
 *  Much time and thought has gone into this software and you are           *
 *  benefitting.  We hope that you share your changes too.  What goes       *
 *  around, comes around.                                                   *
 ****************************************************************************/


/***************************************************************************
 *       ROM 2.4 is copyright 1993-1998 Russ Taylor                         *
 *       ROM has been brought to you by the ROM consortium                  *
 *           Russ Taylor (rtaylor@hypercube.org)                            *
 *           Gabrielle Taylor (gtaylor@hypercube.org)                       *
 *           Brian Moore (zump@rom.org)                                     *
 *       By using this code, you have agreed to follow the terms of the     *
 *       ROM license, in the file Rom24/doc/rom.license                     *
 ****************************************************************************/

#if defined(macintosh)
#include <types.h>
#else /*  */
#include <sys/types.h>
#endif /*  */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "clans/new_clans.h"

extern AREA_DATA *area_first;
bool room_Class_check args ((CHAR_DATA * ch, ROOM_INDEX_DATA * location));
bool room_race_check args((CHAR_DATA * ch, ROOM_INDEX_DATA * to_room));
//Quest.c
//Iblis  - Created 1/20/04 to deal with autoquests
//Paarshad - Modified 6/12/04 added hunt mob diversity and separate types of hunt mobs
void do_quest(CHAR_DATA *ch, char* argument)
{
  char arg1[MAX_INPUT_LENGTH];
  CHAR_DATA *mob;
  char buf[MAX_STRING_LENGTH];
  int race;
  if (IS_NPC(ch))
    return;
  if (argument[0] == '\0')
    {
      send_to_char("Syntax : Quest request find/hunt\n\r",ch);
      send_to_char("               clear\n\r",ch);
      send_to_char("               complete\n\r",ch);
      send_to_char("               info\n\r",ch);
      return;
    }
  sprintf(buf,"$N -> quest %s",argument);
  wiznet (buf, ch, NULL, WIZ_QUEST, 0, get_trust (ch));
  argument = one_argument(argument,arg1);
  for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room)
    {
      if (IS_NPC (mob) && IS_SET (mob->act2, ACT_QUESTMASTER))
	break;
    }
  if (mob == NULL && str_cmp(arg1,"info") && str_cmp(arg1,"clear"))
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
    }

  if (!str_cmp(arg1,"request"))
    {
      int number_of_areas=0,counter=0,k=0;
      AREA_DATA *pArea=NULL;
      if (ch->pcdata->quest_ticks < 0)
	{
   	  sprintf(buf,"%s You are not ready for another quest.",ch->name);
	  do_tell(mob,buf);
	  return;
	}
      
      if ((time (NULL) - ch->pcdata->last_fight < 120))
        {
          send_to_char ("Take a breather before you start a quest.\n\r", ch);
	  return;
        }
       
	      
      if (ch->pcdata->quest_ticks > 0)
	{
          sprintf(buf,"%s You're a bit over eager. You already have a quest you need to complete.",ch->name);
	  do_tell(mob,buf);
	  return;
	}

      for (k = 0;k<2;k++)
	{
	  for (pArea = area_first; pArea != NULL; pArea = pArea->next)
		// Updating this list for many reasons.  Some areas are too hard for one player, many areas are properly
		// flagged to have them otherwise excluded (Imm areas not properly flaged, Unfinished zones that are live
		// in game but not linked.  And others.  
		// Tower of Babel and Orchard should be included when they are done.  Possibly also Lightning Rod and 
		// Spiritual Plane.  
		// Duiwel 7/7/2012
	    {
	      if (!str_cmp (pArea->name, "Haven") || !str_cmp (pArea->name, "`lThe Ocean``") || !str_cmp (pArea->name, "`kPlaying Field``") || !str_cmp (pArea->name, "`dGuttersnipes' Fort``") || !str_cmp (pArea->name, "`gStillwater``") || !str_cmp (pArea->name, "Shattered Domain of Eyunel") || !str_cmp (pArea->name, "`bThe Hell of Auvyron``") || !str_cmp (pArea->name, "`k+\`n Arena of Legends `k/+``") || !str_cmp (pArea->name, "`hTemple of the Phalanx``") || !str_cmp (pArea->name, "The Bazaar") || !str_cmp (pArea->name, "Druids") || !str_cmp (pArea->name, "Lightning Rod") || !str_cmp (pArea->name, "Orchard") || !str_cmp (pArea->name, "`o|`k| `oThe Tower of Babel `k|`o|``")  || !str_cmp (pArea->name, "player") || !str_cmp (pArea->name, "`athe null set``") || !str_cmp (pArea->name, "The Minefield!!!") || !str_cmp (pArea->name, "Exodus Homelands") || !str_cmp (pArea->name, "Thesden") || !str_cmp (pArea->name, "`bWarped Fulk Nerra``"))
		   
		continue;
	      
		  // Duiwel 7/7/2012  I commented out this entire section because the affluence check didn't seem
		  // to be functioning properly, in any case I have exclude areas by hand and will continue with this method 
		  // as it is more reliable.
		  
	      // if the area is not public then don't show it - when an
	      // areas points (affluence) are greater than zero then it is public
		//   if ((pArea->points <= 0))
		//{
		//  if (!IS_IMMORTAL (ch))
		//    continue;
		//	}
	
	      if (pArea->ulev >= ch->level && pArea->llev <= ch->level)
		{
		  if (k == 0)
		    ++number_of_areas;
		  else if (++counter == number_of_areas)
		    {
		      //Use this area for the quest
		      break;
		    }
		}
	    }
	  number_of_areas = number_range(1,number_of_areas);
	}
      if (pArea == NULL)
      {
        send_to_char("Error, try again.\n\r",ch);
	return;
      }

      if (!str_cmp(argument,"hunt"))
        {
          ROOM_INDEX_DATA *room_using;
          CHAR_DATA *mob2;
	  char first1[MAX_STRING_LENGTH], first2[MAX_STRING_LENGTH], second1[MAX_STRING_LENGTH];
	  int bacounter=0,i=0;
	  int qtype = 1;
	  if (ch->level < 20)
	  {
		  send_to_char ("I'm sorry, there are no beasts weak enough for you to kill.\n\r",ch);
		  return;
	  }
	 
	  if (ch->level > 40 && ch->level <= 50)
		  qtype = number_range(1,2);
	
	  if (ch->level >= 50)
		  qtype = 2;

	  if (ch->level >= 70)
	  {
		  if (number_percent() > 5+(ch->level-70)) 
		  	qtype = 2;
		  else
			qtype = 3;
	  }
	  
          while ((room_using = get_room_index(number_range(pArea->lvnum, pArea->uvnum))) == NULL 
		 || !(str_cmp(room_using->name,"")) || IS_SET(room_using->room_flags,ROOM_TRANSPORT)
		 || (room_using->Class_flags != 0 && !room_Class_check (ch, room_using))
		 || (room_using->race_flags != 0 && !room_race_check(ch,room_using))
		 || (room_using->max_level != 0)
		 || (IS_SET (room_using->room_flags, ROOM_IMP_ONLY) && !IS_IMMORTAL(ch)))
	  {
  	    if (++bacounter > 1000)
	    {
	    send_to_char("There are no quests at the moment.  Try back again very soon though!\n\r",ch);
	    return;
	    }
	  }
	  mob2 = create_mobile(get_mob_index(MOB_VNUM_BEAST));
	  mob2->level = ch->level;
	  mob2->max_hit = ch->level*55;
	  mob2->hit = mob2->max_hit;
	  mob2->max_mana = ch->level*5;
	  mob2->mana = mob2->max_mana;
	  mob2->max_move = ch->level*3;
	  mob2->move = mob2->max_move;
	  mob2->imm_flags = IMM_CHARM | IMM_SUMMON;
	  mob2->res_flags = 0;
	  mob2->vuln_flags = 0;
	  mob2->act = ACT_IS_NPC | ACT_SENTINEL | ACT_STAY_AREA;
	  mob2->act2 = ACT_NOMOVE | ACT_QUESTMOB;
	  mob2->damroll = 4 + ((ch->level-20) * (50/70.0));
	  mob2->hitroll = 4 + ((ch->level-20) * (50/70.0));
          for (i = 0; i < 4; i++)
            mob2->armor[i] = 0-50-(450*(ch->level-20)/70);
	  mob2->damage[DICE_NUMBER] = 4 + ((ch->level-20) * (1/14.0));
	  mob2->damage[DICE_TYPE] = 4 + ((ch->level-20) * (1/14.0));
	  ch->pcdata->qtype = qtype;  
          switch (number_range(1,18))
	  {
		case 1: sprintf(first1,"a magical"); break;
		case 2: sprintf(first1,"a");break;
		case 3: sprintf(first1,"a horrendous");break;
		case 4: sprintf(first1,"a horrible");break;
		case 5: sprintf(first1,"a monstrous");break;
		case 6: sprintf(first1,"a hideous");break;
		case 7: sprintf(first1,"a terrifying");break;
		case 8: sprintf(first1,"a ridiculous");break;
		case 9: sprintf(first1,"a pathetic");break;
		case 10: sprintf(first1,"an aroused");break;
		case 11: sprintf(first1,"a distracted");break;
		case 12: sprintf(first1,"a batshit insane");break;
		case 13: sprintf(first1,"an effeminate");break;
		case 14: sprintf(first1,"a mentally-challenged");break;
		case 15: sprintf(first1,"a whiny");break;
		case 16: sprintf(first1,"a complaining");break;
		case 17: sprintf(first1,"an egotistical");break;
		default: sprintf(first1,"an evil");break;
	  }
	  switch (number_range(1,12))
	  {
	        case 1:
			sprintf(first2,"mountain");
			mob2->off_flags = mob2->off_flags | OFF_BERSERK;
			break;
	        case 2: sprintf(first2,"prairie");break;
	        case 3: 
			sprintf(first2,"cloud");
			mob2->affected_by = mob2->affected_by | AFF_FLYING;
			break;
			
	        case 4: 
			sprintf(first2,"black");
			mob2->act = mob2->act | ACT_UNDEAD;
			break;
	        case 5: 
			sprintf(first2,"fire");
			mob2->vuln_flags |= VULN_COLD;
			mob2->res_flags |= RES_FIRE;
			break;
	        case 6: 
			sprintf(first2,"ice");
			mob2->vuln_flags |= VULN_FIRE;
			mob2->res_flags |= RES_COLD;
			break;
		case 7:
			sprintf(first2,"swamp");
			break;
		case 8: 
			sprintf(first2,"aquatic");
			break;
	        case 9: 
			sprintf(first2,"white");
			mob2->affected_by = mob2->affected_by | AFF_SANCTUARY;
			break;
		case 10: sprintf(first2,"one-legged");break;
		case 11: sprintf(first2,"one-eyed");break;
	        default: 
			sprintf(first2,"shadow");
			mob2->affected_by = mob2->affected_by | AFF_SNEAK;
			break;
	  }
	  
	  if (qtype==1)
	  {
		switch (number_range(1,9))
	  	{
		        case 1: 
				sprintf(second1,"goblin");
				mob2->dam_type = attack_lookup("punch");
				mob2->spec_fun = spec_lookup ("spec_nasty");
				mob2->off_flags = mob2->off_flags | OFF_KICK_DIRT; 

				if ((race = race_lookup ("goblin")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
			break;
			case 2: 
				sprintf(second1,"bat");
				mob2->dam_type = attack_lookup("vbite"); 
				if ((race = race_lookup ("bat")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
		        case 3: 
				sprintf(second1,"bear");
				mob2->dam_type = attack_lookup("claw"); 
				if ((race = race_lookup ("bear")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
		        case 4: 
				sprintf(second1,"lion");
				mob2->dam_type = attack_lookup("claw"); 
				if ((race = race_lookup ("cat")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
		        case 5: 
				sprintf(second1,"tiger");
				mob2->dam_type = attack_lookup("claw"); 
				mob2->off_flags = mob2->off_flags | OFF_KICK;
				if ((race = race_lookup ("cat")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
		        case 6: 
				sprintf(second1,"wolf");
				mob2->dam_type = attack_lookup("bite"); 
				mob2->off_flags = mob2->off_flags | OFF_FAST;
				if ((race = race_lookup ("wolf")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
		        case 7: 
				sprintf(second1,"gremlin");
				mob2->dam_type = attack_lookup("chomp"); 
				mob2->spec_fun = spec_lookup ("spec_nasty");
				mob2->off_flags = mob2->off_flags | OFF_DODGE | OFF_TRIP;
				if ((race = race_lookup ("kobold")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
		        case 8: 
				sprintf(second1,"ogre");
				mob2->dam_type = attack_lookup("beating"); 
				if ((race = race_lookup ("troll")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
		        default: 
				sprintf(second1,"gargoyle");
				mob2->dam_type = attack_lookup("chill"); 
				mob2->affected_by = mob2->affected_by | AFF_FLYING;
				if ((race = race_lookup ("bat")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
		}        
	  }			
			
	  if (qtype==2)
	  {
		  switch (number_range(1,10))
		  {
		        case 1: 
				sprintf(second1,"werewolf");
				mob2->dam_type = attack_lookup("acbite");
				mob2->off_flags = mob2->off_flags | OFF_FAST | OFF_DODGE; 
		        	mob2->act = mob2->act | ACT_REMEMBER | ACT_SMART_HUNT;
				REMOVE_BIT(mob2->act,ACT_NOMOVE);
				mob2->off_flags = mob2->off_flags | OFF_BERSERK;
				if ((race = race_lookup ("wolf")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
			case 2: 

				sprintf(second1,"chimera");
				mob2->dam_type = attack_lookup("claw");
				mob2->spec_fun = spec_lookup ("spec_breath_lightning");
				if ((race = race_lookup ("dragon")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
		 		break;
			case 3: 
				sprintf(second1,"behemoth");
				mob2->dam_type = attack_lookup("crush");
				mob2->off_flags = mob2->off_flags | OFF_CHARGE; 
				mob2->damage[DICE_NUMBER] = mob2->damage[DICE_NUMBER]*2;
				mob2->max_hit = mob2->max_hit*1.25 ;
				mob2->hit = mob2->max_hit;
				if ((race = race_lookup ("lizard")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
			case 4: 
				sprintf(second1,"cyclops");
				mob2->dam_type = attack_lookup("pound");
				mob2->off_flags = mob2->off_flags | OFF_DODGE;
				mob2->act2 = mob2->act2 | ACT_MONK;
				if ((race = race_lookup ("giant")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
		        case 5: 
				sprintf(second1,"centaur");
				mob2->dam_type = attack_lookup("slash");
				mob2->off_flags = mob2->off_flags | OFF_KICK | OFF_CHARGE; 
		        	mob2->act = mob2->act | ACT_WARRIOR;
				if ((race = race_lookup ("elf")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
			case 6: 
				sprintf(second1,"demon");
				mob2->dam_type = attack_lookup("flame");
				mob2->spec_fun = spec_lookup ("spec_breath_fire");
				mob2->off_flags = mob2->off_flags | OFF_DODGE;
				mob2->imm_flags = mob2->imm_flags | IMM_FIRE;
				mob2->act = mob2->act | ACT_MAGE;
				if ((race = race_lookup ("kalian")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
		        case 7: 
				sprintf(second1,"titan");
				mob2->dam_type = attack_lookup("frbite");
				mob2->max_hit = mob2->max_hit*1.5 ;
				mob2->hit = mob2->max_hit;
				mob2->damage[DICE_NUMBER] = mob2->damage[DICE_NUMBER]*2;
				mob2->off_flags = mob2->off_flags | OFF_KICK;
				if ((race = race_lookup ("cloudgiant")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
		        case 8:
				sprintf(second1,"leviathan");
				mob2->dam_type = attack_lookup("wrath");
				mob2->off_flags = mob2->off_flags | OFF_CHARGE;
				mob2->max_hit = mob2->max_hit*1.51 ;
                                mob2->hit = mob2->max_hit;
				mob2->damage[DICE_NUMBER] = mob2->damage[DICE_NUMBER]*2;
				if ((race = race_lookup ("dragon")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;				
		        case 9: 
				sprintf(second1,"monstrocity");
				mob2->dam_type = attack_lookup("claw");
				mob2->max_hit = mob2->max_hit*2 ;
				mob2->hit = mob2->max_hit;
				mob2->damage[DICE_NUMBER] = mob2->damage[DICE_NUMBER]*2 + 5;
				mob2->off_flags = mob2->off_flags | OFF_CHARGE;
				mob2->affected_by = mob2->affected_by | AFF_SLOW; 
				if ((race = race_lookup ("lizard")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
		        	break;
			default: 
				sprintf(second1,"banshee");
				mob2->dam_type = attack_lookup("drain");
				mob2->spec_fun = spec_lookup ("spec_bard");
				mob2->max_hit = (int)(mob2->max_hit*.75 );
				mob2->hit = mob2->max_hit;
				mob2->imm_flags = mob2->imm_flags | IMM_WEAPON;
		        	mob2->affected_by = mob2->affected_by | AFF_FLYING | AFF_PASS_DOOR;
				if ((race = race_lookup ("lich")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
		}
	  }
	  
	  if (qtype==3)
	  {
		  switch (number_range(1,5))
		  {
			case 1: 
				sprintf(second1,"warlock");
				mob2->dam_type = attack_lookup("magic");
				mob2->act = mob2->act | ACT_REMEMBER | ACT_MAGE;
				mob2->act2 = mob2->act2 | ACT_MAGE_SKILLS;
				mob2->vuln_flags = mob2->vuln_flags | VULN_BASH;
		        	mob2->affected_by = mob2->affected_by | AFF_DETECT_HIDDEN | AFF_DETECT_INVIS;
				mob2->off_flags = mob2->off_flags | OFF_DODGE;
				if ((race = race_lookup ("elf")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
			case 2: 
				sprintf(second1,"lich");
				mob2->dam_type = attack_lookup("vbite");
		        	mob2->act = mob2->act | ACT_REMEMBER | ACT_UNDEAD;
				mob2->act2 = mob2->act2 | ACT_NECROMANCER;
				mob2->vuln_flags = mob2->vuln_flags | VULN_HOLY;
				mob2->affected_by = mob2->affected_by | AFF_REGENERATION;
				if ((race = race_lookup ("lich")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
			case 3: 
				sprintf(second1,"murderer");
				mob2->dam_type = attack_lookup("cleave");
				mob2->off_flags = mob2->off_flags |OFF_BERSERK| OFF_FAST | OFF_BASH | OFF_DODGE | OFF_DISARM | OFF_KICK_DIRT | OFF_PARRY | OFF_TRIP; 
		        	mob2->affected_by = mob2->affected_by | AFF_SNEAK;
				mob2->max_hit = (int)(mob2->max_hit*1.25) ;
				mob2->hit = mob2->max_hit;
				mob2->damage[DICE_NUMBER] =(int)( mob2->damage[DICE_NUMBER]*1.5) ;
				mob2->act = mob2->act | ACT_REMEMBER | ACT_HUNT | ACT_WARRIOR;		
				REMOVE_BIT(mob2->act,ACT_NOMOVE);
				for (i = 0; i < 4; i++)
					mob2->armor[i] = mob2->armor[i]*2;
				if ((race = race_lookup ("syvin")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
			case 4: 
				sprintf(second1,"dark priest");
				mob2->dam_type = attack_lookup("divine");
				mob2->act = mob2->act | ACT_REMEMBER | ACT_CLERIC;
				mob2->spec_fun = spec_lookup ("spec_cast_healer");
		        	mob2->affected_by = mob2->affected_by | AFF_SANCTUARY;
				if ((race = race_lookup ("human")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
			default: 
				sprintf(second1,"dragon");
				mob2->dam_type = attack_lookup("frbite");
				mob2->spec_fun = spec_lookup ("spec_breath_any");
				mob2->max_hit = mob2->max_hit*2 ;
				mob2->hit = mob2->max_hit;
				mob2->off_flags = mob2->off_flags | OFF_FAST | OFF_BASH | OFF_DODGE | OFF_DISARM | OFF_KICK_DIRT | OFF_PARRY | OFF_TRIP;
		        	mob2->affected_by = mob2->affected_by | AFF_FLYING;
				if ((race = race_lookup ("dragon")) != 0)
				{
      					mob2->race = race;
				      	mob2->act |= race_table[race].act;
      					mob2->off_flags |= race_table[race].off;
      					mob2->imm_flags |= race_table[race].imm;
      					mob2->res_flags |= race_table[race].res;
      					mob2->vuln_flags |= race_table[race].vuln;
      					mob2->form |= race_table[race].form;
      					mob2->parts |= race_table[race].parts;
    				}
				break;
	      	  }
	  }
			      
	  
	  free_string(mob2->name);
	  sprintf(buf,"%s %s",first2,second1);
	  mob2->name = str_dup(buf);
	  free_string(mob2->short_descr);
	  sprintf(buf,"%s %s %s",first1,first2,second1);
	  mob2->short_descr = str_dup(buf);
	  free_string(mob2->long_descr);
	  sprintf(buf,"%s %s %s wanders around here, enjoying the freedom.\n\r",first1,first2,second1);
	  mob2->long_descr = str_dup(capitalize(buf));
	  char_to_room(mob2,room_using);
	  sprintf(buf,"%s, %s `Khas has escaped from captivity. It is very dangerous.",ch->name, mob2->short_descr);
          do_say(mob,buf);
          sprintf(buf,"You must find it and kill it. Look in the vicinity of %s `Kfor %s.",pArea->name,room_using->name);
          do_say(mob,buf);
	  sprintf(buf,"You have %d hours to kill it and then notify me.",40+ (29*ch->pcdata->qtype));
          do_say(mob,buf);
          do_say(mob,"You have been denied magical transportation for the duration of this quest.");
          ch->pcdata->quest_ticks = 40+ (29*ch->pcdata->qtype);
          ch->pcdata->quest_mob = mob2;
          //ch->pcdata->nogate = TRUE;
          ++ch->pcdata->total_qh_attempted;
	  return;
	}


	  
      if (!str_cmp(argument,"find"))
	{
	  char desc1[MAX_STRING_LENGTH],stone1[MAX_STRING_LENGTH],type1[MAX_STRING_LENGTH];
	  ROOM_INDEX_DATA *room_using;
	  OBJ_DATA *obj;
	  int bacounter=0;
	  while ((room_using = get_room_index(number_range(pArea->lvnum, pArea->uvnum))) == NULL 
	      || !(str_cmp(room_using->name,"")) 
	      || IS_SET(room_using->room_flags,ROOM_TRANSPORT)
 	      || (room_using->Class_flags != 0 && !room_Class_check (ch, room_using))
	      || (room_using->race_flags != 0 && !room_race_check(ch,room_using))
	      || (room_using->max_level != 0 && ch->level > room_using->max_level)
	      || (IS_SET (room_using->room_flags, ROOM_IMP_ONLY) && !IS_IMMORTAL(ch)))
	  {
	  if (++bacounter > 1000)
	   {
	   send_to_char("There are no quests at the moment.  Try back again very soon though!\n\r",ch);
	   return;
	   }
	  }
	  obj = create_object(get_obj_index(OBJ_VNUM_LIGHT_BALL),0);
	  free_string(obj->plr_owner);
	  obj->plr_owner = str_dup(ch->name);
	  obj_to_room(obj,room_using);
	  switch (number_range(1,4))
	    {
	    case 1: sprintf(desc1,"the Queen's");break;
	    case 2: sprintf(desc1,"the Princess's");break;
	    case 3: sprintf(desc1,"a");break;
	    default: sprintf(desc1,"a famous");break;
	    }
	  switch (number_range(1,20))
	    {
	    case 1: sprintf(stone1,"diamond");break;
	    case 2: sprintf(stone1,"emerald");break;
	    case 3: sprintf(stone1,"tanzanite");break;
	    case 4: sprintf(stone1,"ruby");break;
	    case 5: sprintf(stone1,"opal");break;
	    case 6: sprintf(stone1,"turquoise");break;
	    case 7: sprintf(stone1,"sunstone");break;
	    case 8: sprintf(stone1,"bloodstone");break;
	    case 9: sprintf(stone1,"moonstone");break;
	    case 10: sprintf(stone1,"sapphire");break;
	    case 11: sprintf(stone1,"lapis");break;
	    case 12: sprintf(stone1,"agate");break;
	    case 13: sprintf(stone1,"aquamarine");break;
	    case 14: sprintf(stone1,"amethyst");break;
	    case 15: sprintf(stone1,"alexandrite");break;
	    case 16: sprintf(stone1,"morganite");break;
	    case 17: sprintf(stone1,"iblisanite");break;
	    case 18: sprintf(stone1,"iverathanite");break;
	    case 19: sprintf(stone1,"minaxanite");break;
	    default: sprintf(stone1,"ametrine");break;
	    }
	  switch (number_range(1,5))
	    {
	    case 1: sprintf(type1,"ring");break;
	    case 2: sprintf(type1,"earring");break;
	    case 3: sprintf(type1,"necklace");break;
	    case 4: sprintf(type1,"pendant");break;
	    case 5: sprintf(type1,"bracelet");break;
	    }
	  free_string(obj->short_descr);
	  sprintf(buf,"%s %s %s",desc1,stone1,type1);
	  obj->short_descr = str_dup(buf);
	  free_string(obj->name);
	  sprintf(buf,"%s %s",stone1,type1);
	  obj->name = str_dup(buf);
	  free_string(obj->description);
	  sprintf(buf,"A %s lies here waiting to be retrieved.",type1);
	  obj->description = str_dup(buf);
	   
	      obj->extra_flags[0] |= ITEM_NOLOCATE | ITEM_NOPURGE | ITEM_NOSAVE | ITEM_NOIDENTIFY;
	      obj->item_type = ITEM_TRASH;
	  sprintf(buf,"%s, %s `Khas been stolen from King Athanmere's treasury.",ch->name, obj->short_descr);
	  do_say(mob,buf);
	  sprintf(buf,"You must find it. Look in the vicinity of %s `Kfor %s.",pArea->name,room_using->name);
	  do_say(mob,buf);
	  do_say(mob,"You have 69 hours to find it and bring it to me.");
	  do_say(mob,"You have been denied magical transportation for the duration of this quest.");
	  ch->pcdata->quest_ticks = 69;
	  ch->pcdata->quest_obj = obj;
          //ch->pcdata->nogate = TRUE;
	  ++ch->pcdata->total_qf_attempted;
	  return;

	}
      do_quest(ch,"");
      return;


    }
  if (!str_cmp(arg1,"clear"))
    {
      if (ch->pcdata->quest_ticks <= 0)
	{
		send_to_char("You are not currently on a quest.\n\r",ch);
	  return;
	}
      if (ch->pcdata->quest_obj != NULL)
	{
          sprintf(buf,"I'm sorry you were unable to complete the quest to return %s.\n\r",ch->pcdata->quest_obj->short_descr);
	  send_to_char(buf,ch);
	}
      else if (ch->pcdata->quest_mob != NULL)
	{
	  if (ch->pcdata->quest_mob == ch)
	    sprintf(buf, "I'm sorry you were unable to report your finishing of the quest.\n\r");
	  else sprintf(buf, "I'm sorry you were unable to complete the quest to slay %s.\n\r",ch->pcdata->quest_mob->short_descr);
	  send_to_char(buf,ch);
	}
      else 
      {
	      send_to_char("I'm sorry you were unable to complete the quest.\n\r",ch);
      }
      send_to_char("Better luck next time!\n\r",ch);
      if (ch->pcdata->quest_obj)
        extract_obj(ch->pcdata->quest_obj);
      else if (ch->pcdata->quest_mob && ch->pcdata->quest_mob != ch)
	extract_char(ch->pcdata->quest_mob,TRUE);


	
      ch->pcdata->quest_mob = NULL;
      ch->pcdata->quest_obj = NULL;
	  	// Duiwel 7/7/2012 - Quest recet time reduced from 10 ticks to 6 ticks
      ch->pcdata->quest_ticks = -6;
      if (IS_IMMORTAL(ch))
        ch->pcdata->quest_ticks = 0;
//      ch->pcdata->nogate = FALSE;
      ch->pcdata->qtype = 0;
      return;

    }
  if (!str_cmp(arg1,"info"))
  {
    if (ch->pcdata->quest_ticks == 0)
    {
      send_to_char("You cannot get info on a quest you do not have!\n\r",ch);
      return;
    }
    if (ch->pcdata->quest_ticks < 0)
    {
	    sprintf(buf,"You have to wait %d more hours to begin another quest.\n\r",0-ch->pcdata->quest_ticks);
	    send_to_char(buf,ch);
	    return;
    }
    if (ch->pcdata->quest_obj == NULL)
    {
	    if (ch->pcdata->quest_mob == NULL)
	    {
		    bug("quest_ticks > 0, but questmob and obj null",0);
		    send_to_char("Try again later.\n\r",ch);
		    return;
	    }
            if (ch->pcdata->quest_mob == ch)
	      {
		sprintf(buf, "You have completed the quest, now just go tell the Questmaster within %d hours!\n\r",
				ch->pcdata->quest_ticks);
		send_to_char(buf,ch);
		return;
	      }
	    if (ch->pcdata->quest_mob->in_room)
	      {
		if (ch->in_room && !str_cmp(ch->in_room->area->name,ch->pcdata->quest_mob->in_room->area->name))
		  sprintf(buf,"``You remember you are looking for %s ``in %s``.  You have %d hours left.\n\r"
			  ,ch->pcdata->quest_mob->short_descr, ch->pcdata->quest_mob->in_room->name
			  ,ch->pcdata->quest_ticks);
		else sprintf(buf,"``You remember you are looking for %s ``in %s``.  You have %d hours left.\n\r"
			     ,ch->pcdata->quest_mob->short_descr, ch->pcdata->quest_mob->in_room->area->name,
			     ch->pcdata->quest_ticks);
	      }
	    else sprintf(buf,"Try again later.\n\r");
	    send_to_char(buf,ch);
	    return;
    }
    if (ch->pcdata->quest_mob == NULL)
    {
      if (ch->pcdata->quest_obj == NULL)
        {
           bug("quest_ticks > 0, but questmob and obj null",0);
           send_to_char("Try again later.\n\r",ch);
           return;
        }
      else
      {
	if (ch->pcdata->quest_obj->in_room == NULL)
	{
 	  if (ch->pcdata->quest_obj->carried_by && ch->pcdata->quest_obj->carried_by->in_room)
	  {
	    if (ch->pcdata->quest_obj->carried_by == ch)
	      sprintf(buf,"You have gathered the required obj, now just go tell the Questmaster within %d hours!\n\r"
			      ,ch->pcdata->quest_ticks);
	    else sprintf(buf,"``You remember you are looking for %s ``in %s``, though someone might have picked it up.  You have %d hours remaining.\n\r",
			    ch->pcdata->quest_obj->short_descr, ch->pcdata->quest_obj->carried_by->in_room->name,
			    ch->pcdata->quest_ticks);
	  }
		 
	  else sprintf(buf,"Alas, someone hath stolen your object!\n\r");
	}
	else if (ch->in_room && !str_cmp(ch->in_room->area->name,ch->pcdata->quest_obj->in_room->area->name))
	{
          sprintf(buf,"``You remember you are looking for %s ``in %s``.  You have %d hours left.\n\r",
			  ch->pcdata->quest_obj->short_descr, ch->pcdata->quest_obj->in_room->name
			  ,ch->pcdata->quest_ticks);
	}
	else sprintf(buf,"``You remember you are looking for %s ``in %s``.  You have %d hours left.\n\r",
			ch->pcdata->quest_obj->short_descr,
			ch->pcdata->quest_obj->in_room->area->name,ch->pcdata->quest_ticks);
	send_to_char(buf,ch);
	return;
      }

    }
  }
	    
  if (!str_cmp(arg1,"complete"))
    {
      if (ch->pcdata->quest_obj != NULL)
	{
	  if (ch->pcdata->quest_obj->carried_by != ch)
	    {
	      sprintf(buf,"%s You do not appear to have retrieved %s.",ch->name,ch->pcdata->quest_obj->short_descr);
	      do_tell(mob,buf);
	      return;
	    }
	  else
	    {
	      sprintf(buf,"Thank you for returning %s`K, %s!",ch->pcdata->quest_obj->short_descr,ch->name);
	      do_say(mob,buf);
	      sprintf(buf,"%s carefully takes %s from your outstretched hand.\n\r",
			      mob->short_descr,ch->pcdata->quest_obj->short_descr);
              send_to_char(buf,ch);
	      sprintf(buf,"%s carefully takes %s from %s's outstretched hand.",
			      mob->short_descr,ch->pcdata->quest_obj->short_descr,ch->name);
	      extract_obj(ch->pcdata->quest_obj);
	      act(buf,ch,NULL,NULL,TO_ROOM);
	      mob->gold = dice(3,ch->level);
	      
				// Duiwel 7/7/2012 - Gold increased by 3x
	      sprintf(buf,"%ld gold %s",mob->gold,ch->name);
	      do_give(mob,buf);
	      ++ch->pcdata->total_quest_finds;
	      ch->pcdata->quest_mob = NULL;
	      ch->pcdata->quest_obj = NULL;
		  	// Duiwel 7/7/2012 - Quest recet time reduced from 10 ticks to 4 ticks
	      ch->pcdata->quest_ticks = -4;
	      if (IS_IMMORTAL(ch))
	        ch->pcdata->quest_ticks = 0;
//	      ch->pcdata->nogate = FALSE;
				// Duiwel 7/7/2012 - Reward chance increased from 5% base to 8%
	      if (number_percent() <= 8)
	      {
		ch->pcdata->qpoints++;
		send_to_char("You receive 1 Quest Point as an additional reward for completing this quest.\n\r",ch);
	      }
		
	      return;
	    }
	}
      if (ch->pcdata->quest_mob != NULL)
        {
          if (ch->pcdata->quest_mob != ch)
            {
              sprintf(buf,"%s You do not appear to have killed %s.",ch->name,ch->pcdata->quest_mob->short_descr);
              do_tell(mob,buf);
              return;
            }
	  else
	    {
	
     		sprintf(buf,"Thank you for completing your hunt quest, %s!",ch->name);
        	do_say(mob,buf);
				// Duiwel 7/7/2012 - Gold increased by 3x
             	mob->gold = dice(3,ch->level)*ch->pcdata->qtype;
              	sprintf(buf,"%ld gold %s",mob->gold,ch->name);
              	do_give(mob,buf);
              	++ch->pcdata->total_quest_hunts;
				// Duiwel 7/7/2012 - Reward chance increased from 5/10/15% base to 8/16/24%
              	if (number_percent() <= 8*ch->pcdata->qtype)
		{
			ch->pcdata->qpoints++;
			send_to_char("You receive 1 Quest Point as an additional reward for completing this quest.\n\r",ch);
		}
             	
	        ch->pcdata->quest_mob = NULL;
                ch->pcdata->quest_obj = NULL;
					// Duiwel 7/7/2012 - Quest recet time reduced from 10 ticks to 4 ticks
                ch->pcdata->quest_ticks = -4;
                if (IS_IMMORTAL(ch))
                ch->pcdata->quest_ticks = 0;
              //ch->pcdata->nogate = FALSE;
	        ch->pcdata->qtype = 0;
                return;
	    }
	}

    }
  do_quest(ch,"");
}


