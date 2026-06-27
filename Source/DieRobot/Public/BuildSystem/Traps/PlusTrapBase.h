// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "TrapBase.h"
#include "PlusTrapBase.generated.h"

UCLASS()
class DIEROBOT_API APlusTrapBase : public ATrapBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APlusTrapBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap Components")
	float MaxHitBoxLength = 750.f;

	//Z
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Box Data")
	float BoxHeight = 170.f;

	//X
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Box Data")
	float BoxWidth = 170.f;

	//y
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Box Data")
	float BoxLength = 400.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Box Data")
	float CalculatedBoxLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap Components")
	USceneComponent* BoxExtentRaycastStart;
	
	UFUNCTION()
	void RaycastForHitBoxLength();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Raycast")
	TArray<FHitResult> HitResults;
};
