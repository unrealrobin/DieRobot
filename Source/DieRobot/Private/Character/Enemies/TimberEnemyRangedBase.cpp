// Property of Paracosm Industries. 

#include "Character/Enemies/TimberEnemyRangedBase.h"

#include "Engine/ContentEncryptionConfig.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapons/TimberWeaponBase.h"
#include "Weapons/TimberWeaponRangedBase.h"

// Sets default values
ATimberEnemyRangedBase::ATimberEnemyRangedBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	EnemyType = EEnemyType::RangedWeaponRobot;

	AimStartPointComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AimStartPointComponent"));
	AimStartPointComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ATimberEnemyRangedBase::BeginPlay()
{
	Super::BeginPlay();
	EquipRangedWeapon();
	if (CharacterMovementComponent)
	{
		CharacterMovementComponent->MaxWalkSpeed = 375.0f;
	}
}

void ATimberEnemyRangedBase::EquipRangedWeapon()
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = GetInstigator();

	//Getting the Socket Info from the Mesh
	const FTransform RHandSocketTransform = GetMesh()->GetSocketTransform(RangedSocketEquippedName);

	if(RangedWeaponClassName != nullptr)
	{
		//Spawning the Class at the Socket Info
		ATimberWeaponBase* RangedWeaponInstance = GetWorld()->SpawnActor<ATimberWeaponBase>(
			RangedWeaponClassName,
			RHandSocketTransform.GetLocation(),
			RHandSocketTransform
			.GetRotation().Rotator(),
			SpawnParameters);

		//Casting the Instance to the Ranged Weapon Class, this will allow spawning of any weapon but only attaching of Ranged Weapon. Maybe change later idk.
		if (RangedWeaponInstance)
		{
			EquippedWeapon = Cast<ATimberWeaponRangedBase>(RangedWeaponInstance);
			if (EquippedWeapon)
			{
				//Attaching the Weapon to the Socket, this locks the weapon to the socket during movement.
				EquippedWeapon->AttachToComponent(
					GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RangedSocketEquippedName);
				EnemyWeaponType = EEnemyWeaponState::RangedWeaponEquipped;
			}
		}
	}
}

void ATimberEnemyRangedBase::GetRotationToCurrentTarget()
{
	if (CurrentTarget && EquippedWeapon)
	{
		FVector TargetLocation = CurrentTarget->GetActorLocation();
		FVector StartLocation = AimStartPointComponent->GetComponentLocation();

		FRotator WorldRotationToTarget = UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetLocation);
		FRotator ActorRotation = GetActorRotation();

		FRotator AdjustedRotation = WorldRotationToTarget - ActorRotation;;
		AdjustedRotation.Normalize();
		//PitchToTarget = FMath::Clamp(RotationToTarget.Pitch, -45.0f, 45.0f);
		//YawToTarget = FMath::Clamp(RotationToTarget.Yaw, 79.0f, -79.0f);
		YawToTarget = AdjustedRotation.Yaw;
		PitchToTarget = AdjustedRotation.Pitch;

		//UE_LOG(LogTemp, Warning, TEXT("Pitch To Target = %f"), PitchToTarget);
		//UE_LOG(LogTemp, Warning, TEXT("Yaw To Target = %f"), YawToTarget);

		/*DrawDebugLine(GetWorld(), StartLocation, TargetLocation, FColor::Red, false, 0.1f);
		DrawDebugSphere(GetWorld(), TargetLocation, 20, 30, FColor::Red);*/
		//UE_LOG(LogTemp, Warning, TEXT("Ranged Enemy Current Target: %s"), *CurrentTarget->GetName())
	}
}

// Called every frame
void ATimberEnemyRangedBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}
