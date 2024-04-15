using System.Linq.Expressions;
using ExodusMudClient.Data.Game.Models;
using ExodusMudClient.Services.StateManagement;
using ExodusMudClient.Utility;
using Microsoft.AspNetCore.Components;

namespace ExodusMudClient.Services.BusinessLogic;

public class HelpFileBusinessLogic
{
    private readonly StateContainer _state;
    public List<AreaFile> Areas { get; private set; }
    private AreaFileConverter _converter { get; set; }
    public HelpFileBusinessLogic(StateContainer state, NavigationManager navManager)
    {
        _state = state;
    }
}
