// Property of Paracosm Industries. Dont use my shit.


#include "GameModes/StartUpGameMode.h"

#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameConfig/DieRobotGameConfigSubsystem.h"
#include "Subsystems/Music/UMusicManagerSubsystem.h"
#include "Subsystems/SaveLoad/SaveLoadSubsystem.h"
#include "Subsystems/Wave/WaveGameInstanceSubsystem.h"

void AStartUpGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeGameConfigSubsystem();
	InitializeStartUpMenu();
	InitializeMusicManager();
	
	ResetWaveSubsystem();

	InitializeLastLoadedGameSlot();
}

void AStartUpGameMode::InitializeMusicManager()
{
	//We must Initialize the Music Manager in Every Game Mode so that it has the World Context to play Audio
	UUMusicManagerSubsystem* MusicManager = GetWorld()->GetGameInstance()->GetSubsystem<UUMusicManagerSubsystem>();
	if (MusicManager)
	{
		//AudioComponent Creation
		MusicManager->HandleInitialization();

		//Play the Startup1 Song
		MusicManager->PlayMusic("CyberpunkBoy", 0.1f);
		
		UE_LOG(LogTemp, Warning, TEXT("Startup Level Initialized the Music Manager and Played Song."))
	}
}

void AStartUpGameMode::InitializeStartUpMenu()
{
	if (!StartUpMenu )
	{
		StartUpMenu = CreateWidget<UUserWidget>(GetWorld(), StartUpMenuClass );
		if (StartUpMenu)
		{
			StartUpMenu->AddToViewport();
		}
	}
}

void AStartUpGameMode::InitializeGameConfigSubsystem()
{
	UDieRobotGameConfigSubsystem* DieRobotGameConfig = GetGameInstance()->GetSubsystem<UDieRobotGameConfigSubsystem>();
	if (DieRobotGameConfig)
	{
		DieRobotGameConfig->GameConfig = EDieRobotGameConfigType::MainMenu;
	}
}

void AStartUpGameMode::InitializeLastLoadedGameSlot()
{
	USaveLoadSubsystem* SaveLoadSubsystem = GetGameInstance()->GetSubsystem<USaveLoadSubsystem>();
	if (SaveLoadSubsystem)
	{
		//When a game is loaded it sets the Current Save Slot on the Save Load Subsystem.
		// WHen the Game is saved, the value is then set on the Global Save Data File.
		//Here we retrieve from the Global Save Data and repopulate that data
		//TimberGameModeBase then uses that value
		SaveLoadSubsystem->SetLastPlayedSaveSlot();
	}
}

void AStartUpGameMode::ResetWaveSubsystem()
{
	if (UWaveGameInstanceSubsystem* WaveSubsystem = GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>())
	{
		WaveSubsystem->ResetWaveSubsystem();
	}
}

void AStartUpGameMode::SetGameConfig(EDieRobotGameConfigType InGameState)
{
	UDieRobotGameConfigSubsystem* DieRobotGameConfig = GetGameInstance()->GetSubsystem<UDieRobotGameConfigSubsystem>();
	
	if (DieRobotGameConfig)
	{
		DieRobotGameConfig->GameConfig = InGameState;
	}
}

void AStartUpGameMode::SwitchToLabLevel()
{
	//Called from the PlayDemo Button in the Startup Menu - See Startup Widget
	if (StartUpMenu)
	{
		StartUpMenu->RemoveFromParent();
		StartUpMenu = nullptr;

		//Runs Tutorial Etc. Standard GamePlay
		SetGameConfig(EDieRobotGameConfigType::Standard);
		
		UE_LOG(LogTemp, Warning, TEXT("Startup Game Mode - Set Game Config to Standard"));

		UGameplayStatics::OpenLevel(GetWorld(), FName("TheLab"));
		
	}
}

void AStartUpGameMode::SwitchToMidgameDemo()
{
	if (StartUpMenu)
	{
		StartUpMenu->RemoveFromParent();
		StartUpMenu = nullptr;

		//Should load from a preconfigured Save Slot.

		UWaveGameInstanceSubsystem* WaveSubsystem  = GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>();
		if (WaveSubsystem)
		{
			WaveSubsystem->CurrentWaveNumber = 9;
		}
		
		SetGameConfig(EDieRobotGameConfigType::MidGameDemo);
		UE_LOG(LogTemp, Warning, TEXT("Startup Game Mode - Set Game Config to MidGameDemo"));

		
		UGameplayStatics::OpenLevel(GetWorld(), FName("TheLab"));
	}
}



