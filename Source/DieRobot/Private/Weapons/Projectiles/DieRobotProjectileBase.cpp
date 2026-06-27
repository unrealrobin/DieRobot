// Property of Paracosm Industries. Dont use my shit.


#include "Weapons/Projectiles/DieRobotProjectileBase.h"

#include "Character/DieRobotPlayableCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interfaces/DamageableEnemy.h"

ADieRobotProjectileBase::ADieRobotProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("CapsuleComponent");
	RootComponent = CapsuleComponent;
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("Projectile Movement Component");
}

void ADieRobotProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle DestroyProjectileTimerHandle;

	GetWorld()->GetTimerManager().SetTimer(
		DestroyProjectileTimerHandle, this,
		&ADieRobotProjectileBase::HandleDestroyAfterNoCollision, 5.0f, false);

	ProjectileOwner = Cast<ADieRobotPlayableCharacter>(GetOwner());
}

void ADieRobotProjectileBase::HandleDestroyAfterNoCollision()
{
	//Used to destroy the projectile after a certain amount of time if it has not collided with anything.
	Destroy();
}
