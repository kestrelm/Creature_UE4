#include "CreatureEditorPCH.h"
#include "CreatureCustomSplineDetails.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"

#define LOCTEXT_NAMESPACE "CreatureCustomSplineDetails"

TSharedRef<IDetailCustomization>
FCreatureCustomSplineDetails::MakeInstance()
{
	return MakeShareable(new FCreatureCustomSplineDetails);
}
 
void FCreatureCustomSplineDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create a category so this is displayed early in the properties
	IDetailCategoryBuilder& MyCategory = DetailBuilder.EditCategory("CreatureTool", LOCTEXT("Extra info", "Creature Tool"), ECategoryPriority::Important);
	TArray<TWeakObjectPtr<UObject> > ref_objects;
	DetailBuilder.GetObjectsBeingCustomized(ref_objects);

	// Grab the first Spline component object
	active_spline = nullptr;
	for (auto cur_obj : ref_objects)
	{
		active_spline = dynamic_cast<USplineComponent *>(cur_obj.Get());
		if (active_spline)
		{
			break;
		}
	}

	MyCategory.AddCustomRow(FText::FromString(TEXT("Tools")))
        .NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Curve Export")))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent().MinDesiredWidth(500)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Save as...")))
			.OnClicked(this, &FCreatureCustomSplineDetails::ExportPressed)
		];
}

FReply FCreatureCustomSplineDetails::ExportPressed()
{
	const FString Filter(TEXT("curveData Files (*.curvedata)|*.curvedata"));
	TArray<FString> save_filenames;

	if (FDesktopPlatformModule::Get()->SaveFileDialog(
		nullptr,
		FString(TEXT("Save a Creature Curve file")),
		FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT),
		TEXT("mycurve.curvedata"),
		Filter,
		EFileDialogFlags::None,
		save_filenames))
	{
		if ((save_filenames.Num() == 1) && active_spline)
		{
			SaveCurveToFile(save_filenames[0]);
		}
	}

	return FReply::Handled();
}

void 
FCreatureCustomSplineDetails::SaveCurveToFile(const FString& write_filename)
{
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
	auto total_length = active_spline->GetSplineLength();
	auto num_pts = active_spline->GetNumberOfSplinePoints();

	const float scale_factor = 1.0f / 100.0f;
	float cur_distance = 0;
	TArray<TSharedPtr<FJsonObject>> write_objs;

	for (auto i = 0; i < num_pts; i++)
	{
		auto cur_distance = active_spline->GetDistanceAlongSplineAtSplinePoint(i);

		FVector cur_location, cur_tangent;
		active_spline->GetLocationAndTangentAtSplinePoint(i, cur_location, cur_tangent, ESplineCoordinateSpace::Local);
		
		TSharedPtr<FJsonObject> new_write_obj = MakeShareable(new FJsonObject);
		new_write_obj->SetNumberField("distance", cur_distance);
		new_write_obj->SetNumberField("posX", cur_location.X);
		new_write_obj->SetNumberField("posY", cur_location.Y);
		new_write_obj->SetNumberField("posZ", cur_location.Z);
		new_write_obj->SetNumberField("tangentX", cur_tangent.X);
		new_write_obj->SetNumberField("tangentY", cur_tangent.Y);
		new_write_obj->SetNumberField("tangentZ", cur_tangent.Z);

		write_objs.Add(new_write_obj);
	}

	int32 i = 0;
	for (auto cur_obj : write_objs)
	{
		jsonObject->SetObjectField("data" + FString::FromInt(i), cur_obj);
		i++;
	}

	jsonObject->SetNumberField("numPts", write_objs.Num());

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);
	FFileHelper::SaveStringToFile(OutputString, *write_filename);
}


#undef LOCTEXT_NAMESPACE