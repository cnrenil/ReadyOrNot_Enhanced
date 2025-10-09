#include "Cheats.h"
#include "Utils.h"
#include <chrono>
#include <cmath>
#include <numbers>

bool Init = false;

static AActor* GetBestTarget()
{
	if (!GVars.Level || !GVars.ReadyOrNotChar || !GVars.PlayerController) return nullptr;
	std::wstring WideString = UtfN::StringToWString(AimbotSettings.TargetBone);
	FName BoneName = UKismetStringLibrary::Conv_StringToName(WideString.c_str());

	AActor* BestTarget = nullptr;
	float BestFOV = AimbotSettings.MaxFOV;

	for (AActor* Actor : GVars.Level->Actors)
	{
		if (!Actor || !Utils::IsValidActor(Actor))
			continue;
		AReadyOrNotCharacter* ReadyOrNotChar;

		if (Actor->IsA(AReadyOrNotCharacter::StaticClass()))
		{
			ReadyOrNotChar = (AReadyOrNotCharacter*)Actor;
			if (!ReadyOrNotChar->IsSuspect() || (!AimbotSettings.TargetCivilians && ReadyOrNotChar->IsCivilian()))
				continue;
		}
		else
			continue;

		if (!ReadyOrNotChar)
			continue;
		if (!AimbotSettings.TargetDead && ReadyOrNotChar->IsDeadOrUnconscious())
			continue;
		if (!AimbotSettings.TargetArrested && ReadyOrNotChar->IsArrestedOrSurrendered())
			continue;

		// Get the target bone location
		FVector BoneLocation = ReadyOrNotChar->Mesh->GetBoneTransform(BoneName, ERelativeTransformSpace::RTS_World).Translation;
		if (AimbotSettings.LOS && !GVars.ReadyOrNotChar->HasLineOfSightTo(BoneLocation))
			continue;
		FVector2D ScreenLocation;
		if (!GVars.PlayerController->ProjectWorldLocationToScreen(BoneLocation, &ScreenLocation, true))
			continue;
		FVector2D ViewportSize = Utils::ImVec2ToFVector2D(GVars.ScreenSize);
		FVector2D ViewportCenter = ViewportSize / 2.0;
		FVector2D Delta = ScreenLocation - ViewportCenter;

		// Compute length manually
		float DeltaLength = std::sqrt(Delta.X * Delta.X + Delta.Y * Delta.Y);

		// Normalize by half the screen height
		float NormalizedOffset = DeltaLength / (ViewportSize.Y * 0.5f);

		float FOV = NormalizedOffset * 90.0f;
		if (FOV < BestFOV)
		{
			BestFOV = FOV;
			BestTarget = Actor;
		}
	}
	if (BestTarget)
		return BestTarget;
	return nullptr;
}

void Cheats::Aimbot()
{
	if (!CVars.Aimbot) return;
	if (!GVars.POV || !GVars.PlayerController || !GVars.Level || !GVars.ReadyOrNotChar) return;

	if (AimbotSettings.DrawFOV)
	{
		
	}

	if (AimbotSettings.RequireKeyHeld && !AimbotKeyDown)
		return;

	std::wstring WideString = UtfN::StringToWString(AimbotSettings.TargetBone);
	FName BoneName = UKismetStringLibrary::Conv_StringToName(WideString.c_str());

	AActor* Target = GetBestTarget();
	if (!Target) return;

	FVector CameraPos = GVars.POV->Location;
	FVector TargetPos = ((AReadyOrNotCharacter*)Target)->Mesh->GetBoneTransform(BoneName, ERelativeTransformSpace::RTS_World).Translation;

	FVector AngleDiff = {
	TargetPos.X - CameraPos.X,
	TargetPos.Y - CameraPos.Y,
	TargetPos.Z - CameraPos.Z
	};

	GVars.PlayerController->ControlRotation.Yaw = (float)(atan2(AngleDiff.Y, AngleDiff.X) * 180.0f / std::numbers::pi);
	GVars.PlayerController->ControlRotation.Pitch = (float)(atan2(AngleDiff.Z, std::sqrt(std::pow(AngleDiff.X, 2) + std::pow(AngleDiff.Y, 2))) * 180.0f / std::numbers::pi);
}