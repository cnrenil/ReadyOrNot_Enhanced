# RON CHEAT

A simple RON cheat I made. Most injectors should work. I personally use my custom injector, but other injectors should also work.

**Important:** Make sure you're running in DX11 and not in fullscreen. You can do this by going to the game's properties and setting your selected launch option to DirectX 11. My injector automatically opens the game in DX11 so you don't need to do this if you are using it but still make sure you are not in fullscreen.

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
- Aimbot : Snap Line, Line of Sight (LOS) Checks, FOV Checks, Max/Min Distance, Smoothing, Target Bone, Targeting Options (civilians, dead, arrested, etc.)
- ESP : Team, Boxes, Bones, Traps, Distance, Bone Colors/Opacity, LOS.
- Silent Aim : Hit Chance, FOV Checks, LOS Checks, Target Bone, Snap Line, Targeting Options (civilians, dead, arrested, etc.)
- Reticle : Cross/Dot, Size, Color, Position, Only Show when throwing.
- Saving and loading of options : Auto Saving/Loading, Manual Saving/Loading
- GodMode
- Infinite Ammo
- No Recoil
- No Spread
- Add Auto Fire to any Gun
- Set Fire Rate
- Infinite Bullet Penetration
- Infinite Bullet Damage
- Speed Cheat
- Add Magazine
- Arrest All : Civilian or Suspect
- Kill All : Civilian or Suspect
- TriggerBot : LOS Checks, Silent Aim, Targeting Options (civilians, dead, arrested, etc.)
- Show Enabled Options
- Auto Win
- Unlock All Doors
- List Players : Give GodMode or Infinite Ammo, Teleport to you.
---
## Known issues:
- Changing resolutions while the cheat is injected will crash your game.
- Clicking Try Again after a mission causes an infinite loading state.
---
## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

# RON Cheat – Build Instructions

This project is written in C++ and uses ImGui and DirectX11. It is intended for educational purposes.

## Requirements
- Visual Studio 2022
- Windows SDK (latest version)
- DirectX 11
- ImGui (Included already)
- MinHook (Included already)

## Building
1. Open `ReadyOrNot.sln` in Visual Studio 2022.
2. Select `Release` build configuration.
3. Build the project.
4. The output DLL will be in the `x64/Release` folder.

## Notes
- This build is meant to be injected into the RON game while running in DX11 mode and at or past the main menu.
- Use the provided injector or your preferred injector to load the DLL.
- Press `Insert` to open the cheat menu and `End` to uninject.
- This project uses [ImGui](https://github.com/ocornut/imgui), [Dumper-7](https://github.com/Encryqed/Dumper-7), and [MinHook](https://github.com/TsudaKageyu/minhook).
