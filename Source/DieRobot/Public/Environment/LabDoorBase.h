// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "LabDoorBase.generated.h"

struct FTimeline;
class ATimberEnemyCharacter;
class UBoxComponent;

UCLASS()
class DIEROBOT_API ALabDoorBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALabDoorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Closed Position for Doors.
	FVector LabDoorLeftClosePos;
	FVector LabDoorRightClosePos;

	//Timers for Testing Open and Closing of Lab Doors
	FTimerHandle LabDoorOpenTimerHandle;
	FTimerHandle LabDoorCloseTimerHandle;

	UPROPERTY()
	UTimelineComponent* DoorOpenTimeline;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation")
	UCurveFloat* FloatCurve;
	UFUNCTION()
	void HandleTimelineProgress(float Value);
	UFUNCTION()
	void HandleTimelineFinished();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Tracer")
	USceneComponent* PathTracerStart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lab Door Meshes")
	UStaticMeshComponent* LabDoorFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lab Door Meshes")
	UStaticMeshComponent* LabDoorLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lab Door Meshes")
	UStaticMeshComponent* LabDoorRight;

	UFUNCTION(BlueprintCallable)
	void OpenLabDoor(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void CloseLabDoor(float DeltaTime);
	
	void SetupLabDoorComponents();
};
