/********************************************************************************
** auth£º God Of Pen
** desc£º ¶¯»­×´Ì¬½Úµã
** Ver.:  V1.0.0
*********************************************************************************/
#include "Engine.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraph.h"
#include "CreatureAnimState.h"
#include "CreatureAnimStateNode.generated.h"
#pragma  once
UCLASS()
class CREATUREEDITOR_API UCreatureAnimStateNode :public UEdGraphNode
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TArray<UEdGraphPin*> InputPins;

	UPROPERTY()
	TArray<UEdGraphPin*> OutputPins;

	UPROPERTY(EditAnyWhere, Category = "CreaturePlugin")
	FString AnimName;

	UPROPERTY()
	UCreatureAnimState* CompiledState;

public:
	UCreatureAnimStateNode()
		:UEdGraphNode()
	{
		
		NodeHeight = 50;
		AnimName = TEXT("DefaultAnimName");
	
	}

	virtual void OnRenameNode(const FString& NewName)override;
	virtual bool CanUserDeleteNode() const override;
#ifdef WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	void InitNode(class UCreatureAnimStateMachine*);

	virtual void Compile();

	virtual FLinearColor GetNodeTitleColor() const override;
};
