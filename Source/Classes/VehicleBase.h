#pragma once

#include "IBaseChangeInterface.h"
#include "VehicleBase.generated.h"

// TODO: Clean and use defined fields once WheeledVehicle is not subclassed anymore

UCLASS(Abstract, HideDropdown, NotBlueprintable)
class AVehicleBase : public APawn, public IBaseChangeInterface
{
	GENERATED_UCLASS_BODY()

	// Begin AActor Interface.
	virtual void PreInitializeComponents() override;
	// End AActor Interface

	/** amount of health this Vehicle has */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pawn, Replicated)
	int32 Health;
	/** normal maximum health of Vehicle - defaults to Default->Health unless explicitly set otherwise */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Pawn)
	int32 HealthMax;

	UPROPERTY(ReplicatedUsing = OnRep_DrivenVehicle)
	AVehicleBase* DrivenVehicle;

	UFUNCTION()
	virtual void OnRep_DrivenVehicle();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = View)
	float ViewPitchMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = View)
	float ViewPitchMax;

	/** Sets the base the Vehicle is driving on */
	virtual void SetBase(AActor* NewBase, FVector NewFloor = FVector::ZeroVector, USkeletalMeshComponent* SkelComp = NULL, const FName AttachName = NAME_None);

	virtual void BaseChange_Implementation() override;

	/**
	* Base change - if new base is pawn or decoration, damage based on relative mass and old velocity
	* Also, non-players will jump off pawns immediately
	*/
	virtual void JumpOffPawn();
};
