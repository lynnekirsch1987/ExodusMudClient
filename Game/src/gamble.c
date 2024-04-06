/*  Roulette code
 *  Author: Cronel (cronel_kal@hotmail.com)
 *  of FrozenMUD (empire.digiunix.net 4000)
 *
 *  Permission to use and distribute this code is granted provided
 *  this header is retained and unaltered, and the distribution
 *  package contains all the original files unmodified.
 *  If you modify this code and use/distribute modified versions
 *  you must give credit to the original author(s).
 *  
 *  Written in 29/1/98.
 */
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"

/* 
 * DATA DEFINITIONS
 */


#define CREATE(result, type, number)                     \
do                                                       \
{                                                        \
    (result) = (type *) calloc ((number), sizeof(type)); \
} while(0)

#define QUICKLINK(point)    str_dup((point))

#define LINK(link, first, last, next, prev)  \
do                                           \
{                                            \
    if ( (first) == NULL)                    \
      (first)           = (link);            \
    else                                     \
      (last)->next      = (link);            \
    (link)->next        = NULL;              \
    (link)->prev        = (last);            \
    (last)              = (link);            \
} while(0)


#define DISPOSE(point)                       \
do                                           \
{                                            \
  if ((point) == NULL)                       \
  {                                          \
    bug("Trying to dispose NULL pointer", 0);\
                                             \
  }                                          \
  else free(point);                          \
  point = NULL;                              \
} while(0)


typedef enum
{
  ROULETTE_IDLE,		/* Roulette is idle. No bets recieved. */
  ROULETTE_BETTING,		/* Someone has bet.. */
  ROULETTE_END_BETTING,		/* Players have time till
				   next update to bet. In next update the
				   ball will be rolled */
  ROULETTE_ROLLING		/* Ball is rolling. In next update, the rolling
				   will be over and bets will be resolved;
				   roulette will go back to idle */
}
roulete_states;

/* bet types */
typedef enum
{
  BET_NONE,
  /* from 1 to 36 the bet_type is the number.. */
  BET_RED = 37, BET_BLACK, BET_ODD, BET_EVEN, BET_1STCOL, BET_2NDCOL,
  BET_3RDCOL, BET_1STDOZEN, BET_2NDDOZEN, BET_3RDDOZEN
}
BET_TYPE;

/* keywords for betting in each of the non-numeric types */
char *const bet_names[] = {
  "red", "black", "odd", "even", "col1", "col2", "col3",
  "doz1", "doz2", "doz3",
  NULL				/* null acts as array terminator, dont remove */
};

/* descriptive names for non numeric bet types */
char *const desc_bet_names[] = {
  "red", "black", "odds", "evens", "first column", "second column",
  "third column", "first dozen", "second dozen", "third dozen"
};


typedef struct bet_data BET_DATA;
struct bet_data
{
  BET_DATA *next_bet;
  BET_DATA *prev_bet;

  char *player_name;		/* Name of the gambler */
  int amount;			/* Amount of money in the bet */
  sh_int bet_type;		/* On what did they bet? */
};

/* 
 *  MODULE DATA
 */

sh_int roulette_state = ROULETTE_IDLE;
BET_DATA *first_bet = NULL;
BET_DATA *last_bet = NULL;
char *roulette_extra_descr;
extern sh_int top_ed;		/* from db.c */

/*
 *  LOCAL FUNCTIONS
 */

OBJ_DATA *get_roulette_obj ();
CHAR_DATA *get_croupier (ROOM_INDEX_DATA * room);
EXTRA_DESCR_DATA *get_roulette_extra (OBJ_DATA * roulette);
EXTRA_DESCR_DATA *get_extra_from_list (EXTRA_DESCR_DATA * extra_list,
				       char *name);
void clean_bets ();
void resolve_bets (CHAR_DATA * croupier, int number);
bool has_won (sh_int type, sh_int number);
int calc_gain (sh_int type, int amount);
char *get_bet_name (sh_int type);
sh_int get_bet_type (char *str);
char *colorize_number (int num);
int advatoi (const char *s);		/*     from bet.h (included in act_obj.c).
				   used in auctions.
				   god knows why it's in an *.h .. */

void do_bet (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int amount;
  BET_TYPE bet_type;
  OBJ_DATA *roulette;
  ROOM_INDEX_DATA *room;
  CHAR_DATA *croupier;
  BET_DATA *bet;
  EXTRA_DESCR_DATA *extra;

  if ((roulette = get_roulette_obj ()) == NULL
      || (room = roulette->in_room) == NULL
      || (croupier = get_croupier (room)) == NULL)
    {
      send_to_char
	("Sorry but the roulette hasn't been set up properly yet.\n\r", ch);
      return;
    }

  if (!str_cmp (argument, "stop") && IS_IMMORTAL (ch))
    {
      if (roulette_state != ROULETTE_IDLE)
	{
	  act ("$n says '$N wants us to finish this right now, so...",
	       croupier, NULL, ch, TO_ROOM);
	  roulette_state = ROULETTE_ROLLING;
	  bet_update ();
	  send_to_char ("Ok. The roulette round is now over", ch);
	}
      else
	send_to_char
	  ("The roulette is idle right now. No need to do that.\n\r", ch);
      return;
    }

  if (ch->in_room != room)
    {
      send_to_char ("You can't bet here. Go to the roulette!\n\r", ch);
      return;
    }

  if (IS_NPC (ch))
    {
      send_to_char ("Sorry, mobs can't bet.\n\r", ch);
      return;
    }
  if (!can_see (croupier, ch))
    {
      send_to_char ("You can't bet while being invisible.\n\r", ch);
      return;
    }

  if (roulette_state == ROULETTE_ROLLING)
    {
      sprintf (buf,
	       "%s tells you 'You can't bet now! Wait till it stops rolling, please.'\n\r",
	       croupier->short_descr);
      send_to_char (buf, ch);
      return;
    }

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  /* munch optional words */
  if (!str_cmp (arg2, "coins")
      || !str_cmp (arg2, "coin") || !str_cmp (arg2, "gold"))
    argument = one_argument (argument, arg2);
  if (!str_cmp (arg2, "on"))
    argument = one_argument (argument, arg2);

  if (arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char ("Syntax: bet <amount> gold on <something>\n\r", ch);
      return;
    }

  amount = advatoi (arg1);
  if (amount <= 0)
    {
      send_to_char ("Illegal amount.\n\r", ch);
      return;
    }
  if (ch->gold < amount)
    {
      send_to_char ("You don't have that much money.\n\r", ch);
      return;
    }

  if (amount > 10000)
    {
      send_to_char
	("10,000 is the most you may place on a single bet.\n\r", ch);
      return;
    }

  bet_type = get_bet_type (arg2);
//  bet_type = static_cast<BET_TYPE>(get_bet_type(arg2));
  if (bet_type == BET_NONE)
    {
      send_to_char
	("You may bet on 1-36, \"red\", \"black\", \"even\", \"odd\", \"col1\", \"col2\", \"col3\", \"doz1\", \"doz2\", \"doz3\".\n\r",
	 ch);
      return;
    }

  extra = get_roulette_extra (roulette);
  if (roulette_state == ROULETTE_IDLE)
    {
      if (extra != NULL)
	{
	  roulette_extra_descr = extra->description;
	  sprintf (buf, "%s\n\rOn the table you see:\n\r",
		   roulette_extra_descr);
	  extra->description = str_dup (buf);


	}

      roulette_state = ROULETTE_BETTING;
    }
  if (extra != NULL)
    {
      sprintf (buf, "%s%s has %d coins on %s.\n\r",
	       extra->description, ch->name, amount, get_bet_name (bet_type));
      free_string (extra->description);
      extra->description = str_dup (buf);
    }

  ch->gold -= amount;

  CREATE (bet, BET_DATA, 1);
  bet->player_name = QUICKLINK (ch->name);
//      bet->player_name        = ch->name;
  bet->amount = amount;
  bet->bet_type = bet_type;
  LINK (bet, first_bet, last_bet, next_bet, prev_bet);

//      act("$n places a bet in the roulette table.", ch, NULL, NULL, TO_ROOM );
  sprintf (buf, "Your bet of %d coins on %s has been placed, %s.",
	   amount, get_bet_name (bet_type), ch->name);
  do_say (croupier, buf);
  if (ch->level < LEVEL_IMMORTAL)
    WAIT_STATE (ch, PULSE_VIOLENCE);
}

/* look for an extra with the same keyword as the
   roulette's object name.  */
EXTRA_DESCR_DATA *get_roulette_extra (OBJ_DATA * roulette)
{
  EXTRA_DESCR_DATA *extra, *new_extra, *last_extra;

  extra = get_extra_from_list (roulette->extra_descr, "roulette");
  if (extra == NULL)
    {
      extra =
	get_extra_from_list (roulette->pIndexData->extra_descr, "roulette");

      if (extra != NULL)
	{
	  CREATE (new_extra, EXTRA_DESCR_DATA, 1);
	  strcpy ("roulette", new_extra->keyword);
	  new_extra->description = QUICKLINK (extra->description);
	  last_extra = roulette->extra_descr;
	  while (last_extra->next != NULL)
	    last_extra = last_extra->next;

	  if ((roulette->extra_descr) == NULL)
	    roulette->extra_descr = new_extra;
	  else
	    last_extra->next = new_extra;
	  new_extra->next = NULL;

	  top_ed++;
	  return new_extra;
	}
    }
  return extra;
}

EXTRA_DESCR_DATA *get_extra_from_list (EXTRA_DESCR_DATA * extra, char *name)
{
  for (; extra; extra = extra->next)
    {
      if (!str_cmp (name, extra->keyword))
	return extra;
    }
  return NULL;
}

/* return a descriptive name for that type of bet .. static string */
char *get_bet_name (sh_int type)
{
  static char buf[MAX_STRING_LENGTH];

  if (type <= 0)
    {
      bug ("get_bet_name: invalid type passed", 0);
      return "(invalid bet)";
    }

  if (type > 0 && type < 37)
    {
      sprintf (buf, "%d", type);
      return buf;
    }
  else
    return desc_bet_names[type - BET_RED];
}

/* return the type of bet for a given keyword string */
sh_int get_bet_type (char *s)
{
  sh_int number, i;


  number = atoi (s);
  if (number > 0 && number < 37)
    return number;
  else
    {
      for (i = 0; bet_names[i]; i++)
	{
	  if (!str_cmp (bet_names[i], s))
	    return i + BET_RED;
	}
      return BET_NONE;
    }
}

void bet_update (void)
{
  OBJ_DATA *roulette;
  ROOM_INDEX_DATA *room;
  CHAR_DATA *croupier;
  char buf[MAX_STRING_LENGTH];
  sh_int result;
  EXTRA_DESCR_DATA *extra;

  if ((roulette = get_roulette_obj ()) == NULL
      || (room = roulette->in_room) == NULL
      || (croupier = get_croupier (room)) == NULL)
    return;

  switch (roulette_state)
    {
    case ROULETTE_IDLE:
//              do_say(croupier, "Place your bets... Place your bets..." );
//              act("$n is waiting for someone to bet...", croupier, NULL, NULL, TO_ROOM );
      return;
    case ROULETTE_BETTING:
      do_say (croupier, "That's right! Keep betting, keep betting please...");
      roulette_state = ROULETTE_END_BETTING;
      return;
    case ROULETTE_END_BETTING:
      act
	("With a movement of $s hand, $n starts spinning the roulette. After a second, $e throws the ball in.",
	 croupier, NULL, NULL, TO_ROOM);
      do_say (croupier, "No more bets! The ball is rolling!");
      roulette_state = ROULETTE_ROLLING;
      return;
    case ROULETTE_ROLLING:
      /* The big moment! */
      act ("The roulette stops spinning.", croupier, NULL, NULL, TO_ROOM);
      result = (sh_int) number_range (0, 36);
      if (result == 0)
	sprintf (buf,
		 "The roulette has stopped! The ball lands on 0! Everyone loses.");
      else
	sprintf (buf,
		 "The roulette has stopped! The ball lands on %d %s!",
		 result, colorize_number (result));
      do_say (croupier, buf);
      if (result != 0)
	resolve_bets (croupier, result);
      clean_bets ();
      roulette_state = ROULETTE_IDLE;

      extra = get_roulette_extra (roulette);
      if (extra != NULL && roulette_extra_descr != NULL)
	{
	  free_string (extra->description);
	  extra->description = roulette_extra_descr;
	}
      return;
    }
}

/* wipe all bets */
void clean_bets ()
{
  BET_DATA *bet, *next_bet;

  for (bet = first_bet; bet; bet = next_bet)
    {
      next_bet = bet->next_bet;

      free_string (bet->player_name);
      DISPOSE (bet);
    }

  first_bet = NULL;
  last_bet = NULL;
}

/* given the result number, resolve each of the outstanding bets, giving
   money to the winners, and informing the losers */
void resolve_bets (CHAR_DATA * croupier, int number)
{
  BET_DATA *bet;
  char buf[MAX_STRING_LENGTH];
  int gain;
  bool is_in_room;
  CHAR_DATA *ch;
  int bet_count;

  for (bet = first_bet, bet_count = 0; bet; bet = bet->next_bet, bet_count++)
    {
      is_in_room = TRUE;
      ch = get_char_room (croupier, bet->player_name);
      if (!ch)
	{
	  ch = get_char_world (croupier, bet->player_name);
	  is_in_room = FALSE;
	}
      if (!ch)			/* the player left the game... lost his money */
	continue;

      if (has_won (bet->bet_type, number))
	{
	  if (is_in_room)
	    {
	      act ("$n says '$N has won!'", croupier, NULL, ch, TO_ROOM);
	      gain = calc_gain (bet->bet_type, bet->amount);
	      sprintf (buf,
		       "%s tells you 'You won on %s. Here's your %d coins'\n\r",
		       croupier->short_descr, get_bet_name (bet->bet_type),
		       gain);
	      send_to_char (buf, ch);
	      ch->gold += gain;
	      if (!IS_NPC(ch))
	        ch->pcdata->tg_won += gain/2;
	      act ("$n gives you the money.", croupier, NULL, ch, TO_VICT);
	    }
	  else
	    {
	      act
		("$n says '$N has won! But since $E left, I keep the money.'",
		 croupier, NULL, ch, TO_ROOM);
	    }
	}
      else
	{
	  if (is_in_room)
	    {
	      sprintf (buf,
		       "%s tells you 'Sorry, you have lost your %d coins on %s'\n\r",
		       croupier->short_descr, bet->amount,
		       get_bet_name (bet->bet_type));
	      if (!IS_NPC(ch))
                ch->pcdata->tg_lost += bet->amount;
	      send_to_char (buf, ch);
	    }
	}
    }
}

int calc_gain (sh_int type, int amount)
{
  if (type > 0 && type < 37)
    return amount * 36;
  else
    {
      switch (type)
	{
	case BET_EVEN:
	case BET_ODD:
	case BET_RED:
	case BET_BLACK:
	  return amount * 2;
	case BET_1STCOL:
	case BET_2NDCOL:
	case BET_3RDCOL:
	case BET_1STDOZEN:
	case BET_2NDDOZEN:
	case BET_3RDDOZEN:
	  return amount * 3;
	default:
	  bug ("calc_gain: unknown bet type ", 0);
	  return 0;
	}
    }
}

/* return true if the bet_type has won for that given number */
bool has_won (sh_int type, sh_int number)
{
  if (type > 0 && type < 37)
    {
      return (type == number);
    }
  else
    {
      switch (type)
	{
	case BET_EVEN:
	  return ((number % 2) == 0);
	case BET_ODD:
	  return ((number % 2) != 0);
	case BET_RED:
	  return (number == 1 || number == 3 || number == 5 ||
		  number == 7 || number == 9 || number == 12 ||
		  number == 14 || number == 16 || number == 18 ||
		  number == 19 || number == 21 || number == 23 ||
		  number == 25 || number == 27 || number == 30 ||
		  number == 32 || number == 34 || number == 36);
	case BET_BLACK:
	  return (number == 2 || number == 4 || number == 6 ||
		  number == 8 || number == 10 || number == 11 ||
		  number == 13 || number == 15 || number == 17 ||
		  number == 20 || number == 22 || number == 24 ||
		  number == 26 || number == 28 || number == 29 ||
		  number == 31 || number == 33 || number == 35);
	case BET_1STCOL:
	  return (number == 1 || number == 4 || number == 7 ||
		  number == 10 || number == 13 || number == 16 ||
		  number == 19 || number == 22 || number == 25 ||
		  number == 28 || number == 31 || number == 34);
	case BET_2NDCOL:
	  return (number == 2 || number == 5 || number == 8 ||
		  number == 11 || number == 14 || number == 17 ||
		  number == 20 || number == 23 || number == 26 ||
		  number == 29 || number == 32 || number == 35);
	case BET_3RDCOL:
	  return (number == 3 || number == 6 || number == 9 ||
		  number == 12 || number == 15 || number == 18 ||
		  number == 21 || number == 24 || number == 27 ||
		  number == 30 || number == 33 || number == 36);
	case BET_1STDOZEN:
	  return (number >= 1 && number <= 12);
	case BET_2NDDOZEN:
	  return (number >= 13 && number <= 24);
	case BET_3RDDOZEN:
	  return (number >= 25 && number <= 36);
	default:
	  bug ("has_won; unknown bet type", 0);
	  return FALSE;
	}
    }
}

OBJ_DATA *get_roulette_obj ()
{
  OBJ_DATA *obj;

  for (obj = object_list; obj; obj = obj->next)
    {
      if (obj->pIndexData->vnum == OBJ_VNUM_ROULETTE)
	return obj;
    }
  return NULL;
}

CHAR_DATA *get_croupier (ROOM_INDEX_DATA * room)
{
  CHAR_DATA *ch;

  if (!room)
    {
      bug ("get_croupier: null room ", 0);
      return NULL;
    }

  for (ch = room->people; ch; ch = ch->next_in_room)
    {
      if (IS_SET (ch->act2, ACT_CROUPIER))
	return ch;
    }
  return NULL;
}

char *colorize_number (int num)
{
  switch (num)
    {
    case 0:
      return "";
    case 1:
    case 3:
    case 5:
    case 7:
    case 9:
    case 12:
    case 14:
    case 16:
    case 18:
    case 19:
    case 21:
    case 23:
    case 25:
    case 27:
    case 30:
    case 32:
    case 34:
    case 36:
      return "red";
    default:
      return "black";
    }
};
