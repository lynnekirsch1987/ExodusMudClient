#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "olc.h"
#include "tables.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"
extern AREA_DATA *area_first;
extern ROOM_INDEX_DATA *room_index_hash[];
extern MOB_INDEX_DATA *mob_index_hash[];
extern const char *dir_name[];
extern int enc_shift;
extern bool is_encrypted;


void otsave args((FILE *fp, AREA_DATA *pArea));
void do_echo (CHAR_DATA * ch, char *argument);
inline void encrypt_write (char *encbuf, FILE * fp)
{
  char *c, tcbuf[MAX_STRING_LENGTH];
  int newc;
  strcpy (tcbuf, encbuf);
  c = tcbuf;
  while (*c != '\0')
    {
      if (is_encrypted)
	{
	  if (*c > 8 && *c < 127)
	    {
	      newc = *c + enc_shift;
	      if (newc > 126)
		newc = (newc - 126) + 8;
	    }
	  else
	    newc = *c;
	}
      else
	newc = *c;

      /*      fprintf(fp, "%c", newc); */
      fputc (newc, fp);
      c++;
    }
}
char *fix_string (const char *str)
{
  static char strfix[MAX_STRING_LENGTH];
  int i;
  int o;
  if (str == NULL)
    return '\0';
  for (o = i = 0; str[i + o] != '\0'; i++)
    {
      if (str[i + o] == '\r' || str[i + o] == '~')
	o++;
      strfix[i] = str[i + o];
    }
  strfix[i] = '\0';
  return strfix;
}


/*****************************************************************************
 Name:		save_area_list
 Purpose:	Saves the listing of files to be loaded at startup.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area_list ()
{
  FILE *fp;
  AREA_DATA *pArea;
  if ((fp = fopen ("area.lst", "w")) == NULL)
    {
      bug ("Save_area_list: fopen", 0);
      perror ("area.lst");
    }
  else
    {

      /*
       * Add any help files that need to be loaded at
       * startup to this section.
       */
      fprintf (fp, "help.are\n");
      fprintf (fp, "social.are\n");	/* ROM OLC */
      fprintf (fp, "rom.are\n");	/* ROM OLC */
      fprintf (fp, "group.are\n");	/* ROM OLC */
      fprintf (fp, "olc.hlp\n");
      for (pArea = area_first; pArea; pArea = pArea->next)
	{
	  fprintf (fp, "%s\n", pArea->filename);
	}

//      fprintf (fp, "obj_trig.are\n");   /* Object Triggers area */
      fprintf (fp, "$\n");
      fclose (fp);
    }
  return;
}


/*
 * ROM OLC
 * Used in save_mobile and save_object below.  Writes
 * flags on the form fread_flag reads.
 * 
 * buf[] must hold at least 32+1 characters.
 *
 */
char *fwrite_flag (long flags, char buf[])
{
  buf[0] = '\0';
  if (flags == 0)
    {
      strcpy (buf, "0");
      return buf;
    }
  if (flags < 0)
    {
      sprintf (buf, "%d", (int) flags);
      return buf;
    }
  if (flags & A)
    strcat (buf, "A");
  if (flags & B)
    strcat (buf, "B");
  if (flags & C)
    strcat (buf, "C");
  if (flags & D)
    strcat (buf, "D");
  if (flags & E)
    strcat (buf, "E");
  if (flags & F)
    strcat (buf, "F");
  if (flags & G)
    strcat (buf, "G");
  if (flags & H)
    strcat (buf, "H");
  if (flags & I)
    strcat (buf, "I");
  if (flags & J)
    strcat (buf, "J");
  if (flags & K)
    strcat (buf, "K");
  if (flags & L)
    strcat (buf, "L");
  if (flags & M)
    strcat (buf, "M");
  if (flags & N)
    strcat (buf, "N");
  if (flags & O)
    strcat (buf, "O");
  if (flags & P)
    strcat (buf, "P");
  if (flags & Q)
    strcat (buf, "Q");
  if (flags & R)
    strcat (buf, "R");
  if (flags & S)
    strcat (buf, "S");
  if (flags & T)
    strcat (buf, "T");
  if (flags & U)
    strcat (buf, "U");
  if (flags & V)
    strcat (buf, "V");
  if (flags & W)
    strcat (buf, "W");
  if (flags & X)
    strcat (buf, "X");
  if (flags & Y)
    strcat (buf, "Y");
  if (flags & Z)
    strcat (buf, "Z");
  if (flags & aa)
    strcat (buf, "a");
  if (flags & bb)
    strcat (buf, "b");
  if (flags & cc)
    strcat (buf, "c");
  if (flags & dd)
    strcat (buf, "d");
  if (flags & ee)
    strcat (buf, "e");
  return buf;
}


/*****************************************************************************
 Name:		save_mobile
 Purpose:	Save one mobile to file, new format -- Hugin
 Called by:	save_mobiles (below).
 ****************************************************************************/
void save_mobile (FILE * fp, MOB_INDEX_DATA * pMobIndex)
{
  char letter[16];
  char tcbuf[MAX_STRING_LENGTH];
  sh_int race = pMobIndex->race;
  char buf[MAX_STRING_LENGTH];
  int i=0;

#ifdef VERBOSE_SAVE
  sprintf (buf, "save_mobile: saving %s", pMobIndex->player_name);
  log_string (buf);

#endif /*  */
  sprintf (tcbuf, "#%d\n", pMobIndex->vnum);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", pMobIndex->player_name);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", pMobIndex->short_descr);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", fix_string (pMobIndex->long_descr));
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", fix_string (pMobIndex->description));
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", race_table[race].name);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s %d %d %d\n",
	   fwrite_flag (pMobIndex->recruit_flags, buf),
	   pMobIndex->recruit_value[0], pMobIndex->recruit_value[1],
	   pMobIndex->recruit_value[2]);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s ", fwrite_flag (pMobIndex->act, buf));
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s ", fwrite_flag (pMobIndex->affected_by, buf));
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%d %d\n", pMobIndex->alignment, pMobIndex->group);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%d ", pMobIndex->level);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%d ", pMobIndex->hitroll);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%dd%d+%d ", pMobIndex->hit[DICE_NUMBER],
	   pMobIndex->hit[DICE_TYPE], pMobIndex->hit[DICE_BONUS]);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%dd%d+%d ", pMobIndex->mana[DICE_NUMBER],
	   pMobIndex->mana[DICE_TYPE], pMobIndex->mana[DICE_BONUS]);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%dd%d+%d ", pMobIndex->damage[DICE_NUMBER],
	   pMobIndex->damage[DICE_TYPE], pMobIndex->damage[DICE_BONUS]);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s\n", attack_table[pMobIndex->dam_type].name);
  encrypt_write (tcbuf, fp);
/*  if (pMobIndex->act2 == 0)
  {
   sprintf (tcbuf, "%d %d %d %d\n", pMobIndex->ac[AC_PIERCE] / 10,
	   pMobIndex->ac[AC_BASH] / 10, pMobIndex->ac[AC_SLASH] / 10,
	   pMobIndex->ac[AC_EXOTIC] / 10);
   encrypt_write (tcbuf, fp);
  }
  else
  {*/
   sprintf (tcbuf, "%d %d %d %d %d\n", pMobIndex->ac[AC_PIERCE] / 10,
           pMobIndex->ac[AC_BASH] / 10, pMobIndex->ac[AC_SLASH] / 10,
           999, pMobIndex->ac[AC_EXOTIC] / 10);
   encrypt_write (tcbuf, fp);
   sprintf (tcbuf, "%s ", fwrite_flag (pMobIndex->act2, buf));
   encrypt_write (tcbuf, fp);
   if (pMobIndex->number_of_attacks != -1)
   {
     sprintf (tcbuf, "%d ", pMobIndex->blocks_exit+60);
     encrypt_write (tcbuf, fp);
     sprintf (tcbuf, "%d ", pMobIndex->number_of_attacks);
     encrypt_write (tcbuf, fp);
   }
   else
   {
     sprintf (tcbuf, "%d ", pMobIndex->blocks_exit);
     encrypt_write (tcbuf, fp);
   }
		
  sprintf (tcbuf, "%s ", fwrite_flag (pMobIndex->off_flags, buf));
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s ", fwrite_flag (pMobIndex->imm_flags, buf));
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s ", fwrite_flag (pMobIndex->res_flags, buf));
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s\n", fwrite_flag (pMobIndex->vuln_flags, buf));
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s %s %s %ld\n",
	   position_table[pMobIndex->start_pos].short_name,
	   position_table[pMobIndex->default_pos].short_name,
	   flag_string (sex_flags, pMobIndex->sex), pMobIndex->wealth);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s ", fwrite_flag (pMobIndex->form, buf));
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s ", fwrite_flag (pMobIndex->parts, buf));
  encrypt_write (tcbuf, fp);
  //Iblis 8/29/04 - Added setable stats (con/str/etc) + a slot for a rune card vnum
  for (i=0;i<MAX_STATS;i++)
  {
    sprintf (tcbuf, "%d ", pMobIndex->perm_stat[i]);
    encrypt_write(tcbuf, fp);
  }	  
  sprintf (tcbuf, "%d\n",pMobIndex->card_vnum);
  encrypt_write(tcbuf, fp);
  //Iblis end 8/29/04 additions
  switch (pMobIndex->size)
    {
    default:
      strcpy (letter, "medium");
      break;
    case SIZE_TINY:
      strcpy (letter, "tiny");
      break;
    case SIZE_SMALL:
      strcpy (letter, "small");
      break;
    case SIZE_LARGE:
      strcpy (letter, "large");
      break;
    case SIZE_HUGE:
      strcpy (letter, "huge");
      break;
    case SIZE_GIANT:
      strcpy (letter, "giant");
      break;
    }
  sprintf (tcbuf, "%s ", letter);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", pMobIndex->material);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%d %d %d %d\n", pMobIndex->defbonus,
	   pMobIndex->attackbonus, pMobIndex->max_weight, pMobIndex->move);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%d\n%s~\n", pMobIndex->default_mood, pMobIndex->vocfile);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", pMobIndex->script_fn);
  encrypt_write (tcbuf, fp);
  return;
}


/*****************************************************************************
 Name:		save_mobiles
 Purpose:	Save #MOBILES secion of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
void save_mobiles (FILE * fp, AREA_DATA * pArea)
{
  int i;
  MOB_INDEX_DATA *pMob;
  char tcbuf[MAX_STRING_LENGTH];
  sprintf (tcbuf, "#MOBILES\n");
  encrypt_write (tcbuf, fp);
  for (i = pArea->lvnum; i <= pArea->uvnum; i++)
    {
      if ((pMob = get_mob_index (i)))
	save_mobile (fp, pMob);
    }
  sprintf (tcbuf, "#0\n\n");
  encrypt_write (tcbuf, fp);
  return;
}


/*****************************************************************************
 Name:		save_object
 Purpose:	Save one object to file.
                new ROM format saving -- Hugin
 Called by:	save_objects (below).
 ****************************************************************************/
void save_object (FILE * fp, OBJ_INDEX_DATA * pObjIndex)
{
  char letter;
  AFFECT_DATA *pAf;
  EXTRA_DESCR_DATA *pEd;
  int i;

#ifdef VERBOSE_SAVE
  char verb[MAX_STRING_LENGTH];

#endif /*  */
  char buf[MAX_STRING_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];

#ifdef VERBOSE_SAVE
  sprintf (verb, "save_object: vnum: %d %s", pObjIndex->vnum,
	   pObjIndex->name);
  log_string (verb);

#endif /*  */
  sprintf (tcbuf, "#%d\n", pObjIndex->vnum);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", pObjIndex->name);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", pObjIndex->short_descr);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", fix_string (pObjIndex->description));
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", pObjIndex->material);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%d\n", pObjIndex->timer);
  encrypt_write (tcbuf, fp);

  // new_clans -- this is a fixup to handle new style donation boxes
  if (pObjIndex->item_type == ITEM_CLAN_DONATION)
    {
      pObjIndex->item_type = ITEM_NEWCLANS_DBOX;
    }
  sprintf (tcbuf, "%s ", item_name (pObjIndex->item_type));
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%d\n", MAX_EXTRA_FLAGS);
  encrypt_write (tcbuf, fp);
  for (i = 0; i < MAX_EXTRA_FLAGS;i++)
  {
    sprintf (tcbuf, "%s\n", fwrite_flag (pObjIndex->extra_flags[i], buf));
    encrypt_write (tcbuf, fp);
  }
  sprintf (tcbuf, "%s\n", fwrite_flag (pObjIndex->wear_flags, buf));
  encrypt_write (tcbuf, fp);

  // Akamai 4/30/99 - Support Class/race specific equipment
  // Area files that are version AREA_VER_CLASSRACE and greater
  // will include both Class_flags and race_flags in the objects
  sprintf (tcbuf, "%s\n", fwrite_flag (pObjIndex->Class_flags, buf));
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s\n", fwrite_flag (pObjIndex->race_flags, buf));
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s\n", fwrite_flag (pObjIndex->clan_flags, buf));
  encrypt_write (tcbuf, fp);
  
  // Iblis 1/1/04 - Support 16 obj trigger vnum on equipment
  // Area files that are version AREA_VER_OBJTRIG2 and greater
  // will include this vnum slow in the objects
   
  for (i=0;i<MAX_OBJ_TRIGS;i++)
  {
    sprintf (tcbuf, "%d\n", pObjIndex->obj_trig_vnum[i]);
    encrypt_write (tcbuf, fp);
  }
  
  sprintf (tcbuf, "%d\n", pObjIndex->rarity);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", pObjIndex->string1);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", pObjIndex->string2);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", pObjIndex->string3);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", pObjIndex->string4);
  encrypt_write (tcbuf, fp);

/*
 *  Using fwrite_flag to write most values gives a strange
 *  looking area file, consider making a case for each
 *  item type later.
 */
  sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[0], buf));
  encrypt_write (tcbuf, fp);
  switch (pObjIndex->item_type)
    {
    default:

#ifdef VERBOSE_SAVE
      sprintf (verb, "save_object: %s -> type: %d, a %s",
	       pObjIndex->name, pObjIndex->item_type,
	       item_name (pObjIndex->item_type));
      log_string (verb);

#endif /*  */
      sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[1], buf));
      encrypt_write (tcbuf, fp);
      sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[2], buf));
      encrypt_write (tcbuf, fp);
      sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[3], buf));
      encrypt_write (tcbuf, fp);
      sprintf (tcbuf, "%s\n", fwrite_flag (pObjIndex->value[4], buf));
      encrypt_write (tcbuf, fp);
      break;
    case ITEM_PILL:
    case ITEM_SCROLL:
    case ITEM_POTION:

#ifdef VERBOSE_SAVE
      sprintf (verb, "save_object: %s -> type: %d, a %s",
	       pObjIndex->name, pObjIndex->item_type,
	       item_name (pObjIndex->item_type));
      log_string (verb);

#endif /*  */
      if (pObjIndex->value[1] < 0 || pObjIndex->value[1] > MAX_SKILL)
	{
	  sprintf (tcbuf, "-1 ");
	}
      else
	{
	  sprintf (tcbuf, "'%s' ", skill_table[pObjIndex->value[1]].name);
	}
      encrypt_write (tcbuf, fp);
      if (pObjIndex->value[2] < 0 || pObjIndex->value[2] > MAX_SKILL)
	{
	  sprintf (tcbuf, "-1 ");
	}
      else
	{
	  sprintf (tcbuf, "'%s' ", skill_table[pObjIndex->value[2]].name);
	}
      encrypt_write (tcbuf, fp);
      if (pObjIndex->value[3] < 0 || pObjIndex->value[3] > MAX_SKILL)
	{
	  sprintf (tcbuf, "-1 ");
	}
      else
	{
	  sprintf (tcbuf, "'%s' ", skill_table[pObjIndex->value[3]].name);
	}
      encrypt_write (tcbuf, fp);
      if (pObjIndex->value[4] < 0 || pObjIndex->value[4] > MAX_SKILL)
	{
	  sprintf (tcbuf, "-1 ");
	}
      else
	{
	  sprintf (tcbuf, "'%s'\n", skill_table[pObjIndex->value[4]].name);
	}
      encrypt_write (tcbuf, fp);
      break;
    case ITEM_WAND:
    case ITEM_STAFF:

#ifdef VERBOSE_SAVE
      sprintf (verb, "save_object: %s -> type: %d, a %s",
	       pObjIndex->name, pObjIndex->item_type,
	       item_name (pObjIndex->item_type));
      log_string (verb);

#endif /*  */
      sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[1], buf));
      encrypt_write (tcbuf, fp);
      sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[2], buf));
      encrypt_write (tcbuf, fp);
      sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[3], buf));
      encrypt_write (tcbuf, fp);
      if (pObjIndex->value[4] < 0 || pObjIndex->value[4] > MAX_SKILL)
	{
	  sprintf (tcbuf, "-1 ");
	}
      else
	{
	  sprintf (tcbuf, "'%s'\n", skill_table[pObjIndex->value[4]].name);
	}
      encrypt_write (tcbuf, fp);
      break;
    
    case ITEM_CLAN_DONATION:	// new_clans
      // this is here in case this stuff gets through
      // the fixup above should do the job but I'm paranoid
#ifdef VERBOSE_SAVE
      sprintf (verb, "save_object: %s -> type: %d, a %s",
	       pObjIndex->name, pObjIndex->item_type,
	       item_name (pObjIndex->item_type));
      log_string (verb);

#endif /*  */
      sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[1], buf));
      encrypt_write (tcbuf, fp);
      sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[2], buf));
      encrypt_write (tcbuf, fp);
      sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[3], buf));
      encrypt_write (tcbuf, fp);
      sprintf (tcbuf, "%s\n", fwrite_flag (pObjIndex->value[4], buf));
      encrypt_write (tcbuf, fp);
      break;
    case ITEM_NEWCLANS_DBOX:	// new_clans
#ifdef VERBOSE_SAVE
      sprintf (verb, "save_object: %s -> type: %d, a %s",
	       pObjIndex->name, pObjIndex->item_type,
	       item_name (pObjIndex->item_type));
      log_string (verb);

#endif /*  */
      // this is paired with the load_objects read in db2.c
      // got to write items value[1] .. value[4]
      sprintf (tcbuf, "%s~\n", get_clan_name (pObjIndex->value[1]));
      encrypt_write (tcbuf, fp);
      sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[2], buf));
      encrypt_write (tcbuf, fp);
      sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[3], buf));
      encrypt_write (tcbuf, fp);
      sprintf (tcbuf, "%s\n", fwrite_flag (pObjIndex->value[4], buf));
      encrypt_write (tcbuf, fp);
      break;
    }
  sprintf (tcbuf, "%s\n", fwrite_flag (pObjIndex->value[5], buf));
  encrypt_write (tcbuf, fp);
  if (pObjIndex->item_type == ITEM_CARD)
  {
    if (pObjIndex->value[6] < 0 || pObjIndex->value[6] > MAX_SKILL)
    {
      sprintf (tcbuf, "-1 ");
    }
    else
    {
      sprintf (tcbuf, "'%s'\n", skill_table[pObjIndex->value[6]].name);
    }
    encrypt_write (tcbuf, fp);
    sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[7], buf));
    encrypt_write (tcbuf, fp);
    sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[8], buf));
    encrypt_write (tcbuf, fp);
    sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[9], buf));
    encrypt_write (tcbuf, fp);
    sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[10], buf));
    encrypt_write (tcbuf, fp);
    sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[11], buf));
    encrypt_write (tcbuf, fp);
    sprintf (tcbuf, "%s\n", fwrite_flag (pObjIndex->value[12], buf));
    encrypt_write (tcbuf, fp);
  } else if(pObjIndex->item_type == ITEM_OBJ_TRAP ||pObjIndex->item_type == ITEM_ROOM_TRAP ||pObjIndex->item_type == ITEM_PORTAL_TRAP )
  {
    sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[6], buf));
    encrypt_write (tcbuf, fp);
    sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[7], buf));
    encrypt_write (tcbuf, fp);
    sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[8], buf));
    encrypt_write (tcbuf, fp);
    sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[9], buf));
    encrypt_write (tcbuf, fp);
    sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[10], buf));
    encrypt_write (tcbuf, fp);
    sprintf (tcbuf, "%s ", fwrite_flag (pObjIndex->value[11], buf));
    encrypt_write (tcbuf, fp);
    sprintf (tcbuf, "%s \n", fwrite_flag (pObjIndex->value[12], buf));
    encrypt_write (tcbuf, fp);
  }
  else 
  {
    sprintf (tcbuf, "%s\n", fwrite_flag (pObjIndex->value[6], buf));
    encrypt_write (tcbuf, fp);
  }
  sprintf (tcbuf, "%d ", pObjIndex->level);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%d ", pObjIndex->weight);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%d ", pObjIndex->cost);
  encrypt_write (tcbuf, fp);
  if (pObjIndex->condition > 90)
    letter = 'P';

  else if (pObjIndex->condition > 75)
    letter = 'G';

  else if (pObjIndex->condition > 50)
    letter = 'A';

  else if (pObjIndex->condition > 25)
    letter = 'W';

  else if (pObjIndex->condition > 10)
    letter = 'D';

  else if (pObjIndex->condition > 0)
    letter = 'B';

  else
    letter = 'R';
  sprintf (tcbuf, "%c\n", letter);
  encrypt_write (tcbuf, fp);
  for (pAf = pObjIndex->affected; pAf; pAf = pAf->next)
    {
      if (pAf->type == -1)
      {
        sprintf (tcbuf, "A\n%d %d\n", pAf->location, pAf->modifier);
        encrypt_write (tcbuf, fp);
      }
      else
      {
	if (pAf->where != TO_SKILL || pAf->location == 0)
	{
	  sprintf (tcbuf, "S\n %d %d %d %d %d %d %d\n",
           pAf->where,
  	   pAf->type,
           pAf->level,
           pAf->duration,
           pAf->modifier,
           pAf->location,
           pAf->bitvector);
	   encrypt_write (tcbuf, fp);
	}
	else
	{
	 sprintf (tcbuf, "S\n %d %d %d %d %d '%s' %d\n",
		  //		  sprintf (tcbuf, "S\n %d %d %d %d %d %d %d\n",
		  pAf->where,
		  pAf->type,
		  pAf->level,
		  pAf->duration,
		  pAf->modifier,
		  skill_table[pAf->location].name,
		  pAf->bitvector);
	 encrypt_write (tcbuf, fp);
	 
	}
      }
    }
  for (pEd = pObjIndex->extra_descr; pEd; pEd = pEd->next)
    {
      sprintf (tcbuf, "E\n%s~\n%s~\n", pEd->keyword,
	       fix_string (pEd->description));
      encrypt_write (tcbuf, fp);
      if (pEd->next == pEd)
	break;
    }
  return;
}


/*****************************************************************************
 Name:		save_objects
 Purpose:	Save #OBJECTS section of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
void save_objects (FILE * fp, AREA_DATA * pArea)
{
  int i;
  OBJ_INDEX_DATA *pObj;
  char tcbuf[MAX_STRING_LENGTH];
  sprintf (tcbuf, "#OBJECTS\n");
  encrypt_write (tcbuf, fp);
  for (i = pArea->lvnum; i <= pArea->uvnum; i++)
    {
      if ((pObj = get_obj_index (i)))
	save_object (fp, pObj);
    }
  sprintf (tcbuf, "#0\n\n");
  encrypt_write (tcbuf, fp);
  return;
}


/*****************************************************************************
 Name:		save_rooms
 Purpose:	Save #ROOMS section of an area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_rooms (FILE * fp, AREA_DATA * pArea)
{
  ROOM_INDEX_DATA *pRoomIndex;
  EXTRA_DESCR_DATA *pEd;
  EXIT_DATA *pExit;
  int iHash;
  int door;
  char buf[MAX_STRING_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  sprintf (tcbuf, "#ROOMS\n");
  encrypt_write (tcbuf, fp);
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pRoomIndex = room_index_hash[iHash]; pRoomIndex;
	   pRoomIndex = pRoomIndex->next)
	{
	  if (pRoomIndex->area == pArea)
	    {

#ifdef VERBOSE_SAVE
	      sprintf (buf, "save_rooms: saving %s", pRoomIndex->name);
	      log_string (buf);

#endif /*  */
	      sprintf (tcbuf, "#%d\n", pRoomIndex->vnum);
	      encrypt_write (tcbuf, fp);
	      sprintf (tcbuf, "%d %ld\n", pRoomIndex->tp_level,
		       pRoomIndex->tp_exp);
	      encrypt_write (tcbuf, fp);
	      sprintf (tcbuf, "%s~\n", pRoomIndex->tp_msg);
	      encrypt_write (tcbuf, fp);
	      sprintf (tcbuf, "%s~\n%s~\n", pRoomIndex->enter_msg,
		       pRoomIndex->exit_msg);
	      encrypt_write (tcbuf, fp);
	      sprintf (tcbuf, "%ld %ld %d\n", pRoomIndex->Class_flags,
		       pRoomIndex->race_flags, pRoomIndex->max_level);
	      encrypt_write (tcbuf, fp);
	      sprintf (tcbuf, "%s~\n", pRoomIndex->name);
	      encrypt_write (tcbuf, fp);
	      sprintf (tcbuf, "%s~\n", fix_string (pRoomIndex->description));
	      encrypt_write (tcbuf, fp);
	      sprintf (tcbuf, "0 ");
	      encrypt_write (tcbuf, fp);
	      sprintf (tcbuf, "%s ",
		       fwrite_flag (pRoomIndex->room_flags, buf));
	      encrypt_write (tcbuf, fp);
	      sprintf (tcbuf, "%d\n", pRoomIndex->sector_type);
	      encrypt_write (tcbuf, fp);
	      sprintf (tcbuf, "%d\n%s~\n", pRoomIndex->max_in_room,
		       pRoomIndex->max_message);
	      encrypt_write (tcbuf, fp);
	      if (pRoomIndex->heal_rate != 100)
		{
		  sprintf (tcbuf, "H%d\n", pRoomIndex->heal_rate);
		  encrypt_write (tcbuf, fp);
		}
	      // Adeon 6/30/03 -- support new room flags and sinking rooms
	      sprintf (tcbuf, "I %s ",
		       fwrite_flag (pRoomIndex->room_flags2, buf));
	      encrypt_write (tcbuf, fp);
	      if (IS_SET (pRoomIndex->room_flags2, ROOM_SINKING))
		{
		  sprintf (tcbuf, "J %s~\n", pRoomIndex->sink_msg);
		  encrypt_write (tcbuf, fp);
		  sprintf (tcbuf, "K %s~\n", pRoomIndex->sink_warning);
		  encrypt_write (tcbuf, fp);
		  sprintf (tcbuf, "L%d\n", pRoomIndex->sink_timer);
		  encrypt_write (tcbuf, fp);
		  sprintf (tcbuf, "N%d\n", pRoomIndex->sink_dest);
		  encrypt_write (tcbuf, fp);
		  sprintf (tcbuf, "Q %s~\n", pRoomIndex->sink_msg_others);
                  encrypt_write (tcbuf, fp);
		}
	      // insert support for further room_flags2 flags here!!!
	      if (pRoomIndex->mana_rate != 100)
		{
		  sprintf (tcbuf, "M%d\n", pRoomIndex->mana_rate);
		  encrypt_write (tcbuf, fp);
		}
	      if (pRoomIndex->owner[0] != '\0')
		{
		  sprintf (tcbuf, "O %s~\n", pRoomIndex->owner);
		  encrypt_write (tcbuf, fp);
		}
	      if (pRoomIndex->clan != CLAN_BOGUS)
		{
		  sprintf (tcbuf, "C %s~\n",
			   get_clan_name (pRoomIndex->clan));
		  encrypt_write (tcbuf, fp);
		}
	      if (pRoomIndex->epl_filename != NULL)
		{
		  sprintf (tcbuf, "P %s~\n", pRoomIndex->epl_filename);
		  encrypt_write (tcbuf, fp);
		}
	      for (pEd = pRoomIndex->extra_descr; pEd; pEd = pEd->next)
		{
		  sprintf (tcbuf, "E\n%s~\n%s~\n", pEd->keyword,
			   fix_string (pEd->description));
		  encrypt_write (tcbuf, fp);
		}
	      for (door = 0; door < MAX_DIR; door++)
		{
		  if ((pExit = pRoomIndex->exit[door]) && pExit->u1.to_room)
		    {
		      int locks = 0;
		      // Iblis - 4/25/04 - A slightly better way to get save the flags we want.
		      locks = EX_ISDOOR | EX_PICKPROOF | EX_NOBASH | EX_NOPASS | EX_HARD | EX_INFURIATING;
		      locks &= pExit->rs_flags;
		      
/*		      switch (pExit->rs_flags)
			{
			case EX_ISDOOR:
			  locks = 1;
			  break;
			case EX_ISDOOR | EX_PICKPROOF:
			  locks = 2;
			  break;
			case EX_ISDOOR | EX_NOPASS:
			  locks = 3;
			  break;
			case EX_ISDOOR | EX_NOPASS | EX_PICKPROOF:
			  locks = 4;
			  break;
			case EX_ISDOOR | EX_NOBASH:
			  locks = 5;
			  break;
			case EX_ISDOOR | EX_NOBASH | EX_NOPASS:
			  locks = 6;
			  break;
			case EX_ISDOOR | EX_NOBASH | EX_PICKPROOF:
			  locks = 7;
			  break;
			case EX_ISDOOR | EX_NOPASS | EX_PICKPROOF | EX_NOBASH:
			  locks = 8;
			  break;
			}*/
		      sprintf (tcbuf, "D%d\n", pExit->orig_door);
		      encrypt_write (tcbuf, fp);
		      sprintf (tcbuf, "%s~\n",
			       fix_string (pExit->description));
		      encrypt_write (tcbuf, fp);
		      sprintf (tcbuf, "%s~\n", pExit->keyword);
		      encrypt_write (tcbuf, fp);
		      sprintf (tcbuf, "%d %d %d\n", locks, pExit->key,
			       pExit->u1.to_room->vnum);
		      encrypt_write (tcbuf, fp);
		    }
		}
	      sprintf (tcbuf, "S\n");
	      encrypt_write (tcbuf, fp);
	    }
	}
    }
  sprintf (tcbuf, "#0\n\n");
  encrypt_write (tcbuf, fp);
  return;
}


/*****************************************************************************
 Name:		save_specials
 Purpose:	Save #SPECIALS section of area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_specials (FILE * fp, AREA_DATA * pArea)
{
  int iHash;
  MOB_INDEX_DATA *pMobIndex;
  char tcbuf[MAX_STRING_LENGTH];
  sprintf (tcbuf, "#SPECIALS\n");
  encrypt_write (tcbuf, fp);
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pMobIndex = mob_index_hash[iHash]; pMobIndex;
	   pMobIndex = pMobIndex->next)
	{
	  if (pMobIndex && pMobIndex->area == pArea && pMobIndex->spec_fun)
	    {
	      sprintf (tcbuf, "M %d %s\t\t* %s\n", pMobIndex->vnum,
		       spec_string (pMobIndex->spec_fun),
		       pMobIndex->short_descr);
	      encrypt_write (tcbuf, fp);
	    }
	}
    }
  sprintf (tcbuf, "S\n\n");
  encrypt_write (tcbuf, fp);
  return;
}

void save_resets (FILE * fp, AREA_DATA * pArea)
{
  RESET_DATA *pReset;
  MOB_INDEX_DATA *pLastMob = NULL;
  OBJ_INDEX_DATA *pLastObj;
  ROOM_INDEX_DATA *pRoom;
  char buf[MAX_STRING_LENGTH];
  char tcbuf[MAX_STRING_LENGTH];
  sprintf (tcbuf, "#RESETS\n");
  encrypt_write (tcbuf, fp);
  for (pReset = pArea->reset_first; pReset; pReset = pReset->next)
    {

#ifdef VERBOSE_SAVE
      sprintf (buf, "save_resets: reset command %c", pReset->command);
      log_string (buf);

#endif /*  */
      switch (pReset->command)
	{
	default:
	  bug ("Save_resets: bad command %c.", pReset->command);
	  break;
	case 'D':
	  pRoom = get_room_index (pReset->arg1);
	  if (pRoom == NULL)
	    continue;
	  sprintf (tcbuf, "D 0 %4d %3d %4d\t* %s [%s]\n", pReset->arg1,
		   pReset->arg2, pReset->arg3, pRoom->name,
		   dir_name[pReset->arg2]);
	  encrypt_write (tcbuf, fp);
	  break;
	case 'M':
	  pLastMob = get_mob_index (pReset->arg1);
	  if (pLastMob == NULL)
	    continue;
	  sprintf (tcbuf, "M 0 %4d %3d %4d %2d\t* %s\n", pReset->arg1,
		   pReset->arg2, pReset->arg3, pReset->arg4,
		   capitalize (pLastMob->short_descr));
	  encrypt_write (tcbuf, fp);
	  break;
	case 'O':
	  pLastObj = get_obj_index (pReset->arg1);
	  pRoom = get_room_index (pReset->arg3);
	  if (pLastObj == NULL || pRoom == NULL)
	    continue;
	  sprintf (tcbuf, "O 0 %d   0 %d %d\t* %s at %s\n",
		   pReset->arg1, pReset->arg3, pReset->arg4,
		   capitalize (pLastObj->short_descr), pRoom->name);
	  encrypt_write (tcbuf, fp);
	  break;
	case 'P':
	  pLastObj = get_obj_index (pReset->arg1);
	  if (pLastObj == NULL)
	    continue;
	  sprintf (tcbuf, "P 0 %d %d %d %d\n", pReset->arg1,
		   pReset->arg2, pReset->arg3, pReset->arg4);
	  encrypt_write (tcbuf, fp);
	  break;
	case 'G':
	  sprintf (tcbuf, "G 0 %4d -1\t\t* %s to %s\n", pReset->arg1,
		   capitalize (get_obj_index (pReset->arg1)->short_descr),
		   pLastMob ? pLastMob->short_descr : "!NO_MOB!");
	  encrypt_write (tcbuf, fp);
	  if (!pLastMob)
	    {
	      sprintf (buf, "Save_resets: !NO_MOB! in [%s]", pArea->filename);
	      bug (buf, 0);
	    }
	  break;
	case 'E':
	  if (pLastMob == NULL)
	    continue;
	  sprintf (tcbuf, "E 0 %4d   -1 %4d\t* item to %s\n",
		   pReset->arg1, pReset->arg3,
		   capitalize (pLastMob->short_descr));
	  encrypt_write (tcbuf, fp);
	  break;
	case 'R':
	  pRoom = get_room_index (pReset->arg1);
	  if (pRoom == NULL)
	    continue;
	  sprintf (tcbuf, "R 0 %4d %3d\n", pReset->arg1, pReset->arg2);
	  encrypt_write (tcbuf, fp);
	  break;
	}

      /*      if (pReset == pArea->reset_last)
         break; */
    }
  sprintf (tcbuf, "S\n\n");
  encrypt_write (tcbuf, fp);
  return;
}


/*****************************************************************************
 Name:		save_shops
 Purpose:	Saves the #SHOPS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_shops (FILE * fp, AREA_DATA * pArea)
{
  SHOP_DATA *pShopIndex;
  MOB_INDEX_DATA *pMobIndex;
  int iTrade;
  int iHash;
  char tcbuf[MAX_STRING_LENGTH];
  sprintf (tcbuf, "#SHOPS\n");
  encrypt_write (tcbuf, fp);
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pMobIndex = mob_index_hash[iHash]; pMobIndex;
	   pMobIndex = pMobIndex->next)
	{
	  if (pMobIndex && pMobIndex->area == pArea && pMobIndex->pShop)
	    {
	      pShopIndex = pMobIndex->pShop;
	      sprintf (tcbuf, "%d ", pShopIndex->keeper);
	      encrypt_write (tcbuf, fp);
	      for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
		{
		  if (pShopIndex->buy_type[iTrade] != 0)
		    {
		      sprintf (tcbuf, "%2d ", pShopIndex->buy_type[iTrade]);
		      encrypt_write (tcbuf, fp);
		    }
		  else
		    {
		      sprintf (tcbuf, " 0 ");
		      encrypt_write (tcbuf, fp);
		    }
		}
	      sprintf (tcbuf, "\t%d %3d ", pShopIndex->profit_buy,
		       pShopIndex->profit_sell);
	      encrypt_write (tcbuf, fp);
	      sprintf (tcbuf, "\t%d %2d\t* %s\n", pShopIndex->open_hour,
		       pShopIndex->close_hour, pMobIndex->short_descr);
	      encrypt_write (tcbuf, fp);
	    }
	}
    }
  sprintf (tcbuf, "0\n\n");
  encrypt_write (tcbuf, fp);
  return;
}


/*****************************************************************************
 Name:		save_area
 Purpose:	Save an area, note that this format is new.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area (AREA_DATA * pArea)
{
  FILE *fp;

#ifdef VERBOSE_SAVE
  char buf[MAX_STRING_LENGTH];

#endif /*  */
  char tcbuf[MAX_STRING_LENGTH];
  int tpoints;
  fclose (fpReserve);
  if (strstr (pArea->filename, ".enc"))
    is_encrypted = TRUE;
  if (!(fp = fopen (pArea->filename, "w")))
    {
      bug ("Open_area: fopen", 0);

      //perror (pArea->filename);
      do_echo (descriptor_list->character,
	       "I don't THINK so!  Everyone laugh at the stupid imm. HAha!\r\n");
      return;
    }
#ifdef VERBOSE_SAVE
  sprintf (buf, "save_area: saving %s -> file %s", pArea->name,
	   pArea->filename);
  log_string (buf);

#endif /*  */
  sprintf (tcbuf, "#AREA\n %d~\n", AREA_VER_CURRENT);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", pArea->filename);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", pArea->name);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "{%2d %2d} %s  %s~\n", pArea->llev, pArea->ulev,
	   pArea->creator, pArea->name);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "{%2d %2d} %s\t%s", pArea->llev, pArea->ulev,
	   pArea->creator, pArea->name);
  free_string (pArea->credits);
  pArea->credits = str_dup (tcbuf);
  sprintf (tcbuf, "%s~\n%d %d\n", pArea->help, pArea->lvnum, pArea->uvnum);
  encrypt_write (tcbuf, fp);

/*  tpoints = pArea->points;
  if (IS_SET (pArea->area_flags,AREA_IMP_ONLY))
    tpoints += POINTS_IMP_ONLY;
  if (IS_SET (pArea->area_flags,AREA_NO_QUIT))
    tpoints += POINTS_NO_QUIT;
  if (IS_SET (pArea->area_flags,AREA_NO_REPOP_WIA))
    tpoints += POINTS_NO_REPOP_WIA;
  if (IS_SET (pArea->area_flags,AREA_NO_TREE))
    tpoints += POINTS_NO_TREE;*/
  // save and restore clan ownership as the clan name
  tpoints = pArea->area_flags;
  REMOVE_BIT(tpoints,AREA_CHANGED || AREA_ADDED || AREA_LOADING);
  sprintf (tcbuf, "%d %s %d\n", pArea->points, print_flags(tpoints),get_clan_ident (pArea->clan));
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%d %d\n", pArea->noclan, pArea->construct);
  encrypt_write (tcbuf, fp);
  sprintf (tcbuf, "%s~\n", pArea->helper);
  encrypt_write (tcbuf, fp);

#ifdef VERBOSE_SAVE
  log_string ("save_area: save_mobiles");

#endif /*  */
  save_mobiles (fp, pArea);

#ifdef VERBOSE_SAVE
  log_string ("save_area: save_objects");

#endif /*  */
  save_objects (fp, pArea);

#ifdef VERBOSE_SAVE
  log_string ("save_area: save_rooms");

#endif /*  */
  save_rooms (fp, pArea);

#ifdef VERBOSE_SAVE
  log_string ("save_area: save_resets");

#endif /*  */
  save_resets (fp, pArea);

#ifdef VERBOSE_SAVE
  log_string ("save_area: save_shops");

#endif /*  */
  save_shops (fp, pArea);

#ifdef VERBOSE_SAVE
  log_string ("save_area: save_specials");

#endif /*  */
  save_specials (fp, pArea);

#ifdef VERBOSE_SAVE
    log_string ("save_area: save_object_triggers");
#endif /*  */
  otsave(fp,pArea);
  
  sprintf (tcbuf, "#$\n");
  encrypt_write (tcbuf, fp);
  is_encrypted = FALSE;
  fclose (fp);

#ifdef VERBOSE_SAVE
  sprintf (buf, "save_area: completed %s -> %s", pArea->name,
	   pArea->filename);
  log_string (buf);

#endif /*  */
  fpReserve = fopen (NULL_FILE, "r");
  return;
}


/*****************************************************************************
 Name:		do_asave
 Purpose:	Entry point for saving area data.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_asave (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  AREA_DATA *pArea;
  FILE *fp;
  int value;
  if (!IS_BUILDER (ch))
    {
      send_to_char ("You do not have authorization to do that.\n\r", ch);
      return;
    }
  fp = NULL;
  if (!ch)
    {				/* Do an autosave */
      save_area_list ();
      for (pArea = area_first; pArea; pArea = pArea->next)
	{
	  save_area (pArea);
	  REMOVE_BIT (pArea->area_flags, AREA_CHANGED);
	}
      return;
    }
  smash_tilde (argument);
  strcpy (arg1, argument);
  if (arg1[0] == '\0')
    {
      send_to_char ("Syntax:\n\r"
		    "  asave <vnum>   - saves a particular area\n\r"
		    "  asave list     - saves the area.lst file\n\r"
		    "  asave area     - saves the area being edited\n\r"
		    "  asave changed  - saves all changed zones\n\r"
		    "  asave world    - saves the world! (db dump)\n\r", ch);
      return;
    }

  /* Snarf the value (which need not be numeric). */
  value = atoi (arg1);
  if (!(pArea = get_area_data (value)) && is_number (arg1))
    {
      send_to_char ("That area does not exist.\n\r", ch);
      return;
    }

  /* Save area of given vnum. */
  /* ------------------------ */
  if (is_number (arg1))
    {
      if (!is_builder (ch, pArea))
	{
	  send_to_char ("You are not that area's creator.\n\r", ch);
	  return;
	}
      save_area_list ();
      save_area (pArea);
      REMOVE_BIT (pArea->area_flags, AREA_CHANGED);
      send_to_char ("Area saved.\n\r", ch);
      return;
    }

  /* Save the world, only authorized areas. */
  /* -------------------------------------- */
  if (!str_cmp ("world", arg1))
    {
      save_area_list ();
      for (pArea = area_first; pArea; pArea = pArea->next)
	{

	  /* Builder must be assigned this area. */
	  if (!is_builder (ch, pArea))
	    continue;
	  save_area (pArea);
	  REMOVE_BIT (pArea->area_flags, AREA_CHANGED);
	}
      send_to_char ("You saved the world.\n\r", ch);
      return;
    }

  /* Save changed areas, only authorized areas. */
  /* ------------------------------------------ */
  if (!str_cmp ("changed", arg1))
    {
      char buf[MAX_INPUT_LENGTH];
      save_area_list ();
      send_to_char ("Saved zones:\n\r", ch);
      sprintf (buf, "None.\n\r");

      for (pArea = area_first; pArea; pArea = pArea->next)
	{

	  /* Builder must be assigned this area. */
	  if (!is_builder (ch, pArea))
	    continue;

	  /* Save changed areas. */
	  if (IS_SET (pArea->area_flags, AREA_CHANGED))
	    {
	      save_area (pArea);
	      sprintf (buf, "%24s - '%s'\n\r", pArea->name, pArea->filename);
	      send_to_char (buf, ch);
	      REMOVE_BIT (pArea->area_flags, AREA_CHANGED);
	    }
	}
      if (!str_cmp (buf, "None.\n\r"))
	send_to_char (buf, ch);
      return;
    }

  /* Save the area.lst file. */
  /* ----------------------- */
  if (!str_cmp (arg1, "list"))
    {
      save_area_list ();
      return;
    }

  /* Save area being edited, if authorized. */
  /* -------------------------------------- */
  if (!str_cmp (arg1, "area"))
    {

      /* Is character currently editing. */
      if (ch->desc->editor == 0)
	{
	  send_to_char ("You are not editing an area, "
			"therefore an area vnum is required.\n\r", ch);
	  return;
	}

      /* Find the area to save. */
      switch (ch->desc->editor)
	{
	case ED_AREA:
	  pArea = (AREA_DATA *) ch->desc->pEdit;
	  break;
	case ED_ROOM:
	  pArea = ch->in_room->area;
	  break;
	case ED_OBJECT:
	  pArea = ((OBJ_INDEX_DATA *) ch->desc->pEdit)->area;
	  break;
	case ED_MOBILE:
	  pArea = ((MOB_INDEX_DATA *) ch->desc->pEdit)->area;
	  break;
	default:
	  pArea = ch->in_room->area;
	  break;
	}
      if (!is_builder (ch, pArea))
	{
	  send_to_char ("You are not that area's creator.\n\r", ch);
	  return;
	}
      save_area_list ();
      save_area (pArea);
      REMOVE_BIT (pArea->area_flags, AREA_CHANGED);
      send_to_char ("Area saved.\n\r", ch);
      return;
    }

  /* Show correct syntax. */
  /* -------------------- */
  do_asave (ch, "");
  return;
}
