// Property of Paracosm.


#include "Weapons/Abilities/WeaponAbilityBase.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/SFX/SFXManagerSubsystem.h"

void UWeaponAbilityBase::HandleCleanup(FAbilityContext Context)
{
	UE_LOG(LogTemp, Warning, TEXT("Handling Ability Cleanup"));
	Context.CombatComponent->CurrentWeaponAbility = nullptr;
	Context.CombatComponent->bCanMeleeAttack = true;
}

void UWeaponAbilityBase::PlayNiagaraEffectAtLocation(
	UNiagaraSystem* NiagaraSystem, FVector Location, FRotator Rotation, bool AutoDestroy, bool AutoActive)
{
	if (NiagaraSystem != nullptr)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			NiagaraSystem,
			Location,
			Rotation,
			FVector(1.0f),
			AutoDestroy,
			AutoActive);
	}
}

void UWeaponAbilityBase::PlayNiagaraEffectAttached(
	UNiagaraSystem* NiagaraSystem, UPrimitiveComponent* Component, FName Name, const FVector& OffsetVector,
	const FRotator& Rotator, EAttachLocation::Type SnapToTargetIncludingScale, bool AutoDestroy, bool AutoActive)
{
	if (NiagaraSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(NiagaraSystem, Component, Name, OffsetVector, Rotator,SnapToTargetIncludingScale, AutoDestroy, AutoActive);
	}
}

void UWeaponAbilityBase::PlayEffectSFX(FVector SoundLocation, FName TrackName)
{
	USFXManagerSubsystem* SoundSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<USFXManagerSubsystem>();
	if (SoundSubsystem)
	{
		SoundSubsystem->PlaySoundAtLocation(TrackName, SoundLocation);
	}
}




