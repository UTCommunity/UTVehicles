#include "UTVehiclesPrivatePCH.h"
#include "SVehicle.h"
//#include "SVehicleSimBase.h"
#include "DisplayDebugHelpers.h"

// TODO: Clean and use defined fields once WheeledVehicle is not subclassed anymore

FName ASVehicle::VehicleMovementComponentName(TEXT("MovementComp"));
FName ASVehicle::VehicleMeshComponentName(TEXT("VehicleMesh"));

ASVehicle::ASVehicle(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(VehicleMeshComponentName);
	Mesh->SetCollisionProfileName(UCollisionProfile::Vehicle_ProfileName);
	Mesh->BodyInstance.bSimulatePhysics = true;
	Mesh->BodyInstance.bNotifyRigidBodyCollision = true;
	Mesh->BodyInstance.bUseCCD = true;
	Mesh->bBlendPhysics = true;
	Mesh->bGenerateOverlapEvents = true;
	Mesh->bCanEverAffectNavigation = false;
	RootComponent = Mesh;

	//SimObj = CreateDefaultSubobject<USVehicleSimBase, USVehicleSimBase>(VehicleMovementComponentName);
	SimObj = CreateDefaultSubobject<UWheeledVehicleMovementComponent, UWheeledVehicleMovementComponent4W>(VehicleMovementComponentName);
	SimObj->SetIsReplicated(true); // Enable replication by default
	SimObj->UpdatedComponent = Mesh;
}

// TODO: cleanup
void ASVehicle::PreInitializeComponents()
{
	//SimObj = GetVehicleMovement();
	Super::PreInitializeComponents();
}

void ASVehicle::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

#if WITH_VEHICLE
	static FName NAME_Vehicle_Sim = FName(TEXT("Vehicle_Sim"));

	if (DebugDisplay.IsDisplayOn(NAME_Vehicle_Sim) && GetVehicleMovement() != NULL)
	{
		GetVehicleMovement()->DrawDebug(Canvas, YL, YPos);
		//GetSVehicleDebug(DebugInfo);
	}
#endif
}

void ASVehicle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (SimObj != NULL)
	{
		SimObj->SetThrottleInput(Throttle);
		SimObj->SetSteeringInput(Steering);
		//SimObj->SetRiseInput(Rise);
	}
}