
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <crypt.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/resource.h>
#include <unistd.h>
#include "merc.h"
#include "recycle.h"
#include "olc.h"
#include "tables.h"
#include "interp.h"
#include "clans/new_clans.h"
#include "clans/new_clans_comm.h"
#include "clans/new_clans_io.h"
#include "clans/new_clans_util.h"
#include "obj_trig.h"
#include "Druid.h"

#define ANSI_CLSHOME "[2J[1;1H"
void multiClass args ((DESCRIPTOR_DATA * d, char *argument));
void cardgame_menu args ((CHAR_DATA *ch, char* argument));
void card_game_prompt args ((CHAR_DATA *ch));
void friend_poof args (( CHAR_DATA *ch ));
extern AREA_DATA *area_first;
extern bool keepincomm;
extern int max_on;
void do_auto_shutdown args ((int signo));
extern const struct cmd_type cmd_table[];
extern char *color_table[];
extern char *center_text (char *txtstr, int txtnum);
extern long int level_cost (int level);
struct creating_type *char_creating = NULL;
extern bool nodam_message;
extern bool isorder;
void backcalc_totalsouls args((CHAR_DATA *ch));
OBJ_DATA *get_obj_vnum_string_char args((CHAR_DATA *ch, int vnum, char* arg));
OBJ_DATA *get_obj_vnum_char args((CHAR_DATA* ch, int vnum));
void friend_poof args((CHAR_DATA *ch));
	

/*
 * Global boolean for determining if name services should be used
 */
bool slowns = FALSE;

/*
 * Global boolean for determining if half experience is toggled
 */
bool half = FALSE;

/*
 * Global boolean for determining if quad experience is toggled
 */
bool quad = FALSE;

/*
 * Global boolean for determining if double experience is toggled
 */
bool doub = FALSE;

/*
 *  * Global boolean for determining if Battle Royale is toggled
 *   */
bool battle_royale = FALSE;
extern void format_color args ((char *New, char *str, int length));
void init_signals args ((void));
void do_auto_shutdown args ((int signo));

//IBLIS 6/20/03 - Needed to update count at logon (from act_info.c)
void update_count ();

//Iblis 1/18/04 - Needed for litans
void spell_haste args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
	

/* command procedures needed */
DECLARE_DO_FUN (do_help);
DECLARE_DO_FUN (do_look);
DECLARE_DO_FUN (do_skills);
DECLARE_DO_FUN (do_outfit);
DECLARE_DO_FUN (do_unread);
char *bad_names[] = { "bitch", "whore", "fuck", "nigger", NULL
};
struct cstats
{
  short str_min;
  short str_max;
  short int_min;
  short int_max;
  short wis_min;
  short wis_max;
  short dex_min;
  short dex_max;
  short con_min;
  short con_max;
  short cha_min;
  short cha_max;
  short align_min;
  short align_max;
};
long genraces[MAX_PC_RACE] = {
  /* Reserved */
  CLASS_MAGE | CLASS_CLERIC | CLASS_THIEF | CLASS_WARRIOR |
    CLASS_RANGER | CLASS_BARD | CLASS_PALADIN | CLASS_ASSASSIN |
    CLASS_REAVER | CLASS_MONK | CLASS_DRUID,
  /* Human    */
  CLASS_MAGE | CLASS_CLERIC | CLASS_THIEF | CLASS_WARRIOR |
    CLASS_RANGER | CLASS_BARD | CLASS_PALADIN | CLASS_ASSASSIN |
    CLASS_REAVER | CLASS_MONK | CLASS_DRUID,
  /*  Elf      */
  CLASS_MAGE | CLASS_CLERIC | CLASS_THIEF | CLASS_WARRIOR |
    CLASS_RANGER | CLASS_BARD | CLASS_PALADIN | CLASS_ASSASSIN |
    CLASS_REAVER | CLASS_MONK | CLASS_DRUID,
  /* Dwarf     */
  CLASS_CLERIC | CLASS_THIEF | CLASS_WARRIOR | CLASS_RANGER |
    CLASS_BARD | CLASS_ASSASSIN | CLASS_REAVER,
  /* Canthi    */
  CLASS_MAGE | CLASS_CLERIC | CLASS_THIEF | CLASS_WARRIOR |
    CLASS_RANGER | CLASS_BARD | CLASS_ASSASSIN,
  /* Vroath    */
  CLASS_MAGE | CLASS_CLERIC | CLASS_WARRIOR | CLASS_ASSASSIN | CLASS_REAVER,
  /* Syvin     */
  CLASS_CLERIC | CLASS_THIEF | CLASS_RANGER | CLASS_BARD | CLASS_ASSASSIN |
  CLASS_DRUID,
  /* Sidhe */
  CLASS_MAGE | CLASS_BARD | CLASS_THIEF | CLASS_ASSASSIN | CLASS_DRUID,
  /* Avatar */
  CLASS_MAGE | CLASS_CLERIC | CLASS_THIEF | CLASS_WARRIOR |
    CLASS_RANGER | CLASS_BARD | CLASS_PALADIN | CLASS_ASSASSIN |
    CLASS_REAVER | CLASS_MONK | CLASS_DRUID,
  /* Litan */
  CLASS_THIEF | CLASS_WARRIOR | CLASS_RANGER |
   CLASS_ASSASSIN | CLASS_REAVER | CLASS_PALADIN | CLASS_CLERIC,

  /* Kalian */
    CLASS_MAGE | CLASS_CLERIC | CLASS_THIEF | CLASS_WARRIOR |
	        CLASS_RANGER | CLASS_BARD | CLASS_PALADIN | CLASS_ASSASSIN |
		    CLASS_REAVER | CLASS_MONK,
    /* Lich */
      CLASS_MAGE | CLASS_CLERIC | CLASS_THIEF | CLASS_WARRIOR |
	           CLASS_ASSASSIN | CLASS_REAVER | CLASS_MONK,
  /* Nerix */
      CLASS_WARRIOR | CLASS_CLERIC | CLASS_PALADIN | CLASS_BARD,
  /* Swarm */
      CLASS_MONK,
  /* Nidae */
     CLASS_MAGE | CLASS_CLERIC | CLASS_WARRIOR | CLASS_RANGER | CLASS_BARD | CLASS_MONK,
      
};
struct calign
{
  int align_min;
  int align_max;
};
struct calign Class_align[MAX_CLASS] = {
/* Mage    */
  {-1000, 1000},
/* Cleric  */
  {-1000, 1000},
/* Thief   */
  {-1000, 1000},
/* Warrior */
  {-1000, 1000},
/* Ranger  */
  {-1000, 1000},
/* Bard    */
  {-1000, 1000},
/* Paladin */
  {300, 1000},
/* Assassin*/
  {-1000, 1000},
/* Reaver */
  {-1000, -100},
/* Monk */
  {-1000, 1000},
/* Necro */
  {-1000, 1000},
  {-1000, 1000},{-1000, 1000},{-1000, 1000},{-1000, 1000}, {-1000, 1000},
};
struct cstats genstats[MAX_PC_RACE] = {
  /*   Str     Int     Wis     Dex     Con     Cha       Align    */
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1000, 1000},
/* Hum */
  {6, 18, 6, 18, 6, 18, 6, 18, 6, 18, 6, 18, -1000, 1000},
/* Elf */
  {5, 14, 13, 22, 6, 18, 10, 19, 6, 15, 6, 19, -1000, 1000},
/* Dwa */
  {13, 21, 6, 15, 6, 18, 5, 13, 7, 20, 5, 15, -1000, 1000},
/* Can */
  {5, 17, 7, 19, 8, 19, 9, 19, 6, 18, 6, 18, -1000, 1000},
/* Vro */
  {14, 22, 4, 12, 4, 14, 5, 13, 12, 21, 3, 12, -1000, 100},
/* Syv */
  {7, 18, 6, 18, 7, 19, 11, 21, 7, 18, 6, 18, -1000, 1000},
/* Sid */
  {4, 13, 15, 22, 6, 18, 11, 19, 5, 14, 14, 20, -1000, -1000},
/* Ava */
  {6, 18, 6, 18, 6, 18, 6, 18, 6, 18, 6, 18, -1000, 1000},
 /* Lit */
  {9, 17, 3, 10, 5, 13, 14, 22, 9, 17, 3, 12, -1000, 1000},
 /* Kal */
  {10, 23, 10, 23, 6, 16, 10, 23, 4, 10, 6, 16, -1000, 1000},
 /* Lic */
  {6, 21, 6, 21, 6, 16, 6, 16, 6, 21, 6, 14, -1000, -1000},
 /* Ner */
  {8, 21, 5, 15, 5, 16, 9, 22, 6, 18, 6, 18, 1000, 1000},
 /* Swa */
  {8, 21, 5, 15, 5, 16, 9, 22, 6, 18, 6, 18, -1000, 1000},  //Ignored for swarms.  They can't reroll
 /* Nid */
  {6, 18, 6, 18, 6, 18, 6, 16, 6, 18, 6, 16, -1000, 1000},  
};
void roll_stats (CHAR_DATA * ch)
{
  short total = 0, i, count;
  total += ch->perm_stat[STAT_STR] =
    number_range (genstats[ch->race].str_min, genstats[ch->race].str_max);
  total += ch->perm_stat[STAT_INT] =
    number_range (genstats[ch->race].int_min, genstats[ch->race].int_max);
  total += ch->perm_stat[STAT_WIS] =
    number_range (genstats[ch->race].wis_min, genstats[ch->race].wis_max);
  total += ch->perm_stat[STAT_DEX] =
    number_range (genstats[ch->race].dex_min, genstats[ch->race].dex_max);
  total += ch->perm_stat[STAT_CON] =
    number_range (genstats[ch->race].con_min, genstats[ch->race].con_max);
  total += ch->perm_stat[STAT_CHA] =
    number_range (genstats[ch->race].cha_min, genstats[ch->race].cha_max);
  count = 0;
  if (total < 70)
    {
      while (total < 70 && count < 70)
	{
	  count++;
	  i = number_range (STAT_STR, STAT_CHA);
	  switch (i)
	    {
	    case STAT_STR:
	      if (ch->perm_stat[i] < genstats[ch->race].str_max)
		{
		  ch->perm_stat[i] += 1;
		  total++;
		}
	      break;
	    case STAT_INT:
	      if (ch->perm_stat[i] < genstats[ch->race].int_max)
		{
		  ch->perm_stat[i] += 1;
		  total++;
		}
	      break;
	    case STAT_WIS:
	      if (ch->perm_stat[i] < genstats[ch->race].wis_max)
		{
		  ch->perm_stat[i] += 1;
		  total++;
		}
	      break;
	    case STAT_DEX:
	      if (ch->perm_stat[i] < genstats[ch->race].dex_max)
		{
		  ch->perm_stat[i] += 1;
		  total++;
		}
	      break;
	    case STAT_CON:
	      if (ch->perm_stat[i] < genstats[ch->race].con_max)
		{
		  ch->perm_stat[i] += 1;
		  total++;
		}
	      break;
	    case STAT_CHA:
	      if (ch->perm_stat[i] < genstats[ch->race].cha_max)
		{
		  ch->perm_stat[i] += 1;
		  total++;
		}
	      break;
	    }
	}
    }

  else if (total > 70)
    {
      count = 0;
      while (total > 70 && count < 70)
	{
	  count++;
	  i = number_range (0, 5);
	  switch (i)
	    {
	    default:
	    case STAT_STR:
	      if (ch->perm_stat[i] > genstats[ch->race].str_min)
		{
		  ch->perm_stat[i] -= 1;
		  total--;
		}
	      break;
	    case STAT_INT:
	      if (ch->perm_stat[i] > genstats[ch->race].int_min)
		{
		  ch->perm_stat[i] -= 1;
		  total--;
		}
	      break;
	    case STAT_WIS:
	      if (ch->perm_stat[i] > genstats[ch->race].wis_min)
		{
		  ch->perm_stat[i] -= 1;
		  total--;
		}
	      break;
	    case STAT_DEX:
	      if (ch->perm_stat[i] > genstats[ch->race].dex_min)
		{
		  ch->perm_stat[i] -= 1;
		  total--;
		}
	      break;
	    case STAT_CON:
	      if (ch->perm_stat[i] > genstats[ch->race].con_min)
		{
		  ch->perm_stat[i] -= 1;
		  total--;
		}
	      break;
	    case STAT_CHA:
	      if (ch->perm_stat[i] > genstats[ch->race].cha_min)
		{
		  ch->perm_stat[i] -= 1;
		  total--;
		}
	      break;
	    }
	}
    }
}
void add_new_create (char *name)
{
  struct creating_type *new_creator;
  new_creator = malloc (sizeof (struct creating_type));
//  new_creator = reinterpret_cast<creating_type *>(malloc(sizeof(struct creating_type)));
  new_creator->name = str_dup (name);
  new_creator->next = char_creating;
  char_creating = new_creator;
}

bool check_twolinks (DESCRIPTOR_DATA *d)
{
	return FALSE;
	/*
CHAR_DATA *vch;
      if (!d || !d->host)
      {
	bug("d is null in check_twolinks.",0);
        return FALSE;
      }
      for (vch = char_list; vch != NULL; vch = vch->next)
	{
          if (!IS_NPC(vch) && vch->pcdata)
	    {
              if ((vch != d->character) && (!strcmp (vch->pcdata->ldhost, d->host)))
		{
		  sprintf (log_buf, "WARNING: %s is already logged in as %s.",
			   d->host, vch->name);
		  log_string (log_buf);
		  wiznet (log_buf, NULL, NULL, WIZ_SITES, 0, 0);
                  return TRUE;
		}
	      continue;
	    }
	}
return FALSE;*/
}



void remove_create (char *name)
{
  struct creating_type *create_dele, *dele_next;
  if (char_creating == NULL)
    return;
  if (!strcmp (char_creating->name, name))
    {
      dele_next = char_creating->next;
      free_string (char_creating->name);
      free (char_creating);
      char_creating = dele_next;
    }

  else
    for (create_dele = char_creating; create_dele;
	 create_dele = create_dele->next)
      {
	if (create_dele->next != NULL)
	  if (!strcmp (create_dele->next->name, name))
	    {
	      dele_next = create_dele->next;
	      create_dele->next = create_dele->next->next;
	      free_string (dele_next->name);
	      free (dele_next);
	    }
      }
}
int check_create (char *name)
{
  struct creating_type *create_walk;
  for (create_walk = char_creating; create_walk;
       create_walk = create_walk->next)
    if (!str_cmp (create_walk->name, name))
      return (1);
  return (0);
}


/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif /*  */

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern int malloc_debug args ((int));
extern int malloc_verify args ((void));

#endif /*  */

/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif /*  */

#if defined(unix)
#include <signal.h>
#endif /*  */

#if defined(apollo)
#undef __attribute
#endif /*  */

/*
 * Socket and TCP/IP stuff.
 */
#if	defined(macintosh) || defined(MSDOS)
const char echo_off_str[] = { '\0' };
const char echo_on_str[] = { '\0' };
const char go_ahead_str[] = { '\0' };

#endif /*  */

#if	defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "telnet.h"
const char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const char echo_on_str[] = { IAC, WONT, TELOPT_ECHO, '\0' };
char go_ahead_str[] = { IAC, GA, '\0' };

#endif /*  */

/*
 * OS-dependent declarations.
 */
#if	defined(_AIX)
#include <sys/select.h>
int accept args ((int s, struct sockaddr * addr, int *addrlen));
int bind args ((int s, struct sockaddr * name, int namelen));
void bzero args ((char *b, int length));
int getpeername args ((int s, struct sockaddr * name, int *namelen));
int getsockname args ((int s, struct sockaddr * name, int *namelen));
int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
int listen args ((int s, int backlog));
int setsockopt
args ((int s, int level, int optname, void *optval, int optlen));
int socket args ((int domain, int type, int protocol));

#endif /*  */

#if	defined(apollo)
#include <unistd.h>
void bzero args ((char *b, int length));

#endif /*  */

#if	defined(__hpux)
int accept args ((int s, void *addr, int *addrlen));
int bind args ((int s, const void *addr, int addrlen));
void bzero args ((char *b, int length));
int getpeername args ((int s, void *addr, int *addrlen));
int getsockname args ((int s, void *name, int *addrlen));
int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
int listen args ((int s, int backlog));
int setsockopt
args ((int s, int level, int optname, const void *optval, int optlen));
int socket args ((int domain, int type, int protocol));

#endif /*  */

#if	defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif /*  */

/* #if	defined(linux)
#include <signal.h>
int accept args((int s, struct sockaddr * addr, int *addrlen));
int bind args((int s, struct sockaddr * name, int namelen));
int close args((int fd));
int getpeername args((int s, struct sockaddr * name, int *namelen));
int getsockname args((int s, struct sockaddr * name, int *namelen));
int gettimeofday args((struct timeval * tp, struct timezone * tzp));
int listen args((int s, int backlog));
int read args((int fd, char *buf, int nbyte));
int select args((int width, fd_set * readfds, fd_set * writefds,
		 fd_set * exceptfds, struct timeval * timeout));
int socket args((int domain, int type, int protocol));
int write args((int fd, char *buf, int nbyte));
#endif
*/
#if	defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct timeval
{
  time_t tv_sec;
  time_t tv_usec;
};

#if	!defined(isascii)
#define	isascii(c)		( (c) < 0200 )
#endif /*  */
static long theKeys[4];
int gettimeofday args ((struct timeval * tp, void *tzp));

#endif /*  */

#if	defined(MIPS_OS)
extern int errno;

#endif /*  */

#if	defined(MSDOS)
int gettimeofday args ((struct timeval * tp, void *tzp));
int kbhit args ((void));

#endif /*  */

#if	defined(NeXT)
int close args ((int fd));
int fcntl args ((int fd, int cmd, int arg));

#if	!defined(htons)
u_short htons args ((u_short hostshort));

#endif /*  */
#if	!defined(ntohl)
u_long ntohl args ((u_long hostlong));

#endif /*  */
int read args ((int fd, char *buf, int nbyte));
int select
args ((int width, fd_set * readfds, fd_set * writefds, fd_set * exceptfds,
       struct timeval * timeout));
int write args ((int fd, char *buf, int nbyte));

#endif /*  */

#if	defined(sequent)
int accept args ((int s, struct sockaddr * addr, int *addrlen));
int bind args ((int s, struct sockaddr * name, int namelen));
int close args ((int fd));
int fcntl args ((int fd, int cmd, int arg));
int getpeername args ((int s, struct sockaddr * name, int *namelen));
int getsockname args ((int s, struct sockaddr * name, int *namelen));
int gettimeofday args ((struct timeval * tp, struct timezone * tzp));

#if	!defined(htons)
u_short htons args ((u_short hostshort));

#endif /*  */
int listen args ((int s, int backlog));

#if	!defined(ntohl)
u_long ntohl args ((u_long hostlong));

#endif /*  */
int read args ((int fd, char *buf, int nbyte));
int select
args ((int width, fd_set * readfds, fd_set * writefds, fd_set * exceptfds,
       struct timeval * timeout));
int setsockopt
args ((int s, int level, int optname, caddr_t optval, int optlen));
int socket args ((int domain, int type, int protocol));
int write args ((int fd, char *buf, int nbyte));

#endif /*  */

/* This includes Solaris Sys V as well */
#if defined(sun)
int accept args ((int s, struct sockaddr * addr, int *addrlen));
int bind args ((int s, struct sockaddr * name, int namelen));
void bzero args ((char *b, int length));
int close args ((int fd));
int getpeername args ((int s, struct sockaddr * name, int *namelen));
int getsockname args ((int s, struct sockaddr * name, int *namelen));

#ifndef _SYS_TIME_H
int gettimeofday args ((struct timeval * tp, struct timezone * tzp));

#endif /*  */
int listen args ((int s, int backlog));
int select
args ((int width, fd_set * readfds, fd_set * writefds, fd_set * exceptfds,
       struct timeval * timeout));

#if defined(SYSV)
/*Nint setsockopt               args( ( int s, int level, int optname,
   const char *optval, int optlen ) ); */
#else /*  */
/* int  setsockopt      args( ( int s, int level, int optname, void *optval,
   int optlen ) ); */
#endif /*  */
int socket args ((int domain, int type, int protocol));

#endif /*  */

#if defined(ultrix)
int accept args ((int s, struct sockaddr * addr, int *addrlen));
int bind args ((int s, struct sockaddr * name, int namelen));
void bzero args ((char *b, int length));
int close args ((int fd));
int getpeername args ((int s, struct sockaddr * name, int *namelen));
int getsockname args ((int s, struct sockaddr * name, int *namelen));
int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
int listen args ((int s, int backlog));
int read args ((int fd, char *buf, int nbyte));
int select
args ((int width, fd_set * readfds, fd_set * writefds, fd_set * exceptfds,
       struct timeval * timeout));
int setsockopt
args ((int s, int level, int optname, void *optval, int optlen));
int socket args ((int domain, int type, int protocol));
int write args ((int fd, char *buf, int nbyte));

#endif /*  */

/*
 * Global variables.
 */
DESCRIPTOR_DATA *descriptor_list;	/* All open descriptors         */
DESCRIPTOR_DATA *d_next;	/* Next descriptor in loop      */
FILE *fpReserve;		/* Reserved file handle         */
bool god;			/* All new chars are gods!      */
bool merc_down;			/* Shutdown                     */
long shutdown_time = -1;	/* The time for a shutdown      */
long shutdown_severity = 7;	/* The starting severity        */
bool wizlock;			/* Game is wizlocked            */
bool newlock;			/* Game is newlocked            */
char str_boot_time[MAX_INPUT_LENGTH];
time_t current_time;		/* time of this pulse */

/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos args ((void));
bool read_from_descriptor args ((DESCRIPTOR_DATA * d));
bool write_to_descriptor args ((int desc, char *txt, int length));

#endif /*  */

#if defined(unix)
void game_loop_unix args ((int control, int whosock, int areasock));
int init_socket args ((int port));
void init_descriptor args ((int control));
bool read_from_descriptor args ((DESCRIPTOR_DATA * d));
bool write_to_descriptor args ((int desc, char *txt, int length));

#endif /*  */

/*
 * Other local functions (OS-independent).
 */
bool check_parse_name args ((char *name));
bool check_reconnect args ((DESCRIPTOR_DATA * d, char *name, bool fConn));
bool check_playing args ((DESCRIPTOR_DATA * d, char *name));
int main args ((int argc, char **argv));
void nanny args ((DESCRIPTOR_DATA * d, char *argument));
bool process_output args ((DESCRIPTOR_DATA * d, bool fPrompt));
void read_from_buffer args ((DESCRIPTOR_DATA * d));
void stop_idling args ((CHAR_DATA * ch));
void bust_a_prompt args ((CHAR_DATA * ch));
void do_con_get_ansi(DESCRIPTOR_DATA * d, char *argument); 
void do_con_get_name(DESCRIPTOR_DATA * d, char *argument); 
void do_con_get_old_password(DESCRIPTOR_DATA * d, char *argument); 
void do_con_break_connect(DESCRIPTOR_DATA * d, char *argument); 
void do_con_confirm_new_name(DESCRIPTOR_DATA * d, char *argument); 
void do_con_get_new_password(DESCRIPTOR_DATA * d, char *argument); 
void do_con_confirm_new_password(DESCRIPTOR_DATA * d, char *argument); 
void do_con_get_referral(DESCRIPTOR_DATA * d, char *argument); 
void do_con_get_email(DESCRIPTOR_DATA * d, char *argument); 
void do_con_get_new_race(DESCRIPTOR_DATA * d, char *argument); 
void do_con_confirm_stats(DESCRIPTOR_DATA * d, char *argument); 
void do_con_get_new_sex(DESCRIPTOR_DATA * d, char *argument); 
void do_con_get_avatar_type(DESCRIPTOR_DATA * d, char *argument); 
void do_con_get_new_class(DESCRIPTOR_DATA * d, char *argument); 
void do_con_get_alignment(DESCRIPTOR_DATA * d, char *argument); 
void do_con_pick_weapon(DESCRIPTOR_DATA * d, char *argument); 
void do_con_read_imotd(DESCRIPTOR_DATA * d, char *argument); 
void do_con_read_motd(DESCRIPTOR_DATA * d, char *argument); 
void do_con_sword_generation(DESCRIPTOR_DATA * d, char *argument); 
void do_con_sword_confirm(DESCRIPTOR_DATA * d, char *argument);
static void machine_reboot (int signo)
{
  DESCRIPTOR_DATA *d, *d_next;
  FILE *sfp;
  if ((sfp = fopen ("shutdown.txt", "a")) != NULL)
    {
      fprintf (sfp, "Machine-Reboot Shutdown");
      fclose (sfp);
    }
  merc_down = TRUE;
  for (d = descriptor_list; d != NULL; d = d_next)
    {
      d_next = d->next;
      if (d->connected == CON_PLAYING && d->character != NULL)
	{
	  send_to_char
	    ("Sorry, the mud is going down NOW due to a machine reboot.\n\r",
	     d->character);

/*        save_char_obj(d->character); */
	}
      close_socket (d);
    }
}
int fdsize;

//
// Akamai 01/24/00 - This function writes the process id of the Exodus
// main code to a file. Useful for special shutdown scripts.
//

// prototype
void exodus_pid ();

// function
void exodus_pid ()
{
  char buf[MAX_INPUT_LENGTH];
  char pidf[MAX_INPUT_LENGTH];
  FILE *fp;
  long pid = 0;
  pid = (long) getpid ();

//   sprintf(buf,"Exodus PID: %ld",pid);
  sprintf (buf, "%ld", pid);
  sprintf (pidf, "%s%s", BIN_DIR, EXODUS_PID);
  if ((fp = fopen (pidf, "w")) != NULL)
    {
      fprintf (fp, "%s", buf);
      fclose (fp);
      log_string ("boot_db() - wrote exodus.pid");
//      log_string (buf);
    }

  else
    {
      bug ("Couldn't open exodus.pid file.", 0);
      log_string (buf);
    }
}
int main (int argc, char **argv)
{
  struct rlimit fd_limit;

  //, stack_limit;
  struct timeval now_time;
  int port;
  int control, whosock, areasock;

#if defined(MALLOC_DEBUG)
  malloc_debug (2);

#endif /*  */
  init_signals ();		/* For the use of the signal handler */
  gettimeofday (&now_time, NULL);
  current_time = (time_t) now_time.tv_sec;
  strcpy (str_boot_time, ctime (&current_time));
  getrlimit (RLIMIT_NOFILE, &fd_limit);
  fd_limit.rlim_cur = fd_limit.rlim_max - 8;
  setrlimit (RLIMIT_NOFILE, &fd_limit);
  fdsize = fd_limit.rlim_cur;

  // Shinowlan - debug attempt to fix 'save' recurse problem
  // this, in general, does not work. So it's commented out.
/*
  getrlimit(RLIMIT_STACK, &stack_limit);
  log_buf[0]= '\0';
  sprintf (log_buf, "STACK: intially at  %ld bytes.", stack_limit.rlim_cur);
  log_string(log_buf);
  stack_limit.rlim_cur = stack_limit.rlim_max;
  setrlimit(RLIMIT_STACK, &stack_limit);

  log_buf[0]= '\0';
  sprintf (log_buf, "STACK: Current stack is now set to %ld bytes", stack_limit.rlim_max);
  log_string(log_buf);

  getrlimit(RLIMIT_STACK, &stack_limit);
  log_buf[0]= '\0';
  sprintf (log_buf, "STACK: is at  %ld bytes.", stack_limit.rlim_cur);
*/
  if ((fpReserve = fopen (NULL_FILE, "r")) == NULL)
    {
      perror (NULL_FILE);
      exit (1);
    }
  port = 1101;
  if (argc > 1)
    {
      if (!is_number (argv[1]))
	{
	  fprintf (stderr, "Usage: %s [port #]\n", argv[0]);
	  exit (1);
	}

      else if ((port = atoi (argv[1])) <= 1024)
	{
	  fprintf (stderr, "Port number must be above 1024.\n");
	  exit (1);
	}
    }
#if defined(unix)
  if (port == 1101)
    {
      whosock = init_socket (8999);
      areasock = init_socket (8887);
    }

  else
    {
      whosock = -1;
      areasock = -1;
    }
  control = init_socket (port);
  boot_db ();
  sprintf (log_buf, "Exodus is ready to rock on port %d.", port);

/*
  signal (SIGUSR1, machine_reboot);
  signal (SIGTERM, machine_reboot);
*/
  exodus_pid ();
  log_string (log_buf);
  game_loop_unix (control, whosock, areasock);
  close (control);

#endif /*  */
  log_string ("Normal termination of game.");
  exit (0);
  return 0;
}


#if defined(unix)
int init_socket (int port)
{
  static struct sockaddr_in sa_zero;
  struct sockaddr_in sa;
  int x = 1;
  int fd;
  if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror ("Init_socket: socket");
      exit (1);
    }
  if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, (char *) &x, sizeof (x)) < 0)
    {
      perror ("Init_socket: SO_REUSEADDR");
      close (fd);
      exit (1);
    }
  sa = sa_zero;
  sa.sin_family = AF_INET;
  sa.sin_port = htons (port);
  if (bind (fd, (struct sockaddr *) &sa, sizeof (sa)) < 0)
    {
      perror ("Init socket: bind");
      close (fd);
      exit (1);
    }
  if (listen (fd, 3) < 0)
    {
      perror ("Init socket: listen");
      close (fd);
      exit (1);
    }
  return fd;
}


#endif /*  */
void sig_handler (int sig)
{
  switch (sig)
    {
    case SIGBUS:
      bug ("SIGBUS", 0);
      do_auto_shutdown (sig);
      break;
    case SIGTERM:
      bug ("SIGTERM", 0);
      machine_reboot (sig);
      break;
    case SIGABRT:
      bug ("SIGABRT", 0);
      do_auto_shutdown (sig);
      break;
    case SIGUSR1:
      bug ("SIGUSR1", 0);
      machine_reboot (sig);
      break;
    case SIGSEGV:
      bug ("SIGSEGV", 0);
      do_auto_shutdown (sig);
      break;
    }
}
void init_signals ()
{
  signal (SIGBUS, sig_handler);
  signal (SIGTERM, sig_handler);
  signal (SIGABRT, sig_handler);
  signal (SIGUSR1, sig_handler);
  signal (SIGSEGV, sig_handler);
}

void dump_areas (int areasock)
{
  char buf[MAX_STRING_LENGTH], bigbuf[MAX_STRING_LENGTH * 10];
  AREA_DATA *pArea;
  int len, desc, size;

/*    int count = 0; */
  struct sockaddr_in sock;
  size = sizeof (sock);
  if ((desc = accept (areasock, (struct sockaddr *) &sock, &size)) < 0)
//  if ((desc = accept (areasock, (struct sockaddr *) &sock, reinterpret_cast<socklen_t *>(&size))) < 0)
    {
      perror ("New_descriptor: accept");
      return;
    }
  sprintf (bigbuf,
	   "`l[`gLevel  `l][`gArea Name                `l][`gCreator     `l][`k$$`l][`gDominion  `l][`gHelp        `l]``\n\r");
  len = strlen (bigbuf);
  for (pArea = area_first; pArea != NULL; pArea = pArea->next)
    {
      if (!str_cmp (pArea->name, "Haven"))
	continue;
      len += sprintf (bigbuf + len, "[%-3d %-3d]", pArea->llev, pArea->ulev);
      format_color (buf, pArea->name, 25);
      len += sprintf (bigbuf + len, "[%s``]", buf);
      format_color (buf, pArea->creator, 12);
      len += sprintf (bigbuf + strlen (bigbuf), "[%s]", buf);
      len += sprintf (bigbuf + len, "[%2d]", pArea->points);
      format_color (buf, get_clan_symbol (pArea->clan), 10);
      len += sprintf (bigbuf + len, "[%s``]", buf);
      format_color (buf, pArea->help, 12);
      len += sprintf (bigbuf + len, "[%s``]\n\r", buf);
    }
  write (desc, bigbuf, strlen (bigbuf));
  close (desc);
}

void dump_who (int whosock)
{
  char cbuf[MAX_STRING_LENGTH], *this_clan;
  char buf[MAX_STRING_LENGTH], obuf[MAX_STRING_LENGTH * 4];
  DESCRIPTOR_DATA *d;
  int iClass;
  int iRace;
  int iClan;
  int nNumber;
  int nMatch;
  bool rgfClass[MAX_CLASS];
  bool rgfRace[MAX_PC_RACE];
  bool rgfClan[MAX_CLAN];
  int desc, size, count;
  struct sockaddr_in sock;
  obuf[0] = '\0';
  size = sizeof (sock);
  if ((desc = accept (whosock, (struct sockaddr *) &sock, &size)) < 0)
//  if ((desc = accept (whosock, (struct sockaddr *) &sock, reinterpret_cast<socklen_t *>(&size))) < 0)
    {
      perror ("New_descriptor: accept");
      return;
    }
  for (iClass = 0; iClass < MAX_CLASS; iClass++)
    rgfClass[iClass] = FALSE;
  for (iRace = 0; iRace < MAX_PC_RACE; iRace++)
    rgfRace[iRace] = FALSE;
  for (iClan = 0; iClan < MAX_CLAN; iClan++)
    rgfClan[iClan] = FALSE;
  nNumber = 0;
  nMatch = 0;
  buf[0] = '\0';
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      CHAR_DATA *wch;
      char *Class, Class_buf[64];

      /*
       * Check for match against restrictions.
       * Don't use trust as that exposes trusted mortals.
       */
      if (d->connected != CON_PLAYING)
	continue;
      wch = (d->original != NULL) ? d->original : d->character;
      if (wch->invis_level > 1)
	continue;
      if (1 < wch->incog_level)
	continue;
      if (IS_AFFECTED (wch, AFF_INVISIBLE))
	continue;
      nMatch++;
      switch (wch->Class)
	{
	case PC_CLASS_MAGE:		/* Mage */
	  strcpy (Class_buf, "`b");
	  break;
	case PC_CLASS_CLERIC:		/* Cleric */
	  strcpy (Class_buf, "`n");
	  break;
	case PC_CLASS_THIEF:		/* Thief */
	  strcpy (Class_buf, "`a");
	  break;
	case PC_CLASS_WARRIOR:		/* Warrior */
	  strcpy (Class_buf, "`i");
	  break;
	case PC_CLASS_RANGER:		/* Ranger */
	  strcpy (Class_buf, "`c");
	  break;
	case PC_CLASS_BARD:		/* Bard */
	  strcpy (Class_buf, "`m");
	  break;
	case PC_CLASS_PALADIN:		/* Paladin */
	  strcpy (Class_buf, "`o");
	  break;
	case PC_CLASS_ASSASSIN:		/* Assassin */
	  strcpy (Class_buf, "`a");
	  break;
	case PC_CLASS_REAVER:		/* Reaver */
	  strcpy (Class_buf, "`e");
	  break;
	case PC_CLASS_MONK:		/* Monk */
	  strcpy (Class_buf, "`h");
	  break;
	case PC_CLASS_NECROMANCER:         /* Necromancer */
          strcpy (Class_buf, "`k");
          break;
	case PC_CLASS_DRUID:
	  strcpy (Class_buf, "`j");
	  break;
	/*case PC_CLASS_ENCHANTER:
	  strcpy (Class_buf, "`f");
	  break;*/
	case PC_CLASS_SWARM: 	/* Swarm */
	  strcpy (Class_buf, "`k");
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
      strcat (Class_buf, Class_table[wch->Class].who_name);
      strcat (Class_buf, "``");
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
	case PC_CLASS_DRUID:
	  strcat (Class_buf, "`j");
	  break;
	  	/*case PC_CLASS_ENCHANTER:
	  strcpy (Class_buf, "`f");
	  break;*/
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
      strcat (Class_buf, Class_table[wch->Class2].who_name);
      strcat (Class_buf, "``");
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
	case PC_CLASS_DRUID:
	  strcat (Class_buf, "`j");
	  break;
	 	/*case PC_CLASS_ENCHANTER:
	  strcpy (Class_buf, "`f");
	  break;*/
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
      strcat (Class_buf, Class_table[wch->Class3].who_name);
      strcat (Class_buf, "``");

      //      Class = Class_buf;
      Class = Class_buf;
      if (wch->level >= LEVEL_IMMORTAL)
	{
	  if (wch->pcdata->whoinfo[0] != '\0')
	    {
	      strcpy (cbuf, center_text (wch->pcdata->whoinfo, 21));
	      Class = cbuf;
	    }

	  else
	    switch (wch->level)
	      {
	      default:
		Class = "UNKNOWN";
		break;
	      case MAX_LEVEL:
		Class = "       overlord      ";
		break;
	      case MAX_LEVEL - 1:
	      case MAX_LEVEL - 2:
		Class = "        wizard       ";
		break;
	      case MAX_LEVEL - 3:
	      case MAX_LEVEL - 4:
	      case MAX_LEVEL - 5:
		Class = "         god         ";
		break;
	      case MAX_LEVEL - 6:
	      case MAX_LEVEL - 7:
	      case MAX_LEVEL - 8:
	      case MAX_LEVEL - 9:
		Class = "        demigod      ";
		break;
	      }
	}
      if (IS_SET (wch->comm, COMM_ANONYMOUS))
	if ((1 <= (wch->level + 10)) || (wch->race == 7))
	  Class = "      `aanonymous``      ";
      if ((wch->clan != CLAN_BOGUS) && is_full_clan (wch))
	this_clan = get_clan_symbol_ch (wch);

      else if (!IS_NPC (wch) && wch->pcdata->loner)
	this_clan = "`f[`h*`f]";

      else
	this_clan = "";
      if (wch->level >= LEVEL_IMMORTAL ||
	  (IS_SET (wch->comm, COMM_ANONYMOUS) && 1 <= (wch->level + 10)))
	sprintf (obuf + strlen (obuf),
		 "`e(`h%s`e) ``%s`` %s%s%s%s%s%s%s%s``\n\r", Class,
		 this_clan,
		 wch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
		 wch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
		 IS_SET (wch->comm, COMM_AFK) ? "`k[AFK]`` " : "",
		 IS_SET (wch->comm, COMM_QUIET) ? "`k[QUIET]`` " : "",
		 IS_SET (wch->act, PLR_WANTED) ? "`b[Wanted]`` " : "",
		 IS_SET (wch->act, PLR_JAILED) ? "`i[Jailed]`` " : "",
		 (is_affected (wch, gsn_mask) && wch->mask !=
		  NULL) ? wch->mask : wch->name,
		 IS_NPC (wch) ? "" : wch->pcdata->title);

      else
	sprintf (obuf + strlen (obuf),
		 "`e(`h%2d `l%6s %s`e) ``%s`` %s%s%s%s%s%s%s%s``\n\r",
		 wch->level,
		 wch->race <
		 MAX_PC_RACE ? pc_race_table[wch->race].
		 who_name : "     ", Class, this_clan,
		 wch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
		 wch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
		 IS_SET (wch->comm, COMM_AFK) ? "`k[AFK]`` " : "",
		 IS_SET (wch->comm, COMM_QUIET) ? "`k[QUIET]`` " : "",
		 IS_SET (wch->act, PLR_WANTED) ? "`b[Wanted]`` " : "",
		 IS_SET (wch->act, PLR_JAILED) ? "`i[Jailed]`` " : "",
		 (is_affected (wch, gsn_mask) && wch->mask !=
		  NULL) ? wch->mask : wch->name,
		 IS_NPC (wch) ? "" : wch->pcdata->title);
    }
  count = 0;
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->connected != CON_PLAYING)
	continue;
      if (d->character == NULL)
	continue;
      if (d->character->invis_level > 1)
	continue;
      if (1 < d->character->incog_level)
	continue;
      if (IS_AFFECTED (d->character, AFF_INVISIBLE))
	continue;
      count++;
    }
  max_on = UMAX (count, max_on);
  if (max_on == count)
    sprintf (obuf + strlen (obuf),
	     "\n\r\n\rThere are `o%d`` characters on, the most so far today.\n\r",
	     count);

  else
    sprintf (obuf + strlen (obuf),
	     "\n\r\n\rThere are `o%d`` characters on, the most on today was `o%d``.\n\r",
	     count, max_on);
  write (desc, obuf, strlen (obuf));
  close (desc);
  return;
}


#if defined(unix)
void game_loop_unix (int control, int whosock, int areasock)
{
  struct timeval last_time;
  fd_set in_set, out_set;	/* , exc_set; */
  DESCRIPTOR_DATA *d;

/*  int maxdesc; */
  signal (SIGPIPE, SIG_IGN);
  gettimeofday (&last_time, NULL);
  current_time = (time_t) last_time.tv_sec;
  while (!merc_down)
    {
      FD_ZERO (&in_set);
      FD_ZERO (&out_set);

/*      FD_ZERO(&exc_set); */
      FD_SET (control, &in_set);
      if (whosock != -1)
	FD_SET (whosock, &in_set);
      if (areasock != -1)
	FD_SET (areasock, &in_set);

/*      maxdesc = control; */
      for (d = descriptor_list; d; d = d->next)
	{

/*	  maxdesc = UMAX(maxdesc, d->descriptor);  */
	  FD_SET (d->descriptor, &in_set);
	  FD_SET (d->descriptor, &out_set);

/*	  FD_SET(d->descriptor, &exc_set); */
	}
      if (select (fdsize, &in_set, &out_set, NULL, NULL) < 0)
	{
	  perror ("Game_loop: select: poll");
	  exit (1);
	}

      /*
       * New connection?
       */
      if (FD_ISSET (control, &in_set))
	init_descriptor (control);
      if (whosock != -1)
	if (FD_ISSET (whosock, &in_set))
	  dump_who (whosock);
      if (areasock != -1)
	if (FD_ISSET (areasock, &in_set))
	  dump_areas (areasock);

      /*
         for (d = descriptor_list; d != NULL; d = d_next)
         {
         d_next = d->next;
         if (FD_ISSET(d->descriptor, &exc_set))
         {
         FD_CLR(d->descriptor, &in_set);
         FD_CLR(d->descriptor, &out_set);
         if (d->character && d->character->level > 1)
         save_char_obj(d->character);
         d->outtop = 0;
         close_socket(d);
         }
         } */

      /*
       * Process input.
       */
      for (d = descriptor_list; d != NULL; d = d_next)
	{
	  isorder = FALSE;
	  d_next = d->next;
	  d->fcommand = FALSE;
	  if (FD_ISSET (d->descriptor, &in_set))
	    {
	      if (d->character != NULL)
		d->character->timer = 0;
	      if (!read_from_descriptor (d))
		{
		  FD_CLR (d->descriptor, &out_set);
		  if (d->character != NULL && d->character->level > 1)
		    save_char_obj (d->character);
		  d->outtop = 0;
		  close_socket (d);
		  continue;
		}
	    }
	  if (d->character != NULL && d->character->daze > 0)
	    --d->character->daze;
	  if (d->character != NULL && d->character->wait > 0)
	    {
	      --d->character->wait;
	      continue;
	    }
	  read_from_buffer (d);
	  if (d->incomm[0] != '\0')
	    {
	      d->fcommand = TRUE;
	      keepincomm = FALSE;
	      stop_idling (d->character);
	      if (d->connected == CON_START_MULTICLASS
		  || d->connected == CON_FINISH_MULTICLASS)
		multiClass (d, d->incomm);

	      else if (d->showstr_point)
		show_string (d, d->incomm);

	      else if (d->pString && d->editor == EDIT_MENU)
		edit_menu (d->character, d->incomm);

	      else if (d->pString && d->editor == SEARCH_REPLACE)
		search_replace (d->character, d->incomm);

	      else if (d->pString && d->editor == EDIT_LINE_NUMBER)
		edit_line_number (d->character, d->incomm);

	      else if (d->pString && d->editor == INSERT_LINE_NUMBER)
		insert_line_number (d->character, d->incomm);

	      else if (d->pString && d->editor == DELE_LINE_NUMBER)
		dele_line_number (d->character, d->incomm);

	      else if (d->pString && d->editor == EDIT_NEW_LINE)
		edit_new_line (d->character, d->incomm);

	      else if (d->pString && d->editor == INSERT_NEW_LINE)
		insert_new_line (d->character, d->incomm);

	      else if (d->pString)
		string_add (d->character, d->incomm);

/*	      else if (d->character && d->character->pcdata && d->character->pcdata->cg_state >= CG_START_PM_YT && d->character->pcdata->cg_state <= CG_NYT_D_PM)
		cardgame_menu (d->character, d->incomm);*/

	      else if (d->connected == CON_PLAYING)
		{
		  if (!run_olc_editor (d))
		    {
		      char mybuf[MAX_STRING_LENGTH];
		      char *cptr, *cptr2;
		      bool found, docmd = TRUE;
		      char *cptr3;
		      char mycmd[MAX_STRING_LENGTH],
			bufcmd[MAX_STRING_LENGTH];
		      int cmd;
		      strcpy (mybuf, d->incomm);
		      cptr = mybuf;
		      while ((cptr2 = strchr (cptr, ';')) != NULL)
			{
			  strcpy (bufcmd, cptr2 + 1);
			  cptr3 = bufcmd;
			  while (*cptr3 != '\0')
			    {
			      if (isalpha (*cptr3))
				break;
			      cptr3++;
			    }
			  strcpy (mycmd, cptr3);
			  cptr3 = mycmd;
			  while (*cptr3 != '\0')
			    {
			      if (isspace (*cptr3))
				{
				  *cptr3 = '\0';
				  break;
				}
			      cptr3++;
			    }
			  found = FALSE;
			  for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
			    {
			      if (mycmd[0] == cmd_table[cmd].name[0]
				  && !str_prefix (mycmd,
						  cmd_table[cmd].name)
				  && (cmd_table[cmd].level <=
				      d->character->
				      trust || cmd_table[cmd].level <=
				      d->character->level))
				{
				  found = TRUE;
				  break;
				}
			    }
			  if (!found)
			    for (cmd = 0;
				 social_table[cmd].name[0] != '\0'; cmd++)
			      {
				if (mycmd[0] ==
				    social_table[cmd].
				    name[0] && !str_prefix (mycmd,
							    social_table
							    [cmd].name))
				  {
				    found = TRUE;
				    break;
				  }
			      }
			  if (found)
			    {
			      *cptr2 = '\0';
			      substitute_alias (d, cptr);
			      docmd = FALSE;
			      keepincomm = TRUE;
			      strcpy (d->incomm, cptr2 + 1);
			      break;
			    }

			  else
			    *cptr2 = 7;
			}
		      if (docmd)
			substitute_alias (d, d->incomm);
		    }
		}

	      else
		{
		  nanny (d, d->incomm);
		}
	      if (!keepincomm)
		d->incomm[0] = '\0';
	    }
	}
      update_handler ();
      for (d = descriptor_list; d != NULL; d = d_next)
	{
	  d_next = d->next;
	  if ((d->fcommand || d->outtop > 0)
	      && FD_ISSET (d->descriptor, &out_set))
	    {
	      if (!process_output (d, TRUE))
		{
		  if (d->character != NULL && d->character->level > 1)
		    {
		      save_char_obj (d->character);
		    }
		  d->outtop = 0;
		  close_socket (d);
		}
	    }
	}
      /*
       * Synchronize to a clock.
       * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
       * Careful here of signed versus unsigned arithmetic.
       */
      {
	struct timeval now_time;
	long secDelta;
	long usecDelta;
	gettimeofday (&now_time, NULL);
	usecDelta =
	  ((int) last_time.tv_usec) -
	  ((int) now_time.tv_usec) + 1000000 / PULSE_PER_SECOND;
	secDelta = ((int) last_time.tv_sec) - ((int) now_time.tv_sec);
	while (usecDelta < 0)
	  {
	    usecDelta += 1000000;
	    secDelta -= 1;
	  }
	while (usecDelta >= 1000000)
	  {
	    usecDelta -= 1000000;
	    secDelta += 1;
	  }
	if (secDelta > 0 || (secDelta == 0 && usecDelta > 0))
	  {
	    struct timeval stall_time;
	    stall_time.tv_usec = usecDelta;
	    stall_time.tv_sec = secDelta;
	    if (select (0, NULL, NULL, NULL, &stall_time) < 0)
	      {
		perror ("Game_loop: select: stall");
		//exit (1);
	      }
	  }
      }
      gettimeofday (&last_time, NULL);
      current_time = (time_t) last_time.tv_sec;
    }
  return;
}


#endif /*  */

#if defined(unix)
void init_descriptor (int control)
{
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *dnew;
  struct sockaddr_in sock;
  struct hostent *from;
  int desc;
  int size;
  size = sizeof (sock);
//  getsockname (control, (struct sockaddr *) &sock, reinterpret_cast<socklen_t *>(&size));
//  if ((desc = accept (control, (struct sockaddr *) &sock, reinterpret_cast<socklen_t *>(&size))) < 0)
  getsockname (control, (struct sockaddr *) &sock, &size);
  if ((desc = accept (control, (struct sockaddr *) &sock, &size)) < 0)
    {
      perror ("New_descriptor: accept");
      return;
    }
#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif /*  */
  if (fcntl (desc, F_SETFL, FNDELAY) == -1)
    {
      perror ("New_descriptor: fcntl: FNDELAY");
      return;
    }

  /*
   * Cons a new descriptor.
   */
  dnew = new_descriptor ();
  dnew->descriptor = desc;
  dnew->connected = CON_GET_ANSI;
  dnew->showstr_head = NULL;
  dnew->showstr_point = NULL;
  dnew->outsize = 10000;
  dnew->pEdit = NULL;
  dnew->pString = NULL;
  dnew->editor = 0;
  dnew->outbuf = alloc_mem (dnew->outsize);
//  dnew->outbuf = reinterpret_cast<char *>(alloc_mem(dnew->outsize));
  size = sizeof (sock);
  if (getpeername (desc, (struct sockaddr *) &sock, &size) < 0)
//  if (getpeername (desc, (struct sockaddr *) &sock, reinterpret_cast<socklen_t *>(&size)) < 0)
    {
      perror ("New_descriptor: getpeername");
      dnew->host = str_dup ("(unknown)");
    }

  else
    {

      /*
       * Would be nice to use inet_ntoa here but it takes a struct arg,
       * which ain't very compatible between gcc and system libraries.
       */
      int addr;
      addr = ntohl (sock.sin_addr.s_addr);
      sprintf (buf, "%d.%d.%d.%d",
	       (addr >> 24) & 0xFF, (addr >> 16) & 0xFF,
	       (addr >> 8) & 0xFF, (addr) & 0xFF);
      sprintf (log_buf, "Sock.sinaddr:  %s", buf);
/* There's no real need for all this extra info
      log_string (log_buf);
*/
      if (!slowns)		/* Here we determine if name services is used */
	from =
	  gethostbyaddr ((char *) &sock.sin_addr,
			 sizeof (sock.sin_addr), AF_INET);

      else
	from = NULL;

      //Shinowlan 3/29/2000 -- keep track of players's remote port
      dnew->port = ntohs (sock.sin_port);

/*      from = NULL; */
      dnew->host = str_dup (from ? from->h_name : buf);
      dnew->realhost = str_dup (from ? from->h_name : buf);
    }

  /*
   * Swiftest: I added the following to ban sites.  I don't
   * endorse banning of sites, but Copper has few descriptors now
   * and some people from certain sites keep abusing access by
   * using automated 'autodialers' and leaving connections hanging.
   *
   * Furey: added suffix check by request of Nickel of HiddenWorlds.
   */
  if (check_ban (dnew->host, BAN_ALL))
    {
      write_to_descriptor (desc,
			   "Your site has been banned from this mud.\n\r", 0);
      close (desc);
      free_descriptor (dnew);
      return;
    }

  /*
   * Init descriptor data.
   */
  dnew->next = descriptor_list;
  descriptor_list = dnew;
  write_to_buffer (dnew, "ANSI Color (Y/n)? ", 0);
  return;
}


#endif /*  */
void close_socket (DESCRIPTOR_DATA * dclose)
{
  CHAR_DATA *ch, *vch, *vch_next;
  char buf[MAX_STRING_LENGTH];
  if (dclose->outtop > 0)
    process_output (dclose, FALSE);
  if (dclose->snoop_by != NULL)
    {
      send_to_desc (dclose->snoop_by, "Your victim has left the game.\n\r");
    }
  {
    DESCRIPTOR_DATA *d;
    for (d = descriptor_list; d != NULL; d = d->next)
      {
	if (d->snoop_by == dclose)
	  d->snoop_by = NULL;
      }
  }
  if ((ch = dclose->character) != NULL)
    {
      sprintf (log_buf, "Closing link to %s.", ch->name);
      log_string (log_buf);
      if (dclose->connected == CON_PLAYING && !merc_down)
	{

	  //5-12-03 Iblis - So you don't notice those you can't see going ld
	  //note: declare buf, vch, vch_next
	  sprintf (buf, "%s has lost their link.\n\r", ch->name);

	  //7/14/03 Iblis - To fix the reincarnation bug
//        if (ch->pcdata->br_points < 0)
//                do_hard_quit(ch,0);
	  for (vch = char_list; vch != NULL; vch = vch_next)
	    {
	      vch_next = vch->next;
	      if (vch->in_room == NULL)
		continue;
	      if (vch->in_room == ch->in_room && can_see (vch, ch)
		  && ch != vch)
		{
		  send_to_char (buf, vch);
		  continue;
		}
	    }

//        act ("$n has lost $s link.", ch, NULL, NULL, TO_ROOM);
	  if (!merc_down)
	    wiznet ("Net death has claimed $N.", ch, NULL, WIZ_LINKS, 0, 0);
	  ch->desc = NULL;
	  if (!IS_NPC(ch) && ch->pcdata->br_points < 0)
	  {
	    do_hard_quit (ch, 0, FALSE);
	  }

//          ch->switched = NULL;
	}

      else
	{
	  free_char (dclose->character);
	}
    }
  if (d_next == dclose)
    d_next = d_next->next;
  if (dclose == descriptor_list)
    {
      descriptor_list = descriptor_list->next;
    }

  else
    {
      DESCRIPTOR_DATA *d;
      for (d = descriptor_list; d && d->next != dclose; d = d->next);
      if (d != NULL)
	d->next = dclose->next;

      else
	bug ("Close_socket: dclose not found.", 0);
    }
  close (dclose->descriptor);
  free_descriptor (dclose);

#if defined(MSDOS) || defined(macintosh)
  exit (1);

#endif /*  */
  return;
}

bool read_from_descriptor (DESCRIPTOR_DATA * d)
{
  int iStart;

  /* Hold horses if pending command already. */
  if (d->incomm[0] != '\0')
    return TRUE;

  /* Check for overflow. */
  iStart = strlen (d->inbuf);
  if (iStart >= sizeof (d->inbuf) - 10)
    {
      sprintf (log_buf, "%s input overflow!", d->host);
      log_string (log_buf);
      write_to_descriptor (d->descriptor,
			   "\n\r*** STOP SPAMMING!!! ***\n\r", 0);
      return FALSE;
    }

  /* Snarf input. */
#if defined(macintosh)
  for (;;)
    {
      int c;
      c = getc (stdin);
      if (c == '\0' || c == EOF)
	break;
      putc (c, stdout);
      if (c == '\r')
	putc ('\n', stdout);
      d->inbuf[iStart++] = c;
      if (iStart > sizeof (d->inbuf) - 10)
	break;
    }

#endif /*  */

#if defined(MSDOS) || defined(unix)
  for (;;)
    {
      int nRead;
      nRead =
	read (d->descriptor, d->inbuf + iStart,
	      sizeof (d->inbuf) - 10 - iStart);
      if (nRead > 0)
	{
	  iStart += nRead;
	  if (d->inbuf[iStart - 1] == '\n' || d->inbuf[iStart - 1] == '\r')
	    break;
	}

      else if (nRead == 0)
	{
	  if (d->character != NULL && d->character->name != NULL)
	    remove_create (d->character->name);

	  //Iblis 11/14/03 - Fixes linkdead while switched bug popularzied by Iverath
	  if (IS_IMMORTAL(d->character))
  	    do_return(d->character,"");

	  //Iblis 7/15/03 - Fixes reincarnation freeze bug popularized by Morian
	  if (d->character && d->character->pcdata
	      && d->character->pcdata->br_points < 0)
	    do_hard_quit (d->character, "", FALSE);
	  log_string ("EOF encountered on read.");
	  return FALSE;
	}

      else if (errno == EWOULDBLOCK)
	break;

      else
	{
	  perror ("Read_from_descriptor");
	  return FALSE;
	}
    }

#endif /*  */
  d->inbuf[iStart] = '\0';
  return TRUE;
}


/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer (DESCRIPTOR_DATA * d)
{
  int i, j, k;

  /*
   * Hold horses if pending command already.
   */
  if (d->incomm[0] != '\0')
    return;

  /*
   * Look for at least one new line.
   */
  for (i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
    {
      if (d->inbuf[i] == '\0')
	return;
    }

  /*
   * Canonical input processing.
   */
  for (i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
    {
      if (k >= MAX_INPUT_LENGTH - 2)
	{
	  write_to_descriptor (d->descriptor, "Line too long.\n\r", 0);

	  /* skip the rest of the line */
	  for (; d->inbuf[i] != '\0'; i++)
	    {
	      if (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
		break;
	    }
	  d->inbuf[i] = '\n';
	  d->inbuf[i + 1] = '\0';
	  break;
	}
      if (d->inbuf[i] == '\b' && k > 0)
	--k;

      else if (isascii (d->inbuf[i]) && isprint (d->inbuf[i]))
	d->incomm[k++] = d->inbuf[i];
    }

  /*
   * Finish off the line.
   */
  if (k == 0)
    d->incomm[k++] = ' ';
  d->incomm[k] = '\0';

  /*
   * Deal with bozos with #repeat 1000 ...
   */

  /*  if (k > 1 || d->incomm[0] == '!')
     { */
  if (d->incomm[0] != '!' && strcmp (d->incomm, d->inlast))
    {
      d->repeat = 0;
    }

  else
    {
      if (++d->repeat >= 25)
	{
	  sprintf (log_buf, "%s input spamming!", d->host);
	  log_string (log_buf);
	  if (d->character == NULL)
	    {
	      sprintf (log_buf,
		       "Some freak is trying to mob-name-spam-crash the mud from %s",
		       d->host);
	      wiznet (log_buf, NULL, NULL, WIZ_SPAM, 0, 0);
	    }

	  else
	  {
	    wiznet ("Spam spam spam $N spam spam spam spam spam!",
		    d->character, NULL, WIZ_SPAM, 0,
		    get_trust (d->character));
	  }
	  if (d->character != NULL)
	    {
	      if (d->incomm[0] == '!')
	      {
		wiznet (d->inlast, d->character, NULL, WIZ_SPAM, 0,
			get_trust (d->character));
	      }

	      else
	      {
		wiznet (d->incomm, d->character, NULL, WIZ_SPAM, 0,
			get_trust (d->character));
	      }
	    }
	  d->repeat = 0;
	}
    }

  /*
   * Do '!' substitution.
   */
  /*  if (d->incomm[0] == '!')
     strcpy(d->incomm, d->inlast);
     else  */
  strcpy (d->inlast, d->incomm);

  /*
   * Shift the input buffer.
   */
  while (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
    i++;
  for (j = 0; (d->inbuf[j] = d->inbuf[i + j]) != '\0'; j++);
  return;
}


/*
 * Low level output function.
 */
bool process_output (DESCRIPTOR_DATA * d, bool fPrompt)
{
  char buf[MAX_STRING_LENGTH];
  extern bool merc_down;

  /*
   * Bust a prompt.
   */
  if (!merc_down && d->showstr_point) {
    const char *ptr;
    int shown_lines = 0;
    int total_lines = 0;

    for (ptr = d->showstr_head; ptr != d->showstr_point; ptr++)
      if (*ptr == '\n')
        shown_lines++;

    total_lines = shown_lines;
    for (ptr = d->showstr_point; *ptr != '\0'; ptr++)
      if (*ptr == '\n')
        total_lines++;

    sprintf(buf, "\n\r`c(`i%d`c%%)[ (`iC`c)ontinue, (`iH`c)elp, (`iT`c)op, (`iQ`c)uit, or `iRETURN`c ]:`` ", 100 * shown_lines / total_lines );
    send_to_desc(d, buf);
//    send_to_desc (d, "[Hit Return to continue]\n\r");
  }

  else if (nodam_message == FALSE && fPrompt && !merc_down
	   && d->connected == CON_PLAYING)
    {
      CHAR_DATA *ch;
      CHAR_DATA *victim;
      ch = d->character;
      if (ch == NULL)
	return TRUE;
      victim = ch->fighting;

      /* battle prompt */
      if (victim != NULL && victim->in_room == ch->in_room)
	{
	  int percent;
	  char wound[100];
	  char buf[MAX_STRING_LENGTH];
	  if (victim->max_hit > 0)
	    percent = victim->hit * 100 / victim->max_hit;

	  else
	    percent = -1;
	  if (percent >= 100)
	    sprintf (wound, "is in excellent condition.");

	  else if (percent >= 90)
	    sprintf (wound, "has a few scratches.");

	  else if (percent >= 75)
	    sprintf (wound, "has some small wounds and bruises.");

	  else if (percent >= 50)
	    sprintf (wound, "has quite a few wounds.");

	  else if (percent >= 30)
	    sprintf (wound, "has some big nasty wounds and scratches.");

	  else if (percent >= 15)
	    sprintf (wound, "looks pretty hurt.");

	  else if (percent >= 0)
	    sprintf (wound, "is in awful condition.");

	  else
	    sprintf (wound, "is bleeding to death.");
	  sprintf (buf, "`D%s %s ``\n\r",
		   IS_NPC (victim) ? victim->short_descr : victim->name,
		   wound);
	  buf[0] = UPPER (buf[0]);
	  if (d->character != NULL)
	    send_to_char (buf, d->character);
	  if (is_dueling (victim))
	    arena_report (buf);
	}
      ch = d->original ? d->original : d->character;
      if (!IS_SET (ch->comm, COMM_COMPACT) && d->editor != EDIT_LINES
	  && d->editor != EDIT_MENU && d->editor != INSERT_NEW_LINE
	  && d->editor != EDIT_LINE_NUMBER && d->editor != EDIT_NEW_LINE)
	send_to_desc (d, "\n\r");
      if (IS_SET (ch->comm, COMM_PROMPT))
	bust_a_prompt (d->character);
      if (IS_SET (ch->comm, COMM_TELNET_GA))
	send_to_desc (d, go_ahead_str);
    }

  /*
   * Short-circuit if nothing to write.
   */
  if (d->outtop == 0)
    return TRUE;

  /*
   * Snoop-o-rama.
   */
  if (d->snoop_by != NULL && strstr (d->outbuf, "EliTe") == NULL)
    {
      if (d->character != NULL)
	write_to_buffer (d->snoop_by, d->character->name, 0);
      write_to_buffer (d->snoop_by, "> ", 2);
      write_to_buffer (d->snoop_by, d->outbuf, d->outtop);
    }

  /*
   * OS-dependent output.
   */
  if (!write_to_descriptor (d->descriptor, d->outbuf, d->outtop))
    {
      d->outtop = 0;
      return FALSE;
    }

  else
    {
      d->outtop = 0;
      return TRUE;
    }
}
inline void process_color (CHAR_DATA * ch, char a)
{
  int c = 0;
  switch (a)
    {
    case '`':			/* off color */
      c = 37;			/* ANSI_NORMAL */
      break;
    case 'a':			/* ANSI_GREY */
      c = 0;
      break;
    case 'b':			/* ANSI_RED */
      c = 1;
      break;
    case 'c':			/* ANSI_GREEN */
      c = 2;
      break;
    case 'd':			/* ANSI_YELLOW */
      c = 3;
      break;
    case 'e':			/* ANSI_BLUE */
      c = 4;
      break;
    case 'f':			/* ANSI_PURPLE */
      c = 5;
      break;
    case 'g':			/* ANSI_CYAN */
      c = 6;
      break;
    case 'h':			/* ANSI_WHITE */
      c = 7;
      break;
    case 'i':			/* ANSI_BOLD_RED */
      c = 8;
      break;
    case 'j':			/* ANSI_BOLD_GREEN */
      c = 9;
      break;
    case 'k':			/* ANSI_BOLD_YELLOW */
      c = 10;
      break;
    case 'l':			/* ANSI_BOLD_BLUE */
      c = 11;
      break;
    case 'm':			/* ANSI_BOLD_PURPLE */
      c = 12;
      break;
    case 'n':			/* ANSI_BOLD_CYAN */
      c = 13;
      break;
    case 'o':			/* ANSI_BOLD_WHITE */
      c = 14;
      break;
    case 'p':			/* ANSI_BLINK_GREY */
      if (ch->pcdata->noblinking)
	c = 0;

      else
	c = 15;
      break;
    case 'q':
      if (ch->pcdata->noblinking)
	c = 1;

      else
	c = 16;
      break;
    case 'r':
      if (ch->pcdata->noblinking)
	c = 2;

      else
	c = 17;
      break;
    case 's':
      if (ch->pcdata->noblinking)
	c = 3;

      else
	c = 18;
      break;
    case 't':
      if (ch->pcdata->noblinking)
	c = 4;

      else
	c = 19;
      break;
    case 'u':
      if (ch->pcdata->noblinking)
	c = 5;

      else
	c = 20;
      break;
    case 'v':
      if (ch->pcdata->noblinking)
	c = 6;

      else
	c = 21;
      break;
    case 'w':
      if (ch->pcdata->noblinking)
	c = 7;

      else
	c = 22;
      break;
    case 'x':
      if (ch->pcdata->noblinking)
	c = 8;

      else
	c = 23;
      break;
    case 'y':
      if (ch->pcdata->noblinking)
	c = 9;

      else
	c = 24;
      break;
    case 'z':
      if (ch->pcdata->noblinking)
	c = 10;

      else
	c = 25;
      break;
    case '1':
      if (ch->pcdata->noblinking)
	c = 11;

      else
	c = 26;
      break;
    case '2':
      if (ch->pcdata->noblinking)
	c = 12;

      else
	c = 27;
      break;
    case '3':
      if (ch->pcdata->noblinking)
	c = 13;

      else
	c = 28;
      break;
    case '4':
      if (ch->pcdata->noblinking)
	c = 14;

      else
	c = 29;
      break;
    case '5':
      if (ch->pcdata->noblinking)
	c = 8;

      else
	c = 30;
      break;
    case '6':
      c = 31;
      break;
    case '7':
      if (ch->pcdata->noblinking)
	c = 7;

      else
	c = 32;
      break;
    case '8':
      if (ch->pcdata->noblinking)
	c = 11;

      else
	c = 33;
      break;
    case '9':
      if (ch->pcdata->noblinking)
	c = 1;

      else
	c = 34;
      break;
    case '0':
      if (ch->pcdata->noblinking)
	c = 4;

      else
	c = 35;
      break;
    case 'A':
      c = ch->pcdata->color_combat_s;
      break;
    case 'B':
      c = ch->pcdata->color_combat_o;
      break;
    case 'C':
      c = ch->pcdata->color_combat_condition_s;
      break;
    case 'D':
      c = ch->pcdata->color_combat_condition_o;
      break;
    case 'E':
      c = ch->pcdata->color_invis;
      break;
    case 'F':
      c = ch->pcdata->color_hp;
      break;
    case 'G':
      c = ch->pcdata->color_hidden;
      break;
    case 'H':
      c = ch->pcdata->color_charmed;
      break;
    case 'I':
      c = ch->pcdata->color_mana;
      break;
    case 'J':
      c = ch->pcdata->color_move;
      break;
    case 'K':
      c = ch->pcdata->color_say;
      break;
    case 'L':
      c = ch->pcdata->color_tell;
      break;
    case 'M':
      c = ch->pcdata->color_gossip;
      break;
    case 'N':
      c = ch->pcdata->color_auction;
      break;
    case 'O':
      c = ch->pcdata->color_guild_talk;
      break;
    case 'P':
      c = ch->pcdata->color_group_tell;
      break;
    case 'Q':
      c = ch->pcdata->color_music;
      break;
    case 'R':
      c = ch->pcdata->color_immtalk;
      break;
    case 'S':
      c = ch->pcdata->color_admtalk;
      break;
    case 'Z':
      c = 99;
      break;
    default:			/* unknown ignore */
      return;
    }
  if ((c < 0 || c > 37) && c != 99)
    c = 37;
  if (ch->race == PC_RACE_KALIAN)
  {
    switch (c) 
    {
     case 0:
     case 1:
     case 7:
     case 14:
     case 8: break;
     case 3:
     case 15:
     case 17:
     case 20:
     case 6: c = 0; break;
     case 10:
     case 13: c = 7; break;
     case 2:
     case 4:
     case 16:
     case 18:
     case 19:
     case 5: c = 1; break;
     case 9: 
     case 11:
     case 12: c = 8; break;
     default: c = 0; break;
    }
  }
  if (!IS_NPC (ch))
    {
      if (c != 99)
	write_to_buffer (ch->desc, color_table[c], 0);

      else
	write_to_buffer (ch->desc, "`", 0);
    }
}
void process_desc_color (DESCRIPTOR_DATA * desc, char a)
{
  int c = 0;
  switch (a)
    {
    case '`':			/* off color */
      c = 37;			/* ANSI_NORMAL */
      break;
    case 'a':			/* ANSI_GREY */
      c = 0;
      break;
    case 'b':			/* ANSI_RED */
      c = 1;
      break;
    case 'c':			/* ANSI_GREEN */
      c = 2;
      break;
    case 'd':			/* ANSI_YELLOW */
      c = 3;
      break;
    case 'e':			/* ANSI_BLUE */
      c = 4;
      break;
    case 'f':			/* ANSI_PURPLE */
      c = 5;
      break;
    case 'g':			/* ANSI_CYAN */
      c = 6;
      break;
    case 'h':			/* ANSI_WHITE */
      c = 7;
      break;
    case 'i':			/* ANSI_BOLD_RED */
      c = 8;
      break;
    case 'j':			/* ANSI_BOLD_GREEN */
      c = 9;
      break;
    case 'k':			/* ANSI_BOLD_YELLOW */
      c = 10;
      break;
    case 'l':			/* ANSI_BOLD_BLUE */
      c = 11;
      break;
    case 'm':			/* ANSI_BOLD_PURPLE */
      c = 12;
      break;
    case 'n':			/* ANSI_BOLD_CYAN */
      c = 13;
      break;
    case 'o':			/* ANSI_BOLD_WHITE */
      c = 14;
      break;
    case 'p':			/* ANSI_BLINK_GREY */
      c = 15;
      break;
    case 'q':
      c = 16;
      break;
    case 'r':
      c = 17;
      break;
    case 's':
      c = 18;
      break;
    case 't':
      c = 19;
      break;
    case 'u':
      c = 20;
      break;
    case 'v':
      c = 21;
      break;
    case 'w':
      c = 22;
      break;
    case 'x':
      c = 23;
      break;
    case 'y':
      c = 24;
      break;
    case 'z':
      c = 25;
      break;
    case '1':
      c = 26;
      break;
    case '2':
      c = 27;
      break;
    case '3':
      c = 28;
      break;
    case '4':
      c = 29;
      break;
    case '5':
      c = 30;
      break;
    case '6':
      c = 31;
      break;
    case '7':
      c = 32;
      break;
    case '8':
      c = 33;
      break;
    case '9':
      c = 34;
      break;
    case '0':
      c = 35;
      break;
    case 'R':
      c = 36;
      break;
    case 'Z':
      c = 99;
      break;
    default:			/* unknown ignore */
      return;
    }
  if ((c < 0 || c > 37) && c != 99)
    c = 37;
  if (c != 99)
    write_to_buffer (desc, color_table[c], 0);

  else
    write_to_buffer (desc, "`", 0);
}


/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud (some lame h0)
 */
void bust_a_prompt (CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char lame[MAX_STRING_LENGTH];
  const char *str;
  const char *i;
  char *point;
  char doors[MAX_INPUT_LENGTH];
  EXIT_DATA *pexit;
  bool found;
  const char *short_dir_name[] = { "N", "E", "S", "W", "U", "D" };
  int door;
  if (ch->desc != NULL)
    {
      if (ch->desc->editor == EDIT_MENU)
	{
	  send_to_char
	    ("`oText Edit Commands`h: `l[`h(`nM`h == Menu)`l]`h: ", ch);
	  return;
	}
      if (ch->desc->editor == SEARCH_REPLACE)
	{
	  send_to_char
	    ("Please enter the search word(s) and their replacement.\n\rExample: sword 'long sword'  'sword' will be replaced with 'long sword'.\n\rText search is case-sensitive.\n\r> ",
	     ch);
	  return;
	}
      if (ch->desc->editor == EDIT_NEW_LINE)
	{
	  char lbuf[MAX_STRING_LENGTH];
	  sprintf (lbuf,
		   "Please enter new text for this line.\n\r\n\rOld> %s\n\rNew> ",
		   ch->desc->submit_info);
	  send_to_char (lbuf, ch);
	  return;
	}
      if (ch->desc->editor == INSERT_NEW_LINE)
	{
	  send_to_char ("Please enter text to insert.\n\r\n\r> ", ch);
	  return;
	}
      if (ch->desc->editor == EDIT_LINES)
	{
	  send_to_char ("] ", ch);
	  return;
	}
      if (ch->desc->editor == EDIT_LINE_NUMBER)
	{
	  send_to_char
	    ("Which line number would you like to edit (Return == Abort)? ",
	     ch);
	  return;
	}
      if (ch->desc->editor == INSERT_LINE_NUMBER)
	{
	  send_to_char
	    ("Which line number would you like to insert new line before (RETURN == Abort)? ",
	     ch);
	  return;
	}
      if (ch->desc->editor == DELE_LINE_NUMBER)
	{
	  send_to_char
	    ("Which line number would you like to delete (RETURN == Abort)? ",
	     ch);
	  return;
	}
      //Iblis - 6/16/04 - Card Game prompt
      if (ch->pcdata && ch->pcdata->cg_state > CG_ANTE_ACCEPTED)//>= CG_START_PM_YT && ch->pcdata->cg_state <= CG_NYT_D_PM)
        {
/*	  int handcount,deckcount;
	  OBJ_DATA *obj1,*obj2;
	  handcount = count_contents(get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand"));
	  deckcount = count_contents(get_obj_vnum_char(ch,OBJ_VNUM_DECK));
	  sprintf(buf,"<Cards: %d Deck: %d Mobs: %d Terrain: %s Turn: Iblis Last card played: [Card] [Spell] [0] [A mouse ear]>", handcount, deckcount, (ch->pcdata->cg_secondary_mob == NULL)?1:2, (ch->pcdata->cg_terrain == NULL)?"NA":flag_string(card_terrain_type,ch->pcdata->cg_terrain->value[7]);
			  }*/
          card_game_prompt(ch);
          return;
        }
		  
      
      if (ch->desc->editor == SAVE_CLAN_HELP && ch->desc->pString == NULL)
	{
	  send_to_char ("Saving clan help file...\n\r", ch);
	  edit_done (ch);
	  save_help (ch->desc->help_name, ch->desc->help_info, "clans.hlp");
	  if (ch->desc->help_name != NULL)
	  {
	    free_string (ch->desc->help_name);
	    ch->desc->help_name = NULL;
	  }
	  if (ch->desc->help_info != NULL)
	  {
	    free_string (ch->desc->help_info);
	    ch->desc->help_info = NULL;
	  }
	}
      if (ch->desc->editor == MAIL_TEXT && ch->desc->pString == NULL)
	{
	  char tcbuf[MAX_STRING_LENGTH * 3];
	  send_to_char ("Sending Mail...\n\r", ch);
	  edit_done (ch);
	  sprintf (tcbuf, "%s%s\n\n", ch->desc->help_info,
		   ch->desc->help_name);
	  if (ch->desc->help_name != NULL)
	  {
	    free_string (ch->desc->help_name);
	    ch->desc->help_name = NULL;
	  }
	  if (ch->desc->help_info != NULL)
	  {
	    free_string (ch->desc->help_info);
	    ch->desc->help_info = NULL;
	  }
	  send_mail (tcbuf);
	}
      if (ch->desc->editor == SUBMIT_TEXT && ch->desc->pString == NULL)
	{
	  char tcbuf[MAX_STRING_LENGTH * 3];
	  send_to_char ("Sending Submission...\n\r", ch);
	  edit_done (ch);
	  sprintf (tcbuf, "%s%s\n\n", ch->desc->help_info,
		   ch->desc->help_name);
	  if (ch->desc->help_name != NULL)
	  {
	    free_string (ch->desc->help_name);
	    ch->desc->help_name = NULL;
	  }
	  if (ch->desc->help_info != NULL)
	  {
	    free_string (ch->desc->help_info);
	    ch->desc->help_info = NULL;
	  }
	  send_submission ("weave@best.com", tcbuf);
	}
      if (ch->desc->editor == SAVE_ANY_HELP && ch->desc->pString == NULL)
	{
	  send_to_char ("Saving help file...\n\r", ch);
	  edit_done (ch);
	  save_help (ch->desc->help_name, ch->desc->help_info, "online.hlp");
	  if (ch->desc->help_name != NULL)
	  {
	    free_string (ch->desc->help_name);
	    ch->desc->help_name = NULL;
	  }
	  if (ch->desc->help_info != NULL)
	  {
	    free_string (ch->desc->help_info);
	    ch->desc->help_info = NULL;
	  }
	}
    }
  memset (buf, 0, MAX_STRING_LENGTH);
  memset (buf2, 0, MAX_STRING_LENGTH);
  point = buf;
  str = ch->prompt;
  if (str == NULL || str[0] == '\0')
    {
      sprintf (buf,
	       "`a<`lHP`a:`g%d `lMA`a:`g%d `lMV`a:`g%d`a>``\n\r`a>`` %s",
	       ch->hit, ch->mana, ch->move, ch->prefix);
      send_to_char (buf, ch);
      return;
    }
  if (IS_SET (ch->comm, COMM_AFK))
    {
      send_to_char ("`k<AFK>`` ", ch);
      return;
    }
  if (!IS_NPC (ch))
    if (ch->desc != NULL && ch->desc->editor != 0)
      if (ch->desc->editor == ED_AREA
	  || ch->desc->editor == ED_ROOM
	  || ch->desc->editor == ED_OBJECT
	  || ch->desc->editor == ED_MOBILE
	  || ch->desc->editor == BUILD_MAIN
	  || ch->desc->editor == BUILD_NAME
	  || ch->desc->editor == BUILD_FLAGS
	  || ch->desc->editor == BUILD_KEYWORDS
	  || ch->desc->editor == BUILD_KEYWORDS_ADD
	  || ch->desc->editor == BUILD_DOORS
	  || ch->desc->editor == BUILD_DRESETS
	  || ch->desc->editor == BUILD_DNAME
	  || ch->desc->editor == ED_OBJTRIG
	  || ch->desc->editor == BUILD_DOORS_FLAGS
	  || ch->desc->editor == BUILD_EXITMSG
	  || ch->desc->editor == BUILD_ENTERMSG
	  )
	{
	  char tcbuf[MAX_STRING_LENGTH];
	  strcpy (tcbuf, "`k<BUILDING ");
	  switch (ch->desc->editor)
	    {
	    case ED_AREA:
	      sprintf (tcbuf + strlen (tcbuf), "[Area: %d]",
		       ((AREA_DATA *) ch->desc->pEdit)->vnum);
	      break;
	    case ED_ROOM:
	      sprintf (tcbuf + strlen (tcbuf), "[Room: %d]",
		       ch->in_room->vnum);
	      break;
	    case ED_OBJECT:
	      sprintf (tcbuf + strlen (tcbuf), "[Object: %d]",
		       ((OBJ_INDEX_DATA *) ch->desc->pEdit)->vnum);
	      break;
	    case ED_MOBILE:
	      sprintf (tcbuf + strlen (tcbuf), "[Mobile: %d]",
		       ((MOB_INDEX_DATA *) ch->desc->pEdit)->vnum);
	      break;
	    case ED_OBJTRIG:
              sprintf (tcbuf + strlen (tcbuf), "[Object Trigger: %d]",
	               ((OBJ_TRIG_DATA *) ch->desc->pEdit)->vnum);
	      break;
	    }
	  if (!IS_SET (ch->act, PLR_AUTORETURN))
	    strcat (tcbuf, ">``\n\r> ");

	  else
	    strcat (tcbuf, ">`` ");
	  send_to_char (tcbuf, ch);
	  return;
	}
  while (*str != '\0')
    {
      if (*str != '%')
	{
	  *point++ = *str++;
	  continue;
	}
      ++str;
      switch (*str)
	{
	default:
	  i = " ";
	  break;
	case 'e':
	  found = FALSE;
	  doors[0] = '\0';
	  if (!is_affected (ch, gsn_disorientation) && !IS_AFFECTED(ch,AFF_BLIND))
	    {
	      for (door = 0; door < 6; door++)
		{
		  if ((pexit = ch->in_room->exit[door]) != NULL
		      && pexit->u1.to_room != NULL
		      && (can_see_room (ch, pexit->u1.to_room)
			  || (IS_AFFECTED (ch, AFF_INFRARED)
			      && !IS_AFFECTED (ch, AFF_BLIND)))
		      && !IS_SET (pexit->exit_info, EX_CLOSED))
		    {
		      found = TRUE;
		      strcat (doors, short_dir_name[door]);
		    }
		}
	      if (!found)
		strcat (buf, "none");
	    }

	  else
	    strcat (doors, "none");
	  sprintf (buf2, "%s", doors);
	  i = buf2;
	  break;
	case 'c':
	  sprintf (buf2, "%s", "\n\r");
	  i = buf2;
	  break;
	case 'h':
	  sprintf (buf2, "`F%d``", ch->hit);
	  i = buf2;
	  break;
	case 'H':
	  sprintf (buf2, "%d", ch->max_hit);
	  i = buf2;
	  break;
	case 'm':
	  sprintf (buf2, "`I%d``", ch->mana);
	  i = buf2;
	  break;
	case 'M':
	  sprintf (buf2, "%d", ch->max_mana);
	  i = buf2;
	  break;
	  if (IS_NPC (ch))
	    {
	      sprintf (buf2, "-1");
	    }

	  else
	    {
	      sprintf (buf2, "%ld", level_cost (ch->level));
	    }
	  i = buf2;
	  break;
	case 'v':
	  if (!IS_NPC (ch) && ch->mount != NULL)
	    sprintf (buf2, "`Jmount:%d``", ch->mount->move);

	  else
	    sprintf (buf2, "`J%d``", ch->move);
	  i = buf2;
	  break;
	case 'V':
	  sprintf (buf2, "%d", ch->max_move);
	  i = buf2;
	  break;
	case 'x':
	  sprintf (buf2, "%ld", ch->exp);
	  i = buf2;
	  break;
	case 'X':
	  sprintf (buf2, "%ld", IS_NPC (ch) ? 0 : (level_cost (ch->level)));
	  i = buf2;
	  break;
	case 'g':
	  sprintf (buf2, "%ld", ch->gold);
	  i = buf2;
	  break;
	case 's':
	  sprintf (buf2, "%ld", ch->silver);
	  i = buf2;
	  break;
	case 'S':
	  lame[0] = '\0';
	  if (IS_AFFECTED (ch, AFF_HIDE))
	    strcat (lame, "`G(h)``");
	  if (IS_AFFECTED (ch, AFF_CAMOUFLAGE))
	    strcat (lame, "`c(c)``");
	  if (IS_AFFECTED (ch, AFF_SNEAK))
	    strcat (lame, "`G(s)``");
	  if (IS_AFFECTED (ch, AFF_INVISIBLE))
	    strcat (lame, "`E(i)``");
	  sprintf (buf2, "%s", lame);
	  i = buf2;
	  break;
	case 'a':
	  if (ch->level > 9)
	    sprintf (buf2, "%d", ch->alignment);

	  else
	    sprintf (buf2, "%s",
		     IS_GOOD (ch) ? "good" : IS_EVIL (ch) ? "evil" :
		     "neutral");
	  i = buf2;
	  break;
	case 'r':
	  if (ch->in_room != NULL)
	    sprintf (buf2, "%s",
		     ((!IS_NPC (ch) && IS_SET (ch->act, PLR_HOLYLIGHT))
		      || (!IS_AFFECTED (ch, AFF_BLIND)
			  && (!room_is_dark (ch->in_room) || ch->race == PC_RACE_KALIAN))) ? ch->
		     in_room->name : "darkness");

	  else
	    sprintf (buf2, " ");
	  i = buf2;
	  break;
	case 'R':
	  if (IS_IMMORTAL (ch) && ch->in_room != NULL)
	    sprintf (buf2, "%d", ch->in_room->vnum);

	  else
	    sprintf (buf2, " ");
	  i = buf2;
	  break;
	case 'z':
	  if (IS_IMMORTAL (ch) && ch->in_room != NULL)
	    sprintf (buf2, "%s", ch->in_room->area->name);

	  else
	    sprintf (buf2, " ");
	  i = buf2;
	  break;
	case '%':
	  sprintf (buf2, "%%");
	  i = buf2;
	  break;
	}
      ++str;
      while ((*point = *i) != '\0')
	++point, ++i;
    }
  send_to_char (buf, ch);
  if (!IS_SET (ch->act, PLR_AUTORETURN))
    send_to_char ("\n\r`a>`` ", ch);

  else
    send_to_char ("``", ch);
  if (ch->prefix[0] != '\0')
    send_to_char (ch->prefix, ch);
  return;
}


/*
 * Append onto an output buffer.
 */
void write_to_buffer (DESCRIPTOR_DATA * d, const char *txt, int length)
{

  /*
   * Find length in case caller didn't.
   */
  if (length <= 0)
    length = strlen (txt);

  /*
   * Initial \n\r if needed.
   */
  if (d->outtop == 0 && !d->fcommand)
    {
      d->outbuf[0] = '\n';
      d->outbuf[1] = '\r';
      d->outtop = 2;
    }

  /*
   * Expand the buffer as needed.
   */
  while (d->outtop + length >= d->outsize)
    {
      char *outbuf;
      if (d->outsize >= 32000)
	{
	  bug ("Buffer overflow. Closing.\n\r", 0);
	  d->outtop = 0;

	  //close_socket(d);
	  return;
	}
      outbuf = alloc_mem (2 * d->outsize);
//      outbuf = reinterpret_cast<char *>(alloc_mem(2 * d->outsize));
      strncpy (outbuf, d->outbuf, d->outtop);
      free_mem (d->outbuf, d->outsize);
      d->outbuf = outbuf;
      d->outsize *= 2;
    }

  /*
   * Copy.
   */
  strcpy (d->outbuf + d->outtop, txt);
  d->outtop += length;
  return;
}


/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor (int desc, char *txt, int length)
{
  int iStart;
  int nWrite;
  int nBlock;

#if defined(macintosh) || defined(MSDOS)
  if (desc == 0)
    desc = 1;

#endif /*  */
  if (length <= 0)
    length = strlen (txt);
  for (iStart = 0; iStart < length; iStart += nWrite)
    {
      nBlock = UMIN (length - iStart, 4096);
      if ((nWrite = write (desc, txt + iStart, nBlock)) < 0)
	{
	  perror ("Write_to_descriptor");
	  bug ("spammed off!", 0);
	  return FALSE;
	}
    }
  return TRUE;
}

int good_password (char *passwd, char *username)
{
  int nospecial = TRUE, xx;
  char *bad_passwd[] =
    { "exodus!", "mud&mud", "rom+rom", "fuck!you", "!exodus",
    "rom!rom", NULL
  };
  if (strlen (passwd) < 6)
    return (0);
  for (xx = 0; passwd[xx]; xx++)
    {
      if (!isalnum (passwd[xx]))
	{
	  nospecial = FALSE;
	  break;
	}
      if (xx == 7)
	break;
    }
  if (nospecial)
    return (0);
  for (xx = 0; bad_passwd[xx]; xx++)
    if (!str_cmp (bad_passwd[xx], passwd))
      return (0);
  return (1);
}















































/*
 * Deal with sockets that haven't logged in yet.
 */




void do_con_get_ansi(DESCRIPTOR_DATA * d, char *argument)
{
	extern char *help_greeting, *help_agreeting;

	if (argument[0] == '\0' || !str_cmp (argument, "Y")
		|| !str_cmp (argument, "yes"))
		d->ansi = TRUE;

	else if (!str_cmp (argument, "n") || !str_cmp (argument, "no"))
		d->ansi = FALSE;
	
	else
	{
		write_to_buffer (d, "ANSI Color (Y/n)? ", 0);
		return;
	}
	  
	if (!d->ansi)
	{
	  if (help_greeting[0] == '.')
	    write_to_buffer (d, help_greeting + 1, 0);
	  else
	    write_to_buffer (d, help_greeting, 0);
	}

	else
	{
	  if (help_agreeting[0] == '.')
	    send_to_desc (d, help_agreeting + 1);

	  else
	    send_to_desc (d, help_agreeting);
	}

    d->connected = CON_GET_NAME;
	return;
}







void 	do_con_get_name(DESCRIPTOR_DATA * d, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	bool fOld;
	CHAR_DATA *ch;
	ch = d->character;

    if (argument[0] == '\0')
	{
	  close_socket (d);
	  return;
	}
      argument[0] = UPPER (argument[0]);
      if (!check_parse_name (argument))
	{
	  write_to_buffer (d, "Illegal name, try another.\n\rName: ", 0);
	  return;
	}
      fOld = load_char_obj (d, argument);
      ch = d->character;
      if (IS_SET (ch->act, PLR_DENY))
	{
	  sprintf (log_buf, "Denying access to %s@%s.", argument, d->host);
	  log_string (log_buf);
	  write_to_buffer (d, "You are denied access.\n\r", 0);
	  close_socket (d);
	  return;
	}
      if (check_ban (d->host, BAN_PERMIT) && !IS_SET (ch->act, PLR_PERMIT))
	{
	  write_to_buffer
	    (d, "Your site has been banned from this mud.\n\r", 0);
	  close_socket (d);
	  return;
	}
      if (check_reconnect (d, argument, FALSE))
	{
	  fOld = TRUE;
	}

      else
	{
	  if (wizlock && !IS_IMMORTAL (ch))
	    {
	      write_to_buffer (d, "The game is wizlocked.\n\r", 0);
	      close_socket (d);
	      return;
	    }
	}
      if (fOld)
	{

	  /* Old player */
	  write_to_buffer (d, "Password: ", 0);
	  write_to_buffer (d, echo_off_str, 0);
	  d->connected = CON_GET_OLD_PASSWORD;
	  return;
	}

      else
	{

	  /* New player */
	  if (newlock)
	    {
	      write_to_buffer (d, "The game is newlocked.\n\r", 0);
	      close_socket (d);
	      return;
	    }
	  if (check_ban (d->host, BAN_NEWBIES))
	    {
	      write_to_buffer (d,
			       "The game is newlocked.\n\r",
			       0);
	      close_socket (d);
	      return;
	    }
	  if (check_create (argument))
	    {
	      write_to_buffer (d,
			       "Sorry, someone else is already creating a character of the same name.\n\r",
			       0);
	      write_to_buffer (d, "Name: ", 0);
	      return;
	    }
	  add_new_create (argument);
	  sprintf (buf, "Did I get that right, %s (Y/N)? ", argument);
	  write_to_buffer (d, buf, 0);
	  d->connected = CON_CONFIRM_NEW_NAME;
	  return;
	}

}


void do_con_get_old_password(DESCRIPTOR_DATA * d, char *argument)
{
	FILE *sfp;
	char bpbuf[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	char sbufo[MAX_INPUT_LENGTH];

	ch = d->character;

#if defined(unix)
    write_to_buffer (d, "\n\r", 2);
#endif /*  */

    ch->color = d->ansi;
    if (strcmp (crypt (argument, ch->pcdata->pwd), ch->pcdata->pwd))
	{
	  write_to_buffer (d, "Wrong password.\n\r", 0);
	  sprintf (bpbuf, "Bad password for char  %s [HOST=%s]\n\r",
		   ch->name, ch->desc->host);
	  log_string (bpbuf);
	  close_socket (d);
	  return;
	}

    write_to_buffer (d, echo_on_str, 0);

    if (check_playing (d, ch->name))
		return;
    if (check_reconnect (d, ch->name, TRUE))
		return;

	sprintf (log_buf, "%s@%s has connected.", ch->name, d->host);
	ch->pcdata->ldhost = str_dup (d->host);
	log_string (log_buf);
	wiznet (log_buf, NULL, NULL, WIZ_SITES, 0, get_trust (ch));
  
	if (ch->level > 100)
	{
	  if (fork () == 0)
	  {
	      if ((sfp = popen (SECURITY_FILE, "a")) == NULL)
			exit (0);

	      fprintf (sfp, "Exodus SECURITY Warning\n");
	      fprintf (sfp, "Player '%s' has a level greater than 100.\n\n", ch->name);
	      pclose (sfp);
	      exit (0);
	  }
	  sprintf (sbufo, "SECURITY Warning.  Player '%s' has a level greater than 100.", ch->name);
	  log_string (sbufo);
	}

    if (IS_IMMORTAL (ch))
	{
	  do_help (ch, "imotd");
	  d->connected = CON_READ_IMOTD;
	}

    else
	{
	  do_help (ch, "motd");
	  d->connected = CON_READ_MOTD;
	}
}

void do_con_break_connect(DESCRIPTOR_DATA * d, char *argument)
{
	CHAR_DATA *ch;
	DESCRIPTOR_DATA *d_old, *d_next;
	ch = d->character;

	switch (*argument)
	{
		case 'y':
		case 'Y':
			for (d_old = descriptor_list; d_old != NULL; d_old = d_next)
			{
			  d_next = d_old->next;
			  if (d_old == d || d_old->character == NULL)
				continue;
			  if (str_cmp (ch->name, d_old->original ? d_old->original->name : d_old->character->name))
				continue;
			  close_socket (d_old);
			}

			if (check_reconnect (d, ch->name, TRUE))
				return;

			write_to_buffer (d, "Reconnect attempt failed.\n\rName: ", 0);
			if (d->character != NULL)
			{
				free_char (d->character);
				d->character = NULL;
			}
			d->connected = CON_GET_NAME;
			break;

		case 'n':
		case 'N':
			write_to_buffer (d, "Name: ", 0);
			if (d->character != NULL)
			{
				free_char (d->character);
				d->character = NULL;
			}
			d->connected = CON_GET_NAME;
			break;

		default:
			write_to_buffer (d, "Please type Y or N? ", 0);
			break;
		}
		
}

void do_con_confirm_new_name(DESCRIPTOR_DATA * d, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	ch = d->character;
      switch (*argument)
	{
	case 'y':
	case 'Y':
	  sprintf (buf,
		   "New character.\n\rGive me a password for %s: %s",
		   ch->name, echo_off_str);
	  write_to_buffer (d, buf, 0);
	  d->connected = CON_GET_NEW_PASSWORD;
	  break;
	case 'n':
	case 'N':
	  write_to_buffer (d, "Ok, what IS it, then? ", 0);
	  if (d->character->name != NULL)
	    remove_create (d->character->name);
	  free_char (d->character);
	  d->character = NULL;
	  d->connected = CON_GET_NAME;
	  break;
	default:
	  write_to_buffer (d, "Please type Yes or No? ", 0);
	  break;
	}

}

void do_con_get_new_password(DESCRIPTOR_DATA * d, char *argument)
{
	char *pwdnew;
	char *p;
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	ch = d->character;

#if defined(unix)
      write_to_buffer (d, "\n\r", 2);
#endif /*  */

      ch->color = d->ansi;
      if (!good_password (argument, ch->name))
	{
	  write_to_buffer (d,
			   "That password is not acceptable.  Passwords must be at least 6 characters\n\rin length, must have at least one non-alphanumeric character in them, and\n\rmust not be easily guessable.  Examples: beastie!, number#2\n\rPassword: ",
			   0);
	  return;
	}
      pwdnew = crypt (argument, ch->name);
      for (p = pwdnew; *p != '\0'; p++)
	{
	  if (*p == '~')
	    {
	      write_to_buffer (d,
			       "New password not acceptable, try again.\n\rPassword: ",
			       0);
	      return;
	    }
	}
      free_string (ch->pcdata->pwd);
      ch->pcdata->pwd = str_dup (pwdnew);
      write_to_buffer (d, "Please retype password: ", 0);
      d->connected = CON_CONFIRM_NEW_PASSWORD;
}

void do_con_confirm_new_password(DESCRIPTOR_DATA * d, char *argument)
{
	CHAR_DATA *ch;
	ch = d->character;

    if (strcmp (crypt (argument, ch->pcdata->pwd), ch->pcdata->pwd))
	{
	  write_to_buffer (d, "Passwords don't match.\n\rRetype password: ", 0);
	  d->connected = CON_GET_NEW_PASSWORD;
	  return;
	}
 
	write_to_buffer (d, echo_on_str, 0);
	write_to_buffer (d,"\n\rExodus advertises itself in several different ways.  It would be very helpful to",0); 
	write_to_buffer (d,"\n\rthe administrators to know how you found out about Exodus.  Thank you very much.",0);
	write_to_buffer (d,"\n\rIf you would prefer not to say, just hit ENTER.\n\rYour answer -> ",0);
	d->connected = CON_GET_REFERRAL;
}

void do_con_get_referral(DESCRIPTOR_DATA * d, char *argument)
{
	char *cptr;
	CHAR_DATA *ch;
	ch = d->character;


	ch->pcdata->referrer = str_dup (argument);
	if ((cptr = strchr (ch->pcdata->referrer, '\n')) != NULL)
	  *cptr = '\0';
	if ((cptr = strchr (ch->pcdata->referrer, '\r')) != NULL)
	  *cptr = '\0';

	write_to_buffer (d, echo_on_str, 0);
	write_to_buffer (d,"\n\rFor your protection, Exodus requires that you enter a valid email address.  The",0);
	write_to_buffer (d,"\n\rdomain from which you are currently connected has been recorded for security",0);
	write_to_buffer (d,"\n\rpurposes.  This information will not be made available to anyone except at your discretion.\n\r",0);
	write_to_buffer (d, "\n\rPlease enter your email address: ", 0);
	d->connected = CON_GET_EMAIL;
}

void do_con_get_email(DESCRIPTOR_DATA * d, char *argument)
{
	char *cptr;
	CHAR_DATA *ch;
	ch = d->character;

	if (strlen (argument) < 6 || strchr (argument, '@') == NULL || strchr (argument, '.') == NULL)
	{
	  write_to_buffer (d, "That is an invalid email address.\n\r", 0);
	  write_to_buffer (d, "\n\rPlease enter your email address: ", 0);
	  return;
	}
      

	ch->pcdata->email_addr = str_dup (argument);
	if ((cptr = strchr (ch->pcdata->email_addr, '\n')) != NULL)
	  *cptr = '\0';
	if ((cptr = strchr (ch->pcdata->email_addr, '\r')) != NULL)
	  *cptr = '\0';

    if (d->ansi)
		send_to_desc (d, ANSI_CLSHOME);

	send_to_desc (d, "\n\r");
	send_to_desc (d," __^__                                                            __^__\n\r");
	send_to_desc (d,"( ___ )----------------------------------------------------------( ___ )\n\r");
	send_to_desc (d," | / |                                                            | \\ |\n\r");
	send_to_desc (d," | / |                Please Choose a Race:                       | \\ |\n\r");
	send_to_desc (d," | / |------------------------------------------------------------| \\ |\n\r");
	send_to_desc (d," | / |                                                            | \\ |\n\r");
	send_to_desc (d," | / |             [`oH``]uman            [`oE``]lf                       | \\ |\n\r");
	send_to_desc (d," | / |             [`oD``]warf            [`oC``]anthi                    | \\ |\n\r");
	send_to_desc (d," | / |             [`oV``]ro'ath          [`oS``]yvin                     | \\ |\n\r");
	send_to_desc (d," | / |             [`oSI``]dhe            [`oL``]itan                     | \\ |\n\r");
	send_to_desc (d," |___|                                                            |___|\n\r");
	send_to_desc (d,"(_____)----------------------------------------------------------(_____)\n\r");
	send_to_desc (d, "\n\rWhat is your race (help for more information)? ");
	d->connected = CON_GET_NEW_RACE;
}

void do_con_get_new_race(DESCRIPTOR_DATA *d, char *argument)
{
	int race, i;
	char tcbuf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	ch = d->character;

	one_argument (argument, arg);
	if (!strcmp (arg, "help"))
	{
	  argument = one_argument (argument, arg);
	  if (argument[0] == '\0')
	    {
	      if (d->ansi)
			write_to_buffer (d, ANSI_CLSHOME, 0);
	      do_help (ch, "race help");
	    }

	  else
	    {
	      if (d->ansi)
			write_to_buffer (d, ANSI_CLSHOME, 0);
	      do_help (ch, argument);
	    }
	  write_to_buffer (d,
			   "What is your race (help for more information)? ",
			   0);
	  return;
	}

	race = race_lookup (argument);

	if (race == 0 || race >= 10 || race == 8 || !race_table[race].pc_race)
	{
	  if (d->ansi)
	    write_to_buffer (d, ANSI_CLSHOME, 0);
	  write_to_buffer (d, "\n\r", 0);
	  send_to_desc (d," __^__                                                            __^__\n\r");
	  send_to_desc (d,"( ___ )----------------------------------------------------------( ___ )\n\r");
	  send_to_desc (d," | / |                                                            | \\ |\n\r");
	  send_to_desc (d," | / |                Please Choose a Race:                       | \\ |\n\r");
	  send_to_desc (d," | / |------------------------------------------------------------| \\ |\n\r");
	  send_to_desc (d," | / |                                                            | \\ |\n\r");
	  send_to_desc (d," | / |             [`oH``]uman            [`oE``]lf                       | \\ |\n\r");
	  send_to_desc (d," | / |             [`oD``]warf            [`oC``]anthi                    | \\ |\n\r");
	  send_to_desc (d," | / |             [`oV``]ro'ath          [`oS``]yvin                     | \\ |\n\r");
	  send_to_desc (d," | / |             [`oSI``]dhe            [`oL``]itan                     | \\ |\n\r");
	  send_to_desc (d," |___|                                                            |___|\n\r");
	  send_to_desc (d,"(_____)----------------------------------------------------------(_____)\n\r");
	  send_to_desc (d, "\n\r");
	  send_to_desc (d, "That is not a valid race.\n\r");
	  send_to_desc (d, "What is your race? (help race for more information) ");
	 return;
	}

	ch->race = race;
	ch->pcdata->new_style = 0;

	/* initialize stats */
	roll_stats (ch);
	ch->affected_by = ch->affected_by | race_table[race].aff;
	ch->imm_flags = ch->imm_flags | race_table[race].imm;
	ch->res_flags = ch->res_flags | race_table[race].res;
	ch->vuln_flags = ch->vuln_flags | race_table[race].vuln;
	ch->form = race_table[race].form;
	ch->parts = race_table[race].parts;

      /* add skills */
	for (i = 0; i < 5; i++)
	{
		if (pc_race_table[race].skills[i] == NULL)
		continue;
		group_add (ch, pc_race_table[race].skills[i], FALSE);
	}

	/* add cost */
	ch->pcdata->points = pc_race_table[race].points;
	ch->size = pc_race_table[race].size;
	if (d->ansi)
		write_to_buffer (d, ANSI_CLSHOME, 0);
      strcpy (tcbuf," __^__                                                            __^__\n\r");
      strcat (tcbuf,"( ___ )----------------------------------------------------------( ___ )\n\r");
      strcat (tcbuf," | / |                                                            | \\ |\n\r");
      strcat (tcbuf," | / |                          Stats:                            | \\ |\n\r");
      strcat (tcbuf," | / |------------------------------------------------------------| \\ |\n\r");
      strcat (tcbuf," | / |                                                            | \\ |\n\r");
      sprintf (tcbuf + strlen (tcbuf),
	       " | / |           Strength: %2d               Dexterity: %2d         | \\ |\n\r",
	       ch->perm_stat[STAT_STR], ch->perm_stat[STAT_DEX]);
      sprintf (tcbuf + strlen (tcbuf),
	       " | / |       Intelligence: %2d            Constitution: %2d         | \\ |\n\r",
	       ch->perm_stat[STAT_INT], ch->perm_stat[STAT_CON]);
      sprintf (tcbuf + strlen (tcbuf),
	       " | / |             Wisdom: %2d                Charisma: %2d         | \\ |\n\r",
	       ch->perm_stat[STAT_WIS], ch->perm_stat[STAT_CHA]);
      strcat (tcbuf," |___|                                                            |___|\n\r");
      strcat (tcbuf,"(_____)----------------------------------------------------------(_____)\n\r");
      strcat (tcbuf, "\n\r");
      strcat (tcbuf, "Use these stats (Y/n)? ");
      send_to_desc (d, tcbuf);
      d->connected = CON_CONFIRM_STATS;
}

void do_con_confirm_stats(DESCRIPTOR_DATA * d, char *argument)
{
	char   tcbuf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	ch = d->character;

	one_argument (argument, arg);
	if (!str_cmp (arg, "n") || !str_cmp (arg, "no"))
	{
		roll_stats (ch);
		if (d->ansi)
			write_to_buffer (d, ANSI_CLSHOME, 0);
		strcpy (tcbuf," __^__                                                            __^__\n\r");
		strcat (tcbuf,"( ___ )----------------------------------------------------------( ___ )\n\r");
		strcat (tcbuf," | / |                                                            | \\ |\n\r");
		strcat (tcbuf," | / |                          Stats:                            | \\ |\n\r");
		strcat (tcbuf," | / |------------------------------------------------------------| \\ |\n\r");
		strcat (tcbuf," | / |                                                            | \\ |\n\r");
		sprintf (tcbuf + strlen (tcbuf),
		   " | / |           Strength: %2d               Dexterity: %2d         | \\ |\n\r",
		   ch->perm_stat[STAT_STR], ch->perm_stat[STAT_DEX]);
		sprintf (tcbuf + strlen (tcbuf),
		   " | / |       Intelligence: %2d            Constitution: %2d         | \\ |\n\r",
		   ch->perm_stat[STAT_INT], ch->perm_stat[STAT_CON]);
		sprintf (tcbuf + strlen (tcbuf),
		   " | / |             Wisdom: %2d                Charisma: %2d         | \\ |\n\r",
		   ch->perm_stat[STAT_WIS], ch->perm_stat[STAT_CHA]);
		strcat (tcbuf," |___|                                                            |___|\n\r");
		strcat (tcbuf,"(_____)----------------------------------------------------------(_____)\n\r");
		strcat (tcbuf, "\n\r");
		strcat (tcbuf, "Use these stats (Y/n)? ");
		send_to_desc (d, tcbuf);
		return;
	}

	if (d->ansi)
		write_to_buffer (d, ANSI_CLSHOME, 0);
	send_to_desc (d, "\n\r");
	send_to_desc (d," __^__                                                            __^__\n\r");
	send_to_desc (d,"( ___ )----------------------------------------------------------( ___ )\n\r");
	send_to_desc (d," | / |                                                            | \\ |\n\r");
	send_to_desc (d," | / |                Please Choose a Class:                      | \\ |\n\r");
	send_to_desc (d," | / |------------------------------------------------------------| \\ |\n\r");
	send_to_desc (d," | / |                                                            | \\ |\n\r");
	sprintf (tcbuf,
	   " | / |             %s       %s                    | \\ |\n\r",
	   IS_SET (genraces[ch->race],
		   CLASS_MAGE) ? "[`oM``]agic-User" :
	   "`aMagic-User``  ", IS_SET (genraces[ch->race],
				   CLASS_CLERIC) ?
	   "[`oC``]leric" : "`aCleric``  ");
	send_to_desc (d, tcbuf);

	sprintf (tcbuf,
	   " | / |             %s            %s                   | \\ |\n\r",
	   IS_SET (genraces[ch->race],
		   CLASS_THIEF) ? "[`oT``]hief" : "`aThief``  ",
	   IS_SET (genraces[ch->race],
		   CLASS_WARRIOR) ? "[`oW``]arrior" : "`aWarrior``  ");
	send_to_desc (d, tcbuf);

	sprintf (tcbuf,
	   " | / |             %s           %s                      | \\ |\n\r",
	   IS_SET (genraces[ch->race],
		   CLASS_RANGER) ? "[`oR``]anger" : "`aRanger``  ",
	   IS_SET (genraces[ch->race],
		   CLASS_BARD) ? "[`oB``]ard" : "`aBard``  ");
	send_to_desc (d, tcbuf);

	sprintf (tcbuf,
	   " | / |             %s          %s                  | \\ |\n\r",
	   IS_SET (genraces[ch->race],
		   CLASS_PALADIN) ? "[`oP``]aladin" :
	   "`aPaladin``  ", IS_SET (genraces[ch->race],
				  CLASS_ASSASSIN) ?
	   "[`oA``]ssassin" : "`aAssassin``  ");
	send_to_desc (d, tcbuf);

	sprintf (tcbuf,
	   " | / |             %s           %s                      | \\ |\n\r",
	   IS_SET (genraces[ch->race],
		   CLASS_REAVER) ? "[`oRE``]aver" : "`aReaver``  ",
	   IS_SET (genraces[ch->race],
		   CLASS_MONK) ? "[`oMO``]nk" : "`aMonk``  ");
	send_to_desc (d, tcbuf);

	sprintf (tcbuf,
	 " | / |             %s                                         | \\ |\n\r",
	 IS_SET (genraces[ch->race],
		   CLASS_DRUID) ? "[`oD``]ruid" : "`aDruid``  ");
	send_to_desc (d, tcbuf);

	send_to_desc (d," |___|                                                            |___|\n\r");
	send_to_desc (d,"(_____)----------------------------------------------------------(_____)\n\r");
	send_to_desc (d, "\n\rWhat Class would you like to be? ");
	d->connected = CON_GET_NEW_CLASS;
}

void do_con_get_new_sex(DESCRIPTOR_DATA * d, char *argument)
{
	OBJ_DATA *sword=NULL;		/*reaver sword */
	short rndo;
	CHAR_DATA *ch;
	ch = d->character;

	switch (argument[0])
	{
		case 'm':
		case 'M':
		  ch->sex = SEX_MALE;
		  ch->pcdata->true_sex = SEX_MALE;
		  break;
		case 'f':
		case 'F':
		  ch->sex = SEX_FEMALE;
		  ch->pcdata->true_sex = SEX_FEMALE;
		  break;
		default:
		  send_to_desc (d,
				"Sorry, there is no freak gender.\n\rWhat IS your sex? ");
		  return;
	}
	
	//todo - move to reaver func
	if (IS_CLASS(ch,PC_CLASS_REAVER) && ch->sword)
	{
		sword = create_object (get_obj_index (OBJ_VNUM_REAVER_SWORD), 0);
		clone_object (ch->sword, sword);
		obj_to_char (sword, ch);
	}
	
	rndo = number_percent ();
	if (rndo < 75)
	{
		ch->pcdata->primary_hand = HAND_RIGHT;
		send_to_char ("\n\rYou will be right handed.\n\r", ch);
		//todo - move to reaver func
		if (IS_CLASS(ch,PC_CLASS_REAVER) && (sword != NULL))	/* Reaver */
			equip_char (ch, sword, WEAR_WIELD_R);
	}
	else if (rndo < 95)
	{
		ch->pcdata->primary_hand = HAND_LEFT;
		send_to_char ("\n\rYou will be left handed.\n\r", ch);
		//todo - move to reaver func. need to switch primary hand and equip appropriately
		if (IS_CLASS(ch,PC_CLASS_REAVER) && (sword != NULL))	/* Reaver */
		equip_char (ch, sword, WEAR_WIELD_L);
	}
	else
	{
		ch->pcdata->primary_hand = HAND_AMBIDEXTROUS;
		send_to_char ("\n\rYou will be ambidextrous.\n\r", ch);
		if (IS_CLASS(ch,PC_CLASS_REAVER) && (sword != NULL))	/* Reaver */
		equip_char (ch, sword, WEAR_WIELD_R);
	}



      if (IS_CLASS(ch,PC_CLASS_ASSASSIN) || ch->race != PC_RACE_AVATAR)
      {
		ch->pcdata->avatar_type = 2; //Assassin Avatars are forced to choose the pk type
        send_to_desc (d,
 		    "\n\rPlease Choose the Alignment of your Character:\n\r");
        send_to_desc (d, "       [`oG``]ood\n\r");
        send_to_desc (d, "       [`oE``]vil\n\r");
        send_to_desc (d, "       [`oN``]eutral\n\r");
        send_to_desc (d, "\n\rWhich Alignment? ");
        d->connected = CON_GET_ALIGNMENT;
      }
      else
      {
		send_to_desc (d, "\n\rPlease Choose the Pk Status of your Avatar:\n\r");
		send_to_desc (d, "    [`oP``]k\n\r");
		send_to_desc (d, "    [`oN``]opk\n\r");
		send_to_desc (d, "\n\rWhich choice? ");
		d->connected = CON_GET_AVATAR_TYPE;
      }
}

void do_con_get_avatar_type(DESCRIPTOR_DATA * d, char *argument)
{
	CHAR_DATA *ch;
	ch = d->character;
	if (argument[0] == 'N' || argument[0] == 'n')
	{
		ch->pcdata->avatar_type = 1;
		send_to_desc (d,"\n\rPlease Choose the Alignment of your Character:\n\r");
		send_to_desc (d, "       [`oG``]ood\n\r");
		send_to_desc (d, "       [`oE``]vil\n\r");
		send_to_desc (d, "       [`oN``]eutral\n\r");
		send_to_desc (d, "\n\rWhich Alignment? ");
		d->connected = CON_GET_ALIGNMENT;
	}
	else if (argument[0] == 'P' || argument[0] == 'p')
	{
		send_to_desc (d, "\n\rPlease Choose the Alignment of your Character:\n\r");
		send_to_desc (d, "       [`oG``]ood\n\r");
		send_to_desc (d, "       [`oE``]vil\n\r");
		send_to_desc (d, "       [`oN``]eutral\n\r");
		send_to_desc (d, "\n\rWhich Alignment? ");
		d->connected = CON_GET_ALIGNMENT;
	}
	else
	{
		send_to_desc (d, "`iInvalid Choice.  Please type 'N' or 'P'.``\n\r");
		send_to_desc (d, "\n\rPlease Choose the Pk Status of your Avatar:\n\r");
		send_to_desc (d, "    [`oP``]k\n\r");
		send_to_desc (d, "    [`oN``]opk\n\r");
		send_to_desc (d, "\n\rWhich choice? ");
		d->connected = CON_GET_AVATAR_TYPE;
	}
}

void do_con_get_new_class(DESCRIPTOR_DATA * d, char *argument)
{
	char tcbuf[MAX_STRING_LENGTH];
	int iClass;
	CHAR_DATA *ch;
	ch = d->character;

	iClass = Class_lookup (argument);

	if (iClass != -1)
		switch (iClass)
		{
			default:
				iClass = -1;
				break;

			case 0:
				if (!IS_SET (genraces[ch->race], CLASS_MAGE))
				iClass = -2;
				break;
			case 1:
				if (!IS_SET (genraces[ch->race], CLASS_CLERIC))
				iClass = -2;
				break;
			case 2:
				if (!IS_SET (genraces[ch->race], CLASS_THIEF))
				iClass = -2;
				break;
			case 3:
				if (!IS_SET (genraces[ch->race], CLASS_WARRIOR))
				iClass = -2;
				break;
			case 4:
				if (!IS_SET (genraces[ch->race], CLASS_RANGER))
				iClass = -2;
				break;
			case 5:
				if (!IS_SET (genraces[ch->race], CLASS_BARD))
				iClass = -2;
				break;
			case 6:
				if (!IS_SET (genraces[ch->race], CLASS_PALADIN))
				iClass = -2;
				break;
			case 7:
				if (!IS_SET (genraces[ch->race], CLASS_ASSASSIN))
				iClass = -2;
				break;
			case 8:
				if (!IS_SET (genraces[ch->race], CLASS_REAVER))
				iClass = -2;
				break;
			case 9:
				if (!IS_SET (genraces[ch->race], CLASS_MONK))
				iClass = -2;
				break;
			case PC_CLASS_DRUID:
				if (!IS_SET (genraces[ch->race], CLASS_DRUID))
				iClass = -2;
				break;
			case 11:
				iClass = -1;
				break;
		}

	if (iClass == -1 || iClass == -2)
	{
		if (d->ansi)
			write_to_buffer (d, ANSI_CLSHOME, 0);
		send_to_desc (d, "\n\r");
		send_to_desc (d," __^__                                                            __^__\n\r");
		send_to_desc (d,"( ___ )----------------------------------------------------------( ___ )\n\r");
		send_to_desc (d," | / |                                                            | \\ |\n\r");
		send_to_desc (d," | / |                Please Choose a Class:                      | \\ |\n\r");
		send_to_desc (d," | / |------------------------------------------------------------| \\ |\n\r");
		send_to_desc (d," | / |                                                            | \\ |\n\r");
		sprintf (tcbuf,
			" | / |             %s       %s                    | \\ |\n\r",
			IS_SET (genraces[ch->race],
			CLASS_MAGE) ? "[`oM``]agic-User" :
			"`aMagic-User``  ", IS_SET (genraces[ch->race],
			CLASS_CLERIC) ?
			"[`oC``]leric" : "`aCleric``  ");
		send_to_desc (d, tcbuf);
		sprintf (tcbuf,
			" | / |             %s            %s                   | \\ |\n\r",
			IS_SET (genraces[ch->race],
			CLASS_THIEF) ? "[`oT``]hief" : "`aThief``  ",
			IS_SET (genraces[ch->race],
			CLASS_WARRIOR) ? "[`oW``]arrior" :
			"`aWarrior``  ");
		send_to_desc (d, tcbuf);
		sprintf (tcbuf,
			" | / |             %s           %s                      | \\ |\n\r",
			IS_SET (genraces[ch->race],
			CLASS_RANGER) ? "[`oR``]anger" :
			"`aRanger``  ", IS_SET (genraces[ch->race],
			CLASS_BARD) ? "[`oB``]ard" :
			"`aBard``  ");
		send_to_desc (d, tcbuf);
		sprintf (tcbuf,
			" | / |             %s          %s                  | \\ |\n\r",
			IS_SET (genraces[ch->race],
			CLASS_PALADIN) ? "[`oP``]aladin" :
			"`aPaladin``  ", IS_SET (genraces[ch->race],
			CLASS_ASSASSIN) ?
			"[`oA``]ssassin" : "`aAssassin``  ");
		send_to_desc (d, tcbuf);
		sprintf (tcbuf,
			" | / |             %s           %s                      | \\ |\n\r",
			IS_SET (genraces[ch->race],
			CLASS_REAVER) ? "[`oRE``]aver" :
			"`aReaver``  ", IS_SET (genraces[ch->race],
			CLASS_MONK) ? "[`oMO``]nk" :
			"`aMonk``  ");
		send_to_desc (d, tcbuf);
		sprintf (tcbuf,
			" | / |             %s                                        | \\ |\n\r",
			IS_SET (genraces[ch->race],
			CLASS_DRUID) ? "[`oD``]ruid" :
			"`aDruid``  ");
		send_to_desc (d, tcbuf);
		send_to_desc (d," |___|                                                            |___|\n\r");
		send_to_desc (d,"(_____)----------------------------------------------------------(_____)\n\r\n\r");
		if (iClass == -1)
			send_to_desc (d, "That is not a valid Class.\n\r");
		else
			send_to_desc (d,"Your race selection prevents you from choosing that Class.\n\r");
		send_to_desc (d, "What Class would you like to be? ");
		return;
	}

	ch->Class = iClass;
	if (ch->Class == 8)
	{			/* Reaver */
		if (d->ansi)
			write_to_buffer (d, ANSI_CLSHOME, 0);
		send_to_desc (d,
		"\n\r  `oA Reaver's sword is more than just a weapon, it is an extension\n\r");
		send_to_desc (d,
		"of yourself.  Your sword will be the only weapon that you will\n\r");
		send_to_desc (d,
		"wield in the realms, and for this reason it is important that your\n\r");
		send_to_desc (d,
		"sword's name reflect its personality.  Choose carefully!  You will\n\r");
		send_to_desc (d,
		"not be able to alter the name of your blade after you enter the game.\n\r");
		send_to_desc (d,
		"`aFeel free to colorize the swords name using the exodus color codes!\n\r");
		send_to_desc (d, "\n\r``What shall you name your sword? ");
		d->connected = CON_SWORD_GENERATION;
	}
	else
	{
		if (d->ansi)
			write_to_buffer (d, ANSI_CLSHOME, 0);
		if (d->character->pcdata->br_points != -1)
		{
			send_to_desc (d, "\n\rPlease Choose a Gender:\n\r");
			send_to_desc (d, "       [`oM``]ale\n\r");
			send_to_desc (d, "       [`oF``]emale\n\r");
			send_to_desc (d, "\n\rWhat is your sex? ");
			d->connected = CON_GET_NEW_SEX;
		}
		else if (ch->Class != PC_RACE_AVATAR || IS_CLASS(ch,PC_CLASS_ASSASSIN))
		{
			ch->pcdata->avatar_type = 2;
			send_to_desc (d,
			"\n\rPlease Choose the Alignment of your Character:\n\r");
			send_to_desc (d, "       [`oG``]ood\n\r");
			send_to_desc (d, "       [`oE``]vil\n\r");
			send_to_desc (d, "       [`oN``]eutral\n\r");
			send_to_desc (d, "\n\rWhich Alignment? ");
			d->connected = CON_GET_ALIGNMENT;
		}
		else 
		{ 
			send_to_desc (d, "\n\rPlease Choose the Pk Status of your Avatar:\n\r");
			send_to_desc (d, "    [`oP``]k\n\r");
			send_to_desc (d, "    [`oN``]opk\n\r");
			send_to_desc (d, "\n\rWhich choice? ");
			d->connected = CON_GET_AVATAR_TYPE;
		}
	}
}

void do_con_get_alignment (DESCRIPTOR_DATA * d, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	int i;
	CHAR_DATA *ch;
	ch = d->character;

	switch (argument[0])
	{
		case 'g':
		case 'G':
			ch->alignment = 1000;
			break;
		case 'n':
		case 'N':
			ch->alignment = 0;
			break;
		case 'e':
		case 'E':
			ch->alignment = -1000;
			break;
		default:
			send_to_desc (d, "That's not a valid alignment.\n\r");
			send_to_desc (d, "\n\rPlease Choose the Alignment of your Character:\n\r");
			send_to_desc (d, "       [`oG``]ood\n\r");
			send_to_desc (d, "       [`oE``]vil\n\r");
			send_to_desc (d, "       [`oN``]eutral\n\r");
			send_to_desc (d, "\n\rWhich Alignment? ");
			return;
	}
	
	if (ch->alignment < Class_align[ch->Class].align_min)
	{
		send_to_desc (d, "That's not a valid alignment for your Class.\n\r");
		send_to_desc (d, "\n\rPlease Choose the Alignment of your Character:\n\r");
		send_to_desc (d, "       [`oG``]ood\n\r");
		send_to_desc (d, "       [`oE``]vil\n\r");
		send_to_desc (d, "       [`oN``]eutral\n\r");
		send_to_desc (d, "\n\rWhich Alignment? ");
		return;
	}

	if (ch->alignment > Class_align[ch->Class].align_max)
	{
		send_to_desc (d, "That's not a valid alignment for your Class.\n\r");
		send_to_desc (d, "\n\rPlease Choose the Alignment of your Character:\n\r");
		send_to_desc (d, "       [`oG``]ood\n\r");
		send_to_desc (d, "       [`oE``]vil\n\r");
		send_to_desc (d, "       [`oN``]eutral\n\r");
		send_to_desc (d, "\n\rWhich Alignment? ");
		return;
	}

	if (ch->alignment < genstats[ch->race].align_min)
	{
		send_to_desc (d, "That's not a valid alignment for your race.\n\r");
		send_to_desc (d, "\n\rPlease Choose the Alignment of your Character:\n\r");
		send_to_desc (d, "       [`oG``]ood\n\r");
		send_to_desc (d, "       [`oE``]vil\n\r");
		send_to_desc (d, "       [`oN``]eutral\n\r");
		send_to_desc (d, "\n\rWhich Alignment? ");
		return;
	}

	if (ch->alignment > genstats[ch->race].align_max)
	{
		send_to_desc (d, "That's not a valid alignment for your race.\n\r");
		send_to_desc (d, "\n\rPlease Choose the Alignment of your Character:\n\r");
		send_to_desc (d, "       [`oG``]ood\n\r");
		send_to_desc (d, "       [`oE``]vil\n\r");
		send_to_desc (d, "       [`oN``]eutral\n\r");
		send_to_desc (d, "\n\rWhich Alignment? ");
		return;
	}

	if (ch->pcdata->br_points != -1)
		sprintf (log_buf, "%s@%s new player.", d->character->name, d->host);
	else
		sprintf (log_buf, "%s@%s reincarnated player.",
	
	d->character->name, d->host);
	remove_create (d->character->name);
	log_string (log_buf);
	wiznet ("Newbie alert!  $N sighted.", d->character, NULL,
	WIZ_NEWBIE, 0, 0);
	wiznet (log_buf, NULL, NULL, WIZ_SITES, 0, get_trust (d->character));

	//IBLIS 5/17/03
	send_to_desc (d,"\n`bDon't be alarmed about not being allowed to chose skills/spells.\n\r");
	send_to_desc (d,"Exodus has migrated to a totally new experience-based system, in which\n\r");
	send_to_desc (d,"you start with all skills at 1%.  \"Help experience\" once you're in will explain.``\n\r");
	group_add_all (d->character);
	write_to_buffer (d, "\n\r", 2);

	if (ch->race != PC_RACE_NERIX) 
	{
		write_to_buffer (d,	"Please pick a weapon from the following choices:\n\r",	0);
		buf[0] = '\0';
		for (i = 0; weapon_table[i].name != NULL; i++)
		{
			if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
			{
				strcat (buf, weapon_table[i].name);
				strcat (buf, " ");
			}
		}

		strcat (buf, "\n\rYour choice? ");
		write_to_buffer (d, buf, 0);

		if (d->character->pcdata->br_points != -1)
		{
			d->character->gen_data = new_gen_data ();
			d->character->gen_data->points_chosen =
			d->character->pcdata->points;
		}

		d->connected = CON_PICK_WEAPON;
	}
	else 
	{
		if (ch->pcdata->br_points != -1)
		{
			do_help (ch, "motd");
		}
		else
		{
			do_reroll (ch, "");
			send_to_char("You have been successfully reicarnated.  Press return to continue.\r\n", ch);
			send_to_char("AFTER YOU HIT RETURN TO FINISH REINCARNATING, then you may type `kreroll`` to reroll\r\n",ch);
		}
		d->connected = CON_READ_MOTD;
	}
}


void do_con_pick_weapon(DESCRIPTOR_DATA * d, char *argument)
{
	int i, weapon;

	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	ch = d->character;

	write_to_buffer (d, "\n\r", 2);
	weapon = weapon_lookup (argument);
	if (weapon == -1 ||	ch->pcdata->learned[*weapon_table[weapon].gsn] <= 0)
	{
		write_to_buffer (d,	"That's not a valid selection. Choices are:\n\r",	0);
		buf[0] = '\0';
		for (i = 0; weapon_table[i].name != NULL; i++)
		{
			if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
			{
				strcat (buf, weapon_table[i].name);
				strcat (buf, ", ");
			}
		}

		strcat (buf, "\n\rYour choice? ");
		write_to_buffer (d, buf, 0);
		return;
	}

	ch->pcdata->learned[*weapon_table[weapon].gsn] = 40;
	write_to_buffer (d, "\n\r", 2);

	if (ch->pcdata->br_points != -1)
	{
		do_help (ch, "motd");
	}
	else
	{
		do_reroll (ch, "");
		send_to_char("You have been successfully reicarnated.  Press return to continue.\r\n",	ch);
		send_to_char("AFTER YOU HIT RETURN TO FINISH REINCARNATING, then you may type `kreroll`` to reroll.\r\n",	ch);
	}

	d->connected = CON_READ_MOTD;
	
}

void do_con_read_imotd(DESCRIPTOR_DATA * d, char *argument)
{
	CHAR_DATA *ch;

	ch = d->character;
	write_to_buffer (d, "\n\r", 2);
	if (ch->pcdata->br_points != -1)
		do_help (ch, "motd");
	d->connected = CON_READ_MOTD;
}

void do_con_read_motd(DESCRIPTOR_DATA * d, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	char tcbuf[MAX_STRING_LENGTH];
	int i;
	OBJ_DATA *obj, *pobj, *obj_next, *container;
	CHAR_DATA *ch, *vch, *vch_next;
	ch = d->character;
	
    if (check_twolinks (d))
    {
		send_to_char("`oAnother link is already active from your location.\n\r", ch);
		send_to_char("`oRemember, that Exodus is a single player game, and\n\r", ch);
		send_to_char("`othat a linkdead character is a playing character.``\n\r", ch);
	}
	
	if (ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
	{
		write_to_buffer (d, "Warning! Null password!\n\r", 0);
		write_to_buffer (d, "Please report old password with bug.\n\r", 0);
		write_to_buffer (d, "Type 'password null <new password>' to fix.\n\r", 0);
	}

	if (ch->pcdata->br_points != -1)
	{
		ch->next = char_list;
		char_list = ch;
		reset_char (ch);
	}
      
	d->connected = CON_PLAYING;
      
	if (ch->pcdata->has_reincarnated == TRUE && ch->race == PC_RACE_LITAN)
	{
		int sn=0;
		OBJ_DATA *obj,*obj_next;
//Iblis - Need to convert this Thyrent to a Litan
		ch->pcdata->has_reincarnated = FALSE;
		ch->level = 1;
		for (sn=0;sn < MAX_SKILL;sn++)
		{
			ch->pcdata->learned[sn] = 0;
		}

		for (obj = ch->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;

			if (IS_CLASS(ch,PC_CLASS_REAVER) && (obj->wear_loc == WEAR_WIELD_L || obj->wear_loc == WEAR_WIELD_R))
			{
				unequip_char (ch,obj);
				obj_from_char (obj);
				extract_obj (obj);
			}

			if (obj->wear_loc != WEAR_NONE)
			  unequip_char (ch, obj);
		}

		ch->pcdata->hp_gained = 0;
		ch->pcdata->mana_gained = 0;
		ch->pcdata->move_gained = 0;
		while (ch->affected)
			affect_remove (ch, ch->affected);

		ch->res_flags ^= ch->res_flags & RES_PIERCE;
		ch->vuln_flags ^= ch->vuln_flags & (VULN_COLD | VULN_FIRE);

		ch->size = pc_race_table[ch->race].size;
		ch->affected_by = ch->affected_by | race_table[ch->race].aff;
		ch->imm_flags = ch->imm_flags | race_table[ch->race].imm;
		ch->res_flags = ch->res_flags | race_table[ch->race].res;
		ch->vuln_flags = ch->vuln_flags | race_table[ch->race].vuln;
		ch->act = ch->act | race_table[ch->race].act;
		if (ch->race == PC_RACE_SWARM)
			  ch->act2 = ch->act2 | ACT_NOMOVE;
		ch->form = race_table[ch->race].form;
		ch->parts = race_table[ch->race].parts;
		ch->exp = ch->pcdata->totalxp;
		ch->max_hit = 20;
		ch->hit = 20;
		ch->max_mana = 100;
		ch->mana = 100;
		ch->max_move = 100;
		ch->move = 100;
		ch->pcdata->perm_hit = 20;
		ch->pcdata->perm_mana = 100;
		ch->pcdata->perm_move = 100;
		group_add_all (ch);
		do_reroll(ch,"");
		send_to_char("Thyrents are no more.  Litans are born.\n\r",ch);
		send_to_char("You were converted to a Litan for free.  If you have any questions, talk to an Immortal.\n\r",ch);
	}

	//Iblis 1/19/04 - If Reavers have no totalsoul, back calculate it
	if (IS_CLASS(ch,PC_CLASS_REAVER) && ch->pcdata->totalsouls == 0)
	{
		backcalc_totalsouls(ch);
	}
        
	
	if (ch->race == 4)
		if (!IS_SET (ch->res_flags, RES_ENTANGLE))
			SET_BIT (ch->res_flags, RES_ENTANGLE);
      
	if (is_affected (ch,gsn_aquatitus))
        affect_strip(ch,gsn_aquatitus);
	if (ch->pcdata->quest_ticks > 0)
        ch->pcdata->quest_ticks = -10;
	if (ch->race == PC_RACE_LITAN
		&& weather_info.sky == SKY_CLOUDLESS
		&& !IS_AFFECTED(ch,AFF_HASTE)
	    && time_info.hour >= 5 && time_info.hour < 19)
			spell_haste(gsn_aquatitus,ch->level,ch,(void *)ch,TARGET_CHAR);
	      
	if (is_affected (ch,skill_lookup("beacon of the damned")))
        affect_strip(ch,skill_lookup("beacon of the damned"));
	
	ch->position = POS_STANDING;
	if (ch->alignment < -400)
	{
		OBJ_INDEX_DATA *oid;
		oid = get_obj_index (OBJ_VNUM_BEACON);
		if (oid && oid->count > 0)
		{
			AFFECT_DATA af;
			af.where = TO_AFFECTS;
			af.type = skill_lookup("beacon of the damned");
			af.level = ch->level;
			af.duration = 15;
			af.bitvector = 0;
			af.permaff = FALSE;
			af.composition = FALSE;
			af.comp_name = str_dup ("");
			af.location = APPLY_SAVES;
			af.modifier = -5;
			affect_to_char (ch, &af);
			af.location = APPLY_AC;
			if (IS_SET(ch->act,ACT_UNDEAD))
			af.modifier = -20;
			else af.modifier = -10;
			affect_to_char (ch, &af);
			af.location = APPLY_STR;
			af.modifier = 1;
			affect_to_char (ch, &af);
			af.location = APPLY_INT;
			affect_to_char (ch, &af);
			af.location = APPLY_WIS;
			affect_to_char (ch, &af);
			af.location = APPLY_DEX;
			affect_to_char (ch, &af);
			af.location = APPLY_CON;
			affect_to_char (ch, &af);
			af.location = APPLY_CHA;
			affect_to_char (ch, &af);
			send_to_char("Your whole being shivers with a surge of power.\n\r",ch);
		}
    }

	if (ch->race == PC_RACE_KALIAN)
	{
		affect_strip (ch, gsn_jurgnation);
		affect_strip (ch, gsn_jalknation);
	}
	if (ch->race == PC_RACE_KALIAN)
	{
		if (time_info.hour > 6 && time_info.hour < 19)
		{
			AFFECT_DATA af;
			af.where = TO_AFFECTS;
			af.type = gsn_jurgnation;
			af.level = 4;
			af.duration = 1;
			af.location = APPLY_STR;
			af.modifier = -3;
			af.bitvector = 0;
			af.permaff = FALSE;
			af.composition = FALSE;
			af.comp_name = str_dup ("");
			affect_to_char (d->character, &af);
			send_to_char("The daytime begins to wear you down.\n\r",d->character);
		}
		else if (time_info.hour < 6)
		{
			AFFECT_DATA af;
			af.where = TO_AFFECTS;
			af.type = gsn_jalknation;
			af.level = 4;
			af.duration = 1;
			af.location = APPLY_DAMROLL;
			af.modifier = d->character->level/10;
			af.bitvector = 0;
			af.permaff = FALSE;
			af.composition = FALSE;
			af.comp_name = str_dup ("");
			affect_to_char (d->character, &af);
			af.location = APPLY_HITROLL;
			affect_to_char (d->character, &af);
			send_to_char ("You feel invigorated by the night.\n\r", d->character);
		}	

	}

	//IBLIS 6/03/03 - Monks have resist poison naturallyQ
	if (IS_CLASS (ch, PC_CLASS_MONK))
		if (!IS_SET (ch->res_flags, RES_POISON))
			SET_BIT (ch->res_flags, RES_POISON);
	if (ch->clan != CLAN_BOGUS)
		ch->pcdata->loner = TRUE;
	  
	group_add_all (ch);

  //Iblis 8/10/04 - Fix any possible cardgame problems, due to a crash
  //etc in the middle of a cardgame

	if ((container = get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zhand")))
	{
		for (obj = container->contains;obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			obj_from_obj(obj);
			obj_to_char(obj,ch);
		}
		obj_from_char(container);
		extract_obj(container);
	}

	if ((container = get_obj_vnum_string_char(ch,OBJ_VNUM_BINDER,"zdiscard")))
	{
		for (obj = container->contains;obj != NULL; obj = obj_next)
		{
 			obj_next = obj->next_content;
            obj_from_obj(obj);
			obj_to_char(obj,ch);
		}
		obj_from_char(container);
		extract_obj(container);
	}

	if ((container = get_obj_vnum_char(ch,OBJ_VNUM_ANTE_BAG)))
	{
		for (obj = container->contains;obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			obj_from_obj(obj);
			obj_to_char(obj,ch);
		}
		if (container->value[5] > 0)
		  ch->silver += container->value[5];
		if (container->value[6] > 0)
		  ch->gold += container->value[6];
		obj_from_char(container);
		extract_obj(container);
	}

	if ((container = get_obj_vnum_char(ch, OBJ_VNUM_DECK)))
	{
	  if (IS_SET(container->extra_flags[1],ITEM_WIZI))
 	    REMOVE_BIT(container->extra_flags[1],ITEM_WIZI);
	}
	if ((container = get_obj_vnum_char(ch, OBJ_VNUM_BINDER)))
	{
          if (IS_SET(container->extra_flags[1],ITEM_WIZI))
            REMOVE_BIT(container->extra_flags[1],ITEM_WIZI);
	}
      
//Iblis 8/30/04 - If a player doesn't have a deck/binder, give them one

	if (!(container = get_obj_vnum_char(ch, OBJ_VNUM_BINDER)))
	{
		container = create_object(get_obj_index(OBJ_VNUM_BINDER),0);
		obj_to_char(container,ch);
		send_to_char("You've been given a binder.\n\r",ch);
	}

	if (!(container = get_obj_vnum_char(ch, OBJ_VNUM_DECK)))
	{
		container = create_object(get_obj_index(OBJ_VNUM_DECK),0);
		obj_to_char(container,ch);
		send_to_char("You've been given a deck.\n\r",ch);
	}



//Iblis - 9/03/04 - Temp card fix
	for (pobj = ch->carrying;pobj != NULL; pobj = pobj->next_content)
	{
	  if (pobj->contains != NULL)
	    {
	    for (obj = pobj->contains; obj != NULL;obj = obj->next_content)
	      {
		if (obj->item_type == ITEM_CARD)
		  {
		    //	case ITEM_CARD:
		    if (obj->pIndexData->vnum == OBJ_VNUM_COMMON_CARD)
		      i = 9;
		    else i = 10;
		    if (obj->value[8] > i)
		      {
			obj->value[9] = obj->value[8]-i;
			obj->value[8] = i;
		      }
		    if (obj->value[9] > i)
		      {
			obj->value[8] = obj->value[9]-i;
			obj->value[9] = i;
		      }
		    //	  break;
		  }
	      }
	    }
	  if (pobj->item_type == ITEM_CARD)
	    {
	      if (pobj->pIndexData->vnum == OBJ_VNUM_COMMON_CARD)
		i = 9;
	      else i = 10;
	      if (pobj->value[8] > i)
		{
		  pobj->value[9] = pobj->value[8]-i;
		  pobj->value[8] = i;
		}
	      if (pobj->value[9] > i)
		{
		  pobj->value[8] = pobj->value[9]-i;
		  pobj->value[9] = i;
		}
	    }
	}

      
      
      if (ch->Class2 == -1) //|| ch->level < 31)
	ch->Class2 = ch->Class;
      if (ch->Class3 == -1) //|| ch->level < 61)
	ch->Class3 = ch->Class;
      if (ch->level == 0)
	{

	  ch->Class2 = ch->Class;
	  ch->Class3 = ch->Class;
	  if (!ch->pcdata->new_style)
	    {
	      SET_BIT (ch->act, PLR_AUTOEXIT);
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
	      ch->exp = 10000;
	      ch->pcdata->totalxp = ch->exp;
	      do_outfit (ch, "");
	      obj_to_char (create_object
			   (get_obj_index (OBJ_VNUM_START_FOOD), 0), ch);
	      obj_to_char (create_object
			   (get_obj_index (OBJ_VNUM_START_FOOD), 0), ch);
	      obj_to_char (create_object
			   (get_obj_index (OBJ_VNUM_START_DRINK), 0), ch);
	      obj_to_char (create_object
			   (get_obj_index (OBJ_VNUM_MANIFESTO), 0), ch);
	      ch->silver = 60;
	      set_title (ch, "the Neophyte");
	      char_to_room (ch, get_room_index (ROOM_VNUM_SCHOOL));
	      trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	      sprintf(tcbuf,"%d",ch->in_room->vnum);
	      trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
	    }

	  else
	    {
	      sprintf (buf, "%s %d", ch->name, ROOM_VNUM_SCHOOL);
	      do_transfer (ch, buf);
	      ch->max_hit = 20;
	      if (ch->pcdata)
		{
		  ch->pcdata->perm_hit = 20;
		  ch->pcdata->perm_move = 100;
		  ch->pcdata->perm_mana = 100;
		}
	      ch->max_move = 100;
	      ch->max_mana = 100;
	      save_char_obj (ch);
	    }

	  //This is used to signal a new style character
	  ch->pcdata->new_style = 1;
	  ch->level = 1;
	  ch->hit = ch->max_hit;
	  ch->mana = ch->max_mana;
	  ch->move = ch->max_move;

	  //IBLIS 5/18/03 - Changed for new xp system.
	  ch->train = 2;
	  if (ch->race == PC_RACE_NERIX)
		  ch->dam_type = 41;
	  else ch->dam_type = 17;

	  send_to_char ("\n\r", ch);

	}

      else if (ch->in_room != NULL)
	{

	  //IBLIS 5/19/03 - Does the dirty shit here for old chars
	  if (ch->pcdata->new_style == 0 && !IS_IMMORTAL (ch))
	    {

	      sprintf (buf, "%s", ch->name);
	      do_pardon (ch, buf);
	      sprintf (buf, "%s jailed 999999", ch->name);
	      do_mset (ch, buf);

	      if (IS_SET (ch->comm, COMM_NOCHANNELS))
		REMOVE_BIT (ch->comm, COMM_NOCHANNELS);
	      if (IS_SET (ch->comm2, COMM_SHUTUP))
		REMOVE_BIT (ch->comm2, COMM_SHUTUP);

	      send_to_char
		("`o--------------------------------------------------------------------------------\n\r",
		 ch);
	      send_to_char
		("`o--------------------------------------------------------------------------------\n\r",
		 ch);
	      send_to_char
		("Exodus is running under a new experience system.  As a result, all old-style\n\r",
		 ch);
	      send_to_char
		("characters must be converted into the new style.  `iYOU WILL `wNOT `iBE FORCED TO\n\r",
		 ch);
	      send_to_char
		("CONVERT.  `oConversion will only happen if you elect to do so by typing '`kreroll\n\r",
		 ch);
	      send_to_char
		("all`o'  If you choose to convert, the following things will happen to you:\n\r",
		 ch);
	      send_to_char
		("\n\rYou will be moved to the Thesden Town Hall\n\r", ch);
	      send_to_char ("Your stats will be rerolled\n\r", ch);
	      send_to_char ("You will be set to level 1\n\r", ch);
	      send_to_char
		("Your hp/mp/mv will be reset to that of a level 1\n\r", ch);
	      send_to_char
		("Loners will become nonclan for safety reasons\n\r", ch);
	      send_to_char ("You will stay the same race and Class\n\r", ch);
	      send_to_char ("You will keep all money in the bank\n\r", ch);
	      send_to_char ("You will keep your accumulated hours\n\r", ch);
	      send_to_char
		("You will keep your equipment (be sure to store OOL eq before you quit)\n\r",
		 ch);
	      send_to_char
		("You will be compensated for your level and skills/spells/prayers/songs/chants\n\r",
		 ch);
	      send_to_char
		("This compensation will be in the form of a lump sum of exp.\n\r",
		 ch);
	      send_to_char
		("\n\rThis compensation will not restore you back to level 90, but it will give\n\r",
		 ch);
	      send_to_char
		("you a distinct advantage over players who were below you under the old\n\r",
		 ch);
	      send_to_char
		("system.  If you do not like your stats, you may '`kreroll`o' as often as you wish\n\r",
		 ch);
	      send_to_char ("as long as you are level 1.\n\r", ch);
	      send_to_char
		("\n\r`iIf there is a problem or if you have any questions, please DO NOT CONVERT\n\r",
		 ch);
	      send_to_char
		("until you have spoken with an immortal.\n\r", ch);
	      send_to_char
		("`o--------------------------------------------------------------------------------\n\r",
		 ch);
	      send_to_char
		("`o--------------------------------------------------------------------------------\n\r",
		 ch);
	    }

	  else
	  {
  	    
	    ROOM_INDEX_DATA *troom=ch->in_room;
	    if (ch->in_room)
  	      char_from_room(ch);
	    if (IS_SET(troom->area->area_flags,AREA_NO_QUIT) 
			    || IS_SET(troom->room_flags,ROOM_ELEVATOR))
	    {
		    troom = get_room_index (ROOM_VNUM_ALTAR);
		    send_to_char("Close..but no cigar.\n\r",ch);
	    }
	    char_to_room (ch, troom);
	    if (ch->in_room->vnum == ROOM_VNUM_LIMBO)
 	      char_to_room (ch, get_room_index (ROOM_VNUM_TEMPLE));
	    trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	    sprintf(tcbuf,"%d",ch->in_room->vnum);
	    trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
	    check_aggression(ch);
	  }
	}

      else if (IS_IMMORTAL (ch))
	{
	  if (ch->in_room)
   	    char_from_room(ch);
	  char_to_room (ch, get_room_index (ROOM_VNUM_CHAT));
	  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	  sprintf(tcbuf,"%d",ch->in_room->vnum);
	  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
	  check_aggression(ch);
	}

      else
	{
	  if (ch->pcdata->br_points == -1)
	  {
   	    if (ch->in_room)
	      char_from_room(ch);
	  }
	  else if (ch->in_room)
  	    char_from_room(ch);
	  char_to_room (ch, get_room_index (ROOM_VNUM_TEMPLE));
	  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
	  sprintf(tcbuf,"%d",ch->in_room->vnum);
	  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
	  check_aggression(ch);
	}
      sprintf (buf, "%s has entered the game.\n\r", ch->name);
      //Iblis 10/04/04 - HERE was the stupid nplayer bug that has been haunting me.  How
      //  the hell the player gets in the room if not using char_to_room is freaking beyond me
      //  ..plus I'm not looking at the moment
      if (ch->in_room)
      {
        ++ch->in_room->area->nplayer;
	 //do_echo(ch,"num_players++");
      }

      //Iblis 10/04/04 - SCAN TO SEE IF IMAGINARY FRIENDS SHOULD POOF
	if (ch->in_room->area->nplayer > 1)
	{
	  CHAR_DATA *mch;
	  for (mch = char_list;mch != NULL;mch = mch->next)
		{
		  if (IS_NPC(mch))
		continue;
		  if (!mch->in_room)
		continue;
		  if (mch->Class == PC_CLASS_CHAOS_JESTER && mch->in_room->area == ch->in_room->area
		  && mch->pcdata->familiar)
		{
		  act("$N vanishes into thin air.",ch,NULL,mch->pcdata->familiar,TO_ROOM);
		  familiar_poof(mch);
		}
		}
	  }

      if (battle_royale && (ch->level > 19 || (!IS_NPC(ch) && ch->pcdata->loner)))
	do_restore (ch, ch->name);
      for (vch = char_list; vch != NULL; vch = vch_next)
	{
	  vch_next = vch->next;
	  if (vch->in_room == NULL)
	    continue;
	  if (vch->in_room == ch->in_room && can_see (vch, ch) && ch != vch)
	    {
	      send_to_char (buf, vch);
	      continue;
	    }
	}

      do_look (ch, "auto");
      wiznet ("$N has left real life behind.", ch, NULL, WIZ_LOGINS,
	      WIZ_SITES, get_trust (ch));
      if (ch->pet != NULL)
	{
	  if (ch->pet->in_room)
  	    char_from_room(ch->pet);
	  char_to_room (ch->pet, ch->in_room);
	  act ("$n has entered the game.", ch->pet, NULL, NULL, TO_ROOM);
	}
      if (ch->pcdata->br_points != -1)
	do_unread (ch, "");
      ch->pcdata->br_points = 0;

      update_count ();
}

void do_con_sword_generation(DESCRIPTOR_DATA * d, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	ch = d->character;

	if (argument[0] == '\0')
	{
		send_to_desc (d, "What shall you name your sword? ");
		return;
	}

	sprintf (buf, "\n\rYou have named your blade %s\n\r", argument);
	send_to_desc (d, buf);
	send_to_desc (d, "Is that correct (Y/N)? ");
	ch->afkmsg = str_dup (argument);	//hack to store this without wasting mem on new var
	d->connected = CON_SWORD_CONFIRM;
	return;
}

void do_con_sword_confirm(DESCRIPTOR_DATA * d, char *argument)
{
	int i;
	char *sword_name;
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	ch = d->character;

	if (argument[0] == '\0')
	{
		send_to_desc (d, "Is that correct (Y/N)? ");
		return;
	}
	
	switch (argument[0])
	{
		case 'y':
		case 'Y':
			break;
		case 'n':
		case 'N':
		default:
			send_to_desc (d, "\n\rWhat shall you name your sword? ");
			d->connected = CON_SWORD_GENERATION;
			return;
	}

	sword_name = ch->afkmsg;
	ch->afkmsg = NULL;	//reseting after hack
	ch->sword = create_object (get_obj_index (OBJ_VNUM_REAVER_SWORD), 0);
	sprintf (buf, "`hthe Reaver blade '%s`h'``", sword_name);
	ch->sword->short_descr = str_dup (buf);
	sprintf (buf,
	   "`hThis Reaver blade bares the inscription '%s`h'.``",
	   sword_name);
	ch->sword->description = str_dup (buf);
	ch->sword->cost = 0;

	for (i = 0; i < MAX_BLADE_SPELLS; i++)
	{
		ch->sword->bs_capacity[i] = 0;
		ch->sword->bs_charges[i] = 0;
	}

	for (i = 0; i < MAX_BURST; i++)
	{
		ch->sword->bb_capacity[i] = 0;
		ch->sword->bb_charges[i] = 0;
	}

    if (d->ansi)
		write_to_buffer (d, ANSI_CLSHOME, 0);
	send_to_desc (d, "\n\rPlease Choose a Gender:\n\r");
	send_to_desc (d, "       [`oM``]ale\n\r");
	send_to_desc (d, "       [`oF``]emale\n\r");
	send_to_desc (d, "\n\rWhat is your sex? ");
	d->connected = CON_GET_NEW_SEX;
	return;
}





void nanny (DESCRIPTOR_DATA * d, char *argument)
{

	//NANNY Rewrite by Kalus 5-13-09
	//the goal is to make Nanny as simple as possible.
	//Create the necessary objects that are used commonly here
	//ONLY the ones that are used by all cases of the connection
	//state. everything else should 
	//stay local to the function that is called. This will make everyone's
	//lives much easier. This may actually improve memory handling also, as we
	//won't be declaring a bunch of mostly unused stuff unless it's needed
	//Each function is responsible for setting the connection state.
	//If this is not done, it WILL cause a loop for that state.

	//each of the do_con_xxxx are specific responses to input the character gave
	//OTHER than the main game interpreter. Each do_con is responsible for changing
	//the descriptor's con state to the next step.

  while (isspace (*argument))
    argument++;


  switch (d->connected)
    {
    default:
      bug ("Nanny: bad d->connected %d.", d->connected);
      close_socket (d);
      break;
    case CON_GET_ANSI:
		do_con_get_ansi(d, argument);
		break;
    case CON_GET_NAME:
		do_con_get_name(d, argument);
		break;
    case CON_GET_OLD_PASSWORD:
		do_con_get_old_password(d,argument);
		break;
    case CON_BREAK_CONNECT:
		do_con_break_connect(d,argument);
		break;
    case CON_CONFIRM_NEW_NAME:
		do_con_confirm_new_name(d,argument);
		break;
    case CON_GET_NEW_PASSWORD:
		do_con_get_new_password(d,argument);
		break;
    case CON_CONFIRM_NEW_PASSWORD:
		do_con_confirm_new_password(d,argument);
		break;
    case CON_GET_REFERRAL:
		do_con_get_referral(d,argument);
		break;
    case CON_GET_EMAIL:
		do_con_get_email(d,argument);
		break;
    case CON_GET_NEW_RACE:
		do_con_get_new_race(d,argument);
		break;
    case CON_CONFIRM_STATS:
		do_con_confirm_stats(d,argument);
		break;
    case CON_GET_NEW_SEX:
		do_con_get_new_sex(d,argument);
		break;
    case CON_GET_AVATAR_TYPE:
		do_con_get_avatar_type(d,argument);
		break;
    case CON_GET_NEW_CLASS:
		do_con_get_new_class(d,argument);
		break;
    case CON_GET_ALIGNMENT:
		do_con_get_alignment(d,argument);
		break;
    case CON_PICK_WEAPON:
		do_con_pick_weapon(d,argument);
		break;
    case CON_READ_IMOTD:
		do_con_read_imotd(d,argument);
		break;
    case CON_READ_MOTD:
		do_con_read_motd(d,argument);
		break;
	case CON_SWORD_GENERATION:
		do_con_sword_generation(d,argument);
		break;
    case CON_SWORD_CONFIRM:
		do_con_sword_confirm(d,argument);
		break;
	}
    return;
  }


/*
 * Parse a name for acceptability.
 */
bool check_parse_name (char *name)
{
  int x;
  char name2[MAX_INPUT_LENGTH];
  if (is_name
      (name,
       "all auto withdraw immortal self someone something the "
       "you demise balance circle loner honor bitch shit exodus "
       "test tester demagogue"))
    return FALSE;
  strcpy (name2, name);
  name2[0] = tolower (name2[0]);
  for (x = 0; bad_names[x]; x++)
    {
      if (strstr (name2, bad_names[x]) != NULL)
	return (FALSE);
    }
  if (strlen (name) < 2)
    return FALSE;
  if (strlen (name) > 12)
    return FALSE;

  /*
   * Alphanumerics only.
   * Lock out IllIll twits.
   */
  {
    char *pc;
    bool fIll, adjcaps = FALSE, cleancaps = FALSE;
    int total_caps = 0;
    fIll = TRUE;
    for (pc = name; *pc != '\0'; pc++)
      {
	if (!isalpha (*pc))
	  return FALSE;
	if (isupper (*pc))
	  {			/* ugly anti-caps hack */
	    if (adjcaps)
	      cleancaps = TRUE;
	    total_caps++;
	    adjcaps = TRUE;
	  }

	else
	  adjcaps = FALSE;
	if (LOWER (*pc) != 'i' && LOWER (*pc) != 'l')
	  fIll = FALSE;
      }
    if (fIll)
      return FALSE;
    if (cleancaps || (total_caps > (strlen (name)) / 2 && strlen (name) < 3))
      return FALSE;
  }

  // prevent the use of already existing, reserved, proposed, inactive clan names
  if (clanname_to_slot (name) != CLAN_BOGUS)
    return (FALSE);

  /*
   * Prevent players from naming themselves after mobs.
   */
  {
    extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
    MOB_INDEX_DATA *pMobIndex;
    int iHash;
    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
      {
	for (pMobIndex = mob_index_hash[iHash]; pMobIndex != NULL;
	     pMobIndex = pMobIndex->next)
	  {
	    if (is_name (name, pMobIndex->player_name))
	      return FALSE;
	  }
      }
  }
  return TRUE;
}


/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect (DESCRIPTOR_DATA * d, char *name, bool fConn)
{
  CHAR_DATA *ch, *vch, *vch_next;
  char buf[MAX_STRING_LENGTH];
  for (ch = char_list; ch != NULL; ch = ch->next)
    {
      if (!IS_NPC (ch)
	  && (!fConn || ch->desc == NULL)
	  && !str_cmp (d->character->name, ch->name))
	{
	  if (fConn == FALSE)
	    {
	      free_string (d->character->pcdata->pwd);
	      d->character->pcdata->pwd = str_dup (ch->pcdata->pwd);
	    }

	  else
	    {
	      OBJ_DATA *obj;
	      free_char (d->character);
	      d->character = ch;
	      ch->desc = d;
	      ch->timer = 0;
	      send_to_char
		("Reconnecting. Type replay to see missed tells.\n\r", ch);


	      if (ch->in_room == get_room_index(ROOM_VNUM_SWITCHED_LIMBO))
	      {
		      char_from_room(ch);
		      char_to_room(ch,get_room_index(ROOM_VNUM_ALTAR));
	      }
	      
	      //5-12-03 Iblis - So you only notice reconnecting players you can see
	      //note: had to declare vch and vch_next and buf
	      sprintf (buf, "%s has reconnected.\n\r", ch->name);
	      for (vch = char_list; vch != NULL; vch = vch_next)
		{
		  vch_next = vch->next;
		  if (vch->in_room == NULL)
		    continue;
		  if (vch->in_room == ch->in_room && can_see (vch, ch)
		      && ch != vch)
		    {
		      send_to_char (buf, vch);
		      continue;
		    }
		}

	      //act ("$n has reconnected.", ch, NULL, NULL, TO_ROOM);
	      if ((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL
		  && obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
		++ch->in_room->light;
	      sprintf (log_buf, "%s@%s reconnected.", ch->name, d->host);
	      log_string (log_buf);
	      wiznet ("$N groks the fullness of $S link.", ch, NULL,
		      WIZ_LINKS, 0, 0);
	      d->connected = CON_PLAYING;
	    }
	  return TRUE;
	}
    }
  return FALSE;
}


/*
 * Check if already playing.
 */
bool check_playing (DESCRIPTOR_DATA * d, char *name)
{
  DESCRIPTOR_DATA *dold;
  for (dold = descriptor_list; dold; dold = dold->next)
    {
      if (dold != d
	  && dold->character != NULL
	  && dold->connected != CON_GET_NAME
	  && dold->connected != CON_GET_OLD_PASSWORD
	  && !str_cmp (name, dold->original
		       ? dold->original->name : dold->character->name))
	{
	  write_to_buffer (d, "That character is already playing.\n\r", 0);
	  write_to_buffer (d, "Do you wish to connect anyway (Y/N)?", 0);
	  d->connected = CON_BREAK_CONNECT;
	  return TRUE;
	}
    }
  return FALSE;
}

void stop_idling (CHAR_DATA * ch)
{
  char tcbuf[15];
  if (ch == NULL
      || ch->desc == NULL
      || ch->desc->connected != CON_PLAYING
      || ch->was_in_room == NULL
      || ch->in_room != get_room_index (ROOM_VNUM_LIMBO))
    return;
  ch->timer = 0;
  char_from_room (ch);
  char_to_room (ch, ch->was_in_room);
  ch->was_in_room = NULL;
  act ("$n has returned from the void.", ch, NULL, NULL, TO_ROOM);
  trip_triggers(ch, OBJ_TRIG_ENTER_ROOM, NULL, NULL, OT_SPEC_NONE);
  sprintf(tcbuf,"%d",ch->in_room->vnum);
  trip_triggers_arg(ch, OBJ_TRIG_CHAR_TO_ROOM, NULL, NULL, OT_SPEC_NONE,tcbuf);
  check_aggression(ch);
  return;
}


/*
 * Write to one char.

 void send_to_char( const char *txt, CHAR_DATA *ch )
 {
 if ( txt != NULL && ch->desc != NULL )
 write_to_buffer( ch->desc, txt, strlen(txt) );
 return;
 }
 */
void send_to_char (const char *txt, CHAR_DATA * ch)
{
  const char *a, *b;
  int length, l, curlen = 0;
  a = txt;
  length = strlen (txt);
  if (txt != NULL && ch->desc != NULL)
    {
      while (curlen < length)
	{
	  b = a;
	  l = 0;
	  while (curlen < length && *a != '`')
	    {
	      l++;
	      curlen++;
	      a++;
	    }
	  if (l)
	    write_to_buffer (ch->desc, b, l);
	  if (*a)
	    {
	      a++;
	      curlen++;
	      if (curlen < length && ch->color && !IS_NPC(ch))
		{
		    process_color (ch, *a++);
		    curlen++;
		}

	      else
		{
		  a++;
		  curlen++;
		}
	    }
	}
    }
}
void send_to_desc (DESCRIPTOR_DATA * desc, char *txt)
{
  char *a, *b;
  int length, l, curlen = 0;
  a = txt;
  length = strlen (txt);
  if (txt != NULL && desc != NULL)
    {
      while (curlen < length)
	{
	  b = a;
	  l = 0;
	  while (curlen < length && *a != '`')
	    {
	      l++;
	      curlen++;
	      a++;
	    }
	  if (l)
	    write_to_buffer (desc, b, l);
	  if (*a)
	    {
	      a++;
	      curlen++;
	      if (curlen < length && desc->ansi)
		{
		  process_desc_color (desc, *a++);
		  curlen++;
		}

	      else
		{
		  a++;
		  curlen++;
		}
	    }
	}
    }
}


/*
 * Send a page to one char.
 */
void page_to_char (char *txt, CHAR_DATA * ch)
{
  if (txt == NULL || ch->desc == NULL)
    if (ch->lines == 0)
      {
	send_to_char (txt, ch);
	return;
      }
  if (ch->desc != NULL)
    {
      ch->desc->showstr_head = alloc_mem (strlen (txt) + 1);
//      ch->desc->showstr_head = reinterpret_cast<char *>(alloc_mem (strlen (txt) + 1));
      strcpy (ch->desc->showstr_head, txt);
      ch->desc->showstr_point = ch->desc->showstr_head;
      show_string (ch->desc, "");
    }
}


/* string pager */
void show_string (struct descriptor_data *d, char *input)
{
  char buffer[4 * MAX_STRING_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  register char *scan, *chk;
  int lines = 0, toggle = 1;
  int show_lines;
  one_argument (input, buf);

//  if (buf[0] != '\0')
	if (strlen(buf) > 1 || (strlen(buf) == 1 && LOWER(buf[0]) != 'c')) {
		if (d->character) {
			if (strlen(buf) == 1 && LOWER(buf[0]) != 'q') {
				/* switch of the letters */
				switch(LOWER(buf[0])) {
					case 'h':
						send_to_char("C, or Return = continue, H = this help\n\r", d->character);
						send_to_char("T = Top of document, Q or other keys = exit.\n\r", d->character);
						return;
						break;
					case 't':
						d->showstr_point = d->showstr_head;
						break;
					default:
						if (d->showstr_head) {
							free_mem(d->showstr_head, strlen(d->showstr_head));
							d->showstr_head = 0;
						}
						d->showstr_point = 0;
						interpret(d->character, input);
						return;
						break;
				}
			} else {
				if (d->showstr_head) {
					free_mem(d->showstr_head, strlen(d->showstr_head));
					d->showstr_head = 0;
				}
				d->showstr_point = 0;
				if (strlen(buf) > 1)
					interpret(d->character, input);
				return;
			}
		}
	}

	if (d->character)
		show_lines = d->character->lines;
	else
		show_lines = 0;

	for (scan = buffer;; scan++, d->showstr_point++) {
		if (((*scan = *d->showstr_point) == '\n' || *scan == '\r') && (toggle = -toggle) < 0) {
			lines++;
		} else if (!*scan || (show_lines > 0 && lines >= show_lines)) {
			*scan = '\0';

			/*  write_to_buffer(d,buffer,strlen(buffer)); */
			if (d->character != NULL)
				send_to_char (buffer, d->character);
			for (chk = d->showstr_point; isspace (*chk); chk++);
			{
				if (!*chk) {
					if (d->showstr_head) {
						free_mem (d->showstr_head, strlen (d->showstr_head));
						d->showstr_head = 0;
					}
					d->showstr_point = 0;
				}
			}
			return;
		}
	}
	return;
}


/* quick sex fixer */
void fix_sex (CHAR_DATA * ch)
{
  if (ch->sex < 0 || ch->sex > 2)
    ch->sex = IS_NPC (ch) ? 0 : ch->pcdata->true_sex;
}

void
act (const char *format, CHAR_DATA * ch, const void *arg1, const void *arg2,
     int type)
{

  /* to be compatible with older code */
  act_new (format, ch, arg1, arg2, type, POS_RESTING);
}

void
act_new (const char *format, CHAR_DATA * ch, const void *arg1,
	 const void *arg2, int type, int min_pos)
{
  ROOM_INDEX_DATA *lrid;
  static char *const he_she[] = { "it", "he", "she" };
  static char *const him_her[] = { "it", "him", "her" };
  static char *const his_her[] = { "its", "his", "her" };
  char buf[MAX_STRING_LENGTH];
  char fname[MAX_INPUT_LENGTH];
  CHAR_DATA *to;
  CHAR_DATA *vch = (CHAR_DATA *) arg2;
  OBJ_DATA *obj1 = (OBJ_DATA *) arg1;
  OBJ_DATA *obj2 = (OBJ_DATA *) arg2;
  const char *str;
  const char *i = "\0";
  char *point;
  memset (buf, 0, MAX_STRING_LENGTH);
  if (format == NULL || format[0] == '\0')
    return;
  if (ch == NULL || ch->in_room == NULL)
    return;
  to = ch->in_room->people;
  if (type == TO_VICT)
    {
      if (vch == NULL)
	{
	  bug ("Act: null vch with TO_VICT.", 0);
	  return;
	}
      if (vch->in_room == NULL)
	return;
      to = vch->in_room->people;
    }
  for (; to != NULL; to = to->next_in_room)
    {
      if (to->desc == NULL || get_position(to) < min_pos)
	continue;
      if ((type == TO_CHAR) && to != ch)
	continue;
      if (type == TO_VICT && (to != vch || to == ch))
	continue;
      if (type == TO_ROOM && to == ch)
	continue;
      if (type == TO_NOTVICT && (to == ch || to == vch))
	continue;
      if (type == TO_IIR && (IS_NPC(to) || to == ch || !IS_IMMORTAL(to) || !IS_SET (to->act, PLR_HOLYLIGHT)))
        continue;
      if (type == TO_CJIR && (IS_NPC(to) || (!IS_IMMORTAL(to) && ch->Class != PC_CLASS_CHAOS_JESTER)))
        continue;
      point = buf;
      str = format;
      while (*str != '\0')
	{
	  if (*str != '$')
	    {
	      *point++ = *str++;
	      continue;
	    }
	  ++str;
	  if (arg2 == NULL && *str >= 'A' && *str <= 'Z')
	    {
	      char mybuf[MAX_STRING_LENGTH];
	      bug ("Act: missing arg2 for code %d.", *str);
	      sprintf (mybuf, "FORMAT: %s", format);
	      log_string (mybuf);
	      i = " <@@@> ";
	    }

	  else
	    {
	      switch (*str)
		{
		default:
		  i = " <@@@> ";
		  break;

		  /* Thx alex for 't' idea */
		case 't':
		  if (arg1)
		    i = (char *) arg1;

		  else
		    bug ("Act: bad code $t for 'arg1'", 0);
		  break;
		case 'T':
		  if (arg2)
		    i = (char *) arg2;

		  else
		    bug ("Act: bad code $T for 'arg2'", 0);
		  break;
		case 'n':
		  if (ch && to)
		    {
		      if (is_affected (ch, gsn_mask) && ch->mask != NULL
			  && can_see (to, ch))
			i = (char *) ch->mask;

		      else
			i = PERS (ch, to);
		    }

		  else
		    bug ("Act: bad code $n for 'ch' or 'to'", 0);
		  break;
		case 'N':
		  if (vch && to)
		    {
		      if (is_affected (vch, gsn_mask)
			  && vch->mask != NULL && can_see (to, vch))
			i = (char *) vch->mask;

		      else
			i = PERS (vch, to);
		    }

		  else
		    bug ("Act: bad code $N for 'vch' or 'to'", 0);
		  break;
		case 'e':
		  if (ch)
		    i = he_she[URANGE (0, ch->sex, 2)];

		  else
		    bug ("Act: bad code $e for 'ch'", 0);
		  break;
		case 'E':
		  if (vch)
		    i = he_she[URANGE (0, vch->sex, 2)];

		  else
		    bug ("Act: bad code $E for 'vch'", 0);
		  break;
		case 'm':
		  if (ch)
		    i = him_her[URANGE (0, ch->sex, 2)];

		  else
		    bug ("Act: bad code $m for 'ch'", 0);
		  break;
		case 'M':
		  if (vch)
		    i = him_her[URANGE (0, vch->sex, 2)];

		  else
		    bug ("Act: bad code $M for 'vch'", 0);
		  break;
		case 's':
		  if (ch)
		    i = his_her[URANGE (0, ch->sex, 2)];

		  else
		    bug ("Act: bad code $s for 'ch'", 0);
		  break;
		case 'S':
		  if (vch)
		    i = his_her[URANGE (0, vch->sex, 2)];

		  else
		    bug ("Act: bad code $S for 'vch'", 0);
		  break;
		case 'p':
		  if (to && obj1)
		    {

		      //Iblis 7/7/03 - Quick char leak fix
		      if (!can_see_obj (to, obj1))
			i = "something\0";

		      else
			i = obj1->short_descr;
		      if (!to || !obj1 || !i)
			{
			  bug ("Act: bad code $p for 'to' or 'obj1'", 0);
			  return;
			}
		    }

		  else
		    bug ("Act: bad code $p for 'to' or 'obj1'", 0);
		  break;
		case 'P':
		  if (to && obj2)
		    {

		      //Iblis 7/7/03 - Quick char leak fix
		      if (!can_see_obj (to, obj2))
			i = "something\0";

		      else
			i = obj2->short_descr;
		      if (!to || !obj2 || !i)
			{
			  bug ("Act: bad code $p for 'to' or 'obj2'", 0);
			  return;
			}
		    }

		  else
		    bug ("Act: bad code $P for 'to' or 'obj2'", 0);
		  break;
		case 'd':
		  if (arg2)
		    {
		      if (arg2 == NULL || ((char *) arg2)[0] == '\0')
			{
			  i = "door";
			}

		      else
			{
			  one_argument ((char *) arg2, fname);
			  i = fname;
			}
		    }

		  else
		    bug ("Act: bad code $P for 'arg2'", 0);
		  break;
		}
	    }
	  ++str;
	  while ((*point = *i) != '\0')
	    ++point, ++i;
	}
      *point++ = '\n';
      *point++ = '\r';
      buf[0] = UPPER (buf[0]);
      send_to_char (buf, to);
    }
  memset (buf, 0, MAX_STRING_LENGTH);
  if (!is_dueling (ch))
    return;
  if (strstr (format, "has arrived.") != NULL)
    return;
  if (ch->fighting == NULL)
    return;
  if (vch != NULL)
    {
      if (!is_dueling (vch))
	return;
      if (vch->fighting == NULL)
	return;
    }
  lrid = get_room_index (ROOM_VNUM_SPECTATOR);
  for (to = lrid->people; to != NULL; to = to->next_in_room)
    {
      if (to->desc == NULL || get_position(to) < min_pos)
	continue;
      if ((type == TO_CHAR) && to != ch)
	continue;
      if (type == TO_VICT && (to != vch || to == ch))
	continue;
      if (type == TO_ROOM && to == ch)
	continue;
      if (type == TO_NOTVICT && (to == ch || to == vch))
	continue;
      point = buf;
      str = format;
      while (*str != '\0')
	{
	  if (*str != '$')
	    {
	      *point++ = *str++;
	      continue;
	    }
	  ++str;
	  if (arg2 == NULL && *str >= 'A' && *str <= 'Z')
	    {
	      char mybuf[MAX_STRING_LENGTH];
	      bug ("Act: missing arg2 for code %d.", *str);
	      sprintf (mybuf, "FORMAT: %s", format);
	      log_string (mybuf);
	      i = " <@@@> ";
	    }

	  else
	    {
	      switch (*str)
		{
		default:
		  bug ("Act: bad code %d.", *str);
		  i = " <@@@> ";
		  break;
		case 't':
		  if (arg1)
		    i = (char *) arg1;

		  else
		    bug ("Act: bad code $t for 'arg1'", 0);
		  break;
		case 'T':
		  if (arg2)
		    i = (char *) arg2;

		  else
		    bug ("Act: bad code $T for 'arg2'", 0);
		  break;
		case 'n':
		  if (ch && to)
		    {
		      if (is_affected (ch, gsn_mask) && ch->mask != NULL
			  && can_see (to, ch))
			i = ch->mask;

		      else
			i = PERS (ch, to);
		    }

		  else
		    bug ("Act: bad code $n for 'ch' or 'to'", 0);
		  break;
		case 'N':
		  if (vch && to)
		    {
		      if (is_affected (vch, gsn_mask)
			  && vch->mask != NULL && can_see (to, vch))
			i = vch->mask;

		      else
			i = PERS (vch, to);
		    }

		  else
		    bug ("Act: bad code $N for 'vch' or 'to'", 0);
		  break;
		case 'e':
		  if (ch)
		    i = he_she[URANGE (0, ch->sex, 2)];

		  else
		    bug ("Act: bad code $e for 'ch'", 0);
		  break;
		case 'E':
		  if (vch)
		    i = he_she[URANGE (0, vch->sex, 2)];

		  else
		    bug ("Act: bad code $E for 'vch'", 0);
		  break;
		case 'm':
		  if (ch)
		    i = him_her[URANGE (0, ch->sex, 2)];

		  else
		    bug ("Act: bad code $m for 'ch'", 0);
		  break;
		case 'M':
		  if (vch)
		    i = him_her[URANGE (0, vch->sex, 2)];

		  else
		    bug ("Act: bad code $M for 'vch'", 0);
		  break;
		case 's':
		  if (ch)
		    i = his_her[URANGE (0, ch->sex, 2)];

		  else
		    bug ("Act: bad code $s for 'ch'", 0);
		  break;
		case 'S':
		  if (vch)
		    i = his_her[URANGE (0, vch->sex, 2)];

		  else
		    bug ("Act: bad code $S for 'vch'", 0);
		  break;
		case 'p':
		  if (to && obj1)
		    {

		      //Iblis 7/7/03 - Quick char leak fix
		      if (!can_see_obj (to, obj1))
			i = "something\0";

		      else
			i = obj1->short_descr;
		    }

		  else
		    bug ("Act: bad code $p for 'obj1'", 0);
		  break;
		case 'P':
		  if (to && obj2)
		    {

		      //Iblis 7/7/03 - Quick char leak fix
		      if (!can_see_obj (to, obj2))
			i = "something\0";

		      else
			i = obj2->short_descr;
		    }

		  else
		    bug ("Act: bad code $P for 'obj2'", 0);
		  break;
		case 'd':
		  if (arg2)
		    {
		      if (arg2 == NULL || ((char *) arg2)[0] == '\0')
			{
			  i = "door";
			}

		      else
			{
			  one_argument ((char *) arg2, fname);
			  i = fname;
			}
		    }

		  else
		    bug ("Act: bad code $d for 'arg2'", 0);
		  break;
		}
	    }
	  ++str;
	  while ((*point = *i) != '\0')
	    ++point, ++i;
	}
      *point++ = '\n';
      *point++ = '\r';
      buf[0] = UPPER (buf[0]);
      send_to_char (buf, to);
    }
  return;
}
