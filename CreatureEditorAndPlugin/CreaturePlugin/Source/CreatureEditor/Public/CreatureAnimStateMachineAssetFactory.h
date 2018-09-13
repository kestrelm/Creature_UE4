/********************************************************************************
** Author God Of Pen
** Ver.:  V1.0.0
*********************************************************************************/

#include "CreatureAnimStateMachine.h"
#include "CreatureAnimStateMachineAssetFactory.generated.h"
#pragma once
UCLASS()
class CREATUREEDITOR_API UCreatureAnimStateMachineAssetFactory : public UFactory
{
	GENERATED_UCLASS_BODY()
		
	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};
