// Property of Paracosm Industries. 

#pragma once

#include "CoreMinimal.h"
#include "BuildSystem/BuildableBase.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Amplifiable.h"
#include "Interfaces/Interactable.h"
#include "TrapBase.generated.h"

class UStatusEffectDefinition;
class UStatusConditionManager;
class UStatusEffectBase;
class ATimberEnemyCharacter;
class ATimberBuildingComponentBase;
class UBoxComponent;

UENUM(BlueprintType)
enum class EAmplificationState : uint8
{
	Amplified UMETA(DisplayName = "Amplified"),
	NotAmplified UMETA(DisplayName = "Not Amplified")
};

UENUM(BlueprintType)
enum class EBuildingComponentTrapDirection : uint8
{
	Front UMETA(DisplayName = "Front Snap"),
	Back UMETA(DisplayName = "Back Snap"),
	Default UMETA(DisplayName = "Default")
};

UCLASS()
class DIEROBOT_API ATrapBase : public ABuildableBase, public IAmplifiable
{
	GENERATED_BODY()

public:
	ATrapBase();
	
	void ConfigureStaticMeshWalkableSlope(AActor* ParentBuildableRef);

	/* v1 */
	//TODO: To be DELETED. Using StatusEffectDefinitions Array. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	UStatusEffectBase* StatusEffectDataAsset;

	/* v2 */
	//Processes whether or not an Effect should Resolve and be applied on the Enemy.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status Effects")
	UStatusConditionManager* EffectConditionManager;

	//[Minor, Major, Ultimate] Order is Important Here!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	TArray<UStatusEffectDefinition*> StatusEffectDefinitions;

protected:
	virtual void BeginPlay() override;

	void DisableAllStaticMeshCollisions(UStaticMeshComponent* SomeMesh);
	
	UPROPERTY(VisibleAnywhere, Category="Hit Enemies")
	TArray<TWeakObjectPtr<AActor>> InsideHitBoxArray;

public:
	virtual void Tick(float DeltaTime) override;

	/* Amplify Prototype*/
	virtual void SetIsAmplified(bool bIsAmplified) override;
	
	void FreeUpTrapSlotOnBuildingComponent();

	//Used for Independent Deletion. When player deletes a specific Trap.
	virtual void HandleDeletionOfBuildable() override;
	
	//Used for deletion when the Parent Building Component is deleted.
	void HandleDeletionByBuildingComponent();

	/*Delegates*/
	/*DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrapFinalizationChange, bool, CanTrapBeFinalized);
	FOnTrapFinalizationChange OnTrapFinalizationChange;*/
	
	/* Components */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trap Components")
	UStaticMeshComponent* TrapBaseStaticMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trap Components")
	UBoxComponent* HitBoxComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateBrush BuildingComponentIconImage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trap Components")
	USceneComponent* TrapCenterSnapLocation;
	
	/* Placement Utilities */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building Component")
	ATimberBuildingComponentBase* TrapHoveredBuildingComponent = nullptr;

	//TODO:: Work on Renaming this to be more General to Center Snap Buildable. 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap Components")
	EBuildingComponentTrapDirection BuildingComponentTrapDirection = EBuildingComponentTrapDirection::Default;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Amplification")
	EAmplificationState AmplificationState = EAmplificationState::NotAmplified;
	
	
	/*Hit Area Utilities*/
	/* Adds Enemies to the Inside Hit Box Array*/
	UFUNCTION()
	virtual void HitBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	/* Removes Enemies from the Inside Hit Box Array*/
	UFUNCTION()
	virtual void HitBoxEndOverlap(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void AddEnemyToInsideHitBoxArray(AActor* Enemy);
	void RemoveEnemyFromInsideHitBoxArray(AActor* Enemy);

	/*
	 * InsideHitBoxArray now stores TWeakObjPtrs
	 * If an Actor dies while still in the HitBox we need a Way to clean up the HitBox Array of Nullptrs because
	 * they never hit the EndOverlap function
	 * This handles that clean up on a timer with an offset to spread the clean up.
	 */
	void ClearDeadEnemiesFromArray();
};
