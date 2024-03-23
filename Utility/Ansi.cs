using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace ExodusMudClient.Utility {
    public static class AnsiHelper {
        public static string ToHtml(string input) {
            var builder = new StringBuilder();
            var spanStack = new Stack<string>();

            // Adjusted the dictionary to correctly map ANSI codes to CSS styles based on the provided color table.
            var ansiToCss = new Dictionary<string,string> {
                { "\x1b[1;30m", "<span style='color: grey;'>" }, // Grey
                { "\x1b[0;31m", "<span style='color: red;'>" }, // Red
                { "\x1b[0;32m", "<span style='color: lime;'>" }, // Green
                { "\x1b[0;33m", "<span style='color: yellow;'>" }, // Yellow
                { "\x1b[0;34m", "<span style='color: blue;'>" }, // Blue
                { "\x1b[0;35m", "<span style='color: magenta;'>" }, // Purple
                { "\x1b[0;36m", "<span style='color: cyan;'>" }, // Cyan
                { "\x1b[0;37m", "<span style='color: white;'>" }, // White
                // Bold colors
                { "\x1b[0;1;31m", "<span style='color: #ff5555;'>" }, // Bold Red
                { "\x1b[0;1;32m", "<span style='color: #55ff55;'>" }, // Bold Green
                { "\x1b[0;1;33m", "<span style='color: #ffff55;'>" }, // Bold Yellow
                { "\x1b[0;1;34m", "<span style='color: #5555ff;'>" }, // Bold Blue
                { "\x1b[0;1;35m", "<span style='color: #ff55ff;'>" }, // Bold Purple
                { "\x1b[0;1;36m", "<span style='color: #55ffff;'>" }, // Bold Cyan
                { "\x1b[0;1;37m", "<span style='color: white; font-weight:bold;'>" }, // Bold White
                // Blink colors
                { "\x1b[0;5;30m", "<span style='animation: blink-animation 1s step-start 0s infinite; color: grey;'>" }, // Blink Grey
                { "\x1b[0;5;31m", "<span style='animation: blink-animation 1s step-start 0s infinite; color: red;'>" }, // Blink Red
                { "\x1b[0;5;32m", "<span style='animation: blink-animation 1s step-start 0s infinite; color: lime;'>" }, // Blink Green
                { "\x1b[0;5;33m", "<span style='animation: blink-animation 1s step-start 0s infinite; color: yellow;'>" }, // Blink Yellow
                { "\x1b[0;5;34m", "<span style='animation: blink-animation 1s step-start 0s infinite; color: blue;'>" }, // Blink Blue
                { "\x1b[0;5;35m", "<span style='animation: blink-animation 1s step-start 0s infinite; color: magenta;'>" }, // Blink Purple
                { "\x1b[0;5;36m", "<span style='animation: blink-animation 1s step-start 0s infinite; color: cyan;'>" }, // Blink Cyan
                { "\x1b[0;5;37m", "<span style='animation: blink-animation 1s step-start 0s infinite; color: white;'>" }, // Blink White
                // Inverse colors (using background-color for demonstration)
                { "\x1b[0;7;31m", "<span style='background-color: red;'>" }, // Inverse Red
                { "\x1b[0;7;32m", "<span style='background-color: green;'>" }, // Inverse Green
                { "\x1b[0;7;33m", "<span style='background-color: yellow;'>" }, // Inverse Yellow
                { "\x1b[0;7;34m", "<span style='background-color: blue;'>" }, // Inverse Blue
                { "\x1b[0;35;7m", "<span style='background-color: magenta;'>" }, // Inverse Purple
                { "\x1b[0;7;36m", "<span style='background-color: cyan;'>" }, // Inverse Cyan
                { "\x1b[0;7;37m", "<span style='background-color: white;'>" }, // Inverse White
                { "\x1b[0m", "<span style='color: white;'>" }, // default
            };

            var regex = new Regex(@"\x1b\[([\d;]+)m");
            int lastIndex = 0;

            foreach (Match match in regex.Matches(input)) {
                builder.Append(input.Substring(lastIndex,match.Index - lastIndex));
                var code = match.Groups[1].Value;
                var ansiCode = "\x1b[" + code + "m"; // Reconstruct the full ANSI code
                lastIndex = match.Index + match.Length;

                if (ansiToCss.TryGetValue(ansiCode,out string htmlTag)) {
                    if (code == "0") { // Reset
                        while (spanStack.Count > 0) {
                            builder.Append(spanStack.Pop());
                        }
                    } else {
                        builder.Append(htmlTag);
                        spanStack.Push("</span>"); // Assume every ANSI code opens a span that needs to be closed
                    }
                }
            }

            if (lastIndex < input.Length) {
                builder.Append(input.Substring(lastIndex));
            }

            while (spanStack.Count > 0) {
                builder.Append(spanStack.Pop()); // Close any remaining opened spans
            }

            return builder.ToString();
        }
    }
}