// Property of Paracosm Industries.


#include "Character/Enemies/TimberEnemyCharacter.h"
#include "MetasoundSource.h"
#include "AI/TimberAiControllerBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BuildSystem/BuildingComponents/TimberBuildingComponentBase.h"
#include "Character/TimberPlayableCharacter.h"
#include "Character/Enemies/TimberEnemyMeleeWeaponBase.h"
#include "Character/Enemies/TimberEnemyRangedBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/Navigation/NavigationHelperComponent.h"
#include "Components/StatusEffect/StatusEffectHandlerComponent.h"
#include "Controller/TimberPlayerController.h"
#include "Data/DataAssets/LootTable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameModes/TimberGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Loot/EnemyLootDropBase.h"
#include "Subsystems/Events/MissionEventSubsystem.h"
#include "Subsystems/Wave/WaveGameInstanceSubsystem.h"
#include "Types/Combat/DamagePayload.h"
#include "DieRobot/Public/UI/FloatingData/FloatingDataContainer.h"
#include "UI/EnemyDataCluster/EnemyDataCluster.h"
#include "Weapons/TimberWeaponMeleeBase.h"
#include "Weapons/TimberWeaponRangedBase.h"

ATimberEnemyCharacter::ATimberEnemyCharacter()
{
	RaycastStartPoint = CreateDefaultSubobject<USceneComponent>("RaycastStartPoint");
	RaycastStartPoint->SetupAttachment(RootComponent);

	/*StatusEffectBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("StatusEffectBar");
	StatusEffectBarWidgetComponent->SetupAttachment(RootComponent);*/

	DataClusterWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("DataClusterWidgetComponent");
	DataClusterWidgetComponent->SetupAttachment(RootComponent);
	
	FloatingDataSpawnLocation = CreateDefaultSubobject<USceneComponent>("DamageEffectUISpawnPoint");
	FloatingDataSpawnLocation->SetupAttachment(RootComponent);
	
	StatusEffectHandler = CreateDefaultSubobject<UStatusEffectHandlerComponent>("StatusEffectHandler");
	NavHelperComponent = CreateDefaultSubobject<UNavigationHelperComponent>("NavHelperComponent");
}

void ATimberEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetupDataClusterComponent();
	
	SetupCharacterMovementDelegates();
	
	SetupCharacterMovementData();

	ScaleHealth();
}

void ATimberEnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ATimberEnemyCharacter::SelfDestruct()
{
	//TODO:: Add VFX for Destruction of Enemy.
	//Potentially need to delay the rest of this logic for the end of the Destruction VFX.
	StopAiControllerBehaviorTree();

	//We want this actor not collidable so nothing interrupts the Montage about to play.
	OnDeath_HandleCollision();
	
	//Potentially not necessary as destruction completely stops the AI Controller.
	HandleRemoveStatusEffectComponent();

	//Adjusts the Wave Enemy Count Number
	GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>()->CheckArrayForEnemy(this);


	//Calling Destroy in Animation Notify on this Montage.
	if (IsValid(SelfDestructMontage))
	{
		PlayAnimMontage(SelfDestructMontage, 1);
	}
	

	UE_LOG(LogTemp, Warning, TEXT("Actor Self Destructed."));
}

void ATimberEnemyCharacter::SpawnDamageUI(FDamagePayload DamagePayload)
{
	//If there isn't a valid container actor ref and there is a valid container class.
	if (!IsValid(FloatingDamageContainerActor) && FloatingDamageContainerClass)
	{
		FActorSpawnParameters SpawnParams;
		
		AActor* FloatingDamageActor = GetWorld()->SpawnActor<AActor>(FloatingDamageContainerClass,
			FloatingDataSpawnLocation->GetRelativeLocation(),
			FRotator::ZeroRotator, 
			SpawnParams);

		//FloatingDamageActor->AttachToComponent(FloatingDataSpawnLocation, FAttachmentTransformRules::KeepRelativeTransform);
		if (AFloatingDataContainer* FloatingDamage = Cast<AFloatingDataContainer>(FloatingDamageActor))
		{
			if (!IsValid(FloatingDamage)) return;
			
			FloatingDamage->SpawnSceneComponentRef = FloatingDataSpawnLocation;
			
			//Set for Reuse during animation.
			FloatingDamageContainerActor = FloatingDamage;
			FloatingDamage->SetDamageAmount(DamagePayload.DamageAmount);
			FloatingDamage->SetColor(DamagePayload.StatusEffect.GetEffectColor());
			FloatingDamage->SetSize(DamagePayload.StatusEffect.GetEffectTextSize());
		}
	}
	else
	{
		//Updating existing Spawned Actor.
		//FloatingDamageContainerActor->SpawnSceneComponentRef = FloatingDataSpawnLocation;
		FloatingDamageContainerActor->SetDamageAmount(DamagePayload.DamageAmount);
		FloatingDamageContainerActor->SetColor(DamagePayload.StatusEffect.GetEffectColor());
		FloatingDamageContainerActor->SetSize(DamagePayload.StatusEffect.GetEffectTextSize());
	}
}

void ATimberEnemyCharacter::ClearFloatingDamageRef()
{
	FloatingDamageContainerActor = nullptr;
}

void ATimberEnemyCharacter::SpawnEffectNameUI(FName EffectName, UStatusEffectBase* StatusEffect)
{
	if (FloatingDamageContainerClass)
	{
		FActorSpawnParameters SpawnParams;
		AActor* FloatingDamageActor = GetWorld()->SpawnActor<AActor>(FloatingDamageContainerClass,
			FloatingDataSpawnLocation->GetComponentLocation(),
			FRotator::ZeroRotator, 
			SpawnParams);
		
		if (AFloatingDataContainer* FloatingDataContainer = Cast<AFloatingDataContainer>(FloatingDamageActor))
		{
			if (!IsValid(FloatingDataContainer)) return;
			FloatingDataContainer->SpawnSceneComponentRef = FloatingDataSpawnLocation;
			FloatingDataContainer->SetIsDamage(false);
			FloatingDataContainer->SetEffectText(EffectName);
			FloatingDataContainer->SetColor(StatusEffect->StatusEffect.GetEffectColor());	
			FloatingDataContainer->SetSize(StatusEffect->StatusEffect.GetEffectTextSize());
		}
		
	}
}

void ATimberEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ATimberEnemyCharacter::SetupCharacterMovementData()
{
	if (IsValid(CharacterMovementComponent))
	{
		CharacterMovementComponent->SetWalkableFloorAngle(65.0f);
		CharacterMovementComponent->MaxWalkSpeed = MaxWalkSpeedBase;
		CharacterMovementComponent->MaxStepHeight = 60.0f;

		//Used to not slow down in between Nav Points.
		CharacterMovementComponent->BrakingDecelerationWalking = 0.0f;
		
		CharacterMovementComponent->NavAgentProps.AgentRadius = 34.0f;
		CharacterMovementComponent->NavAgentProps.AgentHeight = 180.0f;
		CharacterMovementComponent->NavAgentProps.AgentStepHeight = 60.0f;

		//TODO:: Check a way of Reimplementing this in the future. RVO may be beneficial for this. 
		CharacterMovementComponent->bUseRVOAvoidance = false;
		CharacterMovementComponent->AvoidanceConsiderationRadius = 400.0f;
		CharacterMovementComponent->AvoidanceWeight = 0.5f;
	}
}

void ATimberEnemyCharacter::SetupCharacterMovementDelegates()
{
	CharacterMovementComponent = GetCharacterMovement();
	if (IsValid(CharacterMovementComponent))
	{
		MovementModeChangedDelegate.AddDynamic(this, &ATimberEnemyCharacter::HandleOnMovementModeChanged);
		LandedDelegate.AddDynamic(this, &ATimberEnemyCharacter::HandleOnLanded);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Character Movement Component is NULL."));
	}
}

void ATimberEnemyCharacter::HandleOnMovementModeChanged(class ACharacter* Character, EMovementMode PrevMovementMode,
	uint8 PreviousCustomMode)
{
	if (Character)
	{
		ATimberAiControllerBase* AiController = Cast<ATimberAiControllerBase>(GetController());
		if (IsValid(AiController))
		{
			switch (Character->GetCharacterMovement()->MovementMode.GetValue())
			{
			case MOVE_Falling:
				AiController->BehaviorTreeComponent->StopLogic("Enemy Character is Falling.");
				break;
			default:
				// Handle other movement modes if necessary
				break;
			}
		}
	}
}

void ATimberEnemyCharacter::HandleOnLanded(const FHitResult& Hit)
{
	//Ai Controller to Restart Ai Controller Brain - this should restart the pathing solution.
	ATimberAiControllerBase* AiController = Cast<ATimberAiControllerBase>(GetController());
	if (IsValid(AiController))
	{
		//Restarting the Behavior tree Logic if the AI Controller is not already running.
		if (IsValid(AiController->BehaviorTreeComponent) && !AiController->BehaviorTreeComponent->IsRunning())
		{
			AiController->BehaviorTreeComponent->StartLogic();
		}
	}
}

FMissionEventPayload ATimberEnemyCharacter:: GenerateDamageEventPayload(FDamagePayload DamagePayload)
{
	FMissionEventPayload Payload;
	Payload.EventTag = FGameplayTag::RequestGameplayTag("Event.Combat.Damage");
	Payload.InstigatingActor = DamagePayload.DamageInstigator;
	Payload.TargetActor = this;
	Payload.Count = DamagePayload.DamageAmount;
	PopulateCombatEventContextTags(Payload.ContextTags, DamagePayload);

	return Payload;
}

FMissionEventPayload ATimberEnemyCharacter::GenerateDestroyEventPayload(FDamagePayload DamagePayload)
{
	FMissionEventPayload Payload;
	Payload.EventTag = FGameplayTag::RequestGameplayTag("Event.Combat.Destroy");
	Payload.InstigatingActor = DamagePayload.DamageInstigator;
	Payload.TargetActor = this;
	Payload.Count = 1;
	PopulateCombatEventContextTags(Payload.ContextTags, DamagePayload);

	return Payload;
}

void ATimberEnemyCharacter::PopulateCombatEventContextTags(FGameplayTagContainer& ContextTagsContainer, const FDamagePayload& DamagePayload) const
{
	//Hit Enemy Context Tag
	if (this->IsA(ATimberEnemyMeleeWeaponBase::StaticClass()))
	{
		ContextTagsContainer.AddTag(FGameplayTag::RequestGameplayTag("Event.Enemy.Carbonite.Bruiser"));
	}
	else if (this->IsA(ATimberEnemyRangedBase::StaticClass()))
	{
		ContextTagsContainer.AddTag(FGameplayTag::RequestGameplayTag("Event.Enemy.Carbonite.Shooter"));
	}
	else if (this->IsA(ATimberEnemyMeleeBase::StaticClass()))
	{
		ContextTagsContainer.AddTag(FGameplayTag::RequestGameplayTag("Event.Enemy.Carbonite.Grunt"));
	}

	
	//Instigator Context
	AActor* Src = DamagePayload.DamageInstigator;
	if (Src && Src->IsA(ATimberPlayableCharacter::StaticClass()))
	{
		ContextTagsContainer.AddTag(FGameplayTag::RequestGameplayTag("Event.Combat.Player"));
		TWeakObjectPtr<ATimberPlayableCharacter> Kip = Cast<ATimberPlayableCharacter>(Src);
		if (Kip.IsValid())
		{
			//Weapon Context
			TWeakObjectPtr<ATimberWeaponBase> Weapon =  Kip->CombatComponent->GetCurrentlyEquippedWeapon();
			if (Weapon.IsValid())
			{
				if (Weapon->IsA(ATimberWeaponMeleeBase::StaticClass()))
				{
					ContextTagsContainer.AddTag(FGameplayTag::RequestGameplayTag("Event.Weapon.Melee.Hammer"));
				}
				if (Weapon->IsA(ATimberWeaponRangedBase::StaticClass()))
				{
					ContextTagsContainer.AddTag(FGameplayTag::RequestGameplayTag("Event.Weapon.Ranged.Rifle"));
				}
			}
		}
	}
	else if (this->IsA(ATrapBase::StaticClass()))
	{
		ContextTagsContainer.AddTag(FGameplayTag::RequestGameplayTag("Event.Combat.Trap"));
	}
}


void ATimberEnemyCharacter::TakeDamage(FDamagePayload DamagePayload)
{
	if (IsValid(DamagePayload.DamageInstigator))
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyCharacter - TakeDamage() - %s took %f damage from %s."), *GetName(), DamagePayload.DamageAmount, *DamagePayload.DamageInstigator->GetName());
	}
	
	//Applying damage to Character Health
	CurrentHealth -= DamagePayload.DamageAmount;
	
	SpawnDamageUI(DamagePayload);
	
	//Chance for Hit React to Play - Hit Reacts Interrupt other Montages like Attack Montage so we want to limit it.
	float RandFloat = FMath::FRandRange(0.0f, 10.0f);
	if (RandFloat > 9.0f)
	{
		PlayAnimMontage(HitReactMontage, 1, FName("Back"));
	}
	
	//Glows the Enemy Character briefly when hit.
	AddOverlayMaterialToCharacter(HitMarkerOverlayMaterial, 0.3f);
	
	//Adding new damage to the accumulated damage window
	DamageAccumulatedDuringWindow += DamagePayload.DamageAmount;
	
	//Starting Damage Accumulation Window / Used for Aggro Conditions
	StartDamageTimerWindow();

	//Things to do after Death - 0 Life.
	if (CurrentHealth <= 0.f)
	{
		
		HandleRemoveStatusEffectComponent();
		
		//Checking if the enemy was part of the wave spawn system and thus needs to be tracked.
		GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>()->CheckArrayForEnemy(this);

		//Clears Aggro and Damage Set on the Damage Accumulation Window
		ResetDamageWindow();
		
		//Stops all AI Behavior
		StopAiControllerBehaviorTree();

		//Handles Collisions so the enemy cant be attacked more or block navigation
		OnDeath_HandleCollision();

		//Plays Death Animation
		if (IsValid(DeathMontage))
		{
			//Some Effects Freeze Anim Rate Scale (Set to 0.0), and an Anim Notify is what calls the Death/Destroy functionality.
			if (GetMesh())
			{
				GetMesh()->GlobalAnimRateScale = 1.0f;
				PlayMontageAtRandomSection(DeathMontage);
			}
			else
			{
				//Fall back if there is an issue with the Animation or Mesh Call.
				HandleEnemyDeath();
			}
		}

		//Broadcast to the Mission System 
		UMissionEventSubsystem* CombatEventSubsystem = GetGameInstance()->GetSubsystem<UMissionEventSubsystem>();
		const FMissionEventPayload Payload = GenerateDestroyEventPayload(DamagePayload);
		CombatEventSubsystem->BroadcastMissionEvent(Payload);
		
		OnDeath_DropLoot();

		return;
	}
	
	if (IsValid(DamagePayload.DamageInstigator))
	{
		//TODO:: Potential Removed from Game. - Delete Later is Unused still.
		bHasBeenAggroByPlayer = HandleAggroCheck(DamagePayload.DamageInstigator, DamagePayload.DamageAmount, DamageAccumulatedDuringWindow);
		//UE_LOG(LogTemp, Warning, TEXT("Target hit for: %f. CurrentHealth: %f."), DamageAmount, CurrentHealth);
	}
	
	//Damage Event
	UMissionEventSubsystem* CombatEventSubsystem = GetGameInstance()->GetSubsystem<UMissionEventSubsystem>();
	const FMissionEventPayload Payload = GenerateDamageEventPayload(DamagePayload);
	CombatEventSubsystem->BroadcastMissionEvent(Payload);
}

bool ATimberEnemyCharacter::HandleAggroCheck(AActor* DamageInstigator, float DamageReceived, float fDamageAccumulatedDuringWindow)
{
	//If DamageInstigator is a player character, check if the enemy should aggro the player.
	if (Cast<ATimberPlayableCharacter>(DamageInstigator))
	{
		// If damage accumulated during a damage window is greater than 20 or the enemy has lost more than 40% of its health, it will then aggro the player.
		if(DamageReceived > (MaxHealth * .40) || fDamageAccumulatedDuringWindow > 20 )
		{
			return true;
		}
	}
	
	return false;
}

void ATimberEnemyCharacter::ResetDamageWindow()
{
	DamageAccumulatedDuringWindow = 0.f;
}

void ATimberEnemyCharacter::StartDamageTimerWindow()
{
	bool bIsTimerActive = GetWorld()->GetTimerManager().IsTimerActive(DamageWindowTimerHandle);
	if(!bIsTimerActive)
	{
		// Starts the Damage Window Timer that starts at first hit for (X: DamageWindowTime) and automatically resets itself at the end of the window.
		GetWorld()->GetTimerManager().SetTimer(DamageWindowTimerHandle, this, &ATimberEnemyCharacter::ResetDamageWindow, DamageWindowTime, false);
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Damage Window timer is already active."))
	}
}

void ATimberEnemyCharacter::PlayProjectileHitSound(FHitResult HitResult)
{
	FVector HitLocation = HitResult.ImpactPoint;
	if (IsValid(ProjectileHitSound_MetaSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, ProjectileHitSound_MetaSound, HitLocation);
	}
}

void ATimberEnemyCharacter::PlayMeleeWeaponHitSound(FHitResult HitResult)
{
	//GEngine->AddOnScreenDebugMessage(1, 4, FColor::Red, "PlayMeleeHitSoundCalled.");

	FVector HitLocation = HitResult.ImpactPoint;
	if (MeleeHitSound_MetaSound)
	{
		//GEngine->AddOnScreenDebugMessage(1, 4, FColor::Red, "Playing Hit Sound.");
		UGameplayStatics::PlaySoundAtLocation(this, MeleeHitSound_MetaSound, HitLocation);
	}
}

void ATimberEnemyCharacter::OnDeath_HandleCollision()
{
	if (IsValid(GetCapsuleComponent()))
	{
		GetCapsuleComponent()->SetCollisionProfileName("DR_DeadCharacter");
	}
}

void ATimberEnemyCharacter::OnDeath_DropLoot()
{
	//UE_LOG(LogTemp, Warning, TEXT("Enemy Destroyed. Dropping Loot."))
	//Get the Loot Table
	if (IsValid(LootTable))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Loot Table is Valid."))
		//Loot Table has an array that will be looped over.
		TArray<FLootEntry> LootEntries = LootTable->LootEntries;
		//Each item in the Array is a FLootEntry Struct that has a TSubclassOf<AEnemyLootDropBase> and a TMap<int, float> AmountChance.
		if (LootEntries.Num() > 0)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Found Loot Entries."))
			//We loop over the array, and use the Structs PickAmount() function to get a random amount of loot to drop.
			for (FLootEntry Items : LootEntries)
			{

				//This breaks this loop iteration as the chance was not met.
				//Checking if any loot should drop at all.Drop Chance set in the Loot Table.
				float AnyLootDropChance = FMath::FRand();
				//UE_LOG(LogTemp, Warning, TEXT("AnyLootDropChance: %f. Item Drop Chance: %f."), AnyLootDropChance, Items.DropChance);
				if (AnyLootDropChance > Items.DropChance)
				{
					continue;
				}

				FActorSpawnParameters SpawnParams;
				int32 AmountToSpawn = Items.PickAmount();
				//UE_LOG(LogTemp, Warning, TEXT("Amount to Spawn: %d"), AmountToSpawn);
				//We then spawn the loot using the SpawnLoot() function.
				for (int i = 0; i < AmountToSpawn; i++)
				{
					if (IsValid(Items.LootItemClass))
					{
						AActor* SpawnedLootActor = GetWorld()->SpawnActor<AActor>(Items.LootItemClass,
							GetActorLocation(),
							FRotator::ZeroRotator, 
							SpawnParams);

						if (IsValid(SpawnedLootActor))
						{
							// Try to apply an upward impulse
							UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(SpawnedLootActor->GetRootComponent());
							if (IsValid(PrimitiveComp) && PrimitiveComp->IsSimulatingPhysics())
							{
								FVector Impulse = FVector(
									FMath::FRandRange(-100.0f, 100.0f), // small random left/right push
									FMath::FRandRange(-100.0f, 100.0f), // small random forward/backward push
									FMath::FRandRange(400.0f, 600.0f)   // strong upward push
								);

								PrimitiveComp->AddImpulse(Impulse, NAME_None, false);
							}
						}
					}
				}
			}
		}
	}
}

void ATimberEnemyCharacter::ScaleHealth()
{
	UWaveGameInstanceSubsystem* WaveGameInstance = GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>();
	if (IsValid(WaveGameInstance))
	{
		float WaveNum = WaveGameInstance->CurrentWaveNumber;

		//Using a Temp Var here because it can be changed per BP in Editor.
		float BaseMaxHealth = MaxHealth;

		/*
		 * Wave 1 = 0
		 * Wave 2 = 3.5
		 * Wave 3 = 7
		 */

		float AdjustmentValue;

		if (WaveNum < 7)
		{
			AdjustmentValue = 3.5f;
		}
		else if (WaveNum < 12)
		{
			AdjustmentValue = 7.5f;
		}
		else if (WaveNum < 12)
		{
			AdjustmentValue = 9.5f;
		}
		else if (WaveNum < 18)
		{
			AdjustmentValue = 11.5f;
		}
		else if (WaveNum < 23)
		{
			AdjustmentValue = 13.5f;
		}
		else
		{
			AdjustmentValue = 15.5f;
		}
		
		float ScaleValue = (WaveNum - 1) * AdjustmentValue / 100;
		float ScaledHealth = BaseMaxHealth + (BaseMaxHealth * ScaleValue);

		MaxHealth = ScaledHealth;
		CurrentHealth = ScaledHealth;

		//UE_LOG(LogTemp, Warning, TEXT("Base Health: %f. Scaled Health: %f, Scale Value: %f."), BaseMaxHealth, ScaledHealth, ScaleValue);
	}
	
}

void ATimberEnemyCharacter::HandleRemoveStatusEffectComponent()
{
	if (IsValid(StatusEffectHandler))
	{
		StatusEffectHandler->DestroyComponent();
	}
}

void ATimberEnemyCharacter::HandleToggleDataView(FInputActionValue Input)
{
	/*When holding down Tab
	 * Should show All Data - Status Effect Bar
	 * If not holding, hide the Status Effect Bar
	 */
	if (Input.Get<bool>()) //true
	{
		if (IsValid(DataClusterWidgetComponent) &&
			IsValid(DataClusterWidgetComponent->GetUserWidgetObject()) &&
			DataClusterWidgetComponent->GetUserWidgetObject()->GetVisibility() == ESlateVisibility::Hidden)
		{
			//Toggles the visibility of the Status Effect Bar
			DataClusterWidgetComponent->GetUserWidgetObject()->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else //false
	{
		if (IsValid(DataClusterWidgetComponent) &&
			IsValid(DataClusterWidgetComponent->GetUserWidgetObject()) &&
			DataClusterWidgetComponent->GetUserWidgetObject()->GetVisibility() == ESlateVisibility::Visible)
		{
			DataClusterWidgetComponent->GetUserWidgetObject()->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ATimberEnemyCharacter::HandleEnemyDeath()
{
	HandleWeaponDestruction();
	Destroy();
}

void ATimberEnemyCharacter::HandleWeaponDestruction()
{
	ATimberEnemyMeleeWeaponBase* MeleeWeaponEnemy = Cast<ATimberEnemyMeleeWeaponBase>(this);
	ATimberEnemyRangedBase* RangedWeaponEnemy = Cast<ATimberEnemyRangedBase>(this);

	if (IsValid(MeleeWeaponEnemy))
	{
		if (MeleeWeaponEnemy->EquippedWeapon)
		{
			MeleeWeaponEnemy->EquippedWeapon->Destroy();
		}
	}

	if (IsValid(RangedWeaponEnemy))
	{
		if (RangedWeaponEnemy->EquippedWeapon)
		{
			RangedWeaponEnemy->EquippedWeapon->Destroy();
		}
	}
}

float ATimberEnemyCharacter::CalculateOutputDamage(float Damage)
{
	return Damage;
}

void ATimberEnemyCharacter::StopAiControllerBehaviorTree()
{
	ATimberAiControllerBase* AiController = Cast<ATimberAiControllerBase>(GetController());
	if (IsValid(AiController) && IsValid(AiController->BehaviorTreeComponent))
	{
		//Does not abort the current task, but does FinishtheTask
		AiController->BehaviorTreeComponent->StopLogic("Enemy has been killed");
	}
}

void ATimberEnemyCharacter::PlayMontageAtRandomSection(UAnimMontage* Montage)
{
	if(IsValid(Montage))
	{
		int NumberOfMontageSections = Montage->GetNumSections();
		int RandomSection = FMath::RandRange(0, NumberOfMontageSections - 1);
		PlayAnimMontage(Montage, 1, Montage->GetSectionName(RandomSection));
	}
	
}

ATimberBuildingComponentBase* ATimberEnemyCharacter::LineTraceToSeeda()
{
	FVector RaycastStart = RaycastStartPoint->GetComponentLocation();

	//Get Seeda Location
	FVector RaycastEnd = Cast<ATimberGameModeBase>(GetWorld()->GetAuthGameMode())->SeedaLocation;

	TArray<FHitResult> HitResults;

	bool bHit = GetWorld()->LineTraceMultiByChannel(
		HitResults,
		RaycastStart,
		RaycastEnd,
		ECC_Visibility);

	if (bHit)
	{
		for (FHitResult HitActors : HitResults)
		{
			if (!IsValid(HitActors.GetActor())) continue;
			
			if (HitActors.GetActor()->IsA(ATimberBuildingComponentBase::StaticClass()))
			{
				return Cast<ATimberBuildingComponentBase>(HitActors.GetActor());
			}
		}
	}

	return nullptr;
}

void ATimberEnemyCharacter::SweepForActor(TSubclassOf<AActor> ActorToSweepFor, float SphereRadius, float DistanceToCheck)
{
	FCollisionShape Sphere = FCollisionShape::MakeSphere(SphereRadius);
	FVector SweepStart = RaycastStartPoint->GetComponentLocation();
	FVector SweepEnd = SweepStart + RaycastStartPoint->GetForwardVector() * DistanceToCheck;
	
	TArray<FHitResult> HitResults;
	bool bHit = GetWorld()->SweepMultiByChannel(HitResults,
		SweepStart,
		SweepEnd,
		FQuat::Identity,
		ECC_Visibility,
		Sphere );

	//UE_LOG(LogTemp, Warning, TEXT("Swept for Actor."))
	
	if (bHit)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Actor Type Found In Array"));
		for (FHitResult Hit : HitResults)
		{
			if (IsValid(Hit.GetActor()) && Hit.GetActor()->IsA(ActorToSweepFor))
			{
				UE_LOG(LogTemp, Warning, TEXT("Actor Found: %p"), Hit.GetActor()->GetClass());
				SweepHitActor = Hit.GetActor();
				//DrawDebugSphere(GetWorld(), SweepHitActor->GetActorLocation(), 100.f, 12, FColor::Red, false, 3.f);
				break;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Actor Found"));
	}
}

void ATimberEnemyCharacter::SetupDataClusterComponent()
{
	if (DataClusterWidgetComponent)
	{
		if (!DataClusterWidgetComponent->GetWidget()) return;

		//Initializing the Widget to Hidden. Will be showing with Player Input.
		if (UEnemyDataCluster* DCW = Cast<UEnemyDataCluster>(DataClusterWidgetComponent->GetWidget()))
		{
			DCW->EnemyOwner = this;
		}
		DataClusterWidgetComponent->GetWidget()->SetVisibility(ESlateVisibility::Hidden);

		ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(UGameplayStatics::GetActorOfClass(this, ATimberPlayableCharacter::StaticClass()));
		if (IsValid(PlayerCharacter))
		{
			ATimberPlayerController* PC = Cast<ATimberPlayerController>(PlayerCharacter->GetController());
			if (IsValid(PC))		
			{
				PC->ToggleDataView_DelegateHandle.AddDynamic(this, &ATimberEnemyCharacter::HandleToggleDataView);
			}
		}

		
	}
	
	/*if (IsValid(StatusEffectBarWidgetComponent) && IsValid(StatusEffectBarWidgetComponent->GetWidget()))
	{
		StatusEffectBarWidgetComponent->GetWidget()->SetVisibility(ESlateVisibility::Hidden);
	}
	
	UStatusEffectBar* StatusEffectBarWidget = Cast<UStatusEffectBar>(StatusEffectBarWidgetComponent->GetWidget());
	if (IsValid(StatusEffectBarWidget))
	{
		ATimberPlayableCharacter* PlayerCharacter = Cast<ATimberPlayableCharacter>(UGameplayStatics::GetActorOfClass(this, ATimberPlayableCharacter::StaticClass()));
		if (IsValid(PlayerCharacter))
		{
			ATimberPlayerController* PC = Cast<ATimberPlayerController>(PlayerCharacter->GetController());
			if (IsValid(PC))		
			{
				PC->ToggleDataView_DelegateHandle.AddDynamic(this, &ATimberEnemyCharacter::HandleToggleDataView);
			}
		}
	}*/
}