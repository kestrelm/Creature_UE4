/********************************************************************************
** auth： God Of Pen
** desc： 这是动画状态机编辑器的界面窗口
** Ver.:  V1.0.0
*********************************************************************************/
#include "UnrealEd.h"
#include "Editor/PropertyEditor/Public/PropertyEditing.h"
#include "Editor/PropertyEditor/Public/PropertyCustomizationHelpers.h"
#pragma once

class FCreatureAnimStateMachineDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

protected:
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
};