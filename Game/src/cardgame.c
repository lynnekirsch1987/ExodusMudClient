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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "merc.h"
#include "tables.h"
#include "interp.h"
#include "olc.h"
//#include "recycle.h"
//#include "clans/new_clans.h"

//CardGame.c
//Iblis 6/11/04

void binder_function args((CHAR_DATA* ch, char* argument, int vnum));
OBJ_DATA *get_obj_vnum_char args((CHAR_DATA* ch, int vnum));
void appraise args((CHAR_DATA *ch, OBJ_DATA* card));
void appraise_new args((CHAR_DATA *ch, OBJ_DATA* card, int mob_number, CHAR_DATA *mob_owner));
bool valid_deck args((CHAR_DATA *ch));
OBJ_DATA *draw_card args((CHAR_DATA *ch));
void cardgame_hand_display args((CHAR_DATA *ch));
void terrainify_mobs args((CHAR_DATA *ch, OBJ_DATA *old_terrain, OBJ_DATA *new_terrain, OBJ_DATA *mm, OBJ_DATA *sm));
inline int get_victimnumber args((int cg_ticks));
inline int get_mobnumber args((int cg_ticks));
bool check_counter args((CHAR_DATA *ch, OBJ_DATA *card));
bool cardgame_surrender args((CHAR_DATA *ch,bool premature));
int count_contents args((OBJ_DATA *container));
void do_ante args((CHAR_DATA* ch, char* argument));

void show_list_to_char args ((OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing));
bool pay_cost args((CHAR_DATA *ch, int amount));
bool their_turn args((CHAR_DATA *ch));
void table_scan args((CHAR_DATA *ch));
void appraise_list args((CHAR_DATA *ch));
bool appraise_card_number args((CHAR_DATA *ch, char* argument));
void appraise_all args((CHAR_DATA *ch, OBJ_DATA *card));
	
RUNE_CARD_DATA *new_rune_card args((void));
void free_rune_card args((RUNE_CARD_DATA * b));
void save_rune_cards args((void));
int get_obj_number2 args((OBJ_DATA * obj));

void mobs_scan args((CHAR_DATA *ch));

extern int amount_of_cards;

#define MAX_SAME_CARDS_VNUM 3
	
	

	
	
	
	
	

//Iblis - 6/11/04 - The binder command, used to manipulate cards
void do_binder (CHAR_DATA* ch, char* argument)
{
  binder_function(ch,argument,OBJ_VNUM_BINDER);
}

//Iblis - 6/11/04 - The deck command, used to manipulate cards
void do_deck (CHAR_DATA* ch, char* argument)
{
  binder_function(ch,argument,OBJ_VNUM_DECK);
}

// Iblis-6/11/04- A helper function for the binder/deck commands.  Since they
// use the exact same commands, I allow them to use the same function, just
// passing in whichever vnum is applicable (the vnum for the deck or binder, 
// or some other vnum if further expansion is needed)

void binder_function (CHAR_DATA* ch, char* argument, int vnum)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *binder, *obj, *obj_next;
  int pos=0,counter=0;
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (ch->in_room == NULL)
    return;
  if (arg1[0] == '\0')
  {
    send_to_char("Deck/Binder what?\n\r",ch);
    //List out binder commands here in the future
    return;
  }
  binder = get_obj_vnum_char(ch, vnum);
  if (IS_SET(binder->extra_flags[1],ITEM_WIZI) && !IS_IMMORTAL(ch))
  {
    send_to_char("You can't do that right now.\n\r",ch);
    return;
  }
  if (binder == NULL)
  {
    send_to_char("You appear to have lost your binder/deck.\n\r",ch);
    return;
  }
  if (!str_cmp(arg1,"empty"))
  {
    bool some_dropped = FALSE;
    for (obj = binder->contains; obj != NULL; obj = obj_next)
      {
	obj_next = obj->next_content;
	obj_from_obj(obj);
	//Set it's slot number in the binder to 0 (since it's no longer in the binder)
	obj->bs_capacity[0] = 0;
	if ((ch->carry_number + get_obj_number2 (obj) > can_carry_n (ch)) 
			      || (get_carry_weight (ch) + get_obj_weight (obj) > can_carry_w (ch)))
	  {
	    some_dropped = TRUE;
	    obj_to_room(obj,ch->in_room);
	  }
	else obj_to_char(obj,ch);
      }
    if (!some_dropped)
    {
      if (vnum == OBJ_VNUM_BINDER)
        send_to_char("You empty your binder into your inventory.\n\r",ch);
      else send_to_char("You empty your deck into your inventory.\n\r",ch);
    }
    else 
    {
      if (vnum == OBJ_VNUM_BINDER)
        send_to_char("You empty your binder into your inventory, but you can't hold all the cards so some fall to the ground.\n\r",ch);
      else send_to_char("You empty your deck into your inventory, but you can't hold all the cards so some fall to the ground.\n\r",ch);
    }
  }
  //binder fill
  else if (!str_cmp(arg1, "fill"))
  {
    short type=0;
    OBJ_DATA *tobj;
    if (arg2[0] == '\0')
      type = 0;
    else if (!str_prefix(arg2,"mob"))
	    type = CARD_MOB;
    else if (!str_prefix(arg2,"terrain"))
	    type = CARD_TERRAIN;
    else if (!str_prefix(arg2,"spell"))
	    type = CARD_SPELL_HEAL;
	    
    for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      if (obj->item_type != ITEM_CARD)
        continue;
      if (type > 0)
      {
        if (type <= CARD_TERRAIN)
        {
 	  if (obj->value[1] != type)
  	    continue;
        }
        else if (obj->value[1] < type)
           continue;
      }
      //The binder is full
      if (binder->value[0] <= get_obj_number2 (binder))
      {
	if (vnum == OBJ_VNUM_BINDER)
  	  send_to_char("You fill the binder as much as possible, but you can't fit them all.\n\r",ch);
	else send_to_char("You fill the deck as much as possible, but you can't fit them all.\n\r",ch);
        break;
      }
      if (obj->pIndexData->vnum != OBJ_VNUM_COMMON_CARD && binder->pIndexData->vnum == OBJ_VNUM_DECK)
	{
	  counter = 0;
	  for (tobj = binder->contains;tobj != NULL;tobj = tobj->next_content)
	    {
	      if (tobj->pIndexData->vnum == obj->pIndexData->vnum)
		{
		  if(++counter >= MAX_SAME_CARDS_VNUM)
		    {
		      //  send_to_char("You already have too many cards with that vnum in your deck.\n\r",ch);
		      break;
		    }
		}

	    }
	}
      if (counter < MAX_SAME_CARDS_VNUM)
      {
        obj_from_char (obj);
        obj_to_obj_pos (obj, binder,-1);
      }

    }
    if (vnum == OBJ_VNUM_BINDER)
      send_to_char("You fill the binder with the cards in your inventory.\n\r",ch);
    else send_to_char("You fill the deck with the cards in your inventory.\n\r",ch);
   
  }
  else if (!str_cmp(arg1, "put"))
  {
    OBJ_DATA *card;
    card = get_obj_carry(ch,arg2);
    if (card == NULL)
    {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
    if (card->item_type != ITEM_CARD)
    {
      send_to_char("You may only put cards in there.\n\r",ch);
      return;
    }
    pos = atoi(argument);
    if (pos == 0)
      pos = -1;
    //The position they gave you is out of range for the binder
    if (pos < -1 || pos > binder->value[0])
    {
      send_to_char("That's not a valid position.\n\r",ch);
      return;
    }
    //The binder is full
    if (binder->value[0] <= get_obj_number2 (binder))
    {
      send_to_char("It won't seem to fit\n\r",ch);
      return;
    }
    if (card->pIndexData->vnum != OBJ_VNUM_COMMON_CARD)
    {
      int counter = 0;
      for (obj = binder->contains;obj != NULL;obj = obj->next_content)
      {
	 if (obj->pIndexData->vnum == card->pIndexData->vnum && binder->pIndexData->vnum == OBJ_VNUM_DECK)
	 {
 	   if(++counter >= MAX_SAME_CARDS_VNUM)
	   {
	     send_to_char("You already have too many cards with that vnum in your deck.\n\r",ch);
	     return;
	   }
	 }
      }

    }
    obj_from_char (card);
    if (!obj_to_obj_pos (card, binder,pos))
    {
      obj_to_char (card,ch);
      send_to_char("That slot is already taken.\n\r",ch);
      return;
    }
    act ("$n puts $p in $P.", ch, card, binder, TO_ROOM);
    act ("You put $p in $P.", ch, card, binder, TO_CHAR);
  }
  else if (!str_cmp(arg1, "get"))
  {
    OBJ_DATA *card;
    if (!is_number(arg2))
    {
      card = get_obj_list (ch, arg2, binder->contains);
      if (card == NULL)
      {
        act ("You see nothing like that in the $P.", ch, NULL, binder, TO_CHAR);
        return;
      }
      get_obj (ch, card, binder);
      return;
    }
    else 
    {
      pos = atoi(arg2);
      for (card = binder->contains;card != NULL;card = card->next_content)
      {
        if (card->bs_capacity[0] == pos)
	{
  	  obj_from_obj(card);
	  obj_to_char(card,ch);
	  break;
	}
	//Just in case somehow cards get out of order, let it continue recursing even if
	//bs_capacity[0] > pos, so no need to add the check here
      }
      if (card == NULL)
      {
        send_to_char("There is no card in that slot.\n\r",ch);
	return;
      }
      act ("$n gets $p from $P.", ch, card, binder, TO_ROOM);
      act ("You get $p from $P.", ch, card, binder, TO_CHAR);
    }
  }
  else if (!str_cmp(arg1, "list")) 
  {
    char tcbuf[MAX_STRING_LENGTH * 20];
    short type=0;
    if (arg2[0] == '\0')
      type = 0;
    else if (!str_prefix(arg2,"mob"))
      type = CARD_MOB;
    else if (!str_prefix(arg2,"terrain"))
      type = CARD_TERRAIN;
    else if (!str_prefix(arg2,"spell"))
      type = CARD_SPELL_HEAL;
    strcpy (tcbuf, "Cards:\n\r");
    for (obj = binder->contains;obj != NULL;obj = obj->next_content)
    {
      switch (obj->value[1])
      {
        case CARD_MOB:
	  if (type != 0 && type != CARD_MOB)
	    continue;
	  sprintf(arg1,"`d%d) - #%d %s`d - %s - HP: %d - Off: %d / Def: %d\n\r``",
            obj->bs_capacity[0],obj->value[0],obj->short_descr,"Mob",obj->value[7],obj->value[8],obj->value[9]);
	  break;
	case CARD_TERRAIN:
	  if (type != 0 && type != CARD_TERRAIN)
            continue;
	  sprintf(arg1,"`d%d) - #%d %s`d - %s - ",
	    obj->bs_capacity[0],obj->value[0],obj->short_descr,"Terrain");
	  strcat (tcbuf,arg1);
//	    send_to_char(arg1,ch);
	    sprintf(arg1,"%s - %d / %d / %d / %d",flag_string (card_terrain_type, obj->value[7]), 
		obj->value[8],obj->value[9],obj->value[10],obj->value[11]);
	    strcat(tcbuf,arg1);
	    sprintf(arg1," - %s\n\r``",flag_string (card_terrain_toggles, obj->value[12]));
			    
	  break;
	//SPELL
	case CARD_SPELL_HEAL :
	  if (type != 0 && type != CARD_SPELL_HEAL)
            continue;
	  sprintf(arg1,"`d%d) - #%d %s`d - Heal spell - Amount: %d\n\r``",obj->bs_capacity[0],obj->value[0],obj->short_descr, obj->value[7]);
	  break;
	case CARD_SPELL_DAMAGE :
	  if (type != 0 && type != CARD_SPELL_HEAL)
 	    continue;
	  sprintf(arg1,"`d%d) - #%d %s`d - Damage spell - Amount: %d\n\r``",obj->bs_capacity[0],obj->value[0],obj->short_descr, obj->value[7]);
	  break;
	case CARD_SPELL_BOOST:
	  if (type != 0 && type != CARD_SPELL_HEAL)
 	    continue;
	  sprintf(arg1,"`d%d) - #%d %s`d - Boost spell - Off: %d/ Def: %d\n\r``",obj->bs_capacity[0],obj->value[0],obj->short_descr, obj->value[7], obj->value[8]);
	  break;
	case CARD_SPELL_COUNTER:
	  if (type != 0 && type != CARD_SPELL_HEAL)
  	    continue;
	  sprintf(arg1,"`d%d) - #%d %s`d - Counter spell\n\r``",obj->bs_capacity[0],obj->value[0],obj->short_descr);
	  break;
	default: sprintf(arg1,"`d%d) - #%d %s - %s\n\r``",obj->bs_capacity[0],obj->value[0],obj->short_descr,flag_string (card_Class, obj->value[1]));
      }
  //    send_to_char(arg1,ch);
        strcat (tcbuf,arg1);
    }
    page_to_char (tcbuf, ch);
  }
  else if (!str_cmp(arg1, "check"))
  {
    if (!is_number(arg2))
      {
	obj = get_obj_list (ch, arg2, binder->contains);
	if (obj == NULL)
	  {
	    act ("You see nothing like that in the $P.", ch, NULL, binder, TO_CHAR);
	    return;
	  }
	appraise(ch,obj);
	return;
      }
    else
      {
	pos = atoi(arg2);
	for (obj = binder->contains;obj != NULL;obj = obj->next_content)
	  {
	    if (obj->bs_capacity[0] == pos)
	      {
		appraise(ch,obj);
		break;
	      }
	    //Just in case somehow cards get out of order, let it continue recursing even if
	    //bs_capacity[0] > pos, so no need to add the check here
	  }
	if (obj == NULL)
	  {
	    send_to_char("There is no card in that slot.\n\r",ch);
	    return;
	  }
      }
  }    
}
				      
void do_appraise(CHAR_DATA *ch, char* argument)
{
  OBJ_DATA *card;
  if ((card = get_obj_here (ch, argument)) == NULL)
  {
    send_to_char("You don't see that here.\n\r",ch);
    return;
  }
  if (card->item_type != ITEM_CARD)
  {
    send_to_char("You can't appraise that!\n\r",ch);
    return;
  }
  appraise(ch,card);
}


void appraise(CHAR_DATA *ch, OBJ_DATA* card)
{
   appraise_new(ch,card,-1,ch);
}

//Iblis - 6/24/04 - Short circuited appraise so this same command could be used to appraise
//mobs in play (substituting the proper off/def/hp of the mob from the pcdata stats)
void appraise_new(CHAR_DATA *ch, OBJ_DATA* card, int mob_number, CHAR_DATA *mob_owner)
{
  char buf[MAX_STRING_LENGTH];
  send_to_char("`k+ \n\r``",ch);
  sprintf(buf,"`o|`h %s\n\r`o|`h\n\r``",card->short_descr);
  send_to_char(buf,ch);
  sprintf(buf,"`o|`h Card Number:`k %d\n\r",card->value[0]);
  send_to_char(buf,ch);
  send_to_char("`o| `hRarity:`k ",ch);
  switch(card->pIndexData->rarity)
  {
	  default :
    case RARITY_ALWAYS: send_to_char("very common\n\r`o|`h\n\r``",ch);break;
    case RARITY_COMMON: send_to_char("common\n\r`o|`h\n\r``",ch);break;
    case RARITY_SOMEWHAT_COMMON: send_to_char("somewhat_common\n\r`o|`h\n\r``",ch);break;
    case RARITY_UNCOMMON: send_to_char("uncommon\n\r`o|`h\n\r``",ch);break;
    case RARITY_RARE: send_to_char("rare\n\r`o|`h\n\r``",ch);break;
    case RARITY_VERY_RARE: send_to_char("very rare\n\r`o|`h\n\r``",ch);break;
  }
	    
  switch (card->value[1])
  {
    case CARD_MOB:
      if (mob_number == 1)
      {
	sprintf(buf,"`o|`h Offence:`k %d\n\r``",mob_owner->pcdata->cg_mm_value[CARDGAME_MOB_OFF]);
        send_to_char(buf,ch);
        sprintf(buf,"`o|`h Defence:`k %d\n\r``",mob_owner->pcdata->cg_mm_value[CARDGAME_MOB_DEF]);
	send_to_char(buf,ch);
	sprintf(buf,"`o|`h Hitpoints:`k %d\n\r`o|`h\n\r``",mob_owner->pcdata->cg_mm_value[CARDGAME_MOB_HP]);
	send_to_char(buf,ch);
      }
      else if (mob_number == 2)
      {
	sprintf(buf,"`o|`h Offence:`k %d\n\r``",mob_owner->pcdata->cg_sm_value[CARDGAME_MOB_OFF]);
        send_to_char(buf,ch);
	sprintf(buf,"`o|`h Defence:`k %d\n\r``",mob_owner->pcdata->cg_sm_value[CARDGAME_MOB_DEF]);
	send_to_char(buf,ch);
	sprintf(buf,"`o|`h Hitpoints:`k %d\n\r`o|`h\n\r``",mob_owner->pcdata->cg_sm_value[CARDGAME_MOB_HP]);
	send_to_char(buf,ch);
      }
      else 
      {
        sprintf(buf,"`o|`h Offence:`k %d\n\r``",card->value[8]);
        send_to_char(buf,ch);
        sprintf(buf,"`o|`h Defence:`k %d\n\r``",card->value[9]);
        send_to_char(buf,ch);
        sprintf(buf,"`o|`h Hitpoints:`k %d\n\r`o|`h\n\r``",card->value[7]);
        send_to_char(buf,ch);
      }
      sprintf(buf,"`o|`h Race:`k %s\n\r``",flag_string (card_race_type, card->value[10]));
      send_to_char(buf,ch);
      sprintf(buf,"`o|`h Nemesis:`k %s\n\r`o|`h\n\r``",flag_string (card_race_type, card->value[12]));
      send_to_char(buf,ch);
      switch (card->value[11])
      {
        case CARD_ELEMENT_FIRE : send_to_char("`o| `a[`iF`a]``\n\r`k+``\n\r",ch);break;
	case CARD_ELEMENT_LIGHTNING :send_to_char("`o| `a[`kL`a]``\n\r`k+``\n\r",ch);break;
	case CARD_ELEMENT_STONE : send_to_char("`o| `a[`dS`a]``\n\r`k+``\n\r",ch);break;
	case CARD_ELEMENT_WATER : send_to_char("`o| `a[`eW`a]``\n\r`k+``\n\r",ch);break;
	default : /*send_to_char("`o| `a[`iF`a]\n\r`k+\n\r".ch);*/break;
      }
      break;
    case CARD_TERRAIN:
      sprintf(buf,"`o|`h Type:`k %s\n\r``",flag_string (card_terrain_type, card->value[7]));
      send_to_char(buf,ch);
      sprintf(buf,"`o|`h Off Bonus for Non-Elemental Mobs/Damage:`k %d\n\r``",card->value[8]);
      send_to_char(buf,ch);
      sprintf(buf,"`o|`h Def Bonus for Non-Elemental Mobs Bonus:`k %d\n\r``",card->value[9]);
      send_to_char(buf,ch);
      sprintf(buf,"`o|`h Off Bonus for Elemental Mobs/Damage Bonus:`k %d\n\r``",card->value[10]);
      send_to_char(buf,ch);
      sprintf(buf,"`o|`h Def Bonus for Elemental Mob Bonus:`k %d\n\r`o|`h\n\r``",card->value[11]);
      send_to_char(buf,ch);
      sprintf(buf,"`o|`h Special Flags:`k %s\n\r`k+``\n\r",flag_string (card_terrain_toggles, card->value[12]));
      send_to_char(buf,ch);
      break;
    case CARD_SPELL_HEAL:
      sprintf(buf,"`o|`h Heal Amount:`k %d\n\r`k+``\n\r",card->value[7]);
      send_to_char(buf,ch);
      break;
    case CARD_SPELL_DAMAGE:
      sprintf(buf,"`o|`h Damage Amount:`k %d\n\r``",card->value[7]);
      send_to_char(buf,ch);
      switch (card->value[8])
	{
        case CARD_ELEMENT_FIRE : send_to_char("`o|`h\n\r`o| `a[`iF`a]``\n\r`k+\n\r``",ch);break;
        case CARD_ELEMENT_LIGHTNING :send_to_char("`o|`h\n\r`o| `a[`kL`a]``\n\r`k+\n\r``",ch);break;
        case CARD_ELEMENT_STONE : send_to_char("`o|`h\n\r`o| `a[`dS`a]``\n\r`k+\n\r``",ch);break;
        case CARD_ELEMENT_WATER : send_to_char("`o|`h\n\r`o| `a[`eW`a]``\n\r`k+\n\r``",ch);break;
        default : send_to_char("`o|`h\n\r`o| `a[`h0`a]\n\r`k+``\n\r",ch);break;
	}
      break;
  case CARD_SPELL_BOOST:
    sprintf(buf,"`o|`h Off Bonus:`k %d\n\r``",card->value[7]);
    send_to_char(buf,ch);
    sprintf(buf,"`o|`h Def Bonus:`k %d\n\r`k+``\n\r",card->value[8]);
    send_to_char(buf,ch);
    break;
  }
}

extern int top_obj_index;

	
void do_cardcheck(CHAR_DATA *ch, char* argument)
{
  //if ((pObjIndex = get_obj_index (vnum)) != NULL)
  //{
  int nMatch=0;
  int counter=1;
  OBJ_INDEX_DATA *pObjIndex;
  int vnum = 0;
  char buf[MAX_STRING_LENGTH];
  if (str_cmp(argument,"DINKLE"))
  {
	  send_to_char("To execute this command, do cardcheck DINKILE.  BEWARE IT LAGS THE MUD TO SHIT.\n\r",ch);
	  return;
  }
	  
  //  nMatch++;
  for (vnum = 0; nMatch < top_obj_index; vnum++)
    {
      if (vnum > 40000)
	{
	  bug("Too many vnums in olc_act.c",0);
	  return;
	}
      if ((pObjIndex = get_obj_index (vnum)) != NULL)
	{
	  nMatch++;

	  if (pObjIndex->item_type != ITEM_CARD)
	    continue;
	  if (pObjIndex->value[0] == counter)
	    {
//		    sprintf(buf,"!%d!",pObjIndex->value[0]);
//	      send_to_char(buf,ch);
	      vnum = 0;
	      nMatch = 0;
	      ++counter;
	    }
	}
    }
  sprintf(buf,"The last number unaccounted for is %d.\n\r",counter);
  send_to_char(buf,ch);
  //}
}

void do_cardduel(CHAR_DATA *ch, char* argument)
{
  CHAR_DATA *rch;
  //If a game is going on here, don't let them start it.  
  //If a proposed game is going on here, don't let them start it - DONE
  if (argument[0] == '\0')
  {
    if (!valid_deck(ch))
    {
      send_to_char("Your deck is not valid.  It must have exactly 60 cards, with at least 12 terrain and 12 mob cards.\n\r",ch);
      return;
    }
	   
    for (rch = ch->in_room->people;rch != NULL;rch = rch->next_in_room)
    {
      if (IS_NPC(rch))
        continue;
      if (rch->pcdata->cg_ticks > 0)
      {
        send_to_char("Someone has already started a card game here.\n\r",ch);
	return;
      }
      if (rch->pcdata->cg_state != 0)
      {
	send_to_char("Someone is already playing cards here.\n\r",ch);
        return;
      }
    }
    send_to_char("You let the room know you're ready for a card game.\n\r",ch);
    ch->pcdata->cg_ticks = 3;
    act("$n wants to play cards.  '`kcardduel join``' to join $m.",ch,NULL,NULL,TO_ROOM);
  }
  else if (!str_cmp(argument,"join"))
  {
    if (!valid_deck(ch))
    {
      send_to_char("Your deck is not valid.  It must have exactly 60 cards, with at least 12 terrain and 12 mob cards.\n\r",ch);
      return;
    }
			   
    send_to_char("You let it be known you wish to join the card game.\n\r",ch);
    ch->pcdata->cg_ticks = -1;
    act("$n wants to join the game!",ch,NULL,NULL,TO_ROOM);
  }
  else if (!str_cmp(argument,"quit"))
  {
    if (ch->pcdata->cg_ticks > 0)
    {
      for (rch = ch->in_room->people;rch != NULL;rch = rch->next_in_room)
      {
        if (IS_NPC(rch))
          continue;
	if (rch->pcdata->cg_ticks == -1)
	{
          rch->pcdata->cg_ticks = ch->pcdata->cg_ticks;
	  break;
	}
      }
    }		    
    send_to_char("You take yourself out of the card game.\n\r",ch);
    act("$n opts to leave the card game.",ch,NULL,NULL,TO_ROOM);
    ch->pcdata->cg_ticks = 0;
  }
  else
  { 
    send_to_char("Syntax: `kCardduel``       - To start a new game\n\r",ch);
    send_to_char("        `kCardduel join``  - To join a game\n\r",ch);
    send_to_char("        `kCardduel quit``  - To withdraw from an impending game\n\r",ch);
  }
}


void cardgame_update()
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *ch, *rch;
  for (d = descriptor_list;d != NULL; d = d->next)
  {
    if (d->connected == CON_PLAYING)
    {
      if (!d->character->pcdata || d->character->pcdata->cg_ticks <= 0 || !d->character->in_room || d->character->pcdata->cg_state != 0)
        continue;
      if (d->character->pcdata->cg_ticks == 1)
      {
	bool has_begun = FALSE;
	CHAR_DATA *last_joined=NULL;
	short count=0;
	OBJ_DATA  *deck;
        //scan room and look to see if anyone has joined
	for (ch = d->character->in_room->people;ch != NULL;ch = ch->next_in_room)
	{
	  if (IS_NPC(ch))
       	    continue;
	  if (ch->pcdata->cg_ticks == -1)
	  {
	    if (!valid_deck(ch))
	    {
	      send_to_char("Your deck is invalid, so you cannot join the game.\n\r",ch);
	      continue;
	    }
 	    if (!has_begun)
	    {
	      act("Let the game begin!",d->character,NULL,NULL,TO_ALL);
	      act("$n begins the game.",d->character,NULL,NULL,TO_ROOM);
	      send_to_char("You begin the game!\n\r",d->character);
	      d->character->pcdata->cg_next = ch;
	      d->character->pcdata->cg_ticks = 0;
	      
	      deck = get_obj_vnum_char(d->character,OBJ_VNUM_DECK);
              //This is set so they cannot manipulate the contents of the deck DURING the game
              SET_BIT(deck->extra_flags[1],ITEM_WIZI);
	      
	      has_begun = TRUE;
              last_joined = d->character;
	      ++count;
	    }
	    ++count;
	    if (count > 10)
	    {
  	      send_to_char("I'm sorry the game is full.\n\r",ch);
	      act("$n wanted to join, but the game is full!\n\r",ch,NULL,NULL,TO_ROOM);
	      continue;
	    }
	    last_joined->pcdata->cg_next = ch;
  	    last_joined = ch;
	    ch->pcdata->cg_ticks = 0;

	    deck = get_obj_vnum_char(ch,OBJ_VNUM_DECK);
            //This is set so they cannot manipulate the contents of the deck DURING the game
	    SET_BIT(deck->extra_flags[1],ITEM_WIZI);
	    
	    ch->pcdata->cg_next = d->character;
	    send_to_char("You join the game!\n\r",ch);
	    act("$n joins the game!",ch,NULL,NULL,TO_ROOM);
	  }
	}
	if (!has_begun)
	{
  	  act("Not enough people have joined the game, so it will not happen.",d->character,NULL,NULL,TO_ALL);
	  d->character->pcdata->cg_ticks = 0;
	  return;
	}

	//Iblis - 8/10/04 - Ante
        for (ch = d->character->in_room->people;ch != NULL;ch = ch->next_in_room)
	  {
	    OBJ_DATA *ante_bag;
	    if (IS_NPC(ch) || ch->pcdata->cg_next == NULL)
	      continue;
	    ch->pcdata->cg_state = CG_ANTE;
	    ante_bag = create_object(get_obj_index(OBJ_VNUM_ANTE_BAG),0);
	    //Use it's own vnum so there is never a chance of a ball of light restrung as zante causing problems
	    SET_BIT(ante_bag->extra_flags[1],ITEM_WIZI);
	    free_string (ante_bag->name);
	    ante_bag->name = str_dup ("zante");
	    ante_bag->item_type = ITEM_CONTAINER;
	    obj_to_char(ante_bag,ch);
	   
	    send_to_char("You are now in the ante-ing stage of the pregame show.\n\r",ch);
	    send_to_char("You may use the ante commands now to place, view, accept, and decline the ante.\n\r",ch);
	    do_ante(ch,"");
            for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
	    {
	      rch->pcdata->cg_state = CG_ANTE;
	      ante_bag = create_object(get_obj_index(OBJ_VNUM_ANTE_BAG),0);
	      //Use it's own vnum so there is never a chance of a ball of light restrung as zante causing problems
	      SET_BIT(ante_bag->extra_flags[1],ITEM_WIZI);
	      free_string (ante_bag->name);
	      ante_bag->name = str_dup ("zante");
	      ante_bag->item_type = ITEM_CONTAINER;
	      obj_to_char(ante_bag,rch);

  	      send_to_char("You are now in the ante-ing stage of the pregame show.\n\r",rch);
	      send_to_char("You may use the ante commands now to place, view, accept, and decline the ante.\n\r",rch);
	      do_ante(rch,"");
	    }
	    return;
	  }
	
	continue;
      } 
      else act("A card game is about to start!  '`kcardduel join``' to join in!",d->character,NULL,NULL,TO_ALL);
      d->character->pcdata->cg_ticks--;
      
    }
   }
}

     
      

//Iblis - 6/16/04 - Draws a single card from the player's deck
OBJ_DATA *draw_card(CHAR_DATA *ch)
{
  OBJ_DATA *deck,*card;
  int amount=0,counter=0;
  deck = get_obj_vnum_char(ch, OBJ_VNUM_DECK);
  for (card = deck->contains;card != NULL; card = card->next_content,++amount);
  amount = number_range(1,amount);
  for (card = deck->contains;card != NULL && counter != amount; card = card->next_content,++counter);
  if (card == NULL)
   card = deck->contains;
  if (card == NULL)
    bug ("ERROR, draw_card is returning null!",0);
  obj_from_obj(card);
  act(" You draw $p. ",ch,card,NULL,TO_CHAR);
  return card;
}

//Iblis - 6/16/04 - Returns true if the player has a valid deck
//A valid deck = exactly 60 cards, at least 12 terrain cards and at least 12 mob cards
bool valid_deck(CHAR_DATA *ch)
{
  int mob_counter=0, terrain_counter=0, counter_counter=0;
  OBJ_DATA *deck = get_obj_vnum_char(ch,OBJ_VNUM_DECK), *card;
  if (deck == NULL)
    return FALSE;
  for (card = deck->contains;card != NULL;card = card->next_content)
  {
    if (card->value[1] == CARD_MOB)
      ++mob_counter;
    else if (card->value[1] == CARD_TERRAIN)
      ++terrain_counter;
    ++counter_counter;
  }
  if (counter_counter != 60)
    return FALSE;
  if (terrain_counter < 12)
    return FALSE;
  if (mob_counter < 12)
    return FALSE;
  return TRUE;
}

  
	  
      
      
  
  
		  
  
OBJ_DATA *get_obj_vnum_string_char(CHAR_DATA *ch, int vnum, char* arg)
{
  OBJ_DATA *obj;
  for (obj = ch->carrying;obj != NULL;obj = obj->next_content)
  {
    if (obj->pIndexData->vnum == vnum)
      if (!str_cmp(obj->name,arg))
	      return obj;
  }
  return NULL;
}

void cardgame_hand_display(CHAR_DATA *ch)
{
  int i=1;
  char arg1[MAX_STRING_LENGTH];
  OBJ_DATA *hand, *obj;
  send_to_char("Your Hand\n\r",ch);
  hand = get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand");
  for (obj = hand->contains;obj != NULL;obj = obj->next_content,++i)
    {
      switch (obj->value[1])
	{
	case CARD_MOB:
	  sprintf(arg1,"`d%d) - #%d %s`d - %s - HP: %d - Off: %d / Def: %d\n\r``",
		  i,obj->value[0],obj->short_descr,"Mob",obj->value[7],obj->value[8],obj->value[9]);
	  break;
	case CARD_TERRAIN:
	  sprintf(arg1,"`d%d) - #%d %s`d - %s - ",
		  i,obj->value[0],obj->short_descr,"Terrain");
	  send_to_char(arg1,ch);
	  sprintf(arg1,"%s - %d / %d / %d / %d",flag_string (card_terrain_type, obj->value[7]),
		  obj->value[8],obj->value[9],obj->value[10],obj->value[11]);
	  send_to_char(arg1,ch);
	  sprintf(arg1," - %s\n\r``",flag_string (card_terrain_toggles, obj->value[12]));
	  
	  break;
	  //SPELL
	case CARD_SPELL_HEAL :
	  sprintf(arg1,"`d%d) - #%d %s`d - Heal spell - Amount: %d\n\r``",i,obj->value[0],obj->short_descr, obj->value[7]);
	  break;
	case CARD_SPELL_DAMAGE :
	  sprintf(arg1,"`d%d) - #%d %s`d - Damage spell - Amount: %d\n\r``",i,obj->value[0],obj->short_descr, obj->value[7]);
	  break;
	case CARD_SPELL_BOOST:
	  sprintf(arg1,"`d%d) - #%d %s`d - Boost spell - Off: %d/ Def: %d\n\r``",i,obj->value[0],obj->short_descr, obj->value[7], obj->value[8]);
	  break;
	case CARD_SPELL_COUNTER:
	  sprintf(arg1,"`d%d) - #%d %s`d - Counter spell\n\r``",i,obj->value[0],obj->short_descr);
	  break;
	default: sprintf(arg1,"`d%d) - #%d %s - %s\n\r``",i,obj->value[0],obj->short_descr,flag_string (card_Class, obj->value[1]));
	}
          send_to_char(arg1,ch);
    }
}


void cardgame_attack_menu(CHAR_DATA *ch)
{

  if ((ch->pcdata->cg_state == CG_YT_DONE) || ch->pcdata->cg_timer == 0 || ((ch->pcdata->cg_ticks & CARDGAME_PLAYED_TERRAIN) && ((ch->pcdata->cg_ticks & (CARDGAME_PLAYED_TERRAIN-1)) <= 0)))
    send_to_char("`a[A] Play a card``\n\r",ch);
  else send_to_char("[A] Play a card\n\r",ch);
  if (ch->pcdata->cg_state != CG_YT_DONE && (ch->pcdata->cg_ticks & (CARDGAME_PLAYED_TERRAIN-1)) <= 0 && ch->pcdata->cg_timer != 0)
    send_to_char("[B] Attack with your mobs\n\r",ch);
  else send_to_char("`a[B] Attack with your mobs``\n\r",ch);
  /*if (ch->pcdata->cg_state == CG_YT_DONE)
    send_to_char("`a[C] Pass / End Your Turn``\n\r",ch);
  else send_to_char("`c[C] Pass / End Your Turn``\n\r",ch);*/
  send_to_char("[C] Scan the cards on the table``\n\r",ch);
  send_to_char("[D] Appraise a card on the table``\n\r",ch);
  send_to_char("[E] Scan the mobs in play``\n\r",ch);
  if (count_contents(get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand")) > 0)
    send_to_char("[G] Discard a card``\n\r",ch);
  else send_to_char("`a[G] Discard a card``\n\r",ch);
  if (ch->pcdata->cg_state != CG_YT_DONE)
    send_to_char("[Y] Finished (for this round)``\n\r",ch);
  else send_to_char("`a[Y] Finished (for this round)``\n\r",ch);
  send_to_char("[Z] Surrender``\n\r",ch);
}

void cardgame_default_menu(CHAR_DATA *ch)
{
  //First, get the player whose turn it is
  CHAR_DATA *player;
  for (player = ch->pcdata->cg_next;player != ch;player = player->pcdata->cg_next)
  {
    if (player->pcdata->cg_state <= CG_YT_DONE)
      break;
  }
  //If the player whose turn it is has attacked with their mobs, you're allowed to play a card
  if (((player->pcdata->cg_ticks & (CARDGAME_PLAYED_TERRAIN-1)) > 0) && !ch->pcdata->cg_spell_target && ch->pcdata->cg_state != CG_NYT_DONE)
    send_to_char("[A] Play a card``\n\r",ch);
  else send_to_char("`a[A] Play a card``\n\r",ch);
  send_to_char("[C] Scan the cards on the table``\n\r",ch);
  send_to_char("[D] Appraise a card on the table``\n\r",ch);
  send_to_char("[E] Scan the mobs in play``\n\r",ch);
  if (IS_SET(ch->pcdata->cg_ticks,CARDGAME_BEEN_ATTACKED) && !(get_mobnumber(ch->pcdata->cg_ticks))
		  && ch->pcdata->cg_state != CG_NYT_DONE)
	  send_to_char("[F] Choose a mob to use to defend``\n\r",ch);
  else send_to_char("`a[F] Choose a mob to use to defend``\n\r",ch);
  if (ch->pcdata->cg_state != CG_NYT_DONE)
    send_to_char("[Y] Finished (for this round)``\n\r",ch);
  else send_to_char("`a[Y] Finished (for this round)``\n\r",ch);
  send_to_char("[Z] Surrender``\n\r",ch);
}

void cardgame_opponents_display(CHAR_DATA *ch)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *rch;
//  int counter=1;
  send_to_char("\n\r",ch);
  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
  {
    sprintf(buf,"[%d] %s\n\r",rch->pcdata->cg_player_number,rch->name);
    send_to_char(buf,ch);
  }
  send_to_char("[0] Previous Menu\n\r",ch);
}

void cardgame_spellcountertarget_display(CHAR_DATA *ch)
{
  CHAR_DATA *rch;
  char buf[MAX_STRING_LENGTH];
//  int counter=1;
  send_to_char("\n\r",ch);
  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
    {
      if (rch->pcdata->cg_lcp != NULL)
        sprintf(buf,"``[%d] %s\n\r",rch->pcdata->cg_player_number,rch->name);
      else sprintf(buf,"``[%d] %s\n\r",rch->pcdata->cg_player_number,rch->name);
      send_to_char(buf,ch);
    }
  send_to_char("[0] Previous Menu\n\r",ch);
}



void cardgame_spelltarget_display(CHAR_DATA *ch)
{
  CHAR_DATA *rch;
  //int counter=1;
  char buf[MAX_STRING_LENGTH];
  if (IS_SET(ch->pcdata->cg_ticks,CARDGAME_BEEN_ATTACKED))
    send_to_char("``[1] Your defending mob\n\r",ch);
  else
  {
    for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
    {
      if (IS_SET(rch->pcdata->cg_ticks,CARDGAME_BEEN_ATTACKED))
	{
	  //Change this to the name of their mob (maybe give stats too)
	  act("``[1] $N's defending mob",ch,NULL,rch,TO_CHAR);
/*          sprintf(buf,"[1] %s's defending mob\n\r",rch->name);
          send_to_char(buf,ch);*/
	  break;
	}
    }
  }
  if (ch->pcdata->cg_state <= CG_YT_DONE)
    {
      if (get_mobnumber(ch->pcdata->cg_ticks) == 1)
	send_to_char("[2] Your Main Mob (which is attacking)\n\r",ch);
      else send_to_char("[2] Your Secondary Mob (which is attacking)\n\r",ch);
    }
  else
    {
      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
	{
	  if (rch->pcdata->cg_state <= CG_YT_DONE)
	    {
	      //Change this to the name of their mob (maybe give stats too)
	      sprintf(buf,"[2] %s's attacking mob\n\r",rch->name);
	      send_to_char(buf,ch);
	      break;
	    }
	}
    }
  send_to_char("[0] Previous Menu\n\r",ch);
}


OBJ_DATA *get_obj_pos(OBJ_DATA *obj_from,int pos)
{
  int counter=0;
  OBJ_DATA *obj;
  for (obj = obj_from->contains;obj != NULL;obj = obj->next_content)
  {
    if (++counter == pos)
    {
      obj_from_obj(obj);
      return obj;
//      obj_to_char(obj,ch);
      break;
    }
    //Just in case somehow cards get out of order, let it continue recursing even if
    //bs_capacity[0] > pos, so no need to add the check here
  }
  return NULL;
}     

void discard_card(CHAR_DATA *ch, OBJ_DATA* card)
{
  OBJ_DATA* discard;
  discard = get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zdiscard");
  if (discard == NULL)
  {
    //create the "discard" object, containing the discarded cards
    discard = create_object(get_obj_index(OBJ_VNUM_BINDER),0);
    SET_BIT(discard->extra_flags[1],ITEM_WIZI);
    free_string (discard->name);
    discard->name = str_dup ("zdiscard");
    discard->value[0] = 0;
    obj_to_char(discard,ch);
  }
  obj_to_obj(card,discard);
}

void cardgame_menu(CHAR_DATA *ch, char* argument)
{
  int cardno=0;
  OBJ_DATA *card=NULL, *hand;
  CHAR_DATA *rch, *victim;
  int victimno=0;
  char buf[MAX_STRING_LENGTH];
//  bool legitvictim = FALSE;
//  OBJ_DATA *card=NULL;
  int num;// = //atoi(argument);
  //  cardgame_menu_display(ch);
  switch (ch->pcdata->cg_state)
    {
    case CG_START_PT:
      cardno = atoi(argument);
      if (cardno < 0 || cardno > 8)
	{
	  send_to_char("That is not a valid number.\n\r",ch);
	  cardgame_hand_display(ch);
	  send_to_char("Pick a terrain from your hand to play (1-8).\n\r",ch);
	  return;
	}
      hand = get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand");
      
      //Just in case, DO NOT CRASH
      if (hand == NULL)
        return;
      
      card = get_obj_pos(hand,cardno);
      
      //If for some reason there is no card at that position
      if (card == NULL)
	{
	  send_to_char("That is not a valid number.\n\r",ch);
	  cardgame_hand_display(ch);
	  send_to_char("Pick a terrain from your hand to play (1-8).\n\r",ch);
	  return;
	}
      if (card->value[1] != CARD_TERRAIN)
      {
        send_to_char("That is not a valid terrain!\n\r",ch);

	//Put the card back into their hand
	obj_to_obj_pos(card,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),-1);
        cardgame_hand_display(ch);
        send_to_char("Pick a terrain from your hand to play (1-8).\n\r",ch);
        return;
      }
      
      act(" $n plays $p as the initial terrain card. ",ch,card,NULL,TO_ROOM);
      act(" You play $p as the initial terrain card. ",ch,card,NULL,TO_CHAR);
      appraise_all(ch,card);
      ch->pcdata->cg_terrain = card;
      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
        rch->pcdata->cg_terrain = card;
      discard_card(ch,card);
      
      ch->pcdata->cg_state = CG_START_PM;
      cardgame_hand_display(ch);
      send_to_char("Pick a mob from your hand to play as your Main Mob (1-7).\n\r",ch);
      break;
    case CG_START_PM_YT:
    case CG_START_PM:
      cardno = atoi(argument);
      if (cardno < 0 || cardno > 7)
	{
	  send_to_char("That is not a valid number.\n\r",ch);
	  cardgame_hand_display(ch);
	  send_to_char("Pick a mob from your hand to play as your Main Mob (1-7).\n\r",ch);
	  return;
	}
      hand = get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand");

      //Just in case, DO NOT CRASH
      if (hand == NULL)
        return;
      
      card = get_obj_pos(hand,cardno);
      
      //If for some reason there is no card at that position
      if (card == NULL)
	{
	  send_to_char("That is not a valid number.\n\r",ch);
	  cardgame_hand_display(ch);
	  send_to_char("Pick a mob from your hand to play (1-7).\n\r",ch);
	  return;
	}
      if (card->value[1] != CARD_MOB)
      {
        send_to_char("That is not a valid mob.\n\r",ch);
	//Put it back into their hand
	obj_to_obj_pos(card,hand,-1);
        cardgame_hand_display(ch);
        send_to_char("Pick a mob from your hand to play (1-7).\n\r",ch);
	return;
      }
      
      act(" $n plays $p as $s Main Mob. ",ch,card,NULL,TO_ROOM);
      act(" You play $p as your Main Mob. ",ch,card,NULL,TO_CHAR);
      appraise_all(ch,card);
      ch->pcdata->cg_main_mob = card;
      ch->pcdata->cg_mm_value[CARDGAME_MOB_HP] = card->value[VALUE_MOB_HP];
      ch->pcdata->cg_mm_value[CARDGAME_MOB_OFF] = card->value[VALUE_MOB_OFFENCE];
      ch->pcdata->cg_mm_value[CARDGAME_MOB_DEF] = card->value[VALUE_MOB_DEFENCE];
      discard_card(ch,card);
      //Since it's a main mob, its hp is set to 10
      ch->pcdata->cg_mm_value[CARDGAME_MOB_HP] = 10;
      if (ch->pcdata->cg_state == CG_START_PM)
	{
  	  bool donotstart = FALSE;
	  ch->pcdata->cg_state = CG_NYT_MAIN;
	  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
	  {
  	    if (rch->pcdata->cg_state <= CG_START_PT)
	    {
	      donotstart = TRUE;
	      break;
	    }
	  }
	  //Everyone has chosen a main mob, so the timer may start
	  if (!donotstart)
	  {
            for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
	    {
 	      if (rch->pcdata->cg_state >= CG_YT_MAIN && rch->pcdata->cg_state <= CG_YT_DONE)
	      {
  	        rch->pcdata->cg_timer = current_time;
		act(" Everyone has chosen a main mob, so the game now begins! ",rch,NULL,NULL,TO_ALL);
		//Since everyone has a terrain and mob now, add in all terrain bonuses....NOW
		terrainify_mobs(ch,NULL,ch->pcdata->cg_terrain, ch->pcdata->cg_main_mob, ch->pcdata->cg_secondary_mob);
		for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
		{
  	 	  terrainify_mobs(rch,NULL,ch->pcdata->cg_terrain,ch->pcdata->cg_main_mob, ch->pcdata->cg_secondary_mob);
		  if (their_turn(rch))
 	  	    cardgame_attack_menu(rch);
		}
		break;
	      }
	    }
	  }
	  cardgame_default_menu(ch);
	  //DISPLAY THE NOT YOUR TURN MENU HERE
	  return;
	}
      if (ch->pcdata->cg_state == CG_START_PM_YT)
	{
 	  bool donotstart = FALSE;
	  ch->pcdata->cg_state = CG_YT_MAIN;
	  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
          {
            if (rch->pcdata->cg_state <= CG_START_PT)
            {
              donotstart = TRUE;
              break;
            }
          }
	  //Since this character goes first, we set his timer up and off the game goes!
	  if (!donotstart)
	    {
	      ch->pcdata->cg_timer = current_time;
	      act(" Everyone has chosen a main mob, so the game now begins! ",ch,NULL,NULL,TO_ALL);
	    }
	  if (count_contents(get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand")) < 7)
	  {
		  obj_to_obj_pos(draw_card(ch),get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),-1);
		  act(" $n draws a card. ",ch,NULL,NULL,TO_ROOM);
		  //send_to_char("You draw a card.\n\r",ch);
	  }
	  cardgame_attack_menu(ch);
	  //DISPLAY THE YOUR TURN MENU HERE
	  return;
	}
      break;
    case CG_YT_MAIN:
      switch (argument[0])
	{
	default :
 	  cardgame_attack_menu(ch);
	  return;
	  //Play a card
	case 'a':
	case 'A': 
 	  //If they've played a card prior to attacking BUT have not attacked
	  if ((IS_SET(ch->pcdata->cg_ticks, CARDGAME_PLAYED_TERRAIN) 
			  && (ch->pcdata->cg_ticks & (CARDGAME_PLAYED_TERRAIN-1)) <= 0) || ch->pcdata->cg_timer == 0)
	  {
		  send_to_char("INVALID OPTION!\n\r",ch);
		  cardgame_attack_menu(ch);
		   return;
	  }
	  
	  //If it is pre-attack       (above checks if they played terrain and if they played mob it would not
	  //				longer be their turn)
          if (ch->pcdata->cg_ticks <= 0)
	    ch->pcdata->cg_state = CG_YT_PA_PLAYCARD;
	  else ch->pcdata->cg_state = CG_YT_AA_PLAYCARD;
	  cardgame_hand_display(ch);
	  send_to_char("[0] Previous Menu\n\r",ch);
	  send_to_char("Which card do you want to play?\n\r",ch);
	  break;
        //Attack with your mobs
	case 'b':
	case 'B':
	  if (((ch->pcdata->cg_ticks & (CARDGAME_PLAYED_TERRAIN-1)) > 0) || ch->pcdata->cg_timer == 0)
	    {
	      send_to_char("INVALID OPTION!\n\r",ch);
	      // ch->pcdata->cg_state = CG_YT_MAIN;
	      cardgame_attack_menu(ch);
	      return;
	    }
	  ch->pcdata->cg_state = CG_YT_A_PM;
	  if (ch->pcdata->cg_main_mob)
	    {
	      send_to_char("[1] Main Mob\n\r",ch);
	      appraise_new(ch,ch->pcdata->cg_main_mob,1,ch);
	    }
	  if (ch->pcdata->cg_secondary_mob)
	    {
	      send_to_char("[2] Secondary Mob\n\r",ch);
  	      appraise_new(ch,ch->pcdata->cg_secondary_mob,2,ch);
	    }
	  send_to_char("[0] Previous Menu\n\r",ch);
	  send_to_char("Attack with which mob?\n\r",ch);
	  break;

	//Scan the table
	case 'c':
	case 'C':
	  table_scan(ch);
	  return;
	case 'd':
	case 'D':
	  ch->pcdata->cg_state = CG_YT_APPRAISE;
	  appraise_list(ch);
	  send_to_char("Appraise which card number?\n\r",ch);
	  return;
	case 'e':
	case 'E':
	  mobs_scan(ch);
	  return;
	case 'g':
	case 'G':
	  if (count_contents(get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand")) == 0)
	  {
	    send_to_char("INVALID CHOICE!!!\n\r",ch);
	    return;
	  }
	  if (ch->pcdata->cg_state == CG_YT_DONE)
 	    ch->pcdata->cg_state = CG_YT_DISCARD_D;
	  else ch->pcdata->cg_state = CG_YT_DISCARD;
	  cardgame_hand_display(ch);
	  send_to_char("[0] Previous Menu\n\r",ch);
	  send_to_char("Discard which card?\n\r",ch);
	  return;
	case 'y':
	case 'Y':
	  ch->pcdata->cg_state = CG_YT_DONE;
	  send_to_char(" You declare yourself ready. \n\r",ch);
	  act(" $n declares $e is finished for the round. ",ch,NULL,NULL,TO_ROOM);
	  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
	  {
	    if (rch->pcdata->cg_state != CG_YT_DONE && rch->pcdata->cg_state != CG_NYT_DONE
  	         && rch->pcdata->cg_state != CG_YT_APPRAISE_D && rch->pcdata->cg_state != CG_NYT_APPRAISE_D)
	    {
		    cardgame_attack_menu(ch);
		    return;
	    }
	  }
	  cardgame_attack_menu(ch);
	  //Signal it's ready to go
	  ch->pcdata->cg_timer = 1;
	  return;
	case 'z':
	case 'Z':
	  cardgame_surrender(ch,TRUE);
	  return;
	}
      break;
    case CG_YT_A_PM:
      switch (argument[0])
	{
        case '1':
	  if (ch->pcdata->cg_main_mob == NULL)
	    {
	      send_to_char("INVALID CHOICE!\n\r",ch);
	      if (ch->pcdata->cg_main_mob)
		{
		  send_to_char("[1] Main Mob\n\r",ch);
		  appraise_new(ch,ch->pcdata->cg_main_mob,1,ch);
		}
	      if (ch->pcdata->cg_secondary_mob)
		{
		  send_to_char("[2] Secondary Mob\n\r",ch);
		  appraise_new(ch,ch->pcdata->cg_secondary_mob,2,ch);
		}
	      send_to_char("[0] Previous Menu\n\r",ch);
	      send_to_char("Attack with which mob?\n\r",ch);
	      return;
	    }
	  ch->pcdata->cg_ticks += 1; //Use the main mob to attack
	  ch->pcdata->cg_state = CG_YT_A_PV;
	  cardgame_opponents_display(ch);
	  send_to_char("Use your mobs to attack whom?\n\r",ch);
	  return;
	case '2':
	  if (ch->pcdata->cg_secondary_mob == NULL)
	    {
	      send_to_char("INVALID CHOICE!\n\r",ch);
	      if (ch->pcdata->cg_main_mob)
		{
		  send_to_char("[1] Main Mob\n\r",ch);
		  appraise_new(ch,ch->pcdata->cg_main_mob,1,ch);
		}
	      if (ch->pcdata->cg_secondary_mob)
		{
		  send_to_char("[2] Secondary Mob\n\r",ch);
		  appraise_new(ch,ch->pcdata->cg_secondary_mob,2,ch);
		}
	      send_to_char("[0] Previous Menu\n\r",ch);
	      send_to_char("Attack with which mob?\n\r",ch);
	      return;
	    }
	  ch->pcdata->cg_ticks += 2; //Use the second mob to attack
	  ch->pcdata->cg_state = CG_YT_A_PV;
	  cardgame_opponents_display(ch);
	  send_to_char("Use your mobs to attack whom?\n\r",ch);
	  return;
	  //	break;
	case '0':
	  ch->pcdata->cg_state = CG_YT_MAIN;
	  cardgame_attack_menu(ch);
	  //DISPLAY THE YOUR TURN MENU HERE
	  return;
	  //	break;
	default:
	  send_to_char("INVALID CHOICE!\n\r",ch);
	  if (ch->pcdata->cg_main_mob)
	    {
	      send_to_char("[1] Main Mob\n\r",ch);
	      appraise_new(ch,ch->pcdata->cg_main_mob,1,ch);
	    }
	  if (ch->pcdata->cg_secondary_mob)
	    {
	      send_to_char("[2] Secondary Mob\n\r",ch);
	      appraise_new(ch,ch->pcdata->cg_secondary_mob,2,ch);
	    }
	  send_to_char("[0] Previous Menu\n\r",ch);
	  send_to_char("Attack with which mob?\n\r",ch);
	  return;
	}
      break;
    case CG_YT_A_PV:
   //   int victimno=0;
    //  bool legitvictim = FALSE;
      victimno = atoi(argument);
      //if (num == 0)
      if (victimno == 0)
        {
          ch->pcdata->cg_state = CG_YT_A_PM;
	  //reset the past mob choice
	  if (get_mobnumber(ch->pcdata->cg_ticks) == 2)
	    ch->pcdata->cg_ticks -= 2;
	  else ch->pcdata->cg_ticks -= 1;
	  if (ch->pcdata->cg_main_mob)
	    {
	      send_to_char("[1] Main Mob\n\r",ch);
	      appraise_new(ch,ch->pcdata->cg_main_mob,1,ch);
	    }
	  if (ch->pcdata->cg_secondary_mob)
	    {
	      send_to_char("[2] Secondary Mob\n\r",ch);
	      appraise_new(ch,ch->pcdata->cg_secondary_mob,2,ch);
	    }
	  send_to_char("[0] Previous Menu\n\r",ch);
	  send_to_char("Attack with which mob?\n\r",ch);
	  return;
        }
      victim = NULL;
      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
      {
        if (rch->pcdata->cg_player_number == victimno)
          victim = rch;
      }
      if (victim == NULL)
	{
	  send_to_char("INVALID VICTIM NUMBER.\n\r",ch);
	  cardgame_opponents_display(ch);
          send_to_char("Use your mobs to attack whom?\n\r",ch);
          return;
	}
      ch->pcdata->cg_ticks += (victimno*5);  //A hack to store the victim number AND the mob to attack
                                              //in the same variable
      ch->pcdata->cg_timer = current_time; //They attacked, so they get another 30 seconds
      if (get_mobnumber(ch->pcdata->cg_ticks) == 2)
        act(" $n is using $s secondary mob $p to attack $N. ",ch,ch->pcdata->cg_secondary_mob,victim,TO_ROOM);
      else  act(" $n is using $s primary mob $p to attack $N. ",ch,ch->pcdata->cg_main_mob,victim,TO_ROOM); 
      
      act(" You announce your intent to attack $N with $p. ",ch,(get_mobnumber(ch->pcdata->cg_ticks) == 2)?ch->pcdata->cg_secondary_mob:ch->pcdata->cg_main_mob,victim,TO_CHAR);
      //Flag the victim as having been attacked
      victim->pcdata->cg_ticks += CARDGAME_BEEN_ATTACKED;
      if (victim->pcdata->cg_state == CG_NYT_DONE)
        victim->pcdata->cg_state = CG_NYT_MAIN;
      if (victim->pcdata->cg_state == CG_NYT_APPRAISE_D)
        victim->pcdata->cg_state = CG_NYT_APPRAISE;
	   
      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
      {
        if (rch->pcdata->cg_state == CG_NYT_MAIN)
          cardgame_default_menu(rch);
      }
					      
      ch->pcdata->cg_state = CG_YT_MAIN;
      cardgame_attack_menu(ch);
      return;
    case CG_YT_PA_PLAYCARD:
      card=NULL;
      num = atoi(argument);
      if (num == 0)
        {
          ch->pcdata->cg_state = CG_YT_MAIN;
          cardgame_attack_menu(ch);
          return;
        }
      if (num > 0 && num < 10)
	{
	  card = get_obj_pos(get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),num);
	}
      if (card == NULL || (card->value[1] != CARD_MOB && card->value[1] != CARD_TERRAIN) 
	  || (card->value[1] == CARD_MOB 
	      && ch->pcdata->cg_secondary_mob != NULL && ch->pcdata->cg_main_mob != NULL))
	{
	  send_to_char("INVALID CHOICE!\n\r",ch);
	  //Put it back in their hand
	  if (card != NULL)
	    obj_to_obj_pos(card,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),-1);
	  cardgame_hand_display(ch);
	  send_to_char("[0] Previous Menu\n\r",ch);
	  send_to_char("Which card do you want to play?\n\r",ch);
	  return;
	}
      if (card->value[1] == CARD_MOB)
	{
	  if (ch->pcdata->cg_mobs_played >= CARDGAME_MAX_MOBS_PLAYABLE)
	  {
	    send_to_char("You have played too many mobs for this game already.\n\r",ch);
	    obj_to_obj_pos(card,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),-1);
	    cardgame_hand_display(ch);
	    send_to_char("[0] Previous Menu\n\r",ch);
	    send_to_char("Which card do you want to play?\n\r",ch);
	    return;
	  }
	  if (!pay_cost(ch,card->cost))
	  {
  	    send_to_char ("You cannot pay that card's cost, so you cannot play it.\n\r",ch);
	    obj_to_obj_pos(card,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),-1);
	    cardgame_hand_display(ch);
	    send_to_char("[0] Previous Menu\n\r",ch);
	    send_to_char("Which card do you want to play?\n\r",ch);
	    return;
	  }
	  if (ch->pcdata->cg_main_mob == NULL)
	    {
  	      sprintf(buf,"%d",card->cost);
	      act(" $n plays $p as $s Main Mob. `a($T silver)`` ",ch,card,buf,TO_ROOM);
	      act(" You play $p as your Main Mob. `a($T silver)`` ",ch,card,buf,TO_CHAR);
	      appraise_all(ch,card);
	      ch->pcdata->cg_main_mob = card;
	      ch->pcdata->cg_mm_value[CARDGAME_MOB_HP] = card->value[VALUE_MOB_HP];
	      ch->pcdata->cg_mm_value[CARDGAME_MOB_OFF] = card->value[VALUE_MOB_OFFENCE];
	      ch->pcdata->cg_mm_value[CARDGAME_MOB_DEF] = card->value[VALUE_MOB_DEFENCE];
	      discard_card(ch,card);
	      //All main mobs have 10 hp
	      card->value[7] = 10;
	      //send_to_char("And with that, your turn ends.\n\r",ch);
	      act(" And with that, your turn ends, and $N's turn begins. ",
		  ch,NULL,ch->pcdata->cg_next,TO_CHAR);
	      act(" And with that, $n's turn ends, and $N's turn begins. ",
		  ch,NULL,ch->pcdata->cg_next,TO_NOTVICT);
	      act(" And with that, $n's turn ends, and your turn begins. ",
                  ch,NULL,ch->pcdata->cg_next,TO_VICT);
	      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
	      {
	        rch->pcdata->cg_ticks = 0;
	        rch->pcdata->cg_lcp = NULL;
	      }
	      ch->pcdata->cg_state = CG_NYT_MAIN;
	      ch->pcdata->cg_timer = 0;
	      ch->pcdata->cg_ticks = 0;
	      ch->pcdata->cg_lcp = NULL;
	      ch->pcdata->cg_next->pcdata->cg_state = CG_YT_MAIN;
	      ch->pcdata->cg_next->pcdata->cg_timer = current_time;
	      if (count_contents(get_obj_vnum_string_char(ch->pcdata->cg_next,OBJ_VNUM_BINDER,"zhand")) < 7)
	      {
		obj_to_obj_pos(draw_card(ch->pcdata->cg_next),get_obj_vnum_string_char(ch->pcdata->cg_next,OBJ_VNUM_BINDER,"zhand"),-1);
		act(" $n draws a card. ",ch->pcdata->cg_next,NULL,NULL,TO_ROOM);
		//send_to_char("You draw a card.\n\r",ch->pcdata->cg_next);
	      }
	      cardgame_default_menu(ch);
	      cardgame_attack_menu(ch->pcdata->cg_next);
	      return;
	    }
	  else if (ch->pcdata->cg_secondary_mob == NULL)
	    {
 	      sprintf(buf,"%d",card->cost);
	      act(" $n plays $p as $s Secondary Mob. `a($T silver)`` ",ch,card,buf,TO_ROOM);
              act(" You play $p as your Secondary Mob. `a($T silver)`` ",ch,card,buf,TO_CHAR);
	      appraise_all(ch,card);
              ch->pcdata->cg_secondary_mob = card;
	      ch->pcdata->cg_sm_value[CARDGAME_MOB_HP] = card->value[VALUE_MOB_HP];
	      ch->pcdata->cg_sm_value[CARDGAME_MOB_OFF] = card->value[VALUE_MOB_OFFENCE];
	      ch->pcdata->cg_sm_value[CARDGAME_MOB_DEF] = card->value[VALUE_MOB_DEFENCE];
	      ++ch->pcdata->cg_mobs_played;
	      discard_card(ch,card);
	      //send_to_char("And with that, your turn ends.\n\r",ch);
              act(" And with that, your turn ends, and $N's turn begins. ",
                  ch,NULL,ch->pcdata->cg_next,TO_CHAR);
              act(" And with that, $n's turn ends, and $N's turn begins. ",
                  ch,NULL,ch->pcdata->cg_next,TO_NOTVICT);
              act(" And with that, $n's turn ends, and your turn begins. ",
                  ch,NULL,ch->pcdata->cg_next,TO_VICT);
	      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
	      {
	        rch->pcdata->cg_ticks = 0;
	        rch->pcdata->cg_lcp = NULL;
	      }
              ch->pcdata->cg_state = CG_NYT_MAIN;
	      ch->pcdata->cg_timer = 0;
	      ch->pcdata->cg_ticks = 0;
	      ch->pcdata->cg_lcp = NULL;
              ch->pcdata->cg_next->pcdata->cg_state = CG_YT_MAIN;
	      ch->pcdata->cg_next->pcdata->cg_timer = current_time;
	      if (count_contents(get_obj_vnum_string_char(ch->pcdata->cg_next,OBJ_VNUM_BINDER,"zhand")) < 7)
	      {
                obj_to_obj_pos(draw_card(ch->pcdata->cg_next),get_obj_vnum_string_char(ch->pcdata->cg_next,OBJ_VNUM_BINDER,"zhand"),-1);
	        act(" $n draws a card. ",ch->pcdata->cg_next,NULL,NULL,TO_ROOM);
		//send_to_char("You draw a card.\n\r",ch->pcdata->cg_next);
	      }
	       cardgame_default_menu(ch);
	       cardgame_attack_menu(ch->pcdata->cg_next);
	      return;
            }
	}
      else if (card->value[1] == CARD_TERRAIN)
	{
  	  OBJ_DATA *old_card;
	  if (!pay_cost(ch,card->cost))
	  {
		  send_to_char ("You cannot pay that card's cost, so you cannot play it.\n\r",ch);
		  obj_to_obj_pos(card,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),-1);
		  cardgame_hand_display(ch);
		  send_to_char("[0] Previous Menu\n\r",ch);
		  send_to_char("Which card do you want to play?\n\r",ch);
		  return;
	  }
	  //STOP THIS FROM OCCURRING IF THIS IS STILL THE FIRST ROUND
	  sprintf(buf,"%d",card->cost);
	  act(" $n plays $p as the new terrain. `a($T silver)`` ",ch,card,buf,TO_ROOM);
	  act(" You play $p as the new terrain. `a($T silver)`` ",ch,card,buf,TO_CHAR);
	  appraise_all(ch,card);
	  old_card = ch->pcdata->cg_terrain;
	  ch->pcdata->cg_terrain = card;
	  terrainify_mobs(ch,old_card,card,ch->pcdata->cg_main_mob,ch->pcdata->cg_secondary_mob);
	  for (rch = ch->pcdata->cg_next;rch != ch; rch = rch->pcdata->cg_next)
	  {
	    rch->pcdata->cg_terrain = card;
	    //Setup the variables that allow the cleaner, shorter code
	    terrainify_mobs(rch,old_card,card,rch->pcdata->cg_main_mob, rch->pcdata->cg_secondary_mob);
	  }
		
	  discard_card(ch,card);
	  ch->pcdata->cg_ticks += CARDGAME_PLAYED_TERRAIN;
	  ch->pcdata->cg_state = CG_YT_MAIN;
	  cardgame_attack_menu(ch);
	  return;
	}
      else 
	{
	  send_to_char("INVALID CHOICE.\n\r",ch);
	  obj_to_obj(card,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"));
	  cardgame_hand_display(ch);
	  send_to_char("[0] Previous Menu\n\r",ch);
          send_to_char("Which card do you want to play?\n\r",ch);
          return;
	}
      break;
    case CG_YT_AA_PLAYCARD:
//      OBJ_DATA *card=NULL;
 //     int num = atoi(argument);
      card = NULL;
      num = atoi(argument);
      if (num == 0)
	{
	  ch->pcdata->cg_state = CG_YT_MAIN;
	  cardgame_attack_menu(ch);
          return;
	}
      if (num > 0 && num < 10)
        {
          card = get_obj_pos(get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),num);
        }
      if (card == NULL || card->value[1] == CARD_MOB || card->value[1] == CARD_TERRAIN)
        {
          send_to_char("INVALID CHOICE!\n\r",ch);
	  //Put it back into their hand
	  if (card != NULL)
	    obj_to_obj_pos(card,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),-1);
          cardgame_hand_display(ch);
	  send_to_char("[0] Previous Menu\n\r",ch);
          send_to_char("Which card do you want to play?\n\r",ch);
          return;
        }
      if (!pay_cost(ch,card->cost))
      {
	      send_to_char ("You cannot pay that card's cost, so you cannot play it.\n\r",ch);
	      obj_to_obj_pos(card,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),-1);
	      cardgame_hand_display(ch);
	      send_to_char("[0] Previous Menu\n\r",ch);
	      send_to_char("Which card do you want to play?\n\r",ch);
	      return;
      }
      
      //If there are some terrain toggles
      if (ch->pcdata->cg_terrain->value[12] != 0)
      {
        bool noplay = FALSE;
	switch (card->value[1])
	{
	  case CARD_SPELL_BOOST:
 	    if (IS_SET(ch->pcdata->cg_terrain->value[12],CARD_TT_NOBOOST))
	      noplay = TRUE;
	    break;
	  case CARD_SPELL_HEAL:
	    if (IS_SET(ch->pcdata->cg_terrain->value[12],CARD_TT_NOHEAL))
	      noplay = TRUE;
	    break;
	  case CARD_SPELL_DAMAGE:
	    if (IS_SET(ch->pcdata->cg_terrain->value[12],CARD_TT_NODAMAGE))
	      noplay = TRUE;
	    break;
	  case CARD_SPELL_COUNTER:
	    if (IS_SET(ch->pcdata->cg_terrain->value[12],CARD_TT_NOCOUNTER))
 	      noplay = TRUE;
	    break;
	}
	if (noplay)
	{
 	  send_to_char("You cannot play that spell type on this terrain.\n\r",ch);
  	  obj_to_obj_pos(card,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),-1);
	  cardgame_hand_display(ch);
	  send_to_char("[0] Previous Menu\n\r",ch);
          send_to_char("Which card do you want to play?\n\r",ch);
	  return;
	}
      }
	
      ch->pcdata->cg_lcp = card;
      discard_card(ch,card);
      if (card->value[1] != CARD_SPELL_COUNTER)
	{
	  cardgame_spelltarget_display(ch);
	  //      send_to_char("[0] Previous Menu\n\r",ch);
	  //Check if that type of spell can be played on this terrain
	  send_to_char("Who/what do you wish to target with your spell?\n\r",ch);
	  ch->pcdata->cg_state = CG_YT_AA_PT;
	  return;
	}
      //Card is a counter spell
      cardgame_spellcountertarget_display(ch);
      send_to_char("Who/what do you wish to target with your spell?\n\r",ch);
      ch->pcdata->cg_state = CG_YT_AA_PCT;
      return;

      //For the attacker who is playing a non-counter spell
    case CG_YT_AA_PT:
      //      int num = atoi(argument);
      switch(argument[0])
	{
	case '0':
	  ch->pcdata->cg_state = CG_YT_AA_PLAYCARD;
	  //return the card to their hand, they decided not to play it
	  if (ch->pcdata->cg_lcp == NULL)
	  {
	    ch->pcdata->cg_state = CG_YT_MAIN;
	    return;
	  }
	  obj_from_obj(ch->pcdata->cg_lcp);
	  obj_to_obj(ch->pcdata->cg_lcp,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"));
	  ch->pcdata->cg_lcp = NULL;
          cardgame_hand_display(ch);
	  send_to_char("[0] Previous Menu\n\r",ch);
	  send_to_char("Which card do you want to play?\n\r",ch);	  
          return;
	case '1':
	  ch->pcdata->cg_spell_target = 1;
	  ch->pcdata->cg_state = CG_YT_DONE;
	  //do the act X plays X on X
	  sprintf(buf,"%d",ch->pcdata->cg_lcp->cost);
	  act(" $n plays the card $p on the defending mob. `a($T silver)`` ",ch,ch->pcdata->cg_lcp,buf,TO_ROOM);
	  act(" You play the card $p on the defending mob. `a($T silver)`` ",ch,ch->pcdata->cg_lcp,buf,TO_CHAR);
	  appraise_all(ch,ch->pcdata->cg_lcp);

	  //Figure out who's turn it is and check if the timer should be extended
	  if (their_turn(ch))
	  {
	    //If there is less than 5 seconds in the round, then extend the round by 5 seconds
	    if (current_time - ch->pcdata->cg_timer >= 20)
  	      ch->pcdata->cg_timer += 10;
	  }
	  else
	  {
	    for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
	    {
	      if (their_turn(rch))
	      {
	        //If there is less than 5 seconds in the round, then extend the round by 5 seconds
		if (current_time - rch->pcdata->cg_timer >= 20)
                  rch->pcdata->cg_timer += 10;
	      }
	    }
	  }
		

	  
          cardgame_attack_menu(ch);
	  return;
	case '2':
	  ch->pcdata->cg_spell_target = 2;
	  ch->pcdata->cg_state = CG_YT_DONE;
	  //do the act X plays X on X
	  sprintf(buf,"%d",ch->pcdata->cg_lcp->cost);
	  act( "$n plays the card $p on the attacking mob. `a($T silver)`` ",ch,ch->pcdata->cg_lcp,buf,TO_ROOM);
	  act( "You play the card $p on the attacking mob. `a($T silver)`` ",ch,ch->pcdata->cg_lcp,buf,TO_CHAR);
	  appraise_all(ch,ch->pcdata->cg_lcp);

	  //Figure out who's turn it is and check if the timer should be extended
          if (their_turn(ch))
	    {
	      //If there is less than 5 seconds in the round, then extend the round by 5 seconds
	      if (current_time - ch->pcdata->cg_timer >= 20)
		ch->pcdata->cg_timer += 10;
	    }
          else
	    {
	      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
		{
		  if (their_turn(rch))
		    {
		      //If there is less than 5 seconds in the round, then extend the round by 5 seconds
		      if (current_time - rch->pcdata->cg_timer >= 20)
			rch->pcdata->cg_timer += 10;
		    }
		}
	    }


	  
	  cardgame_attack_menu(ch);
	  return;
	default:
	  cardgame_spelltarget_display(ch);
	  send_to_char("Who/what do you wish to target with your spell?\n\r",ch);
	  return;
	}
      break;

      //For the attacker who is playing a counter spell
    case CG_YT_AA_PCT:
      num = atoi(argument);
      //      bool canplay = FALSE;
      victim = NULL;
      if (num == 0)
	{
	  ch->pcdata->cg_state = CG_YT_AA_PLAYCARD;
	  //return the card to their hand, they decided not to play it
	  if (ch->pcdata->cg_lcp == NULL)
	  {
	    ch->pcdata->cg_state = CG_YT_MAIN;
	    return;
	  }
          obj_from_obj(ch->pcdata->cg_lcp);
          obj_to_obj(ch->pcdata->cg_lcp,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"));
	  ch->pcdata->cg_lcp = NULL;
	  cardgame_hand_display(ch);
	  send_to_char("[0] Previous Menu\n\r",ch);
	  send_to_char("What card do you wish to play?\n\r",ch);
	  return;
	}
      
//      int counter=1;
//      bool canplay = FALSE;
      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
	{
	  if (rch->pcdata->cg_lcp != NULL)
	    {
	      if (rch->pcdata->cg_player_number == num)
	      victim = rch;
	    }
	}
      if (victim == NULL)
	{
	  send_to_char("That player has not played a spell you can counter!\n\r",ch);
	      cardgame_spellcountertarget_display(ch);
	      send_to_char("Who/what do you wish to target with your spell?\n\r",ch);
	      return;
	}
      ch->pcdata->cg_spell_target = num;
      sprintf(buf," $n plays the counter spell card $p on $N. (%d silver) ",ch->pcdata->cg_lcp->cost);
      act(buf,ch,ch->pcdata->cg_lcp,victim,TO_NOTVICT);
      sprintf(buf," $n plays the counter spell card $p on you, (%d silver) ",ch->pcdata->cg_lcp->cost);
      act(buf,ch,ch->pcdata->cg_lcp,victim,TO_VICT);
      sprintf(buf," You play the counter spell card $p on $N. (%d silver) ",ch->pcdata->cg_lcp->cost);
      act(buf,ch,ch->pcdata->cg_lcp,victim,TO_CHAR);
      appraise_all(ch,ch->pcdata->cg_lcp);
      ch->pcdata->cg_state = CG_YT_DONE;
      //do the act X plays X on X
      cardgame_attack_menu(ch);
      return;

    case CG_YT_APPRAISE_D:
    case CG_YT_APPRAISE:
      if (argument[0] == '\0' || !is_number(argument) || !appraise_card_number(ch,argument))
	{
	  send_to_char("Invalid number!\n\r",ch);
	  appraise_list(ch);
	  send_to_char("Appraise which card number?\n\r",ch);
	  return;
	}
      if (ch->pcdata->cg_state == CG_YT_APPRAISE_D)
        ch->pcdata->cg_state = CG_YT_DONE;
      else ch->pcdata->cg_state = CG_YT_MAIN;
      cardgame_default_menu(ch);
      return;
    case CG_YT_DISCARD_D:
    case CG_YT_DISCARD:
      card=NULL;
      num = atoi(argument);
      if (num == 0)
        {
          ch->pcdata->cg_state = CG_YT_MAIN;
          cardgame_attack_menu(ch);
          return;
        }
      if (num > 0 && num < 10)
        {
          card = get_obj_pos(get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),num);
        }
      if (card == NULL)
        {
          send_to_char("INVALID CHOICE!\n\r",ch);
          //Put it back in their hand
          //obj_to_obj_pos(card,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),-1);
          cardgame_hand_display(ch);
          send_to_char("[0] Previous Menu\n\r",ch);
          send_to_char("Which card do you want to play?\n\r",ch);
          return;
        }
      act("You discard $p.",ch,card,NULL,TO_CHAR);
      act("$n discards a card.",ch,NULL,NULL,TO_ROOM);
      discard_card(ch,card);
      if (ch->pcdata->cg_state == CG_YT_DISCARD_D)
	ch->pcdata->cg_state = CG_YT_DONE;
      else ch->pcdata->cg_state = CG_YT_MAIN;



    case CG_YT_DONE:
      switch (argument[0])
      {
        default :
          cardgame_attack_menu(ch);
          return;
	//Scan the table
        case 'c':
        case 'C':
          table_scan(ch);
          return;                                   
	case 'd':
        case 'D':
          ch->pcdata->cg_state = CG_YT_APPRAISE_D;
          appraise_list(ch);
          send_to_char("Appraise which card number?\n\r",ch);
        return;						  
	case 'e':
	case 'E':
	  mobs_scan(ch);
	  return;
	case 'z':
	case 'Z':
	  cardgame_surrender(ch,TRUE);
	  return;
      }								  
      break;
    case CG_NYT_MAIN:
      switch (argument[0])
      {
        default:
	  cardgame_default_menu(ch);
   	  return;
        //Play a card
        case 'a':
	case 'A':
	  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
	    {
	      if (rch->pcdata->cg_state <= CG_YT_DONE)
		break;
	    }
	  if ((rch->pcdata->cg_ticks & (CARDGAME_PLAYED_TERRAIN-1)) <= 0)
	    {
	      send_to_char("The person whose turn it is has not attacked yet, so you may not play a card.\n\r",ch);
	      cardgame_default_menu(ch);
	      return;
	    }
	  cardgame_hand_display(ch);
	  send_to_char("[0] Previous Menu\n\r",ch);
	  ch->pcdata->cg_state = CG_NYT_PLAYCARD;
	  send_to_char("What card do you want to play?\n\r",ch);
	  return;

	//Scan the table
   	case 'c':
	case 'C':
	  table_scan(ch);
	  return;

	case 'd':
        case 'D':
          ch->pcdata->cg_state = CG_NYT_APPRAISE;
          appraise_list(ch);
          send_to_char("Appraise which card number?\n\r",ch);
          return;
	case 'e':
	case 'E':
	  mobs_scan(ch);
	  return;					  	     
	//Choose a mob to defend the attack with
	case 'F':
	case 'f':
	  if (!IS_SET(ch->pcdata->cg_ticks,CARDGAME_BEEN_ATTACKED))
	  {
	    send_to_char("You cannot defend if you've not been attacked.\n\r",ch);
	    cardgame_default_menu(ch);
	    return;
	  }
	  if (get_mobnumber(ch->pcdata->cg_ticks) != 0)
	  {
	    send_to_char("You've already chosen a mob to defend with.\n\r",ch);
	    cardgame_default_menu(ch);
	    return;
	  }
          if (ch->pcdata->cg_main_mob)
            {
              send_to_char("[1] Main Mob\n\r",ch);
              appraise_new(ch,ch->pcdata->cg_main_mob,1,ch);
            }
          if (ch->pcdata->cg_secondary_mob)
            {
              send_to_char("[2] Secondary Mob\n\r",ch);
	      appraise_new(ch,ch->pcdata->cg_secondary_mob,2,ch);
            }
	  ch->pcdata->cg_state = CG_NYT_D_PM;
          send_to_char("[0] Previous Menu\n\r",ch);
          send_to_char("Defend with which mob?\n\r",ch);
          break;

      case 'y':
      case 'Y':
	ch->pcdata->cg_state = CG_NYT_DONE;
	send_to_char(" You declare yourself ready. \n\r",ch);
	act(" $n declares $e is finished for the round. ",ch,NULL,NULL,TO_ROOM);
	victim = NULL;
	for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
	{
	  
	  if (rch->pcdata->cg_state != CG_YT_DONE && rch->pcdata->cg_state != CG_NYT_DONE
	    && rch->pcdata->cg_state != CG_YT_APPRAISE_D && rch->pcdata->cg_state != CG_NYT_APPRAISE_D)
	  {
	    cardgame_default_menu(ch);
	    return;
	  }
	  if (their_turn(rch))
 	    victim = rch;
	}
	//Set the person whose turn it is's timer to 0 to signal everyone is ready
	if (victim)
	  victim->pcdata->cg_timer = 1;
	cardgame_default_menu(ch);
	return;

	case 'z':
        case 'Z':
          cardgame_surrender(ch,TRUE);
          return;
			      
	  
      }
      break;
    case CG_NYT_PLAYCARD:
      card=NULL;
      num = atoi(argument);
      if (num == 0)
        {
          ch->pcdata->cg_state = CG_NYT_MAIN;
          cardgame_default_menu(ch);
          return;
        }
      if (num > 0 && num < 10)
        {
          card = get_obj_pos(get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),num);
        }
      if (card == NULL || card->value[1] == CARD_MOB || card->value[1] == CARD_TERRAIN)
        {
          send_to_char("INVALID CHOICE!\n\r",ch);
	  if (card != NULL)
	    obj_to_obj_pos(card,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),-1);
          cardgame_hand_display(ch);
          send_to_char("[0] Previous Menu\n\r",ch);
          send_to_char("Which card do you want to play?\n\r",ch);
          return;
        }
      if (!pay_cost(ch,card->cost))
      {
	      send_to_char ("You cannot pay that card's cost, so you cannot play it.\n\r",ch);
	      obj_to_obj_pos(card,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),-1);
	      cardgame_hand_display(ch);
	      send_to_char("[0] Previous Menu\n\r",ch);
	      send_to_char("Which card do you want to play?\n\r",ch);
	      return;
      }
      //If there are some terrain toggles
      if (ch->pcdata->cg_terrain->value[12] != 0)
	{
	  bool noplay = FALSE;
	  switch (card->value[1])
	    {
	    case CARD_SPELL_BOOST:
	      if (IS_SET(ch->pcdata->cg_terrain->value[12],CARD_TT_NOBOOST))
		noplay = TRUE;
	      break;
	    case CARD_SPELL_HEAL:
	      if (IS_SET(ch->pcdata->cg_terrain->value[12],CARD_TT_NOHEAL))
		noplay = TRUE;
	      break;
	    case CARD_SPELL_DAMAGE:
	      if (IS_SET(ch->pcdata->cg_terrain->value[12],CARD_TT_NODAMAGE))
		noplay = TRUE;
	      break;
	    case CARD_SPELL_COUNTER:
	      if (IS_SET(ch->pcdata->cg_terrain->value[12],CARD_TT_NOCOUNTER))
		noplay = TRUE;
	      break;
	    }
	  if (noplay)
	    {
	      send_to_char("You cannot play that spell type on this terrain.\n\r",ch);
	      obj_to_obj_pos(card,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),-1);
	      cardgame_hand_display(ch);
	      send_to_char("[0] Previous Menu\n\r",ch);
	      send_to_char("Which card do you want to play?\n\r",ch);
	      return;
	    }
	}

      ch->pcdata->cg_lcp = card;
      discard_card(ch,card);
      if (card->value[1] != CARD_SPELL_COUNTER)
        {
          cardgame_spelltarget_display(ch);
          //      send_to_char("[0] Previous Menu\n\r",ch);
          //Check if that type of spell can be played on this terrain
          send_to_char("Who/what do you wish to target with your spell?\n\r",ch);
          ch->pcdata->cg_state = CG_NYT_PT;
          return;
        }
      //Card is a counter spell
      cardgame_spellcountertarget_display(ch);
      send_to_char("Who/what do you wish to target with your spell?\n\r",ch);
      ch->pcdata->cg_state = CG_NYT_PCT;
      return;
    case CG_NYT_PT:
      switch(argument[0])
        {
        case '0':
          ch->pcdata->cg_state = CG_NYT_PLAYCARD;
          //return the card to their hand, they decided not to play it
	  if (ch->pcdata->cg_lcp == NULL)
	  {
            ch->pcdata->cg_state = CG_NYT_MAIN;
            return;
          }
          obj_from_obj(ch->pcdata->cg_lcp);
          obj_to_obj(ch->pcdata->cg_lcp,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"));
          ch->pcdata->cg_lcp = NULL;
          cardgame_hand_display(ch);
	  send_to_char("[0] Previous Menu\n\r",ch);
	  send_to_char("Which card do you want to play?\n\r",ch);
          return;
        case '1':
          ch->pcdata->cg_spell_target = 1;
          ch->pcdata->cg_state = CG_NYT_MAIN;
          //do the act X plays X on X
	  sprintf(buf,"%d",ch->pcdata->cg_lcp->cost);
          act(" $n plays the card $p on the defending mob. `a($T silver)`` ",ch,ch->pcdata->cg_lcp,buf,TO_ROOM);
	  act(" You play the card $p on the defending mob. `a($T silver)`` ",ch,ch->pcdata->cg_lcp,buf,TO_CHAR);
	  appraise_all(ch,ch->pcdata->cg_lcp);

	  //Figure out who's turn it is and check if the timer should be extended
          if (their_turn(ch))
	    {
	      //If there is less than 5 seconds in the round, then extend the round by 5 seconds
	      if (current_time - ch->pcdata->cg_timer >= 20)
		ch->pcdata->cg_timer += 10;
	    }
          else
	    {
	      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
		{
		  if (their_turn(rch))
		    {
		      //If there is less than 5 seconds in the round, then extend the round by 5 seconds
		      if (current_time - rch->pcdata->cg_timer >= 20)
			rch->pcdata->cg_timer += 10;
		    }
		}
	    }


          cardgame_default_menu(ch);
	  //	  send_to_char("[0] Previous Menu\n\r",ch);
          return;
        case '2':
          ch->pcdata->cg_spell_target = 2;
          ch->pcdata->cg_state = CG_NYT_MAIN;
          //do the act X plays X on X
	  sprintf(buf,"%d",ch->pcdata->cg_lcp->cost);
          act(" $n plays the card $p on the attacking mob. `a($T silver)`` ",ch,ch->pcdata->cg_lcp,buf,TO_ROOM);
	  act(" You play the card $p on the attacking mob. `a($T silver)`` ",ch,ch->pcdata->cg_lcp,buf,TO_CHAR);
	  appraise_all(ch,ch->pcdata->cg_lcp);

	  //Figure out who's turn it is and check if the timer should be extended
          if (their_turn(ch))
	    {
	      //If there is less than 5 seconds in the round, then extend the round by 5 seconds
	      if (current_time - ch->pcdata->cg_timer >= 20)
		ch->pcdata->cg_timer += 10;
	    }
          else
	    {
	      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
		{
		  if (their_turn(rch))
		    {
		      //If there is less than 5 seconds in the round, then extend the round by 5 seconds
		      if (current_time - rch->pcdata->cg_timer >= 20)
			rch->pcdata->cg_timer += 10;
		    }
		}
	    }


          cardgame_default_menu(ch);
	  //	  send_to_char("[0] Previous Menu\n\r",ch);
          return;
        default:
          cardgame_spelltarget_display(ch);
          send_to_char("Who/what do you wish to target with your spell?\n\r",ch);
          return;
        }
      break;
    case CG_NYT_PCT:
      num = atoi(argument);
      //      bool canplay = FALSE;
      victim = NULL;
      if (num == 0)
        {
          ch->pcdata->cg_state = CG_NYT_PLAYCARD;
          //return the card to their hand, they decided not to play it
	  if (ch->pcdata->cg_lcp == NULL)
	  {
	    ch->pcdata->cg_state = CG_NYT_MAIN;
	    return;
	  }
          obj_from_obj(ch->pcdata->cg_lcp);
          obj_to_obj(ch->pcdata->cg_lcp,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"));
          ch->pcdata->cg_lcp = NULL;
          cardgame_hand_display(ch);
	  send_to_char("[0] Previous Menu\n\r",ch);
          send_to_char("What card do you wish to play?\n\r",ch);
          return;
        }

      //      int counter=1;
      //      bool canplay = FALSE;
      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
        {
          if (rch->pcdata->cg_lcp != NULL)
            {
              if (rch->pcdata->cg_player_number == num)
		victim = rch;
            }
        }
      if (victim == NULL)
        {
          send_to_char("That player has not played a spell you can counter!\n\r",ch);
	  cardgame_spellcountertarget_display(ch);
	  send_to_char("Who/what do you wish to target with your spell?\n\r",ch);
	  return;
        }
      ch->pcdata->cg_spell_target = num;
      sprintf(buf," $n plays the counter spell card $p on $N. (%d silver) ",ch->pcdata->cg_lcp->cost);
      act(buf,ch,ch->pcdata->cg_lcp,victim,TO_NOTVICT);
      sprintf(buf," $n plays the counter spell card $p on you. (%d silver) ",ch->pcdata->cg_lcp->cost);
      act(buf,ch,ch->pcdata->cg_lcp,victim,TO_VICT);
      sprintf(buf," You play the counter spell card $p on $N. (%d silver) ",ch->pcdata->cg_lcp->cost);
      act(buf,ch,ch->pcdata->cg_lcp,victim,TO_CHAR);
      appraise_all(ch,ch->pcdata->cg_lcp);
      ch->pcdata->cg_state = CG_NYT_MAIN;
      //do the act X plays X on X
      cardgame_default_menu(ch);
      return;
    case CG_NYT_D_PM:
      switch (argument[0])
	{
	default :
	  send_to_char("INVALID CHOICE!\n\r",ch);
          if (ch->pcdata->cg_main_mob)
            {
              send_to_char("[1] Main Mob\n\r",ch);
              appraise_new(ch,ch->pcdata->cg_main_mob,1,ch);
            }
          if (ch->pcdata->cg_secondary_mob)
            {
              send_to_char("[2] Secondary Mob\n\r",ch);
              appraise_new(ch,ch->pcdata->cg_secondary_mob,2,ch);
            }
          send_to_char("[0] Previous Menu\n\r",ch);
          send_to_char("Defend with which mob?\n\r",ch);
          return;
	case '0':
	  ch->pcdata->cg_state = CG_NYT_MAIN;
          cardgame_default_menu(ch);
	  return;
	case '1':
	  if (ch->pcdata->cg_main_mob == NULL)
            {
              send_to_char("INVALID CHOICE!\n\r",ch);
              if (ch->pcdata->cg_main_mob)
                {
                  send_to_char("[1] Main Mob\n\r",ch);
                  appraise_new(ch,ch->pcdata->cg_main_mob,1,ch);
                }
              if (ch->pcdata->cg_secondary_mob)
                {
                  send_to_char("[2] Secondary Mob\n\r",ch);
                  appraise_new(ch,ch->pcdata->cg_secondary_mob,2,ch);
                }
              send_to_char("[0] Previous Menu\n\r",ch);
              send_to_char("Defend with which mob?\n\r",ch);
              return;
            }
	  ch->pcdata->cg_ticks += 1; //Use the main mob to defend
	  act(" $n chooses $s main mob, $p, to defend the attack. ",ch,ch->pcdata->cg_main_mob,NULL,TO_ROOM);
	  act(" You announce your intent to defend the attack with $p. ",ch,ch->pcdata->cg_main_mob,NULL,TO_CHAR);
          ch->pcdata->cg_state = CG_NYT_MAIN;
          cardgame_default_menu(ch);
          return;
	case '2':
	  if (ch->pcdata->cg_secondary_mob == NULL)
            {
              send_to_char("INVALID CHOICE!\n\r",ch);
              if (ch->pcdata->cg_main_mob)
                {
                  send_to_char("[1] Main Mob\n\r",ch);
                  appraise_new(ch,ch->pcdata->cg_main_mob,1,ch);
                }
              if (ch->pcdata->cg_secondary_mob)
                {
                  send_to_char("[2] Secondary Mob\n\r",ch);
                  appraise_new(ch,ch->pcdata->cg_secondary_mob,2,ch);
                }
              send_to_char("[0] Previous Menu\n\r",ch);
              send_to_char("Defend with which mob?\n\r",ch);
              return;
            }
          ch->pcdata->cg_ticks += 2; //Use the second mob to attack
	  act(" $n chooses $s secondary mob, $p, to defend the attack. ",ch,ch->pcdata->cg_secondary_mob,NULL,TO_ROOM);
          act(" You announce your intent to defend the attack with $p. ",ch,ch->pcdata->cg_secondary_mob,NULL,TO_CHAR);
          ch->pcdata->cg_state = CG_NYT_MAIN;
          cardgame_default_menu(ch);
          return;
	}
      break;
    case CG_NYT_APPRAISE_D:
    case CG_NYT_APPRAISE:
      if (argument[0] == '\0' || !is_number(argument) || !appraise_card_number(ch,argument))
      {
        send_to_char("Invalid number!\n\r",ch);
	appraise_list(ch);
	send_to_char("Appraise which card number?\n\r",ch);
	return;
      }
      if (ch->pcdata->cg_state == CG_NYT_APPRAISE_D)
        ch->pcdata->cg_state = CG_NYT_DONE;
      else ch->pcdata->cg_state = CG_NYT_MAIN;
      cardgame_default_menu(ch);
      return;
    case CG_NYT_DONE:
      switch (argument[0])
	{
        default :
          cardgame_default_menu(ch);
          return;
        //Scan the table
        case 'c':
        case 'C':
          table_scan(ch);
          return;                                    
	case 'd':
        case 'D':
          ch->pcdata->cg_state = CG_NYT_APPRAISE_D;
          appraise_list(ch);
          send_to_char("Appraise which card number?\n\r",ch);
          return;
	case 'e':
	case 'E':
	  mobs_scan(ch);
	  return;
        case 'z':
        case 'Z':
          cardgame_surrender(ch,TRUE);
          return;
	}
      break;
    }
}

	  

int count_contents(OBJ_DATA *container)
{
  OBJ_DATA *obj;
  int counter=0;
  if (container == NULL)
    return 0;
  for (obj = container->contains;obj != NULL; obj = obj->next_content,++counter);
  return counter;
}
	 

void card_game_prompt (CHAR_DATA *ch)
{
  int handcount,deckcount;
//  OBJ_DATA *obj1,*obj2;
  char buf[MAX_STRING_LENGTH], *turnname;
  CHAR_DATA *rch;
  handcount = count_contents(get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"));
  deckcount = count_contents(get_obj_vnum_char(ch,OBJ_VNUM_DECK));
  if (ch->pcdata->cg_state >= CG_START_PM_YT && ch->pcdata->cg_state <= CG_YT_DONE)
    turnname = ch->name;
  else 
  {
    for (rch = ch->pcdata->cg_next;rch != ch && !(rch->pcdata->cg_state >= CG_START_PM_YT && rch->pcdata->cg_state <= CG_YT_DONE);rch = rch->pcdata->cg_next);
    if (rch->pcdata->cg_state >= CG_START_PM_YT && rch->pcdata->cg_state <= CG_YT_DONE)
      turnname = rch->name;
    else turnname = "NA";
  }
  sprintf(buf,"`a<`gCards: `a%d `gDeck: `a%d `gMain Mob HP: `a%d `gMobs: `a%d `gTerrain: `a%s `gTurn: `a%s`g>``", 
		  handcount, deckcount, (ch->pcdata->cg_main_mob)?ch->pcdata->cg_mm_value[CARDGAME_MOB_HP]:-1,
		  (ch->pcdata->cg_secondary_mob == NULL)?1:2, 
		  (ch->pcdata->cg_terrain == NULL)?"NA":flag_string(card_terrain_type,ch->pcdata->cg_terrain->value[7]), turnname);
  send_to_char(buf,ch);
}
		  
void update_cardgame_round()
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *rch, *ch;
  char buf[MAX_STRING_LENGTH];//,buf2[MAX_STRING_LENGTH],buf3[MAX_STRING_LENGTH];
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->connected != CON_PLAYING || !d->character->pcdata || IS_NPC(d->character))
	continue;
      if (d->character->pcdata->cg_state >= CG_YT_MAIN && d->character->pcdata->cg_state <= CG_YT_DONE && d->character->pcdata->cg_timer > 0)
	{
	  ch = d->character;
	  //Has it been 30 seconds?
//	  sprintf(buf,"Your timer -> %d\n\r",d->character->pcdata->cg_timer);
//	  send_to_char(buf,ch);
	  if ((current_time - d->character->pcdata->cg_timer) > 30)
	    {
	      //End their turn, but first check if the attack needs to go through or not
	      if (get_mobnumber(d->character->pcdata->cg_ticks) > 0 && get_victimnumber(d->character->pcdata->cg_ticks) != 0)
		{
		  OBJ_DATA *attacking_mob;
		  short *attacking_stats;
		  OBJ_DATA *defending_mob=NULL;
		  short *defending_stats=NULL;
		  OBJ_DATA *target_mob;
		  short *target_stats;
		  CHAR_DATA *target_player;
		  CHAR_DATA *attacking_player;
		  CHAR_DATA *defending_player=NULL ;
		  int am_off, am_def, dm_off, dm_def, damage;
		  int am_eb=0,am_nb=0,dm_eb=0,dm_nb=0,am_tb[2]={0,0},dm_tb[2]={0,0},am_total[2]={0,0},dm_total[2]={0,0};
		  //	  CHAR_DATA *mch;
		  
		  
		  //There is a pending attack that needs to go through
		  act(" This round is over, so the attack goes through. ",ch,NULL,NULL,TO_ALL);
		  
		  if (get_mobnumber(d->character->pcdata->cg_ticks) == 2)
		  {
		    attacking_mob = d->character->pcdata->cg_secondary_mob;
		    attacking_stats = d->character->pcdata->cg_sm_value;
		    attacking_player = d->character;
			    
		  }
		  else 
		  {
		    attacking_mob = d->character->pcdata->cg_main_mob;
		    attacking_stats = d->character->pcdata->cg_mm_value;
		    attacking_player = d->character;
		  }
		  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
		    {
		      if (rch->pcdata->cg_player_number == get_victimnumber(ch->pcdata->cg_ticks))
			{
			  if (get_mobnumber(rch->pcdata->cg_ticks) == 1 || !rch->pcdata->cg_secondary_mob)
			  {
			    defending_mob = rch->pcdata->cg_main_mob;
			    defending_stats = rch->pcdata->cg_mm_value;
			    defending_player = rch;
			  }
			  //If they did not chose a mob to defend, default to the secondary mob
			  else 
			  {
			    defending_mob = rch->pcdata->cg_secondary_mob;
			    defending_stats = rch->pcdata->cg_sm_value;
			    defending_player = rch;
			  }
			}
		    }
		  
		  //Store their current offence/defence, so they can be set back to normal at the end of turn
		  //(and so all boost card's effects will go away at the end of the turn
		  am_off = attacking_stats[CARDGAME_MOB_OFF];
		  am_def = attacking_stats[CARDGAME_MOB_DEF];
		  dm_off = defending_stats[CARDGAME_MOB_OFF];
		  dm_def = defending_stats[CARDGAME_MOB_DEF];


		  am_tb[0] = am_off - attacking_mob->value[8];
		  am_tb[1] = am_def - attacking_mob->value[9];
		  dm_tb[0] = dm_off - defending_mob->value[8];
                  dm_tb[1] = dm_def - defending_mob->value[9];
		  
		  //First, apply all burst cards
		  
		  //If the player whose turn it is played a boost, play it now
		  if (ch->pcdata->cg_lcp && ch->pcdata->cg_lcp->value[1] == CARD_SPELL_BOOST)
		    {
		      target_mob = ((ch->pcdata->cg_spell_target == 1)?defending_mob:attacking_mob);
		      target_stats = ((ch->pcdata->cg_spell_target == 1)?defending_stats:attacking_stats);
		      act(" $n plays $p on $P. ",ch,ch->pcdata->cg_lcp,target_mob,TO_ROOM);
		      act(" You play $p on $P. ",ch,ch->pcdata->cg_lcp,target_mob,TO_CHAR);
		      if (!check_counter(ch,ch->pcdata->cg_lcp))
			{
			  sprintf(buf," $p adds %d offense and %d defense to $P. ",ch->pcdata->cg_lcp->value[7],ch->pcdata->cg_lcp->value[8]);
			  act(buf,ch,ch->pcdata->cg_lcp,target_mob,TO_ALL);
			  //Add the actual bonus to the mob, at least for this round
			  target_stats[CARDGAME_MOB_OFF] += ch->pcdata->cg_lcp->value[7];
			  target_stats[CARDGAME_MOB_DEF] += ch->pcdata->cg_lcp->value[8];
			  //Nullify this card
			  ch->pcdata->cg_lcp = NULL;
			}
		    }
		  //If any of the OTHER players played a boost, play it now
		      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
			{
			  if (rch->pcdata->cg_lcp && rch->pcdata->cg_lcp->value[1] == CARD_SPELL_BOOST)
			    {
			      target_mob = ((rch->pcdata->cg_spell_target == 1)?defending_mob:attacking_mob);
			      target_stats = ((rch->pcdata->cg_spell_target == 1)?defending_stats:attacking_stats);
			      act(" $n plays $p on $P. ",rch,rch->pcdata->cg_lcp,target_mob,TO_ROOM);
			      act(" You play $p on $P. ",rch,rch->pcdata->cg_lcp,target_mob,TO_CHAR);
			      if (!check_counter(rch,rch->pcdata->cg_lcp))
				{
				  sprintf(buf," $p adds %d offense and %d defense to $P. ",rch->pcdata->cg_lcp->value[7],rch->pcdata->cg_lcp->value[8]);
				  act(buf,rch,rch->pcdata->cg_lcp,target_mob,TO_ALL);
				  //Add the actual bonus to the mob, at least for this round
				  target_stats[CARDGAME_MOB_OFF] += rch->pcdata->cg_lcp->value[7];
				  target_stats[CARDGAME_MOB_DEF] += rch->pcdata->cg_lcp->value[8];
				  //Nullify this card
				  rch->pcdata->cg_lcp = NULL;
				}
			    }
			}
		    
		  
		  //ADD IN ELEMENTAL AND NEMESIS BONUSES HERE
		  //Third, have the mobs attack each other
		  
		  /*	  //Set up intial off/defense variables
			  am_off = attacking_mob->value[8];
			  am_def = attacking_mob->value[9];
			  dm_off = defending_mob->value[8];
			  dm_def = defending_mob->value[9];*/
		  //Deal with nemeses
		  if (attacking_mob->value[12] == defending_mob->value[10])
		    {
		      attacking_stats[CARDGAME_MOB_OFF] += 2;
		      attacking_stats[CARDGAME_MOB_DEF] += 2;
		      am_nb = 2;
		    }
		  if (defending_mob->value[12] == attacking_mob->value[10])
		    {
		      defending_stats[CARDGAME_MOB_OFF] += 2;
		      defending_stats[CARDGAME_MOB_DEF] += 2;
		      dm_nb = 2;
		    }
		  
		  am_eb = attacking_stats[CARDGAME_MOB_DEF];
		  dm_eb = defending_stats[CARDGAME_MOB_DEF];
		  //Deal with elements
		  switch (attacking_mob->value[11])
		    {
		    case CARD_ELEMENT_LIGHTNING:
		      if (defending_mob->value[11] == CARD_ELEMENT_WATER)
			{
 			  ++attacking_stats[CARDGAME_MOB_OFF];
			  ++attacking_stats[CARDGAME_MOB_DEF];
			}
		      if (defending_mob->value[11] == CARD_ELEMENT_STONE)
			{
			  ++defending_stats[CARDGAME_MOB_OFF];
		          ++defending_stats[CARDGAME_MOB_DEF];
			}
		      break;
		    case CARD_ELEMENT_FIRE:
		      if (defending_mob->value[11] == CARD_ELEMENT_STONE)
			{
		 	  ++attacking_stats[CARDGAME_MOB_OFF];
                          ++attacking_stats[CARDGAME_MOB_DEF];
			}
		      if (defending_mob->value[11] == CARD_ELEMENT_WATER)
			{
			  ++defending_stats[CARDGAME_MOB_OFF];
			  ++defending_stats[CARDGAME_MOB_DEF];
			}
		      break;
		    case CARD_ELEMENT_STONE:
		      if (defending_mob->value[11] == CARD_ELEMENT_LIGHTNING)
			{
 			  ++attacking_stats[CARDGAME_MOB_OFF];
			  ++attacking_stats[CARDGAME_MOB_DEF];
			}
		      if (defending_mob->value[11] == CARD_ELEMENT_FIRE)
			{
			  ++defending_stats[CARDGAME_MOB_OFF];
			  ++defending_stats[CARDGAME_MOB_DEF];
			}
		      break;
		    case CARD_ELEMENT_WATER:
		      if (defending_mob->value[11] == CARD_ELEMENT_FIRE)
			{
			  ++attacking_stats[CARDGAME_MOB_OFF];
			  ++attacking_stats[CARDGAME_MOB_DEF];
			}
		      if (defending_mob->value[11] == CARD_ELEMENT_LIGHTNING)
			{
 			  ++defending_stats[CARDGAME_MOB_OFF];				
			  ++defending_stats[CARDGAME_MOB_DEF];
			}
		      break;
		    }
		  am_eb = attacking_stats[CARDGAME_MOB_DEF] - am_eb;
		  dm_eb = defending_stats[CARDGAME_MOB_DEF] - dm_eb;
		  am_total[0] = attacking_stats[CARDGAME_MOB_OFF];// - am_off;
		  am_total[1] = attacking_stats[CARDGAME_MOB_DEF];// - am_def;
		  dm_total[0] = defending_stats[CARDGAME_MOB_OFF];// - dm_off;
                  dm_total[1] = defending_stats[CARDGAME_MOB_DEF];// - dm_def;
		 
		  //add + to front if positive
		  sprintf(buf, "$n's $p has the bonuses E: %s%d%s%d N:%s%d%s%d T:%s%d%s%d, in total: %d/%d",
				  (am_eb >= 0)?"+":"",am_eb,(am_eb >= 0)?"+":"",am_eb,
				  (am_nb >= 0)?"+":"",am_nb,(am_nb >= 0)?"+":"",am_nb,
				  (am_tb[0] >= 0)?"+":"",am_tb[0],(am_tb[1] >= 0)?"+":"",am_tb[1],
				  am_total[0],am_total[1]);
		  act(buf,ch,attacking_mob,NULL,TO_ALL);
		  sprintf(buf, "$n's $p has the bonuses E: %s%d%s%d N:%s%d%s%d T:%s%d%s%d, in total: %d/%d",
		                  (dm_eb >= 0)?"+":"",dm_eb,(dm_eb >= 0)?"+":"",dm_eb,
		                  (dm_nb >= 0)?"+":"",dm_nb,(dm_nb >= 0)?"+":"",dm_nb,
		                  (dm_tb[0] >= 0)?"+":"",dm_tb[0],(dm_tb[1] >= 0)?"+":"",dm_tb[1],
		                  dm_total[0],dm_total[1]);
		   act(buf,defending_player,defending_mob,NULL,TO_ALL);
				  
		  
		  damage = attacking_stats[CARDGAME_MOB_OFF] - defending_stats[CARDGAME_MOB_DEF]; //offence - defence = damage
		  if (damage < 1)
		    damage = 1;
		  sprintf(buf," $p deals %d damage to $P. ",damage);
		  //sprintf(buf," $p (deals %d damage to $P. ",damage);
		  defending_stats[CARDGAME_MOB_HP] -= damage;
		  act(buf,ch,attacking_mob,defending_mob,TO_ALL);
		  damage = defending_stats[CARDGAME_MOB_OFF] - attacking_stats[CARDGAME_MOB_DEF]; //offence - defence = damage
		  if (damage < 0)
		    damage = 0;
		  sprintf(buf," $p deals %d damage to $P. ",damage);
		  act(buf,ch,defending_mob,attacking_mob,TO_ALL);
		  attacking_stats[CARDGAME_MOB_HP] -= damage;
		  
		  //Fourth, play damage cards and heal cards
		  
		  if (ch->pcdata->cg_lcp &&
		      (ch->pcdata->cg_lcp->value[1] == CARD_SPELL_HEAL
		       || ch->pcdata->cg_lcp->value[1] == CARD_SPELL_DAMAGE))
		    {
		      target_mob = ((ch->pcdata->cg_spell_target == 1)?defending_mob:attacking_mob);
		      target_stats = ((ch->pcdata->cg_spell_target == 1)?defending_stats:attacking_stats);
		      target_player = ((ch->pcdata->cg_spell_target == 1)?defending_player:attacking_player);
		      act(" $n plays $p on $P. ",ch,ch->pcdata->cg_lcp,target_mob,TO_ROOM);
		      act(" You play $p on $P. ",ch,ch->pcdata->cg_lcp,target_mob,TO_CHAR);
		      if (!check_counter(ch,ch->pcdata->cg_lcp))
			{
			  if (ch->pcdata->cg_lcp->value[1] == CARD_SPELL_HEAL)
			    {
			      int heal=rch->pcdata->cg_lcp->value[7];

			      if (target_player->pcdata->cg_secondary_mob == target_mob)
				{
				  //This prevents them from healing a mob over it's max HP (if it's not a main mob)
				  if (heal+target_stats[CARDGAME_MOB_HP] > target_mob->value[VALUE_MOB_HP])
				    heal = target_mob->value[VALUE_MOB_HP] - target_stats[CARDGAME_MOB_HP];
				}
			      else //main mob
				{
				  //This prevents them from healing a mob over the max main mob hp (10)
				  if (heal+target_stats[CARDGAME_MOB_HP] > 10)
				    heal = 10 - target_stats[CARDGAME_MOB_HP];
				}

			      sprintf(buf," $p heals $P by %d. ",heal);
			      act(buf,ch,ch->pcdata->cg_lcp,target_mob,TO_ALL);
			      //Add the hp to the mob
			      target_stats[CARDGAME_MOB_HP] += heal;
			      //Nullify this card
			      ch->pcdata->cg_lcp = NULL;
			    }
			  else //Damage card
			    {
			      damage = ch->pcdata->cg_lcp->value[7];
			      if (ch->pcdata->cg_lcp->value[7] != CARD_ELEMENT_NONE)
				{
				  switch (ch->pcdata->cg_lcp->value[8])
				    {
				    case CARD_ELEMENT_LIGHTNING:
				      if (target_mob->value[11] == CARD_ELEMENT_WATER)
					damage++;
				      if (target_mob->value[11] == CARD_ELEMENT_STONE)
					damage--;
				      break;
				    case CARD_ELEMENT_STONE:
				      if (target_mob->value[11] == CARD_ELEMENT_LIGHTNING)
					damage++;
				      if (target_mob->value[11] == CARD_ELEMENT_FIRE)
					damage--;
				      break;
				    case CARD_ELEMENT_FIRE:
				      if (target_mob->value[11] == CARD_ELEMENT_STONE)
					damage++;
				      if (target_mob->value[11] == CARD_ELEMENT_WATER)
					damage--;
				      break;
				    case CARD_ELEMENT_WATER:
				      if (target_mob->value[11] == CARD_ELEMENT_FIRE)
					damage++;
				      if (target_mob->value[11] == CARD_ELEMENT_LIGHTNING)
					damage--;
				      break;
				    }
				}
			      
			      //Deal with Terrain toggles
			      if (ch->pcdata->cg_lcp->value[8] != CARD_ELEMENT_NONE)
			      {
 			        if (IS_SET(ch->pcdata->cg_terrain->value[12],CARD_TT_ELEMENTALPLUS))
				  damage += 2;
				if (IS_SET(ch->pcdata->cg_terrain->value[12],CARD_TT_ELEMENTALMINUS))
				  damage -= 2;
			      }
			      if (IS_SET(ch->pcdata->cg_terrain->value[12],CARD_TT_ALLPLUS))
				damage += 2;
			      if (IS_SET(ch->pcdata->cg_terrain->value[12],CARD_TT_ALLMINUS))
				damage -= 2;

			      //Just in case, make sure a damage card never heals anything
			      if (damage < 0)
				damage = 0;
				
			      sprintf(buf," $p damages $P by %d. ",damage);
			      act(buf,ch,ch->pcdata->cg_lcp,target_mob,TO_ALL);
			      //Damage the mob
			      target_stats[CARDGAME_MOB_HP] -= damage;
			      //Nullify this card
			      ch->pcdata->cg_lcp = NULL;
			    }
			  
			}
		    }
		  
		  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
		    {
		      if (rch->pcdata->cg_lcp && 
			  (rch->pcdata->cg_lcp->value[1] == CARD_SPELL_HEAL 
			   || rch->pcdata->cg_lcp->value[1] == CARD_SPELL_DAMAGE))
			{
			  target_mob = ((rch->pcdata->cg_spell_target == 1)?defending_mob:attacking_mob);
			  target_stats = ((rch->pcdata->cg_spell_target == 1)?defending_stats:attacking_stats);
			  target_player = ((rch->pcdata->cg_spell_target == 1)?defending_player:attacking_player);
			  act(" $n plays $p on $P. ",rch,rch->pcdata->cg_lcp,target_mob,TO_ROOM);
			  act(" You play $p on $P. ",rch,rch->pcdata->cg_lcp,target_mob,TO_CHAR);
			  if (!check_counter(rch,rch->pcdata->cg_lcp))
			    {
			      if (rch->pcdata->cg_lcp->value[1] == CARD_SPELL_HEAL)
				{
				  int heal=rch->pcdata->cg_lcp->value[7];
				  
				  if (target_player->pcdata->cg_secondary_mob == target_mob)
				  {
				    //This prevents them from healing a mob over it's max HP (if it's not a main mob)
				    if (heal+target_stats[CARDGAME_MOB_HP] > target_mob->value[VALUE_MOB_HP])
	 			      heal = target_mob->value[VALUE_MOB_HP] - target_stats[CARDGAME_MOB_HP];
				  }
				  else //main mob
				  {
				    //This prevents them from healing a mob over the max main mob hp (10)
 				    if (heal+target_stats[CARDGAME_MOB_HP] > 10)
				      heal = 10 - target_stats[CARDGAME_MOB_HP];
				  }
				  
				  sprintf(buf," $p heals $P by %d. ",heal);
				  act(buf,rch,rch->pcdata->cg_lcp,target_mob,TO_ALL);
				  //Add the hp to the mob
				  target_stats[CARDGAME_MOB_HP] += heal;
				  //Nullify this card
				  rch->pcdata->cg_lcp = NULL;
				}
			      else //Damage card
				{
				  damage = rch->pcdata->cg_lcp->value[7];
				  if (rch->pcdata->cg_lcp->value[7] != CARD_ELEMENT_NONE)
				    {
				      switch (rch->pcdata->cg_lcp->value[8])
					{
					case CARD_ELEMENT_LIGHTNING:
					  if (target_mob->value[11] == CARD_ELEMENT_WATER)
					    damage++;
					  if (target_mob->value[11] == CARD_ELEMENT_STONE)
						damage--;
					  break;
					case CARD_ELEMENT_STONE:
					  if (target_mob->value[11] == CARD_ELEMENT_LIGHTNING)
					    damage++;
					  if (target_mob->value[11] == CARD_ELEMENT_FIRE)
					    damage--;
					  break;
					case CARD_ELEMENT_FIRE:
					  if (target_mob->value[11] == CARD_ELEMENT_STONE)
					    damage++;
					  if (target_mob->value[11] == CARD_ELEMENT_WATER)
					    damage--;
					  break;
					case CARD_ELEMENT_WATER:
					  if (target_mob->value[11] == CARD_ELEMENT_FIRE)
					    damage++;
					  if (target_mob->value[11] == CARD_ELEMENT_LIGHTNING)
					    damage--;
					  break;
					}
				    }
				  sprintf(buf," $p damages $P by %d. ",damage);
				  act(buf,rch,rch->pcdata->cg_lcp,target_mob,TO_ALL);
				  //Damage the mob
				  target_stats[CARDGAME_MOB_HP] -= damage;
				  //Nullify this card
				  rch->pcdata->cg_lcp = NULL;
				}
			      
			    }//if (!check_counter
			}//if heal or damage card
		    }
		      
		  //Reset the mob's off/def to what they were before boost/nemeses/elements
		  attacking_stats[CARDGAME_MOB_OFF] = am_off;
		  attacking_stats[CARDGAME_MOB_DEF] = am_def;
		  defending_stats[CARDGAME_MOB_OFF] = dm_off;
                  defending_stats[CARDGAME_MOB_DEF] = dm_def;
		  
		  //If for some reason their spell card did NOT get played, silently slip it back
		  //into their hand
   	          if (ch->pcdata->cg_lcp != NULL)
	          {
	            obj_from_obj(ch->pcdata->cg_lcp);
	            obj_to_obj_pos(ch->pcdata->cg_lcp,get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"),-1);
		  }
		  //Just in case, reset all the last card played, and various other imporant vars
		  ch->pcdata->cg_lcp = NULL;
		  ch->pcdata->cg_ticks = 0;
		  ch->pcdata->cg_spell_target = 0;
		  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)	  
		    {
		      //If for some reason their spell card did NOT get played, silently slip it back
		      //into their hand
 	 	      if (rch->pcdata->cg_lcp != NULL)
		      {
		        obj_from_obj(rch->pcdata->cg_lcp);
		        obj_to_obj_pos(rch->pcdata->cg_lcp,get_obj_vnum_string_char(rch,OBJ_VNUM_BINDER,"zhand"),-1);                  
		      }
		      rch->pcdata->cg_lcp = NULL;
		      rch->pcdata->cg_ticks = 0;
		      rch->pcdata->cg_spell_target = 0;
		    }
		 
		  sprintf(buf,"$n's attacking mob has %d hp left.",attacking_stats[CARDGAME_MOB_HP]);
		  act(buf,ch,NULL,NULL,TO_ALL);
		  sprintf(buf,"$n's defending mob has %d hp left.",defending_stats[CARDGAME_MOB_HP]);
		  act(buf,defending_player,NULL,NULL,TO_ALL);
		  //End their turn
		  ch->pcdata->cg_state = CG_NYT_MAIN;
		  ch->pcdata->cg_timer = 0;

		  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
		    {
		      rch->pcdata->cg_ticks = 0;
		      rch->pcdata->cg_lcp = NULL;
		    }
		  ch->pcdata->cg_next->pcdata->cg_state = CG_YT_MAIN;
		  ch->pcdata->cg_next->pcdata->cg_timer = current_time;
		  /*act(" $n's turn ends and $N's begins. ",ch,NULL,ch->pcdata->cg_next,TO_NOTVICT);
		  act(" $n's turn ends and yours begins. ",ch,NULL,ch->pcdata->cg_next,TO_VICT);
		  act(" Your turn ends and $N's begins. ",ch,NULL,ch->pcdata->cg_next,TO_CHAR);
		  cardgame_default_menu(ch);
		  cardgame_attack_menu(ch->pcdata->cg_next);*/
		  /*		  if (count_contents(get_obj_vnum_string_char(ch->pcdata->cg_next,OBJ_VNUM_BINDER,"zhand")) < 7)
		  {
 		    obj_to_obj_pos(draw_card(ch->pcdata->cg_next),get_obj_vnum_string_char(ch->pcdata->cg_next,OBJ_VNUM_BINDER,"zhand"),-1);
		    act("$n draws a card.",ch->pcdata->cg_next,NULL,NULL,TO_ROOM);
		    //send_to_char("You draw a card.\n\r",ch->pcdata->cg_next);
		    }*/ //Taken off so one dying doesn't draw a card before death
		  
		  //Check if any mobs bit the bucket
		  if (attacking_stats[CARDGAME_MOB_HP] < 1)
		    {
		      act(" $p is DEAD! ",ch,attacking_mob,NULL,TO_ALL);
		      if (ch->pcdata->cg_main_mob == attacking_mob)
			//YOU LOSE MOTHER FUCKER! GTFO OF THE GAME
			{
			  if (cardgame_surrender(ch,FALSE))
				  return;
			}
		      if (ch->pcdata->cg_secondary_mob == attacking_mob)
			{
			  //Reset the mob's off/def
			  //terrainify_mobs(ch,ch->pcdata->cg_terrain,NULL,ch->pcdata->cg_secondary_mob,NULL);
			  ch->pcdata->cg_secondary_mob = NULL;
			}
		      attacking_stats[CARDGAME_MOB_HP] = 0;
		      attacking_stats[CARDGAME_MOB_OFF] = 0;
		      attacking_stats[CARDGAME_MOB_DEF] = 0;
		    }
		  if (defending_stats[CARDGAME_MOB_HP] < 1)
		    {
		      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
			{
			  if (rch->pcdata->cg_main_mob == defending_mob)
			    {
			      act(" $p is DEAD! ",ch,defending_mob,NULL,TO_ALL);
			      //YOU LOSE MOTHER FUCKER! GTFO OF THE GAME
			      {
				if (cardgame_surrender(rch,FALSE))
					return;
			      }
			      break;
			    }
			  if (rch->pcdata->cg_secondary_mob == defending_mob)
			    {
			      act(" $p is DEAD! ",ch,defending_mob,NULL,TO_ALL);
			      //Reset the mob's off/def
			    //  terrainify_mobs(rch,rch->pcdata->cg_terrain,NULL,rch->pcdata->cg_secondary_mob,NULL);
			      rch->pcdata->cg_secondary_mob = NULL;
			      break;
			    }
			  defending_stats[CARDGAME_MOB_HP] = 0;
                          defending_stats[CARDGAME_MOB_OFF] = 0;
		          defending_stats[CARDGAME_MOB_DEF] = 0;
			}
		    }
		   act(" $n's turn ends and $N's begins. ",ch,NULL,ch->pcdata->cg_next,TO_NOTVICT);
                   act(" $n's turn ends and yours begins. ",ch,NULL,ch->pcdata->cg_next,TO_VICT);
                   act(" Your turn ends and $N's begins. ",ch,NULL,ch->pcdata->cg_next,TO_CHAR);
											   
		  if (count_contents(get_obj_vnum_string_char(ch->pcdata->cg_next,OBJ_VNUM_BINDER,"zhand")) < 7)
		    {
		      obj_to_obj_pos(draw_card(ch->pcdata->cg_next),get_obj_vnum_string_char(ch->pcdata->cg_next,OBJ_VNUM_BINDER,"zhand"),-1);
		      act(" $n draws a card. ",ch->pcdata->cg_next,NULL,NULL,TO_ROOM);
		      //send_to_char("You draw a card.\n\r",ch->pcdata->cg_next);
		    }
		   cardgame_default_menu(ch);
                   cardgame_attack_menu(ch->pcdata->cg_next);
		  
		}
	      else
		{
		  //There is NO pending attack, so just transfer ownership of the turn to the next person
		  act(" $n has run out of time, so $s turn ends and $N's begins. ",ch,NULL,ch->pcdata->cg_next,TO_NOTVICT);
		  act(" Alas, you have run out of time, so your turn ends, and $N's begins. ",ch,NULL,ch->pcdata->cg_next,TO_CHAR);
		  act(" $n has run out of time, so $s turn ends and yours begins. ",ch,NULL,ch->pcdata->cg_next,TO_VICT);
		  /*	  act("And with that, your turn ends, and $N's turn begins.",
			  ch,NULL,ch->pcdata->cg_next,TO_CHAR);
			  act("And with that, $n's turn ends, and $N's turn begins.",
			  ch,NULL,ch->pcdata->cg_next,TO_NOTVICT);
			  act("And with that, $n's turn ends, and your turn begins.",
			  ch,NULL,ch->pcdata->cg_next,TO_VICT);*/
		  ch->pcdata->cg_state = CG_NYT_MAIN;
		  ch->pcdata->cg_timer = 0;
		  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
		    {
		      rch->pcdata->cg_ticks = 0;
		      rch->pcdata->cg_lcp = NULL;
		    }
		  //RESET TICKS FOR EVERYONE!!!!! IN ALL PLACES
		  ch->pcdata->cg_ticks = 0;
		  ch->pcdata->cg_lcp = NULL;
		  ch->pcdata->cg_next->pcdata->cg_state = CG_YT_MAIN;
		  ch->pcdata->cg_next->pcdata->cg_timer = current_time;
		  if (count_contents(get_obj_vnum_string_char(ch->pcdata->cg_next,OBJ_VNUM_BINDER,"zhand")) < 7)
                  {
                    obj_to_obj_pos(draw_card(ch->pcdata->cg_next),get_obj_vnum_string_char(ch->pcdata->cg_next,OBJ_VNUM_BINDER,"zhand"),-1);
                    act(" $n draws a card. ",ch->pcdata->cg_next,NULL,NULL,TO_ROOM);
                   //send_to_char("You draw a card.\n\r",ch->pcdata->cg_next);
                  }
		  cardgame_default_menu(ch);
		  cardgame_attack_menu(ch->pcdata->cg_next);
		}	      
	    }//if (get_mobnumber)
	}//if ((curent-time)
    }//if (d->character
}//for (d = descriptor->list


inline int get_mobnumber(int cg_ticks)
{
  return ((cg_ticks&(CARDGAME_PLAYED_TERRAIN-1))%5);
}

inline int get_victimnumber(int cg_ticks)
{
  return ((cg_ticks&(CARDGAME_PLAYED_TERRAIN-1))/5);
}

void terrainify_mobs(CHAR_DATA *ch, OBJ_DATA *old_terrain, OBJ_DATA *new_terrain, OBJ_DATA *mm, OBJ_DATA *sm)
{
  //In a rare moment for me, I added some non-essential variable to clean up the code and make it shorter
  //It was much more inportant when I first did this, but in the past two hours I've made this a function,
  //and done some other things that made it less useful, but I'm sticking with it
  short *mm_def, *mm_off, *sm_def, *sm_off;
  int *mm_race, *sm_race;
  if (mm)
    {
      mm_def = &ch->pcdata->cg_mm_value[CARDGAME_MOB_DEF];
      mm_off = &ch->pcdata->cg_mm_value[CARDGAME_MOB_OFF];
      mm_race = &mm->value[VALUE_MOB_RACE];
    }
  else 
    {
      mm_def = NULL;
      mm_off = NULL;
      mm_race = NULL;
    }
  if (sm)
    {
      sm_def = &ch->pcdata->cg_sm_value[CARDGAME_MOB_DEF];
      sm_off = &ch->pcdata->cg_sm_value[CARDGAME_MOB_OFF];
      sm_race = &sm->value[VALUE_MOB_RACE];
    }
  else
    {
      sm_def = NULL;
      sm_off = NULL;
      sm_race = NULL;
    }
  //Take off old terrain bonuses, if this is due to a new terrain, and not simply the first terrain
  if (old_terrain != NULL)
    {

      //Fix specific terrain bonuses
      if (old_terrain->value[8] > 0)
        {
          if (mm && mm->value[11] == CARD_ELEMENT_NONE)
            *mm_off -= old_terrain->value[8];
          if (sm && sm->value[11] == CARD_ELEMENT_NONE)
            *sm_off -= old_terrain->value[8];
        }
      if (old_terrain->value[9] > 0)
        {
          if (mm && mm->value[11] == CARD_ELEMENT_NONE)
            *mm_def -= old_terrain->value[9];
          if (sm && sm->value[11] == CARD_ELEMENT_NONE)
            *sm_def -= old_terrain->value[9];
        }
      if (old_terrain->value[10] > 0)
        {
          if (mm && mm->value[11] != CARD_ELEMENT_NONE)
            *mm_off -= old_terrain->value[10];
          if (sm && sm->value[11] != CARD_ELEMENT_NONE)
            *sm_off -= old_terrain->value[10];
        }
      if (old_terrain->value[11] > 0)
        {
          if (mm && mm->value[11] != CARD_ELEMENT_NONE)
            *mm_def -= old_terrain->value[11];
          if (sm && sm->value[11] != CARD_ELEMENT_NONE)
            *sm_def -= old_terrain->value[11];
        }

      //Fix general terrain bonuses
      switch(old_terrain->value[7])
	{
	case CARD_TERRAIN_CITY:
	  if (mm && *mm_race == CARD_RACE_UNDEAD)
	    ++*mm_def;
	  if (mm && *mm_race == CARD_RACE_HUMANOID)
	    --*mm_off;
	  if (sm && *sm_race == CARD_RACE_UNDEAD)
	    ++*sm_def;
	  if (sm && *sm_race == CARD_RACE_HUMANOID)
	    --*sm_off;
	  break;
	case CARD_TERRAIN_SWAMP:
	  if (mm && *mm_race == CARD_RACE_UNDEAD)
            --*mm_def;
	  if (sm && *sm_race == CARD_RACE_UNDEAD)
            --*sm_def;
	  if (mm && *mm_race == CARD_RACE_INSECT)
	    --*mm_off;
	  if (mm && *mm_race == CARD_RACE_HUMANOID)
	    ++*mm_def;
	  if (sm && *sm_race == CARD_RACE_INSECT)
	    --*sm_off;
	  if (sm && *sm_race == CARD_RACE_HUMANOID)
	    ++*sm_def;
	  break;
	case CARD_TERRAIN_FOREST:
	  if (mm && (*mm_race == CARD_RACE_ANIMAL || *mm_race == CARD_RACE_BIRD || *mm_race == CARD_RACE_INSECT))
	    --*mm_def;
	  if (mm && *mm_race == CARD_RACE_DRAGON)
	    ++*mm_off;
	  if (sm && (*sm_race == CARD_RACE_ANIMAL || *sm_race == CARD_RACE_BIRD || *sm_race == CARD_RACE_INSECT))
	    --*sm_def;
	  if (sm && *sm_race == CARD_RACE_DRAGON)
	    ++*sm_off;
	  break;
	case CARD_TERRAIN_MOUNTAIN:
	  if (mm && (*mm_race == CARD_RACE_DRAGON || *mm_race == CARD_RACE_BIRD || *mm_race == CARD_RACE_MONSTER))
	    --*mm_off;
	  if (sm && (*sm_race == CARD_RACE_DRAGON || *sm_race == CARD_RACE_BIRD || *sm_race == CARD_RACE_MONSTER))
	    --*sm_off;
	  break;
	case CARD_TERRAIN_SEA:
	  if (mm && (*mm_race == CARD_RACE_FISH))
	    {
	      --*mm_def;
	      --*mm_off;
	    }
	  if (mm && (*mm_race == CARD_RACE_BIRD || *mm_race == CARD_RACE_MAGIC))
	    ++*mm_off;
	  if (sm && (*sm_race == CARD_RACE_FISH))
	    {
	      --*sm_def;
	      --*sm_off;
	    }
	  if (sm && (*sm_race == CARD_RACE_DRAGON || *sm_race == CARD_RACE_MAGIC))
	    ++*sm_off;
	  break;
	case CARD_TERRAIN_DESERT:
	  if (mm && *mm_race == CARD_RACE_MAGIC)
	    {
	      --*mm_def;
	      --*mm_off;
	    }
	  if (mm && *mm_race != CARD_RACE_MAGIC)
	    ++*mm_def;
	  if (sm && *sm_race == CARD_RACE_MAGIC)
	    {
	      --*sm_def;
	      --*mm_off;
	    }
	  if (sm && *sm_race != CARD_RACE_MAGIC)
	    ++*sm_def;
	  break;
	case CARD_TERRAIN_AIR:
	  if (mm && *mm_race == CARD_RACE_FISH)
	    {
	      --*mm_def;
	      --*mm_off;
	    }
	  if (mm && (*mm_race == CARD_RACE_BIRD || *mm_race == CARD_RACE_MAGIC))
	    *mm_def += 2;
	  if (sm && (*sm_race == CARD_RACE_BIRD || *sm_race == CARD_RACE_DRAGON))
	    {
	      --*sm_def;
	      --*sm_off;
	    }
	  if (sm && *sm_race == CARD_RACE_FISH)
	    *sm_def += 2;
	  break;
	}
    }
  if (new_terrain != NULL)
    {
      //Add new terrain bonuses

      //Add the specific terrain bonuses
      if (new_terrain->value[8] > 0)
	{
	  if (mm && mm->value[11] == CARD_ELEMENT_NONE)
	    *mm_off += new_terrain->value[8];
	  if (sm && sm->value[11] == CARD_ELEMENT_NONE)
	    *sm_off += new_terrain->value[8];
	}
      if (new_terrain->value[9] > 0)
        {
          if (mm && mm->value[11] == CARD_ELEMENT_NONE)
            *mm_def += new_terrain->value[9];
          if (sm && sm->value[11] == CARD_ELEMENT_NONE)
            *sm_def += new_terrain->value[9];
        }
      if (new_terrain->value[10] > 0)
        {
          if (mm && mm->value[11] != CARD_ELEMENT_NONE)
            *mm_off += new_terrain->value[10];
          if (sm && sm->value[11] != CARD_ELEMENT_NONE)
            *sm_off += new_terrain->value[10];
        }
      if (new_terrain->value[11] > 0)
        {
          if (mm && mm->value[11] != CARD_ELEMENT_NONE)
            *mm_def += new_terrain->value[11];
          if (sm && sm->value[11] != CARD_ELEMENT_NONE)
            *sm_def += new_terrain->value[11];
        }


      //Add the general terrain bonuses
      switch(new_terrain->value[7])
	{
	case CARD_TERRAIN_CITY:
	  if (mm && *mm_race == CARD_RACE_UNDEAD)
	    --*mm_def;
	  if (mm && *mm_race == CARD_RACE_HUMANOID)
	    ++*mm_off;
	  if (sm && *sm_race == CARD_RACE_UNDEAD)
	    --*sm_def;
	  if (sm && *sm_race == CARD_RACE_HUMANOID)
	    ++*sm_off;
	  break;
	case CARD_TERRAIN_SWAMP:
	  if (mm && *mm_race == CARD_RACE_UNDEAD)
            ++*mm_def;
	  if (sm && *sm_race == CARD_RACE_UNDEAD)
            ++*sm_def;
	  if (mm && *mm_race == CARD_RACE_INSECT)
	    ++*mm_off;
	  if (mm && *mm_race == CARD_RACE_HUMANOID)
	    --*mm_def;
	  if (sm && *sm_race == CARD_RACE_INSECT)
	    ++*sm_off;
	  if (sm && *sm_race == CARD_RACE_HUMANOID)
	    --*sm_def;
	  break;
	case CARD_TERRAIN_FOREST:
	  if (mm && (*mm_race == CARD_RACE_ANIMAL || *mm_race == CARD_RACE_BIRD || *mm_race == CARD_RACE_INSECT))
	    ++*mm_def;
	  if (mm && *mm_race == CARD_RACE_DRAGON)
	    --*mm_off;
	  if (sm && (*sm_race == CARD_RACE_ANIMAL || *sm_race == CARD_RACE_BIRD || *sm_race == CARD_RACE_INSECT))
	    ++*sm_def;
	  if (sm && *sm_race == CARD_RACE_DRAGON)
	    --*sm_off;
	  break;
	case CARD_TERRAIN_MOUNTAIN:
	  if (mm && (*mm_race == CARD_RACE_DRAGON || *mm_race == CARD_RACE_BIRD || *mm_race == CARD_RACE_MONSTER))
	    ++*mm_off;
	  if (sm && (*sm_race == CARD_RACE_DRAGON || *sm_race == CARD_RACE_BIRD || *sm_race == CARD_RACE_MONSTER))
	    ++*sm_off;
	  break;
	case CARD_TERRAIN_SEA:
	  if (mm && (*mm_race == CARD_RACE_FISH))
	    {
	      ++*mm_def;
	      ++*mm_off;
	    }
	  if (mm && (*mm_race == CARD_RACE_BIRD || *mm_race == CARD_RACE_MAGIC))
	    --*mm_off;
	  if (sm && (*sm_race == CARD_RACE_FISH))
	    {
	      ++*sm_def;
	      ++*sm_off;
	    }
	  if (sm && (*sm_race == CARD_RACE_DRAGON || *sm_race == CARD_RACE_MAGIC))
	    --*sm_off;
	  break;
	case CARD_TERRAIN_DESERT:
	  if (mm && *mm_race == CARD_RACE_MAGIC)
	    {
	      ++*mm_def;
	      ++*mm_off;
	    }
	  if (mm && *mm_race != CARD_RACE_MAGIC)
	    --*mm_def;
	  if (sm && *sm_race == CARD_RACE_MAGIC)
	    {
	      ++*sm_def;
	      ++*mm_off;
	    }
	  if (sm && *sm_race != CARD_RACE_MAGIC)
	    --*sm_def;
	  break;
	case CARD_TERRAIN_AIR:
	  if (mm && *mm_race == CARD_RACE_FISH)
	    {
	      ++*mm_def;
	      ++*mm_off;
	    }
	  if (mm && (*mm_race == CARD_RACE_BIRD || *mm_race == CARD_RACE_MAGIC))
	    *mm_def -= 2;
	  if (sm && (*sm_race == CARD_RACE_BIRD || *sm_race == CARD_RACE_DRAGON))
	    {
	      ++*sm_def;
	      ++*sm_off;
	    }
	  if (sm && *sm_race == CARD_RACE_FISH)
	    *sm_def -= 2;
	  break;
	}
    }
}


bool check_counter(CHAR_DATA *ch, OBJ_DATA *card)
{
  CHAR_DATA *rch;
  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
    {
      if (rch->pcdata->cg_lcp && rch->pcdata->cg_lcp->value[1] == CARD_SPELL_COUNTER && rch->pcdata->cg_spell_target == ch->pcdata->cg_player_number)
      {
        //Spell has been countered, so do the act then discard both cards
	act(" $N's $p counters $n's spell! ",ch,rch->pcdata->cg_lcp,rch,TO_NOTVICT);
	act(" $N's $p counters your spell! ",ch,rch->pcdata->cg_lcp,rch,TO_CHAR);
	act(" Your $p counters $n's spell! ",ch,rch->pcdata->cg_lcp,rch,TO_VICT);
	ch->pcdata->cg_lcp = NULL;
	rch->pcdata->cg_lcp = NULL;
	return TRUE;
      }
    }
  return FALSE;
}

bool cardgame_surrender(CHAR_DATA *ch, bool premature)
{
  CHAR_DATA *rch;
  OBJ_DATA *card, *container, *deck, *ante_bag, *obj, *obj_next;
  bool gameover = FALSE;
  int i;
  char buf[MAX_STRING_LENGTH];
  if (ch->pcdata->cg_next->pcdata->cg_next == ch)
    {
      //There is only one person left, so they win
      rch = ch->pcdata->cg_next;
      //Terrainify the loser's mobs, so they will be fixed
      //terrainify_mobs(ch,ch->pcdata->cg_terrain,NULL,ch->pcdata->cg_main_mob,ch->pcdata->cg_secondary_mob);

      //DEAL WITH ANTE SHIT HERE

      //Announce ch is a big fucking LOSER
      if (!premature)
      {
        send_to_char(" Your main mob was destroyed, thus you are a LOSER! \n\r",ch);
        act(" $n's main mob was destroyed, thus $e is a LOSER and is out of the game! ",ch,NULL,NULL,TO_ROOM);
      }
      else
      {
        send_to_char(" You have wussed out and surrendered, thus you are a LOSER! \n\r",ch);
	act(" $n has wussed out and surrendered, thus $e is a LOSER and is out of the game! ",ch,NULL,NULL,TO_ROOM);
      }
      ++ch->pcdata->total_cg_lost;
      //game's done
      gameover = TRUE;

      //Reset all their cardgame variables
      ch->pcdata->cg_next = 0;
      ch->pcdata->cg_state = 0;
      ch->pcdata->cg_ticks = 0;
      ch->pcdata->cg_next = NULL;
      ch->pcdata->cg_spell_target = 0;
      ch->pcdata->cg_terrain = NULL;
      ch->pcdata->cg_main_mob = NULL;
      ch->pcdata->cg_secondary_mob = NULL;
      ch->pcdata->cg_lcp = NULL;
      for(i=0; i<3; i++)
        ch->pcdata->cg_mm_value[i] = 0;
      for(i=0; i<3; i++)
        ch->pcdata->cg_sm_value[i] = 0;

      //Get everything from the hand and discard, and put back into the deck
      deck = get_obj_vnum_char(ch,OBJ_VNUM_DECK);
      container = get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zdiscard");
      for (card = container->contains;container->contains;card = container->contains)
      {
        obj_from_obj(card);
	obj_to_obj_pos(card,deck,-1);
      }
      obj_from_char(container);
      extract_obj(container);
      container = get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand");
      for (card = container->contains;container->contains;card = container->contains)
      {
	obj_from_obj(card);
        obj_to_obj_pos(card,deck,-1);
      }
      obj_from_char(container);
      extract_obj(container);

      //Make it so the player can see their deck again
      REMOVE_BIT(deck->extra_flags[1],ITEM_WIZI);

      //Terrainify the winner's mobs, so they will be fixed
      //terrainify_mobs(rch,rch->pcdata->cg_terrain,NULL,rch->pcdata->cg_main_mob,rch->pcdata->cg_secondary_mob);

      //Announce ch is a big fucking WINNER
      send_to_char(" Your main mob is the last one alive, thus you are the WINNER! \n\r",rch);
      act(" $n's main mob is the last one still alive, thus $s is the WINNER! ",rch,NULL,NULL,TO_ROOM);
      ++rch->pcdata->total_cg_won;


      //Deal with Ante stuff
      ante_bag = get_obj_vnum_char(ch,OBJ_VNUM_ANTE_BAG);
      container = get_obj_vnum_char(rch,OBJ_VNUM_ANTE_BAG);
      if (ante_bag->value[5] + container->value[5] > 0)
      {
	sprintf(buf," $n wins %d silver from the pot. ",ante_bag->value[5]+container->value[5]);
	act(buf,rch,NULL,NULL,TO_ROOM);
	sprintf(buf," You win %d silver from the pot. \n\r",ante_bag->value[5]+container->value[5]);
	send_to_char(buf,rch);
	rch->silver += ante_bag->value[5]+container->value[5];
      }
      if (ante_bag->value[6] + container->value[6]> 0)
	{
	  sprintf(buf," $n wins %d gold from the pot. ",ante_bag->value[6]+container->value[6]);
	  act(buf,rch,NULL,NULL,TO_ROOM);
	  sprintf(buf," You win %d gold from the pot. \n\r",ante_bag->value[6]+container->value[6]);
	  send_to_char(buf,rch);
	  rch->gold += ante_bag->value[6]+container->value[6];
	}
      for (obj = ante_bag->contains;obj != NULL; obj = obj_next)
	{
  	  obj_next = obj->next_content;
	  act(" $n wins $p from the pot. ",rch,obj,NULL,TO_ROOM);
	  act(" You win $p from the pot. ",rch,obj,NULL,TO_CHAR);
	  obj_from_obj(obj);
	  obj_to_char(obj,rch);
	}
      obj_from_char(ante_bag);
      extract_obj(ante_bag);
      if ((ante_bag = get_obj_vnum_char(rch,OBJ_VNUM_ANTE_BAG)))
	{
	  for (obj = ante_bag->contains;obj != NULL; obj = obj_next)
	    {
  	      obj_next = obj->next_content;
              act(" $n wins $p from the pot. ",rch,obj,NULL,TO_ROOM);
	      act(" You win $p from the pot. ",rch,obj,NULL,TO_CHAR);
	      obj_from_obj(obj);
	      obj_to_char(obj,rch);
	    }
	  obj_from_char(ante_bag);
	  extract_obj(ante_bag);
	}



      
      //Reset all their cardgame variables
      rch->pcdata->cg_next = 0;
      rch->pcdata->cg_state = 0;
      rch->pcdata->cg_ticks = 0;
      rch->pcdata->cg_next = NULL;
      rch->pcdata->cg_spell_target = 0;
      rch->pcdata->cg_terrain = NULL;
      rch->pcdata->cg_main_mob = NULL;
      rch->pcdata->cg_secondary_mob = NULL;
      rch->pcdata->cg_lcp = NULL;
      for(i=0; i<3; i++)
        rch->pcdata->cg_mm_value[i] = 0;
      for(i=0; i<3; i++)
        rch->pcdata->cg_sm_value[i] = 0;

      //Get everything from the hand and discard, and put back into the deck
      deck = get_obj_vnum_char(rch,OBJ_VNUM_DECK);
      container = get_obj_vnum_string_char(rch,OBJ_VNUM_BINDER,"zdiscard");
      for (card = container->contains;container->contains;card = container->contains)
	{
	  obj_from_obj(card);
	  obj_to_obj_pos(card,deck,-1);
	}
      obj_from_char(container);
      extract_obj(container);
      container = get_obj_vnum_string_char(rch,OBJ_VNUM_BINDER,"zhand");
      for (card = container->contains;container->contains;card = container->contains)
	{
	  obj_from_obj(card);
	  obj_to_obj_pos(card,deck,-1);
	}
      obj_from_char(container);
      extract_obj(container);

      //Make it so the player can see their deck again
      REMOVE_BIT(deck->extra_flags[1],ITEM_WIZI);

    }
  else
    {
      for (rch = ch->pcdata->cg_next;rch->pcdata->cg_next != ch;rch = rch->pcdata->cg_next);
      if (rch->pcdata->cg_next != ch)
	{
	  bug("Fuck, somehow the cg_next is going to get fucked..I'm in cardgame_surrender",0);
	  return FALSE;
	//ERROR
	}

      //Terrainify the loser's mobs, so they will be fixed
      //
      //terrainify_mobs(ch,ch->pcdata->cg_terrain,NULL,ch->pcdata->cg_main_mob,ch->pcdata->cg_secondary_mob);
      
      //DEAL WITH ANTE SHIT HERE
      container = get_obj_vnum_char(ch,OBJ_VNUM_ANTE_BAG);
      ante_bag = get_obj_vnum_char(ch->pcdata->cg_next,OBJ_VNUM_ANTE_BAG);
      ante_bag->value[5] += container->value[5];
      ante_bag->value[6] += container->value[6];
      for (obj = container->contains;obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  obj_from_obj(obj);
	  obj_to_obj(obj,ante_bag);
	}
      obj_from_char(container);
      extract_obj(container);




      //Announce ch is a big fucking LOSER
      if (!premature)
      {
        send_to_char(" Your main mob was destroyed, thus you are a LOSER! \n\r",ch);
        act(" $n's main mob was destroyed, thus $e is a LOSER and is out of the game! ",ch,NULL,NULL,TO_ROOM);
      }
      else
      {
        send_to_char(" You have wussed out and surrendered, thus you are a LOSER! \n\r",ch);
        act(" $n has wussed out and surrendered, thus $e is a LOSER and is out of the game! ",ch,NULL,NULL,TO_ROOM);
      }
      --ch->pcdata->total_cg_lost;
	      
      if (ch->pcdata->cg_state >= CG_YT_MAIN && ch->pcdata->cg_state <= CG_YT_DONE)
	{
	  ch->pcdata->cg_next->pcdata->cg_state = CG_YT_MAIN;
	  act(" Since $n has lost, it is now your turn. ",ch,NULL,ch->pcdata->cg_next,TO_VICT);
	  act(" Since $n has lost, it is now $N's turn. ",ch,NULL,ch->pcdata->cg_next,TO_NOTVICT);
	  
	  //Reset all the variables needing reset on turn change
	  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
	  {
	    rch->pcdata->cg_ticks = 0;
	    rch->pcdata->cg_lcp = NULL;
	    rch->pcdata->cg_spell_target = 0;
	  }
	  cardgame_attack_menu(ch->pcdata->cg_next);
	  
	}

       //Fix the terrain, if the one surrendering owns the terrain currently in play
       for (card = (get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zdiscard"))->contains;card != NULL;card = card->next_content)
       {
	 //The terrain currently being used is owned by this player who is leaving the game
         if (card == ch->pcdata->cg_terrain)
	   {
	        CHAR_DATA *och = ch->pcdata->cg_next;
		int amount=0,counter=0;
		act(" Since $n is leaving the game and $e owns the terrain currently in play, a new one will be randomly selected from the hands/decks of the remaining players. ",ch,NULL,NULL,TO_ROOM);
		act(" Since you are leaving the game and you own the terrain currently in play, a new one will be randomly selected. ",ch,NULL,NULL,TO_CHAR);
		container = get_obj_vnum_char(och,OBJ_VNUM_DECK);
 	  	  for (card = container->contains;card != NULL; card = card->next_content)
		  {
 	 	    if (card->value[1] == CARD_TERRAIN)
		      ++amount;
		  }
		  if (amount == 0)
		  {
		    container = get_obj_vnum_string_char(och,OBJ_VNUM_BINDER,"zdiscard");
		    for (card = container->contains;card != NULL; card = card->next_content)
		    {
 		      if (card->value[1] == CARD_TERRAIN)
		        ++amount;
		    }
		    if (amount == 0)
	  	      bug("There do not have a terrain in their deck or discard!",0);
		  }
		  amount = number_range(1,amount);
		  for (card = container->contains;card != NULL; card = card->next_content)
		  {
		    if (card->value[1] == CARD_TERRAIN && amount == ++counter)
		    {
 		      for (rch = och->pcdata->cg_next;rch != och; rch = rch->pcdata->cg_next)
		        rch->pcdata->cg_terrain = card;
		      och->pcdata->cg_terrain = card;
		      obj_from_obj(card);
		      discard_card(och,card);
		      act(" The new terrain $p was randomly chosen from $n. ",och,card,NULL,TO_ROOM);
		      act(" The new terrain $p was randomly chosen from you. ",och,card,NULL,TO_CHAR);
		      break;
		    }
		  }
	    break;
	   }
       }

       //Make sure the link circle is working properly
       for (rch = ch->pcdata->cg_next;rch->pcdata->cg_next != ch;rch = rch->pcdata->cg_next);
         if (rch->pcdata->cg_next != ch)
	 {
           bug("Fuck, somehow the cg_next is going to get fucked..I'm in cardgame_surrender",0);
           return FALSE;
	 }

      //Fix the cg_next link circle
      rch->pcdata->cg_next = ch->pcdata->cg_next;

      //Reset all their cardgame variables
      ch->pcdata->cg_state = 0;
      ch->pcdata->cg_ticks = 0;
      ch->pcdata->cg_next = NULL;
      ch->pcdata->cg_spell_target = 0;
      ch->pcdata->cg_terrain = NULL;
      ch->pcdata->cg_main_mob = NULL;
      ch->pcdata->cg_secondary_mob = NULL;
      ch->pcdata->cg_lcp = NULL;
      for(i=0; i<3; i++)
        ch->pcdata->cg_mm_value[i] = 0;
      for(i=0; i<3; i++)
        ch->pcdata->cg_sm_value[i] = 0;
	    

      //Get everything from the hand and discard, and put back into the deck
      deck = get_obj_vnum_char(ch,OBJ_VNUM_DECK);
      container = get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zdiscard");
      for (card = container->contains;container->contains;card = container->contains)
	{
	  obj_from_obj(card);
	  obj_to_obj_pos(card,deck,-1);
	}
      obj_from_char(container);
      extract_obj(container);
      container = get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand");
      for (card = container->contains;container->contains;card = container->contains)
	{
	  obj_from_obj(card);
	  obj_to_obj_pos(card,deck,-1);
	}
      obj_from_char(container);
      extract_obj(container);

      //Make it so the player can see their deck again
      REMOVE_BIT(deck->extra_flags[1],ITEM_WIZI);


    }
  return gameover;
}

void do_ante(CHAR_DATA* ch, char* argument)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj, *ante_bag;
//  int amount;
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *rch=NULL;
  if (IS_NPC(ch) || !(ante_bag = get_obj_vnum_char(ch, OBJ_VNUM_ANTE_BAG)) 
      || ch->pcdata->cg_state < CG_ANTE || ch->pcdata->cg_state > CG_ANTE_ACCEPTED)
    {
      send_to_char("Huh?\n\r",ch);
      return;
    }
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0')
    {
      send_to_char("Syntax :\n\r",ch);
      send_to_char("  Ante add `kitem``\n\r",ch);
      send_to_char("  Ante add `kX silver/gold``\n\r",ch);
      send_to_char("  Ante remove `kitem``\n\r",ch);
      send_to_char("  Ante remove `kX silver/gold``\n\r",ch);
      send_to_char("  Ante accept``\n\r",ch);
      send_to_char("  Ante decline``\n\r",ch);
      send_to_char("  Ante list``\n\r",ch);
      send_to_char("  Ante quit``\n\r",ch);
      return;
    }
  if (!str_cmp (arg1, "add"))
    {
      if (arg2[0] == '\0')
	{
	  send_to_char("Ante add what?\n\r",ch);
	  return;
	}
      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
      {
        if (rch->pcdata->cg_state == CG_ANTE_ACCEPTED)
	{
  	  send_to_char("Someone has accepted, so you must ante decline before you can add things.\n\r",ch);
	  return;
	}
      }
      if (is_number (arg2)) //They're adding X Silver/gold
	{
	  int amount = atoi(arg2);
	  bool silver=FALSE;
	  if (amount <= 0 || (str_cmp (argument, "coins") && str_cmp (argument, "coin")
			      && str_cmp (argument, "gold")
			      && str_cmp (argument, "silver")))
	    {
	      send_to_char ("Sorry, you can't do that.\n\r", ch);
	      return;
	    }
	  silver = str_cmp (argument, "gold");
	  if ((!silver && ch->gold < amount) || (silver && ch->silver < amount))
	    {
	      send_to_char ("You haven't got that much.\n\r", ch);
	      return;
	    }
	  if (silver)
	    {
	      ch->silver -= amount;
	      ante_bag->value[5] += amount;
	    }
	  else
	    {
	      ch->gold -= amount;
	      ante_bag->value[6] += amount;
	    }
	  sprintf (buf, " $n adds %d %s to $s ante. ", amount, silver ? "silver" : "gold");
	  act(buf,ch,NULL,NULL,TO_ROOM);
	  sprintf (buf, " You add %d %s to your ante. ", amount, silver ? "silver" : "gold");
	  act(buf,ch,NULL,NULL,TO_CHAR);
	}
      else //They're adding an item
	{
	  if ((obj = get_obj_carry (ch, arg2)) == NULL)
	    {
	      send_to_char ("You do not have that item.\n\r", ch);
	      return;
	    }
	  if (obj->wear_loc != WEAR_NONE)
	    {
	      send_to_char ("You must remove it first.\n\r", ch);
	      return;
	    }
	  if (!can_drop_obj (ch, obj))
	    {
	      send_to_char ("You can't let go of it.\n\r", ch);
	      return;
	    }
	  if (obj->item_type == ITEM_QUIVER || obj->item_type == ITEM_CONTAINER
	      || obj->item_type == ITEM_PACK || obj->item_type == ITEM_BINDER)
	    {
	      send_to_char ("You cannot add that item.\n\r",ch);
	      return;
	    }
	  // Adeon 7/18/03 -- cant put nosave stuff in containers
	  if (IS_OBJ_STAT (obj, ITEM_NOSAVE))
	    {
	      send_to_char ("Strong magic forces prevent you...\n\r", ch);
	      return;
	    }
	  obj_from_char (obj);
	  obj_to_obj (obj, ante_bag);
	  act(" $n has added $p to $s ante. ",ch,obj,NULL,TO_ROOM);
	  act(" You have added $p to your ante. ",ch,obj,NULL,TO_CHAR);
	}
      return; //End of ante add X
    }
  else if (!str_cmp(arg1,"remove"))
    {
      if (arg2[0] == '\0')
      {
        send_to_char("Ante remove what?\n\r",ch);
        return;
      }
	    
      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
      {
        if (rch->pcdata->cg_state == CG_ANTE_ACCEPTED)
        {
          send_to_char("Someone has accepted, so you must ante decline before you can add things.\n\r",ch);
	  return;
	 }
      }
					    
      if (is_number (arg2)) //ante remove X gold/silver
	{
	  int amount = atoi(arg2);
	  if (amount <= 0 || (str_cmp (argument, "coins") && str_cmp (argument, "coin")
			      && str_cmp (argument, "gold")
			      && str_cmp (argument, "silver")))
	    {
	      send_to_char ("Sorry, you can't do that.\n\r", ch);
	      return;
	    }
	  if (!str_cmp(argument,"gold"))
	    {
	      if (ante_bag->value[6] < amount)
		{
		  send_to_char ("You have not anteed that much gold.\n\r",ch);
		  return;
		}
	      ante_bag->value[6] -= amount;
	      ch->gold += amount;
	    }
	  else
	    {
	      if (ante_bag->value[5] < amount)
                {
                  send_to_char ("You have not anteed that much silver.\n\r",ch);
                  return;
                }
              ante_bag->value[5] -= amount;
              ch->silver += amount;
	    }
	}
      else 
	{
	  
	  obj = get_obj_list (ch, arg2, ante_bag->contains);
	  if (obj == NULL)
	    {
	      send_to_char ("You have not anteed anything like that.\n\r",ch);
	      return;
	    }
	  get_obj (ch, obj, ante_bag);
	}
      if (get_carry_weight (ch) > can_carry_w (ch) && IS_AFFECTED (ch, AFF_FLYING))
	{
	  send_to_char("You are so heavy you fall to the ground.\n\r",ch);
	  do_land(ch,"");
	  return;
	}
      return;
    }
  else if (!str_cmp(arg1,"accept"))
    {
      OBJ_DATA *hand,*card, *deck, *discard;
      short count=0, counter=0;
      CHAR_DATA *real_ch;
      real_ch = ch;
      if (ch->pcdata->cg_state == CG_ANTE_ACCEPTED)
	{
	  send_to_char ("You've already accepted.\n\r",ch);
	  return;
	}
      ch->pcdata->cg_state = CG_ANTE_ACCEPTED;
      act(" $n accepts the current ante. ",ch,NULL,NULL,TO_ROOM);
      send_to_char("You let it be known that you accept the current ante.\n\r",ch);
      //CHECK TO SEE IF EVERYONE ELSE HAS ACCEPTED THE ANTE
      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
	{
	  if (rch->pcdata->cg_state != CG_ANTE_ACCEPTED)
	    return;
	}
      //If exited the loop gracefully, thus everyone has accepted the ante, so we can move on
      
      //Pick who goes first.
      count = number_range(1,count);
      for (ch = real_ch->in_room->people;ch != NULL;ch = ch->next_in_room)
        {
          if (IS_NPC(ch) || ch->pcdata->cg_next == NULL)
            continue;
          ++counter;
          if (counter == count)
	    {
	      ch->pcdata->cg_state = CG_START_PM_YT;
	    }
          else ch->pcdata->cg_state = CG_START_PM;

	  //RESET ALL VARIABLES
	  ch->pcdata->cg_ticks = 0;
	  ch->pcdata->cg_spell_target = 0;
	  ch->pcdata->cg_player_number = 0;
	  ch->pcdata->cg_timer = 0;
	  ch->pcdata->cg_main_mob = NULL;
	  ch->pcdata->cg_terrain = NULL;
	  ch->pcdata->cg_lcp = NULL;
	  ch->pcdata->cg_main_mob = NULL;
        }
      /*      if (ch == NULL)
        {
          bug("IT IS NO ONE's TURN",0);
          d->character->pcdata->cg_state = CG_START_PM_YT;
	  }*/
      //Draw hands for them all
      for (ch = real_ch->in_room->people;ch != NULL;ch = ch->next_in_room)
        {
          bool hasmob=FALSE,hasterrain=FALSE;
          if (IS_NPC(ch) || ch->pcdata->cg_next == NULL)
            continue;

          deck = get_obj_vnum_char(ch,OBJ_VNUM_DECK);

          //create the "hand" object, containing the player's hand
          hand = create_object(get_obj_index(OBJ_VNUM_BINDER),0);
          SET_BIT(hand->extra_flags[1],ITEM_WIZI);
          free_string (hand->name);
          hand->name = str_dup ("zhand");
          hand->value[0] = 0;
          obj_to_char(hand,ch);
	  //create the "discard" object, containing the discarded cards
          discard = create_object(get_obj_index(OBJ_VNUM_BINDER),0);
          SET_BIT(discard->extra_flags[1],ITEM_WIZI);
          free_string (discard->name);
          discard->name = str_dup ("zdiscard");
          discard->value[0] = 0;
          obj_to_char(discard,ch);
          if (ch->pcdata->cg_next->pcdata->cg_state != CG_START_PM_YT)
	    {
            do
	      {
		hasmob = FALSE;

		//If looped once, put all the cards from the hand back into the deck and shuffle
		while (hand->contains)
		  {
		    card = hand->contains;
		    obj_from_obj(card);
		    obj_to_obj_pos (card,deck,-1);
		  }

		//Draw a 7 card hand
		for (count=0;count<7;count++)
		  {
		    card = draw_card(ch);

		    //Make sure the hand has at least 1 mob
		    if (card->value[1] == CARD_MOB)
		      hasmob = TRUE;

		    obj_to_obj(card,hand);
		  }

		//If the hand doesn't have at least 1 mob, redraw
	      } while (!hasmob);
	    cardgame_hand_display(ch);
            send_to_char("Pick a mob from your hand to play (1-7).\n\r",ch);
	    }

          //If the player is the last player, they have to chose a terrain, so have them draw
          //an extra card, and make sure their hand also has at least 1 terrain
          if (ch->pcdata->cg_next->pcdata->cg_state == CG_START_PM_YT)
	    {
	      int icounter = 1,i;
	      ch->pcdata->cg_state = CG_START_PT;

	      //Assign players numbers here
	      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
		{
		  //Assign the player's number
		  rch->pcdata->cg_player_number = icounter++;
		  //Reset all to 0, just in case they were not already 0
		  rch->pcdata->cg_ticks = 0;
		  rch->pcdata->cg_main_mob = NULL;
		  rch->pcdata->cg_secondary_mob = NULL;
		  rch->pcdata->cg_terrain = NULL;
		  rch->pcdata->cg_lcp = NULL;
		  rch->pcdata->cg_mobs_played = 0;
		  rch->pcdata->cg_timer = 0;
		  for (i=0; i<3; ++i)
		    rch->pcdata->cg_mm_value[i] = 0;
		  for (i=0; i<3; ++i)
		    rch->pcdata->cg_sm_value[i] = 0;
		}
	      ch->pcdata->cg_ticks = 0;
	      ch->pcdata->cg_main_mob = NULL;
	      ch->pcdata->cg_secondary_mob = NULL;
	      ch->pcdata->cg_terrain = NULL;
	      ch->pcdata->cg_lcp = NULL;
	      ch->pcdata->cg_mobs_played = 0;
	      ch->pcdata->cg_timer = 0;
	      for (i=0; i<3; ++i)
		ch->pcdata->cg_mm_value[i] = 0;
	      for (i=0; i<3; ++i)
		ch->pcdata->cg_sm_value[i] = 0;
	      ch->pcdata->cg_player_number = icounter;

	      card = draw_card(ch);
	      obj_to_obj(card,hand);
	      for (card = hand->contains;card != NULL;card = card->next_content)
		{
		  if (card->value[1] == CARD_MOB)
		    {
		      hasmob = TRUE;
		    }
		  if (card->value[1] == CARD_TERRAIN)
		    {
		      hasterrain = TRUE;
		    }
		}

	      //If the hand doesn't have at least 1 mob and 1 terrain, redraw the hand
	      while (!hasmob || !hasterrain)
		{
		  hasmob = FALSE;
		  hasterrain = FALSE;

		  //Put all the cards from the hand back into the deck, and reshuffle
		  while (hand->contains)
		    {
		      card = hand->contains;
		      obj_from_obj(card);
		      obj_to_obj_pos (card,deck,-1);
		    }

		  //Draw an 8 card hand
		  for (count=0;count<8;count++)
		    {
		      card = draw_card(ch);
		      if (card->value[1] == CARD_MOB)
			hasmob = TRUE;
		      if (card->value[1] == CARD_TERRAIN)
			hasterrain = TRUE;
		      obj_to_obj(card,hand);
		    }
		} //end of while (!hasmob || !hasterrain)
	      cardgame_hand_display(ch);
	      send_to_char("Pick a terrain from your hand to play (1-8).\n\r",ch);
	    } //end of if (ch->pcdata->cg_next->pcdata->cg_state == CG_START_PM_YT)



        }//end of for (ch = d->character->in_room->people;ch != NULL;ch = ch->next_in_room)
      return;
    }
  else if (!str_cmp(arg1,"decline"))
    {
      ch->pcdata->cg_state = CG_ANTE;
      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
	rch->pcdata->cg_state = CG_ANTE;
      act(" $n has declined the current ante. ",ch,NULL,NULL,TO_ROOM);
      send_to_char(" You let it be known that you decline the current ante. \n\r",ch);
      act(" You may now add or remove items from your own ante. ",ch,NULL,NULL,TO_ROOM);
      send_to_char(" You may now add or remove items from your own ante. \n\r",ch);
      return;
    }
  else if (!str_cmp(arg1,"quit"))
    {
      OBJ_DATA *obj_next;
      act(" $n has abandoned the anteing, and thus the game. ",ch,NULL,NULL,TO_ROOM);
      send_to_char(" You abandon the anteing, and thus the game. \n\r",ch);
      if (ante_bag->value[6] > 0)
        ch->gold += ante_bag->value[6];
      if (ante_bag->value[5] > 0)
	ch->silver += ante_bag->value[5];
      for (obj = ante_bag->contains;obj != NULL;obj = obj_next)
	{
	  obj_next = obj->next_content;
	  obj_from_obj(obj);
	  obj_to_char(obj,ch);
	}
      obj_from_char(ante_bag);
      extract_obj(ante_bag);
      send_to_char(" Your own ante has been returned to you. \n\r",ch);
      ante_bag = get_obj_vnum_char(ch,OBJ_VNUM_DECK);
      REMOVE_BIT(ante_bag->extra_flags[1],ITEM_WIZI);
      //Reset the cg_next loop, while checking to make sure there is at least 2 players left
      if (ch->pcdata->cg_next->pcdata->cg_next == ch)
	{
	  act(" There are no longer enough players for the game.  It is cancelled. ",ch,NULL,NULL,TO_ROOM);
	  send_to_char(" Due to your leaving, the game is cancelled. \n\r",ch);
	  if ((ante_bag = get_obj_vnum_char(ch->pcdata->cg_next,OBJ_VNUM_ANTE_BAG)))
	  {
	    if (ante_bag->value[6] > 0)
	      ch->pcdata->cg_next->gold += ante_bag->value[6];
	    if (ante_bag->value[5] > 0)
	      ch->pcdata->cg_next->silver += ante_bag->value[5];
	    for (obj = ante_bag->contains;obj != NULL;obj = obj_next)
	    {
	      obj_next = obj->next_content;
	      obj_from_obj(obj);
	      obj_to_char(obj,ch->pcdata->cg_next);
	    }
	    obj_from_char(ante_bag);
	    extract_obj(ante_bag);
	    send_to_char(" Your own ante has been returned to you. \n\r",ch->pcdata->cg_next);
	  }
          ante_bag = get_obj_vnum_char(ch->pcdata->cg_next,OBJ_VNUM_DECK);
	  REMOVE_BIT(ante_bag->extra_flags[1],ITEM_WIZI);
	  ch->pcdata->cg_next->pcdata->cg_state = 0;
	  ch->pcdata->cg_next->pcdata->cg_next = 0;
	}
      else
	{
	  for (rch = ch->pcdata->cg_next;rch->pcdata->cg_next != ch;rch = rch->pcdata->cg_next);
	  if (rch->pcdata->cg_next != ch)
	    {
	      bug("Fuck, somehow the cg_next is going to get fucked..I'm in ante quit",0);
	      return;
	      //ERROR
	    }
	  rch->pcdata->cg_next = ch->pcdata->cg_next;
	}
      ch->pcdata->cg_state = 0;
      ch->pcdata->cg_next = 0;
      return;
    }
  else if (!str_cmp(arg1,"list"))
    {
      obj = get_obj_vnum_char(ch,OBJ_VNUM_ANTE_BAG);
      act ("`kYour`h ante ``:",ch,NULL,rch,TO_CHAR);
      if (obj->value[5] > 0)
      {
	sprintf(buf,"     %d silver\n\r",obj->value[5]);
	send_to_char(buf,ch);
      }
      if (obj->value[6] > 0)
      {
        sprintf(buf,"     %d gold\n\r",obj->value[6]);
	send_to_char(buf,ch);
      }
      show_list_to_char (obj->contains, ch, TRUE, TRUE);
      for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
	{
	  obj = get_obj_vnum_char(rch,OBJ_VNUM_ANTE_BAG);
	  act ("`k$N's`h ante ``:",ch,NULL,rch,TO_CHAR);
	  if (obj->value[5] > 0)
	    {
	      sprintf(buf,"     %d silver\n\r",obj->value[5]);
	      send_to_char(buf,ch);
	    }
	  if (obj->value[6] > 0)
            {
              sprintf(buf,"     %d gold\n\r",obj->value[6]);
              send_to_char(buf,ch);
            }
	  //GIVE THE LIST OF ITEMS IN OBJ
	  show_list_to_char (obj->contains, ch, TRUE, TRUE);
	}
      return;
    }
  do_ante(ch,"");
}

bool pay_cost(CHAR_DATA *ch, int amount)
{
  int tint;
  bool paid = FALSE;
//  char buf[20]; //THIS SHOULD ONLY BE USED FOR NUMBERS
  OBJ_DATA *ante_bag;
  if (IS_NPC(ch))
    return FALSE;
  if (ch->silver >= amount)
    {
      ch->silver -= amount;
      paid = TRUE;
    }
  else if (ch->gold*100 + ch->silver >= amount)
    {
      amount -= ch->silver;
      ch->silver = 0;
      tint = ch->gold*100 - amount;
      ch->silver = tint%100;
      ch->gold = tint/100;
      paid = TRUE;
    }
  else if (ch->pcdata->bank_silver + ch->gold*100 + ch->silver >= amount)
    {
      amount -= ch->silver;
      ch->silver = 0;
      amount -= ch->gold*100;
      ch->gold = 0;
      ch->pcdata->bank_silver -= amount;
      paid = TRUE;
    }
  else if (ch->pcdata->bank_silver + ch->pcdata->bank_gold*100 + ch->silver + ch->gold*100 >= amount)
    {
      amount -= ch->silver;
      ch->silver = 0;
      amount -= ch->gold*100;
      ch->gold = 0;
      amount -= ch->pcdata->bank_silver;
      ch->pcdata->bank_silver = 0;
      tint = ch->pcdata->bank_gold*100 - amount;
      ch->pcdata->bank_gold = tint/100;
      ch->pcdata->bank_silver = tint%100;
      paid = TRUE;
    }
  if (paid)
  {
/*    sprintf(buf,"%d",amount);
    act("$n pays $T silver to the pot.",ch,NULL,buf,TO_ROOM);
    act("You pay $T silver to the pot.",ch,NULL,buf,TO_CHAR);*/
    if ((ante_bag = get_obj_vnum_char(ch,OBJ_VNUM_ANTE_BAG)))
      ante_bag->value[5] += amount;
    else bug ("Lost ante bag in pay cost.",0);
    return TRUE;
  }
  return FALSE;
}


bool their_turn(CHAR_DATA *ch)
{
  if (ch->pcdata->cg_state == CG_START_PM_YT
       || (ch->pcdata->cg_state >= CG_YT_MAIN && ch->pcdata->cg_state <= CG_YT_DONE))
    return TRUE;
  return FALSE;
}




void send_card_stats(OBJ_DATA *obj, CHAR_DATA *ch)
{
  char arg1[MAX_STRING_LENGTH];
  switch (obj->value[1])
    {
    case CARD_MOB:
      sprintf(arg1,"`d#%d %s`d - %s - HP: %d - Off: %d / Def: %d\n\r``",
	      obj->value[0],obj->short_descr,"Mob",obj->value[7],obj->value[8],obj->value[9]);
      break;
    case CARD_TERRAIN:
      sprintf(arg1,"`d#%d %s`d - %s - ",
	      obj->value[0],obj->short_descr,"Terrain");
      send_to_char(arg1,ch);
      sprintf(arg1,"%s - %d / %d / %d / %d",flag_string (card_terrain_type, obj->value[7]),
	      obj->value[8],obj->value[9],obj->value[10],obj->value[11]);
      send_to_char(arg1,ch);
      sprintf(arg1," - %s\n\r``",flag_string (card_terrain_toggles, obj->value[12]));

      break;
      //SPELL
    case CARD_SPELL_HEAL :
      sprintf(arg1,"`d#%d %s`d - Heal spell - Amount: %d\n\r``",obj->value[0],obj->short_descr, obj->value[7]);
      break;
    case CARD_SPELL_DAMAGE :
      sprintf(arg1,"`d#%d %s`d - Damage spell - Amount: %d\n\r``",obj->value[0],obj->short_descr, obj->value[7]);
      break;
    case CARD_SPELL_BOOST:
      sprintf(arg1,"`d#%d %s`d - Boost spell - Off: %d/ Def: %d\n\r``",obj->value[0],obj->short_descr, obj->value[7], obj->value[8]);
      break;
    case CARD_SPELL_COUNTER:
      sprintf(arg1,"`d#%d %s`d - Counter spell\n\r``",obj->value[0],obj->short_descr);
      break;
    default: sprintf(arg1,"`d#%d %s - %s\n\r``",obj->value[0],obj->short_descr,flag_string (card_Class, obj->value[1]));
    }
  send_to_char(arg1,ch);
}

void send_card_stats2(OBJ_DATA *obj, CHAR_DATA *ch, int mob_number, CHAR_DATA *mob_owner)
{
  char arg1[MAX_STRING_LENGTH];
  switch (obj->value[1])
    {
    case CARD_MOB:
      if (mob_number == 1)
        sprintf(arg1,"`d#%d %s`d - %s - HP: %d - Off: %d / Def: %d\n\r``",
              obj->value[0],obj->short_descr,"Mob",mob_owner->pcdata->cg_mm_value[CARDGAME_MOB_HP],
	      mob_owner->pcdata->cg_mm_value[CARDGAME_MOB_OFF],mob_owner->pcdata->cg_mm_value[CARDGAME_MOB_DEF]);
      else if (mob_number == 2)
        sprintf(arg1,"`d#%d %s`d - %s - HP: %d - Off: %d / Def: %d\n\r``",
              obj->value[0],obj->short_descr,"Mob",mob_owner->pcdata->cg_sm_value[CARDGAME_MOB_HP],
	      mob_owner->pcdata->cg_sm_value[CARDGAME_MOB_OFF],mob_owner->pcdata->cg_sm_value[CARDGAME_MOB_DEF]);
      break;
    }
  send_to_char(arg1,ch);
}


void mobs_scan(CHAR_DATA *ch)
{
  CHAR_DATA *rch;
  if (ch->pcdata->cg_main_mob)
    {
      send_to_char("`kYour `hmain mob : ``",ch);
      send_card_stats2(ch->pcdata->cg_main_mob,ch,1,ch);
    }
  if (ch->pcdata->cg_secondary_mob)
    {
      send_to_char("`kYour `hsecondary mob : ``",ch);
      send_card_stats2(ch->pcdata->cg_secondary_mob,ch,2,ch);
    }

  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
    {
      if (rch->pcdata->cg_main_mob)
	{
          send_to_char("`k",ch);
	  send_to_char(rch->name,ch);
	  send_to_char("'s `hmain mob : ``",ch);
	  send_card_stats2(rch->pcdata->cg_main_mob,ch,1,rch);
	}
      if (rch->pcdata->cg_secondary_mob)
	{
	  send_to_char("`k",ch);
  	  send_to_char(rch->name,ch);
	  send_to_char("'s `hsecondary mob : ``",ch);
	  send_card_stats2(rch->pcdata->cg_secondary_mob,ch,2,rch);
	}
    }
}

      
	  

  
  
	  
void appraise_list(CHAR_DATA *ch)
{
  CHAR_DATA *rch;
  char buf[MAX_STRING_LENGTH];
  int counter=2;
  send_to_char("`i[0]`hGo back to the previous menu.\n\r",ch);
  if (ch->pcdata->cg_terrain)
  {
    send_to_char("`i[1]`hTerrain : ``",ch);
    send_card_stats(ch->pcdata->cg_terrain,ch);
  }
  if (ch->pcdata->cg_main_mob)
    {
      send_to_char("`i[2]`kYour `hmain mob : ``",ch);
      send_card_stats(ch->pcdata->cg_main_mob,ch);
    }
  if (ch->pcdata->cg_secondary_mob)
    {
      send_to_char("`i[3]`kYour `hsecondary mob : ``",ch);
      send_card_stats(ch->pcdata->cg_secondary_mob,ch);
    }
  if (ch->pcdata->cg_lcp)
    {
      send_to_char("`i[4]`kYour `hspell played : ``",ch);
      send_card_stats(ch->pcdata->cg_lcp,ch);
    }

  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
    {
      counter += 3;
      if (rch->pcdata->cg_main_mob)
        {
	  sprintf(buf,"`i[%d]`k%s's `hmain mob : ``",counter,rch->name);
          //send_to_char("`k",ch);
          //send_to_char(rch->name,ch);
          send_to_char(buf,ch);
          send_card_stats(rch->pcdata->cg_main_mob,ch);
        }
      if (rch->pcdata->cg_secondary_mob)
        {
	  sprintf(buf,"`i[%d]`k%s's `hsecondary mob : ``",counter+1,rch->name);
	  //          send_to_char("`k",ch);
          //send_to_char(rch->name,ch);
          send_to_char(buf,ch);
          send_card_stats(rch->pcdata->cg_secondary_mob,ch);
        }
      if (rch->pcdata->cg_lcp)
        {
	  sprintf(buf,"`i[%d]`k%s's `hspell played : ``",counter+2,rch->name);
	  //          send_to_char("`k",ch);
          //send_to_char(rch->name,ch);
          send_to_char(buf,ch);
          send_card_stats(rch->pcdata->cg_lcp,ch);
        }
    }
}
     

bool appraise_card_number(CHAR_DATA *ch, char* argument)
{
  int card_no, counter=2;
  CHAR_DATA *rch;
  card_no = atoi(argument);
  if (card_no < 0 || card_no > 31)
    return FALSE;
  if (card_no == 0)
    return TRUE;      //This means they just want to go back a menu
  if (card_no == 1 && ch->pcdata->cg_terrain)
  {
    appraise(ch,ch->pcdata->cg_terrain);	  
    return TRUE;
  }
  if (card_no == 2 && ch->pcdata->cg_main_mob)
  {
    appraise(ch,ch->pcdata->cg_main_mob);
    return TRUE;
  }
  if (card_no == 3 && ch->pcdata->cg_secondary_mob)
  {
    appraise(ch,ch->pcdata->cg_secondary_mob);
    return TRUE;
  }
  if (card_no == 4 && ch->pcdata->cg_lcp)
    {
      appraise(ch,ch->pcdata->cg_lcp);
      return TRUE;
    }

  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
    {
      counter += 3;
      if (card_no == counter && rch->pcdata->cg_main_mob)
	{
	  appraise(ch,rch->pcdata->cg_main_mob);
	  return TRUE;
	}
      if (card_no == counter+1 && rch->pcdata->cg_secondary_mob)
	{
	  appraise(ch,rch->pcdata->cg_secondary_mob);
	  return TRUE;
	}
      if (card_no == counter+2 && rch->pcdata->cg_lcp)
	{
	  appraise(ch,rch->pcdata->cg_lcp);
	  return TRUE;
	}
    }
  return FALSE;
}

     
void appraise_all(CHAR_DATA *ch, OBJ_DATA *card)
{
  CHAR_DATA *rch;
  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
  {
    send_to_char("  ",rch);
    send_card_stats(card,rch);
  }
}


void do_cardkill (CHAR_DATA * ch, char *argument)
{
  if (IS_NPC (ch))
    return;
  /*  if (ch->pcdata->learned[gsn_butcher] < 1)
    {
      send_to_char
        ("You can't set autobutcher without the butcher skill!", ch);
      return;
      }*/
  if (IS_SET (ch->act, PLR_CARDKILL))
    {
      send_to_char ("Card Killing removed.\n\r", ch);
      REMOVE_BIT (ch->act, PLR_CARDKILL);
    }

  else
    {
      send_to_char ("Card Killing set.\n\r", ch);
      SET_BIT (ch->act, PLR_CARDKILL);
    }
}

RUNE_CARD_DATA *rune_card_list = NULL;
void do_runecard(CHAR_DATA* ch, char* argument)
{
  RUNE_CARD_DATA *nrc, *rc, *trc;
  char arg[MAX_INPUT_LENGTH];
  int vnum;
  //  OBJ_INDEX_DATA *oid;
  //OBJ_DATA *obj;
  argument = one_argument(argument,arg);
  /* if (!is_number(argument))
  {
    send_to_char("Invalid vnum!\n\r",ch);
    return;
  }
  obj = create_object(oid,0);
  if (obj->type != ITEM_CARD)
  {
    send_to_char("That object is not a card!\n\r",ch);
    return;
  }
  nrc = new_rune_card;
  nrc->next = NULL;
  nrc->vnum = vnum;*/
  if (!str_cmp(arg,"add"))
  {
    OBJ_INDEX_DATA *oid;
    OBJ_DATA *obj;
    if (!is_number(argument) || ((vnum = atoi(argument)) < 1) || (!(oid = get_obj_index(vnum))))
      {
	send_to_char("Invalid vnum!\n\r",ch);
	return;
      }
    obj = create_object(oid,0);
    if (obj->item_type != ITEM_CARD)
      {
	send_to_char("That object is not a card!\n\r",ch);
	return;
      }
    nrc = new_rune_card();
    nrc->next = NULL;
    nrc->vnum = vnum;

    if (!rune_card_list)
    {
      rune_card_list = nrc;
      send_to_char("Vnum added to rune card list.\n\r",ch);
      save_rune_cards();
      return;
    }
    if (rune_card_list->next == NULL || nrc->vnum < rune_card_list->vnum)
    { 
      if (nrc->vnum < rune_card_list->vnum)
	{
	  nrc->next = rune_card_list;
	  rune_card_list = nrc;
	}
      else
	{
	  rune_card_list->next = nrc;
	}
      send_to_char("Vnum added to rune card list.\n\r",ch);
      save_rune_cards();
      return;
    }
    for (rc = rune_card_list; rc->next != NULL; rc = rc->next)
      {
	if (nrc->vnum < rc->next->vnum)
	  {
	    if (rc->vnum == nrc->vnum)
	    {
	      send_to_char("That vnum is already in there, dummy!\n\r",ch);
	      return;
	    }
	    nrc->next = rc->next;
	    rc->next = nrc;
	    send_to_char("Vnum added to rune card list.\n\r",ch);
	    return;
	  }
      }
    rc->next = nrc;
    send_to_char("Vnum added to rune card list.\n\r",ch);
    save_rune_cards();
    return;
  }
  else if (!str_cmp(arg,"remove"))
    {
      vnum = atoi(argument);
      if (!rune_card_list)
	{
	  send_to_char("That vnum is not in the rune card list.\n\r",ch);
	  return;
	}
      if (rune_card_list->next == NULL || rune_card_list->vnum == vnum)
	{
	  if (rune_card_list->vnum == vnum)
	    {
	      trc = rune_card_list;
	      rune_card_list = rune_card_list->next;
	      trc->next = NULL;
	      free_rune_card(trc);
	      send_to_char("Vnum successfully removed.\n\r",ch);
	      save_rune_cards();
	      return;
	    }
	  send_to_char("That vnum is not in the rune card list.\n\r",ch);
          return;
	}
      for (rc = rune_card_list; rc->next != NULL; rc = rc->next)
	{
	  if (vnum == rc->next->vnum)
	    {
	      trc = rc->next;
	      rc->next = rc->next->next;
	      trc->next = NULL;
	      free_rune_card(trc);
	      send_to_char("Vnum successfully removed.\n\r",ch);
	      save_rune_cards();
              return;
	    }
	}
      send_to_char("That vnum is not in the rune card list.\n\r",ch);
      return;
    }
  else if (!str_cmp(arg,"list"))
    {
      int counter=0;
      send_to_char("-----Rune Card Vnum Listing-----\n\r",ch);
      if (!rune_card_list)
	{
	  send_to_char("   (empty)  \n\r",ch);
	  return;
	}
      else
	{
	  sprintf(arg,"%8d",rune_card_list->vnum);
	  send_to_char(arg,ch);
	  ++counter;
	}
/*      if (!(rune_card_list->next))
	{
	  send_to_char("\n\r",ch);
	  return;
	}
      else
	{
	  sprintf(arg,"%8d",rune_card_list->vnum);
          send_to_char(arg,ch);
	  ++counter;
        }*/
      for (rc = rune_card_list; rc->next != NULL; rc = rc->next)
	{
	  ++counter;
	  sprintf(arg,"%8d",rc->next->vnum);
          send_to_char(arg,ch);
	  if (counter%3==0)
	    send_to_char("\n\r",ch);
        }
      if (counter%3 != 0)
	send_to_char("\n\r",ch);
    }
  else
    {
      send_to_char("Syntax : Runecard add VNUM\n\r         Runecard remove VNUM\n\r         Runecard list\n\r",ch);
      return;
    }
}

void save_rune_cards (void)
{
  FILE *fp;
  RUNE_CARD_DATA *rc;
  bool found = FALSE;
  int counter=0;
  fclose (fpReserve);
  if ((fp = fopen (RUNE_CARD_FILE, "w")) == NULL)
    {
      perror (RUNE_CARD_FILE);
    }
  for (rc = rune_card_list; rc != NULL; rc = rc->next)
    {
      found = TRUE;
      ++counter;
      fprintf (fp, "%d\n", rc->vnum);
    }
  fclose (fp);
  fpReserve = fopen (NULL_FILE, "r");
  if (!found)
    unlink (RUNE_CARD_FILE);
}


void load_rune_cards ()
{
  FILE *fp;
  RUNE_CARD_DATA *blist;
  if ((fp = fopen (RUNE_CARD_FILE, "r")) == NULL)
    return;
  blist = NULL;
  for (;;)
    {
      RUNE_CARD_DATA *rc;
      if (feof (fp))
        {
          fclose (fp);
          return;
        }
      rc = new_rune_card ();
      rc->vnum = fread_number (fp);
      fread_to_eol (fp);
      if (rune_card_list == NULL)
        rune_card_list = rc;

      else
        blist->next = rc;
      blist = rc;
    }
}

int get_obj_number2 (OBJ_DATA * obj)
{
  int number;
  if (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_PACK
      || obj->item_type == ITEM_MONEY || obj->item_type == ITEM_GEM
      || obj->item_type == ITEM_JEWELRY
      || obj->item_type == ITEM_CLAN_DONATION
      || obj->item_type == ITEM_PLAYER_DONATION
      || obj->item_type == ITEM_BINDER)
    number = 0;

  else
    number = 1;
  if (obj->item_type == ITEM_QUIVER)
    return 1;
  for (obj = obj->contains; obj != NULL; obj = obj->next_content)
    {
      if (obj == obj->next_content)
        break;
      number += get_obj_number2 (obj);
    }
  return number;
}

void table_scan(CHAR_DATA *ch)
{
  CHAR_DATA *rch;
  if (ch->pcdata->cg_terrain)
    {
      send_to_char("`hTerrain : ``",ch);
      send_card_stats(ch->pcdata->cg_terrain,ch);
    }
  if (ch->pcdata->cg_main_mob)
    {
      send_to_char("`kYour `hmain mob : ``",ch);
      send_card_stats(ch->pcdata->cg_main_mob,ch);
    }
  if (ch->pcdata->cg_secondary_mob)
    {
      send_to_char("`kYour `hsecondary mob : ``",ch);
      send_card_stats(ch->pcdata->cg_secondary_mob,ch);
    }
  if (ch->pcdata->cg_lcp)
    {
      send_to_char("`kYour `hspell played : ``",ch);
      send_card_stats(ch->pcdata->cg_lcp,ch);
    }

  for (rch = ch->pcdata->cg_next;rch != ch;rch = rch->pcdata->cg_next)
    {
      if (rch->pcdata->cg_main_mob)
        {
          send_to_char("`k",ch);
          send_to_char(rch->name,ch);
          send_to_char("'s `hmain mob : ``",ch);
          send_card_stats(rch->pcdata->cg_main_mob,ch);
        }
      if (rch->pcdata->cg_secondary_mob)
        {
          send_to_char("`k",ch);
          send_to_char(rch->name,ch);
          send_to_char("'s `hsecondary mob : ``",ch);
          send_card_stats(rch->pcdata->cg_secondary_mob,ch);
        }
      if (rch->pcdata->cg_lcp)
        {
          send_to_char("`k",ch);
          send_to_char(rch->name,ch);
          send_to_char("'s `hspell played : ``",ch);
          send_card_stats(rch->pcdata->cg_lcp,ch);
        }
    }
}


void save_cardcount (void)
{
  FILE *fp;
  //  BOUNTY_DATA *bounty;
  bool found = FALSE;
  fclose (fpReserve);
  if ((fp = fopen (CARDCOUNT_FILE, "w")) == NULL)
    {
      perror (CARDCOUNT_FILE);
    }
  /*  for (bounty = bounty_list; bounty != NULL; bounty = bounty->next)
    {
      found = TRUE;
      fprintf (fp, "%-12s %d\n", bounty->name, bounty->amount);
      }*/
  found = TRUE;
  fprintf (fp, "%d",amount_of_cards);
  fclose (fp);
  fpReserve = fopen (NULL_FILE, "r");
  if (!found)
    unlink (CARDCOUNT_FILE);
}

void load_cardcount ()
{
  FILE *fp;
  if ((fp = fopen (CARDCOUNT_FILE, "r")) == NULL)
    return;
   amount_of_cards = fread_number (fp);
   fclose (fp);
   return;
}
    
	
  //reset the cg_next links
  //check to see if only one player is left, if so they win
  //terrainify their mobs
  //Deal with ante stuff (much later)
	
