namespace ExodusMudClient.Data.Game {
    public class ScentData {
        public ScentData Next { get; set; } // Next scent in the global list
        public ScentData NextInRoom { get; set; } // Next scent in the specific room
        //public CharData Player { get; set; } // The character that left the scent
        public Room InRoom { get; set; } // The room where the scent is located
        public short ScentLevel { get; set; } // The intensity or level of the scent

        public ScentData() {
            // Constructor logic here
        }

        // Additional methods and functionality for the ScentData class can be added as needed
    }
}
