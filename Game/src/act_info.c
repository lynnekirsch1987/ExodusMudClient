#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <crypt.h>
#include <unistd.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "ansi.h"
#include "olc.h"
#include "heros.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"

/* command procedures needed */
DECLARE_DO_FUN (do_exits);
DECLARE_DO_FUN (do_look);
DECLARE_DO_FUN (do_help);
DECLARE_DO_FUN (do_affects);
DECLARE_DO_FUN (do_play);
DECLARE_DO_FUN (do_color);
DECLARE_DO_FUN (do_scanning);
extern bool quad;
extern bool doub;
extern bool half;
extern bool battle_royale;
extern int quad_timer;
extern int double_timer;
extern int half_timer;
extern int battle_royale_timer;
extern CHAR_DATA *br_leader;
extern int setbit args ((char *explored, int index));
extern int getbit args ((char *explored, int index));
extern int arearooms args ((CHAR_DATA * ch));
extern int areacount args ((CHAR_DATA * ch));
extern int roomcount args ((CHAR_DATA * ch));
extern long int level_cost (int level);
void sense_life (CHAR_DATA * ch);
char *condition_noun args ((int condition));
void send_card_stats args((OBJ_DATA *obj, CHAR_DATA *ch));
void appraise args((CHAR_DATA *ch, OBJ_DATA* card));
bool can_move_char args ((CHAR_DATA * ch, ROOM_INDEX_DATA * to_room, bool follow, bool show_messages));

char imm_who_msg[MAX_STRING_LENGTH] = "remove";

void do_sword args((CHAR_DATA * ch, char *argument));
void save_cardcount args((void));

/* do_scanning */
char *ocean_desc[] = {
	"The cold, rolling crystal aquamarine waves wash over your feet, sending\n\r" "stinging sensations up and down your spine.  As you look all around you,\n\r" "you realize with just the tiniest sense of despair that their is no end in\n\r" "sight.  The rippling water surface shimmers like a mirror that has been\n\r" "placed to face the sky, to sate the gods' appetites for drink, a massive\n\r" "pool of swirling salt and water that makes you both ache for dry land, and\n\r" "wish for more.  It is impossible to tell where you are on this ocean, the\n\r" "slightest turn from the smallest wave has probably already set you off\n\r" "course.\n\r",	/* 0 */
	"The frothing blue-green sea rolls off from here to the curving horizon.\n\r"
	"Heavy swells rise and fall steadily and silently, crawling towards the coast\n\r"
	"where they will break on the sand. The murky waters are dark, and you can\n\r"
	"make out life living beneath them in myriad strands of seaweed.\n\r\r",

	/* 1 */
	"A steady cycle of swells pass beneath you and disappear into the distance.\n\r"
	"A hundred blue-green hill-tops rolling silently by, peaceful waters slowly\n\r"
	"grinding into sand a thousand rocks, shells, and sunken ships.  The waters\n\r"
	"are murky, and nothing can be seen floating on the swirling surface.\n\r",

	/* 2 */
	"The dark waters look deceptively peaceful, and as empty as a starless sky.\n\r" "Murky waves roll smoothly and silently by, occasional white crests of foam\n\r" "gathering on their tops only to disappear again into the blue-green waters.\n\r" "The calm swells hiding the chaos beneath the surface, waiting for release\n\r" "on sandy beaches or in raging storms.\n\r",	/* 3 */
	"Light refracts off the dancing waves of the water, sending sparkles of shine\n\r" "and glisten all around you. It is a good thing that you find that\n\r" "interesting, because there isn't much else around you.  Water, water, water\n\r" "and air, their colors blending into one perfectly woven mesh, a blur of\n\r" "grey and blue that never seems to end, and never even seems to begin.  It is\n\r" "a paradox that only one lost at sea has the time or inclination to ponder.\n\r" "The churning of the waves rock you back and forth, a soothing motion that\n\r" "soon has you feeling sick.  Yet another paradox in this massive pool of\n\r" "salt and water.\n\r",	/* 4 */
	"You ponder your existence as you drift along, your thoughts your only\n\r" "companion.  In the distance a bird dives for a fish.  Suddenly, a ripple\n\r" "oscillates around you, almost startling you overboard.  Dark shapes glide by,\n\r" "on errands only they understand.  Some of the shapes are short and round,\n\r" "some are long and skinny, while still others are in shapes impossible to\n\r" "describe.  A few are very large.  You just hope they have plenty more\n\r" "appetizing meals down below...\n\r",	/* 5 */
	"The glistening sunlight is slowly driving you insane.  It stabs into your\n\r"
	"eyes and beats into your brain.  You wonder if the tinge of salt enveloping\n\r"
	"you will ever dissipate. Clouds and darkness are your only solace,\n\r"
	"providing a temporary, uneasy peace.  But even the clouds are not a\n\r"
	"completely welcome sight.  You can see faces in them, as if mocking you for\n\r"
	"your mortal attempt at mastering such a vast entity, the endless ocean.\n\r"
	"Maybe the pain from the stark, desolate light is better...\n\r",

	/* 6 */
	"You begin to think the rhythm of the universe is reflected in the steady\n\r" "movement of the waves.  Up and down...back and forth.  As constant as the\n\r" "stars, as eternal as the earth.  The water begins to look warm and inviting,\n\r" "its promise of death becoming preferable to the endless beating of the sun\n\r" "and sway of the waves. Anything to end the monotony of the ceaseless blue\n\r" "horizon. Time has no meaning here - here where the cycles of life display\n\r" "themselves in unchanging rhythms of waves.\n\r",	/* 7 */
	"Undulating cerulean waves caress the bottom of your seacraft as you continue\n\r"
	"your endless journey across this vast uncharted ocean.  Save for the\n\r"
	"occasional reflections of light gleaming off intrepid crests which spray\n\r"
	"froth over the bow of your ship, the sole movement across the boundless\n\r"
	"horizon comes from gulls swooping low to snatch fish in their protracted\n\r"
	"beaks or creatures splashing in your wake.  So remote from the madness of\n\r"
	"complication, this extremity of solitude forces you to throw aside the\n\r"
	"facades which have buffeted you, so that at this particular moment, you\n\r"
	"stand precariously on the edge of epiphany and utter despair.\n\r",

	/* 8 */
	"The ocean churns on, sea-foam and shells, driving skeletal once-fish and the\n\r"
	"glassy-crystal remains of a jellyfish broken by a storm.  The waters swirl\n\r"
	"on, salty and dark, grinding to sand the last stones of what once was a\n\r"
	"mountain.  The cycle rolls on, a clump of greenish seaweed riding the froth\n\r"
	"of a wave from the other side of the world.  A perpetual inventory of the\n\r"
	"sea, awash with debris, alive and dead, inevitable and silent.\n\r",

	/* 9 */
	"A plethora of blue-green colors is the entirety of existence here.  Light\n\r" "shimmering off the water gives the ocean an appearance of lacking depth,\n\r" "but the dimensions of the crashing waves and tumbling current render that\n\r" "perception false.  The clear sky poses as a reflection of the sea, using the\n\r" "horizon as a point of symmetry.  The monotony is at once tranquil and\n\r" "disturbing.\n\r",	/* 10 */
	"The sounds of tumbling currents drone on, and the colors of the sea behave\n\r"
	"similarly.  There appears to be no end in sight. The landscape is broken\n\r"
	"only by the occasional odd sea organism leaping out of and then back into\n\r"
	"the endless pool of water.  The sky mirrors this loneliness as the blue\n\r"
	"above is marred only by the random cloud streaking across it.  Peering far\n\r"
	"and wide, there is only this infinite world of blues to be seen.\n\r"
	/* 11 */
};
char *ac_info (CHAR_DATA *, int);
extern const char *dir_name[];
extern bool is_encrypted;
char *const dist_name[] =
{ "right here", "close by", "not far off", "a short distance away",
"rather far off", "in the distance", "almost out of sight"
};
const char * dir_desc[] = { "to the north", "to the east", "to the south", "to the west", "upwards", "downwards" };
char *const where_name[] = { "<used as light>     ", "<worn on finger>    ",
"<worn on finger>    ", "<worn around neck>  ",
"<worn around neck>  ",
"<worn on torso>     ", "<worn on head>      ",
"<worn on legs>      ",
"<worn on feet>      ", "<worn on hands>     ",
"<worn on arms>      ",
"<worn as shield>    ", "<worn about body>   ",
"<worn about waist>  ",
"<worn around wrist> ", "<worn around wrist> ",
"<wielded right>     ",
"<wielded left>      ", "<held>              ",
"<floating nearby>   ", "<tattoed>           ",
"<worn on face>      ", "<worn in ears>      ",
"<worn as crest>     ", "<Mark of Clan>      ",
"<worn on wings>     ",
};

/* for do_count */
int max_on = 0;
void do_battlecry (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	if (IS_NPC (ch))
	{
		send_to_char ("NPCs cannot have battlecries.\n\r", ch);
		return;
	}
	if (!str_cmp (argument, "delete"))
	{
		free_string (ch->pcdata->battlecry);
		ch->pcdata->battlecry = str_dup ("");
		send_to_char ("Battlecry deleted.\n\r", ch);
		return;
	}
	if (argument[0] == '\0')
	{
		if (ch->pcdata->battlecry[0] == '\0')
			sprintf (buf, "<none>\n\r");

		else
			sprintf (buf, "%s %s``\n\r", ch->name, ch->pcdata->battlecry);
		send_to_char ("Syntax: battlecry <battle cry message>\n\r", ch);
		send_to_char ("        battlecry delete\n\r\n\r", ch);
		send_to_char ("Your present battlecry is:\n\r", ch);
		send_to_char (buf, ch);
		return;
	}
	argument[65] = '\0';
	free_string (ch->pcdata->battlecry);
	ch->pcdata->battlecry = str_dup (argument);
	send_to_char ("Battlecry changed.\n\r", ch);
	return;
}

void do_plan (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;
	if (argument[0] != '\0')
	{
		if (!str_cmp ("delete", argument))
		{
			send_to_char ("Your finger information has been deleted.\n\r", ch);
			free_string (ch->pcdata->plan);
			ch->pcdata->plan = str_dup ("");
			return;
		}

		else
		{
			send_to_char
				("Syntax: plan         (Edit your finger information)\n\r", ch);
			send_to_char
				("        plan delete  (Delete your current finger information)\n\r",
				ch);
			return;
		}
	}
	send_to_char
		("Please use the Exodus Editor to enter your finger information.\n\r\n\r",
		ch);
	ch->desc->editor = FINGER_INFO;
	edit_string (ch, &ch->pcdata->plan);
	return;
}

void do_description (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;
	if (argument[0] != '\0')
	{
		if (!str_cmp ("delete", argument))
		{
			send_to_char ("Your description has been deleted.\n\r", ch);
			free_string (ch->description);
			ch->description = str_dup ("");
			return;
		}

		else
		{
			send_to_char
				("Syntax: description         (Edit your description)\n\r", ch);
			send_to_char
				("        description delete  (Delete your current description)\n\r",
				ch);
			return;
		}
	}
	send_to_char
		("Please use the Exodus Editor to enter your description.\n\r\n\r", ch);
	ch->desc->editor = DESC_INFO;
	edit_string (ch, &ch->description);
	return;
}

void do_finger (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim = NULL, *original;
	FILE *ffp;
	char to_finger[MAX_INPUT_LENGTH], plan[10 * MAX_STRING_LENGTH] = "";
	char host[MAX_STRING_LENGTH];
	char email[MAX_STRING_LENGTH];
	char referrer[MAX_STRING_LENGTH];
	char tcbuf[MAX_STRING_LENGTH], strsave[MAX_STRING_LENGTH];
	char *word, letter;
	time_t last_logoff = 0, fake_logoff = 0, creation_time = 0;
	FAKE_DATA *fake_vict;
	DESCRIPTOR_DATA *d;
	one_argument (argument, to_finger);
	if (to_finger[0] == '\0')
	{
		send_to_char ("Syntax: finger <user>\n\r", ch);
		return;
	}
	for (fake_vict = fake_list; fake_vict != NULL; fake_vict = fake_vict->next)
	{
		if (is_name (to_finger, fake_vict->name))
		{
			sprintf (tcbuf,
				"Player: %s\n\rPlayer is currently online.\n\r\n\rPlan:\n\rNo plan.\n\r",
				fake_vict->name);
			send_to_char (tcbuf, ch);
			return;
		}
	}
	if ((victim = get_char_world (ch, to_finger)) != NULL
		&& can_see (ch, victim))
	{
		if (IS_NPC (victim))
		{
			sprintf (tcbuf, "Player '%s' not found.\n\r",
				capitalize (to_finger));
			send_to_char (tcbuf, ch);
			return;
		}
		strcpy (plan, victim->pcdata->plan);
		if (victim->desc == NULL)
			last_logoff = -2;

		else
			last_logoff = -1;
	}

	else
	{
		bool period = FALSE;
		int i = 0;
		sprintf (strsave, "%s%c/%s", PLAYER_DIR, toupper (to_finger[0]),
			capitalize (to_finger));
		for (i = 0; i<strlen(to_finger);i++)
			if (to_finger[i] == '.')
				period = TRUE;

		if ((ffp = fopen (strsave, "r")) == NULL || (period && !IS_IMMORTAL(ch)))
		{
			sprintf (tcbuf, "Player '%s' not found.\n\r",
				capitalize (to_finger));
			send_to_char (tcbuf, ch);
			return;
		}
		is_encrypted = FALSE;
		strcpy (referrer, "");
		while (1)
		{
			if (feof (ffp))
				break;
			letter = fread_letter (ffp);
			if (letter != 'P' && letter != 'L' && letter != 'H'
				&& letter != 'E' && letter != 'R' && letter != 'I')
			{
				if (feof (ffp))
				{
					send_to_char ("Error reading player information.\n\r", ch);
					fclose (ffp);
					return;
				}
				fread_to_eol (ffp);
				continue;
			}
			word = fread_word (ffp);
			if (!str_cmp (word, "lan"))
				strcpy (plan, fread_string (ffp));

			else if (!str_cmp (word, "mail"))
				strcpy (email, fread_string (ffp));
			else if (!str_cmp (word, "eferrer"))
				strcpy (referrer, fread_string (ffp));

			else if (!str_cmp (word, "ost"))
				strcpy (host, fread_string (ffp));

			else if (!str_cmp (word, "ogO"))
				last_logoff = fread_number (ffp);

			else if (!str_cmp (word, "ogF"))
				fake_logoff = fread_number (ffp);

			else if (!str_cmp (word, "d"))
				creation_time = fread_number (ffp);
		}
		fclose (ffp);
	}
	if (victim != NULL)
	{
		if (is_affected (victim, gsn_mask) && victim->mask != NULL)
		{
			plan[0] = '\0';
			sprintf (tcbuf, "Player: %s\n\r", victim->mask);
		}

		else
			sprintf (tcbuf, "Player: %s\n\r", capitalize (victim->name));
	}

	else
		sprintf (tcbuf, "Player: %s\n\r", capitalize (to_finger));
	if (creation_time > 0)
	{
		strcat (tcbuf, "Created on: ");
		strcat (tcbuf, ctime (&creation_time));
	}
	else if (victim && victim->id)
	{
		strcat (tcbuf, "Created on: ");
		strcat (tcbuf, ctime (&victim->id));
	}
	if (last_logoff == -2)
		strcat (tcbuf, "Player is currently link-dead.\n");

	else if (last_logoff == -1)
		strcat (tcbuf, "Player is currently online.\n");

	else
	{
		strcat (tcbuf, "Last On: ");
		victim = NULL;
		for (d = descriptor_list; d != NULL; d = d->next)
		{
			if (d == NULL || d->connected != CON_PLAYING)
				continue;
			original = d->original ? d->original : d->character;
			if (tolower (to_finger[0]) != tolower (original->name[0]))
				continue;
			if (!str_cmp (original->name, to_finger))
			{
				victim = original;
				break;
			}
		}
		if (fake_logoff > 0 && victim != NULL && !can_see (ch, victim))
			strcat (tcbuf, ctime (&fake_logoff));

		else if (last_logoff != 0)
			strcat (tcbuf, ctime (&last_logoff));

		else
			strcat (tcbuf, "Unknown\n");
	}
	if (plan[0] != '\0')
	{
		strcat (tcbuf, "\r\n\rPlan:\n\r");
		strcat (tcbuf, plan);
	}

	else
		strcat (tcbuf, "No plan.\n\r");
	if (ch->level == 100) /*&& (last_logoff != -1)*/
	{
		if (last_logoff == -1 && victim != NULL)
			strcpy(host,victim->desc->realhost);
		if (host[0] != '\0')
		{
			strcat (tcbuf, "\n\rLast host used: ");
			strcat (tcbuf, host);
		}

		else
			strcat (tcbuf, "No host recorded.\n\r");
		if (last_logoff == -1 && victim != NULL && victim->pcdata)
			strcpy(referrer,victim->pcdata->referrer);
		if (referrer[0] != '\0')
		{
			strcat (tcbuf, "\r\n\rReferred By: ");
			strcat (tcbuf, referrer);
			strcat (tcbuf, "");
		}
		else strcat(tcbuf, "\r\n\rNo referrer given.");

		if (last_logoff == -1 && victim != NULL && victim->pcdata)
			strcpy(email,victim->pcdata->email_addr);
		if (email[0] != '\0')
		{
			strcat (tcbuf, "\r\n\rEmail address: ");
			strcat (tcbuf, email);
			strcat (tcbuf, "\r\n\r");
		}

		else
			strcat (tcbuf, "\r\n\rNo email address recorded.\r\n\r");
	}
	send_to_char (tcbuf, ch);
	return;
}


/*
* Local functions.
*/
char *format_obj_to_char args ((OBJ_DATA * obj, CHAR_DATA * ch, bool fShort));
char *format_obj_to_char args ((OBJ_DATA * obj, CHAR_DATA * ch, bool fShort));
void show_list_to_char
args ((OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing));
void show_char_to_char_0 args ((CHAR_DATA * victim, CHAR_DATA * ch));
void show_char_to_char_1 args ((CHAR_DATA * victim, CHAR_DATA * ch));
void show_char_to_char args ((CHAR_DATA * list, CHAR_DATA * ch));
bool check_blind args ((CHAR_DATA * ch));
void show_game_stats args ((CHAR_DATA * ch, int type));
char *center_text (char *txtstr, int txtnum)
{
	int xsize, xcnt, osize, txtsize;
	static char retbuf[MAX_STRING_LENGTH];
	char tmpbuf[MAX_STRING_LENGTH], *cptr;
	osize = strlen (txtstr);
	txtsize = txtnum;
	for (cptr = txtstr; *cptr != '\0'; cptr++)
	{
		if (*cptr == '`')
		{
			txtsize += 2;
			cptr++;
		}
	}
	xsize = txtsize - osize;
	xsize /= 2;
	sprintf (tmpbuf, "%%%ds%%s", xsize);
	sprintf (retbuf, tmpbuf, "", txtstr);
	for (xcnt = (xsize + osize); xcnt < txtsize; xcnt++)
		strcat (retbuf, " ");
	return (retbuf);
}

char retbuf[16];
char *level_name (int level)
{
	if (level <= LEVEL_HERO)
		sprintf (retbuf, "%d", level);

	else
		switch (level)
	{
		case 91:
		case 92:
		case 93:
		case 94:
			strcpy (retbuf, "DEMIGOD");
			break;
		case 95:
		case 96:
		case 97:
			strcpy (retbuf, "GOD");
			break;
		case 98:
		case 99:
			strcpy (retbuf, "WIZARD");
			break;
		case 100:
			strcpy (retbuf, "OVERLORD");
			break;
	}
	return (retbuf);
}

bool is_pressed (OBJ_DATA * obj)
{
	OBJ_DATA *eObj;
	ELEVATOR_DEST_LIST *edest;
	bool found = FALSE;
	for (eObj = object_list; eObj != NULL; eObj = eObj->next)
	{
		if (eObj->item_type != ITEM_ELEVATOR)
			continue;
		if (eObj->pIndexData->vnum == obj->value[0])
		{
			found = TRUE;
			break;
		}
	}
	if (!found)
		return (FALSE);
	for (edest = eObj->dest_list; edest; edest = edest->next)
	{
		if (edest->vnum == obj->value[2])
			return (TRUE);
	}
	return (FALSE);
}

char *format_obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch, bool fShort)
{
	static char buf[MAX_STRING_LENGTH];
	buf[0] = '\0';
	if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0')))
		return buf;
	if (IS_SET(obj->extra_flags[1],ITEM_WINGED))
		strcat(buf, "`g(Winged)`` ");
	if (detect_obj_trap(ch,obj))
		strcat(buf, "`b(Trapped)`` ");
	if (IS_OBJ_STAT (obj, ITEM_INVIS))
		strcat (buf, "`E(Invis)`` ");
	if (IS_SET (obj->extra_flags[1], ITEM_WIZI))
		strcat (buf, "`h(Wizi)`` ");
	if (IS_AFFECTED (ch, AFF_DETECT_EVIL) && IS_OBJ_STAT (obj, ITEM_EVIL))
		strcat (buf, "`b(Red Aura)`` ");
	if (IS_AFFECTED (ch, AFF_DETECT_GOOD) && IS_OBJ_STAT (obj, ITEM_BLESS))
		strcat (buf, "`l(Blue Aura)`` ");
	if (IS_AFFECTED (ch, AFF_DETECT_MAGIC) && IS_OBJ_STAT (obj, ITEM_MAGIC))
		strcat (buf, "`j(Magical)`` ");
	if (IS_OBJ_STAT (obj, ITEM_GLOW))
		strcat (buf, "`k(Glowing)`` ");
	if (IS_OBJ_STAT (obj, ITEM_HUM))
		strcat (buf, "`n(Humming)`` ");
	if (IS_SET(obj->extra_flags[1],ITEM_PULSATING))
		strcat (buf, "`h(Pulsating)`` ");
	if (obj->item_type == ITEM_EBUTTON)
		if (is_pressed (obj))
		{
			if (IS_SET (obj->value[3], BUTTON_PULL))
				strcat (buf, "(Pulled) ");

			else
				strcat (buf, "(Pressed) ");
		}
		if (fShort)
		{
			if (obj->short_descr != NULL)
			{
				strcat (buf, "`d");
				strcat (buf, obj->short_descr);
				strcat (buf, "``");
			}
		}

		else
		{
			if (obj->description != NULL)
			{
				strcat (buf, "`d");
				if (obj->pIndexData->vnum == OBJ_VNUM_CLAN_SYMBOL)
				{
					sprintf (buf + strlen (buf), obj->description,
						get_clan_symbol (ch->in_room->area->clan));
				}

				else
				{
					strcat (buf, obj->description);
				}
				strcat (buf, "``");
			}
		}
		return buf;
}


/*
* Show a list to a character.
* Can coalesce duplicated items.
*/
void
show_list_to_char (OBJ_DATA * list, CHAR_DATA * ch, bool fShort,
				   bool fShowNothing)
{
	char buf[MAX_STRING_LENGTH];
	BUFFER *output;
	char **prgpstrShow;
	int *prgnShow;
	char *pstrShow;
	OBJ_DATA *obj;
	int nShow;
	int iShow;
	int count;
	bool fCombine;
	if (ch->desc == NULL)
		return;

	/*
	* Alloc space for output lines.
	*/
	output = new_buf ();
	count = 0;
	for (obj = list; obj != NULL && obj->next_content != obj;
		obj = obj->next_content)
		count++;
	prgpstrShow = alloc_mem (count * sizeof (char *));
	prgnShow = alloc_mem (count * sizeof (int));
//	prgpstrShow = reinterpret_cast<char**>(alloc_mem (count * sizeof (char *)));
//	prgnShow = reinterpret_cast<int *>(alloc_mem (count * sizeof (int)));
	nShow = 0;

	/*
	* Format the list of objects.
	*/
	for (obj = list; obj != NULL && obj->next_content != obj;
		obj = obj->next_content)
	{
		if ((obj->item_type == ITEM_FURNITURE || obj->item_type == ITEM_RAFT) && obj->pIndexData->vnum != OBJ_VNUM_HEXAGRAM)
			if (obj->value[0] > 0 && count_users (obj) >= obj->value[0])
				continue;
		if (obj->wear_loc == WEAR_NONE && can_see_obj (ch, obj)
			&& obj->description[0] != '\0')
		{
			pstrShow = format_obj_to_char (obj, ch, fShort);
			fCombine = FALSE;
			if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
			{

				/*
				* Look for duplicates, case sensitive.
				* Matches tend to be near end so run loop backwords.
				*/
				for (iShow = nShow - 1; iShow >= 0; iShow--)
				{
					if (!strcmp (prgpstrShow[iShow], pstrShow))
					{
						prgnShow[iShow]++;
						fCombine = TRUE;
						break;
					}
				}
			}

			/*
			* Couldn't combine, or didn't want to.
			*/
			if (!fCombine)
			{
				prgpstrShow[nShow] = str_dup (pstrShow);
				prgnShow[nShow] = 1;
				nShow++;
			}
		}
	}

	/*
	* Output the formatted list.
	*/
	for (iShow = 0; iShow < nShow; iShow++)
	{
		if (prgpstrShow[iShow][0] == '\0')
		{
			free_string (prgpstrShow[iShow]);
			continue;
		}
		if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
		{
			if (prgnShow[iShow] != 1)
			{
				sprintf (buf, "(%2d) ", prgnShow[iShow]);
				add_buf (output, buf);
			}

			else
			{
				add_buf (output, "     ");
			}
		}
		add_buf (output, prgpstrShow[iShow]);
		add_buf (output, "\n\r");
		free_string (prgpstrShow[iShow]);
	}
	if (fShowNothing && nShow == 0)
	{
		if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
			send_to_char ("     ", ch);
		send_to_char ("Nothing.\n\r", ch);
	}
	page_to_char (buf_string (output), ch);

	/*
	* Clean up.
	*/
	free_buf (output);
	free_mem (prgpstrShow, count * sizeof (char *));
	free_mem (prgnShow, count * sizeof (int));
	return;
}

void
show_list_to_char_but (OBJ_DATA * list, CHAR_DATA * ch, bool fShort,
					   bool fShowNothing, OBJ_DATA * not_show)
{
	char buf[MAX_STRING_LENGTH];
	BUFFER *output;
	char **prgpstrShow;
	int *prgnShow;
	char *pstrShow;
	OBJ_DATA *obj;
	int nShow;
	int iShow;
	int count;
	bool fCombine;
	if (ch->desc == NULL)
		return;

	/*
	* Alloc space for output lines.
	*/
	output = new_buf ();
	count = 0;
	for (obj = list; obj != NULL; obj = obj->next_content)
		count++;
	prgpstrShow = alloc_mem (count * sizeof (char *));
	prgnShow = alloc_mem (count * sizeof (int));
//	prgpstrShow = reinterpret_cast<char **>(alloc_mem(count * sizeof(char *)));
//	prgnShow = reinterpret_cast<int *>(alloc_mem(count * sizeof(int)));
	nShow = 0;

	/*
	* Format the list of objects.
	*/
	for (obj = list; obj != NULL; obj = obj->next_content)
	{
		if (obj == not_show)
			continue;
		if ((obj->item_type == ITEM_FURNITURE || obj->item_type == ITEM_RAFT) && obj->pIndexData->vnum != OBJ_VNUM_HEXAGRAM)
			if (obj->value[0] > 0 && count_users (obj) >= obj->value[0])
				continue;
		if (obj->wear_loc == WEAR_NONE && can_see_obj (ch, obj))
		{
			pstrShow = format_obj_to_char (obj, ch, fShort);
			fCombine = FALSE;
			if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
			{

				/*
				* Look for duplicates, case sensitive.
				* Matches tend to be near end so run loop backwords.
				*/
				for (iShow = nShow - 1; iShow >= 0; iShow--)
				{
					if (!strcmp (prgpstrShow[iShow], pstrShow))
					{
						prgnShow[iShow]++;
						fCombine = TRUE;
						break;
					}
				}
			}

			/*
			* Couldn't combine, or didn't want to.
			*/
			if (!fCombine)
			{
				prgpstrShow[nShow] = str_dup (pstrShow);
				prgnShow[nShow] = 1;
				nShow++;
			}
		}
	}

	/*
	* Output the formatted list.
	*/
	for (iShow = 0; iShow < nShow; iShow++)
	{
		if (prgpstrShow[iShow][0] == '\0')
		{
			free_string (prgpstrShow[iShow]);
			continue;
		}
		if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
		{
			if (prgnShow[iShow] != 1)
			{
				sprintf (buf, "(%2d) ", prgnShow[iShow]);
				add_buf (output, buf);
			}

			else
			{
				add_buf (output, "     ");
			}
		}
		add_buf (output, prgpstrShow[iShow]);
		add_buf (output, "\n\r");
		free_string (prgpstrShow[iShow]);
	}
	if (fShowNothing && nShow == 0)
	{
		if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
			send_to_char ("     ", ch);
		send_to_char ("Nothing.\n\r", ch);
	}
	page_to_char (buf_string (output), ch);

	/*
	* Clean up.
	*/
	free_buf (output);
	free_mem (prgpstrShow, count * sizeof (char *));
	free_mem (prgnShow, count * sizeof (int));
	return;
}

void scanning (CHAR_DATA * ch, int door)
{
	char buf[MAX_STRING_LENGTH];
	int distance, visibility;
	bool found;
	ROOM_INDEX_DATA *was_in_room;
	visibility = 6;
	if (!IS_SET (ch->act, PLR_HOLYLIGHT))
	{
		switch (weather_info.sunlight)
		{
		case SUN_SET:
			visibility = 4;
			break;
		case SUN_DARK:
			visibility = 2;
			break;
		case SUN_RISE:
			visibility = 4;
			break;
		case SUN_LIGHT:
			visibility = 6;
			break;
		}
		switch (weather_info.sky)
		{
		case SKY_CLOUDLESS:
			break;
		case SKY_CLOUDY:
			visibility -= 1;
			break;
		case SKY_RAINING:
			visibility -= 2;
			break;
		case SKY_LIGHTNING:
			visibility -= 3;
			break;
		}
	}
	was_in_room = ch->in_room;
	found = FALSE;
	for (distance = 1; distance <= 6; distance++)
	{
		EXIT_DATA *pexit;
		CHAR_DATA *list;
		CHAR_DATA *rch;
		if ((pexit = ch->in_room->exit[door]) != NULL
			&& pexit->u1.to_room != NULL && pexit->u1.to_room != was_in_room)
		{

			/* If the door is closed, stop looking... */
			if (IS_SET (pexit->exit_info, EX_CLOSED))
			{
				char door_name[80];
				one_argument (pexit->keyword, door_name);
				if (door_name[0] == '\0')
					strcat (door_name, "door");
				sprintf (buf, "A closed %s %s %s.\n\r", door_name,
					dist_name[distance - 1], dir_desc[door]);
				send_to_char (buf, ch);
				found = TRUE;
				break;
			}
			ch->in_room = pexit->u1.to_room;
			if (IS_OUTSIDE (ch) ? distance > visibility : distance > 4)
				break;
			list = ch->in_room->people;
			for (rch = list; rch != NULL; rch = rch->next_in_room)
				if (can_see (ch, rch))
				{
					found = TRUE;
					sprintf (buf, "%s%s is %s %s.\n\r",
						(is_affected (rch, gsn_mask)
						&& rch->mask != NULL) ? rch->mask : PERS (rch,
						ch),
						(IS_NPC (rch) && !IS_SET(rch->act2,ACT_ILLUSION)) ? "" : " (PLAYER)",
						dist_name[distance], dir_desc[door]);
					buf[0] = UPPER (buf[0]);
					send_to_char (buf, ch);
				}
				if (detect_room_trap(ch,ch->in_room))
				{
					sprintf(buf,"`bA trap is %s %s.``\n\r",dist_name[distance],dir_desc[door]);
					send_to_char(buf,ch);
				}

		}
	}
	ch->in_room = was_in_room;
	if (!found)
	{
		sprintf (buf, "You can't see anything %s.\n\r", dir_desc[door]);
		send_to_char (buf, ch);
	}
	return;
}

void do_scanning (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int dir;
	bool found;
	if (!check_blind (ch))
		return;
	if (is_affected (ch, gsn_disorientation))
	{
		send_to_char ("You are too disoriented to scan.\n\r", ch);
		return;
	}
	if (is_affected(ch,skill_lookup("nearsight")))
	{
		send_to_char ("You can't make out anything outside this room.\n\r",ch);
		return;
	}
	if (IS_AFFECTED(ch,AFF_BLIND))
	{
		send_to_char ("You are blind.\n\r",ch);
		return;
	}

	if (argument[0] == '\0')
	{
		act ("$n scans intensely all around $m.", ch, NULL, NULL, TO_ROOM);
		found = FALSE;
		for (dir = 0; dir <= 5; dir++)
			if (ch->in_room->exit[dir] != NULL)
			{
				sprintf (buf, "\n\r`n[`o%s`n]``\n\r", dir_desc[dir]);
				send_to_char (buf, ch);
				scanning (ch, dir);
				found = TRUE;
			}
			if (!found)
				send_to_char ("There are no exits here.\n\r", ch);
	}

	else
	{
		if (!str_prefix (argument, "north"))
			dir = DIR_NORTH;

		else if (!str_prefix (argument, "east"))
			dir = DIR_EAST;

		else if (!str_prefix (argument, "south"))
			dir = DIR_SOUTH;

		else if (!str_prefix (argument, "west"))
			dir = DIR_WEST;

		else if (!str_prefix (argument, "up"))
			dir = DIR_UP;

		else if (!str_prefix (argument, "down"))
			dir = DIR_DOWN;

		else
		{
			send_to_char ("That's not a direction!\n\r", ch);
			return;
		}
		act ("$n scans the area $T of $m.", ch, NULL, dir_desc[dir], TO_ROOM);
		if (ch->in_room->exit[dir] == NULL)
		{
			send_to_char
				("There does not seem to be an exit in that direction.\n\r", ch);
			return;
		}
		sprintf (buf, "\n\r`n[`o%s`n]``\n\r", dir_desc[dir]);
		send_to_char (buf, ch);
		scanning (ch, dir);
	}
	return;
}

void show_char_to_char_0 (CHAR_DATA * victim, CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH], message[MAX_STRING_LENGTH];
	char wizi[MAX_STRING_LENGTH];
	char incog[MAX_STRING_LENGTH];
	buf[0] = '\0';
	if (victim->invis_level > 1)
		sprintf (wizi, "`l(Wizi %d)`` ", victim->invis_level);

	else
		sprintf (wizi, "``");
	if (victim->incog_level > 1)
		sprintf (incog, "`l(Incog %d)`` ", victim->incog_level);

	else
		sprintf (incog, "``");
	if (victim->riders != NULL)
		return;
	if (IS_NPC (victim) && victim->long_descr[0] == '\0')
		return;
	if (IS_SET (victim->comm, COMM_AFK))
		strcat (buf, "[AFK] ");
	if (IS_SET (victim->comm, COMM_QUIET))
		strcat (buf, "[QUIET] ");
	if (!IS_NPC (victim) && victim->desc == NULL)
	{
		if (IS_SET(victim->act2,ACT_PUPPETEER))
			strcat(buf,"`d(Trance)`` ");
		else strcat (buf, "`h(Linkdead)`` ");
	}
	if (victim->desc != NULL && victim->desc->editor != 0)
		if (victim->desc->editor == ED_AREA
			|| victim->desc->editor == ED_ROOM
			|| victim->desc->editor == ED_OBJECT
			|| victim->desc->editor == ED_MOBILE
			|| victim->desc->editor == BUILD_MAIN
			|| victim->desc->editor == BUILD_NAME
			|| victim->desc->editor == BUILD_FLAGS
			|| victim->desc->editor == BUILD_KEYWORDS
			|| victim->desc->editor == BUILD_KEYWORDS_ADD
			|| victim->desc->editor == BUILD_DOORS
			|| victim->desc->editor == BUILD_DRESETS
			|| victim->desc->editor == BUILD_DNAME)
			strcat (buf, "[BUILDING] ");
	if (is_affected (victim, skill_lookup ("web")))
		strcat (buf, "(Webbed) ");
	if (is_affected (victim, gsn_entangle))
		strcat (buf, "(Entangled) ");
	if (is_affected (victim, skill_lookup("ghost form")))
		strcat (buf, "`h(Blurred)`` ");
	if (is_affected (victim, gsn_clasp))			  
		strcat (buf, "`a(Clasped)`` ");
	if (is_affected (victim, gsn_torture))
		strcat (buf, "`a(Chained)`` ");
	if (victim->stunned)
		strcat (buf, "`b(Stunned)`` ");
	if (is_affected (victim, skill_lookup("eldritch miasma")))
		strcat (buf, "`j(Green fog)`` ");
	if (is_affected (victim, gsn_mirror) &&
		(is_affected (ch, skill_lookup ("detect magic")) || IS_IMMORTAL (ch)))
		strcat (buf, "`h(Mirror)`` ");
	if (IS_AFFECTED (victim, AFF_INVISIBLE))
		strcat (buf, "`E(Invis)`` ");
	if (victim->invis_level >= 2)
		strcat (buf, wizi);
	if (victim->incog_level >= 2)
		strcat (buf, incog);
	if (IS_AFFECTED (victim, AFF_HIDE))
		strcat (buf, "`G(Hide)`` ");
	if (IS_AFFECTED (victim, AFF_CAMOUFLAGE))
		strcat (buf, "`c(Camouflaged)`` ");
	if (IS_AFFECTED (victim, AFF_CHARM))
	{
		if (!(IS_SET(victim->act2,ACT_FAMILIAR)))
			strcat (buf, "`H(Charmed)`` ");
		else strcat (buf, "`h(Controlled)`` ");
	}
	if (is_affected (victim, gsn_locust_wings))
		strcat (buf, "`g(Winged)`` ");
	if (IS_AFFECTED (victim, AFF_PASS_DOOR))
		strcat (buf, "`g(Translucent)`` ");
	if (IS_AFFECTED (victim, AFF_FAERIE_FIRE))
		strcat (buf, "`i(Pink Aura)`` ");
	if (IS_EVIL (victim) && IS_AFFECTED (ch, AFF_DETECT_EVIL))
		strcat (buf, "`b(Red Aura)`` ");
	if (IS_GOOD (victim) && IS_AFFECTED (ch, AFF_DETECT_GOOD))
		strcat (buf, "`k(Golden Aura)`` ");
	if (IS_AFFECTED (victim, AFF_SANCTUARY))
		strcat (buf, "`o(White Aura)`` ");
	if (!IS_NPC (victim) && IS_SET (victim->act, PLR_WANTED))
		strcat (buf, "`b[Wanted]`` ");
	if (!IS_NPC (victim) && IS_SET (victim->act, PLR_JAILED))
		strcat (buf, "`i[Jailed]`` ");
	if (victim->position == victim->start_pos && victim->long_descr[0] != '\0' 
		&& ch->race != PC_RACE_SWARM)
	{
		if ((IS_NPC (victim) && !IS_SET(victim->act2, ACT_ILLUSION)) 
			|| IS_SET(victim->act2,ACT_TRANSFORMER))
			strcat (buf, "`f");

		else
			strcat (buf, "`g");
		if (IS_SET(victim->act2, ACT_ILLUSION) && IS_SET (ch->comm, COMM_BRIEF))
		{
			strcat (buf, victim->short_descr);
			strcat (buf, " ``is here.\n\r");
		}
		else strcat (buf, victim->long_descr);
		send_to_char (buf, ch);
		return;
	}
	if (IS_NPC (victim) || IS_SET(victim->act2,ACT_TRANSFORMER))
		strcat (buf, "`f");

	else
		strcat (buf, "`g");
	if (ch->race != PC_RACE_SWARM)
	{
		if (!IS_NPC (victim) && !victim->pcdata->flaming && !IS_SET(victim->act2,ACT_TRANSFORMER))
		{
			if (victim->pcdata->pretitle[0] != '\0')
			{
				strcat (buf, victim->pcdata->pretitle);
				strcat (buf, " ");
			}
		}
		if (IS_NPC(victim) || (!victim->pcdata->flaming && !IS_SET(victim->act2,ACT_TRANSFORMER)))
		{
			if (is_affected (victim, gsn_mask) && victim->mask != NULL)
				strcat (buf, victim->mask);
			else  strcat (buf, PERS (victim, ch));
		}
		if (!IS_NPC (victim) && !IS_SET (ch->comm, COMM_BRIEF) && victim->position == POS_STANDING
			&& !victim->pcdata->flaming && !IS_SET(victim->act2,ACT_TRANSFORMER))	/*  && ch->on == NULL) */
			strcat (buf, victim->pcdata->title);
		if (victim->position == POS_STANDING && victim->fighting != NULL)
			victim->position = POS_FIGHTING;
		if (!IS_NPC(victim) && victim->pcdata->flaming)
			strcat (buf, "`kA flaming apparition");
		if (!IS_NPC(victim) && IS_SET(victim->act2,ACT_TRANSFORMER))
		{
			strcat (buf, victim->long_descr);
			if (victim->position == POS_STANDING)
			{  
				send_to_char (buf, ch);
				return;
			}
		}
	}
	//Iblis 9/17/04 - Swarm only see this orace_lookup
	else 
	{
		sprintf (message, "`gA %s``", race_table[victim->race].name);
		strcat (buf,message);
	} 
	switch (victim->position)
	{
	case POS_DEAD:
		strcat (buf, " is DEAD!!");
		break;
	case POS_MORTAL:
		strcat (buf, " is mortally wounded.");
		break;
	case POS_INCAP:
		strcat (buf, " is incapacitated.");
		break;
	case POS_STUNNED:
		strcat (buf, " is lying here stunned.");
		break;
	case POS_COMA:
	case POS_SLEEPING:
		if (victim->on != NULL)
		{
			if (IS_SET (victim->on->value[2], SLEEP_AT))
			{
				sprintf (message, " is sleeping at %s.",
					victim->on->short_descr);
				strcat (buf, message);
			}

			else if (IS_SET (victim->on->value[2], SLEEP_ON))
			{
				sprintf (message, " is sleeping on %s.",
					victim->on->short_descr);
				strcat (buf, message);
			}

			else
			{
				sprintf (message, " is sleeping in %s.",
					victim->on->short_descr);
				strcat (buf, message);
			}
		}

		else
			strcat (buf, " is sleeping here.");
		break;
	case POS_RESTING:
		if (victim->on != NULL)
		{
			if (IS_SET (victim->on->value[2], REST_AT))
			{
				sprintf (message, " is resting at %s.",
					victim->on->short_descr);
				strcat (buf, message);
			}

			else if (IS_SET (victim->on->value[2], REST_ON))
			{
				sprintf (message, " is resting on %s.",
					victim->on->short_descr);
				strcat (buf, message);
			}

			else
			{
				sprintf (message, " is resting in %s.",
					victim->on->short_descr);
				strcat (buf, message);
			}
		}

		else
			strcat (buf, " is resting here.");
		break;
	case POS_TETHERED:
		strcat (buf, " is tethered here.");
		break;
	case POS_CHOPPING:
		strcat (buf, " is here, chopping away like a good little lumberjack.");
		break;
	case POS_MILLING:
		strcat (buf, " is here, milling away like a good little miller.");
		break;
	case POS_SITTING:
		if (victim->on != NULL)
		{
			if (IS_SET (victim->on->value[2], SIT_AT))
			{
				sprintf (message, " is sitting at %s.",
					victim->on->short_descr);
				strcat (buf, message);
			}

			else if (IS_SET (victim->on->value[2], SIT_ON))
			{
				sprintf (message, " is sitting on %s.",
					victim->on->short_descr);
				strcat (buf, message);
			}

			else
			{
				sprintf (message, " is sitting in %s.",
					victim->on->short_descr);
				strcat (buf, message);
			}
		}

		else
			strcat (buf, " is sitting here.");
		break;
	case POS_STANDING:
		if (victim->on != NULL)
		{
			if (IS_SET (victim->on->value[2], STAND_AT))
			{
				sprintf (message, " is standing at %s.",
					victim->on->short_descr);
				strcat (buf, message);
			}

			else if (IS_SET (victim->on->value[2], STAND_ON))
			{
				sprintf (message, " is standing on %s.",
					victim->on->short_descr);
				strcat (buf, message);
			}

			else
			{
				sprintf (message, " is standing in %s.",
					victim->on->short_descr);
				strcat (buf, message);
			}
		}

		else
			strcat (buf, " ``is here.");
		break;
	case POS_MOUNTED:
		if (victim->mount != NULL)
		{
			sprintf (message, " ``is riding on %s.",
				victim->mount->short_descr);
			strcat (buf, message);
		}
		break;
	case POS_FIGHTING:
		strcat (buf, " is here, fighting ");
		if (victim->fighting == NULL)
			strcat (buf, "thin air??");

		else if (victim->fighting == ch)
			strcat (buf, "YOU!");

		else if (victim->in_room == victim->fighting->in_room)
		{
			strcat (buf, PERS (victim->fighting, ch));
			strcat (buf, ".");
		}

		else
			strcat (buf, "someone who left??");
		break;
	case POS_FEIGNING_DEATH:
		sprintf (buf, "     `dThe corpse of %s is lying here.",victim->name);
		break;
	}
	strcat (buf, "``\n\r");
	buf[0] = UPPER (buf[0]);
	send_to_char (buf, ch);
	return;
}

void show_char_to_char_1 (CHAR_DATA * victim, CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	int iWear;
	int i=0;
	int percent,amount=0,k=0;
	long wear_flags_left=0;
	bool found;
	char *last_statement = NULL;
	char wbuf[MAX_STRING_LENGTH];
	if (is_dueling (ch) && is_dueling (victim))
	{
		switch (victim->sex)
		{
		case SEX_NEUTRAL:
			sprintf (buf, "%s looks piercingly at %s, sizing it up.\n\r",
				ch->name, victim->name);
			break;
		case SEX_MALE:
			sprintf (buf, "%s looks piercingly at %s, sizing him up.\n\r",
				ch->name, victim->name);
			break;
		case SEX_FEMALE:
			sprintf (buf, "%s looks piercingly at %s, sizing her up.\n\r",
				ch->name, victim->name);
			break;
		}
		arena_report (buf);
	}
	if (can_see (victim, ch))
	{
		if (ch == victim)
			act ("$n looks at $mself.", ch, NULL, NULL, TO_ROOM);

		else if (!IS_IMMORTAL(ch))
		{
			switch (number_range (0, 5))
			{
			case 0:
				act ("$n regards you pensively.", ch, NULL, victim, TO_VICT);
				act ("$n regards $N pensively.", ch, NULL, victim, TO_NOTVICT);
				break;
			case 1:
				act ("$n assesses you.", ch, NULL, victim, TO_VICT);
				act ("$n assesses $N.", ch, NULL, victim, TO_NOTVICT);
				break;
			case 2:
				act ("$n examines you closely.", ch, NULL, victim, TO_VICT);
				act ("$n examines $N closely.", ch, NULL, victim, TO_NOTVICT);
				break;
			case 3:
				act ("$n sizes you up.", ch, NULL, victim, TO_VICT);
				act ("$n sizes up $N.", ch, NULL, victim, TO_NOTVICT);
				break;
			case 4:
				act ("$n looks at you appraisingly.", ch, NULL, victim, TO_VICT);
				act ("$n looks at $N appraisingly.", ch, NULL, victim,
					TO_NOTVICT);
				break;
			case 5:
				act ("$n ruminates your existence.", ch, NULL, victim, TO_VICT);
				act ("$n ruminates $N's existence.", ch, NULL, victim,
					TO_NOTVICT);
				break;
			}
		}
	}
	if (!IS_NPC(victim) && victim->pcdata->flaming && !IS_IMMORTAL(ch) && ch->race != PC_RACE_LITAN)
	{
		AFFECT_DATA af;
		send_to_char ("The flame from the flaming apparition burns your eyes!\n\r",ch);
		act ("$n's eyes are burnt by the flaming apparition!",ch,NULL,victim,TO_ROOM);
		af.where = TO_AFFECTS;
		af.type = skill_lookup ("fire breath");
		af.level = victim->level;
		af.duration = 0;
		af.location = APPLY_HITROLL;
		af.modifier = -4;
		af.bitvector = AFF_BLIND;
		af.permaff = FALSE;
		af.composition = FALSE;
		af.comp_name = str_dup ("");
		affect_to_char (ch, &af);
		return;
	}
	if (victim->description[0] != '\0')
	{
		send_to_char (victim->description, ch);
		send_to_char ("``", ch);
	}

	else
	{
		act ("You see nothing out of the ordinary with $M.", ch, NULL,
			victim, TO_CHAR);
	}
	if (victim->max_hit > 0)
		percent = (100 * victim->hit) / victim->max_hit;

	else
		percent = -1;
	if (is_affected (victim, gsn_mask) && victim->mask != NULL)
		strcpy (buf, victim->mask);

	else
		strcpy (buf, PERS (victim, ch));
	if (percent >= 100)
		strcat (buf, " is in excellent condition.\n\r");

	else if (percent >= 90)
		strcat (buf, " has a few scratches.\n\r");

	else if (percent >= 75)
		strcat (buf, " has some small wounds and bruises.\n\r");

	else if (percent >= 50)
		strcat (buf, " has quite a few wounds.\n\r");

	else if (percent >= 30)
		strcat (buf, " has some big nasty wounds and scratches.\n\r");

	else if (percent >= 15)
		strcat (buf, " has suffered grievous wounds which bleed profusely.\n\r");

	else if (percent >= 0)
		strcat (buf, " is barely clinging to life.\n\r");

	else
		strcat (buf, " is knocking on death's door.\n\r");
	buf[0] = UPPER (buf[0]);
	send_to_char (buf, ch);
	found = FALSE;

	for (i = 1; wear_conversion_table[i].wear_loc != -2;++i)
		wear_flags_left |= wear_conversion_table[i].wear_flag;

	for (i = 1; wear_conversion_table[i].wear_loc != -2; ++i)
	{
		iWear = wear_conversion_table[i].wear_flag;
		if (iWear == ITEM_TAKE || iWear == ITEM_NO_SAC)
			continue;
		if (!IS_SET(wear_flags_left,iWear))
			continue;
		if ((obj = get_eq_char_new (victim, iWear)) != NULL
			&& can_see_obj (ch, obj))
		{

			if (!found)
			{
				send_to_char ("\n\r", ch);
				act ("$N is using:", ch, NULL, victim, TO_CHAR);
				found = TRUE;
			}
			if (victim->race == PC_RACE_SWARM && iWear == ITEM_WEAR_BODY)
				send_to_char("<radiates>          ",ch);

			wear_flags_left -= iWear;
			if (wear_conversion_table[i].wear_flag == ITEM_WEAR_WRIST
				&& IS_SET(obj->wear_loc,ITEM_WEAR_ANOTHER_WRIST))
			{
				sprintf(buf,"<%s","worn around both wrists");
				wear_flags_left -= ITEM_WEAR_ANOTHER_WRIST;
			}
			else if (wear_conversion_table[i].wear_flag == ITEM_WEAR_FINGER
				&& IS_SET(obj->wear_loc,ITEM_WEAR_ANOTHER_FINGER))
			{
				sprintf(buf,"<%s","worn on both fingers");
				wear_flags_left -= ITEM_WEAR_ANOTHER_FINGER;
			}
			else if (wear_conversion_table[i].wear_flag == ITEM_WEAR_NECK
				&& IS_SET(obj->wear_loc,ITEM_WEAR_ANOTHER_NECK))
			{
				sprintf(buf,"<%s","worn around entire neck");
				wear_flags_left -= ITEM_WEAR_ANOTHER_NECK;
			}
			else if (wear_conversion_table[i].wear_flag == ITEM_WIELD
				&&  IS_SET(obj->wear_loc,ITEM_TWO_HANDS))
			{
				sprintf(buf,"<%s","wielded in both hands");
				wear_flags_left -= ITEM_TWO_HANDS;
			}
			else sprintf(buf,"<%s",wear_conversion_table[i].where_name);
			amount = 0;
			last_statement = NULL;
			for (k = i+1; wear_conversion_table[k].wear_loc != -2;++k)
			{
				//should never hit this
				if (!IS_SET(wear_flags_left,wear_conversion_table[k].wear_flag))
					continue;
				if (!IS_SET(obj->wear_loc,wear_conversion_table[k].wear_flag))
					continue;
				++amount;
				if (amount < 2)
				{
				}
				else
				{
					if (amount > 1)
						strcat(buf,", ");
					strcat(buf,last_statement);
				}
				wear_flags_left -= wear_conversion_table[k].wear_flag;
				if (wear_conversion_table[k].wear_flag == ITEM_WEAR_WRIST
					&& IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_WRIST))
				{
					last_statement = "both wrists";
					wear_flags_left -= ITEM_WEAR_ANOTHER_WRIST;
				}
				else if (wear_conversion_table[k].wear_flag == ITEM_WEAR_FINGER
					&& IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_FINGER))
				{
					last_statement = "both fingers";
					wear_flags_left -= ITEM_WEAR_ANOTHER_FINGER;
				}
				else if (wear_conversion_table[k].wear_flag == ITEM_WEAR_NECK
					&& IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_NECK))
				{
					last_statement = "entire neck";
					wear_flags_left -= ITEM_WEAR_ANOTHER_NECK;
				}
				else if (wear_conversion_table[k].wear_flag == ITEM_WIELD
					&& IS_SET(obj->wear_loc,ITEM_TWO_HANDS))
				{
					last_statement = "as a weapon";
					wear_flags_left -= ITEM_TWO_HANDS;
				}
				else last_statement = wear_conversion_table[k].wear_name;
			}
			if (amount > 1)
				strcat(buf,",");
			if (amount > 0)
				strcat(buf," and ");
			if (last_statement)
				strcat(buf,last_statement);
			strcat(buf,">");
			sprintf(wbuf,"%-30s ",buf);
			send_to_char(wbuf,ch);

			//	  send_to_char (where_name[wear_conversion_table[i].wear_loc], ch);
			send_to_char (format_obj_to_char (obj, ch, TRUE), ch);
			send_to_char ("\n\r", ch);
		}
	}
	if (victim != ch
		&& !IS_NPC (ch) && number_percent () < get_skill (ch, gsn_peek))
	{

		send_to_char ("\n\rYou peek at the inventory:\n\r", ch);
		check_improve (ch, gsn_peek, TRUE, 4);
		show_list_to_char (victim->carrying, ch, TRUE, TRUE);
	}
	return;
}

void do_peek (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	if (argument[0] == '\0')
		send_to_char ("Syntax: Peek CharName.\n\r", ch);
	if (get_skill (ch, gsn_peek) <= 0)
		send_to_char
		("The most you know about peek is peek-a-boo, and that won't help you now!\n\r",
		ch);
	victim = get_char_room (ch, argument);
	if (victim == NULL)
	{
		send_to_char ("That person is not here.\n\r", ch);
		return;
	}
	if ((number_percent () < get_skill (ch, gsn_peek) * (2.0/3.0)) && !IS_IMMORTAL(ch))
	{
		act ("$n takes a peek at $N.", ch, NULL, victim, TO_NOTVICT);
		act ("$n takes a peek at you.", ch, NULL, victim, TO_VICT);
	}
	if (victim != ch && !IS_NPC (ch)
		&& number_percent () < get_skill (ch, gsn_peek))
	{
		act ("You peek at $N's inventory:", ch, NULL, victim, TO_CHAR);
		check_improve (ch, gsn_peek, TRUE, 4);

		if (victim->silver + victim->gold * 100 < 1000)
			act ("$N's coin purse looks fairly light.\n\r", ch, NULL,
			victim, TO_CHAR);

		else if (victim->silver + victim->gold * 100 < 10000)
			act ("$N's coin purse looks about half full.\n\r", ch, NULL,
			victim, TO_CHAR);

		else if (victim->silver + victim->gold * 100 < 100000)
			act ("$N's coin purse looks full.\n\r", ch, NULL, victim, TO_CHAR);

		else if (victim->silver + victim->gold * 100 < 1000000)
			act ("$N's coin purse is overflowing with coins!\n\r", ch,
			NULL, victim, TO_CHAR);

		else
			act
			("$N's coin purse is so full you're amazed $N can even lift it!\n\r",
			ch, NULL, victim, TO_CHAR);
		show_list_to_char (victim->carrying, ch, TRUE, TRUE);
	}
}
void show_char_to_char_2 (CHAR_DATA * victim, CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH];
	int percent;
	if (can_see (victim, ch)
		|| (!IS_NPC (ch) && IS_SET (ch->act, PLR_HOLYLIGHT)))
	{
		if (ch == victim)
			act ("$n glances at $mself.", ch, NULL, NULL, TO_ROOM);

		else
		{
			act ("$n glances in your direction.", ch, NULL, victim, TO_VICT);
			act ("$n takes a quick glance at $N.", ch, NULL, victim,
				TO_NOTVICT);
		}
		if (victim->max_hit > 0)
			percent = (100 * victim->hit) / victim->max_hit;

		else
			percent = -1;
		strcpy (buf, PERS (victim, ch));
		if (percent >= 100)
			strcat (buf, " is in excellent condition.\n\r");

		else if (percent >= 90)
			strcat (buf, " has a few scratches.\n\r");

		else if (percent >= 75)
			strcat (buf, " has some small wounds and bruises.\n\r");

		else if (percent >= 50)
			strcat (buf, " has quite a few wounds.\n\r");

		else if (percent >= 30)
			strcat (buf, " has some big nasty wounds and scratches.\n\r");

		else if (percent >= 15)
			strcat (buf, " looks pretty hurt.\n\r");

		else if (percent >= 0)
			strcat (buf, " is in awful condition.\n\r");

		else
			strcat (buf, " is bleeding to death.\n\r");
		buf[0] = UPPER (buf[0]);
		send_to_char (buf, ch);
		return;
	}
	send_to_char ("You do not see that here.\n\r", ch);
	return;
}

void show_char_to_char (CHAR_DATA * list, CHAR_DATA * ch)
{
	CHAR_DATA *rch;
	for (rch = list; rch != NULL; rch = rch->next_in_room)
	{
		if (rch == ch)
			continue;
		if (get_trust (ch) < rch->invis_level)
			continue;
		if (IS_NPC (rch))
			send_to_char ("`f", ch);

		else
			send_to_char ("`g", ch);
		if (can_see (ch, rch))
		{
			if (is_dueling (ch) && is_dueling (rch))
			{
				char buf[MAX_STRING_LENGTH];
				sprintf (buf, "%s prepares as %s approaches.\n\r",
					ch->name, rch->name);
				arena_report (buf);
			}
			show_char_to_char_0 (rch, ch);
		}

		else
		{
			if (is_dueling (ch) && is_dueling (rch))
			{
				char buf[MAX_STRING_LENGTH];
				sprintf (buf, "%s evades %s's notice.\n\r", rch->name,
					ch->name);
				arena_report (buf);
			}
			if (room_is_dark (ch->in_room) && ch->race != PC_RACE_KALIAN && (ch->race != PC_RACE_LICH) && IS_AFFECTED (rch, AFF_INFRARED))
			{
				send_to_char
					("You see nothing except the glowing red eyes watching you in the inky darkness.\n\r",
					ch);
			}
		}
	}
	return;
}

bool check_blind (CHAR_DATA * ch)
{
	if (!IS_NPC (ch) && IS_SET (ch->act, PLR_HOLYLIGHT))
		return TRUE;
	if (IS_AFFECTED (ch, AFF_BLIND))
	{
		send_to_char ("You can't see a thing!\n\r", ch);
		return FALSE;
	}
	return TRUE;
}


/* changes your scroll */
void do_scroll (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[100];
	int lines;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		if (ch->lines == 0)
			send_to_char ("You do not page long messages.\n\r", ch);

		else
		{
			sprintf (buf, "You currently display %d lines per page.\n\r",
				ch->lines + 2);
			send_to_char (buf, ch);
		}
		return;
	}
	if (!is_number (arg))
	{
		send_to_char ("You must provide a number.\n\r", ch);
		return;
	}
	lines = atoi (arg);
	if (lines == 0)
	{
		send_to_char ("Paging disabled.\n\r", ch);
		ch->lines = 0;
		return;
	}
	if (lines < 10 || lines > 100)
	{
		send_to_char ("You must provide a reasonable number.\n\r", ch);
		return;
	}
	sprintf (buf, "Scroll set to %d lines.\n\r", lines);
	send_to_char (buf, ch);
	ch->lines = lines - 2;
}


/* RT does socials */
void do_socials (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int iSocial;
	int col;
	col = 0;
	for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
	{
		sprintf (buf, "%-12s", social_table[iSocial].name);
		send_to_char (buf, ch);
		if (++col % 6 == 0)
			send_to_char ("\n\r", ch);
	}
	if (col % 6 != 0)
		send_to_char ("\n\r", ch);
	return;
}


/* RT Commands to replace news, motd, imotd, etc from ROM */
void do_motd (CHAR_DATA * ch, char *argument)
{
	do_help (ch, "motd");
}

void do_imotd (CHAR_DATA * ch, char *argument)
{
	do_help (ch, "imotd");
}

void do_rules (CHAR_DATA * ch, char *argument)
{
	do_help (ch, "rules");
}

void do_story (CHAR_DATA * ch, char *argument)
{
	do_help (ch, "story");
}

void do_wizlist (CHAR_DATA * ch, char *argument)
{
	do_help (ch, "wizlist");
}

/* RT this following section holds all the auto commands from ROM, as well as
replacements for config */
void do_autolist (CHAR_DATA * ch, char *argument)
{

	/* lists most player flags */
	if (IS_NPC (ch))
		return;

	send_to_char ("   action     status\n\r", ch);
	send_to_char ("---------------------\n\r", ch);

	send_to_char ("autoassist     ", ch);
	if (IS_SET (ch->act, PLR_AUTOASSIST))
	{
		send_to_char ("ON ", ch);
		switch (ch->pcdata->autoassist_level)
		{
		case AASSIST_MOBS:
			send_to_char ("(Mobs)\n\r", ch);
			break;
		case AASSIST_NONE:
			send_to_char ("(OFF)\n\r", ch);
			break;
		}
	}
	else
		send_to_char ("OFF\n\r", ch);
	send_to_char ("autoexit       ", ch);
	if (IS_SET (ch->act, PLR_AUTOEXIT))
		send_to_char ("ON\n\r", ch);

	else
		send_to_char ("OFF\n\r", ch);
	send_to_char ("autogold       ", ch);
	if (IS_SET (ch->act, PLR_AUTOGOLD))
		send_to_char ("ON\n\r", ch);

	else
		send_to_char ("OFF\n\r", ch);
	send_to_char ("autoloot       ", ch);
	if (IS_SET (ch->act, PLR_AUTOLOOT))
		send_to_char ("ON\n\r", ch);

	else
		send_to_char ("OFF\n\r", ch);
	send_to_char ("autoreturn     ", ch);
	if (!IS_SET (ch->act, PLR_AUTORETURN))
		send_to_char ("ON\n\r", ch);

	else
		send_to_char ("OFF\n\r", ch);
	send_to_char ("autosac        ", ch);
	if (IS_SET (ch->act, PLR_AUTOSAC))
		send_to_char ("ON\n\r", ch);

	else
		send_to_char ("OFF\n\r", ch);
	send_to_char ("autosplit      ", ch);
	if (IS_SET (ch->act, PLR_AUTOSPLIT))
		send_to_char ("ON\n\r", ch);

	else
		send_to_char ("OFF\n\r", ch);
	if (ch->pcdata->learned[gsn_butcher] > 0)
	{
		send_to_char ("autobutcher    ", ch);
		if (IS_SET (ch->act, PLR_AUTOBUTCHER))
			send_to_char ("ON\n\r", ch);

		else
			send_to_char ("OFF\n\r", ch);
	}
	send_to_char ("compact mode   ", ch);
	if (IS_SET (ch->comm, COMM_COMPACT))
		send_to_char ("ON\n\r", ch);

	else
		send_to_char ("OFF\n\r", ch);
	send_to_char ("prompt         ", ch);
	if (IS_SET (ch->comm, COMM_PROMPT))
		send_to_char ("ON\n\r", ch);

	else
		send_to_char ("OFF\n\r", ch);
	send_to_char ("combine items  ", ch);
	if (IS_SET (ch->comm, COMM_COMBINE))
		send_to_char ("ON\n\r", ch);

	else
		send_to_char ("OFF\n\r", ch);

	send_to_char ("cardkill       ", ch);
	if (IS_SET (ch->act, PLR_CARDKILL))
		send_to_char ("ON\n\r",ch);
	else send_to_char ("OFF\n\r",ch);

	/*  if (!IS_SET(ch->act, PLR_CANLOOT))
	send_to_char("Your corpse is safe from thieves.\n\r", ch);
	else
	send_to_char("Your corpse may be looted.\n\r", ch);

	if (IS_SET(ch->act, PLR_NOSUMMON))
	send_to_char("You cannot be summoned.\n\r", ch);
	else
	send_to_char("You can be summoned.\n\r", ch);
	*/
	if (IS_SET (ch->act, PLR_NOFOLLOW))
		send_to_char ("You do not welcome followers.\n\r", ch);

	else
		send_to_char ("You accept followers.\n\r", ch);
}


void
do_autoassist (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	if (IS_NPC (ch))
		return;

	one_argument (argument, arg);

	if (!IS_CLASS(ch,PC_CLASS_CLERIC))
	{
		send_to_char("You must autoassist all mobs, and no players, so you don't need this command.\n\r",ch);
		return;
	}

	if ((ch->level > 61 && (ch->Class != ch->Class2 || ch->Class != ch->Class3)) || (ch->level > 31 && (ch->Class != ch->Class2)))
	{
		send_to_char("You must autoassist all mobs, and no players, so you don't need this command!\n\r",ch);
		return;
	}

	if (arg[0] == '\0')
		// Minax 12-20-02 Everyone must autoassist.
		// Iblis 10-20-03 But not clerics
	{       
		send_to_char
			("You must specify an autoassist mode.  Valid modes are:\n\r",
			ch);
		send_to_char
			(" mobs    : Assist group members only when fighting mobiles.\n\r",
			ch);
		send_to_char
			(" none    : Do not assist group members.\n\r",
			ch);
		if (ch->pcdata->autoassist_level == AASSIST_MOBS)
		{
			send_to_char ("\n\rYou currently assist against mobiles.\n\r", ch);
		}
		else if (ch->pcdata->autoassist_level == AASSIST_NONE)
		{
			send_to_char ("\n\rYou currently do not autoassist.\n\r", ch);
		}
		else
			return;
		return;
	}

	if (!str_cmp (arg, "mobs"))
	{
		send_to_char ("You will now assist against mobiles when needed.\n\r",
			ch);
		SET_BIT (ch->act, PLR_AUTOASSIST);
		ch->pcdata->autoassist_level = AASSIST_MOBS;
	}
	else if (!str_cmp (arg, "none"))
	{
		send_to_char ("You will no longer auto assist.\n\r",
			ch);
		REMOVE_BIT (ch->act, PLR_AUTOASSIST);
		ch->pcdata->autoassist_level = AASSIST_NONE;
	}
	else
	{
		send_to_char
			("You must specify an autoassist mode.  Valid modes are:\n\r", ch);
		send_to_char
			(" none    : Do not assist group members.\n\r",
			ch);
		send_to_char
			(" mobs    : Assist group members only when fighting mobiles.\n\r",
			ch);
	}
}

void do_autobutcher (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;
	if (ch->pcdata->learned[gsn_butcher] < 1)
	{
		send_to_char
			("You can't set autobutcher without the butcher skill!", ch);
		return;
	}
	if (IS_SET (ch->act, PLR_AUTOBUTCHER))
	{
		send_to_char ("Autobutcher removed.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_AUTOBUTCHER);
	}

	else
	{
		send_to_char ("Automatic butchering set.\n\r", ch);
		SET_BIT (ch->act, PLR_AUTOBUTCHER);
	}
}
void do_autoexit (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;
	if (IS_SET (ch->act, PLR_AUTOEXIT))
	{
		send_to_char ("Exits will no longer be displayed.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_AUTOEXIT);
	}

	else
	{
		send_to_char ("Exits will now be displayed.\n\r", ch);
		SET_BIT (ch->act, PLR_AUTOEXIT);
	}
}
void do_autogold (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;
	if (IS_SET (ch->act, PLR_AUTOGOLD))
	{
		send_to_char ("Autogold removed.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_AUTOGOLD);
	}

	else
	{
		send_to_char ("Automatic gold looting set.\n\r", ch);
		SET_BIT (ch->act, PLR_AUTOGOLD);
	}
}
void do_autoreturn (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;
	if (IS_SET (ch->act, PLR_AUTORETURN))
	{
		send_to_char ("Autoreturn added.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_AUTORETURN);
	}

	else
	{
		send_to_char ("Autoreturn removed.\n\r", ch);
		SET_BIT (ch->act, PLR_AUTORETURN);
	}
}
void do_autoloot (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;
	if (IS_SET (ch->act, PLR_AUTOLOOT))
	{
		send_to_char ("Autolooting removed.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_AUTOLOOT);
	}

	else
	{
		send_to_char ("Automatic corpse looting set.\n\r", ch);
		SET_BIT (ch->act, PLR_AUTOLOOT);
	}
}
void do_autosac (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;
	if (IS_SET (ch->act, PLR_AUTOSAC))
	{
		send_to_char ("Autosacrificing removed.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_AUTOSAC);
	}

	else
	{
		send_to_char ("Automatic corpse sacrificing set.\n\r", ch);
		SET_BIT (ch->act, PLR_AUTOSAC);
	}
}
void do_autosplit (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;
	if (IS_SET (ch->act, PLR_AUTOSPLIT))
	{
		send_to_char ("Autosplitting removed.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_AUTOSPLIT);
	}

	else
	{
		send_to_char ("Automatic gold splitting set.\n\r", ch);
		SET_BIT (ch->act, PLR_AUTOSPLIT);
	}
}
void do_brief (CHAR_DATA * ch, char *argument)
{
	if (IS_SET (ch->comm, COMM_BRIEF))
	{
		send_to_char ("Full descriptions activated.\n\r", ch);
		REMOVE_BIT (ch->comm, COMM_BRIEF);
	}

	else
	{
		send_to_char ("Short descriptions activated.\n\r", ch);
		SET_BIT (ch->comm, COMM_BRIEF);
	}
}
void do_compact (CHAR_DATA * ch, char *argument)
{
	if (IS_SET (ch->comm, COMM_COMPACT))
	{
		send_to_char ("Compact mode removed.\n\r", ch);
		REMOVE_BIT (ch->comm, COMM_COMPACT);
	}

	else
	{
		send_to_char ("Compact mode set.\n\r", ch);
		SET_BIT (ch->comm, COMM_COMPACT);
	}
}
void send_color_status (CHAR_DATA * ch, char *a, char *c, byte b)
{
	sprintf (c, "%s %s %s\n\r", color_table[b], a, ANSI_NORMAL);
	send_to_char (c, ch);
}

void color_usage (CHAR_DATA * ch)
{
	char oneline[MAX_STRING_LENGTH];
	send_to_char
		("Color Usage:\n\r'color on'\t'color off'\t'color default'\t'color <color> <option>'\n\r\n\r",
		ch);
	send_to_char ("Colors:\n\r\n\r", ch);
	sprintf (oneline, "`a%-18s`b%-18s`c%-18s`d%-18s\n\r", "GREY``",
		"RED``", "GREEN``", "YELLOW``");
	send_to_char (oneline, ch);
	sprintf (oneline, "`e%-18s`f%-18s`g%-18s`h%-18s\n\r", "BLUE``",
		"PURPLE``", "CYAN``", "WHITE``");
	send_to_char (oneline, ch);
	sprintf (oneline, "`i%-18s`j%-18s`k%-18s`l%-18s\n\r", "BOLD RED``",
		"BOLD GREEN``", "BOLD YELLOW``", "BOLD BLUE``");
	send_to_char (oneline, ch);
	sprintf (oneline, "`m%-18s`n%-18s`o%-18s\n\r\n\r", "BOLD PURPLE``",
		"BOLD CYAN``", "BOLD WHITE``");
	send_to_char (oneline, ch);
	send_to_char ("``Options:\n\r\n\r", ch);
	sprintf (oneline, "`A%-18s`B%-18s`C%-18s`D%-18s\n\r", "COMBAT SELF``",
		"COMBAT OPPONENT``", "CONDITION SELF``", "CONDITION OPPONENT``");
	send_to_char (oneline, ch);
	sprintf (oneline, "`E%-18s`F%-18s`G%-18s`H%-18s\n\r", "INVIS``",
		"HP``", "HIDDEN``", "CHARMED``");
	send_to_char (oneline, ch);
	sprintf (oneline, "`I%-18s`J%-18s`K%-18s`L%-18s\n\r", "MANA``",
		"MOVE``", "SAY``", "TELL``");
	send_to_char (oneline, ch);
	sprintf (oneline, "`Q%-18s`O%-18s`P%-18s`N%-18s\n\r", "MUSIC``",
		"GUILD TALK``", "GROUP TELL``", "AUCTION``");
	send_to_char (oneline, ch);
	if (ch->level > 99)
		sprintf (oneline, "`M%-18s`R%-18s`S%-18s\n\r", "GOSSIP``",
		"IMMTALK``", "ADMIN``");

	else if (ch->level > 91)
		sprintf (oneline, "`M%-18s`R%-18s\n\r", "GOSSIP``", "IMMTALK``");

	else
		sprintf (oneline, "`M%-18s\n\r", "GOSSIP``");
	send_to_char (oneline, ch);
	send_to_char ("```\n\r\n\r", ch);
	return;
}

void do_color (CHAR_DATA * ch, char *argument)
{
	char cbuf[MAX_INPUT_LENGTH];
	byte scolor;
	if (IS_NPC (ch))
	{
		send_to_char ("NPC's can't see colors!\n", ch);
		return;
	}
	if (argument[0] == 0)
	{
		color_usage (ch);
		return;
	}

	else
	{
		argument = one_argument (argument, cbuf);
		if (strcasecmp (cbuf, "default") == 0)
		{
			send_to_char ("Reverting Ansi colors to defaults.\n\r", ch);
			ch->pcdata->color_combat_s = 7;
			ch->pcdata->color_combat_o = 1;
			ch->pcdata->color_combat_condition_s = 23;
			ch->pcdata->color_combat_condition_o = 3;
			ch->pcdata->color_invis = 14;
			ch->pcdata->color_hp = 6;
			ch->pcdata->color_hidden = 0;
			ch->pcdata->color_charmed = 12;
			ch->pcdata->color_mana = 6;
			ch->pcdata->color_move = 6;
			ch->pcdata->color_say = 10;
			ch->pcdata->color_tell = 2;
			ch->pcdata->color_guild_talk = 12;
			ch->pcdata->color_group_tell = 5;
			ch->pcdata->color_music = 8;
			ch->pcdata->color_gossip = 5;
			ch->pcdata->color_auction = 13;
			ch->pcdata->color_immtalk = 10;
			ch->pcdata->color_admtalk = 5;
			return;
		}
		if (strcasecmp (cbuf, "on") == 0)
		{
			ch->color = 1;
			send_to_char ("Ansi color turned on.\n\r", ch);
			return;
		}

		else if (strcasecmp (cbuf, "off") == 0)
		{
			ch->color = 0;
			send_to_char ("Ansi color turned off.\n\r", ch);
			return;
		}
		if (str_cmp ("bold", cbuf) != 0)
		{
			if (!str_cmp (cbuf, "grey"))
				scolor = 0;

			else if (!str_cmp (cbuf, "red"))
				scolor = 1;

			else if (!str_cmp (cbuf, "green"))
				scolor = 2;

			else if (!str_cmp (cbuf, "yellow"))
				scolor = 3;

			else if (!str_cmp (cbuf, "blue"))
				scolor = 4;

			else if (!str_cmp (cbuf, "purple"))
				scolor = 5;

			else if (!str_cmp (cbuf, "cyan"))
				scolor = 6;

			else if (!str_cmp (cbuf, "white"))
				scolor = 7;

			else
			{
				color_usage (ch);
				return;
			}
		}

		else
		{
			argument = one_argument (argument, cbuf);
			if (!str_cmp (cbuf, "red"))
				scolor = 8;

			else if (!str_cmp (cbuf, "green"))
				scolor = 9;

			else if (!str_cmp (cbuf, "yellow"))
				scolor = 10;

			else if (!str_cmp (cbuf, "blue"))
				scolor = 11;

			else if (!str_cmp (cbuf, "purple"))
				scolor = 12;

			else if (!str_cmp (cbuf, "cyan"))
				scolor = 13;

			else if (!str_cmp (cbuf, "white"))
				scolor = 14;

			else
			{
				color_usage (ch);
				return;
			}
		}
		argument = one_argument (argument, cbuf);
		if (!str_cmp (cbuf, "combat"))
		{
			argument = one_argument (argument, cbuf);
			if (!str_cmp (cbuf, "self"))
			{
				ch->pcdata->color_combat_s = scolor;
				send_to_char ("`ACOMBAT SELF ``color changed.\n\r", ch);
			}

			else if (!str_cmp (cbuf, "opponent"))
			{
				ch->pcdata->color_combat_o = scolor;
				send_to_char ("`BCOMBAT OPPONENT ``color changed.\n\r", ch);
			}

			else
			{
				color_usage (ch);
				return;
			}
		}

		else if (!str_cmp (cbuf, "condition"))
		{
			argument = one_argument (argument, cbuf);
			if (!str_cmp (cbuf, "self"))
			{
				ch->pcdata->color_combat_condition_s = scolor;
				send_to_char ("'CCONDITION SELF ``color changed.\n\r", ch);
			}

			else if (!str_cmp (cbuf, "opponent"))
			{
				ch->pcdata->color_combat_condition_o = scolor;
				send_to_char ("`DCONDITION OPPONENT ``color changed.\n\r", ch);
			}

			else
			{
				color_usage (ch);
				return;
			}
		}

		else if (!str_cmp (cbuf, "guild"))
		{
			argument = one_argument (argument, cbuf);
			if (!str_cmp (cbuf, "talk"))
			{
				ch->pcdata->color_guild_talk = scolor;
				send_to_char ("`OGUILD TALK ``color changed.\n\r", ch);
			}

			else
			{
				color_usage (ch);
				return;
			}
		}

		else if (!str_cmp (cbuf, "group"))
		{
			argument = one_argument (argument, cbuf);
			if (!str_cmp (cbuf, "tell"))
			{
				ch->pcdata->color_group_tell = scolor;
				send_to_char ("`OGROUP TELL ``color changed.\n\r", ch);
			}

			else
			{
				color_usage (ch);
				return;
			}
		}

		else if (!str_cmp (cbuf, "invis"))
		{
			ch->pcdata->color_invis = scolor;
			send_to_char ("`EINVIS ``color changed.\n\r", ch);
		}

		else if (!str_cmp (cbuf, "hp"))
		{
			ch->pcdata->color_hp = scolor;
			send_to_char ("`FHP ``color changed.\n\r", ch);
		}

		else if (!str_cmp (cbuf, "hidden"))
		{
			ch->pcdata->color_hidden = scolor;
			send_to_char ("`GHIDDEN ``color changed.\n\r", ch);
		}

		else if (!str_cmp (cbuf, "charmed"))
		{
			ch->pcdata->color_charmed = scolor;
			send_to_char ("`HCHARMED ``color changed.\n\r", ch);
		}

		else if (!str_cmp (cbuf, "mana"))
		{
			ch->pcdata->color_mana = scolor;
			send_to_char ("`IMANA ``color changed.\n\r", ch);
		}

		else if (!str_cmp (cbuf, "move"))
		{
			ch->pcdata->color_move = scolor;
			send_to_char ("`JMOVE ``color changed.\n\r", ch);
		}

		else if (!str_cmp (cbuf, "say"))
		{
			ch->pcdata->color_say = scolor;
			send_to_char ("`KSAY ``color changed.\n\r", ch);
		}

		else if (!str_cmp (cbuf, "tell"))
		{
			ch->pcdata->color_tell = scolor;
			send_to_char ("`LTELL ``color changed.\n\r", ch);
		}

		else if (!str_cmp (cbuf, "music"))
		{
			ch->pcdata->color_music = scolor;
			send_to_char ("`QMUSIC ``color changed.\n\r", ch);
		}

		else if (!str_cmp (cbuf, "gossip"))
		{
			ch->pcdata->color_gossip = scolor;
			send_to_char ("`MGOSSIP ``color changed.\n\r", ch);
		}

		else if (!str_cmp (cbuf, "auction"))
		{
			ch->pcdata->color_auction = scolor;
			send_to_char ("`NAUCTION ``color changed.\n\r", ch);
		}

		else if (!str_cmp (cbuf, "immtalk") && ch->level > 91)
		{
			ch->pcdata->color_immtalk = scolor;
			send_to_char ("`NIMMTALK ``color changed.\n\r", ch);
		}

		else if (!str_cmp (cbuf, "admin") && ch->level > 99)
		{
			ch->pcdata->color_admtalk = scolor;
			send_to_char ("`NADMIN ``color changed.\n\r", ch);
		}

		else
		{
			color_usage (ch);
			return;
		}
	}
}
void do_show (CHAR_DATA * ch, char *argument)
{
	if (IS_SET (ch->comm, COMM_SHOW_AFFECTS))
	{
		send_to_char ("Affects will no longer be shown in score.\n\r", ch);
		REMOVE_BIT (ch->comm, COMM_SHOW_AFFECTS);
	}

	else
	{
		send_to_char ("Affects will now be shown in score.\n\r", ch);
		SET_BIT (ch->comm, COMM_SHOW_AFFECTS);
	}
}
void do_prompt (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	if (argument[0] == '\0')
	{
		if (IS_SET (ch->comm, COMM_PROMPT))
		{
			send_to_char ("You will no longer see prompts.\n\r", ch);
			REMOVE_BIT (ch->comm, COMM_PROMPT);
		}

		else
		{
			send_to_char ("You will now see prompts.\n\r", ch);
			SET_BIT (ch->comm, COMM_PROMPT);
		}
		return;
	}
	if (!strcmp (argument, "all"))
		strcpy (buf, "`a<`lHP`a:`F%h `lMA`a:`I%m `lMV`a:`J%v`a>`` %S ");

	else
	{
		if (strlen (argument) > 50)
			argument[50] = '\0';
		strcpy (buf, argument);
		smash_tilde (buf);
		if (str_suffix ("%c", buf))
			strcat (buf, " ");
	}
	free_string (ch->prompt);
	ch->prompt = str_dup (buf);
	sprintf (buf, "Prompt set to %s\n\r", ch->prompt);
	send_to_char (buf, ch);
	return;
}

void do_combine (CHAR_DATA * ch, char *argument)
{
	if (IS_SET (ch->comm, COMM_COMBINE))
	{
		send_to_char ("Long inventory selected.\n\r", ch);
		REMOVE_BIT (ch->comm, COMM_COMBINE);
	}

	else
	{
		send_to_char ("Combined inventory selected.\n\r", ch);
		SET_BIT (ch->comm, COMM_COMBINE);
	}
}
void do_noloot (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;
	if (IS_SET (ch->act, PLR_CANLOOT))
	{
		send_to_char ("Your corpse is now safe from thieves.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_CANLOOT);
	}

	else
	{
		send_to_char ("Your corpse may now be looted.\n\r", ch);
		SET_BIT (ch->act, PLR_CANLOOT);
	}
}
void do_disband (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;
	if (IS_AFFECTED (ch, AFF_CHARM))
	{
		send_to_char ("Sorry, you can't do that right now.\n\r", ch);
		return;
	}
	if (disband_follower (ch))
		send_to_char ("You disband your group.\n\r", ch);

	else
		send_to_char ("There is no one in your group.\n\r", ch);
}

void do_nofollow (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;
	if (IS_SET (ch->act, PLR_NOFOLLOW))
	{
		send_to_char ("You now accept followers.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_NOFOLLOW);
	}

	else
	{
		if (ch->master != NULL && IS_AFFECTED (ch, AFF_CHARM))
		{
			send_to_char
				("But you don't want to leave your master right now!\n\r", ch);
			return;
		}
		send_to_char ("You no longer accept followers.\n\r", ch);
		SET_BIT (ch->act, PLR_NOFOLLOW);
		die_follower (ch);
	}
}


//
// Akamai 4/5/99 -- Players that are lonered, or in a clan cannot
// 'nosummon' this is affiliated with changes to spell_summon
//
void do_nosummon (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
	{

		// we have a mob thing
		if (IS_SET (ch->imm_flags, IMM_SUMMON))
		{
			send_to_char ("You are no longer immune to summon.\n\r", ch);
			REMOVE_BIT (ch->imm_flags, IMM_SUMMON);
		}

		else
		{
			send_to_char ("You are now immune to summoning.\n\r", ch);
			SET_BIT (ch->imm_flags, IMM_SUMMON);
		}
	}

	else
	{

		// we have a player
		if ((ch->clan == CLAN_BOGUS) && (ch->pcdata->loner == FALSE))
		{
			if (IS_SET (ch->act, PLR_NOSUMMON))
			{
				send_to_char ("You are no longer immune to summon.\n\r", ch);
				REMOVE_BIT (ch->act, PLR_NOSUMMON);
			}

			else
			{
				send_to_char ("You are now immune to summoning.\n\r", ch);
				SET_BIT (ch->act, PLR_NOSUMMON);
			}
		}

		else
		{
			send_to_char
				("Clan members and loners can always be summoned.\n\r", ch);
			REMOVE_BIT (ch->act, PLR_NOSUMMON);
		}
	}
}
void do_look (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char tcbuf[MAX_INPUT_LENGTH];
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char *pdesc, *pname;
	int door = -1;
	int number, count;
	ROOM_INDEX_DATA *location;
	if (ch->desc == NULL)
		return;
	if (ch->position < POS_SLEEPING && !battle_royale)
	{
		send_to_char ("You can't see anything but stars!\n\r", ch);
		return;
	}
	if (get_position(ch) == POS_SLEEPING)
	{
		send_to_char ("You can't see anything, you're sleeping!\n\r", ch);
		return;
	}
	if (!check_blind (ch))
	{
		OBJ_DATA *tobj;
		sense_life (ch);
		for (tobj = ch->in_room->contents;tobj != NULL;tobj = tobj->next_content)
		{
			if (IS_SET(tobj->extra_flags[1],ITEM_PULSATING))
				show_list_to_char (tobj, ch, FALSE, FALSE);
		}
		return;
	}
	if (!IS_NPC (ch)
		&& !IS_SET (ch->act, PLR_HOLYLIGHT) && room_is_dark (ch->in_room)  && (ch->race != PC_RACE_KALIAN) && (ch->race != PC_RACE_LICH))
	{
		OBJ_DATA *tobj;
		send_to_char ("`aIt is pitch black ... ``\n\r", ch);
		for (tobj = ch->in_room->contents;tobj != NULL;tobj = tobj->next_content)
		{
			if (IS_SET(tobj->extra_flags[1],ITEM_PULSATING))
				show_list_to_char (tobj, ch, FALSE, FALSE);
		}
		show_char_to_char (ch->in_room->people, ch);

		sense_life (ch);
		return;
	}
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	number = number_argument (arg1, arg3);
	count = 0;
	if (!str_cmp (arg1, "n") || !str_cmp (arg1, "north"))
		door = 0;

	else if (!str_cmp (arg1, "e") || !str_cmp (arg1, "east"))
		door = 1;

	else if (!str_cmp (arg1, "s") || !str_cmp (arg1, "south"))
		door = 2;

	else if (!str_cmp (arg1, "w") || !str_cmp (arg1, "west"))
		door = 3;

	else if (!str_cmp (arg1, "u") || !str_cmp (arg1, "up"))
		door = 4;

	else if (!str_cmp (arg1, "d") || !str_cmp (arg1, "down"))
		door = 5;
	if (door != -1)
	{
		if ((pexit = ch->in_room->exit[door]) == NULL)
		{
			send_to_char ("Nothing special there.\n\r", ch);
			return;
		}
		if (is_affected(ch,skill_lookup("nearsight")) || ch->race == PC_RACE_SWARM)
		{
			send_to_char ("You can't make out that room from here.\n\r",ch);
			return;
		}

		if (IS_SET (pexit->exit_info, EX_CLOSED))
		{
			if (pexit->description != NULL && pexit->description[0] != '\0')
				send_to_char (pexit->description, ch);
		}

		else
		{
			int counter = 0;
			ROOM_INDEX_DATA *pRoomIndex = pexit->u1.to_room;
			sprintf (tcbuf, "Looking %s, you see...``\n\r", dir_name[door]);
			send_to_char (tcbuf, ch);

			if (ch->Class == PC_CLASS_CHAOS_JESTER)
			{
				pRoomIndex = get_random_room (ch);
				while 
					((IS_SET(pRoomIndex->area->area_flags,AREA_IMP_ONLY) && get_trust(ch) < MAX_LEVEL)
					//	 || (pRoomIndex->area != ch->in_room->area)
					|| IS_SET (pRoomIndex->room_flags, ROOM_ARENA)
					|| pRoomIndex->area->construct
					|| pRoomIndex->sector_type == SECT_WATER_OCEAN
					|| (!can_move_char(ch,pRoomIndex,FALSE,FALSE))
					)
				{
					pRoomIndex = get_random_room (ch);
					if (++counter > 10000)
					{
						send_to_char("nothing.\n\r",ch);
						return;
					}
				}
			}

			if (IS_SET(pRoomIndex->room_flags2,ROOM_ALWAYS_DARK)
				&& !(ch->race == PC_RACE_KALIAN)
				&& !(ch->race == PC_RACE_LICH)
				&& !(IS_IMMORTAL(ch) && IS_SET(ch->act,PLR_HOLYLIGHT)))
			{
				if (is_affected(ch,skill_lookup("infravision")))
				{
					show_char_to_char(pRoomIndex->people,ch);
				}
				else send_to_char("You see nothing but darkness.\n\r",ch);
				return;
			}
			if (pRoomIndex != NULL
				&& pRoomIndex != ch->in_room
				&& can_see_room (ch, pRoomIndex))
			{
				char sub_string[32];
				send_to_char ("``\n\r   ", ch);
				strncpy (sub_string, pRoomIndex->description, 32);
				if (!strncmp (sub_string, "AG_O ", 5))
					send_to_char (ocean_desc[atoi (sub_string + 5)], ch);

				else
					send_to_char (pRoomIndex->description, ch);
				send_to_char ("\n\r", ch);
				show_list_to_char (pRoomIndex->contents, ch, FALSE,
					FALSE);
				show_char_to_char (pRoomIndex->people, ch);
				sense_life (ch);
			}

			else
				send_to_char ("Nothing.\n\r", ch);
		}
		if (pexit->keyword != NULL
			&& pexit->keyword[0] != '\0' && pexit->keyword[0] != ' ' && ch->Class != PC_CLASS_CHAOS_JESTER)
		{
			if (IS_SET (pexit->exit_info, EX_CLOSED))
			{
				act ("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
			}

			else if (IS_SET (pexit->exit_info, EX_ISDOOR))
			{
				act ("The $d is open.", ch, NULL, pexit->keyword, TO_CHAR);
			}
		}
		return;
	}
	if (arg1[0] == '\0' || !str_cmp (arg1, "auto"))
	{

		/* 'look' or 'look auto' */
		if (!IS_NPC (ch))
		{
			if (getbit (ch->pcdata->explored, ch->in_room->vnum) == 0)
			{
				gain_exp (ch, 100 * 4);
			}
			setbit (ch->pcdata->explored, ch->in_room->vnum);
		}
		send_to_char ("`k", ch);
		send_to_char (ch->in_room->name, ch);
		send_to_char ("``", ch);
		if (IS_IMMORTAL (ch)
			&& (IS_NPC (ch) || IS_SET (ch->act, PLR_HOLYLIGHT)))
		{
			sprintf (buf, " [Room %d]", ch->in_room->vnum);
			send_to_char (buf, ch);
		}
		send_to_char ("\n\r", ch);
		if (arg1[0] == '\0' || (!IS_NPC (ch) && !IS_SET (ch->comm, COMM_BRIEF)))
		{
			send_to_char ("  ", ch);

			/* STUFF ADDED FOR COLORS */
			switch (ch->in_room->sector_type)
			{
			case SECT_UNDERWATER:
				send_to_char ("`e", ch);
				break;
			case SECT_WATER_SWIM:
			case SECT_WATER_NOSWIM:
			case SECT_WATER_OCEAN:
				send_to_char ("`g", ch);
				break;
			case SECT_FIELD:
			case SECT_FOREST:
			case SECT_GRASSLAND:
				send_to_char ("`c", ch);
				break;
			case SECT_SHORE:
				send_to_char ("`d", ch);
				break;
			case SECT_HILLS:
			case SECT_DESERT:
				send_to_char ("`d", ch);
				break;
			case SECT_MOUNTAIN:
				send_to_char ("`h", ch);
				break;
			case SECT_AIR:
				send_to_char ("`n", ch);
				break;
			case SECT_TUNDRA:
				send_to_char ("`o", ch);
				break;
			case SECT_WASTELAND:
			case SECT_UNDERGROUND:
			case SECT_SWAMP:
				send_to_char ("`a", ch);
				break;
			default:
				send_to_char ("`l", ch);
				break;
			}
			{
				char sub_string[32];
				strncpy (sub_string, ch->in_room->description, 32);
				if (!strncmp (sub_string, "AG_O ", 5))
					send_to_char (ocean_desc[atoi (sub_string + 5)], ch);

				else
					send_to_char (ch->in_room->description, ch);
				send_to_char ("``", ch);
			}
		}
		if (!IS_NPC (ch) && IS_SET (ch->act, PLR_AUTOEXIT))
		{
			send_to_char ("\n\r", ch);
			do_exits (ch, "auto");
		}
		show_list_to_char (ch->in_room->contents, ch, FALSE, FALSE);
		show_char_to_char (ch->in_room->people, ch);
		send_to_char ("``", ch);
		sense_life (ch);
		if (detect_room_trap(ch,ch->in_room))
			send_to_char("`bYou detect a trap in this room!``\n\r",ch);
		return;
	}
	if (!str_cmp (arg1, "o") || !str_cmp (arg1, "out"))
	{

		/* 'look out' */
		if (IS_SET (ch->in_room->room_flags, ROOM_ELEVATOR))
		{
			OBJ_DATA *eObj;
			for (eObj = object_list; eObj; eObj = eObj->next)
			{
				if (eObj->item_type != ITEM_ELEVATOR)
					continue;
				if (!is_name (arg2, eObj->name))
					continue;
				if (eObj->value[0] == ch->in_room->vnum)
				{
					location = eObj->in_room;
					if (location == NULL)
						continue;
					if (location->vnum == ROOM_VNUM_ELIMBO)
					{
						act ("You can't currently see out of $p.", ch,
							eObj, NULL, TO_CHAR);
						return;
					}
					if (location != NULL && location != ch->in_room
						&& can_see_room (ch, location))
					{
						act ("Looking out of $p, you see...", ch, eObj,
							NULL, TO_CHAR);
						{
							char sub_string[32];
							switch (location->sector_type)
							{
							case SECT_UNDERWATER:
								send_to_char ("`e", ch);
								break;
							case SECT_WATER_SWIM:
							case SECT_WATER_NOSWIM:
							case SECT_WATER_OCEAN:
								send_to_char ("`g", ch);
								break;
							case SECT_FIELD:
							case SECT_FOREST:
							case SECT_GRASSLAND:
								send_to_char ("`c", ch);
								break;
							case SECT_HILLS:
							case SECT_DESERT:
								send_to_char ("`d", ch);
								break;
							case SECT_SHORE:
								send_to_char ("`d",ch);
								break;
							case SECT_MOUNTAIN:
								send_to_char ("`h", ch);
								break;
							case SECT_AIR:
								send_to_char ("`n", ch);
								break;
							case SECT_TUNDRA:
								send_to_char ("`o", ch);
								break;
							case SECT_UNDERGROUND:
							case SECT_WASTELAND:
							case SECT_SWAMP:
								send_to_char ("`a", ch);
								break;
							default:
								send_to_char ("`l", ch);
								break;
							}
							strncpy (sub_string, location->description, 32);
							if (!strncmp (sub_string, "AG_O ", 5))
								send_to_char (ocean_desc
								[atoi (sub_string + 5)], ch);

							else
								send_to_char (location->description, ch);
							send_to_char ("``\n\r", ch);
						}
						show_list_to_char (location->contents, ch, FALSE,
							FALSE);
						show_char_to_char (location->people, ch);
						sense_life (ch);
						return;
					}

					else
					{
						act ("You can't currently see out of $p.", ch,
							eObj, NULL, TO_CHAR);
						return;
					}
				}
			}
		}
		if (IS_SET (ch->in_room->room_flags, ROOM_TRANSPORT))
		{
			OBJ_DATA *eObj;
			for (eObj = object_list; eObj; eObj = eObj->next)
			{
				if (eObj->item_type != ITEM_CTRANSPORT)
					continue;

				if (eObj->value[3] == ch->in_room->vnum)
				{
					location = eObj->in_room;
					if (location == NULL)
						continue;
					if (location != NULL && location != ch->in_room
						&& can_see_room (ch, location))
					{
						act ("Looking out of $p, you see...", ch, eObj,
							NULL, TO_CHAR);
						{
							char sub_string[32];
							switch (location->sector_type)
							{
							case SECT_UNDERWATER:
								send_to_char ("`e", ch);
								break;
							case SECT_WATER_SWIM:
							case SECT_WATER_NOSWIM:
							case SECT_WATER_OCEAN:
								send_to_char ("`g", ch);
								break;
							case SECT_FIELD:
							case SECT_FOREST:
							case SECT_GRASSLAND:
								send_to_char ("`c", ch);
								break;
							case SECT_HILLS:
							case SECT_DESERT:
								send_to_char ("`d", ch);
								break;
							case SECT_SHORE:
								send_to_char ("`d",ch);
								break;
							case SECT_MOUNTAIN:
								send_to_char ("`h", ch);
								break;
							case SECT_AIR:
								send_to_char ("`n", ch);
								break;
							case SECT_TUNDRA:
								send_to_char ("`o", ch);
								break;
							case SECT_UNDERGROUND:
							case SECT_WASTELAND:
							case SECT_SWAMP:
								send_to_char ("`a", ch);
								break;
							default:
								send_to_char ("`l", ch);
								break;
							}
							strncpy (sub_string, location->description, 32);
							if (!strncmp (sub_string, "AG_O ", 5))
								send_to_char (ocean_desc
								[atoi (sub_string + 5)], ch);

							else
								send_to_char (location->description, ch);
						}
						send_to_char ("``\n\r", ch);
						do_exits (ch, "rrcar");
						show_list_to_char_but (location->contents, ch,
							FALSE, FALSE, eObj);
						show_char_to_char (location->people, ch);
						sense_life (ch);
						return;
					}

					else
					{
						act ("You can't currently see out of $p.", ch,
							eObj, NULL, TO_CHAR);
						return;
					}
				}
			}
		}
		if (arg2[0] == '\0')
		{
			send_to_char ("Look out of what?\n\r", ch);
			return;
		}
		if ((obj = get_obj_here (ch, arg2)) == NULL)
		{
			send_to_char ("You do not see that here.\n\r", ch);
			return;
		}
		switch (obj->item_type)
		{
		default:
			send_to_char ("That is not an exit.\n\r", ch);
			return;
		case ITEM_PORTAL:
			if (!IS_SET (obj->value[4], PACT_EXIT))
			{
				send_to_char ("That is not an exit.\n\r", ch);
				return;
			}
			if (IS_SET (obj->value[1], EX_CLOSED))
			{
				send_to_char ("It is closed.\n\r", ch);
				return;
			}
			location = get_room_index (obj->value[3]);
			sprintf (tcbuf, "Looking out of %s, you see...\n\r",
				obj->short_descr);
			send_to_char (tcbuf, ch);
			if (location != NULL && location != ch->in_room
				&& can_see_room (ch, location))
			{
				send_to_char ("\n\r", ch);
				{
					char sub_string[32];
					strncpy (sub_string, location->description, 32);
					if (!strncmp (sub_string, "AG_O ", 5))
						send_to_char (ocean_desc[atoi (sub_string + 5)], ch);

					else
						send_to_char (location->description, ch);
				}
				send_to_char ("\n\r", ch);
				show_list_to_char (location->contents, ch, FALSE, FALSE);
				show_char_to_char (location->people, ch);
				sense_life (ch);
			}

			else
				act ("$p doesn't seem to go anywhere.", ch, obj, NULL, TO_CHAR);
			break;
		}
		return;
	}
	if (!str_cmp (arg1, "i") || !str_cmp (arg1, "in") || !str_cmp (arg1, "on"))
	{

		/* 'look in' */
		if (arg2[0] == '\0')
		{
			send_to_char ("Look in what?\n\r", ch);
			return;
		}
		if ((obj = get_obj_here (ch, arg2)) == NULL)
		{
			send_to_char ("You do not see that here.\n\r", ch);
			return;
		}
		switch (obj->item_type)
		{
		default:
			send_to_char ("That is not a container.\n\r", ch);
			break;
		case ITEM_ELEVATOR:
			location = get_room_index (obj->value[0]);
			act ("Looking into $p, you see...", ch, obj, NULL, TO_CHAR);
			if (location != NULL && location != ch->in_room
				&& can_see_room (ch, location))
			{
				send_to_char ("\n\r", ch);
				{
					char sub_string[32];
					strncpy (sub_string, location->description, 32);
					if (!strncmp (sub_string, "AG_O ", 5))
						send_to_char (ocean_desc[atoi (sub_string + 5)], ch);

					else
						send_to_char (location->description, ch);
				}
				send_to_char ("\n\r", ch);
				show_list_to_char (location->contents, ch, FALSE, FALSE);
				show_char_to_char (location->people, ch);
				sense_life (ch);
			}

			else
				act ("$p doesn't seem to go anywhere.", ch, obj, NULL, TO_CHAR);
			break;
		case ITEM_PORTAL:
			if (obj->value[3] == PORTAL_FUCKED)
			{
				send_to_char
					("You are unable to see what lies beyond.\n\r", ch);
				return;
			}
			if (IS_SET (obj->value[1], EX_CLOSED))
			{
				send_to_char ("It is closed.\n\r", ch);
				return;
			}
			if (IS_SET (obj->value[4], PACT_PLAY))
			{
				send_to_char
					("You can't see the virtual game world without playing.\n\r",
					ch);
				return;
			}
			if (IS_SET (obj->value[4], PACT_EXIT))
			{
				sprintf (tcbuf, "Try looking OUT of the %s.\n\r",
					obj->short_descr);
				send_to_char (tcbuf, ch);
				return;
			}
			if (IS_SET (obj->value[4], PACT_CLIMB)
				|| IS_SET (obj->value[4], PACT_DESCEND)
				|| IS_SET (obj->value[4], PACT_CRAWL)
				|| IS_SET (obj->value[4], PACT_SCALE)
				|| IS_SET (obj->value[4], PACT_JUMP))
			{
				send_to_char
					("You don't see anything interesting there.\n\r", ch);
				return;
			}
			location = get_room_index (obj->value[3]);
			sprintf (tcbuf, "Looking into %s, you see...\n\r",
				obj->short_descr);
			send_to_char (tcbuf, ch);
			if (location != NULL && location != ch->in_room
				&& can_see_room (ch, location))
			{
				send_to_char ("  ", ch);
				{
					char sub_string[32];
					strncpy (sub_string, location->description, 32);
					if (!strncmp (sub_string, "AG_O ", 5))
						send_to_char (ocean_desc[atoi (sub_string + 5)], ch);

					else
						send_to_char (location->description, ch);
				}
				send_to_char ("\n\r", ch);
				show_list_to_char (location->contents, ch, FALSE, FALSE);
				show_char_to_char (location->people, ch);
				sense_life (ch);
			}

			else
				act ("$p doesn't seem to go anywhere.", ch, obj, NULL, TO_CHAR);
			break;
		case ITEM_DRINK_CON:
			if (obj->value[1] <= 0)
			{
				send_to_char ("It is empty.\n\r", ch);
				break;
			}
			if (obj->value[2] >= 0)
				sprintf (buf, "It's %sfilled with a %s liquid.\n\r",
				obj->value[1] <
				obj->value[0] /
				4 ? "less than half-" : obj->value[1] <
				3 * obj->value[0] /
				4 ? "about half-" : "more than half-",
				liq_table[obj->value[2]].liq_color);

			else
				sprintf (buf, "It's %sfilled with an unknown liquid.\n\r",
				obj->value[1] <
				obj->value[0] /
				4 ? "less than half-" : obj->value[1] <
				3 * obj->value[0] /
				4 ? "about half-" : "more than half-");
			send_to_char (buf, ch);
			break;
		case ITEM_CLAN_DONATION:
		case ITEM_NEWCLANS_DBOX:
		case ITEM_PLAYER_DONATION:
			act ("$p holds:", ch, obj, NULL, TO_CHAR);
			show_list_to_char (obj->contains, ch, TRUE, TRUE);
			break;
		case ITEM_PACK:
		case ITEM_QUIVER:
		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			if (IS_SET (obj->value[1], CONT_CLOSED))
			{
				send_to_char ("It is closed.\n\r", ch);
				break;
			}
			act ("$p holds:", ch, obj, NULL, TO_CHAR);
			show_list_to_char (obj->contains, ch, TRUE, TRUE);
			break;
		}
		return;
	}
	if ((!str_cmp (arg1, "book"))
		&& IS_SET (ch->in_room->room_flags, ROOM_ARENA_REGISTRATION))
	{
		show_duel_record (ch);
		act ("$n studies the leather bound book intently.", ch, NULL,
			argument, TO_ROOM);
		return;
	}
	if ((victim = get_char_room (ch, arg1)) != NULL)
	{
		show_char_to_char_1 (victim, ch);
		return;
	}
	pdesc = get_extra_descr (arg3, ch->in_room->extra_descr);
	pname = get_extra_name (arg3, ch->in_room->extra_descr);
	if (pdesc != NULL && pname != NULL)
	{
		if (++count == number)
		{
			page_to_char (pdesc, ch);
			switch (number_range (0, 2))
			{
			case 0:
				sprintf (buf, "$n looks at the %s.", pname);
				act (buf, ch, NULL, NULL, TO_ROOM);
				break;
			case 1:
				sprintf (buf, "$n examines the %s closely.", pname);
				act (buf, ch, NULL, NULL, TO_ROOM);
				break;
			case 2:
				sprintf (buf, "$n studies the %s intently.", pname);
				act (buf, ch, NULL, NULL, TO_ROOM);
				break;
			}
			return;
		}
	}
	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	{
		if (can_see_obj (ch, obj))
		{			/* player can see object */
			pdesc = get_extra_descr (arg3, obj->extra_descr);
			if (pdesc != NULL)
			{
				if (++count == number)
				{
					page_to_char (pdesc, ch);
					return;
				}

				else
					continue;
			}
			pdesc = get_extra_descr (arg3, obj->pIndexData->extra_descr);
			if (pdesc != NULL)
			{
				if (++count == number)
				{
					page_to_char (pdesc, ch);
					return;
				}

				else
					continue;
			}
			if (is_name (arg3, obj->name))
				if (++count == number)
				{
					char *cptr;
					if (obj->item_type == ITEM_WRITING_PAPER)
					{
						if ((cptr =
							get_extra_descr ("papermsg",
							obj->extra_descr)) == NULL)
						{
							act ("$p is blank.", ch, obj, NULL, TO_CHAR);
							return;
						}

						else
						{
							page_to_char (cptr, ch);
							return;
						}
					}

					else if (obj->item_type == ITEM_MAP)
					{
						if ((cptr =
							get_extra_descr ("maptxt",
							obj->extra_descr)) == NULL)
						{
							act ("$p is blank.", ch, obj, NULL, TO_CHAR);
							return;
						}

						else
						{
							send_to_char (cptr, ch);
							return;
						}
					}
					send_to_char (obj->description, ch);
					send_to_char ("\n\r", ch);
					return;
				}
		}
	}
	for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
	{
		if (can_see_obj (ch, obj))
		{
			pdesc = get_extra_descr (arg3, obj->extra_descr);
			if (pdesc != NULL)
				if (++count == number)
				{
					send_to_char (pdesc, ch);
					return;
				}
				pdesc = get_extra_descr (arg3, obj->pIndexData->extra_descr);
				if (pdesc != NULL)
					if (++count == number)
					{
						page_to_char (pdesc, ch);
						switch (number_range (0, 2))
						{
						case 0:
							act ("$n looks at $p.", ch, obj, NULL, TO_ROOM);
							break;
						case 1:
							act ("$n examines $p closely.", ch, obj, NULL, TO_ROOM);
							break;
						case 2:
							act ("$n studies $p intently.", ch, obj, NULL, TO_ROOM);
							break;
						}
						return;
					}
		}
		if (is_name (arg3, obj->name))
			if (++count == number)
			{
				char buf[MAX_STRING_LENGTH];
				if (obj->pIndexData->vnum == OBJ_VNUM_CLAN_SYMBOL)
				{
					sprintf (buf, obj->description,
						get_clan_symbol (ch->in_room->area->clan));
				}

				else
				{
					if (obj->description[0] != '\0')
					{
						strcpy (buf, obj->description);
						send_to_char (buf, ch);
						send_to_char ("\n\r", ch);
					}
				}
				switch (number_range (0, 2))
				{
				case 0:
					act ("$n looks at $p.", ch, obj, NULL, TO_ROOM);
					break;
				case 1:
					act ("$n examines $p closely.", ch, obj, NULL, TO_ROOM);
					break;
				case 2:
					act ("$n studies $p intently.", ch, obj, NULL, TO_ROOM);
					break;
				}
				return;
			}
	}
	pdesc = get_extra_descr (arg3, ch->in_room->extra_descr);
	if (pdesc != NULL)
	{
		if (++count == number)
		{
			send_to_char (pdesc, ch);
			return;
		}
	}
	if (count > 0 && count != number)
	{
		if (count == 1)
			sprintf (buf, "You only see one %s here.\n\r", arg3);

		else
			sprintf (buf, "You only see %d of those here.\n\r", count);
		send_to_char (buf, ch);
		return;
	}
	send_to_char ("You do not see that here.\n\r", ch);
	return;
}

void do_glance (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	if (ch->desc == NULL)
		return;
	if (ch->position < POS_SLEEPING)
	{
		send_to_char ("You can't see anything but stars!\n\r", ch);
		return;
	}
	if (get_position(ch) == POS_SLEEPING)
	{
		send_to_char ("You can't see anything, you're sleeping!\n\r", ch);
		return;
	}
	if (!check_blind (ch))
		return;
	if (!IS_NPC (ch)
		&& !IS_SET (ch->act, PLR_HOLYLIGHT) && room_is_dark (ch->in_room) && (ch->race != PC_RACE_KALIAN) && (ch->race != PC_RACE_LICH))
	{
		send_to_char ("`aIt is pitch black ... ``\n\r", ch);
		show_char_to_char (ch->in_room->people, ch);
		return;
	}
	argument = one_argument (argument, arg1);
	if (arg1[0] != '\0')
		if ((victim = get_char_room (ch, arg1)) != NULL)
		{
			show_char_to_char_2 (victim, ch);
			return;
		}
		send_to_char ("You do not see that here.\n\r", ch);
		return;
}


/* RT added back for the hell of it */
void do_read (CHAR_DATA * ch, char *argument)
{
	do_look (ch, argument);
}

void do_examine (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Examine what?\n\r", ch);
		return;
	}
	do_look (ch, arg);
	if ((obj = get_obj_here (ch, arg)) != NULL)
	{
		switch (obj->item_type)
		{
		default:
			break;
		case ITEM_JUKEBOX:
			do_play (ch, "list");
			break;
		case ITEM_MONEY:
			if (obj->value[0] == 0)
			{
				if (obj->value[1] == 0)
					sprintf (buf, "Odd...there are no coins in the pile.\n\r");

				else if (obj->value[1] == 1)
					sprintf (buf, "Wow. One gold coin.\n\r");

				else
					sprintf (buf,
					"There are %d gold coins in the pile.\n\r",
					obj->value[1]);
			}

			else if (obj->value[1] == 0)
			{
				if (obj->value[0] == 1)
					sprintf (buf, "Wow. One silver coin.\n\r");

				else
					sprintf (buf,
					"There are %d silver coins in the pile.\n\r",
					obj->value[0]);
			}

			else
				sprintf (buf,
				"There are %d gold and %d silver coins in the pile.\n\r",
				obj->value[1], obj->value[0]);
			send_to_char (buf, ch);
			break;
		case ITEM_PACK:
		case ITEM_DRINK_CON:
		case ITEM_CONTAINER:
		case ITEM_QUIVER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
		case ITEM_CLAN_DONATION:
		case ITEM_NEWCLANS_DBOX:
		case ITEM_PLAYER_DONATION:
			sprintf (buf, "in %s", argument);
			do_look (ch, buf);
		}
	}
	return;
}


/*
* Thanks to Zrin for auto-exit part.
*/
void do_exits (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char onename[MAX_STRING_LENGTH], *cptr;
	OBJ_DATA *obj;
	EXIT_DATA *pexit;
	bool found;
	bool fAuto, fCar;
	int door, room_vnum;
	ROOM_INDEX_DATA *endloc, *location, *theroom;
	if (str_cmp (argument, "auto") != 0 && argument[0] != '\0'
		&& str_cmp (argument, "rrcar") != 0)
	{
		do_pexit (ch, argument);
		return;
	}
	theroom = ch->in_room;
	fAuto = !str_cmp (argument, "auto");
	fCar = !str_cmp (argument, "rrcar");
	if (fCar)
	{
		OBJ_DATA *eObj;
		fAuto = TRUE;
		for (eObj = object_list; eObj; eObj = eObj->next)
		{
			if (eObj->item_type != ITEM_CTRANSPORT || eObj->in_room == NULL)
				continue;
			if (eObj->value[3] == ch->in_room->vnum)
				theroom = eObj->in_room;
		}
	}
	if (!check_blind (ch))
		return;
	if (!is_affected (ch, gsn_disorientation))
	{
		if (fAuto)
			sprintf (buf, "[Exits:");

		else if (IS_IMMORTAL (ch))
			sprintf (buf, "Obvious exits from room %d:\n\r", theroom->vnum);

		else
			sprintf (buf, "Obvious exits:\n\r");
		found = FALSE;
		for (door = 0; door <= 5; door++)
		{
			if ((pexit = theroom->exit[door]) != NULL
				&& pexit->u1.to_room != NULL
				&& can_see_room (ch, pexit->u1.to_room)
				&& !IS_SET (pexit->exit_info, EX_CLOSED))
			{
				found = TRUE;
				if (fAuto)
				{
					strcat (buf, "`b ");
					strcat (buf, dir_name[door]);
				}

				else
				{
					sprintf (buf + strlen (buf), "%-5s - %s",
						capitalize (dir_name[door]),
						(room_is_dark (pexit->
						u1.to_room) && (ch->race != PC_RACE_KALIAN) && (ch->race != PC_RACE_LICH)) ?
						"Too dark to tell" : pexit->u1.to_room->name);
					if (IS_IMMORTAL (ch))
						sprintf (buf + strlen (buf), " (room %d)\n\r",
						pexit->u1.to_room->vnum);

					else
						sprintf (buf + strlen (buf), "\n\r");
				}
			}
		}
		if (!found && fAuto)
			strcat (buf, " none");
		if (fAuto)
			strcat (buf, "``]");
		if (IS_SET (theroom->room_flags, ROOM_ELEVATOR) && !fCar)
		{
			OBJ_DATA *eObj;
			for (eObj = object_list; eObj; eObj = eObj->next)
			{
				if (eObj->item_type != ITEM_ELEVATOR)
					continue;
				if (eObj->value[0] == theroom->vnum)
				{
					location = eObj->in_room;
					if (location == NULL)
						continue;
					if (location->vnum != ROOM_VNUM_ELIMBO)
					{
						strcpy (onename, eObj->name);
						if ((cptr = strchr (onename, ' ')) != NULL)
							*cptr = '\0';
						if (fAuto)
							sprintf (buf + strlen (buf), " [`bexit %s``]",
							onename);

						else
							sprintf (buf + strlen (buf), "Exit %s - %s",
							onename,
							(room_is_dark (location) && (ch->race != PC_RACE_KALIAN) && (ch->race != PC_RACE_LICH))  ?
							"Too dark to tell" : location->name);
						found = TRUE;
						if (!fAuto)
						{
							if (IS_IMMORTAL (ch))
								sprintf (buf + strlen (buf),
								" (room %d)\n\r", location->vnum);

							else
								sprintf (buf + strlen (buf), "\n\r");
						}
					}
				}
			}
		}
		if (IS_SET (theroom->room_flags, ROOM_TRANSPORT) && !fCar)
		{
			OBJ_DATA *eObj;
			for (eObj = object_list; eObj; eObj = eObj->next)
			{
				if (eObj->item_type != ITEM_CTRANSPORT)
					continue;
				if (IS_SET (eObj->value[5], CONT_CLOSED))
					continue;
				if (eObj->value[2] == theroom->vnum)
				{
					location = eObj->in_room;
					if (location == NULL)
						continue;
					strcpy (onename, eObj->name);
					if ((cptr = strchr (onename, ' ')) != NULL)
						*cptr = '\0';
					if (fAuto)
						sprintf (buf + strlen (buf), " [`bexit %s``]", onename);

					else
						sprintf (buf + strlen (buf), "Exit %s - %s",
						onename,
						(room_is_dark (location) && (ch->race != PC_RACE_KALIAN) && (ch->race != PC_RACE_LICH)) ?
						"Too dark to tell" : location->name);
					found = TRUE;
					if (!fAuto)
					{
						if (IS_IMMORTAL (ch))
							sprintf (buf + strlen (buf), " (room %d)\n\r",
							location->vnum);

						else
							sprintf (buf + strlen (buf), "\n\r");
					}
				}
			}
		}
		for (obj = theroom->contents; obj; obj = obj->next_content)
		{
			if (obj->item_type != ITEM_PORTAL
				&& obj->item_type != ITEM_ELEVATOR
				&& obj->item_type != ITEM_CTRANSPORT)
				continue;
			if (obj->item_type == ITEM_CTRANSPORT
				&& IS_SET (obj->value[5], CONT_CLOSED))
				continue;
			if (obj->item_type == ITEM_PORTAL
				&& !IS_SET (obj->value[2], GATE_SHOWEXIT))
				continue;
			if (!can_see_obj(ch,obj))
				continue;
			strcpy (onename, obj->name);
			if ((cptr = strchr (onename, ' ')) != NULL)
				*cptr = '\0';
			if (obj->item_type == ITEM_PORTAL)
				room_vnum = obj->value[3];

			else if (obj->item_type == ITEM_ELEVATOR)
				room_vnum = obj->value[0];

			else			/* Ctransport */
				room_vnum = obj->value[2];
			endloc = get_room_index (room_vnum);
			if (endloc == NULL)
				continue;
			if (((obj->item_type
				== ITEM_PORTAL && IS_SET (obj->value[4], PACT_ENTER))
				|| obj->item_type == ITEM_ELEVATOR
				|| obj->item_type == ITEM_CTRANSPORT) && !fCar)
			{
				if (fAuto)
				{
					if (obj->item_type == ITEM_CTRANSPORT &&
						(IS_SET (obj->value[4], TACT_SAIL)
						|| IS_SET (obj->value[4], TACT_PADDLE)
						|| IS_SET (obj->value[4], TACT_ROW)))
						sprintf (buf + strlen (buf), " [`bboard %s``]", onename);

					else
						sprintf (buf + strlen (buf), " [`benter %s``]", onename);
				}

				else
				{
					if (obj->item_type == ITEM_CTRANSPORT &&
						(IS_SET (obj->value[4], TACT_SAIL)
						|| IS_SET (obj->value[4], TACT_PADDLE)
						|| IS_SET (obj->value[4], TACT_ROW)))
						sprintf (buf + strlen (buf), "Board %s - %s",
						onename,
						(room_is_dark (endloc) && (ch->race != PC_RACE_KALIAN) && (ch->race != PC_RACE_LICH)) ?
						"Too dark to tell" : endloc->name);

					else
						sprintf (buf + strlen (buf), "Enter %s - %s",
						onename,
						(room_is_dark (endloc) && (ch->race != PC_RACE_KALIAN) && (ch->race != PC_RACE_LICH)) ?
						"Too dark to tell" : endloc->name);
				}
				found = TRUE;
			}
			if (obj->item_type == ITEM_PORTAL
				&& IS_SET (obj->value[4], PACT_EXIT))
			{
				if (fAuto)
					sprintf (buf + strlen (buf), " [`bexit %s``]", onename);

				else
					sprintf (buf + strlen (buf), "Exit %s - %s", onename,
					(room_is_dark (endloc) && (ch->race != PC_RACE_KALIAN) && (ch->race != PC_RACE_LICH)) ? "Too dark to tell" :
					endloc->name);
				found = TRUE;
			}
			if (obj->item_type == ITEM_PORTAL
				&& IS_SET (obj->value[4], PACT_JUMP))
			{
				if (fAuto)
					sprintf (buf + strlen (buf), " [`bjump %s``]", onename);

				else
					sprintf (buf + strlen (buf), "Jump %s - %s", onename,
					(room_is_dark (endloc) && (ch->race != PC_RACE_KALIAN) && (ch->race != PC_RACE_LICH)) ? "Too dark to tell" :
					endloc->name);
				found = TRUE;
			}
			if (obj->item_type == ITEM_PORTAL
				&& IS_SET (obj->value[4], PACT_CLIMB))
			{
				if (fAuto)
					sprintf (buf + strlen (buf), " [`bclimb %s``]", onename);

				else
					sprintf (buf + strlen (buf), "Climb %s - %s", onename,
					(room_is_dark (endloc) && (ch->race != PC_RACE_KALIAN) && (ch->race != PC_RACE_LICH)) ? "Too dark to tell" :
					endloc->name);
				found = TRUE;
			}
			if (obj->item_type == ITEM_PORTAL
				&& IS_SET (obj->value[4], PACT_PLAY))
			{
				if (fAuto)
					sprintf (buf + strlen (buf), " [`bplay %s``]", onename);

				else
					sprintf (buf + strlen (buf), "Play %s - %s", onename,
					(room_is_dark (endloc) && (ch->race != PC_RACE_KALIAN) && (ch->race != PC_RACE_LICH)) ? "Too dark to tell" :
					endloc->name);
				found = TRUE;
			}
			if (obj->item_type == ITEM_PORTAL
				&& IS_SET (obj->value[4], PACT_DESCEND))
			{
				if (fAuto)
					sprintf (buf + strlen (buf), " [`bdescend %s``]", onename);

				else
					sprintf (buf + strlen (buf), "Descend %s - %s",
					onename,
					(room_is_dark (endloc) && (ch->race != PC_RACE_KALIAN) && (ch->race != PC_RACE_LICH)) ? "Too dark to tell" :
					endloc->name);
				found = TRUE;
			}
			if (obj->item_type == ITEM_PORTAL
				&& IS_SET (obj->value[4], PACT_CRAWL))
			{
				if (fAuto)
					sprintf (buf + strlen (buf), " [`bcrawl %s``]", onename);

				else
					sprintf (buf + strlen (buf), "Crawl %s - %s", onename,
					(room_is_dark (endloc) && (ch->race != PC_RACE_KALIAN) && (ch->race != PC_RACE_LICH)) ? "Too dark to tell" :
					endloc->name);
				found = TRUE;
			}
			if (obj->item_type == ITEM_PORTAL
				&& IS_SET (obj->value[4], PACT_SCALE))
			{
				if (fAuto)
					sprintf (buf + strlen (buf), " [`bscale %s``]", onename);

				else
					sprintf (buf + strlen (buf), "Scale %s - %s", onename,
					(room_is_dark (endloc) && (ch->race != PC_RACE_KALIAN) && (ch->race != PC_RACE_LICH)) ? "Too dark to tell" :
					endloc->name);
				found = TRUE;
			}
			if (!fAuto)
			{
				if (IS_IMMORTAL (ch))
					sprintf (buf + strlen (buf), " (room %d)\n\r", room_vnum);

				else
					sprintf (buf + strlen (buf), "\n\r");
			}
		}
		if (!found && !fAuto)
			strcat (buf, "`bNone``.\n\r");
		send_to_char (buf, ch);
		if (fAuto)
			send_to_char ("\n\r", ch);
	}

	else if (fAuto)
		send_to_char
		("You are too disoriented to notice where the exits are.\n\r", ch);
	return;
}

void do_worth (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	if (IS_NPC (ch))
	{
		sprintf (buf, "You have %ld gold and %ld silver.\n\r", ch->gold,
			ch->silver);
		send_to_char (buf, ch);
		return;
	}
	sprintf (buf,
		"You have %ld gold, %ld silver, and %lu experience (%ld exp to level).\n\r",
		ch->gold, ch->silver, ch->exp, level_cost (ch->level));
	send_to_char (buf, ch);
	return;
}

void do_score (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH], bigbuf[MAX_STRING_LENGTH], *cptr;
	int tcnt = 0;
	send_to_char
		("`b+-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+``\n\r",
		ch);
	send_to_char
		("`b|`d                  _____________________________________                  `b|``\n\r",
		ch);
	send_to_char
		("`b|`d         ________|                                     |_______          `b|``\n\r",
		ch);
	sprintf (buf,
		"`b|`d         \\       |" "               `o%-13s  "
		"       `d|      /          `b|``\n\r", ch->name);
	send_to_char (buf, ch);
	if ((ch->clan != CLAN_BOGUS) && is_full_clan (ch))
		sprintf (buf,
		"`b|`d          \\      |           `oof Clan %-16s``  `d|     /           `b|``\n\r",
		get_clan_name_ch (ch));

	else
		strcpy (buf,
		"`b|`d          \\      |                                     |     /           `b|``\n\r");
	send_to_char (buf, ch);
	send_to_char
		("`b|`d          /      |_____________________________________|     \\           `b|``\n\r",
		ch);
	send_to_char
		("`b|`d         /__________)                               (_________\\          `b|``\n\r",
		ch);
	if (!IS_NPC (ch))
		for (cptr = ch->pcdata->title; *cptr != '\0'; cptr++)
		{
			if (*cptr == '`')
			{
				tcnt++;
				cptr++;
			}
			if (*cptr == '\0')
				break;
		}
		sprintf (buf, "`b|`a  Title: `g%%-%ds `b|``\n\r", 63 + (tcnt * 2));
		sprintf (bigbuf, buf, IS_NPC (ch) ? "" : ch->pcdata->title);
		send_to_char (bigbuf, ch);

		sprintf (buf, "`b|`a  Level: `g%-8d", ch->level);
		send_to_char (buf, ch);
		if (get_trust (ch) != ch->level)
		{
			sprintf (buf, "  `aTrust: `g%-2d", get_trust (ch));
			send_to_char (buf, ch);
		}
		sprintf (buf, "  `aAge: `g%-3d  `aHours: `g%-5d", get_age (ch),
			(ch->played + (int) (current_time - ch->logon)) / 3600);
		send_to_char (buf, ch);
		if (!IS_NPC (ch))
		{
			if (ch->race == PC_RACE_SWARM)
			{
				sprintf(buf,"`aHand: `g%-12s", "none");
			}
			else switch (ch->pcdata->primary_hand)
			{
	default:
	case HAND_RIGHT:
		sprintf (buf, "`aHand: `g%-12s", "Right");
		break;
	case HAND_LEFT:
		sprintf (buf, "`aHand: `g%-12s", "Left");
		break;
	case HAND_AMBIDEXTROUS:
		sprintf (buf, "`aHand: `g%-12s", "Ambidextrous");
		break;
			}
			send_to_char (buf, ch);
		}

		else
		{
			sprintf (buf, "%19s", " ");
			send_to_char (buf, ch);
		}
		if (get_trust (ch) == ch->level)
			sprintf (buf, "%14s`b|``\n\r", "");

		else
			strcpy (buf, " `b|``\n\r");
		send_to_char (buf, ch);
		sprintf (buf,
			"`b|  `aRace: `g%-7s `aGender: `g%-7s  `aClass: `g%-8s`a/ `g%-8s`a/ `g%-8s     `b|``\n\r",
			race_table[ch->race].name,
			ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
			IS_NPC (ch) ? "mobile" : Class_table[ch->Class].name,
			IS_NPC (ch) ? "mobile" : ((ch->level > 30) ?
			Class_table[ch->
			Class2].name :
		"???"),
			IS_NPC (ch) ? "mobile" : ((ch->level > 60) ?
			Class_table[ch->Class3].name : "???"));
		send_to_char (buf, ch);
		sprintf (buf,
			"`b|  `aExperience: `g%-10lu  `aTo Level: `g%-9ld  `aTrains: `g%-4d              `b|``\n\r",
			ch->exp, IS_NPC (ch) ? 0 : level_cost (ch->level), ch->train);
		send_to_char (buf, ch);
		sprintf (buf,
			"`b|  `aHP: `g%5d/%-5d `aMana: `g%5d/%-5d `aMove: `g%5d/%-5d `aWimpy: `g%-5d       `b|``\n\r",
			ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
			ch->max_move, ch->wimpy);
		send_to_char (buf, ch);
		sprintf (buf,
			"`b|      -=[ `oAttributes `b]=-                       -=[ `oArmor `b]=-             |``\n\r");
		send_to_char (buf, ch);
		sprintf (buf,
			"`b|    `aStr: `g%02d(%02d)   `aDex: `g%02d(%02d)             `aPierce: `g%-4d   `aExotic: `g%-4d    `b|``\n\r",
			ch->perm_stat[STAT_STR], get_curr_stat (ch, STAT_STR),
			ch->perm_stat[STAT_DEX], get_curr_stat (ch, STAT_DEX),
			GET_AC (ch, AC_PIERCE), GET_AC (ch, AC_EXOTIC));
		send_to_char (buf, ch);
		sprintf (buf,
			"`b|    `aInt: `g%02d(%02d)   `aCon: `g%02d(%02d)    `aSave: `g%-3d  `aBash: `g%-4d    `aSlash: `g%-4d    `b|``\n\r",
			ch->perm_stat[STAT_INT], get_curr_stat (ch, STAT_INT),
			ch->perm_stat[STAT_CON], get_curr_stat (ch, STAT_CON),
			ch->saving_throw, GET_AC (ch, AC_BASH), GET_AC (ch, AC_SLASH));
		send_to_char (buf, ch);
		sprintf (buf,
			"`b|    `aWis: `g%02d(%02d)   `aCha: `g%02d(%02d)            `aHitroll: `g%-3d   `aDamroll: `g%-3d     `b|``\n\r",
			ch->perm_stat[STAT_WIS], get_curr_stat (ch, STAT_WIS),
			ch->perm_stat[STAT_CHA], get_curr_stat (ch, STAT_CHA),
			get_hitroll (ch), get_damroll (ch));
		send_to_char (buf, ch);
		sprintf (buf,
			"`b|  `aItems: `g%4d/%-4d    `aWeight: `g%7ld/%-7d                            `b|``\n\r",
			ch->carry_number, can_carry_n (ch),
			get_carry_weight (ch) / 10, can_carry_w (ch) / 10);
		send_to_char (buf, ch);
		sprintf (buf, "`b|  `aGold: `g%-8ld `aSilver: `g%-8ld ", ch->gold,
			ch->silver);
		strcpy (bigbuf, buf);
		if (ch->level >= 10)
		{
			sprintf (buf, "`aAlignment: `g%5d.  ", ch->alignment);
			strcat (bigbuf, buf);
		}

		else
			strcat (bigbuf, "                   ");
		strcat (bigbuf, "`gYou are ");

		// Akamai 6/4/98 -- Remove magic constants, see merc.h
		if (ALIGN_IS_ANGELIC (ch))
			strcat (bigbuf, "angelic.");

		else if (ALIGN_IS_SAINTLY (ch))
			strcat (bigbuf, "saintly.");

		else if (ALIGN_IS_GOOD (ch))
			strcat (bigbuf, "good.   ");

		else if (ALIGN_IS_KIND (ch))
			strcat (bigbuf, "kind.   ");

		else if (ALIGN_IS_NEUTRAL (ch))
			strcat (bigbuf, "neutral.");

		else if (ALIGN_IS_MEAN (ch))
			strcat (bigbuf, "mean.   ");

		else if (ALIGN_IS_EVIL (ch))
			strcat (bigbuf, "evil.   ");

		else if (ALIGN_IS_DEMONIC (ch))
			strcat (bigbuf, "demonic.");

		else
			strcat (bigbuf, "satanic.");
		sprintf (buf, "%s    `b|``\n\r", bigbuf);
		send_to_char (buf, ch);
		if (!IS_NPC(ch))
		{
			sprintf (buf, "`b|  `aQuest Points: `g%-7ld `aRP Points: `g%-7ld `aNewbie Helper Points: `g%-7ld `b|\n\r", ch->pcdata->qpoints, ch->pcdata->rppoints, ch->pcdata->nhpoints);
			send_to_char (buf, ch);
			sprintf (buf, "`b|  `aFind Quests Completed `g%5ld/%-5ld `aHunt Quests Completed`g%5ld/%-5ld     `b|\n\r",
				ch->pcdata->total_quest_finds,ch->pcdata->total_qf_attempted,
				ch->pcdata->total_quest_hunts,ch->pcdata->total_qh_attempted);
			send_to_char (buf,ch);
		}
		if (IS_IMMORTAL (ch))
		{
			strcpy (bigbuf, "`b|`a  Holy Light: ");
			if (IS_SET (ch->act, PLR_HOLYLIGHT))
				sprintf (buf, "%-5s", "`gon");

			else
				sprintf (buf, "%-5s", "`goff");
			strcat (bigbuf, buf);
			if (ch->invis_level)
			{
				sprintf (buf, "  `aInvisible: `glevel %-8d", ch->invis_level);
				strcat (bigbuf, buf);
			}

			else
			{
				sprintf (buf, "%27s", " ");
				strcat (bigbuf, buf);
			}
			if (ch->incog_level)
			{
				sprintf (buf, "  `aIncognito: `glevel %-8d", ch->incog_level);
				strcat (bigbuf, buf);
			}

			else
			{
				sprintf (buf, "%27s", " ");
				strcat (bigbuf, buf);
			}
			sprintf (buf, "%s  `b|``\n\r", bigbuf);
			send_to_char (buf, ch);
		}
		strcpy (bigbuf, "`b|``  `a");
		if (!IS_NPC (ch))
		{
			if (ch->pcdata->condition[COND_DRUNK] > 10)
				strcat (bigbuf, "You are drunk.  ");
			if (ch->pcdata->condition[COND_THIRST] <= 0)
				strcat (bigbuf, "You are thirsty.  ");
			if (ch->pcdata->condition[COND_HUNGER] <= 0)
				strcat (bigbuf, "You are hungry.  ");
		}
		if (ch->position == POS_STANDING && ch->fighting != NULL)
			ch->position = POS_FIGHTING;
		switch (ch->position)
		{
		case POS_DEAD:
			strcat (bigbuf, "You are DEAD!!");
			break;
		case POS_MORTAL:
			strcat (bigbuf, "You are mortally wounded.");
			break;
		case POS_INCAP:
			strcat (bigbuf, "You are incapacitated.");
			break;
		case POS_STUNNED:
			strcat (bigbuf, "You are stunned.");
			break;
		case POS_COMA:
		case POS_SLEEPING:
			strcat (bigbuf, "You are sleeping.");
			break;
		case POS_RESTING:
			strcat (bigbuf, "You are resting.");
			break;
		case POS_SITTING:
			strcat (bigbuf, "You are sitting.");
			break;
		case POS_STANDING:
			strcat (bigbuf, "You are standing.");
			break;
		case POS_FIGHTING:
			strcat (bigbuf, "You are fighting.");
			break;
		case POS_MOUNTED:
			strcat (bigbuf, "You are riding a mount.");
			break;
		}
		sprintf (buf, "%-80s`b|``\n\r", bigbuf);
		send_to_char (buf, ch);
		if (!IS_NPC(ch) && ch->pcdata->personality)
		{  
			strcpy (bigbuf, "`b|``  `a");
			switch (ch->pcdata->personality)
			{
			case PERS_AGGRESSIVE: strcat (bigbuf,"You are feeling overly aggressive.");break;
			case PERS_PASSIVE: strcat (bigbuf,"You are feeling overly passive.");break;
			}
			sprintf (buf, "%-80s`b|``\n\r", bigbuf);
			send_to_char(buf,ch);
		}

		send_to_char
			("`b+-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+``\n\r",
			ch);
		sprintf (buf, "Area rooms visited: %d (of %d)\n\r", areacount (ch),
			arearooms (ch));
		send_to_char (buf, ch);
		sprintf (buf, "World rooms visited: %d\n\r", roomcount (ch));
		send_to_char (buf, ch);
		if (IS_SET (ch->comm, COMM_SHOW_AFFECTS))
		{
			do_affects (ch, "");
			if (IS_CLASS(ch,PC_CLASS_REAVER))
				do_sword(ch,"affects");
		}
}

char *ac_info (CHAR_DATA * ch, int ac_type)
{
	static char buf[MAX_STRING_LENGTH];

	sprintf (buf, "%d", GET_AC (ch, ac_type));
	return (buf);

}

void do_affects (CHAR_DATA * ch, char *argument)
{
	AFFECT_DATA *paf, *paf_last = NULL;
	char buf[MAX_STRING_LENGTH];
	bool affectie = FALSE;
	if (ch->affected != NULL)
	{
		affectie = TRUE;
		send_to_char ("You are affected by the following:\n\r", ch);
		for (paf = ch->affected; paf != NULL; paf = paf->next)
		{
			if (paf_last != NULL && paf->type == paf_last->type
				&& !paf_last->composition)
				if (ch->level >= 20)
					sprintf (buf, "                              ");
				else
					continue;
			/*if (paf_last != NULL && paf->type == paf_last->type && !paf_last->composition)
			{
			if (ch->level >= 20)
			sprintf (buf, "                              ");
			}

			else continue;*/
			//Iblis 10/01/04 - "Invisible" affects
			else if (paf->where == TO_IGNORE)
				continue;
			else
			{
				if (paf->composition == TRUE)
					sprintf (buf, "Song: %-24s", paf->comp_name);

				else
				{
					switch (skill_table[paf->type].type)
					{
					case SKILL_NORMAL:
						if (*skill_table[paf->type].pgsn != gsn_uppercut && 
							*skill_table[paf->type].pgsn != gsn_dehydration)
							sprintf (buf, "Skill: %-23s",
							skill_table[paf->type].name);

						else
							sprintf (buf, "Penalty: %-21s",
							skill_table[paf->type].name);
						break;
					case SKILL_CAST:
						sprintf (buf, "Spell: %-23s",
							skill_table[paf->type].name);
						break;
					case SKILL_PRAY:
						sprintf (buf, "Prayer: %-22s",
							skill_table[paf->type].name);
						break;
					case SKILL_CHANT:
						sprintf (buf, "Chant: %-23s",
							skill_table[paf->type].name);
						break;
					case SKILL_SING:
						sprintf (buf, "Song: %-24s", skill_table[paf->type].name);
						break;
					}
				}
			}
			send_to_char (buf, ch);
			if (ch->level >= 20)
			{
				if (paf->where == TO_SKILL)
				{
					if(paf->location == 0)
						sprintf(buf,": modifies all abilities by %d",paf->modifier);
					else sprintf(buf,": modifies %s by %d",
						skill_table[paf->location].name,paf->modifier);
					send_to_char (buf,ch);
					send_to_char ("% ",ch);
				}
				else if (paf->where == TO_ACT2)
				{
					sprintf(buf,": adds %s ", act2_bit_name (paf->bitvector));
					send_to_char(buf,ch);
				}
				else
				{
					sprintf (buf, ": modifies %s by %d ",
						affect_loc_name (paf->location), paf->modifier);
					send_to_char (buf, ch);
				}
				if (paf->duration == -1)
				{
					sprintf (buf, "permanently");
					send_to_char (buf, ch);
				}

				else if (paf->type != gsn_jurgnation && paf->type != gsn_jalknation
					&& paf->type != skill_lookup("beacon of the damned")
					&& paf->type != gsn_aquatitus
					&& paf->type != gsn_dehydration
					&& paf->type != gsn_death_spasms
					&& !(paf->type == gsn_fireshield && !IS_NPC(ch) && ch->race == PC_RACE_LITAN && ch->pcdata->flaming))
				{
					sprintf (buf, "for %d hours", paf->duration);
					send_to_char (buf, ch);
				}
			}
			send_to_char ("\n\r", ch);
			if (!paf->composition && str_cmp(paf->comp_name,""))
			{
				sprintf (buf, "                                Affect perpetrated by : %-15s\n\r", paf->comp_name);
				send_to_char(buf,ch);
			}
			paf_last = paf;
		}
	}
	if (!IS_NPC(ch))
	{
		if ((race_table[ch->race].aff & AFF_DETECT_MAGIC) && (ch->affected_by & AFF_DETECT_MAGIC))
		{
			if (!affectie)
			{
				affectie = TRUE;
				send_to_char ("You are affected by the following:\n\r", ch);
			}
			send_to_char("Racial Affect: detect magic   : modifies none by 0 permanently\n\r",ch);
		}
		if ((race_table[ch->race].aff & AFF_DETECT_HIDDEN) && (ch->affected_by & AFF_DETECT_HIDDEN))
		{
			if (!affectie)
			{
				affectie = TRUE;
				send_to_char ("You are affected by the following:\n\r", ch);
			}
			send_to_char("Racial Affect: detect hidden  : modifies none by 0 permanently\n\r",ch);
		}
		if ((race_table[ch->race].aff & AFF_INFRARED) && (ch->affected_by & AFF_INFRARED))
		{
			if (!affectie)
			{
				affectie = TRUE;
				send_to_char ("You are affected by the following:\n\r", ch);
			}
			send_to_char("Racial Affect: infravision    : modifies none by 0 permanently\n\r",ch);
		}
		if ((race_table[ch->race].aff & AFF_FLYING) && (ch->affected_by & AFF_FLYING))
		{
			if (!affectie)
			{
				affectie = TRUE;
				send_to_char ("You are affected by the following:\n\r", ch);
			}
			send_to_char("Racial Affect: fly            : modifies none by 0 permanently\n\r",ch);
		}
		if ((race_table[ch->race].aff & AFF_AQUA_BREATHE) && (ch->affected_by & AFF_AQUA_BREATHE))
		{
			if (!affectie)
			{
				affectie = TRUE;
				send_to_char ("You are affected by the following:\n\r", ch);
			}
			send_to_char("Racial Affect: aqua breathe   : modifies none by 0 permanently\n\r",ch);
		}
		if ((race_table[ch->race].aff & AFF_PASS_DOOR) && (ch->affected_by & AFF_PASS_DOOR))
		{ 
			if (!affectie)
			{
				affectie = TRUE;
				send_to_char ("You are affected by the following:\n\r", ch);
			}
			send_to_char("Racial Affect: pass door      : modifies none by 0 permanently\n\r",ch);
		}
		if ((race_table[ch->race].aff & AFF_DETECT_GOOD) && (ch->affected_by & AFF_DETECT_GOOD))
		{
			if (!affectie)
			{
				affectie = TRUE;
				send_to_char ("You are affected by the following:\n\r", ch);
			}
			send_to_char("Racial Affect: detect good    : modifies none by 0 permanently\n\r",ch);
		}


	}


	if (!affectie)
		send_to_char ("You are not affected by any magic.\n\r", ch);
	return;
}
char *const day_name[] =
{ "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
"the Great Gods", "the Sun"
};
char *const month_name[] =
{ "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
"the Grand Struggle", "the Spring", "Nature", "Futility",
"the Dragon", "the Sun",
"the Heat", "the Battle", "the Dark Shades", "the Shadows",
"the Long Shadows",
"the Ancient Darkness", "the Great Evil"
};

// Expanded do_time to include information about year
// for improved roleplay timeline.
// Morgan, June. 21. 2000
void do_time (CHAR_DATA * ch, char *argument)
{
	extern char str_boot_time[];
	char buf[MAX_STRING_LENGTH];
	char *suf;
	int day;
	day = time_info.day + 1;
	if (day > 4 && day < 20)
		suf = "th";

	else if (day % 10 == 1)
		suf = "st";

	else if (day % 10 == 2)
		suf = "nd";

	else if (day % 10 == 3)
		suf = "rd";

	else
		suf = "th";
	sprintf (buf, "It is %d o'clock %s on this the Day of %s.\n\r"
		"It is the %d%s day of the Month of %s, year 1%d.\n\r",
		(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
		time_info.hour >= 12 ? "pm" : "am", day_name[day % 7], day,
		suf, month_name[time_info.month], time_info.year);
	send_to_char (buf, ch);
	sprintf (buf, "Exodus started up at %s\rThe system time is %s\r",
		str_boot_time, (char *) ctime (&current_time));
	send_to_char (buf, ch);
	return;
}

void do_weather (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	static char *const sky_look[4] =
	{ "cloudless", "cloudy", "rainy", "lit by flashes of lightning"
	};
	if (!IS_OUTSIDE (ch))
	{
		send_to_char ("You can't see the weather indoors.\n\r", ch);
		return;
	}
	sprintf (buf, "The sky is %s and %s.\n\r", sky_look[weather_info.sky],
		weather_info.change >=
		0 ? "a warm southerly breeze blows" :
		"a cold northern gust blows");
	send_to_char (buf, ch);
	return;
}

void do_help (CHAR_DATA * ch, char *argument)
{
	HELP_DATA *pHelp;
	char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH], *cptr;
	if (argument[0] == '\0')
		argument = "help";

	/* this parts handles help a b so that it returns help 'a b' */
	argall[0] = '\0';
	while (argument[0] != '\0')
	{
		argument = one_argument (argument, argone);
		if (argall[0] != '\0')
			strcat (argall, " ");
		strcat (argall, argone);
	}
	for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
	{
		if (pHelp->level > get_trust (ch))
			continue;
		if (is_name (argall, pHelp->keyword))
		{
			if (pHelp->level >= 0 && str_cmp (argall, "imotd"))
			{
				send_to_char (pHelp->keyword, ch);
				send_to_char ("\n\r", ch);
			}

			/*
			* Strip leading '.' to allow initial blanks.
			*/
			if (pHelp->text[0] == '.')
				page_to_char (pHelp->text + 1, ch);

			else
				page_to_char (pHelp->text, ch);
			return;
		}
	}
	if ((cptr = read_current_help (argall, "online.hlp")) != NULL)
	{
		page_to_char (cptr, ch);
		return;
	}
	if ((cptr = read_current_help (argall, "clans.hlp")) != NULL)
	{
		page_to_char (cptr, ch);
		return;
	}
	send_to_char ("No help on that word.\n\r", ch);
	return;
}


/* whois command */
void do_whois (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH], *check, *this_clan;
	BUFFER *output;
	char buf[MAX_STRING_LENGTH];
	char cbuf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	bool found = FALSE;
	FAKE_DATA *fake_vict;
	char wizi[MAX_STRING_LENGTH];
	char incog[MAX_STRING_LENGTH];
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("You must provide a name.\n\r", ch);
		return;
	}
	output = new_buf ();
	for (d = descriptor_list; d != NULL; d = d->next)
	{
		CHAR_DATA *wch;
		char const *Class;
		char Class_buf[MAX_STRING_LENGTH];
		if ((d->connected != CON_PLAYING
			&& d->connected != CON_START_MULTICLASS
			&& d->connected != CON_FINISH_MULTICLASS)
			|| !can_see (ch, d->character))
			continue;

		//Iblis 6/08/03 - I'm sure ths is not needed, but I want to be 100% sure
		if (IS_NPC (d->character))
			continue;
		wch = (d->original != NULL) ? d->original : d->character;
		if (!can_see (ch, wch))
			continue;
		if (is_affected (wch, gsn_mask) && wch->mask != NULL)
			check = wch->mask;

		else
			check = wch->name;
		if (!str_prefix (arg, check))
		{
			found = TRUE;

			/* work out the printing */
			strcpy (Class_buf, Class_table[wch->Class].who_name);
			strcat (Class_buf, "/");
			if (wch->level > 30)
				strcat (Class_buf, Class_table[wch->Class2].who_name);

			else
				strcat (Class_buf, "???");
			strcat (Class_buf, "/");
			if (wch->level > 60)
				strcat (Class_buf, Class_table[wch->Class3].who_name);

			else
				strcat (Class_buf, "???");
			Class = Class_buf;
			if (wch->level >= LEVEL_IMMORTAL)
			{
				if (wch->pcdata->whoinfo[0] != '\0')
				{
					strcpy (cbuf, center_text (wch->pcdata->whoinfo, 13));
					Class = cbuf;
				}

				else
					switch (wch->level)
				{
					default:
						Class = "UNKNOWN";
						break;
					case MAX_LEVEL:
						Class = "   overlord  ";
						break;
					case MAX_LEVEL - 1:
					case MAX_LEVEL - 2:
						Class = "    wizard   ";
						break;
					case MAX_LEVEL - 3:
					case MAX_LEVEL - 4:
					case MAX_LEVEL - 5:
						Class = "     god     ";
						break;
					case MAX_LEVEL - 6:
					case MAX_LEVEL - 7:
					case MAX_LEVEL - 8:
					case MAX_LEVEL - 9:
						Class = "    demigod  ";
						break;
				}
			}
			if (IS_SET (wch->comm, COMM_ANONYMOUS))
				if ((!IS_IMMORTAL (ch) && ch->level <= (wch->level + 10))
					|| (wch->race == 7 && !IS_IMMORTAL (ch)))
			Class = "  `aanonymous``  ";
			if (wch->race == PC_RACE_SWARM && !IS_IMMORTAL(wch))
			Class = "`dA Swarm of Flies";
			if (IS_NPC(d->character))
			{
				switch (d->character->pIndexData->vnum)
				{
				case MOB_VNUM_FROG: Class = "         `jFrog``         ";break;
				case MOB_VNUM_WARPED_CAT: Class = "      `kWarped Cat``      ";break;
				case MOB_VNUM_WARPED_OWL: Class = "      `eWarped Owl``      ";break;
				}
			}
			if (wch->clan != CLAN_BOGUS && is_full_clan (wch))
				this_clan = get_clan_symbol_ch (wch);

			else if (!IS_NPC (wch) && wch->pcdata->loner)
				this_clan = "`g[`hL`g]";

			else
				this_clan = "";
			if (wch->invis_level > 1)
				sprintf (wizi, "`l(Wizi %d)`` ", wch->invis_level);

			else
				sprintf (wizi, "``");
			if (wch->incog_level > 1)
				sprintf (incog, "`l(Incog %d)`` ", wch->incog_level);

			else
				sprintf (incog, "``");
			if (wch->level >= LEVEL_IMMORTAL ||
				(wch->race == PC_RACE_SWARM && !IS_IMMORTAL(wch)) ||
				(IS_NPC(d->character) && 
				(d->character->pIndexData->vnum >= MOB_VNUM_FROG && d->character->pIndexData->vnum <= MOB_VNUM_WARPED_OWL)) ||
				(IS_SET (wch->comm, COMM_ANONYMOUS)
				&& ch->level <= (wch->level + 10) && !IS_IMMORTAL (ch))
				|| (wch->race == 7 && IS_SET (wch->comm, COMM_ANONYMOUS)
				&& !IS_IMMORTAL (ch)))
				sprintf (buf, "`e(`h%s`e) ``%s`` %s%s%s%s%s%s%s%s%s%s\n\r",
			Class, this_clan,
				wch->incog_level >= 2 ? incog : "",
				wch->invis_level >= 2 ? wizi : "",
				IS_SET (wch->comm, COMM_AFK) ? "`k[AFK]`` " : "",
				IS_SET (wch->comm,
				COMM_QUIET) ? "`k[QUIET]`` " : "",
				IS_SET (wch->act,
				PLR_WANTED) ? "`b[Wanted]`` " : "",
				IS_SET (wch->act,
				PLR_JAILED) ? "`i[Jailed]`` " : "",
				wch->pcdata->pretitle[0] ==
				'\0' ? "" : wch->pcdata->pretitle,
				wch->pcdata->pretitle[0] == '\0' ? "" : " ",
				(is_affected (wch, gsn_mask) && wch->mask !=
				NULL) ? wch->mask : wch->name,
				IS_NPC (wch) ? "" : wch->pcdata->title);

			else
				sprintf (buf,
				"`e(`h%2d %6s %s`e) ``%s`` %s%s%s%s%s%s%s%s%s%s\n\r",
				wch->level,
				wch->race <
				MAX_PC_RACE ? pc_race_table[wch->race].
who_name : "     ", Class, this_clan,
		   wch->incog_level >= 2 ? incog : "",
		   wch->invis_level >= 2 ? wizi : "",
		   IS_SET (wch->comm, COMM_AFK) ? "`k[AFK]`` " : "",
		   IS_SET (wch->comm,
		   COMM_QUIET) ? "`k[QUIET]`` " : "",
		   IS_SET (wch->act,
		   PLR_WANTED) ? "`b[Wanted]`` " : "",
		   IS_SET (wch->act,
		   PLR_JAILED) ? "`i[Jailed]`` " : "",
		   wch->pcdata->pretitle[0] ==
		   '\0' ? "" : wch->pcdata->pretitle,
		   wch->pcdata->pretitle[0] == '\0' ? "" : " ",
		   (is_affected (wch, gsn_mask)
		   && wch->mask != NULL) ? wch->mask : wch->name,
		   IS_NPC (wch) ? "" : wch->pcdata->title);
			add_buf (output, buf);
		}
	}
	for (fake_vict = fake_list; fake_vict != NULL; fake_vict = fake_vict->next)
	{
		if (!str_prefix (arg, fake_vict->name))
		{
			sprintf (buf, "`e(`h%2d %6s %s`e)  ``%s %s\n\r",
				fake_vict->level,
				pc_race_table[fake_vict->race].who_name,
				Class_table[fake_vict->Class].who_name,
				fake_vict->name, fake_vict->title);
			add_buf (output, buf);
			found = TRUE;
		}
	}
	if (!found)
	{
		send_to_char ("No one of that name is playing.\n\r", ch);
		return;
	}
	page_to_char (buf_string (output), ch);
	free_buf (output);
}

int
display_fakes (short descriptor, int iLevelLower, int iLevelUpper,
			   int fImmortalOnly, int fClassRestrict, int fRaceRestrict,
			   int fClan, int fClanRestrict, bool rgfClass[MAX_CLASS],
			   bool rgfRace[MAX_PC_RACE], BUFFER * op)
{
	FAKE_DATA *fd;
	int nMatch = 0;
	char buf[MAX_STRING_LENGTH];
	for (fd = fake_list; fd; fd = fd->next)
	{
		if (fd->descriptor >= descriptor || fd->shown)
			continue;
		if (fd->level < iLevelLower
			|| fd->level > iLevelUpper
			|| (fImmortalOnly && fd->level < LEVEL_IMMORTAL)
			|| (fClassRestrict && !rgfClass[fd->Class])
			|| (fRaceRestrict && !rgfRace[fd->race])
			|| (fClan) || (fClanRestrict))
			continue;
		nMatch++;
		sprintf (buf, "`e(`h%2d `l%6s %s`e) ``%s`` %s%s%s%s%s%s %s``\n\r",
			fd->level, pc_race_table[fd->race].who_name,
			Class_table[fd->Class].who_name, "", "", "", "", "", "",
			fd->name, fd->title);
		fd->shown = TRUE;
		add_buf (op, buf);
	}
	return (nMatch);
}

void do_who (CHAR_DATA * ch, char *argument)
{
	char cbuf[MAX_STRING_LENGTH], *this_clan;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char wizi[MAX_STRING_LENGTH];
	char incog[MAX_STRING_LENGTH];
	BUFFER *output;
	DESCRIPTOR_DATA *d;
	int iClass;
	int iRace;
	int iClan;
	int iLevelLower;
	int iLevelUpper;
	int nNumber;
	int nMatch;
	bool rgfClass[MAX_CLASS];
	bool rgfRace[MAX_PC_RACE];
	bool rgfClan[MAX_CLAN];
	bool fClassRestrict = FALSE;
	bool fClanRestrict = FALSE;
	bool fClan = FALSE;
	bool fRaceRestrict = FALSE;
	bool fImmortalOnly = FALSE;
	FAKE_DATA *fake_ch;
	for (fake_ch = fake_list; fake_ch != NULL; fake_ch = fake_ch->next)
		fake_ch->shown = FALSE;

	/*
	* Set default arguments.
	*/
	iLevelLower = 0;
	iLevelUpper = MAX_LEVEL;
	for (iClass = 0; iClass < MAX_CLASS; iClass++)
		rgfClass[iClass] = FALSE;
	for (iRace = 0; iRace < MAX_PC_RACE; iRace++)
		rgfRace[iRace] = FALSE;
	for (iClan = 0; iClan < MAX_CLAN; iClan++)
		rgfClan[iClan] = FALSE;

	/*
	* Parse arguments.
	*/
	nNumber = 0;
	for (;;)
	{
		char arg[MAX_STRING_LENGTH];
		argument = one_argument (argument, arg);
		if (arg[0] == '\0')
			break;
		if (is_number (arg))
		{
			switch (++nNumber)
			{
			case 1:
				iLevelLower = atoi (arg);
				break;
			case 2:
				iLevelUpper = atoi (arg);
				break;
			default:
				send_to_char ("Only two level numbers allowed.\n\r", ch);
				return;
			}
		}

		else
		{

			/*
			* Look for Classes to turn on.
			*/
			if (!str_prefix (arg, "immortals"))
			{
				fImmortalOnly = TRUE;
			}

			else
			{
				iClass = Class_lookup (arg);
				if (iClass == -1)
				{
					iRace = race_lookup (arg);
					if (iRace == 0 || iRace >= MAX_PC_RACE)
					{
						if (!str_prefix (arg, "clan"))
							fClan = TRUE;

						else
						{
							iClan = clan_lookup (arg);
							if (iClan)
							{
								fClanRestrict = TRUE;
								rgfClan[iClan] = TRUE;
							}

							else
							{
								send_to_char
									("That's not a valid race, Class, or clan.\n\r",
									ch);
								return;
							}
						}
					}

					else
					{
						fRaceRestrict = TRUE;
						rgfRace[iRace] = TRUE;
					}
				}

				else
				{
					fClassRestrict = TRUE;
					rgfClass[iClass] = TRUE;
				}
			}
		}
	}

	/*
	* Now show matching chars.
	*/
	nMatch = 0;
	buf[0] = '\0';
	output = new_buf ();
	send_to_char
		("`a:`b----------------------------------------------------------------------------`a:``\n\r",
		ch);
	send_to_char
		("`d[`h\\`d][`h/`d][`h\\`d][`h/`d][`h\\`d][`h/`d][`h\\`d][`h/`d][`h\\`d]`o Players in this Realm`d [`h\\`d][`h/`d][`h\\`d][`h/`d][`h\\`d][`h/`d][`h\\`d][`h/`d][`h\\`d]``\n\r",
		ch);
	send_to_char
		("`a:`b============================================================================`a:``\n\r\n\r",
		ch);
	for (d = descriptor_list; d != NULL; d = d->next)
	{
		CHAR_DATA *wch;
		char *Class, Class_buf[80];

		/*
		* Check for match against restrictions.
		* Don't use trust as that exposes trusted mortals.
		*/
		if ((d->connected != CON_PLAYING
			&& d->connected != CON_START_MULTICLASS
			&& d->connected != CON_FINISH_MULTICLASS)
			|| !can_see (ch, d->character) 
			|| (IS_NPC (d->character) && d->character->pIndexData->vnum != MOB_VNUM_FROG
			&& d->character->pIndexData->vnum != MOB_VNUM_WARPED_CAT
			&& d->character->pIndexData->vnum != MOB_VNUM_WARPED_OWL))
			continue;
		wch = (d->original != NULL) ? d->original : d->character;
		if (!can_see (ch, wch))
			continue;

		nMatch +=
			display_fakes (d->descriptor, iLevelLower, iLevelUpper,
			fImmortalOnly, fClassRestrict, fRaceRestrict,
			fClan, fClanRestrict, rgfClass, rgfRace, output);
		if (wch->level < iLevelLower || wch->level > iLevelUpper
			|| (fImmortalOnly && wch->level < LEVEL_IMMORTAL)
			|| (fClassRestrict && !rgfClass[wch->Class]
		&& !rgfClass[wch->Class2] && !rgfClass[wch->Class3])
			|| (fRaceRestrict && !rgfRace[wch->race])
			|| (fClan && !is_clan (wch)) || (fClanRestrict
			&& !rgfClan[wch->clan]))
			continue;
		nMatch++;

		/*
		* Figure out what to print for Class.
		*/
		switch (wch->Class)
		{
		case 0:		/* Mage */
			strcpy (Class_buf, "`b");
			break;
		case 1:		/* Cleric */
			strcpy (Class_buf, "`n");
			break;
		case 2:		/* Thief */
			strcpy (Class_buf, "`a");
			break;
		case 3:		/* Warrior */
			strcpy (Class_buf, "`i");
			break;
		case 4:		/* Ranger */
			strcpy (Class_buf, "`c");
			break;
		case 5:		/* Bard */
			strcpy (Class_buf, "`m");
			break;
		case 6:		/* Paladin */
			strcpy (Class_buf, "`o");
			break;
		case 7:		/* Assassin */
			strcpy (Class_buf, "`a");
			break;
		case 8:		/* Reaver */
			strcpy (Class_buf, "`e");
			break;
		case 9:		/* Monk */
			strcpy (Class_buf, "`h");
			break;
		case 10:         /* Necromancer */
			strcpy (Class_buf, "`k");
			break;
		case 15:	// Druid
			strcpy (Class_buf, "`j");
			break;
		case PC_CLASS_SWARM:    /* Swarm */
			strcpy (Class_buf, "`D");
			break;
		case PC_CLASS_GOLEM:
			strcpy (Class_buf, "`d");
			break;
		case PC_CLASS_CHAOS_JESTER:
			strcpy (Class_buf, "`f");
			break;
		case PC_CLASS_ADVENTURER:
			strcpy (Class_buf, "`j");
			break;
		}
		if (wch->level > 60 && (wch->Class == wch->Class2) && (wch->Class2 == wch->Class3))
		{
			strcat (Class_buf, capitalize(Class_table[wch->Class].name));
		}
		else
		{
			strcat (Class_buf, Class_table[wch->Class].who_name);
			strcat (Class_buf, "`g/");

			/*
			* Figure out what to print for Class.
			*/
			switch (wch->Class2)
			{
			case 0:		/* Mage */
				strcat (Class_buf, "`b");
				break;
			case 1:		/* Cleric */
				strcat (Class_buf, "`n");
				break;
			case 2:		/* Thief */
				strcat (Class_buf, "`a");
				break;
			case 3:		/* Warrior */
				strcat (Class_buf, "`i");
				break;
			case 4:		/* Ranger */
				strcat (Class_buf, "`c");
				break;
			case 5:		/* Bard */
				strcat (Class_buf, "`m");
				break;
			case 6:		/* Paladin */
				strcat (Class_buf, "`o");
				break;
			case 7:		/* Assassin */
				strcat (Class_buf, "`a");
				break;
			case 8:		/* Reaver */
				strcat (Class_buf, "`e");
				break;
			case 9:		/* Monk */
				strcat (Class_buf, "`h");
				break;
			case 10:         /* Necromancer */
				strcat (Class_buf, "`k");
				break;
			case 15:	// Druid
				strcat (Class_buf, "`j");
				break;
			case PC_CLASS_SWARM:    /* Swarm */
				strcat (Class_buf, "`k");
				break;
			case PC_CLASS_GOLEM:
				strcat (Class_buf, "`d");
				break;
			case PC_CLASS_CHAOS_JESTER:
				strcat (Class_buf, "`f");
				break;
			case PC_CLASS_ADVENTURER:
				strcat (Class_buf, "`j");
				break;


			}
			if (wch->level > 30)
				strcat (Class_buf, Class_table[wch->Class2].who_name);

			else
				strcat (Class_buf, "???");
			strcat (Class_buf, "`g/");

			/*
			* Figure out what to print for Class.
			*/
			switch (wch->Class3)
			{
			case 0:		/* Mage */
				strcat (Class_buf, "`b");
				break;
			case 1:		/* Cleric */
				strcat (Class_buf, "`n");
				break;
			case 2:		/* Thief */
				strcat (Class_buf, "`a");
				break;
			case 3:		/* Warrior */
				strcat (Class_buf, "`i");
				break;
			case 4:		/* Ranger */
				strcat (Class_buf, "`c");
				break;
			case 5:		/* Bard */
				strcat (Class_buf, "`m");
				break;
			case 6:		/* Paladin */
				strcat (Class_buf, "`o");
				break;
			case 7:		/* Assassin */
				strcat (Class_buf, "`a");
				break;
			case 8:		/* Reaver */
				strcat (Class_buf, "`e");
				break;
			case 9:		/* Monk */
				strcat (Class_buf, "`h");
				break;
			case 10:         /* Necromancer */
				strcat (Class_buf, "`k");
				break;
			case 15:	// Druid
				strcat (Class_buf, "`j");
				break;
			case PC_CLASS_SWARM:    /* Swarm */
				strcat (Class_buf, "`k");
				break;
			case PC_CLASS_GOLEM:
				strcat (Class_buf, "`d");
				break;
			case PC_CLASS_CHAOS_JESTER:
				strcat (Class_buf, "`f");
				break;
			case PC_CLASS_ADVENTURER:
				strcat (Class_buf, "`j");
				break;


			}
			if (wch->level > 60)
				strcat (Class_buf, Class_table[wch->Class3].who_name);

			else
				strcat (Class_buf, "???");
		}

		strcat (Class_buf, "``");
		strcpy (Class_buf,center_text (Class_buf,11));
		Class = Class_buf;
		/* if (wch->race == PC_RACE_SWARM)
		strcpy(Class_buf,"`dA Swarm of Flies``");
		Class = Class_buf;*/

		if (wch->invis_level > 1)
			sprintf (wizi, "`l(Wizi %d)`` ", wch->invis_level);

		else
			sprintf (wizi, "``");
		if (wch->incog_level > 1)
			sprintf (incog, "`l(Incog %d)`` ", wch->incog_level);

		else
			sprintf (incog, "``");
		if (wch->level >= LEVEL_IMMORTAL)
		{
			if (wch->pcdata->whoinfo[0] != '\0')
			{
				strcpy (cbuf, center_text (wch->pcdata->whoinfo, 22));
				Class = cbuf;
			}

			else
				switch (wch->level)
			{
				default:
					Class = "UNKNOWN";
					break;
				case MAX_LEVEL:
					Class = "       overlord       ";
					break;
				case MAX_LEVEL - 1:
				case MAX_LEVEL - 2:
					Class = "        wizard        ";
					break;
				case MAX_LEVEL - 3:
				case MAX_LEVEL - 4:
				case MAX_LEVEL - 5:
					Class = "         god          ";
					break;
				case MAX_LEVEL - 6:
				case MAX_LEVEL - 7:
				case MAX_LEVEL - 8:
				case MAX_LEVEL - 9:
					Class = "        demigod       ";
					break;
			}
		}
		if (wch->pcdata->hero)
		{
			if (!IS_IMMORTAL (ch))
			Class = "         `eHero``         ";
		}
		if (IS_SET (wch->comm, COMM_ANONYMOUS))
		{
			if ((!IS_IMMORTAL (ch) && ch->level <= (wch->level + 10)) ||
				(wch->race == 7 && !IS_IMMORTAL (ch)))
			{
				Class = "      `aanonymous``       ";
			}
		}
		if (wch->race == PC_RACE_SWARM && !IS_IMMORTAL(wch))
		Class =  "   `dA Swarm of Flies``   ";
		if (IS_NPC(d->character))
		{
			switch (d->character->pIndexData->vnum)
			{                            
			case MOB_VNUM_FROG: Class = "         `jFrog``         ";break;
			case MOB_VNUM_WARPED_CAT: Class = "      `kWarped Cat``      ";break;
			case MOB_VNUM_WARPED_OWL: Class = "      `eWarped Owl``      ";break;
			}
		}

		if (wch->clan != CLAN_BOGUS && is_visible_clan (wch))
		{
			this_clan = get_clan_symbol_ch (wch);
		}

		else if (!IS_NPC (wch) && wch->pcdata->loner)
		{
			this_clan = "`g[`hL`g]";
		}

		else
		{
			this_clan = "";
		}
		if (!(battle_royale && (wch->level > 19 || (!IS_NPC(wch) && wch->pcdata->loner)))
			|| IS_IMMORTAL (wch) )
		{
			if ((wch->level >= LEVEL_IMMORTAL) ||
				((wch->pcdata->hero) && !IS_IMMORTAL (ch)) ||
				(IS_SET (wch->comm, COMM_ANONYMOUS) && !IS_IMMORTAL (ch)
				&& (ch->level <= (wch->level + 10)))
				|| ((wch->race == 7) && IS_SET (wch->comm, COMM_ANONYMOUS)
				&& !IS_IMMORTAL (ch))
				|| (IS_NPC(d->character) &&
				(d->character->pIndexData->vnum >= MOB_VNUM_FROG && d->character->pIndexData->vnum <= MOB_VNUM_WARPED_OWL)) 
				|| (wch->race == PC_RACE_SWARM && !IS_IMMORTAL(wch)))
			{
				if (!(wch->clan == CLAN_BOGUS && (!IS_NPC(wch) && !wch->pcdata->loner) ))
				{

					sprintf (buf, "`b(`h%s`b) ``%s%s`` %s%s%s%s%s%s%s%s%s``\n\r",
					Class, (wch->pcdata->elder?"`h[`bElder`h]``":""), this_clan,
						wch->incog_level >= 2 ? incog : "",
						wch->invis_level >= 2 ? wizi : "",
						IS_SET (wch->comm	, COMM_AFK) ? "`k[AFK]`` " : "",
						IS_SET (wch->act,	
						PLR_WANTED) ? "`b[Wanted]`` " : "",
						IS_SET (wch->act,
						PLR_JAILED) ? "`i[Jailed]`` " : "",
						wch->pcdata->pretitle[0] ==
						'\0' ? "" : wch->pcdata->pretitle,
						wch->pcdata->pretitle[0] == '\0' ? "" : " ",
						(is_affected (wch, gsn_mask) && wch->mask !=
						NULL) ? wch->mask : wch->name,
						IS_NPC (wch) ? "" : wch->pcdata->title);
				}
				else
				{
					sprintf (buf, "`b(`h%s`b) ``%s%s%s%s%s%s%s%s%s%s``\n\r",
					Class, (wch->pcdata->elder?"`h[`bElder`h]``":""),
						wch->incog_level >= 2 ? incog : "",
						wch->invis_level >= 2 ? wizi : "",
						IS_SET (wch->comm       , COMM_AFK) ? "`k[AFK]`` " : "",
						IS_SET (wch->act,
						PLR_WANTED) ? "`b[Wanted]`` " : "",
						IS_SET (wch->act,
						PLR_JAILED) ? "`i[Jailed]`` " : "",
						wch->pcdata->pretitle[0] ==
						'\0' ? "" : wch->pcdata->pretitle,
						wch->pcdata->pretitle[0] == '\0' ? "" : " ",
						(is_affected (wch, gsn_mask) && wch->mask !=
						NULL) ? wch->mask : wch->name,
						IS_NPC (wch) ? "" : wch->pcdata->title);
				}
			}

			else
			{
				char race2[20], race3[20];
				if (wch->race < MAX_PC_RACE)
				{
					strcpy(race2, pc_race_table[wch->race].who_name);
					strcpy(race3,center_text(race2,7));
				}
				else strcpy(race3, "       ");
				if (!(wch->clan == CLAN_BOGUS && (!IS_NPC(wch) && !wch->pcdata->loner) ))
				{
					sprintf (buf,
						"`b(`h%2d `g%7s %s`b) ``%s%s`` %s%s%s%s%s%s%s%s%s``\n\r",
						wch->level,
						race3, Class, (wch->pcdata->elder?"`h[`bElder`h]``":""), this_clan,
						wch->incog_level >= 2 ? incog : "",
						wch->invis_level >= 2 ? wizi : "",
						IS_SET (wch->comm, COMM_AFK) ? "`k[AFK]`` " : "",
						IS_SET (wch->act,
						PLR_WANTED) ? "`b[Wanted]`` " : "",
						IS_SET (wch->act,
						PLR_JAILED) ? "`i[Jailed]`` " : "",
						wch->pcdata->pretitle[0] ==
						'\0' ? "" : wch->pcdata->pretitle,
						wch->pcdata->pretitle[0] == '\0' ? "" : " ",
						(is_affected (wch, gsn_mask)
						&& wch->mask != NULL) ? wch->mask : wch->name,
						IS_NPC (wch) ? "" : wch->pcdata->title);
				}
				else 
				{
					sprintf (buf,
						"`b(`h%2d `g%7s %s`b)`` %s%s%s%s%s%s%s%s%s%s``\n\r",
						wch->level,
						race3, Class, (wch->pcdata->elder?"`h[`bElder`h]``":""), 
						wch->incog_level >= 2 ? incog : "",
						wch->invis_level >= 2 ? wizi : "",
						IS_SET (wch->comm, COMM_AFK) ? "`k[AFK]`` " : "",
						IS_SET (wch->act,
						PLR_WANTED) ? "`b[Wanted]`` " : "",
						IS_SET (wch->act,
						PLR_JAILED) ? "`i[Jailed]`` " : "",
						wch->pcdata->pretitle[0] ==
						'\0' ? "" : wch->pcdata->pretitle,
						wch->pcdata->pretitle[0] == '\0' ? "" : " ",
						(is_affected (wch, gsn_mask)
						&& wch->mask != NULL) ? wch->mask : wch->name,
						IS_NPC (wch) ? "" : wch->pcdata->title);
				}
			}
		}

		else
		{
			if ((wch->level >= LEVEL_IMMORTAL) ||
				((wch->pcdata->hero) && !IS_IMMORTAL (ch)) ||
				(IS_SET (wch->comm, COMM_ANONYMOUS) && !IS_IMMORTAL (ch)
				&& (ch->level <= (wch->level + 10)))
				|| (wch->race == PC_RACE_SWARM && !IS_IMMORTAL(wch))
				|| (IS_NPC(d->character) &&
				(d->character->pIndexData->vnum >= MOB_VNUM_FROG && d->character->pIndexData->vnum <= MOB_VNUM_WARPED_OWL)) 
				|| ((wch->race == 7)
				&& IS_SET (wch->comm, COMM_ANONYMOUS) && !IS_IMMORTAL (ch)))
			{
				if (!(wch->clan == CLAN_BOGUS && (!IS_NPC(wch) && !wch->pcdata->loner) ))
				{
					sprintf (buf,
						"`b(`h%s`b) `i%d ``%s`` %s%s%s%s%s%s%s%s%s``\n\r",
					Class, wch->pcdata->br_points, this_clan,
						wch->incog_level >= 2 ? incog : "",
						wch->invis_level >= 2 ? wizi : "",
						IS_SET (wch->comm, COMM_AFK) ? "`k[AFK]`` " : "",
						IS_SET (wch->act,
						PLR_WANTED) ? "`b[Wanted]`` " : "",
						IS_SET (wch->act,
						PLR_JAILED) ? "`i[Jailed]`` " : "",
						wch->pcdata->pretitle[0] ==
						'\0' ? "" : wch->pcdata->pretitle,
						wch->pcdata->pretitle[0] == '\0' ? "" : " ",
						(is_affected (wch, gsn_mask) && wch->mask !=
						NULL) ? wch->mask : wch->name,
						IS_NPC (wch) ? "" : wch->pcdata->title);
				}
				else
				{
					sprintf (buf,
						"`b(`h%s`b) `i%d`` %s%s%s%s%s%s%s%s%s``\n\r",
					Class, wch->pcdata->br_points,
						wch->incog_level >= 2 ? incog : "",
						wch->invis_level >= 2 ? wizi : "",
						IS_SET (wch->comm, COMM_AFK) ? "`k[AFK]`` " : "",
						IS_SET (wch->act,
						PLR_WANTED) ? "`b[Wanted]`` " : "",
						IS_SET (wch->act,
						PLR_JAILED) ? "`i[Jailed]`` " : "",
						wch->pcdata->pretitle[0] ==
						'\0' ? "" : wch->pcdata->pretitle,
						wch->pcdata->pretitle[0] == '\0' ? "" : " ",
						(is_affected (wch, gsn_mask) && wch->mask !=
						NULL) ? wch->mask : wch->name,
						IS_NPC (wch) ? "" : wch->pcdata->title);
				}
			}

			else
			{
				char race2[20], race3[20];
				if (wch->race < MAX_PC_RACE)
				{
					strcpy(race2, pc_race_table[wch->race].who_name);
					strcpy(race3, center_text(race2,7));
				}
				else strcpy(race3, "       ");
				if (!(wch->clan == CLAN_BOGUS && (!IS_NPC(wch) && !wch->pcdata->loner) ))
				{
					sprintf (buf,
						"`b(`h%2d `g%7s %s`b) `i%d ``%s`` %s%s%s%s%s%s%s%s%s``\n\r",
						wch->level,
						race3, Class, wch->pcdata->br_points,
						this_clan, wch->incog_level >= 2 ? incog : "",
						wch->invis_level >= 2 ? wizi : "",
						IS_SET (wch->comm, COMM_AFK) ? "`k[AFK]`` " : "",
						IS_SET (wch->act,
						PLR_WANTED) ? "`b[Wanted]`` " : "",
						IS_SET (wch->act,
						PLR_JAILED) ? "`i[Jailed]`` " : "",
						wch->pcdata->pretitle[0] ==
						'\0' ? "" : wch->pcdata->pretitle,
						wch->pcdata->pretitle[0] == '\0' ? "" : " ",
						(is_affected (wch, gsn_mask)
						&& wch->mask != NULL) ? wch->mask : wch->name,
						IS_NPC (wch) ? "" : wch->pcdata->title);
				}
				else
				{
					sprintf (buf,
						"`b(`h%2d `g%7s %s`b) `i%d`` %s%s%s%s%s%s%s%s%s``\n\r",
						wch->level,
						race3, Class, wch->pcdata->br_points,
						wch->incog_level >= 2 ? incog : "",
						wch->invis_level >= 2 ? wizi : "",
						IS_SET (wch->comm, COMM_AFK) ? "`k[AFK]`` " : "",
						IS_SET (wch->act,
						PLR_WANTED) ? "`b[Wanted]`` " : "",
						IS_SET (wch->act,
						PLR_JAILED) ? "`i[Jailed]`` " : "",
						wch->pcdata->pretitle[0] ==
						'\0' ? "" : wch->pcdata->pretitle,
						wch->pcdata->pretitle[0] == '\0' ? "" : " ",
						(is_affected (wch, gsn_mask)
						&& wch->mask != NULL) ? wch->mask : wch->name,
						IS_NPC (wch) ? "" : wch->pcdata->title);
				}
			}
		}
		add_buf (output, buf);
	}
	nMatch +=
		display_fakes (255, iLevelLower, iLevelUpper, fImmortalOnly,
		fClassRestrict, fRaceRestrict, fClan,
		fClanRestrict, rgfClass, rgfRace, output);
	sprintf (buf2, "\n\r``Players Found: %d``\n\r", nMatch);
	add_buf (output, buf2);
	if (strcmp (imm_who_msg, "remove"))
	{
		sprintf(buf2,"`a:`b============================================================================`a:``\r\n");

		add_buf(output,buf2);
		sprintf(buf2,center_text (imm_who_msg, 78));
		add_buf(output,buf2);
		sprintf(buf2,"\r\n`a:`b----------------------------------------------------------------------------`a:``\r\n");
		add_buf(output,buf2);
	}
	if (quad)
	{
		sprintf (buf, "Quadruple Experience is ON for %d more ticks!\n\r",
			quad_timer);
		add_buf(output,buf);
	}
	if (doub)
	{
		sprintf (buf, "Double Experience is ON for %d more ticks!\n\r",
			double_timer);
		add_buf(output,buf);
	}
	if (half)
	{
		sprintf (buf, "Half Experience is ON for %d more ticks!\n\r",
			half_timer);
		add_buf(output,buf);
	}
	if (battle_royale)
	{
		sprintf (buf, "`iBATTLE ROYALE IS ON FOR %d MORE TICKS!\n\r``",
			battle_royale_timer);
		add_buf(output,buf);
		if (br_leader != NULL)
		{
			sprintf (buf,
				"`bYour current Battle Royale leader is -> `g%s `bwith `g%d `bpoints.``\r\n",
				br_leader->name, br_leader->pcdata->br_points);
			add_buf(output,buf);
		}
	}
	page_to_char (buf_string (output), ch);
	free_buf (output);
	return;
}

void do_count (CHAR_DATA * ch, char *argument)
{
	int count;
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];

	count = 0;
	for (d = descriptor_list; d != NULL; d = d->next)
		if (d->connected == CON_PLAYING && can_see (ch, d->character))
			count++;

	max_on = UMAX (count, max_on);
	if (max_on == count)
		sprintf (buf,
		"There are currently `o%d`` denizens wandering the realms, the most so far today.\n\r",
		count);

	else
		sprintf (buf,
		"There are currently `o%d`` denizens wandering the realms, the most on today was `o%d``.\n\r",
		count, max_on);
	send_to_char (buf, ch);
}

void update_count ()
{
	DESCRIPTOR_DATA *d;
	int count = 0;
	for (d = descriptor_list; d != NULL; d = d->next)
		if (d->connected == CON_PLAYING)
			count++;
	max_on = UMAX (count, max_on);
}

void do_inventory (CHAR_DATA * ch, char *argument)
{
	send_to_char ("You are carrying:\n\r", ch);
	show_list_to_char (ch->carrying, ch, TRUE, TRUE);
	return;
}

void do_equipment (CHAR_DATA * ch, char *argument)
{
	OBJ_DATA *obj=NULL;
	long iWear=0;
	long wear_flags_left=0;
	int i=0,k=0,amount=0;
	char buf[MAX_STRING_LENGTH], wbuf[MAX_STRING_LENGTH];
	char *last_statement;

	send_to_char ("You are using:\n\r", ch);

	for (i = 1; wear_conversion_table[i].wear_loc != -2;++i)
		wear_flags_left |= wear_conversion_table[i].wear_flag;

	for (i = 1; wear_conversion_table[i].wear_loc != -2; ++i)
	{
		iWear = wear_conversion_table[i].wear_flag;
		if (iWear == ITEM_TAKE || iWear == ITEM_NO_SAC)
			continue;
		if (ch->race == PC_RACE_NERIX && 
			(iWear == ITEM_WEAR_SHIELD || iWear == ITEM_WEAR_HANDS
			|| iWear == ITEM_WIELD || iWear == ITEM_TWO_HANDS|| iWear == ITEM_WEAR_ARMS))
			continue;
		if (IS_CLASS(ch,PC_CLASS_MONK) && (iWear == ITEM_WIELD || iWear == ITEM_TWO_HANDS))
			continue;
		if (IS_CLASS(ch,PC_CLASS_ASSASSIN) && (iWear == ITEM_WEAR_SHIELD))
			continue;
		if (IS_CLASS(ch, PC_CLASS_DRUID) && (!iWear == ITEM_STAFF))
			continue;
		if (ch->race == PC_RACE_SWARM &&
			(iWear != ITEM_HOLD && iWear != ITEM_WEAR_FLOAT && iWear != ITEM_WEAR_BODY && iWear != ITEM_WEAR_LIGHT))
			continue;
		if (ch->race != PC_RACE_NERIX &&
			(iWear == ITEM_WORN_WINGS))
			continue;
		if (!IS_SET(wear_flags_left,iWear))
			continue;
		if ((obj = get_eq_char_new (ch, iWear)) == NULL)
		{
			//Iblis 10/26/04 - Hide the four new wear slots if nothing is worn there
			if (iWear == ITEM_WEAR_EARS || iWear == ITEM_WEAR_TATTOO 
				|| iWear == ITEM_WEAR_FACE || iWear == ITEM_WEAR_CREST || iWear == ITEM_WEAR_CLAN_MARK)

				continue;
			if (ch->race == PC_RACE_SWARM && iWear == ITEM_WEAR_BODY)
				send_to_char("<radiates>          ",ch);

			wear_flags_left -= iWear;
			sprintf(wbuf,"%-30s ",where_name[wear_conversion_table[i].wear_loc]);
			send_to_char(wbuf,ch);

			send_to_char ("<nothing>\n\r", ch);
			continue;
		}
		if (ch->race == PC_RACE_SWARM && iWear == WEAR_BODY)
			send_to_char("<radiates>          ",ch);
		//    for (k=0;wear_conversion_table[k].wear_loc != -2;++k)
		//  {
		//  if (wear_conversion_table[k].wear_flag == iWear)
		//  {
		wear_flags_left -= iWear;
		if (iWear == ITEM_WEAR_WRIST
			&& IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_WRIST))
		{
			sprintf(buf,"<%s","worn around both wrists");
			wear_flags_left -= ITEM_WEAR_ANOTHER_WRIST;
		}
		else if (iWear == ITEM_WEAR_FINGER
			&& IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_FINGER))
		{
			sprintf(buf,"<%s","worn on both fingers");
			wear_flags_left -= ITEM_WEAR_ANOTHER_FINGER;
		}
		else if (iWear == ITEM_WEAR_NECK
			&& IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_NECK))
		{
			sprintf(buf,"<%s","worn around entire neck");
			wear_flags_left -= ITEM_WEAR_ANOTHER_NECK;
		}
		else if (iWear == ITEM_WIELD
			&&  IS_SET(obj->wear_loc,ITEM_TWO_HANDS))
		{
			sprintf(buf,"<%s","wielded in both hands");
			wear_flags_left -= ITEM_TWO_HANDS;
		}
		else sprintf(buf,"<%s",wear_conversion_table[i].where_name);
		amount = 0;
		last_statement = NULL;
		for (k = i+1; wear_conversion_table[k].wear_loc != -2;++k)
		{
			//should never hit this
			if (!IS_SET(wear_flags_left,wear_conversion_table[k].wear_flag))
				continue;
			if (!IS_SET(obj->wear_loc,wear_conversion_table[k].wear_flag))
				continue;
			++amount;
			if (amount < 2)
			{
			}
			else
			{
				if (amount > 1)
					strcat(buf,", ");
				strcat(buf,last_statement);
			}
			wear_flags_left -= wear_conversion_table[k].wear_flag;
			if (wear_conversion_table[k].wear_flag == ITEM_WEAR_WRIST
				&& IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_WRIST))
			{
				last_statement = "both wrists";
				wear_flags_left -= ITEM_WEAR_ANOTHER_WRIST;
			}
			else if (wear_conversion_table[k].wear_flag == ITEM_WEAR_FINGER
				&& IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_FINGER))
			{
				last_statement = "both fingers";
				wear_flags_left -= ITEM_WEAR_ANOTHER_FINGER;
			}
			else if (wear_conversion_table[k].wear_flag == ITEM_WEAR_NECK
				&& IS_SET(obj->wear_flags,ITEM_WEAR_ANOTHER_NECK))
			{
				last_statement = "entire neck";
				wear_flags_left -= ITEM_WEAR_ANOTHER_NECK;
			}
			else if (wear_conversion_table[k].wear_flag == ITEM_WIELD
				&& IS_SET(obj->wear_loc,ITEM_TWO_HANDS))
			{
				last_statement = "as a weapon";
				wear_flags_left -= ITEM_TWO_HANDS;
			}
			else last_statement = wear_conversion_table[k].wear_name;
		}      


		if (amount > 1)
			strcat(buf,",");
		if (amount > 0)
			strcat(buf," and ");
		if (last_statement)
			strcat(buf,last_statement);
		strcat(buf,">");
		sprintf(wbuf,"%-30s ",buf);
		send_to_char(wbuf,ch);

		//          send_to_char(where_name[wear_conversion_table[i].wear_loc],ch);
		//   break;
		// }
		// }
		//      else send_to_char (where_name[iWear], ch);
		if (can_see_obj (ch, obj))
		{
			send_to_char (format_obj_to_char (obj, ch, TRUE), ch);
			send_to_char ("\n\r", ch);
		}

		else
		{
			send_to_char ("something.\n\r", ch);
		}

	}

	return;
}

void do_compare (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	int value1;
	int value2;
	char *msg;
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	if (arg1[0] == '\0')
	{
		send_to_char ("What would you like to compare?\n\r", ch);
		return;
	}
	if ((obj1 = get_obj_carry (ch, arg1)) == NULL)
	{
		send_to_char ("You do not have that item.\n\r", ch);
		return;
	}
	if (arg2[0] == '\0')
	{
		for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
		{
			if (obj2->wear_loc != WEAR_NONE && can_see_obj (ch, obj2)
				&& obj1->item_type == obj2->item_type
				&& (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0)
				break;
		}
		if (obj2 == NULL)
		{
			send_to_char ("You aren't wearing anything comparable.\n\r", ch);
			return;
		}
	}

	else if ((obj2 = get_obj_carry (ch, arg2)) == NULL)
	{
		send_to_char ("You do not have that item.\n\r", ch);
		return;
	}
	msg = NULL;
	value1 = 0;
	value2 = 0;
	if (obj1 == obj2)
	{
		msg =
			"You compare $p to itself.  Surprisingly, it looks about the same.";
	}

	else if (obj1->item_type != obj2->item_type)
	{
		msg = "You can't compare $p and $P.";
	}

	else
	{
		switch (obj1->item_type)
		{
		default:
			msg = "You can't compare $p and $P.";
			break;
		case ITEM_ARMOR:
			value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
			value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
			break;
		case ITEM_WEAPON:
			value1 = (1 + obj1->value[2]) * obj1->value[1];
			value2 = (1 + obj2->value[2]) * obj2->value[1];
			break;
		}
	}
	if (msg == NULL)
	{
		if (value1 == value2)
			msg = "$p and $P look about the same.";

		else if (value1 > value2)
			msg = "$p looks better than $P.";

		else
			msg = "$p looks worse than $P.";
	}
	act (msg, ch, obj1, obj2, TO_CHAR);
	return;
}

void do_credits (CHAR_DATA * ch, char *argument)
{
	do_help (ch, "credits");
	return;
}

void do_where (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char namebuf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;
	bool found;
	if (ch->race == PC_RACE_SWARM)
	{
		send_to_char("Huh?\n\r",ch);
		return;
	}  
	if (!str_cmp (ch->in_room->area->name, "Haven"))
	{
		send_to_char ("You are secluded in a haven.\n\r", ch);
		return;
	}
	if (is_affected(ch,skill_lookup("nearsight")))
	{
		send_to_char ("You can't tell where anyone is.\n\r",ch);
		return;
	}
	if (IS_SET (ch->in_room->room_flags, ROOM_TRANSPORT))
	{
		send_to_char
			("No other players are visible from your current vantage.\n\r", ch);
		return;
	}
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("`n[`lPlayers in your vicinity`n]``\n\r", ch);
		send_to_char ("`n~~~~~~~~~~~~~~~~~~~~~~~~~~``\n\r", ch);
		found = FALSE;
		if (IS_AFFECTED (ch, AFF_BLIND) && !IS_IMMORTAL (ch))
		{
			send_to_char ("None\n\r", ch);
			return;
		}
		for (d = descriptor_list; d; d = d->next)
		{
			if (d->connected == CON_PLAYING
				&& (victim = d->character) != NULL && !IS_NPC (victim)
				&& victim->in_room != NULL
				&& !IS_SET (victim->in_room->room_flags, ROOM_NOWHERE)
				&& (is_room_owner (ch, victim->in_room)
				|| !room_is_private (victim->in_room))
				&& victim->in_room->area == ch->in_room->area
				&& can_see (ch, victim)
				&& victim->race != PC_RACE_SWARM)
			{
				found = TRUE;
				if (IS_IMMORTAL (ch))
					sprintf (namebuf, "`i[`o%5d`i]`k %s",
					victim->in_room->vnum, victim->in_room->name);

				else
					sprintf (namebuf, "%s", victim->in_room->name);
				sprintf (buf, "`o%-28s`` `k%s``\n\r", victim->name, namebuf);
				send_to_char (buf, ch);
			}
		}
		if (!found)
			send_to_char ("None\n\r", ch);
	}

	else
	{
		found = FALSE;
		if (IS_AFFECTED (ch, AFF_BLIND) && !IS_IMMORTAL (ch))
		{
			act ("You didn't find any $T.", ch, NULL, arg, TO_CHAR);
			return;
		}
		for (victim = char_list; victim != NULL; victim = victim->next)
		{
			if (victim->in_room != NULL
				&& victim->in_room->area == ch->in_room->area
				&& !IS_AFFECTED (victim, AFF_HIDE)
				&& !IS_AFFECTED (victim, AFF_CAMOUFLAGE)
				&& !IS_AFFECTED (victim, AFF_SNEAK)
				&& can_see (ch, victim) && is_name (arg, victim->name))
			{
				found = TRUE;
				sprintf (buf, "%-28s %s\n\r", PERS (victim, ch),
					victim->in_room->name);
				send_to_char (buf, ch);
				break;
			}
		}
		if (!found)
			act ("You didn't find any $T.", ch, NULL, arg, TO_CHAR);
	}
	return;
}

void do_consider (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char *msg;
	int diff;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Consider killing whom?\n\r", ch);
		return;
	}
	if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They're not here.\n\r", ch);
		return;
	}
//Duiwel - Note to Self: Change this to have more diversity.
	diff = victim->level - ch->level;
	if (diff <= -10)
		msg = "`hLevel:           `oYou can kill $N naked and weaponless.``";

	else if (diff <= -5)
		msg = "`hLevel:           `j$N is no match for you.``";

	else if (diff <= -2)
		msg = "`hLevel:           `c$N looks like an easy kill.``";

	else if (diff <= 1)
		msg = "`hLevel:           `kThe perfect match!``";

	else if (diff <= 4)
		msg = "`hLevel:           `d$N says 'Do you feel lucky, punk?'.``";

	else if (diff <= 9)
		msg = "`hLevel:           `b$N laughs at you mercilessly.``";

	else
		msg = "`hLevel:           `iDeath will thank you for your gift.``";
	act (msg, ch, NULL, victim, TO_CHAR);
	diff = victim->hitroll - ch->hitroll;
	if (diff <= -35)
		msg = "`hAttacking Skill: `oYou far outClass $N``";

	else if (diff <= -25)
		msg =
		"`hAttacking Skill: `jYou have a much better chance of landing hits than $N.``";

	else if (diff <= -15)
		msg =
		"`hAttacking Skill: `cYour attacking skill seems superior to $N's.``";

	else if (diff <= -5 || diff == 0)
		msg = "`hAttacking Skill: `kIt's too close to tell.``";

	else if (diff <= 15)
		msg =
		"`hAttacking Skill: `d$N's attacking skill seems superior to yours.``";

	else if (diff <= 25)
		msg =
		"`hAttacking Skill: `b$N has a much better chance of landing hits than you.``";

	else
		msg = "`hAttacking Skill: `i$N far outClasses you.``";
	act (msg, ch, NULL, victim, TO_CHAR);
	diff =
		((victim->armor[0] + victim->armor[1] + victim->armor[2] +
		victim->armor[3]) / 4) - ((ch->armor[0] + ch->armor[1] +
		ch->armor[2] + ch->armor[3]) / 4);
	if (diff <= -100)
		msg = "`hArmor:           `i$N's armor is far superior to yours.``";

	else if (diff <= -50)
		msg = "`hArmor:           `b$N has a distinct advantage.``";

	else if (diff <= -25)
		msg = "`hArmor:           `d$N's armor is better.``.";

	else if (diff <= 5)
		msg = "`hArmor:           `kYou both seem about the same.``";

	else if (diff <= 25)
		msg = "`hArmor:           `cYour armor is better.``";

	else if (diff <= 50)
		msg = "`hArmor:           `jYou have a distinct advantage.``";

	else
		msg = "`hArmor:           `oYour armor is far superior.``";
	act (msg, ch, NULL, victim, TO_CHAR);
	diff = victim->hit - ch->hit;
	if (diff <= -800)
		msg = "`hHitpoints:       `oYou have a tremendous advantage.``";

	else if (diff <= -500)
		msg = "`hHitpoints:       `j$N has far fewer hitpoints than you.``";

	else if (diff <= -200)
		msg = "`hHitpoints:       `c$N shouldn't be a problem.``";

	else if (diff <= 25)
		msg = "`hHitpoints:       `kYou are pretty evenly matched.``";

	else if (diff <= 200)
		msg = "`hHitpoints:       `d$N may be difficult.``";

	else if (diff <= 500)
		msg = "`hHitpoints:       `b$N has far more hitpoints than you.``";

	else
		msg = "`hHitpoints:       `iYou are not even in the same league.``";
	act (msg, ch, NULL, victim, TO_CHAR);
	return;
}

void set_title (CHAR_DATA * ch, char *title)
{
	char buf[MAX_STRING_LENGTH];
	if (IS_NPC (ch))
	{
		bug ("Set_title: NPC.", 0);
		return;
	}
	if (title[0] != '.' && title[0] != ',' && title[0] != '!'
		&& title[0] != '?')
	{
		buf[0] = ' ';
		strcpy (buf + 1, title);
	}

	else
	{
		strcpy (buf, title);
	}
	smash_tilde (buf);
	free_string (ch->pcdata->title);
	ch->pcdata->title = str_dup (buf);
	return;
}

void do_title (CHAR_DATA * ch, char *argument)
{
	char *cptr;
	if (IS_NPC (ch))
		return;

	// Added for the NOTITLE wizcommand by Morgan
	// on June, 25. Year 2000
	if (IS_SET (ch->comm, COMM_NOTITLE))
	{
		send_to_char ("You cannot change your title at this time.\n\r", ch);
		return;
	}
	if (argument[0] == '\0')
	{
		send_to_char ("Change your title to what?\n\r", ch);
		return;
	}
	if (strlen (argument) > 45)
		argument[45] = '\0';
	if (color_strlen (argument) > 45)
	{
		int cnt = 0;
		cptr = argument;
		while (cnt < 45)
		{
			if (*cptr == '`')
				cptr++;

			else
				cnt++;
			cptr++;
		}
		*cptr = '\0';
	}
	for (cptr = argument; *cptr != '\0'; cptr++)
		if (*cptr == '`' && *(cptr + 1) == '\0')
		{
			*cptr = '.';
			break;
		}
		set_title (ch, argument);
		send_to_char ("Ok.\n\r", ch);
}

void do_report (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	if (IS_SET (ch->in_room->room_flags, ROOM_SILENT) && !IS_IMMORTAL (ch))
	{
		send_to_char
			("This room is shielded from disturbance by powerful wards of magic.\n\r",
			ch);
		return;
	}
	// Minax 6/30/02--When Vro'aths talk, they grunt and when Canthi talk,
	// they gurgle.
	if (ch->race == 4 && !IS_IMMORTAL (ch))
	{
		sprintf (buf,
			"You gurgle 'I have `F%d/%d `lhp `I%d/%d `lmana `J%d/%d `lmv `k%ld `jxp``.'\n\r",
			ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
			ch->max_move, ch->exp);
		send_to_char (buf, ch);
		sprintf (buf,
			"$n gurgles 'I have `F%d/%d hp `I%d/%d mana `J%d/%d mv `a%ld xp``.'",
			ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
			ch->max_move, ch->exp);
		act (buf, ch, NULL, NULL, TO_ROOM);
		return;
	}
	if (ch->race == 5 && !IS_IMMORTAL (ch))
	{
		sprintf (buf,
			"You grunt 'I have `F%d/%d `lhp `I%d/%d `lmana `J%d/%d `lmv `k%ld `jxp``.'\n\r",
			ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
			ch->max_move, ch->exp);
		send_to_char (buf, ch);
		sprintf (buf,
			"$n grunts 'I have `F%d/%d hp `I%d/%d mana `J%d/%d mv `a%ld xp``.'",
			ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
			ch->max_move, ch->exp);
		act (buf, ch, NULL, NULL, TO_ROOM);
		return;
	}
	// Minax 6/30-02 - If they are not a Canthi and not a Vro'ath or if they are
	// immortal, they 'say' instead of grunting-gurgling.
	if (ch->race == 7 && !IS_IMMORTAL (ch))
	{
		sprintf (buf,
			"You telepath to all 'I have `F%d/%d `lhp `I%d/%d `lmana `J%d/%d `lmv `k%ld `jxp``.'\n\r",
			ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
			ch->max_move, ch->exp);
		send_to_char (buf, ch);
		sprintf (buf,
			"$n telepaths to you 'I have `F%d/%d hp `I%d/%d mana `J%d/%d mv `a%ld xp``.'",
			ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
			ch->max_move, ch->exp);
		act (buf, ch, NULL, NULL, TO_ROOM);
		return;
	}
	if ((ch->race != 4 && ch->race != 5 && ch->race != 7) || IS_IMMORTAL (ch))
	{
		sprintf (buf,
			"You say 'I have `F%d/%d `lhp `I%d/%d `lmana `J%d/%d `lmv `k%ld `jxp``.'\n\r",
			ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
			ch->max_move, ch->exp);
		send_to_char (buf, ch);
		sprintf (buf,
			"$n says 'I have `F%d/%d hp `I%d/%d mana `J%d/%d mv `a%ld xp``.'",
			ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
			ch->max_move, ch->exp);
		act (buf, ch, NULL, NULL, TO_ROOM);
		return;
	}
	return;
}

void do_estimate (CHAR_DATA * ch, char *argument)
{
	int sn = 0, i = 0;
	long practicecost = 0;
	char buf[MAX_STRING_LENGTH];
	if (IS_NPC (ch))
		return;
	if (argument[0] == '\0')
	{
		send_to_char ("Estimate what?\n\r", ch);
	}

	else
	{
		sn = find_spell (ch, argument);
		if (sn < 0)
		{
			send_to_char ("You cannot estimate that.\r\n", ch);
			return;
		}
		if (ch->level < level_for_skill (ch, sn) || ch->pcdata->learned[sn] < 1)
		{
			send_to_char ("You cannot estimate that.\r\n", ch);
			return;
		}
		if (ch->pcdata->learned[sn] > 99)
		{
			send_to_char ("You are already learned at that skill.\r\n", ch);
			return;
		}

		else if (ch->pcdata->learned[sn] >= 75
			&& (sn == gsn_fletchery || sn == gsn_woodworking))
		{
			sprintf (buf,
				"You can only learn so much about %s from your guildmaster.\n\r",
				skill_table[sn].name);
			send_to_char (buf, ch);
			return;
		}
		else if (ch->pcdata->learned[sn] >= 60 
			&& (sn == gsn_milling))
		{
			send_to_char("You can onl learn so much about milling from your guildmaster.\n\r",ch);
			return;
		}
		else if (sn == gsn_lumberjacking)
		{
			send_to_char("Your guildmaster cannot assist you in learning this.\n\r",ch);
			return;
		}
		practicecost = 1000000 * rating_for_skill (ch, sn);
		if (practicecost > 0)
		{
			for (i = 19; i > ch->pcdata->learned[sn] / 5; i--)
				practicecost /= 1.8;
		}
		practicecost -=
			practicecost * .1 * ((get_curr_stat (ch, STAT_INT)) / 25.0);
		sprintf (buf, "It will cost %ld to practice %s again.\r\n",
			practicecost, argument);
		send_to_char (buf, ch);
		return;
	}
}
void do_practice (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int sn;
	int i;
	if (IS_NPC (ch))
		return;

	if (argument[0] == '\0')
	{
		int col;
		col = 0;
		for (sn = 0; sn < MAX_SKILL; sn++)
		{

			// exit loop when we get to the end of the table
			if (skill_table[sn].name == NULL)
				break;

			// if the level of the player is less than the level of this
			// skill -- or the player's learned percent less than 1
			if ((get_skill(ch,sn) < 1))
			{

				// only display if chars level is greater than 91
				if (ch->level < 92)
				{
					continue;
				}

			}
			// display the skill name and this players percentage
			sprintf (buf, "%-18s %3d%%  ", skill_table[sn].name,
				get_skill(ch,sn));
			send_to_char (buf, ch);
			if (++col % 3 == 0)
				send_to_char ("\n\r", ch);
		}			// end for loop
		if (col % 3 != 0)
			send_to_char ("\n\r", ch);
	}

	else
	{
		CHAR_DATA *mob;
		long practicecost;

		// yeah, char has to be awake
		if (!IS_AWAKE (ch))
		{
			send_to_char ("In your dreams, or what?\n\r", ch);
			return;
		}
		// find the mob that will allow skill training - gotta be in this room
		for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room)
		{
			if (IS_NPC (mob) && IS_SET (mob->act, ACT_PRACTICE))
				break;
		}
		if (mob == NULL)
		{
			send_to_char ("You can't do that here.\n\r", ch);
			return;
		}
		// better have some practices on them
		// IBLIS 5/18/03 - Changed to deal with new experience system
		sn = find_spell (ch, argument);
		if (sn > 0)
		{
			if (!IS_NPC (ch) &&
				((ch->level < level_for_skill (ch, sn) && level_for_skill(ch,sn) != 1) ||
				(ch->pcdata->learned[sn] < 1)))
			{
				if (ch->level < 92)
				{
					send_to_char ("You can't practice that.\n\r", ch);
					return;
				}
			}
		}

		else
		{
			send_to_char ("You can't practice that.\n\r", ch);
			return;
		}
		if (ch->pcdata->learned[sn] >= 100)
		{
			sprintf (buf, "You are already learned at %s.\n\r",
				skill_table[sn].name);
			send_to_char (buf, ch);
		}

		else if (ch->pcdata->learned[sn] >= 75
			&& (sn == gsn_fletchery || sn == gsn_woodworking))
		{
			sprintf (buf,
				"You can only learn so much about %s from your guildmaster.\n\r",
				skill_table[sn].name);
			send_to_char (buf, ch);
		}
		else if (ch->pcdata->learned[sn] >= 60
			&& (sn == gsn_milling))
		{
			send_to_char("You can onl learn so much about milling from your guildmaster.\n\r",ch);
			return;
		}

		else if (sn == gsn_lumberjacking)
		{
			send_to_char("Your guildmaster cannot assist you in learning this.\n\r",ch);
			return;
		}


		else
		{
			practicecost = 1000000 * rating_for_skill (ch, sn);
			if (practicecost > 0)
			{
				for (i = 19; i > ch->pcdata->learned[sn] / 5; i--)
					practicecost /= 1.8;
			}
			practicecost -=
				practicecost * .1 * ((get_curr_stat (ch, STAT_INT)) / 25.0);
			if (practicecost > 0 && ch->exp >= practicecost)
			{
				if (ch->pcdata->learned[sn] < 96)
				{
					ch->pcdata->learned[sn] += 5;
				}
				else
				{
					ch->pcdata->learned[sn] = 100;
				}
				ch->exp = ch->exp - practicecost;
			}

			else if (practicecost > 0)
			{
				sprintf (buf,
					"It costs %ld experience to practice that skill.\n\r",
					practicecost);
				send_to_char (buf, ch);
				return;
			}

			else
			{
				send_to_char ("You cannot practice that skill.\n\r", ch);
				return;
			}
			sprintf (buf, "%ld", practicecost);
			if (ch->pcdata->learned[sn] < 100)
			{
				act ("You practice $T at a cost of $t experience.", ch,
					buf, skill_table[sn].name, TO_CHAR);
				act ("$n practices $T.", ch, NULL, skill_table[sn].name,
					TO_ROOM);
			}

			else
			{
				ch->pcdata->learned[sn] = 100;	//redundancy check to make sure nothing is over 100
				act
					("You are now learned at $T, but it cost you $t experience.",
					ch, buf, skill_table[sn].name, TO_CHAR);
				act ("$n is now learned at $T.", ch, NULL, skill_table[sn].name,
					TO_ROOM);
			}
		}
	}
	return;
}


/*
* 'Wimpy' originally by Dionysos.
*/
void do_wimpy (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int wimpy;
	one_argument (argument, arg);
	if (arg[0] == '\0')
		wimpy = ch->max_hit / 5;

	else
		wimpy = atoi (arg);
	if (wimpy < 0)
	{
		send_to_char ("Your courage exceeds your wisdom.\n\r", ch);
		return;
	}
	if (wimpy > ch->max_hit / 2)
	{
		send_to_char ("Such cowardice ill becomes you.\n\r", ch);
		return;
	}
	ch->wimpy = wimpy;
	sprintf (buf, "Your wimpy has been set to %d hit points.\n\r", wimpy);
	send_to_char (buf, ch);
	return;
}

void do_password (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char *pArg;
	char *pwdnew;
	char *p;
	char cEnd;
	if (IS_NPC (ch))
		return;

	/*
	* Can't use one_argument here because it smashes case.
	* So we just steal all its code.  Bleagh.
	*/
	pArg = arg1;
	while (isspace (*argument))
		argument++;
	cEnd = ' ';
	if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;
	while (*argument != '\0')
	{
		if (*argument == cEnd)
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
	}
	*pArg = '\0';
	pArg = arg2;
	while (isspace (*argument))
		argument++;
	cEnd = ' ';
	if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;
	while (*argument != '\0')
	{
		if (*argument == cEnd)
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
	}
	*pArg = '\0';
	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char ("Syntax: password <old> <new>.\n\r", ch);
		return;
	}
	if (strcmp (crypt (arg1, ch->pcdata->pwd), ch->pcdata->pwd))
	{
		WAIT_STATE (ch, 40);
		send_to_char ("Wrong password.  Wait 10 seconds.\n\r", ch);
		return;
	}
	if (!good_password (arg2, ch->name))
	{
		send_to_char
			("That password is not acceptable.  Passwords must be at least 6 characters\n\rin length, must have at least one non-alphanumeric character in them, and\n\rmust not be easily guessable.\n\r",
			ch);
		return;
	}

	/*
	* No tilde allowed because of player file format.
	*/
	pwdnew = crypt (arg2, ch->name);
	for (p = pwdnew; *p != '\0'; p++)
	{
		if (*p == '~')
		{
			send_to_char ("New password not acceptable, try again.\n\r", ch);
			return;
		}
	}
	free_string (ch->pcdata->pwd);
	ch->pcdata->pwd = str_dup (pwdnew);
	save_char_obj (ch);
	send_to_char ("Ok.\n\r", ch);
	return;
}

void do_explored (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	argument = one_argument (argument, arg);
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("That player is not connected.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Mobiles do not have exploration.\n\r", ch);
		return;
	}
	sprintf (buf, "%s has explored %d rooms in the world.\n\r",
		victim->name, roomcount (victim));
	send_to_char (buf, ch);
	return;
}

void do_chpass (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char *pArg;
	char *pwdnew;
	char *p;
	char cEnd;
	argument = one_argument (argument, arg);
	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		send_to_char ("That player is not connected.\n\r", ch);
		return;
	}
	if (IS_NPC (victim))
	{
		send_to_char ("Mobiles do not have passwords.\n\r", ch);
		return;
	}

	/*
	* Can't use one_argument here because it smashes case.
	* So we just steal all its code.  Bleagh.
	*/
	pArg = arg1;
	while (isspace (*argument))
		argument++;
	cEnd = ' ';
	if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;
	while (*argument != '\0')
	{
		if (*argument == cEnd)
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
	}
	*pArg = '\0';
	if (arg1[0] == '\0')
	{
		send_to_char ("Syntax: chpass <player> <new>.\n\r", ch);
		return;
	}
	if (!good_password (arg1, victim->name))
	{
		send_to_char
			("That password is not acceptable.  Passwords must be at least 6 characters\n\rin length, must have at least one non-alphanumeric character in them, and\n\rmust not be easily guessable.\n\r",
			ch);
		return;
	}

	/*
	* No tilde allowed because of player file format.
	*/
	pwdnew = crypt (arg1, victim->name);
	for (p = pwdnew; *p != '\0'; p++)
	{
		if (*p == '~')
		{
			send_to_char ("New password not acceptable, try again.\n\r", ch);
			return;
		}
	}
	free_string (victim->pcdata->pwd);
	victim->pcdata->pwd = str_dup (pwdnew);
	save_char_obj (victim);
	send_to_char ("Password changed.\n\r", ch);
	return;
}

void do_email (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char *cptr;
	if IS_NPC
		(ch)
	{
		send_to_char ("Mobiles don't have email addresses.\n\r", ch);
		return;
	}
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Syntax: email <new email address>\n\r", ch);
		sprintf (buf, "Your present email address is:  %s\n\r",
			ch->pcdata->email_addr);
		send_to_char (buf, ch);
		return;
	}
	if (strlen (arg) < 6 || strchr (arg, '@') == NULL
		|| strchr (arg, '.') == NULL)
	{
		send_to_char ("That is an invalid email address.\n\r", ch);
		return;
	}
	free_string (ch->pcdata->email_addr);
	ch->pcdata->email_addr = str_dup (argument);
	if ((cptr = strchr (ch->pcdata->email_addr, '\n')) != NULL)
		*cptr = '\0';
	if ((cptr = strchr (ch->pcdata->email_addr, '\r')) != NULL)
		*cptr = '\0';
	save_char_obj (ch);
	send_to_char ("Email address updated.\n\r", ch);
	return;
}

void do_lore (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	bool found = FALSE;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("What would you like to learn more about?\n\r", ch);
		return;
	}
	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		if (can_see_obj (ch, obj))
			if (is_name (arg, obj->name))
			{
				found = TRUE;
				break;
			}
			if (!found)
			{
				if ((obj = get_obj_here (ch, arg)) != NULL)
					found = TRUE;
				if (!found)
				{
					send_to_char ("You do not see that here.\n\r", ch);
					return;
				}
			}
			if (IS_OBJ_STAT (obj, ITEM_NOIDENTIFY))
			{
				send_to_char
					("You cannot glean any more information about this object.\n\r", ch);
				return;
			}
			if (number_percent () < get_skill (ch, gsn_lore))
			{
				send_to_char
					("You remember stories of old told of this object...\n\r", ch);
				check_improve (ch, gsn_lore, TRUE, 4);

				spell_identify(gsn_lore,ch->level,ch,(void*) obj,0);
				return;
			}

			else
			{
				act
					("You've not wandered long or far enough to learn the secrets of $p.",
					ch, obj, NULL, TO_CHAR);
				check_improve (ch, gsn_lore, FALSE, 4);
			}
			return;
}


/********************************************************************************
* Stat List code copyright 1999-2001                                           *
* Markanth : markanth@spaceservices.net                                        *
* Devil's Lament : spaceservices.net port 3778                                 *
* Web Page : http://spaceservices.net/~markanth/                               *
*                                                                              *
* All I ask in return is that you give me credit on your mud somewhere         *
* or email me if you use it.                                                   *
********************************************************************************/
unsigned long exodus_score (CHAR_DATA * ch)
{
	int score;
	if (IS_NPC (ch))
		return 0;
	score =
		((ch->pcdata->has_killed[PLAYER_KILL] * 3) + (ch->pcdata->has_killed[MOB_KILL] / 10) + ((ch->played + (int) (current_time - ch->logon)) / 3600) +
		(roomcount (ch) / 10) + (ch->pcdata->totalxp / 1000)) -
		(((ch->pcdata->been_killed[PLAYER_KILL] + ch->pcdata->been_killed[MOB_KILL]) * 10) + (ch->pcdata->been_killed[PLAYER_KILL] * 5));
	return score;
}

void save_statlist (void)
{
	FILE *fp;
	bool found = FALSE;
	int i,j;
	fclose (fpReserve);
	if ((fp = fopen (STAT_FILE, "w")) == NULL)
	{
		perror (STAT_FILE);
	}
	found = TRUE;
	for (i=0;i < MAX_GAMESTAT;i++)
	{
		fprintf(fp, "Type%d\n",i);
		{
			for (j=0;j < MAX_PEOPLE_STAT;j++)
			{
				fprintf (fp, "%ld %s~\n",TGS_Stats[i][j],TGS_Names[i][j]);

			}
		}
	}


	fclose (fp);
	fpReserve = fopen (NULL_FILE, "r");
	if (!found)
		unlink (STAT_FILE);
}

void load_statlist (void)
{
	FILE *fp;
	int i=0,j=0;
	char buf[MAX_STRING_LENGTH], *temps;
	if ((fp = fopen (STAT_FILE, "r")) == NULL)
	{

		for (i=0;i < MAX_GAMESTAT;i++)
		{
			for (j=0;j < MAX_PEOPLE_STAT;j++)
			{
				TGS_Stats[i][j] = 0;
				TGS_Names[i][j] = str_dup("");
			}
		}
		return;
	}
	//Iblis 1/19/04 - While this may indeed be redundant,
	//its use comes into play when new gamestats are added
	//(IE the first time the stat file is opened it will work)
	for (i=0;i < MAX_GAMESTAT;i++)
	{
		for (j=0;j < MAX_PEOPLE_STAT;j++)
		{
			TGS_Stats[i][j] = 0;
			TGS_Names[i][j] = str_dup("");
		}
	}


	for (i=0;i < MAX_GAMESTAT;i++)
	{
		if (feof (fp))
		{

			fclose (fp);
			return;
		}
		sprintf(buf,"Type%d",i);
		temps = fread_word (fp);
		if (!str_cmp (temps, buf))
		{

			for (j=0;j < MAX_PEOPLE_STAT;j++)
			{
				TGS_Stats[i][j] = fread_number(fp);
				TGS_Names[i][j] = fread_string(fp);
			}
		}
		else 
		{ 
			fclose (fp);
			return;
		}
		if (i == STAT_FASTEST_TN)
		{
			for (j=0;j < MAX_PEOPLE_STAT;j++)
			{
				TGS_Stats[STAT_FASTEST_TN][j] = 0;
				TGS_Names[STAT_FASTEST_TN][j] = str_dup("");
			}
		}

	}
}

//Iblis - Completely recoded showstats so it doesn't lag like a mother fucker anymore.
//I also made it VERY easy to add additional stats to it. 
void do_showstats (CHAR_DATA * ch, char *argument)
{
	int option;
	char arg[MAX_INPUT_LENGTH];
	int stat=-1;
	argument = one_argument (argument, arg);
	if (IS_NPC(ch))
		return;
	if (arg[0] == '\0')
	{
		send_to_char ("      `iOPTIONS AVAILABLE:``\n\r", ch);
		send_to_char
			("      `k0`` - Game Stats on Exodus (`ogstats``)\n\r", ch);
		send_to_char
			("      `k1`` - Ranking of Player Killers (`opkills``)\n\r", ch);
		send_to_char
			("      `k2`` - Ranking of Player Deaths (`opdeaths``)\n\r", ch);
		send_to_char
			("      `k3`` - Ranking of Mob Kills (`omkills``)\n\r", ch);
		send_to_char
			("      `k4`` - Ranking of Mob Deaths (`omdeaths``)\n\r", ch);
		send_to_char
			("      `k5`` - Ranking of Rooms Explored (`oexplore``)\n\r", ch);
		send_to_char
			("      `k6`` - Ranking of Hours Played (`ohours``)\n\r", ch);
		send_to_char
			("      `k7`` - Ranking of Total Experience Points (`oexperience``)\n\r",
			ch);
		send_to_char
			("      `k8`` - Ranking of Combined Exodus Score (`oscore``)\n\r",
			ch);
		send_to_char("      `k9`` - Ranking of Super Duel Kills (`osdkills``)\n\r",ch);
		send_to_char("      `k10`` - Ranking of Super Duel Deaths (`osddeaths``)\n\r",ch);
		send_to_char("      `k11`` - Ranking of Super Duels Participated In (`osdparticipants``)\n\r",ch);
		send_to_char("      `k12`` - Ranking of Super Duel Wins (`osdwins``)\n\r",ch);
		send_to_char("      `k13`` - Ranking of Battle Royale Kills (`obrkills``)\n\r",ch);
		send_to_char("      `k14`` - Ranking of Battle Royale Deaths (`obrdeaths``)\n\r",ch);
		send_to_char("      `k15`` - Ranking of Battle Royales Participated In (`obrparticipants``)\n\r",ch);
		send_to_char("      `k16`` - Ranking of Battle Royale Wins (`obrwins``)\n\r",ch);
		send_to_char("      `k17`` - Ranking of Arena Kills (`oakills``)\n\r",ch);
		send_to_char("      `k18`` - Ranking of Arena Deaths (`oadeaths``)\n\r",ch);
		send_to_char("      `k19`` - Ranking of Total Souls (`ototalsouls``)\n\r",ch);
		//      send_to_char("      `k20`` - Ranking of Fastest To Level 90 (`ofastestto90``)\n\r",ch);
		send_to_char("      `k21`` - Ranking of Total Gambling Losses (`ogamblinglosses``)\n\r",ch);
		send_to_char("      `k22`` - Ranking of Total Gambling Wins (`ogamblingwins``)\n\r",ch);
		send_to_char("      `k23`` - Ranking of Best Avatars (`oavatars``)\n\r",ch);
		send_to_char("      `k24`` - Ranking of Total Find Quests Completed (`otfquestsc``)\n\r",ch);
		send_to_char("      `k25`` - Ranking of Total Find Quests Attempted (`otfquestsa``)\n\r",ch);
		send_to_char("      `k26`` - Ranking of Total Hunt Quests Completed (`othquestsc``)\n\r",ch);
		send_to_char("      `k27`` - Ranking of Total Hunt Quests Attempted (`othquestsa``)\n\r",ch);
		send_to_char("      `k28`` - Ranking of Suicides (`osuicides``)\n\r",ch);
		send_to_char("      `k29`` - Ranking of Total Trees Chopped (`ochopped``)\n\r",ch);
		send_to_char("      `k30`` - Ranking of Total Trees Milled (`omilled``)\n\r",ch);
		send_to_char("      `k31`` - Ranking of Total Objects Sold (`oobjssold``)\n\r",ch);
		send_to_char("      `k32`` - Ranking of Total Money Made (via stores) (`omoneymade``)\n\r",ch);
		send_to_char("      `k33`` - Number of Miverblis games won (`ocgwon``)\n\r",ch);
		send_to_char("      `k34`` - Number of Miverblis games lost (`oocglost``)\n\r",ch);
		if (IS_IMMORTAL (ch))
			send_to_char
			("      `kdelete <name>`` - deletes from statlist\n\r", ch);
		return;
	}
	option = atoi (arg);
	if (!str_cmp (arg, "delete") && IS_IMMORTAL (ch))
	{
		int i,j;
		bool started=FALSE;
		for (i = 0; i < MAX_GAMESTAT; i++)
		{
			started = FALSE;
			for (j = 0;j < MAX_PEOPLE_STAT; j++)
			{
				if (!str_cmp(TGS_Names[i][j],argument))
				{
					started = TRUE;
					free_string(TGS_Names[i][j]);

				}
				if (started)
				{
					if (j < MAX_PEOPLE_STAT-1)
					{
						TGS_Names[i][j] = TGS_Names[i][j+1];
						TGS_Stats[i][j] = TGS_Stats[i][j+1];
					}
					else
					{
						TGS_Names[i][j] = str_dup("");
						if (i == STAT_FASTEST_TN)
							TGS_Stats[i][j] = 9999;
						else TGS_Stats[i][j] = 0;
					}
				}


			}
		}

	}
	else if (option == 1 || !str_prefix (arg, "pkills"))
		stat = STAT_PK_KILLS;

	else if (option == 3 || !str_prefix (arg, "mkills"))
		stat = STAT_MOB_KILLS;

	else if (option == 2 || !str_prefix (arg, "pdeaths"))
		stat = STAT_PK_DEATHS;

	else if (option == 4 || !str_prefix (arg, "mdeaths"))
		stat = STAT_MOB_DEATHS;

	else if (option == 5 || !str_prefix (arg, "explore"))
		stat = STAT_ROOMS_EXPLORED;

	else if (option == 6 || !str_prefix (arg, "hours"))
		stat = STAT_HOURS_PLAYED;

	else if (option == 7 || !str_prefix (arg, "experience"))
		stat = STAT_TOTAL_EXP;
	else if (option == 8 || !str_prefix (arg, "score"))
		stat = STAT_EXODUS_SCORE;
	else if (option == 9 || !str_prefix (arg, "sdkills"))
		stat = STAT_SD_KILLS;
	else if (option == 10 || !str_prefix (arg, "sddeaths"))
		stat = STAT_SD_DEATHS;
	else if (option == 11 || !str_prefix (arg, "sdparticipants"))
		stat = STAT_SD_IN;
	else if (option == 12 || !str_prefix (arg, "sdwins"))
		stat = STAT_SD_WON;
	else if (option == 13 || !str_prefix (arg, "brkills"))
		stat = STAT_BR_KILLS;
	else if (option == 14 || !str_prefix (arg, "brdeaths"))
		stat = STAT_BR_DEATHS;
	else if (option == 15 || !str_prefix (arg, "brparticipants"))
		stat = STAT_BR_IN;
	else if (option == 16 || !str_prefix (arg, "brwins"))
		stat = STAT_BR_WON;
	else if (option == 17 || !str_prefix (arg, "akills"))
		stat = STAT_ARENA_KILLS;
	else if (option == 18 || !str_prefix (arg, "adeaths"))
		stat = STAT_ARENA_DEATHS;
	else if (option == 19 || !str_prefix (arg, "totalsouls"))
		stat = STAT_TOTAL_SOULS;
	else if (option == 20 || !str_prefix (arg, "fastestto90"))
		stat = STAT_FASTEST_TN;
	else if (option == 21 || !str_prefix (arg, "gamblinslosses"))
		stat = STAT_TG_LOSS;
	else if (option == 22 || !str_prefix (arg, "gamblingwins"))
		stat = STAT_TG_WON;
	else if (option == 23 || !str_prefix (arg, "avatars"))
		stat = STAT_AVATAR;
	else if (option == 24 || !str_prefix (arg, "tfquestsc"))
		stat = STAT_TQFindsC;
	else if (option == 25 || !str_prefix (arg, "tfquestsa"))
		stat = STAT_TQFindsA;
	else if (option == 26 || !str_prefix (arg, "thquestsc"))
		stat = STAT_TQHuntsC;
	else if (option == 27 || !str_prefix (arg, "thquestsa"))
		stat = STAT_TQHuntsA;
	else if (option == 28 || !str_prefix (arg, "suicides"))
		stat = STAT_EXTRA_DEATHS;
	else if (option == 29 || !str_prefix (arg, "chopped"))
		stat = STAT_TREES_CHOPPED;
	else if (option == 30 || !str_prefix (arg, "milled"))
		stat = STAT_TREES_MILLED;
	else if (option == 31 || !str_prefix (arg, "objssold"))
		stat = STAT_OBJS_SOLD;
	else if (option == 32 || !str_prefix (arg, "moneymade"))
		stat = STAT_MONEY_MADE;
	else if (option == 33 || !str_prefix (arg, "cgwon"))
		stat = STAT_CG_WON;
	else if (option == 34 || !str_prefix (arg, "cglost"))
		stat = STAT_CG_LOST;



	else
	{
		do_showstats (ch, "");
		return;
	}
	if (stat != -1 && stat < MAX_GAMESTAT)
	{
		int j;
		BUFFER *output;
		char buf[MAX_STRING_LENGTH];
		int count, pos;
		const char *stat_name[MAX_GAMESTAT] =
		{ "PLAYER KILLERS", "MOB KILLERS", "PK DEATHS", "MOB DEATHS",
		"ROOMS EXPLORED", "HOURS PLAYED", "TOTAL EXPERIENCE POINTS",
		"EXODUS SCORE", "SUPERDUEL KILLERS", "SUPERDUEL DEATHS", 
		"SUPERDUEL PARTICIPANTS", "SUPERDUEL VICTORS",
		"BATTLE ROYALE KILLERS", "BATTLE ROYALE DEATHS",
		"BATTLYE ROYALE PARTICIPANTS", "BATTLE ROYALE VICTORS",
		"ARENA KILLERS", "ARENA DEATHS", "TOTAL SOULS", "FASTEST TO 90",
		"GAMBLING LOSSES", "GAMBLING WINS", "BEST AVATARS", "TOTAL COMPLETED FIND QUESTS",
		"TOTAL ATTEMPTED FIND QUESTS","TOTAL COMPLETED HUNT QUESTS", "TOTAL ATTEMPTED HUNT QUESTS"
		,"SUICIDES", "TOTAL TREES CHOPPED", "TOTAL TREES MILLED", "TOTAL OBJS SOLD", "TOTAL MONEY MADE (in gold)", "MIVERBLIS GAMES WON", "MIVERBLIS GAMES LOST"
		};
		output = new_buf ();
		count = 0;
		pos = 0;
		sprintf (buf, "`nRANKING OF %s``", stat_name[stat]);
		add_buf (output, buf);
		add_buf (output, "\n\r");

		for (j = 0;j < MAX_PEOPLE_STAT; j++)
		{
			if (stat == STAT_TOTAL_EXP || stat == STAT_EXODUS_SCORE || stat == STAT_TG_LOSS || stat == STAT_TG_WON
				|| stat == STAT_AVATAR)
				sprintf (buf, "`k%2d`o)`o %-20s `o[`i%10lu`o]``    ", j+1,TGS_Names[stat][j], TGS_Stats[stat][j]);
			else sprintf (buf, "`k%2d`o)`o %-20s `o[`i%8ld`o]``    ", j+1, TGS_Names[stat][j], TGS_Stats[stat][j]);
			add_buf (output, buf);
			if (++pos % 2 == 0)
			{
				add_buf (output, "\n\r");
				pos = 0;
			}

		}
		if (pos % 2 != 0)
			add_buf (output, "\n\r");
		page_to_char (buf_string (output), ch);
		free_buf (output);
	}


	return;
}


void do_prevent (CHAR_DATA * ch, char *argument)
{

	if (IS_NPC (ch))
		return;
	if (get_skill (ch, gsn_prevent_escape) < 1)
	{
		send_to_char ("You cannot Prevent Escape without the skill\r\n", ch);
		return;
	}
	if (argument[0] != '\0')
		send_to_char
		("Syntax: prevent (toggles Prevent Escape on or off)\r\n", ch);

	else if (ch->pcdata->prevent_escape)
	{
		send_to_char ("Prevent Escape is now turned `bOFF``\r\n", ch);
		ch->pcdata->prevent_escape = FALSE;
	}

	else
	{
		send_to_char ("Prevent Escape is now turned `bON``\r\n", ch);
		ch->pcdata->prevent_escape = TRUE;
	}
}

//Iblis - Monk fighting art skill (allows changing of their art)
void do_art (CHAR_DATA * ch, char *argument)
{

	if (IS_NPC (ch))
		return;
	if (get_skill (ch, gsn_the_arts) < 1)
	{
		send_to_char ("You cannot change your fighting art at will.\r\n", ch);
		return;
	}
	if (argument[0] == '\0')
	{
		send_to_char ("Change to which art?\n\r", ch);
	}

	else
	{
		if (number_percent () > get_skill (ch, gsn_the_arts))
		{
			send_to_char ("You fail to change your art of fighting.\r\n", ch);
			return;
		}
		if (!strcmp (argument, "tiger"))
		{
			if ((ch->move < ch->max_move * .2)
				|| (ch->mana < ch->max_mana * .2))
			{
				send_to_char
					("You're not ready to change to the art of the Tiger now.\r\n",
					ch);
				return;
			}

			else
			{
				ch->move -= ch->max_move * .2;
				ch->mana -= ch->max_mana * .2;
				ch->dam_type = 5;
				send_to_char
					("You decide to use the fighting art of the Tiger.\r\n", ch);
			}
		}

		else if (!strcmp (argument, "crane"))
		{
			if ((ch->move < ch->max_move * .2)
				|| (ch->mana < ch->max_mana * .2))
			{
				send_to_char
					("You're not ready to change to the art of the Crane now.\r\n",
					ch);
				return;
			}

			else
			{
				ch->move -= ch->max_move * .2;
				ch->mana -= ch->max_mana * .2;
				ch->dam_type = 3;
				send_to_char
					("You decide to use the fighting art of the Crane.\r\n", ch);
			}
		}

		else if (!strcmp (argument, "snake"))
		{
			if ((ch->move < ch->max_move * .2)
				|| (ch->mana < ch->max_mana * .2))
			{
				send_to_char
					("You're not ready to change to the art of the Snake now.\r\n",
					ch);
				return;
			}

			else
			{
				ch->move -= ch->max_move * .2;
				ch->mana -= ch->max_mana * .2;
				ch->dam_type = 11;
				send_to_char
					("You decide to use the fighting art of the Snake.\r\n", ch);
			}
		}

		else if (!strcmp (argument, "panther"))
		{
			if ((ch->move < ch->max_move * .2)
				|| (ch->mana < ch->max_mana * .2))
			{
				send_to_char
					("You're not ready to change to the art of the Panther now.\r\n",
					ch);
				return;
			}

			else
			{
				ch->move -= ch->max_move * .2;
				ch->mana -= ch->max_mana * .2;
				ch->dam_type = 17;
				send_to_char
					("You decide to use the fighting art of the Panther.\r\n",
					ch);
			}
		}

		else if (!strcmp (argument, "dragon"))
		{
			if ((ch->move < ch->max_move * .5)
				|| (ch->mana < ch->max_mana * .5))
			{
				send_to_char
					("You're not ready to change to the art of the Dragon now.\r\n",
					ch);
				return;
			}

			else
			{
				ch->move -= ch->max_move * .5;
				ch->mana -= ch->max_mana * .5;
				ch->dam_type = 18;
				send_to_char
					("You decide to use the fighting art of the Dragon.\r\n", ch);
			}
		}

		else
		{
			send_to_char ("Change to which art?\n\r", ch);
		}
	}
}

//Iblis - Monk sense life function (skill is used automatically)
void sense_life (CHAR_DATA * ch)
{

	//Iblis 2/17/04 - Aquatic Sense
	if ((ch->in_room && 
		(ch->in_room->sector_type == SECT_UNDERWATER || ch->in_room->sector_type == SECT_WATER_OCEAN)
		&& number_percent () < get_skill (ch, gsn_aquatic_sense)) ||	
		//IBLIS 5/31/03 - Do Sense Life
		(number_percent () < get_skill (ch, gsn_sense_life)))
	{
		CHAR_DATA *gch;
		short lifeforms = 0;
		for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
		{
			if (!can_see (ch, gch) && (gch->invis_level < ch->level))
				if (lifeforms++)
					break;
		}
		if (lifeforms == 1)
			send_to_char ("You sense a hidden life form in the room.\r\n", ch);

		else if (lifeforms)
			send_to_char ("You sense hidden life forms in the room.\r\n", ch);
		check_improve (ch, gsn_sense_life, TRUE, 2);
	}
}

//Iblis - Ranger toggle for their falcon alert
void do_falconry (CHAR_DATA * ch, char *argument)
{
	char buf[100];
	if (IS_NPC (ch))
		return;
	if (get_skill (ch, gsn_falconry) < 1)
	{
		send_to_char ("Falcons don't like you very much.\r\n", ch);
		return;
	}
	if (!strcmp (argument, "on"))
	{
		ch->pcdata->falcon_alert = TRUE;
		send_to_char ("Your falconry proximity alert is now `bON``.\r\n", ch);
	}

	else if (!strcmp (argument, "off"))
	{
		ch->pcdata->falcon_alert = FALSE;
		send_to_char ("Your falconry proximity alert is now `bOFF``.\r\n", ch);
	}

	else
	{
		sprintf (buf,
			"Your falconry proximity alert is currently `b%s``.\r\n",
			ch->pcdata->falcon_alert ? "ON" : "OFF");
		send_to_char (buf, ch);
		send_to_char
			("`kFalconry off ``will turn off this alert.\r\n`kFalconry on ``will turn on this alert.\r\n",
			ch);
	}
}


//Iblis 6/22/03 - New Ranger Skill
void falcon_alert (CHAR_DATA * ch)
{
	int dir, distance, angle;
	bool sqwuaked = FALSE;
	ROOM_INDEX_DATA *was_in_room, *was_in_room2;

	if ((IS_NPC (ch) && !(IS_SET (ch->act, ACT_AGGRESSIVE))))
		return;
	if (ch == NULL || ch->in_room == NULL)
	{
		if (ch == NULL)
			bug ("NULL ch in falcon_alert", 0);

		else
			bug ("NULL ch->in_room in falcon_alert", 0);
		return;
	}
	if (!IS_OUTSIDE (ch))
		return;

	was_in_room = ch->in_room;
	for (dir = 0; dir <= 3; dir++)
	{
		if (ch->in_room->exit[dir] != NULL)
		{
			for (distance = 1; distance <= 4; distance++)
			{
				EXIT_DATA *pexit;
				CHAR_DATA *rch;
				if (((pexit = ch->in_room->exit[dir]) != NULL)
					&& (pexit->u1.to_room != NULL)
					&& (pexit->u1.to_room != was_in_room))
				{

					/* If the door is closed, stop looking... */
					if (IS_SET (pexit->exit_info, EX_CLOSED))
						break;
					ch->in_room = pexit->u1.to_room;
					was_in_room2 = ch->in_room;
					for (angle = 0; angle < 5 - distance; angle++)
					{
						if ((angle >= 1) &&
							(((pexit =
							ch->in_room->exit[(dir + 1) % 4]) !=
							NULL) && (pexit->u1.to_room != NULL)
							&& (pexit->u1.to_room != was_in_room)))
						{

							/* If the door is closed, stop looking... */
							// Iblis - Yes, I believe this is right even for here
							if (IS_SET (pexit->exit_info, EX_CLOSED))
								break;
							ch->in_room = pexit->u1.to_room;
						}

						else if (angle >= 1)
							break;

						if (!IS_OUTSIDE (ch))
							continue;
						for (rch = pexit->u1.to_room->people; rch != NULL;
							rch = rch->next_in_room)
						{

							if (rch->in_room == NULL)
								continue;
							if (can_see (ch, rch))
							{

								if ((IS_SET (rch->act, ACT_AGGRESSIVE)
									&& ch && ch->pcdata
									&& (!ch->pcdata->loner
									&& ch->clan == CLAN_BOGUS))
									|| (rch && rch->
									pcdata && ch && ch->
									pcdata && (ch->pcdata->
									loner || ch->
									clan != CLAN_BOGUS)
									&& ((rch->pcdata->
									loner || rch->clan !=
									CLAN_BOGUS) && (rch->
									clan
									!= ch->
									clan
									|| ch->
									clan
									== CLAN_BOGUS))))
								{
									if (!sqwuaked && ch && ch->pcdata
										&& ch->pcdata->falcon_alert
										&& ch->pcdata->falcon_wait == 0
										&& number_percent () <
										get_skill (ch, gsn_falconry) + 1)
									{

										ch->in_room = was_in_room;
										if (ch->in_room)
										{
											act
												("$n's falcon lets out a blood-curdling `iSQUAWK``.",
												ch, NULL, NULL, TO_ROOM);
											act
												("Your falcon lets out a blood-curdling `iSQUAWK``.",
												ch, NULL, NULL, TO_CHAR);
										}

										else
											bug
											("ch->in_room NULLxx in falcon_alert",
											0);
										ch->in_room = pexit->u1.to_room;
										sqwuaked = TRUE;

									}
								}
							}
							if (can_see (rch, ch))
							{
								if ((IS_SET (ch->act, ACT_AGGRESSIVE)
									&& rch && rch->pcdata
									&& (!rch->pcdata->loner
									&& rch->clan == CLAN_BOGUS))
									|| (ch && ch->pcdata && rch
									&& rch->pcdata
									&& (rch->pcdata->loner
									|| rch->clan != CLAN_BOGUS)
									&&
									((ch->pcdata->loner || ch->
									clan !=
									CLAN_BOGUS) && (ch->clan !=
									rch->
									clan
									|| rch->
									clan == CLAN_BOGUS))))
								{
									if (rch && rch->pcdata
										&& rch->pcdata->falcon_alert
										&& number_percent () <
										get_skill (rch, gsn_falconry) + 1
										&& ch && ch->pcdata && ch->
										pcdata->falcon_wait == 0)
									{

										act
											("$n's falcon lets out a blood-curdling `iSQUAWK``.",
											rch, NULL, NULL, TO_ROOM);
										act
											("\r\nYour falcon lets out a blood-curdling `iSQUAWK``.\r\n",
											rch, NULL, NULL, TO_CHAR);

									}
								}
							}
						}
					}
					ch->in_room = was_in_room2;
				}
			}
		}
		ch->in_room = was_in_room;
	}
}

int amount_of_cards=0;

void do_cardcount(CHAR_DATA *ch, char* argument)
{
	char buf[16];
	if (!IS_IMMORTAL(ch) || argument[0] == '\0')
	{
		sprintf(buf,"%d",amount_of_cards);
		act("The total number of cards is now $t.",ch,buf,NULL,TO_CHAR);
		return;
	}
	if (!is_number(argument))
	{
		send_to_char("Stupid immortal.  You have to set the card count to a NUMBER!",ch);
		return;
	}
	amount_of_cards = atoi(argument);
	sprintf(buf,"%d",amount_of_cards);
	act("The total number of cards is now set to $t.",ch,buf,NULL,TO_CHAR);
	save_cardcount ();
}


void do_present(CHAR_DATA *ch, char* argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *victim=NULL;
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument(argument,arg);
	if (arg[0] == '\0')
	{
		send_to_char("Present what? (to whom)?\n\r",ch);
		return;
	}
	// Scan and try to present a WORN item FIRST
	obj = get_obj_wear(ch,arg);
	if (obj == NULL)
		obj = get_obj_carry(ch,arg);
	if (obj == NULL)
	{
		send_to_char("I don't see that here.\n\r",ch);
		return;
	}
	//present to ROOM
	if (argument[0] == '\0')
	{
		act("You present $p.",ch,obj,NULL,TO_CHAR);
		act("$n presents to you $p.",ch,obj,NULL,TO_ROOM);
		act(obj->description,ch,NULL,NULL,TO_ROOM);
		//    send_to_char(obj->description,victim);
		//send_to_char("\n\r",victim);
		if (obj->item_type == ITEM_CARD)
		{
			for (victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
			{
				if (victim != ch)
					appraise(victim,obj);
			}
		}
		//appraise(victim,obj);
	}    
	else 
	{
		victim = get_char_room(ch,argument);
		if (victim == NULL)
		{
			send_to_char("I don't see them here.\n\r",ch);
			return;
		}
		act("You present $p to $N.",ch,obj,victim,TO_CHAR);
		act("$n presents to you $p.",ch,obj,victim,TO_VICT);
		send_to_char(obj->description,victim);
		send_to_char("\n\r",victim);
		if (obj->item_type == ITEM_CARD)
			appraise(victim,obj);
	}


}

//Iblis 9/29/04 - Gets their "effective" position (IE converts newer "special" positions into older,
// more "well-known" positions
// NOTE: I did not replace ch->position everywhere with this, only in the current places that mattered
//   for POS_COMA.  So if you add a new position and map it to something other than POS_SLEEPING you will
//   need to scan the code and swap a bunch of ch->position's for get_position
short get_position(CHAR_DATA *ch)
{
	//If an old school position, get the hell out of this function
	if (ch->position < POS_FEIGNING_DEATH)
		return ch->position;
	if (ch->position == POS_COMA)
		return POS_SLEEPING;
	//Iblis - A hack to allow more positions to be added (For tradeskills)
	//at the POS_SITTING level, while not having to fix area files,
	//which have default positions for mobs hardcoded to POS_STANDING etc
	else if (ch->position > POS_COMA)
		return POS_SITTING;
	return ch->position;
}


//Iblis 10/02/04 - deals with consent to CJ switch spell
void do_noswitch (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

	if (ch->Class == PC_CLASS_CHAOS_JESTER)
	{
		send_to_char("You are not afforded the luxury of consenting to switches.\n\r",ch);
		return;
	}

	if (IS_SET(ch->act,PLR_SWITCHOK))
	{
		send_to_char ("You are no longer allowing switches.\n\r",ch);
		REMOVE_BIT(ch->act,PLR_SWITCHOK);
	}
	else
	{
		send_to_char ("You are now allowing switches.\n\r",ch);
		SET_BIT(ch->act,PLR_SWITCHOK);
	}
}

/*
 * This command will check to see how many RP Points a player has currently on them.
 * Skill is meant to be used by Lvl 91 or higher.
 * - Justin (Clive)
 */
void do_checkrp(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	one_argument (argument, arg);

	// If your switched into a NPC, or trying to do_force an NPC
	if (IS_NPC(ch))
	{
		send_to_char("Sorry, as a npc you can not do that.\n\r", ch);
		return;
	}

	// If no player name is placed in argument
	if (arg[0] == '\0')
	{
		send_to_char("You must provide a name.\n\r", ch);
		return;
	}

	// If the player is not online
	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	// Adding part of the score borders, purely for visual cosmetics.
	send_to_char("`b+-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+``\n\r", ch);

	sprintf(buf, "  Name:       `i%s``\n\r", victim->name);
	send_to_char(buf, ch);
	sprintf(buf, "  RP Points:  `o%ld``\n\r", victim->pcdata->rppoints);
	send_to_char(buf, ch);

	// Add above this for more information.
	send_to_char("`b+-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+``\n\r", ch);

	return;
}
