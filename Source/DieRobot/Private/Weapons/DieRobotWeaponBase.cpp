// Property of Paracosm Industries.


#include "Weapons/DieRobotWeaponBase.h"

#include "ActorReferencesUtils.h"
#include "Character/DieRobotPlayableCharacter.h"
#include "Character/Enemies/DieRobotEnemyCharacter.h"
#include"Weapons/Projectiles/DieRobotProjectileBase.h"
#include "Components/BoxComponent.h"


// Sets default values
ADieRobotWeaponBase::ADieRobotWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;
	
	WeaponStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	WeaponStaticMesh->SetupAttachment(RootComponent);
	WeaponStaticMesh->SetCollisionProfileName("AestheticMeshOnly");
}

void ADieRobotWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bUsesPower)
	{
		RegeneratePower(DeltaTime, PowerRegenerationPerSecond);
	}
}

// Called when the game starts or when spawned
void ADieRobotWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void ADieRobotWeaponBase::RegeneratePower(float DeltaTime, float RegenerationRate)
{
	//Scales the Regeneration Rate by the Time between Frames (Delta Time)
	CurrentPower += DeltaTime * RegenerationRate;
	CurrentPower = FMath::Clamp(CurrentPower, 0, MaxPower);
}

void ADieRobotWeaponBase::ClearPowerCooldown()
{
	if (bUsesPower)
	{
		GetWorld()->GetTimerManager().ClearTimer(PowerCooldownTimerHandle);
		bIsOnPowerCooldown = false;
	}
}

void ADieRobotWeaponBase::ConsumePower(float AmountToConsume)
{
	CurrentPower -= AmountToConsume;

	if (CurrentPower < 1)
	{
		HandlePowerCooldown();
	}
}

void ADieRobotWeaponBase::HandlePowerCooldown()
{
	bIsOnPowerCooldown = true;
	GetWorld()->GetTimerManager().SetTimer(PowerCooldownTimerHandle, this, &ADieRobotWeaponBase::ClearPowerCooldown, PowerDepletedCooldownTime, false);
}


