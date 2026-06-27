// Property of Paracosm Industries.


#include "Weapons/TimberWeaponBase.h"

#include "ActorReferencesUtils.h"
#include "Character/TimberPlayableCharacter.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include"Weapons/Projectiles/TimberProjectileBase.h"
#include "Components/BoxComponent.h"


// Sets default values
ATimberWeaponBase::ATimberWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;
	
	WeaponStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	WeaponStaticMesh->SetupAttachment(RootComponent);
	WeaponStaticMesh->SetCollisionProfileName("AestheticMeshOnly");
}

void ATimberWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bUsesPower)
	{
		RegeneratePower(DeltaTime, PowerRegenerationPerSecond);
	}
}

// Called when the game starts or when spawned
void ATimberWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void ATimberWeaponBase::RegeneratePower(float DeltaTime, float RegenerationRate)
{
	//Scales the Regeneration Rate by the Time between Frames (Delta Time)
	CurrentPower += DeltaTime * RegenerationRate;
	CurrentPower = FMath::Clamp(CurrentPower, 0, MaxPower);
}

void ATimberWeaponBase::ClearPowerCooldown()
{
	if (bUsesPower)
	{
		GetWorld()->GetTimerManager().ClearTimer(PowerCooldownTimerHandle);
		bIsOnPowerCooldown = false;
	}
}

void ATimberWeaponBase::ConsumePower(float AmountToConsume)
{
	CurrentPower -= AmountToConsume;

	if (CurrentPower < 1)
	{
		HandlePowerCooldown();
	}
}

void ATimberWeaponBase::HandlePowerCooldown()
{
	bIsOnPowerCooldown = true;
	GetWorld()->GetTimerManager().SetTimer(PowerCooldownTimerHandle, this, &ATimberWeaponBase::ClearPowerCooldown, PowerDepletedCooldownTime, false);
}


