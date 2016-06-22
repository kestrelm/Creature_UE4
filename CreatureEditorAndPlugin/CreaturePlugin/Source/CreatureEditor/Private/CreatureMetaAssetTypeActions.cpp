#include "CreatureEditorPCH.h"
#include "CreatureMetaAssetTypeActions.h"
#include "CreatureMetaAsset.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FCreatureMetaAssetTypeActions::FCreatureMetaAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
	:MyAssetCategory(InAssetCategory)
{

}

FText FCreatureMetaAssetTypeActions::GetName() const
{
	return LOCTEXT("CreatureMetaAssetName", "Creature Meta Asset");
}

UClass* FCreatureMetaAssetTypeActions::GetSupportedClass() const
{
	return UCreatureMetaAsset::StaticClass();
}

FColor FCreatureMetaAssetTypeActions::GetTypeColor() const
{
	return FColorList::BrightGold;
}

void FCreatureMetaAssetTypeActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
    /*
	for (UObject *obj : TypeAssets)
	{
		UCreatureMetaAsset *metaAsset = Cast<UCreatureMetaAsset>(obj);

		if (animAsset != nullptr)
		{
			const FString &filename = animAsset->GetCreatureFilename();
			if (!filename.IsEmpty())
			{
				OutSourceFilePaths.Add(filename);
			}
		}
	}
    */
}

uint32 FCreatureMetaAssetTypeActions::GetCategories()
{
	return MyAssetCategory;
}

#undef LOCTEXT_NAMESPACE

