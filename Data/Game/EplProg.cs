namespace ExodusMudClient.Data.Game {
    public class EplProg {
        public bool Running { get; set; }
        public int Wait { get; set; }

        // Assuming EplFunc is a delegate or class you've defined elsewhere
        // that encapsulates the functionality of the EPL functions.
        public List<EplFunc> Funcs { get; set; }
        public EplFunc Where { get; set; }

        public EplProg Next { get; set; }

        public EplProg() {
            Funcs = new List<EplFunc>();
        }

        // Additional methods to manage program execution, like starting, stopping,
        // waiting, and executing the next function in the sequence.
    }

    public class EplFunc {
        public string Name { get; set; }
        public string Text { get; set; }
        public string Where { get; set; }
        public string PassArgs { get; set; }
        public string ArgTypes { get; set; }
        public object ReturnVal { get; set; } // Replacing void* for generic return value
        public bool ReturnWait { get; set; }

        // Assuming RoomVar, CharVar, ObjVar are classes you've defined elsewhere
        // that encapsulate the variables specific to rooms, characters, and objects.
        public List<RoomVar> RoomVars { get; set; }
        public List<CharVar> CharVars { get; set; }
        public List<ObjVar> ObjVars { get; set; }

        public EplFunc Caller { get; set; }
        public EplFunc Next { get; set; }

        public EplFunc() {
            RoomVars = new List<RoomVar>();
            CharVars = new List<CharVar>();
            ObjVars = new List<ObjVar>();
        }

        // Additional methods for function execution, variable management, etc.
    }

    public class RoomVar {
        public string Name { get; set; }
        public Room Value { get; set; } // Assuming RoomIndexData is already defined
        public RoomVar Next { get; set; }

        public RoomVar() {
            // Constructor logic here
        }
    }

    public class CharVar {
        public string Name { get; set; }
        //public CharData Value { get; set; } // Assuming CharData is already defined
        public CharVar Next { get; set; }

        public CharVar() {
            // Constructor logic here
        }
    }

    public class ObjVar {
        public string Name { get; set; }
        public Object Value { get; set; } // Assuming ObjData is already defined
        public ObjVar Next { get; set; }

        public ObjVar() {
            // Constructor logic here
        }
    }


}
