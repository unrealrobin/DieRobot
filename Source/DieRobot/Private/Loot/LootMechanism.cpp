// Property of Paracosm Industries. Dont use my shit.


#include "Loot/LootMechanism.h"

#include "Character/TimberPlayableCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/Inventory/InventoryManagerComponent.h"


// Sets default values
ALootMechanism::ALootMechanism()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
}

// Called when the game starts or when spawned
void ALootMechanism::BeginPlay()
{
	Super::BeginPlay();
	CollisionCapsule->OnComponentBeginOverlap.AddDynamic(this, &ALootMechanism::HandleLootItemOverlap);
}

void ALootMechanism::HandleLootItemOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	Super::HandleLootItemOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	//UE_LOG(LogTemp, Warning, TEXT("Overlapped Part"));
	ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(OtherActor);

	if(PlayerCharacter)
	{
		// Add parts to player Inventory
		PlayerCharacter->InventoryManager->AddMechanismsToInventory(1);

		// Destroy the parts item
		Destroy();
	}
}

// Called every frame
void ALootMechanism::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

