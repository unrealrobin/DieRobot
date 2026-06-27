// Property of Paracosm.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StatusEffect/StatusEffectHandlerComponent.h"
#include "EffectConditionTypes.generated.h"

UENUM(BlueprintType)
enum class EEffectConditionType : uint8
{
	Always UMETA(DisplayName = "Always Trigger"),
	HasTag UMETA(DisplayName = "Has Tag"),
	StackCount UMETA(DisplayName = "Stack Count"),
	HealthThreshold UMETA(DisplayName = "Health Threshold"),
	Chance UMETA(DisplayName = "Chance"),
	Default UMETA(DisplayName = "Default/None"),
};

//This is the State in which the Target Actor is when Evaluating the Effects Condition.
USTRUCT(BlueprintType)
struct FEffectConditionContext
{
	GENERATED_BODY()

public:
	// Who is being hit with this.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* TargetActor = nullptr;

	//TargetActors Status Effect Handler Component
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStatusEffectHandlerComponent* TargetStatusEffectHandlerComponent = nullptr;

	// Trap, Construct, Projectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* SourceActor = nullptr;

	UPROPERTY()
	UWorld* WorldContext = nullptr;
};