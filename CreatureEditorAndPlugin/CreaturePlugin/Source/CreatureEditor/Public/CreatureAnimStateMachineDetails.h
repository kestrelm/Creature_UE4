/********************************************************************************
** Author God Of Pen
** Ver.:  V1.0.0
*********************************************************************************/

#include "Editor/PropertyEditor/Public/PropertyCustomizationHelpers.h"
#include "IDetailCustomization.h"

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