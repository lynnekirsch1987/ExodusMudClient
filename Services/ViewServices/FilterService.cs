using System.Linq.Expressions;

namespace ExodusMudClient.Services.ViewServices;

public class FilterService
{

    public static bool Filter<T>(T element, string searchString, params Expression<Func<T, string>>[] properties)
    {
        if (string.IsNullOrWhiteSpace(searchString))
            return true;

        var type = typeof(T);
        foreach (var property in properties)
        {
            var propValue = property.Compile()(element);
            if (propValue != null && propValue.Contains(searchString, StringComparison.OrdinalIgnoreCase))
                return true;
        }

        return false;
    }
}
