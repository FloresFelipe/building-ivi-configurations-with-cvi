#ifndef IVI_HELPERS_H
#define IVI_HELPERS_H

#include <ivi.h>
#include "IviConfigServer.h"

// =========================
// Configuration Constants
// =========================
static const char* DEFAULT_CONFIG_PATH = "C:\\ProgramData\\IVI Foundation\\IVI\\IviConfigurationStore.xml";

// =========================
// Type Defs
// =========================

typedef struct iviConfig {
	
	char* driverSessionName;
	char* logicalName;
	char* instrumentDriverSoftwareModuleName;
	char* simulationDriverSoftwareModuleName;
	char* fallbackConfigPath;
	ViBoolean isSimulated;
	
} iviConfig;


// =========================
// Error Handling
// =========================
int HandleError(ViStatus status);

// =========================
// High-level Helpers (ordered by flow)
// =========================

ViStatus LoadConfigurationStore(
    IviConfigStoreHandle* store);

ViStatus GetOrCreateSoftwareModule(
    IviConfigStoreHandle store,
    const char* moduleName,
    IviSoftwareModuleHandle* module);

ViStatus GetOrCreateDriverSession(
    IviConfigStoreHandle store,
    const char* name,
    IviDriverSessionHandle* session);

ViStatus ConfigureDriverSessionProperties(
    IviDriverSessionHandle session);

ViStatus ConfigureDriverSessionSimulationDriver(
    IviDriverSessionHandle session,
    const char* moduleName);

// ?? New split functions (introduced in refactor)
ViStatus ConfigureSimulationStructure(
    IviDataComponentCollectionHandle collection,
    const char* moduleName);

ViStatus ConfigureSimulationComponents(
    IviDataComponentCollectionHandle structCollection,
    const char* moduleName);

ViStatus ConfigureDriverSessionInstrumentDriver(
    IviDriverSessionHandle session,
    IviSoftwareModuleHandle module);

ViStatus GetOrCreateLogicalName(
    IviConfigStoreHandle store,
    const char* name,
    IviLogicalNameHandle* handle);

ViStatus LinkLogicalNameToSession(
    IviLogicalNameHandle logicalName,
    IviDriverSessionHandle session);

ViStatus SaveConfiguration(
    IviConfigStoreHandle store,
    const char* fallbackPath);

// =========================
// Low-level Helpers
// =========================
ViStatus GetOrCreateStructure(
    IviDataComponentCollectionHandle collection,
    const char* name,
    const char* firstChild,
    IviDataComponentHandle* handle);

ViStatus GetOrCreateDataComponent(
    IviDataComponentCollectionHandle collection,
    const char* name,
    ViInt32 type,
    IviDataComponentHandle* handle);

ViStatus SetStringValue(
    IviDataComponentHandle component,
    const char* value);

ViStatus SetBooleanValue(
    IviDataComponentHandle component,
    ViBoolean value);

#endif