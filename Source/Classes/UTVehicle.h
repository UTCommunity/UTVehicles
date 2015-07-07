#pragma once

#include "UTVehicleBase.h"
#include "UTVehicleWeapon.h"
#include "UTVehicle.generated.h"

/**	The VehicleSeat struct defines each available seat in the vehicle. */
USTRUCT()
struct FVehicleSeat
{
	GENERATED_USTRUCT_BODY()

	// ---[ Connections] ----------------c--------

	/** Who is sitting in this seat. */
	UPROPERTY(transient, BlueprintReadOnly)
	APawn* StoragePawn; // TODO: Add EditInline-equivalent back to property

	/** Reference to the WeaponPawn if any */
	UPROPERTY(transient, BlueprintReadOnly) // TODO: Add EditInline-equivalent back to property
	AVehicle* SeatPawn;

	// ---[ Weapon ] ------------------------

	/** class of weapon for this seat */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon)
	TSubclassOf<AUTVehicleWeapon> GunClass;

	/** Reference to the gun */
	UPROPERTY(transient, BlueprintReadOnly, Category = Weapon)
	AUTVehicleWeapon* Gun;

	// ---[  Pawn Visibility ] ----------------------------------

	/** Is this a visible Seat */
	UPROPERTY(EditAnywhere, Category = "Pawn Visibility")
	bool bSeatVisible;

	// ---[ Misc ] ----------------------------------

	/** damage to the driver is multiplied by this value */
	UPROPERTY(EditAnywhere, Category = "Misc")
	float DriverDamageMult;

};

UCLASS(Blueprintable, Abstract, NotPlaceable)
class AUTVehicle : public AUTVehicleBase
{
	GENERATED_UCLASS_BODY()

	// Begin AActor Interface.
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	// End AActor Interface.

	// Begin AVehicle Interface.
	virtual bool DriverEnter_Implementation(APawn* NewDriver) override;
	virtual bool DriverLeave_Implementation(bool bForceLeave) override;
	//virtual void DriverLeft() override;

	virtual bool CanEnterVehicle_Implementation(APawn* P) override;
	virtual bool AnySeatAvailable_Implementation() override;
	virtual bool TryToDrive_Implementation(APawn* NewDriver) override;
	// End AVehicle Interface.


	/** If true the driver will have the flag attached to its model */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bDriverHoldsFlag;

	/** Determines if a driver/passenger in this vehicle can carry the flag */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bCanCarryFlag;

	/** whether this vehicle has been driven at any time */
	UPROPERTY(BlueprintReadOnly)
	bool bHasBeenDriven;

	/** Sound played if tries to enter a locked vehicle */
	UPROPERTY(EditDefaultsOnly)
	USoundCue* VehicleLockedSound;

	/*********************************************************************************************
	Look Steering
	********************************************************************************************* */
	
	/** Whether the driver is allowed to exit the vehicle */
	UPROPERTY(BlueprintReadWrite)
	bool bAllowedExit;

	/*********************************************************************************************
	* Vehicle Weapons, Drivers and Passengers
	*********************************************************************************************/

	/** Player state ofthe  player in passenger turret */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = Seats)
	AUTPlayerState* PassengerState;

	/**
	* Called when a passenger enters the vehicle
	*
	* @param P				The Pawn entering the vehicle
	* @param SeatIndex		The seat where he is to sit
	*/
	UFUNCTION()
	virtual bool PassengerEnter(APawn* P, int32 SeatIndex);

	/**
	* Called when a passenger leaves the vehicle
	*
	* @param	SeatIndex		Leaving from which seat
	*/
	UFUNCTION()
	virtual void PassengerLeave(int32 SeatIndex);


	/** information for each seat a player may occupy
	* @note: this array is on clients as well, but SeatPawn and Gun will only be valid for the client in that seat
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Seats)
	TArray<FVehicleSeat> Seats;

	/** This replicated property holds a mask of which seats are occupied.  */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = Vehicle)
	int32 SeatMask;

	/** Create all of the vehicle weapons */
	void InitializeSeats();

	/**
	* Request change to adjacent vehicle seat
	*/
	UFUNCTION(reliable, server, WithValidation)
	virtual void ServerAdjacentSeat(int32 Direction, AController* C);

	/**
	* Called when a client is requesting a seat change
	*
	* @network	Server-Side
	*/
	UFUNCTION(reliable, server, WithValidation)
	virtual void ServerChangeSeat(int32 RequestedSeat);

	/**
	* This function looks at 2 controllers and decides if one as priority over the other.  Right now
	* it looks to see if a human is against a bot but it could be extended to use rank/etc.
	*
	* @returns	ture if First has priority over second
	*/
	virtual bool HasPriority(AController* First, AController* Second);

	/**
	* ChangeSeat, this controller to change from it's current seat to a new one if (A) the new
	* set is empty or (B) the controller looking to move has Priority over the controller already
	* there.
	*
	* If the seat is filled but the new controller has priority, the current seat holder will be
	* bumped and swapped in to the seat left vacant.
	*
	* @param	ControllerToMove		The Controller we are trying to move
	* @param	RequestedSeat			Where are we trying to move him to
	*
	* @returns true if successful
	*/
	bool ChangeSeat(AController* ControllerToMove, int32 RequestedSeat);

	void SetSeatStoragePawn(int32 SeatIndex, APawn* PawnToSit);

	/**  Checks if passed player is the Driver (server only) 
	* @return whether the given vehicle pawn is in this vehicle's driver seat
	* (usually seat 0, but some vehicles may give driver control of a different seat when deployed)
	*/
	bool IsDriverSeat(AVehicle* TestSeatPawn);

	/** @return Returns true if a seat is not occupied (server only) */
	bool SeatAvailable(int32 SeatIndex);

	/** Retrieves the Seat index
	* NETWORK ALL
	*
	* @returns the Index for this Controller's current seat or -1 if there isn't one
	*/
	int32 GetSeatIndexForController(AController* ControllerToMove);

	/**
	* Retrieves the Controller for the given seat index (server only)
	* @returns the controller of a given seat.  Can be none if the seat is empty
	*/
	AController* GetControllerForSeatIndex(int32 SeatIndex);

	/** Retrieves the player state of the given seat
	* NETWORK ALL
	*/
	APlayerState* GetSeatPlayerState(int32 SeatNum);


	/*********************************************************************************************
	* Vehicle flag
	*********************************************************************************************/
	
	/** handles dealing with any flag the given driver/passenger may be holding */
	virtual void HandleEnteringFlag(AUTPlayerState* EnteringPRI);

	/**
	* If the driver enters the vehicle with a UTCarriedObject, this event
	* is triggered.
	*
	* NETWORK ALL
	*
	* @param	FlagActor		The object being carried
	* @param	NewDriver		The driver (may not yet have been set)
	*/
	virtual void AttachFlag(AUTCarriedObject* FlagActor, APawn* NewDriver);

};
