using System.Text.Json.Serialization;
using ExodusMudClient.Data.Static;
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
        public string VnumsHighLow { get; set; }
        public string Uknown1 { get; set; }
        public string Unkown2 { get; set; }
        public string Unkown3 { get; set; }
        public string Unkown4 { get; set; }

        public List<AreaFileMobile> Mobiles { get; set; }
        public List<AreaFileRoom> Rooms { get; set; }
        public List<AreaFileObject> Objects { get; set; }
        public List<AreaFileReset> Resets { get; set; }
    }

    public class AreaFileMobile : IAreaInterface
    {
        public int Vnum { get; set; }
        public string NameForTargeting { get; set; }
        public string ShortDescription { get; set; }
        public string LongDescription { get; set; }
        public string Description { get; set; }
        public string Race { get; set; }
        public string RecruitFlags { get; set; }
        public string DamageType { get; set; }
        public string OffFlags { get; set; }
        public string ImmFlags { get; set; }
        public string ResFlags { get; set; }
        public string VulnFlags { get; set; }
        public string Form { get; set; }
        public string Parts { get; set; }
        public string Size { get; set; }
        public string Material { get; set; }
        public string VocabFile { get; set; }
        public string ScriptFile { get; set; }
        public string AreaFileName { get; set; }
        public ActFlags ActFlags { get; set; }
        public AffectedByFlags AffectFlags { get; set; }
        public PositionType StartPosition { get; set; }
        public PositionType DefaultPosition { get; set; }
        public SexType Sex { get; set; }
        public int Alignment { get; set; }
        public int Group { get; set; }
        public int Level { get; set; }
        public int HitRoll { get; set; }
        public int DefaultMood { get; set; }
        public int DefenseBonus { get; set; }
        public int AttackBonus { get; set; }
        public int MaxWeight { get; set; }
        public int Move { get; set; }
        public int BlocksExit { get; set; }
        public int NumberOfAttacks { get; set; } = 0;
        public int Wealth { get; set; }
        public int CardVnum { get; set; }
        public int[] ArmorClass { get; set; } = new int[4];
        public int[] Stats { get; set; } = new int[6];
        public Dice HitDice { get; set; }
        public Dice ManaDice { get; set; }
        public Dice DamageDice { get; set; }

        [JsonIgnore]
        public AreaFile Area { get; set; }

        [JsonIgnore]
        public List<AreaFileRoom> SpawnRooms { get; set; } = new List<AreaFileRoom>();

        [JsonIgnore]
        public List<AreaFileObject> Drops { get; set; } = new List<AreaFileObject>();
    }

    public class AreaFileObject : IAreaInterface
    {
        public List<string> ExtraFlags { get; set; } = new List<string>();
        public List<int> ObjTrigVnums { get; set; } = new List<int>();
        public List<string> Strings { get; set; } = new List<string>();
        public List<string> Values { get; set; } = new List<string>();
        public string Vnum { get; set; }
        public string Name { get; set; }
        public string ShortDescription { get; set; }
        public string LongDescription { get; set; }
        public string Material { get; set; }
        public string WearFlags { get; set; }
        public string ClassWearFlags { get; set; }
        public string RaceWearFlagString { get; set; }
        public string ClanWearFlagString { get; set; }
        public string Value0Flags { get; set; }
        public string Condition { get; set; }
        public string AreaFileName { get; set; }
        public int Timer { get; set; }
        public int ExtraFlagsNumber { get; set; }
        public int RaceWearFlags { get; set; }
        public int ClanWearFlags { get; set; }
        public int Rarity { get; set; }
        public int Level { get; set; }
        public int Cost { get; set; }
        public int Weight { get; set; }
        public ItemType ItemType { get; set; }

        [JsonIgnore]
        public AreaFile Area { get; set; }

        [JsonIgnore]
        public List<AreaFileMobile> OnMobs { get; set; } = new List<AreaFileMobile>();

        public AreaFileObject()
        {
            Values = Enumerable.Repeat("", 16).ToList();
        }
    }

    public class AreaFileRoom : IAreaInterface
    {
        public int Vnum { get; set; }
        public int Level { get; set; }
        public int Xp { get; set; }
        public int RaceFlags { get; set; }
        public int ClassFlags { get; set; }
        public int MaxLevel { get; set; }
        public int AreaNumber { get; set; }
        public int MaxInRoom { get; set; }
        public int[] Line14 { get; set; } = new int[3];
        public int[] Line18 { get; set; } = new int[3];
        public string Line19 { get; set; }
        public string AreaFileName { get; set; }
        public string Line15 { get; set; }
        public string Line16 { get; set; }
        public string Line17 { get; set; }
        public string MaxMsg { get; set; }
        public string Line11 { get; set; }
        public string Line12 { get; set; }
        public string Line13 { get; set; }
        public string TpMsg { get; set; }
        public string EnterMsg { get; set; }
        public string ExitMsg { get; set; }
        public string RoomName { get; set; }
        public string RoomDescription { get; set; }
        public string RoomFlags { get; set; }
        public string SectorType { get; set; }

        [JsonIgnore]
        public AreaFile Area { get; set; }
    }

    public class AreaFileReset : IAreaInterface
    {
        public int resetNum { get; set; }
        public int virtualNumber { get; set; }
        public int resetTargetVirtualNumber { get; set; }
        public int maxInWorld { get; set; }
        public int maxInRoom { get; set; }
        public int doorState { get; set; }
        public int locationOnBody { get; set; }
        public Directions exit { get; set; }
        public ResetType resetType { get; set; }
        public string comment { get; set; }
        public string AreaFileName { get; set; }

        [JsonIgnore]
        public AreaFile Area { get; set; }
    }

    public interface IAreaInterface
    {

        public string AreaFileName { get; set; }

        [JsonIgnore]
        public AreaFile Area { get; set; }
    }
}
