// Property of Paracosm Industries.


#include "Character/Enemies/DieRobotEnemyMeleeWeaponBase.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Weapons/DieRobotWeaponBase.h"


// Sets default values
ADieRobotEnemyMeleeWeaponBase::ADieRobotEnemyMeleeWeaponBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	EnemyType = EEnemyType::MeleeWeaponRobot;
}

// Called when the game starts or when spawned
void ADieRobotEnemyMeleeWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (CharacterMovementComponent)
	{
		CharacterMovementComponent->MaxWalkSpeed = 350.0f;;
	}
	
	EquipMeleeWeapon(MeleeWeaponClassName);
}

void ADieRobotEnemyMeleeWeaponBase::EquipMeleeWeapon(TSubclassOf<ADieRobotWeaponBase> WeaponClassName)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = Cast<APawn>(this);

	if (GetMesh())
	{
		//GEngine->AddOnScreenDebugMessage(1, 5.0, FColor::Black, "Mesh Found for Melee Weapon Spawn.");

		const FTransform RHandSocketTransform = GetMesh()->GetSocketTransform("EquippedMeleeSocket");
		if (RHandSocketTransform.IsValid())
		{
			//GEngine->AddOnScreenDebugMessage(2, 5.0, FColor::Black, "Right Hand Socket Transform Found");

			AActor* WeaponActor = GetWorld()->SpawnActor<ADieRobotWeaponBase>(
				WeaponClassName,
				RHandSocketTransform.GetLocation(),
				RHandSocketTransform
				.GetRotation().Rotator(),
				SpawnParameters);

			EquippedWeapon = Cast<ADieRobotWeaponBase>(WeaponActor);
			if (EquippedWeapon)
			{
				EquippedWeapon->AttachToComponent(
					GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "EquippedMeleeSocket");
				EnemyWeaponType = EEnemyWeaponState::MeleeWeaponEquipped;
			}
		}
	}
}

// Called every frame
void ADieRobotEnemyMeleeWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
