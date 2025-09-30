#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

#include <cstdio>
#include <iostream>
#include <thread>
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <chrono>
#include <filesystem>
#include <imgui_internal.h>
#include <fstream>

#include "Cheats.h"
#include "Utils.h"

#include "SDK/ReadyOrNot_classes.hpp"
#include "SDK/Engine_classes.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

static bool ESPEnabled = false;
static bool AimbotEnabled = false;
static bool AimbotLOS = true;
static float AimbotFOV = 15.0f;
static bool ShowMenu = true;
static bool Cleaning = false;
bool init = false;

int Frames = 0;

static void HookSwapChain();
static void Cleanup(HMODULE hModule);
static void InitImGui(HWND hwnd);
void SaveSettings();
void LoadSettings();

typedef HRESULT(__stdcall* tPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
tPresent oPresent = nullptr;
IDXGISwapChain* pSwapChain = nullptr;
ID3D11Device* pDevice = nullptr;
ID3D11DeviceContext* pContext = nullptr;
ID3D11RenderTargetView* pRenderTargetView = nullptr;
DXGI_SWAP_CHAIN_DESC sd = {};

static void AddDefaultTooltip(const char* Text)
{
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");

	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text(Text);
		ImGui::EndTooltip();
	}
}

void InitImGui(HWND hwnd)
{
	if (!pDevice || !pContext) {
		std::cout << "[ERROR] Device or Context is null...\n";
		Cleaning = true;
		Sleep(30);
		return;
	}

	if (!hwnd) {
		std::cout << "[ERROR] HWND is null...\n";
		Cleaning = true;
		Sleep(30);
		return;
	}

	if (Cleaning) return;

	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Platform/Renderer backends
	if (!ImGui_ImplWin32_Init(hwnd)) {
		std::cout << "[ERROR] ImGui_ImplWin32_Init failed\n";
		return;
	}
	if (!ImGui_ImplDX11_Init(pDevice, pContext)) {
		std::cout << "[ERROR] ImGui_ImplDX11_Init failed\n";
		ImGui_ImplWin32_Shutdown();
		return;
	}

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Controller Controls
	io.Fonts->AddFontDefault();
	io.MouseDrawCursor = true;  // Let ImGui draw the cursor

	ImGui::StyleColorsDark();

	if (pSwapChain) { // Create render target if we have a valid swapchain
		ID3D11Texture2D* pBackBuffer = nullptr;
		HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		if (SUCCEEDED(hr)) {
			hr = pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
			if (SUCCEEDED(hr)) {
				std::cout << "[InitImGui] Render target view created successfully\n";
			}
			else {
				std::cout << "[ERROR] Failed to create render target view: " << std::hex << hr << std::endl;
			}
			pBackBuffer->Release();
		}
		else {
			std::cout << "[ERROR] Failed to get back buffer: " << std::hex << hr << std::endl;
		}
	}

	std::cout << "[InitImGui] ImGui initialized successfully\n";
}

// Add WndProc hook for input handling
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
WNDPROC oWndProc = nullptr;

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (ImGui::GetCurrentContext()) {
		// Let ImGui handle input when menu is shown
		if (ShowMenu && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
			return true;
		}
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags)
{
	if (Cleaning) {
		return oPresent(SwapChain, SyncInterval, Flags);
	}

	GVars.AutoSetVariables();

	if (Frames % 20 == 0) // Every 20 frames, ensure cheats are correctly applied
	{
		if (GVars.ReadyOrNotChar && GVars.ReadyOrNotChar->GetEquippedWeapon())
		{
			GVars.ReadyOrNotChar->bGodMode = CVars.GodMode;
			GVars.ReadyOrNotChar->GetEquippedWeapon()->bInfiniteAmmo = CVars.InfAmmo;
		}
	}

	if (!init)
	{
		if (SUCCEEDED(SwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
		{
			HWND hwnd = FindWindow(L"UnrealWindow", nullptr);
			pDevice->GetImmediateContext(&pContext);

			pSwapChain = SwapChain;
			SwapChain->GetDesc(&sd);

			if (!hwnd) hwnd = GetForegroundWindow();

			InitImGui(hwnd);

			// Hook WndProc for input handling
			if (hwnd) {
				oWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			}

			init = true;
		}
	}

	if (!oPresent)
		return 0;

	if (!ImGui::GetCurrentContext())
		return oPresent(SwapChain, SyncInterval, Flags);

	// Start the ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (ShowMenu) {
		ImGui::Begin("Free Ready or Not Cheat by PeachMarrow12", nullptr, ImGuiWindowFlags_NoCollapse);


		ImGui::SeparatorText("Hello, Have Fun Cheating!");

		if (ImGui::TreeNode("Configuration"))
		{
			if (ImGui::TreeNode("Aimbot Settings"))
			{
				ImGui::SliderFloat("Aimbot FOV", &AimbotSettings.MaxFOV, 0.01f, 180.0f);

				ImGui::Checkbox("Should Aimbot require LOS", &AimbotSettings.LOS);
				AddDefaultTooltip("Targets must be visible; line - of - sight required.");

				ImGui::Checkbox("Target Civilians", &AimbotSettings.TargetCivilians);

				ImGui::Checkbox("Target Dead", &AimbotSettings.TargetDead);

				ImGui::Checkbox("Target Arrested", &AimbotSettings.TargetArrested);

				ImGui::SliderFloat("Minimum Distance", &AimbotSettings.MinDistance, 0.01f, 10000);

				ImGui::Checkbox("Smooting", &AimbotSettings.Smooth);

				ImGui::SliderFloat("Smoothing Vector", &AimbotSettings.SmoothingVector, 1.0f, 20.0f);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("ESP Settings"))
			{
				ImGui::Checkbox("Show Team", &ESPSettings.ShowTeam);

				ImGui::Checkbox("Show Box", &ESPSettings.ShowBox);

				ImGui::ColorEdit4("Suspect Color", (float*)&ESPSettings.SuspectColor, ImGuiColorEditFlags_NoInputs);

				ImGui::ColorEdit4("Civilian Color", (float*)&ESPSettings.CivilianColor, ImGuiColorEditFlags_NoInputs);

				ImGui::ColorEdit4("Dead Color", (float*)&ESPSettings.DeadColor, ImGuiColorEditFlags_NoInputs);

				ImGui::ColorEdit4("Team Color", (float*)&ESPSettings.TeamColor, ImGuiColorEditFlags_NoInputs);

				ImGui::ColorEdit4("Arrested Color", (float*)&ESPSettings.ArrestColor, ImGuiColorEditFlags_NoInputs);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::Checkbox("GodMode", &CVars.GodMode))
		{
			Cheats::ToggleGodMode();
		}

		ImGui::Checkbox("Aimbot", &CVars.Aimbot);

		ImGui::Checkbox("ESP", &CVars.ESP);

		if (ImGui::Checkbox("Infinite Ammo", &CVars.InfAmmo))
		{
			Cheats::ToggleInfAmmo();
		}

		if (ImGui::Button("Upgrade Weapon"))
		{
			Cheats::UpgradeWeaponStats();
		}
		AddDefaultTooltip("Removes recoil & spread, adds auto-fire, and boosts fire rate.");

		ImGui::End();
	}

	if (CVars.ESP)
		Cheats::RenderESP();

	if (pRenderTargetView) {
		pContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

		D3D11_VIEWPORT vp = {};
		vp.Width = (float)sd.BufferDesc.Width;
		vp.Height = (float)sd.BufferDesc.Height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		pContext->RSSetViewports(1, &vp);
	}

	ImGui::Render();
	
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	if (CVars.Aimbot)
		Cheats::Aimbot();

	return oPresent ? oPresent(SwapChain, SyncInterval, Flags) : S_OK;
}

// Attach hook
void HookPresent()
{
	if (!pSwapChain) {
		std::cout << "[ERROR] SwapChain is null...\n";
		Cleaning = true;
		Sleep(30);
		return;
	}

	void** vTable = *reinterpret_cast<void***>(pSwapChain);
	oPresent = (tPresent)vTable[8];

	DWORD oldProtect;
	VirtualProtect(&vTable[8], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
	vTable[8] = (void*)&hkPresent;
	VirtualProtect(&vTable[8], sizeof(void*), oldProtect, &oldProtect);
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
		Cleaning = true;
		Sleep(200);
		Cleanup(hModule);
		return 0;
	}
	Sleep(1000); // Wait a second to ensure everything is loaded

	HookSwapChain(); // Create a dummy device and swapchain to get the vtable
	HookPresent(); // Hook the Present function

	std::cout << "Cheat Injected\n";

	LoadSettings();

	while (!Cleaning)
	{
		if (GetAsyncKeyState(VK_END) & 1) // Exit with END key
		{
			std::cout << "Exiting...\n";
			Cleaning = true;
			break;
		}

		if (GetAsyncKeyState(VK_INSERT) & 1) // Toggle Cheat Menu with INSERT key
		{
			ShowMenu = !ShowMenu;
			std::cout << "Menu: " << (ShowMenu ? "ON" : "OFF") << "\n";
			ImGui::GetIO().MouseDrawCursor = ShowMenu;
			ShowCursor(false);
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
		Cleaning = true;
		break;
	}
	
	return TRUE;
}

void HookSwapChain()
{
	ZeroMemory(&sd, sizeof(DXGI_SWAP_CHAIN_DESC));
	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = FindWindow(L"UnrealWindow", nullptr); //! Fix: Add Window Name to prevent having two windows with same class causing issues
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL FeatureLevel;
	D3D_FEATURE_LEVEL FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0;

	while (!pSwapChain) {
		if (SUCCEEDED(D3D11CreateDeviceAndSwapChain(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
			&FeatureLevelsRequested, 1, D3D11_SDK_VERSION,
			&sd, &pSwapChain, &pDevice, &FeatureLevel, &pContext)))
		{
			void** vTable = *reinterpret_cast<void***>(pSwapChain);
			// Present = vTable[8]
			// ResizeBuffers = vTable[13]
		}
		else
		{
			break;
		}
	}
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
	file << AimbotSettings.MaxFOV << '\n';
	file << AimbotSettings.LOS << '\n';
	file << AimbotSettings.TargetCivilians << '\n';
	file << AimbotSettings.TargetDead << '\n';
	file << AimbotSettings.TargetArrested << '\n';
	file << AimbotSettings.MinDistance << '\n';
	file << AimbotSettings.Smooth << '\n';
	file << AimbotSettings.SmoothingVector << '\n';

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
	infile >> AimbotSettings.MaxFOV;
	infile >> AimbotSettings.LOS;
	infile >> AimbotSettings.TargetCivilians;
	infile >> AimbotSettings.TargetDead;
	infile >> AimbotSettings.TargetArrested;
	infile >> AimbotSettings.MinDistance;
	infile >> AimbotSettings.Smooth;
	infile >> AimbotSettings.SmoothingVector;

	infile.close();
}

void Cleanup(HMODULE hModule)
{
	Cleaning = true;
	std::cout << "Cleaning up...\n";

	Sleep(300); // Wait a bit to ensure no threads are using resources

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

	if (pSwapChain)
	{
		void** vTable = *reinterpret_cast<void***>(pSwapChain);
		if (vTable && oPresent)
		{
			DWORD oldProtect;
			if (VirtualProtect(&vTable[8], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect))
			{
				vTable[8] = (void*)oPresent;
				VirtualProtect(&vTable[8], sizeof(void*), oldProtect, &oldProtect);
			}
		}
	}

	// Clean up DirectX resources
	if (pRenderTargetView) {
		pRenderTargetView->Release();
		pRenderTargetView = nullptr;
	}
	if (pContext) {
		pContext->Release();
		pContext = nullptr;
	}
	if (pDevice) {
		pDevice->Release();
		pDevice = nullptr;
	}
	if (pSwapChain) {
		pSwapChain->Release();
		pSwapChain = nullptr;
	}
	if (oPresent) {
		oPresent = nullptr;
	}

	std::cout << "Cleanup complete. Unloading DLL...\n";

	// Clean up console
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
}