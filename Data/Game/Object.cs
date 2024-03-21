using static ExodusMudClient.Data.Game.Enums;
using static ExodusMudClient.Data.Game.GlobalConstants;

namespace ExodusMudClient.Data.Game {
    public class Object {
        public int Vnum { get; set; }
        public Area Area { get; set; }
        public int ResetNum { get; set; }
        public string Name { get; set; }
        public string ShortDescription { get; set; }
        public string Description { get; set; }
        public string Material { get; set; }
        public int Timer { get; set; }
        public ItemType ItemType { get; set; } // ItemType should be an enum
        public long[] ExtraFlags { get; set; } = new long[MAX_EXTRA_FLAGS]; // Define MAX_EXTRA_FLAGS
        public long WearFlags { get; set; }
        public long ClassFlags { get; set; }
        public long RaceFlags { get; set; }
        public long ClanFlags { get; set; }
        public int[] ObjTrigVnum { get; set; } = new int[MAX_OBJ_TRIGS]; // Define MAX_OBJ_TRIGS
        public int Rarity { get; set; }
        public string[] Strings { get; set; } = new string[4];
        public long[] Value { get; set; } = new long[500]; // Define MAX_OBJ_VALUES to fit your game
        public int Level { get; set; }
        public int Weight { get; set; }
        public int Cost { get; set; }
        public char Condition { get; set; }
        public List<AffectData> Affected { get; set; }
        public List<ExtraDescrData> ExtraDescriptions { get; set; }
        // Additional properties and methods for handling object behaviors, interactions, etc., would go here.
    }
    public class ObjData {
        public ObjData Next { get; set; }
        public ObjData NextContent { get; set; }
        public ObjData Contains { get; set; }
        public ObjData InObj { get; set; }
        public ObjData On { get; set; }
        public CharData CarriedBy { get; set; }
        public CharData SetBy { get; set; }
        public ExtraDescrData ExtraDescr { get; set; }
        public AffectData Affected { get; set; }
        public Object PIndexData { get; set; }
        public ObjData NextTrap { get; set; }
        public Room InRoom { get; set; }
        public bool Valid { get; set; }
        public bool Enchanted { get; set; }
        public string Owner { get; set; }
        public string Name { get; set; }
        public string ShortDescr { get; set; }
        public string Description { get; set; }
        public short ItemType { get; set; }
        public long[] ExtraFlags { get; set; } = new long[MAX_EXTRA_FLAGS];
        public long WearFlags { get; set; }
        public long RaceFlags { get; set; }
        public long ClassFlags { get; set; }
        public long ClanFlags { get; set; }
        public long WearLoc { get; set; }
        public long PrevWearLoc { get; set; }
        public short Weight { get; set; }
        public int Cost { get; set; }
        public short Level { get; set; }
        public short Condition { get; set; }
        public string Material { get; set; }
        public short Timer { get; set; }
        public int[] Value { get; set; } = new int[13];
        public int ETime { get; set; }
        public ElevatorDestList DestList { get; set; }
        // Blade spells and burst charges/capacities
        public int[] BsCapacity { get; set; } = new int[MAX_BLADE_SPELLS];
        public int[] BsCharges { get; set; } = new int[MAX_BLADE_SPELLS];
        public int[] BbCapacity { get; set; } = new int[MAX_BURST];
        public int[] BbCharges { get; set; } = new int[MAX_BURST];
        public string PlrOwner { get; set; }
        public ObjTrig[] Objtrig { get; set; } = new ObjTrig[MAX_OBJ_TRIGS];

        // Constructor, methods, and any additional class functionality go here
    }

    // Supporting classes/enums and constants need to be defined according to your game's specifics.
    // Examples include:
    // public class ExtraDescrData { /* definition */ }
    // public class AffectData { /* definition */ }
    // public class ObjIndexData { /* definition */ }
    // public class RoomIndexData { /* definition */ }
    // public class ElevatorDestList { /* definition */ }
    // public class ObjTrig { /* definition */ }
    // const int MAX_EXTRA_FLAGS = /* appropriate value */;
    // const int MAX_BLADE_SPELLS = /* appropriate value */;
    // const int MAX_BURST = /* appropriate value */;
    // const int MAX_OBJ_TRIGS = /* appropriate value */;
}
