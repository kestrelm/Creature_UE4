
#include "CreatureAnimStateMachineInstance.h"
#include "CreaturePluginPCH.h"
#include "CreatureAnimState.h"
#include "CreatureAnimStateMachine.h"
#include "CreatureAnimTransition.h"

UCreatureAnimStateMachineInstance::UCreatureAnimStateMachineInstance()
{

}

void UCreatureAnimStateMachineInstance::SetCondition(FString ConditionName, bool Flag)
{
	FName conditionAsName(*ConditionName);
	SetConditionByName(conditionAsName, Flag);
}

bool UCreatureAnimStateMachineInstance::GetCondition(FString ConditionName) const
{
	FName conditionAsName(*ConditionName);
	return GetConditionByName(conditionAsName);
}

bool UCreatureAnimStateMachineInstance::GetConditionByName(FName conditionName) const
{
	const bool *value = m_currentConditionValues.Find(conditionName);

	return (value) ? *value : false;
}

void UCreatureAnimStateMachineInstance::SetConditionByName(FName conditionName, bool Flag)
{
	if (GetConditionByName(conditionName) == Flag)
	{
		// it's already set: skip
		return;
	}

	m_currentConditionValues.Add(conditionName, Flag);

	if (CurrentState)
	{
		CurrentState->CheckCondition(this);
	}
}

void UCreatureAnimStateMachineInstance::SetCurrentState(UCreatureAnimState *state)
{
	if (CurrentState)
	{
		CurrentState->EndState(this);
	}

	CurrentState = state;

	if (CurrentState)
	{
		CurrentState->BeginState(this);
	}
}

UCreatureMeshComponent * UCreatureAnimStateMachineInstance::GetOwningMeshComponent() const
{
	return Cast<UCreatureMeshComponent>(GetOuter());
}

void UCreatureAnimStateMachineInstance::InitInstance(UCreatureAnimStateMachine* forStateMachine)
{
	check(forStateMachine);
	TargetStateMachine = forStateMachine;

	//绑定动画开始与结尾事件到MeshComponent用于支持AnimStart/AnimEnd转换
	UCreatureMeshComponent *owningComponent = GetOwningMeshComponent();
	if (owningComponent !=nullptr)
	{
		owningComponent->CreatureAnimationStartEvent.AddDynamic(this, &UCreatureAnimStateMachineInstance::OnAnimStart);
		owningComponent->CreatureAnimationEndEvent.AddDynamic(this, &UCreatureAnimStateMachineInstance::OnAnimEnd);
	}

	//临时使用，直接从根节点跳到第一个节点
	SetCurrentState(forStateMachine->RootState->TransitionList[0]->TargetState);
}

void UCreatureAnimStateMachineInstance::OnAnimStart(float frame)
{
	//SetCondition(FString(TEXT("AnimationStart")),true);
	//SetCondition(FString(TEXT("AnimationEnd")), false);
}

void UCreatureAnimStateMachineInstance::OnAnimEnd(float frame)
{
	//SetCondition(FString(TEXT("AnimationStart")), false);
	//SetCondition(FString(TEXT("AnimationEnd")), true);
	CurrentState->AnimationEnd(this);
}
