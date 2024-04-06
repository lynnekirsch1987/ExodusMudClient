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
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"

#if !defined(macintosh)
extern int _filbuf args ((FILE *));
#endif

extern bool nowhere;
extern bool is_encrypted;
extern bool deathrestore;
extern int clan_number;
extern int roomcount args ((CHAR_DATA * ch));
CHAR_DATA *loadmount;
bool make_backup = FALSE;
bool make_backup_duel = FALSE;
int rename (const char *oldfname, const char *newfname);
bool load_char_obj_new (DESCRIPTOR_DATA * d, char *name, char* filename);
void free_ot args((OBJ_TRIG *ot));
unsigned long exodus_score args((CHAR_DATA * ch));
void save_statlist args((void));
extern bool is_linkloading;

	
	
OBJ_TRIG *new_ot args((int vnum));
#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}


char *print_flags (int flag)
{
  int count, pos = 0;
  static char buf[52];


  for (count = 0; count < 32; count++)
    {
      if (IS_SET (flag, 1 << count))
	{
	  if (count < 26)
	    buf[pos] = 'A' + count;
	  else
	    buf[pos] = 'a' + (count - 26);
	  pos++;
	}
    }

  if (pos == 0)
    {
      buf[pos] = '0';
      pos++;
    }

  buf[pos] = '\0';

  return buf;
}


/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];



/*
 * Local functions.
 */
void fwrite_char args ((CHAR_DATA * ch, FILE * fp));
void fwrite_obj args ((CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest));
void fwrite_pet args ((CHAR_DATA * pet, FILE * fp));
void fread_char args ((CHAR_DATA * ch, FILE * fp));
void fread_pet args ((CHAR_DATA * ch, FILE * fp));
void fread_obj args ((CHAR_DATA * ch, FILE * fp));
extern int getbit args ((char *explored, int index));

/* Writes sword data for reavers, to allow them to recall their sword */
void fwrite_sword (OBJ_DATA * obj, FILE * fp)
{
  char buf[MAX_STRING_LENGTH];
  EXTRA_DESCR_DATA *ed;
  AFFECT_DATA *paf;
  int i;

  //IBLIS 6/1/03 - Should Prevent the set reaver save crash bug
  if (obj == NULL)
    return;

  fprintf (fp, "\n#SWORD\n");

  sprintf (buf, "Vnum %d\n", obj->pIndexData->vnum);
  encrypt_write (buf, fp);

  encrypt_write ("Nest 0\n", fp);
  if (obj->enchanted)
    {
      sprintf (buf, "Enchanted\n");
      encrypt_write (buf, fp);
    }



  /* these data are only used if they do not match the defaults */

  if (obj->name != obj->pIndexData->name)
    {
      sprintf (buf, "Name %s~\n", obj->name);
      encrypt_write (buf, fp);
    }

  if (obj->owner != NULL && obj->owner[0] != '\0')
    {
      sprintf (buf, "OwnR %s~\n", obj->owner);
      encrypt_write (buf, fp);
    }

  sprintf (buf, "ShD  %s~\n", obj->short_descr);
  encrypt_write (buf, fp);

  sprintf (buf, "Desc %s~\n", obj->description);
  encrypt_write (buf, fp);

  for (i = 0; i < MAX_EXTRA_FLAGS; i++)
  {
    if (obj->extra_flags[i] != obj->pIndexData->extra_flags[i])
      {
        sprintf (buf, "ExF %d %ld\n", i, obj->extra_flags[i]);
        encrypt_write (buf, fp);
      }
  }

  if (obj->wear_flags != obj->pIndexData->wear_flags)
    {
      sprintf (buf, "WeaF %ld\n", obj->wear_flags);
      encrypt_write (buf, fp);
    }

  sprintf (buf, "BS2Cap");
  encrypt_write (buf, fp);
  for (i = 0; i < MAX_BLADE_SPELLS; i++)
    {
      sprintf (buf, " %d", obj->bs_capacity[i]);
      encrypt_write (buf, fp);
    }
  sprintf (buf, "\n");
  encrypt_write (buf, fp);

  sprintf (buf, "BS2Char");
  encrypt_write (buf, fp);
  for (i = 0; i < MAX_BLADE_SPELLS; i++)
    {
      sprintf (buf, " %d", obj->bs_charges[i]);
      encrypt_write (buf, fp);
    }
  sprintf (buf, "\n");
  encrypt_write (buf, fp);

  sprintf (buf, "BBCap");
  encrypt_write (buf, fp);
  for (i = 0; i < MAX_BURST; i++)
    {
      sprintf (buf, " %d", obj->bb_capacity[i]);
      encrypt_write (buf, fp);
    }
  sprintf (buf, "\n");
  encrypt_write (buf, fp);

  sprintf (buf, "BBChar");
  encrypt_write (buf, fp);
  for (i = 0; i < MAX_BURST; i++)
    {
      sprintf (buf, " %d", obj->bb_charges[i]);
      encrypt_write (buf, fp);
    }
  sprintf (buf, "\n");
  encrypt_write (buf, fp);


  // Akamai 4/30/99 - support for Class/race specific eq
  if (obj->Class_flags != obj->pIndexData->Class_flags)
    {
      sprintf (buf, "ClassF %ld\n", obj->Class_flags);
      encrypt_write (buf, fp);
    }
  if (obj->race_flags != obj->pIndexData->race_flags)
    {
      sprintf (buf, "RaceF %ld\n", obj->race_flags);
      encrypt_write (buf, fp);
    }
  if (obj->clan_flags != obj->pIndexData->clan_flags)
    {
      sprintf (buf, "ClanF %ld\n", obj->clan_flags);
      encrypt_write (buf, fp);
    }
  //Iblis 6/12/04 - Store the slot a card is in in a binder/deck
  if (obj->bs_capacity[0] != 0)
    {
      sprintf (buf, "Cardslot %d\n", obj->bs_capacity[0]);
      encrypt_write (buf, fp);
    }
  if (obj->item_type != obj->pIndexData->item_type)
    {
      sprintf (buf, "Ityp %d\n", obj->item_type);
      encrypt_write (buf, fp);
    }

  if (obj->weight != obj->pIndexData->weight)
    {
      sprintf (buf, "Wt   %d\n", obj->weight);
      encrypt_write (buf, fp);
    }

  if (obj->condition != obj->pIndexData->condition)
    {
      sprintf (buf, "Cond %d\n", obj->condition);
      encrypt_write (buf, fp);
    }

  /* variable data */

  sprintf (buf, "WearL %ld\n", obj->wear_loc);
  encrypt_write (buf, fp);
  if (obj->level != obj->pIndexData->level)
    {
      sprintf (buf, "Lev  %d\n", obj->level);
      encrypt_write (buf, fp);
    }
  if (obj->timer != 0)
    {
      sprintf (buf, "Time %d\n", obj->timer);
      encrypt_write (buf, fp);
    }


  sprintf (buf, "Cost %d\n", obj->cost);
  encrypt_write (buf, fp);

  if (obj->value[0] != obj->pIndexData->value[0]
      || obj->value[1] != obj->pIndexData->value[1]
      || obj->value[2] != obj->pIndexData->value[2]
      || obj->value[3] != obj->pIndexData->value[3]
      || obj->value[4] != obj->pIndexData->value[4]
      || obj->value[5] != obj->pIndexData->value[5]
      || obj->value[6] != obj->pIndexData->value[6]
      || obj->value[7] != obj->pIndexData->value[7]
      || obj->value[8] != obj->pIndexData->value[8]
      || obj->value[9] != obj->pIndexData->value[9]
      || obj->value[10] != obj->pIndexData->value[10]
      || obj->value[11] != obj->pIndexData->value[11]
      || obj->value[12] != obj->pIndexData->value[12]
      )
    {
      sprintf (buf, "Vlues  %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
	       obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	       obj->value[4], obj->value[5], obj->value[6], obj->value[7],
	       obj->value[8], obj->value[9], obj->value[10], obj->value[11], obj->value[12] );
      encrypt_write (buf, fp);
    }

  for (paf = obj->affected; paf != NULL; paf = paf->next)
    {
      AFFECT_DATA *taf=NULL;
      if (paf->type < 0 || paf->type >= MAX_SKILL)
	continue;
      for (taf = obj->pIndexData->affected;taf != NULL;taf = taf->next)
      {
        if (taf->type == paf->type)
          break;
      }
      if (taf != NULL && taf->type == paf->type)
        continue;
      sprintf (buf, "Affc '%s' %3d %3d %3d %3d %3d %10d %d '%s'\n",
	       skill_table[paf->type].name,
	       paf->where,
	       paf->level,
	       paf->duration,
	       paf->modifier,
	       paf->location,
	       paf->bitvector,
	       paf->composition, paf->comp_name);
      encrypt_write (buf, fp);
    }

  for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
    {
      sprintf (buf, "ExDe %s~ %s~\n", ed->keyword, ed->description);
      encrypt_write (buf, fp);
    }

  sprintf (buf, "End\n\n");
  encrypt_write (buf, fp);
//end target
  return;
}



/* Called from save_char_obj to write out RLE data */
void fwrite_rle (char *explored, FILE * fp, char* name)
{
  int index;
  /* The first bit is assumed to be zero */
  int bit = 0;
  int count = 0;
  int max = 0;
  fprintf (fp, "#%s  ",name);
//  fprintf (fp, "#RoomRLE  ");
  if (!str_cmp(name,"QbitsRLE"))
	  max = MAX_QUEST_BITS/8;
  else max = MAX_EXPLORE;

  /* Cycle through every bit */
  for (index = 0; index < (max * 8) - 1; index++)
    {
      /* If the bit is the same as last time, add to the count */
      if (getbit (explored, index) == bit)
	{
	  count++;
	}
      else
	{
	  /* Otherwise we write this length out and start over */
	  fprintf (fp, " %d", count);
	  count = 1;
	  bit = getbit (explored, index);
	}
    }
  /* Write out any left over run, -1 terminates */
  fprintf (fp, " %d -1\n\n", count);
}

void fread_sword (CHAR_DATA * ch, FILE * fp)
{
  OBJ_DATA *obj;
  const char *word;
  int iNest;
  bool fMatch;
  bool fNest;
  bool fVnum;
  bool first;
  bool make_new;
  int i;
  /* update object */

  fVnum = FALSE;
  obj = NULL;
  first = TRUE;			/* used to counter fp offset */
  make_new = FALSE;

  obj = create_object (get_obj_index (OBJ_VNUM_REAVER_SWORD), -1);



  log_string ("Reading in sword data");
  word = feof(fp) ? "End" : fread_word(fp);
  if (!str_cmp (word, "Vnum")) {
      int vnum;
      first = FALSE;		/* fp will be in right place */

      vnum = fread_number (fp);
      if (get_obj_index (vnum) == NULL)
	{
	  bug ("Fread_sword: bad vnum %d.", vnum);
	}

    }

  if (obj == NULL)
    {				/* either not found or old style */
      obj = new_obj ();
      obj->name = str_dup ("");
      obj->short_descr = str_dup ("");
      obj->description = str_dup ("");
      //Shinowlan 5/18/98 -- Part of the "spammed ExDe" fix
      obj->extra_descr = NULL;
    }


  fNest = FALSE;
  fVnum = TRUE;
  iNest = 0;

  for (;;)
    {
      if (first)
	first = FALSE;
      else
	word = feof(fp) ? "End" : fread_word(fp);
      fMatch = FALSE;

      switch (UPPER (word[0]))
	{
	case '*':
	  fMatch = TRUE;
	  fread_to_eol (fp);
	  break;

	case 'A':
	  if (!str_cmp (word, "AffD"))
	    {
	      AFFECT_DATA *paf;
	      int sn;

	      paf = new_affect ();

	      sn = exact_skill_lookup (fread_word (fp));
	      if (sn < 0)
		bug ("Fread_sword: unknown skill.", 0);
	      else
		paf->type = sn;

	      paf->level = fread_number (fp);
	      paf->duration = fread_number (fp);
	      paf->modifier = fread_number (fp);
	      paf->location = fread_number (fp);
	      paf->bitvector = fread_number (fp);
	      paf->next = obj->affected;
	      obj->affected = paf;
	      fMatch = TRUE;
	      break;
	    }
	  if (!str_cmp (word, "Affc"))
	    {
	      AFFECT_DATA *paf;
	      int sn;

	      paf = new_affect ();

	      sn = exact_skill_lookup (fread_word (fp));
	      if (sn < 0)
		bug ("Fread_sword: unknown skill.", 0);
	      else
		paf->type = sn;

	      paf->where = fread_number (fp);
	      paf->level = fread_number (fp);
	      paf->duration = fread_number (fp);
	      paf->modifier = fread_number (fp);
	      paf->location = fread_number (fp);
	      //Iblis - 10/03/04 This will make it so it takes a skill % affect by the skill NAME and not
              //the number (in case skills are rearranged)
//              if (paf->where == TO_SKILL && paf->location != 0)
//                paf->location = paf->type;
	      paf->bitvector = fread_number (fp);
	      paf->composition = fread_number (fp);
	      paf->comp_name = str_dup (fread_word (fp));

	      paf->next = obj->affected;
	      obj->affected = paf;
	      fMatch = TRUE;
	      break;
	    }
	  break;
	case 'B':
	  if (!str_cmp (word, "BSCap"))
	    {
	      for (i = 0; i < MAX_BLADE_SPELLS-1; i++)
		{
		  obj->bs_capacity[i] = fread_number (fp);
		}
	      obj->bs_capacity[6]=0;
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "BS2Cap"))
            {
	      for (i = 0; i < MAX_BLADE_SPELLS; i++)
	        {
	          obj->bs_capacity[i] = fread_number (fp);
	        }
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "BSChar"))
	    {
	      for (i = 0; i < MAX_BLADE_SPELLS-1; i++)
		{
		  obj->bs_charges[i] = fread_number (fp);
		}
	      obj->bs_charges[6] = 0;
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "BS2Char"))
	  {
	    for (i = 0; i < MAX_BLADE_SPELLS; i++)
	    {
	      obj->bs_charges[i] = fread_number (fp);
	    }
	    fMatch = TRUE;
	    break;
	  }

	  if (!str_cmp (word, "BBCap"))
	    {
	      for (i = 0; i < MAX_BURST; i++)
		{
		  obj->bb_capacity[i] = fread_number (fp);
		}
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "BBChar"))
	    {
	      for (i = 0; i < MAX_BURST; i++)
		{
		  obj->bb_charges[i] = fread_number (fp);
		}
	      fMatch = TRUE;
	      break;
	    }
	  break;

	case 'C':
	  // Akamai 4/30/99 - support Class/race specific objects
	  KEY ("Cardslot", obj->bs_capacity[0], fread_number (fp));
	  KEY ("ClanFlags", obj->clan_flags, fread_flag (fp));
	  KEY ("ClanF", obj->clan_flags, fread_flag (fp));
	  KEY ("ClassFlags", obj->Class_flags, fread_flag (fp));
	  KEY ("ClassF", obj->Class_flags, fread_flag (fp));
	  KEY ("Cond", obj->condition, fread_number (fp));
	  KEY ("Cost", obj->cost, fread_number (fp));
	  break;

	case 'D':
	  KEY ("Description", obj->description, fread_string (fp));
	  KEY ("Desc", obj->description, fread_string (fp));
	  break;

	case 'E':

	  if (!str_cmp (word, "Enchanted"))
	    {
	      obj->enchanted = TRUE;
	      fMatch = TRUE;
	      break;
	    }

	  KEY ("ExtraFlags", obj->extra_flags[0], fread_number (fp));
	  KEY ("ExtF", obj->extra_flags[0], fread_number (fp));

	  if (!str_cmp(word,"ExF"))
	  {
		  int ai;
		  ai = fread_number(fp);
		  if (ai > MAX_EXTRA_FLAGS)
		  {
			  bug("Extra Flag Index out of range!",0);
			  break;
		  }
		  obj->extra_flags[ai] = fread_number (fp);
		  fMatch = TRUE;
	  }
	  
	  if (!str_cmp (word, "ExtraDescr") || !str_cmp (word, "ExDe"))
	    {
	      EXTRA_DESCR_DATA *ed, *cur;

	      ed = new_extra_descr ();

	      ed->keyword = fread_string (fp);
	      ed->description = fread_string (fp);
	      //Shinowlan 5/18/98 -- Loop to eliminate duplicate ExDe's
	      cur = obj->extra_descr;
	      while ((cur != NULL) && str_cmp (cur->keyword, ed->keyword))
		{
		  cur = cur->next;
		}
	      //Shinowlan 5/18/98 -- only add keyword if it's not a dupe 
	      if (cur == NULL)
		{
		  ed->next = obj->extra_descr;
		  obj->extra_descr = ed;
		}
	      else
		{
		  free_extra_descr (ed);
		}
	      fMatch = TRUE;
	    }

	  if (!str_cmp (word, "End"))
	    {
	      //  fNest = TRUE;
	      // if (!fNest || !fVnum || obj->pIndexData == NULL)
	      //       {
	      //         bug ("Fread_obj: incomplete object.", 0);
	      //         free_obj (obj);
	      //         return;
	      //        }
	      ch->sword = obj;
	      if (obj->condition == 0)
		obj->condition = 100;
	      log_string ("Done reading Sword Data");
	      return;
	    }
	  break;

	case 'I':
	  KEY ("ItemType", obj->item_type, fread_number (fp));
	  KEY ("Ityp", obj->item_type, fread_number (fp));
	  break;

	case 'L':
	  KEY ("Level", obj->level, fread_number (fp));
	  KEY ("Lev", obj->level, fread_number (fp));
	  break;

	case 'N':
	  KEY ("Name", obj->name, fread_string (fp));

	  if (!str_cmp (word, "Nest"))
	    {
	      iNest = fread_number (fp);
	      if (iNest < 0 || iNest >= MAX_NEST)
		{
		  bug ("Fread_sword: bad nest %d.", iNest);
		}
	      else
		{
		  rgObjNest[iNest] = obj;
		  fNest = TRUE;
		}
	      fMatch = TRUE;
	    }
	  break;

	case 'O':
	  KEY ("OwnR", obj->owner, fread_string (fp));

	  if (!str_cmp (word, "Oldstyle"))
	    {
	      if (obj->pIndexData != NULL)
		make_new = TRUE;
	      fMatch = TRUE;
	    }
	  break;


	case 'R':
	  // Akamai 4/30/99 - support for Class/race specific equipment
	  KEY ("RaceFlags", obj->race_flags, fread_flag (fp));
	  KEY ("RaceF", obj->race_flags, fread_flag (fp));
	  break;


	case 'S':
	  KEY ("ShortDescr", obj->short_descr, fread_string (fp));
	  KEY ("ShD", obj->short_descr, fread_string (fp));

	  if (!str_cmp (word, "Spell"))
	    {
	      int iValue;
	      int sn;

	      iValue = fread_number (fp);
	      sn = exact_skill_lookup (fread_word (fp));
	      if (iValue < 0 || iValue > 5)
		{
		  bug ("Fread_sword: bad iValue %d.", iValue);
		}
	      else if (sn < 0)
		{
		  bug ("Fread_sword: unknown skill.", 0);
		}
	      else
		{
		  obj->value[iValue] = sn;
		}
	      fMatch = TRUE;
	      break;
	    }

	  break;

	case 'T':
	  KEY ("Timer", obj->timer, fread_number (fp));
	  KEY ("Time", obj->timer, fread_number (fp));
	  break;

	case 'V':
	  if (!str_cmp (word, "Values") || !str_cmp (word, "Vals"))
	    {
	      obj->value[0] = fread_number (fp);
	      obj->value[1] = fread_number (fp);
	      obj->value[2] = fread_number (fp);
	      obj->value[3] = fread_number (fp);
	      if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
		obj->value[0] = obj->pIndexData->value[0];
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "Val"))
	    {
	      obj->value[0] = fread_number (fp);
	      obj->value[1] = fread_number (fp);
	      obj->value[2] = fread_number (fp);
	      obj->value[3] = fread_number (fp);
	      obj->value[4] = fread_number (fp);
	      obj->value[5] = fread_number (fp);
	      obj->value[6] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "Vls"))
	  {
		  obj->value[0] = fread_number (fp);
		  obj->value[1] = fread_number (fp);
		  obj->value[2] = fread_number (fp);
		  obj->value[3] = fread_number (fp);
		  obj->value[4] = fread_number (fp);
		  obj->value[5] = fread_number (fp);
		  obj->value[6] = fread_number (fp);
		  obj->value[7] = fread_number (fp);
		  obj->value[8] = fread_number (fp);
		  obj->value[9] = fread_number (fp);
		  obj->value[10] = fread_number (fp);
//		  obj->value[11] = fread_number (fp);
//		  obj->value[12] = fread_number (fp);
		  fMatch = TRUE;
		  break;
	  }

	  if (!str_cmp (word, "Vlues"))
	    {
	      obj->value[0] = fread_number (fp);
	      obj->value[1] = fread_number (fp);
	      obj->value[2] = fread_number (fp);
	      obj->value[3] = fread_number (fp);
	      obj->value[4] = fread_number (fp);
	      obj->value[5] = fread_number (fp);
	      obj->value[6] = fread_number (fp);
	      obj->value[7] = fread_number (fp);
	      obj->value[8] = fread_number (fp);
	      obj->value[9] = fread_number (fp);
	      obj->value[10] = fread_number (fp);
	      obj->value[11] = fread_number (fp);
	      obj->value[12] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }


	  if (!str_cmp (word, "Vnum"))
	    {
	      int vnum;

	      vnum = fread_number (fp);
	      if ((obj->pIndexData = get_obj_index (vnum)) == NULL)
		bug ("Fread_sword: bad vnum %d.", vnum);
	      else
		fVnum = TRUE;
	      fMatch = TRUE;
	      break;
	    }
	  break;

	case 'W':
	  KEY ("WearFlags", obj->wear_flags, fread_number (fp));
	  KEY ("WeaF", obj->wear_flags, fread_number (fp));
	  KEY ("WearLoc", obj->wear_loc, fread_number (fp));
          KEY ("WearL", obj->wear_loc, fread_number (fp));
//	  KEY ("Wear", obj->wear_loc, fread_number (fp));
	  KEY ("Weight", obj->weight, fread_number (fp));
	  KEY ("Wt", obj->weight, fread_number (fp));
	  if (!str_cmp (word, "Wear"))
	  {
	    int i=0;
	    int wearloc = fread_number(fp);
	    if (wearloc == -1)
               obj->wear_loc = -1;
	    else 
	    {
	      for (i=0;wear_conversion_table[i].wear_loc != -2;++i)
	      {
 	        if (wear_conversion_table[i].wear_loc == wearloc)
	        {
	 	      obj->wear_loc = wear_conversion_table[i].wear_flag;
		      break;
	        }
	      }
	    }

	    fMatch = TRUE;
  	    break;
	  }
	  break;

	}
      if (!fMatch)
	{
	  bug ("Fread_sword: no match.", 0);
	  fread_to_eol (fp);
	}
    }
}




/* Called from load_char_obj to read in the saved RLE data */
void fread_rle (char *explored, FILE * fp)
{
  int index = 0;
  int bit = 0, count = 0, pos = 0;

  /* Cycle through the unknown number of pairs */
  for (;;)
    {
      /* Get the run length */
      count = fread_number (fp);

      /* Is it the terminating -1? */
      if (count < 0)
	break;

      /* The array initializes to zero, so only set on a one */
      if (bit == 1)
	{
	  /* Fill in all the bytes in the run */
	  for (index = pos; index < pos + count; index++)
	    {
	      explored[index / 8] |= 1 << (index % 8);
	    }
	}

      /* This run is over, so toggle the value */
      bit = (bit) ? 0 : 1;

      /* Advance to the start of the next run */
      pos += count;
    }
}


void save_char_obj (CHAR_DATA * ch)
{
  char tcbuf[MAX_INPUT_LENGTH];
  char strsave[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH * 10];
  CHAR_DATA *mount, *mount_next;
  FILE *fp;



  if ((ch->in_room && (ch->in_room->vnum >= ROOM_VNUM_HAVEN_START &&
       ch->in_room->vnum <= ROOM_VNUM_HAVEN_END)) || is_dueling (ch) ||
      IS_NPC (ch))
    return;
  if (ch->level == 1 && ((ch->played + (int) (current_time - ch->logon)) / 3600) < 2)
    return;

  if (ch->pcdata->security == 42)
    return;

  if (ch->desc != NULL && ch->desc->original != NULL)
    ch = ch->desc->original;

  is_encrypted = FALSE;

  if (IS_IMMORTAL (ch) || ch->level >= LEVEL_IMMORTAL)
    {
      fclose (fpReserve);
      sprintf (strsave, "%s%s", GOD_DIR, capitalize (ch->name));
      if ((fp = fopen (strsave, "w")) == NULL)
	{
	  bug ("Save_char_obj: fopen", 0);
	  perror (strsave);
	}

      sprintf (buf, "Lev %2d Trust %2d  %s%s\n",
	       ch->level, get_trust (ch), ch->name, ch->pcdata->title);
      encrypt_write (buf, fp);

      fclose (fp);
      fpReserve = fopen (NULL_FILE, "r");
    }


  fclose (fpReserve);
  sprintf (tcbuf, "%s/%c", PLAYER_DIR, toupper (ch->name[0]));
  mkdir (tcbuf, 0755);

  if (make_backup)
    sprintf (strsave, "%s%c/%s%s", PLAYER_DIR, toupper (ch->name[0]),
	     capitalize (ch->name), PFILE_BACKUP_DEATH);
  else if (make_backup_duel)
    sprintf (strsave, "%s%c/%s%s", PLAYER_DIR, toupper (ch->name[0]),
	     capitalize (ch->name), PFILE_BACKUP_DUEL);
  else
    sprintf (strsave, "%s%c/%s", PLAYER_DIR, toupper (ch->name[0]),
	     capitalize (ch->name));

  if ((fp = fopen (TEMP_FILE, "w")) == NULL)
    {
      bug ("Save_char_obj: fopen", 0);
      perror (strsave);
    }
  else
    {
      fwrite_char (ch, fp);
      if (ch->carrying != NULL)
	fwrite_obj (ch, ch->carrying, fp, 0);
      /* save the pets */
      if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
	fwrite_pet (ch->pet, fp);

      for (mount = ch->pcdata->boarded; mount != NULL; mount = mount_next)
	{
	  mount_next = mount->next_in_board;
	  fwrite_mount (mount, fp);
	}
      fwrite_rle (ch->pcdata->explored, fp,"RoomRLE");

      fwrite_rle (ch->pcdata->qbits, fp, "QbitsRLE");

      if (ch->sword != NULL)	/* If reaver, save sword data */
	fwrite_sword (ch->sword, fp);

      sprintf (buf, "#END\n");
      encrypt_write (buf, fp);
    }

  fclose (fp);
  rename (TEMP_FILE, strsave);
  fpReserve = fopen (NULL_FILE, "r");
  if (make_backup)
    make_backup = FALSE;
  if (make_backup_duel)
    make_backup_duel = FALSE;



  return;
}



/*
 * Write the char.
 */
void fwrite_char (CHAR_DATA * ch, FILE * fp)
{
  AFFECT_DATA *paf;
  COMP_DATA *comp;
  int sn, pos;
  char buf[MAX_STRING_LENGTH * 10];
//  int i;

  if (IS_NPC (ch))
    {
      bug ("fwrite_char : Attempted to write NPC", 0);
      return;
    }

  if (ch->pcdata->lastlogoff <= 0)
    ch->pcdata->lastlogoff = current_time;

  sprintf (buf, "#%s\n", IS_NPC (ch) ? "MOB" : "PLAYER");
  encrypt_write (buf, fp);

  sprintf (buf, "Name %s~\n", ch->name);
  encrypt_write (buf, fp);

  sprintf (buf, "Id   %ld\n", ch->id);
  encrypt_write (buf, fp);

  sprintf (buf, "LogF %ld\n", ch->pcdata->lastlogoff);
  encrypt_write (buf, fp);

  sprintf (buf, "LogO %ld\n", current_time);
  encrypt_write (buf, fp);

  sprintf (buf, "Vers %d\n", PFILE_VER_CURRENT);
  encrypt_write (buf, fp);

  if (ch->short_descr[0] != '\0')
    {
      sprintf (buf, "ShD  %s~\n", ch->short_descr);
      encrypt_write (buf, fp);
    }

  if (ch->long_descr[0] != '\0')
    {
      sprintf (buf, "LnD  %s~\n", ch->long_descr);
      encrypt_write (buf, fp);
    }

  if (ch->description[0] != '\0')
    {
      sprintf (buf, "Desc %s~\n", ch->description);
      encrypt_write (buf, fp);
    }

  if (ch->prompt != NULL || !str_cmp (ch->prompt, "<%hhp %mma %vmv> "))
    {
      sprintf (buf, "Prom %s~\n", ch->prompt);
      encrypt_write (buf, fp);
    }

  sprintf (buf, "Race %s~\n", pc_race_table[ch->race].name);
  encrypt_write (buf, fp);

  if (ch->clan != CLAN_BOGUS)
    {
      sprintf (buf, "Clan %s~\n", get_clan_name_ch (ch));
      encrypt_write (buf, fp);
    }

  if (ch->desc != NULL)
    if (str_cmp (ch->desc->realhost, ch->desc->host))
      {
	sprintf (buf, "Fhost  %s~\n", ch->desc->host);
	encrypt_write (buf, fp);
      }

  sprintf (buf, "Sex  %d\n", ch->sex);
  encrypt_write (buf, fp);

  sprintf (buf, "Cla  %d\n", ch->Class);
  encrypt_write (buf, fp);

  sprintf (buf, "Cla2  %d\n", ch->Class2);
  encrypt_write (buf, fp);

  sprintf (buf, "Cla3  %d\n", ch->Class3);
  encrypt_write (buf, fp);

  sprintf (buf, "Levl %d\n", ch->level);
  encrypt_write (buf, fp);

  if (ch->times_wanted != 0)
    {
      sprintf (buf, "Wants %d\n", ch->times_wanted);
      encrypt_write (buf, fp);
    }


  if (ch->want_jail_time != 0)
    {
      sprintf (buf, "WTime %d\n", ch->want_jail_time);
      encrypt_write (buf, fp);
    }

//  if (ch->loot_time != 0)
  //  {
    //  sprintf (buf, "LTime %d\n", ch->loot_time);
   //   encrypt_write (buf, fp);
   // }

  if (ch->trust != 0)
    {
      sprintf (buf, "Tru  %d\n", ch->trust);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->autoassist_level != 0)
    {
      sprintf (buf, "AALevl %d\n", ch->pcdata->autoassist_level);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->lost_prayers == TRUE)
    {
      sprintf (buf, "LPray %d\n", ch->pcdata->lost_prayers);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->events_won[BR_WON] != 0)
    {
      sprintf (buf, "EWonBR %d\n", ch->pcdata->events_won[BR_WON]);
      encrypt_write (buf, fp);
    }
  if (ch->pcdata->events_won[BR_IN] != 0)
    {
      sprintf (buf, "EInBR %d\n", ch->pcdata->events_won[BR_IN]);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->events_won[SD_WON] != 0)
    {
      sprintf (buf, "EWonSD %d\n", ch->pcdata->events_won[SD_WON]);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->events_won[SD_IN] != 0)
    {
      sprintf (buf, "EInSD %d\n", ch->pcdata->events_won[SD_IN]);
      encrypt_write (buf, fp);
    }
  if (ch->pcdata->events_won[TEAM_SD_IN] != 0)
    {
      sprintf (buf, "EInTSD %d\n", ch->pcdata->events_won[TEAM_SD_IN]);
      encrypt_write (buf, fp);
    }
  if (ch->pcdata->events_won[TEAM_SD_WON] != 0)
    {
      sprintf (buf, "EWonTSD %d\n", ch->pcdata->events_won[TEAM_SD_WON]);
      encrypt_write (buf, fp);
    }
  
  if (ch->pcdata->other_deaths != 0)
  {
    sprintf (buf, "ODeaths %d\n", ch->pcdata->other_deaths);
    encrypt_write (buf, fp);
  }
  
  
  if (ch->pcdata->been_killed[PLAYER_KILL] != 0)
    {
      sprintf (buf, "PBKill %d\n", ch->pcdata->been_killed[PLAYER_KILL]);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->been_killed[MOB_KILL] != 0)
    {
      sprintf (buf, "MBKill %d\n", ch->pcdata->been_killed[MOB_KILL]);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->been_killed[BR_KILL] != 0)
    {
      sprintf (buf, "BBKill %d\n", ch->pcdata->been_killed[BR_KILL]);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->been_killed[SD_KILL] != 0)
    {
      sprintf (buf, "SBKill %d\n", ch->pcdata->been_killed[SD_KILL]);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->been_killed[ARENA_KILL] != 0)
    {
      sprintf (buf, "ABKill %d\n", ch->pcdata->been_killed[ARENA_KILL]);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->has_killed[PLAYER_KILL] != 0)
    {
      sprintf (buf, "PHKill %d\n", ch->pcdata->has_killed[PLAYER_KILL]);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->has_killed[MOB_KILL] != 0)
    {
      sprintf (buf, "MHKill %d\n", ch->pcdata->has_killed[MOB_KILL]);
      encrypt_write (buf, fp);
    }
  
  if (ch->pcdata->has_killed[BR_KILL] != 0)
    {
      sprintf (buf, "BHKill %d\n", ch->pcdata->has_killed[BR_KILL]);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->has_killed[SD_KILL] != 0)
    {
      sprintf (buf, "SHKill %d\n", ch->pcdata->has_killed[SD_KILL]);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->has_killed[ARENA_KILL] != 0)
    {
      sprintf (buf, "AHKill %d\n", ch->pcdata->has_killed[ARENA_KILL]);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->shutup_ticks >= 1)
    {
      sprintf (buf, "SUTicks %ld\n", ch->pcdata->shutup_ticks);
      encrypt_write (buf, fp);
    }
  if (ch->pcdata->nochan_ticks >= 1)
    {
      sprintf (buf, "NCTicks %ld\n", ch->pcdata->nochan_ticks);
      encrypt_write (buf, fp);
    }

  if (ch->race == PC_RACE_NIDAE && ch->pcdata->oow_ticks >= 0)
    {
      sprintf (buf, "OOWTicks %d\n", ch->pcdata->oow_ticks);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->personality != PERS_NORMAL)
    {
      sprintf (buf, "Pers %d\n", ch->pcdata->personality);
      encrypt_write (buf, fp);
    }
  if (ch->pcdata->all_xp_familiar)
    {
      sprintf (buf, "AEP %d\n", ch->pcdata->all_xp_familiar);
      encrypt_write (buf, fp);
    }
  

  if (ch->pcdata->familiar_type != 0)
    {
      sprintf (buf, "FamT %d\n", ch->pcdata->familiar_type);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->familiar_level != 1)
    {
      sprintf (buf, "FamL %d\n", ch->pcdata->familiar_level);
      encrypt_write (buf, fp);
    }
  
  if (ch->pcdata->familiar_max_hit != 20)
    {
      sprintf (buf, "FamMH %d\n", ch->pcdata->familiar_max_hit);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->familiar_max_mana != 100)
    {
      sprintf (buf, "FamMM %d\n", ch->pcdata->familiar_max_mana);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->familiar_max_move != 100)
    {
      sprintf (buf, "FamMV %d\n", ch->pcdata->familiar_max_move);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->familiar_gained_move != 0)
    {
      sprintf (buf, "FamGMV %d\n", ch->pcdata->familiar_gained_move);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->familiar_gained_hp != 0)
    {
      sprintf (buf, "FamGH %d\n", ch->pcdata->familiar_gained_hp);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->familiar_gained_mana != 0)
    {
      sprintf (buf, "FamGM %d\n", ch->pcdata->familiar_gained_mana);
      encrypt_write (buf, fp);
    }
    
  

  if (ch->pcdata->familiar_exp >= 1)
    {
      sprintf (buf, "FamE %ld\n", ch->pcdata->familiar_exp);
      encrypt_write (buf, fp);
    }

  if (ch->pcdata->familiar_name[0] != '\0')
    {
      sprintf (buf, "FamN %s~\n", ch->pcdata->familiar_name);
      encrypt_write (buf, fp);
    }
  

  sprintf (buf, "Hero %d\n", ch->pcdata->hero);
  encrypt_write (buf, fp);

  sprintf (buf, "Depty %d\n", ch->pcdata->deputy);
  encrypt_write (buf, fp);

  sprintf (buf, "Sec  %d\n", ch->pcdata->security);	/* OLC */
  encrypt_write (buf, fp);

  sprintf (buf, "BldAll %d\n", ch->pcdata->buildall);
  encrypt_write (buf, fp);

  sprintf (buf, "Plyd %d\n", ch->played + (int) (current_time - ch->logon));
  encrypt_write (buf, fp);

  sprintf (buf, "NPlyd %d\n", ch->pcdata->nplayed);
  encrypt_write (buf, fp);
  
  sprintf (buf, "NAge %d\n", ch->pcdata->nage);
  encrypt_write (buf, fp);

  sprintf (buf, "Bank_Gold %ld\n", ch->pcdata->bank_gold);
  encrypt_write (buf, fp);

  sprintf (buf, "Bank_Silv %ld\n", ch->pcdata->bank_silver);
  encrypt_write (buf, fp);

  sprintf (buf, "Email %s~\n", ch->pcdata->email_addr);
  encrypt_write (buf, fp);

  sprintf (buf, "Referrer %s~\n", ch->pcdata->referrer);
  encrypt_write (buf, fp);

  sprintf (buf, "Souls %ld\n", ch->pcdata->souls);
  encrypt_write (buf, fp);

  sprintf (buf, "HP_Gained %d\n", ch->pcdata->hp_gained);
  encrypt_write (buf, fp);

  sprintf (buf, "Mana_Gained %d\n", ch->pcdata->mana_gained);
  encrypt_write (buf, fp);

  sprintf (buf, "Move_Gained %d\n", ch->pcdata->move_gained);
  encrypt_write (buf, fp);

  sprintf (buf, "PointsRp %ld\n", ch->pcdata->rppoints);
  encrypt_write (buf, fp);

  sprintf (buf, "PointsNh %ld\n", ch->pcdata->nhpoints);
  encrypt_write (buf, fp);

  sprintf (buf, "PointsQ %ld\n", ch->pcdata->qpoints);
  encrypt_write (buf, fp);

  sprintf (buf, "Avatar_Type %d\n", ch->pcdata->avatar_type);
  encrypt_write (buf, fp);

  sprintf (buf, "Race_Old %d\n", ch->pcdata->old_race);
  encrypt_write (buf, fp);
  
  if (ch->pcdata->has_reincarnated)
  {
    sprintf (buf, "HasR %d\n", ch->pcdata->has_reincarnated);
    encrypt_write (buf, fp);
  }

  sprintf (buf, "Class_Old %d\n", ch->pcdata->old_Class);
  encrypt_write (buf, fp);
  
  sprintf (buf, "New_Style %d\n", ch->pcdata->new_style);
  encrypt_write (buf, fp);

  sprintf (buf, "Totalxp %lu\n", ch->pcdata->totalxp);
  encrypt_write (buf, fp);

  sprintf (buf, "TotalS %lu\n", ch->pcdata->totalsouls);
  encrypt_write (buf, fp);

  sprintf (buf, "Dam_Type %d\n", ch->dam_type);
  encrypt_write (buf, fp);

  sprintf (buf, "Prevent_Escape %d\n", ch->pcdata->prevent_escape);
  encrypt_write (buf, fp);

  sprintf (buf, "Falcon_Alert %d\n", ch->pcdata->falcon_alert);
  encrypt_write (buf, fp);

  sprintf (buf, "NoBlinking %d\n", ch->pcdata->noblinking);
  encrypt_write (buf, fp);

  sprintf (buf, "GamL %lu\n", ch->pcdata->tg_lost);
  encrypt_write (buf, fp);

  sprintf (buf, "GamW %lu\n", ch->pcdata->tg_won);
  encrypt_write (buf, fp);

  sprintf (buf, "QTicks %d\n", ch->pcdata->quest_ticks);
  encrypt_write (buf, fp);    
  sprintf (buf, "TotalQH %ld\n", ch->pcdata->total_quest_hunts);
  encrypt_write (buf, fp);
  sprintf (buf, "TotalQF %ld\n", ch->pcdata->total_quest_finds);
  encrypt_write (buf, fp);
  sprintf (buf, "TQFAttempt %ld\n", ch->pcdata->total_qf_attempted);
  encrypt_write (buf, fp);
  sprintf (buf, "TQHAttempt %ld\n", ch->pcdata->total_qh_attempted);
  encrypt_write (buf, fp);
  sprintf (buf, "TotalCGL %ld\n", ch->pcdata->total_cg_lost);
  encrypt_write (buf, fp);
  sprintf (buf, "TotalCGW %ld\n", ch->pcdata->total_cg_won);
  encrypt_write (buf, fp);
  if (ch->pcdata->total_trees_chopped > 0)
  {
    sprintf (buf, "TCT %ld\n", ch->pcdata->total_trees_chopped);
    encrypt_write (buf, fp);  
  }
  if (ch->pcdata->total_trees_milled > 0)
  {
    sprintf (buf, "TMT %ld\n", ch->pcdata->total_trees_milled);
    encrypt_write (buf, fp);
  }
  if (ch->pcdata->total_money_made > 0)
  {
    sprintf (buf, "TMM %ld\n", ch->pcdata->total_money_made);
    encrypt_write (buf, fp);
  }
  if (ch->pcdata->total_objs_sold > 0)
  {
    sprintf (buf, "TOS %ld\n", ch->pcdata->total_objs_sold);
    encrypt_write (buf, fp);
  }
    
  sprintf (buf, "MPC %d\n", ch->pcdata->pushed_mobs_counter);
  encrypt_write (buf, fp);
  
    
      

  if (ch->desc != NULL)
    sprintf (buf, "Host %s~\n", ch->desc->host);
  else
    sprintf (buf, "Host PURGED-LD~\n");
  encrypt_write (buf, fp);

  sprintf (buf, "Not  %ld %ld %ld %ld %ld %ld %ld %ld\n",
	   ch->pcdata->last_note, ch->pcdata->last_idea,
	   ch->pcdata->last_penalty, ch->pcdata->last_news,
	   ch->pcdata->last_changes, ch->pcdata->last_oocnote,
	   ch->pcdata->last_legend, ch->pcdata->last_projects);
  encrypt_write (buf, fp);

  sprintf (buf, "Scro %d\n", ch->lines);
  encrypt_write (buf, fp);

  if (make_backup)
    {
      sprintf (buf, "Room 4056\n");
      encrypt_write (buf, fp);
    }
  else
    {
      if (!nowhere)
	{
	  ch->pcdata->last_saved_room =
	    ((ch->in_room == get_room_index (ROOM_VNUM_LIMBO)
	      && ch->was_in_room !=
	      NULL) ? ch->was_in_room->vnum : ch->in_room ==
	     NULL ? 4056 : ch->in_room->vnum);
	  sprintf (buf, "Room %d\n", ch->pcdata->last_saved_room);

	  encrypt_write (buf, fp);
	}
      else
	{
	  sprintf (buf, "Room %d\n", ch->pcdata->last_saved_room);
	  encrypt_write (buf, fp);
	}

    }
  sprintf (buf, "HMV  %d %d %d %d %d %d\n",
	   ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
	   ch->max_move);
  encrypt_write (buf, fp);

  if (ch->pcdata->primary_hand < HAND_LEFT)
    ch->pcdata->primary_hand = HAND_RIGHT;

  sprintf (buf, "Hand %d\n", ch->pcdata->primary_hand);
  encrypt_write (buf, fp);

  if (ch->gold > 0)
    sprintf (buf, "Gold %ld\n", ch->gold);
  else
    sprintf (buf, "Gold %d\n", 0);
  encrypt_write (buf, fp);

  if (ch->silver > 0)
    sprintf (buf, "Silv %ld\n", ch->silver);
  else
    sprintf (buf, "Silv %d\n", 0);
  encrypt_write (buf, fp);

  sprintf (buf, "Exp  %ld\n", ch->exp);
  encrypt_write (buf, fp);

  if (ch->act != 0)
    {
      sprintf (buf, "Act  %s\n", print_flags (ch->act));
      encrypt_write (buf, fp);
    }

  if (ch->act2 != 0)
    {
      sprintf (buf, "Act2  %s\n", print_flags (ch->act2));
      encrypt_write (buf, fp);
    }
  // Adeon 7/17/03 -- Exit blocked
  sprintf (buf, "Blok %d\n", ch->blocks_exit);
  encrypt_write (buf, fp);




  if (ch->affected_by != 0)
    {
      sprintf (buf, "AfBy %s\n", print_flags (ch->affected_by));
      encrypt_write (buf, fp);
    }

  sprintf (buf, "Comm %s\n", print_flags (ch->comm));
  encrypt_write (buf, fp);

  sprintf (buf, "Com2 %s\n", print_flags (ch->comm2));
  encrypt_write (buf, fp);

  if (ch->wiznet)
    {
      sprintf (buf, "Wizn %s\n", print_flags (ch->wiznet));
      encrypt_write (buf, fp);
    }

  if (ch->invis_level)
    {
      sprintf (buf, "Invi %d\n", ch->invis_level);
      encrypt_write (buf, fp);
    }

  if (ch->incog_level)
    {
      sprintf (buf, "Inco %d\n", ch->incog_level);
      encrypt_write (buf, fp);
    }

  if (ch->position == POS_MOUNTED)
    ch->position = POS_STANDING;

  sprintf (buf, "Pos  %d\n",
	   ch->position == POS_FIGHTING ? POS_STANDING : ch->position);
  encrypt_write (buf, fp);


  /*if (ch->practice != 0)
     {
     sprintf (buf, "Prac %d\n", ch->practice);
     encrypt_write (buf, fp);
     } */


  if (ch->train != 0)
    {
      sprintf (buf, "Trai %d\n", ch->train);
      encrypt_write (buf, fp);
    }


  if (ch->saving_throw != 0)
    {
      sprintf (buf, "Save  %d\n", ch->saving_throw);
      encrypt_write (buf, fp);
    }

  sprintf (buf, "Alig  %d\n", ch->alignment);
  encrypt_write (buf, fp);

  if (ch->hitroll != 0)
    {
      sprintf (buf, "Hit   %d\n", ch->hitroll);
      encrypt_write (buf, fp);
    }

  if (ch->damroll != 0)
    {
      sprintf (buf, "Dam   %d\n", ch->damroll);
      encrypt_write (buf, fp);
    }

  sprintf (buf, "ACs %d %d %d %d\n",
	   ch->armor[0], ch->armor[1], ch->armor[2], ch->armor[3]);
  encrypt_write (buf, fp);


  if (ch->wimpy != 0)
    {
      sprintf (buf, "Wimp  %d\n", ch->wimpy);
      encrypt_write (buf, fp);
    }
  sprintf (buf, "Attr %d %d %d %d %d %d\n",
	   ch->perm_stat[STAT_STR],
	   ch->perm_stat[STAT_INT],
	   ch->perm_stat[STAT_WIS],
	   ch->perm_stat[STAT_DEX],
	   ch->perm_stat[STAT_CON], ch->perm_stat[STAT_CHA]);
  encrypt_write (buf, fp);

  sprintf (buf, "AMod %d %d %d %d %d %d\n",
	   ch->mod_stat[STAT_STR],
	   ch->mod_stat[STAT_INT],
	   ch->mod_stat[STAT_WIS],
	   ch->mod_stat[STAT_DEX],
	   ch->mod_stat[STAT_CON], ch->mod_stat[STAT_CHA]);
  encrypt_write (buf, fp);

  if (IS_NPC (ch))
    {
      sprintf (buf, "Vnum %d\n", ch->pIndexData->vnum);
      encrypt_write (buf, fp);
    }
  else
    {
      sprintf (buf, "Pass %s~\n", ch->pcdata->pwd);
      encrypt_write (buf, fp);
      if (ch->mask != NULL && ch->mask[0] != '\0')
	{
	  sprintf (buf, "Mask %s~\n", ch->mask);
	  encrypt_write (buf, fp);
	}
      if (ch->pcdata->bamfin[0] != '\0')
	{
	  sprintf (buf, "Bin  %s~\n", ch->pcdata->bamfin);
	  encrypt_write (buf, fp);
	}
      if (ch->pcdata->bamfout[0] != '\0')
	{
	  sprintf (buf, "Bout %s~\n", ch->pcdata->bamfout);
	  encrypt_write (buf, fp);
	}

      if (ch->pcdata->whoinfo[0] != '\0')
	{
	  sprintf (buf, "Win %s~\n", ch->pcdata->whoinfo);
	  encrypt_write (buf, fp);
	}

      if (ch->pcdata->pretitle[0] != '\0')
	{
	  sprintf (buf, "Pret %s~\n", ch->pcdata->pretitle);
	  encrypt_write (buf, fp);
	}

      if (ch->pcdata->battlecry[0] != '\0')
	{

	  sprintf (buf, "Bcry  %s~\n", ch->pcdata->battlecry);
	  encrypt_write (buf, fp);
	}



      if (ch->pcdata->plan[0] != '\0')
	{
	  sprintf (buf, "Plan  %s~\n", ch->pcdata->plan);
	  encrypt_write (buf, fp);
	}

      sprintf (buf, "Layhands %d\n", ch->lh_ticks);
      encrypt_write (buf, fp);

      sprintf (buf, "Exorcise %d\n", ch->ex_ticks);
      encrypt_write (buf, fp);

      sprintf (buf, "Illusion %d\n", ch->il_ticks);
      encrypt_write (buf, fp);

      sprintf (buf, "Titl %s~\n", ch->pcdata->title);
      encrypt_write (buf, fp);

      if (IS_IMMORTAL (ch))
	if (ch->pcdata->restoremsg[0] != '\0')
	  {
	    sprintf (buf, "RMsg %s~\n", ch->pcdata->restoremsg);
	    encrypt_write (buf, fp);
	  }

      sprintf (buf, "Pnts %d\n", ch->pcdata->points);
      encrypt_write (buf, fp);

      sprintf (buf, "TSex %d\n", ch->pcdata->true_sex);
      encrypt_write (buf, fp);

      sprintf (buf, "LLev %d\n", ch->pcdata->last_level);
      encrypt_write (buf, fp);

      sprintf (buf, "Loner %d\n", ch->pcdata->loner);
      encrypt_write (buf, fp);

      sprintf (buf, "HMVP %d %d %d\n", ch->pcdata->perm_hit,
	       ch->pcdata->perm_mana, ch->pcdata->perm_move);
      encrypt_write (buf, fp);

      sprintf (buf, "Cd  %d %d %d %d %d\n",
	       ch->pcdata->condition[0],
	       ch->pcdata->condition[1],
	       ch->pcdata->condition[2], 
	       ch->pcdata->condition[3],
	       ch->pcdata->condition[4]);
      encrypt_write (buf, fp);

      sprintf (buf,
	       "Color %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
	       ch->color,
	       ch->pcdata->color_combat_s,
	       ch->pcdata->color_combat_o,
	       ch->pcdata->color_combat_condition_s,
	       ch->pcdata->color_combat_condition_o,
	       ch->pcdata->color_invis,
	       ch->pcdata->color_hp,
	       ch->pcdata->color_hidden,
	       ch->pcdata->color_charmed,
	       ch->pcdata->color_mana,
	       ch->pcdata->color_move,
	       ch->pcdata->color_say,
	       ch->pcdata->color_tell,
	       ch->pcdata->color_guild_talk,
	       ch->pcdata->color_group_tell,
	       ch->pcdata->color_music,
	       ch->pcdata->color_auction,
	       ch->pcdata->color_gossip,
	       ch->pcdata->color_immtalk, ch->pcdata->color_admtalk);
      encrypt_write (buf, fp);

/*  for (pos = 0; pos < MAX_IGNORE; pos++)  
    {
      if (ch->pcdata->ignore[pos] == NULL)
      break;
            
      sprintf(buf, "Ignore %s~\n", ch->pcdata->ignore[pos]);
      encrypt_write(buf, fp);
     }*/
                  
      for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	  if (ch->pcdata->alias[pos] == NULL
	      || ch->pcdata->alias_sub[pos] == NULL)
	    break;

	  sprintf (buf, "Alias %s %s~\n", ch->pcdata->alias[pos],
		   ch->pcdata->alias_sub[pos]);
	  encrypt_write (buf, fp);
	}


      for (sn = 0; sn < MAX_SKILL; sn++)
	{
	  if (skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0)
	    {
	      sprintf (buf, "Sk %d '%s'\n", ch->pcdata->learned[sn],
		       skill_table[sn].name);
	      encrypt_write (buf, fp);
	    }
	}
      for (sn = 0; sn < MAX_SKILL; sn++)
        {
          if (skill_table[sn].name != NULL && ch->pcdata->mod_learned[sn] != 0)
            {
              sprintf (buf, "MSk %d '%s'\n", ch->pcdata->mod_learned[sn],
                       skill_table[sn].name);
              encrypt_write (buf, fp);
            }
        }
      

      for (comp = ch->compositions; comp; comp = comp->next)
	{
	  if (comp->name != NULL && comp->title != NULL)
	    {
	      sprintf (buf, "Cmp '%s' %d %d '%s'\n",
		       comp->name, comp->type, comp->skill, comp->title);
	      encrypt_write (buf, fp);
	    }
	}

/*      for (gn = 0; gn < MAX_GROUP; gn++)
	{
	  if (group_table[gn].name != NULL && ch->pcdata->group_known[gn])
	    {
	      sprintf (buf, "Gr '%s'\n", group_table[gn].name);
	      encrypt_write (buf, fp);
	    }
	}*/
    }

  for (paf = ch->affected; paf != NULL; paf = paf->next)
    {
      if (paf->type < 0 || paf->type >= MAX_SKILL)
	continue;

      sprintf (buf, "Affc '%s' %3d %3d %3d %3d %3d %10d %d '%s'\n",
	       skill_table[paf->type].name,
	       paf->where,
	       paf->level,
	       paf->duration,
	       paf->modifier,
	       paf->location,
	       paf->bitvector,
	       paf->composition, paf->comp_name);
      encrypt_write (buf, fp);
    }


  sprintf (buf, "End\n\n");
  encrypt_write (buf, fp);

  return;



}

/* write a pet */
void fwrite_pet (CHAR_DATA * pet, FILE * fp)
{
  AFFECT_DATA *paf;
  char buf[MAX_STRING_LENGTH * 10];

  sprintf (buf, "#PET\n");
  encrypt_write (buf, fp);

  sprintf (buf, "Vnum %d\n", pet->pIndexData->vnum);
  encrypt_write (buf, fp);

  sprintf (buf, "Name %s~\n", pet->name);
  encrypt_write (buf, fp);

  sprintf (buf, "LogO %ld\n", current_time);
  encrypt_write (buf, fp);

  if (pet->short_descr != pet->pIndexData->short_descr)
    {
      sprintf (buf, "ShD  %s~\n", pet->short_descr);
      encrypt_write (buf, fp);
    }

  if (pet->long_descr != pet->pIndexData->long_descr)
    {
      sprintf (buf, "LnD  %s~\n", pet->long_descr);
      encrypt_write (buf, fp);
    }

  if (pet->description != pet->pIndexData->description)
    {
      sprintf (buf, "Desc %s~\n", pet->description);
      encrypt_write (buf, fp);
    }

  if (pet->race != pet->pIndexData->race)
    {
      sprintf (buf, "Race %s~\n", race_table[pet->race].name);
      encrypt_write (buf, fp);
    }

  if (pet->clan != CLAN_BOGUS)
    {
      sprintf (buf, "Clan %s~\n", get_clan_name_ch (pet));
      encrypt_write (buf, fp);
    }

  sprintf (buf, "Sex  %d\n", pet->sex);
  encrypt_write (buf, fp);

  if (pet->level != pet->pIndexData->level)
    {
      sprintf (buf, "Levl %d\n", pet->level);
      encrypt_write (buf, fp);
    }
  // Akamai 6/30/98 -- Bug: #27 Pet re-rolls for hitpoints every log on
  // this little write actually had the dice rolls in it, who ever was in
  // it before me really blew it.  This save should be nearly identical to
  // the mount save. So now it is.
  sprintf (buf, "HMV %d %d %d %d %d %d\n",
	   pet->hit, pet->max_hit, pet->mana, pet->max_mana,
	   pet->move, pet->max_move);
  encrypt_write (buf, fp);


  if (pet->gold > 0)
    {
      sprintf (buf, "Gold %ld\n", pet->gold);
      encrypt_write (buf, fp);
    }

  if (pet->silver > 0)
    {
      sprintf (buf, "Silv %ld\n", pet->silver);
      encrypt_write (buf, fp);
    }

  if (pet->exp > 0)
    {
      sprintf (buf, "Exp  %ld\n", pet->exp);
      encrypt_write (buf, fp);
    }

  if (pet->act != pet->pIndexData->act)
    {
      sprintf (buf, "Act  %s\n", print_flags (pet->act));
      encrypt_write (buf, fp);
    }

  if (pet->act2 != pet->pIndexData->act2)
    {
      sprintf (buf, "Act2  %s\n", print_flags (pet->act2));
      encrypt_write (buf, fp);
    }

  sprintf (buf, "Blok %d\n", pet->blocks_exit);
  encrypt_write (buf, fp);

  if (pet->affected_by != pet->pIndexData->affected_by)
    {
      sprintf (buf, "AfBy %s\n", print_flags (pet->affected_by));
      encrypt_write (buf, fp);
    }

  if (pet->comm != 0)
    {
      sprintf (buf, "Comm %s\n", print_flags (pet->comm));
      encrypt_write (buf, fp);
    }

  if (pet->comm2 != 0)
    {
      sprintf (buf, "Com2 %s\n", print_flags (pet->comm2));
      encrypt_write (buf, fp);
    }
  

  if (pet->position == POS_MOUNTED)
    {
      pet->position = POS_STANDING;
      encrypt_write (buf, fp);
    }

  sprintf (buf, "Pos  %d\n", (pet->position == POS_FIGHTING) ?
	   POS_STANDING : pet->position);
  encrypt_write (buf, fp);

  if (pet->saving_throw != 0)
    {
      sprintf (buf, "Save %d\n", pet->saving_throw);
      encrypt_write (buf, fp);
    }

  if (pet->alignment != pet->pIndexData->alignment)
    {
      sprintf (buf, "Alig %d\n", pet->alignment);
      encrypt_write (buf, fp);
    }

  sprintf (buf, "ACs  %d %d %d %d\n",
	   pet->armor[0], pet->armor[1], pet->armor[2], pet->armor[3]);
  encrypt_write (buf, fp);

  sprintf (buf, "Attr %d %d %d %d %d %d\n",
	   pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
	   pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
	   pet->perm_stat[STAT_CON], pet->perm_stat[STAT_CHA]);
  encrypt_write (buf, fp);

  sprintf (buf, "AMod %d %d %d %d %d %d\n",
	   pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
	   pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
	   pet->mod_stat[STAT_CON], pet->mod_stat[STAT_CHA]);
  encrypt_write (buf, fp);

  sprintf (buf, "DDice %d %d\n",pet->damage[DICE_NUMBER],pet->damage[DICE_TYPE]);
  encrypt_write (buf,fp);

   if (pet->hitroll != pet->pIndexData->hitroll)
	       {
		             sprintf (buf, "Hit  %d\n", pet->hitroll);
			           encrypt_write (buf, fp);
				       }

     if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
	         {
			       sprintf (buf, "Dam  %d\n", pet->damroll);
			             encrypt_write (buf, fp);
				         }

  if (pet->number_of_attacks > 0)
  {
	  sprintf (buf, "NumA  %d \n", pet->number_of_attacks);
  encrypt_write (buf,fp);
}

  

  for (paf = pet->affected; paf != NULL; paf = paf->next)
    {
      if (paf->type < 0 || paf->type >= MAX_SKILL)
	continue;

      sprintf (buf, "Affc '%s' %3d %3d %3d %3d %3d %10d %d '%s'\n",
	       skill_table[paf->type].name,
	       paf->where, paf->level, paf->duration, paf->modifier,
	       paf->location, paf->bitvector, paf->composition,
	       paf->comp_name);
      encrypt_write (buf, fp);
    }

  sprintf (buf, "End\n");
  encrypt_write (buf, fp);

  return;
}

/* Write a mount */
void fwrite_mount (CHAR_DATA * mount, FILE * fp)
{
  AFFECT_DATA *paf;
  char buf[MAX_STRING_LENGTH * 10];


  sprintf (buf, "#MOUNT\n");
  encrypt_write (buf, fp);

  sprintf (buf, "Vnum %d\n", mount->pIndexData->vnum);
  encrypt_write (buf, fp);

  sprintf (buf, "Stable %d\n", mount->lines);
  encrypt_write (buf, fp);

  sprintf (buf, "Name %s~\n", mount->name);
  encrypt_write (buf, fp);

  sprintf (buf, "PutIn %ld\n", mount->wiznet);
  encrypt_write (buf, fp);

  if (mount->short_descr != mount->pIndexData->short_descr)
    {
      sprintf (buf, "ShD  %s~\n", mount->short_descr);
      encrypt_write (buf, fp);
    }

  if (mount->long_descr != mount->pIndexData->long_descr)
    {
      sprintf (buf, "LnD  %s~\n", mount->long_descr);
      encrypt_write (buf, fp);
    }

  if (mount->description != mount->pIndexData->description)
    {
      sprintf (buf, "Desc %s~\n", mount->description);
      encrypt_write (buf, fp);
    }

  if (mount->race != mount->pIndexData->race)
    {
      sprintf (buf, "Race %s~\n", race_table[mount->race].name);
      encrypt_write (buf, fp);
    }

  if (mount->clan != CLAN_BOGUS)
    {
      sprintf (buf, "Clan %s~\n", get_clan_name_ch (mount));
      encrypt_write (buf, fp);
    }

  sprintf (buf, "Sex  %d\n", mount->sex);
  encrypt_write (buf, fp);

  if (mount->level != mount->pIndexData->level)
    {
      sprintf (buf, "Levl %d\n", mount->level);
      encrypt_write (buf, fp);
    }

  sprintf (buf, "HMV  %d %d %d %d %d %d\n",
	   mount->hit, mount->max_hit, mount->mana, mount->max_mana,
	   mount->move, mount->max_move);
  encrypt_write (buf, fp);

  if (mount->gold > 0)
    {
      sprintf (buf, "Gold %ld\n", mount->gold);
      encrypt_write (buf, fp);
    }

  if (mount->silver > 0)
    {
      sprintf (buf, "Silv %ld\n", mount->silver);
      encrypt_write (buf, fp);
    }

  if (mount->exp > 0)
    {
      sprintf (buf, "Exp  %ld\n", mount->exp);
      encrypt_write (buf, fp);
    }

  if (mount->act != mount->pIndexData->act)
    {
      sprintf (buf, "Act  %s\n", print_flags (mount->act));
      encrypt_write (buf, fp);
    }

  if (mount->act2 != mount->pIndexData->act2)
    {
      sprintf (buf, "Act2  %s\n", print_flags (mount->act2));
      encrypt_write (buf, fp);
    }



  sprintf (buf, "Blok %d\n", mount->blocks_exit);
  encrypt_write (buf, fp);

  if (mount->affected_by != mount->pIndexData->affected_by)
    {
      sprintf (buf, "AfBy %s\n", print_flags (mount->affected_by));
      encrypt_write (buf, fp);
    }

  if (mount->comm != 0)
    {
      sprintf (buf, "Comm %s\n", print_flags (mount->comm));
      encrypt_write (buf, fp);
    }

  if (mount->comm != 0)
    {
      sprintf (buf, "Com2 %s\n", print_flags (mount->comm2));
      encrypt_write (buf, fp);
    }
  

  if (mount->position == POS_MOUNTED)
    mount->position = POS_STANDING;

  sprintf (buf, "Pos  %d\n", mount->position =
	   POS_FIGHTING ? POS_STANDING : mount->position);
  encrypt_write (buf, fp);

  if (mount->saving_throw != 0)
    {
      sprintf (buf, "Save %d\n", mount->saving_throw);
      encrypt_write (buf, fp);
    }

  if (mount->alignment != mount->pIndexData->alignment)
    {
      sprintf (buf, "Alig %d\n", mount->alignment);
      encrypt_write (buf, fp);
    }

  if (mount->hitroll != mount->pIndexData->hitroll)
    {
      sprintf (buf, "Hit  %d\n", mount->hitroll);
      encrypt_write (buf, fp);
    }

  if (mount->damroll != mount->pIndexData->damage[DICE_BONUS])
    {
      sprintf (buf, "Dam  %d\n", mount->damroll);
      encrypt_write (buf, fp);
    }

  sprintf (buf, "ACs  %d %d %d %d\n",
	   mount->armor[0], mount->armor[1], mount->armor[2],
	   mount->armor[3]);
  encrypt_write (buf, fp);

  sprintf (buf, "Attr %d %d %d %d %d %d\n",
	   mount->perm_stat[STAT_STR], mount->perm_stat[STAT_INT],
	   mount->perm_stat[STAT_WIS], mount->perm_stat[STAT_DEX],
	   mount->perm_stat[STAT_CON], mount->perm_stat[STAT_CHA]);
  encrypt_write (buf, fp);

  sprintf (buf, "AMod %d %d %d %d %d %d\n",
	   mount->mod_stat[STAT_STR], mount->mod_stat[STAT_INT],
	   mount->mod_stat[STAT_WIS], mount->mod_stat[STAT_DEX],
	   mount->mod_stat[STAT_CON], mount->mod_stat[STAT_CHA]);
  encrypt_write (buf, fp);

  for (paf = mount->affected; paf != NULL; paf = paf->next)
    {
      if (paf->type < 0 || paf->type >= MAX_SKILL)
	continue;

      sprintf (buf, "Affc '%s' %3d %3d %3d %3d %3d %10d %d '%s'\n",
	       skill_table[paf->type].name,
	       paf->where, paf->level, paf->duration, paf->modifier,
	       paf->location, paf->bitvector, paf->composition,
	       paf->comp_name);
      encrypt_write (buf, fp);
    }

  sprintf (buf, "End\n\n");
  encrypt_write (buf, fp);

//  if (mount->carrying != NULL)
 //   fwrite_obj (mount, mount->carrying, fp, 0);

  return;
}


/*
 * Write an object and its contents.
 */
void fwrite_obj_aux (CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest)
{
  char buf[MAX_STRING_LENGTH * 10];
  EXTRA_DESCR_DATA *ed;
  int i;
  AFFECT_DATA *paf;

  /*
   * Castrate storage characters.
   */
  //IBLIS 5/21/03 - To deal with the new xp system and having to store
  //objects, I took away the OOL poof (at least for the time being)
  //if ((ch->level < obj->level - 20 && obj->item_type != ITEM_CONTAINER)
  //  || (obj->item_type == ITEM_KEY && !obj->value[0])
  //    || (obj->item_type == ITEM_MAP && !obj->value[0]))
  // return;

  if (!IS_NPC (ch))
    sprintf (buf, "#O\n");
  else
    sprintf (buf, "#M\n");
  encrypt_write (buf, fp);

  sprintf (buf, "Vnum %d\n", obj->pIndexData->vnum);
  encrypt_write (buf, fp);

  if (obj->enchanted)
    {
      sprintf (buf, "Enchanted\n");
      encrypt_write (buf, fp);
    }

  sprintf (buf, "Nest %d\n", iNest);
  encrypt_write (buf, fp);

  /* these data are only used if they do not match the defaults */

  if (obj->name != obj->pIndexData->name)
    {
      sprintf (buf, "Name %s~\n", obj->name);
      encrypt_write (buf, fp);
    }

  if (obj->owner != NULL && obj->owner[0] != '\0')
    {
      sprintf (buf, "OwnR %s~\n", obj->owner);
      encrypt_write (buf, fp);
    }

  if (obj->short_descr != obj->pIndexData->short_descr)
    {
      sprintf (buf, "ShD  %s~\n", obj->short_descr);
      encrypt_write (buf, fp);
    }

  if (obj->description != obj->pIndexData->description)
    {
      sprintf (buf, "Desc %s~\n", obj->description);
      encrypt_write (buf, fp);
    }

  for (i = 0; i < MAX_EXTRA_FLAGS; i++)
    {
      if (obj->extra_flags[i] != obj->pIndexData->extra_flags[i])
      {
         sprintf (buf, "ExF %d %ld\n", i, obj->extra_flags[i]);
         encrypt_write (buf, fp);
      }
    }
   
  if (obj->wear_flags != obj->pIndexData->wear_flags)
    {
      sprintf (buf, "WeaF %ld\n", obj->wear_flags);
      encrypt_write (buf, fp);
    }
  // Akamai 4/30/99 - support for Class/race specific eq
  if (obj->Class_flags != obj->pIndexData->Class_flags)
    {
      sprintf (buf, "ClassF %ld\n", obj->Class_flags);
      encrypt_write (buf, fp);
    }
  if (obj->race_flags != obj->pIndexData->race_flags)
    {
      sprintf (buf, "RaceF %ld\n", obj->race_flags);
      encrypt_write (buf, fp);
    }
  if (obj->clan_flags != obj->pIndexData->clan_flags)
    {
      sprintf (buf, "ClanF %ld\n", obj->clan_flags);
      encrypt_write (buf, fp);
    }
  //Iblis 6/12/04 - Store the slot a card is in in a binder/deck
  if (obj->bs_capacity[0] != 0)
  {
	  sprintf (buf, "Cardslot %d\n", obj->bs_capacity[0]);
	  encrypt_write (buf, fp);
  }
  // Adeon 6/29/03 -- Allow player specific ownership of items  
  if (obj->plr_owner != NULL)
    {
      sprintf (buf, "POwn %s~\n", obj->plr_owner);
      encrypt_write (buf, fp);
    }

  // Iblis 12/31/03 -- Object Trigger Data
  for (i=0;i<MAX_OBJ_TRIGS;i++)
    {
      if (obj->objtrig[i] != NULL && obj->objtrig[i]->pIndexData->vnum != obj->pIndexData->obj_trig_vnum[i])
      {
        sprintf (buf, "OTVnum2 %d\n", obj->objtrig[0]->pIndexData->vnum);
	encrypt_write (buf, fp);
        for (i=1;i<MAX_OBJ_TRIGS;i++)
	{
          if (obj->objtrig[i])
            sprintf (buf, "%d\n", obj->objtrig[i]->pIndexData->vnum);
	  else sprintf (buf, "%d\n", 0);
	  encrypt_write (buf, fp);
	}
        break;
      }
    }

  if (obj->item_type != obj->pIndexData->item_type)
    {
      sprintf (buf, "Ityp %d\n", obj->item_type);
      encrypt_write (buf, fp);
    }

  if (obj->weight != obj->pIndexData->weight)
    {
      sprintf (buf, "Wt   %d\n", obj->weight);
      encrypt_write (buf, fp);
    }

  if (obj->condition != obj->pIndexData->condition)
    {
      sprintf (buf, "Cond %d\n", obj->condition);
      encrypt_write (buf, fp);
    }

  /* variable data */

  sprintf (buf, "WearL %ld\n", obj->wear_loc);
  encrypt_write (buf, fp);
  if (obj->level != obj->pIndexData->level)
    {
      sprintf (buf, "Lev  %d\n", obj->level);
      encrypt_write (buf, fp);
    }
  if (obj->timer != 0)
    {
      sprintf (buf, "Time %d\n", obj->timer);
      encrypt_write (buf, fp);
    }


  sprintf (buf, "Cost %d\n", obj->cost);
  encrypt_write (buf, fp);

  if (obj->value[0] != obj->pIndexData->value[0]
      || obj->value[1] != obj->pIndexData->value[1]
      || obj->value[2] != obj->pIndexData->value[2]
      || obj->value[3] != obj->pIndexData->value[3]
      || obj->value[4] != obj->pIndexData->value[4]
      || obj->value[5] != obj->pIndexData->value[5]
      || obj->value[6] != obj->pIndexData->value[6]
      || obj->value[7] != obj->pIndexData->value[7]
      || obj->value[8] != obj->pIndexData->value[8]
      || obj->value[9] != obj->pIndexData->value[9]
      || obj->value[10] != obj->pIndexData->value[10]
      || obj->value[11] != obj->pIndexData->value[11]
      || obj->value[12] != obj->pIndexData->value[12]
      )
    {
      sprintf (buf, "Vlues  %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
	       obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	       obj->value[4], obj->value[5], obj->value[6], obj->value[7],
	       obj->value[8], obj->value[9], obj->value[10], obj->value[11], obj->value[12]);
      encrypt_write (buf, fp);
    }


  switch (obj->item_type)
    {
    case ITEM_VIAL:
      if (obj->value[0] > 0)
	{
	  sprintf (buf, "Spell 0 '%s'\n", skill_table[obj->value[0]].name);
	  encrypt_write (buf, fp);
	}
      break;

    case ITEM_POTION:
    case ITEM_SCROLL:
      if (obj->value[1] > 0)
	{
	  sprintf (buf, "Spell 1 '%s'\n", skill_table[obj->value[1]].name);
	  encrypt_write (buf, fp);
	}

      if (obj->value[2] > 0)
	{
	  sprintf (buf, "Spell 2 '%s'\n", skill_table[obj->value[2]].name);
	  encrypt_write (buf, fp);
	}

      if (obj->value[3] > 0)
	{
	  sprintf (buf, "Spell 3 '%s'\n", skill_table[obj->value[3]].name);
	  encrypt_write (buf, fp);
	}
      break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
      if (obj->value[3] > 0 && obj->value[3] < MAX_SKILL)
	{
	  sprintf (buf, "Spell 3 '%s'\n", skill_table[obj->value[3]].name);
	  encrypt_write (buf, fp);
	}

      break;
    }

  for (paf = obj->affected; paf != NULL; paf = paf->next)
    {
      AFFECT_DATA *taf=NULL;
      if (paf->type < 0 || paf->type >= MAX_SKILL)
	continue;
      for (taf = obj->pIndexData->affected;taf != NULL;taf = taf->next)
      {
        if (taf->type == paf->type)
          break;
      }
      if (taf != NULL && taf->type == paf->type)
       continue;
      sprintf (buf, "Affc '%s' %3d %3d %3d %3d %3d %10d %d '%s'\n",
	       skill_table[paf->type].name,
	       paf->where,
	       paf->level,
	       paf->duration,
	       paf->modifier,
	       paf->location,
	       paf->bitvector,
	       paf->composition, paf->comp_name);
      encrypt_write (buf, fp);
    }

  for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
    {
      sprintf (buf, "ExDe %s~ %s~\n", ed->keyword, ed->description);
      encrypt_write (buf, fp);
    }

  sprintf (buf, "End\n\n");
  encrypt_write (buf, fp);
//end target
  return;
}

/*
 * Save a character's inventory to disk.
 */

void fwrite_obj (CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest)
{

  /*
   * Slick recursion to write lists backwards,
   *   so loading them will load in forwards order.
   */
  if (obj->next_content != NULL)
    {
      fwrite_obj (ch, obj->next_content, fp, iNest);
    }
  /*  
   * This call processes the save of a specific object.  This call was
   * made to get around a recursive memory problem (stack overflow) caused
   * by char buf[] being allocated for each recursive call. char buf[]
   * is now declared within fwrite_obj_aux()
   */

  fwrite_obj_aux (ch, obj, fp, iNest);

  if (obj->contains != NULL)
    fwrite_obj (ch, obj->contains, fp, iNest + 1);
  return;
}


void fwrite_objdb (OBJ_DATA * obj, FILE * fp, int iNest)
{
  EXTRA_DESCR_DATA *ed;
  AFFECT_DATA *paf;
  int i=0;

  /*
   * Slick recursion to write lists backwards,
   *   so loading them will load in forwards order.
   */
  if (obj->next_content != NULL)
    fwrite_objdb (obj->next_content, fp, iNest);

  fprintf (fp, "#O\n");

  fprintf (fp, "Vnum %d\n", obj->pIndexData->vnum);

  if (obj->enchanted)
    fprintf (fp, "Enchanted\n");

  fprintf (fp, "Nest %d\n", iNest);

  /* these data are only used if they do not match the defaults */

  if (obj->name != obj->pIndexData->name)
    fprintf (fp, "Name %s~\n", obj->name);

  if (obj->owner != NULL && obj->owner[0] != '\0')
    fprintf (fp, "OwnR %s~\n", obj->owner);

  if (obj->short_descr != obj->pIndexData->short_descr)
    fprintf (fp, "ShD  %s~\n", obj->short_descr);
  if (obj->description != obj->pIndexData->description)
    fprintf (fp, "Desc %s~\n", obj->description);
  for (i = 0; i < MAX_EXTRA_FLAGS; i++)
    {
       if (obj->extra_flags[i] != obj->pIndexData->extra_flags[i])
       {
         fprintf (fp, "ExF %d %ld\n", i, obj->extra_flags[i]);
       }
    } 
  if (obj->wear_flags != obj->pIndexData->wear_flags)
    fprintf (fp, "WeaF %ld\n", obj->wear_flags);

  // Akamai 4/30/99 - support Class/race equipment flags
  if (obj->Class_flags != obj->pIndexData->Class_flags)
    fprintf (fp, "ClassF %ld\n", obj->Class_flags);
  if (obj->race_flags != obj->pIndexData->race_flags)
    fprintf (fp, "RaceF %ld\n", obj->race_flags);
  if (obj->clan_flags != obj->pIndexData->clan_flags)
    fprintf (fp, "ClanF %ld\n", obj->clan_flags);
  //Iblis 6/12/04 - Store the slot a card is in in a binder/deck
  if (obj->bs_capacity[0] != 0)
    fprintf (fp, "Cardslot %d\n", obj->bs_capacity[0]);

  // Adeon 6/29/03 -- Allow player specific ownership of items  
  if (obj->plr_owner != obj->pIndexData->plr_owner)
    fprintf (fp, "POwn %s~\n", obj->plr_owner);

  // Iblis 12/31/03 -- Object Trigger Data
  for (i=0;i<MAX_OBJ_TRIGS;i++)
  {
    if (obj->objtrig[i] != NULL && obj->objtrig[i]->pIndexData->vnum != obj->pIndexData->obj_trig_vnum[i])
    {
      fprintf (fp, "OTVnum2 %d\n", obj->objtrig[0]->pIndexData->vnum);
      for (i=0;i<MAX_OBJ_TRIGS;i++)
      {
	if (obj->objtrig[i])
          fprintf (fp, "%d\n", obj->objtrig[i]->pIndexData->vnum);	   
	else fprintf (fp, "%d\n", 0);
      }
      break;
    }
  }


  if (obj->item_type != obj->pIndexData->item_type)
    fprintf (fp, "Ityp %d\n", obj->item_type);
  if (obj->weight != obj->pIndexData->weight)
    fprintf (fp, "Wt   %d\n", obj->weight);
  if (obj->condition != obj->pIndexData->condition)
    fprintf (fp, "Cond %d\n", obj->condition);

  /* variable data */

  fprintf (fp, "WearL %ld\n", obj->wear_loc);
  if (obj->level != obj->pIndexData->level)
    fprintf (fp, "Lev  %d\n", obj->level);
  if (obj->timer != 0)
    fprintf (fp, "Time %d\n", obj->timer);
  fprintf (fp, "Cost %d\n", obj->cost);
  if (obj->value[0] != obj->pIndexData->value[0]
      || obj->value[1] != obj->pIndexData->value[1]
      || obj->value[2] != obj->pIndexData->value[2]
      || obj->value[3] != obj->pIndexData->value[3]
      || obj->value[4] != obj->pIndexData->value[4]
      || obj->value[5] != obj->pIndexData->value[5]
      || obj->value[6] != obj->pIndexData->value[6]
      || obj->value[7] != obj->pIndexData->value[7]
      || obj->value[8] != obj->pIndexData->value[8]
      || obj->value[9] != obj->pIndexData->value[9]
      || obj->value[10] != obj->pIndexData->value[10]
      || obj->value[11] != obj->pIndexData->value[11]
      || obj->value[12] != obj->pIndexData->value[12]
      )
    fprintf (fp, "Vlues  %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
	     obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	     obj->value[4], obj->value[5], obj->value[6], obj->value[7],
	     obj->value[8], obj->value[9], obj->value[10], obj->value[11], obj->value[12]);


  switch (obj->item_type)
    {
    case ITEM_VIAL:
      if (obj->value[0] > 0)
	{
	  fprintf (fp, "Spell 0 '%s'\n", skill_table[obj->value[0]].name);
	}
      break;

    case ITEM_POTION:
    case ITEM_SCROLL:
      if (obj->value[1] > 0)
	{
	  fprintf (fp, "Spell 1 '%s'\n", skill_table[obj->value[1]].name);
	}

      if (obj->value[2] > 0)
	{
	  fprintf (fp, "Spell 2 '%s'\n", skill_table[obj->value[2]].name);
	}

      if (obj->value[3] > 0)
	{
	  fprintf (fp, "Spell 3 '%s'\n", skill_table[obj->value[3]].name);
	}
      break;
    case ITEM_CLAN_DONATION:	/* new_clan */
      fprintf (fp, "Clan %s~\n", get_clan_name (obj->value[1]));
      break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
      if (obj->value[3] > 0 && obj->value[3] < MAX_SKILL)
	{
	  fprintf (fp, "Spell 3 '%s'\n", skill_table[obj->value[3]].name);
	}
      break;
    }

  for (paf = obj->affected; paf != NULL; paf = paf->next)
    {
      AFFECT_DATA *taf=NULL;
      if (paf->type < 0 || paf->type >= MAX_SKILL)
	continue;
      for (taf = obj->pIndexData->affected;taf != NULL;taf = taf->next)
      {
        if (taf->type == paf->type)
		break;
      }
      if (taf != NULL && taf->type == paf->type)
        continue;
      fprintf (fp, "Affc '%s' %3d %3d %3d %3d %3d %10d %d '%s'\n",
	       skill_table[paf->type].name,
	       paf->where,
	       paf->level,
	       paf->duration,
	       paf->modifier,
	       paf->location,
	       paf->bitvector,
	       paf->composition, paf->comp_name);
    }

  for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
    {
      fprintf (fp, "ExDe %s~ %s~\n", ed->keyword, ed->description);
    }

  fprintf (fp, "End\n\n");

  if (obj->contains != NULL)
    fwrite_objdb (obj->contains, fp, iNest + 1);

  return;
}

void load_ctransports ()
{
  FILE *sfp;

  sfp = fopen ("ctransports.txt", "r");

  if (sfp == NULL)
    return;

  for (;;)
    {
      char *word;

      if (fread_letter (sfp) != '#')
	return;

      word = fread_word (sfp);

      if (!str_cmp (word, "CTRANSPORT"))
	load_one_ctransport (sfp);
    }
}



void load_one_ctransport (FILE * sfp)
{
  const char *word;
  bool fMatch;
  OBJ_DATA *obj;
  OBJ_INDEX_DATA *obj_index;
  int vnum, inroom = 0;

  word = fread_word (sfp);

  if (str_cmp (word, "Vnum"))
    return;

  vnum = fread_number (sfp);

  if (vnum <= 0)
    return;

  obj_index = get_obj_index (vnum);

  if (obj_index == NULL)
    return;

  obj = create_object (obj_index, 0);

  for (;;)
    {
      word = feof(sfp) ? "End" : fread_word(sfp);

      switch (UPPER (word[0]))
	{
	case 'C':
	  // Akamai 4/30/99 - support Class/race specific objects
	  KEY ("ClanF", obj->clan_flags, fread_flag (sfp));
	  KEY ("Cardslot", obj->bs_capacity[0], fread_number (sfp));
	  KEY ("ClanFlags", obj->clan_flags, fread_flag (sfp));
	  KEY ("ClassF", obj->Class_flags, fread_flag (sfp));
	  KEY ("ClassFlags", obj->Class_flags, fread_flag (sfp));
	  KEY ("Cond", obj->condition, fread_flag (sfp));
	  KEY ("Cost", obj->cost, fread_number (sfp));
	  break;

	case 'D':
	  KEY ("Desc", obj->description, fread_string (sfp));
	  break;

	case 'E':
	  KEY ("ExtF", obj->extra_flags[0], fread_number (sfp));
	  if (!str_cmp(word,"ExF"))
	  {
	    int ai;
	    ai = fread_number(sfp);
	    if (ai > MAX_EXTRA_FLAGS)
	    {
		    bug("Extra Flag Index out of range!",0);
		    break;
	    }
	    obj->extra_flags[ai] = fread_number (sfp);
	    fMatch = TRUE;
	  }
	  if (!str_cmp (word, "End"))
	    {
	      if (inroom != 0)
		{
		  ROOM_INDEX_DATA *location;

		  location = get_room_index (inroom);

		  if (location == NULL)
		    {
		      extract_obj (obj);
		      return;
		    }

		  obj_to_room (obj, location);
		}
	      return;
	    }
	  break;

	case 'I':
	  KEY ("InRoom", inroom, fread_number (sfp));
	  break;

	case 'N':
	  KEY ("Name", obj->name, fread_string (sfp));
	  break;

	case 'O':
	  KEY ("OwnR", obj->owner, fread_string (sfp));
	  break;

	case 'R':
	  // Akamai 4/30/99 - support Class/race specific objects
	  KEY ("RaceF", obj->race_flags, fread_flag (sfp));
	  break;

	case 'S':
	  KEY ("ShD", obj->short_descr, fread_string (sfp));
	  break;

	case 'T':
	  KEY ("Time", obj->timer, fread_number (sfp));
	  break;

	case 'W':
	  KEY ("WeaF", obj->wear_flags, fread_number (sfp));
	  break;
	}
    }

}

void save_ctransports ()
{
  FILE *sfp;
  OBJ_DATA *obj;


  sfp = fopen ("ctransports.txt", "w");

  if (sfp == NULL)
    return;

  for (obj = object_list; obj; obj = obj->next)
    if (obj->item_type == ITEM_CTRANSPORT && obj->in_room != NULL)
      fwrite_ctransport (obj, sfp);

  fclose (sfp);
}

void fwrite_ctransport (OBJ_DATA * obj, FILE * fp)
{
  EXTRA_DESCR_DATA *ed;
  int i;

  fprintf (fp, "#CTRANSPORT\n");
  fprintf (fp, "Vnum %d\n", obj->pIndexData->vnum);
  fprintf (fp, "InRoom %d\n", obj->in_room->vnum);

  if (obj->name != obj->pIndexData->name)
    fprintf (fp, "Name %s~\n", obj->name);
  if (obj->owner != NULL && obj->owner[0] != '\0')
    fprintf (fp, "OwnR %s~\n", obj->owner);
  if (obj->short_descr != obj->pIndexData->short_descr)
    fprintf (fp, "ShD  %s~\n", obj->short_descr);
  if (obj->description != obj->pIndexData->description)
    fprintf (fp, "Desc %s~\n", obj->description);
  for (i = 0; i < MAX_EXTRA_FLAGS; i++)
    {
      if (obj->extra_flags[i] != obj->pIndexData->extra_flags[i])
        fprintf (fp, "ExF %d %ld\n", i, obj->extra_flags[i]);
    }
  if (obj->wear_flags != obj->pIndexData->wear_flags)
    fprintf (fp, "WeaF %ld\n", obj->wear_flags);

  // Akamai 4/30/99 - support Class/race specific equipment/objects
  if (obj->Class_flags != obj->pIndexData->Class_flags)
    fprintf (fp, "ClassF %ld\n", obj->Class_flags);
  if (obj->race_flags != obj->pIndexData->race_flags)
    fprintf (fp, "RaceF %ld\n", obj->race_flags);
  if (obj->clan_flags != obj->pIndexData->clan_flags)
    fprintf (fp, "ClanF %ld\n", obj->clan_flags);
  //Iblis 6/12/04 - Store the slot a card is in in a binder/deck
  if (obj->bs_capacity[0] != 0)
    fprintf (fp, "Cardslot %d\n", obj->bs_capacity[0]);

  if (obj->item_type != obj->pIndexData->item_type)
    fprintf (fp, "Ityp %d\n", obj->item_type);
  if (obj->weight != obj->pIndexData->weight)
    fprintf (fp, "Wt   %d\n", obj->weight);
  if (obj->condition != obj->pIndexData->condition)
    fprintf (fp, "Cond %d\n", obj->condition);
  fprintf (fp, "WearL %ld\n", obj->wear_loc);
  if (obj->level != obj->pIndexData->level)
    fprintf (fp, "Lev  %d\n", obj->level);
  if (obj->timer != 0)
    fprintf (fp, "Time %d\n", obj->timer);
  fprintf (fp, "Cost %d\n", obj->cost);
  if (obj->value[0] != obj->pIndexData->value[0]
      || obj->value[1] != obj->pIndexData->value[1]
      || obj->value[2] != obj->pIndexData->value[2]
      || obj->value[3] != obj->pIndexData->value[3]
      || obj->value[4] != obj->pIndexData->value[4]
      || obj->value[5] != obj->pIndexData->value[5]
      || obj->value[6] != obj->pIndexData->value[6]
      || obj->value[7] != obj->pIndexData->value[7]
      || obj->value[8] != obj->pIndexData->value[8]
      || obj->value[9] != obj->pIndexData->value[9]
      || obj->value[10] != obj->pIndexData->value[10]
      || obj->value[11] != obj->pIndexData->value[11]
      || obj->value[12] != obj->pIndexData->value[12]
      )
    fprintf (fp, "Vlues  %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
	     obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	     obj->value[4], obj->value[5], obj->value[6], obj->value[7],
	     obj->value[8], obj->value[9], obj->value[10], obj->value[11], obj->value[12]);

  for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
    {
      fprintf (fp, "ExDe %s~ %s~\n", ed->keyword, ed->description);
    }

  fprintf (fp, "End\n\n");

  return;
}


bool load_char_obj (DESCRIPTOR_DATA * d, char *name)
{
	return load_char_obj_new (d,name,NULL);
}
	



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj_new (DESCRIPTOR_DATA * d, char *name, char* filename)
{
  char strsave[MAX_INPUT_LENGTH];
  char buf[100];
  CHAR_DATA *ch;
  FILE *fp;
  bool found;
  int stat;
  int i;


  ch = new_char ();
  ch->pcdata = new_pcdata ();

  d->character = ch;
  ch->desc = d;
//  ch->switched = NULL;


  ch->name = str_dup (name);
  ch->id = get_pc_id ();
  ch->race = race_lookup ("human");
  ch->act = PLR_NOSUMMON;
  ch->act2 = 0;
  ch->blocks_exit = -1;
  ch->sink_timer = -1;
  ch->comm2 = COMM_NORANT;
  ch->contaminator = NULL;

  //Iblis - 5/12/03 - Set it so ALL channels are on by default
  ch->comm = COMM_COMBINE | COMM_PROMPT;	//| COMM_NOGOSSIP | COMM_NOGRATS | COMM_NOMUSIC |
  //COMM_NOQUESTION;
  ch->prompt = str_dup ("`a<`lHP`a:`F%h `lMA`a:`I%m `lMV`a:`J%v`a>`` %S");

  //Shinowlan - 3/2/99
  //used by do_kai to determine lag to apply to kai user.

  ch->pcdata->last_kai = current_time;
  ch->pcdata->last_pkdeath = 0;
  ch->pcdata->confirm_delete = FALSE;
  ch->pcdata->last_fight = 0;
//  ch->pcdata->last_loot = 0;
  ch->pcdata->events_won[BR_WON] = 0;
  ch->pcdata->events_won[BR_IN] = 0;
  ch->pcdata->events_won[SD_WON] = 0;
  ch->pcdata->events_won[SD_IN] = 0;
  ch->pcdata->events_won[TEAM_SD_WON] = 0;
  ch->pcdata->events_won[TEAM_SD_IN] = 0;
  ch->pcdata->other_deaths = 0;
  ch->pcdata->been_killed[PLAYER_KILL] = 0;
  ch->pcdata->been_killed[MOB_KILL] = 0;
  ch->pcdata->been_killed[BR_KILL] = 0;
  ch->pcdata->been_killed[SD_KILL] = 0;
  ch->pcdata->been_killed[ARENA_KILL] = 0;
  ch->pcdata->has_killed[PLAYER_KILL] = 0;
  ch->pcdata->has_killed[MOB_KILL] = 0;
  ch->pcdata->has_killed[BR_KILL] = 0;
  ch->pcdata->has_killed[SD_KILL] = 0;
  ch->pcdata->has_killed[ARENA_KILL] = 0;
  ch->pcdata->pwd = str_dup ("");
  ch->pcdata->bamfin = str_dup ("");
  ch->pcdata->speedwalk = str_dup ("");
  ch->pcdata->bamfout = str_dup ("");
  ch->pcdata->whoinfo = str_dup ("");
  ch->pcdata->pretitle = str_dup ("");
  ch->pcdata->battlecry = str_dup ("");
  ch->pcdata->plan = str_dup ("");
  ch->pcdata->title = str_dup ("");
  ch->pcdata->restoremsg = str_dup ("");
  ch->pcdata->email_addr = str_dup ("");
  ch->pcdata->referrer = str_dup ("");
  for (stat = 0; stat < MAX_STATS; stat++)
    ch->perm_stat[stat] = 13;
  ch->pcdata->condition[COND_THIRST] = 48;
  ch->pcdata->condition[COND_FULL] = 48;
  ch->pcdata->condition[COND_HUNGER] = 48;
  ch->pcdata->security = 0;	/* OLC */
  ch->pcdata->last_saved_room = 0;
  ch->pcdata->souls = 0;
  ch->pcdata->reaver_chant = -1;
  ch->pcdata->chant_wait = 0;
  ch->pcdata->hp_gained = 0;
  ch->pcdata->mana_gained = 0;
  ch->pcdata->move_gained = 0;
  ch->pcdata->rppoints = 0;
  ch->pcdata->nhpoints = 0;
  ch->pcdata->qpoints = 0;
  ch->pcdata->avatar_type = 0;
  ch->pcdata->old_race = -1;
  ch->pcdata->has_reincarnated = FALSE;
  ch->pcdata->old_Class = -1;
  ch->pcdata->prevent_escape = 0;
  ch->pcdata->noblinking = 0;
  ch->pcdata->falcon_alert = 0;
  //IBLIS 5/31/03 - NOT WRITTEN TO A FILE
  ch->pcdata->feigned = FALSE;
  //IBLIS 6/07/03 - NOT WRITTEN TO A FILE
  ch->pcdata->last_death_counter = 0;
  ch->pcdata->voted = VOTED_NOT;
  ch->pcdata->xp_last_death = 0;
  ch->pcdata->br_points = 0;
  ch->pcdata->br_death_points = 0;
  ch->pcdata->agg_list = NULL;
  ch->pcdata->falcon_wait = 0;
  ch->pcdata->falcon_recipient = NULL;
  ch->pcdata->falcon_object = NULL;
  ch->pcdata->nplayed = 0;
  ch->pcdata->nage = 0;
  ch->pcdata->nochan_ticks = 0;
  ch->pcdata->shutup_ticks= 0;
  ch->pcdata->backup_hit = 20;
  ch->pcdata->backup_move = 0;
  ch->pcdata->backup_mana = 0;
  //IBLIS 5/18/03 - NOTE group_exp is NOT written to/from players file
  //  There is no need to since it only accumulates when group, and when
  //  you quit/logon, you don't have a group, so it should always reset to 0 then
  ch->pcdata->group_exp = 0;
  ch->pcdata->new_style = 0;
  ch->pcdata->fishing = 0;
  ch->pcdata->fishing_ll = 0;
  ch->pcdata->fishing_weight = 0;
  // Below NOT saved
  ch->pcdata->knock_time = 0;
  ch->pcdata->pushed_mobs_counter = 0;
  ch->pcdata->nogate = FALSE;
  // Below 3 NOT SAVED
  ch->pcdata->aggression = 0;
  ch->pcdata->corpse = NULL;
  ch->pcdata->corpse_to_room = NULL;
  ch->pcdata->corpse_timer = 0;
  ch->pcdata->familiar = NULL;
  ch->pcdata->soul_link = NULL;
  ch->pcdata->familiar_type = 0;
  ch->pcdata->familiar_max_hit = 20;
  ch->pcdata->familiar_max_mana = 100;
  ch->pcdata->familiar_max_move = 100;
  ch->pcdata->familiar_exp = 0;
  ch->pcdata->familiar_level = 1;
  ch->pcdata->familiar_gained_hp = 0;
  ch->pcdata->familiar_gained_mana = 0;
  ch->pcdata->familiar_gained_move = 0;
  ch->pcdata->familiar_name = str_dup ("");
  ch->pcdata->flaming = 0;
  ch->pcdata->totalxp = 0;
  ch->pcdata->totalsouls = 0;
  ch->pcdata->tg_won = 0;
  ch->pcdata->tg_lost = 0;
  ch->pcdata->quest_mob = NULL;
  ch->pcdata->quest_obj = NULL;
  ch->pcdata->quest_ticks = 0;
  ch->pcdata->total_quest_finds = 0;
  ch->pcdata->total_quest_hunts = 0;
  ch->pcdata->total_qf_attempted = 0;
  ch->pcdata->oow_ticks = 0;
  ch->pcdata->total_trees_chopped = 0;
  ch->pcdata->total_trees_milled = 0;
  ch->pcdata->total_objs_sold = 0;
  ch->pcdata->total_money_made = 0;
  ch->pcdata->total_cg_won = 0;
  ch->pcdata->total_cg_lost = 0;
  ch->pcdata->cg_next = NULL;
  ch->pcdata->cg_ticks = 0;
  ch->pcdata->cg_main_mob = NULL;
  ch->pcdata->cg_secondary_mob = NULL;
  ch->pcdata->cg_terrain = NULL;
  ch->pcdata->cg_lcp = NULL;
  ch->pcdata->cg_state = 0;
  ch->pcdata->cg_timer = 0;
  ch->pcdata->pain_points = 0;
  ch->pcdata->personality = 0;
  ch->pcdata->all_xp_familiar = FALSE;
/*  for (i = 0; i < (MAX_QUEST_BITS/8); i++)
    ch->pcdata->qbits[i] = '\0';*/
  ch->Class2 = -1;
  ch->Class3 = -1;
  //Iblis 1/06/04 Set the proper values of modified skills
  for (i = 0; i < MAX_SKILL; i++)
    {
      if (skill_table[i].name != NULL)
	  //     && ch->pcdata->mod_learned[sn] != -1)
        {
          ch->pcdata->mod_learned[i] = -999;
        }
    }
  found = FALSE;
  fclose (fpReserve);

  is_encrypted = FALSE;


#if defined(unix)
  /* decompress if .gz file exists */
  sprintf (strsave, "%s%c%s%s", PLAYER_DIR, toupper (name[0]),
	   capitalize (name), ".gz");
  if ((fp = fopen (strsave, "r")) != NULL)
    {
      fclose (fp);
      sprintf (buf, "gzip -dfq %s", strsave);
      system (buf);
    }
#endif

  if (filename != NULL)
    sprintf (strsave, "%s%c/%s%s", PLAYER_DIR, name[0], capitalize (name),
	     filename);
//  else
//    sprintf (strsave, "%s%c/%s%s", PLAYER_DIR, name[0], capitalize (name),
//	     PFILE_BACKUP_DUEL);

  if ((fp = fopen (strsave, "r")) != NULL)
    {
      {
	int iNest;

	for (iNest = 0; iNest < MAX_NEST; iNest++)
	  rgObjNest[iNest] = NULL;

	found = TRUE;
	for (;;)
	  {
	    char letter;
	    char *word;

	    letter = fread_letter (fp);
	    if (letter == '*')
	      {
		fread_to_eol (fp);
		continue;
	      }

	    if (letter != '#')
	      {
		bug ("Load_char_obj: # not found.", 0);
		break;
	      }

	    word = fread_word (fp);
	    if (!str_cmp (word, "PLAYER"))
	      fread_char (ch, fp);
	    else if (!str_cmp (word, "OBJECT"))
	      fread_obj (ch, fp);
	    else if (!str_cmp (word, "O"))
	      fread_obj (ch, fp);
	    else if (!str_cmp (word, "M"))
	      fread_obj (loadmount, fp);
	    else if (!str_cmp (word, "PET"))
	      {
		fread_pet (ch, fp);
	      }
	    else if (!str_cmp (word, "MOUNT"))
	      fread_mount (ch, fp);
	    else if (!str_cmp (word, "RoomRLE"))
	      fread_rle (ch->pcdata->explored, fp);
	    else if (!str_cmp (word, "SWORD"))
	      fread_sword (ch, fp);
            else if (!str_cmp (word, "QbitsRLE"))
              fread_rle (ch->pcdata->qbits, fp);
	    else if (!str_cmp (word, "END"))
	      break;
	    else
	      {
		bug ("Load_char_obj: bad section.", 0);
		break;
	      }
	  }
	fclose (fp);
      }

      fpReserve = fopen (NULL_FILE, "r");


      /* initialize race */
      if (found)
	{
	  int i;

	  if (ch->race == 0)
	    ch->race = race_lookup ("human");

	  ch->size = pc_race_table[ch->race].size;
	  if (ch->Class != PC_CLASS_MONK && ch->race != PC_RACE_KALIAN)
	    ch->dam_type = 17;	/*punch */
	  if (ch->race == PC_RACE_NERIX)
  	    ch->dam_type = 41;  /*lightning claw*/

	  for (i = 0; i < 5; i++)
	    {
	      if (pc_race_table[ch->race].skills[i] == NULL)
		break;
	      group_add (ch, pc_race_table[ch->race].skills[i], FALSE);
	    }
	  ch->affected_by = ch->affected_by | race_table[ch->race].aff;
	  ch->imm_flags = ch->imm_flags | race_table[ch->race].imm;
	  ch->res_flags = ch->res_flags | race_table[ch->race].res;
	  ch->vuln_flags = ch->vuln_flags | race_table[ch->race].vuln;
	  ch->act = ch->act | race_table[ch->race].act;
	  if (ch->race == PC_RACE_SWARM)
            ch->act2 = ch->act2 | ACT_NOMOVE;
	  ch->form = race_table[ch->race].form;
	  ch->parts = race_table[ch->race].parts;
	}


      /* RT initialize skills */

      if (found && ch->version < 2)
	{			/* need to add the new skills */
	  /*      group_add(ch, "rom basics", FALSE); */
	  group_add (ch, Class_table[ch->Class].base_group, FALSE);
	  group_add (ch, Class_table[ch->Class].default_group, TRUE);
	}

      /* fix levels */
      if (found && ch->version < 3 && (ch->level > 35 || ch->trust > 35))
	{
	  switch (ch->level)
	    {
	    case (40):
	      ch->level = 60;
	      break;		/* imp -> imp */
	    case (39):
	      ch->level = 58;
	      break;		/* god -> supreme */
	    case (38):
	      ch->level = 56;
	      break;		/* deity -> god */
	    case (37):
	      ch->level = 53;
	      break;		/* angel -> demigod */
	    }

	  switch (ch->trust)
	    {
	    case (40):
	      ch->trust = 60;
	      break;		/* imp -> imp */
	    case (39):
	      ch->trust = 58;
	      break;		/* god -> supreme */
	    case (38):
	      ch->trust = 56;
	      break;		/* deity -> god */
	    case (37):
	      ch->trust = 53;
	      break;		/* angel -> demigod */
	    case (36):
	      ch->trust = 51;
	      break;		/* hero -> hero */
	    }
	}

      /* ream gold */
      if (found && ch->version < 4)
	{
	  ch->gold /= 100;
	}
      // fix the clan membership of the pet and the mount
      if (ch->pet)
	ch->pet->clan = ch->clan;
      if (ch->mount)
	ch->mount->clan = ch->clan;
      unlink (strsave);
      return (found);
    }

  else
    {
      sprintf (strsave, "%s%c/%s", PLAYER_DIR, name[0], capitalize (name));
      if ((fp = fopen (strsave, "r")) != NULL)
	{
	  int iNest;

	  for (iNest = 0; iNest < MAX_NEST; iNest++)
	    rgObjNest[iNest] = NULL;

	  found = TRUE;
	  for (;;)
	    {
	      char letter;
	      char *word;
	      letter = fread_letter (fp);
	      if (letter == '*')
		{
		  fread_to_eol (fp);
		  continue;
		}

	      if (letter != '#')
		{
		  bug ("Load_char_obj: # not found.", 0);
		  break;
		}

	      word = fread_word (fp);
	      if (!str_cmp (word, "PLAYER"))
		fread_char (ch, fp);
	      else if (!str_cmp (word, "OBJECT"))
		fread_obj (ch, fp);
	      else if (!str_cmp (word, "O"))
		fread_obj (ch, fp);
	      else if (!str_cmp (word, "M"))
		fread_obj (loadmount, fp);
	      else if (!str_cmp (word, "PET"))
		fread_pet (ch, fp);
	      else if (!str_cmp (word, "MOUNT"))
		fread_mount (ch, fp);
	      else if (!str_cmp (word, "RoomRLE"))
		fread_rle (ch->pcdata->explored, fp);
	      else if (!str_cmp (word, "SWORD"))
		fread_sword (ch, fp);
	      else if (!str_cmp (word, "QbitsRLE"))
                fread_rle (ch->pcdata->qbits, fp);
	      else if (!str_cmp (word, "END"))
		break;
	      else
		{
		  bug ("Load_char_obj: bad section.", 0);
		  break;
		}
	    }
	  fclose (fp);
	}

      fpReserve = fopen (NULL_FILE, "r");


      /* initialize race */
      if (found)
	{
	  int i;

	  if (ch->race == 0)
	    ch->race = race_lookup ("human");

	  ch->size = pc_race_table[ch->race].size;
	  if (ch->Class != PC_CLASS_MONK && ch->race != PC_RACE_KALIAN)
	    ch->dam_type = 17;	/*punch */
	  if (ch->race == PC_RACE_NERIX)
	    ch->dam_type = 41;  /*lightning claw */

	  for (i = 0; i < 5; i++)
	    {
	      if (pc_race_table[ch->race].skills[i] == NULL)
		break;
	      group_add (ch, pc_race_table[ch->race].skills[i], FALSE);
	    }
	  ch->affected_by = ch->affected_by | race_table[ch->race].aff;
	  ch->imm_flags = ch->imm_flags | race_table[ch->race].imm;
	  ch->res_flags = ch->res_flags | race_table[ch->race].res;
	  ch->vuln_flags = ch->vuln_flags | race_table[ch->race].vuln;
	  ch->act = ch->act | race_table[ch->race].act;
	  if (ch->race == PC_RACE_SWARM)
              ch->act2 = ch->act2 | ACT_NOMOVE;
	  ch->form = race_table[ch->race].form;
	  ch->parts = race_table[ch->race].parts;
	}


      /* RT initialize skills */

      if (found && ch->version < 2)
	{			/* need to add the new skills */
	  /*      group_add(ch, "rom basics", FALSE); */
	  group_add (ch, Class_table[ch->Class].base_group, FALSE);
	  group_add (ch, Class_table[ch->Class].default_group, TRUE);
	}

      /* fix levels */
      if (found && ch->version < 3 && (ch->level > 35 || ch->trust > 35))
	{
	  switch (ch->level)
	    {
	    case (40):
	      ch->level = 60;
	      break;		/* imp -> imp */
	    case (39):
	      ch->level = 58;
	      break;		/* god -> supreme */
	    case (38):
	      ch->level = 56;
	      break;		/* deity -> god */
	    case (37):
	      ch->level = 53;
	      break;		/* angel -> demigod */
	    }

	  switch (ch->trust)
	    {
	    case (40):
	      ch->trust = 60;
	      break;		/* imp -> imp */
	    case (39):
	      ch->trust = 58;
	      break;		/* god -> supreme */
	    case (38):
	      ch->trust = 56;
	      break;		/* deity -> god */
	    case (37):
	      ch->trust = 53;
	      break;		/* angel -> demigod */
	    case (36):
	      ch->trust = 51;
	      break;		/* hero -> hero */
	    }
	}

      /* ream gold */
      if (found && ch->version < 4)
	{
	  ch->gold /= 100;
	}
      // fix the clan membership of the pet and the mount
      if (ch->pet)
	ch->pet->clan = ch->clan;
      if (ch->mount)
	ch->mount->clan = ch->clan;

// Minax 12-27-02 Player Stats Info





      if (found && !is_linkloading)
	{
	  if (!IS_IMMORTAL(ch))
	    {
	      for (i = 0; i < MAX_GAMESTAT; i++)
		{
		  int j = 0;
		  unsigned long tempstat=0, ttempstat;
		  char *tempname , *ttempname;
		  bool started;
		  started = FALSE;
		  tempname = str_dup(ch->name);
		  switch (i)
		    {
		    case STAT_PK_DEATHS : tempstat = ch->pcdata->been_killed[PLAYER_KILL];break;
		    case STAT_PK_KILLS : tempstat = ch->pcdata->has_killed[PLAYER_KILL];break;
		    case STAT_MOB_KILLS : tempstat = ch->pcdata->has_killed[MOB_KILL];break;
		    case STAT_MOB_DEATHS : tempstat = ch->pcdata->been_killed[MOB_KILL];break;
		    case STAT_ROOMS_EXPLORED : tempstat = roomcount (ch);break;
		    case STAT_HOURS_PLAYED : tempstat = (ch->played + (int) (current_time - ch->logon)) / 3600; break;
		    case STAT_TOTAL_EXP : tempstat = ch->pcdata->totalxp;break;
		    case STAT_EXODUS_SCORE : exodus_score(ch);break;
		    case STAT_SD_KILLS : tempstat = ch->pcdata->has_killed[SD_KILL];break;
		    case STAT_SD_DEATHS : tempstat = ch->pcdata->been_killed[SD_KILL];break;
		    case STAT_SD_IN : tempstat = ch->pcdata->events_won[SD_IN];break;
		    case STAT_SD_WON : tempstat = ch->pcdata->events_won[SD_WON];break;
		    case STAT_BR_KILLS : tempstat = ch->pcdata->has_killed[BR_KILL];break;
                    case STAT_BR_DEATHS : tempstat = ch->pcdata->been_killed[BR_KILL];break;
		    case STAT_BR_IN : tempstat = ch->pcdata->events_won[BR_IN];break;
                    case STAT_BR_WON : tempstat = ch->pcdata->events_won[BR_WON];break;
		    case STAT_ARENA_KILLS : tempstat = ch->pcdata->has_killed[ARENA_KILL];break;
                    case STAT_ARENA_DEATHS : tempstat = ch->pcdata->been_killed[ARENA_KILL];break;
		    case STAT_TOTAL_SOULS : tempstat = ch->pcdata->totalsouls;break;
		    case STAT_TG_LOSS : tempstat = ch->pcdata->tg_lost;break;
		    case STAT_TG_WON : tempstat = ch->pcdata->tg_won;break;
		    case STAT_AVATAR : tempstat = (ch->race == PC_RACE_AVATAR)?ch->pcdata->totalxp:0;break;
		    case STAT_TQFindsC : tempstat = ch->pcdata->total_quest_finds;break;
		    case STAT_TQFindsA : tempstat = ch->pcdata->total_qf_attempted;break;
		    case STAT_TQHuntsC : tempstat = ch->pcdata->total_quest_hunts;break;
		    case STAT_TQHuntsA: tempstat = ch->pcdata->total_qh_attempted;break;
		    case STAT_EXTRA_DEATHS: tempstat = ch->pcdata->other_deaths;break;
		    case STAT_TREES_CHOPPED: tempstat = ch->pcdata->total_trees_chopped;break;
		    case STAT_TREES_MILLED: tempstat = ch->pcdata->total_trees_milled;break;
		    case STAT_OBJS_SOLD: tempstat = ch->pcdata->total_objs_sold;break;
		    case STAT_MONEY_MADE: tempstat = ch->pcdata->total_money_made;break;
		    case STAT_CG_WON: tempstat = ch->pcdata->total_cg_won;break;
		    case STAT_CG_LOST: tempstat = ch->pcdata->total_cg_lost;break;
		    }



		   if (i == STAT_FASTEST_TN)
		          continue;
		   

		  if ((i == STAT_FASTEST_TN && TGS_Stats[i][j] > tempstat) 
			  || (i != STAT_FASTEST_TN && tempstat > TGS_Stats[i][MAX_PEOPLE_STAT-1]))
		    {
		      for (j = 0;j < MAX_PEOPLE_STAT;j++)
			{
			  if (!started && !str_cmp(ch->name,TGS_Names[i][j]))
			  {
				  if ((i != STAT_FASTEST_TN && tempstat > TGS_Stats[i][j])
						  || (i == STAT_FASTEST_TN && tempstat < TGS_Stats[i][j]))
 				    TGS_Stats[i][j] = tempstat;
				  break;
			  }
			  if (started || (i == STAT_FASTEST_TN && TGS_Stats[i][j] > tempstat) 
				      || (i != STAT_FASTEST_TN && TGS_Stats[i][j] < tempstat))
			    {
			      // 		  if (TGS_Stats[i][j] < tempstat)
			      //		  {
			      //		  
			      if (!str_cmp(ch->name,TGS_Names[i][j]) && i+1 != MAX_PEOPLE_STAT)
			      {
				if (!started)		   
				{
					TGS_Stats[i][j] = tempstat;
					break;
				}
			        ttempstat = TGS_Stats[i][j];
                                TGS_Stats[i][j] = tempstat;
                                tempstat = ttempstat;
                                ttempname = TGS_Names[i][j];
                                TGS_Names[i][j] = tempname;
                                tempname = ttempname;
				//log_string("here");
				free_string(tempname);
				break;
			      }
				
			      started = TRUE;
			      ttempstat = TGS_Stats[i][j];
			      TGS_Stats[i][j] = tempstat;
			      tempstat = ttempstat;
			      ttempname = TGS_Names[i][j];
			      TGS_Names[i][j] = tempname;
			      tempname = ttempname;
			      //		  }
			    }
			  }
//		      log_string("there");
		       free_string(tempname);
		      
		    }
		}
	    }
	
		//Iblis 11/06/03 - One of the free_strings above fails in particular cases
		//(tries to free an invalid memory address).
		//I believe this occurs when someone moves up on the list, but others are
		//not bumped off..IE a person is not added to the list, they are just
		//rearranged.  I would have to think rather hard about which case this is to
		//get this to not occur, but free_string takes care of this for me, it just
		//prints an error message
		

	  save_statlist();
//      if(ch->Class == 8)
//         update_sword(ch, ch->sword);
	}
      return (found);
    }

}

CHAR_DATA *load_char_obj2 (char *name)
{

  DESCRIPTOR_DATA d;
  bool isChar;
/*  if (argument[0] == '\0')
    {
      send_to_char ("Load who?\n\r", ch);
      return;
    }*/
  //argument[0] = UPPER (argument[0]);
  //argument = one_argument (argument, name);

  // Dont want to load a second copy of a player who's already online!
  name[0] = UPPER (name[0]);
  is_linkloading = TRUE;
  isChar = load_char_obj (&d, name);    // char pfile exists?*
  is_linkloading = FALSE;
  if (!isChar)
    {
  //    sprintf (buffer, "That character name (%s) is unknown.\n\r", name);
   //   send_to_char (buffer, ch);
      return NULL;
    }
  d.character->desc = NULL;
  d.character->next = char_list;
  char_list = d.character;
  d.connected = CON_PLAYING;
  reset_char (d.character);
  // bring player to me
  if (d.character->in_room != NULL)
    {
      d.character->was_in_room = d.character->in_room;

      // char_from_room(d.character);
//      char_to_room (d.character, ch->in_room);  // put in room imm is in *
    }
  /*  act ("$n has created $N.", ch, NULL, d.character, TO_ROOM);
  sprintf (buffer, "Loading %s.\n\r", d.character->name);
  send_to_char (buffer, ch);*/
/*  if (d.character->pet != NULL)
    {
      char_to_room (d.character->pet, d.character->in_room);
      //      act ("$n has entered the game.", d.character->pet, NULL, NULL, TO_ROOM);
      }*/
  return d.character;
}




/*
 * Read in a char.
 */

void fread_char (CHAR_DATA * ch, FILE * fp)
{
  char buf[MAX_STRING_LENGTH * 10 ];
  const char *word;
  bool fMatch;
  int count = 0,i=0;
  int lastlogoff = current_time, booga;
  int percent;
  int tpclan;
  if (!is_linkloading)
  {
    sprintf (buf, "Loading %s.", ch->name);
    log_string (buf);
  }

  if (IS_NPC (ch))
    {
      bug ("fread_char : Attempted NPC read.", 0);
      return;
    }
  // first set their clan affiliation
  ch->clan = clan_login_player (ch);
  // all players start out *not* heros, only if they have a
  // hero flag in their file do they actually get to be heros
  ch->pcdata->hero = FALSE;

  for (;;)
    {
      word = feof(fp) ? "End" : fread_word(fp);
      fMatch = FALSE;

      switch (UPPER (word[0]))
	{
	case '*':
	  fMatch = TRUE;
	  fread_to_eol (fp);
	  break;

	case 'A':
	  KEY ("Act", ch->act, fread_flag (fp));

	  KEY ("Act2", ch->act2, fread_flag (fp));
	  KEY ("AALevl", ch->pcdata->autoassist_level, fread_number (fp));
	  KEY ("AEF", ch->pcdata->all_xp_familiar, fread_number (fp));
	  KEY ("AffectedBy", ch->affected_by, fread_flag (fp));
	  KEY ("AfBy", ch->affected_by, fread_flag (fp));
	  KEY ("Alignment", ch->alignment, fread_number (fp));
	  KEY ("Alig", ch->alignment, fread_number (fp));
          KEY ("ABKill", ch->pcdata->been_killed[ARENA_KILL], fread_number (fp));
          KEY ("AHKill", ch->pcdata->has_killed[ARENA_KILL], fread_number (fp));
	  KEY ("Avatar_Type", ch->pcdata->avatar_type, fread_number (fp));

	  if (!str_cmp (word, "Alia"))
	    {
	      if (count >= MAX_ALIAS)
		{
		  fread_to_eol (fp);
		  fMatch = TRUE;
		  break;
		}

	      ch->pcdata->alias[count] = str_dup (fread_word (fp));
	      ch->pcdata->alias_sub[count] = str_dup (fread_word (fp));
	      count++;
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "Alias"))
	    {
	      if (count >= MAX_ALIAS)
		{
		  fread_to_eol (fp);
		  fMatch = TRUE;
		  break;
		}

	      ch->pcdata->alias[count] = str_dup (fread_word (fp));
	      ch->pcdata->alias_sub[count] = fread_string (fp);
	      count++;
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "AC") || !str_cmp (word, "Armor"))
	    {
	      fread_to_eol (fp);
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "ACs"))
	    {
	      int i;

	      for (i = 0; i < 4; i++)
		ch->armor[i] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "AffD"))
	    {
	      AFFECT_DATA *paf;
	      int sn;

	      paf = new_affect ();

	      sn = exact_skill_lookup (fread_word (fp));
	      if (sn < 0)
		bug ("Fread_char: unknown skill.", 0);
	      else
		paf->type = sn;

	      paf->level = fread_number (fp);
	      paf->duration = fread_number (fp);
	      paf->modifier = fread_number (fp);
	      paf->location = fread_number (fp);
	      paf->bitvector = fread_number (fp);

	      paf->next = ch->affected;
	      ch->affected = paf;
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "Affc"))
	    {
	      AFFECT_DATA *paf;
	      int sn;

	      paf = new_affect ();

	      sn = exact_skill_lookup (fread_word (fp));
	      if (sn < 0)
		bug ("Fread_char: unknown skill.", 0);
	      else
		paf->type = sn;

	      paf->where = fread_number (fp);
	      paf->level = fread_number (fp);
	      paf->duration = fread_number (fp);
	      paf->modifier = fread_number (fp);
	      paf->location = fread_number (fp);
	      //Iblis - 10/03/04 This will make it so it takes a skill % affect by the skill NAME and not
              //the number (in case skills are rearranged)
//              if (paf->where == TO_SKILL && paf->location != 0)
//                paf->location = paf->type;
	      paf->bitvector = fread_number (fp);
	      paf->composition = fread_number (fp);
	      paf->comp_name = str_dup (fread_word (fp));

	      paf->next = ch->affected;
	      ch->affected = paf;
	      fMatch = TRUE;
	      break;
	    }


	  if (!str_cmp (word, "AttrMod") || !str_cmp (word, "AMod"))
	    {
	      int stat;
	      for (stat = 0; stat < MAX_STATS; stat++)
		ch->mod_stat[stat] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "AttrPerm") || !str_cmp (word, "Attr"))
	    {
	      int stat;

	      for (stat = 0; stat < MAX_STATS; stat++)
		ch->perm_stat[stat] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }
	  break;

	case 'B':
	  KEY ("Bamfin", ch->pcdata->bamfin, fread_string (fp));
	  KEY ("Bamfout", ch->pcdata->bamfout, fread_string (fp));
	  KEY ("Bcry", ch->pcdata->battlecry, fread_string (fp));
	  KEY ("Bin", ch->pcdata->bamfin, fread_string (fp));
	  KEY ("BldAll", ch->pcdata->buildall, fread_number (fp));
	  KEY ("Blok", ch->blocks_exit, fread_number (fp));
	  KEY ("Bout", ch->pcdata->bamfout, fread_string (fp));
	  KEY ("Bank_Gold", ch->pcdata->bank_gold, fread_number (fp));
	  KEY ("Bank_Silv", ch->pcdata->bank_silver, fread_number (fp));
	  KEY ("BBKill", ch->pcdata->been_killed[BR_KILL], fread_number (fp));
          KEY ("BHKill", ch->pcdata->has_killed[BR_KILL], fread_number (fp));
	  break;

	case 'C':
	  KEY ("Class", ch->Class, fread_number (fp));
	  KEY ("Class_Old", ch->pcdata->old_Class, fread_number(fp));
	  KEY ("Cla", ch->Class, fread_number (fp));
	  KEY ("Cla2", ch->Class2, fread_number (fp));
	  KEY ("Cla3", ch->Class3, fread_number (fp));
	  // We don't use this clan tag because the player may
	  // have been expelled while off line. The new clan code
	  // does the clan lookup at the top of this function but
	  // we still need to read and throw away the tag
	  //KEY("Clan", ch->clan, clan_lookup(fread_string(fp)));
	  KEY ("Clan", tpclan, clan_lookup (fread_string (fp)));

	  if (!str_cmp (word, "Condition") || !str_cmp (word, "Cond"))
	    {
	      ch->pcdata->condition[0] = fread_number (fp);
	      ch->pcdata->condition[1] = fread_number (fp);
	      ch->pcdata->condition[2] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }


	  if (!str_cmp (word, "Cmp"))
	    {
	      COMP_DATA *comp;

	      comp = new_comp ();

	      comp->name = str_dup (fread_word (fp));
	      comp->type = fread_number (fp);
	      comp->skill = fread_number (fp);
	      comp->title = str_dup (fread_word (fp));

	      comp->next = ch->compositions;
	      ch->compositions = comp;

	      fMatch = TRUE;
	      break;
	    }


	  if (!str_cmp (word, "Color"))
	    {
//   Yes, we know there's colors on Exodus, but why put loading of it in log
//	      log_string ("Reading colors");

	      ch->color = fread_number (fp);
	      ch->pcdata->color_combat_s = fread_number (fp);
	      ch->pcdata->color_combat_o = fread_number (fp);
	      ch->pcdata->color_combat_condition_s = fread_number (fp);
	      ch->pcdata->color_combat_condition_o = fread_number (fp);
	      ch->pcdata->color_invis = fread_number (fp);
	      ch->pcdata->color_hp = fread_number (fp);
	      ch->pcdata->color_hidden = fread_number (fp);
	      ch->pcdata->color_charmed = fread_number (fp);
	      ch->pcdata->color_mana = fread_number (fp);
	      ch->pcdata->color_move = fread_number (fp);
	      ch->pcdata->color_say = fread_number (fp);
	      ch->pcdata->color_tell = fread_number (fp);
	      ch->pcdata->color_guild_talk = fread_number (fp);
	      ch->pcdata->color_group_tell = fread_number (fp);
	      ch->pcdata->color_music = fread_number (fp);
	      ch->pcdata->color_auction = fread_number (fp);
	      ch->pcdata->color_gossip = fread_number (fp);
	      if (ch->version > 6)
		{
		  ch->pcdata->color_immtalk = fread_number (fp);
		  ch->pcdata->color_admtalk = fread_number (fp);
		}
	      fMatch = TRUE;
	      break;
	    }


	  if (!str_cmp (word, "Cnd"))
	    {
	      ch->pcdata->condition[0] = fread_number (fp);
	      ch->pcdata->condition[1] = fread_number (fp);
	      ch->pcdata->condition[2] = fread_number (fp);
	      ch->pcdata->condition[3] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "Cd"))
            {
	      ch->pcdata->condition[0] = fread_number (fp);
	      ch->pcdata->condition[1] = fread_number (fp);
	      ch->pcdata->condition[2] = fread_number (fp);
	      ch->pcdata->condition[3] = fread_number (fp);
	      ch->pcdata->condition[4] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }
	  
	  KEY ("Comm", ch->comm, fread_flag (fp));
	  KEY ("Com2", ch->comm2, fread_flag (fp));

	  break;

	case 'D':
	  KEY ("Damroll", ch->damroll, fread_number (fp));
	  KEY ("Dam", ch->damroll, fread_number (fp));
	  KEY ("Dam_Type", ch->dam_type, fread_number (fp));
	  KEY ("Description", ch->description, fread_string (fp));
	  KEY ("Desc", ch->description, fread_string (fp));
	  KEY ("Depty", ch->pcdata->deputy, fread_number (fp));
	  break;

	case 'E':
	  KEY ("Exorcise", ch->ex_ticks, fread_number (fp));
	  if (!str_cmp (word, "End"))
	    {
	      /* adjust hp mana move up  -- here for speed's sake */
	      percent = (current_time - lastlogoff) * 25 / (2 * 60 * 60);
	      percent = UMIN (percent, 100);

	      if (percent > 0 && !IS_AFFECTED (ch, AFF_POISON)
		  && !IS_AFFECTED (ch, AFF_PLAGUE))
		{
		  ch->hit += (ch->max_hit - ch->hit) * percent / 100;
		  ch->mana += (ch->max_mana - ch->mana) * percent / 100;
		  ch->move += (ch->max_move - ch->move) * percent / 100;
		}
	      // fix their loner status of clannie people
	      // while I'm at it, PCs who are clanned or lonered are always summonable
	      if (ch->clan != CLAN_BOGUS)
		{
		  ch->pcdata->loner = TRUE;
		  REMOVE_BIT (ch->act, PLR_NOSUMMON);
		}
	      else
		{
		  if (ch->pcdata->loner)
		    {
		      REMOVE_BIT (ch->act, PLR_NOSUMMON);
		    }
		}
	      
	      //Iblis 1/06/04 Set the proper values of modified skills
	      for (i = 0; i < MAX_SKILL; i++)
		{
		  if (skill_table[i].name != NULL
		      && ch->pcdata->mod_learned[i] == -999)
		    {
		      ch->pcdata->mod_learned[i] = 0;
		    }
		}

	      if (ch->pcdata->total_cg_lost < 0)
  	        ch->pcdata->total_cg_lost = 0 - ch->pcdata->total_cg_lost;

	      return;
	    }
	  KEY ("Exp", ch->exp, fread_number (fp));
	  KEY ("Email", ch->pcdata->email_addr, fread_string (fp));
	  KEY ("EWonBR", ch->pcdata->events_won[BR_WON], fread_number (fp));
	  KEY ("EWonSD", ch->pcdata->events_won[SD_WON], fread_number (fp));
	  KEY ("EInBR", ch->pcdata->events_won[BR_IN], fread_number (fp));
          KEY ("EInSD", ch->pcdata->events_won[SD_IN], fread_number (fp));
	  KEY ("EWonTSD", ch->pcdata->events_won[TEAM_SD_WON], fread_number (fp));
	  KEY ("EInTSD", ch->pcdata->events_won[TEAM_SD_IN], fread_number (fp));
	  break;

	case 'F':
	  KEY ("Fhost", ch->desc->host, fread_string (fp));
	  KEY ("FamT", ch->pcdata->familiar_type, fread_number (fp));
	  KEY ("FamL", ch->pcdata->familiar_level, fread_number (fp));
	  KEY ("FamMH", ch->pcdata->familiar_max_hit, fread_number (fp));
	  KEY ("FamMM", ch->pcdata->familiar_max_mana, fread_number (fp));
	  KEY ("FamMV", ch->pcdata->familiar_max_move, fread_number (fp));
	  KEY ("FamGH", ch->pcdata->familiar_gained_hp, fread_number (fp));
	  KEY ("FamGM", ch->pcdata->familiar_gained_mana, fread_number (fp));
	  KEY ("FamGMV", ch->pcdata->familiar_gained_move, fread_number (fp));
	  KEY ("FamE", ch->pcdata->familiar_exp, fread_number (fp));
	  KEY ("FamN", ch->pcdata->familiar_name, fread_string (fp));
	  KEY ("Falcon_Alert", ch->pcdata->falcon_alert, fread_number (fp));
	  break;

	case 'G':
	  KEY ("Gold", ch->gold, fread_number (fp));
	  KEY ("GamL", ch->pcdata->tg_lost, fread_number (fp));
	  KEY ("GamW", ch->pcdata->tg_won, fread_number (fp));
/*	  if (!str_cmp (word, "Group") || !str_cmp (word, "Gr"))
	    {
	      int gn;
	      char *temp;

	      temp = fread_word (fp);
	      gn = group_lookup (temp);
	      if (gn < 0)
		{
		  fprintf (stderr, "%s", temp);
		  bug ("Fread_char: unknown group. ", 0);
		}
	      else
		gn_add (ch, gn);
	      fMatch = TRUE;
	    }*/
	  if (!str_cmp (word, "GStats"))
	    {
	      int i, maxStat = 0;

	      maxStat = fread_number (fp);

	      for (i = 0; i < maxStat; i++)
		{
		  if (i != STAT_TOTAL_EXP)
		    fread_number (fp);
		  else
		    fread_number (fp);
		}
	      fMatch = TRUE;

	      break;
	    }
	  break;

	case 'H':
	  KEY ("Hand", ch->pcdata->primary_hand, fread_number (fp));
	  KEY ("HasR", ch->pcdata->has_reincarnated, fread_number(fp));
	  KEY ("Hitroll", ch->hitroll, fread_number (fp));
	  KEY ("Hit", ch->hitroll, fread_number (fp));
	  KEY ("Hero", ch->pcdata->hero, fread_number (fp));
	  if (!str_cmp (word, "HP_Gained"))
	    {
	      ch->pcdata->hp_gained = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }
	  if (!str_cmp (word, "HpManaMove") || !str_cmp (word, "HMV"))
	    {
	      ch->hit = fread_number (fp);
	      ch->max_hit = fread_number (fp);
	      ch->mana = fread_number (fp);
	      ch->max_mana = fread_number (fp);
	      ch->move = fread_number (fp);
	      ch->max_move = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "HpManaMovePerm") || !str_cmp (word, "HMVP"))
	    {
	      ch->pcdata->perm_hit = fread_number (fp);
	      ch->pcdata->perm_mana = fread_number (fp);
	      ch->pcdata->perm_move = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "Host"))
	    {
	      fread_to_eol (fp);
	      fMatch = TRUE;
	    }
	  break;

	case 'I':
  if (!str_cmp( word, "Ignore"))
    {
      if (count >= MAX_IGNORE)
        {
          fread_to_eol(fp);
          fMatch = TRUE;
          break;                 
         }

      ch->pcdata->ignore[count]        =      fread_string(fp);
      count++;
      fMatch = TRUE;
      break;
    }


	  KEY ("Id", ch->id, fread_number (fp));
	  KEY ("Illusion", ch->il_ticks, fread_number (fp));
	  KEY ("InvisLevel", ch->invis_level, fread_number (fp));
	  KEY ("Inco", ch->incog_level, fread_number (fp));
	  KEY ("Invi", ch->invis_level, fread_number (fp));
	  break;

	case 'L':
	  KEY ("LastLevel", ch->pcdata->last_level, fread_number (fp));
	  KEY ("Layhands", ch->lh_ticks, fread_number (fp));
	  KEY ("LLev", ch->pcdata->last_level, fread_number (fp));
	  KEY ("Level", ch->level, fread_number (fp));
	  KEY ("Lev", ch->level, fread_number (fp));
	  KEY ("Levl", ch->level, fread_number (fp));
	  KEY ("LogO", lastlogoff, fread_number (fp));
	  KEY ("LogF", booga, fread_number (fp));
	  KEY ("Loner", ch->pcdata->loner, fread_number (fp));
	  KEY ("LongDescr", ch->long_descr, fread_string (fp));
	  KEY ("LPray", ch->pcdata->lost_prayers, fread_number (fp));;
	  KEY ("LnD", ch->long_descr, fread_string (fp));
          KEY ("LTime", ch->loot_time, fread_number (fp));
	  break;

	case 'M':
	  KEY ("MBKill", ch->pcdata->been_killed[MOB_KILL],
	       fread_number (fp));
	  KEY ("MPC", ch->pcdata->pushed_mobs_counter,fread_number (fp));
	  KEY ("MHKill", ch->pcdata->has_killed[MOB_KILL], fread_number (fp));
	  if (!str_cmp (word, "Mana_Gained"))
	    {
	      ch->pcdata->mana_gained = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }
	  KEY ("Mask", ch->mask, fread_string (fp));
	  if (!str_cmp (word, "Move_Gained"))
	    {
	      ch->pcdata->move_gained = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }
	  if (!str_cmp (word, "MSk"))
            {
              int sn;
              int value;
              char *temp;

              value = fread_number (fp);
              temp = fread_word (fp);
              sn = exact_skill_lookup (temp);
              /* sn    = skill_lookup( fread_word( fp ) ); */
              if (sn < 0)
                {
                  fprintf (stderr, "%s", temp);
                  bug ("Fread_char: unknown skill. ", 0);
                }
              else
                ch->pcdata->mod_learned[sn] = value;
              fMatch = TRUE;
            }
	    break;
	case 'N':
	  KEY ("Name", ch->name, fread_string (fp));
	  KEY ("New_Style", ch->pcdata->new_style, fread_number (fp));
	  KEY ("NoBlinking", ch->pcdata->noblinking, fread_number (fp));
	  KEY ("NCTicks", ch->pcdata->nochan_ticks, fread_number (fp));
	  KEY ("Note", ch->pcdata->last_note, fread_number (fp));
	  KEY ("NAge", ch->pcdata->nage, fread_number (fp));
	  KEY ("NPlyd", ch->pcdata->nplayed, fread_number (fp));
	  if (!str_cmp (word, "Not"))
	    {
	      ch->pcdata->last_note = fread_number (fp);
	      ch->pcdata->last_idea = fread_number (fp);
	      ch->pcdata->last_penalty = fread_number (fp);
	      ch->pcdata->last_news = fread_number (fp);
	      ch->pcdata->last_changes = fread_number (fp);
	      if (ch->version >= PFILE_VER_NOTES)
		{
		  ch->pcdata->last_oocnote = fread_number (fp);
		  ch->pcdata->last_legend = fread_number (fp);
		}
	      if (ch->version >= PFILE_VER_PROJECTS)
		ch->pcdata->last_projects = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }
	  break;

	case 'O':
	/*   if (!str_cmp (word, "ODeaths"))
		             {
				                 ch->pcdata->other_deaths = fread_number (fp);
						             fMatch = TRUE;
							     do_echo(char_list,"HERE");
							               }*/
	   
	  KEY ("ODeaths",ch->pcdata->other_deaths, fread_number (fp));
	  KEY ("OOWTicks", ch->pcdata->oow_ticks, fread_number (fp));
	  break;

	case 'P':
	  KEY ("PBKill", ch->pcdata->been_killed[PLAYER_KILL],
	       fread_number (fp));
	  KEY ("PHKill", ch->pcdata->has_killed[PLAYER_KILL],
	       fread_number (fp));
	  KEY ("Password", ch->pcdata->pwd, fread_string (fp));
	  KEY ("Pass", ch->pcdata->pwd, fread_string (fp));
	  KEY ("Pers", ch->pcdata->personality, fread_number (fp));
	  KEY ("Plan", ch->pcdata->plan, fread_string (fp));
	  KEY ("Played", ch->played, fread_number (fp));
	  KEY ("Plyd", ch->played, fread_number (fp));
	  KEY ("Points", ch->pcdata->points, fread_number (fp));
	  KEY ("Pnts", ch->pcdata->points, fread_number (fp));
	  KEY ("PointsRp", ch->pcdata->rppoints, fread_number (fp));
	  KEY ("PointsNh", ch->pcdata->nhpoints, fread_number (fp));
	  KEY ("PointsQ", ch->pcdata->qpoints, fread_number (fp));
	  KEY ("Position", ch->position, fread_number (fp));
	  KEY ("Pos", ch->position, fread_number (fp));
	  //KEY ("Practice", ch->practice, fread_number (fp));
	  //KEY ("Prac", ch->practice, fread_number (fp));
	  KEY ("Pret", ch->pcdata->pretitle, fread_string (fp));
	  KEY ("Prevent_Escape", ch->pcdata->prevent_escape,
	       fread_number (fp));
	  KEY ("Prompt", ch->prompt, fread_string (fp));
	  KEY ("Prom", ch->prompt, fread_string (fp));
	  break;

	case 'Q':
	  KEY ("QTicks", ch->pcdata->quest_ticks, fread_number (fp));
	  break;

	case 'R':
	//  KEY ("Race", ch->race, race_lookup (fread_string (fp)));

	  if (!str_cmp (word, "Race_Old"))
	  {
	    ch->pcdata->old_race = fread_number (fp);
	    fMatch = TRUE;
	  }
	  
	  if (!str_cmp (word, "Race"))
	  {
	    char *useless;
	    useless = fread_string (fp);
	    if (!str_cmp (useless, "thyrent"))
	    {
		   ch->pcdata->has_reincarnated = TRUE;
		   ch->race = PC_RACE_LITAN;
		   
	    }
	    else ch->race = race_lookup(useless);
	    free_string(useless);
	    fMatch = TRUE;
	  }
	  KEY ("Referrer", ch->pcdata->referrer, fread_string (fp));
	  if (!str_cmp (word, "RMsg"))
	    {
	      ch->pcdata->restoremsg = fread_string (fp);
	      fMatch = TRUE;
	    }
	  if (!str_cmp (word, "Room"))
	    {
	      ch->in_room = get_room_index (fread_number (fp));

	      if (ch->in_room == NULL)
		ch->in_room = get_room_index (ROOM_VNUM_ALTAR);

	      if (ch->in_room == NULL)
		ch->in_room = get_room_index (ROOM_VNUM_LIMBO);
	      if (ch->in_room == get_room_index (ROOM_VNUM_SWITCHED_LIMBO))
  	        ch->in_room = get_room_index (ROOM_VNUM_ALTAR);

	      if (IS_SET (ch->in_room->room_flags, ROOM_TRANSPORT))
		{
		  OBJ_DATA *tObj = NULL;

		  for (tObj = object_list; tObj != NULL; tObj = tObj->next)
		    {
		      if (tObj->item_type != ITEM_CTRANSPORT)
			continue;

		      if (tObj->pIndexData->vnum ==
			  ch->in_room->ctransport_vnum)
			break;
		    }

		  if (tObj == NULL)
		    ch->in_room = get_room_index (ROOM_VNUM_ALTAR);
		  else
		    {
		      if (tObj->in_room == NULL)
			{
			  if (tObj->carried_by != NULL)
			    {
			      ch->in_room = tObj->carried_by->in_room;
			      if (ch->in_room == NULL)
				ch->in_room =
				  get_room_index (ROOM_VNUM_ALTAR);
			    }
			}
		    }
		}
	      fMatch = TRUE;
	      if (!IS_NPC (ch))
		ch->pcdata->last_saved_room = ch->in_room->vnum;
	      break;
	    }

	  break;

	case 'S':
	  KEY ("SavingThrow", ch->saving_throw, fread_number (fp));
	  KEY ("Save", ch->saving_throw, fread_number (fp));
	  KEY ("SBKill", ch->pcdata->been_killed[SD_KILL], fread_number (fp));
          KEY ("SHKill", ch->pcdata->has_killed[SD_KILL], fread_number (fp));
	  KEY ("Scro", ch->lines, fread_number (fp));
	  KEY ("Sex", ch->sex, fread_number (fp));
	  KEY ("ShortDescr", ch->short_descr, fread_string (fp));
	  KEY ("ShD", ch->short_descr, fread_string (fp));
	  /* OLC */ KEY ("Sec", ch->pcdata->security, fread_number (fp));
	  KEY ("Souls", ch->pcdata->souls, fread_number (fp));

	  KEY ("SUticks", ch->pcdata->shutup_ticks, fread_number (fp));

	  KEY ("Silv", ch->silver, fread_number (fp));


	  if (!str_cmp (word, "Skill") || !str_cmp (word, "Sk"))
	    {
	      int sn;
	      int value;
	      char *temp;

	      value = fread_number (fp);
	      temp = fread_word (fp);
	      sn = exact_skill_lookup (temp);
	      /* sn    = skill_lookup( fread_word( fp ) ); */
	      if (sn < 0)
		{
		  fprintf (stderr, "%s", temp);
		  bug ("Fread_char: unknown skill. ", 0);
		}
	      else
		ch->pcdata->learned[sn] = value;
	      fMatch = TRUE;
	    }

	  break;

	case 'T':
	  KEY ("Totalxp", ch->pcdata->totalxp, fread_number (fp));
	  KEY ("TotalQF", ch->pcdata->total_quest_finds, fread_number (fp));
	  KEY ("TotalQH", ch->pcdata->total_quest_hunts, fread_number (fp));
          KEY ("TotalCGW", ch->pcdata->total_cg_won, fread_number (fp));
          KEY ("TotalCGL", ch->pcdata->total_cg_lost, fread_number (fp));
	  KEY ("TQFAttempt", ch->pcdata->total_qf_attempted, fread_number (fp));
	  KEY ("TQHAttempt", ch->pcdata->total_qh_attempted, fread_number (fp));
	  KEY ("TCT", ch->pcdata->total_trees_chopped,fread_number (fp));
	  KEY ("TMT", ch->pcdata->total_trees_milled,fread_number (fp));
	  KEY ("TMM", ch->pcdata->total_money_made,fread_number (fp));
	  KEY ("TOS", ch->pcdata->total_objs_sold,fread_number (fp));
	  KEY ("TotalS", ch->pcdata->totalsouls, fread_number (fp));
	  KEY ("TrueSex", ch->pcdata->true_sex, fread_number (fp));
	  KEY ("TSex", ch->pcdata->true_sex, fread_number (fp));
	  KEY ("Trai", ch->train, fread_number (fp));
	  KEY ("Trust", ch->trust, fread_number (fp));
	  KEY ("Tru", ch->trust, fread_number (fp));

	  if (!str_cmp (word, "Title") || !str_cmp (word, "Titl"))
	    {
	      ch->pcdata->title = fread_string (fp);
	      if (ch->pcdata->title[0] != '.'
		  && ch->pcdata->title[0] != ','
		  && ch->pcdata->title[0] != '!'
		  && ch->pcdata->title[0] != '?')
		{
		  sprintf (buf, " %s", ch->pcdata->title);
		  free_string (ch->pcdata->title);
		  ch->pcdata->title = str_dup (buf);
		}
	      fMatch = TRUE;
	      break;
	    }

	  break;

	case 'V':
	  KEY ("Version", ch->version, fread_number (fp));
	  KEY ("Vers", ch->version, fread_number (fp));
	  if (!str_cmp (word, "Vnum"))
	    {
	      ch->pIndexData = get_mob_index (fread_number (fp));
	      fMatch = TRUE;
	      break;
	    }
	  break;

	case 'W':
	  KEY ("Wants", ch->times_wanted, fread_number (fp));
	  KEY ("Win", ch->pcdata->whoinfo, fread_string (fp));
	  KEY ("Wimpy", ch->wimpy, fread_number (fp));
	  KEY ("Wimp", ch->wimpy, fread_number (fp));
	  KEY ("Wizn", ch->wiznet, fread_flag (fp));
	  KEY ("WTime", ch->want_jail_time, fread_number (fp));
	  break;
	}


      if (!fMatch)
	{
	  char tcbuf[MAX_STRING_LENGTH];
	  sprintf (tcbuf, "Fread_char: no match (%s).", word);
	  bug (tcbuf, 0);
	  fread_to_eol (fp);
	}
    }

}

/* load a pet from the forgotten reaches */
void fread_pet (CHAR_DATA * ch, FILE * fp)
{
  const char *word;
  CHAR_DATA *pet;
  bool fMatch;
  int lastlogoff = current_time;
  int percent;
  int tpclan;
// return;
  /* first entry had BETTER be the vnum or we barf */
  word = feof (fp) ? "END" : fread_word (fp);
  if (!str_cmp (word, "Vnum"))
    {
      int vnum;

      vnum = fread_number (fp);
      if (get_mob_index (vnum) == NULL)
	{
	  bug ("Fread_pet: bad vnum %d.", vnum);
	  pet = create_mobile (get_mob_index (MOB_VNUM_FIDO));
	}
      else
	pet = create_mobile (get_mob_index (vnum));
    }
  else
    {
      bug ("Fread_pet: no vnum in file.", 0);
      pet = create_mobile (get_mob_index (MOB_VNUM_FIDO));
    }

  for (;;)
    {
      word = feof (fp) ? "END" : fread_word (fp);
      fMatch = FALSE;
      switch (UPPER (word[0]))
	{
	case '*':
	  fMatch = TRUE;
	  fread_to_eol (fp);
	  break;

	case 'A':
	  KEY ("Act", pet->act, fread_flag (fp));
	  KEY ("Act2", pet->act2, fread_flag (fp));
	  KEY ("AfBy", pet->affected_by, fread_flag (fp));
	  KEY ("Alig", pet->alignment, fread_number (fp));

	  if (!str_cmp (word, "ACs"))
	    {
	      int i;

	      for (i = 0; i < 4; i++)
		pet->armor[i] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "AffD"))
	    {
	      AFFECT_DATA *paf;
	      int sn;

	      paf = new_affect ();

	      sn = exact_skill_lookup (fread_word (fp));
	      if (sn < 0)
		bug ("Fread_char: unknown skill.", 0);
	      else
		paf->type = sn;

	      paf->level = fread_number (fp);
	      paf->duration = fread_number (fp);
	      paf->modifier = fread_number (fp);
	      paf->location = fread_number (fp);
	      paf->bitvector = fread_number (fp);
	      paf->next = pet->affected;
	      pet->affected = paf;
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "Affc"))
	    {
	      AFFECT_DATA *paf;
	      int sn;

	      paf = new_affect ();

	      sn = exact_skill_lookup (fread_word (fp));
	      if (sn < 0)
		bug ("Fread_char: unknown skill.", 0);
	      else
		paf->type = sn;

	      paf->where = fread_number (fp);
	      paf->level = fread_number (fp);
	      paf->duration = fread_number (fp);
	      paf->modifier = fread_number (fp);
	      paf->location = fread_number (fp);
	      //Iblis - 10/03/04 This will make it so it takes a skill % affect by the skill NAME and not
              //the number (in case skills are rearranged)
//              if (paf->where == TO_SKILL && paf->location != 0)
//                paf->location = paf->type;
	      paf->bitvector = fread_number (fp);
	      paf->composition = fread_number (fp);
	      paf->comp_name = str_dup (fread_word (fp));

	      paf->next = pet->affected;
	      pet->affected = paf;
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "AMod"))
	    {
	      int stat;

	      for (stat = 0; stat < MAX_STATS; stat++)
		pet->mod_stat[stat] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "Attr"))
	    {
	      int stat;

	      for (stat = 0; stat < MAX_STATS; stat++)
		pet->perm_stat[stat] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }
	  break;

	case 'B':
	  KEY ("Blok", pet->blocks_exit, fread_number (fp));
	  break;

	case 'C':
	  // basically throwing away the clan designation, 
	  // will get it from the pet's owner
	  // KEY("Clan", pet->clan, clan_lookup(fread_string(fp)));
	  KEY ("Clan", tpclan, clan_lookup (fread_string (fp)));
	  KEY ("Comm", pet->comm, fread_flag (fp));
	  KEY ("Com2", pet->comm2, fread_flag (fp));
	  break;

	case 'D':
	  KEY ("Dam", pet->damroll, fread_number (fp));
	  KEY ("Desc", pet->description, fread_string (fp));
          if (!str_cmp (word, "DDice"))
	  {
		  pet->damage[DICE_NUMBER] = fread_number(fp);
		  pet->damage[DICE_TYPE] = fread_number(fp);
	  }
	  break;

	case 'E':
	  if (!str_cmp (word, "End"))
	    {
	      pet->leader = ch;
	      pet->master = ch;
	      ch->pet = pet;
	      /* adjust hp mana move up  -- here for speed's sake */
	      percent = (current_time - lastlogoff) * 25 / (2 * 60 * 60);

	      if (percent > 0 && !IS_AFFECTED (ch, AFF_POISON)
		  && !IS_AFFECTED (ch, AFF_PLAGUE))
		{
		  percent = UMIN (percent, 100);
		  pet->hit += (pet->max_hit - pet->hit) * percent / 100;
		  pet->mana += (pet->max_mana - pet->mana) * percent / 100;
		  pet->move += (pet->max_move - pet->move) * percent / 100;
		}
	      return;
	    }
	  KEY ("Exp", pet->exp, fread_number (fp));
	  break;

	case 'G':
	  KEY ("Gold", pet->gold, fread_number (fp));
	  break;

	case 'H':
	  KEY ("Hit", pet->hitroll, fread_number (fp));

	  if (!str_cmp (word, "HMV"))
	    {
	      pet->hit = fread_number (fp);
	      pet->max_hit = fread_number (fp);
	      pet->mana = fread_number (fp);
	      pet->max_mana = fread_number (fp);
	      pet->move = fread_number (fp);
	      pet->max_move = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }
	  break;

	case 'L':
	  KEY ("Levl", pet->level, fread_number (fp));
	  KEY ("LnD", pet->long_descr, fread_string (fp));
	  KEY ("LogO", lastlogoff, fread_number (fp));
	  break;

	case 'N':
	  KEY ("Name", pet->name, fread_string (fp));
          KEY ("NumA", pet->number_of_attacks, fread_number (fp));
	  break;

	case 'P':
	  KEY ("Pos", pet->position, fread_number (fp));
	  break;

	case 'R':
	  KEY ("Race", pet->race, race_lookup (fread_string (fp)));
	  break;

	case 'S':
	  KEY ("Save", pet->saving_throw, fread_number (fp));
	  KEY ("Sex", pet->sex, fread_number (fp));
	  KEY ("ShD", pet->short_descr, fread_string (fp));
	  KEY ("Silv", pet->silver, fread_number (fp));
	  break;

	  if (!fMatch)
	    {
	      bug ("Fread_pet: no match.", 0);
	      fread_to_eol (fp);
	    }

	}
    }
}



void fread_obj (CHAR_DATA * ch, FILE * fp)
{
  OBJ_DATA *obj;
  const char *word;
  int iNest;
  bool fMatch;
  bool fNest;
  bool fVnum;
  bool first;
  bool make_new;		/* update object */

  fVnum = FALSE;
  obj = NULL;
  first = TRUE;			/* used to counter fp offset */
  make_new = FALSE;


  if (ch == NULL)
    return;

  word = feof (fp) ? "End" : fread_word (fp);
  if (!str_cmp (word, "Vnum"))
    {
      int vnum;
      first = FALSE;		/* fp will be in right place */

      vnum = fread_number (fp);
      if (get_obj_index (vnum) == NULL)
	{
	  bug ("Fread_obj: bad vnum %d.", vnum);
	}
      else
	{
	  obj = create_object (get_obj_index (vnum), -1);
	}

    }

  if (obj == NULL)
    {				/* either not found or old style */
      obj = new_obj ();
      obj->name = str_dup ("");
      obj->short_descr = str_dup ("");
      obj->description = str_dup ("");
      //Shinowlan 5/18/98 -- Part of the "spammed ExDe" fix
      obj->extra_descr = NULL;
    }

  fNest = FALSE;
  fVnum = TRUE;
  iNest = 0;

  for (;;)
    {
      if (first)
	first = FALSE;
      else
	word = feof (fp) ? "End" : fread_word (fp);
      fMatch = FALSE;

      switch (UPPER (word[0]))
	{
	case '*':
	  fMatch = TRUE;
	  fread_to_eol (fp);
	  break;

	case 'A':
	  if (!str_cmp (word, "AffD"))
	    {
	      AFFECT_DATA *paf;
	      int sn;

	      paf = new_affect ();

	      sn = exact_skill_lookup (fread_word (fp));
	      if (sn < 0)
		bug ("Fread_obj: unknown skill.", 0);
	      else
		paf->type = sn;

	      paf->level = fread_number (fp);
	      paf->duration = fread_number (fp);
	      paf->modifier = fread_number (fp);
	      paf->location = fread_number (fp);
	      paf->bitvector = fread_number (fp);
	      paf->next = obj->affected;
	      obj->affected = paf;
	      fMatch = TRUE;
	      break;
	    }
	  if (!str_cmp (word, "Affc"))
	    {
	      AFFECT_DATA *paf;
	      int sn;

	      paf = new_affect ();

	      sn = exact_skill_lookup (fread_word (fp));
	      if (sn < 0)
		bug ("Fread_obj: unknown skill.", 0);
	      else
		paf->type = sn;

	      paf->where = fread_number (fp);
	      paf->level = fread_number (fp);
	      paf->duration = fread_number (fp);
	      paf->modifier = fread_number (fp);
	      paf->location = fread_number (fp);
	      //This will make it so it takes a skill % affect by the skill NAME and not
	      //the number (in case skills are rearranged)
//	      if (paf->where == TO_SKILL && paf->location != 0)
// 	        paf->location = paf->type;
	      paf->bitvector = fread_number (fp);
	      paf->composition = fread_number (fp);
	      paf->comp_name = str_dup (fread_word (fp));

	      paf->next = obj->affected;
	      obj->affected = paf;
	      fMatch = TRUE;
	      break;
	    }
	  break;

	case 'C':
	  // Iblis 6/12/04 - Done so if a new variable is added to store this information
	  // these lines should need minimal modification
	  KEY ("Cardslot", obj->bs_capacity[0], fread_number (fp));
	  // Akamai 4/30/99 - support Class/race specific objects
	  KEY ("ClanFlags", obj->clan_flags, fread_flag (fp));
	  KEY ("ClanF", obj->clan_flags, fread_flag (fp));
	  KEY ("ClassFlags", obj->Class_flags, fread_flag (fp));
	  KEY ("ClassF", obj->Class_flags, fread_flag (fp));
	  KEY ("Cond", obj->condition, fread_number (fp));
	  KEY ("Cost", obj->cost, fread_number (fp));
	  break;

	case 'D':
	  KEY ("Description", obj->description, fread_string (fp));
	  KEY ("Desc", obj->description, fread_string (fp));
	  break;

	case 'E':

	  if (!str_cmp (word, "Enchanted"))
	    {
	      obj->enchanted = TRUE;
	      fMatch = TRUE;
	      break;
	    }

	  KEY ("ExtraFlags", obj->extra_flags[0], fread_number (fp));
	  KEY ("ExtF", obj->extra_flags[0], fread_number (fp));
	  if (!str_cmp(word,"ExF"))
	  {
		  int ai;
		  ai = fread_number(fp);
		  if (ai > MAX_EXTRA_FLAGS)
		  {
			  bug("Extra Flag Index out of range!",0);
			  break;
		  }
		  obj->extra_flags[ai] = fread_number (fp);
		  fMatch = TRUE;
	  }

	  if (!str_cmp (word, "ExtraDescr") || !str_cmp (word, "ExDe"))
	    {
	      EXTRA_DESCR_DATA *ed, *cur;

	      ed = new_extra_descr ();

	      ed->keyword = fread_string (fp);
	      ed->description = fread_string (fp);
	      //Shinowlan 5/18/98 -- Loop to eliminate duplicate ExDe's
	      cur = obj->extra_descr;
	      while ((cur != NULL) && str_cmp (cur->keyword, ed->keyword))
		{
		  cur = cur->next;
		}
	      //Shinowlan 5/18/98 -- only add keyword if it's not a dupe 
	      if (cur == NULL)
		{
		  ed->next = obj->extra_descr;
		  obj->extra_descr = ed;
		}
	      else
		{
		  free_extra_descr (ed);
		}
	      fMatch = TRUE;
	    }

	  if (!str_cmp (word, "End"))
	    {
	      if (!fNest || !fVnum || obj->pIndexData == NULL)
		{
		  bug ("Fread_obj: incomplete object.", 0);
		  free_obj (obj);
		  return;
		}
	      else
		{
		  if (make_new)
		    {
		      int wear;

		      wear = obj->wear_loc;
		      extract_obj (obj);

		      obj = create_object (obj->pIndexData, 0);
		      obj->wear_loc = wear;
		      obj->condition = 100;
		    }
		  if (obj->condition == 0)
		    obj->condition = 100;
		  if (iNest == 0 || rgObjNest[iNest] == NULL)
		    obj_to_char (obj, ch);
		  else
		    obj_to_obj (obj, rgObjNest[iNest - 1]);
		  return;
		}
	    }
	  break;

	case 'I':
	  KEY ("ItemType", obj->item_type, fread_number (fp));
	  KEY ("Ityp", obj->item_type, fread_number (fp));
	  break;

	case 'L':
	  KEY ("Level", obj->level, fread_number (fp));
	  KEY ("Lev", obj->level, fread_number (fp));
	  break;

	case 'N':
	  KEY ("Name", obj->name, fread_string (fp));

	  if (!str_cmp (word, "Nest"))
	    {
	      iNest = fread_number (fp);
	      if (iNest < 0 || iNest >= MAX_NEST)
		{
		  bug ("Fread_obj: bad nest %d.", iNest);
		}
	      else
		{
		  rgObjNest[iNest] = obj;
		  fNest = TRUE;
		}
	      fMatch = TRUE;
	    }
	  break;

	case 'O':
	  KEY ("OwnR", obj->owner, fread_string (fp));

	  if (!str_cmp (word, "Oldstyle"))
	    {
	      if (obj->pIndexData != NULL)
		make_new = TRUE;
	      fMatch = TRUE;
	    }
	  if (!str_cmp (word, "OTVnum2"))
	    {
 	      int tempi,i;
              for (i=0;i<MAX_OBJ_TRIGS;i++)
	      {
		tempi = fread_number(fp);
		if (obj->objtrig[i])
  	  	  free_ot(obj->objtrig[i]);
		if (tempi != 0)
		{
		  obj->objtrig[i] = new_ot(tempi);
		  if (obj->objtrig[i])
		    obj->objtrig[i]->obj_on = obj;
		}
	      }
	       fMatch = TRUE;
	    }
	  if (!str_cmp (word, "OTVnum"))
	  {
	    if (obj->objtrig[0] == NULL)
	    {
	      int tempi = fread_number (fp), i=0;
	      obj->objtrig[0] = new_ot(tempi);
	      if (obj->objtrig[0])
	        obj->objtrig[0]->obj_on = obj;
	      for (i=1;i<MAX_OBJ_TRIGS;i++)
	        obj->objtrig[i] = NULL;
            }
            else
            {
              int tempi = fread_number (fp),i=0;
              free_ot(obj->objtrig[0]);
              obj->objtrig[0] = new_ot(tempi);
	      if (obj->objtrig[0])
                obj->objtrig[0]->obj_on = obj;
	      for (i=1;i<MAX_OBJ_TRIGS;i++)
                obj->objtrig[i] = NULL;
            }
	     fMatch = TRUE;
          }
	  
	  break;

	case 'P':
	  // Adeon 6/29/03 -- player owner flags...
	  KEY ("POwn", obj->plr_owner, fread_string (fp));
	  break;


	case 'R':
	  // Akamai 4/30/99 - support for Class/race specific equipment
	  KEY ("RaceFlags", obj->race_flags, fread_flag (fp));
	  KEY ("RaceF", obj->race_flags, fread_flag (fp));
	  break;


	case 'S':
	  KEY ("ShortDescr", obj->short_descr, fread_string (fp));
	  KEY ("ShD", obj->short_descr, fread_string (fp));

	  if (!str_cmp (word, "Spell"))
	    {
	      int iValue;
	      int sn;

	      iValue = fread_number (fp);
	      sn = exact_skill_lookup (fread_word (fp));
	      if (iValue < 0 || iValue > 5)
		{
		  bug ("Fread_obj: bad iValue %d.", iValue);
		}
	      else if (sn < 0)
		{
		  bug ("Fread_obj: unknown skill.", 0);
		}
	      else
		{
		  obj->value[iValue] = sn;
		}
	      fMatch = TRUE;
	      break;
	    }

	  break;

	case 'T':
	  KEY ("Timer", obj->timer, fread_number (fp));
	  KEY ("Time", obj->timer, fread_number (fp));
	  break;

	case 'V':
	  if (!str_cmp (word, "Values") || !str_cmp (word, "Vals"))
	    {
	      obj->value[0] = fread_number (fp);
	      obj->value[1] = fread_number (fp);
	      obj->value[2] = fread_number (fp);
	      obj->value[3] = fread_number (fp);
	      if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
		obj->value[0] = obj->pIndexData->value[0];
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "Val"))
	    {
	      obj->value[0] = fread_number (fp);
	      obj->value[1] = fread_number (fp);
	      obj->value[2] = fread_number (fp);
	      obj->value[3] = fread_number (fp);
	      obj->value[4] = fread_number (fp);
	      obj->value[5] = fread_number (fp);
	      obj->value[6] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "Vls"))
            {
              obj->value[0] = fread_number (fp);
              obj->value[1] = fread_number (fp);
              obj->value[2] = fread_number (fp);
              obj->value[3] = fread_number (fp);
              obj->value[4] = fread_number (fp);
              obj->value[5] = fread_number (fp);
              obj->value[6] = fread_number (fp);
	      obj->value[7] = fread_number (fp);
	      obj->value[8] = fread_number (fp);
	      obj->value[9] = fread_number (fp);
	      obj->value[10] = fread_number (fp);
//	      obj->value[11] = fread_number (fp);
//	      obj->value[12] = fread_number (fp);
              fMatch = TRUE;
              break;
            }

	  if (!str_cmp (word, "Vlues"))
            {
              obj->value[0] = fread_number (fp);
              obj->value[1] = fread_number (fp);
              obj->value[2] = fread_number (fp);
              obj->value[3] = fread_number (fp);
              obj->value[4] = fread_number (fp);
              obj->value[5] = fread_number (fp);
              obj->value[6] = fread_number (fp);
              obj->value[7] = fread_number (fp);
              obj->value[8] = fread_number (fp);
              obj->value[9] = fread_number (fp);
              obj->value[10] = fread_number (fp);
              obj->value[11] = fread_number (fp);
              obj->value[12] = fread_number (fp);
              fMatch = TRUE;
              break;
            }


	  if (!str_cmp (word, "Vnum"))
	    {
	      int vnum;

	      vnum = fread_number (fp);
	      if ((obj->pIndexData = get_obj_index (vnum)) == NULL)
		bug ("Fread_obj: bad vnum %d.", vnum);
	      else
		fVnum = TRUE;
	      fMatch = TRUE;
	      break;
	    }
	  break;

	case 'W':
	  KEY ("WearFlags", obj->wear_flags, fread_number (fp));
	  KEY ("WeaF", obj->wear_flags, fread_number (fp));
	  KEY ("WearLoc", obj->wear_loc, fread_number (fp));
	  KEY ("WearL", obj->wear_loc, fread_number (fp));
	  KEY ("Weight", obj->weight, fread_number (fp));
	  KEY ("Wt", obj->weight, fread_number (fp));
	  if (!str_cmp (word, "Wear"))
	  {
		  int i=0;
		  int wearloc = fread_number(fp);
		  if (wearloc == -1)
                    obj->wear_loc = -1;
		  else 
		  {
		    for (i=0;wear_conversion_table[i].wear_loc != -2;++i)
		    {
			  if (wear_conversion_table[i].wear_loc == wearloc)
			  {
				  obj->wear_loc = wear_conversion_table[i].wear_flag;
				  break;
			  }
		    }
		  }
		  fMatch = TRUE;
		  break;
	  }
	  break;

	}

      if (!fMatch)
	{
	  bug ("Fread_obj: no match.", 0);
	  fread_to_eol (fp);
	}
    }
}


void fread_objdb (OBJ_DATA * pObj, FILE * fp)
{
  OBJ_DATA *obj;
  const char *word;
  int iNest;
  bool fMatch;
  bool fNest;
  bool fVnum;
  bool first;
  bool make_new;		/* update object */

  fVnum = FALSE;
  obj = NULL;
  first = TRUE;			/* used to counter fp offset */
  make_new = FALSE;


  word = feof (fp) ? "End" : fread_word (fp);
  if (!str_cmp (word, "Vnum"))
    {
      int vnum;
      first = FALSE;		/* fp will be in right place */

      vnum = fread_number (fp);
      if (get_obj_index (vnum) == NULL)
	{
	  bug ("fread_objdb() - bad vnum %d.", vnum);
	}
      else
	{
	  obj = create_object (get_obj_index (vnum), -1);
	}

    }

  if (obj == NULL)
    {				/* either not found or old style */
      obj = new_obj ();
      obj->name = str_dup ("");
      obj->short_descr = str_dup ("");
      obj->description = str_dup ("");
    }

  fNest = FALSE;
  fVnum = TRUE;
  iNest = 0;

  for (;;)
    {
      if (first)
	first = FALSE;
      else
	word = feof (fp) ? "End" : fread_word (fp);
      fMatch = FALSE;

      switch (UPPER (word[0]))
	{
	case '*':
	  fMatch = TRUE;
	  fread_to_eol (fp);
	  break;

	case 'A':
	  if (!str_cmp (word, "AffD"))
	    {
	      AFFECT_DATA *paf;
	      int sn;

	      paf = new_affect ();

	      sn = exact_skill_lookup (fread_word (fp));
	      if (sn < 0)
		bug ("fread_objdb() - unknown skill.", 0);
	      else
		paf->type = sn;

	      paf->level = fread_number (fp);
	      paf->duration = fread_number (fp);
	      paf->modifier = fread_number (fp);
	      paf->location = fread_number (fp);
	      paf->bitvector = fread_number (fp);
	      paf->next = obj->affected;
	      obj->affected = paf;
	      fMatch = TRUE;
	      break;
	    }
	  if (!str_cmp (word, "Affc"))
	    {
	      AFFECT_DATA *paf;
	      int sn;

	      paf = new_affect ();

	      sn = exact_skill_lookup (fread_word (fp));
	      if (sn < 0)
		bug ("fread_objdb() - unknown skill.", 0);
	      else
		paf->type = sn;

	      paf->where = fread_number (fp);
	      paf->level = fread_number (fp);
	      paf->duration = fread_number (fp);
	      paf->modifier = fread_number (fp);
	      paf->location = fread_number (fp);
              //This will make it so it takes a skill % affect by the skill NAME and not
              //the number (in case skills are rearranged)
//	      if (paf->where == TO_SKILL && paf->location != 0)
//	        paf->location = paf->type;
	      paf->bitvector = fread_number (fp);
	      paf->composition = fread_number (fp);
	      paf->comp_name = str_dup (fread_word (fp));

	      paf->next = obj->affected;
	      obj->affected = paf;
	      fMatch = TRUE;
	      break;
	    }
	  break;

	case 'C':
	  // Akamai 4/30/99 - support for Class/race specific eq
	  KEY ("Cardslot", obj->bs_capacity[0], fread_number (fp));
	  KEY ("ClanFlags", obj->clan_flags, fread_flag (fp));
	  KEY ("ClanF", obj->clan_flags, fread_flag (fp));
	  KEY ("ClassFlags", obj->Class_flags, fread_flag (fp));
	  KEY ("ClassF", obj->Class_flags, fread_flag (fp));
	  KEY ("Cond", obj->condition, fread_number (fp));
	  KEY ("Cost", obj->cost, fread_number (fp));
/* ITEM_CLAN_DONATION *//* new_clan */
	  if (!str_cmp (word, "Clan"))
	    {
	      {
		int slot;
		slot = clanname_to_slot (fread_string (fp));
		if (slot == CLAN_BOGUS)
		  {
		    bug
		      ("fread_objdb() - bogus clan identifier on a clan box or object.",
		       0);
		  }
		obj->value[1] = slot;
	      }
	    }
	  break;

	case 'D':
	  KEY ("Description", obj->description, fread_string (fp));
	  KEY ("Desc", obj->description, fread_string (fp));
	  break;

	case 'E':

	  if (!str_cmp (word, "Enchanted"))
	    {
	      obj->enchanted = TRUE;
	      fMatch = TRUE;
	      break;
	    }

	  KEY ("ExtraFlags", obj->extra_flags[0], fread_number (fp));
	  KEY ("ExtF", obj->extra_flags[0], fread_number (fp));
	  if (!str_cmp(word,"ExF"))
	  {
		  int ai;
		  ai = fread_number(fp);
		  if (ai > MAX_EXTRA_FLAGS)
		  {
			  bug("Extra Flag Index out of range!",0);
			  break;
		  }
		  obj->extra_flags[ai] = fread_number (fp);
		  fMatch = TRUE;
	  }

	  if (!str_cmp (word, "ExtraDescr") || !str_cmp (word, "ExDe"))
	    {
	      EXTRA_DESCR_DATA *ed;

	      ed = new_extra_descr ();

	      ed->keyword = fread_string (fp);
	      ed->description = fread_string (fp);
	      ed->next = obj->extra_descr;
	      obj->extra_descr = ed;
	      fMatch = TRUE;
	    }

	  if (!str_cmp (word, "End"))
	    {
	      if (!fNest || !fVnum || obj->pIndexData == NULL)
		{
		  bug ("fread_objdb() - incomplete object.", 0);
		  free_obj (obj);
		  return;
		}
	      else
		{
		  if (make_new)
		    {
		      int wear;
		      obj->condition = 100;
		      wear = obj->wear_loc;
		      extract_obj (obj);

		      obj = create_object (obj->pIndexData, 0);
		      obj->wear_loc = wear;
		    }
		  if (obj->condition == 0)
		    obj->condition = 100;
		  if (iNest == 0 || rgObjNest[iNest] == NULL)
		    obj_to_obj (obj, pObj);
		  else
		    obj_to_obj (obj, rgObjNest[iNest - 1]);
		  return;
		}
	    }
	  break;

	case 'I':
	  KEY ("ItemType", obj->item_type, fread_number (fp));
	  KEY ("Ityp", obj->item_type, fread_number (fp));
	  break;

	case 'L':
	  KEY ("Level", obj->level, fread_number (fp));
	  KEY ("Lev", obj->level, fread_number (fp));
	  break;

	case 'N':
	  KEY ("Name", obj->name, fread_string (fp));

	  if (!str_cmp (word, "Nest"))
	    {
	      iNest = fread_number (fp);
	      if (iNest < 0 || iNest >= MAX_NEST)
		{
		  bug ("fread_objdb() - bad nest %d.", iNest);
		}
	      else
		{
		  rgObjNest[iNest] = obj;
		  fNest = TRUE;
		}
	      fMatch = TRUE;
	    }
	  break;

	case 'O':
	  KEY ("OwnR", obj->owner, fread_string (fp));

	  if (!str_cmp (word, "Oldstyle"))
	    {
	      if (obj->pIndexData != NULL)
		make_new = TRUE;
	      fMatch = TRUE;
	    }

	  if (!str_cmp (word, "OTVnum2"))
	  {
	     int tempi,i;
             for (i=0;i<MAX_OBJ_TRIGS;i++)
             {
               tempi = fread_number(fp);
               if (obj->objtrig[i])
                 free_ot(obj->objtrig[i]);
               if (tempi != 0)
               {
                 obj->objtrig[i] = new_ot(tempi);
		 if (obj->objtrig[i])
                   obj->objtrig[i]->obj_on = obj;
               }
             }
	      fMatch = TRUE;
	  }

	  if (!str_cmp (word, "OTVnum"))
	  {
	    if (obj->objtrig[0] == NULL)
	    {
	      int tempi = fread_number (fp),i=0;
	      obj->objtrig[0] = new_ot(tempi);
	      if (obj->objtrig[0])
	        obj->objtrig[0]->obj_on = obj;
	      for (i=1;i<MAX_OBJ_TRIGS;i++)
		obj->objtrig[i] = NULL;
            }
            else
	    {
	      int tempi = fread_number (fp),i=0;
	      free_ot(obj->objtrig[0]);
	      obj->objtrig[0] = new_ot(tempi);
	      if (obj->objtrig[0])
 	        obj->objtrig[0]->obj_on = obj;
	      for (i=1;i<MAX_OBJ_TRIGS;i++)
                 obj->objtrig[i] = NULL;	      
	    }
	     fMatch = TRUE;
	  }
	  
 
	  break;

	case 'P':
	  // Adeon 6/29/03 -- player owner flags...
	  KEY ("POwn", obj->plr_owner, fread_string (fp));
	  break;

	case 'R':
	  // Akamai 4/30/99 - support for Class/race specific eq
	  KEY ("RaceFlags", obj->race_flags, fread_flag (fp));
	  KEY ("RaceF", obj->race_flags, fread_flag (fp));
	  break;

	case 'S':
	  KEY ("ShortDescr", obj->short_descr, fread_string (fp));
	  KEY ("ShD", obj->short_descr, fread_string (fp));

	  if (!str_cmp (word, "Spell"))
	    {
	      int iValue;
	      int sn;

	      iValue = fread_number (fp);
	      sn = exact_skill_lookup (fread_word (fp));
	      if (iValue < 0 || iValue > 5)
		{
		  bug ("fread_objdb() - bad iValue %d.", iValue);
		}
	      else if (sn < 0)
		{
		  bug ("fread_objdb() - unknown skill.", 0);
		}
	      else
		{
		  obj->value[iValue] = sn;
		}
	      fMatch = TRUE;
	      break;
	    }

	  break;

	case 'T':
	  KEY ("Timer", obj->timer, fread_number (fp));
	  KEY ("Time", obj->timer, fread_number (fp));
	  break;

	case 'V':
	  if (!str_cmp (word, "Values") || !str_cmp (word, "Vals"))
	    {
	      obj->value[0] = fread_number (fp);
	      obj->value[1] = fread_number (fp);
	      obj->value[2] = fread_number (fp);
	      obj->value[3] = fread_number (fp);
	      if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
		obj->value[0] = obj->pIndexData->value[0];
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "Val"))
	    {
	      obj->value[0] = fread_number (fp);
	      obj->value[1] = fread_number (fp);
	      obj->value[2] = fread_number (fp);
	      obj->value[3] = fread_number (fp);
	      obj->value[4] = fread_number (fp);
	      obj->value[5] = fread_number (fp);
	      obj->value[6] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "Vls"))
	    {
	      obj->value[0] = fread_number (fp);
	      obj->value[1] = fread_number (fp);
	      obj->value[2] = fread_number (fp);
	      obj->value[3] = fread_number (fp);
	      obj->value[4] = fread_number (fp);
	      obj->value[5] = fread_number (fp);
	      obj->value[6] = fread_number (fp);
	      obj->value[7] = fread_number (fp);
	      obj->value[8] = fread_number (fp);
	      obj->value[9] = fread_number (fp);
	      obj->value[10] = fread_number (fp);
//	      obj->value[11] = fread_number (fp);
//	      obj->value[12] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "Vlues"))
            {
              obj->value[0] = fread_number (fp);
              obj->value[1] = fread_number (fp);
              obj->value[2] = fread_number (fp);
              obj->value[3] = fread_number (fp);
              obj->value[4] = fread_number (fp);
              obj->value[5] = fread_number (fp);
              obj->value[6] = fread_number (fp);
              obj->value[7] = fread_number (fp);
              obj->value[8] = fread_number (fp);
              obj->value[9] = fread_number (fp);
              obj->value[10] = fread_number (fp);
              obj->value[11] = fread_number (fp);
              obj->value[12] = fread_number (fp);
              fMatch = TRUE;
              break;
            }

	  

	  if (!str_cmp (word, "Vnum"))
	    {
	      int vnum;

	      vnum = fread_number (fp);
	      if ((obj->pIndexData = get_obj_index (vnum)) == NULL)
		bug ("fread_objdb() - bad vnum %d.", vnum);
	      else
		fVnum = TRUE;
	      fMatch = TRUE;
	      break;
	    }
	  break;

	case 'W':
	  KEY ("WearFlags", obj->wear_flags, fread_number (fp));
	  KEY ("WeaF", obj->wear_flags, fread_number (fp));
	  KEY ("WearLoc", obj->wear_loc, fread_number (fp));
	  KEY ("WearL", obj->wear_loc, fread_number (fp));
	  KEY ("Weight", obj->weight, fread_number (fp));
	  KEY ("Wt", obj->weight, fread_number (fp));
	  if (!str_cmp (word, "Wear"))
	  {
		  int i=0;
		  int wearloc = fread_number(fp);
		  if (wearloc == -1)
			  obj->wear_loc = -1;
		  else
		  {
		    for (i=0;wear_conversion_table[i].wear_loc != -2;++i)
		    {
			  if (wear_conversion_table[i].wear_loc == wearloc)
			  {
				  obj->wear_loc = wear_conversion_table[i].wear_flag;
				  break;
			  }
		    }
		  }
		  fMatch = TRUE;
		  break;
	  }
	  break;

	}

      if (!fMatch)
	{
	  bug ("fread_objdb() - no match. - vnum %d", obj->pIndexData->vnum);
	  bug (word,0);
	  fread_to_eol (fp);
	}
    }
}


void fread_mount (CHAR_DATA * ch, FILE * fp)
{
  const char *word;
  CHAR_DATA *mount;
  bool fMatch;
  int lastlogoff = current_time;
  int percent;
  int tpclan;
//return;
  if (IS_NPC (ch))
    {
      bug ("fread_mount : An NPC attempted this.", 0);
      return;
    }

  /* first entry had BETTER be the vnum or we barf */
  word = feof (fp) ? "END" : fread_word (fp);
  if (!str_cmp (word, "Vnum"))
    {
      int vnum;

      vnum = fread_number (fp);
      if (get_mob_index (vnum) == NULL)
	{
	  bug ("Fread_mount: bad vnum %d.", vnum);
	  mount = create_mobile (get_mob_index (MOB_VNUM_FIDO));
	}
      else
	mount = create_mobile (get_mob_index (vnum));
    }
  else
    {
      bug ("Fread_mount: no vnum in file.", 0);
      mount = create_mobile (get_mob_index (MOB_VNUM_FIDO));
    }


  for (;;)
    {
      word = feof (fp) ? "END" : fread_word (fp);
      fMatch = FALSE;

      switch (UPPER (word[0]))
	{
	case '*':
	  fMatch = TRUE;
	  fread_to_eol (fp);
	  break;

	case 'A':
	  KEY ("Act", mount->act, fread_flag (fp));
	  KEY ("Act2", mount->act2, fread_flag (fp));
	  KEY ("AfBy", mount->affected_by, fread_flag (fp));
	  KEY ("Alig", mount->alignment, fread_number (fp));

	  if (!str_cmp (word, "ACs"))
	    {
	      int i;

	      for (i = 0; i < 4; i++)
		mount->armor[i] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "AffD"))
	    {
	      AFFECT_DATA *paf;
	      int sn;

	      paf = new_affect ();

	      sn = exact_skill_lookup (fread_word (fp));
	      if (sn < 0)
		bug ("Fread_char: unknown skill.", 0);
	      else
		paf->type = sn;

	      paf->level = fread_number (fp);
	      paf->duration = fread_number (fp);
	      paf->modifier = fread_number (fp);
	      paf->location = fread_number (fp);
	      paf->bitvector = fread_number (fp);
	      paf->next = mount->affected;
	      mount->affected = paf;
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "Affc"))
	    {
	      AFFECT_DATA *paf;
	      int sn;

	      paf = new_affect ();

	      sn = exact_skill_lookup (fread_word (fp));
	      if (sn < 0)
		bug ("Fread_char: unknown skill.", 0);
	      else
		paf->type = sn;

	      paf->where = fread_number (fp);
	      paf->level = fread_number (fp);
	      paf->duration = fread_number (fp);
	      paf->modifier = fread_number (fp);
	      paf->location = fread_number (fp);
              //Iblis - 10/03/04 This will make it so it takes a skill % affect by the skill NAME and not
              //the number (in case skills are rearranged)
//              if (paf->where == TO_SKILL && paf->location != 0)
//                paf->location = paf->type;
	      paf->bitvector = fread_number (fp);
	      paf->composition = fread_number (fp);
	      paf->comp_name = str_dup (fread_word (fp));

	      paf->next = mount->affected;
	      mount->affected = paf;
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "AMod"))
	    {
	      int stat;

	      for (stat = 0; stat < MAX_STATS; stat++)
		mount->mod_stat[stat] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }

	  if (!str_cmp (word, "Attr"))
	    {
	      int stat;

	      for (stat = 0; stat < MAX_STATS; stat++)
		mount->perm_stat[stat] = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }
	  break;
	case 'B':
	  KEY ("Blok", mount->blocks_exit, fread_number (fp));
	  break;

	case 'C':
// again, throwing away the clan designation, getting it from the owner
//        KEY("Clan", mount->clan, clan_lookup(fread_string(fp)));
	  KEY ("Clan", tpclan, clan_lookup (fread_string (fp)));
	  KEY ("Comm", mount->comm, fread_flag (fp));
	  KEY ("Com2", mount->comm2, fread_flag (fp));
	  break;

	case 'D':
	  KEY ("Dam", mount->damroll, fread_number (fp));
	  KEY ("Desc", mount->description, fread_string (fp));
	  break;

	case 'E':
	  if (!str_cmp (word, "End"))
	    {
	      // Akamai 6/30/98 - Bug: #13 in a group with mounted paladins some will
	      // not follow the leader. On load the mount was not set to follow their
	      // leader nor be mastered by its leader. This may be only a partial fix
	      mount->leader = ch;
	      mount->master = ch;
	      mount->mounter = ch;
	      mount->next_in_board = ch->pcdata->boarded;
	      ch->pcdata->boarded = mount;
	      ch->mount = mount;

	      loadmount = mount;

	      /* adjust hp mana move up  -- here for speed's sake */
	      percent = (current_time - lastlogoff) * 25 / (2 * 60 * 60);

	      if (percent > 0 && !IS_AFFECTED (ch, AFF_POISON)
		  && !IS_AFFECTED (ch, AFF_PLAGUE))
		{
		  percent = UMIN (percent, 100);
		  mount->hit += (mount->max_hit - mount->hit) * percent / 100;
		  mount->mana +=
		    (mount->max_mana - mount->mana) * percent / 100;
		  mount->move +=
		    (mount->max_move - mount->move) * percent / 100;
		}
	      char_to_room (mount, get_room_index (ROOM_VNUM_SLIMBO));
	      return;
	    }
	  KEY ("Exp", mount->exp, fread_number (fp));
	  break;

	case 'G':
	  KEY ("Gold", mount->gold, fread_number (fp));
	  break;

	case 'H':
	  KEY ("Hit", mount->hitroll, fread_number (fp));

	  if (!str_cmp (word, "HMV"))
	    {
	      mount->hit = fread_number (fp);
	      mount->max_hit = fread_number (fp);
	      mount->mana = fread_number (fp);
	      mount->max_mana = fread_number (fp);
	      mount->move = fread_number (fp);
	      mount->max_move = fread_number (fp);
	      fMatch = TRUE;
	      break;
	    }
	  break;

	case 'L':
	  KEY ("Levl", mount->level, fread_number (fp));
	  KEY ("LnD", mount->long_descr, fread_string (fp));
	  KEY ("LogO", lastlogoff, fread_number (fp));
	  break;

	case 'N':
	  KEY ("Name", mount->name, fread_string (fp));
	  break;

	case 'P':
	  KEY ("Pos", mount->position, fread_number (fp));
	  KEY ("PutIn", mount->wiznet, fread_number (fp));
	  break;

	case 'R':
	  KEY ("Race", mount->race, race_lookup (fread_string (fp)));
	  break;

	case 'S':
	  KEY ("Save", mount->saving_throw, fread_number (fp));
	  KEY ("Sex", mount->sex, fread_number (fp));
	  KEY ("ShD", mount->short_descr, fread_string (fp));
	  KEY ("Silv", mount->silver, fread_number (fp));
	  KEY ("Stable", mount->lines, fread_number (fp));
	  break;

	  if (!fMatch)
	    {
	      bug ("Fread_mount: no match.", 0);
	      fread_to_eol (fp);
	    }

	}
    }

}
