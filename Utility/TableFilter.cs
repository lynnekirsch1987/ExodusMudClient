using System.Linq.Expressions;
namespace ExodusMudClient.Utility
{
    public static class Filter
    {
        public static bool FilterFunc<T>(T element, string searchString, params Func<T, string>[] propertySelectors)
        {
            if (string.IsNullOrWhiteSpace(searchString))
                return true;

            foreach (var selector in propertySelectors)
            {
                var propValue = selector(element) ?? "";

                if (propValue.Contains(searchString, StringComparison.OrdinalIgnoreCase))
                    return true;
            }

            return false;
        }


    }
}

