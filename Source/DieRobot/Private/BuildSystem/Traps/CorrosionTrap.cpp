// Property of Paracosm.


#include "BuildSystem/Traps/CorrosionTrap.h"

#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StatusEffect/StatusConditionManager.h"

ACorrosionTrap::ACorrosionTrap()
{
	PrimaryActorTick.bCanEverTick = true;

	SnapCondition = ESnapCondition::CenterSnap;

	CorrosionTrapMeshFront = CreateDefaultSubobject<UStaticMeshComponent>("Corrosion Trap Front");
	CorrosionTrapMeshFront->SetupAttachment(TrapBaseStaticMesh);
	CorrosionTrapMeshFront->SetCollisionProfileName(TEXT("BuildableBlockEverything"));	
}

void ACorrosionTrap::BeginPlay()
{
	Super::BeginPlay();
	if (HitBoxComponent)
	{
		HitBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ACorrosionTrap::HandleFrostTrapBeginOverlap);
	}
	
}

void ACorrosionTrap::HandleFrostTrapBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//Apply Status Effect.
	if (OtherActor->IsA(ATimberEnemyCharacter::StaticClass()))
	{
		EffectConditionManager->ResolveEffect(StatusEffectDefinitions, OtherActor);
	}
}

void ACorrosionTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

