#include "Engine.h"

#include "CreatureAnimState.generated.h"
#pragma once
class UCreatureAnimTransition;
UCLASS()
class CREATUREPLUGIN_API UCreatureAnimState :public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY()
	class UCreatureAnimStateMachine* AnimStateMachine;

	UPROPERTY()
	TArray<UCreatureAnimTransition*> TransitionList;

	UPROPERTY()
	FString AnimStateName;

public:
	void BeginState(class UCreatureAnimStateMachineInstance *forInstance);
	void EndState(class UCreatureAnimStateMachineInstance *forInstance);
	void CheckCondition(class UCreatureAnimStateMachineInstance *forInstance);
	//Check if there is a 'AnimationEnd' Transition
	void AnimationEnd(class UCreatureAnimStateMachineInstance *forInstance);
};
