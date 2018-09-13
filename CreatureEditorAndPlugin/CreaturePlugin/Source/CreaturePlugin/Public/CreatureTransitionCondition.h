/********************************************************************************
** Author God Of Pen
** Ver.:  V1.0.0
*********************************************************************************/

#include "CoreMinimal.h"
#include "CreatureAnimState.h"
#include "CreatureTransitionCondition.generated.h"

#pragma once

USTRUCT(BlueprintType)
struct FCreatureTransitionCondition{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(VisibleAnyWhere, Category = "Creature")
	FName TransitionName;
	//为真还是为假才会触发？
	UPROPERTY(VisibleAnyWhere, Category = "Creature")
	bool	TransitionFlag;

	FCreatureTransitionCondition()
	{
		TransitionName = NAME_None;
		TransitionFlag = true;
	}

	FCreatureTransitionCondition(FName name, bool Flag)
	{
		TransitionName = name;
		TransitionFlag = Flag;
	}
	bool operator == (const FCreatureTransitionCondition& otherB) const{
		return TransitionName == otherB.TransitionName;
	}
};