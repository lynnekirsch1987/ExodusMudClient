#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "magic.h"
void do_deposit (CHAR_DATA * ch, char *argument)
{
  int amount;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  char coin_type[MAX_INPUT_LENGTH];
  argument = one_argument (argument, arg);
  argument = one_argument (argument, coin_type);
  amount = atoi (arg);
  if (IS_NPC (ch))
    return;			/* prevent NPC's from depositing */
  if (!IS_SET (ch->in_room->room_flags, ROOM_BANK))
    {
      send_to_char ("You can only do that in a bank.\n\r", ch);
      return;
    }
  if (!IS_NPC (ch))
    if ((time (NULL) - ch->pcdata->last_fight < 120))
      {
	send_to_char
	  ("The bank teller looks at you suspiciously and tells you 'Come back later'\n\r",
	   ch);
	return;
      }
  if (strcmp (arg, "all")
      && (arg[0] == '\0' || !is_number (arg) || amount <= 0
	  || (str_cmp (coin_type, "gold") && str_cmp (coin_type, "silver"))))
    {
      send_to_char ("Try deposit <amount> <gold|silver>.\n\r", ch);
      send_to_char (" or deposit all.\n\r", ch);
      return;
    }
  if (!strcmp (arg, "all"))
    {
      ch->pcdata->bank_silver += ch->silver;
      ch->pcdata->bank_gold += ch->gold;
      ch->gold = 0;
      ch->silver = 0;
      sprintf (buf, "Your balance is %ld gold, %ld silver.\n\r",
	       ch->pcdata->bank_gold, ch->pcdata->bank_silver);
      send_to_char (buf, ch);
      return;
    }
  if (!str_cmp (coin_type, "silver"))
    {
      if (ch->silver < amount)
	{
	  send_to_char ("You don't have that much silver.\n\r", ch);
	  return;
	}
      ch->silver -= amount;
      ch->pcdata->bank_silver += amount;
      sprintf (buf, "Your balance is %ld gold, %ld silver.\n\r",
	       ch->pcdata->bank_gold, ch->pcdata->bank_silver);
      send_to_char (buf, ch);
    }
  else
    {
      if (ch->gold < amount)
	{
	  send_to_char ("You don't have that much gold.\n\r", ch);
	  return;
	}
      ch->gold -= amount;
      ch->pcdata->bank_gold += amount;
      sprintf (buf, "Your balance is %ld gold, %ld silver.\n\r",
	       ch->pcdata->bank_gold, ch->pcdata->bank_silver);
      send_to_char (buf, ch);
    }
}
void do_balance (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  if (IS_NPC (ch))
    return;			/*prevent NPC's from checking balance */
  if (!IS_SET (ch->in_room->room_flags, ROOM_BANK))
    {
      send_to_char ("You can only do that in a bank.\n\r", ch);
      return;
    }
  if (!IS_NPC (ch))
    if ((time (NULL) - ch->pcdata->last_fight < 120))
      {
	send_to_char
	  ("The bank teller looks at you suspiciously and tells you 'Come back later'\n\r",
	   ch);
	return;
      }
  sprintf (buf, "Your balance: %ld gold, %ld silver.\n\r",
	   ch->pcdata->bank_gold, ch->pcdata->bank_silver);
  send_to_char (buf, ch);
}

void do_withdraw (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char coin_type[MAX_INPUT_LENGTH];
  int amount;
  if (IS_NPC (ch))
    {
      send_to_char ("You can't do that.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg);
  one_argument (argument, coin_type);
  amount = atoi (arg);
  if (!IS_SET (ch->in_room->room_flags, ROOM_BANK))
    {
      send_to_char ("You can only do that in a bank.\n\r", ch);
      return;
    }
  if (!IS_NPC (ch))
    if ((time (NULL) - ch->pcdata->last_fight < 120))
      {
	send_to_char
	  ("The bank teller looks at you suspiciously and tells you 'Come back later'\n\r",
	   ch);
	return;
      }
  if (strcmp (arg, "all")
      && (arg[0] == '\0' || !is_number (arg) || amount <= 0
	  || (str_cmp (coin_type, "gold") && str_cmp (coin_type, "silver"))))
    {
      send_to_char ("Try withdraw <amount> <gold|silver>.\n\r", ch);
      send_to_char (" or withdraw all.\n\r", ch);
      return;
    }
  if (!strcmp (arg, "all"))
    {
      ch->silver += ch->pcdata->bank_silver;
      ch->gold += ch->pcdata->bank_gold;
      ch->pcdata->bank_gold = 0;
      ch->pcdata->bank_silver = 0;
      sprintf (buf, "Your balance is %ld gold, %ld silver.\n\r",
	       ch->pcdata->bank_gold, ch->pcdata->bank_silver);
      send_to_char (buf, ch);
      return;
    }
  if (!str_cmp (coin_type, "silver"))
    {
      if (ch->pcdata->bank_silver < amount)
	{
	  send_to_char
	    ("You don't have that much silver in the bank.\n\r", ch);
	  return;
	}
      ch->silver += amount;
      ch->pcdata->bank_silver -= amount;
      sprintf (buf, "Your balance is %ld gold, %ld silver.\n\r",
	       ch->pcdata->bank_gold, ch->pcdata->bank_silver);
      send_to_char (buf, ch);
    }
  else
    {
      if (ch->pcdata->bank_gold < amount)
	{
	  send_to_char ("You don't have that much gold in the bank.\n\r", ch);
	  return;
	}
      ch->gold += amount;
      ch->pcdata->bank_gold -= amount;
      sprintf (buf, "Your balance is %ld gold, %ld silver.\n\r",
	       ch->pcdata->bank_gold, ch->pcdata->bank_silver);
      send_to_char (buf, ch);
    }
}
