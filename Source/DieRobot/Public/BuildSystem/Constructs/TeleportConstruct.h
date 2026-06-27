// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "ConstructBase.h"
#include "TeleportConstruct.generated.h"

class ADieRobotPlayableCharacter;
class UBoxComponent;

UENUM(BlueprintType) // Allows usage in Blueprints
enum class ETeleportConstructState : uint8
{
	Proxy UMETA(DisplayName = "Proxy"), // Enum value with display name
	Temporary UMETA(DisplayName = "Temporary"),
	Final UMETA(DisplayName = "Final"),
	Default UMETA(DisplayName = "Default")
	
};

class UBoxComponent;

UCLASS()
class DIEROBOT_API ATeleportConstruct : public AConstructBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATeleportConstruct();

	void SetupComponents();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	ETeleportConstructState TeleportConstructState = ETeleportConstructState::Default;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UBoxComponent* OverlapBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* TeleportMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* TeleportEffectMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* TeleportLandingLocation;

	UFUNCTION()
	void HandleTeleportOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void HandleTeleportHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UFUNCTION()
	void SaveDefaultMaterials();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Materials")
	UMaterialInterface* DefaultBaseMaterial = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Materials")
	UMaterialInterface* DefaultEffectMesh = nullptr;

	/*Teleport Details*/
	FVector StartLocation;
	FVector TargetLocation;
	float TeleportDuration;
	float TeleportTimeElapsed;
	FTimerHandle TeleportTimerHandle;
	UPROPERTY()
	ADieRobotPlayableCharacter* PlayerToTeleport = nullptr;
	UFUNCTION()
	void StartTeleport(FVector TargetDestination, float DurationToTeleport);
	UFUNCTION()
	void UpdateTeleport();



public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	ATeleportConstruct* TeleportPair = nullptr;
	
	UFUNCTION()
	void ApplyDefaultMaterials();

	UFUNCTION()
	void CheckForPairOutsideBuildMode();

	/*Handling Pair Destruction*/
	UFUNCTION()
	void LinkToPair(ATeleportConstruct* Pair);
	
	UFUNCTION()
	void OnPairDestruction(AActor* DestroyedActor);
	
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SpawnLoot();

private:
	UPROPERTY()
	ADieRobotPlayableCharacter* PlayerCharacter = nullptr;
};
