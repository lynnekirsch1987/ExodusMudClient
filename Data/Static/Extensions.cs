using ExodusMudClient.Data.Game.Models;
namespace ExodusMudClient.Data.Static;

public static class Extensions
{
    public static async Task SetArea<T>(this IEnumerable<T> items, AreaFile area) where T : IAreaInterface
    {
        foreach (var item in items)
        {
            item.Area = area;
        }
    }
}
