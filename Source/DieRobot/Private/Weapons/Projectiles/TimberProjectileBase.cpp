// Property of Paracosm Industries. Dont use my shit.


#include "Weapons/Projectiles/TimberProjectileBase.h"

#include "Character/TimberPlayableCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interfaces/DamageableEnemy.h"

ATimberProjectileBase::ATimberProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("CapsuleComponent");
	RootComponent = CapsuleComponent;
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("Projectile Movement Component");
}

void ATimberProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle DestroyProjectileTimerHandle;

	GetWorld()->GetTimerManager().SetTimer(
		DestroyProjectileTimerHandle, this,
		&ATimberProjectileBase::HandleDestroyAfterNoCollision, 5.0f, false);

	ProjectileOwner = Cast<ATimberPlayableCharacter>(GetOwner());
}

void ATimberProjectileBase::HandleDestroyAfterNoCollision()
{
	//Used to destroy the projectile after a certain amount of time if it has not collided with anything.
	Destroy();
}
