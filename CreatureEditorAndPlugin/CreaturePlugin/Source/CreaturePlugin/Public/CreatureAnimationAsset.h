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
		//为了向下兼容，请勿随意使用！
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creature")
		FString creature_filename;

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creature")
		TArray<FString> AnimationClipList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
		float animation_speed=1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
		UMaterialInterface * collection_material;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Creature")
	//You can change an animation clip's scale to fix some problem
		float Scale = 1.0f;
	//从文件中读取到的JSon Data
	UPROPERTY()
		FString CreatureFileJSonData;


};
