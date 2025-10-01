#include "Utils.h"

#include <imgui.h>
#include <Windows.h>
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
                if (Exclude && Actor->GetName().find(Exclude) != std::string::npos)
					continue;

                printf("Actor %d: %s - Class: %s\n", i, Actor->GetName().c_str(), Actor->Class->Name.ToString().c_str());
            }
		}
	}
}