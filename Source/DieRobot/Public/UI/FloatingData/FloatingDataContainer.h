// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloatingDataContainer.generated.h"

class UWidgetComponent;
class ADieRobotEnemyCharacter;
UCLASS()
class DIEROBOT_API AFloatingDataContainer : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFloatingDataContainer();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene Component")
	USceneComponent* RootSceneComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Number")
	UWidgetComponent* DamageNumberWidgetComponent;
	
	UPROPERTY()
	TObjectPtr<USceneComponent> SpawnSceneComponentRef;

	UPROPERTY()
	ADieRobotEnemyCharacter* DataActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Number")
	float TimeUntilDestroy = 2.0f;
	float ElapsedTime = 0.0f;

	UPROPERTY()
	TObjectPtr<ADieRobotEnemyCharacter> OwningActor;

	void SetIsDamage(bool bIsThisDamage);
	void SetEffectText(FName EffectName);
	void SetDamageAmount(float InDamageAmount);
	void SetColor(FLinearColor InDamageColor);
	void SetSize(float InDamageSize);
	
protected:
	void HandleDestroy();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	FVector StartLocation = FVector::ZeroVector;
	
	FVector EndLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Number")
	float VerticalRiseAmount = 100.0f;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
