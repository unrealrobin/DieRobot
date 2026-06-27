// Property of Paracosm Industries. 


#include "Character/DieRobotPlayableCharacter.h"
#include "Character/DieRobotSeeda.h"
#include "Controller/DieRobotPlayerController.h"
#include "BuildSystem/Traps/TrapBase.h"
#include "BuildSystem/BuildingComponents/DieRobotBuildingComponentBase.h"
#include "Components/BuildSystem/BuildSystemManagerComponent.h"
#include "Components/Inventory/InventoryManagerComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/DieRobotAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/Combat/CombatComponent.h"
#include "Components/MissionDelivery/MissionDeliveryComponent.h"
#include "Components/Vignette/PlayerVignetteComponent.h"
#include "Containers/Ticker.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameModes/DieRobotGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/Wave/WaveGameInstanceSubsystem.h"
#include "UI/DieRobotHUDBase.h"

ADieRobotPlayableCharacter::ADieRobotPlayableCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	Camera->SetupAttachment(CameraSpringArm);
	CameraSpringArm->SetupAttachment(RootComponent);

	/* Actor Components */
	BuildSystemManager = CreateDefaultSubobject<UBuildSystemManagerComponent>("BuildSystemManager");
	InventoryManager = CreateDefaultSubobject<UInventoryManagerComponent>("InventoryManager");
	CombatComponent = CreateDefaultSubobject<UCombatComponent>("CombatComponent");
	VignetteComponent = CreateDefaultSubobject<UPlayerVignetteComponent>("VignetteComponent");
	MissionDeliveryComponent = CreateDefaultSubobject<UMissionDeliveryComponent>("MissionDeliveryComponent");

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("DR_PlayerCharacterCapsule"));
	GetMesh()->SetCollisionProfileName(TEXT("DR_AestheticMeshOnly"));
}

void ADieRobotPlayableCharacter::SetAmplificationPower(float DeltaPower)
{
	AmplificationPower = FMath::Clamp(AmplificationPower + DeltaPower, 0.f, 100.f);
}

void ADieRobotPlayableCharacter::HandleWaveEnd(int CompletedWaveNumber)
{
	if (!bIsPlayerDead)
	{
		PlayerGainHealth(MaxHealth);
	}
}

void ADieRobotPlayableCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	/* Set Character Movement Defaults*/
	GetCharacterMovement()->MaxWalkSpeed = 800.f;

	PlayerController = Cast<ADieRobotPlayerController>(GetController());
	
	/* Load Inventory */
	GetPlayerInventoryFromPlayerState();

	/*Initialize Anim Instance*/
	UDieRobotAnimInstance* Anim = Cast<UDieRobotAnimInstance>(GetMesh()->GetAnimInstance());
	if (Anim)
	{
		Anim->OwningPawn = this;
		Anim->PlayerController = Cast<ADieRobotPlayerController>(GetController());
	}
	
	/*Delegate Binding*/
	if (ADieRobotHUDBase* HUD = Cast<ADieRobotHUDBase>(Cast<ADieRobotPlayerController>(GetController())->GetHUD()))
	{
		HUD->bIsBuildMenuOpen.AddDynamic(this,&ADieRobotPlayableCharacter::HandleBuildMenuOpen);
	}
	
	//Binding to the GameMode Delegate to know when the Seeda is Initialized and then Binding to the Seeda Delegates
	ADieRobotGameModeBase* GM = Cast<ADieRobotGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Binding to GameMode Delegate to know when Seeda is Initialized."));
		//Basically says now you can Bind to the Seeda Delegates, and Passes in the Seeda Actor Ref.
		//This is possible because we know for sure that the GM instance is available for binding by the time this BeginPlay fires.
		GM->OnSeedaSpawn.AddDynamic(this, &ADieRobotPlayableCharacter::BindToSeedaDelegates);

		/*
	 *Broadcasts a Delegate on the GameMode letting other Systems know that the Character is Initialized.
	 *Allows other systems to bind to the Game Mode Delegate which is typically initialized before anything else.
	 *Used to eliminate potential Initialization Races
	 *Set at the bottoms of the Begin Play Function to ensure all other systems are initialized.
	 */
		GM->PlayerIsInitialized(this);
	}
	
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		CombatComponent->SpawnMeleeWeapon();
		CombatComponent->SpawnRangedWeapon();
	});
	
	/* Tutorial */
	if (GetTutorialState() == ETutorialState::Wake1)
	{
		PlayWakeAnimationMontage();
	}

	/* Vignette */
	if (VignetteComponent)
	{
		VignetteComponent->HandleHealthChange(CurrentHealth/MaxHealth);
	}
	
	/* Wave Completion Binding */
	UWaveGameInstanceSubsystem* W = GetWorld()->GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>();
	if (W)
	{
		W->OnWaveComplete.AddDynamic(this, &ADieRobotPlayableCharacter::HandleWaveEnd);
	}
}

void ADieRobotPlayableCharacter::BindToSeedaDelegates(AActor* Seeda)
{
	ADieRobotSeeda* SeedaRef = Cast<ADieRobotSeeda>(Seeda);
	if (SeedaRef)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Seeda Initialized, Binding to Seeda Delegates."));

		//When seeda dies, player also dies.
		SeedaRef->OnSeedaDeath.AddDynamic(this, &ADieRobotPlayableCharacter::HandlePlayerDeath);
		
		//When Seeda is being Destroyed
		SeedaRef->OnDestroyed.AddDynamic(this, &ADieRobotPlayableCharacter::UnbindFromSeedaDeathDelegate);
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Seeda Not Initialized, Cannot Bind to Seeda Delegates."));
	}
}

void ADieRobotPlayableCharacter::UnbindFromSeedaDeathDelegate(AActor* DestroyedActor)
{
	//If Seeda Exists
	if (Cast<ADieRobotSeeda>(UGameplayStatics::GetActorOfClass(this, ADieRobotSeeda::StaticClass())))
	{
		//Unbind from Seeda Death Delegate - Will need to Rebind on Seeda Respawn, this gets Called when Seeda Dies.
		Cast<ADieRobotSeeda>(UGameplayStatics::GetActorOfClass(this, ADieRobotSeeda::StaticClass()))->OnSeedaDeath.RemoveDynamic(
			this,
			&ADieRobotPlayableCharacter::HandlePlayerDeath);
	}
}

void ADieRobotPlayableCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//Initiated From Player Controller Input
	if (CharacterState == ECharacterState::Building && !bIsTheBuildMenuOpen)
	{
		PerformBuildSystemRaycast();
	}
	
	if (CharacterState != ECharacterState::Amplified)
	{
		if (NotAmplifiedTime > 5.0f)
		{
			SetAmplificationPower(5.0f);
			NotAmplifiedTime = 0.0f;
		}
		else
		{
			NotAmplifiedTime += DeltaSeconds;
		}
	}
	else //Is Amplified
	{
		AmplificationPower -= 10 * DeltaSeconds;
		if (AmplificationPower <= 0.0f)
		{
			SetIsAmplified(false);
		}
	}
	
	
}

/*Build System Stuff*/

void ADieRobotPlayableCharacter::PerformBuildSystemRaycast()
{
	if (PlayerController)
	{
		FVector RaycastStart;
		FRotator PlayerRotation;
		Controller->GetPlayerViewPoint(RaycastStart, PlayerRotation);

		//1000 is the range to perform the Raycast.
		FVector RaycastEnd = RaycastStart + (PlayerRotation.Vector() * BuildRaycastDistance);

		/* Ignore the Player Raycasting and the Weapon*/
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);
		CollisionParams.AddIgnoredActor(this->CurrentlyEquippedWeapon);

		/*Multiple Hits*/
		bool bHits = GetWorld()->LineTraceMultiByChannel(
			HitResults,
			RaycastStart,
			RaycastEnd,
			ECC_Visibility,
			CollisionParams);

		//DrawDebugSphere(GetWorld(), RaycastEnd, 40.f, 8, FColor::Red, false, 0.1f);
		
		HandleShowDeleteWidget();
		
		HandleRaycastHitConditions(bHits);
	}
}

void ADieRobotPlayableCharacter::HandleRaycastHitConditions(bool bHits)
{
	//Active Buildable class set from UI Click in the Build Panel Menu
	if (BuildSystemManager)
	{
		TSubclassOf<ABuildableBase> ActiveBuildableClass = BuildSystemManager->GetActiveBuildableClass();
		
		if (bHits && ActiveBuildableClass)
		{
			//DrawDebugSphere(GetWorld(), HitResults[0].ImpactPoint, 10.f, 8, FColor::Red, false, 0.1f);
			BuildSystemManager->HandleProxyPlacement(HitResults, ActiveBuildableClass);
		}
		else //If the Raycast Hit Nothing
		{
			BuildSystemManager->ResetBuildableComponents();
		}
	}
}

bool ADieRobotPlayableCharacter::HandleShowDeleteWidget()
{
	//Hit Results Passed in as a Global Variable
	//First Hovered Building Component
	HoveredBuildingComponent = nullptr;
	for(FHitResult Hits : HitResults)
	{
		if(Cast<ABuildableBase>(Hits.GetActor()))
		{
			HoveredBuildingComponent = Cast<ABuildableBase>(Hits.GetActor());
			/*UE_LOG(LogTemp, Warning, TEXT("DieRobotPlayableCharacter.cpp - Hovered Building Component Actor: %s"), *HoveredBuildingComponent->GetName());
			UE_LOG(LogTemp, Warning, TEXT("DieRobotPlayableCharacter.cpp - Hovered Building Component: %s"), *Hits.GetComponent()->GetName());*/
			FVector2d ScreenLocationOfImpactPoint;

			/*Setting Impact Point at Location to show Delete Widget in Screen Space*/
			GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen(
				Hits.ImpactPoint,
				ScreenLocationOfImpactPoint);

			// Broadcast a Delegate with the Impact Position to the HUD.
			HandleSpawnDeleteIconLocation_DelegateHandle.Broadcast(ScreenLocationOfImpactPoint.X, ScreenLocationOfImpactPoint.Y);
		}
		break;
	}

	//TODO:: Change this to check that if there are no Hits, to Reset the Delete Icon.
	if(HoveredBuildingComponent == nullptr)
	{
		ResetDeleteIcon();
		return false;
	}
	return false;
}

void ADieRobotPlayableCharacter::HandleBuildMenuOpen(bool bIsBuildMenuOpen)
{
	/*
	 * We are looking for a state where the character is in Buildmode and the Build Menu is Closed.
	 * This lets us know that the build menu is closed.
	 */
	bIsTheBuildMenuOpen = bIsBuildMenuOpen; 
}

/*Death & Damage*/
void ADieRobotPlayableCharacter::HandlePlayerDeath(bool bIsPlayerDeadNow)
{
	if (bIsPlayerDeadNow)
	{
		if (IsValid(PlayerController))
		{
			PlayerController->FlushPressedKeys();		
		}
		//Used for animation bp main state.
		bIsPlayerDead = true;
		
		PlayDeathAnimation();
		//Broadcast to HUD to Update Death UI Reason Text
		//Player Controller && HUD is Subscribed to this Delegate
		HandlePlayerDeath_DelegateHandle.Broadcast(bIsPlayerDeadNow);
	}
}

void ADieRobotPlayableCharacter::PlayDeathAnimation()
{
	PlayAnimMontage(DeathMontage, 1.f, FName("1"));
}


void ADieRobotPlayableCharacter::GetPlayerInventoryFromPlayerState()
{
	if (IsValid(PlayerController))
	{
		if (PlayerController->GetPlayerState<APlayerStateBase>()->MainInventory)
		{
			InventoryObject = GetController()->GetPlayerState<APlayerStateBase>()->MainInventory;
		}
	}

	if (!IsValid(InventoryObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player Character - Inventory Object Not Found."));
	}
}

void ADieRobotPlayableCharacter::ResetDeleteIcon()
{
	HoveredBuildingComponent = nullptr;
	HandleRemoveDeleteIcon_DelegateHandle.Broadcast();
}

void ADieRobotPlayableCharacter::PlayerTakeDamage(float DamageAmount)
{
	CurrentHealth -= DamageAmount;

	if (VignetteComponent)
	{
		VignetteComponent->HandleHealthChange(CurrentHealth/MaxHealth);
	}

	
	if (CurrentHealth <= 0.f)
	{
		bIsPlayerDead = true;
		HandlePlayerDeath(bIsPlayerDead);
	}
	else
	{
		AddOverlayMaterialToCharacter(HitMarkerOverlayMaterial, 0.3f );
	}
}

void ADieRobotPlayableCharacter::PlayerGainHealth(float HealthAmount)
{
	if (bIsPlayerDead) return;
	if (MaxHealth == CurrentHealth) return;

	if (CurrentHealth + HealthAmount >= MaxHealth)
	{
		CurrentHealth = MaxHealth;
	}
	else
	{
		CurrentHealth += HealthAmount;
	}

	if (VignetteComponent)
	{
		VignetteComponent->HandleHealthChange(CurrentHealth/MaxHealth);
		//UE_LOG(LogTemp, Warning, TEXT("Player Gain Health: %f"), CurrentHealth);
	}
	
}

/*Weapon Stuff*/

ETutorialState ADieRobotPlayableCharacter::GetTutorialState()
{
	ADieRobotGameStateBase* DieRobotGameState = Cast<ADieRobotGameStateBase>(GetWorld()->GetGameState());
	if (DieRobotGameState)
	{
		return DieRobotGameState->TutorialState;
	}

	return ETutorialState::Default;
	
}

void ADieRobotPlayableCharacter::StopAllAnimMontages()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance))
	{
		AnimInstance->StopAllMontages(0.25f);
	}
}

void ADieRobotPlayableCharacter::PlayWakeAnimationMontage()
{
	if (TutorialWakeMontage)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Player Character - Playing Wake Animation Montage."));
		
		PlayAnimMontage(TutorialWakeMontage, 1.f, FName("WakingUp"));
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Player Character - Wake Animation Montage Not Found."));
	}
}

void ADieRobotPlayableCharacter::PlayAnimationMontageAtSection(UAnimMontage* MontageToPlay, FName SectionName)
{
	PlayAnimMontage(MontageToPlay, 1.f, SectionName);
}

void ADieRobotPlayableCharacter::StartLerpRotation(const FRotator& TargetRotation, float DurationOfRotation)
{

	if (DurationOfRotation <= 0)
	{
		//UE_LOG(LogTemp, Error, TEXT("Can Not set a Lerp Rotation Duration of 0."));
		return;
	}

	//Dont want to restart the timer if we are already in the rotation animation.
	if (IsRotating)
	{
		return;
	}
	
	FRotator StartRotation = GetActorRotation().Clamp();
	if (StartRotation == FRotator::ZeroRotator)
	{
		return;
	}

	IsRotating = true;
	
	FTickerDelegate TickDelegate = FTickerDelegate::CreateLambda(
	   [this, StartRotation, TargetRotation, DurationOfRotation](float DeltaTime) -> bool
	   {
		   ElapsedTime += DeltaTime;
		   float Alpha = FMath::Clamp(ElapsedTime / DurationOfRotation, 0.0f, 1.0f);
		   float SmoothedAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

		   FRotator NewRotation = FMath::Lerp(StartRotation, TargetRotation, SmoothedAlpha);
		   SetActorRotation(NewRotation, ETeleportType::TeleportPhysics);

	   		//Early brake if movement started or of animation is finished.
		   if (Alpha >= 1.0f || GetVelocity().Length() > 0.0f)
		   {
			   IsRotating = false;
			   ElapsedTime = 0.0f;
			   return false; // Stop the ticker
		   }
		   return true; // Continue ticking
	   });

	FTSTicker::GetCoreTicker().AddTicker(TickDelegate);

	if (TurnInPlaceMontage)
	{
		StopAnimMontage(TurnInPlaceMontage);
	}
	
}

//CombatComponentAnimUser Interface Override
void ADieRobotPlayableCharacter::PlayWeaponEquipAnimationMontage(FName SectionName)
{
	//This is this characters-specific implementation of this Override.
	PlayEquipWeaponMontage(SectionName);
}

void ADieRobotPlayableCharacter::SetIsAmplified(bool bIsAmplified)
{
	if (bIsAmplified)
	{
		CharacterState = ECharacterState::Amplified;
		CombatComponent->UnEquipAllWeapons();
		if (UCharacterMovementComponent* CMC = GetCharacterMovement())
		{
			CMC->MaxWalkSpeed = 0.0f;
		}
		CreateAmplificationSphere();
	}
	else
	{
		CharacterState = ECharacterState::Standard;

		//Reverts characters walk speed back to base walk speed
		if (UCharacterMovementComponent* CMC = GetCharacterMovement())
		{
			CMC->MaxWalkSpeed = 600.0f;
		}
		
		if (TempAmplifyCapsule)
		{
			TempAmplifyCapsule->DestroyComponent();
			TempAmplifyCapsule = nullptr;
		}
		if (TempAmplifyStaticMeshComponent)
		{
			TempAmplifyStaticMeshComponent->DestroyComponent();
			TempAmplifyStaticMeshComponent = nullptr;
		}
	}
	
}

void ADieRobotPlayableCharacter::PlayEquipWeaponMontage(FName SectionName)
{
	//Build in function on the ACharacter Class.
	PlayAnimMontage(EquipWeaponMontage, EquipWeaponPlayRate, SectionName);
}

/* Amplification */
void ADieRobotPlayableCharacter::CreateAmplificationSphere()
{
	//Create the Sphere used for Overlap
	TempAmplifyCapsule = NewObject<UCapsuleComponent>(this);
	TempAmplifyCapsule->SetCapsuleSize(1000.f, 1000.f);
	TempAmplifyCapsule->RegisterComponent();
	TempAmplifyCapsule->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	TempAmplifyCapsule->OnComponentBeginOverlap.AddDynamic(this, &ADieRobotPlayableCharacter::HandleAmplificationCapsuleOverlap);
	TempAmplifyCapsule->OnComponentEndOverlap.AddDynamic(this, &ADieRobotPlayableCharacter::HandleAmplificationCapsuleEndOverlap);
	TempAmplifyCapsule->SetCollisionProfileName("DR_QueryBuildables");

	TempAmplifyStaticMeshComponent = NewObject<UStaticMeshComponent>(this);
	TempAmplifyStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TempAmplifyStaticMeshComponent->SetRelativeScale3D(FVector(1000/100.f, 1000/100.f, 1000/100.f));
	TempAmplifyStaticMeshComponent->SetStaticMesh(AmplifyMesh);
	TempAmplifyStaticMeshComponent->RegisterComponent();
	TempAmplifyStaticMeshComponent->AttachToComponent(TempAmplifyCapsule, FAttachmentTransformRules::KeepRelativeTransform);

	
}

bool ADieRobotPlayableCharacter::bEnoughPowerToAmplify()
{
	if (AmplificationPower >= 5) return true;
	
	return false;
}

void ADieRobotPlayableCharacter::UsePowerForAmplification()
{
}

void ADieRobotPlayableCharacter::GainPowerForAmplification()
{
}

void ADieRobotPlayableCharacter::HandleAmplificationCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                                                    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	IAmplifiable* AmplifiableActor = Cast<IAmplifiable>(OtherActor);
	if (AmplifiableActor && AmplifiableActor != this)
	{
		AmplifiableActor->SetIsAmplified(false);
		//UE_LOG(LogTemp, Warning, TEXT("Ended Overlap on Amplifiable Actor: %s"), *OtherActor->GetName());
	}
}

void ADieRobotPlayableCharacter::HandleAmplificationCapsuleOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	IAmplifiable* AmplifiableActor = Cast<IAmplifiable>(OtherActor);
	if (AmplifiableActor && AmplifiableActor != this)
	{
		UE_LOG(LogTemp, Warning, TEXT("Overlapped Amplifiable Actor: %s"), *OtherActor->GetName())
		AmplifiableActor->SetIsAmplified(true);
	}
}





