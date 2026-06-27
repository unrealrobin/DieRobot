// Property of Paracosm Industries. Dont use my shit.


#include "Character/DieRobotSeeda.h"

#include "Character/DieRobotPlayableCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/Inventory/InventoryManagerComponent.h"
#include "Controller/DieRobotPlayerController.h"
#include "GameModes/DieRobotGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Subsystems/GameConfig/DieRobotGameConfigSubsystem.h"

class UDieRobotGameConfigSubsystem;

ADieRobotSeeda::ADieRobotSeeda()
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

void ADieRobotSeeda::BeginPlay()
{
	Super::BeginPlay();

	// Used when Seeda is respawned to allow Listeners to Rebind to any Delegates
	ADieRobotGameModeBase* GM = Cast<ADieRobotGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->OnCharacterInitialization.AddDynamic(this, &ADieRobotSeeda::HandleCharacterBindingToSeeda);
	}

	/*
	 *Typically Seeda waits for the Initialization of the Character to Broadcast to the Character to bind to the Seeda Delegates.
	 * When Seeda is Respawned during Runtime, the Character Initialization is never Broadcast, because the Broadcast occured during the Character's Begin Play.
	 * So Seeda checks on Respawn if the character is valid, and if it is, ensures the character now rebinds to the required delegates.
	 */
	
	ADieRobotPlayableCharacter* PlayerCharacter = Cast<ADieRobotPlayableCharacter>(UGameplayStatics::GetActorOfClass(this, ADieRobotPlayableCharacter::StaticClass()));
	if (PlayerCharacter)
	{
		HandleCharacterBindingToSeeda();
	}

	InteractOverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ADieRobotSeeda::AddInteractableToPlayer);
	InteractOverlapSphere->OnComponentEndOverlap.AddDynamic(this, &ADieRobotSeeda::RemoveInteractableFromPlayer);

	HandleRepairWidget();

	SpawnLocationMarkerForTutorial();
}

void ADieRobotSeeda::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//If widget is visible, rotate it to the player.
	RotateWidgetToPlayer();
}

void ADieRobotSeeda::TakeDamage_Seeda(float DamageAmount)
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

void ADieRobotSeeda::HandleCharacterBindingToSeeda()
{
	ADieRobotGameModeBase* GM = Cast<ADieRobotGameModeBase>(GetWorld()->GetAuthGameMode());
	if(GM)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Seeda Broadcasts to GameMode"))
		GM->OnSeedaSpawn.Broadcast(this);
	}
}

void ADieRobotSeeda::RepairSeeda()
{
	//UE_LOG(LogTemp, Warning, TEXT("Seeda Repaired."));

	//CheckPlayer Inventory for Repair Items

	ADieRobotPlayableCharacter* Player = Cast<ADieRobotPlayableCharacter>(UGameplayStatics::GetActorOfClass
	(GetWorld(), ADieRobotPlayableCharacter::StaticClass()));

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

void ADieRobotSeeda::Interact()
{
	//UE_LOG(LogTemp, Warning, TEXT("Seeda Interacted with."));

	RepairSeeda();
}

void ADieRobotSeeda::AddInteractableToPlayer(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	ADieRobotPlayableCharacter* PlayerCharacter = Cast<ADieRobotPlayableCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		ADieRobotPlayerController* PC = Cast<ADieRobotPlayerController>(PlayerCharacter->GetController());
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

void ADieRobotSeeda::RemoveInteractableFromPlayer(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ADieRobotPlayableCharacter* PlayerCharacter = Cast<ADieRobotPlayableCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		ADieRobotPlayerController* PC = Cast<ADieRobotPlayerController>(PlayerCharacter->GetController());
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

void ADieRobotSeeda::SpawnLocationMarkerForTutorial()
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

void ADieRobotSeeda::HandleRepairWidget()
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

void ADieRobotSeeda::RotateWidgetToPlayer()
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

