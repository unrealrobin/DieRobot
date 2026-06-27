// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NarrativeDialogueLibrary.generated.h"

class UDialogueSingle;
/**
 * 
 */
UCLASS()
class DIEROBOT_API UNarrativeDialogueLibrary : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identifier")
	TMap<FName, UDialogueSingle*> NarrativeDialogueList;
	
};
