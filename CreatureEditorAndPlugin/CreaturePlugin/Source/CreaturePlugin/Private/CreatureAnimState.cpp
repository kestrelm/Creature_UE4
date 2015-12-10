
#include "CreaturePluginPCH.h"
#include "CreatureAnimState.h"
#include "CreatureAnimStateMachine.h"
#include "CreatureAnimTransition.h"
#include "CreatureAnimStateMachineInstance.h"

void UCreatureAnimState::BeginState(UCreatureAnimStateMachineInstance *forInstance)
{
	check(forInstance);
	UCreatureMeshComponent *targetMeshComp = forInstance->GetOwningMeshComponent();
	check(targetMeshComp);
	if (targetMeshComp->enable_collection_playback)
	{
		targetMeshComp->SetBluePrintActiveCollectionClip(AnimStateName);
	}
	else
	{
		static const float blendFactor = 0.1f;	// TODO: consider making a tunable UPROPERTY
		targetMeshComp->SetBluePrintBlendActiveAnimation(AnimStateName, blendFactor);
	}
}

void UCreatureAnimState::EndState(UCreatureAnimStateMachineInstance *forInstance)
{

}

void UCreatureAnimState::CheckCondition(UCreatureAnimStateMachineInstance *forInstance)
{
	for (UCreatureAnimTransition* Tran:TransitionList)
	{

		if (Tran->Translate(forInstance)==true)
		{
			return;
		}
	}
}

void UCreatureAnimState::AnimationEnd(UCreatureAnimStateMachineInstance *forInstance)
{
	for (UCreatureAnimTransition* Tran : TransitionList)
	{
		if (Tran->TransitionConditions[0].TransitionName==FString(TEXT("AnimationEnd")))
		{
			//当前有一个转换的名称为AnimationEnd
			Tran->AnimationEndTranslate(forInstance);
		}
	}
}
