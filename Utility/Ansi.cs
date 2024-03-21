using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace ExodusMudClient.Utility {
    public static class AnsiHelper {
        public static string ToHtml(string input) {
            var builder = new StringBuilder();
            var spanStack = new Stack<string>();

            // Adjusted the dictionary to handle both individual and compound ANSI codes.
            var ansiToCss = new Dictionary<string,string>
            {
                { "1;30", "<span style='color: grey;'>" }, // Dark grey (bright black)
                { "1;37", "<span style='color: white;'>" }, // Bright white
                { "0;30", "<span style='color: black;'>" }, // Black
                { "0;31", "<span style='color: red;'>" }, // Red
                { "0;32", "<span style='color: lime;'>" }, // Green
                { "0;33", "<span style='color: yellow;'>" }, // Yellow
                { "0;34", "<span style='color: blue;'>" }, // Blue
                { "0;35", "<span style='color: magenta;'>" }, // Magenta
                { "0;36", "<span style='color: cyan;'>" }, // Cyan
                { "0;37", "<span style='color: white;'>" }, // White
                { "1;31", "<span style='color: #ff5555;'>" }, // Bright Red
                { "1;32", "<span style='color: #55ff55;'>" }, // Bright Green
                { "1;33", "<span style='color: #ffff55;'>" }, // Bright Yellow
                { "1;34", "<span style='color: #5555ff;'>" }, // Bright Blue
                { "1;35", "<span style='color: #ff55ff;'>" }, // Bright Magenta
                { "1;36", "<span style='color: #55ffff;'>" }, // Bright Cyan
                { "40", "<span style='background-color: black;'>" }, // Background Black
                { "41", "<span style='background-color: red;'>" }, // Background Red
                { "42", "<span style='background-color: green;'>" }, // Background Green
                { "43", "<span style='background-color: yellow;'>" }, // Background Yellow
                { "44", "<span style='background-color: blue;'>" }, // Background Blue
                { "45", "<span style='background-color: magenta;'>" }, // Background Magenta
                { "46", "<span style='background-color: cyan;'>" }, // Background Cyan
                { "47", "<span style='background-color: white;'>" }, // Background White
                { "1", "<span style='font-weight:bold;'>" }, // Bold/Bright
                { "3", "<span style='font-style:italic;'>" }, // Italic
                { "4", "<span style='text-decoration:underline;'>" }, // Underline
                { "0", "<span style='color: initial; font-weight: normal;'>" }, // Reset to default
                { "0;1;37", "<span style='font-weight:bold; color: white;'>" }, // Example for Bright White
 
            };

            // Improved regex to capture compound ANSI codes
            var regex = new Regex(@"\x1b\[([\d;]+)m");
            int lastIndex = 0;

            foreach (Match match in regex.Matches(input)) {
                builder.Append(input.Substring(lastIndex,match.Index - lastIndex));
                var code = match.Groups[1].Value; // Captures the entire code sequence
                lastIndex = match.Index + match.Length;

                if (ansiToCss.TryGetValue(code,out string htmlTag)) {
                    // If it's a reset code, close all opened spans
                    if (code == "0") {
                        while (spanStack.Count > 0) {
                            builder.Append(spanStack.Pop());
                        }
                    } else {
                        builder.Append(htmlTag);
                        spanStack.Push("</span>"); // Push closing tag for every opened span
                    }
                }
            }

            // Append the remaining text
            if (lastIndex < input.Length) {
                builder.Append(input.Substring(lastIndex));
            }

            // Close any remaining opened spans
            while (spanStack.Count > 0) {
                builder.Append(spanStack.Pop());
            }

            return builder.ToString();
        }
    }
}