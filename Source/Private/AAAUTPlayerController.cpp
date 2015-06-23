#include "UTVehiclesPrivatePCH.h"
#include "AAAUTPlayerController.h"
#include "AAAUTCharacter.h"
#include "Vehicle.h"
#include "UTVehicle.h"

#define AUTPlayerController AAAAUTPlayerController
#define AUTCharacter AAAAUTCharacter

// TODO: Allow to Enter AVehicle and not just AUTVehicle.
//       Original code from UT3 also bypasses checks for AVehicle as APlayerController::FindVehicleToDrive is overriden

// TODO: Merge PerformedUseActionInternal with PerformedUseAction

AUTPlayerController::AUTPlayerController(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	VehicleCheckRadiusScaling = 1.0f;
}

void AUTPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("Use", IE_Pressed, this, &AUTPlayerController::Use);
}

void AUTPlayerController::MoveForward(float Val)
{
	if (AVehicle* Vehicle = Cast<AVehicle>(GetPawn()))
	{
		Vehicle->SetThrottleInput(Val);
	}
	else
	{
		Super::MoveForward(Val);
	}
}

void AUTPlayerController::MoveBackward(float Val)
{
	if (AVehicle* Vehicle = Cast<AVehicle>(GetPawn()))
	{
		Vehicle->SetThrottleInput(Val);
	}
	else
	{
		Super::MoveBackward(Val);
	}
}

void AUTPlayerController::MoveLeft(float Val)
{
	if (AVehicle* Vehicle = Cast<AVehicle>(GetPawn()))
	{
		Vehicle->SetSteeringInput(Val);
	}
	else
	{
		Super::MoveLeft(Val);
	}
}

void AUTPlayerController::MoveRight(float Val)
{
	if (AVehicle* Vehicle = Cast<AVehicle>(GetPawn()))
	{
		Vehicle->SetSteeringInput(Val);
	}
	else
	{
		Super::MoveRight(Val);
	}
}

void AUTPlayerController::Jump()
{
	if (AVehicle* Vehicle = Cast<AVehicle>(GetPawn()))
	{
		Vehicle->SetRiseInput(1.f);
	}
	else
	{
		Super::Jump();
	}
}

void AUTPlayerController::JumpRelease()
{
	if (AVehicle* Vehicle = Cast<AVehicle>(GetPawn()))
	{
		Vehicle->SetRiseInput(0.f);
	}
	else
	{
		Super::JumpRelease();
	}
}

void AUTPlayerController::Crouch()
{
	if (AVehicle* Vehicle = Cast<AVehicle>(GetPawn()))
	{
		Vehicle->SetRiseInput(1.0f);
	}
	else
	{
		Super::Crouch();
	}
}

void AUTPlayerController::UnCrouch()
{
	if (AVehicle* Vehicle = Cast<AVehicle>(GetPawn()))
	{
		Vehicle->SetRiseInput(0.f);
	}
	else
	{
		Super::UnCrouch();
	}
}

void AUTPlayerController::Use()
{
	if (Role < ROLE_Authority)
	{
		PerformedUseAction();
	}
	ServerUse();
}

bool AUTPlayerController::ServerUse_Validate()
{
	return true;
}

void AUTPlayerController::ServerUse_Implementation()
{
	// Limit use frequency
	if ((LastUseTime == GetWorld()->TimeSeconds) || ((Cast<AVehicle>(GetPawn()) != NULL) && (GetWorld()->TimeSeconds - LastUseTime < 1.0f)))
	{
		return;
	}
	LastUseTime = GetWorld()->TimeSeconds;
	PerformedUseAction();
}

bool AUTPlayerController::PerformedUseAction()
{
	//bJustFoundVehicle = false;

	//if (UTPawn(Pawn) != None && UTPawn(Pawn).IsHero())
	//{
	//	Pawn.ToggleMelee();
	//	return false;
	//}

	AUTCharacter* Char = Cast<AUTCharacter>(GetPawn());
	AVehicle* Vec = Cast<AVehicle>(GetPawn());

	if (Char && Char->IsFeigningDeath())
	{
		// can't use things while feigning death
		return true;
	}

	if ((Char != NULL) && (Vec == NULL))
	{
		// TODO: Implement FlagUse of carried objects. UTCarriedObject::Use doesn't seem to be the same (no passed character).
		/*
		TArray<AActor*> Touching;
		GetOverlappingActors(Touching, AUTCarriedObject::StaticClass());
		for (AActor* TouchingActor : Touching)
		{
			AUTCarriedObject* Flag = Cast<AUTCarriedObject>(TouchingActor);
			if (Flag != NULL && Flag->FlagUse(this))
			{
				return true;
			}
		}*/
	}

	if (PerformedUseActionInternal())
	{
		return true;
	}

	// TODO: Implement SmartUse for game pads
	//if ((Role == ROLE_Authority) && !bJustFoundVehicle)
	//{
	//	// Gamepad smart use - bring out translocator or hoverboard if no other use possible
	//	ClientSmartUse();
	//	return true;
	//}

	return false;
}

bool AUTPlayerController::PerformedUseActionInternal()
{
	// if the level is paused,
	if (GetWorldSettings()->Pauser == PlayerState)
	{
		if (Role == ROLE_Authority)
		{
			// unpause and move on
			SetPause(false);
		}
		return true;
	}

	AUTCharacter* Char = Cast<AUTCharacter>(GetPawn());
	AVehicle* Vec = Cast<AVehicle>(GetPawn());

	// skip if character isn't able to interact with objects
	if (Char && !Char->CanUse())
		return true;

	// below is only on server
	if (Role < ROLE_Authority)
	{
		return false;
	}

	// leave vehicle if currently in one
	if (Vec != NULL)
	{
		return Vec->DriverLeave(false);
	}

	// try to find a vehicle to drive
	if (FindVehicleToDrive())
	{
		return true;
	}

	// try to interact with triggers
	return TriggerInteracted();
}

bool AUTPlayerController::FindVehicleToDrive()
{
	// TODO: Check if Pawn can drive a vehicle (like Heros/Titans prevent vehicle driving)
	/* AUTCharacter* Char = Cast<AUTCharacter>(GetPawn());
	if ((Char != NULL) && Char->IsHero())
	{
		return false;
	}*/

	return (CheckVehicleToDrive(true) != NULL);
}

AUTVehicle* AUTPlayerController::CheckVehicleToDrive(bool bEnterVehicle)
{
	bJustFoundVehicle = false;

	// first try to get in vehicle I'm standing on
	AUTVehicle* PickedVehicle = GetPawn() ? CheckPickedVehicle(Cast<AUTVehicle>(GetPawn()->GetAttachParentActor()), bEnterVehicle) : NULL;
	if ((PickedVehicle != NULL) || bJustFoundVehicle)
	{
		return PickedVehicle;
	}

	AUTCharacter* Char = Cast<AUTCharacter>(GetPawn());
	if (Char != NULL)
	{
		FHitResult HitResult;
		static FName NAME_UseTrace = FName(TEXT("UseTrace"));
		FCollisionQueryParams TraceParams(NAME_UseTrace, true, Char);

		FVector ViewPoint = Char->GetPawnViewLocation();
		FRotator ViewRotation = GetControlRotation();
		float CheckDist = Char->VehicleCheckRadius * VehicleCheckRadiusScaling;

		// see if looking at vehicle
		FVector ViewDir = CheckDist * ViewRotation.Vector();
		if (GetWorld()->LineTraceSingleByChannel(HitResult, ViewPoint, ViewPoint + ViewDir, COLLISION_TRACE_WEAPON, TraceParams))
		{
			PickedVehicle = CheckPickedVehicle(Cast<AUTVehicle>(HitResult.Actor.Get()), bEnterVehicle);
			if ((PickedVehicle != NULL) || bJustFoundVehicle)
			{
				return PickedVehicle;
			}
		}

		// make sure not just looking above vehicle
		ViewRotation.Pitch = 0;
		ViewDir = CheckDist * ViewRotation.Vector();
		if (GetWorld()->LineTraceSingleByChannel(HitResult, ViewPoint, ViewPoint + ViewDir, COLLISION_TRACE_WEAPON, TraceParams))
		{
			PickedVehicle = CheckPickedVehicle(Cast<AUTVehicle>(HitResult.Actor.Get()), bEnterVehicle);
			if ((PickedVehicle != NULL) || bJustFoundVehicle)
			{
				return PickedVehicle;
			}
		}

		// make sure ... ?
		ViewRotation.Pitch = -5000;
		ViewDir = CheckDist * ViewRotation.Vector();
		if (GetWorld()->LineTraceSingleByChannel(HitResult, ViewPoint, ViewPoint + ViewDir, COLLISION_TRACE_WEAPON, TraceParams))
		{
			PickedVehicle = CheckPickedVehicle(Cast<AUTVehicle>(HitResult.Actor.Get()), bEnterVehicle);
			if ((PickedVehicle != NULL) || bJustFoundVehicle)
			{
				return PickedVehicle;
			}
		}

		// TODO: implement VehicleList in UTGameMode
		// special case for vehicles like Darkwalker
		/*AUTGameMode* GameMode = GetWorld()->GetAuthGameMode<AUTGameMode>();
		if (GameMode != NULL)
		{
			for (AUTVehicle* V = GameMode->VehicleList; V != NULL; V = V->NextVehicle)
			{
				if (V->bHasCustomEntryRadius && V->InCustomEntryRadius(Pawn))
				{
					V = CheckPickedVehicle(V, bEnterVehicle);
					if ((V != NULL) || bJustFoundVehicle)
					{
						return V;
					}
				}
			}
		}*/
	}

	return NULL;
}

AUTVehicle* AUTPlayerController::CheckPickedVehicle(AUTVehicle* V, bool bEnterVehicle)
{
	if ((V == NULL) || !bEnterVehicle)
	{
		return V;
	}

	// TODO: Implement dropping flag on entering vehicle
	// check if I would drop my flag
	//AUTPlayerState* PS = Cast<AUTPlayerState>(PlayerState);
	//if (PRI.bHasFlag && !V.bCanCarryFlag && (!V.bTeamLocked || WorldInfo.GRI.OnSameTeam(self, V)))
	//{
	//	if (V.bRequestedEntryWithFlag)
	//	{
	//		V.bRequestedEntryWithFlag = false;
	//		ClientSetRequestedEntryWithFlag(V, false, 0);
	//	}
	//	else
	//	{
	//		V.bRequestedEntryWithFlag = true;
	//		ClientSetRequestedEntryWithFlag(V, true, (UTOnslaughtFlag(PRI.GetFlag()) == None) ? 0 : 1);
	//		bJustFoundVehicle = true;
	//		return None;
	//	}
	//}

	if (V->TryToDrive(GetPawn()))
	{
		return V;
	}

	bJustFoundVehicle = true;
	return NULL;
}

#undef AUTPlayerController
#undef AUTCharacter