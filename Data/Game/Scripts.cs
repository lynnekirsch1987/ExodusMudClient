namespace ExodusMudClient.Data.Game {
    public enum TriggerType {
        Command = 0,
        EachPulse = 1,
        Combat = 2,
        TickPulse = 3,
        Born = 4,
        Gets = 5,
        Exit = 6,
        Social = 7,
        Signal = 8,
        Dies = 9,
        Enter = 10,
        Moves = 11
    }
    [Flags]
    public enum ScriptBits {
        Advance = 0x00000001,
        Halt = 0x00000002
    }
    public class ScriptData {
        public ScriptData Next { get; set; }
        public string Command { get; set; }

        public ScriptData(string command) {
            Command = command;
        }
    }
    public class TriggerData {
        public TriggerData Next { get; set; }
        public ScriptData Script { get; set; }
        public ScriptData Current { get; set; }
        public short Tracer { get; set; }
        public short Waiting { get; set; }
        public TriggerType Type { get; set; }
        public ScriptBits Bits { get; set; }
        public string Keywords { get; set; }
        public string Name { get; set; }

        // Constructor and methods as needed
    }
    public class VariableData {
        public VariableData Next { get; set; }
        public string Name { get; set; }
        public string Value { get; set; }

        public VariableData(string name,string value) {
            Name = name;
            Value = value;
        }
    }

}
