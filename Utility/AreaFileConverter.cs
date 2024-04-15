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
using MudBlazor;
using ExodusMudClient.Data.Static;

namespace ExodusMudClient.Utility
{

    public enum Section
    {
        AREA = 0,
        MOBILES = 1,
        OBJECTS = 2,
        ROOMS = 3,
        RESETS = 4,
        SHOPS = 5,
        SPECIALS = 6
    }

    public interface IAreaFileConverter
    {
        AreaFile ParseAreaFile(string filePath, Section? section = null);
        List<AreaFile> GetAreaList(string path);
        string ConvertExodusColor(string input);
    }

    public class AreaFileConverter : IAreaFileConverter
    {
        public delegate AreaFile ParseHandlerDelegate(List<string> buffer, AreaFile convertedArea);
        private Dictionary<Section, ParseHandlerDelegate> parseHandlers;
        public AreaFileConverter()
        {
            parseHandlers = new Dictionary<Section, ParseHandlerDelegate> {
                { Section.AREA, ParseArea },
                { Section.MOBILES, ParseMobiles },
                { Section.ROOMS, ParseRooms },
                { Section.OBJECTS, ParseObjects },
                { Section.RESETS, ParseResets },
            };
        }

        public string ConvertExodusColor(string input)
        {

            var colorCodes = new Dictionary<string, string>
            {
                {"`a", "#807878"},
                {"`b", "#8a0000"},
                {"`c", "#005c11"},
                {"`d", "#856204"},
                {"`e", "#000e5c"},
                {"`f", "#31005c"},
                {"`g", "#0969b3"},
                {"`h", "#dee1e3"},
                {"`i", "#ed1005"},
                {"`j", "#3fed05"},
                {"`k", "#f6ff00"},
                {"`l", "#0008ff"},
                {"`m", "#0008ff"},
                {"`n", "#00e1ff"},
                {"`o", "#ffffff"},
                {"``", "#807878"},
            };

            int spansOpened = 0;

            foreach (var code in colorCodes)
            {
                int localCounter = 0; // This will count replacements for the current code.
                string pattern = Regex.Escape(code.Key); // Ensure the key is treated as a literal string in the regex.

                input = Regex.Replace(input, pattern, match =>
                {
                    localCounter++; // Increment for each match found.
                    return $"<span style='color:{code.Value}'>";
                });

                spansOpened += localCounter; // Update the global counter.
            }

            // Append the necessary </span> tags to close all opened spans.
            string closingTags = new string(' ', spansOpened).Replace(" ", "</span>");
            input += closingTags;

            return input;
        }
        public List<AreaFile> GetAreaList(string path)
        {
            var files = Directory.GetFiles(path);

            List<AreaFile> areas = new List<AreaFile>();
            foreach (var file in files)
            {
                areas.Add(ParseAreaFile(file, Section.AREA));
            }
            return areas;
        }
        public AreaFile ParseHandler(Section currentSection, List<string> buffer, AreaFile convertedArea)
        {
            if (parseHandlers.TryGetValue(currentSection, out var handler))
            {
                convertedArea = handler(buffer, convertedArea);
            }
            return convertedArea;
        }
        public AreaFile ParseAreaFile(string filePath, Section? section = null)
        {
            var currentSection = Section.AREA;
            var sectionNames = Enum.GetNames(typeof(Section));
            var buffer = new List<string>();
            bool processCurrentSection = true;
            var convertedArea = new AreaFile();
            foreach (var line in File.ReadLines(filePath))
            {
                var trimmedLine = line.Trim();
                if (trimmedLine.StartsWith("#$")) break; // end of file
                if (trimmedLine.StartsWith("#"))
                {
                    if (sectionNames.Any(name => trimmedLine.Contains(name)))
                    {
                        if (buffer.Count > 0 && processCurrentSection)
                        {
                            convertedArea = ParseHandler(currentSection, buffer, convertedArea);
                            buffer.Clear();
                        }

                        Enum.TryParse(sectionNames.First(name => trimmedLine.Contains(name)), out currentSection);

                        // Determine if the current section is the one we're interested in
                        processCurrentSection = !section.HasValue || currentSection == section.Value;
                        continue;
                    }
                }
                if (processCurrentSection)
                {
                    buffer.Add(trimmedLine);
                }
            }

            // Handle the case where the last section is the one we're interested in
            if (buffer.Count > 0 && (!section.HasValue || currentSection == section.Value))
            {
                ParseHandler(currentSection, buffer, convertedArea);
            }

            return convertedArea;
        }
        public AreaFile ParseArea(List<string> buffer, AreaFile convertedArea)
        {
            // Attempt to parse the version number.
            if (buffer.Count > 0 && int.TryParse(buffer[0].Trim().TrimEnd('~'), out int version))
            {
                convertedArea.Version = version;
            }
            else
            {
                // Handle the case where the version is not present or not an integer.
                // You could assign a default value or handle it according to your application's logic.
                convertedArea.Version = 0; // Example default value.
            }
            convertedArea.FileName = buffer[1].TrimEnd('~');
            convertedArea.AreaName = buffer[2].TrimEnd('~');
            var buffer3Line = buffer[3].Replace("{ ", "{").TrimEnd('~');
            var buffer3Parts = buffer3Line.Split(' ');
            convertedArea.LowLevel = buffer3Parts[0].Replace("{", "");
            convertedArea.HighLevel = buffer3Parts[1].Replace("}", "");
            convertedArea.Creator = buffer3Parts[2];
            convertedArea.VnumsHighLow = buffer[5].TrimEnd('~');
            return convertedArea;
        }
        private List<string> ParseLine(string line)
        {
            List<string> parts = new List<string>();

            // Regular expression to match words outside of single quotes
            Regex regex = new Regex(@"(?:[^\s']+|'[^']*')+", RegexOptions.Compiled);

            MatchCollection matches = regex.Matches(line);

            foreach (Match match in matches)
            {
                parts.Add(match.Value.Trim(' ', '\''));
            }

            return parts;
        }
        public AreaFile ParseObjects(List<string> buffer, AreaFile convertedArea)
        {
            AreaFileObject? currentObject = null;


            var typesWith12Values = new List<ItemType>
                        {
                            ItemType.ITEM_CARD,ItemType.ITEM_OBJ_TRAP,
                            ItemType.ITEM_ROOM_TRAP,ItemType.ITEM_PORTAL_TRAP,
                        };
            List<AreaFileObject> objects = new List<AreaFileObject>();
            int propertyNum = 0;
            int flagsRead = 0;
            int currentValueNum = 0;
            int index = 0;

            foreach (var line in buffer)
            {
                var parts = ParseLine(line);
                index++;

                if (propertyNum > 7 && line.StartsWith('#') && currentObject != null)
                {
                    objects.Add(currentObject);
                    currentObject = null; // Clear current object for next iteration

                }

                if (currentObject == null)
                {
                    currentObject = new AreaFileObject
                    {
                        Vnum = line.Replace("#", ""),
                        AreaFileName = convertedArea.FileName
                    };
                    propertyNum = 0; // Reset property counter for new object
                    flagsRead = 0;
                    currentValueNum = 0;
                }

                switch (propertyNum)
                {
                    case 1:
                        currentObject.Name = line.TrimEnd('~');
                        break;
                    case 2:
                        currentObject.ShortDescription = line.TrimEnd('~');
                        break;
                    case 3:
                        currentObject.LongDescription = line.TrimEnd('~');
                        break;
                    case 4:
                        currentObject.Material = line.TrimEnd('~');
                        break;
                    case 5:
                        currentObject.Timer = int.Parse(line.TrimEnd('~'));
                        break;
                    case 6:
                        currentObject.ItemType = Enums.ItemLookup(parts[0]);
                        currentObject.ExtraFlagsNumber = int.Parse(parts[1]);
                        break;
                    case 7:
                        if (flagsRead < currentObject.ExtraFlagsNumber)
                        {
                            currentObject.ExtraFlags.Add(line);
                            flagsRead++;
                            if (flagsRead < currentObject.ExtraFlagsNumber)
                            {
                                continue;
                            }

                        }
                        break;
                    case 8:
                        currentObject.WearFlags = line;
                        break;
                    case 9:
                        currentObject.ClassWearFlags = line;
                        break;
                    case 10:
                        currentObject.RaceWearFlagString = line;
                        break;
                    case 11:
                        currentObject.ClanWearFlagString = line;
                        break;
                    case 28:
                        currentObject.Rarity = int.Parse(line);
                        break;
                    case 33:
                        // first line containes v0-v4 (5 values total)
                        // v0 appears to be the same for all types
                        currentObject.Values[0] = parts[0];
                        currentObject.Values[1] = 1 < parts.Count ? parts[1] : "";
                        currentObject.Values[2] = 2 < parts.Count ? parts[2] : "";
                        currentObject.Values[3] = 3 < parts.Count ? parts[3] : "";
                        currentObject.Values[4] = 4 < parts.Count ? parts[4] : "";

                        // some times have V5 on this line for some
                        // FUCKING REASON
                        if (currentObject.ItemType == ItemType.ITEM_POTION)
                        {

                            currentObject.Values[5] = 5 < parts.Count ? parts[5] : "";
                        }
                        break;
                    case 34:
                        // all types get a value 5 from the same line
                        // set it and move on to deal with the next line

                        // if we have not set V5 yet
                        if (string.IsNullOrEmpty(currentObject.Values[5]))
                        {
                            // set v5
                            currentObject.Values[5] = line;

                            // some items have v6 = v12, check if this is one
                            if (typesWith12Values.Contains(currentObject.ItemType))
                            {
                                // if an item is of one of these types, 
                                // set our current value to 6 and proceed to
                                // next line without switching the property
                                currentValueNum = 6;
                                continue;
                            }
                        }

                        if (typesWith12Values.Contains(currentObject.ItemType))
                        {
                            // if we got this far, we have an item with all 12 values to fill.
                            if (currentValueNum <= 12)
                            {
                                currentObject.Values[currentValueNum] = line;
                                currentValueNum++;

                                if (currentValueNum == 12)
                                {
                                    // do not increment property num
                                    break;
                                }

                                continue; // do not increment property num
                            }
                        }

                        // an item with its v5 set and no other values to set
                        // will immediately move to the next line
                        break;
                    case 35:
                        // all types
                        // currentObject.Level = int.Parse(parts[0]);
                        // currentObject.Weight = int.Parse(parts[1]);
                        // currentObject.Cost = int.Parse(parts[2]);
                        // currentObject.Condition = parts[3];
                        break;
                }

                propertyNum++;
            }

            convertedArea.Objects = objects;
            return convertedArea;
        }
        public AreaFile ParseRooms(List<string> buffer, AreaFile convertedArea)
        {
            convertedArea.Rooms ??= new List<AreaFileRoom>();
            var room = new AreaFileRoom();
            room.AreaFileName = convertedArea.FileName;
            int propertyNum = 0;

            foreach (var line in buffer)
            {
                if (line.StartsWith("#0") || string.IsNullOrEmpty(line))
                {
                    // end of rooms
                    continue;
                }

                var parts = line.Split(' ');
                StringBuilder RoomDescription = new StringBuilder();
                switch (propertyNum)
                {
                    case 0:
                        room = new AreaFileRoom();
                        room.Vnum = Convert.ToInt32(line.Replace("#", ""));
                        break;
                    case 1:
                        room.Level = Convert.ToInt32(string.IsNullOrEmpty(parts[0]) ? 0 : parts[0]);
                        room.Xp = Convert.ToInt32(string.IsNullOrEmpty(parts[1]) ? 0 : parts[1]);
                        break;
                    case 2:
                        room.TpMsg = line.TrimEnd('~');
                        break;
                    case 3:
                        room.EnterMsg = line.TrimEnd('~');
                        break;
                    case 4:
                        room.ExitMsg = line.TrimEnd('~');
                        break;
                    case 5:
                        room.RaceFlags = Convert.ToInt32(string.IsNullOrEmpty(parts[0]) ? 0 : parts[0]);
                        room.ClassFlags = Convert.ToInt32(string.IsNullOrEmpty(parts[1]) ? 0 : parts[1]);
                        room.MaxLevel = Convert.ToInt32(string.IsNullOrEmpty(parts[2]) ? 0 : parts[2]);
                        break;
                    case 6:
                        room.RoomName = line.TrimEnd('~');
                        break;
                    case 7:
                        if (line.StartsWith("~"))
                        {
                            break;
                        }

                        room.RoomDescription += line;
                        continue; // move to next line without incrementing property number
                    case 8:
                        room.AreaNumber = Convert.ToInt32(string.IsNullOrEmpty(parts[0]) ? 0 : parts[0]);
                        room.RoomFlags = parts[1];
                        room.SectorType = parts[2];
                        break;
                    case 9:
                        room.MaxInRoom = Convert.ToInt32(string.IsNullOrEmpty(parts[0]) ? 0 : parts[0]);
                        break;
                    case 10:
                        room.MaxMsg = line.TrimEnd('~');
                        break;

                    default:
                        // after property # 10, we don't really know what the rest are and 
                        // that requires more logic. What we do know, is that the end of a room
                        // definition is signified by 'S' so we can break out of the loop as soon
                        // as we see that.
                        if (line.StartsWith('S') && line.EndsWith('S'))
                        {
                            convertedArea.Rooms.Add(room);
                            propertyNum = 0; // Prepare for next room
                            continue; // move to next line without incrementing property number
                        }
                        break;
                }
                propertyNum++;
            }

            return convertedArea;
        }
        public AreaFile ParseMobiles(List<string> buffer, AreaFile convertedArea)
        {
            AreaFileMobile? currentMobile = null;
            List<AreaFileMobile> mobiles = new List<AreaFileMobile>();
            int propertyNum = 0;
            bool inDescription = false;
            StringBuilder fullDescription = new StringBuilder();

            foreach (var line in buffer)
            {
                var parts = line.Split(' ');
                if (currentMobile == null)
                {
                    currentMobile = new AreaFileMobile
                    {
                        Vnum = Convert.ToInt32(line.Replace("#", ""))
                    };
                    propertyNum = 1; // Reset property counter for new mobile
                    continue;
                }

                switch (propertyNum)
                {
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
                        if (line.StartsWith("~"))
                        {
                            if (!inDescription)
                            {
                                inDescription = true;
                                continue; // move to the next line
                            }
                            else
                            {
                                // if the line starts with ~ and we are already in 
                                // the description property, then we've reached the
                                // end of it. so we exit the description property, 
                                // increase the property num to the next, and immediately
                                // begin the next loop.
                                inDescription = false;
                                currentMobile.Description = fullDescription.ToString();
                                fullDescription.Clear();
                                propertyNum++; // manual increase
                                continue; // move to the next line
                            }
                        }
                        else
                        {
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
                        if (currentMobile.BlocksExit > 50)
                        {
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
                        currentMobile.Sex = Enum.Parse<SexType>(parts[2], true);
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
        public Dice ConvertStringToDice(string input)
        {
            var splitBonus = input.Split('+');
            var dice = splitBonus[0];
            var bonus = splitBonus[1];

            var splitDice = dice.Split('d');
            var numDice = splitDice[0];
            var numSides = splitDice[1];
            return new Dice
            {
                NumberOfDice = int.Parse(numDice),
                BonusNumber = int.Parse(bonus),
                SidesOfDice = int.Parse(numSides)
            };
        }


        public AreaFile ParseResets(List<string> buffer, AreaFile convertedArea)
        {
            convertedArea.Resets ??= new List<AreaFileReset>();
            var reset = new AreaFileReset();
            reset.AreaFileName = convertedArea.FileName;

            foreach (var line in buffer)
            {
                if (line.StartsWith("S") || string.IsNullOrEmpty(line))
                {
                    // end of resets
                    continue;
                }



                // Split the line at the comment marker '*'
                var partsA = line.Replace("\t", "").Split(new char[] { '*' }, 2);  // Using 2 as the limit parameter to ensure only one split at the first '*'

                // Split the first part by any number of white spaces, trimming out empty entries
                var parts = partsA[0].Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                reset = new AreaFileReset
                {
                    virtualNumber = int.Parse(parts[2]),
                    maxInWorld = int.Parse(parts[3]),
                    resetTargetVirtualNumber = int.Parse(4 < parts.Length ? parts[4] : "0"),
                    maxInRoom = int.Parse(5 < parts.Length ? parts[5] : "0"),
                    comment = partsA[1 < partsA.Length ? 1 : 0].Trim()
                };

                switch (parts[0])
                {
                    case "M":
                        //'M': Load a mob into a room.
                        //M 0 7908   1 7927  1	* The cooper
                        // ignore 0. vnum
                        reset.resetType = ResetType.MobInRoom;
                        break;
                    case "O":
                        //'O': Place an object in a room and increment its reset count.
                        //O 0 7917   0 7957 1	* ``a steel autopsy table at The Morgue
                        reset.resetType = ResetType.MobInRoom;
                        break;
                    case "P":
                        //'P': Put an object inside another object, again incrementing the reset count.
                        //P 0 7924 -1 7923 1
                        reset.resetType = ResetType.ObjectInObject;
                        break;
                    //'G' or 'E': Equip a mob with an object or give an object to a mob directly, incrementing the object’s reset count.
                    case "G":
                        //G 0 7903 -1		* A bottle of leather polish to The stable master
                        reset.resetType = ResetType.GiveObj;
                        break;
                    case "E":
                        //E 0 7902   -1   16	* item to The stable master
                        reset.resetType = ResetType.EquipObj;
                        reset.locationOnBody = int.Parse(parts[4]);
                        break;
                    case "D":
                        //'D': Set the state of a door in a room.
                        //D 0 7949   2    1	* The Abbey Cemetery [south]
                        reset.exit = (Directions)int.Parse(parts[3]);
                        reset.doorState = int.Parse(parts[4]);
                        reset.resetType = ResetType.DoorToRoom;
                        break;
                    default:
                        break;
                }


                convertedArea.Resets.Add(reset);
            }

            return convertedArea;
        }
    }
}
