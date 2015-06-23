#pragma once

#include "Vehicle.generated.h"

UCLASS(Abstract)
class AVehicle : public APawn, public IVehicleInterface
{
	GENERATED_UCLASS_BODY()

	// Begin AActor Interface.
	virtual void PreInitializeComponents() override;
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	virtual bool CanBeBaseForCharacter(APawn* APawn) const override;
	//virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual float InternalTakeRadialDamage(float Damage, struct FRadialDamageEvent const& RadialDamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;
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
	UFUNCTION()
	virtual void OnRep_Driver();

	/** true if vehicle is being driven. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DrivingChanged, Category = Vehicle)
	bool bDriving;
	UFUNCTION()
	virtual void OnRep_DrivingChanged();

protected:

	// generic controls (set by controller, used by concrete derived classes)

	// between -1 and 1
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control input")
	float Steering; 
	// between -1 and 1
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control input")
	float Throttle;
	// between -1 and 1
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control input")
	float Rise;

public: 

	UFUNCTION(BlueprintCallable, Category = "Control input")
	virtual void SetSteeringInput(float InSteering) { Steering = InSteering; };
	UFUNCTION(BlueprintCallable, Category = "Control input")
	virtual void SetThrottleInput(float InThrottle) { Throttle = InThrottle; };
	UFUNCTION(BlueprintCallable, Category = "Control input")
	virtual void SetRiseInput(float InRise) { Rise = InRise; };
	
	/** whether to render driver seated in vehicle */
	bool bDriverIsVisible;

	/** If true, attach the driver to the vehicle when he starts using it. */
	bool bAttachDriver;

	void SetInputs(float InForward, float InStrafe, float InUp);

	/** @return returns true if Pawn P is allowed to enter this vehicle */
	bool CanEnterVehicle(APawn* P);

	/** @return returns true if Pawn P successfully became driver of this vehicle */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Vehicle)
	bool TryToDrive(APawn* NewDriver);

	/** @return returns true if a seat is available for a pawn */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Vehicle)
		bool AnySeatAvailable();

	/** Make Pawn P the new driver of this vehicle */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Vehicle)
	bool DriverEnter(APawn* NewDriver);

	/** Called from the Controller when player wants to get out. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Vehicle)
	bool DriverLeave(bool bForceLeave);

	// DriverLeft() called by DriverLeave()
	UFUNCTION()
	virtual void DriverLeft();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Vehicle)
	void SetDriving(bool b);

	UFUNCTION(BlueprintNativeEvent, Category = Vehicle)
	void DrivingStatusChanged();

	float DriverTakeRadialDamage(float Damage, struct FRadialDamageEvent const& RadialDamageEvent, class AController* EventInstigator, AActor* DamageCauser);

};
