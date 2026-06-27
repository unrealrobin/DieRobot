// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"

#include "LocationMarkerBase.generated.h"

UCLASS()
class DIEROBOT_API ALocationMarkerBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALocationMarkerBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* StaticMeshComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void HandleLocationOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
};




