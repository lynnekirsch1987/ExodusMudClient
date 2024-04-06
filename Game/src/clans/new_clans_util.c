
//
// new_clans_util.c
//
// Programmer: Akamai
// Date: 7/10/98
// Version: 1.0
//
// This contains some public utility functions that supports the
// new clan code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "../merc.h"
#include "new_clans.h"
#include "new_clans_util.h"
#include "new_clans_io.h"
extern int _clan_ident_;
extern int clan_number;
extern struct clan_type clan_table[MAX_CLAN];

char* anti_nidaeify args((CHAR_DATA *ch, char *argument, char *bufn));
char* nidaeify args((CHAR_DATA *ch, char *argument, char *bufn));
void cj_channel args((CHAR_DATA *ch, char* argument));

bool is_ignoring args((CHAR_DATA * ch, CHAR_DATA * victim));

// ===================================================================
// Simple Diplomacy checks
// ===================================================================

//
// determines if one clan has recently defeated another clan
//
bool has_defeated_clan (int clan1id, int clan2id)
{
  RELATIONS *us;
  RELATIONS *them;
  int clan1slot;
  int clan2slot;
  time_t now;
  clan1slot = clanident_to_slot (clan1id);
  clan2slot = clanident_to_slot (clan2id);

  // if they are still at war, then they have not defeated
  if (is_warring (clan1slot, clan2slot))
    return (FALSE);

  // if there was a war then they both have relation records
  them = find_relation (clan_table[clan2slot].relations, clan1id);
  us = find_relation (clan_table[clan1slot].relations, clan2id);
  if (us == NULL)
    return (FALSE);
  if (them == NULL)
    return (FALSE);

  // if the war was recent enough to count
  now = time (NULL);
  if ((now - (us->wartime)) < RECENT_WAR_TIME)
    {
      if ((us->kills) > (them->kills))
	return (TRUE);
    }
  return (FALSE);
}


//
// Is there any clan which has a reciprocal ally agreement with us?
//
bool clan_has_allies (int slot)
{
  RELATIONS *r;
  r = clan_table[slot].relations;
  while (r)
    {
      if (is_ally (slot, clanident_to_slot (r->ident)))
	return (TRUE);
      r = r->next;
    }
  return (FALSE);
}


//
// is_requested_war checks that each is still at war with the other
//
bool is_requested_war (int clan1, int clan2)
{
  RELATIONS *ally;
  ally = find_relation (clan_table[clan1].relations, clan_table[clan2].ident);
  if (ally != NULL)
    {
      return ((ally->status >= CLAN_WAR));
    }
  return (FALSE);
}


//
// Checks that there is a reciprocal war relationship between the clans
//
bool is_warring (int clan1, int clan2)
{
  if (clan1 == clan2)
    return (FALSE);
  if (is_requested_war (clan1, clan2))
    {
      return (is_requested_war (clan2, clan1));
    }
  return (FALSE);
}


//
// is_requested_ally checks that clan 1 has requested to ally with clan 2
//
bool is_requested_ally (int clan1, int clan2)
{
  RELATIONS *ally;
  ally = find_relation (clan_table[clan1].relations, clan_table[clan2].ident);
  if (ally != NULL)
    {
      return ((ally->status == CLAN_ALLY));
    }
  return (FALSE);
}


//
// is__ally checks that clan 1 and clan 2 have reciprocal ally agreement
//
bool is_ally (int clan1, int clan2)
{
  if (is_requested_ally (clan1, clan2))
    {
      return (is_requested_ally (clan2, clan1));
    }
  return (FALSE);
}


//
// is_requested_enemy checks that clan 1 has requested to enemy with clan 2
//
bool is_requested_enemy (int clan1, int clan2)
{
  RELATIONS *ally;
  ally = find_relation (clan_table[clan1].relations, clan_table[clan2].ident);
  if (ally != NULL)
    {
      return ((ally->status == CLAN_ENEMY));
    }
  return (FALSE);
}


//
// is__enemy checks that clan 1 and clan 2 have reciprocal enemy agreement
//
bool is_enemy (int clan1, int clan2)
{
  if (is_requested_enemy (clan1, clan2))
    {
      return (is_requested_enemy (clan2, clan1));
    }
  return (FALSE);
}


//
// returns true if this clan is at war with *any* other clan
//
bool clan_is_warring (int clan)
{
  RELATIONS *r;
  r = clan_table[clan].relations;
  while (r)
    {
      if (r->status >= CLAN_WAR)
	return (TRUE);
      r = r->next;
    }
  return (FALSE);
}


// ===================================================================
// A PC kills another in a clan war
// ===================================================================

//
//
//
void clanwar_kill (CHAR_DATA * ch, CHAR_DATA * victim)
{
  RELATIONS *r;
  int ch_slot;
  int vic_slot;

//  int level_diff = 0;
  int kill_points = CLAN_MEMBER_KILL;
  char buf[MAX_STRING_LENGTH];
  ch_slot = ch->clan;
  vic_slot = victim->clan;
  if ((ch_slot == CLAN_BOGUS) || (vic_slot == CLAN_BOGUS))
    return;
  if (is_warring (ch_slot, vic_slot))
    {
      r = find_relation (clan_table[ch_slot].relations,
			 clan_table[vic_slot].ident);
      (r->kills)++;

      // Iblis 7/15/03 - Changed clan wars to use BR-like points system
      kill_points = victim->level / 2;
      if (victim->level > ch->level)
	kill_points += (victim->level - ch->level) / 2.0;

      else if (victim->level < ch->level)
	kill_points -= (ch->level - victim->level);

      // check the level difference, a kill of a lower
      // level player is not worth as many points in a war
      // if (ch->level > victim->level)
//      {
      // this was a kill down
//        level_diff = (ch->level) - (victim->level);
//        if (level_diff > OOL_KILL_DIFF)
//          {
//            kill_points = CLAN_MEMBER_KILL - OOL_KILL_DIFF - 1;
//          }
//        else
//          {
//            kill_points = CLAN_MEMBER_KILL - level_diff;
//          }
//      }
      //    else
//      {
      // this was a kill up
//        level_diff = (victim->level) - (ch->level);
//        if (level_diff > OOL_KILL_DIFF)
//          {
//            kill_points = CLAN_MEMBER_KILL + (OOL_KILL_DIFF / 4);
//          }
//        else
//          {
//            kill_points = CLAN_MEMBER_KILL + (level_diff / 4);
//            if (level_diff == OOL_KILL_DIFF)
//              kill_points++;
//          }
//      }

      // add any bonus multipliers to this kill
      // it turns out that you want to check first for is_clan_leader
      // because clan leaders *also* have is_clan_deputy status in hierarchy
      if (is_clan_leader (victim))
	{

	  // kill_points = kill_points * CLAN_LEADER_BONUS;
	  kill_points += 50;
	}

      else if (is_clan_deputy (victim))
	{

	  // kill_points = kill_points * CLAN_DEPUTY_BONUS;
	  kill_points += 20;
	}
      if (kill_points < 10)
	kill_points = 10;

      // add the points to the clan record, if any
      if (kill_points > 0)
	{
	  (r->points) = (r->points) + kill_points;
	}

      else
	{
	  kill_points = 0;
	}

      // let them know that they got some points for it
      if (kill_points > 0)
	{
	  sprintf (buf,
		   "Your clan gains %d points for that clan war kill.\r\n",
		   kill_points);
	}

      else
	{
	  sprintf (buf, "That kill was not worth anything to your clan!\r\n");
	}
      send_to_char (buf, ch);
      save_clan (ch_slot);
    }
}


// ===================================================================
// Counting Routines
// ===================================================================

//
// Provide an experience adjustment to successful or unsuccessful clans
//
int xp_adjust (CHAR_DATA * ch, int xp)
{
  RELATIONS *r;
  int twins, tloss;
  int slot;

  // if they are not in a clan they get no bonus
  slot = ch->clan;
  if (slot == CLAN_BOGUS)
    return (0);

  // run through their relation list and get the total wins and losses
  r = clan_table[slot].relations;
  twins = 0;
  tloss = 0;
  while (r)
    {
      twins += (r->wins);
      tloss += (r->loss);
      r = r->next;
    }
  xp += (twins / 2);		// gain one-half of the total wins a bonus 
  xp -= (tloss / 2);		// lose one-half of the total losses as penalty
  return (xp);
}


//
// count the number of allies that this clan has requested
//
int count_clan_allies (int clan)
{
  RELATIONS *r;
  int allies = 0;

  // run through the clan relations list counting the ally requests
  r = clan_table[clan].relations;
  while (r)
    {
      if (r->status == CLAN_ALLY)
	allies++;
      r = r->next;
    }
  return (allies);
}


//
// count the number of enemies that this clan has declared
//
int count_clan_enemies (int clan)
{
  RELATIONS *r;
  int enemies = 0;

  // run through the clan relations list counting the enemies
  r = clan_table[clan].relations;
  while (r)
    {
      if (r->status == CLAN_ENEMY)
	enemies++;
      r = r->next;
    }
  return (enemies);
}


//
// Checks the current number of levels of a clan updates
// both individual players online and the clan status
//
int clan_levels (int slot)
{
  char buf[MAX_STRING_LENGTH];
  int level_total = 0;
  MEMBER *m;
  CHAR_DATA *ch;
  if (slot == CLAN_BOGUS)
    {
      bug ("clan_levels() Bad clan ID\n", 0);
      return (CLAN_BOGUS);
    }
  // run through all of the members of this clan
  m = clan_table[slot].members;
  while (m)
    {
      ch = find_character (m->name);
      if (ch != NULL)
	{
	  m->levels = ch->level;
	  level_total += (ch->level);
	}

      else
	{
	  level_total += (m->levels);
	}
      m = m->next;
    }
  if (level_total > clan_table[slot].levels)
    {
      clan_table[slot].levels = level_total;

      // if the clan was not previously an active clan
      if (((clan_table[slot].status == CLAN_PROPOSED) ||
	   (clan_table[slot].status == CLAN_RESTRICTED)) &&
	  (clan_table[slot].levels >= FULL_CLAN_LEVELS))
	{
	  clan_table[slot].status = CLAN_ACTIVE;
	  sprintf (buf,
		   "Clan %s`` has gained levels and is now a full clan.\n\r",
		   clan_table[slot].name);
	  send_clan_members (slot, buf);
	  save_clan (slot);
	}
    }

  else if (level_total < clan_table[slot].levels)
    {
      clan_table[slot].levels = level_total;

      // if they are active and lost too many levels then demote them
      if ((clan_table[slot].status == CLAN_ACTIVE) &&
	  (clan_table[slot].levels < FULL_CLAN_LEVELS))
	{
	  clan_table[slot].status = CLAN_RESTRICTED;
	  sprintf (buf,
		   "Clan %s`` has lost too many levels and is now restricted.\n\r",
		   clan_table[slot].name);
	  send_clan_members (slot, buf);
	  save_clan (slot);
	}
    }
  return (level_total);
}


// ===================================================================
// Simple String operations
// ===================================================================

//
int color_strlen (char *str)
{
  int pos = 0;
  int len = 0;
  while (str[pos] != '\0')
    {
      if (str[pos] == '`')
	{
	  pos++;
	  if (str[pos] == '\0')
	    return (len);
	  pos++;
	}

      else
	{
	  len++;
	  pos++;
	}
    }
  return (len);
}


//
char *strip_whitespace (char *str)
{
  int pos = 0;
  while ((str[pos] != '\0') && isspace ((int) str[pos]))
    pos++;
  return (&(str[pos]));
}


//
char *find_sepchar (char *str)
{
  int pos = 0;
  while ((str[pos] != '\0') && str[pos] != ':')
    pos++;
  return (&(str[pos]));
}


// returns true if the copy did not overflow
bool limited_copy_str (char *str1, char *str2, int len)
{
  int pos = 0;
  while ((str2[pos] != '\0') && pos < len)
    {
      str1[pos] = str2[pos];
      pos++;
    }
  if (pos < len)
    {
      str1[pos] = '\0';
      return (TRUE);
    }

  else
    {

      // we filled the buffer, make sure we terminate
      str1[pos - 1] = '\0';
      return (FALSE);
    }
}


// removes all spaces in the string
void remove_spaces (char *str)
{
  int curs = 0;
  int i = 0;
  while ((str[i] != '\0'))
    {
      if (!isspace ((int) str[i]))
	{

	  // if they don't point to the same char already then copy
	  if (curs != i)
	    str[curs] = str[i];

	  // both have to move to the next char
	  curs++;
	  i++;
	}

      else
	{

	  // it is a space, skip it
	  i++;
	}
    }
  str[curs] = '\0';
}


// ===================================================================
// Simple table lookups
// ===================================================================

// return the slot associated with this clan name
int clanname_to_slot (const char *name)
{
	int i;
	if ((NULL == name) || (name[0] == '\0'))
		return (CLAN_BOGUS);
	for (i = CLANS_START; i < clan_number; i++) {
		if (!str_cmp (name, clan_table[i].name))
			return (i);
	}
	return (CLAN_BOGUS);
}


// return the clan table slot for this clan id
int clanident_to_slot (int ident)
{
  int i;
  for (i = CLANS_START; i < clan_number; i++)
    {
      if (ident == clan_table[i].ident)
	return (i);
    }
  return (CLAN_BOGUS);
}


// return the clan symbol given a character who is supposedly in a clan
char *get_clan_symbol_ch (CHAR_DATA * ch)
{
  return (get_clan_symbol (ch->clan));
}


// return the clan symbol given a clan slot
char *get_clan_symbol (int slot)
{
  if ((slot != CLAN_BOGUS) && (slot < clan_number))
    {
      return (clan_table[slot].symbol);
    }
  return (CLAN_BOGUS_SYMB);
}


// return the clan hall given a character who is supposedly in a clan
int get_clan_hall_ch (CHAR_DATA * ch)
{
  return (get_clan_hall (ch->clan));
}


// return the clan hall given a clan slot
int get_clan_hall (int slot)
{
  if ((slot != CLAN_BOGUS) && (slot < clan_number))
    {
      return (clan_table[slot].hall);
    }
  return (CLAN_BOGUS);
}


// return the clan name given a character who is supposedly in a clan
char *get_clan_name_ch (CHAR_DATA * ch)
{
  return (get_clan_name (ch->clan));
}


// return the clan name given a clan slot
char *get_clan_name (int slot)
{
  if ((slot != CLAN_BOGUS) && (slot < clan_number))
    {
      return (clan_table[slot].name);
    }
  return (CLAN_BOGUS_NAME);
}


// return the clan name given a character who is supposedly in a clan
int get_clan_ident_ch (CHAR_DATA * ch)
{
  return (get_clan_ident (ch->clan));
}


// return the clan name given a clan slot
int get_clan_ident (int slot)
{
  if ((slot != CLAN_BOGUS) && (slot < clan_number))
    {
      return (clan_table[slot].ident);
    }
  return (CLAN_BOGUS);
}


// return the slot associated with this clan name
int clan_lookup (const char *name)
{
	return (clanname_to_slot(name));
}


// takes a clan symbol and returns the clans slot in the clan table
int clan_symbol_lookup (char *symbol)
{
  int clan;

  // if this is the loner symbol, then return the bogus clan
  if (!str_cmp (symbol, "`f[`h*`f]")
      || !str_cmp (symbol, "`f[`h*`f]``") || !str_cmp (symbol, "[*]"))
    return (CLAN_BOGUS);

  // go through the clan table looking for the clan symbol
  for (clan = CLANS_START; clan < clan_number; clan++)
    {
      if (clan_table[clan].symbol[0] != '\0'
	  && !str_cmp (clan_table[clan].symbol, symbol))
	return (clan);
    }
  return (CLAN_BOGUS);
}


// checks that the clan is a full clan
bool is_full_clan (CHAR_DATA * ch)
{
  if ((ch->clan != CLAN_BOGUS) && (ch->clan < clan_number))
    {
      return ((clan_table[ch->clan].status == CLAN_ACTIVE));
    }
  return (FALSE);
}


// checks that the clan is in a visible active state for mortals
bool is_visible_clan (CHAR_DATA * ch)
{
  if ((ch->clan != CLAN_BOGUS) && (ch->clan < clan_number))
    {
      return ((clan_table[ch->clan].status >= CLAN_HISTORICAL));
    }
  return (FALSE);
}


// ===================================================================
// Player clan status checks
// ===================================================================

// taken over from handler.c
bool is_clan (CHAR_DATA * ch)
{
  if ((ch->clan != CLAN_BOGUS) && (ch->clan < clan_number))
    {
      if (clan_table[ch->clan].status >= CLAN_PROPOSED)
	return (TRUE);
    }
  return (FALSE);
}


// taken over from handler.c
bool is_same_clan (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (is_clan (ch))
    {
      if (is_clan (victim))
	{
	  if (ch->clan == victim->clan)
	    return (TRUE);
	}
    }
  return (FALSE);
}


// is this guy the clan leader
bool is_clan_leader (CHAR_DATA * ch)
{
  int clan;
  clan = ch->clan;
  if ((clan != CLAN_BOGUS) && (clan < clan_number))
    {
      if (clan_table[clan].status < CLAN_PROPOSED)
	return (FALSE);
      if (!str_cmp ((clan_table[clan].leader->name), ch->name))
	return (TRUE);
    }
  return (FALSE);
}

bool is_clan_deputy (CHAR_DATA * ch)
{
  MEMBER *m;
  int clan;
  clan = ch->clan;
  if ((clan != CLAN_BOGUS) && (clan < clan_number))
    {
      if (clan_table[clan].status < CLAN_PROPOSED)
	return (FALSE);
      m = find_member (clan_table[clan].members, ch->name);
      if (m)
	{
	  if (m->status >= CLAN_DEPUTY)
	    return (TRUE);
	}
    }
  return (FALSE);
}


// see if there is an applicant with this name
bool is_clan_applicant (CHAR_DATA * ch, int clan)
{
  MEMBER *m;
  if ((clan != CLAN_BOGUS) && (clan < clan_number))
    {
      if (clan_table[clan].status < CLAN_PROPOSED)
	return (FALSE);
      m = find_member (clan_table[clan].applicants, ch->name);
      if (m)
	return (TRUE);
    }
  return (FALSE);
}


// is this character a clan member
bool is_clan_member (CHAR_DATA * ch, int clan)
{
  MEMBER *m;
  if (ch == NULL)
    return (FALSE);
  if ((clan != CLAN_BOGUS) && (clan < clan_number))
    {
      if (clan_table[clan].status < CLAN_PROPOSED)
	return (FALSE);
      m = find_member (clan_table[clan].members, ch->name);
      if (m)
	return (TRUE);
    }
  return (FALSE);
}


// find if this character has applied to any known clan
int is_any_clan_applicant (CHAR_DATA * ch)
{
  int i;
  if (ch == NULL)
    return (CLAN_BOGUS);
  for (i = CLANS_START; i < clan_number; i++)
    {
      if (is_clan_applicant (ch, i))
	return (i);
    }
  return (CLAN_BOGUS);
}


// find if this character is a member of any known clan
int is_any_clan_member (CHAR_DATA * ch)
{
  int i;
  if (ch == NULL)
    return (CLAN_BOGUS);
  for (i = CLANS_START; i < clan_number; i++)
    {
      if (is_clan_member (ch, i))
	return (i);
    }
  return (CLAN_BOGUS);
}


// find if this character is a member of any known clan
int clan_login_player (CHAR_DATA * ch)
{
  MEMBER *m;
  int i;
  if (ch == NULL)
    return (CLAN_BOGUS);
  for (i = CLANS_START; i < clan_number; i++)
    {
      if (clan_table[i].status >= CLAN_PROPOSED)
	{
	  m = find_member (clan_table[i].members, ch->name);

	  // found the pc in this clan
	  if (m)
	    {
	      m->laston = time (NULL);
	      return (i);
	    }
	}
    }
  return (CLAN_BOGUS);
}


// ===================================================================
// Clan table List Routines
// ===================================================================

// append a member record to the member list
MEMBER *append_member (MEMBER * list, MEMBER * elt)
{
  MEMBER *cur = list;

  // so long as the current thing, and the next thing are valid
  // keep moving through the list
  while (cur && (cur->next))
    {
      cur = cur->next;
    }

  // if the list is null just return elt
  if (list == NULL)
    {
      return (elt);
    }

  else
    {

      // otherwise, set the cursors next thing to elt and return the list
      elt->next = NULL;
      cur->next = elt;
      return (list);
    }
}


// scan list for a matching name and delete that record
MEMBER *delete_member (MEMBER * list, char *name)
{
  MEMBER *cur = list;
  MEMBER *tmp = NULL;
  if (list == NULL)
    return (NULL);

  // first start by checking the first item
  if (cur && !str_cmp (cur->name, name))
    {

      // the first item is it, set the list to point to the remainder
      list = cur->next;
      cur->next = NULL;
      free (cur);
      return (list);
    }

  else
    {

      // check that the next in the list is a record
      // and if it is *not* the guy we want move through the list
      while ((cur->next) && str_cmp (cur->next->name, name))
	{
	  cur = cur->next;
	}

      // we either found him or we found a null next pointer
      // if the next pointer is not null then we found him
      if (cur->next != NULL)
	{
	  tmp = cur->next;
	  cur->next = cur->next->next;	// remove the item from the list
	  tmp->next = NULL;
	  free (tmp);
	}
      return (list);
    }
}


// scan list for a matching name and return a pointer to that record
MEMBER *find_member (MEMBER * list, char *name)
{

  // if the list is null return null
  if (list == NULL)
    return (NULL);

  // first start by checking the first item
  while (list && str_cmp (list->name, name))
    {
      list = list->next;
    }
  if (list && !str_cmp (list->name, name))
    return (list);

  else
    return (NULL);
}


// append a new clan relation to a relation list
RELATIONS *append_relation (RELATIONS * list, RELATIONS * elt)
{
  RELATIONS *cur = list;

  // so long as the current thing, and the next thing are valid
  // keep moving through the list
  while (cur && (cur->next))
    {
      cur = cur->next;
    }

  // if the list is null just return elt
  if (list == NULL)
    {
      return (elt);
    }

  else
    {

      // otherwise, set the cursors next thing to elt and return the list
      elt->next = NULL;
      cur->next = elt;
      return (list);
    }
}


// remove a clan relation from a relation list
RELATIONS *delete_relation (RELATIONS * list, int ident)
{
  RELATIONS *cur = list;
  RELATIONS *tmp = NULL;

  // just return null if the list is null
  if (list == NULL)
    return (NULL);

  // first start by checking the first item
  if (cur && (ident == cur->ident))
    {

      // the first item is it, set the list to point to the remainder
      list = cur->next;
      cur->next = NULL;
      free (cur);
      return (list);
    }

  else
    {

      // check that the next in the list is a record
      // and if it is *not* the clan we want move through the list
      while ((cur->next) && (ident != cur->next->ident))
	{
	  cur = cur->next;
	}

      // we either found the clan or we found a null next pointer
      // if the next pointer is not null then we found it
      if (cur->next != NULL)
	{
	  tmp = cur->next;
	  cur->next = cur->next->next;	// remove the item from the list
	  tmp->next = NULL;
	  free (tmp);
	}
      return (list);
    }
}


// scan list for a matching name and return a pointer to that record
RELATIONS *find_relation (RELATIONS * list, int ident)
{

  // if the list is null just return null
  if (list == NULL)
    return (NULL);

  // first start by checking the first item
  while (list && (ident != list->ident))
    {
      list = list->next;
    }
  if (list && (ident == list->ident))
    return (list);

  else
    return (NULL);
}


// simply allocate a new clan member record struct
MEMBER *new_member_elt ()
{
  MEMBER *m = NULL;
  m = (MEMBER *) malloc (sizeof (struct clan_member));
  if (m == NULL)
    {
      bug ("new_member_elt() failed to allocate %d bytes.",
	   sizeof (struct clan_member));
      return (NULL);
    }
  m->next = NULL;
  m->name[0] = '\0';
  m->levels = CLAN_BOGUS;
  m->status = CLAN_BOGUS;
  m->align = CLAN_BOGUS;
  m->race = 0;
  m->Class = 0;
  m->laston = time (NULL);
  m->initiative = CLAN_BOGUS;
  return (m);
}


// simply allocate a new clan relations record struct
RELATIONS *new_relations_elt ()
{
  RELATIONS *r = NULL;
  r = (RELATIONS *) malloc (sizeof (struct clan_relations));
  if (r == NULL)
    {
      bug ("new_relations_elt() failed to allocate %d bytes.",
	   sizeof (struct clan_relations));
      return (NULL);
    }
  r->next = NULL;
  r->name[0] = '\0';
  r->ident = CLAN_BOGUS;
  r->status = CLAN_NEUTRAL;
  r->acttime = time (NULL);
  r->wartime = time (NULL);
  r->warcount = 0;
  r->wins = 0;
  r->loss = 0;
  r->kills = 0;
  r->points = 0;
  return (r);
}


// ===================================================================
// Removing clan symbols from the area files
// ===================================================================
//
// the following two functions require access to this mud global
//
extern AREA_DATA *area_first;

//
// Remove all of the clan symbols from all areas of the game
//
void remove_clan_symbols ()
{
  OBJ_DATA *obj, *obj_next;
  AREA_DATA *pArea;
  for (obj = object_list; obj; obj = obj_next)
    {
      obj_next = obj->next;
      if (obj->pIndexData->vnum == OBJ_VNUM_CLAN_SYMBOL && obj->in_room)
	{
	  if (clan_table[obj->in_room->area->clan].name[0] == '\0')
	    {
	      remove_symbol_resets (obj->in_room->area);
	      obj->in_room->area->clan = 0;
	      pArea = obj->in_room->area;
	      extract_obj (obj);
	      save_area (pArea);
	    }
	}
    }
  for (pArea = area_first; pArea; pArea = pArea->next)
    {
      if (pArea->clan != 0 && clan_table[pArea->clan].name[0] == '\0')
	{
	  pArea->clan = 0;
	  save_area (pArea);
	}
    }
}


//
// Remove the clan symbol from one specific area
//
void remove_symbol_resets (AREA_DATA * pArea)
{
  RESET_DATA *pReset, *prev = NULL;
  for (pReset = pArea->reset_first; pReset; pReset = pReset->next)
    {
      if (pReset->command == 'O' && pReset->arg1 == OBJ_VNUM_CLAN_SYMBOL)
	{
	  if (prev)
	    {
	      if (pReset == pArea->reset_last)
		pArea->reset_last = prev;
	      prev->next = prev->next->next;
	    }

	  else
	    {
	      pArea->reset_first = pArea->reset_first->next;
	    }
	  free_reset_data (pReset);
	  prev = pReset;
	  break;
	}
      prev = pReset;
    }
}


// ===================================================================
// Some Player stuff
// ===================================================================

// run through all of the characters on line and make sure the
// player we are going to modify is present
CHAR_DATA *find_character (char *name)
{
  CHAR_DATA *pc;
  pc = char_list;
  while (pc)
    {
      if (!IS_NPC (pc))
	{
	  if (!str_cmp (pc->name, name))
	    return (pc);
	}
      pc = pc->next;
    }
  return (NULL);
}


// run through all the clan members of the specified clan
// and send them the message
void send_clan_members_ch (CHAR_DATA * ch, char *str)
{
  CHAR_DATA *cmember;
  int clan;
  clan = ch->clan;
  cmember = char_list;
  while (cmember)
    {
      if (!IS_NPC (cmember) && (ch != cmember) && (clan == (cmember->clan)))
	{
	  send_to_char (str, cmember);
	}
      cmember = cmember->next;
    }
}


// run through all the clan members of the specified clan
// and send them the message
void send_clan_members (int clan, char *str)
{
  CHAR_DATA *cmember;
  cmember = char_list;
  while (cmember)
    {
      if (!IS_NPC (cmember) && (clan == (cmember->clan)))
	{
	  send_to_char (str, cmember);
	}
      cmember = cmember->next;
    }
}


// run through all the players and send a message
void send_all_players (CHAR_DATA * ch, char *str)
{
  CHAR_DATA *player;
  player = char_list;
  while (player)
    {
      if (!IS_NPC (player) && (ch != player))
	send_to_char (str, player);
      player = player->next;
    }
}


// run through all the players and send a message
void send_allies_clanwar (CHAR_DATA * ch, char *str)
{
  char buf[MAX_STRING_LENGTH], bufn[MAX_STRING_LENGTH];
  CHAR_DATA *player;
  if (ch->race == PC_RACE_NIDAE)
    nidaeify(ch,bufn,str);
  else anti_nidaeify(ch,bufn,str);
  sprintf (buf, "[CLANWAR] %s `O$n`a: '`O$t`a'``", get_clan_symbol(ch->clan));
  player = char_list;
  while (player)
    {
      if ((!IS_NPC (player) &&
	   (ch != player) && ((ch->clan == player->clan)
			      || is_ally (ch->clan, player->clan)))
	  || (!IS_NPC (player)
	      && (IS_IMMORTAL (player)
		  && !IS_SET (player->comm, COMM_HEARCLAN))))
	{
          if (is_ignoring(player,ch) || IS_SET (player->comm, COMM_QUIET))
	  {
            player = player->next;
	    continue;
	  }
	   if ((player->race == PC_RACE_NIDAE && !IS_IMMORTAL(player))|| (ch->race == PC_RACE_NIDAE && IS_IMMORTAL(player)))
	   {
  	     if (player->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(player))
	       cj_channel(player,bufn);
	     else act_new (buf, ch, bufn, player, TO_VICT, POS_DEAD);
	   }
	   else
	   {
	     if (player->Class == PC_CLASS_CHAOS_JESTER && !IS_IMMORTAL(player))
               cj_channel(player,str);
	     else act_new (buf, ch, str, player, TO_VICT, POS_DEAD);
	   }
	}
      player = player->next;
    }
}


void format_clan_symbol (int slot, char *buf, int width)
{
  int i, len, pad;
  if ((buf != NULL))
    {
      sprintf (buf, "%s", clan_table[slot].symbol);
      len = strlen (buf);
      pad = len + (width - color_strlen (buf));
      if (len < pad)
	{
	  for (i = len; i < pad; i++)
	    buf[i] = ' ';
	  buf[i] = '\0';
	}
    }
}


// Akamai 02/21/00 - Clans named after players. BAD
// This looks at the player directory and returns TRUE if the
// character name exists, and FALSE if the player is not found.
//
bool is_player_name (char *name)
{
  char pfile[MAX_INPUT_LENGTH];
  FILE *fp;
  bool found = FALSE;

  // create the string that corresponds to the file
  sprintf (pfile, "%s%c/%s", PLAYER_DIR, name[0], capitalize (name));
  fp = fopen (pfile, "r");

  // ok, if the file pointer is not null, that means
  // that there is a character named this, so we can't
  // let this guy create a clan of this name.
  if (fp != NULL)
    {
      fclose (fp);
      found = TRUE;
    }
  return (found);
}
