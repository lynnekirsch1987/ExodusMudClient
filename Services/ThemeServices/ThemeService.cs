using MudBlazor;

namespace ExodusMudClient.Services.ThemeServices;

public class ThemeService
{
    bool _drawerOpen = false;
    string mainBG = "rgba(6,6,6,1)";
    string darkerBox = "rgba(0, 0, 0, 1)";

    public MudTheme GetTheme()
    {
        return new MudTheme()
        {
            PaletteDark = new PaletteDark()
            {
                Primary = Colors.Blue.Lighten1,
                Secondary = Colors.Green.Accent4,
                AppbarBackground = darkerBox,
                Success = Colors.Cyan.Lighten2,
                DrawerBackground = darkerBox,
                Background = mainBG,
                DrawerText = Colors.Grey.Lighten5,
                DrawerIcon = Colors.Grey.Lighten5,
                AppbarText = Colors.Grey.Lighten5,
                TextPrimary = Colors.Grey.Lighten5,
                TextSecondary = Colors.Grey.Lighten4,
                ActionDefault = Colors.Grey.Lighten3,
                ActionDisabled = Colors.Grey.Lighten1,

                ActionDisabledBackground = Colors.Grey.Darken3,
                Divider = Colors.Grey.Darken2,
                DividerLight = Colors.Grey.Lighten2,
                TableHover = Colors.BlueGrey.Darken4,
                TableStriped = Colors.Grey.Darken2,
                OverlayDark = Colors.Grey.Darken4,
                Surface = darkerBox
            }
        };
    }
}
