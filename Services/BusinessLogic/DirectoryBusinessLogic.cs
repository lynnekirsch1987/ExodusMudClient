using System.Diagnostics.CodeAnalysis;
using System.Linq.Expressions;
using ExodusMudClient.Data.Game.Models;
using ExodusMudClient.Data.Static;
using ExodusMudClient.Services.DataServices;
using ExodusMudClient.Services.StateManagement;
using ExodusMudClient.Services.ViewServices;
using ExodusMudClient.Utility;
using Microsoft.AspNetCore.Components;

namespace ExodusMudClient.Services.BusinessLogic;

public class DirectoryBusinessLogic
{
    private readonly StateContainer _state;
    private readonly DataService _data;
    private readonly IWebHostEnvironment _env;
    public string searchString;
    public List<AreaFile> Areas { get; private set; } = new List<AreaFile>();
    public List<AreaFileMobile> Mobiles { get; private set; } = new List<AreaFileMobile>();
    public List<AreaFileRoom> Rooms { get; private set; } = new List<AreaFileRoom>();
    public List<AreaFileObject> Objects { get; private set; } = new List<AreaFileObject>();
    public List<AreaFileReset> Resets { get; private set; } = new List<AreaFileReset>();
    private AreaFileConverter _converter { get; set; } = new AreaFileConverter();
    public DirectoryBusinessLogic(StateContainer state, DataService data, IWebHostEnvironment env)
    {
        _state = state;
        _data = data;
        _env = env;
    }

    public bool AreaListFilter(AreaFile element) => FilterService.Filter(element, searchString, (element) => element.AreaName);
    public bool MobListFilter(AreaFileMobile element) => FilterService.Filter(element, searchString, (element) => element.NameForTargeting);
    public bool ObjListFilter(AreaFileObject element) => FilterService.Filter(element, searchString, (element) => element.Name);

    public async Task LoadMobiles()
    {
        foreach (var area in Areas)
        {
            if (area.Mobiles != null && area.Mobiles.Count > 0)
            {
                foreach (var mob in area.Mobiles)
                {
                    mob.Area = area;
                    Mobiles.Add(mob);
                }
            }
        }
    }

    public async Task HydrateMobiles()
    {
        foreach (var mob in Mobiles)
        {
            // get all the rooms the mobile spawns in
            var mobRooms = Resets
            .Where(r => r.resetType == ResetType.MobInRoom)
            .Where(r => r.virtualNumber == mob.Vnum)
            .ToList();

            foreach (var reset in mobRooms)
            {
                var room = Rooms.Where(r => r.Vnum == reset.resetTargetVirtualNumber).FirstOrDefault();
                if (room != null)
                {
                    mob.SpawnRooms.Add(room);
                }
            }

            // Initialize a list to keep track of items that this mob can drop
            mob.Drops = new List<AreaFileObject>();

            // Variable to track if we are currently within the item list for the specific mob
            bool trackingItems = false;

            // Iterate over the resets to find items associated with the mob
            foreach (var reset in Resets)
            {
                if (reset.resetType == ResetType.MobInRoom && reset.virtualNumber == mob.Vnum)
                {
                    // Start tracking items when we encounter the mob spawn
                    trackingItems = true;
                }
                else if (reset.resetType == ResetType.MobInRoom)
                {
                    // Stop tracking items when a new mob spawn is encountered
                    trackingItems = false;
                }

                if (trackingItems)
                {
                    // Check if the reset is of type Equip or Give (E or G)
                    if (reset.resetType == ResetType.EquipObj || reset.resetType == ResetType.GiveObj)
                    {
                        // Assuming Reset object has a property 'itemVnum' to identify the item
                        var item = Objects
                        .Where(o => o.Vnum == reset.virtualNumber.ToString()).FirstOrDefault();

                        // the same mob might pop the same item in different rooms and those resets show
                        // up multiple times, so to make sure we only add them once, check if it already 
                        // exists.
                        var containsItem = mob.Drops?.Where(d => d.Vnum == item?.Vnum).FirstOrDefault();
                        if (item != null && containsItem == null) mob.Drops?.Add(item);
                    }
                }
            }
        }
    }

    public async Task LoadRooms()
    {
        foreach (var area in Areas)
        {
            if (area.Rooms != null && area.Rooms.Count > 0)
            {
                foreach (var room in area.Rooms)
                {
                    room.Area = area;
                    Rooms.Add(room);
                }
            }
        }
    }

    public async Task LoadObjects()
    {
        foreach (var area in Areas)
        {
            if (area.Objects != null && area.Objects.Count > 0)
            {
                foreach (var obj in area.Objects)
                {
                    obj.Area = area;
                    Objects.Add(obj);
                }
            }
        }
    }

    public async Task HydrateObjects()
    {
        foreach (var obj in Objects)
        {
            var mobsThatDropMe = Mobiles.Where(d => d.Drops.Any(h => h.Vnum == obj.Vnum)).ToList();
            obj.OnMobs.AddRange(mobsThatDropMe);
        }
    }

    public async Task LoadResets()
    {
        foreach (var area in Areas)
        {
            if (area.Resets != null && area.Resets.Count > 0)
            {
                Resets.AddRange(area.Resets);
            }
        }
    }

    // called once in routes razor, fully loads the exo db
    public async Task LoadAreas()
    {
        var areasFiles = Directory.GetFiles(Path.Combine(_env.ContentRootPath, "Data/Storage/Areas"));
        foreach (var file in areasFiles)
        {
            var area = await _data.LoadItemAsync<AreaFile>($"Areas/{Path.GetFileNameWithoutExtension(file)}");
            Areas.Add(area);
        }

        await LoadRooms();
        await LoadObjects();
        await LoadMobiles();
        await LoadResets();
        await HydrateMobiles();
        await HydrateObjects();
        await _state.UpdateStateAsync();
    }

    public async Task SaveNew()
    {
        foreach (var area in Areas)
        {
            await _data.SaveArea(area);
        }

        var test = "oh my";
    }

    // converts an area to json
    public async Task ConvertArea(string areaName)
    {
        var filePath = Path.Combine(_state._env.WebRootPath, $"AreaFiles/{areaName}");
        var convertedArea = _converter.ParseAreaFile(filePath);
        _data.SaveItemAsync<AreaFile>(convertedArea, convertedArea.FileName.Split(".")[0], "Areas");
        var test = 1;
    }
}
