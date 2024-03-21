using static ExodusMudClient.Data.Game.Enums;

namespace ExodusMudClient.Data.Game {
    public class _Tables {
        public static class SexTypeLookup {
            private static readonly Dictionary<SexType,string> SexTypeToString = new Dictionary<SexType,string>
            {
        { SexType.None, "none" },
        { SexType.Male, "male" },
        { SexType.Female, "female" },
        { SexType.Either, "either" }
    };

            private static readonly Dictionary<string,SexType> StringToSexType = SexTypeToString.ToDictionary(pair => pair.Value,pair => pair.Key);

            public static string ToString(SexType sexType) {
                return SexTypeToString.TryGetValue(sexType,out var result) ? result : null;
            }

            public static SexType? FromString(string str) {
                return StringToSexType.TryGetValue(str.ToLower(),out var result) ? result : (SexType?)null;
            }
        }

    }
}
