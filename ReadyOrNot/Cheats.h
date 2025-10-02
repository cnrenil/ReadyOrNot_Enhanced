#pragma once

#include <imgui.h>
#include <Windows.h>

#include "Utils.h"

struct EspSettingsstruct {
	bool ShowTeam = true;
	bool ShowBox = false;
	ImVec4 SuspectColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	ImVec4 CivilianColor = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
	ImVec4 DeadColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	ImVec4 TeamColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 ArrestColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

} inline ESPSettings;

struct AimbotSettingsstruct {
	float MaxFOV = 15.0f;
	bool LOS = true;
	bool TargetCivilians = false;
	bool TargetDead = false;
	bool TargetArrested = false;
	float MinDistance = 50.0f;
	bool Smooth = false;
	float SmoothingVector = 5.0f;
	bool DrawArrow = true;
} inline AimbotSettings;

struct SilentAimSettingsstruct {
	float HitChance = 100.0f;
	float MaxFOV = 15.0f;
	float AngleWeight = 1000.0f;
	bool TargetCivilians = false;
	const wchar_t* TargetBone = L"Head";
} inline SilentAimSettings;

struct CVarsstruct
{
	bool GodMode = false;
	bool InfAmmo = false;
	bool Aimbot = false;
	bool ESP = false;
	float Speed = 1;
	bool SpeedEnabled = false;
	bool SilentAim = false;
} inline CVars;

struct Settingsstruct
{
	bool ShouldSave = true;
	bool ShouldLoad = true;
} inline Settings;

struct Cheats
{
	static void ToggleGodMode();
	static void ToggleInfAmmo();
	static void Aimbot();
	static void UpgradeWeaponStats();
	static void RenderESP();
	static void SetPlayerSpeed();
	static void SilentAim();
};