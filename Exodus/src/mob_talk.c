#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "merc.h"
#include "tables.h"
#include "interp.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"

inline void variable_substitution_new (CHAR_DATA *ch, CHAR_DATA *victim, CHAR_DATA *wearer, CHAR_DATA *extra, char *string);
	

inline int said_name (char *name, struct talk_list *tlist)
{
  struct talk_list *walk_tlist;
  for (walk_tlist = tlist; walk_tlist; walk_tlist = walk_tlist->next)
    if (is_name (walk_tlist->word, name))
      return (1);
  return (0);
}
inline void free_response_list (struct response_list *rlist)
{
  struct response_list *walk_rlist, *next_rlist;
  for (walk_rlist = rlist; walk_rlist; walk_rlist = next_rlist)
    {
      next_rlist = walk_rlist->next;
      free (walk_rlist);
    }
}
inline void free_talk_list (struct talk_list *tlist)
{
  struct talk_list *walk_tlist, *next_tlist;
  for (walk_tlist = tlist; walk_tlist; walk_tlist = next_tlist)
    {
      next_tlist = walk_tlist->next;
      free (walk_tlist);
    }
}
inline void
respond (CHAR_DATA * ch, CHAR_DATA * mob, struct response_list *rlist,
	 int from)
{
  char first_word[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char *cptr;
  cptr = strtok (rlist->text, ";");
  if (cptr == NULL)
    return;
  cptr = one_argument_nl (cptr, first_word);
  if (!str_cmp (first_word, "cmd"))
    interpret (mob, cptr);

  else if (!str_cmp (first_word, "trig"))
    {
      if (HAS_SCRIPT (mob))
	{
	  TRIGGER_DATA *trig;
	  char trigcmd[MAX_STRING_LENGTH];
	  cptr = one_argument (cptr, trigcmd);
	  for (trig = mob->triggers; trig != NULL; trig = trig->next)
	    {
	      if (trig->current == NULL && !IS_SET (mob->act, ACT_HALT)
		  && !str_cmp (trig->name, trigcmd))
		{
		  act_trigger (mob, trig->name, rlist->wassaid, NAME (ch),
			       NAME (mob));
		  trig->current = trig->script;
		  trig->bits = SCRIPT_ADVANCE;
		}
	    }
	}
    }
  else
    {
      if (from == 0 && ch->race != 7)
	{
	  sprintf (buf, "%s %s %s", ch->name, first_word, cptr);
	  do_tell (mob, buf);
	}
      else
	{
	  sprintf (buf, "%s %s", first_word, cptr);
	  do_say (mob, buf);
	}
    }
  while ((cptr = strtok (NULL, ";")) != NULL)
    {
      cptr = one_argument_nl (cptr, first_word);
      if (!str_cmp (first_word, "cmd"))
	interpret (mob, cptr);

      else if (!str_cmp (first_word, "trig"))
	{
	  if (HAS_SCRIPT (mob))
	    {
	      TRIGGER_DATA *trig;
	      char trigcmd[MAX_STRING_LENGTH];
	      cptr = one_argument (cptr, trigcmd);
	      for (trig = mob->triggers; trig != NULL; trig = trig->next)
		{
		  if (trig->current ==
		      NULL && !IS_SET (mob->act,
				       ACT_HALT) && !str_cmp (trig->
							      name, trigcmd))
		    {
		      act_trigger (mob, trig->name, rlist->wassaid,
				   NAME (ch), NULL);
		      trig->current = trig->script;
		      trig->bits = SCRIPT_ADVANCE;
		    }
		}
	    }
	}
      else
	{
	  if (from == 0 && ch->race != 7)
	    {
	      sprintf (buf, "%s %s %s", ch->name, first_word, cptr);
	      do_tell (mob, buf);
	    }
	  else
	    {
	      sprintf (buf, "%s %s", first_word, cptr);
	      do_say (mob, buf);
	    }
	}
    }
}

inline void variable_substitution (CHAR_DATA * ch, char *string)
{
	variable_substitution_new (ch,NULL,NULL,NULL,string);
}

inline void variable_substitution_new (CHAR_DATA *ch, CHAR_DATA *victim, 
				       CHAR_DATA *wearer, CHAR_DATA *extra, char *string)
{
  char new_string[MAX_STRING_LENGTH];
  char *cptr, *cptr2;
  char buf[MAX_STRING_LENGTH];

  /* Substitute character name */
  cptr2 = string;
  new_string[0] = '\0';
  while ((cptr = strstr (cptr2, "$name")) != NULL)
    {
      *cptr = '\0';
      strcat (new_string, cptr2);
      if (!IS_NPC(ch))
        strcat (new_string, ch->name);
      else strcat (new_string, ch->short_descr);
      cptr2 = (cptr + 5);
    }
  strcat (new_string, cptr2);
  strcpy (string, new_string);

  /* Substitute area name */
  cptr2 = string;
  new_string[0] = '\0';
  while ((cptr = strstr (cptr2, "$area")) != NULL)
    {
      *cptr = '\0';
      strcat (new_string, cptr2);
      strcat (new_string, ch->in_room->area->name);
      cptr2 = (cptr + 5);
    }
  strcat (new_string, cptr2);
  strcpy (string, new_string);

  /* Substitute room name */
  cptr2 = string;
  new_string[0] = '\0';
  while ((cptr = strstr (cptr2, "$room")) != NULL)
    {
      *cptr = '\0';
      strcat (new_string, cptr2);
      strcat (new_string, ch->in_room->name);
      cptr2 = (cptr + 5);
    }
  strcat (new_string, cptr2);
  strcpy (string, new_string);

  /* Substitute level */
  cptr2 = string;
  new_string[0] = '\0';
  while ((cptr = strstr (cptr2, "$level")) != NULL)
    {
      *cptr = '\0';
      strcat (new_string, cptr2);
      sprintf (new_string + strlen (new_string), "%d", ch->level);
      cptr2 = (cptr + 6);
    }
  strcat (new_string, cptr2);
  strcpy (string, new_string);

  /* Substitute race name */
  cptr2 = string;
  new_string[0] = '\0';
  while ((cptr = strstr (cptr2, "$race")) != NULL)
    {
      *cptr = '\0';
      strcat (new_string, cptr2);
      if (!IS_NPC(ch))
        strcat (new_string, pc_race_table[ch->race].name);
      else strcat (new_string, race_table[ch->race].name);
      cptr2 = (cptr + 5);
    }
  strcat (new_string, cptr2);
  strcpy (string, new_string);

  /* Substitute clan name */
  cptr2 = string;
  new_string[0] = '\0';
  while ((cptr = strstr (cptr2, "$clan")) != NULL)
    {
      *cptr = '\0';
      strcat (new_string, cptr2);
      if (ch->clan != CLAN_BOGUS)
	strcat (new_string, get_clan_name_ch (ch));

      else
	strcat (new_string, "no clan");
      cptr2 = (cptr + 5);
    }
  strcat (new_string, cptr2);
  strcpy (string, new_string);

  /* Substitute Class name */
  cptr2 = string;
  new_string[0] = '\0';
  while ((cptr = strstr (cptr2, "$Class")) != NULL)
    {
      *cptr = '\0';
      strcat (new_string, cptr2);
      if (IS_NPC(ch))
        strcat (new_string, "mobile");
      else strcat (new_string, Class_table[ch->Class].name);
      cptr2 = (cptr + 6);
    }
  strcat (new_string, cptr2);
  strcpy (string, new_string);

  /* Substitute time */
  cptr2 = string;
  new_string[0] = '\0';
  while ((cptr = strstr (cptr2, "$time")) != NULL)
    {
      *cptr = '\0';
      strcat (new_string, cptr2);
      sprintf (new_string + strlen (new_string), "%d o'clock %s",
	       (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
	       time_info.hour >= 12 ? "pm" : "am");
      cptr2 = (cptr + 5);
    }
  strcat (new_string, cptr2);
  strcpy (string, new_string);

  /* Substitute he/she/it */
  cptr2 = string;

  new_string[0] = '\0';

  while ((cptr = strstr (cptr2, "$e")) != NULL)
    {
      *cptr = '\0';
      strcat (new_string, cptr2);
      strcat (new_string, (ch->sex == SEX_MALE)?"he":((ch->sex == SEX_FEMALE)?"she":"it"));
      cptr2 = (cptr + 2);
    }

  strcat (new_string, cptr2);

  strcpy (string, new_string);

  /* Substitute him/her/it */
  cptr2 = string;

  new_string[0] = '\0';

  while ((cptr = strstr (cptr2, "$m")) != NULL)
    {
      *cptr = '\0';
      strcat (new_string, cptr2);
      strcat (new_string, (ch->sex == SEX_MALE)?"him":((ch->sex == SEX_FEMALE)?"her":"it"));
      cptr2 = (cptr + 2);
    }

  strcat (new_string, cptr2);

  strcpy (string, new_string);

  /* Substitute his/her/its */
  cptr2 = string;

  new_string[0] = '\0';

  while ((cptr = strstr (cptr2, "$s")) != NULL)
    {
      *cptr = '\0';
      strcat (new_string, cptr2);
      strcat (new_string, (ch->sex == SEX_MALE)?"his":((ch->sex == SEX_FEMALE)?"her":"its"));
      cptr2 = (cptr + 2);
    }

  strcat (new_string, cptr2);

  strcpy (string, new_string);

  /* Substitute random mob */
  cptr2 = string;

  new_string[0] = '\0';

  while ((cptr = strstr (cptr2, "$randmob")) != NULL)
    {
      *cptr = '\0';
      strcat (new_string, cptr2);
      if (ch->in_room)
	{
	  int i=0,amount=0;
	  CHAR_DATA *tch;
	  for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
	    {
	      if (IS_NPC(tch))
		++amount;
	    }
	  if (amount != 0)
	  {
	    amount = number_range(1,amount);

	    for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
            {
	      if (IS_NPC(tch))
	      {
		if (++i == amount)
		  {
                  strcat (new_string, tch->short_descr);
	          break;
		  }
	      }
	    }
	  }
	  else strcat (new_string, "NOONE");
	    
          cptr2 = (cptr + 8);
	  
	}
    }

  strcat (new_string, cptr2);

  strcpy (string, new_string);
  /* Substitute random pc */
  cptr2 = string;

  new_string[0] = '\0';

  while ((cptr = strstr (cptr2, "$randpc")) != NULL)
    {
      *cptr = '\0';
      strcat (new_string, cptr2);
      if (ch->in_room)
        {
          int i=0,amount=0;
          CHAR_DATA *tch;
          for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
            {
              if (!IS_NPC(tch))
                ++amount;
            }
          if (amount != 0)
            {
	      amount = number_range(1,amount);

	      for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
		{
		  if (!IS_NPC(tch))
		    if (++i == amount)
		      {
			strcat (new_string, tch->name);
			break;
		      }
		}
            }
          else strcat (new_string, "NOONE");
	  cptr2 = (cptr + 7);
	}
    }
      strcat (new_string, cptr2);

      strcpy (string, new_string);

      /* Substitute random pc (but NOT ch, setter offer of trigger)*/
      cptr2 = string;

      new_string[0] = '\0';

      while ((cptr = strstr (cptr2, "$2randpc")) != NULL)
	{
	  *cptr = '\0';
	  strcat (new_string, cptr2);
	  if (ch->in_room)
	    {
	      int i=0,amount=0;
	      CHAR_DATA *tch;
	      for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
		{
		  if (!IS_NPC(tch) && tch != ch)
		    ++amount;
		}
	      if (amount != 0)
		{
		  amount = number_range(1,amount);

		  for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
		    {
		      if (!IS_NPC(tch) && tch != ch)
			if (++i == amount)
			  {
			    strcat (new_string, tch->name);
			    break;
			  }
		    }
		}
	      else strcat (new_string, "NOONE");
	      cptr2 = (cptr + 8);
	    }
	}
	  strcat (new_string, cptr2);

	  strcpy (string, new_string);

	  /* Substitute random ch (but not ch IE the triggerer of the trigger)*/
	  cptr2 = string;

	  new_string[0] = '\0';

	  while ((cptr = strstr (cptr2, "$2randch")) != NULL)
	    {
	      *cptr = '\0';
	      strcat (new_string, cptr2);
	      if (ch->in_room)
		{
		  int i=0,amount=0;
		  CHAR_DATA *tch;
		  for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
		    {
		      if (tch != ch)
			++amount;
		    }
		  if (amount != 0)
		    {
		      amount = number_range(1,amount);

		      for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
			{
			  if (tch != ch)
			    if (++i == amount)
			      {
				if (!IS_NPC(tch))
				  strcat (new_string, tch->name);
				else strcat (new_string, tch->short_descr);
				break;
			      }
			}
		    }
		  else strcat (new_string, "NOONE");
		  cptr2 = (cptr + 8);
		}
	    }
	      strcat (new_string, cptr2);

	      strcpy (string, new_string);

	      /* Substitute random ch */
	      cptr2 = string;

	      new_string[0] = '\0';

	      while ((cptr = strstr (cptr2, "$randch")) != NULL)
		{
		  *cptr = '\0';
		  strcat (new_string, cptr2);
		  if (ch->in_room)
		    {
		      int i=0,amount=0;
		      CHAR_DATA *tch;
		      for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
			{
			    ++amount;
			}
		      if (amount != 0)
			{
			  amount = number_range(1,amount);

			  for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
			    {
				if (++i == amount)
				  {
				    if (!IS_NPC(tch))
				      strcat (new_string, tch->name);
				    else strcat (new_string, tch->short_descr);
				    break;
				  }
			    }
			}
		      else strcat (new_string, "NOONE");
		      cptr2 = (cptr + 7);
		    }
		}
		  strcat (new_string, cptr2);

		  strcpy (string, new_string);

		  /* Substitute random ch (can be used for spells */
		  cptr2 = string;

		  new_string[0] = '\0';

		  while ((cptr = strstr (cptr2, "$3randch")) != NULL)
		    {
		      *cptr = '\0';
		      strcat (new_string, cptr2);
		      if (ch->in_room)
			{
			  int i=0,amount=0;
			  CHAR_DATA *tch;
			  for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
			    {
			      ++amount;
			    }
			  if (amount != 0)
			    {
			      amount = number_range(1,amount);

			      for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
				{
				  if (++i == amount)
				    {
				      if (!IS_NPC(tch))
					strcat (new_string, tch->name);
				      else 
				      {
					one_argument(tch->name,buf);
					strcat(new_string,buf);
				      }
				      //else strcat (new_string, tch->short_descr);
				      break;
				    }
				}
			    }
			  else strcat (new_string, "NOONE");
			  cptr2 = (cptr + 8);
			}
		    }
                  strcat (new_string, cptr2);

                  strcpy (string, new_string);

		  /* Substitute random ch (can be used for spells) (NOT THE TRIGGER)*/
		  cptr2 = string;

		  new_string[0] = '\0';

		  while ((cptr = strstr (cptr2, "$4randch")) != NULL)
		    {
		      *cptr = '\0';
		      strcat (new_string, cptr2);
		      if (ch->in_room)
			{
			  int i=0,amount=0;
			  CHAR_DATA *tch;
			  for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
			    {
			      if (tch != ch)
			        ++amount;
			    }
			  if (amount != 0)
			    {
			      amount = number_range(1,amount);

			      for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
				{
 				  if (tch != ch)
				    if (++i == amount)
				      {
				        if (!IS_NPC(tch))
			  		  strcat (new_string, tch->name);
				        else
				        {
				          one_argument(tch->name,buf);
				          strcat(new_string,buf);
				        }
				        break;
				      }
				}
			    }
			  else strcat (new_string, "NOONE");
			  cptr2 = (cptr + 8);
			}
		    }
                  strcat (new_string, cptr2);

                  strcpy (string, new_string);

		  /* Substitute random mob (can be used for spells) */
		  cptr2 = string;

		  new_string[0] = '\0';

		  while ((cptr = strstr (cptr2, "$2randmob")) != NULL)
		    {
		      *cptr = '\0';
		      strcat (new_string, cptr2);
		      if (ch->in_room)
			{
			  int i=0,amount=0;
			  CHAR_DATA *tch;
			  for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
			    {
			      if (IS_NPC(tch))
			        ++amount;
			    }
			  if (amount != 0)
			    {
			      amount = number_range(1,amount);
				
			      for (tch = ch->in_room->people;tch != NULL;tch = tch->next_in_room)
				{
				  if (IS_NPC(tch))
				    if (++i == amount)
				      {
					 one_argument(tch->name,buf);
			                 strcat(new_string,buf);
				         break;
				      }
				}
			    }
			  else strcat (new_string, "NOONE");
			  cptr2 = (cptr + 9);
			}
		    }
                  strcat (new_string, cptr2);

                  strcpy (string, new_string);

		  //Substitues for victim's name (returns short_descr for mobs, can be used in say)
		  cptr2 = string;

                  new_string[0] = '\0';

                  while ((cptr = strstr (cptr2, "$vict")) != NULL)
                    {
                      *cptr = '\0';
                      strcat (new_string, cptr2);
		      if (victim)
		      {
                          if (!IS_NPC(victim))
                            strcat (new_string, victim->name);
                          else
			  {
			    strcat(new_string,victim->short_descr);
			  }
		      }
                      else strcat (new_string, "NOONE");
                      cptr2 = (cptr + 5);
                    }
                  strcat (new_string, cptr2);

                  strcpy (string, new_string);

		  //substitues for victim's name (returns a keyword, can be used for spells)
		  cptr2 = string;

                  new_string[0] = '\0';

                  while ((cptr = strstr (cptr2, "$2vict")) != NULL)
                    {
                      *cptr = '\0';
                      strcat (new_string, cptr2);
		      if (victim)
			{
                          if (!IS_NPC(victim))
                            strcat (new_string, victim->name);
                          else
			    {
			      one_argument(victim->name,buf);
			      strcat(new_string,buf);
			    }
			}
                      else strcat (new_string, "NOONE");
                      cptr2 = (cptr + 6);
                    }
                  strcat (new_string, cptr2);

                  strcpy (string, new_string);

		  //Substitues for extra's name (returns short_descr for mobs, can be used in say)
                  cptr2 = string;

                  new_string[0] = '\0';

                  while ((cptr = strstr (cptr2, "$3vict")) != NULL)
                    {
                      *cptr = '\0';
                      strcat (new_string, cptr2);
                      if (extra)
			{
                          if (!IS_NPC(extra))
                            strcat (new_string, extra->name);
                          else
			    {
			      strcat(new_string,extra->short_descr);
			    }
			}
                      else strcat (new_string, "NOONE");
                      cptr2 = (cptr + 6);
                    }
                  strcat (new_string, cptr2);

                  strcpy (string, new_string);


		  //substitues for extra's name (returns a keyword, can be used for spells)
                  cptr2 = string;

                  new_string[0] = '\0';

                  while ((cptr = strstr (cptr2, "$4vict")) != NULL)
                    {
                      *cptr = '\0';
                      strcat (new_string, cptr2);
                      if (extra)
                        {
                          if (!IS_NPC(extra))
                            strcat (new_string, extra->name);
                          else
                            {
                              one_argument(extra->name,buf);
                              strcat(new_string,buf);
                            }
                        }
                      else strcat (new_string, "NOONE");
                      cptr2 = (cptr + 6);
                    }
                  strcat (new_string, cptr2);

                  strcpy (string, new_string);



		  //substitution for wearer (returns short_descr of mobs, for use in say)
		  cptr2 = string;

                  new_string[0] = '\0';

                  while ((cptr = strstr (cptr2, "$wearer")) != NULL)
                    {
                      *cptr = '\0';
                      strcat (new_string, cptr2);
		      if (wearer)
			{
                          if (!IS_NPC(wearer))
                            strcat (new_string, wearer->name);
                          else
			    {
			      strcat(new_string,wearer->short_descr);
			    }
			}
                      else strcat (new_string, "NOONE");
                      cptr2 = (cptr + 7);
                    }
                  strcat (new_string, cptr2);

                  strcpy (string, new_string);

		  
		  //substituion for wearer (returns a keyword, can be used in spells etc)
		  cptr2 = string;

                  new_string[0] = '\0';

                  while ((cptr = strstr (cptr2, "$2wearer")) != NULL)
                    {
                      *cptr = '\0';
                      strcat (new_string, cptr2);
		      if (wearer)
                        {
                          if (!IS_NPC(wearer))
                            strcat (new_string, wearer->name);
                          else
                            {
                              one_argument(wearer->name,buf);
                              strcat(new_string,buf);
                            }
                        }
                      else strcat (new_string, "NOONE");
                      cptr2 = (cptr + 8);
                    }
                  strcat (new_string, cptr2);

                  strcpy (string, new_string);




}
inline int calculate_new_mood (char *string, int old_mood)
{
  register sh_int new_mood;
  if (string[0] == '-' && string[1] == '\0')
    return (old_mood);
  switch (string[0])
    {
    case '-':
      new_mood = old_mood - atoi (&string[1]);
      if (new_mood < 0)
	new_mood = 0;

      else if (new_mood > 7)
	new_mood = 7;
      break;
    case '+':
      new_mood = old_mood + atoi (&string[1]);
      if (new_mood < 0)
	new_mood = 0;

      else if (new_mood > 7)
	new_mood = 7;
      break;
    default:
      new_mood = atoi (string);
      if (new_mood < 0)
	new_mood = 0;

      else if (new_mood > 8)
	new_mood = 8;
      break;
    }
  return (new_mood);
}
inline int mood_matches (char *string, int mood_value)
{
  switch (string[0])
    {
    case '<':
      if (mood_value < atoi (&string[1]))
	return (1);
      break;
    case '>':
      if (mood_value > atoi (&string[1]) && mood_value != 8)
	return (1);
      break;
    default:
      if (atoi (string) == mood_value)
	return (1);
      break;
    }
  return (0);
}
inline int word_match (char *word, struct talk_list *talk_words)
{
  struct talk_list *step_words;
  char *cptr;
  if ((cptr = strtok (word, "|")) == NULL)
    return (0);
  for (step_words = talk_words; step_words; step_words = step_words->next)
    {
      if (!str_cmp (step_words->word, cptr))
	return (1);
    }
  while ((cptr = strtok (NULL, "|")) != NULL)
    for (step_words = talk_words; step_words; step_words = step_words->next)
      {
	if (!str_cmp (step_words->word, cptr))
	  return (1);
      }
  return (0);
}

int real_matches;
int default_matches;
struct response_list *responses;
struct response_list *default_responses;
int
match_vocab_file (char *filename, struct talk_list *talk_words, int from,
		  CHAR_DATA * mob, CHAR_DATA * ch, int level, char *wassaid)
{
  struct response_list *new_response;
  char fullpath[MAX_STRING_LENGTH];
  FILE *vocfp;
  register bool found;
  char *cptr, *cchar, *cword;
  char voc_line[MAX_STRING_LENGTH];
  if (level == 5)
    return (0);
  sprintf (fullpath, "mobvoc/%s", filename);
  if ((vocfp = fopen (fullpath, "r")) == NULL)
    return (1);

  /* Skip creator line */
  fgets (voc_line, MAX_STRING_LENGTH, vocfp);
  while (fgets (voc_line, MAX_STRING_LENGTH, vocfp))
    {
      found = TRUE;
      cptr = strchr (voc_line, ':');
      if (cptr == NULL)
	break;
      *cptr = '\0';
      if (!strcmp (voc_line, "DEFAULT")
	  && (from == 0 || said_name (mob->name, talk_words)))
	{
	  cchar = (cptr + 1);
	  cptr = strtok (cchar, ":");
	  if (cptr == NULL)
	    {
	      fclose (vocfp);
	      return (-1);
	    }
	  if (!mood_matches (cptr, mob->mood))
	    continue;
	  default_matches++;
	  new_response = malloc (sizeof (struct response_list));
//	  new_response = reinterpret_cast<response_list *>(malloc(sizeof(struct response_list)));
	  if (new_response == NULL)
	    {
	      fclose (vocfp);
	      return (-1);
	    }
	  cptr = strtok (NULL, ":");
	  new_response->new_mood = calculate_new_mood (cptr, mob->mood);
	  cptr = strtok (NULL, "\n");
	  if (cptr == NULL)
	    {
	      fclose (vocfp);
	      return (-1);
	    }
	  variable_substitution (ch, cptr);
	  strcpy (new_response->text, cptr);
	  strcpy (new_response->wassaid, wassaid);
	  new_response->next = default_responses;
	  default_responses = new_response;
	  continue;
	}
      else if (!strcmp (voc_line, "INCLUDE"))
	{
	  if ((cword = (cptr + 1)) == NULL)
	    continue;
	  if ((cptr = strchr (cword, '\n')) != NULL)
	    *cptr = '\0';
	  if (match_vocab_file
	      (cword, talk_words, from, mob, ch, level + 1, wassaid) == -1)
	    {
	      free_response_list (default_responses);
	      free_response_list (responses);
	      free_talk_list (talk_words);
	      return (-1);
	    }
	  continue;
	}
      cchar = cword = voc_line;
      while (*cchar != '\0')
	{
	  if (*cchar == '&')
	    {
	      *cchar = '\0';
	      if (!word_match (cword, talk_words))
		{
		  found = FALSE;
		  break;
		}
	      cword = (cchar + 1);
	    }
	  cchar++;
	}
      if (found)
	if (!word_match (cword, talk_words))
	  found = FALSE;
      if (!found)
	continue;

      else
	{
	  cchar++;
	  cptr = strtok (cchar, ":");
	  if (cptr == NULL)
	    {
	      fclose (vocfp);
	      return (-1);
	    }
	  if (!mood_matches (cptr, mob->mood))
	    continue;
	  real_matches++;
	  new_response = malloc (sizeof (struct response_list));
//	  new_response = reinterpret_cast<response_list *>(malloc(sizeof(struct response_list)));
	  if (new_response == NULL)
	    {
	      fclose (vocfp);
	      return (-1);
	    }
	  cptr = strtok (NULL, ":");
	  new_response->new_mood = calculate_new_mood (cptr, mob->mood);
	  cptr = strtok (NULL, "\n");
	  if (cptr == NULL)
	    {
	      fclose (vocfp);
	      return (-1);
	    }
	  variable_substitution (ch, cptr);
	  strcpy (new_response->text, cptr);
	  strcpy (new_response->wassaid, wassaid);
	  new_response->next = responses;
	  responses = new_response;
	}
    }
  fclose (vocfp);
  return (0);
}

void mob_talk (CHAR_DATA * ch, CHAR_DATA * mob, char *string, int from)
{
  struct response_list *new_response;
  char *cptr;
  struct talk_list *new_talk;
  register sh_int rand_match;
  struct talk_list *talk_words = NULL;
  char wassaid[MAX_STRING_LENGTH];
  responses = NULL;
  default_responses = NULL;
  real_matches = default_matches = 0;
  if (mob->pIndexData->vocfile[0] == '\0' || mob->mood == 0)
    return;
  strcpy (wassaid, string);
  for (cptr = string; *cptr != '\0'; cptr++)
    {
      if (!isalnum (*cptr))
	*cptr = ' ';
    }
  if ((cptr = strtok (string, " ")) == NULL)
    return;
  if ((talk_words = malloc (sizeof (struct talk_list))) == NULL)
//  if ((talk_words = reinterpret_cast<talk_list *>(malloc(sizeof(struct talk_list)))) == NULL)
    return;
  talk_words->word = cptr;
  talk_words->next = NULL;
  while ((cptr = strtok (NULL, " ")) != NULL)
    {
      if ((new_talk = malloc (sizeof (struct talk_list))) == NULL)
//      if ((new_talk = reinterpret_cast<talk_list *>(malloc(sizeof(struct talk_list)))) == NULL)
	return;
      new_talk->word = cptr;
      new_talk->next = talk_words;
      talk_words = new_talk;
    }
  if (talk_words == NULL)
    return;
  if (match_vocab_file
      (mob->pIndexData->vocfile, talk_words, from, mob, ch, 1, wassaid) == -1)
    {
      free_response_list (default_responses);
      free_response_list (responses);
      free_talk_list (talk_words);
    }
  if (real_matches != 0)
    {
      rand_match = number_range (0, real_matches - 1);
      real_matches = 0;
      for (new_response = responses; new_response;
	   new_response = new_response->next)
	{
	  if (rand_match == real_matches)
	    {
	      respond (ch, mob, new_response, from);
	      mob->mood = new_response->new_mood;
	      break;
	    }
	  real_matches++;
	}
    }
  else if (default_matches != 0)
    {
      rand_match = number_range (0, default_matches - 1);
      default_matches = 0;
      for (new_response = default_responses; new_response;
	   new_response = new_response->next)
	{
	  if (rand_match == real_matches)
	    {
	      respond (ch, mob, new_response, from);
	      mob->mood = new_response->new_mood;
	      break;
	    }
	  default_matches++;
	}
    }
  free_response_list (default_responses);
  free_response_list (responses);
  free_talk_list (talk_words);
  return;
}
