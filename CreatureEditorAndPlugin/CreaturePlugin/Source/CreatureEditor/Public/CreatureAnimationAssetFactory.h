/********************************************************************************
** auth： God Of Pen
** desc： 用于产生CreatureAnimationAsset，同时处理打开JSon文件并导入的功能
** Ver.:  V1.0.0
*********************************************************************************/
#include "UnrealEd.h"
#include "CreatureAnimationAssetFactory.generated.h"
#pragma once
UCLASS()
class CREATUREEDITOR_API UCreatureAnimationAssetFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

		// UFactory interface
		virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};
