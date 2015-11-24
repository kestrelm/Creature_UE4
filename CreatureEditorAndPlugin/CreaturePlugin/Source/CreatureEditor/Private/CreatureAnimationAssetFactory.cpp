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
	SupportedClass = UCreatureAnimationAsset::StaticClass();
}
UObject* UCreatureAnimationAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UCreatureAnimationAsset* Asset = NewObject<UCreatureAnimationAsset>(InParent, Class, Name, Flags);
	TArray<FString> OpenFilenames;
	int32 FilterIndex = -1;
	if (FDesktopPlatformModule::Get()->OpenFileDialog(
		nullptr,
		FString(TEXT("Choose a JSON or Zipped JSON file")),
		FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT),
		TEXT(""),
		FString(TEXT("*.json")),
		EFileDialogFlags::Multiple,
		OpenFilenames,
		FilterIndex))
	{
		auto cur_filename = *OpenFilenames[0];
		FString readString;
		FFileHelper::LoadFileToString(readString, cur_filename, 0);

		std::string saveString(TCHAR_TO_UTF8(*readString));

		FArchiveSaveCompressedProxy Compressor =
			FArchiveSaveCompressedProxy(Asset->CreatureZipBinary, ECompressionFlags::COMPRESS_ZLIB);
		TArray<uint8> writeData;
		writeData.Init(saveString.length() + 1);
		for (size_t i = 0; i < saveString.length(); i++)
		{
			writeData[i] = saveString.c_str()[i];
		}

		writeData[writeData.Num() - 1] = '\0';

		Compressor << writeData;
		Compressor.Flush();

		FString setFilename, setFileExtension, setFilePathPart;
		FPaths::Split(FString(*OpenFilenames[0]), setFilePathPart, setFilename, setFileExtension);
		Asset->creature_filename = setFilename + FString(".") + setFileExtension;
	}

	return Asset;
}
#undef LOCTEXT_NAMESPACE