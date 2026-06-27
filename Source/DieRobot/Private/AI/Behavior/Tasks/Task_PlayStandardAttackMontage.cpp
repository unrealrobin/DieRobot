// Property of Paracosm.


#include "AI/Behavior/Tasks/Task_PlayStandardAttackMontage.h"

#include "AI/TimberAiControllerBase.h"
#include "Character/Enemies/TimberEnemyCharacter.h"

UTask_PlayStandardAttackMontage::UTask_PlayStandardAttackMontage()
{
	NodeName = "Play Standard Attack Montage";
	bCreateNodeInstance = true;
	bNotifyTick = false;        
	bNotifyTaskFinished = false;
}

EBTNodeResult::Type UTask_PlayStandardAttackMontage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	OwnerBehaviorTreeComp = &OwnerComp;
	if (!OwnerBehaviorTreeComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Task_StandardAttackMontage - Could not get BT Component."));
		return EBTNodeResult::Failed;
	}
	
	BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Task_StandardAttackMontage - Could not get Blackboard Component."));
		return EBTNodeResult::Failed;
	}

	ATimberAiControllerBase* AIControllerBase = Cast<ATimberAiControllerBase>(OwnerComp.GetAIOwner());
	if (!AIControllerBase)
	{
		UE_LOG(LogTemp, Warning, TEXT("Task_StandardAttackMontage - Could not Cast to AI Controller."));
		return EBTNodeResult::Failed;
	}

	ATimberEnemyCharacter* AICharacter = Cast<ATimberEnemyCharacter>(AIControllerBase->GetPawn());
	if (!AICharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Task_StandardAttackMontage - Could not Cast to AI Character."));
		return EBTNodeResult::Failed;
	}
	
	UAnimMontage* StandardAttackMontage = AICharacter->StandardAttackMontage;
	if (!StandardAttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("Task_StandardAttackMontage - Could not access Standard Attack Montage."));
		return EBTNodeResult::Failed;
	}

	//Play Montage at a Named Section
	if (!bPlayRandomSection && SectionName != NAME_None)
	{
		//AICharacter->PlayAnimMontage(StandardAttackMontage, 1.0, SectionName);

		FName SectionNameCopy = SectionName;
		AsyncTask(ENamedThreads::GameThread, [this, AICharacter, SectionNameCopy, StandardAttackMontage]()
		{
			if (!IsValid(AICharacter) || !StandardAttackMontage) return;
			UAnimInstance* AnimInstance = AICharacter->GetMesh()->GetAnimInstance();
			if (!AnimInstance) return;
			AnimInstance->Montage_Play(StandardAttackMontage, 1.0);
			AnimInstance->Montage_JumpToSection(SectionNameCopy, StandardAttackMontage);
			AnimInstance->OnMontageBlendingOut.RemoveDynamic(this, &UTask_PlayStandardAttackMontage::OnMontageBlendOut);
			AnimInstance->OnMontageBlendingOut.AddDynamic(this, &UTask_PlayStandardAttackMontage::OnMontageBlendOut);
		});

		return EBTNodeResult::InProgress;
	}

	if (bPlayRandomSection) //Play At Random Section
	{
		int NumberOfSections = StandardAttackMontage->GetNumSections();
		int RandSection = FMath::RandRange(0, NumberOfSections - 1);
		

		AsyncTask(ENamedThreads::GameThread, [this, AICharacter, StandardAttackMontage, RandSection]()
		{
			if (!IsValid(AICharacter) || !StandardAttackMontage) return;
			UAnimInstance* AnimInstance = AICharacter->GetMesh()->GetAnimInstance();
			if (!AnimInstance) return;

			FName RandSectionName = StandardAttackMontage->GetSectionName(RandSection);
			AnimInstance->Montage_Play(StandardAttackMontage, 1.0);
			AnimInstance->Montage_JumpToSection(RandSectionName, StandardAttackMontage);
			AnimInstance->OnMontageBlendingOut.RemoveDynamic(this, &UTask_PlayStandardAttackMontage::OnMontageBlendOut);
			AnimInstance->OnMontageBlendingOut.AddDynamic(this, &UTask_PlayStandardAttackMontage::OnMontageBlendOut);
		});

		return EBTNodeResult::InProgress;
	}

	
	return EBTNodeResult::Failed;
}

void UTask_PlayStandardAttackMontage::CleanUpDelegateBindings()
{
	if (OwnerBehaviorTreeComp)
	{
		ATimberAiControllerBase* AIControllerBase = Cast<ATimberAiControllerBase>(OwnerBehaviorTreeComp->GetAIOwner());
		if (AIControllerBase)
		{
			ATimberEnemyCharacter* AICharacter = Cast<ATimberEnemyCharacter>(AIControllerBase->GetPawn());
			if (AICharacter)
			{
				UAnimInstance* AnimInstance = AICharacter->GetMesh()->GetAnimInstance();
				if (AnimInstance)
				{
					AnimInstance->OnMontageBlendingOut.RemoveDynamic(this, &UTask_PlayStandardAttackMontage::OnMontageBlendOut);
				}
			}
		}
	}

}

EBTNodeResult::Type UTask_PlayStandardAttackMontage::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CleanUpDelegateBindings();
	return EBTNodeResult::Aborted;
}

void UTask_PlayStandardAttackMontage::OnTaskFinished(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	CleanUpDelegateBindings();
	UE_LOG(LogTemp, Warning, TEXT("Task_StandardAttackMontage - Task Finished Cleanly."));
}

void UTask_PlayStandardAttackMontage::OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		UE_LOG(LogTemp, Warning, TEXT("Task_StandardAttackMontage - Montage Blended Out - Interrupted."));
		FinishLatentTask(*OwnerBehaviorTreeComp, EBTNodeResult::Aborted);
		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Task_StandardAttackMontage - Montage Blended Out - Finished."));
		FinishLatentTask(*OwnerBehaviorTreeComp, EBTNodeResult::Succeeded);
	}
}
