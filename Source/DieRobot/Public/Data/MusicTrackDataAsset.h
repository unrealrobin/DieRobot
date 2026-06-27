// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MetasoundSource.h"
#include "MusicTrackDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UMusicTrackDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	UMetaSoundSource* TrackMetaSound;
};
