#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include "merc.h"
#include "interp.h"
#include "tables.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"
#include "obj_trig.h"
extern const char *dir_name[];
extern CHAR_DATA *load_char_obj2 (char *name);
extern struct clan_type clan_table[MAX_CLAN];
extern ROOM_INDEX_DATA *room_index_hash[], *new_room_index ();
extern bool is_linkloading;
void ppurge args((CHAR_DATA *ch, CHAR_DATA* victim));
void update_aggressor args ((CHAR_DATA * ch, CHAR_DATA * victim));
CHAR_DATA *get_char_room_nomask args ((CHAR_DATA * ch, char *argument));
void free_char args ((CHAR_DATA * ch));
void close_store args ((ROOM_INDEX_DATA * location));
void deduct_bank args ((CHAR_DATA * ch, int cost));
int quiver_holding args ((OBJ_DATA * obj));
bool has_quiver args ((CHAR_DATA * ch));
void corpse_carve args((CHAR_DATA* ch, char* argument));
void spell_detect_invis args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
void spell_detect_hidden args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
void update_aggressor2 args((CHAR_DATA * ch, CHAR_DATA * victim, bool show_messages));
void appraise args((CHAR_DATA *ch, OBJ_DATA* card));
void obj_cast_spell2 args((int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, char* argument));

int hands_used args((CHAR_DATA *ch));
int hands_required args((CHAR_DATA *ch, OBJ_DATA *obj));
bool all_slots_free args((CHAR_DATA *ch, OBJ_DATA *obj_to_wear, int hand));

extern char *target_name;

void open_store (ROOM_INDEX_DATA * location)
{
  char filename[MAX_INPUT_LENGTH];
  FILE *fp;
  mkdir ("stock/", 0755);
  sprintf (filename, "stock/%d.open", location->vnum);
  fp = fopen (filename, "w");
  fclose (fp);
}

void unhire(ROOM_INDEX_DATA *location)
{
  char filename[MAX_INPUT_LENGTH];
  sprintf (filename, "stock/%d.hired", location->vnum);
  unlink (filename);
}

void close_store (ROOM_INDEX_DATA * location)
{
  char filename[MAX_INPUT_LENGTH];
  sprintf (filename, "stock/%d.open", location->vnum);
  unlink (filename);
}

bool store_closed (ROOM_INDEX_DATA * location)
{
  char filename[MAX_INPUT_LENGTH];
  struct stat statBuf;
  sprintf (filename, "stock/%d.open", location->vnum);
  if (stat (filename, &statBuf) == -1)
    return (TRUE);
  return (FALSE);
}

void stock_obj (OBJ_DATA * obj, OBJ_DATA * shelf)
{
  OBJ_DATA *obj_walk, *last_obj = NULL;
  char stocked = FALSE;
  for (obj_walk = shelf->contains; obj_walk != NULL;
       obj_walk = obj_walk->next_content)
    {
      if (!str_cmp (obj_walk->short_descr, obj->short_descr))
	{
	  stocked = TRUE;
	  obj->next_content = obj_walk->next_content;
	  obj_walk->next_content = obj;
	  break;
	}
      last_obj = obj_walk;
    }
  if (!stocked)
    {
      if (last_obj != NULL)
	{
	  last_obj->next_content = obj;
	  obj->next_content = NULL;
	}
      else
	{
	  shelf->contains = obj;
	  obj->next_content = NULL;
	}
    }
  obj->in_obj = shelf;
  obj->in_room = NULL;
  obj->carried_by = NULL;
}

void do_dismiss (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *mob;
  if (IS_NPC (ch))
    return;
  if (!IS_SET (ch->in_room->race_flags, ROOM_PLAYERSTORE)
      || ch->in_room->owner == NULL || ch->in_room->owner[0] == '\0'
      || str_cmp (ch->name, ch->in_room->owner))
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
    }
  for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room)
    {
      if (!IS_NPC (mob))
	continue;
      if (!IS_SET (mob->recruit_flags, RECRUIT_KEEPER))
	continue;
      if (mob->recruit_value[0] != ch->in_room->vnum)
	continue;
      act
	("You politely ask $N to leave, as $S services will no longer be needed.",
	 ch, NULL, mob, TO_CHAR);
      act
	("$n politely asks $N to leave, as $S services will no longer be needed.",
	 ch, NULL, mob, TO_ROOM);
      interpret(mob,"oocnote post");
      unhire(mob->in_room);
      close_store(mob->in_room);
      extract_char (mob, TRUE);
      return;
    }
  send_to_char ("You don't currently have an employee working here.\n\r", ch);
  return;
}

void do_hire (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  CHAR_DATA *mob;
  if (IS_NPC (ch))
    return;
  if (!IS_SET (ch->in_room->race_flags, ROOM_PLAYERSTORE)
      || ch->in_room->owner == NULL || ch->in_room->owner[0] == '\0'
      || str_cmp (ch->name, ch->in_room->owner))
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Syntax: hire long LONGDESC\n\r",ch);
    send_to_char("	  hire short shortdesc\n\r",ch);
    send_to_char("        hire start X        - begins the mob's shift for X hours (-1 forever)\n\r",ch);
    return;
  }
  if (!str_cmp(arg,"short"))
  {
    if (argument[0] == '\0')
    {
      do_hire(ch,"");
      return;
    }
    free_string(ch->short_descr);
    ch->short_descr = str_dup(argument);
    return;
  }
  if (!str_cmp(arg,"long"))
    {
      if (argument[0] == '\0')
	{
	  do_hire(ch,"");
  	  return;
	}
      free_string(ch->long_descr);
      ch->long_descr = str_dup(argument);
      return;
    }
  if (!str_cmp(arg,"start") && ch->in_room)
    {
      OBJ_DATA *shelf, *obj_walk;
      long total=0;
      int ticks;
      CHAR_DATA *keeper;
      if (argument[0] == '\0' || !is_number(argument))
      {
        do_hire(ch,"");
        return;
      } 
      ticks = atoi(argument);
      if (ticks < -1 || ticks > 30000 || ticks == 0)
      {
        send_to_char("The number of ticks must either be -1 (for pemanent)\n\ror between 1 and 30000.\n\r",ch);
	return;
      }
      for (keeper = ch->in_room->people; keeper != NULL; keeper = keeper->next_in_room)
      {
        if (!IS_NPC (keeper))
          continue;
        if (!IS_SET (keeper->recruit_flags, RECRUIT_KEEPER))
          continue;
        if (keeper->recruit_value[0] != ch->in_room->vnum)
          continue;
        break;
      }
      if (keeper != NULL)
      {
        send_to_char("You already have a shopkeeper!\n\r",ch);
        return;
      }
	
      mob = create_mobile(get_mob_index(MOB_VNUM_DEFAULT_SHOPKEEPER));
      mob->recruit_value[2] = ticks;
      if (ticks == -1)
      {
        char filename[MAX_INPUT_LENGTH];
        FILE *fp;
	mkdir ("stock/", 0755);
	sprintf (filename, "stock/%d.hired", ch->in_room->vnum);
	fp = fopen (filename, "w");
	fclose (fp);
      }
      shelf = create_object(get_obj_index(OBJ_VNUM_SHELF),0);
      load_shelf (shelf, ch->in_room->vnum);
      if (shelf->contains != NULL)
        {
          for (obj_walk = shelf->contains; obj_walk != NULL;
               obj_walk = obj_walk->next_content)
            {
		  total += obj_walk->cost;
		  if (total >= 100000)
		    {
		      total = -1;
		      break;
		    }
	    }
	}
      extract_obj (shelf);
	mob->recruit_value[1] = 1;  //Charge % per item
	     

      if (ch->short_descr[0] != '\0')
	{
	  free_string(mob->short_descr);
	  sprintf(buf,"%s``",ch->short_descr);
	  mob->short_descr = str_dup(buf);
	}
      if (ch->long_descr[0] != '\0')
      {
	free_string(mob->long_descr);
	sprintf(buf,"%s``\n\r",ch->long_descr);
	mob->long_descr = str_dup(buf);
      }
      SET_BIT (mob->recruit_flags, RECRUIT_KEEPER);
      mob->recruit_value[0] = ch->in_room->vnum;
      char_to_room(mob,ch->in_room);
      open_store(ch->in_room);
      act("$n begins $s duty as shopkeeper.",mob,NULL,NULL,TO_ALL);
      //DEAL WITH MONEY HERE
    }
}


  

void do_unstock (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *obj_index;
  OBJ_DATA *shelf, *obj;
  if (!IS_SET (ch->in_room->race_flags, ROOM_PLAYERSTORE))
    {
      send_to_char
	("Sorry, you must be in a store to unstock inventory.\n\r", ch);
      return;
    }
  if (str_cmp (ch->in_room->owner, ch->name) && !IS_IMMORTAL (ch))
    {
      send_to_char ("Sorry, you don't own this store.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg1);
  one_argument (argument, arg2);
  obj_index = get_obj_index (OBJ_VNUM_SHELF);
  if (obj_index == NULL)
    {
      send_to_char ("Sorry, the store's shelves seem to be broken.\n\r", ch);
      return;
    }
  shelf = create_object (obj_index, 0);
  if (shelf == NULL)
    {
      send_to_char ("Sorry, the store's shelves seem to be broken.\n\r", ch);
      return;
    }
  load_shelf (shelf, ch->in_room->vnum);
  if ((obj = get_obj_list (ch, arg1, shelf->contains)) == NULL)
    {
      send_to_char ("You do not have that item in your store.\n\r", ch);
      extract_obj (shelf);
      return;
    }
  if (IS_SET (obj->extra_flags[0], ITEM_PERMSTOCK))
    {
      send_to_char
	("Sorry, you cannot unstock that item at this time.\n\r", ch);
      extract_obj (shelf);
      return;
    }
  if (IS_SET (obj->extra_flags[0], ITEM_DONATE))
    obj->cost = 0;

  else
    obj->cost = obj->pIndexData->cost;
  obj_from_obj (obj);
  save_shelf (shelf, ch->in_room->vnum);
  obj_to_char (obj, ch);
  act ("You remove $p from your store's inventory.", ch, obj, NULL, TO_CHAR);
  act ("$n removes $p from the store's inventory.", ch, obj, NULL, TO_ROOM);
  sprintf(arg1,"%s UNstocked %s in their store at %d.",ch->name,obj->short_descr,ch->in_room->vnum);
  wiznet (arg1, ch, NULL, WIZ_STOCKBUY, 0, 0);      
  extract_obj (shelf);
  //Iblis - 3/11/04 - Antiduping Code
  if (!IS_NPC(ch))
    save_char_obj (ch);
  //12-29-03 Iblis - By Minax's request, if you're overweight you can't fly
  if (IS_AFFECTED (ch, AFF_FLYING) && get_carry_weight (ch) > can_carry_w (ch))
  {
    send_to_char("You are so heavy you fall to the ground.\n\r",ch);
    do_land(ch,"");
    return;
  }
}

void do_stock (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *obj_index;
  OBJ_DATA *shelf, *obj, *last_obj = NULL, *obj_walk;
  int count = 0;
  if (!IS_SET (ch->in_room->race_flags, ROOM_PLAYERSTORE))
    {
      send_to_char
	("Sorry, you must be in a store to stock inventory.\n\r", ch);
      return;
    }
  if (str_cmp (ch->in_room->owner, ch->name) && !IS_IMMORTAL (ch))
    {
      send_to_char ("Sorry, you don't own this store.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg1);
  one_argument (argument, arg2);
  if ((obj = get_obj_carry (ch, arg1)) == NULL)
    {
      send_to_char ("You do not have that item.\n\r", ch);
      return;
    }
  // Adeon 7/18/03 -- not allowed to stock nosave items in stores...
  if (IS_OBJ_STAT (obj, ITEM_NOSAVE) || IS_SET(obj->extra_flags[1],ITEM_NOSTOCK))
    {
      send_to_char ("Strong magic forces prevent you...\n\r", ch);
      return;
    }
  if (!can_drop_obj (ch, obj))
    {
      send_to_char ("You can't let go of it.\n\r", ch);
      return;
    }
  if (obj->item_type == ITEM_KEY && !obj->value[0])
    {
      send_to_char ("That key may not be stocked.\n\r", ch);
      return;
    }
  if (obj->contains != NULL)
    {
      send_to_char ("You have a feeling that would be a bad idea.\n\r", ch);
      return;
    }
  obj_index = get_obj_index (OBJ_VNUM_SHELF);
  if (obj_index == NULL)
    {
      send_to_char ("Sorry, the store's shelves seem to be broken.\n\r", ch);
      return;
    }
  shelf = create_object (obj_index, 0);
  if (shelf == NULL)
    {
      send_to_char ("Sorry, the store's shelves seem to be broken.\n\r", ch);
      return;
    }
  load_shelf (shelf, ch->in_room->vnum);
  for (obj_walk = shelf->contains; obj_walk != NULL;
       obj_walk = obj_walk->next_content)
    {
      if (obj_walk != last_obj)
	count++;
      last_obj = obj_walk;
    }
  if (count >= 50)
    {
      send_to_char ("Sorry, the store's shelves are already full.\n\r", ch);
      extract_obj (shelf);
      return;
    }
  if (arg2[0] != '\0' && is_number (arg2))
  {  
    int cost = atoi (arg2);
    if (cost < 1 || cost > 9999999)
    {
      send_to_char("The cost must be between 1 and 9999999.\n\r",ch);
      return;
    }
    obj->cost = cost;
  }
  if (obj->cost == 0)
    obj->cost = 1;
  obj_from_char (obj);
  if (IS_SET (obj->extra_flags[0], ITEM_PERMSTOCK))
    REMOVE_BIT (obj->extra_flags[0], ITEM_PERMSTOCK);
  stock_obj (obj, shelf);
  save_shelf (shelf, ch->in_room->vnum);
  act ("You add $p to your store's inventory.", ch, obj, NULL, TO_CHAR);
  act ("$n adds $p to the store's inventory.", ch, obj, NULL, TO_ROOM);
  sprintf(arg1,"%s stocked %s for %d in their store at %d.",ch->name,obj->short_descr,obj->cost,ch->in_room->vnum);
  wiznet (arg1, ch, NULL, WIZ_STOCKBUY, 0, 0);
  extract_obj (shelf);
  //Iblis - 3/11/04 - Antiduping Code
  if (!IS_NPC(ch))
    save_char_obj (ch);
}

void do_donate (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  OBJ_DATA *obj, *container;
  argument = one_argument (argument, arg1);
  if (arg1[0] == '\0')
    {
      send_to_char ("Donate what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_carry (ch, arg1)) == NULL)
    {
      send_to_char ("You do not have that item.\n\r", ch);
      return;
    }
  if (obj->wear_loc != -1)
    {
      send_to_char
	("You will have to remove that before you can donate it.\n\r", ch);
      return;
    }
  if (ch->fighting != NULL)
  {
    send_to_char("You cannot donating while fighting.\n\r",ch);
    return;
  }
  // Adeon 7/18/03 -- can't donate nosave eq
  if (IS_OBJ_STAT (obj, ITEM_NOSAVE) || IS_SET(obj->extra_flags[1],ITEM_NODONATE) 
		  || obj->item_type == ITEM_CARD)
    {
      send_to_char ("Strong magic forces prevent you...\n\r", ch);
      return;
    }
  if (IS_OBJ_STAT (obj, ITEM_NODROP))
    {
      send_to_char ("Sorry, you can't donate that.\n\r", ch);
      return;
    }
  if (obj->pIndexData->vnum >= 23000 && obj->pIndexData->vnum <= 23100)
    {
      send_to_char ("Sorry, you can't donate that.\n\r", ch);
      return;
    }
  for (container = object_list; container != NULL;
       container = container->next)
    {
      if (container->pIndexData->vnum == OBJ_VNUM_PIT)
	break;
    }
  if (container == NULL)
    {
      send_to_char
	("Sorry, the donation box seems to be out of order.\n\r", ch);
      return;
    }
  if (ch->in_room->vnum == ROOM_VNUM_PIT)
    {
      act ("$n puts $p in the donation box.", ch, obj, NULL, TO_ROOM);
      act ("You put $p in the donation box.", ch, obj, NULL, TO_CHAR);
    }
  else
    {
      location = get_room_index (ROOM_VNUM_PIT);
      if (location == NULL)
	return;
      act ("You send $p to the donation box.", ch, obj, container, TO_CHAR);
      act
	("Something materializes from thin air and falls into the donation box.",
	 location->people, NULL, NULL, TO_ALL);
    }
  SET_BIT (obj->extra_flags[0], ITEM_DONATE);
  obj->cost = 0;
  obj_from_char (obj);
  obj_to_obj (obj, container);
  save_char_obj (ch);
}

void save_player_box (OBJ_DATA * pObj)
{
  FILE *fp;
  char tcbuf[MAX_STRING_LENGTH];
  if (pObj->contains == NULL)
    {
      sprintf (tcbuf, "boxes/%d", pObj->pIndexData->vnum);
      unlink (tcbuf);
      return;
    }
  strcpy (tcbuf, "boxes");
  mkdir (tcbuf, 0755);
  sprintf (tcbuf + strlen (tcbuf), "/%d", pObj->pIndexData->vnum);
  if ((fp = fopen (tcbuf, "w")) == NULL)
    return;
  fwrite_objdb (pObj->contains, fp, 0);
  fclose (fp);
}

void load_player_box (OBJ_DATA * pObj)
{
  FILE *fp;
  char tcbuf[MAX_STRING_LENGTH];
  sprintf (tcbuf, "boxes/%d", pObj->pIndexData->vnum);
  if ((fp = fopen (tcbuf, "r")) == NULL)
    return;
  while (!feof (fp) && !str_cmp (fread_word (fp), "#O"))
    fread_objdb (pObj, fp);
  fclose (fp);
}

void save_player_corpse (OBJ_DATA * pObj)
{
  FILE *fp;
  char tcbuf[MAX_STRING_LENGTH];
  if (pObj->contains == NULL || pObj->in_room == NULL)
    {
      if (!pObj->plr_owner)
      {
	      //bug("Error corpse has no owner in save_player_corpse",0);
	      return;
      }
      if (pObj->in_room)
        sprintf (tcbuf, "corpses/%s%da%d", pObj->plr_owner, pObj->in_room->vnum, pObj->value[5]);
      else if (pObj->carried_by && pObj->carried_by->in_room)
        sprintf (tcbuf, "corpses/%s%da%d", pObj->plr_owner, pObj->carried_by->in_room->vnum, pObj->value[5]);
      else 
      {
       // bug("save_player_corpse : Corpse is not in a room and not on anyone!",0);
	bug(pObj->short_descr,0);
        return;
      }
      unlink (tcbuf);
      return;
    }
  strcpy (tcbuf, "corpses");
  mkdir (tcbuf, 0755);
  sprintf (tcbuf + strlen (tcbuf), "/%s%da%d", pObj->plr_owner, pObj->in_room->vnum,pObj->value[5]);
  if ((fp = fopen (tcbuf, "w")) == NULL)
    return;
  fwrite_objdb (pObj->contains, fp, 0);
  fclose (fp);
}

void delete_player_corpse (OBJ_DATA * pObj)
{
  char tcbuf[MAX_STRING_LENGTH];
  if (!pObj->plr_owner)
  {
    //bug("Error corpse has no owner in delete_player_corpse",0);
    return;
  }
   
  if (pObj->in_room)
    sprintf (tcbuf, "corpses/%s%da%d", pObj->plr_owner, pObj->in_room->vnum, pObj->value[5]);
  else if (pObj->carried_by && pObj->carried_by->in_room)
    sprintf (tcbuf, "corpses/%s%da%d", pObj->plr_owner, pObj->carried_by->in_room->vnum, pObj->value[5]);
  else
    {
      //bug("delete_player_corpse : Corpse is not in a room and not on anyone!",0);
      bug(pObj->short_descr,0);
      return;
    }
  unlink (tcbuf);
  return;
}


bool stopping_char(char c)
{
	if (c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6'
			|| c == '7' || c == '8' || c == '9' || c == '0' || c == '\0')
		return TRUE;
	return FALSE;
}

void load_player_corpses()
{
  FILE *fp, *mfp;
  char tcbuf[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH],buf3[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  int counter=0;
  char *tempb;
  OBJ_DATA *corpse;
  if ((mfp = fopen ("corpses.txt", "r")) == NULL)
    return;
  while (!feof (mfp))
  {
    sprintf(buf3,fread_word(mfp));
    if (feof (mfp))
	    return;
    counter = 0;
    while (!stopping_char(buf3[counter]))
    {
	    counter++;
    }
    strncpy (buf2,buf3,counter);
    buf2[counter] = '\0';
    corpse = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC),0);
    corpse->cost = 0;
    corpse->level = 1;  //beats me how to save this..Not really important
    if (corpse->plr_owner)
      free_string(corpse->plr_owner);
    corpse->plr_owner = str_dup(buf2);
    sprintf (buf, corpse->short_descr, buf2);
    free_string (corpse->short_descr);
    corpse->short_descr = str_dup (buf);
    sprintf (buf, corpse->description, buf2);
    free_string (corpse->description);
    corpse->description = str_dup (buf);	  
    corpse->timer = number_range(29,31);   //RESET THE TIMER ON CRASH/REBOOT because I'm nice
    sprintf (tcbuf, "corpses/%s", buf3);
    if ((fp = fopen (tcbuf, "r")) == NULL)
      return;
    while (!feof (fp) && !str_cmp (fread_word (fp), "#O"))
      fread_objdb (corpse, fp);
    fclose (fp);
    tempb = buf3;
    tempb += counter;
    counter = 0;
    while (tempb[counter] != 'a' && tempb[counter] != '\0')
	    counter++;
    strncpy (buf2,tempb,counter);
    buf2[counter] = '\0';
    if (!is_number (buf2))
    {
      bug("load_player_corpse() : to_room vnum ERROR",0);
      obj_to_room(corpse,get_room_index(ROOM_VNUM_ALTAR));
      continue;
    }
    obj_to_room(corpse,get_room_index(atoi (buf2)));
    tempb += counter+1;
    strcpy(buf2,tempb);
    if (!is_number (buf2))
    {
      bug("load_player_corpse() : value[5] ERROR",0);
      continue;
    }
    corpse->value[5] = atoi (buf2);
  }
  fclose (mfp);
}


void do_create (CHAR_DATA * ch, char *argument)
{
  char obj_name[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int skill;
  argument = one_argument (argument, obj_name);
  if (obj_name[0] == '\0')
    {
      send_to_char ("Create what?\n\r", ch);
      return;
    }
  if (!str_cmp (obj_name, "wall of thorns"))
    {
      char direction[MAX_INPUT_LENGTH];
      short dir_num;
      if ((skill = get_skill (ch, gsn_wall_thorns)) <= 0)
	{
	  send_to_char
	    ("You don't know anything about creating a wall of thorns.\n\r",
	     ch);
	  return;
	}
      one_argument (argument, direction);
      if (direction[0] == '\0')
	{
	  send_to_char ("Create a wall of thorns at which exit?\n\r", ch);
	  return;
	}
      if (!str_cmp (direction, "n") || !str_cmp (direction, "north"))
	dir_num = DIR_NORTH;

      else if (!str_cmp (direction, "s") || !str_cmp (direction, "south"))
	dir_num = DIR_SOUTH;

      else if (!str_cmp (direction, "e") || !str_cmp (direction, "east"))
	dir_num = DIR_EAST;

      else if (!str_cmp (direction, "w") || !str_cmp (direction, "west"))
	dir_num = DIR_WEST;

      else if (!str_cmp (direction, "u") || !str_cmp (direction, "up"))
	dir_num = DIR_UP;

      else if (!str_cmp (direction, "d") || !str_cmp (direction, "down"))
	dir_num = DIR_DOWN;

      else
	{
	  send_to_char ("Create a wall of thorns at which exit?\n\r", ch);
	  return;
	}
      if (ch->in_room->exit[dir_num] == NULL)
	{
	  send_to_char ("There is no exit in that direction.\n\r", ch);
	  return;
	}
      for (obj = ch->in_room->contents; obj; obj = obj->next_content)
	if (obj->pIndexData->vnum == OBJ_VNUM_WALL_THORNS
	    && obj->value[0] == dir_num)
	  {
	    send_to_char
	      ("There is already a wall of thorns covering that exit.\n\r",
	       ch);
	    return;
	  }
      if (ch->level < 40)
	{
	  if (ch->mana < 100)
	    {
	      send_to_char
		("You don't have enough mana to create that.\n\r", ch);
	      return;
	    }
	  else
	    ch->mana -= 100;
	}
      else if (ch->level < 50)
	{
	  if (ch->mana < 75)
	    {
	      send_to_char
		("You don't have enough mana to create that.\n\r", ch);
	      return;
	    }
	  else
	    ch->mana -= 75;
	}
      else
	{
	  if (ch->mana < 50)
	    {
	      send_to_char
		("You don't have enough mana to create that.\n\r", ch);
	      return;
	    }
	  else
	    ch->mana -= 50;
	}
      if (number_percent () < skill)
	{
	  char buf[MAX_STRING_LENGTH];
	  obj =
	    create_object (get_obj_index (OBJ_VNUM_WALL_THORNS), ch->level);
	  sprintf (buf, obj->description, dir_name[dir_num]);
	  free_string (obj->description);
	  obj->description = str_dup (buf);
	  obj->value[0] = dir_num;
	  obj->timer = (ch->level / 10);
	  obj->level = ch->level;
	  obj_to_room (obj, ch->in_room);
	  act ("You create a wall of thorns over the exit $T.", ch, NULL,
	       dir_name[dir_num], TO_CHAR);
	  act ("$n creates a wall of thorns over the exit $T.", ch, NULL,
	       dir_name[dir_num], TO_ROOM);
	}
      else
	{
	  act
	    ("You attempt to construct a wall of thorns, but it collapses to the ground.",
	     ch, NULL, NULL, TO_CHAR);
	  act
	    ("$n attempts to constuct a wall of thorns, but it collapses to the ground.",
	     ch, NULL, NULL, TO_ROOM);
	  return;
	}
    }
  else
    send_to_char ("You don't know how to create that.\n\r", ch);
}

void do_write (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj, *write_obj;
  char arg[MAX_INPUT_LENGTH];
  bool found = FALSE;
  EXTRA_DESCR_DATA *ed;
  argument = one_argument (argument, arg);
  if (!str_cmp (arg, "on"))
    argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Write on what, with what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_carry (ch, arg)) == NULL)
    {
      send_to_char ("You do not have that item.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg);
  if (!str_cmp (arg, "with"))
    argument = one_argument (argument, arg);
  if ((write_obj = get_obj_carry (ch, arg)) == NULL)
    {
      send_to_char ("You do not have that item.\n\r", ch);
      return;
    }
  if (write_obj->item_type != ITEM_WRITING_INSTRUMENT)
    {
      send_to_char ("You can't write with that.\n\r", ch);
      return;
    }
  if (obj->item_type != ITEM_WRITING_PAPER)
    {
      send_to_char ("You can't write on that.\n\r", ch);
      return;
    }
  for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
    if (!str_cmp (ed->keyword, "papermsg"))
      {
	found = TRUE;
	edit_string (ch, &ed->description);
	return;
      }
  if (!found)
    {
      ed = new_extra_descr ();
      ed->keyword = str_dup ("papermsg");
      ed->description = str_dup ("");
      ed->next = obj->extra_descr;
      obj->extra_descr = ed;
      edit_string (ch, &ed->description);
    }
}
void do_auction (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj=NULL;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int gold = 0, silver = 0;
  argument = one_argument (argument, arg1);
  if (IS_NPC (ch))
    return;
  if (arg1[0] == '\0')
    {
      if (IS_SET (ch->comm, COMM_NOAUCTION))
	{
	  send_to_char ("Auction channel is now ON.\n\r", ch);
	  REMOVE_BIT (ch->comm, COMM_NOAUCTION);
	}
      else
	{
	  send_to_char ("Auction channel is now OFF.\n\r", ch);
	  SET_BIT (ch->comm, COMM_NOAUCTION);
	}
      return;
    }
  if (!str_cmp (arg1, "stats"))
    {
      if (auction->item != NULL)
	{

	  // Removal of the ability to use the auction channel as a
	  // free hero level identify spell done June 22. 2000!
	  //
	  // Morgan.
	  if ((auction->seller == ch) && (!IS_IMMORTAL (ch)))
	    {
	      send_to_char
		("You can't see stats on objects you have auctioned.\n\r",
		 ch);
	      return;
	    }

	  /* show item data here */
	  if (auction->bid > 0)
	    sprintf (buf, "Current bid on this item is %d silver.\n\r",
		     auction->bid);

	  else
	    sprintf (buf, "No bids on this item have been received.\n\r");
	  send_to_char (buf, ch);
	  if (IS_IMMORTAL (ch))
	    {
	      sprintf (buf, "This item was auctioned by %s.\n\r",
		       auction->seller->name);
	      send_to_char (buf, ch);
	    }
	  // Imms can see who bids on an item
	  //
	  //
	  // Added by Morgan, June 22. 2000
	  if ((IS_IMMORTAL (ch) && auction->bid > 0))
	    {
	      sprintf (buf, "Last bid was made by %s.\n\r",
		       auction->buyer->name);
	      send_to_char (buf, ch);
	    }
	  spell_identify (0, LEVEL_HERO - 1, ch, auction->item, 0);
	  if (auction->item->item_type == ITEM_CARD)
	    appraise(ch,auction->item);
	  return;
	}
      else
	{
	  send_to_char ("What would you like to auction?\n\r", ch);
	  return;
	}
    }
  if (IS_IMMORTAL (ch) && !str_cmp (arg1, "stop"))
    {
      if (auction->item == NULL)
	{
	  send_to_char
	    ("There is no auction going on that you can stop.\n\r", ch);
	  return;
	}
      else
	/* stop the auction */
	{
	  sprintf (buf,
		   "Sale of %s has been stopped by God. Item confiscated.",
		   auction->item->short_descr);
	  talk_auction (buf);
	  obj_to_char (auction->item, ch);
	  auction->item = NULL;
	  if (auction->buyer != NULL)
	    {			/* return money to the buyer */

	      // Let the person have gold instead of silver
	      gold = auction->bid / 100;
	      silver = auction->bid - (gold * 100);
	      auction->buyer->gold += gold;
	      auction->buyer->silver += silver;

	      send_to_char ("Your money has been returned.\n\r",
			    auction->buyer);
	    }
	  return;
	}
    }
  if (!str_cmp (arg1, "bid"))
    {
      if (auction->item != NULL)
	{
	  int newbid;
	  if (ch->race == PC_RACE_SWARM)
	  {
	    send_to_char("The auctioneer refuses to accept your bid.\n\r",ch);
	    return;
	  }
	  // No more bidding on own items possible
	  // Added by Morgan, June 22. 2000
	  if ((auction->seller == ch) && (!IS_IMMORTAL (ch)))
	    {
	      send_to_char ("You cannot bid on your own items.\n\r", ch);
	      return;
	    }
	  if (IS_SET (ch->in_room->room_flags, ROOM_ARENA)
	      || IS_SET (ch->act, PLR_JAILED))
	    {
	      send_to_char
		("You cannot bid on auction items from your current location.\n\r",
		 ch);
	      return;
	    }
	  if (auction->item->item_type == ITEM_QUIVER && has_quiver (ch))
	    {
	      send_to_char
		("You already have a quiver, thus you may not bid.\n\r", ch);
	      return;
	    }

	  if (argument[0] == '\0')
	    {
	      send_to_char ("Bid how much?\n\r", ch);
	      return;
	    }
	  newbid = parsebid (auction->bid, argument);

	  // Minimum bid, or else bid not accepted
	  if (newbid < auction->minbid)
	    {
	      sprintf (buf,
		       "You must bid at least the minimum, %d silver.\n\r",
		       auction->minbid);
	      send_to_char (buf, ch);
	      return;
	    }
	  if (newbid < (auction->bid + 50))
	    {
	      send_to_char
		("You must bid at least 50 silver coins over the current bid.\n\r",
		 ch);
	      return;
	    }
	  if (newbid > (ch->silver + 100 * ch->gold))
	    {
	      send_to_char ("You don't have that much money!\n\r", ch);
	      return;
	    }
	  if (auction->buyer != NULL)
	    {

	      gold = auction->bid / 100;
	      silver = auction->bid - (gold * 100);
	      auction->buyer->gold += gold;
	      auction->buyer->silver += silver;
	    }
	  deduct_cost (ch, newbid);
	  auction->buyer = ch;
	  auction->bid = newbid;
	  auction->going = 0;
	  auction->pulse = PULSE_AUCTION;
	  sprintf (buf,
		   "A bid of `o%d`N silver has been received on %s.``",
		   newbid, auction->item->short_descr);
	  talk_auction (buf);
	  return;
	}
      else
	{
	  send_to_char
	    ("There isn't anything being auctioned right now.\n\r", ch);
	  return;
	}
    }
  if (IS_SET (ch->act, PLR_JAILED))
    {
      send_to_char
	("You cannot bid on auction items from your current location.\n\r",
	 ch);
      return;
    }
  if ((obj = get_obj_list (ch, arg1, ch->carrying)) == NULL)
    {
      send_to_char ("You aren't carrying that.\n\r", ch);
      return;
    }
  if ((obj->pIndexData->vnum >= OBJ_VNUM_ASHIELD
      && obj->pIndexData->vnum <= OBJ_VNUM_AINST) || IS_SET(obj->extra_flags[1],ITEM_NOAUCTION))
    {
      send_to_char ("You cannot auction that.\n\r", ch);
      return;
    }
  if (obj->wear_loc != -1)
    {
      send_to_char
	("You will have to remove that before you can auction it.\n\r", ch);
      return;
    }
  if (auction->item == NULL)
    switch (obj->item_type)
      {
      default:
	act ("You cannot auction $Ts.", ch, NULL, item_type_name (obj),
	     TO_CHAR);
	return;
      case ITEM_LIGHT:
      case ITEM_GILLS:
      case ITEM_INSTRUMENT:
      case ITEM_TREASURE:
      case ITEM_JEWELRY:
      case ITEM_WAND:
      case ITEM_STAFF:
      case ITEM_SADDLE:
      case ITEM_WEAPON:
      case ITEM_ARMOR:
      case ITEM_FURNITURE:
      case ITEM_CONTAINER:
      case ITEM_QUIVER:
      case ITEM_CLOTHING:
      case ITEM_WARP_STONE:

	//Iblish 5/12/03 - Added some more item types allowed for auciton at Iverath's request    
      case ITEM_MAP:
      case ITEM_KEY:
      case ITEM_SCROLL:
      case ITEM_PARCHMENT:
      case ITEM_PACK:
      case ITEM_PILL:
      case ITEM_VIAL:
      case ITEM_DRINK_CON:
      case ITEM_POTION:
      case ITEM_ROOM_KEY:
      case ITEM_CARD:
      case ITEM_PORTAL_BOOK:
	if (obj->timer > 0)
	  {
	    send_to_char
	      ("Auctioning decaying items is bad for your health.\n\r", ch);
	    return;
	  }
	if (obj->contains != NULL)
	  {
	    send_to_char
	      ("You're welcome to auction containers, but only if they are empty.\n\r",
	       ch);
	    return;
	  }
	// getting the argument for minimum bid
	// Morgan, June 22. 2000
	argument = one_argument (argument, arg2);
	if (arg2[0] != '\0' && !is_number (arg2))
	  {
	    send_to_char ("Minimum bid must be a number.\n\r", ch);
	    return;
	  }
	if (atoi(arg2) > 500000)
	{
  	  send_to_char("That minbid is excessive.\n\r",ch);
	  return;
	}

	auction->minbid = atoi (arg2);
	obj_from_char (obj);
	auction->item = obj;
	auction->bid = 0;
	auction->buyer = NULL;
	auction->seller = ch;
	auction->pulse = PULSE_AUCTION;
	auction->going = 0;

	// If a minimum bid is made, it is being announced on the
	// auction channel, else it is like normal
	// Changed/added by Morgan, June 22. 2000
	if (auction->minbid > 0)
	  {
	    sprintf (buf, "A new item has been received: %s.",
		     obj->short_descr);
	    talk_auction (buf);
	    sprintf (buf,
		     "The item is sold at a minimum bid of %d silver.",
		     auction->minbid);
	    talk_auction (buf);
	  }
	else
	  {
	    sprintf (buf, "A new item has been received: %s.",
		     obj->short_descr);
	    talk_auction (buf);
	  }
	return;
      }
  /* switch */
  else
    {
      act ("Try again later - $p is being auctioned right now!", ch,
	   auction->item, NULL, TO_CHAR);
      return;
    }
}
void do_fish (CHAR_DATA * ch, char *argument)
{
  sh_int fish_skill, fish_vnum;
  bool fishok = FALSE;
  OBJ_DATA *obj;
  if (ch->position == POS_FIGHTING || ch->fighting != NULL)
    {
      send_to_char ("No way!  You are still fighting!\n\r", ch);
      return;
    }
  fish_skill = get_skill (ch, gsn_fishing);
  if (fish_skill < 1)
    {
      send_to_char ("You don't know how to fish.\n\r", ch);
      return;
    }
  if (ch->mana < 5)
    {
      send_to_char ("Sorry, you don't have enough mana to fish.\n\r", ch);
      return;
    }
  if (ch->in_room->sector_type != SECT_WATER_SWIM
      && ch->in_room->sector_type != SECT_WATER_NOSWIM
      && ch->in_room->sector_type != SECT_UNDERWATER
      && ch->in_room->sector_type != SECT_WATER_OCEAN
      && ch->in_room->sector_type != SECT_SWAMP)
    {
      if (IS_SET (ch->in_room->room_flags, ROOM_TRANSPORT))
	{
	  OBJ_DATA *eObj;
	  for (eObj = object_list; eObj; eObj = eObj->next)
	    {
	      if (eObj->item_type != ITEM_CTRANSPORT)
		continue;
	      if (eObj->value[3] == ch->in_room->vnum)
		{
		  ROOM_INDEX_DATA *location;
		  location = eObj->in_room;
		  if (location != NULL
		      && (location->sector_type ==
			  SECT_WATER_OCEAN || location->
			  sector_type ==
			  SECT_WATER_SWIM || location->sector_type ==
			  SECT_WATER_NOSWIM) && !IS_SET (ch->in_room->
							 room_flags,
							 ROOM_INDOORS))
		    fishok = TRUE;
		  break;
		}
	    }
	}
      if (!fishok)
	{
	  send_to_char ("You can't fish here.\n\r", ch);
	  return;
	}
    }
  if (fish_skill < number_percent ())
    {
      act ("You failed to catch any fish.", ch, NULL, NULL, TO_CHAR);
      act ("$n attempts to catch a fish, but fails.", ch, NULL, NULL,
	   TO_ROOM);
      check_improve (ch, gsn_fishing, FALSE, 2);
      return;
    }
  if (fish_skill <= 33)
    fish_vnum = OBJ_VNUM_FISHIE_SMALL;

  else if (fish_skill <= 66)
    fish_vnum = OBJ_VNUM_FISHIE_MEDIUM;

  else
    fish_vnum = OBJ_VNUM_FISHIE_LARGE;
  if (number_percent () < 10 || ch->in_room->sector_type == SECT_SWAMP)
    fish_vnum = OBJ_VNUM_FISHIE_POISON;
  ch->mana = UMAX (0, ch->mana - 5);
  obj = create_object (get_obj_index (fish_vnum), 0);
  act ("$n has caught $p.", ch, obj, NULL, TO_ROOM);
  act ("You have caught $p.", ch, obj, NULL, TO_CHAR);
  obj_to_char (obj, ch);
  WAIT_STATE (ch, PULSE_VIOLENCE);
  check_improve (ch, gsn_fishing, TRUE, 2);
}


// Minax 7-10-02 Assassin skill Tripwire
void do_tripwire (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *wire;
  OBJ_DATA *rope;
  OBJ_DATA *obj;
  short tripwire_skill;
  tripwire_skill = get_skill (ch, gsn_tripwire);
  if (tripwire_skill < 1)
    {
      send_to_char
	("You don't know how anything about setting tripwires.\n\r", ch);
      return;
    }
  if (ch->mana < 20)
    {
      send_to_char
	("You don't have enough energy to set a tripwire right now..\n\r",
	 ch);
      return;
    }
  rope = get_eq_char (ch, WEAR_HOLD);
  if (rope == NULL || (rope->pIndexData->vnum != OBJ_VNUM_ROPE))
    {
      send_to_char ("You must be holding rope to make a tripwire.\n\r", ch);
      return;
    }
  for (obj = ch->in_room->contents; obj; obj = obj->next_content)
    if (obj->pIndexData->vnum == OBJ_VNUM_TRIPWIRE)
      {
	send_to_char ("A tripwire has already been set up in here.\n\r", ch);
	return;
      }
  ch->mana = UMAX (0, ch->mana - 20);
  WAIT_STATE (ch, PULSE_VIOLENCE);
  if (tripwire_skill < number_percent ())
    {
      act ("You are unable to contruct anything useful.", ch, NULL, NULL,
	   TO_CHAR);
      act ("$n attempts to set a tripwire, but fails miserably.", ch,
	   NULL, NULL, TO_ROOM);
      extract_obj (rope);
      check_improve (ch, gsn_tripwire, FALSE, 2);
      return;
    }
  wire = create_object (get_obj_index (OBJ_VNUM_TRIPWIRE), 0);
  wire->timer = (ch->level / 10);
  obj_to_room (wire, ch->in_room);
  act ("$n has constructed $p.", ch, wire, NULL, TO_ROOM);
  act ("You have constructed $p.", ch, wire, NULL, TO_CHAR);
  extract_obj (rope);
  check_improve (ch, gsn_tripwire, TRUE, 2);
  return;
}

void do_forage (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj;
  short forage_skill;
  int forage_vnum;
  short load_lev = 0;
  forage_skill = get_skill (ch, gsn_foraging);
  if (forage_skill < 1)
    {
      send_to_char ("You don't know how to forage.\n\r", ch);
      return;
    }
  if (ch->mana < 20)
    {
      send_to_char ("Sorry, you don't have enough mana to forage.\n\r", ch);
      return;
    }
  ch->mana = UMAX (0, ch->mana - 20);
  WAIT_STATE (ch, PULSE_VIOLENCE);
  if (ch->in_room->sector_type != SECT_FOREST
      && ch->in_room->sector_type != SECT_FIELD
      && ch->in_room->sector_type != SECT_HILLS
      && ch->in_room->sector_type != SECT_GRASSLAND)
    {
      act ("You find nothing.", ch, NULL, NULL, TO_CHAR);
      act ("$n attempts to forage for food, but finds nothing.", ch,
	   NULL, NULL, TO_ROOM);
      return;
    }
  if (forage_skill < number_percent ())
    {
      act ("You find nothing.", ch, NULL, NULL, TO_CHAR);
      act ("$n attempts to forage for food, but finds nothing.", ch,
	   NULL, NULL, TO_ROOM);
      check_improve (ch, gsn_foraging, FALSE, 2);
      return;
    }
  if (forage_skill <= 5)
    {
      forage_vnum = OBJ_VNUM_SPROUTS;
      load_lev = ch->level;
    }
  else if (forage_skill <= 10)
    {
      forage_vnum = OBJ_VNUM_BLUEGREEN_HERBS;
      load_lev = ch->level;
    }
  else
    forage_vnum = number_range (OBJ_VNUM_BEGIN_GFORAGE, OBJ_VNUM_END_GFORAGE);
  if (number_range (1, 10) == 4)
    forage_vnum = OBJ_VNUM_STICKS;
  obj = create_object (get_obj_index (forage_vnum), load_lev);
  obj_to_char (obj, ch);
  act ("$n has found $p.", ch, obj, NULL, TO_ROOM);
  act ("You have found $p.", ch, obj, NULL, TO_CHAR);
  check_improve (ch, gsn_foraging, TRUE, 2);
}

bool can_loot (CHAR_DATA * ch, OBJ_DATA * obj)
{
  CHAR_DATA *owner, *wch;
  if (IS_IMMORTAL (ch))
    return TRUE;
  if (!obj->owner || obj->owner == NULL)
    return TRUE;
  owner = NULL;
  for (wch = char_list; wch != NULL; wch = wch->next)
    if (!str_cmp (wch->name, obj->owner))
      owner = wch;
  if (owner == NULL)
    return TRUE;
  if (!str_cmp (ch->name, owner->name))
    return TRUE;
  if (!IS_NPC (owner) && IS_SET (owner->act, PLR_CANLOOT))
    return TRUE;
  if (is_same_group (ch, owner))
    return TRUE;
  return FALSE;
}

void do_press (CHAR_DATA * ch, char *argument)
{
  char button_name[MAX_INPUT_LENGTH];
  OBJ_DATA *bObj, *eObj;
  ELEVATOR_DEST_LIST *edest, *odest, *tdest;
  bool found = FALSE;
  one_argument (argument, button_name);
  if (button_name[0] == '\0')
    {
      send_to_char ("Press what?\n\r", ch);
      return;
    }
  if ((bObj = get_obj_here (ch, button_name)) == NULL)
    {
      send_to_char ("You don't see that here.\n\r", ch);
      return;
    }
  if (bObj->item_type != ITEM_EBUTTON)
    {
      act ("You press $p.", ch, bObj, NULL, TO_CHAR);
      act ("$n presses $p.", ch, bObj, NULL, TO_ROOM);
      return;
    }
  if (bObj->value[3] != 0 && !IS_SET (bObj->value[3], BUTTON_PRESS))
    {
      act ("You press $p.", ch, bObj, NULL, TO_CHAR);
      act ("$n presses $p.", ch, bObj, NULL, TO_ROOM);
      return;
    }
  for (eObj = object_list; eObj != NULL; eObj = eObj->next)
    {
      if (eObj->item_type != ITEM_ELEVATOR)
	continue;
      if (eObj->pIndexData->vnum == bObj->value[0])
	{
	  found = TRUE;
	  break;
	}
    }
  if (!found)
    {
      act ("You press $p.", ch, bObj, NULL, TO_CHAR);
      act ("$n presses $p.", ch, bObj, NULL, TO_ROOM);
      return;
    }
  found = FALSE;
  for (edest = eObj->dest_list; edest; edest = edest->next)
    {
      if (edest->vnum == bObj->value[2])
	{
	  found = TRUE;
	  break;
	}
    }
  if (found)
    {
      act ("Pressing $p again won't help.", ch, bObj, NULL, TO_CHAR);
      return;
    }
  act ("You press $p.", ch, bObj, NULL, TO_CHAR);
  act ("$n presses $p.", ch, bObj, NULL, TO_ROOM);

  trip_triggers(ch, OBJ_TRIG_ACTION_WORD, bObj, NULL, OT_SPEC_PRESS);

  if ((edest = malloc (sizeof (struct elevator_dest_list))) == NULL)
//  if ((edest = reinterpret_cast<ELEVATOR_DEST_LIST*>(malloc(sizeof(struct elevator_dest_list)))) == NULL)
    return;
  edest->vnum = (sh_int) bObj->value[2];
  edest->arrival_msg_i = str_dup (bObj->pIndexData->string1);
  edest->arrival_msg_o = str_dup (bObj->pIndexData->string2);
  edest->departure_msg_i = str_dup (bObj->pIndexData->string3);
  edest->departure_msg_o = str_dup (bObj->pIndexData->string4);
  odest = eObj->dest_list;
  if (odest != NULL)
    tdest = odest->next;

  else
    {
      edest->next = NULL;
      eObj->dest_list = edest;
      return;
    }
  if (tdest == NULL)
    {
      odest->next = edest;
      edest->next = NULL;
      return;
    }
  while (1)
    {
      if (edest->vnum < UMAX (odest->vnum, tdest->vnum)
	  && edest->vnum > UMIN (odest->vnum, tdest->vnum))
	{
	  odest->next = edest;
	  edest->next = tdest;
	  return;
	}
      odest = tdest;
      tdest = odest->next;
      if (tdest == NULL)
	{
	  edest->next = NULL;
	  odest->next = edest;
	  return;
	}
    }
}
void do_pull (CHAR_DATA * ch, char *argument)
{
  char button_name[MAX_INPUT_LENGTH];
  OBJ_DATA *bObj, *eObj;
  ELEVATOR_DEST_LIST *edest, *odest, *tdest;
  bool found = FALSE;
  one_argument (argument, button_name);
  if (button_name[0] == '\0')
    {
      send_to_char ("Pull what?\n\r", ch);
      return;
    }
  if ((bObj = get_obj_here (ch, button_name)) == NULL)
    {
      send_to_char ("You don't see that here.\n\r", ch);
      return;
    }
  if (bObj->item_type != ITEM_EBUTTON)
    {
      act ("You pull at $p.", ch, bObj, NULL, TO_CHAR);
      act ("$n pulls at $p.", ch, bObj, NULL, TO_ROOM);
      trip_triggers(ch, OBJ_TRIG_ACTION_WORD, bObj, NULL, OT_SPEC_PULL);
      return;
    }
  if (!IS_SET (bObj->value[3], BUTTON_PULL))
    {
      act ("You pull at $p.", ch, bObj, NULL, TO_CHAR);
      act ("$n pulls at $p.", ch, bObj, NULL, TO_ROOM);
      trip_triggers(ch, OBJ_TRIG_ACTION_WORD, bObj, NULL, OT_SPEC_PULL);
      return;
    }
  for (eObj = object_list; eObj != NULL; eObj = eObj->next)
    {
      if (eObj->item_type != ITEM_ELEVATOR)
	continue;
      if (eObj->pIndexData->vnum == bObj->value[0])
	{
	  found = TRUE;
	  break;
	}
    }
  if (!found)
    {
      act ("You pull at $p.", ch, bObj, NULL, TO_CHAR);
      act ("$n pulls at $p.", ch, bObj, NULL, TO_ROOM);
      trip_triggers(ch, OBJ_TRIG_ACTION_WORD, bObj, NULL, OT_SPEC_PULL);
      return;
    }
  found = FALSE;
  for (edest = eObj->dest_list; edest; edest = edest->next)
    {
      if (edest->vnum == bObj->value[2])
	{
	  found = TRUE;
	  break;
	}
    }
  if (found)
    {
      act ("Pulling $p again won't help.", ch, bObj, NULL, TO_CHAR);
      return;
    }
  act ("You pull $p.", ch, bObj, NULL, TO_CHAR);
  act ("$n pulls $p.", ch, bObj, NULL, TO_ROOM);
  trip_triggers(ch, OBJ_TRIG_ACTION_WORD, bObj, NULL, OT_SPEC_PULL);

  if ((edest = malloc (sizeof (struct elevator_dest_list))) == NULL)
//  if ((edest = reinterpret_cast<ELEVATOR_DEST_LIST *>(malloc(sizeof(struct elevator_dest_list)))) == NULL)
    return;
  edest->vnum = (sh_int) bObj->value[2];
  edest->arrival_msg_i = str_dup (bObj->pIndexData->string1);
  edest->arrival_msg_o = str_dup (bObj->pIndexData->string2);
  edest->departure_msg_i = str_dup (bObj->pIndexData->string3);
  edest->departure_msg_o = str_dup (bObj->pIndexData->string4);
  odest = eObj->dest_list;
  if (odest != NULL)
    tdest = odest->next;

  else
    {
      edest->next = NULL;
      eObj->dest_list = edest;
      return;
    }
  if (tdest == NULL)
    {
      odest->next = edest;
      edest->next = NULL;
      return;
    }
  while (1)
    {
      if (edest->vnum < UMAX (odest->vnum, tdest->vnum)
	  && edest->vnum > UMIN (odest->vnum, tdest->vnum))
	{
	  odest->next = edest;
	  edest->next = tdest;
	  return;
	}
      odest = tdest;
      tdest = odest->next;
      if (tdest == NULL)
	{
	  edest->next = NULL;
	  odest->next = edest;
	  return;
	}
    }
}
void do_elist (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *eObj;
  ELEVATOR_DEST_LIST *edest;
  char tcbuf[MAX_STRING_LENGTH];
  one_argument (argument, tcbuf);
  eObj = get_obj_here (ch, tcbuf);
  if (eObj == NULL)
    {
      send_to_char ("No Elevator.\n\r", ch);
      return;
    }
  for (edest = eObj->dest_list; edest; edest = edest->next)
    {
      sprintf (tcbuf, "Destination: %d\n\r", edest->vnum);
      send_to_char (tcbuf, ch);
    }
}

// NOTE: this function is copied into spell_gravity and check_gravity in magic2.c, 
// so if you add something here, add something there too
void get_obj (CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container)
{

  /* variables for AUTOSPLIT */
  CHAR_DATA *gch;
  int members;
  char buffer[100];
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *cntr;
  if (!CAN_WEAR (obj, ITEM_TAKE))
    {
      send_to_char ("You can't take that.\n\r", ch);
      return;
    }
  if (IS_SET(obj->extra_flags[0],ITEM_NOMOB) && IS_NPC(ch))
  {
	  send_to_char ("You can't take that.\n\r",ch);
	  return;
  }
  if (obj->plr_owner != NULL)
    if (str_cmp (ch->name, obj->plr_owner) && !IS_IMMORTAL(ch))
      {
	sprintf (buf, "You do not own %s!\n\r", obj->short_descr);
	send_to_char (buf, ch);
	return;
      }

  if (ch->carry_number + get_obj_number (obj) > can_carry_n (ch))
    {
      act ("$d: you can't carry that many items.", ch, NULL, obj->name,
	   TO_CHAR);
      return;
    }
  if (obj->item_type == ITEM_QUIVER && has_quiver (ch))
    {
      send_to_char ("You already have a quiver.\n\r", ch);
      return;
    }
  if (obj->item_type == ITEM_PORTAL)
    adjust_portal_weight (obj);
  if ((!obj->in_obj || obj->in_obj->carried_by != ch)
      && (get_carry_weight (ch) + get_obj_weight (obj) > can_carry_w (ch)))
    {
      act ("$d: you can't carry that much weight.", ch, NULL, obj->name,
	   TO_CHAR);
      return;
    }
  if (!IS_NPC (ch))
    {
      if (ch->mount != NULL)
	if (get_char_weight (ch) + get_obj_weight (obj) >
	    ch->mount->pIndexData->max_weight)
	  {
	    act ("$d: your mount can't carry that much weight.", ch,
		 NULL, obj->name, TO_CHAR);
	    return;
	  }
    }
  if (obj->in_room != NULL)
    {
      for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	if (gch->on == obj)
	  {
	    act ("$N appears to be using $p.", ch, obj, gch, TO_CHAR);
	    return;
	  }
    }
  if (container != NULL)
    {
      if (container->pIndexData->vnum == OBJ_VNUM_PIT
	  && get_trust (ch) < obj->level - 10)
	{
	  send_to_char ("You are not powerful enough to use it.\n\r", ch);
	  return;
	}
      if (container->pIndexData->vnum == OBJ_VNUM_PENTAGRAM 
		      && container->plr_owner && str_cmp(container->plr_owner,ch->name))
      {
        send_to_char("Only the necromancer who drew that pentagram can get objects from it.\n\r",ch);
	return;
      }
		      
      if (container->pIndexData->vnum == OBJ_VNUM_PIT
	  && !CAN_WEAR (container, ITEM_TAKE)
	  && !IS_OBJ_STAT (obj, ITEM_HAD_TIMER))
	obj->timer = 0;
      if (container->pIndexData->vnum == OBJ_VNUM_CORPSE_PC && !IS_IMMORTAL(ch))
      {
	if (!IS_NPC(ch) && (current_time - ch->pcdata->last_pkdeath < (60*MULTIKILL_WAIT_TIME)))
          {
             char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
             sprintf(buf,"the corpse of %s1",ch->name);
             strcpy(buf2,container->short_descr);
             strcat(buf2,"1");
             if (strcmp(buf,buf2))
               {
                 send_to_char("You cannot loot unless you are able to be killed.\n\r",ch);
                 return;
               }
          }


	      
      }

      if (container->pIndexData->vnum == OBJ_VNUM_PENTAGRAM)
      {
        act("$p levitates into the hands of $n.",ch,obj,container,TO_ROOM);
        act("$p levitates into your hands.",ch,obj,container,TO_CHAR);
      }
      else
      {
        act ("You get $p from $P.", ch, obj, container, TO_CHAR);
        act ("$n gets $p from $P.", ch, obj, container, TO_ROOM);
      }
      REMOVE_BIT (obj->extra_flags[0], ITEM_HAD_TIMER);
      cntr = obj->in_obj;
      obj_from_obj (obj);
/*      if (container->item_type == ITEM_QUIVER
	  && obj->item_type == ITEM_WEAPON && obj->value[0] == WEAPON_ARROW)
	ch->carry_number++;*/
      if ((cntr != NULL) &&
	  ((cntr->item_type == ITEM_CLAN_DONATION) ||
	   (cntr->item_type == ITEM_NEWCLANS_DBOX)))
	save_clan_box (cntr);
      if (cntr != NULL && cntr->item_type == ITEM_PLAYER_DONATION)
	save_player_box (cntr);
      if (cntr != NULL && cntr->pIndexData->vnum == OBJ_VNUM_CORPSE_PC)
        save_player_corpse (cntr);
	      
    }
  else
    {
      act ("You get $p.", ch, obj, container, TO_CHAR);
      act ("$n gets $p.", ch, obj, container, TO_ROOM);
      obj_from_room (obj);
    }
  if (obj->item_type == ITEM_MONEY)
    {
      ch->silver += obj->value[0];
      ch->gold += obj->value[1];
      if (IS_SET (ch->act, PLR_AUTOSPLIT))
	{			/* AUTOSPLIT code */
	  members = 0;
	  for (gch = ch->in_room->people; gch != NULL;
	       gch = gch->next_in_room)
	    {

/*
 * Mathew: Added !IS_NPC(gch) to be sure uncharmed NPC's and mounts
 * do not get a share of the money. 4/13/99
 */
	      if (is_same_group (gch, ch)
		  && !IS_AFFECTED (gch, AFF_CHARM) && !IS_NPC (gch))
		members++;
	    }
	  if (members > 1 && (obj->value[0] > 1 || obj->value[1]))
	    {
	      sprintf (buffer, "%d %d", obj->value[0], obj->value[1]);
	      do_split (ch, buffer);
	    }
	}
      extract_obj (obj);
    }
  else
    {
      obj_to_char (obj, ch);
      if (obj != NULL && obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC)
       save_player_corpse (obj);
      trip_triggers(ch, OBJ_TRIG_GET, obj, NULL, OT_SPEC_NONE);
    }
  return;
}

void plant_obj (CHAR_DATA * ch, char *argument, CHAR_DATA * victim)
{
  char arg[MAX_INPUT_LENGTH], arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj, *obj_next;
  bool found = FALSE;
  if (victim == NULL)
    {
      argument = one_argument (argument, arg);
      if (arg[0] == '\0')
	{
	  send_to_char ("Plant what?\n\r", ch);
	  return;
	}
      if (IS_SET (ch->act, PLR_WANTED))
	{
	  send_to_char ("You can't do that while you are wanted.\n\r", ch);
	  return;
	}
      if (is_number (arg))
	{

	  /* 'drop NNNN coins' */
	  int amount, gold = 0, silver = 0;
	  amount = atoi (arg);
	  argument = one_argument (argument, arg);
	  if (amount <= 0
	      || (str_cmp (arg, "coins") && str_cmp (arg, "coin")
		  && str_cmp (arg, "gold") && str_cmp (arg, "silver")))
	    {
	      send_to_char ("Sorry, you can't do that.\n\r", ch);
	      return;
	    }
	  if (!str_cmp (arg, "coins") || !str_cmp (arg, "coin")
	      || !str_cmp (arg, "silver"))
	    {
	      if (ch->silver < amount)
		{
		  send_to_char ("You don't have that much silver.\n\r", ch);
		  return;
		}
	      ch->silver -= amount;
	      silver = amount;
	    }

	  else
	    {
	      if (ch->gold < amount)
		{
		  send_to_char ("You don't have that much gold.\n\r", ch);
		  return;
		}
	      ch->gold -= amount;
	      gold = amount;
	    }
	  for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
	    {
	      obj_next = obj->next_content;
	      switch (obj->pIndexData->vnum)
		{
		case OBJ_VNUM_SILVER_ONE:
		  silver += 1;
		  extract_obj (obj);
		  break;
		case OBJ_VNUM_GOLD_ONE:
		  gold += 1;
		  extract_obj (obj);
		  break;
		case OBJ_VNUM_SILVER_SOME:
		  silver += obj->value[0];
		  extract_obj (obj);
		  break;
		case OBJ_VNUM_GOLD_SOME:
		  gold += obj->value[1];
		  extract_obj (obj);
		  break;
		case OBJ_VNUM_COINS:
		  silver += obj->value[0];
		  gold += obj->value[1];
		  extract_obj (obj);
		  break;
		}
	    }
	  obj_to_room (create_money (gold, silver), ch->in_room);
	  act("$n plants some money on the ground here.",ch,NULL,NULL,TO_IIR);
	  send_to_char ("Ok.\n\r", ch);
	  return;
	}
      if (str_cmp (arg, "all") && str_prefix ("all.", arg))
	{

	  /* 'drop obj' */
	  if ((obj = get_obj_carry (ch, arg)) == NULL)
	    {
	      send_to_char ("You do not have that item.\n\r", ch);
	      return;
	    }
	  if (!can_drop_obj (ch, obj) && !IS_NPC(ch))
	    {
	      send_to_char ("You can't let go of it.\n\r", ch);
	      return;
	    }
	  obj_from_char (obj);
	  obj_to_room (obj, ch->in_room);
	  act ("You plant $p here.", ch, obj, NULL, TO_CHAR);
	  act ("$n plants $p here.", ch, obj, NULL, TO_IIR);
	  if (IS_OBJ_STAT (obj, ITEM_MELT_DROP))
	    {
	      act ("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
	      extract_obj (obj);
	    }
	}
      else
	{

	  /* 'drop all' or 'drop all.obj' */
	  found = FALSE;
	  for (obj = ch->carrying; obj != NULL; obj = obj_next)
	    {
	      obj_next = obj->next_content;
	      if ((arg[3] == '\0' || is_name (&arg[4], obj->name))
		  && can_see_obj (ch, obj)
		  && obj->wear_loc == WEAR_NONE && can_drop_obj (ch, obj))
		{
		  found = TRUE;
		  obj_from_char (obj);
		  obj_to_room (obj, ch->in_room);
		  act ("You plant $p here.", ch, obj, NULL, TO_CHAR);
		  act ("$n plants $p here.", ch, obj, NULL, TO_IIR);
		  if (IS_OBJ_STAT (obj, ITEM_MELT_DROP))
		    {
		      act ("$p dissolves into smoke.", ch, obj, NULL,
			   TO_CHAR);
		      extract_obj (obj);
		    }
		}
	    }
	  if (!found)
	    {
	      if (arg[3] == '\0')
		act ("You are not carrying anything.", ch, NULL, arg,
		     TO_CHAR);

	      else
		act ("You are not carrying any $T.", ch, NULL,
		     &arg[4], TO_CHAR);
	    }
	}
      return;
    }
  else
    {
      argument = one_argument (argument, arg1);
      if (arg1[0] == '\0')
	{
	  send_to_char ("Plant what on whom?\n\r", ch);
	  return;
	}
      if (IS_SET (ch->act, PLR_WANTED))
	{
	  send_to_char ("You can't do that while you are wanted.\n\r", ch);
	  return;
	}
      
      if (is_number (arg1))
	{

	  /* 'give NNNN coins victim' */
	  int amount;
	  bool silver;
	  amount = atoi (arg1);
	  if (amount <= 0
	      || (str_cmp (arg2, "coins") && str_cmp (arg2, "coin")
		  && str_cmp (arg2, "gold") && str_cmp (arg2, "silver")))
	    {
	      send_to_char ("Sorry, you can't do that.\n\r", ch);
	      return;
	    }
	  silver = str_cmp (arg2, "gold");
	  if ((!silver && ch->gold < amount)
	      || (silver && ch->silver < amount))
	    {
	      send_to_char ("You haven't got that much.\n\r", ch);
	      return;
	    }
	  if (silver)
	    {
	      ch->silver -= amount;
	      victim->silver += amount;
	    }
	  else
	    {
	      ch->gold -= amount;
	      victim->gold += amount;
	    }
	  sprintf (buf, "You plant %d %s on $N.", amount,
		   silver ? "silver" : "gold");
	  act (buf, ch, NULL, victim, TO_CHAR);
	   sprintf (buf, "$n plants %d %s on $N.", amount, silver ? "silver" : "gold");
	  act (buf, ch, NULL, victim, TO_IIR);
	  if (IS_NPC (victim) && IS_SET (victim->act, ACT_IS_CHANGER))
	    {
	      int change;
	      change = (silver ? 95 * amount / 100 / 100 : 95 * amount);
	      if (!silver && change > victim->silver)
		victim->silver += change;
	      if (silver && change > victim->gold)
		victim->gold += change;
	      if (change < 1 && can_see (victim, ch))
		{
		  act
		    ("$n tells you 'I'm sorry, you did not give me enough to change.'",
		     victim, NULL, ch, TO_VICT);
		  ch->reply = victim;
		  sprintf (buf, "%d %s %s", amount,
			   silver ? "silver" : "gold", ch->name);
		  do_give (victim, buf);
		}
	      else if (can_see (victim, ch))
		{
		  sprintf (buf, "%d %s %s", change,
			   silver ? "gold" : "silver", ch->name);
		  do_give (victim, buf);
		  if (silver)
		    {
		      sprintf (buf, "%d silver %s",
			       (95 * amount / 100 - change * 100), ch->name);
		      do_give (victim, buf);
		    }
		  act ("$n tells you 'Thank you, come again.'", victim,
		       NULL, ch, TO_VICT);
		  ch->reply = victim;
		}
	    }
	  return;
	}
      if ((obj = get_obj_carry (ch, arg1)) == NULL)
	{
	  send_to_char ("You do not have that item.\n\r", ch);
	  return;
	}
      if (obj->wear_loc != WEAR_NONE)
	{
	  send_to_char ("You must remove it first.\n\r", ch);
	  return;
	}
      if (IS_NPC (victim) && victim->pIndexData->pShop != NULL)
	{
	  act ("$N tells you 'Sorry, you'll have to sell that.'", ch,
	       NULL, victim, TO_CHAR);
	  ch->reply = victim;
	  return;
	}
      if (IS_NPC(victim) && IS_SET(obj->extra_flags[0],ITEM_NOMOB))
        {
          send_to_char("They are not allowed to have that item.\n\r",ch);
          return;
        }
       
      if (!can_drop_obj (ch, obj))
	{
	  send_to_char ("You can't let go of it.\n\r", ch);
	  return;
	}
      if (obj->item_type == ITEM_QUIVER && has_quiver (victim))
	{
	  act ("$N already has a quiver.\n\r", ch, NULL, victim, TO_CHAR);
	  return;
	}
      if (victim->carry_number + get_obj_number (obj) > can_carry_n (victim))
	{
	  act ("$N has $S hands full.", ch, NULL, victim, TO_CHAR);
	  return;
	}
      if (obj->item_type == ITEM_PORTAL)
	adjust_portal_weight (obj);
      if (get_carry_weight (victim) + get_obj_weight (obj) >
	  can_carry_w (victim))
	{
	  act ("$N can't carry that much weight.", ch, NULL, victim, TO_CHAR);
	  return;
	}
      if (!IS_NPC (victim))
	{
	  if (victim->mount != NULL)
	    if (get_char_weight (victim) + get_obj_weight (obj) >
		victim->mount->pIndexData->max_weight)
	      {
		act ("$N's mount can't carry that much weight.", ch,
		     NULL, victim, TO_CHAR);
		return;
	      }
	}
      obj_from_char (obj);
      obj_to_char (obj, victim);
      act ("$n plants $p on $N.", ch, obj, victim, TO_IIR);
      act ("You plant $p on $N.", ch, obj, victim, TO_CHAR);
      return;
    }
}
void palm_obj (CHAR_DATA * ch, OBJ_DATA * obj)
{
  CHAR_DATA *gch;
  int members;
  char buffer[100];
  char buf[MAX_STRING_LENGTH];
  if (!CAN_WEAR (obj, ITEM_TAKE))
    {
      send_to_char ("You can't take that.\n\r", ch);
      return;
    }
  if (IS_SET(obj->extra_flags[0],ITEM_NOMOB) && IS_NPC(ch))
    {
      send_to_char ("You can't take that.\n\r",ch);
      return;
    }  
  if (obj->plr_owner != NULL)
	  if (str_cmp (ch->name, obj->plr_owner) && !IS_IMMORTAL(ch))
	  {
		  sprintf (buf, "You do not own %s!\n\r", obj->short_descr);
		  send_to_char (buf, ch);
		  return;
	  }
  
  if (ch->carry_number + get_obj_number (obj) > can_carry_n (ch))
    {
      act ("$d: you can't carry that many items.", ch, NULL, obj->name,
	   TO_CHAR);
      return;
    }
  if (obj->item_type == ITEM_PORTAL)
    adjust_portal_weight (obj);
  if (get_carry_weight (ch) + get_obj_weight (obj) > can_carry_w (ch))
    {
      act ("$d: you can't carry that much weight.", ch, NULL, obj->name,
	   TO_CHAR);
      return;
    }
  if (obj->in_room != NULL)
    {
      for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	if (gch->on == obj)
	  {
	    act ("$N appears to be using $p.", ch, obj, gch, TO_CHAR);
	    return;
	  }
    }
  act ("You palm $p.", ch, obj, NULL, TO_CHAR);
  act ("$n just palmed $p.", ch, obj, NULL, TO_IIR);
  obj_from_room (obj);
  if (obj->item_type == ITEM_MONEY)
    {
      ch->silver += obj->value[0];
      ch->gold += obj->value[1];
      if (IS_SET (ch->act, PLR_AUTOSPLIT))
	{			/* AUTOSPLIT code */
	  members = 0;
	  for (gch = ch->in_room->people; gch != NULL;
	       gch = gch->next_in_room)
	    {
	      if (!IS_AFFECTED (gch, AFF_CHARM) && is_same_group (gch, ch))
		members++;
	    }
	  if (members > 1 && (obj->value[0] > 1 || obj->value[1]))
	    {
	      sprintf (buffer, "%d %d", obj->value[0], obj->value[1]);
	      do_split (ch, buffer);
	    }
	}
      extract_obj (obj);
    }
  else
    {
      obj_to_char (obj, ch);
      if (obj != NULL && obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC)
        save_player_corpse (obj);
      
    }
  return;
}

void do_get (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  OBJ_DATA *container;
  bool found;
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (!str_cmp (arg2, "from"))
    argument = one_argument (argument, arg2);

  /* Get type. */
  if (arg1[0] == '\0')
    {
      send_to_char ("Get what?\n\r", ch);
      return;
    }
  if (arg2[0] == '\0')
    {
      if (str_cmp (arg1, "all") && str_prefix ("all.", arg1))
	{

	  /* 'get obj' */
	  obj = get_obj_list (ch, arg1, ch->in_room->contents);
	  if (obj == NULL)
	    {
	      act ("You see no $T here.", ch, NULL, arg1, TO_CHAR);
	      return;
	    }
	  get_obj (ch, obj, NULL);
	  //Iblis - 3/11/04 - Antiduping Code
	  if (!IS_NPC(ch))
	    save_char_obj (ch);
	} 
      else
	{

	  /* 'get all' or 'get all.obj' */
	  found = FALSE;
	  for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
	    {
	      obj_next = obj->next_content;
	      if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
		  && can_see_obj (ch, obj))
		{
		  found = TRUE;
		  get_obj (ch, obj, NULL);
		}
	    }

          
	  if (!found)
	    {
	      if (arg1[3] == '\0')
		send_to_char ("You see nothing here.\n\r", ch);

	      else
		act ("You see no $T here.", ch, NULL, &arg1[4], TO_CHAR);
	    }
	  //Iblis - 3/11/04 - Antiduping Code
	  else if (!IS_NPC(ch))
	    save_char_obj (ch);
	}
    }
  else
    {
      /* get XX silver/gold */
      if (is_number(arg1))
      {
        if(str_prefix(arg2, "silver") && !str_prefix(arg2, "gold") && !str_prefix(arg2,"coins"))
	{
	  send_to_char ("Syntax: get X silver/gold\n\r",ch);
	  return;
	}
	for (obj = ch->in_room->contents; obj != NULL; obj = obj->next)
	{
	  if (obj->pIndexData->vnum <= 5 && obj->pIndexData->vnum >= 1)
	  {
	    int amount = 0;
	    switch (obj->pIndexData->vnum)
	      {  
	      case OBJ_VNUM_SILVER_ONE: 
		if (atoi(arg1) != 1 || !str_prefix(arg2,"gold"))
		  {
		    send_to_char("You don't see that much here!\n\r",ch);
		    return;
		  }
		++ch->silver;
		act("You get $p.",ch,obj,NULL,TO_CHAR);
		act("$n gets $p.",ch,obj,NULL,TO_ROOM);
		obj_from_room(obj);
		extract_obj(obj);
		obj = NULL;
		break;
	      case OBJ_VNUM_GOLD_ONE:
		if (atoi(arg1) != 1 || str_prefix(arg2,"gold"))
		  {
		    send_to_char("You don't see that much here!\n\r",ch);
		    return;
		  }
		++ch->gold;
		act("You get $p.",ch,obj,NULL,TO_CHAR);
		act("$n gets $p.",ch,obj,NULL,TO_ROOM);
		obj_from_room(obj);
		extract_obj(obj);
		obj = NULL;
		break;
	     case OBJ_VNUM_SILVER_SOME:
	       if ((amount = atoi(arg1)) > obj->value[0] || !str_prefix(arg2,"gold"))
                 {
                   send_to_char("You don't see that much here!\n\r",ch);
                   return;
                 }
	       ch->silver += amount;
	       sprintf(arg2,"You get %d silver coins.",amount);
	       act(arg2,ch,obj,NULL,TO_CHAR);
	       sprintf(arg2,"$n gets %d silver coins.",amount);
	       act(arg2,ch,obj,NULL,TO_ROOM);
	       if (amount >= obj->value[0])
		 {
		   obj_from_room(obj);
		   extract_obj(obj);
		   obj = NULL;
		 }
	       else obj->value[0] -= amount;
	       break;
	      case OBJ_VNUM_GOLD_SOME:
		if ((amount = atoi(arg1)) > obj->value[1] || str_prefix(arg2,"gold"))
		  {
		    send_to_char("You don't see that much here!\n\r",ch);
		    return;
		  }
		ch->gold += amount;
		sprintf(arg2,"You get %d gold coins.",amount);
		act(arg2,ch,obj,NULL,TO_CHAR);
                sprintf(arg2,"$n gets %d gold coins.",amount);
		act(arg2,ch,obj,NULL,TO_ROOM);
		if (amount >= obj->value[1])
		  {
		    obj_from_room(obj);
		    extract_obj(obj);
		    obj = NULL;
		  }
		else obj->value[1] -= amount;
		break;
	      case OBJ_VNUM_COINS:
                if (((amount = atoi(arg1)) > obj->value[1] && !str_prefix(arg2,"gold"))
		    || ((amount = atoi(arg1)) > obj->value[0] && str_prefix(arg2,"gold")) )
                  {
                    send_to_char("You don't see that much here!\n\r",ch);
                    return;
                  }
		if (!str_prefix(arg2,"gold"))
		  {
		    ch->gold += amount;
		    sprintf(arg2,"You get %d gold coins.",amount);
		    act(arg2,ch,obj,NULL,TO_CHAR);
		    sprintf(arg2,"$n gets %d gold coins.",amount);
		    act(arg2,ch,obj,NULL,TO_ROOM);
		    obj->value[1] -= amount;
		  }
		else 
		  {
		    ch->silver += amount;
		    sprintf(arg2,"You get %d silver coins.",amount);
		    act(arg2,ch,obj,NULL,TO_CHAR);
		    sprintf(arg2,"$n gets %d silver coins.",amount);
		    act(arg2,ch,obj,NULL,TO_ROOM);
		    obj->value[0] -= amount;
		  }
		break;
	      }
	    if (obj == NULL)
  	      return;
		    if (obj->value[0] == 0 && obj->value[1] == 0)
		    {
  		      obj_from_room(obj);
		      extract_obj(obj);
		    }
		    else /*if ((obj->value[0] == 1 && obj->value[1] == 0) //1 silver left
			|| (obj->value[0] == 0 && obj->value[1] == 1) //1 gold left
			//Note the two below could cause the same item to be extracted
			// and then reput into the room.  This cost I will accept
			|| (obj->value[0] == 0 && obj->value[1] >= 1) //some gold left
			|| (obj->value[0] >= 1 && obj->value[1] == 0)) //some silver left*/
		      {
			      int silver,gold;
			      silver = obj->value[0];
			      gold = obj->value[1];
			      obj_from_room(obj);
			      extract_obj(obj);
			obj_to_room (create_money (gold, silver), ch->in_room);
			//obj_from_room(obj);
//			extract_obj(obj);
		      }
	  
		    
		    return;
	  }
	}
      }

      /* 'get ... container' */
      if (!str_cmp (arg2, "all") || !str_prefix ("all.", arg2))
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if ((container = get_obj_here (ch, arg2)) == NULL)
	{
	  act ("You see no $T here.", ch, NULL, arg2, TO_CHAR);
	  return;
	}
      switch (container->item_type)
	{
	default:
	  send_to_char ("That's not a container.\n\r", ch);
	  return;
        case ITEM_CORPSE_PC:
	case ITEM_PACK:
	case ITEM_CONTAINER:
	case ITEM_QUIVER:
	case ITEM_CORPSE_NPC:
	case ITEM_CLAN_DONATION:
	case ITEM_NEWCLANS_DBOX:
	case ITEM_PLAYER_DONATION:
	  break;
	}
      if (IS_SET (container->value[1], CONT_CLOSED)
	  && container->item_type != ITEM_CLAN_DONATION
	  && container->item_type != ITEM_NEWCLANS_DBOX
	  && container->item_type != ITEM_PLAYER_DONATION)
	{
	  act ("The $d is closed.", ch, NULL, container->name, TO_CHAR);
	  return;
	}
      if (str_cmp (arg1, "all") && str_prefix ("all.", arg1))
	{

	  /* 'get obj container' */
	  obj = get_obj_list (ch, arg1, container->contains);
	  if (obj == NULL)
	    {
	      act ("You see nothing like that in the $T.", ch, NULL, arg2,
		   TO_CHAR);
	      return;
	    }
	  get_obj (ch, obj, container);
	  trap_check(ch,"object",NULL,container);
	  //Iblis - 3/11/04 - Antiduping Code
	  if (!IS_NPC(ch))
            save_char_obj (ch);
	            
	}
      else
	{
	  if (container->pIndexData->vnum == OBJ_VNUM_PIT)
	    {
	      send_to_char ("Don't be so greedy.\n\r", ch);
	      return;
	    }

	  /* 'get all container' or 'get all.obj container' */
	  found = FALSE;
	  for (obj = container->contains; obj != NULL; obj = obj_next)
	    {
	      obj_next = obj->next_content;
	      if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
		  && can_see_obj (ch, obj))
		{
		  found = TRUE;
		  if (container->pIndexData->vnum == OBJ_VNUM_PIT
		      && !IS_IMMORTAL (ch))
		    {
		      send_to_char ("Don't be so greedy!\n\r", ch);
		      return;
		    }
		  get_obj (ch, obj, container);
	  	  trap_check(ch,"object",NULL,container);
		}
	    }
	  if (!found)
	    {
	      if (arg1[3] == '\0')
		act ("You see nothing in the $T.", ch, NULL, arg2, TO_CHAR);

	      else
		act ("You see nothing like that in the $T.", ch, NULL,
		     arg2, TO_CHAR);
	    }
	  //Iblis - 3/11/04 - Antiduping Code
	  else if (!IS_NPC(ch))
		  save_char_obj (ch);
	}
    }
   //12-29-03 Iblis - By Minax's request, if you're overweight you can't fly
   if (get_carry_weight (ch) > can_carry_w (ch) && IS_AFFECTED (ch, AFF_FLYING))
   {
     send_to_char("You are so heavy you fall to the ground.\n\r",ch);
     do_land(ch,"");
     return;
   }
  return;
}

void do_unpack (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  OBJ_DATA *container = NULL;
  CHAR_DATA *vch;
  bool found = FALSE;
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (!str_cmp (arg2, "from"))
    argument = one_argument (argument, arg2);
  if (arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char ("Unpack what from what?\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "all") || !str_prefix ("all.", arg2))
    {
      send_to_char ("You can't do that.\n\r", ch);
      return;
    }
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (!IS_SET (vch->act, ACT_MOUNT))
	continue;
      if (vch->mounter == ch)
	{
	  container = get_eq_char (vch, WEAR_BODY);
	  if (container != NULL && is_name (arg2, container->name))
	    found = TRUE;
	  break;
	}
    }
  if (!found)
    {
      act ("You see no $T here.", ch, NULL, arg2, TO_CHAR);
      return;
    }
  if (container->item_type != ITEM_PACK)
    {
      send_to_char ("That's not a pack.\n\r", ch);
      return;
    }
  if (IS_SET (container->value[1], CONT_CLOSED))
    {
      act ("The $d is closed.", ch, NULL, container->name, TO_CHAR);
      return;
    }
  if (str_cmp (arg1, "all") && str_prefix ("all.", arg1))
    {

      /* 'get obj container' */
      obj = get_obj_list (ch, arg1, container->contains);
      if (obj == NULL)
	{
	  act ("You see nothing like that in the $T.", ch, NULL, arg2, TO_CHAR);
	  return;
	}
      get_obj (ch, obj, container);
      //Iblis - 3/11/04 - Antiduping Code
      if (!IS_NPC(ch))
        save_char_obj (ch);
    }
  else
    {

      /* 'get all container' or 'get all.obj container' */
      found = FALSE;
      for (obj = container->contains; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
	      && can_see_obj (ch, obj))
	    {
	      found = TRUE;
	      if (container->pIndexData->vnum == OBJ_VNUM_PIT
		  && !IS_IMMORTAL (ch))
		{
		  send_to_char ("Don't be so greedy!\n\r", ch);
		  return;
		}
	      get_obj (ch, obj, container);
	    }
	}
      if (!found)
	{
	  if (arg1[3] == '\0')
	    act ("You see nothing in the $T.", ch, NULL, arg2, TO_CHAR);

	  else
	    act ("You see nothing like that in the $T.", ch, NULL, arg2,
		 TO_CHAR);
	}
      //Iblis - 3/11/04 - Antiduping Code
      else if (!IS_NPC(ch))
        save_char_obj (ch);
    }
  return;
}

void do_put (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char tcbuf[MAX_INPUT_LENGTH];
  CHAR_DATA *mount;
  OBJ_DATA *container;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (!str_cmp (arg2, "in") || !str_cmp (arg2, "on"))
    argument = one_argument (argument, arg2);
  if (arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char ("Put what in what?\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "all") || !str_prefix ("all.", arg2))
    {
      send_to_char ("You can't do that.\n\r", ch);
      return;
    }
  if (IS_SET (ch->act, PLR_WANTED))
    {
      send_to_char ("You can't do that while you are wanted.\n\r", ch);
      return;
    }
  if ((container = get_obj_here (ch, arg2)) == NULL)
    {
      if ((mount = get_char_room (ch, arg2)) == NULL)
	{
	  act ("You see no $T here.", ch, NULL, arg2, TO_CHAR);
	  return;
	}
      if ((obj = get_obj_carry (ch, arg1)) == NULL)
	{
	  send_to_char ("You do not have that item.\n\r", ch);
	  return;
	}
      // Adeon 7/18/03 -- cant put nosave stuff in containers
      if (IS_OBJ_STAT (obj, ITEM_NOSAVE))
	{
	  send_to_char ("Strong magic forces prevent you...\n\r", ch);
	  return;
	}
      if (obj->item_type != ITEM_SADDLE && obj->item_type != ITEM_PACK)
	{
	  send_to_char ("Mounts may only wear packs and saddles.\n\r", ch);
	  return;
	}
      if (!IS_SET (mount->act, ACT_MOUNT))
	{
	  sprintf (tcbuf, "You can't put the $p on %s", mount->short_descr);
	  act (tcbuf, ch, obj, NULL, TO_CHAR);
	  return;
	}
      if (obj->item_type == ITEM_PACK
	  && get_eq_char (mount, WEAR_BODY) != NULL)
	{
	  send_to_char ("That mount is already wearing a pack.\n\r", ch);
	  return;
	}
      if (obj->item_type == ITEM_SADDLE
	  && get_eq_char (mount, WEAR_ABOUT) != NULL)
	{
	  send_to_char ("That mount is already wearing a saddle.\n\r", ch);
	  return;
	}
      if (mount->mounter != ch && mount->mounter != NULL)
	{
	  sprintf (tcbuf, "%s refuses to let you put $p on it.",
		   fcapitalize (mount->short_descr));
	  act (tcbuf, ch, obj, NULL, TO_CHAR);
	  return;
	}
      obj_from_char (obj);
      obj_to_char (obj, mount);
      if (obj->item_type == ITEM_PACK)
	equip_char (mount, obj, WEAR_BODY);

      else
	equip_char (mount, obj, WEAR_ABOUT);
      sprintf (tcbuf, "You place $p on %s.", mount->short_descr);
      act (tcbuf, ch, obj, NULL, TO_CHAR);
      sprintf (tcbuf, "$n places $p on %s.", mount->short_descr);
      act (tcbuf, ch, obj, NULL, TO_ROOM);
      //Iblis - 3/11/04 - Antiduping Code
      if (!IS_NPC(ch))
        save_char_obj (ch);
      return;
    }
  if (container->item_type != ITEM_CONTAINER
      && container->item_type != ITEM_PACK
      && container->item_type != ITEM_PLAYER_DONATION
      && container->item_type != ITEM_NEWCLANS_DBOX
      && container->item_type != ITEM_CLAN_DONATION
      && container->item_type != ITEM_QUIVER)
    {
      send_to_char ("That's not a container.\n\r", ch);
      return;
    }
  if (container->item_type != ITEM_CLAN_DONATION
      && container->item_type != ITEM_NEWCLANS_DBOX
      && container->item_type != ITEM_PLAYER_DONATION
      && IS_SET (container->value[1], CONT_CLOSED))
    {
      act ("The $d is closed.", ch, NULL, container->name, TO_CHAR);
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
  if (str_cmp (arg1, "all") && str_prefix ("all.", arg1))
    {

      /* 'put obj container' */
      if ((obj = get_obj_carry (ch, arg1)) == NULL)
	{
	  send_to_char ("You do not have that item.\n\r", ch);
	  return;
	}
      if (obj == container)
	{
	  send_to_char ("You can't fold it into itself.\n\r", ch);
	  return;
	}
      // Adeon 7/18/03 -- cant put nosave stuff in containers
      if (IS_OBJ_STAT (obj, ITEM_NOSAVE))
	{
	  send_to_char ("Strong magic forces prevent you...\n\r", ch);
	  return;
	}
      if (!can_drop_obj (ch, obj))
	{
	  send_to_char ("You can't let go of it.\n\r", ch);
	  return;
	}
      if (WEIGHT_MULT (obj) != 100 || obj->item_type == ITEM_QUIVER || WEIGHT_MULTP (obj) != 100)
	{
	  send_to_char
	    ("You have a feeling that would be a bad idea.\n\r", ch);
	  return;
	}
      //Iblis 7/19/03 - Fixed so quivers can ONLY hold arrows, and when in 
      //   quivers, arrows don't count towards max item capacity
      if (container->item_type == ITEM_QUIVER)
	{
	  if (obj->item_type != ITEM_WEAPON || obj->value[0] != WEAPON_ARROW)
	    {
	      send_to_char ("Quivers can only hold arrows.\n\r", ch);
	      return;
	    }
	  if (container->value[0] <= quiver_holding (container))
	    {
	      send_to_char ("That quiver is full.\n\r", ch);
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
	 
      if (container->pIndexData->vnum == OBJ_VNUM_PIT
	  && !CAN_WEAR (container, ITEM_TAKE))
	{
	  if (obj->timer)
	    SET_BIT (obj->extra_flags[0], ITEM_HAD_TIMER);

	  else
	    obj->timer = number_range (100, 200);
	}
      obj_from_char (obj);
      obj_to_obj (obj, container);
      if ((container != NULL) &&
	  ((container->item_type == ITEM_CLAN_DONATION) ||
	   (container->item_type == ITEM_NEWCLANS_DBOX)))
	save_clan_box (container);
      if (container != NULL && container->item_type == ITEM_PLAYER_DONATION)
	save_player_box (container);
      if (container != NULL && container->pIndexData->vnum == OBJ_VNUM_CORPSE_PC)
        save_player_corpse (container);
      //Iblis - 3/11/04 - Antiduping Code
      if (!IS_NPC(ch))
        save_char_obj (ch);
      if (IS_SET (container->value[1], CONT_PUT_ON))
	{
	  if (container->pIndexData->vnum == OBJ_VNUM_PENTAGRAM)
	  {
	    act("$n carefully places $p in the pentagram.",ch,obj,NULL,TO_ROOM);
	    act("You carefully place $p in the pentagram.",ch,obj,NULL,TO_CHAR);
	  }
	  else
	  {
	    act ("$n puts $p on $P.", ch, obj, container, TO_ROOM);
	    act ("You put $p on $P.", ch, obj, container, TO_CHAR);
	    trip_triggers(ch, OBJ_TRIG_PUT, obj, NULL, OT_SPEC_NONE);
            trap_check(ch,"object",NULL,container);
	  }
	}
      else
	{
	  if (container->pIndexData->vnum == OBJ_VNUM_PENTAGRAM)
	   {
	     act("$n carefully places $p in the pentagram.",ch,obj,NULL,TO_ROOM);
	     act("You carefully place $p in the pentagram.",ch,obj,NULL,TO_CHAR);
	   }
	  else
	  {
	    act ("$n puts $p in $P.", ch, obj, container, TO_ROOM);
	    act ("You put $p in $P.", ch, obj, container, TO_CHAR);
	  }
	  trip_triggers(ch, OBJ_TRIG_PUT, obj, NULL, OT_SPEC_NONE);
          trap_check(ch,"object",NULL,container);

	}
    }
  else
    {

      if (container->pIndexData->vnum == OBJ_VNUM_PENTAGRAM
          && container->plr_owner && str_cmp(container->plr_owner,ch->name))
      {
        send_to_char("You cannot do that.\n\r", ch);
        return;
       }
       

      /* 'put all container' or 'put all.obj container' */
      for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
	      && can_see_obj (ch, obj)
	      && (!IS_OBJ_STAT (obj, ITEM_NOSAVE))
	      && WEIGHT_MULT (obj) == 100
	      && WEIGHT_MULTP (obj) == 100
	      && obj->item_type != ITEM_QUIVER && obj->wear_loc ==
	      WEAR_NONE && obj != container
	      && can_drop_obj (ch,
			       obj) && ((get_obj_weight (obj) +
					 get_true_weight (container) <=
					 (container->value[0] *
					  10) && get_obj_weight (obj)
					 <
					 container->value[3] *
					 10 && container->
					 item_type !=
					 ITEM_QUIVER) || (container->
							  item_type
							  ==
							  ITEM_QUIVER
							  && obj->
							  item_type
							  ==
							  ITEM_WEAPON
							  && obj->
							  value[0] ==
							  WEAPON_ARROW)))
	    {
	      if (container->value[0] <= quiver_holding (container))
		{

		  return;
		}
	      if (container->pIndexData->vnum == OBJ_VNUM_PIT
		  && !CAN_WEAR (obj, ITEM_TAKE))
		{
		  if (obj->timer)
		    SET_BIT (obj->extra_flags[0], ITEM_HAD_TIMER);

		  else
		    obj->timer = number_range (100, 200);
		}
	      obj_from_char (obj);
	      obj_to_obj (obj, container);
	      if (IS_SET (container->value[1], CONT_PUT_ON))
		{
		  if (container->pIndexData->vnum == OBJ_VNUM_PENTAGRAM)
		  {
 		    act("$n carefully places $p in the pentagram.",ch,obj,NULL,TO_ROOM);
		    act("You carefully place $p in the pentagram.",ch,obj,NULL,TO_CHAR);
		  }
		  else
		  {
		    act ("$n puts $p on $P.", ch, obj, container, TO_ROOM);
		    act ("You put $p on $P.", ch, obj, container, TO_CHAR);
		  }
	      	  trap_check(ch,"object",NULL,container);
		  trip_triggers(ch, OBJ_TRIG_PUT, obj, NULL, OT_SPEC_NONE);
		}
	      else
		{
 		  if (container->pIndexData->vnum == OBJ_VNUM_PENTAGRAM)
		  {
		    act("$n carefully places $p in the pentagram.",ch,obj,NULL,TO_ROOM);
		    act("You carefully place $p in the pentagram.",ch,obj,NULL,TO_CHAR);
		  }
		  else	
		  {
		    act ("$n puts $p in $P.", ch, obj, container, TO_ROOM);
		    act ("You put $p in $P.", ch, obj, container, TO_CHAR);
		  }
	      	  trap_check(ch,"object",NULL,container);
		  trip_triggers(ch, OBJ_TRIG_PUT, obj, NULL, OT_SPEC_NONE);
		}
	    }
	}
        //Iblis - 3/11/04 - Antiduping Code
        if (!IS_NPC(ch))
           save_char_obj (ch);
    }
  return;
}

void do_pack (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *container = NULL;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  CHAR_DATA *vch;
  bool found = FALSE;
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (!str_cmp (arg2, "in") || !str_cmp (arg2, "on"))
    argument = one_argument (argument, arg2);
  if (arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char ("Pack what in what?\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "all") || !str_prefix ("all.", arg2))
    {
      send_to_char ("You can't do that.\n\r", ch);
      return;
    }
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (!IS_SET (vch->act, ACT_MOUNT))
	continue;
      if (vch->mounter == ch)
	{
	  container = get_eq_char (vch, WEAR_BODY);
	  if (container != NULL && is_name (arg2, container->name))
	    found = TRUE;
	  break;
	}
    }
  if (!found)
    {
      act ("You see no $T here.", ch, NULL, arg2, TO_CHAR);
      return;
    }
  if (container->item_type != ITEM_PACK)
    {
      send_to_char ("That's not a pack.\n\r", ch);
      return;
    }
  if (IS_SET (container->value[1], CONT_CLOSED))
    {
      act ("The $d is closed.", ch, NULL, container->name, TO_CHAR);
      return;
    }
  if (str_cmp (arg1, "all") && str_prefix ("all.", arg1))
    {

      /* 'put obj container' */
      if ((obj = get_obj_carry (ch, arg1)) == NULL)
	{
	  send_to_char ("You do not have that item.\n\r", ch);
	  return;
	}
      if (obj == container)
	{
	  send_to_char ("You can't fold it into itself.\n\r", ch);
	  return;
	}
      if (!can_drop_obj (ch, obj))
	{
	  send_to_char ("You can't let go of it.\n\r", ch);
	  return;
	}
      if (WEIGHT_MULT (obj) != 100 || WEIGHT_MULTP (obj) != 100)
	{
	  send_to_char
	    ("You have a feeling that would be a bad idea.\n\r", ch);
	  return;
	}
      if (get_obj_weight (obj) + get_true_weight (container)
	  > (container->value[0] * 10)
	  || get_obj_weight (obj) > (container->value[3] * 10))
	{
	  send_to_char ("It won't fit.\n\r", ch);
	  return;
	}
      if (container->pIndexData->vnum == OBJ_VNUM_PIT
	  && !CAN_WEAR (container, ITEM_TAKE))
	{
	  if (obj->timer)
	    SET_BIT (obj->extra_flags[0], ITEM_HAD_TIMER);

	  else
	    obj->timer = number_range (100, 200);
	}
      obj_from_char (obj);
      obj_to_obj (obj, container);
      //Iblis - 3/11/04 - Antiduping Code
      if (!IS_NPC(ch))
        save_char_obj (ch);

      act ("$n packs $p in $P.", ch, obj, container, TO_ROOM);
      act ("You pack $p in $P.", ch, obj, container, TO_CHAR);

    }
  else
    {

      /* 'put all container' or 'put all.obj container' */
      for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
	      && can_see_obj (ch, obj)
	      && WEIGHT_MULT (obj) == 100
	      && (!IS_OBJ_STAT (obj, ITEM_NOSAVE)) 
	      && obj->wear_loc == WEAR_NONE
	      && obj != container && can_drop_obj (ch, obj)
	      && get_obj_weight (obj) + get_true_weight (container)
	      <= (container->value[0] * 10)
	      && get_obj_weight (obj) < (container->value[3] * 10))
	    {
	      if (container->pIndexData->vnum == OBJ_VNUM_PIT
		  && !CAN_WEAR (obj, ITEM_TAKE))
		{
		  if (obj->timer)
		    SET_BIT (obj->extra_flags[0], ITEM_HAD_TIMER);

		  else
		    obj->timer = number_range (100, 200);
		}
	      obj_from_char (obj);
	      obj_to_obj (obj, container);

	      act ("$n packs $p in $P.", ch, obj, container, TO_ROOM);
	      act ("You pack $p in $P.", ch, obj, container, TO_CHAR);
	      //Iblis - 3/11/04 - Antiduping Code
	      if (!IS_NPC(ch))
	        save_char_obj (ch);

	    }
	}
    }
  return;
}

void do_drop (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  bool found;
  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Drop what?\n\r", ch);
      return;
    }
  if (IS_SET (ch->act, PLR_WANTED))
    {
      send_to_char ("You can't do that while you are wanted.\n\r", ch);
      return;
    }

  if (ch->dueler)
  {
    send_to_char ("You can't do that while super dueling.\n\r",ch);
    return;
  }
  if (is_number (arg))
    {

      /* 'drop NNNN coins' */
      int amount, gold = 0, silver = 0;
      amount = atoi (arg);
      argument = one_argument (argument, arg);
      if (amount <= 0 || (str_cmp (arg, "coins") && str_cmp (arg, "coin")
			  && str_cmp (arg, "gold")
			  && str_cmp (arg, "silver")))
	{
	  send_to_char ("Sorry, you can't do that.\n\r", ch);
	  return;
	}
      if (!str_cmp (arg, "coins") || !str_cmp (arg, "coin")
	  || !str_cmp (arg, "silver"))
	{
	  if (ch->silver < amount)
	    {
	      send_to_char ("You don't have that much silver.\n\r", ch);
	      return;
	    }
	  ch->silver -= amount;
	  silver = amount;
	}

      else
	{
	  if (ch->gold < amount)
	    {
	      send_to_char ("You don't have that much gold.\n\r", ch);
	      return;
	    }
	  ch->gold -= amount;
	  gold = amount;
	}
      for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  switch (obj->pIndexData->vnum)
	    {
	    case OBJ_VNUM_SILVER_ONE:
	      silver += 1;
	      extract_obj (obj);
	      break;
	    case OBJ_VNUM_GOLD_ONE:
	      gold += 1;
	      extract_obj (obj);
	      break;
	    case OBJ_VNUM_SILVER_SOME:
	      silver += obj->value[0];
	      extract_obj (obj);
	      break;
	    case OBJ_VNUM_GOLD_SOME:
	      gold += obj->value[1];
	      extract_obj (obj);
	      break;
	    case OBJ_VNUM_COINS:
	      silver += obj->value[0];
	      gold += obj->value[1];
	      extract_obj (obj);
	      break;
	    }
	}
      obj_to_room (create_money (gold, silver), ch->in_room);
      act ("$n drops some coins.", ch, NULL, NULL, TO_ROOM);
      send_to_char ("OK.\n\r", ch);
      //Iblis - 3/11/04 - Antiduping Code
      if (!IS_NPC(ch))
        save_char_obj (ch);
      return;
    }
  if (str_cmp (arg, "all") && str_prefix ("all.", arg))
    {

      /* 'drop obj' */
      if ((obj = get_obj_carry (ch, arg)) == NULL)
	{
	  send_to_char ("You do not have that item.\n\r", ch);
	  return;
	}
      if (!can_drop_obj (ch, obj))
	{
	  send_to_char ("You can't let go of it.\n\r", ch);
	  return;
	}
      obj_from_char (obj);
      obj_to_room (obj, ch->in_room);
      act ("$n drops $p.", ch, obj, NULL, TO_ROOM);
      act ("You drop $p.", ch, obj, NULL, TO_CHAR);
	  trip_triggers(ch, OBJ_TRIG_DROP, obj, NULL, OT_SPEC_NONE);
      if (IS_OBJ_STAT (obj, ITEM_MELT_DROP))
	{
	  act ("$p dissolves into smoke.", ch, obj, NULL, TO_ROOM);
	  act ("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
	  extract_obj (obj);
	}
      //Iblis - 3/11/04 - Antiduping Code
      if (!IS_NPC(ch))
        save_char_obj (ch);
    }
  else
    {

      /* 'drop all' or 'drop all.obj' */
      found = FALSE;
      for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if ((arg[3] == '\0' || is_name (&arg[4], obj->name))
	      && can_see_obj (ch, obj)
	      && obj->wear_loc == WEAR_NONE && can_drop_obj (ch, obj))
	    {
	      found = TRUE;
	      obj_from_char (obj);
	      obj_to_room (obj, ch->in_room);
	      act ("$n drops $p.", ch, obj, NULL, TO_ROOM);
	      act ("You drop $p.", ch, obj, NULL, TO_CHAR);
		  trip_triggers(ch, OBJ_TRIG_DROP, obj, NULL, OT_SPEC_NONE);
	      if (IS_OBJ_STAT (obj, ITEM_MELT_DROP))
		{
		  act ("$p dissolves into smoke.", ch, obj, NULL, TO_ROOM);
		  act ("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
		  extract_obj (obj);
		}
	    }
	}
      if (!found)
	{
	  if (arg[3] == '\0')
	    act ("You are not carrying anything.", ch, NULL, arg, TO_CHAR);

	  else
	    act ("You are not carrying any $T.", ch, NULL, &arg[4], TO_CHAR);
	}
      //Iblis - 3/11/04 - Antiduping Code
      else if (!IS_NPC(ch))
        save_char_obj (ch);
    }
  return;
}

void do_give (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char ("Give what to whom?\n\r", ch);
      return;
    }
  if (IS_SET (ch->act, PLR_WANTED))
    {
      send_to_char ("You can't do that while you are wanted.\n\r", ch);
      return;
    }
  if (is_number (arg1))
    {

      /* 'give NNNN coins victim' */
      int amount;
      bool silver;
      amount = atoi (arg1);
      if (amount <= 0 || (str_cmp (arg2, "coins") && str_cmp (arg2, "coin")
			  && str_cmp (arg2, "gold")
			  && str_cmp (arg2, "silver")))
	{
	  send_to_char ("Sorry, you can't do that.\n\r", ch);
	  return;
	}
      silver = str_cmp (arg2, "gold");
      argument = one_argument (argument, arg2);
      if (arg2[0] == '\0')
	{
	  send_to_char ("Give what to whom?\n\r", ch);
	  return;
	}
      if ((victim = get_char_room (ch, arg2)) == NULL)
	{
	  send_to_char ("They aren't here.\n\r", ch);
	  return;
	}
      if ((!silver && ch->gold < amount) || (silver && ch->silver < amount))
	{
	  send_to_char ("You haven't got that much.\n\r", ch);
	  return;
	}
      if (silver)
	{
	  ch->silver -= amount;
	  victim->silver += amount;
	}
      else
	{
	  ch->gold -= amount;
	  victim->gold += amount;
	}
      sprintf (buf, "$n gives you %d %s.", amount,
	       silver ? "silver" : "gold");
      act (buf, ch, NULL, victim, TO_VICT);
      act ("$n gives $N some coins.", ch, NULL, victim, TO_NOTVICT);
      sprintf (buf, "You give $N %d %s.", amount, silver ? "silver" : "gold");
      act (buf, ch, NULL, victim, TO_CHAR);
      if (HAS_SCRIPT (victim))
	{
	  TRIGGER_DATA *pTrig;
	  for (pTrig = victim->triggers; pTrig != NULL; pTrig = pTrig->next)
	    {
	      if (pTrig->trigger_type == TRIG_GETS && pTrig->current ==
		  NULL && !IS_SET (victim->act, ACT_HALT))
		{
		  act_trigger (victim, pTrig->name, "gicash", NAME (ch),
			       NAME (victim));
		  pTrig->current = pTrig->script;
		  pTrig->bits = SCRIPT_ADVANCE;
		  script_update (victim, TRIG_EACH_PULSE);
		}
	    }
	}
      if (IS_NPC (victim) && IS_SET (victim->act, ACT_IS_CHANGER)
	  && ch != victim)
	{
	  int change;
	  change = (silver ? 95 * amount / 100 / 100 : 95 * amount);
	  if (!silver && change > victim->silver)
	    victim->silver += change;
	  if (silver && change > victim->gold)
	    victim->gold += change;
	  if (change < 1 && can_see (victim, ch))
	    {
	      act
		("$n tells you 'I'm sorry, you did not give me enough to change.'",
		 victim, NULL, ch, TO_VICT);
	      ch->reply = victim;
	      sprintf (buf, "%d %s %s", amount,
		       silver ? "silver" : "gold", ch->name);
	      do_give (victim, buf);
	    }
	  else if (can_see (victim, ch))
	    {
	      sprintf (buf, "%d %s %s", change,
		       silver ? "gold" : "silver", ch->name);
	      do_give (victim, buf);
	      if (silver)
		{
		  sprintf (buf, "%d silver %s",
			   (95 * amount / 100 - change * 100), ch->name);
		  do_give (victim, buf);
		}
	      act ("$n tells you 'Thank you, come again.'", victim,
		   NULL, ch, TO_VICT);
	      ch->reply = victim;
	    }
	}
      //Iblis - 3/11/04 - Antiduping Code
      if (!IS_NPC(ch))
        save_char_obj (ch);
      if (!IS_NPC(victim))
	save_char_obj (victim);
      return;
    }
  if ((obj = get_obj_carry (ch, arg1)) == NULL)
    {
      send_to_char ("You do not have that item.\n\r", ch);
      return;
    }
  if (obj->wear_loc != WEAR_NONE)
    {
      send_to_char ("You must remove it first.\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg2)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim) && victim->pIndexData->pShop != NULL)
    {
      act ("$N tells you 'Sorry, you'll have to sell that.'", ch, NULL,
	   victim, TO_CHAR);
      ch->reply = victim;
      return;
    }
  if (!can_drop_obj (ch, obj))
    {
      send_to_char ("You can't let go of it.\n\r", ch);
      return;
    }
  if (obj->item_type == ITEM_QUIVER && has_quiver (victim))
    {
      act ("$N already has a quiver.\n\r", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (IS_NPC(victim) && IS_SET(obj->extra_flags[0],ITEM_NOMOB))
  {
    send_to_char("They are not allowed to have that item.\n\r",ch);
    return;
  }
  if (victim->carry_number + get_obj_number (obj) > can_carry_n (victim))
    {
      act ("$N has $S hands full.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (obj->item_type == ITEM_PORTAL)
    adjust_portal_weight (obj);
  if (get_carry_weight (victim) + get_obj_weight (obj) > can_carry_w (victim))
    {
      act ("$N can't carry that much weight.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (!IS_NPC (victim))
    {
      if (victim->mount != NULL)
	if (get_char_weight (victim) + get_obj_weight (obj) >
	    victim->mount->pIndexData->max_weight)
	  {
	    act ("$N's mount can't carry that much weight.", ch, NULL,
		 victim, TO_CHAR);
	    return;
	  }
    }
  if (!can_see_obj (victim, obj))
    {
      act ("$N can't see it.", ch, NULL, victim, TO_CHAR);
      return;
    }
  obj_from_char (obj);
  obj_to_char (obj, victim);
  act ("$n gives $p to $N.", ch, obj, victim, TO_NOTVICT);
  act ("$n gives you $p.", ch, obj, victim, TO_VICT);
  act ("You give $p to $N.", ch, obj, victim, TO_CHAR);
  trip_triggers(victim, OBJ_TRIG_OBJ_GIVEN_CHAR, obj, ch, OT_SPEC_NONE);

  if (HAS_SCRIPT (victim))
    {
      TRIGGER_DATA *pTrig;
      for (pTrig = victim->triggers; pTrig != NULL; pTrig = pTrig->next)
	{
	  if (pTrig->trigger_type == TRIG_GETS && pTrig->current ==
	      NULL && !IS_SET (victim->act, ACT_HALT))
	    {
	      act_trigger (victim, pTrig->name, obj->name, NAME (ch),
			   NAME (victim));
	      pTrig->current = pTrig->script;
	      pTrig->bits = SCRIPT_ADVANCE;
	      script_update (victim, TRIG_EACH_PULSE);
	    }
	}
    }
  //Iblis - 3/11/04 - Antiduping Code
  if (!IS_NPC(ch))
    save_char_obj (ch);
  if (!IS_NPC(victim))
    save_char_obj (victim);
  return;
}


    /* for poisoning weapons and food/drink */
void do_envenom (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj;
  AFFECT_DATA af;
  int percent, skill;

  /* find out what */
  if (argument == '\0')
    {
      send_to_char ("Envenom what item?\n\r", ch);
      return;
    }
  obj = get_obj_list (ch, argument, ch->carrying);
  if (obj == NULL)
    {
      send_to_char ("You don't have that item.\n\r", ch);
      return;
    }
  if ((skill = get_skill (ch, gsn_envenom)) < 1)
    {
      send_to_char ("Are you crazy? You'd poison yourself!\n\r", ch);
      return;
    }
  if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
    {
/*      if (IS_OBJ_STAT (obj, ITEM_BLESS) )
	{
	  act ("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
	  return;
	}
  */    if (number_percent () < skill)
	{			/* success! */
	  act ("$n treats $p with deadly poison.", ch, obj, NULL, TO_ROOM);
	  act ("You treat $p with deadly poison.", ch, obj, NULL, TO_CHAR);
	  if (!obj->value[3])
	    {
	      obj->value[3] = 1;
	      check_improve (ch, gsn_envenom, TRUE, 4);
	    }
	  WAIT_STATE (ch, skill_table[gsn_envenom].beats);
	  return;
	}
      act ("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
      if (!obj->value[3])
	check_improve (ch, gsn_envenom, FALSE, 4);
      WAIT_STATE (ch, skill_table[gsn_envenom].beats);
      return;
    }
  if (obj->item_type == ITEM_WEAPON)
    {
/*      if (IS_WEAPON_STAT (obj, WEAPON_FLAMING)
	  || IS_WEAPON_STAT (obj, WEAPON_FROST)
	  || IS_WEAPON_STAT (obj, WEAPON_VAMPIRIC)
	  || IS_WEAPON_STAT (obj, WEAPON_SHOCKING)
	  || IS_OBJ_STAT (obj, ITEM_BLESS)
	  || IS_OBJ_STAT (obj, ITEM_BURN_PROOF))
	{
	  act ("You can't seem to envenom $p.", ch, obj, NULL, TO_CHAR);
	  return;
	}
  */  
	if (IS_CLASS (ch, PC_CLASS_REAVER))
	{
	  send_to_char("The soul in your blade resists the poison.\n\r",ch);
	  return;
	}
	if (obj->value[3] < 0 || attack_table[obj->value[3]].damage == DAM_BASH)
	{
	  send_to_char ("You can only envenom edged weapons.\n\r", ch);
	  return;
	}
      if (IS_WEAPON_STAT (obj, WEAPON_POISON))
	{
	  act ("$p is already envenomed.", ch, obj, NULL, TO_CHAR);
	  return;
	}
      percent = number_percent ();
      if (percent < skill)
	{
	  af.where = TO_WEAPON;
	  af.type = gsn_poison;
	  af.level = ch->level * percent / 100;
	  af.duration = ch->level / 2 * percent / 100;
	  af.location = 0;
	  af.modifier = 0;
	  af.bitvector = WEAPON_POISON;
	  af.permaff = FALSE;
	  af.composition = FALSE;
	  af.comp_name = str_dup ("");
	  affect_to_obj (obj, &af);
	  act ("$n coats $p with deadly venom.", ch, obj, NULL, TO_ROOM);
	  act ("You coat $p with venom.", ch, obj, NULL, TO_CHAR);
	  check_improve (ch, gsn_envenom, TRUE, 3);
	  WAIT_STATE (ch, skill_table[gsn_envenom].beats);
	  return;
	}
      else
	{
	  act ("You fail to envenom $p.", ch, obj, NULL, TO_CHAR);
	  check_improve (ch, gsn_envenom, FALSE, 3);
	  WAIT_STATE (ch, skill_table[gsn_envenom].beats);
	  return;
	}
    }
  act ("You can't poison $p.", ch, obj, NULL, TO_CHAR);
  return;
}

void do_fill (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *fountain;
  bool found;
one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Fill what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_carry (ch, arg)) == NULL)
    {
      send_to_char ("You do not have that item.\n\r", ch);
      return;
    }
  found = FALSE;
  for (fountain = ch->in_room->contents; fountain != NULL;
       fountain = fountain->next_content)
    {
      if (fountain->item_type == ITEM_FOUNTAIN)
	{
	  found = TRUE;
	  break;
	}
    }
  if (!found)
    {
      send_to_char ("There is no fountain here!\n\r", ch);
      return;
    }
  if (obj->item_type != ITEM_DRINK_CON)
    {
      send_to_char ("You can't fill that.\n\r", ch);
      return;
    }
  if (obj->value[1] != 0 && obj->value[2] != fountain->value[2])
    {
      send_to_char ("There is already another liquid in it.\n\r", ch);
      return;
    }
  if (obj->value[1] >= obj->value[0])
    {
      send_to_char ("Your container is full.\n\r", ch);
      return;
    }
  sprintf (buf, "You fill $p with %s from $P.",
	   liq_table[fountain->value[2]].liq_name);
  act (buf, ch, obj, fountain, TO_CHAR);
  sprintf (buf, "$n fills $p with %s from $P.",
	   liq_table[fountain->value[2]].liq_name);
  act (buf, ch, obj, fountain, TO_ROOM);
  obj->value[2] = fountain->value[2];
  obj->value[1] = obj->value[0];
  return;
}

void do_pour (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
  OBJ_DATA *out, *in;
  CHAR_DATA *vch = NULL;
  int amount;
  argument = one_argument (argument, arg);
  if (arg[0] == '\0' || argument[0] == '\0')
    {
      send_to_char ("Pour what into what?\n\r", ch);
      return;
    }
  if ((out = get_obj_carry (ch, arg)) == NULL)
    {
      send_to_char ("You don't have that item.\n\r", ch);
      return;
    }
  if (out->item_type != ITEM_DRINK_CON)
    {
      send_to_char ("That's not a drink container.\n\r", ch);
      return;
    }
  if (!str_cmp (argument, "out"))
    {
      if (out->value[1] == 0)
	{
	  send_to_char ("It's already empty.\n\r", ch);
	  return;
	}
      out->value[1] = 0;
      out->value[3] = 0;
      sprintf (buf, "You invert $p, spilling %s all over the ground.",
	       liq_table[out->value[2]].liq_name);
      act (buf, ch, out, NULL, TO_CHAR);
      sprintf (buf, "$n inverts $p, spilling %s all over the ground.",
	       liq_table[out->value[2]].liq_name);
      act (buf, ch, out, NULL, TO_ROOM);
      return;
    }
  if ((in = get_obj_here (ch, argument)) == NULL)
    {
      vch = get_char_room (ch, argument);
      if (vch == NULL)
	{
	  send_to_char ("Pour into what?\n\r", ch);
	  return;
	}
      in = get_eq_char (vch, WEAR_HOLD);
      if (in == NULL)
	{
	  send_to_char
	    ("They aren't holding anything in your left hand.", ch);
	  return;
	}
    }
  if (in->item_type != ITEM_DRINK_CON)
    {
      send_to_char ("You can only pour into other drink containers.\n\r", ch);
      return;
    }
  if (in == out)
    {
      send_to_char ("You cannot change the laws of physics!\n\r", ch);
      return;
    }
  if (in->value[1] != 0 && in->value[2] != out->value[2])
    {
      send_to_char ("They don't hold the same liquid.\n\r", ch);
      return;
    }
  if (out->value[1] == 0)
    {
      act ("There's nothing in $p to pour.", ch, out, NULL, TO_CHAR);
      return;
    }
  if (in->value[1] >= in->value[0])
    {
      act ("$p is already filled to the top.", ch, in, NULL, TO_CHAR);
      return;
    }
  amount = UMIN (out->value[1], in->value[0] - in->value[1]);
  in->value[1] += amount;
  out->value[1] -= amount;
  in->value[2] = out->value[2];
  if (vch == NULL)
    {
      sprintf (buf, "You pour %s from $p into $P.",
	       liq_table[out->value[2]].liq_name);
      act (buf, ch, out, in, TO_CHAR);
      sprintf (buf, "$n pours %s from $p into $P.",
	       liq_table[out->value[2]].liq_name);
      act (buf, ch, out, in, TO_ROOM);
    }
  else
    {
      sprintf (buf, "You pour some %s for $N.",
	       liq_table[out->value[2]].liq_name);
      act (buf, ch, NULL, vch, TO_CHAR);
      sprintf (buf, "$n pours you some %s.",
	       liq_table[out->value[2]].liq_name);
      act (buf, ch, NULL, vch, TO_VICT);
      sprintf (buf, "$n pours some %s for $N.",
	       liq_table[out->value[2]].liq_name);
      act (buf, ch, NULL, vch, TO_NOTVICT);
    }
}
void do_drink (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int amount;
  int liquid;
  one_argument (argument, arg);
  if (ch->race == PC_RACE_KALIAN)
  {
    send_to_char ("Drinks of the mortals interest you not.\n\r",ch);
    return;
  }
  if (arg[0] == '\0')
    {
      for (obj = ch->in_room->contents; obj; obj = obj->next_content)
	{
	  if (obj->item_type == ITEM_FOUNTAIN && can_see_obj(ch,obj))
	    break;
	}
      if (obj == NULL)
	{
	  send_to_char ("Drink what?\n\r", ch);
	  return;
	}
    }
  else
    {
      if ((obj = get_obj_here (ch, arg)) == NULL)
	{
	  send_to_char ("You can't find it.\n\r", ch);
	  return;
	}
    }
  if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 23)
    {
      send_to_char ("You fail to reach your mouth.  *Hic*\n\r", ch);
      return;
    }
  switch (obj->item_type)
    {
    default:
      send_to_char ("You can't drink from that.\n\r", ch);
      return;
    case ITEM_FOUNTAIN:
      if ((liquid = obj->value[2]) < 0)
	{
	  bug ("Do_drink: bad liquid number %d.", liquid);
	  liquid = obj->value[2] = 0;
	}
      amount = liq_table[liquid].liq_affect[4] * 3;
      break;
    case ITEM_DRINK_CON:
      if (obj->value[1] <= 0)
	{
	  send_to_char ("It is already empty.\n\r", ch);
	  return;
	}
      if ((liquid = obj->value[2]) < 0)
	{
	  bug ("Do_drink: bad liquid number %d.", liquid);
	  liquid = obj->value[2] = 0;
	}
      amount = liq_table[liquid].liq_affect[4];
      amount = UMIN (amount, obj->value[1]);
      break;
    }
  if (!IS_NPC (ch) && !IS_IMMORTAL (ch)
      && ch->pcdata->condition[COND_FULL] > 45)
    {
      send_to_char ("You're too full to drink more.\n\r", ch);
      return;
    }
  act ("$n drinks $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_ROOM);
  act ("You drink $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_CHAR);
  gain_condition (ch, COND_DRUNK,
		  amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36);
  gain_condition (ch, COND_FULL,
		  amount * liq_table[liquid].liq_affect[COND_FULL] / 4);
  gain_condition (ch, COND_THIRST,
		  amount * liq_table[liquid].liq_affect[COND_THIRST] / 10);
  gain_condition (ch, COND_HUNGER,
		  amount * liq_table[liquid].liq_affect[COND_HUNGER] / 2);
  if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    send_to_char ("You feel drunk.\n\r", ch);
  if (!IS_NPC (ch) && ch->pcdata->condition[COND_FULL] > 40)
    send_to_char ("You are full.\n\r", ch);
  if (!IS_NPC (ch) && ch->pcdata->condition[COND_THIRST] > 40)
    send_to_char ("Your thirst is quenched.\n\r", ch);
  if (obj->value[3] != 0)
    {

      /* The drink was poisoned ! */
      AFFECT_DATA af;
      act ("$n chokes and gags.", ch, NULL, NULL, TO_ROOM);
      send_to_char ("You choke and gag.\n\r", ch);
      af.where = TO_AFFECTS;
      af.type = gsn_poison;
      af.level = number_fuzzy (amount);
      af.duration = 3 * amount;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = AFF_POISON;
      af.permaff = FALSE;
      af.composition = FALSE;
      af.comp_name = str_dup ("");
      affect_join (ch, &af);
    }
  if (obj->value[0] > 0)
    obj->value[1] -= amount;
  return;
}

void do_eat (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Eat what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_carry (ch, arg)) == NULL)
    {
      send_to_char ("You do not have that item.\n\r", ch);
      return;
    }
  if (!IS_IMMORTAL (ch))
    {
      if (ch->race == PC_RACE_KALIAN && obj->item_type == ITEM_FOOD)
        {
          send_to_char ("Food of the mortals interests you not.\n\r",ch);
          return;
        }
      if (obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL 
		      && !(IS_SET(ch->act2,ACT_FAMILIAR) && obj->pIndexData->vnum == OBJ_VNUM_EYE)
		      && !(ch->Class == PC_CLASS_CHAOS_JESTER && obj->item_type == ITEM_FEATHER)
		      )
	{
	  send_to_char ("That's not edible.\n\r", ch);
	  return;
	}
      if (!IS_NPC (ch) && obj->item_type == ITEM_PILL && ch->pcdata->condition[COND_PILLS] >= 3)
        {
          send_to_char ("You are too full of pills to eat any more.\n\r",ch);
	  return;
	}
      if (!IS_NPC (ch) && obj->item_type == ITEM_FOOD && ch->pcdata->condition[COND_FULL] > 40)
	{
	  send_to_char ("You are too full to eat more.\n\r", ch);
	  return;
	}
    }
  act ("$n eats $p.", ch, obj, NULL, TO_ROOM);
  act ("You eat $p.", ch, obj, NULL, TO_CHAR);
  switch (obj->item_type)
    {
    case ITEM_FEATHER:
      if (ch->Class != PC_CLASS_CHAOS_JESTER) //IE Imm
        break;
      else
      {
        int sn = number_range(1,MAX_SKILL);
        while (skill_table[sn].name == NULL || (skill_table[sn].type != SKILL_CHANT 
			&& skill_table[sn].type != SKILL_CAST
			&& skill_table[sn].type != SKILL_PRAY
			&& skill_table[sn].type != SKILL_SING)
			|| skill_table[sn].target == TAR_STRING
			|| skill_table[sn].target == TAR_OBJ_INV
			|| skill_table[sn].target == TAR_OBJ_PENTAGRAM
			|| skill_table[sn].target == TAR_IGNORE
			|| skill_table[sn].target == TAR_CHAR_HEXAGRAM
			|| skill_table[sn].target == TAR_ROOM_SEPTAGRAM
			|| skill_table[sn].spell_fun == NULL)
		sn = number_range(1,MAX_SKILL);
	target_name="";
	 (*skill_table[sn].spell_fun) (sn, ch->level, ch, (void*)ch, TARGET_CHAR);
      }
    case ITEM_FOOD:
      if (!IS_NPC (ch))
	{
	  int condition;
	  if (ch->race == PC_RACE_LICH)
	    break;
	  condition = ch->pcdata->condition[COND_HUNGER];
	  gain_condition (ch, COND_FULL, obj->value[0]);

	  //IBLIS 5/31/03 - Allows one to always take an extra sip after full of food
	  if (ch->pcdata->condition[COND_FULL] > 45)
	    ch->pcdata->condition[COND_FULL] = 45;
	  gain_condition (ch, COND_HUNGER, obj->value[1]);
	  if (condition == 0 && ch->pcdata->condition[COND_HUNGER] > 0)
	    send_to_char ("You are no longer hungry.\n\r", ch);

	  else if (ch->pcdata->condition[COND_FULL] > 40)
	    send_to_char ("You are full.\n\r", ch);
	}
      else
      {
	 if (IS_SET(ch->act2,ACT_FAMILIAR) && (obj->pIndexData->vnum == OBJ_VNUM_TORN_HEART 
			 || obj->pIndexData->vnum == OBJ_VNUM_BRAINS))
	 {
		 if (ch->hit+50 >= ch->max_hit)
			 ch->hit = ch->max_hit;
		 else ch->hit += 50;
	 }
      }
      if (obj->value[3] != 0)
	{

	  /* The food was poisoned! */
	  AFFECT_DATA af;
	  act ("$n chokes and gags.", ch, 0, 0, TO_ROOM);
	  send_to_char ("You choke and gag.\n\r", ch);
	  af.where = TO_AFFECTS;
	  af.type = gsn_poison;
	  af.level = number_fuzzy (obj->value[0]);
	  af.duration = 2 * obj->value[0];
	  af.location = APPLY_NONE;
	  af.modifier = 0;
	  af.bitvector = AFF_POISON;
	  af.permaff = FALSE;
	  af.composition = FALSE;
	  af.comp_name = str_dup ("");
	  affect_join (ch, &af);
	}
      break;
    case ITEM_PILL:
      if (!IS_NPC (ch))
	{
	    gain_condition(ch,COND_PILLS,1);
	}
      //Iblis 9/13/04 - No Magic flag
      if (IS_SET (ch->in_room->room_flags2, ROOM_NOMAGIC) && !IS_IMMORTAL (ch))
      {
         send_to_char("There are no magical forces to draw power from.\n\r",ch);
         return;
      }
      obj_cast_spell (obj->value[1], obj->value[0], ch, ch, NULL);
      obj_cast_spell (obj->value[2], obj->value[0], ch, ch, NULL);
      obj_cast_spell (obj->value[3], obj->value[0], ch, ch, NULL);
      obj_cast_spell (obj->value[4], obj->value[0], ch, ch, NULL);
      break;
    case ITEM_GEM:
      if (!IS_IMMORTAL(ch))
      {
	spell_detect_hidden(skill_lookup("detect hidden"),10,ch,(void*) ch,TARGET_CHAR);
	spell_detect_invis(skill_lookup("detect invis"),10,ch,(void*) ch,TARGET_CHAR);
      }
      break;
    }
  trip_triggers(ch, OBJ_TRIG_EATEN, obj, NULL, OT_SPEC_NONE);
  extract_obj (obj);
  return;
}

/*
 * Remove an object.
 */
bool can_remove_obj (CHAR_DATA * ch, long iWear, bool fReplace)
{
  OBJ_DATA *obj;
  if ((obj = get_eq_char_new (ch, iWear)) == NULL)
    return TRUE;
  if (!fReplace)
    return FALSE;
  if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
    {
      act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
      return FALSE;
    }
  if (obj->item_type == ITEM_POLE && ch->pcdata && ch->pcdata->fishing)
    {
      send_to_char
        ("You cannot remove your pole WHILE you're fishing!\n\r", ch);
      return FALSE;
    }
  if (obj->item_type == ITEM_WEAPON && IS_WEAPON_STAT(obj,WEAPON_HOLY_ACTIVATED))
    REMOVE_BIT(obj->value[4],WEAPON_HOLY_ACTIVATED);
  if (obj->item_type == ITEM_POLE)
    obj->value[1] = 0;
  // unequip_char (ch, obj);
  //act ("$n stops using $p.", ch, obj, NULL, TO_ROOM);
  //act ("You stop using $p.", ch, obj, NULL, TO_CHAR);
  return TRUE;
}

bool can_remove_obj_silent (CHAR_DATA * ch, long iWear, bool fReplace)
{
  OBJ_DATA *obj;
  if ((obj = get_eq_char_new (ch, iWear)) == NULL)
    return TRUE;
  if (!fReplace)
    return FALSE;
  if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
    {
      //      act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
      return FALSE;
    }
  if (obj->item_type == ITEM_POLE && ch->pcdata && ch->pcdata->fishing)
    {
      //      send_to_char
      //  ("You cannot remove your pole WHILE you're fishing!\n\r", ch);
      return FALSE;
    }
  if (obj->item_type == ITEM_WEAPON && IS_WEAPON_STAT(obj,WEAPON_HOLY_ACTIVATED))
    REMOVE_BIT(obj->value[4],WEAPON_HOLY_ACTIVATED);
  if (obj->item_type == ITEM_POLE)
    obj->value[1] = 0;
  // unequip_char (ch, obj);
  //act ("$n stops using $p.", ch, obj, NULL, TO_ROOM);
  //act ("You stop using $p.", ch, obj, NULL, TO_CHAR);
  return TRUE;
}


    /*
     * Remove an object.
     */
bool remove_obj (CHAR_DATA * ch, int iWear, bool fReplace)
{
  OBJ_DATA *obj;
  if ((obj = get_eq_char (ch, iWear)) == NULL)
    return TRUE;
  if (!fReplace)
    return FALSE;
  if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
    {
      act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
      return FALSE;
    }
  if (obj->item_type == ITEM_POLE && ch->pcdata && ch->pcdata->fishing)
    {
      send_to_char
	("You cannot remove your pole WHILE you're fishing!\n\r", ch);
      return FALSE;
    }
  if (obj->item_type == ITEM_WEAPON && IS_WEAPON_STAT(obj,WEAPON_HOLY_ACTIVATED))
    REMOVE_BIT(obj->value[4],WEAPON_HOLY_ACTIVATED);
  if (obj->item_type == ITEM_POLE)
    obj->value[1] = 0;
  unequip_char (ch, obj);
  act ("$n stops using $p.", ch, obj, NULL, TO_ROOM);
  act ("You stop using $p.", ch, obj, NULL, TO_CHAR);
  return TRUE;
}


/*
 * Remove an object.
 */
bool remove_obj_new (CHAR_DATA * ch, OBJ_DATA *obj, bool fReplace)
{
  //OBJ_DATA *obj;
  //  if ((obj = get_eq_char_new (ch, iWear)) == NULL)
  //   return TRUE;
  if (!fReplace)
    return FALSE;
  if (IS_SET (obj->extra_flags[0], ITEM_NOREMOVE) && !IS_IMMORTAL (ch))
    {
      act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
      return FALSE;
    }
  if (obj->item_type == ITEM_POLE && ch->pcdata && ch->pcdata->fishing)
    {
      send_to_char
        ("You cannot remove your pole WHILE you're fishing!\n\r", ch);
      return FALSE;
    }
  if (obj->item_type == ITEM_WEAPON && IS_WEAPON_STAT(obj,WEAPON_HOLY_ACTIVATED))
    REMOVE_BIT(obj->value[4],WEAPON_HOLY_ACTIVATED);
  if (obj->item_type == ITEM_POLE)
    obj->value[1] = 0;
  unequip_char (ch, obj);
  act ("$n stops using $p.", ch, obj, NULL, TO_ROOM);
  act ("You stop using $p.", ch, obj, NULL, TO_CHAR);
  return TRUE;
}


    /*
     * Wear one object.
     * Optional replacement of existing objects.
     * Big repetitive code, ick.
     */
void wear_obj (CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace, int hand)
{
  char buf[MAX_STRING_LENGTH];
  //int wear_first_choice, wear_second_choice;
 // long wear_flags_remaining = 0;
  int i,amount;
  char *last_statement;

  if (hand == 0)
  {
    if (!IS_NPC(ch))
      hand = ch->pcdata->primary_hand;
    else 
    {
      hand = HAND_RIGHT;
      if (number_percent() <= 10)
        hand = HAND_LEFT;
    }
  }
  else if (!IS_NPC(ch))
    {
      switch (hand)
	{
	default:
	case HAND_RIGHT:
	  if (ch->pcdata->primary_hand == HAND_LEFT
	      && get_skill (ch, gsn_dual_wield) == 0)
	    {
                      send_to_char
                        ("You can't wield a weapon in that hand.\n\r", ch);
                      return;
	    }
	  break;
	case HAND_LEFT:
	  if (ch->pcdata->primary_hand == HAND_RIGHT
	      && get_skill (ch, gsn_dual_wield) == 0)
	    {
                      send_to_char
                        ("You can't wield a weapon in that hand.\n\r", ch);
                      return;
	    }
	  break;
	}
    }

  if (IS_CLASS(ch, PC_CLASS_DRUID) && (obj->item_type == ITEM_WEAPON) && (obj->value[0] != WEAPON_STAFF)) {
	send_to_char("You may wield only your Shillelagh or a staff.\n\r", ch);
	return;
  }

  // Adeon 6/29/03 -- prevent people from wearing eq owned by others
  if (obj->plr_owner != NULL)
    if (str_cmp (ch->name, obj->plr_owner) && !IS_IMMORTAL(ch))
      {
	sprintf (buf, "You do not own %s!\n\r", obj->short_descr);
	send_to_char (buf, ch);
	return;
      }
  if (ch->level < (obj->level - EQUIP_LEVEL_DIFF))
    {
      sprintf (buf,
	       "You must be at least level %d to use this object.\n\r",
	       (obj->level - EQUIP_LEVEL_DIFF));
      send_to_char (buf, ch);
      act ("$n tries to use $p, but is too inexperienced.", ch, obj,
	   NULL, TO_ROOM);
      return;
    }
// Minax 7-9-02 Elves and Syvin cannot use items made of steel or iron.
  if (((str_cmp (obj->material, "steel") == 0) ||
       (str_cmp (obj->material, "iron") == 0)) &&
      ((ch->race == PC_RACE_ELF) || (ch->race == PC_RACE_SYVIN)))
    {
      act ("You are burned by the touch of iron and drop $p.", ch, obj,
	   NULL, TO_CHAR);
      act ("$n is burned by $p and drops it.", ch, obj, NULL, TO_ROOM);
      obj_from_char (obj);
      obj_to_room (obj, ch->in_room);
      return;
    }
  if (ch->race == PC_RACE_NERIX)
  {
    if (IS_SET(obj->wear_flags,ITEM_WIELD) || IS_SET(obj->wear_flags,ITEM_WEAR_ARMS)
	    || IS_SET(obj->wear_flags,ITEM_WEAR_HANDS) || IS_SET(obj->wear_flags,ITEM_WEAR_SHIELD))
    {
	    act("You cannot wear $p.",ch,obj,NULL,TO_CHAR);
	    return;
    }
  }
  if (ch->race == PC_RACE_SWARM)
  {
    if (!(IS_SET(obj->wear_flags,ITEM_HOLD) || 
 	    (IS_SET(obj->wear_flags,ITEM_WEAR_BODY) && IS_SET(obj->race_flags,RACE_SWARM))
		    || !IS_SET(obj->wear_flags,ITEM_WEAR_FLOAT) || obj->item_type == ITEM_LIGHT))
    {
	    act("You cannot wear $p.",ch,obj,NULL,TO_CHAR);
	    return;
    }
  }
  if (IS_SET(ch->act2,ACT_FAMILIAR))
  {
    if (IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_FAM_SKELETON)
     {
       if (obj->item_type == ITEM_LIGHT
           || (!IS_SET(obj->wear_flags,ITEM_WEAR_HEAD)
		   && !IS_SET(obj->wear_flags,ITEM_WEAR_SHIELD)
		   && !IS_SET(obj->wear_flags,ITEM_WIELD)))
       {
	       return;
       }
     }
    if (IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_FAM_ZOMBIE)
    {
      if (obj->item_type == ITEM_LIGHT
          || (!IS_SET(obj->wear_flags,ITEM_WEAR_LEGS)
                && !IS_SET(obj->wear_flags,ITEM_WIELD)
                && !IS_SET(obj->wear_flags,ITEM_WEAR_BODY)))
      {
	      return;
      }
    }
    if (IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_FAM_BANSHEE)
    {
      if (obj->item_type != ITEM_LIGHT
          && (!IS_SET(obj->wear_flags,ITEM_WEAR_FLOAT)))
	  {
	  return;
	  }
    }
  }

  if (IS_CLASS (ch, PC_CLASS_ASSASSIN) && IS_SET(obj->wear_flags,ITEM_WEAR_SHIELD))
  {
    if (fReplace)
      send_to_char ("Assassins may not wear shields.\n\r",ch);
    return;
  }

  if (IS_SET(obj->wear_flags,ITEM_WEAR_BODY) && is_affected(ch,skill_lookup("swell")))
   {
       send_to_char("You're too swollen to wear that.\n\r",ch);
     return;
   }

  if (hands_required(ch,obj) > 2-hands_used(ch))
  {
    int hands_needed=0;
    //check if removing the slots that it takes would free up enough hands.  If so, it is ok
    hands_needed = hands_required(ch,obj) - (2-hands_used(ch));
    if (hands_needed && IS_SET(obj->wear_flags,ITEM_WEAR_SHIELD) && get_eq_char_new(ch,ITEM_WEAR_SHIELD) 
		    && can_remove_obj_silent(ch,ITEM_WEAR_SHIELD,fReplace))
      --hands_needed;
    if (hands_needed && IS_SET(obj->wear_flags,ITEM_HOLD) && get_eq_char_new(ch,ITEM_HOLD) 
		    && can_remove_obj_silent(ch,ITEM_HOLD,fReplace))
    {
      OBJ_DATA *held;
      held = get_eq_char_new(ch,ITEM_HOLD);
      if (held->item_type == ITEM_INSTRUMENT && IS_SET (obj->value[0], A) && ch->size < SIZE_LARGE)
        --hands_needed;	      
      --hands_needed;
    }
    if (hands_needed > 0)
    {
      if (fReplace)
      {
        send_to_char("You don't have enough hands for that object.\n\r",ch);
        if (IS_IMMORTAL(ch))
        {
          sprintf (buf,"You need %d free hands.\n\r",hands_needed);
          send_to_char(buf,ch);
        }
      }
      return;
    }
  }
  if (!fReplace)
  {
    if (!all_slots_free(ch,obj,hand))
      return;
    if (hands_used(ch) > 1 && 
 	    (IS_SET(obj->wear_flags,ITEM_HOLD) || IS_SET(obj->wear_flags,ITEM_WIELD)
	     || IS_SET(obj->wear_flags,ITEM_TWO_HANDS) || IS_SET(obj->wear_flags,ITEM_WEAR_SHIELD)))
      return;
    if (hands_used(ch) > 0 && 
	     ((obj->item_type == ITEM_INSTRUMENT && IS_SET (obj->value[0], A) && ch->size < SIZE_LARGE) ||
	     (obj->item_type == ITEM_WEAPON && ch->size < SIZE_LARGE && IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS))))
      return;
    if ((get_eq_char_new(ch,ITEM_WIELD) || get_eq_char_new(ch,ITEM_TWO_HANDS))
         && get_skill (ch, gsn_dual_wield) == 0 
	 && IS_SET(obj->wear_flags,ITEM_WIELD))
      return;
  }
  else
  {
    if (!char_can_remove_to_wear_hand(ch,obj,TRUE,hand))
      return;
  }
  if (IS_SET(obj->wear_flags,ITEM_WIELD) && obj->item_type != ITEM_WEAPON)
  {
	act("$p is a BAD item.  You cannot wield a non-weapon.  Tell an imm.",ch,obj,NULL,TO_CHAR);
	return;
  }

  if (!(obj->wear_flags & ~(ITEM_TAKE) & ~(ITEM_NO_SAC)) && obj->item_type != ITEM_LIGHT)
  {
    if (fReplace)
      send_to_char ("You can't wear, wield, or hold that.\n\r", ch);
    return;
  }

  //Need to Handle BOTH WRIST, BOTH NECK, and BOTH FINGER, and BOTH HANDS
  //remove_and_equip_char(ch,obj,TRUE);
//  if (!equip_char_new_hand (ch,obj,hand))
//	  return;
  if (IS_SET(obj->extra_flags[1],ITEM_INVIS_WEAR))
  {
    if (!equip_char_new_hand (ch,obj,hand))
      return;
    return;
  }
  send_to_char ("You wear ",ch);
  send_to_char (obj->short_descr,ch);
  send_to_char (" ",ch);
  amount = 0;
  last_statement = NULL;
  for (i = 1; wear_conversion_table[i].wear_loc != -2;++i)
  {
    if (IS_SET(obj->wear_flags,wear_conversion_table[i].wear_flag))
    {
      //take care of cases already taken case of
      if (wear_conversion_table[i].wear_flag == ITEM_WEAR_ANOTHER_FINGER
		      && IS_SET(obj->wear_flags,ITEM_WEAR_FINGER))
        continue;
      if (wear_conversion_table[i].wear_flag == ITEM_WEAR_ANOTHER_NECK
		      && IS_SET(obj->wear_flags,ITEM_WEAR_NECK))
        continue;
      if (wear_conversion_table[i].wear_flag == ITEM_WEAR_ANOTHER_WRIST
		      && IS_SET(obj->wear_flags,ITEM_WEAR_WRIST))
        continue;
      if (wear_conversion_table[i].wear_flag == ITEM_TWO_HANDS
	      	      && IS_SET(obj->wear_flags,ITEM_WIELD))
        continue;
		
	    ++amount;
      if (amount < 2)
      {
       // send_to_char(wear_conversion_table[i].wear_string);
	
      }
      else
      {
        if (amount > 2)
         send_to_char(", ",ch);
        send_to_char(last_statement,ch);
      }
//      send_to_char(last_statement);
      if (wear_conversion_table[i].wear_flag == ITEM_WEAR_WRIST
		      && IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_WRIST))
        last_statement = "around both wrists";
      else if (wear_conversion_table[i].wear_flag == ITEM_WEAR_FINGER
		       && IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_FINGER))
        last_statement = "on both fingers";
      else if (wear_conversion_table[i].wear_flag == ITEM_WEAR_NECK
		       && IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_NECK))
        last_statement = "around your entire neck";
      else last_statement = wear_conversion_table[i].wear_string;
    }
  }
  if (amount > 2)
    send_to_char(",",ch);
  if (amount > 1)
    send_to_char(" and ",ch);
  if (last_statement)
    send_to_char(last_statement,ch);
  send_to_char(".\n\r",ch);
  act("$n wears $p.",ch,obj,NULL,TO_ROOM);
  if (!equip_char_new_hand (ch,obj,hand))
	      return;
  if (IS_SET(obj->wear_flags,ITEM_WIELD))
    {
      int sn=0,skill=0;
      if (IS_SET(obj->wear_loc,ITEM_WIELD))
        sn = get_weapon_sn (ch, WEAR_WIELD_R);
      else sn = get_weapon_sn (ch, WEAR_WIELD_L);
      if (sn == gsn_hand_to_hand)
	return;
      skill = get_weapon_skill (ch, sn);
      if (skill >= 100)
	act ("$p feels like a part of you!", ch, obj, NULL, TO_CHAR);

      else if (skill > 85)
	act ("You feel quite confident with $p.", ch, obj, NULL, TO_CHAR);

      else if (skill > 70)
	act ("You are skilled with $p.", ch, obj, NULL, TO_CHAR);

      else if (skill > 50)
	act ("Your skill with $p is adequate.", ch, obj, NULL, TO_CHAR);

      else if (skill > 25)
	act ("$p feels a little clumsy in your hands.", ch, obj,
	     NULL, TO_CHAR);

      else if (skill > 1)
	act ("You fumble and almost drop $p.", ch, obj, NULL, TO_CHAR);

          else
            act ("You don't even know which end is up on $p.", ch,
                 obj, NULL, TO_CHAR);
    }
  
  return;
  
/*wear_flags_remaining = obj->wear_flags;
  
  if (obj->item_type == ITEM_LIGHT || IS_SET(wear_flags_remaining,ITEM_WEAR_LIGHT))
    {
      if (!remove_obj (ch, WEAR_LIGHT, fReplace))
	return;
      act ("$n lights $p.", ch, obj, NULL, TO_ROOM);
      act ("You light $p.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, ITEM_WEAR_LIGHT);
      return;
    }
  if (IS_SET(wear_flags_remaining, ITEM_WEAR_FINGER))
    {
      if (get_eq_char (ch, WEAR_FINGER_L) != NULL
	  && get_eq_char (ch, WEAR_FINGER_R) != NULL
	  && !remove_obj (ch, WEAR_FINGER_L, fReplace)
	  && !remove_obj (ch, WEAR_FINGER_R, fReplace))
	return;
      if (get_eq_char (ch, WEAR_FINGER_L) == NULL)
	{
	  act ("$n wears $p on $s left finger.", ch, obj, NULL, TO_ROOM);
	  act ("You wear $p on your left finger.", ch, obj, NULL, TO_CHAR);
	  equip_char (ch, obj, ITEM_WEAR_FINGER);
	  if (IS_SET(wear_flags_remaining,ITEM_WEAR_ANOTHER_FINGER))
  	    wear_flags_remaining -= ITEM_WEAR_ANOTHER_FINGER;
          else wear_flags_remaining -= ITEM_WEAR_FINGER;
//	  return;
	}
      if (get_eq_char (ch, WEAR_FINGER_R) == NULL && IS_SET(wear_flags_remaining,ITEM_WEAR_FINGER))
	{
	  act ("$n wears $p on $s right finger.", ch, obj, NULL, TO_ROOM);
	  act ("You wear $p on your right finger.", ch, obj, NULL, TO_CHAR);
	  equip_char (ch, obj, ITEM_WEAR_ANOTHER_FINGER);
	  wear_flags_remaining -= ITEM_WEAR_FINGER;
	  return;
	}
      bug ("Wear_obj: no free finger.", 0);
      send_to_char ("You already wear two rings.\n\r", ch);
      return;
    }
  if (IS_SET (wear_flags_remaining, ITEM_WEAR_NECK))
    {
      if (get_eq_char (ch, WEAR_NECK_1) != NULL
	  && get_eq_char (ch, WEAR_NECK_2) != NULL
	  && !remove_obj (ch, WEAR_NECK_1, fReplace)
	  && !remove_obj (ch, WEAR_NECK_2, fReplace))
	return;
      if (get_eq_char (ch, WEAR_NECK_1) == NULL)
	{
	  act ("$n wears $p around $s neck.", ch, obj, NULL, TO_ROOM);
	  act ("You wear $p around your neck.", ch, obj, NULL, TO_CHAR);
	  equip_char (ch, obj, ITEM_WEAR_NECK);
          if (IS_SET(wear_flags_remaining,ITEM_WEAR_ANOTHER_NECK))
            wear_flags_remaining -= ITEM_WEAR_ANOTHER_NECK;
          else wear_flags_remaining -= ITEM_WEAR_NECK;
//	  return;
	}
      if (get_eq_char (ch, WEAR_NECK_2) == NULL && IS_SET(wear_flags_remaining,ITEM_WEAR_NECK))
	{
	  act ("$n wears $p around $s neck.", ch, obj, NULL, TO_ROOM);
	  act ("You wear $p around your neck.", ch, obj, NULL, TO_CHAR);
	  equip_char (ch, obj, ITEM_WEAR_ANOTHER_NECK);
	  wear_flags_remaining -= ITEM_WEAR_NECK;
//	  return;
	}
      bug ("Wear_obj: no free neck.", 0);
      send_to_char ("You already wear two neck items.\n\r", ch);
      return;
    }
  if (IS_SET (wear_flags_remaining, ITEM_WEAR_BODY))
    {
      if (!remove_obj (ch, WEAR_BODY, fReplace))
	return;
      if (is_affected(ch,skill_lookup("swell")))
        {
          send_to_char("You're too swollen to wear that.\n\r",ch);
          return;
        }     
      act ("$n wears $p about $s torso.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p about your torso.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, ITEM_WEAR_BODY);
      wear_flags_remaining -= ITEM_WEAR_BODY;
//      return;
    }
  if (IS_SET (wear_flags_remaining, ITEM_WEAR_HEAD))
    {
      if (!remove_obj (ch, WEAR_HEAD, fReplace))
	return;
      act ("$n wears $p on $s head.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p on your head.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, ITEM_WEAR_HEAD);
      wear_flags_remaining -= ITEM_WEAR_HEAD;
  //    return;
    }
  if (IS_SET (wear_flags_remaining, ITEM_WEAR_LEGS))
    {
      if (is_affected(ch,skill_lookup("swell")))
      {
        send_to_char("You're too swollen to wear that.\n\r",ch);
	return;
      }
      if (!remove_obj (ch, WEAR_LEGS, fReplace))
	return;
      act ("$n wears $p on $s legs.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p on your legs.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, ITEM_WEAR_LEGS);
      wear_flags_remaining -= ITEM_WEAR_LEGS;
    //  return;
    }
  if (IS_SET (wear_flags_remaining, ITEM_WEAR_FEET))
    {
      if (!remove_obj (ch, WEAR_FEET, fReplace))
	return;
      act ("$n wears $p on $s feet.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p on your feet.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, ITEM_WEAR_FEET);
      wear_flags_remaining -= ITEM_WEAR_FEET;
     // return;
    }
  if (IS_SET (wear_flags_remaining, ITEM_WEAR_CLAN_MARK))
    {
      if (!remove_obj (ch, WEAR_CLAN_MARK, fReplace))
	return;
      act ("$n wears $p as a mark of $s clan.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p as a mark of your clan.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, ITEM_WEAR_CLAN_MARK);
      wear_flags_remaining -= ITEM_WEAR_CLAN_MARK;
     // return;
    }
  if (IS_SET (wear_flags_remaining, ITEM_WEAR_HANDS))
    {
      if (!remove_obj (ch, WEAR_HANDS, fReplace))
	return;
      act ("$n wears $p on $s hands.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p on your hands.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, ITEM_WEAR_HANDS);
      wear_flags_remaining -= ITEM_WEAR_HANDS;
    //  return;
    }
  if (IS_SET (wear_flags_remaining, ITEM_WEAR_ARMS))
    {
      if (!remove_obj (ch, WEAR_ARMS, fReplace))
	return;
      act ("$n wears $p on $s arms.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p on your arms.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, ITEM_WEAR_ARMS);
      wear_flags_remaining -= ITEM_WEAR_ARMS;
     // return;
    }
  if (IS_SET (wear_flags_remaining, ITEM_WEAR_ABOUT))
    {
      if (!remove_obj (ch, WEAR_ABOUT, fReplace))
	return;
      act ("$n wears $p about $s body.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p about your body.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, ITEM_WEAR_ABOUT);
      wear_flags_remaining -= ITEM_WEAR_ABOUT;
     // return;
    }
  if (IS_SET (wear_flags_remaining, ITEM_WEAR_WAIST))
    {
      if (!remove_obj (ch, WEAR_WAIST, fReplace))
	return;
      act ("$n wears $p about $s waist.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p about your waist.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, ITEM_WEAR_WAIST);
      wear_flags_remaining -= ITEM_WEAR_WAIST;
    //  return;
    }
  if (IS_SET (wear_flags_remaining, ITEM_WEAR_WRIST))
    {
      if (get_eq_char (ch, WEAR_WRIST_L) != NULL
	  && get_eq_char (ch, WEAR_WRIST_R) != NULL
	  && !remove_obj (ch, WEAR_WRIST_L, fReplace)
	  && !remove_obj (ch, WEAR_WRIST_R, fReplace))
	return;
      if (get_eq_char (ch, WEAR_WRIST_L) == NULL)
	{
	  act ("$n wears $p around $s left wrist.", ch, obj, NULL, TO_ROOM);
	  act ("You wear $p around your left wrist.", ch, obj, NULL, TO_CHAR);
	  equip_char (ch, obj, ITEM_WEAR_WRIST);
	  if (IS_SET(wear_flags_remaining,ITEM_WEAR_ANOTHER_WRIST))
             wear_flags_remaining -= ITEM_WEAR_ANOTHER_WRIST;
          else wear_flags_remaining -= ITEM_WEAR_WRIST;
//	  return;
	}
      if (get_eq_char (ch, WEAR_WRIST_R) == NULL && IS_SET(wear_flags_remaining,ITEM_WEAR_WRIST))
	{
	  act ("$n wears $p around $s right wrist.", ch, obj, NULL, TO_ROOM);
	  act ("You wear $p around your right wrist.", ch, obj, NULL,
	       TO_CHAR);
	  equip_char (ch, obj, ITEM_WEAR_ANOTHER_WRIST);
	  wear_flags_remaining -= ITEM_WEAR_WRIST;
//	  return;
	}
      bug ("Wear_obj: no free wrist.", 0);
      send_to_char ("You already wear two wrist items.\n\r", ch);
      return;
    }
  if (IS_SET (wear_flags_remaining, ITEM_WEAR_SHIELD))
    {
      OBJ_DATA *weapon, *weapon2, *held;
      if (!remove_obj (ch, WEAR_SHIELD, fReplace))
	return;
      held = get_eq_char (ch, WEAR_HOLD);
      if (held)
	{
	  if (held->item_type == ITEM_INSTRUMENT
	      && IS_SET (held->value[0], A))
	    {
	      send_to_char
		("You may not wear a shield while holding a two-handed instrument.\n\r",
		 ch);
	      return;
	    }
	}
      weapon = get_eq_char (ch, WEAR_WIELD_R);
      weapon2 = get_eq_char (ch, WEAR_WIELD_L);
      if (held && (weapon != NULL || weapon2 != NULL))
	{
	  send_to_char
	    ("Your hands are full wielding a weapon and holding an item.\n\r",
	     ch);
	  return;
	}
      if (weapon != NULL && weapon2 != NULL)
	{
	  send_to_char
	    ("You may not wear a shield while wielding two weapons.\n\r", ch);
	  return;
	}
// Minax 7/1/02 Assassins do not get to use shields
      if (IS_CLASS (ch, PC_CLASS_ASSASSIN))
	{
	  send_to_char ("Assassins may not use shields.\n\r", ch);
	  return;
	}
      if (weapon != NULL && ch->size < SIZE_LARGE
	  && IS_WEAPON_STAT (weapon, WEAPON_TWO_HANDS))
	{
	  send_to_char ("Your hands are tied up with your weapon!\n\r", ch);
	  return;
	}
      if (weapon2 != NULL && ch->size < SIZE_LARGE
	  && IS_WEAPON_STAT (weapon2, WEAPON_TWO_HANDS))
	{
	  send_to_char ("Your hands are tied up with your weapon!\n\r", ch);
	  return;
	}
      act ("$n wears $p as a shield.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p as a shield.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, WEAR_SHIELD);
      return;
    }
  if (CAN_WEAR (obj, ITEM_WIELD))
    {
      int sn, skill;
      OBJ_DATA *weapon, *held, *shield;
      if (IS_SET (obj->wear_flags, ITEM_WIELD) && IS_CLASS (ch, PC_CLASS_REAVER))
	{
	  send_to_char ("You may wield only your Reaver Blade.\n\r", ch);
	  return;
	}
      if (IS_SET (obj->wear_flags, ITEM_WIELD) && IS_CLASS (ch, PC_CLASS_MONK))
	{
	  send_to_char ("Your fists are the only weapon you need\n\r", ch);
	  return;
	}

	if (IS_CLASS(ch, PC_CLASS_DRUID) && (obj->item_type == ITEM_WEAPON) && (obj->value[0] != WEAPON_STAFF)) {
		send_to_char("You may wield only your Shillelagh or a staff.\n\r", ch);
		return;
	}

      held = get_eq_char (ch, WEAR_HOLD);
      shield = get_eq_char (ch, WEAR_SHIELD);
      if (held)
	{
	  if (held->item_type == ITEM_INSTRUMENT
	      && IS_SET (held->value[0], A) && ch->size < SIZE_LARGE)
	    {
	      send_to_char
		("You may not wield a weapon while holding a two-handed instrument.\n\r",
		 ch);
	      return;
	    }
	}
      if (IS_NPC (ch))
	{
	  wear_first_choice = WEAR_WIELD_R;
	  wear_second_choice = WEAR_WIELD_L;
	}
      else
	{
	  if (hand != 0)
	    {
	      switch (hand)
		{
		default:
		case HAND_RIGHT:
		  if (ch->pcdata->primary_hand == HAND_LEFT
		      && get_skill (ch, gsn_dual_wield) == 0)
		    {
		      send_to_char
			("You can't wield a weapon in that hand.\n\r", ch);
		      return;
		    }
		  wear_first_choice = WEAR_WIELD_R;
		  wear_second_choice = WEAR_WIELD_R;
		  break;
		case HAND_LEFT:
		  if (ch->pcdata->primary_hand == HAND_RIGHT
		      && get_skill (ch, gsn_dual_wield) == 0)
		    {
		      send_to_char
			("You can't wield a weapon in that hand.\n\r", ch);
		      return;
		    }
		  wear_first_choice = WEAR_WIELD_L;
		  wear_second_choice = WEAR_WIELD_L;
		  break;
		}
	    }
	  else
	    {
	      switch (ch->pcdata->primary_hand)
		{
		default:
		case HAND_AMBIDEXTROUS:
		case HAND_RIGHT:
		  wear_first_choice = WEAR_WIELD_R;
		  wear_second_choice = WEAR_WIELD_L;
		  break;
		case HAND_LEFT:
		  wear_first_choice = WEAR_WIELD_L;
		  wear_second_choice = WEAR_WIELD_R;
		}
	      if (ch->pcdata->primary_hand == HAND_AMBIDEXTROUS
		  && get_skill (ch, gsn_dual_wield) == 0
		  && (get_eq_char (ch, WEAR_WIELD_L) != NULL
		      || get_eq_char (ch, WEAR_WIELD_R) != NULL))
		{
		  send_to_char
		    ("You aren't capable of wielding two weapons at once.\n\r",
		     ch);
		  return;
		}
	    }
	}

      if (held && shield)
	{
	  send_to_char
	    ("Your hands are already full wearing a shield and holding an item.\n\r",
	     ch);
	  return;
	}
      if (held && (get_eq_char (ch, WEAR_WIELD_L) != NULL
			                            || get_eq_char (ch, WEAR_WIELD_R) != NULL))
      {
	      send_to_char("Your hands are already full holding an item and wielding a weapon.\n\r",ch);
	      return;
      } 
      if (get_eq_char (ch, wear_first_choice) != NULL
	  && get_eq_char (ch, wear_second_choice) != NULL
	  && !remove_obj (ch, wear_first_choice, fReplace)
	  && !remove_obj (ch, wear_second_choice, fReplace))
	return;
      if (get_eq_char (ch, wear_first_choice) == NULL)
	{
	  if (wear_first_choice == WEAR_WIELD_R)
	    weapon = get_eq_char (ch, WEAR_WIELD_L);

	  else
	    weapon = get_eq_char (ch, WEAR_WIELD_R);
	  if (weapon != NULL
	      &&
	      ((IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS) && ch->size < SIZE_LARGE)
	       || get_eq_char (ch, WEAR_SHIELD) != NULL
	       || get_eq_char (ch, WEAR_HOLD) != NULL))
	    {
	      send_to_char ("You need a free hand for that weapon.\n\r", ch);
	      return;
	    }
	  if (weapon != NULL && ch->size < SIZE_LARGE
	      && IS_WEAPON_STAT (weapon, WEAPON_TWO_HANDS))
	    {
	      send_to_char
		("Your hands are tied up with your weapon!\n\r", ch);
	      return;
	    }
	  if (!IS_NPC (ch)
	      && get_obj_weight (obj) >
	      (str_app[get_curr_stat (ch, STAT_STR)].wield * 10))
	    {
	      send_to_char ("It is too heavy for you to wield.\n\r", ch);
	      return;
	    }
	  if (!IS_NPC (ch) && ch->size < SIZE_LARGE
	      && IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS)
	      && (get_eq_char (ch, WEAR_SHIELD) != NULL
		  || get_eq_char (ch, WEAR_HOLD)))
	    {
	      send_to_char
		("You need two hands free for that weapon.\n\r", ch);
	      return;
	    }
	  if (!IS_NPC (ch))
	    {
	      if (ch->pcdata->primary_hand != HAND_AMBIDEXTROUS)
		{
		  if (wear_first_choice == WEAR_WIELD_R)
		    {
		      OBJ_DATA *tobj;
		      tobj = get_eq_char (ch, WEAR_WIELD_L);
		      if (tobj != NULL)
			if (get_obj_weight (tobj) >= get_obj_weight (obj))
			  {
			    act
			      ("Wielding $p in your right hand will throw off your balance.",
			       ch, obj, NULL, TO_CHAR);
			    return;
			  }
		    }
		  if (wear_first_choice == WEAR_WIELD_L)
		    {
		      OBJ_DATA *tobj;
		      tobj = get_eq_char (ch, WEAR_WIELD_R);
		      if (tobj != NULL)
			if (get_obj_weight (tobj) >= get_obj_weight (obj))
			  {
			    act
			      ("Wielding $p in your left hand will throw off your balance.",
			       ch, obj, NULL, TO_CHAR);
			    return;
			  }
		    }
		}
	    }
	  if (wear_first_choice == WEAR_WIELD_L)
	    {
	      act ("You wield $p in your left hand.", ch, obj, NULL, TO_CHAR);
	      act ("$n wields $p in $s left hand.", ch, obj, NULL, TO_ROOM);
	    }
	  else
	    {
	      act ("You wield $p in your right hand.", ch, obj, NULL,
		   TO_CHAR);
	      act ("$n wields $p in $s right hand.", ch, obj, NULL, TO_ROOM);
	    }
	  equip_char (ch, obj, wear_first_choice);
	  sn = get_weapon_sn (ch, wear_first_choice);
	  if (sn == gsn_hand_to_hand)
	    return;
	  skill = get_weapon_skill (ch, sn);
	  if (skill >= 100)
	    act ("$p feels like a part of you!", ch, obj, NULL, TO_CHAR);

	  else if (skill > 85)
	    act ("You feel quite confident with $p.", ch, obj, NULL, TO_CHAR);

	  else if (skill > 70)
	    act ("You are skilled with $p.", ch, obj, NULL, TO_CHAR);

	  else if (skill > 50)
	    act ("Your skill with $p is adequate.", ch, obj, NULL, TO_CHAR);

	  else if (skill > 25)
	    act ("$p feels a little clumsy in your hands.", ch, obj,
		 NULL, TO_CHAR);

	  else if (skill > 1)
	    act ("You fumble and almost drop $p.", ch, obj, NULL, TO_CHAR);

	  else
	    act ("You don't even know which end is up on $p.", ch,
		 obj, NULL, TO_CHAR);
	  return;
	}
      if (get_skill (ch, gsn_dual_wield) < 1)
	{
	  send_to_char
	    ("You aren't capable of wielding two weapons at once.\n\r", ch);
	  return;
	}
      if (get_eq_char (ch, wear_second_choice) == NULL)
	{
	  if (!IS_NPC (ch)
	      && get_obj_weight (obj) >
	      (str_app[get_curr_stat (ch, STAT_STR)].wield * 10))
	    {
	      send_to_char ("It is too heavy for you to wield.\n\r", ch);
	      return;
	    }
	  if (!IS_NPC (ch) && ch->size < SIZE_LARGE
	      && IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS)
	      && get_eq_char (ch, WEAR_SHIELD) != NULL)
	    {
	      send_to_char
		("You need two hands free for that weapon.\n\r", ch);
	      return;
	    }
	  if (wear_second_choice == WEAR_WIELD_R)
	    weapon = get_eq_char (ch, WEAR_WIELD_L);

	  else
	    weapon = get_eq_char (ch, WEAR_WIELD_R);
	  if (weapon != NULL && ch->size < SIZE_LARGE
	      && IS_WEAPON_STAT (weapon, WEAPON_TWO_HANDS))
	    {
	      send_to_char
		("Your hands are tied up with your other weapon!\n\r", ch);
	      return;
	    }
// Minax 7-4-02 Changed things so that large races (Vro'ath) may wield two
// two-handed weapons at the same time.
	  if (weapon != NULL && (IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS)
				 && ch->size < SIZE_LARGE))
	    {
	      send_to_char ("You need a free hand for that weapon.\n\r", ch);
	      return;
	    }
	  if (weapon != NULL && get_eq_char (ch, WEAR_SHIELD) != NULL)
	    {
	      send_to_char ("You need a free hand for that weapon.\n\r", ch);
	      return;
	    }
	  if (!IS_NPC (ch))
	    {
	      if (ch->pcdata->primary_hand != HAND_AMBIDEXTROUS)
		{
		  if (wear_second_choice == WEAR_WIELD_R)
		    {
		      OBJ_DATA *tobj;
		      tobj = get_eq_char (ch, WEAR_WIELD_L);
		      if (tobj != NULL)
			if (get_obj_weight (tobj) <= get_obj_weight (obj))
			  {
			    act
			      ("Wielding $p in your right hand will throw off your balance.",
			       ch, obj, NULL, TO_CHAR);
			    return;
			  }
		    }
		  if (wear_second_choice == WEAR_WIELD_L)
		    {
		      OBJ_DATA *tobj;
		      tobj = get_eq_char (ch, WEAR_WIELD_R);
		      if (tobj != NULL)
			if (get_obj_weight (tobj) <= get_obj_weight (obj))
			  {
			    act
			      ("Wielding $p in your left hand will throw off your balance.",
			       ch, obj, NULL, TO_CHAR);
			    return;
			  }
		    }
		}
	    }
	  if (wear_second_choice == WEAR_WIELD_L)
	    {
	      act ("You wield $p in your left hand.", ch, obj, NULL, TO_CHAR);
	      act ("$n wields $p in $s left hand.", ch, obj, NULL, TO_ROOM);
	    }
	  else
	    {
	      act ("You wield $p in your right hand.", ch, obj, NULL,
		   TO_CHAR);
	      act ("$n wields $p in $s right hand.", ch, obj, NULL, TO_ROOM);
	    }
	  equip_char (ch, obj, wear_second_choice);
	  sn = get_weapon_sn (ch, wear_second_choice);
	  if (sn == gsn_hand_to_hand)
	    return;
	  skill = get_weapon_skill (ch, sn);
	  if (skill >= 100)
	    act ("$p feels like a part of you!", ch, obj, NULL, TO_CHAR);

	  else if (skill > 85)
	    act ("You feel quite confident with $p.", ch, obj, NULL, TO_CHAR);

	  else if (skill > 70)
	    act ("You are skilled with $p.", ch, obj, NULL, TO_CHAR);

	  else if (skill > 50)
	    act ("Your skill with $p is adequate.", ch, obj, NULL, TO_CHAR);

	  else if (skill > 25)
	    act ("$p feels a little clumsy in your hands.", ch, obj,
		 NULL, TO_CHAR);

	  else if (skill > 1)
	    act ("You fumble and almost drop $p.", ch, obj, NULL, TO_CHAR);

	  else
	    act ("You don't even know which end is up on $p.", ch,
		 obj, NULL, TO_CHAR);
	  return;
	}
      return;
    }
  if (CAN_WEAR (obj, ITEM_HOLD))
    {
      OBJ_DATA *weapon_l, *weapon_r, *shield, *held;

      // this checks to see if there is an empty hand to hold something
      weapon_l = get_eq_char (ch, WEAR_WIELD_L);
      weapon_r = get_eq_char (ch, WEAR_WIELD_R);
      shield = get_eq_char (ch, WEAR_SHIELD);
      if (!remove_obj (ch, WEAR_HOLD, fReplace))
	return;
      held = get_eq_char (ch, WEAR_HOLD);
      if (shield && (weapon_l || weapon_r))
	{
	  send_to_char
	    ("Your hands are full wielding a weapon and wearing a shield.\n\r",
	     ch);
	  return;
	}
      // Akamai 6/9/98 -- Bug: #30 Can hold an instrument with two full hands
      // if you wear the things in the right order
      if (obj->item_type == ITEM_INSTRUMENT)
	{

	  // this is true if the instrument is a two handed instrument
	  if (IS_SET (obj->value[0], A))
	    {
	      if (get_eq_char (ch, WEAR_SHIELD) != NULL)
		{
		  send_to_char
		    ("You'll have to remove your shield before using this two-handed instrument.\n\r",
		     ch);
		  return;
		}
	      if ((weapon_r != NULL) || (weapon_l != NULL))
		{
		  send_to_char
		    ("You'll have to remove your weapon before using this two-handed instrument.\n\r",
		     ch);
		  return;
		}
	    }
	  // so it must be a one handed instrument, meaning you need at least one free hand
	  if (weapon_r != NULL && weapon_l != NULL)
	    {
	      send_to_char
		("You can't hold an instrument when both hands are full.\n\r",
		 ch);
	      send_to_char
		("You'll have to remove a weapon before using this instrument.\n\r",
		 ch);
	      return;
	    }
	}
      if (weapon_r != NULL && weapon_l != NULL)
	{
	  send_to_char
	    ("You'll have to remove a weapon before using this item.\n\r",
	     ch);
	  return;
	}
      if ((weapon_r != NULL
	   && IS_WEAPON_STAT (weapon_r, WEAPON_TWO_HANDS))
	  || (weapon_l != NULL
	      && IS_WEAPON_STAT (weapon_l, WEAPON_TWO_HANDS)))
	{
	  send_to_char
	    ("You'll have to remove your weapon before using this item.\n\r",
	     ch);
	  return;
	}
      equip_char (ch, obj, WEAR_HOLD);
      act ("You hold $p in your hand.", ch, obj, NULL, TO_CHAR);
      act ("$n holds $p in $s hand.", ch, obj, NULL, TO_ROOM);
      return;
    }
  if (IS_SET (wear_flags_remaining, ITEM_WEAR_FLOAT))
  {
      if (!remove_obj (ch, WEAR_FLOAT, fReplace))
	return;
      act ("$n releases $p to float next to $m.", ch, obj, NULL, TO_ROOM);
      act ("You release $p and it floats next to you.", ch, obj, NULL,
	   TO_CHAR);
      equip_char (ch, obj, WEAR_FLOAT);
      wear_flags_remaining -= ITEM_WEAR_FLOAT;
//      return;
    }
  if (IS_SET (wear_flags_remaining, ITEM_WEAR_WORN_WINGS))
  {
	if (!remove_obj (Ch, WEAR_WORN_WINGS, fReplace))
	  return;
	act ("$n wears $p on their wings.", ch, obj, NULL, TO_ROOM);
	act ("You wear $p on your wings.", ch, obj, NULL, TO_CHAR);
	equip_char (ch, obj, WEAR_WORN_WINGS);
	wear_flags_remaining -= ITEM_WEAR_WORN_WINGS;
  }	
  if (fReplace && wear_flags_remaining != 0)
    send_to_char ("You can't wear, wield, or hold that.\n\r", ch);
  return;*/
}

void do_wear (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Wear, wield, or hold what?\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "all"))
    {
      OBJ_DATA *obj_next;
      for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if (IS_SET (ch->act, ACT_PET) && obj->item_type != ITEM_CLOTHING)
	    continue;
	  //Mike 2/23/07 Fix for monk bug (they could wield weapons if they type 'wear all' added reavers to it too...)
	  if (IS_SET (obj->wear_flags, ITEM_WIELD) && (IS_CLASS (ch, PC_CLASS_REAVER) || IS_CLASS (ch, PC_CLASS_MONK)))
	    continue;
	  if (IS_SET (obj->wear_flags, ITEM_WORN_WINGS) && (!ch->race ==PC_RACE_NERIX))
	    continue;
	  if (obj->wear_loc == WEAR_NONE && can_see_obj (ch, obj))
	    wear_obj (ch, obj, FALSE, 0);
	}
      return;
    }
  else
    {
      if ((obj = get_obj_carry (ch, arg)) == NULL)
	{
	  send_to_char ("You do not have that item.\n\r", ch);
	  return;
	}
      if (IS_SET (obj->wear_flags, ITEM_WIELD) && IS_CLASS (ch, PC_CLASS_REAVER))
	{
	  send_to_char ("You may wield only your Reaver Blade.\n\r", ch);
	  return;
	}
      if (IS_SET (obj->wear_flags, ITEM_WIELD) && IS_CLASS (ch, PC_CLASS_MONK))
	{
	  send_to_char ("Your fists are the only weapon you need.\n\r", ch);
	  return;
	}
      if (obj->item_type != ITEM_CLOTHING && IS_SET (ch->act, ACT_PET))
	return;
      one_argument (argument, arg2);
      if (!str_cmp (arg2, "right"))
	wear_obj (ch, obj, TRUE, HAND_RIGHT);

      else if (!str_cmp (arg2, "left"))
	wear_obj (ch, obj, TRUE, HAND_LEFT);

      else
	wear_obj (ch, obj, TRUE, 0);
    }
  return;
}

void do_disrobe (CHAR_DATA * ch, char *argument)
{
  if (argument[0] != '\0')
    {
      send_to_char ("You can only disrobe yourself.\n\r", ch);
      return;
    }
  do_remove (ch, "all");
}

void do_remove (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char tcbuf[MAX_INPUT_LENGTH];
  CHAR_DATA *mount;
  OBJ_DATA *obj;
  argument = one_argument (argument, arg);
  argument = one_argument (argument, arg2);
  if (arg2[0] != '\0')
    {
      if (!str_cmp (arg2, "from"))
	one_argument (argument, arg2);
      if ((mount = get_char_room (ch, arg2)) == NULL)
	{
	  act ("You see no $T here.", ch, NULL, arg2, TO_CHAR);
	  return;
	}
      if (((obj = get_eq_char (mount, WEAR_BODY)) == NULL)
	  || !is_name (arg, obj->name))
	if (((obj = get_eq_char (mount, WEAR_ABOUT)) == NULL)
	    || !is_name (arg, obj->name))
	  {
	    sprintf (tcbuf, "Can't find that on %s.\n\r", mount->short_descr);
	    send_to_char (tcbuf, ch);
	    return;
	  }
      if (!IS_SET (mount->act, ACT_MOUNT)
	  || (mount->mounter != ch && mount->mounter != NULL))
	{
	  sprintf (tcbuf, "%s refuses to let you take $p from it.",
		   fcapitalize (mount->short_descr));
	  act (tcbuf, ch, obj, NULL, TO_CHAR);
	  return;
	}
      unequip_char (mount, obj);
      obj_from_char (obj);
      obj_to_char (obj, ch);
      sprintf (tcbuf, "You remove $p from %s.", mount->short_descr);
      act (tcbuf, ch, obj, NULL, TO_CHAR);
      sprintf (tcbuf, "$n removes $p from %s.", mount->short_descr);
      act (tcbuf, ch, obj, NULL, TO_ROOM);
      return;
    }
  if (arg[0] == '\0')
    {
      send_to_char ("Remove what?\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "all"))
    {
      OBJ_DATA *obj_next;
      for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if (obj->wear_loc != WEAR_NONE && can_see_obj (ch, obj))
	    remove_obj_new (ch, obj, TRUE);
	}
      return;
    }
  else
    {
      if ((obj = get_obj_wear (ch, arg)) == NULL)
	{
	  send_to_char ("You do not have that item.\n\r", ch);
	  return;
	}

      remove_obj_new (ch, obj, TRUE);
    }
  return;
}

void do_sacrifice (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  int silver;

  /* variables for AUTOSPLIT */
  CHAR_DATA *gch;
  int members;
  char buffer[100];
  one_argument (argument, arg);
  if (arg[0] == '\0' || !str_cmp (arg, ch->name))
    {
      act ("$n offers $mself to Chaos, who graciously declines.", ch,
	   NULL, NULL, TO_ROOM);
      send_to_char
	("Chaos appreciates your offer and may accept it later.\n\r", ch);
      return;
    }
  obj = get_obj_list (ch, arg, ch->in_room->contents);
  if (obj == NULL)
    {
      send_to_char ("You can't find it.\n\r", ch);
      return;
    }

  if (!CAN_WEAR (obj, ITEM_TAKE) || CAN_WEAR (obj, ITEM_NO_SAC)
      || IS_SET (obj->extra_flags[0], ITEM_NOSAC))
    {
      act ("$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR);
      return;
    }
  if (obj->item_type == ITEM_FURNITURE || obj->item_type == ITEM_RAFT)
    {
      CHAR_DATA *gch;
      for (gch = char_list; gch != NULL; gch = gch->next)
	{
	  if (gch->on == obj)
	    {
	      if (obj->item_type == ITEM_FURNITURE)
	        send_to_char ("You can't sacrifice furniture that is in use.\n\r", ch);
	      else send_to_char ("You can't sacrifice rafts that is in use.\n\r", ch);
	      return;
	    }
	}
    }
  silver = UMAX (1, obj->level * 2);
  if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    silver = UMIN (silver, obj->cost);
  if (silver == 1)
    send_to_char
      ("The gods give you one silver coin for your sacrifice.\n\r", ch);

  else
    {
      sprintf (buf,
	       "The gods give you %d silver coins for your sacrifice.\n\r",
	       silver);
      send_to_char (buf, ch);
    }
  ch->silver += silver;
  if (IS_SET (ch->act, PLR_AUTOSPLIT))
    {				/* AUTOSPLIT code */
      members = 0;
      for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
	  if (is_same_group (gch, ch))
	    members++;
	}
      if (members > 1 && silver > 1)
	{
	  sprintf (buffer, "%d", silver);
	  do_split (ch, buffer);
	}
    }
  act ("$n sacrifices $p to the gods.", ch, obj, NULL, TO_ROOM);
  wiznet ("$N sends up $p as a burnt offering.", ch, obj, WIZ_SACCING, 0, 0);
  extract_obj (obj);
  return;
}

void bad_affect (CHAR_DATA * ch, OBJ_DATA * obj)
{
  int sn;
  if (obj->item_type == ITEM_SCROLL)
    sn = number_range (0, 3);

  else
    sn = number_range (0, 5);
  switch (sn)
    {
    case 0:
      sn = gsn_blindness;
      break;
    case 1:
      sn = gsn_curse;
      break;
    case 2:
      sn = skill_lookup ("weaken");
      break;
    case 3:
      sn = skill_lookup ("slow");
      break;
    case 4:
      sn = gsn_poison;
      break;
    case 5:
      sn = skill_lookup ("change sex");
      break;
    }
  obj_cast_spell (sn, obj->level, ch, ch, obj);
}

void do_quaff (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Quaff what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_carry (ch, arg)) == NULL)
    {
      send_to_char ("You do not have that potion.\n\r", ch);
      return;
    }
  if (obj->item_type != ITEM_POTION && obj->item_type != ITEM_VIAL)
    {
      send_to_char ("You can quaff only potions.\n\r", ch);
      return;
    }
  if (obj->item_type == ITEM_VIAL && obj->value[0] == 0)
    {
      char tcbuf[MAX_INPUT_LENGTH];
      sprintf (tcbuf, "Sorry, %s is currently empty.\n\r", obj->short_descr);
      send_to_char (tcbuf, ch);
      return;
    }
   // Iblis 9/13/04 - No Magic flag
   if (IS_SET (ch->in_room->room_flags2, ROOM_NOMAGIC) && !IS_IMMORTAL (ch))
   {
     send_to_char("There are no magical forces to draw power from.\n\r",ch);
     return;
   }

  if (ch->level < (obj->level - MAGIC_LEVEL_DIFF))
    {
      int mojo;
      mojo = number_range (0, 4);
      if (mojo == 3 || mojo == 1)
	{
	  act ("$n quaffs $p.", ch, obj, NULL, TO_ROOM);
	  send_to_char
	    ("You drain the contents of a vial and a frightening surge rushes through you.\n\r",
	     ch);
	  bad_affect (ch, obj);
	  extract_obj (obj);
	  return;
	}
    }
  act ("$n quaffs $p.", ch, obj, NULL, TO_ROOM);
  act ("You quaff $p.", ch, obj, NULL, TO_CHAR);
  if (obj->item_type == ITEM_POTION)
    {
      obj_cast_spell (obj->value[1], obj->value[0], ch, ch, NULL);
      obj_cast_spell (obj->value[2], obj->value[0], ch, ch, NULL);
      obj_cast_spell (obj->value[3], obj->value[0], ch, ch, NULL);
      obj_cast_spell (obj->value[4], obj->value[0], ch, ch, NULL);
    }
  else
    {
      if (obj->value[0] != -1)
	obj_cast_spell (obj->value[0], obj->level, ch, ch, NULL);

      else
	spell_poison (gsn_poison, obj->level, ch, ch, TARGET_CHAR);
      obj->value[0] = 0;
    }
  trip_triggers(ch, OBJ_TRIG_QUAFFED, obj, NULL, OT_SPEC_NONE);
  WAIT_STATE (ch, 10);
  extract_obj (obj);
  return;
}

void do_recite (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *scroll;
  OBJ_DATA *obj;
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if ((scroll = get_obj_carry (ch, arg1)) == NULL)
    {
      send_to_char ("You do not have that scroll.\n\r", ch);
      return;
    }
  if (scroll->item_type != ITEM_SCROLL && scroll->item_type != ITEM_PARCHMENT)
    {
      send_to_char
	("You can recite only scrolls and scribed parchments.\n\r", ch);
      return;
    }
  // Akamai 6/4/98 -- Bug: #11 Blank parchments should not recite
  // Check the values on the scroll and if they are all empty give
  // a message and bail
  if ((scroll->value[1] <= 0) && (scroll->value[2] <= 0)
      && (scroll->value[3] <= 0))
    {
      if (scroll->item_type == ITEM_SCROLL)
	send_to_char
	  ("You unroll the scroll and see that it is blank.\n\r", ch);

      else
	send_to_char
	  ("You stare in awe of the blankness of this parchment.\n\r", ch);
      return;
    }
  // Minax 6/27/02 -- This prevents people from reciting in rooms
  // that have the SILENT flag
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
  if (is_affected (ch, gsn_mirror))
    {
      act ("The mirror arround you is destroyed by your outgoing magic.",
	   ch, NULL, NULL, TO_CHAR);
      act ("The mirror around $n is destroyed by $s outgoing magic.",
	   ch, NULL, NULL, TO_ROOM);
      affect_strip (ch, gsn_mirror);
    }
  // Akamai 6/4/98 -- Bug: #5 Parchment level restrictions should be the
  // same as the restrictions for brews.
  if (ch->level < (scroll->level - MAGIC_LEVEL_DIFF))
    {
      int mojo;
      mojo = number_range (0, 4);
      if (mojo == 3 || mojo == 1)
	{
	  act ("$n recites $p.", ch, scroll, NULL, TO_ROOM);
	  send_to_char
	    ("You recite the contents of a scroll and a frightening surge rushes through you.\n\r",
	     ch);
	  bad_affect (ch, scroll);
	  extract_obj (scroll);
	  return;
	}
    }
  obj = NULL;
  if (arg2[0] == '\0')
    {
      victim = ch;
    }
  else
    {
      if ((victim = get_char_room (ch, arg2)) == NULL
	  && (obj = get_obj_here (ch, arg2)) == NULL)
	{
	  send_to_char ("You can't find it.\n\r", ch);
	  return;
	}
    }
  act ("$n recites $p on $N.", ch, scroll, victim, TO_ROOM);
  act ("You recite $p on $N.", ch, scroll, victim, TO_CHAR);
  if (number_percent () >= 10 + get_skill (ch, gsn_scrolls) * 9 / 10)
    {
      send_to_char ("You mispronounce a syllable.\n\r", ch);
      check_improve (ch, gsn_scrolls, FALSE, 2);
    }

  else
    {
      if (scroll->value[1] > 0)
	{			/* Make sure scrolls has a spell */
	  if (((IS_CLASS (ch, PC_CLASS_MAGE)) &&	/* Mage spells for mages */
	       (skill_table[scroll->value[1]].type == SKILL_CAST)) || ((IS_CLASS (ch, PC_CLASS_CLERIC)) &&	/* cleric spells for clerics */
      (skill_table[scroll->value[1]].type == SKILL_PRAY))
			  || (IS_CLASS(ch,PC_CLASS_NECROMANCER) && skill_table[scroll->value[1]].type == SKILL_CHANT)  )
	    obj_cast_spell (scroll->value[1], scroll->value[0], ch,
			    victim, obj);

	  else			/* if tests are failing, no spells are cast */
	    send_to_char
	      ("You lack the knowledge to perform the action.\n\r", ch);
	}
      if (scroll->value[2] > 0)
	{
	  if (((IS_CLASS (ch, PC_CLASS_MAGE)) &&
	       (skill_table[scroll->value[2]].type == SKILL_CAST)) ||
	      ((IS_CLASS (ch, PC_CLASS_CLERIC)) &&
	       (skill_table[scroll->value[2]].type == SKILL_PRAY))
	      || (IS_CLASS(ch,PC_CLASS_NECROMANCER) && skill_table[scroll->value[1]].type == SKILL_CHANT))
	    obj_cast_spell (scroll->value[2], scroll->value[0], ch,
			    victim, obj);

	  else
	    send_to_char
	      ("You lack the knowledge to perform the action.\n\r", ch);
	}
      if (scroll->value[3] > 0)
	{
	  if (((IS_CLASS (ch, PC_CLASS_MAGE)) &&
	       (skill_table[scroll->value[3]].type == SKILL_CAST)) ||
	      ((IS_CLASS (ch, PC_CLASS_CLERIC)) &&
	       (skill_table[scroll->value[3]].type == SKILL_PRAY))
	      || (IS_CLASS(ch,PC_CLASS_NECROMANCER) && skill_table[scroll->value[1]].type == SKILL_CHANT))
	    obj_cast_spell (scroll->value[3], scroll->value[0], ch,
			    victim, obj);

	  else
	    send_to_char
	      ("You lack the knowledge to perform the action.\n\r", ch);
	}
      if (scroll->value[4] > 0)
	{
	  if (((IS_CLASS (ch, PC_CLASS_MAGE)) &&
	       (skill_table[scroll->value[4]].type == SKILL_CAST)) ||
	      ((IS_CLASS (ch, PC_CLASS_CLERIC)) &&
	       (skill_table[scroll->value[4]].type == SKILL_PRAY))
	      || (IS_CLASS(ch,PC_CLASS_NECROMANCER) && skill_table[scroll->value[1]].type == SKILL_CHANT))
	    obj_cast_spell (scroll->value[4], scroll->value[0], ch,
			    victim, obj);

	  else
	    send_to_char
	      ("You lack the knowledge to perform the action.\n\r", ch);
	}
      check_improve (ch, gsn_scrolls, TRUE, 2);
    }
  WAIT_STATE (ch, 10);
  extract_obj (scroll);
  return;
}

void do_brandish (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  OBJ_DATA *staff;
  int sn;
  staff = get_eq_char (ch, WEAR_HOLD);

  if (staff == NULL)
    {
      send_to_char ("You hold nothing in your hand.\n\r", ch);
      return;
    }
  if (staff->item_type != ITEM_STAFF)
    {
      send_to_char ("You can brandish only with a staff.\n\r", ch);
      return;
    }
  if ((sn = staff->value[3]) < 0
      || sn >= MAX_SKILL || skill_table[sn].spell_fun == 0)
    {
      bug ("Do_brandish: bad sn %d.", sn);
      return;
    }
  // Iblis 9/13/04 - No Magic flag
  if (IS_SET (ch->in_room->room_flags2, ROOM_NOMAGIC) && !IS_IMMORTAL (ch))
  {
    send_to_char("There are no magical forces to draw power from.\n\r",ch);
    return;
  }
  if (is_affected (ch, gsn_mirror))
    {
      act ("The mirror arround you is destroyed by your outgoing magic.",
	   ch, NULL, NULL, TO_CHAR);
      act ("The mirror around $n is destroyed by $s outgoing magic.",
	   ch, NULL, NULL, TO_ROOM);
      affect_strip (ch, gsn_mirror);
    }
  WAIT_STATE (ch, 2 * PULSE_VIOLENCE);
  if (staff->value[2] > 0)
    {
      act ("$n brandishes $p.", ch, staff, NULL, TO_ROOM);
      act ("You brandish $p.", ch, staff, NULL, TO_CHAR);
      if (ch->level < staff->level
	  || number_percent () >= 10 + get_skill (ch, gsn_staves) * 9 / 10)
	{
	  act ("You fail to invoke $p.", ch, staff, NULL, TO_CHAR);
	  act ("...and nothing happens.", ch, NULL, NULL, TO_ROOM);
	  check_improve (ch, gsn_staves, FALSE, 2);
	}

      else if (skill_table[sn].target == TAR_AREA_OFF)
	{
	  obj_cast_spell (staff->value[3], staff->value[0], ch, NULL, NULL);
	}
      else
	for (vch = ch->in_room->people; vch; vch = vch_next)
	  {
	    vch_next = vch->next_in_room;
	    switch (skill_table[sn].target)
	      {
	      default:
		bug ("Do_brandish: bad target for sn %d.", sn);
		return;
	      case TAR_IGNORE:
		if (vch != ch)
		  continue;
		break;
	      case TAR_CHAR_OFFENSIVE:
		if (IS_NPC (ch) ? IS_NPC (vch) : !IS_NPC (vch))
		  continue;
		break;
	      case TAR_CHAR_DEFENSIVE:
		if (IS_NPC (ch) ? !IS_NPC (vch) : IS_NPC (vch))
		  continue;
		break;
	      case TAR_CHAR_SELF:
		if (vch != ch)
		  continue;
		break;
	      }
	    obj_cast_spell (staff->value[3], staff->value[0], ch, vch, NULL);
	    check_improve (ch, gsn_staves, TRUE, 2);
	  }
    }
  if (--staff->value[2] <= 0)
    {
      act ("$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM);
      act ("Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR);
      extract_obj (staff);
    }
  return;
}

void do_zap (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *wand;
  OBJ_DATA *obj;
  one_argument (argument, arg);
  if (arg[0] == '\0' && ch->fighting == NULL)
    {
      send_to_char ("Zap whom or what?\n\r", ch);
      return;
    }
  if ((wand = get_eq_char (ch, WEAR_HOLD)) == NULL)
    {
      send_to_char ("You hold nothing in your hand.\n\r", ch);
      return;
    }
  if (wand->item_type != ITEM_WAND)
    {
      send_to_char ("You can zap only with a wand.\n\r", ch);
      return;
    }
  // Iblis 9/13/04 - No Magic flag
  if (IS_SET (ch->in_room->room_flags2, ROOM_NOMAGIC) && !IS_IMMORTAL (ch))
  {
    send_to_char("There are no magical forces to draw power from.\n\r",ch);
    return;
  }
  if (is_affected (ch, gsn_mirror))
    {
      act ("The mirror arround you is destroyed by your outgoing magic.",
	   ch, NULL, NULL, TO_CHAR);
      act ("The mirror around $n is destroyed by $s outgoing magic.",
	   ch, NULL, NULL, TO_ROOM);
      affect_strip (ch, gsn_mirror);
    }
  obj = NULL;
/*  if (skill_table[wand->value[3]].target == TAR_STRING)
  {
    obj_cast_spell (wand->value[3], wand->value[0], ch, NULL, NULL,argument);
    return;
  }*/
  if (arg[0] == '\0')
    {
      if (ch->fighting != NULL)
	{
	  victim = ch->fighting;
	}
      else
	{
	  send_to_char ("Zap whom or what?\n\r", ch);
	  return;
	}
    }
  else
    {
      if ((victim = get_char_room (ch, arg)) == NULL
	  && (obj = get_obj_here (ch, arg)) == NULL
	  && skill_table[wand->value[3]].target != TAR_STRING)
	{
	  send_to_char ("You can't find it.\n\r", ch);
	  return;
	}
    }
  WAIT_STATE (ch, 2 * PULSE_VIOLENCE);
  if (wand->value[2] > 0)
    {
      if (victim != NULL)
	{
	  act ("$n zaps $N with $p.", ch, wand, victim, TO_NOTVICT);
	  act ("You zap $N with $p.", ch, wand, victim, TO_CHAR);
	  act ("$n zaps you with $p.", ch, wand, victim, TO_VICT);
	}
      else if (skill_table[wand->value[3]].target == TAR_STRING)
      {
	  act("You zap the air with $p.",ch,wand,NULL,TO_CHAR);
	  act("$n zaps the air with $p.",ch,wand,NULL,TO_ROOM);
      }
      else 
	{
	  act ("$n zaps $P with $p.", ch, wand, obj, TO_ROOM);
	  act ("You zap $P with $p.", ch, wand, obj, TO_CHAR);
	}
      if (ch->level < wand->level
	  || number_percent () >= 10 + get_skill (ch, gsn_wands) * 9 / 10)
	{
	  act ("Your efforts with $p produce only smoke and sparks.",
	       ch, wand, NULL, TO_CHAR);
	  act ("$n's efforts with $p produce only smoke and sparks.",
	       ch, wand, NULL, TO_ROOM);
	  check_improve (ch, gsn_wands, FALSE, 2);
	}
      else
	{
	  obj_cast_spell2 (wand->value[3], wand->value[0], ch, victim, obj,argument);
	  check_improve (ch, gsn_wands, TRUE, 2);
	}
    }
  if (--wand->value[2] <= 0)
    {
      act ("$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM);
      act ("Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR);
      extract_obj (wand);
    }
  return;
}

void steal_store (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *mob, *keeper;
  OBJ_INDEX_DATA *obj_index;
  OBJ_DATA *obj, *shelf;
  int percent, found = FALSE, skl;
  char log_buf[MAX_STRING_LENGTH];
  if (!IS_NPC (ch) && !IS_IMMORTAL (ch) &&
      (clan_table[ch->clan].ctype != CLAN_TYPE_PC) && (!ch->pcdata->loner))
    {
      send_to_char
	("Stealing from stores is PK action, and that you are not.\n\r", ch);
      return;
    }
  if (!IS_SET (ch->in_room->race_flags, ROOM_PLAYERSTORE)
      || ch->in_room->owner == NULL || ch->in_room->owner[0] == '\0')
    {
      send_to_char ("Sorry, you can't do that here.\n\r", ch);
      return;
    }
  if (store_closed (ch->in_room))
    {
      send_to_char
	("Sorry, the store is closed, and the inventory has been safely locked away.\n\r",
	 ch);
      return;
    }
  for (keeper = ch->in_room->people; keeper != NULL; keeper = keeper->next_in_room)
    {
      if (!IS_NPC (keeper))
        continue;
      if (!IS_SET (keeper->recruit_flags, RECRUIT_KEEPER))
        continue;
      if (keeper->recruit_value[0] != ch->in_room->vnum)
        continue;
      break;
    }
   
  if (keeper != NULL)
  {
	  send_to_char("Sorry, the shopkeeper is guarding the store too well.\n\r",ch);
	  return;
  }
		 
		  
  obj_index = get_obj_index (OBJ_VNUM_SHELF);
  if (obj_index == NULL)
    {
      send_to_char ("Sorry, you can't do that here.\n\r", ch);
      return;
    }
  shelf = create_object (obj_index, 0);
  if (shelf == NULL)
    {
      send_to_char ("Sorry, you can't do that here.\n\r", ch);
      return;
    }
  load_shelf (shelf, ch->in_room->vnum);
  if (shelf->contains == NULL)
    {
      send_to_char ("You don't see that here.\n\r", ch);
      extract_obj (shelf);
      return;
    }
  obj = get_obj_list (ch, argument, shelf->contains);
  if (obj == NULL)
    {
      send_to_char ("You don't see that here.\n\r", ch);
      extract_obj (shelf);
      return;
    }
  WAIT_STATE (ch, skill_table[gsn_steal].beats);
  percent = number_percent ();
  skl = get_skill (ch, gsn_steal);
  if (get_char_room (ch, ch->in_room->owner) != NULL)
    found = TRUE;

  else
    for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room)
      {
	if (!IS_NPC (mob))
	  continue;
	if (!IS_SET (mob->recruit_flags, RECRUIT_KEEPER))
	  continue;
	if (mob->recruit_value[0] != ch->in_room->vnum)
	  continue;
	found = TRUE;
	break;
      }
  if (!found && skl > 0)
    skl += 10;
  skl /= 10;
  if (percent > skl)
    {
      send_to_char ("Someone notices you stealing from the store.\n\r", ch);
      act ("$n tried to steal from the store.", ch, NULL, NULL, TO_ROOM);
      check_improve (ch, gsn_steal, FALSE, 2);
      set_wanted (ch);
      sprintf (log_buf,
	       "%s has been set wanted for stealing from the store in room %d.\n\r",
	       ch->name, ch->in_room->vnum);
      log_string (log_buf);
      if (!IS_NPC (ch))
	{
	  if (!ch->pcdata->loner &&
	      (ch->clan == CLAN_BOGUS) && !is_dueling (ch))
	    {
	      ch->pcdata->loner = TRUE;
	      send_to_char
		("For your crimes against humanity, you are now marked a loner.\n\r",
		 ch);
	    }
	}
      extract_obj (shelf);
      return;
    }
  obj_from_obj (obj);
  if (!IS_SET (obj->extra_flags[0], ITEM_PERMSTOCK))
    save_shelf (shelf, ch->in_room->vnum);
  obj_to_char (obj, ch);
  extract_obj (shelf);
  send_to_char ("Got it!\n\r", ch);
  check_improve (ch, gsn_steal, TRUE, 2);
}


// Disallowed stealing from immortals, contemplating dealing damage to player
// when attempting
// Morgan, June 21. 2000
void do_steal (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int percent;
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char ("Steal what from whom?\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "store"))
    {
      steal_store (ch, arg1);
      return;
    }
  if ((victim = get_char_room (ch, arg2)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if ((victim->level > 91) && (!IS_NPC (victim)))
    {
      send_to_char ("Stealing from immortals is not a good idea.\n\r", ch);
      sprintf (buf, "%s just tried stealing from you.\n\r",
	       IS_NPC (ch) ? ch->short_descr : ch->name);
      send_to_char (buf, victim);
      return;
    }
  if (!IS_IMMORTAL (ch) && !IS_NPC (victim))
    if ((clan_table[ch->clan].ctype == CLAN_BOGUS) && (!ch->pcdata->loner))
      {
	send_to_char
	  ("Stealing from players is PK action, and that you are not.\n\r",
	   ch);
	return;
      }
  if (!IS_IMMORTAL (ch) && !IS_NPC (victim))
    if ((clan_table[victim->clan].ctype != CLAN_TYPE_PC) &&
	(!victim->pcdata->loner))
      {
	send_to_char ("Not on that target.\n\r", ch);
	return;
      }
  if (victim == ch)
    {
      send_to_char ("That's pointless.\n\r", ch);
      return;
    }
  if (is_safe (ch, victim))
    return;
  if (!IS_NPC(victim) && (!victim->pcdata->loner && victim->clan == CLAN_BOGUS))
  {
	  send_to_char("No stealing from noclans.  Period.\n\r",ch);
	  return;
  }
  if (victim->position == POS_FIGHTING || victim->fighting != NULL)
    {
      send_to_char ("You'd better not -- you might get hit.\n\r", ch);
      return;
    }
  WAIT_STATE (ch, skill_table[gsn_steal].beats);
  percent = number_percent ();
  if (IS_IMMORTAL (victim))
    percent -= 95;
  if ((percent > (get_skill (ch, gsn_steal) + 1) && IS_AWAKE (victim)) || victim->race == PC_RACE_SWARM)
    {

      /*
       * Failure.
       */
      send_to_char ("Oops.\n\r", ch);
      act ("$n tried to steal from you.", ch, NULL, victim, TO_VICT);
      act ("$n tried to steal from $N.", ch, NULL, victim, TO_NOTVICT);
      switch (number_range (0, 3))
	{
	case 0:
	  sprintf (buf, "%s is a lousy thief!", NAME (ch));
	  break;
	case 1:
	  sprintf (buf, "%s couldn't rob %s way out of a paper bag!",
		   NAME (ch), (ch->sex == 2) ? "her" : "his");
	  break;
	case 2:
	  sprintf (buf, "%s tried to rob me!", NAME (ch));
	  break;
	case 3:
	  sprintf (buf, "Keep your hands out of there, %s!", NAME (ch));
	  break;
	}
      do_yell (victim, buf);
      update_aggressor (ch, victim);
      if (!IS_NPC (ch))
	{
	  if (IS_SET (ch->in_room->room_flags, ROOM_LAW) && !IS_NPC (victim) && victim->Class != PC_CLASS_CHAOS_JESTER)
	  {
		  
	    set_wanted (ch);
	    sprintf (log_buf,
		   "%s has been set wanted for stealing from %s.\n\r",
		   ch->name, victim->name);
	    log_string (log_buf);
	  }
	  if (!IS_NPC (victim) && !ch->pcdata->loner &&
	      (ch->clan == CLAN_BOGUS) && !is_dueling (ch))
	    {
	      ch->pcdata->loner = TRUE;
	      send_to_char
		("For your crimes against humanity, you are now marked a loner.\n\r",
		 ch);
	    }
	  if (IS_NPC (victim))
	    {
	      check_improve (ch, gsn_steal, FALSE, 2);
	      multi_hit (victim, ch, TYPE_UNDEFINED);
	    }
	  else
	    {
	      sprintf (buf, "$N tried to steal from %s.", victim->name);
	      wiznet (buf, ch, NULL, WIZ_FLAGS, 0, 0);
	    }
	}
      return;
    }
  if (!str_cmp (arg1, "coin")
      || !str_cmp (arg1, "coins")
      || !str_cmp (arg1, "gold") || !str_cmp (arg1, "silver"))
    {
      int gold, silver;
      gold = victim->gold * number_range (1, ch->level) / 100;
      silver = victim->silver * number_range (1, ch->level) / 100;
      if (gold <= 0)
	gold = 0;
      if (silver <= 0)
	silver = 0;
      if (gold <= 0 && silver <= 0)
	{
	  send_to_char ("You couldn't get any coins.\n\r", ch);
	  return;
	}
      ch->gold += gold;
      ch->silver += silver;
      victim->silver -= silver;
      victim->gold -= gold;
      if (silver <= 0)
	sprintf (buf, "Bingo!  You got %d gold coins.\n\r", gold);

      else if (gold <= 0)
	sprintf (buf, "Bingo!  You got %d silver coins.\n\r", silver);

      else
	sprintf (buf,
		 "Bingo!  You got %d silver and %d gold coins.\n\r",
		 silver, gold);
      send_to_char (buf, ch);
      if (silver <= 0)
        sprintf (buf, "$n just stole %d gold coins from $N.\n\r", gold);
      else if (gold <= 0)
        sprintf (buf, "$n just stole %d silver coins from $N.\n\r", silver);
      else
        sprintf (buf, "$n just stole %d silver and %d gold coins from $N.\n\r", silver, gold);
      act(buf,ch,NULL,victim,TO_IIR);
      check_improve (ch, gsn_steal, TRUE, 2);
      update_aggressor2 (ch, victim,TRUE);
      //Iblis - 3/11/04 - Antiduping Code
      if (!IS_NPC(ch))
        save_char_obj (ch);
      if (!IS_NPC(victim))
        save_char_obj (victim);
      return;
    }
  if ((obj = get_obj_carry (victim, arg1)) == NULL)
    {
      send_to_char ("You can't find it.\n\r", ch);
      return;
    }
  if ( (!can_drop_obj (victim, obj) && (!IS_IMMORTAL(ch) /*|| IS_NPC(ch)*/) )
      || IS_SET (obj->extra_flags[0], ITEM_INVENTORY) || obj->level > ch->level)
    {
      send_to_char ("You can't pry it away.\n\r", ch);
      return;
    }

   if (IS_SET(obj->extra_flags[0],ITEM_NOMOB) && IS_NPC(ch))
    {
      send_to_char ("You can't take that.\n\r",ch);
      return;
    }
    
  
  if (obj->plr_owner != NULL)
	  if (str_cmp (ch->name, obj->plr_owner) && !IS_IMMORTAL(ch))
	  {
		  sprintf (buf, "You do not own %s!\n\r", obj->short_descr);
		  send_to_char (buf, ch);
		  return;
	  }
  if (ch->carry_number + get_obj_number (obj) > can_carry_n (ch))
    {
      send_to_char ("You have your hands full.\n\r", ch);
      return;
    }
  if (ch->carry_weight + get_obj_weight (obj) > can_carry_w (ch))
    {
      send_to_char ("You can't carry that much weight.\n\r", ch);
      return;
    }
  obj_from_char (obj);
  obj_to_char (obj, ch);
  check_improve (ch, gsn_steal, TRUE, 2);
  send_to_char ("Got it!\n\r", ch);
  act ("$n just stole $p from $N.",ch,obj,victim,TO_IIR);
  update_aggressor2 (ch, victim,TRUE);
  //Iblis - 3/11/04 - Antiduping Code
  if (!IS_NPC(ch))
    save_char_obj (ch);
  if (!IS_NPC(victim))
    save_char_obj (victim);
  return;
}

void do_palm (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int percent;
  argument = one_argument (argument, arg1);
  if (arg1[0] == '\0')
    {
      send_to_char ("Palm what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_list (ch, arg1, ch->in_room->contents)) == NULL)
    {
      send_to_char ("You don't see that here.\n\r", ch);
      return;
    }
  percent = number_percent ();
  if (((!IS_NPC (ch) && percent > get_skill (ch, gsn_palm))))
    {
      get_obj (ch, obj, NULL);
      //Iblis - 3/11/04 - Antiduping Code
      if (!IS_NPC(ch))
        save_char_obj (ch);
      check_improve (ch, gsn_palm, FALSE, 2);
    }
  else
    {
      palm_obj (ch, obj);
      //Iblis - 3/11/04 - Antiduping Code
      if (!IS_NPC(ch))
        save_char_obj (ch);
      check_improve (ch, gsn_palm, TRUE, 2);
    }
}
void do_plant (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim = NULL;
  int percent;
  argument = one_argument (argument, arg1);
  one_argument (argument, arg2);
  if (arg1[0] == '\0')
    {
      send_to_char ("Plant what?\n\r", ch);
      return;
    }
  percent = number_percent ();
  if (((!IS_NPC (ch) && percent > get_skill (ch, gsn_palm))))
    {
      if (arg2[0] != '\0')
	{
	  sprintf (buf, "%s %s", arg1, arg2);
	  do_give (ch, buf);
	}
      else
	{
	  do_drop (ch, arg1);
	}
      check_improve (ch, gsn_palm, FALSE, 2);
    }
  else
    {
      if (arg2[0] != '\0')
	{
	  if ((victim = get_char_room (ch, arg2)) == NULL)
	    {
	      send_to_char ("They aren't here.\n\r", ch);
	      return;
	    }
	}
      plant_obj (ch, arg1, victim);
      //Iblis - 3/11/04 - Antiduping Code
      if (!IS_NPC(ch))
        save_char_obj (ch);
      if (victim && !IS_NPC(victim))
        save_char_obj (victim);
      check_improve (ch, gsn_palm, TRUE, 2);
    }
}
void adjust_portal_weight (OBJ_DATA * obj)
{

  //5-12-03 Iblis - Iverath whined about the portal weight being screwym so 
  //for now the function will be voided out
}
void adjust_elevator_weight (OBJ_DATA * obj)
{
  int new_weight;
  CHAR_DATA *vch;
  ROOM_INDEX_DATA *location;
  OBJ_DATA *vobj;
  location = get_room_index (obj->value[0]);
  if (location == NULL)
    return;
  new_weight = 0;
  for (vch = location->people; vch != NULL; vch = vch->next_in_room)
    {
      new_weight += vch->carry_weight;
      switch (vch->size)
	{
	case SIZE_TINY:
	  new_weight += 500;
	  break;
	case SIZE_SMALL:
	  new_weight += 800;
	  break;
	case SIZE_MEDIUM:
	  new_weight += 1400;
	  break;
	case SIZE_LARGE:
	  new_weight += 1750;
	  break;
	case SIZE_HUGE:
	  new_weight += 2500;
	  break;
	case SIZE_GIANT:
	  new_weight += 4000;
	  break;
	}
    }
  for (vobj = location->contents; vobj != NULL; vobj = vobj->next_content)
    new_weight += vobj->weight;
  obj->weight = new_weight;
}


/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper (CHAR_DATA * ch)
{

  CHAR_DATA *keeper;
  SHOP_DATA *pShop;
  pShop = NULL;
  for (keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room)
    {
      if (IS_NPC (keeper) && (pShop = keeper->pIndexData->pShop) != NULL)
	break;
    }
  if (pShop == NULL)
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return NULL;
    }

  /*
   * Shop hours.
   */
  if (time_info.hour < pShop->open_hour && !IS_IMMORTAL (ch))
    {
      do_say (keeper, "Sorry, I am closed. Come back later.");
      return NULL;
    }
  if (time_info.hour > pShop->close_hour && !IS_IMMORTAL (ch))
    {
      do_say (keeper, "Sorry, I am closed. Come back tomorrow.");
      return NULL;
    }

  /*
   * Invisible or hidden people.
   */
  if (!can_see (keeper, ch) && !IS_IMMORTAL (ch))
    {
      do_say (keeper, "I don't trade with folks I can't see.");
      return NULL;
    }
  return keeper;
}


/* insert an object at the right spot for the keeper */
void obj_to_keeper (OBJ_DATA * obj, CHAR_DATA * ch)
{
  OBJ_DATA *t_obj, *t_obj_next;

  /* see if any duplicates are found */
  for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next)
    {
      t_obj_next = t_obj->next_content;
      if (obj->pIndexData == t_obj->pIndexData
	  && !str_cmp (obj->short_descr, t_obj->short_descr))
	{

	  /* if this is an unlimited item, destroy the new one */
	  if (IS_OBJ_STAT (t_obj, ITEM_INVENTORY))
	    {
	      extract_obj (obj);
	      return;
	    }
	  obj->cost = t_obj->cost;	/* keep it standard */
	  break;
	}
    }
  if (t_obj == NULL)
    {
      obj->next_content = ch->carrying;
      ch->carrying = obj;
    }
  else
    {
      obj->next_content = t_obj->next_content;
      t_obj->next_content = obj;
    }
  obj->carried_by = ch;
  obj->in_room = NULL;
  obj->in_obj = NULL;
  ch->carry_number += get_obj_number (obj);
  ch->carry_weight += get_obj_weight (obj);
}


/* get an object from a shopkeeper's list */
OBJ_DATA *get_obj_keeper (CHAR_DATA * ch, CHAR_DATA * keeper, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;
  number = number_argument (argument, arg);
  count = 0;
  for (obj = keeper->carrying; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc == WEAR_NONE && can_see_obj (keeper, obj)
	  && can_see_obj (ch, obj) && is_name (arg, obj->name))
	{
	  if (++count == number)
	    return obj;

	  /* skip other objects of the same name */
	  while (obj->next_content != NULL
		 && obj->pIndexData == obj->next_content->pIndexData
		 && !str_cmp (obj->short_descr,
			      obj->next_content->short_descr))
	    obj = obj->next_content;
	}
    }
  return NULL;
}

int get_cost (CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy)
{
  SHOP_DATA *pShop;
  int cost;
  if (obj == NULL || (pShop = keeper->pIndexData->pShop) == NULL)
    return 0;
  if (fBuy)
    {
      cost = obj->cost * pShop->profit_buy / 100;
    }
  else
    {
      OBJ_DATA *obj2;
      int itype;
      cost = 0;
      for (itype = 0; itype < MAX_TRADE; itype++)
	{
	  if (obj->item_type == pShop->buy_type[itype])
	    {
	      cost = obj->cost * pShop->profit_sell / 100;
	      break;
	    }
	}
      if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT))
	for (obj2 = keeper->carrying; obj2; obj2 = obj2->next_content)
	  {
	    if (obj->pIndexData == obj2->pIndexData
		&& !str_cmp (obj->short_descr, obj2->short_descr))
	      {
		if (IS_OBJ_STAT (obj2, ITEM_INVENTORY))
		  cost /= 2;

		else
		  cost = cost * 3 / 4;
	      }
	  }
    }
  if (obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND)
    {
      if (obj->value[1] == 0)
	cost /= 4;

      else
	cost = cost * obj->value[2] / obj->value[1];
    }
  return cost;
}

ROOM_INDEX_DATA *find_pier (AREA_DATA * pArea)
{
  ROOM_INDEX_DATA *pRoom, *rRoom = NULL;
  int room_vnum;
  for (room_vnum = pArea->min_vnum; room_vnum < pArea->max_vnum; room_vnum++)
    {
      if ((pRoom = get_room_index (room_vnum)) == NULL)
	continue;
      if (IS_SET (pRoom->room_flags, ROOM_PIER))
	{
	  bool has_trans = FALSE;
	  OBJ_DATA *obj;
	  rRoom = pRoom;
	  for (obj = pRoom->contents; obj; obj = obj->next_content)
	    if (obj->item_type == ITEM_CTRANSPORT)
	      {
		has_trans = TRUE;
		break;
	      }
	  if (!has_trans)
	    return (pRoom);
	}
    }
  return (rRoom);
}

void do_buy (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int cost, roll, origcost, available;
  if (argument[0] == '\0')
    {
      send_to_char ("Buy what?\n\r", ch);
      return;
    }
  if (IS_SET (ch->in_room->room_flags, ROOM_STABLE))
    {
      char arg[MAX_INPUT_LENGTH];
      char buf[MAX_STRING_LENGTH];
      CHAR_DATA *mount;
      ROOM_INDEX_DATA *pRoomIndexNext;
      ROOM_INDEX_DATA *in_room;

      if (IS_NPC (ch))
	return;
      argument = one_argument (argument, arg);
      pRoomIndexNext = get_room_index (ch->in_room->vnum + 1);
      if (pRoomIndexNext == NULL)
	{
	  bug ("Do_buy: bad stable at vnum %d.", ch->in_room->vnum);
	  send_to_char ("Sorry, you can't buy that here.\n\r", ch);
	  return;
	}
      in_room = ch->in_room;
      ch->in_room = pRoomIndexNext;
      mount = get_char_room (ch, arg);
      ch->in_room = in_room;
      if (mount == NULL || !IS_SET (mount->act, ACT_MOUNT))
	{
	  send_to_char ("Sorry, you can't buy that here.\n\r", ch);
	  return;
	}
      origcost = cost = 10 * mount->level * mount->level;
      if (((ch->silver + 100 * ch->gold) < cost) & !IS_IMMORTAL (ch))
	{
	  send_to_char ("You can't afford it.\n\r", ch);
	  return;
	}
      if (ch->level < mount->level)
	{
	  send_to_char
	    ("You're not powerful enough to master this mount.\n\r", ch);
	  return;
	}
      mount = create_mobile (mount->pIndexData);
      SET_BIT (mount->act, ACT_MOUNT);
      SET_BIT (mount->act, ACT_TAME);
      mount->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;
      argument = one_argument (argument, arg);
      if (arg[0] != '\0')
	{
	  char strsave[MAX_STRING_LENGTH];
	  struct stat statBuf;
	  sprintf (strsave, "%s%c/%s", PLAYER_DIR, toupper (arg[0]),
		   capitalize (arg));
	  if (!check_parse_name (arg) || stat (strsave, &statBuf) != -1)
	    {
	      send_to_char
		("Sorry, that name cannot be used for your mount.\n\r", ch);

	      //Iblis 07/05/03 - To deal with a SIGSEGV after trying to buy a pet with a bad
	      //  name.  No clue why it worked for so many years without this, or exactly
	      //  why there is a SIGSEGV.  Since Adeon changed some building things, I can't boot
	      //  an older version of Exodus, so I can't test exactly what happenned before.
	      char_to_room (mount, ch->in_room);
	      extract_char (mount, TRUE);
	      return;
	    }
	  sprintf (buf, "%s %s", mount->name, arg);
	  free_string (mount->name);
	  mount->name = str_dup (buf);
	}

      /* haggle */
      roll = number_percent ();
      if (roll < get_skill (ch, gsn_haggle))
	{
	  cost -= cost / 2 * roll / 100;
	  if (cost < origcost / 4)
	    cost = (origcost / 4);
	  sprintf (buf, "You haggle the price down to %d coins.\n\r", cost);
	  send_to_char (buf, ch);
	  check_improve (ch, gsn_haggle, TRUE, 4);
	}
      roll = number_percent ();
      if (roll < get_skill (ch, gsn_influence))
	{
	  CHAR_DATA *keeper;
	  if ((keeper = find_keeper (ch)) == NULL)
	    return;
	  cost -= cost / 2 * roll / 100;
	  if (cost < origcost / 4)
	    cost = (origcost / 4);
	  sprintf (buf,
		   "You influence %s to sell you the mount for %d coins.\n\r",
		   keeper->short_descr, cost);
	  send_to_char (buf, ch);
	  check_improve (ch, gsn_influence, TRUE, 4);
	}
      deduct_cost (ch, cost);
      char_to_room (mount, ch->in_room);
      mount->leader = ch;
      mount->mounter = ch;
      send_to_char ("Enjoy your mount.\n\r", ch);
      act ("$n bought $N as a mount.", ch, NULL, mount, TO_ROOM);
      return;
    }
  else if (IS_SET (ch->in_room->race_flags, ROOM_PLAYERSTORE)
	   && ch->in_room->owner != NULL && ch->in_room->owner[0] != '\0')
    {
      OBJ_DATA *shelf, *obj, *t_obj, *obj2;
      char arg[MAX_INPUT_LENGTH];
      OBJ_INDEX_DATA *obj_index;
      int number, count = 1;
      CHAR_DATA *keeper;
      if (store_closed (ch->in_room) && !IS_IMMORTAL (ch))
	{
	  send_to_char ("Sorry, this store is currently closed.\n\r", ch);
	  return;
	}
      for (keeper = ch->in_room->people; keeper != NULL;
	   keeper = keeper->next_in_room)
	{
	  if (!IS_NPC (keeper))
	    continue;
	  if (!IS_SET (keeper->recruit_flags, RECRUIT_KEEPER))
	    continue;
	  if (keeper->recruit_value[0] != ch->in_room->vnum)
	    continue;
	  break;
	}
      if (keeper == NULL)
	{
	  keeper = get_char_room_nomask (ch, ch->in_room->owner);
	  if (keeper != NULL)
	  {
	    if (strcmp (keeper->name, ch->in_room->owner))
	    {
	      keeper = NULL;
	    }
	  }
	}
      if (keeper == NULL && !IS_IMMORTAL (ch))
	{
	  send_to_char
	    ("Sorry, the store keeper seems to have stepped out.  Try again later.\n\r",
	     ch);
	  return;
	}
      obj_index = get_obj_index (OBJ_VNUM_SHELF);
      if (obj_index == NULL && !IS_IMMORTAL (ch))
	{
	  send_to_char
	    ("Sorry, the store keeper seems to have stepped out.  Try again later.\n\r",
	     ch);
	  return;
	}
      shelf = create_object (obj_index, 0);
      if (shelf == NULL && !IS_IMMORTAL (ch))
	{
	  send_to_char
	    ("Sorry, the store keeper seems to have stepped out.  Try again later.\n\r",
	     ch);
	  return;
	}
      load_shelf (shelf, ch->in_room->vnum);
      if (shelf->contains == NULL)
	{
	  send_to_char ("Sorry, this store is out of inventory.\n\r", ch);
	  extract_obj (shelf);
	  return;
	}
      number = mult_argument (argument, arg);
      obj = get_obj_list (ch, arg, shelf->contains);
      if (obj == NULL)
	{
	  send_to_char ("Sorry, you can't buy that here.\n\r", ch);
	  extract_obj (shelf);
	  return;
	}
      cost = obj->cost;
      available = 0;
      if ((cost <= 0 || !can_see_obj (ch, obj)) && !IS_IMMORTAL (ch))
	{
	  send_to_char ("Sorry, you can't buy that here.\n\r", ch);
	  extract_obj (shelf);
	  return;
	}
      if (obj->level - 10 > ch->level)
	{
	  act ("Sorry, you can't use $p yet.", ch, obj, NULL, TO_CHAR);
	  extract_obj (shelf);
	  return;
	}
      if (((ch->silver + ch->gold * 100) < cost * number)
	  && !IS_IMMORTAL (ch))
	{
	  if (number > 1)
	    send_to_char
	      ("Sorry, you can't afford to buy that many.\n\r", ch);

	  else
	    act ("Sorry, you can't afford to buy $p.", ch, obj, NULL,
		 TO_CHAR);
	  extract_obj (shelf);
	  return;
	}
      for (obj2 = shelf->contains; obj2 != NULL; obj2 = obj2->next_content)
	{
	  if (IS_SET (obj2->extra_flags[0], ITEM_PERMSTOCK))
	    {
	      available = 1000;
	      break;
	    }
	  if (obj2->pIndexData == obj->pIndexData)
	    available++;
	}
      if (number < 1 || number > available)
	{
	  send_to_char ("Sorry, you can't buy that many.\n\r", ch);
	  extract_obj (shelf);
	  return;
	}
      if (ch->carry_number + number * get_obj_number (obj) > can_carry_n (ch))
	{
	  send_to_char ("You can't carry that many items.\n\r", ch);
	  extract_obj (shelf);
	  return;
	}
      if (obj->item_type != ITEM_CTRANSPORT)
	if (ch->carry_weight + number * get_obj_weight (obj) >
	    can_carry_w (ch))
	  {
	    send_to_char ("You can't carry that much weight.\n\r", ch);
	    extract_obj (shelf);
	    return;
	  }
      if (number > 1)
	{
	  sprintf (buf, "$n buys $p[%d].", number);
	  act (buf, ch, obj, NULL, TO_ROOM);
	  sprintf (buf, "You buy $p[%d] for %d silver.", number,
		   cost * number);
	  act (buf, ch, obj, NULL, TO_CHAR);
	}
      else
	{
	  act ("$n buys $p.", ch, obj, NULL, TO_ROOM);
	  sprintf (buf, "You buy $p for %d silver.", cost);
	  act (buf, ch, obj, NULL, TO_CHAR);
	}
      deduct_cost (ch, cost * number);
      if (!IS_IMMORTAL (ch))
	{
	  if (!IS_NPC (keeper))
	    {
	      keeper->gold += cost * number / 100;
	      keeper->silver += cost * number - (cost * number / 100) * 100;
	      if (number > 1)
  	        sprintf(buf,"%s buys %d %s from %s for %d at %d.",ch->name, number, obj->short_descr,keeper->name,cost*number,ch->in_room->vnum);
	      else sprintf(buf,"%s buys %s from %s for %d at %d.",ch->name,obj->short_descr,keeper->name,cost*number,ch->in_room->vnum);
   	      wiznet (buf, ch, NULL, WIZ_STOCKBUY, 0, 0);
	    }
	  else
	    {
	      CHAR_DATA *owner;
	      int total=cost*number;
	      if (number > 1)
  	        sprintf(buf,"%s buys %d %s from %s's store for %d at %d.",ch->name,number,obj->short_descr,ch->in_room->owner,total,ch->in_room->vnum);
	      else sprintf(buf,"%s buys %s from %s's store for %d at %d.",ch->name,obj->short_descr,ch->in_room->owner,total,ch->in_room->vnum);
	      wiznet (buf, ch, NULL, WIZ_STOCKBUY, 0, 0);
		  
	      sprintf(buf,"oocnote to %s",ch->in_room->owner);
	      interpret(keeper,buf);
	      interpret(keeper,"oocnote subject `dItems sold``");
		      total *= .95;
		      sprintf(buf,"oocnote + Buyer - %s, Item - %s, Money Made (after commission) - %d",ch->name,obj->short_descr,total);
	      interpret(keeper,buf);
	      if (number > 1)
	      {
	        sprintf(buf,"oocnote + ``    Number Sold - %d",number);
	        interpret(keeper,buf);
	      }
	      if ((owner = get_char_world2(ch, ch->in_room->owner)) != NULL)
		{
		  if (!IS_NPC (owner) && !IS_IMMORTAL (ch))
		    {
		      owner->pcdata->bank_gold += total / 100;
		      owner->pcdata->bank_silver += total -
			(total / 100) * 100;
		      owner->pcdata->total_objs_sold += number;
		      owner->pcdata->total_money_made += total/100;
		      if (owner->pcdata->total_money_made > 2000000000)
		       owner->pcdata->total_money_made = 2000000000;
		      
		    }
		}
	      else
		{
		  if (!IS_IMMORTAL (ch))
		    {
		      owner = load_char_obj2 (ch->in_room->owner);
		      if (IS_NPC(owner))
		      {
			      bug("Owner not an npc in %d",ch->in_room->vnum);
		      }
		      else
		      {
		      owner->pcdata->bank_gold += total / 100;
		      owner->pcdata->bank_silver += total -
			(total / 100) * 100;
		      owner->pcdata->total_objs_sold += number;
                      owner->pcdata->total_money_made += total/100;
		      if (owner->pcdata->total_money_made > 2000000000)
  	  	        owner->pcdata->total_money_made = 2000000000;
		      save_char_obj (owner);
		      is_linkloading = TRUE;
		      ppurge(ch,owner);
		      is_linkloading = FALSE;
		      }
		    }
		}
	    }
	}
      for (count = 0; count < number; count++)
	{
	  if (!IS_SET (obj->extra_flags[0], ITEM_PERMSTOCK))
	    {
	      t_obj = get_obj_list (ch, arg, shelf->contains);
	      if (IS_SET (t_obj->extra_flags[0], ITEM_DONATE))
		t_obj->cost = 0;

	      else
		t_obj->cost = t_obj->pIndexData->cost;
	      obj_from_obj (t_obj);
	    }
	  else
	    t_obj = create_object (obj->pIndexData, obj->level);
	  if (t_obj->timer > 0 && !IS_OBJ_STAT (t_obj, ITEM_HAD_TIMER))
	    t_obj->timer = 0;
	  REMOVE_BIT (t_obj->extra_flags[0], ITEM_HAD_TIMER);
	  obj_to_char (t_obj, ch);
	  if (cost < t_obj->cost)
	    t_obj->cost = cost;
	}
      save_shelf (shelf, ch->in_room->vnum);
      extract_obj (shelf);
      return;
    }
  else if (IS_SET (ch->in_room->room_flags, ROOM_PET_SHOP))
    {
      char arg[MAX_INPUT_LENGTH];
      char buf[MAX_STRING_LENGTH];
      CHAR_DATA *pet;
      ROOM_INDEX_DATA *pRoomIndexNext;
      ROOM_INDEX_DATA *in_room;

      if (IS_NPC (ch))
	return;
      argument = one_argument (argument, arg);

      /* hack to make new thalos pets work */
      if (ch->in_room->vnum == 9621)
	pRoomIndexNext = get_room_index (9706);

      else
	pRoomIndexNext = get_room_index (ch->in_room->vnum + 1);
      if (pRoomIndexNext == NULL)
	{
	  bug ("Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum);
	  send_to_char ("Sorry, you can't buy that here.\n\r", ch);
	  return;
	}
      in_room = ch->in_room;
      ch->in_room = pRoomIndexNext;
      pet = get_char_room (ch, arg);
      ch->in_room = in_room;
      if (pet == NULL || !IS_SET (pet->act, ACT_PET))
	{
	  send_to_char ("Sorry, you can't buy that here.\n\r", ch);
	  return;
	}
      if (ch->pet != NULL)
	{
	  send_to_char ("You already own a pet.\n\r", ch);
	  return;
	}
      origcost = cost = 10 * pet->level * pet->level;
      if ((ch->silver + 100 * ch->gold) < cost)
	{
	  send_to_char ("You can't afford it.\n\r", ch);
	  return;
	}
      if (ch->level < pet->level)
	{
	  send_to_char
	    ("You're not powerful enough to master this pet.\n\r", ch);
	  return;
	}
      pet = create_mobile (pet->pIndexData);
      SET_BIT (pet->act, ACT_PET);
      SET_BIT (pet->affected_by, AFF_CHARM);
      pet->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;
      argument = one_argument (argument, arg);
      if (arg[0] != '\0')
	{
	  char strsave[MAX_STRING_LENGTH];
	  struct stat statBuf;
	  sprintf (strsave, "%s%c/%s", PLAYER_DIR, toupper (arg[0]),
		   capitalize (arg));
	  if (!check_parse_name (arg) || stat (strsave, &statBuf) != -1)
	    {
	      send_to_char
		("Sorry, that name cannot be used for your pet.\n\r", ch);

	      //Iblis 07/05/03 - To deal with a SIGSEGV after trying to buy a pet with a bad
	      //  name.  No clue why it worked for so many years without this, or exactly
	      //  why there is a SIGSEGV.  Since Adeon changed some building things, I can't boot
	      //  an older version of Exodus, so I can't test exactly what happenned before.
	      //pet->in_room = ch->in_room;
	      char_to_room (pet, ch->in_room);
	      extract_char (pet, TRUE);
	      return;
	    }
	  sprintf (buf, "%s %s", pet->name, arg);
	  free_string (pet->name);
	  pet->name = str_dup (buf);
	}

      /* haggle */
      roll = number_percent ();
      if (roll < get_skill (ch, gsn_haggle))
	{
	  cost -= cost / 2 * roll / 100;
	  if (cost < origcost / 4)
	    cost = (origcost / 4);
	  sprintf (buf, "You haggle the price down to %d coins.\n\r", cost);
	  send_to_char (buf, ch);
	  check_improve (ch, gsn_haggle, TRUE, 4);
	}
      roll = number_percent ();
      if (roll < get_skill (ch, gsn_influence))
	{
	  CHAR_DATA *keeper;
	  if ((keeper = find_keeper (ch)) == NULL)
	    return;
	  cost -= cost / 2 * roll / 100;
	  if (cost < origcost / 4)
	    cost = (origcost / 4);
	  sprintf (buf,
		   "You influence %s to sell you the pet for %d coins.\n\r",
		   keeper->short_descr, cost);
	  send_to_char (buf, ch);
	  check_improve (ch, gsn_influence, TRUE, 4);
	}
      deduct_cost (ch, cost);
      sprintf (buf, "%sA neck tag says 'I belong to %s'.\n\r",
	       pet->description, ch->name);
      free_string (pet->description);
      pet->description = str_dup (buf);
      char_to_room (pet, ch->in_room);
      add_follower (pet, ch);
      pet->leader = ch;
      ch->pet = pet;
      send_to_char ("Enjoy your pet.\n\r", ch);
      act ("$n bought $N as a pet.", ch, NULL, pet, TO_ROOM);
      return;
    }
  else
    {
      CHAR_DATA *keeper;
      OBJ_DATA *obj, *t_obj;
      char arg[MAX_INPUT_LENGTH];
      int number, count = 1;
      if ((keeper = find_keeper (ch)) == NULL)
	return;
      number = mult_argument (argument, arg);
      obj = get_obj_keeper (ch, keeper, arg);
      origcost = cost = get_cost (keeper, obj, TRUE);
      if (number < 1)
	{
	  act ("$n tells you 'You can't buy that many.", keeper, NULL,
	       ch, TO_VICT);
	  return;
	}
      if (cost <= 0 || !can_see_obj (ch, obj))
	{
	  act ("$n tells you 'I don't sell that -- try 'list''.",
	       keeper, NULL, ch, TO_VICT);
	  ch->reply = keeper;
	  return;
	}
      if (!IS_OBJ_STAT (obj, ITEM_INVENTORY))
	{
	  for (t_obj = obj->next_content;
	       count < number && t_obj != NULL; t_obj = t_obj->next_content)
	    {
	      if (t_obj->pIndexData ==
		  obj->pIndexData && !str_cmp (t_obj->short_descr,
					       obj->short_descr))
		count++;

	      else
		break;
	    }
	  if (count < number)
	    {
	      act ("$n tells you 'I don't have that many in stock.",
		   keeper, NULL, ch, TO_VICT);
	      ch->reply = keeper;
	      return;
	    }
	}
      if (((ch->silver + ch->gold * 100) < cost * number)
	  && !IS_IMMORTAL (ch))
	{
	  if (number > 1)
	    act ("$n tells you 'You can't afford to buy that many.",
		 keeper, obj, ch, TO_VICT);

	  else
	    act ("$n tells you 'You can't afford to buy $p'.", keeper,
		 obj, ch, TO_VICT);
	  ch->reply = keeper;
	  return;
	}
      if (obj->level - 20 > ch->level)
	{
	  act ("$n tells you 'You can't use $p yet'.", keeper, obj, ch,
	       TO_VICT);
	  ch->reply = keeper;
	  return;
	}
      if (ch->carry_number + number * get_obj_number (obj) > can_carry_n (ch))
	{
	  send_to_char ("You can't carry that many items.\n\r", ch);
	  return;
	}
      if (obj->item_type != ITEM_CTRANSPORT)
	if (ch->carry_weight + number * get_obj_weight (obj) >
	    can_carry_w (ch))
	  {
	    send_to_char ("You can't carry that much weight.\n\r", ch);
	    return;
	  }

      /* haggle */
      roll = number_percent ();
      if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT)
	  && roll < get_skill (ch, gsn_haggle))
	{
	  cost -= obj->cost / 2 * roll / 100;
	  if (cost < origcost / 4)
	    cost = (origcost / 4);
	  act ("You haggle with $N.", ch, NULL, keeper, TO_CHAR);
	  check_improve (ch, gsn_haggle, TRUE, 4);
	}
      roll = number_percent ();
      if (roll < get_skill (ch, gsn_influence))
	{
	  cost -= obj->cost / 2 * roll / 100;
	  if (cost < origcost / 4)
	    cost = (origcost / 4);
	  sprintf (buf,
		   "You influence %s to sell you %s for %d coins.\n\r",
		   keeper->short_descr, obj->short_descr, cost);
	  send_to_char (buf, ch);
	  check_improve (ch, gsn_influence, TRUE, 4);
	}
      if (number > 1)
	{
	  sprintf (buf, "$n buys $p[%d].", number);
	  act (buf, ch, obj, NULL, TO_ROOM);
	  sprintf (buf, "You buy $p[%d] for %d silver.", number,
		   cost * number);
	  act (buf, ch, obj, NULL, TO_CHAR);
	}
      else
	{
	  act ("$n buys $p.", ch, obj, NULL, TO_ROOM);
	  sprintf (buf, "You buy $p for %d silver.", cost);
	  act (buf, ch, obj, NULL, TO_CHAR);
	}
      if (obj->item_type != ITEM_CTRANSPORT)
	{
	  deduct_cost (ch, cost * number);
	  keeper->gold += cost * number / 100;
	  keeper->silver += cost * number - (cost * number / 100) * 100;
	  for (count = 0; count < number; count++)
	    {
	      if (IS_SET (obj->extra_flags[0], ITEM_INVENTORY))
		t_obj = create_object (obj->pIndexData, obj->level);

	      else
		{
		  t_obj = obj;
		  obj = obj->next_content;
		  obj_from_char (t_obj);
		}
	      if (t_obj->timer > 0 && !IS_OBJ_STAT (t_obj, ITEM_HAD_TIMER))
		t_obj->timer = 0;
	      REMOVE_BIT (t_obj->extra_flags[0], ITEM_HAD_TIMER);
	      obj_to_char (t_obj, ch);
	      if (cost < t_obj->cost)
		t_obj->cost = cost;
	    }
	}
      else
	{
	  char tcbuf[MAX_STRING_LENGTH];
	  ROOM_INDEX_DATA *location;
	  OBJ_DATA *map_obj;
	  location = find_pier (ch->in_room->area);
	  if (location == NULL)
	    {
	      act ("$n tells you 'Sorry, but no piers exist to hold $p'.",
		   keeper, obj, ch, TO_VICT);
	      ch->reply = keeper;
	      return;
	    }
	  obj_from_char (obj);
	  obj_to_room (obj, location);
	  sprintf (tcbuf, "$n tells you 'You can board $p at %s'.",
		   location->name);
	  act (tcbuf, keeper, obj, ch, TO_VICT);
	  ch->reply = keeper;
	  if (obj->value[6] > 0)
	    {
	      int vin;
	      char vbuf[16];
	      OBJ_DATA *key_obj;
	      OBJ_INDEX_DATA *key_obj_i;
	      vin = number_range (16384, 1048560);
	      sprintf (vbuf, "%d", vin);
	      obj->owner = str_dup (vbuf);
	      key_obj_i = get_obj_index (obj->value[6]);
	      if (key_obj_i != NULL)
		{
		  key_obj = create_object (get_obj_index (obj->value[6]), 0);
		  if (key_obj != NULL)
		    {
		      key_obj->owner = str_dup (vbuf);
		      act ("$n hands you $p.", keeper, key_obj, ch, TO_VICT);
		      obj_to_char (key_obj, ch);
		    }
		}
	    }
	  if ((map_obj =
	       create_map (ch, ch->in_room->vnum, location->vnum)) != NULL)
	    {
	      act ("$n tells you 'Here is $p to help you find your way'.",
		   keeper, map_obj, ch, TO_VICT);
	      act ("$n hands you $p.", keeper, map_obj, ch, TO_VICT);
	      obj_to_char (map_obj, ch);
	    }
	  deduct_cost (ch, cost);
	  keeper->gold += cost / 100;
	  keeper->silver += cost - (cost / 100) * 100;
	  save_ctransports ();
	}
    }
}
void do_list (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  if (IS_SET (ch->in_room->room_flags, ROOM_STABLE))
    {
      ROOM_INDEX_DATA *pRoomIndexNext;
      bool found = FALSE;
      CHAR_DATA *mount, *mount_next;
      char tcbuf[MAX_STRING_LENGTH];
      if (IS_NPC (ch))
	return;
      send_to_char ("Mounts you have boarded here:\n\r", ch);
      if (ch->pcdata->boarded != NULL)
	{
	  send_to_char ("[Days\tFees   ]\n\r", ch);
	  for (mount = ch->pcdata->boarded; mount != NULL; mount = mount_next)
	    {
	      mount_next = mount->next_in_board;
	      if (mount->lines == ch->in_room->vnum)
		sprintf (tcbuf, "[%4d\t%-7d] %s\n\r",
			 UMAX ((int) (time (NULL) - mount->wiznet) /
			       86400, 1),
			 UMAX ((int) (time (NULL) - mount->wiznet) /
			       86400, 1) * mount->level * 40,
			 mount->short_descr);
	      send_to_char (tcbuf, ch);
	      found = TRUE;
	    }
	}
      if (!found)
	send_to_char ("None.\n\r", ch);
      send_to_char ("\n\r", ch);
      pRoomIndexNext = get_room_index (ch->in_room->vnum + 1);
      if (pRoomIndexNext == NULL)
	{
	  bug ("Do_list: bad stable at vnum %d.", ch->in_room->vnum);
	  send_to_char ("You can't do that here.\n\r", ch);
	  return;
	}
      found = FALSE;
      for (mount = pRoomIndexNext->people; mount; mount = mount->next_in_room)
	{
	  if (IS_SET (mount->act, ACT_MOUNT))
	    {
	      if (!found)
		{
		  found = TRUE;
		  send_to_char ("Mounts for sale:\n\r", ch);
		}
	      sprintf (buf, "[%2d] %8d - %s\n\r", mount->level,
		       10 * mount->level * mount->level, mount->short_descr);
	      send_to_char (buf, ch);
	    }
	}
      if (!found)
	send_to_char
	  ("Sorry, we're out of mounts for sale right now.\n\r", ch);
      return;
    }
  if (IS_SET (ch->in_room->race_flags, ROOM_PLAYERSTORE)
      && ch->in_room->owner != NULL && ch->in_room->owner[0] != '\0')
    {
      OBJ_DATA *shelf, *obj_walk;
      OBJ_INDEX_DATA *obj_index;
      CHAR_DATA *keeper;
      if (store_closed (ch->in_room) && !IS_IMMORTAL (ch) && !can_build (ch, ch->in_room) )
	{
	  send_to_char ("Sorry, this store is currently closed.\n\r", ch);
	  return;
	}
      if ((keeper = get_char_room_nomask (ch, ch->in_room->owner)) != NULL)
      {
         if (str_cmp (keeper->name, ch->in_room->owner))
		 keeper = NULL;
      }
      if (keeper == NULL)
      {
	for (keeper = ch->in_room->people; keeper != NULL;
	     keeper = keeper->next_in_room)
	  {
	    if (!IS_NPC (keeper))
	      continue;
	    if (!IS_SET (keeper->recruit_flags, RECRUIT_KEEPER))
	      continue;
	    if (keeper->recruit_value[0] != ch->in_room->vnum)
	      continue;
	    break;
	  }
      }
      if (keeper == NULL && !IS_IMMORTAL (ch))
	{
	  send_to_char
	    ("Sorry, the store keeper seems to have stepped out.  Try again later.\n\r",
	     ch);
	  return;
	}
      obj_index = get_obj_index (OBJ_VNUM_SHELF);
      if (obj_index == NULL && !IS_IMMORTAL (ch))
	{
	  send_to_char
	    ("Sorry, the store keeper seems to have stepped out.  Try again later.\n\r",
	     ch);
	  return;
	}
      shelf = create_object (obj_index, 0);
      if (shelf == NULL && !IS_IMMORTAL (ch))
	{
	  send_to_char
	    ("Sorry, the store keeper seems to have stepped out.  Try again later.\n\r",
	     ch);
	  return;
	}
      load_shelf (shelf, ch->in_room->vnum);
      if (shelf->contains != NULL)
	{
	  send_to_char ("[Lv Price Qty] Item\n\r", ch);
	  for (obj_walk = shelf->contains; obj_walk != NULL;
	       obj_walk = obj_walk->next_content)
	    {
	      if (IS_SET (obj_walk->extra_flags[0], ITEM_PERMSTOCK))
		sprintf (buf, "[%2d %5d -- ] %s\n\r", obj_walk->level,
			 obj_walk->cost, obj_walk->short_descr);

	      else
		{
		  int count = 1;
		  while (obj_walk->next_content != NULL
			 && obj_walk->cost == obj_walk->next_content->cost && !str_cmp (obj_walk->short_descr,
					   obj_walk->next_content->
					   short_descr))
		    {
		      obj_walk = obj_walk->next_content;
		      count++;
		    }
		  sprintf (buf, "[%2d %5d %2d ] %s\n\r",
			   obj_walk->level, obj_walk->cost, count,
			   obj_walk->short_descr);
		}
	      send_to_char (buf, ch);
	    }
	}
      else
	send_to_char
	  ("Sorry, this story is currently out of inventory.\n\r", ch);
      extract_obj (shelf);
      return;
    }
  if (IS_SET (ch->in_room->room_flags, ROOM_PET_SHOP))
    {
      ROOM_INDEX_DATA *pRoomIndexNext;
      CHAR_DATA *pet;
      bool found;

      /* hack to make new thalos pets work */
      if (ch->in_room->vnum == 9621)
	pRoomIndexNext = get_room_index (9706);

      else
	pRoomIndexNext = get_room_index (ch->in_room->vnum + 1);
      if (pRoomIndexNext == NULL)
	{
	  bug ("Do_list: bad pet shop at vnum %d.", ch->in_room->vnum);
	  send_to_char ("You can't do that here.\n\r", ch);
	  return;
	}
      found = FALSE;
      for (pet = pRoomIndexNext->people; pet; pet = pet->next_in_room)
	{
	  if (IS_SET (pet->act, ACT_PET))
	    {
	      if (!found)
		{
		  found = TRUE;
		  send_to_char ("Pets for sale:\n\r", ch);
		}
	      sprintf (buf, "[%2d] %8d - %s\n\r", pet->level,
		       10 * pet->level * pet->level, pet->short_descr);
	      send_to_char (buf, ch);
	    }
	}
      if (!found)
	send_to_char ("Sorry, we're out of pets right now.\n\r", ch);
      return;
    }
  else
    {
      CHAR_DATA *keeper;
      OBJ_DATA *obj;
      int cost, count;
      bool found;
      char arg[MAX_INPUT_LENGTH];
      if ((keeper = find_keeper (ch)) == NULL)
	return;
      one_argument (argument, arg);
      found = FALSE;
      for (obj = keeper->carrying; obj; obj = obj->next_content)
	{
	  if (obj->wear_loc == WEAR_NONE && can_see_obj (ch, obj)
	      && (cost = get_cost (keeper, obj, TRUE)) > 0
	      && (arg[0] == '\0' || is_name (arg, obj->name)))
	    {
	      if (!found)
		{
		  found = TRUE;
		  send_to_char ("[Lv Price Qty] Item\n\r", ch);
		}
	      if (IS_OBJ_STAT (obj, ITEM_INVENTORY))
		sprintf (buf, "[%2d %5d -- ] %s\n\r", obj->level,
			 cost, obj->short_descr);

	      else
		{
		  count = 1;
		  while (obj->next_content != NULL
			 && obj->pIndexData ==
			 obj->next_content->
			 pIndexData && !str_cmp (obj->short_descr,
						 obj->next_content->
						 short_descr))
		    {
		      obj = obj->next_content;
		      count++;
		    }
		  sprintf (buf, "[%2d %5d %2d ] %s\n\r", obj->level,
			   cost, count, obj->short_descr);
		}
	      send_to_char (buf, ch);
	    }
	}
      if (!found)
	send_to_char ("You can't buy anything here.\n\r", ch);
      return;
    }
}

OBJ_DATA *get_ctransport_obj (CHAR_DATA * ch, char *name)
{
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *pRoom;
  int room_vnum;
  pArea = ch->in_room->area;
  for (room_vnum = pArea->min_vnum; room_vnum < pArea->max_vnum; room_vnum++)
    {
      if ((pRoom = get_room_index (room_vnum)) == NULL)
	continue;
      if (IS_SET (pRoom->room_flags, ROOM_PIER))
	{
	  OBJ_DATA *obj, *kObj;
	  for (obj = pRoom->contents; obj; obj = obj->next_content)
	    if (obj->item_type == ITEM_CTRANSPORT
		&& is_name (name, obj->name))
	      for (kObj = ch->carrying; kObj; kObj = kObj->next_content)
		if (kObj->pIndexData->vnum ==
		    obj->value[6] && !str_cmp (kObj->owner, obj->owner))
		  return (obj);
	}
    }
  return (NULL);
}

void do_sell (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *keeper;
  OBJ_DATA *obj = NULL, *kObj;
  int cost, roll, itype;
  bool sell_ctransport = FALSE;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Sell what?\n\r", ch);
      return;
    }
  if ((keeper = find_keeper (ch)) == NULL)
    return;
  if (keeper->pIndexData->pShop != NULL)
    for (itype = 0; itype < MAX_TRADE; itype++)
      if (keeper->pIndexData->pShop->buy_type[itype] == ITEM_CTRANSPORT)
	{
	  if ((obj = get_ctransport_obj (ch, arg)) != NULL)
	    sell_ctransport = TRUE;
	  break;
	}
  if (!sell_ctransport)
    {
      if ((obj = get_obj_carry (ch, arg)) == NULL)
	{
	  act ("$n tells you 'You don't have that item'.", keeper, NULL,
	       ch, TO_VICT);
	  ch->reply = keeper;
	  return;
	}
      if (!can_drop_obj (ch, obj))
	{
	  send_to_char ("You can't let go of it.\n\r", ch);
	  return;
	}
      if (!can_see_obj (keeper, obj))
	{
	  act ("$n doesn't see what you are offering.", keeper, NULL, ch,
	       TO_VICT);
	  return;
	}
    }
  if (obj == NULL)
    {
      act ("$n tells you 'You don't have that item'.", keeper, NULL, ch,
	   TO_VICT);
      ch->reply = keeper;
      return;
    }
  if ((cost = get_cost (keeper, obj, FALSE)) <= 0)
    {
      act ("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
      return;
    }
  if (cost > (keeper->silver + 100 * keeper->gold))
    {
      act ("$n tells you 'I'm afraid I don't have enough wealth to buy $p.",
	   keeper, obj, ch, TO_VICT);
      return;
    }
  act ("$n sells $p.", ch, obj, NULL, TO_ROOM);

  /* haggle */
  roll = number_percent ();
  if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT)
      && roll < get_skill (ch, gsn_haggle))
    {
      send_to_char ("You haggle with the shopkeeper.\n\r", ch);
      cost += obj->cost / 2 * roll / 100;
      cost = UMIN (cost, 95 * get_cost (keeper, obj, TRUE) / 100);
      cost = UMIN (cost, (keeper->silver + 100 * keeper->gold));
      check_improve (ch, gsn_haggle, TRUE, 4);
    }
  sprintf (buf, "You sell $p for %d silver and %d gold piece%s.",
	   cost - (cost / 100) * 100, cost / 100, cost == 1 ? "" : "s");
  act (buf, ch, obj, NULL, TO_CHAR);
  ch->gold += cost / 100;
  ch->silver += cost - (cost / 100) * 100;
  deduct_cost (keeper, cost);
  if (keeper->gold < 0)
    keeper->gold = 0;
  if (keeper->silver < 0)
    keeper->silver = 0;
  if (obj->item_type == ITEM_CTRANSPORT)
    {
      obj_from_room (obj);
      obj_to_char (obj, keeper);
      for (kObj = ch->carrying; kObj; kObj = kObj->next_content)
	{
	  if (kObj->pIndexData->vnum == obj->value[6]
	      && !str_cmp (kObj->owner, obj->owner))
	    {
	      extract_obj (kObj);
	      break;
	    }
	}
      save_ctransports ();
    }
  else if (obj->item_type == ITEM_TRASH
	   || IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT))
    {
      extract_obj (obj);
    }
  else
    {
      obj_from_char (obj);
      if (obj->timer)
	SET_BIT (obj->extra_flags[0], ITEM_HAD_TIMER);

      else
	obj->timer = number_range (50, 100);
      obj_to_keeper (obj, keeper);
    }
  return;
}

void do_value (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *keeper;
  OBJ_DATA *obj;
  int cost;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Value what?\n\r", ch);
      return;
    }
  if ((keeper = find_keeper (ch)) == NULL)
    return;
  if ((obj = get_obj_carry (ch, arg)) == NULL)
    {
      act ("$n tells you 'You don't have that item'.", keeper, NULL, ch,
	   TO_VICT);
      ch->reply = keeper;
      return;
    }
  if (!can_see_obj (keeper, obj))
    {
      act ("$n doesn't see what you are offering.", keeper, NULL, ch,
	   TO_VICT);
      return;
    }
  if (!can_drop_obj (ch, obj))
    {
      send_to_char ("You can't let go of it.\n\r", ch);
      return;
    }
  if ((cost = get_cost (keeper, obj, FALSE)) <= 0)
    {
      act ("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
      return;
    }
  sprintf (buf,
	   "$n tells you 'I'll give you %d silver and %d gold coins for $p'.",
	   cost - (cost / 100) * 100, cost / 100);
  act (buf, keeper, obj, ch, TO_VICT);
  ch->reply = keeper;
  return;
}

void do_rarity (CHAR_DATA * ch, char *argument)
{
  char target_obj[MAX_INPUT_LENGTH];
  char rarity_Class[MAX_INPUT_LENGTH];
  char buffer[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *obj;
  int base;
  argument = one_argument (argument, target_obj);

  //get the obj_index record for the desired item
  if (is_number (target_obj))
    {

      //the argument is a vnum -- so we can call get_obj_index directly
      if ((obj = get_obj_index (atol (target_obj))) == NULL)
	{
	  send_to_char ("Object does not exist.", ch);
	  return;
	}
      //calculate normal base rate for object's 'pop'
      switch (obj->rarity)
	{
	case RARITY_RARE:
	  strcpy (rarity_Class, "rare");
	  base = 19;
	  break;
	case RARITY_UNCOMMON:
	  strcpy (rarity_Class, "uncommon");
	  base = 45;
	  break;
	default:
	case RARITY_SOMEWHAT_COMMON:
	  strcpy (rarity_Class, "semi-common");
	  base = 69;
	  break;
	case RARITY_COMMON:
	  strcpy (rarity_Class, "common");
	  base = 100;
	  break;
	}

      //base percentage reflects number of items already in world.
      base -= (obj->count * 2);
      switch (obj->rarity)
	{
	case RARITY_RARE:
	  base = UMAX (base, 2);
	  break;
	case RARITY_UNCOMMON:
	  base = UMAX (base, 20);
	  break;
	case RARITY_SOMEWHAT_COMMON:
	  base = UMAX (base, 46);
	  break;
	case RARITY_COMMON:
	  base = UMAX (base, 70);
	  break;
	}			//end switch
      if (obj->rarity == RARITY_ALWAYS)
	{
	  strcpy (rarity_Class, "always");
	  base = 100;
	}
      if (obj->rarity == RARITY_VERY_RARE)
	{
	  strcpy (rarity_Class, "very rare");
	  base = 1;
	}
      //end of handling for argument passed in as a vnum  
    }
  else
    {
      send_to_char ("Argument be passed as a vnumber, not obj name.", ch);
      return;
    }

  // go ahead and dump results to user
  sprintf (buffer,
	   "Rarity: %-13s  World Count: %5d Base Pop-rate %3d.\n\r",
	   rarity_Class, obj->count, base);
  send_to_char (buffer, ch);
}

//Iblis - Deliver command (for use with the falconry skill)
void do_deliver (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  CHAR_DATA *victim;
  if (IS_NPC(ch))
  {
	  send_to_char ("GODDAMNIT MINAX NO!!!!\n\r",ch);
	  return;
  }
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax: `kDeliver obj char``\r\n", ch);
      return;
    }
  argument = one_argument (argument, arg1);
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax: `kDeliver obj char``\r\n", ch);
      return;
    }
  argument = one_argument (argument, arg2);
  if ((obj = get_obj_carry (ch, arg1)) == NULL)
    {
      send_to_char ("You do not have that item.\n\r", ch);
      return;
    }
  if (obj->wear_loc != WEAR_NONE)
    {
      send_to_char ("You must remove it first.\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg2)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (ch->pcdata->falcon_wait != 0)
    {
      send_to_char ("Your falcon appears to be gone.\n\r", ch);
      return;
    }
  if (!IS_OUTSIDE (ch))
    {
      send_to_char
	("Your falcon awaits your orders, but is currently waiting for you outside.\r\n",
	 ch);
      return;
    }
  if (number_percent () >= get_skill (ch, gsn_falconry) || obj->weight > 20)
    {
      act ("Your falcon swoops down, but fails to get a good grasp on $p.",
	   ch, obj, NULL, TO_CHAR);
      return;
    }
  ch->pcdata->falcon_wait = 10;
  ch->pcdata->falcon_recipient = victim;
  ch->pcdata->falcon_object = obj;

  act ("Your falcon swoops down, grabs $p and flies off.", ch, obj,
       NULL, TO_CHAR);
  act ("$n's falcon swoops down, grabs $p and flies off.", ch, obj, NULL,
       TO_ROOM);
  obj_from_char (obj);
}

//Iblis - The second called function for the deliver command
void delivery_part2 (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  if (IS_NPC (ch))
    return;
  obj = ch->pcdata->falcon_object;
  victim = ch->pcdata->falcon_recipient;
  ch->pcdata->falcon_wait = -10;
  if (victim != NULL && obj != NULL && IS_OUTSIDE (victim))
    {
      if (victim->carry_number + get_obj_number (obj) >
	  can_carry_n (victim) || get_carry_weight (victim) +
	  get_obj_weight (obj) > can_carry_w (victim))
	{
	  act ("A falcon swoops in from the sky and drops $p on the ground.",
	       victim, obj, NULL, TO_CHAR);
	  act ("A falcon swoops in from the sky and drops $p on the ground.",
	       victim, obj, NULL, TO_ROOM);
	  obj_to_room (obj, victim->in_room);
	  ch->pcdata->falcon_object = NULL;
	  ch->pcdata->falcon_recipient = NULL;
	}
      else
	{
	  act ("A falcon swoops in from the sky and drops $p in your hands.",
	       victim, obj, NULL, TO_CHAR);
	  act ("A falcon swoops in from the sky and drops $p in $n's hands.",
	       victim, obj, NULL, TO_ROOM);
	  obj_to_char (obj, victim);
	  ch->pcdata->falcon_object = NULL;
	  ch->pcdata->falcon_recipient = NULL;
	}
    }
}
//Iblis - The final called function (in update.c) for the deliver command
void delivery_part3 (CHAR_DATA * ch)
{
  OBJ_DATA *obj;
  obj = ch->pcdata->falcon_object;
  if (obj == NULL)
    {
      if (IS_OUTSIDE (ch))
	send_to_char
	  ("Your falcon returns, having delivered the item successfully.\r\n",
	   ch);

      else
	send_to_char
	  ("You notice its been a while since your falcon left, it should be back by now...\r\n",
	   ch);
    }
  else
    {
      if (IS_OUTSIDE (ch))
	{
	  if (ch->carry_number + get_obj_number (obj) > can_carry_n (ch)
	      || get_carry_weight (ch) + get_obj_weight (obj) >
	      can_carry_w (ch))
	    {
	      act
		("Your falcon returns with $p, and drops it at your feet, having been unable to deliver it.",
		 ch, obj, NULL, TO_CHAR);
	      act
		("$n's falcon swoops in from the sky and drops $p at $n's feet.",
		 ch, obj, NULL, TO_ROOM);
	      obj_to_room (obj, ch->in_room);
	    }
	  else
	    {
	      act
		("Your falcon returns with $p, and drops it in your hands, having been unable to deliver it.",
		 ch, obj, NULL, TO_CHAR);
	      act
		("$n's falcon swoops in from the sky and drops $p at $n's hands.",
		 ch, obj, NULL, TO_ROOM);
	      obj_to_char (obj, ch);
	    }
	}
      else
	{
	  int randint = number_range (1, 5);
	  switch (randint)
	    {
	    case 1:
	      obj_to_room (obj, get_room_index (12946));
	      break;
	    case 2:
	      obj_to_room (obj, get_room_index (12930));
	      break;
	    case 3:
	      obj_to_room (obj, get_room_index (12944));
	      break;
	    case 4:
	      obj_to_room (obj, get_room_index (13213));
	      break;
	    default:
	      obj_to_room (obj, get_room_index (13022));
	      break;
	    }
	  send_to_char
	    ("You noticed its been a while since your falcon has left, did it even complete its task...\r\n",
	     ch);
	}
    }
}

//Iblis - A simple function that gives a noun describing an object's condition
char *condition_noun (int condition)
{
  static char rvalue[50];

  if (condition == 100)
    strcpy (rvalue, "perfect");

  else if (condition > 90)
    strcpy (rvalue, "excellent");

  else if (condition > 80)
    strcpy (rvalue, "very good");

  else if (condition > 70)
    strcpy (rvalue, "good");

  else if (condition > 60)
    strcpy (rvalue, "fairly good");

  else if (condition > 50)
    strcpy (rvalue, "decent");

  else if (condition > 40)
    strcpy (rvalue, "fair");

  else if (condition > 30)
    strcpy (rvalue, "below average");

  else if (condition > 20)
    strcpy (rvalue, "worn");

  else if (condition > 10)
    strcpy (rvalue, "poor");

  else if (condition > 1)
    strcpy (rvalue, "terrible");

  else
    strcpy (rvalue, "broken");
  return rvalue;
}

//Iblis - A helper function for dealing with quivers
int quiver_holding (OBJ_DATA * obj)
{
  int rvalue = 0;
  OBJ_DATA *counter;
  for (counter = obj->contains; counter != NULL;
       counter = counter->next_content, ++rvalue);

  return rvalue;
}

//Iblis - A simple function that returns if a character already has a quiver or not
bool has_quiver (CHAR_DATA * ch)
{
  OBJ_DATA *counter;
  for (counter = ch->carrying; counter != NULL;
       counter = counter->next_content)
    if (counter->item_type == ITEM_QUIVER)
      return TRUE;
  if (auction != NULL)
    {
      if (auction->seller == ch)
	{
	  if (auction->bid == 0)
	    return TRUE;
	}
    }
  return FALSE;
}

//Iblis - A future command allowing a player to consent to having their corpse looted by another
void do_consent (CHAR_DATA * ch, char* argument)
{


}

//Iblis - A helper function which returns if a player already has a store or not.  Used to 
//enforce the Iverath/Minax suggestion that players only have one store now
bool has_store (CHAR_DATA *ch)
{
  AREA_DATA *pArea;
  int iHash;
  ROOM_INDEX_DATA *pRoomIndex;
  pArea = ch->in_room->area;
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    for (pRoomIndex = room_index_hash[iHash]; pRoomIndex;
         pRoomIndex = pRoomIndex->next)
      {
	if (IS_SET (pRoomIndex->race_flags, ROOM_PLAYERSTORE)
	    && pRoomIndex->owner != NULL && !str_cmp(pRoomIndex->owner,ch->name))
	  return TRUE;
      }
  return FALSE;
}

void do_release (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *wand;
  OBJ_DATA *obj;
  one_argument (argument, arg);
  if (arg[0] == '\0' && ch->fighting == NULL)
    {
      send_to_char ("Release whom or what?\n\r", ch);
      return;
    }
  if ((wand = get_eq_char (ch, WEAR_HOLD)) == NULL)
    {
      send_to_char ("You hold nothing in your hand.\n\r", ch);
      return;
    }
  if (wand->item_type != ITEM_CARD)
    {
      send_to_char ("You can only release cards.\n\r", ch);
      return;
    }
  // Iblis 9/13/04 - No Magic flag
  if (IS_SET (ch->in_room->room_flags2, ROOM_NOMAGIC) && !IS_IMMORTAL (ch))
  {
    send_to_char("There are no magical forces to draw power from.\n\r",ch);
    return;
  }
  if (is_affected (ch, gsn_mirror))
    {
      act ("The mirror arround you is destroyed by your outgoing magic.",
           ch, NULL, NULL, TO_CHAR);
      act ("The mirror around $n is destroyed by $s outgoing magic.",
           ch, NULL, NULL, TO_ROOM);
      affect_strip (ch, gsn_mirror);
    }
  obj = NULL;
  if (arg[0] == '\0')
    {
      if (ch->fighting != NULL)
        {
          victim = ch->fighting;
        }
      else
        {
          send_to_char ("Release whom or what?\n\r", ch);
          return;
        }
    }
  else
    {
      if ((victim = get_char_room (ch, arg)) == NULL
        && (obj = get_obj_here (ch, arg)) == NULL)
      {
        send_to_char ("You can't find it.\n\r", ch);
        return;
      }
    }

  if (ch->Class == PC_CLASS_CHAOS_JESTER)
    WAIT_STATE (ch, PULSE_VIOLENCE);
  else WAIT_STATE (ch, 2 * PULSE_VIOLENCE);
  if (victim != NULL)
  {
    act ("$n releases the powers of $p upon $N.", ch, wand, victim, TO_NOTVICT);
    act ("You release the powers of $p upon $N.", ch, wand, victim, TO_CHAR);
    act ("$n releases the powers of $p upon you.", ch, wand, victim, TO_VICT);
  }
  else
  {
    act ("$n releases the powers of $p upon $P.", ch, wand, obj, TO_ROOM);
    act ("You release the powers of $p upon $P.", ch, wand, obj, TO_CHAR);
  }
      /*if (ch->level < wand->level
          || number_percent () >= 10 + get_skill (ch, gsn_wands) * 9 / 10)
        {
          act ("Your efforts with $p produce only smoke and sparks.",
               ch, wand, NULL, TO_CHAR);
          act ("$n's efforts with $p produce only smoke and sparks.",
               ch, wand, NULL, TO_ROOM);
          check_improve (ch, gsn_wands, FALSE, 2);
        }
      else
        {*/
          obj_cast_spell (wand->value[6], wand->value[5], ch, victim, obj);
	  extract_obj (wand);
          //check_improve (ch, gsn_wands, TRUE, 2);
        //}
/*  if (--wand->value[2] <= 0)
    {
      act ("$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM);
      act ("Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR);
      extract_obj (wand);
    }*/
  return;
}
