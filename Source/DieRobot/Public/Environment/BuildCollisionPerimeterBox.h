// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildCollisionPerimeterBox.generated.h"

class UBoxComponent;

UCLASS()
class DIEROBOT_API ABuildCollisionPerimeterBox : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABuildCollisionPerimeterBox();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UBoxComponent> PerimeterCollisionBox;

protected:
	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 INT32, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
