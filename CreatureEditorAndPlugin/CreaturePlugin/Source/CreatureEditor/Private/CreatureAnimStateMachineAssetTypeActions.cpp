#include "CreatureEditorPCH.h"
#include "CreatureAnimStateMachineAssetTypeActions.h"
#include "CreatureAnimStateMachine.h"
#include "CreatureAnimStateMachineEditor.h"
#define LOCTEXT_NAMESPACE "AssetTypeActions"
FCreatureAnimStateMachineAssetTypeActions ::FCreatureAnimStateMachineAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
	:MyAssetCategory(InAssetCategory)
{

}
FText FCreatureAnimStateMachineAssetTypeActions::GetName() const
{
	return LOCTEXT("FCreatureAnimStateMachineAssetTypeActionsName", "Creature State Machine");
}
UClass* FCreatureAnimStateMachineAssetTypeActions::GetSupportedClass() const
{
	return UCreatureAnimStateMachine::StaticClass();
}
void FCreatureAnimStateMachineAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor){
	FAssetTypeActions_Base::OpenAssetEditor(InObjects, EditWithinLevelEditor);
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UCreatureAnimStateMachine* StateMachine = Cast<UCreatureAnimStateMachine>(*ObjIt))
		{
			TSharedRef<FCreatureAnimStateMachineEditor> NewStateMachineEditor(new FCreatureAnimStateMachineEditor());
			NewStateMachineEditor->InitAnimStateMachineEditor(Mode, EditWithinLevelEditor, StateMachine);
		}
	}

}
FColor FCreatureAnimStateMachineAssetTypeActions :: GetTypeColor() const
{
	return FColorList::BrightGold;
}
uint32 FCreatureAnimStateMachineAssetTypeActions::GetCategories()
{
	return MyAssetCategory;
}
#undef LOCTEXT_NAMESPACE
