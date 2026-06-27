// Property of Paracosm Industries.


#include "Weapons/DieRobotWeaponRangedBase.h"
#include "Character/DieRobotCharacterBase.h"
#include "Controller/DieRobotPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Weapons/Projectiles/DieRobotPlayerProjectile.h"
#include "Weapons/Projectiles/DieRobotProjectileBase.h"

// Sets default values
ADieRobotWeaponRangedBase::ADieRobotWeaponRangedBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProjectileSpawnComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawnComponent"));
	ProjectileSpawnComponent->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void ADieRobotWeaponRangedBase::BeginPlay()
{
	Super::BeginPlay();

	//Set in Spawn Params
	WeaponOwner = GetOwner();

	//Setting Ammo
	CurrentAmmo = MaxAmmo;
}

void ADieRobotWeaponRangedBase::HandleFiringRate(float InTime)
{
	//Called from the Weapons Ability to Initiate Cooldown and Pass Data Up the chain to the combat component.
	bIsFireOnCooldown = true;
	GetWorld()->GetTimerManager().SetTimer(TimeBetweenShotsHandle, this, &ADieRobotWeaponRangedBase::ResetFiringCooldown, InTime, false);
}

// Called every frame
void ADieRobotWeaponRangedBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADieRobotWeaponRangedBase::FireRangedWeapon(FVector TargetLocation)
{
	UE_LOG(LogTemp, Warning, TEXT("In Fire Ranged Weapon"));
	if (bIsFireOnCooldown == false && CurrentAmmo > 0 && bIsReloading == false)
	{
		if (WeaponOwner && GetWorld())
		{
			if (ProjectileType)
			{
				FVector ProjectileSpawnLocation = ProjectileSpawnComponent->GetComponentLocation();
				FRotator ProjectileAimRotation = (TargetLocation - ProjectileSpawnLocation).Rotation();
				
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = WeaponOwner;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
				
				FTransform ProjectileSpawnTransform = FTransform(ProjectileAimRotation, ProjectileSpawnLocation);
				ADieRobotPlayerProjectile* Projectile = GetWorld()->SpawnActorDeferred<ADieRobotPlayerProjectile>(
					ProjectileType,
					ProjectileSpawnTransform, this);
				Projectile->PlayerProjectileOwner = Cast<ADieRobotPlayableCharacter>(WeaponOwner);
				Projectile->FinishSpawning(ProjectileSpawnTransform);

				if (Projectile)
				{
					//Setting Timer to next shot
					GetWorld()->GetTimerManager().SetTimer(TimeBetweenShotsHandle, this, 
					&ADieRobotWeaponRangedBase::ResetFiringCooldown, TimeBetweenProjectiles, false);
					//Setting Firing to on Cooldown
					bIsFireOnCooldown = true;
					//Playing firing sounds
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), FiringSound, ProjectileSpawnLocation);
					Projectile->SetOwner(this);
					//Removing Ammo from Available Ammo
			
					if (!bUsesPower)
					{
						CurrentAmmo--;
					}
					else
					{
						if (CurrentPower <= 1)
						{
							bIsPowerWeaponCooldown = true;
							//GetWorld()->GetTimerManager().SetTimer(PowerDepletedHandle, this, &ADieRobotWeaponRangedBase::ClearPowerCooldown, PowerDepletedCooldownTime, false);
						}
					}

					

					//If a Weapon uses Power, Power usage is handled in CombatComponent.
				}
			}
		}
	}

	//TODO:: Can we remove this cast? Reload is on the Players Combat Component
	//TODO:: Tentatively Removing ammo and using Power as projectile Control.
	if (!bUsesPower && CurrentAmmo == 0)
	{
		ADieRobotPlayerController* PlayerController = Cast<ADieRobotPlayerController>(Cast<ADieRobotPlayableCharacter>(GetOwner())->GetController());
		if (PlayerController)
		{
			//Automatic Reload at 0 Ammo.
			PlayerController->ReloadWeapon(true);
		}
	}
}

void ADieRobotWeaponRangedBase::AI_FireRangedWeapon()
{
	if (bIsFireOnCooldown == false)
	{
		if (WeaponOwner && GetWorld())
		{
			UE_LOG(LogTemp, Warning, TEXT("Enemy Ranged Weapon Owner: %s"), *WeaponOwner->GetName());
			if (ProjectileType)
			{
				//GEngine->AddOnScreenDebugMessage(1, 5.0, FColor::Green, "Projectile All Variables Loaded");
				FVector ProjectileSpawnLocation = ProjectileSpawnComponent->GetComponentLocation();

				//Get Enemy Player Character
				FRotator ControllerDirection = Cast<ADieRobotCharacterBase>(WeaponOwner)->GetController()->GetControlRotation();
				FRotator RandomAimOffset = FRotator(
					FMath::RandRange(-1 * AIWeaponAccuracy, AIWeaponAccuracy), FMath::RandRange(-1 * AIWeaponAccuracy, AIWeaponAccuracy), FMath::RandRange(-1 * AIWeaponAccuracy, AIWeaponAccuracy));
				FTransform DeferredSpawnTransform = FTransform(ControllerDirection + RandomAimOffset, ProjectileSpawnLocation);

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				
				ADieRobotProjectileBase* EnemyProjectile = GetWorld()->SpawnActorDeferred<ADieRobotProjectileBase>(ProjectileType, DeferredSpawnTransform, this);
				
				UGameplayStatics::FinishSpawningActor(EnemyProjectile, DeferredSpawnTransform);
				
				if (EnemyProjectile)
				{
					//UE_LOG(LogTemp, Warning, TEXT("Weapon Owner for Enemy Ranged Weapon: %s"), *this->GetOwner()->GetName());
					//UE_LOG(LogTemp, Warning, TEXT("Projectile Owner for Enemy Ranged Weapon: %s"), *EnemyProjectile->GetOwner()->GetName());
					//Projectile->SetOwner(this);
					//Cooldown for AI Automatic Fire.
					GetWorld()->GetTimerManager().SetTimer(TimeBetweenShotsHandle, this, &ADieRobotWeaponRangedBase::ResetFiringCooldown, TimeBetweenProjectiles, false);
					bIsFireOnCooldown = true;
					
					UGameplayStatics::PlaySoundAtLocation(this, FiringSound, ProjectileSpawnLocation);
					
					
				}
			}
		}
	}
}

void ADieRobotWeaponRangedBase::PlayReloadMontage()
{
	ADieRobotPlayableCharacter* PlayerCharacter = Cast<ADieRobotPlayableCharacter>(WeaponOwner);
	if (PlayerCharacter && PlayerCharacter->ReloadMontage)
	{
		
		//Getting the PLayers Anim Instance so we can bind Delegates
		UAnimInstance* PlayerAnimInstance = PlayerCharacter->GetMesh()->GetAnimInstance();
		if (!PlayerAnimInstance) return;

		//Creating Delegate Call for Early Exit of Reload Montage
		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindUObject(this, &ADieRobotWeaponRangedBase::HandleReloadMontageInterruption);

		//Setting Delegate on this Exact Montage
		PlayerAnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, PlayerCharacter->ReloadMontage);
		
		//Plays the montage. Notify on Montage end handles the Reload.
		PlayerCharacter->PlayAnimMontage(PlayerCharacter->ReloadMontage, 1.f, FName("Reload1"));

		//Setting the Reload Bool to True - If Animation Notify Runs, this will be reset.
		bIsReloading = true;
	}
}

void ADieRobotWeaponRangedBase::HandleReloadMontageInterruption(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage) return;
	ADieRobotPlayableCharacter* PlayerCharacter = Cast<ADieRobotPlayableCharacter>(WeaponOwner);
	UAnimInstance* PlayerAnimInstance = PlayerCharacter->GetMesh()->GetAnimInstance();
	if (PlayerAnimInstance)
	{
		//This is us basically unbinding the delegates from the Montage.
		//We just set it to an empty delegate.
		FOnMontageEnded EmptyDelegate;
		PlayerAnimInstance->Montage_SetEndDelegate(EmptyDelegate, PlayerCharacter->ReloadMontage);
	};

	if (bInterrupted)
	{
		//Even if the Montage Plays past the Notify and gets reset there, and a Montage Interrupt is called, it's ok as long as the Reload Bool is reset to false.
		bIsReloading = false;
	}
	
}

void ADieRobotWeaponRangedBase::ReloadWeapon()
{
	//Called from Anim Notify on the Anim Montage for the Reload Animation.
	CurrentAmmo = MaxAmmo;
	bIsReloading = false;
}

void ADieRobotWeaponRangedBase::ResetFiringCooldown()
{
	GetWorld()->GetTimerManager().ClearTimer(TimeBetweenShotsHandle);
	bIsFireOnCooldown = false;
}



