// Property of Paracosm Industries. Don't use my shit.

#pragma once

#include "CoreMinimal.h"
#include "BossBase.h"
//#include "Character/Enemies/TimberEnemyCharacter.h"
#include "BossLola.generated.h"

class ABossAIControllerBase;
class AFloaterDrones;

UENUM(BlueprintType)
enum class ELolaState : uint8
{
	Stunned UMETA(DisplayName = "Stunned"),
	NotStunned UMETA(DisplayName = "Not Stunned")
};

UENUM(BlueprintType)
enum class ELolaTakeDamePotential : uint8
{
	NotDamageable UMETA(DisplayName = "Not Damageable"),
	Damageable UMETA(DisplayName = "Damageable")
};


UCLASS()
class DIEROBOT_API ABossLola : public ABossBase
{
	GENERATED_BODY()

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLolaStunChange, bool, bIsLolaStunned);
	FOnLolaStunChange OnLolaStunChange;
	
	// Sets default values for this character's properties
	ABossLola();
	void BindToDroneDeathDelegates();
	void InitializeLolaController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	ELolaState LolaState = ELolaState::NotStunned;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	ELolaTakeDamePotential LolaTakeDamagePotential = ELolaTakeDamePotential::NotDamageable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	bool bCanLolaTakeDamage = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drones")
	TArray<AFloaterDrones*> FloaterDronesArray;

	UFUNCTION()
	void PopulateDronesArray();

	UFUNCTION()
	void RandomizeDroneVulnerability();

	UFUNCTION()
	void RemoveDroneFromArray(AFloaterDrones* Drone);

	UFUNCTION()
	void SetDronesToNotDamageableDuringStun();

	FTimerHandle RandomizeDroneVulnerability_Handle;
	FTimerHandle LolaStunnedTimer_Handle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timers")
	float LolaStunTime = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timers")
	float RandomizationTime = 8.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	ABossAIControllerBase* LolaController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float WalkSpeed = 300.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<UPrimitiveComponent*> CapsuleComponentsArray;
	
	void SetLolaNotDamageable();

	void SetLolaToDamageable();
	
	void GetAllCapsuleComponents();
	
public:

	virtual void Tick(float DeltaTime) override;

	virtual void TakeDamage(FDamagePayload DamagePayload) override;
	
	UFUNCTION(BlueprintCallable)
	void SetLolaToNotStunned();
	
	UFUNCTION()
	void SetLolaToStunned(AFloaterDrones* Drone);

	UFUNCTION(BlueprintCallable)
	void DemolishBuildable();

	/*Collision Components*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UCapsuleComponent* HeadCollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UCapsuleComponent* LeftArmCollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UCapsuleComponent* RightArmCollisionComponent;

	/*Drones*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UChildActorComponent* Drone1Component;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UChildActorComponent* Drone2Component;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UChildActorComponent* Drone3Component;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drones")
	AFloaterDrones* Drone1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drones")
	AFloaterDrones* Drone2;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drones")
	AFloaterDrones* Drone3;

	UFUNCTION()
	void HandleDroneDeath(AFloaterDrones* Drone);
	
	//Handles the initial components of the boss.
	UFUNCTION(BlueprintCallable)
	void LolaInitializeComponents();
};




