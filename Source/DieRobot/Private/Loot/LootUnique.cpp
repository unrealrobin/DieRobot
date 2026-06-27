// Property of Paracosm Industries. Dont use my shit.


#include "Loot/LootUnique.h"

#include "Character/TimberPlayableCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/Inventory/InventoryManagerComponent.h"


class ATimberPlayableCharacter;
// Sets default values
ALootUnique::ALootUnique()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALootUnique::BeginPlay()
{
	Super::BeginPlay();

	CollisionCapsule->OnComponentBeginOverlap.AddDynamic(this, &ALootUnique::HandleLootItemOverlap);
	
}

// Called every frame
void ALootUnique::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALootUnique::HandleLootItemOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	Super::HandleLootItemOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	//UE_LOG(LogTemp, Warning, TEXT("Overlapped Part"));
	ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(OtherActor);

	if(PlayerCharacter)
	{
		// Add parts to player Inventory
		PlayerCharacter->InventoryManager->AddUniquesToInventory(1);

		// Destroy the parts item
		Destroy();
	}
}

