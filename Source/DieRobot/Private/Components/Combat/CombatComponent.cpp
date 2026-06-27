// Property of Paracosm.

#include "Components/Combat/CombatComponent.h"

#include "Character/TimberAnimInstance.h"
#include "Character/TimberPlayableCharacter.h"
#include "Weapons/TimberWeaponMeleeBase.h"
#include "Weapons/TimberWeaponRangedBase.h"
#include "Weapons/Abilities/WeaponAbilityBase.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	//Using A Character because a character will always be the owner of this component. Has Mesh.
	OwningCharacter = Cast<ACharacter>(GetOwner());
	UE_LOG(LogTemp, Warning, TEXT("OwningCharacter: %s"), *OwningCharacter->GetName());
	
}

void UCombatComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::SpawnRangedWeapon()
{
	SpawnWeaponAtSocketLocation(RangedWeaponClass,UnEquippedRangeSocket);
}

void UCombatComponent::SpawnMeleeWeapon()
{
	SpawnWeaponAtSocketLocation(MeleeWeaponClass, UnEquippedMeleeSocket);
}

EAbilityInputRequirement UCombatComponent::GetAbilityInputRequirement(bool bIsPrimaryAbility) const
{
	if (bIsPrimaryAbility)
	{
		//Get Primary Ability Input Type
		if (CurrentlyEquippedWeapon && CurrentlyEquippedWeapon->PrimaryWeaponAbility)
		{
			const UWeaponAbilityBase* PrimaryAbility = CurrentlyEquippedWeapon->PrimaryWeaponAbility->GetDefaultObject<UWeaponAbilityBase>();
			return PrimaryAbility->InputRequirement;

		}
	
		UE_LOG(LogTemp, Warning, TEXT("No Primary Ability Found"));
		return EAbilityInputRequirement::Default;
		
	}

	//Get Secondary Ability Input Type
	if (CurrentlyEquippedWeapon && CurrentlyEquippedWeapon->SecondaryWeaponAbility)
	{
		const UWeaponAbilityBase* SecondaryAbility = CurrentlyEquippedWeapon->SecondaryWeaponAbility->GetDefaultObject<UWeaponAbilityBase>();
		return SecondaryAbility->InputRequirement;
	}
	UE_LOG(LogTemp, Warning, TEXT("No Secondary Ability Found"));
	return EAbilityInputRequirement::Default;
	
}

void UCombatComponent::SpawnWeaponAtSocketLocation(TSubclassOf<ATimberWeaponBase> WeaponClassToSpawn, FName SocketName)
{
	// Spawning and Attaching the Weapon to the Socket of Right Hand on Leeroy
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	
	//Ensuring Sockets exists on the Mesh
	if (OwningCharacter && OwningCharacter->GetMesh() && OwningCharacter->GetMesh()->GetSocketByName(SocketName))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Socket: %s exists on the Mesh"), *SocketName.ToString());
		FTransform SocketWorldTransform = OwningCharacter->GetMesh()->GetSocketTransform(SocketName, RTS_World);
		FVector SocketWorldLocation = SocketWorldTransform.GetLocation();
		FRotator SocketWorldRotation = SocketWorldTransform.Rotator();
		
		//Spawn the Weapon
		ATimberWeaponBase* SpawnedActor = GetWorld()->SpawnActor<ATimberWeaponBase>(
			WeaponClassToSpawn,
			SocketWorldLocation,
			SocketWorldRotation,
			SpawnParams);
		
		if (SpawnedActor)
		{
			//Not Actually UnEquipping were doing the Initial Socket Attachment to the Unequipped Slot.
			if (ATimberWeaponRangedBase* SpawnedRangedWeapon = Cast<ATimberWeaponRangedBase>(SpawnedActor))
			{
				RangedWeaponInstance = SpawnedRangedWeapon;
				UnEquipWeapon(RangedWeaponInstance, UnEquippedRangeSocket);
			}
			else if (ATimberWeaponMeleeBase* SpawnedMeleeWeapon = Cast<ATimberWeaponMeleeBase>(SpawnedActor))
			{
				MeleeWeaponInstance = SpawnedMeleeWeapon;
				UnEquipWeapon(MeleeWeaponInstance, UnEquippedMeleeSocket);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket: %s does not exist on the Mesh"), *SocketName.ToString());
	}
	
}

FVector UCombatComponent::GetProjectileTargetLocation()
{
	/*
	 * Performs Raycast from the camera to the center of the screen and aligns the reticule to the hit location.
	 */
	FVector CameraLocation;
	FVector CameraDirection;

	int ViewportSizeX;
	int ViewportSizeY;

	if (APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController()))
	{
		PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

		FVector2d ScreenCenter = FVector2d(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);

		/*
		* The Camera is the screen as you see it.
		* Line Trace goes from the center of the screen (where reticule should be) out to the world by X (10,000.f) units.
		* Expensive, but that's why we only want 1 hit result.
		*/
		if (PC->DeprojectScreenPositionToWorld(ScreenCenter.X, ScreenCenter.Y, CameraLocation, CameraDirection))
		{
			if (CameraDirection.Normalize())
			{
				FHitResult HitResult;
				FVector End = CameraLocation + (CameraDirection * 3000.f);
				
				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(OwningCharacter);
				QueryParams.AddIgnoredActor(CurrentlyEquippedWeapon);
				bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, End, ECC_Visibility);

				/* Here we handle how to adjust Raycast End for UpClose Targets.*/
				if (bHit)
				{
					//If the Target Object is closer than 500 Units. Only Adjusting for Close Targets.
					if (FVector::Dist(HitResult.ImpactPoint, CameraLocation) < ProjectileAlignmentAdjustmentDistance)
					{
						return HitResult.ImpactPoint;
					}
					return End;
				}
				return End;
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Could not Retrieve Controller from Owner."));
	return FVector::ZeroVector;
}

void UCombatComponent::EquipWeapon(ATimberWeaponBase* WeaponInstance, FName EquippedWeaponSocketName)
{
	if (WeaponInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Equipping Weapon."));
		WeaponInstance->AttachToComponent(OwningCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, EquippedWeaponSocketName);

		CurrentlyEquippedWeapon = WeaponInstance;

		if (ATimberWeaponRangedBase* RangedWeapon = Cast<ATimberWeaponRangedBase>(CurrentlyEquippedWeapon))
		{
			UpdateCurrentWeaponState(EOwnerWeaponState::RangedWeaponEquipped);

			//Clean up of Reloading State during Ranged Equip.
			RangedWeapon->bIsReloading = false;
			Cast<UTimberAnimInstance>(OwningCharacter->GetMesh()->GetAnimInstance())->bIsReloading = false;
			bIsRifleEquipped = true;
		}
		else if (Cast<ATimberWeaponMeleeBase>(CurrentlyEquippedWeapon))
		{
			UpdateCurrentWeaponState(EOwnerWeaponState::MeleeWeaponEquipped);
			bIsMeleeEquipped = true;
			bCanMeleeAttack = true;
		}
		else
		{
			//Catch all
			UpdateCurrentWeaponState(EOwnerWeaponState::Default);
			UE_LOG(LogTemp, Warning, TEXT("Combat Component - EquipWeapon() - Weapon Instance not of a valid type."));
		}

		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Melee Weapon Instance to Equip."));
	}
}

void UCombatComponent::UnEquipWeapon(ATimberWeaponBase* WeaponInstance, FName UnEquipSocketName)
{
	//Place the Ranged Weapon on the Owners UnEquipped Ranged Socket Location
	if (WeaponInstance)
	{
		if (WeaponInstance == RangedWeaponInstance)
		{
			bIsRifleEquipped = false;
		}
		else if (WeaponInstance == MeleeWeaponInstance)
		{
			bIsMeleeEquipped = false;
		}

		CurrentlyEquippedWeapon = nullptr;
		
		WeaponInstance->AttachToComponent(OwningCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, 
		UnEquipSocketName);

		UpdateCurrentWeaponState(EOwnerWeaponState::Unequipped);
	}
}

void UCombatComponent::UnEquipCurrentlyEquippedWeapon()
{
	//Default is a catch-all, potentially not even possible to achieve.
	if (CurrentWeaponState == EOwnerWeaponState::Default || CurrentWeaponState == EOwnerWeaponState::Unequipped)
	{
		UE_LOG(LogTemp, Warning, TEXT("Combat Component - UnEquipCurrentlyEquippedWeapon() - No Weapon to UnEquip"));
		return;
	}

	if (CurrentWeaponState == EOwnerWeaponState::RangedWeaponEquipped && RangedWeaponInstance)
	{
		UnEquipWeapon(RangedWeaponInstance, UnEquippedRangeSocket);
	}
	else if (CurrentWeaponState == EOwnerWeaponState::MeleeWeaponEquipped && MeleeWeaponInstance)
	{
		UnEquipWeapon(MeleeWeaponInstance, UnEquippedMeleeSocket);
	}
}

void UCombatComponent::EquipMelee()
{
	if (CurrentlyEquippedWeapon)
	{
		UnEquipCurrentlyEquippedWeapon();
	}
	
	EquipWeapon(MeleeWeaponInstance, EquippedMeleeSocket);
	bIsEquipMontagePlaying = false;
}

void UCombatComponent::EquipRanged()
{
	if (CurrentlyEquippedWeapon)
	{
		UnEquipCurrentlyEquippedWeapon();
	}
	EquipWeapon(RangedWeaponInstance, EquippedRangeSocket);
	bIsEquipMontagePlaying = false;
}

void UCombatComponent::UnEquipAllWeapons()
{
	UnEquipWeapon(MeleeWeaponInstance, UnEquippedMeleeSocket);
	UnEquipWeapon(RangedWeaponInstance, UnEquippedRangeSocket);
}

void UCombatComponent::SendWeaponStateToOwnerAnimInstance()
{
	if (OwningCharacter && OwningCharacter->GetMesh()->GetAnimInstance())
	{
		if (UTimberAnimInstance* Anim = Cast<UTimberAnimInstance>(OwningCharacter->GetMesh()->GetAnimInstance()))
		{
			//Anim Instance will handle what to do with that information.
			Anim->UpdateOwnerWeaponState(CurrentWeaponState);
		}
	}
}

bool UCombatComponent::bHasEnoughPower(float AbilityCost, float CurrentWeaponPower)
{
	bool HasEnoughPower = AbilityCost < CurrentWeaponPower ? true : false;

	return HasEnoughPower;
}

void UCombatComponent::ConsumePower(ATimberWeaponBase* WeaponInstance, float AmountToConsume)
{
	if (WeaponInstance)
	{
		if (bHasEnoughPower(AmountToConsume, WeaponInstance->CurrentPower))
		{
			WeaponInstance->CurrentPower -= AmountToConsume;
			UE_LOG(LogTemp, Warning, TEXT("Consumed: %f, Remaining Power: %f"), AmountToConsume, WeaponInstance->CurrentPower);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Not Enough Power to Consume."));
		}
	}
}

void UCombatComponent::UpdateCurrentWeaponState(EOwnerWeaponState NewWeaponState)
{
	CurrentWeaponState = NewWeaponState;

	// We can do any broadcasts needed or value changes directly from here.
	SendWeaponStateToOwnerAnimInstance();
}

void UCombatComponent::HandlePrimaryAbility(const FInputActionValue& Value)
{
	if (CurrentlyEquippedWeapon && CurrentlyEquippedWeapon->PrimaryWeaponAbility)
	{
		/*
		 * Getting the default object of the primary ability. (Grants access before making Instance)
		 * Can check ability defaults validity before creating the ability object. 
		 */ 
		const UWeaponAbilityBase* PrimaryAbility = CurrentlyEquippedWeapon->PrimaryWeaponAbility->GetDefaultObject<UWeaponAbilityBase>();
		//Checking if the ability can be fired.
		if (ValidateWeaponAbility(PrimaryAbility))
		{
			/*
			 * Ability already Instantiated (Ex. Combo Attacks)
			 */
			if (CurrentWeaponAbility)
			{
				CurrentWeaponAbility->Execute(GenerateCurrentAbilityContext(Value));
			}
			else
			{
				/*
				 * Instantiating Ability Object from Ability Class
				 */
				TSubclassOf<UWeaponAbilityBase> AbilityClass = CurrentlyEquippedWeapon->PrimaryWeaponAbility;
				//Creating Instance of the Ability.
				UWeaponAbilityBase* Ability = NewObject<UWeaponAbilityBase>(this, AbilityClass);
				if (IsValid(Ability))
				{
					CurrentWeaponAbility = Ability;
					CurrentWeaponAbility->Execute(GenerateCurrentAbilityContext(Value));
				}
			}
		}
	}
}

void UCombatComponent::HandleSecondaryAbility(const FInputActionValue& Value)
{
	//If there is a weapon and the weapon has a secondary ability.
	if (CurrentlyEquippedWeapon && CurrentlyEquippedWeapon->SecondaryWeaponAbility)
	{
		//Get the default object of the secondary ability. (Grants access before making Instance)
		const UWeaponAbilityBase* SecondaryAbility = CurrentlyEquippedWeapon->SecondaryWeaponAbility->GetDefaultObject<UWeaponAbilityBase>();

		//Validate whether or not the Ability can be Fired.
		bool bIsAbilityValidated = ValidateWeaponAbility(SecondaryAbility);

		if (bIsAbilityValidated)
		{
			//Get the class of the Ability.
			TSubclassOf<UWeaponAbilityBase> AbilityClass = CurrentlyEquippedWeapon->SecondaryWeaponAbility;
			
			//Create an Instance of the Ability.
			UWeaponAbilityBase* Ability = NewObject<UWeaponAbilityBase>(this, AbilityClass);

			if (Ability)
			{
				//Run the Encapsulated Execute Function that runs the logic for the ability.
				Ability->Execute(GenerateCurrentAbilityContext(Value));
			}
		}
	}
}

void UCombatComponent::HandleSecondaryAbility_Started(const FInputActionValue& Value)
{
	//Just like HandleSecondaryAbility() BUT we Store a Ref to the Ability on the Combat Component for Future Updates.
	//If there is a weapon and the weapon has a secondary ability.
	if (CurrentlyEquippedWeapon && CurrentlyEquippedWeapon->SecondaryWeaponAbility)
	{
		//Get the default object of the secondary ability. (Grants access before making Instance)
		const UWeaponAbilityBase* SecondaryAbility = CurrentlyEquippedWeapon->SecondaryWeaponAbility->GetDefaultObject<UWeaponAbilityBase>();

		//Validate whether or not the Ability can be Fired.
		bool bIsAbilityValidated = ValidateWeaponAbility(SecondaryAbility);

		if (bIsAbilityValidated)
		{
			//Get the class of the Ability.
			TSubclassOf<UWeaponAbilityBase> AbilityClass = CurrentlyEquippedWeapon->SecondaryWeaponAbility;
			//Create an Instance of the Ability.
			UWeaponAbilityBase* Ability = NewObject<UWeaponAbilityBase>(this, AbilityClass);

			if (Ability)
			{
				//Storing the Ability on the Combat Component for Future Updates.
				CurrentWeaponAbility = Ability;
				//Run the Encapsulated Execute Function that runs the logic for the ability.
				Ability->Execute(GenerateCurrentAbilityContext(Value));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Secondary skill could not be validated."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Secondary Ability to Execute."));
	}
}

void UCombatComponent::HandleSecondaryAbility_Cancelled(const FInputActionValue& Value)
{
	if (CurrentWeaponAbility)
	{
		CurrentWeaponAbility->Execute_Cancelled(GenerateCurrentAbilityContext(Value));
	}
}

void UCombatComponent::HandleSecondaryAbility_Completed(const FInputActionValue& Value)
{
	if (CurrentWeaponAbility && CurrentWeaponAbility->InputRequirement == EAbilityInputRequirement::HoldOnly)
	{
		//UE_LOG(LogTemp, Warning, TEXT("CurrentWeaponAbility is Valid."));
		//ConsumePower(CurrentlyEquippedWeapon, CurrentWeaponAbility->PowerCost);
		CurrentWeaponAbility->Execute_Completed(GenerateCurrentAbilityContext(Value));
		
	}
}

bool UCombatComponent::ValidateWeaponAbility(const UWeaponAbilityBase* AbilityToValidate)
{
	switch (AbilityToValidate->ValidationType)
	{
	case EAbilityValidation::RequiresPower :
		return ValidatePowerWeaponAbility(AbilityToValidate);
	case EAbilityValidation::NoResourceCost:
		return ValidateNoResourceCostAbility(AbilityToValidate);
	case EAbilityValidation::RequiresAmmo:
		break;
	case EAbilityValidation::Default:
		UE_LOG(LogTemp, Warning, TEXT("No Validation Type Defined on Ability."));
		break;
	}
	return false;
		
}

bool UCombatComponent::ValidatePowerWeaponAbility(const UWeaponAbilityBase* AbilityToValidate)
{
	//Not Enough Power? On Cooldown? Not Valid.
	if (!bHasEnoughPower(AbilityToValidate->PowerCost, CurrentlyEquippedWeapon->CurrentPower) || CurrentlyEquippedWeapon->bIsOnPowerCooldown)
	{
		return false;
	}

	//Time Between Projectiles Firing isn't complete?
	ATimberWeaponRangedBase* RangedWeapon = Cast<ATimberWeaponRangedBase>(CurrentlyEquippedWeapon);
	if (RangedWeapon)
	{
		if (AbilityToValidate->bNeedsProjectileData)
		{
			if (RangedWeapon->bIsFireOnCooldown)
			{
				return false;
			}
		}
	}
	return true;
}

void UCombatComponent::ReloadRangedWeapon()
{
	if (CurrentWeaponState == EOwnerWeaponState::RangedWeaponEquipped && CurrentlyEquippedWeapon == RangedWeaponInstance)
	{
		//If the Weapon Uses power, there is no reloading.
		if (CurrentlyEquippedWeapon->bUsesPower)
		{
			return;
		}
		
		if (UTimberAnimInstance* Anim = Cast<UTimberAnimInstance>(OwningCharacter->GetMesh()->GetAnimInstance()))
		{
			if (!RangedWeaponInstance->bIsReloading && !Anim->bIsReloading)
			{
				if (RangedWeaponInstance->CurrentAmmo != RangedWeaponInstance->MaxAmmo)
				{
					Anim->bIsReloading = true;
					RangedWeaponInstance->PlayReloadMontage();
				}
			}
		}
	}
}

void UCombatComponent::PlayEquipWeaponMontage(FName MontageSectionName)
{
	bIsEquipMontagePlaying = true;

	//Need to call the PlayEquipWeaponMontage on the Player Character but want to avoid Coupling/Casting to that specific Character.
	if (ICombatComponentAnimUser* AnimUser = Cast<ICombatComponentAnimUser>(OwningCharacter))
	{
		AnimUser->PlayWeaponEquipAnimationMontage(MontageSectionName);
	}
}

void UCombatComponent::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	//Report back to Current Ability that some montage has finished playing.
	//Ability must track which montage was last played.
	if (CurrentWeaponAbility)
	{
		//HandleMontageEnded() Override function on WeaponAbilityBase
		CurrentWeaponAbility->HandleMontageEnded(Montage, bInterrupted);
	}
}

void UCombatComponent::PlayCharacterAnimationMontage(UAnimMontage* Montage, FName MontageSectionName, float PlayRate, bool TrackStages)
{
	if (OwningCharacter)
	{
		if (UAnimInstance* Anim = OwningCharacter->GetMesh()->GetAnimInstance())
		{
			Anim->Montage_Play(Montage, PlayRate);
		
			Anim->Montage_JumpToSection(MontageSectionName, Montage);
			if (TrackStages && !Anim->OnMontageEnded.IsBound())
			{
				UE_LOG(LogTemp, Warning, TEXT("Binding OnMontageEnded to Combat Component."));
				//If we want reports back to ability that some montage has finished playing or been interrupted.
				Anim->OnMontageEnded.AddDynamic(this, &UCombatComponent::HandleMontageEnded);
			}
		}
	}
}

void UCombatComponent::StopCharacterAnimationMontage(UAnimMontage* Montage, float BlendOutTime)
{
	if (OwningCharacter)
	{
		if (UAnimInstance* Anim = OwningCharacter->GetMesh()->GetAnimInstance())
		{
			Anim->StopAllMontages(BlendOutTime);
		}
	}
}

FAbilityContext UCombatComponent::GenerateCurrentAbilityContext(const FInputActionValue& InputValue)
{
	FAbilityContext CurrentAbilityContext;
	
	CurrentAbilityContext.Instigator = OwningCharacter;

	CurrentAbilityContext.WeaponInstance = CurrentlyEquippedWeapon;
	//UE_LOG(LogTemp, Warning, TEXT("Current Weapon = %s"), *CurrentAbilityContext.WeaponInstance->GetName())

	CurrentAbilityContext.TargetLocation = GetProjectileTargetLocation();

	CurrentAbilityContext.CombatComponent = this;

	CurrentAbilityContext.InputActionValue = InputValue;

	return CurrentAbilityContext;
}

bool UCombatComponent::ValidateNoResourceCostAbility(const UWeaponAbilityBase* WeaponAbilityBase)
{
	if (ATimberWeaponMeleeBase* MeleeWeapon = Cast<ATimberWeaponMeleeBase>(CurrentlyEquippedWeapon))
	{
		// Is fully Equipped?
		if (MeleeWeapon && bIsMeleeEquipped)
		{
			// CanAttackAgain?
			if (bCanMeleeAttack)
			{
				return true;
			}
		}
	}
	return false;
}





