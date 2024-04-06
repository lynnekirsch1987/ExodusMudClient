// obj_trig_save.c
// Saves object triggers to files in the otm directory, each obj_trig
// get its own file to allow for easier managment
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
#include "obj_trig.h"

extern bool is_encrypted;
extern bool fBootDb;
int rename (const char *oldfname, const char *newfname);
OBJ_TRIG_DATA *new_obj_trig(void);
extern OBJ_TRIG *ot_list;
//extern OBJ_TRIG_DATA *obj_trig_list;
extern OBJ_TRIG *giant_ot_list;
extern OBJ_TRIG *on_tick_ot_list;

void save_obj_trig(OBJ_TRIG_DATA *ot);
void fwrite_obj_trig(OBJ_TRIG_DATA *ot, FILE *fp);
AREA_DATA *get_vnum_area args((int vnum));


void otsave(FILE *fp, AREA_DATA *pArea)
{
//	FILE *fp;
	int i=0;
	bool found;
	OBJ_TRIG_DATA *ot;


//	sprintf(strsave, "%s", OTM_FILE);
//	is_encrypted = FALSE;

		encrypt_write("#OT\n", fp);
		for(i = pArea->lvnum; i <= pArea->uvnum; i++)
		{
			if((ot = get_obj_trig(i)) == NULL)
 	                        continue;
			found = TRUE;
			fwrite_obj_trig(ot, fp);
		}
 //	if (found)
  	  encrypt_write("#0\n", fp);
//	else encrypt_write("0\n",fp);
//	encrypt_write("#$\n", fp);
//	fclose(fp);
//	fpReserve = fopen (NULL_FILE, "r");

 //	send_to_char("Save successful.\n\r", ch);
        return;
}

void fwrite_obj_trig(OBJ_TRIG_DATA *ot, FILE *fp)
{
  char buf[MAX_STRING_LENGTH];
  int i;
  
  sprintf(buf, "#%d\n", ot->vnum);
  encrypt_write(buf, fp);
  
  sprintf(buf, "%s~\n", ot->name);
  encrypt_write(buf, fp);
  
  
  
  sprintf(buf, "%s~\n", ot->trigger);
  encrypt_write(buf, fp);
  
  
  sprintf(buf, "%ld\n", ot->trig_flags);
  encrypt_write(buf, fp);
  
  sprintf(buf, "%ld\n", ot->extra_flags);
  encrypt_write(buf, fp);
  
  
  for (i=0;i<16;i++)
    {
      sprintf(buf, "%s~\n", ot->action[i]);
      encrypt_write(buf, fp);
      
      sprintf(buf, "%d\n", ot->chance[i]);
      encrypt_write(buf, fp);
      
      sprintf(buf, "%d\n", ot->delay[i]);
      encrypt_write(buf, fp);
    }
  sprintf(buf, "%d\n", ot->overall_chance);
  encrypt_write(buf, fp);
  
  
  
  return;
}

// load_obj_trigs: called from boot_db loads ot's from obj_trig.are
void load_obj_trigs (FILE *fp)
{
  AREA_DATA *pArea;
  if (area_last == NULL)
    {
      bug ("Load_resets: no #AREA seen yet.", 0);
      exit (1);
    }
  for (;;)
    {
      OBJ_TRIG_DATA *ot, *otp = NULL;
      
      ot_list = NULL;
      giant_ot_list = NULL;
      on_tick_ot_list = NULL;
      for(;;)
	{
	  sh_int vnum;
	  char letter;
	  int i;
	  
	  letter = fread_letter (fp);
	  if (letter != '#')
	    {
	      bug ("load_obj_trigs: # not found.", 0);
	      exit (1);
	    }
	  
	  vnum = fread_number (fp);
	  pArea = get_vnum_area(vnum);
	  if (pArea == NULL)
  	    pArea = get_vnum_area(100);
	  if (vnum == 0)
	    break;
	  
	  fBootDb = FALSE;
	  
	  if (get_obj_trig(vnum) != NULL)
	    {
	      bug ("load_obj_trig: vnum %d duplicated.", vnum);
	      exit (1);
	    }
	  
	  fBootDb = TRUE;
	  
	  ot = new_obj_trig();
	  ot->vnum = vnum;
	  ot->name = fread_string(fp);
	  ot->trigger = fread_string(fp);
	  ot->trig_flags = fread_number(fp);
	  ot->extra_flags = fread_number(fp);
	  for (i = 0; i < 16; i++)
	    {
	      ot->action[i] = fread_string(fp);
	      ot->chance[i] = fread_number(fp);
	      ot->delay[i] = fread_number(fp);
	    }
	  ot->overall_chance = fread_number(fp);
	  // A little bit of sloppy cleanup:
	  if(!str_cmp(ot->name, "(null)"))
	    ot->name = NULL;
	  if(!str_cmp(ot->trigger, "(null)"))
	    ot->trigger = NULL;
	  for (i = 0; i < 16; i++)
	    {
	      if(!str_cmp(ot->action[i], "(null)"))
		ot->action[i] = NULL;
	    }
	  
	  ot->fakie = create_mobile(get_mob_index(MOB_VNUM_FAKIE));
	 

/*          if (pArea == NULL)
	  {
	    bug("Objtrigger vnum %d is NOT IN AN AREA!",vnum);
	    free_ot(ot);
	    continue;
	  }*/
	  
	  if (pArea->ot_first == NULL)
	  {
	    pArea->ot_first = ot;
	    ot->next = NULL;
	  }
	  else 
	  {
	    for (otp = pArea->ot_first; otp->next != NULL;otp = otp->next)
	    {
  	    }
	    otp->next = ot;
	  }
	  /*if (otp == NULL)
	    {
		    //pArea to area_last
	      ot->next = pArea->ot_first;
	      pArea->ot_first = ot;
	      otp = ot;
	    }
	  else
	    {
	      otp->next = ot;
	      ot->next = NULL;
			otp = ot;
	    }*/
	  
	}
      
      return;
    }
}


		










