namespace ExodusMudClient.Data.Game {
    public class GlobalConstants {
        [Flags]
        public enum CharacterClass {
            None = 0,
            Mage = 1 << 0, // Equivalent to (A)
            Cleric = 1 << 1, // (B)
            Thief = 1 << 2, // (C)
            Warrior = 1 << 4, // (E), skipped D for consistency with source
            Ranger = 1 << 5, // (F)
            Bard = 1 << 6, // (G)
            Paladin = 1 << 7, // (H)
            Assassin = 1 << 8, // (I)
            Reaver = 1 << 9, // (J)
            Monk = 1 << 10, // (K)
            Necromancer = 1 << 11, // (L)
            Swarm = 1 << 12, // (M)
            Golem = 1 << 13, // (N)
            ChaosJester = 1 << 14, // (O)
            Adventurer = 1 << 15, // (P)
            Druid = 1 << 16, // (Q)
            Empath = 1 << 17, // (R)
            Hero = 1 << 30 // (ee), assigned a high value to denote its unique status
        }

        public class ElevatorDestList {
            public ElevatorDestList Next { get; set; }
            public int Vnum { get; set; }

            public string ArrivalMsgI { get; set; }
            public string ArrivalMsgO { get; set; }
            public string DepartureMsgI { get; set; }
            public string DepartureMsgO { get; set; }

            // Constructor
            public ElevatorDestList(int vnum,string arrivalMsgI,string arrivalMsgO,string departureMsgI,string departureMsgO) {
                Vnum = vnum;
                ArrivalMsgI = arrivalMsgI;
                ArrivalMsgO = arrivalMsgO;
                DepartureMsgI = departureMsgI;
                DepartureMsgO = departureMsgO;
            }
        }

        public const int MAX_EXTRA_FLAGS = 2;
        public const int MAX_OBJ_TRIGS = 16;
        public const int MAX_STATS = 16;
        public const int MAX_KEY_HASH = 2048;
        public const int MAX_STRING_LENGTH = 4608;
        public const int MAX_INPUT_LENGTH = 256;
        public const int PAGELEN = 22;
        public const int MAX_CHUNKS = 80;

        public const int NO_FLAG = -99;

        public const int AREA_VER_DEFAULT = 0;
        public const int AREA_VER_CLASSRACE = 1;
        public const int AREA_VER_OBJTRIG = 2;
        public const int AREA_VER_OBJTRIG2 = 3;
        public const int AREA_VER_EXTRA_FLAGS = 4;
        public const int AREA_VER_MD_AF = 5;
        public const int AREA_VER_CARDS = 6;
        public const int AREA_VER_CURRENT = AREA_VER_CARDS;

        public const int PFILE_VER_DEFAULT = 5;
        public const int PFILE_VER_NOTES = 6;
        public const int PFILE_VER_IMMCOLORS = 7;
        public const int PFILE_VER_PROJECTS = 8;
        public const int PFILE_VER_CURRENT = PFILE_VER_PROJECTS;

        public const int MAX_DIR = 6;
        public const int MAX_SOCIALS = 512;
        public const int MAX_SKILL = 512;
        public const int MAX_GROUP = 64;
        public const int MAX_IN_GROUP = 16;
        public const int MAX_ALIAS = 48;
        public const int MAX_CLASS = 17;
        public const int MAX_PC_RACE = 15;
        public const int MAX_HISTORY = 20;
        public const int MAX_IGNORE = 10;
        public const int MAX_LEVEL = 100;
        public const int MAX_EXPLORE = 8192;
        public const int REAVER_WEAPON_MAX = 8;
        public const int MAX_SOUL_GAIN = 100;

        public const int OVERLORD = MAX_LEVEL;
        public const int WIZARD = MAX_LEVEL - 2;
        public const int GOD = MAX_LEVEL - 5;
        public const int DEMIGOD = MAX_LEVEL - 8;
        public const int LEVEL_HERO = MAX_LEVEL - 9;
        public const int HERO = LEVEL_HERO;
        public const int LEVEL_EXPLORER = MAX_LEVEL - 9;
        public const int LEVEL_IMMORTAL = MAX_LEVEL - 8;

        public const int PULSE_PER_SECOND = 4;
        public const int PULSE_SPEED = 1 * PULSE_PER_SECOND;
        public const int PULSE_VIOLENCE = 3 * PULSE_PER_SECOND;
        public const int PULSE_MOBILE = 4 * PULSE_PER_SECOND;
        public const int PULSE_MUSIC = 6 * PULSE_PER_SECOND;
        public const int PULSE_TICK = 40 * PULSE_PER_SECOND;
        public const int PULSE_REBOOT = 60 * PULSE_PER_SECOND;
        public const int PULSE_SHUTDOWN = 60 * PULSE_PER_SECOND;
        public const int PULSE_AREA = 120 * PULSE_PER_SECOND;
        public const int PULSE_CLANWAR = 180 * PULSE_PER_SECOND; // Original
        public const int PULSE_AUCTION = 18 * PULSE_PER_SECOND;
        public const int PULSE_SCENT = 10 * PULSE_PER_SECOND;
        public const int PULSE_BET = 18 * PULSE_PER_SECOND;
        public const int MULTIKILL_WAIT_TIME = 8; // In terms of minutes

        public const int STOCK_PERM = 0;
        public const int STOCK_TEMP = 1;


        [Flags]
        public enum AreaFlags {
            None = 0,
            Changed = 1 << 0,
            Added = 1 << 1,
            Loading = 1 << 2,
            ImpOnly = 1 << 3,
            NoQuit = 1 << 4,
            NoRepopWia = 1 << 5,
            NoTree = 1 << 6,
            Solo = 1 << 7,
            NoRepop = 1 << 8
        }

        public enum PlayerFileVersions {
            Default = 5,
            Notes = 6,
            ImmColors = 7,
            Projects = 8,
            Current = Projects // Assuming Projects is the latest version
        }

        // Reaver upgrade constants
        public const int BLADE_POWER = 1;
        public const int BLADE_HATRED = 2;
        public const int BLADE_STRENGTH = 3;
        public const int BLADE_FURY = 4;

        // Blade Spells Enum
        public enum BladeSpell {
            Shocking = 0,
            Frost = 1,
            Flaming = 2,
            Venemous = 3,
            Vampiric = 4,
            Chaotic = 5,
            Apathy = 6
        }
        public const int MAX_BLADE_SPELLS = 7; // Alternatively, you could use Enum.GetNames(typeof(BladeSpell)).Length;

        // Reaver Burst Spells Enum
        public enum BurstSpell {
            Lightning = 0,
            Ice = 1,
            Fire = 2,
            Unholy = 3,
            Hellscape = 4
        }
        public const int MAX_BURST = 5; // Similarly, Enum.GetNames(typeof(BurstSpell)).Length;
        public enum Race {
            Human = 1,
            Dwarf = 2,
            Vroath = 3,
            Elf = 4,
            Canthi = 5,
            Syvin = 6,
            Sidhe = 7,
            Avatar = 8,
            Thyrent = 9, // Note: 'L' used for both Thyrent and Litan in the original. Assuming a typo or unique values required.
            Litan = 10, // Assigning a unique value considering the note above.
            Kalian = 11,
            Lich = 12,
            Nerix = 13,
            Swarm = 14
        }
        public const int ROOM_NOTRANSPORT = 15;
        public const int ROOM_PLAYERSTORE = 16;
        public const int ROOM_NOTELEPORT = 17;
        [Flags]
        public enum RoomFlags {
            Notransport = 1 << 0, // Assuming starting from 1 for demonstration
            PlayerStore = 1 << 1,
            Noteleport = 1 << 2
        }

    }
}
