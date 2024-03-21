namespace ExodusMudClient.Data.Game {
    public class AffectData {
        public AffectData Next { get; set; }
        public bool Valid { get; set; }
        public short Where { get; set; }
        public short Type { get; set; }
        public short Level { get; set; }
        public short Duration { get; set; }
        public short Location { get; set; }
        public short Modifier { get; set; }
        public int Bitvector { get; set; }
        public bool Permaff { get; set; } // Assuming this indicates a permanent affect
        public bool Composition { get; set; } // Assuming this relates to a composed affect, possibly from multiple sources
        public string CompName { get; set; } // Name of the composition, if applicable

        // Constructor and any additional functionality or methods would go here
        public AffectData() {
            // Initialize the AffectData object with default values or logic as necessary
        }
    }
}
