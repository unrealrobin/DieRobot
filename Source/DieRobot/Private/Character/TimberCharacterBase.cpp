// Property of Paracosm Industries.


#include "Character/TimberCharacterBase.h"

#include "Components/CapsuleComponent.h"

// Sets default values
ATimberCharacterBase::ATimberCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ATimberCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATimberCharacterBase::AddOverlayMaterialToCharacter(UMaterialInterface* MaterialToOverlay, float TimeBeforeRemoval)
{
	//Function has an option for Automatic Remove with the TImeBeforeRemoval float.
	//Passing in 0 will allow us to remove on some event instead.
	if (TimeBeforeRemoval > 0)
	{
		//Material to be removed with timer.
		if (MaterialToOverlay != nullptr)
		{
			USkeletalMeshComponent* CharacterMesh = GetMesh();
			if (CharacterMesh)
			{
				CharacterMesh->SetOverlayMaterial(MaterialToOverlay);

				//Remove at end of Timer.
				FTimerHandle TimeToRemoveOverlayHandle;
				GetWorld()->GetTimerManager().SetTimer(TimeToRemoveOverlayHandle, this,  
				&ATimberCharacterBase::RemoveOverlayMaterialFromCharacter, TimeBeforeRemoval);
			}
		}
	}
	else
	{
		//Material to be removed by some other event.
		if (MaterialToOverlay != nullptr)
		{
			USkeletalMeshComponent* CharacterMesh = GetMesh();
			if (CharacterMesh)
			{
				CharacterMesh->SetOverlayMaterial(MaterialToOverlay);
			}
		}
	}
}

void ATimberCharacterBase::RemoveOverlayMaterialFromCharacter()
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (CharacterMesh)
	{
		CharacterMesh->SetOverlayMaterial(nullptr);
	}
}
