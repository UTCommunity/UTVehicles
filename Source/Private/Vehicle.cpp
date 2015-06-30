#include "UTVehiclesPrivatePCH.h"
#include "Vehicle.h"
#include "AAAUTCharacter.h"
#include "AAAUTGameMode.h"

DEFINE_LOG_CATEGORY_STATIC(LogUTCharacter, Log, All);

#define AUTCharacter AAAAUTCharacter
#define AUTGameMode AAAAUTGameMode

AVehicle::AVehicle(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	AutoPossessPlayer = EAutoReceiveInput::Disabled;

	MomentumMult = 1.0f;
	bAttachDriver = true;

	Health = 100;
}

void AVehicle::PreInitializeComponents()
{
	// important that this comes before Super so mutators can modify it
	if (HealthMax == 0)
	{
		HealthMax = GetDefault<AVehicle>()->Health;
	}

	Super::PreInitializeComponents();
}

void AVehicle::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AVehicle, Health, COND_None); // would be nice to bind to teammates and spectators, but that's not an option :(

	DOREPLIFETIME_CONDITION(AVehicle, bDriving, COND_None); // UC: bNetDirty && Role == ROLE_Authority
	DOREPLIFETIME_CONDITION(AVehicle, Driver, COND_OwnerOnly); // UC: bNetDirty && (bNetOwner || Driver == None || !Driver.bHidden) && Role == ROLE_Authority
}

void AVehicle::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	DOREPLIFETIME_ACTIVE_OVERRIDE(AVehicle, Driver, Driver == NULL || !Driver->bHidden);
}

void AVehicle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AVehicle::OnRep_Driver()
{
	if ((PlayerState != NULL) && (Driver != NULL))
	{
		Driver->NotifyTeamChanged();
	}
}

void AVehicle::OnRep_DrivingChanged()
{
	DrivingStatusChanged();
}

void AVehicle::DisplayDebug(class UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	UFont* RenderFont = GEngine->GetSmallFont();
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	Canvas->SetDrawColor(255, 255, 255);
	YL = Canvas->DrawText(RenderFont, FString::Printf(TEXT("Steering %f throttle %f rise %f"), Steering, Throttle, Rise), 4.0f, YPos);
	YPos += YL;

	Canvas->SetDrawColor(255, 0, 0);
	if (Driver == NULL)
	{
		YL = Canvas->DrawText(RenderFont, TEXT("NO DRIVER"), 4.0f, YPos);
	}
	else if (Driver->GetMesh() == NULL)
	{
		YL = Canvas->DrawText(RenderFont, TEXT("NO MESH"), 4.0f, YPos);
	}
	else
	{
		YL = Canvas->DrawText(RenderFont, FString::Printf(TEXT("Driver Mesh %s hidden %s"), *(Driver->GetMesh()->GetName()), Driver->bHidden), 4.0f, YPos);
	}
	YPos += YL;
}

bool AVehicle::CanBeBaseForCharacter(class APawn* APawn) const
{
	return true;
}

void AVehicle::SetInputs(float InForward, float InStrafe, float InUp)
{
	Throttle = InForward;
	Steering = InStrafe;
	Rise = InUp;
}

void AVehicle::PossessedBy(AController* NewController) //  TODO: Add VehicleTransition ==> PossessedBy(AController* NewController, bool bVehicleTransition)
{
	Super::PossessedBy(NewController);

	EntryAnnouncement(NewController);
	NetPriority = 3.0f;
	NetUpdateFrequency = 100.0f;

	// TODO: Implement support for AI
	//ThrottleTime = WorldInfo.TimeSeconds;
	//OnlySteeringStartTime = WorldInfo.TimeSeconds;
}

void AVehicle::UnPossessed()
{
	// restore original netpriority changed when possessing
	NetPriority = GetDefault<AVehicle>()->NetPriority;;
	NetUpdateFrequency = 8;
	ForceNetUpdate();

	Super::UnPossessed();
}

void AVehicle::EntryAnnouncement(AController* NewController)
{
}

bool AVehicle::PlayerSuicideInternal()
{
	if (Role == ROLE_Authority && Driver != NULL)
	{
		Driver->PlayerSuicide();
		return true;
	}

	return false;
}

void AVehicle::AttachDriver_Implementation(APawn* P)
{
	if (!bAttachDriver)
		return;

	P->SetActorEnableCollision(false);
	P->DetachRootComponentFromParent(true); // UC: P.SetBase(none);
	//P.SetHardAttach(true); // TODO: FIXME: add hard attach
	//P.SetPhysics(PHYS_None); // TODO: FIXME: Set Physics state

	//if ((P.Mesh != None) && (Mesh != None)) // TODO: FIXME: Set shadow parent
	//	P.Mesh.SetShadowParent(Mesh);

	if (!bDriverIsVisible)
	{
		P->SetActorHiddenInGame(true);
		P->SetActorLocation(GetActorLocation());
	}
	P->AttachRootComponentToActor(this); // UC: P.SetBase(self);
	// need to set PHYS_None again, because SetBase() changes physics to PHYS_Falling
	//P.SetPhysics(PHYS_None); // TODO: FIXME: Set Physics state
}

void AVehicle::DetachDriver_Implementation(APawn* NewDriver)
{
}

bool AVehicle::CanEnterVehicle(APawn* P)
{
	// SafeGuard checks
	if (P == NULL || Role != ROLE_Authority || IsPendingKillPending() || Health <= 0)
	{
		return false;
	}

	// Check if player isn't driving a vehicle already
	// TODO: Implement Driver to be non-UTCharacter type
	AUTCharacter* Char = Cast<AUTCharacter>(P);
	if (Char != NULL && (!bAttachDriver || !Char->bIsCrouched) && Char->DrivenVehicle == NULL && !P->IsA(AVehicle::StaticClass()) && AnySeatAvailable())
	{
		return true; // FIXME: Check for Is-Player flag; UC: P->Controller != NULL && P->Controller->bIsPlayer;
	}

	return false;
}

bool AVehicle::TryToDrive_Implementation(APawn* NewDriver)
{
	if (Role != ROLE_Authority || !CanEnterVehicle(NewDriver))
	{
		return false;
	}

	return DriverEnter(NewDriver);
}

bool AVehicle::AnySeatAvailable_Implementation()
{
	return (Driver == NULL);
}

bool AVehicle::DriverEnter_Implementation(APawn* P)
{
	if (Role != ROLE_Authority)
	{
		UE_LOG(UTVehicles, Warning, TEXT("%s::DriverEnter() called on client"), *GetName());
		return false;
	}

	// FIXME: Should kick out previous driver if there is one?
	/*if (Driver != NULL)
	{
		DriverLeave(true);
	}*/

	// TODO: Implement Driver to be non-UTCharacter type
	AUTCharacter* NewDriver = Cast<AUTCharacter>(P);
	if (NewDriver != NULL)
	{
		// Set pawns current controller to control the vehicle pawn instead
		AController* C = NewDriver->Controller;
		if (C != NULL)
		{
			Driver = NewDriver;
			if (AUTCharacter *UTChar = Cast<AUTCharacter>(NewDriver))
			{
				UTChar->StartDriving(this);
			}
			if (Driver->Health <= 0)
			{
				Driver = NULL;
				return false;
			}
			SetDriving(true);

			// Disconnect PlayerController from Driver and connect to Vehicle.
			C->UnPossess();
			NewDriver->SetOwner(this); // This keeps the driver relevant.
			C->Possess(this); // TODO: Add VehicleTransition ==> C->Possess(this, true);

			// TODO: Implement switching PlayerController state
			/*if (PlayerController(C) != None && !C.IsChildState(C.GetStateName(), LandMovementState))
			{
				PlayerController(C).GotoState(LandMovementState);
			}*/

			if (AUTGameMode* Game = GetWorld()->GetAuthGameMode<AUTGameMode>())
			{
				Game->DriverEnteredVehicle(this, NewDriver);
			}
		}
	}

	return true;
}

bool AVehicle::DriverLeave_Implementation(bool bForceLeave)
{
	if (Role < ROLE_Authority)
	{
		UE_LOG(UTVehicles, Warning, TEXT("DriverLeave() called on client"));
		return false;
	}

	if (!bForceLeave)
	{
		AUTGameMode* Game = GetWorld()->GetAuthGameMode<AUTGameMode>();
		if (Game && !Game->CanLeaveVehicle(this, Driver))
		{
			return false;
		}
	}

	// Do nothing if we're not being driven
	if (Controller == NULL)
	{
		return false;
	}

	// Before we can exit, we need to find a place to put the driver.
	// Iterate over array of possible exit locations.
	if (Driver != NULL)
	{
		//Driver.SetHardAttach(false); // FIXME: add hard attach
		Driver->SetActorEnableCollision(true);

		if (!PlaceExitingDriver())
		{
			if (!bForceLeave)
			{
				// If we could not find a place to put the driver, leave driver inside as before.
				//Driver.SetHardAttach(true); // FIXME: add hard attach
				Driver->SetActorEnableCollision(false);
				return false;
			}
			else
			{
				Driver->SetActorLocation(GetTargetLocation());
			}
		}
	}

	FRotator ExitRotation = GetExitRotation(Controller);
	SetDriving(false);

	// Reconnect Controller to Driver.
	AController* C = Controller;
	// TODO: Implement AI logic for leaving vehicle
	/*if (C->RouteGoal == self)
	{
		C->RouteGoal = None;
	}
	if (C->MoveTarget == self)
	{
		C->MoveTarget = None;
	}*/
	Controller->UnPossess();

	if ((Driver != NULL) && (Driver->Health > 0))
	{
		Driver->SetActorRotation(ExitRotation);
		Driver->SetOwner(C);
		C->Possess(Driver);

		if (APlayerController *PC = Cast<APlayerController>(C))
		{
			// Set playercontroller to view the person that got out
			PC->ClientSetViewTarget(Driver);
		}

		if (AUTCharacter *UTChar = Cast<AUTCharacter>(Driver))
		{
			UTChar->StopDriving(this);
		}
	}

	// If controller didn't change, clear it...
	if (C == Controller)
	{
		Controller = NULL;
	}

	if (AUTGameMode* Game = GetWorld()->GetAuthGameMode<AUTGameMode>())
	{
		Game->DriverLeftVehicle(this, Driver);
	}

	// Vehicle now has no driver
	DriverLeft();
	return true;
}

void AVehicle::DriverLeft()
{
	Driver = NULL;
	SetDriving(false);
}

void AVehicle::DriverDied()
{
	if (Role < ROLE_Authority || Driver == NULL)
		return;

	if (AUTGameMode* Game = GetWorld()->GetAuthGameMode<AUTGameMode>())
	{
		// TODO: Killer is not reliable. Use proper stored killer
		//       but due to the call order, the specific killer is not set
		AController* Killer = LastHitBy;
		if (Killer == NULL && Driver != NULL)
		{
			Killer = Driver->Controller;
		}
		if (Killer == NULL)
		{
			Killer = Controller;
		}
		Game->DiscardInventory(Driver, Killer);
	}

	AController* C = Controller;
	Driver->StopDriving(this);
	Driver->Controller = C;
	Driver->DrivenVehicle = this; //for in game stats, so it knows pawn was killed inside a vehicle

	if (Controller == NULL)
		return;

	if (APlayerController *PC = Cast<APlayerController>(C))
	{
		// TODO: SetActorLocation is private. Move controller somehow!
		//Controller->SetActorLocation(GetActorLocation());

		PC->SetViewTarget(Driver);
		PC->ClientSetViewTarget(Driver);
	}

	Controller->UnPossess();
	if (Controller == C)
		Controller = NULL;
	C->SetPawn(Driver);

	// make sure driver has PRI temporarily
	APlayerState* RealPS = Driver->PlayerState;
	if (RealPS == NULL)
	{
		Driver->PlayerState = C->PlayerState;
	}
	if (AUTGameMode* Game = GetWorld()->GetAuthGameMode<AUTGameMode>())
	{
		Game->DriverLeftVehicle(this, Driver);
	}
	Driver->PlayerState = RealPS;

	// Vehicle now has no driver
	DriverLeft();
}

FRotator AVehicle::GetExitRotation_Implementation(AController* C)
{
	return FRotator(0.f, (C ? C->GetControlRotation().Yaw : 0.f), 0.f);
}

bool AVehicle::PlaceExitingDriver(APawn* ExitingDriver)
{
	if (ExitingDriver == NULL)
		ExitingDriver = Driver;

	if (ExitingDriver == NULL)
		return false;

	FVector Extent = ExitingDriver->GetSimpleCollisionRadius() * FVector(1.f);
	Extent.Z = ExitingDriver->GetSimpleCollisionHalfHeight() * 2.f;
	FVector ZOffset = Extent.Z * FVector(0.f, 0.f, 1.f);

	if (ExitPositions.Num() > 0)
	{
		// specific exit positions specified
		FVector HitLocation;
		FVector HitNormal;
		for (auto ExitPosition : ExitPositions)
		{
			if (ExitPositions[0].Z != 0)
				ZOffset = FVector(0.f, 0.f, ExitPosition.Z);
			else
				ZOffset = ExitingDriver->GetSimpleCollisionHalfHeight() * 2.f * FVector(0.f, 0.f, 2);

			FVector tryPlace = GetActorLocation() + GetActorRotation().RotateVector(ExitPosition - ZOffset) + ZOffset;

			// First, do a line check (stops us passing through things on exit).
			if (Trace(this, HitLocation, HitNormal, tryPlace, GetActorLocation() + ZOffset, false, Extent) != NULL)
				continue;

			// Then see if we can place the player there.
			if (!ExitingDriver->SetActorLocation(tryPlace))
				continue;

			return true;
		}
	}
	else
	{
		return FindAutoExit(ExitingDriver);
	}

	return false;
}

bool AVehicle::FindAutoExit(APawn* ExitingDriver)
{
	FVector FacingDir = GetActorRotation().Vector();
	FVector CrossProduct = FVector::CrossProduct(FacingDir, FVector(0.f, 0.f, 1.f)).GetSafeNormal();

	if (ExitRadius == 0)
	{
		ExitRadius = GetSimpleCollisionRadius();
		if (AUTCharacter* Char = Cast<AUTCharacter>(ExitingDriver))
		{
			ExitRadius += Char->VehicleCheckRadius;
		}
	}
	
	float PlaceDist = ExitRadius + ExitingDriver->GetSimpleCollisionRadius();
	FVector ExitPos = GetTargetLocation() + ExitOffset;

	return TryExitPos(ExitingDriver, ExitPos + PlaceDist * CrossProduct, false)
		|| TryExitPos(ExitingDriver, ExitPos - PlaceDist * CrossProduct, false)
		|| TryExitPos(ExitingDriver, ExitPos - PlaceDist * FacingDir, false)
		|| TryExitPos(ExitingDriver, ExitPos + PlaceDist * FacingDir, false);
}

bool AVehicle::TryExitPos(APawn* ExitingDriver, FVector ExitPos, bool bMustFindGround)
{
	if (ExitingDriver == NULL)
	{
		return false;
	}

	FVector Slice = ExitingDriver->GetSimpleCollisionRadius() * FVector(1.f);
	Slice.Z = 2;

	// First, do a line check (stops us passing through things on exit).
	FVector HitLocation;
	FVector HitNormal;
	FVector StartLocation = GetTargetLocation();
	if (Trace(this, HitLocation, HitNormal, ExitPos, StartLocation, false, Slice) != NULL)
	{
		return false;
	}

	// Now trace down, to find floor
	float CollisionHeight = ExitingDriver->GetSimpleCollisionHalfHeight() * 2.f;
	AActor* HitActor = Trace(this, HitLocation, HitNormal, ExitPos - (CollisionHeight * FVector(0.f, 0.f, 5.f)), ExitPos, true, Slice);

	if (HitActor == NULL)
	{
		if (bMustFindGround)
		{
			return false;
		}
		HitLocation = ExitPos;
	}

	ACharacter* Char = Cast<ACharacter>(ExitingDriver);
	float MaxStepHeight = (Char && Char->GetCharacterMovement() ? Char->GetCharacterMovement()->MaxStepHeight : 0.f);
	FVector NewActorPos = HitLocation + (CollisionHeight + MaxStepHeight) * FVector(0.f, 0.f, 1.f);

	// Check this wont overlap this vehicle.
	if (PointCheckComponent(GetMesh(), NewActorPos, ExitingDriver->GetSimpleCollisionCylinderExtent()))
	{
		return false;
	}

	// try placing driver on floor
	return ExitingDriver->SetActorLocation(NewActorPos);
}

void AVehicle::SetDriving_Implementation(bool bNewDriving)
{
	if (bDriving != bNewDriving)
	{
		bDriving = bNewDriving;
		DrivingStatusChanged();
	}
}

void AVehicle::DrivingStatusChanged_Implementation()
{
	if (!bDriving)
	{
		// Put brakes on before driver gets out! :)
		Throttle = 0;
		Steering = 0;
		Rise = 0;
	}
}

float AVehicle::InternalTakeRadialDamage(float Damage, struct FRadialDamageEvent const& RadialDamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (Role < ROLE_Authority)
		return Damage;

	float ActualDamage = Super::InternalTakeRadialDamage(Damage, RadialDamageEvent, EventInstigator, DamageCauser);

	if (Health > 0)
	{
		DriverTakeRadialDamage(Damage, RadialDamageEvent, EventInstigator, DamageCauser);
	}

	return ActualDamage;
}

float AVehicle::DriverTakeRadialDamage(float Damage, struct FRadialDamageEvent const& RadialDamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	//if driver has collision, whatever is causing the radius damage will hit the driver by itself
	if (EventInstigator != NULL && Driver != NULL && bAttachDriver && !Driver->GetActorEnableCollision()) // TODO: Check: !Driver.bCollideActors && !Driver.bBlockActors
	{
		// TODO: Implement taking radial damage. Routing to Driver
		//Driver->TakeDamage(Damage, FRadialDamageEvent, RadialDamageEvent, EventInstigator, DamageCauser);
	}

	return 0.f;
}

// TODO: Check if needed
void AVehicle::StartFire(uint8 FireModeNum)
{
	UE_LOG(LogUTCharacter, Verbose, TEXT("StartFire %d"), FireModeNum);

	if (!IsLocallyControlled())
	{
		UE_LOG(LogUTCharacter, Warning, TEXT("StartFire() can only be called on the owning client"));
	}
	else if (Weapon != NULL)
	{
		Weapon->StartFire(FireModeNum);
	}
}

// TODO: Check if needed
void AVehicle::StopFire(uint8 FireModeNum)
{
	if (!IsLocallyControlled())
	{
		UE_LOG(LogUTCharacter, Warning, TEXT("StopFire() can only be called on the owning client"));
	}
	else if (Weapon != NULL)
	{
		Weapon->StopFire(FireModeNum);
	}
}

// TODO: Check if needed
void AVehicle::StopFiring()
{
	for (int32 i = 0; i < PendingFire.Num(); i++)
	{
		if (PendingFire[i])
		{
			StopFire(i);
		}
	}
}

#undef AUTCharacter
#undef AUTGameMode