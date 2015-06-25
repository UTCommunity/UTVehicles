#include "UTVehiclesPrivatePCH.h"

#include "ModuleManager.h"
#include "ModuleInterface.h"

#include "UnrealTournament.h"
#include "Components/PrimitiveComponent.h"

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

AActor* Trace(FVector& HitLocation, FVector& HitNormal, FVector TraceEnd, FVector TraceStart, bool bTraceActors, FVector Extent/*, FHitResult& OutHit*/, int ExtraTraceFlags)
{
	const UWorld* const World = GWorld;
	if (World != NULL)
	{
		FHitResult HitResult;
		static FName NAME_UseTrace = FName(TEXT("UseTrace"));
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

