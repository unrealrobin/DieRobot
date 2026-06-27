// Property of Paracosm Industries. Dont use my shit.


#include "Environment/LocationMarkerBase.h"

#include "Character/TimberPlayableCharacter.h"
#include "States/DieRobotGameStateBase.h"


// Sets default values
ALocationMarkerBase::ALocationMarkerBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	RootComponent = StaticMeshComponent;

	StaticMeshComponent->SetCollisionProfileName("DR_HitEventOnly");
	StaticMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ALocationMarkerBase::HandleLocationOverlap);
}

// Called when the game starts or when spawned
void ALocationMarkerBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALocationMarkerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALocationMarkerBase::HandleLocationOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<ATimberPlayableCharacter>(OtherActor))
	{
		//Send Broadcast to Game System to Increment the Tutorial State

		ADieRobotGameStateBase* DieRobotGameStateBase = GetWorld()->GetGameState<ADieRobotGameStateBase>();
		if (DieRobotGameStateBase)
		{
			DieRobotGameStateBase->ChangeTutorialGameState(ETutorialState::Wake3);

			Destroy();
		}
	}
}


