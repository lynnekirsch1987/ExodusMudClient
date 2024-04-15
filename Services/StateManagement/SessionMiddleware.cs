namespace ExodusMudClient.Services.StateManagement;

public class SessionMiddleware
{
    private readonly RequestDelegate _next;

    public SessionMiddleware(RequestDelegate next)
    {
        _next = next;
    }

    public async Task InvokeAsync(HttpContext context)
    {
        // Ensure the session is accessed early, so it's established before any response begins
        context.Session.SetString("Initialized", "True");

        // Call the next delegate/middleware in the pipeline
        await _next(context);
    }
}
public static class SessionMiddlewareExtensions
{
    public static IApplicationBuilder UseSessionMiddleware(this IApplicationBuilder builder)
    {
        return builder.UseMiddleware<SessionMiddleware>();
    }
}