// Property of Paracosm Industries.


#include "Character/Enemies/TimberEnemyMeleeBase.h"
#include "BuildSystem/BuildingComponents/TimberBuildingComponentBase.h"
#include "Character/TimberPlayableCharacter.h"
#include "Character/TimberSeeda.h"
#include "Components/CapsuleComponent.h"


ATimberEnemyMeleeBase::ATimberEnemyMeleeBase()
{
	RightHandCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RightHandCapsule"));
	LeftHandCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("LeftHandCapsule"));
	RightFootCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RightFootCapsule"));
	LeftFootCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("LeftFootCapsule"));

	RightHandCapsuleComponent->SetupAttachment(GetMesh(), FName(TEXT("RHandCollisionSocket")));
	LeftHandCapsuleComponent->SetupAttachment(GetMesh(), FName(TEXT("LHandCollisionSocket")));
	LeftFootCapsuleComponent->SetupAttachment(GetMesh(), FName(TEXT("LFootCollisionSocket")));
	RightFootCapsuleComponent->SetupAttachment(GetMesh(), FName(TEXT("RFootCollisionSocket")));

	//Used for Player Hits
	RightHandCapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ATimberEnemyMeleeBase::HandleCapsuleOverlap);
	LeftHandCapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ATimberEnemyMeleeBase::HandleCapsuleOverlap);
	RightFootCapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ATimberEnemyMeleeBase::HandleCapsuleOverlap);
	LeftFootCapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ATimberEnemyMeleeBase::HandleCapsuleOverlap);

	EnemyType = EEnemyType::BasicRobot;
}

void ATimberEnemyMeleeBase::BeginPlay()
{
	Super::BeginPlay();
}

void ATimberEnemyMeleeBase::EnableCapsuleComponent(UCapsuleComponent* MeleeCapsuleComponent)
{
	MeleeCapsuleComponent->SetCollisionProfileName("DR_MeleeAttackShapes");
}

void ATimberEnemyMeleeBase::DisableCapsuleComponent(UCapsuleComponent* MeleeCapsuleComponent)
{
	MeleeCapsuleComponent->SetCollisionProfileName("NoCollision");
	//Empties the array of actors to ignore after the completion of the hand and leg swing.
	ActorsToIgnore.Empty();
}

float ATimberEnemyMeleeBase::CalculateOutputDamage(float Damage)
{
	return Super::CalculateOutputDamage(Damage);
}

void ATimberEnemyMeleeBase::HandleCapsuleOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	
	//Current Target can Range from Player, Seeda, Building Component
	// We want to focus damage only on the Current Damage and avoid "Collateral Damage"
	if(ActorsToIgnore.Contains(OtherActor) || OtherActor != CurrentTarget)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Hit Actor is not the current target. Returning."));
		return;
	}
	//UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s. Current Target: %s"), *OtherActor->GetName(), *CurrentTarget->GetName());

	ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(OtherActor);
	if (PlayerCharacter && PlayerCharacter->CurrentHealth > 0)
	{
		
		PlayerCharacter->PlayerTakeDamage(BaseMeleeAttackDamage);
		ActorsToIgnore.Add(OtherActor);
	}

	ATimberSeeda* Seeda = Cast<ATimberSeeda>(OtherActor);
	if(Seeda && Seeda->CurrentHealth > 0)
	{
		Seeda->TakeDamage_Seeda(BaseMeleeAttackDamage);
		ActorsToIgnore.Add(OtherActor);
	}

	//UE_LOG(LogTemp, Warning, TEXT("Hit Overlap on Building Component. Other Actor: %s"), *OtherActor->GetName());
	ATimberBuildingComponentBase* BuildingComponent = Cast<ATimberBuildingComponentBase>(OtherActor);
	//ARampBase* Building = Cast<ARampBase>(OtherActor);
	if(BuildingComponent && BuildingComponent == CurrentTarget)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Timber Enemy Melee Base Capsule Overlap. Other Actor: %s"), *OtherActor->GetName());
		//UE_LOG(LogTemp, Warning, TEXT("Overlapped Component: %s"), *OtherComp->GetName());
		//UE_LOG(LogTemp, Warning, TEXT("Melee Robot hit a Building Component and dealt damage"));
		//TODO::Ramps dont have a take damage function
		BuildingComponent->BuildingComponentTakeDamage(BaseMeleeAttackDamage, this);
		ActorsToIgnore.Add(OtherActor);
	}
}


