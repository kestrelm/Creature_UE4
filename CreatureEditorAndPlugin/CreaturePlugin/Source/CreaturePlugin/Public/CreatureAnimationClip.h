/********************************************************************************
** Author God Of Pen
** Ver.:  V1.0.0
*********************************************************************************/

#include "CoreMinimal.h"
#include "CreatureAnimationAsset.h"
#include "CreatureAnimationClip.generated.h"
#pragma  once
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
USTRUCT(BlueprintType)
struct FCreatureAnimationShortClip
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Creature")
	UCreatureAnimationAsset* SourceAsset;
		
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Creature")
	FName ClipNameInAsset;
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
USTRUCT(BlueprintType)
struct FCreatureAnimationClip
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Creature")
	FName ClipName;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Creature")
	TArray<FCreatureAnimationShortClip> ShortClipList;


};