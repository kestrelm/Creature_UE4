
#include "CreaturePackRuntimePluginPCH.h"
#include "CreaturePackAnimationAsset.h"

FString UCreaturePackAnimationAsset::GetCreatureFilename() const
{
#if WITH_EDITORONLY_DATA
	TArray<FString> filenames;
	if (AssetImportData)
	{
		AssetImportData->ExtractFilenames(filenames);
	}
	if (filenames.Num() > 0)
	{
		return filenames[0];
	}
	else
	{
		return creature_filename;
	}
#else
	return creature_filename;
#endif
}

TArray<uint8>& UCreaturePackAnimationAsset::GetFileData()
{
	// Decompress only when needed
	if (CreatureFileData.Num() == 0)
	{
		FArchiveLoadCompressedProxy Decompressor =
			FArchiveLoadCompressedProxy(CreatureZipBinary, ECompressionFlags::COMPRESS_ZLIB);

		if (Decompressor.IsError() || (CreatureZipBinary.Num() == 0))
		{
			UE_LOG(LogTemp, Warning, TEXT("UCreatureAnimationAsset::Could not uncompress data"));
			return CreatureFileData;
		}

		FBufferArchive DecompressedBinaryArray;
		Decompressor << DecompressedBinaryArray;
		
		CreatureFileData.SetNum(DecompressedBinaryArray.Num());
		for(auto j = 0; j < CreatureFileData.Num(); j++)
		{
			CreatureFileData[j] = DecompressedBinaryArray[j];
		}
	}

	return CreatureFileData;
}

void UCreaturePackAnimationAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}

#if WITH_EDITORONLY_DATA
void UCreaturePackAnimationAsset::SetCreatureFilename(const FString &newFilename)
{
	AssetImportData->UpdateFilenameOnly(newFilename);

	// extract again to ensure properly sanitised
	TArray<FString> filenames;
	AssetImportData->ExtractFilenames(filenames);
	if (filenames.Num() > 0)
	{
		creature_filename = filenames[0];
	}
}

void UCreaturePackAnimationAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	if (!creature_filename.IsEmpty())
	{
		OutTags.Add(FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}

	Super::GetAssetRegistryTags(OutTags);
}

void UCreaturePackAnimationAsset::PostLoad()
{
	Super::PostLoad();

	if (!creature_filename.IsEmpty() && AssetImportData && AssetImportData->GetSourceData().SourceFiles.Num() == 0)
	{
		// convert old source file path to proper UE4 Asset data system
		FAssetImportInfo Info;
		Info.Insert(FAssetImportInfo::FSourceFile(creature_filename));
		AssetImportData->SourceData = MoveTemp(Info);
	}

	if ((CreatureZipBinary.Num() != 0) || (CreatureFileData.Num() != 0))
	{
		// load the animation data caches from the json data
		GatherAnimationData();
	}
}

void UCreaturePackAnimationAsset::GatherAnimationData()
{
	// ensure the filenames are synced
	creature_filename = GetCreatureFilename();
	GetFileData();
}

void UCreaturePackAnimationAsset::PreSave()
{
	Super::PreSave();

	// before saving, always ensure animation data is up to date
	GatherAnimationData();
}

void UCreaturePackAnimationAsset::PostInitProperties()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}

	Super::PostInitProperties();
}

#endif /*WITH_EDITORONLY_DATA*/