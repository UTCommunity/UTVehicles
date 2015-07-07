#include "UTVehiclesPrivatePCH.h"
#include "UTVehicleWeapon.h"

AUTVehicleWeapon::AUTVehicleWeapon(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	
}

void AUTVehicleWeapon::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// TODO: Custom replication
	DOREPLIFETIME_CONDITION(AUTVehicleWeapon, SeatIndex, COND_OwnerOnly); // UC: (Role == ROLE_Authority && bNetInitial && bNetOwner)
	DOREPLIFETIME_CONDITION(AUTVehicleWeapon, MyVehicle, COND_OwnerOnly); // UC: (Role == ROLE_Authority && bNetInitial && bNetOwner)
}

void AUTVehicleWeapon::OnRep_Vehicle()
{
	// TODO: implement check for Deployable Vehicle
	/*AUTDeployableVehicle* DeployableVehicle = Cast<AUTDeployableVehicle>(MyVehicle);
	if (DeployableVehicle != NULL && DeployableVehicle.IsDeployed())
	{
		NotifyVehicleDeployed();
	}*/
}
