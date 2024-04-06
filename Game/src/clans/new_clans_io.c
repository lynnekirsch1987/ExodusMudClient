//
// new_clans_io.c
//
// Programmer: Akamai
// Date: 7/10/98
// Version: 1.0
//
// This code handles the file io for the clan table

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include "../merc.h"
#include "../db.h"
#include "new_clans.h"
#include "new_clans_io.h"
#include "new_clans_util.h"

// 
// define access to some public variables
//
extern int _clan_ident_;
extern int clan_number;
extern struct clan_type clan_table[MAX_CLAN];

//
// private prototypes only valid in this file
//
#ifndef TESTING
void low_save_clanfile (FILE * cfp, int clan);
void low_save_members (FILE * cfp, MEMBER * member);
void low_save_relations (FILE * cfp, RELATIONS * relate);
void low_load_clanfile (FILE * cfp, int clan);
MEMBER *low_load_members (FILE * cfp);
RELATIONS *low_load_relations (FILE * cfp);

#endif // TESTING
void init_clan_table (void);

//
// private variable for the i/o functions
//
static bool clans_loaded = FALSE;

// ==============================================================
//
//   Public Functions for Clan file operations
//
// ==============================================================

//
// The clan load routine, executed on mud startup
//
void load_clans ()
{
  char buf[MAX_STRING_LENGTH];
  char name[MAX_STRING_LENGTH];
  int s, slot, i, j;
#ifdef VERBOSE_BOOT
  log_string ("boot_db() - loading clans.");
#endif
  // load the list of the clans that should be loaded
  if (load_clanindex ())
    {
      s = CLANS_START;
      slot = CLANS_START;
      while (slot < clan_number)
	{
	  i = 0;
	  j = 0;

	  // make the clan name lowercase
	  while (clan_table[s].name[i])
	    {
	      if (isalnum ((int) clan_table[s].name[i]))
		{
		  name[j] = tolower (clan_table[s].name[i]);
		  j++;
		}
	      i++;
	    }
	  name[j] = '\0';

	  // load the clan into this slot
	  if (load_clan (slot, name))
	    {
	      s++;
	      slot++;
	    }

	  else
	    {
	      s++;
	      clan_number--;
	      sprintf (buf,
		       "load_clans() - clan %s appears corrupt, not loaded.",
		       name);
	      bug (buf, 0);
	    }
	}
    }

  else
    {
      bug
	("load_clans() - clanindex appears to be corrupt, clan load failed.",
	 0);
      return;
    }
  clans_loaded = TRUE;
//  log_string ("Clan loading finished, check log for individual errors.");
}


// 
// Save the clan index and save all the individual clans
//
void save_clans ()
{
  int slot = 0;
  log_string ("Saving all clans.");
  save_clanindex ();
  for (slot = CLANS_START; slot < clan_number; slot++)
    {
      save_clan (slot);
    }
  log_string ("Clan saving finished, check log for individual errors.");
}


//
// the old text saving routine, this is here for backward compatibility
//
void save_clans_txt ()
{
  save_clans ();
}

//
// Load the index of active clans
//
bool load_clanindex ()
{
  char buf[MAX_STRING_LENGTH];
  char linebuf[MAX_STRING_LENGTH];
  char *line = NULL;
  FILE *cfp;
  int slot;
  clan_number = CLANS_START;
  init_clan_table ();
  sprintf (buf, "%s/%s", CLAN_DIR, CLANINDEX);
  cfp = fopen (buf, "r");

  // check that the open was successful
  if (cfp == NULL)
    {
      bug ("load_clanindex() file could not be opened.", 0);
      return (FALSE);
    }
  while ((fgets (linebuf, MAX_STRING_LENGTH, cfp) != NULL))
    {

      // remove the newline at the end of this line
      linebuf[strlen (linebuf) - 1] = '\0';
      line = strip_whitespace (linebuf);

      // check that it's not a comment and not empty
      switch (line[0])
	{
	case '#':
	case '\0':

	  // do nothing this is just a comment or blank
	  break;
	case 'I':
	case 'i':
	  line = find_sepchar (line);
	  line = strip_whitespace (&(line[1]));
	  if (sscanf (line, "%d", &(_clan_ident_)) != 1)
	    {
	      _clan_ident_ = 1;
	      bug ("load_clanindex() the clan ident bad.", 0);
	    }
	  break;
	case 'C':
	case 'c':
	  line = find_sepchar (line);
	  line = strip_whitespace (&(line[1]));

	  // copy the name into the table, this will be replaced when the
	  // table is loaded from the clan file
	  if (line[0] != '\0')
	    {
	      slot = clanname_to_slot (line);
	      if (slot == CLAN_BOGUS)
		{
		  if (!limited_copy_str
		      (clan_table[clan_number].name, line, MAX_NAME))
		    {
		      bug
			("load_clanindex() clan name was too long, truncated.",
			 0);
		    }
		  clan_number++;
		}

	      else
		{
		  bug ("load_clanindex() found duplicate clan name.", 0);
		}
	    }

	  else
	    {
	      bug ("load_clanindex() found a blank/null clan.", 0);
	    }
	  break;
	default:
	  bug ("load_clanindex() unrecognized tag.", 0);
	  break;
	}
    }
  fclose (cfp);
//  log_string ("Loaded clan index.");
  return (TRUE);
}


//
// This saves the a file listing each of the clans in the clan table
//
void save_clanindex ()
{
  char temp[MAX_STRING_LENGTH];
  char name[MAX_STRING_LENGTH];
  char line[MAX_STRING_LENGTH];
  char *strtime;
  FILE *clanfp;
  int i, j, clan = 0;
  if (!clans_loaded)
    {
      bug ("save_clanindex() - clans were not loaded properly.", 0);
      bug ("save_clanindex() - load error or corruption prevents a save.", 0);
      return;
    }
  sprintf (temp, "%s/%s.update", CLAN_DIR, CLANINDEX);
  sprintf (name, "%s/%s", CLAN_DIR, CLANINDEX);
  clanfp = fopen (temp, "w");

  // check that the open was successful
  if (clanfp == NULL)
    {
      bug ("save_clanindex() file could not be opened.", 0);
      return;
    }
  // get the time and chop off the newline
  current_time = time (NULL);
  strtime = ctime (&current_time);
  strtime[strlen (strtime) - 1] = '\0';

  // print a little header for this file
  fprintf (clanfp, "# Modified: %s\n", strtime);
  fprintf (clanfp, "# Total Clans: %d\n", (clan_number - 1));
  fprintf (clanfp, "#\n");
  fprintf (clanfp, "Ident: %d\n", _clan_ident_);

  // now write out each of the clan names
  for (clan = CLANS_START; clan < clan_number; clan++)
    {
      i = 0;
      j = 0;

      // make the clan name lowercase
      while (clan_table[clan].name[i])
	{

	  // make sure it is lowercase alphanumeric name only
	  if (isalnum ((int) clan_table[clan].name[i]))
	    {
	      line[j] = tolower (clan_table[clan].name[i]);
	      j++;
	    }
	  i++;
	}
      line[j] = '\0';
      if (line[0] == '\0')
	{
	  bug ("save_clanindex() null clan name.", 0);
	}

      else
	{
	  fprintf (clanfp, "Clan: %s\n", line);
	}
    }
  fclose (clanfp);

  // now that the file is written, remove the old file and
  // rename the temp file to be the new file name
  unlink (name);
  rename (temp, name);
  log_string ("Saved clan index.");
}


//
// Save a clan box
//
void save_clan_box (OBJ_DATA * pObj)
{

#ifdef VERBOSE_CLANS
  char buf[MAX_STRING_LENGTH];

#endif /*  */
  char fname[MAX_STRING_LENGTH];
  char clan_name[MAX_STRING_LENGTH];
  int slot, i, j;
  FILE *fp;
  slot = pObj->value[1];

  // if the container object value does not have a clan code
  if (slot == CLAN_BOGUS)
    {
      bug ("save_clan_box() - CLAN_BOGUS on the donation box object.", 0);
      return;
    }
  // make the clan name lowercase
  i = 0;
  j = 0;
  while (clan_table[slot].name[i])
    {
      if (isalnum ((int) clan_table[slot].name[i]))
	{
	  clan_name[j] = tolower (clan_table[slot].name[i]);
	  j++;
	}
      i++;
    }
  clan_name[j] = '\0';
  if (clan_name[0] == '\0')
    {
      bug ("save_clan_box() - null clan name.", 0);
      return;
    }
  // set the file name
  sprintf (fname, "%s/%s.dbox", CLAN_DIR, clan_name);

#ifdef VERBOSE_CLANS
  sprintf (buf, "save_clan_box() - file \"%s\"", fname);
  log_string (buf);

#endif /*  */

  // if the donation box has no items, just delete the file
  if (pObj->contains == NULL)
    {

#ifdef VERBOSE_CLANS
      log_string ("save_clan_box() - box is empty, deleting file.");

#endif /*  */
      unlink (fname);
      return;
    }
  // open and save the clan donation box;
  fp = fopen (fname, "w");
  if (fp == NULL)
    {
      bug ("save_clan_box() - could not open file for write.", 0);
      return;
    }
#ifdef VERBOSE_CLANS
  log_string ("save_clan_box() - saving objects.");

#endif /*  */
  fwrite_objdb (pObj->contains, fp, 0);
  fclose (fp);
}


//
// load a clan box from disk
//
void load_clan_box (OBJ_DATA * pObj)
{

#ifdef VERBOSE_CLANS
  char buf[MAX_STRING_LENGTH];

#endif /*  */
  char fname[MAX_STRING_LENGTH];
  char clan_name[MAX_STRING_LENGTH];
  int slot, i, j;
  FILE *fp;
  slot = pObj->value[1];

  // if the container object value does not have a clan code
  if (slot == CLAN_BOGUS)
    {
      bug ("load_clan_box() - CLAN_BOGUS on the donation box object.", 0);
      return;
    }
  if ((slot < CLANS_START) || (slot >= clan_number))
    {
      bug ("load_clan_box() - Bad clan number on the donation box object.",
	   0);
      return;
    }
#ifdef VERBOSE_CLANS
  sprintf (buf, "load_clan_box() - attempting to load %d.", slot);
  log_string (buf);

#endif /*  */

  // make the clan name lowercase
  i = 0;
  j = 0;
  while (clan_table[slot].name[i])
    {
      if (isalnum ((int) clan_table[slot].name[i]))
	{
	  clan_name[j] = tolower (clan_table[slot].name[i]);
	  j++;
	}
      i++;
    }
  clan_name[j] = '\0';
  if (clan_name[0] == '\0')
    {
      bug ("load_clan_box() - null clan name.", 0);
      return;
    }
  // set the file name
  sprintf (fname, "%s/%s.dbox", CLAN_DIR, clan_name);

#ifdef VERBOSE_CLANS
  sprintf (buf, "load_clan_box() - file \"%s\"", fname);
  log_string (buf);

#endif /*  */
  fp = fopen (fname, "r");
  if (fp == NULL)
    {
      bug ("load_clan_box() - could not open file for read.", 0);
      return;
    }
  while (!feof (fp) && !str_cmp (fread_word (fp), "#O"))
    fread_objdb (pObj, fp);
  fclose (fp);
}


//
// Save a single clan in a single slot
//
void save_clan (int clan_slot)
{
  char temp[MAX_STRING_LENGTH];
  char fname[MAX_STRING_LENGTH];
  char clan_name[MAX_STRING_LENGTH];
  FILE *cfp;
  int i = 0;
  int j = 0;
  if (!clans_loaded || (clan_table[clan_slot].status == CLAN_BOGUS))
    {
      bug
	("save_clan() - improper load or corrupted clan prevents clan save.",
	 0);
      return;
    }
  // make the clan name lowercase alphanumeric
  while (clan_table[clan_slot].name[i])
    {

      // make sure it is lowercase alphanumeric name only
      if (isalnum ((int) clan_table[clan_slot].name[i]))
	{
	  clan_name[j] = tolower (clan_table[clan_slot].name[i]);
	  j++;
	}
      i++;
    }
  clan_name[j] = '\0';

  // better have some clan name
  if (clan_name[0] == '\0')
    {
      sprintf (temp, "save_clan() null length clan name slot %d.", clan_slot);
      bug (temp, 0);
      return;
    }
  sprintf (temp, "%s/%s.update", CLAN_DIR, clan_name);
  sprintf (fname, "%s/%s.clan", CLAN_DIR, clan_name);
  cfp = fopen (temp, "w");
  if (cfp == NULL)
    {
      sprintf (fname,
	       "save_clan() - the clanfile \"%s\" could not be opened.",
	       temp);
      bug (fname, 0);
      return;
    }
  // Write the clan data
  low_save_clanfile (cfp, clan_slot);
  fclose (cfp);

  // now remove the old one and replace it with the new one
  unlink (fname);
  rename (temp, fname);
  sprintf (temp, "Saved clan %s ID:%d Status:%d Type:%d Slot:%d",
	   clan_table[clan_slot].name, clan_table[clan_slot].ident,
	   clan_table[clan_slot].status, clan_table[clan_slot].ctype,
	   clan_slot);
  log_string (temp);
}


//
// load a clan file into a specified slot in the clan table
//
bool load_clan (int clan_slot, char *name)
{
  char clan_name[MAX_STRING_LENGTH];
  char fname[MAX_STRING_LENGTH];
  FILE *cfp;
  int i = 0;
  int j = 0;

  // make the clan name lowercase alphanumeric
  while (name[i])
    {

      // make sure it is lowercase alphanumeric name only
      if (isalnum ((int) name[i]))
	{
	  clan_name[j] = tolower (name[i]);
	  j++;
	}
      i++;
    }
  clan_name[j] = '\0';

  // better have some clan name
  if (clan_name[0] == '\0')
    {
      bug ("load_clan() null length clan name.", 0);
      return (FALSE);
    }
  sprintf (fname, "%s/%s.clan", CLAN_DIR, clan_name);
  cfp = fopen (fname, "r");
  if (cfp == NULL)
    {
      sprintf (clan_name, "load_clan() - \"%s\" could not be opened.", fname);
      bug (clan_name, 0);
      return (FALSE);
    }
  // load this clan into this slot in the clan table
  low_load_clanfile (cfp, clan_slot);
  fclose (cfp);

//       Since we just about now know it works, there's no reason for
//       putting it in logs
//
//  sprintf (clan_name, "Loaded clan %s ID:%d Status:%d Type:%d Slot:%d",
//	   clan_table[clan_slot].name, clan_table[clan_slot].ident,
//	   clan_table[clan_slot].status, clan_table[clan_slot].ctype,
//	   clan_slot);
//  log_string (clan_name);


  return (TRUE);
}


// ==============================================================
//
//   Private Functions for Clan file operations
//
// ==============================================================

// this function does a low level save of a clan's entry in
// the clan table
void low_save_clanfile (FILE * cfp, int clan)
{
  char *strtime;

  // get the time and chop off the newline
  current_time = time (NULL);
  strtime = ctime (&current_time);
  strtime[strlen (strtime) - 1] = '\0';

  // print a little header for this file
  fprintf (cfp, "# Modified: %s\n", strtime);
  fprintf (cfp, "#\n");
  fprintf (cfp, "Name: %s\n", clan_table[clan].name);
  fprintf (cfp, "Symb: %s\n", clan_table[clan].symbol);
  switch (clan_table[clan].ctype)
    {
    case CLAN_TYPE_TEST:
      fprintf (cfp, "Type: TestClan\n");
      break;
    case CLAN_TYPE_MOB:
      fprintf (cfp, "Type: Mobiles\n");
      break;
    case CLAN_TYPE_PC:
      fprintf (cfp, "Type: NoType\n");
      break;
    case CLAN_TYPE_DEM:
      fprintf (cfp, "Type: Democratic\n");
      break;
    case CLAN_TYPE_REL:
      fprintf (cfp, "Type: Religious\n");
      break;
    case CLAN_TYPE_TECH:
      fprintf (cfp, "Type: Technologist\n");
      break;
    case CLAN_TYPE_EXPL:
      fprintf (cfp, "Type: Explorer\n");
      break;
    case CLAN_TYPE_MIL:
      fprintf (cfp, "Type: Militaristic\n");
      break;
    default:
      fprintf (cfp, "Type: %d - bogus?\n", clan_table[clan].ctype);
      bug ("low_save_clan() - bogus clan type.", 0);
      break;
    }
  fprintf (cfp, "Ident: %d\n", clan_table[clan].ident);
  fprintf (cfp, "Hall: %d\n", clan_table[clan].hall);
  if (clan_table[clan].align == CLAN_GOOD_ALIGN)
    {
      fprintf (cfp, "Align: Good\n");
    }

  else if (clan_table[clan].align == CLAN_EVIL_ALIGN)
    {
      fprintf (cfp, "Align: Evil\n");
    }

  else if (clan_table[clan].align == CLAN_NEUTRAL_ALIGN)
    {
      fprintf (cfp, "Align: Neutral\n");
    }

  else
    {
      fprintf (cfp, "Align: NoAlign\n");
    }
  switch (clan_table[clan].status)
    {
    case CLAN_DISPERSED:
      fprintf (cfp, "Status: Dispersed\n");
      break;
    case CLAN_DISBAND:
      fprintf (cfp, "Status: Disband\n");
      break;
    case CLAN_PROPOSED:
      fprintf (cfp, "Status: Proposed\n");
      break;
    case CLAN_ACTIVE:
      fprintf (cfp, "Status: Active\n");
      break;
    case CLAN_RESTRICTED:
      fprintf (cfp, "Status: Restricted\n");
      break;
    case CLAN_HISTORICAL:
      fprintf (cfp, "Status: Historical\n");
      break;
    case CLAN_INACTIVE:
      fprintf (cfp, "Status: Inactive\n");
      break;
    default:
      fprintf (cfp, "Status: Inactive\n");
      bug ("low_save_clanfile() - bad status, saving clan as inactive.", 0);
      break;
    }
  fprintf (cfp, "Levels: %d\n", clan_table[clan].levels);

  // save the members and applicants
  fprintf (cfp, "Members:\n");
  low_save_members (cfp, clan_table[clan].members);
  fprintf (cfp, "End\n");
  fprintf (cfp, "Applicants:\n");
  low_save_members (cfp, clan_table[clan].applicants);
  fprintf (cfp, "End\n");

  // save the relations to other clans
  fprintf (cfp, "Relations:\n");
  low_save_relations (cfp, clan_table[clan].relations);
  fprintf (cfp, "End\n");

  // yeah, ok, end the whole record
  fprintf (cfp, "End\n");
}


//
// save a list of members
//
void low_save_members (FILE * cfp, MEMBER * m)
{

  // run through the list dumping one line per member
  while (m != NULL)
    {
      fprintf (cfp, "Member: %s:%d:%d:%d:%d:%d:%ld:%d\n", m->name,
	       m->levels, m->status, m->align, m->race, m->Class,
	       m->laston, m->initiative);
      m = m->next;
    }
}


//
// save a relations list
//
void low_save_relations (FILE * cfp, RELATIONS * r)
{

  // run through the list of relations
  while (r != NULL)
    {
      fprintf (cfp, "Relate: %s:%d:%d:%ld:%ld:%d:%d:%d:%d:%d\n",
	       r->name, r->ident, r->status, r->acttime, r->wartime,
	       r->warcount, r->loss, r->wins, r->kills, r->points);
      r = r->next;
    }
}


// load the clans entry into the clan table
void low_load_clanfile (FILE * cfp, int clan)
{
  MEMBER *m;
  char linebuf[MAX_STRING_LENGTH];
  bool end_found = FALSE;
  char *line;
  while (!end_found && (fgets (linebuf, MAX_STRING_LENGTH, cfp) != NULL))
    {

      // remove the newline character
      linebuf[strlen (linebuf) - 1] = '\0';
      line = strip_whitespace (linebuf);

      // comment line?
      if ((line[0] != '#') && (line[0] != '\0'))
	{

	  // at the end the record?
	  if (!str_cmp ("End", line))
	    {
	      end_found = TRUE;
	    }

	  else
	    {

	      // ok, it's one of the things we are interested in
	      switch (line[0])
		{
		case 'N':
		case 'n':	/* clan Name */
		  line = find_sepchar (line);
		  line = strip_whitespace (&(line[1]));
		  if (!limited_copy_str
		      (clan_table[clan].name, line, MAX_NAME))
		    bug ("low_load_clanfile() clan name too long.", 0);
		  remove_spaces (clan_table[clan].name);
		  break;
		case 'S':
		case 's':	/* clan Symbol and clan Status */
		  if ((line[1] == 'y') || (line[1] == 'Y'))
		    {
		      line = find_sepchar (line);
		      line = strip_whitespace (&(line[1]));
		      if (!limited_copy_str
			  (clan_table[clan].symbol, line, MAX_SYMB))
			bug ("low_load_clanfile() clan symbol too long.", 0);
		    }

		  else if ((line[1] == 't') || (line[1] == 'T'))
		    {
		      line = find_sepchar (line);
		      line = strip_whitespace (&(line[1]));

		      // there are two values that begin with a 'd'
		      if ((line[0] == 'D') || (line[0] == 'd'))
			{
			  if (strchr (line, 'P') || strchr (line, 'p'))
			    {
			      clan_table[clan].status = CLAN_DISPERSED;
			    }

			  else if (strchr (line, 'B') || strchr (line, 'b'))
			    {
			      clan_table[clan].status = CLAN_DISBAND;
			    }

			  else
			    {
			      clan_table[clan].status = CLAN_INACTIVE;
			      bug
				("low_load_clanfile() unknown status value.",
				 0);
			    }
			}

		      else if ((line[0] == 'R') || (line[0] == 'r'))
			{
			  clan_table[clan].status = CLAN_RESTRICTED;
			}

		      else if ((line[0] == 'P') || (line[0] == 'p'))
			{
			  clan_table[clan].status = CLAN_PROPOSED;
			}

		      else if ((line[0] == 'A') || (line[0] == 'a'))
			{
			  clan_table[clan].status = CLAN_ACTIVE;
			}

		      else if ((line[0] == 'I') || (line[0] == 'i'))
			{
			  clan_table[clan].status = CLAN_INACTIVE;
			}

		      else if ((line[0] == 'H') || (line[0] == 'h'))
			{
			  clan_table[clan].status = CLAN_HISTORICAL;
			}

		      else
			{
			  clan_table[clan].status = CLAN_INACTIVE;
			  bug ("low_load_clanfile() unknown status value.",
			       0);
			}
		    }

		  else
		    {
		      bug ("low_load_clanfile() unknown 's' tag.", 0);
		    }
		  break;
		case 'I':
		case 'i':	/* clan Ident */
		  if ((line[1] == 'd') || (line[1] == 'D'))
		    {

		      // this is Identifier
		      line = find_sepchar (line);
		      line = strip_whitespace (&(line[1]));
		      if (sscanf (line, "%d", &(clan_table[clan].ident)) != 1)
			{
			  clan_table[clan].ident = CLAN_BOGUS;
			  bug
			    ("low_load_clanfile() the clan identifier was bad.",
			     0);
			}
		    }

		  else
		    {
		      bug ("low_load_clanfile() unknown 'i' flag.", 0);
		    }
		  break;
		case 'H':
		case 'h':	/* clan Hall */
		  line = find_sepchar (line);
		  line = strip_whitespace (&(line[1]));
		  if (sscanf (line, "%d", &(clan_table[clan].hall)) != 1)
		    {
		      clan_table[clan].hall = ROOM_VNUM_ALTAR;
		      bug ("low_load_clanfile() the clan hall was bad.", 0);
		    }
		  break;
		case 'T':
		case 't':	/* clan type */
		  line = find_sepchar (line);
		  line = strip_whitespace (&(line[1]));
		  switch (line[0])
		    {
		    case 'M':
		    case 'm':
		      if ((line[1] == 'I') || (line[1] == 'i'))
			{
			  clan_table[clan].ctype = CLAN_TYPE_MIL;
			}

		      else if ((line[1] == 'O') || (line[1] == 'o'))
			{
			  clan_table[clan].ctype = CLAN_TYPE_MOB;
			}

		      else
			{
			  clan_table[clan].ctype = CLAN_BOGUS;
			  bug ("low_load_clanfile() the clan type was bad.",
			       0);
			}
		      break;
		    case 'N':
		    case 'n':
		      clan_table[clan].ctype = CLAN_TYPE_PC;
		      break;
		    case 'E':
		    case 'e':
		      clan_table[clan].ctype = CLAN_TYPE_EXPL;
		      break;
		    case 'T':
		    case 't':
		      clan_table[clan].ctype = CLAN_TYPE_TECH;
		      break;
		    case 'R':
		    case 'f':
		      clan_table[clan].ctype = CLAN_TYPE_REL;
		      break;
		    case 'D':
		    case 'd':
		      clan_table[clan].ctype = CLAN_TYPE_DEM;
		      break;
		    default:
		      clan_table[clan].ctype = CLAN_BOGUS;
		      bug ("low_load_clanfile() the clan type was bad.", 0);
		      break;
		    }
		  break;
		case 'L':
		case 'l':	/* clan levels */
		  line = find_sepchar (line);
		  line = strip_whitespace (&(line[1]));
		  if (sscanf (line, "%d", &(clan_table[clan].levels)) != 1)
		    {
		      clan_table[clan].levels = CLAN_BOGUS;
		      bug ("low_load_clanfile() the clan level was bad.", 0);
		    }
		  break;
		case 'M':
		case 'm':	/* clan membership */
		  if (!str_cmp ("Members:", line))
		    {
		      clan_table[clan].members = low_load_members (cfp);

		      // fix up the leader record
		      if (clan_table[clan].members != NULL)
			{
			  m = clan_table[clan].members;
			  while (m && (m->status != CLAN_LEADER))
			    m = m->next;
			  if (m->status == CLAN_LEADER)
			    {
			      clan_table[clan].leader = m;
			    }

			  else
			    {

			      // someone hosed the clan leader, first guy is now it
			      clan_table[clan].leader =
				clan_table[clan].members;
			      clan_table[clan].leader->status = CLAN_LEADER;
			    }
			}
		    }

		  else
		    {
		      bug ("low_load_clanfile() the members list is bad.", 0);
		    }
		  break;
		case 'A':
		case 'a':	/* clan applicants or clan alignment */
		  if ((line[1] == 'L') || (line[1] == 'l'))
		    {
		      line = find_sepchar (line);
		      line = strip_whitespace (&(line[1]));
		      if ((line[0] == 'G') || (line[0] == 'g'))
			{
			  clan_table[clan].align = CLAN_GOOD_ALIGN;
			}

		      else if ((line[0] == 'E') || (line[0] == 'e'))
			{
			  clan_table[clan].align = CLAN_EVIL_ALIGN;
			}

		      else if ((line[0] == 'N') || (line[0] == 'n'))
			{
			  if ((line[1] == 'O') || (line[1] == 'o'))
			    {
			      clan_table[clan].align = CLAN_NOALIGN;
			    }

			  else if ((line[1] == 'E') || (line[1] == 'e'))
			    {
			      clan_table[clan].align = CLAN_NEUTRAL;
			    }

			  else
			    {
			      bug
				("low_load_clanfile() clan alignment value bad.",
				 0);
			      clan_table[clan].align = CLAN_NOALIGN;
			    }
			}

		      else
			{
			  bug ("low_load_clanfile() clan alignment tag bad.",
			       0);
			  clan_table[clan].align = CLAN_NOALIGN;
			}
		    }

		  else if ((line[1] == 'P') || (line[1] == 'p'))
		    {
		      if (!str_cmp ("Applicants:", line))
			{
			  clan_table[clan].applicants =
			    low_load_members (cfp);
			}

		      else
			{
			  bug
			    ("low_load_clanfile() the applicant list is bad.",
			     0);
			}
		    }

		  else
		    {
		      bug ("low_load_clanfile() bogus 'a' tag.", 0);
		    }
		  break;
		case 'R':
		case 'r':	/* clan relations */
		  if (!str_cmp ("Relations:", line))
		    {
		      clan_table[clan].relations = low_load_relations (cfp);
		    }

		  else
		    {
		      bug ("low_load_clanfile() the relations list is bad.",
			   0);
		    }
		  break;
		default:
		  bug ("low_load_clanfile() bogus tag.", 0);
		  break;
		}		// end the switch( line[0] )
	    }
	}
    }				// end while
}

RELATIONS *low_load_relations (FILE * cfp)
{
  RELATIONS *list = NULL;
  RELATIONS *last = NULL;
  RELATIONS *r = NULL;
  char linebuf[MAX_STRING_LENGTH];
  char *line = NULL;
  bool end_found = FALSE;
  int pos = 0;
  int items = 0;
  while (!end_found && (fgets (linebuf, MAX_STRING_LENGTH, cfp) != NULL))
    {

      // remove the newline character
      linebuf[strlen (linebuf) - 1] = '\0';
      line = strip_whitespace (linebuf);

      // comment line?
      if ((line[0] != '#') && (line[0] != '\0'))
	{

	  // at the end of this set?
	  if (!str_cmp ("End", line))
	    {
	      end_found = TRUE;
	    }

	  else
	    {
	      r = new_relations_elt ();
	      if (r == NULL)
		{
		  bug ("low_load_relations() Memory allocation failed", 0);
		  return (NULL);
		}

	      else
		{

		  // chop off the little bit of formatting
		  line = &(line[strlen ("Relate:")]);
		  line = strip_whitespace (line);
		  pos = 0;

		  // copy the clan name into the record
		  while ((line[pos] != ':') && (line[pos] != '\0')
			 && (pos < MAX_NAME))
		    {
		      r->name[pos] = line[pos];
		      pos++;
		    }

		  // remember to terminate the string
		  r->name[pos] = '\0';

		  // check for some possible error conditions
		  if (pos == MAX_NAME)
		    {

		      // ok, someone edited this file and blew it
		      bug ("low_load_relations() clan name too long.", 0);
		    }
		  // find that colon separator
		  while ((line[pos] != ':') && (line[pos] != '\0'))
		    pos++;

		  // check that we didn't run out of data
		  if ((line[pos] == '\0'))
		    {

		      // this line is hosed, dump this record and try to recover
		      // by continuing in the loop by reading the next line
		      bug ("low_load_relations() bogus clan relation.", 0);
		      free (r);
		      continue;
		    }
		  items =
		    sscanf (&(line[pos]),
			    ":%d:%d:%ld:%ld:%d:%d:%d:%d:%d",
			    &(r->ident), &(r->status), &(r->acttime),
			    &(r->wartime), &(r->warcount), &(r->loss),
			    &(r->wins), &(r->kills), &(r->points));

		  // ok, there better be 9 items here or there is a problem
		  if (items != 9)
		    {

		      // now, this line is hosed, dump this record and try to recover
		      // by continuing in the loop by reading the next line
		      bug ("low_load_relations() wrong number of clan items.",
			   0);
		      free (r);
		      continue;
		    }

/*
		  fixup - on a load all clan wars are ended
		  if (r->status >= CLAN_WAR)
		    r->status = CLAN_ENEMY;
*/
		  // ahhh, we made it, now append the record to the list
		  if (last == NULL)
		    {
		      list = r;
		      last = r;
		    }

		  else
		    {
		      last->next = r;
		      last = r;
		    }
		}
	    }
	}
    }
  return (list);
}

MEMBER *low_load_members (FILE * cfp)
{
  MEMBER *list = NULL;
  MEMBER *last = NULL;
  MEMBER *m = NULL;
  char linebuf[MAX_STRING_LENGTH];
  char *line = NULL;
  bool end_found = FALSE;
  int pos = 0;
  int items = 0;
  while (!end_found && (fgets (linebuf, MAX_STRING_LENGTH, cfp) != NULL))
    {

      // remove the newline character
      linebuf[strlen (linebuf) - 1] = '\0';
      line = strip_whitespace (linebuf);

      // comment line?
      if ((line[0] != '#') && (line[0] != '\0'))
	{

	  // at the end of this set?
	  if (!str_cmp ("End", line))
	    {
	      end_found = TRUE;
	    }

	  else
	    {
	      m = new_member_elt ();
	      if (m == NULL)
		{
		  bug ("low_load_members() Memory allocation failed", 0);
		  return (NULL);
		}

	      else
		{

		  // chop off the little bit of formatting
		  line = &(line[strlen ("Member:")]);
		  line = strip_whitespace (line);
		  pos = 0;

		  // copy the name into the member record
		  while ((line[pos] != ':') && (line[pos] != '\0')
			 && (pos < MAX_NAME))
		    {
		      m->name[pos] = line[pos];
		      pos++;
		    }

		  // remember to terminate the string
		  m->name[pos] = '\0';

		  // check for some possible error conditions
		  if (pos == MAX_NAME)
		    {

		      // ok, someone edited this file and blew it
		      bug ("low_load_members() clan member name too long.",
			   0);
		    }
		  // find that colon separator
		  while ((line[pos] != ':') && (line[pos] != '\0'))
		    pos++;

		  // check that we didn't run out of data
		  if ((line[pos] == '\0'))
		    {

		      // this line is hosed, dump this record and try to recover
		      // by continuing in the loop by reading the next line
		      bug ("low_load_members() bogus clan member.", 0);
		      free (m);
		      continue;
		    }
		  items =
		    sscanf (&(line[pos]), ":%d:%d:%d:%d:%d:%ld:%d",
			    &(m->levels), &(m->status), &(m->align),
			    &(m->race), &(m->Class), &(m->laston),
			    &(m->initiative));

		  // ok, there better be four items here or there is a problem
		  if (items != 7)
		    {

		      // now, this line is hosed, dump this record and try to recover
		      // by continuing in the loop by reading the next line
		      bug ("low_load_members() wrong number of member items.",
			   0);
		      free (m);
		      continue;
		    }
		  // ahhh, we made it, now append the record to the list
		  if (last == NULL)
		    {
		      list = m;
		      last = m;
		    }

		  else
		    {
		      last->next = m;
		      last = m;
		    }
		}
	    }
	}
    }
  return (list);
}


// ==============================================================
//
//   Private Initialization of the clan_table
//
// ==============================================================
void init_clan_table ()
{
  int i;

  // initialize slot zero in the clan table
  // slot zero is always reserved for the bogus clan
  // and the CLAN_BOGUS identifier
  strcpy (clan_table[CLAN_BOGUS].name, CLAN_BOGUS_NAME);
  strcpy (clan_table[CLAN_BOGUS].symbol, CLAN_BOGUS_SYMB);
  clan_table[CLAN_BOGUS].ctype = CLAN_TYPE_TEST;
  clan_table[CLAN_BOGUS].ident = CLAN_RES_IDENT_MIN;
  clan_table[CLAN_BOGUS].hall = ROOM_VNUM_ALTAR;
  clan_table[CLAN_BOGUS].status = CLAN_INACTIVE;
  clan_table[CLAN_BOGUS].levels = CLAN_BOGUS;
  clan_table[CLAN_BOGUS].align = CLAN_BOGUS;
  clan_table[CLAN_BOGUS].leader = NULL;
  clan_table[CLAN_BOGUS].members = NULL;
  clan_table[CLAN_BOGUS].applicants = NULL;
  clan_table[CLAN_BOGUS].relations = NULL;
  clan_table[CLAN_BOGUS].clan_box = NULL;

  // mark all other clan status as bogus
  for (i = CLANS_START; i < MAX_CLAN; i++)
    clan_table[i].status = CLAN_BOGUS;
}
