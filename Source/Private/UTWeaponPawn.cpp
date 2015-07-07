#include "UTVehiclesPrivatePCH.h"
#include "UTWeaponPawn.h"

AUTWeaponPawn::AUTWeaponPawn(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	MySeatIndex = INDEX_NONE;
}

void AUTWeaponPawn::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUTWeaponPawn, MyVehicleWeapon);
	DOREPLIFETIME(AUTWeaponPawn, MyVehicle);
	DOREPLIFETIME(AUTWeaponPawn, MySeatIndex);
}

void AUTWeaponPawn::OnRep_VehicleWeapon()
{
	TryToSetupSeat();
}

void AUTWeaponPawn::OnRep_Vehicle()
{
	TryToSetupSeat();
}

void AUTWeaponPawn::OnRep_SeatIndex()
{
	TryToSetupSeat();
}

void AUTWeaponPawn::TryToSetupSeat()
{
	if (MySeatIndex > 0 && MyVehicle != NULL && MySeatIndex < MyVehicle->Seats.Num())
	{
		MyVehicle->Seats[MySeatIndex].SeatPawn = this;
		MyVehicle->Seats[MySeatIndex].Gun = MyVehicleWeapon;
		SetBase(MyVehicle);
	}
}