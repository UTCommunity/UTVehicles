#include "UTVehiclesPrivatePCH.h"
#include "UTWeaponPawn.h"
#include "UTVehicle.h"

// TODO: Use PlayerCameraManager for proper vehicle view calculation

AUTVehicle::AUTVehicle(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	MomentumMult = 2.f;

	bAllowedExit = true;

	bCanCarryFlag = false;
	bDriverHoldsFlag = false;

	SeatCameraScale = 1.0f;

	bNoZSmoothing = true;
	bLimitCameraZLookingUp = false;
	bNoFollowJumpZ = false;
	CameraSmoothingFactor = 2.0f;
	CameraLag = 0.12f;
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

		if (Seats[i].SeatMovementEffect != NULL)
		{
			SetMovementEffect(i, false);
		}
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
	DOREPLIFETIME(AUTVehicle, SeatMask);
	DOREPLIFETIME(AUTVehicle, bDeadVehicle);
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
			Seats[i].Gun = SeatPawn->CreateInventory<AUTVehicleWeapon>(Seats[i].GunClass);

			// TODO: Set EyeHeight
			//Seats[i].SeatPawn->EyeHeight = Seats[i].SeatPawn->BaseEyeHeight;

			SeatPawn->MyVehicleWeapon = Seats[i].Gun;
			SeatPawn->MyVehicle = this;
			SeatPawn->MySeatIndex = i;

			// set View pitch min and max
			SeatPawn->ViewPitchMin = Seats[i].ViewPitchMin != 0.0f ? Seats[i].ViewPitchMin : ViewPitchMin;
			SeatPawn->ViewPitchMax = Seats[i].ViewPitchMax != 0.0f ? Seats[i].ViewPitchMax : ViewPitchMax;
		}
		else
		{
			// TODO: Create gun with InventoryManager

			Seats[i].SeatPawn = this;
			Seats[i].Gun = CreateInventory<AUTVehicleWeapon>(Seats[i].GunClass);
		}

		Seats[i].SeatPawn->DriverDamageMult = Seats[i].DriverDamageMult;
		Seats[i].SeatPawn->bDriverIsVisible = Seats[i].bSeatVisible;

		if (Seats[i].Gun != NULL)
		{
			ActorSetBase(Seats[i].Gun, this);
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

void AUTVehicle::SetMovementEffect(int32 SeatIndex, bool bSetActive, AUTCharacter* UTChar)
{
	// TODO: Implement movement effects
}

void AUTVehicle::AttachDriver_Implementation(APawn* P)
{
	// reset vehicle camera
	OldPositions.Empty();
	
	// TODO: Check Eyeheight
	//Eyeheight = BaseEyeheight;

	AUTCharacter* UTChar = Cast<AUTCharacter>(P);
	if (UTChar != NULL)
	{
		//UTChar.SetWeaponAttachmentVisibility(false); // TODO: FIXME: Weapon attachement visibility // TODO: PR for SetWeaponAttachmentVisibility
		//UTChar.SetHandIKEnabled(false); // TODO: FIXME: Implement SetHandIKEnabled

		if (bAttachDriver)
		{
			//UTChar.SetCollision(false, false);
			//UTChar.bCollideWorld = false;
			UTChar->SetActorEnableCollision(false); // FIXME: Check if no-collision is properly set

			ActorSetBase(UTChar, NULL);
			//UTP.SetHardAttach(true);// TODO: FIXME: add hard attach
			UTChar->SetActorLocation(GetActorLocation());
			//UTChar->SetPhysics(PHYS_None); // TODO: FIXME: Set Physics state

			SitDriver(UTChar, 0);
		}
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
	bool bResult = Super::DriverLeave_Implementation(bForceLeave);

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
			BumpPawn = Seats[RequestedSeat].StoragePawn;
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

void AUTVehicle::SitDriver(AUTCharacter* UTChar, int32 SeatIndex)
{
	// SafeGuard
	if (UTChar == NULL || !Seats.IsValidIndex(SeatIndex))
	{
		return;
	}

	if (!Seats[SeatIndex].SeatBone.IsNone())
	{
		ActorSetBase(UTChar, this, FVector::ZeroVector, GetMesh(), Seats[SeatIndex].SeatBone);
	}
	else
	{
		ActorSetBase(UTChar, this);
	}
	
	SetMovementEffect(SeatIndex, true, UTChar);

	// TODO: Update physic asset
	/* UC:
	// Shut down physics when getting in vehicle.
	if (UTP.Mesh.PhysicsAssetInstance != None)
	{
		UTP.Mesh.PhysicsAssetInstance.SetAllBodiesFixed(TRUE);
	}
	UTP.Mesh.bUpdateKinematicBonesFromAnimation = FALSE;
	UTP.Mesh.PhysicsWeight = 0.0;
	*/

	if (Seats[SeatIndex].bSeatVisible)
	{
		// TODO: FIXME: Set shadow parent and light env
		/* UC:
		if ((UTP.Mesh != None) && (Mesh != None))
		{
			UTP.Mesh.SetShadowParent(Mesh);
			UTP.Mesh.SetLightEnvironment(LightEnvironment);
		}*/
		
		//UTChar->SetMeshVisibility(true); // TODO: PR for SetMeshVisibility
		UTChar->GetMesh()->SetVisibility(true, true); // FIXME: use SetMeshVisibility instead

		UTChar->SetActorRelativeLocation(Seats[SeatIndex].SeatOffset);
		UTChar->SetActorRelativeRotation(Seats[SeatIndex].SeatRotation);
		
		if (UTChar->GetMesh() != NULL)
		{
			UTChar->GetMesh()->SetCullDistance(5000.0f); // TODO: parameter / static fields for cull distance
		}

		// TODO: Hack needed. Legacy code. Remove hack?
		//HACK - don't let this translation get too large (for Liandri riding high)
		//float ClampedTranslation = Clamp(UTP.BaseTranslationOffset, -7.0, 7.0);
		//UTP.Mesh.SetTranslation(vect(0, 0, 1) * ClampedTranslation);

		UTChar->SetActorHiddenInGame(false);
	}
	else
	{
		UTChar->SetActorHiddenInGame(true);
	}
}

void AUTVehicle::OnRep_VehicleDied()
{
	// TODO: Handle bDeadVehicle replication
}

FVector AUTVehicle::GetCameraFocus(int SeatIndex)
{
	// SafeGuard // TODO: maybe return cached camera focus value
	if (!Seats.IsValidIndex(SeatIndex))
	{
		return FVector(0.0f);
	}

	FVector CamStart;

	//  calculate camera focus
	if (!bDeadVehicle && !Seats[SeatIndex].CameraTag.IsNone())
	{
		CamStart = Mesh->GetSocketLocation(Seats[SeatIndex].CameraTag);

		// Do a line check from actor location to this socket. If we hit the world, use that location instead.
		FVector HitLocation;
		FVector HitNormal;
		auto HitActor = Trace(HitLocation, HitNormal, CamStart, GetActorLocation(), false, FVector(12.0f));
		if (HitActor != NULL)
		{
			CamStart = HitLocation;
		}
	}
	else
	{
		CamStart = GetActorLocation();
	}

	CamStart += GetActorRotation().RotateVector(Seats[SeatIndex].CameraBaseOffset);

	//DrawDebugSphere(CamStart, 8, 10, 0, 255, 0, FALSE);
	//DrawDebugSphere(Location, 8, 10, 255, 255, 0, FALSE);
	
	return CamStart;
}

FVector AUTVehicle::GetCameraStart(int32 SeatIndex)
{
	// If we've already updated the cameraoffset, just return it
	int32 len = OldPositions.Num();
	if (len > 0 && SeatIndex == 0 && OldPositions[len - 1].Time == GetWorld()->GetTimeSeconds())
	{
		return CameraOffset + GetActorLocation();
	}

	FVector CamStart = GetCameraFocus(SeatIndex);
	float OriginalCamZ = CamStart.Z;
	if (CameraLag == 0 || SeatIndex != 0 || !IsControlled())
	{
		return CamStart;
	}

	// cache our current location
	FTimePosition NewPos, PrevPos;
	NewPos.Time = GetWorld()->GetTimeSeconds();
	NewPos.Position = CamStart;
	OldPositions.Add(NewPos);

	//// if no old locations saved, return offset
	if (len == 0)
	{
		CameraOffset = CamStart - GetActorLocation();
		return CamStart;
	}

	float DeltaTime = (len > 1) ? (GetWorld()->GetTimeSeconds() - OldPositions[len - 2].Time) : 0.0;

	len = OldPositions.Num();
	int32 ObsoleteCount = 0;
	for (int32 i = 0; i<len; i++)
	{
		if (OldPositions[i].Time < GetWorld()->GetTimeSeconds() - CameraLag)
		{
			PrevPos = OldPositions[i];
			ObsoleteCount++;
		}
		else
		{
			if (ObsoleteCount > 0)
			{
				// linear interpolation to maintain same distance in past
				if ((i == 0) || (OldPositions[i].Time - PrevPos.Time > 0.2))
				{
					CamStart = OldPositions[i].Position;
				}
				else
				{
					CamStart = PrevPos.Position + (OldPositions[i].Position - PrevPos.Position)*(GetWorld()->GetTimeSeconds() - CameraLag - PrevPos.Time) / (OldPositions[i].Time - PrevPos.Time);
				}
				if (ObsoleteCount > 1)
				{
					OldPositions.RemoveAt(0, ObsoleteCount - 1);
				}
			}
			else
			{
				CamStart = OldPositions[i].Position;
			}

			// need to smooth camera to vehicle distance, since vehicle update rate not synched with frame rate
			if (DeltaTime > 0)
			{
				DeltaTime *= CameraSmoothingFactor;
				CameraOffset = (CamStart - GetActorLocation())*DeltaTime + CameraOffset*(1 - DeltaTime);
				if (bNoZSmoothing)
				{
					// don't smooth z - want it bouncy
					CameraOffset.Z = CamStart.Z - GetActorLocation().Z;
				}
			}
			else
			{
				CameraOffset = CamStart - GetActorLocation();
			}

			CamStart = CameraOffset + GetActorLocation();
			if (bLimitCameraZLookingUp)
			{
				CamStart.Z = LimitCameraZ(CamStart.Z, OriginalCamZ, SeatIndex);
			}
			return CamStart;
		}
	}

	CamStart = OldPositions[len - 1].Position;
	if (bLimitCameraZLookingUp)
	{
		CamStart.Z = LimitCameraZ(CamStart.Z, OriginalCamZ, SeatIndex);
	}

	return CamStart;
}

float AUTVehicle::LimitCameraZ(float CurrentCamZ, float OriginalCamZ, int32 SeatIndex)
{
	if (Seats.IsValidIndex(SeatIndex))
	{
		FRotator CamRot = Seats[SeatIndex].SeatPawn->GetViewRotation();
		CamRot.Pitch = FRotator::ClampAxis(CamRot.Pitch);
		if ((CamRot.Pitch < 32768.0f))
		{
			float Pct = FMath::Clamp<float>(CamRot.Pitch*0.00025f, 0.0f, 1.0f);
			CurrentCamZ = OriginalCamZ*Pct + CurrentCamZ*(1.0 - Pct);
		}
	}

	return CurrentCamZ;
}

void AUTVehicle::CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult)
{
	VehicleCalcCamera(DeltaTime, 0, OutResult);
}

void AUTVehicle::VehicleCalcCamera(float DeltaTime, int32 SeatIndex, struct FMinimalViewInfo& OutResult, bool bPivotOnly)
{
	// TODO: Implement forcing mesh to be visible // TODO: Check if this is needed as we apply visibiltiy elsewhere in UpdateHiddenComponents
	/* UC
	Mesh.SetOwnerNoSee(false);
	if ((UTPawn(Driver) != None) && !Driver.bHidden && Driver.Mesh.bOwnerNoSee)
	{
		UTPawn(Driver).SetMeshVisibility(true);
	}
	*/

	// SafeGuard
	if (!Seats.IsValidIndex(SeatIndex) || Seats[SeatIndex].SeatPawn == NULL)
	{
		return;
	}

	// TODO: Implement fixed view
	/*
	// Handle the fixed view
	AUTCharacter* UTChar = Cast<AUTCharacter>(Seats[SeatIndex].SeatPawn->Driver);
	if (UTChar != NULL && UTChar->bFixedView)
	{
		OutResult.Location = UTChar->FixedViewLoc;
		OutResult.Rotation = UTChar->FixedViewRot;
		return;
	}*/

	FVector CamStart = GetCameraStart(SeatIndex);
	
	// Get the rotation
	if ((Seats[SeatIndex].SeatPawn->Controller != NULL) && !bSpectatedView)
	{
		OutResult.Rotation = Seats[SeatIndex].SeatPawn->GetViewRotation();
	}

	// TODO: Support debug free cam
	/*
	// support debug 3rd person cam
	if (P != None)
	{
		P.ModifyRotForDebugFreeCam(out_CamRot);
	}*/

	FRotationMatrix CamMat(OutResult.Rotation);
	FVector CamRotX, CamRotY, CamRotZ;
	CamMat.GetScaledAxes(CamRotX, CamRotY, CamRotZ);
	// TODO: Use Eyeheight instead of BaseEyeHeight
	CamStart += (Seats[SeatIndex].SeatPawn->BaseEyeHeight + LookForwardDist * FMath::Max<float>(0.0, (1.0 - CamRotZ.Z)))* CamRotZ;

	// if bNoFollowJumpZ, Z component of Camera position is fixed during a jump
	if (bNoFollowJumpZ)
	{
		float NewCamStartZ = CamStart.Z;
		if ((GetVelocity().Z > 0) && !HasWheelsOnGround() && (OldCamPosZ != 0))
		{
			// upward part of jump. Fix camera Z position.
			bFixedCamZ = true;
			if (OldPositions.Num() > 0)
			{
				OldPositions.Last().Position.Z += (OldCamPosZ - CamStart.Z);
			}
			CamStart.Z = OldCamPosZ;
			if (NewCamStartZ - CamStart.Z > 64)
			{
				CamStart.Z = NewCamStartZ - 64;
			}
		}
		else if (bFixedCamZ)
		{
			// Camera z position is being fixed, now descending
			if (HasWheelsOnGround() || (CamStart.Z <= OldCamPosZ))
			{
				// jump has ended
				if (DeltaTime >= 0.1f)
				{
					// all done
					bFixedCamZ = false;
				}
				else
				{
					// Smoothly return to normal camera mode.
					CamStart.Z = 10 * DeltaTime*CamStart.Z + (1 - 10 * DeltaTime)*OldCamPosZ;
					if (abs(NewCamStartZ - CamStart.Z) < 1.f)
					{
						bFixedCamZ = false;
					}
				}
			}
			else
			{
				// descending from jump, still in the air, so fix camera Z position
				if (OldPositions.Num() > 0)
				{
					OldPositions.Last().Position.Z += (OldCamPosZ - CamStart.Z);
				}
				CamStart.Z = OldCamPosZ;
			}
		}
	}

	// Trace up to the view point to make sure it's not obstructed.
	FVector SafeLocation;
	if (Seats[SeatIndex].CameraSafeOffset == FVector::ZeroVector)
	{
		SafeLocation = GetActorLocation();
	}
	else
	{
		FRotationMatrix Mat(GetActorRotation());
		FVector X, Y, Z;
		Mat.GetScaledAxes(X, Y, Z);
		SafeLocation = GetActorLocation() + Seats[SeatIndex].CameraSafeOffset.X * X + Seats[SeatIndex].CameraSafeOffset.Y * Y + Seats[SeatIndex].CameraSafeOffset.Z * Z;
	}

	// DrawDebugSphere(SafeLocation, 16, 10, 255, 0, 255, FALSE);
	// DrawDebugSphere(CamStart, 16, 10, 255, 255, 0, FALSE);

	bool bObstructed = false;
	FVector HitLocation, HitNormal;
	AActor* HitActor = Trace(HitLocation, HitNormal, CamStart, SafeLocation, false, FVector(12.0f));
	if (HitActor != NULL)
	{
		bObstructed = true;
		CamStart = HitLocation;
	}

	OldCamPosZ = CamStart.Z;
	if (bPivotOnly)
	{
		OutResult.Location = CamStart;
		return;
	}

	// Calculate the optimal camera position
	FVector CamDir = CamRotX * Seats[SeatIndex].CameraOffset * SeatCameraScale;

	// keep camera from going below vehicle
	if (!bRotateCameraUnderVehicle && (CamDir.Z < 0))
	{
		CamDir *= (CamDir.Size() - FMath::Abs(CamDir.Z)) / (CamDir.Size() + FMath::Abs(CamDir.Z));
	}

	FVector CamPos = CamStart + CamDir;

	// Adjust for obstructions
	HitActor = Trace(HitLocation, HitNormal, CamPos, CamStart, false, FVector(12.0f));
	if (HitActor != NULL)
	{
		OutResult.Location = HitLocation;
		bObstructed = true;
	}
	else
	{
		OutResult.Location = CamPos;
	}

	// TODO: Replace Mesh with proper CollisionComponent in TraceComponent
	bool bInsideVehicle = false;
	FVector FirstHitLocation;
	if (!bRotateCameraUnderVehicle && (CamDir.Z < 0) && TraceComponent(FirstHitLocation, HitNormal, Mesh, OutResult.Location, CamStart, FVector(0.f)))
	{
		// going through vehicle - it's ok if outside collision on other side
		if (!TraceComponent(HitLocation, HitNormal, Mesh, CamStart, OutResult.Location, FVector(0.0f)))
		{
			// end point is inside collision - that's bad
			OutResult.Location = FirstHitLocation;
			bObstructed = true;
			bInsideVehicle = true;
		}
	}

	// if trace doesn't hit collisioncomponent going back in, it means we are inside the collision box
	// in which case we want to hide the vehicle
	if (!bCameraNeverHidesVehicle && bObstructed)
	{
		bInsideVehicle = bInsideVehicle
			|| !TraceComponent(HitLocation, HitNormal, Mesh, SafeLocation, OutResult.Location, FVector(0.0f))
			|| ((HitLocation - OutResult.Location).SizeSquared() < MinCameraDistSq);

		// TODO: Implement forcing mesh to be visible // TODO: Check if this is needed as we apply visibility elsewhere in UpdateHiddenComponents
		/* UC
		Mesh.SetOwnerNoSee(bInsideVehicle);
		if ((UTPawn(Driver) != None) && !Driver.bHidden && (Driver.Mesh.bOwnerNoSee != Mesh.bOwnerNoSee))
		{
			// Handle the main player mesh
			Driver.Mesh.SetOwnerNoSee(Mesh.bOwnerNoSee);
		}
		*/
	}
}