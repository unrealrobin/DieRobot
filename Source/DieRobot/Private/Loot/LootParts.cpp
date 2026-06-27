// Property of Paracosm Industries. Dont use my shit.


#include "Loot/LootParts.h"

#include "Character/TimberPlayableCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/Inventory/InventoryManagerComponent.h"


// Sets default values
ALootParts::ALootParts()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionCapsule->OnComponentBeginOverlap.AddDynamic(this, &ALootParts::HandleLootItemOverlap);
}

// Called when the game starts or when spawned
void ALootParts::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALootParts::HandleLootItemOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	Super::HandleLootItemOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	//When overlapped, add coins to the players state inventory
	//UE_LOG(LogTemp, Warning, TEXT("Overlapped Part"));
	ATimberPlayableCharacter* Character = Cast<ATimberPlayableCharacter>(OtherActor);
	if(Character)
	{
		// Add parts to player Inventory
		Character->InventoryManager->AddPartsToInventory(1);

		// Destroy the parts item
		Destroy();
	}
}

// Called every frame
void ALootParts::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

