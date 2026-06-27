// Property of Paracosm Industries.


#include "BuildSystem/Traps/SpikeTrap.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/StatusEffect/StatusConditionManager.h"

// Sets default values
ASpikeTrap::ASpikeTrap()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SnapCondition = ESnapCondition::CenterSnap;
	TrapSpikeMesh = CreateDefaultSubobject<UStaticMeshComponent>("Spikes");
	TrapSpikeMesh->SetupAttachment(TrapBaseStaticMesh);
	TrapSpikeMesh->SetCollisionProfileName(TEXT("NoCollision"));
	SpikeOutTimeline = CreateDefaultSubobject<UTimelineComponent>("SpikeOutTimeline");
}

void ASpikeTrap::BeginPlay()
{
	Super::BeginPlay();
	HitBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ASpikeTrap::HandleSpikeTrapOverlap);
	SetupTimelineData();
	FVector SpikeStartScale = FVector(1.0f, 1.0f, 0.1f);
	TrapSpikeMesh->SetRelativeScale3D(SpikeStartScale);
}

// Called every frame
void ASpikeTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Enemies pile up but don't Begin Overlap within the spike trap, so we force the spike out attack if the Trap is off cooldown.
	if(InsideHitBoxArray.Num() != 0 && !IsSpikeOnCooldown && AmplificationState != EAmplificationState::Amplified)
	{
		SetSpikeOutAttackTimer();
	}
}

void ASpikeTrap::SetSpikeOutAttackTimer()
{
	GetWorld()->GetTimerManager().SetTimer(
		TimeToActiveSpikeOutAttack, this, &ASpikeTrap::HandleSpikeOutAttack,
		TimeToActiveSpikeOutAttackValue, false);
	IsSpikeOnCooldown = true;
}

void ASpikeTrap::SetIsAmplified(bool bIsAmplified)
{
	Super::SetIsAmplified(bIsAmplified);
	//Changes that need to be made to this trap when it is amplified.
	if (bIsAmplified)
	{
		StartAmplifiedTimelineAnimationLoop();
	}
	else
	{
		StopAmplifiedTimelineAnimationLoop();
	}
}

void ASpikeTrap::HandleSpikeTrapOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent*
	OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AmplificationState == EAmplificationState::Amplified) return;
	
	//Timer from initial activation to spike out attack
	ATimberEnemyCharacter* Enemy = Cast<ATimberEnemyCharacter>(OtherActor);
	if (Enemy)
	{
		if (!IsSpikeOnCooldown) // IF the spike is not on cooldown, then we can activate the spike out attack.
		{
			SetSpikeOutAttackTimer();
		}
	}
}

void ASpikeTrap::HandleSpikeOutAttack()
{
	PlaySpikeOutTimeline();
	
	ApplyStatusEffectToEnemy();
}

void ASpikeTrap::ApplyStatusEffectToEnemy()
{
	if (InsideHitBoxArray.Num() > 0)
	{
		//Creating a shallow copy here to avoid modifications to InsideHitBoxArray during execution of Resolve Effect
		//Causing Ensure Condition Failure.
		const TArray<TWeakObjectPtr<AActor>> Snapshot = InsideHitBoxArray;
		
		for (TWeakObjectPtr HitActor : Snapshot)
		{
			if (HitActor.IsValid() && HitActor->IsA(ATimberEnemyCharacter::StaticClass()))
			{
				EffectConditionManager->ResolveEffect(StatusEffectDefinitions, HitActor.Get());
			}
		}
	}
}

/*Timeline Animations*/
void ASpikeTrap::SetupTimelineComponents()
{
	SpikeOutTimeline = CreateDefaultSubobject<UTimelineComponent>("SpikeOutTimeline");
}

void ASpikeTrap::SetupTimelineData()
{
	if (SpikeAnimVectorCurve)
	{
		//Called every frame while timeline is running.
		FOnTimelineVector SpikeOutProgress;
		//Binding this callback function to this Timeline
		SpikeOutProgress.BindUFunction(this, FName("HandleVectorCurveUpdate"));

		FOnTimelineEvent SpikeOutFinished;
		SpikeOutFinished.BindUFunction(this, FName("SpikeOutTimelineFinished"));
		//Takes a vector and an event to bind to the timeline
		SpikeOutTimeline->AddInterpVector(SpikeAnimVectorCurve, SpikeOutProgress);
		//Takes an event to bind to the timeline finishing. The event then calls the function that is bound to it. (SpikeOutTimelineFinished())
		SpikeOutTimeline->SetTimelineFinishedFunc(SpikeOutFinished);
		SpikeOutTimeline->SetLooping(false);
	}
}

void ASpikeTrap::HandleVectorCurveUpdate(FVector CurveVector)
{
	//where you define what happens every frame, such as changing scale, position, or material.
	if (TrapSpikeMesh)
	{
		TrapSpikeMesh->SetRelativeScale3D(CurveVector);
	}
}

void ASpikeTrap::SpikeOutTimelineFinished()
{
	if (IsSpikesOut) // This is what is called when the Spike Anim unleashes the spikes as the end function.
	{
		PlaySpikeOutTimeline_Reverse();
	}
	else // This is what is called when the Spike Anim retracts the spikes as the end function.
	{
		IsSpikesOut = false;
		//Timer Initiates a Cool down before the Spike Attack can Happen again.
		GetWorld()->GetTimerManager().SetTimer(
			SpikeOutCooldown, this, &ASpikeTrap::EndSpikeTrapCooldown, SpikeOutCooldownValue,
			false);
	}
}

void ASpikeTrap::PlaySpikeOutTimeline()
{
	if (SpikeOutTimeline)
	{
		SpikeOutTimeline->PlayFromStart();
		IsSpikesOut = true;
	}
}

void ASpikeTrap::PlaySpikeOutTimeline_Reverse()
{
	SpikeOutTimeline->ReverseFromEnd();
	IsSpikesOut = false;
}

void ASpikeTrap::EndSpikeTrapCooldown()
{
	IsSpikeOnCooldown = false;
}
