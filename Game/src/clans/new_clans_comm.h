//
// new_clans_comm.h
//
// Programmer: Pulewai
// Date: 7/10/98
// Version: 1.0
//
// These routines provide the main command hooks for the command
// interpreter. There is one

#ifndef __new_clans_comm_h__
#define __new_clans_comm_h__


//
// Public entries into the clan code
//
void do_clan (CHAR_DATA * ch, char *argument);
void do_desecrate (CHAR_DATA * ch, char *argument);
void do_sanctify (CHAR_DATA * ch, char *argument);
void do_clanwar (CHAR_DATA * ch, char *argument);
void update_wars ();
void do_disperse (CHAR_DATA * ch, char *argument);
void do_reclan (CHAR_DATA * ch, char *argument);



#endif // __new_clans_comm_h__
