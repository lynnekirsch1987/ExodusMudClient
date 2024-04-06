//
// new_clans_util.h
//
// Programmer: Pulewai
// Date: 7/10/98
// Version: 1.0
//
// This contains some public utility functions that support the
// new clan code

#ifndef __new_clans_util_h__
#define __new_clans_util_h__

//
// clan diplomacy checks
//
bool has_defeated_clan (int ident1, int ident2);
bool clan_has_allies (int slot);
bool is_requested_war (int clan1, int clan2);
bool is_warring (int clan1, int clan2);
bool is_requested_ally (int clan1, int clan2);
bool is_ally (int clan1, int clan2);
bool is_requested_enemy (int clan1, int clan2);
bool is_enemy (int clan1, int clan2);
bool clan_is_warring (int clan);

void clanwar_kill (CHAR_DATA * ch, CHAR_DATA * victim);

int xp_adjust (CHAR_DATA * ch, int xp);
int count_clan_allies (int clan);
int count_clan_enemies (int clan);
int clan_levels (int slot);

//
// some string ops that are useful
//
int color_strlen (char *str);
char *strip_whitespace (char *str);
char *find_sepchar (char *str);
bool limited_copy_str (char *str1, char *str2, int len);
void remove_spaces (char *str);

//
// some simple lookups
//
int clanname_to_slot (const char *name);
int clanident_to_slot (int ident);
char *get_clan_symbol_ch (CHAR_DATA * ch);
char *get_clan_symbol (int slot);
int get_clan_hall_ch (CHAR_DATA * ch);
int get_clan_hall (int slot);
char *get_clan_name_ch (CHAR_DATA * ch);
char *get_clan_name (int slot);
int get_clan_ident_ch (CHAR_DATA * ch);
int get_clan_ident (int slot);
int clan_lookup (const char *name);
int clan_symbol_lookup (char *symbol);
bool is_full_clan (CHAR_DATA * ch);
bool is_visible_clan (CHAR_DATA * ch);

//
// check player status in a clan
//
bool is_clan (CHAR_DATA * ch);
bool is_same_clan (CHAR_DATA * ch, CHAR_DATA * victim);
bool is_clan_leader (CHAR_DATA * ch);
bool is_clan_deputy (CHAR_DATA * ch);
bool is_clan_applicant (CHAR_DATA * ch, int clan);
bool is_clan_member (CHAR_DATA * ch, int clan);
int is_any_clan_applicant (CHAR_DATA * ch);
int is_any_clan_member (CHAR_DATA * ch);
int clan_login_player (CHAR_DATA * ch);

//
// clan tables have lists, these are the list operators
//
MEMBER *append_member (MEMBER * list, MEMBER * elt);
MEMBER *delete_member (MEMBER * list, char *name);
MEMBER *find_member (MEMBER * list, char *name);
RELATIONS *append_relation (RELATIONS * list, RELATIONS * elt);
RELATIONS *delete_relation (RELATIONS * list, int ident);
RELATIONS *find_relation (RELATIONS * list, int ident);
MEMBER *new_member_elt ();
RELATIONS *new_relations_elt ();

//
// when we need to clean out a clans space in the world
//
void remove_clan_symbols ();
void remove_symbol_resets (AREA_DATA * pArea);

//
// finding players in the world and sending them messages
//
CHAR_DATA *find_character (char *name);
void send_clan_members_ch (CHAR_DATA * ch, char *str);
void send_clan_members (int clan, char *str);
void send_all_players (CHAR_DATA * ch, char *str);
void send_allies_clanwar (CHAR_DATA * ch, char *str);

void format_clan_symbol (int slot, char *buf, int width);


bool is_player_name (char *name);


#endif
