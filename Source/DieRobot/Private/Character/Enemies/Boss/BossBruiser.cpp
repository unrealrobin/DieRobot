// Property of Paracosm Industries. Dont use my shit.


#include "Character/Enemies/Boss/BossBruiser.h"

#include "AudioDevice.h"
#include "Character/TimberPlayableCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"


// Sets default values
ABossBruiser::ABossBruiser()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CurrentHealth = MaxHealth;
	SetupCapsuleComponents();

	WhirlwindRightCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABossBruiser::HandleWhirlwindOverlap);
	WhirlwindLeftCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABossBruiser::HandleWhirlwindOverlap);
	
	RightArmCapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ABossBruiser::HandleBHandSlapOverlap);

	BossTechnicalName = "8RU153R";
}

// Called when the game starts or when spawned
void ABossBruiser::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABossBruiser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABossBruiser::EnableCollisionToDamagePlayerOnly(UCapsuleComponent* ComponentToUpdate)
{
	if (ComponentToUpdate)
	{
		ComponentToUpdate->SetCollisionProfileName("DR_HitEventOnly");
	}
}

void ABossBruiser::DisableCollisionToDamagePlayerOnly(UCapsuleComponent* ComponentToUpdate)
{
	if (ComponentToUpdate)
	{
		ComponentToUpdate->SetCollisionProfileName("NoCollision");
	}
}

void ABossBruiser::HandleWhirlwindOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(OtherActor);

	//Only damages the Player, will not Damage other enemies or Building Components.
	if (PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Whirlwind Applied Damage to Player."));
		PlayerCharacter->PlayerTakeDamage(WhirlwindTickDamage);
	}
}

void ABossBruiser::HandleBHandSlapOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{

	UE_LOG(LogTemp, Warning, TEXT("Overlapped Component: %s"), *OtherComp->GetName());
	UE_LOG(LogTemp, Warning, TEXT("Overlapped Component Owner: %s"), *OtherComp->GetOwner()->GetName());
	//Issue is the OtherComp is the Hit Box from the Weapon Ability and its Owner returns to the Player Character Initiating Damage.
	
	ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Backhand Slap Applied Damage to Player."));
		PlayerCharacter->PlayerTakeDamage(BHandSlapDamage);
	}
}

void ABossBruiser::HandleOverHeadSmashOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Bruiser - Overhead Smash Overlapped."));
	ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Overhead Smash Applied Damage to Player."));
		PlayerCharacter->PlayerTakeDamage(OverHeadSmashDamage);
	}
}

void ABossBruiser::SpawnOverHeadCapsule()
{
	OverHeadSmashCapsuleComponent = NewObject<UCapsuleComponent>(this);
	if (OverHeadSmashCapsuleComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("OverHeadSmashCapsuleComponent Created."));
		OverHeadSmashCapsuleComponent->AttachToComponent(
			OverHeadSmashCapsuleSpawnLocation,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		//Collision Delegate
		OverHeadSmashCapsuleComponent->OnComponentBeginOverlap.AddDynamic(
			this, &ABossBruiser::HandleOverHeadSmashOverlap);
		OverHeadSmashCapsuleComponent->RegisterComponent();
		OverHeadSmashCapsuleComponent->SetCapsuleHalfHeight(OverHeadSmashCapsuleHeight, true);
		OverHeadSmashCapsuleComponent->SetCapsuleRadius(OverHeadSmashCapsuleRadius, true);
		OverHeadSmashCapsuleComponent->SetCollisionProfileName("DR_HitEventOnly");

		
	}
}

void ABossBruiser::DestroyOverHeadCapsule()
{
	if (OverHeadSmashCapsuleComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("OverHeadSmashCapsuleComponent Destroyed."));
		OverHeadSmashCapsuleComponent->DestroyComponent();
	}
}

void ABossBruiser::SetupCapsuleComponents()
{
	if (GetMesh())
	{
		//Used for HeadShotCollisions
		HeadCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("HeadCapsuleComponent");
		HeadCapsuleComponent->SetupAttachment(GetMesh(), FName("headSocket"));
		HeadCapsuleComponent->SetCollisionProfileName("DR_HitEventOnly");

		BodyCollisionComponent = CreateDefaultSubobject<UBoxComponent>("BodyCollisionComponent");
		BodyCollisionComponent->SetupAttachment(RootComponent);
		BodyCollisionComponent->SetCollisionProfileName("DR_HitEventOnly");

		//Used only for attack animation collision
		RightArmCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("RightArmCapsuleComponent");
		LeftArmCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("LeftArmCapsuleComponent");
		RightArmCapsuleComponent->SetupAttachment(GetMesh(), FName("rightArmSocket"));
		LeftArmCapsuleComponent->SetupAttachment(GetMesh(), FName("leftArmSocket"));
		RightArmCapsuleComponent->SetCollisionProfileName("DR_EnemyCharacterCapsule");
		LeftArmCapsuleComponent->SetCollisionProfileName("DR_EnemyCharacterCapsule");

		//Capsules for Whirldwind Animation
		WhirlwindRightCollisionSphere = CreateDefaultSubobject<UCapsuleComponent>("WhirlwindRightCollisionSphere");
		WhirlwindLeftCollisionSphere = CreateDefaultSubobject<UCapsuleComponent>("WhirlwindLeftCollisionSphere");
		WhirlwindRightCollisionSphere->SetupAttachment(RightArmCapsuleComponent);
		WhirlwindLeftCollisionSphere->SetupAttachment(LeftArmCapsuleComponent);
		
		//Collision Enabling will happen in the Event Notify Custom Event Function
		DisableCollisionToDamagePlayerOnly(WhirlwindRightCollisionSphere);
		DisableCollisionToDamagePlayerOnly(WhirlwindLeftCollisionSphere);

		//Creating Scene Component To Spawn Overhead Smash Location
		OverHeadSmashCapsuleSpawnLocation = CreateDefaultSubobject<
			USceneComponent>("OverHeadSmashCapsuleSpawnLocation");
		OverHeadSmashCapsuleSpawnLocation->SetupAttachment(GetRootComponent());
	}
}
