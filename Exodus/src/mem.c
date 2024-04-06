#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
extern char str_empty[1];
extern AFFECT_DATA *affect_free;
extern int top_affect, top_obj_index, top_mob_index;
extern int top_reset, top_area, top_exit, top_ed, top_room;
AREA_DATA *area_free;
extern EXTRA_DESCR_DATA *extra_descr_free;
EXIT_DATA *exit_free;
ROOM_INDEX_DATA *room_index_free;
OBJ_INDEX_DATA *obj_index_free;
SHOP_DATA *shop_free;
MOB_INDEX_DATA *mob_index_free;
HELP_DATA *help_free;
extern HELP_DATA *help_last;
RESET_DATA *reset_free = NULL;
TRIGGER_DATA *trigger_free;
SCRIPT_DATA *script_free;
VARIABLE_DATA *variable_free;
RESET_DATA *new_reset_data (void)
{
  RESET_DATA *pReset;
  if (!reset_free)
    {
      pReset = alloc_perm (sizeof (*pReset));
//      pReset = reinterpret_cast<RESET_DATA *>(alloc_perm(sizeof(*pReset)));
      top_reset++;
    }
  else
    {
      pReset = reset_free;
      reset_free = reset_free->next;
    }
  pReset->next = NULL;
  pReset->command = 'X';
  pReset->arg1 = 0;
  pReset->arg2 = 0;
  pReset->arg3 = 0;
  pReset->arg4 = 0;
  return pReset;
}

void free_reset_data (RESET_DATA * pReset)
{
  pReset->next = reset_free;
  reset_free = pReset;
  return;
}

AREA_DATA *new_area (void)
{
  AREA_DATA *pArea;
  char buf[MAX_INPUT_LENGTH];
  if (!area_free)
    {
      pArea = alloc_perm (sizeof (*pArea));
//      pArea = reinterpret_cast<AREA_DATA *>(alloc_perm(sizeof(*pArea)));
      top_area++;
    }
  else
    {
      pArea = area_free;
      area_free = area_free->next;
    }
  pArea->next = NULL;
  pArea->name = str_dup ("New area");
  pArea->area_flags = AREA_ADDED;
  pArea->helper = str_dup ("");
  pArea->lvnum = 0;
  pArea->uvnum = 0;
  pArea->age = 0;
  pArea->nplayer = 0;
  pArea->empty = TRUE;		/* ROM patch */
  sprintf (buf, "area%d.are", top_area - 1);
  pArea->filename = str_dup (buf);
  pArea->vnum = top_area - 1;
  pArea->llev = 1;
  pArea->ulev = 10;
  pArea->help = str_dup ("");
  pArea->points = 0;
  return pArea;
}

void free_area (AREA_DATA * pArea)
{
  free_string (pArea->name);
  free_string (pArea->filename);
  free_string (pArea->creator);
  free_string (pArea->help);
  pArea->next = area_free->next;
  area_free = pArea;
  return;
}

EXIT_DATA *new_exit (void)
{
  EXIT_DATA *pExit;
  if (!exit_free)
    {
      pExit = alloc_perm (sizeof (*pExit));
//      pExit = reinterpret_cast<EXIT_DATA *>(alloc_perm(sizeof(*pExit)));
      top_exit++;
    }
  else
    {
      pExit = exit_free;
      exit_free = exit_free->next;
    }
  pExit->u1.to_room = NULL;	/* ROM OLC */
  pExit->next = NULL;
  pExit->exit_info = 0;
  pExit->key = 0;
  pExit->keyword = &str_empty[0];
  pExit->description = &str_empty[0];
  pExit->rs_flags = 0;
  return pExit;
}

void free_exit (EXIT_DATA * pExit)
{
  free_string (pExit->keyword);
  free_string (pExit->description);
  pExit->next = exit_free;
  exit_free = pExit;
  return;
}

ROOM_INDEX_DATA *new_room_index (void)
{
  ROOM_INDEX_DATA *pRoom;
  int door;
  if (!room_index_free)
    {
      pRoom = alloc_perm (sizeof (*pRoom));
//      pRoom = reinterpret_cast<ROOM_INDEX_DATA *>(alloc_perm(sizeof(*pRoom)));
      top_room++;
    }
  else
    {
      pRoom = room_index_free;
      room_index_free = room_index_free->next;
    }
  pRoom->next = NULL;
  pRoom->people = NULL;
  pRoom->contents = NULL;
  pRoom->extra_descr = NULL;
  pRoom->area = NULL;
  for (door = 0; door < MAX_DIR; door++)
    pRoom->exit[door] = NULL;
  pRoom->name = &str_empty[0];
  pRoom->description = &str_empty[0];
  pRoom->enter_msg = &str_empty[0];
  pRoom->exit_msg = &str_empty[0];
  pRoom->owner = str_dup ("");
  pRoom->vnum = 0;
  pRoom->room_flags = 0;
  pRoom->light = 0;
  pRoom->sector_type = 0;
  pRoom->max_in_room = 0;
  pRoom->max_message = str_dup ("");
  pRoom->tp_level = 0;
  pRoom->tp_exp = 0;
  pRoom->tp_msg = &str_empty[0];
  pRoom->enter_msg = &str_empty[0];
  return pRoom;
}

ROOM_INDEX_DATA *copy_room_index (ROOM_INDEX_DATA * old_room)
{
  EXTRA_DESCR_DATA *newed, *ed;
  ROOM_INDEX_DATA *pRoom;
  int door;
  if (!room_index_free)
    {
      pRoom = alloc_perm (sizeof (*pRoom));
//      pRoom = reinterpret_cast<ROOM_INDEX_DATA *>(alloc_perm(sizeof(*pRoom)));
      top_room++;
    }
  else
    {
      pRoom = room_index_free;
      room_index_free = room_index_free->next;
    }
  pRoom->next = NULL;
  pRoom->reset_first = NULL;
  pRoom->reset_last = NULL;
  pRoom->scents = NULL;
  pRoom->people = NULL;
  pRoom->contents = NULL;
  pRoom->extra_descr = NULL;
  for (ed = old_room->extra_descr; ed != NULL; ed = ed->next)
    {
      newed = new_extra_descr ();
      newed->keyword = str_dup (ed->keyword);
      newed->description = str_dup (ed->description);
      newed->next = pRoom->extra_descr;
      pRoom->extra_descr = newed;
    }
  pRoom->area = NULL;
  for (door = 0; door < MAX_DIR; door++)
    pRoom->exit[door] = NULL;
  pRoom->name = str_dup (old_room->name);
  pRoom->description = str_dup (old_room->description);
  pRoom->owner = str_dup (old_room->owner);
  pRoom->enter_msg = str_dup (old_room->enter_msg);
  pRoom->exit_msg = str_dup (old_room->exit_msg);
  pRoom->vnum = 0;
  pRoom->max_in_room = old_room->max_in_room;
  pRoom->max_message = str_dup (old_room->max_message);
  pRoom->room_flags = old_room->room_flags;
  pRoom->light = old_room->light;
  pRoom->sector_type = old_room->sector_type;
  pRoom->heal_rate = old_room->heal_rate;
  pRoom->mana_rate = old_room->mana_rate;
  pRoom->clan = old_room->clan;
  pRoom->max_level = old_room->max_level;
  pRoom->race_flags = old_room->race_flags;
  pRoom->Class_flags = old_room->Class_flags;
  pRoom->ctransport_vnum = 0;
  return (pRoom);
}

void free_room_index (ROOM_INDEX_DATA * pRoom)
{
  int door;
  EXTRA_DESCR_DATA *pExtra;
  RESET_DATA *pReset;
  free_string (pRoom->name);
  free_string (pRoom->description);
  for (door = 0; door < MAX_DIR; door++)
    {
      if (pRoom->exit[door])
	free_exit (pRoom->exit[door]);
    }
  for (pExtra = pRoom->extra_descr; pExtra; pExtra = pExtra->next)
    {
      free_extra_descr (pExtra);
    }
  for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
    {
      free_reset_data (pReset);
    }
  pRoom->next = room_index_free;
  room_index_free = pRoom;
  return;
}

SHOP_DATA *new_shop (void)
{
  SHOP_DATA *pShop;
  int buy;
  if (!shop_free)
    {
      pShop = alloc_perm (sizeof (*pShop));
//      pShop = reinterpret_cast<SHOP_DATA *>(alloc_perm(sizeof(*pShop)));
      top_shop++;
    }
  else
    {
      pShop = shop_free;
      shop_free = shop_free->next;
    }
  pShop->next = NULL;
  pShop->keeper = 0;
  for (buy = 0; buy < MAX_TRADE; buy++)
    pShop->buy_type[buy] = 0;
  pShop->profit_buy = 100;
  pShop->profit_sell = 100;
  pShop->open_hour = 0;
  pShop->close_hour = 23;
  return pShop;
}

void free_shop (SHOP_DATA * pShop)
{
  pShop->next = shop_free;
  shop_free = pShop;
  return;
}

OBJ_INDEX_DATA *new_obj_index (void)
{
  OBJ_INDEX_DATA *pObj;
  int value;
  if (!obj_index_free)
    {
      pObj = alloc_perm (sizeof (*pObj));
//      pObj = reinterpret_cast<OBJ_INDEX_DATA *>(alloc_perm(sizeof(*pObj)));
      top_obj_index++;
    }
  else
    {
      pObj = obj_index_free;
      obj_index_free = obj_index_free->next;
    }
  pObj->next = NULL;
  pObj->extra_descr = NULL;
  pObj->affected = NULL;
  pObj->area = NULL;
  pObj->name = str_dup ("no name");
  pObj->short_descr = str_dup ("(no short description)");
  pObj->description = str_dup ("");
  pObj->vnum = 0;
  pObj->item_type = ITEM_TRASH;
  for (value = 0; value < MAX_EXTRA_FLAGS;value++)
    pObj->extra_flags[value] = 0;
  pObj->wear_flags = 0;

  // Akamai 4/30/99 -- adding Class and race restrictions to objects
  // They are initially set so that race and Class can hold/own them
  pObj->race_flags = 0;
  pObj->Class_flags = 0;
  pObj->clan_flags = 0;
  pObj->timer = 0;
  pObj->count = 0;
  pObj->weight = 0;
  pObj->cost = 0;
  pObj->material = str_dup ("unknown");
  pObj->condition = 100;
  pObj->rarity = 6;
  pObj->string1 = str_dup ("");
  pObj->string2 = str_dup ("");
  pObj->string3 = str_dup ("");
  pObj->string4 = str_dup ("");
  for (value = 0; value < 7; value++)
    pObj->value[value] = 0;
  return pObj;
}

OBJ_INDEX_DATA *copy_obj_index (OBJ_INDEX_DATA * old_obj)
{
  OBJ_INDEX_DATA *pObj;
  EXTRA_DESCR_DATA *ed, *newed;
  AFFECT_DATA *af, *newaf;
  int value;
  if (!obj_index_free)
    {
      pObj = alloc_perm (sizeof (*pObj));
//      pObj = reinterpret_cast<OBJ_INDEX_DATA *>(alloc_perm(sizeof(*pObj)));
      top_obj_index++;
    }
  else
    {
      pObj = obj_index_free;
      obj_index_free = obj_index_free->next;
    }
  pObj->next = NULL;
  pObj->area = NULL;
  pObj->extra_descr = NULL;
  for (ed = old_obj->extra_descr; ed != NULL; ed = ed->next)
    {
      newed = new_extra_descr ();
      newed->keyword = str_dup (ed->keyword);
      newed->description = str_dup (ed->description);
      newed->next = pObj->extra_descr;
      pObj->extra_descr = newed;
    }
  pObj->affected = NULL;
  for (af = old_obj->affected; af != NULL; af = af->next)
    {
      newaf = new_affect ();
      newaf->where = af->where;
      newaf->type = af->type;
      newaf->level = af->level;
      newaf->duration = af->duration;
      newaf->location = af->location;
      newaf->modifier = af->modifier;
      newaf->bitvector = af->bitvector;
      newaf->permaff = af->permaff;
      newaf->composition = FALSE;
      newaf->comp_name = str_dup ("");
      newaf->next = pObj->affected;
      pObj->affected = newaf;
    }
  pObj->name = str_dup (old_obj->name);
  pObj->short_descr = str_dup (old_obj->short_descr);
  pObj->description = str_dup (old_obj->description);
  pObj->vnum = 0;
  pObj->material = str_dup (old_obj->material);
  pObj->item_type = old_obj->item_type;
  for (value = 0; value < MAX_EXTRA_FLAGS; value++)
    pObj->extra_flags[value] = old_obj->extra_flags[value];
  pObj->wear_flags = old_obj->wear_flags;

  // Akamai 4/30/99 -- supporting clas/race specific equipment/objects
  pObj->Class_flags = old_obj->Class_flags;
  pObj->race_flags = old_obj->race_flags;
  pObj->clan_flags = old_obj->clan_flags;
  pObj->level = old_obj->level;
  pObj->condition = 100;

//  pObj->condition = old_obj->condition;
  pObj->count = 0;
  pObj->weight = old_obj->weight;
  pObj->rarity = old_obj->rarity;
  pObj->timer = old_obj->timer;
  pObj->cost = old_obj->cost;
  for (value = 0; value < 13; value++)
    pObj->value[value] = old_obj->value[value];
  pObj->string1 = str_dup (old_obj->string1);
  pObj->string2 = str_dup (old_obj->string2);
  pObj->string3 = str_dup (old_obj->string3);
  pObj->string4 = str_dup (old_obj->string4);
  if (old_obj->plr_owner)
    pObj->plr_owner = str_dup (old_obj->plr_owner);
  else pObj->plr_owner = NULL;
  for (value = 0; value < MAX_OBJ_TRIGS;value++)
    pObj->obj_trig_vnum[value] = old_obj->obj_trig_vnum[value];
  return pObj;
}

void free_obj_index (OBJ_INDEX_DATA * pObj)
{
  EXTRA_DESCR_DATA *pExtra;
  AFFECT_DATA *pAf;
  free_string (pObj->name);
  free_string (pObj->short_descr);
  free_string (pObj->description);
  for (pAf = pObj->affected; pAf; pAf = pAf->next)
    {
      free_affect (pAf);
    }
  for (pExtra = pObj->extra_descr; pExtra; pExtra = pExtra->next)
    {
      free_extra_descr (pExtra);
    }
  pObj->next = obj_index_free;
  obj_index_free = pObj;
  return;
}

MOB_INDEX_DATA *new_mob_index (void)
{
  MOB_INDEX_DATA *pMob;
  if (!mob_index_free)
    {
      pMob = alloc_perm (sizeof (*pMob));
//      pMob = reinterpret_cast<MOB_INDEX_DATA *>(alloc_perm(sizeof(*pMob)));
      top_mob_index++;
    }
  else
    {
      pMob = mob_index_free;
      mob_index_free = mob_index_free->next;
    }
  pMob->next = NULL;
  pMob->spec_fun = NULL;
  pMob->pShop = NULL;
  pMob->area = NULL;
  pMob->player_name = str_dup ("no name");
  pMob->short_descr = str_dup ("(no short description)");
  pMob->long_descr = str_dup ("(no long description)\n\r");
  pMob->description = &str_empty[0];
  pMob->vocfile = &str_empty[0];
  pMob->script_fn = &str_empty[0];
  pMob->triggers = NULL;
  pMob->variables = NULL;
  pMob->vnum = 0;
  pMob->default_mood = 0;
  pMob->count = 0;
  pMob->killed = 0;
  pMob->sex = 0;
  pMob->level = 0;
  pMob->recruit_flags = 0;
  pMob->recruit_value[0] = 0;
  pMob->recruit_value[1] = 0;
  pMob->recruit_value[2] = 0;
  pMob->act = ACT_IS_NPC;
  pMob->affected_by = 0;
  pMob->alignment = 0;
  pMob->hitroll = 0;
  pMob->race = race_lookup ("human");	/* - Hugin */
  pMob->form = 0;		/* ROM patch -- Hugin */
  pMob->parts = 0;		/* ROM patch -- Hugin */
  pMob->imm_flags = 0;		/* ROM patch -- Hugin */
  pMob->res_flags = 0;		/* ROM patch -- Hugin */
  pMob->vuln_flags = 0;		/* ROM patch -- Hugin */
  pMob->material = str_dup ("oldstyle");
  pMob->off_flags = 0;		/* ROM patch -- Hugin */
  pMob->size = SIZE_MEDIUM;	/* ROM patch -- Hugin */
  pMob->ac[AC_PIERCE] = 0;	/* ROM patch -- Hugin */
  pMob->ac[AC_BASH] = 0;	/* ROM patch -- Hugin */
  pMob->ac[AC_SLASH] = 0;	/* ROM patch -- Hugin */
  pMob->ac[AC_EXOTIC] = 0;	/* ROM patch -- Hugin */
  pMob->hit[DICE_NUMBER] = 0;	/* ROM patch -- Hugin */
  pMob->hit[DICE_TYPE] = 0;	/* ROM patch -- Hugin */
  pMob->hit[DICE_BONUS] = 0;	/* ROM patch -- Hugin */
  pMob->mana[DICE_NUMBER] = 0;	/* ROM patch -- Hugin */
  pMob->mana[DICE_TYPE] = 0;	/* ROM patch -- Hugin */
  pMob->mana[DICE_BONUS] = 0;	/* ROM patch -- Hugin */
  pMob->damage[DICE_NUMBER] = 0;	/* ROM patch -- Hugin */
  pMob->damage[DICE_TYPE] = 0;	/* ROM patch -- Hugin */
  pMob->damage[DICE_BONUS] = 0;	/* ROM patch -- Hugin */
  pMob->start_pos = POS_STANDING;	/*  -- Hugin */
  pMob->default_pos = POS_STANDING;	/*  -- Hugin */
  pMob->wealth = 0;
  return pMob;
}

MOB_INDEX_DATA *copy_mob_index (MOB_INDEX_DATA * old_mob)
{
  MOB_INDEX_DATA *pMob;

/* FIXIT */
  if (!mob_index_free)
    {
      pMob = alloc_perm (sizeof (*pMob));
//      pMob = reinterpret_cast<MOB_INDEX_DATA *>(alloc_perm(sizeof(*pMob)));
      top_mob_index++;
    }
  else
    {
      pMob = mob_index_free;
      mob_index_free = mob_index_free->next;
    }
  pMob->next = NULL;
  pMob->area = NULL;
  pMob->spec_fun = old_mob->spec_fun;
  pMob->pShop = old_mob->pShop;
  pMob->vnum = 0;
  pMob->group = old_mob->group;
  pMob->count = 0;
  pMob->killed = 0;
  pMob->player_name = str_dup (old_mob->player_name);
  pMob->short_descr = str_dup (old_mob->short_descr);
  pMob->long_descr = str_dup (old_mob->long_descr);
  pMob->description = str_dup (old_mob->description);
  pMob->act = old_mob->act;
  pMob->act2 = old_mob->act2;
  pMob->affected_by = old_mob->affected_by;
  pMob->alignment = old_mob->alignment;
  pMob->level = old_mob->level;
  pMob->hitroll = old_mob->hitroll;
  pMob->recruit_flags = old_mob->recruit_flags;
  pMob->recruit_value[0] = old_mob->recruit_value[0];
  pMob->recruit_value[1] = old_mob->recruit_value[1];
  pMob->recruit_value[2] = old_mob->recruit_value[2];
  pMob->hit[DICE_NUMBER] = old_mob->hit[DICE_NUMBER];
  pMob->hit[DICE_TYPE] = old_mob->hit[DICE_TYPE];
  pMob->hit[DICE_BONUS] = old_mob->hit[DICE_BONUS];
  pMob->mana[DICE_NUMBER] = old_mob->mana[DICE_NUMBER];
  pMob->mana[DICE_TYPE] = old_mob->mana[DICE_TYPE];
  pMob->mana[DICE_BONUS] = old_mob->mana[DICE_BONUS];
  pMob->damage[DICE_NUMBER] = old_mob->damage[DICE_NUMBER];
  pMob->damage[DICE_TYPE] = old_mob->damage[DICE_TYPE];
  pMob->damage[DICE_BONUS] = old_mob->damage[DICE_NUMBER];
  pMob->ac[AC_PIERCE] = old_mob->ac[AC_PIERCE];
  pMob->ac[AC_BASH] = old_mob->ac[AC_BASH];
  pMob->ac[AC_SLASH] = old_mob->ac[AC_SLASH];
  pMob->ac[AC_EXOTIC] = old_mob->ac[AC_EXOTIC];
  pMob->dam_type = old_mob->dam_type;
  pMob->off_flags = old_mob->off_flags;
  pMob->imm_flags = old_mob->imm_flags;
  pMob->res_flags = old_mob->res_flags;
  pMob->vuln_flags = old_mob->vuln_flags;
  pMob->start_pos = old_mob->start_pos;
  pMob->default_pos = old_mob->default_pos;
  pMob->sex = old_mob->sex;
  pMob->race = old_mob->race;
  pMob->wealth = old_mob->wealth;;
  pMob->form = old_mob->form;
  pMob->parts = old_mob->parts;
  pMob->size = old_mob->size;
  pMob->material = str_dup (old_mob->material);
  pMob->vocfile = str_dup (old_mob->vocfile);
  pMob->script_fn = str_dup (old_mob->script_fn);
  pMob->triggers = NULL;
  pMob->variables = NULL;
  pMob->default_mood = old_mob->default_mood;
  pMob->move = old_mob->move;
  pMob->number_of_attacks = old_mob->number_of_attacks;
  pMob->perm_stat[STAT_STR] = old_mob->perm_stat[STAT_STR];
  pMob->perm_stat[STAT_INT] = old_mob->perm_stat[STAT_INT];
  pMob->perm_stat[STAT_WIS] = old_mob->perm_stat[STAT_WIS];
  pMob->perm_stat[STAT_DEX] = old_mob->perm_stat[STAT_DEX];
  pMob->perm_stat[STAT_CON] = old_mob->perm_stat[STAT_CON];
  pMob->perm_stat[STAT_CHA] = old_mob->perm_stat[STAT_CHA];
  return pMob;
}

void free_mob_index (MOB_INDEX_DATA * pMob)
{
  free_string (pMob->player_name);
  free_string (pMob->short_descr);
  free_string (pMob->long_descr);
  free_string (pMob->description);
  if (pMob->pShop != NULL)
    free_shop (pMob->pShop);
  pMob->next = mob_index_free;
  mob_index_free = pMob;
  return;
}
