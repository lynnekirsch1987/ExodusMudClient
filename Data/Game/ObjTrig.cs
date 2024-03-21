namespace ExodusMudClient.Data.Game {
    public class ObjTrig {
        public ObjTrig Next { get; set; }
        public ObjTrig NextOt { get; set; }
        public ObjTrig NextOnTick { get; set; }
        public bool Valid { get; set; }
        public int StatementOn { get; set; }
        public int SecondsLeft { get; set; }
        public ObjTrigData PIndexData { get; set; } // Assuming a class named ObjTrigData exists
        public ObjData ObjOn { get; set; } // Assuming a class named ObjData exists
        public CharData Ch { get; set; } // Assuming a class named CharData exists for characters
        public CharData Victim { get; set; }

        public ObjTrig() {
            // Constructor logic here
        }

        // Additional methods and functionality for the ObjTrig class...
    }
    public class ObjTrigData {
        public ObjTrigData Next { get; set; }
        public bool Valid { get; set; }
        public int Vnum { get; set; }
        public string Name { get; set; }
       // public CharData Fakie { get; set; } // Simulation character data
        public string Trigger { get; set; } // The trigger string
        public long TrigFlags { get; set; } // Conditions that will trigger the action
        public long ExtraFlags { get; set; }
        public string[] Action { get; set; } = new string[16]; // Actions to be taken when triggered
        public int[] Chance { get; set; } = new int[16]; // Chance of each action being triggered
        public int[] Delay { get; set; } = new int[16]; // Delay before action is triggered
        public int OverallChance { get; set; } // Overall chance of the trigger firing

        // Constructor, methods for handling trigger activation or behaviors would go here.
    }


}
