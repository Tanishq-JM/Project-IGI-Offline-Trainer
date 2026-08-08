<div align="center">

# IGI Offline Trainer

### A childhood mission I could never quite finish. A curiosity that never quite left.

</div>

---

## Remember Border Crossing?

<div align="center">

<!-- THE hero shot. Get to the chokepoint where the Dragunov guard sits and grab your screenshot there — that's the moment this whole project is named after. -->
<img src="assets/images/border-crossing-hero.JPG" alt="Border Crossing — the mission that started it all" width="800">

</div>

Project I.G.I. has been one of my favorite games since I first played it back in 2008. Like a lot of players, I kept hitting missions that felt almost impossible to get through on a kid's patience and a kid's reflexes. The one I remember most is **Border Crossing** — the single most terrifying mission of all time, where one particular guard holding a Dragunov got killed roughly one million times by yours truly. 😂

I'd creep through it carefully — searching for a safe route, rationing every bullet, trying not to take a single unnecessary hit. And then, without fail, that helicopter would show up. Circling overhead. Hunting.

What made it across the line, mission after mission:

- Limited health
- Reduced ammunition
- Very little cover
- One specific guard with a Dragunov who apparently could not be killed enough times to make peace with it
- The helicopter attacking from above, and a tank hunting you down — it felt less like a mission and more like a mission-fail simulator
- Another restart
- Another attempt at crossing the exact same stretch of dangerous ground

That memory stayed with me for years, long after I'd stopped actively playing.

Much later, instead of just going looking for another shortcut or a walkthrough, I got curious about something else entirely: *how does the game actually store all of this?* Where does it keep player health, accumulated damage, magazine ammunition, inventory quantities? That curiosity is what turned into my first real reverse-engineering project.

What started as a childhood gaming grudge slowly became a full technical learning journey through:

- Runtime memory observation
- Static analysis
- Pointer-chain discovery
- Health and damage calculation
- Inventory-record analysis
- Magazine-state research
- Defensive memory validation
- Native Windows interface development
- Direct2D rendering
- Build automation
- Public-source documentation

**IGI Offline Trainer** was built out of that journey.

It's meant for players who remember getting stuck at sections like Border Crossing and want to go back through the game on their own terms — a calmer, offline, single-player experience. It's also, honestly, just a personal tribute to a childhood game that stuck with me far longer than I expected.

> A favorite childhood game. A brutal border crossing. A helicopter that never let up. One unforgettable mission that, years later, became my first reverse-engineering project.

<div align="center">

<!-- Add a personally recorded, lawful offline gameplay screenshot here. Remove usernames, folder paths, and any desktop notifications before uploading. -->
<img src="assets/images/border-crossing-01.JPG" alt="The Approach — entering the exposed section of Border Crossing" width="720">

**The Approach**
*Carefully entering the exposed section of Border Crossing.*

<br>

<!-- Add a personally recorded, lawful offline gameplay screenshot here. -->
<img src="assets/images/border-crossing-02.JPG" alt="That Helicopter — circling above and constantly attacking" width="720">

**That Helicopter**
*Circling above, and constantly attacking.*

<br>

<!-- Add a personally recorded, lawful offline gameplay screenshot here. -->
<img src="assets/images/border-crossing-03.JPG" alt="Stuck Again — limited health, limited ammunition, another attempt to survive" width="720">

**Stuck Again**
*Limited health, limited ammunition, and another attempt to survive.*

</div>

These moments are the experience that inspired the trainer: pushing through the mission, losing health, running short on ammunition, hunting for cover, and getting stopped by the same helicopter again and again.

> *All screenshots above should come from personally recorded, lawful offline gameplay. Before publishing, remove usernames, folder paths, desktop notifications, or any other private information.*

<div align="center">

<!-- Add a short gameplay GIF of Border Crossing here once recorded — see the "Media and Visual Assets" section below for size/format guidance. -->
<img src="assets/gifs/border-crossing-demo.gif" alt="Border Crossing gameplay demo GIF — add your recording here" width="720">

*Space reserved for a Border Crossing gameplay GIF — drop yours in at `assets/gifs/border-crossing-demo.gif`.*

</div>

---

<div align="center">

## Professional GPU-Rendered Offline Gameplay Utility

*Minimal Direct2D interface for health protection, magazine auto-fill, and dynamic inventory management in an authorized offline Project I.G.I. installation*

**Direct2D • C++20 • Win32 • x86 • Visual Studio 2026 • No Game Files Included**

[Download the Latest Compiled Release](../../releases/latest)

</div>

---

## Overview

IGI Offline Trainer is a professional Windows desktop utility designed for authorized offline, single-player experimentation with Project I.G.I. The application combines a minimal GPU-rendered Direct2D interface with persistent gameplay policies, strict memory validation, dynamic pointer resolution, live LED status indicators, and per-feature correction counters.

The C++20 implementation preserves the operational behavior confirmed by the author's working Python v2 prototype and technical research. It is configured for the specific 32-bit executable layout used during development. Runtime heap addresses may move between missions, so the application resolves the required object chains continuously rather than storing final addresses permanently.

> **Offline use only:** This project must not be used with networked games, competitive environments, anti-cheat systems, unauthorized copies, third-party computers, or remote services.

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#EEF2FF','primaryTextColor':'#312E81','primaryBorderColor':'#A5B4FC','lineColor':'#818CF8','secondaryColor':'#ECFEFF','tertiaryColor':'#ECFDF5','noteBkgColor':'#FEF3C7','noteTextColor':'#78350F','noteBorderColor':'#FCD34D'}}}%%
flowchart TB
    Start([Trainer Launch]) --> UI[Initialize Win32 Window]
    UI --> GPU[Create Direct2D and DirectWrite Resources]
    GPU --> Worker[Start Background Trainer Worker]

    Worker --> Detect{igi.exe Running?}
    Detect -->|No| Waiting[Amber LED: Waiting for Game]
    Waiting --> Detect
    Detect -->|Yes| Attach[Open Authorized Local Process]

    Attach --> Module[Resolve Module Base]
    Module --> Player{Player Object Valid?}
    Player -->|No| MissionWait[Amber LED: Start or Resume Mission]
    MissionWait --> Player
    Player -->|Yes| Active[Green Status: Mission Active]

    Active --> Policies{Enabled Policies}
    Policies --> Health[Invincible Policy]
    Policies --> Magazine[Magazine Auto-Fill Policy]
    Policies --> Inventory[Inventory Auto-Max Policy]

    Health --> ValidateHealth[Validate Damage and Capacity]
    ValidateHealth --> HealthWrite[Write Damage = 0]

    Magazine --> ResolveMag[Resolve Current Magazine Chain]
    ResolveMag --> ValidateMag[Validate Plausible Range]
    ValidateMag --> MagWrite[Write Magazine Target]

    Inventory --> ReadTable[Read Runtime Record Count]
    ReadTable --> ValidateTable[Validate 0 to 64 Records]
    ValidateTable --> InventoryWrite[Update Current Fields Only]

    HealthWrite --> Counters[Increment Session Counters]
    MagWrite --> Counters
    InventoryWrite --> Counters
    Counters --> Render[Render LED State and Statistics]
    Render --> Active

    style Start fill:#EEF2FF,stroke:#818CF8,stroke-width:3px,color:#312E81
    style Active fill:#ECFDF5,stroke:#34D399,stroke-width:3px,color:#065F46
    style Detect fill:#FEF3C7,stroke:#FCD34D,stroke-width:2px,color:#78350F
    style Player fill:#FEF3C7,stroke:#FCD34D,stroke-width:2px,color:#78350F
    style Policies fill:#FDF2F8,stroke:#F9A8D4,stroke-width:2px,color:#831843
    style GPU fill:#ECFEFF,stroke:#22D3EE,stroke-width:2px,color:#164E63
    style Counters fill:#EEF2FF,stroke:#A5B4FC,stroke-width:2px,color:#312E81
```

---

## Key Features

### **Persistent Gameplay Policies**

- **F1 Invincible:** Maintains the confirmed accumulated-damage field at zero while enabled
- **F2 Magazine Auto-Fill:** Maintains the current weapon magazine at the configured target
- **F3 Inventory Auto-Max:** Continuously updates every valid runtime inventory record
- **F12 Emergency Disable:** Immediately disables all active gameplay policies
- Policies remain enabled through temporary loading, weapon switching, and object-transition states
- Invalid data skips only the current polling cycle instead of disabling the selected feature

### **Minimal GPU-Rendered Interface**

- Native Win32 desktop application
- Direct2D GPU-accelerated card rendering
- DirectWrite typography
- No browser engine, Electron runtime, or embedded web view
- Three focused feature cards with hotkey labels
- Live process, mission, health, feature, and session-correction status
- Compact About dialog with scope and authorship information

### **Professional LED Status System**

- **Green:** Feature enabled and latest runtime state validated successfully
- **Amber:** Feature enabled but temporarily waiting for a valid mission object or transition
- **Gray:** Feature disabled
- **Red:** Reserved for persistent access or compatibility failure
- Session counters display successful corrections performed by each feature

### **Defensive Runtime Validation**

- Every pointer is checked before dereferencing
- Runtime player object is resolved repeatedly
- Inventory count is limited to a conservative safe range
- Health values must be finite and capacity must be positive
- Magazine values must remain inside a plausible range
- Inventory ID and maximum fields are never modified
- Only independently confirmed mutable fields are eligible for writes

### **Privacy-First Local Operation**

- No telemetry
- No remote analytics
- No account system
- No cloud dependency
- No remote logging
- No network service
- No DLL injection
- No hooks
- No game executables or proprietary game assets included

---

## Interface Experience

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#ECFEFF','primaryTextColor':'#164E63','primaryBorderColor':'#67E8F9','lineColor':'#818CF8','secondaryColor':'#EEF2FF','tertiaryColor':'#ECFDF5'}}}%%
graph LR
    subgraph Header[Application Header]
        Brand[IGI Trainer]
        Scope[Offline Single-Player Utility]
        ProcessLED[Live Process LED]
    end

    subgraph Cards[Feature Cards]
        Invincible[Invincible Card<br/>F1 and Correction Count]
        Magazine[Magazine Auto-Fill Card<br/>F2 and Refill Count]
        Inventory[Inventory Auto-Max Card<br/>F3 and Record Update Count]
    end

    subgraph Footer[Status and Actions]
        Health[Calculated Health Percentage]
        About[About Dialog]
        Exit[Safe Exit]
    end

    Header --> Cards
    Cards --> Footer

    style Brand fill:#EEF2FF,stroke:#A5B4FC,stroke-width:2px,color:#312E81
    style Scope fill:#ECFEFF,stroke:#67E8F9,stroke-width:2px,color:#164E63
    style ProcessLED fill:#ECFDF5,stroke:#6EE7B7,stroke-width:2px,color:#065F46
    style Invincible fill:#FDF2F8,stroke:#F9A8D4,stroke-width:2px,color:#831843
    style Magazine fill:#FEF3C7,stroke:#FCD34D,stroke-width:2px,color:#78350F
    style Inventory fill:#EEF2FF,stroke:#A5B4FC,stroke-width:2px,color:#312E81
    style Health fill:#ECFDF5,stroke:#6EE7B7,stroke-width:2px,color:#065F46
```

### Minimal UI Layout

```text
┌────────────────────────────────────────────────┐
│  IGI TRAINER                         ● READY   │
│  Offline single-player utility                 │
├────────────────────────────────────────────────┤
│  ●  INVINCIBLE                           [F1]  │
│     Maintains accumulated damage at zero       │
│                               18 corrections   │
├────────────────────────────────────────────────┤
│  ●  MAGAZINE AUTO-FILL                   [F2]  │
│     Maintains the active magazine target       │
│                               73 corrections   │
├────────────────────────────────────────────────┤
│  ●  INVENTORY AUTO-MAX                   [F3]  │
│     Maintains active inventory quantities      │
│                              156 corrections   │
├────────────────────────────────────────────────┤
│  HEALTH 100.0%       Click a card or hotkey    │
│  [ About ]                              [ Exit ]│
└────────────────────────────────────────────────┘
```

### Live Demo GIFs

<div align="center">

<!-- Add your recorded trainer demo GIFs here. Suggested size: 960x540, 15-20 FPS. See "Media and Visual Assets" below for the exact ImageMagick command. -->

<img src="assets/gifs/trainer-overview.gif" alt="Trainer overview demo GIF — add your recording here" width="720">

*Overview: launching the trainer, attaching to the game, and the live LED status.*

<br>

<img src="assets/gifs/invincible-demo.gif" alt="Invincible feature demo GIF — add your recording here" width="360"> <img src="assets/gifs/magazine-demo.gif" alt="Magazine Auto-Fill demo GIF — add your recording here" width="360">

*Left: Invincible in action. Right: Magazine Auto-Fill surviving a weapon switch.*

<br>

<img src="assets/gifs/inventory-demo.gif" alt="Inventory Auto-Max demo GIF — add your recording here" width="720">

*Inventory Auto-Max keeping every carried item topped up in real time.*

</div>

---

## System Requirements

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#FDF2F8','primaryTextColor':'#831843','primaryBorderColor':'#F9A8D4','lineColor':'#F9A8D4','secondaryColor':'#ECFEFF','tertiaryColor':'#FEF3C7'}}}%%
graph LR
    subgraph Runtime[Runtime Requirements]
        OS[Windows 10 or 11<br/>64-bit Host]
        Game[Lawfully Obtained<br/>32-bit Project I.G.I.]
        Mode[Offline Single-Player<br/>Mission]
    end

    subgraph Build[Build Requirements]
        VS[Visual Studio Community 2026]
        Workload[Desktop Development with C++]
        MSVC[MSVC x64 and x86 Tools]
        SDK[Windows 11 SDK 10.0.26100]
        CMake[CMake 3.21 or Newer]
        Git[Git for Repository Operations]
    end

    Runtime --> Build

    style OS fill:#E0F2FE,stroke:#7DD3FC,stroke-width:2px,color:#075985
    style Game fill:#FDF2F8,stroke:#F9A8D4,stroke-width:2px,color:#831843
    style Mode fill:#ECFDF5,stroke:#6EE7B7,stroke-width:2px,color:#065F46
    style VS fill:#EEF2FF,stroke:#A5B4FC,stroke-width:2px,color:#312E81
    style Workload fill:#FEF3C7,stroke:#FCD34D,stroke-width:2px,color:#78350F
    style MSVC fill:#ECFEFF,stroke:#67E8F9,stroke-width:2px,color:#164E63
    style SDK fill:#ECFDF5,stroke:#6EE7B7,stroke-width:2px,color:#065F46
    style CMake fill:#EEF2FF,stroke:#A5B4FC,stroke-width:2px,color:#312E81
    style Git fill:#FDF2F8,stroke:#F9A8D4,stroke-width:2px,color:#831843
```

### Required Visual Studio Components

Open **Visual Studio Installer**, select **Modify** for Visual Studio Community 2026, and verify:

- **Desktop development with C++** workload
- MSVC C++ x64/x86 build tools
- C++ CMake tools for Windows
- Windows 11 SDK `10.0.26100.0` or a compatible installed SDK
- C++ core desktop features
- Windows Resource Compiler

### Build Environment Verification

Use **Developer PowerShell for Visual Studio 2026** and run:

```powershell
where.exe cmake
where.exe cl
where.exe link
where.exe rc
cmake --version
cmake --help | Select-String "Visual Studio"
```

The required compiler target is **Win32/x86**, because the supported game executable and documented pointer model use the 32-bit address space.

---

## Build Process

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#EEF2FF','primaryTextColor':'#312E81','primaryBorderColor':'#A5B4FC','lineColor':'#818CF8','secondaryColor':'#ECFEFF','tertiaryColor':'#ECFDF5'}}}%%
sequenceDiagram
    autonumber
    participant Developer
    participant VS as Visual Studio 2026 Environment
    participant CMake
    participant MSVC as MSVC Compiler
    participant SDK as Windows SDK
    participant Output as Release Output

    Developer->>VS: Open Developer PowerShell
    VS->>SDK: Load compiler and SDK environment
    Developer->>CMake: Configure Visual Studio 18 2026 Win32
    CMake->>MSVC: Detect C++20 compiler
    CMake->>SDK: Detect Direct2D, DirectWrite and Resource Compiler
    SDK-->>CMake: Headers and libraries available
    MSVC-->>CMake: Compiler available
    CMake-->>Developer: Configuration generated
    Developer->>CMake: Build Release target
    CMake->>MSVC: Compile source modules
    MSVC->>SDK: Link Win32, Direct2D and DirectWrite libraries
    SDK-->>Output: Create IGI-Offline-Trainer.exe
    Output-->>Developer: Release executable ready
```

### Recommended Local Build

Open **Developer PowerShell for Visual Studio 2026**:

```powershell
cd "FULL\PATH\TO\IGI-Offline-Trainer"

cmake -S . -B build -G "Visual Studio 18 2026" -A Win32
cmake --build build --config Release --parallel
```

Expected executable:

```text
build\bin\Release\IGI-Offline-Trainer.exe
```

### CMake Preset Build

```powershell
cmake --preset windows-x86-release
cmake --build --preset release
```

Expected executable:

```text
out\build\x86-release\bin\Release\IGI-Offline-Trainer.exe
```

### Clean Rebuild

```powershell
if (Test-Path ".\build") {
    Remove-Item ".\build" -Recurse -Force
}

cmake -S . -B build -G "Visual Studio 18 2026" -A Win32
cmake --build build --config Release --parallel
```

---

## Usage Workflow

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#FDF2F8','primaryTextColor':'#831843','primaryBorderColor':'#F9A8D4','lineColor':'#A5B4FC','secondaryColor':'#EEF2FF','tertiaryColor':'#ECFDF5'}}}%%
stateDiagram-v2
    [*] --> StartGame
    StartGame: Start a lawful offline Project I.G.I. mission
    StartGame --> LaunchTrainer
    LaunchTrainer: Launch IGI-Offline-Trainer.exe
    LaunchTrainer --> Waiting
    Waiting: Amber status while locating igi.exe
    Waiting --> Attached: Process detected
    Attached --> MissionActive: Player object resolved
    MissionActive --> SelectFeatures

    state SelectFeatures {
        [*] --> Idle
        Idle --> Invincible: F1 or select card
        Idle --> Magazine: F2 or select card
        Idle --> Inventory: F3 or select card
        Invincible --> Idle: Toggle off
        Magazine --> Idle: Toggle off
        Inventory --> Idle: Toggle off
    }

    SelectFeatures --> EmergencyDisable: F12
    EmergencyDisable --> SelectFeatures
    SelectFeatures --> ExitTrainer: Exit selected
    ExitTrainer --> [*]
```

### Step-by-Step Usage

1. Start a lawful offline Project I.G.I. mission.
2. Launch `IGI-Offline-Trainer.exe`.
3. Wait for the process status to indicate that `igi.exe` is attached.
4. Wait for the mission state to become active.
5. Select a feature card or use F1, F2, or F3.
6. Observe the LED state and correction counter.
7. Press F12 to disable all features immediately.
8. Exit the trainer before leaving the offline session.

### Hotkeys

```text
F1   Toggle Invincible
F2   Toggle Magazine Auto-Fill
F3   Toggle Inventory Auto-Max
F12  Emergency Disable All Features
```

---

## Runtime Architecture

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#ECFEFF','primaryTextColor':'#164E63','primaryBorderColor':'#67E8F9','lineColor':'#818CF8','secondaryColor':'#EEF2FF','tertiaryColor':'#ECFDF5'}}}%%
graph TB
    subgraph Presentation[GPU Presentation Layer]
        Window[Win32 Window]
        Direct2D[Direct2D Renderer]
        DirectWrite[DirectWrite Typography]
        LEDs[LED State Indicators]
        Statistics[Correction Counters]
    end

    subgraph Control[Application Control Layer]
        UIThread[UI Thread]
        WorkerThread[Worker Thread]
        Snapshot[Mutex-Protected UI Snapshot]
        Hotkeys[Global Polling Hotkeys]
    end

    subgraph Memory[Validated Memory Layer]
        Process[Process Discovery and Attachment]
        Module[Module Base Resolution]
        Player[Player Pointer Resolver]
        MagazineResolver[Magazine Pointer Resolver]
        Reader[Typed Memory Reads]
        Writer[Bounded Typed Writes]
    end

    subgraph Policies[Persistent Policy Layer]
        InvinciblePolicy[Invincible Policy]
        MagazinePolicy[Magazine Auto-Fill Policy]
        InventoryPolicy[Inventory Auto-Max Policy]
    end

    Presentation --> Control
    Control --> Memory
    Memory --> Policies
    Policies --> Snapshot
    Snapshot --> Presentation

    style Presentation fill:#EEF2FF,stroke:#A5B4FC,stroke-width:2px,color:#312E81
    style Control fill:#ECFEFF,stroke:#67E8F9,stroke-width:2px,color:#164E63
    style Memory fill:#FEF3C7,stroke:#FCD34D,stroke-width:2px,color:#78350F
    style Policies fill:#ECFDF5,stroke:#6EE7B7,stroke-width:2px,color:#065F46
```

### Architecture Components

- **Win32 application shell:** Creates and manages the native Windows application window
- **Direct2D:** Renders the background, cards, controls, borders, and LED indicators
- **DirectWrite:** Renders clean text and live numeric information
- **UI thread:** Owns all graphical resources and processes application messages
- **Worker thread:** Owns attachment, pointer resolution, validation, and bounded update policies
- **Snapshot bridge:** Shares only the small current UI state under mutex protection
- **Process memory layer:** Provides typed read and write operations through Windows APIs
- **Policy layer:** Applies only explicitly enabled and validated gameplay policies

---

## Health and Damage Model

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#ECFDF5','primaryTextColor':'#065F46','primaryBorderColor':'#6EE7B7','lineColor':'#A5B4FC','secondaryColor':'#EEF2FF','tertiaryColor':'#FEF3C7'}}}%%
flowchart LR
    Player[Resolved Player Object] --> Damage[Read Float32 at +0x254<br/>Accumulated Damage]
    Player --> Capacity[Read Float32 at +0x258<br/>Damage Capacity]
    Damage --> Validate{Finite Values?}
    Capacity --> Validate
    Validate -->|No| Skip[Skip Current Cycle]
    Validate -->|Yes| Positive{Capacity Greater Than Zero?}
    Positive -->|No| Skip
    Positive -->|Yes| Calculate[Remaining = Capacity - Damage]
    Calculate --> Percent[Health Percent = Clamp<br/>Remaining / Capacity x 100]
    Percent --> Display[Render Health Percentage]
    Positive -->|Invincible Enabled| Zero[Write 0.0 to Damage Field]
    Zero --> Counter[Increment Invincible Correction Counter]

    style Player fill:#EEF2FF,stroke:#A5B4FC,stroke-width:2px,color:#312E81
    style Validate fill:#FEF3C7,stroke:#FCD34D,stroke-width:2px,color:#78350F
    style Positive fill:#FEF3C7,stroke:#FCD34D,stroke-width:2px,color:#78350F
    style Display fill:#ECFDF5,stroke:#6EE7B7,stroke-width:2px,color:#065F46
    style Zero fill:#FDF2F8,stroke:#F9A8D4,stroke-width:2px,color:#831843
```

### Confirmed Fields

```text
player + 0x254  Float32  Accumulated damage
player + 0x258  Float32  Damage capacity / lethal threshold
```

### Health Formula

```text
remaining_health = max(0, damage_capacity - accumulated_damage)
health_percent = clamp(
    remaining_health / damage_capacity * 100,
    0,
    100
)
```

Invincible mode does not write an arbitrary health number. It maintains the confirmed accumulated-damage field at `0.0f` after validating both health fields.

---

## Player Pointer Resolution

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#EEF2FF','primaryTextColor':'#312E81','primaryBorderColor':'#A5B4FC','lineColor':'#818CF8','secondaryColor':'#ECFEFF','tertiaryColor':'#FEF3C7'}}}%%
flowchart LR
    Base[igi.exe Module Base] --> Root[+0x16E210]
    Root --> P0[Read Pointer p0]
    P0 --> S1[+0x08]
    S1 --> P1[Read Pointer p1]
    P1 --> S2[+0x7CC]
    S2 --> P2[Read Pointer p2]
    P2 --> S3[+0x14]
    S3 --> Player[Read Final Player Pointer]

    Root -. Validate .-> Guard1{32-bit User Range}
    P1 -. Validate .-> Guard2{32-bit User Range}
    P2 -. Validate .-> Guard3{32-bit User Range}
    Player -. Validate .-> Guard4{32-bit User Range}

    style Base fill:#EEF2FF,stroke:#A5B4FC,stroke-width:2px,color:#312E81
    style Player fill:#ECFDF5,stroke:#6EE7B7,stroke-width:3px,color:#065F46
    style Guard1 fill:#FEF3C7,stroke:#FCD34D,color:#78350F
    style Guard2 fill:#FEF3C7,stroke:#FCD34D,color:#78350F
    style Guard3 fill:#FEF3C7,stroke:#FCD34D,color:#78350F
    style Guard4 fill:#FEF3C7,stroke:#FCD34D,color:#78350F
```

### Resolution Formula

```text
p0     = read_u32(module_base + 0x16E210)
p1     = read_u32(p0 + 0x08)
p2     = read_u32(p1 + 0x7CC)
player = read_u32(p2 + 0x14)
```

The module-relative root is stable for the supported executable layout. The final player object is dynamically allocated and may move after mission transitions, loading screens, or restart events.

---

## Inventory Management

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#FEF3C7','primaryTextColor':'#78350F','primaryBorderColor':'#FCD34D','lineColor':'#A5B4FC','secondaryColor':'#EEF2FF','tertiaryColor':'#ECFDF5'}}}%%
flowchart TB
    Player[Resolved Player] --> Count[Read Count at +0x340]
    Count --> SafeCount{Count Between 0 and 64?}
    SafeCount -->|No| Wait[Amber LED and Skip Cycle]
    SafeCount -->|Yes| Walk[Walk Runtime Records]
    Walk --> Formula[Record = Player + 0x344 + Index x 0x0C]
    Formula --> ReadFields[Read ID, Current and Maximum]
    ReadFields --> CurrentValid{Current Non-Negative?}
    CurrentValid -->|No| Next[Skip Record]
    CurrentValid -->|Yes| Target{Maximum Positive?}
    Target -->|Yes| UseMax[Target = Record Maximum]
    Target -->|No| UseSafe[Target = 100]
    UseMax --> Bounds{Target Between 0 and 100000?}
    UseSafe --> Bounds
    Bounds -->|No| Next
    Bounds -->|Yes| Changed{Current Differs?}
    Changed -->|No| Next
    Changed -->|Yes| Write[Write Current Field Only]
    Write --> CountWrite[Increment Inventory Counter]
    CountWrite --> Next
    Next --> Walk

    style Player fill:#EEF2FF,stroke:#A5B4FC,stroke-width:2px,color:#312E81
    style SafeCount fill:#FEF3C7,stroke:#FCD34D,stroke-width:2px,color:#78350F
    style Write fill:#ECFDF5,stroke:#6EE7B7,stroke-width:2px,color:#065F46
    style Wait fill:#FDF2F8,stroke:#F9A8D4,stroke-width:2px,color:#831843
```

### Runtime Record Structure

```cpp
struct InventoryRecord {
    std::int32_t id;       // +0x00, read-only
    std::int32_t current;  // +0x04, bounded mutable field
    std::int32_t maximum;  // +0x08, read-only
};
```

### Inventory Formulas

```text
count_address   = player + 0x340
record_address  = player + 0x344 + index * 0x0C
id_address      = record_address + 0x00
current_address = record_address + 0x04
maximum_address = record_address + 0x08
```

### Inventory Safety Rules

- Reject record counts below `0` or above `64`
- Never write the record ID
- Never write the record maximum
- Skip negative current values
- Respect a positive maximum
- Use `100` only for an unclamped or non-positive maximum
- Reject targets above `100000`
- Increment the counter only after a successful write

---

## Magazine Management

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#FDF2F8','primaryTextColor':'#831843','primaryBorderColor':'#F9A8D4','lineColor':'#818CF8','secondaryColor':'#EEF2FF','tertiaryColor':'#ECFDF5'}}}%%
flowchart LR
    Base[igi.exe Module Base] --> Root[+0x671890]
    Root --> M0[Read m0]
    M0 --> Step1[+0x00]
    Step1 --> M1[Read m1]
    M1 --> Step2[+0x4C4]
    Step2 --> M2[Read m2]
    M2 --> Final[+0x144]
    Final --> Current[Read Current Magazine]
    Current --> Plausible{Value Between 0 and 500?}
    Plausible -->|No| Transition[Amber LED: Skip Transition]
    Plausible -->|Yes| Target{Already 99?}
    Target -->|Yes| Active[Green LED: No Write Needed]
    Target -->|No| Write[Write Magazine = 99]
    Write --> Counter[Increment Refill Counter]

    style Base fill:#EEF2FF,stroke:#A5B4FC,stroke-width:2px,color:#312E81
    style Plausible fill:#FEF3C7,stroke:#FCD34D,stroke-width:2px,color:#78350F
    style Transition fill:#FDF2F8,stroke:#F9A8D4,stroke-width:2px,color:#831843
    style Active fill:#ECFDF5,stroke:#6EE7B7,stroke-width:2px,color:#065F46
    style Write fill:#ECFEFF,stroke:#67E8F9,stroke-width:2px,color:#164E63
```

### Magazine Chain

```text
m0 = read_u32(module_base + 0x671890)
m1 = read_u32(m0 + 0x00)
m2 = read_u32(m1 + 0x4C4)
magazine_address = m2 + 0x144
```

Magazine state is resolved independently from inventory reserve quantities. Transitional values, including weapon-switch states, are skipped without disabling the feature.

---

## Repository Structure

```text
IGI-Offline-Trainer/
├── .github/
│   ├── ISSUE_TEMPLATE/
│   │   ├── bug_report.yml
│   │   └── feature_request.yml
│   ├── workflows/
│   │   ├── build.yml
│   │   └── release.yml
│   └── pull_request_template.md
├── assets/
│   ├── gifs/
│   │   ├── README.md
│   │   ├── border-crossing-demo.gif
│   │   ├── trainer-overview.gif
│   │   ├── invincible-demo.gif
│   │   ├── magazine-demo.gif
│   │   └── inventory-demo.gif
│   └── images/
│       ├── README.md
│       ├── border-crossing-01.JPG
│       ├── border-crossing-02.JPG
│       ├── border-crossing-03.JPG
│       ├── border-crossing-hero.JPG
│       ├── hero-1600x900.png
│       ├── ui-preview-1200x800.png
│       └── social-preview-1280x640.png
├── docs/
│   ├── ARCHITECTURE.md
│   └── RELEASE.md
├── include/
│   ├── App.hpp
│   ├── GameTrainer.hpp
│   ├── Offsets.hpp
│   └── ProcessMemory.hpp
├── resources/
│   └── app.rc
├── src/
│   ├── App.cpp
│   ├── GameTrainer.cpp
│   ├── ProcessMemory.cpp
│   └── main.cpp
├── .gitattributes
├── .gitignore
├── CHANGELOG.md
├── CMakeLists.txt
├── CMakePresets.json
├── CONTRIBUTING.md
├── LEGAL.md
├── LICENSE
├── README.md
├── SECURITY.md
└── THIRD_PARTY_NOTICES.md
```

---

## Media and Visual Assets

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#EEF2FF','primaryTextColor':'#312E81','primaryBorderColor':'#A5B4FC','lineColor':'#A5B4FC','secondaryColor':'#ECFEFF','tertiaryColor':'#FDF2F8'}}}%%
graph TD
    Source[Original Source Artwork] --> Hero[Hero Image<br/>1600 x 900]
    Source --> Preview[UI Preview<br/>1200 x 800]
    Source --> Social[GitHub Social Preview<br/>1280 x 640]
    Source --> Icon[Icon Source<br/>1024 x 1024]
    Recording[Recorded Offline Demonstration] --> Frames[PNG Frame Sequence]
    Frames --> GIF[Optimized Demo GIF<br/>960 x 540]

    style Source fill:#EEF2FF,stroke:#A5B4FC,stroke-width:2px,color:#312E81
    style Recording fill:#FDF2F8,stroke:#F9A8D4,stroke-width:2px,color:#831843
    style GIF fill:#ECFDF5,stroke:#6EE7B7,stroke-width:2px,color:#065F46
```

### Recommended File Sizes

- Hero image: `1600 x 900`
- UI screenshot: `1200 x 800`
- Social preview: `1280 x 640`
- Application icon source: `1024 x 1024`
- Border Crossing screenshots: `1600 x 900` (or your native capture resolution)
- Demo GIF: `960 x 540`, 15 to 20 FPS

### ImageMagick Commands

Hero image:

```powershell
magick input.png -resize 1600x900^ -gravity center -extent 1600x900 assets/images/hero-1600x900.png
```

GitHub social preview:

```powershell
magick input.png -resize 1280x640^ -gravity center -extent 1280x640 assets/images/social-preview-1280x640.png
```

Windows icon:

```powershell
magick input.png -define icon:auto-resize=256,128,64,48,32,16 assets/app.ico
```

GIF from PNG frames:

```powershell
magick -delay 5 -loop 0 frames/frame-*.png -resize 960x540 -layers Optimize assets/gifs/trainer-demo.gif
```

Use only original artwork, original screenshots, or media that you have permission to publish. Do not include extracted game textures, logos, models, audio, maps, or other proprietary assets.

---

## GitHub Actions and Release Process

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#ECFEFF','primaryTextColor':'#164E63','primaryBorderColor':'#67E8F9','lineColor':'#818CF8','secondaryColor':'#EEF2FF','tertiaryColor':'#ECFDF5'}}}%%
sequenceDiagram
    autonumber
    participant Developer
    participant Git as Git Repository
    participant Actions as GitHub Actions
    participant Runner as Windows Runner
    participant Release as GitHub Release

    Developer->>Git: Commit release changes
    Developer->>Git: Push version tag v1.0.0
    Git->>Actions: Trigger release workflow
    Actions->>Runner: Start Windows build environment
    Runner->>Runner: Configure Win32 CMake project
    Runner->>Runner: Build Release executable
    Runner->>Runner: Package executable and documentation
    Runner->>Runner: Generate SHA-256 checksum
    Runner-->>Actions: Upload release artifacts
    Actions->>Release: Publish generated release notes
    Actions->>Release: Attach ZIP and checksum
    Release-->>Developer: Latest release available
```

### Create the First Release

```powershell
git add .
git commit -m "Release IGI Offline Trainer v1.0.0"
git push origin main

git tag v1.0.0
git push origin v1.0.0
```

The release workflow creates:

```text
IGI-Offline-Trainer-Windows-x86-v1.0.0.zip
checksums-sha256.txt
```

### Direct Release Link

[Download the Latest Compiled Release](../../releases/latest)

The direct link begins working after the repository has at least one published GitHub release.

---

## Compatibility and Validation

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#FEF3C7','primaryTextColor':'#78350F','primaryBorderColor':'#FCD34D','lineColor':'#A5B4FC','secondaryColor':'#EEF2FF','tertiaryColor':'#ECFDF5'}}}%%
flowchart TB
    Launch[Trainer Launch] --> Process{igi.exe Found?}
    Process -->|No| Wait[Wait Without Writing]
    Process -->|Yes| Module{Module Base Found?}
    Module -->|No| Wait
    Module -->|Yes| Player{Player Chain Valid?}
    Player -->|No| WaitMission[Wait for Active Mission]
    Player -->|Yes| Health{Health Fields Valid?}
    Health -->|No| SkipHealth[Skip Health Cycle]
    Health -->|Yes| Inventory{Inventory Count Safe?}
    Inventory -->|No| SkipInventory[Skip Inventory Cycle]
    Inventory -->|Yes| Magazine{Magazine Value Plausible?}
    Magazine -->|No| SkipMagazine[Skip Magazine Cycle]
    Magazine -->|Yes| Ready[Validated Runtime State]
    Ready --> Write[Apply Enabled Policies Only]

    style Process fill:#FEF3C7,stroke:#FCD34D,stroke-width:2px,color:#78350F
    style Module fill:#FEF3C7,stroke:#FCD34D,stroke-width:2px,color:#78350F
    style Player fill:#FEF3C7,stroke:#FCD34D,stroke-width:2px,color:#78350F
    style Ready fill:#ECFDF5,stroke:#6EE7B7,stroke-width:3px,color:#065F46
    style Write fill:#EEF2FF,stroke:#A5B4FC,stroke-width:2px,color:#312E81
```

### Supported Build Statement

This repository is configured for the specific 32-bit Project I.G.I. executable layout used by the author during development and Python v2 testing. The C++ implementation preserves the same offsets, field meanings, policy behavior, validation limits, and transition handling.

The C++ version was statically reviewed in the repository-generation environment. The game itself was not available in that environment, so runtime validation must be completed by the author against the already tested local game build before publishing the first compiled release.

Other releases, patches, storefront editions, or modified executables may use different memory layouts and are not guaranteed to work.

---

## Troubleshooting

### Game Status Remains Amber

Possible causes:

- `igi.exe` is not running
- The game is at the menu instead of inside a mission
- The player object has not been initialized
- The executable layout differs from the supported build
- Windows denied access to the target process

Recommended response:

1. Start a lawful offline mission.
2. Launch the trainer after the mission loads.
3. Confirm both applications run under the same Windows user context.
4. Verify that the game executable is named `igi.exe`.
5. Confirm that the local executable is the same build used during offset validation.

### LED Changes to Amber During Weapon Switching

This is expected behavior. The magazine chain may temporarily resolve to an invalid or transitional value. The trainer skips that polling cycle and keeps the policy enabled.

### Inventory Counter Does Not Increase

The counter increases only when a valid record actually requires correction. No increment occurs when every current value already equals its target.

### Health Shows Zero or Does Not Update

The application rejects non-finite damage values, non-finite capacity values, and capacity less than or equal to zero. Start or resume a mission and allow the player object to initialize.

### Resource Compiler Not Found

Open **Developer PowerShell for Visual Studio 2026** rather than ordinary PowerShell. Verify:

```powershell
where.exe rc
```

If required for the current session, add the installed x86 Windows SDK tools:

```powershell
$env:Path = "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x86;$env:Path"
```

### CMake Generator Not Found

List installed generators:

```powershell
cmake --help | Select-String "Visual Studio"
```

For Visual Studio Community 2026, use the exact generator name printed by CMake, expected as:

```text
Visual Studio 18 2026
```

---

## Security and Privacy Architecture

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#ECFDF5','primaryTextColor':'#065F46','primaryBorderColor':'#6EE7B7','lineColor':'#A5B4FC','secondaryColor':'#EEF2FF','tertiaryColor':'#FEF3C7'}}}%%
graph LR
    subgraph Boundaries[Project Boundaries]
        Offline[Offline Single-Player Only]
        Local[Local Process Only]
        NoInjection[No DLL Injection]
        NoHooks[No Runtime Hooks]
        NoNetwork[No Network Features]
    end

    subgraph Validation[Validation Controls]
        Pointer[Pointer Range Validation]
        Float[Finite Float Validation]
        Count[Inventory Count Limit]
        Bounds[Write Target Bounds]
        Fields[Confirmed Fields Only]
    end

    subgraph Privacy[Privacy Controls]
        NoTelemetry[No Telemetry]
        NoAccount[No Account]
        NoRemoteLog[No Remote Logging]
        SessionOnly[Session-Only Counters]
        NoSecrets[No Credentials Required]
    end

    Boundaries --> Validation
    Validation --> Privacy

    style Boundaries fill:#EEF2FF,stroke:#A5B4FC,stroke-width:2px,color:#312E81
    style Validation fill:#FEF3C7,stroke:#FCD34D,stroke-width:2px,color:#78350F
    style Privacy fill:#ECFDF5,stroke:#6EE7B7,stroke-width:2px,color:#065F46
```

### Security Rules

- No support for online games or competitive play
- No anti-cheat bypass
- No stealth or detection-evasion functionality
- No authentication or entitlement bypass
- No DRM circumvention
- No credential, cookie, token, or session access
- No kernel drivers
- No process injection
- No modification of unrelated processes
- No proprietary game content in source or releases

### Repository Hygiene

Never commit:

```text
*.pfx
*.p12
*.key
*.pem
.env
igi.exe
Game DLL files
Private logs
Personal absolute paths
Authentication data
Signing passwords
Crash dumps containing identifiers
```

---

## Support and Maintenance

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#FEF3C7','primaryTextColor':'#78350F','primaryBorderColor':'#FCD34D','lineColor':'#FCD34D','secondaryColor':'#ECFEFF','tertiaryColor':'#FDF2F8'}}}%%
mindmap
  root((IGI Trainer Support))
    Build Support
      Visual Studio 2026
      CMake Configuration
      Win32 Target
      Windows SDK
    Runtime Support
      Process Detection
      Mission State
      Pointer Validation
      Transition Handling
    Release Support
      GitHub Actions
      Portable ZIP
      SHA-256 Checksum
      Release Notes
    Documentation
      Architecture
      Legal Notice
      Security Policy
      Contribution Guide
    Media
      Original Screenshots
      Demonstration GIFs
      Social Preview
      Application Icon
```

### Bug Reports

A useful public bug report should include:

- Trainer release version
- Windows version
- Build type, such as Win32 Release
- Visual Studio and CMake version if reporting a build issue
- Whether `igi.exe` was detected
- Whether an offline mission was active
- LED color and affected feature
- Reproduction steps
- Sanitized error information

Do not upload game executables, proprietary assets, private paths, tokens, certificates, or personal information.

---

## Professional Standards

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#EEF2FF','primaryTextColor':'#312E81','primaryBorderColor':'#A5B4FC','lineColor':'#818CF8','secondaryColor':'#ECFDF5','tertiaryColor':'#FDF2F8'}}}%%
graph LR
    subgraph Standards[Engineering Standards]
        Minimal[Minimal Interface]
        Native[Native Windows Rendering]
        Separation[UI and Worker Separation]
        Validation[Fail-Closed Validation]
        Documentation[Transparent Documentation]
    end

    subgraph Outcomes[Project Outcomes]
        Responsive[Responsive Interface]
        Focused[Focused User Experience]
        Safe[Bounded Runtime Behavior]
        Maintainable[Centralized Offsets]
        Auditable[Readable Open Source]
    end

    Minimal --> Focused
    Native --> Responsive
    Separation --> Responsive
    Validation --> Safe
    Documentation --> Auditable
    Documentation --> Maintainable

    style Standards fill:#EEF2FF,stroke:#A5B4FC,stroke-width:2px,color:#312E81
    style Outcomes fill:#ECFDF5,stroke:#6EE7B7,stroke-width:2px,color:#065F46
```

### Engineering Principles

- Minimal UI instead of decorative complexity
- Centralized offsets instead of scattered constants
- Typed memory access instead of raw unstructured buffers
- Persistent policies instead of one-shot toggles
- Transition awareness instead of permanent feature shutdown
- Session counters instead of noisy console logging
- Explicit offline scope instead of ambiguous positioning
- Honest compatibility statements instead of universal support claims

---

## Notice of Cooperation & Take-Down Policy

I deeply respect the intellectual property rights of game developers and publishers. Please note that this is my very first reverse engineering project made public, and it was created entirely as a personal learning experiment.

If you are an **official copyright holder, authorized legal representative, or publisher** associated with *Project I.G.I.* or related intellectual property and believe this repository conflicts with your intellectual property guidelines:

- **Preferred Contact Method:** Please open an **Issue** or a **Pull Request** directly here on GitHub. This repository is actively monitored.
- **Verification Requirement:** To prevent fraudulent requests or trolling, a request for modification or removal should include reasonable official verification showing authority to act for the relevant rights holder. Sensitive evidence should not be posted publicly. Request a suitable private contact channel where necessary.
- **Commitment to Compliance:** A verified request will be reviewed promptly and in good faith. Appropriate modification, content removal, release removal, or repository take-down action will be taken when required.

This cooperation policy does not limit any legal rights or formal notice mechanisms available to a rights holder, publisher, platform, or authorized representative.

---

## Legal and Intellectual Property

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#FDF2F8','primaryTextColor':'#831843','primaryBorderColor':'#F9A8D4','lineColor':'#A5B4FC','secondaryColor':'#EEF2FF','tertiaryColor':'#ECFDF5'}}}%%
flowchart LR
    Project[IGI Offline Trainer Repository] --> Includes[Includes]
    Project --> Excludes[Does Not Include]

    Includes --> Source[Independent C++ Source]
    Includes --> Build[Build and CI Files]
    Includes --> Docs[Original Documentation]
    Includes --> Assets[Original Repository Assets]

    Excludes --> Executable[Game Executable]
    Excludes --> DLLs[Game DLLs]
    Excludes --> Media[Textures, Models and Audio]
    Excludes --> Levels[Maps and Level Data]
    Excludes --> Cracks[Cracks and DRM Bypass]
    Excludes --> Proprietary[Proprietary Source or SDK Files]

    style Project fill:#EEF2FF,stroke:#A5B4FC,stroke-width:3px,color:#312E81
    style Includes fill:#ECFDF5,stroke:#6EE7B7,stroke-width:2px,color:#065F46
    style Excludes fill:#FDF2F8,stroke:#F9A8D4,stroke-width:2px,color:#831843
```

This repository contains independently written source code, project configuration, documentation, and original repository assets. It does not contain the original game executable, DLLs, levels, textures, models, audio, fonts, manuals, cracks, DRM circumvention material, or other proprietary game content.

Project I.G.I. and related names, trademarks, and intellectual property belong to their respective rights holders. This project is unofficial and is not affiliated with, sponsored by, authorized by, or endorsed by those rights holders.

Users must own a lawfully obtained copy of the game and are responsible for complying with applicable law, license terms, and platform rules.

See:

- [Legal Notice](LEGAL.md)
- [MIT License](LICENSE)
- [Security Policy](SECURITY.md)
- [Third-Party Notices](THIRD_PARTY_NOTICES.md)

---

## License

The independently written source code in this repository is provided under the MIT License. The MIT License applies only to the original project code and documentation covered by the repository copyright notice.

The project license does not grant rights to Project I.G.I., its executable, assets, trademarks, or any other third-party intellectual property.

---

## Important Notes

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#FEF3C7','primaryTextColor':'#78350F','primaryBorderColor':'#FCD34D','lineColor':'#A5B4FC','secondaryColor':'#EEF2FF','tertiaryColor':'#FDF2F8'}}}%%
flowchart LR
    Offline[Offline Use Only] --> Lawful[Lawfully Obtained Copy]
    Lawful --> Version[Version-Specific Compatibility]
    Version --> Validate[Runtime Validation Required]
    Validate --> NoAssets[No Proprietary Assets]
    NoAssets --> Responsibility[User Responsibility]

    style Offline fill:#ECFDF5,stroke:#6EE7B7,stroke-width:2px,color:#065F46
    style Version fill:#FEF3C7,stroke:#FCD34D,stroke-width:2px,color:#78350F
    style Responsibility fill:#FDF2F8,stroke:#F9A8D4,stroke-width:2px,color:#831843
```

- This project is restricted to authorized offline single-player experimentation.
- The configured offsets are version-specific.
- Runtime compatibility must be verified before publishing a compiled release.
- Antivirus products may inspect unsigned memory-access utilities more aggressively.
- A compiled release should be signed when a suitable code-signing process is available.
- Publish SHA-256 checksums with every release.
- Never bundle the original game or proprietary game content.
- Keep all public screenshots and GIFs free of personal paths and private information.

---

<div align="center">

## IGI Offline Trainer

### Professional Offline Gameplay Experimentation Utility

*Minimal • Native • Validated • Private • Offline-Only*

*Built from one childhood mission that never stopped nagging at me.*

<br>

**Designed and Developed by TJM**

*First Public Reverse-Engineering Learning Project*

<br>

[Download the Latest Compiled Release](../../releases/latest)

</div>
