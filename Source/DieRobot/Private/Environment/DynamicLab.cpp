// Property of Paracosm Industries. Dont use my shit.
#include "Environment/DynamicLab.h"

#include "Components/InstancedStaticMeshComponent.h"


// Sets default values
ADynamicLab::ADynamicLab()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
}

// Called when the game starts or when spawned
void ADynamicLab::BeginPlay()
{
	Super::BeginPlay();
}

void ADynamicLab::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	BuildLab();
}

// Called every frame
void ADynamicLab::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADynamicLab::BuildLab()
{
	SetupInstancedWallStaticMeshComponent();
	SetupInstancedCeilingStaticMeshComponent();
	SetDimensionsFromState(LabDimension);
	EmptyChildComponentArray();
	HandleFloorLayout();
	HandleWallLayout(LabDimension);
	HandleBossSpawnArea();
	HandleEnemySpawnArea();
}

void ADynamicLab::SetDimensionsFromState(ELabDimension LabDimensionEnum)
{
	/*These are template Dimensions.
	 *
	 * Lab Dimensions Number Order:
	 * X =  In + X Direction (Typically the Longest Run of the Lab.)(Length)
	 * Y = In + Y Direction (Typically the Shortest Run of the Lab.)(Width)(Doors Created on this Axis.)
	 * Z = In + Z Direction (Typically the Height of the Lab.)(Levels)
	 */
	switch (LabDimensionEnum)
	{
	case ELabDimension::S :
		UpdateLabDimensions(3,3,3);
		break;
	case ELabDimension::M :
		UpdateLabDimensions(6,6,3);
		break;
	case ELabDimension::L :
		UpdateLabDimensions( 16, 8, 3);
		break;
	case ELabDimension::XL :
		UpdateLabDimensions( 16, 12, 4);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Incompatible Lab Dimension in Lab State"));
		break;
	}
}

void ADynamicLab::UpdateLabDimensions(int x, int y, int z)
{
	LabLength = x;
	LabWidth = y;
	LabLevels = z;
}

void ADynamicLab::EmptyChildComponentArray()
{
	if (ChildComponents.Num() > 0)
	{
		for (UChildActorComponent* ChildActor : ChildComponents)
		{
			if (ChildActor)
			{
				ChildActor->DestroyComponent();
			}
		}
		ChildComponents.Empty();
	}
}

void ADynamicLab::HandleFloorLayout()
{
	NumberOfFloors = 0;
	NumberOfCeilingTiles = 0;
	//Looping through Length -> +X Direction
	for (int i = 0; i <= LabLength -1; i++ )
	{
		//Looping through Width -> +Y Direction
		for (int k = 0; k <= LabWidth -1; k++ )
		{
			//Handling Lab Floor Tile Generation
			if (EnvironmentFloors)
			{
				int LengthIncrement = i; //(0,1,2,3,4)
				int WidthIncrement = k; //(0,1,2,3,4)
				FVector FloorSpawnLocation = FVector::ZeroVector;
				FloorSpawnLocation.X = LengthIncrement * FloorSizeX;
				FloorSpawnLocation.Y = WidthIncrement * FloorSizeY;

				GenerateChildComponent(EnvironmentFloors, FloorSpawnLocation, FRotator::ZeroRotator);
				NumberOfFloors++;
			}

			//Handling Lab Ceiling Tile Generation
			if (EnvironmentFloors)
			{
				int LengthIncrement = i; //(0,1,2,3,4)
				int WidthIncrement = k; //(0,1,2,3,4)
				FVector CeilingSpawnLocation = FVector::ZeroVector;
				CeilingSpawnLocation.X = LengthIncrement * FloorSizeX;
				CeilingSpawnLocation.Y = WidthIncrement * FloorSizeY;
				
				float CeilingHeight = WallSizeZ * LabLevels;
				CeilingSpawnLocation.Z = CeilingHeight;

				//GenerateChildComponent(EnvironmentFloors, CeilingSpawnLocation, FRotator::ZeroRotator);
				//GenerateStaticMeshComponent(FloorMesh, CeilingSpawnLocation, FRotator::ZeroRotator);
				GenerateInstancedCeilingStaticMeshComponent(CeilingSpawnLocation, FRotator::ZeroRotator);
				

				/*Handling Lighting Placement*/
				// If on the First Length Tile or On an Even Tile Increment
				if (CeilingLights)
				{
					if (LengthIncrement == 0 || LengthIncrement % 2 == 0)
					{
						if (WidthIncrement == 0 || WidthIncrement % 2 == 0)
						{
							FVector LightSpawnLocation = CeilingSpawnLocation;
							
							//Moves Light Spawn Locations +Z to Ceiling Height, Then Drops it down 50.f to be just below the Ceiling
							LightSpawnLocation.Z -=  CeilingLightDropOffset;

							//Handles Ceiling Light Generation
							HandleCeilingLightLayout(LightSpawnLocation);
							NumberOfCeilingLights++;
						}
					}
					else
					{
						if (WidthIncrement != 0 && WidthIncrement % 2 != 0)
						{
							FVector LightSpawnLocation = CeilingSpawnLocation;
							
							//Moves Light Spawn Locations +Z to Ceiling Height, Then Drops it down 50.f to be just below the Ceiling
							LightSpawnLocation.Z -= CeilingLightDropOffset;

							//Handles Ceiling Light Generation
							HandleCeilingLightLayout(LightSpawnLocation);
							NumberOfCeilingLights++;
						}
					}
				}
			}
		}
	}
}

void ADynamicLab::HandleWallLayout(ELabDimension InLabDimension)
{
	NumberOfWalls = 0;
	//How many Levels of Walls. (Vertical)
	for (int i = 0; i <= LabLevels - 1; i++ )
	{
		int LevelIncrement = i;
		FVector InitialWallLocationLength = FVector::ZeroVector;
		InitialWallLocationLength.Y -= 205.0f;
		InitialWallLocationLength.Z += (LevelIncrement) * WallSizeZ;

		//How many "Lengths" of the walls -> +X Direction
		for (int k = 0; k <= LabLength - 1; k++ )
		{
			if (WallMesh)
			{
				//Walls Closer to 0 X
				//Using the Increment to calculate the Location of the wall.
				FVector UpdatedWallLocation = InitialWallLocationLength;
				UpdatedWallLocation.X += k * FloorSizeX;
				//GenerateChildComponent(EnvironmentWalls, UpdatedWallLocation, FRotator::ZeroRotator);
				//GenerateStaticMeshComponent(WallMesh, UpdatedWallLocation, FRotator::ZeroRotator);
				GenerateInstancedWallStaticMeshComponent(UpdatedWallLocation, FRotator::ZeroRotator);

				//Walls Closer to 1 X
				FVector AdjacentWallLocation = UpdatedWallLocation;
				AdjacentWallLocation.Y += FloorSizeX * LabWidth + 15;
				//GenerateChildComponent(EnvironmentWalls, AdjacentWallLocation, FRotator::ZeroRotator);
				//GenerateStaticMeshComponent(WallMesh, AdjacentWallLocation, FRotator::ZeroRotator);
				GenerateInstancedWallStaticMeshComponent(AdjacentWallLocation, FRotator::ZeroRotator);
			}
		}

		FVector InitialWallLocationWidth = FVector::ZeroVector;
		InitialWallLocationWidth.X -= 205.0f;
		InitialWallLocationWidth.Z += (LevelIncrement) * WallSizeZ;
		FRotator AdjustedWallRotation = FRotator::ZeroRotator;
		AdjustedWallRotation.Yaw += 90.0f;

		/*Garage Door Side*/
		for (int j = 0; j <= LabWidth - 1; j++ )
		{
			if (WallMesh && GarageDoor)
			{
				FVector UpdatedWallLocation = InitialWallLocationWidth;
				UpdatedWallLocation.Y += j * FloorSizeX;

				if (InLabDimension == ELabDimension::L)
				{
					//At the third index place the Garage Door.
					if ( j == 3 || j == 4 )
					{
						//If on the 3rd Index on 1st floor, Place the Garage Door.
						if ( j == 3 && i == 0)
						{
							FVector GarageDoorLocationAdjustment = UpdatedWallLocation;
							//Shifting the garage door 200.
							GarageDoorLocationAdjustment.Y += 200.f;
							GarageDoorLocationAdjustment.X -= 10.0f;
							
							FRotator GarageDoorRotation = FRotator::ZeroRotator;
							GarageDoorRotation.Yaw -= 90.0f;
							GenerateChildComponent(GarageDoor, GarageDoorLocationAdjustment, GarageDoorRotation);
							NumberOfLabDoor++;
							
						}
						else if (i == 0 || i == 1)
						{
							//Skipping placing walls in locations that would overlap the Garage door.
							continue;
						}
						else
						{
							//This handles the situation where Above the Garage Door on Index j=3 && j=4, we still want walls. Walls Directly above garage door.
							//GenerateChildComponent(EnvironmentWalls, UpdatedWallLocation, AdjustedWallRotation);
							//GenerateStaticMeshComponent(WallMesh, UpdatedWallLocation, AdjustedWallRotation);
							GenerateInstancedWallStaticMeshComponent(UpdatedWallLocation, AdjustedWallRotation);
						}
					}
					else
					{
						//Handles all other wall placements.
						//GenerateChildComponent(EnvironmentWalls, UpdatedWallLocation, AdjustedWallRotation);
						//GenerateStaticMeshComponent(WallMesh, UpdatedWallLocation, AdjustedWallRotation);
						GenerateInstancedWallStaticMeshComponent(UpdatedWallLocation, AdjustedWallRotation);
					}

				}
			}
		}

		/*Lab Door Side*/
		for (int l = 0; l <=LabWidth-1; l++)
		{
			//Walls Closer to 0 Y
			FVector UpdatedWallLocation = InitialWallLocationWidth;
			UpdatedWallLocation.Y += l * FloorSizeX;
			FVector AdjacentWallLocation = UpdatedWallLocation;
			AdjacentWallLocation.X += FloorSizeY * LabLength + 15;

			//If on the first Floor Level
			if (i == 0 && InLabDimension == ELabDimension::L)
			{
				//In Positions 2 & 5, Create Lab Door, Skip Wall Placement.
				if ( l == 2 || l == 5)
				{
					if (LabDoors)
					{
						AdjacentWallLocation.X += 20.0f;
						GenerateChildComponent(LabDoors, AdjacentWallLocation, AdjustedWallRotation);
						NumberOfLabDoor++;

						//this exits this loops iteration so we don't place a wall on top of the door.
						continue;
					}
				}
				GenerateInstancedWallStaticMeshComponent(AdjacentWallLocation, AdjustedWallRotation);
			}
			GenerateInstancedWallStaticMeshComponent(AdjacentWallLocation, AdjustedWallRotation);
			//GenerateChildComponent(EnvironmentWalls, AdjacentWallLocation, AdjustedWallRotation);
			//GenerateStaticMeshComponent(WallMesh, AdjacentWallLocation, AdjustedWallRotation);
		}
	}
}

void ADynamicLab::HandleCeilingLightLayout(FVector Location)
{
	if (CeilingLights)
	{
		GenerateChildComponent(CeilingLights, Location, FRotator::ZeroRotator);
	}
}

void ADynamicLab::HandleBossSpawnArea()
{

	//Handling Floors.
	//Boss Area Always have 3 Floors for Length +X Direction
	if (EnvironmentFloors)
	{
		for (int i = 0; i < 3; i++)
		{
			for (int k = 0; k < LabWidth; k++)
			{
				FVector BossAreaLocation = FVector::ZeroVector;
				//For every increamenet we move in the X Direction the floor size units more.
				BossAreaLocation.X -= (i + 1) * FloorSizeX;
				BossAreaLocation.Y += k * FloorSizeY;
				GenerateChildComponent(EnvironmentFloors, BossAreaLocation, FRotator::ZeroRotator);
				NumberOfFloors++;

				//Handle Ceiling piece above new Floor Piece.
				float ZFloorOffset = WallSizeZ * LabLevels;
				BossAreaLocation.Z += ZFloorOffset;
				//GenerateChildComponent(EnvironmentFloors, BossAreaLocation, FRotator::ZeroRotator);
				GenerateInstancedCeilingStaticMeshComponent(BossAreaLocation, FRotator::ZeroRotator);
			}
		}
	}

	/*
	 * Handling Walls
	 *
	 * We need to Create the 3 Sides of the Walls.
	 */

	
	if (WallMesh)
	{
		for (int i = 0; i < 3; i++)
		{
			for (int k = 0; k < LabLevels; k++)
			{
				//Walls on X axis 
				//For every increment we move in the X Direction the floor size units more.
				
				FVector BossAreaLocation = FVector::ZeroVector;
				BossAreaLocation.Y -= 205.0f;
				BossAreaLocation.X -= (i + 1) * FloorSizeX;	
				BossAreaLocation.Z += k * WallSizeZ;
				//GenerateChildComponent(EnvironmentWalls, BossAreaLocation, FRotator::ZeroRotator);
				//GenerateStaticMeshComponent(WallMesh, BossAreaLocation, FRotator::ZeroRotator);
				GenerateInstancedWallStaticMeshComponent(BossAreaLocation, FRotator::ZeroRotator);
				
				FVector AdjacentWallLocation = BossAreaLocation;
				AdjacentWallLocation.Y += FloorSizeX * LabWidth + 5;
				//GenerateChildComponent(EnvironmentWalls, AdjacentWallLocation, FRotator::ZeroRotator);
				//GenerateStaticMeshComponent(WallMesh, AdjacentWallLocation, FRotator::ZeroRotator);
				GenerateInstancedWallStaticMeshComponent(AdjacentWallLocation, FRotator::ZeroRotator);
			}
		}
	}
	if (WallMesh)
	{
		for (int i = 0; i < LabWidth; i++)
		{
			for (int k = 0; k < LabLevels; k++)
			{
				//Walls on Y Axis
				//Horizontal Placement of the Wall
				FVector BossAreaLocation = FVector::ZeroVector;
				FVector BackWallLocation = BossAreaLocation;
				BackWallLocation.X -= 1390.0f;
				FRotator AdjustedWallRotation = FRotator::ZeroRotator;
				AdjustedWallRotation.Yaw += 90.0f;
				BackWallLocation.Y += i * FloorSizeY;
				BackWallLocation.Z += k * WallSizeZ;
				//GenerateChildComponent(EnvironmentWalls, BackWallLocation, AdjustedWallRotation);
				//GenerateStaticMeshComponent(WallMesh, BackWallLocation, AdjustedWallRotation);
				GenerateInstancedWallStaticMeshComponent(BackWallLocation, AdjustedWallRotation);
				
			}
		}
	}
	
	
}

void ADynamicLab::HandleEnemySpawnArea()
{
	//Handling Floors.
	//Boss Area Always have 3 Floors for Length +X Direction
	if (EnvironmentFloors)
	{
		for (int i = 0; i < 3; i++)
		{
			for (int k = 0; k < LabWidth; k++)
			{
				FVector EnemySpawnAreaStartLocation = FVector::ZeroVector;
				//For every incremenet we move in the X Direction the floor size units more.
				EnemySpawnAreaStartLocation.X += (LabLength + i) * FloorSizeX;
				EnemySpawnAreaStartLocation.Y += k * FloorSizeY;
				GenerateChildComponent(EnvironmentFloors, EnemySpawnAreaStartLocation, FRotator::ZeroRotator);
				NumberOfFloors++;

				//Handle Ceiling piece above new Floor Piece.
				float ZFloorOffset = WallSizeZ * LabLevels;
				EnemySpawnAreaStartLocation.Z += ZFloorOffset;
				//GenerateChildComponent(EnvironmentFloors, EnemySpawnAreaStartLocation, FRotator::ZeroRotator);
				GenerateInstancedCeilingStaticMeshComponent(EnemySpawnAreaStartLocation, FRotator::ZeroRotator);
			}
		}
	}

	//Handling Walls in the X Direction | |
	if (WallMesh)
	{
		//Spawn Areas are always 3 Floor in Length X Width Amount
		for (int i = 0; i < 3; i++)
		{
			for (int k = 0; k < LabLevels; k++)
			{
				//Walls on X axis 
				//For every increment we move in the X Direction the floor size units more.
				
				FVector SpawnAreaLocation = FVector::ZeroVector;
				SpawnAreaLocation.Y -= 205.0f;
				SpawnAreaLocation.X += (LabLength + i) * FloorSizeX;	
				SpawnAreaLocation.Z += k * WallSizeZ;
				//GenerateChildComponent(EnvironmentWalls, SpawnAreaLocation, FRotator::ZeroRotator);
				//GenerateStaticMeshComponent(WallMesh, SpawnAreaLocation, FRotator::ZeroRotator);
				GenerateInstancedWallStaticMeshComponent(SpawnAreaLocation, FRotator::ZeroRotator);
				
				
				FVector AdjacentWallLocation = SpawnAreaLocation;
				AdjacentWallLocation.Y += FloorSizeX * LabWidth + 5;
				//GenerateChildComponent(EnvironmentWalls, AdjacentWallLocation, FRotator::ZeroRotator);
				//GenerateStaticMeshComponent(WallMesh, AdjacentWallLocation, FRotator::ZeroRotator);
				GenerateInstancedWallStaticMeshComponent(AdjacentWallLocation, FRotator::ZeroRotator);
				
			}
		}
	}

	//Handling Walls in the Y Direction  < -- >
	if (WallMesh)
	{
		for (int i = 0; i < LabWidth; i++)
		{
			for (int k = 0; k < LabLevels; k++)
			{
				//Walls on Y Axis
				//Horizontal Placement of the Wall
				FVector SpawnAreaLocation = FVector::ZeroVector;
				FVector BackWallLocation = SpawnAreaLocation;
				
				BackWallLocation.X += (LabLength + 3) * FloorSizeX - 200.0f;
				FRotator AdjustedWallRotation = FRotator::ZeroRotator;
				AdjustedWallRotation.Yaw += 90.0f;
				BackWallLocation.Y += i * FloorSizeY;
				BackWallLocation.Z += k * WallSizeZ;
				//GenerateChildComponent(EnvironmentWalls, BackWallLocation, AdjustedWallRotation);
				//GenerateStaticMeshComponent(WallMesh, BackWallLocation, AdjustedWallRotation);
				GenerateInstancedWallStaticMeshComponent(BackWallLocation, AdjustedWallRotation);
				
			}
		}
	}
}

void ADynamicLab::GenerateChildComponent(TSubclassOf<AActor> BuildableActor, FVector Location, FRotator Rotation)
{
	//Create the New Component
	UChildActorComponent* ChildActorComponent = NewObject<UChildActorComponent>(this,  UChildActorComponent::StaticClass());
	//Attach it to the Root Component of the BP_Lab
	ChildActorComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	//Set the Child Actor Class - This Can be the BP of Walls, Floor, Lights, Doors Etc.
	ChildActorComponent->SetChildActorClass(BuildableActor);
	//Set Location
	ChildActorComponent->SetRelativeLocation(Location);
	//Set Rotation
	ChildActorComponent->SetRelativeRotation(Rotation);
	ChildActorComponent->RegisterComponent();
	
	ChildComponents.Add(ChildActorComponent);
	
}

void ADynamicLab::GenerateStaticMeshComponent(UStaticMesh* StaticMesh, FVector Location, FRotator Rotation)
{
	if (StaticMesh)
	{
		UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>(this);
		StaticMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		StaticMeshComponent->SetStaticMesh(StaticMesh);

		// Set location/rotation/scale as needed
		StaticMeshComponent->SetRelativeLocation(Location);
		StaticMeshComponent->SetRelativeRotation(Rotation);

		// Register so it shows up in the editor
		StaticMeshComponent->RegisterComponent();
		StaticMeshComponent->bCastDynamicShadow = false;
		StaticMeshComponentsArray.Add(StaticMeshComponent);
	}
}

void ADynamicLab::GenerateInstancedWallStaticMeshComponent(FVector Location, FRotator Rotation)
{
	const FTransform InstanceTransform(Rotation, Location);
	InstancedWallStaticMesh->AddInstance(InstanceTransform);

	//Used for Lighting Cube Map Pass Through
	InstancedWallStaticMesh->SetCastShadow(false);
	InstancedWallStaticMesh->bAffectDynamicIndirectLighting = false;
	InstancedWallStaticMesh->bAffectDistanceFieldLighting = false;
	InstancedWallStaticMesh->MarkRenderStateDirty();
	
	NumberOfWalls++;
	//UE_LOG(LogTemp, Warning, TEXT("Instanced Wall Static Mesh Component Created"));
}

void ADynamicLab::GenerateInstancedCeilingStaticMeshComponent(FVector Location, FRotator Rotation)
{
	const FTransform InstanceTransform(Rotation, Location);
	InstancedCeilingStaticMesh->AddInstance(InstanceTransform);
	NumberOfCeilingTiles++;
}

void ADynamicLab::SetupInstancedWallStaticMeshComponent()
{
	if (InstancedWallStaticMesh)
	{
		InstancedWallStaticMesh->ClearInstances();
	}
	InstancedWallStaticMesh = NewObject<UInstancedStaticMeshComponent>(this);
	InstancedWallStaticMesh->SetupAttachment(RootComponent);
	InstancedWallStaticMesh->SetStaticMesh(WallMesh);
	InstancedWallStaticMesh->RegisterComponent();
	InstancedWallStaticMesh->SetCollisionProfileName("DR_BuildableBlockEverything");
	InstancedWallStaticMesh->bCastDynamicShadow = false;

	
}

void ADynamicLab::SetupInstancedCeilingStaticMeshComponent()
{
	if (InstancedCeilingStaticMesh)
	{
		InstancedCeilingStaticMesh->ClearInstances();
	}
	InstancedCeilingStaticMesh = NewObject<UInstancedStaticMeshComponent>(this);
	InstancedCeilingStaticMesh->SetupAttachment(RootComponent);
	InstancedCeilingStaticMesh->SetStaticMesh(FloorMesh);
	InstancedCeilingStaticMesh->RegisterComponent();
	InstancedCeilingStaticMesh->ClearInstances();
	InstancedCeilingStaticMesh->SetCollisionProfileName("DR_BuildableBlockEverything");
	InstancedCeilingStaticMesh->bCastDynamicShadow = false;
}
