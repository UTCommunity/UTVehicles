#pragma once

#include "Core.h"
#include "Engine.h"
#include "Net/UnrealNetwork.h"

#include "UnrealTournament.h"

DECLARE_LOG_CATEGORY_EXTERN(UTVehicles, Log, All);

extern AActor* Trace
(
	AActor* TraceActor,
	FVector& HitLocation,
	FVector& HitNormal,
	FVector TraceEnd,
	FVector TraceStart = FVector::ZeroVector,
	bool bTraceActors = false,
	FVector Extent = FVector::ZeroVector,
	FHitResult* OutHit = NULL,
	int ExtraTraceFlags = 0
);

extern AActor* Trace
(
	FVector& HitLocation,
	FVector& HitNormal,
	FVector TraceEnd,
	FVector TraceStart = FVector::ZeroVector,
	bool bTraceActors = false,
	FVector Extent = FVector::ZeroVector,
	FHitResult* OutHit = NULL,
	int ExtraTraceFlags = 0
);

extern bool PointCheckComponent
(
	UPrimitiveComponent* InComponent,
	FVector PointLocation,
	FVector PointExtent
);

extern void ActorSetBase
(
	AActor* ThisActor,
	AActor* NewBase,
	FVector NewFloor = FVector::ZeroVector, 
	USkeletalMeshComponent* SkelComp = NULL, 
	const FName AttachName = NAME_None
);
