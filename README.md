# IVI Configuration Builder (Clean Code Refactor)

This project demonstrates how to programmatically create and manage an IVI (Interchangeable Virtual Instrument) configuration using the NI IVI Configuration Server API in C.

The implementation follows **Uncle Bob's Clean Code principles**, focusing on:
- Readability
- Separation of concerns
- Reusability
- Idempotent operations (safe to run multiple times)

---

## 📁 Project Structure

```
.
├── main.c            # Application entry point (high-level orchestration)
├── ivi_helpers.h     # Public API for helper utilities
├── ivi_helpers.c     # Implementation of reusable IVI logic
```

---

## 🧠 Design Philosophy

This project is structured around two key layers:

### 1. High-Level Orchestration (`main.c`)
- Describes the workflow of building an IVI configuration
- Contains minimal logic
- Reads like a **story**

### 2. Helper Layer (`ivi_helpers.c`)
- Encapsulates all IVI API interactions
- Provides reusable, testable functions
- Handles “get-or-create” patterns safely

---

## ✅ What the Application Does

The program:

1. Connects to the IVI Configuration Store (MAX)
2. Creates or reuses:
   - Software Module
   - Driver Session
   - Logical Name
3. Configures the Driver Session for simulation
4. Adds simulation-specific data components
5. Links Logical Name → Driver Session
6. Saves the configuration

---

## 🔁 Idempotent Design

All "create" operations follow a **get-or-create pattern**, meaning:

✅ Running the program multiple times will NOT:
- Duplicate entries
- Cause errors
- Corrupt configuration

---

## 🔧 Main Workflow (`main.c`)

The `main()` function is intentionally simple:

```c
CHECK(Ivi_GetConfigStoreHandle(&store));

CHECK(GetOrCreateSoftwareModule(store, MODULE, &module));
CHECK(GetOrCreateDriverSession(store, SESSION, &session));

CHECK(ConfigureDriverSession(session));
CHECK(ConfigureSimulationData(session, MODULE));

CHECK(IviConfig_SetSessionSoftwareModuleReference(session, module));

CHECK(GetOrCreateLogicalName(store, LOGICAL, &logical));
CHECK(LinkLogicalName(logical, session));

SaveConfiguration(store, FALLBACK);
```

👉 This reflects the desired Clean Code outcome:
> "Code should read like well-written prose."

---

## 🧰 Helper API Overview (`ivi_helpers.h`)

### 🔹 Error Handling

```c
int HandleError(ViStatus status);
```

- Centralized error reporting
- Logs IVI error messages

---

### 🔹 High-Level Operations

#### Software Module

```c
ViStatus GetOrCreateSoftwareModule(...);
```

Gets an existing module or creates it if missing.

---

#### Driver Session

```c
ViStatus GetOrCreateDriverSession(...);
```

Creates or reuses a session and prepares it for configuration.

---

#### Logical Name

```c
ViStatus GetOrCreateLogicalName(...);
```

Creates or retrieves a logical alias for a driver session.

---

#### Configuration

```c
ViStatus ConfigureDriverSession(...);
```

Sets:
- Simulation ON
- Caching ON
- Range checking ON

---

```c
ViStatus ConfigureSimulationData(...);
```

Creates or updates structured simulation settings:

```
NI Settings
├── Simulation Driver Session (string)
└── Use Specific Simulation (boolean)
```

---

#### Linking

```c
ViStatus LinkLogicalName(...);
```

Links:
```
Logical Name → Driver Session
```

---

#### Persistence

```c
void SaveConfiguration(...);
```

- Attempts default save location (MAX)
- Falls back to user-defined path if needed

---

## 🔹 Low-Level Reusable Helpers

These eliminate repeated logic:

### Structure handling

```c
GetOrCreateStructure(...)
```

---

### Data components

```c
GetOrCreateDataComponent(...)
```

---

### Value setters

```c
SetStringValue(...)
SetBooleanValue(...)
```

---

## ✅ Key Improvements from Original Code

| Problem | Solution |
|--------|--------|
| Large `main()` | Split into small functions |
| Code duplication | Reusable helpers |
| Magic strings | Central constants |
| Unsafe re-runs | Idempotent logic |
| Mixed responsibilities | Clear separation of layers |

---

## 🚀 Build & Run

### Requirements
- NI IVI Drivers installed
- NI Measurement & Automation Explorer (MAX)

### Compile
Use your preferred compiler/project setup (e.g., LabWindows/CVI or Visual Studio).

### Run
Simply execute the program:
```
Run → Program creates/updates config safely
```

---

## 💡 Example Output

```
=== IVI DMM Config ===
Using existing driver session.
Structure 'NI Settings' exists.
Component 'Simulation Driver Session' exists.
Done.
```

---

## 📈 Future Improvements

Possible next steps:

- Convert to C++ (RAII for automatic cleanup)
- Add unit tests for helper layer
- Create a configuration "builder" API
- Add logging levels (INFO / ERROR / DEBUG)

---

## ✅ Summary

This project demonstrates how to:

✅ Write clean, maintainable C code  
✅ Wrap complex APIs into reusable layers  
✅ Apply professional software design to hardware/configuration code  

---

> "Clean code is not written by following a set of rules.  
> Clean code is written by programmers who care." – Robert C. Martin