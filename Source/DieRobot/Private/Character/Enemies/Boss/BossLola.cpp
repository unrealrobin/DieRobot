// Property of Paracosm Industries. Don't use my shit.


#include "Character/Enemies/Boss/BossLola.h"

#include "AI/BossAIControllerBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BuildSystem/BuildableBase.h"
#include "BuildSystem/BuildingComponents/TimberBuildingComponentBase.h"
#include "BuildSystem/Ramps/RampBase.h"
#include "Character/Enemies/FloaterDrones.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Types/Combat/DamagePayload.h"


// Sets default values
ABossLola::ABossLola()
{
	PrimaryActorTick.bCanEverTick = true;
	LolaInitializeComponents();

	BossTechnicalName = "L0L4";
}

void ABossLola::BeginPlay()
{
	Super::BeginPlay();

	//Set Lola Controller
	InitializeLolaController();

	//Initializing Drones
	PopulateDronesArray();

	//When a drone dies Lola Get Stunned & isDamageable
	BindToDroneDeathDelegates();

	//Initial Drone Vulnerability Randomization
	RandomizeDroneVulnerability();
	
	//Timer for randomizing Drone Vulnerability
	GetWorld()->GetTimerManager().SetTimer(RandomizeDroneVulnerability_Handle, this, 
	&ABossLola::RandomizeDroneVulnerability, RandomizationTime, true );

	//Setting Max Walk Speed for Lola
	GetCharacterMovement()->MaxWalkSpeed = 300.f;

	//Get All Collision Components for handling change in damage state
	GetAllCapsuleComponents();

	//Lola is not damageable at the start - must kill the first drone.
	SetLolaNotDamageable();
}

void ABossLola::SetLolaNotDamageable()
{
	/*if (CapsuleComponentsArray.Num() > 0)
	{
		for (UPrimitiveComponent* LolaCapsuleComponent : CapsuleComponentsArray)
		{
			//No overlap Hits.
			LolaCapsuleComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
			LolaCapsuleComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);
		}
	}*/

	if (GetMesh())
	{
		/*GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);*/

		LolaTakeDamagePotential = ELolaTakeDamePotential::NotDamageable;

		//Adding Shield Overlay Material VFX.
		AddOverlayMaterialToCharacter(ShieldOverlayMaterial, 0.0f);
	}
}

void ABossLola::SetLolaToDamageable()
{
	if (CapsuleComponentsArray.Num() > 0)
	{
		//Projectile and Melee Weapons handle damage on Overlap NOT Block (Tho projectiles could.)
		for (UPrimitiveComponent* LolaCapsuleComponent : CapsuleComponentsArray)
		{
			//Removing Shield Overlay Material VFX
			RemoveOverlayMaterialFromCharacter();

			LolaTakeDamagePotential = ELolaTakeDamePotential::Damageable;
			
			/*//Overlapping Melee Weapons
			LolaCapsuleComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

			//Overlapping Projectiles
			LolaCapsuleComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);*/
		}
	}

	
}

void ABossLola::GetAllCapsuleComponents()
{
TArray<UPrimitiveComponent*> PrimitiveComponentArray;
	GetComponents(PrimitiveComponentArray);

    for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponentArray)
    {
        UShapeComponent* ShapeComponent = Cast<UShapeComponent>(PrimitiveComponent);
        if (ShapeComponent)
        {
            CapsuleComponentsArray.Add(ShapeComponent);
        }
    }
}

void ABossLola::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABossLola::TakeDamage(FDamagePayload DamagePayload)
{
	/*This won't ALWAYS fire because the collision settings change periodically.
	See the SetLolaToDamageable() and SetLolaNotDamageable() functions.
	See the SetLolaStunned() and SetLolaNotStunned() functions.
	*/
	if (LolaTakeDamagePotential == ELolaTakeDamePotential::NotDamageable)
	{
		return;
	}
	FDamagePayload Payload;
	Super::TakeDamage(Payload);
}

void ABossLola::BindToDroneDeathDelegates()
{
	if (Drone1 && Drone2 && Drone3)
	{
		Drone1->DroneDestroyedHandle.AddDynamic(this, &ABossLola::HandleDroneDeath);
		Drone2->DroneDestroyedHandle.AddDynamic(this, &ABossLola::HandleDroneDeath);
		Drone3->DroneDestroyedHandle.AddDynamic(this, &ABossLola::HandleDroneDeath);
	}
}

void ABossLola::InitializeLolaController()
{
	LolaController = Cast<ABossAIControllerBase>(GetController());
	if (LolaController)
	{
		UE_LOG(LogTemp, Warning, TEXT("Lola bound to the Lola Controller."))
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Lola not bound to the Lola Controller."))
	}
	
}

void ABossLola::PopulateDronesArray()
{
	//Get Actors from ChildActorComponents
	Drone1 = Cast<AFloaterDrones>(Drone1Component->GetChildActor());
	Drone2  = Cast<AFloaterDrones>(Drone2Component->GetChildActor());
	Drone3 = Cast<AFloaterDrones>(Drone3Component->GetChildActor());

	if(Drone1 && Drone2 && Drone3)
	{
		FloaterDronesArray.Add(Drone1);
		FloaterDronesArray.Add(Drone2);
		FloaterDronesArray.Add(Drone3);
	}

	//Binding the Drones to the Stun Status changes on Lola.
	if (FloaterDronesArray.Num() == 3)
	{
		for (AFloaterDrones* Drone : FloaterDronesArray)
		{
			Drone->BindToLolaStun(this);
		}
	}
}

void ABossLola::RandomizeDroneVulnerability()
{

	//Changing drones vulnerability randomly-ish

	//Dynamic Range
	int RandomNum = FMath::RandRange(0,FloaterDronesArray.Num() - 1);

	for (int i = 0; i < FloaterDronesArray.Num(); i++)
	{
		if (i == RandomNum)
		{
			FloaterDronesArray[i]->HandleChangeDamageState(EDroneState::Damageable);
		}
		else
		{
			FloaterDronesArray[i]->HandleChangeDamageState(EDroneState::NotDamageable);
		}
	}
}

void ABossLola::RemoveDroneFromArray(AFloaterDrones* Drone)
{
	if (FloaterDronesArray.Contains(Drone))
	{
		FloaterDronesArray.Remove(Drone);
	}
}

void ABossLola::SetDronesToNotDamageableDuringStun()
{
	for (int i = 0; i < FloaterDronesArray.Num(); i++)
	{
		FloaterDronesArray[i]->HandleChangeDamageState(EDroneState::NotDamageable);

		//Also Resetting Health
		FloaterDronesArray[i]->CurrentHealth = FloaterDronesArray[i]->MaxHealth;
	}
}

void ABossLola::LolaInitializeComponents()
{
	HeadCollisionComponent = CreateDefaultSubobject<UCapsuleComponent>("HeadCollisionComponent");
	LeftArmCollisionComponent = CreateDefaultSubobject<UCapsuleComponent>("LeftArmCollisionComponent");
	LeftArmCollisionComponent->SetupAttachment(RootComponent);
	RightArmCollisionComponent = CreateDefaultSubobject<UCapsuleComponent>("RightArmCollisionComponent");
	RightArmCollisionComponent->SetupAttachment(RootComponent);

	

	/* Minions / Actor Components Set on BP */
	Drone1Component = CreateDefaultSubobject<UChildActorComponent>("Drone1");
	Drone2Component = CreateDefaultSubobject<UChildActorComponent>("Drone2");
	Drone3Component = CreateDefaultSubobject<UChildActorComponent>("Drone3");
	Drone1Component->SetupAttachment(RootComponent);
	Drone2Component->SetupAttachment(RootComponent);
	Drone3Component->SetupAttachment(RootComponent);
	
}

void ABossLola::HandleDroneDeath(AFloaterDrones* Drone)
{
	//Change State
	SetLolaToStunned(Drone);
	//While Stunned Lola Is Damageable for a period of Time
	SetLolaToDamageable();
	//Remove Drone from Array
	RemoveDroneFromArray(Drone);
	//Pause the Timer for Randomizing Drone Vulnerability
	GetWorld()->GetTimerManager().PauseTimer(RandomizeDroneVulnerability_Handle);

	SetDronesToNotDamageableDuringStun();

	//Set Timer for Lola to be un-stunned
	/*GetWorld()->GetTimerManager().SetTimer(LolaStunnedTimer_Handle, this, &ABossLola::SetLolaToNotStunned, LolaStunTime, 
	false);*/
}

void ABossLola::SetLolaToStunned(AFloaterDrones* Drone)
{
	
	LolaState = ELolaState::Stunned;

	//Setting a BB variable when Stunned
	if (LolaController->BlackboardComponent)
	{
		LolaController->BlackboardComponent->SetValueAsBool("bIsLolaStunned", true);
	}

	//Broadcast to Drones that Lola is Stunned. Sets bool on Drones to not shoot, used in Custom Behavior Task C++
	OnLolaStunChange.Broadcast(true);
	
}

void ABossLola::DemolishBuildable()
{
	if (SweepHitActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Lola - Sweep Hit Actor Found. Demolishing Buildable."));
		ABuildableBase* Buildable = Cast<ABuildableBase>(SweepHitActor);
		float BuildableHealth = Buildable->ComponentDurability;
		if (ATimberBuildingComponentBase* WallorFloor = Cast<ATimberBuildingComponentBase>(SweepHitActor))
		{
			WallorFloor->BuildingComponentTakeDamage(BuildableHealth, this);
			SweepHitActor = nullptr;
			UE_LOG(LogTemp, Warning, TEXT("Lola - Buildable Destroyed. Sweep Hit Cleared."));
		}
		else if (ARampBase* Ramp = Cast<ARampBase>(SweepHitActor))
		{
			//Ramps dont have a specific TakeDamage function yet. Lola will just destroy it outright.
			//Ramps dont have parnts, butare considered Attachments?
			//TODO:: Implement a DamageableBuildable Interface and have Ramp Inherit from it and use the BuildableTakeDamage() function.
			Ramp->Destroy();
		}
	}
}

void ABossLola::SetLolaToNotStunned()
{
	//This function is called at the end of the Stun Animation, making the length of the Stun the Length of the Animation.
	if (FloaterDronesArray.Num() == 0)
	{
		SetLolaToDamageable();	
	}else
	{
		SetLolaNotDamageable();
	}
	
	LolaState = ELolaState::NotStunned;
	
	//Broadcast to Drones that Lola is not Stunned. Sets bool on Drones to shoot, used in Custom Behavior Task C++
	OnLolaStunChange.Broadcast(false);
	
	if (LolaController->BlackboardComponent)
	{
		LolaController->BlackboardComponent->SetValueAsBool("bIsLolaStunned", false);
	}
	//Randomize Drone Vulnerability - Forced
	RandomizeDroneVulnerability();

	//Unpause the Timer for Randomizing Drone Vulnerability
	GetWorld()->GetTimerManager().UnPauseTimer(RandomizeDroneVulnerability_Handle);
}


