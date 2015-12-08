#include "CustomProceduralMesh.h"
#include "CreatureAnimState.h"
#include "CreatureAnimStateMachine.h"
#include "CreatureAnimTransition.h"
void UCreatureAnimState::BeginState()
{
	if (AnimStateMachine->OwningComponent->enable_collection_playback)
	{
		AnimStateMachine->OwningComponent->SetBluePrintActiveCollectionClip(AnimStateName);
	}
	else
	{
		static const float blendFactor = 0.1f;	// TODO: consider making a tunable UPROPERTY
		AnimStateMachine->OwningComponent->SetBluePrintBlendActiveAnimation(AnimStateName, blendFactor);
	}
}

void UCreatureAnimState::EndState()
{

}

void UCreatureAnimState::CheckCondition()
{
	for (UCreatureAnimTransition* Tran:TransitionList)
	{

		if (Tran->Translate()==true)
		{
			return;
		}
	}
}

void UCreatureAnimState::AnimationEnd()
{
	for (UCreatureAnimTransition* Tran : TransitionList)
	{
		if (Tran->TransitionConditions[0].TransitionName==FString(TEXT("AnimationEnd")))
		{
			//当前有一个转换的名称为AnimationEnd
			Tran->AnimationEndTranslate();
		}
	}
}
