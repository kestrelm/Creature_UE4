#include "CreaturePackEditorPCH.h"
#include "CreaturePackEditor.h"
#include "CreaturePackAnimationAssetTypeActions.h"
#define LOCTEXT_NAMESPACE "CreaturePackEditor"
void CreaturePackEditor::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	//Custom detail views

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	EAssetTypeCategories::Type CreaturePackAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("CreatureAssetCategory")), LOCTEXT("CreatureAssetCategory", "Creature"));

	AssetTools.RegisterAssetTypeActions(MakeShareable(new FCreaturePackAnimationAssetTypeActions(CreaturePackAssetCategoryBit)));
}

void CreaturePackEditor::ShutdownModule()
{
}

IMPLEMENT_MODULE(CreaturePackEditor, CreaturePackEditor)

#undef LOCTEXT_NAMESPACE
