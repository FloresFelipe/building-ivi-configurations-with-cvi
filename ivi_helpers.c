#include <stdio.h>
#include "utility.h"
#include "ivi_helpers.h"

// =========================
// Error Handling
// =========================
int HandleError(ViStatus status)
{
    if (status < VI_SUCCESS) {
        char errMsg[512];
        IviConfig_GetError(sizeof(errMsg), errMsg);
        DebugPrintf("Error: %s\n", errMsg);
        return 1;
    }
    return 0;
}

// =========================
// High-level Helpers
// =========================

ViStatus LoadConfigurationStore(IviConfigStoreHandle* store)
{
    ViStatus status = Ivi_GetConfigStoreHandle(store);

    if (status == VI_SUCCESS)
        DebugPrintf("Configuration Store Loaded!\n");

    return status;
}

ViStatus GetOrCreateSoftwareModule(
    IviConfigStoreHandle store,
    const char* moduleName,
    IviSoftwareModuleHandle* module)
{
    IviSoftwareModuleCollectionHandle collection;
    ViStatus status;

    status = IviConfig_GetConfigStoreSoftwareModuleCollection(store, &collection);
    if (HandleError(status)) return status;

    status = IviConfig_GetSoftwareModuleItemByName(collection, moduleName, module);
    if (status == VI_SUCCESS) return VI_SUCCESS;

    DebugPrintf("Creating software module...\n");
    return IviConfig_CreateSoftwareModule(collection, moduleName, module);
}

ViStatus GetOrCreateDriverSession(
    IviConfigStoreHandle store,
    const char* name,
    IviDriverSessionHandle* session)
{
    IviDriverSessionCollectionHandle collection;
    ViStatus status;

    status = IviConfig_GetConfigStoreDriverSessionCollection(store, &collection);
    if (HandleError(status)) return status;

    status = IviConfig_GetDriverSessionItemByName(collection, name, session);
    if (status == VI_SUCCESS) return VI_SUCCESS;

    DebugPrintf("Creating driver session...\n");
    return IviConfig_CreateDriverSession(collection, name, session);
}

ViStatus ConfigureDriverSessionProperties(IviDriverSessionHandle s)
{
    ViStatus status;

    status = IviConfig_SetDriverSessionPropertyViBoolean(
        s, IVICONFIG_VAL_DRIVER_SESSION_SIMULATE, VI_TRUE);
    if (HandleError(status)) return status;

    status = IviConfig_SetDriverSessionPropertyViBoolean(
        s, IVICONFIG_VAL_DRIVER_SESSION_CACHE, VI_TRUE);
    if (HandleError(status)) return status;

    status = IviConfig_SetDriverSessionPropertyViBoolean(
        s, IVICONFIG_VAL_DRIVER_SESSION_RANGE_CHECK, VI_TRUE);
    if (HandleError(status)) return status;

    return VI_SUCCESS;
}

ViStatus ConfigureDriverSessionSimulationDriver(
    IviDriverSessionHandle session,
    const char* moduleName)
{
    IviDataComponentCollectionHandle collection;
    ViStatus status;

    status = IviConfig_GetConfigComponentDataComponentCollection(
        (IviConfigComponentHandle)session, &collection);
    if (HandleError(status)) return status;

    // ? Calls next function below
    return ConfigureSimulationStructure(collection, moduleName);
}

ViStatus ConfigureSimulationStructure(
    IviDataComponentCollectionHandle collection,
    const char* moduleName)
{
    IviDataComponentCollectionHandle structCollection;
    IviDataComponentHandle structHandle;
    ViStatus status;

    status = GetOrCreateStructure(
        collection,
        "NI Settings",
        "Simulation Driver Session",
        &structHandle);
    if (HandleError(status)) return status;

    status = IviConfig_GetStructureDataComponentCollection(
        structHandle, &structCollection);
    if (HandleError(status)) return status;

    // ? Calls next function below
    return ConfigureSimulationComponents(structCollection, moduleName);
}

ViStatus ConfigureSimulationComponents(
    IviDataComponentCollectionHandle structCollection,
    const char* moduleName)
{
    ViStatus status;

    IviDataComponentHandle strComp;
    status = GetOrCreateDataComponent(
        structCollection,
        "Simulation Driver Session",
        IVICONFIG_VAL_TYPE_STRING,
        &strComp);
    if (HandleError(status)) return status;

    status = SetStringValue(strComp, moduleName);
    if (HandleError(status)) return status;

    IviDataComponentHandle boolComp;
    status = GetOrCreateDataComponent(
        structCollection,
        "Use Specific Simulation",
        IVICONFIG_VAL_TYPE_BOOLEAN,
        &boolComp);
    if (HandleError(status)) return status;

    return SetBooleanValue(boolComp, VI_FALSE);
}

ViStatus ConfigureDriverSessionInstrumentDriver(
    IviDriverSessionHandle session,
    IviSoftwareModuleHandle module)
{
    return IviConfig_SetSessionSoftwareModuleReference(session, module);
}

ViStatus GetOrCreateLogicalName(
    IviConfigStoreHandle store,
    const char* name,
    IviLogicalNameHandle* handle)
{
    IviLogicalNameCollectionHandle collection;
    ViStatus status;

    status = IviConfig_GetConfigStoreLogicalNameCollection(store, &collection);
    if (HandleError(status)) return status;

    status = IviConfig_GetLogicalNameItemByName(collection, name, handle);
    if (status == VI_SUCCESS) return VI_SUCCESS;

    DebugPrintf("Creating logical name...\n");
    return IviConfig_CreateLogicalName(collection, name, handle);
}

ViStatus LinkLogicalNameToSession(
    IviLogicalNameHandle logicalName,
    IviDriverSessionHandle session)
{
    return IviConfig_SetLogicalNameSessionReference(
        logicalName, (IviSessionHandle)session);
}

ViStatus SaveConfiguration(
    IviConfigStoreHandle store,
    const char* fallbackPath)
{
    ViStatus status = IviConfig_Serialize(store, DEFAULT_CONFIG_PATH);

    if (status == VI_SUCCESS) {
        DebugPrintf("IVI Configuration Saved Successfully!\n");
        return VI_SUCCESS;
    }

    DebugPrintf("Default save failed. Using fallback...\n");

    return IviConfig_Serialize(store, fallbackPath);
}

// =========================
// Low-level Helpers
// =========================

ViStatus GetOrCreateStructure(
    IviDataComponentCollectionHandle collection,
    const char* name,
    const char* firstChild,
    IviDataComponentHandle* handle)
{
    ViStatus status = IviConfig_GetDataComponentItemByName(collection, name, handle);

    if (status == VI_SUCCESS) {
        DebugPrintf("Structure '%s' exists.\n", name);
        return VI_SUCCESS;
    }

    DebugPrintf("Creating structure '%s'...\n", name);

    return IviConfig_CreateStructure(
        collection,
        name,
        IVICONFIG_VAL_TYPE_STRING,
        firstChild,
        handle);
}

ViStatus GetOrCreateDataComponent(
    IviDataComponentCollectionHandle collection,
    const char* name,
    ViInt32 type,
    IviDataComponentHandle* handle)
{
    ViStatus status = IviConfig_GetDataComponentItemByName(collection, name, handle);

    if (status == VI_SUCCESS) {
        DebugPrintf("Component '%s' exists.\n", name);
        return VI_SUCCESS;
    }

    DebugPrintf("Creating component '%s'...\n", name);

    return IviConfig_CreateDataComponent(
        collection,
        type,
        name,
        handle);
}

ViStatus SetStringValue(IviDataComponentHandle component, const char* value)
{
    return IviConfig_SetDataComponentPropertyViString(
        component,
        IVICONFIG_VAL_DATA_COMPONENT_VALUE,
        value);
}

ViStatus SetBooleanValue(IviDataComponentHandle component, ViBoolean value)
{
    return IviConfig_SetDataComponentPropertyViBoolean(
        component,
        IVICONFIG_VAL_DATA_COMPONENT_VALUE,
        value);
}