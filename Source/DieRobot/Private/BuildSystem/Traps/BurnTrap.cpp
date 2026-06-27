// Property of Paracosm.


#include "BuildSystem/Traps/BurnTrap.h"

#include "NiagaraComponent.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StatusEffect/StatusConditionManager.h"
#include "Data/DataAssets/StatusEffects/StatusEffectBase.h"


// Sets default values
ABurnTrap::ABurnTrap()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SnapCondition = ESnapCondition::CenterSnap;
	
	BurnTrapInternalsMesh = CreateDefaultSubobject<UStaticMeshComponent>("BurnTrapInternalsMesh");
	BurnTrapInternalsMesh->SetupAttachment(TrapBaseStaticMesh);

	BurnTrapInternalsNiagara = CreateDefaultSubobject<UNiagaraComponent>("BurnTrapInternalsNiagara");
	BurnTrapInternalsNiagara->SetupAttachment(BurnTrapInternalsMesh);
	
	HitBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ABurnTrap::HandleTrapBeginOverlap);
	HitBoxComponent->OnComponentEndOverlap.AddDynamic(this, &ABurnTrap::HandleTrapEndOverlap);
	
}

// Called when the game starts or when spawned
void ABurnTrap::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle CheckNiagaraActiveTimerHandle;
	GetWorldTimerManager().SetTimer(CheckNiagaraActiveTimerHandle, this, &ABurnTrap::CheckNiagaraActive, NiagaraCheckTimer, true);
}

void ABurnTrap::HandleTrapBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (BurnTrapInternalsNiagara && EffectConditionManager && OtherActor->IsA(ATimberEnemyCharacter::StaticClass()))
	{
		BurnTrapInternalsNiagara->ActivateSystem();
		EffectConditionManager->ResolveEffect(StatusEffectDefinitions, OtherActor);
	}
}

void ABurnTrap::HandleTrapEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 INT32)
{
	//Check if anyone is still in hit-box, if not, turn off the effect.
	if (OtherActor->IsA(ATimberEnemyCharacter::StaticClass()) && InsideHitBoxArray.Num() == 0 )
	{
		if (BurnTrapInternalsNiagara)
		{
			BurnTrapInternalsNiagara->Deactivate();
			//UE_LOG(LogTemp, Display, TEXT("Burn Trap - Deactivated Niagara System."));
		}
	}
}

void ABurnTrap::CheckNiagaraActive()
{
	if (BurnTrapInternalsNiagara)
	{
		if (BurnTrapInternalsNiagara->IsActive() && InsideHitBoxArray.Num() == 0)
		{
			BurnTrapInternalsNiagara->Deactivate();
		}

		if (InsideHitBoxArray.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Enemies still inside Hit Box."));
		}

		if (!BurnTrapInternalsNiagara->IsActive())
		{
			//UE_LOG(LogTemp, Warning, TEXT("Burn Trap Niagra is Inactive."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Burn Trap - Burn Trap Internals Niagara is NULL!"));
	}
}

