#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>
#include "merc.h"
#include "tables.h"
#include "recycle.h"
#include "olc.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"
//x
extern ROOM_INDEX_DATA *room_index_hash[], *new_room_index ();
extern int top_vnum_room;
extern const char *dir_name[];
extern AREA_DATA *get_vnum_area ();
extern EXIT_DATA *new_exit ();
extern void save_area (), build_edit ();
extern OBJ_INDEX_DATA *obj_index_hash[];
extern struct clan_type clan_table[MAX_CLAN];

bool has_store args((CHAR_DATA *ch));

//prototypes
void show_doors_edit_menu args((CHAR_DATA * ch, char *argument));
	

bool can_build (CHAR_DATA *, ROOM_INDEX_DATA * location);
bool can_build_from (CHAR_DATA *, ROOM_INDEX_DATA * location);
void built_room (char *, int);
int build_smod[] = { 150,	/* INSIDE */
  175,				/* CITY   */
  100,				/* FIELD  */
  110,				/* FOREST */
  120,				/* HILLS */
  125,				/* MOUNTAIN */
  100,				/* WATER_SWIM */
  100,				/* WATER_NOSWIM */
  100,				/* UNUSED */
  100,				/* AIR */
  110,				/* DESERT */
  100,				/* UNDERWATER */
  100,				/* GRASSLAND */
  100,				/* TUNDRA */
  120,				/* SWAMP */
  100,				/* WASTELAND */
  100,				/* WATER_OCEAN */
};
int build_table[] = {
  /* Base Costs */
  300000,			/* Creation */
  5000,				/* Remodel (Description Change) */
  1000,				/* Name Change */

  /* Room Flags */
  0,				/* Dark Room Flag */
  300000,			/* NoMob Room Flag */
  500000,			/* NoRecall Room Flag */
  100000,			/* NoMount Room Flag */
  1000000,			/* Private Room Flag */
  0,				/* Indoors Room Flag */
  3000,				/* Add a keyword */
  1000,				/* Add a door */
  1000,				/* Get a key */
  250000,			/* Make clan room */
  2000000,			/* Player store */
  1500000,			/* No-Teleport */
  1000000,			/* PickProof */
  1000000			/* NoBash */
};
inline int get_build_price (int sector, int type)
{
  return ((build_table[type] * build_smod[sector]) / 100);
}

char *display_build_price (int sector, int type)
{
  static char ret_buf[MAX_STRING_LENGTH];
  char tmp_buf[MAX_STRING_LENGTH], *cptr;
  int cost, x, y = 0;
  cost = get_build_price (sector, type);
  if (cost == 0)
    return ("None");
  sprintf (ret_buf, "%d", cost);
  cptr = tmp_buf;
  for (x = strlen (ret_buf) - 1; x > -1; x--, y++)
    {
      *cptr = ret_buf[x];
      cptr++;
      if (y == 2 && x > 0)
	{
	  y = -1;
	  *cptr = ',';
	  cptr++;
	}
    }
  *cptr = '$';
  cptr++;
  *cptr = '\0';
  cptr = ret_buf;
  for (x = strlen (tmp_buf) - 1; x > -1; x--)
    {
      *cptr = tmp_buf[x];
      cptr++;
    }
  *cptr = '\0';
  strcat (ret_buf, " Silver");
  return (ret_buf);
}


/* Show room building flags options to user. */
void do_math (CHAR_DATA * ch, char *argument)
{
  int vnums, con = 0, start = 0, xv;
  AREA_DATA *pArea;
  char tcbuf[MAX_STRING_LENGTH];
  if (!IS_BUILDER (ch))
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }
  if (!is_number (argument))
    {
      send_to_char ("Syntax: math <# of vnums needed>\n\r", ch);
      return;
    }
  vnums = atoi (argument);
  for (xv = 0; xv < 65535; xv++)
    {
      if ((pArea = get_vnum_area (xv)) != NULL)
	{
	  xv = pArea->uvnum + 1;
	  con = 0;
	  start = xv;
	}
      if (con == vnums)
	{
	  sprintf (tcbuf, "Vnums: %d through %d are unused.\n\r", start, xv);
	  send_to_char (tcbuf, ch);
	  return;
	}
      con++;
    }
  send_to_char
    ("Sorry, there aren't that many continuous vnums available.\n\r", ch);
  return;
}

void show_flags_menu (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_STRING_LENGTH];
  send_to_char
    ("\n\r`a`0   -------------------------------------------------   ``\n\r",
     ch);
  send_to_char
    ("`6`0   To edit your room flags, type the number of         ``\n\r", ch);
  send_to_char
    ("`6`0   the function followed by the letter of the flag     ``\n\r", ch);
  send_to_char
    ("`6`0   you wish to modify.                                 ``\n\r", ch);
  send_to_char
    ("`6`0   Example: 1 A - Adds Room is Dark flag.              ``\n\r", ch);
  send_to_char
    ("`6`0            2 C - Deletes Room is Private flag.        ``\n\r", ch);
  send_to_char
    ("`a`0   -------------------------------------------------   ``\n\r", ch);
  send_to_char
    ("`a`0                                                       ``\n\r", ch);
  send_to_char
    ("`a`0 (`o`01`a`0)  `6`0Add flag.                                        ``\n\r",
     ch);
  if (!IS_SET (ch->in_room->room_flags, ROOM_DARK))
    {
      sprintf (tcbuf,
	       "`o`0    A`a`0.  `6`0Room is Dark.         Cost: %-18s ``\n\r",
	       display_build_price (ch->in_room->sector_type,
				    COST_BUILD_DARK_FLAG));
      send_to_char (tcbuf, ch);
    }
  if (!IS_SET (ch->in_room->room_flags, ROOM_NO_MOB))
    {
      sprintf (tcbuf,
	       "`o`0    B`a`0.  `6`0No Mobs Allowed.      Cost: %-18s ``\n\r",
	       display_build_price (ch->in_room->sector_type,
				    COST_BUILD_NOMOB_FLAG));
      send_to_char (tcbuf, ch);
    }
  if (!IS_SET (ch->in_room->room_flags, ROOM_PRIVATE))
    {
      sprintf (tcbuf,
	       "`o`0    C`a`0.  `6`0Room is Private.      Cost: %-18s ``\n\r",
	       display_build_price (ch->in_room->sector_type,
				    COST_BUILD_PRIVATE_FLAG));
      send_to_char (tcbuf, ch);
    }
  if (!IS_SET (ch->in_room->room_flags, ROOM_NOMOUNT))
    {
      sprintf (tcbuf,
	       "`o`0    D`a`0.  `6`0No Mounts Allowed.    Cost: %-18s ``\n\r",
	       display_build_price (ch->in_room->sector_type,
				    COST_BUILD_NOMOUNT_FLAG));
      send_to_char (tcbuf, ch);
    }
  if (!IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL))
    {
      sprintf (tcbuf,
	       "`o`0    E`a`0.  `6`0No Recall from Room.  Cost: %-18s ``\n\r",
	       display_build_price (ch->in_room->sector_type,
				    COST_BUILD_NORECALL_FLAG));
      send_to_char (tcbuf, ch);
    }
  if (!IS_SET (ch->in_room->room_flags, ROOM_INDOORS))
    {
      sprintf (tcbuf,
	       "`o`0    F`a`0.  `6`0Room is Indoors.      Cost: %-18s ``\n\r",
	       display_build_price (ch->in_room->sector_type,
				    COST_BUILD_INDOORS_FLAG));
      send_to_char (tcbuf, ch);
    }
  if ((ch->in_room->clan == CLAN_BOGUS) && (is_clan_leader (ch) || is_clan_deputy(ch)) 
		  && ch->clan != CLAN_BOGUS && clan_table[ch->clan].status == CLAN_ACTIVE)
    {
      sprintf (tcbuf,
	       "`o`0    G`a`0.  `6`0Room is clan room.    Cost: %-18s ``\n\r",
	       display_build_price (ch->in_room->sector_type,
				    COST_BUILD_CLAN_FLAG));
      send_to_char (tcbuf, ch);
    }
  if (!IS_SET (ch->in_room->race_flags, ROOM_PLAYERSTORE))
    {
      sprintf (tcbuf,
	       "`o`0    H`a`0.  `6`0Room is a store.      Cost: %-18s ``\n\r",
	       display_build_price (ch->in_room->sector_type,
				    COST_BUILD_STORE_FLAG));
      send_to_char (tcbuf, ch);
    }
  else
    {
      if (store_closed (ch->in_room))
	send_to_char
	  ("`o`0    I`a`0.  `6`0Store is open.        Cost: None               ``\n\r",
	   ch);
    }
  if (ch->in_room->enter_msg[0] == '\0')
    {
      sprintf (tcbuf, "`o`0    K`a`0.  `6`0Room has an Enter Msg.Cost: %-18s ``\n\r","$100,000 silver");
      send_to_char (tcbuf,ch);
    }
  if (ch->in_room->exit_msg[0] == '\0')
    {
      sprintf (tcbuf, "`o`0    L`a`0.  `6`0Room has an Exit Msg. Cost: %-18s ``\n\r","$100,000 silver");
      send_to_char (tcbuf,ch);
    }
  send_to_char
    ("`a`0 (`o`02`a`0)  `6`0Delete flag.                                     ``\n\r",
     ch);
  if (IS_SET (ch->in_room->room_flags, ROOM_DARK))
    send_to_char
      ("`o`0    A`a`0.  `6`0Room is Dark.                                  ``\n\r",
       ch);
  if (IS_SET (ch->in_room->room_flags, ROOM_NO_MOB))
    send_to_char
      ("`o`0    B`a`0.  `6`0No Mobs Allowed.                               ``\n\r",
       ch);
  if (IS_SET (ch->in_room->room_flags, ROOM_PRIVATE))
    send_to_char
      ("`o`0    C`a`0.  `6`0Room is Private.                               ``\n\r",
       ch);
  if (IS_SET (ch->in_room->room_flags, ROOM_NOMOUNT))
    send_to_char
      ("`o`0    D`a`0.  `6`0No Mounts Allowed.                             ``\n\r",
       ch);
  if (IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL))
    send_to_char
      ("`o`0    E`a`0.  `6`0No Recall from Room.                           ``\n\r",
       ch);
  if (IS_SET (ch->in_room->room_flags, ROOM_INDOORS))
    send_to_char
      ("`o`0    F`a`0.  `6`0Room is Indoors.                               ``\n\r",
       ch);
  if ((ch->in_room->clan != CLAN_BOGUS))
    send_to_char
      ("`o`0    G`a`0.  `6`0Room is clan room.                             ``\n\r",
       ch);
  if (IS_SET (ch->in_room->race_flags, ROOM_PLAYERSTORE))
    {
      send_to_char
	("`o`0    H`a`0.  `6`0Room is a store.                               ``\n\r",
	 ch);
      if (!store_closed (ch->in_room))
	send_to_char
	  ("`o`0    I`a`0.  `6`0Store is open.                                 ``\n\r",
	   ch);
    }
  if (ch->in_room->enter_msg[0] != '\0')
    {
      sprintf (tcbuf, "`o`0    K`a`0.  `6`0Room has an Enter Msg.                         ``\n\r");
      send_to_char (tcbuf,ch);
    }
  if (ch->in_room->exit_msg[0] != '\0')
    {
      sprintf (tcbuf, "`o`0    L`a`0.  `6`0Room has an Exit Msg.                          ``\n\r");
      send_to_char (tcbuf,ch);
    }

  send_to_char
    ("`a`0 (`o`03`a`0)  `6`0End room flag editing.                           ``\n\r",
     ch);
  send_to_char
    ("`a`0                                                       ``\n\r", ch);
  return;
}


/* Show room building keywords options to user. */
void show_keywords_menu (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_STRING_LENGTH];
  EXTRA_DESCR_DATA *ed;
  int mchoice = 'A';
  send_to_char
    ("\n\r`a`0              ** `6`0Keyword Editing Menu `a`0**              ``\n\r",
     ch);
  send_to_char
    ("`a`0                                                      ``\n\r", ch);
  sprintf (tcbuf,
	   "`a`0 (`o`01`a`0)  `6`0Add a room keyword.    Cost: %-18s ``\n\r",
	   display_build_price (ch->in_room->sector_type,
				COST_BUILD_KEYWORD));
  send_to_char (tcbuf, ch);
  send_to_char
    ("`a`0 (`o`02`a`0)  `6`0Delete a room keyword.                          ``\n\r",
     ch);
  for (ed = ch->in_room->extra_descr; ed; ed = ed->next)
    {
      sprintf (tcbuf, "`o`0    %c`a`0.  `6`0%-40s      ``\n\r", mchoice,
	       ed->keyword);
      send_to_char (tcbuf, ch);
      mchoice++;
    }
  send_to_char
    ("`a`0 (`o`03`a`0)  `6`0End keyword editing.                            ``\n\r",
     ch);
  send_to_char
    ("`a`0                                                      ``\n\r", ch);
  return;
}


/* Show room building options to user. */
void show_build_menu (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_STRING_LENGTH];
  send_to_char
    ("\n\r`a`0              ** `6`0Room Building Menu `a`0**              ``\n\r",
     ch);
  send_to_char
    ("`6`0                                                    ``\n\r", ch);
  sprintf (tcbuf,
	   "`a`0 (`o`01`a`0)  `6`0Remodel your room     Cost: %-18s``\n\r",
	   display_build_price (ch->in_room->sector_type,
				COST_BUILD_REMODEL));
  send_to_char (tcbuf, ch);
  sprintf (tcbuf,
	   "`a`0 (`o`02`a`0)  `6`0Rename your room      Cost: %-18s``\n\r",
	   display_build_price (ch->in_room->sector_type, COST_BUILD_NAME));
  send_to_char (tcbuf, ch);
  send_to_char
    ("`a`0 (`o`03`a`0)  `6`0Edit room flags                               ``\n\r",
     ch);
  send_to_char
    ("`a`0 (`o`04`a`0)  `6`0Edit room keywords                            ``\n\r",
     ch);
  send_to_char
    ("`a`0 (`o`05`a`0)  `6`0Edit room doors/keys                          ``\n\r",
     ch);
  send_to_char
    ("`a`0 (`o`06`a`0)  `6`0Exit building menu                            ``\n\r",
     ch);
  return;
}

void show_resets_menu (CHAR_DATA * ch, char *argument)
{
  send_to_char ("\n\r`a`0   ** `6`0Door Resets Menu `a`0**  ``\n\r", ch);
  send_to_char ("`6`0                           ``\n\r", ch);
  send_to_char ("`a`0 (`o`01`a`0)  `6`0Door is open         ``\n\r", ch);
  send_to_char ("`a`0 (`o`02`a`0)  `6`0Door is closed       ``\n\r", ch);
  send_to_char ("`a`0 (`o`03`a`0)  `6`0Door is locked       ``\n\r", ch);
  return;
}

void show_doors_menu (CHAR_DATA * ch, char *argument)
{
  int door_dir;
  char mchoice = 'A', tcbuf[MAX_INPUT_LENGTH];
  send_to_char
    ("\n\r`a`0             ** `6`0Door/Key Edit Menu `a`0**           ``\n\r",
     ch);
  send_to_char
    ("`6`0                                                ``\n\r", ch);
  sprintf (tcbuf,
	   "`a`0 (`o`01`a`0)  `6`0Add a door       Cost: %-18s ``\n\r",
	   display_build_price (ch->in_room->sector_type, COST_BUILD_DOOR));
  send_to_char (tcbuf, ch);
  for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
    if (ch->in_room->exit[door_dir] != NULL)
      if (!IS_SET (ch->in_room->exit[door_dir]->exit_info, EX_ISDOOR))
	{
	  sprintf (tcbuf,
		   "`o`0    %c`a`0.  `6`0%-21s                   ``\n\r",
		   mchoice, dir_name[door_dir]);
	  send_to_char (tcbuf, ch);
	  mchoice++;
	}
  send_to_char
    ("`a`0 (`o`02`a`0)  `6`0Delete a door                             ``\n\r",
     ch);
  mchoice = 'A';
  for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
    if (ch->in_room->exit[door_dir] != NULL)
      if (IS_SET (ch->in_room->exit[door_dir]->exit_info, EX_ISDOOR))
	{
	  sprintf (tcbuf,
		   "`o`0    %c`a`0.  `6`0%-21s                   ``\n\r",
		   mchoice, dir_name[door_dir]);
	  send_to_char (tcbuf, ch);
	  mchoice++;
	}
  sprintf (tcbuf,
	   "`a`0 (`o`03`a`0)  `6`0Make a room key  Cost: %-18s ``\n\r",
	   display_build_price (ch->in_room->sector_type, COST_BUILD_KEY));
  send_to_char (tcbuf, ch);
  mchoice = 'A';
  for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
    if (ch->in_room->exit[door_dir] != NULL)
      if (IS_SET (ch->in_room->exit[door_dir]->exit_info, EX_ISDOOR))
	{
	  sprintf (tcbuf,
		   "`o`0    %c`a`0.  `6`0%-21s                   ``\n\r",
		   mchoice, dir_name[door_dir]);
	  send_to_char (tcbuf, ch);
	  mchoice++;
	}
  send_to_char ("`a`0 (`o`04`a`0)  `6`0Enter door flag menu                      ``\n\r",ch);
  mchoice = 'A';
  for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
    if (ch->in_room->exit[door_dir] != NULL)
      if (IS_SET (ch->in_room->exit[door_dir]->exit_info, EX_ISDOOR))
        {
          sprintf (tcbuf,
                   "`o`0    %c`a`0.  `6`0%-21s                   ``\n\r",
                   mchoice, dir_name[door_dir]);
          send_to_char (tcbuf, ch);
          mchoice++;
        }

  send_to_char
    ("`a`0 (`o`05`a`0)  `6`0Exit door/key edit menu                   ``\n\r",
     ch);
  return;
}

void build_resets (CHAR_DATA * ch, char *argument)
{
  int door_flags, dreset, door;
  RESET_DATA *pReset;
  ROOM_INDEX_DATA *oRoom, *pRoom;
  bool pFound = FALSE;
  switch (argument[0])
    {
    default:
      show_resets_menu (ch, argument);
      return;
    case '1':
      send_to_char ("Door will be open.\n\r", ch);
      door_flags = EX_ISDOOR | EX_NOPASS;
      dreset = 0;
      break;
    case '2':
      send_to_char ("Door will be closed.\n\r", ch);
      door_flags = EX_ISDOOR | EX_NOPASS;
      dreset = 1;
      break;
    case '3':
      send_to_char ("Door will be locked.\n\r", ch);
      door_flags = EX_ISDOOR | EX_NOPASS;
      dreset = 2;
      break;
    }
  pRoom = ch->in_room;
  for (pReset = pRoom->area->reset_first; pReset; pReset = pReset->next)
    {
      if (pReset->command == 'D' && pReset->arg1 == pRoom->vnum
	  && pReset->arg2 == ch->desc->door_dir)
	{
	  pFound = TRUE;
	  pReset->arg3 = dreset;
	  break;
	}
    }
  if (!pFound)
    {
      pReset = new_reset_data ();
      pReset->command = 'D';
      pReset->arg1 = ch->in_room->vnum;
      pReset->arg2 = ch->desc->door_dir;
      pReset->arg3 = dreset;
      add_reset (pRoom, pReset, 0);
    }
  pRoom = pRoom->exit[ch->desc->door_dir]->u1.to_room;
  door = rev_dir[ch->desc->door_dir];
  pFound = FALSE;
  if (pRoom != NULL)
    {
      for (pReset = pRoom->area->reset_first; pReset; pReset = pReset->next)
	{
	  if (pReset->command == 'D'
	      && pReset->arg1 == pRoom->vnum && pReset->arg2 == door)
	    {
	      pFound = TRUE;
	      pReset->arg3 = dreset;
	      break;
	    }
	}
      if (!pFound)
	{
	  pReset = new_reset_data ();
	  pReset->command = 'D';

	  /*Edited by Cory on 6/28/03 -- this is the opposite side of the exit
	     so it should NOT be resetting to the room the character is in
	     but rather pRoom, which has already been defined as the destination
	     room */
	  pReset->arg1 = pRoom->vnum;
	  pReset->arg2 = door;
	  pReset->arg3 = dreset;
	  add_reset (pRoom, pReset, 0);
	}
    }
  ch->in_room->exit[ch->desc->door_dir]->exit_info = door_flags;
  ch->in_room->exit[ch->desc->door_dir]->rs_flags = door_flags;
  oRoom = ch->in_room->exit[ch->desc->door_dir]->u1.to_room;
  if (oRoom != NULL)
    if (oRoom->exit[rev_dir[ch->desc->door_dir]] != NULL) {
	free_string (oRoom->exit[rev_dir[ch->desc->door_dir]]->keyword);
	oRoom->exit[rev_dir[ch->desc->door_dir]]->keyword = str_dup(ch->desc->door_name);
	oRoom->exit[rev_dir[ch->desc->door_dir]]->exit_info = door_flags;
	oRoom->exit[rev_dir[ch->desc->door_dir]]->rs_flags = door_flags;
      }
  free_string (ch->in_room->exit[ch->desc->door_dir]->keyword);
  ch->in_room->exit[ch->desc->door_dir]->keyword =
    str_dup (ch->desc->door_name);
  ch->desc->pEdit = ch->in_room->area;
  ch->desc->editor = BUILD_DOORS;
  save_area (ch->in_room->area);
  save_area (oRoom->area);
  show_doors_menu (ch, argument);
  return;
}

void build_door_name (CHAR_DATA * ch, char *argument)
{
  free_string (ch->desc->door_name);
  ch->desc->door_name = str_dup (argument);
  ch->desc->pEdit = ch->in_room->area;
  ch->desc->editor = BUILD_DRESETS;
  show_resets_menu (ch, argument);
}

void build_doors (CHAR_DATA * ch, char *argument)
{
  char command[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  int door_dir, numchoice = 0, cchoice = 0, key_vnum = ROOM_VNUM_PLAYER_START;
  OBJ_INDEX_DATA *key_obj;
  OBJ_DATA *keyd;
  int iHash, cost;
  ROOM_INDEX_DATA *oRoom;
  RESET_DATA *pReset;		//, *pRoom;
  argument = one_argument (argument, command);
  switch (command[0])
    {
    default:
      show_doors_menu (ch, argument);
      break;
    case '1':
      argument = one_argument (argument, arg);
      for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
	if (ch->in_room->exit[door_dir] != NULL)
	  if (!IS_SET (ch->in_room->exit[door_dir]->exit_info, EX_ISDOOR))
	    numchoice++;
      switch (arg[0])
	{
	default:
	  show_doors_menu (ch, argument);
	  return;
	  break;
	case 'a':
	  if (numchoice < 1)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 1;
	  break;
	case 'b':
	  if (numchoice < 2)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 2;
	  break;
	case 'c':
	  if (numchoice < 3)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 3;
	  break;
	case 'd':
	  if (numchoice < 4)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 4;
	  break;
	case 'e':
	  if (numchoice < 5)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 5;
	  break;
	case 'f':
	  if (numchoice < 6)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 6;
	  break;
	}
      for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
	{
	  if (ch->in_room->exit[door_dir] != NULL)
	    if (!IS_SET (ch->in_room->exit[door_dir]->exit_info, EX_ISDOOR))
	      cchoice++;
	  if (cchoice == numchoice)
	    break;
	}
      cost = get_build_price (ch->in_room->sector_type, COST_BUILD_DOOR);
      if ((ch->silver + 100 * ch->gold) < cost)
	{
	  send_to_char ("Sorry, you can't afford to add a door.\n\r", ch);
	  show_doors_menu (ch, argument);
	  return;
	}
      deduct_cost (ch, cost);
      ch->desc->door_dir = door_dir;
      ch->desc->pEdit = ch->in_room->area;
      ch->desc->editor = BUILD_DNAME;
      send_to_char ("Please name this door:\n\r", ch);
      break;
    case '2':
      argument = one_argument (argument, arg);
      for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
	if (ch->in_room->exit[door_dir] != NULL)
	  if (IS_SET (ch->in_room->exit[door_dir]->exit_info, EX_ISDOOR))
	    numchoice++;
      switch (arg[0])
	{
	default:
	  show_doors_menu (ch, argument);
	  return;
	  break;
	case 'a':
	  if (numchoice < 1)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 1;
	  break;
	case 'b':
	  if (numchoice < 2)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 2;
	  break;
	case 'c':
	  if (numchoice < 3)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 3;
	  break;
	case 'd':
	  if (numchoice < 4)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 4;
	  break;
	case 'e':
	  if (numchoice < 5)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 5;
	  break;
	case 'f':
	  if (numchoice < 6)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 6;
	  break;
	}
      for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
	{
	  if (ch->in_room->exit[door_dir] != NULL)
	    if (IS_SET (ch->in_room->exit[door_dir]->exit_info, EX_ISDOOR))
	      cchoice++;
	  if (cchoice == numchoice)
	    break;
	}

      //Iblis 5/26/03 - Added to properly delete door resets
      cchoice = 1;
      for (pReset = ch->in_room->area->reset_first; pReset;
	   pReset = pReset->next)
	{

	  //send_to_char ("Entering It\r\n");
	  if (pReset->arg1 == ch->in_room->vnum
	      && pReset->command == 'D' && pReset->arg2 == door_dir)
	    {
	      if (!ch->pcdata->security)
		{
		  ch->pcdata->security = !ch->pcdata->security;
		  sprintf (tcbuf, "%d delete", cchoice);
		  do_resets (ch, tcbuf);
		  ch->pcdata->security = !ch->pcdata->security;
		}
	      else
		{
		  sprintf (tcbuf, "%d delete", cchoice);
		  do_resets (ch, tcbuf);
		}
	    }
	  else
	    cchoice++;
	}

      // IBLIS end of add
      REMOVE_BIT (ch->in_room->exit[door_dir]->exit_info, EX_ISDOOR);
      REMOVE_BIT (ch->in_room->exit[door_dir]->exit_info, EX_LOCKED);
      REMOVE_BIT (ch->in_room->exit[door_dir]->exit_info, EX_CLOSED);
      REMOVE_BIT (ch->in_room->exit[door_dir]->exit_info, EX_NOPASS);
      send_to_char ("Door removed.\n\r", ch);
      show_doors_menu (ch, argument);
      break;
    case '3':
      argument = one_argument (argument, arg);
      for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
	if (ch->in_room->exit[door_dir] != NULL)
	  if (IS_SET (ch->in_room->exit[door_dir]->exit_info, EX_ISDOOR))
	    numchoice++;
      switch (arg[0])
	{
	default:
	  show_doors_menu (ch, argument);
	  return;
	  break;
	case 'a':
	  if (numchoice < 1)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 1;
	  break;
	case 'b':
	  if (numchoice < 2)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 2;
	  break;
	case 'c':
	  if (numchoice < 3)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 3;
	  break;
	case 'd':
	  if (numchoice < 4)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 4;
	  break;
	case 'e':
	  if (numchoice < 5)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 5;
	  break;
	case 'f':
	  if (numchoice < 6)
	    {
	      show_doors_menu (ch, argument);
	      return;
	    }
	  numchoice = 6;
	  break;
	}
      for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
	{
	  if (ch->in_room->exit[door_dir] != NULL)
	    if (IS_SET (ch->in_room->exit[door_dir]->exit_info, EX_ISDOOR))
	      cchoice++;
	  if (cchoice == numchoice)
	    break;
	}
      cost = get_build_price (ch->in_room->sector_type, COST_BUILD_KEY);
      if ((ch->silver + 100 * ch->gold) < cost)
	{
	  send_to_char ("Sorry, you can't afford to add a room key.\n\r", ch);
	  show_doors_menu (ch, argument);
	  return;
	}
      deduct_cost (ch, cost);
      if (ch->in_room->exit[door_dir]->key != 0)
	{
	  keyd = create_object (get_obj_index
				(ch->in_room->exit[door_dir]->key), 0);
	  obj_to_char (keyd, ch);
	  send_to_char ("Key created.\n\r", ch);
	  show_doors_menu (ch, argument);
	  return;
	}
      while (1)
	{
	  if (get_obj_index (key_vnum) == NULL)
	    break;
	  key_vnum++;
	  if (key_vnum == ROOM_VNUM_PLAYER_END)
	    {
	      send_to_char ("Sorry, no new keys are available.\n\r", ch);
	      return;
	    }
	}
      key_obj = new_obj_index ();
      key_obj->vnum = key_vnum;
      key_obj->area = ch->in_room->area;
      iHash = key_vnum % MAX_KEY_HASH;
      key_obj->next = obj_index_hash[iHash];
      obj_index_hash[iHash] = key_obj;
      ch->desc->pEdit = (void *) key_obj;
      free_string (key_obj->name);
      free_string (key_obj->short_descr);
      free_string (key_obj->description);
      free_string (key_obj->material);
      key_obj->value[0] = TRUE;
      key_obj->name = str_dup ("key room");
      sprintf (tcbuf, "key to %s's room", ch->name);
      key_obj->short_descr = str_dup (tcbuf);
      if (strlen (ch->in_room->name) < 1)
	{
	  sprintf (tcbuf, "A metal key to %s's room is here.", ch->name);
	  key_obj->description = str_dup (tcbuf);
	}
      else
	{
	  sprintf (tcbuf, "A metal key to '%s' is here.", ch->in_room->name);
	  key_obj->description = str_dup (tcbuf);
	}
      key_obj->material = str_dup ("metal");
      key_obj->item_type = ITEM_KEY;
      key_obj->wear_flags = A;
      key_obj->weight = 10;
      ch->in_room->exit[door_dir]->key = key_vnum;
      oRoom = ch->in_room->exit[door_dir]->u1.to_room;
      if (oRoom != NULL)
	if (oRoom->exit[rev_dir[door_dir]] != NULL)
	  oRoom->exit[rev_dir[door_dir]]->key = key_vnum;
      save_area (oRoom->area);
      save_area (ch->in_room->area);
      keyd = create_object (get_obj_index (key_vnum), 0);
      obj_to_char (keyd, ch);
      send_to_char ("Key created.\n\r", ch);
      show_doors_menu (ch, argument);
      break;
    case '4':
      numchoice = 0;
      argument = one_argument (argument, arg);
      for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
        if (ch->in_room->exit[door_dir] != NULL)
          if (IS_SET (ch->in_room->exit[door_dir]->exit_info, EX_ISDOOR))
            numchoice++;
      switch (arg[0])
        {
        default:
          show_doors_menu (ch, argument);
          return;
          break;
        case 'a':
          if (numchoice < 1)
            {
              show_doors_menu (ch, argument);
              return;
            }
          numchoice = 1;
          break;
        case 'b':
          if (numchoice < 2)
            {
              show_doors_menu (ch, argument);
              return;
            }
          numchoice = 2;
          break;
        case 'c':
          if (numchoice < 3)
            {
              show_doors_menu (ch, argument);
              return;
            }
          numchoice = 3;
          break;
        case 'd':
          if (numchoice < 4)
            {
              show_doors_menu (ch, argument);
              return;
            }
          numchoice = 4;
          break;
        case 'e':
          if (numchoice < 5)
            {
              show_doors_menu (ch, argument);
              return;
            }
          numchoice = 5;
          break;
	case 'f':
          if (numchoice < 6)
            {
              show_doors_menu (ch, argument);
              return;
            }
          numchoice = 6;
          break;
        }
      cchoice = 0;
      for (door_dir = 0; door_dir < MAX_DIR; door_dir++)
        {
          if (ch->in_room->exit[door_dir] != NULL)
            if (IS_SET (ch->in_room->exit[door_dir]->exit_info, EX_ISDOOR))
              cchoice++;
          if (cchoice == numchoice)
            break;
        }
      ch->desc->door_dir = door_dir;
      ch->desc->pEdit = ch->in_room->area;
      ch->desc->editor = BUILD_DOORS_FLAGS;
      show_doors_edit_menu (ch, argument);
      break;
    case '5':
      ch->desc->pEdit = ch->in_room->area;
      ch->desc->editor = BUILD_MAIN;
      show_build_menu (ch, argument);
      break;
    }
  return;
}

void build_name (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_INPUT_LENGTH];
  if (strlen (argument) < 1)
    {
      send_to_char ("\n\r Please enter a new room name: \n\r", ch);
      return;
    }
  sprintf (tcbuf, "%s``", argument);
  free_string (ch->in_room->name);
  ch->in_room->name = str_dup (tcbuf);
  sprintf (tcbuf,
	   "\n\rThe name of your room has been changed to '%s``'.\n\r",
	   ch->in_room->name);
  send_to_char (tcbuf, ch);
  ch->desc->pEdit = ch->in_room->area;
  ch->desc->editor = BUILD_MAIN;
  show_build_menu (ch, argument);
  return;
}

void build_keywords_add (CHAR_DATA * ch, char *argument)
{
  char kword[MAX_INPUT_LENGTH];
  EXTRA_DESCR_DATA *ed;
  ROOM_INDEX_DATA *pRoom;
  one_argument (argument, kword);
  if (strlen (kword) < 1)
    {
      send_to_char ("Keyword adding aborted.\n\r", ch);
      ch->desc->pEdit = ch->in_room->area;
      ch->desc->editor = BUILD_KEYWORDS;
      return;
    }
  pRoom = ch->in_room;
  ed = new_extra_descr ();
  ed->keyword = str_dup (kword);
  ed->description = str_dup ("");
  ed->next = pRoom->extra_descr;
  pRoom->extra_descr = ed;
  send_to_char
    ("Please use the Exodus Editor to enter a keyword description:\n\r\n\r",
     ch);
  ch->desc->editor = BUILD_KEYWORDS;
  edit_string (ch, &ed->description);
  return;
}


//Shinowlan 7/29/98 -- This procedure is called when a player tries to
// abuse the build command by moving to another room after they have started
// building.  It sends a notice to the player, terminates building, and
// provides notice to immortals via Wiznet.
void bad_build (CHAR_DATA * ch)
{
  send_to_char
    ("Please do not attempt to build in rooms you do not own.\n\r", ch);
  send_to_char ("Room editing terminated.\n\r", ch);
  ch->desc->pEdit = NULL;
  edit_done (ch);
  wiznet ("SECURITY ALERT: $N may be attempting to exploit build bug.",
	  ch, NULL, 0, 0, 95);
}

void build_menu (CHAR_DATA * ch, char *argument)
{
  int cost;
  if (strlen (argument) != 1)
    {
      show_build_menu (ch, argument);
      return;
    }
  switch (argument[0])
    {
    default:
      show_build_menu (ch, argument);
      break;
    case '1':

      //Shinowlan 7/29/98 -- make sure they have build priveleges
      // If not, it means they are trying to abuse build.  Terminate build.
      if (!can_build (ch, ch->in_room))
	{
	  bad_build (ch);
	  return;
	}
      cost = get_build_price (ch->in_room->sector_type, COST_BUILD_REMODEL);
      if ((ch->silver + 100 * ch->gold) < cost)
	{
	  send_to_char
	    ("Sorry, you can't afford to remodel your room.\n\r", ch);
	  show_build_menu (ch, argument);
	  return;
	}
      deduct_cost (ch, cost);
      send_to_char
	("Please use the Exodus Editor to create a description for your room:\n\r\n\r",
	 ch);
      edit_string (ch, &ch->in_room->description);
      break;
    case '2':

      //Shinowlan 7/29/98 -- make sure they have build priveleges
      // If not, it means they are trying to abuse build.  Terminate build.
      if (!can_build (ch, ch->in_room))
	{
	  bad_build (ch);
	  return;
	}
      cost = get_build_price (ch->in_room->sector_type, COST_BUILD_NAME);
      if ((ch->silver + 100 * ch->gold) < cost)
	{
	  send_to_char
	    ("Sorry, you can't afford to rename your room.\n\r", ch);
	  show_build_menu (ch, argument);
	  return;
	}
      deduct_cost (ch, cost);
      ch->desc->pEdit = ch->in_room->area;
      ch->desc->editor = BUILD_NAME;
      send_to_char ("Please enter a new room name: \n\r", ch);
      break;
    case '3':

      //Shinowlan 7/29/98 -- make sure they have build priveleges
      // If not, it means they are trying to abuse build.  Terminate build.
      if (!can_build (ch, ch->in_room))
	{
	  bad_build (ch);
	  return;
	}
      ch->desc->pEdit = ch->in_room->area;
      ch->desc->editor = BUILD_FLAGS;
      show_flags_menu (ch, argument);
      break;
    case '4':

      //Shinowlan 7/29/98 -- make sure they have build priveleges
      // If not, it means they are trying to abuse build.  Terminate build.
      if (!can_build (ch, ch->in_room))
	{
	  bad_build (ch);
	  return;
	}
      ch->desc->pEdit = ch->in_room->area;
      ch->desc->editor = BUILD_KEYWORDS;
      show_keywords_menu (ch, argument);
      break;
    case '5':

      //Shinowlan 7/29/98 -- make sure they have build priveleges
      // If not, it means they are trying to abuse build.  Terminate build.
      if (!can_build (ch, ch->in_room))
	{
	  bad_build (ch);
	  return;
	}
      ch->desc->pEdit = ch->in_room->area;
      ch->desc->editor = BUILD_DOORS;
      show_doors_menu (ch, argument);
      break;
    case '6':
      save_area (ch->in_room->area);
      send_to_char ("Room editing finished.\n\r", ch);
      ch->desc->pEdit = NULL;
      edit_done (ch);
      break;
    }
}

void build_keywords (CHAR_DATA * ch, char *argument)
{
  char command[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];
  EXTRA_DESCR_DATA *ed, *ped = NULL;
  int numchoice = 0, cchoice = 1, cost;
  argument = one_argument (argument, command);
  switch (command[0])
    {
    default:
      show_keywords_menu (ch, argument);
      return;
    case '1':
      cost = get_build_price (ch->in_room->sector_type, COST_BUILD_KEYWORD);
      if ((ch->silver + 100 * ch->gold) < cost)
	{
	  send_to_char
	    ("Sorry, you can't afford to add a room keyword.\n\r", ch);
	  show_keywords_menu (ch, argument);
	  return;
	}
      deduct_cost (ch, cost);
      send_to_char
	("Please enter the name keyword you would like to add:\n\r", ch);
      ch->desc->pEdit = ch->in_room->area;
      ch->desc->editor = BUILD_KEYWORDS_ADD;
      break;
    case '2':
      argument = one_argument (argument, arg);
      for (ed = ch->in_room->extra_descr; ed; ed = ed->next)
	numchoice++;
      switch (arg[0])
	{
	default:
	  show_keywords_menu (ch, argument);
	  return;
	  break;
	case 'a':
	  if (numchoice < 1)
	    {
	      show_keywords_menu (ch, argument);
	      return;
	    }
	  numchoice = 1;
	  break;
	case 'b':
	  if (numchoice < 2)
	    {
	      show_keywords_menu (ch, argument);
	      return;
	    }
	  numchoice = 2;
	  break;
	case 'c':
	  if (numchoice < 3)
	    {
	      show_keywords_menu (ch, argument);
	      return;
	    }
	  numchoice = 3;
	  break;
	case 'd':
	  if (numchoice < 4)
	    {
	      show_keywords_menu (ch, argument);
	      return;
	    }
	  numchoice = 4;
	  break;
	case 'e':
	  if (numchoice < 5)
	    {
	      show_keywords_menu (ch, argument);
	      return;
	    }
	  numchoice = 5;
	  break;
	case 'f':
	  if (numchoice < 6)
	    {
	      show_keywords_menu (ch, argument);
	      return;
	    }
	  numchoice = 6;
	  break;
	case 'g':
	  if (numchoice < 7)
	    {
	      show_keywords_menu (ch, argument);
	      return;
	    }
	  numchoice = 7;
	  break;
	case 'h':
	  if (numchoice < 8)
	    {
	      show_keywords_menu (ch, argument);
	      return;
	    }
	  numchoice = 8;
	  break;
	case 'i':
	  if (numchoice < 9)
	    {
	      show_keywords_menu (ch, argument);
	      return;
	    }
	  numchoice = 9;
	  break;
	case 'j':
	  if (numchoice < 10)
	    {
	      show_keywords_menu (ch, argument);
	      return;
	    }
	  numchoice = 10;
	  break;
	}
      for (ed = ch->in_room->extra_descr; ed; ed = ed->next)
	{
	  if (numchoice == cchoice)
	    break;
	  ped = ed;
	  cchoice++;
	}
      if (ped == NULL)
	ch->in_room->extra_descr = ed->next;

      else
	ped->next = ed->next;
      free_extra_descr (ed);
      send_to_char ("Keyword deleted.\n\r", ch);
      break;
    case '3':
      ch->desc->pEdit = ch->in_room->area;
      ch->desc->editor = BUILD_MAIN;
      show_build_menu (ch, argument);
      break;
    }
  return;
}

void flags_menu (CHAR_DATA * ch, char *argument)
{
  char command[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];
  int flagno=0, cost;
  argument = one_argument (argument, command);
  switch (command[0])
    {
    default:
      show_flags_menu (ch, argument);
      return;
      break;
    case '1':
      argument = one_argument (argument, arg);
      switch (arg[0])
	{
	default:
	  show_flags_menu (ch, argument);
	  return;
	  break;
	case 'a':
	  if (IS_SET (ch->in_room->room_flags, ROOM_DARK))
	    {
	      send_to_char ("That flag has already been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  cost =
	    get_build_price (ch->in_room->sector_type, COST_BUILD_DARK_FLAG);
	  if ((ch->silver + 100 * ch->gold) < cost)
	    {
	      send_to_char
		("Sorry, you can't afford to make your room dark.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  deduct_cost (ch, cost);
	  flagno = ROOM_DARK;
	  break;
	case 'b':
	  if (IS_SET (ch->in_room->room_flags, ROOM_NO_MOB))
	    {
	      send_to_char ("That flag has already been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  cost =
	    get_build_price (ch->in_room->sector_type, COST_BUILD_NOMOB_FLAG);
	  if ((ch->silver + 100 * ch->gold) < cost)
	    {
	      send_to_char
		("Sorry, you can't afford to disallow mobiles from your room.\n\r",
		 ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  deduct_cost (ch, cost);
	  flagno = ROOM_NO_MOB;
	  break;
	case 'c':
	  if (IS_SET (ch->in_room->room_flags, ROOM_PRIVATE))
	    {
	      send_to_char ("That flag has already been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  cost =
	    get_build_price (ch->in_room->sector_type,
			     COST_BUILD_PRIVATE_FLAG);
	  if ((ch->silver + 100 * ch->gold) < cost)
	    {
	      send_to_char
		("Sorry, you can't afford to make your room private.\n\r",
		 ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  deduct_cost (ch, cost);
	  flagno = ROOM_PRIVATE;
	  break;
	case 'd':
	  if (IS_SET (ch->in_room->room_flags, ROOM_NOMOUNT))
	    {
	      send_to_char ("That flag has already been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  cost =
	    get_build_price (ch->in_room->sector_type,
			     COST_BUILD_NOMOUNT_FLAG);
	  if ((ch->silver + 100 * ch->gold) < cost)
	    {
	      send_to_char
		("Sorry, you can't afford to disallow mounts from your room.\n\r",
		 ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  deduct_cost (ch, cost);
	  flagno = ROOM_NOMOUNT;
	  break;
	case 'e':
	  if (IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL))
	    {
	      send_to_char ("That flag has already been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  cost =
	    get_build_price (ch->in_room->sector_type,
			     COST_BUILD_NORECALL_FLAG);
	  if ((ch->silver + 100 * ch->gold) < cost)
	    {
	      send_to_char
		("Sorry, you can't afford to make your room no-recall.\n\r",
		 ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  deduct_cost (ch, cost);
	  flagno = ROOM_NO_RECALL;
	  break;
	case 'f':
	  if (IS_SET (ch->in_room->room_flags, ROOM_INDOORS))
	    {
	      send_to_char ("That flag has already been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  cost =
	    get_build_price (ch->in_room->sector_type,
			     COST_BUILD_INDOORS_FLAG);
	  if ((ch->silver + 100 * ch->gold) < cost)
	    {
	      send_to_char
		("Sorry, you can't afford to make your room indoors.\n\r",
		 ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  deduct_cost (ch, cost);
	  flagno = ROOM_INDOORS;
	  break;
	case 'h':
	  if (IS_SET (ch->in_room->race_flags, ROOM_PLAYERSTORE))
	    {
	      send_to_char ("That flag has already been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  cost =
	    get_build_price (ch->in_room->sector_type, COST_BUILD_STORE_FLAG);
	  if ((ch->silver + 100 * ch->gold) < cost)
	    {
	      send_to_char
		("Sorry, you can't afford to make your room a store.\n\r",
		 ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
//	  if (has_store(ch))
//	  {
//		  send_to_char("Nice try.  You already have a store!\n\r",ch);
//		  show_flags_menu (ch, argument);
//		  return;
//	  }
	  deduct_cost (ch, cost);
	  SET_BIT (ch->in_room->race_flags, ROOM_PLAYERSTORE);
	  send_to_char ("Flag added!\n\r", ch);
	  show_flags_menu (ch, argument);
	  return;
	  break;
	case 'i':
	  if (!IS_SET (ch->in_room->race_flags, ROOM_PLAYERSTORE))
	    {
	      show_flags_menu (ch, argument);
	      return;
	    }
	  open_store (ch->in_room);
	  send_to_char ("Flag added!\n\r", ch);
	  show_flags_menu (ch, argument);
	  return;
	  break;
	case 'g':
	  if ((ch->in_room->clan != CLAN_BOGUS))
	    {
	      send_to_char ("That flag has already been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  cost =
	    get_build_price (ch->in_room->sector_type, COST_BUILD_CLAN_FLAG);
	  if ((ch->silver + 100 * ch->gold) < cost)
	    {
	      send_to_char
		("Sorry, you can't afford to make your room a clan room.\n\r",
		 ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  if (!is_full_clan(ch))
	  {
	    send_to_char("Sorry, your clan is not a full clan.\n\r",ch);
	    show_flags_menu (ch, argument);
	    return;
	  }
	  if (!is_clan_leader (ch) && !is_clan_deputy (ch))
	  {
 	    send_to_char("Sorry, you are not a clan deputy or leader.\n\r",ch);
	    show_flags_menu (ch, argument);
	    return;
	  }
					   
	  deduct_cost (ch, cost);
	  ch->in_room->clan = ch->clan;
	  send_to_char ("Flag added!\n\r", ch);
	  show_flags_menu (ch, argument);
	  return;
	  break;
	case 'k':
	  if (ch->in_room->enter_msg[0] != '\0')
	  {
	    send_to_char("That flag has already been added!\n\r",ch);
	    show_flags_menu (ch, argument);
	    return;
	  }
	  cost = 100000;
	  if ((ch->silver + 100 * ch->gold) < cost)
            {
              send_to_char
                ("Sorry, you can't afford to set an enter message.\n\r",
                 ch);
              show_flags_menu (ch, argument);
              return;
            }
	  deduct_cost (ch, cost);
	  ch->desc->pEdit = ch->in_room->area;
	  ch->desc->editor = BUILD_ENTERMSG;
	  send_to_char ("Enter the Enter Message:\n\r",ch);
	  return;
	  break;
	case 'l':
          if (ch->in_room->exit_msg[0] != '\0')
	    {
	      send_to_char("That flag has already been added!\n\r",ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
          cost = 100000;
          if ((ch->silver + 100 * ch->gold) < cost)
            {
              send_to_char
                ("Sorry, you can't afford to set an exit message.\n\r",
                 ch);
              show_flags_menu (ch, argument);
              return;
            }
          deduct_cost (ch, cost);
          ch->desc->pEdit = ch->in_room->area;
          ch->desc->editor = BUILD_EXITMSG;
          send_to_char ("Enter the Exit Message:\n\r",ch);
	  return;
          break;

	}
      TOGGLE_BIT (ch->in_room->room_flags, flagno);
      send_to_char ("Flag added!\n\r", ch);
      show_flags_menu (ch, argument);
      break;
    case '2':
      argument = one_argument (argument, arg);
      switch (arg[0])
	{
	default:
	  show_flags_menu (ch, argument);
	  return;
	  break;
	case 'a':
	  if (!IS_SET (ch->in_room->room_flags, ROOM_DARK))
	    {
	      send_to_char ("That flag has not been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  flagno = ROOM_DARK;
	  break;
	case 'b':
	  if (!IS_SET (ch->in_room->room_flags, ROOM_NO_MOB))
	    {
	      send_to_char ("That flag has not been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  flagno = ROOM_NO_MOB;
	  break;
	case 'c':
	  if (!IS_SET (ch->in_room->room_flags, ROOM_PRIVATE))
	    {
	      send_to_char ("That flag has not been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  flagno = ROOM_PRIVATE;
	  break;
	case 'd':
	  if (!IS_SET (ch->in_room->room_flags, ROOM_NOMOUNT))
	    {
	      send_to_char ("That flag has not been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  flagno = ROOM_NOMOUNT;
	  break;
	case 'e':
	  if (!IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL))
	    {
	      send_to_char ("That flag has not been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  flagno = ROOM_NO_RECALL;
	  break;
	case 'f':
	  if (!IS_SET (ch->in_room->room_flags, ROOM_INDOORS))
	    {
	      send_to_char ("That flag has not been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  flagno = ROOM_INDOORS;
	  break;
	case 'g':
	  if ((ch->in_room->clan == CLAN_BOGUS))
	    {
	      send_to_char ("That flag has not been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  ch->in_room->clan = CLAN_BOGUS;
	  send_to_char ("Flag Deleted!\n\r", ch);
	  show_flags_menu (ch, argument);
	  return;
	case 'h':
	  if (!IS_SET (ch->in_room->race_flags, ROOM_PLAYERSTORE))
	    {
	      send_to_char ("That flag has not been added.\n\r", ch);
	      show_flags_menu (ch, argument);
	      return;
	    }
	  TOGGLE_BIT (ch->in_room->race_flags, ROOM_PLAYERSTORE);
	  send_to_char ("Flag Deleted!\n\r", ch);
	  show_flags_menu (ch, argument);
	  return;
	case 'i':
	  if (!IS_SET (ch->in_room->race_flags, ROOM_PLAYERSTORE))
	    {
	      show_flags_menu (ch, argument);
	      return;
	    }
	  close_store (ch->in_room);
	  send_to_char ("Flag Deleted!\n\r", ch);
	  show_flags_menu (ch, argument);
	  return;
	  break;
	  
	case 'k':
	  if (ch->in_room->enter_msg[0] == '\0')
	    {
	      show_flags_menu (ch, argument);
	      return;
	    }
	  free_string(ch->in_room->enter_msg);
	  ch->in_room->enter_msg = str_dup("");
	  send_to_char ("Flag Deleted!\n\r", ch);
	  show_flags_menu (ch, argument);
	  return;
	  break;
	case 'l':
	  if (ch->in_room->exit_msg[0] == '\0')
	    {
	      show_flags_menu (ch, argument);
	      return;
	    }
	  free_string(ch->in_room->exit_msg);
	  ch->in_room->exit_msg = str_dup("");
	  send_to_char ("Flag Deleted!\n\r", ch);
	  show_flags_menu (ch, argument);
	  return;
	  break;
	}
    


/*      break; */
      TOGGLE_BIT (ch->in_room->room_flags, flagno);
      send_to_char ("Flag Deleted!\n\r", ch);
      show_flags_menu (ch, argument);
      break;
    case '3':
      ch->desc->pEdit = ch->in_room->area;
      ch->desc->editor = BUILD_MAIN;
      show_build_menu (ch, argument);
      break;
    }
}

void edit_player_room (int room_num, int to_room, int door_direction, CHAR_DATA * ch, int cost)
{
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *pRoom;
	EXIT_DATA *pExit;
	int iHash, new_door;
	char tcbuf[MAX_STRING_LENGTH];
	if (!can_build_from (ch, ch->in_room))
	{
		send_to_char ("Sorry, you can't build here.\n\r", ch);
		return;
	}
	new_door = rev_dir[door_direction];
	pArea = get_vnum_area (room_num);
	if (!pArea)
	{
		send_to_char ("Room creation error.\n\r", ch);
		return;
	}
	pRoom = new_room_index ();
	pRoom->area = pArea;
	pRoom->vnum = room_num;
	pRoom->sector_type = ch->in_room->sector_type;
	if (ch->in_room->sector_type == SECT_CITY)
		SET_BIT (pRoom->room_flags, ROOM_LAW);
	if (room_num > top_vnum_room)
		top_vnum_room = room_num;
	free_string (pRoom->owner);
	pRoom->owner = str_dup (ch->name);
	pRoom->heal_rate = 100;
	pRoom->mana_rate = 100;
	iHash = room_num % MAX_KEY_HASH;
	pRoom->next = room_index_hash[iHash];
	room_index_hash[iHash] = pRoom;
	pRoom->exit[new_door] = new_exit ();
	pRoom->exit[new_door]->u1.to_room = ch->in_room;
	pRoom->exit[new_door]->orig_door = new_door;
	pExit = new_exit ();
	pExit->u1.to_room = pRoom;
	pExit->orig_door = door_direction;
	pRoom = ch->in_room;
	pRoom->exit[door_direction] = pExit;
	deduct_cost (ch, cost);
	act ("$n builds a new room $T.", ch, NULL, dir_name[door_direction],
		TO_ROOM);
	sprintf (tcbuf, "You build a new room %s.\n\r", dir_name[door_direction]);
	send_to_char (tcbuf, ch);
	save_area (pArea);
	save_area (pRoom->area);
	return;
}

bool can_build (CHAR_DATA * ch, ROOM_INDEX_DATA * location)
{
	if (location->owner[0] != '\0' && !str_cmp (location->owner, ch->name))
		return (TRUE);
	if ((location->clan != CLAN_BOGUS) &&
		(location->clan == ch->clan) && is_clan_leader (ch))
		return (TRUE);
	return (FALSE);
}

bool can_build_from (CHAR_DATA * ch, ROOM_INDEX_DATA * location)
{
	if (location->owner[0] != '\0' && !str_cmp (location->owner, ch->name))
		return (TRUE);
	if ((location->clan != CLAN_BOGUS) && (location->clan == ch->clan))
		return (TRUE);
	if (!IS_SET (location->room_flags, ROOM_BUILD))
		return (FALSE);
	if ((location->clan != CLAN_BOGUS) && (location->clan != ch->clan))
		return (FALSE);
	if (location->owner[0] != '\0' && str_cmp (location->owner, ch->name))
		return (FALSE);
	return (TRUE);
}

void do_blist (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location;
  char tcbuf[MAX_STRING_LENGTH];
  int vnum;
  send_to_char ("Vnum\t\tOwner of Room\n\r", ch);
  for (vnum = ROOM_VNUM_PLAYER_START; vnum < ROOM_VNUM_PLAYER_END; vnum++)
    {
      if ((location = get_room_index (vnum)) == NULL)
	continue;
      sprintf (tcbuf, "[%5d]\t\t%s\n\r", location->vnum, location->owner);
      send_to_char (tcbuf, ch);
    }
}


void do_openstore (CHAR_DATA * ch, char *argument)
{

	if (can_build (ch, ch->in_room))
	{
		if (IS_SET (ch->in_room->race_flags, ROOM_PLAYERSTORE))
		{
			send_to_char ("Store opened. \n\r", ch);
			open_store (ch->in_room);
			return;
		}
		else 
		{
			send_to_char ("There is no store here.\n\r", ch);
			return;
		}
	}
	else
	{
		send_to_char ("You do not own this room.\n\r", ch);
		return;
	}
	return;
}

void do_closestore (CHAR_DATA * ch, char *argument)
{
	if (can_build (ch, ch->in_room))
	{
		if (IS_SET (ch->in_room->race_flags, ROOM_PLAYERSTORE))
		{
			send_to_char ("Store closed. \n\r", ch);
			close_store (ch->in_room);
			return;
		}
		else 
		{
			send_to_char ("There is no store here.\n\r", ch);
			return;
		}
	}
	else
	{
		send_to_char ("You do not own this room.\n\r", ch);
		return;
	}
	return;
}
void do_build (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int door, room_num = ROOM_VNUM_PLAYER_START, cost;
	argument = one_argument (argument, arg);
	if (!str_cmp (arg, "n") || !str_cmp (arg, "north"))
		door = 0;

	else if (!str_cmp (arg, "e") || !str_cmp (arg, "east"))
		door = 1;

	else if (!str_cmp (arg, "s") || !str_cmp (arg, "south"))
		door = 2;

	else if (!str_cmp (arg, "w") || !str_cmp (arg, "west"))
		door = 3;

	else if (!str_cmp (arg, "u") || !str_cmp (arg, "up"))
		door = 4;

	else if (!str_cmp (arg, "d") || !str_cmp (arg, "down"))
		door = 5;

	else
	{
		if (ch->in_room->vnum < ROOM_VNUM_PLAYER_START
			|| ch->in_room->vnum > ROOM_VNUM_PLAYER_END)
		{
			send_to_char ("Please don't build here.\n\r", ch);
			return;
		}
		if (!can_build (ch, ch->in_room))
		{
			send_to_char ("You do not own this room.\n\r", ch);
			return;
		}
		ch->desc->pEdit = ch->in_room->area;
		ch->desc->editor = BUILD_MAIN;
		show_build_menu (ch, "");
		return;
	}
	if (ch->in_room->exit[door] != NULL)
	{
		send_to_char ("There already exists a room in that direction.\n\r", ch);
		return;
	}
	cost = get_build_price (ch->in_room->sector_type, COST_BUILD_CREATE);
	if ((ch->silver + 100 * ch->gold) < cost)
	{
		send_to_char ("Sorry, you can't afford to build a room here.\n\r", ch);
		return;
	}
	while (1)
	{
		if (get_room_index (room_num) == NULL)
			break;
		if (room_num == ROOM_VNUM_PLAYER_END)
		{
			send_to_char
				("Sorry, we are out of available rooms. Please petition your local elected officials to modify zoning allocations.\n\r",
				ch);
			return;
		}
		room_num++;
	}
	edit_player_room (room_num, ch->in_room->vnum, door, ch, cost);
}


// Akamai 2/26/99 -- Code snippit from Galliard to check exits and print
// all of the connections from the area in which the char is standing and
// all other areas in the mud.
//
#define EXIT_NULL  0
#define EXIT_TO    1
#define EXIT_FROM  2
#define EXIT_BOTH  3
void checkexits (ROOM_INDEX_DATA * room, AREA_DATA * pArea, char *buffer);
void room_pair (int direction, ROOM_INDEX_DATA * room1,
		ROOM_INDEX_DATA * room2, char *buffer);
int has_return_exit (ROOM_INDEX_DATA * room, ROOM_INDEX_DATA * to_room);
void do_exlist (CHAR_DATA * ch, char *argument)
{
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *room;
  int i;
  char buffer[MAX_STRING_LENGTH];

  // info on the area where the char is standing
  pArea = ch->in_room->area;
  for (i = 0; i < MAX_KEY_HASH; i++)
    {

      /* room index hash table */
      // Oh, god - talk about possible lag -- Akamai
      for (room = room_index_hash[i]; room != NULL; room = room->next)
	{

	  /* run through all the rooms on the MUD */
	  checkexits (room, pArea, buffer);
	  send_to_char (buffer, ch);
	}
    }
}


void checkexits (ROOM_INDEX_DATA * room, AREA_DATA * pArea, char *buffer)
{
  char buf[MAX_STRING_LENGTH];
  int i;
  EXIT_DATA *exit;
  ROOM_INDEX_DATA *to_room;
  buffer[0] = '\0';
  for (i = 0; i < 6; i++)
    {
      exit = room->exit[i];
      if (exit)
	{
	  to_room = exit->u1.to_room;

	  /* there is something on the other side */
	  if (to_room)
	    {
	      if ((room->area == pArea) && (to_room->area != pArea))
		{

		  /* an exit from our area to another area */
		  /* check first if it is a two-way exit */
		  if (has_return_exit (room, to_room))
		    {
		      room_pair (EXIT_BOTH, room, to_room, buf);
		    }
		  else
		    {
		      room_pair (EXIT_TO, room, to_room, buf);
		    }
		  strcat (buffer, buf);
		}
	      else
		{

		  /* an exit from another area to our area */
		  if ((room->area != pArea)
		      && (exit->u1.to_room->area == pArea))
		    {

		      /* an exit from another area to our area */
		      if (!has_return_exit (room, to_room))
			{

			  /* two-way exits are handled in the other if */
			  room_pair (EXIT_FROM, room, to_room, buf);
			  strcat (buffer, buf);
			}
		    }		/* if room->area */
		}
	    }			/* if to_room */
	}			/* if exit */
    }				/* for */
}


// Akamai 2/26/99 -- room_pair and has_return_exit were not included in
// the snippet, but something like them were obviously necessary
//
void
room_pair (int direction, ROOM_INDEX_DATA * room1, ROOM_INDEX_DATA * room2,
	   char *buffer)
{

  // print out the area name, room name and vnum for each room
  // and print the direction of the exit
  if (room1 && room2)
    {
      sprintf (buffer, "\"%-25s``\"[%6d][%-18s``]\n\r", room1->name,
	       room1->vnum, room1->area->name);
      if (direction == EXIT_FROM)
	{
	  sprintf (buffer, "%s\t<-- ", buffer);
	}
      else if (direction == EXIT_TO)
	{
	  sprintf (buffer, "%s\t --> ", buffer);
	}
      else if (direction == EXIT_BOTH)
	{
	  sprintf (buffer, "%s\t<--> ", buffer);
	}
      else
	{
	  sprintf (buffer, "%s ???? ", buffer);
	}
      sprintf (buffer, "%s\"%-25s``\"[%6d][%-18s``]\n\r", buffer,
	       room2->name, room2->vnum, room2->area->name);
    }
}

int has_return_exit (ROOM_INDEX_DATA * room, ROOM_INDEX_DATA * to_room)
{
  int i = 0;
  EXIT_DATA *exit;

  // it is a wrong assumption to figure directions are completly fixed
  // so look at all the possible exits from this room to see if they return
  for (i = 0; i < 6; i++)
    {
      exit = to_room->exit[i];
      if (exit && (exit->u1.to_room == room))
	return (i + 1);
    }
  return (0);
}

//Iblis - Player room door flags menu (nobash,nopick, etc)
void show_doors_edit_menu (CHAR_DATA * ch, char *argument)
{
  int door_dir = 0;
  char mchoice = 'A', tcbuf[MAX_INPUT_LENGTH];
  door_dir = ch->desc->door_dir;
  sprintf(tcbuf, "\n\r`a`0     ** `6`0Door Flag Editing Menu for the `j`0%5s `6`0door`a`0**    ``", dir_name[door_dir]);
  send_to_char(tcbuf,ch);
 send_to_char
   ("\n\r`a`0   -------------------------------------------------   ``\n\r",
    ch);
  send_to_char
    ("`6`0   To edit your room flags, type the number of         ``\n\r", ch);
  send_to_char
    ("`6`0   the function followed by the letter of the flag     ``\n\r", ch);
  send_to_char
    ("`6`0   you wish to modify.                                 ``\n\r", ch);
  send_to_char
    ("`6`0   Example: 1 C - Makes the lock pick proof.           ``\n\r", ch);
  send_to_char
    ("`6`0            2 D - Removes bash-proof from the door.    ``\n\r", ch);
  send_to_char
    ("`a`0   -------------------------------------------------   ``\n\r", ch);
  send_to_char
    ("`a`0                                                       ``\n\r", ch);

  send_to_char
    ("`6`0                                                       ``\n\r", ch);
  sprintf (tcbuf,
           "`a`0 (`o`01`a`0)  `6`0Add a door flag                                  ``\n\r");
  send_to_char (tcbuf, ch);
  if (IS_SET (ch->in_room->exit[door_dir]->exit_info, EX_ISDOOR))
    {
      if (!IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_PICKPROOF))
        send_to_char("`a`0                    *** Lock Flags ***                 ``\n\r",ch);
      if (!IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_PICKPROOF) 
	  && !IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_INFURIATING)
	  && !IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_HARD))
	{
	  sprintf (tcbuf, "`o`0    A`a`0]  `6`0Hard.         Cost: %-18s         ``\n\r",
		   "$50,000 silver");
	  send_to_char (tcbuf,ch);
	}
      if (!IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_PICKPROOF)
	  && !IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_INFURIATING))
	{
	  sprintf (tcbuf, "`o`0    B`a`0]  `6`0Infuriating.  Cost: %-18s         ``\n\r",
		   "$500,000 silver");
	  send_to_char (tcbuf,ch);
	}
      if (!IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_PICKPROOF))
	{
	  sprintf (tcbuf, "`o`0    C`a`0]  `6`0Pick-proof.   Cost: %-18s         ``\n\r",
		   "$1,000,000 silver");
	  send_to_char (tcbuf,ch);
	}
      if (!IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_NOBASH)
        || !IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_NOPASS))
        send_to_char("`a`0                    *** Door Flags ***                 ``\n\r",ch);
      if (!IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_NOBASH))
	{
	  sprintf (tcbuf, "`o`0    D`a`0]  `6`0Bash-proof.   Cost: %-18s         ``\n\r",
		   "$1,000,000 silver");
	  send_to_char (tcbuf,ch);
	}
      if (!IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_NOPASS))
	{
	  sprintf (tcbuf, "`o`0    E`a`0]  `6`0Not-passable (via magic).                      ``\n\r");
	  send_to_char (tcbuf,ch);
	}
      
      mchoice++;
    }
  send_to_char
    ("`a`0 (`o`02`a`0)  `6`0Delete a door flag                               ``\n\r",
     ch);
    if (ch->in_room->exit[door_dir] != NULL)
      if (IS_SET (ch->in_room->exit[door_dir]->exit_info, EX_ISDOOR))
        {
 	  if (IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_HARD)
   	      || IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_INFURIATING)
	      || IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_PICKPROOF))
   	  send_to_char("`a`0                    *** Lock Flags ***                 ``\n\r",ch);
	  if (IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_HARD))
            {
              sprintf (tcbuf, "`o`0    A`a`0]  `6`0Hard.                                          ``\n\r");
              send_to_char (tcbuf,ch);
            }
	  if (IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_INFURIATING))
            {
              sprintf (tcbuf, "`o`0    B`a`0]  `6`0Infuriating.                                   ``\n\r");
              send_to_char (tcbuf,ch);
            }
	  if (IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_PICKPROOF))
            {
              sprintf (tcbuf, "`o`0    C`a`0]  `6`0Pick-proof.                                    ``\n\r");
              send_to_char (tcbuf,ch);
            }

	  if (IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_NOBASH) 
			  || IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_NOPASS))
  	    send_to_char("`a`0                    *** Door Flags ***                 ``\n\r",ch);
          if (IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_NOBASH))
            {
              sprintf (tcbuf, "`o`0    D`a`0]  `6`0Bash-proof.                                    ``\n\r");
              send_to_char (tcbuf,ch);
            }
          if (IS_SET (ch->in_room->exit[door_dir]->rs_flags, EX_NOPASS))
            {
              sprintf (tcbuf, "`o`0    E`a`0]  `6`0Not-passable (via magic).                      ``\n\r");
              send_to_char (tcbuf,ch);
            }

        }
  send_to_char
    ("`a`0 (`o`03`a`0)  `6`0Exit door flag edit menu                         ``\n\r",
     ch);
  return;
}

//Iblis - Player room build function for door flags (nobash,nopick,passable, etc)
void build_doors_flags (CHAR_DATA * ch, char *argument)
{
  char command[MAX_INPUT_LENGTH];
  int door_dir;
  int cost;
  int rev;
  ROOM_INDEX_DATA *pToRoom;
  argument = one_argument (argument, command);
  door_dir = ch->desc->door_dir;
    switch (command[0])
      {
      default:
	show_doors_edit_menu (ch, argument);
	break;
      case '1':
	switch (argument[0])
	  {
	  case 'a':
	    if (IS_SET(ch->in_room->exit[door_dir]->rs_flags,EX_HARD))
	      {
		send_to_char("That door already has a hard lock!\n\r",ch);
		ch->desc->pEdit = ch->in_room->area;
		ch->desc->editor = BUILD_DOORS_FLAGS;
		show_doors_edit_menu (ch, argument);
		return;
	      }
	    if (IS_SET(ch->in_room->exit[door_dir]->rs_flags,EX_INFURIATING))
              {
                send_to_char("That door has an infuriating lock!\n\r",ch);
		send_to_char("If you want to downgrade it to hard, remove infuriating first!\n\r",ch);
                ch->desc->pEdit = ch->in_room->area;
                ch->desc->editor = BUILD_DOORS_FLAGS;
		show_doors_edit_menu (ch, argument);
                return;
              }
            if (IS_SET(ch->in_room->exit[door_dir]->rs_flags,EX_PICKPROOF))
              {
                send_to_char("That door is pick-proof!\n\r",ch);
                send_to_char("If you want to downgrade it to hard, remove pick-proof first!\n\r",ch);
                ch->desc->pEdit = ch->in_room->area;
                ch->desc->editor = BUILD_DOORS_FLAGS;
		show_doors_edit_menu (ch, argument);
                return;
              }
	    cost = 50000;
	    if ((ch->silver + 100 * ch->gold) < cost)
	      {
		send_to_char ("Sorry, you can't afford to add that flag.\n\r", ch);
		show_doors_edit_menu (ch, argument);
		return;
	      }
	    deduct_cost (ch, cost);
	    SET_BIT(ch->in_room->exit[door_dir]->rs_flags,EX_HARD);
	    SET_BIT(ch->in_room->exit[door_dir]->exit_info,EX_HARD);
	    if (ch->in_room->exit[door_dir]->keyword[0] == '\0')
	      {
		free_string (ch->in_room->exit[door_dir]->keyword);
		ch->in_room->exit[door_dir]->keyword = str_dup ("door");
	      }
	    pToRoom = ch->in_room->exit[door_dir]->u1.to_room;
	    if (pToRoom != NULL)
	      {
		rev = rev_dir[door_dir];
		if (pToRoom->exit[rev] != NULL)
		  {
		    pToRoom->exit[rev]->rs_flags = ch->in_room->exit[door_dir]->rs_flags;
                  pToRoom->exit[rev]->exit_info =
                    ch->in_room->exit[door_dir]->exit_info;
                  if (pToRoom->exit[rev]->keyword[0] == '\0')
                    {
                      free_string (pToRoom->exit[rev]->keyword);
                      pToRoom->exit[rev]->keyword = str_dup ("door");
                    }
                  save_area (pToRoom->area);
		  }
	      }

	    send_to_char ("Flag added!\n\r", ch);
	    show_doors_edit_menu (ch, argument);
	    return;
	    break;
	  case 'b':
            if (IS_SET(ch->in_room->exit[door_dir]->rs_flags,EX_INFURIATING))
              {
                send_to_char("That door already has an infuriating lock!\n\r",ch);
                ch->desc->pEdit = ch->in_room->area;
                ch->desc->editor = BUILD_DOORS_FLAGS;
		show_doors_edit_menu (ch, argument);
                return;
              }
	    if (IS_SET(ch->in_room->exit[door_dir]->rs_flags,EX_PICKPROOF))
              {
                send_to_char("That door is already pick-proof!\n\r",ch);
		send_to_char("If you want to downgrade it to infuriating, remove pick-proof first!\n\r",ch);
                ch->desc->pEdit = ch->in_room->area;
                ch->desc->editor = BUILD_DOORS_FLAGS;
		show_doors_edit_menu (ch, argument);
                return;
              }

            cost = 500000;
            if ((ch->silver + 100 * ch->gold) < cost)
              {
                send_to_char ("Sorry, you can't afford to add that flag.\n\r", ch);
                show_doors_edit_menu (ch, argument);
                return;
              }
            deduct_cost (ch, cost);
	    REMOVE_BIT(ch->in_room->exit[door_dir]->rs_flags,EX_HARD);
            REMOVE_BIT(ch->in_room->exit[door_dir]->exit_info,EX_HARD);
            SET_BIT(ch->in_room->exit[door_dir]->rs_flags,EX_INFURIATING);
            SET_BIT(ch->in_room->exit[door_dir]->exit_info,EX_INFURIATING);
            if (ch->in_room->exit[door_dir]->keyword[0] == '\0')
              {
                free_string (ch->in_room->exit[door_dir]->keyword);
                ch->in_room->exit[door_dir]->keyword = str_dup ("door");
              }
            pToRoom = ch->in_room->exit[door_dir]->u1.to_room;
            if (pToRoom != NULL)
              {
                rev = rev_dir[door_dir];
                if (pToRoom->exit[rev] != NULL)
                  {
                    pToRoom->exit[rev]->rs_flags = ch->in_room->exit[door_dir]->rs_flags;
                  pToRoom->exit[rev]->exit_info =
                    ch->in_room->exit[door_dir]->exit_info;
                  if (pToRoom->exit[rev]->keyword[0] == '\0')
                    {
                      free_string (pToRoom->exit[rev]->keyword);
                      pToRoom->exit[rev]->keyword = str_dup ("door");
                    }
                  save_area (pToRoom->area);
                  }
              }

            send_to_char ("Flag added!\n\r", ch);
            show_doors_edit_menu (ch, argument);
            return;
            break;
	  case 'c':
            if (IS_SET(ch->in_room->exit[door_dir]->rs_flags,EX_PICKPROOF))
              {
                send_to_char("That door is already pick-proof!\n\r",ch);
                ch->desc->pEdit = ch->in_room->area;
                ch->desc->editor = BUILD_DOORS_FLAGS;
		show_doors_edit_menu (ch, argument);
                return;
              }
            cost = 1000000;
            if ((ch->silver + 100 * ch->gold) < cost)
              {
                send_to_char ("Sorry, you can't afford to add that flag.\n\r", ch);
                show_doors_edit_menu (ch, argument);
                return;
              }
            deduct_cost (ch, cost);
	    REMOVE_BIT(ch->in_room->exit[door_dir]->rs_flags,EX_HARD);
            REMOVE_BIT(ch->in_room->exit[door_dir]->exit_info,EX_HARD);
	    REMOVE_BIT(ch->in_room->exit[door_dir]->rs_flags,EX_INFURIATING);
            REMOVE_BIT(ch->in_room->exit[door_dir]->exit_info,EX_INFURIATING);
            SET_BIT(ch->in_room->exit[door_dir]->rs_flags,EX_PICKPROOF);
            SET_BIT(ch->in_room->exit[door_dir]->exit_info,EX_PICKPROOF);
            if (ch->in_room->exit[door_dir]->keyword[0] == '\0')
              {
                free_string (ch->in_room->exit[door_dir]->keyword);
                ch->in_room->exit[door_dir]->keyword = str_dup ("door");
              }
            pToRoom = ch->in_room->exit[door_dir]->u1.to_room;
            if (pToRoom != NULL)
              {
                rev = rev_dir[door_dir];
                if (pToRoom->exit[rev] != NULL)
                  {
                    pToRoom->exit[rev]->rs_flags = ch->in_room->exit[door_dir]->rs_flags;
                  pToRoom->exit[rev]->exit_info =
                    ch->in_room->exit[door_dir]->exit_info;
                  if (pToRoom->exit[rev]->keyword[0] == '\0')
                    {
                      free_string (pToRoom->exit[rev]->keyword);
                      pToRoom->exit[rev]->keyword = str_dup ("door");
                    }
                  save_area (pToRoom->area);
                  }
              }

            send_to_char ("Flag added!\n\r", ch);
            show_doors_edit_menu (ch, argument);
            return;
            break;
	  case 'd':
	    if (IS_SET(ch->in_room->exit[door_dir]->rs_flags,EX_NOBASH))
              {
                send_to_char("That door is already bash-proof!\n\r",ch);
                ch->desc->pEdit = ch->in_room->area;
                ch->desc->editor = BUILD_DOORS_FLAGS;
		show_doors_edit_menu (ch, argument);
                return;
              }
	    cost = 1000000;
            if ((ch->silver + 100 * ch->gold) < cost)
              {
                send_to_char ("Sorry, you can't afford to add that flag.\n\r", ch);
                show_doors_edit_menu (ch, argument);
                return;
              }
            deduct_cost (ch, cost);
            SET_BIT(ch->in_room->exit[door_dir]->rs_flags,EX_NOBASH);
            SET_BIT(ch->in_room->exit[door_dir]->exit_info,EX_NOBASH);
            if (ch->in_room->exit[door_dir]->keyword[0] == '\0')
              {
                free_string (ch->in_room->exit[door_dir]->keyword);
                ch->in_room->exit[door_dir]->keyword = str_dup ("door");
              }
            pToRoom = ch->in_room->exit[door_dir]->u1.to_room;
            if (pToRoom != NULL)
              {
                rev = rev_dir[door_dir];
                if (pToRoom->exit[rev] != NULL)
                  {
                    pToRoom->exit[rev]->rs_flags = ch->in_room->exit[door_dir]->rs_flags;
                  pToRoom->exit[rev]->exit_info =
                    ch->in_room->exit[door_dir]->exit_info;
                  if (pToRoom->exit[rev]->keyword[0] == '\0')
                    {
                      free_string (pToRoom->exit[rev]->keyword);
                      pToRoom->exit[rev]->keyword = str_dup ("door");
                    }
                  save_area (pToRoom->area);
                  }
              }
            send_to_char ("Flag added!\n\r", ch);
	    show_doors_edit_menu (ch, argument);
	    return;
	    break;
	  case 'e':
            if (IS_SET(ch->in_room->exit[door_dir]->rs_flags,EX_NOPASS))
              {
                send_to_char("That door is already not passable!\n\r",ch);
                ch->desc->pEdit = ch->in_room->area;
		ch->desc->editor = BUILD_DOORS_FLAGS;
		show_doors_edit_menu (ch, argument);
                return;
              }
            SET_BIT(ch->in_room->exit[door_dir]->rs_flags,EX_NOPASS);
            SET_BIT(ch->in_room->exit[door_dir]->exit_info,EX_NOPASS);
            if (ch->in_room->exit[door_dir]->keyword[0] == '\0')
              {
                free_string (ch->in_room->exit[door_dir]->keyword);
                ch->in_room->exit[door_dir]->keyword = str_dup ("door");
              }
            pToRoom = ch->in_room->exit[door_dir]->u1.to_room;
            if (pToRoom != NULL)
              {
                rev = rev_dir[door_dir];
                if (pToRoom->exit[rev] != NULL)
                  {
                    pToRoom->exit[rev]->rs_flags = ch->in_room->exit[door_dir]->rs_flags;
                  pToRoom->exit[rev]->exit_info =
                    ch->in_room->exit[door_dir]->exit_info;
                  if (pToRoom->exit[rev]->keyword[0] == '\0')
                    {
                      free_string (pToRoom->exit[rev]->keyword);
                      pToRoom->exit[rev]->keyword = str_dup ("door");
                    }
                  save_area (pToRoom->area);
                  }
              }

            send_to_char ("Flag added!\n\r", ch);
	    show_doors_edit_menu (ch, argument);
	    return;
	    break;
	  }
	break;
      case '2':
        switch (argument[0])
          {
	  case 'a':
            if (!IS_SET(ch->in_room->exit[door_dir]->rs_flags,EX_HARD))
              {
                send_to_char("That door does not have a hard lock!\n\r",ch);
                ch->desc->pEdit = ch->in_room->area;
		show_doors_edit_menu (ch, argument);
		ch->desc->editor = BUILD_DOORS_FLAGS;
                return;
              }
            REMOVE_BIT(ch->in_room->exit[door_dir]->rs_flags,EX_HARD);
            REMOVE_BIT(ch->in_room->exit[door_dir]->exit_info,EX_HARD);
            if (ch->in_room->exit[door_dir]->keyword[0] == '\0')
              {
                free_string (ch->in_room->exit[door_dir]->keyword);
                ch->in_room->exit[door_dir]->keyword = str_dup ("door");
              }
            pToRoom = ch->in_room->exit[door_dir]->u1.to_room;
            if (pToRoom != NULL)
              {
                rev = rev_dir[door_dir];
                if (pToRoom->exit[rev] != NULL)
                  {
                    pToRoom->exit[rev]->rs_flags = ch->in_room->exit[door_dir]->rs_flags;
                  pToRoom->exit[rev]->exit_info =
                    ch->in_room->exit[door_dir]->exit_info;
                  if (pToRoom->exit[rev]->keyword[0] == '\0')
                    {
                      free_string (pToRoom->exit[rev]->keyword);
                      pToRoom->exit[rev]->keyword = str_dup ("door");
                    }
                  save_area (pToRoom->area);
                  }
              }

            send_to_char ("Flag removed!\n\r", ch);
            show_doors_edit_menu (ch, argument);
            return;
            break;
	  case 'b':
            if (!IS_SET(ch->in_room->exit[door_dir]->rs_flags,EX_INFURIATING))
              {
                send_to_char("That door does not have an infuriating lock!\n\r",ch);
                ch->desc->pEdit = ch->in_room->area;
		ch->desc->editor = BUILD_DOORS_FLAGS;
		show_doors_edit_menu (ch, argument);
                return;
              }
            REMOVE_BIT(ch->in_room->exit[door_dir]->rs_flags,EX_INFURIATING);
            REMOVE_BIT(ch->in_room->exit[door_dir]->exit_info,EX_INFURIATING);
            if (ch->in_room->exit[door_dir]->keyword[0] == '\0')
              {
                free_string (ch->in_room->exit[door_dir]->keyword);
                ch->in_room->exit[door_dir]->keyword = str_dup ("door");
              }
            pToRoom = ch->in_room->exit[door_dir]->u1.to_room;
            if (pToRoom != NULL)
              {
                rev = rev_dir[door_dir];
                if (pToRoom->exit[rev] != NULL)
                  {
                    pToRoom->exit[rev]->rs_flags = ch->in_room->exit[door_dir]->rs_flags;
                  pToRoom->exit[rev]->exit_info =
                    ch->in_room->exit[door_dir]->exit_info;
                  if (pToRoom->exit[rev]->keyword[0] == '\0')
                    {
                      free_string (pToRoom->exit[rev]->keyword);
                      pToRoom->exit[rev]->keyword = str_dup ("door");
                    }
                  save_area (pToRoom->area);
                  }
              }

            send_to_char ("Flag removed!\n\r", ch);
            show_doors_edit_menu (ch, argument);
            return;
            break;
          case 'c':
            if (!IS_SET(ch->in_room->exit[door_dir]->rs_flags,EX_PICKPROOF))
              {
                send_to_char("That door is not pick-proof!\n\r",ch);
                ch->desc->pEdit = ch->in_room->area;
                ch->desc->editor = BUILD_DOORS_FLAGS;
		show_doors_edit_menu (ch, argument);
                return;
              }
            REMOVE_BIT(ch->in_room->exit[door_dir]->rs_flags,EX_PICKPROOF);
            REMOVE_BIT(ch->in_room->exit[door_dir]->exit_info,EX_PICKPROOF);
            if (ch->in_room->exit[door_dir]->keyword[0] == '\0')
              {
                free_string (ch->in_room->exit[door_dir]->keyword);
                ch->in_room->exit[door_dir]->keyword = str_dup ("door");
              }
            pToRoom = ch->in_room->exit[door_dir]->u1.to_room;
            if (pToRoom != NULL)
              {
                rev = rev_dir[door_dir];
                if (pToRoom->exit[rev] != NULL)
                  {
                    pToRoom->exit[rev]->rs_flags = ch->in_room->exit[door_dir]->rs_flags;
                  pToRoom->exit[rev]->exit_info =
                    ch->in_room->exit[door_dir]->exit_info;
                  if (pToRoom->exit[rev]->keyword[0] == '\0')
                    {
                      free_string (pToRoom->exit[rev]->keyword);
                      pToRoom->exit[rev]->keyword = str_dup ("door");
                    }
                  save_area (pToRoom->area);
                  }
              }

            send_to_char ("Flag removed!\n\r", ch);
            show_doors_edit_menu (ch, argument);
	    return;
	    break;
	  case 'd':
            if (!IS_SET(ch->in_room->exit[door_dir]->rs_flags,EX_NOBASH))
              {
                send_to_char("That door is not bash-proof!\n\r",ch);
                ch->desc->pEdit = ch->in_room->area;
                ch->desc->editor = BUILD_DOORS_FLAGS;
		show_doors_edit_menu (ch, argument);
                return;
              }
            REMOVE_BIT(ch->in_room->exit[door_dir]->rs_flags,EX_NOBASH);
            REMOVE_BIT(ch->in_room->exit[door_dir]->exit_info,EX_NOBASH);
            if (ch->in_room->exit[door_dir]->keyword[0] == '\0')
              {
                free_string (ch->in_room->exit[door_dir]->keyword);
                ch->in_room->exit[door_dir]->keyword = str_dup ("door");
              }
            pToRoom = ch->in_room->exit[door_dir]->u1.to_room;
            if (pToRoom != NULL)
              {
                rev = rev_dir[door_dir];
                if (pToRoom->exit[rev] != NULL)
                  {
                    pToRoom->exit[rev]->rs_flags = ch->in_room->exit[door_dir]->rs_flags;
                  pToRoom->exit[rev]->exit_info =
                    ch->in_room->exit[door_dir]->exit_info;
                  if (pToRoom->exit[rev]->keyword[0] == '\0')
                    {
                      free_string (pToRoom->exit[rev]->keyword);
                      pToRoom->exit[rev]->keyword = str_dup ("door");
                    }
                  save_area (pToRoom->area);
                  }
              }
            send_to_char ("Flag removed!\n\r", ch);
            show_doors_edit_menu (ch, argument);
            return;
            break;
	  case 'e':
            if (!IS_SET(ch->in_room->exit[door_dir]->rs_flags,EX_NOPASS))
              {
                send_to_char("That door is already passable!\n\r",ch);
                ch->desc->pEdit = ch->in_room->area;
                ch->desc->editor = BUILD_DOORS_FLAGS;
		show_doors_edit_menu (ch, argument);
                return;
              }
            REMOVE_BIT(ch->in_room->exit[door_dir]->rs_flags,EX_NOPASS);
            REMOVE_BIT(ch->in_room->exit[door_dir]->exit_info,EX_NOPASS);
            if (ch->in_room->exit[door_dir]->keyword[0] == '\0')
              {
                free_string (ch->in_room->exit[door_dir]->keyword);
                ch->in_room->exit[door_dir]->keyword = str_dup ("door");
              }
            pToRoom = ch->in_room->exit[door_dir]->u1.to_room;
            if (pToRoom != NULL)
              {
                rev = rev_dir[door_dir];
                if (pToRoom->exit[rev] != NULL)
                  {
                    pToRoom->exit[rev]->rs_flags = ch->in_room->exit[door_dir]->rs_flags;
                  pToRoom->exit[rev]->exit_info =
                    ch->in_room->exit[door_dir]->exit_info;
                  if (pToRoom->exit[rev]->keyword[0] == '\0')
                    {
                      free_string (pToRoom->exit[rev]->keyword);
                      pToRoom->exit[rev]->keyword = str_dup ("door");
                    }
                  save_area (pToRoom->area);
                  }
              }
            send_to_char ("Flag removed!\n\r", ch);
            show_doors_edit_menu (ch, argument);
            return;
            break;
	  }
	break;
      case '3':
	 ch->desc->pEdit = ch->in_room->area;
         ch->desc->editor = BUILD_DOORS;
         show_doors_menu (ch, argument);
	break;
      }
	
}

//Iblis - Player room build function for enter messages
void build_entermsg (CHAR_DATA * ch, char *argument)
{
  free_string (ch->in_room->enter_msg);
  ch->in_room->enter_msg = str_dup (argument);
  ch->desc->pEdit = ch->in_room->area;
  ch->desc->editor = BUILD_FLAGS;
  send_to_char("Flag added!\n\r",ch);
  show_flags_menu (ch, argument);
}

//Iblis - Player room build function for exit messages
void build_exitmsg (CHAR_DATA * ch, char *argument)
{
  free_string (ch->in_room->exit_msg);
  ch->in_room->exit_msg = str_dup (argument);
  ch->desc->pEdit = ch->in_room->area;
  ch->desc->editor = BUILD_FLAGS;
  send_to_char("Flag added!\n\r",ch);
  show_flags_menu (ch, argument);
}
