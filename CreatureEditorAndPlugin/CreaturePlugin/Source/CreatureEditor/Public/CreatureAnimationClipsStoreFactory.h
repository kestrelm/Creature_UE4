/********************************************************************************
** auth£∫ God Of Pen
** desc£∫ …–Œ¥±‡–¥√Ë ˆ
** Ver.:  V1.0.0
*********************************************************************************/

#include "UnrealEd.h"
#include "CreatureAnimationClipsStoreFactory.generated.h"
#pragma once
UCLASS()
class CREATUREEDITOR_API UCreatureAnimationClipsStoreFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

		// UFactory interface
		virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};
