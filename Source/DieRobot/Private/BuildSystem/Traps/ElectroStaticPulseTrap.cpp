// Property of Paracosm Industries. 


#include "BuildSystem/Traps/ElectroStaticPulseTrap.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StatusEffect/StatusConditionManager.h"

AElectroStaticPulseTrap::AElectroStaticPulseTrap()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SnapCondition = ESnapCondition::CenterSnap;
}

void AElectroStaticPulseTrap::SetIsAmplified(bool bIsAmplified)
{
	Super::SetIsAmplified(bIsAmplified);
	
	if (GetWorld()->GetTimerManager().IsTimerActive(ElectroPulseFiringCooldownTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(ElectroPulseFiringCooldownTimer);
	}

	//stop any current spawning of Electro pulse spheres
	if (bIsAmplified)
	{
		SetupElectroPulseLoopTimer(0.4f);
	}
	else
	{
		SetupElectroPulseLoopTimer(2.0f);
	}
}

void AElectroStaticPulseTrap::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		SetupElectroPulseLoopTimer(2.0f);
	});
}

void AElectroStaticPulseTrap::BeginDestroy()
{
	Super::BeginDestroy();
	if (PulseSphereMeshComponent)
	{
		PulseSphereMeshComponent->DestroyComponent();
	}
}

void AElectroStaticPulseTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!bIsProxy)
	{
		MovePulseHitSphere(DeltaTime);
		
		for (FGuid ids : DestroyedSphereGuids)
		{
			FElectroPulseSphereData* Found = ElectroPulseSphereInstances.FindByPredicate([&, ids](const FElectroPulseSphereData& Sphere)
			{
				return Sphere.ElectroPulseSphereGuid == ids;
			});
			
			if (Found)
			{
				ElectroPulseSphereInstances.RemoveSingleSwap(*Found);
			}
		}
		
		DestroyedSphereGuids.Empty();
	}
}

void AElectroStaticPulseTrap::FireElectroPulse()
{
	CreatePulseHitSphere();
}

void AElectroStaticPulseTrap::CreatePulseHitSphere()
{
	FElectroPulseSphereData NewSphereData; 
	NewSphereData.ElectroPulseSphereGuid = FGuid::NewGuid();
	
	/*Adjusting Spawn Location by Radius of Sphere*/
	const FVector BaseLocation = BoxExtentRaycastStart->GetComponentLocation();
	const FVector ForwardOffset = BoxExtentRaycastStart->GetForwardVector() * SphereRadius;
	//DrawDebugSphere(GetWorld(), BaseLocation + ForwardOffset, 20.0f, 12, FColor::Blue, false, 5.0f);
	
	const FVector AdjustedLocation = BaseLocation + ForwardOffset;
	
	NewSphereData.SphereCapsule = NewObject<USphereComponent>(this);
	if (NewSphereData.SphereCapsule)
	{
		/*Sphere CollisionComponent*/
		NewSphereData.SphereCapsule->SetSphereRadius(SphereRadius);
		NewSphereData.SphereCapsule->SetWorldLocation(AdjustedLocation);
		NewSphereData.SphereCapsule->RegisterComponent();
		NewSphereData.SphereCapsule->SetCollisionProfileName(TEXT("HitEventOnly"));
		NewSphereData.SphereCapsule->SetGenerateOverlapEvents(true);
		NewSphereData.SphereCapsule->OnComponentBeginOverlap.AddDynamic(this, &AElectroStaticPulseTrap::HandlePulseSphereOverlap);

		if (PulseSphereMesh)
		{
			//Create the Static Mesh Component of the Pulse
			NewSphereData.SphereMesh = NewObject<UStaticMeshComponent>(this);
			NewSphereData.SphereMesh->RegisterComponent();
			NewSphereData.SphereMesh->AttachToComponent(NewSphereData.SphereCapsule, FAttachmentTransformRules::KeepRelativeTransform);
			NewSphereData.SphereMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			NewSphereData.SphereMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
			NewSphereData.SphereMesh->SetStaticMesh(PulseSphereMesh);
			NewSphereData.SphereMesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f));
		}
	}
	
	if (NewSphereData.SphereCapsule && NewSphereData.SphereMesh)
	{
		AddNewSphereToPulseArray(NewSphereData);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SphereCapsule or SphereMesh is NULL"));
	}
}

void AElectroStaticPulseTrap::HandlePulseSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//If it overlaps an enemy, add the status effect to the enemy.
	if (OtherActor->IsA(ATimberEnemyCharacter::StaticClass()))
	{
		EffectConditionManager->ResolveEffect(StatusEffectDefinitions, OtherActor);
	}
}

void AElectroStaticPulseTrap::MovePulseHitSphere(float DeltaTime)
{
	for (FElectroPulseSphereData SphereInstance : ElectroPulseSphereInstances)
	{
		if (SphereInstance.SphereCapsule)
		{
			//Original Location
			FVector PulseStartLocation = SphereInstance.SphereCapsule->GetComponentLocation();

			//Pulse Direction
			FVector ForwardVector = BoxExtentRaycastStart->GetForwardVector();
			
			//Amount of Translation 
			FVector NewLocation =	PulseStartLocation + (ForwardVector * 200 * DeltaTime);
			
			SphereInstance.SphereCapsule->SetRelativeLocation(NewLocation);

			//Once the Pulse has Reached the End of the Hit Box Extents, Destroy the Pulse Hit Box.
			bool HitEndOfBoxExtent = FVector::Dist(SphereInstance.SphereCapsule->GetComponentLocation(), 
			                                       BoxExtentRaycastStart->GetComponentLocation()) >= (CalculatedBoxLength - SphereRadius);

			if ( HitEndOfBoxExtent )
			{
				/*
				 * When the sphere hits its destination, we destroy both the capsule and mesh components.
				 * The sphere Struct ref, still lives on the ElectroPulseSphereInstances Array, which then removes those structs, 
				 * in tick, after completing movement for each of the spheres. 
				 */
				DestroySphereComponents(SphereInstance.ElectroPulseSphereGuid);
			}
		}
	}
}

void AElectroStaticPulseTrap::SetupElectroPulseLoopTimer(float InRate)
{
	TWeakObjectPtr<AElectroStaticPulseTrap> WeakThis = this;
	GetWorld()->GetTimerManager().SetTimer(ElectroPulseFiringCooldownTimer, [WeakThis]()
	{
		if (WeakThis.IsValid())
		{
			WeakThis->FireElectroPulse();
		}
	}, InRate, true);
}

void AElectroStaticPulseTrap::DestroySphereComponents(FGuid InSphereGuid)
{
	for (int i = 0; i < ElectroPulseSphereInstances.Num(); i++)
	{
		if (ElectroPulseSphereInstances[i].ElectroPulseSphereGuid == InSphereGuid)
		{
			ElectroPulseSphereInstances[i].SphereCapsule->DestroyComponent();
			ElectroPulseSphereInstances[i].SphereMesh->DestroyComponent();
			UE_LOG(LogTemp, Warning, TEXT("Destroyed Sphere Components"));
			//Storing an array of FGUID that will be used to remove the elements from the ElectroPulseSphereInstances Array
			DestroyedSphereGuids.Add(InSphereGuid);
		}
	}
}

void AElectroStaticPulseTrap::AddNewSphereToPulseArray(FElectroPulseSphereData NewSphereData)
{
	ElectroPulseSphereInstances.Add(NewSphereData);
	UE_LOG(LogTemp, Warning, TEXT("Added Sphere to Array"));
}





