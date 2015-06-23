#include "UTVehiclesPrivatePCH.h"
#include "AAAUTCharacter.h"

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

#undef AUTCharacter