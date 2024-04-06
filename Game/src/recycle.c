#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"
#include "obj_trig.h"

extern bool ch_is_quitting;
extern OBJ_TRIG *ot_list;
extern OBJ_TRIG *giant_ot_list;
extern OBJ_TRIG *on_tick_ot_list;
//extern OBJ_TRIG_DATA *obj_trig_list;
void free_ot(OBJ_TRIG *ot);
void free_trap (TRAP_DATA * trap);
OBJ_TRIG *new_ot(int vnum);

/* stuff for recyling notes */
NOTE_DATA *note_free;
NOTE_DATA *new_note ()
{
  NOTE_DATA *note;
  if (note_free == NULL)
    note = alloc_perm (sizeof (*note));
//    note = reinterpret_cast<NOTE_DATA *>(alloc_perm(sizeof(*note)));

  else
    {
      note = note_free;
      note_free = note_free->next;
    }
  VALIDATE (note);
  return note;
}

void free_note (NOTE_DATA * note)
{
  if (!IS_VALID (note))
    return;
  free_string (note->text);
  free_string (note->subject);
  free_string (note->to_list);
  free_string (note->date);
  free_string (note->sender);
  INVALIDATE (note);
  note->next = note_free;
  note_free = note;
}


/* stuff for recycling ban structures */
BAN_DATA *ban_free;
BAN_DATA *new_ban (void)
{
  static BAN_DATA ban_zero;
  BAN_DATA *ban;
  if (ban_free == NULL)
    ban = alloc_perm (sizeof (*ban));
//    ban = reinterpret_cast<BAN_DATA *>(alloc_perm(sizeof(*ban)));

  else
    {
      ban = ban_free;
      ban_free = ban_free->next;
    }
  *ban = ban_zero;
  VALIDATE (ban);
  ban->name = &str_empty[0];
  return ban;
}

void free_ban (BAN_DATA * ban)
{
  if (!IS_VALID (ban))
    return;
  free_string (ban->name);
  INVALIDATE (ban);
  ban->next = ban_free;
  ban_free = ban;
}


/* stuff for recycling descriptors */
DESCRIPTOR_DATA *descriptor_free;
DESCRIPTOR_DATA *new_descriptor (void)
{
	static DESCRIPTOR_DATA d_zero;
	DESCRIPTOR_DATA *d;
	if (descriptor_free == NULL) {
	  d = alloc_perm (sizeof (*d));
//		d = reinterpret_cast<DESCRIPTOR_DATA *>(alloc_perm(sizeof(*d)));
	} else {
		d = descriptor_free;
		descriptor_free = descriptor_free->next;
	}
	*d = d_zero;
	VALIDATE (d);
	return d;
}

void free_descriptor (DESCRIPTOR_DATA * d)
{
  if (!IS_VALID (d))
    return;
  free_string (d->host);
  free_mem (d->outbuf, d->outsize);
  INVALIDATE (d);
  d->next = descriptor_free;
  descriptor_free = d;
}


/* stuff for recycling gen_data */
GEN_DATA *gen_data_free;
GEN_DATA *new_gen_data (void)
{
	static GEN_DATA gen_zero;
	GEN_DATA *gen;
	if (gen_data_free == NULL) {
		gen = alloc_perm (sizeof (*gen));
//		gen = reinterpret_cast<GEN_DATA *>(alloc_perm(sizeof(*gen)));
	} else {
		gen = gen_data_free;
		gen_data_free = gen_data_free->next;
	}
	*gen = gen_zero;
	VALIDATE (gen);
	return gen;
}

void free_gen_data (GEN_DATA * gen)
{
  if (!IS_VALID (gen))
    return;
  INVALIDATE (gen);
  gen->next = gen_data_free;
  gen_data_free = gen;
}


/* stuff for recycling extended descs */
EXTRA_DESCR_DATA *extra_descr_free;
EXTRA_DESCR_DATA *new_extra_descr (void)
{
  EXTRA_DESCR_DATA *ed;
  if (extra_descr_free == NULL) {
    ed = alloc_perm (sizeof (*ed));
//    ed = reinterpret_cast<EXTRA_DESCR_DATA *>(alloc_perm(sizeof(*ed)));
  } else {
    ed = extra_descr_free;
    extra_descr_free = extra_descr_free->next;
  }
  ed->keyword = &str_empty[0];
  ed->description = &str_empty[0];
  VALIDATE (ed);
  return ed;
}

void free_extra_descr (EXTRA_DESCR_DATA * ed)
{
  if (!IS_VALID (ed))
    return;
  free_string (ed->keyword);
  free_string (ed->description);
  INVALIDATE (ed);
  ed->next = extra_descr_free;
  extra_descr_free = ed;
}


/* stuff for recycling affects */
AFFECT_DATA *affect_free;
AFFECT_DATA *new_affect (void)
{
  static AFFECT_DATA af_zero;
  AFFECT_DATA *af;
  if (affect_free == NULL) {
    af = alloc_perm (sizeof (*af));
//    af = reinterpret_cast<AFFECT_DATA *>(alloc_perm(sizeof(*af)));
  } else {
    af = affect_free;
    affect_free = affect_free->next;
  }
  *af = af_zero;
  af->comp_name = str_dup("");
  af->permaff = FALSE;
  af->composition = FALSE;
  VALIDATE (af);
  return af;
}

void free_affect (AFFECT_DATA * af)
{
  if (!IS_VALID (af))
    return;
  if (af->comp_name != NULL)
    free_string (af->comp_name);
  INVALIDATE (af);
  af->next = affect_free;
  affect_free = af;
}

SCENT_DATA *scent_free;
MEMORY_DATA *memory_free;
COMP_DATA *comp_free;

/* stuff for recycling objects */
OBJ_DATA *obj_free;
OBJ_DATA *new_obj (void)
{
  static OBJ_DATA obj_zero;
  OBJ_DATA *obj;
  if (obj_free == NULL) {
    obj = alloc_perm (sizeof (*obj));
//    obj = reinterpret_cast<OBJ_DATA *>(alloc_perm(sizeof(*obj)));
  } else {
    obj = obj_free;
    obj_free = obj_free->next;
  }
  *obj = obj_zero;
  VALIDATE (obj);
  return obj;
}

TRAP_DATA *trap_free;
TRAP_DATA *new_trap (void)
{
  static TRAP_DATA trap_zero;
  TRAP_DATA *trap;
  if (trap_free == NULL) {
    trap = alloc_perm (sizeof (*trap));
//    trap = reinterpret_cast<TRAP_DATA *>(alloc_perm(sizeof(*trap)));
  } else {
    trap = trap_free;
    trap_free = trap_free->next;
  }
  *trap = trap_zero;
  VALIDATE(trap);
  // Don't know if these two lines are necessary but I'm paranoid, Paarsh
  trap->dammessage = str_dup("");
  trap->owner = str_dup("");
  return trap;
}

/*BLADE_SPELL_DATA *bs_free;
BLADE_SPELL_DATA *new_blade_spell(void)
{
   static BLADE_SPELL_DATA bs_zero;
   BLADE_SPELL_DATA *bs;
   if(bs_free == NULL)
      bs = alloc_perm(sizeof(*bs));
   else
   {
      bs = bs_free;
      bs_free = bs_free->next;
   }

   *bs = bs_zero;
   VALIDATE(bs);

   return bs;
} */
SCENT_DATA *new_scent (void)
{
  static SCENT_DATA scent_zero;
  SCENT_DATA *scent;
  if (scent_free == NULL) {
    scent = alloc_perm (sizeof (*scent));
//    scent = reinterpret_cast<SCENT_DATA *>(alloc_perm(sizeof(*scent)));
  } else {
    scent = scent_free;
    scent_free = scent_free->next;
  }
  *scent = scent_zero;
  return (scent);
}

COMP_DATA *new_comp(void)
{
  static COMP_DATA comp_zero;
  COMP_DATA *comp;
  if (comp_free == NULL) {
    comp = alloc_perm (sizeof (*comp));
//    comp = reinterpret_cast<COMP_DATA *>(alloc_perm(sizeof(*comp)));
  } else {
    comp = comp_free;
    comp_free = comp_free->next;
  }
  *comp = comp_zero;
  comp->name = NULL;
  comp->title = NULL;
  comp->next = NULL;
  comp->type = 0;
  comp->skill = 0;
  return (comp);
}

MEMORY_DATA *new_memory (void)
{
  static MEMORY_DATA memory_zero;
  MEMORY_DATA *memd;
  if (memory_free == NULL) {
    memd = alloc_perm (sizeof (*memd));
//    memd = reinterpret_cast<MEMORY_DATA *>(alloc_perm(sizeof(*memd)));
  } else { 
    memd = memory_free;
    memory_free = memory_free->next;
  }
  *memd = memory_zero;
  memd->player = NULL;
  memd->mob = NULL;
  memd->next = NULL;
  memd->next_memory = NULL;
  return (memd);
}


void free_obj (OBJ_DATA * obj)
{
  AFFECT_DATA *paf, *paf_next;
  EXTRA_DESCR_DATA *ed, *ed_next;
  int i;
  if (!IS_VALID (obj))
    return;
  for (paf = obj->affected; paf != NULL; paf = paf_next)
    {
      paf_next = paf->next;
      free_affect (paf);
    }
  obj->affected = NULL;
  for (ed = obj->extra_descr; ed != NULL; ed = ed_next)
    {
      ed_next = ed->next;
      free_extra_descr (ed);
    }
  obj->extra_descr = NULL;
  free_string (obj->name);
  free_string (obj->description);
  free_string (obj->short_descr);
  free_string (obj->owner);
  if (obj->plr_owner)
    free_string (obj->plr_owner);
  //Iblis 12-31-03 Object triggers
  for (i=0;i<MAX_OBJ_TRIGS;i++)
  {
    if (obj->objtrig[i])
    {
      obj->objtrig[i]->obj_on = NULL;
      free_ot(obj->objtrig[i]);
      obj->objtrig[i] = NULL;
    }
  }
  INVALIDATE (obj);
  obj->next = obj_free;
  obj_free = obj;
}

void free_trap (TRAP_DATA * trap)
{
  if (!IS_VALID (trap))
	return;
  
  free_string (trap->owner);
  free_string (trap->dammessage);
  INVALIDATE (trap);
  trap->next = trap_free;
  trap_free = trap;
}

/*void free_blade_spell (BLADE_SPELL_DATA *bs)
{
   int i;
   if(!IS_VALID(bs))
      return;

   for(i = 0; i < MAX_BLADE_SPELLS; i++)
   {
      bs->capacity[i] = 0;
      bs->charges[i] = 0;
      bs->level[i] = 0;
   }
   INVALIDATE(bs);
   bs->next = bs_free;
   bs_free = bs;
} */
void free_scent (SCENT_DATA * sc)
{
  sc->player = NULL;
  sc->next_in_room = NULL;
  sc->in_room = NULL;
  sc->next = scent_free;
  scent_free = sc;
}

void free_memory (MEMORY_DATA * memd)
{
  memd->player = NULL;
  memd->mob = NULL;
  memd->next_memory = NULL;
  memd->next = memory_free;
  memory_free = memd;
}

void free_comp (COMP_DATA * comp)
{
  comp->name = NULL;
  comp->title = NULL;
  comp->type = 0;
  comp->skill = 0;
  comp->next = comp_free;
  comp_free = comp;
}

/* stuff for recyling characters */
CHAR_DATA *char_free;
FAKE_DATA *fake_free;
FAKE_DATA *new_fake (void)
{
  static FAKE_DATA fake_zero;
  FAKE_DATA *fd;
  if (fake_free == NULL) {
    fd = alloc_perm (sizeof (*fd));
//    fd = reinterpret_cast<FAKE_DATA *>(alloc_perm(sizeof(*fd)));
  } else {
    fd = fake_free;
    fake_free = fake_free->next;
  }
  *fd = fake_zero;
  fd->name = &str_empty[0];
  fd->title = &str_empty[0];
  fd->race = 0;
  fd->Class = 0;
  fd->level = 0;
  fd->descriptor = 0;
  fd->next = NULL;
  return fd;
}

CHAR_DATA *new_char (void)
{
  static CHAR_DATA ch_zero;
  CHAR_DATA *ch;
  int i;
  if (char_free == NULL) {
    ch = alloc_perm (sizeof (*ch));
//    ch = reinterpret_cast<CHAR_DATA *>(alloc_perm(sizeof(*ch)));
  } else {
    ch = char_free;
    char_free = char_free->next;
  }
  *ch = ch_zero;
  VALIDATE(ch);
  ch->name = &str_empty[0];
  ch->short_descr = &str_empty[0];
  ch->long_descr = &str_empty[0];
  ch->description = &str_empty[0];
  ch->prompt = &str_empty[0];
  ch->prefix = &str_empty[0];
  ch->tracking = &str_empty[0];
  ch->duel = &str_empty[0];
  ch->afkmsg = str_dup ("");
  ch->next_in_board = NULL;
  ch->logon = current_time;
  ch->lines = PAGELEN;
  for (i = 0; i < 4; i++)
    ch->armor[i] = 100;
  ch->position = POS_STANDING;
  ch->hit = 20;
  ch->max_hit = 20;
  ch->mana = 100;
  ch->max_mana = 100;
  ch->move = 100;
  ch->max_move = 100;
  ch->stunned = 0;
  for (i = 0; i < MAX_STATS; i++) {
    ch->perm_stat[i] = 13;
    ch->mod_stat[i] = 0;
  }
  ch->contaminator = NULL;
  return ch;
}

void free_fake (FAKE_DATA * fd)
{
  free_string (fd->name);
  free_string (fd->title);
  fd->next = fake_free;
  fake_free = fd;
}

void free_char (CHAR_DATA * ch)
{
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;
  TRIGGER_DATA *trigger, *trig_next;
  SCRIPT_DATA *scr, *scr_next;
  ROOM_INDEX_DATA *old_room;
  if (!IS_VALID (ch))
    return;
  if (IS_NPC (ch))
    {
      mobile_count--;
      for (trigger = ch->triggers; trigger != NULL; trigger = trig_next)
	{
	  trig_next = trigger->next;
	  free_string (trigger->keywords);
	  free_string (trigger->name);
	  for (scr = trigger->script; scr != NULL; scr = scr_next)
	    {
	      scr_next = scr->next;
	      if (scr->command != NULL)
		free_string (scr->command);
	      free (scr);
	    }
	  free (trigger);
	}
    }
  //10/05/03 Iblis - Added a super cheap set to fix the age old linkdead duping weilding bug
  ch_is_quitting = TRUE; //Set so that the message of dropping item won't appear
  old_room = ch->in_room;
  ch->in_room = get_room_index(1);
  for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      extract_obj (obj);
    }
  ch->in_room = old_room;
  ch_is_quitting = FALSE;
  for (paf = ch->affected; paf != NULL; paf = paf_next)
    {
      paf_next = paf->next;
      affect_remove (ch, paf);
    }
  free_string (ch->name);
  free_string (ch->short_descr);
  free_string (ch->long_descr);
  free_string (ch->description);
  free_string (ch->prompt);
  free_string (ch->prefix);
  free_string (ch->tracking);
  if (ch->sword != NULL)
  {
    log_string("Extracting ch->sword from");
    log_string(ch->name);
    extract_obj (ch->sword);
  }
  if (ch->mask != NULL)
    free_string (ch->mask);
  if (ch->pcdata != NULL)
    free_pcdata (ch->pcdata);
  ch->next = char_free;
  char_free = ch;
  INVALIDATE (ch);
  return;
}

PC_DATA *pcdata_free;
PC_DATA *new_pcdata (void)
{
  short x;
  static PC_DATA pcdata_zero;
  PC_DATA *pcdata;
  if (pcdata_free == NULL) {
    pcdata = alloc_perm (sizeof (*pcdata));
//    pcdata = reinterpret_cast<PC_DATA *>(alloc_perm(sizeof(*pcdata)));
  } else {
    pcdata = pcdata_free;
    pcdata_free = pcdata_free->next;
  }
  *pcdata = pcdata_zero;
  for (x = 0; x < MAX_ALIAS; x++) {
    pcdata->alias[x] = NULL;
    pcdata->alias_sub[x] = NULL;
  }
  for (x = 0; x < MAX_HISTORY; x++)
    pcdata->history[x] = NULL;
  pcdata->buffer = new_buf();
  pcdata->boarded = NULL;
  VALIDATE(pcdata);
  return pcdata;
}

void free_pcdata (PC_DATA * pcdata)
{
  short x;
  if (!IS_VALID (pcdata))
    return;
  free_string (pcdata->pwd);
  free_string (pcdata->bamfin);
  free_string (pcdata->speedwalk);
  free_string (pcdata->bamfout);
  free_string (pcdata->whoinfo);
  free_string (pcdata->pretitle);
  free_string (pcdata->battlecry);
  free_string (pcdata->plan);
  free_string (pcdata->title);
  free_string (pcdata->restoremsg);
  free_string (pcdata->email_addr);
  if (pcdata->buffer != NULL)
    free_buf (pcdata->buffer);
  for (x = 0; x < MAX_ALIAS; x++)
    {
      free_string (pcdata->alias[x]);
      free_string (pcdata->alias_sub[x]);
    }
  for (x = 0; x < MAX_HISTORY; x++)
    free_string (pcdata->history[x]);
  free_string (pcdata->familiar_name);
  if (pcdata->familiar)
    extract_char(pcdata->familiar, TRUE);
  INVALIDATE (pcdata);
  pcdata->next = pcdata_free;
  pcdata_free = pcdata;
  return;
}


/* stuff for recycling bounties */
BOUNTY_DATA *bounty_free;
BOUNTY_DATA *new_bounty (void)
{
  static BOUNTY_DATA b_zero;
  BOUNTY_DATA *b;
  if (bounty_free == NULL) {
    b = alloc_perm (sizeof (*b));
//    b = reinterpret_cast<BOUNTY_DATA *>(alloc_perm(sizeof(*b)));
  } else {
    b = bounty_free;
    bounty_free = bounty_free->next;
  }
  *b = b_zero;
  VALIDATE(b);
  return b;
}

void free_bounty (BOUNTY_DATA * b)
{
  if (!IS_VALID (b))
    return;
  free_string (b->name);
  INVALIDATE (b);
  b->next = bounty_free;
  bounty_free = b;
}


/* stuff for recycling inventions */
INVENTION_DATA *invention_free;
INVENTION_DATA *new_invention (void)
{
  static INVENTION_DATA b_zero;
  INVENTION_DATA *b;
  if (invention_free == NULL) {
    b = alloc_perm (sizeof (*b));
//    b = reinterpret_cast<INVENTION_DATA *>(alloc_perm(sizeof(*b)));
  } else {
    b = invention_free;
    invention_free = invention_free->next;
  }
  *b = b_zero;
  VALIDATE(b);
  return b;
}

void free_invention (INVENTION_DATA * b)
{
  if (!IS_VALID (b))
    return;

  //  free_string( b->name );
  INVALIDATE (b);
  b->next = invention_free;
  invention_free = b;
}


/* stuff for recycling aggressor_lists */
AGGRESSOR_LIST *aggressor_list_free;
AGGRESSOR_LIST *new_aggressor (void)
{
  static AGGRESSOR_LIST b_zero;
  AGGRESSOR_LIST *b;
  if (aggressor_list_free == NULL) {
    b = alloc_perm (sizeof (*b));
//    b = reinterpret_cast<AGGRESSOR_LIST *>(alloc_perm(sizeof(*b)));
  } else {
    b = aggressor_list_free;
    aggressor_list_free = aggressor_list_free->next;
  }
  *b = b_zero;
  VALIDATE(b);
  return b;
}

void free_aggressor_list (AGGRESSOR_LIST * b)
{
  if (!IS_VALID (b))
    return;

  //  free_string( b->name );
  INVALIDATE (b);

  //Iblis 6/28/03 - I take char of pointers in the places this is called..that was just easier
  b->next = aggressor_list_free;
  aggressor_list_free = b;
}


/* stuff for setting ids */
long last_pc_id;
long last_mob_id;
long get_pc_id (void)
{
  int val;
  val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;
  last_pc_id = val;
  return val;
}

long get_mob_id (void)
{
  last_mob_id++;
  return last_mob_id;
}

BUFFER *buf_free;
const int buf_size[MAX_BUF_LIST] =
  { 16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768, 65536
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int get_size (int val)
{
  int i;
  for (i = 0; i < MAX_BUF_LIST; i++)
    if (buf_size[i] >= val)
      {
	return buf_size[i];
      }
  return -1;
}

BUFFER *new_buf()
{
  BUFFER *buffer;
  if (buf_free == NULL) {
    buffer = alloc_perm (sizeof (*buffer));
//    buffer = reinterpret_cast<BUFFER *>(alloc_perm(sizeof(*buffer)));
  } else {
    buffer = buf_free;
    buf_free = buf_free->next;
  }
  buffer->next = NULL;
  buffer->state = BUFFER_SAFE;
  buffer->size = get_size(BASE_BUF);
  buffer->string = alloc_mem (buffer->size);
//  buffer->string = reinterpret_cast<char *>(alloc_mem(buffer->size));
  buffer->string[0] = '\0';
  VALIDATE(buffer);
  return buffer;
}

BUFFER *new_buf_size (int size)
{
  BUFFER *buffer;
  if (buf_free == NULL) {
    buffer = alloc_perm (sizeof (*buffer));
//    buffer = reinterpret_cast<BUFFER *>(alloc_perm(sizeof(*buffer)));
  } else {
    buffer = buf_free;
    buf_free = buf_free->next;
  }
  buffer->next = NULL;
  buffer->state = BUFFER_SAFE;
  buffer->size = get_size(size);
  if (buffer->size == -1) {
    bug ("new_buf: buffer size %d too large.", size);
    exit (1);
  }
  buffer->string = alloc_mem (buffer->size);
//  buffer->string = reinterpret_cast<char *>(alloc_mem(buffer->size));
  buffer->string[0] = '\0';
  VALIDATE(buffer);
  return buffer;
}

void free_buf (BUFFER * buffer)
{
  if (!IS_VALID (buffer) || buffer == NULL)
    return;
  free_mem (buffer->string, buffer->size);
  buffer->string = NULL;
  buffer->size = 0;
  buffer->state = BUFFER_FREED;
  INVALIDATE (buffer);
  buffer->next = buf_free;
  buf_free = buffer;
}

bool add_buf (BUFFER *buffer, char *string)
{
	int len;
	char *oldstr;
	int oldsize;
	oldstr = buffer->string;
	oldsize = buffer->size;
	if (buffer->state == BUFFER_OVERFLOW)	/* don't waste time on bad strings! */
		return FALSE;
	len = strlen(buffer->string) + strlen(string) + 1;
	while (len >= buffer->size) { /* increase the buffer size */
		buffer->size = get_size (buffer->size + 1);
		if (buffer->size == -1) { /* overflow */
			buffer->size = oldsize;
			buffer->state = BUFFER_OVERFLOW;
			bug ("buffer overflow past size %d", buffer->size);
			return FALSE;
		}
	}
	if (buffer->size != oldsize) {
		buffer->string = alloc_mem (buffer->size);
//		buffer->string = reinterpret_cast<char *>(alloc_mem(buffer->size));
		strcpy(buffer->string, oldstr);
		free_mem(oldstr, oldsize);
	}
	strcat(buffer->string, string);
	return TRUE;
}

void clear_buf (BUFFER * buffer)
{
  buffer->string[0] = '\0';
  buffer->state = BUFFER_SAFE;
}

char *buf_string (BUFFER * buffer)
{
  return buffer->string;
}



OBJ_TRIG_DATA *obj_trig_free;
OBJ_TRIG_DATA *new_obj_trig(void)
{
	static OBJ_TRIG_DATA obj_trig_zero;
	OBJ_TRIG_DATA *ot;
	int i;

	if (NULL == obj_trig_free) {
		ot = alloc_perm(sizeof(*ot));
//		ot = reinterpret_cast<OBJ_TRIG_DATA *>(alloc_perm(sizeof(*ot)));
	} else {
		ot = obj_trig_free;
		obj_trig_free = obj_trig_free->next;
	}

	*ot = obj_trig_zero;
	VALIDATE(ot);
	ot->trig_flags = 0;
	ot->trigger = NULL;
	ot->extra_flags = 0;
	for (i = 0; i < 16; i++) {
		ot->action[i] = NULL;
		ot->chance[i] = 0;
		ot->delay[i] = 0;
	}
	ot->overall_chance = 100;
//	for (i = 0; i < MAX_OBJ_TRIG_PARENTS; i++)
//		ot->parent[i] = 0;

	return ot;
}

void free_obj_trig(OBJ_TRIG_DATA *ot)
{
	int i = 0;

	if (NULL == ot || !IS_VALID(ot))
		return;

	free_string(ot->trigger);
	for (i = 0; i < 16; i++)
		free_string(ot->action[i]);

	INVALIDATE(ot);

	ot->next = obj_trig_free;
	obj_trig_free = ot;
}

OBJ_TRIG *ot_free;
OBJ_TRIG *new_ot(int vnum)
{
  static OBJ_TRIG ot_zero;
  OBJ_TRIG *ot;
  OBJ_TRIG_DATA *otd;

  otd = get_obj_trig(vnum);
  if (NULL == otd) {
    bug("when creating ot, Object Trigger vnum -> %d not found", vnum);
    return NULL;
  }
  if (NULL == ot_free) {
    ot = alloc_perm(sizeof(*ot));
//    ot = reinterpret_cast<OBJ_TRIG *>(alloc_perm(sizeof(*ot)));
  } else {
    ot = ot_free;
    ot_free = ot_free->next;
  }

  *ot = ot_zero;
  VALIDATE(ot);
/*  ot->trig_flags = 0;
  ot->trigger = NULL;
  for (i = 0; i < 16; i++) {
    ot->action[i] = NULL;
    ot->chance[i] = 0;
    ot->delay[i] = 0;
  }
  for(i = 0; i < MAX_OBJ_TRIG_PARENTS; i++)
    ot->parent[i] = 0;
*/
//  ot->next = ot_list;
//  ot_list = ot;
  ot->next = NULL;
  ot->next_on_tick = NULL;
  ot->statement_on = 0;
  ot->seconds_left = 0;
  ot->pIndexData = otd;
  ot->obj_on = NULL;
/*  if(IS_SET(ot->pIndexData->trig_flags, OBJ_TRIG_CHAR_TO_ROOM))
  {
    ot->next_on_tick = on_tick_ot_list;
    on_tick_ot_list = ot;
  }
  else ot->next_on_tick = NULL;
*/
  ot->next_ot = giant_ot_list;
  giant_ot_list = ot;
  if (IS_SET(otd->trig_flags, OBJ_TRIG_ON_TICK)) {
    ot->next_on_tick = on_tick_ot_list;
    on_tick_ot_list = ot;
  }

  return ot;
}

void free_ot(OBJ_TRIG *ot)
{
	OBJ_TRIG *otp;
	bool found=FALSE;
	int i,k;
	

	if(ot == NULL || !IS_VALID(ot))
          return;
	if ((ot->next != NULL || ot->statement_on != 0 || ot->seconds_left != 0) && ot_list != NULL)
	{
	  if (ot_list == ot)
	    ot_list = ot->next;
	  else if (ot_list->next == ot)
	    ot_list->next = ot->next;
	  else 
  	  {
	    for (otp = ot_list;otp->next != NULL;otp = otp->next)
	    {
	      if (otp->next == ot)
	      {
	        otp->next = ot->next;
	        found = TRUE;
	        break;
	      }
	    }
	    if (!found)
	    {
  	      bug("object trigger to be freed NOT FOUND.",0);
	      //if (ot->obj_on)
	      //  ot->obj_on->objtrig = NULL;
	//      return;
	    }
	  }
	}
	if (on_tick_ot_list != NULL)
	{
	 if (on_tick_ot_list == ot)
	  on_tick_ot_list = ot->next_on_tick;
	 else if (on_tick_ot_list->next_on_tick == ot)
	  on_tick_ot_list->next_on_tick = ot->next_on_tick;
          else
            {
              for (otp = on_tick_ot_list;otp->next_on_tick != NULL;otp = otp->next_on_tick)
                {
                  if (otp->next_on_tick == ot)
                    {
                      otp->next_on_tick = ot->next_on_tick;
                      found = TRUE;
                      break;
                    }
                }
/*              if (!found)
                {
                  bug("object trigger to be freed NOT FOUND on on_tick_ot_list.",0);
                  //if (ot->obj_on)
                  //  ot->obj_on->objtrig = NULL;
                  return;
                }*/
            }
	}

        found = FALSE;
	ot->pIndexData = NULL;
	    if (giant_ot_list == ot)
	      giant_ot_list = ot->next_ot;
	    else if (giant_ot_list->next_ot == ot)
	      giant_ot_list->next_ot = ot->next_ot;
          else
	    {
	      for (otp = giant_ot_list;otp->next_ot != NULL;otp = otp->next_ot)
		{
		  if (otp->next_ot == ot)
		    {
		      otp->next_ot = ot->next_ot;
		      found = TRUE;
		      break;
		    }
		}
	      if (!found)
		{
		  bug("object trigger to be freed NOT FOUND on giant_ot list.",0);
		  //if (ot->obj_on)
		  //  ot->obj_on->objtrig = NULL;
//		  return;
		}
	    }

	if (ot->obj_on)
	{
 	  for (i=0;i<MAX_OBJ_TRIGS;i++)
	  {
  	    if (ot->obj_on->objtrig[i] == ot)
	    {
	      for(k=i;k<MAX_OBJ_TRIGS-1;k++)
	        ot->obj_on->objtrig[k] = ot->obj_on->objtrig[k+1];
	      ot->obj_on->objtrig[MAX_OBJ_TRIGS-1] = NULL;
	    }
	  }
	}
	ot->obj_on = NULL;
	INVALIDATE(ot);
	ot->next = ot_free;
	ot_free = ot;
}

/* Iblis - 8/30/04 - stuff for recycling rune_card data */
RUNE_CARD_DATA *rune_card_free;
RUNE_CARD_DATA *new_rune_card (void)
{
  static RUNE_CARD_DATA b_zero;
  RUNE_CARD_DATA *b;
  if (NULL == rune_card_free) {
    b = alloc_perm (sizeof (*b));
//    b = reinterpret_cast<RUNE_CARD_DATA *>(alloc_perm(sizeof(*b)));
  } else {
    b = rune_card_free;
    rune_card_free = rune_card_free->next;
  }
  *b = b_zero;
  VALIDATE(b);
  return b;
}

void free_rune_card (RUNE_CARD_DATA * b)
{
  if (!IS_VALID (b))
    return;

  //  free_string( b->name );
  INVALIDATE (b);
  b->next = rune_card_free;
  rune_card_free = b;
}
