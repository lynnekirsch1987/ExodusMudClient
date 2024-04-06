#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include "merc.h"
#include "olc.h"
#include "tables.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"
#include "obj_trig.h"

extern AREA_DATA *area_first;
extern int top_area;
extern const char *dir_name[];
extern int top_vnum_room;
extern ROOM_INDEX_DATA *room_index_hash[];
extern OBJ_INDEX_DATA *obj_index_hash[];
extern MOB_INDEX_DATA *mob_index_hash[];
extern SHOP_DATA *shop_last;
extern int top_mob_index;
extern int top_obj_index;
extern int insert_mob_reset args ((ROOM_INDEX_DATA * room,
				   RESET_DATA * pReset,
				   int mob_vnum, int room_vnum));
extern int insert_obj_reset args ((ROOM_INDEX_DATA * room,
				   RESET_DATA * pReset,
				   int obj_vnum, int room_vnum));
OBJ_TRIG_DATA *get_obj_trig args ((int vnum));
void do_immtalk args((CHAR_DATA * ch, char *argument));
	

/* Return TRUE if area changed, FALSE if not. */
#define REDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define OEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define MEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define AEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )

//char *const dir_name[] = {
//  "north", "east", "south", "west", "up", "down"
//};

bool show_version (CHAR_DATA * ch, char *argument)
{
  send_to_char ("OLC 9.0\n\r", ch);
  return FALSE;
}


/*
 * This table contains help commands and a brief description of each.
 * ------------------------------------------------------------------
 */
extern const struct olc_help_type help_table[];
 /* {"area", area_flags, "Area attributes."},
  {"room", room_flags, "Room attributes."},
  {"Class", Class_flags, "Classes available."},
  {"pcrace", pcrace_flags, "PC Races available."},
  {"sector", sector_flags, "Sector types, terrain."},
  {"exit", exit_flags, "Exit types."},
  {"type", type_flags, "Types of objects."},
  {"extra", extra_flags[0], "Object attributes."},
  {"wear", wear_flags, "Where to wear object."},
  {"spec", spec_table, "Available special programs."},
  {"sex", sex_flags, "Sexes."},
  {"act", act_flags, "Mobile attributes."},
  {"act2", act_flags2, "More Mobile attributes."},
  {"affect", affect_flags, "Mobile affects."},
  {"recruit", recruit_flags, "Mobile recruit options."},
  {"wear-loc", wear_loc_flags, "Where mobile wears object."},
  {"spells", skill_table, "Names of current spells."},
  {"weapon", weapon_flags, "Type of weapon."},
  {"container", container_flags, "Container status."},
  {"liquid", liquid_flags, "Types of liquids."},
  {"ac", ac_type, "Ac for different attacks."},
  {"form", form_flags, "Mobile body form."},
  {"part", part_flags, "Mobile body parts."},
  {"imm", imm_flags, "Mobile immunity."},
  {"res", res_flags, "Mobile resistance."},
  {"vuln", vuln_flags, "Mobile vlnerability."},
  {"off", off_flags, "Mobile offensive behaviour."},
  {"size", size_flags, "Mobile size."},
  {"position", position_flags, "Mobile positions."},
  {"material", material_type, "Material mob/obj is made from."},
  {"wClass", weapon_Class, "Weapon Class."},
  {"wtype", weapons_type, "Special weapon type."},
  {"ftype", furniture_type, "Furniture type."},
  {"actions", pact_flags, "Portal Action Commands."},
  {"gate", gate_flags, "Gate Flags."},
  {"apply", apply_flags, "Apply flags."},
  {"doors", exit_flags, "Type 'help doors' for information."},
  {"rarity", rarity_flags, "Object rarity flags."},
  {"button", button_flags, "Button flags."}, 
  {"clan",clan_flags, "Clan flags."},
  {NULL, 0, NULL}
};*/
int valid_material_name (char *material_name)
{
  int xcnt;
  for (xcnt = 0; material_list[xcnt] != NULL; xcnt++)
    if (!str_cmp (material_list[xcnt], material_name))
      return (1);
  return (0);
}


/*****************************************************************************
 Name:		show_flag_cmds
 Purpose:	Displays settable flags and stats.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_flag_cmds (CHAR_DATA * ch, const struct flag_type *flag_table)
{
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  int flag;
  int col;
  buf1[0] = '\0';
  col = 0;
  for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
      if (flag_table[flag].settable)
	{
	  sprintf (buf, "%-19.18s", flag_table[flag].name);
	  strcat (buf1, buf);
	  if (++col % 4 == 0)
	    strcat (buf1, "\n\r");
	}
    }
  if (col % 4 != 0)
    strcat (buf1, "\n\r");
  send_to_char (buf1, ch);
  return;
}


/*****************************************************************************
 Name:		show_skill_cmds
 Purpose:	Displays all skill functions.
 		Does remove those damn immortal commands from the list.
 		Could be improved by:
 		(1) Adding a check for a particular Class.
 		(2) Adding a check for a level range.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_skill_cmds (CHAR_DATA * ch, int tar)
{
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH * 2];
  int sn;
  int col;
  buf1[0] = '\0';
  col = 0;
  for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (!skill_table[sn].name)
	break;
      if (!str_cmp (skill_table[sn].name, "reserved")
	  || skill_table[sn].spell_fun == spell_null)
	continue;
      if (tar == -1 || skill_table[sn].target == tar)
	{
	  sprintf (buf, "%-19.18s", skill_table[sn].name);
	  strcat (buf1, buf);
	  if (++col % 4 == 0)
	    strcat (buf1, "\n\r");
	}
    }
  if (col % 4 != 0)
    strcat (buf1, "\n\r");
  send_to_char (buf1, ch);
  return;
}


/*****************************************************************************
 Name:		show_spec_cmds
 Purpose:	Displays settable special functions.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_spec_cmds (CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  int spec;
  int col;
  buf1[0] = '\0';
  col = 0;
  send_to_char ("Preceed special functions with 'spec_'\n\r\n\r", ch);
  for (spec = 0;
       spec_table[spec].name != NULL
       && spec_table[spec].name[0] != '\0'; spec++)
    {
      sprintf (buf, "%-19.18s", &spec_table[spec].name[5]);
      strcat (buf1, buf);
      if (++col % 4 == 0)
	strcat (buf1, "\n\r");
    }
  if (col % 4 != 0)
    strcat (buf1, "\n\r");
  send_to_char (buf1, ch);
  return;
}


/*****************************************************************************
 Name:		show_help
 Purpose:	Displays help for many tables used in OLC.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_help (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH], tcbuf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char spell[MAX_INPUT_LENGTH];
  int cnt;
  argument = one_argument (argument, arg);
  one_argument (argument, spell);
  if (!str_cmp (arg, "m") || !str_cmp (arg, "ma")
      || !str_cmp (arg, "mat") || !str_cmp (arg, "material")
      || !str_cmp (arg, "materials"))
    {
      buf[0] = '\0';
      for (cnt = 0; material_list[cnt] != NULL; cnt++)
	{
	  sprintf (tcbuf, "%-19s", material_list[cnt]);
	  strcat (buf, tcbuf);
	  if (cnt % 4 == 0)
	    {
	      strcat (buf, "\n\r");
	      send_to_char (buf, ch);
	      buf[0] = '\0';
	    }
	}
      return TRUE;
    }

  /*
   * Display syntax.
   */
  if (arg[0] == '\0')
    {
      send_to_char ("Syntax:  ? [command]\n\r\n\r", ch);
      send_to_char ("[command]  [description]\n\r", ch);
      for (cnt = 0; help_table[cnt].command != NULL; cnt++)
	{
	  sprintf (buf, "%-10.10s -%s\n\r",
		   capitalize (help_table[cnt].command),
		   help_table[cnt].desc);
	  send_to_char (buf, ch);
	}
      return FALSE;
    }

  /*
   * Find the command, show changeable data.
   * ---------------------------------------
   */
  for (cnt = 0; help_table[cnt].command != NULL; cnt++)
    {
      if (arg[0] == help_table[cnt].command[0]
	  && !str_prefix (arg, help_table[cnt].command))
	{
	  if (help_table[cnt].structure == spec_table)
	    {
	      show_spec_cmds (ch);
	      return FALSE;
	    }

	  else if (help_table[cnt].structure == skill_table)
	    {
	      if (spell[0] == '\0')
		{
		  send_to_char ("Syntax:  ? spells "
				"[ignore/attack/defend/self/object/all]\n\r",
				ch);
		  return FALSE;
		}
	      if (!str_prefix (spell, "all"))
		show_skill_cmds (ch, -1);

	      else if (!str_prefix (spell, "ignore"))
		show_skill_cmds (ch, TAR_IGNORE);

	      else if (!str_prefix (spell, "attack"))
		show_skill_cmds (ch, TAR_CHAR_OFFENSIVE);

	      else if (!str_prefix (spell, "defend"))
		show_skill_cmds (ch, TAR_CHAR_DEFENSIVE);

	      else if (!str_prefix (spell, "self"))
		show_skill_cmds (ch, TAR_CHAR_SELF);

	      else if (!str_prefix (spell, "object"))
		show_skill_cmds (ch, TAR_OBJ_INV);

	      else
		send_to_char ("Syntax:  ? spell "
			      "[ignore/attack/defend/self/object/all]\n\r",
			      ch);
	      return FALSE;
	    }

	  else
	    {
	      show_flag_cmds (ch, help_table[cnt].structure);
//	      show_flag_cmds (ch, reinterpret_cast<const flag_type *>(help_table[cnt].structure));
	      if (!str_cmp (arg,"act"))
	        show_help(ch,"act2");
	      if (!str_cmp (arg,"extra"))
	        show_help(ch,"extra2");
	      if (!str_cmp (arg,"room"))
                show_help(ch,"room2");
	      return FALSE;
	    }
	}
    }
  show_help (ch, "");
  return FALSE;
}

REDIT (redit_mlist)
{
  MOB_INDEX_DATA *pMobIndex;
  AREA_DATA *pArea;
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH * 2];
  char arg[MAX_INPUT_LENGTH];
  bool fAll, found;
  int vnum;
  int col = 0;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Syntax:  mlist <all/name>\n\r", ch);
      return FALSE;
    }
  pArea = ch->in_room->area;
  buf1[0] = '\0';
  fAll = !str_cmp (arg, "all");
  found = FALSE;
  for (vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++)
    {
      if ((pMobIndex = get_mob_index (vnum)))
	{
	  if (fAll || is_name (arg, pMobIndex->player_name))
	    {
	      found = TRUE;
	      sprintf (buf, "[%5d] %-17.16s", pMobIndex->vnum,
		       capitalize (pMobIndex->short_descr));
	      strcat (buf1, buf);
	      if (++col % 3 == 0)
		strcat (buf1, "\n\r");
	    }
	}
    }
  if (!found)
    {
      send_to_char ("Mobile(s) not found in this area.\n\r", ch);
      return FALSE;
    }
  if (col % 3 != 0)
    strcat (buf1, "\n\r");
  send_to_char (buf1, ch);
  return FALSE;
}

REDIT (redit_olist)
{
  OBJ_INDEX_DATA *pObjIndex;
  AREA_DATA *pArea;
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH * 40];
  char arg[MAX_INPUT_LENGTH];
  bool fAll, found;
  int vnum;
  int col = 0;
  if (!IS_BUILDER (ch))
    {
      send_to_char ("You do not have authorization to do that.\n\r", ch);
      return (FALSE);
    }
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Syntax:  olist <all/name/item_type>\n\r", ch);
      return FALSE;
    }
  pArea = ch->in_room->area;
  buf1[0] = '\0';
  fAll = !str_cmp (arg, "all");
  found = FALSE;
  for (vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++)
    {
      if ((pObjIndex = get_obj_index (vnum)))
	{
	  if (fAll || is_name (arg, pObjIndex->name)
	      || flag_value (type_flags, arg) == pObjIndex->item_type)
	    {
	      found = TRUE;
	      sprintf (buf, "[%5d] %-17.16s", pObjIndex->vnum,
		       capitalize (pObjIndex->short_descr));

//            send_to_char (buf, ch);
	      strcat (buf1, buf);

//            
	      if (++col % 3 == 0)
		strcat (buf1, "\n\r");

//            strcat(buf1,buf);
	      //send_to_char ("\n\r", ch);
	    }
//        page_to_char(buf1,ch);
	}
    }
  page_to_char (buf1, ch);
  if (!found)
    {
      send_to_char ("Object(s) not found in this area.\n\r", ch);
      return FALSE;
    }
  if (col % 3 != 0)
    send_to_char ("\n\r", ch);
  return FALSE;
}

REDIT (redit_mshow)
{
  MOB_INDEX_DATA *pMob;
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  mshow <vnum>\n\r", ch);
      return FALSE;
    }
  if ((pMob = get_mob_index (atoi (argument))) == NULL)
    {
      send_to_char ("That mobile does not exist.\n\r", ch);
      return FALSE;
    }
  ch->desc->pEdit = (void *) pMob;
  medit_show (ch, argument);
  ch->desc->pEdit = (void *) ch->in_room;
  return FALSE;
}

REDIT (redit_oshow)
{
  OBJ_INDEX_DATA *pObj;
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  oshow <vnum>\n\r", ch);
      return FALSE;
    }
  if ((pObj = get_obj_index (atoi (argument))) == NULL)
    {
      send_to_char ("That object does not exist.\n\r", ch);
      return FALSE;
    }
  ch->desc->pEdit = (void *) pObj;
  oedit_show (ch, argument);
  ch->desc->pEdit = (void *) ch->in_room;
  return FALSE;
}


// Adeon 6/20/03 -- functions for setting sinking room values
REDIT (redit_sinkvnum)
{
  ROOM_INDEX_DATA *pRoom;
  int value;
  EDIT_ROOM (ch, pRoom);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  sinkvnum <vnum>\n\r", ch);
      return FALSE;
    }
  if (get_room_index ((value = atoi (argument))) == NULL)
    {
      send_to_char ("That room does not exist!\n\r", ch);
      return FALSE;
    }
  pRoom->sink_dest = value;
  send_to_char ("Sink Destination Vnum Set.\n\r", ch);
  return TRUE;
}

REDIT (redit_sinktimer)
{
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM (ch, pRoom);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  sinktimer <seconds>\n\r", ch);
      return FALSE;
    }
  pRoom->sink_timer = atoi (argument);
  send_to_char ("Sink timer set.\n\r", ch);
  return TRUE;
}

REDIT (redit_sinkwarning)
{
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM (ch, pRoom);
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  sinkwarning <message>\n\r", ch);
      return FALSE;
    }
  if (!str_cmp (argument, "none"))
    {
      free_string (pRoom->sink_warning);
      pRoom->sink_warning = NULL;
      send_to_char ("Sink Warning Removed.\n\r", ch);
    }

  else
    {
      free_string (pRoom->sink_warning);
      pRoom->sink_warning = str_dup (argument);
      send_to_char ("Sink Warning Message Set.\n\r", ch);
    }
  return TRUE;
}

REDIT (redit_sinkmsg)
{
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM (ch, pRoom);
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  sinkmsg <message>\n\r", ch);
      return FALSE;
    }
  free_string (pRoom->sink_msg);
  pRoom->sink_msg = str_dup (argument);
  send_to_char ("Sink Message Set.\n\r", ch);
  return TRUE;
}

REDIT (redit_sinkmsgothers)
{
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM (ch, pRoom);
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  sinkmsgothers <message>\n\r", ch);
      return FALSE;
    }
  free_string (pRoom->sink_msg_others);
  pRoom->sink_msg_others = str_dup (argument);
  send_to_char ("Sink Message Others Set.\n\r", ch);
  return TRUE;
}



/*****************************************************************************
 Name:		check_range( lower vnum, upper vnum )
 Purpose:	Ensures the range spans only one area.
 Called by:	aedit_vnum(olc_act.c).
 ****************************************************************************/
bool check_range (int lower, int upper)
{
  AREA_DATA *pArea;
  int cnt = 0;
  for (pArea = area_first; pArea; pArea = pArea->next)
    {

      /*
       * lower < area < upper
       */
      if ((lower <= pArea->lvnum && pArea->lvnum <= upper)
	  || (lower <= pArea->uvnum && pArea->uvnum <= upper))
	++cnt;
      if (cnt > 1)
	return FALSE;
    }
  return TRUE;
}

AREA_DATA *get_vnum_area (int vnum)
{
  AREA_DATA *pArea;
  for (pArea = area_first; pArea; pArea = pArea->next)
    {
      if (vnum >= pArea->lvnum && vnum <= pArea->uvnum)
	return pArea;
    }
  return 0;
}


/*
 * Area Editor Functions.
 */
AEDIT (aedit_show)
{
  AREA_DATA *pArea;
  char buf[MAX_STRING_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  EDIT_AREA (ch, pArea);
  sprintf (buf, "   Name:  [%5d] %s", pArea->vnum, pArea->name);
  sprintf (tcbuf, "%-45s|  File:  %s\n\r", buf, pArea->filename);
  send_to_char (tcbuf, ch);
  sprintf (buf, "Author:  [%s]", pArea->creator);
  sprintf (tcbuf, "%-45s| Vnums:  [%d-%d]\n\r", buf, pArea->lvnum,
	   pArea->uvnum);
  send_to_char (tcbuf, ch);
  sprintf (buf, " Levels:  [%d-%d]", pArea->llev, pArea->ulev);
  sprintf (tcbuf, "%-45s|   Age:  [%d]\n\r", buf, pArea->age);
  send_to_char (tcbuf, ch);
  sprintf (buf, "Players:  [%d]", pArea->nplayer);
  sprintf (tcbuf, "%-45s| Flags: [%s]\n\r", buf,
	   flag_string (area_flags, pArea->area_flags));
  send_to_char (tcbuf, ch);
  if (pArea->helper[0] != '\0')
    {
      sprintf (tcbuf, "Helper: [%s]\n\r", pArea->helper);
      send_to_char (tcbuf, ch);
    }
  if (pArea->clan == CLAN_BOGUS)
    {
      sprintf (tcbuf, "Clan: [%s``]\n\r", NOCLAN_NAME),
	send_to_char (tcbuf, ch);
    }

  else
    {
      sprintf (tcbuf, "Clan: [%s``]\n\r", get_clan_name (pArea->clan)),
	send_to_char (tcbuf, ch);
    }
  sprintf (tcbuf, "Clanwar: [%s]\n\r", pArea->noclan ? "no" : "yes"),
    send_to_char (tcbuf, ch);
  sprintf (tcbuf, "Affluence: [%d]\n\r", pArea->points);
  send_to_char (tcbuf, ch);
  sprintf (tcbuf, "In Construction: [%s]\n\r",
	   pArea->construct ? "yes" : "no"), send_to_char (tcbuf, ch);
  sprintf (tcbuf, "Helps: [%s]\n\r", pArea->help);
  send_to_char (tcbuf, ch);
  return FALSE;
}

AEDIT (aedit_reset)
{
  AREA_DATA *pArea;
  EDIT_AREA (ch, pArea);
  reset_area (pArea);
  send_to_char ("Area reset.\n\r", ch);
  return FALSE;
}

AEDIT (aedit_create)
{
  AREA_DATA *pArea;
  if (IS_NPC (ch))
    return FALSE;
  if (top_area >= INT_MAX)
    {
      send_to_char ("We're out of vnums for new areas.\n\r", ch);
      return FALSE;
    }
  pArea = new_area ();
  area_last->next = pArea;
  area_last = pArea;
  ch->desc->pEdit = (void *) pArea;
  pArea->creator = str_dup (ch->name);
  SET_BIT (pArea->area_flags, AREA_ADDED);
  send_to_char ("Area Created.\n\r", ch);
  return TRUE;
}

AEDIT (aedit_name)
{
  AREA_DATA *pArea;
  EDIT_AREA (ch, pArea);
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:   name [$name]\n\r", ch);
      return FALSE;
    }
  free_string (pArea->name);
  pArea->name = str_dup (argument);
  send_to_char ("Name set.\n\r", ch);
  return TRUE;
}

AEDIT (aedit_helps)
{
  AREA_DATA *pArea;
  EDIT_AREA (ch, pArea);
  free_string (pArea->help);
  pArea->help = str_dup (argument);
  if (argument[0] == '\0')
    send_to_char ("Help string deleted.\n\r", ch);

  else
    send_to_char ("Help string set.\n\r", ch);
  return TRUE;
}

AEDIT (aedit_creator)
{
  AREA_DATA *pArea;
  EDIT_AREA (ch, pArea);
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:   creator [name]\n\r", ch);
      return FALSE;
    }
  free_string (pArea->creator);
  pArea->creator = str_dup (argument);
  send_to_char ("Creator set.\n\r", ch);
  return TRUE;
}

AEDIT (aedit_file)
{
  AREA_DATA *pArea, *this_area;
  char file[MAX_STRING_LENGTH];
  int i, length;
  EDIT_AREA (ch, pArea);
  one_argument (argument, file);	/* Forces Lowercase */
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  filename [$file]\n\r", ch);
      return FALSE;
    }

  /*
   * Simple Syntax Check.
   */
  length = strlen (argument);
  if (length > 8)
    {
      send_to_char ("No more than eight characters allowed.\n\r", ch);
      return FALSE;
    }

  /*
   * Allow only letters and numbers.
   */
  for (i = 0; i < length; i++)
    {
      if (!isalnum (file[i]))
	{
	  send_to_char ("Only letters and numbers are valid.\n\r", ch);
	  return FALSE;
	}
    }
  strcat (file, ".enc");
  for (this_area = area_first; this_area; this_area = this_area->next)
    if (!str_cmp (this_area->filename, file))
      {
	send_to_char ("Sorry, that filename is already in use.\n\r", ch);
	return (FALSE);
      }
  free_string (pArea->filename);
  pArea->filename = str_dup (file);
  send_to_char ("Filename set.\n\r", ch);
  return TRUE;
}

AEDIT (aedit_age)
{
  AREA_DATA *pArea;
  char age[MAX_STRING_LENGTH];
  EDIT_AREA (ch, pArea);
  one_argument (argument, age);
  if (!is_number (age) || age[0] == '\0')
    {
      send_to_char ("Syntax:  age [#age]\n\r", ch);
      return FALSE;
    }
  pArea->age = atoi (age);
  send_to_char ("Age set.\n\r", ch);
  return TRUE;
}

AEDIT (aedit_clanwar)
{
  AREA_DATA *pArea;
  EDIT_AREA (ch, pArea);
  if (!str_cmp (argument, "yes"))
    {
      pArea->noclan = FALSE;
      return TRUE;
    }
  if (!str_cmp (argument, "no"))
    {
      pArea->noclan = TRUE;
      return TRUE;
    }
  send_to_char ("Syntax: clanwar [yes|no]\n\r", ch);
  return FALSE;
}

AEDIT (aedit_clan)
{
  char buf[MAX_INPUT_LENGTH];
  int slot;
  AREA_DATA *pArea;
  if (!IS_IMMORTAL (ch))
    return FALSE;
  EDIT_AREA (ch, pArea);
  if (argument[0] != '\0')
    {
      if (!IS_IMMORTAL (ch))
	return FALSE;
      if (!str_cmp (argument, NOCLAN_NAME))
	{
	  pArea->clan = CLAN_BOGUS;
	  sprintf (buf, "This area no longer belongs to any clan.\n\r");
	  send_to_char (buf, ch);
	  return TRUE;
	}

      else
	{
	  slot = clanname_to_slot (argument);
	  if (slot == CLAN_BOGUS)
	    {
	      sprintf (buf, "There is no clan named %s``.\n\r",
		       capitalize (argument));
	      send_to_char (buf, ch);
	      send_to_char ("Clan ownership of this area unchanged.\n\r", ch);
	      return FALSE;
	    }
	  pArea->clan = slot;
	  sprintf (buf, "This area now belongs to clan %s``.\n\r",
		   get_clan_name (slot));
	  send_to_char (buf, ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax: clan <clan_name>\n\r", ch);
  return FALSE;
}

AEDIT (aedit_inconstruction)
{
  AREA_DATA *pArea;
  EDIT_AREA (ch, pArea);
  if (!str_cmp (argument, "no"))
    {
      pArea->construct = FALSE;
      return TRUE;
    }
  if (!str_cmp (argument, "yes"))
    {
      pArea->construct = TRUE;
      return TRUE;
    }
  send_to_char ("Syntax: inconstruction [yes|no]\n\r", ch);
  return FALSE;
}

AEDIT (aedit_affluence)
{
  AREA_DATA *pArea;
  char affluence[MAX_STRING_LENGTH];
  EDIT_AREA (ch, pArea);
  one_argument (argument, affluence);
  if (!is_number (affluence) || affluence[0] == '\0')
    {
      send_to_char ("Syntax:  affluence [#affluence]\n\r", ch);
      return FALSE;
    }
  if (ch->level < 99)
    {
      send_to_char
	("Sorry you aren't high enough level to set affluence.\n\r", ch);
      return FALSE;
    }
  pArea->points = atoi (affluence);
  send_to_char ("Affluence for this area set.\n\r", ch);
  return TRUE;
}


#if 0				/* ROM OLC */
AEDIT (aedit_recall)
{
  AREA_DATA *pArea;
  char room[MAX_STRING_LENGTH];
  int value;
  EDIT_AREA (ch, pArea);
  one_argument (argument, room);
  if (!is_number (argument) || argument[0] == '\0')
    {
      send_to_char ("Syntax:  recall [#rvnum]\n\r", ch);
      return FALSE;
    }
  value = atoi (room);
  if (!get_room_index (value))
    {
      send_to_char ("AEdit:  Room vnum does not exist.\n\r", ch);
      return FALSE;
    }
  pArea->recall = value;
  send_to_char ("Recall set.\n\r", ch);
  return TRUE;
}


#endif /* ROM OLC */
AEDIT (aedit_vnum)
{
  AREA_DATA *pArea;
  char lower[MAX_STRING_LENGTH];
  char upper[MAX_STRING_LENGTH];
  int ilower;
  int iupper;
  EDIT_AREA (ch, pArea);
  argument = one_argument (argument, lower);
  one_argument (argument, upper);
  if (!is_number (lower) || lower[0] == '\0'
      || !is_number (upper) || upper[0] == '\0')
    {
      send_to_char ("Syntax:  vnum [#lower] [#upper]\n\r", ch);
      return FALSE;
    }
  if ((ilower = atoi (lower)) > (iupper = atoi (upper)))
    {
      send_to_char ("AEdit:  Upper must be larger then lower.\n\r", ch);
      return FALSE;
    }

  /* OLC 1.1b */
  if (ilower <= 0 || ilower >= INT_MAX || iupper <= 0 || iupper >= INT_MAX)
    {
      char output[MAX_STRING_LENGTH];
      sprintf (output, "AEdit: vnum must be between 0 and %d.\n\r", INT_MAX);
      send_to_char (output, ch);
      return FALSE;
    }
  if (!check_range (atoi (lower), atoi (upper)))
    {
      send_to_char ("AEdit:  Range must include only this area.\n\r", ch);
      return FALSE;
    }
  if (get_vnum_area (ilower) && get_vnum_area (ilower) != pArea)
    {
      send_to_char ("AEdit:  Lower vnum already assigned.\n\r", ch);
      return FALSE;
    }
  pArea->lvnum = ilower;
  send_to_char ("Lower vnum set.\n\r", ch);
  if (get_vnum_area (iupper) && get_vnum_area (iupper) != pArea)
    {
      send_to_char ("AEdit:  Upper vnum already assigned.\n\r", ch);
      return TRUE;		/* The lower value has been set. */
    }
  pArea->uvnum = iupper;
  send_to_char ("Upper vnum set.\n\r", ch);
  return TRUE;
}

AEDIT (aedit_lev)
{
  AREA_DATA *pArea;
  char lower[MAX_STRING_LENGTH];
  char upper[MAX_STRING_LENGTH];
  int ilower;
  int iupper;
  EDIT_AREA (ch, pArea);
  argument = one_argument (argument, lower);
  one_argument (argument, upper);
  if (!is_number (lower) || lower[0] == '\0'
      || !is_number (upper) || upper[0] == '\0')
    {
      send_to_char ("Syntax:  lev [#lower] [#upper]\n\r", ch);
      return FALSE;
    }
  if ((ilower = atoi (lower)) > (iupper = atoi (upper)))
    {
      send_to_char ("Upper level must be larger then lower level.\n\r", ch);
      return FALSE;
    }
  if (ilower < 1 || ilower > MAX_LEVEL || iupper < 1 || iupper > MAX_LEVEL)
    {
      char output[MAX_STRING_LENGTH];
      sprintf (output, "Level must be between 1 and %d.\n\r", MAX_LEVEL);
      send_to_char (output, ch);
      return FALSE;
    }
  pArea->llev = ilower;
  send_to_char ("Lower level set.\n\r", ch);
  pArea->ulev = iupper;
  send_to_char ("Upper level set.\n\r", ch);
  return TRUE;
}

AEDIT (aedit_llev)
{
  AREA_DATA *pArea;
  char lower[MAX_STRING_LENGTH];
  int ilower;
  int iupper;
  EDIT_AREA (ch, pArea);
  one_argument (argument, lower);
  if (!is_number (lower) || lower[0] == '\0')
    {
      send_to_char ("Syntax:  llev [#lower]\n\r", ch);
      return FALSE;
    }
  if ((ilower = atoi (lower)) > (iupper = pArea->ulev))
    {
      send_to_char ("Value must be less than the upper level.\n\r", ch);
      return FALSE;
    }
  if (ilower < 1 || ilower > MAX_LEVEL || iupper < 1 || iupper > MAX_LEVEL)
    {
      char output[MAX_STRING_LENGTH];
      sprintf (output, "Level must be between 1 and %d.\n\r", MAX_LEVEL);
      send_to_char (output, ch);
      return FALSE;
    }
  pArea->llev = ilower;
  send_to_char ("Lower level set.\n\r", ch);
  return TRUE;
}

AEDIT (aedit_lvnum)
{
  AREA_DATA *pArea;
  char lower[MAX_STRING_LENGTH];
  int ilower;
  int iupper;
  EDIT_AREA (ch, pArea);
  one_argument (argument, lower);
  if (!is_number (lower) || lower[0] == '\0')
    {
      send_to_char ("Syntax:  lvnum [#lower]\n\r", ch);
      return FALSE;
    }
  if ((ilower = atoi (lower)) > (iupper = pArea->uvnum))
    {
      send_to_char ("AEdit:  Value must be less than the uvnum.\n\r", ch);
      return FALSE;
    }

  /* OLC 1.1b */
  if (ilower <= 0 || ilower >= INT_MAX || iupper <= 0 || iupper >= INT_MAX)
    {
      char output[MAX_STRING_LENGTH];
      sprintf (output, "AEdit: vnum must be between 0 and %d.\n\r", INT_MAX);
      send_to_char (output, ch);
      return FALSE;
    }
  if (!check_range (ilower, iupper))
    {
      send_to_char ("AEdit:  Range must include only this area.\n\r", ch);
      return FALSE;
    }
  if (get_vnum_area (ilower) && get_vnum_area (ilower) != pArea)
    {
      send_to_char ("AEdit:  Lower vnum already assigned.\n\r", ch);
      return FALSE;
    }
  pArea->lvnum = ilower;
  send_to_char ("Lower vnum set.\n\r", ch);
  return TRUE;
}

AEDIT (aedit_ulev)
{
  AREA_DATA *pArea;
  char upper[MAX_STRING_LENGTH];
  int ilower;
  int iupper;
  EDIT_AREA (ch, pArea);
  one_argument (argument, upper);
  if (!is_number (upper) || upper[0] == '\0')
    {
      send_to_char ("Syntax:  ulev [#upper]\n\r", ch);
      return FALSE;
    }
  if ((ilower = pArea->llev) > (iupper = atoi (upper)))
    {
      send_to_char ("Upper level must be larger then lower level.\n\r", ch);
      return FALSE;
    }
  if (ilower < 1 || ilower > MAX_LEVEL || iupper < 1 || iupper > MAX_LEVEL)
    {
      char output[MAX_STRING_LENGTH];
      sprintf (output, "Level must be between 0 and %d.\n\r", MAX_LEVEL);
      send_to_char (output, ch);
      return FALSE;
    }
  pArea->ulev = iupper;
  send_to_char ("Upper level set.\n\r", ch);
  return TRUE;
}

AEDIT (aedit_helper)
{
  AREA_DATA *pArea;
  EDIT_AREA (ch, pArea);
  if (argument[0] == '\0')
    {
      free_string (pArea->helper);
      pArea->helper = str_dup ("");
      send_to_char ("Helper delete.\n\r", ch);
      return TRUE;
    }
  free_string (pArea->helper);
  pArea->helper = str_dup (argument);
  send_to_char ("Area helper set.\n\r", ch);
  return TRUE;
}

AEDIT (aedit_uvnum)
{
  AREA_DATA *pArea;
  char upper[MAX_STRING_LENGTH];
  int ilower;
  int iupper;
  EDIT_AREA (ch, pArea);
  one_argument (argument, upper);
  if (!is_number (upper) || upper[0] == '\0')
    {
      send_to_char ("Syntax:  uvnum [#upper]\n\r", ch);
      return FALSE;
    }
  if ((ilower = pArea->lvnum) > (iupper = atoi (upper)))
    {
      send_to_char ("AEdit:  Upper must be larger then lower.\n\r", ch);
      return FALSE;
    }

  /* OLC 1.1b */
  if (ilower <= 0 || ilower >= INT_MAX || iupper <= 0 || iupper >= INT_MAX)
    {
      char output[MAX_STRING_LENGTH];
      sprintf (output, "AEdit: vnum must be between 0 and %d.\n\r", INT_MAX);
      send_to_char (output, ch);
      return FALSE;
    }
  if (!check_range (ilower, iupper))
    {
      send_to_char ("AEdit:  Range must include only this area.\n\r", ch);
      return FALSE;
    }
  if (get_vnum_area (iupper) && get_vnum_area (iupper) != pArea)
    {
      send_to_char ("AEdit:  Upper vnum already assigned.\n\r", ch);
      return FALSE;
    }
  pArea->uvnum = iupper;
  send_to_char ("Upper vnum set.\n\r", ch);
  return TRUE;
}

REDIT (redit_rate)
{
  ROOM_INDEX_DATA *pRoom;
  char cmd[MAX_INPUT_LENGTH], val[MAX_INPUT_LENGTH];
  EDIT_ROOM (ch, pRoom);
  argument = one_argument (argument, cmd);
  if (!str_cmp (cmd, "heal"))
    {
      argument = one_argument (argument, val);
      if (!is_number (val))
	{
	  send_to_char ("Syntax: rate heal [value]\n\r", ch);
	  return FALSE;
	}
      pRoom->heal_rate = atoi (val);
      send_to_char ("Heal rate set.\n\r", ch);
      return TRUE;
    }

  else if (!str_cmp (cmd, "mana"))
    {
      argument = one_argument (argument, val);
      if (!is_number (val))
	{
	  send_to_char ("Syntax: rate mana [value]\n\r", ch);
	  return FALSE;
	}
      pRoom->mana_rate = atoi (val);
      send_to_char ("Mana rate set.\n\r", ch);
    }

  else
    {
      send_to_char ("Syntax: rate [heal|mana] [value]\n\r", ch);
      return FALSE;
    }
  return FALSE;
}

REDIT (redit_maxnum)
{
  ROOM_INDEX_DATA *pRoom;
  int value;
  EDIT_ROOM (ch, pRoom);
  if (!is_number (argument))
    {
      send_to_char ("Syntax: maxnum [value]\n\r", ch);
      return FALSE;
    }
  value = atoi (argument);
  if (value < 0 || value > 100)
    {
      send_to_char
	("Max number of characters must be between 0 and 100.\n\r", ch);
      return FALSE;
    }
  pRoom->max_in_room = value;
  send_to_char ("Maximum character number for room set.\n\r", ch);
  return TRUE;
}

REDIT (redit_level)
{
  ROOM_INDEX_DATA *pRoom;
  int value;
  EDIT_ROOM (ch, pRoom);
  if (!is_number (argument))
    {
      send_to_char ("Syntax: level [value]\n\r", ch);
      return FALSE;
    }
  value = atoi (argument);
  if (value < 0 || value > 100)
    {
      send_to_char ("Level must be between 0 and 100.\n\r", ch);
      return FALSE;
    }
  pRoom->max_level = value;
  send_to_char ("Maximum level for room set.\n\r", ch);
  return TRUE;
}

REDIT (redit_show)
{
  ROOM_INDEX_DATA *pRoom;
  char buf[MAX_STRING_LENGTH];
  char buf1[3 * MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  CHAR_DATA *rch;
  int door;
  bool fcnt;
  char *muff;
  EDIT_ROOM (ch, pRoom);
  buf1[0] = '\0';
  sprintf (buf, "Description:\n\r%s", pRoom->description);
  strcat (buf1, buf);
  sprintf (buf, "Name:       [%s]\n\rArea:       [%5d] %s\n\r",
	   pRoom->name, pRoom->area->vnum, pRoom->area->name);
  strcat (buf1, buf);
  sprintf (buf, "Vnum:       [%5d]\n\rSector:     [%s]\n\r",
	   pRoom->vnum, flag_string (sector_flags, pRoom->sector_type));
  strcat (buf1, buf);
  muff = str_dup (flag_string (room_flags2, pRoom->room_flags2));
  sprintf (buf, "Room flags: [%s %s%s%s%s]\n\r",
	   flag_string (room_flags, pRoom->room_flags), muff,
	   IS_SET (pRoom->race_flags,
		   ROOM_NOTRANSPORT) ? " no_transport" : "",
	   IS_SET (pRoom->race_flags,
		   ROOM_PLAYERSTORE) ? " player_store" : "",
	   IS_SET (pRoom->race_flags, ROOM_NOTELEPORT) ? " no_teleport" : "");
  strcat (buf1, buf);
  if (pRoom->clan == CLAN_BOGUS)
    {
      sprintf (buf, "Room Clan:  [%s]\n\r", NOCLAN_NAME);
      strcat (buf1, buf);
    }

  else
    {
      sprintf (buf, "Room Clan:  [%s``]\n\r", get_clan_name (pRoom->clan));
      strcat (buf1, buf);
    }
  if (pRoom->owner != NULL)
    {
      sprintf (buf, "Room Owner: [%s]\n\r", pRoom->owner);
      strcat (buf1, buf);
    }

  else
    {
      sprintf (buf, "Room Owner: [NULL]\n\r");
      strcat (buf1, buf);
    }
  sprintf (buf, "Explore Exp: [%ld]\n\r", pRoom->tp_exp);
  strcat (buf1, buf);
  sprintf (buf, "Explore Max Level: [%d]\n\r", pRoom->tp_level);
  strcat (buf1, buf);
  if (pRoom->tp_exp > 0)
    {
      sprintf (buf, "Explore Msg: [%s]\n\r", pRoom->tp_msg);
      strcat (buf1, buf);
    }
  sprintf (buf, "Enter msg: [%s]\n\r", pRoom->enter_msg);
  strcat (buf1, buf);
  sprintf (buf, "Exit msg: [%s]\n\r", pRoom->exit_msg);
  strcat (buf1, buf);
  sprintf (buf, "MaxNum of Characters: [%d]\n\r", pRoom->max_in_room);
  strcat (buf1, buf);
  sprintf (buf, "MaxMsg:  [%s]\n\r", pRoom->max_message);
  strcat (buf1, buf);
  sprintf (buf, "ManaRate: [%d]   HealRate: [%d]\n\r", pRoom->mana_rate,
	   pRoom->heal_rate);
  strcat (buf1, buf);
  sprintf (buf, "Classes Allowed: [%s]\n\r",
	   crflag_string (Class_flags, pRoom->Class_flags));
  strcat (buf1, buf);
  sprintf (buf, "Races Allowed:   [%s]\n\r",
	   crflag_string (pcrace_flags, pRoom->race_flags));
  strcat (buf1, buf);

  // Adeon -- Show sink stuff if room has been flagged sinking
  if (IS_SET (pRoom->room_flags2, ROOM_SINKING))
    {
      sprintf (buf, "Sink Destination Vnum: [%d]\n\r", pRoom->sink_dest);
      strcat (buf1, buf);
      sprintf (buf, "Sink Timer: [%d]\n\r", pRoom->sink_timer);
      strcat (buf1, buf);
      sprintf (buf, "Sink Warning: [%s]\n\r", pRoom->sink_warning);
      strcat (buf1, buf);
      sprintf (buf, "Sink Message: [%s]\n\r", pRoom->sink_msg);
      strcat (buf1, buf);
      sprintf (buf, "Sink Message to Others: [%s]\n\r", pRoom->sink_msg_others);
      strcat (buf1, buf);
    }
  if (pRoom->max_level == 0)
    strcpy (buf, "Maximum Player Level: [unrestricted]\n\r");

  else
    sprintf (buf, "Maximum Player Level: [%d]\n\r", pRoom->max_level);
  strcat (buf1, buf);
  if (pRoom->extra_descr)
    {
      EXTRA_DESCR_DATA *ed;
      strcat (buf1, "Desc Kwds:  [");
      for (ed = pRoom->extra_descr; ed; ed = ed->next)
	{
	  strcat (buf1, ed->keyword);
	  if (ed->next)
	    strcat (buf1, " ");
	}
      strcat (buf1, "]\n\r");
    }
  strcat (buf1, "Characters: [");
  fcnt = FALSE;
  for (rch = pRoom->people; rch; rch = rch->next_in_room)
    {
      one_argument (rch->name, buf);
      strcat (buf1, buf);
      strcat (buf1, " ");
      fcnt = TRUE;
    }
  if (fcnt)
    {
      int end;
      end = strlen (buf1) - 1;
      buf1[end] = ']';
      strcat (buf1, "\n\r");
    }

  else
    strcat (buf1, "none]\n\r");
  strcat (buf1, "Objects:    [");
  fcnt = FALSE;
  for (obj = pRoom->contents; obj; obj = obj->next_content)
    {
      one_argument (obj->name, buf);
      strcat (buf1, buf);
      strcat (buf1, " ");
      fcnt = TRUE;
    }
  if (fcnt)
    {
      int end;
      end = strlen (buf1) - 1;
      buf1[end] = ']';
      strcat (buf1, "\n\r");
    }

  else
    strcat (buf1, "none]\n\r");
  for (door = 0; door < MAX_DIR; door++)
    {
      EXIT_DATA *pexit;
      if ((pexit = pRoom->exit[door]))
	{
	  char reset_state[MAX_STRING_LENGTH];
	  sprintf (buf, "-%-5s to [%5d] Key: [%5d]",
		   capitalize (dir_name[door]),
		   pexit->u1.to_room ? pexit->u1.to_room->vnum : 0,
		   /* ROM OLC */
		   pexit->key);
	  strcat (buf1, buf);

	  /*
	   * Format up the exit info.
	   * Capitalize all flags that are not part of the reset info.
	   */
	  strcpy (reset_state, flag_string (lock_flags, pexit->rs_flags));
	  strcat (buf1, " Lock Flags: [");
	  strcat (buf1, reset_state);
	  strcat (buf1, "]\n\r");
	  if (pexit->keyword && pexit->keyword[0] != '\0')
	    {
	      sprintf (buf, "Kwds: [%s]\n\r", pexit->keyword);
	      strcat (buf1, buf);
	    }
	  if (pexit->description && pexit->description[0] != '\0')
	    {
	      sprintf (buf, "%s", pexit->description);
	      strcat (buf1, buf);
	    }
	}
    }
  send_to_char (buf1, ch);
  return FALSE;
}


/* Local function. */
bool change_exit (CHAR_DATA * ch, char *argument, int door)
{
  ROOM_INDEX_DATA *pRoom, *linkroom;
  RESET_DATA *pReset;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int value, pFound = FALSE;
  EDIT_ROOM (ch, pRoom);

  /*
   * Now parse the arguments.
   */
  argument = one_argument (argument, command);
  one_argument (argument, arg);
  if (command[0] == '\0' && argument[0] == '\0')
    {				/* Move command. */
      move_char (ch, door, TRUE);	/* ROM OLC */
      return FALSE;
    }
  if (command[0] == '?')
    {
      do_help (ch, "EXIT");
      return FALSE;
    }
  if (!str_cmp (command, "delete"))
    {
      ROOM_INDEX_DATA *pToRoom;
      sh_int rev;
      if (!pRoom->exit[door])
	{
	  send_to_char ("Cannot delete a null exit.\n\r", ch);
	  return FALSE;
	}

      /*
       * Remove ToRoom Exit.
       */
      rev = rev_dir[door];
      pToRoom = pRoom->exit[door]->u1.to_room;
      if (pToRoom->exit[rev])
	{
	  free_exit (pToRoom->exit[rev]);
	  pToRoom->exit[rev] = NULL;
	  save_area (pToRoom->area);
	}
      free_exit (pRoom->exit[door]);
      pRoom->exit[door] = NULL;
      send_to_char ("Exit unlinked.\n\r", ch);
      return TRUE;
    }
  if (!str_cmp (command, "link"))
    {
      EXIT_DATA *pExit;
      if (arg[0] == '\0' || !is_number (arg))
	{
	  send_to_char ("Syntax:  [direction] link [vnum]\n\r", ch);
	  return FALSE;
	}
      value = atoi (arg);
      if ((linkroom = get_room_index (value)) == NULL)
	{
	  send_to_char ("Cannot link to non-existant room.\n\r", ch);
	  return FALSE;
	}
      if (!is_builder (ch, linkroom->area))
	{
	  send_to_char
	    ("You can't link into areas you have not created.\n\r", ch);
	  return FALSE;
	}
      if (pRoom->exit[door] != NULL)
	{
	  send_to_char
	    ("You should remove the current link in this direction.", ch);
	  return FALSE;
	}
      if (get_room_index (value)->exit[rev_dir[door]])
	{
	  send_to_char ("Remote side's exit already exists.\n\r", ch);
	  return FALSE;
	}
      if (!pRoom->exit[door])
	{
	  pRoom->exit[door] = new_exit ();
	}
      pRoom->exit[door]->u1.to_room = get_room_index (value);
      pRoom->exit[door]->orig_door = door;
      pRoom = get_room_index (value);
      door = rev_dir[door];
      pExit = new_exit ();
      pExit->u1.to_room = ch->in_room;
      pExit->orig_door = door;
      pRoom->exit[door] = pExit;
      send_to_char ("Two-way link established.\n\r", ch);
      return TRUE;
    }
  if (!str_cmp (command, "dig"))
    {
      char buf[MAX_STRING_LENGTH];
      if (arg[0] == '\0' || !is_number (arg))
	{
	  char tcbuf[MAX_STRING_LENGTH];
	  int next_vnum;
	  next_vnum = get_unused_vnum (ch->in_room->area);
	  sprintf (tcbuf, "Using default vnum of %d\n\r", next_vnum);
	  send_to_char (tcbuf, ch);
	  sprintf (arg, "%d", next_vnum);
	}
      redit_create (ch, arg);
      sprintf (buf, "link %s", arg);
      change_exit (ch, buf, door);
      return TRUE;
    }
  if (!str_cmp (command, "room"))
    {
      if (arg[0] == '\0' || !is_number (arg))
	{
	  send_to_char ("Syntax:  [direction] room [vnum]\n\r", ch);
	  return FALSE;
	}
      if (!pRoom->exit[door])
	{
	  pRoom->exit[door] = new_exit ();
	}
      value = atoi (arg);
      if (!get_room_index (value))
	{
	  send_to_char ("Cannot link to non-existant room.\n\r", ch);
	  return FALSE;
	}
      pRoom->exit[door]->u1.to_room = get_room_index (value);	/* ROM OLC */

/*      pRoom->exit[door]->vnum = value;                 Can't set vnum in ROM */
      pRoom->exit[door]->orig_door = door;
      send_to_char ("One-way link established.\n\r", ch);
      return TRUE;
    }
  if (!str_cmp (command, "key"))
    {
      if (arg[0] == '\0' || !is_number (arg))
	{
	  send_to_char ("Syntax:  [direction] key [vnum]\n\r", ch);
	  return FALSE;
	}
      if (!pRoom->exit[door])
	{
	  pRoom->exit[door] = new_exit ();
	}
      value = atoi (arg);
      if (!get_obj_index (value))
	{
	  send_to_char ("Item doesn't exist.\n\r", ch);
	  return FALSE;
	}
      if (get_obj_index (atoi (argument))->item_type != ITEM_KEY
	  && get_obj_index (atoi (argument))->item_type != ITEM_ROOM_KEY)
	{
	  send_to_char ("Key doesn't exist.\n\r", ch);
	  return FALSE;
	}
      pRoom->exit[door]->key = value;
      send_to_char ("Exit key set.\n\r", ch);
      return TRUE;
    }
  if (!str_cmp (command, "name"))
    {
      if (arg[0] == '\0')
	{
	  send_to_char ("Syntax:  [direction] name [string]\n\r", ch);
	  return FALSE;
	}
      if (!pRoom->exit[door])
	{
	  pRoom->exit[door] = new_exit ();
	}
      free_string (pRoom->exit[door]->keyword);
      pRoom->exit[door]->keyword = str_dup (arg);
      send_to_char ("Exit name set.\n\r", ch);
      return TRUE;
    }
  if (!str_prefix (command, "description"))
    {
      if (arg[0] == '\0')
	{
	  if (!pRoom->exit[door])
	    {
	      pRoom->exit[door] = new_exit ();
	    }
	  send_to_char
	    ("Please use the Exodus Editor to enter the text of the direction description:\n\r\n\r",
	     ch);
	  edit_string (ch, &pRoom->exit[door]->description);
	  return TRUE;
	}
      send_to_char ("Syntax:  [direction] desc\n\r", ch);
      return FALSE;
    }
  if (!str_cmp (command, "reset"))
    {
      int dreset;
      if (!str_cmp (arg, "open"))
	{
	  dreset = 0;
	}

      else if (!str_cmp (arg, "closed"))
	{
	  dreset = 1;
	}

      else if (!str_cmp (arg, "locked"))
	{
	  dreset = 2;
	}

      else
	{
	  send_to_char
	    ("Syntax: direction reset [open|closed|locked]\n\r", ch);
	  return FALSE;
	}
      if (pRoom->exit[door] == NULL || pRoom->exit[door]->u1.to_room == NULL)
	{
	  send_to_char
	    ("Doors must be set up on both sides before resets can be added.\n\r",
	     ch);
	  return FALSE;
	}
      if (!IS_SET (pRoom->exit[door]->rs_flags, EX_ISDOOR))
	SET_BIT (pRoom->exit[door]->rs_flags, EX_ISDOOR);
      if (!IS_SET (pRoom->exit[door]->exit_info, EX_ISDOOR))
	SET_BIT (pRoom->exit[door]->exit_info, EX_ISDOOR);
      if (pRoom->exit[door]->keyword[0] == '\0')
	{
	  free_string (pRoom->exit[door]->keyword);
	  pRoom->exit[door]->keyword = str_dup ("door");
	}
      if (pRoom->exit[door]->u1.to_room->exit[rev_dir[door]] != NULL)
	if (pRoom->exit[door]->u1.to_room->exit[rev_dir[door]]->
	    keyword[0] == '\0')
	  {
	    free_string (pRoom->exit[door]->u1.to_room->
			 exit[rev_dir[door]]->keyword);
	    pRoom->exit[door]->u1.to_room->exit[rev_dir[door]]->
	      keyword = str_dup ("door");
	  }
      for (pReset = pRoom->area->reset_first; pReset; pReset = pReset->next)
	{
	  if (pReset->command == 'D' && pReset->arg1 == pRoom->vnum
	      && pReset->arg2 == door)
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

/*	  pReset->arg1 = pRoom->vnum;
*/ pReset->arg1 = ch->in_room->vnum;
	  pReset->arg2 = door;
	  pReset->arg3 = dreset;
	  add_reset (ch->in_room /*pRoom */ , pReset, 0);
	}
      pRoom = pRoom->exit[door]->u1.to_room;
      door = rev_dir[door];
      pFound = FALSE;
      if (pRoom != NULL)
	{
	  for (pReset = pRoom->area->reset_first; pReset;
	       pReset = pReset->next)
	    {
	      if (pReset->command == 'D' && pReset->arg1 == pRoom->vnum
		  && pReset->arg2 == door)
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
	      pReset->arg1 = pRoom->vnum;

	      /*Edited by Cory 6/28/03 -- This is the opposite end of the exit it
	         should NOT reset to ch->in_room!  Should be resetting to the pRoom!    
	         pReset->arg1 = ch->in_room->vnum; */
	      pReset->arg2 = door;
	      pReset->arg3 = dreset;
	      add_reset (pRoom, pReset, 0);
	    }
	}
      if (!pRoom)
      {
	      bug("pRoom, line 1706 of olc_act.c, is NULL.  This caused the most recent crash.",0);
	      send_to_char("ERROR! Talk to Iblis.\n\r",ch);
	      do_immtalk(ch,"I just caused pRoom to be NULL in olc_act.c  Someone tell Iblis.");
	      return FALSE;
      }
      if (!pRoom->exit[door])
      {
	      bug("pRoom->exit[door], line 1706 of olc_act.c, is NULL.  This caused the most recent crash.",0);
              send_to_char("ERROR! Talk to Iblis.\n\r",ch);
              do_immtalk(ch,"I just caused pRoom->exit[door] to be NULL in olc_act.c  Someone tell Iblis.");
              return FALSE;
      }
      
      if (!IS_SET (pRoom->exit[door]->rs_flags, EX_ISDOOR))
	SET_BIT (pRoom->exit[door]->rs_flags, EX_ISDOOR);
      if (!IS_SET (pRoom->exit[door]->exit_info, EX_ISDOOR))
	SET_BIT (pRoom->exit[door]->exit_info, EX_ISDOOR);
      send_to_char ("Reset enabled.  ", ch);
      switch (dreset)
	{
	case 0:
	  send_to_char ("Exit will reset to open\n\r", ch);
	  break;
	case 1:
	  send_to_char ("Exit will reset to closed\n\r", ch);
	  break;
	case 2:
	  send_to_char ("Exit will reset to locked\n\r", ch);
	  break;
	}
      return TRUE;
    }
  if (!str_cmp (command, "lock"))
    {
      if (pRoom->exit[door] == NULL || pRoom->exit[door]->u1.to_room == NULL)
	{
	  send_to_char
	    ("There isn't currently an exit in that direction.\n\r", ch);
	  return (FALSE);
	}
      if ((value = flag_value (lock_flags, argument)) != NO_FLAG)
	{
	  ROOM_INDEX_DATA *pToRoom;
	  sh_int rev;
	  if (!pRoom->exit[door])
	    pRoom->exit[door] = new_exit ();
	  TOGGLE_BIT (pRoom->exit[door]->rs_flags, value);
	  TOGGLE_BIT (pRoom->exit[door]->exit_info, value);
	  if (pRoom->exit[door]->keyword[0] == '\0')
	    {
	      free_string (pRoom->exit[door]->keyword);
	      pRoom->exit[door]->keyword = str_dup ("door");
	    }
	  pToRoom = pRoom->exit[door]->u1.to_room;
	  if (pToRoom != NULL)
	    {
	      rev = rev_dir[door];
	      if (pToRoom->exit[rev] != NULL)
		{
		  pToRoom->exit[rev]->rs_flags = pRoom->exit[door]->rs_flags;
		  pToRoom->exit[rev]->exit_info =
		    pRoom->exit[door]->exit_info;
		  if (pToRoom->exit[rev]->keyword[0] == '\0')
		    {
		      free_string (pToRoom->exit[rev]->keyword);
		      pToRoom->exit[rev]->keyword = str_dup ("door");
		    }
		  save_area (pToRoom->area);
		}
	    }
	  send_to_char ("Lock flag toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Command not understood.\n\r", ch);
  return FALSE;
}

int get_unused_vnum (AREA_DATA * pArea)
{
  int ret_vnum;
  ret_vnum = pArea->lvnum;
  if (ret_vnum == 0)
    ret_vnum = 1;
  while (get_room_index (ret_vnum) != NULL)
    ret_vnum++;
  return (ret_vnum);
}

void do_ocopy (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  OBJ_INDEX_DATA *source_obj, *dest_obj;
  int src_vnum, dest_vnum, iHash;
  AREA_DATA *pArea;
  if (!IS_BUILDER (ch))
    {
      send_to_char ("You do not have authorization to do that.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (!is_number (arg1) || !is_number (arg2))
    {
      send_to_char ("Syntax: ocopy <source-vnum> <destination-vnum>\n\r", ch);
      return;
    }
  src_vnum = atoi (arg1);
  dest_vnum = atoi (arg2);
  source_obj = get_obj_index (src_vnum);
  if (source_obj == NULL)
    {
      sprintf (tcbuf, "Vnum %d does not currently exist.\n\r", src_vnum);
      send_to_char (tcbuf, ch);
      return;
    }
  if (get_obj_index (dest_vnum) != NULL)
    {
      sprintf (tcbuf, "Obj vnum %d already exists.\n\r", dest_vnum);
      send_to_char (tcbuf, ch);
      return;
    }
  pArea = get_vnum_area (dest_vnum);
  if (pArea == NULL)
    {
      sprintf (tcbuf, "Obj vnum %d is not assigned to an area.\n\r",
	       dest_vnum);
      send_to_char (tcbuf, ch);
      return;
    }
  if (!is_builder (ch, pArea))
    {
      send_to_char
	("You cannot use that destination vnum as you are not that area's creator.\n\r",
	 ch);
      return;
    }
  dest_obj = copy_obj_index (source_obj);
  dest_obj->vnum = dest_vnum;
  dest_obj->area = pArea;
  if (dest_vnum > top_vnum_obj)
    top_vnum_obj = dest_vnum;
  iHash = dest_vnum % MAX_KEY_HASH;
  dest_obj->next = obj_index_hash[iHash];
  obj_index_hash[iHash] = dest_obj;
  send_to_char ("Object copied.\n\r", ch);
  return;
}

void do_mcopy (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  MOB_INDEX_DATA *source_mob, *dest_mob;
  int src_vnum, dest_vnum, iHash;
  AREA_DATA *pArea;
  if (!IS_BUILDER (ch))
    {
      send_to_char ("You do not have authorization to do that.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (!is_number (arg1) || !is_number (arg2))
    {
      send_to_char ("Syntax: mcopy <source-vnum> <destination-vnum>\n\r", ch);
      return;
    }
  src_vnum = atoi (arg1);
  dest_vnum = atoi (arg2);
  source_mob = get_mob_index (src_vnum);
  if (source_mob == NULL)
    {
      sprintf (tcbuf, "Mob vnum %d does not currently exist.\n\r", src_vnum);
      send_to_char (tcbuf, ch);
      return;
    }
  if (get_mob_index (dest_vnum) != NULL)
    {
      sprintf (tcbuf, "Mob vnum %d already exists.\n\r", dest_vnum);
      send_to_char (tcbuf, ch);
      return;
    }
  pArea = get_vnum_area (dest_vnum);
  if (pArea == NULL)
    {
      sprintf (tcbuf, "Vnum %d is not assigned to an area.\n\r", dest_vnum);
      send_to_char (tcbuf, ch);
      return;
    }
  if (!is_builder (ch, pArea))
    {
      send_to_char
	("You cannot use that destination vnum as you are not that area's creator.\n\r",
	 ch);
      return;
    }
  dest_mob = copy_mob_index (source_mob);
  dest_mob->vnum = dest_vnum;
  dest_mob->area = pArea;
  if (dest_vnum > top_vnum_mob)
    top_vnum_mob = dest_vnum;
  iHash = dest_vnum % MAX_KEY_HASH;
  dest_mob->next = mob_index_hash[iHash];
  mob_index_hash[iHash] = dest_mob;
  send_to_char ("Mobile copied.\n\r", ch);
  return;
}

void do_rcopy (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  ROOM_INDEX_DATA *source_room, *dest_room;
  int src_vnum, dest_vnum, iHash;
  AREA_DATA *pArea;
  if (!IS_BUILDER (ch))
    {
      send_to_char ("You do not have authorization to do that.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (!is_number (arg1))
    {
      send_to_char ("Syntax: rcopy <source-vnum> <destination-vnum>\n\r", ch);
      return;
    }
  src_vnum = atoi (arg1);
  source_room = get_room_index (src_vnum);
  if (source_room == NULL)
    {
      sprintf (tcbuf, "Room vnum %d does not currently exist.\n\r", src_vnum);
      send_to_char (tcbuf, ch);
      return;
    }
  if (arg2[0] == '\0')
    {
      dest_vnum = get_unused_vnum (source_room->area);
      sprintf (tcbuf, "Using default destination vnum of %d\n\r", dest_vnum);
      send_to_char (tcbuf, ch);
    }

  else
    {
      if (!is_number (arg2))
	{
	  send_to_char
	    ("Syntax: rcopy <source-vnum> <destination-vnum>\n\r", ch);
	  return;
	}
      dest_vnum = atoi (arg2);
    }
  if (dest_vnum < 1)
    {
      send_to_char ("Invalid destination vnum.\n\r", ch);
      return;
    }
  if (get_room_index (dest_vnum) != NULL)
    {
      sprintf (tcbuf, "Room vnum %d already exists.\n\r", dest_vnum);
      send_to_char (tcbuf, ch);
      return;
    }
  pArea = get_vnum_area (dest_vnum);
  if (pArea == NULL)
    {
      sprintf (tcbuf, "Vnum %d is not assigned to an area.\n\r", dest_vnum);
      send_to_char (tcbuf, ch);
      return;
    }
  if (!is_builder (ch, pArea))
    {
      send_to_char
	("You cannot use that destination vnum as you are not that area's creator.\n\r",
	 ch);
      return;
    }
  dest_room = copy_room_index (source_room);
  dest_room->vnum = dest_vnum;
  dest_room->area = pArea;
  if (dest_vnum > top_vnum_room)
    top_vnum_room = dest_vnum;
  iHash = dest_vnum % MAX_KEY_HASH;
  dest_room->next = room_index_hash[iHash];
  room_index_hash[iHash] = dest_room;
  send_to_char ("Room copied.\n\r", ch);
  return;
}

REDIT (redit_north)
{
  if (change_exit (ch, argument, DIR_NORTH))
    return TRUE;
  return FALSE;
}

REDIT (redit_south)
{
  if (change_exit (ch, argument, DIR_SOUTH))
    return TRUE;
  return FALSE;
}

REDIT (redit_east)
{
  if (change_exit (ch, argument, DIR_EAST))
    return TRUE;
  return FALSE;
}

REDIT (redit_west)
{
  if (change_exit (ch, argument, DIR_WEST))
    return TRUE;
  return FALSE;
}

REDIT (redit_up)
{
  if (change_exit (ch, argument, DIR_UP))
    return TRUE;
  return FALSE;
}

REDIT (redit_down)
{
  if (change_exit (ch, argument, DIR_DOWN))
    return TRUE;
  return FALSE;
}

REDIT (redit_ed)
{
  ROOM_INDEX_DATA *pRoom;
  EXTRA_DESCR_DATA *ed;
  char command[MAX_INPUT_LENGTH];
  char keyword[MAX_INPUT_LENGTH];
  EDIT_ROOM (ch, pRoom);
  argument = one_argument (argument, command);
  one_argument (argument, keyword);
  if (command[0] == '\0' || keyword[0] == '\0')
    {
      send_to_char ("Syntax:  ed add [keyword]\n\r", ch);
      send_to_char ("         ed edit [keyword]\n\r", ch);
      send_to_char ("         ed delete [keyword]\n\r", ch);
      send_to_char ("         ed format [keyword]\n\r", ch);
      return FALSE;
    }
  if (!str_cmp (command, "add"))
    {
      if (keyword[0] == '\0')
	{
	  send_to_char ("Syntax:  ed add [keyword]\n\r", ch);
	  return FALSE;
	}
      ed = new_extra_descr ();
      ed->keyword = str_dup (keyword);
      ed->description = str_dup ("");
      ed->next = pRoom->extra_descr;
      pRoom->extra_descr = ed;
      send_to_char
	("Please use the Exodus Editor to enter the keyword description:\n\r\n\r",
	 ch);
      edit_string (ch, &ed->description);
      return TRUE;
    }
  if (!str_cmp (command, "edit"))
    {
      if (keyword[0] == '\0')
	{
	  send_to_char ("Syntax:  ed edit [keyword]\n\r", ch);
	  return FALSE;
	}
      for (ed = pRoom->extra_descr; ed; ed = ed->next)
	{
	  if (is_name (keyword, ed->keyword))
	    break;
	}
      if (!ed)
	{
	  send_to_char ("Extra description keyword not found.\n\r", ch);
	  return FALSE;
	}
      send_to_char
	("Please use the Exodus Editor to enter the extra description:\n\r\n\r",
	 ch);
      edit_string (ch, &ed->description);
      return TRUE;
    }
  if (!str_cmp (command, "delete"))
    {
      EXTRA_DESCR_DATA *ped = NULL;
      if (keyword[0] == '\0')
	{
	  send_to_char ("Syntax:  ed delete [keyword]\n\r", ch);
	  return FALSE;
	}
      for (ed = pRoom->extra_descr; ed; ed = ed->next)
	{
	  if (is_name (keyword, ed->keyword))
	    break;
	  ped = ed;
	}
      if (!ed)
	{
	  send_to_char ("Extra description keyword not found.\n\r", ch);
	  return FALSE;
	}
      if (!ped)
	pRoom->extra_descr = ed->next;

      else
	ped->next = ed->next;
      free_extra_descr (ed);
      send_to_char ("Extra description deleted.\n\r", ch);
      return TRUE;
    }
  if (!str_cmp (command, "format"))
    {
      if (keyword[0] == '\0')
	{
	  send_to_char ("Syntax:  ed format [keyword]\n\r", ch);
	  return FALSE;
	}
      for (ed = pRoom->extra_descr; ed; ed = ed->next)
	{
	  if (is_name (keyword, ed->keyword))
	    break;
	}
      if (!ed)
	{
	  send_to_char ("Extra description keyword not found.\n\r", ch);
	  return FALSE;
	}
      ed->description = format_string (ed->description);
      send_to_char ("Extra description formatted.\n\r", ch);
      return TRUE;
    }
  redit_ed (ch, "");
  return FALSE;
}

REDIT (redit_create)
{
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *pRoom;
  int value;
  int iHash;
  EDIT_ROOM (ch, pRoom);
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
  if (get_room_index (value))
    {
      send_to_char ("Room vnum already exists.\n\r", ch);
      return FALSE;
    }
  pRoom = new_room_index ();
  pRoom->area = pArea;
  pRoom->vnum = value;
  pRoom->mana_rate = pRoom->heal_rate = 100;
  if (value > top_vnum_room)
    top_vnum_room = value;
  iHash = value % MAX_KEY_HASH;
  pRoom->next = room_index_hash[iHash];
  room_index_hash[iHash] = pRoom;
  ch->desc->pEdit = (void *) pRoom;
  send_to_char ("Room created.\n\r", ch);
  return TRUE;
}

REDIT (redit_purge)
{
  CHAR_DATA *fch, *next_person;
  ROOM_INDEX_DATA *pRoom, *pRoomIndex, *prev;
  AREA_DATA *pArea;
  int value, iHash, door;
  char tcbuf[15];
  if (argument[0] == '\0' || (value = atoi (argument)) <= 0)
    {
      send_to_char ("Syntax: purge <vnum>\n\r", ch);
      return FALSE;
    }
  if ((pArea = get_vnum_area (value)) == NULL)
    {
      send_to_char ("That vnum is not assigned an area.\n\r", ch);
      return FALSE;
    }
  if (!is_builder (ch, pArea))
    {
      send_to_char ("Vnum in an area you cannot build in.\n\r", ch);
      return FALSE;
    }
  if ((pRoom = get_room_index (value)) == NULL)
    {
      send_to_char ("Room doesn't exist.\n\r", ch);
      return FALSE;
    }
  if (ch->desc->pEdit == (void *) pRoom)
    edit_done (ch);
  for (fch = pRoom->people; fch != NULL; fch = next_person)
    {
      next_person = fch->next_in_room;
      char_from_room (fch);
      char_to_room (fch, get_room_index (ROOM_VNUM_CHAT));
      trip_triggers(fch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      sprintf(tcbuf,"%d",fch->in_room->vnum);
      trip_triggers_arg(fch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      check_aggression(fch);
    }

  /* remove all objects and mobs (players?) from room */

  /* unlink room */
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pRoomIndex = room_index_hash[iHash]; pRoomIndex != NULL;
	   pRoomIndex = pRoomIndex->next)
	{
	  for (door = 0; door < MAX_DIR; door++)
	    {
	      if (pRoomIndex->exit[door] != NULL
		  && pRoomIndex->exit[door]->u1.to_room == pRoom)
		{
		  SET_BIT (pRoomIndex->area->area_flags, AREA_CHANGED);
		  SET_BIT (pRoomIndex->exit[door]->u1.to_room->area->
			   area_flags, AREA_CHANGED);
		  free_exit (pRoomIndex->exit[door]);
		  pRoomIndex->exit[door] = NULL;
		}
	    }
	}
    }

  /* remove from linked list */
  if (room_index_hash[iHash = value % MAX_KEY_HASH] == pRoom)
    {
      room_index_hash[iHash] = pRoom->next;
    }

  else
    {
      for (prev = room_index_hash[iHash]; prev != NULL; prev = prev->next)
	{
	  if (prev->next == pRoom)
	    break;
	}
      prev->next = pRoom->next;
    }
  free_room_index (pRoom);

  /*  for(pReset = pArea->reset_first; pReset; pReset = pReset->next)
     {
     switch(pReset->command)
     {
     case 'O':
     case 'M':
     if(value == pReset->arg3)
     deleit = TRUE;
     break;

     case 'D':
     if(value == pReset->arg1)
     deleit = TRUE;
     break;
     }

     if(deleit == TRUE)
     {
     if(pReset->command == 'M')
     while(pReset->next != NULL && 
     (pReset->next->command == 'E' || 
     pReset->next->command == 'G'))
     {
     aReset = pReset->next;
     pReset->next = aReset->next;
     if (prevreset)
     {
     if(aReset == pArea->reset_last)
     pArea->reset_last = pReset;
     }

     free_reset_data(aReset);
     }
     else if(pReset->command == 'O')
     while(pReset->next != NULL && 
     pReset->next->command == 'P')
     {
     aReset = pReset->next;
     pReset->next = aReset->next;
     if (prevreset)
     {
     if(aReset == pArea->reset_last)
     pArea->reset_last = pReset;
     }
     free_reset_data(aReset);
     } 

     if (prevreset)
     {
     if(pReset == pArea->reset_last)
     pArea->reset_last = prevreset;
     prevreset->next = prevreset->next->next;
     }
     else
     pArea->reset_first = 
     pArea->reset_first->next;
     free_reset_data(pReset);

     prevreset = pReset;
     break;
     }
     prevreset = pReset;
     }
     } */
  send_to_char ("Room Purged.\n\r", ch);
  return TRUE;
}

REDIT (redit_name)
{
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM (ch, pRoom);
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  name [name]\n\r", ch);
      return FALSE;
    }
  free_string (pRoom->name);
  pRoom->name = str_dup (argument);
  send_to_char ("Name set.\n\r", ch);
  return TRUE;
}

REDIT (redit_maxmsg)
{
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM (ch, pRoom);
  if (argument[0] == '\0')
    {
      free_string (pRoom->max_message);
      pRoom->max_message = str_dup ("");
      send_to_char ("Max message deleted.\n\r", ch);
      return TRUE;
    }
  free_string (pRoom->max_message);
  pRoom->max_message = str_dup (argument);
  send_to_char ("Max message set.\n\r", ch);
  return TRUE;
}

REDIT (redit_exitmsg)
{
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM (ch, pRoom);
  if (argument[0] == '\0')
    {
      free_string (pRoom->exit_msg);
      pRoom->exit_msg = str_dup ("");
      send_to_char ("Exit message deleted.\n\r", ch);
      return TRUE;
    }
  free_string (pRoom->exit_msg);
  pRoom->exit_msg = str_dup (argument);
  send_to_char ("Exit message set.\n\r", ch);
  return TRUE;
}

REDIT (redit_travelmsg)
{
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM (ch, pRoom);
  if (argument[0] == '\0')
    {
      free_string (pRoom->tp_msg);
      pRoom->tp_msg = str_dup ("");
      send_to_char ("Exploration point message deleted.\n\r", ch);
      return TRUE;
    }
  free_string (pRoom->tp_msg);
  pRoom->tp_msg = str_dup (argument);
  send_to_char ("Exploration point message set.\n\r", ch);
  return TRUE;
}

REDIT (redit_entermsg)
{
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM (ch, pRoom);
  if (argument[0] == '\0')
    {
      free_string (pRoom->enter_msg);
      pRoom->enter_msg = str_dup ("");
      send_to_char ("Enter message deleted.\n\r", ch);
      return TRUE;
    }
  free_string (pRoom->enter_msg);
  pRoom->enter_msg = str_dup (argument);
  send_to_char ("Enter message set.\n\r", ch);
  return TRUE;
}

REDIT (redit_desc)
{
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM (ch, pRoom);
  if (argument[0] == '\0')
    {
      send_to_char
	("Please use the Exodus Editor to enter the description:\n\r\n\r",
	 ch);
      edit_string (ch, &pRoom->description);
      return TRUE;
    }
  send_to_char ("Syntax:  desc\n\r", ch);
  return FALSE;
}

REDIT (redit_format)
{
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM (ch, pRoom);
  pRoom->description = format_string (pRoom->description);
  send_to_char ("String formatted.\n\r", ch);
  return TRUE;
}

REDIT (redit_mreset)
{
  ROOM_INDEX_DATA *pRoom;
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *newmob;
  char arg[MAX_INPUT_LENGTH];
  char lim1[MAX_INPUT_LENGTH];
  char lim2[MAX_INPUT_LENGTH];
  RESET_DATA *pReset;
  char output[MAX_STRING_LENGTH];
  EDIT_ROOM (ch, pRoom);
  argument = one_argument (argument, arg);
  argument = one_argument (argument, lim1);
  argument = one_argument (argument, lim2);
  if (arg[0] == '\0' || !is_number (arg))
    {
      send_to_char
	("Syntax:  mreset <vnum> <max [world]#> <max [room]#>\n\r", ch);
      return FALSE;
    }
  if (!(pMobIndex = get_mob_index (atoi (arg))))
    {
      send_to_char ("No mobile has that vnum.\n\r", ch);
      return FALSE;
    }
  if (pMobIndex->area != pRoom->area)
    {
      send_to_char ("No such mobile in this area.\n\r", ch);
      return FALSE;
    }

  /*
   * Create the mobile reset.
   */
  pReset = new_reset_data ();
  pReset->command = 'M';
  pReset->arg1 = pMobIndex->vnum;
  pReset->arg2 = is_number (lim1) ? atoi (lim1) : MAX_MOB;

/*
  pReset->arg3 = pRoom->vnum;
*/
  pReset->arg3 = ch->in_room->vnum;
  pReset->arg4 = is_number (lim2) ? atoi (lim2) : MAX_MOB;
  add_reset (ch->in_room /*pRoom */ , pReset, 0 /* Last slot */ );

  /*
   * Create the mobile.
   */
  newmob = create_mobile (pMobIndex);
  char_to_room (newmob, pRoom);
  sprintf (output, "%s (%d) has been loaded and added to resets.\n\r"
	   "There will be a maximum of %d loaded in the world.\n\r"
	   "There will be a maximum of %d loaded into this room.\n\r",
	   capitalize (pMobIndex->short_descr), pMobIndex->vnum,
	   pReset->arg2, pReset->arg4);
  send_to_char (output, ch);
  act ("$n has created $N!", ch, NULL, newmob, TO_ROOM);
  return TRUE;
}
struct wear_type
{
  int wear_loc;
  int wear_bit;
};
const struct wear_type wear_table[] = {
  {WEAR_NONE, ITEM_TAKE},
  {WEAR_LIGHT, ITEM_LIGHT},
  {WEAR_FINGER_L, ITEM_WEAR_FINGER},
  {WEAR_FINGER_R, ITEM_WEAR_FINGER},
  {WEAR_NECK_1, ITEM_WEAR_NECK},
  {WEAR_NECK_2, ITEM_WEAR_NECK},
  {WEAR_BODY, ITEM_WEAR_BODY},
  {WEAR_HEAD, ITEM_WEAR_HEAD},
  {WEAR_LEGS, ITEM_WEAR_LEGS},
  {WEAR_FEET, ITEM_WEAR_FEET},
  {WEAR_HANDS, ITEM_WEAR_HANDS},
  {WEAR_ARMS, ITEM_WEAR_ARMS},
  {WEAR_SHIELD, ITEM_WEAR_SHIELD},
  {WEAR_ABOUT, ITEM_WEAR_ABOUT},
  {WEAR_WAIST, ITEM_WEAR_WAIST},
  {WEAR_WRIST_L, ITEM_WEAR_WRIST},
  {WEAR_WRIST_R, ITEM_WEAR_WRIST},
  {WEAR_WIELD_R, ITEM_WIELD},
  {WEAR_HOLD, ITEM_HOLD},
  {WEAR_WIELD_L, ITEM_WIELD},
  {WEAR_CLAN_MARK, ITEM_WEAR_CLAN_MARK},
  {WEAR_WORN_WINGS, ITEM_WORN_WINGS},
  {NO_FLAG, NO_FLAG}
};

/*****************************************************************************
 Name:		wear_loc
 Purpose:	Returns the location of the bit that matches the count.
 		1 = first match, 2 = second match etc.
 Called by:	oedit_reset(olc_act.c).
 ****************************************************************************/
int wear_loc (int bits, int count)
{
  int flag;
  for (flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++)
    {
      if (IS_SET (bits, wear_table[flag].wear_bit) && --count < 1)
	return wear_table[flag].wear_loc;
    }
  return NO_FLAG;
}


/*****************************************************************************
 Name:		wear_bit
 Purpose:	Converts a wear_loc into a bit.
 Called by:	redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit (int loc)
{
  int flag;
  for (flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++)
    {
      if (loc == wear_table[flag].wear_loc)
	return wear_table[flag].wear_bit;
    }
  return 0;
}

REDIT (redit_Class)
{
  ROOM_INDEX_DATA *pRoom;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_ROOM (ch, pRoom);
      if ((value = flag_value (Class_flags, argument)) != NO_FLAG)
	{
	  TOGGLE_BIT (pRoom->Class_flags, value);
	  send_to_char ("Class flag(s) toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax: Class [flag]\n\r"
		"Type '? Class' for a list of flags.\n\r", ch);
  return FALSE;
}

REDIT (redit_clan)
{
  char buf[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *pRoom;
  int slot;
  if (!IS_IMMORTAL (ch))
    return FALSE;
  if (argument[0] != '\0')
    {
      EDIT_ROOM (ch, pRoom);
      if (!str_cmp (argument, NOCLAN_NAME))
	{
	  pRoom->clan = CLAN_BOGUS;
	  sprintf (buf, "This room is no longer owned by any clan.\n\r");
	  send_to_char (buf, ch);
	  return TRUE;
	}

      else
	{
	  slot = clanname_to_slot (argument);
	  if (slot == CLAN_BOGUS)
	    {
	      sprintf (buf, "There is no clan named %s``.\n\r",
		       capitalize (argument));
	      send_to_char (buf, ch);
	      send_to_char ("Clan ownership of this room unchanged.\n\r", ch);
	      return FALSE;
	    }
	  pRoom->clan = slot;
	  sprintf (buf, "This room is now owned by clan %s``.\n\r",
		   get_clan_name (slot));
	  send_to_char (buf, ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax: clan <clan_name>\n\r", ch);
  return FALSE;
}

REDIT (redit_owner)
{
  ROOM_INDEX_DATA *pRoom;
  if (!IS_IMMORTAL (ch))
    return FALSE;
  EDIT_ROOM (ch, pRoom);
  if (argument[0] == '\0')
    {
      free_string (pRoom->owner);
      pRoom->owner = str_dup ("");
      send_to_char ("Room owner deleted.\n\r", ch);
      return TRUE;
    }
  free_string (pRoom->owner);
  pRoom->owner = str_dup (argument);
  send_to_char ("Room owner set.\n\r", ch);

  //  send_to_char("Syntax: owner <player_name>\n\r", ch);
  return TRUE;
}

REDIT (redit_race)
{
  ROOM_INDEX_DATA *pRoom;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_ROOM (ch, pRoom);
      if ((value = flag_value (pcrace_flags, argument)) != NO_FLAG)
	{
	  TOGGLE_BIT (pRoom->race_flags, value);
	  send_to_char ("Race flag(s) toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax: race [flag]\n\r"
		"Type '? race' for a list of flags.\n\r", ch);
  return FALSE;
}

REDIT (redit_epl)
{
  ROOM_INDEX_DATA *pRoom;
  char fullpath[1024];
  FILE *fp;
  EDIT_ROOM (ch, pRoom);
  if (argument[0] == '\0')
    {
      if (pRoom->epl_filename != NULL)
	free_string (pRoom->epl_filename);
      pRoom->epl_filename = NULL;
      send_to_char ("EPL program for room cleared.\n\r", ch);
      return (TRUE);
    }
  if (pRoom->epl_filename != NULL)
    free_string (pRoom->epl_filename);
  pRoom->epl_filename = str_dup (argument);
  sprintf (fullpath, "epl/%s", argument);
  if ((fp = fopen (fullpath, "r")) == NULL)
    {
      send_to_char ("No such EPL program exists.\n\r", ch);
      return (FALSE);
    }
  fclose (fp);
  send_to_char ("EPL program filename set.\n\r", ch);
  return (TRUE);
}

REDIT (redit_oreset)
{
  ROOM_INDEX_DATA *pRoom;
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *newobj;
  OBJ_DATA *to_obj;
  CHAR_DATA *to_mob;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  int olevel = 0;
  RESET_DATA *pReset;
  char output[MAX_STRING_LENGTH];
  EDIT_ROOM (ch, pRoom);
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0' || !is_number (arg1))
    {
      send_to_char ("Syntax:  oreset <vnum> <args>\n\r", ch);
      send_to_char ("        -no_args               = into room\n\r", ch);
      send_to_char
	("        -max <max in world>    = into room + max world\n\r", ch);
      send_to_char ("        -<obj_name>            = into obj\n\r", ch);
      send_to_char ("        -<mob_name> <wear_loc> = into mob\n\r", ch);
      return FALSE;
    }
  if (!(pObjIndex = get_obj_index (atoi (arg1))))
    {
      send_to_char ("No object has that vnum.\n\r", ch);
      return FALSE;
    }
  if (pObjIndex->area != pRoom->area)
    {
      send_to_char ("No such object in this area.\n\r", ch);
      return FALSE;
    }

  /*
   * Load into room.
   */
  if (arg2[0] == '\0')
    {
      pReset = new_reset_data ();
      pReset->command = 'O';
      pReset->arg1 = pObjIndex->vnum;
      pReset->arg2 = 0;

/*
      pReset->arg3 = pRoom->vnum;
*/
      pReset->arg3 = ch->in_room->vnum;
      pReset->arg4 = -1;
      add_reset (ch->in_room /*pRoom */ , pReset, 0 /* Last slot */ );
      newobj = create_object (pObjIndex, number_fuzzy (olevel));
      obj_to_room (newobj, pRoom);
      sprintf (output,
	       "%s (%d) has been loaded and added to resets.\n\r",
	       capitalize (pObjIndex->short_descr), pObjIndex->vnum);
      send_to_char (output, ch);
    }

  else if (!str_cmp (arg2, "max"))
    {
      argument = one_argument (argument, arg3);
      pReset = new_reset_data ();
      pReset->command = 'O';
      pReset->arg1 = pObjIndex->vnum;
      pReset->arg2 = 0;

/*
      pReset->arg3 = pRoom->vnum;
*/
      pReset->arg3 = ch->in_room->vnum;
      pReset->arg4 = is_number (arg3) ? atoi (arg3) : -1;
      add_reset (ch->in_room /*pRoom */ , pReset, 0 /* Last slot */ );
      newobj = create_object (pObjIndex, number_fuzzy (olevel));
      obj_to_room (newobj, pRoom);
      sprintf (output,
	       "%s (%d) has been loaded and added to resets.\n\r",
	       capitalize (pObjIndex->short_descr), pObjIndex->vnum);
      send_to_char (output, ch);
      return (TRUE);
    }

  else
    /*
     * Load into object's inventory.
     */
  if (argument[0] == '\0'
	&& ((to_obj = get_obj_list (ch, arg2, pRoom->contents)) != NULL))
    {
      pReset = new_reset_data ();
      pReset->command = 'P';
      pReset->arg1 = pObjIndex->vnum;
      pReset->arg2 = -1;
      pReset->arg3 = to_obj->pIndexData->vnum;
      pReset->arg4 = 1;
      add_reset (ch->in_room /*pRoom */ , pReset, 0);

/*
             insert_obj_reset(pRoom, pReset, to_obj->pIndexData->vnum, 
         ch->in_room->vnum);
*/
      newobj = create_object (pObjIndex, number_fuzzy (olevel));
      newobj->cost = 0;
      obj_to_obj (newobj, to_obj);
      sprintf (output, "%s (%d) has been loaded into "
	       "%s (%d) and added to resets.\n\r",
	       capitalize (newobj->short_descr),
	       newobj->pIndexData->vnum, to_obj->short_descr,
	       to_obj->pIndexData->vnum);
      send_to_char (output, ch);
    }

  else if ((to_mob = get_char_room (ch, arg2)) != NULL)
    {
      int wear_loc;
      if ((wear_loc = flag_value (wear_loc_flags, argument)) == NO_FLAG)
	{
	  send_to_char ("Invalid wear_loc.  '? wear-loc'\n\r", ch);
	  return FALSE;
	}
      if (!IS_SET (pObjIndex->wear_flags, wear_bit (wear_loc)))
	{
	  sprintf (output, "%s (%d) has wear flags: [%s]\n\r",
		   capitalize (pObjIndex->short_descr), pObjIndex->vnum,
		   flag_string (wear_flags, pObjIndex->wear_flags));
	  send_to_char (output, ch);
	  return FALSE;
	}
      if (get_eq_char (to_mob, wear_loc) && wear_loc != WEAR_NONE)
	{
	  send_to_char ("Object already equipped.\n\r", ch);
	  return FALSE;
	}
      pReset = new_reset_data ();
      pReset->arg1 = pObjIndex->vnum;
      pReset->arg2 = -1;
      if (wear_loc == WEAR_NONE)
	pReset->command = 'G';

      else
	pReset->command = 'E';
      pReset->arg3 = wear_loc;
      add_reset (ch->in_room /*pRoom */ , pReset, 0);

/*
           insert_mob_reset(pRoom, pReset, to_mob->pIndexData->vnum, 
         ch->in_room->vnum); 
*/
      olevel = URANGE (0, to_mob->level - 2, LEVEL_HERO);
      newobj = create_object (pObjIndex, number_fuzzy (olevel));
      if (to_mob->pIndexData->pShop)
	{
	  switch (pObjIndex->item_type)
	    {
	    default:
	      olevel = 0;
	      break;
	    case ITEM_PILL:
	      olevel = number_range (0, 10);
	      break;
	    case ITEM_POTION:
	      olevel = number_range (0, 10);
	      break;
	    case ITEM_SCROLL:
	      olevel = number_range (5, 15);
	      break;
	    case ITEM_WAND:
	      olevel = number_range (10, 20);
	      break;
	    case ITEM_STAFF:
	      olevel = number_range (15, 25);
	      break;
	    case ITEM_ARMOR:
	      olevel = number_range (5, 15);
	      break;
	    case ITEM_WEAPON:
	      if (pReset->command == 'G')
		olevel = number_range (5, 15);

	      else
		olevel = number_fuzzy (olevel);
	      break;
	    }
	  newobj = create_object (pObjIndex, olevel);
	  if (pReset->arg2 == WEAR_NONE)
	    SET_BIT (newobj->extra_flags[0], ITEM_INVENTORY);
	}

      else
	newobj = create_object (pObjIndex, number_fuzzy (olevel));
      obj_to_char (newobj, to_mob);
      if (pReset->command == 'E')
	equip_char (to_mob, newobj, pReset->arg3);
      sprintf (output, "%s (%d) has been loaded "
	       "%s of %s (%d) and added to resets.\n\r",
	       capitalize (pObjIndex->short_descr), pObjIndex->vnum,
	       flag_string (wear_loc_strings, pReset->arg3),
	       to_mob->short_descr, to_mob->pIndexData->vnum);
      send_to_char (output, ch);
    }

  else
    {
      send_to_char ("That mobile isn't here.\n\r", ch);
      return FALSE;
    }
  act ("$n has created $p!", ch, newobj, NULL, TO_ROOM);
  return TRUE;
}


/*
 * Object Editor Functions.
 */
void show_obj_values (CHAR_DATA * ch, OBJ_INDEX_DATA * obj)
{
  char buf[MAX_STRING_LENGTH];
  bool fucked = FALSE;
  switch (obj->item_type)
    {
    default:			/* No values. */
      break;
    case ITEM_WRITING_PAPER:
      sprintf (buf, "[v0] Hours message lasts:  [%d]\n\r", obj->value[0]);
      send_to_char (buf, ch);
      break;
    case ITEM_LIGHT:
      if (obj->value[2] == -1 || obj->value[2] == 999)	/* ROM OLC */
	sprintf (buf, "[v2] Light:  Infinite[-1]\n\r");

      else
	sprintf (buf, "[v2] Light:  [%d]\n\r", obj->value[2]);
      send_to_char (buf, ch);
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      sprintf (buf, "[v0] Level:          [%d]\n\r"
	       "[v1] Charges Total:  [%d]\n\r"
	       "[v2] Charges Left:   [%d]\n\r[v3] ", obj->value[0],
	       obj->value[1], obj->value[2]);
      switch (skill_table[obj->value[3]].type)
	{
	default:
	  strcat (buf, "Unused:  ");
	  fucked = TRUE;
	  break;
	case SKILL_CAST:
	  strcat (buf, "Spell:  ");
	  break;
	case SKILL_SING:
	  strcat (buf, "Song:  ");
	  break;
	case SKILL_PRAY:
	  strcat (buf, "Prayer:  ");
	  break;
	case SKILL_CHANT:
	  strcat (buf, "Chant:  ");
	  break;
	}
      if (!fucked)
	strcat (buf,
		obj->value[3] >=
		0 ? skill_table[obj->value[3]].name : "none");

      else
	strcat (buf, "none");
      strcat (buf, "\n\r");
      send_to_char (buf, ch);
      break;
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
      sprintf (buf, "[v0] Level:  [%d]\n\r[v1] ", obj->value[0]);
      switch (skill_table[obj->value[1]].type)
	{
	default:
	  strcat (buf, "Unused:  ");
	  break;
	case SKILL_CAST:
	  strcat (buf, "Spell:  ");
	  break;
	case SKILL_SING:
	  strcat (buf, "Song:  ");
	  break;
	case SKILL_PRAY:
	  strcat (buf, "Prayer:  ");
	  break;
	case SKILL_CHANT:
	  strcat (buf, "Chant:  ");
	  break;
	}
      strcat (buf,
	      obj->value[1] != -1 ? skill_table[obj->value[1]].name : "none");
      strcat (buf, "\n\r[v2] ");
      switch (skill_table[obj->value[2]].type)
	{
	default:
	  strcat (buf, "Unused:  ");
	  break;
	case SKILL_CAST:
	  strcat (buf, "Spell:  ");
	  break;
	case SKILL_SING:
	  strcat (buf, "Song:  ");
	  break;
	case SKILL_PRAY:
	  strcat (buf, "Prayer:  ");
	  break;
	case SKILL_CHANT:
	  strcat (buf, "Chant:  ");
	  break;
	}
      strcat (buf,
	      obj->value[2] != -1 ? skill_table[obj->value[2]].name : "none");
      strcat (buf, "\n\r[v3] ");
      switch (skill_table[obj->value[3]].type)
	{
	default:
	  strcat (buf, "Unused:  ");
	  break;
	case SKILL_CAST:
	  strcat (buf, "Spell:  ");
	  break;
	case SKILL_SING:
	  strcat (buf, "Song:  ");
	  break;
	case SKILL_PRAY:
	  strcat (buf, "Prayer:  ");
	  break;
	case SKILL_CHANT:
	  strcat (buf, "Chant:  ");
	  break;
	}
      strcat (buf,
	      obj->value[3] != -1 ? skill_table[obj->value[3]].name : "none");
      strcat (buf, "\n\r[v4] ");
      switch (skill_table[obj->value[4]].type)
	{
	default:
	  strcat (buf, "Unused:  ");
	  break;
	case SKILL_CAST:
	  strcat (buf, "Spell:  ");
	  break;
	case SKILL_SING:
	  strcat (buf, "Song:  ");
	  break;
	case SKILL_PRAY:
	  strcat (buf, "Prayer:  ");
	  break;
	case SKILL_CHANT:
	  strcat (buf, "Chant:  ");
	  break;
	}
      strcat (buf,
	      obj->value[4] != -1 ? skill_table[obj->value[4]].name : "none");
      strcat (buf, "\n\r");
      send_to_char (buf, ch);
      break;
    case ITEM_ARMOR:
      sprintf (buf, "[v0] Ac pierce       [%d]\n\r"
	       "[v1] Ac bash         [%d]\n\r"
	       "[v2] Ac slash        [%d]\n\r"
	       "[v3] Ac exotic       [%d]\n\r", obj->value[0],
	       obj->value[1], obj->value[2], obj->value[3]);
      send_to_char (buf, ch);
      break;
    case ITEM_FURNITURE:
      sprintf (buf, "[v0] Number of People: %d\n\r", obj->value[0]);
      send_to_char (buf, ch);
      sprintf (buf, "[v1] Total Weight: %d\n\r", obj->value[1]);
      send_to_char (buf, ch);
      sprintf (buf, "[v2] Furniture Flags: %s\n\r",
	       flag_string (furniture_type, obj->value[2]));
      send_to_char (buf, ch);
      sprintf (buf, "[v3] Heal Bonus: %d\n\r", obj->value[3]);
      send_to_char (buf, ch);
      sprintf (buf, "[v4] Mana Bonus: %d\n\r", obj->value[4]);
      send_to_char (buf, ch);
      break;
   case ITEM_RAFT:
      sprintf (buf, "[v0] Number of People: %d\n\r", obj->value[0]);
      send_to_char (buf, ch);
      sprintf (buf, "[v1] Total Weight: %d\n\r", obj->value[1]);
      send_to_char (buf, ch);
      sprintf (buf, "[v2] Furniture Flags: %s\n\r",
          flag_string (furniture_type, obj->value[2]));
      send_to_char (buf, ch);
      sprintf (buf, "[v3] Sector Types:  [%s]\n\r",
          flag_string (tsector_flags, obj->value[3]));
      send_to_char (buf, ch);					  
      break;
    case ITEM_CTRANSPORT:
      sprintf (buf, "[v0] Sector Types:  [%s]\n\r",
	       flag_string (tsector_flags, obj->value[0]));
      send_to_char (buf, ch);
      sprintf (buf, "[v1] Max Weight:  [%d]\n\r"
	       "[v2] Entrance/Exit vnum:  [%d]\n\r"
	       "[v3] Cockpit vnum:  [%d]\n\r"
	       "[v4] Action types:  [%s]\n\r", obj->value[1],
	       obj->value[2], obj->value[3], flag_string (tact_flags,
							  obj->value[4]));
      send_to_char (buf, ch);
      sprintf (buf, "[v5] Door Flags:  [%s]\n\r"
	       "[v6] Key Vnum:    [%d]\n\r",
	       flag_string (container_flags, obj->value[5]), obj->value[6]);
      send_to_char (buf, ch);
      break;
    case ITEM_ELEVATOR:
      sprintf (buf, "[v0] Elevator holding room:  [%d]\n\r"
	       "[v1] Maximum people in elevator:  [%d]\n\r"
	       "[v2] Maximum weight in elevator:  [%d]\n\r",
	       obj->value[0], obj->value[1], obj->value[2]);
      send_to_char (buf, ch);
      break;
    case ITEM_EBUTTON:
      sprintf (buf, "[v0] Vnum of Elevator Object:  [%d]\n\r"
	       "[v1] Button 'floor':  [%d]\n\r"
	       "[v2] Elevator goto:  [%d]\n\r", obj->value[0],
	       obj->value[1], obj->value[2]);
      send_to_char (buf, ch);
      sprintf (buf, "[v3] Button Actions:  [%s]\n\r",
	       flag_string (button_flags, obj->value[3]));
      send_to_char (buf, ch);
      break;
    case ITEM_INSTRUMENT:
      sprintf (buf, "[v0] Instrument Flags:  %s\n\r",
	       flag_string (instrument_flags, obj->value[0]));
      send_to_char (buf, ch);
      break;
    case ITEM_KEY:
      sprintf (buf, "[v0] Key Saved:  %s\n\r", obj->value[0] ? "Yes" : "No");
      send_to_char (buf, ch);
      break;
    case ITEM_PLAYER_DONATION:
      sprintf (buf, "[v0] Max Items: %d\n\r", obj->value[0]);
      send_to_char (buf, ch);
      break;
    case ITEM_QUIVER:
      sprintf (buf, "[v0] Capacity: %d\n\r", obj->value[0]);
      send_to_char (buf, ch);
      break;
    case ITEM_BINDER:
      sprintf (buf, "[v0] Capacity: %d\n\r", obj->value[0]);
      send_to_char (buf, ch);
      break;
    case ITEM_POLE:
      sprintf (buf, "[v1] Baited: %d\n\r", obj->value[0]);
      send_to_char (buf, ch);
      break;
    case ITEM_CLAN_DONATION:
    case ITEM_NEWCLANS_DBOX:
      sprintf (buf, "[v0] Max Items: %d\n\r", obj->value[0]);
      send_to_char (buf, ch);
      if (obj->value[1] == CLAN_BOGUS)
	send_to_char ("[v1] Clan Owned By; [None]\n\r", ch);

      else
	{
	  sprintf (buf, "[v1] Clan Owned By: %s``\n\r",
		   get_clan_name (obj->value[1]));
	  send_to_char (buf, ch);
	}
      break;
    case ITEM_OBJ_TRAP:
    case ITEM_ROOM_TRAP:
    case ITEM_PORTAL_TRAP:
     	sprintf (buf, "[v0] Trap Type:       %s\n\r", flag_string (trap_type, obj->value[0]));
     	send_to_char (buf, ch);
	switch(obj->value[0])
	{
	case 0:
		send_to_char("[v1] Set Trap Type: see ? traptype \n\r",ch);
		send_to_char("[v2] Set Trap Type: see ? traptype \n\r",ch);
		send_to_char("[v3] Set Trap Type: see ? traptype \n\r",ch);
		break;
	case TRAP_R_SPELL:
	case TRAP_O_SPELL:
	case TRAP_P_SPELL:
		switch (skill_table[obj->value[1]].type)
		{
		default:
		  sprintf(buf, "[v1] Unused Spell:    ");
		  break;
		case SKILL_CAST:
		  sprintf(buf, "[v1] Spell:           ");
		  break;
		case SKILL_SING:
		  sprintf(buf, "[v1] Song:            ");
		  break;
		case SKILL_PRAY:
		  sprintf(buf, "[v1] Prayer:          ");
		  break;
		case SKILL_CHANT:
		  sprintf(buf, "[v1] Chant:           ");
		  break;
		}
        	strcat (buf, obj->value[1] != -1 ? skill_table[obj->value[1]].name : "none");
   		
		send_to_char (buf, ch);
		 sprintf (buf, "\n\r[v2] Spell Level:    [%d]\n\r", obj->value[2]);
		 send_to_char (buf, ch);
		 break;
	default:
	     	sprintf (buf, "[v1] Number of dice:  [%d]\n\r", obj->value[1]);
		send_to_char (buf, ch);
		sprintf (buf, "[v2] Type of dice:    [%d]\n\r", obj->value[2]);
		send_to_char (buf, ch);
		sprintf	(buf, "[v3] Damage Mod:      [%d]\n\r", obj->value[3]);
		send_to_char (buf, ch);
		break;
	}
	sprintf (buf, "[v5] Trap Dex:        [%d]\n\r", obj->value[5]);
	send_to_char (buf, ch);
	sprintf (buf, "[v6] Summon Mob Vnum: [%d]\n\r", obj->value[6]);
	send_to_char (buf, ch);
	
	if (obj->value[0] >= MIN_PORTAL_TRAP && obj->value[0] <= MAX_PORTAL_TRAP)
	{
		if (obj->value[7] <= -1)
		{
			send_to_char("[v7] Trap Exit Type  [none]\n\r",ch);
		}
		else
		{
			sprintf (buf, "[v7] Trap Exit Type:  [%s]\n\r", flag_string (pact_flags,obj->value[7]));
			send_to_char (buf, ch);
		}
	}
     	sprintf (buf, "[v8] Damage Type:     %s\n\r", flag_string (weapon_flags, obj->value[8]));
	send_to_char (buf, ch);
	if (obj->value[0] >= MIN_PORTAL_TRAP && obj->value[0] <= MAX_PORTAL_TRAP)
	{
		if(obj->value[9] <= -1)
			send_to_char("[v9] Trap Door Dir:    [none]\n\r",ch);
		else
		{
			sprintf (buf, "[v9] Trap Door Dir:   [%s]\n\r", flag_string (dir_flags,obj->value[9]));
			send_to_char (buf, ch);
		}
	}
	// 12 stores trap flags 
	break;
		
    case ITEM_WEAPON:
      sprintf (buf, "[v0] Weapon Class:   %s\n\r",
	       flag_string (weapon_Class, obj->value[0]));
      send_to_char (buf, ch);
      sprintf (buf, "[v1] Number of dice: [%d]\n\r", obj->value[1]);
      send_to_char (buf, ch);
      sprintf (buf, "[v2] Type of dice:   [%d]\n\r", obj->value[2]);
      send_to_char (buf, ch);
      sprintf (buf, "[v3] Type:           %s\n\r",
	       flag_string (weapon_flags, obj->value[3]));
      send_to_char (buf, ch);
      sprintf (buf, "[v4] Special type:   %s\n\r",
	       flag_string (weapons_type, obj->value[4]));
      send_to_char (buf, ch);
      if (obj->value[0] == WEAPON_DICE)
      {
	if (obj->value[5] > MAX_SKILL || skill_table[obj->value[5]].name == NULL
			|| skill_table[obj->value[5]].type == SKILL_NORMAL ||
			obj->value[5] < 0)
  	  obj->value[5] = 0;
        sprintf (buf, "[v5] Spell:          %s\n\r",skill_table[obj->value[5]].name);
        send_to_char (buf,ch);
	  
      }
      break;
    case ITEM_CONTAINER:
      sprintf (buf, "[v0] Weight: [%d kg]\n\r"
	       "[v1] Flags:  [%s]\n\r"
	       "[v2] Key:  [%d]\n\r"
	       "[v3] Max Weight (Single Object): [%d]\n\r"
	       "[v4] Weight Multiplier: [%d]\n\r", obj->value[0],
	       flag_string (container_flags, obj->value[1]),
	       obj->value[2], obj->value[3], obj->value[4]);
      send_to_char (buf, ch);
      break;
    case ITEM_PACK:
      sprintf (buf, "[v0] Weight: [%d kg]\n\r"
	       "[v1] Flags:  [%s]\n\r"
	       "[v3] Max Weight (Single Object): [%d]\n\r"
	       "[v4] Weight Multiplier: [%d]\n\r", obj->value[0],
	       flag_string (container_flags, obj->value[1]),
	       obj->value[3], obj->value[4]);
      send_to_char (buf, ch);
      break;
    case ITEM_DRINK_CON:
      sprintf (buf, "[v0] Liquid Total: [%d]\n\r"
	       "[v1] Liquid Left:  [%d]\n\r"
	       "[v2] Liquid:       %s\n\r"
	       "[v3] Poisoned:     %s\n\r", obj->value[0],
	       obj->value[1], flag_string (liquid_flags, obj->value[2]),
	       obj->value[3] != 0 ? "Yes" : "No");
      send_to_char (buf, ch);
      break;
    case ITEM_FOUNTAIN:
      sprintf (buf, "[v2] Liquid:       %s\n\r"
	       "[v3] Poisoned:     %s\n\r", flag_string (liquid_flags,
							 obj->value[2]),
	       obj->value[3] != 0 ? "Yes" : "No");
      send_to_char (buf, ch);
      break;
    case ITEM_PORTAL:
      sprintf (buf, "[v0] Charges: [%d]\n\r"
	       "[v1] ExitFlags: [%s]\n\r", obj->value[0],
	       flag_string (exit_flags, obj->value[1]));
      send_to_char (buf, ch);
      sprintf (buf, "[v2] Gate Flags: [%s]\n\r"
	       "[v3] VnumTo: [%d]\n\r", flag_string (gate_flags,
						     obj->value[2]),
	       obj->value[3]);
      send_to_char (buf, ch);
      sprintf (buf, "[v4] ActionFlags: [%s]\n\r",
	       flag_string (pact_flags, obj->value[4]));
      send_to_char (buf, ch);
      break;
    case ITEM_PORTAL_BOOK:
      sprintf (buf, "[v0] Charges: [%d]\n\r"
	       "[v1] VnumTo: [%d]\n\r", obj->value[0], obj->value[1]);
      send_to_char (buf, ch);
      break;
    case ITEM_FOOD:
      sprintf (buf, "[v0] Food (full) hours: [%d]\n\r"
	       "[v1] Food (not hungry) hours: [%d]\n\r"
	       "[v2] Hours until rot:  [%d]\n\r"
	       "[v3] Poisoned:   %s\n\r", obj->value[0], obj->value[1],
	       obj->value[2], obj->value[3] != 0 ? "Yes" : "No");
      send_to_char (buf, ch);
      break;
    case ITEM_MONEY:
      sprintf (buf, "[v0] Silver:   [%d]\n\r", obj->value[0]);
      send_to_char (buf, ch);
      sprintf (buf, "[v1] Gold:     [%d]\n\r", obj->value[1]);
      send_to_char (buf, ch);
      break;
    case ITEM_WOOD:
      if (obj->value[0] < 0 || obj->value[0] >= MAX_TREES)
        obj->value[0] = 0;
      sprintf (buf, "[v0] Wood Type:  %s\n\r", tree_table[obj->value[0]].name);
      send_to_char (buf,ch);
      break;
    case ITEM_CARD:
      sprintf (buf, "[v0] Card number:     [%d]\n\r",obj->value[0]);
      send_to_char (buf, ch);
      sprintf (buf, "[v1] Card type:       %s\n\r", flag_string (card_Class, obj->value[1]));
      send_to_char (buf, ch);
      sprintf (buf, "[v2] Number of dice:  [%d]\n\r", obj->value[2]);
      send_to_char (buf, ch);
      sprintf (buf, "[v3] Type of dice:    [%d]\n\r", obj->value[3]);
      send_to_char (buf, ch);
      sprintf (buf, "[v4] Bonus to dice:   [%d]\n\r",obj->value[4]);
      send_to_char (buf, ch);
      sprintf (buf, "[v5] Level of spell:  [%d]\n\r[v6] ",obj->value[5]);
//      send_to_char (buf, ch);
      switch (skill_table[obj->value[6]].type)
        {
        default:
          strcat (buf, "Unused:          ");
          fucked = TRUE;
          break;
        case SKILL_CAST:
          strcat (buf, "Spell:           ");
          break;
        case SKILL_SING:
          strcat (buf, "Song:            ");
          break;
        case SKILL_PRAY:
          strcat (buf, "Prayer:          ");
          break;
        case SKILL_CHANT:
          strcat (buf, "Chant:           ");
          break;
        }
      if (!fucked)
        strcat (buf,
                obj->value[6] >=
                0 ? skill_table[obj->value[6]].name : "none");

      else
        strcat (buf, "none");
      strcat (buf, "\n\r");
      send_to_char (buf, ch);
      //Area to pop in
      
      switch (obj->value[1])
      {
        case CARD_MOB :
  	  sprintf (buf, "[v7] Hitpoints:       [%d]\n\r",obj->value[7]);
	  send_to_char (buf, ch);
	  sprintf (buf, "[v8] Offence:         [%d]\n\r",obj->value[8]);
	  send_to_char (buf, ch);
 	  sprintf (buf, "[v9] Defence:         [%d]\n\r",obj->value[9]);
	  send_to_char (buf, ch);
	  sprintf (buf, "[v10] Race:           %s\n\r",flag_string (card_race_type, obj->value[10]));
          send_to_char (buf, ch);
	  sprintf (buf, "[v11] Element:        %s\n\r",flag_string (card_element_type, obj->value[11]));
          send_to_char (buf, ch);
	  sprintf (buf, "[v12] Nemesis:        %s\n\r",flag_string (card_race_type, obj->value[12]));
          send_to_char (buf, ch);
	  break;
        case CARD_TERRAIN:
	  sprintf (buf, "[v7] Terrain type:                 %s\n\r", flag_string (card_terrain_type, obj->value[7]));
          send_to_char (buf, ch);
	  sprintf (buf, "[v8] Off Bonus for Non-Elems mobs and damage: [%d]\n\r",obj->value[8]);
	  send_to_char (buf, ch);
	  sprintf (buf, "[v9] Def Bonus for Non-Elems mobs:            [%d]\n\r", obj->value[9]);
	  send_to_char (buf, ch);
	  sprintf (buf, "[v10] Off Bonus for Elems mobs and damage:    [%d]\n\r", obj->value[10]);
	  send_to_char (buf, ch);
	  sprintf (buf, "[v11] Def Bonus for Elems mobs:               [%d]\n\r", obj->value[11]);
	  send_to_char (buf, ch);
	  sprintf (buf, "[v12] Terrain Toggles:                        %s\n\r",flag_string (card_terrain_toggles, obj->value[12]));
	  send_to_char (buf, ch);
	  break;
	case CARD_SPELL_HEAL:
	  sprintf (buf, "[v7] Heal Amount:     [%d]\n\r",obj->value[7]);
	  send_to_char (buf,ch);
	  break;
	case CARD_SPELL_DAMAGE:
          sprintf (buf, "[v7] Damage Amount:   [%d]\n\r",obj->value[7]);
          send_to_char (buf,ch);
	  sprintf (buf, "[v8] Element:         %s\n\r",flag_string (card_element_type, obj->value[8]));
          send_to_char (buf,ch);
	  break;
	case CARD_SPELL_BOOST:
	  sprintf (buf, "[v7] Off Bonus:       [%d]\n\r",obj->value[7]);
          send_to_char (buf,ch);
          sprintf (buf, "[v8] Def Bonus:       [%d]\n\r",obj->value[8]);
          send_to_char (buf,ch);
          break;
	case CARD_SPELL_COUNTER:
	  break;
      }
   

					 
    }
  return;
}

bool
set_obj_values (CHAR_DATA * ch, OBJ_INDEX_DATA * pObj, int value_num,
		char *argument)
{
  int value;
  int type;
  int clan_num;
  char buf[MAX_STRING_LENGTH];
  switch (pObj->item_type)
    {
    default:
      break;
    case ITEM_WRITING_PAPER:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_WRITING_PAPER");
	  return FALSE;
	case 0:
	  send_to_char ("Hours message lasts set.\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	}
      break;
    case ITEM_LIGHT:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_LIGHT");
	  return FALSE;
	case 2:
	  send_to_char ("HOURS OF LIGHT SET.\n\r\n\r", ch);
	  pObj->value[2] = atoi (argument);
	  break;
	}
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_STAFF_WAND");
	  return FALSE;
	case 0:
	  if (!is_number (argument))
	    {
	      send_to_char
		("Affect level takes a positive numeric argument.\n\r", ch);
	      return FALSE;
	    }
	  send_to_char ("Affect level set.\n\r\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	case 1:
	  if (!is_number (argument))
	    {
	      send_to_char
		("Max charges takes a positive numeric argument.\n\r", ch);
	      return FALSE;
	    }
	  send_to_char ("Maximum number of charges set.\n\r\n\r", ch);
	  pObj->value[1] = atoi (argument);
	  break;
	case 2:
	  if (!is_number (argument))
	    {
	      send_to_char
		("Current charges takes a positive numeric argument.\n\r",
		 ch);
	      return FALSE;
	    }
	  send_to_char ("Current number of charges set.\n\r\n\r", ch);
	  pObj->value[2] = atoi (argument);
	  break;
	case 3:
	  send_to_char ("Spell/Song/Prayer/Chant type set.\n\r", ch);
	  pObj->value[3] = skill_lookup (argument);
	  break;
	}
      break;
    case ITEM_PORTAL:
      switch (value_num)
	{
	case 0:
	  send_to_char ("PORTAL CHARGES SET.\n\r\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	case 1:
	  if ((value = flag_value (exit_flags, argument)) != NO_FLAG)
	    {
	      TOGGLE_BIT (pObj->value[1], value);
	      send_to_char ("PORTAL EXIT FLAGS SET.\n\r\n\r", ch);
	    }

	  else
	    send_to_char ("PORTAL EXIT FLAGS ERROR.\n\r\n\r", ch);
	  break;
	case 2:
	  if ((value = flag_value (gate_flags, argument)) != NO_FLAG)
	    {
	      TOGGLE_BIT (pObj->value[2], value);
	      send_to_char ("PORTAL GATE FLAGS SET.\n\r\n\r", ch);
	    }

	  else
	    send_to_char ("PORTAL GATE FLAGS ERROR.\n\r\n\r", ch);
	  break;
	case 3:
	  send_to_char ("PORTAL VNUM-TO SET.\n\r\n\r", ch);
	  pObj->value[3] = atoi (argument);
	  break;
	case 4:
	  if ((value = flag_value (pact_flags, argument)) != NO_FLAG)
	    {
	      TOGGLE_BIT (pObj->value[4], value);
	      send_to_char ("PORTAL ACTION FLAGS SET.\n\r\n\r", ch);
	    }

	  else
	    send_to_char ("PORTAL ACTION FLAGS ERROR.\n\r\n\r", ch);
	  break;
	}
      break;
    case ITEM_PORTAL_BOOK:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_PORTAL_BOOK");
	  return FALSE;
	case 0:
	  if (atoi (argument) % 2 != 0 && atoi (argument) != -1)
	    {
	      send_to_char
		("Portal-book charges must be an even number or -1.\n\r\n\r",
		 ch);
	      return (FALSE);
	    }
	  send_to_char ("Portal-book charges set.\n\r\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	case 1:
	  send_to_char ("Portal-book vnum-to set.\n\r\n\r", ch);
	  pObj->value[1] = atoi (argument);
	  break;
	}
      break;
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_SCROLL_POTION_PILL");
	  return FALSE;
	case 0:
	  if (!is_number (argument))
	    {
	      send_to_char
		("Spell level takes a positive numeric argument.\n\r", ch);
	      return FALSE;
	    }
	  send_to_char ("Spell Level Set.\n\r\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	case 1:
	  if ((pObj->value[1] = skill_lookup (argument)) <= 0)
	    {
	      send_to_char ("Spell/Song/Prayer/Chant not found.\n\r", ch);
	      return FALSE;
	    }
	  if (skill_table[pObj->value[1]].type == SKILL_NORMAL)
	    {
	      send_to_char
		("'Normal' skills can not be associated with this object type.\n\r",
		 ch);
	      pObj->value[1] = 0;
	      return FALSE;
	    }
	  send_to_char ("Spell/Song/Prayer/Chant-1 SET.\n\r\n\r", ch);
	  break;
	case 2:
	  if ((pObj->value[2] = skill_lookup (argument)) <= 0)
	    {
	      send_to_char ("Spell/Song/Prayer/Chant not found.\n\r", ch);
	      return FALSE;
	    }
	  if (skill_table[pObj->value[2]].type == SKILL_NORMAL)
	    {
	      send_to_char
		("'Normal' skills can not be associated with this object type.\n\r",
		 ch);
	      pObj->value[2] = 0;
	      return FALSE;
	    }
	  send_to_char ("Spell/Song/Prayer/Chant-2 SET.\n\r\n\r", ch);
	  break;
	case 3:
	  if ((pObj->value[3] = skill_lookup (argument)) <= 0)
	    {
	      send_to_char ("Spell/Song/Prayer/Chant not found.\n\r", ch);
	      return FALSE;
	    }
	  if (skill_table[pObj->value[3]].type == SKILL_NORMAL)
	    {
	      send_to_char
		("'Normal' skills can not be associated with this object type.\n\r",
		 ch);
	      pObj->value[3] = 0;
	      return FALSE;
	    }
	  send_to_char ("Spell/Song/Prayer/Chant-3 SET.\n\r\n\r", ch);
	  break;
	case 4:
	  if ((pObj->value[4] = skill_lookup (argument)) <= 0)
	    {
	      send_to_char ("Spell/Song/Prayer/Chant not found.\n\r", ch);
	      return FALSE;
	    }
	  if (skill_table[pObj->value[4]].type == SKILL_NORMAL)
	    {
	      send_to_char
		("'Normal' skills can not be associated with this object type.\n\r",
		 ch);
	      pObj->value[4] = 0;
	      return FALSE;
	    }
	  send_to_char ("Spell/Song/Prayer/Chant-4 SET.\n\r\n\r", ch);
	  break;
	}
      break;

/* ARMOR for ROM: */
    case ITEM_ARMOR:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_ARMOR");
	  return FALSE;
	case 0:
	  send_to_char ("AC PIERCE SET.\n\r\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	case 1:
	  send_to_char ("AC BASH SET.\n\r\n\r", ch);
	  pObj->value[1] = atoi (argument);
	  break;
	case 2:
	  send_to_char ("AC SLASH SET.\n\r\n\r", ch);
	  pObj->value[2] = atoi (argument);
	  break;
	case 3:
	  send_to_char ("AC EXOTIC SET.\n\r\n\r", ch);
	  pObj->value[3] = atoi (argument);
	  break;
	}
      break;
    case ITEM_CTRANSPORT:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_CTRANSPORT");
	  break;
	case 0:
	  if ((value = flag_value (tsector_flags, argument)) != NO_FLAG)
	    {
	      TOGGLE_BIT (pObj->value[0], value);
	      send_to_char
		("Controlled transport sector types set.\n\r\n\r", ch);
	    }

	  else
	    send_to_char
	      ("Controlled transport sector types error.\n\r\n\r", ch);
	  break;
	case 1:
	  send_to_char ("Max weight set.\n\r", ch);
	  pObj->value[1] = atoi (argument);
	  break;
	case 2:
	  send_to_char ("Entrance/Exit vnum set.\n\r", ch);
	  pObj->value[2] = atoi (argument);
	  break;
	case 3:
	  send_to_char ("Cockpit vnum set.\n\r", ch);
	  pObj->value[3] = atoi (argument);
	  break;
	case 4:
	  if ((value = flag_value (tact_flags, argument)) != NO_FLAG)
	    {
	      TOGGLE_BIT (pObj->value[4], value);
	      send_to_char
		("Controlled transport action type set.\n\r\n\r", ch);
	    }

	  else
	    send_to_char
	      ("Controlled transport action type error.\n\r\n\r", ch);
	  break;
	case 5:
	  if ((value = flag_value (container_flags, argument)) != NO_FLAG)
	    {
	      TOGGLE_BIT (pObj->value[5], value);
	      send_to_char
		("Controlled transport door flags set.\n\r\n\r", ch);
	    }

	  else
	    send_to_char
	      ("Controlled transport door flags error.\n\r\n\r", ch);
	  break;
	case 6:
	  send_to_char ("Key vnum set.\n\r", ch);
	  pObj->value[6] = atoi (argument);
	  break;
	}
      break;
    case ITEM_ELEVATOR:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_ELEVATOR");
	  break;
	case 0:
	  send_to_char ("Elevator holding room set.\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	case 1:
	  send_to_char ("Elevator max-people set.\n\r", ch);
	  pObj->value[1] = atoi (argument);
	  break;
	case 2:
	  send_to_char ("Elevator max-weight set.\n\r", ch);
	  pObj->value[2] = atoi (argument);
	  break;
	}
      break;
    case ITEM_EBUTTON:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_EBUTTON");
	  break;
	case 0:
	  send_to_char ("Elevator vnum set.\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	case 1:
	  send_to_char ("Button 'Floor' set.\n\r", ch);
	  pObj->value[1] = atoi (argument);
	  break;
	case 2:
	  send_to_char ("Elevator goto set.\n\r", ch);
	  pObj->value[2] = atoi (argument);
	  break;
	case 3:
	  if ((value = flag_value (button_flags, argument)) != NO_FLAG)
	    {
	      TOGGLE_BIT (pObj->value[3], value);
	      send_to_char ("Button action type set.\n\r\n\r", ch);
	    }

	  else
	    send_to_char ("Button action type error.\n\r\n\r", ch);
	  break;
	}
      break;
    case ITEM_INSTRUMENT:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_INSTRUMENT");
	  return FALSE;
	case 0:
	  if ((value = flag_value (instrument_flags, argument)) != NO_FLAG)
	    {
	      TOGGLE_BIT (pObj->value[0], value);
	      send_to_char ("Instrument flags set.\n\r\n\r", ch);
	    }

	  else
	    send_to_char ("Instrument flags error.\n\r\n\r", ch);
	  break;
	}
      break;
    case ITEM_KEY:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_KEY");
	  return FALSE;
	case 0:
	  if (!str_cmp (argument, "y") || !str_cmp (argument, "yes"))
	    pObj->value[0] = TRUE;

	  else
	    pObj->value[0] = FALSE;
	  send_to_char ("Key-Save Value Set.\n\r", ch);
	  break;
	}
      break;
    case ITEM_PLAYER_DONATION:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_PLAYER_DONATION");
	  return FALSE;
	case 0:
	  send_to_char ("Max. items value changed.\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	}
      break;
    case ITEM_QUIVER:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_QUIVER");
	  return FALSE;
	case 0:
	  send_to_char ("Max. items value changed.\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	}
      break;
      //Iblis /6/10/04 - Added a new object to deal with card games
    case ITEM_BINDER:
      switch (value_num)
        {
 	  default:
	    do_help (ch, "ITEM_BINDER");
	    return FALSE;
	  case 0:
	    send_to_char ("Capacity changed.\n\r", ch);
	    pObj->value[0] = atoi (argument);
	    break;
	}
      break;
    case ITEM_CLAN_DONATION:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_CLAN_DONATION");
	  return FALSE;
	case 0:
	  send_to_char ("Max. items value changed.\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	case 1:
	  if (argument[0] == '\0')
	    {
	      pObj->value[0] = 0;
	      pObj->value[1] = CLAN_BOGUS;
	      send_to_char ("Clan ownership flag removed.\n\r", ch);
	      return TRUE;
	    }
	  clan_num = clan_lookup (argument);
	  if (clan_num == CLAN_BOGUS)
	    {
	      send_to_char ("No such clan exists.\n\r", ch);
	      return FALSE;
	    }
	  pObj->value[1] = clan_num;
	  sprintf (buf,
		   "Clan ownership is now set to %s`` for this object.\n\r",
		   get_clan_name (clan_num));
	  send_to_char (buf, ch);
	  return TRUE;
	}
      break;
    case ITEM_WEAPON:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_WEAPON");
	  return FALSE;
	case 0:
	  send_to_char ("WEAPON CLASS SET.\n\r\n\r", ch);
	  pObj->value[0] = flag_value (weapon_Class, argument);
	  break;
	case 1:
	  if (atoi (argument) > 3000)
	    send_to_char ("Value too large.\n\r", ch);

	  else
	    {
	      send_to_char ("NUMBER OF DICE SET.\n\r\n\r", ch);
	      pObj->value[1] = atoi (argument);
	    }
	  break;
	case 2:
	  if (atoi (argument) > 3000)
	    send_to_char ("Value too large.\n\r", ch);

	  else
	    {
	      send_to_char ("TYPE OF DICE SET.\n\r\n\r", ch);
	      pObj->value[2] = atoi (argument);
	    }
	  break;
	case 3:
	  send_to_char ("WEAPON TYPE SET.\n\r\n\r", ch);
	  pObj->value[3] = flag_value (weapon_flags, argument);
	  break;
	case 4:
	  send_to_char ("SPECIAL WEAPON TYPE SET.\n\r\n\r", ch);
	  pObj->value[4] = flag_value (weapons_type, argument);
	  if (pObj->value[0] == WEAPON_DICE)
	  {
	    if (pObj->value[4] & WEAPON_SHARP)
  	      pObj->value[4] -= WEAPON_SHARP;
	  }
	  break;
	case 5:
	  if (pObj->value[0] != WEAPON_DICE)
	    break;
	  {
  	  int sn = skill_lookup(argument);
          if (sn > MAX_SKILL)
            send_to_char ("Value too large.\n\r", ch);
	  pObj->value[5] = sn;
	  send_to_char("SPELL SET.\n\r\n\r",ch);
	  }
	  break;
	}
      break;
    case ITEM_FURNITURE:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_FURNITURE");
	  return FALSE;
	case 0:
	  send_to_char ("NUMBER OF PEOPLE SET.\n\r\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	case 1:
	  send_to_char ("MAXIMUM WEIGHT SET.\n\r\n\r", ch);
	  pObj->value[1] = atoi (argument);
	  break;
	case 2:
	  if ((value = flag_value (furniture_type, argument)) != NO_FLAG)
	    {
	      TOGGLE_BIT (pObj->value[2], value);
	      send_to_char ("FURNITURE FLAGS SET.\n\r\n\r", ch);
	    }

	  else
	    send_to_char ("FURNITURE FLAGS ERROR.\n\r\n\r", ch);
	  break;
	case 3:
	  send_to_char ("HEALING BONUS SET.\n\r\n\r", ch);
	  pObj->value[3] = atoi (argument);
	  break;
	case 4:
	  send_to_char ("MANA BONUS SET.\n\r\n\r", ch);
	  pObj->value[4] = atoi (argument);
	  break;
	}
      break;
    case ITEM_RAFT:
      switch (value_num)
        {
        default:
          do_help (ch, "ITEM_FURNITURE");
          return FALSE;
        case 0:
          send_to_char ("NUMBER OF PEOPLE SET.\n\r\n\r", ch);
          pObj->value[0] = atoi (argument);
          break;
        case 1:
          send_to_char ("MAXIMUM WEIGHT SET.\n\r\n\r", ch);
          pObj->value[1] = atoi (argument);
          break;
        case 2:
          if ((value = flag_value (furniture_type, argument)) != NO_FLAG)
            {
              TOGGLE_BIT (pObj->value[2], value);
              send_to_char ("FURNITURE FLAGS SET.\n\r\n\r", ch);
            }

          else
            send_to_char ("FURNITURE FLAGS ERROR.\n\r\n\r", ch);
          break;
	case 3:
          if ((value = flag_value (tsector_flags, argument)) != NO_FLAG)
            {
              TOGGLE_BIT (pObj->value[3], value);
              send_to_char
                ("Controlled transport sector types set.\n\r\n\r", ch);
            }

          else
            send_to_char
              ("Controlled transport sector types error.\n\r\n\r", ch);
          break;

        }
      break;

    case ITEM_CONTAINER:
      switch (value_num)
	{
	  int value;
	default:
	  do_help (ch, "ITEM_CONTAINER");
	  return FALSE;
	case 0:
	  send_to_char ("WEIGHT CAPACITY SET.\n\r\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	case 1:
	  if ((value = flag_value (container_flags, argument)) != NO_FLAG)
	    TOGGLE_BIT (pObj->value[1], value);

	  else
	    {
	      do_help (ch, "ITEM_CONTAINER");
	      return FALSE;
	    }
	  send_to_char ("CONTAINER TYPE SET.\n\r\n\r", ch);
	  break;
	case 2:
	  send_to_char ("Key vnum set.\n\r\n\r", ch);
	  pObj->value[2] = atoi (argument);
	  break;
	case 3:
	  send_to_char
	    ("MAX WEIGHT CAPACITY OF A SINGLE OBJECT SET.\n\r\n\r", ch);
	  pObj->value[3] = atoi (argument);
	  break;
	case 4:
	  send_to_char ("WEIGHT MULTIPLIER SET.\n\r\n\r", ch);
	  pObj->value[4] = atoi (argument);
	  break;
	}
      break;
    case ITEM_PACK:
      switch (value_num)
	{
	  int value;
	default:
	  do_help (ch, "ITEM_PACK");
	  return FALSE;
	case 0:
	  send_to_char ("WEIGHT CAPACITY SET.\n\r\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	case 1:
	  if ((value = flag_value (container_flags, argument)) != NO_FLAG)
	    TOGGLE_BIT (pObj->value[1], value);

	  else
	    {
	      do_help (ch, "ITEM_PACK");
	      return FALSE;
	    }
	  send_to_char ("PACK TYPE SET.\n\r\n\r", ch);
	  break;
	case 3:
	  send_to_char
	    ("MAX WEIGHT CAPACITY OF A SINGLE OBJECT SET.\n\r\n\r", ch);
	  pObj->value[3] = atoi (argument);
	  break;
	case 4:
	  send_to_char ("WEIGHT MULTIPLIER SET.\n\r\n\r", ch);
	  pObj->value[4] = atoi (argument);
	  break;
	}
      break;
    case ITEM_DRINK_CON:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_DRINK");

/* OLC              do_help( ch, "liquids" );    */
	  return FALSE;
	case 0:
	  send_to_char ("MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	case 1:
	  send_to_char ("CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch);
	  pObj->value[1] = atoi (argument);
	  break;
	case 2:
	  send_to_char ("LIQUID TYPE SET.\n\r\n\r", ch);
	  pObj->value[2] = flag_value (liquid_flags, argument);
	  if (pObj->value[2] < 0)
	    pObj->value[2] = 0;
	  break;
	case 3:
	  send_to_char ("POISON VALUE TOGGLED.\n\r\n\r", ch);
	  pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
	  break;
	}
      break;
    case ITEM_FOUNTAIN:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_DRINK");

/* OLC              do_help( ch, "liquids" );    */
	  return FALSE;
	case 2:
	  send_to_char ("LIQUID TYPE SET.\n\r\n\r", ch);
	  pObj->value[2] = flag_value (liquid_flags, argument);
	  if (pObj->value[2] < 0)
	    pObj->value[2] = 0;
	  break;
	case 3:
	  send_to_char ("POISON VALUE TOGGLED.\n\r\n\r", ch);
	  pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
	  break;
	}
      break;
    case ITEM_FOOD:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_FOOD");
	  return FALSE;
	case 0:
	  send_to_char ("Hours of food (full) set.\n\r\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	case 1:
	  send_to_char ("Hours of food (not hungry) set.\n\r\n\r", ch);
	  pObj->value[1] = atoi (argument);
	  break;
	case 2:
	  send_to_char ("Hours until rot set.\n\r\n\r", ch);
	  pObj->value[2] = atoi (argument);
	  break;
	case 3:
	  send_to_char ("Poison value toggled.\n\r\n\r", ch);
	  pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
	  break;
	}
      break;
    case ITEM_MONEY:
      switch (value_num)
	{
	default:
	  do_help (ch, "ITEM_MONEY");
	  return FALSE;
	case 0:
	  send_to_char ("SILVER AMOUNT SET.\n\r\n\r", ch);
	  pObj->value[0] = atoi (argument);
	  break;
	case 1:
	  send_to_char ("GOLD AMOUNT SET.\n\r\n\r", ch);
	  pObj->value[1] = atoi (argument);
	  break;
	}
      break;
    case ITEM_WOOD:
      {
      int i=0;
      for (i=0;i < MAX_TREES; ++i)
      {
        if (!str_prefix(argument,tree_table[i].name))
           break;
      }
      if (i >= MAX_TREES)
      {
        send_to_char("Invalid Choice!\n\r\n\r",ch);
	send_to_char("Choices are : '",ch);
	for (i=0;i < MAX_TREES; ++i)
	{
	  if (i > 0)
	  {
 	    if (i%6 == 3)
		    send_to_char("\n\r'",ch);
	    else send_to_char(" '",ch);
	  }
	  send_to_char(tree_table[i].name,ch);
	  send_to_char("'",ch);
	}
	send_to_char("\n\r\n\r",ch);
	return FALSE;
      }
      pObj->value[0] = i;
      send_to_char("Type set!\n\r\n\r",ch);
      break;
      }
    case ITEM_OBJ_TRAP:
    case ITEM_ROOM_TRAP:
    case ITEM_PORTAL_TRAP:
      switch(value_num)
      {
	default: 
		send_to_char("See ? weapon for damage types, and ? for trap types, the rest are numbers.\n\r",ch);
		break;
	case 0:
		value = flag_value (trap_type, argument);
	  	if (value < MIN_OBJ_TRAP || value > MAX_ROOM_TRAP)
		{
			send_to_char("Not a valid trap type. (? traptype)\n\r",ch);
			return FALSE;
		}else if (pObj->item_type == ITEM_OBJ_TRAP &&(value < MIN_OBJ_TRAP || value > MAX_OBJ_TRAP))
		{
			send_to_char("Not a valid object trap type. (? traptype)\n\r",ch);
			return FALSE;
		}else if (pObj->item_type == ITEM_PORTAL_TRAP &&(value < MIN_PORTAL_TRAP || value > MAX_PORTAL_TRAP))
		{
			send_to_char("Not a valid portal trap type. (? traptype)\n\r",ch);
			return FALSE;
		}else if (pObj->item_type == ITEM_ROOM_TRAP &&(value < MIN_ROOM_TRAP || value > MAX_ROOM_TRAP))
		{
			send_to_char("Not a valid room trap type.\n\r",ch);
			return FALSE;
		}else
		{
		send_to_char ("Trap Type Set.\n\r",ch);
		pObj->value[0] = value;
		if (pObj->value[0] == TRAP_R_SPELL ||pObj->value[0] == TRAP_P_SPELL ||pObj->value[0] == TRAP_O_SPELL)
		{
			pObj->value[1] = 0; // No Spell
			pObj->value[2] = 0;  // Spell level = 0
		}else
		{
  		  EXTRA_DESCR_DATA *ed;
			type = trap_info_lookup(pObj->value[0]);
			if (type != NO_FLAG)
			{
			   pObj->value[1] = trapinfo[type].num_dice;
			   pObj->value[2] = trapinfo[type].size_dice;
			   pObj->value[3] = trapinfo[type].mod_dice;
			   pObj->value[8] = flag_value (weapon_flags, trapinfo[type].damtype);
	  		   pObj->value[12] = trapinfo[type].flags;
			   for (ed = pObj->extra_descr; ed; ed = ed->next)
    			   {
			       if (is_name ("specmessage", ed->keyword) )
			          break;
   			   }			
				  
			   if (ed != NULL)
		      	      ed->description = str_dup (trapinfo[type].smess);

		  	   for (ed = pObj->extra_descr; ed; ed = ed->next)
         		   {
			      if ( is_name ("victmessage", ed->keyword))
				  break;
			   }		
 			   if (ed != NULL)
	      		      ed->description = str_dup (trapinfo[type].vmess);
			
			}
			else
			{
			pObj->value[1] = 0; //number of dice
			pObj->value[2] = 0; // size of dice
			pObj->value[3] = 0; // dam modifier
			}
		}
		}
		break;
	case 1:
		switch(pObj->value[0])
		{
		case 0:
		   send_to_char("Set trap type first.\n\r",ch);
		   break;
		case TRAP_R_SPELL:
		case TRAP_O_SPELL:
		case TRAP_P_SPELL:
			
		  	if ((pObj->value[1] = skill_lookup (argument)) <= 0)
	    		{
	      			send_to_char ("Spell/Song/Prayer/Chant not found.\n\r", ch);
	      		  	pObj->value[1] = 0;
	      			return FALSE;
	    		}
	  		if (skill_table[pObj->value[1]].type == SKILL_NORMAL)
	   		{
	      		  	send_to_char ("'Normal' skills can not be associated with this object type.\n\r", ch);
	      		  	pObj->value[1] = 0;
	      		 	return FALSE;
	    		}
	  		send_to_char ("Spell/Song/Prayer/Chant SET.\n\r\n\r", ch);
	  		break;
		default:
			send_to_char("Number of Dice Set.\n\r",ch);
			pObj->value[1] = atoi (argument);
			break;
		}
		break;
	case 2:
		switch(pObj->value[0])
		{
		case 0:
		   send_to_char("Set trap type first.\n\r",ch);
		   break;
		case TRAP_R_SPELL:
		case TRAP_O_SPELL:
		case TRAP_P_SPELL:
			send_to_char("Spell Level Set.\n\r",ch);
			pObj->value[2] = atoi (argument);
			break;
		default:
			send_to_char("Type of Dice Set.\n\r",ch);
			pObj->value[2] = atoi (argument);
			break;
		}
		break;
	case 3:
		switch(pObj->value[0])
		{
		case 0:
		   send_to_char("Set trap type first.\n\r",ch);
		   break;
		case TRAP_R_SPELL:
		case TRAP_O_SPELL:
		case TRAP_P_SPELL:
			send_to_char("Option not setable for Spell Traps.\n\r",ch);
			break;
		default:
			send_to_char("Damage modifer set.\n\r",ch);
			pObj->value[3] = atoi (argument);
			break;
		}
		break;
	case 5:
		pObj->value[5] = atoi (argument);
		if(pObj->value[5]<1 || pObj->value[5]>25)
		{
			send_to_char("Allowed range is 1-25.\n\r",ch);
			pObj->value[5] = 0;
			return FALSE;
		}
		send_to_char("Trap Dexterity set.\n\r",ch);
		
		break;
	case 6:
		send_to_char("Summon mob vnum set.\n\r",ch);
		pObj->value[6] = atoi (argument);
		break;
	case 7:
		if (pObj->value[0] >= MIN_PORTAL_TRAP && pObj->value[0] <= MAX_PORTAL_TRAP)
		{
			if (!strcmp("none",argument))
				pObj->value[7] = -1;
			else
			{
				send_to_char("Exit type set.\n\r",ch);
				pObj->value[7] = flag_value(pact_flags,argument);
			}
		}
		break;
	case 8:
		send_to_char("Damage type set .\n\r",ch);
		pObj->value[8] = flag_value (weapon_flags, argument);
		break;
	case 9: if (pObj->value[0] >= MIN_PORTAL_TRAP && pObj->value[0] <= MAX_PORTAL_TRAP)
		{
			if (!strcmp("none",argument))
				pObj->value[9] = -1;
			else
			{
				send_to_char("Door direction set.\n\r",ch);
				pObj->value[9] = flag_value(dir_flags,argument);
			}
		}
		break;
	}
      	// 10 is the door/
	// 11 is used for Rearm timer ticker
	// 12 stores trap flags 
	break;		 
    case ITEM_CARD:
      switch (value_num)
      {
       default:
         switch (pObj->value[1])
	 {
  	   case CARD_MOB:
 	     switch (value_num)
	     {
 	       case 7:
		 send_to_char ("Hitpoints Set.\n\r\n\r",ch);
		 pObj->value[7] = atoi (argument);
		 if (pObj->value[7] < 1 || pObj->value[7] > 10)
  	  	   pObj->value[7] = 1;
		 break;
	       case 8:
		 send_to_char ("Offence Set.\n\r\n\r",ch);
		 pObj->value[8] = atoi (argument);
		 if (pObj->value[8] < 0 || pObj->value[8] > 10)
                   pObj->value[8] = 0;
		 break;
	       case 9:
		 send_to_char ("Defence Set.\n\r\n\r",ch);
		 pObj->value[9] = atoi (argument);
		 if (pObj->value[9] < 0 || pObj->value[8] > 10)
                   pObj->value[9] = 0;
		 break;
	       case 10:
		 send_to_char ("Race Set.\n\r\n\r",ch);
		 pObj->value[10] = flag_value (card_race_type, argument);
		 break;
	       case 11:
		 send_to_char ("Element Set.\n\r\n\r",ch);
		 pObj->value[11] = flag_value (card_element_type, argument);
		 break;
	       case 12:
		 send_to_char ("Nemesis Set.\n\r\n\r",ch);
		 pObj->value[12] = flag_value (card_race_type, argument);
		 break;
	       default :
		 do_help(ch,"card_options");
		 return FALSE;
	     }
	     break;
	   case CARD_TERRAIN:
	     switch (value_num)
	     {
  	       case 7: 
		 send_to_char ("Terrain Type Set.\n\r\n\r",ch);
		 pObj->value[7] = flag_value (card_terrain_type, argument);
		 break;
	       case 8:
		 send_to_char ("Offence Bonus For All Set.\n\r\n\r",ch);
		 pObj->value[8] = atoi (argument);
		 if (pObj->value[8] > 3 || pObj->value[8] < -3)
  	  	   pObj->value[8] = 0;
		 break;
	       case 9:
		 send_to_char ("Defence Bonus For All Set.\n\r\n\r",ch);
		 pObj->value[9] = atoi (argument);
		 if (pObj->value[9] > 3 || pObj->value[9] < -3)
                   pObj->value[9] = 0;
		 break;
	       case 10:
		 send_to_char ("Offence Bonus for Elementals Set.\n\r\n\r",ch);
		 pObj->value[10] = atoi (argument);
		 if (pObj->value[10] > 3 || pObj->value[10] < -3)
  		   pObj->value[10] = 0;
		 break;
	       case 11:
		 send_to_char ("Defence Bonus for Elementals Set.\n\r\n\r",ch);
		 pObj->value[11] = atoi (argument);
		 if (pObj->value[11] > 3 || pObj->value[11] < -3)
  	  	   pObj->value[11] = 0;
		 break;
	       case 12:
		 send_to_char ("Terrain Toggles Set.\n\r\n\r",ch);
		 pObj->value[12] = flag_value (card_terrain_toggles, argument);
		 break;
	       default :
		 do_help(ch,"card_options");
		 return FALSE;
	     }
	     break;
	   case CARD_SPELL_HEAL:
	     if (value_num == 7)
	     {
  	       send_to_char ("Heal Amount Set.\n\r\n\r",ch);
	       pObj->value[7] = atoi (argument);
	       if (pObj->value[7] < 1 || pObj->value[7] > 10)
                 pObj->value[7] = 1;
	     }
	     else 
	     {
	       do_help(ch,"card_options");
  	       return FALSE;
	     }
	     break;
           case CARD_SPELL_DAMAGE:
	     switch (value_num)
	     {
  	       case 7: 
		 send_to_char ("Damage Amount Set.\n\r\n\r",ch);
		 pObj->value[7] = atoi (argument);
		 if (pObj->value[7] < 1 || pObj->value[7] > 10)
                   pObj->value[7] = 1;
 	         break;
	       case 8:
		 send_to_char ("Element Set.\n\r\n\r",ch);
		 pObj->value[8] = flag_value (card_element_type, argument);
		 break;
	       default :
		 do_help(ch,"card_options");
		 return FALSE;
	     }
	     break;
	   case CARD_SPELL_BOOST:
	     switch (value_num)
	     {
  	       case 7:
  	         send_to_char ("Offensive Bonus Set.\n\r\n\r",ch);
		 if (pObj->value[7] > 3 || pObj->value[7] < -3)
		   pObj->value[7] = 0;
		 pObj->value[7] = atoi (argument);
		 break;
	       case 8:
		 send_to_char ("Defensive Bonus Set.\n\r\n\r",ch);
		 if (pObj->value[8] > 3 || pObj->value[8] < -3)
		   pObj->value[8] = 0;
		 pObj->value[8] = atoi (argument);
		 break;
	       default :
		 do_help(ch,"card_options");
		 return FALSE;
	     }
	     break;
	   default :
	     do_help(ch,"card_options");
             return FALSE;
	 }
	 break;
       case 0:
	 {
	 int vnum=0, nMatch=0;
	 OBJ_INDEX_DATA *pObjIndex;
	 if (atoi (argument) > 9999 || atoi (argument) < 0)
	 {
	   send_to_char("Card number out of range!\n\r",ch);
	   return FALSE;
	 }
	 for (vnum = 0; nMatch < top_obj_index; vnum++)
	 {
		 if (vnum > 40000)
		 {
			 bug("Too many vnums in olc_act.c",0);
			 return FALSE;
		 }
		 if ((pObjIndex = get_obj_index (vnum)) != NULL)
		 {
			 nMatch++;
			 if (pObjIndex->item_type != ITEM_CARD)
				 continue;
			 if (pObjIndex->value[0] == atoi (argument))
			 {
				 sprintf(buf,"A card (vnum %d) already has that card number.\n\r",vnum);
				 send_to_char(buf,ch);
				 return FALSE;
			 }
		 }
	 }
	 }
	 send_to_char ("Card Number Set.\n\r\n\r",ch);
	 pObj->value[0] = atoi (argument);
	 break;
       case 1:
	 send_to_char ("Card Type Set.\n\r\n\r",ch);
 	 pObj->value[1] = flag_value (card_Class, argument);
	 if (pObj->value[1] == CARD_MOB)
	   pObj->value[7] = 1; //Make sure hitpoints will never be 0 
	 break;
       case 2:
	 send_to_char ("Number of Dice Set.\n\r\n\r",ch);
	 pObj->value[2] = atoi (argument);
	 break;
       case 3:
	 send_to_char ("Type of Dice Set.\n\r\n\r",ch);
	 pObj->value[3] = atoi (argument);
	 break;
       case 4:
	 send_to_char ("Bonus to Dice Set.\n\r\n\r",ch);
	 pObj->value[4] = atoi (argument);
	 break;
       case 5:
	 send_to_char ("Level of Spell Set.\n\r\n\r",ch);
         pObj->value[5] = atoi (argument);
         break;
       case 6:
         send_to_char ("Spell/Song/Prayer/Chant type set.\n\r", ch);
	 pObj->value[6] = skill_lookup (argument);
	 break;
      }

    }
  show_obj_values (ch, pObj);
  return TRUE;
}

OEDIT (oedit_show)
{
  OBJ_INDEX_DATA *pObj;
  char buf[MAX_STRING_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  AFFECT_DATA *paf;
  int cnt,i;
  EDIT_OBJ (ch, pObj);
  sprintf (buf, "      Name:  [%s]", pObj->name);
  sprintf (tcbuf, "%-40s| Area:  [%5d] %s\n\r", buf,
	   !pObj->area ? -1 : pObj->area->vnum,
	   !pObj->area ? "No Area" : pObj->area->name);
  send_to_char (tcbuf, ch);
  sprintf (buf, "      Vnum:  [%5d]", pObj->vnum);
  sprintf (tcbuf, "%-40s| Type:  [%s]\n\r", buf,
	   flag_string (type_flags, pObj->item_type));
  send_to_char (tcbuf, ch);
  sprintf (buf, "     Level:  [%5d]", pObj->level);
  sprintf (tcbuf, "%-40s| Rarity:  [%s]\n\r", buf,
	   flag_string (rarity_flags, pObj->rarity));
  send_to_char (tcbuf, ch);
  sprintf (buf, "  Material:  [%s]", pObj->material);
  sprintf (tcbuf, "%-40s| Condition:  [%5d]\n\r", buf, pObj->condition);
  send_to_char (tcbuf, ch);
  sprintf (buf, "    Weight:  [%5d]", pObj->weight);
  sprintf (tcbuf, "%-40s|      Cost:  [%5d]\n\r", buf, pObj->cost);
  send_to_char (tcbuf, ch);
  sprintf (buf, " WearFlags:  [%s]\n\r",
	   flag_string (wear_flags, pObj->wear_flags));
  send_to_char (buf, ch);

  // Akamai 4/30/99 - support Class/race object restrictions
  sprintf (buf, "Classes Allowed: [%s]\n\r",
	   crflag_string (Class_flags, pObj->Class_flags));
  send_to_char (buf, ch);
  sprintf (buf, "Races Allowed:   [%s]\n\r",
	   crflag_string (pcrace_flags, pObj->race_flags));
  send_to_char (buf, ch);
  sprintf (buf, "Clans Allowed:   [%s]\n\r",
	   crflag_string (clan_flags, pObj->clan_flags));
  send_to_char (buf, ch);

  // Adeon 6/29/03 -- support owner specific objects
  if (pObj->plr_owner != NULL)
    sprintf (buf, "Owner: [%s]\n\r", pObj->plr_owner);

  else
    sprintf (buf, "Owner: [%s]\n\r", "none");
  send_to_char (buf, ch);
//  sprintf (buf, "ExtraFlags: [%s]\n\r",
//	   flag_string2 (extra_flags, MAX_EXTRA_FLAGS, pObj->extra_flags));
  sprintf (buf, "ExtraFlags: [%s]\n\r",extra_bit_name (pObj->extra_flags));
  send_to_char (buf, ch);
  
  if (pObj->item_type == ITEM_OBJ_TRAP || pObj->item_type == ITEM_ROOM_TRAP || pObj->item_type == ITEM_PORTAL_TRAP )
  {
	  sprintf(buf,"Trap flags: %s\n\r",flag_string (trap_flag, pObj->value[12]));
	  send_to_char (buf,ch);	  
  }
  sprintf (buf, "ItemTimer: [%d]\n\r", pObj->timer);
  send_to_char (buf, ch);
  for (i=0;i<MAX_OBJ_TRIGS;i++)
  {
    if (pObj->obj_trig_vnum[i] != 0)
    {
      sprintf (buf, "Object Trigger %d vnum: [%d]\n\r",i, pObj->obj_trig_vnum[i]);
      send_to_char (buf, ch);
    }
  }
  if (pObj->item_type == ITEM_CTRANSPORT)
    {
      sprintf (buf, "Passenger message:  [%s]\n\r", pObj->string1);
      send_to_char (buf, ch);
    }

  else if (pObj->item_type == ITEM_PORTAL)
    {
      sprintf (buf, "Enter message (others):  [%s]\n\r", pObj->string1);
      send_to_char (buf, ch);
      sprintf (buf, "Enter message (character):  [%s]\n\r", pObj->string2);
      send_to_char (buf, ch);
      sprintf (buf, "Exit message (others):  [%s]\n\r", pObj->string3);
      send_to_char (buf, ch);
      sprintf (buf, "Exit message (character):  [%s]\n\r", pObj->string4);
      send_to_char (buf, ch);
    }
  if (pObj->item_type == ITEM_EBUTTON)
    {
      sprintf (buf, "Arrival message (inside):  [%s]\n\r", pObj->string1);
      send_to_char (buf, ch);
      sprintf (buf, "Arrival message (outside):  [%s]\n\r", pObj->string2);
      send_to_char (buf, ch);
      sprintf (buf, "Departure message (inside):  [%s]\n\r", pObj->string3);
      send_to_char (buf, ch);
      sprintf (buf, "Departure message (outside):  [%s]\n\r", pObj->string4);
      send_to_char (buf, ch);
    }
  if (pObj->extra_descr)
    {
      EXTRA_DESCR_DATA *ed;
      send_to_char ("ExDescKwd: ", ch);
      for (ed = pObj->extra_descr; ed; ed = ed->next)
	{
	  send_to_char ("[", ch);
	  send_to_char (ed->keyword, ch);
	  send_to_char ("]", ch);
	}
      send_to_char ("\n\r", ch);
    }
  sprintf (buf, "Short desc:  %s\n\r Long desc:\n\r     %s\n\r",
	   pObj->short_descr, pObj->description);
  send_to_char (buf, ch);
  {
    char *cptr;
    if (pObj->item_type == ITEM_WRITING_PAPER)
      {
	if ((cptr = get_extra_descr ("papermsg", pObj->extra_descr)) != NULL)
	  {
	    sprintf (buf, "Initial Message:\n%s\n\r", cptr);
	    send_to_char (buf, ch);
	  }

	else
	  send_to_char ("Initial Message: [NONE]\n\r", ch);
      }
  }
  for (cnt = 0, paf = pObj->affected; paf; paf = paf->next)
    {
      if (cnt == 0)
	{
	  send_to_char ("Number Modifier Affects\n\r", ch);
	  send_to_char ("------ -------- -------\n\r", ch);
	}
      if (paf->type == -1)
        sprintf (buf, "[%4d] %-8d %s\n\r", cnt, paf->modifier, flag_string (apply_flags, paf->location));
      else  
      {
        if (paf->where == TO_AFFECTS)
          sprintf (buf, "[%4d] perm     [%s]\n\r", cnt, flag_string (affect_flags, paf->bitvector));
	else if (paf->type > 0 && paf->type < MAX_SKILL)
  	  sprintf (buf, "[%4d] %-8d (%s)\n\r", cnt, paf->modifier, skill_table[paf->location].name);
	else  sprintf (buf, "[%4d] %-8d (ERROR)\n\r", cnt, paf->modifier);
      }
      send_to_char (buf, ch);
      cnt++;
    }
  show_obj_values (ch, pObj);
  return FALSE;
}


/*
 * Need to issue warning if flag isn't valid. -- does so now -- Hugin.
 */
OEDIT (oedit_addaffect)
{
  int value;
  OBJ_INDEX_DATA *pObj;
  AFFECT_DATA *pAf;
  char loc[MAX_STRING_LENGTH];
  char mod[MAX_STRING_LENGTH];
  EDIT_OBJ (ch, pObj);
  argument = one_argument (argument, loc);
  one_argument (argument, mod);
  if (loc[0] == '\0' || mod[0] == '\0' || !is_number (mod))
    {
      send_to_char ("Syntax:  addaffect [location] [#mod]\n\r", ch);
      return FALSE;
    }
  if ((value = flag_value (apply_flags, loc)) == NO_FLAG)
    {				/* Hugin */
      send_to_char ("Valid affects are:\n\r", ch);
      show_help (ch, "? affect");
      return FALSE;
    }
  pAf = new_affect ();
  pAf->location = value;
  pAf->modifier = atoi (mod);
  pAf->type = -1;
  pAf->duration = -1;
  pAf->bitvector = 0;
  pAf->composition = FALSE;
  pAf->comp_name = str_dup ("");
  pAf->next = pObj->affected;
  pObj->affected = pAf;
  send_to_char ("Affect added.\n\r", ch);
  return TRUE;
}

/*
 * Need to issue warning if flag isn't valid. -- does so now -- Hugin.
 */
OEDIT (oedit_addskillaffect)
{
  int value;
  OBJ_INDEX_DATA *pObj;
  AFFECT_DATA *pAf;
  char loc[MAX_STRING_LENGTH];
  char mod[MAX_STRING_LENGTH];
  EDIT_OBJ (ch, pObj);
  argument = one_argument (argument, loc);
  one_argument (argument, mod);
  if (loc[0] == '\0' || mod[0] == '\0' || !is_number (mod))
    {
      send_to_char ("Syntax:  addskillaffect [skill] [%bonus]\n\r", ch);
      return FALSE;
    }
  if ((value = skill_lookup(loc)) == -1)
    {                           /* Hugin */
      send_to_char ("That is not a valid skill.\n\r", ch);
      //      show_help (ch, "? affect");
      return FALSE;
    }

  pAf = new_affect ();
  pAf->where = TO_SKILL;
  pAf->location = value;
  pAf->modifier = atoi (mod);
  pAf->type = value;
  pAf->duration = -1;
  pAf->bitvector = 0;
  pAf->composition = FALSE;
  pAf->comp_name = str_dup ("");
  pAf->next = pObj->affected;
  pObj->affected = pAf;
  send_to_char ("Affect added.\n\r", ch);
  return TRUE;
}

OEDIT (oedit_addpermaffect)
{
  //  extern int top_affect;
  OBJ_INDEX_DATA *pObj;
  int affby, level;
  AFFECT_DATA *paf;
  char level_string[MAX_INPUT_LENGTH];//, item_string[MAX_INPUT_LENGTH];
  char affect_string[MAX_INPUT_LENGTH];
  EDIT_OBJ (ch, pObj);
  //argument = one_argument (argument, item_string);
  argument = one_argument (argument, affect_string);
  argument = one_argument (argument, level_string);
  if (affect_string[0] == '\0' || level_string[0] == '\0')
    {
      send_to_char ("Syntax: addpermaff <affect> <level>\n\r", ch);
      send_to_char ("Affect may be one of:\n\r", ch);
      show_help (ch, "? affect");
/*
      send_to_char
        ("      haste          invisible          detect_evil          detect_invis\n\r",
         ch);
      send_to_char
        ("      detect_magic   detect_hidden      detect_good          faerie_fire\n\r",
         ch);
      send_to_char
        ("      sanctuary      infrared           curse                poison\n\r",
         ch);
      send_to_char
        ("      sneak          hide               aqua_breathe         flying\n\r",
         ch);
      send_to_char
        ("      pass_door      plague             weaken               slow\n\r",
         ch);*/
      return FALSE;
    }
  level = atol (level_string);
  if ((affby = flag_value (affect_flags, affect_string)) != NO_FLAG)
    {
      paf = new_affect ();
      paf->where = TO_AFFECTS;
      paf->level = level;
      paf->duration = -1;
      paf->bitvector = affby;
      switch (affby)
	{
        default:
          free_affect (paf);
          send_to_char ("Huh?\n\r", ch);
          return FALSE;
          break;
        case AFF_INVISIBLE:
          send_to_char ("Adding invis affect to object.\n\r", ch);
          paf->type = skill_lookup ("invis");
          //paf->location = APPLY_NONE;
          //paf->modifier = 0;
          break;
        case AFF_DETECT_EVIL:
          send_to_char ("Adding detect evil affect to object.\n\r", ch);
          paf->type = skill_lookup ("detect evil");
          //paf->location = APPLY_NONE;
          //paf->modifier = 0;
          break;
        case AFF_DETECT_INVIS:
          send_to_char ("Adding detect invis affect to object.\n\r", ch);
          paf->type = skill_lookup ("detect invis");
          //paf->location = APPLY_NONE;
          //paf->modifier = 0;
          break;
        case AFF_DETECT_MAGIC:
          send_to_char ("Adding detect magic affect to object.\n\r", ch);
          paf->type = skill_lookup ("detect magic");
          //paf->location = APPLY_NONE;
          //paf->modifier = 0;
          break;
        case AFF_DETECT_HIDDEN:
          send_to_char ("Adding detect hidden affect to object.\n\r", ch);
          paf->type = skill_lookup ("detect hidden");
          //paf->location = APPLY_NONE;
          //paf->modifier = 0;
          break;
        case AFF_DETECT_GOOD:
          send_to_char ("Adding detect good affect to object.\n\r", ch);
          paf->type = skill_lookup ("detect good");
          //paf->location = APPLY_NONE;
          //paf->modifier = 0;
          break;
        case AFF_FAERIE_FIRE:
          send_to_char ("Adding faerie fire affect to object.\n\r", ch);
          paf->type = skill_lookup ("faerie fire");
          //paf->location = APPLY_AC;
          //paf->modifier = 2 * level;
	  break;
        case AFF_INFRARED:
          send_to_char ("Adding infrared affect to object.\n\r", ch);
          paf->type = skill_lookup ("infravision");
          //paf->location = APPLY_NONE;
          //paf->modifier = 0;
          break;
        case AFF_CURSE:
          send_to_char ("Adding curse affect to object.\n\r", ch);
          paf->type = skill_lookup ("curse");
          //paf->location = APPLY_HITROLL;
          //paf->modifier = -1 * (level / 8);
          break;
        case AFF_POISON:
          send_to_char ("Adding poison affect to object.\n\r", ch);
          paf->type = skill_lookup ("poison");
          //paf->location = APPLY_STR;
          //paf->modifier = -2;
          break;
        case AFF_SNEAK:
          send_to_char ("Adding sneak affect to object.\n\r", ch);
          paf->type = skill_lookup ("sneak");
          //paf->location = 0;
          //paf->modifier = 0;
          break;
        case AFF_HIDE:
          send_to_char ("Adding hide affect to object.\n\r", ch);
          paf->type = skill_lookup ("hide");
          //paf->location = 0;
          //paf->modifier = 0;
          break;
        case AFF_CAMOUFLAGE:
          send_to_char ("Adding camouflage affect to object.\n\r", ch);
          paf->type = skill_lookup ("camouflage");
          //paf->location = 0;
          //paf->modifier = 0;
          break;
        case AFF_FLYING:
          send_to_char ("Adding fly affect to object.\n\r", ch);
          paf->type = skill_lookup ("fly");
          //paf->location = 0;
          //paf->modifier = 0;
          break;
	case AFF_PASS_DOOR:
          send_to_char ("Adding pass door affect to object.\n\r", ch);
          paf->type = skill_lookup ("pass door");
          //paf->location = APPLY_NONE;
          //paf->modifier = 0;
          break;
        case AFF_PLAGUE:
          send_to_char ("Adding plague affect to object.\n\r", ch);
          paf->type = skill_lookup ("plague");
          //paf->location = APPLY_STR;
          //paf->modifier = -5;
          break;
        case AFF_WEAKEN:
          send_to_char ("Adding weaken affect to object.\n\r", ch);
          paf->type = skill_lookup ("weaken");
          //paf->location = APPLY_STR;
          //paf->modifier = -1 * (level / 5);
          break;
        case AFF_SLOW:
          send_to_char ("Adding slow affect to object.\n\r", ch);
          paf->type = skill_lookup ("slow");
          //paf->location = APPLY_DEX;
          //paf->modifier = -1 - (level >= 18) - (level >= 25) - (level >= 32);
          break;
        case AFF_AQUA_BREATHE:
          send_to_char ("Adding aqua breathe affect to object.\n\r", ch);
          paf->type = skill_lookup ("aqua breathe");
          //paf->location = 0;
          //paf->modifier = 0;
          break;
        case AFF_SANCTUARY:
          send_to_char ("Adding sanctuary affect to object.\n\r", ch);
          paf->type = skill_lookup ("sanctuary");
          paf->location = APPLY_NONE;
          //paf->modifier = 0;
          //paf->bitvector = AFF_SANCTUARY;
          break;
        case AFF_HASTE:
          send_to_char ("Adding haste affect to object.\n\r", ch);
          paf->type = skill_lookup ("haste");
          //paf->location = APPLY_DEX;
          //paf->modifier = 1 + (level >= 18) + (level >= 25) + (level >= 32);
          break;
	case AFF_PROTECT_EVIL:
          send_to_char ("Adding protect evil affect to object.\n\r", ch);
          paf->type = skill_lookup ("protection evil");
          //paf->location = APPLY_SAVES;
          //paf->modifier = -1;
          break;
        case AFF_PROTECT_GOOD:
          send_to_char ("Adding protect good affect to object.\n\r", ch);
          paf->type = skill_lookup ("protection good");
          //paf->location = APPLY_SAVES;
          //paf->modifier = -1;
          break;
        }
      paf->location = APPLY_SPELL_AFFECT;
      paf->modifier = 0;
      paf->composition = FALSE;
      paf->comp_name = str_dup ("");
      paf->next = pObj->affected;
      pObj->affected = paf;
      send_to_char ("Affect added.\n\r", ch);
      return TRUE;
    }
  else
    {
      send_to_char ("Unknown affect name used.\n\r", ch);
      send_to_char ("Affect may be one of:\n\r", ch);
      send_to_char
        ("      haste          invisible          detect_evil          detect_invis\n\r",
         ch);
      send_to_char
        ("      detect_magic   detect_hidden      detect_good          faerie_fire\n\r",
         ch);
      send_to_char
        ("      sanctuary      infrared           curse                poison\n\r",
         ch);
      send_to_char
        ("      sneak          hide               aqua_breathe         flying\n\r",
         ch);
      send_to_char
        ("      pass_door      plague             weaken               slow\n\r",
         ch);
      return FALSE;
    }
}

/*
 * My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers.
 */
OEDIT (oedit_delaffect)
{
  OBJ_INDEX_DATA *pObj;
  AFFECT_DATA *pAf;
  AFFECT_DATA *pAf_next;
  char affect[MAX_STRING_LENGTH];
  int value;
  int cnt = 0;
  EDIT_OBJ (ch, pObj);
  one_argument (argument, affect);
  if (!is_number (affect) || affect[0] == '\0')
    {
      send_to_char ("Syntax:  delaffect [#affect]\n\r", ch);
      return FALSE;
    }
  value = atoi (affect);
  if (value < 0)
    {
      send_to_char ("Only non-negative affect-numbers allowed.\n\r", ch);
      return FALSE;
    }
  if (!(pAf = pObj->affected))
    {
      send_to_char ("OEdit:  Non-existant affect.\n\r", ch);
      return FALSE;
    }
  if (value == 0)
    {				/* First case: Remove first affect */
      pAf = pObj->affected;
      pObj->affected = pAf->next;
      free_affect (pAf);
    }

  else
    /* Affect to remove is not the first */
    {
      while ((pAf_next = pAf->next) && (++cnt < value))
	pAf = pAf_next;
      if (pAf_next)
	{			/* See if it's the next affect */
	  pAf->next = pAf_next->next;
	  free_affect (pAf_next);
	}

      else
	/* Doesn't exist */
	{
	  send_to_char ("No such affect.\n\r", ch);
	  return FALSE;
	}
    }
  send_to_char ("Affect removed.\n\r", ch);
  return TRUE;
}

OEDIT (oedit_name)
{
  OBJ_INDEX_DATA *pObj;
  EDIT_OBJ (ch, pObj);
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  name [string]\n\r", ch);
      return FALSE;
    }
  free_string (pObj->name);
  pObj->name = str_dup (argument);
  send_to_char ("Name set.\n\r", ch);
  return TRUE;
}

OEDIT (oedit_rarity)
{
  OBJ_INDEX_DATA *pObj;
  int value;
  EDIT_OBJ (ch, pObj);
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  rarity [flag]\n\r"
		    "Type '? rarity' for a list of flags.\n\r", ch);
      return FALSE;
    }
  if ((value = flag_value (rarity_flags, argument)) == NO_FLAG)
    {
      send_to_char
	("Invalid rarity.  Type '? rarity' for a list of flags.\n\r", ch);
      return FALSE;
    }
  pObj->rarity = value;
  send_to_char ("Rarity set.\n\r", ch);
  return TRUE;
}

OEDIT (oedit_short)
{
  OBJ_INDEX_DATA *pObj;
  EDIT_OBJ (ch, pObj);
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  short [string]\n\r", ch);
      return FALSE;
    }
  free_string (pObj->short_descr);
  pObj->short_descr = str_dup (argument);
  send_to_char ("Short description set.\n\r", ch);
  return TRUE;
}

OEDIT (oedit_arrival)
{
  OBJ_INDEX_DATA *pObj;
  char cmd[MAX_INPUT_LENGTH];
  EDIT_OBJ (ch, pObj);
  if (pObj->item_type != ITEM_EBUTTON)
    {
      send_to_char ("Huh?\n\r", ch);
      return FALSE;
    }
  argument = one_argument (argument, cmd);
  if (str_cmp (cmd, "inside") && str_cmp (cmd, "outside"))
    {
      send_to_char ("Syntax:  arrival [inside|outside] [string]\n\r", ch);
      return FALSE;
    }
  if (!str_cmp (cmd, "inside"))
    {
      free_string (pObj->string1);
      pObj->string1 = str_dup (argument);
      send_to_char ("Arrival message (inside) set.\n\r", ch);
    }

  else
    {
      free_string (pObj->string2);
      pObj->string2 = str_dup (argument);
      send_to_char ("Arrival message (outside) set.\n\r", ch);
    }
  return TRUE;
}

OEDIT (oedit_departure)
{
  OBJ_INDEX_DATA *pObj;
  char cmd[MAX_INPUT_LENGTH];
  EDIT_OBJ (ch, pObj);
  if (pObj->item_type != ITEM_EBUTTON)
    {
      send_to_char ("Huh?\n\r", ch);
      return FALSE;
    }
  argument = one_argument (argument, cmd);
  if (str_cmp (cmd, "inside") && str_cmp (cmd, "outside"))
    {
      send_to_char ("Syntax:  departure [inside|outside] [string]\n\r", ch);
      return FALSE;
    }
  if (!str_cmp (cmd, "inside"))
    {
      free_string (pObj->string3);
      pObj->string3 = str_dup (argument);
      send_to_char ("Departure message (inside) set.\n\r", ch);
    }

  else
    {
      free_string (pObj->string4);
      pObj->string4 = str_dup (argument);
      send_to_char ("Departure message (outside) set.\n\r", ch);
    }
  return TRUE;
}

OEDIT (oedit_initmsg)
{
  OBJ_INDEX_DATA *pObj;
  EXTRA_DESCR_DATA *ed;
  bool found = FALSE;
  EDIT_OBJ (ch, pObj);
  if (pObj->item_type != ITEM_WRITING_PAPER)
    {
      send_to_char ("Huh?\n\r", ch);
      return FALSE;
    }
  if (!str_cmp (argument, "delete"))
    {
      for (ed = pObj->extra_descr; ed != NULL; ed = ed->next)
	if (!str_cmp (ed->keyword, "papermsg"))
	  {
	    free_extra_descr (ed);
	    break;
	  }
      send_to_char ("Initial paper message deleted.\n\r", ch);
      return TRUE;
    }
  for (ed = pObj->extra_descr; ed != NULL; ed = ed->next)
    if (!str_cmp (ed->keyword, "papermsg"))
      {
	found = TRUE;
	send_to_char
	  ("Please use the Exodus Editor to enter initial paper message:\n\r\n\r",
	   ch);
	edit_string (ch, &ed->description);
	return (TRUE);
      }
  if (!found)
    {
      ed = new_extra_descr ();
      ed->keyword = str_dup ("papermsg");
      ed->description = str_dup ("");
      ed->next = pObj->extra_descr;
      pObj->extra_descr = ed;
      send_to_char
	("Please use the Exodus Editor to enter the initial paper message:\n\r\n\r",
	 ch);
      edit_string (ch, &ed->description);
    }
  return FALSE;
}

OEDIT (oedit_entermsg)
{
  OBJ_INDEX_DATA *pObj;
  char cmd[MAX_INPUT_LENGTH];
  EDIT_OBJ (ch, pObj);
  if (pObj->item_type != ITEM_PORTAL)
    {
      send_to_char ("Huh?\n\r", ch);
      return FALSE;
    }
  argument = one_argument (argument, cmd);
  if (str_cmp (cmd, "others") && str_cmp (cmd, "character"))
    {
      send_to_char ("Syntax:  entermsg [others|character] [string]\n\r", ch);
      return FALSE;
    }
  if (!str_cmp (cmd, "others"))
    {
      if (argument[0] != '\0')
	{
	  free_string (pObj->string1);
	  pObj->string1 = str_dup (argument);
	  send_to_char ("Enter message (others) set.\n\r", ch);
	  return TRUE;
	}

      else
	{
	  free_string (pObj->string1);
	  pObj->string1 = str_dup ("");
	  send_to_char ("Enter message (others) cleared.\n\r", ch);
	  return TRUE;
	}
    }

  else
    {
      if (argument[0] != '\0')
	{
	  free_string (pObj->string2);
	  pObj->string2 = str_dup (argument);
	  send_to_char ("Enter message (character) set.\n\r", ch);
	  return TRUE;
	}

      else
	{
	  free_string (pObj->string2);
	  pObj->string2 = str_dup ("");
	  send_to_char ("Enter message (character) cleared.\n\r", ch);
	  return TRUE;
	}
    }
  return FALSE;
}

OEDIT (oedit_exitmsg)
{
  OBJ_INDEX_DATA *pObj;
  char cmd[MAX_INPUT_LENGTH];
  EDIT_OBJ (ch, pObj);
  if (pObj->item_type != ITEM_PORTAL)
    {
      send_to_char ("Huh?\n\r", ch);
      return FALSE;
    }
  argument = one_argument (argument, cmd);
  if (str_cmp (cmd, "others") && str_cmp (cmd, "character"))
    {
      send_to_char ("Syntax:  exitmsg [others|character] [string]\n\r", ch);
      return FALSE;
    }
  if (!str_cmp (cmd, "others"))
    {
      if (argument[0] != '\0')
	{
	  free_string (pObj->string3);
	  pObj->string3 = str_dup (argument);
	  send_to_char ("Exit message (others) set.\n\r", ch);
	  return TRUE;
	}

      else
	{
	  free_string (pObj->string3);
	  pObj->string3 = str_dup ("");
	  send_to_char ("Exit message (others) cleared.\n\r", ch);
	  return TRUE;
	}
    }

  else
    {
      if (argument[0] != '\0')
	{
	  free_string (pObj->string4);
	  pObj->string4 = str_dup (argument);
	  send_to_char ("Exit message (character) set.\n\r", ch);
	  return TRUE;
	}

      else
	{
	  free_string (pObj->string4);
	  pObj->string4 = str_dup ("");
	  send_to_char ("Exit message (character) cleared.\n\r", ch);
	  return TRUE;
	}
    }
  return FALSE;
}

OEDIT (oedit_passenger)
{
  OBJ_INDEX_DATA *pObj;
  EDIT_OBJ (ch, pObj);
  if (pObj->item_type != ITEM_CTRANSPORT)
    {
      send_to_char ("Huh?\n\r", ch);
      return FALSE;
    }
  free_string (pObj->string1);
  pObj->string1 = str_dup (argument);
  send_to_char ("Passenger message set.\n\r", ch);
  return TRUE;
}

OEDIT (oedit_long)
{
  OBJ_INDEX_DATA *pObj;
  EDIT_OBJ (ch, pObj);
  free_string (pObj->description);
  pObj->description = str_dup (argument);
  pObj->description[0] = UPPER (pObj->description[0]);
  if (argument[0] == '\0')
    send_to_char ("Long description deleted.\n\r", ch);

  else
    send_to_char ("Long description set.\n\r", ch);
  return TRUE;
}

bool
set_value (CHAR_DATA * ch, OBJ_INDEX_DATA * pObj, char *argument, int value)
{
  if (argument[0] == '\0')
    {
      set_obj_values (ch, pObj, -1, "");	/* '\0' changed to "" -- Hugin */
      return FALSE;
    }
  if (set_obj_values (ch, pObj, value, argument))
    return TRUE;
  return FALSE;
}


/*****************************************************************************
 Name:		oedit_values
 Purpose:	Finds the object and sets its value.
 Called by:	The four valueX functions below. (now five -- Hugin )
 ****************************************************************************/
bool oedit_values (CHAR_DATA * ch, char *argument, int value)
{
  OBJ_INDEX_DATA *pObj;
  EDIT_OBJ (ch, pObj);
  if (set_value (ch, pObj, argument, value))
    return TRUE;
  return FALSE;
}

OEDIT (oedit_value0)
{
  if (oedit_values (ch, argument, 0))
    return TRUE;
  return FALSE;
}

OEDIT (oedit_value1)
{
  if (oedit_values (ch, argument, 1))
    return TRUE;
  return FALSE;
}

OEDIT (oedit_value2)
{
  if (oedit_values (ch, argument, 2))
    return TRUE;
  return FALSE;
}

OEDIT (oedit_value3)
{
  if (oedit_values (ch, argument, 3))
    return TRUE;
  return FALSE;
}

OEDIT (oedit_value4)
{
  if (oedit_values (ch, argument, 4))
    return TRUE;
  return FALSE;
}

OEDIT (oedit_value5)
{
  if (oedit_values (ch, argument, 5))
    return TRUE;
  return FALSE;
}

OEDIT (oedit_value6)
{
  if (oedit_values (ch, argument, 6))
    return TRUE;
  return FALSE;
}

OEDIT (oedit_value7)
{
  if (oedit_values (ch, argument, 7))
    return TRUE;
  return FALSE;
}

OEDIT (oedit_value8)
{
  if (oedit_values (ch, argument, 8))
    return TRUE;
  return FALSE;
}

OEDIT (oedit_value9)
{
  if (oedit_values (ch, argument, 9))
    return TRUE;
  return FALSE;
}

OEDIT (oedit_value10)
{
  if (oedit_values (ch, argument, 10))
    return TRUE;
  return FALSE;
}

OEDIT (oedit_value11)
{
  if (oedit_values (ch, argument, 11))
    return TRUE;
  return FALSE;
}

OEDIT (oedit_value12)
{
  if (oedit_values (ch, argument, 12))
    return TRUE;
  return FALSE;
}



OEDIT (oedit_weight)
{
  OBJ_INDEX_DATA *pObj;
  EDIT_OBJ (ch, pObj);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  weight [number]\n\r", ch);
      return FALSE;
    }
  pObj->weight = atoi (argument);
  send_to_char ("Weight set.\n\r", ch);
  return TRUE;
}

OEDIT (oedit_cost)
{
  OBJ_INDEX_DATA *pObj;
  EDIT_OBJ (ch, pObj);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  cost [number]\n\r", ch);
      return FALSE;
    }
  pObj->cost = atoi (argument);
  send_to_char ("Cost set.\n\r", ch);
  return TRUE;
}

OEDIT (oedit_create)
{
  OBJ_INDEX_DATA *pObj;
  AREA_DATA *pArea;
  int value;
  int iHash;
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
  if (get_obj_index (value))
    {
      send_to_char ("Object vnum already exists.\n\r", ch);
      return FALSE;
    }
  pObj = new_obj_index ();
  pObj->vnum = value;
  pObj->area = pArea;
  pObj->rarity = RARITY_ALWAYS;
  pObj->string1 = str_dup ("");
  pObj->string2 = str_dup ("");
  pObj->string3 = str_dup ("");
  pObj->string4 = str_dup ("");
  SET_BIT (pObj->extra_flags[0], ITEM_NONMETAL);
  if (value > top_vnum_obj)
    top_vnum_obj = value;
  iHash = value % MAX_KEY_HASH;
  pObj->next = obj_index_hash[iHash];
  obj_index_hash[iHash] = pObj;
  ch->desc->pEdit = (void *) pObj;
  send_to_char ("Object Created.\n\r", ch);
  return TRUE;
}

OEDIT (oedit_purge)
{
  OBJ_INDEX_DATA *pObj, *prev;
  OBJ_DATA *obj;
  AREA_DATA *pArea;
  int value;
  int iHash;
  if (argument[0] == '\0' || (value = atoi (argument)) <= 0)
    {
      send_to_char ("Syntax: purge <vnum>\n\r", ch);
      return FALSE;
    }
  if ((pArea = get_vnum_area (value)) == NULL)
    {
      send_to_char ("That vnum is not assigned an area.\n\r", ch);
      return FALSE;
    }
  if (!is_builder (ch, pArea))
    {
      send_to_char ("Vnum in an area you cannot build in.\n\r", ch);
      return FALSE;
    }
  if ((pObj = get_obj_index (value)) == NULL)
    {
      send_to_char ("Object doesn't exist.\n\r", ch);
      return FALSE;
    }
  if (ch->desc->pEdit == (void *) pObj)
    edit_done (ch);

  /* Change top_vnum_obj ? */

  /* Remove objects in the game */
  for (obj = object_list; obj != NULL; obj = obj->next)
    {
      if (obj->pIndexData == pObj)
	extract_obj (obj);
    }

  /* remove from linked list */
  if (obj_index_hash[iHash = value % MAX_KEY_HASH] == pObj)
    {
      obj_index_hash[iHash] = pObj->next;
    }

  else
    {
      for (prev = obj_index_hash[iHash]; prev != NULL; prev = prev->next)
	{
	  if (prev->next == pObj)
	    break;
	}
      prev->next = pObj->next;
    }
  free_obj_index (pObj);
  send_to_char ("Object Purged.\n\r", ch);
  top_obj_index--;
  return TRUE;
}

OEDIT (oedit_ed)
{
  OBJ_INDEX_DATA *pObj;
  EXTRA_DESCR_DATA *ed;
  char command[MAX_INPUT_LENGTH];
  char keyword[MAX_INPUT_LENGTH];
  EDIT_OBJ (ch, pObj);
  argument = one_argument (argument, command);
  one_argument (argument, keyword);
  if (command[0] == '\0')
    {
      send_to_char ("Syntax:  ed add [keyword]\n\r", ch);
      send_to_char ("         ed delete [keyword]\n\r", ch);
      send_to_char ("         ed edit [keyword]\n\r", ch);
      send_to_char ("         ed format [keyword]\n\r", ch);
      return FALSE;
    }
  if (!str_cmp (command, "add"))
    {
      if (keyword[0] == '\0')
	{
	  send_to_char ("Syntax:  ed add [keyword]\n\r", ch);
	  return FALSE;
	}
      ed = new_extra_descr ();
      ed->keyword = str_dup (keyword);
      ed->next = pObj->extra_descr;
      pObj->extra_descr = ed;
      send_to_char
	("Please use the Exodus Editor to enter the keyword description:\n\r\n\r",
	 ch);
      edit_string (ch, &ed->description);
      return TRUE;
    }
  if (!str_cmp (command, "edit"))
    {
      if (keyword[0] == '\0')
	{
	  send_to_char ("Syntax:  ed edit [keyword]\n\r", ch);
	  return FALSE;
	}
      for (ed = pObj->extra_descr; ed; ed = ed->next)
	{
	  if (is_name (keyword, ed->keyword))
	    break;
	}
      if (!ed)
	{
	  send_to_char
	    ("OEdit:  Extra description keyword not found.\n\r", ch);
	  return FALSE;
	}
      send_to_char
	("Please use the Exodus Editor to enter the extra description:\n\r\n\r",
	 ch);
      edit_string (ch, &ed->description);
      return TRUE;
    }
  if (!str_cmp (command, "delete"))
    {
      EXTRA_DESCR_DATA *ped = NULL;
      if (keyword[0] == '\0')
	{
	  send_to_char ("Syntax:  ed delete [keyword]\n\r", ch);
	  return FALSE;
	}
      for (ed = pObj->extra_descr; ed; ed = ed->next)
	{
	  if (is_name (keyword, ed->keyword))
	    break;
	  ped = ed;
	}
      if (!ed)
	{
	  send_to_char
	    ("OEdit:  Extra description keyword not found.\n\r", ch);
	  return FALSE;
	}
      if (!ped)
	pObj->extra_descr = ed->next;

      else
	ped->next = ed->next;
      free_extra_descr (ed);
      send_to_char ("Extra description deleted.\n\r", ch);
      return TRUE;
    }
  if (!str_cmp (command, "format"))
    {
      EXTRA_DESCR_DATA *ped = NULL;
      if (keyword[0] == '\0')
	{
	  send_to_char ("Syntax:  ed format [keyword]\n\r", ch);
	  return FALSE;
	}
      for (ed = pObj->extra_descr; ed; ed = ed->next)
	{
	  if (is_name (keyword, ed->keyword))
	    break;
	  ped = ed;
	}
      if (!ed)
	{
	  send_to_char
	    ("OEdit:  Extra description keyword not found.\n\r", ch);
	  return FALSE;
	}
      ed->description = format_string (ed->description);
      send_to_char ("Extra description formatted.\n\r", ch);
      return TRUE;
    }
  oedit_ed (ch, "");
  return FALSE;
}


// ROM object functions :
//
// Moved out of oedit() due to naming conflicts -- Hugin
/*OEDIT (oedit_extra)
{
  OBJ_INDEX_DATA *pObj;
  int value,i;
  bool found=FALSE;
  if (argument[0] != '\0')
    {
      EDIT_OBJ (ch, pObj);
      for (i = 0; i < MAX_EXTRA_FLAGS;i++)
      {
        if ((value = flag_value (extra_flags[i], argument)) != NO_FLAG)
  	  {
	    TOGGLE_BIT (pObj->extra_flags[i], value);
//	    send_to_char ("Extra flag toggled.\n\r", ch);
	    found = TRUE;
//	    return TRUE;
	  }
      }
      if (found)
      {
        send_to_char ("Extra flag toggled.\n\r", ch);
	return TRUE;
      }
    }
  send_to_char ("Syntax:  extra [flag]\n\r"
		"Type '? extra' for a list of flags.\n\r", ch);
  return FALSE;
}*/


// Moved out of oedit() due to naming conflicts -- Hugin
OEDIT (oedit_wear)
{
  OBJ_INDEX_DATA *pObj;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_OBJ (ch, pObj);
      if ((value = flag_value (wear_flags, argument)) != NO_FLAG)
	{
	  TOGGLE_BIT (pObj->wear_flags, value);
	  send_to_char ("Wear flag toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax:  wear [flag]\n\r"
		"Type '? wear' for a list of flags.\n\r", ch);
  return FALSE;
}


// Akamai 4/30/99 - support Class/race specific objects
OEDIT (oedit_Class)
{
  OBJ_INDEX_DATA *pObj;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_OBJ (ch, pObj);
      if ((value = flag_value (Class_flags, argument)) != NO_FLAG)
	{
	  TOGGLE_BIT (pObj->Class_flags, value);
	  send_to_char ("Class flag toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax:  Class [flag]\n\r"
		"Type '? Class' for a list of flags.\n\r", ch);
  return FALSE;
}


// Akamai 4/30/99 - support Class/race specific objects
OEDIT (oedit_race)
{
  OBJ_INDEX_DATA *pObj;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_OBJ (ch, pObj);
      if ((value = flag_value (pcrace_flags, argument)) != NO_FLAG)
	{
	  TOGGLE_BIT (pObj->race_flags, value);
	  send_to_char ("Race flag toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax:  race [flag]\n\r"
		"Type '? race' for a list of flags.\n\r", ch);
  return FALSE;
}


// Akamai 4/30/99 - support clan type specific objects
OEDIT (oedit_clan)
{
  OBJ_INDEX_DATA *pObj;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_OBJ (ch, pObj);
      if ((value = flag_value (clan_flags, capitalize(argument))) != NO_FLAG)
	{
	  TOGGLE_BIT (pObj->clan_flags, value);
	  send_to_char ("Clan flag toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax:  clan [flag]\n\r"
		"Type '? clan' for a list of flags.\n\r", ch);
  return FALSE;
}


// Adeon 6/29/03 -- player owned eq
OEDIT (oedit_owner)
{
  OBJ_INDEX_DATA *pObj;
  if (argument[0] != '\0')
    {
      EDIT_OBJ (ch, pObj);
      if (!strcmp (argument, "none"))
	{
          if (pObj->plr_owner)
	    free_string (pObj->plr_owner);
	  pObj->plr_owner = NULL;
	  return TRUE;
	}
      if (pObj->plr_owner)
        free_string (pObj->plr_owner);
      pObj->plr_owner = str_dup (argument);
      send_to_char ("Player Owner Set.", ch);
      return TRUE;
    }
  send_to_char ("Syntax:  owner [player name]\n\r", ch);
  return FALSE;
}

OEDIT (oedit_type)
{
  OBJ_INDEX_DATA *pObj;
  EXTRA_DESCR_DATA *ed/* ,*ped=NULL*/;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_OBJ (ch, pObj);
      if ((value = flag_value (type_flags, argument)) != NO_FLAG)
	{
	  pObj->item_type = value;
	  send_to_char ("Type set.\n\r", ch);

	  /*
	   * Clear the values.
	   */
	  pObj->value[0] = 0;
	  pObj->value[1] = 0;
	  pObj->value[2] = 0;
	  pObj->value[3] = 0;
	  pObj->value[4] = 0;
	  if (value == ITEM_PORTAL)
	    SET_BIT (pObj->value[2], GATE_SHOWEXIT);
	  if (value == ITEM_DRINK_CON)
	    pObj->value[2] = 0;
	  if (value == ITEM_OBJ_TRAP || value == ITEM_ROOM_TRAP || value == ITEM_PORTAL_TRAP )
	  {
	      SET_BIT(pObj->extra_flags[1],ITEM_WIZI);
	  	for (ed = pObj->extra_descr; ed; ed = ed->next)
		{
		    if (is_name ("specmessage", ed->keyword) )
			break;
		}			
				  
		if (ed == NULL)
		{
		      ed = new_extra_descr ();
		      ed->keyword = str_dup ("specmessage");
		      ed->next = pObj->extra_descr;
	      	      ed->description = str_dup ("");
		      pObj->extra_descr = ed;
		}

	  	for (ed = pObj->extra_descr; ed; ed = ed->next)
		{
		    if ( is_name ("victmessage", ed->keyword))
			break;
		}		
		if (ed == NULL)
		{
		      ed = new_extra_descr ();
		     ed->keyword = str_dup ("victmessage");
		      ed->next = pObj->extra_descr;
      		      ed->description = str_dup ("");
		      pObj->extra_descr = ed;
		}
		if (value == ITEM_ROOM_TRAP)
			pObj->value[0] = TRAP_R_PLAIN;
		if (value == ITEM_PORTAL_TRAP)
			pObj->value[0] = TRAP_P_PLAIN;
		if (value == ITEM_OBJ_TRAP)
			pObj->value[0] = TRAP_O_PLAIN;
		
                {
                  EXTRA_DESCR_DATA *ed;
		  int type;
                        type = trap_info_lookup(pObj->value[0]);
                        if (type != NO_FLAG)
                        {
                           pObj->value[1] = trapinfo[type].num_dice;
                           pObj->value[2] = trapinfo[type].size_dice;
                           pObj->value[3] = trapinfo[type].mod_dice;
                           pObj->value[8] = flag_value (weapon_flags, trapinfo[type].damtype);
                           pObj->value[12] = trapinfo[type].flags;
                           for (ed = pObj->extra_descr; ed; ed = ed->next)
                           {
                               if (is_name ("specmessage", ed->keyword) )
                                  break;
                           }

                           if (ed != NULL)
                              ed->description = str_dup (trapinfo[type].smess);

                           for (ed = pObj->extra_descr; ed; ed = ed->next)
                           {
                              if ( is_name ("victmessage", ed->keyword))
                                  break;
                           }
                           if (ed != NULL)
                              ed->description = str_dup (trapinfo[type].vmess);

                        }
                        else
                        {
                        pObj->value[1] = 0; //number of dice
                        pObj->value[2] = 0; // size of dice
                        pObj->value[3] = 0; // dam modifier
                        }
                }

	  }
	  return TRUE;
	}
    }
  send_to_char ("Syntax:  type [flag]\n\r"
		"Type '? type' for a list of flags.\n\r", ch);
  return FALSE;
}

OEDIT (oedit_material)
{
  OBJ_INDEX_DATA *pObj;
  if (argument[0] != '\0')
    {
      EDIT_OBJ (ch, pObj);
      if (valid_material_name (argument))
	{
	  free_string (pObj->material);
	  pObj->material = str_dup (argument);
	  send_to_char ("Material type set.\n\r", ch);
	  return TRUE;
	}

      else
	{
	  send_to_char
	    ("That is not a valid material name.  Type '? material'.\n\r",
	     ch);
	  return FALSE;
	}
    }
  send_to_char ("Syntax:  material [material-name]\n\r"
		"Type '? material' for a list of suggested materials.\n\r",
		ch);
  return FALSE;
}

OEDIT (oedit_level)
{
  OBJ_INDEX_DATA *pObj;
  int value;
  EDIT_OBJ (ch, pObj);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  level [number]\n\r", ch);
      return FALSE;
    }
  value = atoi (argument);
  if (value < 0 || value > MAX_LEVEL)
    {
      send_to_char ("Level range is 0 to 100.\n\r", ch);
      return FALSE;
    }
  pObj->level = value;
  send_to_char ("Level set.\n\r", ch);
  return TRUE;
}

OEDIT (oedit_timer)
{
  OBJ_INDEX_DATA *pObj;
  int value;
  EDIT_OBJ (ch, pObj);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  timer [number]\n\r", ch);
      return FALSE;
    }
  value = atoi (argument);
  if (value < 0)
    {
      send_to_char ("Timer value must be greater than 0.\n\r", ch);
      return FALSE;
    }
  pObj->timer = value;
  send_to_char ("Timer set.\n\r", ch);
  return TRUE;
}

OEDIT (oedit_condition)
{
  OBJ_INDEX_DATA *pObj;
  int value;
  if (argument[0] != '\0' && (value = atoi (argument)) >= 0 && (value <= 100))
    {
      EDIT_OBJ (ch, pObj);
      pObj->condition = value;
      send_to_char ("Condition set.\n\r", ch);
      return TRUE;
    }
  send_to_char ("Syntax:  condition [number]\n\r"
		"Where number can range from 0 (ruined) to 100 (perfect).\n\r",
		ch);
  return FALSE;
}

OEDIT (oedit_objtrigger)
{
  OBJ_INDEX_DATA *pObj;
  int value,i,k;
  EDIT_OBJ (ch, pObj);
  if (argument[0] == '\0' || !is_number (argument))
  {
    send_to_char ("Syntax:  objtrigger [vnum]\n\r", ch);
    return FALSE;
  }
  value = atoi (argument);
  for (i=0;i<MAX_OBJ_TRIGS;i++)
  {   
    if (pObj->obj_trig_vnum[i] == 0)
      break;
    if (pObj->obj_trig_vnum[i] == value)
    {
	    //ERASE
      for (k=i;k<MAX_OBJ_TRIGS-1;k++)
        pObj->obj_trig_vnum[k] = pObj->obj_trig_vnum[k+1];
      pObj->obj_trig_vnum[MAX_OBJ_TRIGS-1] = 0;
      send_to_char("That object trigger vnum removed.\n\r",ch);
      return TRUE;
    }
  }
  if (value != 0 && !get_obj_trig(value))
  {
    send_to_char ("An object trigger with that vnum does not exist.\n\r", ch);
    return FALSE;
  }
  pObj->obj_trig_vnum[i] = value;
  send_to_char ("Vnum set.\n\r", ch);
  return TRUE;
}




OEDIT (oedit_trapflag)
{
  OBJ_INDEX_DATA *pObj;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_OBJ (ch, pObj);
      if ((value = flag_value (trap_flag, argument)) != NO_FLAG)
	{
	  pObj->value[12] ^= value;
	  send_to_char ("Trap flag toggled.\n\r", ch);
	}
      return TRUE;
    }
  send_to_char ("Syntax: trapflag [flag]\n\r"
		"Type '? trapflag' for a list of flags.\n\r", ch);
  return FALSE;
}


/*
 * Mobile Editor Functions.
 */
MEDIT (medit_show)
{
  MOB_INDEX_DATA *pMob;
  char buf[MAX_STRING_LENGTH];
  char bigbuf[MAX_STRING_LENGTH * 4];
  char *muff;
  const char *dir;			// for blocking
  EDIT_MOB (ch, pMob);
  bigbuf[0] = '\0';
  sprintf (buf, "Name:  [%s]", pMob->player_name);
  sprintf (bigbuf + strlen (bigbuf), "%-35s| Area:  [%5d] %s\n\r", buf,
	   !pMob->area ? -1 : pMob->area->vnum,
	   !pMob->area ? "No Area" : pMob->area->name);
  sprintf (buf, "Vnum:  [%5d]", pMob->vnum);
  sprintf (bigbuf + strlen (bigbuf), "%-35s| Sex:  [%s]\n\r", buf,
	   pMob->sex == SEX_MALE ? "male" : pMob->sex ==
	   SEX_FEMALE ? "female" : pMob->sex == 3 ? "either" : "neutral");
  sprintf (buf, "Level:  [%2d]", pMob->level);
  sprintf (bigbuf + strlen (bigbuf), "%-35s| Align:  [%4d]\n\r", buf,
	   pMob->alignment);
  sprintf (buf, "Race:  [%s]", race_table[pMob->race].name);
  sprintf (bigbuf + strlen (bigbuf), "%-35s| Hitroll:  [%d]\n\r", buf,
	   pMob->hitroll);
  sprintf (buf, "HitDice:  [%2dd%-3d+%4d]", pMob->hit[DICE_NUMBER],
	   pMob->hit[DICE_TYPE], pMob->hit[DICE_BONUS]);
  sprintf (bigbuf + strlen (bigbuf), "%-35s| DamDice: [%2dd%-3d+%4d]\n\r",
	   buf, pMob->damage[DICE_NUMBER], pMob->damage[DICE_TYPE],
	   pMob->damage[DICE_BONUS]);
  sprintf (buf, "ManaDice:  [%2dd%-3d+%4d]", pMob->mana[DICE_NUMBER],
	   pMob->mana[DICE_TYPE], pMob->mana[DICE_BONUS]);
  sprintf (bigbuf + strlen (bigbuf), "%-35s| Wealth:  [%5ld]\n\r", buf,
	   pMob->wealth);
  sprintf (buf, "StartPos:  [%s]",
	   flag_string (position_flags, pMob->start_pos));
  sprintf (bigbuf + strlen (bigbuf), "%-35s| DefaultPos:  [%s]\n\r", buf,
	   flag_string (position_flags, pMob->default_pos));
  sprintf (buf, "Size:  [%s]", flag_string (size_flags, pMob->size));
  sprintf (bigbuf + strlen (bigbuf),
	   "%-35s| AC:  [P:%d  B:%d  S:%d  M:%d]\n\r", buf,
	   pMob->ac[AC_PIERCE], pMob->ac[AC_BASH], pMob->ac[AC_SLASH],
	   pMob->ac[AC_EXOTIC]);
  sprintf (buf, "MobGroup:  [%d]", pMob->group);
  sprintf (bigbuf + strlen (bigbuf), "%-35s| DamType:  [%s]\n\r", buf,
	   attack_table[pMob->dam_type].name);
  sprintf (buf, "Material:    [%s]", pMob->material);
  sprintf (bigbuf + strlen (bigbuf), "%-35s| MaxWeight(Mount): [%d]\n\r",
	   buf, pMob->max_weight);
  sprintf (buf, "DefenseBonus(Mount): [%d]", pMob->defbonus);
  sprintf (bigbuf + strlen (bigbuf),
	   "%-35s| AttackBonus(Mount): [%d]\n\r", buf, pMob->attackbonus);
  sprintf (buf, "Imm: [%s]", flag_string (imm_flags, pMob->imm_flags));
  sprintf (bigbuf + strlen (bigbuf), "%-35s| Res: [%s]\n\r", buf,
	   flag_string (res_flags, pMob->res_flags));
  sprintf (buf, "Vuln: [%s]", flag_string (vuln_flags, pMob->vuln_flags));
  sprintf (bigbuf + strlen (bigbuf), "%-35s| Off: [%s]\n\r", buf,
	   flag_string (off_flags, pMob->off_flags));
  muff = str_dup (flag_string (act_flags2, pMob->act2));
  sprintf (buf, "Movement: [%d]", pMob->move);
  sprintf (bigbuf + strlen (bigbuf), "%-35s| Act: [%s %s]\n\r", buf, flag_string (act_flags, pMob->act), muff);
  if (pMob->vocfile[0] != '\0')
    sprintf (buf, "VocabFile:   [%s]", pMob->vocfile);

  else
    strcpy (buf, "VocabFile:    [none]");
  sprintf (bigbuf + strlen (bigbuf), "%-35s| Mood(default): [%d]\n\r",
	   buf, pMob->default_mood);
  if (pMob->number_of_attacks != -1)
    sprintf (buf,"Number of Attacks: [%d]", pMob->number_of_attacks);
  else sprintf (buf,"Number of Attacks: [Not Relevant]");
  sprintf (bigbuf + strlen (bigbuf), "%-35s| Card Vnum: [%d]\n\r", buf, pMob->card_vnum);
  sprintf (bigbuf + strlen (bigbuf), "Str [%2d] Int [%2d] Wis [%2d] Dex [%2d] Con [%2d] Cha [%2d]\n\r",pMob->perm_stat[STAT_STR],
		  pMob->perm_stat[STAT_INT],pMob->perm_stat[STAT_WIS],pMob->perm_stat[STAT_DEX],pMob->perm_stat[STAT_CON],pMob->perm_stat[STAT_CHA]);
  sprintf (bigbuf + strlen (bigbuf), "Recruit: [%s]\n\r",
	   flag_string (recruit_flags, pMob->recruit_flags));
  if (IS_SET (pMob->recruit_flags, RECRUIT_KEEPER))
    sprintf (bigbuf + strlen (bigbuf), "PSVnum: [%d]\n\r",
	     pMob->recruit_value[0]);
  if (IS_SET (pMob->recruit_flags, RECRUIT_KEEPER))
    sprintf (bigbuf + strlen (bigbuf), "PSCost: [%d]\n\r",
	     pMob->recruit_value[1]);
  if (IS_SET (pMob->act2, ACT_BLOCK_EXIT))
    {
      if (pMob->blocks_exit == -1)
	dir = "none";

      else
	dir = dir_name[pMob->blocks_exit];
      sprintf (bigbuf + strlen (bigbuf), "Blocks Exit: [%s]\n\r", dir);
    }
  if (pMob->script_fn[0] != '\0')
    sprintf (bigbuf + strlen (bigbuf), "TriggerFile:   [%s]\n\r",
	     pMob->script_fn);
  sprintf (bigbuf + strlen (bigbuf), "Affected by: [%s]\n\r",
	   flag_string (affect_flags, pMob->affected_by));
  sprintf (bigbuf + strlen (bigbuf), "Form:        [%s]\n\r",
	   flag_string (form_flags, pMob->form));
  sprintf (bigbuf + strlen (bigbuf), "Parts:       [%s]\n\r",
	   flag_string (part_flags, pMob->parts));
  if (pMob->spec_fun)
    sprintf (bigbuf + strlen (bigbuf), "Spec fun:    [%s]\n\r",
	     spec_string (pMob->spec_fun));
  sprintf (bigbuf + strlen (bigbuf),
	   "Short descr: %s\n\rLong descr:\n\r%s", pMob->short_descr,
	   pMob->long_descr);
  sprintf (bigbuf + strlen (bigbuf), "Description:\n\r%s", pMob->description);
  if (pMob->pShop)
    {
      SHOP_DATA *pShop;
      int iTrade;
      pShop = pMob->pShop;
      sprintf (bigbuf + strlen (bigbuf), "Shop data for [%5d]:\n\r"
	       "  Markup for purchaser: %d%%\n\r"
	       "  Markdown for seller:  %d%%\n\r", pShop->keeper,
	       pShop->profit_buy, pShop->profit_sell);
      sprintf (bigbuf + strlen (bigbuf), "  Hours: %d to %d.\n\r",
	       pShop->open_hour, pShop->close_hour);
      for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
	{
	  if (pShop->buy_type[iTrade] != 0)
	    {
	      if (iTrade == 0)
		{
		  strcat (bigbuf, "  Number Trades Type\n\r");
		  strcat (bigbuf, "  ------ -----------\n\r");
		}
	      sprintf (bigbuf + strlen (bigbuf), "  [%4d] %s\n\r", iTrade,
		       flag_string (type_flags, pShop->buy_type[iTrade]));
	    }
	}
    }
  page_to_char (bigbuf, ch);
  return FALSE;
}

MEDIT (medit_create)
{
  MOB_INDEX_DATA *pMob;
  AREA_DATA *pArea;
  int value;
  int iHash;
  int i=0;
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
  if (get_mob_index (value))
    {
      send_to_char ("Mobile vnum already exists.\n\r", ch);
      return FALSE;
    }
  pMob = new_mob_index ();
  pMob->vnum = value;
  pMob->area = pArea;
  if (value > top_vnum_mob)
    top_vnum_mob = value;
  pMob->act = ACT_IS_NPC;
  pMob->move = 100;
  for (i = 0;i<MAX_STATS;++i)
    pMob->perm_stat[i] = 10;
  pMob->number_of_attacks = -1;
  iHash = value % MAX_KEY_HASH;
  pMob->next = mob_index_hash[iHash];
  mob_index_hash[iHash] = pMob;
  ch->desc->pEdit = (void *) pMob;
  send_to_char ("Mobile Created.\n\r", ch);
  return TRUE;
}

MEDIT (medit_purge)
{
  MOB_INDEX_DATA *pMob, *prev;
  CHAR_DATA *mob;
  AREA_DATA *pArea;
  int value;
  int iHash;
  if (argument[0] == '\0' || (value = atoi (argument)) <= 0)
    {
      send_to_char ("Syntax: purge <vnum>\n\r", ch);
      return FALSE;
    }
  if ((pArea = get_vnum_area (value)) == NULL)
    {
      send_to_char ("That vnum is not assigned an area.\n\r", ch);
      return FALSE;
    }
  if (!is_builder (ch, pArea))
    {
      send_to_char ("Vnum in an area you cannot build in.\n\r", ch);
      return FALSE;
    }
  if ((pMob = get_mob_index (value)) == NULL)
    {
      send_to_char ("MEdit:  Mobile doesn't exist.\n\r", ch);
      return FALSE;
    }
  if (ch->desc->pEdit == (void *) pMob)
    edit_done (ch);

  /* Change top_vnum_mob ? */

  /* Remove mobiles in the game */
  for (mob = char_list; mob != NULL; mob = mob->next)
    {
      if (IS_NPC (mob) && mob->pIndexData == pMob)
	extract_char (mob, TRUE);
    }

  /* remove from linked list */
  if (mob_index_hash[iHash = value % MAX_KEY_HASH] == pMob)
    {
      mob_index_hash[iHash] = pMob->next;
    }

  else
    {
      for (prev = mob_index_hash[iHash]; prev != NULL; prev = prev->next)
	{
	  if (prev->next == pMob)
	    break;
	}
      prev->next = pMob->next;
    }
  free_mob_index (pMob);
  send_to_char ("Mobile Purged.\n\r", ch);
  top_mob_index--;
  return TRUE;
}

MEDIT (medit_spec)
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  spec [special function]\n\r", ch);
      return FALSE;
    }
  if (!str_cmp (argument, "none"))
    {
      pMob->spec_fun = NULL;
      send_to_char ("Spec removed.\n\r", ch);
      return TRUE;
    }
  if (spec_lookup (argument))
    {
      pMob->spec_fun = spec_lookup (argument);
      send_to_char ("Spec set.\n\r", ch);
      return TRUE;
    }
  send_to_char ("MEdit: No such special function.\n\r", ch);
  return FALSE;
}

MEDIT (medit_align)
{
  MOB_INDEX_DATA *pMob;
  int value;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  alignment [number]\n\r", ch);
      return FALSE;
    }
  value = atoi (argument);
  if (value < -1000 || value > 1000)
    {
      send_to_char ("Alignment range is -1000 to 1000.\n\r", ch);
      return FALSE;
    }
  pMob->alignment = value;
  send_to_char ("Alignment set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_pscost)
{
  MOB_INDEX_DATA *pMob;
  int value;
  EDIT_MOB (ch, pMob);
  if (!IS_SET (pMob->recruit_flags, RECRUIT_KEEPER))
    {
      send_to_char
	("This mob must first be flagged as a player-store keeper.\n\r", ch);
      return FALSE;
    }
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  pscost [number]\n\r", ch);
      return FALSE;
    }
  value = atoi (argument);
  if (value < 0)
    {
      send_to_char ("Cost value must be positive or zero.\n\r", ch);
      return FALSE;
    }
  pMob->recruit_value[1] = value;
  send_to_char ("Player store hire cost set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_psvnum)
{
  MOB_INDEX_DATA *pMob;
  int value;
  EDIT_MOB (ch, pMob);
  if (!IS_SET (pMob->recruit_flags, RECRUIT_KEEPER))
    {
      send_to_char
	("This mob must first be flagged as a player-store keeper.\n\r", ch);
      return FALSE;
    }
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  psvnum [number]\n\r", ch);
      return FALSE;
    }
  value = atoi (argument);
  if (value < 0)
    {
      send_to_char ("Vnum value must be positive or zero.\n\r", ch);
      return FALSE;
    }
  pMob->recruit_value[0] = value;
  send_to_char ("Player store vnum set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_level)
{
  MOB_INDEX_DATA *pMob;
  int value;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  level [number]\n\r", ch);
      return FALSE;
    }
  value = atoi (argument);
  if (value < 0 || value > MAX_LEVEL)
    {
      send_to_char ("Level range is 0 to 100.\n\r", ch);
      return FALSE;
    }
  pMob->level = value;
  send_to_char ("Level set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_mood)
{
  MOB_INDEX_DATA *pMob;
  int value;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  mood [number]\n\r", ch);
      return FALSE;
    }
  value = atoi (argument);
  if (value < 0 || value > 7)
    {
      send_to_char ("Mood range is 0 to 7.\n\r", ch);
      return FALSE;
    }
  pMob->default_mood = value;
  send_to_char ("Default mood set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_movement)
{
  MOB_INDEX_DATA *pMob;
  int value;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  movement [number]\n\r", ch);
      return FALSE;
    }
  value = atoi (argument);
  if (value < 0 || value > 30000)
    {
      send_to_char ("Mobile's movement must be between 0 and 30000.\n\r", ch);
      return FALSE;
    }
  pMob->move = value;
  send_to_char ("Movement set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_defbonus)
{
  MOB_INDEX_DATA *pMob;
  int value;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  defensebonus [number]\n\r", ch);
      return FALSE;
    }
  value = atoi (argument);
  pMob->defbonus = value;
  send_to_char ("Mount's defense bonus set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_attackbonus)
{
  MOB_INDEX_DATA *pMob;
  int value;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  attackbonus [number]\n\r", ch);
      return FALSE;
    }
  value = atoi (argument);
  pMob->attackbonus = value;
  send_to_char ("Mount's attack bonus set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_maxweight)
{
  MOB_INDEX_DATA *pMob;
  int value;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  maxweight [number]\n\r", ch);
      return FALSE;
    }
  value = atoi (argument);
  if (value < 0 || value > 30000)
    {
      send_to_char
	("Mobile's maximum weight must be between 0 and 30000.\n\r", ch);
      return FALSE;
    }
  pMob->max_weight = value;
  send_to_char ("Mount's maximum carry weight set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_mobgroup)
{
  MOB_INDEX_DATA *pMob;
  int value;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  mobgroup [number]\n\r", ch);
      return FALSE;
    }
  value = atoi (argument);
  if (value < 0)
    {
      send_to_char ("Negative mob groups are not allowed.\n\r", ch);
      return FALSE;
    }
  pMob->group = value;
  send_to_char ("MobGroup value set.\n\r", ch);
  return TRUE;
}


// Adeon 7/17/03 -- set the direction of the exit a mob will block
MEDIT (medit_blockexit)
{
  MOB_INDEX_DATA *pMob;
  int dir=0;
  bool valid = FALSE;
  int i;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || is_number (argument))
    {
      send_to_char ("Syntax: blockexit <direction/none>\n\r", ch);
      return FALSE;
    }
  if (!strcmp (argument, "none"))
    {
      send_to_char ("Blocked exit removed.\n\r", ch);
      pMob->blocks_exit = -1;
      return FALSE;
    }
  for (i = 0; i < 6; i++)
    {
      if (!str_cmp (argument, dir_name[i]))
	{
	  valid = TRUE;
	  dir = i;
	}
    }
  if (!valid)
    {
      send_to_char ("That is not a valid direction.\n\r", ch);
      return FALSE;
    }
  pMob->blocks_exit = dir;
  send_to_char ("Successfully set.\n\r",ch);
  return TRUE;
}

MEDIT (medit_numattacks)
{
  MOB_INDEX_DATA *pMob;
  int number;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument))
  {
    send_to_char ("Syntax: numattacks <number>\n\r", ch);
    return FALSE;
  }
  number = atoi(argument);
  if (number < -1 || number > 100)
  {
    send_to_char("The number of attacks must be between -1 and 100.\n\r",ch);
    send_to_char("  (use -1 to ignore this feature and do it the old/normal way)\n\r",ch);
    return FALSE;
  }
  pMob->number_of_attacks = number;
  send_to_char ("Successfully set.\n\r",ch);
  return TRUE;
}
  
  
  

MEDIT (medit_desc)
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0')
    {
      send_to_char
	("Please use the Exodus Editor to enter the mob description:\n\r\n\r",
	 ch);
      edit_string (ch, &pMob->description);
      return TRUE;
    }
  send_to_char ("Syntax:  desc    - line edit\n\r", ch);
  return FALSE;
}

MEDIT (medit_long)
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0')
    {
      send_to_char ("Long description deleted.\n\r", ch);
      free_string (pMob->long_descr);
      pMob->long_descr = str_dup ("");
    }

  else
    {
      free_string (pMob->long_descr);
      strcat (argument, "\n\r");
      pMob->long_descr = str_dup (argument);
      pMob->long_descr[0] = UPPER (pMob->long_descr[0]);
      send_to_char ("Long description set.\n\r", ch);
    }
  return TRUE;
}

MEDIT (medit_short)
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax: short [short descr]\n\r", ch);
      return FALSE;
    }

  else
    {
      free_string (pMob->short_descr);
      pMob->short_descr = str_dup (argument);
      pMob->short_descr[0] = UPPER (pMob->short_descr[0]);
      send_to_char ("Short description set.\n\r", ch);
    }
  return TRUE;
}

MEDIT (medit_name)
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  name [string]\n\r", ch);
      return FALSE;
    }
  free_string (pMob->player_name);
  pMob->player_name = str_dup (argument);
  send_to_char ("Name set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_shop)
{
  MOB_INDEX_DATA *pMob;
  char command[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  argument = one_argument (argument, command);
  argument = one_argument (argument, arg1);
  EDIT_MOB (ch, pMob);
  if (command[0] == '\0')
    {
      send_to_char ("Syntax:  shop hours [#opening] [#closing]\n\r", ch);
      send_to_char ("         shop profit [#buying%] [#selling%]\n\r", ch);
      send_to_char ("         shop type [#0-4] [item type]\n\r", ch);
      send_to_char ("         shop delete [#0-4]\n\r", ch);
      return FALSE;
    }
  if (!str_cmp (command, "hours"))
    {
      if (arg1[0] == '\0' || !is_number (arg1)
	  || argument[0] == '\0' || !is_number (argument))
	{
	  send_to_char ("Syntax:  shop hours [#opening] [#closing]\n\r", ch);
	  return FALSE;
	}
      if (!pMob->pShop)
	{
	  pMob->pShop = new_shop ();
	  pMob->pShop->keeper = pMob->vnum;
	  shop_last->next = pMob->pShop;
	}
      pMob->pShop->open_hour = atoi (arg1);
      pMob->pShop->close_hour = atoi (argument);
      send_to_char ("Shop hours set.\n\r", ch);
      return TRUE;
    }
  if (!str_cmp (command, "profit"))
    {
      if (arg1[0] == '\0' || !is_number (arg1)
	  || argument[0] == '\0' || !is_number (argument))
	{
	  send_to_char
	    ("Syntax:  shop profit [#buying%] [#selling%]\n\r", ch);
	  return FALSE;
	}
      if (!pMob->pShop)
	{
	  pMob->pShop = new_shop ();
	  pMob->pShop->keeper = pMob->vnum;
	  shop_last->next = pMob->pShop;
	}
      pMob->pShop->profit_buy = atoi (arg1);
      pMob->pShop->profit_sell = atoi (argument);
      send_to_char ("Shop profit set.\n\r", ch);
      return TRUE;
    }
  if (!str_cmp (command, "type"))
    {
      char buf[MAX_INPUT_LENGTH];
      int value;
      if (arg1[0] == '\0' || !is_number (arg1) || argument[0] == '\0')
	{
	  send_to_char ("Syntax:  shop type [#0-4] [item type]\n\r", ch);
	  return FALSE;
	}
      if (atoi (arg1) >= MAX_TRADE)
	{
	  sprintf (buf, "May sell %d items max.\n\r", MAX_TRADE);
	  send_to_char (buf, ch);
	  return FALSE;
	}
      if ((value = flag_value (type_flags, argument)) == NO_FLAG)
	{
	  send_to_char ("That type of item is not known.\n\r", ch);
	  return FALSE;
	}
      if (!pMob->pShop)
	{
	  pMob->pShop = new_shop ();
	  pMob->pShop->keeper = pMob->vnum;
	  shop_last->next = pMob->pShop;
	}
      pMob->pShop->buy_type[atoi (arg1)] = value;
      send_to_char ("Shop type set.\n\r", ch);
      return TRUE;
    }
  if (!str_cmp (command, "delete"))
    {
      SHOP_DATA *pShop;
      SHOP_DATA *pShop_next;
      int value;
      int cnt = 0;
      if (arg1[0] == '\0' || !is_number (arg1))
	{
	  send_to_char ("Syntax:  shop delete [#0-4]\n\r", ch);
	  return FALSE;
	}
      value = atoi (argument);
      if (!pMob->pShop)
	{
	  send_to_char ("Non-existant shop.\n\r", ch);
	  return FALSE;
	}
      if (value == 0)
	{
	  pShop = pMob->pShop;
	  pMob->pShop = pMob->pShop->next;
	  free_shop (pShop);
	}

      else
	for (pShop = pMob->pShop, cnt = 0; pShop; pShop = pShop_next, cnt++)
	  {
	    pShop_next = pShop->next;
	    if (cnt + 1 == value)
	      {
		pShop->next = pShop_next->next;
		free_shop (pShop_next);
		break;
	      }
	  }
      send_to_char ("Shop deleted.\n\r", ch);
      return TRUE;
    }
  medit_shop (ch, "");
  return FALSE;
}


/* ROM medit functions: */
MEDIT (medit_sex)
{				/* Moved out of medit() due to naming conflicts -- Hugin */
  MOB_INDEX_DATA *pMob;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);
      if ((value = flag_value (sex_flags, argument)) != NO_FLAG)
	{
	  pMob->sex = value;
	  send_to_char ("Sex set.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax: sex [sex]\n\r"
		"Type '? sex' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_act)
{
  MOB_INDEX_DATA *pMob;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);
      if ((value = flag_value (act_flags, argument)) != NO_FLAG)
	{
	  pMob->act ^= value;
	  SET_BIT (pMob->act, ACT_IS_NPC);
	  send_to_char ("Act flag toggled.\n\r", ch);
	}
      if ((value = flag_value (act_flags2, argument)) != NO_FLAG)
	{
	  pMob->act2 ^= value;
	  SET_BIT (pMob->act, ACT_IS_NPC);
	  if (value & ACT_IMAGINARY)
	    pMob->form |= FORM_INSTANT_DECAY;
	  send_to_char ("Act flag toggled.\n\r", ch);
	}
      return TRUE;
    }
  send_to_char ("Syntax: act [flag]\n\r"
		"Type '? act' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_vocab)
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  vocab [string]\n\r", ch);
      return FALSE;
    }
  free_string (pMob->vocfile);
  pMob->vocfile = str_dup (argument);
  send_to_char ("VocabFile set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_script)
{
  MOB_INDEX_DATA *pMob;
  FILE *fp;
  char fullpath[1024];
  EDIT_MOB (ch, pMob);
  /*if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  script [string]\n\r", ch);
      return FALSE;
    }*/
  free_string (pMob->script_fn);
  if (!str_cmp(argument,"none"))
    pMob->script_fn = str_dup("");
  else pMob->script_fn = str_dup (argument);
  if (!str_cmp(argument,"") || !str_cmp(argument,"none"))
  {
	send_to_char ("Script removed.\n\r",ch);
	return TRUE;
  }
  sprintf (fullpath, "mobscr/%s", argument);
  if ((fp = fopen (fullpath, "r")) == NULL)
    {
      send_to_char ("No such script file exists.\n\r", ch);
      return FALSE;
    }
  fclose (fp);
  send_to_char ("Script filename set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_affect)
{
  MOB_INDEX_DATA *pMob;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);
      if ((value = flag_value (affect_flags, argument)) != NO_FLAG)
	{
	  pMob->affected_by ^= value;
	  send_to_char ("Affect flag toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax: affect [flag]\n\r"
		"Type '? affect' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_ac)
{
  MOB_INDEX_DATA *pMob;
  char arg[MAX_INPUT_LENGTH];
  int pierce, bash, slash, exotic;

  do
    {				/* So that I can use break and send the syntax in one place */
      if (argument[0] == '\0')
	break;
      EDIT_MOB (ch, pMob);
      argument = one_argument (argument, arg);
      if (!is_number (arg))
	break;
      pierce = atoi (arg);
      argument = one_argument (argument, arg);
      if (arg[0] != '\0')
	{
	  if (!is_number (arg))
	    break;
	  bash = atoi (arg);
	  argument = one_argument (argument, arg);
	}

      else
	bash = pMob->ac[AC_BASH];
      if (arg[0] != '\0')
	{
	  if (!is_number (arg))
	    break;
	  slash = atoi (arg);
	  argument = one_argument (argument, arg);
	}

      else
	slash = pMob->ac[AC_SLASH];
      if (arg[0] != '\0')
	{
	  if (!is_number (arg))
	    break;
	  exotic = atoi (arg);
	}

      else
	exotic = pMob->ac[AC_EXOTIC];
      pMob->ac[AC_PIERCE] = pierce;
      pMob->ac[AC_BASH] = bash;
      pMob->ac[AC_SLASH] = slash;
      pMob->ac[AC_EXOTIC] = exotic;
      send_to_char ("Ac set.\n\r", ch);
      return TRUE;
    }
  while (FALSE);		/* Just do it once.. */
  send_to_char
    ("Syntax:  ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\n\r"
     "help MOB_AC  gives a list of reasonable ac-values.\n\r", ch);
  return FALSE;
}

MEDIT (medit_form)
{
  MOB_INDEX_DATA *pMob;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);
      if ((value = flag_value (form_flags, argument)) != NO_FLAG)
	{
	  pMob->form ^= value;
	  send_to_char ("Form toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax: form [flags]\n\r"
		"Type '? form' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_part)
{
  MOB_INDEX_DATA *pMob;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);
      if ((value = flag_value (part_flags, argument)) != NO_FLAG)
	{
	  pMob->parts ^= value;
	  send_to_char ("Parts toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax: part [flags]\n\r"
		"Type '? part' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_imm)
{
  MOB_INDEX_DATA *pMob;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);
      if ((value = flag_value (imm_flags, argument)) != NO_FLAG)
	{
	  pMob->imm_flags ^= value;
	  send_to_char ("Immunity toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax: imm [flags]\n\r"
		"Type '? imm' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_res)
{
  MOB_INDEX_DATA *pMob;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);
      if ((value = flag_value (res_flags, argument)) != NO_FLAG)
	{
	  pMob->res_flags ^= value;
	  send_to_char ("Resistance toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax: res [flags]\n\r"
		"Type '? res' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_vuln)
{
  MOB_INDEX_DATA *pMob;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);
      if ((value = flag_value (vuln_flags, argument)) != NO_FLAG)
	{
	  pMob->vuln_flags ^= value;
	  send_to_char ("Vulnerability toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax: vuln [flags]\n\r"
		"Type '? vuln' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_recruit)
{
  MOB_INDEX_DATA *pMob;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);
      if ((value = flag_value (recruit_flags, argument)) != NO_FLAG)
	{
	  pMob->recruit_flags ^= value;
	  send_to_char ("Recruitment value toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax: recruit [flags]\n\r"
		"Type '? recruit' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_material)
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  material [material-name]\n\r"
		    "Type '? material' for a list of materials.\n\r", ch);
      return FALSE;
    }
  if (valid_material_name (argument))
    {
      free_string (pMob->material);
      pMob->material = str_dup (argument);
      send_to_char ("Material type set.\n\r", ch);
      return TRUE;
    }
  send_to_char
    ("That is not a valid material name.  Type '? material'.\n\r", ch);
  return FALSE;
}

MEDIT (medit_off)
{
  MOB_INDEX_DATA *pMob;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);
      if ((value = flag_value (off_flags, argument)) != NO_FLAG)
	{
	  pMob->off_flags ^= value;
	  send_to_char ("Offensive behaviour toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax: off [flags]\n\r"
		"Type '? off' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_size)
{
  MOB_INDEX_DATA *pMob;
  int value;
  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);
      if ((value = flag_value (size_flags, argument)) != NO_FLAG)
	{
	  pMob->size = value;
	  send_to_char ("Size set.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax: size [size]\n\r"
		"Type '? size' for a list of sizes.\n\r", ch);
  return FALSE;
}

MEDIT (medit_hitdice)
{
  static char syntax[] = "Syntax:  hitdice <number> d <type> + <bonus>\n\r";
  char *num, *type, *bonus, *cp;
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0')
    {
      send_to_char (syntax, ch);
      return FALSE;
    }
  num = cp = argument;
  while (isdigit (*cp))
    ++cp;
  while (*cp != '\0' && !isdigit (*cp))
    *(cp++) = '\0';
  type = cp;
  while (isdigit (*cp))
    ++cp;
  while (*cp != '\0' && !isdigit (*cp))
    *(cp++) = '\0';
  bonus = cp;
  while (isdigit (*cp))
    ++cp;
  if (*cp != '\0')
    *cp = '\0';
  if ((!is_number (num) || atoi (num) < 1)
      || (!is_number (type) || atoi (type) < 1)
      || (!is_number (bonus) || atoi (bonus) < 0))
    {
      send_to_char (syntax, ch);
      return FALSE;
    }
  pMob->hit[DICE_NUMBER] = atoi (num);
  pMob->hit[DICE_TYPE] = atoi (type);
  pMob->hit[DICE_BONUS] = atoi (bonus);
  send_to_char ("Hitdice set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_manadice)
{
  static char syntax[] = "Syntax:  manadice <number> d <type> + <bonus>\n\r";
  char *num, *type, *bonus, *cp;
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0')
    {
      send_to_char (syntax, ch);
      return FALSE;
    }
  num = cp = argument;
  while (isdigit (*cp))
    ++cp;
  while (*cp != '\0' && !isdigit (*cp))
    *(cp++) = '\0';
  type = cp;
  while (isdigit (*cp))
    ++cp;
  while (*cp != '\0' && !isdigit (*cp))
    *(cp++) = '\0';
  bonus = cp;
  while (isdigit (*cp))
    ++cp;
  if (*cp != '\0')
    *cp = '\0';
  if (!(is_number (num) && is_number (type) && is_number (bonus)))
    {
      send_to_char (syntax, ch);
      return FALSE;
    }
  if ((!is_number (num) || atoi (num) < 1)
      || (!is_number (type) || atoi (type) < 1)
      || (!is_number (bonus) || atoi (bonus) < 0))
    {
      send_to_char (syntax, ch);
      return FALSE;
    }
  pMob->mana[DICE_NUMBER] = atoi (num);
  pMob->mana[DICE_TYPE] = atoi (type);
  pMob->mana[DICE_BONUS] = atoi (bonus);
  send_to_char ("Manadice set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_damdice)
{
  static char syntax[] = "Syntax:  damdice <number> d <type> + <bonus>\n\r";
  char *num, *type, *bonus, *cp;
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0')
    {
      send_to_char (syntax, ch);
      return FALSE;
    }
  num = cp = argument;
  while (isdigit (*cp))
    ++cp;
  while (*cp != '\0' && !isdigit (*cp))
    *(cp++) = '\0';
  type = cp;
  while (isdigit (*cp))
    ++cp;
  while (*cp != '\0' && !isdigit (*cp))
    *(cp++) = '\0';
  bonus = cp;
  while (isdigit (*cp))
    ++cp;
  if (*cp != '\0')
    *cp = '\0';
  if (!(is_number (num) && is_number (type) && is_number (bonus)))
    {
      send_to_char (syntax, ch);
      return FALSE;
    }
  if ((!is_number (num) || atoi (num) < 1)
      || (!is_number (type) || atoi (type) < 1)
      || (!is_number (bonus) || atoi (bonus) < 0))
    {
      send_to_char (syntax, ch);
      return FALSE;
    }
  pMob->damage[DICE_NUMBER] = atoi (num);
  pMob->damage[DICE_TYPE] = atoi (type);
  pMob->damage[DICE_BONUS] = atoi (bonus);
  send_to_char ("Damdice set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_race)
{
  MOB_INDEX_DATA *pMob;
  int race;
  if (argument[0] != '\0' && (race = orace_lookup (argument)) != 0)
    {
      EDIT_MOB (ch, pMob);
      pMob->race = race;
      pMob->act |= race_table[race].act;
      pMob->off_flags |= race_table[race].off;
      pMob->imm_flags |= race_table[race].imm;
      pMob->res_flags |= race_table[race].res;
      pMob->vuln_flags |= race_table[race].vuln;
      pMob->form |= race_table[race].form;
      pMob->parts |= race_table[race].parts;
      send_to_char ("Race set.\n\r", ch);
      return TRUE;
    }
  if (argument[0] == '?')
    {
      char buf[MAX_STRING_LENGTH];
      send_to_char ("Available races are:", ch);
      for (race = 1; race_table[race].name != NULL; race++)
	{
	  if (((race - 1) % 3) == 0)
	    send_to_char ("\n\r", ch);
	  sprintf (buf, " %-15s", race_table[race].name);
	  send_to_char (buf, ch);
	}
      send_to_char ("\n\r", ch);
      return FALSE;
    }
  send_to_char ("Syntax:  race [race]\n\r"
		"Type 'race ?' for a list of races.\n\r", ch);
  return FALSE;
}

MEDIT (medit_damtype)
{
  MOB_INDEX_DATA *pMob;
  int dam_type;
  if (argument[0] != '\0' && (dam_type = attack_lookup (argument)) != 0)
    {
      EDIT_MOB (ch, pMob);
      pMob->dam_type = dam_type;
      send_to_char ("Damage type set.\n\r", ch);
      return TRUE;
    }
  if (argument[0] == '?')
    {
      char buf[MAX_STRING_LENGTH];
      send_to_char ("Available damage types are:", ch);
      for (dam_type = 0; attack_table[dam_type].name != NULL; dam_type++)
	{
	  if ((dam_type % 3) == 0)
	    send_to_char ("\n\r", ch);
	  sprintf (buf, " %-15s", attack_table[dam_type].name);
	  send_to_char (buf, ch);
	}
      send_to_char ("\n\r", ch);
      return FALSE;
    }
  send_to_char ("Syntax:  damtype [damage type]\n\r"
		"Type 'damtype ?' for a list of damage types.\n\r", ch);
  return FALSE;
}

MEDIT (medit_position)
{
  MOB_INDEX_DATA *pMob;
  char arg[MAX_INPUT_LENGTH];
  int value;
  argument = one_argument (argument, arg);
  switch (arg[0])
    {
    default:
      break;
    case 'S':
    case 's':
      if (str_prefix (arg, "start"))
	break;
      if ((value = flag_value (position_flags, argument)) == NO_FLAG)
	break;
      EDIT_MOB (ch, pMob);
      pMob->start_pos = value;
      send_to_char ("Start position set.\n\r", ch);
      return TRUE;
    case 'D':
    case 'd':
      if (str_prefix (arg, "default"))
	break;
      if ((value = flag_value (position_flags, argument)) == NO_FLAG)
	break;
      EDIT_MOB (ch, pMob);
      pMob->default_pos = value;
      send_to_char ("Default position set.\n\r", ch);
      return TRUE;
    }
  send_to_char ("Syntax:  position [start/default] [position]\n\r"
		"Type '? position' for a list of positions.\n\r", ch);
  return FALSE;
}

MEDIT (medit_wealth)
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  wealth [number]\n\r", ch);
      return FALSE;
    }
  pMob->wealth = atoi (argument);
  send_to_char ("Wealth set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_hitroll)
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  hitroll [number]\n\r", ch);
      return FALSE;
    }
  pMob->hitroll = atoi (argument);
  send_to_char ("Hitroll set.\n\r", ch);
  return TRUE;
}

REDIT (redit_travelexp)
{
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM (ch, pRoom);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  exploreexp [number]\n\r", ch);
      return FALSE;
    }
  if (atol (argument) < 0 || atol (argument) > 1000000)
    {
      send_to_char
	("Exportation point experience must be between 0 and 1000000.\n\r",
	 ch);
      return FALSE;
    }
  pRoom->tp_exp = atol (argument);
  send_to_char ("Exploration point experience set.\n\r", ch);
  return TRUE;
}

REDIT (redit_travellev)
{
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM (ch, pRoom);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  explorelev [number]\n\r", ch);
      return FALSE;
    }
  pRoom->tp_level = atoi (argument);
  send_to_char ("Exploration point maximum level set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_cardvnum)
{
  MOB_INDEX_DATA *pMob;
  OBJ_INDEX_DATA *obj;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  cardvnum [number]\n\r", ch);
      return FALSE;
    }
  if ((obj = get_obj_index(atoi(argument))))
    {
      if (obj->item_type != ITEM_CARD)
	{
	  send_to_char ("The object with that vnum is not a card!\n\r",ch);
	  return FALSE;
	}
      pMob->card_vnum = atoi (argument);
    }
  else 
    {
      send_to_char ("No object of that vnum exists.\n\r",ch);
      return FALSE;
    }
  send_to_char ("Cardvnum set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_str)
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument) || (atoi(argument) < 0 || atoi(argument) > 25))
    {
      send_to_char ("Syntax:  str [number]\n\r", ch);
      return FALSE;
    }
  pMob->perm_stat[STAT_STR] = atoi(argument);
  send_to_char ("Str set.\n\r",ch);
  return TRUE;
}

MEDIT (medit_int)
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument) || (atoi(argument) < 0 || atoi(argument) > 25))
    {
      send_to_char ("Syntax:  int [number]\n\r", ch);
      return FALSE;
    }
  pMob->perm_stat[STAT_INT] = atoi(argument);
  send_to_char ("Int set.\n\r",ch);
  return TRUE;
}

MEDIT (medit_wis)
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument) || (atoi(argument) < 0 || atoi(argument) > 25))
    {
      send_to_char ("Syntax:  wis [number]\n\r", ch);
      return FALSE;
    }
  pMob->perm_stat[STAT_WIS] = atoi(argument);
  send_to_char ("Wis set.\n\r",ch);
  return TRUE;
}

MEDIT (medit_dex)
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument) || (atoi(argument) < 0 || atoi(argument) > 25))
    {
      send_to_char ("Syntax:  dex [number]\n\r", ch);
      return FALSE;
    }
  pMob->perm_stat[STAT_DEX] = atoi(argument);
  send_to_char ("Dex set.\n\r",ch);
  return TRUE;
}

MEDIT (medit_con)
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument) || (atoi(argument) < 0 || atoi(argument) > 25))
    {
      send_to_char ("Syntax:  con [number]\n\r", ch);
      return FALSE;
    }
  pMob->perm_stat[STAT_CON] = atoi(argument);
  send_to_char ("Con set.\n\r",ch);
  return TRUE;
}

MEDIT (medit_cha)
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB (ch, pMob);
  if (argument[0] == '\0' || !is_number (argument) || (atoi(argument) < 0 || atoi(argument) > 25))
    {
      send_to_char ("Syntax:  cha [number]\n\r", ch);
      return FALSE;
    }
  pMob->perm_stat[STAT_CHA] = atoi(argument);
  send_to_char ("Cha set.\n\r",ch);
  return TRUE;
}


