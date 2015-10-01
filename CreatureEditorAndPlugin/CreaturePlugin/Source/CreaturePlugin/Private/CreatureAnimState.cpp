#include "CustomProceduralMesh.h"
#include "CreatureAnimState.h"
#include "CreatureAnimStateMachine.h"
#include "CreatureAnimTransition.h"
void UCreatureAnimState::BeginState()
{
	//AnimStateMachine->OwningComponent->SetBluePrintBlendActiveAnimation(AnimStateName, 0.1f);
	AnimStateMachine->OwningComponent->SetBluePrintActiveCollectionClip(AnimStateName);
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
