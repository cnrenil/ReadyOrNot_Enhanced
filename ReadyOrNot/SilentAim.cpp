#include <Windows.h>

#include "Cheats.h"

void Cheats::SilentAim()
{
	if (!CVars.SilentAim) return;

	if (SilentAimSettings.DrawFOV)
		Utils::DrawFOV(SilentAimSettings.MaxFOV, SilentAimSettings.FOVThickness);

	AActor* TargetActor =
		Utils::GetBestTarget(
			SilentAimSettings.TargetCivilians,
			SilentAimSettings.TargetArrested,
			SilentAimSettings.TargetSurrendered,
			SilentAimSettings.TargetDead,
			SilentAimSettings.MaxFOV,
			SilentAimSettings.RequiresLOS,
			SilentAimSettings.TargetBone,
			SilentAimSettings.TargetAll);

	if (GetAsyncKeyState(VK_LBUTTON) & 1)
	{
		if (!GVars.PlayerController || !TargetActor) return;

		float RandomValue = UKismetMathLibrary::RandomFloatInRange(0.0f, 100.0f);
		bool bShouldShoot = (RandomValue <= SilentAimSettings.HitChance);
		if (!bShouldShoot) return;

		if (!GVars.ReadyOrNotChar) return;
		auto* RONC = GVars.ReadyOrNotChar;

		std::wstring WideString = UtfN::StringToWString(SilentAimSettings.TargetBone);
		FName BoneName = UKismetStringLibrary::Conv_StringToName(WideString.c_str());

		FVector TargetLocation = ((AReadyOrNotCharacter*)TargetActor)->Mesh->GetBoneTransform(BoneName, ERelativeTransformSpace::RTS_World).Translation;

		if (RONC && RONC->GetEquippedWeapon())
		{
			RONC->GetEquippedWeapon()->OnFire(FRotator(), TargetLocation);
		}
	}
	if (!TargetActor) return;

	if (SilentAimSettings.DrawArrow)
		Utils::DrawSnapLine(TargetActor->K2_GetActorLocation(), SilentAimSettings.ArrowThickness);
}