#include "CreatureMetaAssetFactory.h"
#include "CreatureMetaAsset.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include <string>
#define LOCTEXT_NAMESPACE "CreatureMetaAssetFactory"

UCreatureMetaAssetFactory::UCreatureMetaAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	bEditorImport = true;
	SupportedClass = UCreatureMetaAsset::StaticClass();

	Formats.Add(TEXT("mdata;MDATA"));
}
UObject* UCreatureMetaAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    const FString Filter(TEXT("mdata Files (*.mdata)|*.mdata"));
    
	UCreatureMetaAsset* Asset = NewObject<UCreatureMetaAsset>(InParent, Class, Name, Flags);
	TArray<FString> OpenFilenames;
	int32 FilterIndex = -1;
	if (FDesktopPlatformModule::Get()->OpenFileDialog(
		nullptr,
		FString(TEXT("Choose a Creature mdata file")),
		FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT),
		TEXT(""),
		Filter,
		EFileDialogFlags::None,
		OpenFilenames,
		FilterIndex))
	{
		ImportSourceFile(Asset, OpenFilenames[0]);
	}

	return Asset;
}

bool UCreatureMetaAssetFactory::ImportSourceFile(UCreatureMetaAsset *forAsset, FString importFilename) const
{
	if (forAsset == nullptr)
	{
		return false;
	}

	FString readString;
	if (!FFileHelper::LoadFileToString(readString, *importFilename))
	{
		return false;
	}

    forAsset->jsonString = readString;
	forAsset->BuildMetaData();
	forAsset->SetSourceFilename(importFilename);
	return true;
}

bool UCreatureMetaAssetFactory::FactoryCanImport(const FString& Filename)
{
	return true;
}

bool UCreatureMetaAssetFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UCreatureMetaAsset* asset = Cast<UCreatureMetaAsset>(Obj);
	if (asset)
	{
		FString filename = asset->GetSourceFilename();
		if (!filename.IsEmpty())
		{
			OutFilenames.Add(filename);
		}

		return true;
	}
	return false;
}

void UCreatureMetaAssetFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) 
{
	UCreatureMetaAsset* asset = Cast<UCreatureMetaAsset>(Obj);
	if (asset && ensure(NewReimportPaths.Num() == 1))
	{
		asset->SetSourceFilename(*NewReimportPaths[0]);
	}
}

EReimportResult::Type 
UCreatureMetaAssetFactory::Reimport(UObject* Obj) 
{
	UCreatureMetaAsset *metaAsset = Cast<UCreatureMetaAsset>(Obj);
	if (metaAsset && ImportSourceFile(metaAsset, metaAsset->GetSourceFilename()))
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

int32 UCreatureMetaAssetFactory::GetPriority() const
{
	return ImportPriority;
}

#undef LOCTEXT_NAMESPACE