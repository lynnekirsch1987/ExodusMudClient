namespace ExodusMudClient.Services.StateManagement;
using Microsoft.AspNetCore.Http;
public class CookieService
{
    private readonly IHttpContextAccessor _httpContextAccessor;

    public CookieService(IHttpContextAccessor httpContextAccessor)
    {
        _httpContextAccessor = httpContextAccessor;
    }

    public void SetSessionData(string key, string value)
    {
        var httpContext = _httpContextAccessor.HttpContext;
        if (httpContext != null)  // Always a good practice to check if HttpContext is not null
        {
            httpContext.Session.SetString(key, value);
        }
    }

    public string GetSessionData(string key)
    {
        var httpContext = _httpContextAccessor.HttpContext;
        return httpContext != null ? httpContext.Session.GetString(key) : null;
    }
}
