// Property of Paracosm.


#include "Character/Enemies/EnemyDrone.h"

#include "StructDeserializer.h"
#include "Weapons/Projectiles/TimberEnemyProjectile.h"
#include "BuildSystem/BuildingComponents/TimberBuildingComponentBase.h"
#include "BuildSystem/buildingComponents/TimberVerticalBuildingComponent.h"
#include "Components/CapsuleComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Components/SplineComponent.h"
#include "Environment/EnemyDroneSplinePath.h"
#include "Environment/LabDoorBase.h"
#include "Kismet/GameplayStatics.h"
#include "Types/Combat/DamagePayload.h"
#include "Weapons/Projectiles/DroneProjectile.h"


// Sets default values
AEnemyDrone::AEnemyDrone()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	DroneParticleSystem = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DroneParticleSystem"));
	DroneParticleSystem->SetupAttachment(RootComponent);
	DroneParticleSystem->SetAutoActivate(false);
	ProjectileSpawnSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawnLocation"));
	ProjectileSpawnSceneComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AEnemyDrone::BeginPlay()
{
	Super::BeginPlay();

	//Stores Lab Door Exit Location locally.
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AEnemyDrone::GetClosestLabDoorExitLocation);
	//GetClosestLabDoorExitLocation();
	
	GetAllDroneSplinePathActors();
	
	SelectRandomSplinePath();

	FTimerHandle ShootTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(ShootTimerHandle, this, &AEnemyDrone::ShootTarget, 3.0f, true);
	
}

void AEnemyDrone::SelectRandomSplinePath()
{
	if (SplinePathActors.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, SplinePathActors.Num() - 1);
		SplinePathRef = Cast<AEnemyDroneSplinePath>(SplinePathActors[RandomIndex]);
		if (SplinePathRef)
		{
			SplineComponent = SplinePathRef->SplineComponent;
			if (SplineComponent)
			{
				
				bIsApproachSpline = true;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No spline paths found!"));
	}
}

void AEnemyDrone::GetAllDroneSplinePathActors()
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyDroneSplinePath::StaticClass(), SplinePathActors);
	if (SplinePathActors.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No spline path actors found in the level!"));
	}
}

void AEnemyDrone::MoveAlongSplinePath(float DeltaTime)
{
	if (SplineComponent)
	{
		//Getting the length of the spline
		const float SplineLength = SplineComponent->GetSplineLength();
		//Setting the Would be Distance along the spline scaled with Delta Time (1 frame)
		CurrentDistanceAlongSplinePath += DroneFlightSpeed * DeltaTime;

		if (CurrentDistanceAlongSplinePath > SplineLength)
		{
			//Resetting the current distance to 0 (completed a full loop)
			CurrentDistanceAlongSplinePath = 0.0f;
		}

		//Using the distance we are along the spline to get the new Move Target.
		FVector NewMoveTarget = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentDistanceAlongSplinePath, ESplineCoordinateSpace::World);
		FRotator NewRotation = SplineComponent->GetRotationAtDistanceAlongSpline(CurrentDistanceAlongSplinePath, ESplineCoordinateSpace::World);

		SetActorLocationAndRotation(NewMoveTarget, NewRotation);
	}
}

void AEnemyDrone::FlyToSplineStart(float DeltaTime)
{
	//Initialized to 0 (Can be any distance along the spline)
	FVector SplineStartLocation  = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentDistanceAlongSplinePath, ESplineCoordinateSpace::World);
	FVector DroneLocation = GetActorLocation();
	//Getting a vector that points from drone to spline start location
	FVector MoveDirection = (SplineStartLocation - DroneLocation).GetSafeNormal();
	//Getting a point scaled by the speed of the drone along the Move Direction
	FVector NewLocation = DroneLocation + MoveDirection * DroneApproachSpeed * DeltaTime;

	SetActorLocation(NewLocation);
	SetActorRotation(MoveDirection.Rotation());

	//Checking if within range of the spline start location
	if (FVector::Dist(NewLocation, SplineStartLocation) < 10.0f)
	{
		//When within range of spline start, move to standard spline flight trajectory.
		bIsApproachSpline = false;
		bShouldMoveAlongSpline = true;
	}
}

void AEnemyDrone::ClearCurrentTarget()
{
	if (CurrentTarget)
	{
		CurrentTarget = nullptr;
		//UE_LOG(LogTemp, Warning, TEXT("No Current Target"));
	}
}

void AEnemyDrone::ShootTarget()
{
	//Checking bIsDead because there is a time in between being destroyed and loosing all life.
	if (CurrentTarget && !bIsDead)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		FVector Location = ProjectileSpawnSceneComponent->GetComponentLocation();

		//Adding some precision offset to the projectile spawn rotation.
		FRotator Rotation = (CurrentTarget->GetActorLocation() - Location).Rotation();
		float OffsetPercent = FMath::RandRange(-1,1);
		float OffsetAmount = 2.0f;
		Rotation.Yaw += OffsetAmount * OffsetPercent;
		Rotation.Pitch += OffsetAmount * OffsetPercent;
		Rotation.Roll += OffsetAmount * OffsetPercent;

		FTransform SpawnTransform = FTransform(Rotation, Location);
		
		ADroneProjectile* SpawnedProjectile = GetWorld()->SpawnActorDeferred<ADroneProjectile>(ProjectileClass, SpawnTransform, this);

		UGameplayStatics::FinishSpawningActor(SpawnedProjectile, SpawnTransform);
		
		if (SpawnedProjectile)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Drone Projectile Spawned."));
		}
	}
	
}

// Called every frame
void AEnemyDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsOutsideSpawnArea && !bIsDead)
	{
		MoveToOutsideSpawnArea(DeltaTime);
	}
	else if (SplineComponent && bIsOutsideSpawnArea && !bIsDead)
	{
		if (bIsApproachSpline)
		{
			FlyToSplineStart(DeltaTime);
		}
		else if (bShouldMoveAlongSpline)
		{
			MoveAlongSplinePath(DeltaTime);
		}
	}
}

void AEnemyDrone::GetClosestLabDoorExitLocation()
{
	TArray<AActor*> LabDoorExitActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALabDoorBase::StaticClass(), LabDoorExitActors);

	/*if (LabDoorExitActors.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT(" Found Lab Doors."));
	}*/
	ALabDoorBase* ClosestLabDoor = nullptr;
	
	float DistToClosestLabDoor = UE_MAX_FLT;
	
	for (AActor* LabDoor : LabDoorExitActors)
	{
		if (ALabDoorBase* LabDoorBase = Cast<ALabDoorBase>(LabDoor))
		{
			//DrawDebugSphere(GetWorld(), LabDoor->GetActorLocation(), 50.f, 12, FColor::Green, false, 10.0f);

			float DistToLabDoor = FVector::Dist(GetActorLocation(), LabDoorBase->GetActorLocation());
			
			if (DistToLabDoor < DistToClosestLabDoor)
			{
				//Resetting the value of closest Lab Door to check against next lab doors.
				DistToClosestLabDoor = DistToLabDoor;
				ClosestLabDoor = LabDoorBase;
			}
		}
	}
	
	if (ClosestLabDoor)
	{
		FVector ExitLocation = ClosestLabDoor->GetActorLocation();
		ExitLocation.Z += 200.0f;
		ExitLocation.Y -= 100.0f;
		ClosestLabDoorExitLocation = ExitLocation;
		//DrawDebugSphere(GetWorld(), ClosestLabDoorExitLocation, 50.f, 12, FColor::Blue, false, 5.0f);
	}
}

void AEnemyDrone::TakeDamage(const FDamagePayload DamagePayload)
{
	Super::TakeDamage(DamagePayload);

	if (CurrentHealth <= 0)
	{
		bIsDead = true;
		if (DroneParticleSystem)
		{
			DroneParticleSystem->Activate();
		}
		// Stop the drone's movement when it takes fatal damage
		GetCapsuleComponent()->SetEnableGravity(true);
		GetMesh()->SetEnableGravity(true);
		GetCapsuleComponent()->SetSimulatePhysics(true);

		FTimerHandle DestroyTimerHandle;

		GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AEnemyDrone::DestroyAfterDelay, 5.0f, false);
		
	}
	
}

void AEnemyDrone::DestroyAfterDelay()
{
	//TODO:: If we do end up using a death montage, then we need to rethink this and the TakeDamage override.
	//UE_LOG(LogTemp, Warning, TEXT("Drone Destroyed after Delay."));
	Destroy();
}

void AEnemyDrone::GatherTargets()
{
	TargetActors.Empty();
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATimberBuildingComponentBase::StaticClass(), TargetActors);

	//Remove all loops through the TargetActors array to remove any Environment Building Components, ensure to do it safely without skipping
	TargetActors.RemoveAll([](AActor* Actor)
	{
		ATimberBuildingComponentBase* BuildingComponent = Cast<ATimberBuildingComponentBase>(Actor);
		if (BuildingComponent && BuildingComponent->BuildingComponentType == EBuildingComponentType::Environment)
		{
			return true; // Remove Environment Building Components
		}
		return false;
	});

	int Attempts = 0;
	int MaxAllowableAttempts = 10;

	if (TargetActors.Num() > 0)
	{
		while (!CurrentTarget && Attempts < MaxAllowableAttempts)
		{
			
			//Choose a Random Target
			int32 RandomIndex = FMath::RandRange(0, TargetActors.Num() - 1);
			AActor* PotentialTarget = TargetActors[RandomIndex];

			ATimberBuildingComponentBase* BuildingComponent = Cast<ATimberVerticalBuildingComponent>(PotentialTarget);
			if (BuildingComponent && BuildingComponent->BuildingComponentType == EBuildingComponentType::Environment)
			{
				//Do not want to target Environment Building Components
				continue;
			}
			
			Attempts++;
			
			FHitResult Hit;
			bool bHit = GetWorld()->LineTraceSingleByChannel(Hit,
			ProjectileSpawnSceneComponent->GetComponentLocation(), 
			PotentialTarget->GetActorLocation(),
			ECC_Visibility);
			if (bHit)
			{
				if (Hit.GetActor() == PotentialTarget)
				{
					CurrentTarget = PotentialTarget;
					//UE_LOG(LogTemp, Warning, TEXT("Found Current Target: %s. Has Line of Sight."), *CurrentTarget->GetName());
					//DrawDebugSphere(GetWorld(), CurrentTarget->GetActorLocation(), 50.f, 12, FColor::Red, false, 5.0f);
				}
			}
		}
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("No Potential Building Component Targets Found"));
	}
}

void AEnemyDrone::MoveToOutsideSpawnArea(float DeltaTime)
{
	//Using this to get the Drone from the Spawn Area into the open Map.
	//Initialized to 0 (Can be any distance along the spline)
	if (ClosestLabDoorExitLocation != FVector::ZeroVector)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Closest Lab Door Exit Location: %s"), *ClosestLabDoorExitLocation.ToString());
		//DrawDebugSphere(GetWorld(), ClosestLabDoorExitLocation, 50.f, 12, FColor::Red, false, 5.0f);
		FVector DroneLocation = GetActorLocation();
		//Getting a vector that points from drone to spline start location
		FVector MoveDirection = (ClosestLabDoorExitLocation - DroneLocation).GetSafeNormal();
		//Getting a point scaled by the speed of the drone along the Move Direction
		FVector NewLocation = DroneLocation + MoveDirection * DroneApproachSpeed * DeltaTime;

		SetActorLocation(NewLocation);
		SetActorRotation(MoveDirection.Rotation());

		//Checking if within range of the spline start location
		if (FVector::Dist(NewLocation, ClosestLabDoorExitLocation) < 20.0f)
		{
			//When within range of spline start, move to standard spline flight trajectory.
			bIsOutsideSpawnArea = true;
		}
	}
}

