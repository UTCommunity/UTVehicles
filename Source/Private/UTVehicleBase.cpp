#include "UTVehiclesPrivatePCH.h"
#include "UTVehicleBase.h"

AUTVehicleBase::AUTVehicleBase(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	
}

void AUTVehicleBase::SwitchWeapon(int32 Group)
{
	ServerChangeSeat(Group - 1);
}

void AUTVehicleBase::SwitchWeaponGroup(int32 Group)
{
	ServerChangeSeat(Group - 1);
}

void AUTVehicleBase::AdjacentSeat(int32 Direction, AController* C)
{
	ServerAdjacentSeat(Direction, C);
}

bool AUTVehicleBase::ServerAdjacentSeat_Validate(int32 Direction, AController* C)
{
	return true;
}

void AUTVehicleBase::ServerAdjacentSeat_Implementation(int32 Direction, AController* C)
{
	// overridden in subclass
}

bool AUTVehicleBase::ServerChangeSeat_Validate(int32 RequestedSeat)
{
	return true;
}

void AUTVehicleBase::ServerChangeSeat_Implementation(int32 RequestedSeat)
{
	// overridden in subclass
}