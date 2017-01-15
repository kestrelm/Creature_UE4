#include "CreatureEditorPCH.h"
#include "CreatureAnimationAssetFactory.h"
#include "CreatureAnimationAsset.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include <string>
#define LOCTEXT_NAMESPACE "CreatureAnimationAssetFactory"

UCreatureAnimationAssetFactory::UCreatureAnimationAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	bEditorImport = true;
	SupportedClass = UCreatureAnimationAsset::StaticClass();

	Formats.Add(TEXT("json;JSON"));
}
UObject* UCreatureAnimationAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    const FString Filter(TEXT("JSON Files (*.json)|*.json"));
    
	UCreatureAnimationAsset* Asset = NewObject<UCreatureAnimationAsset>(InParent, Class, Name, Flags);
	TArray<FString> OpenFilenames;
	int32 FilterIndex = -1;
	if (FDesktopPlatformModule::Get()->OpenFileDialog(
		nullptr,
		FString(TEXT("Choose a JSON or Zipped JSON file")),
		FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT),
		TEXT(""),
		Filter,
		EFileDialogFlags::None,
		OpenFilenames,
		FilterIndex))
	{
		Asset->SetCreatureFilename(FName(*OpenFilenames[0]));

		ImportSourceFile(Asset);
	}

	return Asset;
}

bool UCreatureAnimationAssetFactory::ImportSourceFile(UCreatureAnimationAsset *forAsset) const
{
	FString creatureFilename = forAsset->UpdateAndGetCreatureFilename().ToString();
	if (forAsset == nullptr || creatureFilename.IsEmpty())
	{
		return false;
	}

	FString readString;
	if (!FFileHelper::LoadFileToString(readString, *creatureFilename, 0))
	{
		return false;
	}

#ifdef CREATURE_USE_COMPRESS_JSON
	// Run compression routine
	std::string saveString(TCHAR_TO_UTF8(*readString));

	forAsset->CreatureZipBinary.Reset();
	FArchiveSaveCompressedProxy Compressor =
		FArchiveSaveCompressedProxy(forAsset->CreatureZipBinary, ECompressionFlags::COMPRESS_ZLIB);
	TArray<uint8> writeData;
	writeData.SetNumUninitialized(saveString.length() + 1);
	for (size_t i = 0; i < saveString.length(); i++)
	{
		writeData[i] = saveString.c_str()[i];
	}

	writeData[writeData.Num() - 1] = '\0';

	Compressor << writeData;
	Compressor.Flush();
#else
	// Just use the uncompressed string
	forAsset->CreatureRawJSONString = readString;
#endif

	forAsset->GatherAnimationData();

	return true;
}

bool UCreatureAnimationAssetFactory::FactoryCanImport(const FString& Filename)
{
	return true;
}

bool UCreatureAnimationAssetFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UCreatureAnimationAsset* asset = Cast<UCreatureAnimationAsset>(Obj);
	if (asset)
	{
		FString filename = asset->UpdateAndGetCreatureFilename().ToString();
		if (!filename.IsEmpty())
		{
			OutFilenames.Add(filename);
		}

		return true;
	}
	return false;
}

void UCreatureAnimationAssetFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UCreatureAnimationAsset* asset = Cast<UCreatureAnimationAsset>(Obj);
	if (asset && ensure(NewReimportPaths.Num() == 1))
	{
		asset->SetCreatureFilename(FName(*NewReimportPaths[0]));
	}
}

EReimportResult::Type UCreatureAnimationAssetFactory::Reimport(UObject* Obj)
{
	if (ImportSourceFile(Cast<UCreatureAnimationAsset>(Obj)))
	{
		// Try to find the outer package so we can dirty it up
		if (Obj->GetOuter())
		{
			Obj->GetOuter()->MarkPackageDirty();
		}
		else
		{
			Obj->MarkPackageDirty();
		}
		return EReimportResult::Succeeded;
	}
	else
	{
		return EReimportResult::Failed;
	}
}

int32 UCreatureAnimationAssetFactory::GetPriority() const
{
	return ImportPriority;
}

#undef LOCTEXT_NAMESPACE