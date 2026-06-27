// Property of Paracosm Industries.


#include "BuildSystem/BuildableBase.h"

#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/StatusEffect/StatusEffectHandlerComponent.h"
#include "Data/DataAssets/StatusEffects/StatusEffectBase.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Subsystems/SFX/SFXManagerSubsystem.h"
#include "UObject/ConstructorHelpers.h"


class ATimberEnemyCharacter;
// Sets default values
ABuildableBase::ABuildableBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>("RootSceneComponent");
	RootComponent = RootSceneComponent;
}

void ABuildableBase::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer = GameplayTagContainer;
}

// Called when the game starts or when spawned
void ABuildableBase::BeginPlay()
{
	Super::BeginPlay();

	//This creates a new Guid after the Actor is Spawned but only if its not Loaded.
	//WHen its loaded the Actor receives the GUID before begin play.
	
	BuildSystemGUID = FGuid::NewGuid();
	
	
}

void ABuildableBase::HandleDeletionOfBuildable()
{

	/*Case 1
	 * BuildingComponent ->Wall or floor
	 * Needs to Account for any traps or Constructs built on this Building Component
	 * Needs to also "delete" those components and call SpawnLoot for each of them.
	 */

	/*Case 2
	 * Trap or Construct
	 * Needs to free up the slot on the Building Component they were attached to, to allow a new placement.
	 * 
	 */
	
	//Handles Loot and Destruction
	SpawnLootInRange(BuildableCost.CostOfParts, BuildableCost.CostOfMechanisms, BuildableCost.CostOfUniques);

	USFXManagerSubsystem* SFXManager = GetGameInstance()->GetSubsystem<USFXManagerSubsystem>();
	if (SFXManager)
	{
		SFXManager->PlaySound("BuildDestroyedRandom");
	}
	
	Destroy();
}

FGuid ABuildableBase::GetGUID()
{
	//UE_LOG(LogTemp, Warning, TEXT("Getting GUID for Actor: %s"), *GetName());
	return BuildSystemGUID;
}

void ABuildableBase::SetGUID(FGuid NewGUID)
{
	BuildSystemGUID = NewGUID;
}

/*void ABuildableBase::AddEffectToEnemy(AActor* EnemyActor, FStatusEffect& Effect)
{
	/*
	 * Used for Traps and Constructs that add effects to enemies.
	 #1#
	ATimberEnemyCharacter* EnemyCharacter = Cast<ATimberEnemyCharacter>(EnemyActor);
	if (EnemyCharacter && !EnemyCharacter->IsPendingKillPending() && EnemyCharacter->StatusEffectHandler)
	{
		
		EnemyCharacter->StatusEffectHandler->AddStatusEffectToComponent(Effect);
		//UE_LOG(LogTemp, Warning, TEXT("Added %s to Enemy: %s"), *Effect.EffectIdTag.ToString(), *EnemyCharacter->GetName());
	}
}*/

void ABuildableBase::GetAllStaticMeshComponents()
{
	GetComponents<UStaticMeshComponent>(StaticMeshComponents);
}

void ABuildableBase::HandleStaticMeshWalkableSlope(AActor* NoWalkingBuildable)
{
	//Parent Buildable is a Vertical Wall
	if (ABuildableBase* Buildable = Cast<ABuildableBase>(NoWalkingBuildable))
	{
		if (Buildable->bIsWalkable == false)
		{
			//Get All the Static Meshes for this Buildable
			GetAllStaticMeshComponents();
			
			if (StaticMeshComponents.Num() > 0)
			{
				for (UStaticMeshComponent* SomeMesh : StaticMeshComponents)
				{
					SomeMesh->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.0f));
					//UE_LOG(LogTemp, Warning, TEXT("Set Walkable Slope Override on Static Mesh Component: %s"), *SomeMesh->GetName());
				}
			}
		}
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Parent Buildable is not a Vertical Wall or not valid."));
	}
}

void ABuildableBase::SpawnLootInRange(int NumberOfParts, int NumberOfMechanisms, int NumberOfUniques)
{
	FActorSpawnParameters SpawnParameters;
	FVector ActorLocation = GetActorLocation();
	ActorLocation.Z += 100.0f;
	FVector SpawnLocation;
	for(int i = 0; i < NumberOfParts; i++)
	{
		SpawnLocation = FVector(FMath::RandRange(-50, 50) + ActorLocation.X, FMath::RandRange(-50, 50) + ActorLocation.Y, FMath::RandRange(0, 20) + ActorLocation.Z );
		GetWorld()->SpawnActor<AEnemyLootDropBase>(PartsClass, SpawnLocation, FRotator::ZeroRotator, SpawnParameters);
	}
	
	for(int i = 0; i < NumberOfMechanisms; i++)
	{
		SpawnLocation = FVector(FMath::RandRange(-50, 50) + ActorLocation.X, FMath::RandRange(-50, 50) + ActorLocation.Y, FMath::RandRange(0, 20) + ActorLocation.Z );
		GetWorld()->SpawnActor<AEnemyLootDropBase>(MechanismsClass, SpawnLocation, FRotator::ZeroRotator, SpawnParameters);
	}

	for(int i = 0; i < NumberOfUniques; i++)
	{
		SpawnLocation = FVector(FMath::RandRange(-50, 50) + ActorLocation.X, FMath::RandRange(-50, 50) + ActorLocation.Y, FMath::RandRange(0, 20) + ActorLocation.Z );
		GetWorld()->SpawnActor<AEnemyLootDropBase>(UniquesClass, SpawnLocation, FRotator::ZeroRotator, SpawnParameters);
	}
	
}

// Called every frame
void ABuildableBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
