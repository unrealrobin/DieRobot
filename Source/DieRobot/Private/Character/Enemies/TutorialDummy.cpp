// Property of Paracosm Industries. Dont use my shit.


#include "Character/Enemies/TutorialDummy.h"

#include "Components/CapsuleComponent.h"
#include "Loot/LootHealthDrop.h"
#include "States/DieRobotGameStateBase.h"
#include "Types/Combat/DamagePayload.h"


// Sets default values
ATutorialDummy::ATutorialDummy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionCapsule = GetCapsuleComponent();

	LootDropLocation = CreateDefaultSubobject<USceneComponent>("LootDropSceneComponent");
	LootDropLocation->SetupAttachment(RootComponent);
	
}

// Called when the game starts or when spawned
void ATutorialDummy::BeginPlay()
{
	Super::BeginPlay();

	DieRobotGameStateBase = Cast<ADieRobotGameStateBase>(GetWorld()->GetGameState());

	//Broadcasts to Increment Tutorial State
	OnDestroyed.AddDynamic(this, &ATutorialDummy::HandleDeath);
	
}

void ATutorialDummy::TakeDamage(FDamagePayload DamagePayload)
{
	FDamagePayload Payload;
	Super::TakeDamage(Payload);
	
	if (CurrentHealth <= 0.f)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Tutorial Dummy is Destroyed."));
		Destroy();
	}
	
}

/*void ATutorialDummy::OnDeath_DropLoot()
{
	//TODO:: Rework Loot Drop System after GDC.
	for(TSubclassOf<AEnemyLootDropBase> LootDrop : StandardLootArray)
	{
		if (LootDropLocation)
		{
			//Randomizing exact Spawn Location every iteration
			FVector Base = LootDropLocation->GetComponentLocation();
			float AdjustAmount = 100.f;
			FVector AdjustedLocation = FVector(FMath::RandRange(Base.X - AdjustAmount, Base.X + AdjustAmount), FMath::RandRange(Base.Y - AdjustAmount, Base.Y + AdjustAmount), Base.Z);
			
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			//Spawn at our around the Loot Drop Location
			AActor* LootItem = GetWorld()->SpawnActor<AActor>(LootDrop, AdjustedLocation, FRotator::ZeroRotator, SpawnParams);

			//UE_LOG(LogTemp, Warning, TEXT("Spawned: %s at: %s"), *LootDrop->GetName(), *AdjustedLocation.ToString());
		}
	}
}*/

void ATutorialDummy::HandleDeath(AActor* DeadActor)
{
	//Tutorial State Increment to TutorialState::Parts
	if (DieRobotGameStateBase)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Switching to Parts Tutorial State."));
		DieRobotGameStateBase->ChangeTutorialGameState(ETutorialState::Parts1);
	}
}


