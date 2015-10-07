#include "CreatureEditorPCH.h"
#include "CreatureAnimStateNode.h"
#include "SGraphNode.h"
#include "CreatureAnimTransitionNode.h"
#include "CreatureAnimStateMachine.h"

void UCreatureAnimStateNode::OnRenameNode(const FString& NewName)
{
	AnimName = NewName;
}

bool UCreatureAnimStateNode::CanUserDeleteNode() const
{
	return true;
}

FText UCreatureAnimStateNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(AnimName);
}
#ifdef WITH_EDITOR
void UCreatureAnimStateNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet&&GetGraph()!=nullptr)
	{
		GetGraph()->NotifyGraphChanged();
	}
	

}
#endif


void UCreatureAnimStateNode::Compile()
{
	CompiledState->AnimStateName = AnimName;

	CompiledState->TransitionList.Empty();
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->Direction == EEdGraphPinDirection::EGPD_Output)
		{
			for (UEdGraphPin* TargetPin : Pin->LinkedTo)
			{
				//将变换信息填充入编译后的State
				if (UCreatureAnimTransitionNode* TargetNode = Cast<UCreatureAnimTransitionNode>(TargetPin->GetOwningNode()))
				{
					UCreatureAnimTransition* Tran = NewObject<UCreatureAnimTransition>(CompiledState->GetOuter());
					FCreatureTransitionCondition TranCondition = FCreatureTransitionCondition(TargetNode->TransitionCondition, TargetNode->TransitionFlag);
					Tran->TargetState = TargetNode->TransitionTargetNode->CompiledState;
					INT16 Index= Tran->TransitionConditions.AddUnique(TranCondition);
					//再次填充一遍变换Condition避免出现修改了TranCondition但是无效的情况
					Tran->TransitionConditions[Index].TransitionFlag = TranCondition.TransitionFlag;
					Tran->AnimStateMachine = CompiledState->AnimStateMachine;

					//向状态机注册当前状态转换信息
					Tran->AnimStateMachine->TransitionConditionList.AddUnique(TranCondition);
					CompiledState->TransitionList.Add(Tran);
				}
				

			}
		}

	}
	//如果是根节点,通知状态机
	if (AnimName==FString(TEXT("Default")))
	{
		CompiledState->AnimStateMachine->RootState = CompiledState;
	}
}

void UCreatureAnimStateNode::InitNode(class UCreatureAnimStateMachine* stateMachine)
{
	CompiledState = NewObject<UCreatureAnimState>(stateMachine->GetOuter());
	CompiledState->AnimStateMachine = stateMachine;
}

FLinearColor UCreatureAnimStateNode::GetNodeTitleColor() const
{
	if (CompiledState==nullptr)
	{
		return FLinearColor::Gray;
	}
	if (CompiledState->bIsCurrentState)
	{
		return FLinearColor::Yellow;
	}
	else
		return FLinearColor::Gray;
}




