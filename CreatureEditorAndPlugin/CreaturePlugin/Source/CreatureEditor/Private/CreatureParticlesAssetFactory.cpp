#include "CreatureParticlesAssetFactory.h"
#include "CreatureParticlesAsset.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include <string>
#define LOCTEXT_NAMESPACE "CreatureParticlesAssetFactory"

UCreatureParticlesAssetFactory::UCreatureParticlesAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	bEditorImport = true;
	SupportedClass = UCreatureParticlesAsset::StaticClass();

	Formats.Add(TEXT("particles;PARTICLES"));
}
UObject* UCreatureParticlesAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    const FString Filter(TEXT("particles Files (*.particles)|*.particles"));
    
	UCreatureParticlesAsset* Asset = NewObject<UCreatureParticlesAsset>(InParent, Class, Name, Flags);
	TArray<FString> OpenFilenames;
	int32 FilterIndex = -1;
	if (FDesktopPlatformModule::Get()->OpenFileDialog(
		nullptr,
		FString(TEXT("Choose a Creature particles file")),
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

bool UCreatureParticlesAssetFactory::ImportSourceFile(UCreatureParticlesAsset *forAsset, FString importFilename) const
{
	TArray<uint8> readBytes;
	if (!FFileHelper::LoadFileToArray(readBytes, *importFilename, 0))
	{
		return false;
	}

	forAsset->setData(readBytes);
	return true;
}

bool UCreatureParticlesAssetFactory::FactoryCanImport(const FString& Filename)
{
	return true;
}

bool UCreatureParticlesAssetFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	return false;
}

void UCreatureParticlesAssetFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) 
{
}

EReimportResult::Type 
UCreatureParticlesAssetFactory::Reimport(UObject* Obj) 
{
	return EReimportResult::Failed;
}

int32 UCreatureParticlesAssetFactory::GetPriority() const
{
	return ImportPriority;
}

#undef LOCTEXT_NAMESPACE