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

		bool bIsCurrentState=false;

public:
	void BeginState();
	void EndState();
	void CheckCondition();

};