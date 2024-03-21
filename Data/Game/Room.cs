namespace ExodusMudClient.Data.Game {
    public class Room {
        public Room Next { get; set; }
        public List<ResetData> Resets { get; set; }
        public ScentData Scents { get; set; }
        public CharData People { get; set; }
        public ObjData Contents { get; set; }
        public List<ExtraDescrData> ExtraDescriptions { get; set; }
        public Area Area { get; set; }
        public ExitData[] Exits { get; set; } = new ExitData[6];
        public EplProg EplProg { get; set; }
        public string EplFilename { get; set; }
        public string Name { get; set; }
        public string Description { get; set; }
        public string Owner { get; set; }
        public string EnterMessage { get; set; }
        public string ExitMessage { get; set; }
        public int Vnum { get; set; }
        public short MaxInRoom { get; set; }
        public string MaxMessage { get; set; }
        public long RoomFlags { get; set; }
        public long RoomFlags2 { get; set; }
        public short Light { get; set; }
        public short SectorType { get; set; }
        public short HealRate { get; set; }
        public short ManaRate { get; set; }
        public short Clan { get; set; }
        public short MaxLevel { get; set; }
        public long RaceFlags { get; set; }
        public long ClassFlags { get; set; }
        public int CTransportVnum { get; set; }
        public short TpLevel { get; set; }
        public long TpExp { get; set; }
        public string TpMessage { get; set; }
        // Sinking room messages and properties
        public string SinkMessage { get; set; }
        public string SinkMessageOthers { get; set; }
        public string SinkWarning { get; set; }
        public int SinkTimer { get; set; }
        public int SinkDest { get; set; } // Destination after sinking
    }
}
