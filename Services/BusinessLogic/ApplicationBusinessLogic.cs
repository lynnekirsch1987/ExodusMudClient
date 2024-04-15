using System.Linq.Expressions;
using ExodusMudClient.Services.StateManagement;
using Microsoft.AspNetCore.Components;

namespace ExodusMudClient.Services.BusinessLogic;

public class ApplicationBusinessLogic
{
    private readonly StateContainer _state;
    private readonly NavigationManager _navManager;
    public ApplicationBusinessLogic(StateContainer state, NavigationManager navManager)
    {
        _state = state;
    }
}
