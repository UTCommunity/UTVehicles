#include "UTVehiclesPrivatePCH.h"

#include "ModuleManager.h"
#include "ModuleInterface.h"

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
