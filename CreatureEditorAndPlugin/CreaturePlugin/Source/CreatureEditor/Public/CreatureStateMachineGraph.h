/********************************************************************************
** auth： God Of Pen
** desc： 用于Creature状态机的节点图类
** Ver.:  V1.0.0
*********************************************************************************/
#include "Engine.h"
#include "EdGraph/EdGraph.h"

#include "CreatureStateMachineGraph.generated.h"
#pragma once
UCLASS()
class  UCreatureStateMachineGraph :public UEdGraph{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY()
	class	UCreatureAnimStateMachine* ParentStateMachine;
	UPROPERTY()
	class	UCreatureAnimStateNode* DefaultRootNode;
	
	virtual void Serialize(FArchive& Ar) override;
	//编译当前状态机图中的所有节点
	void CompileNodes();
	void CreateDefaultStateNode();
private:

	void OnBeginPIE(const bool bIsSimulating);
	void OnGraphChanged(const FEdGraphEditAction& Action);

	bool m_isDirty;

	UPROPERTY()
	TArray<UCreatureAnimStateNode*> StateNodeList;
};
