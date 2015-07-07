#include "UTVehiclesPrivatePCH.h"
#include "VehicleBase.h"

AVehicleBase::AVehicleBase(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	Health = 100;

	// FOV / Sight
	ViewPitchMin = -16384;
	ViewPitchMax = 16383;
}

void AVehicleBase::PreInitializeComponents()
{
	// important that this comes before Super so mutators can modify it
	if (HealthMax == 0)
	{
		HealthMax = GetDefault<AVehicleBase>()->Health;
	}

	Super::PreInitializeComponents();
}

void AVehicleBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AVehicleBase, Health, COND_None); // would be nice to bind to teammates and spectators, but that's not an option :(
	DOREPLIFETIME_CONDITION(AVehicleBase, DrivenVehicle, COND_OwnerOnly);
}

void AVehicleBase::OnRep_DrivenVehicle()
{
	if (DrivenVehicle != NULL)
	{
		// TODO: implement NotifyTeamChanged

		// since pawn doesn't have a PRI while driving, and may become initially relevant while driving,
		// we may only be able to ascertain the pawn's team (for team coloring, etc.) through its drivenvehicle
		//NotifyTeamChanged();
	}
}

/**	Change the Vehicle's base. Note: copied from Character.cpp */
void AVehicleBase::SetBase(AActor* NewBase, FVector NewFloor, USkeletalMeshComponent* SkelComp, const FName AttachName)
{
	ActorSetBase(this, NewBase, NewFloor, SkelComp, AttachName);
}

void AVehicleBase::BaseChange_Implementation()
{
	// Pawns can only set base to non-pawns, or pawns which specifically allow it.
	// Otherwise we do some damage and jump off.
	AActor* Base = GetAttachParentActor();
	if (Cast<APawn>(Base) != NULL && (DrivenVehicle == NULL || !DrivenVehicle->IsBasedOnActor(Base)))
	{
		if (!Cast<APawn>(Base)->CanBeBaseForCharacter(this))
		{
			// TODO: Implement crushed by
			//Cast<ACharacter>(Base)->CrushedBy(this);
			
			UCharacterMovementComponent* MovementComp = Cast<UCharacterMovementComponent>(GetMovementComponent());
			if (MovementComp != NULL)
			{
				MovementComp->JumpOff(Base);
			}
		}
	}

	// TODO: implement Kactor "jump-off"
	//// If it's a KActor, see if we can stand on it.
	//Dyn = DynamicSMActor(Base);
	//if (Dyn != None && !Dyn.CanBasePawn(self))
	//{
	//	JumpOffPawn();
	//}
}

void AVehicleBase::JumpOffPawn()
{
	// TODO: Implement generic jump off
}