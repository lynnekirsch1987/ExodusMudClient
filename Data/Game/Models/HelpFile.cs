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
        public List<string> RelatedTags { get; set; } = new List<string>();
    }
}