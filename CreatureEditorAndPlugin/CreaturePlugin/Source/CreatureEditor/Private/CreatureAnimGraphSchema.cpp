#include "CreatureEditorPCH.h"
#include "CreatureAnimGraphSchema.h"
#include "EdGraph/EdGraph.h"
#include "CreatureAnimStateNode.h"
#include "GenericCommands.h"
#include "CreatureAnimTransitionNode.h"
#include "CreatureStateMachineGraph.h"
#define LOCTEXT_NAMESPACE "CreatureStateMachineSchema"
TSharedPtr<FEdGraphSchemaAction_NewCreatureStateNode> AddNewStateNodeAction(FGraphContextMenuBuilder& ContextMenuBuilder, const FText& Category, const FText& MenuDesc, const FText& Tooltip, const int32 Grouping = 0)
{
	TSharedPtr<FEdGraphSchemaAction_NewCreatureStateNode> NewStateNode(new FEdGraphSchemaAction_NewCreatureStateNode(Category, MenuDesc, Tooltip, Grouping));
	ContextMenuBuilder.AddAction(NewStateNode);
	return NewStateNode;
}
void UCreatureAnimGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	// Create the entry/exit tunnels

}

const FPinConnectionResponse UCreatureAnimGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

bool UCreatureAnimGraphSchema::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const
{
	if (A==B)
	{
		return false;
	}
	else
	{
		if (Cast<UCreatureAnimStateNode>(A->GetOwningNode()) != nullptr&&Cast<UCreatureAnimStateNode>(B->GetOwningNode()))
		{
			UCreatureAnimTransitionNode* TransitionNode = NewObject<UCreatureAnimTransitionNode>(A->GetOwningNode()->GetGraph());
			TransitionNode->NodePosX = (A->GetOwningNode()->NodePosX + B->GetOwningNode()->NodePosX) / 2;
			TransitionNode->NodePosY = (A->GetOwningNode()->NodePosY + B->GetOwningNode()->NodePosY) / 2;
			if (UCreatureStateMachineGraph* Graph = Cast<UCreatureStateMachineGraph>(TransitionNode->GetGraph()))
			{
				if (TransitionNode->CompiledTransition != nullptr)
				{
					TransitionNode->CompiledTransition = NewObject<UCreatureAnimTransition>(Graph->GetOuter());
					TransitionNode->CompiledTransition->AnimStateMachine = Graph->ParentStateMachine;
				}
			}
			A->MakeLinkTo(TransitionNode->CreatePin(EEdGraphPinDirection::EGPD_Input, A->PinType, FString("In")));
			TransitionNode->CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinType(), FString("Out"))->MakeLinkTo(B);
			A->GetOwningNode()->GetGraph()->AddNode(TransitionNode);
			TransitionNode->TransitionTargetNode = Cast<UCreatureAnimStateNode>(B->GetOwningNode());
		}
		//Transition-->State
		if (Cast<UCreatureAnimTransitionNode>(A->GetOwningNode())!=nullptr&&Cast<UCreatureAnimStateNode>(B->GetOwningNode()))
		{
			A->MakeLinkTo(B);
		}
		//State-->Transition
		if (Cast<UCreatureAnimStateNode>(A->GetOwningNode()) != nullptr&&Cast<UCreatureAnimTransitionNode>(B->GetOwningNode()))
		{
			A->MakeLinkTo(B);
		}
	}
	
	return true;
}

bool UCreatureAnimGraphSchema::CreateAutomaticConversionNodeAndConnections(UEdGraphPin* PinA, UEdGraphPin* PinB) const
{
	return true;
}

void UCreatureAnimGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	{
		TSharedPtr<FEdGraphSchemaAction_NewCreatureStateNode> Action = AddNewStateNodeAction(ContextMenuBuilder, FText::GetEmpty(), LOCTEXT("AddState", "Add State..."), LOCTEXT("A new state", "A new state"));
		//Action->NodeTemplate = NewObject<UCreatureAnimStateNode>(ContextMenuBuilder.OwnerOfTemporaries);
	}

	// Add Animation End Transition Node
	{
		TSharedPtr<FEdGraphSchemaAction_NewCreatureAnimationEndTransition> NewAnimationEndNode(new FEdGraphSchemaAction_NewCreatureAnimationEndTransition(FText::GetEmpty(), LOCTEXT("AddAnimEndTransition", "Add Animation End Tran..."), LOCTEXT("Add a Animation End Transition Node", "Add a Animation End Transition Node"),0));
		ContextMenuBuilder.AddAction(NewAnimationEndNode);

	}
}

EGraphType UCreatureAnimGraphSchema::GetGraphType(const UEdGraph* TestEdGraph) const
{
	return GT_StateMachine;
}

void UCreatureAnimGraphSchema::GetContextMenuActions(const UEdGraph* CurrentGraph, const UEdGraphNode* InGraphNode, const UEdGraphPin* InGraphPin, FMenuBuilder* MenuBuilder, bool bIsDebugging) const
{
	
	MenuBuilder->AddMenuEntry(FGenericCommands::Get().Delete);
	MenuBuilder->AddMenuEntry(FGenericCommands::Get().Rename);
	
	Super::GetContextMenuActions(CurrentGraph, InGraphNode, InGraphPin, MenuBuilder, bIsDebugging);
}

FLinearColor UCreatureAnimGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FLinearColor::White;
}

void UCreatureAnimGraphSchema::GetGraphDisplayInformation(const UEdGraph& Graph, /*out*/ FGraphDisplayInfo& DisplayInfo) const
{
	DisplayInfo.DisplayName =LOCTEXT("State Name","Creature Anim Stat Node");
	DisplayInfo.Tooltip = LOCTEXT("Use to Play an Animation", "Use to Play an Animation");
}

void UCreatureAnimGraphSchema::DroppedAssetsOnGraph(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraph* Graph) const
{

}

void UCreatureAnimGraphSchema::DroppedAssetsOnNode(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraphNode* Node) const
{

}

void UCreatureAnimGraphSchema::DroppedAssetsOnPin(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraphPin* Pin) const
{

}

void UCreatureAnimGraphSchema::GetAssetsNodeHoverMessage(const TArray<FAssetData>& Assets, const UEdGraphNode* HoverNode, FString& OutTooltipText, bool& OutOkIcon) const
{

}

void UCreatureAnimGraphSchema::GetAssetsPinHoverMessage(const TArray<FAssetData>& Assets, const UEdGraphPin* HoverPin, FString& OutTooltipText, bool& OutOkIcon) const
{

}



#undef LOCTEXT_NAMESPACE

UEdGraphNode* FEdGraphSchemaAction_NewCreatureStateNode::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	UCreatureAnimStateNode* NewStateNode = NewObject<UCreatureAnimStateNode>(ParentGraph);
	NewStateNode->NodePosX = Location.X;
	NewStateNode->NodePosY = Location.Y;
	NewStateNode->InputPins.Add(NewStateNode->CreatePin(EEdGraphPinDirection::EGPD_Input, FEdGraphPinType(), TEXT("Input")));
	NewStateNode->InputPins.Add(NewStateNode->CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinType(), TEXT("Output")));
	if (UCreatureStateMachineGraph* Graph=Cast<UCreatureStateMachineGraph>(NewStateNode->GetGraph()))
	{
		if (NewStateNode->CompiledState==nullptr)
		{
			NewStateNode->CompiledState = NewObject<UCreatureAnimState>(ParentGraph->GetOuter());
			NewStateNode->CompiledState->AnimStateMachine = Graph->ParentStateMachine;
		}	
	}
	ParentGraph->AddNode(NewStateNode, true, false);

	return NewStateNode;
}

UEdGraphNode* FEdGraphSchemaAction_NewCreatureAnimationEndTransition::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	UCreatureAnimTransitionNode* NewAnimationEndTransitionNode = NewObject<UCreatureAnimTransitionNode>(ParentGraph);
	NewAnimationEndTransitionNode->NodePosX		= Location.X;
	NewAnimationEndTransitionNode->NodePosY		= Location.Y;
	NewAnimationEndTransitionNode->InputPin		= NewAnimationEndTransitionNode->CreatePin(EEdGraphPinDirection::EGPD_Input, FEdGraphPinType(), TEXT("Input"));
	NewAnimationEndTransitionNode->OutputPin	= NewAnimationEndTransitionNode->CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinType(), TEXT("Output"));
	NewAnimationEndTransitionNode->TransitionCondition = FName(TEXT("AnimationEnd"));
	NewAnimationEndTransitionNode->TransitionFlag = true;
	ParentGraph->AddNode(NewAnimationEndTransitionNode, true, false);

	return NewAnimationEndTransitionNode;
}
