#pragma once

#include "CreatureAnimStateMachineInstance.generated.h"

UCLASS()
class CREATUREPLUGIN_API UCreatureAnimStateMachineInstance : public UObject
{
	GENERATED_BODY()

public:
	UCreatureAnimStateMachineInstance();

	UFUNCTION(BlueprintCallable, Category = "Creature", meta = (DeprecatedFunction, DeprecationMessage = "Please replace with SetConditionByName to improve performance"))
	void SetCondition(FString ConditionName, bool Flag);

	UFUNCTION(BlueprintCallable, Category = "Creature", meta = (DeprecatedFunction, DeprecationMessage = "Please replace with _Name version of this function to improve performance"))
	bool GetCondition(FString conditionName) const;

	UFUNCTION(BlueprintCallable, Category = "Creature")
	void SetConditionByName(FName ConditionName, bool Flag);

	UFUNCTION(BlueprintCallable, Category = "Creature")
	bool GetConditionByName(FName conditionName) const;

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

