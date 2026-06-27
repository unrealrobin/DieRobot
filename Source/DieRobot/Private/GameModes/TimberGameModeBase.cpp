// Property of Paracosm Industries.
#include "GameModes/TimberGameModeBase.h"

#include "BrainComponent.h"
#include "AI/TimberAiControllerBase.h"
#include "Character/TimberSeeda.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/BuildSystem/BuildSystemManagerComponent.h"
#include "Controller/TimberPlayerController.h"
#include "Environment/DynamicLab.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Environment/LabDoorBase.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameConfig/DieRobotGameConfigSubsystem.h"
#include "Subsystems/Music/UMusicManagerSubsystem.h"
#include "Subsystems/SaveLoad/SaveLoadSubsystem.h"
#include "Subsystems/Wave/WaveGameInstanceSubsystem.h"
#include "SentrySubsystem.h"

class UDialogueManager;
class UBuildingComponentPanel;

void ATimberGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	PlayBuildMusic();
	
	InitializeGameState();
	
	GetWaveGameInstanceSubsystem()->OpenLabDoorHandle.AddDynamic(this, &ATimberGameModeBase::OpenLabDoors);
	GetWaveGameInstanceSubsystem()->CloseLabDoorHandle.AddDynamic(this, &ATimberGameModeBase::CloseLabDoors);
	GetWaveGameInstanceSubsystem()->OnWaveComplete.AddDynamic(this, &ATimberGameModeBase::HandleWaveComplete);
	
	GetWaveGameInstanceSubsystem()->PrepareSpawnPoints();
	GatherSeedaData();
	GatherAllLabDoors();

	if (WaveCompositionCurveTable)
	{
		GetWaveGameInstanceSubsystem()->SetWaveCompositionCurveTable(WaveCompositionCurveTable);
	}

	InitializeSaveLoadSession();
}

void ATimberGameModeBase::PathTracer_RedrawDelegateBinding()
{
	/*
	 * When the build system component on the character spawns a building component at some location
	 * the Game Mode will then Redraw the Path Trace to show the player where they can build.
	 */
	
	if(GetWorld())
	{
		if(TimberCharacter && TimberCharacter->BuildSystemManager)
		{
			
			TimberCharacter->BuildSystemManager->RedrawPathTraceHandle.AddDynamic(this, 
				&ATimberGameModeBase::HandleRedrawPathTrace);
		}
	}
}

void ATimberGameModeBase::GatherSeedaData()
{
	Seeda = Cast<ATimberSeeda>(UGameplayStatics::GetActorOfClass(GetWorld(), ATimberSeeda::StaticClass()));
	if (Seeda)
	{
		SeedaLocation = Seeda->GetActorLocation();

		//Passing Seeda Class to Save Load Susbsytem for future Seeda Spawning.
		if (USaveLoadSubsystem* SLSubsystem = GetGameInstance()->GetSubsystem<USaveLoadSubsystem>())
		{
			if (Seeda)
			{
				SLSubsystem->SeedaClass = Seeda->GetClass();
			}
		}
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("GameMode - GatherSeedaData() - Seeda Not Found in World."));
	}
}

/*Game State*/
void ATimberGameModeBase::InitializeGameState()
{
	ADieRobotGameStateBase* DieRobotGameState = Cast<ADieRobotGameStateBase>(GetWorld()->GetGameState());
	UDieRobotGameConfigSubsystem* DieRobotGameConfig = GetGameInstance()->GetSubsystem<UDieRobotGameConfigSubsystem>();
	
	if (DieRobotGameState && DieRobotGameConfig)
	{
		//Standard Game with Tutorial
		if (DieRobotGameConfig->GameConfig == EDieRobotGameConfigType::Standard)
		{
			//UE_LOG(LogTemp, Warning, TEXT("ATimberGameModeBase - Initialized Standard Game State."))
			DieRobotGameState->OnTutorialStateChange.AddDynamic(this, &ATimberGameModeBase::UpdateTutorialState);
			DieRobotGameState->OnTutorialStateChange.AddDynamic(this, &ATimberGameModeBase::HandleTutorialStateChange);
			
			GetTutorialState();
			
			//Just initiating the Broadcast
			if (TutorialState == ETutorialState::Wake1)
			{
				DieRobotGameState->ChangeTutorialGameState(ETutorialState::Wake1);
				SpawnDummyForTutorial();
			}
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("GameModeBase - No GameState Set."))
		}
	}
}

void ATimberGameModeBase::GetTutorialState()
{
	ADieRobotGameStateBase* DieRobotGameState = Cast<ADieRobotGameStateBase>(GetWorld()->GetGameState());
	if (DieRobotGameState)
	{
		TutorialState = DieRobotGameState->TutorialState;
	}
}

void ATimberGameModeBase::UpdateTutorialState(ETutorialState NewState)
{
	//Updates Locally on the Game Mode
	TutorialState = NewState;
}

void ATimberGameModeBase::SpawnDummyForTutorial()
{
	FString DummyAssetLocationString = "/Game/Blueprints/Character/TutorialDummy/BP_TutorialDummy.BP_TutorialDummy_C";

	// Load Blueprint asset
	UClass* DummyBPClass = LoadObject<UClass>(nullptr, *DummyAssetLocationString);
    
	const FVector SpawnLocation = FVector(1845.0f, 2916.0f, 150.0f);
	const FRotator SpawnRotation = FRotator(180.f, 0.f, -180.f);
    
	if (DummyBPClass)
	{
		FActorSpawnParameters SpawnParams;
		GetWorld()->SpawnActor<ATimberEnemyCharacter>(DummyBPClass, SpawnLocation, SpawnRotation, SpawnParams);
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("Failed to load Dummy Blueprint Class."));
	}
}

void ATimberGameModeBase::HandleTutorialStateChange(ETutorialState NewState)
{
	/*if (NewState == ETutorialState::Wake2)
	{
		SpawnLocationMarker();
	}*/
	/*if (NewState == ETutorialState::Wake3)
	{
		UDialogueManager* DialogueManager = GetWorld()->GetGameInstance()->GetSubsystem<UDialogueManager>();
		if (DialogueManager)
		{
			DialogueManager->PlayVoiceover("Molly_Wake_3");
		}
	}*/
	if (NewState == ETutorialState::WaveStart)
	{
		UWaveGameInstanceSubsystem* WaveSubsystem = GetWaveGameInstanceSubsystem();
		if (WaveSubsystem)
		{
			//Starting wave 1 (Tutorial Wave)
			WaveSubsystem->StartWave();
			//UE_LOG(LogTemp, Warning, TEXT("GameModeBase - Starting Wave 1."));
		}
		else
		{
			//UE_LOG(LogTemp, Error, TEXT("GameModeBase - Wave Subsystem Not Found."));
		}
	}
}

void ATimberGameModeBase::PassDataTableToWaveSubsystem(UDataTable* DataTable)
{
	GetWaveGameInstanceSubsystem()->SetWaveCompositionDataTable(DataTable);
	//UE_LOG(LogTemp, Warning, TEXT("Game Mode - Received DataTable and Passed to Wave Subsystem"));
}

/* Tells all relying on systems that the character is initialized */
void ATimberGameModeBase::PlayerIsInitialized(AActor* InitializedPlayer)
{
	//UE_LOG(LogTemp, Warning, TEXT("TimberGameModeBase - Player Is Initialized."));
	TimberCharacter = Cast<ATimberPlayableCharacter>(InitializedPlayer);
	if(IsValid(TimberCharacter))
	{
		TimberCharacter->HandlePlayerDeath_DelegateHandle.RemoveDynamic(this, &ATimberGameModeBase::FreezeAllAICharacters);
		TimberCharacter->HandlePlayerDeath_DelegateHandle.AddDynamic(this, &ATimberGameModeBase::FreezeAllAICharacters);
		OnCharacterInitialization.Broadcast();
		PathTracer_RedrawDelegateBinding();
	}
}

//Switches to the Main Menu Level from Game Play Level.
void ATimberGameModeBase::SwitchToMainMenu()
{
	//Stopping the Subsystem from continuing to spawn enemies.
	UWaveGameInstanceSubsystem* WaveGameInstanceSubsystem = GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>();
	if (WaveGameInstanceSubsystem)
	{
		//Whenever you switch to the main menu, you assume the player is either dead or has quit the game and has saved any progress they want to keep.
		WaveGameInstanceSubsystem->EndWave(EWaveStopReason::LevelSwitch);
	}
	
	//Handles Switching Levels.
	UGameplayStatics::OpenLevel(GetWorld(), FName("StartUp"));
	//UE_LOG(LogTemp, Warning, TEXT("TimberGameModeBase - Switching to Main Menu."));
	
}

UWaveGameInstanceSubsystem* ATimberGameModeBase::GetWaveGameInstanceSubsystem()
{
	return GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>();
}

void ATimberGameModeBase::PlayBuildMusic()
{
	UUMusicManagerSubsystem* MusicManager = GetGameInstance()->GetSubsystem<UUMusicManagerSubsystem>();

	if (MusicManager)
	{
		//Delays the playing of the music by InRate - Might be useful for a fade in effect later.

		FTimerHandle DelayHandle;
		GetWorld()->GetTimerManager().SetTimer(DelayHandle, FTimerDelegate::CreateLambda([MusicManager]()
		{
			UE_LOG(LogTemp, Warning, TEXT("Playing Startup Music."));
			MusicManager->PlayMusic("Build1", 0.1f);	
		}), 0.1f, false);
		
	}
}

void ATimberGameModeBase::PlayAttackMusic()
{
	UUMusicManagerSubsystem* MusicManager = GetGameInstance()->GetSubsystem<UUMusicManagerSubsystem>();

	if (MusicManager)
	{
		//Delays the playing of the music by InRate - Might be useful for a fade in effect later.
		FTimerHandle DelayHandle;
		GetWorld()->GetTimerManager().SetTimer(DelayHandle, FTimerDelegate::CreateLambda([MusicManager]()
		{
			MusicManager->PlayMusic("Attack1", 1.0f);	
		}), 1.0f, false);
	}
}

void ATimberGameModeBase::TestSentry()
{
	//UGameInstance* GameInstance = GetGameInstance();
	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
	SentrySubsystem->CaptureMessage(TEXT("Captured Message from Die Robot"));
	
}

void ATimberGameModeBase::InitializeSaveLoadSession()
{
	USaveLoadSubsystem* SaveLoadSubsystem = GetGameInstance()->GetSubsystem<USaveLoadSubsystem>();
	if (SaveLoadSubsystem)
	{
		/* Tries to load the current session from the GUID ID on the Save Load Subsystem
		 * If this is a new game, this will fail on the first run because no SaveLoadStruct file is created.
		 */
		SaveLoadSubsystem->LoadGame(SaveLoadSubsystem->GetCurrentSessionSaveSlot());

		/*
		 * On a new game, this creates the original save file. Otherwise, it is a redundant Save.
		 */
		SaveLoadSubsystem->SaveCurrentGame();
	}
}

void ATimberGameModeBase::GatherAllLabDoors()
{
	AActor* LabActor = UGameplayStatics::GetActorOfClass(GetWorld(), ADynamicLab::StaticClass());
	ADynamicLab* Lab = Cast<ADynamicLab>(LabActor);
	if (Lab)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Lab Asset Found. Gathering Lab Doors"));
		TArray<UChildActorComponent*> ChildComponents;
		Lab->GetComponents<UChildActorComponent*>(ChildComponents);
		//UE_LOG(LogTemp, Warning, TEXT(" There are %d Child Components"), ChildComponents.Num());
		for (UChildActorComponent* Child : ChildComponents)
		{
			if (Cast<ALabDoorBase>(Child->GetChildActor()))
			{
				ArrayOfLabDoors.Add(Cast<AActor>((Child->GetChildActor())));
				//UE_LOG(LogTemp, Warning, TEXT("Gathered Lab Door: %s"), *Child->GetName());
			}
		}
	}
	else
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALabDoorBase::StaticClass(), ArrayOfLabDoors);
		//UE_LOG(LogTemp, Warning, TEXT("Could not get Dynamic Lab Actor."));
	}
}

void ATimberGameModeBase::HandleRedrawPathTrace()
{
	RedrawPathTrace();
}

//Used to Freeze all AI Characters when the Player Dies.
void ATimberGameModeBase::FreezeAllAICharacters(bool bIsPlayerDead)
{
	//UE_LOG(LogTemp, Warning, TEXT("Game Mode - Freezing AI Characters."));
	TArray<AActor*> ArrayOfAICharacters;
	UGameplayStatics::GetAllActorsOfClass(this, ATimberEnemyCharacter::StaticClass(), ArrayOfAICharacters);

	for (AActor* Character : ArrayOfAICharacters)
	{
		ATimberEnemyCharacter* CharacterEnemy = Cast<ATimberEnemyCharacter>(Character);

		if (CharacterEnemy)
		{
			//Stopping Enemy Movement
			CharacterEnemy->GetCharacterMovement()->StopMovementImmediately();

			//Stopping Enemy AI Tree Logic
			if (CharacterEnemy->GetController())
			{
				ATimberAiControllerBase* AIController = Cast<ATimberAiControllerBase>(CharacterEnemy->GetController());
				if (AIController)
				{
					UBrainComponent* Brain = AIController->BrainComponent;
					if (Brain)
					{
						Brain->StopLogic("Freezing because Player Death");
					}
				}
			}
		}
	}
}

void ATimberGameModeBase::OpenAllLabDoors()
{
	PlayAttackMusic();
	
	for(AActor* LabDoors : ArrayOfLabDoors)
	{
		ALabDoorBase* LabDoor = Cast<ALabDoorBase>(LabDoors);
		if (LabDoor)
		{
			LabDoor->OpenLabDoor(GetWorld()->GetDeltaSeconds());
		}
	}
}

void ATimberGameModeBase::OpenLabDoors()
{
	if(ArrayOfLabDoors.Num() <= 0)
	{
		GatherAllLabDoors();
		OpenAllLabDoors();
	}
	else
	{
		OpenAllLabDoors();
	}
}

void ATimberGameModeBase::CloseAllLabDoors()
{
	PlayBuildMusic();
	for(AActor* LabDoors : ArrayOfLabDoors)
	{
		ALabDoorBase* LabDoor = Cast<ALabDoorBase>(LabDoors);
		if (LabDoor)
		{
			LabDoor->CloseLabDoor(GetWorld()->GetDeltaSeconds());
		}
	}
}

void ATimberGameModeBase::CloseLabDoors()
{
	if(ArrayOfLabDoors.Num() <= 0)
	{
		GatherAllLabDoors();
		CloseAllLabDoors();
	}
	else
	{
		CloseAllLabDoors();
	}
}

void ATimberGameModeBase::HandleWaveComplete(int CompletedWave)
{
	if (CompletedWave == 1)
	{
		ADieRobotGameStateBase* DieRobotGameState = Cast<ADieRobotGameStateBase>(GetWorld()->GetGameState());
		if (DieRobotGameState)
		{
			//Wave 1 Completed, progress to finished Tutorial.
			DieRobotGameState->ChangeTutorialGameState(ETutorialState::WaveComplete);
		}
	}
}


