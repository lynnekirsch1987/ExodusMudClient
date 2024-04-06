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
#include "interp.h"
#include "recycle.h"
#include "clans/new_clans.h"

//trade_skills.c
//Iblis - Created 4/05/04
//Initial skills taken from archery.c

//from archery.c
bool damage_object args ((OBJ_DATA * obj, short amount));

//from necro.c
void corpse_carve args((CHAR_DATA* ch, char* argument));

//from skills.c
void improve_skill args((CHAR_DATA * ch, int sn));

//prototypes
char *arrowhead_name args ((char *material));



//Iblis - Attach command, for usage with the fletchery skill
void do_attach (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *feather, *shaft, *arrowhead, *arrow;

  char buf[MAX_STRING_LENGTH], arg[MAX_INPUT_LENGTH];
  if (ch->position != POS_SITTING || ch->fighting != NULL)
    {
      send_to_char
        ("You must be sitting in order to concentrate enough to attach everything together.\n\r",
         ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax: `kattach feather arrowhead shaft``\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg);
  if ((feather = get_obj_carry (ch, arg)) == NULL)
    {
      send_to_char ("You don't seem to be carrying that feather.\n\r", ch);
      return;
    }
  if (feather->item_type != ITEM_FEATHER)
    {
      act ("$p is not a feather.", ch, feather, NULL, TO_CHAR);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax: `kattach feather arrowhead shaft``\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg);
  if ((arrowhead = get_obj_carry (ch, arg)) == NULL)
    {
      send_to_char ("You don't seem to be carrying that arrowhead.\n\r", ch);
      return;
    }
  if (arrowhead->item_type != ITEM_WEAPON
      || arrowhead->value[0] != WEAPON_ARROWHEAD)
    {
      act ("$p is not an arrowhead.", ch, arrowhead, NULL, TO_CHAR);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax: `kattach feather arrowhead shaft``\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg);
  if ((shaft = get_obj_carry (ch, arg)) == NULL)
    {
      send_to_char ("You don't seem to be carrying that shaft.\n\r", ch);
      return;
    }
  if (shaft->pIndexData->vnum != OBJ_VNUM_SHAFT)
    {
      act ("$p is not a shaft.", ch, shaft, NULL, TO_CHAR);
      return;
    }
  //DEPENDING ON THE ARROWHEAD, MAKE AN ARROW
  arrow = create_object (get_obj_index (OBJ_VNUM_ARROW), 0);

  //THESE VALUES COULD DEPEND ON SHAFT/FEATHER/ARROWHEAD type
  arrow->value[1] = arrowhead->value[1];
  arrow->value[2] = arrowhead->value[2];
  arrow->value[3] = arrowhead->value[3];
  arrow->value[4] = arrowhead->value[4];
  arrow->condition =
    (shaft->condition + feather->condition + arrowhead->condition) / 3.0;
  sprintf (buf, "%s arrow with %s", arrowhead_name (arrowhead->material),
           feather->short_descr);
  free_string (arrow->short_descr);
  arrow->short_descr = str_dup (buf);
  obj_from_char (feather);
  extract_obj (feather);
  obj_from_char (shaft);
  extract_obj (shaft);
  obj_from_char (arrowhead);
  extract_obj (arrowhead);
  obj_to_char (arrow, ch);
  if (number_percent () < get_skill (ch, gsn_fletchery))
    {
      act ("You have successfully crafted $p!", ch, arrow, NULL, TO_CHAR);
      act ("$n has successfully crafted $p!", ch, arrow, NULL, TO_ROOM);
      check_improve (ch, gsn_fletchery, TRUE, 1);
    }
  else
    {
      if (damage_object (arrow, 50 + (100 - gsn_fletchery)))
        {
          act
            ("You push a little too hard and crack the shaft of $p, breaking it in half.",
             ch, arrow, NULL, TO_CHAR);
          act
            ("$n pushes a little too hard and cracks the shaft of $p, breaking it in half.",
             ch, arrow, NULL, TO_CHAR);
          obj_from_char (arrow);
          extract_obj (arrow);
        }
      else
        {
          act ("You push a little too hard and crack the shaft of $p.",
               ch, arrow, NULL, TO_CHAR);
          act ("$n pushes a little too hard and cracks the shaft of $p.",
               ch, arrow, NULL, TO_CHAR);
          act ("You have semi-successfully crafted $p!", ch, arrow, NULL,
               TO_CHAR);
          act ("$n has semi-successfully crafted $p!", ch, arrow, NULL,
               TO_ROOM);
        }
    }
}

//Iblis - A small function that returns the name of an arrowhead based on the material its made from
char *arrowhead_name (char *material)
{
  static char rvalue[MAX_INPUT_LENGTH];
  if (!str_cmp (material, "steel"))
    {
      strcpy (rvalue, "a `gsteel-tipped``");
    }
  else if (!str_cmp (material, "wood"))
    {
      strcpy (rvalue, "a `dbrown-tipped``");
    }
  else if (!str_cmp (material, "brass"))
    {
      strcpy (rvalue, "a `kbronze-tipped``");
    }
  else if (!str_cmp (material, "copper"))
    {
      strcpy (rvalue, "a `dcopper-tipped``");
    }
  else if (!str_cmp (material, "metal"))
    {
      strcpy (rvalue, "a `ametal-tipped``");
    }
  else if (!str_cmp (material, "silver"))
    {
      strcpy (rvalue, "a silver-tipped");
    }
  else if (!str_cmp (material, "tin"))
    {
      strcpy (rvalue, "a `ametal-tipped``");
    }
  else if (!str_cmp (material, "horn"))
    {
      strcpy (rvalue, "a `osparkling white-tipped``");
    }
  else if (!str_cmp (material, "gold"))
    {
      strcpy (rvalue, "a `kgold-tipped``");
    }
  else if (!str_cmp (material, "shell"))
    {
      strcpy (rvalue, "a `hwhite-tipped``");
    }
  else if (!str_cmp (material, "pottery"))
    {
      strcpy (rvalue, "a `eblue-tipped``");
    }
  else if (!str_cmp (material, "clay"))
    {
      strcpy (rvalue, "a `bred-tipped``");
    }
  else if (!str_cmp (material, "platinum"))
    {
      strcpy (rvalue, "a platinum-tipped");
    }
  else if (!str_cmp (material, "flint"))
    {
      strcpy (rvalue, "a `6pitch black-tipped``");
    }
  else
    strcpy (rvalue, "a non-descript");

  return rvalue;
}

//Iblis - Carve command, for use with the woodworking skill
void do_carve (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  char *cptr, *cptr2;
  OBJ_DATA *obj, *weapon, *obj2;
  int i=0;
  if (IS_CLASS(ch,PC_CLASS_NECROMANCER))
    {
      corpse_carve(ch,argument);
      return;
    }
  if (ch->position != POS_SITTING || ch->fighting != NULL)
    {
      send_to_char
        ("You must be sitting comfortably in order to do intricate carvings.\n\r",
         ch);
      return;
    }
  weapon = get_eq_char (ch, WEAR_WIELD_R);
  if (weapon == NULL || (weapon->value[0] != WEAPON_DAGGER))
    weapon = get_eq_char (ch, WEAR_WIELD_L);
  if (weapon == NULL || (weapon->value[0] != WEAPON_DAGGER))
    {
              send_to_char
                ("You need to be wielding a dagger to properly carve.\n\r",
                 ch);
              return;
    }

  argument = one_argument (argument, arg);
  argument = one_argument (argument, arg2);
  if (get_skill (ch, gsn_woodworking) < 1)
    {
      send_to_char ("You know nothing about carving.\n\r", ch);
      return;
    }
  if ((obj = get_obj_carry (ch, arg2)) == NULL)
    {
      send_to_char ("You do not have that item.\n\r", ch);
      return;
    } 
  if (obj->pIndexData->vnum == OBJ_VNUM_STICKS)
    {
      if (!str_cmp (arg, "shaft"))
        {
          for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
            if (obj->pIndexData->vnum == OBJ_VNUM_STICKS)
              break;
          if (obj == NULL)
            {
              send_to_char
                ("You do not have any wood left to carve.\n\r", ch);
              return;
            }
          obj_from_char (obj);
          extract_obj (obj);
          obj = create_object (get_obj_index (OBJ_VNUM_SHAFT), 0);
          obj_to_char (obj, ch);
          obj->condition =
            number_range (get_skill (ch, gsn_woodworking), 100);
          check_improve (ch, gsn_woodworking, TRUE, 1);
          if (obj->condition > 90)
            send_to_char
              ("You have successfully carved a stick into a proper shaft.\n\r",
               ch);

          else if (obj->condition > 50)
 send_to_char
   ("You have semi-successfully carved a stick into a proper shaft.\n\r",
    ch);

          else
            send_to_char
              ("You have successfully carved a stick into something slightly resembling a shaft.\
n\r",
               ch);
          return;
        }
      if (!str_cmp (arg, "arrowhead"))
        {
          for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
            if (obj->pIndexData->vnum == OBJ_VNUM_STICKS)
              break;
          if (obj == NULL)
            {
              send_to_char
                ("You do not have any wood left to carve.\n\r", ch);
              return;
            }
          weapon = get_eq_char (ch, WEAR_WIELD_R);
          if (weapon == NULL || (weapon->value[0] != WEAPON_DAGGER))
            weapon = get_eq_char (ch, WEAR_WIELD_L);
          if (weapon == NULL || (weapon->value[0] != WEAPON_DAGGER))
            {
              send_to_char
                ("You need to be wielding a dagger to properly carve.\n\r",
                 ch);
              return;
            }
          obj_from_char (obj);
          extract_obj (obj);
          obj = create_object (get_obj_index (OBJ_VNUM_WOODEN_ARROWHEAD), 0);
          obj_to_char (obj, ch);
          obj->condition =
            number_range (get_skill (ch, gsn_woodworking), 100);
          check_improve (ch, gsn_woodworking, TRUE, 1);
          if (obj->condition > 90)
            send_to_char
              ("You have successfully carved a stick into a wooden arrowhead.\n\r",
               ch);

          else if (obj->condition > 50)
            send_to_char
              ("You have semi-successfully carved a stick into a proper arrowhead.\n\r",
               ch);

          else
            send_to_char
              ("You have successfully carved a stick into something slightly resembling an arrowhead.\n\r",
               ch);
          return;
        }
    }
  else if (obj->item_type == ITEM_WOOD)
  {
    if (IS_NPC(ch))
    {
      send_to_char("Bad NPC, NO!\n\r",ch);
      return;
     }
    if (!str_cmp(arg,"figurine"))
    {

      obj2 = create_object(get_obj_index (OBJ_VNUM_WOOD_OBJ), 0);
      obj2->value[0] = obj->value[0]; //Type of wood object is made from
      switch (obj2->value[0])
      {
	      case TREE_TYPE_MOONBLOSSOM:
	      case TREE_TYPE_JEWELWOOD: obj2->cost = 10000;break;
	      case TREE_TYPE_RISIRIEL: obj2->cost = 8000;break;
	      case TREE_TYPE_IRONWOOD: obj2->cost = 5000;break;
	      case TREE_TYPE_SLIMEWOOD:
	      case TREE_TYPE_HELLROOT:
	      case TREE_TYPE_BARBVINE:
	      case TREE_TYPE_GREEN_LEECHLEAF: obj2->cost = 2000;break;
	      default : obj2->cost = 1000;break;
      }
      sprintf(buf,"`dA wooden '`h`d' made of %s wood is here.``",tree_table[obj2->value[0]].name);
      free_string(obj2->description);
      obj2->description = str_dup(buf);
      sprintf(buf,"`da wooden '`h`d' made of %s wood``",tree_table[obj2->value[0]].name);
      free_string(obj2->short_descr);
      obj2->short_descr = str_dup(buf);
      sprintf(buf,"figurine %s",tree_table[obj2->value[0]].name);
      free_string(obj2->name);
      obj2->name = str_dup(buf);
      obj2->weight = obj->weight*.66;
      obj2->value[5] = -1;
      obj_to_char(obj2,ch);
      obj->condition = number_range (get_skill (ch, gsn_woodworking), 100);
      check_improve (ch, gsn_woodworking, TRUE, 1);
      WAIT_STATE(ch,30);
      act("You begin an intricate carving from $p.",ch,obj,NULL,TO_CHAR);
      act("$n begins to intricate carving from $p.",ch,obj,NULL,TO_ROOM);      
      extract_obj(obj);
      return;
    }

/*    if (!str_cmp(arg,"plaque"))
      {

	obj2 = create_object(get_obj_index (OBJ_VNUM_PLAQUE), 0);
	obj2->value[0] = obj->value[0]; //Type of wood object is made from
	switch (obj2->value[0])
	  {
	  case TREE_TYPE_MOONBLOSSOM:
	  case TREE_TYPE_JEWELWOOD: obj2->cost = 10000;break;
	  case TREE_TYPE_RISIRIEL: obj2->cost = 8000;break;
	  case TREE_TYPE_IRONWOOD: obj2->cost = 5000;break;
	  case TREE_TYPE_SLIMEWOOD:
	  case TREE_TYPE_HELLROOT:
	  case TREE_TYPE_BARBVINE:
	  case TREE_TYPE_GREEN_LEECHLEAF: obj2->cost = 2000;break;
	  default : obj2->cost = 1000;break;
	  }
	sprintf(buf,obj2->short_descr,tree_table[obj2->value[0]].name);
	free_string(obj2->short_descr);
	obj2->short_descr = str_dup(buf);
	sprintf(buf,"plaque %s",tree_table[obj2->value[0]].name);
	free_string(obj2->name);
	obj2->name = str_dup(buf);
	obj2->weight = obj->weight*.66;
	obj2->timer = obj->weight*200;
	//value[5] will be used to store the number of letters in the object
	//I set it to -1 here as a sort of sanity check for "carve figure letter"
	obj2->value[5] = -1;
	obj_to_char(obj2,ch);
	obj->condition = number_range (get_skill (ch, gsn_woodworking), 100);
	check_improve (ch, gsn_woodworking, TRUE, 1);
	WAIT_STATE(ch,30);
	act("You begin an intricate carving from $p.",ch,obj,NULL,TO_CHAR);
	act("$n begins to intricate carving from $p.",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
	return;
      }*/
      

    if (!str_cmp(arg,"chair"))
    {
      one_argument(argument,arg);
      
      if ((obj2 = get_obj_carry (ch, arg)) == NULL)
      {
        send_to_char ("You do not have that item.\n\r", ch);
        return;
      }
      if (obj == obj2)
      {
        send_to_char ("You can't carve a chair with just one piece of wood!\n\r",ch);
        send_to_char ("(Try carve chair wood 2.wood)\n\r",ch);
        return;
      }
      if (obj2->item_type != ITEM_WOOD)
      {
        send_to_char ("You need two pieces of wood to carve a chair.  Those are not both wood.\n\r",ch);
	return;
      }
      ch->position = POS_CARVING; 
      weapon = create_object(get_obj_index(OBJ_VNUM_WOOD_DEFAULT),0);
      weapon->value[0] = WOODEN_OBJ_CHAIR;
      weapon->value[1] = obj->value[0];
      weapon->value[2] = obj2->value[0];
      weapon->value[3] = 1;//This is where the progress is stored
      if (weapon->value[1] != weapon->value[2])
        sprintf (buf, "a partially constructed %s and %s chair", tree_table[weapon->value[1]].name, tree_table[weapon->value[2]].name);
      else  sprintf (buf, "a partially constructed %s chair", tree_table[weapon->value[1]].name);
      free_string (weapon->short_descr);
      weapon->short_descr = str_dup (buf);
      if (weapon->value[1] != weapon->value[2])
        sprintf (buf, "A partially constructed %s and %s chair is here.", tree_table[weapon->value[1]].name, tree_table[weapon->value[2]].name);
      else sprintf (buf, "A partially constructed %s chair is here.", tree_table[weapon->value[1]].name);
      free_string (weapon->description);
      weapon->description = str_dup (buf);
      free_string (weapon->name);
      weapon->name = str_dup ("wooden chair");      
      obj_to_char(weapon,ch);
      sprintf (buf, "You combine %s and %s into %s and begin carving.\n\r",obj->short_descr,obj2->short_descr,weapon->short_descr);
      send_to_char(buf,ch);
      sprintf (buf, "$n combines %s and %s into %s and begins carving.",obj->short_descr,obj2->short_descr,weapon->short_descr);
      act(buf,ch,NULL,NULL,TO_ROOM);
      obj_from_char(obj);
      obj_from_char(obj2);
      extract_obj(obj);
      extract_obj(obj2);
      ch->pcdata->trade_skill_obj = weapon;
      return;		 

    }
    else if (!str_cmp(arg,"cup"))
    {
      ch->position = POS_CARVING;
      weapon = create_object(get_obj_index(OBJ_VNUM_WOOD_DEFAULT),0);
      weapon->value[0] = WOODEN_OBJ_CUP;
      weapon->value[1] = obj->value[0];
      weapon->value[3] = 1;//This is where the progress is stored
      sprintf(buf,"`dA partially constructed cup made of %s wood is here.``",tree_table[weapon->value[1]].name);
      free_string(weapon->description);
      weapon->description = str_dup(buf);
      sprintf(buf,"`da partially constructed cup made of %s wood``",tree_table[weapon->value[1]].name);
      free_string(weapon->short_descr);
      weapon->short_descr = str_dup(buf);
      sprintf(buf,"partially %s cup",tree_table[weapon->value[1]].name);
      free_string(weapon->name);
      weapon->name = str_dup(buf); 
      obj_to_char(weapon,ch);
      sprintf(buf, "You begin carving %s into %s.\n\r",obj->short_descr,weapon->short_descr);
      send_to_char(buf,ch);
      sprintf(buf, "$n begins carving %s into %s.",obj->short_descr,weapon->short_descr);
      act(buf,ch,NULL,NULL,TO_ROOM);
      obj_from_char(obj);     
      extract_obj(obj);
      ch->pcdata->trade_skill_obj = weapon;
      return;
    }
      
  }
  else if (!str_cmp(arg,"continue"))
    {  
       if (obj->item_type != ITEM_WOODEN_INCONSTRUCTION)
       {
         send_to_char("You can't continue to carve that!\n\r",ch);
	 return;
       }
       ch->pcdata->trade_skill_obj = obj;
       ch->position = POS_CARVING;
       act("$n continues $s carving of $p.",ch,ch->pcdata->trade_skill_obj,NULL,TO_ROOM);
       act("You continue your carving of $p.",ch,ch->pcdata->trade_skill_obj,NULL,TO_CHAR);
       return;
    }
  else if (!str_cmp(arg,"letter"))
      {

	if (arg2[0] == '\0')
	{
   	  send_to_char("Carve what letter?\n\r",ch);
	  return;
	}
	if (arg2[0] == '\'') //cannot carve this char since its used for determining location
	{
  	  send_to_char("You cannot carve that character.\n\r",ch);
	  return;
	}
	//temporary check for safety
	if (obj->value[5] != -1)
	{
	  send_to_char("You cannot carve letters in that!\n\r",ch);
	  return;
	}
	//do shit to pull characters from in between ' and ' in the short desc
	cptr=strchr(obj->short_descr,'\'');
	if (strlen(cptr) > 3)	
	{
  	  cptr += 2; //to get past `h
  	  cptr2=strchr(cptr+1,'\'');
	}
	else cptr2 = NULL;
	if (cptr == NULL || cptr2 == NULL)
	{
	  send_to_char("You cannot carve letters in that!\n\r",ch);
	  return;
	}
/*	if (str_len(cptr) < cptr-cptr2)
	{
	  send_to_char("You cannot carve letters in that!\n\r",ch);
	  return;
	}*/
	//Otherwise here we go, lets hope no one modified the short or the value[5] of the obj
	for (i=0;i < cptr2-cptr-3;i++)
  	  arg[i] = cptr[i+1];
	if (i > 20)
	{
		send_to_char("Sorry, that figurine has too many letters already.\n\r",ch);
		return;
	}
	if (number_percent()-1 < get_skill(ch,gsn_woodworking))
	{
	  arg[i] = argument[0];
	  if (argument[0] == '_')
	    arg[i] = ' ';
	}
	else arg[i] = (char) number_range(0,25)+65+(number_percent()>=50?32:0);
	arg[i+1] = '\0';
	if (obj->pIndexData->vnum != OBJ_VNUM_PLAQUE)
	  sprintf(buf,"`dA wooden '`h%s`d' made of %s wood is here.``",arg,tree_table[obj->value[0]].name);
	else  sprintf(buf,"`dA wooden plaque is here, displaying the message '`h%s`d'``",arg);

	free_string(obj->description);
	obj->description = str_dup(buf);
	if (obj->pIndexData->vnum != OBJ_VNUM_PLAQUE)
	  {
	    sprintf(buf,"`da wooden '`h%s`d' made of %s wood``",arg,tree_table[obj->value[0]].name);
	    free_string(obj->short_descr);
	    obj->short_descr = str_dup(buf);
	    sprintf(buf,"figurine %s %s",tree_table[obj->value[0]].name, arg);
	    free_string(obj->name);
	    obj->name = str_dup(buf);
	  }
	WAIT_STATE(ch,30);
	act("You begin an intricate carving of $p.",ch,obj,NULL,TO_CHAR);
	act("$n beings to intricate carving on $p.",ch,obj,NULL,TO_ROOM);
	return;
      }
 
  send_to_char
    ("You can't carve like that! Try \"`kcarve shaft stick``\"\n\r", ch);
  send_to_char ("  `kcarve arrowhead stick``\n\r", ch);
  send_to_char ("  `kcarve figurine wood``\n\r",ch);
  send_to_char ("  `kcarve letter figurine ASINGLELETTER``\n\r",ch);
  send_to_char ("  `kcarve chair wood1 wood2``\n\r",ch);
  send_to_char ("  `kcarve cup wood``\n\r",ch);  
  send_to_char ("  `kcarve continue OBJ``\n\r",ch);
 return;
}



//Iblis - The update function for trade skills, called in update.c
void update_trade_skills()
{
  DESCRIPTOR_DATA *d;
  OBJ_DATA* tree,*obj;
  CHAR_DATA* fch;
  char buf[MAX_STRING_LENGTH];
  int chance;
  for (d = descriptor_list;d != NULL;d = d->next)
  {
    if (d->connected != CON_PLAYING || !d->character->in_room || !d->character->pcdata)
	    continue;
    if (d->character->position == POS_CHOPPING)
      {
	if ((chance = get_skill(d->character,gsn_lumberjacking)) < 1)
	  {
	      send_to_char("You realize you can't remember how to chop anymore, so you stop.\n\r",d->character);
	      d->character->position = POS_STANDING;
	      continue;
	  }
	if ((tree = get_obj_vnum_room(d->character->in_room,OBJ_VNUM_TREE)) == NULL 
	    || IS_SET(tree->value[4],TREE_VALUE_FALLEN))
	  {
	    d->character->position = POS_STANDING;
	    send_to_char("You realize there is no longer a need to chop.\n\r",d->character);	   
	    continue;
	  }
	
        if (d->character->pcdata->condition[COND_DRUNK] > 10)
          chance /= 2*(d->character->pcdata->condition[COND_DRUNK]/10);
	if (weather_info.sky == SKY_RAINING)
  	  chance = chance*.80;
	else if (weather_info.sky == SKY_LIGHTNING)
  	  chance = chance*.60;
	if (get_curr_stat (d->character,STAT_STR) > 18)
  	  chance += (get_curr_stat (d->character,STAT_STR) - 18)*3;
	else if (get_curr_stat (d->character,STAT_STR) < 18)
  	  chance -= (18 - get_curr_stat (d->character,STAT_STR))*2;
	if (d->character->pcdata->primary_hand == HAND_LEFT)
	  {
	    if ((obj = get_eq_char (d->character, WEAR_WIELD_L)) == NULL)
	      {
		obj = get_eq_char (d->character, WEAR_WIELD_R);
		//Since its not their primary hand
		chance /= 2;
	      }
	  }
	else
	  {
	  if ((obj = get_eq_char (d->character, WEAR_WIELD_R)) == NULL)
	    {
	      if (d->character->pcdata->primary_hand == HAND_RIGHT)
		chance /= 2;
	      obj = get_eq_char (d->character, WEAR_WIELD_L);
	    }
	  }
	if (obj == NULL)
	  {
	    send_to_char("You realize you can't continue chopping without a weapon, so you stop.\n\r",d->character);
	    d->character->position = POS_STANDING;
	    continue;
	  }
	chance += obj->weight/10;
	switch (obj->value[0])
	  {
	  case WEAPON_AXE: chance += 5;break;
	  case WEAPON_SWORD:break;
	  case WEAPON_SHORTBOW:
	  case WEAPON_LONGBOW:
	  case WEAPON_ARROW:
	  case WEAPON_ARROWHEAD:  
	    send_to_char("You realize what you are holding is useless in chopping, so you stop.\n\r",d->character);
	    d->character->position = POS_STANDING;
	    continue;
	  default: chance /= 5;break;
	  }
	switch(attack_table[obj->value[3]].damage)
	  {
	  case DAM_SLASH : chance += 2;break;
	  case DAM_BASH : chance /= 10;break;
	  default : chance /= 4;break;
	  }
	if (!str_cmp(attack_table[obj->value[3]].noun,"chop"))
	  chance += 5;
	if (IS_SET(obj->value[4],WEAPON_SHARP))
	  chance += 5;
	if (IS_IMMORTAL(d->character))
	{			
	  sprintf(buf,"Your chance is %d.\n\r",chance);
	  send_to_char(buf,d->character);
	}
        if (number_percent() < chance)
	  {
	    ++tree->value[3];
	    if (tree->value[3] >= 1+(tree->value[2]*2)+((number_percent() > 50)?1:0))
	      {
		tree->value[1] = 0;
		++d->character->pcdata->total_trees_chopped;
		if (d->character->pcdata->total_trees_chopped == 1000 
				&& d->character->pcdata->learned[gsn_lumberjacking] < 1)
		{
			send_to_char("You have now mastered the art of lumberjacking!\n\r",d->character);
			d->character->pcdata->learned[gsn_lumberjacking] = 1;
		}
			
		do_yell(d->character,"TIMBER!");
		act("$p starts to lean, and as $n takes one last chop, it falls to the ground.",
		    d->character,tree,NULL,TO_ROOM);
		act("$p starts to lean, and as you take one last chop, it falls to the ground.",
		    d->character,tree,NULL,TO_CHAR);
		d->character->position = POS_STANDING;
		check_improve (d->character, gsn_lumberjacking, TRUE, tree->value[2]);
		SET_BIT(tree->value[4],TREE_VALUE_FALLEN);
		tree->value[3] = tree->value[2]*3 + ((number_percent()<50)?1:0);
		switch (tree->value[2])
		  {
		  case TREE_SAPLING:
		    sprintf(buf,"An %s sapling lies here.",tree_table[tree->value[0]].name);
		    tree->value[3] = 0;
		    break;
		  case TREE_SMALL:
		    sprintf(buf,"A small %s tree lies here.",tree_table[tree->value[0]].name);
		    break;
		  case TREE_MEDIUM:
                    sprintf(buf,"A medium-sized %s tree lies here.",tree_table[tree->value[0]].name);
                    break;
		  case TREE_LARGE:
                    sprintf(buf,"A large %s tree lies here.",tree_table[tree->value[0]].name);
                    break;
		  default: //TREE_HUGE
                    sprintf(buf,"A huge %s tree lies here.",tree_table[tree->value[0]].name);
                    break;
		  }
		free_string(tree->description);
		tree->description = str_dup(buf);
		for (fch = d->character->in_room->people;fch != NULL;fch = fch->next_in_room)
		  {
		    if (!IS_NPC(fch) && fch->position == POS_CHOPPING)
		      {
			fch->position = POS_STANDING;
			send_to_char("Now that the tree has fallen, you stop chopping.\n\r",fch);
		      }
		  }
	      }
	    else 
	    {
	       send_to_char("Your chopping is really making a difference!\n\r",d->character);
	       act("Chopping sounds can be heard coming from $n's direction.",d->character,NULL,NULL,TO_ROOM);
	    }
	  }
	else send_to_char("You chop and chop, but do not see a lot of progress.\n\r",d->character);
      }
    else if (d->character->position == POS_MILLING)
      {
	if ((chance = get_skill(d->character,gsn_milling)) < 1)
          {
	    send_to_char("You realize you can't remember how to mill anymore, so you stop.\n\r",d->character);
	    d->character->position = POS_STANDING;
	    continue;
          }
        if ((tree = get_obj_vnum_room(d->character->in_room,OBJ_VNUM_TREE)) == NULL
            || !IS_SET(tree->value[4],TREE_VALUE_FALLEN))
          {
            d->character->position = POS_STANDING;
            send_to_char("You realize there is no longer a tree to mill, so you stop.\n\r",d->character);
            continue;
          }
	if (d->character->pcdata->primary_hand == HAND_LEFT)
          {
            if ((obj = get_eq_char (d->character, WEAR_WIELD_L)) == NULL)
              {
                obj = get_eq_char (d->character, WEAR_WIELD_R);
                //Since its not their primary hand
                chance /= 2;
              }
          }
        else
          {
	    if ((obj = get_eq_char (d->character, WEAR_WIELD_R)) == NULL)
	      {
		if (d->character->pcdata->primary_hand == HAND_RIGHT)
		  chance /= 2;
		obj = get_eq_char (d->character, WEAR_WIELD_L);
	      }
          }
        if (obj == NULL)
          {
            send_to_char("You realize you can't continue milling without a weapon, so you stop.\n\r",d->character);
            d->character->position = POS_STANDING;
            continue;
          }
	switch (obj->value[0])
          {
          case WEAPON_AXE: chance += 5;break;
          case WEAPON_SWORD:break;
          case WEAPON_SHORTBOW:
          case WEAPON_LONGBOW:
          case WEAPON_ARROW:
          case WEAPON_ARROWHEAD:
            send_to_char("You realize what you are holding is useless in milling, so you stop.\n\r",d->character);
            d->character->position = POS_STANDING;
            continue;
          default: chance /= 5;break;
          }

	if (IS_SET(tree->value[4],TREE_VALUE_ROTTEN) || tree->value[3] <= 0)
	  {
	    act("You attempt to extract useful lumber from $p, but all you end up doing is destroying it.",d->character,tree,NULL,TO_CHAR);
	    act("$n attempts to extract useful lumber from $p, but all $e ends up doing is destroying it.",d->character,tree,NULL,TO_ROOM);
	    extract_obj(tree);
	    continue;
	  }


	switch(attack_table[obj->value[3]].damage)
          {
          case DAM_SLASH : chance += 2;break;
          case DAM_BASH : chance /= 10;break;
          default : chance /= 4;break;
          }
	if (IS_SET(obj->value[4],WEAPON_SHARP))
          chance += 5;
	if (number_percent() < chance)
          {
	    --tree->value[3];
	    obj = create_object (get_obj_index (OBJ_VNUM_WOOD), 0);
	    obj_to_room(obj,d->character->in_room);
	    obj->value[0] = tree->value[0]; //Type of tree
	    obj->weight = tree_table[tree->value[0]].weight*10+number_range(0,3);
	    sprintf (buf, obj->short_descr, tree_table[tree->value[0]].name);
	    free_string (obj->short_descr);
	    obj->short_descr = str_dup (buf);
	    sprintf (buf, obj->description, tree_table[tree->value[0]].name);
	    free_string (obj->description);
	    obj->description = str_dup (buf);
	    sprintf (buf, "wood %s", tree_table[tree->value[0]].name);
	    free_string (obj->name);
	    obj->name = str_dup (buf);

	    obj->condition = 50;

		//make wood and shit and shit
            if (tree->value[3] <= 0)
	    {
  	      act ("You successfully mill the remains of $p into one last piece of lumber.",d->character,tree,NULL,TO_CHAR);
	      act("$n finishes off $p, extracing one last piece of lumber from it.",d->character,tree,NULL,TO_ROOM);
	      extract_obj(tree);
	      d->character->position = POS_STANDING;
	      ++d->character->pcdata->total_trees_milled;
	      if (d->character->pcdata->total_trees_milled%300 == 0)
	        improve_skill(d->character,gsn_milling);
	       
	      for (fch = d->character->in_room->people;fch != NULL;fch = fch->next_in_room)
		{
		  if (!IS_NPC(fch) && fch->position == POS_CHOPPING)
		    {
		      fch->position = POS_STANDING;
		      send_to_char("Now that the tree has been fully milled, you stop.\n\r",fch);
		    }
		}

	    }
	    else 
	      {
		act ("You successfully manuever around $p, cutting off various useless parts and creating a piece of lumber.",d->character,tree,NULL,TO_CHAR);
		act ("$n continues to hack away at $p, extracting a piece of lumber from it.",d->character,tree,NULL,TO_ROOM);
	      }

	  }
	else 
	  {
	    if (number_percent() < chance && number_percent() < 50)
	      {
		//Screwup, crappy piece of wood made
		--tree->value[3];
		//create shit wood
		obj = create_object (get_obj_index (OBJ_VNUM_WOOD), 0);
		obj_to_room(obj,d->character->in_room);
		obj->value[0] = tree->value[0]; //Type of tree
		sprintf (buf, "a warped piece of %s wood", tree_table[tree->value[0]].name);
		free_string (obj->short_descr);
		obj->short_descr = str_dup (buf);
		sprintf (buf, "A warped piece of %s wood is here.", tree_table[tree->value[0]].name);
		free_string (obj->description);
		obj->description = str_dup (buf);
		sprintf (buf, "wood %s", tree_table[tree->value[0]].name);
		free_string (obj->name);
		obj->name = str_dup (buf);

		obj->condition = 5;

		if (tree->value[3] <= 0)
		  {
		    act ("Your weapon slips, and you cut up the last remains of $p into a less than ideal piece of lumber.",d->character,tree,NULL,TO_CHAR);
		    act("$n finishes off $p, throwing one more piece of warped lumber on the pile.",d->character,tree,NULL,TO_ROOM);
		    extract_obj(tree);
		    d->character->position = POS_STANDING;
		    ++d->character->pcdata->total_trees_milled;
		    if (d->character->pcdata->total_trees_milled%300 == 0)
 		      improve_skill(d->character,gsn_milling);
		    for (fch = d->character->in_room->people;fch != NULL;fch = fch->next_in_room)
		      {
			if (!IS_NPC(fch) && fch->position == POS_CHOPPING)
			  {
			    fch->position = POS_STANDING;
			    send_to_char("Now that the tree has been fully milled, you stop.\n\r",fch);
			  }
		      }

		  }
		else
		  {
		    act ("$n hacks away at $p, extracting a piece of warped lumber from it.",d->character,tree,NULL,TO_ROOM);
		    act ("While milling, your weapon slips and you cut into $p, producing a less than ideal piece of lumber.",d->character,tree,NULL,TO_CHAR);
		  }
	      }
	    else act ("You maneuever around $p, attempting to convert it to lumber.",d->character,tree,NULL,TO_CHAR);
	  }
      }
    else if (d->character->position == POS_CARVING)
      {
	if (!d->character->pcdata || d->character->pcdata->trade_skill_obj == NULL)
	  {
	    send_to_char("You no longer have the object you were carving, so you stop.\n\r",d->character);
	    d->character->position = POS_STANDING;
	    return;
	  }
	if (get_skill(d->character,gsn_woodworking) < number_percent())
	{
          act("$n continues $s carving of $p.",d->character,d->character->pcdata->trade_skill_obj,NULL,TO_ROOM);
          act("You continue your carving of $p, making little progress.",d->character,d->character->pcdata->trade_skill_obj,NULL,TO_CHAR);
  	  return;
	}
		
	if (++d->character->pcdata->trade_skill_obj->value[3] >= wooden_obj_table[d->character->pcdata->trade_skill_obj->value[0]].updates_to_complete)
	{
  	  int i=0;
  	  switch (d->character->pcdata->trade_skill_obj->value[0])
	  {
 	    case WOODEN_OBJ_CHAIR: 
	      obj = create_object(get_obj_index(OBJ_VNUM_WOOD_CHAIR),0);
              obj->condition = number_range (get_skill (d->character, gsn_woodworking), 100);
	      obj->weight = .66*10*((tree_table[d->character->pcdata->trade_skill_obj->value[1]].weight + tree_table[d->character->pcdata->trade_skill_obj->value[2]].weight));
	      obj->value[3] = 100+tree_table[d->character->pcdata->trade_skill_obj->value[1]].hp_heal_bonus + tree_table[d->character->pcdata->trade_skill_obj->value[2]].hp_heal_bonus;
	      obj->value[4] = 100+tree_table[d->character->pcdata->trade_skill_obj->value[1]].mana_heal_bonus + tree_table[d->character->pcdata->trade_skill_obj->value[2]].mana_heal_bonus;
	      if (d->character->pcdata->trade_skill_obj->value[1]  != d->character->pcdata->trade_skill_obj->value[2])
		sprintf (buf, "a chair made of %s and %s wood", tree_table[d->character->pcdata->trade_skill_obj->value[1]].name, tree_table[d->character->pcdata->trade_skill_obj->value[2]].name);
	      else
		sprintf (buf, "a chair made of %s wood", tree_table[d->character->pcdata->trade_skill_obj->value[1]].name);
	      free_string (obj->short_descr);
	      obj->short_descr = str_dup (buf);
	      if (d->character->pcdata->trade_skill_obj->value[1] != d->character->pcdata->trade_skill_obj->value[2])
                sprintf (buf, "A chair made of %s and %s wood, marked with the sign of '%s'", tree_table[d->character->pcdata->trade_skill_obj->value[1]].name, tree_table[d->character->pcdata->trade_skill_obj->value[2]].name, d->character->name);
              else
                sprintf (buf, "A chair made of %s wood, marked with the sign of '%s'", tree_table[d->character->pcdata->trade_skill_obj->value[1]].name,d->character->name);
	      free_string (obj->description);
	      obj->description = str_dup (buf);
	      if (d->character->pcdata->trade_skill_obj->value[1] != d->character->pcdata->trade_skill_obj->value[2])
		sprintf (buf, "chair %s %s",  tree_table[d->character->pcdata->trade_skill_obj->value[1]].name, tree_table[d->character->pcdata->trade_skill_obj->value[2]].name);
	      else  sprintf (buf, "chair %s", tree_table[d->character->pcdata->trade_skill_obj->value[1]].name);
	      free_string (obj->name);
	      obj->name = str_dup (buf);
	      for (i = 0;i<2;i++)
		{
		  switch (d->character->pcdata->trade_skill_obj->value[i+1])
		    {
		    case TREE_TYPE_MOONBLOSSOM:
		    case TREE_TYPE_JEWELWOOD: obj->cost += 3500;break;
		    case TREE_TYPE_RISIRIEL: obj->cost += 3000;break;
		    case TREE_TYPE_HELLROOT: 
		    case TREE_TYPE_ROTSTENCH:
		    case TREE_TYPE_GREEN_LEECHLEAF:
		    case TREE_TYPE_SLIMEWOOD:
		    case TREE_TYPE_BARBVINE: obj->cost += 50;break;
		    default: obj->cost = 2000;break;
		    }
		}
	      obj_to_char(obj,d->character);
	      extract_obj(d->character->pcdata->trade_skill_obj);
	      d->character->pcdata->trade_skill_obj = NULL;
	      d->character->position = POS_STANDING;
	      act("$n finishes $s carving of $p.",d->character,obj,NULL,TO_ROOM);
	      act("You finish your carving of $p.",d->character,obj,NULL,TO_CHAR);
	      break;
	  case WOODEN_OBJ_CUP:
	    obj = create_object(get_obj_index(OBJ_VNUM_WOOD_CUP),0);
	    obj->condition = number_range (get_skill (d->character, gsn_woodworking), 100);
	    obj->weight = .40*(tree_table[d->character->pcdata->trade_skill_obj->value[1]].weight);
	    switch (d->character->pcdata->trade_skill_obj->value[1])
	      {
	      case TREE_TYPE_MOONBLOSSOM: obj->value[0] = 100;break;
	      case TREE_TYPE_JEWELWOOD: obj->value[0] = 10;break;
	      case TREE_TYPE_SLIMEWOOD: obj->value[0] = 50;break;
	      case TREE_TYPE_RISIRIEL: obj->value[0] = 150;break;
	      case TREE_TYPE_HELLROOT: obj->value[0] = 210;break;
	      case TREE_TYPE_IRONWOOD:
	      case TREE_TYPE_GREEN_LEECHLEAF:
	      case TREE_TYPE_BARBVINE: obj->value[0] = 170;break;
	      default: obj->value[0] = 110;break;
	      }
	    sprintf (buf, "`da cup made of %s wood``", tree_table[d->character->pcdata->trade_skill_obj->value[1]].name);
	    free_string (obj->short_descr);
	    obj->short_descr = str_dup (buf);
	    sprintf (buf, "`dA cup made of %s wood, marked by the sign of '%s'.``", tree_table[d->character->pcdata->trade_skill_obj->value[1]].name, d->character->name);
	    free_string (obj->description);
	    obj->description = str_dup (buf);
	    sprintf (buf, "%s cup", tree_table[d->character->pcdata->trade_skill_obj->value[1]].name);
	    free_string (obj->name);
	    obj->name = str_dup (buf);
	    obj_to_char(obj,d->character);
	    extract_obj(d->character->pcdata->trade_skill_obj);
	    d->character->pcdata->trade_skill_obj = NULL;
	    d->character->position = POS_STANDING;
	    act("$n finishes $s carving of $p.",d->character,obj,NULL,TO_ROOM);
	    act("You finish your carving of $p.",d->character,obj,NULL,TO_CHAR);
	  }
	}
	else
	  {
	    act("$n continues $s carving of $p.",d->character,d->character->pcdata->trade_skill_obj,NULL,TO_ROOM);
	    act("You continue your carving of $p.",d->character,d->character->pcdata->trade_skill_obj,NULL,TO_CHAR);
	  }
      }
  }
}

//Iblis - The chop command, for use with the lumberjacking skill
void do_chop (CHAR_DATA *ch, char* argument)
{
  OBJ_DATA *obj, *weapon;
  if (get_skill(ch,gsn_lumberjacking) < 1 || IS_NPC(ch))
  {
    send_to_char("You can't even chop suey.\n\r",ch);
    return;
  }
  if (argument[0] == '\0')
  {
    send_to_char("Chop what?\n\r",ch);
    return;
  }
  if ((obj = get_obj_here (ch, argument)) == NULL)
  {
    send_to_char("You do not see that here.\n\r",ch);
    return;
  }
  if (obj->item_type != ITEM_TREE)
  {
    send_to_char("You can only chop down trees!\n\r",ch);
    return;
  }
  if (IS_SET(obj->value[4],TREE_VALUE_FALLEN))
  {
    send_to_char("That tree has already been chopped down!\n\r",ch);
    return;
  }
  if ((weapon = get_eq_char (ch, WEAR_WIELD_R)) == NULL)
      weapon = get_eq_char (ch, WEAR_WIELD_L);
  if (weapon == NULL)
  {
    send_to_char ("You can't chop down a tree with your bare hands!\n\r",ch);
    return;
  }
  switch (weapon->value[0])
  {
    case WEAPON_SHORTBOW:
    case WEAPON_LONGBOW:
    case WEAPON_ARROW:
    case WEAPON_ARROWHEAD: send_to_char("You can't chop down a tree weilding that!\n\r",ch);
			   return;
  }
  act("You begin chopping away at $p.",ch,obj,NULL,TO_CHAR);
  act("$n begins chopping away at $p.",ch,obj,NULL,TO_ROOM);
  ch->position = POS_CHOPPING;
}

//Iblis - the command to stop performing any trade skill
void do_stop(CHAR_DATA *ch, char* argument)
{
  if (ch->position == POS_CHOPPING)
  {
    ch->position = POS_STANDING;
    send_to_char("You decide to stop chopping.\n\r",ch);
    return;
  }
  if (ch->position == POS_MILLING)
  {
    ch->position = POS_STANDING;
    send_to_char("You decide to stop milling.\n\r",ch);
    return;
  }
  if (ch->position == POS_CARVING)
  {
    ch->position = POS_STANDING;
    send_to_char("You decide to stop carving.\n\r",ch);
    if (!IS_NPC(ch))
      ch->pcdata->trade_skill_obj = NULL;
    return;
  }
	  
  send_to_char("You can't stop what you haven't started!\n\r",ch);
  return;
}

//Iblis - the Mill command, for usage with the milling skill
void do_mill(CHAR_DATA *ch, char* argument)
{
  OBJ_DATA *obj, *weapon;
  if (get_skill(ch,gsn_milling) < 1 || IS_NPC(ch))
    {
      send_to_char("Mill.  It rhymes with drill.  Neither of which you know how to do.\n\r",ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char("Mill what?\n\r",ch);
      return;
    }
  if ((obj = get_obj_here (ch, argument)) == NULL)
    {
      send_to_char("You do not see that here.\n\r",ch);
      return;
    }
  if (obj->item_type != ITEM_TREE)
    {
      send_to_char("You can only mill fallen trees!\n\r",ch);
      return;
    }
  if (!IS_SET(obj->value[4],TREE_VALUE_FALLEN))
    {
      send_to_char("That tree needs to be chopped down first.\n\r",ch);
      return;
    }
  if ((weapon = get_eq_char (ch, WEAR_WIELD_R)) == NULL)
    weapon = get_eq_char (ch, WEAR_WIELD_L);
  if (weapon == NULL)
    {
      send_to_char ("You can't mill a tree with your bare hands!\n\r",ch);
      return;
    }
  switch (weapon->value[0])
    {
    case WEAPON_SHORTBOW:
    case WEAPON_LONGBOW:
    case WEAPON_ARROW:
    case WEAPON_ARROWHEAD: send_to_char("You can't mill a tree weilding that!\n\r",ch);
      return;
    }
  if (IS_SET(obj->value[4],TREE_VALUE_ROTTEN) || obj->value[3] <= 0)
  {
	act("You attempt to extract useful lumber from $p, but all you end up doing is destroying it.",ch,obj,NULL,TO_CHAR);
	act("$n attempts to extract useful lumber from $p, but all $e ends up doing is destroying it.",ch,obj,NULL,TO_CHAR);
	extract_obj(obj);
	return;
  }
	
  
  ch->position = POS_MILLING;
  send_to_char("You begin to mill the tree, in an attempt to create lumber.\n\r",ch);
}

/*void do_nail(CHAR_DATA *ch, char* argument)
{
  if ((obj = get_obj_carry (ch, argument)) == NULL)
    {
      send_to_char ("You do not have that item.\n\r", ch);
      return;
    }
  if (obj->pIndexData->vnum != OBJ_VNUM_PLAQUE)
    {
      send_to_char ("You can't nail that!\n\r",ch);
      return;
    }
}*/
