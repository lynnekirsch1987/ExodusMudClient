namespace ExodusMudClient.Data.Game {
    public delegate bool SpecFun(CharData character);
    public static class SpecialFunctions {
        public static bool SpecBard(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecBreathAny(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecBreathAcid(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecBreathFire(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecBreathFrost(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecBreathGas(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecBreathLightning(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecCastAdept(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecCastCleric(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecCastJudge(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecCastMage(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecCastHealer(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecCastUnderwater(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecCastUndead(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecExecutioner(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecFido(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecGuard(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecJanitor(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecMayor(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecPoison(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecThief(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecNasty(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecTrollMember(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecOgreMember(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecPatrolman(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecWantedChecker(CharData character) {
            // Implementation
            return true;
        }

        public static bool SpecShopkeeperTalker(CharData character) {
            // Implementation
            return true;
        }

        // Add other special function implementations here...
    }
    public static class SpecialFunctionTable {
        public static readonly Dictionary<string,SpecFun> SpecTable = new Dictionary<string,SpecFun>
        {
        {"spec_bard", SpecialFunctions.SpecBard},
        {"spec_breath_any", SpecialFunctions.SpecBreathAny},
        {"spec_breath_acid", SpecialFunctions.SpecBreathAcid},
        {"spec_breath_fire", SpecialFunctions.SpecBreathFire},
        {"spec_breath_frost", SpecialFunctions.SpecBreathFrost},
        {"spec_breath_gas", SpecialFunctions.SpecBreathGas},
        {"spec_breath_lightning", SpecialFunctions.SpecBreathLightning},
        {"spec_cast_adept", SpecialFunctions.SpecCastAdept},
        {"spec_cast_cleric", SpecialFunctions.SpecCastCleric},
        {"spec_cast_judge", SpecialFunctions.SpecCastJudge},
        {"spec_cast_mage", SpecialFunctions.SpecCastMage},
        {"spec_cast_healer", SpecialFunctions.SpecCastHealer},
        {"spec_cast_underwater", SpecialFunctions.SpecCastUnderwater},
        {"spec_cast_undead", SpecialFunctions.SpecCastUndead},
        {"spec_executioner", SpecialFunctions.SpecExecutioner},
        {"spec_fido", SpecialFunctions.SpecFido},
        {"spec_guard", SpecialFunctions.SpecGuard},
        {"spec_janitor", SpecialFunctions.SpecJanitor},
        {"spec_mayor", SpecialFunctions.SpecMayor},
        {"spec_poison", SpecialFunctions.SpecPoison},
        {"spec_thief", SpecialFunctions.SpecThief},
        {"spec_nasty", SpecialFunctions.SpecNasty},
        {"spec_troll_member", SpecialFunctions.SpecTrollMember},
        {"spec_ogre_member", SpecialFunctions.SpecOgreMember},
        {"spec_patrolman", SpecialFunctions.SpecPatrolman},
        {"spec_wanted_checker", SpecialFunctions.SpecWantedChecker},
        {"spec_shopkeeper_talker", SpecialFunctions.SpecShopkeeperTalker},
    };

        public static SpecFun SpecLookup(string name) {
            if (SpecTable.TryGetValue(name,out SpecFun func)) {
                return func;
            }
            return null;
        }

        public static string SpecString(SpecFun function) {
            foreach (var pair in SpecTable) {
                if (pair.Value == function) {
                    return pair.Key;
                }
            }
            return null;
        }
    }

    public class SpecialAssignment {
        public int Vnum { get; set; }
        public string SpecFunName { get; set; }
        public string Description { get; set; }

        public SpecialAssignment(int vnum,string specFunName,string description) {
            Vnum = vnum;
            SpecFunName = specFunName;
            Description = description;
        }
    }


    public class SpecialFunctionRegistry {
        private List<SpecialAssignment> assignments = new List<SpecialAssignment>();

        // Add a new special function assignment to the registry.
        public void AddSpecialAssignment(int vnum,string specFunName,string description) {
            assignments.Add(new SpecialAssignment(vnum,specFunName,description));
        }

        // Attempt to find a special function by name. Returns null if not found.
        public SpecFun FindSpecialFunction(string name) {
            if (SpecialFunctionTable.SpecTable.TryGetValue(name,out SpecFun specFun)) {
                return specFun;
            } else {
                return null; // Or however you wish to handle not found cases.
            }
        }

        // Optionally, provide a way to list all assignments for inspection.
        public IEnumerable<SpecialAssignment> GetAllAssignments() {
            return assignments.AsReadOnly();
        }
    }


}
