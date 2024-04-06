#ifndef __TABLES_H
#define __TABLES_H

struct flag_type
{
  char *name;
  int bit;
  bool settable;
};

struct position_type
{
  char *name;
  char *short_name;
};

struct sex_type
{
  char *name;
};

struct size_type
{
  char *name;
};

struct wear_translation_type
{
  int wear_loc;
  long wear_flag;
  char *wear_string;
  char *wear_name;
  char *where_name;
};

/* game tables */
extern const struct position_type position_table[];
extern const struct sex_type sex_table[];
extern const struct size_type size_table[];

/* flag tables */
extern const struct flag_type act_flags[];
extern const struct flag_type act_flags2[];
extern const struct flag_type plr_flags[];
extern const struct flag_type affect_flags[];
extern const struct flag_type paffect_flags[];
extern const struct flag_type recruit_flags[];
extern const struct flag_type off_flags[];
extern const struct flag_type imm_flags[];
extern const struct flag_type form_flags[];
extern const struct flag_type part_flags[];
extern const struct flag_type comm_flags[];
extern const struct flag_type comm2_flags[];
extern const struct flag_type extra_flags[MAX_EXTRA_FLAGS][31];
extern const struct flag_type wear_flags[];
extern const struct flag_type weapon_flags[];
extern const struct flag_type container_flags[];
extern const struct flag_type portal_flags[];
extern const struct flag_type room_flags[];
extern const struct flag_type room_flags2[];
extern const struct flag_type exit_flags[];
extern const struct flag_type gate_flags[];
extern const struct flag_type ot_extra_flags[];
extern const struct flag_type area_flags[];
extern const struct flag_type sex_flags[];
extern const struct flag_type Class_flags[];
extern const struct flag_type pcrace_flags[];
extern const struct flag_type exit_flags[];
extern const struct flag_type pact_flags[];
extern const struct flag_type tact_flags[];
extern const struct flag_type button_flags[];
extern const struct flag_type lock_flags[];
extern const struct flag_type door_resets[];
extern const struct flag_type room_flags[];
extern const struct flag_type sector_flags[];
extern const struct flag_type room_flags2[];
extern const struct flag_type tsector_flags[];
extern const struct flag_type type_flags[];
extern const struct flag_type clan_flags[];
extern const struct flag_type act_flags[];
extern const struct flag_type affect_flags[];
extern const struct flag_type act_flags2[];
extern const struct flag_type apply_flags[];
extern const struct flag_type rarity_flags[];
extern const struct flag_type wear_loc_strings[];
extern const struct flag_type wear_loc_flags[];
extern const struct flag_type weapon_flags[];
extern const struct flag_type container_flags[];
extern const struct flag_type liquid_flags[];
extern const struct flag_type material_type[];
extern const struct flag_type form_flags[];
extern const struct flag_type part_flags[];
extern const struct flag_type ac_type[];
extern const struct flag_type size_flags[];
extern const struct flag_type off_flags[];
extern const struct flag_type imm_flags[];
extern const struct flag_type res_flags[];
extern const struct flag_type vuln_flags[];
extern const struct flag_type position_flags[];
extern const struct flag_type weapon_Class[];
extern const struct flag_type instrument_flags[];
extern const struct flag_type furniture_type[];
extern const struct flag_type weapons_type[];
extern const struct flag_type card_Class[];
extern const struct flag_type card_terrain_type[];
extern const struct flag_type card_element_type[];
extern const struct flag_type card_race_type[];
extern const struct flag_type card_terrain_toggles[];
extern const struct flag_type trap_type[];
extern const struct flag_type trap_flag[];
extern const struct flag_type dir_flags[];

extern const struct wear_translation_type wear_conversion_table[];

#endif


