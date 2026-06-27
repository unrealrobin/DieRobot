// Property of Paracosm Industries. 


#include "Subsystems/SaveLoad/SaveLoadSubsystem.h"

#include "BuildSystem/Constructs/ConstructBase.h"
#include "BuildSystem/Constructs/TeleportConstruct.h"
#include "Character/TimberPlayableCharacter.h"
#include "Character/TimberSeeda.h"
#include "Components/Combat/CombatComponent.h"
#include "Components/Inventory/InventoryManagerComponent.h"
#include "Components/MissionDelivery/MissionDeliveryComponent.h"
#include "Components/Vignette/PlayerVignetteComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameModes/TimberGameModeBase.h"
#include "Subsystems/Wave/WaveGameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "States/PlayerStateBase.h"
#include "Subsystems/SaveLoad/Types/DieRobotGlobalSaveDataStruct.h"
#include "Weapons/TimberWeaponMeleeBase.h"
#include "Weapons/TimberWeaponRangedBase.h"

void USaveLoadSubsystem::SetNewGameSaveSlot()
{
	FGuid SaveSlotID = FGuid::NewGuid();
	/*UE_LOG(LogTemp, Warning, TEXT("SetNewGameSaveSlot() - SaveLoadSubsystem - GUID: %s"), *SaveSlotID.ToString(EGuidFormats::Digits));*/
	//Converts to string and removes dashes.
	FString IDasString = SaveSlotID.ToString(EGuidFormats::Digits);

	//Adding the new ID for this save slot to the list of saved games.
	//Saved Locally on the Save Load Subsystem. Potentially can be removed.
	SaveSlots.Add(IDasString);

	//Setting the sessions Save Slot to use.
	SetCurrentSessionSaveSlot(IDasString);
	AddNewSaveSlotToGlobalSaveSlotList();
}

void USaveLoadSubsystem::SetLoadGameSaveSlot(FString SlotName)
{
	//Load an Existing Game from the List of Saved Games
	CurrentSessionSaveSlot = SlotName;
}

/*Save System*/
void USaveLoadSubsystem::SaveCurrentGame()
{
	if (CurrentSessionSaveSlot.IsEmpty()) return;
	/*UE_LOG(LogTemp, Warning, TEXT("Saving Game to Slot: %s"), *CurrentSessionSaveSlot);*/
	
	FString SaveSlot = CurrentSessionSaveSlot;
	
	//Checking for existing Save Slot and Loading, Otherwise, creating a new save file for the slot.

	bool bDoesSaveExist = UGameplayStatics::DoesSaveGameExist(CurrentSessionSaveSlot, 0);
	USaveLoadStruct* SaveGameInstance;
	if (!bDoesSaveExist)
	{
		SaveGameInstance = Cast<USaveLoadStruct>(
		UGameplayStatics::CreateSaveGameObject
		(USaveLoadStruct::StaticClass()));
	}
	else
	{
		SaveGameInstance = Cast<USaveLoadStruct>(UGameplayStatics::LoadGameFromSlot(CurrentSessionSaveSlot, 0));
	}

	SaveBuildableData(SaveGameInstance);
	SaveWaveData(SaveGameInstance);
	SavePlayerData(SaveGameInstance);
	SaveSeedaData(SaveGameInstance);

	//Saves this sessions Data to the Current SaveSlot and SaveGameInstance
	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlot, 0);

	/*Update Save Slot Info for Global List used for Load Menu*/
	SaveSessionDataToGlobalSaveSlotList();
}

void USaveLoadSubsystem::SaveBuildableData(USaveLoadStruct* SaveGameInstance)
{
	if (SaveGameInstance)
	{
		TArray<AActor*> CurrentBuildingComponents;
		UGameplayStatics::GetAllActorsOfClass(
			GetWorld(), ABuildableBase::StaticClass(), CurrentBuildingComponents);

		/*
		 * 1. Loop Through All Buildables.
		 */
		for (AActor* BuildableActor : CurrentBuildingComponents)
		{
			ABuildableBase* Buildable = Cast<ABuildableBase>(BuildableActor);

			//Skip Buildable if it's a proxy. It will be saved at the next save.
			if (Buildable->bIsProxy) continue;
			
			if (Buildable && Buildable->BuildableType != EBuildableType::Environment && !Buildable->IsPendingKillPending())
			{
				//Creating the Building Component Struct to pass to the Save System's Building Component Array
				FBuildableData BuildableData;
				BuildableData.BuildingComponentClass = BuildableActor->GetClass();
				BuildableData.BuildingComponentTransform = BuildableActor->GetActorTransform();

				//Will Set whether or not this buildable can be walked on.
				// Ex. Tops of Walls / Traps on Vertical Surfaces etc.
				BuildableData.bSetIsWalkable = Buildable->bIsWalkable;
				
				/*if (BuildableData.bSetIsWalkable == false)
				{
					UE_LOG(LogTemp, Warning, TEXT("Buildable is Not Walkable. %s"), *BuildableActor->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Buildable is Walkable. %s"), *BuildableActor->GetName());
				}*/

				//Saving the GUID of this Buildable
				//UE_LOG(LogTemp, Warning, TEXT("--------------------------------------"));
				//UE_LOG(LogTemp, Warning, TEXT("Saving Buildable: %s. GUID: %s"), *BuildableActor->GetName(), *Buildable->GetGUID().ToString());
				BuildableData.GUID = Buildable->GetGUID();
				/*if (!BuildableData.GUID.IsValid())
				{
					UE_LOG(LogTemp, Warning, TEXT(" GUI Invalid"));
					continue;
					
				}*/

				/*
				 * WALLS & FLOORS SPECIFIC
				 * Checking if the Saved Buildable is Building Component (Walls/Floors)
				 * Walls and floors have Arrays of attachments that we need to save as an array of GUID's.
				 * They also have Snap Attachments that need to be saved.
				 */
				
				//Looping through the array of attached Buildable Instances and saving their GUID's to our AttachedBuildablesGUID Array.
				if (ATimberBuildingComponentBase* BuildingComponent = Cast<ATimberBuildingComponentBase>(Buildable))
				{
					if (BuildingComponent->AttachedBuildingComponents.Num() > 0)
					{
						for (ABuildableBase* AttachedComponent : BuildingComponent->AttachedBuildingComponents)
						{
							if (AttachedComponent && AttachedComponent->GetGUID().IsValid())
							{
								BuildableData.AttachedBuildablesArray.Add(AttachedComponent->GetGUID());
							}
							//UE_LOG(LogTemp, Warning, TEXT("Attachment: %s. Attachment GUID: %s"), *AttachedComponent->GetName(), *AttachedComponent->GetGUID().ToString());
						}
						//UE_LOG(LogTemp, Warning, TEXT("AttachedBuildablesArray Size: %d"), BuildableData.AttachedBuildablesArray.Num());
					}
					//Checks each potential snap point for the Building Component and saves the GUID of the attached Buildable.
					CheckBuildingComponentForSnapAttachments(BuildableData, BuildingComponent);
				}

				/*Trap Specific*/
				//Linking the Parent Building Component to the Trap
				if (ATrapBase* Trap = Cast<ATrapBase>(Buildable))
				{
					if (IsValid(Trap->ParentBuildable))
					{
						BuildableData.ParentBuildableGUID = Trap->ParentBuildable->GetGUID();
						//Setting the Direction of the Trap on the Building Component
						BuildableData.TrapDirection = Trap->BuildingComponentTrapDirection;
						
						/*UE_LOG(LogTemp, Warning, TEXT("Trap: %s. Trap GUID: %s. Parent: %s. Parent GUID: %s"), 
							*Trap->GetName(),
							*Trap->GetGUID().ToString(),
							*Trap->ParentBuildable->GetName(),
							*Trap->ParentBuildable->GetGUID().ToString());*/
					}
					//Saving the GUID of the Parent Building Component
				}

				/*Construct Specific*/
				//Linking the Parent Building Component to the Construct
				if (AConstructBase* Construct = Cast<AConstructBase>(Buildable))
				{
					if (IsValid(Construct->ParentBuildable))
					{
						BuildableData.ParentBuildableGUID = Construct->ParentBuildable->GetGUID();
					}
					else
					{
						//UE_LOG(LogTemp, Warning, TEXT("Construct %s has no ParentBuildable!"), *GetNameSafe(Construct));
					}
				}

				/*Teleporter Specific*/
				//Linking the Parent Building Component to the Teleport Construct & the Teleport Pair
				if (ATeleportConstruct* TeleportConstruct = Cast<ATeleportConstruct>(Buildable))
				{
					//Saving the GUID of the Parent Building Component
					if (TeleportConstruct->ParentBuildable)
					{
						BuildableData.ParentBuildableGUID = TeleportConstruct->ParentBuildable->GetGUID();
					}
					//Saving Teleport Pair GUID
					check(TeleportConstruct->TeleportPair);
					BuildableData.TeleportPairGUID = TeleportConstruct->TeleportPair->GetGUID();
					//UE_LOG(LogTemp, Warning, TEXT(" Saved for Teleport: %s -> Pair GUID for Teleport: %s"),*TeleportConstruct->GetName(), *TeleportConstruct->TeleportPair->GetName());
				}
				
				SaveGameInstance->BuildingComponentsArray.Add(BuildableData);
				//UE_LOG(LogTemp, Warning, TEXT("--------------------------------------"));
			}
		}
	}
}

void USaveLoadSubsystem::SaveWaveData(USaveLoadStruct* SaveGameInstance)
{
	UWaveGameInstanceSubsystem* WaveSubsystem = GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>();
	
	SaveGameInstance->WaveNumber = WaveSubsystem->CurrentWaveNumber;
	
	/*UE_LOG(LogTemp, Warning, TEXT("Saved Current Wave Number: %d"), SaveGameInstance->WaveNumber);*/
}

void USaveLoadSubsystem::SavePlayerData(USaveLoadStruct* SaveGameInstance)
{
	ATimberGameModeBase* GameMode = Cast<ATimberGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		ATimberPlayableCharacter* Character = GameMode->TimberCharacter;
		if (Character)
		{
			/*Save Player Location*/
			SaveGameInstance->PlayerData.PlayerLocation = Character->GetActorLocation();
			SaveGameInstance->PlayerData.PlayerRotation = Character->GetActorRotation();
			
			/*Save Player Inventory/Currency*/
			APlayerStateBase* PS = Cast<APlayerStateBase>(Character->GetPlayerState());
			if (PS)
			{
				FInventoryData& PlayerInventory = SaveGameInstance->PlayerData.PlayerInventory;
				PlayerInventory.NumberOfParts = PS->MainInventory->NumberOfParts;
				PlayerInventory.NumberOfMechanism = PS->MainInventory->NumberOfMechanism;
				PlayerInventory.NumberOfUniques = PS->MainInventory->NumberOfUniques;
				
				/*UE_LOG(LogTemp, Warning, TEXT("Saved Player Inventory - Parts: %d, Mechanisms: %d, Uniques: %d"),
					PlayerInventory.NumberOfParts,
					PlayerInventory.NumberOfMechanism,
					PlayerInventory.NumberOfUniques);*/
			}
			 /*Save Players Completed Missions*/
			if (Character->MissionDeliveryComponent)
			{
				for (FGuid MissionGuid : Character->MissionDeliveryComponent->CompletedMissionGuids)
				{
					SaveGameInstance->PlayerData.CompletedMissionList.Add(MissionGuid);
					/*UE_LOG(LogTemp, Warning, TEXT(" Saved Mission: %s. GUID: %ls"), *Character->MissionDeliveryComponent->GetMissionTitle(), *LexToString(MissionGuid));*/
				}
				/*UE_LOG(LogTemp, Warning, TEXT(" Saved %d Missions"), SaveGameInstance->PlayerData.CompletedMissionList.Num());*/
			}
		}
	}
}

void USaveLoadSubsystem::SaveSeedaData(USaveLoadStruct* SaveGameInstance)
{
	/*Saving Seeda Location & Health*/
	ATimberSeeda* Seeda = Cast<ATimberSeeda>(UGameplayStatics::GetActorOfClass(GetWorld(), ATimberSeeda::StaticClass()));
	if (Seeda && SaveGameInstance)
	{
		SaveGameInstance->SeedaData.SeedaLocation = Seeda->GetActorLocation();
		//UE_LOG(LogTemp, Warning, TEXT("Saved Seeda Location: %s"), *Seeda->GetActorLocation().ToString());
		SaveGameInstance->SeedaData.SeedaRotation = Seeda->GetActorRotation();

		//When dieing Seeda Health is Regenerated.
		SaveGameInstance->SeedaData.SeedaHealth = Seeda->MaxHealth;
	}
}

void USaveLoadSubsystem::CheckBuildingComponentForSnapAttachments(FBuildableData& BuildableData, ATimberBuildingComponentBase* BuildingComponent)
{
	//Individual Snap Points used for information player if snap is available.
	if (BuildingComponent->FrontBottomAttachment &&
		BuildingComponent->FrontBottomAttachment->GetGUID().IsValid())
	{
		BuildableData.FrontBottomAttachmentGUID = BuildingComponent->FrontBottomAttachment->GetGUID();
		UE_LOG(LogTemp, Warning, TEXT("FrontBottomAttachment Found: %s. Set GUID: %s"), *BuildingComponent->FrontBottomAttachment->GetName(), *BuildingComponent->FrontBottomAttachment->GetGUID().ToString());

	}

	if (BuildingComponent->FrontCenterAttachment &&
		BuildingComponent->FrontCenterAttachment->GetGUID().IsValid())
	{
		BuildableData.FrontCenterAttachmentGUID = BuildingComponent->FrontCenterAttachment->GetGUID();
		UE_LOG(LogTemp, Warning, TEXT("FrontCenterAttachment Found: %s. Set GUID: %s"), 
			*BuildingComponent->FrontCenterAttachment->GetName(),
			*BuildingComponent->FrontCenterAttachment->GetGUID().ToString());

	}

	if (BuildingComponent->FrontTopAttachment &&
		BuildingComponent->FrontTopAttachment->GetGUID().IsValid())
	{
		BuildableData.FrontTopAttachmentGUID = BuildingComponent->FrontTopAttachment->GetGUID();
	}

	if (BuildingComponent->FrontRightAttachment &&
		BuildingComponent->FrontRightAttachment->GetGUID().IsValid())
	{
		BuildableData.FrontRightAttachmentGUID = BuildingComponent->FrontRightAttachment->GetGUID();
	}

	if (BuildingComponent->FrontLeftAttachment &&
		BuildingComponent->FrontLeftAttachment->GetGUID().IsValid())
	{
		BuildableData.FrontLeftAttachmentGUID = BuildingComponent->FrontLeftAttachment->GetGUID();
	}

	if (BuildingComponent->BackBottomAttachment &&
		BuildingComponent->BackBottomAttachment->GetGUID().IsValid())
	{
		BuildableData.BackBottomAttachmentGUID = BuildingComponent->BackBottomAttachment->GetGUID();
	}

	if (BuildingComponent->BackCenterAttachment &&
		BuildingComponent->BackCenterAttachment->GetGUID().IsValid())
	{
		BuildableData.BackCenterAttachmentGUID = BuildingComponent->BackCenterAttachment->GetGUID();
		/*
		UE_LOG(LogTemp, Warning, TEXT("BackCenterAttachment Found: %s. Set GUID: %s,"), *BuildingComponent->BackCenterAttachment->GetName(), *BuildingComponent->BackCenterAttachment->GetGUID().ToString());
		*/

	}

	if (BuildingComponent->BackTopAttachment &&
		BuildingComponent->BackTopAttachment->GetGUID().IsValid())
	{
		BuildableData.BackTopAttachmentGUID = BuildingComponent->BackTopAttachment->GetGUID();
	}

	if (BuildingComponent->BackRightAttachment &&
		BuildingComponent->BackRightAttachment->GetGUID().IsValid())
	{
		BuildableData.BackRightAttachmentGUID = BuildingComponent->BackRightAttachment->GetGUID();
	}

	if (BuildingComponent->BackLeftAttachment &&
		BuildingComponent->BackLeftAttachment->GetGUID().IsValid())
	{
		BuildableData.BackLeftAttachmentGUID = BuildingComponent->BackLeftAttachment->GetGUID();
	}
}

void USaveLoadSubsystem::SetCurrentSessionSaveSlot(FString SlotName)
{
	CurrentSessionSaveSlot = SlotName;
	UDieRobotGlobalSaveData* GlobalSaveDataInstance = GetGlobalSaveDataInstance();
	if (IsValid(GlobalSaveDataInstance))
	{
		GlobalSaveDataInstance->LastSavedSlot.LastSavedGame = CurrentSessionSaveSlot;
		UGameplayStatics::SaveGameToSlot(GlobalSaveDataInstance, GlobalSaveDataSlotName, 0);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Load or Create Global Save Data Instance from the GLOBAL_SAVE_DATA slot."));
	}
}

UDieRobotGlobalSaveData* USaveLoadSubsystem::GetGlobalSaveDataInstance()
{
	 //Attempt to Load the an Existing GLobal Save Data File
	UDieRobotGlobalSaveData* LoadedGlobalSaveData = Cast<UDieRobotGlobalSaveData>(
		UGameplayStatics::LoadGameFromSlot(GlobalSaveDataSlotName, 0));
	
	if (LoadedGlobalSaveData)
	{
		return LoadedGlobalSaveData;
	}

	//Creating a Save Slot for the Global Save Data.
	UDieRobotGlobalSaveData* GlobalSaveDataInstance = Cast<UDieRobotGlobalSaveData>(
		UGameplayStatics::CreateSaveGameObject
		(UDieRobotGlobalSaveData::StaticClass()));
	if (GlobalSaveDataInstance)
	{
		/*UE_LOG(LogTemp, Warning, TEXT("Created and loaded a new Global Save Data File."));*/
		return GlobalSaveDataInstance;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Could not Load or Create Global Save Data Instance."));
	return nullptr;
	
	
	
}

FString USaveLoadSubsystem::GetLastPlayedSaveSlot()
{
	UDieRobotGlobalSaveData* GlobalSaveDataInstance = GetGlobalSaveDataInstance();
	if (GlobalSaveDataInstance)
	{
		return GlobalSaveDataInstance->LastSavedSlot.LastSavedGame;
	}
	UE_LOG(LogTemp, Warning, TEXT("Failed to retrieve Last Saved Game Save Slot FString."));
	return CurrentSessionSaveSlot;
	
	
}

/* Load System */
void USaveLoadSubsystem::LoadGame(FString SlotToLoad)
{
	UWaveGameInstanceSubsystem* WaveSubsystem = GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>();
	ATimberGameModeBase* GameMode = Cast<ATimberGameModeBase>(GetWorld()->GetAuthGameMode());
	
	if (WaveSubsystem && GameMode)
	{
		BindToGameModeBaseDelegate(GameMode);
		
		RemoveAllLabBuildables();
		RemoveAllLootItems();
		
		WaveSubsystem->ResetWaveEnemies();
		
		//Returns the correct Slot to use based on Game Config
		FString LoadSlot = SlotToLoad;
		
		USaveLoadStruct* LoadGameInstance = Cast<USaveLoadStruct>(
			UGameplayStatics::LoadGameFromSlot(LoadSlot, 0));

		if (IsValid(LoadGameInstance))
		{
			LoadBuildableData(LoadGameInstance);
			LoadWaveData(LoadGameInstance);
			LoadPlayerState(LoadGameInstance);
			LoadSeedaData(LoadGameInstance);
			
			GameMode->CloseAllLabDoors();
			WaveSubsystem->CloseBossDoor();
			GameMode->HandleRedrawPathTrace();


			if (GameMode->SwitchToStandardUI.IsBound())
			{
				GameMode->SwitchToStandardUI.Execute();
			}

			//Potential to be called before player controller is bound to this.
			//Needs at least 1 listener to not error.
			if (GameMode->EnableStandardInputMappingContext.IsBound())
			{
				GameMode->EnableStandardInputMappingContext.Execute();
			}
		}
	}
}

void USaveLoadSubsystem::LoadBuildableData(USaveLoadStruct* LoadGameInstance)
{
	if (LoadGameInstance)
	{
		for (FBuildableData BuildingComponentData : LoadGameInstance->BuildingComponentsArray)
		{
			if (BuildingComponentData.BuildingComponentClass)
			{
				/*UE_LOG(LogTemp, Warning, TEXT("--------------------------------------"));
				UE_LOG(LogTemp, Warning, TEXT("Loading Buildable: %s"), *BuildingComponentData.BuildingComponentClass->GetName());*/
				
				AActor* SpawnedActor = GetWorld()->SpawnActor<ABuildableBase>(
					BuildingComponentData.BuildingComponentClass,
					BuildingComponentData.BuildingComponentTransform.GetLocation(),
					BuildingComponentData.BuildingComponentTransform.GetRotation().Rotator());

				if (ABuildableBase* SpawnedBuildable = Cast<ABuildableBase>(SpawnedActor))
				{
					//Set the Saved GUID back on the newly Spawned Buildable
					SpawnedBuildable->SetGUID(BuildingComponentData.GUID);
					//UE_LOG(LogTemp, Warning, TEXT("Buildable: %s , Buildable GUID: %s"), *SpawnedBuildable->GetName(), *SpawnedBuildable->GetGUID().ToString());

					//Add the KV Pair to the TMap
					RegisterBuildable(SpawnedBuildable);
					
					//Setting the IsWalkable Flag && WalkableSlopeOverride (Makes it so you cannot walk on the top of walls or Traps attached to Walls.)
					if (BuildingComponentData.bSetIsWalkable == false)
					{
						SpawnedBuildable->bIsWalkable = false;
						SpawnedBuildable->HandleStaticMeshWalkableSlope(SpawnedBuildable);

						//UE_LOG(LogTemp, Warning, TEXT("Setting %s to not walkable."), *SpawnedBuildable->GetName());
					}
					else
					{
						//UE_LOG(LogTemp, Warning, TEXT("Buildable is Walkable."));
					}
				}

			}
		}

		//Second Pass over the Saved Buildable Data Array. Resolving References (Attachments and Parenting)
		ResolveBuildableReferences(LoadGameInstance->BuildingComponentsArray);
		
	}
}

void USaveLoadSubsystem::LoadWaveData(USaveLoadStruct* LoadGameInstance)
{
	UWaveGameInstanceSubsystem* WaveSubsystem = GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>();
	ATimberGameModeBase* GameMode = Cast<ATimberGameModeBase>(GetWorld()->GetAuthGameMode());
	if (LoadGameInstance && WaveSubsystem && GameMode)
	{
		WaveSubsystem->CurrentWaveNumber = LoadGameInstance->WaveNumber;
		WaveSubsystem->SetCurrentWaveNumber(LoadGameInstance->WaveNumber);
		
		WaveSubsystem->bIsWaveActive = false;
		/*UE_LOG(LogTemp, Warning, TEXT("Loaded Current Wave Number: %d"), WaveSubsystem->CurrentWaveNumber);*/
	}
}

void USaveLoadSubsystem::LoadPlayerState(USaveLoadStruct* LoadGameInstance)
{
	ATimberGameModeBase* GameMode = Cast<ATimberGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		ATimberPlayableCharacter* TimberCharacter = GameMode->TimberCharacter;
		if (IsValid(TimberCharacter))
		{
			if(!LoadGameInstance->PlayerData.PlayerLocation.IsZero())
			{
				TimberCharacter->SetActorLocation(LoadGameInstance->PlayerData.PlayerLocation);
				TimberCharacter->SetActorRotation(LoadGameInstance->PlayerData.PlayerRotation);
			}
			Cast<APlayerController>(TimberCharacter->GetController())->FlushPressedKeys();
			TimberCharacter->GetCharacterMovement()->StopMovementImmediately();
			
			TimberCharacter->bIsPlayerDead = false;

			//Resetting Vignette to Full health
			TimberCharacter->CurrentHealth = TimberCharacter->MaxHealth;
			if (IsValid(TimberCharacter->VignetteComponent))
			{
				TimberCharacter->VignetteComponent->SetVignetteStateToFullHealth();
			}
			
			
			//Reverting player Inventory to last save.
			APlayerStateBase* PS = Cast<APlayerStateBase>(TimberCharacter->GetPlayerState());
			if (IsValid(PS))
			{
				PS->MainInventory->NumberOfParts = LoadGameInstance->PlayerData.PlayerInventory.NumberOfParts;
				PS->MainInventory->NumberOfMechanism = LoadGameInstance->PlayerData.PlayerInventory.NumberOfMechanism;
				PS->MainInventory->NumberOfUniques = LoadGameInstance->PlayerData.PlayerInventory.NumberOfUniques;
				
				//Broadcast update so HUD Reflects Inventory
				TimberCharacter->InventoryManager->UpdateInventoryHandle.Broadcast();

				/*UE_LOG(LogTemp, Warning, TEXT("Loaded Player Inventory - Parts: %d, Mechanisms: %d, Uniques: %d"),
	                PS->MainInventory->NumberOfParts,
	                PS->MainInventory->NumberOfMechanism,
	                PS->MainInventory->NumberOfUniques);*/
			}

			//Ensuring weapons are loaded
			if (ATimberWeaponRangedBase* RangedWeapon = TimberCharacter->CombatComponent->RangedWeaponInstance)
			{
				if (!IsValid(RangedWeapon)) return;
				//Loading the Ammo of the Weapon
				RangedWeapon->CurrentAmmo = RangedWeapon->MaxAmmo;
			}
			//Ensuring Sword energy is at 100%
			if (ATimberWeaponMeleeBase* MeleeWeapon = TimberCharacter->CombatComponent->MeleeWeaponInstance)
			{
				if (!IsValid(MeleeWeapon)) return;
				MeleeWeapon->CurrentWeaponEnergy = MeleeWeapon->MaxWeaponEnergy;
			}

			
			if (IsValid(TimberCharacter->MissionDeliveryComponent))
			{
				TimberCharacter->MissionDeliveryComponent->CompletedMissionGuids = LoadGameInstance->PlayerData.CompletedMissionList;
			}
		}
	}
}

void USaveLoadSubsystem::LoadSeedaData(USaveLoadStruct* LoadGameInstance)
{
	ATimberSeeda* Seeda = Cast<ATimberSeeda>(UGameplayStatics::GetActorOfClass(this, ATimberSeeda::StaticClass()));

	if (!IsValid(Seeda))
	{
		//This is the load situation if Seeda Was destroyed.
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* SeedaActor = GetWorld()->SpawnActor<ATimberSeeda>(SeedaClass,
			LoadGameInstance->SeedaData.SeedaLocation,
			LoadGameInstance->SeedaData.SeedaRotation,
			SpawnParams);
		//UE_LOG(LogTemp, Warning, TEXT("Spawned Seeda Location: %s"), *LoadGameInstance->SeedaData.SeedaLocation.ToString());
		if (SeedaActor)
		{
			//SeedaActor->SetActorLocation(LoadGameInstance->SeedaData.SeedaLocation);
			//UE_LOG(LogTemp, Warning, TEXT("Set Seeda Location: %s"), *LoadGameInstance->SeedaData.SeedaLocation.ToString());
		}
	}
	else
	{
		//Seeda->SetActorLocation(LoadGameInstance->SeedaData.SeedaLocation);
		Seeda->CurrentHealth = Seeda->MaxHealth;
	}

}

void USaveLoadSubsystem::BindToGameModeBaseDelegate(ATimberGameModeBase* GameModeBase)
{
	if (GameModeBase)
	{
		//Removing all function bindings from this Delegate.
		GameModeBase->OnCharacterInitialization.RemoveAll(this);
		//Binding to the Delegate.
		GameModeBase->OnCharacterInitialization.AddDynamic(this, &USaveLoadSubsystem::OnCharacterInitialization);
	}
}

void USaveLoadSubsystem::OnCharacterInitialization()
{
	/*UE_LOG(LogTemp, Warning, TEXT("SaveLoadSubsystem - OnCharacterInitialization"));*/
	
	USaveLoadStruct* LoadGameInstance = Cast<USaveLoadStruct>(UGameplayStatics::LoadGameFromSlot(CurrentSessionSaveSlot, 0));
	if (IsValid(LoadGameInstance))
	{
		LoadPlayerState(LoadGameInstance);
	}
}

/* Utils */

void USaveLoadSubsystem::RemoveAllLabBuildables()
{
	TArray<AActor*> LabBuildables;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildableBase::StaticClass(), LabBuildables);

	for (AActor* LabBuildableActor : LabBuildables)
	{
		ABuildableBase* LabBuildable = Cast<ABuildableBase>(LabBuildableActor);
		if (LabBuildable && LabBuildable->BuildableType != EBuildableType::Environment)
		{
			LabBuildable->Destroy();
		}
	}
}

void USaveLoadSubsystem::RemoveAllLootItems()
{
	TArray<AActor*> LootItems;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyLootDropBase::StaticClass(), LootItems);

	for (AActor* LootItemActor : LootItems)
	{
		AEnemyLootDropBase* LootItem = Cast<AEnemyLootDropBase>(LootItemActor);
		if (LootItem)
		{
			LootItem->Destroy();
		}
	}
}

void USaveLoadSubsystem::RegisterBuildable(ABuildableBase* Buildable)
{
	//Adds Instance to GUID pair to Map
	// Map holds and Ref to the Spawned Buildable and Its GUID.
	FGuid BuildableGUID = Buildable->GetGUID();
	if (Buildable && BuildableGUID.IsValid())
	{
		GuidToBuildableMap.Add(BuildableGUID, Buildable);
		//UE_LOG(LogTemp, Warning, TEXT("Buildable Added to TMap: %s with GUID: %s"), *Buildable->GetName(), *Buildable->GetGUID().ToString());
		//UE_LOG(LogTemp, Warning, TEXT(" TMap has %i Buildables"), GuidToBuildableMap.Num());
	}
}

void USaveLoadSubsystem::DeRegisterBuildable(FGuid BuildableGUID)
{
	//Removes Key Value Pair
	if (GuidToBuildableMap.Contains(BuildableGUID))
	{
		GuidToBuildableMap.Remove(BuildableGUID);
	}
}

bool USaveLoadSubsystem::bIsBuildableRegistered(FGuid BuildableGUID)
{

	if (GuidToBuildableMap.Contains(BuildableGUID))
	{
		return true;
	}
	
	return false;
}

void USaveLoadSubsystem::ResolveBuildableReferences(TArray<FBuildableData> BuildableData)
{
	/*
	 *Env Buildables don't get registered, so when loading anything placed on an env buildable, they will have a null parent.
	 */
	for (FBuildableData& Data : BuildableData)
	{
		//Checking if the Buildable class is registered in the GuidToBuildableMap
		if (bIsBuildableRegistered(Data.GUID))
		{
			
			//UE_LOG(LogTemp, Warning, TEXT("Buildable: %s is Registered"), *GuidToBuildableMap[Data.GUID]->GetName());
			//Handling Attachment Array of Buildables
			//Retrieving Instance of the Buildable from Guid from TMap
			ABuildableBase* Buildable = GuidToBuildableMap[Data.GUID];
			if (Buildable)
			{
				//Checking if the Buildable is a Building Component
				if (ATimberBuildingComponentBase* BuildingComponent = Cast<ATimberBuildingComponentBase>(Buildable))
				{
					/*UE_LOG(LogTemp, Warning, TEXT("-------------------------------"));
					UE_LOG(LogTemp, Warning, TEXT("Handling Building Component Attachments"));
					UE_LOG(LogTemp, Warning, TEXT("-------------------------------"));
					UE_LOG(LogTemp, Warning, TEXT("Handling Building Component: %s"), *BuildingComponent->GetName());*/
					//Checking if the Building Component has any attached Buildables
					if (Data.AttachedBuildablesArray.Num() > 0)
					{
						//UE_LOG(LogTemp, Warning, TEXT("Buildable has Attached Buildables"));
						//Looping through an array of GUID that Represent some Attached Buildables
						for (FGuid AttachedBuildableGUID : Data.AttachedBuildablesArray)
						{
							//Checking if the Attached Buildable is registered in the GuidToBuildableMap
							if (bIsBuildableRegistered(AttachedBuildableGUID))
							{
								ABuildableBase* AttachedBuildable = GuidToBuildableMap[AttachedBuildableGUID];
								if (AttachedBuildable)
								{
									//Adding the Actual Buildable Instance Back to the AttachedBuildingComponents Array
									BuildingComponent->AttachedBuildingComponents.Add(AttachedBuildable);
									//UE_LOG(LogTemp, Warning, TEXT("Adding: %s to AttachedBuildingComponents Array of: %s"), *AttachedBuildable->GetName(), *BuildingComponent->GetName());
								}
							}
						}
					}
					else
					{
						//UE_LOG(LogTemp, Warning, TEXT("Buildable has no Attached Buildable"));
					}
					/* Checking and Setting GUID References for Each Individual Snap Point*/
					if (Data.FrontCenterAttachmentGUID.IsValid())
					{
						//Checking that this GUID is already registered. It should be as all Buildables are spawned and registered beforehand.
						if (bIsBuildableRegistered(Data.FrontCenterAttachmentGUID))
						{
							BuildingComponent->FrontCenterAttachment = GuidToBuildableMap[Data.FrontCenterAttachmentGUID];
						}
					}

					if (Data.BackCenterAttachmentGUID.IsValid())
					{
						if (bIsBuildableRegistered(Data.BackCenterAttachmentGUID))
						{
							BuildingComponent->BackCenterAttachment = GuidToBuildableMap[Data.BackCenterAttachmentGUID];
						}
					}
				}
				//UE_LOG(LogTemp, Warning, TEXT("-------------------------------"));
			}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("Could not get Instance of Buildable from Guid: %s"), *Data.GUID.ToString());
			}

			/* Handling ParentBuildableReference*/
			ATrapBase* Trap = Cast<ATrapBase>(GuidToBuildableMap[Data.GUID]);
			AConstructBase* Construct = Cast<AConstructBase>(GuidToBuildableMap[Data.GUID]);
			ATeleportConstruct* TeleportConstruct = Cast<ATeleportConstruct>(GuidToBuildableMap[Data.GUID]);
			if (Trap || Construct || TeleportConstruct)
			{
				//Checking if the Parent Building Component is registered in the GuidToBuildableMap
				if (bIsBuildableRegistered(Data.ParentBuildableGUID))
				{
					//Casting the Paired Building Component to the correct type and setting the Parent Building Component
					if (ATimberBuildingComponentBase* BuildingComponent = Cast<ATimberBuildingComponentBase>(GuidToBuildableMap[Data.ParentBuildableGUID]))
					{
						/*Traps*/
						if (Trap)
						{
							Trap->ParentBuildable= BuildingComponent;
							Trap->BuildingComponentTrapDirection = Data.TrapDirection;
							//UE_LOG(LogTemp, Warning, TEXT("Setting %s ParentBuildable to: %s"), *Trap->GetName(), *BuildingComponent->GetName());
						}
						/*Constructs*/
						if (Construct)
						{
							Construct->ParentBuildable= BuildingComponent;
							//UE_LOG(LogTemp, Warning, TEXT("Setting %s ParentBuildable to: %s"), *Construct->GetName(), *BuildingComponent->GetName());
						}
						/*Teleporters*/
						if (TeleportConstruct)
						{
							//UE_LOG(LogTemp, Warning, TEXT("--- Handling Teleporters --"));
							// Teleporter have some Construction necessary when respawning themselves. The link Delegates etc.
							TeleportConstruct->ParentBuildable = BuildingComponent;
							
							TeleportConstruct->TeleportPair = Cast<ATeleportConstruct>(GuidToBuildableMap[Data.TeleportPairGUID]);
							if (TeleportConstruct->TeleportPair && TeleportConstruct->TeleportPair->TeleportPair)
							{
								//UE_LOG(LogTemp, Warning, TEXT("Teleport is Linking to Pair."));
								TeleportConstruct->LinkToPair(TeleportConstruct->TeleportPair);
							}
							else
							{
								//UE_LOG(LogTemp, Error, TEXT("TeleportPair is not set in TMap. GUID: %s"), *Data.TeleportPairGUID.ToString());
							}
						}
					}
				}
				else //No Parent Buildable
				{
					if (TeleportConstruct)
					{
						//Teleport Constructs still need to link to their Teleport Pair w/ or w/o a parent.
						//Some traps/Constructs can be placed on ENV Walls/Floors that dont get registered.
						TeleportConstruct->TeleportPair = Cast<ATeleportConstruct>(GuidToBuildableMap[Data.TeleportPairGUID]);
						if (TeleportConstruct->TeleportPair && TeleportConstruct->TeleportPair->TeleportPair)
						{
							//UE_LOG(LogTemp, Warning, TEXT("Teleport is Linking to Pair."));
							TeleportConstruct->LinkToPair(TeleportConstruct->TeleportPair);
						}
						else
						{
							//UE_LOG(LogTemp, Error, TEXT("TeleportPair is not set in TMap. GUID: %s"), *Data.TeleportPairGUID.ToString());
						}
					}
				}
			}
		}
		else
		{
			//UE_LOG(LogTemp, Error, TEXT("Buildable is not set in TMap. GUID: %s"), *Data.GUID.ToString());
		}
	}
}

void USaveLoadSubsystem::AddNewSaveSlotToGlobalSaveSlotList()
{
	UDieRobotGlobalSaveData* GSD = GetGlobalSaveDataInstance();
	if (GSD)
	{
		GSD->ActiveSaveSlots.Add(GenerateSaveSlotDataStruct(CurrentSessionSaveSlot));
		bool bSuccessfulSaveSlot = UGameplayStatics::SaveGameToSlot(GSD, GlobalSaveDataSlotName, 0);
		if (bSuccessfulSaveSlot)
		{
			/*UE_LOG(LogTemp, Warning, TEXT("SaveLoadSubsystem - Added New Save Slot to Global Save Slot List."));*/
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("SaveLoadSubsystem - Failed to Add New Save Slot to Global Save Slot List."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SaveLoadSubsystem - Failed Add New Save Slot to Global Save Instance because couldnt Load the GSD."));
	}
}

FSaveSlotDataStruct USaveLoadSubsystem::GenerateSaveSlotDataStruct(FString SlotName)
{
	FSaveSlotDataStruct Data;
	Data.SlotName = SlotName;
	
	UWaveGameInstanceSubsystem* WaveSubsystem = GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>();
	if (WaveSubsystem)
	{
		Data.SlotCurrentWave = WaveSubsystem->CurrentWaveNumber;
	}
	else
	{
		Data.SlotCurrentWave = 1;
	}

	FGuid OutGuid;
	FGuid::ParseExact(SlotName, EGuidFormats::Digits, OutGuid);
	Data.SlotGuid = OutGuid;

	UDieRobotGlobalSaveData* GSD = GetGlobalSaveDataInstance();
	if (GSD)
	{
		FString TimeStamp = GSD->GetCurrentTimeStamp();
		Data.LastTimeStamp = TimeStamp;
	}

	return Data;
}

void USaveLoadSubsystem::SaveSessionDataToGlobalSaveSlotList()
{
	//When saving, Find the Load Slot for the current session based on ID
	UDieRobotGlobalSaveData* GlobalSaveInstance = GetGlobalSaveDataInstance();
	if (IsValid(GlobalSaveInstance))
	{
		//Generate a new SaveSlotDataStruct
		FSaveSlotDataStruct UpdatedSaveInfo = GenerateSaveSlotDataStruct(CurrentSessionSaveSlot);
		//Update the Data struct on the Global Save Data List.
		for (FSaveSlotDataStruct& Slot: GlobalSaveInstance->ActiveSaveSlots)
		{
			if (Slot.SlotName == UpdatedSaveInfo.SlotName)
			{
				Slot.LastTimeStamp = UpdatedSaveInfo.LastTimeStamp;
				Slot.SlotCurrentWave = UpdatedSaveInfo.SlotCurrentWave;
				bool Saved = UGameplayStatics::SaveGameToSlot(GlobalSaveInstance, GlobalSaveDataSlotName, 0);

				if (!Saved)
				{
					UE_LOG(LogTemp, Error, TEXT("SaveLoadSubsystem - Failed to Save Global Save Data."));
				}
			}
		}
	}
}

void USaveLoadSubsystem::SetLastPlayedSaveSlot()
{
	CurrentSessionSaveSlot = GetLastPlayedSaveSlot();
}
