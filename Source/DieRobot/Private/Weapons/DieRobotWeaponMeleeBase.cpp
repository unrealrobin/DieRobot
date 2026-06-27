// Property of Paracosm Industries.


#include "Weapons/DieRobotWeaponMeleeBase.h"

#include "BuildSystem/BuildingComponents/DieRobotBuildingComponentBase.h"
#include "Character/DieRobotCharacterBase.h"
#include "Character/DieRobotPlayableCharacter.h"
#include "Character/DieRobotSeeda.h"
#include "Character/Enemies/DieRobotEnemyCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/Combat/CombatComponent.h"
#include "Controller/DieRobotPlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Types/Combat/DamagePayload.h"

// Sets default value
ADieRobotWeaponMeleeBase::ADieRobotWeaponMeleeBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponBoxComponent = CreateDefaultSubobject<UBoxComponent>("Collision Box");
	WeaponBoxComponent->SetupAttachment(RootComponent);
	WeaponBoxComponent->SetCollisionProfileName("NoCollision");
	
	NiagaraEffectSpawnLocation = CreateDefaultSubobject<USceneComponent>("NiagaraSpawnLocation");
	NiagaraEffectSpawnLocation->SetupAttachment(RootComponent);

	WeaponSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponSkeletalMesh");
	WeaponSkeletalMesh->SetupAttachment(RootComponent);
	WeaponSkeletalMesh->SetCollisionProfileName("DR_AestheticMeshOnly");

	if (WeaponBoxComponent)
	{
		WeaponBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ADieRobotWeaponMeleeBase::OnWeaponOverlapBegin);
		//WeaponBoxComponent->OnComponentEndOverlap.AddDynamic(this, &ADieRobotWeaponMeleeBase::OnWeaponOverlapEnd);
	}
}

// Called when the game starts or when spawned
void ADieRobotWeaponMeleeBase::BeginPlay()
{
	Super::BeginPlay();

	WeaponOwner = GetOwner();
	WeaponInstigator = Cast<ADieRobotPlayableCharacter>(GetInstigator());
}

// Called every frame
void ADieRobotWeaponMeleeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//Alternates collision states, handled in Anim Notifies.
void ADieRobotWeaponMeleeBase::HandleWeaponCollision(bool ShouldReadyCollision) const
{
	if (ShouldReadyCollision)
	{
		UE_LOG(LogTemp, Warning, TEXT("DieRobotWeaponMeleeBase - HandleWeaponCollision() - Setting Collision Profile to HitEventOnly"));
		WeaponBoxComponent->SetCollisionProfileName("DR_HitEventOnly");
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DieRobotWeaponMeleeBase - HandleWeaponCollision() - Setting Collision Profile to NoCollision"));
		WeaponBoxComponent->SetCollisionProfileName("NoCollision");
	}
}

void ADieRobotWeaponMeleeBase::OnWeaponOverlapBegin(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	ADieRobotPlayableCharacter* PlayerCharacter = Cast<ADieRobotPlayableCharacter>(GetOwner());
	ADieRobotEnemyCharacter* EnemyCharacter = Cast<ADieRobotEnemyCharacter>(GetOwner());

	if (IsValid(EnemyCharacter))
	{
		OnAiWeaponOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	}
	else if (IsValid(PlayerCharacter))
	{
		OnPlayerWeaponOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	}
}

void ADieRobotWeaponMeleeBase::OnWeaponOverlapEnd(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//We only run this for the player character, because we empty the actor array for AI enemies in the Animation Blueprint.
	if (Cast<ADieRobotPlayableCharacter>(GetOwner()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player Character Emptying Actor to Ignore Array"));
		EmptyActorToIgnoreArray();
	}
}

//Runs Player Sword Overlap Logic
void ADieRobotWeaponMeleeBase::OnPlayerWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent*
		OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Player Weapon Overlapped with Actor: %s"), *OtherActor->GetName());
	ADieRobotEnemyCharacter* HitEnemy = Cast<ADieRobotEnemyCharacter>(OtherActor);
	if (HitEnemy)
	{
		if (ActorsToIgnore.Contains(HitEnemy))
		{
			return;
		}
		ActorsToIgnore.Add(HitEnemy);
		HitEnemy->PlayMeleeWeaponHitSound(SweepResult);

		FDamagePayload Payload;
		Payload.DamageAmount = BaseWeaponDamage;
		Payload.DamageInstigator = GetOwner();
		Payload.DamageOwner = this;
		HitEnemy->TakeDamage(Payload);
	}
}

void ADieRobotWeaponMeleeBase::OnAiWeaponOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Handling AI Sword Hit Response."));
	if (ActorsToIgnore.Contains(OtherActor) ||
		OtherActor != Cast<ADieRobotEnemyCharacter>(GetOwner())->CurrentTarget ||
		OtherActor == GetOwner() || OtherActor == this)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Ignoring Actor: %s"), *OtherActor->GetName());
		return;
	}
	
	ActorsToIgnore.Add(OtherActor);
	UE_LOG(LogTemp, Warning, TEXT("Added Actor to Ignore Array: %s"), *OtherActor->GetName());
	
	ADieRobotBuildingComponentBase* HitBuildingComponent = Cast<ADieRobotBuildingComponentBase>(OtherActor);
	ADieRobotPlayableCharacter* HitCharacter = Cast<ADieRobotPlayableCharacter>(OtherActor);
	ADieRobotSeeda* Seeda = Cast<ADieRobotSeeda>(OtherActor);
	
	if (HitCharacter) //Is the player character and doesn't need to be ignored.
	{
		UE_LOG(LogTemp, Warning, TEXT("Applying Damage to Player. Amount = %f"), TotalWeaponDamage);
		HitCharacter->PlayerTakeDamage(TotalWeaponDamage);
		return;
	}
	if(Seeda)
	{
		UE_LOG(LogTemp, Warning, TEXT("Applying Damage to Seeda. Amount = %f"), TotalWeaponDamage);
		Seeda->TakeDamage_Seeda(TotalWeaponDamage);
		return;
	}
	if (HitBuildingComponent && HitBuildingComponent->BuildingComponentType != EBuildingComponentType::Environment)
	{
		HitBuildingComponent->BuildingComponentTakeDamage(BaseWeaponDamage, GetOwner());
	}
}

void ADieRobotWeaponMeleeBase::EmptyActorToIgnoreArray()
{
	ActorsToIgnore.Empty();
}




