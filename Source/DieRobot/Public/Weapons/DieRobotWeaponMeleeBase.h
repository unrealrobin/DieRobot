// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/DieRobotWeaponBase.h"
#include "DieRobotWeaponMeleeBase.generated.h"

class ADieRobotPlayableCharacter;
class USoundCue;

UCLASS()
class DIEROBOT_API ADieRobotWeaponMeleeBase : public ADieRobotWeaponBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADieRobotWeaponMeleeBase();

	/*Collision Component*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components")
	UBoxComponent* WeaponBoxComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* NiagaraEffectSpawnLocation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* WeaponSkeletalMesh = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	AActor* WeaponOwner;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	ADieRobotPlayableCharacter* WeaponInstigator;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapons")
	float MaxWeaponEnergy = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapons")
	float CurrentWeaponEnergy = 100.0f;
	
	UFUNCTION(BlueprintCallable, Category="Collision")
	void HandleWeaponCollision(bool ShouldReadyCollision) const;
	
	UFUNCTION()
	void OnWeaponOverlapBegin(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent*
		OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnWeaponOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	

	UFUNCTION()
	void OnPlayerWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent*
		OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void EmptyActorToIgnoreArray();
	
	UFUNCTION()
	virtual void OnAiWeaponOverlap(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent*
		OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	/* Sounds */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Sounds")
	USoundCue* AttackSwooshSound;
	
};
