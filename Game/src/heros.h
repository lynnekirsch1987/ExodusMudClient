//
//  heros.h
//  Functions and routines specifically in support of new style hero
//  characters.
//
//  Exodus Consortium - Copyright 1999 all rights reserved.
//
//

#ifndef __HEROS_H
#define __HEROS_H

// Akamai 4/26/99 - HERO changes - basically this is the set of routines
// that allow the manipulation of hero'd characters. There are special
// case functions for promoting, demoting and 'training' heros.

//
// Make this caracter a hero in exodus.
//
void do_make_hero (CHAR_DATA * imm, CHAR_DATA * ch);

//
// Bad caracter is losing his hero status
//
void do_unmake_hero (CHAR_DATA * imm, CHAR_DATA * ch);

//
// This is a special version of the gain function and should be called
// from do_gain when gain finds that the character is a hero. This
// special version allows/enforces the ability for hero characters to
// get skills that were not originally part of their player Class
//
void do_hero_gain (CHAR_DATA * ch, CHAR_DATA * trainer, char *argument);
void gain_hero_skills (CHAR_DATA * imm, CHAR_DATA * ch);
//
// This is a special version of the practice function - performs some
// simple fix ups for hero characters.
//
void do_hero_practice (CHAR_DATA * ch, char *argument);

#endif

