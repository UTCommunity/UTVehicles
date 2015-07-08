#include "UTVehiclesPrivatePCH.h"
#include "UTVehicle.h"
#include "UTWeaponPawn.h"

AUTWeaponPawn::AUTWeaponPawn(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bOnlyRelevantToOwner = true;

	BaseEyeHeight = 180.0f;
	// Eyeheight = 180 // TODO: Check Eyeheight

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

void AUTWeaponPawn::DisplayDebug(class UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	UFont* RenderFont = GEngine->GetSmallFont();

	Canvas->DrawText(RenderFont, FString::Printf(TEXT("[WeaponPawn]")), 4.0f, YPos);
	YPos += YL;
	Canvas->DrawText(RenderFont, FString::Printf(TEXT("Owner: %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("NONE")), 4.0f, YPos);
	YPos += YL;
	Canvas->DrawText(RenderFont, FString::Printf(TEXT("Vehicle: %s %s"), MyVehicleWeapon != NULL ? *MyVehicleWeapon->GetName() : TEXT("NONE"), MyVehicle != NULL ? *MyVehicle->GetName() : TEXT("NONE")), 4.0f, YPos);
	YPos += YL;
	Canvas->DrawText(RenderFont, FString::Printf(TEXT("Rotation/Location: %s %s"), *GetActorRotation().ToString(), *GetActorLocation().ToString()), 4.0f, YPos);
	YPos += YL;

	if (MyVehicle != NULL)
	{
		MyVehicle->DisplayDebug(Canvas, DebugDisplay, YL, YPos);
	}
}

void AUTWeaponPawn::TryToSetupSeat()
{
	if (MyVehicle != NULL && MyVehicle->Seats.IsValidIndex(MySeatIndex))
	{
		MyVehicle->Seats[MySeatIndex].SeatPawn = this;
		MyVehicle->Seats[MySeatIndex].Gun = MyVehicleWeapon;
		SetBase(MyVehicle);
	}
}

void AUTWeaponPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (Role == ROLE_Authority)
	{
		if (MyVehicleWeapon)
		{
			// TODO: Implement ClientWeaponSet
			//MyVehicleWeapon->ClientWeaponSet(false);
		}

		RecalculateBaseEyeHeight();

		// TODO: Check the use of EyeHeight and BaseEyeheight
		//Eyeheight = BaseEyeheight;
	}
}

/**
* Called when the driver leaves the WeaponPawn.
* We forward it along as a PassengerLeave call to the controlling vehicle.
*/
void AUTWeaponPawn::DriverLeft()
{
	Super::DriverLeft();

	if (Role == ROLE_Authority && MyVehicle != NULL)
	{
		MyVehicle->PassengerLeave(MySeatIndex);
	}
}

void AUTWeaponPawn::ServerAdjacentSeat_Implementation(int32 Direction, AController* C)
{
	if (MyVehicle != NULL)
	{
		MyVehicle->ServerAdjacentSeat(Direction, C);
	}
}

void AUTWeaponPawn::ServerChangeSeat_Implementation(int32 RequestedSeat)
{
	if (MyVehicle != NULL)
	{
		MyVehicle->ChangeSeat(Controller, RequestedSeat);
	}
}

bool AUTWeaponPawn::PlaceExitingDriver(APawn* ExitingDriver)
{
	if (Role == ROLE_Authority)
	{
		if (ExitingDriver == NULL)
		{
			ExitingDriver = Driver;
		}

		if (MyVehicle != NULL)
		{
			return MyVehicle->PlaceExitingDriver(ExitingDriver);
		}
	}

	return false;
}

// TODO: Override methods. To stubs
//function DropToGround() {}
//function AddVelocity(vector NewVelocity, vector HitLocation, class<DamageType> damageType, optional TraceHitInfo HitInfo) {}
void AUTWeaponPawn::JumpOffPawn() {}
void AUTWeaponPawn::BaseChange_Implementation() {}
//function SetMovementPhysics() {}

/**
* Set the Base Eye height.  We override it here to pull it from the CameraEyeHeight variable of in the seat array
*/
void AUTWeaponPawn::RecalculateBaseEyeHeight()
{
	if (MyVehicle != NULL && MyVehicle->Seats.IsValidIndex(MySeatIndex))
	{
		BaseEyeHeight = MyVehicle->Seats[MySeatIndex].CameraEyeHeight;
	}
}

void AUTWeaponPawn::AttachDriver_Implementation(APawn* P)
{
	AUTCharacter* Char = Cast<AUTCharacter>(P);
	if (Char != NULL)
	{
		// UTP.SetWeaponAttachmentVisibility(false); // TODO: FIXME: Weapon attachement visibility // TODO: PR for SetWeaponAttachmentVisibility

		if (MyVehicle->bAttachDriver)
		{
			// UC: UTP.SetCollision(false, false);
			// UC: UTP.bCollideWorld = false;
			Char->SetActorEnableCollision(false);

			//UTP.SetHardAttach(true);  // TODO: FIXME: add hard attach
			Char->SetActorLocation(GetActorLocation());
			//UTP.SetPhysics(PHYS_None); // TODO: FIXME: Set Physics state

			MyVehicle->SitDriver(Char, MySeatIndex);
		}
	}
}