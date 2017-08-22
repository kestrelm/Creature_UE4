#include "CreatureEditorPCH.h"
#include "CreatureToolsDetails.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "UnrealEd.h"
#include "BusyCursor.h"
#include "ScopedTransaction.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "CreatureToolsDetails"

// DLL Function imports
typedef bool(*_dll_startConnection)();
typedef void(*_dll_stopConnection)();
typedef bool(*_dll_isConnected)();
typedef const char *(*_dll_retrieveRequestExportFilename)(char * msg_type);

_dll_startConnection creatureClient_startConnection;
_dll_stopConnection creatureClient_stopConnection;
_dll_isConnected creatureClient_isConnected;
_dll_retrieveRequestExportFilename creatureClient_retrieveRequestExportFilename;

// FCreatureToolsDetails
TSharedRef<IDetailCustomization>
FCreatureToolsDetails::MakeInstance()
{
	return MakeShareable(new FCreatureToolsDetails);
}
 
FCreatureToolsDetails::~FCreatureToolsDetails()
{
	if (creatureClient_isConnected())
	{
		creatureClient_stopConnection();
	}
}

void FCreatureToolsDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	InitFramework();

	// Create a category so this is displayed early in the properties
	IDetailCategoryBuilder& MyCategory = DetailBuilder.EditCategory("CreatureTool", LOCTEXT("Extra info", "Creature Tool"), ECategoryPriority::Important);
	TArray<TWeakObjectPtr<UObject> > ref_objects;
	DetailBuilder.GetObjectsBeingCustomized(ref_objects);

	active_mesh = nullptr;
	for (auto cur_obj : ref_objects)
	{
		active_mesh = dynamic_cast<UCreatureMeshComponent *>(cur_obj.Get());
		if (active_mesh)
		{
			break;
		}
	}

	MyCategory.AddCustomRow(FText::FromString(TEXT("Tools")))
        .NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Live Sync")))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent().MinDesiredWidth(500)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Sync Now...")))
		.OnClicked(this, &FCreatureToolsDetails::LiveSyncPressed)
		];

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
			.OnClicked(this, &FCreatureToolsDetails::ExportSplinePressed)
			];
}

FReply FCreatureToolsDetails::LiveSyncPressed()
{
	if (!client_dll)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString("Only Windows 64 bit platforms supported for this feature."));
		return FReply::Handled();
	}

	if (!active_mesh) 
	{
		return FReply::Handled();
	}

	const FScopedBusyCursor BusyCursor;
	if (!creatureClient_isConnected())
	{
		auto can_connect = creatureClient_startConnection();
		if (!can_connect)
		{
			creatureClient_stopConnection();
			FMessageDialog::Open(
				EAppMsgType::Ok,
				FText::FromString("Cannot connect to Creature Server! Make sure you have Creature running in Animation Mode and try again."));

			return FReply::Handled();
		}
	}

	auto retrieve_filename = FString(creatureClient_retrieveRequestExportFilename((char *)"REQUEST_JSON"));

	if (retrieve_filename.IsEmpty())
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString("Make sure you have Creature running in Animation Mode and try again."));

		return FReply::Handled();
	}

	FString raw_json;
	if (!FFileHelper::LoadFileToString(raw_json, *retrieve_filename, 0))
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString("Invalid Data for import, try again."));

		return FReply::Handled();
	}
	
	// Update data
	{
		UCreatureAnimationAsset * refresh_anim_asset = active_mesh->creature_animation_asset;
		const FScopedTransaction Transaction(NSLOCTEXT("Creature", "CreatureAsset", "Live Sync"));
		refresh_anim_asset->SetFlags(RF_Transactional);
		refresh_anim_asset->Modify();
		refresh_anim_asset->SetNewJsonString(raw_json);

		auto cur_world = GEditor->GetEditorWorldContext().World();
		for (TActorIterator<AActor> ActorItr(cur_world); ActorItr; ++ActorItr)
		{
			auto * cur_actor = *ActorItr;
			UCreatureMeshComponent * cur_mesh = cur_actor->FindComponentByClass<UCreatureMeshComponent>();
			if (cur_mesh)
			{
				if (cur_mesh->creature_animation_asset == refresh_anim_asset)
				{
					cur_actor->ReregisterAllComponents();
				}
			}
		}
	}



	return FReply::Handled();
}

void FCreatureToolsDetails::InitFramework()
{
	client_dll = nullptr;
	auto cur_platform = UGameplayStatics::GetPlatformName();
	if (cur_platform == "Windows")
	{
		FString filePath = *FPaths::GamePluginsDir() + FString("/CreaturePlugin/Source/ThirdParty/CreatureClientDLL.dll");
		client_dll = FPlatformProcess::GetDllHandle(*filePath);

		if (client_dll)
		{
			std::vector<void *> check_loaded_functions;

			creatureClient_startConnection =
				(_dll_startConnection)FPlatformProcess::GetDllExport(client_dll, 
					*FString("creatureClient_startConnection"));
			check_loaded_functions.push_back((void *)creatureClient_startConnection);

			creatureClient_stopConnection =
				(_dll_stopConnection)FPlatformProcess::GetDllExport(client_dll,
					*FString("creatureClient_stopConnection"));
			check_loaded_functions.push_back((void *)creatureClient_stopConnection);

			creatureClient_isConnected =
				(_dll_isConnected)FPlatformProcess::GetDllExport(client_dll,
					*FString("creatureClient_isConnected"));
			check_loaded_functions.push_back((void *)creatureClient_isConnected);

			creatureClient_retrieveRequestExportFilename =
				(_dll_retrieveRequestExportFilename)FPlatformProcess::GetDllExport(client_dll,
					*FString("creatureClient_retrieveRequestExportFilename"));
			check_loaded_functions.push_back((void *)creatureClient_retrieveRequestExportFilename);

			for (auto cur_func : check_loaded_functions)
			{
				if (cur_func == nullptr)
				{
					client_dll = nullptr;
					UE_LOG(LogTemp, Warning, TEXT("Creature Client DLL Load error!"));
					break;
				}
			}
		}
	}
}

FReply FCreatureToolsDetails::ExportSplinePressed()
{
	if (!active_mesh)
	{
		return  FReply::Handled();
	}

	auto actor_parent =
		active_mesh->GetOwner();
	USplineComponent * active_spline =
		actor_parent->FindComponentByClass<USplineComponent>();
	if (active_spline)
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
				SaveCurveToFile(save_filenames[0], active_spline);
			}
		}
	}
	else {
		FMessageDialog::Open(
			EAppMsgType::Ok, 
			FText::FromString("You need to add a Spline Component to your Actor in order for this function to run!"));
	}

	return FReply::Handled();
}

void
FCreatureToolsDetails::SaveCurveToFile(const FString& write_filename, USplineComponent * active_spline)
{
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
	auto total_length = active_spline->GetSplineLength();
	auto num_pts = active_spline->GetNumberOfSplinePoints();

	const float scale_factor = 1.0f / 100.0f;
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