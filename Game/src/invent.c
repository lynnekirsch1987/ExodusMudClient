//Iblis - Minax's Idea, implemented (however crappily) by me

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"

/* Global variables */
INVENTION_DATA *invention_list = NULL;

//Iblis - saves the list of invention combos to a file
void save_inventions (void)
{
  FILE *fp;
  INVENTION_DATA *invention;
  bool found = FALSE;
  fclose (fpReserve);
  if ((fp = fopen (INVENTION_FILE, "w")) == NULL)
    {
      perror (INVENTION_FILE);
    }
  for (invention = invention_list; invention != NULL;
       invention = invention->next)
    {
      found = TRUE;
      fprintf (fp, "%d %d %d\n", invention->vnum1, invention->vnum2,
	       invention->vnum3);
    }
  fclose (fp);
  fpReserve = fopen (NULL_FILE, "r");
  if (!found)
    unlink (INVENTION_FILE);
}

//Iblis - the actual function used by thieves to combine items for the invention skill
void do_mix (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH],
    arg3[MAX_STRING_LENGTH];
  INVENTION_DATA *id = NULL, *id2 = NULL, *newid = NULL;
  OBJ_DATA *obj1 = NULL, *obj2 = NULL, *obj3 = NULL;
  int vnum1, vnum2, vnum3;
  char buf[MAX_STRING_LENGTH];
  OBJ_INDEX_DATA *pObj = NULL;
  if (get_skill (ch, gsn_invention) <= 0)
    {
      send_to_char
	("You shouldn't mix things if you don't know what you are doing.\r\n",
	 ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("Syntax : mix item with item\r\n", ch);
      if (IS_IMMORTAL (ch))
	send_to_char
	  (" - mix add vnum1 vnum2 vnumresult\r\n - mix delete vnum1 vnum2 vnumresult\r\n - mix save\r\n",
	   ch);
      return;
    }
  argument = one_argument (argument, arg1);
  if (!strcmp (arg1, "list") && IS_IMMORTAL (ch))
    {
      for (id = invention_list; id->next != NULL; id = id->next)
	{
	  sprintf (buf, "%d %d %d | ", id->vnum1, id->vnum2, id->vnum3);
	  send_to_char (buf, ch);
	}
      send_to_char ("\r\n", ch);
      return;
    }
  if (!strcmp (arg1, "add"))
    {
      if (IS_IMMORTAL (ch))
	{
	  if (argument[0] != '\0')
	    argument = one_argument (argument, arg1);
	  if (argument[0] != '\0')
	    argument = one_argument (argument, arg2);
	  if (argument[0] != '\0')
	    argument = one_argument (argument, arg3);
	  if (arg1 == NULL || arg2 == NULL || arg3 == NULL)
	    {
	      send_to_char
		("Syntax : mix add vnum1 vnum2 vnumresult\r\n", ch);
	      return;
	    }
	  if (is_number (arg1))
	    {
	      vnum1 = atoi (arg1);
	      if (!(pObj = get_obj_index (vnum1)))
		{
		  send_to_char ("The 1st vnum does not exist.\n\r", ch);
		  return;
		}
	      if (is_number (arg2))
		{
		  vnum2 = atoi (arg2);
		  if (!(pObj = get_obj_index (vnum2)))
		    {
		      send_to_char ("The 2nd vnum does not exist.\n\r", ch);
		      return;
		    }
		  if (is_number (arg3))
		    {
		      vnum3 = atoi (arg3);
		      if (!(pObj = get_obj_index (vnum3)))
			{
			  send_to_char
			    ("The 3rd vnum does not exist.\n\r", ch);
			  return;
			}
		      if (invention_list == NULL)
			{
			  newid = new_invention ();
			  newid->vnum1 = vnum1;
			  newid->vnum2 = vnum2;
			  newid->vnum3 = vnum3;
			  newid->next = NULL;
			  send_to_char
			    ("You have added a new mixture to the Alchemy table.\r\n",
			     ch);
			  invention_list = newid;
			}
		      else
			{
			  for (id = invention_list; id->next != NULL;
			       id = id->next)
			    if (id->vnum1 == vnum1 && id->vnum2 == vnum2)
			      {
				sprintf (arg1,
					 "There is already a mixture of %d and %d, resulting in %d",
					 id->vnum1, id->vnum2, id->vnum3);
				send_to_char (arg1, ch);
				return;
			      }
			  newid = new_invention ();
			  newid->vnum1 = vnum1;
			  newid->vnum2 = vnum2;
			  newid->vnum3 = vnum3;
			  newid->next = NULL;	//invention_list;
//                      invention_list = newid;
			  id->next = newid;
			  send_to_char
			    ("You have added a new mixture to the Alchemy table.\r\n",
			     ch);
			  return;
			}
		    }
		}
	    }
	  send_to_char ("Syntax : mix add vnum1 vnum2 vnumresult.\r\n", ch);
	  return;
	}
    }
  if (!strcmp (arg1, "save"))
    if (IS_IMMORTAL (ch))
      {
	save_inventions ();
	send_to_char ("Invention list saved.\r\n", ch);
	return;
      }
  if (!strcmp (arg1, "delete"))
    if (IS_IMMORTAL (ch))
      {
	if (argument[0] != '\0')
	  argument = one_argument (argument, arg1);
	if (argument[0] != '\0')
	  argument = one_argument (argument, arg2);
	if (argument[0] != '\0')
	  argument = one_argument (argument, arg3);
	if (arg1[0] == '\0' || arg2 == '\0' || arg3 == '\0')
	  {
	    send_to_char ("Syntax : mix add vnum1 vnum2 vnumresult.", ch);
	    return;
	  }
	if (is_number (arg1))
	  {
	    vnum1 = atoi (arg1);
	    if (!(pObj = get_obj_index (vnum1)))
	      {
		send_to_char ("The 1st vnum does not exist.\n\r", ch);
		return;
	      }
	    if (is_number (arg2))
	      {
		vnum2 = atoi (arg2);
		if (!(pObj = get_obj_index (vnum2)))
		  {
		    send_to_char ("The 2nd vnum does not exist.\n\r", ch);
		    return;
		  }
		if (is_number (arg3))
		  {
		    vnum3 = atoi (arg3);
		    if (!(pObj = get_obj_index (vnum3)))
		      {
			send_to_char ("The 3rd vnum does not exist.\n\r", ch);
			return;
		      }
		    if (invention_list == NULL)
		      {
			send_to_char
			  ("There are no mixtures to remove.\r\n", ch);
			return;
		      }
		    if (invention_list != NULL)
		      {
			if ((invention_list->vnum1 == vnum1
			     && invention_list->vnum2 == vnum2
			     && invention_list->vnum3 == vnum3)
			    || (invention_list->vnum1 == vnum2
				&& invention_list->vnum2 == vnum1
				&& invention_list->vnum3 == vnum3))
			  {
			    id2 = invention_list;

			    invention_list = id2->next;
			    free_invention (id2);
			    send_to_char
			      ("You have removed a mixture from the invention table.\r\n",
			       ch);
			    return;
			  }
			for (id = invention_list; id->next != NULL;
			     id = id->next)
			  {
			    if ((id->next->vnum1 ==
				 vnum1 && id->next->vnum2 ==
				 vnum2 && id->next->vnum3 ==
				 vnum3) || (id->next->vnum1 ==
					    vnum2 && id->next->
					    vnum2 ==
					    vnum1 && id->next->
					    vnum3 == vnum3))
			      {
				id2 = id->next;
				id->next = id->next->next;
				free_invention (id2);
				send_to_char
				  ("You have removed a mixture from the invention table.\r\n",
				   ch);
				return;
			      }
			  }
		      }
		  }
	      }
	  }
	send_to_char ("Syntax : mix add vnum1 vnum2 vnumresult.\r\n", ch);
	return;
      }

  if (argument[0] != '\0')
    argument = one_argument (argument, arg2);

  else
    {
      send_to_char ("Syntax : mix item with item.\r\n", ch);
      return;
    }
  if (strcmp (arg2, "with"))
    {
      send_to_char ("Syntax : mix item with item.\r\n", ch);
      return;
    }
  if (argument[0] != '\0')
    argument = one_argument (argument, arg2);

  else
    {
      send_to_char ("Syntax : mix item with item.\r\n", ch);
      return;
    }

  if (((obj1 = get_obj_carry (ch, arg1)) == NULL)
      || (obj2 = get_obj_carry (ch, arg2)) == NULL)
    {
      send_to_char ("You can only mix two items you actually have.\r\n", ch);
      return;
    }
  if (number_percent () >= get_skill (ch, gsn_invention))
    {
      send_to_char
	("Your inventing skills appear to need a little work.\r\n", ch);
      return;
    }
  else
    {
      if (invention_list != NULL)
	{
	  if ((invention_list->vnum1 == obj1->pIndexData->vnum
	       && invention_list->vnum2 == obj2->pIndexData->vnum)
	      || (invention_list->vnum1 == obj2->pIndexData->vnum
		  && invention_list->vnum2 == obj1->pIndexData->vnum))
	    {
	      obj3 = create_object (get_obj_index (invention_list->vnum3), 0);
	      sprintf (buf, "You mix %s with %s to create %s.\r\n",
		       obj1->short_descr, obj2->short_descr,
		       obj3->short_descr);
	      send_to_char (buf, ch);
	      sprintf (buf, "$n mixes %s with %s to create %s.\r\n",
		       obj1->short_descr, obj2->short_descr,
		       obj3->short_descr);
	      act (buf, ch, NULL, NULL, TO_ROOM);
	      extract_obj (obj1);
	      extract_obj (obj2);
	      obj_to_char (obj3, ch);
	      return;
	    }
	  for (id = invention_list; id != NULL; id = id->next)
	    {
	      if ((id->vnum1 == obj1->pIndexData->vnum
		   && id->vnum2 == obj2->pIndexData->vnum)
		  || (id->vnum1 == obj2->pIndexData->vnum
		      && id->vnum2 == obj1->pIndexData->vnum))
		{
		  obj3 = create_object (get_obj_index (id->vnum3), 0);
		  sprintf (buf, "You mix %s with %s to create %s.\r\n",
			   obj1->short_descr, obj2->short_descr,
			   obj3->short_descr);
		  send_to_char (buf, ch);
		  sprintf (buf, "$n mixes %s with %s to create %s.\r\n",
			   obj1->short_descr, obj2->short_descr,
			   obj3->short_descr);
		  act (buf, ch, NULL, NULL, TO_ROOM);
		  extract_obj (obj1);
		  extract_obj (obj2);
		  obj_to_char (obj3, ch);
		  return;
		}
	    }
	}
      sprintf (buf, "%s cannot be mixed with %s.\r\n", obj1->short_descr,
	       obj2->short_descr);
      send_to_char (buf, ch);
      return;
    }
  send_to_char ("Syntax : mix add vnum1 vnum2 vnumresult.", ch);
  return;
}

//Iblis - loads the inventions file from disk
void load_inventions ()
{
  FILE *fp;
  INVENTION_DATA *blist;
  if ((fp = fopen (INVENTION_FILE, "r")) == NULL)
    return;
  blist = NULL;
  for (;;)
    {
      INVENTION_DATA *invention;
      if (feof (fp))
	{
	  fclose (fp);
	  return;
	}
      invention = new_invention ();
      invention->vnum1 = fread_number (fp);
      invention->vnum2 = fread_number (fp);
      invention->vnum3 = fread_number (fp);
      fread_to_eol (fp);
      if (invention_list == NULL)
	invention_list = invention;

      else
	blist->next = invention;
      blist = invention;
    }
}
