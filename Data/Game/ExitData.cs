namespace ExodusMudClient.Data.Game {
    public class ExitData {
        private Room _toRoom;
        private int _vnum;

        // Represents the room this exit leads to. If set, also updates the vnum to the room's vnum.
        public Room ToRoom {
            get => _toRoom;
            set {
                _toRoom = value;
                _vnum = value != null ? value.Vnum : 0;
            }
        }

        // Represents the vnum of the room this exit leads to. Setting this does not update the ToRoom property.
        public int Vnum {
            get => _toRoom?.Vnum ?? _vnum;
            set => _vnum = value;
        }

        public ExitData Next { get; set; } // Next exit in the list (for OLC)
        public int RsFlags { get; set; } // OLC-specific flags for the exit
        public int OrigDoor { get; set; } // Original door number (for OLC)
        public short ExitInfo { get; set; } // Flags for the exit's state (locked, hidden, etc.)
        public short Key { get; set; } // The key's vnum that opens this exit, if any
        public string Keyword { get; set; } // Keywords for the exit (e.g., "door", "gate")
        public string Description { get; set; } // Description of the exit

        public ExitData() {
            // Constructor logic here
        }

        // Additional methods and logic for the ExitData class as needed
    }
}
