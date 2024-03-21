namespace ExodusMudClient.Data.Game {
    public class ResetData {
        public ResetData Next { get; set; }
        public char Command { get; set; }
        public int Arg1 { get; set; }
        public int Arg2 { get; set; }
        public int Arg3 { get; set; }
        public int Arg4 { get; set; }

        // Constructor for initializing a new reset data instance.
        public ResetData(char command,int arg1,int arg2,int arg3,int arg4) {
            Command = command;
            Arg1 = arg1;
            Arg2 = arg2;
            Arg3 = arg3;
            Arg4 = arg4;
        }

        // Additional methods for handling reset application or behavior would go here.
    }
}
