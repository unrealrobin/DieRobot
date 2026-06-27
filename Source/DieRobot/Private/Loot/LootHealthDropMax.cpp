// Property of Paracosm Industries. Dont use my shit.


#include "Loot/LootHealthDropMax.h"

#include "Character/TimberPlayableCharacter.h"


ALootHealthDropMax::ALootHealthDropMax()
{
	PrimaryActorTick.bCanEverTick = true;
}


void ALootHealthDropMax::BeginPlay()
{
	Super::BeginPlay();
}

//Bound In Parent Class
void ALootHealthDropMax::HandlePlayerOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(OtherActor);

	//Do Nothing if already at max health.
	if (PlayerCharacter && PlayerCharacter->CurrentHealth < PlayerCharacter->MaxHealth)
	{
		PlayerCharacter->PlayerGainHealth(PlayerCharacter->MaxHealth);
		PlaySFX();
		Destroy();
	}
}


void ALootHealthDropMax::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

