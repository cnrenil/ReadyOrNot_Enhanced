#include <Windows.h>

#include "Cheats.h"
#include "Utils.h"

#include "SDK/Engine_classes.hpp"
#include "SDK/ReadyOrNot_classes.hpp"

using namespace SDK;

void Cheats::ToggleGodMode() {
	if (!GVars.PlayerController) return;
	if (!GVars.ReadyOrNotChar) return;
	auto RONC = GVars.ReadyOrNotChar;
	RONC->bGodMode = CVars.GodMode;
}

void Cheats::ToggleInfAmmo() {
	if (!GVars.PlayerController) return;
	if (!GVars.ReadyOrNotChar) return;
	auto RONC = GVars.ReadyOrNotChar;
	auto Character = (APlayerCharacter*)RONC;
	Character->GetEquippedWeapon()->bInfiniteAmmo = CVars.InfAmmo;
}

void Cheats::UpgradeWeaponStats()
{
	if (!GVars.PlayerController) return;
	if (!GVars.ReadyOrNotChar) return;
	auto* RONC = GVars.ReadyOrNotChar;
	auto* Character = (APlayerCharacter*)RONC;
	auto* Gun = Character->GetEquippedWeapon();

	// Recoil removal
	Gun->RecoilMultiplierPitch = 0.0f;
	Gun->RecoilMultiplierYaw = 0.0f;
	Gun->FirstShotRecoil = 0.0f;
	Gun->RecoilFireStrength = 0.0f;
	Gun->RecoilFireStrengthFirst = 0.0f;
	Gun->RecoilAngleStrength = 0.0f;
	Gun->RecoilRandomness = 0.0f;
	Gun->RecoilFireADSModifier = 0.0f;
	Gun->RecoilAngleADSModifier = 0.0f;
	Gun->RecoilBuildupADSModifier = 0.0f;
	Gun->RecoilHasBuildup = false;
	Gun->RecoilBuildupDampStrength = 0.0f;
	Gun->Wobble = 0.0f;
	Gun->FirstShotRecoil = 0.0f;
	Gun->FirstShotResetTime = 0.0f;
	Gun->RecoilPattern = TArray<FRotator>();
	Gun->RecoilPositionBuildup = FVector();
	Gun->RecoilRotationBuildup = FRotator();
	Gun->VelocityRecoilMultiplier = 0.0f;
	Gun->ADSRecoilMultiplier = 0.0f;
	Gun->FireCameraShake = nullptr;
	Gun->FireCameraShakeInst = nullptr;
	Gun->Reload_CameraShake = nullptr;
	Gun->ReloadEmpty_CameraShake = nullptr;
	Gun->ProcRecoil_Trans = FVector();
	Gun->ProcRecoil_Rot = FRotator();
	Gun->ProcRecoil_Trans_Buildup = FVector();
	Gun->ProcRecoil_Rot_Buildup = FRotator();
	Gun->bCalculateProcRecoil = false;
	Gun->CurrentHighTimer = 0.0f;
	Gun->FireHighTimer = 0.0f;
	Gun->RecoilFireTime = 0.0f;
	Gun->RecoilReturnRate = 0.0f;
	Gun->RecoilReturnInterpSpeed = 0.0f;
	Gun->RecoilReturnPercentage = 0.0f;
	Gun->RecoilDampStrength = 0.0f;
	Character->WeaponBobRot = FRotator();
	Character->WeaponBobTrans = FVector();
	Gun->bUseFireLoopAnims = false;
	Gun->DisableOrEnableAnimation();
	Character->CameraBobRot = FRotator();
	Character->CameraBobTrans = FVector();
	Character->MeshspaceRecoilMovementMultiplier = FVector();
	Character->RecoilSpeed = 0.0f;

	// Spread removal
	Gun->SpreadPattern = FRotator();
	Gun->PendingSpread = FRotator();
	Gun->SpreadReturnRate = 0.0f;
	Gun->FirstShotSpread = 0.0f;
	Gun->VelocitySpreadMultiplier = 0.0f;
	Gun->VelocityRecoilMultiplier = 0.0f;
	Gun->ADSSpreadMultiplier = 0.0f;
	Gun->bIgnoreAmmoTypeSpread = true;

	// Misc
	Gun->AvailableFireModes.Clear();
	Gun->AvailableFireModes.Add(EFireMode::FM_Single);
	Gun->AvailableFireModes.Add(EFireMode::FM_Auto);
	Gun->FireRate = 0.001f;
	Gun->MuzzleFlashChance = 0;
	Gun->PenetrationDistance = 10000;
	Gun->RefireDelay = 0.0f;
}

void Cheats::SetPlayerSpeed()
{
	APlayerCharacter* PlayerChar = (APlayerCharacter*)GVars.ReadyOrNotChar;

	if (PlayerChar)
	{
		PlayerChar->Server_SetWalkSpeed(240.0f * CVars.Speed, 240.0f * CVars.Speed);
	}
}

void Cheats::SilentAim()
{
	if (!CVars.SilentAim) return;

	if (GetAsyncKeyState(VK_LBUTTON) & 1)
	{
		if (!GVars.PlayerController) return;

		if (!GVars.ReadyOrNotChar) return;
		auto RONC = GVars.ReadyOrNotChar;

		AReadyOrNotCharacter* TargetActor = Utils::GetBestTarget(SilentAimSettings.AngleWeight, SilentAimSettings.MaxFOV, SilentAimSettings.TargetCivilians);

		FVector TargetLocation;
		if (TargetActor)
		{
			TargetActor->GetActorEyesViewPoint(&TargetLocation, nullptr);
		}
		else
		{
			return;
		}

		if (RONC && RONC->GetEquippedWeapon())
		{
			RONC->GetEquippedWeapon()->Server_OnFire(FRotator(), TargetLocation, 0);
			return;
		}
	}
}

void Cheats::AddMag()
{
	if (!GVars.ReadyOrNotChar) return;
	auto* Gun = GVars.ReadyOrNotChar->GetEquippedWeapon();
	if (!Gun) return;
	FMagazine NewMag;
	NewMag.Ammo = 30;
	NewMag.AmmoType = 1;
	Gun->Server_AddMagazine(NewMag);
}

void Cheats::Revive()
{
	if (!GVars.ReadyOrNotChar) return;
	((APlayerCharacter*)GVars.ReadyOrNotChar)->bForceLowReady = CVars.AlwaysAllowGuns;
	((APlayerCharacter*)GVars.ReadyOrNotChar)->SetForceLowReady(CVars.AlwaysAllowGuns);
}

void Cheats::ArrestAll(ETeam Team)
{
	if (!GVars.ReadyOrNotChar) return;
	ULevel* Level = GVars.Level;
	if (Level) {
		TArray<AActor*> ActorsCopy = Level->Actors; // snapshot to prevent mid-iteration changes causing crashes
		if (ActorsCopy)
		{
			for (AActor* Actor : ActorsCopy)
			{
				if (!Actor) continue;

				if (Team == ETeam::TEAM_CIVILIAN && Actor->IsA(ACivilianCharacter::StaticClass()) || Team == ETeam::TEAM_SUSPECT && Actor->IsA(ASuspectCharacter::StaticClass()) || Team == ETeam::TEAM_SWAT && Actor->IsA(ASWATCharacter::StaticClass()))
				{
					AReadyOrNotCharacter* Char = (AReadyOrNotCharacter*)Actor;
					if (Char->IsArrested()) continue; // Can't re-arrest already arrested civilians or it will crash
					Char->Arrest(GVars.ReadyOrNotChar);
					Char->ArrestComplete(GVars.ReadyOrNotChar, nullptr);
					GVars.ReadyOrNotChar->Server_ReportToTOC(Char, false, false);
				}
			}
		}
	}
}

void Cheats::KillAll(ETeam Team)
{
	if (!GVars.ReadyOrNotChar) return;
	ULevel* Level = GVars.Level;
	if (Level) {
		TArray<AActor*> ActorsCopy = Level->Actors; // snapshot to prevent mid-iteration changes causing crashes
		if (ActorsCopy)
		{
			for (AActor* Actor : ActorsCopy)
			{
				if (!Actor) continue;
				if (Team == ETeam::TEAM_SUSPECT && Actor->IsA(ASuspectCharacter::StaticClass()) || Team == ETeam::TEAM_CIVILIAN && Actor->IsA(ACivilianCharacter::StaticClass()) || Team == ETeam::TEAM_SWAT && Actor->IsA(ASWATCharacter::StaticClass()))
				{
					((AReadyOrNotCharacter*)Actor)->Server_Kill();
					GVars.ReadyOrNotChar->Server_ReportToTOC(Actor, false, false);
				}
			}
		}
	}
}

void Cheats::ToggleNoClip()
{
	if (!GVars.PlayerController) return;
	if (!GVars.ReadyOrNotChar) return;
	auto RONC = GVars.ReadyOrNotChar;
	auto Character = (APlayerCharacter*)RONC;
	if (CVars.NoClip)
	{
		Character->CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Character->CharacterMovement->SetMovementMode(EMovementMode::MOVE_Flying, 0);
	}
	else
	{
		Character->CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Character->CharacterMovement->SetMovementMode(EMovementMode::MOVE_Walking, 0);
	}
}

void Cheats::DrawReticle()
{
	if (!CVars.Reticle) return;
	ImVec2 ScreenSize = ImGui::GetIO().DisplaySize;
	ImVec2 WindowPos = ImGui::GetWindowPos();
	ImGui::GetBackgroundDrawList()->AddCircleFilled(
		ImVec2(ScreenSize.x / 2 + MiscSettings.ReticlePosition.x, ScreenSize.y / 2 + MiscSettings.ReticlePosition.y),
		MiscSettings.ReticleSize, 
		Utils::ConvertImVec4toU32(MiscSettings.ReticleColor));
}
