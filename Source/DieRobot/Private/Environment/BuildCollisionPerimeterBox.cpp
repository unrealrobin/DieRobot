// Property of Paracosm.


#include "Environment/BuildCollisionPerimeterBox.h"

#include "BuildSystem/BuildableBase.h"
#include "Components/BoxComponent.h"


// Sets default values
ABuildCollisionPerimeterBox::ABuildCollisionPerimeterBox()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PerimeterCollisionBox = CreateDefaultSubobject<UBoxComponent>("CollisionBoxComponent");
	RootComponent = PerimeterCollisionBox;
}

// Called when the game starts or when spawned
void ABuildCollisionPerimeterBox::BeginPlay()
{
	Super::BeginPlay();

	if (PerimeterCollisionBox)
	{
		PerimeterCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ABuildCollisionPerimeterBox::HandleOverlap);
		PerimeterCollisionBox->OnComponentEndOverlap.AddDynamic(this, &ABuildCollisionPerimeterBox::HandleEndOverlap);
	}
}

void ABuildCollisionPerimeterBox::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 INT32, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA(ABuildableBase::StaticClass()))
	{
		TObjectPtr<ABuildableBase> Buildable = Cast<ABuildableBase>(OtherActor);
		if (IsValid(Buildable) && OtherComp->IsA(UStaticMeshComponent::StaticClass()))
		{
			Buildable->bIsOverlappingPerimeter = true;
			//UE_LOG(LogTemp, Warning, TEXT("Buildable overlapping Perimeter."));
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("Buildable is not valid!"));
		}
	}
}

void ABuildCollisionPerimeterBox::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA(ABuildableBase::StaticClass()))
	{
		TObjectPtr<ABuildableBase> Buildable = Cast<ABuildableBase>(OtherActor);
		if (IsValid(Buildable) && Buildable->bIsOverlappingPerimeter == true)
		{
			Buildable->bIsOverlappingPerimeter = false;
			//UE_LOG(LogTemp, Warning, TEXT("Buildable NOT overlapping Perimeter."));
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("Buildable is not valid!"));
		}
	}
}

