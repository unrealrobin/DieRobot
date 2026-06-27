// Property of Paracosm Industries. Dont use my shit.


#include "Environment/GarageDoorBase.h"
#include "Components/BoxComponent.h"


// Sets default values
AGarageDoorBase::AGarageDoorBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetupGarageComponents();
}

// Called when the game starts or when spawned
void AGarageDoorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGarageDoorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGarageDoorBase::SetupGarageComponents()
{
	GarageFrameStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("GarageFrame");
	RootComponent = GarageFrameStaticMesh;

	GarageDoorStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("GarageDoor");
	GarageDoorStaticMesh->SetupAttachment(RootComponent);

	GarageDoorBoxComponent = CreateDefaultSubobject<UBoxComponent>("GarageDoorCollisionBox");
	GarageDoorBoxComponent->SetupAttachment(GarageDoorStaticMesh);
}

void AGarageDoorBase::OpenGarageDoor()
{
	PlayGarageOpenTimeline();
}

void AGarageDoorBase::CloseGarageDoor()
{
	PlayGarageCloseTimeline();
}