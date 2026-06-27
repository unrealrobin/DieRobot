// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GarageDoorBase.generated.h"

class UBoxComponent;

UCLASS()
class DIEROBOT_API AGarageDoorBase : public AActor
{
	GENERATED_BODY()

public:
	
	// Sets default values for this actor's properties
	AGarageDoorBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Garage")
	UStaticMeshComponent* GarageFrameStaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Garage")
	UStaticMeshComponent* GarageDoorStaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Garage")
	UBoxComponent* GarageDoorBoxComponent;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetupGarageComponents();

	/*Open Close Timeline */

	UFUNCTION(BlueprintImplementableEvent)
	void PlayGarageOpenTimeline();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayGarageCloseTimeline();

	UFUNCTION(BlueprintCallable)
	void OpenGarageDoor();

	UFUNCTION(BlueprintCallable)
	void CloseGarageDoor();
};
