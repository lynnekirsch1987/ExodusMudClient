//
// new_clans_io.h
//
// Programmer: Pulewai
// Date: 7/10/98
// Version: 1.0
//
// The public prototypes through which the rest of the clan code
// manipulates the clan related files.

#ifndef __new_clans_io_h__
#define __new_clans_io_h__

void load_clans (void);		// loads the index and all clans into the table

void save_clans (void);		// saves the clanindex and all clans in the table

void save_clans_txt (void);	// backward compatibility

bool load_clanindex (void);	// loads the index of clans into the clan_table

void save_clanindex (void);	// saves an index of the clans in the clan_table

void save_clan_box (OBJ_DATA * pObj);	// saves the clan box

void load_clan_box (OBJ_DATA * pObj);	// load a clan box from disk

void save_clan (int clan_slot);	// saves one clan slot

bool load_clan (int clan_slot, char *name);	// loads one clan, 

#ifdef TESTING
// only export the low-level routines only if we are in a testing mode
void low_save_clanfile (FILE * cfp, int clan);
void low_save_members (FILE * cfp, MEMBER * member);
void low_save_relations (FILE * cfp, RELATIONS * relate);

void low_load_clanfile (FILE * cfp, int clan_slot);
MEMBER *low_load_members (FILE * cfp);
RELATIONS *low_load_relations (FILE * cfp);
#endif // TESTING

#endif // __new_clans_io_h__
