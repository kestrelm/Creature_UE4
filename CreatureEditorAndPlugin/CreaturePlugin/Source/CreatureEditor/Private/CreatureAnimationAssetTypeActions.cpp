#include "CreatureEditorPCH.h"
#include "CreatureAnimationAssetTypeActions.h"
#include "CreatureAnimationAsset.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FCreatureAnimationAssetTypeActions::FCreatureAnimationAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
	:MyAssetCategory(InAssetCategory)
{

}

FText FCreatureAnimationAssetTypeActions::GetName() const
{
	return LOCTEXT("CreatureAnimAssetName", "Creature Animation Asset");
}

UClass* FCreatureAnimationAssetTypeActions::GetSupportedClass() const
{
	return UCreatureAnimationAsset::StaticClass();
}

FColor FCreatureAnimationAssetTypeActions::GetTypeColor() const
{
	return FColorList::BrightGold;
}

void FCreatureAnimationAssetTypeActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (UObject *obj : TypeAssets)
	{
		UCreatureAnimationAsset *animAsset = Cast<UCreatureAnimationAsset>(obj);

		if (animAsset != nullptr)
		{
			const FName &filename = animAsset->GetCreatureFilename();
			if (!filename.IsNone())
			{
				OutSourceFilePaths.Add(filename.ToString());
			}
		}
	}
}

uint32 FCreatureAnimationAssetTypeActions::GetCategories()
{
	return MyAssetCategory;
}

#undef LOCTEXT_NAMESPACE

