#pragma once

#include "CreatureAnimStateMachineInstance.generated.h"

UCLASS()
class CREATUREPLUGIN_API UCreatureAnimStateMachineInstance : public UObject
{
	GENERATED_BODY()

public:
	UCreatureAnimStateMachineInstance();

	UFUNCTION(BlueprintCallable, Category = "Creature")
	void SetCondition(FString ConditionName, bool Flag);

	bool GetCondition(FString conditionName) const;

	void InitInstance(class UCreatureAnimStateMachine* forStateMachine);

	void SetCurrentState(class UCreatureAnimState *state);
	class UCreatureAnimState *GetCurrentState() const
	{
		return CurrentState;
	}

	class UCreatureMeshComponent *GetOwningMeshComponent() const;
	class UCreatureAnimStateMachine *GetTargetStateMachine() const
	{
		return TargetStateMachine;
	}

protected:

	TMap<FName, bool> m_currentConditionValues;

	UPROPERTY(Transient)
	UCreatureAnimState* CurrentState;
	
	UPROPERTY(Transient)
	UCreatureAnimStateMachine* TargetStateMachine;
	
	UFUNCTION()
	void OnAnimStart(float frame);

	UFUNCTION()
	void OnAnimEnd(float frame);

};

