#include "UTVehiclesPrivatePCH.h"
#include "AAAUTCharacter.h"
#include "Vehicle.h"

#define AUTCharacter AAAAUTCharacter

AUTCharacter::AUTCharacter(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bCanUse = true;
	VehicleCheckRadius = 150.f;
}

bool AUTCharacter::CanUse() const
{
	return CanUseInternal();
}

bool AUTCharacter::CanUseInternal_Implementation() const
{
	return bCanUse;
}

// TODO: Implement once methods are virtual
/*void AUTCharacter::StartDriving(APawn* Vehicle)
{
	Super::StartDriving(Vehicle);

	if (AVehicle* V = Cast<AVehicle>(Vehicle))
	{
		V->AttachDriver(this);
	}
}

void AUTCharacter::StopDriving(APawn* Vehicle)
{
	Super::StopDriving(Vehicle);

	if (AVehicle* V = Cast<AVehicle>(Vehicle))
	{
		V->StopFiring();
		V->DetachDriver(this);
	}
}*/

#undef AUTCharacter