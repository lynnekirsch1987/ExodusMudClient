#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "songs.h"
#include "pray.h"
#include "magic.h"
#include "interp.h"
#include "chant.h"

/* item type list */
const struct item_type item_table[] =
{
	{ITEM_LIGHT, "light"},
	{ITEM_SCROLL, "scroll"},
	{ITEM_WAND, "wand"},
	{ITEM_STAFF, "staff"},
	{ITEM_WEAPON, "weapon"},
	{ITEM_TREASURE, "treasure"},
	{ITEM_ARMOR, "armor"},
	{ITEM_POTION, "potion"},
	{ITEM_CLOTHING, "clothing"},
	{ITEM_FURNITURE, "furniture"},
	{ITEM_TRASH, "trash"},
	{ITEM_CONTAINER, "container"},
	{ITEM_CLAN_DONATION, "clan_donation"},
	{ITEM_NEWCLANS_DBOX, "newclan_donation"},	/* new clans */
	{ITEM_PLAYER_DONATION, "player_box"},
	{ITEM_SADDLE, "saddle"},
	{ITEM_PACK, "pack"},
	{ITEM_ELEVATOR, "elevator"},
	{ITEM_RAFT, "raft"},
	{ITEM_STRANSPORT, "stransport"},
	{ITEM_CTRANSPORT, "ctransport"},
	{ITEM_EBUTTON, "ebutton"},
	{ITEM_VIAL, "vial"},
	{ITEM_PARCHMENT, "parchment"},
	{ITEM_DRINK_CON, "drink"},
	{ITEM_KEY, "key"},
	{ITEM_FOOD, "food"},
	{ITEM_MONEY, "money"},
	{ITEM_GILLS, "gills"},
	{ITEM_CORPSE_NPC, "npc_corpse"},
	{ITEM_CORPSE_PC, "pc_corpse"},
	{ITEM_FOUNTAIN, "fountain"},
	{ITEM_PILL, "pill"},
	{ITEM_PROTECT, "protect"},
	{ITEM_MAP, "map"},
	{ITEM_PORTAL, "portal"},
	{ITEM_WARP_STONE, "warp_stone"},
	{ITEM_ROOM_KEY, "room_key"},
	{ITEM_GEM, "gem"},
	{ITEM_JEWELRY, "jewelry"},
	{ITEM_JUKEBOX, "jukebox"},
	{ITEM_INSTRUMENT, "instrument"},
	{ITEM_WRITING_INSTRUMENT, "writing_instrument"},
	{ITEM_WRITING_PAPER, "writing_paper"},
	{ITEM_CTRANSPORT_KEY, "ctransport_key"},
	{ITEM_PORTAL_BOOK, "portal_book"},
	{ITEM_QUIVER, "quiver"},
	{ITEM_FEATHER, "feather"},
	{ITEM_POLE, "pole"},
	{ITEM_BAIT, "bait"},
	{ITEM_TREE, "tree"},
	{ITEM_WOOD, "wood"},
	{ITEM_SEED, "seed"},
	{ITEM_WOODEN_INCONSTRUCTION, "wooden_in_construction"},
	{ITEM_CARD,	"card"},
	{ITEM_BINDER,	"binder"},
	{ITEM_OBJ_TRAP, "objecttrap"},
	{ITEM_PORTAL_TRAP, "portaltrap"},
	{ITEM_ROOM_TRAP, "roomtrap"},
	{ITEM_RANDOM, "random"},
	{0, NULL}
};

const struct tree_type tree_table[] =
{
	{TREE_TYPE_OAK, "oak", 6, RARITY_COMMON, "all",3,0},
	{TREE_TYPE_MAPLE, "maple", 5, RARITY_COMMON, "all",3,0},
	{TREE_TYPE_BIRCH, "birch", 5, RARITY_COMMON, "all",3,0},
	{TREE_TYPE_PINE, "pine", 5, RARITY_COMMON, "all",3,0},
	{TREE_TYPE_WILLOW, "willow", 4, RARITY_COMMON, "all",3,0},
	{TREE_TYPE_ELM, "elm", 4, RARITY_COMMON, "all",3,0},
	{TREE_TYPE_KARRI, "karri", 4, RARITY_COMMON, "all",3,0},
	{TREE_TYPE_MARRI, "marri", 4, RARITY_COMMON, "all",3,0},
	{TREE_TYPE_JARRAH, "jarrah", 5, RARITY_COMMON, "all",3,0},
	{TREE_TYPE_POPLAR, "poplar", 6, RARITY_COMMON, "all",3,0},
	{TREE_TYPE_MOONBLOSSOM, "moonblossom", 1, RARITY_RARE, "`oThe City of The `kSun``",10,0},
	{TREE_TYPE_SLIMEWOOD, "slimewood", 8, RARITY_VERY_RARE, "`dThe Desert of Ath'Tanar``",-10,0},
	{TREE_TYPE_RISIRIEL, "risiriel", 3, RARITY_RARE, "`nSilver `lCrescent `nWoods``",0,5},
	{TREE_TYPE_JEWELWOOD, "jewelwood", 10, RARITY_VERY_RARE, "`kUnicorn Point``",7,7},
	{TREE_TYPE_ROTSTENCH, "rotstench", 2, RARITY_COMMON, "`aLu`hna`or Pl`hai`ans``",-15,-15},
	{TREE_TYPE_IRONWOOD, "ironwood", 9, RARITY_RARE, "`lCove``",3,0},
	{TREE_TYPE_HELLROOT, "hellroot", 7, RARITY_UNCOMMON, "all",-15,-15},
	{TREE_TYPE_BARBVINE, "barbvine", 3, RARITY_UNCOMMON, "all",-10,0},
	{TREE_TYPE_GREEN_LEECHLEAF, "green leechleaf", 7, RARITY_UNCOMMON, "all",0,-15},
	{0,NULL,0,0,NULL,0,0}
};

const struct item_type card_table[] =
{
	{CARD_MOB,			"mob"},
	{CARD_TERRAIN,			"terrain"},
	{CARD_SPELL_HEAL,		"spell_heal"},
	{CARD_SPELL_DAMAGE,		"spell_damage"},
	{CARD_SPELL_BOOST,		"spell_boost"},
	{CARD_SPELL_COUNTER,		"spell_counter"},
	{0, NULL}
};

#define WOODEN_OBJ_CLUB                 3
#define WOODEN_OBJ_DAGGER               4
#define WOODEN_OBJ_POLEARM              5
#define WOODEN_OBJ_STAFF                6
#define WOODEN_OBJ_SWORD                7
#define WOODEN_OBJ_SHIELD               8

const struct wooden_obj_type wooden_obj_table[] =
{
	{WOODEN_OBJ_CHAIR, 60, 2, 30030},
	{WOODEN_OBJ_CUP, 12, 1, 30031},
	{WOODEN_OBJ_PLAQUE, 20, 1, 30030},
	{WOODEN_OBJ_CLUB, 20, 1, 30030},
	{0,0,0,0}
};

/* weapon selection table */
const struct weapon_type weapon_table[] =
{
	{"sword", OBJ_VNUM_SCHOOL_SWORD, WEAPON_SWORD, &gsn_sword},
	{"mace", OBJ_VNUM_SCHOOL_MACE, WEAPON_MACE, &gsn_mace},
	{"dagger", OBJ_VNUM_SCHOOL_DAGGER, WEAPON_DAGGER, &gsn_dagger},
	{"axe", OBJ_VNUM_SCHOOL_AXE, WEAPON_AXE, &gsn_axe},
	{"staff", OBJ_VNUM_SCHOOL_STAFF, WEAPON_STAFF, &gsn_staff},
	{"flail", OBJ_VNUM_SCHOOL_FLAIL, WEAPON_FLAIL, &gsn_flail},
	{"polearm", OBJ_VNUM_SCHOOL_POLEARM, WEAPON_POLEARM, &gsn_polearm},
	{"hand to hand", 0, 0, &gsn_hand_to_hand}, {NULL, 0, 0, NULL}
};

/* wiznet table and prototype for future flag setting */
const struct wiznet_type wiznet_table[] =
{
	{"on", WIZ_ON, IM},
	{"prefix", WIZ_PREFIX, IM},
	{"ticks", WIZ_TICKS, IM},
	{"logins", WIZ_LOGINS, IM},
	{"sites", WIZ_SITES, L4},
	{"links", WIZ_LINKS, L7},
	{"newbies", WIZ_NEWBIE, IM},
	{"spam", WIZ_SPAM, L5},
	{"deaths", WIZ_DEATHS, IM},
	{"resets", WIZ_RESETS, L4},
	{"mobdeaths", WIZ_MOBDEATHS, L4},
	{"flags", WIZ_FLAGS, L5},
	{"penalties", WIZ_PENALTIES, L5},
	{"saccing", WIZ_SACCING, L5},
	{"levels", WIZ_LEVELS, IM},
	{"load", WIZ_LOAD, L2},
	{"restore", WIZ_RESTORE, L2},
	{"snoops", WIZ_SNOOPS, L2},
	{"switches", WIZ_SWITCHES, L2},
	{"secure", WIZ_SECURE, ML},
	{"clan", WIZ_CLAN, L4},
	{"time", WIZ_TSTAMP, L4},
	{"notes", WIZ_NOTES, 92},
	{"poll", WIZ_POLL, 92},
	{"quest", WIZ_QUEST, 92},
	{"stockbuy", WIZ_STOCKBUY, 92},
	{NULL, 0, 0}
};

/* attack table  -- not very organized :( */
const struct attack_type attack_table[] =
{
	{"none", "hit", -1},	/*  0 */
	{"slice", "slice", DAM_SLASH},
	{"stab", "stab", DAM_PIERCE},
	{"slash", "slash", DAM_SLASH},
	{"whip", "whip", DAM_SLASH},
	{"claw", "claw", DAM_SLASH},
	{"blast", "blast", DAM_BASH},
	{"pound", "pound", DAM_BASH},
	{"crush", "crush", DAM_BASH},
	{"grep", "grep", DAM_SLASH},
	{"bite", "bite", DAM_PIERCE},
	{"pierce", "pierce", DAM_PIERCE},
	{"suction", "suction", DAM_BASH},
	{"beating", "beating", DAM_BASH},
	{"digestion", "digestion", DAM_ACID},
	{"charge", "charge", DAM_BASH},
	{"slap", "slap", DAM_BASH},	/* 15 */
	{"punch", "punch", DAM_BASH},
	{"wrath", "wrath", DAM_ENERGY},
	{"magic", "magic", DAM_ENERGY},
	{"divine", "divine power", DAM_HOLY},
	{"cleave", "cleave", DAM_SLASH},	/* 20 */
	{"scratch", "scratch", DAM_PIERCE},
	{"peck", "peck", DAM_PIERCE},
	{"peckb", "peck", DAM_BASH},
	{"chop", "chop", DAM_SLASH},
	{"sting", "sting", DAM_PIERCE},	/* 25 */
	{"smash", "smash", DAM_BASH},
	{"shbite", "shocking bite", DAM_LIGHTNING},
	{"flbite", "flaming bite", DAM_FIRE},
	{"frbite", "freezing bite", DAM_COLD},
	{"acbite", "acidic bite", DAM_ACID},	/* 30 */
	{"chomp", "chomp", DAM_PIERCE},
	{"drain", "life drain", DAM_NEGATIVE},
	{"thrust", "thrust", DAM_PIERCE},
	{"slime", "slime", DAM_ACID},
	{"shock", "shock", DAM_LIGHTNING},	/* 35 */
	{"thwack", "thwack", DAM_BASH},
	{"flame", "flame", DAM_FIRE},
	{"chill", "chill", DAM_COLD},
	{"vbite", "vampiric bite", DAM_VAMPIRIC}, //[40]
	{"lclaw", "lightning claw", DAM_LIGHTNING},
	{"breath", "breath", DAM_WIND},
	{"spit", "spit", DAM_DROWNING},
	{"splash", "splash", DAM_DROWNING},
	{"wslash", "wind slash", DAM_WIND},
	{NULL, NULL, 0}
};

//if you modify above, modify MAX_DAMAGE_MESSAGE in fight.c

/* race table */
const struct race_type race_table[] = {
	/*
	{ name, pc_race?, act bits, aff_by bits, off bits, imm, res, vuln, form, parts },
	*/
	{"unique", FALSE, 0, 0, 0, 0, 0, 0, 0, 0},
	{"human", TRUE, 0, 0, 0, 0, 0, 0, A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | aa},
	{"elf", TRUE, 0, AFF_INFRARED |	AFF_DETECT_MAGIC | AFF_DETECT_HIDDEN, 0, 0, RES_CHARM | RES_MAGIC, VULN_IRON, A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | aa},
	{"dwarf", TRUE, 0, AFF_INFRARED, 0, 0, RES_POISON | RES_DISEASE, VULN_DROWNING, A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | aa},
	{"canthi", TRUE, 0, AFF_AQUA_BREATHE, 0, IMM_DROWNING, RES_FIRE, VULN_COLD, A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | aa},
	{"vro'ath", TRUE, 0, 0, 0, 0, 0, VULN_MENTAL | VULN_HOLY, A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | aa},
	{"syvin", TRUE, 0, AFF_DARK_VISION, 0, 0, RES_WOOD, VULN_IRON, A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | aa},
	{"sidhe", TRUE, 0, AFF_FLYING | AFF_PASS_DOOR, 0, IMM_BASH,	RES_MENTAL | RES_CHARM, VULN_ENERGY | VULN_LIGHT, C | H | M, A | B | C | E | G | H | P | aa},
	{"avatar", TRUE, 0, 0, 0, 0, RES_HOLY, 0, A | H | M | V, A | B | C |	D | E | F |	G | H | I | J | K | aa},
	{"litan", TRUE, 0, 0, 0, IMM_FIRE, 0, VULN_COLD | VULN_DROWNING, A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | aa},
	{"kalian", TRUE, 0, AFF_REGENERATION, 0, IMM_POISON, RES_WEAPON | RES_COLD | RES_LIGHTNING | RES_BASH  , VULN_HOLY | VULN_FIRE, A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | aa},
	{"lich", TRUE, ACT_UNDEAD, AFF_DETECT_INVIS | AFF_DETECT_HIDDEN | AFF_AQUA_BREATHE | AFF_DETECT_GOOD | AFF_DETECT_INVIS | AFF_DETECT_HIDDEN, 0, IMM_DISEASE | IMM_POISON , RES_MAGIC | RES_COLD | RES_NEGATIVE, VULN_HOLY | VULN_LIGHT | VULN_ENERGY | VULN_FIRE, A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | aa},
	{"nerix",  TRUE, 0, 0, 0, 0, RES_HOLY | RES_LIGHTNING | RES_WEAPON, VULN_MAGIC | VULN_NEGATIVE, A | H | M | W, A | B | C | D | E | F | G | H | I | J | K | P | Q | U | Z | aa},
	{"giant", FALSE, 0, 0, 0, 0, RES_FIRE | RES_COLD, VULN_MENTAL | VULN_LIGHTNING, A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | aa},
	{"cloudgiant", FALSE, 0, AFF_FLYING, 0, IMM_LIGHTNING, RES_COLD, VULN_MENTAL, A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | aa},
	{"bat",	FALSE, 0, AFF_FLYING | AFF_DARK_VISION, OFF_DODGE | OFF_FAST, 0, 0, VULN_LIGHT, A | G | V, A | C | D | E | F | H | J | K | P | aa},
	{"bear", FALSE, 0, 0, OFF_CRUSH | OFF_DISARM | OFF_BERSERK, 0, RES_BASH | RES_COLD, 0, A | G | V, A | B | C | D | E | F | H | J | K | U | V | aa},
	{"cat", FALSE, 0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE, 0, 0, 0, A | G | V, A | C | D | E | F | H | J | K | Q | U | V | aa},
	{"centipede", FALSE, 0, AFF_DARK_VISION, 0, 0, RES_PIERCE | RES_COLD, VULN_BASH, A | B | G | O, A | C | K | aa},
	{"dog", FALSE, 0, 0, OFF_FAST, 0, 0, 0, A | G | V, A | C | D | E | F | H | J | K | U | V | aa},
	{"doll", FALSE, 0, 0, 0, IMM_COLD | IMM_POISON | IMM_HOLY | IMM_NEGATIVE | IMM_MENTAL | IMM_DISEASE | IMM_DROWNING, RES_BASH | RES_LIGHT, VULN_SLASH | VULN_FIRE | VULN_ACID | VULN_LIGHTNING |	VULN_ENERGY, E | J | M | cc, A | B | C | G | H | K | aa},
	{"dragon", FALSE, 0, AFF_INFRARED | AFF_FLYING, 0, 0, RES_FIRE | RES_BASH | RES_CHARM, VULN_PIERCE | VULN_COLD, A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X | aa},
	{"fido", FALSE, 0, 0, OFF_DODGE | ASSIST_RACE, 0, 0, VULN_MAGIC, A | B | G | V,	A | C | D | E | F | H | J | K | Q | V | aa},
	{"fox", FALSE, 0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE, 0, 0, 0, A | G | V, A | C | D | E | F | H | J | K | Q | V | aa},
	{"goblin", FALSE, 0, AFF_INFRARED, 0, 0, RES_DISEASE, VULN_MAGIC, A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | aa},
	{"hobgoblin", FALSE, 0, AFF_INFRARED, 0, 0, RES_DISEASE | RES_POISON, 0, A | H | M | V,	A | B | C | D | E | F | G | H | I | J | K | Y | aa},
	{"kobold", FALSE, 0, AFF_INFRARED, 0, 0, RES_POISON, VULN_MAGIC, A | B | H | M | V,	A | B | C | D | E | F | G | H | I | J | K | Q | aa},
	{"lizard", FALSE, 0, 0, 0, 0, RES_POISON, VULN_COLD, A | G | X | cc, A | C | D | E | F | H | K | Q | V | aa},
	{"modron", FALSE, 0, AFF_INFRARED, ASSIST_RACE | ASSIST_ALIGN, IMM_CHARM | IMM_DISEASE | IMM_MENTAL | IMM_HOLY | IMM_NEGATIVE, RES_FIRE | RES_COLD | RES_ACID, 0, H, A | B | C | G | H | J | K | aa},
	{"orc", FALSE, 0, AFF_INFRARED, 0, 0, RES_DISEASE, VULN_LIGHT, A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | aa},
	{"pig", FALSE, 0, 0, 0, 0, 0, 0, A | G | V, A | C | D | E | F | H | J | K | aa},
	{"rabbit", FALSE, 0, 0,	OFF_DODGE | OFF_FAST, 0, 0, 0, A | G | V, A | C | D | E | F | H | J | K | aa},
	{"school monster", FALSE, 0, 0, 0, IMM_CHARM | IMM_SUMMON, 0, VULN_MAGIC, A | M | V, A | B | C | D | E | F | H | J | K | Q | U | aa},
	{"snake", FALSE, 0, 0, 0, 0, RES_POISON, VULN_COLD, A | G | X | Y | cc, A | D | E |	F | K | L |	Q | V | X | aa},
	{"song bird", FALSE, 0, AFF_FLYING, OFF_FAST | OFF_DODGE, 0, 0, 0, A | G | W, A | C | D | E | F | H | K | P | Z | aa},
	{"troll", FALSE, 0, AFF_REGENERATION | AFF_INFRARED | AFF_DETECT_HIDDEN, OFF_BERSERK, 0, RES_CHARM | RES_BASH, VULN_FIRE | VULN_ACID, A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V | aa},
	{"water fowl", FALSE, 0, AFF_SWIM | AFF_FLYING, 0, 0, RES_DROWNING, 0, A | G | W, A | C | D | E | F | H | K | P | Z | aa},
	{"wolf", FALSE, 0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE, 0, 0, 0, A | G | V, A | C | D | E | F | J | K | Q | V | aa},
	{"wyvern", FALSE, 0, AFF_FLYING | AFF_DETECT_INVIS | AFF_DETECT_HIDDEN,	OFF_BASH | OFF_FAST | OFF_DODGE, IMM_POISON, 0, VULN_LIGHT,	A | B | G | Z, A | C | D | E | F | H | J | K | Q | V | X | aa},
	{"fish", TRUE, 0, AFF_AQUA_BREATHE, 0, IMM_DROWNING, RES_FIRE, VULN_COLD, A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | aa},
	{"unique", FALSE, 0, 0, 0, 0, 0, 0, 0, 0},
	{NULL, 0, 0, 0, 0, 0, 0}
};

const struct pc_race_type pc_race_table[] =
{
	{"null race", "", 0, {100, 100, 100, 100, 100, 100}, {""}, {13, 13, 13, 13, 13, 13}, {18, 18, 18, 18, 18, 18}, 0},
	/*
	{ "race name", short name, points, { class multipliers }, { bonus skills }, { base stats }, { max stats }, size },
	*/

	// Minax 7-4-02 Gave Vro'ath Hand to Hand and Fisticuffery for free, but
	// they will only get it if they are warriors or thieves I think.

	{"human", "Human", 0, {100, 100, 100, 100, 100, 100, 100, 100, 100}, {""}, {13, 13, 13, 13, 13, 13}, {18, 18, 18, 18, 18, 18}, SIZE_MEDIUM},
	{"elf", " Elf ", 6, {100, 125, 100, 120, 110, 125, 120, 125, 125}, {"sneak"}, {12, 14, 13, 15, 11, 13},	/*avg 13 */ {16, 22, 18, 21, 15, 20}, SIZE_MEDIUM}, /* avg 18.7 */
	{"dwarf", "Dwarf", 5, {150, 100, 140, 100, 110, 130, 140, 100, 110}, {"berserk"}, {14, 12, 12, 10, 14, 11}, /* avg 12.2 */ {21, 16, 18, 15, 21, 16}, SIZE_MEDIUM}, /* avg 17.8 */
	{"canthi", "Canthi", 6, {115, 115, 125, 150, 105, 110, 150, 100, 105}, {"swimming"}, {13, 13, 13, 14, 13, 12}, /* avg 13 */ {17, 17, 19, 20, 18, 20}, SIZE_MEDIUM}, /* avg 18.5 */
	{"vro'ath", "Vroath", 6, {150, 130, 130, 100, 120, 130, 150, 120, 130}, {"bash", "hand to hand", "fisticuffery"}, {14, 10, 11, 10, 15, 10}, /* avg 11.7 */ {22, 13, 15, 14, 21, 13}, SIZE_LARGE},	/* avg 16.3 */
	{"syvin", "Syvin", 7, {120, 125, 110, 130, 100, 120, 120, 110, 100}, {"lore", "track"}, {12, 14, 13, 15, 11, 13}, /* avg 13 */ {18, 17, 19, 21, 20, 17}, SIZE_MEDIUM},	/* avg 18.7 */
	{"sidhe", "Sidhe", 8, {120, 150, 120, 150, 150, 120, 150, 100, 100}, {"mask", "hide"}, {10, 15, 13, 13, 10, 15}, /* 12.7 */ {13, 23, 18, 19, 14, 20}, SIZE_MEDIUM},	/* avg 17.8 */
	{"avatar", "Avatar", 15, {150, 150, 150, 150, 150, 150, 150, 150, 150}, {""}, {13, 13, 13, 13, 13, 13}, {25, 25, 25, 25, 25, 25}, SIZE_MEDIUM},
	{"litan", "Litan", 12, {120, 125, 110, 130, 100, 120, 120, 110, 100}, {""}, {13, 5, 8, 18, 13, 7}, /* avg 10.6667 */ {17, 10, 13, 22, 17, 12}, SIZE_MEDIUM}, /* avg 15.166*/
	{"kalian", "Kalian", 25, {120, 150, 120, 150, 150, 120, 150, 100, 100}, {"sneak"}, {15, 15, 15, 13, 8, 10}, /* 12.666 */ {23, 23, 16, 23, 10, 16}, SIZE_MEDIUM}, /* avg 18.5 */
	{"lich", "Lich", 25, {120, 150, 120, 150, 150, 120, 150, 100, 100}, {""}, {16, 16, 11, 11, 16, 8}, /* 13 */ {21, 21, 16, 16, 21, 14}, SIZE_MEDIUM}, /* avg 18.17 */
	{"nerix", "Nerix", 25, {120, 150, 120, 150, 150, 120, 150, 100, 100}, {""}, {16, 10, 10, 16, 13, 13},  /* 13 */ {21, 15, 16, 22, 18, 18}, SIZE_MEDIUM}, /* avg 18.33 */
};

/*
* Class table.
*/
const struct Class_type Class_table[MAX_CLASS] =
{
	{"mage", "Mag", STAT_INT, OBJ_VNUM_SCHOOL_STAFF, {3018, 9618}, 75, 20, 6, 4, 8, TRUE, "", ""},
	{"cleric", "Cle", STAT_WIS, OBJ_VNUM_SCHOOL_MACE, {3003, 9619}, 75, 20, 2, 6, 10, TRUE, "", ""},
	{"thief", "Thi", STAT_DEX, OBJ_VNUM_SCHOOL_DAGGER, {3028, 9639}, 60, 20, -4, 8, 14, FALSE, "", ""},
	{"warrior", "War", STAT_STR, OBJ_VNUM_SCHOOL_SWORD, {3022, 9633}, 60, 20, -10, 12, 19, FALSE, "", ""},
	{"ranger", "Ran", STAT_CON, OBJ_VNUM_SCHOOL_SWORD, {3022, 9633}, 60, 20, -10, 10, 16, TRUE, "", ""},
	{"bard", "Bar", STAT_CHA, OBJ_VNUM_SCHOOL_DAGGER, {3028, 9639}, 60, 20, -4, 6, 11, TRUE, "", ""},
	{"paladin", "Pal", STAT_CON, OBJ_VNUM_SCHOOL_SWORD, {3022, 9633}, 70, 20, -10, 9, 15, TRUE, "", ""},
	{"assassin", "Ass", STAT_DEX, OBJ_VNUM_SCHOOL_DAGGER, {3028, 9639}, 60, 20, -4, 8, 14, FALSE, "", ""},
	{"reaver", "Rea", STAT_STR, OBJ_VNUM_SCHOOL_SWORD, {3022, 9633}, 70, 20, -10, 9, 15, TRUE, "", ""},
	{"monk", "Mon", STAT_CON, OBJ_VNUM_SCHOOL_SWORD, {3022, 9633}, 60, 20, -4, 8, 14, FALSE, "", ""},
	{"necromancer", "Nec", STAT_INT, OBJ_VNUM_SCHOOL_STAFF, {3018, 9618}, 75, 20, 6, 4, 8, TRUE, "", ""},
	{"swarm", "Swa", STAT_INT, OBJ_VNUM_SCHOOL_STAFF, {3018, 9618}, 75, 20, 6, 4, 8, TRUE, "", ""},
	{"yanzelp", "Yan", STAT_INT, OBJ_VNUM_SCHOOL_STAFF, {3018, 9618}, 75, 20, 6, 4, 8, TRUE, "", ""},
	{"chaos", "Cha", STAT_INT, OBJ_VNUM_SCHOOL_STAFF, {3018, 9618}, 75, 20, 6, 4, 8, TRUE, "", ""},
	{"moxuiq", "Mox", STAT_INT, OBJ_VNUM_SCHOOL_STAFF, {3018, 9618}, 75, 20, 6, 6, 11, TRUE, "", ""},
	{"druid", "Dru", STAT_WIS, OBJ_VNUM_SCHOOL_STAFF, {3018, 9618}, 60, 20, -10, 6, 14, TRUE, "", ""},
	{"enchanter", "Enc", STAT_WIS, OBJ_VNUM_SCHOOL_STAFF, {3018, 9618}, 75, 20, 6, 4, 8, TRUE, "", ""},
};

/*
* Attribute bonus tables.
* Strength Table:
* {hitroll, damroll, weight capacity, number of items }
*/
const struct str_app_type str_app[26] =
{
	{-5, -4, 0, 0},		/* 0  */
	{-5, -4, 3, 1},		/* 1  */
	{-3, -2, 3, 2},
	{-3, -1, 10, 3},	/* 3  */
	{-2, -1, 25, 4},
	{-2, -1, 55, 5},	/* 5  */
	{-1, 0, 80, 6},
	{-1, 0, 90, 7},
	{0, 0, 100, 8},
	{0, 0, 100, 9},
	{0, 0, 115, 10},	/* 10  */
	{0, 0, 115, 11},
	{0, 0, 130, 12},
	{0, 0, 130, 13},	/* 13  */
	{0, 1, 140, 14},
	{1, 1, 150, 15},	/* 15  */
	{1, 2, 165, 16},
	{2, 3, 180, 22},
	{2, 3, 200, 25},	/* 18  */
	{3, 4, 225, 30},
	{3, 5, 250, 35},	/* 20  */
	{4, 6, 300, 40},
	{4, 6, 350, 45},
	{5, 7, 400, 50},
	{5, 8, 450, 55},
	{6, 9, 500, 60}		/* 25   */
};

const struct int_app_type int_app[26] =
{
	{3},	/*  0 */
	{5},	/*  1 */
	{7},
	{8},	/*  3 */
	{9},
	{10},	/*  5 */
	{11},
	{12},
	{13},
	{15},
	{17},	/* 10 */
	{19},
	{22},
	{25},
	{28},
	{31},	/* 15 */
	{34},
	{37},
	{40},	/* 18 */
	{44},
	{49},	/* 20 */
	{55},
	{60},
	{70},
	{80},
	{85}	/* 25 */
};
const struct wis_app_type wis_app[26] =
{
	{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},			/* 0..7		*/
	{1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1},	/* 8..16	*/
	{2}, {2}, {2}, {2},								/* 17..20	*/
	{3}, {3}, {3},									/* 21..23	*/ 
	{4}, {4}										/* 24..25	*/
};

const struct wis_app_type mage_gain_hp[26] =
{
	{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},			/* 0..7		*/
	{1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1},	/* 8..16	*/
	{2}, {2}, {2}, {2},								/* 17..20	*/
	{3}, {3}, {3}, {3},								/* 21..24	*/
	{4}												/* 25		*/
};

const struct wis_app_type prim_app[26] =
{
	{0}, {0}, {0},									/* 0..2		*/
	{1}, {1}, {1}, {1}, {1}, {1}, {1}, {1},			/* 3..10	*/
	{2}, {2}, {2}, {2}, {2}, {2}, {2},				/* 11..17	*/
	{3}, {3}, {3}, {3},								/* 18..21	*/
	{4}, {4}, {4}, {4}								/* 22..25	*/
};

/* Dexterity Table - Naked Armor Class */
const struct dex_app_type dex_app[26] = 
{
	{60},											/* 0		*/
	{50},				
	{50},
	{40},
	{30},
	{20},
	{10},
	{0}, {0}, {0}, {0},	{0}, {0}, {0}, {0},			/* 7..14	*/
	{-10},
	{-15},
	{-20},
	{-30},
	{-40},
	{-50},
	{-60},
	{-75},
	{-90},
	{-105},
	{-120}											/* 25		*/
};

const int hp_gains[MAX_CLASS][26][2] =
{
	{	/* Mage */
		{1, 1},	/*  0 */
		{1, 1},	/*  1 */
		{1, 2},	/*  2 */
		{1, 2},	/*  3 */
		{1, 2},	/*  4 */
		{1, 2},	/*  5 */
		{1, 3},	/*  6 */
		{2, 3},	/*  7 */
		{2, 3},	/*  8 */
		{2, 3},	/*  9 */
		{2, 3},	/* 10 */
		{3, 5},	/* 11 */
		{3, 5},	/* 12 */
		{3, 6},	/* 13 */
		{4, 6},	/* 14 */
		{4, 6},	/* 15 */
		{5, 7},	/* 16 */
		{5, 7},	/* 17 */
		{5, 8},	/* 18 */
		{6, 8},	/* 19 */
		{7, 9},	/* 20 */
		{7, 9},	/* 21 */
		{8, 10},	/* 22 */
		{8, 11},	/* 23 */
		{9, 11},	/* 24 */
		{9, 11}	/* 25 */
	},
	{	/* Cleric */
		{1, 1},	/*  0 */
		{1, 1},	/*  1 */
		{1, 2},	/*  2 */
		{1, 2},	/*  3 */
		{2, 2},	/*  4 */
		{2, 2},	/*  5 */
		{3, 3},	/*  6 */
		{3, 3},	/*  7 */
		{3, 3},	/*  8 */
		{3, 4},	/*  9 */
		{3, 4},	/* 10 */
		{4, 6},	/* 11 */
		{4, 6},	/* 12 */
		{5, 7},	/* 13 */
		{5, 7},	/* 14 */
		{6, 8},	/* 15 */
		{6, 8},	/* 16 */
		{6, 9},	/* 17 */
		{7, 9},	/* 18 */
		{8, 10},	/* 19 */
		{8, 10},	/* 20 */
		{9, 11},	/* 21 */
		{9, 11},	/* 22 */
		{10, 12},	/* 23 */
		{10, 12},	/* 24 */
		{10, 12}	/* 25 */
	},
	{	/* Thief */
		{1, 1},	/*  0 */
		{1, 2},	/*  1 */
		{2, 2},	/*  2 */
		{2, 2},	/*  3 */
		{2, 3},	/*  4 */
		{2, 3},	/*  5 */
		{3, 5},	/*  6 */
		{3, 6},	/*  7 */
		{3, 6},	/*  8 */
		{4, 7},	/*  9 */
		{5, 7},	/* 10 */
		{6, 9},	/* 11 */
		{6, 10},	/* 12 */
		{7, 10},	/* 13 */
		{8, 11},	/* 14 */
		{8, 11},	/* 15 */
		{9, 13},	/* 16 */
		{10, 13},	/* 17 */
		{11, 13},	/* 18 */
		{11, 14},	/* 19 */
		{12, 14},	/* 20 */
		{13, 15},	/* 21 */
		{14, 15},	/* 22 */
		{15, 15},	/* 23 */
		{15, 15},	/* 24 */
		{15, 15}	/* 25 */
	},
	{	/* Warrior */
		{1, 1},	/*  0 */
		{2, 3},	/*  1 */
		{2, 4},	/*  2 */
		{4, 4},	/*  3 */
		{4, 5},	/*  4 */
		{4, 6},	/*  5 */
		{5, 6},	/*  6 */
		{5, 7},	/*  7 */
		{7, 7},	/*  8 */
		{7, 8},	/*  9 */
		{7, 9},	/* 10 */
		{8, 10},	/* 11 */
		{9, 11},	/* 12 */
		{10, 12},	/* 13 */
		{10, 13},	/* 14 */
		{11, 14},	/* 15 */
		{12, 15},	/* 16 */
		{13, 16},	/* 17 */
		{14, 17},	/* 18 */
		{14, 17},	/* 19 */
		{15, 18},	/* 20 */
		{15, 18},	/* 21 */
		{16, 19},	/* 22 */
		{16, 19},	/* 23 */
		{17, 20},	/* 24 */
		{17, 20}	/* 25 */
	},
	{	/* Ranger */
		{1, 1},	/*  0 */
		{1, 1},	/*  1 */
		{1, 2},	/*  2 */
		{1, 3},	/*  3 */
		{3, 3},	/*  4 */
		{3, 4},	/*  5 */
		{4, 5},	/*  6 */
		{4, 6},	/*  7 */
		{4, 7},	/*  8 */
		{4, 8},	/*  9 */
		{5, 8},	/* 10 */
		{6, 9},	/* 11 */
		{6, 9},	/* 12 */
		{7, 9},	/* 13 */
		{8, 11},	/* 14 */
		{9, 12},	/* 15 */
		{10, 13},	/* 16 */
		{11, 14},	/* 17 */
		{12, 15},	/* 18 */
		{13, 15},	/* 19 */
		{14, 16},	/* 20 */
		{15, 17},	/* 21 */
		{15, 17},	/* 22 */
		{16, 17},	/* 23 */
		{16, 17},	/* 24 */
		{16, 17}	/* 25 */
	},
	{	/* Bard */
		{1,1},   /*  0 */
		{1,1},   /*  1 */
		{1,2},   /*  2 */
		{1,2},   /*  3 */
		{3,3},   /*  4 */
		{3,3},   /*  5 */
		{3,4},   /*  6 */
		{4,5},   /*  7 */
		{4,6},   /*  8 */
		{4,7},   /*  9 */
		{5,7},   /* 10 */
		{5,8},   /* 11 */
		{6,9},   /* 12 */
		{6,10},  /* 13 */
		{7,10},  /* 14 */
		{7,11},  /* 15 */
		{8,12},  /* 16 */
		{9,12},  /* 17 */
		{9,13},  /* 18 */
		{10,14}, /* 19 */
		{10,14}, /* 20 */
		{11,15}, /* 21 */
		{11,15}, /* 22 */
		{11,15}, /* 23 */
		{11,15}, /* 24 */
		{11,15}  /* 25 */
	},
	{	/* Paladin */
		{1, 2},	/*  0 */
		{1, 2},	/*  1 */
		{2, 3},	/*  2 */
		{3, 4},	/*  3 */
		{4, 4},	/*  4 */
		{4, 5},	/*  5 */
		{4, 6},	/*  6 */
		{4, 7},	/*  7 */
		{4, 7},	/*  8 */
		{5, 8},	/*  9 */
		{6, 8},	/* 10 */
		{7, 11},	/* 11 */
		{8, 11},	/* 12 */
		{9, 12},	/* 13 */
		{10, 12},	/* 14 */
		{10, 13},	/* 15 */
		{12, 14},	/* 16 */
		{12, 14},	/* 17 */
		{12, 14},	/* 18 */
		{13, 15},	/* 19 */
		{13, 16},	/* 20 */
		{14, 16},	/* 21 */
		{14, 17},	/* 22 */
		{14, 17},	/* 23 */
		{15, 17},	/* 24 */
		{15, 17}	/* 25 */
	},
	{	/* Assassin */
		{1, 1},	/*  0 */
		{1, 2},	/*  1 */
		{2, 2},	/*  2 */
		{2, 2},	/*  3 */
		{2, 3},	/*  4 */
		{2, 3},	/*  5 */
		{3, 5},	/*  6 */
		{3, 6},	/*  7 */
		{3, 6},	/*  8 */
		{4, 7},	/*  9 */
		{5, 7},	/* 10 */
		{6, 9},	/* 11 */
		{6, 10},	/* 12 */
		{7, 10},	/* 13 */
		{8, 11},	/* 14 */
		{8, 11},	/* 15 */
		{9, 13},	/* 16 */
		{10, 13},	/* 17 */
		{11, 13},	/* 18 */
		{11, 14},	/* 19 */
		{12, 14},	/* 20 */
		{13, 15},	/* 21 */
		{14, 15},	/* 22 */
		{15, 15},	/* 23 */
		{15, 15},	/* 24 */
		{15, 15}	/* 25 */
	}, 
	{	/* Reaver */
		{1, 2},	/*  0 */
		{1, 2},	/*  1 */
		{2, 3},	/*  2 */
		{3, 4},	/*  3 */
		{4, 4},	/*  4 */
		{4, 5},	/*  5 */
		{4, 6},	/*  6 */
		{4, 7},	/*  7 */
		{4, 7},	/*  8 */
		{5, 8},	/*  9 */
		{6, 8},	/* 10 */
		{7, 11},	/* 11 */
		{8, 11},	/* 12 */
		{9, 12},	/* 13 */
		{10, 12},	/* 14 */
		{10, 13},	/* 15 */
		{12, 14},	/* 16 */
		{12, 14},	/* 17 */
		{12, 14},	/* 18 */
		{13, 15},	/* 19 */
		{13, 16},	/* 20 */
		{14, 16},	/* 21 */
		{14, 17},	/* 22 */
		{14, 17},	/* 23 */
		{15, 17},	/* 24 */
		{15, 17}	/* 25 */
	},
	{	/* Monk */
		{1, 1},	/*  0 */
		{1, 2},	/*  1 */
		{2, 2},	/*  2 */
		{2, 2},	/*  3 */
		{2, 3},	/*  4 */
		{2, 3},	/*  5 */
		{3, 5},	/*  6 */
		{3, 6},	/*  7 */
		{3, 6},	/*  8 */
		{4, 7},	/*  9 */
		{5, 7},	/* 10 */
		{6, 9},	/* 11 */
		{6, 10},	/* 12 */
		{7, 10},	/* 13 */
		{8, 11},	/* 14 */
		{8, 11},	/* 15 */
		{9, 13},	/* 16 */
		{10, 13},	/* 17 */
		{11, 13},	/* 18 */
		{11, 14},	/* 19 */
		{12, 14},	/* 20 */
		{13, 15},	/* 21 */
		{14, 15},	/* 22 */
		{15, 15},	/* 23 */
		{15, 15},	/* 24 */
		{15, 15}	/* 25 */
	},
	{      /* Necromancer */
		{1, 1},      /*  0 */
		{1, 1},      /*  1 */
		{1, 2},      /*  2 */
		{1, 2},      /*  3 */
		{1, 2},      /*  4 */
		{1, 2},      /*  5 */
		{1, 3},      /*  6 */
		{2, 3},      /*  7 */
		{2, 3},      /*  8 */
		{2, 3},      /*  9 */
		{2, 3},      /* 10 */
		{3, 5},      /* 11 */
		{3, 5},      /* 12 */
		{3, 6},      /* 13 */
		{4, 6},      /* 14 */
		{4, 6},      /* 15 */
		{5, 7},      /* 16 */
		{5, 7},      /* 17 */
		{5, 8},      /* 18 */
		{6, 8},      /* 19 */
		{7, 9},      /* 20 */
		{7, 9},      /* 21 */
		{8, 10},     /* 22 */
		{8, 11},     /* 23 */
		{9, 11},     /* 24 */
		{9, 11}      /* 25 */
	},
	{      /* Necromancer */
		{1, 1},      /*  0 */
		{1, 1},      /*  1 */
		{1, 2},      /*  2 */
		{1, 2},      /*  3 */
		{1, 2},      /*  4 */
		{1, 2},      /*  5 */
		{1, 3},      /*  6 */
		{2, 3},      /*  7 */
		{2, 3},      /*  8 */
		{2, 3},      /*  9 */
		{2, 3},      /* 10 */
		{3, 5},      /* 11 */
		{3, 5},      /* 12 */
		{3, 6},      /* 13 */
		{4, 6},      /* 14 */
		{4, 6},      /* 15 */
		{5, 7},      /* 16 */
		{5, 7},      /* 17 */
		{5, 8},      /* 18 */
		{6, 8},      /* 19 */
		{7, 9},      /* 20 */
		{7, 9},      /* 21 */
		{8, 10},     /* 22 */
		{8, 11},     /* 23 */
		{9, 11},     /* 24 */
		{9, 11}      /* 25 */
	},
	{      /* Necromancer */
		{1, 1},      /*  0 */
		{1, 1},      /*  1 */
		{1, 2},      /*  2 */
		{1, 2},      /*  3 */
		{1, 2},      /*  4 */
		{1, 2},      /*  5 */
		{1, 3},      /*  6 */
		{2, 3},      /*  7 */
		{2, 3},      /*  8 */
		{2, 3},      /*  9 */
		{2, 3},      /* 10 */
		{3, 5},      /* 11 */
		{3, 5},      /* 12 */
		{3, 6},      /* 13 */
		{4, 6},      /* 14 */
		{4, 6},      /* 15 */
		{5, 7},      /* 16 */
		{5, 7},      /* 17 */
		{5, 8},      /* 18 */
		{6, 8},      /* 19 */
		{7, 9},      /* 20 */
		{7, 9},      /* 21 */
		{8, 10},     /* 22 */
		{8, 11},     /* 23 */
		{9, 11},     /* 24 */
		{9, 11}      /* 25 */
	},
	{ /* Bard */
		{1,1},   /*  0 */
		{1,1},   /*  1 */
		{1,2},   /*  2 */
		{1,2},   /*  3 */
		{3,3},   /*  4 */
		{3,3},   /*  5 */
		{3,4},   /*  6 */
		{4,5},   /*  7 */
		{4,6},   /*  8 */
		{4,7},   /*  9 */
		{5,7},   /* 10 */
		{5,8},   /* 11 */
		{6,9},   /* 12 */
		{6,10},  /* 13 */
		{7,10},  /* 14 */
		{7,11},  /* 15 */
		{8,12},  /* 16 */
		{9,12},  /* 17 */
		{9,13},  /* 18 */
		{10,14}, /* 19 */
		{10,14}, /* 20 */
		{11,15}, /* 21 */
		{11,15}, /* 22 */
		{11,15}, /* 23 */
		{11,15}, /* 24 */
		{11,15}  /* 25 */
	},
	{      /* Necromancer */
		{1, 1},      /*  0 */
		{1, 1},      /*  1 */
		{1, 2},      /*  2 */
		{1, 2},      /*  3 */
		{1, 2},      /*  4 */
		{1, 2},      /*  5 */
		{1, 3},      /*  6 */
		{2, 3},      /*  7 */
		{2, 3},      /*  8 */
		{2, 3},      /*  9 */
		{2, 3},      /* 10 */
		{3, 5},      /* 11 */
		{3, 5},      /* 12 */
		{3, 6},      /* 13 */
		{4, 6},      /* 14 */
		{4, 6},      /* 15 */
		{5, 7},      /* 16 */
		{5, 7},      /* 17 */
		{5, 8},      /* 18 */
		{6, 8},      /* 19 */
		{7, 9},      /* 20 */
		{7, 9},      /* 21 */
		{8, 10},     /* 22 */
		{8, 11},     /* 23 */
		{9, 11},     /* 24 */
		{9, 11}      /* 25 */
	},
    {      /* Druid */
        {1, 1},      /*  0 */
        {1, 1},      /*  1 */
        {1, 2},      /*  2 */
        {1, 2},      /*  3 */
        {1, 2},      /*  4 */
        {1, 2},      /*  5 */
        {1, 3},      /*  6 */
        {2, 3},      /*  7 */
        {2, 3},      /*  8 */
        {2, 3},      /*  9 */
        {2, 3},      /* 10 */
        {3, 5},      /* 11 */
        {3, 5},      /* 12 */
        {4, 7},      /* 13 */
        {5, 7},      /* 14 */
        {5, 7},      /* 15 */
        {5, 8},      /* 16 */
        {8, 9},      /* 17 */
        {9, 10},      /* 18 */
        {9, 10},     /* 19 */
        {10, 12},     /* 20 */
        {10, 12},     /* 21 */
        {10, 12},     /* 22 */
        {19, 12},     /* 23 */
        {10, 12},     /* 24 */
        {10, 12}      /* 25 */
    },
	
	{      /* Enchanter */
        {1, 1},      /*  0 */
        {1, 1},      /*  1 */
        {1, 2},      /*  2 */
        {1, 2},      /*  3 */
        {1, 2},      /*  4 */
        {1, 2},      /*  5 */
        {1, 3},      /*  6 */
        {2, 3},      /*  7 */
        {2, 3},      /*  8 */
        {2, 3},      /*  9 */
        {2, 3},      /* 10 */
        {3, 5},      /* 11 */
        {3, 5},      /* 12 */
        {4, 7},      /* 13 */
        {5, 7},      /* 14 */
        {5, 7},      /* 15 */
        {5, 8},      /* 16 */
        {8, 9},      /* 17 */
        {9, 10},      /* 18 */
        {9, 10},     /* 19 */
        {10, 12},     /* 20 */
        {10, 12},     /* 21 */
        {10, 12},     /* 22 */
        {19, 12},     /* 23 */
        {10, 12},     /* 24 */
        {10, 12}      /* 25 */
    }
};

const int mana_gains[MAX_CLASS][26][2] =
{
	{	/* Mage */
		{1, 1},	/*  0 */
		{1, 1},	/*  1 */
		{1, 1},	/*  2 */
		{1, 1},	/*  3 */
		{1, 1},	/*  4 */
		{1, 1},	/*  5 */
		{1, 2},	/*  6 */
		{1, 2},	/*  7 */
		{1, 3},	/*  8 */
		{2, 5},	/*  9 */
		{3, 5},	/* 10 */
		{3, 6},	/* 11 */
		{3, 7},	/* 12 */
		{4, 8},	/* 13 */
		{5, 9},	/* 14 */
		{6, 10},	/* 15 */
		{7, 11},	/* 16 */
		{9, 13},	/* 17 */
		{10, 14},	/* 18 */
		{12, 16},	/* 19 */
		{14, 17},	/* 20 */
		{15, 18},	/* 21 */
		{15, 18},	/* 22 */
		{15, 18},	/* 23 */
		{16, 19},	/* 24 */
		{16, 19}	/* 25 */
	}, 
	{	/* Cleric */
		{1, 1},	/*  0 */
		{1, 1},	/*  1 */
		{1, 2},	/*  2 */
		{1, 2},	/*  3 */
		{1, 2},	/*  4 */
		{1, 2},	/*  5 */
		{2, 3},	/*  6 */
		{2, 3},	/*  7 */
		{2, 4},	/*  8 */
		{3, 4},	/*  9 */
		{3, 4},	/* 10 */
		{3, 5},	/* 11 */
		{4, 6},	/* 12 */
		{5, 8},	/* 13 */
		{5, 9},	/* 14 */
		{6, 9},	/* 15 */
		{6, 10},	/* 16 */
		{7, 11},	/* 17 */
		{8, 12},	/* 18 */
		{9, 14},	/* 19 */
		{12, 14},	/* 20 */
		{13, 15},	/* 21 */
		{13, 16},	/* 22 */
		{14, 16},	/* 23 */
		{17, 17},	/* 24 */
		{17, 17}	/* 25 */
	},
	{	/* Thief */
		{0, 1},	/*  0 */
		{0, 1},	/*  1 */
		{0, 1},	/*  2 */
		{0, 1},	/*  3 */
		{0, 1},	/*  4 */
		{0, 1},	/*  5 */
		{0, 1},	/*  6 */
		{0, 1},	/*  7 */
		{1, 1},	/*  8 */
		{1, 1},	/*  9 */
		{1, 2},	/* 10 */
		{1, 2},	/* 11 */
		{1, 2},	/* 12 */
		{1, 2},	/* 13 */
		{1, 3},	/* 14 */
		{1, 3},	/* 15 */
		{1, 3},	/* 16 */
		{1, 3},	/* 17 */
		{2, 4},	/* 18 */
		{2, 4},	/* 19 */
		{2, 5},	/* 20 */
		{3, 5},	/* 21 */
		{3, 5},	/* 22 */
		{3, 6},	/* 23 */
		{3, 6},	/* 24 */
		{4, 6}	/* 25 */
	},
	{	/* Warrior */
		{0, 1},	/*  0 */
		{0, 1},	/*  1 */
		{0, 1},	/*  2 */
		{0, 1},	/*  3 */
		{0, 1},	/*  4 */
		{0, 1},	/*  5 */
		{0, 1},	/*  6 */
		{0, 1},	/*  7 */
		{0, 1},	/*  8 */
		{0, 1},	/*  9 */
		{0, 1},	/* 10 */
		{0, 1},	/* 11 */
		{0, 1},	/* 12 */
		{1, 1},	/* 13 */
		{1, 1},	/* 14 */
		{1, 2},	/* 15 */
		{1, 2},	/* 16 */
		{1, 2},	/* 17 */
		{1, 3},	/* 18 */
		{1, 3},	/* 19 */
		{1, 3},	/* 20 */
		{1, 3},	/* 21 */
		{1, 3},	/* 22 */
		{1, 3},	/* 23 */
		{1, 4},	/* 24 */
		{1, 4}	/* 25 */
	},
	{	/* Ranger */
		{1, 1},	/*  0 */
		{1, 1},	/*  1 */
		{1, 1},	/*  2 */
		{1, 1},	/*  3 */
		{1, 1},	/*  4 */
		{1, 1},	/*  5 */
		{1, 1},	/*  6 */
		{1, 1},	/*  7 */
		{1, 1},	/*  8 */
		{1, 1},	/*  9 */
		{1, 1},	/* 10 */
		{1, 1},	/* 11 */
		{1, 1},	/* 12 */
		{1, 2},	/* 13 */
		{1, 2},	/* 14 */
		{1, 2},	/* 15 */
		{2, 3},	/* 16 */
		{2, 4},	/* 17 */
		{2, 4},	/* 18 */
		{3, 5},	/* 19 */
		{3, 5},	/* 20 */
		{3, 5},	/* 21 */
		{3, 6},	/* 22 */
		{4, 6},	/* 23 */
		{4, 6},	/* 24 */
		{4, 6}	/* 25 */
	},
	{	/* Bard */
		{1,1},   /*  0 */
		{1,1},	 /*  1 */
		{1,1},   /*  2 */
		{1,1},	 /*  3 */
		{1,1},	 /*  4 */
		{1,1},	 /*  5 */
		{1,1},	 /*  6 */
		{1,2},	 /*  7 */
		{1,2},	 /*  8 */
		{1,2},	 /*  9 */
		{2,3},	 /* 10 */
		{2,3},   /* 11 */
		{3,4},	 /* 12 */
		{3,5},   /* 13 */
		{4,6},	 /* 14 */
		{4,7},	 /* 15 */
		{4,7},	 /* 16 */
		{5,8},	 /* 17 */
		{5,8},	 /* 18 */
		{6,9},	 /* 19 */
		{6,9},   /* 20 */
		{7,10},	 /* 21 */
		{8,11},  /* 22 */
		{8,11},	 /* 23 */
		{9,11},	 /* 24 */
		{10,11}  /* 25 */
	},
	{	/* Paladin */
		{1, 1},	/*  0 */
		{1, 1},	/*  1 */
		{1, 1},	/*  2 */
		{1, 2},	/*  3 */
		{1, 2},	/*  4 */
		{1, 2},	/*  5 */
		{1, 2},	/*  6 */
		{1, 2},	/*  7 */
		{1, 2},	/*  8 */
		{1, 2},	/*  9 */
		{2, 3},	/* 10 */
		{2, 3},	/* 11 */
		{2, 3},	/* 12 */
		{2, 4},	/* 13 */
		{3, 4},	/* 14 */
		{3, 5},	/* 15 */
		{3, 5},	/* 16 */
		{3, 6},	/* 17 */
		{4, 6},	/* 18 */
		{4, 7},	/* 19 */
		{5, 7},	/* 20 */
		{5, 7},	/* 21 */
		{6, 8},	/* 22 */
		{6, 8},	/* 23 */
		{7, 10},	/* 24 */
		{7, 10}	/* 25 */
	},
	{	/* Assassin */
		{0, 1},	/*  0 */
		{0, 1},	/*  1 */
		{0, 1},	/*  2 */
		{0, 1},	/*  3 */
		{0, 1},	/*  4 */
		{0, 1},	/*  5 */
		{0, 1},	/*  6 */
		{0, 1},	/*  7 */
		{1, 1},	/*  8 */
		{1, 1},	/*  9 */
		{1, 2},	/* 10 */
		{1, 2},	/* 11 */
		{1, 2},	/* 12 */
		{1, 2},	/* 13 */
		{1, 3},	/* 14 */
		{1, 3},	/* 15 */
		{1, 3},	/* 16 */
		{1, 3},	/* 17 */
		{2, 4},	/* 18 */
		{2, 4},	/* 19 */
		{2, 5},	/* 20 */
		{3, 5},	/* 21 */
		{3, 5},	/* 22 */
		{3, 6},	/* 23 */
		{3, 6},	/* 24 */
		{4, 6}	/* 25 */
	}, 
	{	/* Reaver */
		{1, 1},	/*  0 */
		{1, 1},	/*  1 */
		{1, 1},	/*  2 */
		{1, 2},	/*  3 */
		{1, 2},	/*  4 */
		{1, 2},	/*  5 */
		{1, 2},	/*  6 */
		{1, 2},	/*  7 */
		{1, 2},	/*  8 */
		{1, 2},	/*  9 */
		{2, 3},	/* 10 */
		{2, 3},	/* 11 */
		{2, 3},	/* 12 */
		{2, 4},	/* 13 */
		{3, 4},	/* 14 */
		{3, 5},	/* 15 */
		{3, 5},	/* 16 */
		{3, 6},	/* 17 */
		{4, 6},	/* 18 */
		{4, 7},	/* 19 */
		{5, 7},	/* 20 */
		{5, 7},	/* 21 */
		{6, 8},	/* 22 */
		{6, 8},	/* 23 */
		{7, 10},	/* 24 */
		{7, 10}	/* 25 */
	},
	{	/* Monk */
		{0, 1},	/*  0 */
		{0, 1},	/*  1 */
		{0, 1},	/*  2 */
		{0, 1},	/*  3 */
		{0, 1},	/*  4 */
		{0, 1},	/*  5 */
		{0, 1},	/*  6 */
		{0, 1},	/*  7 */
		{1, 1},	/*  8 */
		{1, 1},	/*  9 */
		{1, 2},	/* 10 */
		{1, 2},	/* 11 */
		{1, 2},	/* 12 */
		{1, 2},	/* 13 */
		{1, 3},	/* 14 */
		{1, 3},	/* 15 */
		{1, 3},	/* 16 */
		{1, 3},	/* 17 */
		{2, 4},	/* 18 */
		{2, 4},	/* 19 */
		{2, 5},	/* 20 */
		{3, 5},	/* 21 */
		{3, 5},	/* 22 */
		{3, 6},	/* 23 */
		{3, 6},	/* 24 */
		{4, 6}	/* 25 */
	},
	{   /* Necromancer */
		{1, 1},    /*  0 */
		{1, 1},    /*  1 */
		{1, 1},    /*  2 */
		{1, 1},    /*  3 */
		{1, 1},    /*  4 */
		{1, 1},    /*  5 */
		{1, 2},    /*  6 */
		{1, 2},    /*  7 */
		{1, 3},    /*  8 */
		{2, 5},    /*  9 */
		{3, 5},    /* 10 */
		{3, 6},    /* 11 */
		{3, 7},    /* 12 */
		{4, 8},    /* 13 */
		{5, 9},    /* 14 */
		{6, 10},   /* 15 */
		{7, 11},   /* 16 */
		{9, 13},   /* 17 */
		{10, 14},  /* 18 */
		{12, 16},  /* 19 */
		{14, 17},  /* 20 */
		{15, 18},  /* 21 */
		{15, 18},  /* 22 */
		{15, 18},  /* 23 */
		{16, 19},  /* 24 */
		{16, 19}   /* 25 */
	},
	{   /* Necromancer */
		{1, 1},    /*  0 */
		{1, 1},    /*  1 */
		{1, 1},    /*  2 */
		{1, 1},    /*  3 */
		{1, 1},    /*  4 */
		{1, 1},    /*  5 */
		{1, 2},    /*  6 */
		{1, 2},    /*  7 */
		{1, 3},    /*  8 */
		{2, 5},    /*  9 */
		{3, 5},    /* 10 */
		{3, 6},    /* 11 */
		{3, 7},    /* 12 */
		{4, 8},    /* 13 */
		{5, 9},    /* 14 */
		{6, 10},   /* 15 */
		{7, 11},   /* 16 */
		{9, 13},   /* 17 */
		{10, 14},  /* 18 */
		{12, 16},  /* 19 */
		{14, 17},  /* 20 */
		{15, 18},  /* 21 */
		{15, 18},  /* 22 */
		{15, 18},  /* 23 */
		{16, 19},  /* 24 */
		{16, 19}   /* 25 */
	},
	{   /* Necromancer */
		{1, 1},    /*  0 */
		{1, 1},    /*  1 */
		{1, 1},    /*  2 */
		{1, 1},    /*  3 */
		{1, 1},    /*  4 */
		{1, 1},    /*  5 */
		{1, 2},    /*  6 */
		{1, 2},    /*  7 */
		{1, 3},    /*  8 */
		{2, 5},    /*  9 */
		{3, 5},    /* 10 */
		{3, 6},    /* 11 */
		{3, 7},    /* 12 */
		{4, 8},    /* 13 */
		{5, 9},    /* 14 */
		{6, 10},   /* 15 */
		{7, 11},   /* 16 */
		{9, 13},   /* 17 */
		{10, 14},  /* 18 */
		{12, 16},  /* 19 */
		{14, 17},  /* 20 */
		{15, 18},  /* 21 */
		{15, 18},  /* 22 */
		{15, 18},  /* 23 */
		{16, 19},  /* 24 */
		{16, 19}   /* 25 */
	},
	{ /* Bard */
		{1,1},   /*  0 */
		{1,1},   /*  1 */
		{1,1},   /*  2 */
		{1,1},   /*  3 */
		{1,1},   /*  4 */
		{1,1},   /*  5 */
		{1,1},   /*  6 */
		{1,2},   /*  7 */
		{1,2},   /*  8 */
		{1,2},   /*  9 */
		{2,3},   /* 10 */
		{2,3},   /* 11 */
		{3,4},   /* 12 */
		{3,5},   /* 13 */
		{4,6},   /* 14 */
		{4,7},   /* 15 */
		{4,7},   /* 16 */
		{5,8},   /* 17 */
		{5,8},   /* 18 */
		{6,9},   /* 19 */
		{6,9},   /* 20 */
		{7,10},  /* 21 */
		{8,11},  /* 22 */
		{8,11},  /* 23 */
		{9,11},  /* 24 */
		{10,11}  /* 25 */
	},
	{   /* Necromancer */
		{1, 1},    /*  0 */
		{1, 1},    /*  1 */
		{1, 1},    /*  2 */
		{1, 1},    /*  3 */
		{1, 1},    /*  4 */
		{1, 1},    /*  5 */
		{1, 2},    /*  6 */
		{1, 2},    /*  7 */
		{1, 3},    /*  8 */
		{2, 5},    /*  9 */
		{3, 5},    /* 10 */
		{3, 6},    /* 11 */
		{3, 7},    /* 12 */
		{4, 8},    /* 13 */
		{5, 9},    /* 14 */
		{6, 10},   /* 15 */
		{7, 11},   /* 16 */
		{9, 13},   /* 17 */
		{10, 14},  /* 18 */
		{12, 16},  /* 19 */
		{14, 17},  /* 20 */
		{15, 18},  /* 21 */
		{15, 18},  /* 22 */
		{15, 18},  /* 23 */
		{16, 19},  /* 24 */
		{16, 19}   /* 25 */
	},
    {   /* Druid */
        {1, 1},     /*  0 */
        {1, 1},     /*  1 */
        {1, 1},     /*  2 */
        {1, 1},     /*  3 */
        {1, 1},     /*  4 */
        {1, 1},     /*  5 */
        {1, 2},     /*  6 */
        {1, 2},     /*  7 */
        {1, 3},     /*  8 */
        {2, 5},     /*  9 */
        {3, 5},     /* 10 */
        {3, 6},     /* 11 */
        {3, 7},     /* 12 */
        {4, 8},     /* 13 */
        {5, 9},     /* 14 */
        {9, 12},    /* 15 */
        {9, 12},    /* 16 */
        {10, 13},    /* 17 */
        {10, 14},   /* 18 */
        {12, 16},   /* 19 */
        {14, 17},   /* 20 */
        {15, 18},   /* 21 */
        {15, 18},   /* 22 */
        {15, 18},   /* 23 */
        {16, 19},   /* 24 */
        {16, 19}    /* 25 */
    },

	{	/* Enchanter */
		{1, 1},	/*  0 */
		{1, 1},	/*  1 */
		{1, 1},	/*  2 */
		{1, 1},	/*  3 */
		{1, 1},	/*  4 */
		{1, 1},	/*  5 */
		{1, 2},	/*  6 */
		{1, 2},	/*  7 */
		{1, 3},	/*  8 */
		{2, 5},	/*  9 */
		{3, 5},	/* 10 */
		{3, 6},	/* 11 */
		{3, 7},	/* 12 */
		{4, 8},	/* 13 */
		{5, 9},	/* 14 */
		{6, 10},	/* 15 */
		{7, 11},	/* 16 */
		{9, 13},	/* 17 */
		{10, 14},	/* 18 */
		{12, 16},	/* 19 */
		{14, 17},	/* 20 */
		{15, 18},	/* 21 */
		{15, 18},	/* 22 */
		{15, 18},	/* 23 */
		{16, 19},	/* 24 */
		{16, 19}	/* 25 */
	}
};

/*
* Liquid properties.
* Used in world.obj.
*/
const struct liq_type liq_table[] =
{
	/*    name, color, proof, full, thirst, food, ssize */
	{"water", "clear", {0, 1, 10, 0, 16}},
	{"beer", "amber", {12, 1, 8, 10, 12}},
	{"red wine", "burgundy", {30, 1, 8, 10, 5}},
	{"ale", "brown", {15, 1, 8, 1, 12}},
	{"dark ale", "dark", {16, 50, 50, 50, 12}},
	{"whisky", "golden", {120, 1, 5, 10, 2}},
	{"lemonade", "pink", {0, 1, 9, 0, 12}},
	{"firebreather", "boiling", {190, 0, 4, 0, 2}},
	{"local specialty", "clear", {151, 1, 3, 0, 2}},
	{"slime mold juice", "green", {0, 2, -8, 1, 2}},
	{"milk", "white", {0, 2, 9, 0, 12}},
	{"tea", "tan", {0, 1, 8, 0, 6}},
	{"coffee", "black", {0, 1, 8, 0, 6}},
	{"blood", "red", {0, 2, -1, 0, 6}},
	{"salt water", "clear", {0, 1, -2, 0, 1}},
	{"coke", "brown", {0, 2, 9, 0, 12}},
	{"root beer", "brown", {0, 2, 9, 0, 12}},
	{"elvish wine", "green", {35, 2, 8, 0, 5}},
	{"white wine", "golden", {28, 1, 8, 0, 5}},
	{"champagne", "golden", {32, 1, 8, 0, 5}},
	{"mead", "honey-colored", {34, 2, 8, 0, 12}},
	{"rose wine", "pink", {26, 1, 8, 0, 5}},
	{"benedictine wine", "burgundy", {40, 1, 8, 0, 5}},
	{"vodka", "clear", {130, 1, 5, 0, 2}},
	{"cranberry juice", "red", {0, 1, 9, 0, 12}},
	{"orange juice", "orange", {0, 2, 9, 0, 12}},
	{"absinthe", "green", {200, 1, 4, 0, 2}},
	{"brandy", "golden", {80, 1, 5, 0, 4}},
	{"aquavit", "clear", {140, 1, 5, 0, 2}},
	{"schnapps", "clear", {90, 1, 5, 0, 2}},
	{"icewine", "purple", {50, 2, 6, 0, 5}},
	{"amontillado", "burgundy", {35, 2, 8, 0, 5}},
	{"sherry", "red", {38, 2, 7, 0, 5}},
	{"framboise", "red", {50, 1, 7, 0, 5}},
	{"rum", "amber", {151, 1, 4, 0, 2}},
	{"cordial", "clear", {100, 1, 5, 0, 2}},
	{NULL, NULL, {0, 0, 0, 0, 0}}
};

/*
* The skill and spell table.
* Slot numbers must never be changed as they appear in #OBJECTS sections.
*/
#define SLOT(n)	n


/* Classes:  
 * { mage, cleric, thief, warrior, 
 *   ranger, bard, paladin, assassin, 
 *   reaver, monk, necro, swarm, golem, 
 *   chaos jesters, adventurers, druid,
*    enchanter} */
const struct cmdskill_type command_table[] = {
	{SKILL_CAST, {-1, 91, 91, 91, -1, 91, 91, 91, 91, 91, 91, 91, 91, -1, 91, -1, -1}},
	{SKILL_SING, {91, 91, 91, 91, 91, -1, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}},
	{SKILL_PRAY, {91, -1, 91, 91, 91, 91, -1, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}},
	{SKILL_CHANT, {91, 91, 91, 91, 91, 91, 91, 91, -1, 91, -1, 91, 91, 91, 91, 91, 91}},
	{0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
};

const struct skill_type skill_table[MAX_SKILL] = {
	{SKILL_NORMAL, "reserved", {99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, 
        {99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}, 0, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(0), 0, 0, "", "", ""},
    /* SPELLS - MAGE CLASS */
	{SKILL_CAST, "armor", {7, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_armor, TAR_CHAR_DEFENSIVE, POS_STANDING, 
        NULL, SLOT(1), 5, 12, "", "You feel less armored.", ""},
	{SKILL_CAST, "acid blast",{50, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_acid_blast, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(70), 20, 12, "acid blast", "!Acid Blast!"},
	{SKILL_CAST, "fireshield", {35, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_fireshield, TAR_CHAR_SELF,POS_STANDING, 
        &gsn_fireshield, SLOT(101),40, 12, "","The scorching shield of fire around you burns away.",""},        
	{SKILL_CAST, "blindness",{12, 91, 91, 91, 91, 91,91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_blindness, TAR_CHAR_OFFENSIVE,POS_FIGHTING, 
        &gsn_blindness, SLOT(4),16, 12, "",	"You can see again.", ""},
	{SKILL_CAST, "web", {55, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_web, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(104), 60, 12, "web","The magical webs holding you dissolve and fall away.", ""},
	{SKILL_CAST, "burning hands", {7, 91, 91, 91,91, 91, 91, 91,91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_burning_hands, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT (5), 10, 12, "burning hands", "!Burning Hands!", ""},
	{SKILL_CAST, "bee sting", {91, 91, 91, 91, 12, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_bee_sting, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(87), 10, 12, "bee sting", "!Bee Sting!", ""},
	{SKILL_CAST, "bee swarm", {91, 91, 91, 91, 45, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_bee_swarm, TAR_AREA_OFF, POS_FIGHTING, 
        NULL, SLOT(88), 50, 12,"swarm of bees", "!Bee Swarm!", ""},
	{SKILL_CAST, "call lightning",{26, 91, 91, 91, 53, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_call_lightning, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(6), 15, 12, "lightning bolt","!Call Lightning!", ""},
	{SKILL_CAST, "cancellation", {30, 91, 91, 91,91, 91, 91, 91,91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_cancellation, TAR_CHAR_DEFENSIVE, POS_FIGHTING, 
        NULL, SLOT(507), 20, 12, "", "!cancellation!", ""},
	{SKILL_PRAY, "purification", {91, 30, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_purification, TAR_CHAR_DEFENSIVE, POS_FIGHTING, 
        NULL, SLOT(705), 20, 12, "", "!purification!",""},
	{SKILL_PRAY, "halo of seraphs",	{91, 43, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_halos, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(539), 30, 12, "", "You no longer feel as enchanted.", ""},
	{SKILL_PRAY, "illumination", {91, 44, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_illumination, TAR_CHAR_DEFENSIVE, POS_STANDING, 
        NULL, SLOT(540), 30, 12, "",	"The seed of divine wisdom slips from your grasp.", ""},
	{SKILL_PRAY, "cause critical",{91, 13, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_cause_critical, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(63), 10, 12, "spell", "!Cause Critical!", ""},
	{SKILL_PRAY, "cause light", {91, 1, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_cause_light, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(62), 5, 12, "spell",	"!Cause Light!", ""},
	{SKILL_PRAY, "cause serious", {91, 7, 91, 91, 91, 91, 17, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_cause_serious, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(64),	8, 12, "spell", "!Cause Serious!", ""},
	{SKILL_CAST, "chain lightning",	{46, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_chain_lightning, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL,SLOT(500), 25, 12, "lightning","!Chain Lightning!", ""},
	{SKILL_CAST, "change sex", {25, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_change_sex, TAR_CHAR_DEFENSIVE,	POS_FIGHTING, 
        NULL, SLOT(82), 15, 12, "",	"Your body feels familiar again.", ""},
	{SKILL_CAST, "charm person", {20, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_charm_person, TAR_CHAR_OFFENSIVE, POS_STANDING, 
        &gsn_charm_person, SLOT(7),	20, 12, "",	"You feel more self-confident.", ""},
	{SKILL_CAST, "chill touch", {4, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_chill_touch, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(8), 10, 12, "chilling touch", "You feel less cold.", "The ice encrusting your blade melts away."},
	{SKILL_CAST, "colour spray", {16, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_colour_spray, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, 
        SLOT(10), 10, 12, "colour spray", "!Colour Spray!", ""},
	{SKILL_PRAY, "continual light",	{91, 4, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_continual_light, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(57), 7, 12, "", "!Continual Light!", ""},
	{SKILL_CAST, "control weather",	{15, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_control_weather, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(11), 25, 12, "", "!Control Weather!", ""},
	{SKILL_CAST, "create food", {10, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_create_food, TAR_IGNORE, POS_STANDING, NULL, 
        SLOT(12), 5, 12,"", "!Create Food!", ""},
	{SKILL_CAST, "create rose", {16, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_create_rose, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(511), 30, 12, "", "!Create Rose!",""},
	{SKILL_CAST, "create spring", {14, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 10, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_create_spring, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(80), 20, 12,	"", "!Create Spring!", ""},
	{SKILL_CAST, "create water", {8, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_create_water, TAR_OBJ_INV, POS_STANDING, 
        NULL, SLOT(13), 5, 12,	"", "!Create Water!", ""},
	{SKILL_PRAY, "cure blindness", {91, 6, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_cure_blindness, TAR_CHAR_DEFENSIVE, POS_FIGHTING, 
        NULL,SLOT(14), 10, 12, "", "!Cure Blindness!", ""},
	{SKILL_PRAY, "heroes feast", {91, 52, 91, 91, 91, 91, 58, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_heroes_feast, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(98), 40, 12,	"", "!Heroes Feast!", ""},
	{SKILL_PRAY, "cure critical", {91, 13, 91, 91, 91, 91, 30, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_cure_critical, TAR_CHAR_DEFENSIVE, POS_FIGHTING, 
        NULL, SLOT(15),	20, 12, "cure critical", "!Cure Critical!", ""},
	{SKILL_CAST, "herbal lore", {91, 91, 91, 91, 13, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_herb_lore, TAR_CHAR_DEFENSIVE, POS_FIGHTING, 
        NULL, SLOT(88), 15, 12, "", "!Herbal Lore!", ""},
	{SKILL_PRAY, "cure disease", {91, 13, 91, 91, 91, 91, 36, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_cure_disease, TAR_CHAR_DEFENSIVE, POS_STANDING, 
        NULL, SLOT(501), 20, 12, "", "!Cure Disease!", ""},
	{SKILL_PRAY, "cure light", {91, 1, 91, 91, 91, 91, 8, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_cure_light, TAR_CHAR_DEFENSIVE, POS_FIGHTING, 
        NULL, SLOT(16), 10, 12, "cure light", "!Cure Light!", ""},
	{SKILL_PRAY, "cure poison", {91, 14, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_cure_poison, TAR_CHAR_DEFENSIVE, POS_STANDING, 
        NULL, SLOT(43),	5, 12, "", "!Cure Poison!", ""},
	{SKILL_PRAY, "cure serious", {91, 7, 91, 91, 91, 91, 24, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_cure_serious, TAR_CHAR_DEFENSIVE, POS_FIGHTING, 
        NULL, SLOT(61), 15, 12, "cure serious", "!Cure Serious!", ""},
	{SKILL_CAST, "curse", {18, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_curse, TAR_OBJ_CHAR_OFF, POS_FIGHTING, 
        &gsn_curse, SLOT(17), 20, 12, "curse", "The curse wears off.", "$p is no longer impure."},
	{SKILL_PRAY, "demonfire", {91, 55, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},	
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_demonfire, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(505), 20, 12, "torments", "!Demonfire!", ""},
	{SKILL_PRAY, "detect evil", {91, 4, 91, 91, 91, 91, 17, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1,	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_detect_evil, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(18), 5, 12, "", "The red in your vision disappears.", ""},
	{SKILL_PRAY, "edification", {91, 32, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_edification, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(545), 90, 12, "", "!edification!", ""},
	{SKILL_PRAY, "penance", {91, 43, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_penance, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(547), 0, 12, "", "!penance!", ""},
	{SKILL_PRAY, "deconsecrate", {91, 32, 91, 91,91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_deconsecrate, TAR_CHAR_SELF, POS_STANDING,	
        NULL, SLOT(546), 90, 12, "", "!deconsecrate!", ""},
	{SKILL_PRAY, "detect good", {91, 4, 91, 91, 91, 91, 10, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_detect_good, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(513), 5,	12, "",	"The gold in your vision disappears.", ""},
	{SKILL_CHANT, "sense good", {91, 91,91, 91, 91, 91, 91, 91, 1, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, spell_detect_good, TAR_CHAR_SELF,	POS_STANDING, 
        NULL, SLOT(513), 5, 12, "",	"The gold in your vision disappears.",	""},
	{SKILL_PRAY, "true sight", {91, 27, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_true_sight, TAR_CHAR_SELF,	POS_STANDING, 
        NULL, SLOT(241), 5, 12, "", "You no longer possess true sight.", ""},
	{SKILL_CAST, "detect hidden", {15, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_detect_hidden, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(44), 5, 12, "", "You feel less aware of your surroundings.", ""},
	{SKILL_CHANT, "sense hidden", {91, 91, 91, 91, 91, 91, 91, 91, 50, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, spell_detect_hidden, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(44), 5, 12, "", "You feel less aware of your surroundings.", ""},
	{SKILL_CAST, "detect invis", {4, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_detect_invis, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(19), 5, 12, "", "You no longer see invisible objects.", ""},
	{SKILL_CAST, "detect magic", {2, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_detect_magic, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(20), 5, 12, "", "The detect magic wears off.", ""},
	{SKILL_CAST, "detect poison", {15, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_detect_poison, TAR_OBJ_INV, POS_STANDING, 
        NULL, SLOT(21), 5, 12, "", "You no longer detect poison.", ""},
	{SKILL_PRAY, "dispel evil", {91, 15, 91, 91, 91, 91, 19, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_dispel_evil, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(22), 15, 12, "dispel evil", "!Dispel Evil!",	""},
	{SKILL_PRAY, "dispel good",	{91, 15, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2, 2},	spell_dispel_good, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(512), 15, 12, "dispel good",	"!Dispel Good!", ""},
	{SKILL_CAST, "dispel magic", {16, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_dispel_magic, TAR_CHAR_OFFENSIVE, POS_FIGHTING,	
        NULL, SLOT(59), 15, 12, "",	"!Dispel Magic!", ""},
	{SKILL_PRAY, "earthquake",	{91, 10, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_earthquake, TAR_AREA_OFF, POS_FIGHTING, 
        NULL, SLOT(23), 15, 12, "earthquake", "!Earthquake!", ""},
	{SKILL_CAST, "firestorm", {60, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},	
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_firestorm, TAR_AREA_OFF, POS_FIGHTING, 
        NULL, SLOT(79), 50, 12, "firestorm", "!Firestorm!", ""},
	{SKILL_CAST, "enchant armor", {22, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},	
        {2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}, spell_enchant_armor, TAR_OBJ_INV, POS_STANDING, 
        NULL, SLOT(510), 100, 24, "", "!Enchant Armor!", ""},
	{SKILL_CAST, "enchant weapon", {23, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}, spell_enchant_weapon, TAR_OBJ_INV, POS_STANDING, 
        NULL, SLOT(24), 100, 24, "", "!Enchant Weapon!", ""},
	{SKILL_CAST, "energy drain", {19, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_energy_drain, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(25),	35, 12, "energy drain", "!Energy Drain!",	""},
	{SKILL_CAST, "faerie fire",	{6, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 43, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3}, spell_faerie_fire, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(72), 5, 12, "faerie fire", "The pink aura around you fades away.", ""},
	{SKILL_CAST, "faerie fog", {14, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_faerie_fog,	TAR_IGNORE,	POS_STANDING, 
        NULL, SLOT(73), 12, 12, "faerie fog", "!Faerie Fog!", ""},
	{SKILL_CAST, "farsight", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_farsight, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(521), 36, 20, "farsight", "!Farsight!", ""},
	{SKILL_CAST, "fireball", {22, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_fireball, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(26), 15, 12, "fireball", "!Fireball!", ""},
	{SKILL_CAST, "fireproof", {13, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},	
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_fireproof, TAR_OBJ_INV, POS_STANDING, 
        NULL, SLOT(523), 10, 12, "", "", "$p's protective aura fades."},
	{SKILL_PRAY, "flamestrike", {91, 20, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_flamestrike, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(65), 20, 12, "flamestrike", "!Flamestrike!", ""},
	{SKILL_CAST, "fly", {10, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_fly, TAR_CHAR_DEFENSIVE, POS_STANDING, 
        NULL, SLOT (56), 10, 18, "", "You slowly float to the ground.", ""},
	{SKILL_CAST, "floating disc", {4, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_floating_disc, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT (522), 40, 24, "", "!Floating disc!", ""},
    {SKILL_CAST, "floating letters", {15, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_floating_letters, TAR_STRING, POS_STANDING, 
        NULL, SLOT(739), 200, 24, "", "!Floating Letters!"},
	{SKILL_CAST, "giant strength", {11, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_giant_strength, TAR_CHAR_DEFENSIVE, POS_STANDING, 
        NULL, SLOT(39), 20, 12, "", "You feel weaker.", ""},
	{SKILL_PRAY, "harm", {91, 23, 91, 91, 91, 91, 40, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_harm, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(27), 15, 12, "harm spell", "!Harm!", ""},
	{SKILL_CAST, "haste", {21, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},	
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_haste, TAR_CHAR_DEFENSIVE, POS_FIGHTING, 
        NULL, SLOT(502), 30, 12, "", "You feel yourself slow down.", ""},
	{SKILL_PRAY, "heal", {91, 21, 91, 91, 91, 91, 33, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1}, spell_heal, TAR_CHAR_DEFENSIVE, POS_FIGHTING, 
        NULL, SLOT(28), 50, 12, "heal", "!Heal!", ""},
	{SKILL_PRAY, "greater heal", {91, 62, 91, 91, 91, 91, 66, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_greater_heal, TAR_CHAR_DEFENSIVE, POS_FIGHTING, 
        NULL, SLOT(548), 100, 12, "greater heal", "!Greater Heal!",	""},
	{SKILL_PRAY, "restore", {91, 85, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_restore, TAR_CHAR_DEFENSIVE, POS_FIGHTING, 
        NULL, SLOT(549), 500, 12, "restore", "!Restore!", ""},
	{SKILL_CAST, "heat metal", {47, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_heat_metal, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(516), 25, 18, "spell", "!Heat Metal!",""},
	{SKILL_CHANT, "scorch metal", {91, 91, 91, 91, 91, 91, 91, 91, 30, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, spell_heat_metal, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(516), 25, 18, "spell", "!Scorch Metal!", ""},
	{SKILL_CAST, "identify", {15, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_identify, TAR_OBJ_INV, POS_STANDING, 
        NULL, SLOT(91), 20, 12,	"", "!Identify!", ""},
	{SKILL_CAST, "infravision", {9, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_infravision, TAR_CHAR_DEFENSIVE, POS_STANDING, 
        NULL, SLOT(77), 20, 12, "", "You no longer see in the dark.", ""},
	{SKILL_CAST, "invisibility", {5, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_invis, TAR_OBJ_CHAR_DEF, POS_STANDING, 
        &gsn_invis,	SLOT(29), 15, 12, "", "You are no longer invisible.", "$p fades into view."},
	{SKILL_CAST, "mirror", {70, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_mirror, TAR_CHAR_SELF, POS_STANDING, 
        &gsn_mirror, SLOT(255), 80, 12, "", "The liquid reflector around you fades away.", ""},
	{SKILL_PRAY, "know alignment", {91,	9, 91, 91, 91, 91, 13, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_know_alignment,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING, 
        NULL, SLOT(58), 9, 12, "", "!Know Alignment!", ""},
	{SKILL_CHANT, "sense alignment", {91, 91, 91, 91, 91, 91, 91, 91, 5, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, spell_know_alignment,	TAR_CHAR_DEFENSIVE, POS_FIGHTING, 
        NULL, SLOT(58), 9, 12, "", "!Sense Alignment!", ""},
	{SKILL_CAST, "lightning bolt", {13, 91, 91, 91, 91, 91, 91, 91, 92, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_lightning_bolt,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING, 
        NULL, SLOT(30), 10, 12,	"lightning bolt", "!Lightning Bolt!", ""},
	{SKILL_CAST, "locate object", {9, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_locate_object, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(31), 20, 18, "", "!Locate Object!", ""},
	{SKILL_CAST, "magic missile", {1, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_magic_missile, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(32), 5, 12, "magic missile", "!Magic Missile!", ""},
	{SKILL_PRAY, "mass healing", {91, 38, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 2, 4, 4, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_mass_healing, TAR_IGNORE, POS_STANDING,	
        NULL, SLOT(508), 100, 36, "", "!Mass Healing!", ""},
	{SKILL_CAST, "mass invis", {22, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_mass_invis, TAR_IGNORE, POS_STANDING,
        &gsn_mass_invis, SLOT(69), 20, 24, "", "You are no longer invisible.", ""},
	{SKILL_CAST, "pass door",{24, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_pass_door, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(74), 20, 12, "", "You feel solid again.",	""},
	{SKILL_PRAY, "plague", {91, 30, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_plague, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_plague, SLOT (503), 20, 12, "sickness", "Your sores vanish.", ""},
	{SKILL_CAST, "poison", {17, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_poison, TAR_OBJ_CHAR_OFF, POS_FIGHTING,
        &gsn_poison, SLOT(33), 10, 12, "poison", "You feel less sick.", "The poison on $p dries up."},

	/* Mage Transportation */
	{SKILL_CAST, "portal",	{55, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 2, 4, 4, 4, 4, 2, 2, 4, 4, 4, 4, 4, 4, 4, 91}, spell_portal, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(519), 100, 24, "", "!Portal!", ""},
	{SKILL_CAST, "summon", {24, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_summon, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(40), 50, 12, "", "!Summon!", ""},
	{SKILL_CAST, "gate", {40, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_gate, TAR_IGNORE, POS_FIGHTING, 
        NULL, SLOT (83), 80, 12, "", "!Gate!", ""},
	{SKILL_PRAY, "nexus", {91, 45,	91, 91,	91, 91,	91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {2, 2, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_nexus, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(520), 150, 36, "", "!Nexus!", ""},
	{SKILL_PRAY, "evocation", {91, 28, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_evocation, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(619), 50, 12, "", "!Evocation!", ""},
	{SKILL_PRAY,"heavenly cloak", {91, 33, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_heavenly_cloak, TAR_IGNORE, POS_STANDING,
        &gsn_heavenly_cloak, SLOT(621), 20,	24, "", "You are no longer invisible.", ""},
	{SKILL_PRAY, "protection evil",	{91, 3, 91, 91, 91, 91, 25, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_protection_evil, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(34), 5, 12, "", "You feel less protected.", ""},
	{SKILL_PRAY, "protection good",	{91, 9, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_protection_good, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(514), 5, 12, "", "You feel less protected.", ""},
	{SKILL_CHANT, "aura of evil", {91, 91, 91, 91, 91, 91, 91, 91, 20, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, spell_protection_good, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(514), 5, 12, "", "You feel less protected.", ""},
	{SKILL_PRAY, "soul fire", {91, 12, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_soul_fire, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(617), 5, 12, "soul fire", "The pink aura around you fades away.", ""},
	{SKILL_PRAY, "hallowed vision", {91, 17, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_hallowed_vision, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(613), 12, 12, "hallowed vision", "!Hallowed Vision!", ""},
	{SKILL_PRAY, "celestial sight", {91, 16, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2}, prayer_celestial_sight, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(620), 20, 18, "", "!Celestial Sight!", ""},
	{SKILL_PRAY, "sustenance",	{91, 20, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_sustenance, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(614), 5, 12, "", "!Sustenance!", ""},
	{SKILL_PRAY, "holy water", {91, 14, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_holy_water, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(615), 20, 12, "", "!Holy Water!", ""},
	{SKILL_PRAY, "revelation",	{91, 26, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_revelation, TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(618), 12, 24, "", "!Revelation!", ""},
	{SKILL_PRAY, "levitation", {91, 26, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_levitation, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(616), 10, 18, "", "You slowly float to the ground.", ""},
	{SKILL_PRAY, "ray of truth", {91, 46, 91, 91, 91, 91, 61, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_ray_of_truth, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(518), 20, 12, "ray of truth", "!Ray of Truth!", ""},
	{SKILL_CAST, "recharge", {9, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_recharge, TAR_OBJ_INV, POS_STANDING, 
        NULL, SLOT(517), 60, 24, "", "!Recharge!", ""},
	{SKILL_CAST, "refresh", {8, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_refresh, TAR_CHAR_DEFENSIVE, POS_STANDING, 
        NULL, SLOT(81), 16, 18, "refresh", "!Refresh!", ""},
	{SKILL_PRAY, "renewal", {91, 18, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_renewal, TAR_CHAR_DEFENSIVE, POS_STANDING, 
        NULL, SLOT(637), 12, 18, "renewal", "!Renewal!", ""},
	{SKILL_CAST, "shield", {20, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_shield, TAR_CHAR_DEFENSIVE, POS_STANDING, 
        NULL, SLOT(67), 12, 18, "", "Your force shield shimmers then fades away.", ""},
	{SKILL_CAST, "shocking grasp",	{10, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_shocking_grasp,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(110), 10, 12, "shocking grasp", "!Shocking Grasp!", "Your blade no longer crackles with electricity."},
	{SKILL_CAST, "sleep", {10, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},	
        {1, 1,	2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_sleep, TAR_CHAR_OFFENSIVE, POS_STANDING,
        &gsn_sleep, SLOT(38), 15, 12, "", "You feel less tired.", ""},
	{SKILL_CAST, "slow", {23, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_slow, TAR_CHAR_OFFENSIVE, POS_FIGHTING,	
        NULL, SLOT(515), 30, 12, "", "You feel yourself speed up.",	""},
	{SKILL_CAST, "stone skin", {25, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_stone_skin, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT (66), 12, 18, "", "Your skin feels soft again.", ""},
	{SKILL_CAST, "teleport", {13, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_teleport, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(2), 35, 12, "", "!Teleport!", ""},
	{SKILL_CAST, "ventriloquate", {1, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_ventriloquate, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(41), 5, 12, "", "!Ventriloquate!", ""},
	{SKILL_CAST, "weaken", {11, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_weaken, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(68), 20, 12, "spell", "You feel stronger.", ""},
	{SKILL_CAST, "word of recall", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_word_of_recall, TAR_CHAR_SELF, POS_RESTING,
        NULL, SLOT(42), 5, 12, "", "!Word of Recall!", ""},

	{SKILL_PRAY, "holy ceremony", {91, 50, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_holy_ceremony, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(638), 12, 48, "", "!Holy Ceremony!", ""},

	{SKILL_CAST, "acid breath",	{71, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_acid_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(200), 100, 24, "blast of acid", "!Acid Breath!", ""},
	{SKILL_CAST, "fire breath", {80, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_fire_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT (201), 200, 24, "blast of flame", "The smoke leaves your eyes.", "Your blade loses its red-hot glow."},
	{SKILL_CAST, "frost breath", {74, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_frost_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT (202), 125, 24, "blast of frost", "!Frost Breath!", ""},
	{SKILL_CAST, "gas breath",	{82, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_gas_breath, TAR_AREA_OFF, POS_FIGHTING, 
        NULL, SLOT(203), 175, 24, "blast of gas", "!Gas Breath!", ""},
	{SKILL_CAST, "lightning breath", {81, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_lightning_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,	
        NULL, SLOT(204), 200, 12, "blast of lightning", "!Lightning Breath!", ""},

	{SKILL_CAST, "aqua breathe", {12, 91, 91, 91, 36, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},	
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_aqua_breathe, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(600), 10, 18, "", "Your lungs revert back to normal form."},
	{SKILL_CAST, "channel electricity",	{30, 91, 91, 91, 91,91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_channel_electricity, TAR_AREA_OFF, POS_FIGHTING, 
        NULL, SLOT(601), 20, 12, "surge of electricity", "!Electricity!"},
	{SKILL_CAST, "ice beam", {14, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_ice_beam, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(602), 15, 12, "beam of ice", "!Ice beam!", ""},
	{SKILL_CAST, "electrical charge", {19, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2}, spell_electrical_charge, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        NULL, SLOT(603), 15, 12, "charge of electricity", "!Electrical Charge!", ""},

	{SKILL_CAST, "general purpose",	{96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_general_purpose, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(401), 0, 12, "general purpose ammo", "!General Purpose Ammo!", ""},
	{SKILL_CAST, "high explosive",	{96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_high_explosive, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(402), 0, 12, "high explosive ammo", "!High Explosive Ammo!", ""},

	/* SONGS - BARD CLASS */
	{SKILL_SING, "Ballad of Ole Rip", {91, 91, 91, 91, 91, 72, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, song_rip, TAR_AREA_OFF, POS_STANDING,
        NULL, SLOT(688), 150, 12, "", "You feel less tired.", ""},
	{SKILL_SING, "Dance of Shadows", {91, 91, 91, 91, 91, 40, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, song_dance_shad, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(699), 50, 12, "ghoulish apparition", "You feel more able to sleep.", ""},
	{SKILL_SING, "Quiet Mountain Lake",	{91, 91, 91, 91, 91, 25, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, song_qmlake, TAR_IGNORE, POS_FIGHTING,
        NULL, SLOT(599), 30, 12, "", "Your serenity leaves you.", ""},
	{SKILL_SING, "The Pipers Melody", {91, 91, 91, 91, 91, 40, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, song_piper, TAR_AREA_OFF, POS_STANDING,
        &gsn_the_piper, SLOT(48), 60, 12, "", "You feel more self-confident.", ""},
	{SKILL_SING, "Ballad of the Cantankerous Woodsman",	{91, 91, 91, 91, 91, 28, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, song_ballad_woodie, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(457), 30, 12, "vindictive venom", "!Ballad of the Cantankerous Woodsman!", ""},
	{SKILL_SING, "Maelstrom of Nyvenban", {91, 91, 91, 91, 91, 38, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, song_maelstrom_of_nyvenban, TAR_IGNORE, POS_FIGHTING,
        NULL, SLOT(609), 50, 12, "", "!Maelstrom of Nyvenban!", ""},
	{SKILL_SING, "Kiss of First Love", {91, 91, 91, 91, 91, 32, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, song_kiss_of_first_love, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
        NULL, SLOT(609), 40, 12, "", "!Kiss of First Love!", ""},
	{SKILL_SING, "Song of Elyteness", {92, 92, 92, 92, 92, 63, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 91},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, song_elyteness, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(70), 60, 12, "pure elyteness", "!Elyteness!", ""},
	{SKILL_SING, "Song of Shattered Dreams", {92, 92, 92, 92, 92, 85, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 91},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, song_shattereddream, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(71), 100, 12, "Dream Shattering", "!Dreamshatter!", ""},
	{SKILL_SING, "Despair of Orpheus", {91, 91, 91, 91, 91, 44, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, song_orpheus, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(76), 20, 20, "song of despair", "You feel hope rush back into your body.", "$p looks more confident."},
	{SKILL_SING, "To the Four Corners", {91, 91, 91, 91, 91, 42, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, song_scatter, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(102), 50, 20, "", "!To the Four Corners!", ""},
	{SKILL_SING, "Path of the Nomad", {91, 91, 91, 91, 91, 42, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, song_disperse, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(103), 50, 20, "", "!Path of the Nomad!", ""},
	{SKILL_SING, "Falkentynes Fury", {91, 91, 91, 91, 91, 13, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, song_falkentynes_fury, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(605), 10, 12, "", "The howling rage of Falkentyne has left you.", "$p's fervent aura fades."},
	{SKILL_SING, "Walking on Clouds", {91, 91, 91, 91, 91, 9, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, song_walking_on_clouds, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(606), 10, 18, "", "Your feet no longer float amongst the clouds.", ""},
	{SKILL_SING, "Good Vibrations", {91, 91, 91, 91, 91, 14, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, song_good_vibrations, TAR_AREA_OFF, POS_FIGHTING,
        NULL, SLOT(607), 15, 12, "good vibrations", "!Good Vibrations!", ""},

	/* SPELLS - RANGER CLASS */
	{SKILL_CAST, "bark skin", {91, 91, 91, 91, 30, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1}, spell_bark_skin, TAR_CHAR_SELF, POS_STANDING, 
        &gsn_bark_skin, SLOT(604), 12, 18, "", "Your skin feels soft again.", ""},
	{SKILL_CAST, "Eyes of the Owl", {91, 91, 91, 91, 18, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2}, spell_eyes_of_the_owl, TAR_CHAR_DEFENSIVE, POS_STANDING, 
        NULL, SLOT(608), 5, 18, "", "You no longer see in the dark.", ""},

	/* PRAYERS - CLERIC CLASS */
	{SKILL_PRAY, "sanctuary", {91, 40, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_sanctuary,	TAR_CHAR_DEFENSIVE, POS_STANDING,
        &gsn_sanctuary, SLOT(36), 75, 12, "", "The white aura around your body fades.", ""},
	{SKILL_PRAY, "damnation", {91, 32, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_damnation, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT (612), 20, 12, "damnation", "You no longer feel forsaken.", "$p is no longer forsaken."},
	{SKILL_PRAY, "divine protection", {91, 23, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_divine_protection, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT (611), 12, 18, "", "You feel the heavenly force surrounding you ebb.", ""},
	{SKILL_PRAY, "haven", {91, 31, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_haven, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(234), 150, 12, "", "!Haven!", ""},
	{SKILL_PRAY, "holy sphere", {91, 76, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_group_sanctuary, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(92), 250, 12, "", "The white aura around your body fades.", ""},
	{SKILL_PRAY, "mass levitation",	{91, 34, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_mass_levitation, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(238), 50, 12, "", "You slowly float to the ground.",	""},
	{SKILL_PRAY, "calm", {91, 16, 91, 91, 91, 91, 43, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_calm, TAR_IGNORE, POS_FIGHTING, 
        NULL, SLOT(509), 30, 12, "", "You have lost your peace of mind.", ""},
	{SKILL_PRAY, "frenzy", {91, 24, 91, 91, 91, 91, 32, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_frenzy, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(504), 30, 24, "", "Your rage ebbs.", ""},
	{SKILL_PRAY, "holy word", {91, 36, 91, 91, 91, 91, 53, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {4, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}, prayer_holy_word, TAR_IGNORE, POS_FIGHTING,
        NULL, SLOT(506), 200, 24, "divine wrath", "!Holy Word!", ""},
	{SKILL_PRAY, "bless", {91, 7, 91, 91, 91, 91, 20, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_bless, TAR_OBJ_CHAR_DEF, POS_STANDING,
        NULL, SLOT(3), 5, 12, "", "You feel less righteous.", "$p's holy aura fades."},
	{SKILL_PRAY, "conservancy", {91, 6, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_conservancy, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(610), 5, 12, "", "You feel less armored.", ""},
	{SKILL_PRAY, "remove curse", {91, 18, 91, 91, 91, 91, 31, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, prayer_remove_curse, TAR_OBJ_CHAR_DEF, POS_STANDING,
        NULL, SLOT(35), 5, 12, "", "!Remove Curse!", ""},

	/* Chants - Reaver Class */
	{SKILL_CHANT, "shocking blade",	{92, 92, 92, 92, 92, 92, 92, 92, 7, 92, 92, 92, 92, 92, 92, 92},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, chant_shocking_blade, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(0), 30, 24, "", "!Shocking Blade!", ""},
	{SKILL_CHANT, "frost blade", {92, 92, 92, 92, 92, 92, 92, 92, 21, 92, 92, 92, 92, 92, 92, 92},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, chant_frost_blade, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(0), 50, 24, "", "!Frost Blade!", ""},
	{SKILL_CHANT, "flaming blade", {92, 92, 92, 92, 92, 92, 92, 92, 35, 92, 92, 92, 92, 92, 92, 92},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, chant_flaming_blade, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(0), 70, 24, "", "!Flaming Blade!", ""},
	{SKILL_CHANT, "chaotic blade", {92, 92, 92, 92, 92, 92, 92, 92, 49, 92, 92, 92, 92, 92, 92, 92},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, chant_chaotic_blade, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(0), 110, 24, "", "!Flaming Blade!", ""},
	{SKILL_CHANT, "venomous blade",	{92, 92, 92, 92, 92, 92, 92, 92, 63, 92, 92, 92, 92, 92, 92, 92},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, chant_venomous_blade, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(0), 130, 24, "", "!Venomus Blade!", ""},
	{SKILL_CHANT, "vampiric blade",	{92, 92, 92, 92, 92,92, 92, 92, 70, 92, 92, 92, 92, 92, 92, 92},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, chant_vampiric_blade, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(0), 150, 24, "", "!Vampiric Blade!", ""},
	{SKILL_CHANT, "apathy blade", {92, 92, 92, 92, 92, 92, 92, 92, 31, 92, 92, 92, 92, 92, 92, 92},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, chant_apathy_blade, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(0), 90, 24, "", "!Apathy Blade!",""},
	{SKILL_CHANT, "lightning burst", {92, 92, 92, 92, 92, 92, 92, 92, 14, 92, 92, 92, 92, 92, 92, 92},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, chant_lightning_burst, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(0), 40, 24, "electrical maelstrom", "!Lightning Burst!", ""},
	{SKILL_CHANT, "ice burst", {92, 92, 92, 92, 92, 92, 92, 92, 28, 92, 92, 92, 92, 92, 92, 92},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, chant_ice_burst, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(0), 60, 24, "devlish blizzard", "!Ice Burst!", ""},
	{SKILL_CHANT, "fire burst", {92, 92, 92, 92, 92, 92, 92, 92, 42, 92, 92, 92, 92, 92, 92, 92},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, chant_fire_burst, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(0), 80, 24, "scorching firestorm", "!Fire Burst!", ""},
	{SKILL_CHANT, "unholy burst", {92, 92, 92, 92, 92, 92, 92, 92, 56, 92, 92, 92, 92, 92, 92, 92},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, chant_unholy_burst, TAR_IGNORE, POS_STANDING,
        &gsn_unholy_burst, SLOT(0), 120, 24, "unholy wrath", "!Unholy Burst!", ""},
	{SKILL_CHANT, "hellscape", {92, 92, 92, 92, 92, 92, 92, 92, 77, 92, 92, 92, 92, 92, 92, 92},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, chant_hellscape_burst, TAR_IGNORE, POS_STANDING, 
        &gsn_hellscape,	SLOT(0), 150, 24, "demonic torment", "!Hellscape!", ""},
	{SKILL_CHANT, "summon sword", {92, 92, 92, 92, 92, 92, 92, 92, 1, 92, 92, 92, 92, 92, 92, 92},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2}, chant_summon_sword, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(0), 20, 24, "", "!Summon Sword!", ""},
	{SKILL_NORMAL, "invocation", {92, 92, 92, 92, 92, 92, 92, 92, 7, 92, 92, 92, 92, 92, 92, 92},
        {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_invocation, SLOT(0), 100, 48, "invoke", "Your blade fades, it's elemental magic drained.", ""},

	{SKILL_NORMAL, "axe", {91, 91, 1, 1, 1, 91, 1, 9, 92, 92, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 6, 4, 4, 0, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_axe, SLOT(0), 0, 0, "", "!Axe!", ""},
	{SKILL_NORMAL, "dagger", {36, 91, 1, 1, 1, 1, 1, 1, 92, 92, 1, 91, 91, 91, 91, 91, 91},
        {6, 0, 2, 2, 2, 2, 2, 2, 0, 0, 2, 0, 0, 0, 0, 4}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_dagger, SLOT(0), 0, 0, "", "!Dagger!", ""},
	{SKILL_NORMAL, "flail", {91, 1, 91, 1, 1, 91, 1, 3, 92, 92, 91, 91, 91, 91, 91, 91, 91},
        {0, 3, 0, 5, 5, 0, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_flail, SLOT(0), 0, 0, "", "!Flail!", ""},
	{SKILL_NORMAL, "mace", {91, 1, 91, 1, 91, 91, 1, 7, 92, 92, 91, 91, 91, 91, 91, 91, 91},
        {0, 2, 0, 3, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_mace, SLOT(0), 0, 0, "", "!Mace!", ""},
	{SKILL_NORMAL, "polearm", {91, 91, 91, 1, 1, 91, 1, 11, 92, 92, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 4, 4, 0, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_polearm, SLOT(0), 0, 0, "", "!Polearm!", ""},
	{SKILL_NORMAL, "shield block", {91, 1, 10, 1, 10, 1, 1, 91, 1, 91, 91, 91, 91, 91, 91, 8, 91},
        {0, 5, 3, 2, 3, 4, 2, 0, 2, 0, 0, 0, 0, 0 ,0, 2}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_shield_block, SLOT(0), 0, 0, "", "!Shield!", ""},
	{SKILL_NORMAL, "stun",	{91, 91, 91, 70, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 5, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_stun, SLOT(0), 0, 24, "stun attack", "!Stun!", ""},
	{SKILL_NORMAL, "staff",	{1, 5, 91, 1, 91, 91, 91, 6, 92, 92, 1, 91, 91, 91, 91, 1, 91}, 
        {2, 6, 0, 2, 0, 0, 0, 3, 0,	0, 2, 0, 0, 0, 1}, spell_null, TAR_IGNORE, POS_FIGHTING, 
        &gsn_staff, SLOT(0), 0, 0, "", "!Staff!", ""},
	{SKILL_NORMAL, "sword", {91, 91, 25, 1, 1, 10, 1, 5, 1, 92, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 6, 2, 3, 5, 3, 4, 1, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_sword,	SLOT(0), 0, 0, "", "!sword!", ""},
	{SKILL_NORMAL, "mask", {10, 10, 10, 10, 10, 10, 10, 50, 10, 10, 10, 10, 10, 10, 10, 10, 91}, 
        {1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1}, spell_null, TAR_IGNORE, POS_STANDING, &gsn_mask,
        SLOT(0), 0, 24, "mask", "Your disguise is discarded as you reclaim your true identity.", ""},
	{SKILL_NORMAL, "uppercut", {91, 91, 91, 23, 92, 57, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 4, 5, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_uppercut, SLOT(0), 0, 24, "uppercut", "You feel stronger.", ""},
	{SKILL_NORMAL, "riding", {43, 41, 33, 26, 1, 24, 11, 31, 20, 3, 30, 91, 91, 10, 91, 3, 91},
        {7, 6, 4, 3, 1, 4, 2, 4, 3, 3, 7, 0, 0, 7, 0, 3}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_riding, SLOT(0), 0, 24, "riding", "!Riding!", ""},
	{SKILL_NORMAL, "elusiveness", {91, 91, 15, 91, 91, 91, 91, 15, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 3, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_elusiveness, SLOT(0), 0, 24, "", "!Elusiveness!", ""},
	{SKILL_NORMAL, "retreat", {91, 91, 91, 16, 91, 91, 91, 91, 55, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_retreat, SLOT(0), 0, 24, "", "!Retreat!", ""},
	{SKILL_NORMAL, "tame", {91, 91, 91, 91, 8, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 12, 91},
        {0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_tame, SLOT(0), 0, 24, "tame", "!Tame!", ""},
	{SKILL_NORMAL, "kai", {91, 91, 30, 91, 91, 91, 91, 60, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_kai, SLOT(0), 0, 24, "paralyzing scream", "!Kai!", ""},
	{SKILL_NORMAL, "backstab", {91, 91, 10, 91, 91, 62, 91, 1, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 4, 0, 0, 10, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_backstab, SLOT(0), 0, 18, "backstab", "!Backstab!", ""},
	{SKILL_NORMAL, "shadowslash", {91, 91, 91, 91, 91, 91, 91, 91, 60, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 4, 0, 0, 10, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_shadowslash, SLOT(0), 0, 18, "shadow slash", "!Shadow Slash!", ""},
	{SKILL_NORMAL, "sap", {91, 91, 91, 91, 91, 91, 91, 40, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_sap, SLOT(0), 0, 18, "sap attempt", "!Sap!", ""},
	{SKILL_NORMAL, "eviscerate", {91, 91, 91, 91, 91, 91, 91, 81, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_eviscerate, SLOT(0), 0, 18, "evisceration", "!Eviscerate!", ""},
	{SKILL_NORMAL, "circle", {91, 91, 31, 91, 91, 91, 91, 50, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 5, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_circle, SLOT(0), 0, 21, "circle attack", "!Circle!", ""},
	{SKILL_NORMAL, "dual wield", {91, 91, 91, 45, 35, 91, 91, 75, 92, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 8, 8, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_dual_wield, SLOT(0), 0, 24, "dual wield", "!Dual wield!", ""},
	{SKILL_NORMAL, "bash", {91, 91, 91, 8, 91, 91, 11, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 3, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_bash, SLOT(0), 0, 20, "bash", "!Bash!", ""},
	{SKILL_NORMAL, "charge", {91, 91, 91, 15, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_charge, SLOT(0), 0, 24, "charge", "!Charge!", ""},
	{SKILL_NORMAL, "berserk", {91, 91, 91, 18, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_berserk, SLOT(0), 0, 24, "", "You feel your pulse slow down.", ""},
	{SKILL_NORMAL, "dirt kicking", {91, 91, 20, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 3, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_dirt, SLOT(0), 0, 24, "kicked dirt", "You rub the dirt out of your eyes.", ""},
	{SKILL_NORMAL, "disarm", {91, 91, 12, 11, 24, 91, 29, 91, 10, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 6, 4, 6, 0, 7, 0, 5, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_disarm, SLOT(0), 0, 12, "", "!Disarm!", ""},
	{SKILL_NORMAL, "dodge", {45, 29, 5, 20, 20, 15, 20, 4, 20, 2, 91, 10, 91, 50, 91, 5, 91},
        {8, 8, 4, 6, 6, 5, 6, 5, 5, 4, 0, 0, 0, 6, 0, 6}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_dodge, SLOT(0), 0, 0, "", "!Dodge!", ""},
	{SKILL_NORMAL, "fisticuffery", {91, 91, 30, 39, 91, 52, 91, 65, 91, 16, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 5, 7, 0, 7, 0, 6, 0, 5, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_fisticuffery, SLOT(0), 0, 0, "", "!Fisticuffery!", ""},
	{SKILL_NORMAL, "enhanced damage", {91, 91, 25, 30, 35, 91, 35, 20, 35, 27, 91, 91, 91, 40, 91, 26, 91},
        {0, 0, 5, 2, 5, 0, 4, 4, 3, 3, 0, 0, 0, 5, 0, 10}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_enhanced_damage, SLOT(0), 0, 0, "", "!Enhanced Damage!", ""},
	{SKILL_NORMAL, "critical strike", {91, 91, 91, 65, 91, 91, 91, 38, 91, 70, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 3, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_critical_strike, SLOT(0), 0, 0, "", "!Critical Strike!", ""},
	{SKILL_NORMAL, "swimming", {25, 25, 10, 18, 5, 15, 20, 22, 25, 5, 2, 91, 91, 5, 91, 2, 91},
        {6, 5, 2, 4, 1, 3, 4, 4, 6, 6, 6, 6, 6, 4, 6, 3}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_swim, SLOT(0), 0, 0, "", "!Swim!", ""},
	{SKILL_NORMAL, "envenom", {91, 91, 10, 91, 91, 91, 91, 23, 91, 91, 10, 91, 91, 91, 91, 91, 91},
        {0, 0, 4, 0, 0, 0, 0, 4, 0, 0, 4, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_envenom, SLOT(0), 0, 36, "", "!Envenom!", ""},
	{SKILL_NORMAL, "hand to hand", {91, 91, 8, 1, 17, 35, 26, 32, 91, 1, 91, 91, 91, 1, 91, 91, 91},
        {0, 0, 4, 4, 6, 6, 6, 4, 0, 4, 0, 0, 0, 6, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_hand_to_hand, SLOT(0), 0, 0, "", "!Hand to Hand!", ""},
	{SKILL_NORMAL, "kick", {91, 91, 14, 8, 10, 91, 8, 5, 7, 7, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 6, 3, 4, 0, 4, 4, 3, 3, 0, 0, 0, 0, 0, 0}, spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_kick, SLOT(0), 0, 12, "kick", "!Kick!", ""},
	{SKILL_NORMAL, "sweep", {91, 91, 91, 28, 91, 91, 28, 43, 91, 15, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 3, 0, 0, 4, 4, 0, 4, 0, 0, 0, 0, 0, 0}, spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_sweep, SLOT(0), 0, 12, "sweep", "!sweep!", ""},
	{SKILL_NORMAL, "parry", {91, 91, 23, 10, 20, 61, 10, 91, 10, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 6, 3, 4, 6, 4, 0, 2, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_parry, SLOT(0), 0, 0, "", "!Parry!", ""},
	{SKILL_NORMAL, "rescue", {91, 91, 91, 25, 91, 91, 6, 91, 12, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 4, 0, 0, 5, 0, 6, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_rescue, SLOT(0), 0, 12, "", "!Rescue!", ""},
	{SKILL_NORMAL, "trip", {91, 91, 12, 91, 23, 20, 91, 12, 5, 5, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 4, 0, 3, 6, 0, 5, 4, 4, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_trip, SLOT(0), 0, 10, "trip", "!Trip!", ""},
	{SKILL_NORMAL, "leadership", {91, 91, 91, 91, 91, 91, 29, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_leadership, SLOT(0), 0, 24, "", "!Leadership!", ""},
	{SKILL_NORMAL, "assassinate", {91, 91, 35, 91, 91, 91, 91, 21, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 5, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_assassinate, SLOT(0), 0, 24, "assassination attempt", "!Assassinate!", ""},
	{SKILL_NORMAL, "door bash",	{91, 91, 91, 13, 91, 91, 91, 91, 16, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 3, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_doorbash, SLOT(0), 0, 24, "door bash", "!Doorbash!", ""},
	{SKILL_NORMAL, "hone", {91, 91, 91, 55, 45, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_hone, SLOT(0), 0, 12, "", "!Hone!", ""},
	{SKILL_NORMAL, "throwing", {65, 91, 5, 21, 91, 10, 91, 5, 92, 35, 91, 91, 91, 91, 91, 91, 91},
        {6, 0, 3, 5, 0, 4, 0, 4, 0, 4, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_throwing, SLOT(0), 0, 20, "throw", "!Throwing!", ""},
	{SKILL_NORMAL, "layhands", {91, 91, 91, 91, 91, 91, 17, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_layhands, SLOT(0), 0, 24, "layhands", "!Layhands!", ""},
	{SKILL_NORMAL, "exorcism", {91, 80, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_exorcism, SLOT(0), 0, 24, "exorcism", "!exorcism!", ""},
	{SKILL_NORMAL, "instruments", {91, 91, 91, 91, 91, 1, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_instruments, SLOT(0), 0, 24, "instruments", "!Instruments!", ""},
	{SKILL_NORMAL, "composition", {91, 91, 91, 91, 91, 1, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_compose, SLOT(0), 0, 24, "compose", "!Compose!", ""},

	{SKILL_NORMAL, "conceal", {91, 91, 10, 91, 10, 13, 91, 20, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 4, 0, 4, 5, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_conceal, SLOT(0), 0, 24, "conceal", "!Conceal!", ""},
	{SKILL_NORMAL, "butcher", {18, 18, 18, 18, 5, 18, 18, 18, 5, 1, 15, 91, 91, 1, 91, 16, 91},
        {2, 2, 2, 2, 1,	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_butcher, SLOT(0), 0, 24, "butcher", "!Butcher!", ""},
	{SKILL_NORMAL, "endurance",	{91, 91, 91, 91, 9, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_endurance, SLOT(0), 0, 24, "endurance", "!Endurance!", ""},
	{SKILL_NORMAL, "redirect", {91, 91, 17, 14, 25, 91, 91, 91, 18, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 4, 3, 5, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_redirect, SLOT(0), 0, 0, "redirect", "!Redirect!", ""},
	{SKILL_NORMAL, "trample", {91, 91, 91, 91, 30, 91, 25, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 5, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_trample, SLOT(0), 0, 24, "trample", "!Trample!", ""},

	{SKILL_NORMAL, "second attack",	{91, 91, 30, 10, 91, 35, 15, 35, 15, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 4, 3, 4, 6, 3, 4, 3, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_second_attack,	SLOT(0), 0, 0, "", "!Second Attack!", ""},
	{SKILL_NORMAL, "third attack", {91, 91, 91, 40, 91, 91, 55, 91, 41, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 4, 5, 0, 4, 0, 4, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_third_attack, SLOT(0), 0, 0, "", "!Third Attack!", ""},

	//IBLIS 5/31/03 - MONK COMBAT SKILLS
	{SKILL_NORMAL, "split vision", {91, 91, 91, 91, 91, 91, 91, 91, 91, 43, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_split_vision, SLOT(0), 0, 0, "", "!split vision!", ""},
	{SKILL_NORMAL, "dirty tactics",	{91, 91, 67, 91, 91, 91, 91, 91, 91, 48, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 4, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_dirty_tactics, SLOT (0), 0, 0, "", "!dirty tactics!", ""},
	{SKILL_NORMAL, "block",	{91, 91, 91, 91, 91, 91, 91, 91, 91, 20, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_block, SLOT(0), 0, 0, "", "!Block!", ""},
	{SKILL_NORMAL, "counter", {91, 91, 91, 91, 91, 91, 91, 91, 91, 31, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_counter, SLOT(0), 0, 0, "", "!Counter!", ""},
	{SKILL_NORMAL, "knock", {91, 91, 91, 91, 91, 91, 91, 91, 91, 61, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_knock, SLOT(0), 0, 18, "knock attempt", "!Knock!", ""},
	{SKILL_NORMAL, "feigned amazement",	{91, 91, 91, 91, 91, 91, 91, 91, 91, 50, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}, spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_feign, SLOT(0), 0, 18, "", "!Feigned Amazement!", ""},
	{SKILL_NORMAL, "prevent escape", {91, 91, 91, 91, 91, 91, 91, 91, 50, 39, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_prevent_escape, SLOT(0), 0, 0, "failed escape attempt", "!Prevent Escape!", ""},
	{SKILL_NORMAL, "dive", {91, 91, 91, 91, 91, 91, 91, 91, 91, 12, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_dive, SLOT(0), 0, 20, "", "!Dive!", ""},
	{SKILL_NORMAL, "toss", {91, 91, 91, 91, 91, 91, 91, 91, 91, 38, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0 ,0 ,0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_toss, SLOT(0), 0, 12, "", "!Toss!", ""},
	{SKILL_NORMAL, "strike", {91, 91, 91, 91, 91, 91, 91, 91, 91, 37, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_strike, SLOT(0), 0, 24, "strike", "!Strike!", ""},
	{SKILL_NORMAL, "whirlwind", {91, 91, 91, 91, 91, 91, 91, 91, 91, 75, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_whirlwind,	SLOT (0), 0, 24, "whirlwind attack",	"!Whirlwind!", ""},
	{SKILL_NORMAL, "fists of fury",	{91, 91, 91, 91, 91, 91, 91, 91, 91, 80, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_fists_of_fury,	SLOT(0), 0, 13, "", "!Fists of Fury!", ""},

	/* non-combat skills */
	{SKILL_NORMAL, "fast healing", {91, 31, 91, 6, 26, 91, 10, 91, 10, 26, 91, 91, 91, 91, 91, 91, 91},
        {0, 5, 0, 4, 6, 0, 4, 0, 4, 4, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_SLEEPING,
        &gsn_fast_healing, SLOT(0), 0, 0, "", "!Fast Healing!", ""},
	{SKILL_NORMAL, "blindfighting", {91, 91, 91, 22, 91, 91, 30, 12, 60, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 4, 0, 0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_SLEEPING,
        &gsn_blindfighting, SLOT(0), 0, 0, "", "!Blindfighting!", ""},
	{SKILL_NORMAL, "haggle", {91, 91, 1, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_haggle, SLOT(0), 0, 0, "", "!Haggle!", ""},
	{SKILL_NORMAL, "influence", {91, 91, 91, 91, 91, 1, 6, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_influence, SLOT(0), 0, 0, "", "!Influence!", ""},
	{SKILL_NORMAL, "hide", {91, 91, 1, 91, 91, 22, 91, 10, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 3, 0, 0, 6, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_hide, SLOT(0), 0, 12, "", "!Hide!", ""},
	{SKILL_NORMAL, "camouflage", {91, 91, 91, 91, 20, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_camouflage, SLOT(0), 0, 12, "", "!Camouflage!", ""},
	{SKILL_NORMAL, "stalk", {91, 91, 9, 91, 91, 91, 91, 8, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 4, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_stalk, SLOT(0), 0, 12, "", "!Stalk!", ""},
	{SKILL_NORMAL, "lore", {91, 91, 6, 91, 33, 20, 91, 91, 91, 55, 91, 91, 91, 91, 91, 40, 91},
        {0, 0, 4, 0, 5, 4, 0, 0, 0, 4, 0, 0, 0, 0, 0, 4}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_lore, SLOT(0), 0, 36, "", "!Lore!", ""},
	{SKILL_NORMAL, "meditation", {6, 6, 91, 91, 91, 91, 91, 91, 91, 91, 20, 91, 91, 91, 91, 6, 91},
        {5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 5}, spell_null, TAR_IGNORE, POS_SLEEPING,
        &gsn_meditation, SLOT(0), 0, 0, "", "Meditation", ""},
	{SKILL_NORMAL, "peek", {91, 91, 1, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_peek, SLOT(0), 0, 0, "", "!Peek!", ""},
	{SKILL_NORMAL, "pick lock", {91, 91, 7, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_pick_lock, SLOT (0), 0, 12, "", "!Pick!", ""},
	{SKILL_NORMAL, "sneak", {91, 91, 4, 91, 91, 27, 91, 8, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 3, 0, 0, 6, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_sneak, SLOT (0), 0, 12, "", "You no longer feel stealthy.", ""},
	{SKILL_NORMAL, "track", {91, 91, 22, 91, 4, 91, 91, 25, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {0, 0, 4, 0, 2, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_track, SLOT(0), 0, 12, "", "!Track!", ""},
	{SKILL_NORMAL, "fishing", {10, 10, 10, 10, 10, 10, 10, 10, 10, 1, 18, 91, 91, 91, 91, 4, 91},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_fishing, SLOT(0), 0, 12, "", "!Fishing!", ""},
	{SKILL_NORMAL, "camping", {91, 91, 91, 91, 12, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_camping, SLOT(0), 0, 12, "", "!Camping!", ""},
	{SKILL_NORMAL, "foraging", {91, 91, 91, 91, 8, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_foraging, SLOT(0), 0, 12, "", "!Foraging!", ""},
	{SKILL_NORMAL, "brew", {37, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_brew, SLOT(0), 0, 12, "", "!Brew!", ""},
	{SKILL_NORMAL, "scribe", {30, 35, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {7, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_scribe, SLOT (0), 0, 12, "", "!Brew!", ""},
	{SKILL_NORMAL, "steal", {91, 91, 4, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_steal, SLOT(0), 0, 24, "", "!Steal!", ""},
	{SKILL_NORMAL, "palm", {91, 91, 5, 91, 91, 31, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 2, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_palm, SLOT(0), 0, 24, "", "!Palm!", ""},
	{SKILL_NORMAL, "scrolls", {1, 1, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	spell_null, TAR_IGNORE,	POS_STANDING, &gsn_scrolls,	SLOT (0), 0, 24, "", "!Scrolls!",	""},
	{SKILL_NORMAL, "staves", {1, 1, 91, 91, 91, 91, 91,	91, 91, 91, 6, 91, 91, 91, 91, 91, 91},
        {2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_staves, SLOT(0), 0, 12, "", "!Staves!", ""},
	{SKILL_NORMAL, "wands", {1, 12, 51, 91, 51, 51, 91, 65, 91, 91, 5, 91, 91, 91, 91, 91, 91},
        {3, 4, 7, 0, 7, 6, 0, 7, 0, 0, 5, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_wands, SLOT(0), 0, 12, "", "!Wands!", ""},
	{SKILL_NORMAL, "entangle", {91, 91, 91, 91, 30, 91, 91, 80, 91, 91, 91, 91, 91, 91, 91, 75, 91},
        {0, 0, 0, 0, 3, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 5}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_entangle, SLOT(0), 12, 18, "", "The plants holding you break and fall away.", ""},
	{SKILL_NORMAL, "tripwire", {91, 91, 91, 91, 91, 91, 91, 30, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0,	0, 0, 0, 0,	0, 0, 3, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_tripwire, SLOT(0), 12, 18, "", "!Tripwire!", ""},
	{SKILL_NORMAL, "wall of thorns", {91, 91, 91, 91, 23, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_wall_thorns, SLOT(0), 12, 18, "", "!Wall of thorns!", ""},
	{SKILL_NORMAL, "call mount", {91, 91, 91, 91, 91, 91, 23, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_summon_mount, SLOT(0), 0, 12, "", "!Summon Mount!", ""},

	//IBLIS 5/31/03 - MONK NONCOMBAT SKILLS
	{SKILL_NORMAL, "sense life",	{91, 91, 91, 91, 91, 91, 91, 91, 91, 10, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_sense_life, SLOT(0), 0, 0, "", "!Sense Life!", ""},
	{SKILL_NORMAL, "fighting arts",	{91, 91, 91, 91, 91, 91, 91, 91, 91, 25, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_the_arts, SLOT(0), 0, 0, "", "!Fighting Arts!", ""},
	{SKILL_NORMAL, "bandage", {91, 91, 91, 91, 91, 91, 91, 91, 91, 4, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_bandage, SLOT(0), 0, 60, "bandage", "!Bandage!", ""},
	{SKILL_NORMAL, "awareness",	{91, 91, 91, 91, 91, 91, 91, 91, 91, 70, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_awareness,	SLOT(0), 0, 0, "", "!Awareness!", ""},
	{SKILL_NORMAL, "hear the wind",	{91, 91, 91, 91, 91, 91, 91, 91, 91, 36, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_hear_the_wind,	SLOT(0), 0, 0, "", "!Hear The Wind!", ""},

	{SKILL_NORMAL, "invention",	{91, 91, 70, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 9, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_invention, SLOT(0), 0, 0, "", "!Invention!", ""},
	{SKILL_NORMAL, "falconry", {91, 91, 91, 91, 69, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0,	0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_falconry, SLOT(0), 0, 0, "", "!Falconry!", ""},
	{SKILL_NORMAL, "repulse", {91, 91, 91, 91, 91, 91, 91, 91, 23, 91, 91, 91, 91, 91, 91, 91, 91},	
        {0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_repulse, SLOT(0), 0, 24, "", "!Repulse!", ""},
	{SKILL_NORMAL, "tempt", {91, 91, 91, 91, 91, 91, 91, 91, 28, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_tempt, SLOT(0), 0, 24, "", "!Tempt!", ""},
	{SKILL_NORMAL, "archery", {91, 91, 91, 91, 30, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_archery, SLOT(0), 0, 6, "", "!Archery!", ""},
	{SKILL_NORMAL, "call beast", {91, 91, 91, 91, 61, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_summon_beast, SLOT (0), 0, 12, "", "!Summon Beast!", ""},
	{SKILL_NORMAL, "pounce", {91, 91, 91, 91, 65, 91, 91, 91, 91, 91,91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_pounce, SLOT(0), 0, 18, "", "!Pounce!", ""},
	{SKILL_NORMAL, "disorientation", {91, 91, 91, 91, 75, 91, 91, 91, 91, 91,91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_disorientation, SLOT(0), 0, 24, "disorientation", "You feel less disoriented.", ""},

	//Litan sunlight haste affect
	{SKILL_NORMAL, "swiftness",	{92, 92, 92, 92, 92, 92, 92, 92, 92, 92,92, 92, 92, 92, 92, 92, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_aquatitus, SLOT (0), 9999, 24, "You develop an allergic reaction to the water.", "Your swiftness wears off.", ""},
	//Kalian Night bonus
	{SKILL_NORMAL, "nachton",	{92, 92, 92, 92, 92, 92, 92, 92, 92, 92,92, 92, 92, 92, 92, 92, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_jalknation, SLOT(0), 9999, 24, "You feel invigorated by the night.", "Dawn approaches, and your invigoration subsides.", ""},
	//Kalian Day Penalty
	{SKILL_NORMAL, "aroran",	{92, 92, 92, 92, 92, 92, 92, 92, 92, 92,92, 92, 92, 92, 92, 92, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_jurgnation, SLOT(0), 9999, 24, "The daylight begins to wear you down.", "The sunlight receeds slightly and your vision returns.", ""},
	//Kalian Skill
	{SKILL_NORMAL, "fear",	{92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_fear, SLOT(0), 9999, 18, "","You feel less fearful now.", ""},
	{SKILL_NORMAL, "illusion",	{92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_illusion, SLOT(0), 9999, 24, "","!Illusion!", ""},
	{SKILL_NORMAL, "suck", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_suck, SLOT(0), 9999, 12, "","!Suck!", ""},

	//Kalian Day Penalty
	{SKILL_NORMAL, "likobe", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING, 
        &gsn_likobe, SLOT (0), 9999, 24, "The daylight begins to wear you down.", "Your sunburn vanishes.", ""},

	{SKILL_CAST, "spider vine",	{92, 92, 92, 92, 62, 91, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 91},
        {0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_spider_vine, TAR_CHAR_SELF, POS_STANDING,
        &gsn_spider_vine, SLOT(0), 0, 24, "", "The vines around your arms slowly recede and fall to the ground.", ""},
	{SKILL_NORMAL, "aggression", {92, 92, 92, 92, 63, 91, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 91},
        {0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_aggression, SLOT(0), 0, 24, "", "!Aggression!.", ""},

	// Necromancer chants
	{SKILL_CHANT, "death palm", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 62, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_death_palm, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(701), 30, 12, "", "!Death Palm!", ""},
	{SKILL_CHANT, "decay", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 49, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_decay, TAR_OBJ_PENTAGRAM, POS_STANDING,
        NULL, SLOT(702), 100,	24, "", "!Decay!", ""},
	{SKILL_CHANT, "knowledge of the dead", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 15, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_knowledge_of_the_dead, TAR_OBJ_PENTAGRAM, POS_STANDING,
        NULL, SLOT(703), 10,	1,"", "!Knowledge of the Dead!", ""},
	{SKILL_CHANT, "vampirism", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 66, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_vampirism, TAR_OBJ_PENTAGRAM, POS_STANDING,
        NULL, SLOT(704), 50, 4,"", "!Vampirism!", ""},
	{SKILL_CHANT, "exoskeleton", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 14, 91, 91, 91, 91, 91, 91},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_exoskeleton, TAR_CHAR_SELF,	POS_STANDING,
        NULL, SLOT(705), 20, 18, "", "Your skin returns to normal.", ""},
	{SKILL_CHANT, "unholy ceremony", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 50, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_unholy_ceremony, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(706), 20, 36, "", "!Unholy Ceremony!", ""},
	{SKILL_CHANT, "intoxicate", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 27, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_intoxicate, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_intoxicate, SLOT(707), 20, 12, "", "You snap back into reality.", ""},
	{SKILL_CHANT, "skeletal mutation", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 10, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_skeletal_mutation, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_skeletal_mutation,	SLOT(708), 40, 24, "", "Your bones shift back to normal.", ""},
	{SKILL_CHANT, "pox", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 8, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_pox, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_pox, SLOT(709), 20, 24, "", "The pocks disappear.", ""},
	{SKILL_CHANT, "drain aura", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 46, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_drain_aura, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(710), 0, 12, "", "!Drain Aura!", ""},
	{SKILL_CHANT, "immunity", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 36, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_immunity, TAR_CHAR_HEXAGRAM, POS_STANDING,
        &gsn_immunity, SLOT(711), 45, 24, "", "The grey in your skin fades.", ""},
	{SKILL_CHANT, "brain blisters", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 55, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_brain_blisters, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_brain_blisters, SLOT(712), 30, 18, "", "You are relieved to discover that all that remains of the blisters are black crusts.", ""},
	{SKILL_CHANT, "vomit", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 47, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_vomit, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_vomit, SLOT(713), 25, 12, "", "You feel better.", ""},
	{SKILL_CHANT, "atrophy", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 15, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_atrophy, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_atrophy, SLOT(714), 25, 12, "", "Strength floods back into your body.", ""},
	{SKILL_CHANT, "greymantle", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 67, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_greymantle, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_greymantle, SLOT(715), 75, 12, "", "Your shadow fades back into existance.", ""},
	{SKILL_CHANT, "lich eyes", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 1, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_lich_eyes, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(716), 5, 24, "", "Once again you see the world through mortal eyes.", ""},
	{SKILL_CHANT, "seizure", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 29, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_seizure, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(717), 0, 12, "", "!seizure!", ""},
	{SKILL_CHANT, "ghost form", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 29, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_ghost_form, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(718), 75, 12, "", "You return to the world of the living.", ""},
	{SKILL_CHANT, "eldritch miasma", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 31, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_eldritch_miasma, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(719), 31, 12, "", "The miasma mixes with the air and disappears.", ""},
	{SKILL_CHANT, "nearsight", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 7, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_nearsight, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(720), 30, 12, "", "Your vision returns to normal.", ""},
	{SKILL_CHANT, "clasp", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 60, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_clasp, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_clasp, SLOT(721), 60, 12, "", "The skeletal hands break into splinters.", ""},
	{SKILL_CHANT, "corpse walk", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 26, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_corpse_walk, TAR_CHAR_HEXAGRAM, POS_STANDING,
        NULL, SLOT(722), 100, 12, "", "!Corpse Walk!", ""},
	{SKILL_CHANT, "jinx", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 49, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_jinx, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(723), 50, 12, "", "Somehow you know that luck is on your side again.", ""},
	{SKILL_CHANT, "demonic possession", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 51, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_demonic_possession, TAR_CHAR_HEXAGRAM, POS_STANDING,
        NULL, SLOT(724), 100, 24, "", "You feel weaker as the sprit is released.", ""},
	{SKILL_CHANT, "beacon of the damned", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 80, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_beacon_of_the_damned, TAR_ROOM_SEPTAGRAM, POS_STANDING,
        NULL, SLOT(725), 600, 60, "", "The green pillar of fire on the horizon fades into nothingness.", ""},
	{SKILL_CHANT, "swell", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 86, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_swell, TAR_CHAR_HEXAGRAM, POS_STANDING,
        NULL, SLOT(726), 100, 24, "", "You shrink back to your natural size.", ""},
	{SKILL_CHANT, "death vision", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 18, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_death_vision, TAR_CHAR_WORLD, POS_STANDING,
        NULL, SLOT (727), 10, 1, "", "!Death Vision!.", ""},
	{SKILL_CHANT, "bone craft", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 40, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_bone_craft, TAR_STRING, POS_STANDING,
        NULL, SLOT(728), 69, 24, "", "!Bone Craft!.", ""},
	{SKILL_CHANT, "darkhounds", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 30, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_darkhounds, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(729), 30, 24, "", "!Darkhounds!.", ""},
	{SKILL_CHANT, "animate dead", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 23, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_animate_dead, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(730), 60, 24, "", "!Animate Dead!.", ""},
	{SKILL_CHANT, "astral body", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 5, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_astral_body, TAR_CHAR_DEFENSIVE, POS_STANDING,
        &gsn_astral_body, SLOT(731), 15, 12, "", "You are no longer invisible.", ""},
	{SKILL_CHANT, "calling", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 28, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_calling, TAR_CHAR_WORLD, POS_STANDING,
        NULL, SLOT(732), 50, 12, "", "!Calling!", ""},
	{SKILL_CHANT, "bone hail", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 75, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_bone_hail, TAR_ROOM_SEPTAGRAM, POS_FIGHTING,
        NULL, SLOT(732), 150, 12, "bone hail", "!Bone Hail!", ""},
	{SKILL_CHANT, "wind of death", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 46, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_wind_of_death, TAR_IGNORE, POS_FIGHTING, 
        NULL, SLOT(733), 45, 12, "wind of death", "!Wind of Death!", ""},
	{SKILL_CHANT, "festering boils", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 17, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_festering_boils, TAR_CHAR_SELF, POS_STANDING, 
        &gsn_festering_boils, SLOT(734), 50, 12, "", "The boils shrivel into crusts.", ""},
	{SKILL_CHANT, "soul link", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 81, 91, 91, 91, 91, 91, 91}, 
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_soul_link, TAR_CHAR_HEXAGRAM, POS_STANDING,
        &gsn_soul_link, SLOT(735), 55, 60, "soul link", "Your soul has been freed.", ""},
	{SKILL_CHANT, "death shroud", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 85, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_death_shroud, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_death_shroud, SLOT(736), 100, 24, "", "The smell of death leaves you.", ""},
	{SKILL_CHANT, "bone puppet", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 37, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_bone_puppet, TAR_STRING, POS_STANDING,
        NULL, SLOT(737), 303, 24, "", "!Bone Puppet!", ""},
	{SKILL_CHANT, "dark sphere", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 76, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_dark_sphere, TAR_ROOM_SEPTAGRAM, POS_STANDING,
        NULL, SLOT(738), 75, 24, "", "!Dark Sphere!", ""},
	{SKILL_CHANT, "aura rot", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 43, 91, 91, 91, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, chant_aura_rot,TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_aura_rot, SLOT(739), 30, 18, "", "Your aura strengthens and repels the curse.", ""},

	// Necromancer skills
	{SKILL_NORMAL, "leeching", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 14, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_leeching, SLOT(0), 0, 24, "", "!Leeching!", ""},
	{SKILL_NORMAL, "death ward", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 34, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_death_ward, SLOT (0),	0, 24, "", "The mask of blood cracks and falls off.", ""},
	{SKILL_NORMAL, "draw", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 13, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_draw, SLOT (0), 0, 12, "", "!Draw!", ""},
	{SKILL_NORMAL, "embalm", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 25, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_embalm, SLOT(0), 0, 24, "", "!Embalm!", ""},
	{SKILL_NORMAL, "sense death", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 25, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_sense_death, SLOT(0), 0, 0, "", "!Sense Death!", ""},
	{SKILL_NORMAL, "feed death", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 52, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_feed_death, SLOT(0), 0, 0, "", "!Feed Death!", ""},
	{SKILL_NORMAL, "feign death", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 40, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_feign_death, SLOT(0), 0, 0, "", "!Feign Death!", ""},
	{SKILL_NORMAL, "corrupt", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 47, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_corrupt, SLOT (0),	75, 0, "", "!Corrupt!", ""},
	{SKILL_NORMAL, "feast", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 60, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_feast, SLOT(0), 10, 12, "", "!Feast!", ""},
	{SKILL_NORMAL, "hellbreath", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 33, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_hellbreath, SLOT(0), 10, 12, "hellbreath", "!Hellbreath!", ""},
	{SKILL_NORMAL, "corpse carving", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 16, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,	
        &gsn_corpse_carving, SLOT(0), 10, 12, "", "!Corpse Carving!", ""},
	{SKILL_NORMAL, "unsummon", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 24, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,	
        &gsn_unsummon, SLOT(0), 10, 24, "", "!Unsummon!", ""},

	//Lich Skills
	{SKILL_NORMAL, "defile", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_defile, SLOT(0), 100, 24, "", "!Defile!", ""},
	{SKILL_NORMAL, "despoil", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_despoil, SLOT(0), 100, 24, "", "You feel somewhat more vulnerable.", ""},
	{SKILL_NORMAL, "contaminate", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_contaminate, SLOT(0), 100, 24, "", "You feel more pure.", ""},
//	{SKILL_NORMAL, "lich gaze", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92},
//        {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6}, spell_null, TAR_IGNORE, POS_STANDING,
//        &gsn_gaze, SLOT(0), 0, 0, "", "!Lich Gaze!", ""},
        

	// Litan skill
	{SKILL_NORMAL, "flare", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_flare, SLOT(0), 100, 24, "", "Your pupils return to their normal size.", ""},

	// Nerix skills
	{SKILL_NORMAL, "lightningbreath", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_lightningbreath, SLOT (0),	100, 24, "lightning breath", "!Lightningbreath!", ""},
	{SKILL_NORMAL, "swoop", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_swoop, SLOT(0), 0, 36, "", "!Swoop!", ""},

	//New Thief Skills (Traps)
	{SKILL_NORMAL, "detect traps", {91, 91, 32, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_detect_traps, SLOT(0), 100, 12, "", "!Detect traps!", ""},
	{SKILL_NORMAL, "avoid traps", {91, 91, 40, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_avoid_traps, SLOT(0), 100, 12, "", "!Avoid traps!", ""},
	{SKILL_NORMAL, "disarm traps", {91, 91, 50, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_disarm_traps, SLOT(0), 100, 12, "", "!Disarm traps!", ""},
	{SKILL_NORMAL, "create traps", {91, 91, 32, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_create_traps, SLOT(0), 100, 12, "", "!Create traps!", ""},

	//Chaos Jester Skills
	{SKILL_NORMAL, "flick", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 44, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_flick, SLOT(0), 0, 9, "flick", "!Flick!", ""},
	{SKILL_NORMAL, "flicker", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 1, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,4 ,0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_flicker, SLOT(0), 0, 0, "", "!Flicker!", ""},
	{SKILL_NORMAL, "warp touch", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 30, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_warp_touch, SLOT(0), 0, 0, "warp touch", "!Warp Touch!", ""},
	{SKILL_NORMAL, "mad rush", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 51, 91, 91, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_mad_rush, SLOT(0), 0, 24, "mad rush", "!Mad Rush!", ""},
	{SKILL_NORMAL, "coma", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 33, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_coma, SLOT (0),	0, 0, "", "!Coma!", ""},
	{SKILL_NORMAL, "vanish", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 11, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_vanish, SLOT(0), 0, 12, "", "You crawl back out from inside your navel.", ""},
	{SKILL_NORMAL, "persuade", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 7, 91, 91, 91},	
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_persuade, SLOT(0),	0, 24, "persuasion", "!Persuade!", ""},
	{SKILL_NORMAL, "maddance", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 66, 91, 91, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_mad_dance, SLOT(0), 0, 36, "", "You realize what a fool you look like and calm down.", ""},
	{SKILL_NORMAL, "firedance", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 55, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_fire_dance, SLOT(0), 0, 36, "", "The multicolored fire surrounding your hands dies down.", ""},
	{SKILL_NORMAL, "escape", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_escape, SLOT(0), 0, 12, "", "!Escape!", ""},
	{SKILL_NORMAL, "death spasms", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 15, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_death_spasms, SLOT(0), 0, 12, "", "You feel weaker.", ""},
	{SKILL_NORMAL, "shuffle", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 45, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_shuffle, SLOT(0), 0, 24, "shuffle", "!Shuffle!", ""},
	{SKILL_NORMAL, "dice", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 20, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_dice, SLOT(0), 0, 24, "", "!Dice!", ""},
	{SKILL_NORMAL, "lucky throw", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 52, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_lucky_throw, SLOT(0), 0, 24, "", "!Lucky Throw!", ""},

	//Chaos Jester Spells
	//NOTE : If SKILL_CAST and mana cost == 0, that means in the spell function the mana cost
	//will be handled.

	{SKILL_CAST,"vermin skin", {91, 91, 91, 91, 91, 91,  91, 91, 91, 91, 91, 91, 91, 19, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_vermin_skin, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(751), 0, 12, "", "The vermin decide you are too gross for them, and they scamper off.", ""},
	{SKILL_CAST,"vortex", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 56, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_vortex, TAR_CHAR_OFFENSIVE, POS_STANDING,
        NULL, SLOT(752), 0, 12, "", "!Vortex!", ""},
	{SKILL_CAST,"maelstrom", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 68, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_maelstrom, TAR_AREA_OFF, POS_FIGHTING,
        NULL, SLOT(753), 0, 12, "maelstrom", "!Maelstrom!", ""},
	{SKILL_CAST, "locust wings", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 28, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_locust_wings, TAR_OBJ_CHAR_DEF, POS_STANDING,
        &gsn_locust_wings, SLOT(754), 0, 12, "", "The wings slowly recede into your back.", ""},
	{SKILL_CAST, "torture", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 54, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_torture, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        &gsn_torture, SLOT(755), 0, 12, "barbed chains", "You finally break free of the barbed chains!", ""},
	{SKILL_CAST,"chaos warp", {91, 91, 91, 91, 91, 91,  91, 91, 91, 91, 91, 91, 91, 34, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_chaos_warp, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(756), 0, 12, "", "Your body returns to normal as reality sets in again.", ""},
	{SKILL_CAST,"warped space", {91, 91, 91, 91, 91, 91,  91, 91, 91, 91, 91, 91, 91, 82, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_warped_space, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(757), 0, 36, "", "!Warped spac!", ""},
	{SKILL_CAST,"malevolence", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 46, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_malevolence, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(758), 0, 12, "", "!Malevolence!", ""},
	{SKILL_CAST,"meat storm", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 21, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_meat_storm, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(759), 0, 12, "", "!Meat Storm!", ""},
	{SKILL_CAST, "imaginary garb", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 12, 91, 91, 91},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 6, 2, 2}, spell_imaginary_garb, TAR_STRING, POS_STANDING,
        NULL, SLOT(760), 0, 0, "", "!Imaginary Garb!"},
	{SKILL_CAST, "gravity",	{91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 13, 91, 91, 91},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_gravity, TAR_CHAR_SELF, POS_STANDING,
        &gsn_gravity, SLOT(761), 0, 12, "", "You jerk suddenly, and realize some of the power stored within you is gone."},
	{SKILL_CAST, "chaos bolt", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 52, 91, 91, 91},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_chaos_bolt, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(762), 0, 12, "", "!Chaos Bolt!"},
	{SKILL_CAST, "spider body",	{91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 60, 91, 91, 91},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2}, spell_spider_body, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT (763), 0, 12, "", "Your body returns to its normal form."},
	{SKILL_CAST, "meteor swarm", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 81, 91, 91, 91},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2}, spell_meteor_swarm, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_meteor_swarm, SLOT(764), 0, 12, "", "!Meteor Swarm!"},
	{SKILL_CAST, "scramble", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 32, 91, 91, 91},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_scramble, TAR_CHAR_SELF, POS_FIGHTING,
        &gsn_scramble, SLOT(765), 0, 12, "", "Your body parts return to the normal positions."},
	{SKILL_CAST, "vivify", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 23, 91, 91, 91},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_vivify,	TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(766), 0, 12,	"", "!Vivify!"},
	{SKILL_CAST, "wielded enemy", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 6, 91, 91, 91},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_wielded_enemy, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(767), 0, 24, "", "!Wielded Enemy!"},
	{SKILL_CAST, "erratic bolt", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 43, 91, 91, 91},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_erratic_bolt, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(768), 0, 12,	"erratic bolt", "!erratic bolt!"},
	{SKILL_CAST, "frog", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 17, 91, 91, 91},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_frog, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(769), 10, 12, "", "!Frog!"},
	{SKILL_CAST, "warped cat", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 27, 91, 91, 91},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_warped_cat, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(770), 100, 12, "", "!Warped Cat!"},
	{SKILL_CAST, "warped owl", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 37, 91, 91, 91},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_warped_owl,	TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(771), 100, 12, "", "!Warped Owl!"},
	{SKILL_CAST,"tornado", {91, 91, 91, 91, 91, 91,  91, 91, 91, 91, 91, 91, 91, 69, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_tornado, TAR_AREA_OFF, POS_FIGHTING, 
        NULL, SLOT(772), 0, 12, "tornado", "!Tornado!", ""},
	{SKILL_CAST,"switch", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 70, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_switch, TAR_IGNORE, POS_FIGHTING,
        NULL, SLOT(773), 200, 12, "", "!Switch!", ""},
	{SKILL_CAST,"evil twin", {91, 91, 91, 91, 91, 91,  91, 91, 91, 91, 91, 91, 91, 80, 91, 91, 91},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2}, spell_evil_twin, TAR_IGNORE, POS_STANDING,
        &gsn_evil_twin, SLOT(774), 0, 12, "", "You feel more alone.", ""},
	{SKILL_NORMAL, "larvae", {101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_larva,	SLOT(0), 25, 12, "", "The larvae leave your body.", ""},

	{SKILL_NORMAL, "lumberjacking",	{91, 91, 91, 91, 90, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_lumberjacking, SLOT(0), 0, 24, "", "!Lumberjacking!", ""},

	//TRADE SKILLS
	{SKILL_NORMAL, "forestry", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING, 
        &gsn_forestry, SLOT(0), 0, 24, "", "!Forestry!", ""},
	{SKILL_NORMAL, "milling", {91, 91, 91, 91, 90, 91, 91, 91, 91, 91,91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_milling, SLOT(0), 0, 24, "", "!Milling!", ""},
	{SKILL_NORMAL, "woodworking", {91, 91, 91, 91, 90, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING, 
        &gsn_woodworking, SLOT(0), 0, 24, "", "!Woodworking!", ""},
	{SKILL_NORMAL, "fletchery", {91, 91, 91, 91, 90, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91},
        {0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_fletchery, SLOT(0), 0, 24, "", "!Fletchery!", ""},

	// 06/27/2006 Hunyuk Dwarf skill Kuldar.
//	{SKILL_NORMAL, "kuldar", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92},
//        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE, POS_STANDING,
//        &gsn_kuldar, SLOT(0), 0, 0, "","!kuldar!", ""},
    
    // Druid Skills
    {SKILL_NORMAL, "shillelagh", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 1, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2}, spell_null, TAR_IGNORE, POS_RESTING, 
        &gsn_shillelagh, SLOT(0), 5, 1, "", "!shillelagh!", ""},
    {SKILL_NORMAL, "nature sense", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 10, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8}, spell_null, TAR_IGNORE, POS_STANDING, 
        &gsn_nature_sense, SLOT(0), 5, 10, "", "!nature sense!", ""},
    {SKILL_NORMAL, "animal empathy", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 20, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, spell_null, TAR_IGNORE, POS_STANDING, 
        &gsn_animal_empathy, SLOT(0), 5, 10, "", "!animal empathy!", ""},
    {SKILL_NORMAL, "animal friendship", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 31, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15}, spell_null, TAR_IGNORE, POS_STANDING, 
        &gsn_animal_friendship, SLOT(0), 5, 10, "", "!animal friendship!", ""},
    {SKILL_NORMAL, "satiate", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 49, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10}, spell_null, TAR_IGNORE, POS_STANDING, 
        &gsn_satiate, SLOT(0), 5, 10, "", "!wild invigoration!", ""},
    {SKILL_NORMAL, "wild invigoration", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 55, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10}, spell_null, TAR_IGNORE, POS_STANDING, 
        &gsn_wild_invigoration, SLOT(0), 5, 10, "", "!wild invigoration!", ""},
    {SKILL_NORMAL, "primeval guidance", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 64, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10}, spell_null, TAR_IGNORE, POS_STANDING, 
        &gsn_primeval_guidance, SLOT(0), 5, 10, "", "!primeval guidance!", ""},
    {SKILL_NORMAL, "shapechange", {91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 81, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25}, spell_null, TAR_IGNORE, POS_STANDING, 
        &gsn_shapechange, SLOT(0), 5, 10, "", "!shapechange!", ""},

    // Druid Spells
    {SKILL_CAST, "bear heart", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 3, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2}, spell_bear_heart, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(775), 10, 10, "", "You no longer posess the bears heart.", "You no longer posess the bears heart."},
    {SKILL_CAST, "chameleon shroud", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 5, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2}, spell_chameleon_shroud, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(776), 15, 10, "", "No more chameleon shroud.", ""},
    {SKILL_CAST, "neutralize poison", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 13, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2}, spell_neutralize_poison, TAR_CHAR_DEFENSIVE, POS_STANDING, 
        NULL, SLOT(777), 20, 10, "", "The poison in your veins has been neutralized.", ""},
    {SKILL_CAST, "tree stride", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 15, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2}, spell_tree_stride, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(778), 60, 10, "", "!tree_stride!", ""},
    {SKILL_CAST, "treant", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 18, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3}, spell_treant, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(779), 150, 10, "", "!treant!", ""},
    {SKILL_CAST, "plant shell", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 21, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3}, spell_plant_shell, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(780), 80, 10, "", "The plants growing on your body return to the wilds.", ""},
    {SKILL_CAST, "trackless step", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 24, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3}, spell_trackless_step, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(781), 40, 10, "", "!trackless_step!", ""},
    {SKILL_CAST, "oceanic wrath", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 26, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3}, spell_oceanic_wrath, TAR_AREA_OFF, POS_FIGHTING, 
        NULL, SLOT(782), 50, 10, "wave", "!oceanic_wrath!", ""},
    {SKILL_CAST, "commune", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 30, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3}, spell_commune, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(783), 20, 10, "", "!commune!", ""},
    {SKILL_CAST, "spore eruption", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 31, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, spell_spore_eruption, TAR_AREA_OFF, POS_FIGHTING, 
        NULL, SLOT(785), 40, 10, "`dbrown cloud of spores``", "!spore_eruption!", ""},
    {SKILL_CAST, "birth return", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 73, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5}, spell_birth_return, TAR_IGNORE, POS_STANDING, 
        NULL, SLOT(786), 120, 25, "", "!birth return!", ""},
    {SKILL_CAST, "waterwalk", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 32, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, spell_waterwalk, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(787), 30, 10, "", "!waterwalk!", ""},
    {SKILL_CAST, "wild enchant", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 33, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6}, spell_wild_enchant, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(788), 100, 10, "", "!wild_enchant!", ""},
    {SKILL_CAST, "winter wrath", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 34, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, spell_winter_wrath, TAR_AREA_OFF, POS_FIGHTING, 
        NULL, SLOT(789), 50, 10, "`oraging blizzard``", "!winter_wrath!", ""},
    {SKILL_CAST, "resist elements", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 35, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6}, spell_resist_elements, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(790), 125, 10, "", "Your resistance to the elements subsides.", "Your resistance to the elements subsides."},
    {SKILL_CAST, "ironwood", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 38, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6}, spell_ironwood, TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(791), 300, 40, "", "!ironwood!", ""},
    {SKILL_CAST, "insect plague", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 65, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, spell_insect_plague, TAR_CHAR_OFFENSIVE, POS_FIGHTING, 
        &gsn_insect_plague, SLOT(793), 60, 10, "", "The cloud of insects surrounding you disperses.", "The cloud of insects surrounding you disperses."},
    {SKILL_CAST, "desert wrath", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 49, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, spell_desert_wrath, TARGET_OBJ, POS_FIGHTING, 
        NULL, SLOT(794), 100, 10, "", "!desert_wrath!", ""},
    {SKILL_CAST, "draw life", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 50, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, spell_draw_life, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(795), 50, 20, "`bdraw life``", "!draw_life!", ""},
    {SKILL_CAST, "omen of clarity", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 25, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, spell_clarity, TAR_CHAR_SELF, POS_STANDING, 
        &gsn_clarity, SLOT(796), 20, 10, "", "`gYour spells no longer ring with clarity.``", ""},
    {SKILL_CAST, "tree form", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 58, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6}, spell_tree_form, TARGET_OBJ, POS_STANDING,
        NULL, SLOT(797), 150, 10, "", "!tree_form!", ""},
    {SKILL_CAST, "serene grove", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 61, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6}, spell_serene_grove, TARGET_OBJ, POS_STANDING, 
        NULL, SLOT(799), 80, 10, "", "!serene_grove!", ""},
    {SKILL_CAST, "regeneration", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 62, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6}, spell_regeneration, TAR_CHAR_SELF, POS_STANDING, 
        NULL, SLOT(800), 200, 10, "", "Your inner troll goes back to sleep.", "!regeneration!"},
    {SKILL_CAST, "arcane diminish", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 63, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6}, spell_arcane_diminish, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(801), 80, 10, "", "!diminish_unnatural_magic!", "Your arcane diminishing shroud subsides."},
    {SKILL_CAST, "moonlance", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 25, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6}, spell_moonlance, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(803), 40, 10, "`opiercing moonlance``", "!moonlance!", ""},
    {SKILL_CAST, "betrayal", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 78, 91},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6}, spell_betrayal, TAR_CHAR_OFFENSIVE, POS_STANDING, 
        NULL, SLOT(805), 50, 10, "", "!betrayal!", ""},
    {SKILL_CAST, "tap root", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 80, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6}, spell_tap_root, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(806), 60, 10, "`ddeadly tap root``", "!tap root!", ""},
    {SKILL_CAST, "scry", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 82, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6}, spell_scry, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(807), 150, 10, "", "!scry!", ""},
    {SKILL_CAST, "lunar heal", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 28, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6}, spell_lunar_heal, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
        NULL, SLOT(808), 100, 10, "", "!lunar_heal!", ""},
    {SKILL_CAST, "life circle", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 87, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6}, spell_life_circle, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(809), 100, 10, "", "!life_circle!", ""},
	{SKILL_CAST, "sprout tree", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 7, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3}, spell_sprout_tree, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(810), 25, 10, "", "!sprout_tree!", ""},
	{SKILL_CAST, "force of nature", {92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 20, 91}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3}, spell_force, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(811), 5, 10, "force of `cnature``", "!force_of_nature!", ""}
};   

const struct group_type group_table[MAX_GROUP] =
{
	{"rom basics", {-1, -1, -1, -1, -1, -1, -1, -1, -1}, {"recall"}	},
	{"mage basics", {-1, -1, -1, -1, -1, -1, -1, -1, -1}, {"staff"}},
	{"cleric basics", {-1, -1, -1, -1, -1, -1, -1, -1, -1},	{"mace"}},
	{"thief basics", {-1, -1, -1, -1, -1, -1, -1, -1, -1}, {"dagger", "steal"}},
	{"warrior basics", {-1, -1, -1, -1, -1, -1, -1, -1, -1}, {"sword", "second attack"}},
	{"ranger basics", {-1, -1, -1, -1, -1, -1, -1, -1, -1},	{"sword", "riding"}},
	{"bard basics", {-1, -1, -1, -1, -1, -1, -1, -1, -1}, {"dagger", "steal"}},
	{"paladin basics", {-1, -1, -1, -1, -1, -1, -1, -1, -1}, {"sword", "second attack"}},
	{"reaver basics", {-1, -1, -1, -1, -1, -1, -1, -1, 0}, {"sword", "second attack", "invocation", "summon sword"}},
	{"mage default", {-1, -1, -1, -1, -1, -1, -1, -1, -1}, {"beguiling", "combat", "detection", "enhancement", "illusion", "maladictions", "protective", "transportation", "weather"}},
	{"cleric default", {-1, -1, -1, -1, -1, -1, -1, -1, -1}, {"flail", "attack", "creation", "curative", "benedictions", "detection", "healing", "maladictions", "protective", "shield block", "transportation", "weather"}},
	{"thief default", {-1, -1, -1, -1, -1, -1, -1, -1, -1}, {"mace", "sword", "backstab", "disarm", "dodge", "second attack", "trip", "hide", "peek", "pick lock", "sneak"}},
	{"warrior default", {-1, -1, -1, -1, -1, -1, -1, -1, -1}, {"weaponsmaster", "shield block", "bash", "disarm", "enhanced damage", "parry", "rescue", "third attack"}},
	{"ranger default", {-1, -1, -1, -1, -1, -1, -1, -1, -1}, {"weaponsmaster", "shield block", "bash", "disarm", "enhanced damage", "parry", "rescue", "tame", "riding"}},
	{"bard default", {-1, -1, -1, -1, -1, -1, -1, -1, -1}, {"songs", "disarm", "dodge", "peek", "pick lock", "sneak", "lore",	"influence"}},
	{"paladin default", {-1, -1, -1, -1, -1, -1, -1, -1, -1}, {"sword",	"polearm",	"shield block",	"riding", "disarm", "enhanced damage", "parry", "rescue", "third attack"}},
	{"dark knight", {-1, -1, -1, -1, -1, -1, -1, -1, 45}, {"shield block", "sword", "disarm", "dodge", "enhanced damage", "kick", "parry", "rescue", "trip", "redirect", "second attack", "third attack", "fast healing"}},
	{"weaponsmaster", {-1, -1, -1, 15, -1, -1, -1, 18, -1}, {"axe", "dagger", "flail", "mace", "polearm", "staff", "sword"}},
	{"beguiling", {6, -1, -1, -1, -1, -1, -1, -1, -1}, {"charm person", "sleep", "mirror"}},
	{"combat", {7, -1, -1, -1, -1, -1,	-1, -1,	-1}, {"acid blast",	"burning hands", "chain lightning", "chill touch", "colour spray",	"fireball",	"lightning bolt", "magic missile", "shocking grasp"}},
	{"creation", {3, -1, -1, -1, -1, -1, -1, -1, -1}, {"create food", "create spring", "create water", "create rose", "floating disc"}},
	{"detection", {4, -1, -1, -1, -1, -1, -1, -1, -1}, {"detect hidden", "detect invis", "detect magic", "detect poison", "identify", "locate object"}},
	{"draconian", {12, -1, -1, -1, -1, -1, -1, -1, -1}, {"acid breath", "fire breath", "frost breath", "gas breath", "lightning breath"}},
	{"enchantment", {6, -1, -1, -1, -1,	-1, -1, -1, -1}, {"enchant armor", "enchant weapon", "fireproof", "recharge"}},
	{"enhancement", {5, -1, -1, -1, -1, -1, -1, -1, -1}, {"giant strength", "haste", "infravision", "refresh"}},
	{"illusion", {5, -1, -1, -1, -1, -1, -1, -1, -1}, {"invis", "mass invis", "ventriloquate", "web"}},
	{"maladictions", {5, -1, -1, -1, -1, -1, -1, -1, -1}, {"blindness", "change sex", "curse", "energy drain", "poison", "slow", "weaken"}},
	{"protective", {4, -1, -1, -1, -1, -1,	-1, -1, -1}, {"armor", "cancellation",	"dispel magic", "fireproof", "shield", "stone skin"}},
	{"transportation", {6, -1, -1, -1, -1, -1, -1, -1, -1},	{"fly", "gate", "pass door", "portal", "summon", "teleport"}},
	{"lullabies", {-1, -1, -1, -1, -1, 6, -1, -1, -1}, {"Ballad of Ole Rip", "Quiet Mountain Lake", "Walking on Clouds"},	},
	{"dissention", {-1, -1, -1, -1, -1, 7, -1, -1, -1},	{"To the Four Corners", "Path of the Nomad", "Dance of Shadows", "Maelstrom of Nyvenban"},},
	{"inspirational", {-1, -1, -1, -1, -1, 5, -1, -1, -1}, {"Kiss of First Love", "Falkentynes Fury", "The Pipers Melody"},	},
	{"rallying cries", {-1, -1, -1, -1, -1, 8, -1, -1, -1},	{"Good Vibrations", "Despair of Orpheus", "Song of Elyteness", "Ballad of the Cantankerous Woodsman", "Song of Shattered Dreams"},},
	{"weather", {5, -1,	-1, -1,	-1, -1,	-1, -1,	-1}, {"call lightning",	"control weather", "faerie fire", "faerie fog", "lightning bolt"}	},
	{"transcendence", {-1, 9, -1, -1, -1, -1, -1, -1, -1}, {"haven", "nexus", "evocation", "true sight", "heavenly cloak", "divine protection", "purification", "levitation"}},
	{"prophecy", {-1, 4, -1, -1, -1, -1, -1, -1, -1}, {"detect good", "detect evil", "know alignment", "revelation", "celestial sight", "illumination"}},
	{"wrath", {-1, 6, -1,	-1, -1,	-1, -1,	-1, -1}, {"damnation", "plague", "demonfire", "soul fire", "cause light", "cause serious", "cause critical", "harm"}},
	{"miracles", {-1, 7, -1, -1, -1, -1, -1, -1, -1}, {"flamestrike", "earthquake", "cure blindness", "cure poison", "cure disease", "continual light", "mass healing",	"mass levitation"}},
	{"determination", {-1, 6, -1, -1, -1,	-1, -1, -1, -1}, {"remove curse", "dispel evil", "dispel good", "conservancy", "protection evil", "protection good", "ray of truth",	"edification", "deconsecrate"}},
	{"restoration", {-1, 6, -1, -1, -1, -1, -1, -1, -1}, {"cure light", "cure serious", "cure critical", "heal",	"heroes feast", "sustenance", "holy water", "renewal",	"penance", "greater heal", "restore"}},
	{"piety", {-1, 8, -1, -1,	-1, -1,	-1, -1,	-1}, {"frenzy",	"bless", "holy word", "sanctuary", "holy sphere", "hallowed vision", "calm", "halo of seraphs"}	},
	{"integrity", {-1, -1, -1, -1, -1, -1, 6, -1, -1}, {"detect good", "detect evil", "know alignment",	"protection evil", "ray of truth"}},
	{"divine evocation", {-1, -1, -1, -1, -1, -1, 7, -1, -1}, {"frenzy", "bless", "holy word", "heal", "greater heal"},},
	{"indignation", {-1, -1, -1, -1, -1, -1, 6, -1, -1}, {"cause light", "cause critical", "cause serious", "harm"},},
	{"mercy", {-1, -1, -1, -1, -1, -1, 8, -1, -1}, {"remove curse", "cure disease", "cure light", "cure serious",	"heroes feast", "calm", "cure critical"},},
	{"dance of the wild", {-1, -1, -1, -1, 9, -1, -1, -1, -1}, {"bee swarm", "bee sting", "call lightning", "herbal lore",	"aqua breathe", "bark skin", "Eyes of the Owl"},},
	{"elemental", {8, -1, -1, -1, -1, -1, -1, -1, -1}, {"aqua breathe", "channel electricity", "ice beam",	"electrical charge", "fireshield", "firestorm",	"heat metal"}},

	/* Reaver Spells */
	{"faces of evil", {-1, -1, -1, -1, -1, -1, -1, -1, 10}, {"sense good", "sense hidden", "unholy smirk", "scorch metal",	"sense alignment", "aura of evil", "summon sword"}},
	{"shades of the blade", {-1, -1, -1, -1, -1, -1, -1, -1, 8}, {"shocking blade", "frost blade", "flaming blade", "chaotic blade", "venomous blade", "vampiric blade"}},
	{"elemental burst", {-1, -1, -1, -1, -1, -1, -1, -1, 10}, {"lightning burst", "ice burst", "fire burst", "unholy burst", "hellscape",}}
};
