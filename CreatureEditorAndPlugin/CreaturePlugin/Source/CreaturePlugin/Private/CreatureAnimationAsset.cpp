
#include "CreaturePluginPCH.h"
#include "CreatureAnimationAsset.h"

FString UCreatureAnimationAsset::GetCreatureFilename() const
{
#if WITH_EDITORONLY_DATA
	TArray<FString> filenames;
	AssetImportData->ExtractFilenames(filenames);
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

FString& UCreatureAnimationAsset::GetJsonString()
{
	// Decompress only when needed
	if (CreatureFileJSonData.IsEmpty())
	{
		FArchiveLoadCompressedProxy Decompressor =
			FArchiveLoadCompressedProxy(CreatureZipBinary, ECompressionFlags::COMPRESS_ZLIB);

		if (Decompressor.IsError() || (CreatureZipBinary.Num() == 0))
		{
			UE_LOG(LogTemp, Warning, TEXT("UCreatureAnimationAsset::Could not uncompress data"));
			return CreatureFileJSonData;
		}

		FBufferArchive DecompressedBinaryArray;
		Decompressor << DecompressedBinaryArray;
		CreatureFileJSonData = UTF8_TO_TCHAR((char *)DecompressedBinaryArray.GetData());
	}

	return CreatureFileJSonData;
}
#if WITH_EDITORONLY_DATA

void UCreatureAnimationAsset::SetCreatureFilename(const FString &newFilename)
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

void UCreatureAnimationAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	if (!creature_filename.IsEmpty())
	{
		OutTags.Add(FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}

	Super::GetAssetRegistryTags(OutTags);
}

void UCreatureAnimationAsset::PostLoad()
{
	Super::PostLoad();

	if (!creature_filename.IsEmpty() && AssetImportData && AssetImportData->GetSourceData().SourceFiles.Num() == 0)
	{
		// convert old source file path to proper UE4 Asset data system
		FAssetImportInfo Info;
		Info.Insert(FAssetImportInfo::FSourceFile(creature_filename));
		AssetImportData->SourceData = MoveTemp(Info);
	}
}

void UCreatureAnimationAsset::PreSave()
{
	Super::PreSave();

	// ensure the filenames are synced
	creature_filename = GetCreatureFilename();
}

void UCreatureAnimationAsset::PostInitProperties()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}

	Super::PostInitProperties();
}

#endif