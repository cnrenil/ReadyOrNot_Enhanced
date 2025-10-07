#include "Cheats.h"
#include "Utils.h"
#include <chrono>
#include <numbers>

void Cheats::Aimbot()
{
    if (!CVars.Aimbot) return;

    if (!GVars.PlayerController || !GVars.Character || !GVars.ReadyOrNotChar) return;

    ULevel* Level = GVars.Level;
    if (!Level) return;

    auto* PC = GVars.PlayerController;

    FRotator CurrentRot = GVars.PlayerController->GetControlRotation();
    FVector PlayerPos;
    GVars.PlayerController->GetPlayerViewPoint(&PlayerPos, nullptr);

	FVector Forward = ForwardFromRot(CurrentRot);

    AActor* BestTarget = nullptr;
    float BestAngle = 99999.0f;
    float BestDist = 999999.f;

    ImVec2 ScreenSize = ImGui::GetIO().DisplaySize;
    if (AimbotSettings.DrawFOV)
    {
        ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(ScreenSize.x / 2, ScreenSize.y / 2), AimbotSettings.MaxFOV, IM_COL32(255, 0, 0, 255));
    }

    for (int i = 0; i < Level->Actors.Num(); ++i)
    {
        AActor* Actor = Level->Actors[i];
        AReadyOrNotCharacter* TargetActor;
		if (!Actor) continue;

        if (Actor->IsA(ASuspectCharacter::StaticClass()) || AimbotSettings.TargetCivilians && Actor->IsA(ACivilianCharacter::StaticClass()))
        {
            TargetActor = (AReadyOrNotCharacter*)Actor;
        }
        else 
            continue;

		if (!AimbotSettings.TargetDead && TargetActor->IsDeadOrUnconscious() || !AimbotSettings.TargetDead && TargetActor->IsIncapacitated())
		{
			if (TargetActor == BestTarget)
				BestTarget = nullptr;
			continue;
		} 
		if (!AimbotSettings.TargetArrested && TargetActor->IsArrestedOrSurrendered()) 
        {
            if (TargetActor == BestTarget)
                BestTarget = nullptr;
            continue;
        }
        if (AimbotSettings.LOS && !PC->LineOfSightTo(TargetActor, PlayerPos, false)) 
        {
            if (TargetActor == BestTarget)
                BestTarget = nullptr;
            continue;
        }

        if (!TargetActor || TargetActor == GVars.Character) continue;
        
        FVector TargetPos;
        TargetActor->GetActorEyesViewPoint(&TargetPos, nullptr);

		if (TargetPos.X == 0.f && TargetPos.Y == 0.f && TargetPos.Z == 0.f) continue;


        FVector Delta = FVector {TargetPos.X - PlayerPos.X,
                                 TargetPos.Y - PlayerPos.Y,
                                 TargetPos.Z - PlayerPos.Z };

        float Dist = Length3(Delta);
        if (Dist < AimbotSettings.MinDistance) continue;
        if (Dist <= 0.0001f) continue;

		// We are aiming from the right eye so we need to offset the position a bit based on distance
        PlayerPos += (GVars.Character->GetActorRightVector() * (Dist / 1000));
		PlayerPos -= (GVars.Character->GetActorUpVector() * (Dist / 2000));

        FVector Dir = Normalize(Delta);
        float Dot = Dot3(Forward, Dir);
        float Angle = AngleDegFromDot(Dot);

        if (Angle > AimbotSettings.MaxFOV) continue;

        if (Angle < BestAngle || (fabsf(Angle - BestAngle) < 5.0f && Dist < BestDist))
        {
            BestAngle = Angle;
            BestDist = Dist;
            BestTarget = TargetActor;
        }
    }

    if (!BestTarget) return;

    // Compute desired aim rotation
    FVector AimPos = ((AReadyOrNotCharacter*)BestTarget)->Mesh->GetBoneTransform(BasicFilesImpleUtils::StringToName(L"Head"), ERelativeTransformSpace::RTS_World).Translation;

    AimPos = AimPos + (BestTarget->GetVelocity() * 0.04f); // Simple prediction

    FVector D = FVector{ AimPos.X - PlayerPos.X,
                         AimPos.Y - PlayerPos.Y,
                         AimPos.Z - PlayerPos.Z };

	float DistXY = sqrtf(D.X * D.X + D.Y * D.Y);
    if (DistXY < 0.0001f) return;

	FVector Delta = AimPos - PlayerPos;
    if (Delta.X == 0 && Delta.Y == 0 && Delta.Z == 0)
        return;

    FRotator TargetRot = Utils::VectorToRotation(Delta);
    TargetRot.Normalize();

    FRotator FinalRot = TargetRot;

    if (AimbotSettings.Smooth)
    {
        // Normalize current rotation too
        CurrentRot.Normalize();

        // Calculate the shortest rotation delta
        FRotator DeltaRot = (TargetRot - CurrentRot).GetNormalized();

        // Apply smoothing
        FinalRot = CurrentRot + (DeltaRot / AimbotSettings.SmoothingVector);

        // Normalize the final result
        FinalRot.Normalize();
    }

	FVector2D ScreenPos;

    if (AimbotSettings.DrawArrow && PC->ProjectWorldLocationToScreen(AimPos, &ScreenPos, true))
    {
        ImGui::GetBackgroundDrawList()->AddLine(ImVec2(ScreenSize.x / 2, ScreenSize.y / 2), ImVec2(ScreenPos.X, ScreenPos.Y), IM_COL32(255, 255, 255, 255), 2);
		ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(ScreenSize.x / 2, ScreenSize.y / 2), 3, IM_COL32(255, 0, 0, 255), 0);
	}

    PC->SetControlRotation(FinalRot);
}