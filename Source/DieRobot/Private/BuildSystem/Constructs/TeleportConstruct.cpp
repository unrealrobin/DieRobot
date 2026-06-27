// Property of Paracosm Industries.


#include "BuildSystem/Constructs/TeleportConstruct.h"
#include "Character/TimberPlayableCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATeleportConstruct::ATeleportConstruct()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SnapCondition = ESnapCondition::FloorEdgeSnapTopOnly;

	SetupComponents();

	SaveDefaultMaterials();

	OverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ATeleportConstruct::HandleTeleportOverlap);
	OverlapBox->OnComponentHit.AddDynamic(this, &ATeleportConstruct::HandleTeleportHit);
}

// Called when the game starts or when spawned
void ATeleportConstruct::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<ATimberPlayableCharacter>(UGameplayStatics::GetActorOfClass(GetWorld(), 
	ATimberPlayableCharacter::StaticClass()));
	//UE_LOG(LogTemp, Warning, TEXT("Begin Play Teleporter GUID: %s"), *BuildSystemGUID.ToString());

	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		if (bIsProxy)
		{
			//We dont want to Square Mesh Plane to show in the proxy.
			if (TeleportEffectMesh)
			{
				TeleportEffectMesh->DestroyComponent();
			}
		}
	});
}

// Called every frame
void ATeleportConstruct::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CheckForPairOutsideBuildMode();
}

void ATeleportConstruct::SetupComponents()
{
	TeleportMesh = CreateDefaultSubobject<UStaticMeshComponent>("BaseStaticMesh");
	TeleportMesh->SetupAttachment(RootComponent);

	TeleportEffectMesh = CreateDefaultSubobject<UStaticMeshComponent>("TeleportEffectMesh");
	TeleportEffectMesh->SetupAttachment(TeleportMesh);
	
	TeleportLandingLocation = CreateDefaultSubobject<USceneComponent>("TeleportLandingLocation");
	TeleportLandingLocation->SetupAttachment(TeleportMesh);

	OverlapBox = CreateDefaultSubobject<UBoxComponent>("OverlapBox");
	OverlapBox->SetupAttachment(RootComponent);

	TeleportMesh->SetCollisionProfileName(TEXT("DR_BuildableBlockEverything"));
	OverlapBox->SetCollisionProfileName(TEXT("DR_HitEventOnly"));
}

void ATeleportConstruct::HandleTeleportOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlapped Teleporter."));
	ATimberPlayableCharacter* Player = Cast<ATimberPlayableCharacter>(OtherActor);
	if (Player && TeleportPair)
	{
		PlayerToTeleport = Player;
		FVector TeleportToLocation = TeleportPair->TeleportLandingLocation->GetComponentLocation();

		//Instant Teleport
		PlayerToTeleport->SetActorLocation(TeleportToLocation);
	}
	
}

void ATeleportConstruct::HandleTeleportHit(
	UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("Hit Teleporter."));
	ATimberPlayableCharacter* Player = Cast<ATimberPlayableCharacter>(OtherActor);
	if (Player && TeleportPair)
	{
		PlayerToTeleport = Player;
		FVector TeleportToLocation = TeleportPair->TeleportLandingLocation->GetComponentLocation();
		
		//Instant Teleport
		PlayerToTeleport->SetActorLocation(TeleportToLocation);
	}
}

void ATeleportConstruct::StartTeleport(FVector TargetDestination, float DurationToTeleport)
{
	StartLocation = PlayerCharacter->GetActorLocation();
	TargetLocation = TeleportPair->TeleportLandingLocation->GetComponentLocation();
	TeleportDuration = DurationToTeleport;
	TeleportTimeElapsed = 0.f;

	GetWorld()->GetTimerManager().SetTimer(TeleportTimerHandle, this, &ATeleportConstruct::UpdateTeleport, 0.01f, true);
}

void ATeleportConstruct::UpdateTeleport()
{
	TeleportTimeElapsed += 0.01f;

	//Gets me the amount of time that has elapsed as a percentage of total time.
	float Alpha = FMath::Clamp(TeleportTimeElapsed / TeleportDuration, 0.f, 1.f);
	FVector NewLocation = FMath::Lerp(StartLocation, TargetLocation, Alpha);
	PlayerToTeleport->SetActorLocation(NewLocation);

	if (Alpha >= 1.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(TeleportTimerHandle);
		PlayerToTeleport = nullptr;
	}
}

void ATeleportConstruct::SaveDefaultMaterials()
{
	if (TeleportMesh)
	{
		DefaultBaseMaterial = TeleportMesh->GetMaterial(0);
		DefaultEffectMesh = TeleportEffectMesh->GetMaterial(0);
	}
}

void ATeleportConstruct::ApplyDefaultMaterials()
{
	//Apply Default Materials to the Pillars - defaults set in SaveDefaultMaterials
	if (TeleportMesh)
	{
		TeleportMesh->SetMaterial(0, DefaultBaseMaterial);
		TeleportEffectMesh->SetMaterial(0, DefaultEffectMesh);
	}
}

void ATeleportConstruct::CheckForPairOutsideBuildMode()
{
	//If the character is outside of Build State and the Pair for this Teleport isn't set, destroy the teleporter.
	//Must have a pair to exist.
	if (PlayerCharacter && PlayerCharacter->CharacterState != ECharacterState::Building && !TeleportPair)
	{
		Destroy();
	}
}

void ATeleportConstruct::LinkToPair(ATeleportConstruct* Pair)
{
	if (Pair && Pair != this)
	{
		// This is Pair(Pair A), Linking the other Pair(Pair B) to this pairs(Pairs A) death.
		// When this pair is destroyed, it will call the other Pairs OnPairDestroyedFunction.
		Pair->OnDestroyed.AddDynamic(this, &ATeleportConstruct::OnPairDestruction);
	}
}

void ATeleportConstruct::OnPairDestruction(AActor* DestroyedActor)
{
	//This only gets called on the pair that isn't destroyed, but is linked to the destroyed pair.
	if (DestroyedActor == TeleportPair)
	{
		//Removing this Pairs (Pair A) reference to the destroyed Pair (Pair B) - Its Already destroyed.
		// and doesn't need a callback to its own pair (Pair b)
		OnDestroyed.RemoveDynamic(this, &ATeleportConstruct::OnPairDestruction);
	}

	Destroy();
}

void ATeleportConstruct::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (TeleportPair)
	{
		UE_LOG(LogTemp, Warning, TEXT("Teleport Construct End Play. Went through Destruction."));
		//Because Pair A was linked earlier, but destroys first, we need to remove the callback to Pair B that was added to Pair A.
		// We are unbinding from the callback that would be called if Pair B was destroyed first.
		TeleportPair->OnDestroyed.RemoveDynamic(this, &ATeleportConstruct::OnPairDestruction);
	}
}

void ATeleportConstruct::SpawnLoot()
{
	SpawnLootInRange(BuildableCost.CostOfParts, BuildableCost.CostOfMechanisms, BuildableCost.CostOfUniques);
}



