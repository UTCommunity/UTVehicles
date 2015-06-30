#pragma once

#include "Vehicle.h"
#include "GameFramework/WheeledVehicle.h"
#include "SVehicle.generated.h"

// TODO: Clean and use defined fields once WheeledVehicle is not subclassed anymore

UCLASS(Abstract)
class ASVehicle : public AVehicle
{
	GENERATED_UCLASS_BODY()

	// Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	virtual void Tick(float DeltaSeconds) override;
	// End Actor interface


private_subobject:
//	/**  The main skeletal mesh associated with this Vehicle */
//	UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
//	class USkeletalMeshComponent* Mesh;

	// TODO: Rename. Maybe to VehicleMovement as in WheeledVehicle? (or VehicleSim as in previous UE4 versions)
	// TODO: Use USVehicleSimBase as SimObj type
	/** vehicle simulation component */
	UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly, NoClear, meta = (AllowPrivateAccess = "true"))
	//class USVehicleSimBase* SimObj;
	class UWheeledVehicleMovementComponent* SimObj;

public:

	/** Name of the MeshComponent. Use this name if you want to prevent creation of the component (with ObjectInitializer.DoNotCreateDefaultSubobject). */
	static FName VehicleMeshComponentName;

	/** Name of the VehicleMovement. Use this name if you want to use a different class (with ObjectInitializer.SetDefaultSubobjectClass). */
	static FName VehicleMovementComponentName;

	/** Returns Mesh subobject **/
	USkeletalMeshComponent* GetMesh() const { return Mesh; };

	/** Returns Vehicle simulation subobject **/
	//USVehicleSimBase* GetVehicleMovement() const { return SimObj; };
	UWheeledVehicleMovementComponent* GetVehicleMovement() const { return SimObj; };

};
