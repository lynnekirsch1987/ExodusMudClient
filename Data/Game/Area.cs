namespace ExodusMudClient.Data.Game {
    public class Area {
        public Area Next { get; set; }
        public ResetData ResetFirst { get; set; }
        public ResetData ResetLast { get; set; }
        public ObjTrigData OtFirst { get; set; }
        public ObjTrigData OtLast { get; set; }
        public string FileName { get; set; }
        public string Name { get; set; }
        public string Credits { get; set; }
        public int Points { get; set; }
        public int Clan { get; set; }
        public string Help { get; set; }
        public int Age { get; set; }
        public int NPlayer { get; set; }
        public int LowRange { get; set; }
        public int HighRange { get; set; }
        public int MinVnum { get; set; }
        public int MaxVnum { get; set; }
        public bool Empty { get; set; }
        public string Creator { get; set; }
        public string Helper { get; set; }
        public bool NoClan { get; set; }
        public bool Construct { get; set; }
        public int Llev { get; set; }
        public int Ulev { get; set; }
        public int Lvnum { get; set; }
        public int Uvnum { get; set; }
        public int Vnum { get; set; }
        public int AreaFlags { get; set; }
        public int Recall { get; set; }
        public int Version { get; set; }
        public List<Mobile> Mobiles { get; set; }
        public List<Room> Rooms { get; set; }
        public List<Object> Objects { get; set; }
        public List<Shop> Shops { get; set; }
        public List<ResetData> Resets { get; set; }
        public List<ObjTrigData> ObjectTriggers { get; set; }
    }
}