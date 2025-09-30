#pragma once
#include "Cheats.h"
#include "SDK/Engine_classes.hpp"
#include "SDK/ReadyOrNot_classes.hpp"
#include "ImGui/imgui.h"

using namespace SDK;

struct Utils
{
	static UWorld* GetWorldSafe(); // can return nullptr
	static APlayerController* GetPlayerController(); // can return nullptr
	static unsigned ConvertImVec4toU32(ImVec4 Color);
};

struct Variables
{
	APlayerController* PlayerController = nullptr;
	APawn* Pawn = nullptr;
	ACharacter* Character = nullptr;
	AReadyOrNotCharacter* ReadyOrNotChar = nullptr;
	UWorld* World = nullptr;
	ULevel* Level = nullptr;

	// Constructor to initialize variables safely
	Variables() {
		AutoSetVariables();
	}

	void AutoSetVariables() {

		// Get PlayerController first
		APlayerController* currentPC = Utils::GetPlayerController();
		if (!currentPC) {
			// Clear all dependent variables if PlayerController is null
			this->PlayerController = nullptr;
			this->Pawn = nullptr;
			this->Character = nullptr;
			this->ReadyOrNotChar = nullptr;
			this->World = Utils::GetWorldSafe(); // World can exist without PlayerController
			this->Level = this->World ? this->World->PersistentLevel : nullptr;
			return;
		}

		// Update PlayerController if changed
		if (this->PlayerController != currentPC) {
			this->PlayerController = currentPC;
			// Reset dependent variables when PlayerController changes
			this->Pawn = nullptr;
			this->Character = nullptr;
			this->ReadyOrNotChar = nullptr;
		}

		// Update Pawn
		if (this->PlayerController && this->Pawn != this->PlayerController->Pawn) {
			this->Pawn = this->PlayerController->Pawn;
			// Reset Character-dependent variables when Pawn changes
			this->Character = nullptr;
			this->ReadyOrNotChar = nullptr;
		}

		// Update Character
		if (this->PlayerController && this->Character != this->PlayerController->Character) {
			this->Character = this->PlayerController->Character;
			// Reset ReadyOrNotChar when Character changes
			this->ReadyOrNotChar = nullptr;
		}

		// Update ReadyOrNotChar
		if (this->Character) {
			AReadyOrNotCharacter* newReadyOrNotChar = static_cast<AReadyOrNotCharacter*>(this->Character);
			if (this->ReadyOrNotChar != newReadyOrNotChar) {
				this->ReadyOrNotChar = newReadyOrNotChar;
			}
		}

		// Update World
		UWorld* currentWorld = Utils::GetWorldSafe();
		if (this->World != currentWorld) {
			this->World = currentWorld;
			this->Level = nullptr; // Reset Level when World changes
		}

		// Update Level
		if (this->World && this->Level != this->World->PersistentLevel) {
			this->Level = this->World->PersistentLevel;
		}
	}
} inline GVars;

static inline float Dot3(const FVector& A, const FVector& B)
{
	return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
}

static inline float Length3(const FVector& V)
{
	return sqrtf(V.X * V.X + V.Y * V.Y + V.Z * V.Z);
}

static inline FVector Normalize(const FVector& V)
{
	float L = Length3(V);
	if (L <= 0.0001f) return FVector{ 0,0,0 };
	return FVector{ V.X / L, V.Y / L, V.Z / L };
}

static inline float ClampFloat(float v, float a, float b)
{
	return v < a ? a : (v > b ? b : v);
}

static inline float AngleDegFromDot(float Dot)
{
	Dot = ClampFloat(Dot, -1.0f, 1.0f);
	return acosf(Dot) * (180.0f / 3.1415926535f);
}

static inline void ClampRotator(FRotator& R)
{
	// Use UE5's built-in normalization instead of manual clamping
	R.Normalize();

	// Still clamp pitch for gameplay reasons
	if (R.Pitch > 89.f)  R.Pitch = 89.f;
	if (R.Pitch < -89.f) R.Pitch = -89.f;
	R.Roll = 0.f;
}

static inline FVector ForwardFromRot(const FRotator& Rot)
{
	float PitchRad = Rot.Pitch * (3.1415926535f / 180.0f);
	float YawRad = Rot.Yaw * (3.1415926535f / 180.0f);
	float CP = cosf(PitchRad);
	return FVector{
		cosf(YawRad) * CP,
		sinf(YawRad) * CP,
		sinf(PitchRad)
	};
}