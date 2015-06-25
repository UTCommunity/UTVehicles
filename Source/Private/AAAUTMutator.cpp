#include "UTVehiclesPrivatePCH.h"
#include "AAAUTMutator.h"

// Note: Class is unused. Only created for merging/migrating purposes for pull request

#define AUTMutator AAAAUTMutator

AUTMutator::AUTMutator(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

// Note: Insert after OverridePickupQuery(...)
// ...

void AUTMutator::DriverEnteredVehicle_Implementation(AVehicle* V, APawn* P)
{
	if (NextMutator != NULL)
	{
		NextMutator->DriverEnteredVehicle(V, P);
	}
}

void AUTMutator::DriverLeftVehicle_Implementation(AVehicle* V, APawn* P)
{
	if (NextMutator != NULL)
	{
		NextMutator->DriverLeftVehicle(V, P);
	}
}

bool AUTMutator::CanLeaveVehicle_Implementation(AVehicle* V, APawn* P)
{
	if (NextMutator != NULL)
	{
		return NextMutator->CanLeaveVehicle(V, P);
	}

	return true;
}

#undef AUTMutator