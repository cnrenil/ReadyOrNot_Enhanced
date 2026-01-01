#include "Engine.h"

void** vTable = nullptr;

using tProcessEvent = void(*)(const UObject*, UFunction*, void*);
tProcessEvent oProcessEvent = nullptr;


void hkProcessEvent(const UObject* Object, UFunction* Function, void* Params)
{
	if (Function)
	{
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
				printf(
					"Function: %s\nClass: %s\nObject: %s\n\n",
					FuncName.c_str(),
					Object->Class->GetName().c_str(),
					ObjName.c_str()
				);
			}
		}

		if (strcmp(Function->GetName().c_str(), "Server_OnFire") == 0)
		{
			auto* FireParams =
				reinterpret_cast<Params::BaseMagazineWeapon_OnFire*>(Params);

			Cheats::SilentAim(FireParams);
		}
	}

	// Call original
	oProcessEvent(Object, Function, Params);
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
