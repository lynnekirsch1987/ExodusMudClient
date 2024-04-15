namespace ExodusMudClient.Services.DataServices
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Text.Json;
    using System.Threading.Tasks;
    using ExodusMudClient.Data.Game.Models;
    using Microsoft.AspNetCore.Hosting; // Ensure correct namespace for IWebHostEnvironment

    public class DataService
    {
        private readonly string _storagePath;
        private readonly string _areaStoragePath;
        private readonly string _objectStoragePath;
        private readonly string _mobileStoragePath;
        private readonly string _roomStoragePath;

        public DataService(IWebHostEnvironment env)
        {
            _storagePath = Path.Combine(env.ContentRootPath, "Data/Storage");
        }


        public async Task SaveArea(AreaFile area)
        {
            area.FileName = area.FileName.Replace("are", "json"); // leave that here for a bit, no harm
            string jsonData = JsonSerializer.Serialize(area, new JsonSerializerOptions { WriteIndented = true });
            await File.WriteAllTextAsync(Path.Combine(_storagePath, $"Areas/{area.FileName}"), jsonData);
        }

        public async Task SaveItemAsync<T>(T data, string name = null, string path = null)
        {
            string filename = string.IsNullOrEmpty(name) ? typeof(T).Name : name + ".json";
            string filePath = GetFilePath(Path.Combine(path, filename));
            string jsonData = JsonSerializer.Serialize(data, new JsonSerializerOptions { WriteIndented = true });
            await File.WriteAllTextAsync(filePath, jsonData);
        }

        public async Task<T> LoadItemAsync<T>(string path = null) where T : new()
        {
            string filename = (path ?? typeof(T).Name) + ".json";
            string filePath = GetFilePath(filename);
            if (!File.Exists(filePath))
            {
                return new T(); // Return an empty list if file doesn't exist
            }

            string jsonData = await File.ReadAllTextAsync(filePath);
            var data = JsonSerializer.Deserialize<T>(jsonData);
            return data ?? new T();
        }


        public async Task SaveDataAsync<T>(IEnumerable<T> data, string name = null, string path = null)
        {
            string filename = string.IsNullOrEmpty(name) ? typeof(T).Name : name + ".json";
            string filePath = GetFilePath(Path.Combine(path, filename));
            string jsonData = JsonSerializer.Serialize(data, new JsonSerializerOptions { WriteIndented = true });
            await File.WriteAllTextAsync(filePath, jsonData);
        }

        public async Task<IEnumerable<T>> LoadDataAsync<T>(string path = null) where T : new()
        {
            string filename = path ?? typeof(T).Name + ".json";
            string filePath = GetFilePath(filename);
            if (!File.Exists(filePath))
            {
                return new List<T>(); // Return an empty list if file doesn't exist
            }

            string jsonData = await File.ReadAllTextAsync(filePath);
            var data = JsonSerializer.Deserialize<IEnumerable<T>>(jsonData);
            return data ?? new List<T>();
        }

        public async Task<IEnumerable<T>> QueryDataAsync<T>(Func<T, bool> query = null) where T : new()
        {
            var items = await LoadDataAsync<T>();
            return items.Where(query).ToList();
        }

        private string GetFilePath(string filename)
        {
            return Path.Combine(_storagePath, filename);
        }
    }

}
