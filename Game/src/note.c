#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"

//
// File name constants
//
#define NOTE_FILE         "notes.not"
#define IDEA_FILE         "ideas.not"
#define PENALTY_FILE      "penal.not"
#define NEWS_FILE         "news.not"
#define CHANGES_FILE      "chang.not"
#define LEGEND_FILE       "poetry.not"
#define OOCNOTE_FILE      "oocnote.not"
#define PROJECTS_FILE     "project.not"	//Adeon 7/18/03

#define EXPIRE_EXTENSION  "exp"

#define MIN_LEVEL_ALL  15

//
// Expiration Constants - expressed in seconds
//
// Expire notes every 28 days
#define NOTE_EXPIRE     (28 * 24 * 60 * 60)
// Expire ideas every 28 days
#define IDEA_EXPIRE     (28 * 24 * 60 * 60)
// Never expire penalty notes
#define PENALTY_EXPIRE   0
// Never expire changes
#define CHANGES_EXPIRE   0
// Expire news every 180 days (six months)
#define NEWS_EXPIRE     (180 * 24 * 60 * 60)
// poetry changed to 'Legend', legends never expire. - Avariel 9/13/12
#define LEGEND_EXPIRE   0
// Expire notes every 28 days
#define OOCNOTE_EXPIRE  (28 * 24 * 60 * 60)
//Never expire projects
#define PROJECTS_EXPIRE 0

/* globals from db.c for load_notes */
#if !defined(macintosh)
extern int _filbuf args ((FILE *));

#endif /*  */
extern FILE *fpArea;
extern char strArea[MAX_INPUT_LENGTH];

//12-29-03 Iblis - needed to echo when a note has been posted 
void do_immtalk args((CHAR_DATA * ch, char *argument));

/* local procedures */
void load_thread (char *name, NOTE_DATA ** list, int type, time_t free_time);
void parse_note (CHAR_DATA * ch, char *argument, int type);
bool hide_note (CHAR_DATA * ch, NOTE_DATA * pnote);
void backup_note (NOTE_DATA * pnote);
NOTE_DATA *note_list;
NOTE_DATA *idea_list;
NOTE_DATA *penalty_list;
NOTE_DATA *news_list;
NOTE_DATA *changes_list;

// Akamai 5/6/99 - adding note bases for legend/music and OOC notes
NOTE_DATA *legend_list;
NOTE_DATA *oocnote_list;

// Adeon 7/18/03 -- immortal projects, to cut out the idea spam
NOTE_DATA *projects_list;
int count_spool (CHAR_DATA * ch, NOTE_DATA * spool)
{
  int count = 0;
  NOTE_DATA *pnote;
  for (pnote = spool; pnote != NULL; pnote = pnote->next)
    //Iblis - 06/04/04 - Fixed so everyone's note numbers will be the same
    if (!hide_note (ch, pnote))
      count++;
  return count;
}

void do_unread (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int count;
  bool found = FALSE;
  if (IS_NPC (ch))
    return;
  if ((count = count_spool (ch, news_list)) > 0)
    {
      found = TRUE;
      sprintf (buf, "There %s `o%d`` new news article%s waiting.\n\r",
	       count > 1 ? "are" : "is", count, count > 1 ? "s" : "");
      send_to_char (buf, ch);
    }
  if ((count = count_spool (ch, changes_list)) > 0)
    {
      found = TRUE;
      sprintf (buf, "There %s `o%d`` change%s waiting to be read.\n\r",
	       count > 1 ? "are" : "is", count, count > 1 ? "s" : "");
      send_to_char (buf, ch);
    }
  // Akamai 5/6/99 - Addition of poem/music and ooc note bases
  if ((count = count_spool (ch, legend_list)) > 0)
    {
      found = TRUE;
      sprintf (buf, "`hYou have `g%d`h new legend%s to uncover.``\n\r", count,
	       count > 1 ? "s" : "");
      send_to_char (buf, ch);
    }
  if ((count = count_spool (ch, oocnote_list)) > 0)
    {
      found = TRUE;
      sprintf (buf, "You have `o%d`` new OOC note%s waiting.\n\r",
	       count, count > 1 ? "s" : "");
      send_to_char (buf, ch);
    }
  if ((count = count_spool (ch, note_list)) > 0)
    {
      found = TRUE;
      sprintf (buf, "You have `o%d`` new scroll%s waiting.\n\r", count,
	       count > 1 ? "s" : "");
      send_to_char (buf, ch);
    }
  if ((count = count_spool (ch, idea_list)) > 0)
    {
      found = TRUE;
      sprintf (buf, "You have `o%d`` unread idea%s to peruse.\n\r",
	       count, count > 1 ? "s" : "");
      send_to_char (buf, ch);
    }
  if (IS_TRUSTED (ch, DEMIGOD)
      && (count = count_spool (ch, penalty_list)) > 0)
    {
      found = TRUE;
      sprintf (buf, "`o%d`` %s been added.\n\r", count,
	       count > 1 ? "penalties have" : "penalty has");
      send_to_char (buf, ch);
    }
  if (IS_TRUSTED (ch, DEMIGOD)
      && (count = count_spool (ch, projects_list)) > 0)
    {
      found = TRUE;
      sprintf (buf, "`o%d`` %s been designated.\n\r", count,
	       count > 1 ? "projects have" : "project has");
      send_to_char (buf, ch);
    }
  if (!found)
    send_to_char ("You have no unread notes.\n\r", ch);
}

void do_note (CHAR_DATA * ch, char *argument)
{
  parse_note (ch, argument, NOTE_NOTE);
}

void do_idea (CHAR_DATA * ch, char *argument)
{
  parse_note (ch, argument, NOTE_IDEA);
}

void do_penalty (CHAR_DATA * ch, char *argument)
{
  parse_note (ch, argument, NOTE_PENALTY);
}

void do_news (CHAR_DATA * ch, char *argument)
{
  parse_note (ch, argument, NOTE_NEWS);
}

void do_changes (CHAR_DATA * ch, char *argument)
{
  parse_note (ch, argument, NOTE_CHANGES);
}

void do_legend (CHAR_DATA * ch, char *argument)
{
  parse_note (ch, argument, NOTE_LEGEND);
}

void do_oocnote (CHAR_DATA * ch, char *argument)
{
  parse_note (ch, argument, NOTE_OOCNOTE);
}

void do_projects (CHAR_DATA * ch, char *argument)
{
  parse_note (ch, argument, NOTE_PROJECTS);
}

void save_notes (int type)
{
  FILE *fp;
  char *name;
  NOTE_DATA *pnote;
  switch (type)
    {
    default:
      return;
    case NOTE_NOTE:
      name = NOTE_FILE;
      pnote = note_list;
      break;
    case NOTE_IDEA:
      name = IDEA_FILE;
      pnote = idea_list;
      break;
    case NOTE_PENALTY:
      name = PENALTY_FILE;
      pnote = penalty_list;
      break;
    case NOTE_NEWS:
      name = NEWS_FILE;
      pnote = news_list;
      break;
    case NOTE_CHANGES:
      name = CHANGES_FILE;
      pnote = changes_list;
      break;
    case NOTE_LEGEND:
      name = LEGEND_FILE;
      pnote = legend_list;
      break;
    case NOTE_OOCNOTE:
      name = OOCNOTE_FILE;
      pnote = oocnote_list;
      break;
    case NOTE_PROJECTS:
      name = PROJECTS_FILE;
      pnote = projects_list;
      break;
    }
  fclose (fpReserve);
  if ((fp = fopen (name, "w")) == NULL)
    {
      perror (name);
    }
  else
    {
      for (; pnote != NULL; pnote = pnote->next)
	{
	  fprintf (fp, "Sender  %s~\n", pnote->sender);
	  fprintf (fp, "Date    %s~\n", pnote->date);
	  fprintf (fp, "Stamp   %ld\n", pnote->date_stamp);
	  fprintf (fp, "To      %s~\n", pnote->to_list);
	  fprintf (fp, "Subject %s~\n", pnote->subject);
	  fprintf (fp, "Text\n%s~\n", pnote->text);
	}
      fclose (fp);
      fpReserve = fopen (NULL_FILE, "r");
      return;
    }
}
void load_notes (void)
{
  load_thread (NOTE_FILE, &note_list, NOTE_NOTE, NOTE_EXPIRE);
  load_thread (IDEA_FILE, &idea_list, NOTE_IDEA, IDEA_EXPIRE);
  load_thread (PENALTY_FILE, &penalty_list, NOTE_PENALTY, PENALTY_EXPIRE);
  load_thread (NEWS_FILE, &news_list, NOTE_NEWS, NEWS_EXPIRE);
  load_thread (CHANGES_FILE, &changes_list, NOTE_CHANGES, CHANGES_EXPIRE);
  load_thread (LEGEND_FILE, &legend_list, NOTE_LEGEND, LEGEND_EXPIRE);
  load_thread (OOCNOTE_FILE, &oocnote_list, NOTE_OOCNOTE, OOCNOTE_EXPIRE);
  load_thread (PROJECTS_FILE, &projects_list, NOTE_PROJECTS, PROJECTS_EXPIRE);
}

void load_thread (char *name, NOTE_DATA ** list, int type, time_t free_time)
{
  FILE *fp;
  NOTE_DATA *pnotelast;
  if ((fp = fopen (name, "r")) == NULL)
    return;
  pnotelast = NULL;
  for (;;)
    {
      NOTE_DATA *pnote;
      char letter;

      do
	{
	  letter = getc (fp);
	  if (feof (fp))
	    {
	      fclose (fp);
	      return;
	    }
	}
      while (isspace (letter));
      ungetc (letter, fp);
      pnote = alloc_perm (sizeof (*pnote));
//      pnote = reinterpret_cast<NOTE_DATA *>(alloc_perm(sizeof(*pnote)));
      if (str_cmp (fread_word (fp), "sender"))
	break;
      pnote->sender = fread_string (fp);
      if (str_cmp (fread_word (fp), "date"))
	break;
      pnote->date = fread_string (fp);
      if (str_cmp (fread_word (fp), "stamp"))
	break;
      pnote->date_stamp = fread_number (fp);
      if (str_cmp (fread_word (fp), "to"))
	break;
      pnote->to_list = fread_string (fp);
      if (str_cmp (fread_word (fp), "subject"))
	break;
      pnote->subject = fread_string (fp);
      if (str_cmp (fread_word (fp), "text"))
	break;
      pnote->text = fread_string (fp);

      // Akamai 6/10/99 - fix so that notes are automatically backedup
      // when they expire
      if (free_time && pnote->date_stamp < current_time - free_time)
	{
	  backup_note (pnote);
	  free_note (pnote);
	  continue;
	}
      pnote->type = type;
      if (*list == NULL)
	*list = pnote;

      else
	pnotelast->next = pnote;
      pnotelast = pnote;
    }

  // Akamai - looks like this is designed to identify which file
  // a failure occurs in - so I'll make it the note file's name
  strcpy (strArea, name);

  //  strcpy(strArea, NOTE_FILE);
  fpArea = fp;
  bug ("Load_notes: bad key word.", 0);
  exit (1);
  return;
}


// Akamai 6/10/99 - Add function to allow notes to be backed up to
// special backup note bases.
//
void backup_note (NOTE_DATA * pnote)
{
  FILE *fp;
  char name[MAX_INPUT_LENGTH];

  // set the backup name
  switch (pnote->type)
    {
    default:
      return;
    case NOTE_NOTE:
      sprintf (name, "%s.%s", NOTE_FILE, EXPIRE_EXTENSION);
      break;
    case NOTE_IDEA:
      sprintf (name, "%s.%s", IDEA_FILE, EXPIRE_EXTENSION);
      break;
    case NOTE_PENALTY:
      sprintf (name, "%s.%s", PENALTY_FILE, EXPIRE_EXTENSION);
      break;
    case NOTE_NEWS:
      sprintf (name, "%s.%s", NEWS_FILE, EXPIRE_EXTENSION);
      break;
    case NOTE_CHANGES:
      sprintf (name, "%s.%s", CHANGES_FILE, EXPIRE_EXTENSION);
      break;
    case NOTE_LEGEND:
      sprintf (name, "%s.%s", LEGEND_FILE, EXPIRE_EXTENSION);
      break;
    case NOTE_OOCNOTE:
      sprintf (name, "%s.%s", OOCNOTE_FILE, EXPIRE_EXTENSION);
      break;
    case NOTE_PROJECTS:
      sprintf (name, "%s.%s", PROJECTS_FILE, EXPIRE_EXTENSION);
      break;
    }

  // open a reserved file pointer, for appending
  fclose (fpReserve);
  if ((fp = fopen (name, "a")) == NULL)
    {
      perror (name);
    }
  else
    {
      fprintf (fp, "Sender  %s~\n", pnote->sender);
      fprintf (fp, "Date    %s~\n", pnote->date);
      fprintf (fp, "Stamp   %ld\n", pnote->date_stamp);
      fprintf (fp, "To      %s~\n", pnote->to_list);
      fprintf (fp, "Subject %s~\n", pnote->subject);
      fprintf (fp, "Text\n%s~\n", pnote->text);
      fclose (fp);
    }
  fpReserve = fopen (NULL_FILE, "r");
}

void append_note (NOTE_DATA * pnote)
{
  FILE *fp;
  char *name;
  NOTE_DATA **list;
  NOTE_DATA *last;
  switch (pnote->type)
    {
    default:
      return;
    case NOTE_NOTE:
      name = NOTE_FILE;
      list = &note_list;
      break;
    case NOTE_IDEA:
      name = IDEA_FILE;
      list = &idea_list;
      break;
    case NOTE_PENALTY:
      name = PENALTY_FILE;
      list = &penalty_list;
      break;
    case NOTE_NEWS:
      name = NEWS_FILE;
      list = &news_list;
      break;
    case NOTE_CHANGES:
      name = CHANGES_FILE;
      list = &changes_list;
      break;
    case NOTE_LEGEND:
      name = LEGEND_FILE;
      list = &legend_list;
      break;
    case NOTE_OOCNOTE:
      name = OOCNOTE_FILE;
      list = &oocnote_list;
      break;
    case NOTE_PROJECTS:
      name = PROJECTS_FILE;
      list = &projects_list;
      break;
    }
  if (*list == NULL)
    *list = pnote;

  else
    {
      for (last = *list; last->next != NULL; last = last->next);
      last->next = pnote;
    }
  fclose (fpReserve);
  if ((fp = fopen (name, "a")) == NULL)
    {
      perror (name);
    }
  else
    {
      fprintf (fp, "Sender  %s~\n", pnote->sender);
      fprintf (fp, "Date    %s~\n", pnote->date);
      fprintf (fp, "Stamp   %ld\n", pnote->date_stamp);
      fprintf (fp, "To      %s~\n", pnote->to_list);
      fprintf (fp, "Subject %s~\n", pnote->subject);
      fprintf (fp, "Text\n%s~\n", pnote->text);
      fclose (fp);
    }
  fpReserve = fopen (NULL_FILE, "r");
}

bool is_note_to (CHAR_DATA * ch, NOTE_DATA * pnote)
{
  if (!str_cmp (ch->name, pnote->sender))
    return TRUE;

  // Akamai - remove current backdoor for notes
  //  if(!str_cmp(ch->name, "Cailet"))
  //    return TRUE;
  if (is_name_no_abbrev ("all", pnote->to_list))
    return TRUE;
  if (IS_IMMORTAL (ch) && (is_name_no_abbrev ("immortal", pnote->to_list) || is_name_no_abbrev ("immortals", pnote->to_list) || is_name_no_abbrev ("imm", pnote->to_list) || is_name_no_abbrev ("imms", pnote->to_list)))
    return TRUE;
  if (is_name_no_abbrev ("leaders", pnote->to_list) && is_clan_leader (ch))
    return TRUE;
	  
  if (IS_IMMORTAL (ch) && (ch->level > 99)
      && is_name_no_abbrev ("admin", pnote->to_list))
    return TRUE;
  if ((ch->clan != CLAN_BOGUS)
      && is_name_no_abbrev (get_clan_name_ch (ch), pnote->to_list))
    return TRUE;
  if (is_name_no_abbrev (ch->name, pnote->to_list))
    return TRUE;
  return FALSE;
}

void note_attach (CHAR_DATA * ch, int type)
{
  NOTE_DATA *pnote;
  if (ch->pnote != NULL)
    return;
  pnote = new_note ();
  pnote->next = NULL;
  if (!IS_NPC(ch))
     pnote->sender = str_dup (ch->name);
  else pnote->sender = str_dup (ch->short_descr);
  pnote->date = str_dup ("");
  pnote->to_list = str_dup ("");
  pnote->subject = str_dup ("");
  pnote->text = str_dup ("");
  pnote->type = type;
  ch->pnote = pnote;
  return;
}

void note_remove (CHAR_DATA * ch, NOTE_DATA * pnote, bool deletenote)
{
  char to_new[MAX_INPUT_LENGTH];
  char to_one[MAX_INPUT_LENGTH];
  NOTE_DATA *prev;
  NOTE_DATA **list;
  char *to_list;
  if (!deletenote)
    {

      /* make a new list */
      to_new[0] = '\0';
      to_list = pnote->to_list;
      while (*to_list != '\0')
	{
	  to_list = one_argument (to_list, to_one);
	  if (to_one[0] != '\0' && str_cmp (ch->name, to_one))
	    {
	      strcat (to_new, " ");
	      strcat (to_new, to_one);
	    }
	}

      /* Just a simple recipient removal? */
      if (str_cmp (ch->name, pnote->sender) && to_new[0] != '\0')
	{
	  free_string (pnote->to_list);
	  pnote->to_list = str_dup (to_new + 1);
	  return;
	}
    }

  /* nuke the whole note */
  switch (pnote->type)
    {
    default:
      return;
    case NOTE_NOTE:
      list = &note_list;
      break;
    case NOTE_IDEA:
      list = &idea_list;
      break;
    case NOTE_PENALTY:
      list = &penalty_list;
      break;
    case NOTE_NEWS:
      list = &news_list;
      break;
    case NOTE_CHANGES:
      list = &changes_list;
      break;
    case NOTE_OOCNOTE:
      list = &oocnote_list;
      break;
    case NOTE_LEGEND:
      list = &legend_list;
      break;
    case NOTE_PROJECTS:
      list = &projects_list;
      break;
    }

  /*
   * Remove note from linked list.
   */
  if (pnote == *list)
    {
      *list = pnote->next;
    }
  else
    {
      for (prev = *list; prev != NULL; prev = prev->next)
	{
	  if (prev->next == pnote)
	    break;
	}
      if (prev == NULL)
	{
	  bug ("Note_remove: pnote not found.", 0);
	  return;
	}
      prev->next = pnote->next;
    }
  save_notes (pnote->type);
  free_note (pnote);
  return;
}

bool hide_note (CHAR_DATA * ch, NOTE_DATA * pnote)
{
  time_t last_read;
  if (IS_NPC (ch))
    return TRUE;
  switch (pnote->type)
    {
    default:
      return TRUE;
    case NOTE_NOTE:
      last_read = ch->pcdata->last_note;
      break;
    case NOTE_IDEA:
      last_read = ch->pcdata->last_idea;
      break;
    case NOTE_PENALTY:
      last_read = ch->pcdata->last_penalty;
      break;
    case NOTE_NEWS:
      last_read = ch->pcdata->last_news;
      break;
    case NOTE_CHANGES:
      last_read = ch->pcdata->last_changes;
      break;
    case NOTE_OOCNOTE:
      last_read = ch->pcdata->last_oocnote;
      break;
    case NOTE_LEGEND:
      last_read = ch->pcdata->last_legend;
      break;
    case NOTE_PROJECTS:
      last_read = ch->pcdata->last_projects;
    }
  if (pnote->date_stamp <= last_read)
    return TRUE;
  if (!str_cmp (ch->name, pnote->sender))
    return TRUE;

  // Akamai -- remove a back door
  //  if (!is_note_to(ch, pnote) && str_cmp(ch->name, "Cailet") != 0)
  if (!is_note_to (ch, pnote))
    return TRUE;
  return FALSE;
}

void update_read (CHAR_DATA * ch, NOTE_DATA * pnote)
{
  time_t stamp;
  if (IS_NPC (ch))
    return;
  stamp = pnote->date_stamp;
  switch (pnote->type)
    {
    default:
      return;
    case NOTE_NOTE:
      ch->pcdata->last_note = UMAX (ch->pcdata->last_note, stamp);
      break;
    case NOTE_IDEA:
      ch->pcdata->last_idea = UMAX (ch->pcdata->last_idea, stamp);
      break;
    case NOTE_PENALTY:
      ch->pcdata->last_penalty = UMAX (ch->pcdata->last_penalty, stamp);
      break;
    case NOTE_NEWS:
      ch->pcdata->last_news = UMAX (ch->pcdata->last_news, stamp);
      break;
    case NOTE_CHANGES:
      ch->pcdata->last_changes = UMAX (ch->pcdata->last_changes, stamp);
      break;
    case NOTE_OOCNOTE:
      ch->pcdata->last_oocnote = UMAX (ch->pcdata->last_oocnote, stamp);
      break;
    case NOTE_LEGEND:
      ch->pcdata->last_legend = UMAX (ch->pcdata->last_legend, stamp);
      break;
    case NOTE_PROJECTS:
      ch->pcdata->last_projects = UMAX (ch->pcdata->last_projects, stamp);
      break;
    }
}
void do_catchup (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  if (IS_NPC (ch))
    {
      send_to_char ("Mobiles cannot catchup what they do not have.\r\n", ch);
      return;
    }
  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char
	("Syntax: catchup all\n\rWill catchup all notes in all spools.\n\r",
	 ch);
      return;
    }
  if (!str_cmp (arg, "all"))
    {
      ch->pcdata->last_note = current_time;
      ch->pcdata->last_idea = current_time;
      ch->pcdata->last_penalty = current_time;
      ch->pcdata->last_news = current_time;
      ch->pcdata->last_changes = current_time;
      ch->pcdata->last_oocnote = current_time;
      ch->pcdata->last_legend = current_time;
      ch->pcdata->last_projects = current_time;
      send_to_char ("All notes marked as read.\n\r", ch);
      return;
    }
  do_catchup (ch, "");
}

void parse_note (CHAR_DATA * ch, char *argument, int type)
{
  BUFFER *buffer;
  char buf[MAX_STRING_LENGTH];
  char bigbuf[MAX_STRING_LENGTH * 20];
  char arg[MAX_INPUT_LENGTH];
  NOTE_DATA *pnote;
  NOTE_DATA **list;
  char *list_name;
  int vnum;
  int anum;
  if (IS_NPC (ch) && !IS_SET(ch->act2,ACT_NO_KILL))
    return;
  switch (type)
    {
    default:
      return;
    case NOTE_NOTE:
      list = &note_list;
      list_name = "scrolls";
      break;
    case NOTE_IDEA:
      list = &idea_list;
      list_name = "ideas";
      break;
    case NOTE_PENALTY:
      list = &penalty_list;
      list_name = "penalties";
      break;
    case NOTE_NEWS:
      list = &news_list;
      list_name = "news";
      break;
    case NOTE_CHANGES:
      list = &changes_list;
      list_name = "changes";
      break;
    case NOTE_LEGEND:
      list = &legend_list;
      list_name = "legends";
      break;
    case NOTE_OOCNOTE:
      list = &oocnote_list;
      list_name = "oocnotes";
      break;
    case NOTE_PROJECTS:
      list = &projects_list;
      list_name = "projects";
      break;
    }
  argument = one_argument (argument, arg);
  smash_tilde (argument);
  if (arg[0] == '\0' || !str_prefix (arg, "read"))
    {
      bool fAll;
      if (!str_cmp (argument, "all"))
	{
	  fAll = TRUE;
	  anum = 0;
	}

      else if (argument[0] == '\0' || !str_prefix (argument, "next"))

	/* read next unread note */
	{
	  vnum = 0;
	  for (pnote = *list; pnote != NULL; pnote = pnote->next)
	    {
	      if (!hide_note (ch, pnote))
		{
		  sprintf (buf,
			   "[`o%3d``] `l%s``: %s\n\r%s\n\rTo: `k%s``\n\r",
			   vnum, pnote->sender, pnote->subject,
			   pnote->date, pnote->to_list);
		  send_to_char (buf, ch);
		  page_to_char (pnote->text, ch);
		  update_read (ch, pnote);
		  return;
		}
	      //Iblis - 06/04/04 - Fixed so everyone's note numbers will be the same
	      else //if (is_note_to (ch, pnote))
		vnum++;
	    }
	  sprintf (buf, "You have no unread %s.\n\r", list_name);
	  send_to_char (buf, ch);

	  /*      if (!vnum)
	     {
	     switch(type)
	     {
	     case NOTE_NOTE:
	     send_to_char("There are no notes for you.\n\r",ch);
	     break;
	     case NOTE_IDEA:
	     send_to_char("There are no ideas for you.\n\r",ch);
	     break;
	     case NOTE_PENALTY:
	     send_to_char("There are no penalties for you.\n\r",ch);
	     break;
	     case NOTE_NEWS:
	     send_to_char("There is no news for you.\n\r",ch);
	     break;
	     case NOTE_CHANGES:
	     send_to_char("There are no changes for you.\n\r",ch);
	     break;
	     }
	     }
	   */
	  return;
	}

      else if (is_number (argument))
	{
	  fAll = FALSE;
	  anum = atoi (argument);
	}
      else
	{
	  send_to_char ("Read which number?\n\r", ch);
	  return;
	}
      vnum = 0;
      for (pnote = *list; pnote != NULL; pnote = pnote->next)
	{
	  if (is_note_to (ch, pnote) )
	  {			  
  	    if (vnum++ == anum || fAll)
	      {
	      sprintf (buf,
		       "[`o%3d``] `l%s``: %s\n\r%s\n\rTo: `k%s``\n\r",
		       vnum - 1, pnote->sender, pnote->subject,
		       pnote->date, pnote->to_list);
	      send_to_char (buf, ch);
	      page_to_char (pnote->text, ch);
	      update_read (ch, pnote);
	      return;
	      }
	  }
	  //Iblis - 06/04/04 - Added so everyone's note numbers will be the same
	  else vnum++;
	 // sprintf (buf,"vnum - %d, anum - %d, subj - %s",vnum,anum,pnote->subject);
	 // send_to_char(buf,ch);
	}
      sprintf (buf, "There aren't that many %s, or you can't read that one.\n\r", list_name);
      send_to_char (buf, ch);
      return;
    }
  if (!str_prefix (arg, "list"))
    {
      int to_list=-1, counter=0;
      //char arg2[MAX_INPUT_STRING];
      argument = one_argument(argument,arg);
      if (is_number (arg))
        to_list = atoi(arg);
      vnum = 0;
      bigbuf[0] = '\0';
      for (pnote = *list; pnote != NULL; pnote = pnote->next)
	{
	  if (is_note_to (ch, pnote))
	    {
	      if (to_list == -1)
	      {
	        sprintf (buf, "[`o%3d`n%s``] `l%s``: %s\n\r", vnum,
	  	       hide_note (ch, pnote) ? " " : "N", pnote->sender,
		       pnote->subject);

  	        //send_to_char(buf, ch);
	        strcat (bigbuf, buf);
	        vnum++;
	      }
	      else counter++;
	    }
	  //Iblis - 06/04/04 - Added so everyone's note numbers will be the same
	  else {
		  vnum++;
//		  counter++;
	  }
	}
      if (to_list != -1)
      {
        int counter2 = 0;
        vnum = 0;
        for (pnote = *list; pnote != NULL; pnote = pnote->next)
        {
		
          if (is_note_to (ch, pnote))
          {
	    if (vnum++ >= counter-to_list)
	    {
              sprintf (buf, "[`o%3d`n%s``] `l%s``: %s\n\r", counter2,
	           hide_note (ch, pnote) ? " " : "N", pnote->sender,
	           pnote->subject);
	      strcat (bigbuf, buf);
	    }
          }
	  //Iblis - 06/04/04 - Added so everyone's note numbers will be the same
//	  else ter2;
	  counter2++;
	}
      }

      // Shinowlan  7/27/98 -- send note base list one page at a time.  
      // Sending the list as one continous display causes connections
      // to be dropped if there are more than about 300 lines to be 
      // displayed.
      bigbuf[strlen (bigbuf) + 1] = '\0';
      page_to_char (bigbuf, ch);
      return;
    }
  if (!str_prefix (arg, "remove"))
    {
      if (!is_number (argument))
	{
	  send_to_char ("Note remove which number?\n\r", ch);
	  return;
	}
      anum = atoi (argument);
      vnum = 0;
      for (pnote = *list; pnote != NULL; pnote = pnote->next)
	{
	  if (is_note_to (ch, pnote))
	  {
  	    if (vnum++ == anum)
	      {
	        note_remove (ch, pnote, FALSE);
	        send_to_char ("Ok.\n\r", ch);
	        return;
	      }
	  }
	  //Iblis - 06/04/04 - Added so everyone's note numbers will be the same
	  else ++vnum;
	}
      sprintf (buf, "There aren't that many %s, or you can't remove that one..", list_name);
      send_to_char (buf, ch);
      return;
    }
  if (!str_prefix (arg, "delete") && get_trust (ch) >= MAX_LEVEL - 1)
    {
      if (!is_number (argument))
	{
	  send_to_char ("Note delete which number?\n\r", ch);
	  return;
	}
      anum = atoi (argument);
      vnum = 0;
      for (pnote = *list; pnote != NULL; pnote = pnote->next)
	{
	  if (is_note_to (ch, pnote))
	  {
  	    if (vnum++ == anum)
	    {
	      note_remove (ch, pnote, TRUE);
	      send_to_char ("Ok.\n\r", ch);
	      return;
	    }
	  }
	  //Iblis - 06/04/04 - Added so everyone's note numbers will be the same
	  else ++vnum;
	}
      sprintf (buf, "There aren't that many %s, or you can't delete that one..", list_name);
      send_to_char (buf, ch);
      return;
    }
  if (!str_prefix (arg, "catchup"))
    {
      switch (type)
	{
	case NOTE_NOTE:
	  ch->pcdata->last_note = current_time;
	  break;
	case NOTE_IDEA:
	  ch->pcdata->last_idea = current_time;
	  break;
	case NOTE_PENALTY:
	  ch->pcdata->last_penalty = current_time;
	  break;
	case NOTE_NEWS:
	  ch->pcdata->last_news = current_time;
	  break;
	case NOTE_CHANGES:
	  ch->pcdata->last_changes = current_time;
	  break;
	case NOTE_OOCNOTE:
	  ch->pcdata->last_oocnote = current_time;
	  break;
	case NOTE_LEGEND:
	  ch->pcdata->last_legend = current_time;
	  break;
	case NOTE_PROJECTS:
	  ch->pcdata->last_projects = current_time;
	  break;
	}
      return;
    }

  /* below this point only certain people can edit notes */
  if ((type == NOTE_NEWS && !IS_TRUSTED (ch, DEMIGOD))
      || (type == NOTE_CHANGES && !IS_TRUSTED (ch, WIZARD))
      || (type == NOTE_PROJECTS && !IS_TRUSTED (ch, WIZARD)))
    {
      sprintf (buf, "You aren't high enough level to write %s.", list_name);
      send_to_char (buf, ch);
      return;
    }
  if (!str_cmp (arg, "+"))
    {
      note_attach (ch, type);
      if (ch->pnote->type != type)
	{
	  send_to_char
	    ("You already have a different note in progress.\n\r", ch);
	  return;
	}
      if (strlen (ch->pnote->text) + strlen (argument) >= 4096)
	{
	  send_to_char ("Note too long.\n\r", ch);
	  return;
	}
      buffer = new_buf ();
      add_buf (buffer, ch->pnote->text);
      add_buf (buffer, argument);
      add_buf (buffer, "\n\r");
      free_string (ch->pnote->text);
      ch->pnote->text = str_dup (buf_string (buffer));
      free_buf (buffer);
      send_to_char ("Ok.\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "-"))
    {
      int len;
      bool found = FALSE;
      note_attach (ch, type);
      if (ch->pnote->type != type)
	{
	  send_to_char
	    ("You already have a different note in progress.\n\r", ch);
	  return;
	}
      if (ch->pnote->text == NULL || ch->pnote->text[0] == '\0')
	{
	  send_to_char ("No lines left to remove.\n\r", ch);
	  return;
	}
      strcpy (buf, ch->pnote->text);
      for (len = strlen (buf); len > 0; len--)
	{
	  if (buf[len] == '\r')
	    {
	      if (!found)
		{		/* back it up */
		  if (len > 0)
		    len--;
		  found = TRUE;
		}
	      else
		/* found the second one */
		{
		  buf[len + 1] = '\0';
		  free_string (ch->pnote->text);
		  ch->pnote->text = str_dup (buf);
		  return;
		}
	    }
	}
      buf[0] = '\0';
      free_string (ch->pnote->text);
      ch->pnote->text = str_dup (buf);
      return;
    }
  if (!str_prefix (arg, "subject"))
    {
      note_attach (ch, type);
      if (ch->pnote->type != type)
	{
	  send_to_char
	    ("You already have a different note in progress.\n\r", ch);
	  return;
	}
      free_string (ch->pnote->subject);
      ch->pnote->subject = str_dup (argument);
      send_to_char ("Ok.\n\r", ch);
      return;
    }
  if (!str_prefix (arg, "to"))
    {
      note_attach (ch, type);
      if (ch->pnote->type != type)
	{
	  send_to_char
	    ("You already have a different note in progress.\n\r", ch);
	  return;
	}
      free_string (ch->pnote->to_list);
      ch->pnote->to_list = str_dup (argument);
      send_to_char ("Ok.\n\r", ch);
      return;
    }
  if (!str_prefix (arg, "clear"))
    {
      if (ch->pnote != NULL)
	{
	  free_note (ch->pnote);
	  ch->pnote = NULL;
	}
      send_to_char ("Ok.\n\r", ch);
      return;
    }
  if (!str_prefix (arg, "show"))
    {
      if (ch->pnote == NULL)
	{
	  send_to_char ("You have no note in progress.\n\r", ch);
	  return;
	}
      if (ch->pnote->type != type)
	{
	  send_to_char ("You aren't working on that kind of note.\n\r", ch);
	  return;
	}
      sprintf (buf, "%s: %s\n\rTo: %s\n\r", ch->pnote->sender,
	       ch->pnote->subject, ch->pnote->to_list);
      send_to_char (buf, ch);
      send_to_char (ch->pnote->text, ch);
      return;
    }
  if (!str_prefix (arg, "post") || !str_prefix (arg, "send"))
    {
      char *strtime;
      if (ch->pnote == NULL)
	{
	  send_to_char ("You have no note in progress.\n\r", ch);
	  return;
	}
      if (ch->pnote->type != type)
	{
	  send_to_char ("You aren't working on that kind of note.\n\r", ch);
	  return;
	}
      if (!str_cmp (ch->pnote->to_list, ""))
	{
	  send_to_char
	    ("You need to provide a recipient (name, all, admin or immortal).\n\r",
	     ch);
	  return;
	}
      else
	{

	  // Akamai 06/09/99 - Fix notes so that chars must be at least
	  // level MIN_LEVEL_ALL to post to 'all' - constant defined above
	  if (is_name_no_abbrev ("all", ch->pnote->to_list))
	    {
	      if (ch->level < MIN_LEVEL_ALL)
		{
		  send_to_char ("Your note is addressed to \"all\".\n\r", ch);
		  sprintf (buf,
			   "You must be at least level %d to post to \"all\"\n\r",
			   MIN_LEVEL_ALL);
		  send_to_char (buf, ch);
		  return;
		}
	    }
	}
      if (!str_cmp (ch->pnote->subject, ""))
	{
	  send_to_char ("You need to provide a subject.\n\r", ch);
	  return;
	}
//      sprintf(buf,"`hSomeone just posted a note to someone else, and if you ask me, that is a private matter!!``");
//  You can't be fricking serious, the fact that someone posts notes to
//  someone else, is a private matter, Back when Cailet imped this place
//  she had a paranoia clause, that made her able to read all notes posted.
//  That was removed, and now you are going back to that again ?
//
//      sprintf(buf,"`h$N just posted a note to %s``",ch->pnote->to_list);
      //wiznet (buf, ch, NULL, WIZ_NOTES, 0, get_trust (ch));
      ch->pnote->next = NULL;
      strtime = ctime (&current_time);
      strtime[strlen (strtime) - 1] = '\0';
      ch->pnote->date = str_dup (strtime);
      ch->pnote->date_stamp = current_time;
      append_note (ch->pnote);
      ch->pnote = NULL;
      send_to_char ("Ok.\n\r", ch);
      return;
    }
  send_to_char ("You can't do that.\n\r", ch);
  return;
}
