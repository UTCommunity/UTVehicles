#include "UTVehiclesPrivatePCH.h"
#include "AAAUTGameMode.h"

#define AUTGameMode AAAAUTGameMode

AUTGameMode::AUTGameMode(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

void AUTGameMode::DriverEnteredVehicle_Implementation(AVehicle* V, APawn* P)
{
	// TODO: Implement AUTMutator::DriverEnteredVehicle(...)
	//if (BaseMutator != NULL)
	//{
	//	BaseMutator->DriverEnteredVehicle(V, P);
	//}
}

void AUTGameMode::DriverLeftVehicle_Implementation(AVehicle* V, APawn* P)
{
	// TODO: Implement AUTMutator::DriverLeftVehicle(...)
	//if (BaseMutator != NULL)
	//{
	//	BaseMutator->DriverLeftVehicle(V, P);
	//}
}

bool AUTGameMode::CanLeaveVehicle_Implementation(AVehicle* V, APawn* P)
{
	// TODO: Implement AUTMutator::CanLeaveVehicle(...)
	//return (BaseMutator == NULL ? true : BaseMutator->CanLeaveVehicle(V, P));
	return true;
}

#undef AUTGameMode