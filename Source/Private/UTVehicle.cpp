#include "UTVehiclesPrivatePCH.h"
#include "UTWeaponPawn.h"
#include "AAATempUTVehicleWeapon.h"
#include "UTVehicle.h"

AUTVehicle::AUTVehicle(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	MomentumMult = 2.f;

	bAllowedExit = true;

	bCanCarryFlag = false;
	bDriverHoldsFlag = false;
}

void AUTVehicle::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		// Setup the Seats array
		InitializeSeats();
	}
	else if (Seats.Num() > 0)
	{
		// Insure our reference to self is always setup
		Seats[0].SeatPawn = this;
	}

	// add to local HUD's post-rendered list
	if (GetWorld()->GetNetMode() != NM_DedicatedServer)
	{
		APlayerController* PC = GEngine->GetFirstLocalPlayerController(GetWorld());
		if (PC != NULL && PC->MyHUD != NULL)
		{
			PC->MyHUD->AddPostRenderedActor(this);
		}
	}
}

void AUTVehicle::Destroyed()
{
	for (int32 i = 1; i<Seats.Num(); i++)
	{
		if (Seats[i].SeatPawn != NULL)
		{
			if (Seats[i].SeatPawn->Controller != NULL)
			{
				UE_LOG(UTVehicles, Warning, TEXT("%s destroying seat %i still controlled by %s %s"), *GetName(), i, *Seats[i].SeatPawn->Controller->GetName(), *Seats[i].SeatPawn->Controller->GetHumanReadableName());
			}
			Seats[i].SeatPawn->Destroy();
		}

		// TODO: Implement Movement effects
		/*if (Seats[i].SeatMovementEffect != NULL)
		{
			SetMovementEffect(i, false);
		}*/
	}

	Super::Destroyed();

	// remove from local HUD's post-rendered list
	if (GetWorld()->GetNetMode() != NM_DedicatedServer && GEngine->GetWorldContextFromWorld(GetWorld()) != NULL) // might not be able to get world context when exiting PIE
	{
		APlayerController* PC = GEngine->GetFirstLocalPlayerController(GetWorld());
		if (PC != NULL && PC->MyHUD != NULL)
		{
			PC->MyHUD->RemovePostRenderedActor(this);
		}
	}
}

void AUTVehicle::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AUTVehicle, PassengerState);
	DOREPLIFETIME(AUTVehicle, SeatMask)
}

void AUTVehicle::InitializeSeats()
{
	if (Seats.Num() == 0)
	{
		UE_LOG(UTVehicles, Warning, TEXT("Vehicle (%s) **MUST** have at least one seat defined"), *GetName());
		Destroy();
		return;
	}

	for (int32 i = 0; i<Seats.Num(); i++)
	{
		// Seat 0 = Driver Seat. It doesn't get a WeaponPawn

		if (i>0)
		{
			AUTWeaponPawn* SeatPawn = GetWorld()->SpawnActor<AUTWeaponPawn>(AUTWeaponPawn::StaticClass());
			if (SeatPawn == NULL)
			{
				continue;
			}

			Seats[i].SeatPawn = SeatPawn;
			SeatPawn->SetBase(this);

			AUTVehicleWeapon* Gun = SeatPawn->CreateInventory<AUTVehicleWeapon>(Seats[i].GunClass);
			if (Gun == NULL)
			{
				continue;
			}

			Seats[i].Gun = Gun;

			// TODO: implement SetBase for UTVehicleWeapon
			//Gun->SetBase(this);
	
			// TODO: Set EyeHeight
			//Seats[i].SeatPawn->EyeHeight = Seats[i].SeatPawn->BaseEyeHeight;

			SeatPawn->MyVehicleWeapon = Seats[i].Gun;
			SeatPawn->MyVehicle = this;
			SeatPawn->MySeatIndex = i;

			// TODO: Implement ViewPitchMin
	//		if (Seats[i].ViewPitchMin != 0.0f)
	//		{
	//			SeatPawn->ViewPitchMin = Seats[i].ViewPitchMin;
	//		}
	//		else
	//		{
	//			SeatPawn->ViewPitchMin = ViewPitchMin;
	//		}

			// TODO: Implement ViewPitchMax
	//		if (Seats[i].ViewPitchMax != 0.0f)
	//		{
	//			SeatPawn->ViewPitchMax = Seats[i].ViewPitchMax;
	//		}
	//		else
	//		{
	//			SeatPawn->ViewPitchMax = ViewPitchMax;
	//		}
		}
		else
		{
			// TODO: Create gun with InventoryManager

			Seats[i].SeatPawn = this;
			AUTVehicleWeapon* Gun = CreateInventory<AUTVehicleWeapon>(Seats[i].GunClass);
			if (Gun == NULL)
			{
				continue;
			}
			
			Seats[i].Gun = Gun;
			
			// TODO: implement SetBase for UTVehicleWeapon
			//Gun->SetBase(this);
		}

		Seats[i].SeatPawn->DriverDamageMult = Seats[i].DriverDamageMult;
		Seats[i].SeatPawn->bDriverIsVisible = Seats[i].bSeatVisible;

		if (Seats[i].Gun != NULL)
		{
			Seats[i].Gun->SeatIndex = i;
			Seats[i].Gun->MyVehicle = this;
		}

		// Cache the names used to access various variables
	}
}

void AUTVehicle::SetSeatStoragePawn(int32 SeatIndex, APawn* PawnToSit)
{
	// SafeGuard
	if (!Seats.IsValidIndex(SeatIndex))
	{
		UE_LOG(UTVehicles, Warning, TEXT("%s::SetSeatStoragePawn() called with in invalid index of %i"), *GetName(), SeatIndex);
		return;
	}

	Seats[SeatIndex].StoragePawn = PawnToSit;
	if ((SeatIndex == 1) && (Role == ROLE_Authority))
	{
		if (PawnToSit == NULL || Seats[SeatIndex].SeatPawn == NULL)
		{
			PassengerState = NULL;
		}
		else
		{
			PassengerState = Cast<AUTPlayerState>(Seats[SeatIndex].SeatPawn->PlayerState);
		}
	}

	int32 Mask = 1 << SeatIndex;
	if (PawnToSit != NULL)
	{
		SeatMask = SeatMask | Mask;
	}
	else if ((SeatMask & Mask) > 0)
	{
		SeatMask = SeatMask ^ Mask;
	}
}

bool AUTVehicle::DriverEnter_Implementation(APawn* P)
{
	// SafeGuards
	if (P == NULL)
	{
		return false;
	}

	// TODO: Implement stop firing player weapon on entering
	//P->StopFiring();

	if (Seats[0].Gun != NULL)
	{
		// TODO: Implement set current weapon
		//InvManager.SetCurrentWeapon(Seats[0].Gun);
	}

	Instigator = this;

	if (!Super::DriverEnter_Implementation(P))
	{
		return false;
	}

	HandleEnteringFlag(Cast<AUTPlayerState>(PlayerState));

	SetSeatStoragePawn(0, P);

	// TODO: Implement event triggering for Level Blueprint
	/* UC
	if (ParentFactory != None)
	{
		ParentFactory.TriggerEventClass(class'UTSeqEvent_VehicleFactory', None, 3);
	}*/

	if (P->IsControlled()) // UC: PlayerController(Controller) != None
	{
		// TODO: set VehicleLostTime
		//VehicleLostTime = 0;
	}

	// TODO: Implement flags for resetting
	//StuckCount = 0;
	//ResetTime = WorldInfo.TimeSeconds - 1;

	bHasBeenDriven = true;

	// TODO: implement check for key vehicle
	/*if (bKeyVehicle || (Cast<UTDeployableVehicle>(this) != NULL))
	{
		// notify any players that have this as their objective
		for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
		{
			AUTPlayerController* PC = Cast<AUTPlayerController>(*Iterator);
			if (PC != NULL && C.LastAutoObjective == this)
			{
				// TODO: implement CheckAutoObjective
				PC->CheckAutoObjective(true);
			}
		}
	}*/

	return true;
}

bool AUTVehicle::DriverLeave_Implementation(bool bForceLeave)
{
	if (!bForceLeave && !bAllowedExit)
	{
		return false;
	}

	auto OldDriver = Driver;
	bool bResult = Super::DriverLeave(bForceLeave);

	if (bResult)
	{
		SetSeatStoragePawn(0, NULL);

		// set Instigator to old driver so if vehicle continues on and runs someone over, the appropriate credit is given
		Instigator = OldDriver;
		
		// TODO: Implement event triggering for Level Blueprint
		/* UC
		if (ParentFactory != None)
		{
			ParentFactory.TriggerEventClass(class'UTSeqEvent_VehicleFactory', None, 4);
		}*/
	}

	return bResult;
}

bool AUTVehicle::PassengerEnter(APawn* P, int32 SeatIndex)
{
	// SafeGuard
	if (Role != ROLE_Authority || P == NULL)
	{
		return false;
	}

	// Restrict someone not on the same team
	{
		auto Game = GetWorld()->GetAuthGameMode<AUTGameMode>();
		auto GS = GetWorld()->GetGameState<AUTGameState>();
		if (Game != NULL && GS != NULL && Game->bTeamGame && !GS->OnSameTeam(P, this))
		{
			return false;
		}
	}

	if (!Seats.IsValidIndex(SeatIndex))
	{
		UE_LOG(UTVehicles, Warning, TEXT("Attempted to add a passenger to unavailable passenger seat %i"), SeatIndex);
		return false;
	}

	if (Seats[SeatIndex].SeatPawn == NULL || !Seats[SeatIndex].SeatPawn->DriverEnter(P))
	{
		return false;
	}

	HandleEnteringFlag(Cast<AUTPlayerState>(Seats[SeatIndex].SeatPawn->PlayerState));

	SetSeatStoragePawn(SeatIndex, P);

	bHasBeenDriven = true;
	return true;
}

void AUTVehicle::PassengerLeave(int32 SeatIndex)
{
	if (Role == ROLE_Authority)
	{
		SetSeatStoragePawn(SeatIndex, NULL);
	}
}

/** handles dealing with any flag the given driver/passenger may be holding */
void AUTVehicle::HandleEnteringFlag(AUTPlayerState* EnteringPRI)
{
	if (EnteringPRI != NULL)
	{
		AUTCarriedObject* Flag = EnteringPRI->CarriedObject;
		if (Flag != NULL) // UC: EnteringPRI.bHasFlag
		{
			if (!bCanCarryFlag)
			{
				Flag->Drop();
			}
			else if (!bDriverHoldsFlag)
			{
				AttachFlag(Flag, Driver);
			}
		}
	}
}

void AUTVehicle::AttachFlag(AUTCarriedObject* FlagActor, APawn* NewDriver)
{
	// TODO: implement attaching flag
}

bool AUTVehicle::IsDriverSeat(AVehicle* TestSeatPawn)
{
	return (Role == ROLE_Authority && Seats[0].SeatPawn == TestSeatPawn);
}

bool AUTVehicle::SeatAvailable(int32 SeatIndex)
{
	return Role == ROLE_Authority && (Seats[SeatIndex].SeatPawn == NULL || Seats[SeatIndex].SeatPawn->Controller == NULL);
}

bool AUTVehicle::AnySeatAvailable_Implementation()
{
	if (Role != ROLE_Authority)
		return false;

	for (int32 i = 0; i<Seats.Num(); i++)
	{
		if ((Seats[i].SeatPawn != NULL) && (Seats[i].SeatPawn->Controller == NULL))
		{
			return true;
		}
	}
	return false;
}

int32 AUTVehicle::GetSeatIndexForController(AController* ControllerToMove)
{
	for (int32 i = 0; i<Seats.Num(); i++)
	{
		if (Seats[i].SeatPawn != NULL && Seats[i].SeatPawn->Controller != NULL && Seats[i].SeatPawn->Controller == ControllerToMove)
		{
			return i;
		}
	}

	return -1;
}

AController* AUTVehicle::GetControllerForSeatIndex(int32 SeatIndex)
{
	//SafeGuard
	if (Role != ROLE_Authority || !Seats.IsValidIndex(SeatIndex) || Seats[SeatIndex].SeatPawn == NULL)
		return NULL;

	return Seats[SeatIndex].SeatPawn->Controller;
}

APlayerState* AUTVehicle::GetSeatPlayerState(int32 SeatNum)
{
	if (Role != ROLE_Authority)
	{
		return (SeatNum == 0) ? PlayerState : PassengerState;
		
	}
	else if (Seats.IsValidIndex(SeatNum) && Seats[SeatNum].SeatPawn != NULL)
	{
		return Seats[SeatNum].SeatPawn->PlayerState;
	}

	return NULL;
}

bool AUTVehicle::CanEnterVehicle_Implementation(APawn* P)
{
	// SafeGuard checks
	if (P == NULL || Health <= 0 || IsPendingKillPending() || P->Controller == NULL)
	{
		return false;
	}

	// TODO: Implement check Is-player flag
	//if (!P.Controller->bIsPlayer)
	//{
	//	return false;
	//}

	AUTCharacter* Char = Cast<AUTCharacter>(P);
	if (Char != NULL && (Char->bIsCrouched || Char->DrivenVehicle != NULL))
	{
		return false;
	}

	// TODO: Implement check for Hero/Titan
	//if (UTP != NULL && UTP.IsHero())
	//{
	//	return false;;
	//}

	// check for available seat, and no enemies in vehicle
	// allow humans to enter if full but with bots (TryToDrive() will kick one out if possible)
	bool bIsHuman = IsControlled(); // UC: P.IsHumanControlled()
	bool bSeatAvailable = false;
	for (int32 i = 0; i<Seats.Num(); i++)
	{
		APlayerState* SeatState = GetSeatPlayerState(i);
		if (SeatState == NULL)
		{
			bSeatAvailable = true;
		}
		else if (GetWorld()->GetGameState<AUTGameState>() != NULL && !GetWorld()->GetGameState<AUTGameState>()->OnSameTeam(P, SeatState))
		{
			return false;
		}
		else if (bIsHuman && SeatState->bIsABot)
		{
			bSeatAvailable = true;
		}
	}

	return bSeatAvailable;
}

bool AUTVehicle::TryToDrive_Implementation(APawn* NewDriver)
{
	// TODO: Implement TryToDrive for UTVehicle
	return Super::TryToDrive_Implementation(NewDriver);
}

bool AUTVehicle::ServerAdjacentSeat_Validate(int32 Direction, AController* C)
{
	return true;
}

void AUTVehicle::ServerAdjacentSeat_Implementation(int32 Direction, AController* C)
{
	int32 CurrentSeat = GetSeatIndexForController(C);
	if (CurrentSeat != INDEX_NONE)
	{
		int32 NewSeat = CurrentSeat;
		do
		{
			NewSeat += Direction;
			if (NewSeat < 0)
			{
				NewSeat = Seats.Num() - 1;
			}
			else if (NewSeat == Seats.Num())
			{
				NewSeat = 0;
			}
			if (NewSeat == CurrentSeat)
			{
				// no available seat
				if (auto PC = Cast<APlayerController>(C))
				{
					PC->ClientPlaySound(VehicleLockedSound);
				}

				return;
			}
		} while (!SeatAvailable(NewSeat) && (Seats[NewSeat].SeatPawn == NULL || Cast<AUTBot>(Seats[NewSeat].SeatPawn->Controller) == NULL));

		// change to the seat we found
		ChangeSeat(C, NewSeat);
	}
}

bool AUTVehicle::ServerChangeSeat_Validate(int32 RequestedSeat)
{
	return true;
}

void AUTVehicle::ServerChangeSeat_Implementation(int32 RequestedSeat)
{
	if (RequestedSeat == -1)
	{
		DriverLeave(false);
	}
	else
	{
		ChangeSeat(Controller, RequestedSeat);
	}
}

bool AUTVehicle::HasPriority(AController* First, AController* Second)
{
	if (First != Second && Cast<APlayerController>(First) != NULL && Cast<APlayerController>(Second) == NULL)
	{
		return true;
	}

	return false;
}

bool AUTVehicle::ChangeSeat(AController* ControllerToMove, int32 RequestedSeat)
{
	// Make sure we are looking to switch to a valid seat
	if (!Seats.IsValidIndex(RequestedSeat))
	{
		return false;
	}

	// get the seat index of the pawn looking to move.
	int32 OldSeatIndex = GetSeatIndexForController(ControllerToMove);
	if (OldSeatIndex == -1)
	{
		// Couldn't Find the controller, should never happen
		UE_LOG(UTVehicles, Warning, TEXT("Attempted to switch %s to a seat in %s when he is not already in the vehicle"), *ControllerToMove->GetName(), *GetName());
		return false;
	}

	// If someone is in the seat, see if we can bump him
	APawn* BumpPawn = NULL;
	if (!SeatAvailable(RequestedSeat))
	{
		// Get the Seat holder's controller and check it for Priority
		AController* BumpController = GetControllerForSeatIndex(RequestedSeat);
		if (BumpController == NULL)
		{
			UE_LOG(UTVehicles, Warning, TEXT("Attempted to bump a phantom Controller in seat in %i (%s)"), RequestedSeat, (Seats[RequestedSeat].SeatPawn != NULL ? *Seats[RequestedSeat].SeatPawn->GetName() : TEXT("NO VEHICLE")));
			return false;
		}

		if (!HasPriority(ControllerToMove, BumpController))
		{
			// Nope, same or great priority on the seat holder, deny the move
			if (auto PC = Cast<APlayerController>(ControllerToMove))
			{
				PC->ClientPlaySound(VehicleLockedSound);
			}
			return false;
		}

		// If we are bumping someone, free their seat.
		if (BumpController != NULL)
		{
			APawn* BumpPawn = Seats[RequestedSeat].StoragePawn;
			if (Seats[RequestedSeat].SeatPawn != NULL)
			{
				Seats[RequestedSeat].SeatPawn->DriverLeave(true);
			}

			// Handle if we bump the driver
			if (RequestedSeat == 0)
			{
				// Reset the controller's AI if needed
				// TODO: Implement AI logic for changing seat
				/*if (BumpController->RouteGoal == this)
				{
					BumpController->RouteGoal = NULL;
				}
				if (BumpController->MoveTarget == this)
				{
					BumpController->MoveTarget = NULL;
				}*/
			}
		}
	}

	APawn* OldPawn = Seats[OldSeatIndex].StoragePawn;

	// Leave the current seat and take over the new one
	if (Seats[OldSeatIndex].SeatPawn != NULL)
	{
		Seats[OldSeatIndex].SeatPawn->DriverLeave(true);
	}

	if (OldSeatIndex == 0)
	{
		// Reset the controller's AI if needed
		// TODO: Implement AI logic for changing seat
		/*if (ControllerToMove->RouteGoal == this)
		{
			ControllerToMove->RouteGoal = NULL;
		}
		if (ControllerToMove->MoveTarget == this)
		{
			ControllerToMove->MoveTarget = NULL;
		}*/
	}

	if (RequestedSeat == 0)
	{
		DriverEnter(OldPawn);
	}
	else
	{
		PassengerEnter(OldPawn, RequestedSeat);
	}

	// If we had to bump a pawn, seat them in this controller's old seat.
	if (BumpPawn != NULL)
	{
		if (OldSeatIndex == 0)
		{
			DriverEnter(BumpPawn);
		}
		else
		{
			PassengerEnter(BumpPawn, OldSeatIndex);
		}
	}

	return true;
}