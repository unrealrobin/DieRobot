// Property of Paracosm Industries. Dont use my shit.


#include "Character/TimberSeeda.h"

#include "Character/TimberPlayableCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/Inventory/InventoryManagerComponent.h"
#include "Controller/TimberPlayerController.h"
#include "GameModes/TimberGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Subsystems/GameConfig/DieRobotGameConfigSubsystem.h"

class UDieRobotGameConfigSubsystem;

ATimberSeeda::ATimberSeeda()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<UCapsuleComponent>("Collision Sphere");
	RootComponent = CollisionSphere;

	DataSeedBase = CreateDefaultSubobject<UStaticMeshComponent>("Data Seed Base");
	DataSeedBase->SetupAttachment(RootComponent);

	DataSeedCore = CreateDefaultSubobject<UStaticMeshComponent>("Core");
	DataSeedUpperPanel = CreateDefaultSubobject<UStaticMeshComponent>("Upper Panel");
	DataSeedLowerPanel = CreateDefaultSubobject<UStaticMeshComponent>("Lower Panel");
	DataSeedSmallLeftUpper = CreateDefaultSubobject<UStaticMeshComponent>("Small Left Lower Panel");
	DataSeedSmallRightUpper = CreateDefaultSubobject<UStaticMeshComponent>("Small Right Lower Panel");
	DataSeedSmallLeftLower = CreateDefaultSubobject<UStaticMeshComponent>("Small Left Upper Panel");
	DataSeedSmallRightLower = CreateDefaultSubobject<UStaticMeshComponent>("Small Right Upper Panel");

	DataSeedCore->SetupAttachment(DataSeedBase);
	
	DataSeedUpperPanel->SetupAttachment(DataSeedCore);
	DataSeedLowerPanel->SetupAttachment(DataSeedCore);
	DataSeedSmallLeftUpper->SetupAttachment(DataSeedCore);
	DataSeedSmallRightUpper->SetupAttachment(DataSeedCore);
	DataSeedSmallLeftLower->SetupAttachment(DataSeedCore);
	DataSeedSmallRightLower->SetupAttachment(DataSeedCore);
	
	InteractOverlapSphere = CreateDefaultSubobject<UCapsuleComponent>("Interact Sphere");
	InteractOverlapSphere->SetupAttachment(RootComponent);
	
	RepairWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Repair Widget Component");
	RepairWidgetComponent->SetupAttachment(DataSeedBase);
	//RepairWidgetComponent->RegisterComponent();
}

void ATimberSeeda::BeginPlay()
{
	Super::BeginPlay();

	// Used when Seeda is respawned to allow Listeners to Rebind to any Delegates
	ATimberGameModeBase* GM = Cast<ATimberGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->OnCharacterInitialization.AddDynamic(this, &ATimberSeeda::HandleCharacterBindingToSeeda);
	}

	/*
	 *Typically Seeda waits for the Initialization of the Character to Broadcast to the Character to bind to the Seeda Delegates.
	 * When Seeda is Respawned during Runtime, the Character Initialization is never Broadcast, because the Broadcast occured during the Character's Begin Play.
	 * So Seeda checks on Respawn if the character is valid, and if it is, ensures the character now rebinds to the required delegates.
	 */
	
	ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(UGameplayStatics::GetActorOfClass(this, ATimberPlayableCharacter::StaticClass()));
	if (PlayerCharacter)
	{
		HandleCharacterBindingToSeeda();
	}

	InteractOverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ATimberSeeda::AddInteractableToPlayer);
	InteractOverlapSphere->OnComponentEndOverlap.AddDynamic(this, &ATimberSeeda::RemoveInteractableFromPlayer);

	HandleRepairWidget();

	SpawnLocationMarkerForTutorial();
}

void ATimberSeeda::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//If widget is visible, rotate it to the player.
	RotateWidgetToPlayer();
}

void ATimberSeeda::TakeDamage_Seeda(float DamageAmount)
{
	CurrentHealth -= DamageAmount;
	if (CurrentHealth <= 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Seeda Destroyed."));

		//Handles UI Updates required before other logic.
		OnSeedaDeathUI.Broadcast(true);
		//UE_LOG(LogTemp, Warning, TEXT("Broadcast Seeda Death to HUD."));

		//Handles Destruction, Calls to Player, Player Controller, GameMode.
		OnSeedaDeath.Broadcast(true);
		//UE_LOG(LogTemp, Warning, TEXT("Broadcast Seeda Death to player."));

		//TODO::Play Some Sound or Animation Signifying the Death of Seeda
		//Destroy();
	}
	
}

void ATimberSeeda::HandleCharacterBindingToSeeda()
{
	ATimberGameModeBase* GM = Cast<ATimberGameModeBase>(GetWorld()->GetAuthGameMode());
	if(GM)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Seeda Broadcasts to GameMode"))
		GM->OnSeedaSpawn.Broadcast(this);
	}
}

void ATimberSeeda::RepairSeeda()
{
	//UE_LOG(LogTemp, Warning, TEXT("Seeda Repaired."));

	//CheckPlayer Inventory for Repair Items

	ATimberPlayableCharacter* Player = Cast<ATimberPlayableCharacter>(UGameplayStatics::GetActorOfClass
	(GetWorld(), ATimberPlayableCharacter::StaticClass()));

	if (CurrentHealth >= MaxHealth)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Seeda is already at Full Health."));
		return;
	}

	if (Player)
	{
		if (Player->InventoryManager)
		{
			int AvailableParts = Player->InventoryManager->GetPartsInInventory();
			if (AvailableParts >= PartsToRepairSeeda)
			{
				Player->InventoryManager->RemovePartsFromInventory(2);
				if (CurrentHealth + HealthAmountGainedOnRepair > MaxHealth)
				{
					CurrentHealth = MaxHealth;
				}
				else
				{
					{
						CurrentHealth += HealthAmountGainedOnRepair;
					}
				}
			}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("Not Enough Parts to Repair Seeda."));
			}
		}
	}
}

void ATimberSeeda::Interact()
{
	//UE_LOG(LogTemp, Warning, TEXT("Seeda Interacted with."));

	RepairSeeda();
}

void ATimberSeeda::AddInteractableToPlayer(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		ATimberPlayerController* PC = Cast<ATimberPlayerController>(PlayerCharacter->GetController());
		if (PC)
		{
			PC->SetInteractableItem(this);
			
			if (RepairWidget)
			{
				//Show the Widget when the Player is in Range.
				RepairWidget->SetVisibility(ESlateVisibility::Visible);
				//UE_LOG(LogTemp, Warning, TEXT("Seeda - Repair Widget Visible."));
			}
		}
	}
}

void ATimberSeeda::RemoveInteractableFromPlayer(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		ATimberPlayerController* PC = Cast<ATimberPlayerController>(PlayerCharacter->GetController());
		if (PC)
		{
			PC->ClearInteractableItem();

			if (RepairWidget)
			{
				//Hide the Widget when the Player is out of Range.
				RepairWidget->SetVisibility(ESlateVisibility::Hidden);
				//UE_LOG(LogTemp, Warning, TEXT("Seeda - Repair Widget Not Visible."));
			}
		}
	}
}

void ATimberSeeda::SpawnLocationMarkerForTutorial()
{
	//Only spawning this in the standard game mode if tutorial is in the first state.
	ADieRobotGameStateBase* DieRobotGameState = Cast<ADieRobotGameStateBase>(GetWorld()->GetGameState());
	UDieRobotGameConfigSubsystem* DieRobotGameConfig = GetGameInstance()->GetSubsystem<UDieRobotGameConfigSubsystem>();
	
	if (DieRobotGameState && DieRobotGameState->TutorialState == ETutorialState::Wake1 && 
		DieRobotGameConfig->GameConfig == EDieRobotGameConfigType::Standard)
	{
		//Spawn the Location Marker for the Tutorial
		FActorSpawnParameters SpawnParams;

		FVector SeedaLocation = GetActorLocation();

		//Arbitrary Z Offset to ensure the Location Marker is above the Seeda
		SeedaLocation.Z -= 100.f;
		
		if (TutorialLocationMarkerClass)
		{
			GetWorld()->SpawnActor<AActor>(TutorialLocationMarkerClass, SeedaLocation, GetActorRotation(), SpawnParams);
		}
	}
	
}

void ATimberSeeda::HandleRepairWidget()
{
	if (RepairWidgetComponent)
	{
		RepairWidget = RepairWidgetComponent->GetUserWidgetObject();
		if (RepairWidget)
		{
			RepairWidget->SetVisibility(ESlateVisibility::Hidden);
			//UE_LOG(LogTemp, Warning, TEXT("Seeda - Repair Widget initialized and stored. Hidden."))
		}
	}
}

void ATimberSeeda::RotateWidgetToPlayer()
{
	if (RepairWidgetComponent && RepairWidget && RepairWidget->IsVisible())
	{
		//Get the Camera of the Player @ Index 0 (Single Player Game)
		APlayerCameraManager* CamManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
		FVector CameraLoc = CamManager->GetCameraLocation();

		// Calculate a look-at rotation from this actor’s location to the camera
		FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CameraLoc);

		// Apply rotation
		RepairWidgetComponent->SetWorldRotation(LookRotation);
	}
}

