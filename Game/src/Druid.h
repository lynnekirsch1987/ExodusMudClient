#ifndef _DRUID_H
#define _DRUID_H

// Define VNUMs For Shapechange Animals
#define MOB_VNUM_SHAPECHANGE_MOUSE   1605 // Wisdom 20
#define MOB_VNUM_SHAPECHANGE_RAVEN   1606 // Wisdom 21
#define MOB_VNUM_SHAPECHANGE_WOLF    1607 // Wisdom 22
#define MOB_VNUM_SHAPECHANGE_BEAR    1608 // Wisdom 23

#define MIN_WIS_SHAPECHANGE_MOUSE    19
#define MIN_WIS_SHAPECHANGE_RAVEN    20
#define MIN_WIS_SHAPECHANGE_WOLF     21
#define MIN_WIS_SHAPECHANGE_BEAR     22

// Define VNUM for Tree Form
#define MOB_VNUM_SHAPECHANGE_TREE    1609

// Define VNUM for Treant mob
#define MOB_VNUM_TREANT              1609

// Define VNUM for Shillelagh objects
#define OBJ_VNUM_SHILLELAGH_COMMON1        2351
#define OBJ_VNUM_SHILLELAGH_COMMON2        2352
#define OBJ_VNUM_SHILLELAGH_COMMON3        2353 
#define OBJ_VNUM_SHILLELAGH_COMMON4        2354 
#define OBJ_VNUM_SHILLELAGH_COMMON5       2355 
#define OBJ_VNUM_SHILLELAGH_COMMON6        2356 
#define OBJ_VNUM_SHILLELAGH_COMMON7        2357 
#define OBJ_VNUM_SHILLELAGH_COMMON8        2358 
#define OBJ_VNUM_SHILLELAGH_COMMON9        2359 
#define OBJ_VNUM_SHILLELAGH_COMMON10        2360 
#define OBJ_VNUM_SHILLELAGH_COMMON11        2361 
#define OBJ_VNUM_SHILLELAGH_COMMON12        2362 
#define OBJ_VNUM_SHILLELAGH_COMMON13        2363 
#define OBJ_VNUM_SHILLELAGH_COMMON14       2364 
#define OBJ_VNUM_SHILLELAGH_COMMON15        2365 
#define OBJ_VNUM_SHILLELAGH_COMMON16        2366 
#define OBJ_VNUM_SHILLELAGH_COMMON17        2367 
#define OBJ_VNUM_SHILLELAGH_COMMON18        2368 
#define OBJ_VNUM_SHILLELAGH_COMMON19        2369 
#define OBJ_VNUM_SHILLELAGH_COMMON20        2370  
#define OBJ_VNUM_SHILLELAGH_MOONBLOSSOM   1727 
#define OBJ_VNUM_SHILLELAGH_SLIMEWOOD     1728 
#define OBJ_VNUM_SHILLELAGH_RISIRIEL      1729 
#define OBJ_VNUM_SHILLELAGH_JEWELWOOD     1730 
#define OBJ_VNUM_SHILLELAGH_LEACHLEAF     1731 
#define OBJ_VNUM_SHILLELAGH_BARBVINE      1732 
#define OBJ_VNUM_SHILLELAGH_HELLROOT      1733 
#define OBJ_VNUM_SHILLELAGH_IRONWOOD      1734 
#define OBJ_VNUM_SHILLELAGH_SPECIAL       1735


// Define VNUM for Druid GM Location (Serene Grove)
#define ROOM_VNUM_SERENE_GROVE       1703

// Define Druid specific commands
DECLARE_DO_FUN(do_makedruid);
DECLARE_DO_FUN(do_shillelagh);
DECLARE_DO_FUN(do_befriend);
DECLARE_DO_FUN(do_shapechange);
DECLARE_DO_FUN(do_shillelaghdismiss);

// Define Druid Spells
DECLARE_SPELL_FUN(spell_bear_heart);
DECLARE_SPELL_FUN(spell_sprout_tree);
DECLARE_SPELL_FUN(spell_chameleon_shroud);
DECLARE_SPELL_FUN(spell_neutralize_poison);
DECLARE_SPELL_FUN(spell_tree_stride);
DECLARE_SPELL_FUN(spell_treant);
DECLARE_SPELL_FUN(spell_plant_shell);
DECLARE_SPELL_FUN(spell_trackless_step);
DECLARE_SPELL_FUN(spell_oceanic_wrath);
DECLARE_SPELL_FUN(spell_commune);
DECLARE_SPELL_FUN(spell_spore_eruption);
DECLARE_SPELL_FUN(spell_birth_return);
DECLARE_SPELL_FUN(spell_waterwalk);
DECLARE_SPELL_FUN(spell_wild_enchant);
DECLARE_SPELL_FUN(spell_winter_wrath);
DECLARE_SPELL_FUN(spell_resist_elements);
DECLARE_SPELL_FUN(spell_ironwood);
DECLARE_SPELL_FUN(spell_insect_plague);
DECLARE_SPELL_FUN(spell_desert_wrath);
DECLARE_SPELL_FUN(spell_draw_life);
DECLARE_SPELL_FUN(spell_clarity);
DECLARE_SPELL_FUN(spell_tree_form);
DECLARE_SPELL_FUN(spell_serene_grove);
DECLARE_SPELL_FUN(spell_regeneration);
DECLARE_SPELL_FUN(spell_arcane_diminish);
DECLARE_SPELL_FUN(spell_moonlance);
DECLARE_SPELL_FUN(spell_betrayal);
DECLARE_SPELL_FUN(spell_tap_root);
DECLARE_SPELL_FUN(spell_scry);
DECLARE_SPELL_FUN(spell_lunar_heal);
DECLARE_SPELL_FUN(spell_life_circle);
DECLARE_SPELL_FUN(spell_force);

// Define Utility APIs from Druid.c that will be called in other files.
bool is_druid_follower(CHAR_DATA *ch);
bool is_shapechanged_druid(CHAR_DATA *ch);
void perform_shapechange(CHAR_DATA *ch, int shapechange_vnum);
void shapechange_return(CHAR_DATA *ch, int how_returned);
void create_shillelagh(CHAR_DATA *ch, OBJ_DATA *tree);
void update_shillelagh();
OBJ_DATA* char_has_shillelagh(CHAR_DATA *ch);
bool is_tree_in_room(ROOM_INDEX_DATA *room);

// APIs to check success of Druid skills.
bool check_shillelagh(CHAR_DATA *ch);
bool check_nature_sense(CHAR_DATA *ch);
bool check_animal_empathy(CHAR_DATA *ch, CHAR_DATA *victim);
bool check_animal_friendship(CHAR_DATA *ch, CHAR_DATA *victim);
bool check_satiate(CHAR_DATA *ch);
bool check_wild_invigoration(CHAR_DATA *ch);
bool check_primeval_guidance(CHAR_DATA *ch);
bool check_shapechange(CHAR_DATA *ch);
// This api is defined in handler.c but has no declaration in a .h file...so...
void affect_strip_skills args((CHAR_DATA *ch));
bool can_move_char args((CHAR_DATA * ch, ROOM_INDEX_DATA * to_room, bool follow, bool show_messages));
void reverse_heal args((CHAR_DATA* ch, CHAR_DATA* victim, int dam, int sn));

#endif
