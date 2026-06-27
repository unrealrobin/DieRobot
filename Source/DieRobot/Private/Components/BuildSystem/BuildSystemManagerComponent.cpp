// Property of Paracosm Industries.


#include "Components/BuildSystem/BuildSystemManagerComponent.h"
#include "BuildSystem/BuildingComponents/TimberBuildingComponentBase.h"
#include "BuildSystem/BuildingComponents/TimberHorizontalBuildingComponent.h"
#include "BuildSystem/BuildingComponents/TimberVerticalBuildingComponent.h"
#include "BuildSystem/Traps/TrapBase.h"
#include "BuildSystem/BuildableBase.h"
#include "BuildSystem/Constructs/PowerPlate.h"
#include "BuildSystem/Constructs/TeleportConstruct.h"
#include "BuildSystem/Ramps/RampBase.h"
#include "Character/TimberPlayableCharacter.h"
#include "Character/TimberSeeda.h"
#include "Components/BoxComponent.h"
#include "Components/Inventory/InventoryManagerComponent.h"
#include "Controller/TimberPlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "States/DieRobotGameStateBase.h"
#include "Subsystems/Events/MissionEventSubsystem.h"
#include "Subsystems/SFX/SFXManagerSubsystem.h"
#include "Types/MissionEventPayloads/MissionEventPayload.h"

UBuildSystemManagerComponent::UBuildSystemManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBuildSystemManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuildSystemManagerComponent::HandleBuildingComponentSnapping(FHitResult HitResult)
{
	if (ATimberBuildingComponentBase* BuildingComponentProxy = Cast<ATimberBuildingComponentBase>(BuildableProxyInstance))
	{
		if (HitResult.GetActor()->IsA(ATimberBuildingComponentBase::StaticClass()) && HitResult.GetComponent())
		{
			//Gets Proxy Building Component Enum Orientation (Horizontal/Vertical)(Wall/Floor)
			const EBuildingComponentOrientation ProxyBuildingComponentOrientation = BuildingComponentProxy->BuildingOrientation;
			
			// Get HitResult Building Component Enum Orientation (Horizontal/Vertical)(Wall/Floor)
			const EBuildingComponentOrientation PlacedBuildingComponentOrientation = CheckClassBuildingComponentOrientation(HitResult.GetActor());

			// Returns the condition to snap based on Orientation
			int SnappingConditionNum = SnappingCondition(ProxyBuildingComponentOrientation, PlacedBuildingComponentOrientation);
			
			switch (SnappingConditionNum)
			{
			case 1:
				SameOrientationSnapCondition(HitResult); //Wall Proxy to Wall
				//UE_LOG(LogTemp, Warning, TEXT("Building Component Snap Condition 1."));
				break;
			case 2:
				SameOrientationSnapCondition(HitResult); // Floor Proxy to Floor
				//UE_LOG(LogTemp, Warning, TEXT("Building Component Snap Condition 2."));
				break;
			case 3:
				VerticalToHorizontalSnapCondition(HitResult); //Wall Proxy to Floor
				//UE_LOG(LogTemp, Warning, TEXT("Building Component Snap Condition 3."));
				break;
			case 4:
				//Likely Unnecessary as Case 3 handles the same condition.
				HorizontalToVerticalSnapCondition(HitResult); //Floor Proxy to Wall
				//UE_LOG(LogTemp, Warning, TEXT("Building Component Snap Condition 4."));
				break;
			default:
				//UE_LOG(LogTemp, Error, TEXT("Error handling Orientation Condition Check."));
				break;
			}
		}
	}
}

int UBuildSystemManagerComponent::SnappingCondition(EBuildingComponentOrientation Orientation1, EBuildingComponentOrientation Orientation2)
{
	if (Orientation1 == Orientation2 && Orientation1 == EBuildingComponentOrientation::Vertical)
	{
		//Both Vertical
		return 1;
	}

	if (Orientation1 == Orientation2 && Orientation1 == EBuildingComponentOrientation::Horizontal)
	{
		//Both Horizontal
		return 2;
	}

	if (Orientation1 != Orientation2 && Orientation1 == EBuildingComponentOrientation::Vertical)
	{
		//Orientation 1 = Vertical, Orientation 2 = Horizontal
		return 3;
	}

	//TODO:: Possibly not necessary.
	if (Orientation1 != Orientation2 && Orientation1 == EBuildingComponentOrientation::Horizontal)
	{
		//Orientation 1 = Horizontal, Orientation 2 = Vertical
		return 3;
	}

	return 4;
}

int UBuildSystemManagerComponent::QuadrantCondition(FString QuadrantName)
{
	//Returns a case for the Overlapped Quadrant of the Raycast 
	if (QuadrantName == FString("TopQuadrant"))
	{
		return 1;
	}
	if (QuadrantName == FString("RightQuadrant"))
	{
		return 2;
	}
	if (QuadrantName == FString("BottomQuadrant"))
	{
		return 3;
	}
	if (QuadrantName == FString("LeftQuadrant"))
	{
		return 4;
	}
	if (QuadrantName == FString("CenterQuadrant"))
	{
		return 5;
	}

	return 6;
}

void UBuildSystemManagerComponent::SameOrientationSnapCondition(FHitResult HitResult)
{
	FVector ProxySnapLocation;
	FVector HitActorSnapLocation;

	ATimberBuildingComponentBase* ActiveBuildingComponentProxy = Cast<ATimberBuildingComponentBase>(BuildableProxyInstance);

	/*Returns in int based on the Name of the Quadrant
	 * "TopQuadrant" = 1
	 * "RightQuadrant" = 2
	 * "BottomQuadrant" = 3
	 * "LeftQuadrant" = 4
	 * "CenterQuadrant" = 5
	 */
	int QuadrantConditionNum = QuadrantCondition(HitResult.GetComponent()->GetName());
	
	switch (QuadrantConditionNum)
	{
	case 1: //Top
		{
			HitActorSnapLocation = Cast<ATimberBuildingComponentBase>(HitResult.GetActor())->TopSnap->GetComponentTransform().GetLocation();
			ProxySnapLocation = ActiveBuildingComponentProxy->BottomSnap->GetComponentTransform().GetLocation();
			MoveBuildingComponentProxyToSnapLocation(ProxySnapLocation, HitActorSnapLocation);
		}
		break;
	case 2: //Right
		{
			HitActorSnapLocation = Cast<ATimberBuildingComponentBase>(HitResult.GetActor())->RightSnap->GetComponentTransform().GetLocation();
			ProxySnapLocation = ActiveBuildingComponentProxy->LeftSnap->GetComponentTransform().GetLocation();
			MoveBuildingComponentProxyToSnapLocation(ProxySnapLocation, HitActorSnapLocation);
		}
		break;
	case 3: //Bottom
		{
			HitActorSnapLocation = Cast<ATimberBuildingComponentBase>(HitResult.GetActor())->BottomSnap->GetComponentTransform().GetLocation();
			ProxySnapLocation = ActiveBuildingComponentProxy->TopSnap->GetComponentTransform().GetLocation();
			MoveBuildingComponentProxyToSnapLocation(ProxySnapLocation, HitActorSnapLocation);
		}
		break;
	case 4: //Left
		{
			HitActorSnapLocation = Cast<ATimberBuildingComponentBase>(HitResult.GetActor())->LeftSnap->GetComponentTransform().GetLocation();
			ProxySnapLocation = ActiveBuildingComponentProxy->RightSnap->GetComponentTransform().GetLocation();
			MoveBuildingComponentProxyToSnapLocation(ProxySnapLocation, HitActorSnapLocation);
		}
		break;
	case 5: //Center
		{
			//TODO:: What should we do here? Center Quad.
		}
		break;
	default:
		{
			//UE_LOG(LogTemp, Error, TEXT("Quadrant Not Specified. "))
		}
		break;
	}
}

void UBuildSystemManagerComponent::VerticalToHorizontalSnapCondition(FHitResult HitResult)
{
	int QuadrantConditionNum = QuadrantCondition(HitResult.GetComponent()->GetName());
	FVector HitActorSnapLocation;
	//FRotator HitActorSnapRotation;
	FVector ProxySnapLocation;

	ATimberBuildingComponentBase* ActiveBuildingComponentProxy = Cast<ATimberBuildingComponentBase>(BuildableProxyInstance);
	switch (QuadrantConditionNum)
	{
		case 1:
			{
				//Bottom Vertical SnapPoint -> Top Quad on Horizontal
				HitActorSnapLocation = Cast<ATimberBuildingComponentBase>(HitResult.GetActor())->TopSnap->GetComponentTransform().GetLocation();
				//HitActorSnapRotation = Cast<ATimberBuildingComponentBase>(HitActor.GetActor())->TopSnap->GetComponentTransform().GetRotation().Rotator();
				ProxySnapLocation = ActiveBuildingComponentProxy->BottomSnap->GetComponentTransform().GetLocation();
				MoveBuildingComponentProxyToSnapLocation(ProxySnapLocation, HitActorSnapLocation);
			}
			break;
		case 2:
			{
				//Bottom Vertical SnapPoint -> Right Quad on Horizontal
				HitActorSnapLocation = Cast<ATimberBuildingComponentBase>(HitResult.GetActor())->RightSnap->GetComponentTransform().GetLocation();
				//HitActorSnapRotation = Cast<ATimberBuildingComponentBase>(HitActor.GetActor())->RightSnap->GetComponentTransform().GetRotation().Rotator();
				ProxySnapLocation = ActiveBuildingComponentProxy->BottomSnap->GetComponentTransform().GetLocation();
				MoveBuildingComponentProxyToSnapLocation(ProxySnapLocation, HitActorSnapLocation);
			}
			break;
		case 3:
			{
				//Bottom Vertical SnapPoint -> Bottom Quad on Horizontal
				HitActorSnapLocation = Cast<ATimberBuildingComponentBase>(HitResult.GetActor())->BottomSnap->GetComponentTransform().GetLocation();
				//HitActorSnapRotation = Cast<ATimberBuildingComponentBase>(HitActor.GetActor())->BottomSnap->GetComponentTransform().GetRotation().Rotator();
				ProxySnapLocation = ActiveBuildingComponentProxy->BottomSnap->GetComponentTransform().GetLocation();
				MoveBuildingComponentProxyToSnapLocation(ProxySnapLocation, HitActorSnapLocation);
			}
			break;
		case 4:
			{
				//Bottom Vertical SnapPoint -> Left Quad on Horizontal
				HitActorSnapLocation = Cast<ATimberBuildingComponentBase>(HitResult.GetActor())->LeftSnap->GetComponentTransform().GetLocation();
				//HitActorSnapRotation = Cast<ATimberBuildingComponentBase>(HitActor.GetActor())->LeftSnap->GetComponentTransform().GetRotation().Rotator();
				ProxySnapLocation = ActiveBuildingComponentProxy->BottomSnap->GetComponentTransform().GetLocation();
				MoveBuildingComponentProxyToSnapLocation(ProxySnapLocation, HitActorSnapLocation);
			}
			break;
		case 5:
			{
				//Bottom Vertical SnapPoint -> Center Quad on Horizontal
				HitActorSnapLocation = Cast<ATimberBuildingComponentBase>(HitResult.GetActor())->CenterSnap->GetComponentTransform().GetLocation();
				//HitActorSnapRotation = Cast<ATimberBuildingComponentBase>(HitActor.GetActor())->CenterSnap->GetComponentTransform().GetRotation().Rotator();
				ProxySnapLocation = ActiveBuildingComponentProxy->BottomSnap->GetComponentTransform().GetLocation();
				MoveBuildingComponentProxyToSnapLocation(ProxySnapLocation, HitActorSnapLocation);
				//No Snap Rotation, Player will Rotate on their own.
			}
			break;
		default:
			{
				//UE_LOG(LogTemp, Error, TEXT("No Matching Snap Condition from Vertical to Horizontal."))
			}
	}
}

void UBuildSystemManagerComponent::HorizontalToVerticalSnapCondition(FHitResult HitResult)
{
	int QuadrantCondition1 = QuadrantCondition(HitResult.GetComponent()->GetName());
	FVector HitActorSnapLocation;
	FVector ProxySnapLocation;
	ATimberBuildingComponentBase* ActiveBuildingComponentProxy = Cast<ATimberBuildingComponentBase>(BuildableProxyInstance);

	switch (QuadrantCondition1)
	{
	case 1:
		{
			//Bottom Vertical SnapPoint -> Top Quad on Horizontal
			HitActorSnapLocation = Cast<ATimberBuildingComponentBase>(HitResult.GetActor())->TopSnap->
				GetComponentTransform().GetLocation();
			ProxySnapLocation = ActiveBuildingComponentProxy->TopSnap->GetComponentTransform().GetLocation();
			MoveBuildingComponentProxyToSnapLocation(ProxySnapLocation, HitActorSnapLocation);
		}
		break;
	case 3:
		{
			//Bottom Vertical SnapPoint -> Top Quad on Horizontal
			HitActorSnapLocation = Cast<ATimberBuildingComponentBase>(HitResult.GetActor())
			                       ->BottomSnap->GetComponentTransform().GetLocation();
			ProxySnapLocation = ActiveBuildingComponentProxy->TopSnap->GetComponentTransform().GetLocation();
			MoveBuildingComponentProxyToSnapLocation(ProxySnapLocation, HitActorSnapLocation);
		}
		break;
	default:
		//UE_LOG(LogTemp, Error, TEXT("Floors and Ceilings can not Attach to Sides of Walls."));
		break;
	}
}

void UBuildSystemManagerComponent::MoveBuildingComponentProxyToSnapLocation(FVector ProxySnapLocation, FVector SnapLocation)
{
	/*Handles Movement of the Buildable*/
	ATimberBuildingComponentBase* ActiveBuildingComponentProxy = Cast<ATimberBuildingComponentBase>(BuildableProxyInstance);
	//Getting Vector Between 2 assumed Snap Points.
	FVector MoveLocation = SnapLocation - ProxySnapLocation;
	FVector CurrentLocation = ActiveBuildingComponentProxy->GetActorLocation();
	
	//Moving Actor the MoveLocation distance from the Current Location.
	ActiveBuildingComponentProxy->SetActorLocation(CurrentLocation + MoveLocation);
	FQuat Offset;
	if (ActiveBuildingComponentProxy->IsA(ATimberVerticalBuildingComponent::StaticClass()))
	{
		Offset = FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(90));
	}
	else if (ActiveBuildingComponentProxy->IsA(ATimberHorizontalBuildingComponent::StaticClass()))
	{
		Offset = FQuat(FVector(1, 0, 0), FMath::DegreesToRadians(90));
	}
	FQuat ProxyRotation = ActiveBuildingComponentProxy->GetActorQuat() * Offset;
	/*
	 * Here we are starting to check if whether our proposed snap location already has a Building Component in that spot.
	 * We set bWillOverlap to true if there is an overlap. and set the component bool BCanBuildableBeFinalized to False.
	 *
	 * We add 200 to the Z Location if the component is vertical because the ActorTransform is from its PivotPoint but we want the Center of the Wall Location.
	 * Used for the Box we are using to check overlaps.
	 */
	FVector WorldLocation = ActiveBuildingComponentProxy->GetActorTransform().TransformPosition(MoveLocation);
	if (Cast<ATimberVerticalBuildingComponent>(ActiveBuildingComponentProxy))
	{
		WorldLocation.Z += 200.0f;
	}
	FVector SweepBoxSize = FVector(170, 10, 170);
	/*Checking for Hit Building Component*/
	TArray<FHitResult> BuildableHitResults;
	TArray<FHitResult> PawnHitResults	;
	
	bool bWillOverlapBuildable = GetWorld()->SweepMultiByChannel(
		BuildableHitResults,
		WorldLocation,
		WorldLocation,  // Same start and end if stationary
		ProxyRotation,
		ECC_GameTraceChannel1,
		FCollisionShape::MakeBox(SweepBoxSize)
		);

	//Checking for a Mesh Hit Specifically, overlapping the Overlap Box of a BP can trigger a hit so we need to ensure we are overlapping a mesh.
	bool bHitMesh = false;
	if (bWillOverlapBuildable && BuildableHitResults.Num() > 0)
	{
		for (FHitResult Hit: BuildableHitResults)
		{
			UPrimitiveComponent* Component = Hit.GetComponent();
			//UE_LOG(LogTemp, Warning, TEXT("Proxy Overlap Component Hit: %s"), *Component->GetName());s
			if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Component))
			{
				if (Mesh)
				{
					//break that loop if any mesh was hit.
					bHitMesh = true;

					if (Cast<ARampBase>(Hit.GetActor()))
					{
						bHitMesh = false;
					}
					break;
				}
			}
		}
	}
	
	bool bWillOverlapPawn = GetWorld()->SweepMultiByChannel(
		PawnHitResults,
		WorldLocation,
		WorldLocation,  
		ProxyRotation,
		ECC_Visibility,  
		FCollisionShape::MakeBox(SweepBoxSize)
		);
	
	bool bHitPawnMesh = false;
	if (bWillOverlapPawn && PawnHitResults.Num() > 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Pawn Overlap Component Hit: %s"), *PawnHitResults[0].GetActor()->GetName());
		for (FHitResult Hit: PawnHitResults)
		{
			UPrimitiveComponent* Component = Hit.GetComponent();
			//UE_LOG(LogTemp, Warning, TEXT("Proxy Overlap Component Hit: %s"), *Component->GetName());
			UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Component);
			if (Hit.GetActor()->IsA(ATimberSeeda::StaticClass()) || Hit.GetActor()->IsA(ATimberCharacterBase::StaticClass()))
			{
				if (Mesh) //if the Hit Component is Static Mesh Component
				{
					//UE_LOG(LogTemp, Warning, TEXT("Mesh Hit: %s"), *Mesh->GetName());
					//UE_LOG(LogTemp, Warning, TEXT("Character Hit: %s"), *Hit.GetActor()->GetName());
					bHitPawnMesh = true;
					break;
				}
			}
			//UE_LOG(LogTemp, Warning, TEXT("No Character Actor Hit Results."));
		}
	}
	
	//DrawDebugBox(GetWorld(), WorldLocation, SweepBoxSize, ProxyRotation, FColor::Green, false, -1);
	
	//If Overlapped a Buildable && We overlapped its Mesh - Set the Buildable to not be finalized.
	if ((bWillOverlapBuildable && bHitMesh) || (bWillOverlapPawn && bHitPawnMesh))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Building Component Will Overlap."));
		ActiveBuildingComponentProxy->bCanBuildableBeFinalized = false;
		MakeMaterialHoloColor(ActiveBuildingComponentProxy, RedHoloMaterial);
	}
	else
	{
		//good to be Placed
		ActiveBuildingComponentProxy->bCanBuildableBeFinalized = true;
		MakeMaterialHoloColor(ActiveBuildingComponentProxy, BlueHoloMaterial);
	}

	/*
	 *USED FOR DEBUGGING
	 *FBox ActorBox = ActiveBuildingComponentProxy->GetComponentsBoundingBox();
	DrawDebugBox(GetWorld(), ActorBox.GetCenter(), ActorBox.GetExtent(), FColor::Red, false, 1.0f, 0, 5.0f);*/
}

EBuildingComponentOrientation UBuildSystemManagerComponent::CheckClassBuildingComponentOrientation(AActor* ClassToBeChecked)
{
	if (const ATimberVerticalBuildingComponent* VBC = Cast<ATimberVerticalBuildingComponent>(ClassToBeChecked))
	{
		return VBC->BuildingOrientation;
	}
	if (const ATimberHorizontalBuildingComponent* HBC = Cast<ATimberHorizontalBuildingComponent>(ClassToBeChecked))
	{
		return HBC->BuildingOrientation;
	}
	return EBuildingComponentOrientation::Default;
}

FVector UBuildSystemManagerComponent::SnapToGrid(FVector RaycastLocation)
{
	FVector SnappedVector;
	const int SnappedX = (FMath::FloorToInt(RaycastLocation.X) / GridSize) * GridSize;
	const int SnappedY = (FMath::FloorToInt(RaycastLocation.Y) / GridSize) * GridSize;

	SnappedVector.X = SnappedX;
	SnappedVector.Y = SnappedY;

	//Not Snapping the Z Axis. This should Spawn the Component Directly on the Ground where they raycast hits.
	SnappedVector.Z = RaycastLocation.Z;

	//Used in the Final Spawn Function on the Controller
	FinalSpawnLocation = SnappedVector;

	return FinalSpawnLocation;
}

FRotator UBuildSystemManagerComponent::SnapToRotation(FRotator CharactersRotation)
{
	//Use last saved Rotation of Component for future components.
	if (SavedRotation != FRotator::ZeroRotator)
	{
		return SavedRotation;
	}
	float CharacterRotationYaw = CharactersRotation.Yaw;

	// I think the intent here was to always make the Spawned Proxy and the Spawned Component Face the player when spawned.
	if (CharacterRotationYaw <= 45 && CharacterRotationYaw >= 0)
	{
		SavedRotation.Yaw = 180;
	}
	else if (CharacterRotationYaw >= -45 && CharacterRotationYaw < 0)
	{
		SavedRotation.Yaw = 180;
	}
	else if (CharacterRotationYaw > 45 && CharacterRotationYaw <= 135)
	{
		SavedRotation.Yaw = -90;
	}
	else if (CharacterRotationYaw < -45 && CharacterRotationYaw > -135)
	{
		SavedRotation.Yaw = 90;
	}
	else if (CharacterRotationYaw > 135)
	{
		SavedRotation.Yaw = 0;
	}
	else if (CharacterRotationYaw <= -135)
	{
		SavedRotation.Yaw = 0;
	}

	FinalSpawnRotation = SavedRotation;

	return SavedRotation;
}

void UBuildSystemManagerComponent::RotateProxyToSnapRotation(
	FRotator HitActorRotation, ABuildableBase* BuildingComponent)
{
	BuildingComponent->SetActorRotation(HitActorRotation);
}

void UBuildSystemManagerComponent::HandleTrapMaterialChange(bool bCanTrapBeFinalized)
{
	if (bCanTrapBeFinalized)
	{
		//GEngine->AddOnScreenDebugMessage(3, 4.0, FColor::Red, "Trap Can Be Finalized. Setting Color to Blue.");
		MakeMaterialHoloColor(BuildableProxyInstance, BlueHoloMaterial);
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(3, 4.0, FColor::Red, "Trap Cant Be Finalized. Setting Color to Red.");
		MakeMaterialHoloColor(BuildableProxyInstance, RedHoloMaterial);
	}
}

void UBuildSystemManagerComponent::MakeMaterialHoloColor(AActor* BuildingComponentActor, UMaterial* HoloMaterialColor)
{
	//Should get the static meshes for the new Building Component everytime we try to change the color.
	ClearStoredStaticMeshes();

	//TODO:: Add a check here so that we arent calling this setting of material every frame.
	if (StaticMeshs.Num() == 0)
	{
		GetStaticMeshComponents(BuildingComponentActor);
	}

	// Get all Static Meshes from the Actor
	if (StaticMeshs.Num() > 0)
	{
		//GEngine->AddOnScreenDebugMessage(5, 3.0f, FColor::Green, "Changing mesh color now.");
		for (UStaticMeshComponent* MeshComponent : StaticMeshs)
		{
			if (MeshComponent && MeshComponent->GetStaticMesh())
			{
				TArray<FStaticMaterial> MaterialSlots = MeshComponent->GetStaticMesh()->GetStaticMaterials();
				for (int Index = 0; Index < MaterialSlots.Num(); Index++)
				{
					if (HoloMaterialColor)
					{
						MeshComponent->SetMaterial(Index, HoloMaterialColor);
					}
				}
			}
		}
	}
}

void UBuildSystemManagerComponent::AddToBuildableAttachments(ABuildableBase* AttachingBuildable)
{
	ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(GetOwner());
	if (PlayerCharacter && PlayerCharacter->HoveredBuildingComponent && AttachingBuildable)
	{
		ATimberBuildingComponentBase* BuildingComponentBase = Cast<ATimberBuildingComponentBase>(PlayerCharacter->HoveredBuildingComponent);
		if (BuildingComponentBase)
		{
			BuildingComponentBase->AttachedBuildingComponents.Add(AttachingBuildable);
		}
	}
}

void UBuildSystemManagerComponent::SpawnFinalBuildable()
{
	if(ActiveBuildableComponentClass && BuildableProxyInstance)
	{
		// If a player can afford the transaction, apply the transaction and spawn the final building component.
		if (ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(GetOwner()))
		{
			if(PlayerCharacter->InventoryManager && PlayerCharacter->InventoryManager->bCanAffordCost(BuildableProxyInstance->BuildableCost))
			{
				FActorSpawnParameters SpawnParameters;

				switch (BuildableProxyInstance->SnapCondition)
				{
				//Walls, Floors, Etc.
				case ESnapCondition::BuildingComponent:
					SpawnFinalBuildingComponent(SpawnParameters);
					break;
				case ESnapCondition::CenterSnap:
					SpawnFinalCenterSnapBuildable(SpawnParameters);
					break;
				case ESnapCondition::FloorCenterSnapTopOnly:
					SpawnFinalFloorCenterSnapTopOnlyBuildable(SpawnParameters);
					break;
				case ESnapCondition::Ramp:
					SpawnFinalRampBuildable(SpawnParameters);
					break;
				case ESnapCondition::FloorEdgeSnapTopOnly:
					SpawnFinalFloorEdgeSnapTopOnlyBuildable(SpawnParameters);
					break;
				default:
					UE_LOG(LogTemp, Warning, TEXT("UBuildSystemManagerComponent::SpawnFinalBuildable() - No Valid Snap Condition to Spawn too."));
					PlayBuildDeniedSound();
					return;
				}


				/*Tutorial Stuff.*/
				ADieRobotGameStateBase* DieRobotGameStateBase = Cast<ADieRobotGameStateBase>(GetWorld()->GetGameState());
				if (DieRobotGameStateBase)
				{
					ETutorialState CurrentState = DieRobotGameStateBase->TutorialState;
					if (CurrentState == ETutorialState::Building1)
					{
						TutorialBuildsPlaced += 1;
						if (TutorialBuildsPlaced == 2)
						{
							//Wave Ready to begin.
							DieRobotGameStateBase->ChangeTutorialGameState(ETutorialState::Building2);
						}
					}
				}
				
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("UBuildSystemManagerComponent::SpawnFinalBuildable() - Player can not afford buildable."));
				PlayBuildDeniedSound();
			}
		}
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("BSMC - ActiveBuildableComponentClass or BuildableProxyInstance is Null."));
		PlayBuildDeniedSound();
	}
}

void UBuildSystemManagerComponent::SpawnFinalRampBuildable(FActorSpawnParameters SpawnParameters)
{
	ARampBase* ActiveRampComponentProxy = Cast<ARampBase>(BuildableProxyInstance);
	if (ActiveRampComponentProxy && ActiveRampComponentProxy->bCanBuildableBeFinalized)
	{
		//Spawn Final Ramp at the Component Proxies location
		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>
		(
			ActiveBuildableComponentClass,
			ActiveRampComponentProxy->GetActorLocation(),
			ActiveRampComponentProxy->GetActorRotation(),
			SpawnParameters);
		
		if(SpawnedActor)
		{
			BroadcastControllerUpdateNewBuildable(SpawnedActor);
			
			PlayBuildablePlacementSound();
			//Adding Ramp to Hovered Building Components Attachment Array, on Deletion or Destruction, Ramp will also be deleted.
			AddToBuildableAttachments(Cast<ABuildableBase>(SpawnedActor));

			//Build Event Subsystem 
			SendBuildEventPayload(SpawnedActor);

			if (TObjectPtr<ATimberPlayableCharacter> Player = Cast<ATimberPlayableCharacter>(GetOwner()))
			{
				Player->InventoryManager->bHandleBuildableTransaction(BuildableProxyInstance->BuildableCost);
			}
		}
	}
	else
	{
		PlayBuildDeniedSound();
	}
}

void UBuildSystemManagerComponent::SpawnFinalCenterSnapBuildable(FActorSpawnParameters SpawnParameters)
{
	
	ATrapBase* CenterSnapBuildable = Cast<ATrapBase>(BuildableProxyInstance);
	
	if (CenterSnapBuildable && CenterSnapBuildable->bCanBuildableBeFinalized)
	{
		//Spawn Final Trap
		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>
		(
			ActiveBuildableComponentClass,
			CenterSnapBuildable->GetActorLocation(),
			CenterSnapBuildable->GetActorRotation(),
			SpawnParameters);
		
		ATrapBase* FinalizedCenterSnapBuildable = Cast<ATrapBase>(SpawnedActor);
		if (FinalizedCenterSnapBuildable &&  CenterSnapBuildable->TrapHoveredBuildingComponent)
		{
			//Passing the Trap Direction from the Proxy to the Final Trap.
			FinalizedCenterSnapBuildable->BuildingComponentTrapDirection = CenterSnapBuildable->BuildingComponentTrapDirection;
			
			//Setting Parent-BC-Ref on Trap.
			FinalizedCenterSnapBuildable->ParentBuildable = CenterSnapBuildable->TrapHoveredBuildingComponent;

			//Handle Walkable Slop Override is Placed on Wall.
			//Enemies should be able to walk over traps when placed Horizontally on floor, but when vertical they should not as this would
			//allow the player to walk ontop of walls.
			FinalizedCenterSnapBuildable->ConfigureStaticMeshWalkableSlope(FinalizedCenterSnapBuildable->ParentBuildable);

			//Sets Collision to BC
			FinalizedCenterSnapBuildable->SetActorEnableCollision(true);

			//Resetting the Trap Direction back to default for further usage of the proxy.
			CenterSnapBuildable->BuildingComponentTrapDirection = EBuildingComponentTrapDirection::Default;

			//BuildableProxyInstance = Cast<ABuildableBase>(FinalizedCenterSnapBuildable);
			
			//Uses the solved Trap Direction to know which snap points to occupy on the Building Component (Wall/Floor)
			EBuildingComponentTrapDirection TrapDirection = FinalizedCenterSnapBuildable->BuildingComponentTrapDirection;
			switch (TrapDirection)
			{
			case EBuildingComponentTrapDirection::Front:
				CenterSnapBuildable->TrapHoveredBuildingComponent->FrontCenterAttachment = FinalizedCenterSnapBuildable;
				//UE_LOG(LogTemp, Warning, TEXT("Set Trap on Building Component Front Trap Slot."));
				break;
			case EBuildingComponentTrapDirection::Back:
				CenterSnapBuildable->TrapHoveredBuildingComponent->BackCenterAttachment = FinalizedCenterSnapBuildable;
				//UE_LOG(LogTemp, Warning, TEXT("Set Trap on Building Component Back Trap Slot."));
				break;
			case EBuildingComponentTrapDirection::Default:
				GEngine->AddOnScreenDebugMessage(3, 3.0f, FColor::Red, "Trap Direction Not Specified.");
				break;
			}
			
			PlayBuildablePlacementSound();
			BroadcastControllerUpdateNewBuildable(SpawnedActor);
			AddToBuildableAttachments(Cast<ABuildableBase>(FinalizedCenterSnapBuildable));
			
			//Build Event Subsystem 
			SendBuildEventPayload(SpawnedActor);

			if (TObjectPtr<ATimberPlayableCharacter> Player = Cast<ATimberPlayableCharacter>(GetOwner()))
			{
				Player->InventoryManager->bHandleBuildableTransaction(BuildableProxyInstance->BuildableCost);
			}
		}
		ResetBuildableComponents();
	}
	else
	{
		PlayBuildDeniedSound();
		GEngine->AddOnScreenDebugMessage(8, 5.0f, FColor::Red, "Trap Cannot Be Finalized. BuildSystemManager-SpawnFinalBuildingComponent");
	}

}

void UBuildSystemManagerComponent::SpawnFinalBuildingComponent(FActorSpawnParameters SpawnParameters)
{
	ATimberBuildingComponentBase* ActiveBuildingComponentProxy = Cast<ATimberBuildingComponentBase>(BuildableProxyInstance);
	//Use the InputTransform as the Location to Spawn the ActiveBuildingComponent
	if(ActiveBuildableComponentClass && ActiveBuildingComponentProxy && ActiveBuildingComponentProxy->bCanBuildableBeFinalized)
	{
		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>
		(
		ActiveBuildableComponentClass,
		ActiveBuildingComponentProxy->GetActorLocation(),
		ActiveBuildingComponentProxy->GetActorRotation(),
		SpawnParameters
		);
		
		if(SpawnedActor)
		{
			SpawnedActor->SetActorEnableCollision(true);
			BroadcastControllerUpdateNewBuildable(SpawnedActor);
			PlayBuildablePlacementSound();

			//Build Event Subsystem 
			SendBuildEventPayload(SpawnedActor);

			if (TObjectPtr<ATimberPlayableCharacter> Player = Cast<ATimberPlayableCharacter>(GetOwner()))
			{
				Player->InventoryManager->bHandleBuildableTransaction(BuildableProxyInstance->BuildableCost);
			}
		}
	}
	else if (!ActiveBuildingComponentProxy->bCanBuildableBeFinalized)
	{
		PlayBuildDeniedSound();
		//GEngine->AddOnScreenDebugMessage(5, 3.0f, FColor::Red, TEXT("Building Component Cannot Be Finalized. Overlapping other building Component."));
	}
}

void UBuildSystemManagerComponent::SpawnFinalFloorEdgeSnapTopOnlyBuildable(FActorSpawnParameters SpawnParameters)
{
	/*
	 * Classes Using This Snap:
	 * Teleporters
	 */
	
	if (BuildableProxyInstance && BuildableProxyInstance->bCanBuildableBeFinalized)
	{
		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
			ActiveBuildableComponentClass,
			BuildableProxyInstance->GetActorLocation(),
			BuildableProxyInstance->GetActorRotation(),
			SpawnParameters);

		/* Setting Parent Buildable*/
		if (ABuildableBase* SpawnedBuildable = Cast<ABuildableBase>(SpawnedActor))
		{
			//Attaching EdgeSnapBuildable to the Player Character Hovered Building Component, when Destroyed Teleporter handles destruction of its own pair.
			ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(GetOwner());
			if (IsValid(PlayerCharacter) && IsValid(PlayerCharacter->HoveredBuildingComponent))
			{
				SpawnedBuildable->ParentBuildable = Cast<ATimberBuildingComponentBase>(PlayerCharacter->HoveredBuildingComponent);
			}
		}

		if (SpawnedActor)
		{
			//Adding Buildable to Parent Buildable Attachments Array.
			AddToBuildableAttachments(Cast<ABuildableBase>(SpawnedActor));
			
			BroadcastControllerUpdateNewBuildable(SpawnedActor);
			
			PlayBuildablePlacementSound();
			
			ATeleportConstruct* TeleportConstruct = Cast<ATeleportConstruct>(SpawnedActor);
			if (TeleportConstruct)
			{
				HandleIsTeleporter(TeleportConstruct);
			}
			
			SendBuildEventPayload(SpawnedActor);
			
		}
	}
	else
	{
		PlayBuildDeniedSound();
	}
}

void UBuildSystemManagerComponent::SpawnFinalFloorCenterSnapTopOnlyBuildable(FActorSpawnParameters SpawnParameters)
{
	/*
	 * Classes Using this Snap:
	 * PowerPlate
	 */
	if (BuildableProxyInstance && BuildableProxyInstance->bCanBuildableBeFinalized)
	{
		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>
		(
			ActiveBuildableComponentClass,
			BuildableProxyInstance->GetActorLocation(),
			BuildableProxyInstance->GetActorRotation(),
			SpawnParameters);

		if (SpawnedActor)
		{
			/*
			 * Attaching the Buildable to the Building Component Slot
			 * Lets the player know that another Buildable can not be placed in this slot.
			 */
			ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(GetOwner());
			if (PlayerCharacter && IsValid(PlayerCharacter->HoveredBuildingComponent))
			{
				if (ATimberBuildingComponentBase* BuildingComponent = Cast<ATimberBuildingComponentBase>(PlayerCharacter->HoveredBuildingComponent))
				{
					BuildingComponent->FrontCenterAttachment = Cast<ABuildableBase>(SpawnedActor);
					//UE_LOG(LogTemp, Warning, TEXT("Construct set on FrontTrapSnap Slot."))
					APowerPlate* PowerPlate = Cast<APowerPlate>(SpawnedActor);

					//Setting the parent floor component on this actor, so we can free up slot during deletion.
					if (PowerPlate && Cast<ATimberBuildingComponentBase>(PlayerCharacter->HoveredBuildingComponent))
					{
						PowerPlate->ParentBuildable = Cast<ATimberBuildingComponentBase>(PlayerCharacter->HoveredBuildingComponent);
					}
				}
			}
			BroadcastControllerUpdateNewBuildable(SpawnedActor);
			AddToBuildableAttachments(Cast<ABuildableBase>(SpawnedActor));
			PlayBuildablePlacementSound();
			SendBuildEventPayload(SpawnedActor);
			if (TObjectPtr<ATimberPlayableCharacter> Player = Cast<ATimberPlayableCharacter>(GetOwner()))
			{
				Player->InventoryManager->bHandleBuildableTransaction(BuildableProxyInstance->BuildableCost);
			}
		}
	}
	else
	{
		PlayBuildDeniedSound();
	    //UE_LOG(LogTemp, Warning, TEXT("BSMC - SpawnFinalFloorOnlyBC - Buildable Can Not be Finalized."));
	}
}

void UBuildSystemManagerComponent::HandleIsTeleporter(ATeleportConstruct* TeleportConstruct)
{
	Cast<ATimberPlayableCharacter>(GetOwner())->InventoryManager->bHandleBuildableTransaction(BuildableProxyInstance->BuildableCost);

	//Handle Pairing - BSM
	//This is in the BuildSystemManager's Memory of the Teleport Construct Pair
	if (TeleportTempPair.Key == nullptr)
	{
		//Makes newly Spawned Material the Yellow Holo Material - Because it is temporary. We will move this to a delegate system.
		MakeMaterialHoloColor(TeleportConstruct, YellowHoloMaterial);
		//Setting the First Half of the Teleport Construct - Stored on BSM
		TeleportTempPair.Key = TeleportConstruct;
		//This is the first half of the Teleport Construct - storing in Memory and preparing for the second half.
		TeleportConstruct->TeleportConstructState = ETeleportConstructState::Temporary;
	}
	else if (IsValid(TeleportTempPair.Key) && !TeleportTempPair.Value)
	{
		//Second Half of the Teleport Construct - Setting the Pairing - Stored on BSM
		TeleportTempPair.Value = TeleportConstruct;

		/* Now there is a full Pair of teleporters -> Linking & Clean up */
		
		// Temp Local Value       -						Stored on BSM
		ATeleportConstruct* TeleportConstructAlpha = TeleportTempPair.Key;
		ATeleportConstruct* TeleportConstructBeta = TeleportTempPair.Value;
				
		//Each Teleport Item now has a reference to the other in its pair.
		//The Teleport Pair is stored on the Teleport Actor Class - this is how the teleporters will reference each other.
		TeleportConstructAlpha->TeleportPair = TeleportConstructBeta;
		TeleportConstructBeta->TeleportPair = TeleportConstructAlpha;

		//Linking the Destruct Delegates for the pairs.
		TeleportConstructAlpha->LinkToPair(TeleportConstructBeta);
		TeleportConstructBeta->LinkToPair(TeleportConstructAlpha);

		//Setting Finalized State
		TeleportConstructAlpha->TeleportConstructState = ETeleportConstructState::Final;
		TeleportConstructBeta->TeleportConstructState = ETeleportConstructState::Final;

		//Applying the default materials for the Teleporter - Stored on the Teleporter after BeginPlay() 
		TeleportConstructAlpha->ApplyDefaultMaterials();
		TeleportConstructBeta->ApplyDefaultMaterials();
				
		//Clear the Temp Pair on the BSM -  We don't need the local values if the Teleport Construct has ref to each other.
		TeleportTempPair.Key = nullptr;
		TeleportTempPair.Value = nullptr;
	}
}

void UBuildSystemManagerComponent::SendBuildEventPayload(TObjectPtr<AActor> Buildable)
{
	//Gets called on all successful Build Type Placements
	UMissionEventSubsystem* MES = GetWorld()->GetGameInstance()->GetSubsystem<UMissionEventSubsystem>();
	if (!MES) return;
	
	FMissionEventPayload Payload;
	Payload.InstigatingActor = GetOwner();
	Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Build.Placed"));
	Payload.Count = 1;

	FGameplayTagContainer BuildEventContextTags;
	GenerateBuildEventsContextTags(BuildEventContextTags, Buildable);
	Payload.ContextTags = BuildEventContextTags;

	MES->BroadcastMissionEvent(Payload);
	
}

void UBuildSystemManagerComponent::GenerateBuildEventsContextTags(FGameplayTagContainer& TagContainer, const TObjectPtr<AActor>& Buildable)
{
	if (Buildable)
	{
		if (Buildable.IsA(ATimberBuildingComponentBase::StaticClass()))
		{
			TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Event.Build.Placed.Structure")));
		}
		else if (Buildable.IsA(ATrapBase::StaticClass()))
		{
			TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Event.Build.Placed.Trap")));
		}
		else if (Buildable.IsA(AConstructBase::StaticClass()))
		{
			TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Event.Build.Placed.Construct")));
		}
		else if (Buildable.IsA(ARampBase::StaticClass()))
		{
			TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Event.Build.Placed.Ramp")));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BSMC - GenerateBuildEventsContextTags - Unknown Buildable Type."));
		}
	}
}

void UBuildSystemManagerComponent::DisableBuildableProxyCollisions(ABuildableBase* BuildingComponent)
{
	/*
	 * Needed for multiple things:
	 * 1. So that the player can walk through the proxy.
	 * 2. So proxy doesn't trigger the Overlap Check on other building components.
	 */

	BuildingComponent->GetAllStaticMeshComponents();
	if (BuildingComponent->StaticMeshComponents.Num() > 0)
	{
		for (UStaticMeshComponent* MeshComponent : BuildingComponent->StaticMeshComponents)
		{
			MeshComponent->SetCollisionProfileName("DR_ProxyBuildable");
		}
	}
	
	TArray<UBoxComponent*> BoxComponents;
	BuildingComponent->GetComponents<UBoxComponent*>(BoxComponents);
	if (BoxComponents.Num() > 0)
	{
		for (UBoxComponent* Comp : BoxComponents)
		{
			//Comp->SetCollisionProfileName("DR_ProxyBuildable");
			Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

USceneComponent* UBuildSystemManagerComponent::GetClosestFaceSnapPoint(FHitResult HitResult)
{
	FVector ImpactPointLocation = HitResult.ImpactPoint;
	ATimberBuildingComponentBase* BuildingComponent = Cast<ATimberBuildingComponentBase>(HitResult.GetActor());
	if (BuildingComponent)
	{
		FVector BackCenterSnapLocation  = BuildingComponent->BackCenterSnapPoint->GetComponentLocation();
		FVector FrontCenterSnapLocation = BuildingComponent->FrontCenterSnapPoint->GetComponentLocation();
		float LengthToFrontSnap = FVector::Dist(ImpactPointLocation, FrontCenterSnapLocation);
		float LengthToBackSnap = FVector::Dist(ImpactPointLocation, BackCenterSnapLocation);
		if (LengthToFrontSnap < LengthToBackSnap)
		{
			return BuildingComponent->FrontCenterSnapPoint;
		}
		return BuildingComponent->BackCenterSnapPoint;
		
	}
	return nullptr;
}

void UBuildSystemManagerComponent::BroadcastControllerUpdateNewBuildable(AActor* Buildable)
{
	ABuildableBase* CastBuildable = Cast<ABuildableBase>(Buildable);
	ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(GetOwner());
	
	if (IsValid(PlayerCharacter) && IsValid(CastBuildable))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Changed Buildable Name: %s"), *CastBuildable->GetName());
		ATimberPlayerController* PC = PlayerCharacter->GetController<ATimberPlayerController>();
		if (IsValid(PC))
		{
			//Used for Enemy to Update Path.
			PC->OnBuildableAdded.Broadcast(CastBuildable);

			//Used for Game Mode to update the Path Tracer.
			RedrawPathTraceHandle.Broadcast();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BSMC - BroadcastControllerUpdateNewBuildable - Player Character or Buildable is Null."));
	}
}

void UBuildSystemManagerComponent::HandleBuildingComponentPlacement(FHitResult FirstHitBuildingComponentHitResult)
{
	if (!BuildableProxyInstance) return;

	ATimberBuildingComponentBase* FirstHitBuildingComponent = Cast<ATimberBuildingComponentBase>(FirstHitBuildingComponentHitResult.GetActor());
	
	//Ensures floors are not placed on Environment Buildable.
	if (BuildableProxyInstance->IsA(ATimberHorizontalBuildingComponent::StaticClass()))
	{
		if (ATimberHorizontalBuildingComponent* FloorComponent = Cast<ATimberHorizontalBuildingComponent>(BuildableProxyInstance))
		{
			//Ensures we don't place a floor structure on an environment type structure
			if (FirstHitBuildingComponent && FirstHitBuildingComponent->BuildingComponentType == EBuildingComponentType::Environment)
			{
				UE_LOG(LogTemp, Warning, TEXT("Structure is an Environment Buildable that can not be built on."));
				MakeBuildableNotFinalizable(BuildableProxyInstance);
				MoveBuildable(FirstHitBuildingComponentHitResult.ImpactPoint, FloorComponent);
				return;
			}
			
			MakeBuildableFinalizable(BuildableProxyInstance);
		}
	}

	//Using this just as a check for a Valid Hit Result
	if (FirstHitBuildingComponent)
	{
		if(FirstHitBuildingComponentHitResult.GetComponent()->IsA(UBoxComponent::StaticClass()))
		{
			HandleBuildingComponentSnapping(FirstHitBuildingComponentHitResult);

			//TODO:: We need to clean up this elevation issue. We dont want players to unknowingly snap walls to below the floor.
			// Snapping right now, snaps the Wall Buildable to the lowest most floor to -11.0f. Im chushioning it to -15.0f.
			// We need a cleaner way of handling this.
			if (BuildableProxyInstance->bIsOverlappingPerimeter || BuildableProxyInstance->GetActorLocation().Z < -15.0f)
			{
				MakeBuildableNotFinalizable(BuildableProxyInstance);
			}
		}
	}
	else
	{
		/* Valid Hit, but Hit Actor is not a Building Component Structure. */
		MakeBuildableNotFinalizable(BuildableProxyInstance);
		MoveBuildable(FirstHitBuildingComponentHitResult.ImpactPoint, BuildableProxyInstance);
	}
}

void UBuildSystemManagerComponent::MoveBuildable(FVector_NetQuantize Location, ABuildableBase* BuildingComponent, const FRotator& Rotation)
{
	if (BuildingComponent)
	{
		BuildingComponent->SetActorLocation(Location, true);
		//OnHits we will pass in the Rotation of the HitActor so the Components Match its Rotation
		if (Rotation != FRotator::ZeroRotator)
		{
			//GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, "Rotated Trap to match Building Component.");
			BuildingComponent->SetActorRotation(Rotation);
		}
	}
	
}

void UBuildSystemManagerComponent::CleanUpBuildSystemManagerComponent()
{
	ClearTeleporterTPair();
	RemoveBuildingComponentProxies_All();
	ResetBuildableComponents();
}

void UBuildSystemManagerComponent::ResetBuildableComponents()
{
	if (BuildableProxyInstance)
	{
		BuildableProxyInstance->Destroy();
		BuildableProxyInstance = nullptr;
	}
	
	StaticMeshs.Empty();
}

void UBuildSystemManagerComponent::RemoveBuildingComponentProxies_All()
{
	if (BuildableProxyInstance)
	{
		BuildableProxyInstance->Destroy();
		BuildableProxyInstance = nullptr;
	}
	if (GEngine)
	{
		//GEngine->AddOnScreenDebugMessage(3, 5.0f, FColor::Green, "ATimberPlayableCharacter::ExitBuildMode() : Building Component Proxy Removed. ");
	}
}

void UBuildSystemManagerComponent::ClearTeleporterTPair()
{
	if (IsValid(TeleportTempPair.Key))
	{
		TObjectPtr<ATeleportConstruct> TeleportConstructAlpha = TeleportTempPair.Key;
		if (IsValid(TeleportConstructAlpha) && TeleportConstructAlpha->TeleportConstructState == ETeleportConstructState::Temporary)
		{
			TeleportConstructAlpha->SpawnLoot();
			
			if (TObjectPtr<ATimberBuildingComponentBase> BC = Cast<ATimberBuildingComponentBase>(TeleportConstructAlpha->ParentBuildable))
			{
				if(BC->AttachedBuildingComponents.Contains(TeleportConstructAlpha))
				{
					BC->AttachedBuildingComponents.Remove(TeleportConstructAlpha);	
				};
			}
		}
	}
	TeleportTempPair.Key = nullptr;
	TeleportTempPair.Value = nullptr;
}

void UBuildSystemManagerComponent::GetStaticMeshComponents(AActor* BuildingComponentActor)
{
	if (BuildingComponentActor)
	{
		//Gets static Meshes and returns them to the GlobalArray.
		BuildingComponentActor->GetComponents<UStaticMeshComponent>(StaticMeshs);
	}
}

void UBuildSystemManagerComponent::RotateBuildingComponent()
{
	if (BuildableProxyInstance)
	{
		SavedRotation.Yaw += 90;
		if (SavedRotation.Yaw >= 360) 
		{
			SavedRotation.Yaw = 0;
		}
		BuildableProxyInstance->SetActorRotation(SavedRotation);

		//Updating FinalSpawnRotation after Player Rotates Component
		FinalSpawnRotation = SavedRotation;
	}
}

void UBuildSystemManagerComponent::MakeBuildableFinalizable(ABuildableBase* Buildable)
{
	Buildable->bCanBuildableBeFinalized = true;
	MakeMaterialHoloColor(BuildableProxyInstance, BlueHoloMaterial);
}

void UBuildSystemManagerComponent::MakeBuildableNotFinalizable(ABuildableBase* Buildable)
{
	Buildable->bCanBuildableBeFinalized = false;
	MakeMaterialHoloColor(BuildableProxyInstance, RedHoloMaterial);
}

void UBuildSystemManagerComponent::SetActiveBuildingComponentClass(TSubclassOf<AActor> BuildingComponentClass)
{
	ActiveBuildableComponentClass = BuildingComponentClass;
}

void UBuildSystemManagerComponent::PlayBuildablePlacementSound()
{
	USFXManagerSubsystem* SFXManager = GetWorld()->GetGameInstance()->GetSubsystem<USFXManagerSubsystem>();
	if (SFXManager)
	{
		SFXManager->OnBuildablePlacement.Broadcast();
	}
}

void UBuildSystemManagerComponent::PlayBuildDeniedSound()
{
	USFXManagerSubsystem* SFXManager = GetWorld()->GetGameInstance()->GetSubsystem<USFXManagerSubsystem>();
	if (SFXManager)
	{
		SFXManager->PlaySound("BuildDenied_1");
	}
}

AActor* UBuildSystemManagerComponent::SpawnProxy(TSubclassOf<ABuildableBase> ActiveBuildableClass, FVector SpawnLocation, FRotator SpawnRotation)
{
	FActorSpawnParameters SpawnParams;
	
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ActiveBuildableClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (SpawnedActor)
	{
		if (ABuildableBase* SpawnedBuildingComponent = Cast<ABuildableBase>(SpawnedActor))
		{
			//We need to set this to proxy to ensure that the overlap check box is created.
			//UE_LOG(LogTemp, Warning, TEXT("Spawned Building Component Proxy"));
			SpawnedBuildingComponent->bIsProxy = true;
		}
		
		DisableBuildableProxyCollisions(Cast<ABuildableBase>(SpawnedActor));
		MakeMaterialHoloColor(SpawnedActor, BlueHoloMaterial);
		return SpawnedActor;
	}
	return nullptr;
}

FHitResult UBuildSystemManagerComponent::FirstHitBuildingComponentHitResult(TArray<FHitResult> HitResults)
{
	//We return the First Hit Building Component, else, we return the first hit result that can be anything.
	FHitResult FirstHitBuildingComponentHitResult;
	for (FHitResult Hit : HitResults)
	{
		if (Cast<ATimberBuildingComponentBase>(Hit.GetActor()) && Hit.GetActor() != BuildableProxyInstance)
		{
			FirstHitBuildingComponentHitResult = Hit;
			return FirstHitBuildingComponentHitResult;
		}
	}
	
	return HitResults[0];
}

void UBuildSystemManagerComponent::HandleProxyPlacement(TArray<FHitResult> HitResults, TSubclassOf<ABuildableBase> BuildableProxyClass)
{
	//Called Every Tick
	
	/*
	 * SPAWNING PROXY IF ONE DOESNT EXIST
	 */
	if (!BuildableProxyInstance || BuildableProxyInstance->GetClass() != ActiveBuildableComponentClass) //Checking to see if a Proxy is already spawned.
	{
		//We are Spawning the Initial Proxy at the Hit Location. At the next frame there may be a Snap that we can move to.
		BuildableProxyInstance = Cast<ABuildableBase>(SpawnProxy(BuildableProxyClass, HitResults[0].ImpactPoint, 
		FRotator::ZeroRotator));
		if (BuildableProxyInstance)
		{
			BuildableProxyInstance->bCanBuildableBeFinalized = false;
		}
	}

	/*
	 * GATHERING FIRST HIT BUILDING COMPONENT Hit Result
	 */
	//ATimberBuildingComponentBase* FirstHitBuildingComponent = FindFirstHitBuildingComponent(HitResults);
	FHitResult BuildingComponentHitResult = FirstHitBuildingComponentHitResult(HitResults);
	
	/*
	 * GATHERING FINAL SPAWN INFORMATION
	 *
	 *  This will either Snap the Buildable to a Proper Snap Location OR Move it to the Hit Location.
	 */
	
	if (BuildableProxyInstance && BuildingComponentHitResult.GetActor())
	{
		switch (BuildableProxyInstance->SnapCondition)
		{
		case ESnapCondition::Default:
			break;
		case ESnapCondition::BuildingComponent:
			HandleBuildingComponentPlacement(BuildingComponentHitResult);
			break;
		case ESnapCondition::CenterSnap:
			HandleCenterSnapPlacement(BuildingComponentHitResult);
			break;
		case ESnapCondition::FloorEdgeSnapTopOnly:
			HandleFloorEdgeSnapTopOnlyPlacement(BuildingComponentHitResult);
			break;
		case ESnapCondition::Ramp:
			HandleRampPlacement(BuildingComponentHitResult);
			break;
		case ESnapCondition::FloorCenterSnapBottomOnly:
			break;
		case ESnapCondition::FloorCenterSnapTopOnly:
			HandleFloorCenterSnapTopOnlyPlacement(BuildingComponentHitResult);
			break;
		default:
			break;
		}
	}
}

void UBuildSystemManagerComponent::HandleCenterSnapPlacement(FHitResult FirstHitBuildingComponentHitResult)
{
	/*
	 * Usages:
	 * Traps that mount Center but can be applied to both sides of wall/floor.
	 * Ex. Spike Trap, Frost Trap, Electro Pulse.
	 */
	ATimberBuildingComponentBase* BuildingComponent = Cast<ATimberBuildingComponentBase>(FirstHitBuildingComponentHitResult.GetActor());
	ATrapBase* ActiveCenterSnapBuildable = Cast<ATrapBase>(BuildableProxyInstance);
	
	if (ActiveCenterSnapBuildable)
	{
		if (BuildingComponent)
		{
			/* HIT EVIRONMENT BUILDING COMPONENT*/
			if (BuildingComponent->BuildingComponentType == EBuildingComponentType::Environment || BuildingComponent->BuildableType == EBuildableType::Environment)
			{
				MoveBuildable(FVector_NetQuantize(FirstHitBuildingComponentHitResult.ImpactPoint),
					ActiveCenterSnapBuildable,
					FRotator::ZeroRotator);
				MakeBuildableNotFinalizable(BuildableProxyInstance);
				return;
			}

			/*HIT BUIDLING COMPONENT - NON-ENVIRONMENT*/
			ActiveCenterSnapBuildable->TrapHoveredBuildingComponent = BuildingComponent;

			//Handles Snapping to Closest Snap Point. Selecting the Correct Snap Slot on the Building Component Wall or Floor. Returns an object with Placement Data.
			FBuildablePlacementData PlacementData = GetTrapSnapTransform(FirstHitBuildingComponentHitResult.ImpactPoint, BuildingComponent, ActiveCenterSnapBuildable);
		
			MoveBuildable(FVector_NetQuantize(PlacementData.TrapLocation),
				ActiveCenterSnapBuildable,
				PlacementData.TrapRotation);

			//If set to Trap Direction::Default, it means both front and back snap locations were occupied.
			if (ActiveCenterSnapBuildable->BuildingComponentTrapDirection == EBuildingComponentTrapDirection::Default)
			{
				MakeBuildableNotFinalizable(BuildableProxyInstance);
			}
			else
			{
				MakeBuildableFinalizable(ActiveCenterSnapBuildable);
			}
			return;
		}
		
		/*HIT A NON-BUILDING COMPONENT*/
		ActiveCenterSnapBuildable->TrapHoveredBuildingComponent = nullptr;
		
		if (FirstHitBuildingComponentHitResult.ImpactPoint != FVector::ZeroVector)
		{
			FRotator PlayerRotation = GetOwner()->GetActorTransform().GetRotation().Rotator();
			PlayerRotation.Yaw = PlayerRotation.Yaw - 180;
			
			MoveBuildable(
				FirstHitBuildingComponentHitResult.ImpactPoint, ActiveCenterSnapBuildable,
				PlayerRotation);

			MakeBuildableNotFinalizable(BuildableProxyInstance);
		}
	}
}

void UBuildSystemManagerComponent::HandleFloorCenterSnapTopOnlyPlacement(FHitResult FirstHitBuildingComponentHitResult)
{
	
	ATimberHorizontalBuildingComponent* FloorComponent = Cast<ATimberHorizontalBuildingComponent>(FirstHitBuildingComponentHitResult.GetActor());
	if (FloorComponent)
	{
		//Checking for finalization - if Building Component already has something in Snap Slot.
		if (FloorComponent->FrontCenterAttachment != nullptr)
		{
			MakeBuildableNotFinalizable(BuildableProxyInstance);
			
			MoveBuildable(FloorComponent->FrontCenterSnapPoint->GetComponentLocation(), BuildableProxyInstance, FRotator::ZeroRotator);
		}
		else
		{
			MoveBuildable(FloorComponent->FrontCenterSnapPoint->GetComponentLocation(), BuildableProxyInstance, FRotator::ZeroRotator);

			MakeBuildableFinalizable(BuildableProxyInstance);
		}
	}
	else
	{
		//If the Hit Actor is not a Floor Component, we move the Proxy to the Hit Location & set to not Finalizable.
		MoveBuildable(FirstHitBuildingComponentHitResult.ImpactPoint, BuildableProxyInstance, FRotator::ZeroRotator);
		
		MakeBuildableNotFinalizable(BuildableProxyInstance);
	}
}

void UBuildSystemManagerComponent::HandleFloorEdgeSnapTopOnlyPlacement(FHitResult FirstHitBuildingComponentHitResult)
{
	/*
	 * Classes Using this:
	 * Teleporter
	 */
	
	if (ATimberHorizontalBuildingComponent* FloorComponent = Cast<ATimberHorizontalBuildingComponent>(FirstHitBuildingComponentHitResult.GetActor()))
	{
		/* Getting Top Snap Points */
		TArray<USceneComponent*> AllSceneComponents;
		TArray<USceneComponent*> FilteredSnapPoints;
		FloorComponent->GetComponents<USceneComponent>(AllSceneComponents);
		for (USceneComponent* SnapPoint : AllSceneComponents )
		{
			FString ComponentName = SnapPoint->GetName();
			if (ComponentName == TEXT("LeftSnap") || ComponentName == TEXT("RightSnap") || ComponentName == TEXT("TopSnap") || ComponentName == TEXT("BottomSnap"))
			{
				FilteredSnapPoints.Add(SnapPoint);
			}
		}
		if (FilteredSnapPoints.Num() <= 0)
		{
			return;
		}

		//Check the distance between Raycast Hit and Snap Point && Save the closest one.
		float DistanceToClosestSnapPoint = 0;
		USceneComponent* ClosestSnapPoint = nullptr;
		for (USceneComponent* SnapPoint : FilteredSnapPoints)
		{
			float CheckDistance = FVector::Dist(FirstHitBuildingComponentHitResult.ImpactPoint, SnapPoint->GetComponentLocation());
			if (CheckDistance < DistanceToClosestSnapPoint || DistanceToClosestSnapPoint == 0)
			{
				DistanceToClosestSnapPoint = CheckDistance;
				ClosestSnapPoint = SnapPoint;
			}
		}
		
		/* Orientation & Placement */ 
		FVector BuildableProxy = BuildableProxyInstance->GetActorLocation();
		FVector FloorCenter = FloorComponent->CenterSnap->GetComponentLocation();
		FRotator RotateToFace = UKismetMathLibrary::FindLookAtRotation(BuildableProxy, FloorCenter);
		
		MoveBuildable(ClosestSnapPoint->GetComponentLocation(), BuildableProxyInstance, RotateToFace);

		if (BuildableProxyInstance->bIsOverlappingPerimeter)
		{
			MakeBuildableNotFinalizable(BuildableProxyInstance);
		}
		else
		{
			MakeBuildableFinalizable(BuildableProxyInstance);
		}
		
		
		//Teleporters Have Addition Construct States due to the need of Multiple Placements.
		if(BuildableProxyInstance->IsA(ATeleportConstruct::StaticClass()))
		{
			Cast<ATeleportConstruct>(BuildableProxyInstance)->TeleportConstructState = ETeleportConstructState::Proxy;
		}

		return;
	}
	
	MoveBuildable(FirstHitBuildingComponentHitResult.ImpactPoint, BuildableProxyInstance, FRotator::ZeroRotator);
	MakeBuildableNotFinalizable(BuildableProxyInstance);
	
}

void UBuildSystemManagerComponent::HandleRampPlacement(FHitResult FirstHitBuildingComponentHitResult)
{
	ATimberBuildingComponentBase* BuildingComponent = Cast<ATimberBuildingComponentBase>(FirstHitBuildingComponentHitResult.GetActor());
	ARampBase* ActiveRampComponentProxy = Cast<ARampBase>(BuildableProxyInstance);
	
	if (ActiveRampComponentProxy)
	{
		if (BuildingComponent)
		{
			if (BuildingComponent->BuildingOrientation == EBuildingComponentOrientation::Vertical)
			{
				USceneComponent* BCSnapComponent = GetClosestFaceSnapPoint(FirstHitBuildingComponentHitResult);
				
				FVector RampSnapLocation = ActiveRampComponentProxy->VerticalCenterSnap->GetComponentLocation();
				if (BCSnapComponent)
				{
					FRotator SnapRotation = BCSnapComponent->GetComponentRotation();
					
					ActiveRampComponentProxy->SetActorRotation(SnapRotation);

					FVector BCSnapLocation = BCSnapComponent->GetComponentLocation();

					FVector OffsetVector = BCSnapLocation - RampSnapLocation;
	
					ActiveRampComponentProxy->SetActorLocation(ActiveRampComponentProxy->GetActorLocation() + OffsetVector);
				}
				MakeBuildableFinalizable(ActiveRampComponentProxy);

			}
			else if (BuildingComponent->BuildingOrientation == EBuildingComponentOrientation::Horizontal)
			{
				// Snap Ramps Horizontal Snap to the Building Components Horizontal Center Snap
				FVector RampHorizontalCenterSnap = ActiveRampComponentProxy->HorizontalCenterSnap->GetComponentLocation();
				
				FVector HitBuildingCenterSnap = BuildingComponent->CenterSnap->GetComponentLocation();

				FVector OffsetVector = HitBuildingCenterSnap - RampHorizontalCenterSnap;

				ActiveRampComponentProxy->SetActorLocation(ActiveRampComponentProxy->GetActorLocation() + OffsetVector);

				MakeBuildableFinalizable(ActiveRampComponentProxy);
			}
		}
		else //Hit but not a building Component that is snappable.
		{
			ActiveRampComponentProxy->SetActorLocation(FirstHitBuildingComponentHitResult.ImpactPoint);

			MakeBuildableFinalizable(ActiveRampComponentProxy);
		}

		bool IsRampBlocked = ActiveRampComponentProxy->bIsRampProxyBlocked();
		if (IsRampBlocked)
		{
			MakeBuildableNotFinalizable(ActiveRampComponentProxy);
		}
	}
}

FBuildablePlacementData UBuildSystemManagerComponent::GetTrapSnapTransform(FVector ImpactPoint, ATimberBuildingComponentBase* BuildingComponent, ATrapBase* TrapComponentProxy)
{
	// Default Snap Data is facing the player at the impact point
	FBuildablePlacementData TrapSnapData;
	TrapSnapData.TrapLocation = ImpactPoint;
	TrapSnapData.TrapRotation = GetOwner()->GetActorRotation();
	//TrapSnapData.TrapRotation.Yaw += 90;

	//DrawDebugSphere(GetWorld(), ImpactPoint, 5.0f, 12, FColor::Red, false, 0.1f, 0, 1.0f);
	if (BuildingComponent->FrontCenterSnapPoint && BuildingComponent->BackCenterSnapPoint)
	{
		/* Getting Distance from impact point to the closest scene component
		 * that would always give the side of the wall/floor/ceiling's Snap location
		 */
		FVector FrontTrapSnapLocation = BuildingComponent->FrontCenterSnapPoint->GetComponentTransform().GetLocation();
		//FrontTrapSnapLocation.Z -= 200.f;
		FVector BackTrapSnapLocation = BuildingComponent->BackCenterSnapPoint->GetComponentTransform().GetLocation();
		float LengthToFrontTrapSnap = FVector::Dist(ImpactPoint, FrontTrapSnapLocation);
		float LengthToBackTrapSnap = FVector::Dist(ImpactPoint, BackTrapSnapLocation);

		//UE_LOG(LogTemp, Warning, TEXT("Length to Front Snap: %f. Length to Back Snap: %f."), LengthToFrontTrapSnap, LengthToBackTrapSnap);

		if (LengthToFrontTrapSnap < LengthToBackTrapSnap)
		{
			//If the FrontSnap is empty, assign the TrapComponent to the FrontSnap, else do not snap to component.
			if (BuildingComponent->FrontCenterAttachment == nullptr)
			{
				//The actual Scene Component Representing to Snap Location should have the correct Rotation.
				TrapSnapData.TrapLocation = FrontTrapSnapLocation;
				TrapSnapData.TrapRotation = BuildingComponent->FrontCenterSnapPoint->GetComponentTransform().GetRotation().Rotator();
				TrapComponentProxy->BuildingComponentTrapDirection = EBuildingComponentTrapDirection::Front;
				//UE_LOG(LogTemp, Warning, TEXT("Attacking Center Snap Buildable to Front Snap."));
			}
			else
			{
				TrapComponentProxy->BuildingComponentTrapDirection = EBuildingComponentTrapDirection::Default;
				GEngine->AddOnScreenDebugMessage(7, 5.0f, FColor::Red, "FrontSnap Taken.");
			}
		}
		else
		{
			if (BuildingComponent->BackCenterAttachment == nullptr)
			{
				//The actual Scene Component Representing to Snap Location should have the correct Rotation.
				TrapSnapData.TrapLocation = BackTrapSnapLocation;
				TrapSnapData.TrapRotation = BuildingComponent->BackCenterSnapPoint->GetComponentTransform().GetRotation().Rotator();
				TrapComponentProxy->BuildingComponentTrapDirection = EBuildingComponentTrapDirection::Back;
				//UE_LOG(LogTemp, Warning, TEXT("Attacking Center Snap Buildable to Back Snap."));
			}
			else
			{
				TrapComponentProxy->BuildingComponentTrapDirection = EBuildingComponentTrapDirection::Default;
			}
		}
	}
	else
	{
		TrapComponentProxy->BuildingComponentTrapDirection = EBuildingComponentTrapDirection::Default;
	}
	return TrapSnapData;
}



