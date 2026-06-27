// Property of Paracosm.


#include "Weapons/Abilities/MeleeWeapon/ChargedSwing.h"

#include "Character/TimberPlayableCharacter.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/SphereComponent.h"
#include "Types/Combat/DamagePayload.h"


UChargedSwing::UChargedSwing()
{
	ABILITY_NAME = "ChargedSwing";
	ValidationType = EAbilityValidation::RequiresPower;
	WeaponAbilityType = EForWeaponType::MeleeWeapon;
	bNeedsProjectileData = false;
	PowerCost = 30.0f;
}

void UChargedSwing::Execute(FAbilityContext Context)
{
	
	ChargeTimer = GetWorld()->GetTimeSeconds();

	AbilityContext = Context;
	
	Context.CombatComponent->bCanMeleeAttack = false;

	UE_LOG(LogTemp, Warning, TEXT("Charged Swing Started."));

	Context.CombatComponent->PlayCharacterAnimationMontage(ChargedSwingMontage, "WindUp", 1, true);
	CurrentMontageStage = EMontageStage::WindUp;
}

void UChargedSwing::Execute_Completed(FAbilityContext Context)
{
	Context.WeaponInstance->ConsumePower(PowerCost);
	
	UE_LOG(LogTemp, Warning, TEXT("Charged Swing Attack Started."));
	Context.CombatComponent->PlayCharacterAnimationMontage(ChargedSwingMontage, "ChargedAttack", 1.0f);
	CurrentMontageStage = EMontageStage::Final;

	/* Cleanup Removed from here because it Cleans up Ability Reference before execution of Collision Creation*/
	//HandleCleanup(Context);
}

void UChargedSwing::Execute_Cancelled(FAbilityContext Context)
{
	float CancelTime = GetWorld()->GetTimeSeconds();
	ChargeTimer = CancelTime - ChargeTimer;
	if (ChargeTimer > FullChargeTimer)
	{
		Execute_Completed(Context);
	}
	else
	{
		HandleIncompleteAbility(Context);
	}
}

void UChargedSwing::HandleIncompleteAbility(FAbilityContext Context)
{
	UE_LOG(LogTemp, Warning, TEXT("Player Released Before Full Charge Met."));
	//Stop Any Anim Montage
	if (Context.CombatComponent)
	{
		Context.CombatComponent->StopCharacterAnimationMontage(ChargedSwingMontage, .33f);
		//Cannot Primary Ability Melee during animation
		Context.CombatComponent->bCanMeleeAttack = true;
	}
	HandleCleanup(Context);
}

void UChargedSwing::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	//Interrupted only true if the animation was interrupted.If finished properly, it will be false.
	//We only want to force a new montage if the previous montage was finished.
	//if (bInterrupted) return;

	if (CurrentMontageStage == EMontageStage::Final && !bInterrupted)
	{
		//Cannot Primary Ability Melee during animation
		AbilityContext.CombatComponent->bCanMeleeAttack = true;
		UE_LOG(LogTemp, Warning, TEXT("Charged Swing Completed."));
		HandleCleanup(AbilityContext);
	}
}

void UChargedSwing::HandleCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ATimberEnemyCharacter* EnemyCharacter = Cast<ATimberEnemyCharacter>(OtherActor);
	UE_LOG(LogTemp, Warning, TEXT("Heavy Attack Collision Sphere Overlap Triggered."));
	
	if (ActorsToIgnore.Contains(OtherActor))
	{
		return;
	}
	
	if (EnemyCharacter && AbilityContext.Instigator)
	{
		ActorsToIgnore.Add(EnemyCharacter);

		FDamagePayload Payload;
		Payload.DamageAmount = PerEnemyHitDamage;
		Payload.DamageInstigator = AbilityContext.Instigator;
		EnemyCharacter->TakeDamage(Payload);
	}
}

void UChargedSwing::HandleDamage(FAbilityContext Context)
{
	AActor* Weapon = Context.WeaponInstance;
	//UE_LOG(LogTemp, Warning, TEXT("Weapon : %s"), *Weapon->GetName());
	
	if (Weapon)
	{
		USphereComponent* CollisionSphere = NewObject<USphereComponent>(this);
		if (CollisionSphere)
		{
			CollisionSphere->SetupAttachment(Weapon->GetRootComponent());
			CollisionSphere->RegisterComponent();
			CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &UChargedSwing::HandleCollisionSphereBeginOverlap);
			CollisionSphere->SetSphereRadius(CollisionSphereRadius);
			CollisionSphere->SetCollisionProfileName("DR_MeleeAttackShapes");
			CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			CollisionSphere->SetCanEverAffectNavigation(false);
			
			//UE_LOG(LogTemp, Warning, TEXT("Heavy Melee Attack Collision Sphere Created."));
			
			//DrawDebugSphere(GetWorld(), Weapon->GetActorLocation(), CollisionSphereRadius, 12, FColor::Red, false, 10.0f);

			//Handles Destruction of the Collision Sphere and Emptys the Abilities ActorsToIgnore Array. (Clean up)
			FTimerHandle DestroyCollisionSphereTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(DestroyCollisionSphereTimerHandle, [this, CollisionSphere, Context]()
			{
					if (CollisionSphere)
					{
						CollisionSphere->DestroyComponent();
						ActorsToIgnore.Empty();

						/* After Successful Creation of Damage Collision Sphere, Clean up Ability on Combat Actor Component.*/
						HandleCleanup(Context);
					}
			}, .3f, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon Ability - HandleDamage() - Weapon is Null."));
	}
}
