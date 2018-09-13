/********************************************************************************
** Author God Of Pen
** Ver.:  V1.0.0
*********************************************************************************/


#include "CoreMinimal.h"
#include "CreatureAnimationClip.h"
#include "CreatureAnimationClipsStore.generated.h"

#pragma  once



UCLASS(Blueprintable)
class CREATUREPLUGIN_API UCreatureAnimationClipsStore :public UObject{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Creature")
		TArray<FCreatureAnimationClip> ClipList;

	void LoadAnimationDataToComponent(class UCreatureMeshComponent* MeshComponent);
};