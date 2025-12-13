#include "Engine.h"

static const std::pair<const char*, std::string> BoneOptions[] = {
	{"Head", BoneList.HeadBone},
	{"Neck", BoneList.NeckBone},
	{"Chest", BoneList.ChestBone},
	{"Stomach", BoneList.StomachBone},
	{"Pelvis", BoneList.PelvisBone},
	{"Left Shoulder", BoneList.LeftShoulderBone},
	{"Left Elbow", BoneList.LeftElbowBone},
	{"Left Hand", BoneList.LeftHandBone},
	{"Right Shoulder", BoneList.RightShoulderBone},
	{"Right Elbow", BoneList.RightElbowBone},
	{"Right Hand", BoneList.RightHandBone}
};

static bool ShowMenu = true;
bool init = false;

int Frames = 0;

std::atomic<int> g_PresentCount{ 0 };
std::atomic<bool> Cleaning{ false };
std::atomic<bool> Resizing{ false };

static void Cleanup(HMODULE hModule);
void SaveSettings();
void LoadSettings();

static ImGuiKey TriggerBotKey = ImGuiKey_None;
static ImGuiKey ESPKey = ImGuiKey_None;
static ImGuiKey AimButton = ImGuiKey_None;

// Add WndProc hook for input handling
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
WNDPROC oWndProc = nullptr;

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (ImGui::GetCurrentContext()) {
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	}

	if (uMsg == WM_SETCURSOR) {
		if (!ShowMenu) {   // only block cursor when menu is hidden
			SetCursor(NULL);
			return TRUE;    // prevent Windows from drawing it
		}
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}



HRESULT __stdcall Engine::hkResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	Resizing.store(true);
	while (g_PresentCount.load() != 0)
		Sleep(0); 


	// Release ImGui render target
	if (Engine::pRenderTargetView) {
		Engine::pRenderTargetView->Release();
		Engine::pRenderTargetView = nullptr;
	}

	// Call original function
	HRESULT hr = Engine::oResizeBuffers(Engine::pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	// Recreate render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	if (SUCCEEDED(Engine::pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer))) {
		Engine::pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &Engine::pRenderTargetView);
		pBackBuffer->Release();
	}

	Resizing.store(false);

	return hr;
}

HRESULT __stdcall Engine::hkPresent(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags)
{
	if (Cleaning.load())  // If we are cleaning, stop drawing.
		return Engine::oPresent(SwapChain, SyncInterval, Flags);

	if (Resizing.load())
		return Engine::oPresent(SwapChain, SyncInterval, Flags);

	g_PresentCount.fetch_add(1);

	GVars.AutoSetVariables();

	if (Frames % 30 == 0) // Every 30 frames, ensure cheats are correctly applied
	{
		AReadyOrNotCharacter* RONCT = GVars.ReadyOrNotChar;
		if (GVars.PlayerController && RONCT && Utils::IsValidActor(RONCT) && RONCT->GetEquippedWeapon())
		{
			RONCT->bGodMode = CVars.GodMode;
			RONCT->GetEquippedWeapon()->bInfiniteAmmo = CVars.InfAmmo;
		}
	}

	if (!init)
	{
		if (SUCCEEDED(SwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&Engine::pDevice)))
		{
			HWND hwnd = FindWindow(L"UnrealWindow", nullptr);
			Engine::pDevice->GetImmediateContext(&Engine::pContext);

			Engine::pSwapChain = SwapChain;
			SwapChain->GetDesc(&Engine::sd);

			if (!hwnd) hwnd = GetForegroundWindow();

			Engine::InitImGui();

			// Hook WndProc for input handling
			if (hwnd) {
				oWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			}

			init = true;
		}
		if (GVars.PlayerController && GVars.PlayerController->PlayerState)
		{
			auto PlayerState = GVars.PlayerController->PlayerState;
			auto PlayerName = PlayerState->GetPlayerName().ToString();
			if (PlayerName == "PeachMarrow12" || PlayerName == "DiaperBlastrPC")
				CVars.SecretFeatures = true;
		}

		MiscSettings.SpamText.reserve(512);
	}

	if (!Engine::oPresent)
		return 0;

	if (!ImGui::GetCurrentContext())
		return Engine::oPresent(SwapChain, SyncInterval, Flags);

	if (GVars.ScreenSize.x != ImGui::GetIO().DisplaySize.x || GVars.ScreenSize.y != ImGui::GetIO().DisplaySize.y)
	{
		GVars.ScreenSize = ImGui::GetIO().DisplaySize;
	}

	// Start the ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (ShowMenu) {
		ImGui::Begin("Free Ready or Not Cheat by PeachMarrow12", nullptr, ImGuiWindowFlags_NoCollapse);


		ImGui::SeparatorText("Hello, Have Fun Cheating!");

		if  (ImGui::BeginTabBar("MainTabBar"))
		{
			if (ImGui::BeginTabItem("About"))
			{
				ImGui::Text("Free Ready or Not Cheat by PeachMarrow12");
				ImGui::Text("Version 2.0");
				ImGui::Text("Message me on Discord for support!");

				if (GVars.PlayerController && GVars.PlayerController->PlayerState)
				{
					APlayerState* PlayerState = GVars.PlayerController->PlayerState;
					std::string PlayerName = PlayerState->GetPlayerName().ToString();
					ImGui::Text("Thank you for using my cheat %s!", PlayerName.c_str());
				}
				else
				{
					ImGui::Text("Username not found but thanks anyways!");
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Player"))
			{
				if (ImGui::Checkbox("GodMode", &CVars.GodMode))
					Cheats::ToggleGodMode();
				HostOnlyTooltip();

				ImGui::Checkbox("Aimbot", &CVars.Aimbot);

				ImGui::Checkbox("Silent Aim", &CVars.SilentAim);
				AddDefaultTooltip("Not fully fleshed out so use with caution.");
				HostOnlyTooltip();

				ImGui::Checkbox("ESP", &CVars.ESP);

				ImGui::SliderFloat("Player Speed", &CVars.Speed, 1, 30, "%.1f");
				ImGui::SameLine();
				ImGui::Checkbox("Enable Speed", &CVars.SpeedEnabled);

				if (ImGui::SliderFloat("FOV", &CVars.FOV, 0.1f, 179.9f))
				{
					Cheats::ChangeFOV();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Weapon"))
			{
				if (ImGui::Checkbox("Infinite Ammo", &CVars.InfAmmo))
				{
					Cheats::ToggleInfAmmo();
				}
				HostOnlyTooltip();

				if (ImGui::Button("Upgrade Weapon"))
				{
					Cheats::UpgradeWeaponStats();
				}
				AddDefaultTooltip("Removes recoil & spread, adds auto-fire, and boosts fire rate.");

				if (ImGui::Button("Add Magazine"))
				{
					Cheats::AddMag();
				}

				ImGui::Checkbox("TriggerBot", &CVars.TriggerBot);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("World"))
			{
				if (ImGui::Button("Kill All Suspects"))
				{
					Cheats::KillAll(ETeam::TEAM_SUSPECT);
				}
				ImGui::SameLine();
				if (ImGui::Button("Kill All Civilians"))
				{
					Cheats::KillAll(ETeam::TEAM_CIVILIAN);
				}
				HostOnlyTooltip();
				if (ImGui::Button("Arrest All Suspects"))
				{
					Cheats::ArrestAll(ETeam::TEAM_SUSPECT);
				}
				ImGui::SameLine();
				if (ImGui::Button("Arrest All Civilians"))
				{
					Cheats::ArrestAll(ETeam::TEAM_CIVILIAN);
				}
				AddDefaultTooltip("This will also automatically report them.");

				if (ImGui::Button("Collect All Evidence"))
				{
					Cheats::GetAllEvidence();
				}

				if (ImGui::Button("AutoWin"))
				{
					Cheats::AutoWin();
				}

				if (ImGui::Button("Unlock All Doors"))
				{
					Cheats::UnlockDoors();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Misc"))
			{


				if (ImGui::Button("Save Settings"))
				{
					SaveSettings();
				}
				ImGui::SameLine();
				if (ImGui::Button("Load Settings"))
				{
					LoadSettings();
				}
				AddDefaultTooltip("These only save and load the configs not which cheats are enabled.");

				ImGui::Checkbox("Debug", &CVars.Debug);
				AddDefaultTooltip("This just enables options in the menu I use for finding bugs and useful information. This is most likely useless to you.");

				if (CVars.Debug)
				{
					if (ImGui::Button("Print Actors"))
					{
						Utils::PrintActors(nullptr);
					}
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Configuration"))
			{
				if (ImGui::TreeNode("Aimbot Settings"))
				{
					ImGui::SliderFloat("Aimbot FOV", &AimbotSettings.MaxFOV, 0.01f, 180.0f, "%.1f");

					ImGui::Checkbox("Should Aimbot require LOS", &AimbotSettings.LOS);
					AddDefaultTooltip("Targets must be visible; line - of - sight required.");

					ImGui::Checkbox("Target Civilians", &AimbotSettings.TargetCivilians);

					ImGui::Checkbox("Target Dead", &AimbotSettings.TargetDead);

					ImGui::Checkbox("Target Arrested", &AimbotSettings.TargetArrested);

					ImGui::Checkbox("Target All", &AimbotSettings.TargetAll);

					ImGui::SliderFloat("Max Distance", &AimbotSettings.MaxDistance, 0.0f, 300.0f, "%.1f");

					ImGui::SliderFloat("Minimum Distance", &AimbotSettings.MinDistance, 0.0f, 100.0f, "%.1f");

					ImGui::Checkbox("Smoothing", &AimbotSettings.Smooth);

					ImGui::SliderFloat("Smoothing Vector", &AimbotSettings.SmoothingVector, 1.0f, 20.0f, "%.2f");

					ImGui::Checkbox("Draw Arrow", &AimbotSettings.DrawArrow);

					ImGui::Checkbox("Draw FOV", &AimbotSettings.DrawFOV);

					if (ImGui::BeginCombo("Target Bone", AimbotSettings.TargetBone.c_str()))
					{
						for (int i = 0; i < IM_ARRAYSIZE(BoneOptions); i++)
						{
							bool is_selected = (AimbotSettings.TargetBone == BoneOptions[i].second);
							if (ImGui::Selectable(BoneOptions[i].first, is_selected))
							{
								AimbotSettings.TargetBone = BoneOptions[i].second;
							}
							if (is_selected)
								ImGui::SetItemDefaultFocus(); // make the selected item visible
						}
						ImGui::EndCombo();
					}

					ImGui::Checkbox("Require HotKey", &AimbotSettings.RequireKeyHeld);

					const char* ABpreview = ImGui::GetKeyName(AimbotSettings.AimbotKey);
					if (!ABpreview) ABpreview = "None";

					if (ImGui::BeginCombo("Select Key for Aimbot", ABpreview))
					{
						for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; ++key)
						{
							ImGuiKey current = static_cast<ImGuiKey>(key);
							const char* keyName = ImGui::GetKeyName(current);
							if (!keyName || !*keyName) continue; // skip empty names

							bool isSelected = (AimbotSettings.AimbotKey == current);
							if (ImGui::Selectable(keyName, isSelected))
								AimbotSettings.AimbotKey = current;

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
						AddDefaultTooltip("Only activate the aimbot while this key is held");
					}

					ImGui::TreePop();
				}

				if (ImGui::TreeNode("ESP Settings"))
				{
					ImGui::Checkbox("Show Team", &ESPSettings.ShowTeam);

					ImGui::Checkbox("Show Box", &ESPSettings.ShowBox);

					ImGui::Checkbox("Show Traps", &ESPSettings.ShowTraps);

					ImGui::Checkbox("Show Bones", &ESPSettings.Bones);

					ImGui::ColorEdit4("Suspect Color", (float*)&ESPSettings.SuspectColor, ImGuiColorEditFlags_NoInputs);

					ImGui::ColorEdit4("Civilian Color", (float*)&ESPSettings.CivilianColor, ImGuiColorEditFlags_NoInputs);

					ImGui::ColorEdit4("Dead Color", (float*)&ESPSettings.DeadColor, ImGuiColorEditFlags_NoInputs);

					ImGui::ColorEdit4("Team Color", (float*)&ESPSettings.TeamColor, ImGuiColorEditFlags_NoInputs);

					ImGui::ColorEdit4("Arrested Color", (float*)&ESPSettings.ArrestColor, ImGuiColorEditFlags_NoInputs);

					ImGui::Checkbox("LOS", &ESPSettings.LOS);

					if (ImGui::SliderFloat("Bone Opacity", &ESPSettings.BoneOpacity, 0.0f, 1.0f, "%.2f"))
					{
						ESPSettings.SuspectColor = ImVec4(1.0f, 0.0f, 0.0f, ESPSettings.BoneOpacity);
						ESPSettings.CivilianColor = ImVec4(0.0f, 0.0f, 1.0f, ESPSettings.BoneOpacity);
						ESPSettings.DeadColor = ImVec4(0.0f, 0.0f, 0.0f, ESPSettings.BoneOpacity);
						ESPSettings.TeamColor = ImVec4(0.0f, 1.0f, 0.0f, ESPSettings.BoneOpacity);
						ESPSettings.ArrestColor = ImVec4(1.0f, 1.0f, 0.0f, ESPSettings.BoneOpacity);
					}

					ImGui::Checkbox("Show Objectives", &ESPSettings.ShowObjectives);
					AddDefaultTooltip("The Objectives don't show the actual location");
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Silent Aim Settings"))
				{
					ImGui::Checkbox("Target Civilians", &SilentAimSettings.TargetCivilians);

					ImGui::Checkbox("Target All", &SilentAimSettings.TargetAll);

					ImGui::Checkbox("Target Dead", &SilentAimSettings.TargetDead);

					ImGui::Checkbox("Target Surrendered", &SilentAimSettings.TargetSurrendered);

					ImGui::Checkbox("Target Arrested", &SilentAimSettings.TargetArrested);

					ImGui::SliderFloat("Silent Aim FOV", &SilentAimSettings.MaxFOV, 0, 180.0f, "%.1f");

					ImGui::Checkbox("Draw FOV", &SilentAimSettings.DrawFOV);

					ImGui::SliderFloat("FOV Line Thickness", &SilentAimSettings.FOVThickness, 0.1f, 10.0f, "%.2f");

					ImGui::Checkbox("Draw Snap line", &SilentAimSettings.DrawArrow);

					ImGui::SliderFloat("Snap Line Thickness", &SilentAimSettings.ArrowThickness, 0.1f, 10.0f, "%.2f");

					ImGui::Checkbox("Require LOS", &SilentAimSettings.RequiresLOS);

					ImGui::SliderFloat("Hit Chance", &SilentAimSettings.HitChance, 0.0f, 100, "%.1f");

					if (ImGui::BeginCombo("Target Bone", SilentAimSettings.TargetBone.c_str()))
					{
						for (int i = 0; i < IM_ARRAYSIZE(BoneOptions); i++)
						{
							bool is_selected = (SilentAimSettings.TargetBone == BoneOptions[i].second);
							if (ImGui::Selectable(BoneOptions[i].first, is_selected))
							{
								SilentAimSettings.TargetBone = BoneOptions[i].second;
							}
							if (is_selected)
								ImGui::SetItemDefaultFocus(); // make the selected item visible
						}
						ImGui::EndCombo();
					}

					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Misc Settings"))
				{
					ImGui::SeparatorText("Reticle Settings");

					ImGui::Checkbox("Reticle", &CVars.Reticle);

					ImGui::ColorEdit4("Reticle Color", (float*)&MiscSettings.ReticleColor);

					ImGui::DragFloat2("Reticle Position", (float*)&MiscSettings.ReticlePosition, 1, -100, 100);

					ImGui::SliderFloat("Reticle Size", &MiscSettings.ReticleSize, 1, 15);

					ImGui::Checkbox("Use a Cross Reticle", &MiscSettings.CrossReticle);

					ImGui::Checkbox("Only Show Reticle while Throwing a Grenade", &MiscSettings.ReticleWhenThrowing);

					ImGui::SeparatorText("TriggerBot Settings");

					ImGui::Checkbox("TriggerBot Shoots Civilians", &MiscSettings.TriggerBotTargetsCivilians);

					ImGui::Checkbox("TriggerBot Uses SilentAim", &MiscSettings.TriggerBotUsesSilentAim);
					AddDefaultTooltip("Guarantees you will hit the target");

					ImGui::SeparatorText("Other");

					ImGui::Checkbox("Show Enabled Options", &CVars.RenderOptions);

					ImGui::Checkbox("Promote when Spamming", &MiscSettings.Promote);

					ImGui::Checkbox("List Players", &CVars.ListPlayers);

					ImGui::SeparatorText("KeyBinds");

					// Create a combo box
					const char* TBpreview = ImGui::GetKeyName(TriggerBotKey);
					if (!TBpreview) TBpreview = "None";

					if (ImGui::BeginCombo("Select Key for TriggerBot", TBpreview))
					{
						for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; ++key)
						{
							ImGuiKey current = static_cast<ImGuiKey>(key);
							const char* keyName = ImGui::GetKeyName(current);
							if (!keyName || !*keyName) continue; // skip empty names

							bool isSelected = (TriggerBotKey == current);
							if (ImGui::Selectable(keyName, isSelected))
								TriggerBotKey = current;

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					const char* ESPpreview = ImGui::GetKeyName(ESPKey);
					if (!ESPpreview) ESPpreview = "None";

					if (ImGui::BeginCombo("Select Key for ESP", ESPpreview))
					{
						for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; ++key)
						{
							ImGuiKey current = static_cast<ImGuiKey>(key);
							const char* keyName = ImGui::GetKeyName(current);
							if (!keyName || !*keyName) continue; // skip empty names

							bool isSelected = (ESPKey == current);
							if (ImGui::Selectable(keyName, isSelected))
								ESPKey = current;

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					const char* AimPreview = ImGui::GetKeyName(AimButton);
					if (!AimPreview) AimPreview = "None";

					if (ImGui::BeginCombo("Select AimLock button", AimPreview))
					{
						for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; ++key)
						{
							ImGuiKey current = static_cast<ImGuiKey>(key);
							const char* keyName = ImGui::GetKeyName(current);
							if (!keyName || !*keyName) continue; // skip empty names

							bool isSelected = (AimButton == current);
							if (ImGui::Selectable(keyName, isSelected))
								AimButton = current;

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					ImGui::TreePop();
				}


				ImGui::EndTabItem();
			}

			if (CVars.SecretFeatures)
			{
				if (ImGui::BeginTabItem("Secret Features"))
				{
					ImGui::Checkbox("NoClip", &CVars.NoClip);
					AddDefaultTooltip("Doesn't work currently.");
					HostOnlyTooltip();

					ImGui::Checkbox("Spam", &CVars.Spam);
					ImGui::SameLine();
					ImGui::InputText("Spam Text", &MiscSettings.SpamText);
					AddDefaultTooltip("Spams the text in chat. Has a heavy hit to performance due to me being horrible at coding.");
					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}

		ImGui::Separator();

		ImGui::Text("This cheat will most likely not be getting any more features as I have moved on but it will still get bug fixes and offset updates.");

		ImGui::Text("You can find me on Discord, UnknownCheats.me, and GitHub under Peachmarrow12 or Peachmarrow13.");

		ImGui::End();
	}

	if (AimbotSettings.AimbotKey != ImGuiKey_None)
	{
		AimbotKeyDown = ImGui::IsKeyDown(AimbotSettings.AimbotKey);
	}
	else
		AimbotKeyDown = true;

	if (CVars.RenderOptions)
		Cheats::RenderEnabledOptions();

	if (CVars.ListPlayers)
		Cheats::ListPlayers();

	if (CVars.ESP)
		Cheats::RenderESP();

	if (CVars.SilentAim)
		Cheats::SilentAim();

	if (CVars.Reticle)
		Cheats::DrawReticle();

	if (CVars.Aimbot)
		Cheats::Aimbot();

	if (CVars.NoClip)
		Cheats::UpdateNoClip();

	if (CVars.TriggerBot)
		Cheats::TriggerBot();

	if (CVars.SpeedEnabled)
		Cheats::SetPlayerSpeed();

	if (CVars.Spam)
		Cheats::Spam();

	if (Engine::pRenderTargetView) {
		Engine::pContext->OMSetRenderTargets(1, &Engine::pRenderTargetView, nullptr);

		D3D11_VIEWPORT vp = {};
		vp.Width = (float)Engine::sd.BufferDesc.Width;
		vp.Height = (float)Engine::sd.BufferDesc.Height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		Engine::pContext->RSSetViewports(1, &vp);
	}

	ImGui::Render();
	
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (TriggerBotKey != ImGuiKey_None && ImGui::IsKeyPressed(TriggerBotKey, false))
	{
		CVars.TriggerBot = !CVars.TriggerBot;
	}

	if (ESPKey != ImGuiKey_None && ImGui::IsKeyPressed(ESPKey, false))
	{
		CVars.ESP = !CVars.ESP;
	}

	g_PresentCount.fetch_sub(1);

	return Engine::oPresent ? Engine::oPresent(SwapChain, SyncInterval, Flags) : S_OK;
}

DWORD MainThread(HMODULE hModule)
{
	AllocConsole();
	FILE* Dummy;
	freopen_s(&Dummy, "CONOUT$", "w", stdout);
	freopen_s(&Dummy, "CONIN$", "r", stdin);

	std::cout << "Cheat Injecting...\n";

	int FailAmount = 0;

	while (!GVars.World and FailAmount < 8) {
		GVars.World = Utils::GetWorldSafe();
		FailAmount++;
		Sleep(100);
	}

	while (!GVars.PlayerController and FailAmount < 8) {
		GVars.PlayerController = Utils::GetPlayerController();
		FailAmount++;
		Sleep(100);
	}

	if (FailAmount >= 8) {
		std::cout << "[ERROR] Failed to get essential game pointers. Exiting...\n";
		Cleaning.store(true);
		Sleep(200);
		Cleanup(hModule);
		return 0;
	}
	Sleep(1000); // Wait a second to ensure everything is loaded

	Engine::Init();

	std::cout << "Cheat Injected\n";

	LoadSettings();

	while (!Cleaning.load())
	{
		if (GetAsyncKeyState(VK_END) & 1) // Exit with END key
		{
			std::cout << "Exiting...\n";
			Cleaning.store(true);
			break;
		}

		if (GetAsyncKeyState(VK_INSERT) & 1) // Toggle Cheat Menu with INSERT key
		{
			ShowMenu = !ShowMenu;
			std::cout << "Menu: " << (ShowMenu ? "ON" : "OFF") << "\n";
			ImGui::GetIO().MouseDrawCursor = ShowMenu;
			ShowCursor(ShowMenu);
		}

		Sleep(100);
	}

	SaveSettings();

	Cleanup(hModule);

	return 0;
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
	switch (reason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		Cleaning.store(true);
		break;
	}
	
	return TRUE;
}

void SaveSettings()
{
	if (!Settings.ShouldSave)
		return;

	std::ofstream file("settings.txt", std::ios::trunc);

	if (!file.is_open()) return;

	file.seekp(0);

	file << ESPSettings.ShowTeam << '\n';
	file << ESPSettings.ShowBox << '\n';
	file << ESPSettings.SuspectColor.x << '\n' << ESPSettings.SuspectColor.y << '\n' << ESPSettings.SuspectColor.z << '\n' << ESPSettings.SuspectColor.w << '\n';
	file << ESPSettings.CivilianColor.x << '\n' << ESPSettings.CivilianColor.y << '\n' << ESPSettings.CivilianColor.z << '\n' << ESPSettings.CivilianColor.w << '\n';
	file << ESPSettings.DeadColor.x << '\n' << ESPSettings.DeadColor.y << '\n' << ESPSettings.DeadColor.z << '\n' << ESPSettings.DeadColor.w << '\n';
	file << ESPSettings.TeamColor.x << '\n' << ESPSettings.TeamColor.y << '\n' << ESPSettings.TeamColor.z << '\n' << ESPSettings.TeamColor.w << '\n';
	file << ESPSettings.ArrestColor.x << '\n' << ESPSettings.ArrestColor.y << '\n' << ESPSettings.ArrestColor.z << '\n' << ESPSettings.ArrestColor.w << '\n';
	file << ESPSettings.ShowObjectives << '\n';
	file << AimbotSettings.MaxFOV << '\n';
	file << AimbotSettings.LOS << '\n';
	file << AimbotSettings.TargetCivilians << '\n';
	file << AimbotSettings.TargetDead << '\n';
	file << AimbotSettings.TargetArrested << '\n';
	file << AimbotSettings.MinDistance << '\n';
	file << AimbotSettings.Smooth << '\n';
	file << AimbotSettings.SmoothingVector << '\n';
	file << AimbotSettings.DrawArrow << '\n';
	file << AimbotSettings.DrawFOV << '\n';
	file << MiscSettings.Reticle << '\n';
	file << MiscSettings.ReticleColor.x << '\n' << MiscSettings.ReticleColor.y << '\n' << MiscSettings.ReticleColor.z << '\n' << MiscSettings.ReticleColor.w << '\n';
	file << MiscSettings.ReticlePosition.x << '\n' << MiscSettings.ReticlePosition.y << '\n';
	file << MiscSettings.ReticleSize << '\n';
	file << CVars.RenderOptions << '\n';
	file << MiscSettings.SpamText << '\n';
	file << MiscSettings.Promote << '\n';
	file << CVars.ListPlayers << '\n';
	file << AimbotSettings.TargetBone << '\n';
	file << AimbotSettings.RequireKeyHeld << '\n';
	file << ESPSettings.ShowTraps << '\n';
	file << ((int&)AimbotSettings.AimbotKey) << '\n';
	file << SilentAimSettings.TargetCivilians << '\n';
	file << SilentAimSettings.TargetAll << '\n';
	file << SilentAimSettings.TargetDead << '\n';
	file << SilentAimSettings.TargetSurrendered << '\n';
	file << SilentAimSettings.TargetArrested << '\n';
	file << SilentAimSettings.MaxFOV << '\n';
	file << SilentAimSettings.DrawFOV << '\n';
	file << SilentAimSettings.FOVThickness << '\n';
	file << SilentAimSettings.DrawArrow << '\n';
	file << SilentAimSettings.ArrowThickness << '\n';
	file << SilentAimSettings.RequiresLOS << '\n';
	file << SilentAimSettings.HitChance << '\n';
	file << SilentAimSettings.TargetBone << '\n';
	file << MiscSettings.TriggerBotTargetsCivilians << '\n';
	file << MiscSettings.TriggerBotUsesSilentAim << '\n';
	file << MiscSettings.CrossReticle << '\n';
	file << MiscSettings.ReticleWhenThrowing << '\n';

	file.close();
}

void LoadSettings()
{
	if (!Settings.ShouldLoad)
		return;

	std::ifstream infile("settings.txt");

	if (!infile.is_open()) return;

	infile.seekg(0);

	infile >> ESPSettings.ShowTeam;
	infile >> ESPSettings.ShowBox;
	infile >> ESPSettings.SuspectColor.x >> ESPSettings.SuspectColor.y >> ESPSettings.SuspectColor.z >> ESPSettings.SuspectColor.w;
	infile >> ESPSettings.CivilianColor.x >> ESPSettings.CivilianColor.y >> ESPSettings.CivilianColor.z >> ESPSettings.CivilianColor.w;
	infile >> ESPSettings.DeadColor.x >> ESPSettings.DeadColor.y >> ESPSettings.DeadColor.z >> ESPSettings.DeadColor.w;
	infile >> ESPSettings.TeamColor.x >> ESPSettings.TeamColor.y >> ESPSettings.TeamColor.z >> ESPSettings.TeamColor.w;
	infile >> ESPSettings.ArrestColor.x >> ESPSettings.ArrestColor.y >> ESPSettings.ArrestColor.z >> ESPSettings.ArrestColor.w;
	infile >> ESPSettings.ShowObjectives;
	infile >> AimbotSettings.MaxFOV;
	infile >> AimbotSettings.LOS;
	infile >> AimbotSettings.TargetCivilians;
	infile >> AimbotSettings.TargetDead;
	infile >> AimbotSettings.TargetArrested;
	infile >> AimbotSettings.MinDistance;
	infile >> AimbotSettings.Smooth;
	infile >> AimbotSettings.SmoothingVector;
	infile >> AimbotSettings.DrawArrow;
	infile >> AimbotSettings.DrawFOV;
	infile >> MiscSettings.Reticle;
	infile >> MiscSettings.ReticleColor.x >> MiscSettings.ReticleColor.y >> MiscSettings.ReticleColor.z >> MiscSettings.ReticleColor.w;
	infile >> MiscSettings.ReticlePosition.x >> MiscSettings.ReticlePosition.y;
	infile >> MiscSettings.ReticleSize;
	infile >> CVars.RenderOptions;
	std::getline(infile >> std::ws, MiscSettings.SpamText);
	infile >> MiscSettings.Promote;
	infile >> CVars.ListPlayers;
	infile >> AimbotSettings.TargetBone;
	infile >> AimbotSettings.RequireKeyHeld;
	infile >> ESPSettings.ShowTraps;
	infile >> ((int&)AimbotSettings.AimbotKey);
	infile >> SilentAimSettings.TargetCivilians;
	infile >> SilentAimSettings.TargetAll;
	infile >> SilentAimSettings.TargetDead;
	infile >> SilentAimSettings.TargetSurrendered;
	infile >> SilentAimSettings.TargetArrested;
	infile >> SilentAimSettings.MaxFOV;
	infile >> SilentAimSettings.DrawFOV;
	infile >> SilentAimSettings.FOVThickness;
	infile >> SilentAimSettings.DrawArrow;
	infile >> SilentAimSettings.ArrowThickness;
	infile >> SilentAimSettings.RequiresLOS;
	infile >> SilentAimSettings.HitChance;
	infile >> SilentAimSettings.TargetBone;
	infile >> MiscSettings.TriggerBotTargetsCivilians;
	infile >> MiscSettings.TriggerBotUsesSilentAim;
	infile >> MiscSettings.CrossReticle;
	infile >> MiscSettings.ReticleWhenThrowing;

	infile.close();
}

void Cleanup(HMODULE hModule)
{
	Cleaning.store(true);
	std::cout << "Cleaning up...\n";

	CVars.Aimbot = false;
	CVars.ESP = false;
	CVars.GodMode = false;
	CVars.InfAmmo = false;
	CVars.SpeedEnabled = false;
	CVars.Speed = 1;
	Cheats::ToggleGodMode();
	Cheats::ToggleInfAmmo();
	Cheats::SetPlayerSpeed();

	while (g_PresentCount.load() > 0)
		Sleep(1);  // wait until all Present calls finish

	if (ImGui::GetCurrentContext())
	{
		ImGui::GetIO().MouseDrawCursor = false;
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	HWND hwnd = FindWindow(L"UnrealWindow", nullptr);
	if (hwnd && oWndProc)
	{
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);
		oWndProc = nullptr;
	}

	if (Engine::pSwapChain)
	{
		void** vTable = *reinterpret_cast<void***>(Engine::pSwapChain);
		if (vTable && Engine::oPresent)
		{
			DWORD oldProtect;
			if (VirtualProtect(&vTable[8], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect))
			{
				vTable[8] = (void*)Engine::oPresent;
				VirtualProtect(&vTable[8], sizeof(void*), oldProtect, &oldProtect);
			}
		}
	}

	// Clean up DirectX resources
	if (Engine::pRenderTargetView) {
		Engine::pRenderTargetView->Release();
		Engine::pRenderTargetView = nullptr;
	}
	if (Engine::pContext) {
		Engine::pContext->Release();
		Engine::pContext = nullptr;
	}
	if (Engine::pDevice) {
		Engine::pDevice->Release();
		Engine::pDevice = nullptr;
	}
	if (Engine::pSwapChain) {
		Engine::pSwapChain->Release();
		Engine::pSwapChain = nullptr;
	}
	if (Engine::oPresent) {
		Engine::oPresent = nullptr;
	}

	std::cout << "Cleanup complete. Unloading DLL...\n";

	// Clean up console
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
}