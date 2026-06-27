// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerVignetteComponent.generated.h"

UENUM(BlueprintType , meta = (ScriptName = "DieRobotDamageVignetteState"))
enum class EDamageVignetteRate: uint8
{
	FullHealth UMETA(DisplayName = "100%"),
	LightDamage UMETA(DisplayName = "80%"),
	HighDamage UMETA(DisplayName = "50%"),
	Critical UMETA(DisplayName = "25%"),
};

USTRUCT(BlueprintType)
struct FDamageVignetteState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	EDamageVignetteRate State = EDamageVignetteRate::FullHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EdgeFalloff = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor VignetteColor = FLinearColor::Red;

	FDamageVignetteState() {}

	FDamageVignetteState(EDamageVignetteRate S, float InIntensity, float InFalloff, FLinearColor InColor)
		: State(S) , DamageIntensity(InIntensity), EdgeFalloff(InFalloff), VignetteColor(InColor)
	{}
};

class UPostProcessComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DIEROBOT_API UPlayerVignetteComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerVignetteComponent();

	void BeginDestroy() override;

	//Where the Vignette gets added and modified.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UPostProcessComponent* VignettePostProcessComponent = nullptr;

	//Stores the Base Material used for the Vignette.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* DamageVignetteMaterialBase = nullptr;

	//Not Assignable in Editor. Required for Adjusting Params at Runtime.
	UPROPERTY()
	UMaterialInstanceDynamic* DamageVignetteMid = nullptr;

	//Tracks the Current State of the Vignette.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vignette")
	EDamageVignetteRate CurrentVignetteState = EDamageVignetteRate::FullHealth;

	UFUNCTION()
	void HandleHealthChange(float HealthPercent);

	void SetVignetteStateToFullHealth();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette")
	float VignetteLerpTime = 0.5f;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Vignette State")
	FDamageVignetteState CurrentState = FDamageVignetteState(EDamageVignetteRate::FullHealth, 0.0f, 0.0f, FLinearColor::Red);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Vignette State")
	FDamageVignetteState FullState = FDamageVignetteState(EDamageVignetteRate::FullHealth, 0.0f, 0.0f, FLinearColor::Red);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Vignette State")
	FDamageVignetteState LightState = FDamageVignetteState(EDamageVignetteRate::LightDamage,2.0f, 5.0f, FLinearColor::Red);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Vignette State")
	FDamageVignetteState HighState = FDamageVignetteState(EDamageVignetteRate::HighDamage,4.0f, 4.0f, FLinearColor::Red);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Vignette State")
	FDamageVignetteState CriticalState = FDamageVignetteState(EDamageVignetteRate::Critical,5.0f, 3.0f, FLinearColor::Red);

	//Gets the Damage Vignette Params at Any Point in Time.
	FDamageVignetteState GetCurrentDamageVignetteState();
	
private:

	//Current Vignette Lerp Timer. Destroyed in BeginDestroyed()
	FTimerHandle DamageVignetteTimerHandle;
	
	void InitializeVignetteDynamicMaterial();
	
	void LerpDamageVignetteState(FDamageVignetteState From, FDamageVignetteState To, float Time);
};
