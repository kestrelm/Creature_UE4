#include "CreatureEditorPCH.h"
#include "CreatureAnimTransitionNode.h"
#include "CreatureAnimTransition.h"
#include "CreatureStateMachineGraph.h"
#include "EdGraph/EdGraph.h"
FText UCreatureAnimTransitionNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromName(TransitionCondition);
}

FLinearColor UCreatureAnimTransitionNode::GetNodeTitleColor() const
{
	return TransitionFlag ? FLinearColor::Green : FLinearColor::Red;
}
#ifdef WITH_EDITOR
void  UCreatureAnimTransitionNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet&&GetGraph() != nullptr)
	{
		GetGraph()->NotifyGraphChanged();
	}


}

UCreatureAnimTransitionNode::UCreatureAnimTransitionNode()
	:Super()
{
	TransitionCondition = FName(TEXT("DefaultTransition"));
	TransitionFlag = false;
	NodeWidth = 20;

}

#endif

void SCreatureGraphNodeAnimState::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);
}
