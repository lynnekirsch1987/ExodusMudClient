using ExodusMudClient.Data.Game;
using System;
using System.Collections.Generic;
using System.IO;
using Newtonsoft.Json;
using System.Linq;
using System.Text;
using static ExodusMudClient.Data.Game.Enums;

namespace ExodusMudClient.Utility {
    public interface IAreaFileConverter {
        Area ParseAreaFile(string filePath);
    }
    public class AreaFileConverter : IAreaFileConverter {
        public enum Section {
            AREA = 0,
            MOBILES = 1,
            OBJECTS = 2,
            ROOMS = 3,
            RESETS = 4,
            SHOPS = 5,
            SPECIALS = 6
        }

        public Area ConvertedArea { get; set; }

        private Dictionary<Section,Action<List<string>>> parseHandlers;

        public AreaFileConverter() {
            ConvertedArea = new Area();
            parseHandlers = new Dictionary<Section,Action<List<string>>> {
                { Section.AREA, ParseArea },
                { Section.MOBILES, ParseMobiles },
                { Section.OBJECTS, ParseObjects },
                { Section.ROOMS, ParseRooms },
                { Section.RESETS, ParseResets },
                { Section.SHOPS, ParseShops },
                { Section.SPECIALS, ParseSpecials }
            };
        }

        public Area ParseAreaFile(string filePath) {
            var currentSection = Section.AREA;
            var sectionNames = Enum.GetNames(typeof(Section));
            var buffer = new List<string>();

            foreach (var line in File.ReadLines(filePath)) {
                var trimmedLine = line.Trim();
                if (trimmedLine.StartsWith("#$")) break; // end of file
                if (trimmedLine.StartsWith("#")) {
                    if (sectionNames.Any(name => trimmedLine.Contains(name))) {
                        if(buffer.Count > 0) {
                            ParseHandler(currentSection,buffer);
                            buffer.Clear();
                        }
                        Enum.TryParse(sectionNames.First(name => trimmedLine.Contains(name)),out currentSection);
                        continue;
                    }
                }
                buffer.Add(trimmedLine);
            }

            return ConvertedArea;
        }

        public void ParseHandler(Section currentSection,List<string> buffer) {
            if (parseHandlers.TryGetValue(currentSection,out var handler)) {
                handler(buffer);
            }
        }

        public (int DiceCount, int DiceType, int DiceBonus) ParseDiceNotation(string notation) {
            // Simple implementation - adjust as necessary
            var parts = notation.Split(new[] { 'd','+' },StringSplitOptions.RemoveEmptyEntries);
            return (Convert.ToInt32(parts[0]), Convert.ToInt32(parts[1]), Convert.ToInt32(parts[2]));
        }

        public void ParseArea(List<string> buffer) {
            // buffer should contain area meta data
            // data is in order
       
        }

        public void ParseMobiles(List<string> buffer) {
            Mobile? currentMobile = null;
            List<Mobile> mobiles = new List<Mobile>();
            int propertyNum = 0;
            bool inDescription = false;
            StringBuilder fullDescription = new StringBuilder();

            foreach (var line in buffer) {
                var parts = line.Split(' ');
                if (currentMobile == null) {
                    currentMobile = new Mobile {
                        Vnum = Convert.ToInt32(line.Replace("#",""))
                    };
                    propertyNum = 1; // Reset property counter for new mobile
                    continue;
                }

                switch (propertyNum) {
                    case 1:
                        currentMobile.PlayerName = line.TrimEnd('~');
                        break;
                    case 2:
                        currentMobile.ShortDescription = line.TrimEnd('~');
                        break;
                    case 3:
                        currentMobile.LongDescription = line.TrimEnd('~');
                        break;
                    case 4:
                        if(line.StartsWith("~")) {
                            if(!inDescription) {
                                inDescription = true;
                                continue; // move to the next line
                            } else {
                                // if the line starts with ~ and we are already in 
                                // the description property, then we've reached the
                                // end of it. so we exit the description property, 
                                // increase the property num to the next, and immediately
                                // begin the next loop.
                                inDescription = false;
                                propertyNum++; // manual increase
                                continue; // move to the next line
                            }
                        } else {
                            fullDescription.Append(line);
                            continue; // move to the next line
                        }
                    case 5:
                        currentMobile.RaceStr = line.TrimEnd('~');
                        break;
                    case 6:
                        // These four zeros are placeholders/flags, you might decide to skip or parse if relevant:
                        // Example: currentMobile.Flags = ParseFlags(line);
                        break;
                    case 7:
                        // this line has 4 parts
                        var actFlags = parts[0];
                        var affFlags = parts[1];
                        currentMobile.Act = Lookup.FlagConvert<ActFlags>(actFlags);
                        currentMobile.AffectedBy = Lookup.FlagConvert<AffectedByFlags>(actFlags);
                        var alignment = parts[2];
                        var unknown = parts[3];
                        break;
                    case 8:
                        var stats = line.Split(' ');
                        // Assuming the first two numbers are level placeholders, and the third is the level
                        currentMobile.Level = int.Parse(stats[2]);
                        // Further parsing for Hitpoints, Mana, and Damage Dice if applicable
                        break;
                    case 9:
                        currentMobile.Hitroll = int.Parse(line);
                        break;
                    case 10:
                        // Parse Damage dice and type; assuming it follows "1d1+9999 1d1+9999 1d1+9999 divine"
                        var damageParts = line.Split(' ');
                        // Implement parsing logic based on the actual format
                        break;
                    case 11:
                        // Armor class parsing; assuming "0 0 0 0"
                        var armorParts = line.Split(' ');
                        currentMobile.AC = armorParts.Select(int.Parse).ToArray();
                        break;
                    case 12:
                        // Offense, immunity, resistance, vulnerability flags parsing; assuming "0 AB 0 0"
                        var flagParts = line.Split(' ');
                        currentMobile.OffFlags = Convert.ToInt64(flagParts[1],16); // Adjust based on actual data
                                                                                   // Further parsing for ImmFlags, ResFlags, VulnFlags
                        break;
                    case 13:
                        // Parsing default/current position, sex; assuming "stand stand male 0"
                        var positionParts = line.Split(' ');
                        currentMobile.StartPos = Enum.Parse<PositionType>(positionParts[0],true);
                        currentMobile.DefaultPos = Enum.Parse<PositionType>(positionParts[1],true);
                        currentMobile.Sex = Enum.Parse<SexType>(positionParts[2],true);
                        // Assuming the fourth part is not used directly in the Mobile class
                        break;
                    case 14:
                        // Form, parts, size, material parsing; assuming "AHMV ABCDEFGHIJKa medium flesh~"
                        var formParts = line.Split(' ');
                        currentMobile.Form = Convert.ToInt64(formParts[0],16); // Adjust based on actual data
                                                                               // Further parsing for Parts, Size, Material if needed
                        break;
                    case 15:
                        // Skipping or parsing custom attributes as applicable
                        break;
                    case 16:
                        // Assuming this case is for vocational file, script file, or other custom parsing needs
                        currentMobile.Vocfile = line.TrimEnd('~');
                        break;
                    case 17:
                        currentMobile.ScriptFilename = line.TrimEnd('~');
                        propertyNum = 0; // Prepare for next mobile
                        mobiles.Add(currentMobile);
                        currentMobile = null; // Clear current mobile for next iteration
                        break;
                }
                propertyNum++;
            }

            ConvertedArea.Mobiles = mobiles;
        }

        public void ParseObjects(List<string> buffer) { }
        public void ParseRooms(List<string> buffer) { }
        public void ParseResets(List<string> buffer) { }
        public void ParseShops(List<string> buffer) { }
        public void ParseSpecials(List<string> buffer) { }

        public void SaveAreaToJson(string outputFilePath) {
            var json = JsonConvert.SerializeObject(ConvertedArea,Formatting.Indented);
            File.WriteAllText(outputFilePath,json);
        }


    }
}
