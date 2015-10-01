/********************************************************************************
** auth£∫ God Of Pen
** desc£∫ …–Œ¥±‡–¥√Ë ˆ
** Ver.:  V1.0.0
*********************************************************************************/

#include "Engine.h"
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