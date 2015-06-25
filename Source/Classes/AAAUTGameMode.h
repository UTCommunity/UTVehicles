#pragma once

#include "UTGameMode.h"
#include "Vehicle.h"
#include "AAAUTGameMode.generated.h"

UCLASS()
class AAAAUTGameMode : public AUTGameMode
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly)
	void DriverEnteredVehicle(AVehicle* V, APawn* P);

	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly)
	void DriverLeftVehicle(AVehicle* V, APawn* P);

	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly)
	bool CanLeaveVehicle(AVehicle* V, APawn* P);

};
