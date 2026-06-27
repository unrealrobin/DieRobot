// Property of Paracosm.


#include "Synergy/EffectLogic/EffectAbilityHandlerBase.h"

#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/StatusEffect/StatusEffectHandlerComponent.h"
#include "Data/DataAssets/StatusEffects/StatusEffectBase.h"

UEffectAbilityHandlerBase::UEffectAbilityHandlerBase()
{
}

void UEffectAbilityHandlerBase::ExecuteEffect(AActor* TargetActor,
                                              const FStatusEffect& StatusEffect)
{
	if (IsValid(TargetActor))
	{
		TargetActorCache = TargetActor;
	}
	
	if (StatusEffect.TagsToRemove.Num() > 0)
	{
		FGameplayTag TagToRemove = StatusEffect.TagsToRemove.GetByIndex(0);
		RemoveEmergentTagFromActor(TargetActor, TagToRemove);
	}
}

void UEffectAbilityHandlerBase::HandleSFX(AActor* TargetActor, const FStatusEffect& StatusEffect)
{
	//Play Any Sounds Required.
}

void UEffectAbilityHandlerBase::HandleVFX(AActor* TargetActor, const FStatusEffect& StatusEffect)
{
	//Active any Niagara Systems that are required.
}

void UEffectAbilityHandlerBase::RemoveEmergentTagFromActor(AActor* TargetActor, FGameplayTag Tag)
{
	if (IsValid(TargetActor))
	{
		ATimberEnemyCharacter* Enemy = Cast<ATimberEnemyCharacter>(TargetActor);
		if (IsValid(Enemy))
		{
			Enemy->StatusEffectHandler->RemoveEmergentTag(Tag);
		}
	}
}

TArray<AActor*> UEffectAbilityHandlerBase::SortActorsByDistance(TArray<AActor*> TargetActors, bool bReturnAllActors,int HowManyToReturn)
{
	//Target Actor Cache is a Stored Ref to the initial Actor the rest of this should be based on.
	//Target Actors is already retrieved.
	if (!TargetActorCache || TargetActors.Num() < 2)
	{
		return TargetActors;
	}

	// Sort by distance to the cached target actor
	TargetActors.Sort([this](const AActor& A, const AActor& B)
	{
		float DistanceA = FVector::DistSquared(A.GetActorLocation(), TargetActorCache->GetActorLocation());
		float DistanceB = FVector::DistSquared(B.GetActorLocation(), TargetActorCache->GetActorLocation());
		return DistanceA < DistanceB;
	});

	if (bReturnAllActors)
	{
		return TargetActors;
	}
    
	// Return only the specified number of closest actors
	// We are not 100% sure how many actors we have in Range so we verify each index.
	TArray<AActor*> ResultActors;
	int32 NumToReturn = FMath::Min(HowManyToReturn, TargetActors.Num());
	for (int32 i = 0; i < NumToReturn; i++)
	{
		if (ResultActors.IsValidIndex(i))
		{
			ResultActors.Add(TargetActors[i]);
		}
		else
		{
			break;
		}
	}
    
	return ResultActors;
}

TArray<AActor*> UEffectAbilityHandlerBase::GetActorsInRadius_IncludeTarget(AActor* SourceActor, float Radius)
{
	TArray<AActor*> ActorsInRadius;

	//If the found actors Z location difference is larger than this, don't count it.
	float VerticalOffsetAmount = 75.0f;
	
	if (IsValid(SourceActor) && Radius > 0.0f)
	{
		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams QueryParams;
		bool bHits = SourceActor->GetWorld()->OverlapMultiByChannel(OverlapResults, SourceActor->GetActorLocation(), FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Radius), QueryParams);
		if (bHits && OverlapResults.Num() > 0)
		{
			for (FOverlapResult& OverlapResult : OverlapResults)
			{
				//Checking Z Diffrence of Hit Actor and Source Actor
				if (FMath::Abs(SourceActor->GetActorLocation().Z - OverlapResult.GetActor()->GetActorLocation().Z) < VerticalOffsetAmount)
				{
					if (OverlapResult.GetActor()->IsA(ATimberEnemyCharacter::StaticClass()))
					{
						ActorsInRadius.Add(OverlapResult.GetActor());
					}
				}
			}
		}
	}
	return ActorsInRadius;
}
