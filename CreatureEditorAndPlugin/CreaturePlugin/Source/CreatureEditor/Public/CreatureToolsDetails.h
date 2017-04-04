#include "IDetailCustomization.h"
#include "CreatureMeshComponent.h"
#include "Runtime/Engine/Classes/Components/SplineComponent.h"

#pragma once

class FCreatureToolsDetails : public IDetailCustomization
{
public:
    /** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual ~FCreatureToolsDetails();
 
	/** IDetailCustomization interface */
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder ) override;

    FReply LiveSyncPressed();

	FReply ExportSplinePressed();

protected:

	void InitFramework();

	void SaveCurveToFile(const FString& write_filename, USplineComponent * active_spline);

	UCreatureMeshComponent * active_mesh;
	void * client_dll;
};