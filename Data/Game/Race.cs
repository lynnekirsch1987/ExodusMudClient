using static ExodusMudClient.Data.Game.Enums;

namespace ExodusMudClient.Data.Game {
    public class RaceType {
        public string Name { get; set; }
        public bool PcRace { get; set; }
        public ActFlags ActBits { get; set; }
        public AffectedByFlags AffByBits { get; set; }
        public int OffBits { get; set; }
        public int Imm { get; set; }
        public int Res { get; set; }
        public int Vuln { get; set; }
        public int Form { get; set; }
        public int Parts { get; set; }

        public RaceType(string name,bool pcRace,ActFlags actBits,AffectedByFlags affByBits,int offBits,int imm,int res,int vuln,int form,int parts) {
            Name = name;
            PcRace = pcRace;
            ActBits = actBits;
            AffByBits = affByBits;
            OffBits = offBits;
            Imm = imm;
            Res = res;
            Vuln = vuln;
            Form = form;
            Parts = parts;
        }
    }

    public static class RaceTable {
        public static List<RaceType> Races = new List<RaceType>
        {
            new RaceType(/*name*/"unique", /*pc race?*/false, /*actBits*/0, /*affByBits*/0, /*offBits*/0, 
                /*imm*/0, /*res*/0, /*vuln*/0, /*form*/0, /*parts*/0)
        };
    }
}
