using System.Linq.Expressions;
using ExodusMudClient.Services;
using ExodusMudClient.Services.DataServices;
using MudBlazor;
namespace ExodusMudClient.Services.StateManagement;

public class StateContainer
{
    private readonly DataService _data;
    public IWebHostEnvironment _env { get; private set; }
    public event Func<Task> StateChangedAsync;
    private readonly CookieService _cookieService;
    public string[] errors = { };
    public bool IsAuthenticated { get; private set; } = false;

    public StateContainer(IWebHostEnvironment env, CookieService cookieService)
    {
        _env = env;
        _cookieService = cookieService;
    }

    public async Task AuthenticateUser()
    {
        IsAuthenticated = true;
        await UpdateStateAsync();

    }

    public async Task UpdateStateAsync()
    {
        if (StateChangedAsync != null)
        {
            foreach (Func<Task> handler in StateChangedAsync.GetInvocationList())
            {
                await handler();
            }
        }
    }

}
