#include "UTVehiclesPrivatePCH.h"
#include "AAAUTPlayerInput.h"

UAAAUTPlayerInput::UAAAUTPlayerInput()
: Super()
{
	TArray<FCustomKeyBinding>& TempBinds = GetClass()->GetDefaultObject<UAAAUTPlayerInput>()->CustomBinds;
	OverrideCustomBinding(TempBinds, FName(TEXT("E")), FString(TEXT("Use")));
	OverrideCustomBinding(TempBinds, FName(TEXT("G")), FString(TEXT("SwitchToBestWeapon")));
}

bool UAAAUTPlayerInput::OverrideCustomBinding(TArray<FCustomKeyBinding>& InCustomBinds, FName KeyName, FString Command)
{
	bool bFound = false;
	for (int32 i = 0; i < InCustomBinds.Num(); i++)
	{
		if (FKey(InCustomBinds[i].KeyName) == KeyName)
		{
			InCustomBinds[i].Command = Command;
			InCustomBinds[i].FriendlyName = FString(TEXT(""));

			bFound = true;
			break;
		}
	}

	// binding not found, append
	if (!bFound)
	{
		InCustomBinds.Add(FCustomKeyBinding(KeyName, IE_Pressed, Command));
	}

	return bFound;
}