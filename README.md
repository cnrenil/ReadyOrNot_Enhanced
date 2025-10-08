# RON CHEAT

A simple RON cheat I made. Most injectors should work. I personally use Xenos, but other injectors may also work.  

**Important:** Make sure you're running in DX11 and not in fullscreen. You can do this by going to the game's properties and setting your selected launch option to DirectX 11.

---

### Injection:
1. Open the game
2. Wait for the menu screen
3. Inject into ReadyOrNotSteam-Win64-Shipping.exe
4. Press insert to open or close the menu

> Also, don't grief, please.
---
## Features
- Open/close the menu with the insert key
- Aimbot - FOV, Line of Sight, Prediction, and snap line.
- ESP - Skeleton, Custom colors, and boxes.
- Godmode
- Reticle
- Infinite ammo
- Silent Aim
- Arrest all civs or suspects
- Kill all civs or suspects
- NoClip - Currently not working.
- TriggerBot.
- Secure All Evidence.
- Saving and loading of settings.
- Customizable hotkeys
- Spamming - Currently dev only to prevent griefing.
---
## Known issues:
- Changing resolutions while the cheat is injected will crash your game.
- Clicking Try Again after a mission causes an infinite loading state.
---
## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---
---

# RON Cheat – Build Instructions

This project is written in C++ and uses ImGui and DirectX11. It is intended for educational purposes.

## Requirements
- Visual Studio 2022
- Windows SDK (latest version)
- DirectX 11 libraries (usually included with the Windows SDK)
- ImGui

## Building
1. Open `ReadyOrNot.sln` in Visual Studio 2022.
2. Select `Release` build configuration.
3. Build the project.
4. The output DLL will be in the `x64/Release` folder.

## Notes
- This build is meant to be injected into the RON game while running in DX11 mode and at or past the main menu.
- Use the provided injector or your preferred injector to load the DLL.
- Press `Insert` to open the cheat menu and `End` to uninject.
- This project uses both [ImGui](https://github.com/ocornut/imgui) and [Dumper-7](https://github.com/Encryqed/Dumper-7)
