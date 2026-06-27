// Property of Paracosm Industries. Dont use my shit.


#include "Character/Enemies/FloaterDrones.h"

#include "Character/Enemies/Boss/BossLola.h"
#include "Components/CapsuleComponent.h"
#include "Weapons/Projectiles/TimberEnemyProjectile.h"


// Sets default values
AFloaterDrones::AFloaterDrones()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AFloaterDrones::BeginPlay()
{
	Super::BeginPlay();
}

void AFloaterDrones::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFloaterDrones::BindToLolaStun(ABossLola* LolaRef)
{
	if (LolaRef)
	{
		LolaRef->OnLolaStunChange.AddDynamic(this, &AFloaterDrones::HandleLolaStunChange);
	}
}

void AFloaterDrones::HandleLolaStunChange(bool bIsLolaStunned)
{
	bIsLolaStunnedLocal = bIsLolaStunned;
}

void AFloaterDrones::TakeDamage(FDamagePayload DamagePayload)
{
	if (DroneState == EDroneState::Damageable)
	{
		CurrentHealth = CurrentHealth - DamagePayload.DamageAmount;
		if (CurrentHealth <= 0)
		{
			HandleDroneDeath();
		}
	}
	else
	{
		/*if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 3.0, FColor::Red, "Drone is not Damageable.");
		}*/
	}
}

void AFloaterDrones::HandleDroneDeath()
{

	//Broadcast to Lola to change her state to Stunned
	DroneDestroyedHandle.Broadcast(this);
	Destroy();
}

void AFloaterDrones::HandleBaseMaterialColorChange(EDroneState State)
{
	//Changing BaseMaterialColor to Some Color.
	if (State == EDroneState::Damageable)
	{
		GetMesh()->SetMaterial(0, DamageableMaterial);
		//Remove Shield Overlay Material to Signify, Damageable.
		RemoveOverlayMaterialFromCharacter();
	}
	else
	{
		//Add Shield Overlay Material to Signify, not Damageable.
		AddOverlayMaterialToCharacter(ShieldOverlayMaterial, 0.0f);
		
		GetMesh()->SetMaterial(0, NotDamageableMaterial);
	}
}

void AFloaterDrones::SetActorUnDamageable()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFloaterDrones::SetActorDamageable()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AFloaterDrones::HandleChangeDamageState(EDroneState NewState)
{
	DroneState = NewState;

	HandleBaseMaterialColorChange(DroneState);

	//Change Collision Parameters Based on current state.
}

void AFloaterDrones::FireProjectile()
{
	FActorSpawnParameters SpawnParams;\
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	if (GetWorld())
	{
		ATimberProjectileBase* Projectile = GetWorld()->SpawnActor<ATimberProjectileBase>(
		ProjectileClass,
		RaycastStartPoint->GetComponentLocation(),
		GetActorRotation(),
		SpawnParams);

		/*if (Projectile)
		{
			UE_LOG(LogTemp, Warning, TEXT("Floater Drone C++ - Projectile Fired."));
		}*/
	}
	
}


