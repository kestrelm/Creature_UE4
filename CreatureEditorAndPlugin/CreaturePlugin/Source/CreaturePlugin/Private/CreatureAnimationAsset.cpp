
#include "CreaturePluginPCH.h"
#include "CreatureAnimationAsset.h"
#include "CreatureCore.h"

FName UCreatureAnimationAsset::GetCreatureFilename() const
{
#if WITH_EDITORONLY_DATA
	TArray<FString> filenames;
	if (AssetImportData)
	{
		AssetImportData->ExtractFilenames(filenames);
	}
	if (filenames.Num() > 0)
	{
		return FName(*filenames[0]);
	}
	else
	{
		return creature_filename;
	}
#else
	return creature_filename;
#endif
}

bool 
UCreatureAnimationAsset::UseCompressedData() const
{
	return (CreatureZipBinary.Num() > 0);
}

FString& UCreatureAnimationAsset::GetJsonString()
{
	// Decide if we should decompress or return the raw uncompressed string
	if(!UseCompressedData())
	{
		if (CreatureFileJSonData.IsEmpty())
		{
			CreatureFileJSonData = CreatureRawJSONString;
		}
	}
	else {
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
	}

	return CreatureFileJSonData;
}

const FCreatureAnimationDataCache * UCreatureAnimationAsset::GetDataCacheForClip(const FName & clipName) const
{
	for (const FCreatureAnimationDataCache & cache : m_dataCache)
	{
		if (cache.m_animationName == clipName)
		{
			return &cache;
		}
	}

	return nullptr;
}

float UCreatureAnimationAsset::GetClipLength(const FName & clipName) const
{
	const FCreatureAnimationDataCache *cacheForAnim = GetDataCacheForClip(clipName);
	if (cacheForAnim)
	{
		return cacheForAnim->m_length / animation_speed;
	}
	else
	{
		return 0.0f;
	}
}

void UCreatureAnimationAsset::LoadPointCacheForAllClips(class CreatureCore *forCore) const
{
	for (const FCreatureAnimationDataCache & cache : m_dataCache)
	{
		LoadPointCacheForClip(cache.m_animationName, forCore);
	}
}

void UCreatureAnimationAsset::LoadPointCacheForClip(const FName &animName, class CreatureCore *forCore) const
{
	check(forCore);
	const FCreatureAnimationDataCache *cacheForAnim = GetDataCacheForClip(animName);
	if (cacheForAnim && forCore->GetCreatureManager())
	{
		CreatureModule::CreatureAnimation *anim = forCore->GetCreatureManager()->GetAnimation(animName);
		if (anim == nullptr || anim->hasCachePts())
		{
			return;
		}

		check(forCore->GetCreatureManager()->GetCreature());
		int32 arraySize = forCore->GetCreatureManager()->GetCreature()->GetTotalNumPoints() * 3;
		auto &pts = anim->getCachePts();
		int32 sourcePtIdx = 0;
		ensure(cacheForAnim->m_numArrays * arraySize == cacheForAnim->m_points.Num());
		for (int32 i = 0; i < cacheForAnim->m_numArrays; i++)
		{
			auto new_pts = new glm::float32[arraySize];
			for (int32 j = 0; j < arraySize; j++)
			{
				new_pts[j] = cacheForAnim->m_points[sourcePtIdx++];
			}
			pts.Add(new_pts);
		}
	}
}

void UCreatureAnimationAsset::Serialize(FArchive& Ar)
{
	if (Ar.IsSaving() && !Ar.IsCooking())
	{
		// when saving non-cooked asset, don't include the datacache as it's huge
		TArray<FCreatureAnimationDataCache> cacheCopy = m_dataCache;
		m_dataCache.Reset();

		Super::Serialize(Ar);

		m_dataCache = cacheCopy;
	}
	else
	{
		Super::Serialize(Ar);
	}
}

#if WITH_EDITORONLY_DATA
void UCreatureAnimationAsset::SetCreatureFilename(const FName &newFilename)
{
	AssetImportData->UpdateFilenameOnly(newFilename.ToString());

	// extract again to ensure properly sanitised
	TArray<FString> filenames;
	AssetImportData->ExtractFilenames(filenames);
	if (filenames.Num() > 0)
	{
		creature_filename = FName(*filenames[0]);
	}
}

void UCreatureAnimationAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	if (creature_filename != NAME_None)
	{
		OutTags.Add(FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}

	Super::GetAssetRegistryTags(OutTags);
}

void UCreatureAnimationAsset::PostLoad()
{
	Super::PostLoad();

	if (!creature_filename.IsNone() && AssetImportData && AssetImportData->GetSourceData().SourceFiles.Num() == 0)
	{
		// convert old source file path to proper UE4 Asset data system
		FAssetImportInfo Info;
		Info.Insert(FAssetImportInfo::FSourceFile(creature_filename.ToString()));
		AssetImportData->SourceData = MoveTemp(Info);
	}

	if (UseCompressedData())
	{
		if (CreatureZipBinary.Num() != 0 || CreatureFileJSonData.IsEmpty() == false)
		{
			// load the animation data caches from the json data
			GatherAnimationData();
		}
	}
	else {
		if (CreatureRawJSONString.Len() > 0)
		{
			// load the animation data caches from the json data
			GatherAnimationData();
		}
	}
}

void UCreatureAnimationAsset::GatherAnimationData()
{
	// ensure the filenames are synced
	creature_filename = GetCreatureFilename();
	
	// load the JSON data into creature so we can extract the animation names and generate the point caches for the anims
	CreatureCore creature_core;
	creature_core.pJsonData = &GetJsonString();
	creature_core.creature_filename = creature_filename;
	creature_core.InitCreatureRender();

	auto all_animation_names = creature_core.GetCreatureManager()->GetCreature()->GetAnimationNames();

	int32 arraySize = creature_core.GetCreatureManager()->GetCreature()->GetTotalNumPoints() * 3;

	m_dataCache.Reset(all_animation_names.Num());

	for (auto& cur_name : all_animation_names)
	{
		CreatureModule::CreatureAnimation *anim = creature_core.GetCreatureManager()->GetAnimation(cur_name);
		if (ensure(anim))
		{
			FCreatureAnimationDataCache &animDataCache = m_dataCache[m_dataCache.AddZeroed(1)];
			animDataCache.m_animationName = cur_name;
			animDataCache.m_length = anim->getEndTime() - anim->getStartTime();

			if (m_pointsCacheApproximationLevel >= 0)
			{
				creature_core.GetCreatureManager()->ClearPointCache(cur_name);
				creature_core.GetCreatureManager()->MakePointCache(cur_name, m_pointsCacheApproximationLevel);
				if (anim->hasCachePts())
				{
					auto &pts = anim->getCachePts();

					float startTime = anim->getStartTime();
					float endTime = anim->getEndTime();

					animDataCache.m_numArrays = pts.Num();
					animDataCache.m_points.Reserve(animDataCache.m_numArrays * arraySize);

					for (float *pt : pts)
					{
						for (int32 i = 0; i < arraySize; i++)
						{
							animDataCache.m_points.Add(pt[i]);
						}
					}
				}
			}
		}
	}
}

void UCreatureAnimationAsset::PreSave(const class ITargetPlatform* TargetPlatform)
{
	Super::PreSave(TargetPlatform);

	// before saving, always ensure animation data is up to date
	GatherAnimationData();
}

void UCreatureAnimationAsset::PostInitProperties()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}

	Super::PostInitProperties();
}

#endif /*WITH_EDITORONLY_DATA*/