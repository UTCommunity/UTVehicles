#pragma once

#include "UTMutator.h"
#include "Vehicle.h"
#include "AAAUTMutator.generated.h"

// Note: Class is unused. Only created for merging/migrating purposes for pull request

UCLASS()
class AAAAUTMutator : public AUTMutator
{
	GENERATED_UCLASS_BODY()

	// TEMP: TODO: REMOVE
	AAAAUTMutator* NextMutator;

	// Note: Insert after NotifyMatchStateChange(...)
	// ...

	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly)
	void DriverEnteredVehicle(AVehicle* V, APawn* P);

	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly)
	void DriverLeftVehicle(AVehicle* V, APawn* P);

	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly)
	bool CanLeaveVehicle(AVehicle* V, APawn* P);

	// ... //

};
