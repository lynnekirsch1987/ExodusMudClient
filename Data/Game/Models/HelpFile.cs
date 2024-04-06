namespace ExodusMudClient.Data.Game.Models
{
    public class HelpEntry
    {
        public string Id { get; set; }
        public int LevelAccess { get; set; }
        public string Keywords { get; set; }
        public string HelpText { get; set; }
        public string Category { get; set; }
        public List<string> Tags { get; set; } = new List<string>();
        // tags that are related to this help
        public List<string> RelatedTags { get; set; } = new List<string>();

        public static List<string> Categories = new List<string>
        {
            "Game Mechanics", "Classes", "Races", "Directions", "Areas", "Building", "Immortal","Clans","Skills","Spells"
        };
    }
}