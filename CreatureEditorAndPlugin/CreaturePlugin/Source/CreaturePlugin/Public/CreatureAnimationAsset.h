/********************************************************************************
** auth： God Of Pen
** desc： 存储单个动画文件包含的所有动画
** Ver.:  V1.0.0
*********************************************************************************/
#pragma  once
#include "Engine.h"
#include "CreatureAnimationAsset.generated.h"


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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Creature")
		//You can change an animation clip's scale to fix some problem
	float Scale = 1.0f;

	// Zip Binary Data
	UPROPERTY()
	TArray<uint8> CreatureZipBinary;

	FString& GetJsonString();


#if WITH_EDITORONLY_DATA
	void SetCreatureFilename(const FString &newFilename);
	void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	void PostLoad() override;
	void PreSave() override;
	void PostInitProperties() override;

protected:
	// Denoting creature filename using UE4's asset registry system
	// kepy in sync with creature_filename
	UPROPERTY(VisibleAnywhere, Instanced, Category = ImportSettings)
	class UAssetImportData* AssetImportData;
#endif

protected:
	// Uncompressed JSon Data
	FString CreatureFileJSonData;
	
	// Denoting creature filename: stored as the creature runtime uses this in packaged builds
	// kept in sync with AssetImportData
	UPROPERTY()
	FString creature_filename;

};
