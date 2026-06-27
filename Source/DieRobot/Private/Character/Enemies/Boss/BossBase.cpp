// Property of Paracosm Industries. 


#include "Character/Enemies/Boss/BossBase.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Loot/EnemyLootDropBase.h"
#include "Loot/LootHealthDropMax.h"


// Sets default values
ABossBase::ABossBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABossBase::BeginPlay()
{
	Super::BeginPlay();

	if (CharacterMovementComponent)
	{
		CharacterMovementComponent->SetWalkableFloorAngle(56.0f);
		CharacterMovementComponent->MaxWalkSpeed = MaxWalkSpeedBase;
		CharacterMovementComponent->NavAgentProps.AgentRadius = 200.0f;
		CharacterMovementComponent->NavAgentProps.AgentHeight = 200.0f;
		CharacterMovementComponent->NavAgentProps.AgentStepHeight = 65.0f;
	}
	
	OnDestroyed.AddDynamic(this, &ABossBase::BroadcastDeathDelegate);
}

// Called every frame
void ABossBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/*void ABossBase::HandleDropHealthLoot(TSubclassOf<AEnemyLootDropBase> HealthDropClass)
{

	//Bosses always Drop a Max Health
	if (HealthDropClass->IsChildOf(ALootHealthDropMax::StaticClass()))
	{
		SpawnLoot(HealthDropClass);
	}

	//This calls the Original function which has a 5% chance for another Max Health and a 20% chance for a normal health drop.
	Super::HandleDropHealthLoot(HealthDropClass);
}*/

void ABossBase::BroadcastDeathDelegate(AActor* DestroyedActor)
{

	//TODO:: Need to call the HUD somehow and tell it to remove the boss Health Bar widget
	
	OnBossDeath.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("BossBase - Broadcasting Boss Death."));
}


