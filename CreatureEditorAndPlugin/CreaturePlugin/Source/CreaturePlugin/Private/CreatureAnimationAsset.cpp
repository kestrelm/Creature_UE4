
#include "CreaturePluginPCH.h"
#include "CreatureAnimationAsset.h"
#include "CreatureCore.h"

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

const FCreatureAnimationPointsCache * UCreatureAnimationAsset::GetPointsCacheForClip(const FString & clipName) const
{
	for (const FCreatureAnimationPointsCache & cache : m_pointsCache)
	{
		if (cache.m_animationName == clipName)
		{
			return &cache;
		}
	}

	return nullptr;
}

void UCreatureAnimationAsset::LoadPointCacheForAllClips(class CreatureCore *forCore) const
{
	for (const FCreatureAnimationPointsCache & cache : m_pointsCache)
	{
		LoadPointCacheForClip(cache.m_animationName, forCore);
	}
}

void UCreatureAnimationAsset::LoadPointCacheForClip(const FString &animName, class CreatureCore *forCore) const
{
	check(forCore);
	const FCreatureAnimationPointsCache *cacheForAnim = GetPointsCacheForClip(animName);
	if (cacheForAnim && forCore->GetCreatureManager())
	{
		CreatureModule::CreatureAnimation *anim = forCore->GetCreatureManager()->GetAnimation(TCHAR_TO_UTF8(*animName));
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
			pts.push_back(new_pts);
		}
	}
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

	if (CreatureZipBinary.Num() != 0 || CreatureFileJSonData.IsEmpty() == false)
	{
		// load the animation data caches from the json data
		GatherAnimationData();
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

	m_pointsCache.Reset(all_animation_names.size());

	for (auto& cur_name : all_animation_names)
	{
		FString animName(cur_name.c_str());
		m_clipNames.Add(animName);

		if (m_pointsCacheApproximationLevel >= 0)
		{
			creature_core.GetCreatureManager()->ClearPointCache(cur_name);
			creature_core.GetCreatureManager()->MakePointCache(cur_name, m_pointsCacheApproximationLevel);
			CreatureModule::CreatureAnimation *anim = creature_core.GetCreatureManager()->GetAnimation(cur_name);
			if (ensure(anim) && anim->hasCachePts())
			{
				auto &pts = anim->getCachePts();

				FCreatureAnimationPointsCache &animPtsCache = m_pointsCache[m_pointsCache.AddZeroed(1)];
				animPtsCache.m_animationName = animName;
				animPtsCache.m_numArrays = pts.size();
				animPtsCache.m_points.Reserve(animPtsCache.m_numArrays * arraySize);

				for (float *pt : pts)
				{
					for (int32 i = 0; i < arraySize; i++)
					{
						animPtsCache.m_points.Add(pt[i]);
					}
				}
			}
		}
	}
}

void UCreatureAnimationAsset::PreSave()
{
	Super::PreSave();

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