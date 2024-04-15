using ExodusMudClient;
using ExodusMudClient.Components;
using ExodusMudClient.Data;
using ExodusMudClient.Services;
using ExodusMudClient.Services.BusinessLogic;
using ExodusMudClient.Services.DataServices;
using ExodusMudClient.Services.StateManagement;
using ExodusMudClient.Services.TcpServices;
using ExodusMudClient.Services.ThemeServices;
using ExodusMudClient.Services.ViewServices;
using ExodusMudClient.Utility;
using Microsoft.AspNetCore.Components.Authorization;
using Microsoft.AspNetCore.Identity;
using Microsoft.EntityFrameworkCore;
using MudBlazor.Services;

var builder = WebApplication.CreateBuilder(args);

// Add services to the container.
builder.Services.AddRazorComponents()
    .AddInteractiveServerComponents();

builder.Services.AddMudServices();
builder.Services.AddCascadingAuthenticationState();
builder.Services.AddHttpContextAccessor();

// Set up session state
builder.Services.AddDistributedMemoryCache(); // Stores session in-memory, use only for development
builder.Services.AddSession(options =>
{
    options.IdleTimeout = TimeSpan.FromMinutes(30); // Timeout for session
    options.Cookie.HttpOnly = true;
    options.Cookie.IsEssential = true; // Mark the session cookie as essential for the app
});

/** APPLICATION SERVICES **/
builder.Services.AddScoped<ITcpService, TcpService>();
builder.Services.AddScoped<IAreaFileConverter, AreaFileConverter>();
builder.Services.AddScoped<IHelpFileConverter, HelpFileConverter>();


builder.Services.AddAuthentication(options =>
{
    options.DefaultScheme = IdentityConstants.ApplicationScheme;
    options.DefaultSignInScheme = IdentityConstants.ExternalScheme;
})
    .AddIdentityCookies();


builder.Services.AddDatabaseDeveloperPageExceptionFilter();
builder.Services.AddScoped<StateContainer>();
builder.Services.AddScoped<ThemeService>();
builder.Services.AddScoped<CookieService>();
builder.Services.AddScoped<CookieService>();
builder.Services.AddScoped<FilterService>();
builder.Services.AddScoped<DataService>();

// business logic
builder.Services.AddScoped<ApplicationBusinessLogic>();
builder.Services.AddScoped<DirectoryBusinessLogic>();
builder.Services.AddScoped<AuthenticationBusinessLogic>();
builder.Services.AddScoped<HelpEditorBusinessLogic>();
builder.Services.AddScoped<HelpFileBusinessLogic>();
var app = builder.Build();

// Configure the HTTP request pipeline.
if (app.Environment.IsDevelopment())
{
    app.UseMigrationsEndPoint();
}
else
{
    app.UseExceptionHandler("/Error", createScopeForErrors: true);
    // The default HSTS value is 30 days. You may want to change this for production scenarios, see https://aka.ms/aspnetcore-hsts.
    app.UseHsts();
}

// Use session middleware
app.UseSession();
app.UseSessionMiddleware();
app.UseHttpsRedirection();

app.UseStaticFiles();
app.UseAntiforgery();

app.MapRazorComponents<App>()
    .AddInteractiveServerRenderMode();

app.Run();
