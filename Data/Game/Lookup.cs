using System.Text.RegularExpressions;
using static ExodusMudClient.Data.Game.Enums;

namespace ExodusMudClient.Data.Game {
    public static class Lookup {
        public static T FlagConvert<T>(string flagString) where T : Enum {
            long bitsum = 0;
            foreach (char letter in flagString) {
                if ('A' <= letter && letter <= 'Z') {
                    bitsum |= 1L << (letter - 'A');
                } else if ('a' <= letter && letter <= 'z') {
                    // Adjust based on your enum's specific layout.
                    // This assumes 'a' continues immediately after 'Z' in bit positions.
                    bitsum |= 1L << (26 + (letter - 'a'));
                }
            }
            return (T)Enum.ToObject(typeof(T),bitsum);
        }
    }
    
}
