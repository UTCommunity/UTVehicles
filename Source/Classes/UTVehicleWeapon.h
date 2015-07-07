#pragma once

#include "UTVehicleWeapon.generated.h"

UCLASS(Blueprintable, Abstract)
class AUTVehicleWeapon : public AUTWeapon
{
	GENERATED_UCLASS_BODY()

	/** Holds a link in to the Seats array in MyVehicle that represents this weapon */
	UPROPERTY(BlueprintReadOnly, Category = Vehicle)
	int32 SeatIndex;

	/** Holds a link to the parent vehicle */
	UPROPERTY(transient, BlueprintReadOnly, ReplicatedUsing = OnRep_Vehicle, Category = Vehicle)
	class AUTVehicle* MyVehicle;

	// ******************************************************************************
	// Replication

	/** Replication event called when MyVehicle is replicated */
	UFUNCTION()
	virtual void OnRep_Vehicle();

};

