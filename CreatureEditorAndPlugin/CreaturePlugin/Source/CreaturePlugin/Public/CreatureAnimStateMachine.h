#pragma once
#include "Engine.h"
#include "AnimationRuntime.h"
#include "EdGraph/EdGraph.h"
#include "CreatureTransitionCondition.h"
#include "CreatureMeshComponent.h"
#include "CreatureAnimStateMachine.generated.h"

UCLASS(BlueprintType)
class CREATUREPLUGIN_API UCreatureAnimStateMachine :
	public UObject
{
	GENERATED_BODY()
		UCreatureAnimStateMachine();
public:
	UPROPERTY(EditAnywhere, Category = "CreatureAnimStateMachine")
		FString Name;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UEdGraph* StateMachineGraph=nullptr;
#endif /*WITH_EDITORONLY_DATA*/

	UPROPERTY(VisibleAnyWhere, Category = "Creature")
	TArray<FCreatureTransitionCondition> TransitionConditionList;

	UCreatureAnimState* CurrentState;
	//根节点，用于从该State开始进行状态转换
	UPROPERTY()
		UCreatureAnimState* RootState;

	UCreatureMeshComponent* OwningComponent;

public:
	UFUNCTION(BlueprintCallable, Category = "Creature")
		void SetCondition(FString ConditionName, bool Flag);
	//初始化状态机，播放默认根节点动画
	void InitStateMachine();

	UFUNCTION()
	void OnAnimStart(float frame);

	UFUNCTION()
	void OnAnimEnd(float frame);
};

