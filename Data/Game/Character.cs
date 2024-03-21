using System;
using System.Collections.Generic;
using System.Xml.Linq;
using static ExodusMudClient.Data.Game.Enums;
using static ExodusMudClient.Data.Game.GlobalConstants;

namespace ExodusMudClient.Data.Game {
    public class TalkList {
        public string Word { get; set; }
        public TalkList Next { get; set; }

        public TalkList(string word) {
            Word = word;
        }
    }
    public class ResponseList {
        public string Wassaid { get; set; }
        public string Text { get; set; }
        public int NewMood { get; set; }
        public ResponseList Next { get; set; }

        public ResponseList(string wassaid,string text,int newMood) {
            Wassaid = wassaid;
            Text = text;
            NewMood = newMood;
        }
    }
    public class CompData {
        public CompData Next { get; set; }
        public string Name { get; set; }
        public string Title { get; set; }
        public int Type { get; set; }
        public int Subtype { get; set; }
        public int Skill { get; set; }
        public int CompLevels { get; set; }

        public CompData(string name,string title) {
            Name = name;
            Title = title;
        }
    }
    public class MemoryData {
        public MemoryData Next { get; set; }
        public MemoryData NextMemory { get; set; }
        public CharData Player { get; set; }
        public CharData Mob { get; set; }

        public MemoryData(CharData player,CharData mob) {
            Player = player;
            Mob = mob;
        }
    }


    public class Buffer {
        public Buffer Next { get; set; }
        public bool Valid { get; set; }
        public int State { get; set; } // Error state of the buffer
        public int Size { get; set; } // Size in kilobytes (k)
        public string String { get; set; } // Buffer's string content

        // Constructor for initializing a new instance of Buffer
        public Buffer(int size) {
            this.Valid = true; // Assuming a new Buffer should be valid by default
            this.Size = size;
            this.String = string.Empty; // Initialize the string to empty
        }

        // Additional methods or properties can be added here if needed
    }
    public class AggressorList {
        public AggressorList Next { get; set; }
        public bool Valid { get; set; }
        public CharData Ch { get; set; }
        public int TicksLeft { get; set; }
        public bool Silent { get; set; }

        // Constructor for initializing a new instance of AggressorList
        public AggressorList(CharData ch,int ticksLeft,bool silent) {
            this.Valid = true; // Assuming a new AggressorList instance should be valid by default
            this.Ch = ch;
            this.TicksLeft = ticksLeft;
            this.Silent = silent;
        }

        // Additional methods or properties can be added here if needed
    }
    public class PcData {
        public PcData Next { get; set; }
        public Buffer Buffer { get; set; } // Placeholder for actual Buffer type
        public AggressorList AggList { get; set; } // Placeholder for actual AggressorList type
        public bool Valid { get; set; }
        public bool AutoassistLevel { get; set; }
        public string Pwd { get; set; }
        public string Bamfin { get; set; }
        public string Bamfout { get; set; }
        public string Whoinfo { get; set; }
        public string Pretitle { get; set; }
        public string Battlecry { get; set; }
        public string VocabFile { get; set; }
        public string Plan { get; set; }
        public string Title { get; set; }
        public string Restoremsg { get; set; }
        public DateTime Lastlogoff { get; set; }
        public DateTime LastKai { get; set; }
        public string EmailAddr { get; set; }
        public bool Deputy { get; set; }
        public bool Anonymous { get; set; }
        public CharData Boarded { get; set; } // Placeholder for CharData type
        public string Speedwalk { get; set; }
        public int Speedlen { get; set; }
        public long BankGold { get; set; }
        public long BankSilver { get; set; }
        public int PrimaryHand { get; set; }
        public DateTime LastFight { get; set; }
        public DateTime LastNote { get; set; }
        public DateTime LastIdea { get; set; }
        public DateTime LastPenalty { get; set; }
        public DateTime LastNews { get; set; }
        public DateTime LastChanges { get; set; }
        public DateTime LastLegend { get; set; }
        public DateTime LastOocnote { get; set; }
        public DateTime LastProjects { get; set; }
        public int PermHit { get; set; }
        public int PermMana { get; set; }
        public int PermMove { get; set; }
        public int TrueSex { get; set; }
        public bool Security { get; set; }
        public bool BuildAll { get; set; }
        public int LastLevel { get; set; }
        public int[] Condition { get; set; } = new int[5];
        public int[] Learned { get; set; } = new int[MAX_SKILL]; // Assuming MAX_SKILL is defined
        public int[] ModLearned { get; set; } = new int[MAX_SKILL]; // Same assumption
        public bool[] GroupKnown { get; set; } = new bool[MAX_GROUP]; // Assuming MAX_GROUP is defined
        public int Points { get; set; }
        public bool ConfirmDelete { get; set; }
        public string[] Ignore { get; set; } = new string[MAX_IGNORE]; // Assuming MAX_IGNORE is defined
        public string[] Alias { get; set; } = new string[MAX_ALIAS]; // Assuming MAX_ALIAS is defined
        public string[] AliasSub { get; set; } = new string[MAX_ALIAS]; // Same assumption
        public string[] History { get; set; } = new string[MAX_HISTORY]; // Assuming MAX_HISTORY is defined
        public int[] BeenKilled { get; set; } = new int[5]; // Simplified array for tracking kills
        public int[] HasKilled { get; set; } = new int[5]; // Simplified array for tracking kills
                                                           // Add other properties as needed...

        // Constructor and methods...
    }
    public class CharData {
        // Assuming definitions for other classes like COMP_DATA, SPEC_FUN, MOB_INDEX_DATA, etc.
        public CharData Next { get; set; }
        public CharData NextInRoom { get; set; }
        public CharData NextInBoard { get; set; }
        public CharData Master { get; set; }
        public CharData Stalking { get; set; }
        public CharData Leader { get; set; }
        public CharData Fighting { get; set; }
        public CharData Aggres { get; set; }
        public CharData Reply { get; set; }
        public CharData Pet { get; set; }
        public CharData Mount { get; set; }
        public CompData Compositions { get; set; }
        public CharData LastFought { get; set; }
        public MemData Memory { get; set; }
        public SpecFun SpecFun { get; set; } // Delegate for spec_fun
        public Mobile PIndexData { get; set; }
        public DescriptorData Desc { get; set; }
        public AffectData Affected { get; set; }
        public NoteData Pnote { get; set; }
        public ObjData Carrying { get; set; }
        public ObjData On { get; set; }
        public Room InRoom { get; set; }
        public Room WasInRoom { get; set; }
        public Area Zone { get; set; }
        public PcData Pcdata { get; set; }
        public GenData GenData { get; set; }
        public MemoryData HuntMemory { get; set; }
        public ObjData TrapList { get; set; }
        public bool Valid { get; set; }
        public bool Color { get; set; }
        public string Name { get; set; }
        public string Mask { get; set; }
        public string Afkmsg { get; set; }
        // Add other properties as needed...

        public CharData() {
            // Constructor logic here
        }

        // Additional methods and functionality for the CharData class...
    }

    public class GenData {
        public GenData Next { get; set; }
        public bool Valid { get; set; }
        public bool[] SkillChosen { get; set; }
        public bool[] GroupChosen { get; set; }
        public int PointsChosen { get; set; }

        // Assuming MAX_SKILL and MAX_GROUP are defined constants
        private const int MAX_SKILL = 100; // Placeholder value
        private const int MAX_GROUP = 20;  // Placeholder value

        // Constructor initializes the SkillChosen and GroupChosen arrays
        public GenData() {
            Valid = true; // Assume a new GenData object is valid by default
            SkillChosen = new bool[MAX_SKILL];
            GroupChosen = new bool[MAX_GROUP];
            PointsChosen = 0;
        }

        // Additional methods for character generation could be added here
    }


    public class MemData : IDisposable {
        public long Id { get; private set; }
        public static long CurrentPcId { get; private set; }
        public static long CurrentMobId { get; private set; }
        private static List<MemData> memoryList = new List<MemData>();

        // Constructor to create MemData with a unique ID
        public MemData(bool isPc) {
            Id = isPc ? GetPcId() : GetMobId();
            memoryList.Add(this);
        }

        // Generates or retrieves a unique identifier for player characters
        public static long GetPcId() {
            return ++CurrentPcId; // Increment and return the current PC ID
        }

        // Similar to GetPcId, but for non-player characters or "mobs."
        public static long GetMobId() {
            return ++CurrentMobId; // Increment and return the current Mob ID
        }

        // Searches for a MemData instance within the list based on a provided id.
        public static MemData FindMemory(long id) {
            return memoryList.Find(memory => memory.Id == id);
        }

        // Implementing IDisposable to handle resource cleanup, if necessary.
        public void Dispose() {
            // Remove this instance from the memory list when disposed of.
            memoryList.Remove(this);

            // Additional cleanup logic here
        }
    }
    public class Mobile {
        public Area Area { get; set; }
        public SpecFun SpecFunc { get; set; }
        public Shop Shop { get; set; }
        public int Vnum { get; set; }
        public int Group { get; set; }
        public long RecruitFlags { get; set; }
        public int[] RecruitValue { get; set; } = new int[3];
        public List<TriggerData> Triggers { get; set; }
        public List<VariableData> Variables { get; set; }
        public string ScriptFilename { get; set; }
        public int Count { get; set; }
        public int Killed { get; set; }
        public int DefaultMood { get; set; }
        public string PlayerName { get; set; }
        public string ShortDescription { get; set; }
        public string LongDescription { get; set; }
        public string Description { get; set; }
        public string Vocfile { get; set; }
        public string LastKiller { get; set; }
        public string LastKiller2 { get; set; }
        public ActFlags Act { get; set; }
        public AffectedByFlags AffectedBy { get; set; }
        public int Alignment { get; set; }
        // Mounts
        public int DefBonus { get; set; }
        public int AttackBonus { get; set; }
        public int MaxWeight { get; set; }
        // End mounts
        public int Level { get; set; }
        public int Hitroll { get; set; }
        public int[] Hit { get; set; } = new int[3];
        public int[] Mana { get; set; } = new int[3];
        public int[] Damage { get; set; } = new int[3];
        public int[] AC { get; set; } = new int[4];
        public DamageType DamType { get; set; }
        public long OffFlags { get; set; }
        public long ImmFlags { get; set; }
        public long ResFlags { get; set; }
        public long VulnFlags { get; set; }
        public PositionType StartPos { get; set; }
        public PositionType DefaultPos { get; set; }
        public SexType Sex { get; set; }
        public int Class { get; set; }
        public int Race { get; set; }
        public string RaceStr { get; set; }
        public long Wealth { get; set; }
        public long Form { get; set; }
        public long Parts { get; set; }
        public int Size { get; set; }
        public int Move { get; set; }
        public string Material { get; set; }
        public int BlocksExit { get; set; }
        public int NumberOfAttacks { get; set; }
        public int[] PermStat { get; set; } = new int[MAX_STATS]; // Define MAX_STATS according to your game's requirements
        public int CardVnum { get; set; }

        // Constructors, methods, and any additional functionality would follow.
    }

    public class DescriptorData {
        public DescriptorData Next { get; set; }
        public DescriptorData SnoopBy { get; set; }
        public CharData Character { get; set; }
        public CharData Original { get; set; }
        public bool Valid { get; set; }
        public string Host { get; set; }
        public int Port { get; set; } // Stores user's remote port
        public string RealHost { get; set; }
        public int Descriptor { get; set; }
        public int Connected { get; set; }
        public bool FCommand { get; set; }
        public string InBuf { get; set; } // Simplified from char array to string
        public string InComm { get; set; }
        public string InLast { get; set; }
        public int Repeat { get; set; }
        public string OutBuf { get; set; } // Simplifying buffer management to a single string
        public int OutSize { get; set; }
        public int OutTop { get; set; }
        public string ShowStrHead { get; set; }
        public string ShowStrPoint { get; set; }
        public object PEdit { get; set; } // Assuming a generic object; specify further as needed
        public string[] PString { get; set; } // Pointer to string, translated to string array
        public int Editor { get; set; }
        public int OldEditor { get; set; }
        public int DoorDir { get; set; }
        public string SubmitInfo { get; set; }
        public string DoorName { get; set; }
        public string HelpInfo { get; set; }
        public string HelpName { get; set; }
        public bool Ansi { get; set; }

        // Constructor and any necessary methods here
    }
}

