#include "CreatureParticlesAssetTypeActions.h"
#include "CreatureParticlesAsset.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FCreatureParticlesAssetTypeActions::FCreatureParticlesAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
	:MyAssetCategory(InAssetCategory)
{

}

FText FCreatureParticlesAssetTypeActions::GetName() const
{
	return LOCTEXT("CreatureParticlesAssetName", "Creature Particles Asset");
}

UClass* FCreatureParticlesAssetTypeActions::GetSupportedClass() const
{
	return UCreatureMetaAsset::StaticClass();
}

FColor FCreatureParticlesAssetTypeActions::GetTypeColor() const
{
	return FColorList::BrightGold;
}

void FCreatureParticlesAssetTypeActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{

}

uint32 FCreatureParticlesAssetTypeActions::GetCategories()
{
	return MyAssetCategory;
}

#undef LOCTEXT_NAMESPACE
