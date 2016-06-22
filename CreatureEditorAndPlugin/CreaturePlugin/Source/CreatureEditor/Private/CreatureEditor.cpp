#include "CreatureEditorPCH.h"
#include "CreatureEditor.h"
#include "CreatureAnimStateMachineDetails.h"
#include "CreatureAnimStoreAssetTypeActions.h"
#include "CreatureAnimStateMachineAssetTypeActions.h"
#include "CreatureAnimationAssetTypeActions.h"
#include "CreatureMetaAssetTypeActions.h"
#define LOCTEXT_NAMESPACE "CreatureEditor"
void CreatureEditor::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	//Custom detail views
	PropertyModule.RegisterCustomClassLayout("CreatureAnimStateMachine", FOnGetDetailCustomizationInstance::CreateStatic(&FCreatureAnimStateMachineDetails::MakeInstance));

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	EAssetTypeCategories::Type CreatureAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("CreatureAssetCategory")), LOCTEXT("CreatureAssetCategory", "Creature"));

	AssetTools.RegisterAssetTypeActions(MakeShareable(new FCreatureAnimStateMachineAssetTypeActions(CreatureAssetCategoryBit)));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FCreatureAnimStoreAssetTypeActions(CreatureAssetCategoryBit)));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FCreatureAnimationAssetTypeActions(CreatureAssetCategoryBit)));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FCreatureMetaAssetTypeActions(CreatureAssetCategoryBit)));
}

void CreatureEditor::ShutdownModule()
{
}

IMPLEMENT_MODULE(CreatureEditor, CreatureEditor)

#undef LOCTEXT_NAMESPACE
