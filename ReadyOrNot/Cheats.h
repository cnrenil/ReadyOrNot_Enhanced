#pragma once

#include <imgui.h>
#include "Utils.h"
#include <string>

inline bool AimbotKeyDown = false;

struct BoneListStruct
{
	std::string HeadBone = "Head";
	std::string NeckBone = "neck_1";
	std::string ChestBone = "spine_2";
	std::string StomachBone = "spine_1";
	std::string PelvisBone = "pelvis";
	std::string LeftShoulderBone = "clavicle_LE";
	std::string LeftElbowBone = "lowerarm_LE";
	std::string LeftHandBone = "hand_LE";
	std::string RightShoulderBone = "clavicle_RI";
	std::string RightElbowBone = "lowerarm_RI";
	std::string RightHandBone = "hand_RI";
} inline BoneList;

struct EspSettingsstruct {
	bool ShowTeam = true;
	bool ShowBox = false;
	bool ShowTraps = true;
	bool ShowObjectives = false;
	float BoneOpacity = 1.0f;
	ImVec4 SuspectColor = ImVec4(1.0f, 0.0f, 0.0f, BoneOpacity);
	ImVec4 CivilianColor = ImVec4(0.0f, 0.0f, 1.0f, BoneOpacity);
	ImVec4 DeadColor = ImVec4(0.0f, 0.0f, 0.0f, BoneOpacity);
	ImVec4 TeamColor = ImVec4(0.0f, 1.0f, 0.0f, BoneOpacity);
	ImVec4 ArrestColor = ImVec4(1.0f, 1.0f, 0.0f, BoneOpacity);

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
	bool DrawArrow = false;
	bool DrawFOV = false;
	std::string TargetBone = BoneList.HeadBone;
	bool RequireKeyHeld = false;
	ImGuiKey AimbotKey = ImGuiKey_None;
} inline AimbotSettings;

struct SilentAimSettingsstruct {
	float HitChance = 100.0f;
	float MaxFOV = 15.0f;
	float AngleWeight = 1000.0f;
	bool TargetCivilians = false;
} inline SilentAimSettings;

struct CVarsstruct
{
	bool SecretFeatures = false;
	bool GodMode = false;
	bool InfAmmo = false;
	bool Aimbot = false;
	bool ESP = false;
	float Speed = 1;
	bool SpeedEnabled = false;
	bool SilentAim = false;
	bool NoClip = false;
	bool Reticle = false;
	bool Spam = false;
	bool TriggerBot = false;
	bool RenderOptions = false;
	float FOV = 120.0f;
	bool ListPlayers = false;
} inline CVars;

struct MiscSettingsStruct {
	bool Reticle = false;
	ImVec4 ReticleColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	float ReticleSize = 5.0f;
	ImVec2 ReticlePosition = ImVec2(0.0f, 0.0f);
	bool ReticleWhenThrowing = false;
	bool TriggerBotTargetsCivilians = false;
	bool TriggerBotUsesSilentAim = false;
	std::string SpamText = "Created by peachmarrow13 check me out on UnknownCheats.me!";
	bool Promote = true;
} inline MiscSettings;

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
	static void AddMag();
	static void ArrestAll(ETeam Team); // Arrest all of a specific team
	static void KillAll(ETeam Team);   // Kill all of a specific team
	static void UpdateNoClip();
	static void DrawReticle();
	static void Spam();
	static void GetAllEvidence();
	static void TriggerBot();
	static void RenderEnabledOptions();
	static void ChangeFOV();
	static void Lean();
	static void AutoWin();
	static void UnlockDoors();
	static void ListPlayers();
	static void ForceReady();
};