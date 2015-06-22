#include "UTVehiclesPrivatePCH.h"
#include "AAATempVehicleGameMode.h"
#include "AAAUTPlayerController.h"
#include "UTGameMode.h"

AAAATempVehicleGameMode::AAAATempVehicleGameMode(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UClass> PlayerPawnObject(TEXT("Class'/UTVehicles/DefaultCharacter_Vec.DefaultCharacter_Vec_C'"));
	if (PlayerPawnObject.Object != NULL)
	{
		DefaultPawnClass = PlayerPawnObject.Object;
	}

	PlayerControllerClass = AAAAUTPlayerController::StaticClass();
}
