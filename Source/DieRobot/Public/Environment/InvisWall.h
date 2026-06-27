// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"

#include "InvisWall.generated.h"

UCLASS()
class DIEROBOT_API AInvisWall : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInvisWall();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* RootSceneComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* WallStaticMesh;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
