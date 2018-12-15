#pragma  once
#include "UObject/Object.h"
#include "EditorFramework/AssetImportData.h"
#include "Materials/MaterialInterface.h"
#include "CreaturePackAnimationAsset.generated.h"

UCLASS()
class CREATUREPACKRUNTIMEPLUGIN_API UCreaturePackAnimationAsset :public UObject{
	GENERATED_BODY()
public:

	FString GetCreatureFilename() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float animation_speed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	UMaterialInterface * collection_material;

	// Zip Binary Data
	UPROPERTY()
	TArray<uint8> CreatureZipBinary;

	TArray<uint8>& GetFileData();
	
	virtual void Serialize(FArchive& Ar) override;

#if WITH_EDITORONLY_DATA
	void SetCreatureFilename(const FString &newFilename);
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
	// Uncompressed Binary Data
	TArray<uint8> CreatureFileData;
	
	// Denoting creature filename: stored as the creature runtime uses this in packaged builds
	// kept in sync with AssetImportData
	UPROPERTY()
	FString creature_filename;	
};
