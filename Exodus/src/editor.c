#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "olc.h"
char *string_replace (CHAR_DATA * ch, char *orig, char *old, char *newstr)
{
  char ybuf[MAX_STRING_LENGTH];
  char xbuf[MAX_STRING_LENGTH], *cptr, *bufptr;
  int count = 0;
  if (strlen (old) <= 0)
    {
      send_to_char ("Error: No text given to replace.\n\r", ch);
      return (orig);
    }
    
  xbuf[0] = '\0';
  strcpy (ybuf, orig);
  bufptr = ybuf;
  while ((cptr = strstr (bufptr, old)) != NULL)
    {
      *cptr = '\0';
      strcat (xbuf, bufptr);
      smash_tilde(newstr);
      strcat (xbuf, newstr);
      bufptr = cptr + strlen (old);
      count++;
    }
  strcat (xbuf, bufptr);
  free_string (orig);
  if (count == 0)
    send_to_char ("No occurences found.\n\r\n\r", ch);

  else
    {
      sprintf (ybuf, "%d occurence%s found (%s => %s).\n\r\n\r", count,
	       count == 1 ? "" : "s", old, newstr);
      send_to_char (ybuf, ch);
    }
  return str_dup (xbuf);
}

void string_add (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  smash_tilde (argument);
  if (ch->desc->editor == EDIT_LINES)
    {
      if (*argument == ' ' && *(argument + 1) == '\0')
	{
	  ch->desc->editor = EDIT_MENU;
	  return;
	}
    }
  if (argument[0] == '.' && argument[1] == 'f' && argument[2] == '\0')
    {
      *ch->desc->pString = format_string (*ch->desc->pString);
      send_to_char ("String formatted.\n\r", ch);
      return;
    }
  strcpy (buf, *ch->desc->pString);
  if (strlen (buf) + strlen (argument) >= (MAX_STRING_LENGTH - 4))
    {
      send_to_char ("String too long, previous line removed.\n\r", ch);
      ch->desc->pString = NULL;
      return;
    }
  smash_tilde (argument);
  strcat (buf, argument);
  strcat (buf, "\n\r");
  free_string (*ch->desc->pString);
  *ch->desc->pString = str_dup (buf);
  return;
}

char *format_string (char *oldstring /*, bool fSpace */ )
{
  char xbuf[MAX_STRING_LENGTH];
  char xbuf2[MAX_STRING_LENGTH];
  char *rdesc;
  int i = 0;
  bool cap = TRUE;
  xbuf[0] = xbuf2[0] = 0;
  i = 0;
  for (rdesc = oldstring; *rdesc; rdesc++)
    {
      if (*rdesc == '\n')
	{
	  if (xbuf[i - 1] != ' ')
	    {
	      xbuf[i] = ' ';
	      i++;
	    }
	}
      else if (*rdesc == '\r');

      else if (*rdesc == ' ')
	{
	  if (xbuf[i - 1] != ' ')
	    {
	      xbuf[i] = ' ';
	      i++;
	    }
	}
      else if (*rdesc == ')')
	{
	  if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' ' &&
	      (xbuf[i - 3] == '.' || xbuf[i - 3] == '?'
	       || xbuf[i - 3] == '!'))
	    {
	      xbuf[i - 2] = *rdesc;
	      xbuf[i - 1] = ' ';
	      xbuf[i] = ' ';
	      i++;
	    }
	  else
	    {
	      xbuf[i] = *rdesc;
	      i++;
	    }
	}
      else if (*rdesc == '.' || *rdesc == '?' || *rdesc == '!')
	{
	  if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' ' &&
	      (xbuf[i - 3] == '.' || xbuf[i - 3] == '?'
	       || xbuf[i - 3] == '!'))
	    {
	      xbuf[i - 2] = *rdesc;
	      if (*(rdesc + 1) != '\"')
		{
		  xbuf[i - 1] = ' ';
		  xbuf[i] = ' ';
		  i++;
		}
	      else
		{
		  xbuf[i - 1] = '\"';
		  xbuf[i] = ' ';
		  xbuf[i + 1] = ' ';
		  i += 2;
		  rdesc++;
		}
	    }
	  else
	    {
	      xbuf[i] = *rdesc;
	      if (*(rdesc + 1) != '\"')
		{
		  xbuf[i + 1] = ' ';
		  xbuf[i + 2] = ' ';
		  i += 3;
		}
	      else
		{
		  xbuf[i + 1] = '\"';
		  xbuf[i + 2] = ' ';
		  xbuf[i + 3] = ' ';
		  i += 4;
		  rdesc++;
		}
	    }
	  cap = TRUE;
	}
      else
	{
	  xbuf[i] = *rdesc;
	  if (cap)
	    {
	      cap = FALSE;
	      xbuf[i] = UPPER (xbuf[i]);
	    }
	  i++;
	}
    }
  xbuf[i] = 0;
  strcpy (xbuf2, xbuf);
  rdesc = xbuf2;
  xbuf[0] = 0;
  for (;;)
    {
      for (i = 0; i < 77; i++)
	{
	  if (!*(rdesc + i))
	    break;
	}
      if (i < 77)
	{
	  break;
	}
      for (i = (xbuf[0] ? 76 : 73); i; i--)
	{
	  if (*(rdesc + i) == ' ')
	    break;
	}
      if (i)
	{
	  *(rdesc + i) = 0;
	  strcat (xbuf, rdesc);
	  strcat (xbuf, "\n\r");
	  rdesc += i + 1;
	  while (*rdesc == ' ')
	    rdesc++;
	}
      else
	{
	  bug ("No spaces", 0);
	  *(rdesc + 75) = 0;
	  strcat (xbuf, rdesc);
	  strcat (xbuf, "-\n\r");
	  rdesc += 76;
	}
    }
  while (*(rdesc + i)
	 && (*(rdesc + i) == ' ' || *(rdesc + i) == '\n'
	     || *(rdesc + i) == '\r'))
    i--;
  *(rdesc + i + 1) = 0;
  strcat (xbuf, rdesc);
  if (xbuf[strlen (xbuf) - 2] != '\n')
    strcat (xbuf, "\n\r");
  free_string (oldstring);
  return (str_dup (xbuf));
}

char *first_arg (char *argument, char *arg_first, bool fCase)
{
  char cEnd;
  while (*argument == ' ')
    argument++;
  cEnd = ' ';
  if (*argument == '\'' || *argument == '"'
      || *argument == '%' || *argument == '(')
    {
      if (*argument == '(')
	{
	  cEnd = ')';
	  argument++;
	}
      else
	cEnd = *argument++;
    }
  while (*argument != '\0')
    {
      if (*argument == cEnd)
	{
	  argument++;
	  break;
	}
      if (fCase)
	*arg_first = LOWER (*argument);

      else
	*arg_first = *argument;
      arg_first++;
      argument++;
    }
  *arg_first = '\0';
  while (*argument == ' ')
    argument++;
  return argument;
}

bool show_string_sofar (CHAR_DATA * ch, bool page, bool line_num)
{
  char tmp_buf[MAX_STRING_LENGTH * 2];
  char buf[4];
  char *cptr;
  int count = 0;
  int line = 0;
  cptr = *ch->desc->pString;
  memset (tmp_buf, 0, MAX_STRING_LENGTH * 2);
  if (*cptr != '\0')
    {
      if (line_num)
	{
	  sprintf (buf, "%d", line);
	  strcat (tmp_buf, buf);
	  count += strlen (buf);
	}
      tmp_buf[count] = ']';
      count++;
      tmp_buf[count] = ' ';
      count++;
    }
  while (*cptr != '\0')
    {
      if (*cptr == '\r')
	{
	  tmp_buf[count] = '\r';
	  count++;
	  if (line_num && *(cptr + 1) != '\0')
	    {
	      line++;
	      sprintf (buf, "%d", line);
	      strcat (tmp_buf, buf);
	      count += strlen (buf);
	    }
	  tmp_buf[count] = ']';
	  count++;
	  tmp_buf[count] = ' ';
	}
      else
	tmp_buf[count] = *cptr;
      count++;
      cptr++;
    }
  if (count == 0)
    return (FALSE);
  tmp_buf[count - 2] = '\0';
  if (page)
    page_to_char (tmp_buf, ch);

  else
    send_to_char (tmp_buf, ch);
  return (TRUE);
}

void edit_line_number (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], line[MAX_STRING_LENGTH * 3];
  char buf[MAX_STRING_LENGTH];
  char *cptr, *cptr2;
  int num, linenum = 0;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      ch->desc->editor = EDIT_MENU;
      return;
    }
  num = atoi (arg);
  line[0] = 1;
  cptr = *ch->desc->pString;
  while (*cptr != '\0')
    {
      if (linenum == num)
	{
	  strcpy (line, cptr);
	  if ((cptr2 = strchr (line, '\n')) != NULL)
	    *cptr2 = '\0';
	  break;
	}
      if (*cptr == '\r')
	linenum++;
      cptr++;
    }
  if (line[0] == 1)
    {
      if (linenum == 1)
	send_to_char ("Line not within valid range (0).\n\r", ch);

      else
	{
	  sprintf (buf, "Line not within valid range (0 to %d).",
		   linenum - 1);
	  send_to_char (buf, ch);
	}
      return;
    }
  ch->desc->door_dir = linenum;
  ch->desc->submit_info = str_dup (line);
  ch->desc->editor = EDIT_NEW_LINE;
}

void insert_line_number (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], line[MAX_STRING_LENGTH * 3];
  char buf[MAX_STRING_LENGTH];
  char *cptr, *cptr2;
  int num, linenum = 0;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      ch->desc->editor = EDIT_MENU;
      return;
    }
  num = atoi (arg);
  line[0] = 1;
  cptr = *ch->desc->pString;
  while (*cptr != '\0')
    {
      if (linenum == num)
	{
	  strcpy (line, cptr);
	  if ((cptr2 = strchr (line, '\n')) != NULL)
	    *cptr2 = '\0';
	  break;
	}
      if (*cptr == '\r')
	linenum++;
      cptr++;
    }
  if (line[0] == 1)
    {
      if (linenum == 1)
	send_to_char ("Line not within valid range (0).\n\r", ch);

      else
	{
	  sprintf (buf, "Line not within valid range (0 to %d).",
		   linenum - 1);
	  send_to_char (buf, ch);
	}
      return;
    }
  ch->desc->door_dir = linenum;
  ch->desc->submit_info = str_dup (line);
  ch->desc->editor = INSERT_NEW_LINE;
}

void dele_line_number (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], line[MAX_STRING_LENGTH * 3];
  char buf[MAX_STRING_LENGTH];
  char *cptr, *cptr2;
  int num, linenum = 0, count = 0;
  bool found = FALSE;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      ch->desc->editor = EDIT_MENU;
      return;
    }
  num = atoi (arg);
  memset (line, 0, MAX_STRING_LENGTH * 3);
  cptr = *ch->desc->pString;
  while (*cptr != '\0')
    {
      if (linenum == num)
	{
	  if ((cptr2 = strchr (cptr, '\n')) != NULL)
	    {
	      if (linenum == 0)
		{
		  while (*cptr2 == '\n' || *cptr2 == '\r')
		    cptr2++;
		}
	      else
		cptr2++;
	      strcat (line, cptr2);
	    }
	  found = TRUE;
	  break;
	}
      else
	{
	  line[count] = *cptr;
	  count++;
	}
      if (*cptr == '\n')
	linenum++;
      cptr++;
    }
  if (!found)
    {
      if (linenum == 1)
	send_to_char ("Line not within valid range (0).\n\r", ch);

      else
	{
	  sprintf (buf, "Line not within valid range (0 to %d).",
		   linenum - 1);
	  send_to_char (buf, ch);
	}
      return;
    }
  free_string (*ch->desc->pString);
  *ch->desc->pString = str_dup (line);
  sprintf (buf, "Line %d has been deleted.\n\r\n\r", num);
  send_to_char (buf, ch);
  ch->desc->editor = EDIT_MENU;
}

void edit_new_line (CHAR_DATA * ch, char *argument)
{
  char bbuf[MAX_STRING_LENGTH * 4];
  char *cptr, *cptr2;
  int num, count = 0, linenum = 0;
  num = ch->desc->door_dir;
  memset (bbuf, 0, MAX_STRING_LENGTH * 4);
  cptr = *ch->desc->pString;
  while (*cptr != '\0')
    {
      if (linenum == num)
	{
	  cptr2 = strchr (cptr, '\n');
	  strcat (bbuf, argument);
	  if (cptr2 != NULL)
	    strcat (bbuf, cptr2);
	  break;
	}
      else
	{
	  bbuf[count] = *cptr;
	  count++;
	}
      if (*cptr == '\r')
	linenum++;
      cptr++;
    }
  free_string (*ch->desc->pString);
  smash_tilde(bbuf);
  *ch->desc->pString = str_dup (bbuf);
  ch->desc->editor = EDIT_MENU;
}

void search_replace (CHAR_DATA * ch, char *argument)
{
  char search[MAX_INPUT_LENGTH];
  argument = one_argument_nl (argument, search);
  *ch->desc->pString =
    string_replace (ch, *ch->desc->pString, search, argument);
  ch->desc->editor = EDIT_MENU;
}

void insert_new_line (CHAR_DATA * ch, char *argument)
{
  char bbuf[MAX_STRING_LENGTH * 4];
  char *cptr;
  int num, count = 0, linenum = 0;
  num = ch->desc->door_dir;
  memset (bbuf, 0, MAX_STRING_LENGTH * 4);
  cptr = *ch->desc->pString;
  while (*cptr != '\0')
    {
      if (linenum == num)
	{
	  strcat (bbuf, argument);
	  strcat (bbuf, "\n\r");
	  strcat (bbuf, cptr);
	  break;
	}
      else
	{
	  bbuf[count] = *cptr;
	  count++;
	}
      if (*cptr == '\r')
	linenum++;
      cptr++;
    }
  free_string (*ch->desc->pString);
  *ch->desc->pString = str_dup (bbuf);
  ch->desc->editor = EDIT_MENU;
}

void edit_menu_display (CHAR_DATA * ch)
{
  send_to_char
    ("`g:`e----------------------------------------------------------------------------`g:``\n\r\n\r",
     ch);
  send_to_char
    ("               `l[`nA`l]`hppend Lines             `l[`nS`l]`have and Exit\n\r",
     ch);
  send_to_char
    ("               `l[`nI`l]`hnsert Line              `l[`nU`l]`hndo Edits and Exit\n\r",
     ch);
  send_to_char
    ("               `l[`nY`l]`hank Line                `l[`nJ`l]`hustify Text\n\r",
     ch);
  send_to_char
    ("               `l[`nD`l]`helete All Text          `l[`nV`l]`hiew Current Text\n\r",
     ch);
  send_to_char
    ("               `l[`nM`l]`henu Display             `l[`nH`l]`help Display\n\r",
     ch);
  send_to_char
    ("               `l[`nL`l]`hine Replace             `l[`nR`l]`heplace String\n\r",
     ch);
  send_to_char ("               `l[`nC`l]`hheck Spelling\n\r\n\r", ch);
  send_to_char
    ("`g:`e----------------------------------------------------------------------------`g:``\n\r\n\r\n\r",
     ch);
}

void edit_menu (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  one_argument (argument, arg);
  if (!str_cmp (arg, "s"))
    {				/* Save and Exit */
      send_to_char ("Saving new text and exiting...\n\r\n\r", ch);
      ch->desc->pString = NULL;
      ch->desc->editor = ch->desc->old_editor;
      return;
    }
  if (!str_cmp (arg, "m"))
    {				/* Menu Display */
      edit_menu_display (ch);
      return;
    }
  if (!str_cmp (arg, "a"))
    {				/* Append Lines */
      send_to_char
	("\n\rEnter new lines of text.  Press RETURN on an otherwise blank line when finished.\n\r",
	 ch);
      show_string_sofar (ch, FALSE, FALSE);
      ch->desc->editor = EDIT_LINES;
      return;
    }
  if (!str_cmp (arg, "j"))
    {				/* Justify Text */
      *ch->desc->pString = format_string (*ch->desc->pString);
      send_to_char ("The text has been justified.\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "l"))
    {				/* Line Replace */
      send_to_char ("\n\r", ch);
      if (!show_string_sofar (ch, TRUE, TRUE))
	{
	  send_to_char ("There is currently no text to replace.\n\r", ch);
	  return;
	}
      ch->desc->editor = EDIT_LINE_NUMBER;
      return;
    }
  if (!str_cmp (arg, "i"))
    {				/* Insert line */
      send_to_char ("\n\r", ch);
      if (!show_string_sofar (ch, TRUE, TRUE))
	{
	  send_to_char ("There is currently no text to insert here.\n\r", ch);
	  return;
	}
      ch->desc->editor = INSERT_LINE_NUMBER;
      return;
    }
  if (!str_cmp (arg, "d"))
    {				/* Delete all text */
      send_to_char ("All current text has been deleted.\n\r", ch);
      **ch->desc->pString = '\0';
      return;
    }
  if (!str_cmp (arg, "r"))
    {				/* Replace String */
      ch->desc->editor = SEARCH_REPLACE;
      return;
    }
  if (!str_cmp (arg, "v"))
    {				/* View Current */
      send_to_char ("Current Text:\n\r\n\r", ch);
      show_string_sofar (ch, TRUE, FALSE);
      send_to_char ("\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "c"))
    {
      send_to_char
	("Spell Check is not yet implemented.  Please try again in the future.\n\r",
	 ch);
      return;
    }
  if (!str_cmp (arg, "y"))
    {				/* Yank line */
      send_to_char ("\n\r", ch);
      if (!show_string_sofar (ch, TRUE, TRUE))
	{
	  send_to_char ("There is currently no text to yank.\n\r", ch);
	  return;
	}
      ch->desc->editor = DELE_LINE_NUMBER;
      return;
    }
  if (!str_cmp (arg, "u"))
    {				/* Undo */
      free_string (*ch->desc->pString);
      *ch->desc->pString = str_dup (ch->desc->door_name);
      free_string (ch->desc->door_name);
      ch->desc->pString = NULL;
      ch->desc->editor = ch->desc->old_editor;
      send_to_char ("\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "h"))
    {
      send_to_char
	("The help function for this editor has not yet been created.\n\r",
	 ch);
      return;
    }
  send_to_char ("Command not understood.\n\r", ch);
}

void edit_string (CHAR_DATA * ch, char **pString)
{
  edit_menu_display (ch);
  if (*pString == NULL)
    {
      *pString = str_dup ("");
    }
  ch->desc->pString = pString;
  if (ch->desc->door_name != NULL)
    free_string (ch->desc->door_name);
  ch->desc->door_name = str_dup (*pString);
  ch->desc->old_editor = ch->desc->editor;
  ch->desc->editor = EDIT_MENU;
}
