namespace ExodusMudClient.Services {
    using System.Text.Json;
    using System.IO;

    public interface IDataService {
        Task SaveDataAsync<T>(T data);
        Task<T> LoadDataAsync<T>() where T : new();
    }

    public class JsonFileDataService : IDataService {
        private readonly string _storagePath;

        public JsonFileDataService(IWebHostEnvironment env) {
            _storagePath = Path.Combine(env.ContentRootPath,"Data/Storage");
            if (!Directory.Exists(_storagePath)) {
                Directory.CreateDirectory(_storagePath);
            }
        }

        public async Task SaveDataAsync<T>(T data) {
            string filename = typeof(T).Name + ".json"; // Gets the type name of T
            string filePath = GetFilePath(filename);
            string jsonData = JsonSerializer.Serialize(data,new JsonSerializerOptions { WriteIndented = true });
            await File.WriteAllTextAsync(filePath,jsonData);
        }

        public async Task<T> LoadDataAsync<T>() where T : new() {
            string filename = typeof(T).Name + ".json";
            string filePath = GetFilePath(filename);
            if (!File.Exists(filePath)) {
                return new T(); // Return a new instance if the file doesn't exist
            }

            string jsonData = await File.ReadAllTextAsync(filePath);
            T data = JsonSerializer.Deserialize<T>(jsonData) ?? new T();
            return data;
        }

        private string GetFilePath(string filename) {
            return Path.Combine(_storagePath,filename);
        }
    }

}
