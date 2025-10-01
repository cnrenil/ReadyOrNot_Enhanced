#include "Cheats.h"
#include "Utils.h"
#include <chrono>
#include <numbers>

void Cheats::Aimbot()
{
    if (!CVars.Aimbot) return;

    if (!GVars.PlayerController || !GVars.Character) return;

    ULevel* Level = GVars.Level;
    if (!Level) return;

    auto* PC = GVars.PlayerController;

    FRotator CurrentRot;
    FVector PlayerPos;
    GVars.Character->GetActorEyesViewPoint(&PlayerPos, &CurrentRot);
    FVector Forward = ForwardFromRot(CurrentRot);

    AActor* BestTarget = nullptr;
    float BestAngle = 99999.0f;
    float BestDist = 999999.f;

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

		if (!AimbotSettings.TargetDead && TargetActor->IsDeadOrUnconscious()) continue;
		if (!AimbotSettings.TargetArrested && TargetActor->IsArrestedOrSurrendered()) continue;
        if (AimbotSettings.LOS && !PC->LineOfSightTo(TargetActor, PlayerPos, false)) continue;

        if (!TargetActor || TargetActor == GVars.Character) continue;
        
        FVector TargetPos = TargetActor->K2_GetActorLocation();

		if (TargetPos.X == 0.f && TargetPos.Y == 0.f && TargetPos.Z == 0.f) continue;

        FVector Delta = FVector {TargetPos.X - PlayerPos.X,
                                 TargetPos.Y - PlayerPos.Y,
                                 TargetPos.Z - PlayerPos.Z };

        float Dist = Length3(Delta);
        if (Dist < AimbotSettings.MinDistance) continue;
        if (Dist <= 0.0001f) continue;

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
	//BestTarget->GetActorEyesViewPoint(&AimPos, nullptr);

    FVector D = FVector{ AimPos.X - PlayerPos.X,
                         AimPos.Y - PlayerPos.Y,
                         AimPos.Z - PlayerPos.Z };

	float DistXY = sqrtf(D.X * D.X + D.Y * D.Y);
    if (DistXY < 0.0001f) return;

    FRotator TargetRot;
    TargetRot.Yaw = atan2f(D.Y, D.X) * (180.0f / std::numbers::pi);
    TargetRot.Pitch = atan2f(D.Z, DistXY) * (180.0f / std::numbers::pi);
    TargetRot.Roll = 0.f;
    ClampRotator(TargetRot);

    FRotator FinalRot = TargetRot;

    if (AimbotSettings.Smooth)
    {
        // Normalize current rotation too
        CurrentRot.Normalize();

        // Calculate the shortest rotation delta using UE5's method
        FRotator DeltaRot = (TargetRot - CurrentRot).GetNormalized();

        // Apply smoothing
        FinalRot = CurrentRot + (DeltaRot / AimbotSettings.SmoothingVector);

        // Normalize the final result
        FinalRot.Normalize();
    }

    PC->ControlRotation = FinalRot;
}