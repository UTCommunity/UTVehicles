#pragma once

#include "UTWeapon.h"
#include "UTCharacter.h"
#include "Vehicle.generated.h"

/**
 * Vehicle: The base class of all vehicles.
 */
UCLASS(Abstract)
class AVehicle : public APawn //, public IVehicleInterface // Note: Interface only needed for pull request // FIXME: Remove interface
{
	GENERATED_UCLASS_BODY()

	// Begin AActor Interface.
	virtual void PreInitializeComponents() override;
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	virtual bool CanBeBaseForCharacter(APawn* APawn) const override;
	//virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual float InternalTakeRadialDamage(float Damage, struct FRadialDamageEvent const& RadialDamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;
	virtual void Tick(float DeltaSeconds) override;
	// End AActor Interface

	/** amount of health this Vehicle has */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pawn, Replicated)
	int32 Health;
	/** normal maximum health of Vehicle - defaults to Default->Health unless explicitly set otherwise */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Pawn)
	int32 HealthMax;

	// TODO: originally Driver was a APawn. Allow Driver to be APawn again and set UTDriver?
	/** Pawn driving this vehicle. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Driver, Category = Vehicle)
	AUTCharacter* Driver;
	
	/** true if vehicle is being driven. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DrivingChanged, Category = Vehicle)
	bool bDriving;

	/** Positions (relative to vehicle) to try putting the player when exiting.
	 * Optional: automatic system for determining exitpositions if none is specified. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vehicle)
	TArray<FVector>	ExitPositions;

	/** Radius for automatic exit positions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vehicle)
	float ExitRadius;

	/** Offset from center for Exit test circle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vehicle)
	FVector	ExitOffset;


private_subobject:
	/**  The main skeletal mesh associated with this Vehicle */
	UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh;

public:

	/** Returns Mesh subobject **/
	USkeletalMeshComponent* GetMesh() const { return Mesh; };

protected:

	// ******************************************************************************
	// Weapon related variables/methods

	/** Replication event called when Driver is replicated */
	UFUNCTION()
	virtual void OnRep_Driver();

	/** Replication event called when driving state has changed and is replicated */
	UFUNCTION()
	virtual void OnRep_DrivingChanged();


	// ******************************************************************************
	// generic controls (set by controller, used by concrete derived classes)

	// between -1 and 1
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Control input")
	float Steering; 
	// between -1 and 1
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Control input")
	float Throttle;
	// between -1 and 1
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Control input")
	float Rise;

public: 

	UFUNCTION(BlueprintCallable, Category = "Control input")
	virtual void SetSteeringInput(float InSteering) { Steering = InSteering; };
	UFUNCTION(BlueprintCallable, Category = "Control input")
	virtual void SetThrottleInput(float InThrottle) { Throttle = InThrottle; };
	UFUNCTION(BlueprintCallable, Category = "Control input")
	virtual void SetRiseInput(float InRise) { Rise = InRise; };
	
	void SetInputs(float InForward, float InStrafe, float InUp);

	/** Sets the base the Vehicle is driving on */
	virtual void SetBase(AActor* NewBase, FVector NewFloor = FVector::ZeroVector, USkeletalMeshComponent* SkelComp = NULL, const FName AttachName = NAME_None);

	// ******************************************************************************
	// Driver attachment, handling, etc.

	/** whether to render driver seated in vehicle */
	UPROPERTY(BlueprintReadWrite, Category = Vehicle)
	bool bDriverIsVisible;

	/** If true, attach the driver to the vehicle when he starts using it. */
	UPROPERTY(BlueprintReadWrite, Category = Vehicle)
	bool bAttachDriver;

	/** damage to the driver is multiplied by this value */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Vehicle)
	float DriverDamageMult;

	/** damage momentum multiplied by this value before being applied to vehicle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Vehicle)
	float MomentumMult;

	/** Crank up the net priority when posessed by a player */
	virtual void PossessedBy(AController* NewController) override;
	/** restore original netpriority changed when possessing */
	virtual void UnPossessed() override;

	/** Called when Controller possesses vehicle, for any visual/audio effects */
	virtual void EntryAnnouncement(AController* NewController);

	/** Called when the player wants to suicide. This will trigger PlayerSuicide on the driver. */
	virtual bool PlayerSuicideInternal();

	/**
	* Attach driver to vehicle.
	* Sets up the Pawn to drive the vehicle (rendering, physics, collision..).
	* Called only if bAttachDriver is true.
	* Network : ALL
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Vehicle)
	void AttachDriver(APawn* P);

	/**
	* Detach Driver from vehicle.
	* Network : ALL
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Vehicle)
	void DetachDriver(APawn* P);

	/**
	* Checks if the given Pawn can enter this vehicle
	* Network : ALL
	*
	* @return returns true if Pawn P is allowed to enter this vehicle
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Vehicle)
	bool CanEnterVehicle(APawn* P);

	// ******************************************************************************
	// Main vehicle interface... like entering, exiting, trying to drive vehicle, etc.

	/** @return returns true if Pawn P successfully became driver of this vehicle */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Vehicle)
	bool TryToDrive(APawn* NewDriver);

	/** @return returns true if a seat is available for a pawn (Server only) */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Vehicle)
	bool AnySeatAvailable();

	/** Make Pawn P the new driver of this vehicle. Changes controller ownership across pawns. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Vehicle)
	bool DriverEnter(APawn* NewDriver);

	/** Called from the Controller when player wants to get out. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Vehicle)
	bool DriverLeave(bool bForceLeave);

	// DriverLeft() called by DriverLeave()
	UFUNCTION()
	virtual void DriverLeft();

	/** Called when the driver inside a vehicle has died */
	UFUNCTION()
	virtual void DriverDied();

	/** @return The exit rotation for this controller on leaving the vehicle */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Vehicle)
	FRotator GetExitRotation(AController* C);

	/** Find an acceptable position to place the exiting driver pawn, and move it there.
	*	Returns true if pawn was successfully placed.
	*/
	UFUNCTION(BlueprintCallable, Category = Vehicle)
	bool PlaceExitingDriver(APawn* ExitingDriver = NULL);

	/** Tries to find exit position on either side of vehicle, in back, or in front
	* @return true if driver successfully exited. 
	*/
	UFUNCTION(BlueprintCallable, Category = Vehicle)
	bool FindAutoExit(APawn* ExitingDriver);

	/* Used by PlaceExitingDriver() to evaluate automatically generated exit positions */
	UFUNCTION(BlueprintCallable, Category = Vehicle)
	bool TryExitPos(APawn* ExitingDriver, FVector ExitPos, bool bMustFindGround);

	/*
	* Change the driving status of the vehicle
	* replicates to clients and notifies via DrivingStatusChanged()
	* @param bNewDriving TRUE for actively driving the vehicle, FALSE otherwise
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Vehicle)
	void SetDriving(bool bNewDriving);

	/**Called client/server when 'bDriving' changes*/
	UFUNCTION(BlueprintNativeEvent, Category = Vehicle)
	void DrivingStatusChanged();

	// ******************************************************************************
	// Damaging

	float DriverTakeRadialDamage(float Damage, struct FRadialDamageEvent const& RadialDamageEvent, class AController* EventInstigator, AActor* DamageCauser);

	// ******************************************************************************
	// Weapon related variables/methods

	UPROPERTY(BlueprintReadOnly, Replicated, Category = Vehicle)
	AUTWeapon* Weapon;

	// firemodes with input currently being held down (pending or actually firing)
	UPROPERTY(BlueprintReadOnly, Category = Vehicle)
	TArray<uint8> PendingFire;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Vehicle|Inventory", meta = (DisplayName = "CreateInventory", AdvancedDisplay = "bAutoActivate"))
	virtual AUTInventory* Blueprint_CreateInventory(TSubclassOf<AUTInventory> NewInvClass, bool bAutoActivate = true);

	template<typename InvClass = AUTInventory>
	inline InvClass* CreateInventory(TSubclassOf<InvClass> NewInvClass, bool bAutoActivate = true)
	{
		InvClass* Result = (InvClass*)Blueprint_CreateInventory(NewInvClass, bAutoActivate);
		checkSlow(Result == NULL || Result->IsA(InvClass::StaticClass()));
		return Result;
	}

	/** weapon firing */
	UFUNCTION(BlueprintCallable, Category = Vehicle)
	virtual void StartFire(uint8 FireModeNum);

	UFUNCTION(BlueprintCallable, Category = Vehicle)
	virtual void StopFire(uint8 FireModeNum);

	UFUNCTION(BlueprintCallable, Category = Vehicle)
	virtual void StopFiring();

	// ******************************************************************************
};
