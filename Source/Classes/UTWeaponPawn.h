#pragma once

#include "UTVehicleBase.h"
#include "UTWeaponPawn.generated.h"

UCLASS(NotPlaceable)
class AUTWeaponPawn : public AUTVehicleBase
{
	GENERATED_UCLASS_BODY()

	// Begin AActor Interface.
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	// End AActor Interface

	// Begin APawn Interface.
	virtual void PossessedBy(AController* NewController) override;
	virtual void RecalculateBaseEyeHeight() override;
	// End APawn Interface

	// Begin AVehicleBase Interface.
	virtual void BaseChange_Implementation() override;
	virtual void JumpOffPawn() override;
	// End AVehicleBase Interface

	// Begin AVehicle Interface.
	virtual void AttachDriver_Implementation(APawn* P) override;
	virtual bool PlaceExitingDriver(APawn* ExitingDriver = NULL) override;
	virtual void DriverLeft() override;
	// End AVehicle Interface

	// Begin AUTVehicleBase Interface.
	virtual void ServerAdjacentSeat_Implementation(int32 Direction, AController* C) override;
	virtual void ServerChangeSeat_Implementation(int32 RequestedSeat) override;
	// End AUTVehicleBase Interface


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

