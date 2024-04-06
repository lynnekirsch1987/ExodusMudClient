using static ExodusMudClient.Data.Game.Enums;

namespace ExodusMudClient.Data.Game.Models
{
    public class AreaFile
    {
        public int Version { get; set; }
        public string FileName { get; set; }
        public string AreaName { get; set; }
        public string LevelRangeCredits { get; set; }
        public string LowLevel { get; set; }
        public string HighLevel { get; set; }
        public string Creator { get; set; }

        public string Uknown1 { get; set; }
        public string VnumsHighLow { get; set; }
        public string Unkown2 { get; set; }
        public string Unkown3 { get; set; }
        public string Unkown4 { get; set; }

        public List<AreaFileMobile> Mobiles { get; set; }
        public List<AreaFileRoom> Rooms { get; set; }
        public List<AreaFileObject> Objects { get; set; }
    }

    /**
    0 // the mobile vnum
    #4090 

    1 // mobile name for targeting
    thesden guard~

    2 // short description
    a guard~

    3 // long description
    A Thesden guard patrols the streets here.

    4 // description
    ~
    Eagerness shines on th
    ~

    5 // race
    human~

    6 // recruit flags having to do with player shops
    0 0 0 0

    7 // act flags, affect flags, alignment, group
    AGTX DNV 0 406

    8
    // 35: level
    // 35: hitroll
    // 10d90+535: hit dice #, hit dice type, hit dice bonus
    // 10d10+50: mana dice #, mana dice type, mana dice bonus
    // 5d7+20: dam dice #, dam dice type, dam dice bonus
    // pound: dam type
    35 35 10d90+535 10d10+50 5d7+20 pound

    9 // ac pierce, bash, slash, exotic (if exotic is 999, then exotic is -1, otherwise line ends at first exotic)
    -12 -12 -12 999 -1

    10
    // act 2 flags, 
    // blocks exit, 
    // (if blocks exit > 50, num_attacks is the next number, otherwise empty), 
    // off flags, 
    // imm flags,
    // res flags,
    // vuln flags
    0 -1 CFHKT 0 BCP OS

    11
    // start position, default position, sex, wealth
    stand stand either 150

    12
    // form, parts, 6x stats, card vnum
    AHMV ABCDEFGHIJKa 13 13 13 13 13 13 0

    13
    // size, material
    medium flesh~

    14
    // defense bonus, attach bonus, max weight, move
    0 0 0 300

    15
    // default mood
    0

    16
    // vocab file
    ~

    17
    // script file
    ~
     */
    public class AreaFileMobile
    {
        public int Vnum { get; set; }
        public string NameForTargeting { get; set; }
        public string ShortDescription { get; set; }
        public string LongDescription { get; set; }
        public string Description { get; set; }
        public string Race { get; set; }
        public string RecruitFlags { get; set; }
        public ActFlags ActFlags { get; set; }
        public AffectedByFlags AffectFlags { get; set; }
        public int Alignment { get; set; }
        public int Group { get; set; }
        public int Level { get; set; }
        public int HitRoll { get; set; }
        public Dice HitDice { get; set; }
        public Dice ManaDice { get; set; }
        public Dice DamageDice { get; set; }
        public string DamageType { get; set; }
        public int[] ArmorClass { get; set; } = new int[4];
        public int BlocksExit { get; set; }
        public int NumberOfAttacks { get; set; } = 0; // Default to 0, assuming it might be optional
        public string OffFlags { get; set; }
        public string ImmFlags { get; set; }
        public string ResFlags { get; set; }
        public string VulnFlags { get; set; }
        public PositionType StartPosition { get; set; }
        public PositionType DefaultPosition { get; set; }
        public SexType Sex { get; set; }
        public int Wealth { get; set; }
        public string Form { get; set; }
        public string Parts { get; set; }
        public int[] Stats { get; set; } = new int[6];
        public int CardVnum { get; set; }
        public string Size { get; set; }
        public string Material { get; set; }
        public int DefenseBonus { get; set; }
        public int AttackBonus { get; set; }
        public int MaxWeight { get; set; }
        public int Move { get; set; }
        public int DefaultMood { get; set; }
        public string VocabFile { get; set; }
        public string ScriptFile { get; set; }
    }

    /**
     * 
     *  
     *  #7901 // vnum (0)
     *  
        white robe erion~ //name (1)

        `oa white robe of the Order Erion``~ //short (2)

        The white robe worn by the Brothers of Erion lies here in a heap.~ // long (3)
        hemp~ //material (4)
        0 // timer (5)
        armor 2 // riname (item lookup assigned to item type), number of extra flags (6)
        ST // extra flags 1/2 (7) 
        0 // extra flags 2/2 (7)
        AK // wear flags (8)
        K // class wear flags (9)
        0 // race wear flags (10)
        0 // clan wear flags (11)
        0 // objtrig vnum (12)
        0 // objtrig vnum 
        0 // objtrig vnum   
        0 // objtrig vnum  
        0 // objtrig vnum
        0 // objtrig vnum
        0 // objtrig vnum
        0 // objtrig vnum
        0 // objtrig vnum
        0 // objtrig vnum
        0 // objtrig vnum
        0 // objtrig vnum
        0 // objtrig vnum
        0 // objtrig vnum
        0 // objtrig vnum
        0 // objtrig vnum 
        5 // rarity (28)
        ~ // string 1 (29)
        ~ // string 2 (30)
        ~ // string 3 (31)  
        ~ // string 4 (32)
        AD ABCD AD 0 0 // value[0] - value[4] flags (33)
        0 v5 (34)
        0 v6 (35) 
        30 50 5000 P (36) // level weight cost condition
        A 
        19 1 
        A
        18 1
        A
        13 9
        A
        12 9
     */
    public class AreaFileObject
    {
        public string Vnum { get; set; } // Virtual number (unique identifier)
        public string Name { get; set; } // Name of the object
        public string ShortDescription { get; set; } // Short description
        public string LongDescription { get; set; } // Long description when it's lying in a room
        public string Material { get; set; } // Material made from
        public int Timer { get; set; } // Timer for object's existence
        public ItemType ItemType { get; set; } // Type of item (armor, weapon, etc.)
        public int ExtraFlagsNumber { get; set; } // Number of extra flags
        public List<string> ExtraFlags { get; set; } // Extra flags
        public string WearFlags { get; set; } // Where the item can be worn
        public string ClassWearFlags { get; set; } // Class restrictions for wearing
        public int RaceWearFlags { get; set; } // Race restrictions for wearing
        public int ClanWearFlags { get; set; } // Clan restrictions for wearing
        public List<int> ObjTrigVnums { get; set; } = new List<int>(); // Object trigger VNUMs (up to 20)
        public int Rarity { get; set; } // Rarity level
        public List<string> Strings { get; set; } = new List<string>(); // Additional strings (up to 4)
        public string Value0Flags { get; set; } // Value flags for the object
        public List<string> Values { get; set; } = new List<string>(); // Values (stat bonuses, etc.)
        public int Level { get; set; }
        public int Cost { get; set; }
        public string Condition { get; set; }
        public int Weight { get; set; }

        public AreaFileObject()
        {
            ExtraFlags = new List<string>();
            Values = Enumerable.Repeat("", 16).ToList();
        }
    }
    /**
#7904 // vnum (0)
0 0 // level, xp  (1)
~ //tp_msg (2)
~ //enter msg (3)
~ //exit msg (4) 
0 0 0 // race flags, class flags, max level (5)
Watchtower above the Main Gate~ // room name (6)
You are standing on the walkway above the main gate.  It extends over the
entrance and a short way onto each of the buildings striding the gate.  You
step carefully, watchfull not to slip on the wet and moss-ridden stones,
worn smooth by years of pacing by the brothers of the order.  Leaning
against the sturdy wooden railing you look to the north, still amazed at the
gothic beauty of the Abbey Church, despite having seen it before, it's many
spires rise to the sky, and you feel you can understand a little of why the
monks have chosen to devote their life to their religion.
~ // room description (7)
0 D 0  // area number, room flags, sector type (8)
0 // max_in_room (9)
~  // max msg (10)
I 0 D0 (11) Line11
~ 12
butthole~ 13
97 25698 26822 14
D5 15
~ 16
~ 17
0 0 7901 18
S END (19)
*/
    public class AreaFileRoom
    {
        public int Vnum { get; set; }
        public int Level { get; set; }
        public int Xp { get; set; }
        public string TpMsg { get; set; }
        public string EnterMsg { get; set; }
        public string ExitMsg { get; set; }
        public int RaceFlags { get; set; }
        public int ClassFlags { get; set; }
        public int MaxLevel { get; set; }
        public string RoomName { get; set; }
        public string RoomDescription { get; set; }
        public int AreaNumber { get; set; }
        public string RoomFlags { get; set; }
        public string SectorType { get; set; }
        public int MaxInRoom { get; set; }
        public string MaxMsg { get; set; }
        public string Line11 { get; set; }
        public string Line12 { get; set; }
        public string Line13 { get; set; }
        public int[] Line14 { get; set; } = new int[3];
        public string Line15 { get; set; }
        public string Line16 { get; set; }
        public string Line17 { get; set; }
        public int[] Line18 { get; set; } = new int[3];
        public string Line19 { get; set; }
    }
}
