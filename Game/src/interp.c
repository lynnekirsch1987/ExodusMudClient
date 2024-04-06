#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "olc.h"
bool check_social args ((CHAR_DATA * ch, char *command, char *argument));
bool keepincomm = FALSE;
char last_command[MAX_STRING_LENGTH];

void cardgame_menu args ((CHAR_DATA * ch, char *argument));

/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2
#define LOG_PASSWORD    3

/*
 * Log-all switch.
 */
bool fLogAll = FALSE;

/*
 * Command table.
 */
const struct cmd_type cmd_table[] = {
  /*
   * Common movement commands.
   */
  {"north", do_north, POS_STANDING, 0, LOG_NEVER, 0, FALSE},
  {"east", do_east, POS_STANDING, 0, LOG_NEVER, 0, FALSE},
  {"south", do_south, POS_STANDING, 0, LOG_NEVER, 0, FALSE},
  {"west", do_west, POS_STANDING, 0, LOG_NEVER, 0, FALSE},
  {"up", do_up, POS_STANDING, 0, LOG_NEVER, 0, FALSE},
  {"down", do_down, POS_STANDING, 0, LOG_NEVER, 0, FALSE},
  /*
   * Common other commands.
   * Placed here so one and two letter abbreviations work.
   */
  {"donate", do_donate, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"dice", do_dice, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"write", do_write, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"at", do_at, POS_DEAD, L7, LOG_NORMAL, 1, FALSE},
  {"cast", do_cast, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"cash", do_cash, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"cook", do_cook, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"hone", do_hone, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"catchup", do_catchup, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"circle", do_circle, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"pray", do_pray, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"pack", do_pack, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"unpack", do_unpack, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"fish", do_fish, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"camp", do_camp, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"addfake", do_addfake, POS_DEAD, L1, LOG_NORMAL, 1, FALSE},
  {"remfake", do_remfake, POS_DEAD, L1, LOG_NORMAL, 1, FALSE},
  {"finger", do_finger, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"explore", do_explored, POS_DEAD, ML, LOG_NORMAL, 1, FALSE},
  {"charge", do_charge, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"auction", do_auction, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"balance", do_balance, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"buy", do_buy, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"channels", do_channels, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},

    {"delevel", do_delevel, POS_DEAD, ML, LOG_NORMAL, 1, FALSE},
    {"relevel",	do_relevel, POS_DEAD,  1, LOG_ALWAYS, 1, FALSE},
  {"deposit", do_deposit, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"duel", do_duel, POS_RESTING, 9, LOG_NORMAL, 1, FALSE},
  {"elist", do_elist, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"exits", do_exits, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"exorcise", do_exorcise, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"get", do_get, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"goto", do_goto, POS_DEAD, L8, LOG_NORMAL, 1, FALSE},
  {"group", do_group, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"gallop", do_gallop, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"hit", do_kill, POS_FIGHTING, 0, LOG_NORMAL, 0, FALSE},
  {"inventory", do_inventory, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"ignore", do_ignore, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"kill", do_kill, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"killcount", do_killcount, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"kc", do_killcount, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"look", do_look, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"lore", do_lore, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"glance", do_glance, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"clan", do_clan, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"clantalk", do_clantalk, POS_SLEEPING, 0, LOG_NORMAL, 1, TRUE},
  {"ct", do_clantalk, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"cls", do_cls, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"clear", do_cls, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"music", do_music, POS_SLEEPING, 0, LOG_NORMAL, 1, TRUE},
  {"math", do_math, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"tell", do_tell, POS_RESTING, 0, LOG_NORMAL, 1, TRUE},
  {"telepath", do_telepath, POS_RESTING, 0, LOG_NORMAL, 1, TRUE},
  {"throw", do_throw, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"mount", do_mount, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"tether", do_tether, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"untether", do_untether, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"dismount", do_dismount, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"dismiss", do_dismiss, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"tame", do_tame, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"order", do_order, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"practice", do_practice, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"estimate", do_estimate, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"rest", do_rest, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"redirect", do_redirect, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"sit", do_sit, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"sockets", do_newsockets, POS_DEAD, L5, LOG_NORMAL, 1, FALSE},
  {"stand", do_stand, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"stalk", do_stalk, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"unlock", do_unlock, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"wield", do_wear, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"withdraw", do_withdraw, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"wizhelp", do_wizhelp, POS_DEAD, L8, LOG_NORMAL, 1, FALSE},
  {"newwizhelp", do_newwizhelp, POS_DEAD, 91, LOG_NORMAL, 1, FALSE},
  /*
   * Informational commands.
   */
  {"affects", do_affects, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"areas", do_areas, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"bet", do_bet, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"bug", do_bug, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"build", do_build, POS_STANDING, 0, LOG_ALWAYS, 1, FALSE},
  {"coax", do_coax, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"commands", do_commands, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"compare", do_compare, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"consider", do_consider, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"count", do_count, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"create", do_create, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"credits", do_credits, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"equipment", do_equipment, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"examine", do_examine, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"help", do_help, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"info", do_info, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"motd", do_motd, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"read", do_read, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"report", do_report, POS_RESTING, 0, LOG_NORMAL, 1, TRUE},
  {"repertoire", do_repertoire, POS_RESTING, 0, LOG_NORMAL, 1,
   FALSE}, {"rules", do_rules, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"score", do_score, POS_DEAD, 0, LOG_NORMAL, 1, FALSE}, {"scan",
							   do_scanning,
							   POS_RESTING,
							   0,
							   LOG_NORMAL,
							   1,
							   FALSE},
  {"skills", do_skills, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"socials", do_socials, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"show", do_show, POS_DEAD, 0, LOG_NORMAL, 1, FALSE}, {"showmem",
							 do_showmem,
							 POS_DEAD,
							 ML,
							 LOG_NORMAL,
							 1, FALSE},
  {"spells", do_spells, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"speedwalk", do_speedwalk, POS_STANDING, IM, LOG_NORMAL, 1,
   FALSE}, {"song", do_songs, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"songs", do_songs, POS_DEAD, 0, LOG_NORMAL, 1, FALSE}, {"chant",
							   do_chant,
							   POS_FIGHTING,
							   0,
							   LOG_NORMAL,
							   1,
							   FALSE},
  {"chants", do_chants, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"prayers", do_prayers, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"stock", do_stock, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"unstock", do_unstock, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"hire", do_hire, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"showstats", do_showstats, POS_SLEEPING, 0, LOG_NORMAL, 1,
   FALSE}, {"story", do_story, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"time", do_time, POS_DEAD, 0, LOG_NORMAL, 1, FALSE}, {"typo",
							 do_typo,
							 POS_DEAD,
							 0,
							 LOG_NORMAL,
							 1, FALSE},
  {"weather", do_weather, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"who", do_who, POS_DEAD, 0, LOG_NORMAL, 1, FALSE}, {"whois",
						       do_whois,
						       POS_DEAD, 0,
						       LOG_NORMAL,
						       1, FALSE},
  {"wizlist", do_wizlist, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"worth", do_worth, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  /*
   * Configuration commands.
   */
  {"alia", do_alia, POS_DEAD, 0, LOG_NORMAL, 0, FALSE},
  {"alias", do_alias, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"autolist", do_autolist, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"autoassist", do_autoassist, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"autobutcher", do_autobutcher, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"autoexit", do_autoexit, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"autogold", do_autogold, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"autoloot", do_autoloot, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"autoreturn", do_autoreturn, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"autosac", do_autosac, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"autosplit", do_autosplit, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"brief", do_brief, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"combine", do_combine, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"compact", do_compact, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"color", do_color, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"description", do_description, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"delet", do_delet, POS_DEAD, 0, LOG_PASSWORD, 0, FALSE},
  {"delete", do_delete, POS_STANDING, 0, LOG_PASSWORD, 1, FALSE},
  {"nofollow", do_nofollow, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"assassinate", do_assassinate, POS_STANDING, 0, LOG_NORMAL, 1,
   FALSE}, {"noloot", do_noloot, POS_DEAD, 0, LOG_NORMAL, 1,
	    FALSE}, {"nosummon", do_nosummon, POS_DEAD, 0,
		     LOG_NORMAL, 1, FALSE}, {"call", do_call,
					     POS_STANDING, 0,
					     LOG_NORMAL, 1,
					     FALSE}, {"outfit",
						      do_outfit,
						      POS_RESTING,
						      0,
						      LOG_NORMAL,
						      1, FALSE},
  {"password", do_password, POS_DEAD, 0, LOG_PASSWORD, 1, FALSE},
  {"prompt", do_prompt, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"lines", do_scroll, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"title", do_title, POS_DEAD, 15, LOG_NORMAL, 1, FALSE},
  {"unalias", do_unalias, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"wimpy", do_wimpy, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  /*
   * Communication commands.
   */
  {"afk", do_afk, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
 {"answer", do_answer, POS_SLEEPING, 0, LOG_NORMAL, 1, TRUE},
  {"anonymous", do_anonymous, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"deaf", do_deaf, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"emote", do_emote, POS_RESTING, 0, LOG_NORMAL, 1, TRUE},
  {"email", do_email, POS_RESTING, 0, LOG_NORMAL, 1, TRUE},
  {"desecrate", do_desecrate, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"pmote", do_pmote, POS_RESTING, 0, LOG_NORMAL, 1, TRUE}, {".",
							     do_gossip,
							     POS_SLEEPING,
							     0,
							     LOG_NORMAL,
							     0,
							     TRUE},
  {"gossip", do_gossip, POS_SLEEPING, 0, LOG_NORMAL, 1, TRUE},
  {"ooc", do_ooc, POS_SLEEPING, 0, LOG_NORMAL, 1, TRUE}, 
  {",",
							  do_emote,
							  POS_RESTING,
							  0,
							  LOG_NORMAL,
							  0, TRUE},
  {"grats", do_grats, POS_SLEEPING, 0, LOG_NORMAL, 1, TRUE},
  {"gtell", do_gtell, POS_DEAD, 0, LOG_NORMAL, 1, TRUE}, {"]",
							  do_gtell,
							  POS_DEAD,
							  0,
							  LOG_NORMAL,
							  0, TRUE},
  {"cd", clan_donate, POS_RESTING, 0, LOG_NORMAL, 0, TRUE},
  {"scroll", do_note, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"rpreward", do_rpreward, POS_DEAD, 91, LOG_ALWAYS, 1, FALSE},
  {"nhreward", do_nhreward, POS_DEAD, 93, LOG_ALWAYS, 1, FALSE},
  {"qreward", do_qreward, POS_DEAD, 93, LOG_ALWAYS, 1, FALSE},
  {"news", do_news, POS_DEAD, 0, LOG_NORMAL, 1, FALSE}, 
  {"legend", do_legend, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"oocnote", do_oocnote, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"changes", do_changes, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"idea", do_idea, POS_DEAD, 0, LOG_NORMAL, 1, FALSE}, {"submit",
							 do_submit,
							 POS_SLEEPING,
							 92,
							 LOG_NORMAL,
							 1, FALSE},
  {"oldsockets", do_sockets, POS_DEAD, L5, LOG_NORMAL, 1, FALSE},
/* Command MAIL disabled as it made the mud lock up
   June 22. 2000, Morgan

   {"mail", do_mail, POS_SLEEPING, 99, LOG_NORMAL, 1, FALSE},
 */
  {"pose", do_pose, POS_RESTING, 0, LOG_NORMAL, 1, TRUE},
//  {"question", do_question, POS_SLEEPING, 0, LOG_NORMAL, 1, TRUE},
  {"quiet", do_quiet, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"reply", do_reply, POS_SLEEPING, 0, LOG_NORMAL, 1, TRUE},
  {"replay", do_replay, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"layhands", do_layhands, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"say", do_say, POS_RESTING, 0, LOG_NORMAL, 1, TRUE},
  {"'", do_say, POS_RESTING, 0, LOG_NORMAL, 0, TRUE},
  {"#", do_multi, POS_DEAD, 0, LOG_NORMAL, 0, FALSE},
  {"sanctify", do_sanctify, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"disperse", do_disperse, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"shout", do_shout, POS_RESTING, 3, LOG_NORMAL, 1, TRUE},
  {"shriek", do_shout, POS_RESTING, 3, LOG_NORMAL, 1, TRUE},
  {"unread", do_unread, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"yell", do_yell, POS_RESTING, 0, LOG_NORMAL, 1, TRUE},
  {"brew", do_brew, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"scribe", do_scribe, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"bounty", do_bounty, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"brandish", do_brandish, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"close", do_close, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"drag", do_drag, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"drink", do_drink, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"drive", do_drive, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"drop", do_drop, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"eat", do_eat, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"envenom", do_envenom, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"fill", do_fill, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"give", do_give, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"heal", do_heal, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"hold", do_wear, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"list", do_list, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"lock", do_lock, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"open", do_open, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"pick", do_pick, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"doorbash", do_doorbash, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"db", do_doorbash, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"cw", do_clanwar, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"clanwar", do_clanwar, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"pour", do_pour, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"put", do_put, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"push", do_push, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"pull", do_pull, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"press", do_press, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"quaff", do_quaff, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"recite", do_recite, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"remove", do_remove, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"disrobe", do_disrobe, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"sell", do_sell, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"take", do_get, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"sacrifice", do_sacrifice, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"sail", do_sail, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"paddle", do_paddle, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"row", do_row, POS_RESTING, 0, LOG_NORMAL, 1, FALSE}, {"junk",
							  do_sacrifice,
							  POS_RESTING,
							  0,
							  LOG_NORMAL,
							  0,
							  FALSE},
  {"tap", do_sacrifice, POS_RESTING, 0, LOG_NORMAL, 0, FALSE},
  {"ban", do_ban, POS_DEAD, L2, LOG_ALWAYS, 1, FALSE}, {"value",
							do_value,
							POS_RESTING,
							0,
							LOG_NORMAL,
							1, FALSE},
  {"wear", do_wear, POS_RESTING, 0, LOG_NORMAL, 1, FALSE}, {"zap",
							    do_zap,
							    POS_RESTING,
							    0,
							    LOG_NORMAL,
							    1,
							    FALSE},
  {"zapchain", do_zapchain, POS_DEAD, L8, LOG_NORMAL, 1, FALSE},
  {"backstab", do_backstab, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"sap", do_sap, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE}, {"knock",
							   do_knock,
							   POS_FIGHTING,
							   0,
							   LOG_NORMAL,
							   1,
							   FALSE},
  {"feign", do_feign, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"prevent", do_prevent, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"dive", do_dive, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"toss", do_toss, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE}, {"art",
							     do_art,
							     POS_RESTING,
							     0,
							     LOG_NORMAL,
							     1,
							     FALSE},
  {"strike", do_strike, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"whirlwind", do_whirlwind, POS_FIGHTING, 0, LOG_NORMAL, 1,
   FALSE}, {"fists", do_fists, POS_FIGHTING, 0, LOG_NORMAL, 1,
	    FALSE}, {"bandage", do_bandage, POS_FIGHTING, 0,
		     LOG_NORMAL, 1, FALSE}, {"mix", do_mix,
					     POS_RESTING, 0,
					     LOG_NORMAL, 1,
					     FALSE}, {"repulse",
						      do_repulse,
						      POS_STANDING,
						      0,
						      LOG_NORMAL,
						      1, FALSE},
  {"tempt", do_tempt, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"falconry", do_falconry, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"deliver", do_deliver, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"fire", do_fire, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"fear", do_fear, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"pounce", do_pounce, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"disorient", do_disorient, POS_FIGHTING, 0, LOG_NORMAL, 1,
   FALSE}, {"carve", do_carve, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"attach", do_attach, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"reel", do_reel, POS_RESTING, 0, LOG_NORMAL, 1, FALSE}, {"bait",
							    do_bait,
							    POS_RESTING,
							    0,
							    LOG_NORMAL,
							    1,
							    FALSE},
  {"noblink", do_noblink, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"peek", do_peek, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"superduel", do_superduel, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"illusion", do_illusion, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"eviscerate", do_eviscerate, POS_FIGHTING, 0, LOG_NORMAL, 1,
   FALSE}, {"uppercut", do_uppercut, POS_FIGHTING, 0, LOG_NORMAL,
	    1, FALSE}, {"kai", do_kai, POS_FIGHTING, 0,
			LOG_NORMAL, 1, FALSE},	/*{"stun", do_stun,
						   POS_FIGHTING, 0,
						   LOG_NORMAL, 1,
						   FALSE}, */ {"bash",
							       do_bash,
							       POS_FIGHTING,
							       0,
							       LOG_NORMAL,
							       1,
							       FALSE},
  {"bs", do_backstab, POS_FIGHTING, 0, LOG_NORMAL, 0, FALSE},
  {"slash", do_shadowslash, POS_FIGHTING, 0, LOG_NORMAL, 0, FALSE},
  {"berserk", do_berserk, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"dirt", do_dirt, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"disarm", do_disarm, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"flee", do_flee, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"kick", do_kick, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
/*
  {"murde", do_murde, POS_FIGHTING, 0, LOG_NORMAL, 0, FALSE},
  {"murder", do_murder, POS_FIGHTING, 5, LOG_NORMAL, 1, FALSE},
*/
  {"rescue", do_rescue, POS_FIGHTING, 0, LOG_NORMAL, 0, FALSE},
  {"trip", do_trip, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"tripwire", do_tripwire, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"conceal", do_conceal, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"butcher", do_butcher, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"trample", do_trample, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"checksafe", do_checksafe, POS_DEAD, L2, LOG_ALWAYS, 1, FALSE},
  {"checkbuild", do_checkbuild, POS_DEAD, L2, LOG_ALWAYS, 1, FALSE},
  {"checkcity", do_checkcity, POS_DEAD, L2, LOG_ALWAYS, 1, FALSE},
  {"killwipe", do_killwipe, POS_DEAD, ML, LOG_ALWAYS, 1, FALSE},
  {"enter", do_enter, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"board", do_board, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"climb", do_climb, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"scale", do_scale, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"entangle", do_entangle, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"tug", do_tug, POS_STANDING, 0, LOG_NORMAL, 1, FALSE}, {"ring",
							   do_ring,
							   POS_STANDING,
							   0,
							   LOG_NORMAL,
							   1,
							   FALSE},
  {"shove", do_shove, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"smash", do_smash, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"sing", do_sing, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"play", do_play, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"compose", do_compose, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"jump", do_jump, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"crawl", do_crawl, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"descend", do_descend, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"follow", do_follow, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"gain", do_gain, POS_STANDING, 0, LOG_NORMAL, 1, FALSE}, {"go",
							     do_enter,
							     POS_STANDING,
							     0,
							     LOG_NORMAL,
							     0,
							     FALSE},
//  {"groups", do_groups, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"hide", do_hide, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"camouflage", do_camouflage, POS_RESTING, 0, LOG_NORMAL, 1,
   FALSE}, {"qui", do_qui, POS_DEAD, 0, LOG_NORMAL, 0, FALSE},
  {"quit", do_quit, POS_DEAD, 0, LOG_NORMAL, 1, FALSE}, {"recall",
							 do_recall,
							 POS_FIGHTING,
							 0,
							 LOG_NORMAL,
							 1, FALSE},
  {"/", do_recall, POS_FIGHTING, 0, LOG_NORMAL, 0, FALSE},
  {"rent",
   do_rent,
   POS_DEAD,
   0,
   LOG_NORMAL,
   0,
   FALSE},
  {"rename", do_rename, POS_DEAD, 98, LOG_ALWAYS, 1, FALSE},

  {"battlecry", do_battlecry, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"save", do_save, POS_DEAD, 0, LOG_NORMAL, 1, FALSE}, {"defect",
							 do_defect,
							 POS_RESTING,
							 0,
							 LOG_NORMAL,
							 1, FALSE},
  {"disband", do_disband, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"sleep", do_sleep, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"sneak", do_sneak, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"stable", do_stable, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"retrieve", do_retrieve, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"split", do_split, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"steal", do_steal, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"swap", do_swap, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"sweep", do_sweep, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"palm", do_palm, POS_RESTING, 0, LOG_NORMAL, 1, FALSE}, {"plan",
							    do_plan,
							    POS_DEAD,
							    0,
							    LOG_NORMAL,
							    1,
							    FALSE},
  {"plant", do_plant, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"pat", do_pat, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"topfishers", do_topfishers, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"train", do_train, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"track", do_track, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"visible", do_visible, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"land", do_land, POS_RESTING, 0, LOG_NORMAL, 1, FALSE}, {"mask",
							    do_mask,
							    POS_STANDING,
							    0,
							    LOG_ALWAYS,
							    1,
							    FALSE},
  {"fly", do_fly, POS_RESTING, 0, LOG_NORMAL, 1, FALSE}, {"wake",
							  do_wake,
							  POS_SLEEPING,
							  0,
							  LOG_NORMAL,
							  1,
							  FALSE},
  {"where", do_where, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"version", do_version, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"advance", do_advance, POS_DEAD, L4, LOG_ALWAYS, 1, FALSE},
  {"dump", do_dump, POS_DEAD, ML, LOG_ALWAYS, 0, FALSE}, {"clist",
							  do_clist,
							  POS_DEAD,
							  L6,
							  LOG_NORMAL,
							  1,
							  FALSE},
  {"fakehost", do_fakehost, POS_DEAD, 97, LOG_ALWAYS, 1, FALSE},
  {"trust", do_trust, POS_DEAD, ML, LOG_ALWAYS, 1, FALSE},
  {"violate", do_violate, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"allow", do_allow, POS_DEAD, L1, LOG_ALWAYS, 1, FALSE},
  {"deny", do_deny, POS_DEAD, L1, LOG_ALWAYS, 1, FALSE}, {"discon",
							  do_disconnect,
							  POS_DEAD,
							  L4,
							  LOG_ALWAYS,
							  1,
							  FALSE},
  {"flag", do_flag, POS_DEAD, L4, LOG_ALWAYS, 1, FALSE}, {"freeze",
							  do_freeze,
							  POS_DEAD,
							  L5,
							  LOG_ALWAYS,
							  1,
							  FALSE},
  {"unfreeze", do_unfreeze, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"reward", do_reward, POS_DEAD, 93, LOG_ALWAYS, 1, FALSE},
  {"permban", do_permban, POS_DEAD, ML, LOG_ALWAYS, 1, FALSE},
  {"permaff", do_permaff, POS_DEAD, L1, LOG_ALWAYS, 1, FALSE},
  {"skillaff", do_skillaff, POS_DEAD, L1, LOG_ALWAYS, 1, FALSE},
  {"whoinfo", do_whoinfo, POS_DEAD, L7, LOG_ALWAYS, 1, FALSE},
  {"whomsg", do_whomsg, POS_DEAD, 93, LOG_ALWAYS, 1, FALSE},
  {"noble", make_noble, POS_DEAD, 97, LOG_ALWAYS, 1, FALSE},
  {"protect", do_protect, POS_DEAD, ML, LOG_ALWAYS, 1, FALSE},
  {"reboo", do_reboo, POS_DEAD, L1, LOG_NORMAL, 0, FALSE},
  {"reboot", do_reboot, POS_DEAD, L1, LOG_ALWAYS, 1, FALSE},
  {"reclan", do_reclan, POS_DEAD, ML, LOG_ALWAYS, 1, FALSE},
  {"set", do_set, POS_DEAD, L4, LOG_ALWAYS, 1, FALSE}, {"script",
							do_script,
							POS_DEAD,
							L8,
							LOG_NORMAL,
							1, FALSE},
  {"shutdow", do_shutdow, POS_DEAD, L1, LOG_NORMAL, 0, FALSE},
  {"shutdown", do_shutdown, POS_DEAD, ML, LOG_ALWAYS, 1, FALSE},
  {"seize", do_seize, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"drestore", do_deathrestore, POS_DEAD, ML, LOG_ALWAYS, 1, FALSE},
  {"quad_exp", do_quad_xp, POS_DEAD, 97, LOG_ALWAYS, 1, FALSE},
  {"double_exp", do_double_xp, POS_DEAD, 97, LOG_ALWAYS, 1, FALSE},
  {"half_exp", do_half_xp, POS_DEAD, 97, LOG_ALWAYS, 1, FALSE},
  {"cancel_exp", do_cancel_xp, POS_DEAD, 97, LOG_ALWAYS, 1, FALSE},
  {"battle_royale", do_battle_royale, POS_DEAD, 97, LOG_ALWAYS, 1, FALSE},
  {"slowns", do_slowns, POS_DEAD, ML, LOG_ALWAYS, 1, FALSE},
  {"wizlock", do_wizlock, POS_DEAD, L2, LOG_ALWAYS, 1, FALSE},
  {"for", do_for, POS_DEAD, 97, LOG_ALWAYS, 1, FALSE},
  {"force", do_force, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"forage", do_forage, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"load", do_load, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"newlock", do_newlock, POS_DEAD, L4, LOG_ALWAYS, 1, FALSE},
  {"nochannel", do_nochannels, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"shutup", do_shutup, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"noemote", do_noemote, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"noshout", do_noshout, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"notell", do_notell, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"notitle", do_notitle, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"pecho", do_pecho, POS_DEAD, L6, LOG_ALWAYS, 1, FALSE},
  {"pardon", do_pardon, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"jail", do_jail, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE}, {"purge",
							  do_purge,
							  POS_DEAD,
							  0,
							  LOG_ALWAYS,
							  1,
							  FALSE},
  {"zonepurge", do_zonepurge, POS_DEAD, 0, LOG_ALWAYS, 1, FALSE},
  {"restore", do_restore, POS_DEAD, L3, LOG_ALWAYS, 1, FALSE},
  {"restoremsg", do_restoremsg, POS_DEAD, L3, LOG_ALWAYS, 1, FALSE},
  {"sla", do_sla, POS_DEAD, L4, LOG_NORMAL, 0, FALSE}, {"slay",
							do_slay,
							POS_DEAD,
							L4,
							LOG_ALWAYS,
							1, FALSE},
  {"teleport", do_transfer, POS_DEAD, L7, LOG_ALWAYS, 1, FALSE},
  {"transfer", do_transfer, POS_DEAD, L7, LOG_ALWAYS, 1, FALSE},
  //Shinowlan - 2/3/99 Rarity command
  {"rarity", do_rarity, POS_DEAD, L2, LOG_NORMAL, 1, FALSE},
  {"poofin", do_bamfin, POS_DEAD, L8, LOG_NORMAL, 1, FALSE},
  {"poofout", do_bamfout, POS_DEAD, L8, LOG_NORMAL, 1, FALSE},
  {"gecho", do_echo, POS_DEAD, L4, LOG_ALWAYS, 1, FALSE},
  {"holylight", do_holylight, POS_DEAD, L8, LOG_NORMAL, 1, FALSE},
  {"incognito", do_incognito, POS_DEAD, L8, LOG_NORMAL, 1, FALSE},
  {"invis", do_invis, POS_DEAD, IM, LOG_NORMAL, 0, FALSE},
  {"log", do_log, POS_DEAD, L1, LOG_ALWAYS, 1, FALSE},
  // Morgan June 20. 2000 - linkload/unload/chpass commands
  {"linkload", do_linkload, POS_DEAD, 97, LOG_ALWAYS, 1, FALSE},
  {"lunload", do_lunload, POS_DEAD, 97, LOG_NORMAL, 1, FALSE},
  {"chpass", do_chpass, POS_DEAD, 100, LOG_NORMAL, 1, FALSE},
  // Morgan June 24. 2000 Administrator channel
  {"admin", do_admin, POS_DEAD, ML, LOG_NORMAL, 1, FALSE},
  {"hearclan", do_hearclan, POS_DEAD, L5, LOG_NORMAL, 1, FALSE},
  {"sendclan", do_sendclan, POS_DEAD, L5, LOG_NORMAL, 1, FALSE},
  {"sendcw", do_sendcw, POS_DEAD, L5, LOG_NORMAL, 1, FALSE},
  {"memory", do_memory, POS_DEAD, L1, LOG_NORMAL, 1, FALSE},
  {"mwhere", do_mwhere, POS_DEAD, L6, LOG_NORMAL, 1, FALSE},
  {"owhere", do_owhere, POS_DEAD, L6, LOG_NORMAL, 1, FALSE},
  {"peace", do_peace, POS_DEAD, L5, LOG_NORMAL, 1, FALSE},
  {"penalty", do_penalty, POS_DEAD, L8, LOG_NORMAL, 1, FALSE},
  {"projects", do_projects, POS_DEAD, L8, LOG_NORMAL, 1, FALSE},
  {"echo", do_recho, POS_DEAD, L8, LOG_ALWAYS, 1, FALSE},
  {"return", do_return, POS_DEAD, L6, LOG_NORMAL, 1, FALSE},
  {"retreat", do_retreat, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"snoop", do_snoop, POS_DEAD, L3, LOG_ALWAYS, 1, FALSE},
  {"stat", do_stat, POS_DEAD, L7, LOG_NORMAL, 1, FALSE},
  {"string", do_string, POS_DEAD, 91, LOG_ALWAYS, 1, FALSE},
  {"switch", do_switch, POS_DEAD, L6, LOG_ALWAYS, 1, FALSE},
  {"wizinvis", do_invis, POS_DEAD, L8, LOG_NORMAL, 1, FALSE},
  {"vnum", do_vnum, POS_DEAD, L5, LOG_NORMAL, 1, FALSE},
  {"zecho", do_zecho, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"clone", do_clone, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"grant", do_grant, POS_DEAD, ML, LOG_ALWAYS, 1, FALSE},
  {"wiznet", do_wiznet, POS_DEAD, L8, LOG_NORMAL, 1, FALSE},
  {"immtalk", do_immtalk, POS_DEAD, L8, LOG_NORMAL, 1, FALSE},
  {"imotd", do_imotd, POS_DEAD, 99, LOG_NORMAL, 1, FALSE},
  {":", do_immtalk, POS_DEAD, L8, LOG_NORMAL, 0, FALSE},
  {"ocopy", do_ocopy, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"mcopy", do_mcopy, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"rcopy", do_rcopy, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"edit", do_olc, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"asave", do_asave, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"alist", do_alist, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"blist", do_blist, POS_DEAD, L4, LOG_NORMAL, 1, FALSE},
  {"rlist", do_rlist, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"olist", do_olist, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"mlist", do_mlist, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"resets", do_resets, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"aresets", do_aresets, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"smote", do_smote, POS_DEAD, L6, LOG_NORMAL, 1, FALSE},
  {"prefi", do_prefi, POS_DEAD, IM, LOG_NORMAL, 0, FALSE},
  {"prefix", do_prefix, POS_DEAD, IM, LOG_NORMAL, 1, FALSE},
  //Adeon - 8/27/03 saves object triggers
  {"otlist", do_otlist, POS_DEAD, IM, LOG_NORMAL, 1, FALSE},
  //Akamai - 2/26/99 exitlist, lists all exits from area
  {"exitlist", do_exlist, POS_DEAD, L2, LOG_NORMAL, 1, FALSE},
  //Akamai - 4/15/99 badsummon, turns on and off the nasty summoning
  {"badsummon", do_badsummon, POS_DEAD, L1, LOG_NORMAL, 1, FALSE},
  //Morgan - 9/19/04 exptrack, turns on and off the nasty summoning
  {"exptrack", do_tractivate, POS_DEAD, L1, LOG_NORMAL, 1, FALSE},
  // Morgan - 6/29/2000, nonpk protection, toggles on and off
  {"safenopk", do_safe_nopk, POS_DEAD, L1, LOG_NORMAL, 1, FALSE},
  {"elite", do_elite, POS_DEAD, 0, LOG_NORMAL, 0, FALSE},
  {"edithelp", do_edithelp, POS_DEAD, ML, LOG_NORMAL, 1, FALSE},
  {"remhelp", do_remhelp, POS_DEAD, ML, LOG_NORMAL, 1, FALSE},
  // Akamai - 5/3/99 makehero makes a character a hero
  // while unhero unmakes the hero character
  {"makehero", do_hero, POS_DEAD, L1, LOG_NORMAL, 1, FALSE},
  {"unhero", do_unhero, POS_DEAD, L1, LOG_NORMAL, 1, FALSE},
  // Akamai - 6/24/99 garbage collection function to help with
  // memory leaks - primarily free up objects
  {"gc", do_garbage_collection, POS_DEAD, ML, LOG_NORMAL, 1, FALSE},
  //Reaver related stuff
  {"nogate", do_nogate, POS_DEAD, 93, LOG_ALWAYS, 1, FALSE},
  {"nogateall", do_nogateall, POS_DEAD, 93, LOG_ALWAYS, 1, FALSE},
  {"sword", do_sword, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"invoke", do_invoke, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  //IBLIS 5/19/03 - command to help with conversion
  {"reroll", do_reroll, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"reincarnate", do_reincarnate, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"killable", do_killable, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"arena", do_arena, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"pk", do_pk, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"consent", do_consent, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"suck", do_suck, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"damage", do_damage, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"trecho", do_trecho, POS_DEAD, L8, LOG_ALWAYS, 1, FALSE},
  {"aggression", do_aggression, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"leeching", do_leeching, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"draw", do_draw, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"embalm", do_embalm, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"corrupt", do_corrupt, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"feast", do_feast, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"hellbreath", do_hellbreath, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"unsummon", do_unsummon, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"rant", do_rant, POS_SLEEPING, 0, LOG_NORMAL, 1, TRUE},
  {"rantonly", do_rantonly, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"familiar", do_familiar, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"makenecro", do_makenecro, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"undeadscore", do_undeadscore, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"undeadgain", do_undeadgain, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"puppet", do_puppet, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"defile", do_defile, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"despoil", do_despoil, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"contaminate", do_contaminate, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"flame", do_flame, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"startdouble", do_startdouble, POS_STANDING, 0, LOG_ALWAYS, 1, FALSE},
  {"contribute", do_contribute, POS_STANDING, 0, LOG_ALWAYS, 1, FALSE},
  {"flare", do_flare, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"makelich", do_makelich, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"makekalian", do_makekalian, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"makeavatar", do_makeavatar, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"quest", do_quest, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"questor", do_questor, POS_DEAD, 92, LOG_ALWAYS, 1, FALSE},
  {"elder", do_elder, POS_DEAD, 95, LOG_ALWAYS, 1, FALSE},
  {"question", do_question, POS_SLEEPING, 0, LOG_NORMAL, 1, TRUE},
  {"stripaffects", do_stripaffects, POS_DEAD, 92, LOG_ALWAYS, 1, FALSE},
  {"lightningbreath", do_lightningbreath, POS_FIGHTING, 0, LOG_NORMAL, 1,
   FALSE},
  {"swoop", do_swoop, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"makenerix", do_makenerix, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"poll", do_poll, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"sayto", do_sayto, POS_RESTING, 0, LOG_NORMAL, 1, TRUE},
  {"whisper", do_whisper, POS_RESTING, 0, LOG_NORMAL, 1, TRUE},
  {"gemote", do_gemote, POS_RESTING, 0, LOG_NORMAL, 1, TRUE},
  {"gpmote", do_gpmote, POS_RESTING, 0, LOG_NORMAL, 1, TRUE},
  {"nogemote", do_nogemote, POS_DEAD, 92, LOG_ALWAYS, 1, FALSE},
  {"no_delete", do_nodelete, POS_DEAD, 92, LOG_ALWAYS, 1, FALSE},
  {"toll", do_toll, POS_DEAD, 92, LOG_NORMAL, 1, FALSE},
  {"bubble", do_bubble, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"spear", do_spear, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"locate", do_locate, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"find", do_find, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"swim", do_swim, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"transform", do_transform, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"ignoreac", do_ignoreac, POS_DEAD, ML, LOG_NORMAL, 1, FALSE},
  {"ignorehitroll", do_ignorehitroll, POS_DEAD, ML, LOG_NORMAL, 1, FALSE},
  {"ignoresave", do_ignoresave, POS_DEAD, ML, LOG_NORMAL, 1, FALSE},
  {"paddle", do_paddle, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"forcetick", do_forcetick, POS_DEAD, ML, LOG_NORMAL, 1, FALSE},
  {"newac", do_newac, POS_DEAD, ML, LOG_NORMAL, 1, FALSE},
  {"blagbgone", do_blagbgone, POS_DEAD, 92, LOG_NORMAL, 1, FALSE},
  {"opurge", do_opurge, POS_DEAD, 0, LOG_ALWAYS, 1, FALSE},
  {"chop", do_chop, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"stop", do_stop, POS_SITTING, 0, LOG_NORMAL, 1, FALSE},
  {"mill", do_mill, POS_SITTING, 0, LOG_NORMAL, 1, FALSE},
  {"release", do_release, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"binder", do_binder, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"deck", do_deck, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"appraise", do_appraise, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"newfix", do_newfix, POS_DEAD, ML, LOG_NORMAL, 1, FALSE},
  {"cardcheck", do_cardcheck, POS_DEAD, 92, LOG_NORMAL, 1, FALSE},
  {"cardduel", do_cardduel, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"ante", do_ante, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"cardkill", do_cardkill, POS_SLEEPING, 0, LOG_NORMAL, 1, FALSE},
  {"runecard", do_runecard, POS_DEAD, 95, LOG_NORMAL, 1, FALSE},
  {"newfix", do_newfix, POS_DEAD, ML, LOG_NORMAL, 1, FALSE},
  {"closestore", do_closestore, POS_STANDING, 0, LOG_ALWAYS, 1, FALSE},
  {"openstore", do_openstore, POS_STANDING, 0, LOG_ALWAYS, 1, FALSE},
  {"install", do_install, POS_SITTING, 0, LOG_ALWAYS, 1, FALSE},
  {"disarmtrap", do_disarmtrap, POS_SITTING, 0, LOG_NORMAL, 1, FALSE},
  {"qbitset", do_qbitset, POS_DEAD, 92, LOG_NORMAL, 1, FALSE},
  {"qbitunset", do_qbitunset, POS_DEAD, 92, LOG_NORMAL, 1, FALSE},
  {"qbitget", do_qbitget, POS_DEAD, 92, LOG_NORMAL, 1, FALSE},
  {"cardcount", do_cardcount, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"present", do_present, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"otlistall", do_otlistall, POS_DEAD, IM, LOG_NORMAL, 1, FALSE},
  {"makecj", do_makecj, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"makenidae", do_makenidae, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"warptouch", do_warptouch, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"madrush", do_madrush, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"coma", do_coma, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"vanish", do_vanish, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"persuade", do_persuade, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"maddance", do_maddance, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"firedance", do_firedance, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"escape", do_escape, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"noswitch", do_noswitch, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"flick", do_flick, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"shuffle", do_shuffle, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"lucky throw", do_luckythrow, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
  {"friend", do_friend, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"friendscore", do_undeadscore, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"box", do_box, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"friendname", do_friendname, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"friendxp", do_friendxp, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"makenewbclannie", do_makenewbclannie, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"checknplayers", do_checknplayers, POS_DEAD, 100, LOG_NORMAL, 1, FALSE},
  {"dig", do_dig, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"revert", do_revert, POS_DEAD, 0, LOG_NORMAL, 1, FALSE},
  {"makedruid", do_makedruid, POS_DEAD, L5, LOG_ALWAYS, 1, FALSE},
  {"befriend", do_befriend, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"shillelagh", do_shillelagh, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  {"shapechange", do_shapechange, POS_STANDING, 0, LOG_NORMAL, 1, FALSE},
  {"wither", do_shillelaghdismiss, POS_RESTING, 0, LOG_NORMAL, 1, FALSE},
  /*{"smother", do_smother, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE},
     {"attack", do_attack, POS_FIGHTING, 0, LOG_NORMAL, 1, FALSE}, */
  {"checkrp", do_checkrp, POS_DEAD, 91, LOG_NORMAL, 1, FALSE},
  {"", 0, POS_DEAD, 0, LOG_NORMAL, 0, FALSE}
};
void
do_multi (CHAR_DATA * ch, char *argument)
{
  char command[MAX_INPUT_LENGTH];
  char tmparg[MAX_INPUT_LENGTH], mycmd[MAX_INPUT_LENGTH];
  bool found;
  int times, cmd;
  if (IS_NPC (ch))
    return;
  argument = one_argument (argument, command);
  if ((times = atoi (command)) != 0)
    {
      if (times > 10)
	times = 10;
      strcpy (tmparg, argument);
      one_argument (tmparg, mycmd);
      found = FALSE;
      for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
	{
	  if (mycmd[0] == cmd_table[cmd].name[0]
	      && !str_prefix (mycmd, cmd_table[cmd].name)
	      && (cmd_table[cmd].level <= ch->trust
		  || cmd_table[cmd].level <= ch->level))
	    {
	      found = TRUE;
	      if (cmd_table[cmd].spams)
		{
		  sprintf (mycmd,
			   "Do you really feel the need to do that %d times?\n\r",
			   atoi (command));
		  send_to_char (mycmd, ch);
		  return;
		}
	      break;
	    }
	}
      if (!found)
	{
	  send_to_char ("Huh?\n\r", ch);
	  return;
	}
      interpret (ch, argument);
      if (times > 1)
	{
	  sprintf (ch->desc->incomm, "#%d %s", times - 1, argument);
	  keepincomm = TRUE;
	}
    }
  else
    send_to_char ("Huh?\n\r", ch);
}


/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void
interpret (CHAR_DATA * ch, char *argument)
{
  char command[MAX_STRING_LENGTH];

  //char buf[MAX_STRING_LENGTH];
  char logline[MAX_INPUT_LENGTH], *cptr;
  int cmd;
  int trust;
  int position;
  bool found;
  short x;

  /*
   * Strip leading spaces.
   */
  while (isspace (*argument))
    argument++;
  if (argument[0] == '\0')
    return;
  cptr = argument;
  while (*cptr != '\0')
    {
      if (*cptr == 7)
	*cptr = ';';
      cptr++;
    }

  /*
   * No hiding.
   */
  if (!IS_NPC (ch))
    {
      if (argument[0] == '!')
	{
	  if (argument[1] == '?')
	    {
	      char big_buf[MAX_STRING_LENGTH * 2];
	      strcpy (big_buf, "Command History:\n\r\n\r");
	      for (x = MAX_HISTORY - 1; x >= 0; x--)
		if (ch->pcdata->history[x] != NULL)
		  sprintf (big_buf + strlen (big_buf),
			   "%2d.\t%s\n\r", x + 1, ch->pcdata->history[x]);
	      page_to_char (big_buf, ch);
	      return;
	    }
	  else if (argument[1] == '\0' || argument[1] == '!')
	    {
	      if (ch->pcdata->history[0] != NULL)
		{
		  strcpy (command, ch->pcdata->history[0]);
		  interpret (ch, command);
		  return;
		}
	    }
	  else
	    {
	      short num;
	      num = atoi (argument + 1);
	      if (num > 0 && num < 21)
		{
		  if (ch->pcdata->history[num - 1] != NULL)
		    {
		      strcpy (command, ch->pcdata->history[num - 1]);
		      interpret (ch, command);
		    }
		  else
		    send_to_char ("No such history entry.\n\r", ch);
		  return;
		}
	      else
		{
		  send_to_char ("Huh?\n\r", ch);
		  return;
		}
	    }
	}
      else
	{
	  if (ch->pcdata->history[19] != NULL)
	    free_string (ch->pcdata->history[19]);
	  for (x = 18; x >= 0; x--)
	    if (ch->pcdata->history[x] != NULL)
	      ch->pcdata->history[x + 1] = ch->pcdata->history[x];
	  ch->pcdata->history[0] = str_dup (argument);
	}
    }

  /*
   * Implement freeze command.
   */
  if (!IS_NPC (ch) && IS_SET (ch->act, PLR_FREEZE))
    {
      send_to_char ("You're totally frozen!\n\r", ch);
      return;
    }

  /*
   * Grab the command word.
   * Special parsing so ' can be a command,
   *   also no spaces needed after punctuation.
   */
  strcpy (logline, argument);
  if (ch->in_room)
    sprintf (last_command, "%s in room[%d]: %s.", ch->name,
	     ch->in_room->vnum, argument);
  else
    sprintf (last_command, "%s in Null room: %s.", ch->name, argument);
  if (!isalpha (argument[0]) && !isdigit (argument[0]))
    {
      command[0] = argument[0];
      command[1] = '\0';
      argument++;
      while (isspace (*argument))
	argument++;
    }
  else
    {
      argument = one_argument (argument, command);
    }

  /*
   * Look for command in command table.
   */
  found = FALSE;
  trust = get_trust (ch);
  for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
    {
      if (command[0] == cmd_table[cmd].name[0]
	  && !str_prefix (command, cmd_table[cmd].name)
	  && cmd_table[cmd].level <= trust)
	{
	  found = TRUE;
	  break;
	}
    }
  if (ch->in_room != NULL && !IS_NPC (ch) && !found)
    {
      CHAR_DATA *mob;
      TRIGGER_DATA *trig;
      for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room)
	{
	  if (IS_NPC (mob))
	    {
	      for (trig = mob->triggers; trig != NULL; trig = trig->next)
		{
		  if (trig->trigger_type ==
		      TRIG_COMMAND && !str_prefix (command, trig->keywords))
		    {
		      if (trig->current == NULL)
			{
			  act_trigger (mob, trig->name, argument,
				       NAME (ch), NAME (mob));
			  trig->current = trig->script;
			  trig->bits = SCRIPT_ADVANCE;
			}
		      found = TRUE;
		    }
		}
	    }
	}
      if (found)
	return;
    }

  /*
   * Log and snoop.
   */
  if (cmd_table[cmd].log == LOG_NEVER || cmd_table[cmd].log == LOG_PASSWORD)
    strcpy (logline, "");
  if (cmd_table[cmd].log == LOG_PASSWORD)
    {
      sprintf (log_buf, "Log: %s: %s", ch->name, cmd_table[cmd].name);
      wiznet (log_buf, ch, NULL, WIZ_SECURE, 0, get_trust (ch));
      log_string (log_buf);
    }
  else if ((!IS_NPC (ch) && IS_SET (ch->act, PLR_LOG))
	   || fLogAll || (cmd_table[cmd].log == LOG_ALWAYS
			  && ch->desc != NULL))
    {
      char s[2 * MAX_INPUT_LENGTH], *ps;
      int i;
      ps = s;
      sprintf (log_buf, "Log %s: %s", ch->name, logline);

      /* Make sure that was is displayed is what is typed */
      for (i = 0; log_buf[i]; i++)
	{
	  *ps++ = log_buf[i];
	  if (log_buf[i] == '$')
	    *ps++ = '$';
	  if (log_buf[i] == '{')
	    *ps++ = '{';
	}
      *ps = 0;
      wiznet (s, ch, NULL, WIZ_SECURE, 0, get_trust (ch));
      log_string (log_buf);
    }
  if (!IS_NPC (ch) && ch->pcdata && ch->pcdata->cg_state >= CG_START_PM_YT
      && ch->pcdata->cg_state <= CG_NYT_DONE)
    {
      if (!str_prefix (command, "a") || !str_prefix (command, "A")
	  || !str_prefix (command, "b") || !str_prefix (command, "B")
	  || !str_prefix (command, "c") || !str_prefix (command, "C")
	  || !str_prefix (command, "d") || !str_prefix (command, "D")
	  || !str_prefix (command, "e") || !str_prefix (command, "E")
	  || !str_prefix (command, "f") || !str_prefix (command, "F")
	  || !str_prefix (command, "g") || !str_prefix (command, "G")
	  || !str_prefix (command, "y") || !str_prefix (command, "Y")
	  || !str_prefix (command, "z") || !str_prefix (command, "Z"))
//                   command[0] != '\0' && command[1] == '\0'
	{
/*       char buf2[MAX_INPUT_LENGTH];
       sprintf(buf2,"!%s! - %d",command,strlen(command));
       do_echo(ch,buf2);*/
	  cardgame_menu (ch, command);
	  return;
	}
    }
  if (!found)
    {

      /*
       * Look for command in socials table.
       */
      if (ch->race == PC_RACE_SWARM || !check_social (ch, command, argument))
	{
	  if (!IS_NPC (ch) && ch->pcdata
	      && ch->pcdata->cg_state >= CG_START_PM_YT
	      && ch->pcdata->cg_state <= CG_NYT_DONE)
	    cardgame_menu (ch, command);
	  else
	    send_to_char ("Huh?\n\r", ch);
	}
      return;
    }
  if (cmd_table[cmd].position > POS_SLEEPING)
    REMOVE_BIT (ch->affected_by, AFF_HIDE);
  if (cmd_table[cmd].position > POS_SLEEPING)
    {
      REMOVE_BIT (ch->affected_by, AFF_CAMOUFLAGE);
      //Iblis - If the character does much, they stop feigning death
      if (ch->position == POS_FEIGNING_DEATH)
	{
	  send_to_char ("You stop feigning death.\n\r", ch);
	  ch->position = POS_STANDING;
	}
    }


  /*
   * Character not in position for command?
   */

  position = get_position (ch);

  if (!IS_NPC (ch) && ch->pcdata && ch->pcdata->cg_state != 0)
    {
      if (cmd_table[cmd].position > POS_RESTING)
	{
	  send_to_char ("You can't do that while playing cards.\n\r", ch);
	  return;
	}
    }
  if (position < cmd_table[cmd].position)
    {
      if (!IS_NPC (ch))
	ch->pcdata->speedlen = 0;
      if (ch->position == POS_STANDING && ch->fighting != NULL)
	ch->position = POS_FIGHTING;
      switch (ch->position)
	{
	case POS_DEAD:
	  send_to_char ("Lie still; you are DEAD.\n\r", ch);
	  break;
	case POS_MORTAL:
	case POS_INCAP:
	  send_to_char ("You are hurt far too bad for that.\n\r", ch);
	  break;
	case POS_STUNNED:
	  send_to_char ("You are too stunned to do that.\n\r", ch);
	  break;
	case POS_SLEEPING:
	  send_to_char ("In your dreams, or what?\n\r", ch);
	  break;
	case POS_RESTING:
	  send_to_char ("Nah... You feel too relaxed...\n\r", ch);
	  break;
	case POS_CHOPPING:
	  send_to_char ("You're too busy chopping!\n\r", ch);
	  break;
	case POS_MILLING:
	  send_to_char ("You're too busy milling!\n\r", ch);
	  break;
	case POS_CARVING:
	  send_to_char ("You're too busy carving!\n\r", ch);
	  break;
	case POS_SITTING:
	  send_to_char ("Better stand up first.\n\r", ch);
	  break;
	case POS_FIGHTING:
	  send_to_char ("No way!  You are still fighting!\n\r", ch);
	  break;
	}
      return;
    }

  /*
   * Dispatch the command.
   */

  //Iblis - This is a cheap hack to limit the commands a bone puppet can perform
  if (IS_SET (ch->act2, ACT_PUPPET))
    {
      bool okcommand = FALSE;
      if (IS_NPC (ch))
	{
	  if (ch->pIndexData->vnum == MOB_VNUM_FROG
	      || ch->pIndexData->vnum == MOB_VNUM_WARPED_CAT
	      || ch->pIndexData->vnum == MOB_VNUM_WARPED_OWL)
	    {
	      if (!str_cmp (cmd_table[cmd].name, "say")
		  || !str_cmp (cmd_table[cmd].name, "sayto")
		  || !str_cmp (cmd_table[cmd].name, "whisper")
		  || !str_cmp (cmd_table[cmd].name, "kill"))
		okcommand = TRUE;
	      if (ch->pIndexData->vnum == MOB_VNUM_FROG)
		{
		  if (!str_cmp (cmd_table[cmd].name, "cast"))
		    okcommand = TRUE;
		}
	      else if (ch->pIndexData->vnum == MOB_VNUM_WARPED_OWL)
		{
		  if (!str_cmp (cmd_table[cmd].name, "swoop"))
		    okcommand = TRUE;
		}
	    }
	}
      if (!okcommand &&
	  !(!str_cmp (cmd_table[cmd].name, "east")
	    || !str_cmp (cmd_table[cmd].name, "west")
	    || !str_cmp (cmd_table[cmd].name, "exits")
	    || !str_cmp (cmd_table[cmd].name, "north")
	    || !str_cmp (cmd_table[cmd].name, "south")
	    || !str_cmp (cmd_table[cmd].name, "look")
	    || !str_cmp (cmd_table[cmd].name, "where")
	    || !str_cmp (cmd_table[cmd].name, "emote")
	    || !str_cmp (cmd_table[cmd].name, "exits")
	    || !str_cmp (cmd_table[cmd].name, "enter")
	    || !str_cmp (cmd_table[cmd].name, "exit")
	    || !str_cmp (cmd_table[cmd].name, "climb")
	    || !str_cmp (cmd_table[cmd].name, "descend")
	    || !str_cmp (cmd_table[cmd].name, "scale")
	    || !str_cmp (cmd_table[cmd].name, "board")
	    || !str_cmp (cmd_table[cmd].name, "up")
	    || !str_cmp (cmd_table[cmd].name, "down")
	    || !str_cmp (cmd_table[cmd].name, "open")
	    || !str_cmp (cmd_table[cmd].name, "puppet")
	    || !str_cmp (cmd_table[cmd].name, "revert")))

	{
	  send_to_char ("You cannot perform that command.\n\r", ch);
	  return;
	}
    }

  (*cmd_table[cmd].do_fun) (ch, argument);
  tail_chain ();
  return;
}

bool check_social (CHAR_DATA * ch, char *command, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int cmd;
	bool found;

	found = FALSE;
	for (cmd = 0; social_table[cmd].name[0] != '\0'; cmd++) {
		if (command[0] == social_table[cmd].name[0] && !str_prefix (command, social_table[cmd].name)) {
			found = TRUE;
			break;
		}
	}
	if (!found)
		return FALSE;

	if (!IS_NPC (ch) && IS_SET (ch->comm, COMM_NOEMOTE)) {
		send_to_char ("You are anti-social!\n\r", ch);
		return TRUE;
	}

	switch (ch->position) {
		case POS_DEAD:
			send_to_char ("Lie still; you are DEAD.\n\r", ch);
			return TRUE;
		case POS_INCAP:
		case POS_MORTAL:
			send_to_char ("You are hurt far too bad for that.\n\r", ch);
			return TRUE;
		case POS_STUNNED:
			send_to_char ("You are too stunned to do that.\n\r", ch);
			return TRUE;
		case POS_SLEEPING:

			/*
			 * I just know this is the path to a 12" 'if' statement.  :(
			 * But two players asked for it already!  -- Furey
			 */
			if (!str_cmp (social_table[cmd].name, "snore"))
				break;
			send_to_char ("In your dreams, or what?\n\r", ch);
			return TRUE;
	}
	if (ch->in_room == NULL)
		return TRUE;
	if (IS_SET (ch->in_room->room_flags, ROOM_SILENT) && !IS_IMMORTAL (ch)) {
		send_to_char("This room is shielded from disturbance by powerful wards of magic.\n\r", ch);
		return TRUE;
	}
	one_argument (argument, arg);
	victim = NULL;
	if (arg[0] == '\0') {
		act (social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM);
		act (social_table[cmd].char_no_arg, ch, NULL, victim, TO_CHAR);
	} else if ((victim = get_char_room (ch, arg)) == NULL) {
		send_to_char ("They aren't here.\n\r", ch);
	} else if (victim == ch) {
		act (social_table[cmd].others_auto, ch, NULL, victim, TO_ROOM);
		act (social_table[cmd].char_auto, ch, NULL, victim, TO_CHAR);
	} else {
		act (social_table[cmd].others_found, ch, NULL, victim, TO_NOTVICT);
		act (social_table[cmd].char_found, ch, NULL, victim, TO_CHAR);
		act (social_table[cmd].vict_found, ch, NULL, victim, TO_VICT);
		if (!IS_NPC (ch) && IS_NPC (victim) && !IS_AFFECTED (victim, AFF_CHARM) && IS_AWAKE (victim) && victim->desc == NULL) {
			switch (number_bits (4)) {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
					act (social_table[cmd].others_found, victim, NULL, ch, TO_NOTVICT);
					act (social_table[cmd].char_found, victim, NULL, ch, TO_CHAR);
					act (social_table[cmd].vict_found, victim, NULL, ch, TO_VICT);
					break;
				case 9:
				case 10:
				case 11:
				case 12:
					act ("`N$n slaps $N.``", victim, NULL, ch, TO_NOTVICT);
					act ("`NYou slap $N.``", victim, NULL, ch, TO_CHAR);
					act ("`N$n slaps you.``", victim, NULL, ch, TO_VICT);
					break;
			}
		}
	}
	if (victim != NULL) {
		if (HAS_SCRIPT (victim)) {
			TRIGGER_DATA *pTrig;
			for (pTrig = victim->triggers; pTrig != NULL; pTrig = pTrig->next) {
				if (pTrig->trigger_type == TRIG_SOCIAL && pTrig->current == NULL && !IS_SET (victim->act, ACT_HALT)) {
					act_trigger (victim, pTrig->name, social_table[cmd].name, NAME (ch), NAME (victim));
					pTrig->current = pTrig->script;
					pTrig->bits = SCRIPT_ADVANCE;
				}
			}
		}
	}
	return TRUE;
}


/*
 * Return true if an argument is completely numeric.
 */
bool
is_number (char *arg)
{
  if (*arg == '\0')
    return FALSE;
  if (*arg == '+' || *arg == '-')
    arg++;
  for (; *arg != '\0'; arg++)
    {
      if (!isdigit (*arg))
	return FALSE;
    }
  return TRUE;
}


/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int
number_argument (char *argument, char *arg)
{
  char *pdot;
  int number;
  for (pdot = argument; *pdot != '\0'; pdot++)
    {
      if (*pdot == '.')
	{
	  *pdot = '\0';
	  number = atoi (argument);
	  *pdot = '.';
	  strcpy (arg, pdot + 1);
	  return number;
	}
    }
  strcpy (arg, argument);
  return 1;
}


/* 
 * Given a string like 14*foo, return 14 and 'foo'
 */
int
mult_argument (char *argument, char *arg)
{
  char *pdot;
  int number;
  for (pdot = argument; *pdot != '\0'; pdot++)
    {
      if (*pdot == '*')
	{
	  *pdot = '\0';
	  number = atoi (argument);
	  *pdot = '*';
	  strcpy (arg, pdot + 1);
	  return number;
	}
    }
  strcpy (arg, argument);
  return 1;
}


/*
 * Contributed by Alander.
 */
void
do_commands (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int cmd;
  int col;
  col = 0;
  for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
    {
      if (cmd_table[cmd].level < LEVEL_HERO
	  && cmd_table[cmd].level <= get_trust (ch) && cmd_table[cmd].show)
	{
	  sprintf (buf, "%-12s", cmd_table[cmd].name);
	  send_to_char (buf, ch);
	  if (++col % 6 == 0)
	    send_to_char ("\n\r", ch);
	}
    }
  if (col % 6 != 0)
    send_to_char ("\n\r", ch);
  return;
}

void
do_newwizhelp (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int cmd;
  int col;
  int clevel;
  col = 0;

  for (clevel = LEVEL_HERO + 1; clevel < MAX_LEVEL + 1; clevel++)
    {
      for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
	{
	  if (cmd_table[cmd].level >= LEVEL_HERO
	      && cmd_table[cmd].level <= get_trust (ch)
	      && cmd_table[cmd].show && cmd_table[cmd].level == clevel)
	    {
	      sprintf (buf, "[%-3d] %-12s", cmd_table[cmd].level,
		       cmd_table[cmd].name);
	      send_to_char (buf, ch);
	      if (++col % 5 == 0)
		send_to_char ("\n\r", ch);
	    }
	}
    }

  if (col % 5 != 0)
    send_to_char ("\n\r", ch);
  return;
}


// Akamai 5/3/99 - Fix columns for wiz commands
void
do_wizhelp (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char command_buff[MAX_STRING_LENGTH];
  char command_buff2[MAX_STRING_LENGTH];
  int level = 0;
  bool match = FALSE;
  int cmd;
  int col;
  argument = one_argument (argument, arg);
  level = atoi (arg);

  // for showing level of commands when using level argument
  if (level > 91 && level <= get_trust (ch))
    {
      sprintf (buf, "Level `o%d`` commands:\n\r", level);
      send_to_char (buf, ch);
    }
  buf[0] = '\0';
  col = 0;
  for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
    {
      if ((cmd_table[cmd].level >= LEVEL_HERO) &&
	  (cmd_table[cmd].level <= get_trust (ch)) && cmd_table[cmd].show)
	{
	  if (level != 0)
	    {
	      if (cmd_table[cmd].level == level && level <= get_trust (ch))
		{
		  match = TRUE;
		  sprintf (buf, "%-15s", cmd_table[cmd].name);
		}
	      else
		match = FALSE;
	    }
	  else
	    {
	      sprintf (command_buff, "%s", cmd_table[cmd].name);

	      //Shinowlan -- 06/26/2000 -- "level" option to print levels of wizcommands
	      if (arg[0] != '\0' && !str_prefix (arg, "level"))
		{
		  sprintf (command_buff2, "(%d)", cmd_table[cmd].level);
		  strcat (command_buff, command_buff2);
		}
	      //sprintf(buf, "%-18s(%d)", cmd_table[cmd].name, cmd_table[cmd].level);
	      sprintf (buf, "%-15s", command_buff);
	      command_buff[0] = '\0';
	      match = TRUE;
	    }
	  send_to_char (buf, ch);
	  buf[0] = '\0';
	  if (match)
	    if (++col % 5 == 0)
	      send_to_char ("\n\r", ch);
	}
    }
  if (col % 6 != 0)
    send_to_char ("\n\r", ch);
  return;
}
