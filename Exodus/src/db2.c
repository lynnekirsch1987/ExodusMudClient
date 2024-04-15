#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"
#include "db.h"
#include "lookup.h"
#include "merc.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#if defined(KEY)
#undef KEY
#endif /*  */

#define KEY(literal, field, value)                                             \
  if (!str_cmp(word, literal)) {                                               \
    field = value;                                                             \
    fMatch = TRUE;                                                             \
    break;                                                                     \
  }

/* values for db2.c */
struct social_type social_table[MAX_SOCIALS];
int social_count;
extern char strArea[];
extern FILE *fpArea;
extern bool is_encrypted;
extern int enc_shift;
TRIGGER_DATA *fread_trigger(FILE *fp) {
  char *word;
  bool fMatch;
  TRIGGER_DATA *trigger;
  trigger = alloc_perm(sizeof(*trigger));
  //  trigger = reinterpret_cast<TRIGGER_DATA *>(alloc_perm(sizeof(*trigger)));
  trigger->next = NULL;
  trigger->waiting = 0;
  trigger->current = NULL;
  trigger->script = NULL;
  trigger->trigger_type = 0;
  trigger->bits = 0;
  trigger->keywords = str_dup("");
  trigger->name = str_dup(fread_word(fp));
  {
    SCRIPT_DATA *scr;
    trigger->script = alloc_perm(sizeof(*scr));
    //    trigger->script = reinterpret_cast<SCRIPT_DATA
    //    *>(alloc_perm(sizeof(*scr)));
    scr = trigger->script;
    scr->command = str_dup("");
    scr->next = NULL;
  }
  for (;;) {
    word = feof(fp) ? "End" : fread_word(fp);
    //      word = feof (fp) ? const_cast<char *>("End") : fread_word (fp);
    fMatch = FALSE;
    switch (UPPER(word[0])) {
    case '*':
      fread_to_eol(fp);
      fMatch = TRUE;
      break;
    case 'K':
      KEY("Keywords", trigger->keywords, fread_string(fp));
      break;
    case 'E':
      if (!str_cmp(word, "End")) {
        fMatch = TRUE;
        return trigger;
      }
      if (!str_cmp(word, "END")) {
        fMatch = TRUE;
        return trigger;
      }
      break;
    case 'S':
      if (!str_cmp(word, "Sc") || !str_cmp(word, "Script")) {
        SCRIPT_DATA *scr;
        if (trigger->script != NULL) {
          for (scr = trigger->script; scr->next != NULL; scr = scr->next)
            ; /* scan to the end */
          scr->next = alloc_perm(sizeof(*scr));
          //		  scr->next = reinterpret_cast<SCRIPT_DATA
          //*>(alloc_perm(sizeof(*scr)));
          scr = scr->next;
        } else {
          trigger->script = alloc_perm(sizeof(*scr));
          //		  trigger->script = reinterpret_cast<SCRIPT_DATA
          //*>(alloc_perm (sizeof (*scr)));
          scr = trigger->script;
        }
        scr->command = fread_string(fp);
        scr->next = NULL;
        fMatch = TRUE;
      }
      break;
    case 'T':
      KEY("Type", trigger->trigger_type, fread_number(fp));
      break;
    }
    if (!fMatch) {
      char buf[80];
      sprintf(buf, "fread_trigger: incorrect '%s'", word);
      bug(buf, 0);
      fread_to_eol(fp);
    }
  };
}

/* snarf a socials file */
void load_socials(FILE *fp) {
#ifdef VERBOSE_BOOT
  log_string("load_socials()");
#endif /*  */
  for (;;) {
    struct social_type social;
    char *temp;

    /* clear social */
    social.char_no_arg = NULL;
    social.others_no_arg = NULL;
    social.char_found = NULL;
    social.others_found = NULL;
    social.vict_found = NULL;
    social.char_not_found = NULL;
    social.char_auto = NULL;
    social.others_auto = NULL;
    temp = fread_word(fp);
    if (!strcmp(temp, "#0"))
      return; /* done */

#define social_debug
#if defined(social_debug)
    else
      fprintf(stderr, "%s\n", temp);
#endif /*  */
    strcpy(social.name, temp);
    fread_to_eol(fp);
    temp = fread_string_eol(fp);
    if (!strcmp(temp, "$")) {
      social.char_no_arg = NULL;
    } else if (!strcmp(temp, "#")) {
      social_table[social_count] = social;
      social_count++;
      continue;
    } else {
      social.char_no_arg = temp;
    }

    temp = fread_string_eol(fp);
    if (!strcmp(temp, "$")) {
      social.others_no_arg = NULL;
    } else if (!strcmp(temp, "#")) {
      social_table[social_count] = social;
      social_count++;
      continue;
    } else {
      social.others_no_arg = temp;
    }

    temp = fread_string_eol(fp);
    if (!strcmp(temp, "$")) {
      social.char_found = NULL;
    } else if (!strcmp(temp, "#")) {
      social_table[social_count] = social;
      social_count++;
      continue;
    } else {
      social.char_found = temp;
    }

    temp = fread_string_eol(fp);
    if (!strcmp(temp, "$")) {
      social.others_found = NULL;
    } else if (!strcmp(temp, "#")) {
      social_table[social_count] = social;
      social_count++;
      continue;
    } else {
      social.others_found = temp;
    }

    temp = fread_string_eol(fp);
    if (!strcmp(temp, "$")) {
      social.vict_found = NULL;
    } else if (!strcmp(temp, "#")) {
      social_table[social_count] = social;
      social_count++;
      continue;
    } else {
      social.vict_found = temp;
    }

    temp = fread_string_eol(fp);
    if (!strcmp(temp, "$")) {
      social.char_not_found = NULL;
    } else if (!strcmp(temp, "#")) {
      social_table[social_count] = social;
      social_count++;
      continue;
    } else {
      social.char_not_found = temp;
    }

    temp = fread_string_eol(fp);
    if (!strcmp(temp, "$")) {
      social.char_auto = NULL;
    } else if (!strcmp(temp, "#")) {
      social_table[social_count] = social;
      social_count++;
      continue;
    } else {
      social.char_auto = temp;
    }

    temp = fread_string_eol(fp);
    if (!strcmp(temp, "$")) {
      social.others_auto = NULL;
    } else if (!strcmp(temp, "#")) {
      social_table[social_count] = social;
      social_count++;
      continue;
    } else {
      social.others_auto = temp;
    }
    social_table[social_count] = social;
    social_count++;
  }
  return;
}

void load_scripts(char *fn, MOB_INDEX_DATA *pMob) {
  FILE *fp;
  TRIGGER_DATA *trigger;
  char *word;
  bool fMatch = TRUE, was_encrypted;
  char fullpath[1024];

#ifdef VERBOSE_BOOT
  log_string("load_scripts()");
  log_string(fn);

#endif /*  */
  was_encrypted = is_encrypted;
  is_encrypted = FALSE;
  pMob->triggers = NULL;
  sprintf(fullpath, "mobscr/%s", fn);
  if ((fp = fopen(fullpath, "r")) == NULL) {
    char buf[MAX_STRING_LENGTH];
    sprintf(buf, "%s script file not found!", fn);
    bug(buf, 0);
    return;
  }
  for (; fMatch;) {
    fMatch = FALSE;
    word = feof(fp) ? "#END" : fread_word(fp);
    //      word = feof (fp) ? const_cast<char *>("#END") : fread_word (fp);
    if (word[0] == '*') { /* In-file comments */
      fread_to_eol(fp);
      fMatch = TRUE;
    } else if (word[0] == '#') {
      if (!str_cmp(word, "#END")) {
        fMatch = TRUE;
        break;
      } else if (!str_cmp(word, "#TRIGGER")) {
        fMatch = TRUE;
        trigger = fread_trigger(fp);
        trigger->next = pMob->triggers;
        pMob->triggers = trigger;
      }
    }
  }
  if (!fMatch) {
    strcpy(strArea, fn);
    fpArea = fp;
    bug("Load_scripts: bad key word.", 0);
    exit(1);
  }
  fclose(fp);
  is_encrypted = was_encrypted;
  return;
}

/*
 * Snarf a mob section.  new style
 */
void load_mobiles(FILE *fp) {
  MOB_INDEX_DATA *pMobIndex;
  int i = 0;

#ifdef VERBOSE_BOOT
  log_string("load_mobiles()");

#endif              /*  */
  if (!area_last) { /* OLC */
    bug("Load_mobiles: no #AREA seen yet.", 0);
    exit(1);
  }
  for (;;) {
    sh_int vnum;
    char letter;
    int iHash;
    letter = fread_letter(fp);
    if (letter != '#') {
      bug("Load_mobiles: # not found.", 0);
      exit(1);
    }
    vnum = fread_number(fp);
    if (vnum == 0)
      break;
    fBootDb = FALSE;
    if (get_mob_index(vnum) != NULL) {
      bug("Load_mobiles: vnum %d duplicated.", vnum);
      exit(1);
    }
    fBootDb = TRUE;
    pMobIndex = alloc_perm(sizeof(*pMobIndex));
    //      pMobIndex = reinterpret_cast<MOB_INDEX_DATA *>(alloc_perm (sizeof
    //      (*pMobIndex)));
    pMobIndex->vnum = vnum;
    pMobIndex->area = area_last; /* OLC */
    newmobs++;
    pMobIndex->player_name = fread_string(fp);
    pMobIndex->short_descr = fread_string(fp);
    pMobIndex->long_descr = fread_string(fp);
    pMobIndex->description = fread_string(fp);
    pMobIndex->race = race_lookup(fread_string(fp));
    pMobIndex->triggers = NULL;
    pMobIndex->variables = NULL;
    pMobIndex->last_killer = NULL;
    pMobIndex->last_killer2 = NULL;
    pMobIndex->recruit_flags = fread_flag(fp);
    pMobIndex->recruit_value[0] = fread_number(fp);
    pMobIndex->recruit_value[1] = fread_number(fp);
    pMobIndex->recruit_value[2] = fread_number(fp);
    pMobIndex->long_descr[0] = UPPER(pMobIndex->long_descr[0]);
    pMobIndex->description[0] = UPPER(pMobIndex->description[0]);
    pMobIndex->act =
        fread_flag(fp) | ACT_IS_NPC | race_table[pMobIndex->race].act;

    pMobIndex->affected_by = fread_flag(fp) | race_table[pMobIndex->race].aff;
    pMobIndex->pShop = NULL;
    pMobIndex->alignment = fread_number(fp);
    pMobIndex->group = fread_number(fp);
    pMobIndex->level = fread_number(fp);
    pMobIndex->hitroll = fread_number(fp);
    // IBLIS TEMPORARY THING
    //      if (pMobIndex->level < 61)
    //        pMobIndex->hitroll = pMobIndex->level;

    /* read hit dice */
    pMobIndex->hit[DICE_NUMBER] = fread_number(fp);

    /* 'd'          */ fread_letter(fp);
    pMobIndex->hit[DICE_TYPE] = fread_number(fp);

    /* '+'          */ fread_letter(fp);
    pMobIndex->hit[DICE_BONUS] = fread_number(fp);

    /* read mana dice */
    pMobIndex->mana[DICE_NUMBER] = fread_number(fp);
    fread_letter(fp);
    pMobIndex->mana[DICE_TYPE] = fread_number(fp);
    fread_letter(fp);
    pMobIndex->mana[DICE_BONUS] = fread_number(fp);

    /* read damage dice */
    pMobIndex->damage[DICE_NUMBER] = fread_number(fp);
    fread_letter(fp);
    pMobIndex->damage[DICE_TYPE] = fread_number(fp);
    fread_letter(fp);
    pMobIndex->damage[DICE_BONUS] = fread_number(fp);
    pMobIndex->dam_type = attack_lookup(fread_word(fp));

    /* read armor Class */
    pMobIndex->ac[AC_PIERCE] = fread_number(fp) * 10;
    pMobIndex->ac[AC_BASH] = fread_number(fp) * 10;
    pMobIndex->ac[AC_SLASH] = fread_number(fp) * 10;
    pMobIndex->ac[AC_EXOTIC] = fread_number(fp) * 10;

    /*10/12/03 - Iblis - Added this to properly load new flags Cory added a
     * couple months ago*/
    if (pMobIndex->ac[AC_EXOTIC] == 9990) {
      pMobIndex->ac[AC_EXOTIC] = fread_number(fp) * 10;
      pMobIndex->act2 = fread_flag(fp);
      pMobIndex->blocks_exit = fread_number(fp);
      if (pMobIndex->blocks_exit > 50) {
        pMobIndex->blocks_exit -= 60;
        pMobIndex->number_of_attacks = fread_number(fp);
      } else
        pMobIndex->number_of_attacks = -1;
    } else {
      pMobIndex->blocks_exit = -1;
      pMobIndex->act2 = 0;
      pMobIndex->number_of_attacks = -1;
    }

    /* read flags and add in data from the race table */
    pMobIndex->off_flags = fread_flag(fp) | race_table[pMobIndex->race].off;
    pMobIndex->imm_flags = fread_flag(fp) | race_table[pMobIndex->race].imm;
    pMobIndex->res_flags = fread_flag(fp) | race_table[pMobIndex->race].res;
    pMobIndex->vuln_flags = fread_flag(fp) | race_table[pMobIndex->race].vuln;

    /* vital statistics */
    pMobIndex->start_pos = position_lookup(fread_word(fp));
    pMobIndex->default_pos = position_lookup(fread_word(fp));
    pMobIndex->sex = sex_lookup(fread_word(fp));
    //       fprintf (stderr, "!! - %d - %d - %ld - %d - %d - %d \n",
    //       pMobIndex->ac[AC_EXOTIC], pMobIndex->blocks_exit,
    //       pMobIndex->vuln_flags,
    //		       pMobIndex->start_pos, pMobIndex->default_pos,
    // pMobIndex->sex);
    pMobIndex->wealth = fread_number(fp);
    pMobIndex->form = fread_flag(fp) | race_table[pMobIndex->race].form;
    pMobIndex->parts = fread_flag(fp) | race_table[pMobIndex->race].parts;

    // Iblis - 8/29/04 - Added a slot for a rune card vnum, and setable stats
    // (con/str/etc)
    if (area_last->version >= AREA_VER_CARDS) {
      for (i = 0; i < MAX_STATS; i++)
        pMobIndex->perm_stat[i] = fread_number(fp);
      pMobIndex->card_vnum = fread_number(fp);
    } else {
      pMobIndex->card_vnum = 0;
      // Iblis 8/29/04- Set Mob's stats according to Iverath's requests
      for (i = 0; i < MAX_STATS; i++)
        pMobIndex->perm_stat[i] = 6 + (pMobIndex->level * (19 / 90));
    }

    /* size */
    pMobIndex->size = size_lookup(fread_word(fp));
    pMobIndex->material = str_dup(fread_string(fp));
    pMobIndex->defbonus = fread_number(fp);
    pMobIndex->attackbonus = fread_number(fp);
    pMobIndex->max_weight = fread_number(fp);
    pMobIndex->move = fread_number(fp);
    pMobIndex->default_mood = fread_number(fp);
    pMobIndex->vocfile = fread_string(fp);
    pMobIndex->script_fn = fread_string(fp);
    if (pMobIndex->script_fn[0] != '\0')
      load_scripts(pMobIndex->script_fn, pMobIndex);
    for (;;) {
      letter = fread_letter(fp);
      if (letter == 'F') {
        char *word;
        long vector;
        word = fread_word(fp);
        vector = fread_flag(fp);
        if (!str_prefix(word, "act"))
          REMOVE_BIT(pMobIndex->act, vector);

        else if (!str_prefix(word, "aff"))
          REMOVE_BIT(pMobIndex->affected_by, vector);

        else if (!str_prefix(word, "off"))
          REMOVE_BIT(pMobIndex->off_flags, vector);

        else if (!str_prefix(word, "imm"))
          REMOVE_BIT(pMobIndex->imm_flags, vector);

        else if (!str_prefix(word, "res"))
          REMOVE_BIT(pMobIndex->res_flags, vector);

        else if (!str_prefix(word, "vul"))
          REMOVE_BIT(pMobIndex->vuln_flags, vector);

        else if (!str_prefix(word, "for"))
          REMOVE_BIT(pMobIndex->form, vector);

        else if (!str_prefix(word, "par"))
          REMOVE_BIT(pMobIndex->parts, vector);

        else {
          bug("Flag remove: flag not found.", 0);
          exit(1);
        }
      } else {
        if (is_encrypted) {
          if (letter > 8 && letter < 127) {
            letter = letter + enc_shift;
            if (letter > 126)
              letter = (letter - 126) + 8;
          }
        }
        ungetc(letter, fp);
        break;
      }
    }
    iHash = vnum % MAX_KEY_HASH;
    pMobIndex->next = mob_index_hash[iHash];
    mob_index_hash[iHash] = pMobIndex;
    top_mob_index++;
    top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;
    assign_area_vnum(vnum);
    kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL - 1)].number++;
  }
  return;
}

int GetObjCondition(char letter) {
  switch (letter) {
  case ('P'):
    return 100;
  case ('G'):
    return 90;
    break;
  case ('A'):
    return 75;
    break;
  case ('W'):
    return 50;
    break;
  case ('D'):
    return 25;
    break;
  case ('B'):
    return 10;
    break;
  case ('R'):
    return 1;
    break;
  default:
    return 100;
    break;
  }
}
/*
 * Snarf an obj section. new style
 */
void load_objects(FILE *fp) {
  if (!area_last) {
    bug("Load_objects: no #AREA seen yet.", 0);
    exit(1);
  }
  for (;;) {
    sh_int vnum;
    char letter;
    int iHash;
    letter = fread_letter(fp);
    if (letter != '#') {
      bug("Load_objects: # not found.", 0);
      exit(1);
    }
    vnum = fread_number(fp);
    if (vnum == 0)
      break;
    fBootDb = FALSE;
    if (get_obj_index(vnum) != NULL) {
      bug("Load_objects: vnum %d duplicated.", vnum);
      exit(1);
    }

    fBootDb = TRUE;
    pObjIndex = alloc_perm(sizeof(*pObjIndex));
    pObjIndex->vnum = vnum;
    pObjIndex->area = area_last;
    pObjIndex->reset_num = 0;
    newobjs++;
    pObjIndex->name = fread_string(fp);
    pObjIndex->short_descr = fread_string(fp);
    pObjIndex->description = fread_string(fp);
    pObjIndex->material = fread_string(fp);
    pObjIndex->timer = fread_number(fp);
    riname = fread_word(fp);
    pObjIndex->item_type = item_lookup(riname);

    if (area_last->version >= AREA_VER_EXTRA_FLAGS) {
      x = fread_number(fp);
      for (i = 0; i < x; i++) {
        pObjIndex->extra_flags[i] = fread_flag(fp);
      }
    } else {
      pObjIndex->extra_flags[0] = fread_flag(fp);
    }

    pObjIndex->wear_flags = fread_flag(fp);

    if (area_last->version >= AREA_VER_CLASSRACE) {
      pObjIndex->Class_flags = fread_flag(fp);
      pObjIndex->race_flags = fread_flag(fp);
      pObjIndex->clan_flags = fread_flag(fp);
    }

    if (area_last->version == AREA_VER_OBJTRIG) {
      pObjIndex->obj_trig_vnum[0] = fread_number(fp);
      for (i = 1; i < MAX_OBJ_TRIGS; i++)
        pObjIndex->obj_trig_vnum[i] = 0;
    } else if (area_last->version >= AREA_VER_OBJTRIG2) {
      for (i = 0; i < MAX_OBJ_TRIGS; i++)
        pObjIndex->obj_trig_vnum[i] = fread_number(fp);
    }

    pObjIndex->rarity = fread_number(fp);
    pObjIndex->string1 = fread_string(fp);
    pObjIndex->string2 = fread_string(fp);
    pObjIndex->string3 = fread_string(fp);
    pObjIndex->string4 = fread_string(fp);
    pObjIndex->value[0] = fread_flag(fp);
    switch (pObjIndex->item_type) {
    default:
      pObjIndex->value[1] = fread_flag(fp);
      pObjIndex->value[2] = fread_flag(fp);
      pObjIndex->value[3] = fread_flag(fp);
      pObjIndex->value[4] = fread_flag(fp);
      break;
    case ITEM_PILL:
    case ITEM_SCROLL:
    case ITEM_POTION:
      for (x = 1; x < 5; x++) {
        word = fread_word(fp);
        if ((tsn = atoi(word)) == 0 && word[0] != '0')
          tsn = skill_lookup(word);
        pObjIndex->value[x] = tsn;
      }
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      pObjIndex->value[1] = fread_flag(fp);
      pObjIndex->value[2] = fread_flag(fp);
      pObjIndex->value[3] = fread_flag(fp);
      word = fread_word(fp);
      if ((tsn = atoi(word)) == 0 && word[0] != '0')
        tsn = skill_lookup(word);
      pObjIndex->value[4] = tsn;
      break;
    case ITEM_CLAN_DONATION:

      pObjIndex->value[1] = fread_flag(fp);
      pObjIndex->value[1] = CLAN_BOGUS;
      pObjIndex->value[2] = fread_flag(fp);
      pObjIndex->value[3] = fread_flag(fp);
      pObjIndex->value[4] = fread_flag(fp);
      break;
    case ITEM_NEWCLANS_DBOX:

      word = fread_string(fp);
      pObjIndex->value[1] = clanname_to_slot(word);
      pObjIndex->value[2] = fread_flag(fp);
      pObjIndex->value[3] = fread_flag(fp);
      pObjIndex->value[4] = fread_flag(fp);
      break;
    }

// new line
    pObjIndex->value[5] = fread_flag(fp);

    if (pObjIndex->item_type == ITEM_CARD) {
      word = fread_word(fp);
      if ((tsn = atoi(word)) == 0 && word[0] != '0')
        tsn = skill_lookup(word);
      pObjIndex->value[6] = tsn;
      pObjIndex->value[7] = fread_flag(fp);
      pObjIndex->value[8] = fread_flag(fp);
      pObjIndex->value[9] = fread_flag(fp);
      pObjIndex->value[10] = fread_flag(fp);
      pObjIndex->value[11] = fread_flag(fp);
      pObjIndex->value[12] = fread_flag(fp);
    } else if (pObjIndex->item_type == ITEM_OBJ_TRAP ||
               pObjIndex->item_type == ITEM_ROOM_TRAP ||
               pObjIndex->item_type == ITEM_PORTAL_TRAP) {
      pObjIndex->value[6] = fread_flag(fp);
      pObjIndex->value[7] = fread_flag(fp);
      pObjIndex->value[8] = fread_flag(fp);
      pObjIndex->value[9] = fread_flag(fp);
      pObjIndex->value[10] = fread_flag(fp);
      pObjIndex->value[11] = fread_flag(fp);
      pObjIndex->value[12] = fread_flag(fp);

    } else {
      pObjIndex->value[6] = fread_flag(fp);
    }

    if (pObjIndex->item_type == ITEM_WEAPON) {
      if (pObjIndex->value[3] < 0 || pObjIndex->value[3] > 39) {
        pObjIndex->value[3] = 0;
        SET_BIT(area_last->area_flags, AREA_CHANGED);
      }
    }

    pObjIndex->level = fread_number(fp);
    pObjIndex->weight = fread_number(fp);
    pObjIndex->cost = fread_number(fp);

    /* condition */
    letter = fread_letter(fp);
    pObjIndex->condition = GetObjCondition(letter);

    for (;;) {
      char letter;
      letter = fread_letter(fp);
      if (letter == 'A') {
        AFFECT_DATA *paf;
        paf = alloc_perm(sizeof(*paf));
        paf->where = TO_OBJECT;
        paf->type = -1;
        paf->level = pObjIndex->level;
        paf->duration = -1;
        paf->location = fread_number(fp);
        paf->modifier = fread_number(fp);
        paf->bitvector = 0;
        paf->composition = FALSE;
        paf->comp_name = str_dup("");
        paf->next = pObjIndex->affected;
        pObjIndex->affected = paf;
        top_affect++;
      } else if (letter == 'S') {
        AFFECT_DATA *paf;
        paf = alloc_perm(sizeof(*paf));

        paf->where = fread_number(fp);
        paf->type = fread_number(fp);
        paf->level = fread_number(fp);
        paf->duration = fread_number(fp);
        paf->modifier = fread_number(fp);
        if (paf->where == TO_SKILL) {
          int sn = sn = exact_skill_lookup(fread_word(fp));
          if (sn < 0) {
            bug("Fread_obj_index: On item %d unknown skill.", pObjIndex->vnum);
            paf->location = 1;
          } else
            paf->location = sn;
        } else
          paf->location = fread_number(fp);
        paf->bitvector = fread_number(fp);
        paf->composition = FALSE;
        paf->comp_name = str_dup("");
        paf->next = pObjIndex->affected;
        pObjIndex->affected = paf;
        top_affect++;
      }

      else if (letter == 'F') {
        AFFECT_DATA *paf;
        paf = alloc_perm(sizeof(*paf));
        letter = fread_letter(fp);
        switch (letter) {
        case 'A':
          paf->where = TO_AFFECTS;
          break;
        case 'I':
          paf->where = TO_IMMUNE;
          break;
        case 'R':
          paf->where = TO_RESIST;
          break;
        case 'V':
          paf->where = TO_VULN;
          break;
        default:
          bug("Load_objects: Bad where on flag set.", 0);
          exit(1);
        }
        paf->type = -1;
        paf->level = pObjIndex->level;
        paf->duration = -1;
        paf->location = fread_number(fp);
        paf->modifier = fread_number(fp);
        paf->bitvector = fread_flag(fp);
        paf->composition = FALSE;
        paf->comp_name = str_dup("");
        paf->next = pObjIndex->affected;
        pObjIndex->affected = paf;
        top_affect++;
      } else if (letter == 'E') {
        EXTRA_DESCR_DATA *ed;
        ed = alloc_perm(sizeof(*ed));
        ed->keyword = fread_string(fp);
        ed->description = fread_string(fp);
        ed->next = pObjIndex->extra_descr;
        pObjIndex->extra_descr = ed;
        top_ed++;
      } else {
        if (is_encrypted) {
          if (letter > 8 && letter < 127) {
            letter = letter + enc_shift;
            if (letter > 126)
              letter = (letter - 126) + 8;
          }
        }
        ungetc(letter, fp);
        break;
      }
    }
    iHash = vnum % MAX_KEY_HASH;
    pObjIndex->next = obj_index_hash[iHash];
    obj_index_hash[iHash] = pObjIndex;
    top_obj_index++;
    top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;
    assign_area_vnum(vnum);
  }
  return;
}
