// Property of Paracosm.


#include "Subsystems/SynergySystem/SynergySystem.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/StatusEffect/StatusEffectHandlerComponent.h"
#include "Data/DataAssets/StatusEffects/StatusEffectBase.h"
#include "Synergy/EffectLogic/EffectAbilityHandlerBase.h"

void USynergySystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	EmergentEffectsMap = LoadEmergentEffects();
	SetupEmergentEffectRules();
	//UE_LOG(LogTemp, Warning, TEXT("Synergy System Subsystem - Synergy Rules Created."));
}

UEffectAbilityHandlerBase* USynergySystem::GetOrSetAbilityEffect(FGameplayTag Tag, FStatusEffect StatusEffect)
{
	if (EffectAbilityMap.Find(Tag))
	{
		return EffectAbilityMap.FindChecked(Tag);
	}

	//Creating the Instance of the Handler Class for the Unique Tag Type.
	//Polymorphism uses the Runtime Execute of the Derived Class.
	UEffectAbilityHandlerBase* NewEffectAbilityHandler = NewObject<UEffectAbilityHandlerBase>(this, StatusEffect.EffectAbilityHandlerClass);
	EffectAbilityMap.Add(Tag, NewEffectAbilityHandler);
	return NewEffectAbilityHandler;
	
}

void USynergySystem::HandleEmergentTagChecks(FGameplayTag Tag, UStatusEffectHandlerComponent* StatusEffectComponent)
{
	if (!SynergyRules.Contains(Tag)) return;
	
	for (FSynergyRules Rule: *SynergyRules.Find(Tag))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Synergy System Subsystem - Checking Tag for Synergy: %s"), *Tag.ToString());
		if (StatusEffectComponent->StatusEffectIdTagContainer.HasAll(Rule.SynergyTags))
		{
			//UE_LOG(LogTemp, Warning, TEXT("Synergy System Subsystem - Found Matching Synergy: %s"), *Tag.ToString());
			StatusEffectComponent->AddEmergentTag(Rule.EmergentTag, 5.0f);
		}
	}
}

void USynergySystem::HandleCriticalEffectsLogic(FGameplayTag Tag, UStatusEffectHandlerComponent* StatusEffectComponent,
	const FStatusEffect& Effect)
{
	if (IsValid(Effect.EffectAbilityHandlerClass))
	{
		UEffectAbilityHandlerBase* Handler = GetOrSetAbilityEffect(Tag, Effect);
		if (IsValid(Handler))
		{
			AActor* Target = StatusEffectComponent->OwningEnemyCharacter;
			if (IsValid(Target))
			{
				Handler->ExecuteEffect(Target, Effect);
			}
		}
	}
}

void USynergySystem::ProcessTagForSynergy(FGameplayTag Tag, UStatusEffectHandlerComponent* StatusEffectComponent, FStatusEffect& Effect)
{

	switch (Effect.EffectLevel)
	{
	case EStatusEffectLevel::Major:
		HandleEmergentTagChecks(Tag, StatusEffectComponent);
		break;
	case EStatusEffectLevel::Ultimate:
		HandleCriticalEffectsLogic(Tag, StatusEffectComponent, Effect);
		break;
	case EStatusEffectLevel::Default:
		UE_LOG(LogTemp, Warning, TEXT("Synergy System Subsystem - Tag not Synergizable."));
		break;
	default: break;
	}
}

FName USynergySystem::GetLastNameOfGameplayTag(FGameplayTag Tag)
{
	FString TagName = Tag.GetTagName().ToString();
	TArray<FString> Parts;
	TagName.ParseIntoArray(Parts, TEXT("."), true);

	//Converts the String to Uppercase
	FName EffectName = Parts.Num() > 0 ? FName(*Parts.Last().ToUpper()) : NAME_None;

	return EffectName;
}

void USynergySystem::ExecuteEffectHandlerLogic(UEffectAbilityHandlerBase* EffectHandler)
{
	//Logic.
}

void USynergySystem::CreateSynergyRule(FName Tag1, FName Tag2, FName EmergentTag)
{
	//Creation of Rule - Takes 2 Tags FName
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(Tag1));
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(Tag2));

	//Populates EmergentEffectComboRequirements
	EmergentEffectComboRequirements.FindOrAdd(FGameplayTag::RequestGameplayTag(EmergentTag)).Add(TagContainer);
	
	//Populates SynergyRules TMap
	FSynergyRules NewRule(TagContainer, FGameplayTag::RequestGameplayTag(EmergentTag));

	for (FGameplayTag Tag: TagContainer)
	{
		SynergyRules.FindOrAdd(Tag).Add(NewRule);
	}
	
}

void USynergySystem::SetupEmergentEffectRules()
{
	// --- Ensure Proper Spelling of Gameplay Tags with SynergySystemTags.ini - Will cause Crashes if Misspelled ---
	
	/* Emergent - Wet */
	CreateSynergyRule(FName("SynergySystem.Effect.Fire.Scorch"), FName("SynergySystem.Effect.Frost.Freeze"), FName("SynergySystem.EmergentEffect.Wet"));
	CreateSynergyRule(FName("SynergySystem.Effect.Arc.Static"), FName("SynergySystem.Effect.Frost.Freeze"), FName("SynergySystem.EmergentEffect.Wet"));

	/* Emergent - Charred */
	CreateSynergyRule(FName("SynergySystem.Effect.Fire.Scorch"), FName("SynergySystem.Effect.Corrosion.Degrade"), FName("SynergySystem.EmergentEffect.Charred"));
	CreateSynergyRule(FName("SynergySystem.Effect.Fire.Scorch"), FName("SynergySystem.Effect.Physical.Crush"), FName("SynergySystem.EmergentEffect.Charred"));

	/* Emergent - Sparked */
	CreateSynergyRule(FName("SynergySystem.Effect.Arc.Static"), FName("SynergySystem.Effect.Physical.Crush"), FName("SynergySystem.EmergentEffect.Sparked"));
	CreateSynergyRule(FName("SynergySystem.Effect.Arc.Static"), FName("SynergySystem.Effect.Fire.Scorch"), FName("SynergySystem.EmergentEffect.Sparked"));

	/* Emergent - Demolished */
	CreateSynergyRule(FName("SynergySystem.Effect.Corrosion.Degrade"), FName("SynergySystem.Effect.Physical.Crush"), FName("SynergySystem.EmergentEffect.Demolish"));
	CreateSynergyRule(FName("SynergySystem.Effect.Frost.Freeze"), FName("SynergySystem.Effect.Physical.Crush"), FName("SynergySystem.EmergentEffect.Demolish"));

	/* Emergent - Corroded */
	CreateSynergyRule(FName("SynergySystem.Effect.Corrosion.Degrade"), FName("SynergySystem.Effect.Arc.Static"), FName("SynergySystem.EmergentEffect.Corroded"));
	CreateSynergyRule(FName("SynergySystem.Effect.Corrosion.Degrade"), FName("SynergySystem.Effect.Frost.Freeze"), FName("SynergySystem.EmergentEffect.Corroded"));
}

TMap<FGameplayTag, UStatusEffectBase*> USynergySystem::LoadEmergentEffects()
{
	TMap<FGameplayTag, UStatusEffectBase*> TagToEffectMap;
	FString FilePathToEmergentEffects = TEXT("/Game/Data/StatusEffects/Emergent");
	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	
	TArray<FAssetData> DataAssetsBase;
	
	AssetRegistryModule.Get().GetAssetsByPath(FName(*FilePathToEmergentEffects), DataAssetsBase, false, true);

	for (const FAssetData& Data : DataAssetsBase)
	{
		if (UStatusEffectBase* LoadedAsset = Cast<UStatusEffectBase>(Data.GetAsset()))
		{
			FGameplayTag EffectIdTag = LoadedAsset->StatusEffect.EffectIdTag;
			// Add to the map using the EffectIdTag as the key
			TagToEffectMap.Add(EffectIdTag, LoadedAsset);
		}
	}

	return TagToEffectMap;
}
