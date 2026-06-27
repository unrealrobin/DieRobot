// Property of Paracosm.


#include "Weapons/Abilities/RangedWeapon/KnockbackBlast.h"

#include "AI/TimberAiControllerBase.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Types/Combat/DamagePayload.h"
#include "UObject/FastReferenceCollector.h"
#include "Weapons/TimberWeaponRangedBase.h"

UKnockbackBlast::UKnockbackBlast()
{
	ABILITY_NAME = "KnockbackBlast";
	ValidationType = EAbilityValidation::RequiresPower;
	WeaponAbilityType = EForWeaponType::RangedWeapon;
	bNeedsProjectileData = false;
}

void UKnockbackBlast::Execute(FAbilityContext Context)
{
	//Spawn a UBoxComponent that starts at the muzzle of the Weapon.
	ATimberWeaponRangedBase* OwningWeapon = Cast<ATimberWeaponRangedBase>(Context.WeaponInstance);
	if (OwningWeapon && OwningWeapon->ProjectileSpawnComponent)
	{
		OwningWeapon->ConsumePower(PowerCost);
		LocalContext = Context;
		
		HitBox = NewObject<UBoxComponent>(OwningWeapon, UBoxComponent::StaticClass());

		FRotator Rotation = OwningWeapon->ProjectileSpawnComponent->GetComponentRotation();
		Rotation.Roll = 0.0f;
		FVector Location = OwningWeapon->ProjectileSpawnComponent->GetComponentLocation();
		
		HitBox->SetWorldRotation(Rotation);
		HitBox->SetWorldLocation(Location);
		HitBox->SetBoxExtent(InitialBoxExtent, true);

		HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		
		HitBox->RegisterComponent();

		//HitBox->OnComponentHit.AddDynamic(this, &UKnockbackBlast::HandleHit);
		HitBox->OnComponentBeginOverlap.AddDynamic(this, &UKnockbackBlast::HandleOverlap);
		InitialBoxLocation = HitBox->GetComponentLocation();
	
		//UE_LOG(LogTemp, Warning, TEXT("PSC: Pitch: %f Yaw: %f Roll: %f"), Rotation.Pitch, Rotation.Yaw, Rotation.Roll);
		GetWorld()->GetTimerManager().SetTimer(BlastTimerHandle, this, &UKnockbackBlast::IncrementBlastBox, 0.05f, true);
		
		PlayNiagaraEffectAttached(NiagaraEffect, HitBox, NAME_None, EffectVectorOffset, FRotator::ZeroRotator, EAttachLocation::SnapToTargetIncludingScale, false, true);
		
		PlayEffectSFX(HitBox->GetComponentLocation(), SoundFXTrackName);

		HandleCleanup(Context);
	}
}

void UKnockbackBlast::MoveBoxForward()
{
	float Delta = CurrentTimeStep / EndTimeStep;
	
	float ForwardOffset = FMath::Lerp(0.0f, EndLocationDelta, Delta );
	
	FVector AdjustedLocation = InitialBoxLocation + (HitBox->GetForwardVector() * ForwardOffset);

	FHitResult* Hit = nullptr;;
	HitBox->SetRelativeLocation(AdjustedLocation, true, Hit );
}

void UKnockbackBlast::ScaleBoxExtent()
{
	if (HitBox)
	{
		float Delta = CurrentTimeStep / EndTimeStep;
		FVector AdjustedVector = FMath::Lerp(InitialBoxExtent, EndBoxExtent, Delta);
		HitBox->SetBoxExtent(AdjustedVector, true);
	}
}

void UKnockbackBlast::HandleDestructionCheck()
{
	CurrentTimeStep++;
	
	if (CurrentTimeStep > EndTimeStep)
	{
		GetWorld()->GetTimerManager().ClearTimer(BlastTimerHandle);

		HitBox->DestroyComponent();
	}
}

void UKnockbackBlast::IncrementBlastBox()
{
	MoveBoxForward();
	ScaleBoxExtent();
	HandleDestructionCheck();
}

void UKnockbackBlast::HandleOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && !ActorsToIgnore.Contains(OtherActor))
	{
		ActorsToIgnore.Add(OtherActor);
		UE_LOG(LogTemp, Warning, TEXT("HitActor = %s"), *OtherActor->GetName());
		if (ATimberEnemyCharacter* Enemy = Cast<ATimberEnemyCharacter>(OtherActor))
		{
			//Getting a Straight Line from Actor to Player & Flattening the Vector. (Straight Back Push Away from Player)
			FVector ImpulseDirection = OtherActor->GetActorLocation() - LocalContext.Instigator->GetActorLocation();
			ImpulseDirection.Z = 0.0f;
			ImpulseDirection = ImpulseDirection.GetSafeNormal();

			//Adding a little verticality
			ImpulseDirection.Z += ImpulseForceZ;
			ImpulseDirection = ImpulseDirection.GetSafeNormal();

			if (Enemy->GetController())
			{
				if (ATimberAiControllerBase* AiController = Cast<ATimberAiControllerBase>(Enemy->GetController()) )
				{
					if (AiController->GetBrainComponent())
					{
						AiController->GetBrainComponent()->StopLogic("Enemy Knocked back");
						
						FTimerHandle StunTimerHandle;
						GetWorld()->GetTimerManager().SetTimer(StunTimerHandle, [this, AiController]()
						{
							AiController->GetBrainComponent()->RestartLogic();

							//TODO:: We can add like a stun Animation Here to the enemy and set a var on the Controller to Stunned.
						}, EnemyStunTime, false);
					}

				}
			}
			Enemy->GetCharacterMovement()->AddImpulse(ImpulseDirection * ImpulseForce, true);

			FDamagePayload Payload;
			Payload.DamageAmount = DamageAmount;
			Payload.DamageInstigator = LocalContext.Instigator;
			Enemy->TakeDamage(Payload);
		
		}
	}
}
