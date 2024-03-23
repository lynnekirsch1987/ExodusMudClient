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

namespace ExodusMudClient.Utility {

    public enum Section {
        AREA = 0,
        MOBILES = 1,
        OBJECTS = 2,
        ROOMS = 3,
        RESETS = 4,
        SHOPS = 5,
        SPECIALS = 6
    }

    public interface IAreaFileConverter {
        AreaFile ParseAreaFile(string filePath, Section? section = null);
        List<AreaFile> GetAreaList(string path);
        string ConvertExodusColor(string input);
    }

    public class AreaFileConverter : IAreaFileConverter {
        public delegate AreaFile ParseHandlerDelegate(List<string> buffer,AreaFile convertedArea);
        private Dictionary<Section,ParseHandlerDelegate> parseHandlers;


        public AreaFileConverter() {
            parseHandlers = new Dictionary<Section,ParseHandlerDelegate> {
                { Section.AREA, ParseArea },
                { Section.MOBILES, ParseMobiles }
            };
        }

        public string ConvertExodusColor(string input) {
            var colorCodes = new Dictionary<string,string>
            {
                {"`a", ""},
                {"`b", ""},
                {"`c", "<span style='color: green'>"},
                {"`d", "<span style='color: brown'>"},
                {"`e", "<span style='color: blue'>"},
                {"`f", "<span style='color: purple'>"},
                {"`g", "<span style='color: cyan'>"},
                {"`h", "<span style='color: white'>"},
                {"`i", "<span style='color: bold red'>"},
                {"`j", "<span style='color: bold green'>"},
                {"`k", "<span style='color: bold yellow'>"},
                {"`l", "<span style='color: bold blue'>"},
                {"`m", "<span style='color: bold purple'>"},
                {"`n", "<span style='color: bold cyan'>"},
                {"`o", "<span style='color: bold white'>"},
                {"`p", "<span style='color: blink grey'>"},
                {"`q", "<span style='color: blink red'>"},
                {"`r", "<span style='color: blink green'>"},
                {"`s", "<span style='color: blink yellow'>"},
                {"`t", "<span style='color: blink blue'>"},
                {"`u", "<span style='color: blink purple'>"},
                {"`v", "<span style='color: blink cyan'>"},
                {"``", "<span style='color: blink cyan'>"},
                {"`w", "<span style='color: blink white'>"}
            };

            // Pattern to find keys in the text
            string pattern = "`[a-w]";

            // Replace keys with span tags
            string transformedText = Regex.Replace(input,pattern,match => "");


            return transformedText.Replace("``", "");
        }

        public List<AreaFile> GetAreaList(string path) {
            var files = Directory.GetFiles(path);
    
            List<AreaFile> areas = new List<AreaFile> ();
            foreach (var file in files) {
                areas.Add(ParseAreaFile(file, Section.AREA));
            }
            return areas;
        }

        public AreaFile ParseHandler(Section currentSection,List<string> buffer,AreaFile convertedArea) {
            if (parseHandlers.TryGetValue(currentSection,out var handler)) {
                convertedArea = handler(buffer,convertedArea);
            }
            return convertedArea;
        }

        public AreaFile ParseArea(List<string> buffer,AreaFile convertedArea) {
            // Attempt to parse the version number.
            if (buffer.Count > 0 && int.TryParse(buffer[0].Trim().TrimEnd('~'),out int version)) {
                convertedArea.Version = version;
            } else {
                // Handle the case where the version is not present or not an integer.
                // You could assign a default value or handle it according to your application's logic.
                convertedArea.Version = 0; // Example default value.
            }

            // Assign FileName and AreaName if the buffer has enough lines.
            if (buffer.Count > 1) {
                convertedArea.FileName = buffer[1].TrimEnd('~');
            }
            if (buffer.Count > 2) {
                convertedArea.AreaName = buffer[2].TrimEnd('~'); // Also trimming the potential '~' at the end of area name.
            }

            return convertedArea;
        }

        public AreaFile ParseAreaFile(string filePath,Section? section = null) {
            var currentSection = Section.AREA;
            var sectionNames = Enum.GetNames(typeof(Section));
            var buffer = new List<string>();
            bool processCurrentSection = true;
            var convertedArea = new AreaFile();
            foreach (var line in File.ReadLines(filePath)) {
                var trimmedLine = line.Trim();
                if (trimmedLine.StartsWith("#$")) break; // end of file
                if (trimmedLine.StartsWith("#")) {
                    if (sectionNames.Any(name => trimmedLine.Contains(name))) {
                        if (buffer.Count > 0 && processCurrentSection) {
                            convertedArea = ParseHandler(currentSection,buffer,convertedArea);
                            buffer.Clear();
                        }

                        Enum.TryParse(sectionNames.First(name => trimmedLine.Contains(name)),out currentSection);
                        // Determine if the current section is the one we're interested in
                        processCurrentSection = !section.HasValue || currentSection == section.Value;
                        continue;
                    }
                }
                if (processCurrentSection) {
                    buffer.Add(trimmedLine);
                }
            }

            // Handle the case where the last section is the one we're interested in
            if (buffer.Count > 0 && (!section.HasValue || currentSection == section.Value)) {
                ParseHandler(currentSection,buffer,convertedArea);
            }

            return convertedArea;
        }

        public AreaFile ParseObjects(List<string> buffer, AreaFile convertedArea) {
            return convertedArea;
        }

        public AreaFile ParseMobiles(List<string> buffer,AreaFile convertedArea) {
            AreaFileMobile? currentMobile = null;
            List<AreaFileMobile> mobiles = new List<AreaFileMobile>();
            int propertyNum = 0;
            bool inDescription = false;
            StringBuilder fullDescription = new StringBuilder();

            foreach (var line in buffer) {
                var parts = line.Split(' ');
                if (currentMobile == null) {
                    currentMobile = new AreaFileMobile {
                        Vnum = Convert.ToInt32(line.Replace("#",""))
                    };
                    propertyNum = 1; // Reset property counter for new mobile
                    continue;
                }

                switch (propertyNum) {
                    case 1:
                        currentMobile.NameForTargeting = line.TrimEnd('~');
                        break;
                    case 2:
                        currentMobile.ShortDescription = line.TrimEnd('~');
                        break;
                    case 3:
                        currentMobile.LongDescription = line.TrimEnd('~');
                        break;
                    case 4:
                        if (line.StartsWith("~")) {
                            if (!inDescription) {
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
                        currentMobile.Race = line.TrimEnd('~');
                        break;
                    case 6:
                        // recruit flags maybe for shops?
                        currentMobile.RecruitFlags = line.TrimEnd('~');
                        break;
                    case 7:
                        // this line has 4 parts
                        var actFlags = parts[0];
                        var affFlags = parts[1];
                        currentMobile.ActFlags = Lookup.FlagConvert<ActFlags>(actFlags);
                        currentMobile.AffectFlags = Lookup.FlagConvert<AffectedByFlags>(actFlags);
                        currentMobile.Alignment = Convert.ToInt32(parts[2]);
                        currentMobile.Group = Convert.ToInt32(parts[3]);
                        break;
                    case 8:
                        // 35 35 10d90+535 10d10+50 5d7+20 pound
                        // 35: level
                        // 35: hitroll
                        // 10d90+535: hit dice #, hit dice type, hit dice bonus
                        // 10d10+50: mana dice #, mana dice type, mana dice bonus
                        // 5d7+20: dam dice #, dam dice type, dam dice bonus
                        // pound: dam type
                        currentMobile.Level = int.Parse(parts[0]);
                        currentMobile.HitRoll = int.Parse(parts[1]);
                        currentMobile.HitDice = ConvertStringToDice(parts[2]);
                        currentMobile.ManaDice = ConvertStringToDice(parts[3]);
                        currentMobile.DamageDice = ConvertStringToDice(parts[4]);
                        currentMobile.DamageType = parts[5];
                        break;
                    case 9:
                        // Armor class parsing; assuming "0 0 0 0"
                        var armorParts = line.Split(' ');
                        currentMobile.ArmorClass = armorParts.Select(int.Parse).ToArray();
                        break;
                    case 10:
                        // act 2 flags, 0
                        // blocks exit, 1
                        // (if blocks exit > 50, num_attacks is the next number, otherwise empty), 
                        // off flags, 2
                        // imm flags,3
                        // res flags,4
                        // vuln flags 5
                        //0 -1 CFHKT 0 BCP OS
                        currentMobile.ActFlags |= Lookup.FlagConvert<ActFlags>(parts[0]);
                        currentMobile.BlocksExit = Convert.ToInt32(parts[1]);

                        var index = 2;
                        if (currentMobile.BlocksExit > 50) {
                            currentMobile.NumberOfAttacks = Convert.ToInt32(parts[index]);
                            index++;
                        }

                        currentMobile.OffFlags = parts[index++];
                        currentMobile.ImmFlags = parts[index++];
                        currentMobile.ResFlags = parts[index++];
                        currentMobile.VulnFlags = parts[index];
                        break;
                    case 11:
                        // start position, default position, sex, wealth
                        currentMobile.StartPosition = PositionHelper.ConvertToPositionType(parts[0]);
                        currentMobile.DefaultPosition = PositionHelper.ConvertToPositionType(parts[1]);
                        currentMobile.Sex = Enum.Parse<SexType>(parts[2],true);
                        currentMobile.Wealth = int.Parse(parts[3]);
                        break;
                    case 12:
                        // form, parts, 6x stats, card vnum
                        currentMobile.Form = parts[0];
                        currentMobile.Parts = parts[1];
                        // i don't care about the card bullshit
                        break;
                    case 13:
                        // size, material
                        currentMobile.Size = parts[0];
                        currentMobile.Material = parts[0];
                        break;
                    case 14:
                        // defense bonus, attack bonus, max weight, move
                        // 0 0 0 300
                        currentMobile.DefenseBonus = int.Parse(parts[0]);
                        currentMobile.AttackBonus = int.Parse(parts[1]);
                        currentMobile.MaxWeight = int.Parse(parts[2]);
                        currentMobile.Move = int.Parse(parts[3]);
                        break;
                    case 15:
                        // default mood
                        currentMobile.DefaultMood = int.Parse(parts[0]);
                        break;
                    case 16:
                        // vocab file
                        currentMobile.VocabFile = line.TrimEnd('~');
                        break;
                    case 17:
                        // script file & reset
                        currentMobile.ScriptFile = line.TrimEnd('~');
                        propertyNum = 0; // Prepare for next mobile
                        mobiles.Add(currentMobile);
                        currentMobile = null; // Clear current mobile for next iteration
                        break;
                }
                propertyNum++;
            }

            convertedArea.Mobiles = mobiles;
            return convertedArea;
        }

        public Dice ConvertStringToDice(string input) {
            var splitBonus = input.Split('+');
            var dice = splitBonus[0];
            var bonus = splitBonus[1];

            var splitDice = dice.Split('d');
            var numDice = splitDice[0];
            var numSides = splitDice[1];
            return new Dice {
                NumberOfDice = int.Parse(numDice),
                BonusNumber = int.Parse(bonus),
                SidesOfDice = int.Parse(numSides)
            };
        }

        //public void ParseObjects(List<string> buffer) { }
        //public void ParseRooms(List<string> buffer) { }
        //public void ParseResets(List<string> buffer) { }
        //public void ParseShops(List<string> buffer) { }
        //public void ParseSpecials(List<string> buffer) { }

        //public void SaveAreaToJson(string outputFilePath) {
        //    var json = JsonConvert.SerializeObject(ConvertedArea,Formatting.Indented);
        //    File.WriteAllText(outputFilePath,json);
        //}


    }
}
