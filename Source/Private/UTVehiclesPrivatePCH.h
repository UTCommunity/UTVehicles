#pragma once

#include "Core.h"
#include "Engine.h"
#include "Net/UnrealNetwork.h"

#include "UnrealTournament.h"

DECLARE_LOG_CATEGORY_EXTERN(UTVehicles, Log, All);

// TODO: Implement HitResult as optional by-ref parameter
extern AActor* Trace
(
	AActor* TraceActor,
	FVector& HitLocation,
	FVector& HitNormal,
	FVector TraceEnd,
	FVector TraceStart = FVector::ZeroVector,
	bool bTraceActors = false,
	FVector Extent = FVector::ZeroVector,
	//const FHitResult& OutHit = FHitResult(),
	int ExtraTraceFlags = 0
);

// TODO: Implement HitResult as optional by-ref parameter
extern AActor* Trace
(
	FVector& HitLocation,
	FVector& HitNormal,
	FVector TraceEnd,
	FVector TraceStart = FVector::ZeroVector,
	bool bTraceActors = false,
	FVector Extent = FVector::ZeroVector,
	//const FHitResult& OutHit = FHitResult(),
	int ExtraTraceFlags = 0
);

extern bool PointCheckComponent
(
	UPrimitiveComponent* InComponent,
	FVector PointLocation,
	FVector PointExtent
);
