// Property of Paracosm Industries. Dont use my shit.


#include "Components/Inventory/InventoryManagerComponent.h"

#include "BuildSystem/BuildableBase.h"
#include "Character/TimberPlayableCharacter.h"
#include "Controller/TimberPlayerController.h"


UInventoryManagerComponent::UInventoryManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInventoryManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	LoadOwningPlayerState();
}

void UInventoryManagerComponent::LoadOwningPlayerState()
{

	/*
	 * PCH - Player Character
	 * PC - Player Controller
	 * PS - Player State
	 */
	ATimberPlayableCharacter* PCh = Cast<ATimberPlayableCharacter>(GetOwner());
	if(PCh)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Inventory Manager Component - Player Character Loaded from GetOwner()"));
		ATimberPlayerController* PC = Cast<ATimberPlayerController>(PCh->GetController());
		if(PC)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Inventory Manager Component - Player Controller Loaded."));
			PS = PC->GetPlayerState<APlayerStateBase>();
			if(PS == nullptr)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Inventory Manager Component - Player State Cast Failed."));
			}
			if(PS)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Player State Loaded for the Inventory Management Component."))
			}
		}
	}
}

void UInventoryManagerComponent::AddPartsToInventory(int PartsToAdd)
{
	if(PS)
	{
		
		PS->MainInventory->NumberOfParts += PartsToAdd;
		//UE_LOG(LogTemp, Warning, TEXT("Part Added to Inventory. Parts In Inventory = %d"), PS->MainInventory->NumberOfParts);

		UpdateInventoryHandle.Broadcast();
	}
}

void UInventoryManagerComponent::AddMechanismsToInventory(int MechanismsToAdd)
{
	if(PS)
	{
		PS->MainInventory->NumberOfMechanism += MechanismsToAdd;
		UpdateInventoryHandle.Broadcast();
	}
}

void UInventoryManagerComponent::AddUniquesToInventory(int UniquesToAdd)
{
	if(UniquesToAdd)
	{
		PS->MainInventory->NumberOfUniques += UniquesToAdd;
		UpdateInventoryHandle.Broadcast();
	}
}

void UInventoryManagerComponent::RemovePartsFromInventory(int PartsToRemove)
{
	if(PartsToRemove)
	{
		PS->MainInventory->NumberOfParts -= PartsToRemove;
		UpdateInventoryHandle.Broadcast();
	}
}

void UInventoryManagerComponent::RemoveMechanismsFromInventory(int MechanismsToRemove)
{
	if(MechanismsToRemove)
	{
		PS->MainInventory->NumberOfMechanism -= MechanismsToRemove;
		UpdateInventoryHandle.Broadcast();
	}
}

void UInventoryManagerComponent::RemoveUniquesFromInventory(int UniquesToRemove)
{
	if(UniquesToRemove)
	{
		PS->MainInventory->NumberOfUniques -= UniquesToRemove;
		UpdateInventoryHandle.Broadcast();
	}
}

bool UInventoryManagerComponent::bCanAffordCost(FBuildableCost CostOfBuildable)
{
	// Getting Values
	int CostOfParts = CostOfBuildable.CostOfParts;
	int CostOfMechanisms = CostOfBuildable.CostOfMechanisms;
	int CostOfUniques = CostOfBuildable.CostOfUniques;

	int AvailableParts = PS->MainInventory->NumberOfParts;
	int AvailableMechanisms = PS->MainInventory->NumberOfMechanism;
	int AvailableUniques = PS->MainInventory->NumberOfUniques;

	bool bCanAffordParts = false;
	bool bCanAffordMechanisms = false;
	bool bCanAffordUniques = false;

	
	// Comparing Costs
	if(AvailableParts >= CostOfParts)
	{
		bCanAffordParts = true;
	}
	else
	{
		return false;
	}

	if(AvailableMechanisms >= CostOfMechanisms)
	{
		bCanAffordMechanisms = true;
	}
	else
	{
		return false;
	}

	if(AvailableUniques >= CostOfUniques)
	{
		bCanAffordUniques = true;
	}
	else
	{
		return false;
	}

	if(bCanAffordParts && bCanAffordMechanisms && bCanAffordUniques)
	{
		return true;
	}

	return false;
}

bool UInventoryManagerComponent::bHandleBuildableTransaction(FBuildableCost CostOfBuildable)
{

	//If the player can afford the cost of the buildable, then remove the parts from the inventory.
	
	if(bCanAffordCost(CostOfBuildable))
	{
		RemovePartsFromInventory(CostOfBuildable.CostOfParts);
		RemoveMechanismsFromInventory(CostOfBuildable.CostOfMechanisms);
		RemoveUniquesFromInventory(CostOfBuildable.CostOfUniques);
		return true;
	}
	
	return false;
	
}

int UInventoryManagerComponent::GetPartsInInventory()
{
	if(PS)
	{
		return PS->MainInventory->NumberOfParts;
	}
	else
	{
		return 9999999;
	}
}

