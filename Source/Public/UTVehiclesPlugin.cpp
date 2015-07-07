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

AActor* Trace(AActor* TraceActor, FVector& HitLocation, FVector& HitNormal, FVector TraceEnd, FVector TraceStart, bool bTraceActors, FVector Extent/*, FHitResult& OutHit*/, int ExtraTraceFlags)
{
	check(TraceActor != NULL);
	check(TraceActor->GetWorld() != NULL);

	FHitResult HitResult;
	static FName NAME_Trace = FName(TEXT("Trace"));
	FCollisionQueryParams TraceParams(NAME_Trace, true, TraceActor);

	// TODO: Implement Sweep trace with Extent
	bool bHit = TraceActor->GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, COLLISION_TRACE_WEAPON, TraceParams);
	if (bHit)
	{
		HitLocation = HitResult.ImpactPoint;
		HitNormal = HitResult.ImpactNormal;
		return HitResult.Actor.Get();
	}

	return NULL;
}

AActor* Trace(FVector& HitLocation, FVector& HitNormal, FVector TraceEnd, FVector TraceStart, bool bTraceActors, FVector Extent/*, FHitResult& OutHit*/, int ExtraTraceFlags)
{
	const UWorld* const World = GWorld;
	if (World != NULL)
	{
		FHitResult HitResult;
		static FName NAME_UseTrace = FName(TEXT("Trace"));
		FCollisionQueryParams TraceParams(NAME_UseTrace, true);

		// TODO: Implement Sweep trace with Extent
		bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, COLLISION_TRACE_WEAPON, TraceParams);
		if (bHit)
		{
			HitLocation = HitResult.ImpactPoint;
			HitNormal = HitResult.ImpactNormal;
			return HitResult.Actor.Get();
		}
	}

	return NULL;
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
	ThisActor->AttachRootComponentToActor(NewBase, AttachName);
	
	if (IBaseChangeInterface* BaseChangeInterface = Cast<IBaseChangeInterface>(NewBase))
	{
		BaseChangeInterface->BaseChange();
	}
}
