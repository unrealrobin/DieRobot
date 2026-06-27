// Property of Paracosm Industries. Dont use my shit.


#include "BuildSystem/Traps/TrapBase.h"

#include "BuildSystem/BuildingComponents/TimberBuildingComponentBase.h"
#include "BuildSystem/BuildingComponents/TimberVerticalBuildingComponent.h"
#include "Character/TimberPlayableCharacter.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StatusEffect/StatusConditionManager.h"
#include "Components/StatusEffect/StatusEffectHandlerComponent.h"


ATrapBase::ATrapBase()
{
	PrimaryActorTick.bCanEverTick = true;

	TrapBaseStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("TrapBaseStaticMesh");
	TrapBaseStaticMesh->SetupAttachment(RootComponent);
	TrapBaseStaticMesh->SetCollisionProfileName(TEXT("DR_BuildableBlockEverything"));

	HitBoxComponent = CreateDefaultSubobject<UBoxComponent>("DamageArea");
	HitBoxComponent->SetupAttachment(TrapBaseStaticMesh);

	//Only Generates Hit Events on Pawns and Building Components
	HitBoxComponent->SetCollisionProfileName(TEXT("DR_HitEventOnly"));

	//Evaluates Application of Status Effects.
	EffectConditionManager = CreateDefaultSubobject<UStatusConditionManager>("EffectConditionManager");
}

void ATrapBase::BeginPlay()
{
	Super::BeginPlay();
	
	HitBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ATrapBase::HitBoxBeginOverlap);
	HitBoxComponent->OnComponentEndOverlap.AddDynamic(this, &ATrapBase::HitBoxEndOverlap);

	
	FTimerHandle ClearDeadEnemiesTimerHandle;
	float RandomTimerOffset = FMath::FRandRange(0.0f, 3.0f);
	GetWorld()->GetTimerManager().SetTimer(ClearDeadEnemiesTimerHandle, this, &ATrapBase::ClearDeadEnemiesFromArray, 0.1f, true, RandomTimerOffset);
}

void ATrapBase::DisableAllStaticMeshCollisions(UStaticMeshComponent* SomeMesh)
{
	SomeMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	SomeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATrapBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATrapBase::SetIsAmplified(bool bIsAmplified)
{
	if (bIsAmplified)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trap Base - Trap is now Amplified."));
		AmplificationState = EAmplificationState::Amplified;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Trap Base - Trap is now De-Amplified."));
		AmplificationState = EAmplificationState::NotAmplified;
	}
}

void ATrapBase::FreeUpTrapSlotOnBuildingComponent()
{
	if (ATimberBuildingComponentBase* ParentBuildingComponent = Cast<ATimberBuildingComponentBase>(ParentBuildable))
	{
	
		UE_LOG(LogTemp, Warning, TEXT("Parent Building Component Name: %s"), *ParentBuildable->GetName());
		switch (BuildingComponentTrapDirection)
		{
		case EBuildingComponentTrapDirection::Back :
			ParentBuildingComponent->BackCenterAttachment = nullptr;
			UE_LOG(LogTemp, Warning, TEXT("Trap Base - Parent Building Component Back Trap Set to Nullptr."));
			break;
		case EBuildingComponentTrapDirection::Front :
			ParentBuildingComponent->FrontCenterAttachment = nullptr;
			UE_LOG(LogTemp, Warning, TEXT("Trap Base - Parent Building Component Front Trap Set to Nullptr."));
			break;
		case EBuildingComponentTrapDirection::Default:
			UE_LOG(LogTemp, Warning, TEXT("Trap Base - Trap Direction not set on this Trap. Can not Free up Slot on Parent Building Component."))
			break;
		}

		//We need to also remove the trap from the attachments array when deleting. Important.
		if (ParentBuildingComponent->AttachedBuildingComponents.Contains(this))
		{
			ParentBuildingComponent->AttachedBuildingComponents.Remove(this);
		}
	}
}

void ATrapBase::HandleDeletionOfBuildable()
{
	//Free up the slot on the Parent Building Component
	FreeUpTrapSlotOnBuildingComponent();
	UE_LOG(LogTemp, Warning, TEXT("Freed Up Slot on Parent Building Component."));
	
	//Spawns Loot Value -> Destroy()
	Super::HandleDeletionOfBuildable();
	UE_LOG(LogTemp, Warning, TEXT("Spawned Loot and Destroyed the Actor."));
}

void ATrapBase::HandleDeletionByBuildingComponent()
{
	SpawnLootInRange(BuildableCost.CostOfParts, BuildableCost.CostOfMechanisms, BuildableCost.CostOfUniques);

	Destroy();
}

void ATrapBase::HitBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	//GEngine->AddOnScreenDebugMessage(13, 3.0f, FColor::Red, "Trap HitBox Overlap");
	if (IsValid(OtherActor) && OtherActor->IsA(ATimberEnemyCharacter::StaticClass()) && !InsideHitBoxArray.Contains(OtherActor))
	{
		AddEnemyToInsideHitBoxArray(OtherActor);
	}
}

void ATrapBase::HitBoxEndOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsValid(OtherActor) && OtherActor->IsA(ATimberEnemyCharacter::StaticClass()) && InsideHitBoxArray.Contains(OtherActor))
	{
		RemoveEnemyFromInsideHitBoxArray(OtherActor);
	}
}

void ATrapBase::AddEnemyToInsideHitBoxArray(AActor* Enemy)
{
	InsideHitBoxArray.Add(Enemy);
}

void ATrapBase::RemoveEnemyFromInsideHitBoxArray(AActor* Enemy)
{
	InsideHitBoxArray.Remove(Enemy);
}

void ATrapBase::ClearDeadEnemiesFromArray()
{
	InsideHitBoxArray.RemoveAll([](const TWeakObjectPtr<AActor>& EnemyPtr)
	{
		return !EnemyPtr.IsValid();
	});
}

void ATrapBase::ConfigureStaticMeshWalkableSlope(AActor* ParentBuildableRef)
{
	/*
	 * This is called from the BuildSystemManagerComponent when Initially Spawned.
	 * bIsUnwalk
	 */
	//Parent Buildable is a Vertical Wall
	if (Cast<ATimberVerticalBuildingComponent>(ParentBuildableRef))
	{
		GetAllStaticMeshComponents();
		
		if (StaticMeshComponents.Num() > 0)
		{
			for (UStaticMeshComponent* SomeMesh : StaticMeshComponents)
			{
				SomeMesh->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.0f));
				UE_LOG(LogTemp, Warning, TEXT("Set Walkable Slope Override on Static Mesh Component: %s"), *SomeMesh->GetName());
			}

			bIsWalkable = false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Parent Buildable is not a Vertical Wall or not valid."));
	}
}
