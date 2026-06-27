// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "BuildSystem/BuildableBase.h"
#include "BuildSystem/Traps/TrapBase.h"
#include "GameFramework/Actor.h"
#include "Styling/SlateBrush.h"
#include "TimberBuildingComponentBase.generated.h"

/*Forward Declarations*/
class UBoxComponent;

UENUM(BlueprintType)
enum class EBuildingComponentOrientation : uint8
{
	Vertical UMETA(DisplayName = "Vertical"),
	Horizontal UMETA(DisplayName = "Horizontal"),
	Default UMETA(DisplayName = "Default"),
};

UENUM(BlueprintType)
enum class EBuildingComponentType : uint8
{
	BasicWall UMETA(DisplayName = "BasicWall"),
	BasicFloorWall UMETA(DisplayName = "BasicFloor"),
	Environment UMETA(DisplayName = "Environment"),
	Default UMETA(DisplayName = "Default"),
};

UCLASS()
class DIEROBOT_API ATimberBuildingComponentBase : public ABuildableBase
{
	GENERATED_BODY()

public:
	
	ATimberBuildingComponentBase();

	//Checks overlaps with other buildables for Placement
	UFUNCTION()
	void HandleHitBuildingComponent(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	//Disables most collisions for proxy.
	void SetupProxyCollisionHandling();

	// Not Used currently.
	UFUNCTION()
	void HandleOverlappedBuildingComponent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void HandleEndOverlappedBuildingComponent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION()
	void BuildingComponentTakeDamage(float AmountOfDamage, AActor* DamagingActor);
	
	void CreateSnapPoints();
	void CreateQuadrantComponents();

	/*States*/
	UPROPERTY(VisibleAnywhere, Category="Building Component Info")
	EBuildingComponentOrientation BuildingOrientation = EBuildingComponentOrientation::Default;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Building Component Info")
	EBuildingComponentType BuildingComponentType = EBuildingComponentType::Default;
	
	virtual void HandleDeletionOfBuildable() override;

	UFUNCTION()
	void DeleteAllAttachments();
	
	UFUNCTION()
	void DestroyAllAttachments();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UStaticMeshComponent* StaticMesh;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Quadrants")
	UBoxComponent* TopQuadrant;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Quadrants")
	UBoxComponent* BottomQuadrant;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Quadrants")
	UBoxComponent* RightQuadrant;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Quadrants")
	UBoxComponent* LeftQuadrant;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Quadrants")
	UBoxComponent* CenterQuadrant;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Snap Locations")
	USceneComponent* TopSnap;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Snap Locations")
	USceneComponent* BottomSnap;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Snap Locations")
	USceneComponent* RightSnap;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Snap Locations")
	USceneComponent* LeftSnap;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Snap Locations")
	USceneComponent* CenterSnap;

	/* Trap/Construct Snap Locations & Slots */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Snap Locations")
	USceneComponent* FrontCenterSnapPoint;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Snap Locations")
	USceneComponent* BackCenterSnapPoint;

	//Tracks the Attached Traps Placed on the Walls/Floors
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Attached Buildables")
	ABuildableBase* FrontCenterAttachment = nullptr;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Attached Buildables")
	ABuildableBase* FrontTopAttachment = nullptr;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Attached Buildables")
	ABuildableBase* FrontBottomAttachment = nullptr;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Attached Buildables")
	ABuildableBase* FrontRightAttachment = nullptr;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Attached Buildables")
	ABuildableBase* FrontLeftAttachment = nullptr;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Attached Buildables")
	ABuildableBase* BackCenterAttachment = nullptr;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Attached Buildables")
	ABuildableBase* BackTopAttachment = nullptr;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Attached Buildables")
	ABuildableBase* BackBottomAttachment = nullptr;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Attached Buildables")
	ABuildableBase* BackRightAttachment = nullptr;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Attached Buildables")
	ABuildableBase* BackLeftAttachment = nullptr;

	/*Attached Traps/Constructs*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Attached Building Components")
	TArray<ABuildableBase*> AttachedBuildingComponents;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateBrush BuildingComponentIconImage;
	
};
