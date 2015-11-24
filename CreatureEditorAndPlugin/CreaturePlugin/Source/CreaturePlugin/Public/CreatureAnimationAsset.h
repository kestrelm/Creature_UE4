/********************************************************************************
** auth£º God Of Pen
** desc£º ´æ´¢µ¥¸ö¶¯»­ÎÄ¼þ°üº¬µÄËùÓÐ¶¯»­
** Ver.:  V1.0.0
*********************************************************************************/
#pragma  once
#include "Engine.h"
#include "CreatureAnimationAsset.generated.h"


UCLASS()
class CREATUREPLUGIN_API UCreatureAnimationAsset :public UObject{
	GENERATED_BODY()
public:
	// Denoting creature filename¡
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creature")
	FString creature_filename;

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

protected:
	// Uncompressed JSon Data
	FString CreatureFileJSonData;

};
