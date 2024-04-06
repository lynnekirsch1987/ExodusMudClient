#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include "merc.h"
#include "tables.h"
#include "olc.h"
extern const char *dir_name[];
char *strip_color (char *string)
{
  static char return_string[MAX_STRING_LENGTH];
  char *cptr;
  short x;
  cptr = return_string;
  for (x = 0; string[x] != '\0'; x++)
    {
      if (string[x] == '`')
	{
	  if (string[x + 1] == '\0')
	    break;

	  else
	    x += 2;
	}
      *cptr = string[x];
      cptr++;
    }
  *cptr = '\0';
  return (return_string);
}

void display_resets (CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH], big_buf[MAX_STRING_LENGTH * 5];
  ROOM_INDEX_DATA *pRoomIndexPrev, *pRoomIndex;
  MOB_INDEX_DATA *pMobIndex = NULL;
  OBJ_INDEX_DATA *pObjIndex = NULL, *pPutObjIndex;
  int room_vnum, iReset = 1;
  RESET_DATA *pReset;
  AREA_DATA *pArea;
  room_vnum = ch->in_room->vnum;
  pArea = ch->in_room->area;
  strcpy (big_buf,
	  " No.  Loads    Description       Location         Vnum    Max  Description"
	  "\n\r"
	  "==== ======== ============= =================== ======== ===== ==========="
	  "\n\r");
  for (pReset = pArea->reset_first; pReset; pReset = pReset->next)
    {
      sprintf (buf, "[%2d] ", iReset);
      switch (pReset->command)
	{
	default:
	  sprintf (buf + strlen (buf),
		   "Invalid reset command: %c (Please report this to an IMP)\n\r",
		   pReset->command);
	  break;
	case 'M':
	  pObjIndex = NULL;
	  pMobIndex = NULL;
	  if (pReset->arg3 != room_vnum)
	    continue;
	  if (!(pMobIndex = get_mob_index (pReset->arg1)))
	    {
	      sprintf (buf + strlen (buf),
		       "Invalid Mob VNUM: %d (Please report this to an IMP)\n\r",
		       pReset->arg1);
	      continue;
	    }
	  if (!(pRoomIndex = get_room_index (pReset->arg3)))
	    {
	      sprintf (buf + strlen (buf),
		       "Invalid Room VNUM: %d (Please report this to an IMP)\n\r",
		       pReset->arg3);
	      continue;
	    }
	  sprintf (buf + strlen (buf),
		   "M[%5d] %-13.13s in room             R[%5d]",
		   pReset->arg1, strip_color (pMobIndex->short_descr),
		   pReset->arg3);
	  sprintf (buf + strlen (buf), " [%3d] %-15.15s\n\r",
		   pReset->arg2, strip_color (pRoomIndex->name));
	  pRoomIndexPrev = get_room_index (pRoomIndex->vnum - 1);
	  if (pRoomIndexPrev
	      && IS_SET (pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
	    buf[5] = 'P';
	  break;
	case 'O':
	  pMobIndex = NULL;
	  pObjIndex = NULL;
	  if (pReset->arg3 != room_vnum)
	    continue;
	  if (!(pObjIndex = get_obj_index (pReset->arg1)))
	    {
	      sprintf (buf + strlen (buf),
		       "Invalid Obj VNUM: %d (Please report this to an IMP)\n\r",
		       pReset->arg1);
	      continue;
	    }
	  if (!(pRoomIndex = get_room_index (pReset->arg3)))
	    {
	      sprintf (buf + strlen (buf),
		       "Invalid Room VNUM: %d (Please report this to an IMP\n\r",
		       pReset->arg3);
	      continue;
	    }
	  sprintf (buf + strlen (buf),
		   "O[%5d] %-13.13s in room             ",
		   pReset->arg1, strip_color (pObjIndex->short_descr));
	  sprintf (buf + strlen (buf), "R[%5d] [%3d] %-15.15s\n\r",
		   pReset->arg3, pReset->arg4,
		   strip_color (pRoomIndex->name));
	  break;
	case 'P':
	  if (pObjIndex == NULL)
	    {
	      sprintf (buf + strlen (buf),
		       "P Reset: No object to put it in.\n\r");
	      continue;
	    }
	  if (!(pPutObjIndex = get_obj_index (pReset->arg1)))
	    {
	      sprintf (buf + strlen (buf),
		       "P Reset: Invalid Obj VNUM: %d (Please report this to an IMP)\n\r",
		       pReset->arg1);
	      continue;
	    }
	  sprintf (buf + strlen (buf),
		   "O[%5d] %-13.13s inside              O[%5d]       ",
		   pReset->arg1,
		   strip_color (pPutObjIndex->short_descr), pReset->arg3);
	  sprintf (buf + strlen (buf), "%-15.15s\n\r",
		   strip_color (pObjIndex->short_descr));
	  break;
	case 'G':
	case 'E':
	  if (pMobIndex == NULL)
	    continue;
	  if (!(pObjIndex = get_obj_index (pReset->arg1)))
	    {
	      sprintf (buf + strlen (buf),
		       "G/E Reset: No Obj VNUM: %d (Please report this to an IMP)\n\r",
		       pReset->arg1);
	      continue;
	    }
	  if (pMobIndex->pShop)
	    {
	      sprintf (buf + strlen (buf),
		       "O[%5d] %-13.13s in the inventory of S[%5d]       ",
		       pReset->arg1,
		       strip_color (pObjIndex->short_descr), pMobIndex->vnum);
	      sprintf (buf + strlen (buf), "%-15.15s\n\r",
		       strip_color (pMobIndex->short_descr));
	    }

	  else
	    {
	      sprintf (buf + strlen (buf),
		       "O[%5d] %-13.13s %-19.19s M[%5d]       ",
		       pReset->arg1,
		       strip_color (pObjIndex->short_descr),
		       (pReset->command ==
			'G') ? flag_string (wear_loc_strings,
					    WEAR_NONE) :
		       flag_string (wear_loc_strings, pReset->arg3),
		       pMobIndex->vnum);
	      sprintf (buf + strlen (buf), "%-15.15s\n\r",
		       strip_color (pMobIndex->short_descr));
	    }
	  break;
	case 'D':
	  pMobIndex = NULL;
	  pObjIndex = NULL;
	  if (pReset->arg1 != room_vnum)
	    continue;
	  pRoomIndex = get_room_index (pReset->arg1);
	  sprintf (buf + strlen (buf),
		   "R[%5d] %s door of %-19.19s reset to %s\n\r",
		   pReset->arg1, capitalize (dir_name[pReset->arg2]),
		   strip_color (pRoomIndex->name),
		   flag_string (door_resets, pReset->arg3));
	  break;
	case 'R':
	  pMobIndex = NULL;
	  pObjIndex = NULL;
	  if (pReset->arg1 != room_vnum)
	    continue;
	  if (!(pRoomIndex = get_room_index (pReset->arg1)))
	    {
	      sprintf (buf + strlen (buf),
		       "Randomize Exits - Bad Room %d (Please report this to an IMP)\n\r",
		       pReset->arg1);
	      continue;
	    }
	  sprintf (buf + strlen (buf),
		   "R[%5d] Exits are randomized in %s\n\r",
		   pReset->arg1, strip_color (pRoomIndex->name));
	  break;
	}
      strcat (big_buf, buf);
      iReset++;
    }
  page_to_char (big_buf, ch);
}

bool check_last_mob_reset(ROOM_INDEX_DATA *rid)
{
  RESET_DATA *rd, *last_mob_reset=NULL;
  for (rd = rid->area->reset_first;rd != NULL;rd = rd->next)
    {
      if (rd->command == 'M')
	last_mob_reset = rd;
    }
  if (last_mob_reset->arg3 != rid->vnum)
    return FALSE;
  return TRUE;
}

bool check_last_obj_reset(ROOM_INDEX_DATA *rid)
{
  RESET_DATA *rd, *last_mob_reset=NULL, *last_obj_reset=NULL;
//  char buf[MAX_STRING_LENGTH];
  for (rd = rid->area->reset_first;rd != NULL;rd = rd->next)
    {
      if (rd->command == 'M')
        last_mob_reset = rd;
      if (rd->command == 'O' || rd->command == 'G' ||  rd->command == 'E')
	last_obj_reset = rd;
    }
  if (last_obj_reset == NULL)
    return FALSE;
//  sprintf(buf,"roomvnum-> %d lor->arg3-> %d command-> %c",rid->vnum,last_obj_reset->arg3,last_obj_reset->command);
  //do_echo(char_list,buf);
  if (last_obj_reset->command == 'O')
    {
//	    do_echo(char_list,"command = 'O'");
    if (last_obj_reset->arg3 != rid->vnum)
      return FALSE;
    return TRUE;
    }
  if (last_obj_reset->command == 'G' || last_obj_reset->command == 'E')
    {
//	    do_echo(char_list,"command = G or E");
      if (last_mob_reset->arg3 != rid->vnum)
	return FALSE;
      return TRUE;
    }
  return FALSE;
}
