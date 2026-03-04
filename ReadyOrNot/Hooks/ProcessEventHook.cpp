#include "pch.h"
#include "Engine.h"

using tProcessEvent = void(*)(const UObject*, UFunction*, void*);
tProcessEvent oProcessEvent = nullptr;

extern std::atomic<bool> Cleaning;
extern std::atomic<int> g_ProcessEventCount;

void hkProcessEvent(const UObject* Object, UFunction* Function, void* Params)
{
	g_ProcessEventCount.fetch_add(1);
	static thread_local bool bInsideHook = false;
	if (!Object || !Function || Cleaning.load() || bInsideHook || Utils::bIsLoading) {
		if (oProcessEvent) oProcessEvent(Object, Function, Params);
		g_ProcessEventCount.fetch_sub(1);
		return;
	}

	bInsideHook = true;

	try {
		if (CVars.Debug)
		{
			const std::string FuncName = Function->GetName();
			const std::string ObjName = Object->GetName();

			std::string lowerName = FuncName;
			std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

			bool bHasFunctionFilter = !TextVars.DebugFunctionNameMustInclude.empty();
			bool bHasObjectFilter = !TextVars.DebugFunctionObjectMustInclude.empty();

			bool bFunctionPass = bHasFunctionFilter && FuncName.find(TextVars.DebugFunctionNameMustInclude) != std::string::npos;
			bool bObjectPass = bHasObjectFilter && ObjName.find(TextVars.DebugFunctionObjectMustInclude) != std::string::npos;

			bool bIsBulletEvent = !bHasFunctionFilter && !bHasObjectFilter && 
								  (lowerName.find("hit") != std::string::npos || 
								   lowerName.find("impact") != std::string::npos || 
								   lowerName.find("deflect") != std::string::npos || 
								   lowerName.find("ricochet") != std::string::npos ||
								   lowerName.find("bullet") != std::string::npos ||
								   lowerName.find("damage") != std::string::npos);

			bool bShouldLog = false;
			
			if (bHasFunctionFilter || bHasObjectFilter) {
				bool fMatch = bHasFunctionFilter ? bFunctionPass : true;
				bool oMatch = bHasObjectFilter ? bObjectPass : true;
				bShouldLog = (fMatch && oMatch);
			} else {
				bShouldLog = bIsBulletEvent;
			}

			if (bShouldLog)
			{
				std::ofstream debugFile("ProcessEventLog.txt", std::ios::app);
				if (debugFile.is_open())
				{
					debugFile << "[DEBUG] Function: " << FuncName << " | Class: " << (Object->Class ? Object->Class->GetName() : "None") << " | Object: " << ObjName << "\n";
					debugFile.close();
				}
			}
		}

		if (CVars.SilentAim || CVars.ShootFromReticle || ESPSettings.BulletTracers)
		{
			if (Function->Name.ComparisonIndex != 0)
			{
				const std::string FuncName = Function->GetName();

				if (FuncName == "Server_OnFire" || FuncName == "Multicast_OnFire" || FuncName == "OnFire")
				{
					if (Object->IsA(ABaseMagazineWeapon::StaticClass()))
					{
						auto* Weapon = static_cast<const ABaseMagazineWeapon*>(Object);
						if (!Weapon || !Weapon->IsA(ABaseMagazineWeapon::StaticClass())) return;

						bool IsLocalPlayerShot = (GVars.ReadyOrNotChar && Utils::IsValidActor(GVars.ReadyOrNotChar) && Weapon->Owner == GVars.ReadyOrNotChar);

						FVector TracerStart; // Where the line starts VISUALLY (Muzzle)
						FVector ShotStart;   // Where the bullet starts LOGICALLY (Muzzle or Reticle)
						FVector Direction;
						int32 Seed = 0;
						bool ShouldAddTracer = false;

						if (FuncName == "Server_OnFire") {
							auto* P = static_cast<Params::BaseMagazineWeapon_Server_OnFire*>(Params);
							if (IsLocalPlayerShot) {
								if (CVars.ShootFromReticle && GVars.PlayerController && Utils::IsValidActor(GVars.PlayerController)) {
									FVector ReticleSpawnLoc, ReticleDir;
									if (GVars.PlayerController->DeprojectScreenPositionToWorld(
										GVars.ScreenSize.x / 2.0f + MiscSettings.ReticlePosition.x,
										GVars.ScreenSize.y / 2.0f + MiscSettings.ReticlePosition.y,
										&ReticleSpawnLoc, &ReticleDir
									)) {
										P->SpawnLoc = ReticleSpawnLoc;
										P->Direction = UKismetMathLibrary::Conv_VectorToRotator(ReticleDir);
									}
								}
								if (CVars.SilentAim) Cheats::SilentAim(P, true);
							}
						}
						else if (FuncName == "OnFire") {
							auto* P = static_cast<Params::BaseMagazineWeapon_OnFire*>(Params);
							if (IsLocalPlayerShot) {
								if (CVars.ShootFromReticle && GVars.PlayerController && Utils::IsValidActor(GVars.PlayerController)) {
									FVector ReticleSpawnLoc, ReticleDir;
									if (GVars.PlayerController->DeprojectScreenPositionToWorld(
										GVars.ScreenSize.x / 2.0f + MiscSettings.ReticlePosition.x,
										GVars.ScreenSize.y / 2.0f + MiscSettings.ReticlePosition.y,
										&ReticleSpawnLoc, &ReticleDir
									)) {
										P->SpawnLoc = ReticleSpawnLoc;
										P->Direction = UKismetMathLibrary::Conv_VectorToRotator(ReticleDir);
									}
								}
								if (CVars.SilentAim) Cheats::SilentAim(P, false);
							}
						}
						else { // Multicast_OnFire
							auto* P = static_cast<Params::BaseMagazineWeapon_Multicast_OnFire*>(Params);
							TracerStart = P->SpawnLoc;
							ShotStart = P->SpawnLoc;
							Direction = FVector(P->DirectionNet.X, P->DirectionNet.Y, P->DirectionNet.Z);
							ShouldAddTracer = true;
						}

						if (ESPSettings.BulletTracers && ShouldAddTracer)
						{
							BulletTracer NewTracer;
							FVector MaxEnd = TracerStart + Direction * 15000.0f;
							
							NewTracer.Points.push_back(TracerStart);

							FVector CurrentTraceStart = TracerStart;
							TArray<AActor*> IgnoreActors;
							if (GVars.ReadyOrNotChar) IgnoreActors.Add(GVars.ReadyOrNotChar);

							bool bHitSolid = false;
							for (int j = 0; j < 3; j++)
							{
								FHitResult HitResult;
								if (GVars.World && GVars.World->VTable && UKismetSystemLibrary::LineTraceSingle(
									GVars.World,
									CurrentTraceStart,
									MaxEnd,
									ETraceTypeQuery::TraceTypeQuery1,
									true,
									IgnoreActors,
									EDrawDebugTrace::None,
									&HitResult,
									true,
									FLinearColor(),
									FLinearColor(),
									0.0f
								))
								{
									if (HitResult.bBlockingHit && HitResult.Location.X != 0.0f)
									{
										NewTracer.Points.push_back(HitResult.Location);
										bHitSolid = true;
										CurrentTraceStart = HitResult.Location + Direction * 3.0f;
									}
									else
									{
										NewTracer.Points.push_back(MaxEnd);
										break;
									}
								}
								else
								{
									NewTracer.Points.push_back(MaxEnd);
									break;
								}
							}

							NewTracer.CreationTime = ImGui::GetTime();
							NewTracer.bClosed = bHitSolid;

							std::lock_guard<std::mutex> lock(TracerMutex);
							BulletTracersList.push_back(NewTracer);
						}
					}
				}
			}
		}
	}
	catch (...) {
		// Suppress exceptions
	}

	bInsideHook = false;
	if (oProcessEvent) oProcessEvent(Object, Function, Params);
	g_ProcessEventCount.fetch_sub(1);
}

bool Hooks::HookProcessEvent()
{
	void* ProcessEventAddr = reinterpret_cast<void*>(InSDKUtils::GetImageBase() + Offsets::ProcessEvent);

	if (MH_CreateHook(
		ProcessEventAddr,
		&hkProcessEvent,
		reinterpret_cast<void**>(&oProcessEvent)
	) != MH_OK)
	{
		return false;
	}

	if (MH_EnableHook(ProcessEventAddr) != MH_OK)
		return false;

	return true;
}

