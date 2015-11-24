//need Change!!!
#include "CustomProceduralMesh.h"
//#include "CreaturePluginPCH.h"
#include "CreatureAnimationAsset.h"

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
