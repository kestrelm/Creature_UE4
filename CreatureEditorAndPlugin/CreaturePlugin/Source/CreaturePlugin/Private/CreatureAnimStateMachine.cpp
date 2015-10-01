#include "CustomProceduralMesh.h"
#include "CreatureAnimStateMachine.h"
#include "CreatureAnimTransition.h"
#include "EdGraph/EdGraphSchema.h"
UCreatureAnimStateMachine::UCreatureAnimStateMachine(){
	//StateMachineGraph = NewObject<UCreatureStateMachineGraph>(UCreatureStateMachineGraph::StaticClass());

}

void UCreatureAnimStateMachine::SetCondition(FString ConditionName, bool Flag)
{
	//更新状态

	for (int i = 0; i < TransitionConditionList.Num();i++)
	{
		if (TransitionConditionList[i].TransitionName == ConditionName)
		{
			TransitionConditionList[i].TransitionFlag = Flag;
		}
	}
	//判断是否需要转换
	CurrentState->CheckCondition();
}

void UCreatureAnimStateMachine::InitStateMachine()
{
	if (CurrentState != nullptr&&CurrentState != RootState->TransitionList[0]->TargetState)
	{
		CurrentState->bIsCurrentState = false;//重置初始节点状态，用于Debug
	}
	//临时使用，直接从根节点跳到第一个节点
	CurrentState = RootState->TransitionList[0]->TargetState;
	CurrentState->bIsCurrentState = true;
	CurrentState->BeginState();
}

