#include "CustomProceduralMesh.h"
#include "CreatureAnimTransition.h"
#include "CreatureAnimStateMachine.h"
bool UCreatureAnimTransition::Translate()
{
	for (FCreatureTransitionCondition condition : TransitionConditions)
	{
		int32 index;
		if (AnimStateMachine->TransitionConditionList.Find(condition, index) == true){
			if (AnimStateMachine->TransitionConditionList [index].TransitionFlag!= condition.TransitionFlag)
			{
				return false;
			}else
				continue;
		}
		
	}
	//所有的条件都匹配，可以跳转
	AnimStateMachine->CurrentState->bIsCurrentState = false;
	AnimStateMachine->CurrentState = TargetState;
	//有可能会多次跳转，要检查
	/*TargetState->CheckCondition();*/
	AnimStateMachine->CurrentState->BeginState();
	AnimStateMachine->CurrentState->bIsCurrentState = true;
	return true;
}
