using System.Linq.Expressions;
using ExodusMudClient.Data.Game.Models;
using ExodusMudClient.Services.DataServices;
using ExodusMudClient.Services.StateManagement;
using ExodusMudClient.Services.ViewServices;
using ExodusMudClient.Utility;
using Microsoft.AspNetCore.Components;
using Microsoft.AspNetCore.Components.Web;
using MudBlazor;

namespace ExodusMudClient.Services.BusinessLogic;

public class HelpEditorBusinessLogic
{
    public MudTextField<string> searchTagInput;
    public MudTextField<string> relatedTagInput;
    private readonly StateContainer _state;
    private readonly DataService _data;
    public bool EditorOpen;
    public Anchor Anchor;
    public string NewSearchTag = string.Empty;
    public string NewRelatedTag = string.Empty;
    public string searchString = string.Empty;
    public Dictionary<string, object> editorConfig { get; set; }
    public string tinyMCEkey = "gmz196ednjlh8ecuzg9erjbmo8h2lz139jdalzs13amytxd0";
    public List<HelpEntry> _helpEntries = new List<HelpEntry>();
    public List<HelpEntry> _selectedHelpCategoryEntries = new List<HelpEntry>();
    public List<HelpEntry> _selectedHelpRelatedEntries = new List<HelpEntry>();
    public HelpEntry? selectedHelp = null;

    public bool HelpEntryFilter(HelpEntry element) => FilterService.Filter(element, searchString,
    (element) => string.Join(", ", element.Tags));

    public HelpEditorBusinessLogic(StateContainer state, DataService data)
    {
        _state = state;
        _data = data;

    }

    public async Task LoadHelpEntries()
    {
        // Call QueryDataAsync, passing in the appropriate lambda expression for filtering
        var helpEntries = await _data.LoadDataAsync<HelpEntry>();
        _helpEntries = helpEntries.ToList();
        _state.UpdateStateAsync();
    }


    public async Task LoadHelpEntriesForCategory()
    {
        _selectedHelpCategoryEntries = _helpEntries.Where(h => h.Category == selectedHelp.Category).ToList();
    }


    public async Task LoadHelpEntriesForRelated()
    {
        var relatedTagEntries = _helpEntries
        .Where(e => e.Tags.Intersect(selectedHelp.RelatedTags).Any())
        .ToList();
        _selectedHelpRelatedEntries = relatedTagEntries;
    }

    public async Task EditHelpFile(HelpEntry entry)
    {
        selectedHelp = entry;
        OpenEditor(Anchor.End);
        _state.UpdateStateAsync();
    }

    public async Task ViewHelp(HelpEntry entry)
    {
        selectedHelp = entry;
        await LoadHelpEntriesForCategory();
        await LoadHelpEntriesForRelated();
        _state.UpdateStateAsync();
    }

    public void RemoveTag(string tag)
    {
        selectedHelp.Tags.Remove(tag);
    }

    public void RemoveRelatedTag(string tag)
    {
        selectedHelp.RelatedTags.Remove(tag);
    }

    void OpenEditor(Anchor anchor)
    {
        EditorOpen = true;
        this.Anchor = anchor;
    }

    public void HandleSearchTagAdd(KeyboardEventArgs e)
    {
        if (e.Key == "Enter")
        {
            // add the tag to the selected help entry
            selectedHelp.Tags.Add(NewSearchTag);

            // clear the tag input
            NewSearchTag = string.Empty;
            searchTagInput.Clear();

            // update the state
            _state.UpdateStateAsync();
        }
    }

    public void HandleRelatedTagAdd(KeyboardEventArgs e)
    {
        if (e.Key == "Enter")
        {
            // add the tag to the selected help entry
            selectedHelp.RelatedTags.Add(NewRelatedTag);

            // clear the tag input
            NewRelatedTag = string.Empty;
            relatedTagInput.Clear();

            // update the state
            _state.UpdateStateAsync();
        }
    }

    public async Task SaveHelp()
    {
        selectedHelp = null;
        await _data.SaveDataAsync<HelpEntry>(_helpEntries);
        _state.UpdateStateAsync();
    }

    public async Task CloseHelp()
    {
        selectedHelp = null;
        _state.UpdateStateAsync();
    }

    public async Task RemoveHelpEntry(string id)
    {
        _helpEntries.RemoveAll(t => t.Id == id);
        await _data.SaveDataAsync<HelpEntry>(_helpEntries);
        selectedHelp = null;
        _state.UpdateStateAsync();
    }
}

