#pragma once

#include "UTVehicleBase.h"
#include "UTVehicleWeapon.h"
#include "VehicleMovementEffect.h"
#include "UTVehicle.generated.h"

/** Stored Camera properties for saved positions */
struct FTimePosition
{
	FVector Position;
	float Time;

	FTimePosition() : Position(FVector(0.f)), Time(0.f) {};
	FTimePosition(FVector InPosition, float InTime)
		: Position(InPosition)
		, Time(InTime)
	{};

};

/**	The VehicleSeat struct defines each available seat in the vehicle. */
USTRUCT(BlueprintType)
struct FVehicleSeat
{
	GENERATED_USTRUCT_BODY()

	// ---[ Connections] ------------------------

	/** Who is sitting in this seat. */
	UPROPERTY(transient, BlueprintReadOnly, Category = References)
	APawn* StoragePawn;

	/** Reference to the WeaponPawn if any */
	UPROPERTY(transient, BlueprintReadOnly, Category = References)
	AVehicle* SeatPawn;

	/** Reference to the VehicleMovementEffect if any */
	UPROPERTY(transient, BlueprintReadOnly, Category = References)
	AVehicleMovementEffect* SeatMovementEffect;

	// ---[ Weapon ] ------------------------

	/** class of weapon for this seat */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon)
	TSubclassOf<AUTVehicleWeapon> GunClass;

	/** Reference to the gun */
	UPROPERTY(transient, BlueprintReadOnly, Category = Weapon) // TODO: Add EditInline-equivalent back to property
	AUTVehicleWeapon* Gun;

	// ---[ Camera ] ----------------------------------

	/** Name of the Bone/Socket to base the camera on */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FName CameraTag;

	/** Optional offset to add to the cameratag location, to determine base camera */
	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector CameraBaseOffset;

	/** Optional offset to add to the vehicle location, to determine safe trace start point */
	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector CameraSafeOffset;

	/** how far camera is pulled back */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraOffset;

	/** The Eye Height for Weapon Pawns */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraEyeHeight;

	// ---[ View Limits ] ----------------------------------

	// NOTE!! If ViewPitchMin/Max are set to 0.0f, the values associated with the host vehicle will be used

	/** Used for setting the ViewPitchMin on the Weapon pawn */
	UPROPERTY(EditAnywhere, Category = "View Limits")
	float ViewPitchMin;

	/** Used for setting the ViewPitchMax on the Weapon pawn */
	UPROPERTY(EditAnywhere, Category = "View Limits")
	float ViewPitchMax;

	// ---[  Pawn Visibility ] ----------------------------------

	/** Is this a visible Seat */
	UPROPERTY(EditAnywhere, Category = "Pawn Visibility")
	bool bSeatVisible;

	/** Name of the Bone to use as an anchor for the pawn */
	UPROPERTY(EditDefaultsOnly, Category = "Pawn Visibility")
	FName SeatBone;

	///** Offset from the origin to place the based pawn */
	UPROPERTY(EditDefaultsOnly, Category = "Pawn Visibility")
	FVector SeatOffset;

	/** Any additional rotation needed when placing the based pawn */
	UPROPERTY(EditDefaultsOnly, Category = "Pawn Visibility")
	FRotator SeatRotation;

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
	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;
	// End AActor Interface.

	// Begin AVehicle Interface.
	virtual bool DriverEnter_Implementation(APawn* NewDriver) override;
	virtual bool DriverLeave_Implementation(bool bForceLeave) override;
	//virtual void DriverLeft() override;

	virtual bool CanEnterVehicle_Implementation(APawn* P) override;
	virtual bool AnySeatAvailable_Implementation() override;
	virtual bool TryToDrive_Implementation(APawn* NewDriver) override;
	// End AVehicle Interface.

	// Begin AUTVehicleBase Interface.
	virtual void ServerAdjacentSeat_Implementation(int32 Direction, AController* C) override;
	virtual void ServerChangeSeat_Implementation(int32 RequestedSeat) override;
	// End AUTVehicleBase Interface.

	// Begin AVehicle Interface.
	virtual void AttachDriver_Implementation(APawn* P);
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
	* Look Steering
	********************************************************************************************* */
	
	/** Whether the driver is allowed to exit the vehicle */
	UPROPERTY(BlueprintReadWrite)
	bool bAllowedExit;

	/*********************************************************************************************
	* Camera
	********************************************************************************************* */

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Misc")
	float SeatCameraScale;

	/** If true, this will allow the camera to rotate under the vehicle which may obscure the view */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Camera")
	bool bRotateCameraUnderVehicle;

	/** If true, don't Z smooth lagged camera (for bumpier looking ride */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bNoZSmoothing;

	/** If true, make sure camera z stays above vehicle when looking up (to avoid clipping when flying vehicle going up) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bLimitCameraZLookingUp;

	/** If true, don't change Z while jumping, for more dramatic looking jumps */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bNoFollowJumpZ;

	/** Used only if bNoFollowJumpZ=true.  True when Camera Z is being fixed. */
	bool bFixedCamZ;

	/** Used only if bNoFollowJumpZ=true.  saves the Camera Z position from the previous tick. */
	float OldCamPosZ;

	/** Smoothing scale for lagged camera - higher values = shorter smoothing time. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CameraSmoothingFactor;

	/** Saved Camera positions (for lagging camera) */
	TArray<FTimePosition> OldPositions;
	
	/** Amount of camera lag for this vehicle (in seconds */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CameraLag;

	/** Smoothed Camera Offset */
	FVector CameraOffset;

	/** How far forward to bring camera if looking over nose of vehicle */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera")
	float LookForwardDist;

	/** hide vehicle if camera is too close */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera")
	float MinCameraDistSq;

	bool bCameraNeverHidesVehicle;

	/*********************************************************************************************/

	/** true if being spectated (set temporarily in UTPlayerController.GetPlayerViewPoint() */
	bool bSpectatedView;

	/*********************************************************************************************
	* Misc
	********************************************************************************************* */

	/** Is this vehicle dead */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_VehicleDied, Category = Damage)
	bool bDeadVehicle;

	/*********************************************************************************************
	* Replication methods
	*********************************************************************************************/

protected:

	/** Called when bDeadVehicle is replicated */
	UFUNCTION()
	virtual void OnRep_VehicleDied();

public:

	/*********************************************************************************************
	* Vehicle Weapons, Drivers and Passengers
	*********************************************************************************************/

	/** 
	* Places the driver character's mesh into the seat position 
	* NETWORK ALL
	*/
	void SitDriver(AUTCharacter* UTChar, int32 SeatIndex);

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

	/** 
	* Actives/deactivates vehicle effects for the given seat
	* @network ALL
	*/
	void SetMovementEffect(int32 SeatIndex, bool bSetActive, AUTCharacter* UTChar = NULL);

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
	* @NETWORK ALL
	*
	* @param	FlagActor		The object being carried
	* @param	NewDriver		The driver (may not yet have been set)
	*/
	virtual void AttachFlag(AUTCarriedObject* FlagActor, APawn* NewDriver);


	/*********************************************************************************************
	* View / Camera methods
	*********************************************************************************************/

	/**
	* Retrieves the camera start position for the given seat (without camera lag)
	*
	* @NETWORK ALL
	*
	* @return the camera focus position (without camera lag)
	*/
	virtual FVector GetCameraFocus(int32 SeatIndex);

	/**
	* Retrieves the camera start position for the given seat (with camera lag)
	*
	* @NETWORK ALL
	*
	* @return the camera focus position (adjusted for camera lag)
	*/
	virtual FVector GetCameraStart(int32 SeatIndex);

	/**
	* Clamps the camera Z-value for the given seat
	*
	* @NETWORK ALL
	* 
	* @return the camera focus position (adjusted for camera lag)
	*/
	virtual float LimitCameraZ(float CurrentCamZ, float OriginalCamZ, int32 SeatIndex);

	/**
	* Calculate camera view point for the seat when driving this vehicle 
	*
	* @NETWORK ALL
	*
	* @param	DeltaTime	Delta time seconds since last update
	* @param	SeatIndex	The seat idnex to retrieve the camera view point for
	* @param	OutResult	Camera configuration
	*/
	virtual void VehicleCalcCamera(float DeltaTime, int32 SeatIndex, struct FMinimalViewInfo& OutResult, bool bPivotOnly = false);

};
