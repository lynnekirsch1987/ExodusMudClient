//Object Triggers by Adeon
// obj_trig.h

#ifndef __OBJ_TRIG_H
#define __OBJ_TRIG_H

//OBJ_TRIG_DATA *obj_trig_hash[MAX_KEY_HASH];
//OBJ_TRIG_DATA *obj_trig_list;
//OBJ_TRIG *ot_list;

//borrowing this...
#define A      1
#define B      2
#define C      4
#define D      8
#define E      16
#define F      32
#define G      64
#define H      128

#define I      256
#define J      512
#define K      1024
#define L      2048
#define M      4096
#define N      8192
#define O      16384
#define P      32768

#define Q      65536
#define R      131072
#define S      262144
#define T      524288
#define U      1048576
#define V      2097152
#define W      4194304
#define X      8388608

#define Y      16777216
#define Z      33554432
#define aa     67108864
#define bb     134217728
#define cc     268435456
#define dd     536870912
#define ee     1073741824

//Trigger flags
#define OBJ_TRIG_ENTER_ROOM			(A) // A character enters the room an obj is laying in
#define OBJ_TRIG_GET				(B) // A character comes into possession of the obj
#define OBJ_TRIG_WEAR				(C) // A character wear/wield/holds the obj
#define OBJ_TRIG_CHAR_HP_PCT		(D)	// Character using obj hp is at x%
#define OBJ_TRIG_VICT_HP_PCT		(E)	// Characters oppenent hp is at x%
#define OBJ_TRIG_MANA_PCT			(F)	// Character using obj mana is at x%
#define OBJ_TRIG_MOVE_PCT			(G)	// Characters opponent mana is at x%
#define OBJ_TRIG_CHANCE_ON_HIT		(H) // x% chance of triggering on hitting opponent
#define OBJ_TRIG_CHANCE_ON_ROUND	(I)	// x% chance of triggereing every round of combat
#define OBJ_TRIG_REMOVE				(J)	// A character removes the obj
#define OBJ_TRIG_DROP				(K)	// A character drops the obj
#define OBJ_TRIG_PUT				(L)	// A character puts the obj in something
#define OBJ_TRIG_OBJ_GIVEN_CHAR		(M)	// The obj is given to a char
#define OBJ_TRIG_OPEN				(N)	// The obj is opened (if container)
#define OBJ_TRIG_CLOSE				(O) // The obj is closed (if container)
#define OBJ_TRIG_ACTION_WORD		(P)	// The obj is pulled, tugged, smashed, climbed etc.
#define OBJ_TRIG_CHAR_TO_ROOM		(Q) // The character goes to room <vnum>
#define OBJ_TRIG_CHAR_VOCALIZE		(R) // The character says/shouts/yells x
#define OBJ_TRIG_CHAR_ATTACKS		(S) // The character attacks an opponent
#define OBJ_TRIG_CHAR_IS_ATTACKED	(T) // The character is attacked
#define OBJ_TRIG_QUAFFED			(U) // The obj is quaffed (if potion)
#define OBJ_TRIG_EATEN				(V) // The obj is eaten (if food or pill)
#define OBJ_TRIG_CHAR_FLEES			(W) // The character flees
#define OBJ_TRIG_VICT_FLEES			(X) // The characters opponent flees
#define OBJ_TRIG_CHAR_POS_CHANGE	(Y) // The character sits, rests, stands, mounts, etc.
#define OBJ_TRIG_CHAR_RECALLS		(Z) // The character recalls
#define OBJ_TRIG_CHAR_USES_SKILL	(aa)// The character uses a particular skill
#define OBJ_TRIG_CHAR_USES_SPELL	(bb)// The character uses a spell/prayer/chant/song
#define OBJ_TRIG_CHAR_INVOKES		(cc)// Character invokes (if char is reaver)
#define OBJ_TRIG_ON_TICK		(dd)// Randomly occurs (no specific trigger)
#define OBJ_TRIG_EXTRA_FLAGS	        (ee)

//These are specials for determining certain subtypes
#define OT_SPEC_NONE			0
#define OT_SPEC_ENTER			1
#define OT_SPEC_EXIT			2
#define OT_SPEC_CRAWL			3
#define OT_SPEC_CLIMB			4
#define OT_SPEC_PLAY			5
#define OT_SPEC_DESCEND			6
#define OT_SPEC_SCALE			7
#define OT_SPEC_JUMP			8
#define OT_SPEC_TUG				9
#define OT_SPEC_RING			10
#define OT_SPEC_SHOVE			11
#define OT_SPEC_SMASH			12	
#define OT_SPEC_PRESS			13
#define OT_SPEC_PULL			14
#define OT_SPEC_BOARD			15
#define OT_SPEC_DIG			16

#define OT_VOC_SAY				1
#define OT_VOC_SHOUT			2
#define OT_VOC_YELL				3

#define OT_TYPE_SPELL			1
#define OT_TYPE_PRAYER                  2
#define OT_TYPE_CHANT                   3
#define OT_TYPE_SONG                    4

#define OT_EXTRA_WHEN_WORN              	(A) 
#define OT_EXTRA_WHEN_CARRIED           	(B)
#define OT_EXTRA_WHEN_ON_GROUND			(C)
#define OT_EXTRA_WHEN_TRIGGERER_WORN	        (D)
#define OT_EXTRA_WHEN_TRIGGERER_CARRIED		(E)
#define OT_EXTRA_ON_NPC				(F)
#define OT_EXTRA_ON_PC				(G)
//#define OT_EXTRA_WHEN_ANY			(D)
//#define OT_EXTRA_WHEN_WORN_OR_CARRIED   	(E)

#endif

