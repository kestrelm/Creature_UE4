#include "CreaturePackEditorPCH.h"
#include "CreaturePackAnimationAssetFactory.h"
#include "CreaturePackAnimationAsset.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include <string>
#define LOCTEXT_NAMESPACE "CreaturePackAnimationAssetFactory"

UCreaturePackAnimationAssetFactory::UCreaturePackAnimationAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	bEditorImport = true;
	SupportedClass = UCreaturePackAnimationAsset::StaticClass();

	Formats.Add(TEXT("creature_pack;CREATURE_PACK"));
}
UObject* UCreaturePackAnimationAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    const FString Filter(TEXT("Creature Pack Files (*.creature_pack)|*.creature_pack"));
    
	UCreaturePackAnimationAsset* Asset = NewObject<UCreaturePackAnimationAsset>(InParent, Class, Name, Flags);
	TArray<FString> OpenFilenames;
	int32 FilterIndex = -1;
	if (FDesktopPlatformModule::Get()->OpenFileDialog(
		nullptr,
		FString(TEXT("Choose a Creature Pack file")),
		FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT),
		TEXT(""),
		Filter,
		EFileDialogFlags::None,
		OpenFilenames,
		FilterIndex))
	{
		Asset->SetCreatureFilename(OpenFilenames[0]);

		ImportSourceFile(Asset);
	}

	return Asset;
}

bool UCreaturePackAnimationAssetFactory::ImportSourceFile(UCreaturePackAnimationAsset *forAsset) const
{
	const FString &creatureFilename = forAsset->GetCreatureFilename();
	if (forAsset == nullptr || creatureFilename.IsEmpty())
	{
		return false;
	}

	TArray<uint8> readBytes;
	if (!FFileHelper::LoadFileToArray(readBytes, *creatureFilename, 0))
	{
		return false;
	}

	forAsset->CreatureZipBinary.Reset();
	FArchiveSaveCompressedProxy Compressor =
		FArchiveSaveCompressedProxy(forAsset->CreatureZipBinary, ECompressionFlags::COMPRESS_ZLIB);

	Compressor << readBytes;
	Compressor.Flush();

	forAsset->GatherAnimationData();

	return true;
}

bool UCreaturePackAnimationAssetFactory::FactoryCanImport(const FString& Filename)
{
	return true;
}

bool UCreaturePackAnimationAssetFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UCreaturePackAnimationAsset* asset = Cast<UCreaturePackAnimationAsset>(Obj);
	if (asset)
	{
		const FString &filename = asset->GetCreatureFilename();
		if (!filename.IsEmpty())
		{
			OutFilenames.Add(filename);
		}

		return true;
	}
	return false;
}

void UCreaturePackAnimationAssetFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UCreaturePackAnimationAsset* asset = Cast<UCreaturePackAnimationAsset>(Obj);
	if (asset && ensure(NewReimportPaths.Num() == 1))
	{
		asset->SetCreatureFilename(NewReimportPaths[0]);
	}
}

EReimportResult::Type UCreaturePackAnimationAssetFactory::Reimport(UObject* Obj)
{
	if (ImportSourceFile(Cast<UCreaturePackAnimationAsset>(Obj)))
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

int32 UCreaturePackAnimationAssetFactory::GetPriority() const
{
	return ImportPriority;
}

#undef LOCTEXT_NAMESPACE