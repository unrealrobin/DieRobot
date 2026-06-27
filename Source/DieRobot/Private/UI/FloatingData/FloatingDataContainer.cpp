// Property of Paracosm.


#include "DieRobot/Public/UI/FloatingData/FloatingDataContainer.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/WidgetComponent.h"
#include "DieRobot/Public/UI/FloatingData/FloatingDataWidget.h"


// Sets default values
AFloatingDataContainer::AFloatingDataContainer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>("RootSceneComponent");
	RootComponent = RootSceneComponent;
	
	DamageNumberWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("DamageNumberWidgetComponent");
	DamageNumberWidgetComponent->SetupAttachment(RootComponent);
}

void AFloatingDataContainer::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle DestroyTimer;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimer, this, &AFloatingDataContainer::HandleDestroy, TimeUntilDestroy, false);
}

void AFloatingDataContainer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!SpawnSceneComponentRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Scene Component Ref."));
		return;
	}
	ElapsedTime += DeltaTime;

	//How much to Scale Z by.
	float Alpha = FMath::Clamp(ElapsedTime / TimeUntilDestroy, 0.0f, 1.0f);
	//Used as an effect for the ALpha
	float BounceAlpha = FMath::InterpEaseOut(0.f, 1.f, Alpha, 4.0f);
	//Setting of the Z Value
	float NewZLocation = FMath::Lerp(0, VerticalRiseAmount, BounceAlpha);

	FVector FinalAdjustedLocation = SpawnSceneComponentRef->GetComponentLocation() + FVector(0, 0, NewZLocation);

	SetActorLocation(FinalAdjustedLocation);
}

void AFloatingDataContainer::HandleDestroy()
{
	if (IsValid(OwningActor))
	{
		OwningActor->ClearFloatingDamageRef();
	}
	SpawnSceneComponentRef = nullptr;
	Destroy();
}

void AFloatingDataContainer::SetIsDamage(bool bIsThisDamage)
{
	if (DamageNumberWidgetComponent)
	{
		UUserWidget* DamageNumberWidget = DamageNumberWidgetComponent->GetWidget();
		if (UFloatingDataWidget* DamageNumberWidgetCast = Cast<UFloatingDataWidget>(DamageNumberWidget))
		{
			DamageNumberWidgetCast->bIsDamage = bIsThisDamage;
		}
	}
}

void AFloatingDataContainer::SetEffectText(FName EffectName)
{
	if (DamageNumberWidgetComponent)
	{
		UUserWidget* DamageNumberWidget = DamageNumberWidgetComponent->GetWidget();
		if (UFloatingDataWidget* DamageNumberWidgetCast = Cast<UFloatingDataWidget>(DamageNumberWidget))
		{
			DamageNumberWidgetCast->EffectName = EffectName;
		}
	}
}

void AFloatingDataContainer::SetDamageAmount(float InDamageAmount)
{
	if (DamageNumberWidgetComponent)
	{
		UUserWidget* DamageNumberWidget = DamageNumberWidgetComponent->GetWidget();
		if (UFloatingDataWidget* DamageNumberWidgetCast = Cast<UFloatingDataWidget>(DamageNumberWidget))
		{
			DamageNumberWidgetCast->DamageNumber += InDamageAmount; 
			//DamageNumberWidgetCast->DamageNumber = InDamageAmount;
		}
		
	}
}

void AFloatingDataContainer::SetColor(FLinearColor InDamageColor)
{
	if (DamageNumberWidgetComponent)
	{
		UUserWidget* DamageNumberWidget = DamageNumberWidgetComponent->GetWidget();
		if (UFloatingDataWidget* DamageNumberWidgetCast = Cast<UFloatingDataWidget>(DamageNumberWidget))
		{
			DamageNumberWidgetCast->DamageColor = InDamageColor;
		}
		
	}
}

void AFloatingDataContainer::SetSize(float InDamageSize)
{
	if (DamageNumberWidgetComponent)
	{
		UUserWidget* DamageNumberWidget = DamageNumberWidgetComponent->GetWidget();
		if (UFloatingDataWidget* DamageNumberWidgetCast = Cast<UFloatingDataWidget>(DamageNumberWidget))
		{
			DamageNumberWidgetCast->DamageSize = InDamageSize;
		}
		
	}
}





