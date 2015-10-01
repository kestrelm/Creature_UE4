#include "CreatureEditorPCH.h"
#include "CreatureAnimStateMachineAssetFactory.h"
#include "CreatureStateMachineGraph.h"
#define LOCTEXT_NAMESPACE "CreatureAnimStateMachineAssetFactory"
UCreatureAnimStateMachineAssetFactory::UCreatureAnimStateMachineAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UCreatureAnimStateMachine::StaticClass();
}

UObject* UCreatureAnimStateMachineAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UCreatureAnimStateMachine * StateMachineAsset = NewObject<UCreatureAnimStateMachine>(InParent, Class, Name, Flags);
	
		UCreatureStateMachineGraph* graph = NewObject<UCreatureStateMachineGraph>(StateMachineAsset->GetOuter(), FName(TEXT("Graph")));
		graph->ParentStateMachine = StateMachineAsset;
		graph->CreateDefaultStateNode();
		StateMachineAsset->StateMachineGraph =graph ;
		
	return StateMachineAsset;
}
#undef LOCTEXT_NAMESPACE