## IVI Configuration Builder (Clean Code Refactor)

This project demonstrates how to programmatically create and manage IVI (Interchangeable Virtual Instrument) configurations using the NI IVI Configuration Server API in C.

It follows **Clean Code principles**:
- Readability
- Separation of concerns
- Reusability
- Idempotent operations (safe re-execution)

---

## 🎯 Understanding IVI: Theory & Context

### What is IVI?

**IVI (Interchangeable Virtual Instrument)** is a standardized framework developed by the IVI Foundation for controlling test and measurement instruments. The key philosophy is **interchangeability**: you can swap one vendor's driver for another without changing your application code.

**Core IVI Components:**
- **IVI Foundation Standards** – Defines driver architecture and APIs
- **IVI Drivers** – Vendor-specific implementations (NI, Keysight, etc.)
- **Configuration Framework** – Stores instrument definitions and logical names

---

### IVI Configuration Server: The Core Theory

The **IVI Configuration Server** (`IviConfigServer.lib`) is responsible for:

1. **Configuration Storage** – Manages the IVI configuration database (XML-based, typically at `C:\ProgramData\IVI Foundation\IVI\IviConfigurationStore.xml`)

2. **Logical Name Resolution** – Maps human-friendly names (e.g., `"MyDmm"`) to actual driver sessions and instrument resources

3. **Hardware Assets** – Stores instrument metadata:
   - Driver sessions (connection to physical instruments)
   - Software modules (driver implementations)
   - Simulation settings
   - Resource assignments

4. **Configuration Persistence** – Serializes/deserializes configurations to/from XML

**Key Concept:**  
The Configuration Server acts as a **configuration registry**. Instead of hardcoding instrument addresses and driver names in your application, you store them centrally and reference them by logical name. This allows runtime reconfiguration without code changes.

---

### IVI Libraries: The Two-Tier System

#### 1. **IVI Foundation Library** (`ivi.lib`)

This is the **base runtime library** that provides:

- **Session Management** – Handles driver session lifecycle
- **Attribute/Property Access** – Standard interface for instrument settings
- **Error Handling** – IVI-compliant error codes and reporting
- **Type System** – Standard data types across all IVI drivers

**Purpose:**  
Provides the low-level infrastructure that all IVI drivers depend on. If you're using any IVI driver, you link against `ivi.lib`.

#### 2. **IVI Configuration Server Library** (`IviConfigServer.lib`)

This is the **configuration management library** that provides:

- **Configuration Store Access** – Load/save configurations
- **Collection Management** – Navigate driver sessions, logical names, software modules
- **Component Data** – Create/modify structured configuration data
- **Serialization** – Save configurations to persistent storage

**Purpose:**  
Extends the base IVI system to support **centralized configuration management**. You only use this library if you need to programmatically build or modify configurations (as opposed to using Measurement & Automation Explorer (MAX)).

---

### How They Work Together

```
Your Application
       │
       │ (calls IVI ConfigServer APIs)
       ▼
IviConfigServer.lib (configuration management)
       │
       │ (uses)
       ▼
ivi.lib (runtime & session management)
       │
       │ (instantiates)
       ▼
IVI Configuration Store (XML database)
       │
       │ (references)
       ▼
Driver Sessions → IVI Drivers → Hardware
```

**Data Flow Example:**

1. **Load Configuration** → IviConfigServer reads XML
2. **Access Driver Session** → Get reference to `"cviDmm"`
3. **Configure Properties** → Set simulation = TRUE, caching = ON
4. **Link Logical Name** → `"MyDmm"` → `"cviDmm"` driver session
5. **Save Configuration** → IviConfigServer serializes back to XML

---

### Why This Matters for This Project

This application demonstrates **programmatic configuration building**, which is useful for:

- **Automated Test Setup** – Configure instruments via script instead of manual MAX clicks
- **Multi-Machine Deployments** – Generate configurations for different labs/stations
- **CI/CD Integration** – Set up test environments automatically
- **Dynamic Configuration** – Adjust instrument setups based on runtime conditions

Without the Configuration Server API, you'd have to:
- Edit XML manually (error-prone)
- Use MAX GUI repeatedly (not scriptable)
- Hardcode driver names in code (inflexible)

---

## 📁 Project Structure

```
.
├── main.c           # High-level orchestration
├── ivi_helpers.h    # Public helper API
├── ivi_helpers.c    # IVI logic implementation
```

---

## 🧠 Design Overview: Layers & Abstraction

The project builds **three abstraction layers** on top of the IVI libraries:

```
Your Configuration Logic (main.c)
         │
         │ (clean, high-level calls)
         ▼
Helper Wrapper Layer (ivi_helpers.c)
         │
         │ (encapsulates IVI ConfigServer API complexity)
         ▼
IviConfigServer.lib + ivi.lib
         │
         │ (manages configuration store & runtime)
         ▼
IVI Configuration Store (XML)
```

### Layer 1: High-Level Orchestration (`main.c`)
- **What it does:** Defines the business workflow
- **How:** Calls helper functions with semantic meaning
- **Example:** "Create a simulated DMM configuration"
- **Benefit:** Reads like a story; minimal IVI knowledge required

### Layer 2: Helper Wrapper (`ivi_helpers.c` + `ivi_helpers.h`)
- **What it does:** Encapsulates the IVI ConfigServer API
- **How:** Wraps repetitive `IviConfig_*` calls with safer, simpler functions
- **Example:** `GetOrCreateDriverSession()` handles:
  - Collection access (`IviConfig_GetConfigStoreDriverSessionCollection`)
  - Item lookup (`IviConfig_GetDriverSessionItemByName`)
  - Creation if needed (`IviConfig_CreateDriverSession`)
  - Error checking
- **Benefit:** Eliminates API boilerplate; enables idempotent operations

### Layer 3: IVI Libraries (External)
- **ivi.lib:** Provides session/runtime infrastructure
- **IviConfigServer.lib:** Provides configuration database access
- **What they do:** Manage the actual XML store and driver session lifecycle

---

## ✅ What the Application Does

The application:

1. Loads the IVI Configuration Store
2. Creates **two configurations**:

### Configuration 1 (Simulated)
- Driver session: `cviDmm`
- Logical name: `MyDmm`
- Uses simulation driver: `nisDmm`
- Configures simulation-specific data

### Configuration 2 (Real Instrument)
- Driver session: `cviScope`
- Logical name: `MyScope`
- Uses real driver: `niScope`
- Links software module directly

3. For each configuration:
- Creates or reuses Driver Session
- Configures session properties
- Applies simulation OR real driver setup
- Creates or reuses Logical Name
- Links Logical Name → Driver Session

4. Saves configuration:
- Attempts default save location
- Falls back to user-defined path if needed

---

## 🔁 Idempotent Design

All creation functions follow a **get-or-create pattern**:

✅ Safe to run multiple times  
✅ No duplicate entries  
✅ Existing items are reused  

---

## 🏗️ IVI Configuration Entities: The Data Model

To understand what this code is building, you need to know the **four key entities** in the IVI Configuration Store:

### 1. **Driver Session**

A **Driver Session** is a logical connection to a driver instance. Think of it as a named slot that says: "I am using the NI DMM driver in this slot, and it should connect to these resources."

**Properties:**
- Session Name (e.g., `"cviDmm"`)
- Simulation mode (ON/OFF)
- Caching (ON/OFF)
- Range checking (ON/OFF)
- Instrument resource assignment

**In this project:**
- Created via `GetOrCreateDriverSession()`
- Configured via `ConfigureDriverSessionProperties()`

---

### 2. **Software Module**

A **Software Module** represents a specific **driver implementation** available on the system. For a DMM, examples include:
- `niDmm` – NI digital multimeter driver
- `nisDmm` – NI simulation driver for DMM
- `agDmm` – Agilent DMM driver

**Purpose:** Software modules are libraries that implement the IVI standard for a specific instrument class.

**In this project:**
- Created/referenced via `GetOrCreateSoftwareModule()`
- Linked to driver session via `ConfigureDriverSessionInstrumentDriver()`
- Simulation driver assigned via `ConfigureDriverSessionSimulationDriver()`

**Key Insight:**  
A single driver session can reference either:
- A **real driver** (e.g., `niDmm`) – controls actual hardware
- A **simulation driver** (e.g., `nisDmm`) – emulates hardware without real instruments

---

### 3. **Logical Name**

A **Logical Name** is the **user-friendly alias** that applications use when opening an instrument. Instead of hardcoding driver names, applications reference logical names.

**Example:**
- Application code: `ViSession session; viOpen("MyDmm", VI_FALSE, VI_FALSE, &session);`
- Logical Name `"MyDmm"` resolves to: Driver session `"cviDmm"` → Software module `"niDmm"`
- Result: Application connects to the instrument

**Benefits:**
- Swap drivers without changing code
- Support multi-site deployments (different hardware per site)
- Enable simulation testing (same code, different logical name)

**In this project:**
- Created via `GetOrCreateLogicalName()`
- Linked to driver session via `LinkLogicalNameToSession()`

---

### 4. **Configuration Data Components**

**Data Components** are typed configuration values stored within driver sessions. They form a **hierarchical tree** (like JSON/XML).

**Example Structure:**
```
DriverSession "cviDmm"
  └─ NI Settings (structure)
      ├─ Simulation Driver Session (string) = "nisDmm"
      └─ Use Specific Simulation (boolean) = FALSE
```

**In this project:**
- Created via `GetOrCreateDataComponent()`
- Organized via `GetOrCreateStructure()`
- Set via `SetStringValue()` and `SetBooleanValue()`

**Why it matters:**
- Driver-specific settings stored outside code
- Runtime reconfigurable
- IVI ConfigServer manages serialization to/from XML

---

## 🔀 Configuration Topology: Simulated vs. Real

### Simulated Configuration
```
Logical Name "MyDmm"
       │
       └─→ Driver Session "cviDmm"
              │
              ├─ Software Module: "nisDmm" (simulation driver)
              │
              └─ Data: Simulation Driver Session = "nisDmm"
```

**Characteristics:**
- No real hardware needed
- Uses simulation driver
- Data components specify simulation settings

---

### Real Configuration
```
Logical Name "MyScope"
       │
       └─→ Driver Session "cviScope"
              │
              ├─ Software Module: "niScope" (real driver)
              │
              └─ Hardware Resource: "GPIB::1::INSTR"
```

**Characteristics:**
- Real hardware required
- Uses production driver
- Resource string specifies hardware address

---

## 🔄 Idempotent Design  

---

## 🔧 Main Workflow (Simplified)

```c
CHECK(LoadConfigurationStore(&store));

CreateNewConfiguration(... simulated config ...);
CreateNewConfiguration(... real config ...);

CHECK(SaveConfiguration(store, fallbackPath));
```

Inside `CreateNewConfiguration()`:

```c
GetOrCreateDriverSession(...)
ConfigureDriverSessionProperties(...)

if (isSimulated)
    ConfigureDriverSessionSimulationDriver(...)
else
    GetOrCreateSoftwareModule(...)
    ConfigureDriverSessionInstrumentDriver(...)

GetOrCreateLogicalName(...)
LinkLogicalNameToSession(...)
```

---

## 🧰 Helper API Overview

### 🔹 Error Handling

```c
int HandleError(ViStatus status);
```

- Centralized error checking
- Prints IVI error messages
- Used via `CHECK()` macro

---

### 🔹 Core Operations

#### Configuration Store

```c
ViStatus LoadConfigurationStore(...)
```

Loads IVI config store (MAX)

---

#### Driver Session

```c
ViStatus GetOrCreateDriverSession(...)
ViStatus ConfigureDriverSessionProperties(...)
```

**IVI Functions Wrapped:**
- `IviConfig_GetConfigStoreDriverSessionCollection()` – Access the driver session collection
- `IviConfig_GetDriverSessionItemByName()` – Lookup existing session
- `IviConfig_CreateDriverSession()` – Create if not found
- `IviConfig_SetDriverSessionPropertyViBoolean()` – Set properties

**What Gets Configured:**
- `IVICONFIG_VAL_DRIVER_SESSION_SIMULATE` = TRUE
- `IVICONFIG_VAL_DRIVER_SESSION_CACHE` = TRUE  
- `IVICONFIG_VAL_DRIVER_SESSION_RANGE_CHECK` = TRUE

**Impact on IVI Runtime:**
When an application opens this session, it will:
- Run in simulation mode (no real hardware)
- Cache attribute values (better performance)
- Validate all range-check operations

> ⚠️ Note: Simulation is always enabled at property level, even for "real" config. The distinction is made by which driver module you assign (real vs. simulation driver).

---

#### Software Module

```c
ViStatus GetOrCreateSoftwareModule(...)
ViStatus ConfigureDriverSessionInstrumentDriver(...)
```

**IVI Functions Wrapped:**
- `IviConfig_GetConfigStoreSoftwareModuleCollection()` – Access module registry
- `IviConfig_GetSoftwareModuleItemByName()` – Lookup existing module
- `IviConfig_CreateSoftwareModule()` – Create if not found
- `IviConfig_SetSessionSoftwareModuleReference()` – Link session → module

**What Happens:**
When you link a software module to a driver session, the IVI runtime will:
1. Load the module DLL/library
2. Initialize the IVI driver from that module
3. Use that driver's implementation for all VISA calls

**Example:**
- Link `niDmm` → Real NI DMM driver loads
- Link `nisDmm` → NI Simulation DMM driver loads (fake measurements)

---

#### Simulation Configuration

```c
ViStatus ConfigureDriverSessionSimulationDriver(...)
```

**IVI Functions Wrapped:**
- `IviConfig_GetConfigComponentDataComponentCollection()` – Access session config data
- `IviConfig_GetStructureDataComponentCollection()` – Access nested structures
- `IviConfig_CreateStructure()` – Create "NI Settings" hierarchy
- `IviConfig_CreateDataComponent()` – Create individual settings
- `IviConfig_SetDataComponentPropertyViString()` – Store string values
- `IviConfig_SetDataComponentPropertyViBoolean()` – Store boolean values

**Data Structure Created:**
```xml
<DriverSession name="cviDmm">
  <NI Settings>
    <Simulation Driver Session>nisDmm</Simulation Driver Session>
    <Use Specific Simulation>FALSE</Use Specific Simulation>
  </NI Settings>
</DriverSession>
```

**Purpose:**
These configuration data components tell the IVI framework:
- Which simulation driver to use
- How to configure the simulation behavior

---

#### Logical Name

```c
ViStatus GetOrCreateLogicalName(...)
ViStatus LinkLogicalNameToSession(...)
```

**IVI Functions Wrapped:**
- `IviConfig_GetConfigStoreLogicalNameCollection()` – Access logical name registry
- `IviConfig_GetLogicalNameItemByName()` – Lookup existing name
- `IviConfig_CreateLogicalName()` – Create if not found
- `IviConfig_SetLogicalNameSessionReference()` – Link name → session

**Usage in Applications:**
```c
// Application code (not in this project)
ViSession handle;
viOpen("MyDmm", VI_FALSE, VI_FALSE, &handle);  // Opens logical name
```

**What Happens Under the Hood:**
1. IVI Runtime looks up logical name `"MyDmm"` in config store
2. Finds driver session `"cviDmm"`
3. Loads the software module linked to that session
4. Establishes connection, returns `handle`

---

#### Persistence

```c
ViStatus SaveConfiguration(...)
```

**IVI Functions Wrapped:**
- `IviConfig_Serialize()` – Write configuration to XML file

**Output:**
Writes to `C:\ProgramData\IVI Foundation\IVI\IviConfigurationStore.xml`:
```xml
<?xml version="1.0" encoding="utf-8"?>
<IviConfigurationStore>
  <DriverSessions>
    <DriverSession name="cviDmm">
      <!-- session config -->
    </DriverSession>
  </DriverSessions>
  <LogicalNames>
    <LogicalName name="MyDmm">
      <!-- references cviDmm -->
    </LogicalName>
  </LogicalNames>
  <!-- more entities -->
</IviConfigurationStore>
```

**Fallback Mechanism:**
If the default path fails (permissions, path doesn't exist), tries user-supplied fallback path.

---

### 🔹 Low-Level Reusable Helpers

These eliminate repetitive IVI API calls:

- `GetOrCreateStructure` – Wraps `IviConfig_GetDataComponentItemByName()` + `IviConfig_CreateStructure()`
- `GetOrCreateDataComponent` – Wraps `IviConfig_GetDataComponentItemByName()` + `IviConfig_CreateDataComponent()`
- `SetStringValue` – Wraps `IviConfig_SetDataComponentPropertyViString()`
- `SetBooleanValue` – Wraps `IviConfig_SetDataComponentPropertyViBoolean()`

---

## 🚀 Build & Run

### Requirements

- NI IVI Drivers installed
- NI Measurement & Automation Explorer (MAX)

### Compile

Use:
- LabWindows/CVI
- Visual Studio (with IVI libraries)

### Run

```
Run program → Configurations created/updated safely
```

---

## 🔍 Relationship to Measurement & Automation Explorer (MAX)

**What is MAX?**  
MAX is the graphical tool for managing IVI configurations. It provides:
- Driver session editor
- Logical name editor
- Software module registry browser
- Configuration visualization

**This Project vs. MAX:**

| Task | Using MAX | Using This Project |
|------|-----------|-------------------|
| Create one configuration | GUI clicks (1 min) | Program runs (instant) |
| Create 10 configurations | GUI clicks × 10 (10 min) | Program runs once (instant) |
| Automate setup | Not possible | ✅ Scriptable |
| CI/CD integration | Not possible | ✅ Supported |
| Version control | Manual copying | ✅ Commit to git |
| Verify syntax | Manual review | ✅ Compile-time checked |

**Key Point:**  
Both MAX and this project modify the **same XML file** (`IviConfigurationStore.xml`). The difference is:
- MAX: Interactive GUI-based editor
- This project: Programmatic API-based editor

You can:
- Create a configuration with MAX, read it with this code
- Create a configuration with this code, inspect it in MAX
- Combine both approaches

**In MAX, this configuration looks like:**
```
My Instrument Configuration
├── Driver Sessions
│   ├── cviDmm (simulated)
│   └── cviScope (real)
├── Logical Names
│   ├── MyDmm → cviDmm
│   └── MyScope → cviScope
└── Software Modules
    ├── nisDmm (simulation)
    └── niScope (real)
```

---

## 📚 IVI Configuration Store Format

The underlying storage is a **single XML file** containing all configurations:

**Location:** `C:\ProgramData\IVI Foundation\IVI\IviConfigurationStore.xml`

**Example Structure:**
```xml
<?xml version="1.0" encoding="utf-8"?>
<IviConfigurationStore Version="1">
  <DriverSessions>
    <DriverSession name="cviDmm">
      <Properties>
        <Property name="Simulate" value="true"/>
        <Property name="Cache" value="true"/>
        <Property name="RangeCheck" value="true"/>
      </Properties>
      <ConfigurationData>
        <NI_Settings>
          <SimulationDriverSession type="string">nisDmm</SimulationDriverSession>
        </NI_Settings>
      </ConfigurationData>
    </DriverSession>
  </DriverSessions>
  
  <LogicalNames>
    <LogicalName name="MyDmm">
      <Reference DriverSessionName="cviDmm"/>
    </LogicalName>
  </LogicalNames>
  
  <SoftwareModules>
    <SoftwareModule name="nisDmm" Type="DriverModule"/>
    <SoftwareModule name="niDmm" Type="DriverModule"/>
  </SoftwareModules>
</IviConfigurationStore>
```

**This Project Programmatically Builds:** All elements of this XML structure using `IviConfigServer.lib` APIs.

---

## 🚀 Build & Run

### Requirements

- NI IVI Drivers installed
- NI Measurement & Automation Explorer (MAX)

### Compile

Use:
- LabWindows/CVI
- Visual Studio (with IVI libraries)

### Run

```
Run program → Configurations created/updated safely
```

---

## 💡 Example Output

```
Configuration Store Loaded!

Creating Configuration 1
========================
Creating driver session...
Creating logical name...

Creating Configuration 2
========================
Creating driver session...
Creating software module...
Creating logical name...

IVI Configuration Saved Successfully!
Done.
```

---

## 🎯 Practical Use Cases

### When to Use This Programmatic Approach

✅ **Automated Test Lab Setup**  
Deploy identical configurations across 50 test stations with one script run.

✅ **CI/CD Test Automation**  
Spin up fresh instrument configurations as part of test pipeline initialization.

✅ **Multi-Site Deployments**  
Generate site-specific configurations (different hardware per location) from a template.

✅ **Dynamic Reconfiguration**  
Swap between simulation and real hardware at runtime based on availability.

✅ **Configuration Version Control**  
Store configurations in git; generate XML on demand instead of binary MAX files.

✅ **Instrument Farm Management**  
Manage hundreds of instruments programmatically; impossible with MAX GUI.

### When to Use MAX Instead

✅ One-time manual configuration  
✅ Exploring available drivers and modules  
✅ Troubleshooting connection issues  
✅ Verifying configurations visually  

**Best Practice:** Use MAX for exploration and validation, then automate repetitive tasks with code like this project.

---

## 📈 Future Improvements

- Toggle simulation property based on configuration type
- Add hardware asset configuration
- Introduce logging levels (INFO / DEBUG / ERROR)
- Convert to C++ (RAII cleanup)
- Add unit tests for helper functions

---

## ✅ Summary

This project shows how to:

✅ Structure IVI configuration code cleanly  
✅ Wrap complex APIs into reusable helpers  
✅ Implement safe, idempotent configuration logic  
✅ Separate orchestration from implementation  

> "Clean code is code that is easy to understand and easy to change." – Robert C. Martin