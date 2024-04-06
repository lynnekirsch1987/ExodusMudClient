#ifndef __OLC_H
#define __OLC_H

typedef bool OLC_FUN args ((CHAR_DATA * ch, char *argument));
#define DECLARE_OLC_FUN( fun )	OLC_FUN    fun


/* Command procedures needed ROM OLC */
DECLARE_DO_FUN (do_help);
DECLARE_SPELL_FUN (spell_null);

/*
 * Connected states for editor.
 */
#define ED_AREA 1
#define ED_ROOM 2
#define ED_OBJECT 3
#define ED_MOBILE 4
#define BUILD_MAIN 5
#define BUILD_NAME 6
#define BUILD_FLAGS 7
#define BUILD_KEYWORDS 8
#define BUILD_KEYWORDS_ADD 9
#define BUILD_DOORS 10
#define BUILD_DRESETS 11
#define BUILD_DNAME 12
#define FINGER_INFO 13
#define ED_VOCAB_MAIN 14
#define ED_VOCAB_EDIT 15
#define ED_VOCAB_ADD_WORDS 16
#define ED_VOCAB_ADD_MOOD 17
#define ED_VOCAB_ADD_MOOD_OFFSET 18
#define ED_VOCAB_ADD_TEXT 19
#define ED_VOCAB_ADD_INCLUDE 20
#define SUBMIT_MOB_VNUM 21
#define SUBMIT_MOB_NAME 22
#define SUBMIT_ROOM_VNUM 23
#define SUBMIT_TEXT 24
#define MAIL_GET_TO 25
#define MAIL_GET_SUBJECT 26
#define MAIL_TEXT 27
#define SAVE_CLAN_HELP 28
#define SAVE_ANY_HELP 29
#define EDIT_MENU 30
#define EDIT_LINES 31
#define EDIT_LINE_NUMBER 32
#define EDIT_NEW_LINE 33
#define DELE_LINE_NUMBER 34
#define INSERT_LINE_NUMBER 35
#define INSERT_NEW_LINE 36
#define SEARCH_REPLACE 37
#define DESC_INFO 38
#define ED_OBJTRIG 39
#define BUILD_DOORS_FLAGS 40
#define BUILD_EXITMSG 41
#define BUILD_ENTERMSG 42


//Iblis - 8/10/04 - Added for Ante
#define CG_ANTE		  41  //Card game, Ante stage
#define CG_ANTE_ACCEPTED  42  //Card game, Ante stage, Ante accepted

//Iblis - 6/15/04 - Added for Card Game
#define CG_START_PM_YT    43  //Pick a mob (this is the player who goes first)
#define CG_START_PM       44  //Pick a mob (this is the player who doesn't go first)	
#define CG_START_PT       45  //Pick a terrain (this it eh player who goes last)
#define CG_YT_MAIN	  46
#define CG_YT_PA_PLAYCARD 47  //Card Game, Your turn, Pre-attack, Play card
#define CG_YT_A_PM        48  //Card Game, Your turn, attack, pick mob
#define CG_YT_A_PV	  49  //Card Game, Your turn, attack, pick victim
#define CG_YT_AA_PLAYCARD 50  //Card Game, Your Turn, after attack, Play card
#define CG_YT_AA_PT       51  //Card game, Your turn, after attack, pick (spell) target
#define CG_YT_AA_PCT      52  //Card game, Your turn, after attack, pick counter target
#define CG_YT_APPRAISE    53  //Card game, Your turn, appraise
#define CG_YT_APPRAISE_D  54  //Card game, Your turn, appraise, Done with playing that round
#define CG_YT_DISCARD     55  //Card game, Your turn, discard which card?
#define CG_YT_DISCARD_D   56  //Card game, Your turn, discard which card? (done playing that round)
#define CG_YT_DONE	  57  //Card game, Your turn, finished
#define CG_NYT_MAIN       58  //Card game, Not Your Turn, main
#define CG_NYT_PLAYCARD   59  //Card game, NOt Your Turn, play a card
#define CG_NYT_PT         60  //Card game, Not Your Turn, Pick (spell) t:arget
#define CG_NYT_PCT	  61  //Card game, Not Your Turn, Pick counter target
#define CG_NYT_D_PM	  62  //Card game, Not Your Turn, Defending, Pick Mob
#define CG_NYT_APPRAISE   63  //Card game, Not Your turn, Appraise
#define CG_NYT_APPRAISE_D 64  //Card game, Not Your turn, Appraise, Done with playing that round
#define CG_NYT_DONE	  65  //Card game, Not Your Turn, Done


/*
 * Interpreter Prototypes
 */
void aedit args((CHAR_DATA * ch, char *argument));
void redit args((CHAR_DATA * ch, char *argument));
void medit args((CHAR_DATA * ch, char *argument));
void oedit args((CHAR_DATA * ch, char *argument));
void otedit args((CHAR_DATA * ch, char *argument));



/*
 * OLC Constants
 */
#define MAX_MOB	1		/* Default maximum number for resetting mobs */



/*
 * Structure for an OLC editor command.
 */
struct olc_cmd_type
{
  char *const name;
  OLC_FUN *olc_fun;
};


struct olc_help_type
{
	const char *command;
	const void *structure;
	const char *desc;
};

/*
 * Structure for an OLC editor startup command.
 */
struct editor_cmd_type
{
  char *const name;
  DO_FUN *do_fun;
};



/*
 * Utils.
 */
AREA_DATA *get_vnum_area args ((int vnum));
AREA_DATA *get_area_data args ((int vnum));
int flag_value args ((const struct flag_type * flag_table, char *argument));
char *flag_string args ((const struct flag_type * flag_table, int bits));
void add_reset args ((ROOM_INDEX_DATA * room,
		      RESET_DATA * pReset, int index));



/*
 * Interpreter Table Prototypes
 */
extern const struct olc_cmd_type aedit_table[];
extern const struct olc_cmd_type redit_table[];
extern const struct olc_cmd_type oedit_table[];
extern const struct olc_cmd_type medit_table[];
extern const struct olc_cmd_type otedit_table[]; //Obj triggers


/*
 * Editor Commands.
 */
DECLARE_DO_FUN(do_aedit);
DECLARE_DO_FUN(do_redit);
DECLARE_DO_FUN(do_oedit);
DECLARE_DO_FUN(do_medit);
DECLARE_DO_FUN(do_vedit);
DECLARE_DO_FUN(do_otedit);



/*
 * General Functions
 */
bool show_commands args ((CHAR_DATA * ch, char *argument));
bool show_help args ((CHAR_DATA * ch, char *argument));
bool edit_done args ((CHAR_DATA * ch));
bool show_version args ((CHAR_DATA * ch, char *argument));



/*
 * Area Editor Prototypes
 */
DECLARE_OLC_FUN (aedit_show);
DECLARE_OLC_FUN (aedit_helper);
DECLARE_OLC_FUN (aedit_create);
DECLARE_OLC_FUN (aedit_creator);
DECLARE_OLC_FUN (aedit_name);
DECLARE_OLC_FUN (aedit_file);
DECLARE_OLC_FUN (aedit_age);
DECLARE_OLC_FUN (aedit_clanwar);
DECLARE_OLC_FUN (aedit_clan);
DECLARE_OLC_FUN (aedit_affluence);
DECLARE_OLC_FUN (aedit_inconstruction);
DECLARE_OLC_FUN (aedit_helps);
/* DECLARE_OLC_FUN( aedit_recall        );       ROM OLC */
DECLARE_OLC_FUN (aedit_reset);
DECLARE_OLC_FUN (aedit_security);
DECLARE_OLC_FUN (aedit_builder);
DECLARE_OLC_FUN (aedit_vnum);
DECLARE_OLC_FUN (aedit_lvnum);
DECLARE_OLC_FUN (aedit_uvnum);
DECLARE_OLC_FUN (aedit_lev);
DECLARE_OLC_FUN (aedit_llev);
DECLARE_OLC_FUN (aedit_ulev);



/*
 * Room Editor Prototypes
 */
DECLARE_OLC_FUN (redit_travelmsg);
DECLARE_OLC_FUN (redit_travelexp);
DECLARE_OLC_FUN (redit_travellev);

DECLARE_OLC_FUN (redit_show);
DECLARE_OLC_FUN (redit_epl);
DECLARE_OLC_FUN (redit_Class);
DECLARE_OLC_FUN (redit_race);
DECLARE_OLC_FUN (redit_level);
DECLARE_OLC_FUN (redit_create);
DECLARE_OLC_FUN (redit_name);
DECLARE_OLC_FUN (redit_entermsg);
DECLARE_OLC_FUN (redit_exitmsg);
DECLARE_OLC_FUN (redit_desc);
DECLARE_OLC_FUN (redit_ed);
DECLARE_OLC_FUN (redit_format);
DECLARE_OLC_FUN (redit_north);
DECLARE_OLC_FUN (redit_south);
DECLARE_OLC_FUN (redit_east);
DECLARE_OLC_FUN (redit_west);
DECLARE_OLC_FUN (redit_up);
DECLARE_OLC_FUN (redit_down);
DECLARE_OLC_FUN (redit_clan);
DECLARE_OLC_FUN (redit_owner);
DECLARE_OLC_FUN (redit_purge);
DECLARE_OLC_FUN (redit_mreset);
DECLARE_OLC_FUN (redit_maxnum);
DECLARE_OLC_FUN (redit_maxmsg);
DECLARE_OLC_FUN (redit_oreset);
DECLARE_OLC_FUN (redit_rate);
DECLARE_OLC_FUN (redit_mlist);
DECLARE_OLC_FUN (redit_olist);
DECLARE_OLC_FUN (redit_mshow);
DECLARE_OLC_FUN (redit_oshow);

//Adeon 6/30/03 -- suport for sinking rooms
DECLARE_OLC_FUN (redit_sinkvnum);
DECLARE_OLC_FUN (redit_sinktimer);
DECLARE_OLC_FUN (redit_sinkwarning);
DECLARE_OLC_FUN (redit_sinkmsg);

//Iblis 8/29/04 - support for sinkmsg OTHERS see
DECLARE_OLC_FUN (redit_sinkmsgothers);




/*
 * Object Editor Prototypes
 */
DECLARE_OLC_FUN (oedit_show);
DECLARE_OLC_FUN (oedit_entermsg);
DECLARE_OLC_FUN (oedit_exitmsg);
DECLARE_OLC_FUN (oedit_arrival);
DECLARE_OLC_FUN (oedit_departure);
DECLARE_OLC_FUN (oedit_passenger);
DECLARE_OLC_FUN (oedit_create);
DECLARE_OLC_FUN (oedit_name);
DECLARE_OLC_FUN (oedit_short);
DECLARE_OLC_FUN (oedit_rarity);
DECLARE_OLC_FUN (oedit_long);
DECLARE_OLC_FUN (oedit_initmsg);
DECLARE_OLC_FUN (oedit_addaffect);
DECLARE_OLC_FUN (oedit_addpermaffect);
DECLARE_OLC_FUN (oedit_addskillaffect);
DECLARE_OLC_FUN (oedit_delaffect);
DECLARE_OLC_FUN (oedit_value0);
DECLARE_OLC_FUN (oedit_value1);
DECLARE_OLC_FUN (oedit_value2);
DECLARE_OLC_FUN (oedit_value3);
DECLARE_OLC_FUN (oedit_value4);
DECLARE_OLC_FUN (oedit_value5);
DECLARE_OLC_FUN (oedit_value6);
DECLARE_OLC_FUN (oedit_value7);
DECLARE_OLC_FUN (oedit_value8);
DECLARE_OLC_FUN (oedit_value9);
DECLARE_OLC_FUN (oedit_value10);
DECLARE_OLC_FUN (oedit_value11);
DECLARE_OLC_FUN (oedit_value12);
DECLARE_OLC_FUN (oedit_weight);
DECLARE_OLC_FUN (oedit_cost);
DECLARE_OLC_FUN (oedit_ed);

DECLARE_OLC_FUN (oedit_extra);	/* ROM */
DECLARE_OLC_FUN (oedit_wear);	/* ROM */

// Akamai 4/30/99 - Support Class/race specific objects
DECLARE_OLC_FUN (oedit_Class);
DECLARE_OLC_FUN (oedit_race);
DECLARE_OLC_FUN (oedit_clan);

// Adeon 6/29/03 -- Player ownership of objects
DECLARE_OLC_FUN (oedit_owner);

DECLARE_OLC_FUN (oedit_type);	/* ROM */
DECLARE_OLC_FUN (oedit_affect);	/* ROM */
DECLARE_OLC_FUN (oedit_material);	/* ROM */
DECLARE_OLC_FUN (oedit_level);	/* ROM */
DECLARE_OLC_FUN (oedit_timer);	/* ROM */
DECLARE_OLC_FUN (oedit_condition);	/* ROM */
DECLARE_OLC_FUN (oedit_purge);
DECLARE_OLC_FUN (oedit_trapflag);

//Iblis 12/31/03 -- Object Trigger Vnum set function
DECLARE_OLC_FUN (oedit_objtrigger);


/*
 * Mobile Editor Prototypes
 */
DECLARE_OLC_FUN(medit_show);
DECLARE_OLC_FUN(medit_vocab);
DECLARE_OLC_FUN(medit_script);
DECLARE_OLC_FUN(medit_create);
DECLARE_OLC_FUN(medit_name);
DECLARE_OLC_FUN(medit_short);
DECLARE_OLC_FUN(medit_long);
DECLARE_OLC_FUN(medit_shop);
DECLARE_OLC_FUN(medit_desc);
DECLARE_OLC_FUN(medit_level);
DECLARE_OLC_FUN(medit_mood);
DECLARE_OLC_FUN(medit_movement);
DECLARE_OLC_FUN(medit_maxweight);
DECLARE_OLC_FUN(medit_defbonus);
DECLARE_OLC_FUN(medit_attackbonus);
DECLARE_OLC_FUN(medit_mobgroup);
DECLARE_OLC_FUN(medit_damtype);
DECLARE_OLC_FUN(medit_align);
DECLARE_OLC_FUN(medit_spec);

DECLARE_OLC_FUN(medit_purge);
DECLARE_OLC_FUN(medit_sex);	/* ROM */
DECLARE_OLC_FUN(medit_act);	/* ROM */
DECLARE_OLC_FUN(medit_affect);	/* ROM */
DECLARE_OLC_FUN(medit_ac);	/* ROM */
DECLARE_OLC_FUN(medit_form);	/* ROM */
DECLARE_OLC_FUN(medit_part);	/* ROM */
DECLARE_OLC_FUN(medit_imm);	/* ROM */
DECLARE_OLC_FUN(medit_res);	/* ROM */
DECLARE_OLC_FUN(medit_vuln);	/* ROM */
DECLARE_OLC_FUN(medit_recruit);	/* ROM */
DECLARE_OLC_FUN(medit_psvnum);	/* ROM */
DECLARE_OLC_FUN(medit_pscost);	/* ROM */
DECLARE_OLC_FUN(medit_material);	/* ROM */
DECLARE_OLC_FUN(medit_off);	/* ROM */
DECLARE_OLC_FUN(medit_size);	/* ROM */
DECLARE_OLC_FUN(medit_hitdice);	/* ROM */
DECLARE_OLC_FUN(medit_manadice);	/* ROM */
DECLARE_OLC_FUN(medit_damdice);	/* ROM */
DECLARE_OLC_FUN(medit_race);	/* ROM */
DECLARE_OLC_FUN(medit_position);	/* ROM */
DECLARE_OLC_FUN(medit_wealth);	/* ROM */
DECLARE_OLC_FUN(medit_hitroll);	/* ROM */
DECLARE_OLC_FUN(medit_blockexit); // Adeon 7/17/03 -- mobs blocking exits
DECLARE_OLC_FUN(medit_numattacks); // Iblis 2/05/04 -- setting number of mob attacks
DECLARE_OLC_FUN(medit_cardvnum); // Iblis 8/29/04 -- setting the rune card vnum that pops on a mob
DECLARE_OLC_FUN(medit_str); // Iblis 8/29/04 -- setting the perm_stat on a mob
DECLARE_OLC_FUN(medit_int); // Iblis 8/29/04 -- setting the perm_stat on a mob
DECLARE_OLC_FUN(medit_wis); // Iblis 8/29/04 -- setting the perm_stat on a mob
DECLARE_OLC_FUN(medit_dex); // Iblis 8/29/04 -- setting the perm_stat on a mob
DECLARE_OLC_FUN(medit_con); // Iblis 8/29/04 -- setting the perm_stat on a mob
DECLARE_OLC_FUN(medit_cha); // Iblis 8/29/04 -- setting the perm_stat on a mob
/* Add Obj Trigger Declarations here! */
DECLARE_OLC_FUN(otedit_create);
DECLARE_OLC_FUN(otedit_show);
DECLARE_OLC_FUN(otedit_chance);
DECLARE_OLC_FUN(otedit_extra);
DECLARE_OLC_FUN(otedit_name);
DECLARE_OLC_FUN(otedit_trigger);
DECLARE_OLC_FUN(otedit_addaction);
DECLARE_OLC_FUN(otedit_remaction);
DECLARE_OLC_FUN(otedit_purge);


/*
 * Macros
 */

#define IS_SWITCHED( ch )       ( ch->desc->original )	/* ROM OLC */

#define IS_BUILDER( ch )	( IS_NPC(ch) ? 0 : (ch->pcdata->security && !IS_SWITCHED(ch)))

#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))

/* Return pointers to what is being edited. */
#define EDIT_MOB(Ch, Mob)	( Mob = (MOB_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_OBJ(Ch, Obj)	( Obj = (OBJ_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_ROOM(Ch, Room)	( Room = Ch->in_room )
#define EDIT_AREA(Ch, Area)	( Area = (AREA_DATA *)Ch->desc->pEdit )
#define EDIT_OBJ_TRIG(Ch, ObjTrig) ( ObjTrig = (OBJ_TRIG_DATA *) Ch->desc->pEdit )






/*
 * Prototypes
 */
/* mem.c - memory prototypes. */
#define ED	EXTRA_DESCR_DATA
RESET_DATA *new_reset_data args ((void));
void free_reset_data args ((RESET_DATA * pReset));
AREA_DATA *new_area args ((void));
void free_area args ((AREA_DATA * pArea));
EXIT_DATA *new_exit args ((void));
void free_exit args ((EXIT_DATA * pExit));
ED *new_extra_descr args ((void));
void free_extra_descr args ((ED * pExtra));
ROOM_INDEX_DATA *new_room_index args ((void));
void free_room_index args ((ROOM_INDEX_DATA * pRoom));
AFFECT_DATA *new_affect args ((void));
void free_affect args ((AFFECT_DATA * pAf));
SHOP_DATA *new_shop args ((void));
void free_shop args ((SHOP_DATA * pShop));
OBJ_INDEX_DATA *new_obj_index args ((void));
void free_obj_index args ((OBJ_INDEX_DATA * pObj));
MOB_INDEX_DATA *new_mob_index args ((void));
void free_mob_index args ((MOB_INDEX_DATA * pMob));
#undef	ED

#endif

