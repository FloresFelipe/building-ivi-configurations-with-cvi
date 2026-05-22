## IVI Configuration Builder (Clean Code Refactor)

This project demonstrates how to programmatically create and manage IVI (Interchangeable Virtual Instrument) configurations using the NI IVI Configuration Server API in C.

It follows **Clean Code principles**:
- Readability
- Separation of concerns
- Reusability
- Idempotent operations (safe re-execution)

---

## 📁 Project Structure

```
.
├── main.c           # High-level orchestration
├── ivi_helpers.h    # Public helper API
├── ivi_helpers.c    # IVI logic implementation
```

---

## 🧠 Design Overview

The project is divided into two clear layers:

### 1. High-Level Orchestration (`main.c`)
- Defines the workflow
- Creates configurations
- Uses helper functions only
- Minimal logic ("reads like a story")

### 2. Helper Layer (`ivi_helpers.c`)
- Encapsulates IVI API calls
- Implements reusable functions
- Handles "get-or-create" safely
- Manages configuration details

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

Configures:
- Simulation = ON
- Caching = ON
- Range checking = ON

> ⚠️ Note: Simulation is always enabled at property level, even for "real" config.

---

#### Software Module

```c
ViStatus GetOrCreateSoftwareModule(...)
ViStatus ConfigureDriverSessionInstrumentDriver(...)
```

Used for real instrument configurations.

---

#### Simulation Configuration

```c
ViStatus ConfigureDriverSessionSimulationDriver(...)
```

Creates structure:

```
NI Settings
├── Simulation Driver Session (string)
└── Use Specific Simulation (boolean)
```

---

#### Logical Name

```c
ViStatus GetOrCreateLogicalName(...)
ViStatus LinkLogicalNameToSession(...)
```

Links:
```
Logical Name → Driver Session
```

---

#### Persistence

```c
ViStatus SaveConfiguration(...)
```

- Tries default IVI config path
- Falls back to user path if needed

---

### 🔹 Low-Level Reusable Helpers

These eliminate repetitive IVI API calls:

- `GetOrCreateStructure`
- `GetOrCreateDataComponent`
- `SetStringValue`
- `SetBooleanValue`

---

## ⚠️ Important Behavioral Notes

- Driver sessions are always configured with:
  - Simulation = TRUE
  - Even for "real" configurations
- The distinction between simulated vs real is done by:
  - Adding simulation data OR
  - Assigning a software module

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