#pragma once

#include "Vehicle.h"
#include "UTVehicle.h"
#include "AAAUTPlayerController.generated.h"

UCLASS()
class AAAAUTPlayerController : public AUTPlayerController
{
	GENERATED_UCLASS_BODY()

	// Temp
	// Begin AUTPlayerController Interface.
	virtual void GetPlayerViewPoint(FVector& Location, FRotator& Rotation) const override;
	// End AUTPlayerController Interface.

	// Temp
	// Begin AUTPlayerController Interface.
	virtual void InitInputSystem() override;
	virtual void SetupInputComponent() override;
	virtual void ServerSuicide_Implementation() override;
	virtual void UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& HiddenComponents) override;
	
	virtual void SwitchWeapon(int32 Group) override;
	virtual void SwitchWeaponGroup(int32 Group) override;

	virtual void BehindView(bool bWantBehindView) override;
	// End AUTPlayerController Interface.

	/** Getter for Vehicle */
	FORCEINLINE AVehicle* GetVehicle() const { return Cast<AVehicle>(GetPawn()); }

	virtual void MoveForward(float Val) override;
	virtual void MoveBackward(float Val) override;
	virtual void MoveLeft(float Val) override;
	virtual void MoveRight(float Val) override;
	virtual void Jump() override;
	virtual void JumpRelease() override;
	virtual void Crouch() override;
	virtual void UnCrouch() override;

	// Note: Insert after DebugTest()

	//-----------------------------------------------
	// World interaction

	/** Last "use" time - used to limit "use" frequency */
	UPROPERTY()
	float LastUseTime;

	/** Entry point function for player interactions with the world, redirects to ServerUse. */
	UFUNCTION(Exec)
	virtual void Use();

	/** Player pressed UseKey. */
	UFUNCTION(unreliable, server, WithValidation)
	virtual void ServerUse();

	/**
	* Will perform the Use action which tries to find an interactable object nearby
	* (like a trigger button, vehicle...).
	*
	* @Return Returns true if player the Use action was handled
	*/
	UFUNCTION()
	virtual bool PerformedUseAction();
	virtual bool PerformedUseActionInternal();

	// TODO: Implement triggering Level Blueprint nodes
	UFUNCTION()
	virtual bool TriggerInteracted() { return false; };

	//------------------------

	//-----------------------------------------------
	// Vehicle support

	/** Set when use fails to enter nearby vehicle (to prevent smart use from also putting you on hoverboard) */
	UPROPERTY()
	bool bJustFoundVehicle;

	/** Custom scaling for vehicle check radius. */
	UPROPERTY(EditAnywhere)
	float VehicleCheckRadiusScaling;

	/** Tries to find a vehicle to drive within a limited radius. Returns true if successful */
	UFUNCTION()
	virtual bool FindVehicleToDrive();

	/**
	* Tries to find a vehicle nearby to drive
	*
	* @param bEnterVehicle if true then player enters the found vehicle
	* @return Returns vehicle which can be driven
	*/
	UFUNCTION()
	virtual AUTVehicle* CheckVehicleToDrive(bool bEnterVehicle);

	/**
	* Check whether the given vehicle can be driven
	* @param V The vehicle to check
	* @param bEnterVehicle 
	* @return returns the Vehicle passed in if it can be driven
	*/
	UFUNCTION()
	virtual AUTVehicle* CheckPickedVehicle(AUTVehicle* V, bool bEnterVehicle);

	//------------------------

};
