namespace ExodusMudClient.Data.Game {
    public class ExtraDescrData {
        public ExtraDescrData Next { get; set; } // Next extra description in the list
        public bool Valid { get; set; } // To check if the extra description is valid
        public string Keyword { get; set; } // Keywords associated with this description
        public string Description { get; set; } // The descriptive text

        // Constructor to initialize an instance of ExtraDescrData
        public ExtraDescrData() {
            // Initializations can be done here, if necessary
        }

        // Additional functionality or methods can be added as needed
    }
}
