#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include "merc.h"
#include "tables.h"
#include "olc.h"
#include "interp.h"
#include "obj_trig.h"

extern bool is_encrypted;
extern AREA_DATA *area_first;
extern const char *dir_name[];
extern int top_area;

/*
 * Local functions.
 */
AREA_DATA *get_area_data args ((int vnum));
extern ROOM_INDEX_DATA *room_index_hash[];
extern MOB_INDEX_DATA *mob_index_hash[];
int color_strlen args ((char *str));

void build_exitmsg args ((CHAR_DATA * ch, char *argument));
void build_entermsg args ((CHAR_DATA * ch, char *argument));

bool check_last_mob_reset args((ROOM_INDEX_DATA *rid));
bool check_last_obj_reset args((ROOM_INDEX_DATA *rid));
	

void do_olist (CHAR_DATA * ch, char *argument)
{
  OBJ_INDEX_DATA *pObjIndex;
  AREA_DATA *pArea;
  int vnum;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH * 2];
  char tcbuf[MAX_STRING_LENGTH * 30];
  bool fAll, found = FALSE;
  if (!IS_BUILDER(ch))
  {
	send_to_char("You do not have authorization to do that.\n\r", ch);
	return;
  }
  one_argument (argument, arg);
  if (arg[0] == '\0')
  {
	send_to_char("Syntax: olist <all/name/item_type>\n\r", ch);
	return;
  }
  strcpy(tcbuf, "Obj Listing:\n\r");
  fAll = !str_cmp(arg, "all");
  pArea = ch->in_room->area;
  
  for (vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++)
  {
	if ((pObjIndex = get_obj_index(vnum)) != NULL)
	{
		if (fAll || is_name (arg, pObjIndex->name)
		  || flag_value (type_flags, arg) == pObjIndex->item_type)
		{
			found = TRUE;
			sprintf(buf, "[%5d] [Lvl: %2d] [Name(s): %s]\n\r",
				pObjIndex->vnum, pObjIndex->level, pObjIndex->name);
			strcat (tcbuf, buf);
			sprintf(buf, "    [Type: %s] [Short: %s]\n\r",
				item_name (pObjIndex->item_type), pObjIndex->short_descr);
			strcat (tcbuf, buf);
		}
	}
    }
    page_to_char(tcbuf, ch);
    if (!found)
    {
	send_to_char("Obj(s) not found in this area.\n\r", ch);
	return;
    }
}

bool is_builder (CHAR_DATA * ch, AREA_DATA * pArea)
{
  static char buf[MAX_STRING_LENGTH],*helper2;//static since this function could be called a lot?
  if (IS_NPC (ch))
    return FALSE;
  if (ch->level >= 99)
    return (TRUE);

  //Shinowlan -- 12/13/2001 -- buildall allows a lower level imm to have global
  //build access without being level 99 or 100.
  if (ch->pcdata->buildall && ch->level >= 92)
    return (TRUE);
  if (pArea != NULL)
    {
      if (!str_cmp (pArea->creator, ch->name))
	return (TRUE);
      helper2 = one_argument(pArea->helper,buf);
      if (pArea->helper[0] != '\0' && (!str_cmp (helper2, ch->name) || !str_cmp(buf,ch->name)))
	return (TRUE);
    }
  return (FALSE);
}


/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool run_olc_editor (DESCRIPTOR_DATA * d)
{
  switch (d->editor)
    {
    case ED_AREA:
      aedit (d->character, d->incomm);
      break;
    case ED_ROOM:
      redit (d->character, d->incomm);
      break;
    case ED_OBJECT:
      oedit (d->character, d->incomm);
      break;
    case ED_MOBILE:
      medit (d->character, d->incomm);
      break;
    case BUILD_MAIN:
      build_menu (d->character, d->incomm);
      break;
    case BUILD_FLAGS:
      flags_menu (d->character, d->incomm);
      break;
    case BUILD_NAME:
      build_name (d->character, d->incomm);
      break;
    case BUILD_KEYWORDS:
      build_keywords (d->character, d->incomm);
      break;
    case BUILD_KEYWORDS_ADD:
      build_keywords_add (d->character, d->incomm);
      break;
    case BUILD_DOORS:
      build_doors (d->character, d->incomm);
      break;
    case BUILD_DOORS_FLAGS:
      build_doors_flags (d->character, d->incomm);
      break;
    case BUILD_DRESETS:
      build_resets (d->character, d->incomm);
      break;
    case BUILD_DNAME:
      build_door_name (d->character, d->incomm);
      break;
    case ED_VOCAB_MAIN:
      vocab_main (d->character, d->incomm);
      break;
    case ED_VOCAB_EDIT:
      vocab_edit (d->character, d->incomm);
      break;
    case ED_VOCAB_ADD_WORDS:
      vocab_add_words (d->character, d->incomm);
      break;
    case ED_VOCAB_ADD_MOOD:
      vocab_add_mood (d->character, d->incomm);
      break;
    case ED_VOCAB_ADD_MOOD_OFFSET:
      vocab_add_mood_offset (d->character, d->incomm);
      break;
    case ED_VOCAB_ADD_TEXT:
      vocab_add_text (d->character, d->incomm);
      break;
    case ED_VOCAB_ADD_INCLUDE:
      vocab_add_include (d->character, d->incomm);
      break;
    case MAIL_GET_SUBJECT:
      mail_get_subject (d->character, d->incomm);
      break;
    case SUBMIT_MOB_VNUM:
      submit_mob_vnum (d->character, d->incomm);
      break;
    case SUBMIT_MOB_NAME:
      submit_mob_name (d->character, d->incomm);
      break;
    case SUBMIT_ROOM_VNUM:
      submit_room_vnum (d->character, d->incomm);
      break;
    case EDIT_MENU:
      edit_menu (d->character, d->incomm);
      break;
    case EDIT_LINE_NUMBER:
      edit_line_number (d->character, d->incomm);
      break;
    case INSERT_LINE_NUMBER:
      insert_line_number (d->character, d->incomm);
      break;
    case DELE_LINE_NUMBER:
      dele_line_number (d->character, d->incomm);
      break;
    case MAIL_TEXT:
      if (d->pString == NULL)
	{
	  char tcbuf[MAX_STRING_LENGTH * 3];
	  send_to_char ("Sending Mail.\n\r", d->character);
	  edit_done (d->character);
	  sprintf (tcbuf, "%s%s\n\n", d->help_info, d->help_name);
	  send_mail (tcbuf);
	}
      break;

/*    case SAVE_CLAN_HELP:
      if(d->pString == NULL)
	{
	  send_to_char("Saving clan help.\n\r", d->character);
	  edit_done(d->character);
	  save_help(d->help_name, d->help_info, "clans.hlp");
	}
      break; */
    case SAVE_ANY_HELP:
      if (d->pString == NULL)
	{
	  send_to_char ("Saving help file.\n\r", d->character);
	  edit_done (d->character);
	  save_help (d->help_name, d->help_info, "online.hlp");
	}
      break;
    case SUBMIT_TEXT:
      if (d->pString == NULL)
	{
	  char tcbuf[MAX_STRING_LENGTH * 3];
	  send_to_char ("Sending Submission.\n\r", d->character);
	  edit_done (d->character);
	  sprintf (tcbuf, "%s%s\n\n", d->help_info, d->help_name);
	  send_submission ("weave@best.com", tcbuf);
	}
      break;
    case ED_OBJTRIG:
      otedit (d->character, d->incomm);
      break;
    case BUILD_EXITMSG:
      build_exitmsg(d->character, d->incomm);
      break;
    case BUILD_ENTERMSG:
      build_entermsg(d->character, d->incomm);
      break;

    default:
      return FALSE;
      break;
    }
  return TRUE;
}

char *get_player_email (char *pname)
{
  static char email_addr[32];
  char strsave[MAX_STRING_LENGTH];
  char letter, *word;
  FILE *ffp;
  sprintf (strsave, "%s%c/%s", PLAYER_DIR, toupper (pname[0]),
	   capitalize (pname));
  if ((ffp = fopen (strsave, "r")) == NULL)
    return (NULL);
  is_encrypted = TRUE;
  while (1)
    {
      if (feof (ffp))
	{
	  fclose (ffp);
	  is_encrypted = FALSE;
	  return (NULL);
	}
      letter = fread_letter (ffp);
      if (letter != 'E')
	{
	  if (feof (ffp))
	    {
	      fclose (ffp);
	      is_encrypted = FALSE;
	      return (NULL);
	    }
	  fread_to_eol (ffp);
	  continue;
	}
      word = fread_word (ffp);
      if (!str_cmp (word, "mail"))
	{
	  strcpy (email_addr, fread_string (ffp));
	  fclose (ffp);
	  is_encrypted = FALSE;
	  return (email_addr);
	}
    }
  is_encrypted = FALSE;
}

void send_mail (char *msg_text)
{
  FILE *sfp;
  if (fork () == 0)
    {
      if ((sfp = popen ("/usr/lib/sendmail -t", "w")) == NULL)
	exit (0);
      fprintf (sfp, "%s\n", msg_text);
      pclose (sfp);
      exit (0);
    }
}
char *read_current_help (char *help_name, char *filename)
{
  FILE *hfp;
  char *name_buf, *text_buf;
  int c;
  if ((hfp = fopen (filename, "r")) == NULL)
    return (NULL);
  while (1)
    {
      c = fread_letter (hfp);
      if (c == '*' || c == EOF)
	{
	  fclose (hfp);
	  return (NULL);
	}
      name_buf = fread_string (hfp);
      text_buf = fread_string (hfp);
      if (!str_cmp (name_buf, help_name))
	{
	  fclose (hfp);
	  return (text_buf);
	}
      if (feof (hfp))
	{
	  fclose (hfp);
	  return (NULL);
	}
    }
}
void save_help (char *help_name, char *help_text, char *filename)
{
  FILE *hfp, *nfp;
  char c, *name_buf, *text_buf;
  if (help_name == NULL || help_text == NULL)
    return;
  if ((hfp = fopen (filename, "r")) == NULL)
    {
      if ((hfp = fopen (filename, "w")) == NULL)
	return;
      fprintf (hfp, "# %s~\n%s~\n\n*", help_name, help_text);
      fclose (hfp);
      return;
    }
  nfp = fopen ("tmpwrite.fil", "w");
  while (1)
    {
      c = fread_letter (hfp);
      if (c == '*' || c == EOF)
	{
	  fprintf (nfp, "# %s~\n%s~\n\n*", help_name, help_text);
	  fclose (hfp);
	  fclose (nfp);
	  rename ("tmpwrite.fil", filename);
	  return;
	}
      name_buf = fread_string (hfp);
      text_buf = fread_string (hfp);
      if (str_cmp (name_buf, help_name))
	fprintf (nfp, "# %s~\n%s~\n\n", name_buf, text_buf);
    }
}
void remove_help (char *help_name, char *filename)
{
  FILE *hfp, *nfp;
  char c, *name_buf, *text_buf;
  if ((hfp = fopen (filename, "r")) == NULL)
    return;
  nfp = fopen ("tmpwrite.fil", "w");
  while (1)
    {
      c = fread_letter (hfp);
      if (c == '*' || c == EOF)
	{
	  fclose (hfp);
	  fclose (nfp);
	  rename ("tmpwrite.fil", filename);
	  return;
	}
      name_buf = fread_string (hfp);
      text_buf = fread_string (hfp);
      if (str_cmp (name_buf, help_name))
	fprintf (nfp, "# %s~\n%s~\n\n", name_buf, text_buf);
    }
}
void send_submission (char *send_to, char *msg_text)
{
  FILE *sfp;
  if (fork () == 0)
    {
      if ((sfp = popen ("/usr/lib/sendmail -t", "w")) == NULL)
	exit (0);
      fprintf (sfp, "From: exodus@mischief.com\nTo: %s\n%s", send_to,
	       msg_text);
      pclose (sfp);
      if ((sfp = fopen ("mobscr/submit.txt", "a")) == NULL)
	exit (0);
      fprintf (sfp, "From: exodus@mischief.com\nTo: %s\n%s\n", send_to,
	       msg_text);
      fprintf (sfp, "-----\n");
      fclose (sfp);
      exit (0);
    }
}
void submit_mob_vnum (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_STRING_LENGTH];
  bool notspace = FALSE;
  char *cptr;
  if (IS_NPC (ch))
    return;
  cptr = argument;
  while (*cptr != '\0')
    {
      if (!isspace (*cptr))
	{
	  notspace = TRUE;
	  break;
	}
      cptr++;
    }
  if (!notspace)
    {
      send_to_char ("Submission cancelled.\n\r", ch);
      edit_done (ch);
      return;
    }
  if (!is_number (argument))
    {
      send_to_char ("You must supply a mob vnum.\n\r", ch);
      send_to_char ("Please enter the mob's vnum:\n\r\n\r", ch);
      return;
    }
  if (get_mob_index (atoi (argument)) == NULL)
    {
      send_to_char ("No such mob (vnum) found.\n\r", ch);
      send_to_char ("Please enter the mob's vnum:\n\r\n\r", ch);
      return;
    }
  sprintf (tcbuf,
	   "Reply-To: %s\nSubject: %s MobScript\n\nMob Vnum: %s",
	   ch->pcdata->email_addr, ch->name, argument);
  ch->desc->submit_info = str_dup (tcbuf);
  send_to_char ("Please enter the mob's name:\n\r", ch);
  ch->desc->editor = SUBMIT_MOB_NAME;
}

void mail_get_subject (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_STRING_LENGTH];
  bool notspace = FALSE;
  char *cptr;
  cptr = argument;
  while (*cptr != '\0')
    {
      if (!isspace (*cptr))
	{
	  notspace = TRUE;
	  break;
	}
      cptr++;
    }
  if (!notspace)
    {
      send_to_char ("Mail cancelled.\n\r", ch);
      edit_done (ch);
      return;
    }
  sprintf (tcbuf, "%sSubject: %s\n\n", (char *) ch->desc->help_info,
	   argument);
  free_string (ch->desc->help_info);
  ch->desc->help_info = str_dup (tcbuf);
  send_to_char
    ("Please use the Exodus Editor the enter the text of your mail.\n\r\n\r",
     ch);
  ch->desc->editor = MAIL_TEXT;
  edit_string (ch, &ch->desc->help_name);
}

void submit_mob_name (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_STRING_LENGTH];
  bool notspace = FALSE;
  char *cptr;
  cptr = argument;
  while (*cptr != '\0')
    {
      if (!isspace (*cptr))
	{
	  notspace = TRUE;
	  break;
	}
      cptr++;
    }
  if (!notspace)
    {
      send_to_char ("Submission cancelled.\n\r", ch);
      edit_done (ch);
      return;
    }
  sprintf (tcbuf, "%s\nMob Name: %s", (char *) ch->desc->submit_info,
	   argument);
  free_string (ch->desc->submit_info);
  ch->desc->submit_info = str_dup (tcbuf);
  send_to_char ("Please enter the room vnum:\n\r", ch);
  ch->desc->editor = SUBMIT_ROOM_VNUM;
}

void submit_room_vnum (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_STRING_LENGTH];
  bool notspace = FALSE;
  char *cptr;
  cptr = argument;
  while (*cptr != '\0')
    {
      if (!isspace (*cptr))
	{
	  notspace = TRUE;
	  break;
	}
      cptr++;
    }
  if (!notspace)
    {
      send_to_char ("Submission cancelled.\n\r", ch);
      edit_done (ch);
      return;
    }
  if (!is_number (argument))
    {
      send_to_char ("You must supply a room vnum.\n\r", ch);
      return;
    }
  if (get_room_index (atoi (argument)) == NULL)
    {
      send_to_char ("No such room (vnum) found.\n\r", ch);
      return;
    }
  sprintf (tcbuf, "%s\nRoom Vnum: %s\n", ch->desc->help_info, argument);
  free_string (ch->desc->help_info);
  ch->desc->help_info = str_dup (tcbuf);
  send_to_char
    ("Please use the Exodus Editor to enter the mobscript information:\n\r\n\r",
     ch);
  ch->desc->editor = SUBMIT_TEXT;
  edit_string (ch, &ch->desc->help_name);
}

char *olc_ed_name (CHAR_DATA * ch)
{
  static char buf[10];
  buf[0] = '\0';
  switch (ch->desc->editor)
    {
    case ED_AREA:
      sprintf (buf, "AEdit");
      break;
    case ED_ROOM:
      sprintf (buf, "REdit");
      break;
    case ED_OBJECT:
      sprintf (buf, "OEdit");
      break;
    case ED_MOBILE:
      sprintf (buf, "MEdit");
      break;
    default:
      sprintf (buf, " ");
      break;
    }
  return buf;
}

char *olc_ed_vnum (CHAR_DATA * ch)
{
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *pRoom;
  OBJ_INDEX_DATA *pObj;
  MOB_INDEX_DATA *pMob;
  static char buf[10];
  buf[0] = '\0';
  switch (ch->desc->editor)
    {
    case ED_AREA:
      pArea = (AREA_DATA *) ch->desc->pEdit;
      sprintf (buf, "%d", pArea ? pArea->vnum : 0);
      break;
    case ED_ROOM:
      pRoom = ch->in_room;
      sprintf (buf, "%d", pRoom ? pRoom->vnum : 0);
      break;
    case ED_OBJECT:
      pObj = (OBJ_INDEX_DATA *) ch->desc->pEdit;
      sprintf (buf, "%d", pObj ? pObj->vnum : 0);
      break;
    case ED_MOBILE:
      pMob = (MOB_INDEX_DATA *) ch->desc->pEdit;
      sprintf (buf, "%d", pMob ? pMob->vnum : 0);
      break;
    default:
      sprintf (buf, " ");
      break;
    }
  return buf;
}


/*****************************************************************************
 Name:		show_olc_cmds
 Purpose:	Format up the commands from given table.
 Called by:	show_commands(olc_act.c).
 ****************************************************************************/
void show_olc_cmds (CHAR_DATA * ch, const struct olc_cmd_type *olc_table)
{
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  int cmd;
  int col;
  buf1[0] = '\0';
  col = 0;
  for (cmd = 0; olc_table[cmd].name[0] != '\0'; cmd++)
    {
      sprintf (buf, "%-15.15s", olc_table[cmd].name);
      strcat (buf1, buf);
      if (++col % 5 == 0)
	strcat (buf1, "\n\r");
    }
  if (col % 5 != 0)
    strcat (buf1, "\n\r");
  send_to_char (buf1, ch);
  return;
}


/*****************************************************************************
 Name:		show_commands
 Purpose:	Display all olc commands.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_commands (CHAR_DATA * ch, char *argument)
{
  switch (ch->desc->editor)
    {
    case ED_AREA:
      show_olc_cmds (ch, aedit_table);
      send_to_char("imp_only no_repop_wia no_trees no_quit\n\r",ch);
      break;
    case ED_ROOM:
      show_olc_cmds (ch, redit_table);
      break;
    case ED_OBJECT:
      show_olc_cmds (ch, oedit_table);
      break;
    case ED_MOBILE:
      show_olc_cmds (ch, medit_table);
      break;
    case ED_OBJTRIG:
      show_olc_cmds (ch, otedit_table);
      break;
    default:
      break;
    }
  return FALSE;
}


/*****************************************************************************
 *                           Interpreter Tables.                             *
 *****************************************************************************/
const struct olc_cmd_type aedit_table[] = {
/*  {   command         function        }, */

  {"age", aedit_age},
  {"author", aedit_creator},
  {"commands", show_commands},
  {"create", aedit_create},
  {"filename", aedit_file},
  {"name", aedit_name},
  {"reset", aedit_reset},
  {"show", aedit_show},
  {"helper", aedit_helper},
  {"vnum", aedit_vnum},
  {"lvnum", aedit_lvnum},
  {"uvnum", aedit_uvnum},
  {"lev", aedit_lev},
  {"llev", aedit_llev},
  {"ulev", aedit_ulev},
  {"clan", aedit_clan},
  {"clanwar", aedit_clanwar},
  {"affluence", aedit_affluence},
  {"inconstruction", aedit_inconstruction},
  {"helps", aedit_helps},
  {"?", show_help}, {"version", show_version}, {"", 0,}
};
const struct olc_cmd_type redit_table[] = {
/*  {   command         function        }, */

  {"commands", show_commands},
  {"create", redit_create},
  {"desc", redit_desc},
  {"ed", redit_ed},
  {"format", redit_format},
  {"name", redit_name},
  {"epl", redit_epl},
  {"exitmsg", redit_exitmsg},
  {"entermsg", redit_entermsg},
  {"exploremsg", redit_travelmsg},
  {"exploreexp", redit_travelexp},
  {"explorelev", redit_travellev},
  {"show", redit_show},
  {"rate", redit_rate},
  {"Class", redit_Class},
  {"race", redit_race},
  {"level", redit_level},
  {"maxnum", redit_maxnum},
  {"maxmsg", redit_maxmsg},
  {"north", redit_north},
  {"south", redit_south},
  {"east", redit_east},
  {"west", redit_west},
  {"up", redit_up},
  {"down", redit_down},
  {"purge", redit_purge},
  {"clan", redit_clan}, {"owner", redit_owner},
  /* New reset commands. */
  {"mreset", redit_mreset}, {"oreset", redit_oreset},
  /* {"mlist", redit_mlist},
     {"olist", redit_olist}, */
  {"mshow", redit_mshow}, {"oshow", redit_oshow},
  // Adeon 6/30/03 -- sinking rooms
  {"sinkvnum", redit_sinkvnum},
  {"sinktimer", redit_sinktimer},
  {"sinkwarning", redit_sinkwarning},
  {"sinkmsg", redit_sinkmsg},
  {"sinkmsgothers", redit_sinkmsgothers},
  {"?", show_help}, {"version", show_version}, {"", 0,}
};
const struct olc_cmd_type oedit_table[] = {
/*  {   command         function        }, */

  {"addaffect", oedit_addaffect},
  {"addpermaffect", oedit_addpermaffect},
  {"addskillaffect", oedit_addskillaffect},
  {"commands", show_commands},
  {"cost", oedit_cost},
  {"create", oedit_create},
  {"delaffect", oedit_delaffect},
  {"ed", oedit_ed},
  {"long", oedit_long},
  {"initmsg", oedit_initmsg},
  {"name", oedit_name},
  {"purge", oedit_purge},
  {"short", oedit_short},
  {"arrival", oedit_arrival},
  {"departure", oedit_departure},
  {"passenger", oedit_passenger},
  {"rarity", oedit_rarity},
  {"show", oedit_show},
  {"v0", oedit_value0},
  {"v1", oedit_value1},
  {"v2", oedit_value2},
  {"v3", oedit_value3},
  {"v4", oedit_value4},
  {"v5", oedit_value5},
  {"v6", oedit_value6},
  {"v7", oedit_value7},
  {"v8", oedit_value8},
  {"v9", oedit_value9},
  {"v10", oedit_value10},
  {"v11", oedit_value11},
  {"v12", oedit_value12},
  {"weight", oedit_weight}, {"extra", oedit_extra},	/* ROM */
  {"wear", oedit_wear},		/* ROM */
  // Akamai 4/30/99 - support Class/race objects
  {"Class", oedit_Class},
  {"race", oedit_race},
  {"clan", oedit_clan},
  {"owner", oedit_owner}, {"type", oedit_type},	/* ROM */
  {"material", oedit_material},	/* ROM */
  {"level", oedit_level},	/* ROM */
  {"timer", oedit_timer},	/* ROM */
  {"condition", oedit_condition},	/* ROM */
  {"entermsg", oedit_entermsg},
  {"exitmsg", oedit_exitmsg},
  {"objtrigger", oedit_objtrigger},
  {"trapflag", oedit_trapflag},
  {"?", show_help}, {"version", show_version}, {"", 0,}
};
const struct olc_cmd_type medit_table[] = {
/*  {   command         function        }, */

  {"alignment", medit_align},
  {"commands", show_commands},
  {"create", medit_create},
  {"desc", medit_desc},
  {"level", medit_level},
  {"mood", medit_mood},
  {"movement", medit_movement},
  {"maxweight", medit_maxweight},
  {"defensebonus", medit_defbonus},
  {"attackbonus", medit_attackbonus},
  {"long", medit_long},
  {"name", medit_name},
  {"purge", medit_purge},
  {"shop", medit_shop},
  {"short", medit_short},
  {"vocab", medit_vocab},
  {"script", medit_script},
  {"show", medit_show},
  {"spec", medit_spec}, {"sex", medit_sex},	/* ROM */
  {"ac", medit_ac},		/* ROM */
  {"act", medit_act},		/* ROM */
  {"affect", medit_affect},	/* ROM */
  {"form", medit_form},		/* ROM */
  {"part", medit_part},		/* ROM */
  {"imm", medit_imm},		/* ROM */
  {"res", medit_res},		/* ROM */
  {"vuln", medit_vuln},		/* ROM */
  {"recruit", medit_recruit},
  {"psvnum", medit_psvnum},
  {"pscost", medit_pscost},
  {"material", medit_material},	/* ROM */
  {"off", medit_off},		/* ROM */
  {"size", medit_size},		/* ROM */
  {"hitdice", medit_hitdice},	/* ROM */
  {"manadice", medit_manadice},	/* ROM */
  {"damdice", medit_damdice},	/* ROM */
  {"race", medit_race},		/* ROM */
  {"position", medit_position},	/* ROM */
  {"wealth", medit_wealth},	/* ROM */
  {"hitroll", medit_hitroll},	/* ROM */
  {"mobgroup", medit_mobgroup},
  {"damtype", medit_damtype},
  {"blockexit", medit_blockexit}, // Adeon 7/17/03
  {"numattacks", medit_numattacks}, // Iblis 2/05/04
  {"cardvnum", medit_cardvnum}, //Iblis 8/29/04
  {"str", medit_str}, //Iblis 8/29/04
  {"int", medit_int}, //Iblis 8/29/04
  {"wis", medit_wis}, //Iblis 8/29/04
  {"dex", medit_dex}, //Iblis 8/29/04
  {"con", medit_con}, //Iblis 8/29/04
  {"cha", medit_cha}, //Iblis 8/29/04
  {"?", show_help},
  {"version", show_version},

  {"", 0,}
};

// Adeon 7/19/03 -- Object trigger editing
const struct olc_cmd_type otedit_table[] = {
	{"create", otedit_create},
	{"commands", show_commands},
	{"show", otedit_show},
	{"chance",otedit_chance},
	{"extra",otedit_extra},
//	{"addparent", otedit_addparent},
//	{"remparent", otedit_remparent},
	{"name", otedit_name},
	{"trigger", otedit_trigger},
	{"addaction", otedit_addaction},
	{"remaction", otedit_remaction},
	{"purge", otedit_purge},
	{"", 0,}
};

/*****************************************************************************
 *                          End Interpreter Tables.                          *
 *****************************************************************************/

/*****************************************************************************
 Name:		get_area_data
 Purpose:	Returns pointer to area with given vnum.
 Called by:	do_aedit(olc.c).
 ****************************************************************************/
AREA_DATA *get_area_data (int vnum)
{
  AREA_DATA *pArea;
  for (pArea = area_first; pArea; pArea = pArea->next)
    {
      if (pArea->vnum == vnum)
	return pArea;
    }
  return 0;
}


/*****************************************************************************
 Name:		edit_done
 Purpose:	Resets builder information on completion.
 Called by:	aedit, redit, oedit, medit(olc.c)
 ****************************************************************************/
bool edit_done (CHAR_DATA * ch)
{
  ch->desc->pEdit = NULL;
  ch->desc->editor = 0;
  return FALSE;
}


/*****************************************************************************
 *                              Interpreters.                                *
 *****************************************************************************/

/* Area Interpreter, called by do_aedit. */
void aedit (CHAR_DATA * ch, char *argument)
{
  AREA_DATA *pArea;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int cmd;
  int value;
  if (IS_NPC (ch))
    return;
  EDIT_AREA (ch, pArea);
  smash_tilde (argument);
  strcpy (arg, argument);
  argument = one_argument (argument, command);
  if (!is_builder (ch, pArea))
    send_to_char ("You are not this area's creator.\n\r", ch);
  if (command[0] == '\0')
    {
      aedit_show (ch, argument);
      return;
    }
  if (!str_cmp (command, "done"))
    {
      edit_done (ch);
      return;
    }
  if (!str_cmp (command, "vnum"))
    {
      aedit_vnum (ch, argument);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }
  if (!is_builder (ch, pArea))
    {
      interpret (ch, arg);
      return;
    }
  if ((value = flag_value (area_flags, arg)) != NO_FLAG)
    {
      TOGGLE_BIT (pArea->area_flags, value);
      send_to_char ("Flag toggled.\n\r", ch);
      return;
    }

  /* Search Table and Dispatch Command. */
  for (cmd = 0; aedit_table[cmd].name[0] != '\0'; cmd++)
    {
      if (!str_prefix (command, aedit_table[cmd].name))
	{
	  if ((*aedit_table[cmd].olc_fun) (ch, argument))
	    {
	      SET_BIT (pArea->area_flags, AREA_CHANGED);
	      return;
	    }
	  else
	    return;
	}
    }

  /* Default to Standard Interpreter. */
  interpret (ch, arg);
  return;
}


/* Room Interpreter, called by do_redit. */
void redit (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *pRoom;
  AREA_DATA *pArea;
  char arg[MAX_STRING_LENGTH];
  char command[MAX_INPUT_LENGTH];
  int cmd;
  int value;
  if (IS_NPC (ch))
    return;
  EDIT_ROOM (ch, pRoom);
  pArea = pRoom->area;
  smash_tilde (argument);
  strcpy (arg, argument);
  if (!IS_NPC (ch) && argument[0] != '\0')
    {
      short x;
      char command[MAX_INPUT_LENGTH];
      if (argument[0] == '!')
	{
	  if (argument[1] == '?')
	    {
	      char big_buf[MAX_STRING_LENGTH * 2];
	      strcpy (big_buf, "Command History:\n\r\n\r");
	      for (x = MAX_HISTORY - 1; x >= 0; x--)
		if (ch->pcdata->history[x] != NULL)
		  sprintf (big_buf + strlen (big_buf),
			   "%2d.\t%s\n\r", x + 1, ch->pcdata->history[x]);
	      page_to_char (big_buf, ch);
	      return;
	    }
	  else if (argument[1] == '\0' || argument[1] == '!')
	    {
	      if (ch->pcdata->history[0] != NULL)
		{
		  strcpy (command, ch->pcdata->history[0]);
		  redit (ch, command);
		  return;
		}
	    }
	  else
	    {
	      short num;
	      num = atoi (argument + 1);
	      if (num > 0 && num < 21)
		{
		  if (ch->pcdata->history[num - 1] != NULL)
		    {
		      strcpy (command, ch->pcdata->history[num - 1]);
		      redit (ch, command);
		    }
		  else
		    send_to_char ("No such history entry.\n\r", ch);
		  return;
		}
	      else
		{
		  send_to_char ("Huh?\n\r", ch);
		  return;
		}
	    }
	}
      else
	{
	  if (ch->pcdata->history[19] != NULL)
	    free_string (ch->pcdata->history[19]);
	  for (x = 18; x >= 0; x--)
	    if (ch->pcdata->history[x] != NULL)
	      ch->pcdata->history[x + 1] = ch->pcdata->history[x];
	  ch->pcdata->history[0] = str_dup (argument);
	}
    }
  argument = one_argument (argument, command);
  if (!is_builder (ch, pArea))
    send_to_char ("You are not this area's creator.\n\r", ch);
  if (command[0] == '\0')
    {
      redit_show (ch, argument);
      return;
    }
  if (!str_cmp (command, "done"))
    {
      edit_done (ch);
      return;
    }
  if (!is_builder (ch, pArea))
    {
      interpret (ch, arg);
      return;
    }

  /* Search Table and Dispatch Command. */
  for (cmd = 0; redit_table[cmd].name[0] != '\0'; cmd++)
    {
      if (!str_prefix (command, redit_table[cmd].name))
	{
	  if ((*redit_table[cmd].olc_fun) (ch, argument))
	    SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
    }
  if ((value = flag_value (room_flags, arg)) != NO_FLAG)
    {
      if ((value != ROOM_ARENA_REGISTRATION && value != ROOM_ARENA
	   && value != ROOM_SPECTATOR) || ch->level == 100)
	{
	  TOGGLE_BIT (pRoom->room_flags, value);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  send_to_char ("Room flag toggled.\n\r", ch);
	}
      else
	send_to_char ("That room flag can not be toggled.\n\r", ch);
      return;
    }
  // Adeon -- 6/30/03
  if ((value = flag_value (room_flags2, arg)) != NO_FLAG)
    {
      TOGGLE_BIT (pRoom->room_flags2, value);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      send_to_char ("Room flag toggled.\n\r", ch);
      return;
    }
  if ((value = flag_value (sector_flags, arg)) != NO_FLAG)
    {
      pRoom->sector_type = value;
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      send_to_char ("Sector type set.\n\r", ch);
      return;
    }
  else if (!str_cmp ("no_transport", arg))
    {
      TOGGLE_BIT (pRoom->race_flags, ROOM_NOTRANSPORT);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      send_to_char ("Room flag toggled.\n\r", ch);
      return;
    }
  else if (!str_cmp ("player_store", arg))
    {
      TOGGLE_BIT (pRoom->race_flags, ROOM_PLAYERSTORE);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      send_to_char ("Room flag toggled.\n\r", ch);
      return;
    }
  else if (!str_cmp ("no_teleport", arg))
    {
      TOGGLE_BIT (pRoom->race_flags, ROOM_NOTELEPORT);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      send_to_char ("Room flag toggled.\n\r", ch);
      return;
    }

  /* Default to Standard Interpreter. */
  interpret (ch, arg);
  return;
}


/* Object Interpreter, called by do_oedit. */
void oedit (CHAR_DATA * ch, char *argument)
{
  AREA_DATA *pArea;
  OBJ_INDEX_DATA *pObj;
  char arg[MAX_STRING_LENGTH];
  char command[MAX_INPUT_LENGTH];
  int cmd;
  smash_tilde (argument);
  strcpy (arg, argument);
  if (IS_NPC (ch))
    return;
  if (argument[0] != '\0')
    {
      short x;
      char command[MAX_INPUT_LENGTH];
      if (argument[0] == '!')
	{
	  if (argument[1] == '?')
	    {
	      char big_buf[MAX_STRING_LENGTH * 2];
	      strcpy (big_buf, "Command History:\n\r\n\r");
	      for (x = MAX_HISTORY - 1; x >= 0; x--)
		if (ch->pcdata->history[x] != NULL)
		  sprintf (big_buf + strlen (big_buf),
			   "%2d.\t%s\n\r", x + 1, ch->pcdata->history[x]);
	      page_to_char (big_buf, ch);
	      return;
	    }
	  else if (argument[1] == '\0' || argument[1] == '!')
	    {
	      if (ch->pcdata->history[0] != NULL)
		{
		  strcpy (command, ch->pcdata->history[0]);
		  oedit (ch, command);
		  return;
		}
	    }
	  else
	    {
	      short num;
	      num = atoi (argument + 1);
	      if (num > 0 && num < 21)
		{
		  if (ch->pcdata->history[num - 1] != NULL)
		    {
		      strcpy (command, ch->pcdata->history[num - 1]);
		      oedit (ch, command);
		    }
		  else
		    send_to_char ("No such history entry.\n\r", ch);
		  return;
		}
	      else
		{
		  send_to_char ("Huh?\n\r", ch);
		  return;
		}
	    }
	}
      else
	{
	  if (ch->pcdata->history[19] != NULL)
	    free_string (ch->pcdata->history[19]);
	  for (x = 18; x >= 0; x--)
	    if (ch->pcdata->history[x] != NULL)
	      ch->pcdata->history[x + 1] = ch->pcdata->history[x];
	  ch->pcdata->history[0] = str_dup (argument);
	}
    }
  argument = one_argument (argument, command);
  EDIT_OBJ (ch, pObj);
  pArea = pObj->area;
  if (!is_builder (ch, pArea))
    send_to_char ("You are not this area's creator.\n\r", ch);
  if (command[0] == '\0')
    {
      oedit_show (ch, argument);
      return;
    }
  if (!str_cmp (command, "done"))
    {
      edit_done (ch);
      return;
    }
  if (!is_builder (ch, pArea))
    {
      interpret (ch, arg);
      return;
    }

  /* Search Table and Dispatch Command. */
  for (cmd = 0; oedit_table[cmd].name[0] != '\0'; cmd++)
    {
      if (!str_prefix (command, oedit_table[cmd].name))
	{
	  if ((*oedit_table[cmd].olc_fun) (ch, argument))
	    SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
    }

  /* Default to Standard Interpreter. */
  interpret (ch, arg);
  return;
}


/* Mobile Interpreter, called by do_medit. */
void medit (CHAR_DATA * ch, char *argument)
{
  AREA_DATA *pArea;
  MOB_INDEX_DATA *pMob;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_STRING_LENGTH];
  int cmd;
  smash_tilde (argument);
  strcpy (arg, argument);
  if (IS_NPC (ch))
    {
      send_to_char (ch->name, ch);
      send_to_char (act_bit_name (ch->act), ch);
      return;
    }
  if (argument[0] != '\0')
    {
      short x;
      char command[MAX_INPUT_LENGTH];
      if (argument[0] == '!')
	{
	  if (argument[1] == '?')
	    {
	      char big_buf[MAX_STRING_LENGTH * 2];
	      strcpy (big_buf, "Command History:\n\r\n\r");
	      for (x = MAX_HISTORY - 1; x >= 0; x--)
		if (ch->pcdata->history[x] != NULL)
		  sprintf (big_buf + strlen (big_buf),
			   "%2d.\t%s\n\r", x + 1, ch->pcdata->history[x]);
	      page_to_char (big_buf, ch);
	      return;
	    }
	  else if (argument[1] == '\0' || argument[1] == '!')
	    {
	      if (ch->pcdata->history[0] != NULL)
		{
		  strcpy (command, ch->pcdata->history[0]);
		  medit (ch, command);
		  return;
		}
	    }
	  else
	    {
	      short num;
	      num = atoi (argument + 1);
	      if (num > 0 && num < 21)
		{
		  if (ch->pcdata->history[num - 1] != NULL)
		    {
		      strcpy (command, ch->pcdata->history[num - 1]);
		      medit (ch, command);
		    }
		  else
		    send_to_char ("No such history entry.\n\r", ch);
		  return;
		}
	      else
		{
		  send_to_char ("Huh?\n\r", ch);
		  return;
		}
	    }
	}
      else
	{
	  if (ch->pcdata->history[19] != NULL)
	    free_string (ch->pcdata->history[19]);
	  for (x = 18; x >= 0; x--)
	    if (ch->pcdata->history[x] != NULL)
	      ch->pcdata->history[x + 1] = ch->pcdata->history[x];
	  ch->pcdata->history[0] = str_dup (argument);
	}
    }
  argument = one_argument (argument, command);
  if (IS_NPC (ch))
    return;
  EDIT_MOB (ch, pMob);
  pArea = pMob->area;
  if (!is_builder (ch, pArea))
    send_to_char ("You are not this area's creator.\n\r", ch);
  if (command[0] == '\0')
    {
      medit_show (ch, argument);
      return;
    }
  if (!str_cmp (command, "done"))
    {
      edit_done (ch);
      return;
    }
  if (!is_builder (ch, pArea))
    {
      interpret (ch, arg);
      return;
    }

  /* Search Table and Dispatch Command. */
  for (cmd = 0; medit_table[cmd].name[0] != '\0'; cmd++)
    {
      if (!str_prefix (command, medit_table[cmd].name))
	{
	  if ((*medit_table[cmd].olc_fun) (ch, argument))
	    SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
    }

  /* Default to Standard Interpreter. */
  interpret (ch, arg);
  return;
}

void
otedit (CHAR_DATA * ch, char *argument)
{
  OBJ_TRIG_DATA *pObjTrig;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_STRING_LENGTH];
  int cmd;

  smash_tilde (argument);
  strcpy (arg, argument);

  if (IS_NPC (ch))
    return;
  
  if (argument[0] != '\0')
    {
      short x;
      char command[MAX_INPUT_LENGTH];

      if (argument[0] == '!')
	{
	  if (argument[1] == '?')
	    {
	      char big_buf[MAX_STRING_LENGTH * 2];

	      strcpy (big_buf, "Command History:\n\r\n\r");

	      for (x = MAX_HISTORY - 1; x >= 0; x--)
		if (ch->pcdata->history[x] != NULL)
		  sprintf (big_buf + strlen (big_buf),
			   "%2d.\t%s\n\r", x + 1, ch->pcdata->history[x]);

	      page_to_char (big_buf, ch);
	      return;
	    }
	  else if (argument[1] == '\0' || argument[1] == '!')
	    {
	      if (ch->pcdata->history[0] != NULL)
		{
		  strcpy (command, ch->pcdata->history[0]);
		  otedit (ch, command);
		  return;
		}
	    }
	  else
	    {
	      short num;

	      num = atoi (argument + 1);

	      if (num > 0 && num < 21)
		{
		  if (ch->pcdata->history[num - 1] != NULL)
		    {
		      strcpy (command, ch->pcdata->history[num - 1]);
		      medit (ch, command);
		    }
		  else
		    send_to_char ("No such history entry.\n\r", ch);
		  return;
		}
	      else
		{
		  send_to_char ("Huh?\n\r", ch);
		  return;
		}
	    }
	}
      else
	{
	  if (ch->pcdata->history[19] != NULL)
	    free_string (ch->pcdata->history[19]);

	  for (x = 18; x >= 0; x--)
	    if (ch->pcdata->history[x] != NULL)
	      ch->pcdata->history[x + 1] = ch->pcdata->history[x];

	  ch->pcdata->history[0] = str_dup (argument);
	}
    }

  argument = one_argument (argument, command);

  if (IS_NPC (ch))
    return;

  EDIT_OBJ_TRIG (ch, pObjTrig);


  if (command[0] == '\0')
    {
      otedit_show (ch, argument);
      return;
    }

  if (!str_cmp (command, "done"))
    {
      edit_done (ch);
      return;
    }


  /* Search Table and Dispatch Command. */
  for (cmd = 0; otedit_table[cmd].name[0] != '\0'; cmd++)
  {
      if (!str_prefix (command, otedit_table[cmd].name))
	  {
	   char buf[MAX_INPUT_LENGTH];
	   strcpy(buf,argument);
	    if ((*otedit_table[cmd].olc_fun) (ch, argument))
	    {
  	      AREA_DATA *pArea;
	      pArea = get_vnum_area(pObjTrig->vnum);
	      if (pArea != NULL)
 	        SET_BIT (pArea->area_flags, AREA_CHANGED);
	      return;
	    }
	  }
  }

  /* Default to Standard Interpreter. */
  interpret (ch, arg);
  return;
}
const struct editor_cmd_type editor_table[] = {
/*  {   command         function        }, */

  {"area", do_aedit},
  {"room", do_redit},
  {"object", do_oedit},
  {"mobile", do_medit},
  {"vocabulary", do_vedit},
  {"objtrigger", do_otedit},

  {"", 0,}
};

/* Entry point for all editors. */
void do_olc (CHAR_DATA * ch, char *argument)
{
  char command[MAX_INPUT_LENGTH];
  int cmd;
  if (!IS_BUILDER (ch))
    {
      send_to_char ("You do not have authorization to do that.\n\r", ch);
      return;
    }
  argument = one_argument (argument, command);
  if (command[0] == '\0')
    {
      do_help (ch, "olc");
      return;
    }

  /* Search Table and Dispatch Command. */
  for (cmd = 0; editor_table[cmd].name[0] != '\0'; cmd++)
    {
      if (!str_prefix (command, editor_table[cmd].name))
	{
	  (*editor_table[cmd].do_fun) (ch, argument);
	  return;
	}
    }

  /* Invalid command, send help. */
  do_help (ch, "olc");
  return;
}


/* Entry point for editing area_data. */
void do_aedit (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_STRING_LENGTH];
  AREA_DATA *pArea;
  if (IS_NPC (ch))
    return;
  if (argument[0] == '\0')
    {
      pArea = ch->in_room->area;
      ch->desc->pEdit = (void *) pArea;
      ch->desc->editor = ED_AREA;
      sprintf (tcbuf, "Editing area '%s' [#%d].\n\r", pArea->name,
	       pArea->vnum);
      send_to_char (tcbuf, ch);
    }
  else if (is_number (argument))
    {
      if ((pArea = get_area_data (atoi (argument))) == NULL)
	{
	  send_to_char ("That area vnum does not exist.\n\r", ch);
	  return;
	}
      if (!is_builder (ch, pArea))
	{
	  send_to_char ("Sorry, you are not that area's creator.\n\r", ch);
	  return;
	}
      ch->desc->pEdit = (void *) pArea;
      ch->desc->editor = ED_AREA;
      sprintf (tcbuf, "Editing area '%s' [#%d].\n\r", pArea->name,
	       pArea->vnum);
      send_to_char (tcbuf, ch);
    }
  else
    {
      if (!str_cmp (argument, "create"))
	{
	  if (top_area >= INT_MAX)
	    {
	      send_to_char ("We're out of vnums for new areas.\n\r", ch);
	      return;
	    }
	  pArea = new_area ();
	  pArea->creator = str_dup (ch->name);
	  area_last->next = pArea;
	  area_last = pArea;	/* Thanks, Walker. */
	  ch->desc->pEdit = (void *) pArea;
	  ch->desc->editor = ED_AREA;
	  pArea->credits = NULL;
	  SET_BIT (pArea->area_flags, AREA_ADDED);
	  sprintf (tcbuf, "Area [#%d] Created.\n\r", pArea->vnum);
	  send_to_char (tcbuf, ch);
	}
      else
	{
	  do_help (ch, "edit");
	  return;
	}
    }
  return;
}


/* Entry point for editing room_index_data. */
void do_redit (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *pRoom;
  char arg1[MAX_STRING_LENGTH];
  int room_vnum;
  if (IS_NPC (ch))
    return;
  pRoom = ch->in_room;
  argument = one_argument (argument, arg1);
  if (is_number (arg1))
    {
      room_vnum = atoi (arg1);
      if ((pRoom = get_room_index (room_vnum)) == NULL)
	{
	  send_to_char ("No such room exists.\n\r", ch);
	  return;
	}
      if (!is_builder (ch, pRoom->area))
	{
	  send_to_char
	    ("Sorry, you are not that room's area creator.\n\r", ch);
	  return;
	}
      char_from_room (ch);
      char_to_room (ch, pRoom);
    }
  if (!str_cmp (arg1, "reset"))
    {
      reset_room (pRoom);
      send_to_char ("Room reset.\n\r", ch);
      return;
    }
  else if (!str_cmp (arg1, "create"))
    {
      if (argument[0] == '\0' || atoi (argument) == 0)
	{
	  send_to_char ("Syntax:  edit room create [vnum]\n\r", ch);
	  return;
	}
      if (!is_builder (ch, get_vnum_area (atoi (argument))))
	{
	  send_to_char ("Sorry, you are not that area's creator.\n\r", ch);
	  return;
	}
      if (redit_create(ch, argument)) {
	  char_from_room(ch);
	  char_to_room (ch, ch->desc->pEdit);
//	  char_to_room(ch, reinterpret_cast<ROOM_INDEX_DATA *>(ch->desc->pEdit));
	  SET_BIT (pRoom->area->area_flags, AREA_CHANGED);
	  pRoom = ch->in_room;
	}
      else
	{
	  send_to_char ("Room creation error.\n\r", ch);
	  return;
	}
    }
  if (!is_builder (ch, pRoom->area))
    {
      send_to_char ("Sorry, you are not that room's area creator.\n\r", ch);
      return;
    }
  ch->desc->editor = ED_ROOM;
  return;
}


/* Entry point for editing obj_index_data. */
void do_oedit (CHAR_DATA * ch, char *argument)
{
  OBJ_INDEX_DATA *pObj;
  AREA_DATA *pArea;
  char arg1[MAX_STRING_LENGTH];
  int value;
  if (IS_NPC (ch))
    return;
  argument = one_argument (argument, arg1);
  if (is_number (arg1))
    {
      value = atoi (arg1);
      if (!(pObj = get_obj_index (value)))
	{
	  send_to_char ("OEdit:  That vnum does not exist.\n\r", ch);
	  return;
	}
      if (!is_builder (ch, pObj->area))
	{
	  send_to_char
	    ("Sorry, you are not that object's area creator.\n\r", ch);
	  return;
	}
      ch->desc->pEdit = (void *) pObj;
      ch->desc->editor = ED_OBJECT;
      return;
    }
  else
    {
      if (!str_cmp (arg1, "create"))
	{
	  value = atoi (argument);
	  if (argument[0] == '\0' || value == 0)
	    {
	      send_to_char ("Syntax:  edit object create [vnum]\n\r", ch);
	      return;
	    }
	  pArea = get_vnum_area (value);
	  if (!is_builder (ch, pArea))
	    {
	      send_to_char
		("Sorry, you are not that object's area creator.\n\r", ch);
	      return;
	    }
	  if (oedit_create (ch, argument))
	    {
	      SET_BIT (pArea->area_flags, AREA_CHANGED);
	      ch->desc->editor = ED_OBJECT;
	    }
	  return;
	}
    }
  send_to_char ("There is no default object to edit.\n\r", ch);
  return;
}


/* Entry point for editing mob_index_data. */
void do_medit (CHAR_DATA * ch, char *argument)
{
  MOB_INDEX_DATA *pMob;
  AREA_DATA *pArea;
  int value;
  char arg1[MAX_STRING_LENGTH];
  argument = one_argument (argument, arg1);
  if (is_number (arg1))
    {
      value = atoi (arg1);
      if (!(pMob = get_mob_index (value)))
	{
	  send_to_char ("MEdit:  That vnum does not exist.\n\r", ch);
	  return;
	}
      if (!is_builder (ch, pMob->area))
	{
	  send_to_char
	    ("Sorry, you are not that mobile's area creator.\n\r", ch);
	  return;
	}
      ch->desc->pEdit = (void *) pMob;
      ch->desc->editor = ED_MOBILE;
      return;
    }
  else
    {
      if (!str_cmp (arg1, "create"))
	{
	  value = atoi (argument);
	  if (arg1[0] == '\0' || value == 0)
	    {
	      send_to_char ("Syntax:  edit mobile create [vnum]\n\r", ch);
	      return;
	    }
	  pArea = get_vnum_area (value);
	  if (!is_builder (ch, pArea))
	    {
	      send_to_char
		("Sorry, you are not that mobile's area creator.\n\r", ch);
	      return;
	    }
	  if (medit_create (ch, argument))
	    {
	      SET_BIT (pArea->area_flags, AREA_CHANGED);
	      ch->desc->editor = ED_MOBILE;
	    }
	  return;
	}
    }
  send_to_char ("There is no default mobile to edit.\n\r", ch);
  return;
}

// Adeon 7/19/03 -- Entry point for editing object triggers
void do_otedit(CHAR_DATA *ch, char *argument)
{
	OBJ_TRIG_DATA *pObjTrig;
	int value;
	char arg1[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg1);

	if (is_number (arg1))
    {
      value = atoi (arg1);
      if (!(pObjTrig = get_obj_trig (value)))
	  {
	    send_to_char ("That Object Triger Does Not Exist.\n\r", ch);
	    return;
	  }

	  ch->desc->pEdit = (void *) pObjTrig;
      ch->desc->editor = ED_OBJTRIG;
      return;
	}
	else
    {
      if (!str_cmp (arg1, "create"))
	  {
		value = atoi (argument);
		if (arg1[0] == '\0' || value == 0)
		{
	      send_to_char ("Syntax:  edit objtrigger create [vnum]\n\r", ch);
	      return;
	    }

		if (otedit_create (ch, argument))
	    {
	      ch->desc->editor = ED_OBJTRIG;
	    }
		return;
	  }
			
	}

	send_to_char("You must specify a vnum or create a new trigger.\n\r", ch);
	return;
}



void do_aresets (CHAR_DATA * ch, char *argument)
{
  AREA_DATA *pArea;
  RESET_DATA *pReset;
  char tcbuf[MAX_STRING_LENGTH];
  if (!IS_BUILDER (ch))
    {
      send_to_char ("You do not have authorization to do that.\n\r", ch);
      return;
    }
  pArea = ch->in_room->area;
  for (pReset = pArea->reset_first; pReset; pReset = pReset->next)
    {
      sprintf (tcbuf, "> [%c] [%d %d %d %d]\n\r", pReset->command,
	       pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4);
      send_to_char (tcbuf, ch);
    }
}
int
insert_mob_reset (ROOM_INDEX_DATA * room, RESET_DATA * pReset, int mob_vnum,
		  int room_vnum)
{
  RESET_DATA *fReset;
  AREA_DATA *this_area;
  bool pFound = FALSE;
  this_area = room->area;
  for (fReset = this_area->reset_first; fReset; fReset = fReset->next)
    {
      if (fReset->command == 'M' && fReset->arg1 == mob_vnum
	  && fReset->arg3 == room_vnum)
	{
	  pFound = TRUE;
	  break;
	}
    }
  if (pFound)
    {
      pReset->next = fReset->next;
      fReset->next = pReset;
      return (1);
    }
  return (0);
}

int
insert_obj_reset (ROOM_INDEX_DATA * room, RESET_DATA * pReset,
		  int obj_vnum, int room_vnum)
{
  RESET_DATA *fReset;
  AREA_DATA *this_area;
  bool pFound = FALSE;
  this_area = room->area;
  for (fReset = this_area->reset_first; fReset; fReset = fReset->next)
    {
      if (fReset->command == 'O' && fReset->arg1 == obj_vnum
	  && fReset->arg3 == room_vnum)
	{
	  pFound = TRUE;
	  break;
	}
    }
  if (pFound)
    {
      pReset->next = fReset->next;
      fReset->next = pReset;
      return (1);
    }
  return (0);
}

void add_reset (ROOM_INDEX_DATA * room, RESET_DATA * pReset, int index)
{
  AREA_DATA *this_area;
  this_area = room->area;
  if (this_area->reset_first == NULL)
    this_area->reset_first = pReset;
  if (this_area->reset_last != NULL)
    this_area->reset_last->next = pReset;
  this_area->reset_last = pReset;
  pReset->next = NULL;

  /*  if ( !room->reset_first )
     {
     room->reset_first  = pReset;
     room->reset_last   = pReset;
     pReset->next               = NULL;
     return;
     }

     index--;

     if ( index == 0 )  
     {
     pReset->next = room->reset_first;
     room->reset_first = pReset;
     return;
     }

     for ( reset = room->reset_first; reset->next; reset = reset->next )
     {
     if ( ++iReset == index )
     break;
     }

     pReset->next       = reset->next;
     reset->next                = pReset;
     if ( !pReset->next )
     room->reset_last = pReset;
     return;   */
}

void do_resets (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char arg4[MAX_INPUT_LENGTH];
  char arg5[MAX_INPUT_LENGTH];
  char arg6[MAX_INPUT_LENGTH];
  RESET_DATA *pReset = NULL, *prev = NULL, *aReset;
  int iReset = 0, pFound = FALSE;
  int last_room = 0;
  if (!IS_BUILDER (ch))
    {
      send_to_char ("You do not have authorization to do that.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);
  argument = one_argument (argument, arg4);
  argument = one_argument (argument, arg5);
  argument = one_argument (argument, arg6);

  /*
   * Display resets in current room.
   * -------------------------------
   */
  if (arg1[0] == '\0')
    {
      send_to_char ("Resets: M = mobile, R = room, O = object, "
		    "P = pet, S = shopkeeper\n\r", ch);
      display_resets (ch);
    }
  if (!str_cmp (arg1, "area"))
    {
      reset_area (ch->in_room->area);
      send_to_char ("Area Reset.\n\r", ch);
      return;
    }
  if (is_number (arg1))
    {
      if (!str_cmp (arg2, "delete"))
	{
	  int insert_loc = atoi (arg1);
	  if (!ch->in_room->area->reset_first)
	    {
	      send_to_char ("No resets in this area.\n\r", ch);
	      return;
	    }
	  for (pReset = ch->in_room->area->reset_first; pReset;
	       pReset = pReset->next)
	    {
	      switch (pReset->command)
		{
		case 'P':
		case 'G':
		case 'E':
		  if (last_room == ch->in_room->vnum)
		    {
		      iReset++;
		    }
		  break;
		case 'M':
		  if (pReset->arg3 == ch->in_room->vnum)
		    {
		      iReset++;
		    }
		  last_room = pReset->arg3;
		  break;
		case 'O':
		  if (pReset->arg3 == ch->in_room->vnum)
		    {
		      iReset++;
		    }
		  last_room = pReset->arg3;
		  break;
		case 'R':
		case 'D':
		  if (pReset->arg1 == ch->in_room->vnum)
		    {
		      iReset++;
		    }
		  break;
		}
	      if (iReset == insert_loc)
		{
		  pFound = TRUE;
		  if (pReset->command == 'M')
		    while (pReset->next != NULL &&
			   (pReset->next->command == 'E'
			    || pReset->next->command == 'G'))
		      {
			aReset = pReset->next;
			pReset->next = aReset->next;
			if (prev)
			  {
			    if (aReset == ch->in_room->area->reset_last)
			      ch->in_room->area->reset_last = pReset;
			  }
			free_reset_data (aReset);
			send_to_char
			  ("Deleting G or E reset from mob...\n\r", ch);
		      }
		  else if (pReset->command == 'O')
		    while (pReset->next != NULL
			   && pReset->next->command == 'P')
		      {
			aReset = pReset->next;
			pReset->next = aReset->next;
			if (prev)
			  {
			    if (aReset == ch->in_room->area->reset_last)
			      ch->in_room->area->reset_last = pReset;
			  }
			free_reset_data (aReset);
			send_to_char ("Deleting P reset from obj...\n\r", ch);
		      }
		  if (prev)
		    {
		      if (pReset == ch->in_room->area->reset_last)
			ch->in_room->area->reset_last = prev;
		      prev->next = prev->next->next;
		    }
		  else
		    ch->in_room->area->reset_first =
		      ch->in_room->area->reset_first->next;
		  free_reset_data (pReset);
		  send_to_char ("Reset deleted.\n\r", ch);
		  prev = pReset;
		  break;
		}
	      prev = pReset;
	    }
	  if (!pFound)
	    send_to_char ("Reset not found.\n\r", ch);
	}
      else if ((!str_cmp (arg2, "mob") || !str_cmp (arg2, "obj"))
	       && is_number (arg3))
	{
	  if (!str_cmp (arg2, "mob"))
	    {
	      if (get_mob_index (atoi (arg3)) == NULL)
		{
		  send_to_char ("No such mob currently exists.\n\r", ch);
		  return;
		}
	      pReset = new_reset_data ();
	      pReset->command = 'M';
	      pReset->arg1 = atoi (arg3);
	      pReset->arg2 = is_number (arg4) ? atoi (arg4) : 1;	/* Max # */
	      pReset->arg3 = ch->in_room->vnum;
	      pReset->arg4 = is_number (arg5) ? atoi (arg5) : 1;	/* Max # */
	    }
	  else if (!str_cmp (arg2, "obj"))
	    {
	      if (get_obj_index (atoi (arg3)) == NULL)
		{
		  send_to_char ("No such obj currently exists.\n\r", ch);
		  return;
		}
	      if (!str_cmp(arg4,""))
	      {
		      send_to_char ("Mistype.  Did you mean reset 1 obj vnum \"ROOM\"?\n\r",ch);
		      return;
	      }
	      pReset = new_reset_data ();
	      pReset->arg1 = atoi (arg3);
	      if (!str_prefix (arg4, "inside"))
		{
		  if (!check_last_obj_reset(ch->in_room))
		  {
  		    send_to_char("To properly add this type of reset, you need to make sure the container you want the item\n\r",ch);
		    send_to_char("to load in was the MOST RECENT OBJ RESET ADDED.  You may need to delete the container's reset\n\r",ch);
		    send_to_char("and readd it.\n\r",ch);
		    return;
		  }
		  pReset->command = 'P';
		  pReset->arg2 = is_number (arg6) ? atoi (arg6) : 1;
		  pReset->arg3 = atoi (arg5);
		  pReset->arg4 = pReset->arg2;
		}
	      else if (!str_cmp (arg4, "room"))
		{
		  pReset->command = 'O';
		  pReset->arg2 = 0;
		  pReset->arg3 = ch->in_room->vnum;
		  pReset->arg4 = is_number (arg5) ? atoi (arg5) : -1;
		}
	      else
		{
		  if (!check_last_mob_reset(ch->in_room))
		    {
		      send_to_char("To properly add this type of reset, you need to make sure the mob you want the item\n\r",ch);
		      send_to_char("to load on was the MOST RECENT MOB RESET ADDED.  You may need to delete the mob's reset\n\r",ch);
		      send_to_char("you want and readd it.\n\r",ch);
		      return;
		    }
		  pReset->arg2 = is_number (arg5) ? atoi (arg5) : -1;
		  if (flag_value (wear_loc_flags, arg4) == NO_FLAG)
		    {
		      send_to_char ("Resets: '? wear-loc'\n\r", ch);
		      free_reset_data (pReset);
		      return;
		    }
		  pReset->arg3 = flag_value (wear_loc_flags, arg4);
		  if (pReset->arg3 == WEAR_NONE)
		    pReset->command = 'G';

		  else
		    pReset->command = 'E';
		}
	    }
	  add_reset (ch->in_room, pReset, atoi (arg1));
	  send_to_char ("Reset added.\n\r", ch);
	}
      else
	{
	  send_to_char
	    ("Syntax: RESET <number> OBJ <vnum> <wear_loc>\n\r", ch);
	  send_to_char
	    ("        RESET <number> OBJ <vnum> in <vnum>\n\r", ch);
	  send_to_char ("        RESET <number> OBJ <vnum> room\n\r", ch);
	  send_to_char
	    ("        RESET <number> MOB <vnum> [<max [world]#>] [<max [room]#>]\n\r",
	     ch);
	  send_to_char ("        RESET <number> DELETE\n\r", ch);
	}
    }
  return;
}

void do_rlist (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *pRoomIndex;
  AREA_DATA *pArea;
  char arg[MAX_STRING_LENGTH];
  bool owner = FALSE;
  int iHash;
  char buf[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH * 20];
  if (!IS_BUILDER (ch))
    {
      send_to_char ("You do not have authorization to do that.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg);
  strcpy (tcbuf, "Room Listing:\n\r");
  pArea = ch->in_room->area;
  if (!str_cmp (pArea->filename, "ocean.are"))
    {
      send_to_char ("I wouldn't do that if I were you.\n\r", ch);
      return;
    }
  if (arg[0] != '\0' && !is_number (arg))
    owner = TRUE;
  if (arg[0] == '\0')
    pArea = ch->in_room->area;

  else if (is_number (arg))
    {
      if ((pArea = get_area_data (atoi (arg))) == NULL)
	{
	  send_to_char ("That area vnum does not exist.\n\r", ch);
	  return;
	}
    }
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    for (pRoomIndex = room_index_hash[iHash]; pRoomIndex;
	 pRoomIndex = pRoomIndex->next)
      {
	if (!owner)
	  if (pRoomIndex->area == pArea)
	    {
	      sprintf (buf, "[%5d] %s\n\r", pRoomIndex->vnum,
		       pRoomIndex->name);
	      strcat (tcbuf, buf);
	    }
	if (owner)
	  if (pRoomIndex->area == pArea && !str_cmp (pRoomIndex->owner, arg))
	    {
	      sprintf (buf, "[%5d] %s\n\r", pRoomIndex->vnum,
		       pRoomIndex->name);
	      strcat (tcbuf, buf);
	    }
      }
  page_to_char (tcbuf, ch);
}

void do_mlist (CHAR_DATA * ch, char *argument)
{
  MOB_INDEX_DATA *pMobIndex;
  AREA_DATA *pArea;
  int vnum;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  char tcbuf[MAX_STRING_LENGTH * 15];
  bool fAll, found=FALSE;
  if (!IS_BUILDER (ch))
    {
      send_to_char ("You do not have authorization to do that.\n\r", ch);
      return;
    }
  one_argument (argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char ("Syntax:  mlist <all/name>\n\r", ch);
    return;
  }
  strcpy (tcbuf, "Mob Listing:\n\r");
  fAll = !str_cmp (arg, "all");
  pArea = ch->in_room->area;
//  if (argument[0] == '\0')
  //pArea = ch->in_room->area;

/*  else if (is_number (argument))
    if ((pArea = get_area_data (atoi (argument))) == NULL)
      {
	send_to_char ("That area vnum does not exist.\n\r", ch);
	return;
      }*/
//  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    for (vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++)
    {
      if ((pMobIndex = get_mob_index (vnum)) != NULL)
      {
 	  if (fAll || is_name (arg, pMobIndex->player_name))
	  {
  	    found = TRUE;
	    sprintf (buf, "[%5d] [Name(s): %s] [Short: %s]\n\r",
		   pMobIndex->vnum, pMobIndex->player_name,
		   pMobIndex->short_descr);
	    strcat (tcbuf, buf);
	  }
      }
    }
  page_to_char (tcbuf, ch);
  if (!found)
  {
    send_to_char ("Mob(s) not found in this area.\n\r", ch);
  }
}


/*****************************************************************************
 Name:		do_alist
 Purpose:	Normal command to list areas and display area information.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_alist (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char name[MAX_STRING_LENGTH];
  char result[MAX_STRING_LENGTH * 4];	/* May need tweaking. */
  AREA_DATA *pArea;
  int i;
  int len;
  int pad;
  if (!IS_BUILDER (ch))
    {
      send_to_char ("You do not have authorization to do that.\n\r", ch);
      return;
    }
  sprintf (result, "[%3s] [%-30s] (%-6s-%6s) [%-10s]\n\r", "Num",
	   "Area Name", "lvnum", "uvnum", "Filename");
  for (pArea = area_first; pArea; pArea = pArea->next)
    {

      // Akamai -- BUG: The color in area names cause the area list
      // to be awkwardly formatted.
      // This fix pads spaces on the end of the area name to make up
      // for the loss of color characters when printing.
      sprintf (name, "%s", pArea->name);
      len = strlen (name);
      pad = len + (32 - color_strlen (name));
      if (len < pad)
	{
	  for (i = len; i < pad; i++)
	    name[i] = ' ';
	  name[i] = '\0';
	}
      sprintf (buf, "[%3d] %-32s`` (%-6d-%6d) %-18s\n\r", pArea->vnum,
	       name, pArea->lvnum, pArea->uvnum, pArea->filename);
      strcat (result, buf);
    }
  page_to_char (result, ch);
  return;
}

void do_vedit (CHAR_DATA * ch, char *argument)
{
  if (IS_NPC (ch))
    return;
  send_to_char ("Vocabulary file editor.\n\r\n\r", ch);
  send_to_char ("Commands...\n\r", ch);
  send_to_char
    ("create <filename>    ...  Create a new vocabulary file.\n\r", ch);
  send_to_char
    ("delete <filename>    ...  Delete a vocabulary file you own.\n\r", ch);
  send_to_char
    ("edit <filename>      ...  Edit a vocabulary file you own.\n\r", ch);
  send_to_char
    ("list                 ...  List existing vocabulary files.\n\r", ch);
  send_to_char
    ("exit                 ...  Exit vocabulary file editor.\n\r", ch);
  ch->desc->editor = ED_VOCAB_MAIN;
  return;
}

void vocab_main (CHAR_DATA * ch, char *argument)
{
  char cmd[MAX_INPUT_LENGTH];
  char filename[MAX_INPUT_LENGTH];
  char fullpath[MAX_STRING_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  struct stat statbuf;
  FILE *fp;
  argument = one_argument (argument, cmd);
  if (!str_cmp (cmd, "exit"))
    {
      if (ch->pcdata->vocab_file)
	free_string (ch->pcdata->vocab_file);
      ch->pcdata->vocab_file = NULL;
      edit_done (ch);
      return;
    }
  else if (!str_cmp (cmd, "create"))
    {
      one_argument (argument, filename);
      if (filename[0] == '\0')
	{
	  send_to_char ("Syntax: create <filename>\n\r", ch);
	  return;
	}
      sprintf (fullpath, "mobvoc/%s", filename);
      if (stat (fullpath, &statbuf) != -1)
	{
	  send_to_char ("That file already exists!\n\r", ch);
	  return;
	}
      send_to_char ("Creating file...\n\r", ch);
      fp = fopen (fullpath, "w");
      fprintf (fp, "%s\n", ch->name);
      fclose (fp);
      ch->pcdata->vocab_file = str_dup (fullpath);
      ch->desc->editor = ED_VOCAB_EDIT;
      sprintf (tcbuf, "Editing vocabulary file: %s\n\r\n\r",
	       ch->pcdata->vocab_file);
      send_to_char (tcbuf, ch);
      send_to_char
	("list            ...  List currently defined vocabulary entries.\n\r",
	 ch);
      send_to_char
	("delete <line #> ...  Delete vocabulary entry by number.\n\r", ch);
      send_to_char
	("add             ...  Add a new vocabulary entry.\n\r", ch);
      send_to_char ("exit            ...  Exit vocabulary editor.\n\r", ch);
      return;
    }
  else if (!str_cmp (cmd, "edit"))
    {
      char *cptr;
      one_argument (argument, filename);
      if (filename[0] == '\0')
	{
	  send_to_char ("Syntax: edit <filename>\n\r", ch);
	  return;
	}
      sprintf (fullpath, "mobvoc/%s", filename);
      if (stat (fullpath, &statbuf) == -1)
	{
	  send_to_char ("That file doesn't exist.\n\r", ch);
	  return;
	}
      if (ch->level < 99)
	{
	  if ((fp = fopen (fullpath, "r")) == NULL)
	    {
	      send_to_char ("Error reading file.\n\r", ch);
	      return;
	    }
	  fgets (tcbuf, MAX_STRING_LENGTH, fp);
	  fclose (fp);
	  if ((cptr = strchr (tcbuf, '\n')) != NULL)
	    *cptr = '\0';
	  if (str_cmp (tcbuf, ch->name))
	    {
	      send_to_char ("You don't own that vocabulary file!\n\r", ch);
	      return;
	    }
	}
      send_to_char ("Editing file...\n\r", ch);
      ch->pcdata->vocab_file = str_dup (fullpath);
      ch->desc->editor = ED_VOCAB_EDIT;
      sprintf (tcbuf, "Editing vocabulary file: %s\n\r\n\r",
	       ch->pcdata->vocab_file);
      send_to_char (tcbuf, ch);
      send_to_char
	("list            ...  List currently defined vocabulary entries.\n\r",
	 ch);
      send_to_char
	("delete <line #> ...  Delete vocabulary entry by number.\n\r", ch);
      send_to_char
	("add             ...  Add a new vocabulary entry.\n\r", ch);
      send_to_char ("exit            ...  Exit vocabulary editor.\n\r", ch);
      return;
    }
  else if (!str_cmp (cmd, "delete"))
    {
      char *cptr;
      one_argument (argument, filename);
      if (filename[0] == '\0')
	{
	  send_to_char ("Syntax: delete <filename>\n\r", ch);
	  return;
	}
      sprintf (fullpath, "mobvoc/%s", filename);
      if (stat (fullpath, &statbuf) == -1)
	{
	  send_to_char ("That file doesn't exist.\n\r", ch);
	  return;
	}
      if (ch->level < 99)
	{
	  if ((fp = fopen (fullpath, "r")) == NULL)
	    {
	      send_to_char ("Error reading file.\n\r", ch);
	      return;
	    }
	  fgets (tcbuf, MAX_STRING_LENGTH, fp);
	  fclose (fp);
	  if ((cptr = strchr (tcbuf, '\n')) != NULL)
	    *cptr = '\0';
	  if (str_cmp (tcbuf, ch->name))
	    {
	      send_to_char ("You don't own that vocabulary file!\n\r", ch);
	      return;
	    }
	}
      send_to_char ("Deleting file...\n\r", ch);
      unlink (fullpath);
      return;
    }
  else if (!str_cmp (cmd, "list"))
    {
      DIR *dire;
      char tcbuf[MAX_STRING_LENGTH];
      struct dirent *dent;
      send_to_char ("Existing vocabulary files:\n\r", ch);
      if ((dire = opendir ("mobvoc/")) != NULL)
	{
	  int count = 0;
	  tcbuf[0] = '\0';
	  while ((dent = readdir (dire)) != NULL)
	    {
	      if (!strcmp (dent->d_name, ".") || !strcmp (dent->d_name, ".."))
		continue;
	      sprintf (tcbuf + strlen (tcbuf), "%-19s ", dent->d_name);
	      count++;
	      if (count % 4 == 0)
		{
		  send_to_char (tcbuf, ch);
		  tcbuf[0] = '\0';
		}
	    }
	  send_to_char (tcbuf, ch);
	  send_to_char ("\n\r", ch);
	  closedir (dire);
	}
      return;
    }
  send_to_char ("Vocabulary file editor.\n\r\n\r", ch);
  send_to_char ("Commands...\n\r", ch);
  send_to_char
    ("create <filename>    ...  Create a new vocabulary file.\n\r", ch);
  send_to_char
    ("delete <filename>    ...  Delete a vocabulary file you own.\n\r", ch);
  send_to_char
    ("edit <filename>      ...  Edit a vocabulary file you own.\n\r", ch);
  send_to_char
    ("list                 ...  List existing vocabulary files.\n\r", ch);
  send_to_char
    ("exit                 ...  Exit vocabulary file editor.\n\r", ch);
}

void vocab_edit (CHAR_DATA * ch, char *argument)
{
  char cmd[MAX_INPUT_LENGTH];
  char tcbuf[MAX_INPUT_LENGTH];
  argument = one_argument (argument, cmd);
  if (!str_cmp (cmd, "exit"))
    {
      if (ch->pcdata->vocab_file)
	free_string (ch->pcdata->vocab_file);
      ch->pcdata->vocab_file = NULL;
      edit_done (ch);
      return;
    }
  else if (!str_cmp (cmd, "list"))
    {
      FILE *fp;
      char tcbuf[MAX_STRING_LENGTH];
      char buf[MAX_STRING_LENGTH];
      int count;
      if ((fp = fopen (ch->pcdata->vocab_file, "r")) == NULL)
	{
	  send_to_char ("Error opening file.\n\r", ch);
	  return;
	}

      /* Dont show creator line */
      fgets (tcbuf, MAX_STRING_LENGTH, fp);
      count = 1;
      while (fgets (tcbuf, MAX_STRING_LENGTH, fp))
	if (tcbuf[0] != '\0' && tcbuf[0] != '\n')
	  {
	    sprintf (buf, "%2d] %s\r", count, tcbuf);
	    send_to_char (buf, ch);
	    count++;
	  }
      fclose (fp);
      return;
    }
  else if (!str_cmp (cmd, "add"))
    {
      send_to_char ("Enter trigger word list, seperated by & or |.\n\r", ch);
      ch->desc->editor = ED_VOCAB_ADD_WORDS;
      return;
    }
  else if (!str_cmp (cmd, "delete"))
    {
      FILE *fp, *tmpfp;
      char tcbuf[MAX_STRING_LENGTH];
      char tmpfile[1024];
      char buf[MAX_STRING_LENGTH];
      int count, del_line;
      bool found = FALSE;
      one_argument (argument, buf);
      del_line = atoi (buf);
      if (del_line < 1)
	{
	  send_to_char ("Syntax: delete <line number>\n\r", ch);
	  return;
	}
      sprintf (tmpfile, "mobvoc/voctmp.%d", number_range (0, 9999));
      if ((fp = fopen (ch->pcdata->vocab_file, "r")) == NULL)
	{
	  send_to_char ("Error opening file.\n\r", ch);
	  return;
	}
      if ((tmpfp = fopen (tmpfile, "w")) == NULL)
	{
	  send_to_char ("Error opening file.\n\r", ch);
	  fclose (fp);
	  return;
	}

      /* Skip over creator line */
      fgets (tcbuf, MAX_STRING_LENGTH, fp);
      fprintf (tmpfp, "%s", tcbuf);
      count = 1;
      while (fgets (tcbuf, MAX_STRING_LENGTH, fp))
	{
	  if (tcbuf[0] == '\0' || tcbuf[0] == '\n')
	    continue;
	  if (count != del_line)
	    fprintf (tmpfp, "%s", tcbuf);

	  else
	    {
	      send_to_char ("Line deleted.\n\r", ch);
	      found = TRUE;
	    }
	  count++;
	}
      fclose (fp);
      fclose (tmpfp);
      unlink (ch->pcdata->vocab_file);
      rename (tmpfile, ch->pcdata->vocab_file);
      if (!found)
	send_to_char ("Line number not found.\n\r", ch);
      return;
    }
  sprintf (tcbuf, "Editing vocabulary file: %s\n\r\n\r",
	   ch->pcdata->vocab_file);
  send_to_char (tcbuf, ch);
  send_to_char
    ("list            ...  List currently defined vocabulary entries.\n\r",
     ch);
  send_to_char
    ("delete <line #> ...  Delete vocabulary entry by number.\n\r", ch);
  send_to_char ("add             ...  Add a new vocabulary entry.\n\r", ch);
  send_to_char ("exit            ...  Exit vocabulary editor.\n\r", ch);
}

void vocab_add_words (CHAR_DATA * ch, char *argument)
{
  bool notspace = FALSE;
  char *cptr;
  cptr = argument;
  while (*cptr != '\0')
    {
      if (!isspace (*cptr))
	{
	  notspace = TRUE;
	  break;
	}
      cptr++;
    }
  if (!notspace)
    {
      send_to_char ("Addition cancelled.\n\r", ch);
      ch->desc->editor = ED_VOCAB_EDIT;
      return;
    }
  if (strchr (argument, ':') != NULL)
    {
      send_to_char ("Word list may not include the ':' character.\n\r", ch);
      send_to_char ("Enter trigger word list, seperated by & or |.\n\r", ch);
      return;
    }
  ch->desc->door_name = str_dup (argument);
  if (!strcmp ("INCLUDE", argument))
    {
      send_to_char ("Please enter the filename to include:\n\r", ch);
      ch->desc->editor = ED_VOCAB_ADD_INCLUDE;
    }
  else
    {
      send_to_char
	("Please enter mood(s) in which this vocabulary will be used:\n\r",
	 ch);
      send_to_char ("Possible values include: '3', '>1', '<6'\n\r", ch);
      ch->desc->editor = ED_VOCAB_ADD_MOOD;
    }
}
void vocab_add_mood (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_STRING_LENGTH];
  bool notspace = FALSE;
  char *cptr;
  cptr = argument;
  while (*cptr != '\0')
    {
      if (!isspace (*cptr))
	{
	  notspace = TRUE;
	  break;
	}
      cptr++;
    }
  if (!notspace)
    {
      send_to_char ("Addition cancelled.\n\r", ch);
      ch->desc->editor = ED_VOCAB_EDIT;
      return;
    }
  if (argument[0] == '>' || argument[0] == '<')
    cptr = &argument[1];

  else
    cptr = argument;
  if (!is_number (cptr) || !isdigit (*cptr) || atoi (cptr) < 0
      || atoi (cptr) > 8)
    {
      send_to_char ("Invalid mood-test value.\n\r", ch);
      send_to_char
	("Please enter mood(s) in which this vocabulary will be used:\n\r",
	 ch);
      send_to_char ("Possible values include: '3', '>1', '<6'\n\r", ch);
      return;
    }
  sprintf (tcbuf, "%s:%s", ch->desc->door_name, argument);
  free_string (ch->desc->door_name);
  ch->desc->door_name = str_dup (tcbuf);
  ch->desc->editor = ED_VOCAB_ADD_MOOD_OFFSET;
  send_to_char
    ("Please enter mood offset that a match on this entry should cause.\n\r",
     ch);
  send_to_char
    ("Possible values include: '+3' (increase mood by 3),\n\r", ch);
  send_to_char
    ("                         '-1' (decrease mood by 1),\n\r", ch);
  send_to_char
    ("                         '6'  (set mood directly to 6),\n\r", ch);
  send_to_char ("                         '-'  (no change in mood)\n\r", ch);
}

void vocab_add_include (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_STRING_LENGTH];
  bool notspace = FALSE;
  char *cptr;
  FILE *fp;
  cptr = argument;
  while (*cptr != '\0')
    {
      if (!isspace (*cptr))
	{
	  notspace = TRUE;
	  break;
	}
      cptr++;
    }
  if (!notspace)
    {
      send_to_char ("Addition cancelled.\n\r", ch);
      ch->desc->editor = ED_VOCAB_EDIT;
      return;
    }
  sprintf (tcbuf, "%s:%s", ch->desc->door_name, argument);
  free_string (ch->desc->door_name);
  ch->desc->door_name = str_dup (tcbuf);
  ch->desc->editor = ED_VOCAB_EDIT;
  if ((fp = fopen (ch->pcdata->vocab_file, "a")) == NULL)
    {
      send_to_char ("Error writing to file.\n\r", ch);
      return;
    }
  fprintf (fp, "%s\n", tcbuf);
  fclose (fp);
  send_to_char ("Vocabulary include added.\n\r", ch);
}

void vocab_add_mood_offset (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_STRING_LENGTH];
  bool notspace = FALSE;
  char *cptr;
  cptr = argument;
  while (*cptr != '\0')
    {
      if (!isspace (*cptr))
	{
	  notspace = TRUE;
	  break;
	}
      cptr++;
    }
  if (!notspace)
    {
      send_to_char ("Addition cancelled.\n\r", ch);
      ch->desc->editor = ED_VOCAB_EDIT;
      return;
    }
  if (strcmp (argument, "-") != 0)
    {
      if (argument[0] == '-' || argument[0] == '+')
	cptr = &argument[1];

      else
	cptr = argument;
      if (!is_number (cptr) || !isdigit (*cptr) || atoi (cptr) < 0
	  || atoi (cptr) > 8)
	{
	  send_to_char ("Invalid mood-offset value.\n\r", ch);
	  send_to_char
	    ("Please enter mood offset that a match on this entry should cause.\n\r",
	     ch);
	  send_to_char
	    ("Possible values include: '+3' (increase mood by 3),\n\r", ch);
	  send_to_char
	    ("                         '-1' (decrease mood by 1),\n\r", ch);
	  send_to_char
	    ("                         '6'  (set mood directly to 6),\n\r",
	     ch);
	  send_to_char
	    ("                         '-'  (no change in mood)\n\r", ch);
	  return;
	}
    }
  sprintf (tcbuf, "%s:%s", ch->desc->door_name, argument);
  free_string (ch->desc->door_name);
  ch->desc->door_name = str_dup (tcbuf);
  ch->desc->editor = ED_VOCAB_ADD_TEXT;
  send_to_char
    ("Please enter the sayings and commands that this entry should trigger:\n\r",
     ch);
}

void vocab_add_text (CHAR_DATA * ch, char *argument)
{
  char tcbuf[MAX_STRING_LENGTH];
  FILE *fp;
  bool notspace = FALSE;
  char *cptr;
  cptr = argument;
  while (*cptr != '\0')
    {
      if (!isspace (*cptr))
	{
	  notspace = TRUE;
	  break;
	}
      cptr++;
    }
  if (!notspace)
    {
      send_to_char ("Addition cancelled.\n\r", ch);
      ch->desc->editor = ED_VOCAB_EDIT;
      return;
    }
  sprintf (tcbuf, "%s:%s", ch->desc->door_name, argument);
  free_string (ch->desc->door_name);
  ch->desc->editor = ED_VOCAB_EDIT;
  if ((fp = fopen (ch->pcdata->vocab_file, "a")) == NULL)
    {
      send_to_char ("Error writing to file.\n\r", ch);
      return;
    }
  fprintf (fp, "%s\n", tcbuf);
  fclose (fp);
  send_to_char ("Vocabulary trigger added.\n\r", ch);
}
