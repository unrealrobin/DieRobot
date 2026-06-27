// Property of Paracosm Industries. 

#pragma once

#include "CoreMinimal.h"
#include "MusicTrackDataAsset.h"
#include "Engine/DataAsset.h"
#include "MusicLibraryDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UMusicLibraryDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TMap<FName, UMusicTrackDataAsset*> MusicTrackList;
};
