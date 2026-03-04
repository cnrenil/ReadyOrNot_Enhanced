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
        // Debug logging (Move inside condition or throttle if needed)
		if (CVars.Debug)
		{
			const std::string FuncName = Function->GetName();
			const std::string ObjName = Object->GetName();

			bool bFunctionPass =
				TextVars.DebugFunctionNameMustInclude.empty() ||
				FuncName.find(TextVars.DebugFunctionNameMustInclude) != std::string::npos;

			bool bObjectPass =
				TextVars.DebugFunctionObjectMustInclude.empty() ||
				ObjName.find(TextVars.DebugFunctionObjectMustInclude) != std::string::npos;

			if (bFunctionPass && bObjectPass)
			{
				if (!CVars.SaveDebugToFile)
				{
					printf("Function: %s | Object: %s\n", FuncName.c_str(), ObjName.c_str());
				}
				else
				{
					std::ofstream debugFile("ProcessEventLog.txt", std::ios::app);
					if (debugFile.is_open())
					{
						debugFile << "Function: " << FuncName << " | Class: " << (Object->Class ? Object->Class->GetName() : "None") << " | Object: " << ObjName << "\n";
						debugFile.close();
					}
				}
			}
		}

		if (CVars.SilentAim || CVars.ShootFromReticle || ESPSettings.BulletTracers)
		{
			if (Function->Name.ComparisonIndex != 0)
			{
				if (Object->IsA(ABaseMagazineWeapon::StaticClass()))
				{
					const std::string FuncName = Function->GetName();
					if (FuncName == "Server_OnFire")
					{
						auto* Weapon = static_cast<const ABaseMagazineWeapon*>(Object);
						auto* FireParams = static_cast<Params::BaseMagazineWeapon_OnFire*>(Params);

						if (Weapon && FireParams && GVars.ReadyOrNotChar && Utils::IsValidActor(GVars.ReadyOrNotChar) && Weapon->Owner == GVars.ReadyOrNotChar)
						{
							if (CVars.ShootFromReticle && GVars.PlayerController && Utils::IsValidActor(GVars.PlayerController))
							{
								FVector SpawnLoc;
								FVector Direction;
								if (GVars.PlayerController->DeprojectScreenPositionToWorld(
									GVars.ScreenSize.x / 2.0f + MiscSettings.ReticlePosition.x,
									GVars.ScreenSize.y / 2.0f + MiscSettings.ReticlePosition.y,
									&SpawnLoc,
									&Direction
								)) {
									FireParams->SpawnLoc = SpawnLoc;
									FireParams->Direction = UKismetMathLibrary::Conv_VectorToRotator(Direction);
								}
							}

							if (CVars.SilentAim)
								Cheats::SilentAim(FireParams);

							if (ESPSettings.BulletTracers)
							{
								BulletTracer NewTracer;
								FVector MuzzleLoc = FireParams->SpawnLoc;
								FVector DirectionVec = Utils::FRotatorToVector(FireParams->Direction);
								FVector MaxEnd = MuzzleLoc + DirectionVec * 10000.0;
								
								NewTracer.Points.push_back(MuzzleLoc);

								FVector CurrentTraceStart = MuzzleLoc;
								TArray<AActor*> IgnoreActors;
								if (GVars.ReadyOrNotChar) IgnoreActors.Add(GVars.ReadyOrNotChar);

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
										if (HitResult.bBlockingHit)
										{
											NewTracer.Points.push_back(HitResult.Location);
											// Reset start to just past the hit for next segment
											CurrentTraceStart = HitResult.Location + DirectionVec * 2.0f;
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
								
								{
									std::lock_guard<std::mutex> lock(TracerMutex);
									BulletTracersList.push_back(NewTracer);
								}
							}
						}
					}
				}
			}
		}
	} catch (...) {
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

