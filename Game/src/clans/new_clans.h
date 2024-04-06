//
// new_clans.h
//
// Programmer: Akamai
// Date: 7/10/98
// Version: 1.0
//
// This code was rewritten from the ground up in order to fix the
// problematic dependencies on gndbm. The problems with gndbm under
// Linux broke many of the clan operations.
//

#ifndef __new_clans_h__
#define __new_clans_h__

#define MAX_CLAN 128		// no more than 128 clans in play at one time
#define MAX_NAME  36		// max 36 characters for clan name
#define MAX_SYMB  36		// max 36 characters for clan symbol with colors
#define MAX_COLOR 24		// max 24 coloring characters in name or symbol
#define MAX_NAME_CH  (MAX_NAME-MAX_COLOR)
#define MAX_SYMB_CH  (MAX_SYMB-MAX_COLOR)
#define CLAN_DIR  "../clans"
#define CLANINDEX "clanindex"
#define NOCLAN_NAME "none"
#define CLANS_START        1	// the clan_table starts with the first clan
#define CLAN_BOGUS         0	// clan_table[0] is the bogus clan
#define CLAN_BOGUS_NAME    "ClanBogus"
#define CLAN_BOGUS_SYMB    "`f<`cBOGUS`f>``"
//#define MIN_CLAN_LEVEL     8 // for test only level 8
//#define MIN_MEMBER_LEVEL   6 // for test only level 6
#define MIN_CLAN_LEVEL    20	// player must be at least level 20 to start a clan
#define MIN_MEMBER_LEVEL   9	// must be at least level 8 to be a clan member

//
//  Clan status types
//
#define CLAN_STATUSES       7	// The number of statuses
#define HIDDEN_STATUSES     3	// the number of hidden statuses
// the clan statuses need to be entered in the clan_statuses array
// so that they can be indexed when used all hidden statuses should
// be listed before any visible statuses and have status type values
// lower than all visible statuses.
#define CLAN_INACTIVE       3	// This clan has been made inactive for now
#define CLAN_DISBAND        5	// Clan was disbanded
#define CLAN_DISPERSED      7	// Clan was dispersed
#define CLAN_HISTORICAL     9	// This clan has historical significance
#define CLAN_PROPOSED      11	// Clan is a proposed clan
#define CLAN_RESTRICTED    13	// Clan has lost too many members levels
#define CLAN_ACTIVE        17	// Clan is an active clan

//
//  Clan types
//
#define CLAN_TYPE_TEST      3	// Clans that can be used for testing
#define CLAN_TYPE_MOB       5	// Clans can now be composed of mobs
#define CLAN_TYPE_PC      100	// Unrestricted PC clan
#define CLAN_TYPE_DEM     200	// Democratic allow elections
#define CLAN_TYPE_REL     300	// Religious orders
#define CLAN_TYPE_TECH    400	// Technologist clan
#define CLAN_TYPE_EXPL    500	// Explorer clan
#define CLAN_TYPE_MIL     600	// A militaristic clan - ranks, promotion, etc.

//
// Flags support clan specific objects
//
#define CLAN_TEST        (A)
#define CLAN_MOB         (B)
#define CLAN_PC          (F)
#define CLAN_DEM         (G)
#define CLAN_REL         (H)
#define CLAN_TECH        (I)
#define CLAN_EXPL        (J)
#define CLAN_MIL         (K)

//
// Clan identifier numbers, like clan VNUMs
//
#define CLAN_RES_IDENT_MIN      1	// some clan identifiers are reserved
#define CLAN_RES_IDENT_MAX   4095	// just in case I need them later
#define CLAN_PC_IDENT_MIN    4095	// Clans for PCs will start with this number

// 
// Clan member status flags
//
#define CLAN_APPLIED     1
#define CLAN_DENIED      3
#define CLAN_NOTIFIED    5
#define CLAN_ACCEPTED   10
//
// Militaristic clans can have ranks for members
//
#define CLAN_PRIVATE    10
#define CLAN_CORPORAL   20
#define CLAN_SERGEANT   21
#define CLAN_SERMAJOR   22
#define CLAN_SECLEUT    23
#define CLAN_LIEUTENANT 24
#define CLAN_CAPTAIN    25
#define CLAN_MAJOR      26
#define CLAN_CORNEL     27
#define CLAN_GENERAL1   50
#define CLAN_GENERAL2   51
#define CLAN_GENERAL3   52
#define CLAN_GENERAL4   53
//
// The notion of deputy and clan leader overlay the militaristic ranks
// The democratic clans have special council players
//
#define CLAN_COUNCIL    50
#define CLAN_DEPUTY     50
#define CLAN_LEADER    100
//
// Clan relations status flags
//
#define CLAN_NEUTRAL   30
#define CLAN_ALLY      50
#define CLAN_ENEMY     70
#define CLAN_WAR      100	// A Clan war
#define CLAN_WAR_AGGR 101	// The agressor gets this flag
#define CLAN_WAR_VICT 102	// The clan war victim gets this one
//
// Clan alignment flags
//
#define CLAN_NOALIGN        1
#define CLAN_NEUTRAL_ALIGN  2
#define CLAN_EVIL_ALIGN     3
#define CLAN_GOOD_ALIGN     4
//
// Clan war kill points
//
#define OOL_KILL_DIFF       PLAYER_LEVEL_DIFF
#define CLAN_MEMBER_KILL   10
#define CLAN_DEPUTY_BONUS   2	// bonus is a multiplier
#define CLAN_LEADER_BONUS   5
//
//
//
//#define FULL_CLAN_LEVELS   60 // testing only 60 levels for full clan
//#define LEVELS_PER_ALLY    75 // testing only 75
//#define LEVELS_PER_ENEMY   75 // testing only 75
#define FULL_CLAN_LEVELS   200	// need 200 pc levels to become a full clan
#define LEVELS_PER_ALLY    300	// allow 1 ally per every 300 pc levels
#define LEVELS_PER_ENEMY   300	// allow 1 enemy per every 300 pc levels
#define CLAN_ALLIES_MAX     15	//
#define CLAN_ENEMIES_MAX    15	//
#define CLAN_WARS_MAX       10	//
#define CLAN_ONE_DAY     86400	// 24 hours represented in seconds (24*60*60)
#define CLAN_WAR_TIME    ( 3*CLAN_ONE_DAY)	// clan wars last 3 days
#define RECENT_WAR_TIME  ( 7*CLAN_ONE_DAY)	// clan wars are recent for 1 week
#define CLAN_ENEMY_TIME  ( 2*CLAN_ONE_DAY)	// got to be enemies 2 days
//#define CLAN_WAR_TIME    864 // clan war length testing
//#define RECENT_WAR_TIME  432 // recent for testing
//#define CLAN_ENEMY_TIME  180 // enemy for testing

typedef struct clan_member MEMBER;
typedef struct clan_relations RELATIONS;

//
//  The clan table is composed of these
//
struct clan_type
{
  char name[MAX_NAME + 1];	// The name of this clan

  char symbol[MAX_SYMB + 1];	// The symbol of this clan

  MEMBER *leader;		// The leader of this clan

  int ctype;			// Clans are now typed

  int ident;			// Clans have a unique identifier, like a vnum

  int hall;			// Vnum of the clan hall

  int status;			// Flag set when clan is promoted to full clan

  int levels;			// The total number of levels of this clan

  int align;			// The clan has an alignment

  MEMBER *members;		// A list of the members

  MEMBER *applicants;		// A list of players applying to this clan

  RELATIONS *relations;		// A list of this clans relations to other clans

  OBJ_DATA *clan_box;		// The clan box

};


//
// Linked list of players, used for membership, application, etc
//
struct clan_member
{
  MEMBER *next;
  char name[MAX_NAME + 1];	// the player name

  int levels;			/* levels of the player at application time
				   this needs to be stored even though it's used
				   very infrequently, sigh. */

  int align;			// same with this, sigh.

  int race;			// same

  int Class;			// same

  int status;			// the status of this player in the clan

  int initiative;		// this a players vote or petition

  time_t laston;		// the last time that this player logged on

};

//
// The relationship to other clans
//
struct clan_relations
{
  RELATIONS *next;		// next clan in the list

  char name[MAX_NAME];		// name of the clan

  int ident;			// the vnum of this clan

  int status;			// the status of the relationship

  time_t acttime;		// the time of the last status change

  time_t wartime;		// the time of the last war with this clan

  int warcount;			// number of times war has broken out with this clan

  int loss;			// number of times 'we' lost to this clan

  int wins;			// number of times 'we' beat this clan

  int kills;			// number of other players 'we' killed in the current war

  int points;			// number of points accumulated in the war

};


#endif // __new_clans_h__
