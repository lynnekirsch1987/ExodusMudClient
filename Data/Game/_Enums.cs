namespace ExodusMudClient.Data.Game {
    public class Enums {
        public enum PositionType {
            Dead = 0,
            Mortal = 1,
            Incap = 2,
            Stunned = 3,
            Sleeping = 4,
            Resting = 5,
            Sitting = 6,
            Fighting = 7,
            Standing = 8,
            Tethered = 9,
            Mounted = 10,
            FeigningDeath = 11,
            Coma = 12,
            Chopping = 13,
            Milling = 14,
            Carving = 15
        }
        public enum SexType {
            None,
            Male,
            Female,
            Either,
            Neutral
        }

        public enum DamageType {
            None = 0,
            Bash = 1,
            Pierce = 2,
            Slash = 3,
            Fire = 4,
            Cold = 5,
            Lightning = 6,
            Acid = 7,
            Poison = 8,
            Negative = 9,
            Holy = 10,
            Energy = 11,
            Mental = 12,
            Disease = 13,
            Drowning = 14,
            Light = 15,
            Other = 16,
            Harm = 17,
            Charm = 18,
            Sound = 19,
            // Added types
            Clawing = 20,
            Wrath = 21,
            Vampiric = 22,
            Unique = 23,
            Wind = 24
        }


        public enum ItemType {
            ITEM_LIGHT,
            ITEM_SCROLL,
            ITEM_WAND,
            ITEM_STAFF,
            ITEM_WEAPON,
            ITEM_TREASURE,
            ITEM_ARMOR,
            ITEM_POTION,
            ITEM_CLOTHING,
            ITEM_FURNITURE,
            ITEM_TRASH,
            ITEM_CONTAINER,
            ITEM_CLAN_DONATION,
            ITEM_NEWCLANS_DBOX, // new clans
            ITEM_PLAYER_DONATION,
            ITEM_SADDLE,
            ITEM_PACK,
            ITEM_ELEVATOR,
            ITEM_RAFT,
            ITEM_STRANSPORT,
            ITEM_CTRANSPORT,
            ITEM_EBUTTON,
            ITEM_VIAL,
            ITEM_PARCHMENT,
            ITEM_DRINK_CON,
            ITEM_KEY,
            ITEM_FOOD,
            ITEM_MONEY,
            ITEM_GILLS,
            ITEM_CORPSE_NPC,
            ITEM_CORPSE_PC,
            ITEM_FOUNTAIN,
            ITEM_PILL,
            ITEM_PROTECT,
            ITEM_MAP,
            ITEM_PORTAL,
            ITEM_WARP_STONE,
            ITEM_ROOM_KEY,
            ITEM_GEM,
            ITEM_JEWELRY,
            ITEM_JUKEBOX,
            ITEM_INSTRUMENT,
            ITEM_WRITING_INSTRUMENT,
            ITEM_WRITING_PAPER,
            ITEM_CTRANSPORT_KEY,
            ITEM_PORTAL_BOOK,
            ITEM_QUIVER,
            ITEM_FEATHER,
            ITEM_POLE,
            ITEM_BAIT,
            ITEM_TREE,
            ITEM_WOOD,
            ITEM_SEED,
            ITEM_WOODEN_INCONSTRUCTION,
            ITEM_CARD,
            ITEM_BINDER,
            ITEM_OBJ_TRAP,
            ITEM_PORTAL_TRAP,
            ITEM_ROOM_TRAP,
            ITEM_RANDOM
        }

        [Flags]
        public enum AffectedByFlags {
            AFF_BLIND = 1 << 0,          // 1
            AFF_INVISIBLE = 1 << 1,      // 2
            AFF_DETECT_EVIL = 1 << 2,    // 4
            AFF_DETECT_INVIS = 1 << 3,   // 8
            AFF_DETECT_MAGIC = 1 << 4,   // 16
            AFF_DETECT_HIDDEN = 1 << 5,  // 32
            AFF_DETECT_GOOD = 1 << 6,    // 64
            AFF_SANCTUARY = 1 << 7,      // 128
            AFF_FAERIE_FIRE = 1 << 8,    // 256
            AFF_INFRARED = 1 << 9,       // 512
            AFF_CURSE = 1 << 10,         // 1024
            AFF_CAMOUFLAGE = 1 << 11,    // 2048
            AFF_POISON = 1 << 12,        // 4096
            AFF_PROTECT_EVIL = 1 << 13,  // 8192
            AFF_PROTECT_GOOD = 1 << 14,  // 16384
            AFF_SNEAK = 1 << 15,         // 32768
            AFF_HIDE = 1 << 16,          // 65536
            AFF_SLEEP = 1 << 17,         // 131072
            AFF_CHARM = 1 << 18,         // 262144
            AFF_FLYING = 1 << 19,        // 524288
            AFF_PASS_DOOR = 1 << 20,     // 1048576
            AFF_HASTE = 1 << 21,         // 2097152
            AFF_CALM = 1 << 22,          // 4194304
            AFF_PLAGUE = 1 << 23,        // 8388608
            AFF_WEAKEN = 1 << 24,        // 16777216
            AFF_DARK_VISION = 1 << 25,   // 33554432
            AFF_BERSERK = 1 << 26,       // 67108864
            AFF_SWIM = 1 << 27,          // 134217728
            AFF_REGENERATION = 1 << 28,  // 268435456
            AFF_SLOW = 1 << 29,          // 536870912
            AFF_AQUA_BREATHE = 1 << 30   // 1073741824
        }

        [Flags]
        public enum ActFlags : long {
            ACT_IS_NPC = 1L << 0,
            ACT_SENTINEL = 1L << 1,
            ACT_SCAVENGER = 1 << 2,      // Picks up objects
            ACT_RANGER = 1 << 3,
            ACT_NO_SCENT = 1 << 4,
            ACT_AGGRESSIVE = 1 << 5,     // Attacks PCs
            ACT_STAY_AREA = 1 << 6,      // Won't leave area
            ACT_WIMPY = 1 << 7,
            ACT_PET = 1 << 8,            // Auto set for pets
            ACT_TRAIN = 1 << 9,          // Can train PCs
            ACT_PRACTICE = 1 << 10,      // Can practice PCs
            ACT_HALT = 1 << 11,
            ACT_HUNT = 1 << 12,
            ACT_REMEMBER = 1 << 13,
            ACT_UNDEAD = 1 << 14,
            ACT_BARD = 1 << 15,
            ACT_CLERIC = 1 << 16,
            ACT_MAGE = 1 << 17,
            ACT_THIEF = 1 << 18,
            ACT_WARRIOR = 1 << 19,
            ACT_BOUNTY = 1 << 20,
            ACT_NOPURGE = 1 << 21,
            ACT_OUTDOORS = 1 << 22,
            ACT_SMART_HUNT = 1 << 23,
            ACT_INDOORS = 1 << 24,
            ACT_MOUNT = 1 << 25,
            ACT_IS_HEALER = 1 << 26,
            ACT_GAIN = 1 << 27,
            ACT_LOOTER = 1 << 28,
            ACT_IS_CHANGER = 1 << 29,
            ACT_TAME = 1L << 30,
            ACT_BLOCK_EXIT = 1L << 31,
            ACT_NOSINK = 1L << 32,
            ACT_ILLUSION = 1L << 33,
            ACT_NOMOVE = 1L << 34,
            ACT_FAMILIAR = 1L << 35,
            ACT_PUPPET = 1L << 36,
            ACT_PUPPETEER = 1L << 37,
            ACT_MONK = 1L << 38,
            ACT_NECROMANCER = 1L << 39,
            ACT_QUESTMASTER = 1L << 40,
            ACT_QUESTMOB = 1L << 41,
            ACT_AGGIE_EVIL = 1L << 42,
            ACT_AGGIE_NEUTRAL = 1L << 43,
            ACT_AGGIE_GOOD = 1L << 44,
            ACT_TRANSFORMER = 1L << 45,
            ACT_CROUPIER = 1L << 46,
            ACT_MAGE_SKILLS = 1L << 47,
            ACT_NO_KILL = 1L << 48,
            ACT_NO_ALIGN = 1L << 49,
            ACT_TRAPMOB = 1L << 50,
            ACT_NO_BLOCK = 1L << 51,
            ACT_STAY_SECTOR = 1L << 52,
            ACT_WARPED = 1L << 53,
            ACT_IMAGINARY = 1L << 54,
            ACT_SWITCHED = 1L << 55,
            ACT_ALL_SKILLS = 1L << 56,
            ACT_ALWAYS_MIRRORED = 1L << 57
        }
    }
}
