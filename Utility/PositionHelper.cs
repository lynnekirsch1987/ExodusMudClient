using static ExodusMudClient.Data.Game.Enums;

namespace ExodusMudClient.Utility {
    public static class PositionHelper {
        public static readonly Dictionary<string,PositionType> PositionMappings = new Dictionary<string,PositionType>
        {
        {"dead", PositionType.Dead},
        {"mort", PositionType.Mortal},
        {"incap", PositionType.Incap},
        {"stun", PositionType.Stunned},
        {"sleep", PositionType.Sleeping},
        {"rest", PositionType.Resting},
        {"sit", PositionType.Sitting},
        {"fight", PositionType.Fighting},
        {"stand", PositionType.Standing},
        {"tether", PositionType.Tethered},
        {"mount", PositionType.Mounted},
        {"feign", PositionType.FeigningDeath},
        {"chop", PositionType.Chopping},
        {"mill", PositionType.Milling},
        {"carve", PositionType.Carving}
    };

        public static PositionType ConvertToPositionType(string nickname) {
            if (PositionMappings.TryGetValue(nickname,out PositionType positionType)) {
                return positionType;
            } else {
                throw new ArgumentException($"Unknown position nickname: {nickname}",nameof(nickname));
            }
        }
    }
}
