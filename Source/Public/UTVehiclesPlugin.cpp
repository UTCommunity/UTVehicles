#include "UTVehiclesPrivatePCH.h"

#include "ModuleManager.h"
#include "ModuleInterface.h"

#include "UnrealTournament.h"
#include "Components/PrimitiveComponent.h"

#include "IBaseChangeInterface.h"

DEFINE_LOG_CATEGORY(UTVehicles);

class FUTVehiclesPlugin : public IModuleInterface
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FUTVehiclesPlugin, UTVehicles )

void FUTVehiclesPlugin::StartupModule()
{
	
}
void FUTVehiclesPlugin::ShutdownModule()
{
	
}

AActor* Trace(AActor* TraceActor, FVector& HitLocation, FVector& HitNormal, FVector TraceEnd, FVector TraceStart, bool bTraceActors, FVector Extent, FHitResult* OutHit, int ExtraTraceFlags)
{
	check(TraceActor != NULL);
	check(TraceActor->GetWorld() != NULL);

	bool bHit;
	FHitResult HitResult;
	static FName NAME_Trace = FName(TEXT("Trace"));
	FCollisionQueryParams TraceParams(NAME_Trace, true, TraceActor);

	if (Extent.IsZero())
	{
		bHit = TraceActor->GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, COLLISION_TRACE_WEAPON, TraceParams);
	}
	else
	{
		auto shape = FCollisionShape::MakeBox(0.5f*Extent); // recude box by half as extent is generally the full size
		bHit = TraceActor->GetWorld()->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity, COLLISION_TRACE_WEAPON, shape, TraceParams);
	}

	if (OutHit != NULL)
	{
		*OutHit = HitResult;
	}
	if (bHit)
	{
		HitLocation = HitResult.ImpactPoint;
		HitNormal = HitResult.ImpactNormal;
		return HitResult.Actor.Get();
	}

	return NULL;
}

AActor* Trace(FVector& HitLocation, FVector& HitNormal, FVector TraceEnd, FVector TraceStart, bool bTraceActors, FVector Extent, FHitResult* OutHit, int ExtraTraceFlags)
{
	const UWorld* const World = GWorld;
	if (World != NULL)
	{
		bool bHit;
		FHitResult HitResult;
		static FName NAME_Trace = FName(TEXT("Trace"));
		FCollisionQueryParams TraceParams(NAME_Trace, true);

		if (Extent.IsZero())
		{
			bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, COLLISION_TRACE_WEAPON, TraceParams);
		}
		else
		{
			auto Collider = FCollisionShape::MakeBox(0.5f*Extent); // recude box by half as extent is generally the full size
			bHit = World->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity, COLLISION_TRACE_WEAPON, Collider, TraceParams);
		}

		if (OutHit != NULL)
		{
			*OutHit = HitResult;
		}
		if (bHit)
		{
			HitLocation = HitResult.ImpactPoint;
			HitNormal = HitResult.ImpactNormal;
			return HitResult.Actor.Get();
		}
	}

	return NULL;
}

bool TraceComponent(FVector& HitLocation, FVector& HitNormal, UPrimitiveComponent* InComponent, FVector TraceEnd, FVector TraceStart, FVector Extent, FHitResult* OutHitInfo)
{
	check(InComponent != NULL);
	check(InComponent->IsValidLowLevel() != NULL);

	FHitResult HitResult;
	static FName NAME_Trace = FName(TEXT("Trace"));
	FCollisionQueryParams TraceParams(NAME_Trace, true);

	bool bHit = InComponent->LineTraceComponent(HitResult, TraceStart, TraceEnd, TraceParams);
	if (OutHitInfo != NULL)
	{
		*OutHitInfo = HitResult;
	}

	return bHit;
}

bool PointCheckComponent(UPrimitiveComponent* InComponent, FVector PointLocation, FVector PointExtent)
{
	FHitResult HitResult;
	
	if (InComponent != NULL && InComponent->GetAttachmentRootActor())
	{
		return InComponent->OverlapComponent(PointLocation, FQuat::Identity, FCollisionShape::MakeBox(PointExtent));
	}

	return true;
}

void ActorSetBase(AActor* ThisActor, AActor* NewBase, FVector NewFloor, USkeletalMeshComponent* SkelComp, const FName AttachName)
{
	ThisActor->AttachRootComponentToActor(NewBase, AttachName, EAttachLocation::SnapToTarget);
	
	if (IBaseChangeInterface* BaseChangeInterface = Cast<IBaseChangeInterface>(NewBase))
	{
		BaseChangeInterface->Execute_BaseChange(NewBase);
	}
}
