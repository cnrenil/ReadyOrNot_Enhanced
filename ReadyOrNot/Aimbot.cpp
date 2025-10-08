#include "Cheats.h"
#include "Utils.h"
#include <chrono>
#include <cmath>

float MaxFOV = AimbotSettings.MaxFOV;

float VerticalFOVRadians;
float RadiusPixels;

bool Init = false;

void Cheats::Aimbot()
{
    if (!CVars.Aimbot) return;

    if (AimbotSettings.DrawFOV)
    {
        if (std::fabs(AimbotSettings.MaxFOV - MaxFOV) > 0.01f || RadiusPixels <= 0.0f)
        {
            MaxFOV = AimbotSettings.MaxFOV;
            VerticalFOVRadians = UKismetMathLibrary::DegreesToRadians(AimbotSettings.MaxFOV);
            RadiusPixels = (GVars.ScreenSize.y / 2.0f) * std::tan(VerticalFOVRadians / 2.0f);
        }

        ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(GVars.ScreenSize.x / 2, GVars.ScreenSize.y / 2), RadiusPixels, IM_COL32(255, 0, 0, 255));
    }

    if (AimbotSettings.RequireKeyHeld && !AimbotKeyDown) return;
    if (!GVars.PlayerController || !GVars.ReadyOrNotChar) return;

    if (!Init)
    {
        VerticalFOVRadians = UKismetMathLibrary::DegreesToRadians(AimbotSettings.MaxFOV);
        RadiusPixels = (ImGui::GetIO().DisplaySize.y / 2.0f) * std::tan(VerticalFOVRadians / 2.0f);

        Init = true;
    }

    ULevel* Level = GVars.Level;
    if (!Level) return;

    auto* PC = GVars.PlayerController;

    FRotator CurrentRot = PC->GetControlRotation();
    FVector CameraPos;
    FRotator CameraRot;
    PC->GetPlayerViewPoint(&CameraPos, &CameraRot);

    std::wstring WideString = UtfN::StringToWString(AimbotSettings.TargetBone);
    FName BoneName = UKismetStringLibrary::Conv_StringToName(WideString.c_str());

    // Get the current weapon
    ABaseMagazineWeapon* CurrentWeapon = nullptr;
    FVector BulletSpawnPos = CameraPos;

    if (GVars.ReadyOrNotChar && GVars.ReadyOrNotChar->GetEquippedWeapon())
    {
        if (GVars.ReadyOrNotChar->GetEquippedWeapon()->IsA(ABaseMagazineWeapon::StaticClass()))
        {
            CurrentWeapon = GVars.ReadyOrNotChar->GetEquippedWeapon();
            if (CurrentWeapon && CurrentWeapon->BulletSpawn)
            {
                BulletSpawnPos = CurrentWeapon->BulletSpawn->K2_GetComponentLocation();
            }
        }
    }

    FVector Forward = ForwardFromRot(CameraRot);

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
            TargetActor = reinterpret_cast<AReadyOrNotCharacter*>(Actor);
        }
        else
            continue;

        if (!AimbotSettings.TargetArrested && TargetActor->IsArrestedOrSurrendered())
        {
            if (TargetActor == BestTarget)
                BestTarget = nullptr;
            continue;
        }
        if (!AimbotSettings.TargetDead && TargetActor->IsDeadOrUnconscious() || !AimbotSettings.TargetDead && TargetActor->IsIncapacitated())
        {
            if (TargetActor == BestTarget)
                BestTarget = nullptr;
            continue;
        }
        if (AimbotSettings.LOS && !PC->LineOfSightTo(TargetActor, CameraPos, false))
        {
            if (TargetActor == BestTarget)
                BestTarget = nullptr;
            continue;
        }

        if (!TargetActor || TargetActor == GVars.Character) continue;

        FVector TargetPos = TargetActor->Mesh->GetBoneTransform(BoneName, ERelativeTransformSpace::RTS_World).Translation;

        if (TargetPos.X == 0.f && TargetPos.Y == 0.f && TargetPos.Z == 0.f) continue;

        FVector Delta = FVector{ TargetPos.X - CameraPos.X,
                                 TargetPos.Y - CameraPos.Y,
                                 TargetPos.Z - CameraPos.Z };

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

    FVector AimPos = reinterpret_cast<AReadyOrNotCharacter*>(BestTarget)->Mesh->GetBoneTransform(BoneName, ERelativeTransformSpace::RTS_World).Translation;
    AimPos = AimPos + (BestTarget->GetVelocity() * 0.01f); // Simple prediction

    // Calculate rotation from camera to target (simplified and stable)
    FVector Delta = AimPos - CameraPos;

    if (Delta.X == 0 && Delta.Y == 0 && Delta.Z == 0)
        return;

    FRotator TargetRot = Utils::VectorToRotation(Delta);
    TargetRot.Normalize();

    FRotator FinalRot = TargetRot;

    if (AimbotSettings.Smooth)
    {
        CurrentRot.Normalize();
        FRotator DeltaRot = (TargetRot - CurrentRot).GetNormalized();
        FinalRot = CurrentRot + (DeltaRot / AimbotSettings.SmoothingVector);
        FinalRot.Normalize();
    }

    FVector2D ScreenPos;

    if (AimbotSettings.DrawArrow && PC->ProjectWorldLocationToScreen(AimPos, &ScreenPos, true))
    {
        ImGui::GetBackgroundDrawList()->AddLine(ImVec2(GVars.ScreenSize.x / 2, GVars.ScreenSize.y / 2), ImVec2(ScreenPos.X, ScreenPos.Y), IM_COL32(255, 255, 255, 255), 2);
        ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(GVars.ScreenSize.x / 2, GVars.ScreenSize.y / 2), 2, IM_COL32(255, 0, 0, 255), 0);
    }

    PC->SetControlRotation(FinalRot);
}