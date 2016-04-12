/********************************************************************************
** auth： God Of Pen
** desc： 尚未编写描述
** Ver.:  V1.0.0
*********************************************************************************/
#include "Engine.h"
#include "CreatureAnimationAsset.h"
#include "CreatureAnimationClip.generated.h"
#pragma  once
//////////////////////////////////////////////////////////////////////////
//短片段，多个来自不同Asset的短片段将会组成一个真正的片段用于播放
//////////////////////////////////////////////////////////////////////////
USTRUCT()
struct FCreatureAnimationShortClip
{
	GENERATED_USTRUCT_BODY()
	//指向源AnimationAsset的指针
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Creature")
	UCreatureAnimationAsset* SourceAsset;
		
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Creature")
	FName ClipNameInAsset;
};
//////////////////////////////////////////////////////////////////////////
//真正的Clip片段
//////////////////////////////////////////////////////////////////////////
USTRUCT()
struct FCreatureAnimationClip
{
	GENERATED_USTRUCT_BODY()
	//指向源AnimationAsset的指针
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Creature")
	FName ClipName;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Creature")
	TArray<FCreatureAnimationShortClip> ShortClipList;


};