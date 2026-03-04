#include "pch.h"

struct BonePair { int Parent; int Child; };
BonePair SuspectSkeletonBones_1[] = {
    {50, 51}, // neck_1 -> Head
    {9, 50}, // spine_3 -> neck_1
    {8, 9}, // spine_2 -> spine_3
    {7, 8}, // spine_1 -> spine_2
    {10, 11}, // clavicle_LE -> upperarm_LE
    {11, 12}, // upperarm_LE -> lowerarm_LE
    {12, 15}, // lowerarm_LE -> hand_LE
    {62, 63}, // clavicle_RI -> upperarm_RI
    {63, 64}, // upperarm_RI -> lowerarm_RI
    {64, 67}, // lowerarm_RI -> hand_RI
    {103, 104}, // thigh_LE -> calf_LE
    {104, 107}, // calf_LE -> foot_LE
    {115, 116}, // thigh_RI -> calf_RI
    {116, 119}, // calf_RI -> foot_RI
};

BonePair SuspectSkeletonBones_2[] = {
    {44, 45}, // Neck -> Head
    {5, 44},  // Spine3 -> Neck
    {3, 5},   // Spine2 -> Spine3
    {2, 3},   // Spine1 -> Spine2
    {6, 7},   // Left clavicle -> UpperArm_LE
    {7, 8},   // UpperArm_LE -> LowerArm_LE
    {8, 11},  // LowerArm_LE -> Hand_LE
    {49, 50}, // Right clavicle -> UpperArm_RI
    {50, 51}, // UpperArm_RI -> LowerArm_RI
    {51, 54}, // LowerArm_RI -> Hand_RI
    {111, 112}, // Thigh_LE -> Calf_LE
    {112, 115}, // Calf_LE -> Foot_LE
    {99, 100},  // Thigh_RI -> Calf_RI
    {100, 103}  // Calf_RI -> Foot_RI
};

BonePair SuspectSkeletonBones[] = {
    {44, 45}, // Neck -> Head
    {5, 44},  // Spine3 -> Neck
    {3, 5},   // Spine2 -> Spine3
    {2, 3},   // Spine1 -> Spine2
    {6, 7},   // Left clavicle -> UpperArm_LE
    {7, 8},   // UpperArm_LE -> LowerArm_LE
    {8, 11},  // LowerArm_LE -> Hand_LE
    {49, 50}, // Right clavicle -> UpperArm_RI
    {50, 51}, // UpperArm_RI -> LowerArm_RI
    {51, 54}, // LowerArm_RI -> Hand_RI
    {111, 112}, // Thigh_LE -> Calf_LE
    {112, 115}, // Calf_LE -> Foot_LE
    {99, 100},  // Thigh_RI -> Calf_RI
    {100, 103}  // Calf_RI -> Foot_RI
};

BonePair CivilianSkeletonBones_1[] = {
    {50, 51}, // Neck -> Head
    {9, 50},  // Spine3 -> Neck
    {7, 9},   // Spine2 -> Spine3
    {1, 7},   // Spine1 -> Spine2
    {10, 11}, // Left clavicle -> UpperArm_LE
    {11, 12}, // UpperArm_LE -> LowerArm_LE
    {12, 15}, // LowerArm_LE -> Hand_LE
    {62, 63}, // Right clavicle -> UpperArm_RI
    {63, 64}, // UpperArm_RI -> LowerArm_RI
    {64, 67}, // LowerArm_RI -> Hand_RI
    {103, 104}, // Thigh_LE -> Calf_LE
    {104, 107}, // Calf_LE -> Foot_LE
    {115, 116}, // Thigh_RI -> Calf_RI
    {116, 119}  // Calf_RI -> Foot_RI
};

BonePair CivilianSkeletonBones_2[] = {
    {44, 45}, // neck_1 -> Head
    {5, 44}, // spine_3 -> neck_1
    {4, 5}, // spine_2 -> spine_3
    {3, 4}, // spine_1 -> spine_2
    {6, 7}, // clavicle_LE -> upperarm_LE
    {7, 8}, // upperarm_LE -> lowerarm_LE
    {8, 11}, // lowerarm_LE -> hand_LE
    {49, 50}, // clavicle_RI -> upperarm_RI
    {50, 51}, // upperarm_RI -> lowerarm_RI
    {51, 54}, // lowerarm_RI -> hand_RI
    {111, 112}, // thigh_LE -> calf_LE
    {112, 115}, // calf_LE -> foot_LE
    {99, 100}, // thigh_RI -> calf_RI
    {100, 103}, // calf_RI -> foot_RI
};

BonePair CivilianSkeletonBones[] = {
    {44, 45}, // neck_1 -> Head
    {5, 44}, // spine_3 -> neck_1
    {4, 5}, // spine_2 -> spine_3
    {3, 4}, // spine_1 -> spine_2
    {6, 7}, // clavicle_LE -> upperarm_LE
    {7, 8}, // upperarm_LE -> lowerarm_LE
    {8, 11}, // lowerarm_LE -> hand_LE
    {49, 50}, // clavicle_RI -> upperarm_RI
    {50, 51}, // upperarm_RI -> lowerarm_RI
    {51, 54}, // lowerarm_RI -> hand_RI
    {111, 112}, // thigh_LE -> calf_LE
    {112, 115}, // calf_LE -> foot_LE
    {99, 100}, // thigh_RI -> calf_RI
    {100, 103}, // calf_RI -> foot_RI
};

bool IsSuspect = false;
bool IsSwat = false;
bool IsPlayer = false;
int32 ViewportX = 0.0f;
int32 ViewportY = 0.0f;

auto RenderColor = IM_COL32(255, 255, 255, 255);

void Cheats::RenderESP()
{
	if (!CVars.ESP || Utils::bIsLoading) return;
    if (!GVars.PlayerController || !GVars.Level || !GVars.GameState) return;

    // Additional safety during transitions
    if (!GVars.World || !GVars.World->VTable) return;

    ULevel* Level = GVars.Level;
    if (!Level) return;

	if (ESPSettings.ShowObjectives && GVars.GameState)
	{
		AReadyOrNotGameState* GameState = GVars.GameState;
		if (!GameState) return;

        TArray<AReportableActor*> AllReportableActors = GameState->AllReportableActors;
        for (AReportableActor* ReportableActor : AllReportableActors)
		{
	        if (!ReportableActor || !Utils::IsValidActor(ReportableActor)) continue;

            FVector2D ObjectiveScreen;
            
            if (GVars.PlayerController->ProjectWorldLocationToScreen(ReportableActor->K2_GetActorLocation(), &ObjectiveScreen, true))
            {
                ImU32 ObjectiveColor = Utils::IsObjectiveCompletedForActor(ReportableActor)
                    ? Utils::ConvertImVec4toU32(ESPSettings.ObjectiveCompletedColor)
                    : Utils::ConvertImVec4toU32(ESPSettings.ObjectiveActiveColor);
		        ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(ObjectiveScreen.X, ObjectiveScreen.Y), 5, ObjectiveColor);
				ImGui::GetBackgroundDrawList()->AddText(ImVec2(ObjectiveScreen.X + 8, ObjectiveScreen.Y - 7), ObjectiveColor, ReportableActor->ReportableName.ToString().c_str());
            }
	    }
	}

	std::vector<AActor*> ActorsSafe;
	if (Level->Actors.Num() > 0)
	{
		for (int i = 0; i < Level->Actors.Num(); i++)
		{
			AActor* Actor = Level->Actors[i];
			if (Actor) ActorsSafe.push_back(Actor);
		}
	}

    for (AActor* Actor : ActorsSafe)
    {
    	if (!Actor || !Utils::IsValidActor(Actor)) continue;

        if (ESPSettings.ShowTraps)
        {
	        FVector2D TrapScreen;

        	if (Actor->IsA(ATrapActor::StaticClass()) && GVars.PlayerController->ProjectWorldLocationToScreen(Actor->K2_GetActorLocation(), &TrapScreen, true))
        	{
				const char* TrapTypeName = Localization::T("UNKNOWN_TRAP");
				if (((ATrapActor*)Actor)->TrapType == ETrapType::Explosive)
					TrapTypeName = Localization::T("EXPLOSIVE_TRAP");
				else if (((ATrapActor*)Actor)->TrapType == ETrapType::Flashbang)
                    TrapTypeName = Localization::T("FLASHBANG_TRAP");
				else if (((ATrapActor*)Actor)->TrapType == ETrapType::Alarm)
					TrapTypeName = Localization::T("ALARM_TRAP");

        		ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(TrapScreen.X, TrapScreen.Y), 3, Colors::Red);
        		ImGui::GetBackgroundDrawList()->AddText(ImVec2(TrapScreen.X + 5, TrapScreen.Y - 5), Colors::Red, TrapTypeName);
        		continue;
        	}
        }

        if (true) // Change to CVars.ShowGunESP
        {

        }

        AReadyOrNotCharacter* TargetActor = nullptr;

        if (Actor->IsA(ASuspectCharacter::StaticClass()) || Actor->IsA(ACivilianCharacter::StaticClass()))
        {
			TargetActor = reinterpret_cast<AReadyOrNotCharacter*>(Actor);
			IsSwat = false;
            IsPlayer = false;
        }
        else if (Actor->IsA(APlayerCharacter::StaticClass()))
        {
            TargetActor = reinterpret_cast<AReadyOrNotCharacter*>(Actor);
            IsSuspect = false;
            IsSwat = true;
            IsPlayer = true;
        }
		else if (ESPSettings.ShowTeam && Actor->IsA(ASWATCharacter::StaticClass())) 
		{
			TargetActor = reinterpret_cast<AReadyOrNotCharacter*>(Actor);
            IsSuspect = false;
			IsSwat = true;
            IsPlayer = false;
		}
    	else
			continue;

        if (!TargetActor) continue;

        if (GVars.ReadyOrNotChar && GVars.ReadyOrNotChar == TargetActor) continue;

        if (!IsSwat && TargetActor->IsSuspect())
            IsSuspect = true;
        else
			IsSuspect = false;

        if (IsSuspect) RenderColor = Utils::ConvertImVec4toU32(ESPSettings.SuspectColor);
		else RenderColor = Utils::ConvertImVec4toU32(ESPSettings.CivilianColor);

		// Highlight unique mission-target suspects by matching actor Tags against objective SuspectTags
		if (IsSuspect && Utils::IsTargetSuspect(TargetActor))
		{
			RenderColor = Utils::ConvertImVec4toU32(ESPSettings.TargetSuspectColor);
		}

		if (IsSwat) RenderColor = Utils::ConvertImVec4toU32(ESPSettings.TeamColor);

        if (TargetActor->IsDeadOrUnconscious() || TargetActor->IsIncapacitated()) RenderColor = Utils::ConvertImVec4toU32(ESPSettings.DeadColor);
        else if (!IsSwat && TargetActor->IsArrestedOrSurrendered() || !IsSwat && TargetActor->bIsBeingArrested) RenderColor = Utils::ConvertImVec4toU32(ESPSettings.ArrestColor);

        USkeletalMeshComponent* Mesh = TargetActor->Mesh;
        if (!Mesh) continue;

        if (!IsSuspect && Mesh->GetBoneName(45).ToString() == "Head")
        {
            memcpy(CivilianSkeletonBones, CivilianSkeletonBones_2, sizeof(CivilianSkeletonBones_2));
        }
        else
            memcpy(CivilianSkeletonBones, CivilianSkeletonBones_1, sizeof(CivilianSkeletonBones_1));

        if (IsSuspect && Mesh->GetBoneName(45).ToString() == "Head")
        {
            memcpy(SuspectSkeletonBones, SuspectSkeletonBones_2, sizeof(SuspectSkeletonBones_2));
        }
        else
            memcpy(SuspectSkeletonBones, SuspectSkeletonBones_1, sizeof(SuspectSkeletonBones_1));

        std::vector<FVector2D> BonePositions = {};

        for (auto& pair : IsSuspect ? SuspectSkeletonBones : CivilianSkeletonBones)
        {
            FName ParentName = Mesh->GetBoneName(pair.Parent);
			FName ChildName = Mesh->GetBoneName(pair.Child);

            FVector ParentPos = Mesh->GetBoneTransform(ParentName, ERelativeTransformSpace::RTS_World).Translation;
            FVector ChildPos = Mesh->GetBoneTransform(ChildName, ERelativeTransformSpace::RTS_World).Translation;
            FVector2D ParentScreen, ChildScreen, ActorScreen;

            if (ESPSettings.LOS)
            {
                FVector Start = GVars.POV->Location;
                FVector End = ParentPos;

                TArray<AActor*> IgnoreActors;
                IgnoreActors.Add(GVars.ReadyOrNotChar);

                FHitResult HitResult;
                bool bHit = UKismetSystemLibrary::LineTraceSingle(
                    GVars.World,
                    Start,
                    End,
                    ETraceTypeQuery::TraceTypeQuery1,
                    true,
                    IgnoreActors,
                    EDrawDebugTrace::None,
                    &HitResult,
                    true,
                    FLinearColor(),
                    FLinearColor(),
                    1.0f
                );

                AActor* HitActor = nullptr;
                HitActor = HitResult.Component->GetOwner();

                bool bHasLOS = !HitResult.bBlockingHit || HitActor == TargetActor;
                if (!bHasLOS)
                    continue;
            }

            if (ESPSettings.Bones)
            {
                
                if (GVars.PlayerController->ProjectWorldLocationToScreen(ParentPos, &ParentScreen, true) &&
                    GVars.PlayerController->ProjectWorldLocationToScreen(ChildPos, &ChildScreen, true))
                {
                    if (ESPSettings.ShowBox)
                    {
                        BonePositions.push_back(ParentScreen);
                        BonePositions.push_back(ChildScreen);
                    }

                    GVars.PlayerController->GetViewportSize(&ViewportX, &ViewportY);
                    if (ParentScreen.X == 0.f && ParentScreen.Y == 0.f or ParentScreen.X > ViewportX or ParentScreen.Y > ViewportY) continue;
                    ImGui::GetBackgroundDrawList()->AddLine(
                        ImVec2(ParentScreen.X, ParentScreen.Y),
                        ImVec2(ChildScreen.X, ChildScreen.Y),
                        RenderColor,
                        1.5f
                    );
                }
            }

            if (ESPSettings.ShowEnemyDistance)
            {
                FVector ActorLocation = TargetActor->K2_GetActorLocation();
				float Distance = GVars.POV->Location.GetDistanceToInMeters(ActorLocation);
                if (Distance < 0.0f) continue;
                FVector2D DistanceScreen;
                if (GVars.PlayerController->ProjectWorldLocationToScreen(ActorLocation, &DistanceScreen, true))
                {
                    char DistanceText[32];
                    snprintf(DistanceText, sizeof(DistanceText), "%.1f m", Distance);
                    ImGui::GetBackgroundDrawList()->AddText(
                        ImVec2(DistanceScreen.X, DistanceScreen.Y + 35),
                        RenderColor,
                        DistanceText
                    );
                }
			}
            if (ESPSettings.ShowEnemyName && !IsSwat)
            {
                FVector NameLocation = TargetActor->K2_GetActorLocation();
                FVector2D NameScreen;
                if (GVars.PlayerController->ProjectWorldLocationToScreen(NameLocation, &NameScreen, true))
                {
                    std::string CharName = TargetActor->SpeechCharacterName.ToString();
                    if (!CharName.empty())
                    {
                        ImGui::GetBackgroundDrawList()->AddText(
                            ImVec2(NameScreen.X, NameScreen.Y - 20),
                            RenderColor,
                            CharName.c_str()
                        );
                    }
                }
            }
            if (ESPSettings.ShowTeam && IsPlayer && TargetActor && TargetActor->PlayerState && TargetActor->PlayerState->GetPlayerName() && GVars.PlayerController->ProjectWorldLocationToScreen(Actor->K2_GetActorLocation(), &ActorScreen, true))
            {
                ImGui::GetBackgroundDrawList()->AddText(
                    ImVec2(ActorScreen.X, ActorScreen.Y + 50),
                    RenderColor,
                    TargetActor->PlayerState->GetPlayerName().ToString().c_str()
                );
            }
        }
        if (ESPSettings.ShowBox)
        {
			FVector2D TopLeft, BottomRight;

			for (FVector2D BonePos : BonePositions)
            {
				if (BonePos.X < TopLeft.X || TopLeft.X == 0)
                    TopLeft.X = BonePos.X;

				if (BonePos.Y < TopLeft.Y || TopLeft.Y == 0)
					TopLeft.Y = BonePos.Y;

                if (BonePos.X > BottomRight.X)
					BottomRight.X = BonePos.X;

				if (BonePos.Y > BottomRight.Y)
					BottomRight.Y = BonePos.Y;
            }
            ImGui::GetBackgroundDrawList()->AddRect(
                ImVec2(TopLeft.X, TopLeft.Y),
                ImVec2(BottomRight.X, BottomRight.Y),
                RenderColor,
                0.0f,
                0,
                1.5f
            );
        }
    }

	if (ESPSettings.BulletTracers && GVars.PlayerController)
	{
		std::lock_guard<std::mutex> lock(TracerMutex);
		float CurrentTime = ImGui::GetTime();
		for (auto it = BulletTracersList.begin(); it != BulletTracersList.end(); )
		{
			if (CurrentTime - it->CreationTime > ESPSettings.TracerDuration)
			{
				it = BulletTracersList.erase(it);
				continue;
			}
			
			float Age = CurrentTime - it->CreationTime;
			float FadeRatio = 1.0f - (Age / ESPSettings.TracerDuration);
			if (FadeRatio < 0.0f) FadeRatio = 0.0f;
			
			ImVec4 BaseColor;
			if (ESPSettings.TracerRainbow)
			{
				float Hue = fmodf(it->CreationTime * 0.5f, 1.0f);
				float R, G, B;
				ImGui::ColorConvertHSVtoRGB(Hue, 1.0f, 1.0f, R, G, B);
				BaseColor = ImVec4(R, G, B, FadeRatio);
			}
			else 
			{
				BaseColor = ImVec4(ESPSettings.TracerColor.x, ESPSettings.TracerColor.y, ESPSettings.TracerColor.z, FadeRatio);
			}

			// Draw segments with 3D clipping to prevent disappearing when behind camera
			FVector CamLoc = GVars.PlayerController->PlayerCameraManager->GetCameraLocation();
			FVector CamFwd = GVars.PlayerController->PlayerCameraManager->GetActorForwardVector();

			size_t PointsCount = it->Points.size();

			for (size_t i = 0; i < PointsCount; i++)
			{
				FVector P1 = it->Points[i];
				FVector2D p1Screen;
				bool bP1Visible = GVars.PlayerController->ProjectWorldLocationToScreen(P1, &p1Screen, true);

				// Draw the segment if we haven't reached the end
				if (i + 1 < PointsCount)
				{
					FVector P2 = it->Points[i+1];
					FVector2D p2Screen;
					bool bP2Visible = GVars.PlayerController->ProjectWorldLocationToScreen(P2, &p2Screen, true);
					
					FVector P1_Final = P1;
					FVector P2_Final = P2;
					bool bCanDraw = false;

					if (bP1Visible && bP2Visible)
					{
						bCanDraw = true;
					}
					else if (bP1Visible || bP2Visible) 
					{
						auto GetDist = [&](const FVector& P) { return ((P.X - CamLoc.X) * CamFwd.X + (P.Y - CamLoc.Y) * CamFwd.Y + (P.Z - CamLoc.Z) * CamFwd.Z); };
						float d1 = GetDist(P1);
						float d2 = GetDist(P2);
						float epsilon = 1.0f;

						if ((d1 < epsilon && d2 >= epsilon) || (d2 < epsilon && d1 >= epsilon))
						{
							float t = (epsilon - d1) / (d2 - d1);
							FVector ClippedPoint = P1 + (P2 - P1) * t;
							if (d1 < epsilon) P1_Final = ClippedPoint;
							else P2_Final = ClippedPoint;
							GVars.PlayerController->ProjectWorldLocationToScreen(P1_Final, &p1Screen, true);
							GVars.PlayerController->ProjectWorldLocationToScreen(P2_Final, &p2Screen, true);
							bCanDraw = true;
						}
					}

					if (bCanDraw)
					{
						ImVec2 start(p1Screen.X, p1Screen.Y);
						ImVec2 end(p2Screen.X, p2Screen.Y);
						ImGui::GetBackgroundDrawList()->AddLine(start, end, ImGui::ColorConvertFloat4ToU32(ImVec4(BaseColor.x, BaseColor.y, BaseColor.z, BaseColor.w * 0.2f)), 6.0f);
						ImGui::GetBackgroundDrawList()->AddLine(start, end, ImGui::ColorConvertFloat4ToU32(ImVec4(BaseColor.x, BaseColor.y, BaseColor.z, BaseColor.w * 0.5f)), 3.0f);
						ImGui::GetBackgroundDrawList()->AddLine(start, end, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, BaseColor.w)), 1.0f);
					}
				}

				// Draw impact points (skip the dummy far point if not closed)
				if (i > 0 && bP1Visible && (it->bClosed || i < PointsCount - 1))
				{
					ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(p1Screen.X, p1Screen.Y), 6.5f, ImGui::ColorConvertFloat4ToU32(ImVec4(BaseColor.x, BaseColor.y, BaseColor.z, BaseColor.w * 0.3f)));
					ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(p1Screen.X, p1Screen.Y), 4.0f, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, BaseColor.w)));
				}
			}
			++it;
		}
	}
}
