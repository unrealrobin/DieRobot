// Property of Paracosm Industries.


#pragma once

#include "CoreMinimal.h"
#include "ConstructBase.h"
#include "PowerPlate.generated.h"

class UBoxComponent;
class ATimberBuildingComponentBase;

UCLASS()
class DIEROBOT_API APowerPlate : public AConstructBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APowerPlate();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* PlateCenter;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UMaterialInstanceDynamic* MaterialInstanceDynamic;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	void FreeUpTrapSlotOnBuildingComponent();

	virtual void HandleDeletionOfBuildable() override;
	
	void HandlePowerPlateMaterialChange(bool bShouldGlow);

	UFUNCTION()
	virtual void HitBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void HitBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
