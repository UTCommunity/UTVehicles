#include "UTVehiclesPrivatePCH.h"
#include "AAAUTPlayerController.h"
#include "AAAUTCharacter.h"
#include "Vehicle.h"
#include "UTVehicle.h"
#include "AAAUTPlayerInput.h"

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

void AUTPlayerController::InitInputSystem()
{
	if (PlayerInput == NULL)
	{
		PlayerInput = NewObject<UUTPlayerInput>(this, UAAAUTPlayerInput::StaticClass());
	}

	Super::InitInputSystem();
}

void AUTPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("Use", IE_Pressed, this, &AUTPlayerController::Use);
}

void AUTPlayerController::ServerSuicide_Implementation()
{
	// TODO: Pull request? Use Interface, use ServerSuicideInternal, ...?

	// throttle suicides to avoid spamming to grief own team in TDM
	if (GetPawn() != NULL && (GetWorld()->TimeSeconds - GetPawn()->CreationTime > 10.0f || GetWorld()->WorldType == EWorldType::PIE || GetNetMode() == NM_Standalone))
	{
		AVehicle* Vec = Cast<AVehicle>(GetPawn());
		if (Vec != NULL && Vec->PlayerSuicideInternal())
		{
			return;
		}

		Super::ServerSuicide_Implementation();
	}
}

void AUTPlayerController::BehindView(bool bWantBehindView)
{
	Super::BehindView(bWantBehindView);
	
	// make sure we recalculate camera position for this frame
	if (PlayerCameraManager != NULL)
	{
		PlayerCameraManager->LastFrameCameraCache.TimeStamp = GetWorld()->GetTimeSeconds() - 1.0;
	}
}

void AUTPlayerController::GetPlayerViewPoint(FVector& out_Location, FRotator& out_Rotation) const
{
	AVehicle* Vehicle = Cast<AVehicle>(GetPawn());
	
	if (IsLocalController() && Vehicle != NULL)
	{
		float DeltaTime = 0;
		
		FMinimalViewInfo OutResult;
		OutResult.Location = out_Location;
		OutResult.Rotation = out_Rotation;

		if (PlayerCameraManager != NULL)
		{
			DeltaTime = GetWorld()->GetTimeSeconds() - PlayerCameraManager->LastFrameCameraCache.TimeStamp;
			PlayerCameraManager->FillCameraCache(OutResult);
		}

		Vehicle->CalcCamera(DeltaTime, OutResult);

		out_Location = OutResult.Location;
		out_Rotation = OutResult.Rotation;
	}
	else
	{
		Super::GetPlayerViewPoint(out_Location, out_Rotation);
	}
}

void AUTPlayerController::UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& HiddenComponents)
{
	Super::UpdateHiddenComponents(ViewLocation, HiddenComponents);

	// prevent hiding vehicle
	AVehicle* Vec = Cast<AVehicle>(GetPawn());
	if (Vec != NULL)
	{
		UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Vec->GetRootComponent());
		if (RootPrim != NULL)
		{
			HiddenComponents.Remove(RootPrim->ComponentId);
		}
	}
}

void AUTPlayerController::SwitchWeapon(int32 Group)
{
	if (auto Vec = Cast<AUTVehicleBase>(GetPawn()))
	{
		Vec->SwitchWeapon(Group); // TODO: FIXME: Maybe change directly to switch seat?
		return;
	}
		
	Super::SwitchWeapon(Group);
}

void AUTPlayerController::SwitchWeaponGroup(int32 Group)
{
	if (auto Vec = Cast<AUTVehicleBase>(GetPawn()))
	{
		Vec->SwitchWeapon(Group); // TODO: FIXME: Maybe change directly to switch seat?
		return;
	}

	Super::SwitchWeaponGroup(Group);
}

void AUTPlayerController::MoveForward(float Value)
{
	if (AVehicle* Vehicle = Cast<AVehicle>(GetPawn()))
	{
		MovementForwardAxis = FMath::Clamp<float>(MovementForwardAxis + Value, -1., 1.f);
		Vehicle->SetThrottleInput(MovementForwardAxis);
	}
	else
	{
		Super::MoveForward(Value);
	}
}

void AUTPlayerController::MoveBackward(float Value)
{
	if (AVehicle* Vehicle = Cast<AVehicle>(GetPawn()))
	{
		MoveForward(Value * -1);
	}
	else
	{
		Super::MoveBackward(Value);
	}
}

void AUTPlayerController::MoveLeft(float Value)
{
	if (AVehicle* Vehicle = Cast<AVehicle>(GetPawn()))
	{
		MoveRight(Value * -1);
	}
	else
	{
		Super::MoveLeft(Value);
	}
}

void AUTPlayerController::MoveRight(float Value)
{
	if (AVehicle* Vehicle = Cast<AVehicle>(GetPawn()))
	{
		MovementStrafeAxis = FMath::Clamp<float>(MovementStrafeAxis + Value, -1., 1.f);
		Vehicle->SetSteeringInput(MovementStrafeAxis);
	}
	else
	{
		Super::MoveRight(Value);
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
		Vehicle->SetRiseInput(-1.0f);
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
	bJustFoundVehicle = false;

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