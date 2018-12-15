#include "CreatureAnimStoreAssetTypeActions.h"
#include "CreatureAnimationClipsStore.h"
#include "CreatureAnimStoreEditor.h"
#define LOCTEXT_NAMESPACE "AssetTypeActions"
FCreatureAnimStoreAssetTypeActions::FCreatureAnimStoreAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
	:MyAssetCategory(InAssetCategory)
{

}
FText FCreatureAnimStoreAssetTypeActions::GetName() const
{
	return LOCTEXT("FCreatureAnimStoreActionsName", "Creature Animation Clips Store");
}
UClass* FCreatureAnimStoreAssetTypeActions::GetSupportedClass() const
{
	return UCreatureAnimationClipsStore::StaticClass();
}
void FCreatureAnimStoreAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor){
	FAssetTypeActions_Base::OpenAssetEditor(InObjects, EditWithinLevelEditor);
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UCreatureAnimationClipsStore* Store = Cast<UCreatureAnimationClipsStore>(*ObjIt))
		{
			TSharedRef<FCreatureAnimStoreEditor> NewStoreEditor(new FCreatureAnimStoreEditor());
			NewStoreEditor->InitAnimStoreEditor(Mode, EditWithinLevelEditor, Store);
		}
	}

}
FColor FCreatureAnimStoreAssetTypeActions::GetTypeColor() const
{
	return FColorList::BrightGold;
}
uint32 FCreatureAnimStoreAssetTypeActions::GetCategories()
{
	return MyAssetCategory;
}
#undef LOCTEXT_NAMESPACE

