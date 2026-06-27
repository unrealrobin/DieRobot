# Build System

## Goals
1. Snap Walls and Floors Together.
2. Snap Components to Walls and Floors.
3. Snap Traps to Walls and Floors.
4. Save and Load Buildables.
5. Support payment, refunding, and deletion of buildables.
6. Provide clear visual feedback for invalid placement.

## Why this works
1. There is substantial logic required to make different buildable types snap together. Consolidating this in a single Actor Component improves separation of concerns.
2. The controller only dispatches input events, while the Build System Manager performs the build logic.
3. The HUD, Build Menu Panel, and build icons provide a clear, consistent way for the player to interact with the system.


## In Game Screenshots

### Build Menu
![Build Menu Screenshot](./Images/BuildMenuUI.png)

### Build Placement
![Building Screenshot](./Images/BuildingScreenshot.png)

### Invalid Build Placement
![Incorrect Placement Screenshot](./Images/InvalidPlacementScreenshot.png)

---

##  Code Map

<!-- blank line above! -->

| Class                                            | Role                                                                                          | Key Methods                                                                                                   | GitHub                                                                                                                                                                                                                                                                         |
|--------------------------------------------------|-----------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `UBuildSystemManagerComponent` (Actor Component) | Handles Spawning, Placement and Snapping of all Buildables. (Structures, Components, Traps)   | `SpawnFinalBuildable()`, `SpawnProxy()`, `FirstHitBuildingComponentHitResult()`,`HandleProxyPlacement()`      | [H](https://github.com/unrealrobin/DieRobot/blob/main/Source/DieRobot/Public/Components/BuildSystem/BuildSystemManagerComponent.h) - [CPP](https://github.com/unrealrobin/DieRobot/blob/main/Source/DieRobot/Private/Components/BuildSystem/BuildSystemManagerComponent.cpp) |
| `ABuildableBase` (Actor)                         | Base class for any buildable class.                                                           |                                                                                                               | [H](https://github.com/unrealrobin/DieRobot/blob/main/Source/DieRobot/Public/BuildSystem/BuildableBase.h) - [CPP](https://github.com/unrealrobin/DieRobot/blob/main/Source/DieRobot/Private/BuildSystem/BuildableBase.cpp)                                                 |
| `UBuildingComponentPanel` (UI)                   | UI for selecting a buildable and viewing synergies, costs and details.                        | `LoadAllDataAssetsForMenu()`, `CreateBuildableIconsInMenu()`, `PrepareTrapEffectList()`                       | [H](https://github.com/unrealrobin/DieRobot/blob/main/Source/DieRobot/Public/UI/BuildingComponentPanel.h) - [CPP](https://github.com/unrealrobin/DieRobot/blob/main/Source/DieRobot/Private/UI/BuildingComponentPanel.cpp)                                                 |
| `USaveLoadSubsystem`  (Game Instance Subsystem)  | Saves and loads build data.                                                                   | `SaveBuildableData(USaveLoadStruct* SaveGameInstance)`,`LoadBuildableData(USaveLoadStruct* LoadGameInstance)` | [H](https://github.com/unrealrobin/DieRobot/blob/main/Source/DieRobot/Public/Subsystems/SaveLoad/SaveLoadSubsystem.h) - [CPP](https://github.com/unrealrobin/DieRobot/blob/main/Source/DieRobot/Private/Subsystems/SaveLoad/SaveLoadSubsystem.cpp)                         |
| `UBuildingComponent`  (UI)                       | UI widget for selecting a specific buildable and viewing synergies and costs.                 | **Functions In Blueprint**                                                                                    | [H](https://github.com/unrealrobin/DieRobot/blob/main/Source/DieRobot/Public/UI/BuildingComponent.h)                                                          |
| `UBuildComponentDataAsset`  (DA)                 | Data Asset used for Loading specific buildable information into the Building Component Panel. |                                                                                                               | [H](https://github.com/unrealrobin/DieRobot/blob/main/Source/DieRobot/Public/Data/DataAssets/BuildComponentDataAsset.h)                                                                                                                                                                                                           |

---

##  Key Features
- Walls and floors snap together via a quadrant and snap-point system.
- Traps and constructs attach to designated slots on structures.
- Save/Load of placed buildables using GUIDs and durable parent–child relationships.
- Grid, collision, and NavMesh-aware placement validation with clear visual feedback.

---


# Architecture

## Sequence Diagram
![Build System Sequence Diagram](./Images/BuildSystemSequence.png)

The build system includes multiple components needed to generate a build in game. The player enters build mode, selects a buildable from the build menu, spawns the proxy,
which is simply a representation of the buildable, and then if possible, spawns the final buildable. The build menu is populated
at the games initialization using Data Assets for each buildable type. The Data Assets comprise all the information about buildable cost and name
and are stored in their respective icons in the build menu.

## 🔗 Links
- [View Full Repo on GitHub](https://github.com/unrealrobin/DieRobot)