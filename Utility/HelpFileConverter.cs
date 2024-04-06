using ExodusMudClient.Data.Game;
using System;
using System.Collections.Generic;
using System.IO;
using Newtonsoft.Json;
using System.Linq;
using System.Text;
using static ExodusMudClient.Data.Game.Enums;
using ExodusMudClient.Data.Game.Models;
using MudBlazor.Charts;
using System.Text.RegularExpressions;
using Microsoft.AspNetCore.Mvc.Razor;

namespace ExodusMudClient.Utility
{
    public interface IHelpFileConverter
    {
        List<HelpEntry> ParseHelpFile(string pathToHelps);
        List<HelpEntry> GetAllHelpEntries();
        Task SaveHelps(List<HelpEntry> helps);
        Task SaveHelp(HelpEntry help);
    }

    public class HelpFileConverter : IHelpFileConverter
    {
        public IWebHostEnvironment _env { get; set; }
        public HelpFileConverter(IWebHostEnvironment env)
        {
            _env = env;
        }
        static List<string> ParseKeywords(string input)
        {
            if (!string.IsNullOrEmpty(input))
            {
                var pattern = @"'([^']*)'|\b(\w+)\b";
                var matches = Regex.Matches(input, pattern);

                List<string> entries = new List<string>();
                foreach (Match match in matches)
                {
                    if (match.Groups[1].Success)
                    {
                        // Group 1 captures content inside quotes
                        entries.Add(match.Groups[1].Value);
                    }
                    else if (match.Groups[2].Success)
                    {
                        // Group 2 captures unquoted words
                        entries.Add(match.Groups[2].Value);
                    }
                }

                return entries;
            }

            return new List<string>();
        }

        public async Task SaveHelps(List<HelpEntry> helps)
        {
            // sneaky shit here
            foreach (var entry in helps)
            {
                //entry.HelpText = entry.HelpText.Replace("\n\r", "<br>");
            }
            var path = Path.Combine(_env.WebRootPath, "helps/helps.json");
            var json = JsonConvert.SerializeObject(helps);
            await File.WriteAllTextAsync(path, json);

        }

        public async Task SaveHelp(HelpEntry help)
        {
            var helps = GetAllHelpEntries();
            var helpIndex = helps.FindIndex(h => h.Id == help.Id); // Find the index of the help to update
            if (helpIndex != -1) // Check if the help was found
            {
                helps[helpIndex] = help; // Update the help entry directly in the collection
                var path = Path.Combine(_env.WebRootPath, "helps/helps.json");
                var json = JsonConvert.SerializeObject(helps, Formatting.Indented); // Optionally, make the JSON pretty
                await File.WriteAllTextAsync(path, json);
            }
        }


        public List<HelpEntry> GetAllHelpEntries()
        {

            var json = File.ReadAllText(Path.Combine(_env.WebRootPath, "helps/helps.json"));
            var _helpEntries = JsonConvert.DeserializeObject<List<HelpEntry>>(json);
            _helpEntries = _helpEntries
            .OrderBy(h => string.Join(" ", h.Tags ?? new List<string>())).ToList();
            return _helpEntries;
        }
        public List<HelpEntry> ParseHelpFile(string pathToHelps)
        {
            var helpEntries = new List<HelpEntry>();
            if (Directory.Exists(pathToHelps))
            {
                var helps = Directory.GetFiles(pathToHelps);
                foreach (var helpFile in helps)
                {
                    var help = new HelpEntry();
                    StringBuilder buffer = new StringBuilder();
                    var helpFileLines = File.ReadAllLines(helpFile);
                    bool inHelpEntry = false;

                    foreach (var line in helpFileLines)
                    {
                        var parts = line.TrimEnd('~').Split(' ');

                        // if we're not actively buffering in a help
                        // entry
                        if (!inHelpEntry)
                        {
                            // skip null lines between entries
                            if (string.IsNullOrEmpty(line))
                            {
                                continue;
                            }

                            // begin reading new entry
                            inHelpEntry = true;

                            // handle help entry meta data
                            if (parts[0].StartsWith('#'))
                            {
                                help.LevelAccess = -1;
                            }
                            else
                            {
                                if (int.TryParse(parts[0], out int levelAccess))
                                {
                                    help.LevelAccess = levelAccess;
                                }
                                else
                                {
                                    // Handle the case where parts[0] is not a valid integer.
                                    // For example, you can set a default value or log a warning.
                                    help.LevelAccess = 0; // Set a default value or take appropriate action.
                                }
                            }


                            for (var i = 1; i < parts.Length; i++)
                            {
                                // help.Keywords += parts[i] + " ";
                            }

                            // move to next line
                            continue;
                        }

                        // if we're currently reading a help entry and this 
                        //line contains only ~, then we've reached the end 
                        // of the entry.
                        if (line.EndsWith('~'))
                        {
                            inHelpEntry = false;
                            help.HelpText = buffer.ToString();
                            buffer.Clear();
                            helpEntries.Add(help);
                            help = new HelpEntry();
                            continue;
                        }

                        // start buffering the help entry text
                        buffer.AppendLine(line);
                        continue;
                    }
                }
                var json = JsonConvert.SerializeObject(helpEntries);
                File.WriteAllText(pathToHelps + "/helps.json", json);
                return helpEntries;
            }

            return new List<HelpEntry>();
        }
    }
}
