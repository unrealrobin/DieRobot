// Property of Paracosm Industries. 


#include "BuildSystem/Traps/FrostTrap.h"

#include "NiagaraComponent.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StatusEffect/StatusConditionManager.h"
#include "Components/StatusEffect/StatusEffectHandlerComponent.h"
#include "Data/DataAssets/StatusEffects/StatusEffectBase.h"


// Sets default values
AFrostTrap::AFrostTrap()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SnapCondition = ESnapCondition::CenterSnap;

	FrostTrapFanMesh = CreateDefaultSubobject<UStaticMeshComponent>("Frost Trap Vent");
	FrostTrapFanMesh->SetupAttachment(TrapBaseStaticMesh);
	FrostTrapFanMesh->SetCollisionProfileName(TEXT("BuildableBlockEverything"));

	FrostTrapNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("Frost Trap Niagara");
	FrostTrapNiagaraComponent->SetupAttachment(FrostTrapFanMesh);

	HitBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AFrostTrap::HandleFrostTrapBeginOverlap);
	
}

// Called when the game starts or when spawned
void AFrostTrap::BeginPlay()
{
	Super::BeginPlay();

	if (!bIsProxy)
	{
		FrostTrapNiagaraComponent->Activate();
	}
	
}

// Called every frame
void AFrostTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFrostTrap::HandleFrostTrapBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA(ATimberEnemyCharacter::StaticClass()))
	{
		EffectConditionManager->ResolveEffect(StatusEffectDefinitions, OtherActor);
	}
}


