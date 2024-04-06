#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "clans/new_clans.h"
#include "obj_trig.h"
#include "olc.h"

//Iblis 4/23/04 - Needed for oset
extern CHAR_DATA *GLOBAL_ot_ch;
extern OBJ_DATA *GLOBAL_ot_obj;
void free_ot args((OBJ_TRIG *ot));
OBJ_TRIG *new_ot args((int vnum));

//Iblis 4/23/04 - Needed for OEDIT extra
#define OEDIT( fun )            bool fun( CHAR_DATA *ch, char *argument )


char *material_list[] =
{ "plastic", "silver", "ebony", "slime", "iron", "cork", "flesh",
"jelly", "mithril", "cork", "wax", "platinum", "horn", "scales",
"rubber", "lead",
"sponge", "oil", "copper", "teflon", "balm", "cream", "hard leather",
"soft leather",
"kid leather", "fur", "snakeskin", "gut", "food", "meat", "bread",
"wood", "hardwood",
"softwood", "bamboo", "electrum", "bronze", "brass", "wire", "tin",
"pewter", "metal",
"porcelain", "ivory", "marble", "stone", "quartz", "flint", "enamel",
"glass",
"elastic", "silk", "satin", "lace", "wool", "linen", "canvas",
"cloth", "velvet",
"cotton", "felt", "paper", "parchment", "hemp", "feather",
"cardboard", "steel",
"gold", "pottery", "radioactive", "crystal", "ice", "bone", "shell",
"coral", "energy",
"fire", "air", "water", "acid", "coal", "ore", "sandstone", "clay",
"earth", "diamond",
"ethereal", "void", "copper", "fiber optics", "gunpowder",
"adamantite", "explosive",
"lumber", "wheat", "sugar", "vinyl", "liquid", "aluminum", "kevlar",
"silicon",
"magnetic", "tabacco", "biochemical", "bulletproof glass", NULL
};
struct flag_stat_type
{
	const struct flag_type *structure;
	bool stat;
};

const struct flag_type extra_flags[MAX_EXTRA_FLAGS][31] = {
	{
		{"glow", ITEM_GLOW, TRUE},
		{"hum", ITEM_HUM, TRUE},
		{"dark", ITEM_DARK, TRUE},
		{"lock", ITEM_LOCK, TRUE},
		{"evil", ITEM_EVIL, TRUE},
		{"invis", ITEM_INVIS, TRUE},
		{"magic", ITEM_MAGIC, TRUE},
		{"no_drop", ITEM_NODROP, TRUE},
		{"bless", ITEM_BLESS, TRUE},
		{"anti-good", ITEM_ANTI_GOOD, TRUE},
		{"anti-evil", ITEM_ANTI_EVIL, TRUE},
		{"anti-neutral", ITEM_ANTI_NEUTRAL, TRUE},
		{"no_remove", ITEM_NOREMOVE, TRUE},
		{"inventory", ITEM_INVENTORY, TRUE},
		{"nopurge", ITEM_NOPURGE, TRUE},
		{"rot-death", ITEM_ROT_DEATH, TRUE},
		{"vis-death", ITEM_VIS_DEATH, TRUE},
		{"no_sac", ITEM_NOSAC, TRUE},
		{"nonmetal", ITEM_NONMETAL, TRUE},
		{"nolocate", ITEM_NOLOCATE, TRUE},
		{"melt_drop", ITEM_MELT_DROP, TRUE},
		{"had_timer", ITEM_HAD_TIMER, TRUE},
		{"sell_extract", ITEM_SELL_EXTRACT, TRUE},
		{"burn_proof", ITEM_BURN_PROOF, TRUE},
		{"no_uncurse", ITEM_NOUNCURSE, TRUE},
		{"no_identify", ITEM_NOIDENTIFY, TRUE},
		{"no_save", ITEM_NOSAVE, TRUE},
		{"no_mob", ITEM_NOMOB, TRUE},
		{"no_enchant", ITEM_NOENCHANT, TRUE},
		{NULL, 0, 0},
	},
	{
		{"no_stock", ITEM_NOSTOCK, TRUE},
		{"no_donate", ITEM_NODONATE, TRUE},
		{"no_auction", ITEM_NOAUCTION, TRUE},
		{"tattoo", ITEM_TATTOO, TRUE},
		{"wizi", ITEM_WIZI, TRUE},
		{"imaginary", ITEM_IMAGINARY, TRUE},
		{"winged", ITEM_WINGED, FALSE},
		{"noavatar", ITEM_NOAVATAR, TRUE},
		{"nokalian", ITEM_NOKALIAN, TRUE},
		{"lightning_proof", ITEM_LIGHTNING_PROOF, TRUE},
		{"cold_proof", ITEM_COLD_PROOF, TRUE},
		{"acid_proof", ITEM_ACID_PROOF, TRUE},
		{"invis_wear", ITEM_INVIS_WEAR, TRUE},
		{"pulsating", ITEM_PULSATING, TRUE},
		{NULL, 0, 0},
		{NULL, 0, 0},
		{NULL, 0, 0},
		{NULL, 0, 0},
		{NULL, 0, 0},
		{NULL, 0, 0},
		{NULL, 0, 0},
		{NULL, 0, 0},
		{NULL, 0, 0},
		{NULL, 0, 0},
		{NULL, 0, 0},
		{NULL, 0, 0},
		{NULL, 0, 0},
		{NULL, 0, 0},
		{NULL, 0, 0},
		{NULL, 0, 0},
		{NULL, 0, 0},
		}
};


extern int flag_lookup (const char *name, const struct flag_type *flag_table);

/*****************************************************************************
Name:		flag_stat_table
Purpose:	This table catagorizes the tables following the lookup
functions below into stats and flags.  Flags can be toggled
but stats can only be assigned.  Update this table when a
new set of flags is installed.
****************************************************************************/
const struct flag_stat_type flag_stat_table[] = {
	/*  {   structure               stat    }, */
	{area_flags, FALSE}, {sex_flags, TRUE}, {exit_flags, FALSE}, {door_resets, TRUE}, {room_flags, FALSE}, {Class_flags, FALSE}, {clan_flags, FALSE}, {pcrace_flags, FALSE}, {sector_flags, TRUE}, {tsector_flags, FALSE}, {type_flags, TRUE}, 
	{extra_flags[0], FALSE}, 
	{extra_flags[1], FALSE}, 
	{gate_flags, FALSE}, {pact_flags, FALSE}, {wear_flags, FALSE}, {act_flags, FALSE}, {act_flags2, FALSE},	//Adeon 7/1/03
	{affect_flags, FALSE}, {recruit_flags, FALSE}, {apply_flags, TRUE}, {wear_loc_flags, TRUE}, {wear_loc_strings, TRUE}, {weapon_flags, TRUE}, {container_flags, FALSE}, {liquid_flags, TRUE}, {material_type, TRUE}, {form_flags, FALSE}, {part_flags, FALSE}, {ac_type, TRUE}, {size_flags, TRUE}, {position_flags, TRUE}, {off_flags, FALSE}, {imm_flags, FALSE}, {res_flags, FALSE}, {vuln_flags, FALSE}, {weapon_Class, TRUE}, {instrument_flags, FALSE}, {furniture_type, FALSE}, {weapons_type, FALSE}, {rarity_flags, TRUE}, {room_flags2, FALSE}, {card_Class, TRUE}, {card_terrain_type, TRUE}, {card_element_type, TRUE}, {card_race_type, TRUE}, {card_terrain_toggles,FALSE},{trap_type,TRUE},{trap_flag,FALSE},{dir_flags,TRUE},
	{0, 0}
};

/*****************************************************************************
Name:		is_stat( table )
Purpose:	Returns TRUE if the table is a stat table and FALSE if flag.
Called by:	flag_value and flag_string.
Note:		This function is local and used only in bit.c.
****************************************************************************/
bool is_stat (const struct flag_type *flag_table)
{
	int flag;
	for (flag = 0; flag_stat_table[flag].structure; flag++)
	{
		if (flag_stat_table[flag].structure == flag_table
			&& flag_stat_table[flag].stat)
			return TRUE;
	}
	return FALSE;
}


/*****************************************************************************
Name:		flag_value( table, flag )
Purpose:	Returns the value of the flags entered.  Multi-flags accepted.
Called by:	olc.c and olc_act.c.
****************************************************************************/
int flag_value (const struct flag_type *flag_table, char *argument)
{
	char word[MAX_INPUT_LENGTH];
	int bit;
	int marked = 0;
	bool found = FALSE;
	if (is_stat (flag_table))
	{
		one_argument (argument, word);
		if ((bit = flag_lookup (word, flag_table)) != NO_FLAG)
			return bit;

		else
			return NO_FLAG;
	}

	/*
	* Accept multiple flags.
	*/
	for (;;)
	{
		argument = one_argument (argument, word);
		if (word[0] == '\0')
			break;
		if ((bit = flag_lookup (word, flag_table)) != NO_FLAG)
		{
			SET_BIT (marked, bit);
			found = TRUE;
		}
	}
	if (found)
		return marked;

	else
		return NO_FLAG;
}


/*****************************************************************************
Name:		flag_string( table, flags/stat )
Purpose:	Returns string with name(s) of the flags or stat entered.
Called by:	act_olc.c, olc.c, and olc_save.c.
****************************************************************************/
char *flag_string (const struct flag_type *flag_table, int bits)
{
	static char buf[512];
	int flag;
	buf[0] = '\0';
	for (flag = 0; flag_table[flag].name != NULL; flag++)
	{
		if (!is_stat (flag_table) && IS_SET (bits, flag_table[flag].bit))
		{
			strcat (buf, " ");
			strcat (buf, flag_table[flag].name);
		}
		else if (flag_table[flag].bit == bits)
		{
			strcat (buf, " ");
			strcat (buf, flag_table[flag].name);
			break;
		}
	}
	return (buf[0] != '\0') ? buf + 1 : "none";
}

/*****************************************************************************
Name:          flag_string( table, flags/stat )
Purpose:       Returns string with name(s) of the flags or stat entered.
Called by:     act_olc.c, olc.c, and olc_save.c.
****************************************************************************/
/*char *flag_string2 (const struct flag_type **flag_table, int max_elements, int bits[])
{
static char buf[1024];
int flag,i,j;
buf[0] = '\0';
for (i=0;i < max_elements;i++)
{
for (j=0;j < max_elements;j++)
{
for (flag = 0; flag_table[i][flag].name != NULL; flag++)
{
if (!is_stat (flag_table[i]) && IS_SET (bits[j], flag_table[i][flag].bit))
{
strcat (buf, " ");
strcat (buf, flag_table[i][flag].name);
}
else if (flag_table[i][flag].bit == bits[j])
{
strcat (buf, " ");
strcat (buf, flag_table[i][flag].name);
break;
}
}
}
}
return (buf[0] != '\0') ? buf + 1 : "none";
}*/


char *crflag_string (const struct flag_type *flag_table, int bits)
{
	static char buf[512];
	int flag;
	buf[0] = '\0';
	for (flag = 0; flag_table[flag].name != NULL; flag++)
	{
		if (!is_stat (flag_table) && IS_SET (bits, flag_table[flag].bit))
		{
			strcat (buf, " ");
			strcat (buf, flag_table[flag].name);
		}
		else if (flag_table[flag].bit == bits)
		{
			strcat (buf, " ");
			strcat (buf, flag_table[flag].name);
			break;
		}
	}
	return (buf[0] != '\0') ? buf + 1 : "all";
}
const struct flag_type area_flags[] = {
	{"none", AREA_NONE, FALSE},
	{"changed", AREA_CHANGED, TRUE},
	{"added", AREA_ADDED, TRUE},
	{"loading", AREA_LOADING, FALSE}, 
	{"imp_only", AREA_IMP_ONLY, TRUE},
	{"no_quit", AREA_NO_QUIT, TRUE},
	{"no_repop_wia", AREA_NO_REPOP_WIA, TRUE},
	{"no_tree", AREA_NO_TREE, TRUE},
	{"solo", AREA_SOLO, TRUE},
	{"no_repop", AREA_NO_REPOP, TRUE},
	{NULL, 0, 0}
};
const struct flag_type sex_flags[] = {
	{"male", SEX_MALE, TRUE},
	{"female", SEX_FEMALE, TRUE},
	{"neutral", SEX_NEUTRAL, TRUE}, {"either", 3, TRUE}, {NULL, 0, 0}
};
const struct flag_type pact_flags[] = {
	{"enter", PACT_ENTER, TRUE},
	{"exit", PACT_EXIT, TRUE},
	{"climb", PACT_CLIMB, TRUE},
	{"play", PACT_PLAY, TRUE},
	{"crawl", PACT_CRAWL, TRUE},
	{"descend", PACT_DESCEND, TRUE},
	{"scale", PACT_SCALE, TRUE},
	{"jump", PACT_JUMP, TRUE},
	{"tug", PACT_TUG, TRUE},
	{"ring", PACT_RING, TRUE},
	{"shove", PACT_SHOVE, TRUE},
	{"smash", PACT_SMASH, TRUE}, 
	{"dig", PACT_DIG, TRUE},
	{NULL, 0, 0}
};
const struct flag_type tact_flags[] = {
	{"drive", TACT_DRIVE, TRUE},
	{"sail", TACT_SAIL, TRUE},
	{"paddle", TACT_PADDLE, TRUE},
	{"row", TACT_ROW, TRUE}, {NULL, 0, 0}
};
const struct flag_type button_flags[] = {
	{"press", BUTTON_PRESS, TRUE},
	{"pull", BUTTON_PULL, TRUE}, {NULL, 0, 0}
};
const struct flag_type gate_flags[] = {
	{"normal", GATE_NORMAL_EXIT, TRUE},
	{"no_curse", GATE_NOCURSE, TRUE},
	{"go_with", GATE_GOWITH, TRUE},
	{"buggy", GATE_BUGGY, TRUE},
	{"random", GATE_RANDOM, TRUE},
	{"transport", GATE_TRANSPORT, TRUE},
	{"showexit", GATE_SHOWEXIT, TRUE}, {NULL, 0, 0}
};
const struct flag_type exit_flags[] = {
	{"door", EX_ISDOOR, TRUE},
	{"closed", EX_CLOSED, TRUE},
	{"locked", EX_LOCKED, TRUE},
	{"pickproof", EX_PICKPROOF, TRUE},
	{"nopass", EX_NOPASS, TRUE},
	{"easy", EX_EASY, TRUE},
	{"hard", EX_HARD, TRUE},
	{"infuriating", EX_INFURIATING, TRUE},
	{"noclose", EX_NOCLOSE, TRUE},
	{"nolock", EX_NOLOCK, TRUE},
	{"nobash", EX_NOBASH, TRUE}, {NULL, 0, 0}
};
const struct flag_type lock_flags[] = {
	{"door", EX_ISDOOR, TRUE},
	{"pickproof", EX_PICKPROOF, TRUE},
	{"nobash", EX_NOBASH, TRUE},
	{"nopass", EX_NOPASS, TRUE}, 
	{"hard", EX_HARD, TRUE},
	{"infuriating", EX_INFURIATING, TRUE},
	{NULL, 0, 0}
};
const struct flag_type door_resets[] = {
	{"open and unlocked", 0, TRUE},
	{"closed and unlocked", 1, TRUE},
	{"closed and locked", 2, TRUE}, {NULL, 0, 0}
};
const struct flag_type room_flags[] = {
	{"dark", ROOM_DARK, TRUE},
	{"elevator", ROOM_ELEVATOR, TRUE},
	{"nomount", ROOM_NOMOUNT, TRUE},
	{"silent", ROOM_SILENT, TRUE},
	{"no_mob", ROOM_NO_MOB, TRUE},
	{"transport", ROOM_TRANSPORT, TRUE},
	{"indoors", ROOM_INDOORS, TRUE},
	{"private", ROOM_PRIVATE, TRUE},
	{"stable", ROOM_STABLE, TRUE},
	{"safe", ROOM_SAFE, TRUE},
	{"solitary", ROOM_SOLITARY, TRUE},
	{"pet_shop", ROOM_PET_SHOP, TRUE},
	{"no_recall", ROOM_NO_RECALL, TRUE},
	{"imp_only", ROOM_IMP_ONLY, TRUE},
	{"gods_only", ROOM_GODS_ONLY, TRUE},
	{"noflymount", ROOM_NOFLYMOUNT, TRUE},
	{"noswimmount", ROOM_NOSWIMMOUNT, TRUE},
	{"newbies_only", ROOM_NEWBIES_ONLY, TRUE},
	{"justice", ROOM_LAW, TRUE},
	{"nowhere", ROOM_NOWHERE, TRUE},
	{"arena", ROOM_ARENA, TRUE},
	{"spectator", ROOM_SPECTATOR, TRUE},
	{"aregistration", ROOM_ARENA_REGISTRATION, TRUE},
	{"no_push", ROOM_NO_PUSH, TRUE},
	{"bank", ROOM_BANK, TRUE},
	{"build", ROOM_BUILD, TRUE},
	{"bloodbath", ROOM_BLOODBATH, TRUE},
	{"no_charmie", ROOM_NO_CHARMIE, TRUE},
	{"no_fly", ROOM_NO_FLY, TRUE},
	{"pier", ROOM_PIER, TRUE},
	{"shrine", ROOM_SHRINE, TRUE}, {NULL, 0, 0}
};

//Adeon -- had to expand room flags
const struct flag_type room_flags2[] = {
	{"sinking", ROOM_SINKING, TRUE},
	{"novehicle", ROOM_NOVEHICLE, TRUE}, 
	{"alwaysdark", ROOM_ALWAYS_DARK, TRUE},
	{"nomagic", ROOM_NOMAGIC,TRUE},
	{"pkonly", ROOM_PKONLY,TRUE},
	{"warped", ROOM_WARPED, TRUE},
	{"solo",ROOM_SOLO,TRUE},
	{"mirror",ROOM_MIRROR,TRUE},
	{"ice",ROOM_ICE,TRUE},
	{"snow",ROOM_SNOW,TRUE},
	{NULL, 0, 0}
};
const struct flag_type sector_flags[] = {
	{"default", SECT_INSIDE, TRUE},
	{"city", SECT_CITY, TRUE},
	{"field", SECT_FIELD, TRUE},
	{"forest", SECT_FOREST, TRUE},
	{"hills", SECT_HILLS, TRUE},
	{"mountain", SECT_MOUNTAIN, TRUE},
	{"water_crossable", SECT_WATER_SWIM, TRUE},
	{"water_swim", SECT_WATER_NOSWIM, TRUE},
	{"air", SECT_AIR, TRUE},
	{"desert", SECT_DESERT, TRUE},
	{"underwater", SECT_UNDERWATER, TRUE},
	{"grassland", SECT_GRASSLAND, TRUE},
	{"tundra", SECT_TUNDRA, TRUE},
	{"swamp", SECT_SWAMP, TRUE},
	{"wasteland", SECT_WASTELAND, TRUE},
	{"water_ocean", SECT_WATER_OCEAN, TRUE}, 
	{"shore", SECT_SHORE, TRUE},
	{"underground", SECT_UNDERGROUND, TRUE},
	{NULL, 0, 0}
};
const struct flag_type tsector_flags[] = {
	{"default", TSECT_INSIDE, TRUE},
	{"city", TSECT_CITY, TRUE},
	{"field", TSECT_FIELD, TRUE},
	{"forest", TSECT_FOREST, TRUE},
	{"hills", TSECT_HILLS, TRUE},
	{"mountain", TSECT_MOUNTAIN, TRUE},
	{"water_crossable", TSECT_WATER_SWIM, TRUE},
	{"water_swim", TSECT_WATER_NOSWIM, TRUE},
	{"air", TSECT_AIR, TRUE},
	{"desert", TSECT_DESERT, TRUE},
	{"underwater", TSECT_UNDERWATER, TRUE},
	{"grassland", TSECT_GRASSLAND, TRUE},
	{"tundra", TSECT_TUNDRA, TRUE},
	{"swamp", TSECT_SWAMP, TRUE},
	{"wasteland", TSECT_WASTELAND, TRUE},
	{"water_ocean", TSECT_WATER_OCEAN, TRUE}, 
	{"shore", TSECT_SHORE, TRUE},
	{"underground", TSECT_UNDERGROUND, TRUE},
	{NULL, 0, 0}
};
const struct flag_type pcrace_flags[] = {
	{"human", RACE_HUMAN, TRUE},
	{"dwarf", RACE_DWARF, TRUE},
	{"vro'ath", RACE_VROATH, TRUE},
	{"elf", RACE_ELF, TRUE},
	{"canthi", RACE_CANTHI, TRUE},
	{"syvin", RACE_SYVIN, TRUE},
	{"sidhe", RACE_SIDHE, TRUE},
	{"avatar", RACE_AVATAR, TRUE}, 
	{"litan", RACE_LITAN, TRUE},
	{"kalian", RACE_KALIAN, TRUE},
	{"lich", RACE_LICH, TRUE},
	{NULL, 0, 0}
};
const struct flag_type Class_flags[] = {
	{"mage", CLASS_MAGE, TRUE},
	{"cleric", CLASS_CLERIC, TRUE},
	{"thief", CLASS_THIEF, TRUE},
	{"warrior", CLASS_WARRIOR, TRUE},
	{"ranger", CLASS_RANGER, TRUE},
	{"bard", CLASS_BARD, TRUE},
	{"paladin", CLASS_PALADIN, TRUE},
	{"assassin", CLASS_ASSASSIN, TRUE},
	{"reaver", CLASS_REAVER, TRUE},
	{"monk", CLASS_MONK, TRUE},
	{"necromancer", CLASS_NECROMANCER, TRUE},
	{"swarm", CLASS_SWARM, FALSE},
	{"golem", CLASS_GOLEM, FALSE},
	{"chaos", CLASS_CHAOS_JESTER,TRUE},
	{"adventurer", CLASS_ADVENTURER, FALSE},
	{"druid", CLASS_DRUID, FALSE},
	{"hero", CLASS_HERO, TRUE}, 
	{NULL, 0, 0}
};
const struct flag_type clan_flags[] = {
	{"None", CLAN_NONE, TRUE},
	{"Alliant", CLAN_ALL, TRUE},
	{"Quicksand", CLAN_QUI, TRUE},
	{"Twilight", CLAN_TWI, TRUE},
	{"Clubcrazed", CLAN_CLU, TRUE},
	{"Greenwood", CLAN_GRE, TRUE},
	{"Brotherhood", CLAN_BRO, TRUE},
	{"Pirates", CLAN_PIR, TRUE},
	{"Jhereg", CLAN_JHE, TRUE}, 
	{"Forsaken", CLAN_FOR, TRUE},
	{"Varruk", CLAN_VAR, TRUE},
	{"Vlades", CLAN_VLA, TRUE},
	{"Fools", CLAN_FOO, TRUE},
	{"Escargot", CLAN_ESC, TRUE},
	{"Loner", CLAN_LONER, TRUE},
	{"Parlous", CLAN_PAR, TRUE},
	{"Guttersnipes", CLAN_GUT, TRUE},
	{"Minions", CLAN_MIN, TRUE},
	{NULL, 0, 0}
};
const struct flag_type type_flags[] = {
	{"light", ITEM_LIGHT, TRUE},
	{"scroll", ITEM_SCROLL, TRUE},
	{"wand", ITEM_WAND, TRUE},
	{"staff", ITEM_STAFF, TRUE},
	{"saddle", ITEM_SADDLE, TRUE},
	{"weapon", ITEM_WEAPON, TRUE},
	{"treasure", ITEM_TREASURE, TRUE},
	{"armor", ITEM_ARMOR, TRUE},
	{"potion", ITEM_POTION, TRUE},
	{"furniture", ITEM_FURNITURE, TRUE},
	{"trash", ITEM_TRASH, TRUE},
	{"container", ITEM_CONTAINER, TRUE},
	{"clan_donation", ITEM_CLAN_DONATION, TRUE},
	{"newclan_donation", ITEM_NEWCLANS_DBOX, TRUE},
	{"player_box", ITEM_PLAYER_DONATION, TRUE},
	{"drink_container", ITEM_DRINK_CON, TRUE},
	{"elevator", ITEM_ELEVATOR, TRUE},
	{"ebutton", ITEM_EBUTTON, TRUE},
	{"raft", ITEM_RAFT, TRUE},
	{"stransport", ITEM_STRANSPORT, TRUE},
	{"ctransport", ITEM_CTRANSPORT, TRUE},
	{"key", ITEM_KEY, TRUE},
	{"food", ITEM_FOOD, TRUE},
	{"money", ITEM_MONEY, TRUE},
	{"npc_corpse", ITEM_CORPSE_NPC, TRUE},
	{"pc_corpse", ITEM_CORPSE_PC, FALSE},
	{"fountain", ITEM_FOUNTAIN, TRUE},
	{"pill", ITEM_PILL, TRUE},
	{"clothing", ITEM_CLOTHING, TRUE},
	{"protect", ITEM_PROTECT, TRUE},
	{"map", ITEM_MAP, TRUE},
	{"portal", ITEM_PORTAL, TRUE},
	{"portal_book", ITEM_PORTAL_BOOK, TRUE},
	{"warp_stone", ITEM_WARP_STONE, TRUE},
	{"room_key", ITEM_ROOM_KEY, TRUE},
	{"gem", ITEM_GEM, TRUE},
	{"jewelry", ITEM_JEWELRY, TRUE},
	{"jukebox", ITEM_JUKEBOX, TRUE},
	{"gills", ITEM_GILLS, TRUE},
	{"pack", ITEM_PACK, TRUE},
	{"vial", ITEM_VIAL, TRUE},
	{"parchment", ITEM_PARCHMENT, TRUE},
	{"instrument", ITEM_INSTRUMENT, TRUE},
	{"writing_instrument", ITEM_WRITING_INSTRUMENT, TRUE},
	{"writing_paper", ITEM_WRITING_PAPER, TRUE},
	{"ctransport_key", ITEM_CTRANSPORT_KEY, TRUE},
	{"quiver", ITEM_QUIVER, TRUE},
	{"feather", ITEM_FEATHER, TRUE},
	{"pole", ITEM_POLE, TRUE}, 
	{"bait", ITEM_BAIT, TRUE}, 
	{"tree", ITEM_TREE, TRUE},
	{"wood", ITEM_WOOD, TRUE},
	{"seed", ITEM_SEED, TRUE},
	{"wooden_in_construction",ITEM_WOODEN_INCONSTRUCTION,TRUE},
	{"card",ITEM_CARD,TRUE},
	{"binder",ITEM_BINDER,TRUE},
	{"objecttrap",ITEM_OBJ_TRAP,TRUE},
	{"portaltrap",ITEM_PORTAL_TRAP,TRUE},
	{"roomtrap",ITEM_ROOM_TRAP,TRUE},
	{"random",ITEM_RANDOM,TRUE},
	{NULL, 0, 0}
};
const struct flag_type rarity_flags[] = {
	{"very rare", RARITY_VERY_RARE, TRUE},
	{"rare", RARITY_RARE, TRUE},
	{"uncommon", RARITY_UNCOMMON, TRUE},
	{"somewhat common", RARITY_SOMEWHAT_COMMON, TRUE},
	{"common", RARITY_COMMON, TRUE},
	{"always", RARITY_ALWAYS, TRUE}, {NULL, 0, 0}
};
/*const struct flag_type extra_flags[MAX_EXTRA_FLAGS][31] = {
{
{"glow", ITEM_GLOW, TRUE},
{"hum", ITEM_HUM, TRUE},
{"dark", ITEM_DARK, TRUE},
{"lock", ITEM_LOCK, TRUE},
{"evil", ITEM_EVIL, TRUE},
{"invis", ITEM_INVIS, TRUE},
{"magic", ITEM_MAGIC, TRUE},
{"nodrop", ITEM_NODROP, TRUE},
{"bless", ITEM_BLESS, TRUE},
{"anti-good", ITEM_ANTI_GOOD, TRUE},
{"anti-evil", ITEM_ANTI_EVIL, TRUE},
{"anti-neutral", ITEM_ANTI_NEUTRAL, TRUE},
{"noremove", ITEM_NOREMOVE, TRUE},
{"inventory", ITEM_INVENTORY, TRUE},
{"nopurge", ITEM_NOPURGE, TRUE},
{"rot-death", ITEM_ROT_DEATH, TRUE},
{"vis-death", ITEM_VIS_DEATH, TRUE},
{"nosac", ITEM_NOSAC, TRUE},
{"nonmetal", ITEM_NONMETAL, TRUE},
{"nolocate", ITEM_NOLOCATE, TRUE},
{"melt_drop", ITEM_MELT_DROP, TRUE},
{"had_timer", ITEM_HAD_TIMER, TRUE},
{"sell_extract", ITEM_SELL_EXTRACT, TRUE},
{"burn_proof", ITEM_BURN_PROOF, TRUE},
{"nouncurse", ITEM_NOUNCURSE, TRUE},
{"noidentify", ITEM_NOIDENTIFY, TRUE},
{"nosave", ITEM_NOSAVE, TRUE}, 
{"nomob", ITEM_NOMOB, TRUE},
{"noenchant", ITEM_NOENCHANT, TRUE},
{NULL, 0, 0},
},
{
{"nostock", ITEM_NOSTOCK, TRUE},
{"nodonate", ITEM_NODONATE, TRUE},
{"noauction", ITEM_NOAUCTION, TRUE},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
{NULL, 0, 0},
}
};*/
const struct flag_type wear_flags[] = {
	{"take", ITEM_TAKE, TRUE},
	{"finger", ITEM_WEAR_FINGER, TRUE},
	{"neck", ITEM_WEAR_NECK, TRUE},
	{"body", ITEM_WEAR_BODY, TRUE},
	{"head", ITEM_WEAR_HEAD, TRUE},
	{"legs", ITEM_WEAR_LEGS, TRUE},
	{"feet", ITEM_WEAR_FEET, TRUE},
	{"hands", ITEM_WEAR_HANDS, TRUE},
	{"arms", ITEM_WEAR_ARMS, TRUE},
	{"shield", ITEM_WEAR_SHIELD, TRUE},
	{"about", ITEM_WEAR_ABOUT, TRUE},
	{"waist", ITEM_WEAR_WAIST, TRUE},
	{"wrist", ITEM_WEAR_WRIST, TRUE},
	{"wield", ITEM_WIELD, TRUE},
	{"hold", ITEM_HOLD, TRUE},
	{"two-hands", ITEM_TWO_HANDS, TRUE},
	{"nosac", ITEM_NO_SAC, TRUE},
	{"wear-float", ITEM_WEAR_FLOAT, TRUE}, 
	{"another_finger", ITEM_WEAR_ANOTHER_FINGER, TRUE},
	{"another_neck", ITEM_WEAR_ANOTHER_NECK, TRUE},
	{"another_wrist", ITEM_WEAR_ANOTHER_WRIST, TRUE},
	{"light", ITEM_WEAR_LIGHT, TRUE},
	{"tattoo", ITEM_WEAR_TATTOO, TRUE},
	{"face", ITEM_WEAR_FACE, TRUE},
	{"ears", ITEM_WEAR_EARS, TRUE},
	{"crest", ITEM_WEAR_CREST, TRUE},
	{"clan_mark", ITEM_WEAR_CLAN_MARK, TRUE},
	{"wings", ITEM_WORN_WINGS, TRUE},
	{NULL, 0, 0}
};
extern const struct flag_type act_flags[];
extern const struct flag_type act_flags2[];
extern const struct flag_type affect_flags[];
extern const struct flag_type recruit_flags[];

/*
* Used when adding an affect to tell where it goes.
* See addaffect and delaffect in act_olc.c
*/
const struct flag_type apply_flags[] = {
	{"none", APPLY_NONE, TRUE},
	{"strength", APPLY_STR, TRUE},
	{"dexterity", APPLY_DEX, TRUE},
	{"intelligence", APPLY_INT, TRUE},
	{"wisdom", APPLY_WIS, TRUE},
	{"constitution", APPLY_CON, TRUE},
	{"charisma", APPLY_CHA, TRUE},
	{"sex", APPLY_SEX, TRUE}, {"Class", APPLY_CLASS, TRUE},
	/*  {"level", APPLY_LEVEL, TRUE}, 
	{"age", APPLY_AGE, TRUE}, 
	{"height", APPLY_HEIGHT, TRUE},
	{"weight", APPLY_WEIGHT, TRUE}, */
	{"mana", APPLY_MANA, TRUE},
	{"hp", APPLY_HIT, TRUE}, {"move", APPLY_MOVE, TRUE},
	/*  {"gold", APPLY_GOLD, TRUE}, */
	/*  {"experience", APPLY_EXP, TRUE}, */
	{"ac", APPLY_AC, TRUE},
	{"hitroll", APPLY_HITROLL, TRUE},
	{"damroll", APPLY_DAMROLL, TRUE},
	{"saving-throw", APPLY_SAVING_SPELL, TRUE},
	/*  {"saving-rod", APPLY_SAVING_ROD, TRUE},
	{"saving-petri", APPLY_SAVING_PETRI, TRUE},
	{"saving-breath", APPLY_SAVING_BREATH, TRUE},
	{"saving-spell", APPLY_SAVING_SPELL, TRUE},  */
	/*  {"spell-affect", APPLY_SPELL_AFFECT, TRUE}, */
	{NULL, 0, 0}
};

/*
* What is seen.
*/
const struct flag_type wear_loc_strings[] = {
	{"in the inventory", WEAR_NONE, TRUE},
	{"as a light", WEAR_LIGHT, TRUE},
	{"on the left finger", WEAR_FINGER_L, TRUE},
	{"on the right finger", WEAR_FINGER_R, TRUE},
	{"around the neck (1)", WEAR_NECK_1, TRUE},
	{"around the neck (2)", WEAR_NECK_2, TRUE},
	{"on the body", WEAR_BODY, TRUE},
	{"over the head", WEAR_HEAD, TRUE},
	{"on the legs", WEAR_LEGS, TRUE},
	{"on the feet", WEAR_FEET, TRUE},
	{"on the hands", WEAR_HANDS, TRUE},
	{"on the arms", WEAR_ARMS, TRUE},
	{"as a shield", WEAR_SHIELD, TRUE},
	{"about the shoulders", WEAR_ABOUT, TRUE},
	{"around the waist", WEAR_WAIST, TRUE},
	{"on the left wrist", WEAR_WRIST_L, TRUE},
	{"on the right wrist", WEAR_WRIST_R, TRUE},
	{"wielded right", WEAR_WIELD_R, TRUE},
	{"held in a hand", WEAR_HOLD, TRUE},
	{"used as a floatation device", WEAR_FLOAT, TRUE},
	{"wielded left", WEAR_WIELD_L, TRUE},
	{"as a mark of a clan", WEAR_CLAN_MARK, TRUE},
	{"on your wings", WEAR_WORN_WINGS, TRUE},
	{NULL, 0}
};
const struct flag_type wear_loc_flags[] = {
	{"none", WEAR_NONE, TRUE},
	{"light", WEAR_LIGHT, TRUE},
	{"lfinger", WEAR_FINGER_L, TRUE},
	{"rfinger", WEAR_FINGER_R, TRUE},
	{"neck1", WEAR_NECK_1, TRUE},
	{"neck2", WEAR_NECK_2, TRUE},
	{"body", WEAR_BODY, TRUE},
	{"head", WEAR_HEAD, TRUE},
	{"legs", WEAR_LEGS, TRUE},
	{"feet", WEAR_FEET, TRUE},
	{"hands", WEAR_HANDS, TRUE},
	{"arms", WEAR_ARMS, TRUE},
	{"shield", WEAR_SHIELD, TRUE},
	{"about", WEAR_ABOUT, TRUE},
	{"waist", WEAR_WAIST, TRUE},
	{"lwrist", WEAR_WRIST_L, TRUE},
	{"rwrist", WEAR_WRIST_R, TRUE},
	{"rwield", WEAR_WIELD_R, TRUE},
	{"hold", WEAR_HOLD, TRUE},
	{"float", WEAR_FLOAT, TRUE},
	{"lwield", WEAR_WIELD_L, TRUE},
	{"clanmark", WEAR_CLAN_MARK, TRUE},
	{"wings", WEAR_WORN_WINGS, TRUE}, {NULL, 0, 0}
};

// Increment MAX_WEAPON_FLAGS in merc.h if you add to this.
const struct flag_type weapon_flags[] = {
	{"hit", 0, TRUE},
	{"slice", 1, TRUE},
	{"stab", 2, TRUE},
	{"slash", 3, TRUE},
	{"whip", 4, TRUE},
	{"claw", 5, TRUE},
	{"blast", 6, TRUE},
	{"pound", 7, TRUE},
	{"crush", 8, TRUE},
	{"grep", 9, TRUE},
	{"bite", 10, TRUE},
	{"pierce", 11, TRUE},
	{"suction", 12, TRUE}, {"beating", 13, TRUE},	/* ROM */
	{"digestion", 14, TRUE},
	{"charge", 15, TRUE},
	{"slap", 16, TRUE},
	{"punch", 17, TRUE},
	{"wrath", 18, TRUE},
	{"magic", 19, TRUE},
	{"divine", 20, TRUE},
	{"cleave", 21, TRUE},
	{"scratch", 22, TRUE},
	{"peck", 23, TRUE},
	{"peckb", 24, TRUE},
	{"chop", 25, TRUE},
	{"sting", 26, TRUE},
	{"smash", 27, TRUE},
	{"shbite", 28, TRUE},
	{"flbite", 29, TRUE},
	{"frbite", 30, TRUE},
	{"acbite", 31, TRUE},
	{"chomp", 32, TRUE},
	{"drain", 33, TRUE},
	{"thrust", 34, TRUE},
	{"slime", 35, TRUE},
	{"shock", 36, TRUE},
	{"thwack", 37, TRUE},
	{"flame", 38, TRUE}, 
	{"chill", 39, TRUE}, 
	{"vbite", 40, TRUE},
	{"lclaw", 41, TRUE},
	{"breath", 42, TRUE},
	{"spit", 43, TRUE},
	{"splash",44,TRUE},
	{"wslash",45,TRUE},
	{NULL, 0, 0}
};
const struct flag_type container_flags[] = {
	{"closeable", 1, TRUE},
	{"pickproof", 2, TRUE},
	{"closed", 4, TRUE},
	{"locked", 8, TRUE}, {"nobash", 32, TRUE}, {NULL, 0, 0}
};
const struct flag_type liquid_flags[] = {
	{"water", 0, TRUE},
	{"beer", 1, TRUE},
	{"wine", 2, TRUE},
	{"ale", 3, TRUE},
	{"dark-ale", 4, TRUE},
	{"whisky", 5, TRUE},
	{"lemonade", 6, TRUE},
	{"firebreather", 7, TRUE},
	{"local-specialty", 8, TRUE},
	{"slime-mold-juice", 9, TRUE},
	{"milk", 10, TRUE},
	{"tea", 11, TRUE},
	{"coffee", 12, TRUE},
	{"blood", 13, TRUE},
	{"salt-water", 14, TRUE},
	{"cola", 15, TRUE},
	{"root-beer", 16, TRUE},
	{"elvish-wine", 17, TRUE},
	{"white-wine", 18, TRUE},
	{"champagne", 19, TRUE},
	{"mead", 20, TRUE},
	{"rose-wine", 21, TRUE},
	{"benedictine-wine", 22, TRUE},
	{"vodka", 23, TRUE},
	{"cranberry-juice", 24, TRUE},
	{"orange-juice", 25, TRUE},
	{"absinthe", 26, TRUE},
	{"brandy", 27, TRUE},
	{"aquavit", 28, TRUE},
	{"schnapps", 29, TRUE},
	{"icewine", 30, TRUE},
	{"amontillado", 31, TRUE},
	{"sherry", 32, TRUE},
	{"framboise", 33, TRUE},
	{"rum", 34, TRUE}, {"cordial", 35, TRUE}, {NULL, 0, 0}
};
extern const struct flag_type form_flags[];
extern const struct flag_type part_flags[];
const struct flag_type ac_type[] = {
	{"pierce", AC_PIERCE, TRUE},
	{"bash", AC_BASH, TRUE},
	{"slash", AC_SLASH, TRUE},
	{"exotic", AC_EXOTIC, TRUE}, {NULL, 0, 0}
};
const struct flag_type size_flags[] = {
	{"tiny", SIZE_TINY, TRUE},
	{"small", SIZE_SMALL, TRUE},
	{"medium", SIZE_MEDIUM, TRUE},
	{"large", SIZE_LARGE, TRUE},
	{"huge", SIZE_HUGE, TRUE},
	{"giant", SIZE_GIANT, TRUE}, {NULL, 0, 0},
};
const struct flag_type furniture_type[] = {
	{"stand_at", A, TRUE},
	{"stand_on", B, TRUE},
	{"stand_in", C, TRUE},
	{"sit_at", D, TRUE},
	{"sit_on", E, TRUE},
	{"sit_in", F, TRUE},
	{"rest_at", G, TRUE},
	{"rest_on", H, TRUE},
	{"rest_in", I, TRUE},
	{"sleep_at", J, TRUE},
	{"sleep_on", K, TRUE},
	{"sleep_in", L, TRUE},
	{"put_at", M, TRUE},
	{"put_on", N, TRUE},
	{"put_in", O, TRUE}, {"put_inside", P, TRUE}, {NULL, 0, 0}
};
const struct flag_type instrument_flags[] = {
	{"two-handed", A, TRUE}, {NULL, 0, 0}
};
const struct flag_type weapon_Class[] = {
	{"exotic", 0, TRUE},
	{"sword", 1, TRUE},
	{"dagger", 2, TRUE},
	{"staff", 3, TRUE},
	{"spear", 3, TRUE},
	{"mace", 4, TRUE},
	{"axe", 5, TRUE},
	{"flail", 6, TRUE},
	{"polearm", 7, TRUE},
	{"shortbow", 8, TRUE},
	{"longbow", 9, TRUE},
	{"arrow", 10, TRUE}, 
	{"arrowhead", 11, TRUE}, 
	{"dice", WEAPON_DICE, TRUE},
	{NULL, 0, 0}
};

const struct flag_type weapons_type[] = {
	{"none", 0, TRUE},
	{"flaming", WEAPON_FLAMING, TRUE},
	{"frost", WEAPON_FROST, TRUE},
	{"vampiric", WEAPON_VAMPIRIC, TRUE},
	{"sharp", WEAPON_SHARP, TRUE},
	{"vorpal", WEAPON_VORPAL, TRUE},
	{"two-hands", WEAPON_TWO_HANDS, TRUE},
	{"shocking", WEAPON_SHOCKING, TRUE},
	{"poison", WEAPON_POISON, TRUE},
	{"lance", WEAPON_LANCE, TRUE},
	{"throwing", WEAPON_THROWING, TRUE}, 
	{"apathy", WEAPON_APATHY, FALSE},
	{"windslash", WEAPON_WINDSLASH, TRUE},
	{"larva", WEAPON_LARVA, TRUE},
	{"holy", WEAPON_HOLY, TRUE},
	//  {"holy_activated",WEAPON_HOLY_ACTIVATE, FALSE},
	{NULL, 0, 0}
};
extern const struct flag_type off_flags[];
extern const struct flag_type imm_flags[];
const struct flag_type res_flags[] = {
	{"summon", RES_SUMMON, TRUE},
	{"charm", RES_CHARM, TRUE},
	{"magic", RES_MAGIC, TRUE},
	{"weapon", RES_WEAPON, TRUE},
	{"bash", RES_BASH, TRUE},
	{"pierce", RES_PIERCE, TRUE},
	{"slash", RES_SLASH, TRUE},
	{"fire", RES_FIRE, TRUE},
	{"cold", RES_COLD, TRUE},
	{"lightning", RES_LIGHTNING, TRUE},
	{"acid", RES_ACID, TRUE},
	{"poison", RES_POISON, TRUE},
	{"negative", RES_NEGATIVE, TRUE},
	{"holy", RES_HOLY, TRUE},
	{"energy", RES_ENERGY, TRUE},
	{"mental", RES_MENTAL, TRUE},
	{"disease", RES_DISEASE, TRUE},
	{"drowning", RES_DROWNING, TRUE},
	{"light", RES_LIGHT, TRUE},
	{"sound", RES_SOUND, TRUE},
	{"wood", RES_WOOD, TRUE},
	{"silver", RES_SILVER, TRUE},
	{"iron", RES_IRON, TRUE}, 
	{"wind", RES_WIND, TRUE},
	{NULL, 0, 0}
};
const struct flag_type vuln_flags[] = {
	{"summon", VULN_SUMMON, TRUE},
	{"charm", VULN_CHARM, TRUE},
	{"magic", VULN_MAGIC, TRUE},
	{"weapon", VULN_WEAPON, TRUE},
	{"bash", VULN_BASH, TRUE},
	{"pierce", VULN_PIERCE, TRUE},
	{"slash", VULN_SLASH, TRUE},
	{"fire", VULN_FIRE, TRUE},
	{"cold", VULN_COLD, TRUE},
	{"lightning", VULN_LIGHTNING, TRUE},
	{"acid", VULN_ACID, TRUE},
	{"poison", VULN_POISON, TRUE},
	{"negative", VULN_NEGATIVE, TRUE},
	{"holy", VULN_HOLY, TRUE},
	{"energy", VULN_ENERGY, TRUE},
	{"mental", VULN_MENTAL, TRUE},
	{"disease", VULN_DISEASE, TRUE},
	{"drowning", VULN_DROWNING, TRUE},
	{"light", VULN_LIGHT, TRUE},
	{"sound", VULN_SOUND, TRUE},
	{"wood", VULN_WOOD, TRUE},
	{"silver", VULN_SILVER, TRUE},
	{"iron", VULN_IRON, TRUE}, 
	{"wind", VULN_WIND, TRUE},
	{NULL, 0, 0}
};
const struct flag_type material_type[] =	/* not yet implemented */
{ {"wood", 0, TRUE}, {"iron", 0, TRUE}, {"steel", 0, TRUE}, {NULL, 0, 0}
};
const struct flag_type position_flags[] = {
	{"dead", POS_DEAD, FALSE},
	{"mortal", POS_MORTAL, FALSE},
	{"incap", POS_INCAP, FALSE},
	{"stunned", POS_STUNNED, FALSE},
	{"sleeping", POS_SLEEPING, TRUE},
	{"resting", POS_RESTING, TRUE},
	{"sitting", POS_SITTING, TRUE},
	{"fighting", POS_FIGHTING, FALSE},
	{"standing", POS_STANDING, TRUE}, 
	{"tethered", POS_TETHERED, TRUE},
	{"mounted", POS_MOUNTED, FALSE},
	{"feigning death", POS_FEIGNING_DEATH, FALSE},
	{"comatose", POS_COMA, TRUE},
	{NULL, 0, 0}
};

const struct flag_type ot_extra_flags[] = {
	{"worn", OT_EXTRA_WHEN_WORN, TRUE},
	{"carried", OT_EXTRA_WHEN_CARRIED, TRUE},
	{"on_ground", OT_EXTRA_WHEN_ON_GROUND, TRUE}, 
	{"triggerer_worn", OT_EXTRA_WHEN_TRIGGERER_WORN, TRUE},
	{"triggerer_carried", OT_EXTRA_WHEN_TRIGGERER_CARRIED, TRUE},
	{"on_npc",OT_EXTRA_ON_NPC, TRUE},
	{"on_pc",OT_EXTRA_ON_PC, TRUE},
	{NULL, 0 ,0}
};


void do_oset (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	//  char buf[MAX_INPUT_LENGTH];
	OBJ_DATA *obj = NULL;
	int value;
	smash_tilde (argument);
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	strcpy (arg3, argument);
	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
	{
		send_to_char ("Syntax:\n\r", ch);
		send_to_char ("  set obj <object> <field> <value>\n\r", ch);
		send_to_char ("  Field being one of:\n\r", ch);
		send_to_char ("    value0 value1 value2 value3 value4 (v1-v4)\n\r", ch);
		send_to_char ("    value5 value6 value7 value8 (v5-v8)\n\r", ch);
		send_to_char ("    value9 value10 value11 value12 (v9-v12)\n\r", ch);
		send_to_char ("    extra wear level weight cost timer owner\n\r", ch);
		send_to_char ("    objtrigger\n\r",ch);
		return;
	}
	if (IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_FAKIE)
	{
		if (GLOBAL_ot_obj && is_name (arg1, GLOBAL_ot_obj->name))
			obj = GLOBAL_ot_obj;
		else if (GLOBAL_ot_ch)
			obj = get_obj_here (GLOBAL_ot_ch, arg1);
	}
	// if (!strcmp(arg1,"stiletto"))
	//        do_echo(char_list,"!SSSSS!");
	if (IS_NPC(ch))
	{
		obj = get_obj_here (ch, arg1);
	}
	if (obj == NULL)
	{
		if ((obj = get_obj_world (ch, arg1)) == NULL)
		{
			send_to_char ("Nothing like that in heaven or earth.\n\r", ch);
			return;
		}
	}
	/*
	* Snarf the value (which need not be numeric).
	*/

	//Shinowlan -- switch from atoi() to atol() to support flag values
	// above 65536
	value = atol (arg3);

	/*
	* Set something.
	*/
	if (!str_cmp (arg2, "value0") || !str_cmp (arg2, "v0"))
	{
		obj->value[0] = value;
		return;
	}
	if (!str_cmp (arg2, "value1") || !str_cmp (arg2, "v1"))
	{
		obj->value[1] = value;
		return;
	}
	if (!str_cmp (arg2, "value2") || !str_cmp (arg2, "v2"))
	{
		obj->value[2] = value;
		return;
	}
	if (!str_cmp (arg2, "value3") || !str_cmp (arg2, "v3"))
	{
		obj->value[3] = value;
		return;
	}
	if (!str_cmp (arg2, "value4") || !str_cmp (arg2, "v4"))
	{
		obj->value[4] = value;
		if (obj->item_type == ITEM_WEAPON && obj->value[0] == WEAPON_DICE)
		{
			if (obj->value[4] & WEAPON_SHARP)
				obj->value[4] -= WEAPON_SHARP;
		}
		return;
	}
	if (!str_cmp (arg2, "value5") || !str_cmp (arg2, "v5"))
	{
		obj->value[5] = value;
		if (obj->item_type == ITEM_WEAPON && obj->value[0] == WEAPON_DICE)
		{
			if (value < 0 || value > MAX_SKILL || skill_table[value].name == NULL
				|| skill_table[value].type == SKILL_NORMAL)
				obj->value[5] = 0;
		}
		return;
	}
	if (!str_cmp (arg2, "value6") || !str_cmp (arg2, "v6"))
	{
		obj->value[6] = value;
		return;
	}
	if (!str_cmp (arg2, "value7") || !str_cmp (arg2, "v7"))
	{
		obj->value[7] = value;
		return;
	}
	if (!str_cmp (arg2, "value8") || !str_cmp (arg2, "v8"))
	{
		obj->value[8] = value;
		return;
	}
	if (!str_cmp (arg2, "value9") || !str_cmp (arg2, "v9"))
	{
		obj->value[9] = value;
		return;
	}
	if (!str_cmp (arg2, "value10") || !str_cmp (arg2, "v10"))
	{
		obj->value[10] = value;
		return;
	}
	if (!str_cmp (arg2, "value11") || !str_cmp (arg2, "v11"))
	{
		obj->value[11] = value;
		return;
	}
	if (!str_cmp (arg2, "value12") || !str_cmp (arg2, "v12"))
	{
		obj->value[12] = value;
		return;
	}
	if (!str_prefix (arg2, "extra"))
	{
		if (!is_number (arg3))
		{
			int i;
			bool found = FALSE;
			// these are flags such as glow, hum, etc.

			for (i = 0; i < MAX_EXTRA_FLAGS;i++)
			{
				if ((value = flag_value (extra_flags[i], arg3)) != NO_FLAG)
				{
					TOGGLE_BIT (obj->extra_flags[i], value);
					//              send_to_char ("Extra flag toggled.\n\r", ch);
					found = TRUE;
					//          return TRUE;
				}
			}
			if (found)
			{
				send_to_char ("Extra flag toggled.\n\r", ch);
				return;
			}
			send_to_char ("No such extra bit.\n\r", ch);
			return;               //return if flags are bogus
		}

		else
		{
			obj->extra_flags[0] = value;
			send_to_char("Extra flags set.  Remember this only sets the original extra flag.  Use names to flag them all.\n\r",ch);
			return;
		}
	}
	if (!str_prefix (arg2, "wear"))
	{
		obj->wear_flags = value;
		return;
	}
	if (!str_prefix (arg2, "level"))
	{
		obj->level = value;
		return;
	}
	if (!str_prefix (arg2, "weight"))
	{
		obj->weight = value;
		return;
	}
	if (!str_prefix (arg2, "cost"))
	{
		obj->cost = value;
		return;
	}
	if (!str_prefix (arg2, "timer"))
	{
		if (obj == NULL)
		{
			bug("obj null in set obj name timer %d",value);
			return;
		}
		obj->timer = value;
		return;
	}
	// Adeon 6/29/03 -- set the objects owner
	if (!str_prefix (arg2, "owner"))
	{
		if (!is_number (arg3))
		{
			//              sprintf(buf,"!%s!",arg3);
			//              do_echo(char_list,buf);
			//one_argument(argument,arg3);
			if (!str_cmp (arg3, "none"))
			{
				if (obj->plr_owner)
					free_string (obj->plr_owner);
				obj->plr_owner = NULL;
			}

			else
			{
				if (obj->plr_owner)
					free_string (obj->plr_owner);
				obj->plr_owner = str_dup (arg3);
			}
		}
		return;
	}
	if (!str_prefix (arg2, "objtrigger"))
	{
		int i=0;//,k=0;
		for (i=0;i<MAX_OBJ_TRIGS;i++)
		{
			//            sprintf(buf,"name of item is -> %s\n\r",obj->name);
			//            send_to_char(buf,ch);
			if (obj->objtrig[i])
			{
				if (obj->objtrig[i]->pIndexData->vnum == value)
				{
					free_ot(obj->objtrig[i]);
					//          sprintf(buf,"removing obj trigger %d with vnum %d\n\r",i,value);
					//          send_to_char(buf,ch);
					/*          for (k=i;k<MAX_OBJ_TRIGS-1;k++)
					{
					send_to_char("! ",ch);
					obj->objtrig[k] = obj->objtrig[k+1];
					if (obj->objtrig[k])
					{
					sprintf(buf,"moving %d to i -> %d \n\r",obj->objtrig[k]->pIndexData->vnum,i);
					send_to_char(buf,ch);
					}
					}
					obj->objtrig[MAX_OBJ_TRIGS-1] = NULL;*/

					send_to_char("Object trigger successfully removed.\n\r",ch);
					return;
				}
			}
			else break;
		}
		if (i<MAX_OBJ_TRIGS)
		{
			if (get_obj_trig(value))
			{
				obj->objtrig[i] = new_ot(value);
				obj->objtrig[i]->obj_on = obj;
			}
			else send_to_char("That object trigger vnum does not exist.\n\r",ch);
		}
		else send_to_char("There are too many object triggers on that item already.  Delete one first.\n\r",ch)
			;
		return;
	}

	/*
	* Generate usage message.
	*/
	do_oset (ch, "");
	return;
}

// ROM object functions :
//
// Moved out of oedit() due to naming conflicts -- Hugin
OEDIT (oedit_extra)
{
	OBJ_INDEX_DATA *pObj;
	int value,i;
	bool found=FALSE;
	if (argument[0] != '\0')
	{
		EDIT_OBJ (ch, pObj);
		for (i = 0; i < MAX_EXTRA_FLAGS;i++)
		{
			if ((value = flag_value (extra_flags[i], argument)) != NO_FLAG)
			{
				TOGGLE_BIT (pObj->extra_flags[i], value);
				//          send_to_char ("Extra flag toggled.\n\r", ch);
				found = TRUE;
				//          return TRUE;
			}
		}
		if (found)
		{
			send_to_char ("Extra flag toggled.\n\r", ch);
			return TRUE;
		}
	}
	send_to_char ("Syntax:  extra [flag]\n\r"
		"Type '? extra' for a list of flags.\n\r", ch);
	return FALSE;
}


/*
* This table contains help commands and a brief description of each.
* ------------------------------------------------------------------
*/
const struct olc_help_type help_table[] = {
	{"area", area_flags, "Area attributes."},
	{"room", room_flags, "Room attributes."},
	{"room2", room_flags2, "Room attributes2."},
	{"Class", Class_flags, "Classes available."},
	{"pcrace", pcrace_flags, "PC Races available."},
	{"sector", sector_flags, "Sector types, terrain."},
	{"exit", exit_flags, "Exit types."},
	{"type", type_flags, "Types of objects."},
	{"extra", extra_flags[0], "Object attributes."},
	{"extra2", extra_flags[1], "More Object attributes."},
	{"wear", wear_flags, "Where to wear object."},
	{"spec", spec_table, "Available special programs."},
	{"sex", sex_flags, "Sexes."},
	{"act", act_flags, "Mobile attributes."},
	{"act2", act_flags2, "More Mobile attributes."},
	{"affect", affect_flags, "Mobile affects."},
	{"recruit", recruit_flags, "Mobile recruit options."},
	{"wear-loc", wear_loc_flags, "Where mobile wears object."},
	{"spells", skill_table, "Names of current spells."},
	{"weapon", weapon_flags, "Type of weapon."},
	{"container", container_flags, "Container status."},
	{"liquid", liquid_flags, "Types of liquids."},
	{"ac", ac_type, "Ac for different attacks."},
	{"form", form_flags, "Mobile body form."},
	{"part", part_flags, "Mobile body parts."},
	{"imm", imm_flags, "Mobile immunity."},
	{"res", res_flags, "Mobile resistance."},
	{"vuln", vuln_flags, "Mobile vlnerability."},
	{"off", off_flags, "Mobile offensive behaviour."},
	{"size", size_flags, "Mobile size."},
	{"position", position_flags, "Mobile positions."},
	{"material", material_type, "Material mob/obj is made from."},
	{"wClass", weapon_Class, "Weapon Class."},
	{"wtype", weapons_type, "Special weapon type."},
	{"ftype", furniture_type, "Furniture type."},
	{"actions", pact_flags, "Portal Action Commands."},
	{"gate", gate_flags, "Gate Flags."},
	{"apply", apply_flags, "Apply flags."},
	{"doors", exit_flags, "Type 'help doors' for information."},
	{"rarity", rarity_flags, "Object rarity flags."},
	{"button", button_flags, "Button flags."},
	{"clan",clan_flags, "Clan flags."},
	{"traptype",trap_type, "Trap types."},
	{"trapflag",trap_flag, "Trap flags."},
	{"dirname",dir_flags, "Direction names."},
	{NULL, 0, NULL}
};

//Iblis - 6/07/04 - For cards
const struct flag_type card_Class[] = {
	{"mob", CARD_MOB, TRUE},
	{"terrain", CARD_TERRAIN, TRUE},
	{"spell_heal", CARD_SPELL_HEAL, TRUE},
	{"spell_damage", CARD_SPELL_DAMAGE, TRUE},
	{"spell_boost", CARD_SPELL_BOOST, TRUE},
	{"spell_counter", CARD_SPELL_COUNTER, TRUE},
	{NULL, 0, 0}
};

//Iblis - 6/7/4 - For terrain cards
const struct flag_type card_terrain_type[] = {
	{"city", CARD_TERRAIN_CITY, TRUE},
	{"swamp", CARD_TERRAIN_SWAMP, TRUE},
	{"forest", CARD_TERRAIN_FOREST, TRUE},
	{"mountain", CARD_TERRAIN_MOUNTAIN, TRUE},
	{"sea", CARD_TERRAIN_SEA, TRUE},
	{"desert", CARD_TERRAIN_DESERT, TRUE},
	{"air", CARD_TERRAIN_AIR, TRUE},
	{"plains", CARD_TERRAIN_PLAINS, TRUE},
	{NULL, 0, 0}
};

const struct flag_type card_race_type[] = {
	{"dragon", CARD_RACE_DRAGON, TRUE},
	{"humanoid", CARD_RACE_HUMANOID, TRUE},
	{"monster", CARD_RACE_MONSTER, TRUE},
	{"bird", CARD_RACE_BIRD, TRUE},
	{"insect", CARD_RACE_INSECT, TRUE},
	{"animal", CARD_RACE_ANIMAL, TRUE},
	{"fish", CARD_RACE_FISH, TRUE},
	{"undead", CARD_RACE_UNDEAD, TRUE},
	{"magic", CARD_RACE_MAGIC, TRUE},
	{NULL,0,0}
};

const struct flag_type card_element_type[] = {
	{"none", CARD_ELEMENT_NONE, TRUE},
	{"lightning", CARD_ELEMENT_LIGHTNING, TRUE},
	{"water", CARD_ELEMENT_WATER, TRUE},
	{"fire", CARD_ELEMENT_FIRE, TRUE},
	{"stone", CARD_ELEMENT_STONE, TRUE},
	{NULL,0,0}
};


const struct flag_type card_terrain_toggles[] = {
	{"none", 0, TRUE},
	{"nodamage", CARD_TT_NODAMAGE, TRUE},
	{"noheal", CARD_TT_NOHEAL, TRUE},
	{"elementalplus", CARD_TT_ELEMENTALPLUS, TRUE},
	{"elementalminus", CARD_TT_ELEMENTALMINUS, TRUE},
	{"allplus", CARD_TT_ALLPLUS, TRUE},
	{"allminus", CARD_TT_ALLMINUS, TRUE},
	{"nocounter", CARD_TT_NOCOUNTER, TRUE},
	{"noboost", CARD_TT_NOBOOST, TRUE},
	{NULL,0,0}
};


const struct flag_type trap_type[] = {
	{"none", 0, TRUE},
	{"obj_plain",TRAP_O_PLAIN,TRUE},
	{"obj_spring_blade",TRAP_O_BLADE,TRUE},
	{"obj_spring_needle",TRAP_O_NEEDLE,TRUE},
	{"obj_jar_bees",TRAP_O_BEE,TRUE},
	{"obj_explosive",TRAP_O_EXPLODING,TRUE},
	{"obj_spell",TRAP_O_SPELL,TRUE},
	{"por_plain",TRAP_P_PLAIN,TRUE},
	{"por_falling_rock",TRAP_P_ROCK,TRUE},
	{"por_dart",TRAP_P_DART,TRUE},
	{"por_explosive",TRAP_P_EXPLODING,TRUE},
	{"por_wall_spikes",TRAP_P_SPIKES,TRUE},
	{"por_spell",TRAP_P_SPELL,TRUE},
	{"por_alarm",TRAP_P_ALARM,TRUE},
	{"room_plain",TRAP_R_PLAIN,TRUE},
	{"room_spike_pit",TRAP_R_SPIKEPIT,TRUE},
	{"room_log",TRAP_R_LOG,TRUE},
	{"room_landmine",TRAP_R_LANDMINE,TRUE},
	{"room_spiked_snare",TRAP_R_SPIKESNARE,TRUE},
	{"room_spell",TRAP_R_SPELL,TRUE},
	{"room_alarm",TRAP_R_ALARM,TRUE},
	{NULL,0,0}
};

const struct flag_type dir_flags[] = {
	{"north", DIR_NORTH, TRUE},
	{"east",DIR_EAST , TRUE},
	{"south", DIR_SOUTH, TRUE},
	{"west",DIR_WEST , TRUE},
	{"up",DIR_UP, TRUE},
	{"down",DIR_DOWN , TRUE},
	{NULL,0,0}
};

const struct flag_type trap_flag[] = {
	{"none", 0, TRUE},
	{"nodisarm", TFLAG_NODISARM, TRUE},
	{"unavoidable", TFLAG_UNAVOID, TRUE},
	{"nodetect", TFLAG_NODETECT, TRUE},
	{"area", TFLAG_AREA, TRUE},
	{"excellent", TFLAG_EXCELLENT, TRUE},
	{"poison", TFLAG_POISON, TRUE},
	{"knock", TFLAG_KNOCK, TRUE},
	{"snare", TFLAG_SNARE, TRUE},
	{"beeswarm", TFLAG_BEE, TRUE},
	{"disorient", TFLAG_DISORIENT, TRUE},
	{NULL,0,0}
};
