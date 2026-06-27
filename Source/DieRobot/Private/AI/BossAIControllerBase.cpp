// Property of Paracosm Industries. Dont use my shit.


#include "AI/BossAIControllerBase.h"

#include "BehaviorTree/BlackboardComponent.h"


// Sets default values
ABossAIControllerBase::ABossAIControllerBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABossAIControllerBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABossAIControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



