#include "db.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"
#include "lookup.h"
#include "merc.h"
#include "music.h"
#include "olc.h"
#include "recycle.h"
#include "tables.h"
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#if !defined(macintosh)
extern int _filbuf args((FILE *));

#endif /*  */
extern bool newlock;
extern bool wizlock;
extern bool is_linkloading;
extern const char *dir_name[];
void load_player_box args((OBJ_DATA * pObj));

// Iblis 8/2/03 - For Fishing
void make_hotspots args((void));
void load_topfishers args((void));

void fix_stores args((void));
void load_trees args((void));

CHAR_DATA *load_char_obj2 args((char *name));
void ppurge args((CHAR_DATA * ch, CHAR_DATA *victim));

/* externals for counting purposes */
extern int getbit args((char *explored, int index));
extern OBJ_DATA *obj_free;
extern CHAR_DATA *char_free;
extern DESCRIPTOR_DATA *descriptor_free;
extern PC_DATA *pcdata_free;
extern AFFECT_DATA *affect_free;
extern NOTE_DATA *note_free;

void free_ot args((OBJ_TRIG * ot));

/*
 * Globals.
 */
bool exploration_tracking = FALSE;
bool is_encrypted = FALSE;
int enc_shift;
HELP_DATA *help_first;
HELP_DATA *help_last;
SHOP_DATA *shop_first;
SHOP_DATA *shop_last;
char bug_buf[2 * MAX_INPUT_LENGTH];
CHAR_DATA *char_list;
FAKE_DATA *fake_list = NULL;
char *help_greeting;
char *help_agreeting;
char log_buf[2 * MAX_INPUT_LENGTH];
KILL_DATA kill_table[MAX_LEVEL];
OBJ_DATA *corpse_list;

// NOTE_DATA *           note_list;
OBJ_DATA *object_list;
TIME_INFO_DATA time_info;
WEATHER_DATA weather_info;
AUCTION_DATA *auction;
SCENT_DATA *scents;
MEMORY_DATA *memories;
int reboot_counter;
int shutdown_counter;

// OBJ_TRIG_DATA *obj_trig_list;
OBJ_TRIG *ot_list;
OBJ_TRIG *giant_ot_list;
OBJ_TRIG *on_tick_ot_list;
OBJ_TRIG *new_ot args((int vnum));
// Iblis 1/02/04 Needed for setting objects properly
CHAR_DATA *GLOBAL_ot_ch;
OBJ_DATA *GLOBAL_ot_obj;

// Iblis 1/19/04 Needed for Top Gamestats
unsigned long TGS_Stats[MAX_GAMESTAT][MAX_PEOPLE_STAT];
char *TGS_Names[MAX_GAMESTAT][MAX_PEOPLE_STAT];

void load_player_corpses args(());

short gsn_foraging;
short gsn_mask;
short gsn_exorcism;
short gsn_elusiveness;
short gsn_retreat;
short gsn_swim;
short gsn_summon_mount;
short gsn_leadership;
short gsn_doorbash;
short gsn_throwing;
short gsn_heavenly_cloak;
short gsn_blindfighting;
short gsn_layhands;
short gsn_conceal;
short gsn_stalk;
short gsn_sweep;
short gsn_circle;
short gsn_charge;
short gsn_compose;
short gsn_track;
short gsn_butcher;
short gsn_endurance;
short gsn_entangle;
short gsn_backstab;
short gsn_shadowslash;
short gsn_fishing;
short gsn_camping;
short gsn_hone;
short gsn_fisticuffery;
short gsn_riding;
short gsn_tame;
short gsn_kai;
short gsn_stun;
short gsn_uppercut;
short gsn_dual_wield;
short gsn_dodge;
short gsn_envenom;
short gsn_hide;
short gsn_camouflage;
short gsn_peek;
short gsn_pick_lock;
short gsn_brew;
short gsn_scribe;
short gsn_sneak;
short gsn_steal;
short gsn_palm;
short gsn_sap;
short gsn_eviscerate;
short gsn_disarm;
short gsn_enhanced_damage;
short gsn_critical_strike;
short gsn_kick;
short gsn_parry;
short gsn_rescue;
short gsn_second_attack;
short gsn_third_attack;

// IBLIS 5/31/03 Monk Skills
short gsn_split_vision;
short gsn_sense_life;
short gsn_dirty_tactics;
short gsn_counter;
short gsn_block;
short gsn_knock;
short gsn_feign;
short gsn_prevent_escape;
short gsn_dive;
short gsn_toss;
short gsn_the_arts;
short gsn_strike;
short gsn_whirlwind;
short gsn_fists_of_fury;
short gsn_bandage;
short gsn_awareness;
short gsn_hear_the_wind;

short gsn_invention;
short gsn_falconry;
short gsn_repulse;
short gsn_tempt;
short gsn_archery;
short gsn_summon_beast;
short gsn_pounce;
short gsn_disorientation;
short gsn_aquatitus;
short gsn_jurgnation;
short gsn_jalknation;
short gsn_fear;
short gsn_illusion;
short gsn_suck;
short gsn_likobe;
short gsn_fireshield;

// Iblis 7/22/03 TRADE SKILLS
short gsn_woodworking;
short gsn_fletchery;
short gsn_lumberjacking;
short gsn_milling;
short gsn_forestry;

short gsn_blindness;
short gsn_charm_person;
short gsn_curse;
short gsn_invis;
short gsn_mass_invis;
short gsn_mirror;
short gsn_assassinate;
short gsn_poison;
short gsn_plague;
short gsn_sleep;
short gsn_sanctuary;
short gsn_fly;
short gsn_axe;
short gsn_dagger;
short gsn_flail;
short gsn_mace;
short gsn_polearm;
short gsn_shield_block;
short gsn_staff;
short gsn_sword;
short gsn_bash;
short gsn_berserk;
short gsn_dirt;
short gsn_hand_to_hand;
short gsn_trip;
short gsn_instruments;
short gsn_redirect;
short gsn_tripwire;
short gsn_trample;
short gsn_fast_healing;
short gsn_haggle;
short gsn_influence;
short gsn_lore;
short gsn_meditation;
short gsn_scrolls;
short gsn_staves;
short gsn_wands;
short gsn_wall_thorns;
short gsn_the_piper;
short gsn_invocation;
short gsn_unholy_burst;
short gsn_hellscape;

short gsn_aggression;
short gsn_spider_vine;
short gsn_bark_skin;

// Necro Stuff
short gsn_leeching;
short gsn_death_ward;
short gsn_draw;
short gsn_sense_death;
short gsn_embalm;
short gsn_feign_death;
short gsn_corrupt;
short gsn_feast;
short gsn_hellbreath;
short gsn_feed_death;
short gsn_corpse_carving;
short gsn_unsummon;
short gsn_astral_body;
short gsn_soul_link;
short gsn_death_shroud;
short gsn_pox;
short gsn_immunity;
short gsn_skeletal_mutation;
short gsn_intoxicate;
short gsn_brain_blisters;
short gsn_aura_rot;
short gsn_vomit;
short gsn_atrophy;
short gsn_greymantle;
short gsn_festering_boils;
short gsn_clasp;

// Lich Stuff
short gsn_defile;
short gsn_despoil;
short gsn_contaminate;

// Litan stuff
short gsn_flare;

// Nerix stuff
short gsn_lightningbreath;
short gsn_swoop;

// Nidae stuff
short gsn_bubble;
short gsn_aquatic_sense;
short gsn_spear;
short gsn_locate;
short gsn_call_storm;
short gsn_transform;
short gsn_dehydration;
short gsn_find_land;

short gsn_create_traps;
short gsn_avoid_traps;
short gsn_disarm_traps;
short gsn_detect_traps;
short gsn_trapdamage;

// Chaos Jester stuff
short gsn_flicker;
short gsn_warp_touch;
short gsn_mad_rush;
short gsn_coma;
short gsn_vanish;
short gsn_persuade;
short gsn_mad_dance;
short gsn_fire_dance;
short gsn_locust_wings;
short gsn_torture;
short gsn_gravity;
short gsn_escape;
short gsn_meteor_swarm;
short gsn_scramble;
short gsn_death_spasms;
short gsn_flick;
short gsn_shuffle;
short gsn_dice;
short gsn_lucky_throw;
short gsn_evil_twin;

short gsn_larva;
// Swarm stuff
// short gsn_smother;
// short gsn_attack;

// Druid skills
short gsn_insect_plague;
short gsn_clarity;
short gsn_nature_sense;
short gsn_shillelagh;
short gsn_animal_empathy;
short gsn_animal_friendship;
short gsn_satiate;
short gsn_wild_invigoration;
short gsn_primeval_guidance;
short gsn_shapechange;

/*
 * Locals.
 */
MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
char *string_hash[MAX_KEY_HASH];
AREA_DATA *area_first;
AREA_DATA *area_last;
char *string_space;
char *top_string;
char str_empty[1];
int top_affect;
int top_area;
int top_ed;
int top_exit;
int top_help;
int top_mob_index;
int top_obj_index;
int top_reset;
int top_room;
int top_shop;
int top_vnum_room; /* OLC */
int top_vnum_mob;  /* OLC */
int top_vnum_obj;  /* OLC */
int mobile_count = 0;
int newmobs = 0;
int newobjs = 0;

/*
 * Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing.
 */
#define MAX_STRING 14388608
#define MAX_PERM_BLOCK 131072
#define MAX_MEM_LIST 13
void *rgFreeList[MAX_MEM_LIST];
const int rgSizeList[MAX_MEM_LIST] = {16,         32,    64,    128,  256,
                                      1024,       2048,  4096,  8192, 16384,
                                      32768 - 64, 65536, 131072};
int nAllocString;
int sAllocString;
int nAllocPerm;
int sAllocPerm;
// long nOverFlowString;
// long sOverFlowString;

/*
 * Semi-locals.
 */
bool fBootDb;
FILE *fpArea;
char strArea[MAX_INPUT_LENGTH];
// char str_empty[1];
// char *string_space;
// char *top_string;
// bool Full;
// char *str_dup (const char *);
// void free_string (char ** );
// char *fread_string (FILE *);
// void init_string_space(void);

/*
 * Local booting procedures.
 */
void init_mm args((void));
void load_area args((FILE * fp));
void load_helps args((FILE * fp));
void load_mobiles args((FILE * fp));
void load_objects args((FILE * fp));
void load_resets args((FILE * fp));
void load_rooms args((FILE * fp));
void load_shops args((FILE * fp));
void load_socials args((FILE * fp));
void load_specials args((FILE * fp));
void load_notes args((void));
void load_bans args((void));
void load_bounties args((void));
void load_inventions args((void));
void load_rune_cards args((void));
void load_cardcount args((void));
void fix_exits args((void));
void reset_area args((AREA_DATA * pArea));

/*
 * Big mama top level function.
 */
void boot_db(void) {
  FILE *nfp;
  char *cptr;

  /*
   * Init some data space stuff.
   */
  {
    if ((string_space = calloc(1, MAX_STRING)) == NULL) {
      bug("Boot_db: can't alloc %d string space.", MAX_STRING);
      exit(1);
    }
    top_string = string_space;
    //    init_string_space();
    fBootDb = TRUE;
  }

  /*
   * Init random number generator.
   */
  { init_mm(); }

  /*
   * Set time and weather.
   */
  {
    long lhour, lday, lmonth;
    lhour = (current_time - 650336715) / (PULSE_TICK / PULSE_PER_SECOND);
    time_info.hour = lhour % 24;
    lday = lhour / 24;
    time_info.day = lday % 35;
    lmonth = lday / 35;
    time_info.month = lmonth % 17;
    time_info.year = lmonth / 17;
    if (time_info.hour < 5)
      weather_info.sunlight = SUN_DARK;

    else if (time_info.hour < 6)
      weather_info.sunlight = SUN_RISE;

    else if (time_info.hour < 19)
      weather_info.sunlight = SUN_LIGHT;

    else if (time_info.hour < 20)
      weather_info.sunlight = SUN_SET;

    else
      weather_info.sunlight = SUN_DARK;
    weather_info.change = 0;
    weather_info.mmhg = 960;
    if (time_info.month >= 7 && time_info.month <= 12)
      weather_info.mmhg += number_range(1, 50);

    else
      weather_info.mmhg += number_range(1, 80);
    if (weather_info.mmhg <= 980)
      weather_info.sky = SKY_LIGHTNING;

    else if (weather_info.mmhg <= 1000)
      weather_info.sky = SKY_RAINING;

    else if (weather_info.mmhg <= 1020)
      weather_info.sky = SKY_CLOUDY;

    else
      weather_info.sky = SKY_CLOUDLESS;
  }

  // load the clans, slot zero in the clan table is reserved for the
  // special clan 'clan_bogus' which is automatically loaded/initalized
  // through this call
  load_clans();
  auction = (AUCTION_DATA *)malloc(sizeof(AUCTION_DATA)); /* DOH!!! */
  if (auction == NULL) {
    bug("malloc'ing AUCTION_DATA didn't give %d bytes", sizeof(AUCTION_DATA));
    exit(1);
  }
  auction->item = NULL; /* nothing is being sold */
  scents = NULL;
  memories = NULL;
  reboot_counter = -1;
  shutdown_counter = -1;

  /*
   * Assign gsn's for skills which have them.
   */
  {
    int sn;
    for (sn = 0; sn < MAX_SKILL; sn++) {
      if (skill_table[sn].pgsn != NULL)
        *skill_table[sn].pgsn = sn;
    }
  }
  if ((nfp = fopen("newlock", "r")) != NULL) {
    newlock = TRUE;
    fclose(nfp);
  } else
    newlock = FALSE;
  if ((nfp = fopen("wizlock", "r")) != NULL) {
    wizlock = TRUE;
    fclose(nfp);
  } else
    wizlock = FALSE;

  /*
   * Read in all the area files.
   */
  {
    FILE *fpList;
    if ((fpList = fopen(AREA_LIST, "r")) == NULL) {
      perror(AREA_LIST);
      exit(1);
    }
    for (;;) {
      strcpy(strArea, fread_word(fpList));
      if (strArea[0] == '$')
        break;
      if (strArea[0] == '-') {
        fpArea = stdin;
      } else {
        cptr = strchr(strArea, '.');
        if (!str_cmp(cptr, ".enc")) {
          is_encrypted = TRUE;
          enc_shift = 33;
        }
        char tfbuf[MAX_STRING_LENGTH];
        sprintf(tfbuf, "Opening %s", strArea);
        log_string(tfbuf);
        if ((fpArea = fopen(strArea, "r")) == NULL) {
          perror(strArea);
          exit(1);
        }
      }
      for (;;) {
        char *word;
        if (fread_letter(fpArea) != '#') {
          char tcbuf[MAX_STRING_LENGTH];
          sprintf(tcbuf, "A bug! No # in area: %s", strArea);
          log_string(tcbuf);
          exit(1);
        }
        word = fread_word(fpArea);
        if (word[0] == '$')
          break;

        else if (!str_cmp(word, "AREA"))
          load_area(fpArea);

        else if (!str_cmp(word, "HELPS"))
          load_helps(fpArea);

        else if (!str_cmp(word, "MOBILES"))
          load_mobiles(fpArea);

        else if (!str_cmp(word, "OBJECTS"))
          load_objects(fpArea);

        else if (!str_cmp(word, "RESETS"))
          load_resets(fpArea);

        else if (!str_cmp(word, "ROOMS"))
          load_rooms(fpArea);

        else if (!str_cmp(word, "SHOPS"))
          load_shops(fpArea);

        else if (!str_cmp(word, "SOCIALS"))
          load_socials(fpArea);

        else if (!str_cmp(word, "SPECIALS"))
          load_specials(fpArea);
        else if (!str_cmp(word, "OT"))
          load_obj_trigs(fpArea);
        else {
          bug("Boot_db: bad section name.", 0);
          exit(1);
        }
      }
      is_encrypted = FALSE;
      if (fpArea != stdin)
        fclose(fpArea);
      fpArea = NULL;
    }
    fclose(fpList);
  }

  /*
   * Fix up exits.
   * Declare db booting over.
   * Reset all areas once.
   * Load up the songs, notes and ban files.
   */
  {
    system("ls corpses > corpses.txt");
#ifdef VERBOSE_BOOT
    log_string("boot_db() - fixing exits.");
#endif
    fix_exits();
    fBootDb = FALSE;
#ifdef VERBOSE_BOOT
    log_string("boot_db() - loading transports");
#endif
    load_ctransports();
#ifdef VERBOSE_BOOT
    log_string("boot_db() - area update");
#endif
    area_update();
#ifdef VERBOSE_BOOT
    log_string("boot_db() - loading notes.");
#endif
    load_notes();
#ifdef VERBOSE_BOOT
    log_string("boot_db() - loading bans.");
#endif
    load_bans();
#ifdef VERBOSE_BOOT
    log_string("boot_db() - loading bounties.");
#endif
    load_bounties();
#ifdef VERBOSE_BOOT
    log_string("boot_db() - loading inventions.");
#endif
    load_inventions();
#ifdef VERBOSE_BOOT
    log_string("boot_db() - creating hotspots.");
#endif
    make_hotspots();
#ifdef VERBOSE_BOOT
    log_string("boot_db() - loading top fishers.");
#endif
    load_topfishers();
#ifdef VERBOSE_BOOT
    log_string("boot_db() - loading Top Stats.");
#endif
    load_statlist();
#ifdef VERBOSE_BOOT
    log_string("boot_db() - loading corpses.");
#endif
    load_player_corpses();
#ifdef VERBOSE_BOOT
    log_string("boot_db() - loading trees.");
#endif
    load_trees();
#ifdef VERBOSE_BOOT
    log_string("boot_db() - loading rune cards.");
#endif
    load_rune_cards();
#ifdef VERBOSE_BOOT
    log_string("boot_db() - loading card count.");
#endif
    load_cardcount();
#ifdef VERBOSE_BOOT
    log_string("boot_db() - fixing stores.");
#endif
    fix_stores();
#ifdef VERBOSE_BOOT
    log_string("boot_db() - done");
#endif
  }
  remove_clan_symbols();
  return;
}

/*
 * Snarf an 'area' header line.
 */
void load_area(FILE *fp) {
  AREA_DATA *pArea;

#ifdef VERBOSE_BOOT
  char buf[MAX_STRING_LENGTH];

#endif /*  */
  char ecredits[MAX_STRING_LENGTH];
  char *cptr = NULL;
  char *cptr2 = NULL;
  long ident;

#ifdef VERBOSE_BOOT
  log_string("load_area()");

#endif /*  */
  pArea = alloc_perm(sizeof(*pArea));
  //  pArea = reinterpret_cast<AREA_DATA *>(alloc_perm (sizeof (*pArea)));
  pArea->reset_first = NULL;
  pArea->reset_last = NULL;
  pArea->ot_first = NULL;
  pArea->ot_last = NULL;
  pArea->version = AREA_VER_DEFAULT;

  // Akamai 4/30/99 - Support for race/Class specific objects
  // Either we have a file name or a version number. The way to know if
  // we have both is the first one is a version number - so check it.
  cptr = fread_string(fp);
  if (is_number(cptr)) {
    pArea->version = atoi(cptr);
    pArea->file_name = fread_string(fp);
    cptr = NULL;

#ifdef VERBOSE_BOOT
    sprintf(buf, "load_area: version tag: %d", pArea->version);
    log_string(buf);

#endif /*  */
  } else {
    pArea->file_name = cptr;
    cptr = NULL;

#ifdef VERBOSE_BOOT
    sprintf(buf, "load_area: *no* version tag (default): %d", pArea->version);
    log_string(buf);

#endif /*  */
  }
  pArea->name = fread_string(fp);
  pArea->area_flags = AREA_LOADING;   /* OLC */
  pArea->vnum = top_area;             /* OLC */
  pArea->filename = str_dup(strArea); /* OLC */
  pArea->creator = str_dup("Exodus");
  pArea->llev = 1;
  pArea->ulev = 5;
  pArea->credits = fread_string(fp);
  pArea->help = fread_string(fp);
  pArea->min_vnum = fread_number(fp);
  pArea->max_vnum = fread_number(fp);
  pArea->points = fread_number(fp);
  if (pArea->version < AREA_VER_MD_AF) {
    if (IS_SET(pArea->points, POINTS_IMP_ONLY)) {
      REMOVE_BIT(pArea->points, POINTS_IMP_ONLY);
      SET_BIT(pArea->area_flags, AREA_IMP_ONLY);
    }
    if (IS_SET(pArea->points, POINTS_NO_QUIT)) {
      REMOVE_BIT(pArea->points, POINTS_NO_QUIT);
      SET_BIT(pArea->area_flags, AREA_NO_QUIT);
    }
    if (IS_SET(pArea->points, POINTS_NO_REPOP_WIA)) {
      REMOVE_BIT(pArea->points, POINTS_NO_REPOP_WIA);
      SET_BIT(pArea->area_flags, AREA_NO_REPOP_WIA);
    }
    if (IS_SET(pArea->points, POINTS_NO_TREE)) {
      REMOVE_BIT(pArea->points, POINTS_NO_TREE);
      SET_BIT(pArea->area_flags, AREA_NO_TREE);
    }
  } else
    pArea->area_flags = fread_flag(fp);

#ifdef VERBOSE_BOOT
  sprintf(buf, "load_area: %s - file %s", pArea->name, pArea->filename);
  log_string(buf);

#endif /*  */

  // areas store clans as numbers, so use the permanent clan id
  ident = fread_number(fp);
  pArea->clan = clanident_to_slot(ident);

#ifdef VERBOSE_BOOT
  if (ident < 4000) {
    sprintf(buf, "load_area: old clanident: %ld  slot: %d", ident, pArea->clan);
    log_string(buf);
  } else {
    sprintf(buf, "load_area: new clanident: %ld  slot: %d", ident, pArea->clan);
    log_string(buf);
  }

#endif /*  */
  pArea->noclan = fread_number(fp);
  pArea->construct = fread_number(fp);
  pArea->helper = fread_string(fp);
  pArea->lvnum = pArea->min_vnum; /* OLC */
  pArea->uvnum = pArea->max_vnum; /* OLC */
  pArea->age = 33;
  pArea->nplayer = 0;
  pArea->empty = FALSE;
  strcpy(ecredits, pArea->credits);
  cptr = strchr(ecredits, '{');
  if (cptr != NULL) {
    cptr++;
    if (*cptr == ' ')
      cptr++;
    cptr2 = strchr(cptr, ' ');
    if (cptr2 != NULL) {
      *cptr2 = '\0';
      pArea->llev = atoi(cptr);
      *cptr2 = ' ';
      cptr = strchr(cptr2, '}');
      if (cptr != NULL) {
        *cptr = '\0';
        pArea->ulev = atoi(cptr2);
        *cptr = ' ';

        do {
          cptr++;
        } while (isspace(*cptr));
        cptr2 = strchr(cptr, '\t');
        if (cptr2 == NULL)
          cptr2 = strchr(cptr, ' ');
        if (cptr2 != NULL) {
          *cptr2 = '\0';
          pArea->creator = str_dup(cptr);
        }
      }
    }
  }
  if (area_first == NULL) {
    area_first = pArea;
  }
  if (area_last != NULL) {
    area_last->next = pArea;
    REMOVE_BIT(area_last->area_flags, AREA_LOADING); /* OLC */
  }
  area_last = pArea;
  pArea->next = NULL;
  top_area++;

#ifdef VERBOSE_BOOT
  sprintf(buf, "load_area: finished loading %s", pArea->name);
  log_string(buf);

#endif /*  */
}

/*
 * OLC
 * Use these macros to load any new area formats that you choose to
 * support on your MUD.  See the new_load_area format below for
 * a short example.
 */
#if defined(KEY)
#undef KEY
#endif /*  */

#define KEY(literal, field, value)                                             \
  if (!str_cmp(word, literal)) {                                               \
    field = value;                                                             \
    fMatch = TRUE;                                                             \
    break;                                                                     \
  }

#define SKEY(string, field)                                                    \
  if (!str_cmp(word, string)) {                                                \
    free_string(field);                                                        \
    field = fread_string(fp);                                                  \
    fMatch = TRUE;                                                             \
    break;                                                                     \
  }

/*
 * Sets vnum range for area using OLC protection features.
 */
void assign_area_vnum(int vnum) {

  /*    if ( area_last->lvnum == 0 || area_last->uvnum == 0 )
     area_last->lvnum = area_last->uvnum = vnum;
     if ( vnum != URANGE( area_last->lvnum, vnum, area_last->uvnum ) )
     if ( vnum < area_last->lvnum )
     area_last->lvnum = vnum;
     else
     area_last->uvnum = vnum; */
  return;
}

/* load_helps reads help sections from a file and organizes them into a linked
   list. It expects the file to be formatted with a help level, followed by a
   keyword, and then the help text itself for each help entry. The list is
   terminated by a keyword starting with a '$' character. */
void load_helps(FILE *fp) {
  HELP_DATA *pHelp; // Pointer to hold each new help data
  int level;        // To store the help level read from the file
  char *keyword;    // To store the keyword read from the file

  // Continuously read help entries until a termination keyword is encountered
  while (1) {
    level = fread_number(fp);   // Read the level of the help entry
    keyword = fread_string(fp); // Read the keyword of the help entry

    // Check for termination keyword (indicated by starting with '$')
    if (keyword[0] == '$') {
      free_string(keyword); // Clean up the allocated keyword string
      break;                // Exit the loop, ending the read process
    }

    // Allocate memory for a new help data structure
    pHelp = (HELP_DATA *)alloc_perm(sizeof(*pHelp));
    if (!pHelp) {
      perror("Memory allocation failed for HELP_DATA");
      exit(EXIT_FAILURE); // Exit if memory allocation fails
    }

    // Set the level and keyword for the new help entry
    pHelp->level = level;
    pHelp->keyword =
        keyword; // Previously allocated keyword string is directly assigned

    // Read and assign the help text for the new help entry
    pHelp->text = fread_string(fp);

    // Special handling for predefined keywords
    if (!str_cmp(pHelp->keyword, "greeting"))
      help_greeting = pHelp->text; // Assign greeting text
    if (!str_cmp(pHelp->keyword, "agreeting"))
      help_agreeting = pHelp->text; // Assign alternative greeting text

    // Link the new help entry into the list of helps
    if (!help_first) // If this is the first help, set it as the start of the
                     // list
      help_first = pHelp;
    if (help_last) // If there's a last help, link the new help to the end of
                   // the list
      help_last->next = pHelp;

    help_last = pHelp;  // Update the last help pointer to the new help
    pHelp->next = NULL; // Ensure the new help indicates it's currently the last
                        // in the list
    top_help++;         // Increment the total number of helps loaded
  }
}

/*
 * Adds a reset to a room.  OLC
 * Similar to add_reset in olc.c
 */
void new_reset(ROOM_INDEX_DATA *pR, RESET_DATA *pReset) {
  RESET_DATA *pr;
  if (!pR)
    return;
  pr = pR->reset_last;
  if (!pr) {
    pR->reset_first = pReset;
    pR->reset_last = pReset;
  } else {
    pR->reset_last->next = pReset;
    pR->reset_last = pReset;
    pR->reset_last->next = NULL;
  }
  top_reset++;
  return;
}

void load_resets(FILE *fp) {
  RESET_DATA *pReset;
  char buf[MAX_STRING_LENGTH];

#ifdef VERBOSE_BOOT
  log_string("load_resets()");

#endif /*  */
  if (area_last == NULL) {
    bug("Load_resets: no #AREA seen yet.", 0);
    exit(1);
  }
  for (;;) {
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit;
    char letter;
    OBJ_INDEX_DATA *temp_index;
    if ((letter = fread_letter(fp)) == 'S')
      break;
    if (letter == '*') {
      fread_to_eol(fp);
      continue;
    }
    pReset = alloc_perm(sizeof(*pReset));
    pReset->command = letter;
    fread_number(fp);
    pReset->arg1 = fread_number(fp);
    pReset->arg2 = fread_number(fp);
    pReset->arg3 = (letter == 'G' || letter == 'R') ? 0 : fread_number(fp);
    pReset->arg4 = (letter == 'P' || letter == 'M' || letter == 'O')
                       ? fread_number(fp)
                       : 0;
    fread_to_eol(fp);

#ifdef VERBOSE_BOOT

#endif /*  */
    switch (letter) {
    default:
      bug("Load_resets: bad command '%c'.", letter);
      exit(1);
      break;
    case 'M':
      get_mob_index(pReset->arg1);
      get_room_index(pReset->arg3);
      break;
    case 'O':
      temp_index = get_obj_index(pReset->arg1);
      temp_index->reset_num++;
      get_room_index(pReset->arg3);
      break;
    case 'P':
      temp_index = get_obj_index(pReset->arg1);
      temp_index->reset_num++;
      get_obj_index(pReset->arg3);
      break;
    case 'G':
    case 'E':
      temp_index = get_obj_index(pReset->arg1);
      temp_index->reset_num++;
      break;
    case 'D':
      pRoomIndex = get_room_index(pReset->arg1);
      if ((pReset->arg2 < 0) || (pReset->arg2 > 5) ||
          ((pexit = pRoomIndex->exit[pReset->arg2]) == NULL) ||
          !IS_SET(pexit->exit_info, EX_ISDOOR)) {
        sprintf(buf, "Load_resets: Room '%d': exit %s not door.",
                pRoomIndex->vnum, dir_name[pReset->arg2]);
        bug(buf, 0);
      }
      if (pReset->arg3 < 0 || pReset->arg3 > 2) {
        bug("Load_resets: 'D': bad 'locks': %d.", pReset->arg3);
        exit(1);
      }
      break;
    case 'R':
      pRoomIndex = get_room_index(pReset->arg1);
      if (pReset->arg2 < 0 || pReset->arg2 > 6) {
        bug("Load_resets: 'R': bad exit %d.", pReset->arg2);

        /* exit( 1 );  */
      }
      break;
    } /* switch(letter) */
    if (area_last->reset_first == NULL) {
      area_last->reset_first = pReset;
    }
    if (area_last->reset_last != NULL) {
      area_last->reset_last->next = pReset;
    }
    area_last->reset_last = pReset;
    pReset->next = NULL;
    top_reset++;
  } /* for loop */
}
inline int fread_numberi(FILE *fp) {
  int number;
  bool sign;
  char c, c2;

  do {
    c2 = c = getc(fp);
  } while (isspace(c));
  number = 0;
  sign = FALSE;
  if (c == '+') {
    c2 = c = getc(fp);
  } else if (c == '-') {
    sign = TRUE;
    c2 = c = getc(fp);
  }
  while (isdigit(c)) {
    number = number * 10 + c - '0';
    c2 = c = getc(fp);
  }
  if (sign)
    number = 0 - number;
  if (c == '|')
    number += fread_number(fp);

  else if (c != ' ') {
    ungetc(c2, fp);
  }
  return number;
}
inline long fread_flagi(FILE *fp) {
  int number;
  char c, c2;
  bool negative = FALSE;

  do {
    c2 = c = getc(fp);
  } while (isspace(c));
  if (c == '-') {
    negative = TRUE;
    c2 = c = getc(fp);
  }
  number = 0;
  if (!isdigit(c) && c != '-') { /* ROM OLC */
    while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
      number += flag_convert(c);
      c2 = c = getc(fp);
    }
  }
  if (c == '-') { /* ROM OLC */
    number = fread_number(fp);
    return -number;
  }
  if (!isdigit(c)) {
    while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
      number += flag_convert(c);
      c2 = c = getc(fp);
    }
  }
  while (isdigit(c)) {
    number = number * 10 + c - '0';
    c2 = c = getc(fp);
  }
  if (c == '|')
    number += fread_flag(fp);

  else if (c != ' ') {
    ungetc(c2, fp);
  }
  if (negative)
    return -1 * number;
  return number;
}
inline char fread_letteri(FILE *fp) {
  char c;

  do {
    c = getc(fp);
  } while (isspace(c));
  return c;
}

void load_rooms(FILE *fp) {
  ROOM_INDEX_DATA *pRoomIndex;

#ifdef VERBOSE_BOOT
  log_string("load_rooms()");

#endif /*  */
  if (area_last == NULL) {
    bug("Load_resets: no #AREA seen yet.", 0);
    exit(1);
  }
  for (;;) {
    int vnum;
    char letter;
    int door;
    int iHash;
    letter = fread_letter(fp);
    if (letter != '#') {
      bug("Load_rooms: # not found.", 0);
      exit(1);
    }
    vnum = fread_number(fp);
    if (vnum == 0)
      break;
    fBootDb = FALSE;
    if (get_room_index(vnum) != NULL) {
      bug("Load_rooms: vnum %d duplicated.", vnum);
      exit(1);
    }
    fBootDb = TRUE;
    pRoomIndex = alloc_perm(sizeof(*pRoomIndex));
    //      pRoomIndex = reinterpret_cast<ROOM_INDEX_DATA
    //      *>(alloc_perm(sizeof(*pRoomIndex)));
    pRoomIndex->owner = str_dup("");
    pRoomIndex->people = NULL;
    pRoomIndex->contents = NULL;
    pRoomIndex->extra_descr = NULL;
    pRoomIndex->scents = NULL;
    pRoomIndex->eplprog = NULL;
    pRoomIndex->epl_filename = NULL;
    pRoomIndex->sink_msg = NULL;
    pRoomIndex->sink_msg_others = NULL;
    pRoomIndex->sink_warning = NULL;
    pRoomIndex->sink_timer = 0;
    pRoomIndex->sink_dest = 4200;
    pRoomIndex->area = area_last;
    pRoomIndex->vnum = vnum;

    /*	pRoomIndex->tp_level            = 0;
      pRoomIndex->tp_exp              = 0;
      pRoomIndex->tp_msg              = str_dup("");  */
    pRoomIndex->tp_level = fread_number(fp);
    pRoomIndex->tp_exp = fread_number(fp);
    pRoomIndex->tp_msg = fread_string(fp);
    pRoomIndex->enter_msg = fread_string(fp);
    pRoomIndex->exit_msg = fread_string(fp);
    pRoomIndex->Class_flags = fread_number(fp);
    pRoomIndex->race_flags = fread_number(fp);
    pRoomIndex->max_level = fread_number(fp);
    pRoomIndex->name = fread_string(fp);
    pRoomIndex->description = fread_string(fp);

    /* Area number */ fread_number(fp);
    pRoomIndex->room_flags = fread_flag(fp);
    REMOVE_BIT(pRoomIndex->room_flags, ROOM_TRANSPORT);
    if (IS_SET(pRoomIndex->room_flags, ROOM_ARENA) &&
        strcmp(pRoomIndex->area->filename, "arena.enc") != 0)
      REMOVE_BIT(pRoomIndex->room_flags, ROOM_ARENA);
    pRoomIndex->sector_type = fread_number(fp);
    pRoomIndex->max_in_room = fread_number(fp);
    pRoomIndex->max_message = fread_string(fp);
    pRoomIndex->light = 0;
    for (door = 0; door <= 5; door++)
      pRoomIndex->exit[door] = NULL;

    /* defaults */
    if (strcmp(pRoomIndex->area->filename, "haven.enc") == 0) {
      pRoomIndex->sector_type = SECT_INSIDE;
      pRoomIndex->heal_rate = 115;
      pRoomIndex->mana_rate = 115;
      pRoomIndex->room_flags = ROOM_SAFE | ROOM_NO_MOB | ROOM_NO_RECALL;
    } else {
      pRoomIndex->heal_rate = 100;
      pRoomIndex->mana_rate = 100;
    }
    for (;;) {
      letter = fread_letter(fp);
      if (letter == 'S')
        break;
      if (letter == 'H') /* healing room */
        pRoomIndex->heal_rate = fread_number(fp);

      else if (letter == 'I') // no particular reason for I ;)
        // Adeon 6/30/03 -- read in sinking room data
        pRoomIndex->room_flags2 = fread_flag(fp);

      else if (letter == 'J')

        pRoomIndex->sink_msg = fread_string(fp);

      else if (letter == 'Q')
        pRoomIndex->sink_msg_others = fread_string(fp);

      else if (letter == 'K')

        pRoomIndex->sink_warning = fread_string(fp);

      else if (letter == 'L')

        pRoomIndex->sink_timer = fread_number(fp);

      else if (letter == 'N')

        pRoomIndex->sink_dest = fread_number(fp);

      else if (letter == 'M') /* mana room */
        pRoomIndex->mana_rate = fread_number(fp);

      else if (letter == 'C') { /* clan */
        if (pRoomIndex->clan) {
          bug("Load_rooms: duplicate clan fields.", 0);
          exit(1);
        }
        // Rooms are stored with clan names, looked up on load
        pRoomIndex->clan = clan_lookup(fread_string(fp));
      } else if (letter == 'P') /* EPL program */
        load_room_prog(pRoomIndex, fread_string(fp));

      else if (letter == 'D') {
        EXIT_DATA *pexit;
        int locks = 0;
        door = fread_number(fp);
        if (door < 0 || door > 5) {
          bug("Fread_rooms: vnum %d has bad door number.", vnum);
          exit(1);
        }
        pexit = alloc_perm(sizeof(*pexit));
        //	      pexit = reinterpret_cast<EXIT_DATA
        //*>(alloc_perm(sizeof(*pexit)));
        pexit->description = fread_string(fp);
        pexit->keyword = fread_string(fp);
        pexit->exit_info = 0;
        if (pRoomIndex->area->version < AREA_VER_MD_AF)
          locks = fread_number(fp);
        else
          pexit->exit_info = fread_number(fp);
        pexit->key = fread_number(fp);
        pexit->u1.vnum = fread_number(fp);
        pexit->orig_door = door; /* OLC */
        if (pRoomIndex->area->version < AREA_VER_MD_AF) {
          switch (locks) {
          case 1:
            pexit->exit_info = EX_ISDOOR;
            break;
          case 2:
            pexit->exit_info = EX_ISDOOR | EX_PICKPROOF;
            break;
          case 3:
            pexit->exit_info = EX_ISDOOR | EX_NOPASS;
            break;
          case 4:
            pexit->exit_info = EX_ISDOOR | EX_NOPASS | EX_PICKPROOF;
            break;
          case 5:
            pexit->exit_info = EX_ISDOOR | EX_NOBASH;
            break;
          case 6:
            pexit->exit_info = EX_ISDOOR | EX_NOBASH | EX_NOPASS;
            break;
          case 7:
            pexit->exit_info = EX_ISDOOR | EX_NOBASH | EX_PICKPROOF;
            break;
          case 8:
            pexit->exit_info = EX_ISDOOR | EX_NOBASH | EX_PICKPROOF | EX_NOPASS;
            break;
          }
        }
        pexit->rs_flags = pexit->exit_info;
        pRoomIndex->exit[door] = pexit;
        top_exit++;
      } else if (letter == 'E') {
        EXTRA_DESCR_DATA *ed;
        ed = alloc_perm(sizeof(*ed));
        //	      ed = reinterpret_cast<EXTRA_DESCR_DATA
        //*>(alloc_perm(sizeof(*ed)));
        ed->keyword = fread_string(fp);
        ed->description = fread_string(fp);
        ed->next = pRoomIndex->extra_descr;
        pRoomIndex->extra_descr = ed;
        top_ed++;
      }

      else if (letter == 'O') {
        if (pRoomIndex->owner[0] != '\0') {
          bug("Load_rooms: duplicate owner.", 0);
          exit(1);
        }
        pRoomIndex->owner = fread_string(fp);
      }

      else {
        bug("Load_rooms: vnum %d has flag not 'DES'.", vnum);
        exit(1);
      }
    }
    iHash = vnum % MAX_KEY_HASH;
    pRoomIndex->next = room_index_hash[iHash];
    room_index_hash[iHash] = pRoomIndex;
    top_room++;
    top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; /* OLC */
    assign_area_vnum(vnum);                                      /* OLC */
    /*      if (vnum >= ROOM_VNUM_PLAYER_START && vnum <= ROOM_VNUM_PLAYER_END
              && IS_SET (pRoomIndex->race_flags, ROOM_PLAYERSTORE))
          {
            char filename[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
      struct stat statBuf;
      OBJ_DATA *shelf, *obj_walk;
      CHAR_DATA *ch, *mob;
      int total;
      //	close_store(pRoomIndex);
      sprintf (filename, "stock/%d.hired", pRoomIndex->vnum);
      if (stat (filename, &statBuf) != -1)
      {
        mob = create_mobile(get_mob_index(MOB_VNUM_DEFAULT_SHOPKEEPER));
        mob->recruit_value[2] = -1;
        shelf = create_object(get_obj_index(OBJ_VNUM_SHELF),0);
        load_shelf (shelf, pRoomIndex->vnum);
        if (shelf->contains != NULL)
          {
            for (obj_walk = shelf->contains; obj_walk != NULL;
           obj_walk = obj_walk->next_content)
        {
                      total += obj_walk->cost;
                      if (total >= 100000)
                        {
                          total = -1;
                          break;
                        }
        }
            //              send_to_char (buf, ch);
          }
        extract_obj (shelf);
        if (total != -1 && total < 100000)
          {
            mob->recruit_value[1] = 1;  //Charge % per item
          }

        ch = load_char_obj2 (pRoomIndex->owner);
        if (ch->short_descr[0] != '\0')
          {
            free_string(mob->short_descr);
            sprintf(buf,"%s``",ch->short_descr);
            mob->short_descr = str_dup(buf);
          }
        if (ch->long_descr[0] != '\0')
          {
            free_string(mob->long_descr);
            sprintf(buf,"%s``\n\r",ch->long_descr);
            mob->long_descr = str_dup(buf);
          }
        SET_BIT (mob->recruit_flags, RECRUIT_KEEPER);
        mob->recruit_value[0] = pRoomIndex->vnum;
        is_linkloading = TRUE;
              ppurge(mob,ch);
        is_linkloading = FALSE;
        //      mob->recruit_value[2] = 9999;
        char_to_room(mob,pRoomIndex);
        open_store(pRoomIndex);
      }
      else close_store(pRoomIndex);

      //	  return (TRUE);
      //	return (FALSE);
          }*/
  }
  return;
}

/*
 * Snarf a shop section.
 */
void load_shops(FILE *fp) {
  SHOP_DATA *pShop;
  CHAR_DATA *vch;

#ifdef VERBOSE_BOOT
  log_string("load_shops()");

#endif /*  */
  for (;;) {
    MOB_INDEX_DATA *pMobIndex;
    int iTrade;
    pShop = alloc_perm(sizeof(*pShop));
    //      pShop = reinterpret_cast<SHOP_DATA *>(alloc_perm(sizeof(*pShop)));
    pShop->keeper = fread_number(fp);
    if (pShop->keeper == 0)
      break;
    for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
      pShop->buy_type[iTrade] = fread_number(fp);
    pShop->profit_buy = fread_number(fp);
    pShop->profit_sell = fread_number(fp);
    pShop->open_hour = fread_number(fp);
    pShop->close_hour = fread_number(fp);
    fread_to_eol(fp);
    pMobIndex = get_mob_index(pShop->keeper);
    pMobIndex->pShop = pShop;
    for (vch = char_list; vch != NULL; vch = vch->next) {
      if (!IS_NPC(vch))
        continue;
      if (vch->pIndexData == pMobIndex) {
        if (!IS_SET(vch->imm_flags, IMM_CHARM))
          SET_BIT(vch->imm_flags, IMM_CHARM);
        if (!IS_SET(vch->imm_flags, IMM_MAGIC))
          SET_BIT(vch->imm_flags, IMM_CHARM);
      }
    }
    if (shop_first == NULL)
      shop_first = pShop;
    if (shop_last != NULL)
      shop_last->next = pShop;
    shop_last = pShop;
    pShop->next = NULL;
    top_shop++;
  }
  return;
}

/*
 * Snarf spec proc declarations.
 */
void load_specials(FILE *fp) {

#ifdef VERBOSE_BOOT
  log_string("load_specials()");

#endif /*  */
  for (;;) {
    MOB_INDEX_DATA *pMobIndex;
    char letter;
    switch (letter = fread_letter(fp)) {
    default:
      bug("Load_specials: letter '%c' not *MS.", letter);
      exit(1);
    case 'S':
      return;
    case '*':
      break;
    case 'M':
      pMobIndex = get_mob_index(fread_number(fp));
      pMobIndex->spec_fun = spec_lookup(fread_word(fp));
      if (pMobIndex->spec_fun == 0) {
        bug("Load_specials: 'M': vnum %d.", pMobIndex->vnum);
        exit(1);
      }
      break;
    }
    fread_to_eol(fp);
  }
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits(void) {

  /*    extern const sh_int rev_dir []; */
  ROOM_INDEX_DATA *pRoomIndex;
  EXIT_DATA *pexit;
  int iHash;
  int door;
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
    for (pRoomIndex = room_index_hash[iHash]; pRoomIndex != NULL;
         pRoomIndex = pRoomIndex->next) {
      bool fexit;
      fexit = FALSE;
      for (door = 0; door <= 5; door++) {
        if ((pexit = pRoomIndex->exit[door]) != NULL) {
          if (pexit->u1.vnum <= 0 || get_room_index(pexit->u1.vnum) == NULL)
            pexit->u1.to_room = NULL;

          else {
            fexit = TRUE;
            pexit->u1.to_room = get_room_index(pexit->u1.vnum);
          }
        }
      }
      if (!fexit)
        SET_BIT(pRoomIndex->room_flags, ROOM_NO_MOB);
    }
  }

  /*    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
     {
     for ( pRoomIndex  = room_index_hash[iHash];
     pRoomIndex != NULL;
     pRoomIndex  = pRoomIndex->next )
     {
     for ( door = 0; door <= 5; door++ )
     {
     if ( ( pexit     = pRoomIndex->exit[door]       ) != NULL
     &&   ( to_room   = pexit->u1.to_room            ) != NULL
     &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
     &&   pexit_rev->u1.to_room != pRoomIndex
     &&   (pRoomIndex->vnum < 1200 || pRoomIndex->vnum > 1299))
     {
     sprintf( buf, "Fix_exits: %d:%d -> %d:%d -> %d.",
     pRoomIndex->vnum, door,
     to_room->vnum,    rev_dir[door],
     (pexit_rev->u1.to_room == NULL)
     ? 0 : pexit_rev->u1.to_room->vnum );
     bug( buf, 0 );
     }
     }
     }
     } */
  return;
}

void reset_room(ROOM_INDEX_DATA *pRoom) {
  RESET_DATA *pReset;
  CHAR_DATA *pMob;
  OBJ_DATA *pObj;
  CHAR_DATA *LastMob = NULL;
  OBJ_DATA *LastObj = NULL;
  int iExit;
  int level = 0;
  bool last;
  if (!pRoom)
    return;
  pMob = NULL;
  last = FALSE;
  for (iExit = 0; iExit < MAX_DIR; iExit++) {
    EXIT_DATA *pExit;
    if ((pExit = pRoom->exit[iExit])
        /*  && !IS_SET( pExit->exit_info, EX_BASHED )   ROM OLC */
    ) {
      pExit->exit_info = pExit->rs_flags;
      if ((pExit->u1.to_room != NULL) &&
          ((pExit = pExit->u1.to_room->exit[rev_dir[iExit]]))) {

        /* nail the other side */
        pExit->exit_info = pExit->rs_flags;
      }
    }
  }
  for (pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next) {
    MOB_INDEX_DATA *pMobIndex;
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_INDEX_DATA *pObjToIndex;
    ROOM_INDEX_DATA *pRoomIndex;
    switch (pReset->command) {
    default:
      bug("Reset_room: bad command %c.", pReset->command);
      break;
    case 'M':
      if (!(pMobIndex = get_mob_index(pReset->arg1))) {
        bug("Reset_room: 'M': bad vnum %d.", pReset->arg1);
        continue;
      }
#if 0 /* envy version, ROM doesn't use this */
	  /*
	   * Some hard coding.
	   */
	  if ((pMobIndex->spec_fun == spec_lookup ("spec_cast_ghost")
	       && (weather_info.sunlight != SUN_DARK)))
	    continue;

#endif /* envy version  */
      if (pMobIndex->count >= pReset->arg2) {
        last = FALSE;
        break;
      }
      pMob = create_mobile(pMobIndex);

      /*
       * Some more hard coding.
       */
      if (room_is_dark(pRoom))
        SET_BIT(pMob->affected_by, AFF_INFRARED);

      /*
       * Pet shop mobiles get ACT_PET set.
       */
      {
        ROOM_INDEX_DATA *pRoomIndexPrev;
        pRoomIndexPrev = get_room_index(pRoom->vnum - 1);
        if (pRoomIndexPrev && IS_SET(pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
          SET_BIT(pMob->act, ACT_PET);
      }
      char_to_room(pMob, pRoom);
      if (HAS_SCRIPT(pMob)) {
        TRIGGER_DATA *pTrig;
        for (pTrig = pMob->triggers; pTrig != NULL; pTrig = pTrig->next) {
          if (pTrig->trigger_type == TRIG_BORN && pTrig->current == NULL &&
              !IS_SET(pMob->act, ACT_HALT)) {
            act_trigger(pMob, pTrig->name, NULL, NAME(pMob), NULL);
            pTrig->current = pTrig->script;
            pTrig->bits = SCRIPT_ADVANCE;
          }
        }
      }
      LastMob = pMob;
      level = URANGE(0, pMob->level - 2, LEVEL_HERO - 1); /* -1 ROM */
      last = TRUE;
      break;
    case 'O':
      if (!(pObjIndex = get_obj_index(pReset->arg1))) {
        bug("Reset_room: 'O': bad vnum %d.", pReset->arg1);
        continue;
      }
      if (!(pRoomIndex = get_room_index(pReset->arg3))) {
        bug("Reset_room: 'O': bad vnum %d.", pReset->arg3);
        continue;
      }
      if (pRoom->area->nplayer > 0 ||
          count_obj_list(pObjIndex, pRoom->contents) > 0)
        break;
      pObj = create_object(pObjIndex, /* UMIN - ROM OLC */
                           UMIN(number_fuzzy(level), LEVEL_HERO - 1));
      pObj->cost = 0;
      obj_to_room(pObj, pRoom);
      break;
    case 'P':
      if (!(pObjIndex = get_obj_index(pReset->arg1))) {
        bug("Reset_room: 'P': bad vnum %d.", pReset->arg1);
        continue;
      }
      if (!(pObjToIndex = get_obj_index(pReset->arg3))) {
        bug("Reset_room: 'P': bad vnum %d.", pReset->arg3);
        continue;
      }
      if (pRoom->area->nplayer > 0 || !(LastObj = get_obj_type(pObjToIndex)) ||
          count_obj_list(pObjIndex, LastObj->contains) > 0)
        break;

      /* lastObj->level  -  ROM */
      pObj = create_object(pObjIndex, number_fuzzy(LastObj->level));
      obj_to_obj(pObj, LastObj);
      break;
    case 'G':
    case 'E':
      if (!(pObjIndex = get_obj_index(pReset->arg1))) {
        bug("Reset_room: 'E' or 'G': bad vnum %d.", pReset->arg1);
        continue;
      }
      if (!last)
        break;
      if (!LastMob) {
        bug("Reset_room: 'E' or 'G': null mob for vnum %d.", pReset->arg1);
        last = FALSE;
        break;
      }
      if (LastMob->pIndexData->pShop) { /* Shop-keeper? */
        int olevel;
        switch (pObjIndex->item_type) {
        default:
          olevel = 0;
          break;
        case ITEM_PILL:
          olevel = number_range(0, 10);
          break;
        case ITEM_POTION:
          olevel = number_range(0, 10);
          break;
        case ITEM_SCROLL:
          olevel = number_range(5, 15);
          break;
        case ITEM_WAND:
          olevel = number_range(10, 20);
          break;
        case ITEM_STAFF:
          olevel = number_range(15, 25);
          break;
        case ITEM_ARMOR:
          olevel = number_range(5, 15);
          break;

          /* ROM patch weapon, treasure */
        case ITEM_WEAPON:
          olevel = number_range(5, 15);
          break;
        case ITEM_TREASURE:
          olevel = number_range(10, 20);
          break;

#if 0 /* envy version */
		case ITEM_WEAPON:
		  if (pReset->command == 'G')
		    olevel = number_range (5, 15);

		  else
		    olevel = number_fuzzy (level);

#endif /* envy version */
          break;
        }
        pObj = create_object(pObjIndex, olevel);
        SET_BIT(pObj->extra_flags[0], ITEM_INVENTORY); /* ROM OLC */
      } else { /* ROM OLC else version */
        int limit;
        if (pReset->arg2 > 50) /* old format */
          limit = 6;

        else if (pReset->arg2 == -1 || pReset->arg2 == 0)
          limit = 999;

        else
          limit = pReset->arg2;
        if (pObjIndex->count < limit || number_range(0, 4) == 0)
          pObj = create_object(pObjIndex,
                               UMIN(number_fuzzy(level), LEVEL_HERO - 1));

        else
          break;
      }
      obj_to_char(pObj, LastMob);
      if (pReset->command == 'E')
        equip_char(LastMob, pObj, pReset->arg3);
      last = TRUE;
      break;
    case 'D':
      break;
    case 'R':
      if (!(pRoomIndex = get_room_index(pReset->arg1))) {
        bug("Reset_room: 'R': bad vnum %d.", pReset->arg1);
        continue;
      }
      {
        EXIT_DATA *pExit;
        int d0;
        int d1;
        for (d0 = 0; d0 < pReset->arg2 - 1; d0++) {
          d1 = number_range(d0, pReset->arg2 - 1);
          pExit = pRoomIndex->exit[d0];
          pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
          pRoomIndex->exit[d1] = pExit;
        }
      }
      break;
    }
  }
  return;
}

/*
 * Repopulate areas periodically.
 */
void area_update(void) {
  AREA_DATA *pArea;
  char buf[MAX_STRING_LENGTH];
  for (pArea = area_first; pArea != NULL; pArea = pArea->next) {

    //      ROOM_INDEX_DATA *pRoomIndex;
    // Iblis 6/23/03 - Minax got mad people are using the same areas to kill
    //  over and over.  He is also mad Grimtar is continually cleaning out
    //  the abbey with Hellscape and reaping tons of souls/xp quickly.  He
    //  is not gutsy enough to actually take you sidhe Reavers like he said he
    //  would, or change reaver soul costs, like should be done, so here i go
    //  greatly increasing area repop times.  Lets hope there will not be a
    //  rebellion
    // Iblis 7/05/03 - Minax is gone, I'm setting it back, but setting min tmie
    // 5 ticks not 3,
    //  as a concession to him.  I hope he will be ok with it.  No one can stand
    //  a repop rate of 31 ticks
    //      if (++pArea->age < 31 && pArea !=
    //      (get_room_index(ROOM_VNUM_SCHOOL))->area) continue;
    if (++pArea->age < 5)
      continue;
    if (pArea->nplayer < 0)
      pArea->nplayer = 0;
    if (IS_SET(pArea->area_flags, AREA_NO_REPOP_WIA)) {
      CHAR_DATA *tch;
      bool dontContinue = FALSE;
      if (pArea->nplayer > 0)
        continue;
      for (tch = char_list; tch != NULL; tch = tch->next) {
        if (IS_NPC(tch))
          continue;
        if (tch->was_in_room &&
            tch->in_room == get_room_index(ROOM_VNUM_LIMBO)) {
          if (tch->was_in_room->area == pArea) {
            dontContinue = TRUE;
            break;
          }
        }
      }
      if (dontContinue)
        continue;
    }
    if (IS_SET(pArea->area_flags, AREA_NO_REPOP) && pArea->age == 6) {
      pArea->age = 5;
      continue;
    }
    // log_string(pArea->name);
    /*      if (!strcmp(pArea->name,"`gStillwater``")) {
          sprintf(buf,"pArea->nplayer = %d",pArea->nplayer);
          do_echo(char_list,buf);
          }*/
    /*
     * Check age and reset.
     * Note: Mud School resets every 3 minutes (not 15).
     */
    if ((!pArea->empty && (pArea->nplayer == 0 || pArea->age >= 15)) ||
        pArea->age >= 62) {
      ROOM_INDEX_DATA *pRoomIndex;
      reset_area(pArea);
      sprintf(buf, "%s has just been reset.", pArea->name);
      wiznet(buf, NULL, NULL, WIZ_RESETS, 0, 0);
      pArea->age = number_range(0, 3);
      pRoomIndex = get_room_index(ROOM_VNUM_SCHOOL);
      if (pRoomIndex != NULL && pArea == pRoomIndex->area)
        pArea->age = 15 - 2;

      else if (pArea->nplayer == 0)
        pArea->empty = TRUE;
      pRoomIndex = get_room_index(37968);
      if (pRoomIndex != NULL && pArea == pRoomIndex->area)
        make_hotspots();
    }
  }
  return;
}

/*
 * Reset one area.
 */
void reset_area(AREA_DATA *pArea) {
  RESET_DATA *pReset;
  CHAR_DATA *mob;
  bool last;
  int level;
  OBJ_DATA *obj;
  mob = NULL;
  last = TRUE;
  level = 0;
  obj = NULL;
  for (pReset = pArea->reset_first; pReset != NULL; pReset = pReset->next) {
    ROOM_INDEX_DATA *pRoomIndex;
    MOB_INDEX_DATA *pMobIndex;
    OBJ_INDEX_DATA *pObjIndex;
    EXIT_DATA *pexit;
    OBJ_DATA *cobj;
    int count, limit;
    switch (pReset->command) {
    default:
      bug("Reset_area: bad command %c.", pReset->command);
      break;
    case 'M':
      if ((pMobIndex = get_mob_index(pReset->arg1)) == NULL) {
        bug("Reset_area: 'M': bad vnum %d.", pReset->arg1);
        continue;
      }
      if ((pRoomIndex = get_room_index(pReset->arg3)) == NULL) {
        bug("Reset_area: 'R': bad vnum %d.", pReset->arg3);
        continue;
      }
      if (pMobIndex->count >= pReset->arg2) {
        last = FALSE;
        break;
      }
      count = 0;
      for (mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room)
        if (mob->pIndexData == pMobIndex) {
          count++;
          if (count >= pReset->arg4) {
            last = FALSE;
            break;
          }
        }
      if (count >= pReset->arg4)
        break;
      mob = create_mobile(pMobIndex);
      {
        ROOM_INDEX_DATA *pRoomIndexPrev;
        pRoomIndexPrev = get_room_index(pRoomIndex->vnum - 1);
        if (pRoomIndexPrev != NULL &&
            IS_SET(pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
          SET_BIT(mob->act, ACT_PET);
      }
      mob->zone = pRoomIndex->area;
      char_to_room(mob, pRoomIndex);
      if (HAS_SCRIPT(mob)) {
        TRIGGER_DATA *pTrig;
        for (pTrig = mob->triggers; pTrig != NULL; pTrig = pTrig->next) {
          if (pTrig->trigger_type == TRIG_BORN && pTrig->current == NULL &&
              !IS_SET(mob->act, ACT_HALT)) {
            act_trigger(mob, pTrig->name, NULL, NAME(mob), NULL);
            pTrig->current = pTrig->script;
            pTrig->bits = SCRIPT_ADVANCE;
          }
        }
      }
      level = URANGE(0, mob->level - 2, LEVEL_HERO - 1);
      last = TRUE;
      break;
    case 'O':
      if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL) {
        bug("Reset_area: 'O': bad vnum %d.", pReset->arg1);
        continue;
      }
      if ((pRoomIndex = get_room_index(pReset->arg3)) == NULL) {
        bug("Reset_area: 'R': bad vnum %d.", pReset->arg3);
        continue;
      }
      if (pReset->arg4 != -1 && pObjIndex->count >= pReset->arg4) {
        last = FALSE;
        break;
      }
      if (!rare_enough(pObjIndex)) {
        last = FALSE;
        break;
      }
      if ((pObjIndex->item_type == ITEM_CTRANSPORT ||
           pObjIndex->item_type == ITEM_ELEVATOR) &&
          pObjIndex->count > 0) {
        last = FALSE;
        break;
      }
      if (count_obj_list(pObjIndex, pRoomIndex->contents) > 0) {
        last = FALSE;
        break;
      }
      obj = create_object(pObjIndex, UMIN(number_fuzzy(level), LEVEL_HERO - 1));
      obj->cost = 0;
      obj_to_room(obj, pRoomIndex);
      last = TRUE;
      break;
    case 'P':
      if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL) {
        bug("Reset_area: 'P': bad vnum %d.", pReset->arg1);
        continue;
      }
      if (!last)
        break;
      if (obj == NULL) {
        bug("Reset_area: 'P': bad to obj.", 0);
        continue;
      }
      if (pReset->arg2 == -1)
        limit = 999;

      else
        limit = pReset->arg2;
      if ((obj->in_room == NULL && obj->carried_by == NULL) ||
          (pObjIndex->count >= limit && number_range(0, 4) != 0) ||
          (count = count_obj_list(pObjIndex, obj->contains)) > pReset->arg4) {
        last = FALSE;
        break;
      }
      while (count < pReset->arg4) {
        cobj = create_object(pObjIndex, number_fuzzy(obj->level));
        obj_to_obj(cobj, obj);
        count++;
        if (pObjIndex->count >= limit)
          break;
      }
      obj->value[1] = obj->pIndexData->value[1];
      last = TRUE;
      break;
    case 'G':
    case 'E':
      if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL) {
        bug("Reset_area: 'E' or 'G': bad vnum %d.", pReset->arg1);
        continue;
      }
      if (!last)
        break;
      if (!mob) {
        bug("Reset_room: 'E' or 'G': null mob for vnum %d.", pReset->arg1);
        last = FALSE;
        break;
      }
      if (!rare_enough(pObjIndex))
        break;
      if ((pObjIndex->item_type == ITEM_CTRANSPORT ||
           pObjIndex->item_type == ITEM_ELEVATOR) &&
          pObjIndex->count > 0)
        break;
      if (mob == NULL) {

        /*                bug( "Reset_area: 'E' or 'G': null mob for vnum %d.",
           pReset->arg1 ); */
        last = FALSE;
        break;
      }
      if (mob->pIndexData->pShop != NULL) {
        int olevel = 0, i, j;
        switch (pObjIndex->item_type) {
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
          olevel = 53;
          for (i = 1; i < 5; i++) {
            if (pObjIndex->value[i] > 0) {
              for (j = 0; j < MAX_CLASS; j++) {
                olevel = UMIN(olevel,
                              skill_table[pObjIndex->value[i]].skill_level[j]);
              }
            }
          }
          olevel = UMAX(0, (olevel * 3 / 4) - 2);
          break;
        case ITEM_WAND:
          olevel = number_range(10, 20);
          break;
        case ITEM_STAFF:
          olevel = number_range(15, 25);
          break;
        case ITEM_ARMOR:
          olevel = number_range(5, 15);
          break;
        case ITEM_WEAPON:
          olevel = number_range(5, 15);
          break;
        case ITEM_TREASURE:
          olevel = number_range(10, 20);
          break;
        }
        obj = create_object(pObjIndex, olevel);
        SET_BIT(obj->extra_flags[0], ITEM_INVENTORY);
      }

      else {
        if (pReset->arg2 == -1) /* no limit */
          limit = 999;

        else
          limit = pReset->arg2;
        if (pObjIndex->count < limit || number_range(0, 4) == 0) {
          obj = create_object(pObjIndex,
                              UMIN(number_fuzzy(level), LEVEL_HERO - 1));

          /* error message if it is too high */
          if (obj->level > mob->level + 19)
            fprintf(stderr, "Err: obj %s (%d) -- %d, mob %s (%d) -- %d\n",
                    obj->short_descr, obj->pIndexData->vnum, obj->level,
                    mob->short_descr, mob->pIndexData->vnum, mob->level);
        } else
          break;
      }
      obj_to_char(obj, mob);
      if (pReset->command == 'E')
        equip_char(mob, obj, pReset->arg3);
      last = TRUE;
      break;
    case 'D':
      if ((pRoomIndex = get_room_index(pReset->arg1)) == NULL) {
        bug("Reset_area: 'D': bad vnum %d.", pReset->arg1);
        continue;
      }
      if ((pexit = pRoomIndex->exit[pReset->arg2]) == NULL)
        break;
      if (!IS_SET(pexit->exit_info, EX_BASHED))
        switch (pReset->arg3) {
        case 0:
          REMOVE_BIT(pexit->exit_info, EX_CLOSED);
          REMOVE_BIT(pexit->exit_info, EX_LOCKED);
          break;
        case 1:
          SET_BIT(pexit->exit_info, EX_CLOSED);
          REMOVE_BIT(pexit->exit_info, EX_LOCKED);
          break;
        case 2:
          SET_BIT(pexit->exit_info, EX_CLOSED);
          SET_BIT(pexit->exit_info, EX_LOCKED);
          break;
        }
      last = TRUE;
      break;
    case 'R':
      if ((pRoomIndex = get_room_index(pReset->arg1)) == NULL) {
        bug("Reset_area: 'R': bad vnum %d.", pReset->arg1);
        continue;
      }
      {
        int d0;
        int d1;
        for (d0 = 0; d0 < pReset->arg2 - 1; d0++) {
          d1 = number_range(d0, pReset->arg2 - 1);
          pexit = pRoomIndex->exit[d0];
          pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
          pRoomIndex->exit[d1] = pexit;
        }
      }
      break;
    }
  }
  return;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mobile(CHAR_DATA *parent, CHAR_DATA *clone) {
  int i;
  AFFECT_DATA *paf;
  if (parent == NULL || clone == NULL || !IS_NPC(parent))
    return;

  /* start fixing values */
  clone->name = str_dup(parent->name);
  clone->version = parent->version;
  clone->short_descr = str_dup(parent->short_descr);
  clone->long_descr = str_dup(parent->long_descr);
  clone->description = str_dup(parent->description);
  clone->group = parent->group;
  clone->sex = parent->sex;
  clone->Class = parent->Class;
  clone->race = parent->race;
  clone->level = parent->level;
  clone->trust = 0;
  clone->timer = parent->timer;
  clone->wait = parent->wait;
  clone->hit = parent->hit;
  clone->max_hit = parent->max_hit;
  clone->mana = parent->mana;
  clone->max_mana = parent->max_mana;
  clone->move = parent->move;
  clone->max_move = parent->max_move;
  clone->gold = parent->gold;
  clone->silver = parent->silver;
  clone->exp = parent->exp;
  clone->act = parent->act;
  clone->act2 = parent->act2;
  clone->comm = parent->comm;
  clone->imm_flags = parent->imm_flags;
  clone->res_flags = parent->res_flags;
  clone->vuln_flags = parent->vuln_flags;
  clone->invis_level = parent->invis_level;
  clone->affected_by = parent->affected_by;
  clone->position = parent->position;
  clone->train = parent->train;
  clone->saving_throw = parent->saving_throw;
  clone->alignment = parent->alignment;
  clone->hitroll = parent->hitroll;
  clone->damroll = parent->damroll;
  clone->wimpy = parent->wimpy;
  clone->form = parent->form;
  clone->parts = parent->parts;
  clone->size = parent->size;
  clone->material = str_dup(parent->material);
  clone->off_flags = parent->off_flags;
  clone->dam_type = parent->dam_type;
  clone->start_pos = parent->start_pos;
  clone->default_pos = parent->default_pos;
  clone->spec_fun = parent->spec_fun;
  clone->blocks_exit = parent->blocks_exit;
  clone->number_of_attacks = parent->number_of_attacks;
  for (i = 0; i < 4; i++)
    clone->armor[i] = parent->armor[i];
  for (i = 0; i < MAX_STATS; i++) {
    clone->perm_stat[i] = parent->perm_stat[i];
    clone->mod_stat[i] = parent->mod_stat[i];
  }
  for (i = 0; i < 3; i++)
    clone->damage[i] = parent->damage[i];

  /* now add the affects */
  for (paf = parent->affected; paf != NULL; paf = paf->next)
    affect_to_char(clone, paf);
}

/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object(OBJ_INDEX_DATA *pObjIndex, int level) {
  char buf[MAX_STRING_LENGTH];
  AFFECT_DATA *paf;
  OBJ_DATA *obj;
  ROOM_INDEX_DATA *location;
  EXTRA_DESCR_DATA *ed, *ed_new;
  int i;
  if (pObjIndex == NULL) {
    bug("create_object: NULL pObjIndex.", 0);
    exit(1);
  }
  obj = new_obj();
  obj->pIndexData = pObjIndex;
  obj->in_room = NULL;
  obj->enchanted = FALSE;
  obj->level = pObjIndex->level;
  obj->wear_loc = -1;
  obj->name = str_dup(pObjIndex->name);
  obj->short_descr = str_dup(pObjIndex->short_descr);
  obj->description = str_dup(pObjIndex->description);
  obj->timer = pObjIndex->timer;
  if (pObjIndex->material != NULL)
    obj->material = str_dup(pObjIndex->material);

  else
    obj->material = str_dup("");
  obj->item_type = pObjIndex->item_type;
  for (i = 0; i < MAX_EXTRA_FLAGS; i++)
    obj->extra_flags[i] = pObjIndex->extra_flags[i];
  obj->wear_flags = pObjIndex->wear_flags;

  // Akamai 4/30/99 - Support Class/race specific objects
  obj->Class_flags = pObjIndex->Class_flags;
  obj->race_flags = pObjIndex->race_flags;
  obj->clan_flags = pObjIndex->clan_flags;

  // Iblis 12/31/03 - Object Trigger
  for (i = 0; i < MAX_OBJ_TRIGS; i++) {
    if (pObjIndex->obj_trig_vnum[i] != 0) {
      obj->objtrig[i] = new_ot(pObjIndex->obj_trig_vnum[i]);
      if (obj->objtrig[i])
        obj->objtrig[i]->obj_on = obj;
    } else
      obj->objtrig[i] = NULL;
  }

  // Adeon 6/29/03 -- player owned items
  if (!pObjIndex->plr_owner)
    obj->plr_owner = pObjIndex->plr_owner;
  else
    obj->plr_owner = str_dup(pObjIndex->plr_owner);
  ;
  obj->condition = pObjIndex->condition;
  obj->value[0] = pObjIndex->value[0];
  obj->value[1] = pObjIndex->value[1];
  obj->value[2] = pObjIndex->value[2];
  obj->value[3] = pObjIndex->value[3];
  obj->value[4] = pObjIndex->value[4];
  obj->value[5] = pObjIndex->value[5];
  obj->value[6] = pObjIndex->value[6];
  if (obj->item_type == ITEM_CARD || obj->item_type == ITEM_ROOM_TRAP ||
      obj->item_type == ITEM_OBJ_TRAP || obj->item_type == ITEM_PORTAL_TRAP) {
    obj->value[7] = pObjIndex->value[7];
    obj->value[8] = pObjIndex->value[8];
    obj->value[9] = pObjIndex->value[9];
    obj->value[10] = pObjIndex->value[10];
    obj->value[11] = pObjIndex->value[11];
    obj->value[12] = pObjIndex->value[12];
  }
  obj->weight = pObjIndex->weight;
  obj->dest_list = NULL;
  obj->cost = pObjIndex->cost;

  /*
   * Mess with object properties.
   */
  switch (obj->item_type) {
  default:
    sprintf(buf, "create_object: vnum %d bad type (%d).", pObjIndex->vnum,
            obj->item_type);
    bug(buf, 0);

    // bug( "create_object: vnum %d bad type.", pObjIndex->vnum );
    break;
  case ITEM_LIGHT:
    if (obj->value[2] == 999) {
      obj->value[2] = -1;
    }
    break;
  case ITEM_WRITING_INSTRUMENT:
  case ITEM_WRITING_PAPER:
  case ITEM_CTRANSPORT_KEY:
  case ITEM_INSTRUMENT:
  case ITEM_VIAL:
  case ITEM_PARCHMENT:
  case ITEM_ELEVATOR:
  case ITEM_EBUTTON:
  case ITEM_RAFT:
  case ITEM_CTRANSPORT:
  case ITEM_SADDLE:
  case ITEM_FURNITURE:
  case ITEM_TRASH:
  case ITEM_CONTAINER:
  case ITEM_QUIVER:
  case ITEM_BINDER:
  case ITEM_CLAN_DONATION:
  case ITEM_NEWCLANS_DBOX:
  case ITEM_PLAYER_DONATION:
  case ITEM_DRINK_CON:
  case ITEM_KEY:
  case ITEM_FOOD:
  case ITEM_PACK:
  case ITEM_PROTECT:

    /* BEGIN UNDERWATER */
  case ITEM_GILLS:

    /* END UNDERWATER */
  case ITEM_CORPSE_NPC:
  case ITEM_CORPSE_PC:
  case ITEM_FOUNTAIN:
  case ITEM_MAP:
  case ITEM_CLOTHING:
  case ITEM_PORTAL:
  case ITEM_PORTAL_BOOK:
    break;
  case ITEM_TREASURE:
  case ITEM_WARP_STONE:
  case ITEM_ROOM_KEY:
  case ITEM_GEM:
  case ITEM_JEWELRY:
  case ITEM_FEATHER:
  case ITEM_POLE:
  case ITEM_BAIT:
  case ITEM_TREE:
  case ITEM_SEED:
  case ITEM_WOOD:
  case ITEM_WOODEN_INCONSTRUCTION:
  case ITEM_OBJ_TRAP:
  case ITEM_PORTAL_TRAP:
  case ITEM_ROOM_TRAP:
    break;
  case ITEM_JUKEBOX:
    for (i = 0; i < 5; i++)
      obj->value[i] = -1;
    break;
  case ITEM_SCROLL:
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    break;
  case ITEM_WEAPON:
    break;
  case ITEM_ARMOR:
    break;
  case ITEM_POTION:
  case ITEM_PILL:
    break;
  case ITEM_CARD:
    /*      if (obj->pIndexData->vnum == OBJ_VNUM_COMMON_CARD)
       i = 9;
          else i = 10;
          if (obj->value[8] > i)
          {
            obj->value[9] = obj->value[8]-i;
            obj->value[8] = i;
          }
          if (obj->value[9] > i)
          {
            obj->value[8] = obj->value[9]-i;
      obj->value[9] = i;
          }*/
    break;
  case ITEM_MONEY:
    break;
  // Iblis - 09-10-04 - A new type of object, randomized on vnum, then set to
  // whatever type
  case ITEM_RANDOM:
    // Random object in the Sidhe Graveyard
    if (pObjIndex->vnum == 9688) {
      char buf2[MAX_STRING_LENGTH], buf3[20];
      obj->item_type = ITEM_ARMOR;
      switch (number_range(1, 13)) {
      case 1:
        sprintf(buf2, "sword");
        obj->item_type = ITEM_WEAPON;
        obj->wear_flags = ITEM_WIELD;
        obj->value[0] = WEAPON_SWORD;
        break;
      case 2:
        sprintf(buf2, "helmet");
        obj->wear_flags = ITEM_WEAR_HEAD;
        break;
      case 3:
        sprintf(buf2, "bracer");
        obj->wear_flags = ITEM_WEAR_WRIST;
        break;
      case 4:
        sprintf(buf2, "ring");
        obj->wear_flags = ITEM_WEAR_FINGER;
        break;
      case 5:
        sprintf(buf2, "necklace");
        obj->wear_flags = ITEM_WEAR_NECK;
        break;
      case 6:
        sprintf(buf2, "set of leg guards");
        obj->wear_flags = ITEM_WEAR_LEGS;
        break;
      case 7:
        sprintf(buf2, "pair of boots");
        obj->wear_flags = ITEM_WEAR_FEET;
        break;
      case 8:
        sprintf(buf2, "pair of gloves");
        obj->wear_flags = ITEM_WEAR_HANDS;
        break;
      case 9:
        sprintf(buf2, "cape");
        obj->wear_flags = ITEM_WEAR_ABOUT;
        break;
      case 10:
        sprintf(buf2, "belt");
        obj->wear_flags = ITEM_WEAR_WAIST;
        break;
      case 11:
        sprintf(buf2, "gown");
        obj->wear_flags = ITEM_WEAR_ABOUT;
        break;
      case 12:
        sprintf(buf2, "breastplate");
        obj->wear_flags = ITEM_WEAR_BODY;
        break;
      default:
        sprintf(buf2, "pair of pants");
        obj->wear_flags = ITEM_WEAR_LEGS;
        break;
      }
      obj->wear_flags += ITEM_TAKE;
      sprintf(buf, "%s %s", obj->name, buf2);
      free_string(obj->name);
      obj->name = str_dup(buf);
      switch (number_range(1, 5)) {
      case 1:
        sprintf(buf3, "red");
      case 2:
        sprintf(buf3, "black");
      case 3:
        sprintf(buf3, "brown");
      case 4:
        sprintf(buf3, "grey");
      case 5:
        sprintf(buf3, "white");
      case 6:
        sprintf(buf3, "dark blue");
      case 7:
        sprintf(buf3, "dark red");
      case 8:
        sprintf(buf3, "dark green");
      case 9:
        sprintf(buf3, "dark brown");
      case 10:
        sprintf(buf3, "dark teal");
      default:
        sprintf(buf3, "dark beige");
      }
      switch (number_range(1, 4)) {
      case 1:
        sprintf(buf, "a half decayed %s %s", buf3, buf2);
      case 2:
        sprintf(buf, "a partially digested %s %s", buf3, buf2);
      case 3:
        sprintf(buf, "a mutilated %s %s", buf3, buf2);
      default:
        sprintf(buf, "a worthless %s %s", buf3, buf2);
      }
      free_string(obj->short_descr);
      obj->short_descr = str_dup(buf);
      sprintf(buf2, "%s is here.", buf);
      obj->description = str_dup(capitalize(buf2));
      obj->cost = number_percent();
      obj->weight = number_range(0, 100);
      obj->level = 20 + number_range(1, 40);
      if (obj->item_type == ITEM_ARMOR) {
        obj->value[0] = -10 + number_range(1, 20);
        obj->value[1] = -10 + number_range(1, 20);
        obj->value[2] = -10 + number_range(1, 20);
        obj->value[3] = -10 + number_range(1, 20);
      } else if (obj->item_type == ITEM_WEAPON) {
        obj->value[1] = -5 + number_range(1, 10);
        obj->value[2] = -5 + number_range(1, 10);
        obj->value[3] = number_range(0, 39);
      }
    }
    break;
  }
  for (paf = pObjIndex->affected; paf != NULL; paf = paf->next)
    if (paf->location == APPLY_SPELL_AFFECT)
      affect_to_obj(obj, paf);
  if (obj->item_type == ITEM_ELEVATOR)
    if ((location = get_room_index(obj->value[0])) != NULL)
      if (!IS_SET(location->room_flags, ROOM_ELEVATOR))
        SET_BIT(location->room_flags, ROOM_ELEVATOR);
  if (obj->item_type == ITEM_CTRANSPORT)
    set_transport_flags(obj->value[2], obj->pIndexData->vnum);
  obj->extra_descr = NULL;
  for (ed = pObjIndex->extra_descr; ed != NULL; ed = ed->next) {
    ed_new = new_extra_descr();
    ed_new->keyword = str_dup(ed->keyword);
    ed_new->description = str_dup(ed->description);
    ed_new->next = obj->extra_descr;
    obj->extra_descr = ed_new;
  }
  obj->next = object_list;
  object_list = obj;
  pObjIndex->count++;
  if ((obj->item_type == ITEM_CLAN_DONATION) ||
      (obj->item_type == ITEM_NEWCLANS_DBOX)) {
    load_clan_box(obj);
  } else {
    if (obj->item_type == ITEM_PLAYER_DONATION)
      load_player_box(obj);
  }
  return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object(OBJ_DATA *parent, OBJ_DATA *clone) {
  int i;
  AFFECT_DATA *paf;
  EXTRA_DESCR_DATA *ed, *ed_new;
  if (parent == NULL || clone == NULL)
    return;

  /* start fixing the object */
  clone->name = str_dup(parent->name);
  clone->short_descr = str_dup(parent->short_descr);
  clone->description = str_dup(parent->description);
  clone->item_type = parent->item_type;
  for (i = 0; i < MAX_EXTRA_FLAGS; i++)
    clone->extra_flags[i] = parent->extra_flags[i];
  clone->wear_flags = parent->wear_flags;

  // Akamai 4/30/99 - support for Class/race specific equipment
  clone->Class_flags = parent->Class_flags;
  clone->race_flags = parent->race_flags;
  clone->clan_flags = parent->clan_flags;

  // Iblis 12/31/03 - object triggers
  for (i = 0; i < MAX_OBJ_TRIGS; i++) {
    if (clone->objtrig[i]) {
      clone->objtrig[i]->obj_on = NULL;
      free_ot(clone->objtrig[i]);
      clone->objtrig[i] = NULL;
    }
    if (parent->objtrig[i]) {
      clone->objtrig[i] = new_ot(parent->objtrig[i]->pIndexData->vnum);
      clone->objtrig[i]->obj_on = clone;
    } else
      clone->objtrig[i] = NULL;
  }

  // Adeon 6/29/03 -- player ownership of objects
  if (!parent->plr_owner)
    clone->plr_owner = parent->plr_owner;
  else
    clone->plr_owner = str_dup(parent->plr_owner);
  clone->weight = parent->weight;
  clone->cost = parent->cost;
  clone->level = parent->level;
  clone->condition = parent->condition;
  clone->material = str_dup(parent->material);
  clone->timer = parent->timer;
  for (i = 0; i < 13; i++)
    clone->value[i] = parent->value[i];

  /* affects */
  clone->enchanted = parent->enchanted;
  for (paf = parent->affected; paf != NULL; paf = paf->next)
    affect_to_obj(clone, paf);

  /* extended desc */
  for (ed = parent->extra_descr; ed != NULL; ed = ed->next) {
    ed_new = new_extra_descr();
    ed_new->keyword = str_dup(ed->keyword);
    ed_new->description = str_dup(ed->description);
    ed_new->next = clone->extra_descr;
    clone->extra_descr = ed_new;
  }
}

/*
 * Clear a new character.
 */
void clear_char(CHAR_DATA *ch) {
  static CHAR_DATA ch_zero;
  int i;
  *ch = ch_zero;
  ch->name = &str_empty[0];
  ch->short_descr = &str_empty[0];
  ch->long_descr = &str_empty[0];
  ch->description = &str_empty[0];
  ch->prompt = &str_empty[0];

  /* BEGIN ARENA */
  ch->duel = &str_empty[0];

  /* END ARENA */
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
  ch->on = NULL;
  for (i = 0; i < MAX_STATS; i++) {
    ch->perm_stat[i] = 13;
    ch->mod_stat[i] = 0;
  }
  return;
}

/*
 * Get an extra description from a list.
 */
char *get_extra_name(const char *name, EXTRA_DESCR_DATA *ed) {
  for (; ed != NULL; ed = ed->next) {
    if (is_name((char *)name, ed->keyword))
      return ed->keyword;
  }
  return NULL;
}

char *get_extra_descr(const char *name, EXTRA_DESCR_DATA *ed) {
  for (; ed != NULL; ed = ed->next) {
    if (is_name((char *)name, ed->keyword))
      return ed->description;
  }
  return NULL;
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index(int vnum) {
  MOB_INDEX_DATA *pMobIndex;
  for (pMobIndex = mob_index_hash[vnum % MAX_KEY_HASH]; pMobIndex != NULL;
       pMobIndex = pMobIndex->next) {
    if (pMobIndex->vnum == vnum)
      return pMobIndex;
  }
  if (fBootDb) {
    bug("Get_mob_index: bad vnum %d.", vnum);
    exit(1);
  }
  return NULL;
}

/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index(int vnum) {
  OBJ_INDEX_DATA *pObjIndex;
  for (pObjIndex = obj_index_hash[vnum % MAX_KEY_HASH]; pObjIndex != NULL;
       pObjIndex = pObjIndex->next) {
    if (pObjIndex->vnum == vnum)
      return pObjIndex;
  }
  if (fBootDb) {
    bug("Get_obj_index: bad vnum %d.", vnum);
    exit(1);
  }
  // bug ("Null obj index -> %d",vnum);
  return NULL;
}

/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index(int vnum) {
  ROOM_INDEX_DATA *pRoomIndex;
  long counter = 0;
  if (vnum < 0)
    bug("Get_room_index: bad vnum %d.", vnum);
  for (pRoomIndex = room_index_hash[vnum % MAX_KEY_HASH]; pRoomIndex != NULL;
       pRoomIndex = pRoomIndex->next) {
    if (++counter > 300000) {
      bug("MINAX IT BROKE %d.", vnum);
      return get_room_index(4200);
    }
    if (pRoomIndex->vnum == vnum)
      return pRoomIndex;
  }
  if (fBootDb) {
    bug("Get_room_index: bad vnum %d.", vnum);
    exit(1);
  }
  return NULL;
}

/*
 * Read a letter from a file.
 */
char fread_letter(FILE *fp) {
  char c;

  do {
    c = getc(fp);
    if (is_encrypted) {
      if (c > 8 && c < 127) {
        c = c - enc_shift;
        if (c < 9)
          c = (c + 126) - 8;
      }
    }
  } while (isspace(c));
  return c;
}

bool rare_enough(OBJ_INDEX_DATA *obj) {
  int base;
  if (obj->rarity == RARITY_ALWAYS)
    return (TRUE);
  switch (obj->rarity) {
  case RARITY_VERY_RARE:
    base = 3;
    break;
  case RARITY_RARE:
    base = 10;
    break;
  case RARITY_UNCOMMON:
    base = 25;
    break;
  default:
  case RARITY_SOMEWHAT_COMMON:
    base = 50;
    break;
  case RARITY_COMMON:
    base = 75;
    break;
  }
  base -= (obj->count * 2);
  switch (obj->rarity) {
  case RARITY_VERY_RARE:
    base = UMAX(base, 3);
    break;
  case RARITY_RARE:
    base = UMAX(base, 10);
    break;
  case RARITY_UNCOMMON:
    base = UMAX(base, 25);
    break;
  case RARITY_SOMEWHAT_COMMON:
    base = UMAX(base, 50);
    break;
  case RARITY_COMMON:
    base = UMAX(base, 75);
    break;
  }
  if (base >= number_range(1, 100))
    return (TRUE);
  return (FALSE);
}

int fread_number(FILE *fp) {
  int number;
  bool sign;
  char c, c2;

  do {
    c2 = c = getc(fp);
    if (is_encrypted) {
      if (c > 8 && c < 127) {
        c = c - enc_shift;
        if (c < 9)
          c = (c + 126) - 8;
      }
    }
  } while (isspace(c));
  number = 0;
  sign = FALSE;
  if (c == '+') {
    c2 = c = getc(fp);
    if (is_encrypted) {
      if (c > 8 && c < 127) {
        c = c - enc_shift;
        if (c < 9)
          c = (c + 126) - 8;
      }
    }
  } else if (c == '-') {
    sign = TRUE;
    c2 = c = getc(fp);
    if (is_encrypted) {
      if (c > 8 && c < 127) {
        c = c - enc_shift;
        if (c < 9)
          c = (c + 126) - 8;
      }
    }
  }
  if (!isdigit(c)) {
    bug("Fread_number: bad format.", 0);
    exit(1);
  }
  while (isdigit(c)) {
    number = number * 10 + c - '0';
    c2 = c = getc(fp);
    if (is_encrypted) {
      if (c > 8 && c < 127) {
        c = c - enc_shift;
        if (c < 9)
          c = (c + 126) - 8;
      }
    }
  }
  if (sign)
    number = 0 - number;
  if (c == '|')
    number += fread_number(fp);

  else if (c != ' ') {
    ungetc(c2, fp);
  }
  return number;
}

long fread_flag(FILE *fp) {
  int number;
  char c, c2;
  bool negative = FALSE;

  do {
    c2 = c = getc(fp);
    if (is_encrypted) {
      if (c > 8 && c < 127) {
        c = c - enc_shift;
        if (c < 9)
          c = (c + 126) - 8;
      }
    }
  } while (isspace(c));
  if (c == '-') {
    negative = TRUE;
    c2 = c = getc(fp);
    if (is_encrypted) {
      if (c > 8 && c < 127) {
        c = c - enc_shift;
        if (c < 9)
          c = (c + 126) - 8;
      }
    }
  }
  number = 0;
  if (!isdigit(c) && c != '-') { /* ROM OLC */
    while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
      number += flag_convert(c);
      c2 = c = getc(fp);
      if (is_encrypted) {
        if (c > 8 && c < 127) {
          c = c - enc_shift;
          if (c < 9)
            c = (c + 126) - 8;
        }
      }
    }
  }
  if (c == '-') { /* ROM OLC */
    number = fread_number(fp);
    return -number;
  }
  if (!isdigit(c)) {
    while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
      number += flag_convert(c);
      c2 = c = getc(fp);
      if (is_encrypted) {
        if (c > 8 && c < 127) {
          c = c - enc_shift;
          if (c < 9)
            c = (c + 126) - 8;
        }
      }
    }
  }
  while (isdigit(c)) {
    number = number * 10 + c - '0';
    c2 = c = getc(fp);
    if (is_encrypted) {
      if (c > 8 && c < 127) {
        c = c - enc_shift;
        if (c < 9)
          c = (c + 126) - 8;
      }
    }
  }
  if (c == '|')
    number += fread_flag(fp);

  else if (c != ' ') {
    ungetc(c2, fp);
  }
  if (negative)
    return -1 * number;
  return number;
}

long flag_convert(char letter) {
  long bitsum = 0;
  char i;
  if ('A' <= letter && letter <= 'Z') {
    bitsum = 1;
    for (i = letter; i > 'A'; i--)
      bitsum *= 2;
  } else if ('a' <= letter && letter <= 'z') {
    bitsum = 67108864; /* 2^26 */
    for (i = letter; i > 'a'; i--)
      bitsum *= 2;
  }
  return bitsum;
}

// Reads and allocates space for a read-only, possibly encrypted, shared string
// from a file, using a simple hash based on string length for efficient
// storage.
char *fread_string(FILE *fp) {
  char *plast; // Pointer to the last character processed
  char c;      // Current character read from file

  // Calculate where in the buffer the new string will start. It's offset by the
  // size of a char pointer to leave space for the pointer to the previous
  // string in the hash chain.
  plast = top_string + sizeof(char *);

  // Check if we have exceeded the maximum allowed string space
  if (plast > &string_space[MAX_STRING - MAX_STRING_LENGTH]) {
    bug("Fread_string: MAX_STRING %d exceeded.", MAX_STRING);
    exit(1);
  }

  // Skip leading whitespace characters
  do {
    c = getc(fp);
    // If string encryption is enabled, decrypt the character
    if (is_encrypted && c > 8 && c < 127) {
      c = c - enc_shift;
      if (c < 9)
        c = (c + 126) - 8;
    }
  } while (isspace(c));

  // If the first character is '~', return an empty string
  if ((*plast++ = c) == '~')
    return &str_empty[0];

  // Read characters until we find a terminator ('~'), EOF, or carriage return
  for (;;) {
    c = getc(fp);
    // Decrypt characters if encryption is enabled
    if (is_encrypted && c > 8 && c < 127) {
      c = c - enc_shift;
      if (c < 9)
        c = (c + 126) - 8;
    }

    *plast = c;

    switch (*plast) {
    default:
      plast++;
      break;
    case EOF:
      // Report EOF as an error and return NULL
      bug("Fread_string: EOF", 0);
      return NULL;
    case '\n':
      // Convert newline characters to CRLF sequences
      *plast++ = '\r';
      break;
    case '\r':
      // Ignore carriage returns
      break;
    case '~':
      // String terminator found
      plast++;
      union {
        char *pc;
        char rgc[sizeof(char *)];
      } u1;

      int iHash; // Hash value based on string length
      char *pHash, *pHashPrev, *pString;

      // Null-terminate the string
      plast[-1] = '\0';
      // Calculate the hash value based on string length (with a limit)
      iHash = (plast - 1 - top_string) % MAX_KEY_HASH;

      // Search the hash table for an existing instance of the string
      for (pHash = string_hash[iHash]; pHash; pHash = pHashPrev) {
        for (int ic = 0; ic < sizeof(char *); ic++)
          u1.rgc[ic] = pHash[ic];
        pHashPrev = u1.pc;
        pHash += sizeof(char *);
      }

      // If the system is booting up, store the string in the hash table
      if (fBootDb) {
        pString = top_string;
        top_string = plast;
        u1.pc = string_hash[iHash];
        for (int ic = 0; ic < sizeof(char *); ic++)
          pString[ic] = u1.rgc[ic];
        string_hash[iHash] = pString;
        nAllocString += 1;
        sAllocString += top_string - pString;

        // Return the newly allocated string, offset by the size of a pointer
        // to skip over the hash pointer at the start of the string.
        return pString + sizeof(char *);
      } else {
        // For strings read after boot time, simply duplicate the string.
        // This branch might handle runtime-loaded strings differently, such as
        // logging or using a different storage mechanism.
        return str_dup(top_string + sizeof(char *));
      }
    }
  }
}

char *fread_string_eol(FILE *fp) {
  static bool char_special[256 - EOF];
  char *plast;
  char c;
  if (char_special[EOF - EOF] != TRUE) {
    char_special[EOF - EOF] = TRUE;
    char_special['\n' - EOF] = TRUE;
    char_special['\r' - EOF] = TRUE;
  }
  plast = top_string + sizeof(char *);
  if (plast > &string_space[MAX_STRING - MAX_STRING_LENGTH]) {
    bug("Fread_string_eol: MAX_STRING %d exceeded.", MAX_STRING);
    exit(1);
  }

  /*
   * Skip blanks.
   * Read first char.
   */
  do {
    c = getc(fp);
    if (is_encrypted) {
      if (c > 8 && c < 127) {
        c = c - enc_shift;
        if (c < 9)
          c = (c + 126) - 8;
      }
    }
  } while (isspace(c));
  if ((*plast++ = c) == '\n')
    return &str_empty[0];
  for (;;) {
    c = getc(fp);
    if (is_encrypted) {
      if (c > 8 && c < 127) {
        c = c - enc_shift;
        if (c < 9)
          c = (c + 126) - 8;
      }
    }
    if (!char_special[(*plast++ = c) - EOF])
      continue;
    switch (plast[-1]) {
    default:
      break;
    case EOF:
      bug("Fread_string_eol  EOF", 0);
      exit(1);
      break;
    case '\n':
    case '\r': {
      union {
        char *pc;
        char rgc[sizeof(char *)];
      } u1;
      int ic;
      int iHash;
      char *pHash;
      char *pHashPrev;
      char *pString;
      plast[-1] = '\0';
      iHash = UMIN(MAX_KEY_HASH - 1, plast - 1 - top_string);
      for (pHash = string_hash[iHash]; pHash; pHash = pHashPrev) {
        for (ic = 0; ic < sizeof(char *); ic++)
          u1.rgc[ic] = pHash[ic];
        pHashPrev = u1.pc;
        pHash += sizeof(char *);
        if (top_string[sizeof(char *)] == pHash[0] &&
            !strcmp(top_string + sizeof(char *) + 1, pHash + 1))
          return pHash;
      }
      if (fBootDb) {
        pString = top_string;
        top_string = plast;
        u1.pc = string_hash[iHash];
        for (ic = 0; ic < sizeof(char *); ic++)
          pString[ic] = u1.rgc[ic];
        string_hash[iHash] = pString;
        nAllocString += 1;
        sAllocString += top_string - pString;
        return pString + sizeof(char *);
      } else {
        return str_dup(top_string + sizeof(char *));
      }
    }
    }
  }
}
/*
 * Read to end of line (for comments).
 */
void fread_to_eol(FILE *fp) {
  char c, c2;

  do {
    c2 = c = getc(fp);
    if (is_encrypted) {
      if (c > 8 && c < 127) {
        c = c - enc_shift;
        if (c < 9)
          c = (c + 126) - 8;
      }
    }
  } while (c != '\n' && c != '\r');

  do {
    c2 = c = getc(fp);
    if (is_encrypted) {
      if (c > 8 && c < 127) {
        c = c - enc_shift;
        if (c < 9)
          c = (c + 126) - 8;
      }
    }
  } while (c == '\n' || c == '\r');
  ungetc(c2, fp);
  return;
}

/*
 * Read one word (into static buffer).
 */
char *fread_word(FILE *fp) {
  static char word[MAX_INPUT_LENGTH];
  char *pword;
  int cEnd, c, c2;

  do {
    cEnd = getc(fp);
    if (is_encrypted) {
      if (cEnd > 8 && cEnd < 127) {
        cEnd = cEnd - enc_shift;
        if (cEnd < 9)
          cEnd = (cEnd + 126) - 8;
      }
    }
  } while (isspace(cEnd));
  if (cEnd == '\'' || cEnd == '"') {
    pword = word;
  } else {
    word[0] = cEnd;
    pword = word + 1;
    cEnd = ' ';
  }
  for (; pword < word + MAX_INPUT_LENGTH; pword++) {
    c2 = c = getc(fp);
    if (is_encrypted) {
      if (c > 8 && c < 127) {
        c = c - enc_shift;
        if (c < 9)
          c = (c + 126) - 8;
      }
    }
    *pword = c;
    if (c == EOF) {
      *pword = '\0';
      return word;
    }
    if (cEnd == ' ' ? isspace(*pword) : *pword == cEnd) {
      if (cEnd == ' ') {
        ungetc(c2, fp);
      }
      *pword = '\0';
      return word;
    }
  }
  bug("Fread_word: word too long.", 0);
  exit(1);
  return NULL;
}

/*
 * Allocate some ordinary memory,
 *   with the expectation of freeing it someday.
 */
void *alloc_mem(int sMem) {
  void *pMem;
  int *magic;
  int iList;
  sMem += sizeof(*magic);
  for (iList = 0; iList < MAX_MEM_LIST; iList++) {
    if (sMem <= rgSizeList[iList])
      break;
  }
  if (iList == MAX_MEM_LIST) {
    bug("Alloc_mem: size %d too large.", sMem);
    exit(1);
  }
  if (rgFreeList[iList] == NULL) {
    pMem = alloc_perm(rgSizeList[iList]);
  } else {
    pMem = rgFreeList[iList];
    rgFreeList[iList] = *((void **)rgFreeList[iList]);
  }
  magic = pMem;
  //  magic = reinterpret_cast<int *>(pMem);
  *magic = MAGIC_NUM;
  pMem = (char *)pMem + sizeof(*magic);
  return pMem;
}

/*
 * Free some memory.
 * Recycle it back onto the free list for blocks of that size.
 */
void free_mem(void *pMem, int sMem) {
  int iList;
  int *magic;
  pMem = (char *)pMem - sizeof(*magic);
  magic = pMem;
  //  magic = reinterpret_cast<int *>(pMem);
  if (*magic != MAGIC_NUM) {
    bug("Attempt to recycle invalid memory of size %d.", sMem);
    bug((char *)pMem + sizeof(*magic), 0);
    return;
  }
  *magic = 0;
  sMem += sizeof(*magic);
  for (iList = 0; iList < MAX_MEM_LIST; iList++) {
    if (sMem <= rgSizeList[iList])
      break;
  }
  if (iList == MAX_MEM_LIST) {
    bug("Free_mem: size %d too large.", sMem);
    exit(1);
  }
  *((void **)pMem) = rgFreeList[iList];
  rgFreeList[iList] = pMem;
  return;
}

/*
 * Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely.
 */
void *alloc_perm(int sMem) {
  static char *pMemPerm;
  static int iMemPerm;
  void *pMem;
  while (sMem % sizeof(long) != 0)
    sMem++;
  if (sMem > MAX_PERM_BLOCK) {
    bug("Alloc_perm: %d too large.", sMem);
    exit(1);
  }
  if (pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK) {
    iMemPerm = 0;
    if ((pMemPerm = calloc(1, MAX_PERM_BLOCK)) == NULL) {
      //		if ((pMemPerm = reinterpret_cast<char *>(calloc(1,
      // MAX_PERM_BLOCK))) == NULL) {
      perror("Alloc_perm");
      exit(1);
    }
  }
  pMem = pMemPerm + iMemPerm;
  iMemPerm += sMem;
  nAllocPerm += 1;
  sAllocPerm += sMem;
  return pMem;
}

/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
char *str_dup(const char *str) {
  char *str_new;
  if (str[0] == '\0')
    return &str_empty[0];
  if (str >= string_space && str < top_string)
    return (char *)str;
  str_new = alloc_mem(strlen(str) + 1);
  //  str_new = reinterpret_cast<char *>(alloc_mem(strlen(str) + 1));
  strcpy(str_new, str);
  return str_new;
}

/*
 * Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched.
 */
void free_string(char *pstr) {
  if (pstr == NULL || pstr[0] == '\0')
    return;
  if (pstr == &str_empty[0] || (pstr >= string_space && pstr < top_string))
    return;
  free_mem(pstr, strlen(pstr) + 1);
  return;
}

inline void format_color(char *newstr, char *str, int length) {
  char *cptr;
  int tcnt = 0;
  for (cptr = str; *cptr != '\0'; cptr++) {
    if (*cptr == '`') {
      tcnt++;
      cptr++;
    }
    if (*cptr == '\0')
      break;
  }
  sprintf(newstr, "%-*.*s", length + (tcnt * 2), length + (tcnt * 2), str);
}

void do_areas(CHAR_DATA *ch, char *argument) {
  char buf[MAX_STRING_LENGTH];
  char bigbuf[MAX_STRING_LENGTH * 10];
  AREA_DATA *pArea;
  int len;
  sprintf(bigbuf, "`l[`gLevel  `l][`gArea Name                `l][`gCreator    "
                  " `l][`k$$`l][`gDominion  `l][`gHelp        `l]``\n\r");
  len = strlen(bigbuf);
  for (pArea = area_first; pArea != NULL; pArea = pArea->next) {
    if (!str_cmp(pArea->name, "Haven"))
      continue;

    // if the area is not public then don't show it - when an
    // areas points (affluence) are greater than zero then it is public

    if ((pArea->points <= 0)) {
      if (!IS_IMMORTAL(ch))
        continue;
    }

    // If you have never visited the area, you won't know its existance
    // this is an expansion to the exploration tracking system

    if (exploration_tracking)
      if (!IS_IMMORTAL(ch)) {
        int index = 0, count = 0;

        if (IS_NPC(ch))
          continue;

        for (index = pArea->min_vnum; index <= pArea->max_vnum; index++) {
          count += getbit(ch->pcdata->explored, index);
        }
        if (count == 0)
          continue;
      }

    len += sprintf(bigbuf + len, "[%-3d %-3d]", pArea->llev, pArea->ulev);
    format_color(buf, pArea->name, 25);
    len += sprintf(bigbuf + len, "[%s``]", buf);
    format_color(buf, pArea->creator, 12);
    len += sprintf(bigbuf + strlen(bigbuf), "[%s]", buf);
    len += sprintf(bigbuf + len, "[%2d]", pArea->points);

    // allow distinction between unclaimed and boffo claims
    if ((pArea->clan > CLAN_BOGUS)) {
      format_color(buf, get_clan_symbol(pArea->clan), 10);
    } else {
      sprintf(buf, "          "); // 10 characters match above spacing
    }
    len += sprintf(bigbuf + len, "[%s``]", buf);
    format_color(buf, pArea->help, 12);
    len += sprintf(bigbuf + len, "[%s``]\n\r", buf);
  }
  page_to_char(bigbuf, ch);
}

/*
void do_areas( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH], gbuf[MAX_STRING_LENGTH];
    AREA_DATA *pArea1;
    AREA_DATA *pArea2;
    int iArea;
    int iAreaHalf;

    if (argument[0] != '\0')
    {
  send_to_char("No argument is used with this command.\n\r",ch);
  return;
    }

    iAreaHalf = (top_area + 1) / 2;
    pArea1    = area_first;
    pArea2    = area_first;
    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
  pArea2 = pArea2->next;

    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
    {
      sprintf( gbuf, "{%3d %3d} %-9s %s", pArea1->llev, pArea1->ulev,
         pArea1->creator, pArea1->name);

  if(pArea2 != NULL)
    {
      sprintf( buf, "%-34s {%3d %3d} %-9s %s\n\r", gbuf,
         pArea2->llev, pArea2->ulev,
         pArea2->creator, pArea2->name);
    }
  else
    sprintf(buf, "%s\n\r", gbuf);

  send_to_char( buf, ch );

  pArea1 = pArea1->next;
  if ( pArea2 != NULL )
      pArea2 = pArea2->next;
    }

    return;
}
*/
void do_memory(CHAR_DATA *ch, char *argument) {
  char buf[MAX_STRING_LENGTH];
  sprintf(buf, "Affects %5d\n\r", top_affect);
  send_to_char(buf, ch);
  sprintf(buf, "Areas   %5d\n\r", top_area);
  send_to_char(buf, ch);
  sprintf(buf, "ExDes   %5d\n\r", top_ed);
  send_to_char(buf, ch);
  sprintf(buf, "Exits   %5d\n\r", top_exit);
  send_to_char(buf, ch);
  sprintf(buf, "Helps   %5d\n\r", top_help);
  send_to_char(buf, ch);
  sprintf(buf, "Socials %5d\n\r", social_count);
  send_to_char(buf, ch);
  sprintf(buf, "Mobs    %5d(%d new format)\n\r", top_mob_index, newmobs);
  send_to_char(buf, ch);
  sprintf(buf, "(in use)%5d\n\r", mobile_count);
  send_to_char(buf, ch);
  sprintf(buf, "Objs    %5d(%d new format)\n\r", top_obj_index, newobjs);
  send_to_char(buf, ch);
  sprintf(buf, "Resets  %5d\n\r", top_reset);
  send_to_char(buf, ch);
  sprintf(buf, "Rooms   %5d\n\r", top_room);
  send_to_char(buf, ch);
  sprintf(buf, "Shops   %5d\n\r", top_shop);
  send_to_char(buf, ch);
  sprintf(buf, "Strings %5d strings of %7d bytes (max %d).\n\r", nAllocString,
          sAllocString, MAX_STRING);
  send_to_char(buf, ch);
  sprintf(buf, "Perms   %5d blocks  of %7d bytes.\n\r", nAllocPerm, sAllocPerm);
  send_to_char(buf, ch);
  return;
}

void do_dump(CHAR_DATA *ch, char *argument) {
  int count, count2, num_pcs, aff_count;
  CHAR_DATA *fch;
  MOB_INDEX_DATA *pMobIndex;
  PC_DATA *pc;
  OBJ_DATA *obj;
  OBJ_INDEX_DATA *pObjIndex;
  ROOM_INDEX_DATA *room;
  EXIT_DATA *exit;
  DESCRIPTOR_DATA *d;
  AFFECT_DATA *af;
  FILE *fp;
  int vnum, nMatch = 0;

  /* open file */
  fclose(fpReserve);
  fp = fopen("mem.dmp", "w");

  /* report use of data structures */
  num_pcs = 0;
  aff_count = 0;

  /* mobile prototypes */
  fprintf(fp, "MobProt	%4d (%8d bytes)\n", top_mob_index,
          top_mob_index * (sizeof(*pMobIndex)));

  /* mobs */
  count = 0;
  count2 = 0;
  for (fch = char_list; fch != NULL; fch = fch->next) {
    count++;
    if (fch->pcdata != NULL)
      num_pcs++;
    for (af = fch->affected; af != NULL; af = af->next)
      aff_count++;
  }
  for (fch = char_free; fch != NULL; fch = fch->next)
    count2++;
  fprintf(fp, "Mobs	%4d (%8d bytes), %2d free (%d bytes)\n", count,
          count * (sizeof(*fch)), count2, count2 * (sizeof(*fch)));

  /* pcdata */
  count = 0;
  for (pc = pcdata_free; pc != NULL; pc = pc->next)
    count++;
  fprintf(fp, "Pcdata	%4d (%8d bytes), %2d free (%d bytes)\n", num_pcs,
          num_pcs * (sizeof(*pc)), count, count * (sizeof(*pc)));

  /* descriptors */
  count = 0;
  count2 = 0;
  for (d = descriptor_list; d != NULL; d = d->next)
    count++;
  for (d = descriptor_free; d != NULL; d = d->next)
    count2++;
  fprintf(fp, "Descs	%4d (%8d bytes), %2d free (%d bytes)\n", count,
          count * (sizeof(*d)), count2, count2 * (sizeof(*d)));

  /* object prototypes */
  for (vnum = 0; nMatch < top_obj_index; vnum++)
    if ((pObjIndex = get_obj_index(vnum)) != NULL) {
      for (af = pObjIndex->affected; af != NULL; af = af->next)
        aff_count++;
      nMatch++;
    }
  fprintf(fp, "ObjProt	%4d (%8d bytes)\n", top_obj_index,
          top_obj_index * (sizeof(*pObjIndex)));

  /* objects */
  count = 0;
  count2 = 0;
  for (obj = object_list; obj != NULL; obj = obj->next) {
    count++;
    for (af = obj->affected; af != NULL; af = af->next)
      aff_count++;
  }
  for (obj = obj_free; obj != NULL; obj = obj->next)
    count2++;
  fprintf(fp, "Objs	%4d (%8d bytes), %2d free (%d bytes)\n", count,
          count * (sizeof(*obj)), count2, count2 * (sizeof(*obj)));

  /* affects */
  count = 0;
  for (af = affect_free; af != NULL; af = af->next)
    count++;
  fprintf(fp, "Affects	%4d (%8d bytes), %2d free (%d bytes)\n", aff_count,
          aff_count * (sizeof(*af)), count, count * (sizeof(*af)));

  /* rooms */
  fprintf(fp, "Rooms	%4d (%8d bytes)\n", top_room,
          top_room * (sizeof(*room)));

  /* exits */
  fprintf(fp, "Exits	%4d (%8d bytes)\n", top_exit,
          top_exit * (sizeof(*exit)));
  fclose(fp);

  /* start printing out mobile data */
  fp = fopen("mob.dmp", "w");
  fprintf(fp, "\nMobile Analysis\n");
  fprintf(fp, "---------------\n");
  nMatch = 0;
  for (vnum = 0; nMatch < top_mob_index; vnum++)
    if ((pMobIndex = get_mob_index(vnum)) != NULL) {
      nMatch++;
      fprintf(fp, "#%-4d %3d active %3d killed     %s\n", pMobIndex->vnum,
              pMobIndex->count, pMobIndex->killed, pMobIndex->short_descr);
    }
  fclose(fp);

  /* start printing out object data */
  fp = fopen("obj.dmp", "w");
  fprintf(fp, "\nObject Analysis\n");
  fprintf(fp, "---------------\n");
  nMatch = 0;
  for (vnum = 0; nMatch < top_obj_index; vnum++)
    if ((pObjIndex = get_obj_index(vnum)) != NULL) {
      nMatch++;
      fprintf(fp, "#%-4d %3d active %3d reset      %s\n", pObjIndex->vnum,
              pObjIndex->count, pObjIndex->reset_num, pObjIndex->short_descr);
    }

  /* close file */
  fclose(fp);
  fpReserve = fopen(NULL_FILE, "r");
}

/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy(int number) {
  switch (number_bits(2)) {
  case 0:
    number -= 1;
    break;
  case 3:
    number += 1;
    break;
  }
  return UMAX(1, number);
}

int number_range(int from, int to) {
  int power;
  int number;
  if (from == 0 && to == 0)
    return 0;
  if ((to = to - from + 1) <= 1)
    return from;
  for (power = 2; power < to; power <<= 1)
    ;
  while ((number = number_mm() & (power - 1)) >= to)
    ;
  return from + number;
}

/*
 * Generate a random number, enhanced by Iblis.
 */
/*int number_range (int from, int to)
{
  if (from == 0 && to == 0)
    return 0;
  if ((to = to - from + 1) <= 1)
    return from;
  if (to-from+1 == 0 || to-from+1 > 16777216)
    bug("to-from+1 = %d",to-from+1);
  return ((number_mm() / (16777216/(to-from+1))) + to);
}*/

/*
 * Generate a percentile roll.
 */
int number_percent(void) {
  int percent;
  while ((percent = number_mm() & (128 - 1)) > 99)
    ;
  return 1 + percent;
}

/*
 * Generate a random door.
 */
int number_door(void) {
  int door;
  while ((door = number_mm() & (8 - 1)) > 5)
    ;
  return door;
}

int number_bits(int width) { return number_mm() & ((1 << width) - 1); }

/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static int rgiState[2 + 55];
void init_mm() {
  int *piState;
  int iState;
  piState = &rgiState[2];
  piState[-2] = 55 - 55;
  piState[-1] = 55 - 24;
  piState[0] = ((int)current_time) & ((1 << 30) - 1);
  piState[1] = 1;
  for (iState = 2; iState < 55; iState++) {
    piState[iState] =
        (piState[iState - 1] + piState[iState - 2]) & ((1 << 30) - 1);
  }
  return;
}

long number_mm(void) {
  int *piState;
  int iState1;
  int iState2;
  int iRand;
  piState = &rgiState[2];
  iState1 = piState[-2];
  iState2 = piState[-1];
  iRand = (piState[iState1] + piState[iState2]) & ((1 << 30) - 1);
  piState[iState1] = iRand;
  if (++iState1 == 55)
    iState1 = 0;
  if (++iState2 == 55)
    iState2 = 0;
  piState[-2] = iState1;
  piState[-1] = iState2;
  return iRand >> 6;
}

/*
 * Roll some dice.
 */
int dice(int number, int size) {
  int idice;
  int sum;
  switch (size) {
  case 0:
    return 0;
  case 1:
    return number;
  }
  for (idice = 0, sum = 0; idice < number; idice++)
    sum += number_range(1, size);
  return sum;
}

/*
 * Simple linear interpolation.
 */
int interpolate(int level, int value_00, int value_32) {
  return value_00 + level * (value_32 - value_00) / 32;
}

/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde(char *str) {
  for (; *str != '\0'; str++) {
    if (*str == '~')
      *str = '-';
  }
  return;
}

/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp(const char *astr, const char *bstr) {
  if (astr == NULL) {
    bug("Str_cmp: null astr.", 0);
    return TRUE;
  }
  if (bstr == NULL) {
    bug("Str_cmp: null bstr.", 0);
    return TRUE;
  }
  for (; *astr || *bstr; astr++, bstr++) {
    if (LOWER(*astr) != LOWER(*bstr))
      return TRUE;
  }
  return FALSE;
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix(const char *astr, const char *bstr) {
  if (astr == NULL) {
    bug("Strn_cmp: null astr.", 0);
    return TRUE;
  }
  if (bstr == NULL) {
    bug("Strn_cmp: null bstr.", 0);
    return TRUE;
  }
  for (; *astr; astr++, bstr++) {
    if (LOWER(*astr) != LOWER(*bstr))
      return TRUE;
  }
  return FALSE;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix(const char *astr, const char *bstr) {
  int sstr1;
  int sstr2;
  int ichar;
  char c0;
  if ((c0 = LOWER(astr[0])) == '\0')
    return FALSE;
  sstr1 = strlen(astr);
  sstr2 = strlen(bstr);
  for (ichar = 0; ichar <= sstr2 - sstr1; ichar++) {
    if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
      return FALSE;
  }
  return TRUE;
}

/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix(const char *astr, const char *bstr) {
  int sstr1;
  int sstr2;
  sstr1 = strlen(astr);
  sstr2 = strlen(bstr);
  if (sstr1 <= sstr2 && !str_cmp(astr, bstr + sstr2 - sstr1))
    return FALSE;

  else
    return TRUE;
}

/*
 * Returns an initial-capped string.
 */
char *capitalize(const char *str) {
  static char strcap[MAX_STRING_LENGTH];
  int i;
  for (i = 0; str[i] != '\0'; i++)
    strcap[i] = LOWER(str[i]);
  strcap[i] = '\0';
  strcap[0] = UPPER(strcap[0]);
  return strcap;
}

char *fcapitalize(const char *str) {
  static char retstr[MAX_STRING_LENGTH];
  strcpy(retstr, str);
  retstr[0] = UPPER(retstr[0]);
  return (retstr);
}

/*
 * Append a string to a file.
 */
void append_file(CHAR_DATA *ch, char *file, char *str) {
  FILE *fp;
  if (IS_NPC(ch) || str[0] == '\0')
    return;
  fclose(fpReserve);
  if ((fp = fopen(file, "a")) == NULL) {
    perror(file);
    send_to_char("Could not open the file!\n\r", ch);
  } else {
    fprintf(fp, "[%5d] %s: %s\n", ch->in_room ? ch->in_room->vnum : 0, ch->name,
            str);
    fclose(fp);
  }
  fpReserve = fopen(NULL_FILE, "r");
  return;
}

/*
 * Reports a bug.
 */
void bug(const char *str, int param) {
  char buf[MAX_STRING_LENGTH];
  char c;
  if (fpArea != NULL) {
    int iLine;
    int iChar;
    if (fpArea == stdin) {
      iLine = 0;
    } else {
      iChar = ftell(fpArea);
      fseek(fpArea, 0, 0);
      for (iLine = 0; ftell(fpArea) < iChar; iLine++) {
        while (1) {
          c = getc(fpArea);
          if (is_encrypted) {
            if (c > 8 && c < 127) {
              c = c - enc_shift;
              if (c < 9)
                c = (c + 126) - 8;
            }
          }
          if (c == '\n')
            break;
        }
      }
      fseek(fpArea, iChar, 0);
    }
    sprintf(buf, "[*****] FILE: %s LINE: %d", strArea, iLine);
    log_string(buf);
  }
  strcpy(buf, "[*****] BUG: ");
  sprintf(buf + strlen(buf), str, param);
  log_string(buf);
  return;
}

/*
 * Writes a string to the log.
 */
void log_string(const char *str) {
  char *strtime;
  strtime = ctime(&current_time);
  strtime[strlen(strtime) - 1] = '\0';
  fprintf(stderr, "%s :: %s\n", strtime, str);
  return;
}

//
// Writes a string to the log.
//
void log_activity(const char *note, CHAR_DATA *ach, CHAR_DATA *bch) {
  char buf[MAX_INPUT_LENGTH];
  char *strtime;

  // FILE *actlog;
  strtime = ctime(&current_time);
  strtime[strlen(strtime) - 1] = '\0';

  /*
      fclose( fpReserve );
      if( (actlog=fopen(ACTIVITY_LOG, "a")) == NULL )
        {
          perror( ACTIVITY_LOG );
        }
      else
        {
          fprintf(actlog,"%s :: %s -- %s to %s\n", strtime, note, ach->name,
     bch->name); fclose( actlog );
        }
      fpReserve = fopen( NULL_FILE, "r" );
  */
  sprintf(buf, "%s - %s and %s", note, ach->name, bch->name);
  log_string(buf);
  return;
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain(void) { return; }

void fix_stores(void) {
  char filename[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct stat statBuf;
  OBJ_DATA *shelf, *obj_walk;
  CHAR_DATA *mob;
  //  CHAR_DATA *ch;
  int total = 0;
  ROOM_INDEX_DATA *pRoomIndex;
  int iHash;
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    for (pRoomIndex = room_index_hash[iHash]; pRoomIndex;
         pRoomIndex = pRoomIndex->next) {

      if (pRoomIndex->vnum >= ROOM_VNUM_PLAYER_START &&
          pRoomIndex->vnum <= ROOM_VNUM_PLAYER_END &&
          IS_SET(pRoomIndex->race_flags, ROOM_PLAYERSTORE)) {
        sprintf(filename, "stock/%d.hired", pRoomIndex->vnum);
        if (stat(filename, &statBuf) != -1) {
          mob = create_mobile(get_mob_index(MOB_VNUM_DEFAULT_SHOPKEEPER));
          mob->recruit_value[2] = -1;
          shelf = create_object(get_obj_index(OBJ_VNUM_SHELF), 0);
          load_shelf(shelf, pRoomIndex->vnum);
          if (shelf->contains != NULL) {
            for (obj_walk = shelf->contains; obj_walk != NULL;
                 obj_walk = obj_walk->next_content) {
              total += obj_walk->cost;
              if (total >= 100000) {
                total = -1;
                break;
              }
            }
          }
          extract_obj(shelf);
          if (total != -1 && total < 100000) {
            mob->recruit_value[1] = 1; // Charge % per item
          }
          /**********************************************************
           *  For now, people need to be happy just to have a shop  *
           *  keeper, as the way it was done before would crash us  *
           *  if a player who owned a shop with a shopkeep deleted  *
           **********************************************************/

          /*		ch = load_char_obj2 (pRoomIndex->owner);
              if (ch->short_descr[0] != '\0')
                {
                  free_string(mob->short_descr);
                  sprintf(buf,"%s``",ch->short_descr);
                  mob->short_descr = str_dup(buf);
                }
              if (ch->long_descr[0] != '\0')
                {
                  free_string(mob->long_descr);
                  sprintf(buf,"%s``\n\r",ch->long_descr);
                  mob->long_descr = str_dup(buf);
                }
          */
          free_string(mob->short_descr);
          sprintf(buf, "Bob, the default shopkeeper");
          mob->short_descr = str_dup(buf);
          free_string(mob->long_descr);
          sprintf(buf, "Bob, the default shopkeeper sells goods from the "
                       "shelves.``\n\r");
          mob->long_descr = str_dup(buf);

          SET_BIT(mob->recruit_flags, RECRUIT_KEEPER);
          mob->recruit_value[0] = pRoomIndex->vnum;
          char_to_room(mob, pRoomIndex);
          open_store(pRoomIndex);
        } else
          close_store(pRoomIndex);
      }
    }
}
