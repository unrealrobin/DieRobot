#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MissionEventPayload.generated.h"

USTRUCT(BlueprintType)
struct FMissionEventPayload
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mission Event Payload")
	TWeakObjectPtr<AActor> InstigatingActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mission Event Payload")
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	//Defines the mission Type Tag
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mission Event Payload")
	FGameplayTag EventTag;

	//An Assortment or Context Tags.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mission Event Payload")
	FGameplayTagContainer ContextTags;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mission Event Payload")
	int Count = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mission Event Payload")
	int WaveNumber = 0;
	
};

