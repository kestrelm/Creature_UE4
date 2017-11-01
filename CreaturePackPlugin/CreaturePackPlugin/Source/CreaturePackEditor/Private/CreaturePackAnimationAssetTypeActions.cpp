#include "CreaturePackAnimationAssetTypeActions.h"
#include "CreaturePackAnimationAsset.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FCreaturePackAnimationAssetTypeActions::FCreaturePackAnimationAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
	:MyAssetCategory(InAssetCategory)
{

}

FText FCreaturePackAnimationAssetTypeActions::GetName() const
{
	return LOCTEXT("CreaturePackAnimAssetName", "Creature Pack Animation Asset");
}

UClass* FCreaturePackAnimationAssetTypeActions::GetSupportedClass() const
{
	return UCreaturePackAnimationAsset::StaticClass();
}

FColor FCreaturePackAnimationAssetTypeActions::GetTypeColor() const
{
	return FColorList::BrightGold;
}

void FCreaturePackAnimationAssetTypeActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (UObject *obj : TypeAssets)
	{
		UCreaturePackAnimationAsset *animAsset = Cast<UCreaturePackAnimationAsset>(obj);

		if (animAsset != nullptr)
		{
			const FString &filename = animAsset->GetCreatureFilename();
			if (!filename.IsEmpty())
			{
				OutSourceFilePaths.Add(filename);
			}
		}
	}
}

uint32 FCreaturePackAnimationAssetTypeActions::GetCategories()
{
	return MyAssetCategory;
}

#undef LOCTEXT_NAMESPACE

