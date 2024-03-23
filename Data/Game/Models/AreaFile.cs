using static ExodusMudClient.Data.Game.Enums;

namespace ExodusMudClient.Data.Game.Models {
    public class AreaFile {
        public int Version { get; set; }
        public string FileName { get; set; }
        public string AreaName { get; set; }
        public string LevelRangeCredits { get; set; }
        public string Uknown1 { get; set; }
        public string VnumsHighLow { get; set; }
        public string Unkown2 { get; set; }
        public string Unkown3 { get; set; }
        public string Unkown4 { get; set; }
        
        public List<AreaFileMobile> Mobiles { get; set; }
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

    public class AreaFileMobile {
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
     *  #7901 // vnum
     *  
        white robe erion~ //name

        `oa white robe of the Order Erion``~ //short

        The white robe worn by the Brothers of Erion lies here in a heap.~ // long
        hemp~ //material
        0
        armor 2
        ST
        0
        AK
        K
        0
        0
        0
        0
        0
        0
        0
        0
        0
        0
        0
        0
        0
        0
        0
        0
        0
        0
        5
        ~
        ~
        ~
        ~
        AD ABCD AD 0 0
        0
        0
        30 50 5000 P
        A
        19 1
        A
        18 1
        A
        13 9
        A
        12 9
     */

    public class AreaFileObject {

    }
}
