// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DialogueSingle.generated.h"

UENUM(BlueprintType) // Allows usage in Blueprints
enum class ESpeaker : uint8
{
	Molly UMETA(DisplayName = "Molly"), // Enum value with display name
	DrVexler UMETA(DisplayName = "Dr. Vexler"),
	Default UMETA(DisplayName = "Default"),
};


class UMetaSoundSource;
/**
 * 
 */
UCLASS()
class DIEROBOT_API UDialogueSingle : public UDataAsset
{
	GENERATED_BODY()

public:

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Single Dialogue")
	ESpeaker Speaker = ESpeaker::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Single Dialogue")
	FString DialogueText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Single Dialogue")
	UMetaSoundSource* DialogueVoiceOver = nullptr;
	
	
};
