namespace ExodusMudClient.Data.Game {


    public class NoteData {
        public NoteData Next { get; set; }
        public bool Valid { get; set; }
        public short Type { get; set; }
        public string Sender { get; set; }
        public string Date { get; set; }
        public string ToList { get; set; }
        public string Subject { get; set; }
        public string Text { get; set; }
        public DateTime DateStamp { get; set; }

        // Constructor to initialize a new note with mandatory fields
        public NoteData(string sender,string toList,string subject,string text) {
            Valid = true; // Assuming new notes are valid by default
            Sender = sender;
            ToList = toList;
            Subject = subject;
            Text = text;
            DateStamp = DateTime.Now; // Set the current date and time as the default timestamp
        }

        // Additional methods can be implemented as needed, such as for formatting the note text
    }

}
