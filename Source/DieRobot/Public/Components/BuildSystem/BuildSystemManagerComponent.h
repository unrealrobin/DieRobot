// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "BuildSystem/Traps/TrapBase.h"
#include "Components/ActorComponent.h"
#include "BuildSystemManagerComponent.generated.h"

enum class EBuildingComponentOrientation : uint8;
enum class EBuildingComponentTrapDirection : uint8;
class ATeleportConstruct;
class ARampBase;
class ATrapBase;
class ABuildableBase;
class ATimberBuildingComponentBase;

/*
 * Struct used for Trap Placement auto orientation.
 */
USTRUCT(BlueprintType)
struct FBuildablePlacementData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trap Snap Data")
	FVector TrapLocation;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Trup Snap Data")
	FRotator TrapRotation;
	
	FBuildablePlacementData()
		: TrapLocation(FVector::ZeroVector), TrapRotation(FRotator::ZeroRotator)
	{
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DIEROBOT_API UBuildSystemManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuildSystemManagerComponent();

	/*
	 * Called when a Buildable is Spawned, Makes the Game Mode Redraw Possible Paths from Doors to Data Seed
	 */	 
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRedrawPathTraceHandle);
	FRedrawPathTraceHandle RedrawPathTraceHandle;

protected:
	virtual void BeginPlay() override;

	/*
	 * Class to be spawned with the Spawn function - Set by Selection in Building Component Icon Event Graph
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buildable")
	TSubclassOf<ABuildableBase> ActiveBuildableComponentClass;

	/*
	 * A Reference to the Proxy Buildable (See-through during spawn)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buildable")
	ABuildableBase* BuildableProxyInstance = nullptr;

	/*Grid Snap Option - Currently Unused.*/
	FVector SnapToGrid(FVector RaycastLocation);
	FRotator SnapToRotation(FRotator CharactersRotation);
	FRotator SavedRotation = FRotator::ZeroRotator;
	int GridSize = 100.f;

	/*
	 *Structure Snapping (Walls & Floors)
	 * Using Quadrants and Scene Components to align Walls and floors. This is the other option to Grid Snapping.
	 * These functions are used to determine how a wall attaches to a floor depending on whcih quadrant is being hovered over.
	 */
	int SnappingCondition(EBuildingComponentOrientation Orientation1, EBuildingComponentOrientation Orientation2);
	int QuadrantCondition(FString QuadrantName);
	UFUNCTION()
	void SameOrientationSnapCondition(FHitResult HitResult);
	UFUNCTION()
	void VerticalToHorizontalSnapCondition(FHitResult HitResult);
	UFUNCTION()
	void HorizontalToVerticalSnapCondition(FHitResult HitResult);
	UFUNCTION()
	void MoveBuildingComponentProxyToSnapLocation(FVector ProxySnapLocation, FVector SnapLocation);
	
	UFUNCTION()
	void RotateProxyToSnapRotation(FRotator HitActorRotation, ABuildableBase* BuildingComponent);
	
	/*Static Mesh Utilities*/
	UPROPERTY(VisibleAnywhere, Category="Building Component")
	TArray<UStaticMeshComponent*> StaticMeshs;
	
	/*
	 *Applying the dynamic material to the mesh that makes it see-through OR when placement is invalid (blue or red). Yellow is temp placement
	 * used for teleporters that require 2 placements to be completed.
	 */
	UFUNCTION()
	void MakeMaterialHoloColor(AActor* BuildingComponentActor, UMaterial* HoloMaterialColor);
	UFUNCTION()
	void HandleTrapMaterialChange(bool bCanTrapBeFinalized);
	void GetStaticMeshComponents(AActor* BuildingComponentActor);
	void AddToBuildableAttachments(ABuildableBase* AttachingBuildable);
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Building Component")
	UMaterial* RedHoloMaterial = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Building Component")
	UMaterial* YellowHoloMaterial = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Building Component")
	UMaterial* BlueHoloMaterial = nullptr;

	/*Adjusting Proxy Collisions so that it doesn't follow the standard collision profile. */
	void DisableBuildableProxyCollisions(ABuildableBase* BuildingComponent);

	/*SFX*/
	void PlayBuildablePlacementSound();
	void PlayBuildDeniedSound();

public:
	/* Spawn functions for the different types of Placement options.*/
	UFUNCTION()
	void SpawnFinalBuildable();

	/* Walls & Floors. */
	void SpawnFinalBuildingComponent(FActorSpawnParameters SpawnParameters);

	/* Spawns Traps and some constructs */
	void SpawnFinalCenterSnapBuildable(FActorSpawnParameters SpawnParameters);

	/* Spawns Constructs that can't be placed on ceiling. Ex. Power Plate*/
	void SpawnFinalFloorCenterSnapTopOnlyBuildable(FActorSpawnParameters SpawnParameters);

	/* Used for Ramps/Stairs */
	void SpawnFinalRampBuildable(FActorSpawnParameters SpawnParameters);

	/* Traps/Structures/Constructs that must be spawned on the edge of a floor. */
	void SpawnFinalFloorEdgeSnapTopOnlyBuildable(FActorSpawnParameters SpawnParameters);

	/* Additional spawn logic for teleporters due to requiring 2 placements. */
	void HandleIsTeleporter(ATeleportConstruct* TeleportConstruct);
	
	/* Spawns the proxy Buildable after selecting the options from the Build Menu*/
	UFUNCTION()
	AActor* SpawnProxy(TSubclassOf<ABuildableBase> ActiveBuildableClass, FVector SpawnLocation, FRotator SpawnRotation);

	/* Returns the first Valid Hit Buildable used for Snapping. */
	UFUNCTION()
	FHitResult FirstHitBuildingComponentHitResult(TArray<FHitResult> HitResults);

	/* Called Every Tick while Ray-casting during Build. Determines which placement logic to follow based on Buildable Type */
	UFUNCTION()
	void HandleProxyPlacement(TArray<FHitResult> HitResults, TSubclassOf<ABuildableBase> BuildableProxyClass);

	/*
	 * The Following are specific placement type logics for each Type.
	 */
	UFUNCTION()
	void HandleBuildingComponentPlacement(FHitResult FirstHitBuildingComponentHitResult);
	UFUNCTION()
	void HandleCenterSnapPlacement(FHitResult FirstHitBuildingComponentHitResult);
	UFUNCTION()
	void HandleFloorCenterSnapTopOnlyPlacement(FHitResult FirstHitBuildingComponentHitResult);
	UFUNCTION()
	void HandleRampPlacement(FHitResult FirstHitBuildingComponentHitResult);
	UFUNCTION()
	void HandleFloorEdgeSnapTopOnlyPlacement(FHitResult FirstHitBuildingComponentHitResult);
	UFUNCTION()
	void HandleBuildingComponentSnapping(FHitResult HitResult);

	/* Moves the Buildable during Snapping */
	void MoveBuildable(
		FVector_NetQuantize Location, ABuildableBase* BuildingComponent, const FRotator& Rotation
			= FRotator::ZeroRotator);

	/* Buildables like Floors, Walls and Ramps can be Rotated.*/
	void RotateBuildingComponent();

	/* Marks the buildable as able to be finalized and Spawned.*/
	void MakeBuildableFinalizable(ABuildableBase* Buildable);

	/*
	 * Marks the buildable as not able to be finalized and Spawned.
	 * Can be due to incorrect placement or not enough resources/currency.
	 */
	void MakeBuildableNotFinalizable(ABuildableBase* Buildable);
	
	/*Used for Trap Placement*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trap Placement")
	bool bCanTrapsSnapToEnvironment = false;

	/* Empties saved Static Mesh Components from Array used for Material Updating*/
	UFUNCTION()
	FORCEINLINE void ClearStoredStaticMeshes() { StaticMeshs.Empty(); };
	
	/* Clean up */
	void CleanUpBuildSystemManagerComponent();
	void ResetBuildableComponents();
	void RemoveBuildingComponentProxies_All();
	void ClearTeleporterTPair();
	
	/*Building Component Placement Variables - Used with Grid Snapping Option*/
	FVector FinalSpawnLocation;
	FRotator FinalSpawnRotation;

	/*
	 *Used to remember the previous Rotation of the Building Component after Leaving Build Mode.
	 */
	void SetSavedRotation(FRotator Rotation) { SavedRotation = Rotation; };

	/*Getters & Setters*/
	FORCEINLINE TSubclassOf<ABuildableBase> GetActiveBuildableClass() { return ActiveBuildableComponentClass; } ;
	
	UFUNCTION(BlueprintCallable, Category="Building Component")
	void SetActiveBuildingComponentClass(TSubclassOf<AActor> BuildingComponentClass);
	
private:
	
	/*Teleport Utils*/
	TPair<ATeleportConstruct*, ATeleportConstruct*> TeleportTempPair;

	/*Component Snapping */
	EBuildingComponentOrientation CheckClassBuildingComponentOrientation(AActor* ClassToBeChecked);

	/* Used to create Struct required for Trap Transformation */
	FBuildablePlacementData GetTrapSnapTransform(
		FVector ImpactPoint, ATimberBuildingComponentBase*
		BuildingComponent, ATrapBase* TrapComponentProxy);

	/* Returns the Closest Snap point to the Hit Result on the Hovered Buildable*/
	USceneComponent* GetClosestFaceSnapPoint(FHitResult HitResult);

	void BroadcastControllerUpdateNewBuildable(AActor* Buildable);

	/* Used to send to Mission Event System Build Events */
	void SendBuildEventPayload(TObjectPtr<AActor> Buildable);

	/* Generates the Payload Required for the Mission System*/
	void GenerateBuildEventsContextTags(FGameplayTagContainer& TagContainer, const TObjectPtr<AActor>& Buildable);

	/* Tutorial Data - Unused */
	UPROPERTY()
	float TutorialBuildsPlaced = 0;

	/* Used for walking on tops of walls/edges. */
	//void HandleWalkableSlopeOverride(ATrapBase* FinalizedTrap);
};
