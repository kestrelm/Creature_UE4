#include "CreatureEditorPCH.h"
#include "CreatureAnimationAssetFactory.h"
#include "CreatureAnimationAsset.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#define LOCTEXT_NAMESPACE "CreatureAnimationAssetFactory"
UCreatureAnimationAssetFactory::UCreatureAnimationAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UCreatureAnimationAsset::StaticClass();
}
UObject* UCreatureAnimationAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UCreatureAnimationAsset* Asset = NewObject<UCreatureAnimationAsset>(InParent, Class, Name, Flags);
	TArray<FString> OpenFilenames;
	int32 FilterIndex = -1;
	if (FDesktopPlatformModule::Get()->OpenFileDialog(
		nullptr,
		FString(TEXT("Choose a JSon file")),
		FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT),
		TEXT(""),
		FString(TEXT("*.json")),
		EFileDialogFlags::Multiple,
		OpenFilenames,
		FilterIndex))
	{
		FFileHelper::LoadFileToString(Asset->CreatureFileJSonData, *OpenFilenames[0], 0);

		FString setFilename, setFileExtension, setFilePathPart;
		FPaths::Split(FString(*OpenFilenames[0]), setFilePathPart, setFilename, setFileExtension);
		Asset->creature_filename = setFilename + FString(".") + setFileExtension;
	}
	return Asset;
}
#undef LOCTEXT_NAMESPACE