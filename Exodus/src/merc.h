/*
   merc.h
 */
#ifndef __MERC_H
#define __MERC_H

//Iblis 4/21/04 - This is used to define the size of the extra
//flags array for objects.  This was done so that HOPEFULLY
//the size can be expandable in the future simply by changing this value
//**NOTE** Be careful when increasing this value, since many of the flag_string
//etc functions that return flag values have max lengths
//**NOTE** Affects to objects can only be done if the affect goes to [0]
#define MAX_EXTRA_FLAGS 	2  

//#include "tables.h"

#define	MAX_WEAPON_FLAGS	46

#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#define DECLARE_SONG_FUN( fun )		SONG_FUN  fun
#define DECLARE_PRAY_FUN( fun )		PRAY_FUN fun

#ifdef SGI
#define inline			/* */
#endif

#if	!defined(FALSE)
#define FALSE	 0
#endif
#if	!defined(TRUE)
#define TRUE	 1
#endif

// if this is defined then the boot of the mud spews lots of
// detail as it loads all the objects.
// #define VERBOSE_BOOT  1

// if this is defined then each area save is loged with lots
// of detail spew
// #define VERBOSE_SAVE  1

// if this is defined then every clan command is logged with
// player and such
// #define VERBOSE_CLANS 1

// if this is defined the the new bard features will be added
// and bard commands will be logged 
#define NEW_BARDS 1

//Iblis 9/19/04
#define LEVEL_BEGIN_STARVING 15

#define PLAYER_KILL 0
#define MOB_KILL    1
#define BR_KILL	    2
#define SD_KILL     3
#define ARENA_KILL  4

#define BR_WON      0
#define SD_WON      1
#define BR_IN       2
#define SD_IN       3
#define TEAM_SD_IN  4
#define TEAM_SD_WON 5

#define AASSIST_MOBS    0
#define AASSIST_PLAYERS 1
#define AASSIST_NONE    1
#define AASSIST_ALL     2

#define SD_NO_TEAM	0
#define SD_TEAM_RED 	1
#define SD_TEAM_BLUE	2
#define SD_TEAM_GREEN	3
#define SD_TEAM_YELLOW	4


//Iblis 11/09/03 - LAMELAMELAME hardcoding of clans
//will fix better later, but it was partially in hardcoded
//so just making it work
#define CLAN_NONE	(A)
#define CLAN_ALL	(B)
#define CLAN_TWI	(C)
#define CLAN_QUI	(D)
#define CLAN_CLU	(E)
#define CLAN_GRE	(F)
#define CLAN_BRO	(G)
#define CLAN_PIR	(H)
#define CLAN_JHE	(I)
#define CLAN_FOR	(J)
#define CLAN_VAR	(K)
#define CLAN_VLA	(L)
#define CLAN_FOO	(M)
#define CLAN_ESC	(N)
#define CLAN_LONER	(O)
#define CLAN_PAR	(P)
#define CLAN_GUT	(Q)
#define CLAN_MIN	(R)

typedef int sh_int;
typedef unsigned char bool;
typedef unsigned char byte;

typedef struct epl_prog EPL_PROG;
typedef struct epl_func EPL_FUNC;
typedef struct room_var ROOM_VAR;
typedef struct char_var CHAR_VAR;
typedef struct obj_var OBJ_VAR;

typedef struct scent_data SCENT_DATA;
typedef struct auction_data AUCTION_DATA;
typedef struct affect_data AFFECT_DATA;
typedef struct area_data AREA_DATA;
typedef struct ban_data BAN_DATA;
typedef struct bounty_data BOUNTY_DATA;
typedef struct invention_data INVENTION_DATA;
typedef struct rune_card_data RUNE_CARD_DATA;
typedef struct buf_type BUFFER;
typedef struct char_data CHAR_DATA;
typedef struct fake_data FAKE_DATA;
typedef struct memory_data MEMORY_DATA;
typedef struct comp_data COMP_DATA;
typedef struct descriptor_data DESCRIPTOR_DATA;
typedef struct exit_data EXIT_DATA;
typedef struct extra_descr_data EXTRA_DESCR_DATA;
typedef struct help_data HELP_DATA;
typedef struct kill_data KILL_DATA;
typedef struct mem_data MEM_DATA;
typedef struct mob_index_data MOB_INDEX_DATA;
typedef struct note_data NOTE_DATA;
typedef struct trap_data TRAP_DATA;
typedef struct obj_data OBJ_DATA;
typedef struct obj_index_data OBJ_INDEX_DATA;
typedef struct pc_data PC_DATA;
typedef struct gen_data GEN_DATA;
typedef struct reset_data RESET_DATA;
typedef struct room_index_data ROOM_INDEX_DATA;
typedef struct elevator_dest_list ELEVATOR_DEST_LIST;
typedef struct shop_data SHOP_DATA;
typedef struct time_info_data TIME_INFO_DATA;
typedef struct weather_data WEATHER_DATA;
typedef struct script_data SCRIPT_DATA;
typedef struct trigger_data TRIGGER_DATA;
typedef struct variable_data VARIABLE_DATA;
typedef struct aggressor_list AGGRESSOR_LIST;
typedef struct obj_trig_data OBJ_TRIG_DATA;
typedef struct obj_trig OBJ_TRIG;
typedef struct trap_info_type trap_info_type;
typedef void DO_FUN (CHAR_DATA *, char *);
typedef bool SPEC_FUN (CHAR_DATA *);
typedef void SPELL_FUN (int, int, CHAR_DATA *, void *, int);
typedef void SONG_FUN (int, int, CHAR_DATA *, void *, int);
typedef void PRAY_FUN (int, int, CHAR_DATA *, void *, int);



#define	MAX_KEY_HASH         2048
#define MAX_STRING_LENGTH    4608
#define MAX_INPUT_LENGTH      256
#define PAGELEN                22
#define MAX_CHUNKS             80 /* Used in ssm.c */

#define NO_FLAG               -99
#define AREA_NONE               0
#define AREA_CHANGED            1
#define AREA_ADDED              2
#define AREA_LOADING            4
#define AREA_IMP_ONLY		D
#define AREA_NO_QUIT		E
#define AREA_NO_REPOP_WIA	F
#define AREA_NO_TREE		G
#define AREA_SOLO		H
#define AREA_NO_REPOP		I
#define AREA_VER_DEFAULT        0	// the lowest version
#define AREA_VER_CLASSRACE      1	// version adds Class/race flags to objects
#define AREA_VER_OBJTRIG        2       // version adds obj_triggers to objects
#define AREA_VER_OBJTRIG2	3       // version adds up to 16 obj_triggers per object
#define AREA_VER_EXTRA_FLAGS	4       // version adds more extraflags
#define AREA_VER_MD_AF		5	// version adds more door flags saved and fixed area_flags hack
#define AREA_VER_CARDS	        6
#define AREA_VER_CURRENT        AREA_VER_CARDS

//Iblis 04/04/04 - Temporary hacks to add/save area flags.  Next time the area files are moved to a new version,
//make it so area_flags is saved and then remove these
#define POINTS_IMP_ONLY		K
#define POINTS_NO_QUIT		L
#define POINTS_NO_REPOP_WIA	M
#define POINTS_NO_TREE		N

#define PFILE_VER_DEFAULT       5	// the version when I started this
#define PFILE_VER_NOTES         6	// adds save/restore for legend & oocnote
#define PFILE_VER_IMMCOLORS     7	// adds configurable colors for  imm/adm channels
#define PFILE_VER_PROJECTS      8	// Adeon 7/18/03 -- projects added to pfiles
#define PFILE_VER_CURRENT       PFILE_VER_PROJECTS
#define PFILE_BACKUP_EXT        ".opf"
#define HERO_BACKUP_EXT         ".hbk"
#define PFILE_BACKUP_DEATH      ".death"
#define PFILE_BACKUP_DUEL       ".duel"


#define MAX_DIR                   6
#define MAX_SOCIALS	        512
#define MAX_SKILL               512
#define MAX_GROUP                64
#define MAX_IN_GROUP             16
#define MAX_ALIAS                48
#define MAX_CLASS                17
#define MAX_PC_RACE              15
#define MAX_HISTORY              20
#define MAX_IGNORE               10
#define MAX_LEVEL               100
#define MAX_EXPLORE            8192
#define REAVER_WEAPON_MAX         8
#define	MAX_SOUL_GAIN           100
#define OVERLORD              (MAX_LEVEL)	// 100
#define WIZARD                (MAX_LEVEL - 2)	//  98
#define GOD                   (MAX_LEVEL - 5)	//  95
#define DEMIGOD               (MAX_LEVEL - 8)	//  92
#define LEVEL_HERO            (MAX_LEVEL - 9)	//  90
#define HERO                  (LEVEL_HERO)	//  90
#define LEVEL_EXPLORER        (MAX_LEVEL - 9)	//  91
#define LEVEL_IMMORTAL        (MAX_LEVEL - 8)	//  92

#define PULSE_PER_SECOND      4
#define PULSE_SPEED           (  1 * PULSE_PER_SECOND)
#define PULSE_VIOLENCE        (  3 * PULSE_PER_SECOND)
#define PULSE_MOBILE          (  4 * PULSE_PER_SECOND)
#define PULSE_MUSIC           (  6 * PULSE_PER_SECOND)
#define PULSE_TICK            ( 40 * PULSE_PER_SECOND)
#define PULSE_REBOOT          ( 60 * PULSE_PER_SECOND)
#define PULSE_SHUTDOWN        ( 60 * PULSE_PER_SECOND)
#define PULSE_AREA            (120 * PULSE_PER_SECOND)
#define PULSE_CLANWAR         (180 * PULSE_PER_SECOND)	// orginal
//#define PULSE_CLANWAR         (120 * PULSE_PER_SECOND) // testing
#define PULSE_AUCTION         ( 18 * PULSE_PER_SECOND)
#define PULSE_SCENT           ( 10 * PULSE_PER_SECOND)
#define PULSE_BET             ( 18 * PULSE_PER_SECOND)
#define MULTIKILL_WAIT_TIME    8  //In terms of minutes

#define STOCK_PERM             0
#define STOCK_TEMP             1

#define RACE_HUMAN              (A)
#define RACE_DWARF              (B)
#define RACE_VROATH             (C)
#define RACE_ELF                (D)
#define RACE_CANTHI             (E)
#define RACE_SYVIN              (F)
#define ROOM_NOTRANSPORT        (G)	// HACK to let notransport work
#define RACE_SIDHE              (H)
#define ROOM_PLAYERSTORE        (I)
#define ROOM_NOTELEPORT         (J)	// Another hack, to let no-teleport work
#define RACE_AVATAR		(K)
#define RACE_THYRENT 	 	(L)
#define RACE_LITAN              (L)
#define RACE_KALIAN             (M)
#define RACE_LICH		(N)
#define RACE_NERIX		(O)
#define RACE_SWARM		(P)


//
// Flags support character creation and Class specific objects
//
#define CLASS_MAGE              (A)
#define CLASS_CLERIC            (B)
#define CLASS_THIEF             (C)
#define CLASS_WARRIOR           (E)
#define CLASS_RANGER            (F)
#define CLASS_BARD              (G)
#define CLASS_PALADIN           (H)
#define CLASS_ASSASSIN          (I)
#define CLASS_REAVER            (J)
#define CLASS_MONK              (K)
#define CLASS_NECROMANCER       (L)
#define CLASS_SWARM		(M)
#define CLASS_GOLEM		(N)
#define CLASS_CHAOS_JESTER	(O)
#define CLASS_ADVENTURER	(P)
#define	CLASS_DRUID		(Q)
#define CLASS_EMPATH    (R)
#define CLASS_HERO              (ee)

/*
 * Site ban structure.
 */
#define BAN_SUFFIX              (A)
#define BAN_PREFIX              (B)
#define BAN_NEWBIES             (C)
#define BAN_ALL                 (D)
#define BAN_PERMIT              (E)
#define BAN_PERMANENT           (F)

/* for statlist command*/
#define STAT_PK_KILLS            0
#define STAT_MOB_KILLS           1
#define STAT_PK_DEATHS           2
#define STAT_MOB_DEATHS          3
#define STAT_ROOMS_EXPLORED      4
#define STAT_HOURS_PLAYED        5
#define STAT_TOTAL_EXP           6
#define STAT_EXODUS_SCORE        7
//Iblis added 1/19/04
#define STAT_SD_KILLS		 8
#define STAT_SD_DEATHS		 9
#define STAT_SD_IN		 10
#define STAT_SD_WON              11
#define STAT_BR_KILLS		 12
#define STAT_BR_DEATHS		 13
#define STAT_BR_IN               14
#define STAT_BR_WON		 15
#define STAT_ARENA_KILLS         16
#define STAT_ARENA_DEATHS        17
#define STAT_TOTAL_SOULS	 18
#define STAT_FASTEST_TN		 19
#define STAT_TG_LOSS		 20
#define STAT_TG_WON		 21
#define STAT_AVATAR		 22
#define STAT_TQFindsC		 23
#define STAT_TQFindsA		 24
#define STAT_TQHuntsC            25
#define STAT_TQHuntsA            26
#define STAT_EXTRA_DEATHS	 27
#define STAT_TREES_CHOPPED	 28
#define STAT_TREES_MILLED	 29
#define STAT_OBJS_SOLD		 30
#define STAT_MONEY_MADE		 31
#define STAT_CG_WON		 32
#define STAT_CG_LOST		 33

#define MAX_GAMESTAT             34
#define STAT_FILE   "statlst.dat"
#define MAX_PEOPLE_STAT		 50

/*reaver upgrade constants*/
#define  BLADE_POWER             1
#define  BLADE_HATRED            2
#define  BLADE_STRENGTH          3
#define  BLADE_FURY              4

/* Blade Spells */
#define  BLADE_SPELL_SHOCKING    0
#define  BLADE_SPELL_FROST       1
#define  BLADE_SPELL_FLAMING     2
#define  BLADE_SPELL_VENEMOUS    3
#define  BLADE_SPELL_VAMPIRIC    4
#define  BLADE_SPELL_CHAOTIC     5
#define  BLADE_SPELL_APATHY      6
#define  MAX_BLADE_SPELLS        7

/* Reaver Burst Spells */
#define  BURST_LIGHTNING         0
#define  BURST_ICE               1
#define  BURST_FIRE              2
#define  BURST_UNHOLY            3
#define  BURST_HELLSCAPE         4
#define  MAX_BURST               5

//add change more here

#define MAX_OBJ_TRIGS		 16
//#define MAX_OBJ_TRIG_PARENTS     20 // maximum number of objects an
                                    // object trigger can be appied to^

// Adeon 7/18/03 -- Object triggers!
struct obj_trig_data
{
  OBJ_TRIG_DATA *next;
  bool valid;
  int vnum; //vnum of the obj trigger
  char *name; //name it to identify it
 // int parent[MAX_OBJ_TRIG_PARENTS]; //applied to these obj vnums
  CHAR_DATA *fakie; // simulated char
  char *trigger; // this is the string that is translated by the parser
  long trig_flags; // what will trigger the action? (designated by parser)
  long extra_flags;
  char *action[16];
  short chance[16];
  short delay[16];
  short overall_chance;
  
};

struct obj_trig
{
  OBJ_TRIG *next;
  OBJ_TRIG *next_ot; 
  OBJ_TRIG *next_on_tick;
  bool valid;
  short statement_on;
  short seconds_left;
  OBJ_TRIG_DATA * pIndexData;
  OBJ_DATA* obj_on;
  CHAR_DATA* ch;
  CHAR_DATA* victim;
};

struct ban_data
{
  BAN_DATA *next;
  bool valid;
  short ban_flags;
  short level;
  char *name;
};

/*
 * Bounty Stuff -- by Gomi no sensei
 */
struct bounty_data
{
  BOUNTY_DATA *next;
  bool valid;
  char *name;
  int amount;
};

struct invention_data
{
  INVENTION_DATA *next;
  bool valid;
  int vnum1;
  int vnum2;
  int vnum3;
};

struct rune_card_data
{
  RUNE_CARD_DATA *next;
  bool valid;
  int vnum;
};

struct comp_type
{
  char *name;
  int type;
  int mana;
  int apply;
  long modify_by;
  bool area;
  bool hostile;
  int duration;
  int level;
  long base_damage;
  int level_bonus;
  char *char_string;
  char *vict_string;
  char *wear_off_string;
  char *play_str_ch;
  char *play_str_other;
};

extern const struct comp_type comp_table[];

struct creating_type
{
  char *name;
  struct creating_type *next;
};

extern struct creating_type *char_creating;

struct buf_type
{
  BUFFER *next;
  bool valid;
  short state;			/* error state of the buffer */
  short size;			/* size in k */
  char *string;			/* buffer's string */
};


/*
 * Time and weather stuff.
 */
#define SUN_DARK         0
#define SUN_RISE         1
#define SUN_LIGHT        2
#define SUN_SET          3

#define SKY_CLOUDLESS    0
#define SKY_CLOUDY       1
#define SKY_RAINING      2
#define SKY_LIGHTNING    3

struct build_info
{
  char name[32];
  int room_vnum;
};

struct duel_info
{
  char name[32];
  int wins;
  int losses;
  float per;
};

struct time_info_data
{
  int hour;
  int day;
  int month;
  int year;
};

struct weather_data
{
  int mmhg;
  int change;
  int sky;
  int sunlight;
};

/*
 * Trigger types for repetitive loops, command etc.
 */
#define TRIG_COMMAND          0	/* When a player types x command        */
#define TRIG_EACH_PULSE       1	/* Increments every pulse               */
#define TRIG_COMBAT           2	/* Every combat pulse                   */
#define TRIG_TICK_PULSE       3	/* Every tick (one mud-hour)            */
#define TRIG_BORN             4	/* Happens upon creation                */
#define TRIG_GETS             5	/* Happens when a mob gets/is given     */
#define TRIG_EXIT             6	/* Happens on exit (for each person)    */
#define TRIG_SOCIAL           7	/* Happens when mob is socialed         */
#define TRIG_SIGNAL           8	/* Trigger Signal Handler               */
#define TRIG_DIES             9	/* Happens when the mob is killed       */
#define TRIG_ENTER           10	/* Happens on arrival (for each person) */
#define TRIG_MOVES           11	/* Happens whenever a mobile moves      */

/*
 * Internally used bits for triggers.
 */
#define SCRIPT_ADVANCE       0x00000001	/* Its OK to go to next command set  */
#define SCRIPT_HALT          0x00000002	/* current=NULL & disallow trig      */


struct script_data
{
  SCRIPT_DATA *next;
  char *command;
};

struct trigger_data
{
  TRIGGER_DATA *next;
  SCRIPT_DATA *script;
  SCRIPT_DATA *current;
  short tracer;
  short waiting;
  short trigger_type;
  int bits;
  char *keywords;
  char *name;
};

struct variable_data
{
  VARIABLE_DATA *next;
  char *name;
  char *value;
};

/*
 * Connected state for a channel.
 */
#define CON_PLAYING               0
#define CON_GET_NAME              1
#define CON_GET_OLD_PASSWORD      2
#define CON_CONFIRM_NEW_NAME      3
#define CON_GET_NEW_PASSWORD      4
#define CON_CONFIRM_NEW_PASSWORD  5
#define CON_GET_NEW_RACE          6
#define CON_GET_NEW_SEX           7
#define CON_GET_NEW_CLASS         8
#define CON_GET_ALIGNMENT         9
#define CON_DEFAULT_CHOICE       10
#define CON_GEN_GROUPS           11
#define CON_PICK_WEAPON          12
#define CON_READ_IMOTD           13
#define CON_READ_MOTD            14
#define CON_BREAK_CONNECT        15
#define CON_GET_EMAIL            16
#define CON_GET_ANSI             17
#define CON_PICK_HAND            18
#define CON_CONFIRM_STATS        19
#define	CON_SWORD_GENERATION     20	/* Reaver Dynamic SwordGen */
#define	CON_SWORD_CONFIRM        21	/* Confirm Sword Name And Continue */
#define CON_START_MULTICLASS     22	/* First MultiClass State */
#define CON_FINISH_MULTICLASS    23	/* Second MultiClass State */
#define CON_GET_AVATAR_TYPE      24
#define CON_GET_REFERRAL 	 25

struct room_var
{
  char *name;
  ROOM_INDEX_DATA *value;
  ROOM_VAR *next;
};

struct char_var
{
  char *name;
  CHAR_DATA *value;
  CHAR_VAR *next;
};

struct obj_var
{
  char *name;
  OBJ_DATA *value;
  OBJ_VAR *next;
};

struct epl_func
{
  char *name;
  char *text;
  char *where;
  char *pass_args;
  char *arg_types;
  void *return_val;
  bool return_wait;

  ROOM_VAR *room_vars;
  CHAR_VAR *char_vars;
  OBJ_VAR *obj_vars;

  EPL_FUNC *caller;
  EPL_FUNC *next;
};

struct epl_prog
{
  bool running;
  int wait;

  EPL_FUNC *funcs;
  EPL_FUNC *where;

  EPL_PROG *next;
};

struct fake_data
{
  char *name;
  char *title;
  short race;
  short level;
  short Class;
  short descriptor;
  bool shown;
  FAKE_DATA *next;
};

/*
 * Descriptor (channel) structure.
 */
struct descriptor_data
{
  DESCRIPTOR_DATA *next;
  DESCRIPTOR_DATA *snoop_by;
  CHAR_DATA *character;
  CHAR_DATA *original;
  bool valid;
  char *host;
  int port;			/* Shinowlan 3/29/2000 -- stores user's remote port */
  char *realhost;
  short descriptor;
  short connected;
  bool fcommand;
  char inbuf[4 * MAX_INPUT_LENGTH];
  char incomm[MAX_INPUT_LENGTH];
  char inlast[MAX_INPUT_LENGTH];
  int repeat;
  char *outbuf;
  int outsize;
  int outtop;
  char *showstr_head;
  char *showstr_point;
  void *pEdit;
  char **pString;
  int editor;
  int old_editor;
  int door_dir;
  char *submit_info;
  char *door_name;
  char *help_info;
  char *help_name;
  bool ansi;
};


/*
 * Attribute bonus structures.
 */
struct str_app_type
{
  short tohit;
  short todam;
  short carry;
  short wield;
};

struct int_app_type
{
  short learn;
};

struct wis_app_type
{
  short practice;
};

struct dex_app_type
{
  short defensive;
};

struct con_app_type
{
  short hitp;
  short shock;
};



/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3
#define TO_ALL		    4
#define TO_IIR		    5  /*Iblis 11-05 to_imms in room*/
#define TO_CJIR		    6  //Iblis 10/05/04 - To all CJ's in the room (and imms)

/*
 * Help table types.
 */
struct help_data
{
  HELP_DATA *next;
  short level;
  char *keyword;
  char *text;
};

/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct shop_data
{
  SHOP_DATA *next;		/* Next shop in list            */
  short keeper;			/* Vnum of shop keeper mob      */
  short buy_type[MAX_TRADE];	/* Item types shop will buy     */
  short profit_buy;		/* Cost multiplier for buying   */
  short profit_sell;		/* Cost multiplier for selling  */
  short open_hour;		/* First opening hour           */
  short close_hour;		/* First closing hour           */
};

/*
 * Per-Class stuff.
 */

#define MAX_GUILD   2
#define MAX_STATS   6

#define STAT_STR    0
#define STAT_INT    1
#define STAT_WIS    2
#define STAT_DEX    3
#define STAT_CON    4
#define STAT_CHA    5

struct Class_type
{
  char *name;			/* the full name of the Class */
  char who_name[4];		/* Three-letter name for 'who'  */
  short attr_prime;		/* Prime attribute              */
  short weapon;			/* First weapon                 */
  short guild[MAX_GUILD];	/* Vnum of guild rooms          */
  short skill_adept;		/* Maximum skill level          */
  short thac0_00;		/* Thac0 for level  0           */
  short thac0_32;		/* Thac0 for level 32           */
  short hp_min;			/* Min hp gained on leveling    */
  short hp_max;			/* Max hp gained on leveling    */
  bool fMana;			/* Class gains mana on level    */
  char *base_group;		/* base skills gained           */
  char *default_group;		/* default skills gained        */
};

struct item_type
{
  int type;
  char *name;
};

struct tree_type
{
  short type;
  char *name;
  int weight;
  short rarity;
  //Hardcoding area is a horrible idea here, but I can't think of an easier way to do it without
  //a TON of work.
  char* area;
  short hp_heal_bonus;
  short mana_heal_bonus;
};

struct wooden_obj_type
{
  short type;
  short updates_to_complete;
  short amount_of_wood;
  short vnum;
};


struct weapon_type
{
  char *name;
  int vnum;
  short type;
  short *gsn;
};

struct wiznet_type
{
  char *name;
  long flag;
  int level;
};

struct attack_type
{
  char *name;			/* name */
  char *noun;			/* message */
  int damage;			/* damage Class */
};

struct race_type
{
  char *name;			/* call name of the race */
  bool pc_race;			/* can be chosen by pcs */
  long act;			/* act bits for the race */
  long aff;			/* aff bits for the race */
  long off;			/* off bits for the race */
  long imm;			/* imm bits for the race */
  long res;			/* res bits for the race */
  long vuln;			/* vuln bits for the race */
  long form;			/* default form flag for the race */
  long parts;			/* default parts for the race */
};


struct pc_race_type
{				/* additional data for pc races */
  char *name;			/* MUST be in race_type */
  char who_name[7];
  short points;			/* cost in points of the race */
  short Class_mult[MAX_CLASS];	/* exp multiplier for Class, * 100 */
  char *skills[5];		/* bonus skills for the race */
  short stats[MAX_STATS];	/* starting stats */
  short max_stats[MAX_STATS];	/* maximum stats */
  short size;			/* aff bits for the race */
};


struct spec_type
{
  char *name;			/* special function name */
  SPEC_FUN *function;		/* the function */
};



/*
 * Data structure for notes.
 */

#define NOTE_NOTE      0
#define NOTE_IDEA      1
#define NOTE_PENALTY   2
#define NOTE_NEWS      3
#define NOTE_CHANGES   4
// Akamai 5/6/99 - Adding note bases for legend/music and OOC notes
#define NOTE_LEGEND    5
#define NOTE_OOCNOTE   6
// Adeon 7/18/03 -- Adding an imm project board to seperate it from
//                   player idea spam
#define NOTE_PROJECTS  7

struct note_data
{
  NOTE_DATA *next;
  bool valid;
  short type;
  char *sender;
  char *date;
  char *to_list;
  char *subject;
  char *text;
  time_t date_stamp;
};



/*
 * An affect.
 */
struct affect_data
{
  AFFECT_DATA *next;
  bool valid;
  short where;
  short type;
  short level;
  short duration;
  short location;
  short modifier;
  int bitvector;
  bool permaff;
  bool composition;
  char *comp_name;
};

/* where definitions */
#define TO_AFFECTS	0
#define TO_OBJECT	1
#define TO_IMMUNE	2
#define TO_RESIST	3
#define TO_VULN		4
#define TO_WEAPON	5
#define TO_SKILL        6
#define TO_OBJECT2	7
#define TO_ACT2		8
#define TO_IGNORE	9

/*
 * A kill structure (indexed by level).
 */
struct kill_data
{
  short number;
  short killed;
};

#define GROUP_VNUM_TROLLS      2100
#define GROUP_VNUM_OGRES       2101

#define MOB_VNUM_FIDO          4039
#define MOB_VNUM_PATROLMAN     2106

#define MOB_VNUM_SUMMON_1        10
#define MOB_VNUM_SUMMON_2        15
#define MOB_VNUM_SUMMON_3        11
#define MOB_VNUM_SUMMON_4        16
#define MOB_VNUM_SUMMON_5        12
#define MOB_VNUM_SUMMON_6        13
#define MOB_VNUM_SUMMON_7        14
#define MOB_VNUM_SUMMON_8        29
#define MOB_VNUM_BEAST	         17
#define MOB_VNUM_ILLUSION	 18
#define MOB_VNUM_BONE_PUPPET     19
#define MOB_VNUM_FAM_ZOMBIE      20
#define MOB_VNUM_FAM_SKELETON    21
#define MOB_VNUM_FAM_BANSHEE     22

#define MOB_VNUM_BC_MOUNT        23
#define MOB_VNUM_DARKHOUNDS      24
#define MOB_VNUM_FROG		 25
#define MOB_VNUM_WARPED_CAT	 26
#define MOB_VNUM_WARPED_OWL	 27
#define MOB_VNUM_IMAGINARY_FRIEND 28
#define MOB_VNUM_FAKIE           30 // the obj trigger fakie
#define MOB_VNUM_DEFAULT_SHOPKEEPER 31
#define MOB_VNUM_VIVIFY		 32

#define A      1
#define B      2
#define C      4
#define D      8
#define E      16
#define F      32
#define G      64
#define H      128

#define I      256
#define J      512
#define K      1024
#define L      2048
#define M      4096
#define N      8192
#define O      16384
#define P      32768

#define Q      65536
#define R      131072
#define S      262144
#define T      524288
#define U      1048576
#define V      2097152
#define W      4194304
#define X      8388608

#define Y      16777216
#define Z      33554432
#define aa     67108864
#define bb     134217728
#define cc     268435456
#define dd     536870912
#define ee     1073741824
//#define ff     2147483648     
#define ff     -1073741824


/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC        (A)	/* Auto set for mobs      */
#define ACT_SENTINEL      (B)	/* Stays in one room      */
#define ACT_SCAVENGER     (C)	/* Picks up objects       */
#define ACT_RANGER        (D)
#define ACT_NO_SCENT      (E)
#define ACT_AGGRESSIVE    (F)	/* Attacks PC's           */
#define ACT_STAY_AREA     (G)	/* Won't leave area       */
#define ACT_WIMPY         (H)
#define ACT_PET           (I)	/* Auto set for pets      */
#define ACT_TRAIN         (J)	/* Can train PC's */
#define ACT_PRACTICE      (K)	/* Can practice PC's      */
#define ACT_HALT          (L)
#define ACT_HUNT          (M)
#define ACT_REMEMBER      (N)
#define ACT_UNDEAD        (O)
#define ACT_BARD          (P)
#define ACT_CLERIC        (Q)
#define ACT_MAGE          (R)
#define ACT_THIEF         (S)
#define ACT_WARRIOR       (T)
#define ACT_BOUNTY        (U)
#define ACT_NOPURGE       (V)
#define ACT_OUTDOORS      (W)
#define ACT_SMART_HUNT    (X)
#define ACT_INDOORS       (Y)
#define ACT_MOUNT         (Z)
#define ACT_IS_HEALER     (aa)
#define ACT_GAIN          (bb)
#define ACT_LOOTER        (cc)
#define ACT_IS_CHANGER    (dd)
#define ACT_TAME          (ee)
//#define ACT_CROUPIER      (ff)	//Croupier for roullette game
//#define ACT_AGGRESSIVE_ALL (gg) //Attacks all PC's, no matter what level they or the mob are

//Adeon 7/1/03 -- act_flags2!
#define ACT_BLOCK_EXIT		(A)	//Mobs block passage in a certain direction
#define ACT_NOSINK		(B)	//can't sink in sinking rooms
#define ACT_ILLUSION		(C)     //Mob is created for/by the Illusion skill
#define ACT_NOMOVE		(D)     //Mob can't be pushed/dragged/teleported/fear/maelstrom
#define ACT_FAMILIAR            (E)     //Use in Necromancer's familiar
#define ACT_PUPPET		(F)     //Use in Bone Puppet chant (and any future stuff)
#define ACT_PUPPETEER	        (G)     //Set on the player when switched into puppet
#define ACT_MONK		(H)
#define ACT_NECROMANCER	        (I)
#define ACT_QUESTMASTER		(J)
#define ACT_QUESTMOB		(K)
#define ACT_AGGIE_EVIL		(L)
#define ACT_AGGIE_NEUTRAL	(M)
#define ACT_AGGIE_GOOD		(N)
#define ACT_TRANSFORMER		(O)
#define ACT_CROUPIER            (P)  //Croupier for roullette game
#define ACT_MAGE_SKILLS		(Q)
#define ACT_NO_KILL		(R)  //Cannot attack
#define ACT_NO_ALIGN		(S)  //Align doesn't change but you get no souls
#define ACT_TRAPMOB		(T) // Used for trap justice
#define ACT_NO_BLOCK		(U) // Iblis 9/12/04 - Used for mobs that shouldn't be affected by blocking mobs
#define ACT_STAY_SECTOR		(V) // Iblis 9/12/04 - Forces a mob to not change sectors
#define ACT_WARPED		(W) // Iblis 9/13/04 - Random damage and random 2 resistances
#define ACT_IMAGINARY		(X) // Iblis 9/28/04 - Imaginary mobs (only seen by CJ's)
#define ACT_SWITCHED		(Y) // Iblis 10/02/04 - For Mobs you create and "become".  This signals
					      //        that if they die, you die too
#define ACT_ALL_SKILLS		(Z)
#define ACT_ALWAYS_MIRRORED     (aa)  //Iblis 10/30/04 - Mobs that ALWAYS have mirror on

/* damage Classes */
#define DAM_NONE                0
#define DAM_BASH                1
#define DAM_PIERCE              2
#define DAM_SLASH               3
#define DAM_FIRE                4
#define DAM_COLD                5
#define DAM_LIGHTNING           6
#define DAM_ACID                7
#define DAM_POISON              8
#define DAM_NEGATIVE            9
#define DAM_HOLY                10
#define DAM_ENERGY              11
#define DAM_MENTAL              12
#define DAM_DISEASE             13
#define DAM_DROWNING            14
#define DAM_LIGHT               15
#define DAM_OTHER               16
#define DAM_HARM                17
#define DAM_CHARM               18
#define DAM_SOUND               19
//IBLIS 6/1/03 - Added for Monk Class
#define DAM_CLAWING             20
#define DAM_WRATH	        21
//Iblis 10/17/03 - Added for Kalians
#define DAM_VAMPIRIC	        22
//Iblis 11/15/03 - Added for the do_damage imm function
#define DAM_UNIQUE		23
//Iblis 9/27/04 - Added for the hell of it
#define DAM_WIND                24

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK         (A)
#define OFF_BACKSTAB            (B)
#define OFF_BASH                (C)
#define OFF_BERSERK             (D)
#define OFF_DISARM              (E)
#define OFF_DODGE               (F)
#define OFF_FADE                (G)
#define OFF_FAST                (H)
#define OFF_KICK                (I)
#define OFF_KICK_DIRT           (J)
#define OFF_PARRY               (K)
#define OFF_RESCUE              (L)
#define OFF_TAIL                (M)
#define OFF_TRIP                (N)
#define OFF_CRUSH               (O)
#define ASSIST_ALL              (P)
#define ASSIST_ALIGN	        (Q)
#define ASSIST_RACE    	     	(R)
#define ASSIST_PLAYERS      	(S)
#define ASSIST_GUARD        	(T)
#define ASSIST_VNUM	            (U)
#define OFF_CHARGE              (V)
#define OFF_SWEEP               (W)
#define OFF_THROWING            (X)
#define OFF_TRAMPLE             (Y)
#define OFF_UPPERCUT            (Z)
#define OFF_COUNTER		(aa)
#define ASSIST_EXACT_ALIGN	(bb)
#define OFF_KAI			(cc)
#define OFF_HELLBREATH		(dd)
#define OFF_STUN		(ee)



/* return values for check_imm */
#define IS_NORMAL           0
#define IS_IMMUNE           1
#define IS_RESISTANT        2
#define IS_VULNERABLE       3

/* IMM bits for mobs */
#define IMM_SUMMON              (A)
#define IMM_CHARM               (B)
#define IMM_MAGIC               (C)
#define IMM_WEAPON              (D)
#define IMM_BASH                (E)
#define IMM_PIERCE              (F)
#define IMM_SLASH               (G)
#define IMM_FIRE                (H)
#define IMM_COLD                (I)
#define IMM_LIGHTNING           (J)
#define IMM_ACID                (K)
#define IMM_POISON              (L)
#define IMM_NEGATIVE            (M)
#define IMM_HOLY                (N)
#define IMM_ENERGY              (O)
#define IMM_MENTAL              (P)
#define IMM_DISEASE             (Q)
#define IMM_DROWNING            (R)
#define IMM_LIGHT               (S)
#define IMM_SOUND               (T)
#define IMM_ENTANGLE		(U)
#define IMM_WOOD                (X)
#define IMM_SILVER              (Y)
#define IMM_IRON                (Z)
#define IMM_LEVELS		(aa)	//IBLIS 5/20/03 - This flag toggles if an aggressive mob is immune to a players levels
				     // (IE they are aggressive to all levels).  It was put here because ACT flags are maxed
#define IMM_BLIND	        (bb)
#define IMM_SLEEP		(cc)
#define IMM_ARROWS		(dd)
#define IMM_WIND		(ee)

/* RES bits for mobs */
#define RES_SUMMON              (A)
#define RES_CHARM               (B)
#define RES_MAGIC               (C)
#define RES_WEAPON              (D)
#define RES_BASH                (E)
#define RES_PIERCE              (F)
#define RES_SLASH               (G)
#define RES_FIRE                (H)
#define RES_COLD                (I)
#define RES_LIGHTNING           (J)
#define RES_ACID                (K)
#define RES_POISON              (L)
#define RES_NEGATIVE            (M)
#define RES_HOLY                (N)
#define RES_ENERGY              (O)
#define RES_MENTAL              (P)
#define RES_DISEASE             (Q)
#define RES_DROWNING            (R)
#define RES_LIGHT               (S)
#define RES_SOUND               (T)
#define RES_ENTANGLE            (U)
#define RES_WOOD                (X)
#define RES_SILVER              (Y)
#define RES_IRON                (Z)
#define RES_WIND		(ee)

/* VULN bits for mobs */
#define VULN_SUMMON             (A)
#define VULN_CHARM              (B)
#define VULN_MAGIC              (C)
#define VULN_WEAPON             (D)
#define VULN_BASH               (E)
#define VULN_PIERCE             (F)
#define VULN_SLASH              (G)
#define VULN_FIRE               (H)
#define VULN_COLD               (I)
#define VULN_LIGHTNING          (J)
#define VULN_ACID               (K)
#define VULN_POISON             (L)
#define VULN_NEGATIVE           (M)
#define VULN_HOLY               (N)
#define VULN_ENERGY             (O)
#define VULN_MENTAL             (P)
#define VULN_DISEASE            (Q)
#define VULN_DROWNING           (R)
#define VULN_LIGHT		(S)
#define VULN_SOUND		(T)
#define VULN_WOOD               (X)
#define VULN_SILVER             (Y)
#define VULN_IRON		(Z)
#define VULN_WIND		(ee)

/* body form */
#define FORM_EDIBLE             (A)
#define FORM_POISON             (B)
#define FORM_MAGICAL            (C)
#define FORM_INSTANT_DECAY      (D)
#define FORM_OTHER              (E)	/* defined by material bit */

/* actual form */
#define FORM_ANIMAL             (G)
#define FORM_SENTIENT           (H)
#define FORM_UNDEAD             (I)
#define FORM_CONSTRUCT          (J)
#define FORM_MIST               (K)
#define FORM_INTANGIBLE         (L)

#define FORM_BIPED              (M)
#define FORM_CENTAUR            (N)
#define FORM_INSECT             (O)
#define FORM_SPIDER             (P)
#define FORM_CRUSTACEAN         (Q)
#define FORM_WORM               (R)
#define FORM_BLOB		(S)

#define FORM_MAMMAL             (V)
#define FORM_BIRD               (W)
#define FORM_REPTILE            (X)
#define FORM_SNAKE              (Y)
#define FORM_DRAGON             (Z)
#define FORM_AMPHIBIAN          (aa)
#define FORM_FISH               (bb)
#define FORM_COLD_BLOOD		(cc)

/* body parts */
#define PART_HEAD               (A)
#define PART_ARMS               (B)
#define PART_LEGS               (C)
#define PART_HEART              (D)
#define PART_BRAINS             (E)
#define PART_GUTS               (F)
#define PART_HANDS              (G)
#define PART_FEET               (H)
#define PART_FINGERS            (I)
#define PART_EAR                (J)
#define PART_EYE		(K)
#define PART_LONG_TONGUE        (L)
#define PART_EYESTALKS          (M)
#define PART_TENTACLES          (N)
#define PART_FINS               (O)
#define PART_WINGS              (P)
#define PART_TAIL               (Q)
/* for combat */
#define PART_CLAWS              (U)
#define PART_FANGS              (V)
#define PART_HORNS              (W)
#define PART_SCALES             (X)
#define PART_TUSKS		(Y)
#define PART_FEATHERS	        (Z)
// #define PART_EYE	        (aa)

#define HAND_LEFT               (A)
#define HAND_RIGHT              (B)
#define HAND_AMBIDEXTROUS       (C)

/* Action flags for pseudo-portals */

#define PACT_ENTER               (A)
#define PACT_EXIT                (B)
#define PACT_CLIMB               (C)
#define PACT_PLAY                (D)
#define PACT_DESCEND             (E)
#define PACT_CRAWL               (F)
#define PACT_SCALE               (G)
#define PACT_JUMP                (H)
#define PACT_TUG                 (I)
#define PACT_RING                (J)
#define PACT_SHOVE               (K)
#define PACT_SMASH               (L)
#define PACT_DIG		 (M)

#define BUTTON_PRESS             (A)
#define BUTTON_PULL              (B)

#define TACT_DRIVE               (A)
#define TACT_SAIL                (B)
#define TACT_PADDLE              (C)
#define TACT_ROW                 (D)

#define SKILL_NORMAL             (A)
#define SKILL_CAST               (B)
#define SKILL_SING               (C)
#define SKILL_PRAY               (D)
#define SKILL_CHANT              (E)
#define SKILL_PROJECT            (F)

#define COMP_ARIA                 1
#define COMP_BALLAD               2
#define COMP_BATTLESONG           3
#define COMP_DIRGE                4
#define COMP_SONNET               5
#define COMP_HYMN                 6
#define COMP_SERENADE             7
#define COMP_MELODY               8
#define COMP_SATIRE               9
#define COMP_SYMPHONY            10
#define COMP_JIG                 11
#define COMP_WARSONG             12
#define COMP_OPERA               13
#define COMP_LULLABY             14
#define COMP_SOLO                15
#define COMP_WALTZ               16
#define COMP_CHORAL              17
#define COMP_MEDLEY              18
#define COMP_CONCERTO            19
#define COMP_BLUES               20
#define COMP_CHANT               21
#define COMP_REQUIEM             22
#define COMP_HARMONY             23

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND		(A)
#define AFF_INVISIBLE		(B)
#define AFF_DETECT_EVIL		(C)
#define AFF_DETECT_INVIS	(D)
#define AFF_DETECT_MAGIC	(E)
#define AFF_DETECT_HIDDEN	(F)
#define AFF_DETECT_GOOD		(G)
#define AFF_SANCTUARY		(H)
#define AFF_FAERIE_FIRE		(I)
#define AFF_INFRARED		(J)
#define AFF_CURSE		(K)
#define AFF_CAMOUFLAGE          (L)
#define AFF_POISON		(M)
#define AFF_PROTECT_EVIL	(N)
#define AFF_PROTECT_GOOD	(O)
#define AFF_SNEAK		(P)
#define AFF_HIDE		(Q)
#define AFF_SLEEP		(R)
#define AFF_CHARM		(S)
#define AFF_FLYING		(T)
#define AFF_PASS_DOOR		(U)
#define AFF_HASTE		(V)
#define AFF_CALM		(W)
#define AFF_PLAGUE		(X)
#define AFF_WEAKEN		(Y)
#define AFF_DARK_VISION		(Z)
#define AFF_BERSERK		(aa)
#define AFF_SWIM		(bb)
#define AFF_REGENERATION        (cc)
#define AFF_SLOW		(dd)
#define AFF_AQUA_BREATHE        (ee)

/*
 * Sex.
 * Used in #MOBILES.
 */

#define SEX_NEUTRAL		      0
#define SEX_MALE				1
#define SEX_FEMALE		      2

/* AC types */
#define AC_PIERCE				0
#define AC_BASH				1
#define AC_SLASH				2
#define AC_EXOTIC				3

/* dice */
#define DICE_NUMBER			0
#define DICE_TYPE				1
#define DICE_BONUS			2

/* size */
#define SIZE_TINY				0
#define SIZE_SMALL			1
#define SIZE_MEDIUM			2
#define SIZE_LARGE			3
#define SIZE_HUGE				4
#define SIZE_GIANT			5

/* Necromancer familiar types */
#define FAMILIAR_BANSHEE	       0
#define FAMILIAR_ZOMBIE                1
#define FAMILIAR_SKELETON              2 
#define FAMILIAR_IMAGINARY_FRIEND      3

/* Voting Definitions */
#define VOTED_NOT		       0
#define VOTED_YES                      1
#define VOTED_NO		       2

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_SILVER_ONE	       1
#define OBJ_VNUM_GOLD_ONE	       2
#define OBJ_VNUM_GOLD_SOME	       3
#define OBJ_VNUM_SILVER_SOME	       4
#define OBJ_VNUM_COINS		       5
#define OBJ_VNUM_RUNE_CONV             6
#define OBJ_VNUM_CLAN_SYMBOL           7
#define OBJ_VNUM_BUTCHER_MEAT          8
#define OBJ_VNUM_COIN_ONE              9
#define OBJ_VNUM_CORPSE_NPC	      10
#define OBJ_VNUM_CORPSE_PC	      11
#define OBJ_VNUM_SEVERED_HEAD	      12
#define OBJ_VNUM_TORN_HEART	      13
#define OBJ_VNUM_SLICED_ARM	      14
#define OBJ_VNUM_SLICED_LEG	      15
#define OBJ_VNUM_GUTS		      16
#define OBJ_VNUM_BRAINS		      17
#define OBJ_VNUM_FEATHER	      18
#define OBJ_VNUM_SHAFT                19
#define OBJ_VNUM_MUSHROOM	      20
#define OBJ_VNUM_LIGHT_BALL	      21
#define OBJ_VNUM_SPRING		      22
#define OBJ_VNUM_DISC		      23
#define OBJ_VNUM_TREE		      24
#define OBJ_VNUM_PORTAL		      25
#define OBJ_VNUM_FISHIE_SMALL         26
#define OBJ_VNUM_FISHIE_MEDIUM        27
#define OBJ_VNUM_FISHIE_LARGE         28
#define OBJ_VNUM_FISHIE_POISON        29
#define OBJ_VNUM_BEGIN_GFORAGE        30

#define OBJ_VNUM_END_GFORAGE          35
#define OBJ_VNUM_BLUEGREEN_HERBS      36
#define OBJ_VNUM_SPROUTS              37
#define OBJ_VNUM_RED_BERRY            38
#define OBJ_VNUM_LITAN_FOOTSTEPS      38
#define OBJ_VNUM_SHELF                39
#define OBJ_VNUM_GENERIC_MAP          40
#define OBJ_VNUM_WALL_THORNS          41
#define OBJ_VNUM_ROSE		      42
#define OBJ_VNUM_CAMPFIRE1            43
#define OBJ_VNUM_CAMPFIRE2            44
#define OBJ_VNUM_CAMPFIRE3            45
#define OBJ_VNUM_TRIPWIRE             46
#define OBJ_VNUM_ORE                  47
#define OBJ_VNUM_ARROW                47
#define OBJ_VNUM_ROULETTE             48
#define OBJ_VNUM_STICKS               49
#define OBJ_VNUM_GRINDER              50
#define OBJ_VNUM_PENTAGRAM	      54
#define OBJ_VNUM_WOODEN_ARROWHEAD     55
#define OBJ_VNUM_WOOD		      56
#define OBJ_VNUM_HEXAGRAM	      57
#define OBJ_VNUM_SEPTAGRAM	      58
#define OBJ_VNUM_EYE		      61
#define OBJ_VNUM_WOOD_OBJ	      64
#define OBJ_VNUM_SPIDER_VINE_ARMS     65
#define OBJ_VNUM_ANTE_BAG	      66
#define OBJ_VNUM_CJ_SHARED_BOX	      67
#define	OBJ_VNUM_REAVER_SWORD	      69	/*Reaver Sword Prototype */
#define OBJ_VNUM_BINDER		      78
#define OBJ_VNUM_DECK		      79
#define OBJ_VNUM_SPIDER_VINE_WRISTS   82
#define OBJ_VNUM_WHISTLE	    2116
#define OBJ_VNUM_START_FOOD         4059
#define OBJ_VNUM_SCHOOL_MACE	    4194
#define OBJ_VNUM_SCHOOL_DAGGER      4195
#define OBJ_VNUM_SCHOOL_SWORD	    4196
#define OBJ_VNUM_SCHOOL_STAFF	    4197
#define OBJ_VNUM_SCHOOL_AXE	    4198
#define OBJ_VNUM_SCHOOL_POLEARM     4199
#define OBJ_VNUM_SCHOOL_VEST	    4200
#define OBJ_VNUM_SCHOOL_SHIELD      4201
#define OBJ_VNUM_SCHOOL_BANNER      4202
#define OBJ_VNUM_SCHOOL_BOOTS       4262
#define OBJ_VNUM_SCHOOL_LEGS        4263
#define OBJ_VNUM_MANIFESTO          4264
#define OBJ_VNUM_SCHOOL_FLAIL	    4405
#define OBJ_VNUM_MAP		    4406
#define OBJ_VNUM_START_DRINK        4407
#define OBJ_VNUM_ZTICKET            4415
#define OBJ_VNUM_PIT                4500
#define OBJ_VNUM_BANDAGE            4579
#define OBJ_VNUM_SAP               20382
#define OBJ_VNUM_ROPE              20413
#define OBJ_VNUM_ASHIELD           23000
#define OBJ_VNUM_ASWORD            23001
#define OBJ_VNUM_ADAGGER           23002
#define OBJ_VNUM_AMACE             23003
#define OBJ_VNUM_ASTAFF            23012
#define OBJ_VNUM_ABOOTS            23004
#define OBJ_VNUM_AGLOVES           23005
#define OBJ_VNUM_ALIGHT            23006
#define OBJ_VNUM_ASLEEVES          23007
#define OBJ_VNUM_ABELT             23008
#define OBJ_VNUM_ALEGGINGS         23009
#define OBJ_VNUM_ATICKET           23010
#define OBJ_VNUM_APOLEARM          23014
#define OBJ_VNUM_AINST             23015
#define OBJ_VNUM_TROPHY_FISH       28268
#define OBJ_VNUM_TROPHY_BAIT	   28269
#define OBJ_VNUM_BC_SCYTHE         30005
#define OBJ_VNUM_BC_SADDLEBAGS     30006
#define OBJ_VNUM_BC_DAGGER         30007
#define OBJ_VNUM_BC_NECKLACE	   30008
#define OBJ_VNUM_RIB_BONE	   30009
#define OBJ_VNUM_THIGH_BONE	   30010
#define OBJ_VNUM_SINEW             30011
#define OBJ_VNUM_SKIN		   30012
#define OBJ_VNUM_BEACON		   30013
#define OBJ_VNUM_DARK_SPHERE	   30014
#define OBJ_VNUM_DEFILE		   30015
#define OBJ_VNUM_ICE_STATUE	   30027
#define OBJ_VNUM_FLOATING_LETTERS  30028

//Default wooden objects for woodworking
#define OBJ_VNUM_WOOD_DEFAULT	   30029   //The object used to signal something is being constructed
#define OBJ_VNUM_WOOD_CHAIR	   30030
#define OBJ_VNUM_WOOD_CUP	   30031
#define OBJ_VNUM_PLAQUE		   30038
#define OBJ_VNUM_COMMON_CARD       30403
#define OBJ_VNUM_WARPED_SPACE 	   32517
#define OBJ_VNUM_IMAGINARY_GARB	   32518
#define OBJ_VNUM_IMAGINARY_LIGHT   32519

#define OBJ_VNUM_NECRO1		   7202
#define OBJ_VNUM_NECRO2            7203
#define OBJ_VNUM_NECRO3           22028
#define OBJ_VNUM_NERIX1           16344
#define OBJ_VNUM_NERIX2           10382
#define OBJ_VNUM_NERIX3           11505



#define RARITY_COMMON   	       1
#define RARITY_SOMEWHAT_COMMON         2
#define RARITY_UNCOMMON	               3
#define RARITY_RARE	               4
#define RARITY_VERY_RARE	       5
#define RARITY_ALWAYS   	       6


#define RECRUIT_KEEPER               (A)


/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT                      1
#define ITEM_SCROLL                     2
#define ITEM_WAND                       3
#define ITEM_STAFF                      4
#define ITEM_WEAPON                     5
#define ITEM_SADDLE                     6
#define ITEM_VIAL                       7
#define ITEM_TREASURE                   8
#define ITEM_ARMOR                      9
#define ITEM_POTION                    10
#define ITEM_CLOTHING                  11
#define ITEM_FURNITURE                 12
#define ITEM_TRASH                     13
#define ITEM_PARCHMENT                 14
#define ITEM_CONTAINER                 15
#define ITEM_ELEVATOR                  16
#define ITEM_DRINK_CON                 17
#define ITEM_KEY                       18
#define ITEM_FOOD                      19
#define ITEM_MONEY                     20
#define ITEM_EBUTTON                   21
#define ITEM_RAFT                      22
#define ITEM_CORPSE_NPC                23
#define ITEM_CORPSE_PC                 24
#define ITEM_FOUNTAIN                  25
#define ITEM_PILL                      26
#define ITEM_PROTECT                   27
#define ITEM_MAP                       28
#define ITEM_PORTAL                    29
#define ITEM_WARP_STONE                30
#define ITEM_ROOM_KEY                  31
#define ITEM_GEM                       32
#define ITEM_JEWELRY                   33
#define ITEM_JUKEBOX                   34
#define ITEM_GILLS                     35
#define ITEM_PACK                      36
#define ITEM_STRANSPORT                37
#define ITEM_CTRANSPORT                38
#define ITEM_INSTRUMENT                39
#define ITEM_WRITING_INSTRUMENT        40
#define ITEM_WRITING_PAPER             41
#define ITEM_CTRANSPORT_KEY            42
#define ITEM_PORTAL_BOOK               45
#define ITEM_CLAN_DONATION             46
#define ITEM_PLAYER_DONATION           47
#define ITEM_NEWCLANS_DBOX             48
// Akamai 5/1/99 - All traps share the same item type
#define ITEM_TRAP                      49
// Iblis 7/19/03 - Added for Archery
#define ITEM_QUIVER		       50
#define ITEM_FEATHER		       51
#define ITEM_POLE                      52
#define ITEM_BAIT		       53
#define ITEM_TREE		       54
#define ITEM_WOOD		       55
#define ITEM_SEED		       56
#define ITEM_WOODEN_INCONSTRUCTION     57
#define ITEM_CARD		       58
#define ITEM_BINDER		       59
#define ITEM_OBJ_TRAP		       60
#define ITEM_PORTAL_TRAP	       61	
#define ITEM_ROOM_TRAP		       62
#define ITEM_RANDOM		       63

//Paarshad 06/21/04
// Trap flags

#define TFLAG_NODISARM 			(A)
#define TFLAG_REARM 			(B)
#define TFLAG_UNAVOID			(C)
#define TFLAG_AREA			(D)
#define TFLAG_EXCELLENT			(E)
#define TFLAG_POISON			(F)
#define TFLAG_KNOCK			(G)
#define TFLAG_SNARE			(H)
#define TFLAG_BEE			(I)
#define TFLAG_DISORIENT			(J)
#define TFLAG_NODETECT			(J)

//Trap types
//I left room to grow


//Object traps

#define MIN_OBJ_TRAP			1
#define TRAP_O_BLADE			1
#define TRAP_O_NEEDLE			2
#define TRAP_O_BEE			3
#define TRAP_O_EXPLODING		4
#define TRAP_O_SPELL			5
#define TRAP_O_PLAIN			6
#define MAX_OBJ_TRAP			6

//Portal traps
#define MIN_PORTAL_TRAP			20
#define TRAP_P_ROCK			20
#define TRAP_P_DART			21
#define TRAP_P_EXPLODING		22
#define TRAP_P_SPIKES			23
#define TRAP_P_SPELL			24
#define TRAP_P_ALARM			25
#define TRAP_P_PLAIN			26
#define MAX_PORTAL_TRAP			26
//Room traps

#define MIN_ROOM_TRAP			40
#define TRAP_R_SPIKEPIT			40
#define TRAP_R_LOG			41
#define TRAP_R_LANDMINE			42
#define TRAP_R_SPIKESNARE		43
#define TRAP_R_SPELL			44
#define TRAP_R_ALARM			45
#define TRAP_R_PLAIN			46
#define MAX_ROOM_TRAP			46


//Iblis 04/04/04 - Added to different tree types
#define TREE_TYPE_OAK		 	1
#define TREE_TYPE_MAPLE			2
#define TREE_TYPE_BIRCH			3
#define TREE_TYPE_PINE			4	
#define TREE_TYPE_WILLOW		5
#define TREE_TYPE_ELM			6
#define TREE_TYPE_KARRI			7
#define TREE_TYPE_MARRI			8
#define TREE_TYPE_JARRAH		9
#define TREE_TYPE_POPLAR		10
#define TREE_TYPE_MOONBLOSSOM		11
#define TREE_TYPE_SLIMEWOOD		12
#define TREE_TYPE_RISIRIEL		13
#define TREE_TYPE_JEWELWOOD		14
#define TREE_TYPE_ROTSTENCH		15
#define TREE_TYPE_IRONWOOD		16
#define TREE_TYPE_HELLROOT		17
#define TREE_TYPE_BARBVINE		18
#define TREE_TYPE_GREEN_LEECHLEAF	19

#define MAX_TREES			19

#define TREE_SAPLING			1
#define TREE_SMALL			2
#define TREE_MEDIUM			3
#define TREE_LARGE			4
#define TREE_HUGE			5
//#define TREE_DEAD			6

#define TREE_VALUE_FALLEN	      	A
#define TREE_VALUE_ROTTEN		B


#define WOODEN_OBJ_CHAIR		0
#define WOODEN_OBJ_CUP			1
#define WOODEN_OBJ_PLAQUE		2
#define WOODEN_OBJ_CLUB			3
#define WOODEN_OBJ_DAGGER		4
#define WOODEN_OBJ_POLEARM		5
#define WOODEN_OBJ_STAFF		6
#define WOODEN_OBJ_SWORD		7
#define WOODEN_OBJ_SHIELD		8

//Iblis - 06/07/04 - Added for card types

#define CARD_MOB		  	1
#define CARD_TERRAIN			2
#define CARD_SPELL_HEAL			3
#define CARD_SPELL_DAMAGE		4
#define CARD_SPELL_BOOST		5
#define CARD_SPELL_COUNTER		6

//Iblis - 06/07/04 - Added for card terrain types

#define CARD_TERRAIN_CITY		1
#define CARD_TERRAIN_SWAMP              2
#define CARD_TERRAIN_FOREST             3
#define CARD_TERRAIN_MOUNTAIN           4
#define CARD_TERRAIN_SEA                5
#define CARD_TERRAIN_DESERT             6
#define CARD_TERRAIN_AIR                7
#define CARD_TERRAIN_PLAINS             8

#define CARD_RACE_DRAGON		1
#define CARD_RACE_HUMANOID              2
#define CARD_RACE_MONSTER               3
#define CARD_RACE_BIRD                  4
#define CARD_RACE_INSECT                5
#define CARD_RACE_ANIMAL                6
#define CARD_RACE_FISH                  7
#define CARD_RACE_UNDEAD                8
#define CARD_RACE_MAGIC                 9

#define CARD_ELEMENT_NONE		0
#define CARD_ELEMENT_LIGHTNING		1
#define CARD_ELEMENT_WATER		2
#define CARD_ELEMENT_FIRE		3
#define CARD_ELEMENT_STONE		4

#define CARD_TT_NODAMAGE		A
#define CARD_TT_NOHEAL			B
#define CARD_TT_ELEMENTALPLUS		C
#define CARD_TT_ELEMENTALMINUS		D
#define CARD_TT_ALLPLUS			E
#define CARD_TT_ALLMINUS		F
#define CARD_TT_NOCOUNTER		G
#define CARD_TT_NOBOOST			H

#define VALUE_MOB_RACE			10
#define VALUE_MOB_DEFENCE		9
#define VALUE_MOB_OFFENCE		8
#define VALUE_MOB_HP			7


#define CARDGAME_PLAYED_TERRAIN		64
#define CARDGAME_BEEN_ATTACKED		128
//#define CARDGAME_ST_ATTACKER		256
//
#define CARDGAME_MAX_MOBS_PLAYABLE	4

#define CARDGAME_MOB_HP			0
#define CARDGAME_MOB_OFF		1
#define CARDGAME_MOB_DEF		2

//Iblis 8/31/04 - Added for quest bits variable
#define MAX_QUEST_BITS		      1000



#define SCENT_STRONG                  3
#define SCENT_NOTICEABLE              2
#define SCENT_WEAK                    1

#define PORTAL_FUCKED                 -42

/*
 * Extra flags[0]
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		(A)
#define ITEM_HUM		(B)
#define ITEM_DARK		(C)
#define ITEM_LOCK		(D)
#define ITEM_EVIL		(E)
#define ITEM_INVIS		(F)
#define ITEM_MAGIC		(G)
#define ITEM_NODROP		(H)
#define ITEM_BLESS		(I)
#define ITEM_ANTI_GOOD		(J)
#define ITEM_ANTI_EVIL		(K)
#define ITEM_ANTI_NEUTRAL	(L)
#define ITEM_NOREMOVE		(M)
#define ITEM_INVENTORY		(N)
#define ITEM_NOPURGE		(O)
#define ITEM_ROT_DEATH		(P)
#define ITEM_VIS_DEATH		(Q)
#define ITEM_NOSAC		(R)
#define ITEM_NONMETAL		(S)
#define ITEM_NOLOCATE		(T)
#define ITEM_MELT_DROP		(U)
#define ITEM_HAD_TIMER		(V)
#define ITEM_SELL_EXTRACT	(W)
#define ITEM_DONATE             (X)
#define ITEM_BURN_PROOF		(Y)
#define ITEM_NOUNCURSE		(Z)
#define ITEM_NOIDENTIFY         (aa)
#define ITEM_PERMSTOCK          (bb)	/* For player-store inventory */
#define ITEM_NOSAVE             (cc)	//Adeon 6/29/03 -- object drops to floor on quit
#define ITEM_NOMOB		(dd)    //Iblis 1/24/04 -- objects can't be given/pick up by a mob
#define ITEM_NOENCHANT		(ee)    //Iblis 4/01/04 -- objects can't be enchanted

//Iblis - new extra flags
/*
 * Extra Flags[1]
 * Used in #OBJECTS.
 */

#define ITEM_NOSTOCK		(A)
#define ITEM_NODONATE		(B)
#define ITEM_NOAUCTION		(C)
#define ITEM_TATTOO		(D)   //Iblis 6/10/04 - An item that persists upon death
#define ITEM_WIZI		(E)
#define ITEM_IMAGINARY		(F)   //Iblis 9/28/04 - Imaginary (only seen by CJ) item
#define ITEM_WINGED		(G)   //IBlis 9/29/04 - CJ spell
#define ITEM_NOAVATAR		(H)   //Iblis 10/03/04 - Iverath request
#define ITEM_NOKALIAN		(I)   //Iblis 10/25/04 - Iverath request
#define ITEM_LIGHTNING_PROOF	(J)   //Iblis 10/25/04 - Iverath request
#define ITEM_COLD_PROOF 	(K)   //Iblis 10/25/04 - Iverath request
#define ITEM_ACID_PROOF    	(L)   //Iblis 10/25/04 - Iverath request
#define ITEM_INVIS_WEAR		(M)   //Iblis 10/27/04 - makes no wear message appear
#define ITEM_PULSATING		(N)   //Iblis 11/05/04 - can "see" (feel/hear) the item in the dark


/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		(A)
#define ITEM_WEAR_FINGER	(B)
#define ITEM_WEAR_NECK		(C)
#define ITEM_WEAR_BODY		(D)
#define ITEM_WEAR_HEAD		(E)
#define ITEM_WEAR_LEGS		(F)
#define ITEM_WEAR_FEET		(G)
#define ITEM_WEAR_HANDS		(H)
#define ITEM_WEAR_ARMS		(I)
#define ITEM_WEAR_SHIELD	(J)
#define ITEM_WEAR_ABOUT		(K)
#define ITEM_WEAR_WAIST		(L)
#define ITEM_WEAR_WRIST		(M)
#define ITEM_WIELD		(N)
#define ITEM_HOLD		(O)
#define ITEM_NO_SAC		(P)
#define ITEM_WEAR_FLOAT		(Q)
#define ITEM_TWO_HANDS          (R)
#define ITEM_WEAR_ANOTHER_FINGER (S)
#define ITEM_WEAR_ANOTHER_NECK	(T)
#define ITEM_WEAR_ANOTHER_WRIST (U)
#define ITEM_WEAR_LIGHT		(V)
#define ITEM_WEAR_TATTOO	(W) //Iblis 10/26/04 - New Wear Slot
#define ITEM_WEAR_FACE		(X) //Iblis 10/26/04 - New Wear Slot
#define ITEM_WEAR_EARS		(Y) //Iblis 10/26/04 - New Wear Slot
#define ITEM_WEAR_CREST		(Z) //Iblis 10/26/04 - New Wear Slot
#define ITEM_WEAR_CLAN_MARK	(aa) //Dither 11/10/08 - New Wear Slot
#define	ITEM_WORN_WINGS		(bb)

#define MAX_WEAR_FLAG 		ITEM_WEAR_CLAN_MARK


/* weapon Class */
#define WEAPON_EXOTIC		0
#define WEAPON_SWORD		1
#define WEAPON_DAGGER		2
#define WEAPON_STAFF		3
#define WEAPON_MACE		4
#define WEAPON_AXE		5
#define WEAPON_FLAIL		6
#define WEAPON_POLEARM		7
//Iblis 07/19/03 - Added for archery
#define WEAPON_SHORTBOW		8
#define WEAPON_LONGBOW	        9
#define WEAPON_ARROW		10
#define WEAPON_ARROWHEAD	11
#define WEAPON_DICE		12

/* weapon types */
#define WEAPON_FLAMING		(A)
#define WEAPON_FROST		(B)
#define WEAPON_VAMPIRIC		(C)
#define WEAPON_SHARP		(D)
#define WEAPON_VORPAL		(E)
#define WEAPON_TWO_HANDS	(F)
#define WEAPON_SHOCKING		(G)
#define WEAPON_POISON		(H)
#define WEAPON_LANCE            (I)
#define WEAPON_THROWING         (J)
#define WEAPON_APATHY		(K) //Added for Reaver's Apathy blade
#define WEAPON_WINDSLASH	(L)
#define WEAPON_LARVA		(M)
#define WEAPON_HOLY		(N)
#define WEAPON_HOLY_ACTIVATED	(O)
//ABOVE IS ONLY AN INT! DO NOT GO HIGHER THAN P (can it even do P?)

/* gate flags */
#define GATE_NORMAL_EXIT	(A)
#define GATE_NOCURSE		(B)
#define GATE_GOWITH		(C)
#define GATE_BUGGY		(D)
#define GATE_RANDOM		(E)
#define GATE_TRANSPORT		(F)
#define GATE_SHOWEXIT		(G)

/* furniture flags */
#define STAND_AT		(A)
#define STAND_ON		(B)
#define STAND_IN		(C)
#define SIT_AT			(D)
#define SIT_ON			(E)
#define SIT_IN			(F)
#define REST_AT			(G)
#define REST_ON			(H)
#define REST_IN			(I)
#define SLEEP_AT		(J)
#define SLEEP_ON		(K)
#define SLEEP_IN		(L)
#define PUT_AT			(M)
#define PUT_ON			(N)
#define PUT_IN			(O)
#define PUT_INSIDE		(P)

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE		      0
#define APPLY_STR		      1
#define APPLY_DEX		      2
#define APPLY_INT		      3
#define APPLY_WIS		      4
#define APPLY_CON		      5
#define APPLY_SEX		      6
#define APPLY_CLASS		      7
#define APPLY_LEVEL		      8
#define APPLY_AGE		      9
#define APPLY_HEIGHT		     10
#define APPLY_WEIGHT		     11
#define APPLY_MANA		     12
#define APPLY_HIT		     13
#define APPLY_MOVE		     14
#define APPLY_GOLD		     15
#define APPLY_EXP		     16
#define APPLY_AC		     17
#define APPLY_HITROLL		     18
#define APPLY_DAMROLL		     19
#define APPLY_SAVES		     20
#define APPLY_SAVING_PARA	     20
#define APPLY_SAVING_ROD	     21
#define APPLY_SAVING_PETRI	     22
#define APPLY_SAVING_BREATH	     23
#define APPLY_SAVING_SPELL	     24
#define APPLY_SPELL_AFFECT	     25
#define APPLY_CHA                    26
#define APPLY_ALIGN                  27
// Iblis 1/06/03 - Allows specific skill %'s (or 0 for all skills/spells/etc)
// to be affected
#define APPLY_SKILL		     28
// Akamai 5/1/99 - This flag in songs affects slot will be used to
// apply generic aff_* through the use of composed songs
#define APPLY_SONG_AFFECTS         4000

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8
#define CONT_PUT_ON		     16
#define CONT_NOBASH                  32

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_TEMPLATE            1
#define ROOM_VNUM_LIMBO		      2
#define ROOM_VNUM_SLIMBO	      3
#define ROOM_VNUM_ELIMBO	      4
#define ROOM_VNUM_DV_LIMBO            5
#define ROOM_VNUM_REINC_ROOM          6
#define ROOM_VNUM_SWITCHED_LIMBO      8
#define ROOM_VNUM_PIT              4178
#define ROOM_VNUM_CHAT		   1200
#define ROOM_VNUM_TEMPLE	   4134
#define ROOM_VNUM_ZANNAH           4135
#define ROOM_VNUM_ALTAR		   4200
#define ROOM_VNUM_SCHOOL	   1178
#define ROOM_VNUM_BALANCE	   4500
#define ROOM_VNUM_CIRCLE	   4400
#define ROOM_VNUM_DEMISE	   4200
#define ROOM_VNUM_HONOR		   4300
#define ROOM_VNUM_HALL_LEGENDS     23000
#define ROOM_VNUM_SPECTATOR        23025
#define ROOM_VNUM_ASSASSIN         20475
#define ROOM_VNUM_SD_START	   17634
#define ROOM_VNUM_SD_END	   17635

#define ROOM_VNUM_PLAYER_START     25000
#define ROOM_VNUM_PLAYER_END       27000

#define ROOM_VNUM_HAVEN_START      12714
#define ROOM_VNUM_HAVEN_END        12913

#define ROOM_VNUM_JAIL_START       4255
#define ROOM_VNUM_JAIL_END         4266

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK		        (A)
#define ROOM_NOMOUNT            (B)
#define ROOM_NO_MOB		  (C)
#define ROOM_INDOORS		  (D)
#define ROOM_BLOODBATH          (E)
#define ROOM_STABLE		  (F)
#define ROOM_SILENT		  (G)
#define ROOM_ELEVATOR           (H)
#define ROOM_TRANSPORT          (I)
#define ROOM_PRIVATE		  (J)
#define ROOM_SAFE		        (K)
#define ROOM_SOLITARY		  (L)
#define ROOM_PET_SHOP		  (M)
#define ROOM_NO_RECALL		  (N)
#define ROOM_IMP_ONLY		  (O)
#define ROOM_GODS_ONLY		  (P)
#define ROOM_NOFLYMOUNT	        (Q)
#define ROOM_NEWBIES_ONLY	  (R)
#define ROOM_LAW		        (S)
#define ROOM_NOWHERE		  (T)
#define ROOM_ARENA              (U)
#define ROOM_SPECTATOR          (V)
#define ROOM_ARENA_REGISTRATION (W)
#define ROOM_NO_PUSH		  (X)
#define ROOM_BANK               (Y)
#define ROOM_BUILD              (Z)
#define BFS_MARK                (aa)
#define ROOM_NO_CHARMIE         (bb)
#define ROOM_NO_FLY             (cc)
#define ROOM_PIER               (dd)
#define ROOM_NOSWIMMOUNT        (ee)
#define ROOM_SHRINE             (ff)

// Adeon 6/30/03 -- Ran out of room flags, had to create new set
#define ROOM_SINKING            (A)
#define ROOM_NOVEHICLE		(B)	// Adeon 7/14/03
//Below flag makes the room dark even if you have a light
#define ROOM_ALWAYS_DARK	(C)
#define ROOM_NOMAGIC		(D)    //prevent transports from entering
#define ROOM_PKONLY		(E)
#define ROOM_WARPED		(F)   //Iblis - 9/29/04 - non-actwarped take double damage
#define ROOM_SOLO 		(G)   //Iblis - 10/05/04 - 1 player, many mobs only
#define ROOM_MIRROR		(H)   //Iblis - 10/25/04 - Directions are reversed
#define ROOM_ICE             	(I)   //Iblis - 10/25/04 - You slide (if not flying)
#define ROOM_SNOW	        (J)   //Iblis - 10/25/04 - Leave footprints (if not flying)

/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH		      0
#define DIR_EAST		      1
#define DIR_SOUTH		      2
#define DIR_WEST		      3
#define DIR_UP			      4
#define DIR_DOWN		      5

/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		      (A)
#define EX_CLOSED		      (B)
#define EX_LOCKED		      (C)
#define EX_PICKPROOF		      (F)
#define EX_NOPASS		      (G)
#define EX_EASY			      (H)
#define EX_HARD			      (I)
#define EX_INFURIATING		      (J)
#define EX_NOCLOSE		      (K)
#define EX_NOLOCK		      (L)
#define EX_NOBASH                     (M)
#define EX_BASHED                     (N)

#define COST_BUILD_CREATE                  0
#define COST_BUILD_REMODEL                 1
#define COST_BUILD_NAME                    2
#define COST_BUILD_DARK_FLAG               3
#define COST_BUILD_NOMOB_FLAG              4
#define COST_BUILD_NORECALL_FLAG           5
#define COST_BUILD_NOMOUNT_FLAG            6
#define COST_BUILD_PRIVATE_FLAG            7
#define COST_BUILD_INDOORS_FLAG            8
#define COST_BUILD_KEYWORD                 9
#define COST_BUILD_DOOR                   10
#define COST_BUILD_KEY                    11
#define COST_BUILD_CLAN_FLAG              12
#define COST_BUILD_STORE_FLAG             13
#define COST_BUILD_NOTELE_FLAG            14
#define COST_BUILD_PICKPROOF		  15
#define COST_BUILD_NOBASH		  16

#define KEY_SAVE                     (A)

/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE		      0
#define SECT_CITY		      1
#define SECT_FIELD		      2
#define SECT_FOREST		      3
#define SECT_HILLS		      4
#define SECT_MOUNTAIN		      5
#define SECT_WATER_SWIM		      6
#define SECT_WATER_NOSWIM	      7
#define SECT_UNUSED		      8
#define SECT_AIR		      9
#define SECT_DESERT		     10
#define SECT_UNDERWATER              11
#define SECT_GRASSLAND               12
#define SECT_TUNDRA                  13
#define SECT_SWAMP                   14
#define SECT_WASTELAND               15
#define SECT_WATER_OCEAN             16
#define SECT_SHORE		     17
#define SECT_UNDERGROUND	     18
#define SECT_MAX		     19

#define TSECT_INSIDE		      (A)
#define TSECT_CITY		      (B)
#define TSECT_FIELD		      (C)
#define TSECT_FOREST		      (D)
#define TSECT_HILLS		      (E)
#define TSECT_MOUNTAIN		      (F)
#define TSECT_WATER_SWIM	      (G)
#define TSECT_WATER_NOSWIM	      (H)
#define TSECT_UNUSED		      (I)
#define TSECT_AIR		      (J)
#define TSECT_DESERT		      (K)
#define TSECT_UNDERWATER              (L)
#define TSECT_GRASSLAND               (M)
#define TSECT_TUNDRA                  (N)
#define TSECT_SWAMP                   (O)
#define TSECT_WASTELAND               (P)
#define TSECT_WATER_OCEAN             (Q)
#define TSECT_SHORE		      (R)
#define TSECT_UNDERGROUND	      (S)

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE		     -1
#define WEAR_LIGHT		      0
#define WEAR_FINGER_L		      1
#define WEAR_FINGER_R		      2
#define WEAR_NECK_1		      3
#define WEAR_NECK_2		      4
#define WEAR_BODY		      5
#define WEAR_HEAD		      6
#define WEAR_LEGS		      7
#define WEAR_FEET		      8
#define WEAR_HANDS		      9
#define WEAR_ARMS		     10
#define WEAR_SHIELD		     11
#define WEAR_ABOUT		     12
#define WEAR_WAIST		     13
#define WEAR_WRIST_L		     14
#define WEAR_WRIST_R		     15
#define WEAR_WIELD_R		     16
#define WEAR_WIELD_L		     17
#define WEAR_HOLD		     18
#define WEAR_FLOAT		     19
#define WEAR_TATTOO		     20	 //Iblis 10/26/04 - New Wear Slot
#define WEAR_FACE	             21  //Iblis 10/26/04 - New Wear Slot
#define WEAR_EARS	             22	 //Iblis 10/26/04 - New Wear Slot
#define WEAR_CREST		     23	 //Iblis 10/26/04 - New Wear Slot
#define WEAR_CLAN_MARK		     24  //Dither 11/10/08
#define	WEAR_WORN_WINGS		     25
#define MAX_WEAR		     26

/*
 * Conditions.
 */
#define COND_DRUNK		      0
#define COND_FULL		      1
#define COND_THIRST		      2
#define COND_HUNGER		      3
#define COND_PILLS		      4

/*
 * Positions.
 */
#define POS_DEAD		      0
#define POS_MORTAL		      1
#define POS_INCAP		      2
#define POS_STUNNED		      3
#define POS_SLEEPING		      4
#define POS_RESTING		      5
//#define POS_CHOPPING		      6
//#define POS_MILLING		      7	
#define POS_SITTING		      6
#define POS_FIGHTING		      7
#define POS_STANDING		      8
#define POS_TETHERED		      9
#define POS_MOUNTED                  10
#define POS_FEIGNING_DEATH	     11
#define POS_COMA		     12
#define POS_CHOPPING                 13
#define POS_MILLING                  14
#define POS_CARVING	             15


/*
 * ACT bits for players.
 */
#define PLR_IS_NPC		(A)	/* Don't EVER set.        */
#define PLR_AUTORETURN          (B)
#define PLR_AUTOASSIST		(C)
#define PLR_AUTOEXIT		(D)
#define PLR_AUTOLOOT		(E)
#define PLR_AUTOSAC             (F)
#define PLR_AUTOGOLD		(G)
#define PLR_AUTOSPLIT		(H)





#define PLR_HOLYLIGHT		(N)
#define PLR_UNDEAD		(O)
#define PLR_CANLOOT		(P)
#define PLR_NOSUMMON		(Q)
#define PLR_NOFOLLOW		(R)
/* 2 bits reserved, S-T */
/* penalty flags */
#define PLR_PERMIT		(U)
#define PLR_GHOST		(V)
#define PLR_LOG			(W)
#define PLR_DENY		(X)
#define PLR_FREEZE		(Y)
#define PLR_WANTED		(Z)
#define PLR_JAILED		(aa)
//IBLIS 5/21/03 - Added by request
#define PLR_AUTOBUTCHER         (bb)
#define PLR_CARDKILL	        (cc)
//Iblis 10/02/04 - Added SWITCHOK instead of NOSWITCH due to the fact that be default you are not consenting
// 		   to switching
#define PLR_SWITCHOK		(dd)

#define COMM_QUIET              (A)
#define COMM_DEAF            	(B)
#define COMM_NOWIZ              (C)
#define COMM_NOAUCTION          (D)
#define COMM_NOGOSSIP           (E)
#define COMM_NOQUESTION         (F)
#define COMM_NOMUSIC            (G)
#define COMM_NOCLAN		(H)
#define COMM_NOOOC		(I)
#define COMM_SHOUTSOFF		(J)
#define COMM_TRUE_TRUST		(K)
#define COMM_COMPACT		(L)
#define COMM_BRIEF		(M)
#define COMM_PROMPT		(N)
#define COMM_COMBINE		(O)
#define COMM_TELNET_GA		(P)
#define COMM_SHOW_AFFECTS	(Q)
#define COMM_NOGRATS		(R)
#define COMM_NOINFO		(S)

/* penalties */
#define COMM_NOEMOTE		(T)
#define COMM_NOSHOUT		(U)
#define COMM_NOTELL		(V)
#define COMM_NOCHANNELS		(W)

/* later additions */
#define COMM_ANONYMOUS          (X)
#define COMM_SNOOP_PROOF	(Y)
#define COMM_AFK		(Z)
#define COMM_NOADMIN            (aa)
#define COMM_HEARCLAN           (bb)
#define COMM_NOTITLE            (cc)
#define COMM_NOSUPERDUEL        (dd)
#define COMM_NOPK               (ee)

/* comm2 flags */

#define COMM_NORANT		(A)
#define COMM_RANTONLY	        (B)
#define COMM_NOPOLL		(C)
#define COMM_NOGEMOTE		(D)
#define COMM_IMM_NOGEMOTE	(E)		//For BAD GEmoters
#define COMM_NODELETE		(F)
#define COMM_SHOWDAMAGE         (H)

/* comm2 penalities */

#define COMM_SHUTUP		(G) 


/* WIZnet flags */
#define WIZ_ON			(A)
#define WIZ_TICKS		(B)
#define WIZ_LOGINS		(C)
#define WIZ_SITES		(D)
#define WIZ_LINKS		(E)
#define WIZ_DEATHS		(F)
#define WIZ_RESETS		(G)
#define WIZ_MOBDEATHS		(H)
#define WIZ_FLAGS		(I)
#define WIZ_PENALTIES		(J)
#define WIZ_SACCING		(K)
#define WIZ_LEVELS		(L)
#define WIZ_SECURE		(M)
#define WIZ_SWITCHES		(N)
#define WIZ_SNOOPS		(O)
#define WIZ_RESTORE		(P)
#define WIZ_LOAD		(Q)
#define WIZ_NEWBIE		(R)
#define WIZ_PREFIX		(S)
#define WIZ_SPAM		(T)
#define WIZ_CLAN                (U)
#define WIZ_TSTAMP              (V)
#define WIZ_NOTES		(W)
#define WIZ_POLL		(X)
#define WIZ_QUEST		(Y)
#define WIZ_STOCKBUY		(Z)

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct mob_index_data
{
  MOB_INDEX_DATA *next;
  AREA_DATA *area;		/* OLC */
  SPEC_FUN *spec_fun;
  SHOP_DATA *pShop;
  int vnum;
  short group;
  long recruit_flags;
  int recruit_value[3];
  TRIGGER_DATA *triggers;
  VARIABLE_DATA *variables;
  char *script_fn;
  short count;
  short killed;
  short default_mood;
  char *player_name;
  char *short_descr;
  char *long_descr;
  char *description;
  char *vocfile;
  char *last_killer;
  char *last_killer2;
  long act;
  long act2;
  long affected_by;
  short alignment;
  /* For mounts */
  short defbonus;
  short attackbonus;
  short max_weight;
  /* End mounts */
  short level;
  short hitroll;
  short hit[3];
  short mana[3];
  short damage[3];
  short ac[4];
  short dam_type;
  long off_flags;
  long imm_flags;
  long res_flags;
  long vuln_flags;
  short start_pos;
  short default_pos;
  short sex;
  short Class;
  short race;
  long wealth;
  long form;
  long parts;
  short size;
  short move;
  char *material;
  int blocks_exit;
  int number_of_attacks;
  short perm_stat[MAX_STATS]; 
  int card_vnum;
};

struct talk_list
{
  char *word;
  struct talk_list *next;
};

struct response_list
{
  char wassaid[MAX_INPUT_LENGTH];
  char text[MAX_STRING_LENGTH];
  short new_mood;
  struct response_list *next;
};

struct comp_data
{
  COMP_DATA *next;
  char *name;
  char *title;
  int type;
  int subtype;
  int skill;
  int comp_levels;
};

struct memory_data
{
  MEMORY_DATA *next;
  MEMORY_DATA *next_memory;
  CHAR_DATA *player;
  CHAR_DATA *mob;
};

/*
 * One character (PC or NPC).
 */
struct char_data
{
  CHAR_DATA *next;
  CHAR_DATA *next_in_room;
  CHAR_DATA *next_in_board;
  CHAR_DATA *master;
  CHAR_DATA *stalking;
  CHAR_DATA *leader;
  CHAR_DATA *fighting;
  CHAR_DATA *aggres;
  CHAR_DATA *reply;
  CHAR_DATA *pet;
  CHAR_DATA *mount;
  COMP_DATA *compositions;
  CHAR_DATA *last_fought;
  MEM_DATA *memory;
  SPEC_FUN *spec_fun;
  MOB_INDEX_DATA *pIndexData;
  DESCRIPTOR_DATA *desc;
  AFFECT_DATA *affected;
  NOTE_DATA *pnote;
  OBJ_DATA *carrying;
  OBJ_DATA *on;
  ROOM_INDEX_DATA *in_room;
  ROOM_INDEX_DATA *was_in_room;
  AREA_DATA *zone;
  PC_DATA *pcdata;
  GEN_DATA *gen_data;
  MEMORY_DATA *hunt_memory;
  OBJ_DATA *trap_list;
  bool valid;
  bool color;
  char *name;
  char *mask;
  char *afkmsg;
  int walkto;
  int huntto;
  short clan_apply;
  long id;
  short version;
  char *tracking;
  char *short_descr;
  char *long_descr;
  char *description;
  char *prompt;
  char *prefix;
  int spec_tracer;
  TRIGGER_DATA *triggers;	/* for scripts           */
  VARIABLE_DATA *variables;	/* for scripts           */
  short group;
  short clan;
  short sex;
  short Class;
  short Class2;
  short Class3;
  short race;
  short level;
  short trust;
  short lh_ticks;
  short ex_ticks;
  short il_ticks;	//Uses for illusion (for Kalians), and Putrify (for Liches)
  int played;
  int lines;			/* for the pager */
  time_t logon;
  time_t last_swim_update;
  int want_jail_time;
  int loot_time;
  short times_wanted;
  short drown_damage;
  short timer;
  short wait;
  short daze;
  short hit;
  short max_hit;
  short mana;
  short max_mana;
  short move;
  short max_move;
  long gold;
  long silver;
  unsigned long exp;
  long act;
  long act2;			//Adeon 7/1/03
  long comm;			/* RT added to pad the vector */
  long comm2;                   // Iblis 1/08/04
  long wiznet;			/* wiz stuff */
  long imm_flags;
  long res_flags;
  long vuln_flags;
  short invis_level;
  short incog_level;
  long affected_by;
  short position;
  //short practice;
  short train;
  short carry_weight;
  short carry_number;
  short saving_throw;
  short alignment;
  short hitroll;
  short damroll;
  short armor[4];
  short wimpy;
  char *duel;
  short perm_stat[MAX_STATS];
  short mod_stat[MAX_STATS];
  long form;
  long parts;
  short size;
  char *material;
  long off_flags;
  short damage[3];
  short dam_type;
  short start_pos;
  short default_pos;
  CHAR_DATA *riders;
  CHAR_DATA *mounter;
  short mood;
  char *exit_msg;
  bool bashed;
//    CHAR_DATA *switched;
  OBJ_DATA *sword;		/*saves reaver sword data for recall */
  int sink_timer;
  int blocks_exit;		// Adeon 7/17/03 -- the exit that a mob blocks if any
  bool dueler;
  short ticks_remaining;
  CHAR_DATA *contaminator;      //Used for Lich Contaminator skill
  short stunned;
  int number_of_attacks;
  long recruit_flags;
  int recruit_value[3];
};


// Alignment Constants -- These are to help eliminate the several places
// where alignment constants are assigned in code.
// Akamai 6/4/98 -- Bug: #9 Alignment management
#define ALIGN_GROUP_SPHERE   600	// the max allowed difference in a group
#define ALIGN_MAX           1000
#define ALIGN_MIN          -1000
#define ALIGN_ANGELIC_MAX   1000
#define ALIGN_ANGELIC_MIN    901
#define ALIGN_SAINTLY_MAX    900
#define ALIGN_SAINTLY_MIN    701
#define ALIGN_GOOD_MAX       700
#define ALIGN_GOOD_MIN       351
#define ALIGN_KIND_MAX       350
#define ALIGN_KIND_MIN       101
#define ALIGN_NEUTRAL_MAX    100
#define ALIGN_NEUTRAL_MIN   -100
#define ALIGN_MEAN_MAX      -101
#define ALIGN_MEAN_MIN      -350
#define ALIGN_EVIL_MAX      -351
#define ALIGN_EVIL_MIN      -700
#define ALIGN_DEMONIC_MAX   -799
#define ALIGN_DEMONIC_MIN   -900
#define ALIGN_SATANIC_MAX   -901
#define ALIGN_SATANIC_MIN  -1000

// PC Race Constants -- These are defined relative to the pc_race_table
// in file const.c - If the race table is changed these will need to be
// updated. The goal of this change is to eliminate many of the race based
// magic constants that are scattered through the code. These constants
// will also help fix Bug: #10
// Akamai 6/4/98 -- Bug: #10 - Race grouping restrictions
#define PC_RACE_HUMAN    1
#define PC_RACE_ELF      2
#define PC_RACE_DWARF    3
#define PC_RACE_CANTHI   4
#define PC_RACE_VROATH   5
#define PC_RACE_SYVIN    6
#define PC_RACE_SIDHE    7
#define PC_RACE_AVATAR   8
#define PC_RACE_THYRENT  9
#define PC_RACE_LITAN	 9
#define PC_RACE_KALIAN   10
#define PC_RACE_LICH     11
#define PC_RACE_NERIX    12
#define PC_RACE_SWARM    13
#define PC_RACE_NIDAE    14

// PC Class Constants -- I'm tired of having to remember the number
// for each Class - Iblis 02/11/04
// NOTE NOTE NOTE : When I added this, I grepped for IS_CLASS and changed
// all the numbers I found to PC_CLASS_X.  I DO NOT guarantee that if you
// grep for PC_CLASS_X you will find all instances in the code in which a
// specific Class is referenced, so be careful if you change one of these
// numbers!
#define PC_CLASS_MAGE			0
#define PC_CLASS_CLERIC			1
#define PC_CLASS_THIEF			2
#define PC_CLASS_WARRIOR		3
#define PC_CLASS_RANGER			4
#define PC_CLASS_BARD			5
#define PC_CLASS_PALADIN		6
#define PC_CLASS_ASSASSIN		7
#define PC_CLASS_REAVER			8
#define PC_CLASS_MONK			9
#define PC_CLASS_NECROMANCER	10
#define PC_CLASS_SWARM			11
#define PC_CLASS_GOLEM			12
#define PC_CLASS_CHAOS_JESTER	13
#define PC_CLASS_ADVENTURER		14
#define	PC_CLASS_DRUID			15
#define PC_CLASS_EMPATH			16

#define PERS_NORMAL		0
#define PERS_AGGRESSIVE		1
#define PERS_PASSIVE		2



// PC Group Constant -- When players form a group, there is a maximum number
// of players allowed in the group.
#define MAX_PC_GROUP    32

// Weapon/Magic Level restrictions -- When a player uses a weapon or
// attempts to use a magic item they must be within a number of levels
// of the weapon or magic items skill level
// Akamai 6/4/98 -- Bug: #5 Parchment/Scroll and Quaff level restrictions
// Akamai 6/9/98 -- Bug: #12 Throw level restrictions
#define MAGIC_LEVEL_DIFF      100
#define EQUIP_LEVEL_DIFF      10
#define PLAYER_LEVEL_DIFF     100

// Fight lag -- There is a little lag on certain activities after a PC
// fights. This should be a constant, but in most cases now it is a magic
// number in the code.
#define FIGHT_LAG    120

// Grace Wanted -- After a player has received this number of wanted flags
// the code no longer removes the flag when he dies.  Shinowlan
#define GRACE_WANTED 15

//Iblis 6-28-03 Record to store a list of aggressors
struct aggressor_list
{
  AGGRESSOR_LIST *next;
  bool valid;
  CHAR_DATA *ch;
  short ticks_left;
  bool silent;
};

/*
 * Data which only PC's have.
 */
struct pc_data
{
  PC_DATA *next;
  BUFFER *buffer;
  //Iblis 6/28/03 - Added for Agressor List.  Not Saved^M
  AGGRESSOR_LIST *agg_list;
  bool valid;
  bool autoassist_level;
  char *pwd;
  char *bamfin;
  char *bamfout;
  char *whoinfo;
  char *pretitle;
  char *battlecry;
  char *vocab_file;
  char *plan;
  char *title;
  char *restoremsg;
  time_t lastlogoff;
  //last_kai is used to keep track of the last time kai was invoked.
  //We use this information in do_kai to determine the amount of lag
  // that should be applied to the player invoking kai.
  time_t last_kai;
  char *email_addr;
  bool deputy;
  bool anonymous;
  CHAR_DATA *boarded;
  char *speedwalk;
  int speedlen;
  long bank_gold;
  long bank_silver;
  short primary_hand;
  time_t last_fight;
//  time_t last_loot;
  time_t last_note;
  time_t last_idea;
  time_t last_penalty;
  time_t last_news;
  time_t last_changes;
  time_t last_legend;
  time_t last_oocnote;
  time_t last_projects;
  short perm_hit;
  short perm_mana;
  short perm_move;
  short true_sex;
  bool security;
  //Shinowlan -- 12/13/2001 -- added buildall to allow differentiation
  // between people who should be able to create areas (security =1) 
  // and people who can ALSO edit any area/room/mob.  A buildall value
  // of 1 supercedes security setting.
  bool buildall;
  int last_level;
  short condition[5];
  short learned[MAX_SKILL];
  short mod_learned[MAX_SKILL];
  bool group_known[MAX_GROUP];
  short points;
  bool confirm_delete;
  char *ignore[MAX_IGNORE];
  char *alias[MAX_ALIAS];
  char *alias_sub[MAX_ALIAS];
  char *history[MAX_HISTORY];
  int been_killed[5];
  int has_killed[5];
  int events_won[6];
  byte color_combat_s;
  byte color_combat_o;
  byte color_combat_condition_s;
  byte color_combat_condition_o;
  byte color_invis;
  byte color_hp;
  byte color_hidden;
  byte color_charmed;
  byte color_mana;
  byte color_move;
  byte color_say;
  byte color_tell;
  byte color_guild_talk;
  byte color_group_tell;
  byte color_music;
  byte color_gossip;
  byte color_auction;
  byte color_immtalk;
  byte color_admtalk;
  bool lost_prayers;
  bool loner;
  bool hero;
  int last_saved_room;
  char explored[MAX_EXPLORE];
  unsigned long gamestat[MAX_GAMESTAT];
  long souls;
  unsigned long totalsouls;
  short reaver_chant;
  int chant_wait;
  //IBLIS 5/18/03 - Needed to cap hp/mana/move gain at 300hp
  short hp_gained;
  short mana_gained;
  short move_gained;
  long group_exp;
  bool new_style;
  //IBLIS 5/31/03 - Added for monk skill feign (note : Not Saved)
  bool feigned;
  //IBLIS 5/31/03 - Added for monk skill prevent escape
  bool prevent_escape;
  //IBLIS 6/07/03 - Added for Holy Ceremony Prayer.  Neither are saved
  short last_death_counter;
  long xp_last_death;
  //IBLIS 6/20/03 - Added for Battle Royale.  Not Saved
  int br_points;
  //Iblis 8/03/03 - Added for Battyle Royale.  Not Saved
  int br_death_points;
  //Iblis 6/28/03 - Added for Agressor List.  Not Saved
  //AGGRESSOR_LIST *agg_list;
  //Iblis 6/29/03 - Added for Falconry alert;
  bool falcon_alert;
  //Iblis 6/29/03 - Added for Falconry deliver NOTE: none of the 3 are saved;
  short falcon_wait;
  OBJ_DATA *falcon_object;
  CHAR_DATA *falcon_recipient;
  short fishing;
  short fishing_weight;
  short fishing_ll;
  //Iblis 8/5/03 - Added for the everlasting Iverath vs Serenity battle.  It saves
  bool noblinking;
  //Iblis 10/16/03 - Added for impending rp, newbie helper, and qp system
  long rppoints;
  long nhpoints;
  long qpoints;
  short avatar_type;
  short old_race;
  char *ldhost;
  time_t last_pkdeath;
  long nochan_ticks;
  long shutup_ticks;
  bool nogate;
  short knock_time;
  //Iblis 1/04/04 - int used for ranger aggression skill NOT SAVED
  int aggression;
  //Iblis 1/06/04 - next three used for necro's corpse walk chant
  OBJ_DATA *corpse;
  ROOM_INDEX_DATA *corpse_to_room;
  short corpse_timer;
  //Iblis 1/07/03 - next three used for necromancer's familiar
  short familiar_type; 
  short familiar_level;
  int familiar_max_hit;
  int familiar_max_mana;
  int familiar_max_move;
  int familiar_gained_hp;
  int familiar_gained_mana;
  int familiar_gained_move;
  unsigned long familiar_exp;
  char* familiar_name;
  CHAR_DATA* familiar;
  CHAR_DATA* soul_link;
  short old_Class;
  bool flaming;
  unsigned long totalxp;
  unsigned long tg_won;
  unsigned long tg_lost;
  short questing;
  short elder;
  int qtype; 		//Paarsh 6/12/04 added for various types of autoquests
  OBJ_DATA *quest_obj;
  CHAR_DATA *quest_mob;
  int quest_ticks;
  long total_quest_finds;
  long total_qf_attempted;
  long total_quest_hunts;
  long total_qh_attempted;
  int other_deaths; 	//I did this separate from been killed because there isn't
  			//really a has_killed for this, and I wanted been/has killed to still sync
  int nplayed;
  int nage;
  //Iblis 2/08/05 - Used for Poll channel, NOT saved
  short voted;
  bool has_reincarnated;
  int oow_ticks;   //Iblis 2/18/04 - Out of Water Ticks (used for Nidae)
  short backup_hit;
  short backup_mana;
  short backup_move;       
  int pushed_mobs_counter;
  char* referrer;
  long total_trees_chopped;
  long total_trees_milled;
  OBJ_DATA *trade_skill_obj; //Iblis 4/12/04 - Obj the trade skill is working on (NOT SAVED)
  long total_objs_sold;
  long total_money_made;

  //Iblis 6/16/04 - Card Game Stuff.  I could have made a single pointer to a structure here,
  //in order to deal with all of the cardgame stuff, but I feel it's much safer this way
  //NOTE none of these are saved
  CHAR_DATA *cg_next;  //Iblis 6/15/04 - The person who plays next (in the card game)
  short cg_ticks;      //Iblis 6/15/04 - The amount of ticks left to start the card game
  //I also use the above value throughout the code when i need to store various integers
  //for things.  That's probably not good, but it saves on variable space
  short cg_spell_target;
  int cg_state;        //Iblis - 6/19/04 - The state of the card game they are in..sort
  		       //of like d->editor, except that this will not be reset when a 
		       //character goes linkdead
  short cg_player_number; //This is used so that even if people quit in the middle of the
  			  //card game, the listening of player_opponents will be the same
			  //regardless, so that spells and such will still point to the
			  //proper player
  time_t cg_timer;	//Used to check for a round time being up
  //Below are all pointers to cards
  OBJ_DATA* cg_main_mob;
  OBJ_DATA* cg_secondary_mob;
  OBJ_DATA* cg_terrain;
  OBJ_DATA* cg_lcp;   //last card played
  //Iblis - 6/24/04 - I thought a lot about hacking this using fishing variables etc, or just combining
  //several of the options into a single value, but then I figured wtf, I'm only adding 6 shorts per PC
  //(which isn't very much), so even though PCDATA is crowded, this makes things much simpler
  //I kept thinking about making these more general variables (maybe making a value[X] for all int/short
  //values for the cardgame, but I decided against that.  If you don't like that, blow me.
  short cg_mm_value[3];  //Cardgame,main mob,values 0-2
  short cg_sm_value[3];	 //Cardgame,main mob, value 0-2
  short cg_mobs_played;  //So I'm adding ANOTHER variable for this.  So @%!ing sue me
  short cg_max_mobs;     //Iblis 9/26/04 - allows a variable number of mobs to be played
  //End of Card Game Stuff
  //Iblis 8/31/04 - Added for quest bits
  char qbits[MAX_QUEST_BITS/8];
  long total_cg_won;
  long total_cg_lost;
  short sd_team; //Iblis 9/26/04 - added for superduel team fights - NOT SAVED
  int pain_points; //Iblis 10/03/04 - used for CJ pain points..NOT SAVED
  bool all_xp_familiar; //Iblis 10/03/04 - toggle to have imaginary friend take 50% or 100% of xp 
  short personality;
//  bool qbits[MAX_QUEST_BITS]; 
  short shillelagh;  
};

/* Data for generating characters -- only used during generation */
struct gen_data
{
  GEN_DATA *next;
  bool valid;
  bool skill_chosen[MAX_SKILL];
  bool group_chosen[MAX_GROUP];
  int points_chosen;
};

/*
 * Liquids.
 */
#define LIQ_WATER        0

struct liq_type
{
  char *liq_name;
  char *liq_color;
  short liq_affect[5];
};

/*
 * Extra description data for a room or object.
 */
struct extra_descr_data
{
  EXTRA_DESCR_DATA *next;	/* Next in list                     */
  bool valid;
  char *keyword;		/* Keyword in look/examine          */
  char *description;		/* What to see                      */
};

/* BEGIN ARENA */
struct duel_data
{
  CHAR_DATA *in_duel_1;
  OBJ_DATA *old_carry_1;
  short old_hit_1;
  short old_mana_1;
  short old_move_1;

  CHAR_DATA *in_duel_2;
  OBJ_DATA *old_carry_2;
  short old_hit_2;
  short old_mana_2;
  short old_move_2;
};
/* END ARENA */

//
// Prototype for an object.
//
// Akamai 4/30/99 - modified to support Class/race specific objects
//
struct obj_index_data
{
  OBJ_INDEX_DATA *next;
  AREA_DATA *area;
  EXTRA_DESCR_DATA *extra_descr;
  AFFECT_DATA *affected;
  char *name;
  char *short_descr;
  char *description;
  int vnum;
  short reset_num;
  char *material;
  short item_type;
  long extra_flags[MAX_EXTRA_FLAGS];
  long wear_flags;
  long race_flags;		// implements race exclusive objects
  long Class_flags;		// implements Class exclusive objects
  long clan_flags;		// implements clan type exclusive objects
  short level;
  short condition;
  short count;
  short weight;
  short rarity;
  int timer;
  int cost;
  int value[13];
  char *string1;
  char *string2;
  char *string3;
  char *string4;
  char *plr_owner;		//for player ownership of objects
  int obj_trig_vnum[MAX_OBJ_TRIGS];
};

struct elevator_dest_list
{
  ELEVATOR_DEST_LIST *next;
  int vnum;

  char *arrival_msg_i;
  char *arrival_msg_o;
  char *departure_msg_i;
  char *departure_msg_o;
};

//
// One object.
//
// Akamai 4/30/99 - modified to support Class/race specific objects
//
struct obj_data
{
  OBJ_DATA *next;
  OBJ_DATA *next_content;
  OBJ_DATA *contains;
  OBJ_DATA *in_obj;
  OBJ_DATA *on;
  CHAR_DATA *carried_by;
  CHAR_DATA *set_by;
  EXTRA_DESCR_DATA *extra_descr;
  AFFECT_DATA *affected;
  OBJ_INDEX_DATA *pIndexData;
  OBJ_DATA *next_trap;
  ROOM_INDEX_DATA *in_room;
  bool valid;
  bool enchanted;
  char *owner;
  char *name;
  char *short_descr;
  char *description;
  short item_type;
  long extra_flags[MAX_EXTRA_FLAGS];
  long wear_flags;
  long race_flags;		// implements race exclusive objects
  long Class_flags;		// implements Class exclusive objects
  long clan_flags;		// implements clan_type exclusive objects
  long wear_loc;
  long prev_wear_loc;
  short weight;
  int cost;
  short level;
  short condition;
  char *material;
  short timer;
  int value[13];
  int etime;
  ELEVATOR_DEST_LIST *dest_list;
  //Iblis - 6/7/04 - The following 4 variables are atrocious.  They should be store on in pcdata, NOT
  //on every single goddamn object.  But I know if I fix this, i won't get around to doing cards like
  //Iverath wants for a while, so wtf I'm using them.  So if someone finally fixes this injustice,
  //you will have to add some values to value[7] to deal with cards, since I currently use the follow
  //4 variables.  I am the shittiest programmer in the world for leaving this.
  //I take that back.  I'm the second shitties.  I'm not the one who made it like this.
  int bs_capacity[MAX_BLADE_SPELLS];
  int bs_charges[MAX_BLADE_SPELLS];
  int bb_capacity[MAX_BURST];
  int bb_charges[MAX_BURST];
  char *plr_owner;		//player that owns that piece of eq is only one who can use it
  OBJ_TRIG *objtrig[MAX_OBJ_TRIGS];
};

// One Trap
// Paarshad 6/20/04
//

struct trap_data
{
/*
	victim damage message: ie A huge log comes flying through the trees and impales you predator style.
	disarm: yes/no 
	re-arm: yes/no        //In game traps only
	re_arm time: TICKS    //In game traps only
	unavoidable: yes/no   //In game traps only
	timer: TICKS          //In game traps only, also used with detect traps
	decay timer: Ticks until it rots  // Make stuff cleaner, if a player quits his traps decay also
	disarm difficulty: 1-10 
	trap dex: 3-25 (compares to morts walking into the trap) 
	damage type: acid? pierce? etc 
	damage dice:
	damage modifier: 
	area: yes/no 
	affects: poison, blind , entangle, etc
	summon: MOB VNUM (for in-game traps) 
	spell: SPELL NAME (for in-game traps)
*/
TRAP_DATA *next;
bool valid;	
char *owner;
char *dammessage;
bool disarm;
bool rearm;
bool rearm_ticks;
bool avoidable;
bool activate_ticks;
bool decay_ticks;
bool area_aff;
int diff;
int trapdex;
int damtype;
int damdice;
int dammod;
// int summon_mob_vnum;
int sn;
};

/* Exit data.
 */
struct exit_data
{
  union
  {
    ROOM_INDEX_DATA *to_room;
    int vnum;
  }
  u1;
  EXIT_DATA *next;		/* OLC */
  int rs_flags;			/* OLC */
  int orig_door;		/* OLC */
  short exit_info;
  short key;
  char *keyword;
  char *description;
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile 
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct reset_data
{
  RESET_DATA *next;
  char command;
  int arg1;
  int arg2;
  int arg3;
  int arg4;
};



/*
 * Area definition.
 */
struct area_data
{
  AREA_DATA *next;
  RESET_DATA *reset_first;
  RESET_DATA *reset_last;
  OBJ_TRIG_DATA *ot_first;
  OBJ_TRIG_DATA *ot_last;
  char *file_name;
  char *name;
  char *credits;
  short points;
  int clan;
  char *help;
  short age;
  short nplayer;
  short low_range;
  short high_range;
  int min_vnum;
  int max_vnum;
  bool empty;
  char *filename;
  char *creator;
  char *helper;
  bool noclan;
  bool construct;
  int llev;
  int ulev;
  int lvnum;
  int uvnum;
  int vnum;
  int area_flags;
  int recall;
  int version;
};


struct auction_data
{
   /**/ OBJ_DATA * item;	// a pointer to the item
  CHAR_DATA *seller;		// a pointer to the seller - which may NOT quit
  CHAR_DATA *buyer;		// a pointer to the buyer - which may NOT quit
  int bid;			// last bet - or 0 if noone has bet anything
  int minbid;
  short going;			// 1, 2, sold 
  short pulse;			// how many pulses until another call-out
};

extern SCENT_DATA *scents;
extern MEMORY_DATA *memories;
extern AUCTION_DATA *auction;

void reboot_rom args ((void));
void shutdown_rom args ((void));

/*
 * Room type.
 */
struct scent_data
{
  SCENT_DATA *next;
  SCENT_DATA *next_in_room;
  CHAR_DATA *player;
  ROOM_INDEX_DATA *in_room;
  short scent_level;
};

struct room_index_data
{
  ROOM_INDEX_DATA *next;
  RESET_DATA *reset_first;
  RESET_DATA *reset_last;
  SCENT_DATA *scents;
  CHAR_DATA *people;
  OBJ_DATA *contents;
  EXTRA_DESCR_DATA *extra_descr;
  AREA_DATA *area;
  EXIT_DATA *exit[6];
  EPL_PROG *eplprog;
  char *epl_filename;
  char *name;
  char *description;
  char *owner;
  char *enter_msg;
  char *exit_msg;
  int vnum;
  short max_in_room;
  char *max_message;
  long room_flags;
  // Adeon 6/30/03 -- Had to expand room flags.
  long room_flags2;
  short light;
  short sector_type;
  short heal_rate;
  short mana_rate;
  short clan;
  short max_level;
  long race_flags;
  long Class_flags;
  int ctransport_vnum;
  short tp_level;
  long tp_exp;
  char *tp_msg;
  // Adeon 6/30/03 -- Support for sinking rooms
  char *sink_msg;
  char *sink_msg_others;
  char *sink_warning;
  int sink_timer;
  int sink_dest;		//destination post sinkage ;)
};

struct trap_info_type
{
  int type;
  long flags;
  char *smess;
  char *vmess;
  int  num_dice;
  int size_dice;
  int mod_dice;
  char * damtype;
};


/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_AUTOASSIST              -2
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000



/*
 *  Target types.
 */
#define TAR_IGNORE		    0
#define TAR_CHAR_OFFENSIVE	    1
#define TAR_CHAR_DEFENSIVE	    2
#define TAR_CHAR_SELF		    3
#define TAR_OBJ_INV		    4
#define TAR_OBJ_CHAR_DEF	    5
#define TAR_OBJ_CHAR_OFF	    6
#define TAR_AREA_OFF                7
//Iblis 1/5/04 - Needed for Necromancers
#define TAR_OBJ_PENTAGRAM	    8
#define TAR_CHAR_HEXAGRAM	    9
#define TAR_ROOM_SEPTAGRAM	    10
#define TAR_CHAR_WORLD		    11
#define TAR_STRING		    12

#define TARGET_CHAR		    0
#define TARGET_OBJ		    1
#define TARGET_ROOM		    2
#define TARGET_NONE		    3



/*
 * Skills include spells as a particular case.
 */
struct cmdskill_type
{
  int type;
  short skill_level[MAX_CLASS];
};


struct skill_type
{
  int type;
  char *name;			/* Name of skill                */
  short skill_level[MAX_CLASS];	/* Level needed by Class        */
  short rating[MAX_CLASS];	/* How hard it is to learn      */
  SPELL_FUN *spell_fun;		/* Spell pointer (for spells)   */
  short target;			/* Legal targets                */
  short minimum_position;	/* Position for caster / user   */
  short *pgsn;			/* Pointer to associated gsn    */
  short slot;			/* Slot for #OBJECT loading     */
  short min_mana;		/* Minimum mana used            */
  short beats;			/* Waiting time after use       */
  char *noun_damage;		/* Damage message               */
  char *msg_off;		/* Wear off message             */
  char *msg_obj;		/* Wear off message for obects  */
};

struct group_type
{
  char *name;
  short rating[MAX_CLASS];
  char *spells[MAX_IN_GROUP];
};

extern char *material_list[];

extern AREA_DATA *area_last;
extern int top_vnum_mob;
extern int top_vnum_obj;
extern int top_shop;

/*
 * These are skill_lookup return values for common skills and spells.
 */
extern short gsn_throwing, gsn_elusiveness, gsn_retreat, gsn_swim, gsn_mask;
extern short gsn_summon_mount, gsn_leadership, gsn_assassinate;
extern short gsn_fisticuffery, gsn_doorbash, gsn_foraging, gsn_camping;
extern short gsn_heavenly_cloak, gsn_blindfighting, gsn_layhands;
extern short gsn_wall_thorns, gsn_track, gsn_stalk, gsn_sweep;
extern short gsn_circle, gsn_charge, gsn_compose, gsn_butcher, gsn_entangle;
extern short gsn_endurance, gsn_backstab, gsn_sap, gsn_fishing, gsn_riding;
extern short gsn_tame, gsn_kai, gsn_uppercut, gsn_dual_wield, gsn_stun;
extern short gsn_dodge, gsn_envenom, gsn_hide, gsn_peek, gsn_pick_lock;
extern short gsn_sneak, gsn_steal, gsn_brew, gsn_scribe, gsn_palm;
extern short gsn_enhanced_damage, gsn_critical_strike, gsn_kick, gsn_parry,
  gsn_rescue;
extern short gsn_second_attack, gsn_third_attack, gsn_sanctuary;
extern short gsn_blindness, gsn_conceal, gsn_charm_person, gsn_hone;
extern short gsn_curse, gsn_invis, gsn_instruments, gsn_mass_invis;
extern short gsn_mirror, gsn_plague, gsn_poison, gsn_sleep, gsn_fly;
extern short gsn_axe, gsn_dagger, gsn_flail, gsn_mace, gsn_polearm;
extern short gsn_shield_block, gsn_staff, gsn_sword, gsn_eviscerate;
extern short gsn_bash, gsn_berserk, gsn_dirt, gsn_hand_to_hand;
extern short gsn_trip, gsn_redirect, gsn_trample, gsn_disarm, gsn_fireshield;
extern short gsn_fast_healing, gsn_haggle, gsn_influence, gsn_lore;
extern short gsn_meditation, gsn_scrolls, gsn_staves, gsn_wands, gsn_bark_skin;
extern short gsn_exorcism, gsn_camouflage, gsn_the_piper, gsn_tripwire,
  gsn_invocation;
extern short gsn_unholy_burst, gsn_hellscape, gsn_repulse, gsn_tempt, gsn_shadowslash;
extern short gsn_split_vision, gsn_sense_life, gsn_dirty_tactics,
  gsn_block, gsn_counter;
extern short gsn_knock, gsn_feign, gsn_prevent_escape, gsn_dive, gsn_toss,
  gsn_the_arts, gsn_strike;
extern short gsn_whirlwind, gsn_fists_of_fury, gsn_bandage, gsn_awareness,
  gsn_hear_the_wind, gsn_invention;
extern short gsn_archery, gsn_falconry, gsn_summon_beast, gsn_pounce,
  gsn_disorientation;
extern short gsn_woodworking, gsn_fletchery, gsn_aquatitus, gsn_jalknation, gsn_jurgnation;
extern short gsn_fear, gsn_illusion, gsn_suck, gsn_likobe, gsn_aggression, gsn_clasp;
extern short gsn_leeching, gsn_death_ward, gsn_draw, gsn_embalm, gsn_sense_death;
extern short gsn_feign_death, gsn_corrupt, gsn_hellbreath, gsn_feast, gsn_feed_death;
extern short gsn_corpse_carving, gsn_unsummon, gsn_astral_body, gsn_soul_link, gsn_death_shroud;
extern short gsn_defile, gsn_despoil, gsn_contaminate, gsn_flare,gsn_vomit, gsn_atrophy;
extern short gsn_pox,gsn_immunity,gsn_skeletal_mutation,gsn_intoxicate,gsn_brain_blisters,gsn_aura_rot;
extern short gsn_greymantle,gsn_festering_boils,gsn_lightningbreath,gsn_swoop, gsn_spider_vine;
extern short gsn_detect_traps,gsn_avoid_traps,gsn_disarm_traps,gsn_create_traps,gsn_trapdamage;
extern short gsn_bubble, gsn_aquatic_sense, gsn_spear, gsn_locate, gsn_call_storm, gsn_transform;
extern short gsn_dehydration, gsn_find_land, gsn_forestry, gsn_lumberjacking, gsn_milling;
extern short gsn_flicker,gsn_warp_touch,gsn_mad_rush,gsn_coma,gsn_vanish,gsn_persuade;
extern short gsn_mad_dance, gsn_fire_dance, gsn_locust_wings, gsn_torture, gsn_gravity;
extern short gsn_escape, gsn_meteor_swarm, gsn_scramble, gsn_death_spasms, gsn_larva;
extern short gsn_flick, gsn_shuffle, gsn_dice, gsn_lucky_throw, gsn_evil_twin;
//extern short gsn_smother, gsn_attack;

// Druid skills.
extern short gsn_clarity;
extern short gsn_nature_sense;
extern short gsn_shillelagh;
extern short gsn_animal_empathy;
extern short gsn_animal_friendship;
extern short gsn_satiate;
extern short gsn_wild_invigoration;
extern short gsn_primeval_guidance;
extern short gsn_shapechange;
extern short gsn_insect_plague;
//extern short gsn_elemental_sphere;


//extern unsigned long TopGameStat[MAX_GAMESTAT];	// keeps track of the highest stats.
extern char *TGS_Names[MAX_GAMESTAT][MAX_PEOPLE_STAT];
extern unsigned long TGS_Stats[MAX_GAMESTAT][MAX_PEOPLE_STAT];

/*
 * Utility macros.
 */
#define IS_VALID(data)		((data) != NULL && (data)->valid)
#define VALIDATE(data)		((data)->valid = TRUE)
#define INVALIDATE(data)	((data)->valid = FALSE)
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define STR(dat, field)         (( (dat)->field != NULL                    \
                                             ? (dat)->field                \
                                             : (dat)->pIndexData->field ))
#define NAME( ch )              (( !IS_NPC(ch) ? (ch)->name    \
				               : STR(ch, short_descr) ))
#define HAS_SCRIPT(ch)          ( IS_NPC(ch) && ch->triggers != NULL )

extern VARIABLE_DATA *variable_free;
extern TRIGGER_DATA *trigger_free;
extern SCRIPT_DATA *script_free;
/*
 * Character macros.
 */
#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC))
#define IS_IMMORTAL(ch)		(get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust(ch) >= LEVEL_HERO)
#define IS_TRUSTED(ch,level)	(get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))

#define GET_AGE(ch)		((int) (17 + ((ch)->played \
				    + current_time - (ch)->logon )/72000))

#define IS_GOOD(ch)     (ch->alignment >= ALIGN_GOOD_MIN)
#define IS_EVIL(ch)     (ch->alignment <= ALIGN_EVIL_MAX)
#define IS_NEUTRAL(ch)  (!IS_GOOD(ch) && !IS_EVIL(ch))

// Akamai 6/4/98 -- Bug: #9 Alignment grouping fix
// These macros just simplify alignment code in places
#define ALIGN_IS_ANGELIC(ch)   (ch->alignment >= ALIGN_ANGELIC_MIN)
#define ALIGN_IS_SAINTLY(ch)  ((ch->alignment <= ALIGN_SAINTLY_MAX) && \
                               (ch->alignment >= ALIGN_SAINTLY_MIN))
#define ALIGN_IS_GOOD(ch)     ((ch->alignment <= ALIGN_GOOD_MAX) && \
                               (ch->alignment >= ALIGN_GOOD_MIN))
#define ALIGN_IS_KIND(ch)     ((ch->alignment <= ALIGN_KIND_MAX) && \
                               (ch->alignment >= ALIGN_KIND_MIN))
#define ALIGN_IS_NEUTRAL(ch)  ((ch->alignment <= ALIGN_NEUTRAL_MAX) && \
                               (ch->alignment >= ALIGN_NEUTRAL_MIN))
#define ALIGN_IS_MEAN(ch)     ((ch->alignment <= ALIGN_MEAN_MAX) && \
                               (ch->alignment >= ALIGN_MEAN_MIN))
#define ALIGN_IS_EVIL(ch)     ((ch->alignment <= ALIGN_EVIL_MAX) && \
                               (ch->alignment >= ALIGN_EVIL_MIN))
#define ALIGN_IS_DEMONIC(ch)  ((ch->alignment <= ALIGN_DEMONIC_MAX) && \
                               (ch->alignment >= ALIGN_DEMONIC_MIN))
#define ALIGN_IS_SATANIC(ch)   (ch->alignment <= ALIGN_SATANIC_MAX)

#define ALIGN_IN_SPHERE(c,v) (abs( c->alignment - v->alignment ) \
                               < ALIGN_GROUP_SPHERE )

#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)
#define GET_AC(ch,type)		((ch)->armor[type]			    \
		        + ( IS_AWAKE(ch) && !IS_NPC(ch)			    \
			? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0 ))
#define GET_HITROLL(ch)	\
		((ch)->hitroll+str_app[get_curr_stat(ch,STAT_STR)].tohit)
#define GET_DAMROLL(ch) \
		((ch)->damroll+str_app[get_curr_stat(ch,STAT_STR)].todam)

#define IS_OUTSIDE(ch)		(!IS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS))

#define WAIT_STATE(ch, npulse)	((ch)->wait = UMAX((ch)->wait, (npulse)))
#define DAZE_STATE(ch, npulse)  ((ch)->daze = UMAX((ch)->daze, (npulse)))
#define get_carry_weight(ch)	((ch)->carry_weight + (ch)->silver/10 +  \
						      (ch)->gold * 2 / 5)



/*
 * Command logging macros. A Nasty but cheap inline hack
 */
#define log_player_command(fun, ch, command) \
{ \
   char __lcbuf__[MAX_INPUT_LENGTH]; \
   if(command) { \
      if( ch && ch->name ) { \
         sprintf(__lcbuf__,"%s: %s > \"%s\"",fun, ch->name, command); \
      } else { \
         sprintf(__lcbuf__,"%s: *unknown* > \"%s\"",fun, command); \
      } \
   } else { \
      if( ch && ch->name ) { \
         sprintf(__lcbuf__,"%s: %s > \"<null>\"",fun, ch->name); \
      } else { \
         sprintf(__lcbuf__,"%s: *unknown* > \"<null>\"",fun); \
      } \
   }\
   log_string(__lcbuf__); \
}
/* don't need a backslash here because the macro is done */


/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
//Iblis 4/21/04 - I'm leaving this in atm so I don't have to redo all the code that uses IS_OBJ_STAT
//NOTe, IS_OBJ_STAT ******WILL NOT WORK******* for extra_flags other than those in extra_flags[0]
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags[0], (stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))
#define WEIGHT_MULT(obj)	((obj)->item_type == ITEM_CONTAINER ? \
	(obj)->value[4] : 100)
#define WEIGHT_MULTP(obj)	 ((obj)->item_type == ITEM_PACK ? (obj)->value[4] : 100)
	



/*
 * Description macros.
 */
#define PERS(ch, looker)	( can_see( looker, (ch) ) ?		\
				( (IS_NPC(ch) || IS_SET(ch->act2,ACT_TRANSFORMER)) ? (ch)->short_descr	\
				: (ch)->name ) : "someone" )

/*
 * Structure for a social in the socials table.
 */
struct social_type
{
  char name[20];
  char *char_no_arg;
  char *others_no_arg;
  char *char_found;
  char *others_found;
  char *vict_found;
  char *char_not_found;
  char *char_auto;
  char *others_auto;
};

//Iblis 6-28-03 Record to store a list of aggressors
//struct aggressor_list
 // {
  //  CHAR_DATA* ch;
   // short ticks_left;
    //AGGRESSOR_LIST *next;
  //};


/*
 * Global constants.
 */
extern const struct str_app_type str_app[26];
extern const struct int_app_type int_app[26];
extern const struct wis_app_type wis_app[26];
extern const struct wis_app_type prim_app[26];
extern const struct dex_app_type dex_app[26];
extern const struct con_app_type con_app[26];

extern const struct Class_type Class_table[MAX_CLASS];
extern const struct weapon_type weapon_table[];
extern const struct item_type item_table[];
extern const struct wiznet_type wiznet_table[];
extern const struct attack_type attack_table[];
extern const struct race_type race_table[];
extern const struct pc_race_type pc_race_table[];
extern const sh_int rev_dir[];	/* sh_int - ROM OLC */
extern const sh_int movement_loss[SECT_MAX];
extern const struct spec_type spec_table[];
extern const struct liq_type liq_table[];
extern const struct cmdskill_type command_table[];
extern const struct skill_type skill_table[MAX_SKILL];
extern const struct group_type group_table[MAX_GROUP];
extern struct social_type social_table[MAX_SOCIALS];
extern const struct tree_type tree_table[];
extern const struct item_type card_table[];
extern const struct wooden_obj_type wooden_obj_table[];
extern const sh_int hp_gains[MAX_CLASS][26][2];
extern const sh_int mana_gains[MAX_CLASS][26][2];


/*
 * Global variables.
 */
extern HELP_DATA *help_first;
extern SHOP_DATA *shop_first;

extern CHAR_DATA *char_list;
extern FAKE_DATA *fake_list;
extern DESCRIPTOR_DATA *descriptor_list;
extern OBJ_DATA *object_list;
extern BOUNTY_DATA *bounty_list;
extern INVENTION_DATA *invention_list;
extern RUNE_CARD_DATA *rune_card_list;
extern char bug_buf[];
extern time_t current_time;
extern bool fLogAll;
extern FILE *fpReserve;
extern KILL_DATA kill_table[];
extern char log_buf[];
extern TIME_INFO_DATA time_info;
extern WEATHER_DATA weather_info;
extern char last_command[MAX_STRING_LENGTH];
extern int reboot_counter;
extern int shutdown_counter;
extern OBJ_DATA *corpse_list;

#define LAST_COMMAND    "../log/last_command.txt"	/* For the signal handler */
#define PLAYER_DIR      "../player/"	/* Player files */
#define GOD_DIR         "../gods/"	/* list of gods */
#define BIN_DIR         "../bin/"	/* binaries and scripts */
#define OTM_FILE        "../area/obj_trig.are"  /* obj trigger save file */
#define TEMP_FILE       "../player/romtmp"
#define NULL_FILE       "/dev/null"	/* To reserve one stream */
#define AREA_LIST       "area.lst"	/* List of areas */
#define BUG_FILE        "bugs.txt"	/* For 'bug' and bug() */
#define TYPO_FILE       "typos.txt"	/* For 'typo' */
#define SHUTDOWN_FILE   "shutdown.txt"	/* For 'shutdown' */
#define BAN_FILE        "ban.txt"
#define BOUNTY_FILE     "bounty.txt"
#define MUSIC_FILE      "music.txt"
#define EXODUS_PID      "exodus.pid"	/* the exodus process id, text */
#define ACTIVITY_LOG    "activity.txt"	/* For logging some activities */
#define SECURITY_FILE   "security.txt"
#define INVENTION_FILE  "invention.txt"
#define TOP_FISHER_FILE "top_fishers.txt"
#define CORPSE_FILE     "corpse_file.txt"
#define RUNE_CARD_FILE  "rune_cards.txt"
#define CARDCOUNT_FILE  "cardcount.txt"

/* auction.c */
int parsebid (const int, const char *);

/* act_comm.c */
bool check_parse_name (char *);
bool is_same_group (CHAR_DATA *, CHAR_DATA *);
CHAR_DATA *get_group_leader (CHAR_DATA *);
bool is_race_in_group (CHAR_DATA *, int);
bool in_group_sphere (CHAR_DATA *, CHAR_DATA *);
int number_in_group (CHAR_DATA *);
int get_group_max_align (CHAR_DATA *);
int get_group_min_align (CHAR_DATA *);
int disband_follower (CHAR_DATA *);
void group_gr (CHAR_DATA *, CHAR_DATA *);
void talk_auction (char *);
void adjust_elevator_weight (OBJ_DATA *);
void check_sex (CHAR_DATA *);
void add_follower (CHAR_DATA *, CHAR_DATA *);
void stop_follower (CHAR_DATA *);
void nuke_pets (CHAR_DATA *);
void die_follower (CHAR_DATA *);
void defect_follower (CHAR_DATA *);
void do_hard_quit (CHAR_DATA *, char *, bool avatar);

/* act_enter.c */
void enter_portal (CHAR_DATA *, char *, bool);
ROOM_INDEX_DATA *get_random_room (CHAR_DATA *);

/* arena.c */
ROOM_INDEX_DATA *get_random_arena args ((CHAR_DATA * ch));
void duel_record (char *, char *), show_duel_record (CHAR_DATA *);
void duel_cancel (CHAR_DATA *), arena_report (char *);
int is_dueling (CHAR_DATA *);
void duel_ends args ((CHAR_DATA * loser));
void board_boat args ((CHAR_DATA * ch, OBJ_DATA * ctransport));
void do_enter args ((CHAR_DATA * ch, char *argument));


/* act_build.c */
void build_name args ((CHAR_DATA * ch, char *argument));
void flags_menu args ((CHAR_DATA * ch, char *argument));
void build_menu args ((CHAR_DATA * ch, char *argument));
void build_keywords args ((CHAR_DATA * ch, char *argument));
void build_keywords_add args ((CHAR_DATA * ch, char *argument));
void build_doors args ((CHAR_DATA * ch, char *argument));
void build_doors_flags args ((CHAR_DATA * ch, char *argument));
void build_resets args ((CHAR_DATA * ch, char *argument));
void build_door_name args ((CHAR_DATA * ch, char *argument));
bool can_build (CHAR_DATA *, ROOM_INDEX_DATA * location);

/* act_info.c */
char *PERS args(( CHAR_DATA *victim, CHAR_DATA *ch));
char *level_name args ((int level));
void set_title args ((CHAR_DATA * ch, char *title));
short get_position args((CHAR_DATA *ch));

/* act_move.c */
bool is_campsite (ROOM_INDEX_DATA *);
bool can_use_room (CHAR_DATA *, ROOM_INDEX_DATA *);
bool has_key args ((CHAR_DATA * ch, int key));
void push_char args ((CHAR_DATA * ch, char *argument, short always));
void new_master args ((CHAR_DATA * ch, CHAR_DATA * mount));
void move_char args ((CHAR_DATA * ch, int door, bool follow));
void drag_char
args ((CHAR_DATA * ch, CHAR_DATA * victim, int door, bool follow));

/* act_obj.c */
bool store_closed (ROOM_INDEX_DATA *);
void close_store (ROOM_INDEX_DATA *);
void open_store (ROOM_INDEX_DATA *);
void obj_to_keeper (OBJ_DATA *, CHAR_DATA *);
void wear_obj (CHAR_DATA *, OBJ_DATA *, bool, int);
int get_cost (CHAR_DATA *, OBJ_DATA *, bool);
OBJ_DATA *get_obj_keeper (CHAR_DATA *, CHAR_DATA *, char *);
void bad_affect args ((CHAR_DATA * ch, OBJ_DATA * obj));
void adjust_portal_weight args ((OBJ_DATA * obj));
CHAR_DATA *find_keeper args ((CHAR_DATA * ch));
bool can_loot args ((CHAR_DATA * ch, OBJ_DATA * obj));
void get_obj args ((CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container));
bool remove_obj args ((CHAR_DATA * ch, int iWear, bool fReplace));

/* act_wiz.c */
void load_shelf (OBJ_DATA *, int);
void save_shelf (OBJ_DATA *, int);
void wiznet args ((char *string, CHAR_DATA * ch, OBJ_DATA * obj,
		   long flag, long flag_skip, int min_level));

/* alias.c */
void substitute_alias args ((DESCRIPTOR_DATA * d, char *input));

/* ban.c */
bool check_ban args ((char *site, int type));

/* comm.c */
void show_string args ((struct descriptor_data * d, char *input));
void close_socket args ((DESCRIPTOR_DATA * dclose));
void write_to_buffer args ((DESCRIPTOR_DATA * d, const char *txt,
			    int length));
void send_to_char args ((const char *txt, CHAR_DATA * ch));
void page_to_char args ((char *txt, CHAR_DATA * ch));
void act args ((const char *format, CHAR_DATA * ch,
		const void *arg1, const void *arg2, int type));
void act_new args ((const char *format, CHAR_DATA * ch,
		    const void *arg1, const void *arg2, int type,
		    int min_pos));
int good_password args ((char *passwd, char *username));
void send_to_desc args ((DESCRIPTOR_DATA * desc, char *txt));

/* db.c */
bool rare_enough args ((OBJ_INDEX_DATA * obj));
void reset_room args ((ROOM_INDEX_DATA * pRoom));
void assign_area_vnum args ((int vnum));
void reset_area args ((AREA_DATA * pArea));
char *fcapitalize args ((const char *str));

char *print_flags args ((int flag));
void boot_db args ((void));
void area_update args ((void));
CHAR_DATA *create_mobile args ((MOB_INDEX_DATA * pMobIndex));
void clone_mobile args ((CHAR_DATA * parent, CHAR_DATA * clone));
OBJ_DATA *create_object args ((OBJ_INDEX_DATA * pObjIndex, int level));
void clone_object args ((OBJ_DATA * parent, OBJ_DATA * clone));
void clear_char args ((CHAR_DATA * ch));
char *get_extra_descr args ((const char *name, EXTRA_DESCR_DATA * ed));
char *get_extra_name args ((const char *name, EXTRA_DESCR_DATA * ed));
MOB_INDEX_DATA *get_mob_index args ((int vnum));
OBJ_INDEX_DATA *get_obj_index args ((int vnum));
ROOM_INDEX_DATA *get_room_index args ((int vnum));
char fread_letter args ((FILE * fp));
int fread_number args ((FILE * fp));
long fread_flag args ((FILE * fp));
char *fread_string args ((FILE * fp));
char *fread_string_eol args ((FILE * fp));
void fread_to_eol args ((FILE * fp));
char *fread_word args ((FILE * fp));
long flag_convert args ((char letter));
void *alloc_mem args ((int sMem));
void *alloc_perm args ((int sMem));
void free_mem args ((void *pMem, int sMem));
char *str_dup args ((const char *str));
void free_string args ((char *pstr));
int number_fuzzy args ((int number));
int number_range args ((int from, int to));
int number_percent args ((void));
int number_door args ((void));
int number_bits args ((int width));
long number_mm args ((void));
int dice args ((int number, int size));
int interpolate args ((int level, int value_00, int value_32));
void smash_tilde args ((char *str));
bool str_cmp args ((const char *astr, const char *bstr));
bool str_prefix args ((const char *astr, const char *bstr));
bool str_infix args ((const char *astr, const char *bstr));
bool str_suffix args ((const char *astr, const char *bstr));
char *capitalize args ((const char *str));
void append_file args ((CHAR_DATA * ch, char *file, char *str));
void bug args ((const char *str, int param));
void log_string args ((const char *str));
void log_activity args ((const char *note, CHAR_DATA * ach, CHAR_DATA * bch));
void tail_chain args ((void));

/* effect.c */
void acid_effect args ((void *vo, int level, int dam, int target));
void cold_effect args ((void *vo, int level, int dam, int target));
void fire_effect args ((void *vo, int level, int dam, int target));
void poison_effect args ((void *vo, int level, int dam, int target));
void shock_effect args ((void *vo, int level, int dam, int target));

/* maps.c */
OBJ_DATA *create_map args ((CHAR_DATA * ch, int start_vnum, int end_vnum));

/* fight.c */
void raw_kill args ((CHAR_DATA * victim, CHAR_DATA * killer));
void extract_memory args ((MEMORY_DATA * memd));
void set_fighting_auto args ((CHAR_DATA * ch, CHAR_DATA * victim));
void add_hunting args ((CHAR_DATA * hunter, CHAR_DATA * hunted));
void one_dual_hit args ((CHAR_DATA * ch, CHAR_DATA * victim, int dt));
void remove_wanted args ((CHAR_DATA * ch));
bool check_shopkeeper_attack args ((CHAR_DATA * ch, CHAR_DATA * victim));
void arrest args ((CHAR_DATA * ch, CHAR_DATA * victim));
void unjail args ((CHAR_DATA * ch));
void set_wanted args ((CHAR_DATA * ch));
void set_fighting args ((CHAR_DATA * ch, CHAR_DATA * victim));
void port_kill args ((CHAR_DATA * victim));
bool is_safe args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool is_safe_spell args ((CHAR_DATA * ch, CHAR_DATA * victim, bool area));
void violence_update args ((void));
void multi_hit args ((CHAR_DATA * ch, CHAR_DATA * victim, int dt));
bool damage args ((CHAR_DATA * ch, CHAR_DATA * victim, int dam,
		   int dt, int Class, bool show));
bool damage_old args ((CHAR_DATA * ch, CHAR_DATA * victim, int dam,
		       int dt, int Class, bool show));
void update_pos args ((CHAR_DATA * victim));
void stop_fighting args ((CHAR_DATA * ch, bool fBoth));
void check_killer args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool insta_kill args ((CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type, bool show, bool magic));


/* handler.c */
char *one_argument (char *, char *);
char *one_argument_nl (char *, char *);
bool can_see_hack (CHAR_DATA *, CHAR_DATA *);
int orace_lookup (const char *);
bool is_name_no_abbrev (char *, char *);
int hand_lookup (const char *);
int get_char_weight (CHAR_DATA *);
void extract_scent (SCENT_DATA *);
bool is_hunting (CHAR_DATA *, CHAR_DATA * hunted);
AFFECT_DATA *affect_find (AFFECT_DATA *, int);
void affect_check args ((CHAR_DATA * ch, int where, int vector));
int count_users args ((OBJ_DATA * obj));
void deduct_cost args ((CHAR_DATA * ch, int cost));
void affect_enchant args ((OBJ_DATA * obj));
int check_immune args ((CHAR_DATA * ch, int dam_type));
char *material_name args ((sh_int num));
int liq_lookup args ((const char *name));
int material_lookup args ((const char *name));
int weapon_lookup args ((const char *name));
int weapon_type args ((const char *name));
char *weapon_name args ((int weapon_Type));
int item_lookup args ((const char *name));
char *item_name args ((int item_type));
int attack_lookup args ((const char *name));
int race_lookup args ((const char *name));
long wiznet_lookup args ((const char *name));
int Class_lookup args ((const char *name));
bool is_old_mob args ((CHAR_DATA * ch));
int get_skill args ((CHAR_DATA * ch, int sn));
int get_weapon_sn args ((CHAR_DATA * ch, int wield_type));
int get_weapon_skill args ((CHAR_DATA * ch, int sn));
int get_age args ((CHAR_DATA * ch));
void reset_char args ((CHAR_DATA * ch));
int get_trust args ((CHAR_DATA * ch));
int get_curr_stat args ((CHAR_DATA * ch, int stat));
int get_max_train args ((CHAR_DATA * ch, int stat));
int can_carry_n args ((CHAR_DATA * ch));
int can_carry_w args ((CHAR_DATA * ch));
bool is_name args ((char *str, char *namelist));
void affect_to_char args ((CHAR_DATA * ch, AFFECT_DATA * paf));
void affect_to_obj args ((OBJ_DATA * obj, AFFECT_DATA * paf));
void affect_remove args ((CHAR_DATA * ch, AFFECT_DATA * paf));
void affect_remove_obj args ((OBJ_DATA * obj, AFFECT_DATA * paf));
void affect_strip args ((CHAR_DATA * ch, int sn));
bool is_affected args ((CHAR_DATA * ch, int sn));
void affect_join args ((CHAR_DATA * ch, AFFECT_DATA * paf));
void char_from_room args ((CHAR_DATA * ch));
void char_to_room args ((CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex));
void obj_to_char args ((OBJ_DATA * obj, CHAR_DATA * ch));
void obj_from_char args ((OBJ_DATA * obj));
int apply_ac args ((OBJ_DATA * obj, int iWear, int type));
int apply_ac_new args ((OBJ_DATA * obj, int type));
OBJ_DATA *get_eq_char args ((CHAR_DATA * ch, int iWear));
OBJ_DATA *get_eq_char_new args ((CHAR_DATA * ch, long iWear));
bool char_can_remove_to_wear args((CHAR_DATA *ch, OBJ_DATA *obj, bool show_messages));
bool char_can_remove_to_wear_hand args((CHAR_DATA *ch, OBJ_DATA *obj, bool show_messages, int hand));
void equip_char args ((CHAR_DATA * ch, OBJ_DATA * obj, long iWear));
bool equip_char_new args ((CHAR_DATA * ch, OBJ_DATA * obj));
bool equip_char_new_hand args ((CHAR_DATA * ch, OBJ_DATA * obj, int hand));
void unequip_char args ((CHAR_DATA * ch, OBJ_DATA * obj));
int count_obj_list args ((OBJ_INDEX_DATA * obj, OBJ_DATA * list));
void obj_from_room args ((OBJ_DATA * obj));
void obj_to_room args ((OBJ_DATA * obj, ROOM_INDEX_DATA * pRoomIndex));
void obj_to_obj args ((OBJ_DATA * obj, OBJ_DATA * obj_to));
void obj_from_obj args ((OBJ_DATA * obj));
void extract_obj args ((OBJ_DATA * obj));
void extract_char args ((CHAR_DATA * ch, bool fPull));
CHAR_DATA *get_char_room args ((CHAR_DATA * ch, char *argument));
CHAR_DATA *get_exact_pc_room args ((CHAR_DATA * ch, char *argument));
CHAR_DATA *get_char_room_nomask args ((CHAR_DATA * ch, char *argument));
CHAR_DATA *get_char_world args ((CHAR_DATA * ch, char *argument));
CHAR_DATA *get_char_world2 args((CHAR_DATA * ch, char *argument));
CHAR_DATA *get_exact_pc_world args ((CHAR_DATA * ch, char *argument));
OBJ_DATA *get_obj_type args ((OBJ_INDEX_DATA * pObjIndexData));
OBJ_DATA *get_obj_list args ((CHAR_DATA * ch, char *argument,
			      OBJ_DATA * list));
OBJ_DATA *get_obj_carry args ((CHAR_DATA * ch, char *argument));
OBJ_DATA *get_obj_wear args ((CHAR_DATA * ch, char *argument));
OBJ_DATA *get_obj_here args ((CHAR_DATA * ch, char *argument));
OBJ_DATA *get_obj_world args ((CHAR_DATA * ch, char *argument));
OBJ_DATA *create_money args ((int gold, int silver));
int get_obj_number args ((OBJ_DATA * obj));
int get_obj_weight args ((OBJ_DATA * obj));
int get_true_weight args ((OBJ_DATA * obj));
bool room_is_dark args ((ROOM_INDEX_DATA * pRoomIndex));
bool is_room_owner args ((CHAR_DATA * ch, ROOM_INDEX_DATA * room));
bool room_is_private args ((ROOM_INDEX_DATA * pRoomIndex));
bool can_see args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool can_see_obj args ((CHAR_DATA * ch, OBJ_DATA * obj));
bool can_see_room args ((CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex));
bool can_drop_obj args ((CHAR_DATA * ch, OBJ_DATA * obj));
char *item_type_name args ((OBJ_DATA * obj));
char *affect_loc_name args ((int location));
const char *affect_bit_name args ((int vector));
const char *extra_bit_name_old args ((int extra_flags));
const char *extra_bit_name args ((long extra_flags[]));
const char *extra_bit_name_new args((long extra_flags[], bool show_hidden));
const char *wear_bit_name args ((long wear_flags));
const char *room_bit_name args ((int room_flags));
// Adeon 6/30/03 -- Had to expand upon room flags
const char *room2_bit_name args ((int room_flags2));
const char *act_bit_name args ((int act_flags));
const char *act2_bit_name args ((int act_flags2));
const char *off_bit_name args ((int off_flags));
const char *imm_bit_name args ((int imm_flags));
const char *form_bit_name args ((int form_flags));
const char *part_bit_name args ((int part_flags));
const char *weapon_bit_name args ((int weapon_flags));
const char *comm_bit_name args ((int comm_flags));
const char *comm2_bit_name args ((int comm2_flags));
const char *cont_bit_name args ((int cont_flags));
bool IS_CLASS args ((CHAR_DATA * ch, short Class));
short level_for_skill args ((CHAR_DATA * ch, short sn));
short level_for_command args ((CHAR_DATA * ch, short sn));
bool mana_using_Class args ((CHAR_DATA * ch));
bool Class_flagged_obj args ((OBJ_DATA * obj, CHAR_DATA * ch));
short rating_for_skill args ((CHAR_DATA * ch, short sn));
short current_Class args ((CHAR_DATA * ch));
short Class_level args ((CHAR_DATA * ch, short Class));
void change_alignment args ((CHAR_DATA *ch,int amount));
bool is_Class_tiers args ((CHAR_DATA * ch, short Class, short tiers));
OBJ_DATA *get_obj_vnum_room args ((ROOM_INDEX_DATA* rid, int vnum));
bool obj_to_obj_pos args((OBJ_DATA * obj, OBJ_DATA * obj_to, int pos));
CHAR_DATA *get_random_player_room args((CHAR_DATA *ch));
CHAR_DATA *get_random_mob_room args((CHAR_DATA *ch));
int get_hitroll args((CHAR_DATA *ch));	
int get_damroll args((CHAR_DATA *ch));

/* interp.c */
void interpret args ((CHAR_DATA * ch, char *argument));
bool is_number args ((char *arg));
int number_argument args ((char *argument, char *arg));
int mult_argument args ((char *argument, char *arg));


DECLARE_DO_FUN (do_olc);
DECLARE_DO_FUN (do_stock);
DECLARE_DO_FUN (do_permaff);
DECLARE_DO_FUN (do_asave);
DECLARE_DO_FUN (do_ocopy);
DECLARE_DO_FUN (do_rcopy);
DECLARE_DO_FUN (do_mcopy);
DECLARE_DO_FUN (do_alist);
DECLARE_DO_FUN (do_rlist);
DECLARE_DO_FUN (do_mlist);
DECLARE_DO_FUN (do_blist);
DECLARE_DO_FUN (do_resets);
DECLARE_DO_FUN (do_aresets);
DECLARE_DO_FUN (do_pexit);

/* songs.c */
char *comp_string args ((int comp_type));
int comp_type args ((char *comp_str));
int max_compositions args ((CHAR_DATA * ch));

/* magic.c */
int number_of_charmies args ((CHAR_DATA * ch));
int allowable_charmies args ((CHAR_DATA * ch));
bool can_charm args ((CHAR_DATA * ch));
bool saves_dispel args ((int dis_level, int spell_level, int duration));
bool check_dispel args ((int dis_level, CHAR_DATA * victim, int sn));
void spell_poison args ((int sn, int level, CHAR_DATA * ch, void *vo,
			 int target));
void spell_curse args ((int sn, int level, CHAR_DATA * ch, void *vo,
			int target));
void spell_identify args ((int sn, int level, CHAR_DATA * ch, void *vo,
			   int target));
int check_web args ((CHAR_DATA * ch));
int find_spell args ((CHAR_DATA * ch, const char *name));
int mana_cost args ((CHAR_DATA * ch, int min_mana, int level));
int skill_lookup args ((const char *name));
int exact_skill_lookup args((const char *name));
int slot_lookup args ((int slot));
bool saves_spell args ((int level, CHAR_DATA * victim, int dam_type));
void obj_cast_spell args ((int sn, int level, CHAR_DATA * ch,
			   CHAR_DATA * victim, OBJ_DATA * obj));

/* mem.c */
void free_reset_data (RESET_DATA *);
ROOM_INDEX_DATA *copy_room_index (ROOM_INDEX_DATA *);
MOB_INDEX_DATA *copy_mob_index (MOB_INDEX_DATA *);
OBJ_INDEX_DATA *copy_obj_index (OBJ_INDEX_DATA *);
RESET_DATA *new_reset_data (void);

/* mob_talk.c */
void mob_talk (CHAR_DATA *, CHAR_DATA *, char *, int);
void respond (CHAR_DATA *, CHAR_DATA *, struct response_list *, int);
int word_match (char *, struct talk_list *);
void variable_substitution (CHAR_DATA *, char *);

/* transport.c */
void send_transport_message (int, char *);
void set_transport_flags (int, int);
int get_transport_weight (int);

/* mob_scripts.c */
void script_update (CHAR_DATA *, int);
int find_first_step (ROOM_INDEX_DATA *, ROOM_INDEX_DATA *, CHAR_DATA *);
void script_interpret (CHAR_DATA *, TRIGGER_DATA *);
void act_trigger (CHAR_DATA *, char *, char *, char *, char *);
void assign_variable (char *, char *, CHAR_DATA *);
char *translate_variables (char *, CHAR_DATA *);
char *replace_string (char *, char *, char *);

/* olc_act.c */
int get_unused_vnum (AREA_DATA *);

/* olc_save.c */
void update_statlist args ((CHAR_DATA * ch, bool Delete));
void load_statlist args ((void));
void save_area (AREA_DATA *);
void encrypt_write (char *, FILE *);


/* recycle.c */
AFFECT_DATA *new_affect (void);
void free_fake (FAKE_DATA *);
FAKE_DATA *new_fake (void);
void free_scent (SCENT_DATA *);
MEMORY_DATA *new_memory (void);
void free_memory (MEMORY_DATA *);
void free_comp (COMP_DATA *);
SCENT_DATA *new_scent (void);
COMP_DATA *new_comp (void);
EXTRA_DESCR_DATA *new_extra_descr (void);
void free_extra_descr (EXTRA_DESCR_DATA *);
void free_affect (AFFECT_DATA *);

/* special.c */
char *spec_string (SPEC_FUN *);

/* editor.c */
char *string_replace (CHAR_DATA *, char *, char *, char *);
char *first_arg (char *, char *, bool);
void string_add (CHAR_DATA *, char *);
void edit_string (CHAR_DATA *, char **);
char *format_string (char *);
bool show_string_sofar (CHAR_DATA *, bool, bool);
void edit_line_number (CHAR_DATA *, char *);
void insert_line_number (CHAR_DATA *, char *);
void dele_line_number (CHAR_DATA *, char *);
void edit_new_line (CHAR_DATA *, char *);
void search_replace (CHAR_DATA *, char *);
void insert_new_line (CHAR_DATA *, char *);
void edit_menu_display (CHAR_DATA *);
void edit_menu (CHAR_DATA *, char *argument);
void edit_string (CHAR_DATA *, char **);

/* traps.c */
extern const struct trap_info_type trapinfo[];
/* bit.c */
/*extern const struct flag_type area_flags[], sex_flags[];
extern const struct flag_type Class_flags[], pcrace_flags[];
extern const struct flag_type exit_flags[], pact_flags[];
extern const struct flag_type tact_flags[], button_flags[];
extern const struct flag_type lock_flags[], door_resets[];
extern const struct flag_type room_flags[], sector_flags[];
// Adeon 6/30/03 -- Had to expand upon room flags
extern const struct flag_type room_flags2[];
extern const struct flag_type tsector_flags[], type_flags[];
extern const struct flag_type extra_flags[MAX_EXTRA_FLAGS][31], wear_flags[];
extern const struct flag_type clan_flags[];
extern const struct flag_type act_flags[], affect_flags[];
// Adeon 7/1/03 -- Expanding up Mob Act Flags
extern const struct flag_type act_flags2[];
extern const struct flag_type recruit_flags[];
extern const struct flag_type apply_flags[], rarity_flags[];
extern const struct flag_type wear_loc_strings[], wear_loc_flags[];
extern const struct flag_type weapon_flags[], container_flags[];
extern const struct flag_type liquid_flags[], material_type[];
extern const struct flag_type form_flags[], part_flags[];
extern const struct flag_type ac_type[], size_flags[];
extern const struct flag_type off_flags[], imm_flags[];
extern const struct flag_type res_flags[], vuln_flags[];
extern const struct flag_type position_flags[], weapon_Class[];
extern const struct flag_type instrument_flags[], furniture_type[];
extern const struct flag_type weapons_type[], card_Class[], card_terrain_type[];
extern const struct flag_type card_element_type[], card_race_type[], card_terrain_toggles[];
extern const struct flag_type trap_type[];
extern const struct flag_type trap_flag[];
extern const struct flag_type dir_flags[];

extern const struct wear_translation_type wear_conversion_table[];*/

//char *flag_string (const struct flag_type *, int);
//char *flag_string2 (const struct flag_type **flag_table, int max_elements, int bits[]);
//char *crflag_string (const struct flag_type *, int);

/* travel.c */
void check_travel_point (CHAR_DATA * ch);


/* resets.c */
void display_resets (CHAR_DATA *);

/* epl.c */
void load_room_prog (ROOM_INDEX_DATA *, char *);

/* olc.c */
void vocab_main (CHAR_DATA *, char *);
void vocab_edit (CHAR_DATA *, char *);
void vocab_add_words (CHAR_DATA *, char *);
void vocab_add_mood (CHAR_DATA *, char *);
void vocab_add_mood_offset (CHAR_DATA *, char *);
void vocab_add_text (CHAR_DATA *, char *);
void vocab_add_include (CHAR_DATA *, char *);
void mail_get_subject (CHAR_DATA *, char *);
void submit_mob_vnum (CHAR_DATA *, char *);
void submit_mob_name (CHAR_DATA *, char *);
void submit_room_vnum (CHAR_DATA *, char *);
bool is_builder (CHAR_DATA *, AREA_DATA *);
bool run_olc_editor (DESCRIPTOR_DATA *);
char *read_current_help (char *, char *);
char *get_player_email (char *);
char *olc_ed_name (CHAR_DATA *);
char *olc_ed_vnum (CHAR_DATA *);
void add_reset (ROOM_INDEX_DATA *, RESET_DATA *, int);
void send_mail (char *);
void send_submission (char *, char *);
void remove_help (char *, char *);
void save_help (char *, char *, char *);

/* reaver.c */
int compute_soul_gain (CHAR_DATA *, CHAR_DATA *);
void update_sword (CHAR_DATA * ch, OBJ_DATA * sword);


/* save.c */
void load_ctransports (void);
void fread_mount (CHAR_DATA *, FILE *);
void load_one_ctransport (FILE *);
void fwrite_ctransport (OBJ_DATA *, FILE *);
void fwrite_mount (CHAR_DATA *, FILE *);
void save_ctransports (void);
void save_char_obj (CHAR_DATA *);
bool load_char_obj (DESCRIPTOR_DATA *, char *);
void fwrite_objdb (OBJ_DATA * pObj, FILE * fp, int iNest);
void fread_objdb (OBJ_DATA * pObj, FILE * fp);

/* skills.c */
bool has_skills (CHAR_DATA *, int);
bool parse_gen_groups (CHAR_DATA *, char *);
void list_group_costs (CHAR_DATA *);
void list_group_known (CHAR_DATA *);
int exp_per_level (CHAR_DATA *, int);
void check_improve (CHAR_DATA *, int, bool, int);
int group_lookup (const char *);
void gn_add (CHAR_DATA *, int);
void gn_remove (CHAR_DATA *, int);
void group_add (CHAR_DATA *, const char *, bool);
void group_add_all (CHAR_DATA *);
void group_remove (CHAR_DATA *, const char *);

/* special.c */
SPEC_FUN *spec_lookup (const char *);
char *spec_name (SPEC_FUN *);

/* teleport.c */
ROOM_INDEX_DATA *room_by_name (char *, int, bool);

/* update.c */
bool is_aggro (CHAR_DATA *);
void advance_level (CHAR_DATA *);
long int gain_exp (CHAR_DATA *, long);
void gain_condition (CHAR_DATA *, int, int);
void update_handler (void);

/* bounty.c */
bool is_bountied args ((CHAR_DATA * ch));

/* gamble.c */
void bet_update args ((void));

/* reaver.c */
void perform_chant (CHAR_DATA *);

/* obj_trig.c */
void trip_triggers(CHAR_DATA *ch, long trig_type, OBJ_DATA *obj,
		                                   CHAR_DATA *vict, int special);
void trip_triggers_arg(CHAR_DATA *ch, long trig_type, OBJ_DATA *obj,
		                                           CHAR_DATA *vict, int special, char *arg);
void trip_triggers_special(CHAR_DATA *ch, long trig_type, OBJ_DATA *obj,
		                CHAR_DATA *victim, int special, OBJ_TRIG* ot);

OBJ_TRIG_DATA *get_obj_trig(int vnum);

/* obj_trig_save.c */
void load_obj_trigs (FILE *fp);

/* fight2.c */
bool check_aggression(CHAR_DATA *ch);
int adjust_damage(int dam_to_do);
	
/* swarm.c */
void swarm_heal (CHAR_DATA *ch, CHAR_DATA *victim, int sn);

/* traps.c */
void trap_check(CHAR_DATA *ch,char *type, ROOM_INDEX_DATA *room, OBJ_DATA *obj);
int trap_info_lookup(int traptype);
bool detect_room_trap (CHAR_DATA *ch,ROOM_INDEX_DATA * room);
bool detect_obj_trap (CHAR_DATA *ch,OBJ_DATA *obj);

/* chaosjester.c */
void familiar_poof args((CHAR_DATA *ch));
bool equip_char_slot_new args((CHAR_DATA * ch, OBJ_DATA * obj, long slot));

#endif

