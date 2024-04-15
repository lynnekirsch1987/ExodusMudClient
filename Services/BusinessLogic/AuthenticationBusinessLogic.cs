using System.Linq.Expressions;
using ExodusMudClient.Components.Pages.Staff;
using ExodusMudClient.Data.Game.Models;
using ExodusMudClient.Services.StateManagement;
using ExodusMudClient.Utility;
using Microsoft.AspNetCore.Components;
namespace ExodusMudClient.Services.BusinessLogic;

public class AuthenticationBusinessLogic
{

    private readonly StateContainer _state;
    private readonly CookieService _cookieService;
    public string username { get; set; }
    public string password { get; set; }
    public List<string> errors { get; set; } = new List<string>();
    public bool Failed = false;

    public AuthenticationBusinessLogic(StateContainer state, CookieService cookieService)
    {
        _state = state;
        _cookieService = cookieService;
    }

    public bool IsAuthenticated()
    {
        return _state.IsAuthenticated;
    }

    public async Task<bool> Authenticate()
    {
        if (string.IsNullOrEmpty(username) || string.IsNullOrEmpty(password))
        {
            errors.Add("username & password can't be null");
            return false;
        }
        var userInfo = UserInfo();
        if (userInfo.ContainsKey(username))
        {
            if (String.Equals(userInfo[username], password, StringComparison.Ordinal))
            {
                await _state.AuthenticateUser();
                return true;
            }
            else
            {
                errors.Add("couldn't validate password");
                return false;
            }
        }
        else
        {
            errors.Add("username not found");
            return false;
        }

        errors.Add("unexpected end");
        return false;
    }

    private Dictionary<string, string> UserInfo()
    {
        return new Dictionary<string, string>
        {
            {"galadriel", "butterycheese"},
            {"sallana", "colorfulrain"},
        };
    }
}
