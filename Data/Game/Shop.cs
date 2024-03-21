namespace ExodusMudClient.Data.Game {
    public class Shop {
        public int Keeper { get; set; } // Vnum of shop keeper mob
        public List<int> BuyType { get; set; } = new List<int>(new int[5]); // Item types shop will buy
        public int ProfitBuy { get; set; } // Cost multiplier for buying
        public int ProfitSell { get; set; } // Cost multiplier for selling
        public int OpenHour { get; set; } // First opening hour
        public int CloseHour { get; set; } // First closing hour

        // Constructor
        public Shop() {
        }

        // Additional methods to interact with the shop could be defined here
    }
}
