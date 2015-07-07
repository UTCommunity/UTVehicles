#pragma once

#include "UTVehicleBase.h"
#include "UTWeaponPawn.generated.h"

UCLASS(NotPlaceable)
class AUTWeaponPawn : public AUTVehicleBase
{
	GENERATED_UCLASS_BODY()

	// Note: Replication events are split for retrieving the event 
	//       for each single property specifically and being able to override these in subclasses

	/** MyVehicleWeapon points to the weapon assoicated with this WeaponPawn and is replcated */
	UPROPERTY(transient, BlueprintReadOnly, ReplicatedUsing = OnRep_VehicleWeapon, Category = Vehicle)
	class AUTVehicleWeapon* MyVehicleWeapon;

	/** MyVehicle points to the vehicle that houses this WeaponPawn and is replicated */
	UPROPERTY(transient, BlueprintReadOnly, ReplicatedUsing = OnRep_Vehicle, Category = Vehicle)
	class AUTVehicle* MyVehicle;

	/** An index in to the Seats array of the vehicle housing this WeaponPawn.  It is replicated */
	UPROPERTY(transient, BlueprintReadOnly, ReplicatedUsing = OnRep_SeatIndex, Category = Vehicle)
	int32 MySeatIndex;

	// ******************************************************************************
	// Replication

	/** Replication event called when MyVehicleWeapon is replicated */
	UFUNCTION()
	virtual void OnRep_VehicleWeapon();

	/** Replication event called when MyVehicle is replicated */
	UFUNCTION()
	virtual void OnRep_Vehicle();

	/** Replication event called when MySeatIndex is replicated */
	UFUNCTION()
	virtual void OnRep_SeatIndex();

	/** Setup the specific seat given with MySeatIndex on MyVehicle with MyVehicleWeapon if everything is valid */
	UFUNCTION(BlueprintCallable, Category = Vehicle)
	void TryToSetupSeat();

};

