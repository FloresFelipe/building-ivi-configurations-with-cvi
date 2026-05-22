#include <ivi.h>
#include "utility.h"
#include "ivi_helpers.h"

// =========================
// Local Function Prototypes
// =========================

void CreateNewConfiguration(IviConfigStoreHandle configurationStore, iviConfig configuration);

// =========================
// ERROR_CHECK macro
// =========================
#define CHECK(x) do { if (HandleError((x))) goto Error; } while (0)

// =========================
// Main
// =========================
int main(void)
{
    IviConfigStoreHandle configurationStore = VI_NULL;
	iviConfig currentConfiguration;
	
	//Initialize
    CHECK(LoadConfigurationStore(&configurationStore));
	
	//Core Logic
	//======== Configuration 1 (Simulated) ==============
	DebugPrintf("\nCreating Configuration 1\n");
	DebugPrintf("========================\n");

	currentConfiguration.driverSessionName = "cviDmm";
	currentConfiguration.logicalName = "MyDmm";
	currentConfiguration.instrumentDriverSoftwareModuleName = "";
	currentConfiguration.simulationDriverSoftwareModuleName = "nisDmm";
	currentConfiguration.fallbackConfigPath = "C:\\temp\\ivi_config.xml";
	currentConfiguration.isSimulated = VI_TRUE;
	
    CreateNewConfiguration(configurationStore, currentConfiguration);
	//===================================================
	
	//=========== Configuration 2 (Real) ================
	DebugPrintf("\nCreating Configuration 2\n");
	DebugPrintf("========================\n");

	currentConfiguration.driverSessionName = "cviScope";
	currentConfiguration.logicalName = "MyScope";
	currentConfiguration.instrumentDriverSoftwareModuleName = "niScope";
	currentConfiguration.simulationDriverSoftwareModuleName = "";
	currentConfiguration.fallbackConfigPath = "C:\\temp\\ivi_config.xml";
	currentConfiguration.isSimulated = VI_FALSE;
	
	
    CreateNewConfiguration(configurationStore, currentConfiguration);
	//===================================================
	
	//Save and Cleanup
    CHECK(SaveConfiguration(configurationStore, currentConfiguration.fallbackConfigPath));

    DebugPrintf("Done.\n");

Error:

    if (configurationStore) IviConfig_Close(configurationStore);

    return 0;
}


void CreateNewConfiguration(IviConfigStoreHandle configurationStore, iviConfig configuration)
{

	IviSoftwareModuleHandle InstrumentDriverSoftwareModule = VI_NULL;
    IviDriverSessionHandle driverSession = VI_NULL;
    IviLogicalNameHandle logicalName = VI_NULL;
		
	CHECK(GetOrCreateDriverSession(configurationStore, configuration.driverSessionName, &driverSession));
	
	CHECK(ConfigureDriverSessionProperties(driverSession));
	//Create more helper functions for handling Hardware, VirtualNames and Initial Settings and add them here!
	
	if (configuration.isSimulated == VI_TRUE)
	{
		CHECK(ConfigureDriverSessionSimulationDriver(driverSession, configuration.simulationDriverSoftwareModuleName));
	}
	else
	{
		CHECK(GetOrCreateSoftwareModule(configurationStore, configuration.instrumentDriverSoftwareModuleName, &InstrumentDriverSoftwareModule));
		CHECK(ConfigureDriverSessionInstrumentDriver(driverSession, InstrumentDriverSoftwareModule));
	}

	
    CHECK(GetOrCreateLogicalName(configurationStore, configuration.logicalName, &logicalName));
    CHECK(LinkLogicalNameToSession(logicalName, driverSession));
	
Error:
	if (logicalName) IviConfig_DisposeHandle(logicalName);
    if (driverSession) IviConfig_DisposeHandle(driverSession);
}
