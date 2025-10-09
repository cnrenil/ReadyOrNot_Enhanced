#include "Utils.h"

#include <imgui.h>
#include <numbers>
#include "SDK/Engine_classes.hpp"

using namespace SDK;

// can return nullptr
UWorld* Utils::GetWorldSafe() 
{
    UWorld* World = nullptr;
    int i = 0;
    while (i < 50) {
        i++;
        UEngine* Engine = UEngine::GetEngine();
        if (!Engine) {
            printf("[Error] Engine not found!\n");
            continue;
        }

        UGameViewportClient* Viewport = Engine->GameViewport;
        if (!Viewport) {
            printf("[Error] GameViewport not found!\n");
            continue;
        }

        World = Viewport->World;
        if (!World) {
            printf("[Error] World not found!\n");
            continue;
        }
		break; // Successfully obtained World
    }
    return World;
}

// can return nullptr
APlayerController* Utils::GetPlayerController()
{
    int i = 0;
	APlayerController* PlayerController = nullptr;

    while (i < 50) {
        i++;
        UWorld* World = GetWorldSafe();
        if (!World) return nullptr; // Error already logged in GetWorldSafe
        UGameInstance* GameInstance = World->OwningGameInstance;
        if (!GameInstance) {
            printf("[Error] GameInstance not found!\n");
        }
        if (GameInstance->LocalPlayers.Num() <= 0) {
            printf("[Error] No LocalPlayers in GameInstance!\n");
        }
        ULocalPlayer* LocalPlayer = GameInstance->LocalPlayers[0];
        if (!LocalPlayer) {
            printf("[Error] LocalPlayer is null!\n");
        }
        PlayerController = LocalPlayer->PlayerController;
        if (!PlayerController) {
            printf("[Error] PlayerController not found!\n");
        }
    }
    if (!Utils::IsValidActor(PlayerController)) return nullptr;
    return PlayerController;
}

unsigned Utils::ConvertImVec4toU32(ImVec4 Color)
{
    return IM_COL32((int)(Color.x * 255.0f), (int)(Color.y * 255.0f), (int)(Color.z * 255.0f), (int)(Color.w * 255.0f));
}

void Utils::PrintActors(const char* Exclude)
{
	ULevel* Level = GVars.Level;
	if (Level)
	{
        TArray<AActor*> Actors = Level->Actors;
        for (int i = 0; i < Actors.Num(); i++)
        {
            AActor* Actor = Actors[i];
            if (Actor)
            {
                if (!Utils::IsValidActor(Actor)) continue;
                if (Exclude && Actor->GetName().find(Exclude) != std::string::npos)
					continue;

                printf("Actor %d: %s - Class: %s\n", i, Actor->GetName().c_str(), Actor->Class->Name.ToString().c_str());
            }
		}
	}
}

FRotator Utils::VectorToRotation(const FVector& Vec)
{
    FRotator Rot;
    Rot.Yaw = std::atan2(Vec.Y, Vec.X) * (180.0 / std::numbers::pi);
    Rot.Pitch = std::atan2(Vec.Z, std::sqrt(Vec.X * Vec.X + Vec.Y * Vec.Y)) * (180.0 / std::numbers::pi);
    Rot.Roll = 0.0;
    return Rot;
}

AReadyOrNotCharacter* Utils::GetBestTarget(float AngleWeight, float MaxFOV, bool TargetCivilians)
{
    if (!GVars.PlayerController || !GVars.ReadyOrNotChar || !GVars.Level)
        return nullptr;

    FVector PlayerPos;
    FRotator PlayerRot;
    GVars.PlayerController->GetPlayerViewPoint(&PlayerPos, &PlayerRot);
    FVector Forward = ForwardFromRot(PlayerRot);

    float BestScore = 999999.f;
    AReadyOrNotCharacter* BestTarget = nullptr;

    for (AActor* Actor : GVars.Level->Actors)
    {
        if (!Utils::IsValidActor(Actor)) continue;

        if (!Actor || Actor == GVars.ReadyOrNotChar) continue;

        if (!Actor->IsA(ASuspectCharacter::StaticClass()) &&
            !(TargetCivilians && Actor->IsA(ACivilianCharacter::StaticClass())))
            continue;

        auto* Target = (AReadyOrNotCharacter*)Actor;

        if (Target->IsDeadOrUnconscious() || Target->IsArrestedOrSurrendered()) continue;

        FVector TargetPos;
        Target->GetActorEyesViewPoint(&TargetPos, nullptr);

        FVector Dir = Normalize(TargetPos - PlayerPos);
        float Dot = Dot3(Forward, Dir);
        float Angle = AngleDegFromDot(Dot);

        float Dist = Length3(TargetPos - PlayerPos);

        if (Angle > MaxFOV) continue;

        // score = weighted sum: prioritize angle first, then distance
        float Score = Angle * AngleWeight + Dist; 

        if (Score < BestScore)
        {
            BestScore = Score;
            BestTarget = Target;
        }
    }

    return BestTarget;
}

FVector Utils::FRotatorToVector(const FRotator& Rot)
{
    double PitchRad = Rot.Pitch * (std::numbers::pi / 180.0);
    double YawRad = Rot.Yaw * (std::numbers::pi / 180.0);

    double CP = cos(PitchRad);
    double SP = sin(PitchRad);
    double CY = cos(YawRad);
    double SY = sin(YawRad);

    return FVector(
        CP * CY,   // X
        CP * SY,   // Y
        SP         // Z
    ).GetNormalized(); // normalize just in case
}

// Helper function to get or create player cheat data
PlayerCheatData& Utils::GetPlayerCheats(APlayerCharacter* Player)
{
    // If player doesn't exist in map, this creates a default entry
    return PlayerCheatMap[Player];
}

bool Utils::IsValidActor(AActor* Actor)
{
    if (!Actor) return false;

	if (!Actor->VTable) return false; // additional check

	if (Actor->bActorIsBeingDestroyed) return false;

    if (Actor->IsActorBeingDestroyed()) return false;
    return true;
}

float Utils::GetFOVFromScreenCoords(const ImVec2& ScreenLocation)
{
    ImVec2 ScreenCenter(GVars.ScreenSize.x / 2.0f, GVars.ScreenSize.y / 2.0f);

    float DeltaX = ScreenLocation.x - ScreenCenter.x;
    float DeltaY = ScreenLocation.y - ScreenCenter.y;

    return std::sqrt(DeltaX * DeltaX + DeltaY * DeltaY);
}

ImVec2 Utils::FVector2DToImVec2(FVector2D Vector)
{
	return ImVec2(Vector.X, Vector.Y);
}

FRotator Utils::GetRotationToTarget(const FVector& Start, const FVector& Target)
{
    FVector Delta = Target - Start;
    Delta.Normalize(); // Important for safe calculations

    FRotator Rotation;
    Rotation.Pitch = std::atan2(Delta.Z, std::sqrt(Delta.X * Delta.X + Delta.Y * Delta.Y)) * (180.0f / std::numbers::pi);
    Rotation.Yaw = std::atan2(Delta.Y, Delta.X) * (180.0f / std::numbers::pi);
    Rotation.Roll = 0.0f;

    return Rotation;
}

FVector2D Utils::ImVec2ToFVector2D(ImVec2 Vector)
{
	return FVector2D(Vector.x, Vector.y);
}