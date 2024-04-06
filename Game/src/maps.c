#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"

#define MAP_START -1
#define MAP_END   -2
#define MAP_BLANK -3
bool can_map (int xx, int yy, short directions[32], int updown_dir)
{
  short cnt, cnt2;
  short map_index[7][5];
  for (cnt = 0; cnt < 7; cnt++)
    for (cnt2 = 0; cnt2 < 5; cnt2++)
      map_index[cnt][cnt2] = MAP_BLANK;
  cnt = 0;
  while (1)
    {
      if (cnt >= 32)
	return (FALSE);
      if (directions[cnt] == -1)
	{
	  if (cnt == 0)
	    return (FALSE);

	  else
	    return (TRUE);
	}
      switch (directions[cnt])
	{
	case DIR_NORTH:
	  if (yy < 1)
	    return (FALSE);
	  if (map_index[xx][yy] != MAP_BLANK)
	    return (FALSE);
	  map_index[xx][yy] = DIR_NORTH;
	  yy--;
	  break;
	case DIR_SOUTH:
	  if (yy > 3)
	    return (FALSE);
	  if (map_index[xx][yy] != MAP_BLANK)
	    return (FALSE);
	  map_index[xx][yy] = DIR_SOUTH;
	  yy++;
	  break;
	case DIR_EAST:
	  if (xx > 5)
	    return (FALSE);
	  if (map_index[xx][yy] != MAP_BLANK)
	    return (FALSE);
	  map_index[xx][yy] = DIR_EAST;
	  xx++;
	  break;
	case DIR_WEST:
	  if (xx < 1)
	    return (FALSE);
	  if (map_index[xx][yy] != MAP_BLANK)
	    return (FALSE);
	  map_index[xx][yy] = DIR_WEST;
	  xx--;
	  break;
	case DIR_UP:
	  if (updown_dir == DIR_NORTH)
	    {
	      if (yy < 1)
		return (FALSE);
	      if (map_index[xx][yy] != MAP_BLANK)
		return (FALSE);
	      map_index[xx][yy] = DIR_UP;
	      yy--;
	    }
	  else
	    {
	      if (yy > 3)
		return (FALSE);
	      if (map_index[xx][yy] != MAP_BLANK)
		return (FALSE);
	      map_index[xx][yy] = DIR_UP;
	      yy++;
	    }
	  break;
	case DIR_DOWN:
	  if (updown_dir == DIR_NORTH)
	    {
	      if (yy < 1)
		return (FALSE);
	      if (map_index[xx][yy] != MAP_BLANK)
		return (FALSE);
	      map_index[xx][yy] = DIR_DOWN;
	      yy--;
	    }
	  else
	    {
	      if (yy > 3)
		return (FALSE);
	      if (map_index[xx][yy] != MAP_BLANK)
		return (FALSE);
	      map_index[xx][yy] = DIR_DOWN;
	      yy++;
	    }
	  break;
	}
      cnt++;
    }
}
void
draw_map (CHAR_DATA * ch, int xx, int yy, OBJ_DATA * obj,
	  short directions[32], int start_vnum, int end_vnum, int updown_dir)
{
  short cnt, cnt2;
  short map_index[7][5];
  char buf[MAX_STRING_LENGTH * 2];
  char line1[128];
  char line2[128];
  char line3[128];
  char line4[128];
  ROOM_INDEX_DATA *sRoom, *eRoom;
  char *cptr;
  bool isup = FALSE, isdown = FALSE;
  EXTRA_DESCR_DATA *ed;
  for (cnt = 0; cnt < 7; cnt++)
    for (cnt2 = 0; cnt2 < 5; cnt2++)
      map_index[cnt][cnt2] = MAP_BLANK;
  map_index[xx][yy] = MAP_START;
  cnt = 0;
  buf[0] = '\0';
  while (1)
    {
      if (directions[cnt] == -1)
	{
	  for (cnt2 = 0; cnt2 < 5; cnt2++)
	    {
	      line1[0] = '\0';
	      line2[0] = '\0';
	      line3[0] = '\0';
	      line4[0] = '\0';
	      for (cnt = 0; cnt < 7; cnt++)
		{
		  switch (map_index[cnt][cnt2])
		    {
		    case MAP_BLANK:
		      strcat (line1, "     ");
		      strcat (line2, "     ");
		      strcat (line3, "     ");
		      break;
		    case MAP_START:
		      strcat (line1, " --- ");
		      strcat (line2, "| * |");
		      strcat (line3, " --- ");
		      break;
		    case DIR_NORTH:
		    case DIR_SOUTH:
		    case DIR_EAST:
		    case DIR_WEST:
		    case DIR_UP:
		    case DIR_DOWN:
		      if (cnt == xx && cnt2 == yy)
			{
			  strcat (line1, " --- ");
			  strcat (line2, "| X |");
			  strcat (line3, " --- ");
			}
		      else
			{
			  strcat (line1, " --- ");
			  strcat (line2, "|   |");
			  strcat (line3, " --- ");
			}
		      break;
		    }
		  if (map_index[cnt][cnt2] == DIR_SOUTH ||
		      (cnt2 < 4 && map_index[cnt][cnt2 + 1] == DIR_NORTH))
		    strcat (line4, "  |  ");

		  else if (map_index[cnt][cnt2] == DIR_DOWN
			   && updown_dir == DIR_NORTH)
		    strcat (line4, "  /  ");

		  else if (map_index[cnt][cnt2 + 1] == DIR_DOWN
			   && updown_dir == DIR_SOUTH)
		    strcat (line4, "  /  ");

		  else if (map_index[cnt][cnt2 + 1] == DIR_UP
			   && updown_dir == DIR_SOUTH)
		    strcat (line4, "  \\  ");

		  else if (map_index[cnt][cnt2] == DIR_UP
			   && updown_dir == DIR_NORTH)
		    strcat (line4, "  \\  ");

		  else
		    strcat (line4, "     ");
		  if (map_index[cnt][cnt2] == DIR_EAST ||
		      (cnt < 6 && map_index[cnt + 1][cnt2] == DIR_WEST))
		    {
		      strcat (line1, " ");
		      strcat (line2, "-");
		      strcat (line3, " ");
		      strcat (line4, " ");
		    }
		  else
		    {
		      strcat (line1, " ");
		      strcat (line2, " ");
		      strcat (line3, " ");
		      strcat (line4, " ");
		    }
		}
	      strcat (buf, line1);
	      strcat (buf, "\n\r");
	      strcat (buf, line2);
	      strcat (buf, "\n\r");
	      strcat (buf, line3);
	      strcat (buf, "\n\r");
	      strcat (buf, line4);
	      strcat (buf, "\n\r");
	    }
	  cptr = buf + (strlen (buf) - 1);
	  while (*cptr == '\n' || *cptr == '\r' || *cptr == ' ')
	    cptr--;
	  cptr++;
	  *cptr = '\0';
	  for (cnt = 0; cnt < 7; cnt++)
	    for (cnt2 = 0; cnt2 < 5; cnt2++)
	      {
		if (map_index[cnt][cnt2] == DIR_UP)
		  isdown = TRUE;
		if (map_index[cnt][cnt2] == DIR_DOWN)
		  isup = TRUE;
		if (isup == TRUE && isdown == TRUE)
		  break;
	      }
	  sRoom = get_room_index (start_vnum);
	  eRoom = get_room_index (end_vnum);
	  if (eRoom && sRoom)
	    {
	      char lbuf[MAX_INPUT_LENGTH];
	      sprintf (buf + strlen (buf), "\n\r\n\r*: %s     X: %s",
		       sRoom->name, eRoom->name);
	      sprintf (lbuf, "a map to %s", eRoom->name);
	      free_string (obj->short_descr);
	      obj->short_descr = str_dup (lbuf);
	    }
	  if (isdown || isup)
	    strcat (buf, "\n\r");
	  if (isdown)
	    strcat (buf, "'\\' - Downward   ");
	  if (isup)
	    strcat (buf, "'/' - Upward      ");
	  strcat (buf, "\n\r");
	  ed = new_extra_descr ();
	  ed->keyword = str_dup ("maptxt");
	  ed->description = str_dup (buf);
	  ed->next = obj->extra_descr;
	  obj->extra_descr = ed;
	  return;
	}
      switch (directions[cnt])
	{
	case DIR_NORTH:
	  yy--;
	  map_index[xx][yy] = DIR_SOUTH;
	  break;
	case DIR_SOUTH:
	  yy++;
	  map_index[xx][yy] = DIR_NORTH;
	  break;
	case DIR_EAST:
	  xx++;
	  map_index[xx][yy] = DIR_WEST;
	  break;
	case DIR_WEST:
	  xx--;
	  map_index[xx][yy] = DIR_EAST;
	  break;
	case DIR_UP:
	  if (updown_dir == DIR_NORTH)
	    yy--;

	  else
	    yy++;
	  map_index[xx][yy] = DIR_DOWN;
	  break;
	case DIR_DOWN:
	  if (updown_dir == DIR_NORTH)
	    yy--;

	  else
	    yy++;
	  map_index[xx][yy] = DIR_UP;
	  break;
	}
      cnt++;
    }
}
OBJ_DATA *create_map (CHAR_DATA * ch, int start_vnum, int end_vnum)
{
  OBJ_DATA *obj;
  ROOM_INDEX_DATA *pRoom, *endRoom;
  short directions[32];
  short xx, yy;
  if ((pRoom = get_room_index (start_vnum)) == NULL)
    return (NULL);
  if ((endRoom = get_room_index (end_vnum)) == NULL)
    return (NULL);
  xx = 0;
  while (pRoom != endRoom)
    {
      directions[xx] = find_first_step (pRoom, endRoom, ch);
      if (pRoom->exit[directions[xx]] == NULL)
	return (NULL);
      if ((pRoom = pRoom->exit[directions[xx]]->u1.to_room) == NULL)
	return (NULL);
      xx++;
    }
  directions[xx] = -1;
  for (xx = 0; xx < 7; xx++)
    for (yy = 0; yy < 5; yy++)
      if (can_map (xx, yy, directions, DIR_NORTH))
	{
	  obj = create_object (get_obj_index (OBJ_VNUM_GENERIC_MAP), 0);
	  draw_map (ch, xx, yy, obj, directions, start_vnum,
		    end_vnum, DIR_NORTH);
	  return (obj);
	}
      else if (can_map (xx, yy, directions, DIR_SOUTH))
	{
	  obj = create_object (get_obj_index (OBJ_VNUM_GENERIC_MAP), 0);
	  draw_map (ch, xx, yy, obj, directions, start_vnum,
		    end_vnum, DIR_SOUTH);
	  return (obj);
	}
  return (NULL);
}
