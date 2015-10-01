#include "CreatureEditorPCH.h"
#include "CreatureStateMachineGraph.h"
#include "EdGraph/EdGraphSchema.h"
#include "CreatureAnimStateNode.h"
#include "CreatureAnimGraphSchema.h"
UCreatureStateMachineGraph::UCreatureStateMachineGraph(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Schema = UCreatureAnimGraphSchema::StaticClass();

}

void UCreatureStateMachineGraph::Serialize(FArchive& Ar)
{

	Super::Serialize(Ar);
}

void UCreatureStateMachineGraph::CompileNodes()
{
	//必然有根节点，否则不科学
	if (DefaultRootNode == nullptr)
	{
		return;
	}
	
	GetNodesOfClass<UCreatureAnimStateNode>(StateNodeList);
	for (auto Node : StateNodeList)
	{
		Node->Compile();
	}

}

void UCreatureStateMachineGraph::CreateDefaultStateNode()
{
	if (DefaultRootNode == nullptr)
	{
		UCreatureAnimStateNode* NewStateNode = NewObject<UCreatureAnimStateNode>(this);
		NewStateNode->InitNode(ParentStateMachine);
		NewStateNode->NodePosX = 0;
		NewStateNode->NodePosY = 0;
		NewStateNode->InputPins.Add(NewStateNode->CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinType(), TEXT("Output")));
		NewStateNode->AnimName = FString(TEXT("Default"));
		DefaultRootNode = NewStateNode;
		AddNode(NewStateNode, true, false);
	}
}
