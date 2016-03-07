/********************************************************************************
** auth： God Of Pen
** desc： 存储单个动画文件包含的所有动画
** Ver.:  V1.0.0
*********************************************************************************/
#pragma  once
#include "Engine.h"
#include "CreatureAnimationAsset.generated.h"

USTRUCT()
struct FCreatureAnimationPointsCache
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<float> m_points;

	UPROPERTY()
	int32 m_numArrays;
	
	UPROPERTY()
	FString m_animationName;
};

UCLASS()
class CREATUREPLUGIN_API UCreatureAnimationAsset :public UObject{
	GENERATED_BODY()
public:

	FString GetCreatureFilename() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creature")
	TArray<FString> AnimationClipList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float animation_speed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	UMaterialInterface * collection_material;

	//You can change an animation clip's scale to fix some problem
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Creature")
	float Scale = 1.0f;

	// Zip Binary Data
	UPROPERTY()
	TArray<uint8> CreatureZipBinary;

	FString& GetJsonString();
	
	/** The approximation level to use when generating the point cache (range 0-20; 0=no approximation, -1=no cache generated) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Creature)
	int32 m_pointsCacheApproximationLevel;

	const FCreatureAnimationPointsCache *GetPointsCacheForClip(const FString & clipName) const;
	void LoadPointCacheForAllClips(class CreatureCore *forCore) const;
	void LoadPointCacheForClip(const FString &animName, class CreatureCore *forCore) const;

#if WITH_EDITORONLY_DATA
	void SetCreatureFilename(const FString &newFilename);
	void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	void PostLoad() override;
	void PreSave() override;
	void PostInitProperties() override;

	/** The names of all clips held in this asset, for reference (editor only) */
	UPROPERTY(VisibleAnywhere, Category=Creature)
	TArray<FString> m_clipNames;

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
	FString creature_filename;
	
	/** Cache of points for the animation clips, to improve runtime performance */
	UPROPERTY()
	TArray<FCreatureAnimationPointsCache> m_pointsCache;
};
