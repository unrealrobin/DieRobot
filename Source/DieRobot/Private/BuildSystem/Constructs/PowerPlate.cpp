// Property of Paracosm Industries. 


#include "BuildSystem/Constructs/PowerPlate.h"
#include "BuildSystem/BuildingComponents/TimberBuildingComponentBase.h"
#include "Character/TimberPlayableCharacter.h"
#include "Components/BoxComponent.h"

// Sets default values
APowerPlate::APowerPlate()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SnapCondition = ESnapCondition::FloorCenterSnapTopOnly;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMeshComponent->SetupAttachment(RootComponent);
	StaticMeshComponent->SetCollisionProfileName(TEXT("DR_BuildableBlockEverything"));
	CollisionBox = CreateDefaultSubobject<UBoxComponent>("CollisionBox");
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetCollisionProfileName(TEXT("DR_HitEventOnly"));
	PlateCenter = CreateDefaultSubobject<UStaticMeshComponent>("Plate Center");
	PlateCenter->SetupAttachment(StaticMeshComponent);
	PlateCenter->SetCollisionProfileName(TEXT("DR_BuildableBlockEverything"));
}

// Called when the game starts or when spawned

void APowerPlate::FreeUpTrapSlotOnBuildingComponent()
{
	if (ATimberBuildingComponentBase* ParentBuildingComponent = Cast<ATimberBuildingComponentBase>(ParentBuildable))
	{
		ParentBuildingComponent->FrontCenterAttachment = nullptr;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find a stored Parent Building Component on PowerPlate. Trap Slot not freed."));
	}
}

void APowerPlate::HandleDeletionOfBuildable()
{
	FreeUpTrapSlotOnBuildingComponent();
	
	Super::HandleDeletionOfBuildable();
}

void APowerPlate::HandlePowerPlateMaterialChange(bool bShouldGlow)
{
	if (MaterialInstanceDynamic)
	{
		// Set the emissive properties
		FColor RedHexColor = FColor::FromHex("F84FA8FF");
		FLinearColor InactiveRedColor = RedHexColor.ReinterpretAsLinear();
		FColor BlueHexColor = FColor::FromHex("03FF9EFF");
		FLinearColor ActiveBlueLinear = BlueHexColor.ReinterpretAsLinear();
		FLinearColor EmissiveColor = bShouldGlow ? ActiveBlueLinear: InactiveRedColor;
		
		float EmissiveIntensity = bShouldGlow ? 5.0f : 0.5f;

		MaterialInstanceDynamic->SetScalarParameterValue("EmissiveIntensity", EmissiveIntensity);
		MaterialInstanceDynamic->SetVectorParameterValue("EmissiveColor", EmissiveColor);
	}
}

void APowerPlate::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &APowerPlate::HitBoxBeginOverlap);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &APowerPlate::HitBoxEndOverlap);

	MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(StaticMeshComponent->GetMaterial(0), this);
	if (MaterialInstanceDynamic)
	{
		UMaterialInterface* BaseMaterial = StaticMeshComponent->GetMaterial(0);
		if (!BaseMaterial) return;

		// Create and apply the dynamic instance
		MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		StaticMeshComponent->SetMaterial(0, MaterialInstanceDynamic);
		PlateCenter->SetMaterial(0, MaterialInstanceDynamic);

		//Initializing the Power Plate Material Color to Off.
		HandlePowerPlateMaterialChange(false);
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("Could not get a Valid Dynamic Material instance."));
	}
	
}

void APowerPlate::HitBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	/*Timeline Animations for PowerPad Movement in BP's*/
	ATimberPlayableCharacter* TimberCharacter = Cast<ATimberPlayableCharacter>(OtherActor);
	if (TimberCharacter)
	{
		//Adding 50% Damage Increase
		TimberCharacter->DamageModifierValue += .5f;
		//Power Plate Glow On
		HandlePowerPlateMaterialChange(true);
		//Gives the Character the PowerUp Overlay Material
		TimberCharacter->AddOverlayMaterialToCharacter(TimberCharacter->PowerUpOverlayMaterial, 0.0f);
	}
}

void APowerPlate::HitBoxEndOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ATimberPlayableCharacter* TimberCharacter = Cast<ATimberPlayableCharacter>(OtherActor);
	if (TimberCharacter)
	{
		//Removing 50% Damage Increase
		TimberCharacter->DamageModifierValue -= .5f;
		//Glow Off
		HandlePowerPlateMaterialChange(false);
		//Removes the PowerUp Overlay Material
		TimberCharacter->RemoveOverlayMaterialFromCharacter();
	}
}

