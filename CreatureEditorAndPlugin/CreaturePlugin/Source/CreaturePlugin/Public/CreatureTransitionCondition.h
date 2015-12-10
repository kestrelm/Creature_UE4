/********************************************************************************
** auth： God Of Pen
** desc： 尚未编写描述
** Ver.:  V1.0.0
*********************************************************************************/
#include "Engine.h"
#include "CreatureAnimState.h"
#include "CreatureTransitionCondition.generated.h"
#pragma once
USTRUCT(BlueprintType)
struct FCreatureTransitionCondition{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(VisibleAnyWhere, Category = "Creature")
	FString TransitionName;
	//为真还是为假才会触发？
	UPROPERTY(VisibleAnyWhere, Category = "Creature")
	bool	TransitionFlag;

	FCreatureTransitionCondition()
	{
		TransitionName = FString();
		TransitionFlag = true;
	}

	FCreatureTransitionCondition(FString name, bool Flag)
	{
		TransitionName = name;
		TransitionFlag = Flag;
	}
	bool operator == (const FCreatureTransitionCondition& otherB) const{
		return TransitionName == otherB.TransitionName;
	}
};