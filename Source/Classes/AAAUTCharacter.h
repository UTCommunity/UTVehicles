#pragma once

#include "UTCharacter.h"
#include "AAAUTCharacter.generated.h"

UCLASS()
class AAAAUTCharacter : public AUTCharacter
{
	GENERATED_UCLASS_BODY()

	// Begin AUTCharacter Interface.
	virtual void BeginPlay() override;
	virtual bool Died(AController* EventInstigator, const FDamageEvent& DamageEvent) override;
	virtual void Tick(float DeltaSeconds) override;
	// End AUTCharacter Interface.

	UFUNCTION()
	virtual void OnPawnDied(AController* Killer, const UDamageType* DamageType);

	/** Radius that is checked for nearby vehicles when pressing use */
	UPROPERTY(EditAnywhere)
	float VehicleCheckRadius;

	virtual void StartDriving(APawn* Vehicle) override;
	virtual void StopDriving(APawn* Vehicle) override;

	// Note: Insert after ServerUseCarriedObject()
	// ...

public:

	/**
	* Check if the character can use things
	*
	* The default implementation may be overridden or extended by implementing the custom CanUse event in Blueprints.
	*
	* @Return Whether the character can use things in general.
	*/
	UFUNCTION(BlueprintCallable, Category = "Pawn|Character")
	bool CanUse() const;

protected:

	/** Can this pawn use things? */
	UPROPERTY()
	bool bCanUse;

	/**
	* Customizable event to check if the character can use things in the world.
	* Default implementation returns true if the character has set the specific flag to use things and if
	* specific conditions are met like not feigning etc.
	*
	* @Return Whether the character can use things in general.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Pawn|Character|InternalEvents", meta = (DisplayName = "CanUse"))
	bool CanUseInternal() const;

};
