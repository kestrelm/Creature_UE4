#include "Editor/DetailCustomizations/Private/DetailCustomizationsPrivatePCH.h"
#include "Runtime/Engine/Classes/Components/SplineComponent.h"
 
#pragma once

class FCreatureCustomSplineDetails : public IDetailCustomization
{
public:
    /** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();
 
	/** IDetailCustomization interface */
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder ) override;

    FReply ExportPressed();

protected:

	void SaveCurveToFile(const FString& write_filename);

	USplineComponent * active_spline;
};