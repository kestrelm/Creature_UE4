/********************************************************************************
** auth： God Of Pen
** desc： 存储单个动画文件包含的所有动画
** Ver.:  V1.0.0
*********************************************************************************/
#pragma  once
#include "Engine.h"
#include "CreatureAnimationAsset.generated.h"

/** Container used to cache useful data about an animation, including it's point cache */
USTRUCT()
struct FCreatureAnimationDataCache
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<float> m_points;

	UPROPERTY()
	int32 m_numArrays;

	UPROPERTY(VisibleAnywhere, Category = Creature)
	float m_length;

	UPROPERTY(VisibleAnywhere, Category = Creature)
	FName m_animationName;
};

UCLASS()
class CREATUREPLUGIN_API UCreatureAnimationAsset :public UObject{
	GENERATED_BODY()
public:

	FName GetCreatureFilename() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float animation_speed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	UMaterialInterface * collection_material;

	// Zip Binary Data
	UPROPERTY()
	TArray<uint8> CreatureZipBinary;

	// Uncompressed String
	UPROPERTY()
	FString CreatureRawJSONString;

	FString& GetJsonString();
	
	/** The approximation level to use when generating the point cache (range 0-20; 0=no approximation, -1=no cache generated) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Creature)
	int32 m_pointsCacheApproximationLevel;

	const FCreatureAnimationDataCache *GetDataCacheForClip(const FName & clipName) const;

	float GetClipLength(const FName & clipName) const;
	void LoadPointCacheForAllClips(class CreatureCore *forCore) const;
	void LoadPointCacheForClip(const FName &animName, class CreatureCore *forCore) const;

	bool UseCompressedData() const;

	virtual void Serialize(FArchive& Ar) override;

#if WITH_EDITORONLY_DATA
	void SetCreatureFilename(const FName &newFilename);
	void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	void PostLoad() override;
	void PreSave(const class ITargetPlatform* TargetPlatform) override;
	void PostInitProperties() override;
	
	void GatherAnimationData();
	
protected:
	// Denoting creature filename using UE4's asset registry system
	// keep in sync with creature_filename
	UPROPERTY(VisibleAnywhere, Instanced, Category = ImportSettings)
	class UAssetImportData* AssetImportData;
#endif /*WITH_EDITORONLY_DATA*/

protected:
	// Uncompressed JSon Data
	FString CreatureFileJSonData;
	
	// Denoting creature filename: stored as the creature runtime uses this in packaged builds
	// kept in sync with AssetImportData
	UPROPERTY()
	FName creature_filename;
	
	/** Cache of useful data, including point cache, for the animation clips, to improve runtime performance */
	UPROPERTY(VisibleAnywhere, Category = Creature)
	TArray<FCreatureAnimationDataCache> m_dataCache;
};
