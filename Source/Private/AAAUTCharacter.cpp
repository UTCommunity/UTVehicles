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

void AUTCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		// Bind OnDied event to this pawn to garbage collect this zombie actor
		OnDied.AddDynamic(this, &AUTCharacter::OnPawnDied);
	}
}

bool AUTCharacter::Died(AController* EventInstigator, const FDamageEvent& DamageEvent)
{
	if (Role == ROLE_Authority && !IsDead())
	{
		// notify the vehicle we are currently driving
		AVehicle* DrivenVehicleVec = Cast<AVehicle>(DrivenVehicle);
		if (DrivenVehicleVec != NULL)
		{
			if (GetCharacterMovement() != NULL)
			{
				GetCharacterMovement()->Velocity = DrivenVehicle->GetVelocity();
			}

			DrivenVehicleVec->DriverDied();
		}
	}

	return Super::Died(EventInstigator, DamageEvent);
}

void AUTCharacter::OnPawnDied(AController* Killer, const UDamageType* DamageType)
{
	if (Role < ROLE_Authority)
		return;
	
	// clear driven vehicle
	DrivenVehicle = NULL;
}

bool AUTCharacter::CanUse() const
{
	return CanUseInternal();
}

bool AUTCharacter::CanUseInternal_Implementation() const
{
	return bCanUse;
}

void AUTCharacter::StartDriving(APawn* Vehicle)
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
}

#undef AUTCharacter