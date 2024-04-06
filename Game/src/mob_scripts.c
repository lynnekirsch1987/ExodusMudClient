#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"
#include "olc.h"
#include "obj_trig.h"
extern int top_vnum_room;
extern long int level_cost (int level);

int getbit args((char *explored, int index));
	

void do_submit (CHAR_DATA * ch, char *argument)
{
  send_to_char ("Please enter the mob's vnum:\n\r", ch);
  ch->desc->editor = SUBMIT_MOB_VNUM;
}

void do_mail (CHAR_DATA * ch, char *argument)
{
  char *mailto, tcbuf[MAX_STRING_LENGTH];
  if (argument[0] == '\0' || IS_NPC (ch))
    {
      send_to_char ("Send mail to who?\n\r", ch);
      return;
    }
  if ((mailto = get_player_email (argument)) == NULL)
    {
      send_to_char ("Sorry, no such player exists on Exodus.\n\r", ch);
      return;
    }
  sprintf (tcbuf, "From: exodus@mischief.com\nReply-To: %s\nTo: %s\n",
	   ch->pcdata->email_addr, mailto);
  ch->desc->submit_info = str_dup (tcbuf);
  send_to_char ("Subject:\n\r", ch);
  ch->desc->editor = MAIL_GET_SUBJECT;
}

int get_dir (char *arg)
{
  int door = MAX_DIR;
  if (!str_cmp (arg, "n") || !str_cmp (arg, "north"))
    door = DIR_NORTH;

  else if (!str_cmp (arg, "e") || !str_cmp (arg, "east"))
    door = DIR_EAST;

  else if (!str_cmp (arg, "s") || !str_cmp (arg, "south"))
    door = DIR_SOUTH;

  else if (!str_cmp (arg, "w") || !str_cmp (arg, "west"))
    door = DIR_WEST;

  else if (!str_cmp (arg, "u") || !str_cmp (arg, "up"))
    door = DIR_UP;

  else if (!str_cmp (arg, "d") || !str_cmp (arg, "down"))
    door = DIR_DOWN;
  return door;
}


/*
 * This little command helps a user debug the scripts online.  It can give
 * information about a single trigger, or general info on the mob's current
 * state of execution.
 */
void do_script (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  TRIGGER_DATA *trig;
  VARIABLE_DATA *var;
  argument = one_argument (argument, arg);
  one_argument (argument, arg2);
  if (arg[0] == '\0')
    {
      send_to_char ("Script whom (which trigger)?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      sprintf (buf, "Name: '%s'\n\rArea: [%3d] '%s'\n\r", NAME (victim),
	       victim->pIndexData->area->vnum,
	       victim->pIndexData->area->name);
      send_to_char (buf, ch);
      sprintf (buf, "Vnum: %d  Sex: %s  Room: %d\n\r",
	       victim->pIndexData->vnum,
	       victim->sex == SEX_MALE ? "male" : victim->sex ==
	       SEX_FEMALE ? "female" : "neutral",
	       victim->in_room == NULL ? 0 : victim->in_room->vnum);
      send_to_char (buf, ch);
    }
  if (arg2[0] != '\0')
    {
      for (trig = victim->triggers; trig != NULL; trig = trig->next)
	{
	  if (!str_prefix (trig->name, arg2))
	    break;
	}
      if (trig == NULL)
	send_to_char ("No such trigger.\n\r", ch);

      else
	{
	  SCRIPT_DATA *scr;
	  char buf[MAX_STRING_LENGTH];
	  for (scr = trig->script; scr != NULL; scr = scr->next)
	    {
	      if (scr == trig->current)
		sprintf (buf, "##\n\r%s   <--\n\r", scr->command);

	      else
		sprintf (buf, "##\n\r%s\n\r", scr->command);
	      send_to_char (buf, ch);
	    }
	}
    }
  else
    {
      for (trig = victim->triggers; (trig != NULL); trig = trig->next)
	{
	  char buf[MAX_STRING_LENGTH];
	  if (trig != NULL)
	    {
	      sprintf (buf,
		       "Trigger '%s' (key '%s')\n\r'%s' w/tra/typ: %d %d %d  %s %s\n\r",
		       trig->name, trig->keywords,
		       trig->current !=
		       NULL ? trig->current->command : "not running",
		       trig->waiting, trig->tracer, trig->trigger_type,
		       IS_SET (trig->bits,
			       SCRIPT_ADVANCE) ? "" : "stopped",
		       IS_SET (trig->bits, SCRIPT_HALT) ? "halted" : "");
	      send_to_char (buf, ch);
	    }
	}
      if (victim->variables != NULL)
	send_to_char ("Variables:\n\r", ch);
      for (var = victim->variables; (var != NULL); var = var->next)
	{
	  char buf[MAX_STRING_LENGTH];
	  if (var != NULL)
	    {
	      sprintf (buf, "'%s' = '%s'\n\r", var->name, var->value);
	      send_to_char (buf, ch);
	    }
	}
      if (IS_NPC (victim))
	{
	  sprintf (buf,
		   "Current spec tracer set to %d, loads with file '%s'\n\r",
		   victim->spec_tracer, victim->pIndexData->script_fn);
	  send_to_char (buf, ch);
	}
    }
  send_to_char ("\n\r", ch);
  return;
}


/*
 * This is a quick and dirty utility for adding a few fields to a trigger
 * quickly.  When a script is triggered, information is sent into variables
 * using this procedure.  The arg, from, to are preceded by the command
 * string, thus:
 * act_trigger( bill, "Argyle", "Red", NULL, NULL );
 * would put the following variables on the mob bill:
 * %Argyle-arg%      = Red
 */
void
act_trigger (CHAR_DATA * mob,
	     char *command, char *argument, char *from, char *to)
{
  char buf[MAX_STRING_LENGTH];
  if (!HAS_SCRIPT (mob) || command == NULL)
    return;
  if (from != NULL)
    {
      sprintf (buf, "%%%s-from%%", command);
      assign_variable (buf, from, mob);
    }
  if (to != NULL)
    {
      sprintf (buf, "%%%s-to%%", command);
      assign_variable (buf, to, mob);
    }
  if (argument != NULL)
    {
      sprintf (buf, "%%%s-arg%%", command);
      assign_variable (buf, argument, mob);
    }
  return;
}


/*
 * This searches a list of variables for a variable with a certain name,
 * returning NULL if the variable happens to not currently exist.
 */
VARIABLE_DATA *get_variable (char *var_name, CHAR_DATA * mob)
{
  VARIABLE_DATA *var;
  for (var = mob->variables; var != NULL; var = var->next)
    {
      if (!str_cmp (var_name, var->name))
	{
	  return var;
	}
    }
  return NULL;
}


/*
 * Unlike the above, this takes a variable name and returns a value string
 * no matter if the variable exists or not.  This is for func evaluations
 * and if-thens mostly.
 */
char *varval (char *var_name, CHAR_DATA * mob)
{
  VARIABLE_DATA *var;
  var = get_variable (var_name, mob);
  if (var == NULL)
    return str_dup ("");
  return var->value;
}


/*
 * Assigns or creates a variable with a value.   Translates interior variables.
 */
void assign_variable (char *var_name, char *value, CHAR_DATA * mob)
{
  VARIABLE_DATA *var;
  if ((var = get_variable (var_name, mob)) != NULL)
    {
      free_string (var->value);
      var->value = translate_variables (value, mob);
    }
  else
    {
      if (variable_free == NULL)
	{
	  var = alloc_perm (sizeof (*var));
//	  var = reinterpret_cast<VARIABLE_DATA *>(alloc_perm(sizeof(*var)));
	}
      else
	{
	  var = variable_free;
	  variable_free = variable_free->next;
	}
      var->next = mob->variables;
      mob->variables = var;
      var->name = str_dup (var_name);
      var->value = translate_variables (value, mob);
    }
  return;
}


/*
 * Translates all variables in a string, note the usage of replace_string()..
 * which is as good as any.
 */
char *translate_variables (char *argument, CHAR_DATA * mob)
{
  VARIABLE_DATA *var;
  char *newstr = str_dup (argument);
  for (var = mob->variables; var != NULL; var = var->next)
    {
      if (strcmp (newstr, var->name) == 0)
	return newstr;
      while (strstr (newstr, var->name))
	newstr = replace_string (newstr, var->name, var->value);
    }
  return newstr;
}

char *replace_string (char *orig, char *old, char *newstr)
{
  char xbuf[MAX_STRING_LENGTH];
  int i;
  if (orig == NULL || old == NULL)
    return FALSE;
  xbuf[0] = '\0';
  strcpy (xbuf, orig);
  if (strstr (orig, old) != NULL)
    {
      i = strlen (orig) - strlen (strstr (orig, old));
      xbuf[i] = '\0';
      strcat (xbuf, newstr);
      strcat (xbuf, &orig[i + strlen (old)]);
      if (orig != NULL)
	free_string (orig);
    }
  return (str_dup (xbuf));
}


/*
 * Used to get sequential lines of a multi line string (separated by "\n\r")
 * Thus its like one_argument(), but a trifle different. It is destructive
 * to the multi line string argument, and thus clist must not be shared.
 */
char *next_command (char *argument, char *arg_first)
{
  while (*argument != '\0')
    {
      if (*argument == '\n')
	{
	  argument++;
	  break;
	}
      *arg_first = *argument;
      arg_first++;
      argument++;
    }
  *arg_first = '\0';
  return argument;
}


/*
 * Evaluates multiple line command parses and does them all in one pulse.
 * Kinda kludgy, but useful.
 */
void multi_interpret (CHAR_DATA * mob, char *com_list)
{
  char buf[MAX_STRING_LENGTH];
  char cmd[MAX_STRING_LENGTH];
  while (com_list[0] != '\0')
    {
      com_list = next_command (com_list, buf);
      sprintf(cmd, "%s", buf);
      interpret (mob, buf);
    }
  return;
}


/*
 * Takes a plain and simple 1 + 2 + 4 * 2 and does a infix left to right
 * evaluation, returning the final numeric.
 */
int expression (char *ex, CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];
  char op[MAX_STRING_LENGTH];
  int num;
  int val;
  if (ex == NULL)
    return 0;
  ex = first_arg (ex, buf, FALSE);
  if (buf[0] == '%')
    strcpy (buf, varval (buf, ch));
  num = atoi (buf);
  while (ex[0] != '\0')
    {
      ex = first_arg (ex, op, TRUE);
      ex = first_arg (ex, buf, TRUE);
      if (buf[0] == '%')
	strcpy (buf, varval (buf, ch));
      val = atoi (buf);
      switch (op[0])
	{
	case '+':
	  num = num + val;
	  break;
	case '-':
	  num = num - val;
	  break;
	case '/':
	  if (val == 0)
	    num = 0;

	  else
	    num = num / val;
	  break;
	case '*':
	  num = num * val;
	  break;
	case '&':
	  num = num & val;
	  break;
	case '|':
	  num = num | val;
	  break;
	case '^':
	  num = num ^ val;
	  break;
	default:
	  break;
	}
    }
  return num;
}


/*
 * This procedure is the 'let' routine. It assigns variables on a mobile,
 * allowing the translation of expressions with variables in them.
 */
void eval_assign (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{
  char var[MAX_STRING_LENGTH];
  char op[MAX_STRING_LENGTH];
  char val[MAX_STRING_LENGTH];
  char num[MAX_STRING_LENGTH];
  char nval[MAX_STRING_LENGTH];
  argument = one_argument (argument, var);
  argument = first_arg (argument, op, TRUE);
  one_argument (argument, val);
  strcpy (val, argument);
  if (!str_cmp (op, "=="))
    assign_variable (var, val, ch);

  else if (!str_cmp (op, "="))
    {
      sprintf (num, "%d", expression (val, ch));
      assign_variable (var, num, ch);
    }
  else if (!str_cmp (op, "+="))
    {
      int iVal;
      int iOld;
      iVal = atoi (val);
      iOld =
	get_variable (var,
		      ch) == NULL ? 0 : atoi (get_variable (var, ch)->value);
      sprintf (val, "%d", iVal + iOld);
      assign_variable (var, val, ch);
    }
  else if (!str_cmp (op, "-="))
    {
      int iVal;
      int iOld;
      iVal = atoi (val);
      iOld =
	get_variable (var,
		      ch) == NULL ? 0 : atoi (get_variable (var, ch)->value);
      sprintf (val, "%d", iOld - iVal);
      assign_variable (var, val, ch);
    }
  else if (!str_cmp (op, "|="))
    {
      int iVal;
      int iOld;
      iVal = atoi (val);
      iOld =
	get_variable (var,
		      ch) == NULL ? 0 : atoi (get_variable (var, ch)->value);
      sprintf (val, "%d", iVal | iOld);
      assign_variable (var, val, ch);
    }
  else if (!str_cmp (op, "&="))
    {
      int iVal;
      int iOld;
      iVal = atoi (val);
      iOld =
	get_variable (var,
		      ch) == NULL ? 0 : atoi (get_variable (var, ch)->value);
      sprintf (val, "%d", iVal & iOld);
      assign_variable (var, val, ch);
    }
  else if (!str_cmp (op, "^="))
    {
      int iVal;
      int iOld;
      iVal = atoi (val);
      iOld =
	get_variable (var,
		      ch) == NULL ? 0 : atoi (get_variable (var, ch)->value);
      sprintf (val, "%d", iVal ^ iOld);
      assign_variable (var, val, ch);
    }
  else if (!str_cmp (op, "++"))
    {
      sprintf (nval, "%d", atoi (get_variable (var, ch)->value) + 1);
      assign_variable (var, nval, ch);
    }
  else if (!str_cmp (op, "--"))
    {
      sprintf (nval, "%d", atoi (get_variable (var, ch)->value) - 1);
      assign_variable (var, nval, ch);
    }
  return;
}


/*
 * the 'int' command -> does an interpret at the script level
 */
void eval_interp (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{
  char *cmd;
  int truss;
  cmd = translate_variables (argument, ch);
  truss = ch->trust;
  ch->trust = 100;
  SET_BIT (ch->comm, COMM_TRUE_TRUST);
  interpret (ch, cmd);
  REMOVE_BIT (ch->comm, COMM_TRUE_TRUST);
  ch->trust = truss;
  free_string (cmd);
  return;
}


/*
 * This procedure does a few different 'halt' functions:
 *
 * 1)   triggers  -> will halt the single trigger until mob dies
 * 2)   all       -> halts the whole mob
 * 3)   statement -> aborts a single 'Sc' statement from continuing
 * 4)   once      -> abort it this time
 */
bool eval_halt (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{
  char arg[MAX_STRING_LENGTH];
  one_argument (argument, arg);
  if (!str_cmp (arg, "once"))
    {
      trigger->current = NULL;
      return FALSE;
    }
  else if (!str_cmp (arg, "trigger"))
    {
      trigger->current = NULL;
      SET_BIT (trigger->bits, SCRIPT_HALT);
      return FALSE;
    }
  else if (!str_cmp (arg, "all"))
    {
      TRIGGER_DATA *Trig;
      SET_BIT (ch->act, ACT_HALT);
      for (Trig = ch->triggers; Trig != NULL; Trig = Trig->next)
	Trig->current = NULL;
      return FALSE;
    }
  else if (!str_cmp (arg, "statement"))
    return FALSE;
  return TRUE;
}


/*
 *  This procedure sets the pulse-wait state on mobs ( num = 2*arg1 )
 */
void eval_wait (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{
  char arg[MAX_STRING_LENGTH];
  int waittime;
  one_argument (argument, arg);
  if (arg[0] == '\0')
    return;
  if (is_number (arg))
    {
      trigger->waiting = atoi (arg);
      return;
    }
  else if (!str_cmp (arg, "time"))
    {
      argument = one_argument (argument, arg);
      if (!is_number (arg))
	return;
      waittime = atoi (arg);
      if (waittime != time_info.hour)
	trigger->tracer--;
    }
  return;
}


/*
 *  This goto evaluates things one of two ways:
 *  1)  Will go to a specific script entry (counting 0 as first)
 *  2)  Go to a label ( define labels as the first part of a script ': label')
 */
void eval_goto (TRIGGER_DATA * trigger, char *argument)
{
  char arg[MAX_STRING_LENGTH];
  SCRIPT_DATA *scr = trigger->script;
  one_argument (argument, arg);
  if (is_number (arg))
    {
      int count = 0;
      int jump_loc;
      jump_loc = atoi (arg);
      while (count < jump_loc && scr != NULL)
	{
	  count++;
	  scr = scr->next;
	}
      trigger->current = scr;
    }
  else
    {
      char arg2[MAX_STRING_LENGTH];
      char *cmd;
      while (scr != NULL)
	{
	  cmd = one_argument (scr->command, arg2);
	  if (!str_cmp (arg2, ":"))
	    {
	      cmd = one_argument (cmd, arg2);
	      if (!str_cmp (arg2, arg))
		break;
	    }
	  scr = scr->next;
	}
      if (scr == NULL)
	scr = trigger->script;
      trigger->current = scr;
    }
  return;
}


/*
 * Assigns a single value on a mobile:
 * usage:
 * mfunc <mobile/variable of mobile's name>  <value/variable w/ value> <what>
 */
void eval_msign (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int val;
  argument = one_argument (argument, arg1);	/* mob to assign */
  argument = one_argument_nl (argument, arg2);	/* value to assign */
  argument = one_argument (argument, arg3);	/* deliminator */
  if (arg1[0] == '%')
    strcpy (arg1, varval (arg1, ch));
  if (arg2[0] == '%')
    strcpy (arg2, varval (arg2, ch));
  victim = !str_cmp (arg1, "self") ? ch : get_char_world (ch, arg1);
  val = atoi (arg2);
  if (victim == NULL)
    {
      eval_halt (ch, trigger, "once");
      return;
    }
  else if (!str_cmp (arg3, "position"))
    victim->position = val;

  else if (!str_cmp (arg3, "mood"))
    victim->mood = val;

  else if (!str_cmp (arg3, "gold"))
    victim->gold = val;

  else if (!str_cmp (arg3, "silver"))
    victim->silver = val;

  else if (!str_cmp (arg3, "spectracer"))
    victim->spec_tracer = val;

  else if (!str_cmp (arg3, "exp"))
    victim->exp = val;

  else if (!str_cmp (arg3, "act"))
    victim->act = val;

  else if (!str_cmp (arg3, "affected_by"))
    victim->affected_by = val;

  else if (!str_cmp (arg3, "practice"))
    // victim->practice = val;
    val = val;

  else if (!str_cmp (arg3, "sex"))
    victim->sex = val;

  else if (!str_cmp (arg3, "race"))
    victim->race = val;

  else if (!str_cmp (arg3, "level"))
    victim->level = val;

  else if (!str_cmp (arg3, "align"))
    victim->alignment = val;

  else if (!str_cmp (arg3, "hit"))
    victim->hit = val;

  else if (!str_cmp (arg3, "maxhit"))
    victim->max_hit = val;

  else if (!str_cmp (arg3, "mana"))
    victim->mana = val;

  else if (!str_cmp (arg3, "maxmana"))
    victim->max_mana = val;

  else if (!str_cmp (arg3, "move"))
    victim->move = val;

  else if (!str_cmp (arg3, "short"))
    {
      char buf[MAX_STRING_LENGTH];
      sprintf (buf, "%s\n\r", arg2);
      free_string (victim->short_descr);
      victim->short_descr = str_dup (buf);
    }
  else if (!str_cmp (arg3, "long"))
    {
      char buf[MAX_STRING_LENGTH];
      sprintf (buf, "%s\n\r", arg2);
      free_string (victim->long_descr);
      victim->long_descr = str_dup (buf);
    }
  else if (!str_cmp (arg3, "maxmove"))
    victim->max_move = val;

  else if (!str_cmp (arg3, "str") && !IS_NPC (ch))
    victim->perm_stat[STAT_STR] = val;

  else if (!str_cmp (arg3, "int") && !IS_NPC (ch))
    victim->perm_stat[STAT_INT] = val;

  else if (!str_cmp (arg3, "wis") && !IS_NPC (ch))
    victim->perm_stat[STAT_WIS] = val;

  else if (!str_cmp (arg3, "dex") && !IS_NPC (ch))
    victim->perm_stat[STAT_DEX] = val;

  else if (!str_cmp (arg3, "con") && !IS_NPC (ch))
    victim->perm_stat[STAT_CON] = val;

  else if (!str_cmp (arg3, "cha") && !IS_NPC (ch))
    victim->perm_stat[STAT_CHA] = val;
  return;
}

void eval_osign (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  int val;
  argument = one_argument (argument, arg1);
  argument = one_argument_nl (argument, arg2);
  argument = one_argument (argument, arg3);
  if (arg1[0] == '%')
    strcpy (arg1, varval (arg1, ch));
  if (arg2[0] == '%')
    strcpy (arg2, varval (arg2, ch));
  obj = get_obj_world (ch, arg1);
  val = atoi (arg2);
  if (obj == NULL)
    {
      eval_halt (ch, trigger, "once");
      return;
    }
  else if (!str_cmp (arg3, "cost"))
    obj->cost = val;

  else if (!str_cmp (arg3, "wear_flags"))
    obj->wear_flags = val;

  else if (!str_cmp (arg3, "wear_loc"))
    obj->wear_loc = val;

  else if (!str_cmp (arg3, "weight"))
    obj->weight = val;

  else if (!str_cmp (arg3, "item_type"))
    obj->item_type = val;

  else if (!str_cmp (arg3, "level"))
    obj->level = val;

  else if (!str_cmp (arg3, "condition"))
    obj->condition = val;

  else if (!str_cmp (arg3, "short"))
    {
      free_string (obj->short_descr);
      obj->short_descr = str_dup (arg2);
    }
  else if (!str_cmp (arg3, "long"))
    {
      free_string (obj->description);
      obj->description = str_dup (arg2);
    }
  else if (!str_cmp (arg3, "material"))
    {
      free_string (obj->material);
      obj->material = str_dup (arg2);
    }
  else if (!str_cmp (arg3, "name"))
    {
      free_string (obj->name);
      obj->name = str_dup (arg2);
    }
  else if (!str_cmp (arg3, "timer"))
    obj->timer = val;

//IBLIS NOTE NOTE NOTE - This command only set the xtra flags in [0]
//CHANGE OR ADD if need to set other array elements
  else if (!str_cmp (arg3, "extra_flags"))
    obj->extra_flags[0] = val;

  else if (is_number (arg3))
    obj->value[URANGE (0, atoi (arg3), 4)] = val;
  return;
}

void eval_wsign (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  int val;
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);
  if (arg2[0] == '%')
    strcpy (arg2, varval (arg2, ch));
  val = atoi (arg2);
  if (!str_cmp (arg3, "hour"))
    time_info.hour = val;

  else if (!str_cmp (arg3, "day"))
    time_info.day = val;

  else if (!str_cmp (arg3, "month"))
    time_info.month = val;

  else if (!str_cmp (arg3, "year"))
    time_info.year = val;

  else if (!str_cmp (arg3, "sunlight"))
    weather_info.sunlight = val;

  else if (!str_cmp (arg3, "mmhg"))
    weather_info.mmhg = val;

  else if (!str_cmp (arg3, "weather"))
    weather_info.sky = val;
  return;
}


/*
 * Grabs world stuff/random numbers.
 * Usage:
 * wfunc %variable-to-go-in% <what> [<min random> <max random>]
 */
void eval_wfunc (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{
  char arg1[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg3);
  if (!str_cmp (arg3, "hour"))
    sprintf (buf, "%d", time_info.hour);

  else if (!str_cmp (arg3, "day"))
    sprintf (buf, "%d", time_info.day);

  else if (!str_cmp (arg3, "month"))
    sprintf (buf, "%d", time_info.month);

  else if (!str_cmp (arg3, "year"))
    sprintf (buf, "%d", time_info.year);

  else if (!str_cmp (arg3, "sunlight"))
    sprintf (buf, "%d", weather_info.sunlight);

  else if (!str_cmp (arg3, "mmhg"))
    sprintf (buf, "%d", weather_info.mmhg);

  else if (!str_cmp (arg3, "weather"))
    sprintf (buf, "%d", weather_info.sky);

  else if (!str_cmp (arg3, "topmob"))
    sprintf (buf, "%d", top_vnum_mob);

  else if (!str_cmp (arg3, "topobj"))
    sprintf (buf, "%d", top_vnum_obj);

  else if (!str_cmp (arg3, "toproom"))
    sprintf (buf, "%d", top_vnum_room);

  else if (!str_cmp (arg3, "random"))
    {
      char arg4[MAX_STRING_LENGTH];
      char arg5[MAX_STRING_LENGTH];
      argument = one_argument (argument, arg4);
      if (arg4[0] == '%')
	strcpy (arg4, varval (arg4, ch));
      argument = one_argument (argument, arg5);
      if (arg5[0] == '%')
	strcpy (arg5, varval (arg5, ch));
      sprintf (buf, "%d", number_range (atoi (arg4), atoi (arg5)));
    }
  else
    {
      short door;
      door = get_dir (arg3);
      if (door >= 0 && door < MAX_DIR)
	{
	  sprintf (buf, "%d", ch->in_room->exit[door] != NULL
		   && ch->in_room->exit[door]->u1.to_room !=
		   NULL ? ch->in_room->exit[door]->u1.to_room->
		   vnum : ROOM_VNUM_TEMPLATE);
	}
      else
	strcpy (buf, "0");
    }
  assign_variable (arg1, buf, ch);
  return;
}

void eval_mfunc (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  argument = one_argument (argument, arg1);	/* mob name */
  argument = one_argument (argument, arg2);	/* variable to put val in */
  argument = one_argument (argument, arg3);	/* deliminator */
  if (arg1[0] == '%')
    strcpy (arg1, varval (arg1, ch));
  victim = !str_cmp (arg1, "self") ? ch : get_char_world (ch, arg1);
  if (victim == NULL)
    sprintf (buf, "NOONE");

  else if (!str_cmp (arg3, "position"))
    sprintf (buf, "%d", victim->position);

  else if (!str_cmp (arg3, "gold"))
    sprintf (buf, "%ld", victim->gold);

  else if (!str_cmp (arg3, "silver"))
    sprintf (buf, "%ld", victim->silver);

  else if (!str_cmp (arg3, "name"))
    {
      sprintf (buf, "%s",
	       victim->name !=
	       NULL ? victim->name : victim->pIndexData->player_name);
      one_argument (buf, arg1);
      sprintf (buf, "%s", fcapitalize (arg1));
    }
  else if (!str_cmp (arg3, "room"))
    sprintf (buf, "%d", victim->in_room->vnum);

  else if (!str_cmp (arg3, "mood"))
    sprintf (buf, "%d", victim->mood);

  else if (!str_cmp (arg3, "short"))
    sprintf (buf, "%s", victim->short_descr);

  else if (!str_cmp (arg3, "long"))
    sprintf (buf, "%s", victim->long_descr);

  else if (!str_cmp (arg3, "spectracer"))
    sprintf (buf, "%d", victim->spec_tracer);

  else if (!str_cmp (arg3, "exp"))
    sprintf (buf, "%ld", victim->exp);

  else if (!str_cmp (arg3, "expneeded"))
    sprintf (buf, "%ld", IS_NPC (victim) ? -1 : level_cost (ch->level));

  else if (!str_cmp (arg3, "act"))
    sprintf (buf, "%ld", victim->act);

  else if (!str_cmp (arg3, "affected_by"))
    sprintf (buf, "%ld", victim->affected_by);

  else if (!str_cmp (arg3, "practice"))
    sprintf (buf, "0");

  else if (!str_cmp (arg3, "hitroll"))
    sprintf (buf, "%d", get_hitroll (victim));

  else if (!str_cmp (arg3, "damroll"))
    sprintf (buf, "%d", get_damroll (victim));

  else if (!str_cmp (arg3, "age"))
    sprintf (buf, "%d", get_age (victim));

  else if (!str_cmp (arg3, "sex"))
    sprintf (buf, "%d", victim->sex);

  else if (!str_cmp (arg3, "race"))
    sprintf (buf, "%d", victim->race);

  else if (!str_cmp (arg3, "Class"))
    sprintf (buf, "%d", victim->Class);

  else if (!str_cmp (arg3, "2Class"))
    sprintf (buf, "%d", victim->Class2);

  else if (!str_cmp (arg3, "3Class"))
    sprintf (buf, "%d", victim->Class3);

  else if (!str_cmp (arg3, "level"))
    sprintf (buf, "%d", victim->level);

  else if (!str_cmp (arg3, "align"))
    sprintf (buf, "%d", victim->alignment);

  else if (!str_cmp (arg3, "hit"))
    sprintf (buf, "%d", victim->hit);

  else if (!str_cmp (arg3, "maxhit"))
    sprintf (buf, "%d", victim->max_hit);

  else if (!str_cmp (arg3, "mana"))
    sprintf (buf, "%d", victim->mana);

  else if (!str_cmp (arg3, "maxmana"))
    sprintf (buf, "%d", victim->max_mana);

  else if (!str_cmp (arg3, "move"))
    sprintf (buf, "%d", victim->move);

  else if (!str_cmp (arg3, "maxmove"))
    sprintf (buf, "%d", victim->max_move);

  else if (!str_cmp (arg3, "str"))
    sprintf (buf, "%d", get_curr_stat (victim, STAT_STR));

  else if (!str_cmp (arg3, "int"))
    sprintf (buf, "%d", get_curr_stat (victim, STAT_INT));

  else if (!str_cmp (arg3, "dex"))
    sprintf (buf, "%d", get_curr_stat (victim, STAT_DEX));

  else if (!str_cmp (arg3, "wis"))
    sprintf (buf, "%d", get_curr_stat (victim, STAT_WIS));

  else if (!str_cmp (arg3, "con"))
    sprintf (buf, "%d", get_curr_stat (victim, STAT_CON));

  else if (!str_cmp (arg3, "cha"))
    sprintf (buf, "%d", get_curr_stat (victim, STAT_CHA));
  else if (!str_prefix ("qbit",arg3))
  {
    char qbuf[10];
    int i=0;
    if (arg3[4] != '\0')
      qbuf[i++] = arg3[4];
    if (arg3[5] != '\0')
      qbuf[i++] = arg3[5];
    if (arg3[6] != '\0')
      qbuf[i++] = arg3[6];
    qbuf[i] = '\0';
    i = atoi(qbuf);
    if (i < 0 || i > MAX_QUEST_BITS || IS_NPC(victim))
      i = -1;
    else i = (getbit(victim->pcdata->qbits,i));
    sprintf (buf, "%d",i);
  }
    

  else
    return;
  assign_variable (arg2, buf, ch);
  return;
}

void eval_chobj (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int ovnum;
  OBJ_DATA *obj;
  argument = one_argument (argument, arg1);	/* mob name */
  argument = one_argument (argument, arg2);	/* variable to put val in */
  argument = one_argument (argument, arg3);	/* deliminator */
  if (arg1[0] == '%')
    strcpy (arg1, varval (arg1, ch));
  victim = !str_cmp (arg1, "self") ? ch : get_char_world (ch, arg1);
  strcpy (buf, "0");
  if (victim == NULL)
    {
      assign_variable (arg2, buf, ch);
      return;
    }
  ovnum = atoi (arg3);
  for (obj = victim->carrying; obj; obj = obj->next_content)
    if (obj->pIndexData->vnum == ovnum)
      {
	strcpy (buf, "1");
	break;
      }
  assign_variable (arg2, buf, ch);
  return;
}

void eval_getch (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{
  char arg1[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *fch;
  int cnt = 0, rnd;
  strcpy (buf, "0");
  argument = one_argument (argument, arg1);	/* variable to put val in */
  for (fch = ch->in_room->people; fch; fch = fch->next_in_room)
    if (can_see (ch, fch) && ch != fch)
      cnt++;
  rnd = number_range (1, cnt) - 1;
  cnt = 0;
  for (fch = ch->in_room->people; fch; fch = fch->next_in_room)
    {
      if (!can_see (ch, fch) || ch == fch)
	continue;
      if (cnt == rnd)
	{
	  strcpy (buf, NAME (fch));
	  break;
	}
      cnt++;
    }
  assign_variable (arg1, buf, ch);
  return;
}

void eval_getpc (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{
  char arg1[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *fch;
  int cnt = 0, rnd;
  argument = one_argument (argument, arg1);	/* variable to put val in */
  strcpy (buf, "0");
  for (fch = ch->in_room->people; fch; fch = fch->next_in_room)
    if (!IS_NPC (fch) && can_see (ch, fch))
      cnt++;
  rnd = number_range (1, cnt) - 1;
  cnt = 0;
  for (fch = ch->in_room->people; fch; fch = fch->next_in_room)
    {
      if (IS_NPC (fch) || !can_see (ch, fch))
	continue;
      if (cnt == rnd)
	{
	  strcpy (buf, NAME (fch));
	  break;
	}
      cnt++;
    }
  assign_variable (arg1, buf, ch);
  return;
}

void eval_ofunc (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  argument = one_argument (argument, arg1);	/* object name */
  argument = one_argument (argument, arg2);	/* variable to put val in */
  argument = one_argument (argument, arg3);	/* deliminator */
  if (arg1[0] == '%')
    strcpy (arg1, varval (arg1, ch));
  obj = get_obj_world (ch, arg1);
  if (obj == NULL)
    sprintf (buf, "NOTHING");

  else if (!str_cmp (arg3, "cost"))
    sprintf (buf, "%d", obj->cost);

  else if (!str_cmp (arg3, "vnum"))
    sprintf (buf, "%d", obj->pIndexData->vnum);

  else if (!str_cmp (arg3, "short"))
    sprintf (buf, "%s", obj->short_descr);

  else if (!str_cmp (arg3, "long"))
    sprintf (buf, "%s", obj->description);

  else if (!str_cmp (arg3, "material"))
    sprintf (buf, "%s", obj->material);

  else if (!str_cmp (arg3, "name"))
    {
      sprintf (buf, "%s", STR (obj, name));
      one_argument (buf, arg1);
      sprintf (buf, "%s", arg1);
    }
  else if (!str_cmp (arg3, "room"))
    sprintf (buf, "%d", obj->in_room != NULL ? obj->in_room->vnum : 0);

  else if (!str_cmp (arg3, "inobj"))
    sprintf (buf, "%s",
	     obj->in_obj != NULL ? STR (obj->in_obj, name) : "none");

  else if (!str_cmp (arg3, "carried_by"))
    sprintf (buf, "%s",
	     obj->carried_by != NULL ? obj->carried_by->name !=
	     NULL ? obj->carried_by->name : obj->carried_by->
	     pIndexData->player_name : "none");

  else if (!str_cmp (arg3, "wear_flags"))
    sprintf (buf, "%ld", obj->wear_flags);

  else if (!str_cmp (arg3, "wear_loc"))
    sprintf (buf, "%ld", obj->wear_loc);

  else if (!str_cmp (arg3, "weight"))
    sprintf (buf, "%d", obj->weight);

  else if (!str_cmp (arg3, "item_type"))
    sprintf (buf, "%d", obj->item_type);

  else if (!str_cmp (arg3, "level"))
    sprintf (buf, "%d", obj->level);

  else if (!str_cmp (arg3, "timer"))
    sprintf (buf, "%d", obj->timer);

  else if (!str_cmp (arg3, "condition"))
    sprintf (buf, "%d", obj->condition);

  else if (!str_cmp (arg3, "extra_flags"))
    sprintf (buf, "%ld", obj->extra_flags[0]);

  else if (is_number (arg3))
    sprintf (buf, "%d", obj->value[URANGE (0, atoi (arg3), 4)]);

  else
    return;
  assign_variable (arg2, buf, ch);
  return;
}

void eval_sfunc (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{

/*    char  arg[MAX_STRING_LENGTH];    */
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char *st;
  int count;
  argument = one_argument (argument, arg1);	/* string */
  argument = one_argument (argument, arg2);	/* variable to put it in */
  argument = one_argument (argument, arg3);	/* arg deliminator */
  if (arg1[0] == '%')
    sprintf (arg1, "%s", varval (arg1, ch));
  if (!str_cmp (arg3, "money"))
    {
      sprintf (buf, "%d", atoi (arg1));
    }
  else if (is_number (arg3))
    {
      st = one_argument (arg1, buf);
      for (count = atoi (arg3); count > 0; count--)
	{
	  st = one_argument (st, buf);
	}
    }
  assign_variable (arg2, buf, ch);
  return;
}

bool boolchk (char *v1, char *v2, char *s)
{
  if (!str_cmp (s, "in"))
    return (str_infix (v1, v2));
  if (!str_cmp (s, "pre"))
    return (str_prefix (v1, v2));
  if (!str_cmp (s, "cmp"))
    return (!str_cmp (v1, v2));
  if (!str_cmp (s, "substr"))
    return (strstr (v2, v1) != NULL);
  if (!str_cmp (s, "num"))
    return (is_number (v1));
  if (!str_cmp (s, ">"))
    return (atoi (v1) > atoi (v2));
  if (!str_cmp (s, "<"))
    return (atoi (v1) < atoi (v2));
  if (!str_cmp (s, ">="))
    return (atoi (v1) >= atoi (v2));
  if (!str_cmp (s, "<="))
    return (atoi (v1) <= atoi (v2));
  if (!str_cmp (s, "=="))
    return (atoi (v1) == atoi (v2));
  if (!str_cmp (s, "!="))
    return (atoi (v1) != atoi (v2));
  if (!str_cmp (s, "&"))
    return (atoi (v1) & atoi (v2));
  if (!str_cmp (s, "|"))
    return (atoi (v1) | atoi (v2));
  if (!str_cmp (s, "isset"))
    return (IS_SET (atoi (v1), atoi (v2)));
  if (s[0] == '\0')
    return (atoi (v1));
  return FALSE;
}


#define C_AND     1
#define C_OR      2
#define C_NOT     4
bool eval_if (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char *arg;
  char v1[MAX_STRING_LENGTH];
  char v2[MAX_STRING_LENGTH];
  char s[MAX_STRING_LENGTH];
  bool ifstatement = TRUE;
  bool prev = TRUE;
  int andornot=0;
  while (argument[0] != '\0')
    {
      argument = first_arg (argument, buf, TRUE);
      arg = buf;
      
      while (arg[0] != '\0')
	{			/* do a single ( ) */
		
	  //andornot = 0;
	  arg = one_argument (arg, v1);
	  
	  
	  if (!str_cmp (v1, "not") || !str_cmp (v1, "!"))
	    {
	      arg = one_argument (arg, v1);
	      SET_BIT (andornot, C_NOT);
	      prev = ifstatement;
	      continue;
	    }
	  else if (!str_cmp (v1, "or") || !str_cmp (v1, "||"))
	    {
	      arg = one_argument (arg, v1);
	      SET_BIT (andornot, C_OR);
	      prev = ifstatement;
	       continue;
	    }
	  else if (!str_cmp (v1, "and") || !str_cmp (v1, "&&"))
	    {
	      arg = one_argument (arg, v1);
	      SET_BIT (andornot, C_AND);
	      prev = ifstatement;
	       continue;
	    }
	  else
	    {
	      arg = one_argument (arg, s);
	      arg = one_argument (arg, v2);
	      if (v1[0] == '%')
		strcpy (v1, varval (v1, ch));
	      if (v2[0] == '%')
		strcpy (v2, varval (v2, ch));
	      if (s[0] == '%')
		strcpy (s, varval (s, ch));
	      ifstatement = boolchk (v1, v2, s);
	    }
	  if (IS_SET (andornot, C_NOT))
	  {
	    ifstatement = !ifstatement;
	    andornot = 0;
	  }
	  if (IS_SET (andornot, C_AND))
	  {
	    ifstatement = ifstatement && prev;
	    andornot = 0;
	  }
	  if (IS_SET (andornot, C_OR))
	    {
	      ifstatement = ifstatement || prev;
	      andornot = 0;
	    }
	}
    }
  return ifstatement;
}


/*
bool 
eval_while(CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{ 
  char buf[MAX_STRING_LENGTH];
  char *arg;
  char v1[MAX_STRING_LENGTH];
  char v2[MAX_STRING_LENGTH];
  char s[MAX_STRING_LENGTH];
  bool ifstatement = TRUE;
  bool prev = TRUE;
  int andornot;

  while (argument[0] != '\0')
    {
      argument = first_arg(argument, buf, TRUE);
      arg = buf;

      while (arg[0] != '\0')	
	{
	  andornot = 0;
	  arg = one_argument(arg, v1);

	  if (!str_cmp(v1, "not") || !str_cmp(v1, "!"))
	    {
	      arg = one_argument(arg, v1);
	      SET_BIT(andornot, C_NOT);
	      prev = ifstatement;
	    }
	  else if (!str_cmp(v1, "or") || !str_cmp(v1, "||"))
	    {
	      arg = one_argument(arg, v1);
	      SET_BIT(andornot, C_OR);
	      prev = ifstatement;
	    }
	  else if (!str_cmp(v1, "and") || !str_cmp(v1, "&&"))
	    {
	      arg = one_argument(arg, v1);
	      SET_BIT(andornot, C_AND);
	      prev = ifstatement;
	    }
	  else
	    {
	      arg = one_argument(arg, s);
	      arg = one_argument(arg, v2);

	      if (v1[0] == '%')
		strcpy(v1, varval(v1, ch));
	      if (v2[0] == '%')
		strcpy(v2, varval(v2, ch));
	      if (s[0] == '%')
		strcpy(s, varval(s, ch));

	      ifstatement = boolchk(v1, v2, s);
	    }

	  if (IS_SET(andornot, C_NOT))
	    ifstatement = !ifstatement;
	  if (IS_SET(andornot, C_AND))
	    ifstatement = ifstatement && prev;
	  if (IS_SET(andornot, C_OR))
	    ifstatement = ifstatement || prev;

	}
    }

  if (!ifstatement)
    SET_BIT(trigger->bits, SCRIPT_ADVANCE);
  else
    REMOVE_BIT(trigger->bits, SCRIPT_ADVANCE);

  return ifstatement;
}
*/
#undef C_NOT
#undef C_AND
#undef C_OR
void eval_walkto (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  one_argument (argument, buf);
  if (ch->in_room->vnum == atoi (buf))
    return;
  ch->walkto = atoi (buf);
}

void eval_mob (CHAR_DATA * ch, TRIGGER_DATA * trigger, char *argument)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  char tcbuf[15];
  CHAR_DATA *victim;
  argument = one_argument (argument, arg1);	/* string */
  if (arg1[0] == '%')
    strcpy (arg1, varval (arg1, ch));
  argument = one_argument (argument, arg2);	/* string */
  if (arg2[0] == '%')
    strcpy (arg2, varval (arg2, ch));
  argument = one_argument (argument, arg3);	/* string */
  if (arg3[0] == '%')
    strcpy (arg3, varval (arg3, ch));
  victim = get_char_world (ch, arg2);
  if (!str_cmp (arg1, "push"))
    {
      char parg[MAX_STRING_LENGTH];
      sprintf (parg, "%s %s", arg2, arg3);
      push_char (ch, parg, TRUE);
    }
  else if (!str_cmp (arg1, "lock"))
    {
      int door;
      door = get_dir (arg2);
      if (ch->in_room->exit[door] == NULL
	  || !IS_SET (ch->in_room->exit[door]->exit_info, EX_ISDOOR))
	{
	  eval_halt (ch, trigger, "once");
	  return;
	}
      SET_BIT (ch->in_room->exit[door]->exit_info, EX_LOCKED);
      if (ch->in_room->exit[door]->u1.to_room != NULL
	  && ch->in_room->exit[door]->u1.to_room->exit[rev_dir[door]] !=
	  NULL && IS_SET (ch->in_room->exit[door]->u1.to_room->
			  exit[rev_dir[door]]->exit_info, EX_ISDOOR))
	SET_BIT (ch->in_room->exit[door]->u1.to_room->
		 exit[rev_dir[door]]->exit_info, EX_LOCKED);
    }
  else if (!str_cmp (arg1, "unlock"))
    {
      int door;
      door = get_dir (arg2);
      if (ch->in_room->exit[door] == NULL
	  || !IS_SET (ch->in_room->exit[door]->exit_info, EX_ISDOOR))
	{
	  eval_halt (ch, trigger, "once");
	  return;
	}
      REMOVE_BIT (ch->in_room->exit[door]->exit_info, EX_LOCKED);
      if (ch->in_room->exit[door]->u1.to_room != NULL
	  && ch->in_room->exit[door]->u1.to_room->exit[rev_dir[door]] !=
	  NULL && IS_SET (ch->in_room->exit[door]->u1.to_room->
			  exit[rev_dir[door]]->exit_info, EX_ISDOOR))
	REMOVE_BIT (ch->in_room->exit[door]->u1.to_room->
		    exit[rev_dir[door]]->exit_info, EX_LOCKED);
    }
  else if (!str_cmp (arg1, "open"))
    {
      int door;
      door = get_dir (arg2);
      if (ch->in_room->exit[door] == NULL
	  || !IS_SET (ch->in_room->exit[door]->exit_info, EX_ISDOOR))
	{
	  eval_halt (ch, trigger, "once");
	  return;
	}
      REMOVE_BIT (ch->in_room->exit[door]->exit_info, EX_CLOSED);
      if (ch->in_room->exit[door]->u1.to_room != NULL
	  && ch->in_room->exit[door]->u1.to_room->exit[rev_dir[door]] !=
	  NULL && IS_SET (ch->in_room->exit[door]->u1.to_room->
			  exit[rev_dir[door]]->exit_info, EX_ISDOOR))
	REMOVE_BIT (ch->in_room->exit[door]->u1.to_room->
		    exit[rev_dir[door]]->exit_info, EX_CLOSED);
    }
  else if (!str_cmp (arg1, "close"))
    {
      int door;
      door = get_dir (arg2);
      if (ch->in_room->exit[door] == NULL
	  || !IS_SET (ch->in_room->exit[door]->exit_info, EX_ISDOOR))
	{
	  eval_halt (ch, trigger, "once");
	  return;
	}
      SET_BIT (ch->in_room->exit[door]->exit_info, EX_CLOSED);
      if (ch->in_room->exit[door]->u1.to_room != NULL
	  && ch->in_room->exit[door]->u1.to_room->exit[rev_dir[door]] !=
	  NULL && IS_SET (ch->in_room->exit[door]->u1.to_room->
			  exit[rev_dir[door]]->exit_info, EX_ISDOOR))
	SET_BIT (ch->in_room->exit[door]->u1.to_room->
		 exit[rev_dir[door]]->exit_info, EX_CLOSED);
    }
  else if (!str_cmp (arg1, "trans"))
    {
      ROOM_INDEX_DATA *pRoom;
      if (victim == NULL || (pRoom = get_room_index (atoi (arg3))) == NULL)
	{
	  eval_halt (ch, trigger, "once");
	  return;
	}
      char_from_room (victim);
      char_to_room (victim, pRoom);
      trip_triggers(victim, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
      trap_check(victim,"room",victim->in_room,NULL);
      sprintf(tcbuf,"%d",victim->in_room->vnum);
      trip_triggers_arg(victim, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
      check_aggression(victim);
    }
  else if (!str_cmp (arg1, "cash"))
    {
      int cost;
      cost = atoi (arg3);
      if (victim == NULL)
	{
	  eval_halt (ch, trigger, "once");
	  return;
	}

      /*       if ( cost > 0 ) create_amount( cost, victim, NULL, NULL ); */
    }
  return;
}

void eval_signal (CHAR_DATA * ch, char *buf)
{
  CHAR_DATA *ach;
  char mob_str[MAX_STRING_LENGTH];
  char signal_name[MAX_STRING_LENGTH];
  int mob_vnum;
  buf = one_argument (buf, mob_str);
  one_argument (buf, signal_name);
  if (signal_name[0] == '%')
    strcpy (signal_name, varval (signal_name, ch));
  mob_vnum = atoi (mob_str);
  for (ach = char_list; ach; ach = ach->next)
    {
      if (!IS_NPC (ach))
	continue;
      if (mob_vnum == ach->pIndexData->vnum)
	{
	  if (HAS_SCRIPT (ach))
	    {
	      TRIGGER_DATA *pTrig;
	      for (pTrig = ach->triggers; pTrig != NULL; pTrig = pTrig->next)
		{
		  if (pTrig->trigger_type ==
		      TRIG_SIGNAL && pTrig->current ==
		      NULL && !IS_SET (ach->act, ACT_HALT))
		    {
		      act_trigger (ach, pTrig->name, signal_name,
				   NAME (ch), NULL);
		      pTrig->bits = SCRIPT_ADVANCE;
		      pTrig->current = pTrig->script;
		    }
		}
	    }
	}
    }
}
void script_interpret (CHAR_DATA * ch, TRIGGER_DATA * trigger)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char *cmd;
  if (trigger->current == NULL || trigger->current->command == NULL)
    return;
  trigger->tracer++;
  
  cmd = one_argument (trigger->current->command, buf2);
  while (cmd[0] != '\0')
    {
      cmd = next_command (cmd, buf);
      if (!str_cmp (buf2, "let"))
	eval_assign (ch, trigger, buf);

      else if (!str_cmp (buf2, "signal"))
	eval_signal (ch, buf);

      else if (!str_cmp (buf2, "walkto"))
	eval_walkto (ch, buf);

      else if (!str_cmp (buf2, "mob"))
	eval_mob (ch, trigger, buf);

      else if (!str_cmp (buf2, "ofunc"))
	eval_ofunc (ch, trigger, buf);

      else if (!str_cmp (buf2, "osign"))
	eval_osign (ch, trigger, buf);

      else if (!str_cmp (buf2, "mfunc"))
	eval_mfunc (ch, trigger, buf);

      else if (!str_cmp (buf2, "chobj"))
	eval_chobj (ch, trigger, buf);

      else if (!str_cmp (buf2, "getch"))
	eval_getch (ch, trigger, buf);

      else if (!str_cmp (buf2, "getpc"))
	eval_getpc (ch, trigger, buf);

      else if (!str_cmp (buf2, "msign"))
	eval_msign (ch, trigger, buf);

      else if (!str_cmp (buf2, "wfunc"))
	eval_wfunc (ch, trigger, buf);

      else if (!str_cmp (buf2, "wsign"))
	eval_wsign (ch, trigger, buf);

      else if (!str_cmp (buf2, "sfunc"))
	eval_sfunc (ch, trigger, buf);

      else if (!str_cmp (buf2, "jump"))
	eval_goto (trigger, buf);

      else if (!str_cmp (buf2, "wait"))
	eval_wait (ch, trigger, buf);

      else if (!str_cmp (buf2, "if"))
	{
	  if (!eval_if (ch, trigger, buf))
	    return;
	}
      else if (!str_cmp (buf2, "cmd"))
	{
	  eval_interp (ch, trigger, buf);
	}
      else if (!str_cmp (buf2, "halt"))
	{
	  if (!eval_halt (ch, trigger, buf))
	    return;
	}
      else
	{
	  strcat (buf2, " ");
	  strcat (buf2, buf);
	  eval_interp (ch, trigger, buf2);
	}
      cmd = one_argument (cmd, buf2);
    }
  return;
}


/*
 * Called from the mobile_update() procedure in update.c
 * This should be called every mobile pulse or so.
 */
void script_update (CHAR_DATA * ch, int timetype)
{
  TRIGGER_DATA *trig;
  if (!IS_NPC (ch))
    return;
  for (trig = ch->triggers; trig != NULL; trig = trig->next)
    {
      if (IS_SET (ch->act, ACT_HALT) || IS_SET (trig->bits, SCRIPT_HALT))
	{
	  trig->current = NULL;
	  continue;
	}
      if (trig->waiting > 0)
	{
	  trig->waiting--;
	  continue;
	}
      if (trig->trigger_type == timetype && trig->current == NULL)
	{
	  trig->current = trig->script;
	  trig->bits = SCRIPT_ADVANCE;
	}
      if (trig->current == NULL || !IS_SET (trig->bits, SCRIPT_ADVANCE))
	{
	  continue;
	}
      script_interpret (ch, trig);
      if (trig->current != NULL)
	trig->current = trig->current->next;

      else
	trig->tracer = 0;
    }
  return;
}


/*
 * BFS return values.
 */
#define BFS_ERROR           -1
#define BFS_ALREADY_THERE   -2
#define BFS_NO_PATH         -3
struct bfs_queue_struct
{
  ROOM_INDEX_DATA *room;
  sh_int dir;
  sh_int depth;
  struct bfs_queue_struct *next;
};
struct room_list_struct
{
  ROOM_INDEX_DATA *room;
  struct room_list_struct *next;
};
static struct bfs_queue_struct *queue_head = NULL, *queue_tail = NULL;
static struct room_list_struct *list_head = NULL, *list_tail = NULL;

/* Utility macros */
#define MARK( room )          ( SET_BIT( ( room )->room_flags, BFS_MARK))
#define UNMARK( room )        ( REMOVE_BIT( ( room )->room_flags, BFS_MARK))
#define IS_MARKED( room )     ( IS_SET( ( room )->room_flags, BFS_MARK))
#define TOROOM( room, y )     ( ( room )->exit[ ( y ) ]->u1.to_room )
#define IS_CLOSED( room, y )  ( IS_SET( ( room )->exit[( y )]->exit_info, \
    EX_CLOSED))
/* #define VALID_EDGE( room , y )                                             \
                              ( ( room )->exit[ ( y ) ] &&                 \
                              ( TOROOM( ( room ), ( y ) ) != NULL ) &&     \
                              ( !IS_CLOSED( (room), ( y ) ) ) &&           \
                              ( !IS_MARKED( TOROOM( ( room ), ( y ) ) ) ) )
			      */
inline int valid_edge (ROOM_INDEX_DATA * room, int dir, CHAR_DATA * ch)
{
  if (room->exit[dir] == NULL)
    return (0);
  if (room->exit[dir]->u1.to_room == NULL)
    return (0);
  if (IS_MARKED (TOROOM (room, dir)))
    return (0);
  if (IS_CLOSED (room, dir) && ch->huntto == 0)
    return (0);
  switch (TOROOM (room, dir)->sector_type)
    {
    case SECT_WATER_SWIM:
    case SECT_WATER_NOSWIM:
    case SECT_AIR:
      if (!is_affected (ch, skill_lookup ("fly")))
	return (0);
    }
  if (IS_SET (TOROOM (room, dir)->room_flags, ROOM_IMP_ONLY)
      || IS_SET (TOROOM (room, dir)->room_flags, ROOM_GODS_ONLY) ||
      /*        IS_SET(TOROOM(room, dir)->room_flags, ROOM_HEROES_ONLY) || */
      IS_SET (TOROOM (room, dir)->room_flags, ROOM_NEWBIES_ONLY))
    return (0);
  return (1);
}

void list_enqueue (ROOM_INDEX_DATA * room)
{
  static struct room_list_struct *curr;
  curr = alloc_mem (sizeof (struct room_list_struct));
//  curr = reinterpret_cast<room_list_struct *>(alloc_mem(sizeof(struct room_list_struct)));
  curr->room = room;
  curr->next = NULL;
  if (list_tail != NULL)
    {
      list_tail->next = curr;
      list_tail = curr;
    }
  else
    list_head = list_tail = curr;
  return;
}

void bfs_enqueue (ROOM_INDEX_DATA * room, sh_int dir, sh_int depth)
{
  struct bfs_queue_struct *curr;
  curr = alloc_mem (sizeof (struct bfs_queue_struct));
//  curr = reinterpret_cast<bfs_queue_struct *>(alloc_mem(sizeof(struct bfs_queue_struct)));
  curr->room = room;
  curr->dir = dir;
  curr->depth = depth + 1;
  curr->next = NULL;
  if (queue_tail != NULL)
    {
      queue_tail->next = curr;
      queue_tail = curr;
    }
  else
    queue_head = queue_tail = curr;
  list_enqueue (room);
  return;
}

void bfs_dequeue (void)
{
  struct bfs_queue_struct *curr;
  curr = queue_head;
  if ((queue_head = queue_head->next) == NULL)
    queue_tail = NULL;
  free_mem (curr, sizeof (struct bfs_queue_struct));
  return;
}

void list_dequeue (void)
{
  struct room_list_struct *curr;
  curr = list_head;
  if ((list_head = list_head->next) == NULL)
    list_tail = NULL;
  UNMARK (curr->room);
  free_mem (curr, sizeof (struct room_list_struct));
  return;
}

void bfs_clear_queue (void)
{
  while (queue_head != NULL)
    bfs_dequeue ();
  return;
}

void list_clear_queue (void)
{
  while (list_head != NULL)
    list_dequeue ();
}


/* find_first_step: given a source room and a target room, find the first
   step on the shortest path from the source to the target.

   Intended usage: in mobile_activity, give a mob a dir to go if they're
   tracking another mob or a PC.  Or, a 'track' skill for PCs.
*/
int
find_first_step (ROOM_INDEX_DATA * src, ROOM_INDEX_DATA * target,
		 CHAR_DATA * ch)
{
  int curr_dir;
  if (src == NULL || target == NULL)
    {
      bug ("Illegal value passed to find_first_step", 0);
      return BFS_ERROR;
    }
  if (src == target)
    return BFS_ALREADY_THERE;
  queue_head = queue_tail = NULL;
  list_head = list_tail = NULL;
  MARK (src);
  list_enqueue (src);
  for (curr_dir = 0; curr_dir < 6; curr_dir++)
    if (valid_edge (src, curr_dir, ch))
      {
	MARK (TOROOM (src, curr_dir));
	bfs_enqueue (TOROOM (src, curr_dir), curr_dir, 0);
      }

  /* now, do the Classic BFS. */
  while (queue_head)
    {
      if (queue_head->depth >= 1000)
	{
	  bfs_clear_queue ();
	  list_clear_queue ();
	  return BFS_NO_PATH;
	}
      if (queue_head->room == target)
	{
	  curr_dir = queue_head->dir;
	  bfs_clear_queue ();
	  list_clear_queue ();
	  return curr_dir;
	}
      else
	{
	  for (curr_dir = 0; curr_dir < MAX_DIR; curr_dir++)
	    {
	      if (valid_edge (queue_head->room, curr_dir, ch))
		{
		  MARK (TOROOM (queue_head->room, curr_dir));
		  bfs_enqueue (TOROOM (queue_head->room, curr_dir),
			       queue_head->dir, queue_head->depth);
		}
	    }
	  bfs_dequeue ();
	}
    }
  list_clear_queue ();
  return BFS_NO_PATH;
}
