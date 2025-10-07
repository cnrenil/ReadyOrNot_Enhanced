#include <Windows.h>

#include "Cheats.h"
#include "Utils.h"

#include "SDK/Engine_classes.hpp"
#include "SDK/ReadyOrNot_classes.hpp"

using namespace SDK;

FRChatMessage Msg;
TArray<AActor*> ActorsCopy;

void Cheats::ToggleGodMode() {
	if (!GVars.PlayerController) return;
	if (!GVars.ReadyOrNotChar) return;
	auto RONC = (APlayerCharacter*)GVars.ReadyOrNotChar;
	if (RONC->bGodMode != CVars.GodMode)
	{
		if (GVars.PlayerController->HasAuthority())
		{
			RONC->Server_ToggleGodMode();
		}
		else
			RONC->ToggleGodMode();
	}
}

void Cheats::ToggleInfAmmo() {
	if (!GVars.ReadyOrNotChar || !GVars.ReadyOrNotChar->GetEquippedWeapon()) return;
	ABaseMagazineWeapon* Gun = GVars.ReadyOrNotChar->GetEquippedWeapon();
	Gun->bInfiniteAmmo = CVars.InfAmmo;

}

void Cheats::UpgradeWeaponStats()
{
	if (!GVars.PlayerController) return;
	if (!GVars.ReadyOrNotChar) return;
	auto* Character = (APlayerCharacter*)GVars.ReadyOrNotChar;
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
		if (GVars.PlayerController->HasAuthority())
			PlayerChar->Server_SetWalkSpeed(240.0f * CVars.Speed, 240.0f * CVars.Speed);
		else
			PlayerChar->Client_SetWalkSpeed(240.0f * CVars.Speed, 240.0f * CVars.Speed);
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
			if (GVars.PlayerController->HasAuthority())
				RONC->GetEquippedWeapon()->Server_OnFire(FRotator(), TargetLocation, 0);
			else
				RONC->GetEquippedWeapon()->OnFire(FRotator(), TargetLocation);
			return;
		}
	}
}

void Cheats::AddMag()
{
	if (!GVars.ReadyOrNotChar || !GVars.PlayerController) return;
	auto* Gun = GVars.ReadyOrNotChar->GetEquippedWeapon();
	if (!Gun) return;
	if (!GVars.PlayerController->HasAuthority()) return; // Server_AddMagazine is server-only
	FMagazine NewMag;
	NewMag.Ammo = 30;
	NewMag.AmmoType = 1;
	Gun->Server_AddMagazine(NewMag);
}

void Cheats::ArrestAll(ETeam Team)
{
	if (!GVars.ReadyOrNotChar) return;
	ULevel* Level = GVars.Level;
	if (Level) {
		ActorsCopy = Level->Actors; // snapshot to prevent mid-iteration changes causing crashes
		if (!ActorsCopy || ActorsCopy.Num() == 0) return;
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
		ActorsCopy = Level->Actors; // snapshot to prevent mid-iteration changes causing crashes
		if (!ActorsCopy || ActorsCopy.Num() == 0) return;
		if (ActorsCopy)
		{
			for (AActor* Actor : ActorsCopy)
			{
				if (!Actor) continue;
				if (Team == ETeam::TEAM_SUSPECT && Actor->IsA(ASuspectCharacter::StaticClass()) || Team == ETeam::TEAM_CIVILIAN && Actor->IsA(ACivilianCharacter::StaticClass()) || Team == ETeam::TEAM_SWAT && Actor->IsA(ASWATCharacter::StaticClass()))
				{
					if (GVars.PlayerController->HasAuthority())
					{
						((AReadyOrNotCharacter*)Actor)->Server_Kill();
						GVars.ReadyOrNotChar->Server_ReportToTOC(Actor, false, false);
					}
					else
					{
						((AReadyOrNotCharacter*)Actor)->Kill();
						GVars.ReadyOrNotChar->Server_ReportToTOC(Actor, false, false);
					}
					
				}
			}
		}
	}
}

void Cheats::UpdateNoClip()
{
	if (!GVars.PlayerController) return;
	if (!GVars.ReadyOrNotChar) return;
	auto RONC = GVars.ReadyOrNotChar;
	auto Character = (APlayerCharacter*)RONC;
	if (CVars.NoClip)
	{
		Character->ClientCheatFly();
		Character->ClientCheatGhost();
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

	if (!GVars.ReadyOrNotChar) return;

	if (MiscSettings.ReticleWhenThrowing && !((APlayerCharacter*)GVars.ReadyOrNotChar)->bQuickThrowing) return;

	ImVec2 ScreenSize = ImGui::GetIO().DisplaySize;
	ImVec2 WindowPos = ImGui::GetWindowPos();
	ImGui::GetBackgroundDrawList()->AddCircleFilled(
		ImVec2(ScreenSize.x / 2 + MiscSettings.ReticlePosition.x, ScreenSize.y / 2 + MiscSettings.ReticlePosition.y),
		MiscSettings.ReticleSize, 
		Utils::ConvertImVec4toU32(MiscSettings.ReticleColor));
}

void Cheats::Spam()
{
	if (!CVars.Spam) return;
	if (!GVars.ReadyOrNotChar || !GVars.ReadyOrNotChar->GetEquippedWeapon() || !GVars.ReadyOrNotChar->PlayerState) return;

	// Validate the spam text before processing
	if (MiscSettings.SpamText.empty() || MiscSettings.SpamText.length() > 255) return;

	// Use stack allocation instead of heap allocation
	wchar_t ResultText[256] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, MiscSettings.SpamText.c_str(), -1, ResultText, 256);

	Msg.Message = ResultText;
	Msg.SenderName = L"The Alpha";
	Msg.bCommand = false;
	Msg.AssociatedTeam = ETeamType::TT_SQUAD;
	Msg.TargetPlayerController = GVars.PlayerController;
	Msg.TargetTeam = ETeamType::TT_SQUAD;
	Msg.UniqueNetIdStr = L"";
	Msg.bKillfeed = false;
	Msg.SenderPlayerState = (AReadyOrNotPlayerState*)GVars.ReadyOrNotChar->PlayerState;

	__try {
		if (GVars.PlayerController->HasAuthority())
		{
			// we need to send it to every player individually
			if (!GVars.Level) return;
			ActorsCopy = GVars.Level->Actors; // snapshot to prevent mid-iteration changes causing crashes
			if (!ActorsCopy || ActorsCopy.Num() == 0) return;
			for (AActor* Actor : ActorsCopy)
			{
				if (!Actor) continue;
				if (!Actor->IsA(AReadyOrNotPlayerController::StaticClass())) continue;
				((AReadyOrNotPlayerController*)GVars.PlayerController)->Server_SendChatMessage(Msg);
			}

		}
		else
		{
			// we need to send it to every player individually
			if (!GVars.Level) return;
			ActorsCopy = GVars.Level->Actors; // snapshot to prevent mid-iteration changes causing crashes
			if (!ActorsCopy || ActorsCopy.Num() == 0) return;
			for (AActor* Actor : ActorsCopy)
			{
				if (!Actor) continue;
				if (!Actor->IsA(AReadyOrNotPlayerController::StaticClass())) continue;
				((AReadyOrNotPlayerController*)GVars.PlayerController)->SendChatMessage(Msg);
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		// Log the exception but don't crash
		std::cout << "[ERROR] Exception in Troll function, skipping...\n";
		return;
	}
}

void Cheats::GetAllEvidence()
{
	if (!GVars.ReadyOrNotChar || !GVars.Level) return;

	ActorsCopy = GVars.Level->Actors;
	if (!ActorsCopy || ActorsCopy.Num() == 0) return;

	for (AActor* Actor : ActorsCopy)
	{
		if (!Actor) continue;
		if (Actor->IsA(ABaseWeapon::StaticClass()) && ((ABaseWeapon*)Actor)->EvidenceComponent->CanBeCollected())
		{
			GVars.ReadyOrNotChar->PickupEvidence(Actor);
		}
	}
	
}

void Cheats::TriggerBot()
{
	if (!CVars.TriggerBot) return;
	if (!GVars.PlayerController || !GVars.Level || !GVars.ReadyOrNotChar) return;

	FHitResult HitResult;

	if (UKismetSystemLibrary::LineTraceSingle(
		GVars.World,
		GVars.PlayerController->PlayerCameraManager->GetCameraLocation(),
		GVars.PlayerController->PlayerCameraManager->GetCameraLocation() + (GVars.PlayerController->PlayerCameraManager->GetActorForwardVector() * 10000.0f),
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::ForDuration,
		&HitResult,
		true,
		FLinearColor(255, 0, 0, 255),
		FLinearColor(255, 255, 255, 255),
		5.0f
	))
	{
		if (HitResult.bBlockingHit && HitResult.HitObjectHandle.Actor.Get())
		{
			AActor* HitActor = HitResult.HitObjectHandle.Actor.Get();
			if (HitActor && (HitActor->IsA(ASuspectCharacter::StaticClass()) || MiscSettings.TriggerBotTargetsCivilians && HitActor->IsA(ACivilianCharacter::StaticClass())))
			{
				if (((AReadyOrNotCharacter*)HitActor)->IsDeadOrUnconscious() || ((AReadyOrNotCharacter*)HitActor)->IsIncapacitated() || ((AReadyOrNotCharacter*)HitActor)->IsArrestedOrSurrendered())
					return;

				if (MiscSettings.TriggerBotUsesSilentAim && GVars.ReadyOrNotChar->GetEquippedWeapon())
				{
					GVars.ReadyOrNotChar->GetEquippedWeapon()->Server_OnFire(FRotator(), HitResult.ImpactPoint, 0);
					return;
				}
				if (GVars.ReadyOrNotChar->GetEquippedWeapon())
				{
					GVars.ReadyOrNotChar->GetEquippedWeapon()->Server_OnFire(
						GVars.PlayerController->PlayerCameraManager->GetCameraRotation(), // Direction: if we don't set this the bullet just chills
						GVars.PlayerController->PlayerCameraManager->GetCameraLocation(), // Start location: We set this to the camera location so it looks normal and isn't a silent aim
						0); // Seed: IDK what it does so we just ignore it
					return;
				}
			}
		}
	}
}

void Cheats::RenderEnabledOptions()
{
	if (!CVars.RenderOptions) return;
	float Hue = fmodf(ImGui::GetTime() * 0.2f, 1.0f); // cycles every 5s
	ImVec4 Color = ImColor::HSV(Hue, 1.f, 1.f);

	ImGui::SetNextWindowBgAlpha(0.3);
	ImGui::Begin("Enabled Options", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar);

	ImGui::SetWindowPos(ImVec2(10, 30));

	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Enabled Options:");
	if (CVars.GodMode)
		ImGui::TextColored(Color, "God Mode");
	if (CVars.InfAmmo)
		ImGui::TextColored(Color, "Infinite Ammo");
	if (CVars.Aimbot)
		ImGui::TextColored(Color, "Aimbot");
	if (CVars.ESP)
		ImGui::TextColored(Color, "ESP");
	if (CVars.SpeedEnabled)
		ImGui::TextColored(Color, "Speed x%.1f", CVars.Speed);
	if (CVars.SilentAim)
		ImGui::TextColored(Color, "Silent Aim");
	if (CVars.NoClip)
		ImGui::TextColored(Color, "No Clip");
	if (CVars.Reticle)
		ImGui::TextColored(Color, "Reticle");
	if (CVars.Spam)
		ImGui::TextColored(Color, "Troll");
	if (CVars.TriggerBot)
		ImGui::TextColored(Color, "Trigger Bot");
	ImGui::End();
}

void Cheats::Lean()
{
	if (!GVars.ReadyOrNotChar || !GVars.PlayerController) return;

	GVars.ReadyOrNotChar->QuickLeanAmount = 100000.0f;
	GVars.ReadyOrNotChar->QuickLeanIntensity = 1000000.0f;
}