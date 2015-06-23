#include "UTVehiclesPrivatePCH.h"
#include "Vehicle.h"

AVehicle::AVehicle(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	AutoPossessPlayer = EAutoReceiveInput::Disabled;

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
	else
	{
		YL = Canvas->DrawText(RenderFont, FString::Printf(TEXT("Driver Mesh %s hidden %s"), Driver->GetMesh(), Driver->bHidden), 4.0f, YPos);
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
			/*if (PlayerController(C) != None)
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

	// Do nothing if we're not being driven
	if (Controller == NULL)
	{
		return false;
	}

	// Before we can exit, we need to find a place to put the driver.
	// Iterate over array of possible exit locations.
	/*if (Driver != None)
	{
		Driver.SetHardAttach(false);
		Driver.bCollideWorld = true;
		Driver.SetCollision(true, true);

		if (!PlaceExitingDriver())
		{
			if (!bForceLeave)
			{
				// If we could not find a place to put the driver, leave driver inside as before.
				Driver.SetHardAttach(true);
				Driver.bCollideWorld = false;
				Driver.SetCollision(false, false);
				return false;
			}
			else
			{
				Driver.SetLocation(GetTargetLocation());
			}
		}
	}*/

	FRotator ExitRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
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

void AVehicle::SetDriving_Implementation(bool b)
{
	if (bDriving != b)
	{
		bDriving = b;
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