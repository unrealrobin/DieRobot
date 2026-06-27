// Property of Paracosm Industries.


#include "Controller/DieRobotPlayerController.h"
#include "Interfaces/Interactable.h"
#include "EnhancedInputSubsystems.h"
#include "BuildSystem/Constructs/TeleportConstruct.h"
#include "Character/DieRobotAnimInstance.h"
#include "UI/BuildingComponent.h"
#include "Character/DieRobotPlayableCharacter.h"
#include "Components/BuildSystem/BuildSystemManagerComponent.h"
#include "Components/Combat/CombatComponent.h"
#include "Data/DataAssets/BuildComponentDataAsset.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameModes/DieRobotGameModeBase.h"
#include "UI/DieRobotHUDBase.h"
#include "Weapons/Abilities/WeaponAbilityBase.h"
#include "MVVMGameSubsystem.h"           
#include "Subsystems/Wave/WaveGameInstanceSubsystem.h"
#include "ViewModels/MissionViewModel.h"


class UDialogueManager;

void ADieRobotPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PrepareInputSettings();

	InitializeCharacterAndCamera();
	
	InitializeTutorialStateBinding();

	MissionViewModelInstantiation();

	DieRobotCharacter->HandlePlayerDeath_DelegateHandle.AddDynamic(this, &ADieRobotPlayerController::HandlePlayerDeath);
	
	//Calls the Widget Creation on the HUD, so there are no timing issues between the Begin Play functions of the Controller and the HUD.
	//If no welcome message, remove this.
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ADieRobotPlayerController::ShowWelcomeMessage);
}

void ADieRobotPlayerController::PrepareInputSettings()
{
	Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	//When switching from Game Start Menu to Level, we switch back the Input Mode to Game and UI
	FInputModeGameAndUI GameAndUIInputMode;
	FInputModeGameOnly GameOnlyInputMode;
	SetInputMode(GameOnlyInputMode);

	ADieRobotGameModeBase* GameMode = Cast<ADieRobotGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->EnableStandardInputMappingContext.BindUFunction(this, FName("EnableStandardKeyboardInput"));
	}
	
	EnableStandardKeyboardInput();

	//Initial Broadcast should be ok since it's called from the GameMode on Begin play. This is just a redundant safety measure.
	if (GetTutorialState() == ETutorialState::Wake1)
	{
		DisableAllKeyboardInput();
	}
}

void ADieRobotPlayerController::InitializeCharacterAndCamera()
{
	DieRobotCharacter = Cast<ADieRobotPlayableCharacter>(GetPawn());
	DieRobotCharacterSpringArmComponent = DieRobotCharacter->CameraSpringArm;
	DieRobotCharacterMovementComponent = DieRobotCharacter->GetCharacterMovement();
}

void ADieRobotPlayerController::HideWelcomeWidget()
{
	//Shows Welcome Widget on Controller Start up
	ADieRobotHUDBase* HUD = Cast<ADieRobotHUDBase>(GetHUD());
	if (HUD && HUD->WelcomeWidget)
	{
		DisableCursor();
		HUD->WelcomeWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ADieRobotPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);

	/*Standard Inputs*/
	EnhancedInputComponent->BindAction(StandardAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::UsePrimaryAbility);
	EnhancedInputComponent->BindAction(SecondaryAction, ETriggerEvent::Started, this, &ADieRobotPlayerController::UseSecondaryAbilityStarted);
	EnhancedInputComponent->BindAction(SecondaryAction, ETriggerEvent::Canceled, this, &ADieRobotPlayerController::UseSecondaryAbilityCanceled);
	EnhancedInputComponent->BindAction(SecondaryAction, ETriggerEvent::Completed, this, &ADieRobotPlayerController::UseSecondaryAbilityCompleted);
	EnhancedInputComponent->BindAction(SecondaryAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::UseSecondaryAbilityTriggered);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::Move);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ADieRobotPlayerController::MoveComplete);
	EnhancedInputComponent->BindAction(LookUpAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::LookUp);
	EnhancedInputComponent->BindAction(LookRightAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::LookRight);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::CharacterJump);
	EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::Interact);
	EnhancedInputComponent->BindAction(EquipMeleeWeaponAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::EquipMeleeWeapon);
	EnhancedInputComponent->BindAction(EquipRangedWeaponAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::EquipRangedWeapon);
	EnhancedInputComponent->BindAction(ModifyCursorAction_Controller, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::ModifyCursorWithController);
	EnhancedInputComponent->BindAction(ReloadWeaponInputAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::ReloadWeapon);
	EnhancedInputComponent->BindAction(ToggleSettingsPanelAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::ToggleSettingsPanel);
	EnhancedInputComponent->BindAction(ToggleDataViewAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::ToggleDataView);
	EnhancedInputComponent->BindAction(StartWaveEarlyAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::HandleStartWaveEarly);
	
	EnhancedInputComponent->BindAction(CharacterAmplifyAction, ETriggerEvent::Started, this, &ADieRobotPlayerController::ActivateAmplify);
	EnhancedInputComponent->BindAction(CharacterAmplifyAction, ETriggerEvent::Completed, this, &ADieRobotPlayerController::DeactivateAmplify);
	

	/*Build Inputs*/
	EnhancedInputComponent->BindAction(ToggleBuildModeAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::EnterBuildMode);
	EnhancedInputComponent->BindAction(RotateBuildingComponentAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::RotateBuildingComponent);
	EnhancedInputComponent->BindAction(PlaceBuildingComponentAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::PlaceBuildingComponent);
	EnhancedInputComponent->BindAction(DeleteBuildingComponentAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::DeleteBuildingComponent);
	EnhancedInputComponent->BindAction(SelectIconAction_Controller, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::SelectBCIcon_Controller);
	EnhancedInputComponent->BindAction(ToggleBuildMenuStatusEffectWindowAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::ToggleBuildMenuStatusEffectWindow);
	EnhancedInputComponent->BindAction(ExitBuildModeAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::ExitBuildMode);

	#if !UE_BUILD_SHIPPING
	EnhancedInputComponent->BindAction(HighResShotWithUI_InputAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::TakeHighResShotWithUI);
	EnhancedInputComponent->BindAction(HighResShot_InputAction, ETriggerEvent::Triggered, this, &ADieRobotPlayerController::TakeHighResShot);
	#endif
	
}

void ADieRobotPlayerController::EnableCursor()
{
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		//I had to move this here for some timing issue with the welcome message not  removing the cursor from the screen when calling this.
		// Check if it causes any issues anywhere else.
		bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		SetInputMode(InputMode);
		
		if (DieRobotCharacter && DieRobotCharacter->CameraSpringArm)
		{
			DieRobotCharacter->CameraSpringArm->bUsePawnControlRotation = false;
		}
	});
	
	//TODO:: What is this for? 
	FRotator SavedControllerRotation = GetControlRotation();
	SetControlRotation(SavedControllerRotation);
}

void ADieRobotPlayerController::DisableCursor()
{
	bShowMouseCursor = false;
	
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	
	if (DieRobotCharacter && DieRobotCharacter->CameraSpringArm)
	{
		DieRobotCharacter->CameraSpringArm->bUsePawnControlRotation = true;
	}
	
}

void ADieRobotPlayerController::SetInteractableItem(IInteractable* Item)
{
	InteractableItem = Item;
}

void ADieRobotPlayerController::ClearInteractableItem()
{
	InteractableItem = nullptr;
}

void ADieRobotPlayerController::TakeHighResShotWithUI()
{

	/*FScreenshotRequest::OnScreenshotRequestProcessed().AddLambda([](const FString& Path)
	{
		UE_LOG(LogTemp, Log, TEXT("High-res screenshot WITH UI saved to: %s"), *Path);
	});*/
	FScreenshotRequest::RequestScreenshot(true);
	//GetWorld()->GetGameViewport()->Exec(GetWorld(), TEXT("HighResShot 1920x1080 SHOWUI"), *GLog);
}

void ADieRobotPlayerController::TakeHighResShot()
{

	/*FScreenshotRequest::OnScreenshotRequestProcessed().AddLambda([](const FString& Path)
	{
		UE_LOG(LogTemp, Log, TEXT("High-res screenshot WITHOUT UI saved to: %s"), *Path);
	});*/

	GetWorld()->GetGameViewport()->Exec(GetWorld(), TEXT("HighResShot 1920x1080"), *GLog);
}

void ADieRobotPlayerController::Move(const FInputActionValue& Value)
{
	MoveInputActionValue = Value;

	APawn* ControlledPawn = GetPawn();

	if (ControlledPawn)
	{
		//Controller Rotation and Forward Vector
		const FRotator ControllerRotation = GetControlRotation();
		const FVector ControllerForwardVector = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::X);

		//Current Directions of the Character
		CharacterForwardMoveDirection = ControlledPawn->GetActorForwardVector();
		CharacterRightMoveDirection = ControlledPawn->GetActorRightVector();

		// Rotates the Character to the direction of the Camera Smoothly
		const FRotator TargetForwardDirection = FRotator(0.f, ControllerRotation.Yaw, 0.f);
		FRotator NewRotation = FMath::RInterpTo(ControlledPawn->GetActorRotation(), TargetForwardDirection, GetWorld()->GetDeltaSeconds(), 5.f);
		ControlledPawn->SetActorRotation(NewRotation);

		//Adding Movement Input to the Character
		ControlledPawn->AddMovementInput(ControllerForwardVector, Value.Get<FVector2D>().X, false);
		ControlledPawn->AddMovementInput(CharacterRightMoveDirection, Value.Get<FVector2D>().Y, false);
	}
}

void ADieRobotPlayerController::MoveComplete(const FInputActionValue& Value)
{
	//This just resets the MoveInputActionValue to 0,0 after the last input is released. Otherwise the character will keep moving in the last direction.
	MoveInputActionValue = FVector2d(0.f, 0.f);
}

void ADieRobotPlayerController::LookUp(const FInputActionValue& Value)
{
	const float CurrentPitch = DieRobotCharacter->GetControlRotation().Pitch;
	const float ClampedPitch = FMath::Clamp(CurrentPitch + Value.Get<float>(), ViewPitchMin, ViewPitchMax);

	FRotator UpdatedRotation = DieRobotCharacter->GetControlRotation();
	UpdatedRotation.Pitch = ClampedPitch;

	SetControlRotation(UpdatedRotation);
	PitchAngle = UpdatedRotation.Pitch;
}

void ADieRobotPlayerController::HandleCharacterRotation()
{
	//We want to check the difference in rotation from the Characters Forward Rotation to the Controllers Rotation
	//If the difference is larger than X
	//Rotate the Character some y degrees, and play rotations animation
	
	/*Checking for adjusted Rotation*/
	if (DieRobotCharacter)
	{
		//Dont rotate if Character is moving.
		if (DieRobotCharacter->GetVelocity().Length() > 0)
		{
			return;
		}

		//Don't want the Rotation Montage to Play and Break the Equip Animation Montage.
		//Can be adjusted later by have 2 Rotation Animations and separate Upper and Lower Body Animations.
		if (DieRobotCharacter->CombatComponent && DieRobotCharacter->CombatComponent->bIsEquipMontagePlaying)
		{
			return;
		}
		
		FRotator CharacterRotation = DieRobotCharacter->GetActorRotation().Clamp();
		//UE_LOG(LogTemp, Warning, TEXT("Kip Yaw Rotation = %f"), CharacterRotation.Yaw)
		FRotator ControllerRotation = GetControlRotation().Clamp();
		//UE_LOG(LogTemp, Warning, TEXT("Kip Controller Rotation = %f"), ControllerRotation.Yaw)

		float DeltaYaw = FMath::FindDeltaAngleDegrees(CharacterRotation.Yaw, ControllerRotation.Yaw);
		//UE_LOG(LogTemp, Warning, TEXT("Delta Angle: %f"), DeltaYaw);

		

		//If the Controller Rotation Delta from Character Rotation is larger than 45 degrees...
		if (DeltaYaw < -45.0 || DeltaYaw > 45.0)
		{
			if (DeltaYaw < 0 )
			{
				DieRobotCharacter->PlayAnimationMontageAtSection(DieRobotCharacter->TurnInPlaceMontage, "TurnLeft");
			}
			else
			{
				DieRobotCharacter->PlayAnimationMontageAtSection(DieRobotCharacter->TurnInPlaceMontage, "TurnRight");
			}
			//Rotate Left
			CharacterRotation.Yaw += DeltaYaw;
			//Perform Rotation on the Character.
			DieRobotCharacter->StartLerpRotation(CharacterRotation, 0.25f);
		}
	}
}

void ADieRobotPlayerController::LookRight(const FInputActionValue& Value)
{
	FRotator UpdatedRotation = DieRobotCharacter->GetControlRotation();
	UpdatedRotation.Yaw = UpdatedRotation.Yaw + Value.Get<float>();
	SetControlRotation(UpdatedRotation);
	YawAngle = UpdatedRotation.Yaw;

	//Handles if needed character rotation adjustments.
	//HandleCharacterRotation();
}

void ADieRobotPlayerController::CharacterJump(const FInputActionValue& Value)
{
	//CanCharacterJump(); // sets the CanJump Variable
	//SwitchToWalking = false;
	//Character JumpZVelocity and Other Jump Details are set in the BP for the character.
	DieRobotCharacter->IsNowJumping = true;
	DieRobotCharacter->Jump();
	
}

void ADieRobotPlayerController::Interact(const FInputActionValue& Value)
{
	//TODO:: Need to implement some kind of check if overlapping multiple interactable items.

	if (Value.Get<bool>() && InteractableItem)
	{
		if (InteractableItem)
		{
			InteractableItem->Interact();
		}
	}
}

/*Melee*/
void ADieRobotPlayerController::EquipMeleeWeapon(const FInputActionValue& Value)
{
	if (DieRobotCharacter && DieRobotCharacter->CombatComponent && DieRobotCharacter->CombatComponent->GetCurrentWeaponState() != EOwnerWeaponState::MeleeWeaponEquipped)
	{
		if (DieRobotCharacter->CombatComponent->bIsEquipMontagePlaying)
		{
			return;
		}

		DieRobotCharacter->CombatComponent->UnEquipCurrentlyEquippedWeapon();

		//Equipping a Weapon takes the player out of Build Mode.
		HandleExitBuildMode();

		//Notify calls actual equip logic on CombatComponent->EquipMelee()
		//DieRobotCharacter->PlayEquipWeaponMontage("EquipSword");
		DieRobotCharacter->CombatComponent->PlayEquipWeaponMontage("EquipMelee");
	}
}

/*Rifle*/
void ADieRobotPlayerController::EquipRangedWeapon(const FInputActionValue& Value)
{
	//Input action Function Call for Equipping Weapon.
	//Remap to 1 Key.

	if (DieRobotCharacter && DieRobotCharacter->CombatComponent && DieRobotCharacter->CombatComponent->GetCurrentWeaponState() != EOwnerWeaponState::RangedWeaponEquipped)
	{
		if (DieRobotCharacter->CombatComponent->bIsEquipMontagePlaying)
		{
			return;
		}

		DieRobotCharacter->CombatComponent->UnEquipCurrentlyEquippedWeapon();

		HandleExitBuildMode();
		
		DieRobotCharacter->CombatComponent->PlayEquipWeaponMontage("EquipRanged");
	}
}

void ADieRobotPlayerController::DisableAllKeyboardInput()
{
	Subsystem->RemoveMappingContext(StandardInputMappingContext);
	Subsystem->RemoveMappingContext(BuildModeInputMappingContext);
}

void ADieRobotPlayerController::EnableStandardKeyboardInput()
{
	if (Subsystem)
	{
		Subsystem->AddMappingContext(StandardInputMappingContext, 1);
		DisableCursor();
	}
}

void ADieRobotPlayerController::UsePrimaryAbility(const FInputActionValue& Value)
{
	if (DieRobotCharacter && DieRobotCharacter->CombatComponent)
	{
		DieRobotCharacter->CombatComponent->HandlePrimaryAbility(Value);
	}
}

void ADieRobotPlayerController::UseSecondaryAbilityStarted(const FInputActionValue& Value)
{
	//If started is fired, that means we are holding the button down.
	//Checking if the ability its a Hold and Release Ability.
	EAbilityInputRequirement AbilityInputRequirement = DieRobotCharacter->CombatComponent->GetAbilityInputRequirement(false);
	if (AbilityInputRequirement == EAbilityInputRequirement::HoldOnly)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Secondary Ability Started."));
		DieRobotCharacter->CombatComponent->HandleSecondaryAbility_Started(Value);
	}
}

void ADieRobotPlayerController::UseSecondaryAbilityCanceled(const FInputActionValue& Value)
{
	//UE_LOG(LogTemp, Warning, TEXT("Secondary Ability Canceled."));
	EAbilityInputRequirement AbilityInputRequirement = DieRobotCharacter->CombatComponent->GetAbilityInputRequirement(false);
	if (AbilityInputRequirement == EAbilityInputRequirement::HoldOnly)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Secondary Ability Started."));
		DieRobotCharacter->CombatComponent->HandleSecondaryAbility_Cancelled(Value);
	}
}

void ADieRobotPlayerController::UseSecondaryAbilityCompleted(const FInputActionValue& Value)
{
	if (DieRobotCharacter->CombatComponent )
	{
		//UE_LOG(LogTemp, Warning, TEXT("Secondary Ability Completed."));
		EAbilityInputRequirement AbilityInputRequirement = DieRobotCharacter->CombatComponent->GetAbilityInputRequirement(false);
		if (AbilityInputRequirement == EAbilityInputRequirement::HoldOnly)
		{
			//Reached when a Press and Hold Ability Hit the Max Activation Threshold.
			DieRobotCharacter->CombatComponent->HandleSecondaryAbility_Completed(Value);
		}
	}
}

void ADieRobotPlayerController::UseSecondaryAbilityTriggered(const FInputActionValue& Value)
{
	if (DieRobotCharacter && DieRobotCharacter->CombatComponent)
	{
		EAbilityInputRequirement AbilityInputRequirement = DieRobotCharacter->CombatComponent->GetAbilityInputRequirement(false);
		if (AbilityInputRequirement == EAbilityInputRequirement::Pressed)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Secondary Ability Triggered."));
			//Reached when a Pressed Ability is Pressed.
			DieRobotCharacter->CombatComponent->HandleSecondaryAbility(Value);
		}
	}
}

/*Build System Controls*/

void ADieRobotPlayerController::EnterBuildMode(const FInputActionValue& Value)
{
	//Changing Character State to Build Mode.
	DieRobotCharacter->CharacterState = ECharacterState::Building;

	// Entering Build Mode
	if (DieRobotCharacter->CharacterState == ECharacterState::Building)
	{
		if (Subsystem)
		{
			//Adding IMC for BuildMode - OverWrites on the overwritten Standard IMC
			Subsystem->AddMappingContext(BuildModeInputMappingContext, 2);
		}
		DieRobotCharacter->CombatComponent->UnEquipAllWeapons();

		EnableCursor();

		//Deletes Lingering Proxies
		DieRobotCharacter->BuildSystemManager->RemoveBuildingComponentProxies_All();

		//HUD Responsibility
		OpenBuildModeSelectionMenu();
	}
}

void ADieRobotPlayerController::OpenBuildModeSelectionMenu()
{
	//Broadcast to the HUD to Open the Build Menu
	IsBuildPanelOpen.Broadcast(true);
}

void ADieRobotPlayerController::HandleExitBuildMode()
{
	//Exiting Build Mode
	if (DieRobotCharacter->CharacterState == ECharacterState::Building && DieRobotCharacter->BuildSystemManager)
	{
		DieRobotCharacter->BuildSystemManager->CleanUpBuildSystemManagerComponent();
		
		//Handles changes on the Controller when Leaving Build Mode.
		HandleControllerExitBuildMode();

		//Broadcast to HUD to Hide the Build Menu - Handles Cursor Changes.
		ShouldHideBuildMenu.Broadcast();

		FlushPressedKeys();
	}
}

void ADieRobotPlayerController::ExitBuildMode(const FInputActionValue& Value)
{
	/*This only works when the Build Mode Input Mapping Context is Active
	 * all keymappings here won't work when the Standard Input Mapping Context is Active.
	 */

	HandleExitBuildMode();
}

void ADieRobotPlayerController::ToggleSettingsPanel(const FInputActionValue& Value)
{
	if (DieRobotCharacter->CharacterState == ECharacterState::Standard)
	{
		//The HUD will know whether the visibility of the settings panel is set to true or false. And will toggle between them.
		ToggleSettingsPanel_DelegateHandle.Broadcast();
		//Just letting the HUD know that the player pressed this button while out of Build Mode.
	}
}

void ADieRobotPlayerController::ToggleBuildMenuStatusEffectWindow(const FInputActionValue& Value)
{
	
	ADieRobotHUDBase* HUD = Cast<ADieRobotHUDBase>(GetHUD());
	if (HUD)
	{
		HUD->ToggleBuildMenuStatusEffectDetails();
		//UE_LOG(LogTemp, Warning, TEXT("RMB Pressed in Build Mode"));
	}
	
}

void ADieRobotPlayerController::ToggleDataView(const FInputActionValue& Value)
{
	ToggleDataView_DelegateHandle.Broadcast(Value);
}

void ADieRobotPlayerController::HandleStartWaveEarly()
{
	//Adding cooldown to the actual Controller Trigger.
	if (bStartWaveEarlyIsOnCooldown) return;
	
	if (UWaveGameInstanceSubsystem* WS = GetWorld()->GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>())
	{
		if (WS->bIsWaveActive) return;
		
		WS->EarlyStartWave();
		bStartWaveEarlyIsOnCooldown = true;

		//After five seconds, allow the control to be triggered again.
		GetWorld()->GetTimerManager().SetTimer(OnStartWaveEarlyCooldownTimerHandle, FTimerDelegate::CreateLambda([this]() {
			bStartWaveEarlyIsOnCooldown = false;
		}), 5.0f, false);
	}
}

void ADieRobotPlayerController::HandleControllerExitBuildMode()
{
	DieRobotCharacter->CharacterState = ECharacterState::Standard;

	DisableCursor();

	if (Subsystem)
	{
		// Removing the Buttons used for Build Mode.
		Subsystem->RemoveMappingContext(BuildModeInputMappingContext);
	}

	DieRobotCharacter->ResetDeleteIcon();

	//Deletes Lingering Proxies
	DieRobotCharacter->BuildSystemManager->RemoveBuildingComponentProxies_All();
}

void ADieRobotPlayerController::RotateBuildingComponent(const FInputActionValue& Value)
{
	if (DieRobotCharacter->CharacterState == ECharacterState::Building && DieRobotCharacter->BuildSystemManager)
	{
		DieRobotCharacter->BuildSystemManager->RotateBuildingComponent();
	}
}

void ADieRobotPlayerController::PlaceBuildingComponent(const FInputActionValue& Value)
{
	UBuildSystemManagerComponent* BuildSystemManager = DieRobotCharacter->BuildSystemManager;
	if (BuildSystemManager)
	{
		BuildSystemManager->SpawnFinalBuildable();
	}
}

void ADieRobotPlayerController::DeleteBuildingComponent(const FInputActionValue& Value)
{
	//TODO:: Add Progress like system where Pressing the E button will Delete if Held for 1 Full second. Show Progress Swirl.

	if (DieRobotCharacter && DieRobotCharacter->HoveredBuildingComponent)
	{
		if (DieRobotCharacter->HoveredBuildingComponent->BuildableType == EBuildableType::Environment)
		{
			UE_LOG(LogTemp, Warning, TEXT("DieRobot Player Controller - DeleteBuildingComponent() - Cannot Delete Environment"));
			return;
		}

		if (DieRobotCharacter->CharacterState == ECharacterState::Building && DieRobotCharacter->HoveredBuildingComponent)
		{
			//When the Buildable is Deleted by the Player, it will drop the cost of the buildable.
			//UE_LOG(LogTemp, Warning, TEXT("Deleting Hovered BuildingComponent: %s"), *DieRobotCharacter->HoveredBuildingComponent->GetName());
			
			DieRobotCharacter->HoveredBuildingComponent->HandleDeletionOfBuildable();

			//Need to Spawn 2 Loot Amounts.
			if (DieRobotCharacter->HoveredBuildingComponent->IsA(ATeleportConstruct::StaticClass()))
			{
				DieRobotCharacter->HoveredBuildingComponent->HandleDeletionOfBuildable();
			}

			//Reset the value of HoveredBuildingComponent after deletion.
			DieRobotCharacter->HoveredBuildingComponent = nullptr;

			//Broadcasting to Game Mode to update the Path Tracer.
			DieRobotCharacter->BuildSystemManager->RedrawPathTraceHandle.Broadcast();

		}
	}
}

void ADieRobotPlayerController::HandlePlayerDeath(bool bIsPlayerDead)
{
	if (bIsPlayerDead)
	{
		EnableCursor();
		
		DisableAllKeyboardInput();

		DieRobotCharacter->CombatComponent->UnEquipAllWeapons();

		//Subscribed on the HUD to Show the Death UI
		HandleDeathUI_DelegateHandle.Execute();
	}
}

void ADieRobotPlayerController::ReloadWeapon(const FInputActionValue& Value)
{
	if (DieRobotCharacter && DieRobotCharacter->CombatComponent)
	{
		DieRobotCharacter->CombatComponent->ReloadRangedWeapon();
	}
}

void ADieRobotPlayerController::ActivateAmplify(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Amplification Start."));
	if (Value.Get<bool>() && DieRobotCharacter)
	{
		DieRobotCharacter->SetIsAmplified(true);
	}
	
}

void ADieRobotPlayerController::DeactivateAmplify(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Amplification End."))

	if (DieRobotCharacter)
	{
		DieRobotCharacter->SetIsAmplified(false);
	}
}

/* Controller Only */
void ADieRobotPlayerController::ModifyCursorWithController(const FInputActionValue& Value)
{
	// Get the current mouse position
	FVector2D CurrentMousePos;
	GetMousePosition(CurrentMousePos.X, CurrentMousePos.Y);

	// Get the joystick input value
	FVector2D AnalogValue = Value.Get<FVector2D>();

	// Delta time for frame-independent movement
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// Define cursor speed (you can make this a class variable for tuning)
	float CursorSpeed = 800.0f;

	// Calculate the new mouse position
	FVector2D NewMousePos = CurrentMousePos + (AnalogValue * CursorSpeed * DeltaTime);
	//DrawDebugPoint(GetWorld(), FVector(NewMousePos, 0), 5, FColor::Red, false, 1.0f);

	// Update the mouse location
	SetMouseLocation(NewMousePos.X, NewMousePos.Y);
}

void ADieRobotPlayerController::SelectBCIcon_Controller(const FInputActionValue& Value)
{
	/*
	 * Hover focuses the widget.
	 * Store the Widget in a variable on the controller.
	 * Retrieve the Data Asset Stored on the Widget.
	 * Get the BP Class Name Stored on the Data Asset.
	 * Set that to the ActiveBuildableComponentClass.
	 * Close the Build Menu Panel. HUD::ShouldHideBuildMenu 
	 */
	if (HoveredIconDataAsset)
	{
		TSubclassOf<ABuildableBase> BuildingComponentClassName = HoveredIconDataAsset->BuildingComponentClass;
		//UE_LOG(LogTemp, Warning, TEXT("THE FUCKIN CLASS NAME: %s"), *BuildingComponentClassName->GetName());
		if (DieRobotCharacter)
		{
			DieRobotCharacter->BuildSystemManager->SetActiveBuildingComponentClass(BuildingComponentClassName);
			ADieRobotHUDBase* HUD = Cast<ADieRobotHUDBase>(GetHUD());

			//Close the Build Menu Panel (Doesn't Leave Build State, Same Effect as TAB)
			HUD->CloseBuildPanelMenu();

			//Clear Focused Widget
			FocusedWidget = nullptr;
		}
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("No data asset found on Building Component Icon Widget"));
	}
}

/* Tutorial Handling */

void ADieRobotPlayerController::InitializeTutorialStateBinding()
{
	ADieRobotGameStateBase* DieRobotGameStateBase = Cast<ADieRobotGameStateBase>(GetWorld()->GetGameState());
	if (DieRobotGameStateBase)
	{
		DieRobotGameStateBase->OnTutorialStateChange.AddDynamic(this, &ADieRobotPlayerController::HandleTutorialStateChanges);
	}
}

void ADieRobotPlayerController::HandleTutorialStateChanges(ETutorialState NewState)
{
	switch (NewState)
	{
	case ETutorialState::Wake1:
		DisableAllKeyboardInput();
		break;
	case ETutorialState::Wake2:
		EnableStandardKeyboardInput();
		break;
	case ETutorialState::Wake3:
		break;
	case ETutorialState::Default:
		break;
	case ETutorialState::Combat1:
		break;
	case ETutorialState::Combat2:
		break;
	case ETutorialState::Parts1:
		break;
	case ETutorialState::Building1:
		break;
	case ETutorialState::Building2:
		break;
	case ETutorialState::Building3:
		break;
	case ETutorialState::WaveStart:
		break;
	case ETutorialState::WaveComplete:
		break;
	case ETutorialState::TutorialComplete:
		EnableStandardKeyboardInput();
		break;
	}
}

void ADieRobotPlayerController::MissionViewModelInstantiation()
{
	TObjectPtr<UMVVMGameSubsystem> MVGS = GetWorld()->GetGameInstance()->GetSubsystem<UMVVMGameSubsystem>();
	if (IsValid(MVGS))
	{
		TObjectPtr<UMVVMViewModelCollectionObject> Collection = MVGS->GetViewModelCollection();
		if (IsValid(Collection))
		{
			TObjectPtr<UMissionViewModel> MissionVM = NewObject<UMissionViewModel>(GetLocalPlayer());
			if (!MissionVM)
			{
				UE_LOG(LogTemp, Warning, TEXT("PlayerController - Could not create the Mission View Model."));
				return;
			}
			FMVVMViewModelContext Context;
			Context.ContextClass = UMissionViewModel::StaticClass();
			Context.ContextName = "MissionVM";
			Collection->AddViewModelInstance(Context, MissionVM );
		}
	}
}

void ADieRobotPlayerController::ShowWelcomeMessage()
{
	ADieRobotHUDBase* HUD = Cast<ADieRobotHUDBase>(GetHUD());
	if (HUD)
	{
		HUD->CreateWelcomeMessageWidget();
	}
}


ETutorialState ADieRobotPlayerController::GetTutorialState() const
{
	ADieRobotGameStateBase* DieRobotGameState = Cast<ADieRobotGameStateBase>(GetWorld()->GetGameState());
	if (DieRobotGameState)
	{
		return DieRobotGameState->TutorialState;
	}

	return ETutorialState::Default;
}
